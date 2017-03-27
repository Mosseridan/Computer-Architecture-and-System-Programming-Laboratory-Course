#include "util.h"

#define SYS_WRITE 4
#define SYS_READ 3
#define STDIN 0
#define STDOUT 1
#define STDERR 2

extern int system_call(int arg1, int arg2, char* arg3, int arg4);

int DEBUG = 0;

void dprint(char* msg){
  system_call(SYS_WRITE, STDERR, "\t",1);
  system_call(SYS_WRITE, STDERR, msg,strlen(msg));
}

void dprintI(char* msg, int param){
  system_call(SYS_WRITE, STDERR, "\t",1);
  system_call(SYS_WRITE, STDERR, msg,strlen(msg));
  char* str = itoa(param);
  system_call(SYS_WRITE, STDERR, str,strlen(str));
}

void dprintS(char* msg, char* param){
  system_call(SYS_WRITE, STDERR, "\t",1);
  system_call(SYS_WRITE, STDERR, msg,strlen(msg));
  system_call(SYS_WRITE, STDERR, param,strlen(param));
}

int dsCall(int arg1, int arg2, char* arg3, int arg4){
  int ret = system_call(arg1,arg2,arg3,arg4);
  if(DEBUG){
    dprintI("system call ID: ", arg1);
    dprintI("was called, and it's return code is: ", ret);
    dprint("\n");
  }
  return ret;
}

void fprint(char* msg, int fd){
  dsCall(SYS_WRITE,STDERR,msg, strlen(msg));
}

void print(char* msg){
  fprint(msg, STDIN);
}

void fprintI(char* msg,int param, int fd){
  dsCall(SYS_WRITE,STDERR,msg, strlen(msg));
  char* str = itoa(param);
  dsCall(SYS_WRITE,STDERR, str, strlen(str));
}

void printI(char* msg,int param){
  fprintI (msg, param, STDOUT);
}

void printArgs(int argc, char* argv[], int output){
  int i;
  dprintI("number of args: ", argc);
  dprint("\n");
  for (i = 0 ; i < argc ; i++){
      dprintI("argv[",i);
      dprintS("] = ",argv[i]);
      dprint("\n");
  }
}

/* filters given file with default h Filter */
void filterH(int input, int output){
	char c = 1;
	while(dsCall(SYS_READ, input, &c, 1)>0 && c!='\n'){
    if(DEBUG){
      dprintS("character read: ", &c);
      dprint("\n");
    }
		switch(c){
			case 'h':
			case 'H':
        if(DEBUG){
          dprintS("filtered out: ", &c);
          dprint("\n");
        }
        break;
			default:
        fprint(&c,output);
        break;
		}
	}
  fprint("\n",output);
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
 
