#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define ITERATIONS 1000

int a;

void *thread_fn(void *arg)
{
  int i;
  for(i = 0; i < ITERATIONS; i++)
    a++; 

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
