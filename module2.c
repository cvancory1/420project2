
/*

  Chloe VanCory,  Kalyn Howes & Bevan Smith
  COSC 420
  11/ 20 /2021
  Project 2



*/

#include <ctype.h>
#include <fcntl.h>
#include <mpi.h>
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


void printbuffer(double* buf, int size) {
  for (int i = 0; i < size; i++) {
    printf("arr=%f\n", buf[i]);
  }
}

int * findCounts(int count , int worldsize){
  int * temp = malloc(worldSize * sizeof(int));

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

void everyonePrint(int rank , char * str , int *  arr ){
  printf("Rank =%d %s=%d\n", rank, str, arr[rank]);
}




//  QUESTION - DOES THIS NEED TO HAVE THE GLBAL PAPER ID ALSO? 
// is it a int * ?? because it is just holding either a 0 or 1 
typedef struct rows {
  int length;
  int * data; // Holds the global index from db that this paper cites 
} rows;

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);
  world = MPI_COMM_WORLD;

  // passing the container for the result in second param
  MPI_Comm_size(world, &worldSize);
  MPI_Comm_rank(world, &rank);
  MPI_Get_processor_name(name, &nameLen);


  // opens the citation file 
  FILE * fp;
  // fp = fopen("testCitations.txt", "r");
  // int TOTALPAPERS = 50; 

  int TOTALPAPERS = 1354753; 
  fp = fopen("arxiv-citations.txt", "r");


  if(fp== NULL ) printf("ERROR opening file ");



  // tracks the amount of papers each proc will read in
  int * localPapercount = findCounts(TOTALPAPERS, worldSize); 
  
  //based on localpapercount , calc the bytes ( ie cant divide bytes / worldsize since the file wouldnt be parsed correctly in every case)
  int* sendcnt = malloc(worldSize * sizeof(int));


  // initialize & allocate MatrixA for power method 
  /* assume there are at least 10 citations and allocate, if more than reallocate later */ 

  rows matrixA [TOTALPAPERS];
  int MALLOCEDSIZE = 10 ; 
  if(rank == ROOT) {
    for(int i =0; i< TOTALPAPERS ; i++){
      matrixA[i].data = malloc(MALLOCEDSIZE * sizeof(int));
      matrixA[i].length  = 0;  
    } 

  }
  

  /* does calcs for number of bytes every proc needs to read in based on even dividsion of papers */ 
  if(rank == ROOT){
    char * line= NULL; 
    int numread;
    int curPaperCount = 0;  // counts how many papers have been read 
    int i = 0; 

    for (int i = 0; i < worldSize; i++) sendcnt[i] = 0; // initalize the sendcounts = 0 

    int citedCount = 0;  // counts the len of "rows" array
    size_t len = 0; // used by getline function (gets returned a power of 2 corresponding to bytes read)

    int matrixA_it = 0; // traverse through A arrary
    int checkCitations = 0; // ensures the name being read in is a cited paper and an actual paperID
    while ((numread = getline(&line, &len, fp)) != -1) {
      sendcnt[i] += numread; // tracks the total amount of bytes
      // printf("line: %s count: %zu\n", line, len);

        if(checkCitations == 1 && line[0] != '+'){
          matrixA[matrixA_it].length++;
          // printf("matrixA_it =%d %s\n", matrixA_it , line);
          // TODO - Query bevans db here

          // more than 10 citations , need to realloc the data array
          if(matrixA[matrixA_it].length > MALLOCEDSIZE ){
            int newSize = matrixA[matrixA_it].length * 2; // curent length * 2, first resize occurs after 10 citations
            matrixA[matrixA_it].data = realloc(newSize * sizeof(int));

          }

        }else if (checkCitations == 1 && line[0] == '+' ) {
          checkCitations =0; 
        }

        int length = strlen(line);
        // printf("length=%d \n", length);
        // printf("line[len-1]=%c  line[len] =%c ",line[length-2] , line[length-1] );
        if(line[length-2] == '-' && line[length-1] == '\n'){
          
          // citedCount++;
          // matrixA[curPaperCount].length++;
          // // printf("ENTER \n");
          checkCitations = 1; // the next name you read in is a citation

        }


        // +,  a new paper is being read from the file
        if(line[0] == '+' ){
          curPaperCount++;
          matrixA_it++;

          if(curPaperCount == localPapercount[i] ){
            // printf("citedCount=%d \n", citedCount);
            citedCount =0; // resets the cited count

            i++;
            curPaperCount=0;
          }

        }
        
        free(line);
        line =NULL; 
        len =0; 

    }

  }

  // all nodes know how many to read in from the file 
  MPI_Bcast(sendcnt, worldSize, MPI_INT, ROOT, world);
  // everyonePrint(rank, "count", sendcnt);

  
  
  // printing results 
  if(rank == ROOT) {
    for(int i =0; i< TOTALPAPERS ; i++){
      printf("i=%d matrixA.length=%d\n", i, matrixA[i].length );
    } 
  }



  
  MPI_Finalize();
  return 0;
}
