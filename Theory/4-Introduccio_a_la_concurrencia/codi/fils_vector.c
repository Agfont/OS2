#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

int a[10];

void imprimir(char *str, int *b)
{
  int i;
  printf("%s a = ", str);
  for(i = 0; i < 10; i++)
    printf("%d ", b[i]);
  printf("\n");
}

void *thread_function(void *arg)
{
  printf("Fil creat\n");
  a[3] = 2;
  imprimir("Nou vector al fil: ", a); 

  return ((void *)0);
}

int main(void)
{
  pthread_t ntid;
  int i;

  for(i = 0; i < 10; i++)
    a[i] = 1;
  
  imprimir("Vector original: ", a);

  /* Es crea un fil */
  pthread_create(&ntid, NULL, thread_function, NULL);

  /* Esperem que el fil finalitzi d'executar */
  pthread_join(ntid, NULL);
  
  imprimir("Vector final: ", a);
}
