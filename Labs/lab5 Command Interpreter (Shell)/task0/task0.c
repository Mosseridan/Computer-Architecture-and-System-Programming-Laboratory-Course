#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <linux/limits.h>
#include "LineParser.h"

#define MAX_READ 2048

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
    printf("%s\n",pCmdLine->arguments[1]);
    perror("Error Executing command\n");
  }
}

int main(int argc, char** argv){
  char nextCommand[MAX_READ];
  cmdLine* commands;

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
      execute(commands);
      freeCmdLines(commands);
    }
  }
  return 0;
}
