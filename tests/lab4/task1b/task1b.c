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
  int i, input = STDIN, output = STDOUT;
  char* str;
  char* inputPath ="stdin";
  char* outputPath ="stdout";
  /* get args */
  for (i=1; i<argc; i++){
		if(strcmp(argv[i], "-d") == 0){
      DEBUG =1;
    }
    else if(strcmp(argv[i], "-i") == 0){
      if((input = dsCall(SYS_OPEN, (int)argv[++i], (char*)O_RDONLY, 0777))<0){
        str = "Error opening file ";
        system_call(SYS_WRITE,STDERR, str, strlen(str));
        system_call(SYS_WRITE,STDERR, argv[i], strlen(argv[i]));
        system_call(SYS_WRITE,STDERR,"\n",1);
        system_call(SYS_EXIT, input,0,0);
      }
      inputPath = argv[i];
    }
    else if(strcmp(argv[i], "-o") == 0){
      if((output = dsCall(SYS_OPEN, (int)argv[++i], (char*)O_WRONLY+64, 0777))<0){
        str = "Error opening file ";
        system_call(SYS_WRITE,STDERR, str, strlen(str));
        system_call(SYS_WRITE,STDERR, argv[i], strlen(argv[i]));
        system_call(SYS_WRITE,STDERR,"\n",1);
        system_call(SYS_EXIT, output,0,0);
      }
      outputPath = argv[i];
    }
	}
  /* print args */
  if(DEBUG) {
    printArgs(argc, argv, STDERR);
    /*print input path*/
    str = "input path is: ";
    system_call(SYS_WRITE,STDERR, str, strlen(str));
    system_call(SYS_WRITE,STDERR, inputPath, strlen(inputPath));
    system_call(SYS_WRITE,STDERR,"\n",1);
    /*print input path*/
    str = "output path is path is: ";
    system_call(SYS_WRITE,STDERR, str, strlen(str));
    system_call(SYS_WRITE,STDERR, outputPath, strlen(outputPath));
    system_call(SYS_WRITE,STDERR,"\n",1);
  }
  /* filter text */
  filterH(input, output);
  /* close input/output files if open */

  if(input != STDIN && dsCall(SYS_CLOSE, input, 0, 0)<0){
    str = "Error closing output file ";
    system_call(SYS_WRITE,STDERR, str, strlen(str));
    system_call(SYS_WRITE,STDERR, argv[i], strlen(argv[i]));
    system_call(SYS_WRITE,STDERR,"\n",1);
    system_call(SYS_EXIT, output,0,0);
  }
  if(output != STDOUT && dsCall(SYS_CLOSE, output, 0, 0)<0){
    str = "Error closing output file ";
    system_call(SYS_WRITE,STDERR, str, strlen(str));
    system_call(SYS_WRITE,STDERR, argv[i], strlen(argv[i]));
    system_call(SYS_WRITE,STDERR,"\n",1);
    system_call(SYS_EXIT, output,0,0);
  }

  return 0;
}
