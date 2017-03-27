#include <stdio.h>
#include <stdlib.h>

int DEBUG = 0;
int BUFF_SIZE = 4001;
int FILE_NAME_SIZE = 101;
int INPUT_BUFF_SIZE = 101;
int UNIT_SIZE  = 1;
char* FILE_NAME = NULL;
char* BUFF = NULL;
char* INPUT_BUFF = NULL;
#define dprint if (DEBUG) fprintf

/* define struct fun_desc*/
typedef struct{
  char *name;
  void (*fun)();
}fun_desc;

/* Gets a printer to a char (string) and aprinter to a function,
   and returns a fun_desc with these ointers as its elements. */
fun_desc create_fun_desc(char* name, void (*fun)()){
  fun_desc fd;
  fd.name = name;
  fd.fun = fun;
  return fd;
}

/* get onput from user as a string */
char* get_input(){
  char* input = fgets(INPUT_BUFF,INPUT_BUFF_SIZE,stdin);
  if(input == NULL){
    perror("Error: read error");
  }
  return input;
}

/* get input from user as an int*/
int get_input_int(){
  char* input = get_input();
  if(input == NULL){
    return -1;
  }
  return atoi(input);
}

/* toogle DEBUG FLAG on and of */
void toggle_debug(){
  if(DEBUG){
    DEBUG = 0;
    fprintf(stderr, "Debug flag now off\n");
  }
  else{
    DEBUG = 1;
    fprintf(stderr, "Debug flag now on\n");
  }
}

/* set  FILE_NAME */
void set_file_name(){
  printf("plese enter a file name:\n");
  if(fgets(FILE_NAME,FILE_NAME_SIZE,stdin) == NULL){
      perror("error reading from user");
  }
  dprint(stderr, "Debug: file name set to %s",FILE_NAME);
}

/* set UNIT_SIZE */
void set_unit_size(){
  int u_size;
  printf("plese enter a unit size:\n");
  u_size = get_input_int();
  if(u_size == 1 || u_size == 2 || u_size ==4){
    UNIT_SIZE = u_size;
    dprint(stderr, "Debug: set size to  %d\n",u_size);
  }
  else{
    fprintf(stderr, "Error: incorrect unit size (%d)\n valid values are: 1, 2, 4\n",u_size);
  }
}

/* quits the program */
void quit(){
  dprint(stderr,"quitting\n");
  free(FILE_NAME);
  free(BUFF);
  free(INPUT_BUFF);
	exit(0);
}

void prompt_menu(fun_desc* fdarray){
  int i;
  printf("Please choose a function:\n");
  for(i =0 ; fdarray[i].name != NULL ; i++){
    printf("%i- %s\n", i, fdarray[i].name);
  }
}

/* determins wether the given index is within the menues bounds */
int valid_index(int index, fun_desc* fdarray){
  int i;
  if (index < 0){
    printf("Error: index out of bounds: %d\n",index);
      return 0;
  }
  for (i = 0; i<=index ;i++){
    if(fdarray[i].name == NULL){
      printf("Error: index out of bounds: %d\n",index);
      return 0;
    }
  }
  return 1;
}

int main(int argc, char **argv){
  FILE_NAME = calloc(FILE_NAME_SIZE,1);
  BUFF = calloc(BUFF_SIZE,1);
  INPUT_BUFF = calloc(INPUT_BUFF_SIZE,1);
  fun_desc fdarray[] = {
    create_fun_desc("Toggle Debug Mode",toggle_debug),
    create_fun_desc("Set File Name",set_file_name),
    create_fun_desc("Set Unit Size",set_unit_size),
    create_fun_desc("Quit",quit),
		create_fun_desc(NULL,NULL),
	};
  int input;
	while (1){
    dprint(stderr,"Debug:\tunit size: %d\n\tbuffer address: %p\n\tfile name: %s\n", UNIT_SIZE,BUFF,FILE_NAME);
    prompt_menu(fdarray);
		printf("Option: ");
    input = get_input_int();
    if(valid_index(input, fdarray)){
      fdarray[input].fun();
    }
	}
	return 0;
}
