/**
 * readMatrices.c
 * Natasha Squires
 * CS352 Assignment 1
 * Reads in matrix values from user input and sends to shared memory
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

void constructMatrices();
void sharedMemory(int matrices[], int size);

int rowsA, columnsA, rowsB, columnsB;

int main (int argc, char *argv[]){
  //dimensions for matrix A
  printf("Enter dimensions of matrix A: \n");
  printf("Rows: ");
  scanf("%d", &rowsA);
  printf("Columns: ");
  scanf("%d", &columnsA);
  
  if(rowsA > 10 || columnsA > 10){ //for simplicity's sake
    printf("Error: rows and columns must be less than 10\n");
    exit(1);
  }

  //dimensions for matrix B
  printf("Enter dimensions of matrix B: \n");
  printf("Rows: ");
  scanf("%d", &rowsB);
  printf("Columns: ");
  scanf("%d", &columnsB);

  if(rowsB > 10 || columnsB > 10){
    printf("Error: rows and columns must be less than 10\n");
    exit(1);
  }

  if(rowsB!=columnsA){ //error. Cannot multiply matrices
    printf("Error. no. of rows in B should equal no. of columns in A\n");
  }else{
    constructMatrices();
  }
  
  return 0;
}

/**
 * @param
 * @return void
 * constructs and formats an int array with the matrix information
 **/
void constructMatrices(){
  int max = (rowsA*columnsA)+(rowsB*columnsB)+4;
  int m,k,n,i=max-1;
  int matrices[max+1];
  matrices[0]=max; //indicates amount of useful values in memory

  printf("Enter values of matrix A:\n");
  //The first two entries indicate the dimensions of matrix A
  matrices[max-i] = rowsA; i--;
  matrices[max-i] = columnsA; i--;

  //feed in values of matrix A
  for(m=0; m<rowsA; m++){
    printf("row %d: \n", m+1);
    for(k=0; k<columnsA; k++){
      int value;
      printf("column %d: ", k+1);
      scanf("%d", &value);
      matrices[max-i] = value;
      i--;
    } 
  }

  printf("Enter values of matrix B:\n");
  matrices[max-i] = rowsB; i--;
  matrices[max-i] = columnsB; i--;

  //feed in values of matrix B
  for(k=0; k<rowsB; k++){
    printf("row %d: \n", k+1);
    for(n=0; n<columnsB; n++){
      int value;
      printf("column %d: ", n+1);
      scanf("%d", &value);
      matrices[max-i] = value;
      i--;
    }
  }

  //fill in matrices array
  int rA, cA, rB, cB, pos=1;
  rA = matrices[pos]; pos++;
  cA = matrices[pos]; pos++;
  
  int j;
  printf("matrix A:\n");
  for(i=3; i<rA*cA+3; i=i+cA){
    for(j=i; j<i+cA; j++)
      printf("%d ", matrices[j]);
    printf("\n");
  }

  rB = matrices[i]; i++;
  cB = matrices[i]; i++;

  int l;
  printf("matrix B:\n");
  for(j=i; j<rB*cB+i; j=j+cB){
    for(l=j; l<j+cB; l++)
      printf("%d ", matrices[l]);
    printf("\n");
  }

  sharedMemory(matrices, max+1);
}

/**
 * @param int matrices[], int size
 * @return void
 * Writes matrix data to shared memory
 **/
void sharedMemory(int matrices[], int size){
  int segment_id;
  int *shared_memory; //data that will be sent to shared memory
  const int SIZE = 4086; //allocating lots of memory
  
  key_t key = ftok("/keyFile", 'R');
  
  //shared memory stuff
  segment_id = shmget(key, SIZE, 0666|IPC_CREAT);
  if(segment_id==-1){
    perror("shmget\n");
    exit(1);
  }

  shared_memory = (int *)shmat(segment_id, NULL, 0);
  if(shared_memory==(int *)(-1)){
    perror("shmat\n");
    exit(1);
  }

  //shared memory gets the user entered matrix values
  int i;
  for(i=0; i<size; i++){
    shared_memory[i]=matrices[i];
  }

  //detach from memory
  shmdt(shared_memory);
}
