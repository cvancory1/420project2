
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
  FILE *fp= NULL;
  // fp = fopen("testCitations.txt", "r");
  // int TOTALPAPERS = 50; // number of rows in the AdjacencyList

  //fp = fopen("testCitations2.txt", "r");
  //int TOTALPAPERS = 295;  // number of rows in the AdjacencyList

   int TOTALPAPERS = 1354753;
   if(rank ==ROOT) {
    fp = fopen("arxiv-citations.txt", "r");

    if (fp == NULL){
       printf("ERROR opening file ");
      return 1;
     } // bugs bugs go away come again another day
  }
  // file read will count how many 1's are in every row of sparse matrix and query for ids to be scattered later
  int * Matrixlengths = NULL;
  int * MatrixIds = NULL;
  char *stmt = "select ind from Meta where id=";
  char *query = malloc(300);
  
  // set up for later... so each proc knows how many citations to read in
  int totalcitations = 0;

  if (rank == ROOT) {
    char *line = NULL; // buffer to read in from the file 
    size_t len; 
    int numread;

    int paperNumber = 0;; // current paper index being read in from file 
    int checkCitations = 0; // bool 
  
    MatrixIds = malloc( TOTALPAPERS* sizeof(int));
    Matrixlengths = malloc( TOTALPAPERS* sizeof(int));
    for(int i = 0; i< TOTALPAPERS ;i++) Matrixlengths[i] = 0;

    // while !EOF
    while ((numread = getline(&line, &len, fp)) != -1) {
      int length = strlen(line);
      // printf("i=%d line = %s\n", paperNumber , line);

      // reading in a paperid , query db to find its global index 
     if (checkCitations == 0 && line[0] != '-') {
        line[length - 1] = 0;  // removes the newline  for query
        sprintf(query, "%s \'%s\';", stmt, line);
        // printf("query=%s\n", query);
        rc = sqlite3_prepare_v2(db, query, -1, &res, 0);
        int step = sqlite3_step(res);
        MatrixIds[paperNumber] = (int) sqlite3_column_int(res, 0);
        // //  if (step == SQLITE_ROW) {
            // printf("%s\n", sqlite3_column_text(res, 0));
        // //   }

        // DELETE LATER 
        // MatrixIds[paperNumber] = 100; 

        memset(query, 0 , 300 ); 

        // puts("new paperid");
      }

      // reading the citations of the paper ( versus paperid)
      if (checkCitations == 1 && line[0] != '+') {
        //track number of citations this paperID has and count totalcitations in the .txt
        Matrixlengths[paperNumber]++;

        // citation setup for later, each proc will know how many to read in later 
        totalcitations++; 
       
        // puts("citation");

        // switch back - reading in a regular paper not citations
      }else  if (checkCitations == 1 && line[0] == '+') {
        // puts("no citations (+)");
        checkCitations = 0;
      }

      // paper has citations so check them on ... nxt line(s) is the list of cited papers
      if (line[length - 2] == '-'  && line[length - 1] == '\n') {
        checkCitations = 1;  // the next name you read in is a citation
        // puts("yes citations (-)");

      }
      // next line read in will be a paper to be read in
      if (line[0] == '+') {
        // puts("new paper");
        paperNumber++;
         
      }

      if(paperNumber % 100000 == 0){
        printf("papernumber =%d\n", paperNumber);
      }

      free(line);
      line = NULL;
      len = 0;
      // puts("");

    }
  }

    puts("====== end of file read ======");

  //DEBUG
  // for(int i = 0 ; i < TOTALPAPERS ; i ++){
    // if(rank == ROOT ) printf(" i%d len =%d id =%d \n",i, Matrixlengths[i] , MatrixIds[i] );
  // }

  // Calc send counts for all proc to recv rows of the sparse matrix. used in 2 scatters below
  SGData length_counts = getSGCounts(TOTALPAPERS, 1, worldSize);
  // everyonePrint(rank, "disls=", length_counts.displs);
  // everyonePrint(rank, "cnts=", length_counts.cnts);

  //  allocate to recv the number of 1's for each row ... will be used to  malloc later
  int *localLengths= malloc(sizeof(int) * length_counts.cnts[rank]);
  int *localIds= malloc(sizeof(int) * length_counts.cnts[rank]);

  // scattering the lengths
  MPI_Scatterv(Matrixlengths,             // sendbuf
               length_counts.cnts,        // sendcnts
               length_counts.displs,      // displacements
               MPI_INT,                   // datatype
               localLengths,              // recvbuf
               length_counts.cnts[rank],  // recvcnt
               MPI_INT, ROOT, world);

  // DEBUG 
  // if(rank == 2){
  //    for(int i =0 ; i < length_counts.cnts[rank] ; i++){
  //      printf("localLengths=%d\n", localLengths[i]);
  //    }
  //  }

  // // scattering the IDS
  MPI_Scatterv(MatrixIds,             // sendbuf
               length_counts.cnts,        // sendcnts
               length_counts.displs,      // displacements
               MPI_INT,                   // datatype
               localIds,              // recvbuf
               length_counts.cnts[rank],  // recvcnt
               MPI_INT, ROOT, world);
  // DEBUG 
  // if(rank == 0){
  //    for(int i =0 ; i < length_counts.cnts[rank] ; i++){
  //      printf("localID=%d\n", localIds[i]);
  //    }
  //  }


    if(rank ==0 ) puts("====== end of Scatter ======");

    // FREE 
    if(rank == ROOT ) free(Matrixlengths);
    if(rank ==ROOT )free(MatrixIds);

  // all proc create local sparse matric and populate with the list 
  AdjacencyList *locallistA = malloc(length_counts.cnts[rank] * sizeof(AdjacencyList));
  for (int i = 0; i < length_counts.cnts[rank]; i++) {
    locallistA[i].length = localLengths[i];

    // if papers were cited , allocate space
    if (locallistA[i].length > 0) {
      //  printf(" i=%d YES -localLengths=%d\n", i, localLengths[i]);

      locallistA[i].data = malloc(localLengths[i] * sizeof(int));
    } else {
      locallistA[i].data = NULL;
      //  printf(" i=%d NO localLengths=%d\n", i, localLengths[i]);

    }
    locallistA[i].globalID = localIds[i];
      //  printf("rank =%d MatrixIds -  i=%d MatrixIds=%d\n", rank,i, localIds[i]);

  }



  if(rank == ROOT) puts(" ===== ADJACENCY LIST  =======");
 // count how many total citations exist across the nodes have the node reread and store
 // the global ids into one giant array and then scatter that then 
