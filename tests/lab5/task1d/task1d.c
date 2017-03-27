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

typedef struct History{
    int current;
    char entries[MAX_HISTORY][MAX_READ];
} History;

/* print cwd prompt */
void prompt(){
    char cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);
    printf("%s> ", cwd);
}

/* check if cmdLine hold the command  cmd */
int is_cmd(cmdLine* pCmdLine, char* cmd){
  return ((strcmp(pCmdLine->arguments[0], cmd) == 0));
}

/* execute command */
void execute(cmdLine *pCmdLine) {
  int res;
  /*replaces the current process image with a new process image.*/
  res = execvp(pCmdLine->arguments[0], pCmdLine->arguments);
  if (res == -1) {
    perror("Error Executing command ");
    _exit(1);
  }
}
/* apply cd with dir_path */
void cd(char* dir_path){
  int res;
    dprint(stderr, "\t@ executing command: cd %s", dir_path);
    res = chdir(dir_path);
    if (res == -1){
      perror("Error changing directory ");
    }
}

void print_history(History* history){
    int i, current = history->current;
    if(strcmp(history->entries[current],"")==0){
      for(i=0; i<current; i++){
        printf("#%d %s",i,history->entries[i]);
      }
    }
    else{
      for(i=0; i<MAX_HISTORY ;i++){
        printf("#%d %s",i,history->entries[(current+i)%MAX_HISTORY]);
      }
    }
}

int main(int argc, char** argv){
  char entry[MAX_READ];
  cmdLine* command;
  History history;
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
    fgets(entry, MAX_READ, stdin);
    command = parseCmdLines(entry);
    if(command != NULL){
      /* if command is quit */
      if(is_cmd(command, "quit")){
        freeCmdLines(command);
        printf("Exiting...\n");
        break;
      }
      /* if command is cd */
      else if (is_cmd(command, "cd")) {
        cd(command->arguments[1]);
      }
      else if (is_cmd(command, "history")){
        print_history(&history);
      }
      else{
        /* fork */
        dprint(stderr, "\t@ applying fork\n");
        cpid = fork();
        dprint(stderr, "\t@ running PID: %d, CPID: %d\n", getpid(), cpid);
        /* if child */
        if (cpid == 0) {
          dprint(stderr, "\t@ executing command: %s", entry);
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
      }
      strcpy(history.entries[history.current],entry);
      history.current = (history.current+1)%MAX_HISTORY;

      /* free memory */
      freeCmdLines(0);
    }
  }
  return 0;
}
