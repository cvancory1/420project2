
/*

  Chloe VanCory,  Kalyn Howes & Bevan Smith
  COSC 420
  11/ 20 /2021
  Project 2



*/

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "matrixFunctions.c"

#define ROOT 0 

void printbuffer(double* buf, int size) {
  for (int i = 0; i < size; i++) {
    printf("arr=%f\n", buf[i]);
  }
}

Matrix copyMatrix(Matrix copyMatrix) {
  Matrix newMatrix;
  newMatrix.rows = copyMatrix.rows;
  newMatrix.cols = copyMatrix.cols;
  newMatrix.data = malloc(newMatrix.rows * newMatrix.cols * sizeof(double));

  for (int i = 0; i < copyMatrix.rows * copyMatrix.cols; i++) {
    newMatrix.data[i] = copyMatrix.data[i];
  }

  return newMatrix;
}

typedef struct ones {
  int length;
  int* data;
} ones;

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);
  world = MPI_COMM_WORLD;

  // passing the container for the result in second param
  MPI_Comm_size(world, &worldSize);
  MPI_Comm_rank(world, &rank);
  MPI_Get_processor_name(name, &nameLen);

  /*
     arxiv-citations.txt.gz - list of all the other papers which it cites.
     i. Each paper is in a block of text separated from the rest by +++++
     ii. The first line of the block is the id of the paper. It is then
    separated fro the other ids with
     -----
     iii. All subsequent lines (if any) are papers cited by that paper (i.e.
    outgoing connections in the graph)


    read in paper id
    read in the ---
    while there are more papers cited read them in until the ++++
  */

  // MPI_File citationFile;
  // char* citationFilename = "testCitations.txt";

  // // open the file
  // int status = MPI_File_open(world,                            // comm
  //                            citationFilename,                      // filename
  //                            MPI_MODE_CREATE | MPI_MODE_RDWR,  // mode
  //                            MPI_INFO_NULL,                    // info structure
  //                            &citationFile);

  // if (status == -1) {
  //   puts("ERROR opening file"); 
  //   MPI_Finalize(); 
  //   return 1;
  // }

  // read in a chunk of the file to parse into the matrix later 
  int buffersize = 1;
  char buff[ 2 ];
  int * offset;
  int cntIndex = 0, numBytes; 
  // int fd = open("testCitations.txt", O_RDONLY);
  const unsigned long  CITATIONBYTES = 1664;
  // printf("%lu\n", CITATIONBYTES);

  int totalPapers = 0; // counts the total papers each proc will read in  ( use for the array)
  int curCitationCount = 0; // counter for length of array
  int newEntry = 0; // bool - tracks if we read first or the second set of pluses

  long * sendcnt = malloc( worldSize * sizeof(long) );
  for(int i =0; i< worldSize ;i++) sendcnt[i] = 0;

  // FILE* fd = NULL; 
  int fd; 
  if(rank == ROOT ){
    fd = open("testCitations.txt", O_RDONLY);
    if(fd == -1 ) printf("File did not open\n");
    printf("starting read\n");
    while((numBytes= read(fd,buff,buffersize))>0){
      // new paper to read in and calculate citations cited for 
      sendcnt[ cntIndex ]++;
      printf("current = %c  bytesRead =%ld \n" , buff[0], sendcnt[ cntIndex ]);

      if(buff[0] == '+' ){
        printf("enter addition\n");
        if(newEntry %2 == 0){
          totalPapers++;
        }
        // lseek(fd, 5 , SEEK_CUR); 
        sendcnt[ cntIndex ]+= 5; 
      }

      // THIS IS READING IN THE DASH AND 
      // sleep(1);
      // citation numbers next 
      if(buff[0] == '-'){
        printf("enter subtraction\n");
        curCitationCount++;
        // lseek(fd, 5 , SEEK_CUR); 
        sendcnt[ cntIndex ]+= 5;
        
      }



    }


  }




  // const unsigned long  CITATIONBYTES = 119367527;





  if(fd) close(fd);
  MPI_Finalize();
  return 0;
}

