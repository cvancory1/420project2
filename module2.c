
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

typedef struct rows {
  int length;
  int* data;
} row;

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);
  world = MPI_COMM_WORLD;

  // passing the container for the result in second param
  MPI_Comm_size(world, &worldSize);
  MPI_Comm_rank(world, &rank);
  MPI_Get_processor_name(name, &nameLen);

  FILE * fp;
  fp = fopen("testCitations.txt", "r");

  if(fp== NULL ) printf("ERROR opening file ");

  int TOTALPAPERS = 50; 

   // counts - number of bytes each proc will read 
  int* sendcnt = malloc(worldSize * sizeof(int));
  int * localPapercount = findCounts(TOTALPAPERS, worldSize); 



  if(rank == ROOT){
    char * line= NULL; 
    int numread;
    int curPaperCount = 0;  // counts how many papers have been read 
    int i = 0; 

    for (int i = 0; i < worldSize; i++) sendcnt[i] = 0; // initalize the sendcounts = 0 

    int citedCount = 0;  // counts the len of "rows" array
    size_t len = 0;

    while ((numread = getline(&line, &len, fp)) != -1) {
      sendcnt[i] += numread;
      // line[len-1] = '*';
      // printf("line: %s count: %zu\n", line, len);

        int length = strlen(line);
        // printf("length=%d \n", length);
        printf("line[len-1]=%c  line[len] =%c ",line[length-2] , line[length-1] );
        if(line[length-1] == '-' && line[length] == '\n'){
          citedCount++;

        }

        // +, indicates a new paper in the list 
        if(line[0] == '+' ){
          curPaperCount++;
          if(curPaperCount == localPapercount[i] ){

            printf("citedCount=%d \n", citedCount);
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
  MPI_Bcast(sendcnt, worldSize, MPI_INT, ROOT, world);
  everyonePrint(rank, "count", sendcnt);





  
  MPI_Finalize();
  return 0;
}
