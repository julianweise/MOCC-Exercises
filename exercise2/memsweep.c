/*
**
** memsweep.c        Benchmark to measure memory access performance
**                   (measures time to access large heap array at
**                   varying locations)
**
** To compile:  cc -O -o memsweep memsweep.c -lm
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <float.h>

#define ARR_SIZE (4096 * 4096)

static double second(void)
{
    return ((double)((double)clock()/(double)CLOCKS_PER_SEC));
}

int main(void)
{
  unsigned int i;
  double time;
  volatile char* array;
  
  array = (char*) malloc(sizeof(char)*ARR_SIZE);
  
  time = second();
  for (i = 0; i < 10 * ARR_SIZE; i++)
  {
    array[(4096 * i + i) % ARR_SIZE] = 1 + array[i % ARR_SIZE];
  }
  free((void*)array);
  time = second() - time;
  printf("%05.3f\n", time);
  return 0;
}
