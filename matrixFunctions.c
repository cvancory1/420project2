
/*
  Chloe VanCory & Kalyn Howes
  COSC 420
  11/ 8/2021
  Lab 4 - page rank power method


*/
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define ROOT 0

#define INDEX(i, j, n, m) i *m + j
#define ACCESS(A, i, j) A.data[INDEX(i, j, A.rows, A.cols)]

// declare the MPI set ups in the global scope
MPI_Comm world;
int worldSize, rank;
char name[MPI_MAX_PROCESSOR_NAME];
int nameLen;

typedef struct Matrix {
  int rows;
  int cols;
  double *data;
} Matrix;


typedef struct AdjacencyList {
  int globalID;
  int length;
  int *data;  // Holds the global index from db that this paper cites
} AdjacencyList;


void everyonePrint(int rank, char *str, int *arr) {
  printf("Rank =%d %s=%d\n", rank, str, arr[rank]);
}

// // to error check
// char *arrbuf = bufArr(recvbufA, sendcts[rank]);
// printf("Rank %d received %s\n", rank, arrbuf);
// arrbuf = bufArr(recvbufB, sendcts[rank]);
// printf("\nRank %d received %s\n", rank, arrbuf);
char *bufArr(double *arr, int n) {
  char *buf = malloc(n * (10 + 1) + 1);
  buf[0] = '\0';

  const char *fmt = "%0.4f ";
  char tmp[10 + 1 + 1];  // same width as fmt plus a null term

  for (int i = 0; i < n; i++) {
    sprintf(tmp, fmt, arr[i]);
    strcat(buf, tmp);
  }

  return buf;
}

// void printARR(int* A) {
//   for (int i = 0; i < A.rows * A.cols; i++) {
//     printf("%4d ", A[i]);
//   }
//   printf("\n");
// }

void printMatrix(Matrix A) {
  for (int i = 0; i < A.rows; i++) {
    for (int j = 0; j < A.cols; j++) {
      printf("%2f", ACCESS(A, i, j));
    }
    printf("\n"); 
  }
}

void printMatrixandRank(Matrix A, int rank) {
  printf("RANK=%d\n", rank);
  for (int i = 0; i < A.rows; i++) {
    for (int j = 0; j < A.cols; j++) {
      printf("%2f", ACCESS(A, i, j));
    }
    printf("\n");
  }
}

void printarray(double * arr , int length){
  for (int i = 0; i <length; i++) {
    printf("%f\n", arr[i]);
  }
    // printf("\n");


}

/* To automate this garbage and store the results in a nice struct */
typedef struct SGData {
  int *cnts;
  int *displs;
} SGData;

SGData getSGCounts(int rows, int cols, int worldSize) {
  SGData temp;
  temp.cnts = malloc(worldSize * sizeof(int));
  temp.displs = malloc(worldSize * sizeof(int));
  // printf("rows=%d cols=%d worldSize=%d\n", rows,cols,worldSize);

  int minSend = rows / worldSize;
  // printf("minsend=%d \n")
  for (int i = 0; i < worldSize; i++) {
    temp.cnts[i] = minSend;
  }

  for (int i = 0; i < rows % worldSize; i++) {
    temp.cnts[i]++;
  }

  for (int i = 0; i < worldSize; i++) {
    temp.cnts[i] *= cols;
    // printf("rank=%d sendcount[%d]=%d\n",rank,i,temp.cnts[i]);
  }

  temp.displs[0] = 0;
  for (int i = 1; i < worldSize; i++) {
    temp.displs[i] = temp.displs[i - 1] + temp.cnts[i - 1];
  }
  return temp;
}

// tests to see whether the vector is converging to an eigenvalue
int isConverged(Matrix prevVec, Matrix curretVec) {
  double e = 10E-16;
  double temp;
  double sum = 0;
  for (int i = 0; i < prevVec.rows; i++) {
    temp = prevVec.data[i] - curretVec.data[i];
    temp = temp * temp;
    // printf("prevVec= %f cur=%f Difference=%f \n",  prevVec.data[i]
    // ,curretVec.data[i], temp);

    sum += temp;
  }
  if (sum < e) {
    return 1;  // is converged
  } else {
    return 0;  // not converged
  }
}

