#include <stdio.h>

int main(){
	int iarray[3];
	char carray[3];
	int i;

	for (i =0; i<3; i++){
		printf("the address of iarray[%d] is: %p\n",i ,&iarray[i]);
	}
	for (i =0; i<3; i++){
		printf("the address of carray[%d] is: %p\n",i ,&carray[i]);
	}

	printf("the hexadecimal value of iarray is: %p\n", iarray);
	printf("the hexadecimal value of iarray is: %p\n", iarray+1);
	printf("the hexadecimal value of iarray is: %p\n", carray);
	printf("the hexadecimal value of iarray is: %p\n", carray+1);

	return 0;
} 
