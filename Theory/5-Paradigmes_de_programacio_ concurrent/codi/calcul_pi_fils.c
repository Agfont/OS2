#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_FILS  2 
#define NUM_RECTS 1E09

double result[NUM_FILS];

double integral(int id)
{
  int i;
  double mid, height, width, sum = 0.0;
  double area;

  width = 1.0 / (double) NUM_RECTS;
  for(i = id; i < NUM_RECTS; i += NUM_FILS) {
    mid = (i + 0.5) * width;
    height = 4.0 / (1.0 + mid * mid);
    sum += height;
  }
  area  = width * sum;
  return area;
}


void *thread_fn(void *arg)
{
  int i = (long int) arg;
  printf("Soc el fil numero %d\n", i);
  result[i] = integral(i);

  return ((void *)0);
}

int main(void)
{
  pthread_t ntid[NUM_FILS];

  double valor_pi;
  long int i;

  for(i = 0; i < NUM_FILS; i++)
     pthread_create(&(ntid[i]), NULL, thread_fn, (void *) i);
    
  for(i = 0; i < NUM_FILS; i++)
     pthread_join(ntid[i], NULL);

  valor_pi = 0.0;
  for(i = 0; i < NUM_FILS; i++)
    valor_pi += result[i];

  printf("Valor de pi: %e\n", valor_pi);
}
