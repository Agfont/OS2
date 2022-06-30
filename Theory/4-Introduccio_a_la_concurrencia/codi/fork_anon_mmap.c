#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>

void imprimir(char *str, int *b)
{
  int i;
  printf("%s a = ", str);
  for(i = 0; i < 10; i++)
    printf("%d ", b[i]);
  printf("\n");
}

int main(int argc, char *argv[])
{
  int ret, i, *a;                 

  a = mmap(NULL, sizeof(int) * 10, PROT_READ | PROT_WRITE,
      MAP_SHARED | MAP_ANONYMOUS, -1, 0);

  for(i = 0; i < 10; i++)
    a[i] = 1;   

  ret = fork();

  if (ret == 0) {  // fill
    a[3] = 2;
    imprimir("Fill", a);
  } else { // pare
    sleep(1);
    imprimir("Pare", a);
  }
  
  munmap(a, sizeof(int) * 10);
}
