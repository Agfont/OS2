#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_FILS  2 
#define NUM_RECTS 1E09

double valor_pi;
sem_t clau;

void integral(int id)
{
  int i;
  double mid, height, width;

  width = 1.0 / (double) NUM_RECTS;
  for(i = id; i < NUM_RECTS; i += NUM_FILS) {
    mid = (i + 0.5) * width;
    height = 4.0 / (1.0 + mid * mid);
    sem_wait(&clau); // lock
    valor_pi += height * width;
    sem_post(&clau); // unlock
  }
}


void *thread_fn(void *arg)
{
  int i = (long int) arg;
  printf("Soc el fil numero %d\n", i);
  integral(i);

  return ((void *)0);
}

int main(void)
{
  pthread_t ntid[NUM_FILS];
  long int i;
  
  valor_pi = 0.0;
  sem_init(&clau, 0, 1);

  for(i = 0; i < NUM_FILS; i++)
     pthread_create(&(ntid[i]), NULL, thread_fn, (void *) i);
    
  for(i = 0; i < NUM_FILS; i++)
     pthread_join(ntid[i], NULL);

  printf("Valor de pi: %f\n", valor_pi);
}
