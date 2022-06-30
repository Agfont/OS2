#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define NUM_VALORS 10
#define NUM_ITERS  20

sem_t buit, ocupat;

void *thread_prod(void *arg)
{
  int i, w, value, *valors;

  valors = (int *) arg;

  srand(time(NULL));

  /* write index */
  w = 0;

  for(i = 0; i < NUM_ITERS; i++) {
    value = rand();
    sem_wait(&buit);
    printf("Productor: valors[%d] = %d\n", w, value);
    valors[w] = value;
    w = (w + 1) % NUM_VALORS;
    sem_post(&ocupat);
  }

  return ((void *)0);
}

void *thread_cons(void *arg)
{
  int i, r, value, *valors; 

  valors = (int *) arg; 

  /* Read index */
  r = 0;

  for(i = 0; i < NUM_ITERS; i++) {
    sem_wait(&ocupat);
    value = valors[r];
    printf("Consumidor: valors[%d] = %d\n", r, value);
    r = (r + 1) % NUM_VALORS;
    sem_post(&buit);
  }

  return ((void *) 0);
}

int main(void)
{
  pthread_t prod, cons;
  int valors[NUM_VALORS];
  
  sem_init(&buit, 0, NUM_VALORS);
  sem_init(&ocupat, 0, 0);

  pthread_create(&prod, NULL, thread_cons, (void *) valors);
  pthread_create(&cons, NULL, thread_prod, (void *) valors);
    
  pthread_join(prod, NULL);
  pthread_join(cons, NULL);

  printf("Sortim de l'aplicacio\n");
}
