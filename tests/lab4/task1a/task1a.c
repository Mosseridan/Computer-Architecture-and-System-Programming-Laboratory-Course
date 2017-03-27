#include "util.h"

#define SYS_READ 3
#define SYS_WRITE 4
#define STDIN 0
#define STDOUT 1
#define STDERR 2

extern int system_call(int arg1, int arg2, char* arg3, int arg4);

int DEBUG = 0;

int dsCall(int arg1, int arg2, char* arg3, int arg4){
  int ret = system_call(arg1,arg2,arg3,arg4);
  char* str;
  if(DEBUG){
    str = "\tsystem call ID: ";
    system_call(SYS_WRITE,STDERR,str, strlen(str));
    str = itoa(arg1);
    system_call(SYS_WRITE,STDERR, str,strlen(str));
    str = " was called, and it's return code is: ";
    system_call(SYS_WRITE,STDERR, str, strlen(str));
    str = itoa(ret);
    system_call(SYS_WRITE,STDERR, str,strlen(str));
    system_call(SYS_WRITE,STDERR,".\n",2);
  }
  return ret;
}

void printArgs(int argc, char* argv[], int output){
  int i;
  char * str;
  str = itoa(argc);
  system_call(SYS_WRITE,output, str,strlen(str));
  system_call(SYS_WRITE,output," arguments \n", 12);
  for (i = 0 ; i < argc ; i++){
      system_call(SYS_WRITE,output,"argv[", 5);
	     str = itoa(i);
      system_call(SYS_WRITE,output,str,strlen(str));
      system_call(SYS_WRITE,output,"] = ",4);
      system_call(SYS_WRITE,output,argv[i],strlen(argv[i]));
      system_call(SYS_WRITE,output,"\n",1);
  }
}
/* filters given file with default h Filter */
void filterH(int input, int output){
	char c = 1;
  char* str;
	while(dsCall(SYS_READ, input, &c, 1)>0 && c!='\n'){
    if(DEBUG){
      str = "\tcharacter read: ";
      system_call(SYS_WRITE, STDERR, str, strlen(str));
      system_call(SYS_WRITE, STDERR, &c, 1);
      system_call(SYS_WRITE, STDERR, "\n", 1);
    }
		switch(c){
			case 'h':
			case 'H':
        if(DEBUG){
          str = "\tcharacter filtered: ";
          system_call(SYS_WRITE, STDERR, str, strlen(str));
          system_call(SYS_WRITE, STDERR, &c, 1);
          system_call(SYS_WRITE, STDERR, "\n", 1);
        }
        break;
			default:
        dsCall(SYS_WRITE, output, &c, 1);
        break;
		}
	}
  dsCall(SYS_WRITE, output, "\n", 1);
}


int main (int argc, char* argv[], char* envp[]){
  int i;
  /* get args */
  for (i=1; i<argc; i++){
		if(strcmp(argv[i], "-d") == 0){
      DEBUG = 1;
    }
	}
  /* print args */
  if(DEBUG) {
    printArgs(argc, argv, STDERR);
  }
  /* filter text */
  filterH(STDIN, STDOUT);
  return 0;
}
