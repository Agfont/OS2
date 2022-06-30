#include <stdlib.h>
#include <stdio.h> 
#include <omp.h>

// size of matrix
#define SIZE 2000

// number of threads
#define NUM_THREADS 4

int A[SIZE][SIZE];
int B[SIZE][SIZE];
int C[SIZE][SIZE];

// Driver Code
int main()
{	
  pthread_t threads[NUM_THREADS];
  int row, col, k;
  unsigned long int i;

  /* Initialization */	
  for (row = 0; row < SIZE; row++) {
    for (col = 0; col < SIZE; col++) {
      A[row][col] = rand() % 10;
      B[row][col] = rand() % 10;
      C[row][col] = 0;
    }
  }
  
  /* Product using OpenMP */
#pragma omp parallel for num_threads(NUM_THREADS) private(col,row,k)
  for(col = 0; col < SIZE; col++)
    for(row = 0; row < SIZE; row++)
      for(k = 0; k < SIZE; k++)
        C[row][col] += A[row][k] * B[col][k]; 

  return 0;
}
