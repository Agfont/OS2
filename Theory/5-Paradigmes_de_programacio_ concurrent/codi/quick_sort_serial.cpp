// Algorisme QuickSort. 

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

using namespace std;

#define SIZE 10000000

int *A;

int Partition(int p, int r)
{
  int mid = (p+r)/2;
  int x = A[mid];
  int t;
  
  t = A[p]; A[p] = A[mid]; A[mid] = t;
  
  int k = p;
  int l = r+1;
  
  do k++; while ((A[k] <= x) && (k < r));
  do l--; while (A[l] > x);

  while (k < l) {
    t = A[k]; A[k] = A[l]; A[l] = t;
    do k++; while (A[k] <= x);
    do l--; while (A[l] > x);
  }
  t = A[p]; A[p] = A[l]; A[l] = t;
  return l;
}

void QuickSort(int p, int r)
{
  if (p < r) {
    int q = Partition(p, r);
    QuickSort(p,q-1);
    QuickSort(q+1,r);
  }
}

int main(void)
{
  struct timeval start, end;
  long mtime, seconds, useconds;

  // Random seed
  srand(1); 

  A = new int[SIZE];
  for(int i = 0; i < SIZE; i++)  
    A[i] = rand() % 10000; 

  gettimeofday(&start, NULL); 
  QuickSort(0, SIZE-1);
  gettimeofday(&end, NULL);

  seconds = end.tv_sec - start.tv_sec;
  useconds = end.tv_usec - start.tv_usec;

  mtime = ((seconds) * 1000 + useconds / 1000.0);
  cout << "Elapsed time: " << mtime << " miliseconds" << endl;

  for(int i = 0; i < SIZE-1; i++)
    if (A[i] > A[i+1])
      cout << "Not sorted:" << i << " and " << i+1 << endl;

  delete [] A;

  return 0;
}


