#include <stdio.h>
extern calc_func(long long *x, int numOfRounds);

/* returns true if x and y are equal, and false otherwise. */
int compare (long long * x, long long * y){
  return((x-y) == 0);
}

int main(int argc, char** argv){
  unsigned long long x = 0;
  unsigned int numOfRounds = 0;
  fflush(stdout);
  printf("please enter one unsigned number(up to 64 bits):\n");
  ;scanf("%llX", &x);
  printf("Got number: %llX\n", x);
  printf("please enter one unsigned number (up to 32 bits):\n");
  scanf("%u", &numOfRounds);
  printf("Got number: %u\n", numOfRounds);

  calc_func(&x, numOfRounds);

  return 0;
}
