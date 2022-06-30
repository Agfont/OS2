#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>

#define ITERATIONS 10000

int main(int argc, char *argv[])
{
  int ret, i, *a;                 

  a = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
      MAP_SHARED | MAP_ANONYMOUS, -1, 0);

  *a = 0;

  ret = fork();

  if (ret == 0) {  // fill
    for(i = 0; i < ITERATIONS; i++)
       (*a)++;
    exit(1);
  } else { // pare
    for(i = 0; i < ITERATIONS; i++)
       (*a)++; 
  }
  
  wait(NULL);
  
  printf("a = %d\n", *a);

  munmap(a, sizeof(int));
}
