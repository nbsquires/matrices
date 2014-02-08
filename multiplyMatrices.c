/**
 * multiplyMatrices.c
 * Natasha Squires
 * CS352 Assignment 1
 * Takes values from shared memory and computes matrix product
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>

void constructMatrices(int *matrices);
void pthreadSetup();
void *dotProduct(void *param);
void sharedMemory();

int max, sum, numThreads;
int rowsA, columnsA, rowsB, columnsB, rowsC, columnsC; //matrix attr
//matrices
int A[10][10];
int B[10][10];
int C[10][10];

struct v{
  int i;
  int j;
};

int main(int argc, char *argv[]){
  int segment_id;
  int *matrices;
  const int size = 4086;
  
  key_t key = ftok("/keyFile", 'R');

  segment_id = shmget(key, size, 0666|IPC_CREAT);
  if(segment_id==-1){
    perror("shmget\n");
    exit(1);
  }

  matrices = (int *)shmat(segment_id, NULL, 0);
  if(matrices==(int *)(-1)){
    perror("shmat\n");
    exit(1);
  }

  max = matrices[0]; //first value indicates no. of useful numbers in shared memory

  constructMatrices(matrices);

  //detach from segment
  shmdt(matrices);
  //free up shared memory segment
  shmctl(segment_id, IPC_RMID, NULL);

  //sends C to shared memory
  sharedMemory();

  return 0;
}

/**
 * @param int *matrices
 * @return void
 * constructs matrics A, B, and C
 **/
void constructMatrices(int *matrices){
  //int rowsA, columnsA, rowsB, columnsB, rowsC, columnsC;
  int pos=1;
  
  rowsA = matrices[pos]; pos++; //pos=2
  columnsA = matrices[pos]; pos++; //pos=3

  //Constructs matrix A from shared memory data
  int i, j;
  int rA=0, cA;
  for(i=pos; i<rowsA*columnsA+pos; i=i+columnsA){
    cA=0;
    for(j=i; j<i+columnsA; j++){
      A[rA][cA]= matrices[j];
      cA++;
    }
    rA++;
  } 

  int n, l, rB=0, cB;
  rowsB = matrices[i]; i++;
  columnsB = matrices[i]; i++;

  //Constructs matrix B from shared memory data
  for(j=i; j<rowsB*columnsB+i; j=j+columnsB){
    cB=0;
    for(l=j; l<j+columnsB; l++){
      B[rB][cB] = matrices[l];
      cB++;
    }
    rB++;
  }

  rowsC = rowsA;
  columnsC = columnsB;

  pthreadSetup(); //on to our multithreading
}

/**
 * @param
 * @return void
 * Creates pthreads and calls the runner thread to compute dot product
 **/
void pthreadSetup(){
  pthread_t *pthreads; //here we have multiple possible pthreads
  numThreads = rowsA*columnsB;

  pthreads = (pthread_t *)malloc(numThreads*sizeof(pthread_t));

  int i,j, threadCount=0;
  //create the threads
  for(i=0; i<rowsA; i++){
    for(j=0; j<columnsB; j++){
      C[i][j]=0;

      struct v*data = (struct v*)malloc(sizeof(struct v));
      data->i=i;
      data->j=j;
 
      //pthread stuff
      pthread_t pthread;
      pthread_attr_t attr;
      pthread_attr_init(&attr);

      if(pthread_create(&pthreads[threadCount], &attr, dotProduct, data)!=0){
        perror("pthread_create\n");
        free(pthreads);
        exit(1);
      }

      pthread_join(pthreads[threadCount], NULL);
      threadCount++;
      free(data); 
    }
  }
  free(pthreads); //make sure to clean up allocated memory
}

/**
 * @param void *param
 * @return void *
 * Runner thread that computes dot product of given data
 **/
void *dotProduct(void *param){
  struct v *data = (struct v *)param;
  int i = data->i;
  int j = data->j;
  sum=0;

  int k;
  for(k=0; k<columnsA; k++){
    int dP = A[i][k]*B[k][j];
    sum+=dP;
  }

  C[i][j] = sum;
  pthread_exit(0);
}

/**
 * @param
 * @return void
 * Sends matrix results off to shared memory
 **/
void sharedMemory(){
  int segment_id;
  int *results;
  int rsltlngth = rowsC*columnsC+3;
  const int size = 4086;

  key_t key = ftok("/keyFile", 'R');

  segment_id = shmget(key, size, 0666|IPC_CREAT);
  if(segment_id==-1){
    perror("shmget\n");
    exit(1);
  }

  results = (int *)shmat(segment_id, NULL, 0);
  if(results == (int *)(-1)){
    perror("shmat\n");
  }

  //write results to shared memory
  results[0] = rsltlngth; //number of useful ints in shared mem segment
  int m, n, pos=1;
  results[pos] = rowsC; pos++;  
  results[pos] = columnsC; pos++;

  for(m=0; m<rowsC; m++){
    for(n=0; n<columnsC; n++){
      results[pos] = C[m][n];
      pos++;
    }
  }

  //detach from shared memory
  shmdt(results);
}
