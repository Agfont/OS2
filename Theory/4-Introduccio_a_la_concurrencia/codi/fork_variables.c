#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void imprimir(char *str, int *b)
{
  int i;
  printf("%s a = ", str);
  for(i = 0; i < 10; i++)
    printf("%d ", b[i]);
  printf("\n");
}

int main(void)
{
  int ret, i, *a;

  a = malloc(10 * sizeof(int));

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

  free(a);
}
