
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

typedef struct rows {
  int globalID;
  int length;
  int *data;  // Holds the global index from db that this paper cites
} rows;

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
    printf("error opening db");
    sqlite3_close(db);

    return 1;
  }

  // opens the citation file
  FILE *fp;
  fp = fopen("testCitations.txt", "r");
  int TOTALPAPERS = 50; // number of rows in the adjacenylist

  // fp = fopen("testCitations2.txt", "r");
  // int TOTALPAPERS = 295;  // number of rows in the adjacenylist

  // int TOTALPAPERS = 1354753;
  // fp = fopen("arxiv-citations.txt", "r");

  if (fp == NULL) printf("ERROR opening file ");





  // allocate  sparse Matrix structure 
  AdjacenyList *listA = malloc(TOTALPAPERS * 2 * sizeof(int) * sizeof(int *));
  int * Matrixlengths = NULL; // will be used for scattering later
  int MALLOCEDSIZE = 10;
  
  if (rank == ROOT) {
    // assume a paper might have at least 10 papers cited( high estimate)
    // gets realloced later... 
    Matrixlengths = malloc(TOTALPAPERS * sizeof(int));
    
    //if we run out of memory 
    if (Matrixlengths == NULL) {
      printf("error malloc matrixlengths");
      return 1;
    }

    // initalize data 
    for (int i = 0; i < TOTALPAPERS; i++) {
      listA[i].data = malloc(MALLOCEDSIZE * sizeof(int));
      listA[i].length = 0; 
      listA[i].globalID = -1;
      Matrixlengths[i] = 0;
    }
  }
 



  // REWRITE OF THE FILE READ IN
  if (rank == ROOT) {

    char *line = NULL; // buffer to read in from the file 
    size_t len; 
    int numread;

    // int lineNumber = 0; // line number in the file 
    int totalCited = 0; // # of papers cited by the current paperID 
    // int j =0; // iterates thru data array( different lengths for every paper)
    int paperNumber = 0;; // current paper index being read in from file 
    int checkCitations = 0; // bool 
  
    // while !EOF
    while ((numread = getline(&line, &len, fp)) != -1) {
      int length = strlen(line);
      // printf("line =%d %s\n", paperNumber , line);

      // reading in a paperid , query db to find its global index 
     if (checkCitations == 0 && line[0] != '-') {
        // printf("listA_it =%d %s\n", listA_it , line);

        line[length - 1] = 0;  // removes the newline  for query
        char *stmt = "select ind from Meta where id=";
        char *query = malloc(200);
        sprintf(query, "%s \'%s\';", stmt, line);
        // printf("paperID query=%s\n", query); 

        rc = sqlite3_prepare_v2(db, query, -1, &res, 0);
        int step = sqlite3_step(res);
        // if (step == SQLITE_ROW) {
          // printf("%s\n", sqlite3_column_text(res, 0));
        // }
        // UNCOMMENT THIS ON REAL TEST CASE
         listA[paperNumber].globalID = (int)sqlite3_column_int(res, 0);

        // TEMPTEST
        // listA[listA_it].globalID = rand() % TOTALPAPERS;
        free(query);

      }

      // reading the citations of the paper ( versus paperid)
      if (checkCitations == 1 && line[0] != '+') {
        //track number of citations this paperID has
        listA[paperNumber].length++;
        Matrixlengths[paperNumber]++;

        // printf("listA_it =%d %s\n", paperNumber , line);
    
        line[length - 1] = 0;  // removes the newline for query

        // query db to find the globalindex for the citations
        char *stmt = "select ind from Meta where id=";
        char *query = malloc(200);
        sprintf(query, "%s \'%s\';", stmt, line);
        // printf("count =%d %s\n",paperNumber,  query);

        // query the db NEED CHAGE THIS AFTER WE IMPLEMENT A DIFFERENT STRUCTURE
        rc = sqlite3_prepare_v2(db, query, -1, &res, 0);
        int step = sqlite3_step(res);
        if (step == SQLITE_ROW) {
          // printf("query return %s\n", sqlite3_column_text(res, 0));
        }

        int returnedIndex = (int)sqlite3_column_int(res, 0);
        // printf(" paperNumber =%d query =%s\n", paperNumber, query);

        // counting number of citations it has as we read through the file 
        // so using that index assign the globalindex from the db 
        int dataIT = Matrixlengths[paperNumber]-1;
        // printf("dataIT %d\n",dataIT);

        ///TODO UNCOPY ERROR 
        // listA[paperNumber].data[dataIT] = returnedIndex;
        listA[paperNumber].data[dataIT] = paperNumber;

        free(query);


        // switch back - reading in a regular paper not citations
      }else  if (checkCitations == 1 && line[0] == '+') {
        checkCitations = 0;
      }

      // paper has citations so check them on ... nxt line(s) is the list of cited papers
      if (line[length - 2] == '-' ) {
        checkCitations = 1;  // the next name you read in is a citation
      }


      // next line read in will be a paper to be read in
      if (line[0] == '+') {
        //TODO check here for the length.. if 0 then realloc set to null 
        // if(  listA[paperNumber].length == 0 ){
        //   free( listA[paperNumber].data);
        //   listA[paperNumber].data = NULL;

        // }
        paperNumber++;
         
      }

      // if(paperNumber % 100000 == 0){
      //   printf("papernumber =%d\n", paperNumber);
      // }


      free(line);
      line = NULL;
      len = 0;

    }
  }




  //  DEBUG - printing results of the sparse matrix in listA 
  if (rank == ROOT) {
    for (int i = 0; i < TOTALPAPERS; i++) {
      // print global id and number of 1's in the list
      // printf("i=%d listA.length= %d globalID=%d \n", i, listA[i].length,listA[i].globalID  );

      if(listA[i].length>0){
        for (int j = 0; j < listA[i].length; j++) {
          // printf("  onesLocation=%d \n",  listA[i].data[j] );

        }
      }
    }
    // puts("");
  }







  // scatter the lengths so each proc knows how many of their localList to allocate 
  SGData length_counts = getSGCounts(TOTALPAPERS, 1, worldSize);
  // everyonePrint(rank, "disls=", length_counts.displs);
  // everyonePrint(rank, "cnts=", length_counts.cnts);

  // //  allocate to recv the number of 1's for each row ... will be used to  malloc later
  int *localLenghts = malloc(sizeof(int) * length_counts.cnts[rank]);

  // scattering the lengths
  MPI_Scatterv(Matrixlengths,             // sendbuf
               length_counts.cnts,        // sendcnts
               length_counts.displs,      // displacements
               MPI_INT,                   // datatype
               localLenghts,              // recvbuf
               length_counts.cnts[rank],  // recvcnt
               MPI_INT, ROOT, world);

  // error checking - recv buffer
  for (int i = 0; i < length_counts.cnts[rank]; i++) {
    // printf("rank =%d arr=%d i=%d \n", rank, localLenghts[i], i);
  }

  // create local sparse matric 
  AdjacenyList *locallistA = malloc(length_counts.cnts[rank] * sizeof(AdjacenyList));
  for (int i = 0; i < length_counts.cnts[rank]; i++) {
    locallistA[i].length = localLenghts[i];

    // if papers were cited , allocate space
    if (locallistA[i].length > 0) {
      locallistA[i].data = malloc(localLenghts[i] * sizeof(int));
    } else {
      locallistA[i].data = NULL;
    }
    locallistA[i].globalID = -1;
  }


  // updated version 

