#include <stdio.h>
#include <stdlib.h>

int plus_one(int n) {
  return n+1;
}

/* Gets an integer n, and returns the absolute value of n. */
int abs(int n){
	return abs(n);
}

/* Prints the value of n followed by a new line, and returns n unchanged. */
int iprt(int n){
	printf("%d\n", n);
	return n;
}

/* Prints the character of ASCII value n followed by a new line, and returns n unchanged. 
   If n is not between 0x20 and 0x7E, print the dot ('.') character instead. */
int cprt(int n){
	char c = '.';
	if (n>= 32 && n<=126)
		c = n;
	printf("%c\n",c);
	return n;
}

/* Ignores n, reads a line from stdin, and returns a number given in that line.  */
int my_get(int n){
	fflush(stdin);
	scanf("%d",&n);
	return n;
}

/* Gets an integer n, and ends the program using n as the return value */
int quit(int n){
	exit(n);
}
 
int* map(int *array, int arrayLength, int (*f) (int)){
  int* mappedArray = (int*)(malloc(arrayLength*sizeof(int)));
  int i;
  for (i=0; i<arrayLength; i++){
  	mappedArray[i] = f(array[i]);
  }
  return mappedArray;
}
 
int main(int argc, char **argv){
	int len = 4;
	int i;
	int arr1[] = {5, -2, 7, 8};
	int* arr2 = map(arr1, len, plus_one);
	
	/*test part a*/
	for(i=0 ; i<len ; i++)
 	 	printf("%d,\n", arr2[i]); /* 6, -1, 8, 9, */
	free(arr2);
	
	/*test part b*/
	arr2 = map(arr1, len, my_get);
	int* arr3 = map(arr2, len, abs);
	int* arr4 = map(arr3, len, iprt);
	int* arr5 = map(arr4, len, plus_one);
	int* arr6 = map(arr5, len, cprt);
	free(arr2);
	free(arr3);
	free(arr4);
	free(arr5);
	free(arr6);

	/*test part c - will not print "part c faild" if correct"*/
	quit(0);
	printf("part c faild!");

	return 0;
}
