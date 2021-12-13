
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


//#include "selist.c"
#include "simclist.h"
#include "simclist.c"

#include "tree.c"


/*
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
*/




//  QUESTION - DOES THIS NEED TO HAVE THE GLBAL PAPER ID ALSO? 
// is it a int * ?? because it is just holding either a 0 or 1 
/*
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
*/

int isWord(char *string) {

	for (int i = 0; i < strlen(string); i++) {
		//printf("%c ", string[i]);
		if (!(isalpha(string[i]))) {
			return 0;
		}
	}

	return 1;
}

int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);
	MPI_Comm world = MPI_COMM_WORLD;
	
	int worldSize, rank;

	MPI_Comm_size(world, &worldSize);
	MPI_Comm_rank(world, &rank);
	

	// opens the citation file 
	FILE * fp;

	//if (rank == 0) printf("res: %d\n", isAlpha("test."));

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
	
  
	
//	selist test;

//	selist_set_insert(&test, "hi",  
//
/*
	list_t mylist;
	list_init(&mylist);

	int u;

	u = 10;
	list_append(&mylist, &u);

	u = 1;
	list_append(&mylist, &u);
	*/
    //printf("The list now holds %u elements.\n", \ list_size(& mylist));           /* get the size of the list */

//	list_destroy(&mylist);


	int *sendcounts = malloc(worldSize*sizeof(int));
	//int lines = 100;
	int lines = 50;
	int papers = lines / 5;
	int start = papers / worldSize;
	for (int i = 0; i < worldSize; i++) {
		sendcounts[i] = start;
	}
	for (int i = 0; i < lines%worldSize; i++) {
		sendcounts[i]++;
	}

	if (rank == 0) {
		for (int i = 0; i < worldSize; i++) {
			//printf("Will scatter %d lines to rank %d\n", sendcounts[i], i);
		}
	}
	
	for (int i = 0; i < worldSize; i++) {
		//printf("SENDCOUNTS %d and COLS %d\n", sendcounts[i], A.cols);
	}

	int *displ = malloc(worldSize*sizeof(int));
	displ[0] = 0;
	for (int i = 1; i < worldSize; i++) {
		displ[i] = displ[i-1] + sendcounts[i-1];
		//printf("displ: %d\n", displ[i]);
	}

	if (rank == 0) {
		for (int i = 0; i < worldSize; i++) {
			//printf("Will start scattering to rank %d at index %d\n", i, displ[i]);
		}
	}



	//fp = fopen("example.txt", "r");
	fp = fopen("chloe.txt", "r");

	int count = 0;
	int index = 0;
	char buf[221072];
	//char *id = malloc(25555*sizeof(char));
	char *title = malloc(96384*sizeof(char));
	//char *author = malloc(95536*sizeof(char));
	char *abstract = malloc(95536*sizeof(char));
	//char ins[231072];

	list_t mylist;
	list_init(&mylist);

	list_attributes_copy(&mylist, list_meter_string, 1);
	list_attributes_comparator(&mylist, list_comparator_string);

	while (fgets(buf, 221072, fp)) {
		buf[strlen(buf)-1] = '\0';
		//printf("%s\n", buf);

		// id
		if (count % 5 == 0) {
		}
		// title
		else if (count % 5 == 1) {
			if (rank != worldSize-1) {
			//if (rank <= worldSize-1) {
				if (index >= displ[rank] && index < displ[rank+1]) {
					//printf("buf %s and rank %d\n", buf, rank);
					char *string = strtok(buf, " ");
					while (string != NULL) {
						if (isWord(string)) {
							if (!(list_contains(&mylist, string))) {
								list_append(&mylist, string);	
								//printf("z: %s\n", string);
							}
						}
						string = strtok(NULL, " ");
					}	
				}
			}
			else {
				if (index >= displ[rank]) {
					//printf("buf %s and rank %d\n", buf, rank);
					char *string = strtok(buf, " ");
					while (string != NULL) {
						//list_append(&mylist, string);	
						if (isWord(string)) {
							if (!(list_contains(&mylist, string))) {
								list_append(&mylist, string);	
								//printf("z: %s\n", string);
							}
						}
						//printf("z: %s\n", string);
						string = strtok(NULL, " ");
					}	
				}
			}
		}
		// author	
		else if (count % 5 == 2) {

		}
		// abstract
		else if (count % 5 == 3) {
			if (rank != worldSize-1) {
			//if (rank <= worldSize-1) {
				if (index >= displ[rank] && index < displ[rank+1]) {
					//printf("buf %s and rank %d\n", buf, rank);
					char *string = strtok(buf, " ");
					while (string != NULL) {
					//	list_append(&mylist, string);	
						if (isWord(string)) {
							if (!(list_contains(&mylist, string))) {
								list_append(&mylist, string);	
								//printf("z: %s\n", string);
							}
						}
						//printf("z: %s\n", string);
						string = strtok(NULL, " ");
					}	
				}
			}
			else {
				if (index >= displ[rank]) {
					//printf("buf %s and rank %d\n", buf, rank);
					char *string = strtok(buf, " ");
					while (string != NULL) {
						//list_append(&mylist, string);	
						if (isWord(string)) {	
							if (!(list_contains(&mylist, string))) {
								list_append(&mylist, string);	
								//printf("z: %s\n", string);
							}
						}
						//printf("z: %s\n", string);
						string = strtok(NULL, " ");
					}	
				}
			}
		}
		// +++
		else {
			index++;
		}
		count++;
	}


	//int t;
	//t = list_iterator_start(&mylist);
	list_iterator_start(&mylist);

	void *data;
	//unsigned long long sum = 0;
	int sum = 0;
	while ((data = list_iterator_next(&mylist))) {
		//((char*)data)[strlen(data)] = '#';
		//printf("%s\n", strlen(((char*)data)));
		//printf("%s = %d\n", ((char *)data), strlen(data));
		
		//printf("LENGTH: %d\n", strlen(data));
		sum += 1 + strlen(data);
	}
	sum++;
	//t = list_iterator_stop(&mylist);
	list_iterator_stop(&mylist);
	//printf("RANK: %d and sum: %llu\n", rank, sum);
	//printf("RANK: %d and sum: %d\n", rank, sum);

	//t = list_iterator_start(&mylist);
	list_iterator_start(&mylist);

	char *words = malloc(sum);
	memset(words, '\0', sum);
	while ((data = list_iterator_next(&mylist))) {
		//((char*)data)[strlen(data)] = '#';
		//printf("%s\n", strlen(((char*)data)));
		//printf("%s = %d\n", ((char *)data), strlen(data));
		
		//printf("LENGTH: %d\n", strlen(data));
		//
		strcat(words, data);		
		strcat(words, "#");		
		//words[strlen(data)] = '#';
	}
	//t = list_iterator_stop(&mylist);
	list_iterator_stop(&mylist);
	list_destroy(&mylist);


	printf("%s\nrank %d\n", words, rank);
	//
	//printf("%d\n", strlen(words));

	//unsigned long long *sizes = NULL;
	int *sizes = NULL;
	if (rank == 0) {
		//sizes = malloc(sizeof(unsigned long long)*worldSize);
		sizes = malloc(sizeof(int)*worldSize);
	}
	
	MPI_Gather(
		&sum,
		1,
		MPI_INT, //MPI_UNSIGNED_LONG_LONG,
		sizes,
		1,
		MPI_INT, //MPI_UNSIGNED_LONG_LONG,
		0,
		world
	);

	char *combinedWords = NULL;
	//unsigned long long *displChar = NULL;
	int *displChar = NULL;
	int total = 0;
	if (rank == 0) {
		//unsigned long long total = 0;
		//displChar = malloc(sizeof(unsigned long long)*worldSize);
		displChar = malloc(sizeof(int)*worldSize);
		displChar[0] = 0;
		for (int i = 0; i < worldSize; i++) {
			//printf("%llu ", sizes[i]);
			//printf("sizes: %d ", sizes[i]);
			total += sizes[i];
			if (i > 0) {
				displChar[i] = displChar[i-1] + sizes[i];
				printf("%d ", displChar[i]);
			}
		}
		combinedWords = malloc(total);
		memset(combinedWords, '\0', total);
	}

	printf("total: %d and sum %d and rank %d\n", total, sum, rank);
	//MPI_Barrier(world);
	MPI_Gatherv(
		words,
		109,
		MPI_CHAR,
		combinedWords,
		sizes,
		displChar,
		MPI_CHAR,
		0,
		world		
	);

	if (rank == 0) printf("COMBINED WORDS: %s and size %d\n", combinedWords, strlen(combinedWords));

	list_init(&mylist);

	list_attributes_copy(&mylist, list_meter_string, 1);
	list_attributes_comparator(&mylist, list_comparator_string);

	if (rank == 0) {
		char *string = strtok(combinedWords, "#");
		while (string != NULL) {
			if (!(list_contains(&mylist, string))) {
				list_append(&mylist, string);	
				//printf("z: %s\n", string);
			}
			//printf("z: %s\n", string);
			string = strtok(NULL, "#");
		}

		list_iterator_start(&mylist);

		while ((data = list_iterator_next(&mylist))) {
			//((char*)data)[strlen(data)] = '#';
			//printf("%s\n", strlen(((char*)data)));
			//printf("%s = %d\n", ((char *)data), strlen(data));
			//
			printf("%s ", ((char *)data));
			
			//printf("LENGTH: %d\n", strlen(data));
		}
		list_iterator_stop(&mylist);
		
		unsigned int combSize = list_size(&mylist);
		printf("\nSIZE: %u\n", combSize);
	}

	if (rank == 0) {
		free(sizes);
		free(displChar);
		free(combinedWords);
	}

	free(words);



	/*
	struct tree *test = NULL;
	
	int i;
	for (i = 0; i < argc; ++i) {
		char buf[10];
		sprintf(buf, "%d", i);
		test = tree_insert(test, argv[i], strdup(buf));
	}
	tree_print(test, 2);
	*/




	//list_iterator_stop(&mylist);
	list_destroy(&mylist);

//	free(id);
	free(title);
//	free(author);
	free(abstract);
	free(sendcounts);
	free(displ);
	fclose(fp);
  	MPI_Finalize();
  	return 0;
}
