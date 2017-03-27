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

/* define struct fun_desc*/
typedef struct{
  char *name;
  int (*fun)(int);
}fun_desc;

/* Gets a printer to a char (string) and aprinter to a function,
   and returns a fun_desc with these ointers as its elements. */
fun_desc create_fun_desc(char* name, int (*fun)(int)){
  fun_desc fd;
  fd.name = name;
  fd.fun = fun;
  return fd;
}

int main(int argc, char **argv){
	int i, input;
	int len = 4;
	int* iarray = (int*)(malloc(len*sizeof(int)));
	fun_desc fdarray[] = {
		create_fun_desc("Plus One",plus_one),
		create_fun_desc("Abs",abs),
		create_fun_desc("Print Integer",iprt),
		create_fun_desc("Print Character",cprt),
		create_fun_desc("Get numbers",my_get),
		create_fun_desc("Quit",quit),
		create_fun_desc(NULL,NULL),
	};

	while (1){
		printf("Please choose a function:\n");
		for(i =0 ; fdarray[i].name != NULL ; i++){
			printf("%i) %s\n", i, fdarray[i].name);
		}
		printf("Option: ");
		
		fflush(stdin);
		scanf("%d",&input);
		if (input < 0){
			printf("not within bounds\n");
				quit(0);
		}
		for (i = 0; i<=input ;i++){
			if(fdarray[i].name == NULL){
				printf("not within bounds\n");
				quit(0);
			}
		}
		printf("within bounds\n");
		int *tmpPtr = iarray;
		iarray = map(iarray, len, fdarray[input].fun);
		free(tmpPtr);
		printf("DONE\n");
	}


	return 0;
}

