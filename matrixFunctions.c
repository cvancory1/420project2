
/*
  Chloe VanCory & Kalyn Howes 
  COSC 420 
  11/ 8/2021
  Lab 4 - page rank power method


*/ 
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

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

void initMatrix(Matrix *A, int rows, int cols) {
  A->rows = rows;
  A->cols = cols;
  A->data = malloc(A->rows * A->cols * sizeof(double));
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
      printf("%2f ", ACCESS(A, i, j));
    }
    printf("\n");
  }
}

void printMatrixD(Matrix A) {
  for (int i = 0; i < A.rows; i++) {
    for (int j = 0; j < A.cols; j++) {
      printf("%2f ", ACCESS(A, i, j));
    }
    printf("\n");
  }
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
int isConverged(Matrix prevVec, Matrix curretVec){
  double e = 10E-16;
  double temp;
  double sum =0; 
  for(int i =0; i< prevVec.rows; i++ ){
    temp = prevVec.data[i] - curretVec.data[i];
    temp = temp*temp;
    // printf("prevVec= %f cur=%f Difference=%f \n",  prevVec.data[i] ,curretVec.data[i], temp);

    sum += temp; 
  }
  if(sum < e){
    return 1; // is converged
  }else{
    return 0; // not converged
  }
  
}





Matrix matrixMultiplication(Matrix A, Matrix B) {
  Matrix C;
  C.rows = A.rows;
  C.cols = B.cols;
  C.data = malloc(A.rows*B.cols*sizeof(double));
  // if(rank =0)printMatrix(A);
  // if(rank ==0) printMatrix(B);
  
  // double sum = 0;
  // int count=0;
  // for (int i = 0; i < A.rows; i++) {
  //   for (int j = 0; j < B.rows; j++) {
  //     //printf("sum reset\n");
  //     for (int k = 0; k < A.rows; k++) {
  //       sum += ACCESS(A, i, j) * ACCESS(B, k, j);
  //       // printf("current sum: %f ACCESS(A, j, k)=%f ACCESS(B, k, j)=%f\n",ACCESS(A, j, k),  sum,ACCESS(B, k, j));
  //       printf(" sum: %f A=%f B)=%f\n", sum,ACCESS(A, i, j),  ACCESS(B, k, j));
  //     }
     
     
  //   }
  //     C.data[count++] = sum;
  //     printf("Sum=%f\n",sum);
  //     sum = 0;
  // }

  double sum = 0;
  int count=0;
  for (int i=0; i<A.rows ;i++) {
    for(int j=0; j<B.cols; j++){
      for (int k = 0; k < A.cols; k++) {
          sum += A.data[i*A.cols+k] *B.data[j*B.cols+k];
          // printf("rank=%d A=%d B=%d\n", rank,i, j);
          // printf("rank=%d A=%f B=%f\n", rank,A.data[i*A.cols+k],B.data[j*B.cols+k]);
      }
      
        // printf("\nsum=%f\n",sum);
        C.data[count++]= sum;
        sum=0;
    }
  }

  //puts("RESULT:");
  //printMatrix(C);
  return C;
}

// subtracts 2 matrices
Matrix matrixSubtraction(Matrix A, Matrix B) {
  Matrix d;
  d.rows = A.rows;
  d.cols = A.cols;
  d.data = malloc(d.rows* d.cols* sizeof(double));
  for (int i = 0; i < A.cols; i++) { 
    for (int j = 0; j < A.rows;j++)
    ACCESS(d,i,j) = ACCESS(A,i,j) - ACCESS(B,i,j);
  }
  return d;
}

double euclidean_norm(Matrix X){
  double norm = 0; 
  for(int i =0 ; i < X.rows * X.cols ;i++){
    // printf("data=%f\n",X.data[i]);
    norm+= (X.data[i] * X.data[i]);

  }
  // printf("R=%d norm=%f\n", rank, sqrt(norm));

  return sqrt(norm);
}

Matrix calcNorm( Matrix X){
  double norm = euclidean_norm(X);
  // printf("R=%d norm=%f\n", rank, norm);
  // printf(" norm=%f\n",norm);
  Matrix Xcopy;
  Xcopy.rows = X.rows;
  Xcopy.cols = X.cols;
  Xcopy.data =  malloc(X.rows * X.cols *sizeof(double));
  for(int i =0; i <X.rows * X.cols; i++ ){
    Xcopy.data[i] = X.data[i];
  }

  for (int i =0; i < X.rows * X.cols ; i++){
    Xcopy.data[i] = Xcopy.data[i] / norm;
    // if(rank==0) printf("R=%d X.data[i]=%f norm=%f\n", rank,X.data[i], norm);
  }


  return Xcopy; 

//  for (int i =0; i < X.rows * X.cols ; i++){
//     X.data[i] = X.data[i] / norm;
//     if(rank==0) printf("R=%d X.data[i]=%f norm=%f\n", rank,X.data[i], norm);
//   }
//   return X;

}


double calcEigen(Matrix X){
  return euclidean_norm(X);
}

double powerMethod(Matrix A, Matrix X, int originalRows,int originalCols , int iterationNum, double e ){
  // if(rank ==0) printMatrix(A);
  // if(rank ==1) printMatrix(A);

  SGData xSub_counts =  getSGCounts(originalRows, originalCols ,worldSize);
  for(int i =0; i< worldSize ;i++){
    xSub_counts.cnts[i] = xSub_counts.cnts[i] / originalRows;
    xSub_counts.displs[i] = xSub_counts.displs[i] / originalRows;
    
  }
  int count = 0; 
  double E = 0;
  while(count < iterationNum || E < e){
    Matrix Xsub = matrixMultiplication(A, X);
    // printf("rank=%d counts=%d displs=%d \n", rank, xSub_counts.cnts[rank],xSub_counts.displs[rank]);
    // MPI_Barrier(world);

    MPI_Allgatherv(
                  Xsub.data,  // sendbuf
                  xSub_counts.cnts[rank], //sendcount
                  MPI_DOUBLE,//sendtype
                  X.data,  //recvbuf
                  xSub_counts.cnts, //* recvcounts
                  xSub_counts.displs,// *displs
                  MPI_DOUBLE,world);
    // char *arrbuf = bufArr( X.data,  X.rows *  X.cols);
    // printf("Rank %d received %s\n", rank, arrbuf);

    Matrix diff = matrixSubtraction(Xsub, X);
    // find largest value in difference matrix
    double max = 0;
    for (int i = 0; i < diff.cols; i++) {
      for (int j = 0; j < diff.rows; j++) {
        if (ACCESS(diff, i, j) > 0)
          max = ACCESS(diff, i, j);
      }
    }

    double E = fabs(max);

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


  }



}
