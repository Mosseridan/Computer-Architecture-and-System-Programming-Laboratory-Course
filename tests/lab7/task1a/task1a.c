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

/* get input from user as an int between 0-9 returns the number or -1 on fail*/
int get_input_int(){
  char* input = get_input();
  int int_input;
  if(input != NULL && sscanf(input, "%d", &int_input) == 1){
      return int_input;
  }
  printf("Error: expected number <0-9> but given %s",input);
  return -1;
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
  printf("Please enter <file name>\n");
  if(fgets(FILE_NAME,FILE_NAME_SIZE,stdin) == NULL){
      perror("error reading from user");
  }
  dprint(stderr, "Debug: file name set to %s",FILE_NAME);
}

/* set UNIT_SIZE */
void set_unit_size(){
  int u_size;
  printf("Please enter <unit size>:\n");
  u_size = get_input_int();
  if(u_size == 1 || u_size == 2 || u_size ==4){
    UNIT_SIZE = u_size;
    dprint(stderr, "Debug: set unit size to  %d\n",u_size);
  }
  else{
    fprintf(stderr, "Error: incorrect unit size (%d)\n valid values are: 1, 2, 4\n",u_size);
  }
}

void mem_display(){
  char* input;
  char* address;
  int length, i;
  printf("Please enter <address> <length>\n> ");
  input = get_input();
  dprint(stderr,"Default value: %p\n",BUFF);
  if(input != NULL && sscanf(input,"%p %d",&address,&length) == 2){
    if(address == 0){
      address = BUFF;
    }
    printf("\n");
    for(i = 0; i<length*UNIT_SIZE; i++){
        printf("%02X", (unsigned char)address[i]);
        if((i+1)%UNIT_SIZE == 0) printf(" ");
        if((i+1)%4 == 0) printf(" ");
        if((i+1)%20 ==0) printf("\n");
    }
  }
  else fprintf(stderr, "Error: expected <address> <length> but given %s",input);
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
  printf("Choose action:\n");
  for(i =0 ; fdarray[i].name != NULL ; i++){
    printf("%i- %s\n", i, fdarray[i].name);
  }
}

/* determins wether the given index is within the menues bounds */
int valid_index(unsigned int index, fun_desc* fdarray){
  int i;
  for (i = 0; i<=index ;i++){
    if(fdarray[i].name == NULL){
      fprintf(stderr,"Error: index out of bounds: %d\n",index);
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
    create_fun_desc("Mem Display",mem_display),
    create_fun_desc("Quit",quit),
		create_fun_desc(NULL,NULL),
	};
  int input;
	while (1){
    dprint(stderr,"Debug:\tunit size: %d\n\tbuffer address: %p\n\tfile name: %s\n", UNIT_SIZE,BUFF,FILE_NAME);
    prompt_menu(fdarray);
		printf("> ");
    input = get_input_int();
    if(input >= 0 && valid_index(input, fdarray)){
      fdarray[input].fun();
    }
	}
	return 0;
}