Matrix matrixMultiplication(Matrix A, Matrix B) {
  Matrix C;
  C.rows = A.rows;
  C.cols = B.cols;
  C.data = malloc(A.rows * B.cols * sizeof(double));

  double sum = 0;
  int count = 0;
  for (int i = 0; i < A.rows; i++) {
    for (int j = 0; j < B.cols; j++) {
      for (int k = 0; k < A.cols; k++) {
        sum += A.data[i * A.cols + k] * B.data[j * B.cols + k];
        // printf("rank=%d A=%d B=%d\n", rank,i, j);
        // printf("rank=%d A=%f B=%f\n", rank,A.data[i*A.cols+k],B.data[j*B.cols+k]);
      }

      // printf("\nsum=%f\n",sum);
      C.data[count++] = sum;
      sum = 0;
    }
  }

  // MPI_Barrier(world);
  // puts("RESULT:");
  // // printf("matrix multiplicatin rank=%d\n", rank);
  // printMatrix(C);
  // puts("========");

  return C;
}




// subtracts 2 matrices
Matrix matrixSubtraction(Matrix A, Matrix B) {
  Matrix d;
  d.rows = A.rows;
  d.cols = A.cols;
  // printf("matrid D row=%d col=%d ", A.rows, A.cols);
  d.data = malloc(d.rows * d.cols * sizeof(double));
  for (int i = 0; i < A.cols; i++) {
    for (int j = 0; j < A.rows; j++)
      ACCESS(d, i, j) = ACCESS(A, i, j) - ACCESS(B, i, j);
  }
  return d;
}



double euclidean_norm(Matrix X) {
  double norm = 0;
  for (int i = 0; i < X.rows * X.cols; i++) {
    // printf("data=%f\n",X.data[i]);
    norm += (X.data[i] * X.data[i]);
  }

  // printf("R=%d norm=%f\n", rank, sqrt(norm));

  return sqrt(norm);
}




Matrix calcNorm(Matrix X) {
  double norm = euclidean_norm(X);
  // printf("R=%d norm=%f\n", rank, norm);
  if (rank ==0 ) printf(" norm=%f\n",norm);
  for (int i = 0; i < X.rows * X.cols; i++) {
    X.data[i] = X.data[i] / norm;
    if(rank==0) printf("R=%d X.data[i]=%f norm=%f result =%f \n", rank, X.data[i] , norm,  X.data[i]/norm);
  }

  return X ; 

}

double calcEigen(Matrix X) { 
  return euclidean_norm(X); 
}


double powerMethod(Matrix A, Matrix X, int originalRows,int originalCols , int iterationNum, double epsilon ){
  // if(rank ==0) printMatrix(A);
  if(rank ==0) printMatrix(X);

  // puts("");
  // if(rank ==1) printMatrix(A);
  // printf("it=%d  e= %1.20f\n",iterationNum, epsilon);
  // printf("it=%d\n",iterationNum);

  // xsub - is a portion of the X vector each proc is in charge of 
  SGData xSub_counts =  getSGCounts(originalRows, originalCols ,worldSize);
  for(int i =0; i< worldSize ;i++){
    xSub_counts.cnts[i] = xSub_counts.cnts[i] / originalRows;
    xSub_counts.displs[i] = xSub_counts.displs[i] / originalRows;
    
  }
  // everyonePrint(rank, "disls=", xSub_counts.displs);
  // everyonePrint(rank, "cnts=", xSub_counts.cnts);


  int count = 0; 
  double E = 0;// tracks the amount of "change" each vector has gone through after every iteration .. used for convegerence
  while(count < iterationNum || E < epsilon){

  //   // every proc multiplies with their local array A and the entire X 
    // if(rank == 1){
    // printMatrix(A);
    // printMatrix(X);
    // }
    Matrix Xsub = matrixMultiplication(A, X);
    // printf("rank=%d xsub------\n", rank);
    // printMatrix(Xsub);
    // printf("rank=%d counts=%d displs=%d \n", rank, xSub_counts.cnts[rank],xSub_counts.displs[rank]);
    // MPI_Barrier(world);

    /* gather and every proc receives the updated X vector  */ 
    MPI_Allgatherv(
                  Xsub.data,  // sendbuf
                  xSub_counts.cnts[rank], //sendcount
                  MPI_DOUBLE,//sendtype
                  X.data,  //recvbuf
                  xSub_counts.cnts, //* recvcounts
                  xSub_counts.displs,// *displs
                  MPI_DOUBLE,world);
  //  char *arrbuf = bufArr( X.data,  X.rows *  X.cols);
  //  if(rank ==0 ) printf("count =%d received %s\n", count, arrbuf);

    // calc the difference to see if the threshold has been met ( ie no more change .... A converged,  stop the loop)
  Matrix diff = matrixSubtraction(Xsub, X);
  E = fabs(euclidean_norm(diff));

    //  if(rank ==0 )  printf("LOOP= %d THRESHOLD = %1.15f e=%1.15f \n",count,  E , epsilon  );


  MPI_Barrier(world);
    // if X has converged or we have reached 20 iterations then stop and return the eigen value 
    if(count != iterationNum-1){
      X = calcNorm(X);
      if(rank ==ROOT){
        printf("Count = %d Printing X:\n", count);
        printMatrix(X);
      }
    }else if (count == iterationNum-1){
        // printf("answer=%f ", calcEigen(X));
        return calcEigen(X);
    }

    count++;

    free(diff.data);
    free(Xsub.data);

  }
  
  free(xSub_counts.cnts);
  free(xSub_counts.displs);
}



