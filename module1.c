
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

#include "tree.c"

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

typedef struct node {
	char *title;
	struct node *next;
} node; 

struct node *head = NULL;
void printPapers(struct node *p) {
	//printf("%d", p != NULL);
	while (p->next != NULL) {
		printf("hi\n");
		printf("%s ", p->title);
		p = p->next;
	}
}

void insert(struct node *head, char *text) {
	struct node new = malloc(sizeof(node));
	while (head) {
		
	}
}
int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);
	world = MPI_COMM_WORLD;

	// passing the container for the result in second param
	MPI_Comm_size(world, &worldSize);
	MPI_Comm_rank(world, &rank);
	MPI_Get_processor_name(name, &nameLen);


	// opens the citation file 
	FILE * fp;

	/*
	int TOTALPAPERS = 1354753; 
	// chnage to metadata
	//fp = fopen("arxiv-citations.txt", "r");

	struct tree *test = NULL;
	
	int i;
	for (i = 0; i < argc; ++i) {
		char buf[10];
		sprintf(buf, "%d", i);
		test = tree_insert(test, argv[i], strdup(buf));
	}
	tree_print(test, 2);
	*/
	//if(fp== NULL ) printf("ERROR opening file ");
	
  
	struct node *test = head;
	test->title = "Test title 1";
	struct node *test2;
	test2->title = "New test";
	test2->next = NULL;
	test->next = test2;
	

	printPapers(head);

  MPI_Finalize();
  return 0;
}
