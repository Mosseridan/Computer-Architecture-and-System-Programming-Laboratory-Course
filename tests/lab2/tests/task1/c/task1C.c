#include <stdio.h>

int main(){
  int iarray[3];
  char carray[3];
  int i;

  for(i=0; i<3; i++)
    printf("the address of iarray at[%d] is: %p\n",i,&iarray[i]);
  for(i=0; i<3; i++)
    printf("the address of carray at[%d] is: %p\n",i,&carray[i]);

  printf("the value (address) of iarray is: %p\n",iarray);
  printf("the value (address) of iarray+1 is: %p\n",iarray+1);
  printf("the value (address) of iarray is: %p\n",carray);
  printf("the value (address) of iarray+1 is: %p\n",carray+1);

  return 0;
}
