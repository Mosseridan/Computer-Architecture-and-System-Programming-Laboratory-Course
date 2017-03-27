#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <linux/limits.h>
#include "LineParser.h"

#define MAX_READ 2048
int DEBUG = 0;

void prompt(){
    char cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);
    printf("%s> ", cwd);
}

int isEnd(cmdLine* pCmdLine){
  return ((strcmp(pCmdLine->arguments[0], "quit") == 0));
}

void execute(cmdLine *pCmdLine) {
  int res;
  /*replaces the current process image with a new process image.*/
  res = execvp(pCmdLine->arguments[0], pCmdLine->arguments);
  if (res == -1) {
    perror("Error Executing command\n");
    _exit(1);
  }
}

int main(int argc, char** argv){
  char nextCommand[MAX_READ];
  cmdLine* commands;
  int i, cpid;

  for (i=1; i<argc; i++){
		if(strcmp(argv[i], "-d") == 0){
      DEBUG = 1;
    }
  }

  while(1){
    prompt();
    fgets(nextCommand, MAX_READ, stdin);
    commands = parseCmdLines(nextCommand);
    if(commands != NULL){
      if(isEnd(commands)){
        freeCmdLines(commands);
        printf("Exiting...\n");
        break;
      }

      if (DEBUG) { fprintf(stderr, "applying fork.\n"); }
      cpid = fork();
      if (DEBUG) { fprintf(stderr, "running PID: %d, CPID: %d\n", getpid(), cpid); }

      if (cpid == 0) {
        if(DEBUG) { fprintf(stderr, "executing command: %s", nextCommand); }
        execute(commands);
        _exit(0);
      }
      freeCmdLines(commands);
    }
  }
  return 0;
}
 
