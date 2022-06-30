#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define NUM_FILS  2 

sem_t clau;

void *thread_fn(void *arg)
{
  long int i = (long int) arg;
  printf("Soc el fil numero %ld\n", i);
  sem_wait(&clau);
  printf("Soc el fil numero %ld dintre de la seccio critica\n", i);
  sleep(10);
  sem_post(&clau);
  printf("Soc el fil %ld fora de la seccio critica\n", i);

  return ((void *)0);
}

int main(void)
{
  pthread_t ntid[NUM_FILS];
  long int i;
  
  sem_init(&clau, 0, 1);

  for(i = 0; i < NUM_FILS; i++)
     pthread_create(&(ntid[i]), NULL, thread_fn, (void *) i);
    
  for(i = 0; i < NUM_FILS; i++)
     pthread_join(ntid[i], NULL);

  printf("Sortim de l'aplicacio\n");
}
