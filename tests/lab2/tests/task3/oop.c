#include <stdio.h>
#include <stdlib.h>

/* Gets an integer n, and returns n+1 */
int plus_one(int n) {
  return n+1;
}

/* Gets an integer n, and returns the absolute value of n. */
int abs(int n){
  return abs(n);
}

/* Prints the value of n followed by a new line, and returns n unchanged */
int iprt(int n){
  printf("%d\n",n);
  return n;
}
/* Prints the character of ASCII value n followed by a new line, and returns n unchanged.
   If n is not between 0x20 and 0x7E, print the dot ('.') character instead. */
int cprt(int n){
  char c = '.';
  if(n > 32 && n <  126)
    c = n;
  printf("%c\n",c);
  return n;
}

/* Ignores n, reads a line from stdin, and returns a number given in that line.  */
int my_get(int n){
  fflush(stdin);
  scanf("%d", &n);
  return n;
}

/* Gets an integer n, and ends the program using n as the return value */
int quit(int n){
  exit(n);
}

int* map(int *array, int arrayLength, int (*f) (int)){
  int* mappedArray = (int*)(malloc(arrayLength*sizeof(int)));
  int i;
  for(i=0; i<arrayLength; i++){
    *(mappedArray+i) = f(*(array+i));
  }
  return mappedArray;
}

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
  int i,input;
  int iarray[4] = {2, -40, 1, -55};
  fun_desc fdarray[7] = {
    create_fun_desc("int plus_one(int n) - Gets an integer n, and returns n+1.",
                    plus_one),
    create_fun_desc("int abs(int n) -  Gets an integer n, and returns the absolute value of n.",
                    abs),
    create_fun_desc("int iprt(int n) -  Prints the value of n followed by a new line, and returns n unchanged.",
                    iprt),
    create_fun_desc("int cprt(int n) -  Prints the character of ASCII value n followed by a new line, and returns n unchanged."
                    "\n\t\t\tIf n is not between 0x20 and 0x7E, print the dot ('.') character instead.",
                     cprt),
    create_fun_desc("int my_get(int n) -  Ignores n, reads a line from stdin, and returns a number given in that line.",
                    my_get),
    create_fun_desc("int quit(int n) -  Gets an integer n, and ends the program using n as the return value.",
                    quit),
    create_fun_desc(NULL, NULL)
  };
    printf("Menu:\n");
    /* loop desplaying functon names */
    for (i = 0; fdarray[i].name != NULL; i++){
      printf("%d) %s\n", i, fdarray[i].name);
    }
    int len = sizeof(fdarray)/sizeof(fun_desc);
    while(true){
      printf("Please enter a number between 1 to %d corresponding with one of the menus functions to choose it:\n",len);
      fflush(stdin);
      scanf(%d,&input);
      if(input >= 0 && input < size-1){
        printf("within bounds\n");
        iarray = map(iarray, 4, fd[input].fun)
      }
      else{
        printf("not within bounds\n");
        fdarray[5].fun(0);
      }
    }
  }

  return 0;
}
