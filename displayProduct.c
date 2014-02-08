/**
 * displayProduct.c
 * Natasha Squires
 * CS352 Assignment 1
 * Reads from shared memory and displays matrix product results
 **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main(int argc, char *argv[]){
  //shared memory junk
  int segment_id;
  int *matrix_results;
  const int size = 4086;

  key_t key = ftok("/keyFile", 'R');
  
  segment_id = shmget(key, size, 0666|IPC_CREAT);
  if(segment_id==-1){
    perror("shmget\n");
    exit(1);
  }  

  matrix_results = (int *)shmat(segment_id, NULL, 0);
  if(matrix_results==(int *)(-1)){
    perror("shmat\n");
    exit(1);
  }

  int pos=0;
  int length = matrix_results[pos]; //first element represents useful data
  pos++;
  int rowsC = matrix_results[pos]; pos++;
  int columnsC = matrix_results[pos]; pos++;

  //these are the results
  printf("RESULTS: \n");
  int m, n;
  for(m=pos; m<rowsC*columnsC+pos; m=m+columnsC){
    for(n=m; n<m+columnsC; n++){
      printf("%d ", matrix_results[n]);
    }
    printf("\n");
  }

  //detach from shared memory
  shmdt(matrix_results);

  //free up shared memory segment
  shmctl(segment_id, IPC_RMID, NULL);

  return 0;  
}
