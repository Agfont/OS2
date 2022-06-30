#include <stdio.h>
#include <unistd.h>

#define NUM_RECTS 1E09

double integral(int id)
{
  int i;
  double mid, height, width, sum = 0.0;
  double area;

  width = 1.0 / (double) NUM_RECTS;
  for(i = id; i < NUM_RECTS; i += 2) {
    mid = (i + 0.5) * width;
    height = 4.0 / (1.0 + mid * mid);
    sum += height;
  }
  area  = width * sum;
  return area;
}

int main()
{
  int fd[2];
  double res, res_fill;

  pipe(fd);

  if (fork() == 0) {
    res = integral(1);
    write(fd[1], &res, sizeof(double));
  } else {
    res = integral(0);
    read(fd[0], &res_fill, sizeof(double));
    printf("pi = %e\n", res + res_fill);
  }

  return 0;
}