// OLD CODE FROM LAB 4 - version 1.c
/*



int main(int argc, char** argv) {

  MPI_Init(&argc, &argv);
  world = MPI_COMM_WORLD;

  // passing the container for the result in second param
  MPI_Comm_size(world, &worldSize);
  MPI_Comm_rank(world, &rank);
  MPI_Get_processor_name(name, &nameLen);

  MPI_File matDataFile, matDimFile;

  char* matfilename = "matData.txt";
  char* dimfilename = "matDims.txt";
  int rows = atoi(argv[1]);
  int cols = atoi(argv[2]);
  int THRESHOLD = atoi(argv[3]);
  double e = 10E-16;


  if(argc != 4){
    //   MPI_Barrier(world);
      printf("NOT ENOUGH ARGS PASSED FOR DIMENSIONS");
      MPI_Finalize();
      return 1;
  }


  // generate the numbers
  double* arrNum = malloc(rows * cols * sizeof(double));

  if (rank == ROOT) {
    srand(time(0));
    for (int i = 0; i < rows * cols; i++) {
      arrNum[i] = rand() % 100 + 1;
      // arrNum[i] =1;
    //   printf("arrNum[i]=%f\n", arrNum[i]);
    }
  }
  MPI_Bcast(arrNum, rows * cols, MPI_DOUBLE, ROOT, world);


  // // open the file
  int status = MPI_File_open(world,                            // comm
                             matfilename,                         // filename
                             MPI_MODE_CREATE | MPI_MODE_RDWR,  // mode
                             MPI_INFO_NULL,                    // info structure
                             &matDataFile);

  if (status == -1) {
    MPI_Finalize();
    puts("ERROR opening file");
     MPI_Finalize();
    return 1;
  }

  status = MPI_File_open(world,                            // comm
      dimfilename,                         // filename
      MPI_MODE_CREATE | MPI_MODE_RDWR,  // mode
      MPI_INFO_NULL,                    // info structure
      &matDimFile);


  // // write to the newfile
  // MPI_Barrier(world);

  if( rank == 0 ){
  int offset = 0;
  MPI_File_write(matDimFile,
                    // 1,  // offset
                    &rows,   // buf
                    1, MPI_INT, MPI_STATUS_IGNORE);

  offset += sizeof(int);
   MPI_File_write(matDimFile,
                    // 1,  // offset
                    &cols,   // buf
                    1, MPI_INT, MPI_STATUS_IGNORE);

  offset += sizeof(int);
    MPI_File_write(matDataFile,
                  // 1,  // offset
                  arrNum,   // buf
                  rows *cols, MPI_DOUBLE, MPI_STATUS_IGNORE);
  }

  MPI_Barrier(world);

  MPI_File_seek(matDataFile, 0, MPI_SEEK_SET);
  MPI_File_seek(matDimFile, 0, MPI_SEEK_SET);



  Matrix A;

  // parse the file and into a matrix of doubles

  MPI_File_read(matDimFile,
                &A.rows,
                1,
                MPI_INT,
                MPI_STATUS_IGNORE);
  printf("Rank %d A.rows =%d\n", rank, A.rows);

  MPI_File_read(matDimFile,
                &A.cols,
                1,
                MPI_INT,
                MPI_STATUS_IGNORE);
  printf("Rank %d A.cols =%d\n", rank, A.cols );

  A.data = malloc(A.rows * A.cols * sizeof(double));

  MPI_File_read(matDataFile,
                A.data,
                A.rows*A.cols,
                MPI_DOUBLE,
                MPI_STATUS_IGNORE);


  // if(rank ==ROOT){
  //   printMatrix(A);
  // }


  SGData scatter = getSGCounts(A.rows, A.cols , worldSize);

  Matrix localMatrix;
  localMatrix.rows =  scatter.cnts[rank] / A.cols;
  localMatrix.cols = A.cols;
  localMatrix.data = malloc(localMatrix.rows  * localMatrix.cols *
sizeof(double)); printf("RANK =%d localMatrix rows= %d cols=%d\n",
rank,localMatrix.rows,localMatrix.cols); printf("Rank %d recv count %d\n", rank,
scatter.cnts[rank]); printf("Rank %d Scatter recvbuf sendbuf: %p %p\n", rank,
localMatrix.data, A.data); MPI_Scatterv( A.data,                // sendbuf
            scatter.cnts,        // sendcnts
            scatter.displs,      // displacements
            MPI_DOUBLE,               // datatype
            localMatrix.data,              // recvbuf
            scatter.cnts[rank],  // recvcnt
            MPI_DOUBLE, ROOT, world);


  // char *arrbuf = bufArr(localMatrix.data, scatter.cnts[rank]);
  // printf("Rank %d received %s\n", rank, arrbuf);

  // everyone declares a  X vector
  Matrix X;
  X.rows = localMatrix.cols;
  X.cols = 1;
  X.data= malloc(X.rows * X.cols * sizeof(double));
//   printf("RANK =%d localX rows= %d cols=%d\n", rank,X.rows,X.cols);


//   for(int i =0 ;i < X.rows* X.cols; i++) X.data[i] = 1;
// //   char *arrbuf2 = bufArr(X.data, X.rows* X.cols);
// //   printf("Rank %d received %s\n", rank, arrbuf2);

//   double startTime, stopTime;
//   startTime = MPI_Wtime();
//   double eigenvalue = powerMethod(localMatrix,X,A.rows, A.cols, THRESHOLD,
//   e); stopTime = MPI_Wtime(); if(rank ==0 ) printf("MPI_Wtime measured: %2.5f
//   seconds\n", stopTime-startTime); fflush(stdout); // manually flush the
//   buffer for safety

//   if(rank ==0 )printf("EIGENVALUE= %f\n",eigenvalue );

//   MPI_File_close(&matDataFile);
//   MPI_File_close(&matDimFile);

//   MPI_Finalize();
//   return 0;
// }

// */