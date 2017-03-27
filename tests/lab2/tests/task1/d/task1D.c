#include <stdio.h>

int main(){
  int iarray[] = {1,2,3};
  char carray[] = {'a','b','c'};
  int* iarrayPtr;
  char* carrayPtr;
  int i;
  void* p
  iarrayPtr = iarray;
  carrayPtr = carray;

  for(i=0 ; i<3; i++)
    printf("the address of iarray at[%d] is: %d\n",i,*(iarrayPtr+i));
  for(i=0; i<3; i++)
    printf("the address of carray at[%d] is: %c\n",i,*(carrayPtr+i));

  return 0;
}
