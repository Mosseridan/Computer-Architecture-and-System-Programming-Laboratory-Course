#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "LineParser.h"

#define MAX_READ 2048
#define MAX_HISTORY 10
#define dprint if (DEBUG) fprintf
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
    perror("Error Executing command ");
    _exit(1);
  }
}

int main(int argc, char** argv){
  char nextCommand[MAX_READ];
  cmdLine* command;
  int i, cpid, cStatus;
  /* chec for debug flag */
  for (i=1; i<argc; i++){
    if(strcmp(argv[i], "-d") == 0){
      DEBUG = 1;
    }
  }
  /* main loop */
  dprint(stderr, "\t@ running PID: %d\n", getpid());
  while(1){
    prompt();
    fgets(nextCommand, MAX_READ, stdin);
    command = parseCmdLines(nextCommand);
    if(command != NULL){
      if(isEnd(command)){
        freeCmdLines(command);
        printf("Exiting...\n");
        break;
      }
      /* fork */
      dprint(stderr, "\t@ applying fork\n");
      cpid = fork();
      dprint(stderr, "\t@ running PID: %d, CPID: %d\n", getpid(), cpid);
      /* if child */
      if (cpid == 0) {
        dprint(stderr, "\t@ executing command: %s", nextCommand);
        execute(command);
        _exit(0);
      }
      /* if perant and command is blocking wait for child */
      if (command->blocking == 1) {
        dprint(stderr, "\t@ waiting for CPID: %d to finish\n", cpid);
        waitpid(cpid, &cStatus, 0);
        dprint(stderr, "\t@ CPID: %d has finish with status: %d\n", cpid, cStatus);
        dprint(stderr, "\t@ running PID: %d\n", getpid());
      }
      /* free memory */
      freeCmdLines(command);
    }
  }
  return 0;
}
