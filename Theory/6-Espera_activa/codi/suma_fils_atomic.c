#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* Codi que nomes compila amb gcc */

#define ITERATIONS 10000000

int a;

void *thread_fn(void *arg)
{
  int i;
  for(i = 0; i < ITERATIONS; i++)
    __sync_fetch_and_add(&a, 1); 

  return ((void *)0);
}

int main(void)
{
  pthread_t ntid[2];
  int i;

  a = 0;

  for(i = 0; i < 2; i++)
     pthread_create(&(ntid[i]), NULL, thread_fn, NULL);

  for(i = 0; i < 2; i++)
     pthread_join(ntid[i], NULL);

  printf("Valor d'a: %d\n", a);
}
