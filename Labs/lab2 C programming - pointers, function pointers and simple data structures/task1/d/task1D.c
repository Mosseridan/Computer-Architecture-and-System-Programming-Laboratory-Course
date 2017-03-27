#include <stdio.h>

int main(){
	int iarray[] = {1,2,3};
	char carray[] = {'a','b','c'};
	int* iarrayPtr = iarray;
	char* carrayPtr = carray;
	int* p;
	int i;

	for (i =0; i<3; i++){
		printf("the value of iarray[%d] is: %d\n",i ,*(iarrayPtr+i));
	}
	for (i =0; i<3; i++){
		printf("the address of carray[%d] is: %c\n",i ,*(carrayPtr+i));
	}

	printf("p address: %p\n",p);
 	printf("p address: %d\n",*p);
 	
	return 0;
} 
