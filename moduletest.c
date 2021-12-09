
/*

  Chloe VanCory,  Kalyn Howes & Bevan Smith
  COSC 420
  11/ 20 /2021
  Project 2



*/

#include <ctype.h>
#include <fcntl.h>
#include <mpi.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "matrixFunctions.c"

#define ROOT 0
// const unsigned long  CITATIONBYTES = 119367527;

void printbuffer(double *buf, int size) {
  for (int i = 0; i < size; i++) {
    printf("arr=%f\n", buf[i]);
  }
}

int *findCounts(int count, int worldsize) {
  int *temp = malloc(worldSize * sizeof(int));

  int minSend = count / worldSize;
  // printf("minsend=%d \n", minSend);
  for (int i = 0; i < worldSize; i++) {
    temp[i] = minSend;
  }

  for (int i = 0; i < count % worldSize; i++) {
    temp[i]++;
  }
  return temp;
}

int callback(void *NotUsed, int argc, char **argv, char **azColName) {
  NotUsed = 0;
  for (int i = 0; i < argc; i++) {
    // printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  // printf("\n");

  // if(argv[0] !=  NULL)
  //   return argv[0];

  return 0;
}

void printArr(AdjacenyList temp) {
  for (int i = 0; i < temp.length; i++) {
    printf("arr=%d\n", temp.data[i]);
  }
}

//  QUESTION - DOES THIS NEED TO HAVE THE GLBAL PAPER ID ALSO?
// is it a int * ?? because it is just holding either a 0 or 1

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);
  world = MPI_COMM_WORLD;

  // passing the container for the result in second param
  MPI_Comm_size(world, &worldSize);
  MPI_Comm_rank(world, &rank);
  MPI_Get_processor_name(name, &nameLen);

  // for powermethod
  // int THRESHOLD = atoi(argv[3]);
  int ITERATION = 5;
  double epsilon = 10E-6;

  /***** Open Database ******/
  sqlite3 *db;
  char *err_msg = 0;
  sqlite3_stmt *res;

  int rc = sqlite3_open("meta.db", &db);

  if (rc != SQLITE_OK) {
    fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);

    return 1;
  }

  Matrix test;
  test.rows = 5;
  test.cols = 5;

  double arr2[] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1,
                   1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1};

  // setting test to the input arr
  if (rank == ROOT) {
    test.data = malloc(sizeof(double) * test.rows * test.cols);
    for (int i = 0; i < test.rows * test.cols; i++) {
      test.data[i] = arr2[i];
    }
  }

  AdjacenyList *listA =
      malloc(test.rows * 2 * sizeof(int) *
             sizeof(int *));  // size of the lengths and size of the int *
  int MALLOCEDSIZE = 10;
  int lengthsData [] = {1, 2, 2, 0, 3};
  int indexesData[] = {0, 0, 4, 2,
                   3, 2, 3, 4};  // col index of where the 1's are located
  int * lengths;
  int * indexes;

  if (rank == ROOT) {
    lengths = malloc(sizeof(int) * 5);
    indexes = malloc(sizeof(int) * 8);
    for(int i =0 ;i < 5 ;i++) lengths[i] = lengthsData[i];
    for(int i =0 ;i < 8 ;i++) indexes[i] = indexesData[i];
  }else{
    lengths = NULL;
    indexes = NULL;

  }
  

  if (rank == ROOT) {
    // Matrixlengths.data =malloc(Matrixlengths.rows * Matrixlengths.cols *
    // sizeof(int));
    int count = 0;
    for (int i = 0; i < test.rows; i++) {
      listA[i].data = malloc(MALLOCEDSIZE * sizeof(int));
      listA[i].length = lengths[i];

      for (int j = 0; j < listA[i].length; j++) {
        listA[i].data[j] = indexes[count++];
      }
      // printf("printing matrix i=%d  listA[i].length =%d \n", i, listA[i].length);

      listA[i].globalID = -1;
      // Matrixlengths.data[i] = 0;
    }
    // Matrixlengths.data = NULL;
  }

  Matrix ones;
  ones.rows = test.rows;
  ones.cols = 1;
  ones.data = malloc(sizeof(double) * ones.rows * ones.cols);
  for (int i = 0; i < test.rows; i++) {
    ones.data[i] = 1;
  }

  SGData length_counts = getSGCounts(test.rows, 1, worldSize);
  // everyonePrint(rank, "disls=", length_counts.displs);
  // everyonePrint(rank, "cnts=", length_counts.cnts);

  int *localLenghts = malloc(sizeof(int) * length_counts.cnts[rank]);

  puts("here");
  
  // scattering the lengths
  MPI_Scatterv(lengths,                   // sendbuf
               length_counts.cnts,        // sendcnts
               length_counts.displs,      // displacements
               MPI_INT,                   // datatype
               localLenghts,              // recvbuf
               length_counts.cnts[rank],  // recvcnt
               MPI_INT, ROOT, world);

  // char *arrbuf = bufArr( localLenghts, length_counts.cnts[rank] );
  // printf("Rank %d received %s\n", rank, arrbuf);


  // if(rank == 1){
  for (int i = 0; i < length_counts.cnts[rank]; i++) {
    // everyonePrint(rank, "arr=", localLenghts[i]);
    // printf(" i = %d arr=%d \n", i, localLenghts[i]);
  }

  // }


  // MPI_Barrier(world);
  // everyone malloc their own local structs of listA
  AdjacenyList *locallistA = malloc(length_counts.cnts[rank] * sizeof(AdjacenyList));
  for (int i = 0; i < length_counts.cnts[rank]; i++) {
    locallistA[i].length = localLenghts[i];

    if (locallistA[i].length > 0) {
      locallistA[i].data = malloc(localLenghts[i] * sizeof(int));
    } else {
      locallistA[i].data = NULL;
    }
    locallistA[i].globalID = -1;
  }

  // // // root sends the adjacenylist data
  if (rank == 0) {
    int sendRoot = 0;
    int count = 0;

    for (int i = 0; i < test.rows; i++) {
      for (int j = 0; j < listA[i].length; j++) {
        // printf("list i=%d j=%d arr= %d \n", i, j , listA[i].data[j]);
      }
      printf("rank =%d sendroot=%d  i=%d  count=%d length =%d \n",rank, sendRoot, i, count,  listA[i].length );


      MPI_Send(listA[i].data,    // buf
          listA[i].length,  // amount sending - count 
          MPI_INT,          // dtype
          sendRoot,         //  dest
          sendRoot,         // tag
          world         // comm
      );
     MPI_Recv(locallistA[i].data, localLenghts[i], MPI_INT, ROOT, MPI_ANY_TAG,
             world, &status);

      // printf("j=%d arr= %d sendRoot =%d \n", i , listA[i].data[0], sendRoot);
      // printf(
      //     " ====rank =%d count =%d  i=%d destRoot =%d cnts[sendRoot]=%d "
      //     "arr[0]= %d lengthsent[i]=%d  \n",
      //     rank, count, i, sendRoot, length_counts.cnts[sendRoot],
      //     listA[i].data[0], listA[i].length);
 
      
      count++;

      if (count == length_counts.cnts[sendRoot]) {
        // printf("sendroot=%d  i=%d  count=%d \n",sendRoot, i, count );
        sendRoot++;
        count = 0;
      }
    }
  }


  printf("rank=%d finished \n",rank);

  int number_amount;
  int i = 0;
  for (i = 0; i < length_counts.cnts[rank]; i++) {
    MPI_Status status;

    MPI_Recv(locallistA[i].data, localLenghts[i], MPI_INT, ROOT, MPI_ANY_TAG,
             world, &status);

    MPI_Get_count(&status, MPI_INT , &locallistA[i].data);
    printf("1 received %d numbers from 0. Message source = %d, "
    "tag = %d\n", number_amount, status.MPI_SOURCE, status.MPI_TAG);
  }













  // // error checking recv arrays
  // if (rank == 0) {
  //   for (int i = 0; i < length_counts.cnts[rank]; i++) {
  //     if (locallistA[i].length > 0) {
  //       for (int j = 0; j < locallistA[i].length; j++) {
  //         // printf("Here rank =%d\n", rank);
  //         //  printf("rank =%d i = %d arr= %d \n",rank,i,
  //         //  locallistA[i].data[0]);
  //         // printf(" ---rank = %d  localListIndex=%d totallength = %d  data= %d
  //         // \n", rank, i , locallistA[i].length, locallistA[i].data[j]);
  //       }
  //     } else {
  //       // printf("---rank =%d  i = %d  arr=NULL \n",rank, i);
  //     }
  //   }
  // }

  // // printMatrix(localTest);
  // double e = 10E-16;
  // printf("-%p \n", ones.data);
  // newpowermethod(locallistA, ones, length_counts.cnts[rank], 5, 5, e);
  
  // MPI_Barrier(world);

  // if(rank == ROOT){
  //   puts("=======-priting X ------");
  //   printMatrix(ones);
  // }
    
  



  // TODO include the lab4 case in this folder 

  sqlite3_close(db);
  MPI_Finalize();

  return 0;
}