/*
  look at the array of indicies and access the X vector at those 
  - store the sum maybe create a new array to return as the X vector
  - then normalize like usual

  // we are going to scatter the lists when we call
  // LISTA Is a local copy 
*/



// locallistSize = rows in Xsub 
void newpowermethod(AdjacencyList * listA, Matrix  X, int localListSize, int TOTALLISTSIZE, int iterationNum , double epsilon ){
  // printf("ENTER rank =%d localListSize=%d\n", rank, localListSize);
  // if(rank ==0 ) printf("rows=%d cols=%d\n", X.rows, X.cols);

  SGData xSub_counts =  getSGCounts(TOTALLISTSIZE, 1 ,worldSize);
  // everyonePrint(rank,"dipls=",xSub_counts.displs );
  // everyonePrint(rank,"cnts=",xSub_counts.cnts );

  // evey
  //proc calculates a portion of X 
  Matrix xsub;
  xsub.rows = localListSize;
  xsub.cols = 1;
  xsub.data = malloc( localListSize * sizeof(double)); 

  // set Matrix xsub to 0 , will be used for arithmetic later
  for(int i =0 ; i <localListSize; i++ ) xsub.data[i] =0; 
  

  int count = 0 ; // tracks the loop itertation
  while(count <= iterationNum){
  
    for(int i =0 ; i < localListSize ; i++){
      // look at the recipe of what x1 equals in the indexes 
      // if(listA[i].length == 0 ){
      //   xsub.data[i] = 0; 
      // }

      for(int j = 0 ; j < listA[i].length; j++){
        // printf("length=%d listA[%d].data=%d \n",listA[i].length , i , listA[i].data[j] );
        int xlocation = listA[i].data[j];
        //if(xlocation > X.rows){
         // printf("chloe seg fault will happen on this test case\n");
          //return ;
        //}
        xsub.data[i] += X.data[xlocation];
       // printf("rank =%d xlocation=%d adjIndex[%d] =%d X.data[xlocation]=%f xsub.data=%f\n",rank,  xlocation, i, listA[i].data[j],  X.data[xlocation], xsub.data[i]);
      }
      // printf("FINAL xsub.data[%d] =%f\n", i , xsub.data[i]);
    }

    Matrix diff = matrixSubtraction(xsub, X);
    //double E = fabs(euclidean_norm(diff));

    MPI_Barrier(world);
    // DEBUG 
    //for(int i =0 ; i< localListSize ; i++){
      // printf("rank=%d  xsub[%d] =%f\n\n",rank, i , xsub[i]);

    //}

    MPI_Allgatherv(
                    xsub.data,  // sendbuf
                    xSub_counts.cnts[rank], //sendcount
                    MPI_DOUBLE,//sendtype
                    X.data,  //recvbuf
                    xSub_counts.cnts, //* recvcounts
                    xSub_counts.displs,// *displs
                    MPI_DOUBLE,world);



    if(rank ==0 ){
      // char *arrbuf = bufArr( X.data,  X.rows *  X.cols);
      // printf("Rank %d received %s\n", rank, arrbuf);

    }

    if(count < iterationNum ){
        X = calcNorm(X);
        if(rank ==ROOT){
          // printf("Count = %d Printing X:\n", count);
          // puts("++++++++");
          // printMatrix(X);
          // puts("++++++++");

        }
    }else if(count == iterationNum || E < epsilon) {
        if(E < epsilon ){
          printf("========RETURN EARLY Count = %d Printing X: (========\n", count);
        }
      // puts(" ------- ");
      printf("Count = %d Printing X:\n", count);
      // printMatrix(X);
      // puts(" ------- ");
     
    }

    count++; 
    free(diff.data);
  }

  free(xsub.data);
  free(xSub_counts.cnts);
  free(xSub_counts.displs);
}





