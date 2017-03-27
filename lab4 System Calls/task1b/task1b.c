#include "util.h"

#define SYS_EXIT 1
#define SYS_READ 3
#define SYS_WRITE 4
#define SYS_OPEN 5
#define SYS_CLOSE 6
#define SYS_LSEEK 19
#define O_RDONLY 0
#define O_WRONLY 1
#define SEEK_SET 0
#define SEEK_END 2
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
  dsCall(SYS_WRITE,fd,msg, strlen(msg));
}

void print(char* msg){
  fprint(msg, STDIN);
}

void fprintI(char* msg,int param, int fd){
  dsCall(SYS_WRITE,fd,msg, strlen(msg));
  char* str = itoa(param);
  dsCall(SYS_WRITE,fd, str, strlen(str));
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

void quit(char* msg){
  dprint(msg);
  system_call(SYS_EXIT, 0,0,0); 
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
  int i, input = STDIN, output = STDOUT;
  char* inputPath ="stdin";
  char* outputPath ="stdout";
  /* get args */
  for (i=1; i<argc; i++){
    if(strcmp(argv[i], "-d") == 0){
      DEBUG =1;
    }
    else if(strcmp(argv[i], "-i") == 0){
      inputPath = argv[++i];
      if(DEBUG){ dprintS("opening file: ", inputPath);}
      if((input = dsCall(SYS_OPEN, (int)inputPath, (char*)O_RDONLY, 0777))<0)
        quit("Error opening file");
    }
    else if(strcmp(argv[i], "-o") == 0){
      outputPath = argv[++i];
      if(DEBUG){dprintS("opening file: ", outputPath);}
      if((output = dsCall(SYS_OPEN, (int)outputPath, (char*)O_WRONLY+64, 0777))<0)
        quit("Error opening file");
    }
  }
  /* print args */
  if(DEBUG) {
    printArgs(argc, argv, STDERR);
    /*print input path*/
    dprintS("input path is: ",inputPath);
    dprintS("output path is: ",outputPath);
    dprint("\n");
  }
  /* filter text */
  filterH(input, output);
  /* close input/output files if open */

  if(input != STDIN){
    if(DEBUG){dprintS("closing file: ", inputPath);}
    if(dsCall(SYS_CLOSE, input, 0, 0)<0)
      quit("Error closing file");
  }
  if(input != STDOUT){
    if(DEBUG){dprintS("closing file: ", outputPath);}
    if(dsCall(SYS_CLOSE, output, 0, 0)<0)
      quit("Error closing file"); 
  }

  return 0;
}
