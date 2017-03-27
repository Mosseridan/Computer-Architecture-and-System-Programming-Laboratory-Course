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

typedef struct History{
    int current;
    char entries[MAX_HISTORY][MAX_READ];
} History;

typedef struct Variable Variable;
struct Variable {
    char* name;
    char* value;
    Variable* next;
};

Variable* envptr;
int DEBUG = 0;

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
  return(NULL);
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

Variable* search_var(char* name){
  Variable* var = envptr;
  for(var=envptr; var != NULL; var = var->next){
    if(strcmp(var->name,name)==0)
      return var;
  }
  return NULL;
}

Variable* set_var(char* name, char* value){
  Variable* var = search_var(name);
  char* var_name = malloc(sizeof(name));
  char* var_value = malloc(sizeof(value));
  strcpy(var_name,name);
  strcpy(var_value,value);
  if (var == NULL){
    var = malloc(sizeof(Variable));
    var->name = var_name;
    var->value = var_value;
    var->next = envptr;
    envptr = var;
  }
  /* variable with name "name" already exists - replace value*/
  else{
    free(var->value);
    var->value = var_value;
  }
  return var;
}

void delete_var(char* name){
  Variable* var = envptr;
  Variable* prev = envptr;
  if(var != NULL){
    if(strcmp(var->name,name)==0){
        envptr = var->next;
        free(var);
        return;
    }
    else{
      var = var->next;
      while(var != NULL){
        if(strcmp(var->name,name)==0){
          prev->next = var->next;
          free(var);
          return;
        }
        prev = var;
        var = var->next;
      }
    }
  }
  fprintf(stderr, "Error no environment variable is set for %s\n",name);
}

void replace_vars(cmdLine* pCmdLine){
  int i, argCount = pCmdLine->argCount;
  Variable* var;
  for(i=0; i<argCount; i++){
    if(pCmdLine->arguments[i][0] == '$'){
      if((var = search_var(pCmdLine->arguments[i]+1)) == NULL){
        fprintf(stderr, "Error no environment variable is set for %s\n",pCmdLine->arguments[i]+1);
      }
      else{
        replaceCmdArg(pCmdLine, i, var->value);
      }
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
    parse_cmd:
    command = parseCmdLines(entry);
    if(command != NULL){
      replace_vars(command);
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
        print_history(&history);
        goto cmd_done;
      }
      /* if command is ! */
      if(command->arguments[0][0] == '!'){
        int n = atoi(command->arguments[0]+1);
        if(get_entry(entry, &history, n)==NULL){
          fprintf(stderr,"Error accesing history #%d : index out of bounds\n",n);
          goto cmd_done;
        }
        freeCmdLines(command);
        goto parse_cmd;
      }
      /* if coomand is set */
      else if (is_cmd(command, "set")){
        if(command->argCount < 3)
          fprintf(stderr, "Error not enough arugments supplied: %s",entry);
        else set_var(command->arguments[1],command->arguments[2]);
      }
      /* id command is delete */
      else if (is_cmd(command, "delete")){
        if(command->argCount < 2)
          fprintf(stderr, "Error not enough arugments supplied: %s",entry);
        delete_var(command->arguments[1]);
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
      strcpy(history.entries[history.current],entry);
      history.current = (history.current+1)%MAX_HISTORY;
      /* free memory */
      cmd_done:
      freeCmdLines(command);
    }
  }
  return 0;
}
