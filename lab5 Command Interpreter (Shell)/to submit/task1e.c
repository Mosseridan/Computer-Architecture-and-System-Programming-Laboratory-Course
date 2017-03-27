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
    printf("%s>", cwd);
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

/* returns the n'th entry in history and copy it to entry returns pointer to entry or null if failed*/
char* get_entry(char* entry, History* history, int n){
  int current = history->current;
  if(n < MAX_HISTORY){
      if(strcmp(history->entries[current],"")) {
        return strcpy(entry, history->entries[(current+n)%MAX_HISTORY]);
      }
      else if(n<current){
        return strcpy(entry, history->entries[n]);
      }
  }
  return NULL;
}

char* get_entry_prefix(char* entry, History* history, char prefix){
  int i;
  for (i = 0; i< MAX_HISTORY; i++){
    if(get_entry(entry, history, i)==NULL){
      break;
    }
    else if(entry[0]==prefix){
      return entry;
    }
  }
  fprintf(stderr,"Error : no history entry with the prefix '%c'\n",prefix);
  return NULL;
}

void print_history(History* history){
    int i;
    char entry[MAX_READ];
    for(i =0; i<MAX_HISTORY; i++){
      if(get_entry(entry,history, i) == NULL){
        break;
      }
      printf("#%d %s",i,entry);
    }
}

int main(int argc, char** argv){
  int i, cpid, cStatus;
  char entry[MAX_READ];
  cmdLine* command;
  History* history = calloc(1,sizeof(History));
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
    /* if command is ! */
    if(entry[0]=='!' && get_entry_prefix(entry, history, entry[1])==NULL){
      command = NULL;
    }
    else{
      command = parseCmdLines(entry);
    }
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
      /* if command is history */
      else if (is_cmd(command, "history")){
        print_history(history);
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
      /* update history */
      strcpy(history->entries[history->current],entry);
      history->current = (history->current+1)%MAX_HISTORY;
      /* free memory */
      freeCmdLines(command);
    }
  }
  free(history);
  return 0;
}
