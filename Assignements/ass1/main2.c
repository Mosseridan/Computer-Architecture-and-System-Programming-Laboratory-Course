#include <stdio.h>
extern calc_func(long long *x, int numOfRounds);

/* returns true if x and y are equal, and false otherwise. */
int compare (long long * x, long long * y){
  return((*x)==(*y));
}

int main(int argc, char** argv){
  unsigned long long x = 0;
  unsigned int numOfRounds = 0;
  fflush(stdout);
  scanf("%llX", &x);
  scanf("%u", &numOfRounds);

  calc_func(&x, numOfRounds);

  return 0;
}
