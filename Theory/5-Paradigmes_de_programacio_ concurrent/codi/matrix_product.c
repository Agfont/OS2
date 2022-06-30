#include <stdlib.h>
#include <stdio.h> 
#include <pthread.h>

// size of matrix
#define SIZE 2000

// number of threads
#define NUM_THREADS 4

int A[SIZE][SIZE];
int B[SIZE][SIZE];
int C[SIZE][SIZE];

void *product_multithread(void* arg)
{
  unsigned long int thread_id = (unsigned long int) arg;
  int size_segment, col_init, col_end;
  int row, col, k;

  size_segment = SIZE / NUM_THREADS;
  col_init = size_segment * thread_id;
  col_end  = size_segment * (thread_id + 1); 

  printf("%d %d %d\n", thread_id, col_init, col_end); 

  for (col = col_init; col < col_end; col++)
    for(row = 0; row < SIZE; row++)
      for(k = 0; k < SIZE; k++)
        C[row][col] += A[row][k] * B[col][k]; 
}

// Driver Code
int main()
{	
  pthread_t threads[NUM_THREADS];
  int row, col;
  unsigned long int i;

  /* Initialization */	
  for (row = 0; row < SIZE; row++) {
    for (col = 0; col < SIZE; col++) {
      A[row][col] = rand() % 10;
      B[row][col] = rand() % 10;
      C[row][col] = 0;
    }
  }

  /* Creating four threads, each evaluating its own part */
  for (i = 0; i < NUM_THREADS; i++) {
    pthread_create(&threads[i], NULL, product_multithread, (void *)i);
  }

  /* Joining and waiting for all threads to complete */
  for (i = 0; i < NUM_THREADS; i++)
    pthread_join(threads[i], NULL);	

  return 0;
}