if (rank == 0) {
    int sendRoot = 1;
    int count = 0;

    // root assigns data to itself 
    int i; 
    for (i = 0 ; i < length_counts.cnts[ROOT]; i++) {
      for (int j = 0; j < listA[i].length; j++) {
        locallistA[i].data[j] = listA[i].data[j];
        // printf("list i=%d j=%d arr= %d \n", i, j , listA[i].data[j]);
      }
    }

    // DEBUG 
    for ( i = length_counts.cnts[ROOT]; i < TOTALPAPERS ; i++) {
      for (int j = 0; j < listA[i].length; j++) {
        // printf("list i=%d j=%d arr= %d \n", i, j , listA[i].data[j]);
      }
      // printf("rank =%d sendroot=%d  i=%d  count=%d listlength =%d lengths=%d
      // \n",rank, sendRoot, i, count,  listA[i].length, localLenghts[i] );

    // send the sparisfied  list except for what the root will receive which will be handled later 
      MPI_Send(listA[i].data,    // buf
               listA[i].length,  // amount sending - count
               MPI_INT,          // dtype
               sendRoot,         //  dest
               sendRoot,         // tag
               world             // comm
      );
      // DEBUG 
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


  // other proc recv 
  if(rank != 0){
    int number_amount;
    int i = 0;
    for (i = 0; i < length_counts.cnts[rank]; i++) {
      MPI_Status status;

       MPI_Recv(locallistA[i].data, localLenghts[i], MPI_INT, ROOT, MPI_ANY_TAG,
                world, &status);

      MPI_Get_count(&status, MPI_INT, &number_amount);
      // printf(
      //     "received %d numbers from 0. Message source = %d, "
      //     "tag = %d\n",
      //     number_amount, status.MPI_SOURCE, status.MPI_TAG);
    }

    // puts("");

  }




  //// DEBUG - error checking recv arrays for root , printing the original matrix
  if (rank == 0) {
    puts("error checking ");
    for (int i = 0; i < length_counts.cnts[rank]; i++) {
      if (locallistA[i].length > 0) {
        for (int j = 0; j < locallistA[i].length; j++) {
          // printf("Here rank =%d\n", rank);
           printf("rank =%d i = %d globalID= %d \n",rank,i,
           locallistA[i].data[0]);
          // printf(" rank = %d  localListIndex=%d totallength = %d  data= %d
          // \n", rank, i , locallistA[i].length, locallistA[i].data[j]);
        }
      } else {
        printf("rank =%d  i = %d  ID =NULL \n",rank, i);
      }
    }
  }

  
  /*
  // for testing- print actal matrix
  if (rank == 0) {
    for (int i = 0; i < TOTALPAPERS; i++) {
      // print a 0 row
      if (listA[i].length == 0) {
        for (int j = 0; j < TOTALPAPERS; j++) {
          // printf("0,");
        }
      } else if (listA[i].length > 0) {
        for (int j = 0; j < TOTALPAPERS; j++) {
          if (listA[j].data[j] == j) {
            // printf("%d,", listA[i].data[j]);

          } else {
            //  printf("0,");
          }
        }
      }

      // puts("");
    }
  }

  */
  Matrix X;
  X.rows = TOTALPAPERS;
  X.cols = 1;
  X.data = malloc(sizeof(double) * X.rows * X.cols);
  for (int i = 0; i < TOTALPAPERS; i++) {
    X.data[i] = 1;
  }

  // sending matrix lengths into the power method
  double e = 10E-16;
  newpowermethod(locallistA, X, length_counts.cnts[rank], TOTALPAPERS, 5, e);

  // puts("outside powermethod ");
  if (rank == ROOT) {
    for (int i =0 ; i < TOTALPAPERS ; i++){
    // printf("data=%f \n", pageRanks.data[i]);
    // printf("data=%f \n", X.data[i]);
    }
    // printf("data=%f \n", X.data[0]);
  }


  if(rank ==ROOT) {
    puts("Printing X matrix");
    printMatrix(X);
  }
  // query into db and assign each row to their pagerank
  // MPI_Barrier(world);
  // for (int i = 0; i < length_counts.cnts[rank]; i++) {
  //   char *stmt1 = "UPDATE Meta set PageRank = ";
  //   char *stmt2 = "where ind = ";
  //   char *query = malloc(400);

  //   // TODO SEG FAUTLS CURRENTLY
  //   sprintf(query, "%s%f %s%d;", stmt1, X.data[listA[i].globalID], stmt2,
  //           listA[i].globalID);
  //   printf("RANK=%d query=%s i=%d \n", rank, query, i);

  //   rc = sqlite3_prepare_v2(db, query, -1, &res, 0);
  //   int step = sqlite3_step(res);

  //   free(query);
  // }



  // free all variables
    for (int i = 0; i < length_counts.cnts[rank]; i++) {
      if(locallistA[i].data != NULL ) free( locallistA[i].data); 
  }

   if(rank == ROOT){
     for (int i = 0; i < TOTALPAPERS; i++) {
      free( listA[i].data); 
    }
  }
  free(X.data);
  free(Matrixlengths);
  free(length_counts.cnts);
  free(length_counts.displs);





  sqlite3_close(db);
  MPI_Finalize();

  return 0;
}






































