//  every local list fills that array again ... same as above 



// reset file pointer
if(rank == ROOT )fseek( fp , 0 , SEEK_SET);

// root calc how many citations everyone will read in after the queries 
int * citation_counts = calloc(worldSize , sizeof(int));
int currentRank = 0; 

MPI_Bcast(&totalcitations, 1 , MPI_INT, ROOT, world);
// query based on the citation
int * citationIds = malloc(totalcitations * sizeof(int));

if (rank == ROOT) {

    char *line = NULL; // buffer to read in from the file 
    size_t len; 
    int numread;
    int paperNumber = 0;; // current paper index being read in from file 
    int checkCitations = 0; // bool 
    int j =0; 
    int count = 0; // used to track the amount of papers each node will get 

    // while !EOF
    while ((numread = getline(&line, &len, fp)) != -1) {
      int length = strlen(line);
      // printf("i=%d line = %s\n", paperNumber , line);

      // reading in a paperid , query db to find its global index 
     if (checkCitations == 0 && line[0] != '-') {
      // printf("i=%d line = %s\n", paperNumber , line);

        // puts("new paperid");
      }

      // reading the citations of the paper ( versus paperid)
      if (checkCitations == 1 && line[0] != '+') {
        // DEBUG 
        // puts("yes citation");

        //query the db to find where the 1's would be located 
        line[length - 1] = 0;  // removes the newline
        sprintf(query, "%s \'%s\';", stmt, line);
        rc = sqlite3_prepare_v2(db, query, -1, &res, 0);
        int step = sqlite3_step(res);

        // DEBUG         
        // printf("query=%s\n", query);
        if (step == SQLITE_ROW) {
          // printf("result %s\n", sqlite3_column_text(res, 0));
        }
        citationIds[j++] = (int)sqlite3_column_int(res, 0);
        // printf("citationIds[%d] %d\n",j-1,  citationIds[j-1]);


        // calc how many citations every node needs to read from the citationIDS list 
        citation_counts[currentRank]++;
        // printf("rank =%d papernumber=%d query =%s\n",currentRank,  paperNumber, query );


        // switch back - reading in a regular paper not citations
      }else  if (checkCitations == 1 && line[0] == '+') {
        // puts("no citations (+)");
        checkCitations = 0;
      }

      // paper has citations so check them on ... nxt line(s) is the list of cited papers
      if (line[length - 2] == '-'  && line[length - 1] == '\n') {
        checkCitations = 1;  // the next name you read in is a citation
        // puts("maybe citations (-)");

      }
      // next line read in will be a paper to be read in
      if (line[0] == '+') {
        // puts("new paper");
        paperNumber++;
        count ++; 
        // printf("rank=%d count=%d count=%d \n",currentRank, count, length_counts.cnts[currentRank]);
        if(count == length_counts.cnts[currentRank]){
          currentRank++;
          count = 0; 
        }
         
      }

      if(paperNumber % 100000 == 0){
        printf("papernumber =%d\n", paperNumber);
      }

      free(line);
      line = NULL;
      len = 0;
      //puts("");
    }
  }

  if(rank == ROOT) puts(" ===== end of file read 2 =======");


  MPI_Barrier(world);
  MPI_Bcast(citation_counts, worldSize , MPI_INT, ROOT, world);
  MPI_Bcast(citationIds, totalcitations , MPI_INT, ROOT, world);

  
  //calc displs
  int * displs = calloc(worldSize, sizeof(int));
  if(rank == ROOT){
    displs[0] = 0; 
    for(int i =1 ;i < worldSize; i++){
      displs[i] = displs[i-1] + citation_counts[i-1];
      // printf("disp=%d \n", displs[i]);
    }

  }

  MPI_Bcast(displs, worldSize , MPI_INT, ROOT, world);
  // printf("rank=%d disp=%d counts=%d \n",rank,  displs[rank], citation_counts[rank]);



  MPI_Barrier(world);
  //DEBUG
  // for(int i  = displs[rank], j=0 ; j<citation_counts[rank] ; i++ ,j++ ){
  //    printf("rank=%d citationIds[%d] %d\n", rank,i,  citationIds[i]);
  // }


  // everyone start at their offsets 
  int localcounter = displs[rank];
  // assign citationIDs to the localLists
   for (int i = 0; i < length_counts.cnts[rank]; i++) {
    // if papers were cited , allocate space
    if (locallistA[i].length > 0) {
      for(int j= 0 ; j < locallistA[i].length;j++ ){
        locallistA[i].data[j] = citationIds[localcounter++];

        //DEBUG 
        // printf("rank=%d data[%d]=%d\n",rank, j , locallistA[i].data[j]);
      }
    } 
  }





  Matrix X;
  X.rows = TOTALPAPERS;
  X.cols = 1;
  X.data = calloc(TOTALPAPERS, sizeof(double) * sizeof(Matrix));
  for(int i =0; i< TOTALPAPERS ;i++){
    X.data = 1; 
  }
  // sending matrix lengths into the power method
  double e = 10E-16;
  newpowermethod(locallistA, X, length_counts.cnts[rank], TOTALPAPERS, 5, e);

  puts("outside powermethod ");
  if (rank == ROOT) {
    printMatrix(X);
    puts("finish printing");  
  } 


  // TODO begin querying the database with the page rank 
  // char * test = malloc(300); 
  // sprintf(test, "INSERT INTO Meta (PageRank) VALUES (%f) where id = '%s';", 98.99, "plasm-ph/9607002" );
	// rc = sqlite3_exec(db, test, callback, 0, &err_msg);



  // FREE
  free(citationIds);
  free(citation_counts);
  free(X.data);
  for (int i = 0; i < length_counts.cnts[rank]; i++) {
    free(locallistA[i].data);
  }
  free(locallistA);

  free(displs);
  free(localLengths);
  free(localIds);
  free(length_counts.cnts);
  free(length_counts.displs);

  sqlite3_close(db);
  MPI_Finalize();

  return 0;
}




