#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
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
History* history;
int** pipes;
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

/* apply cd with dir_path */
void cd(char* dir_path){
  dprint(stderr, "\t@ executing command: cd %s\n", dir_path);
  int res = chdir(dir_path);
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

void free_var(Variable* var){
	free(var->name);
	free(var->value);
}

void delete_var(char* name){
  Variable* var = envptr;
  Variable* prev = envptr;
  if(var != NULL){
    if(strcmp(var->name,name)==0){
        envptr = var->next;
        free_var(var);
        free(var);
        return;
    }
    else{
      var = var->next;
      while(var != NULL){
        if(strcmp(var->name,name)==0){
          prev->next = var->next;
          free_var(var);
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
    else if (pCmdLine->arguments[i][0] == '~'){
      char str[MAX_READ];
      sprintf(str,"%s%s",getenv("HOME"),pCmdLine->arguments[i]+1);
      replaceCmdArg(pCmdLine, i, str);
    }
  }
}

void clear_env(){
	while(envptr != NULL){
		delete_var(envptr->name);
	}
}

void print_env(){
  	Variable* var = envptr;
  	while(var != NULL){
  		printf("%s = %s\n",var->name, var->value);
  		var = var->next;
  	}
 }

/* This function receives the number of required pipes and returns an array of pipes. */
 int **createPipes(int nPipes){
   int i;
   int **pipes;
   if((pipes = calloc(nPipes,sizeof(int*))) == NULL){
     return NULL;
   }
   for(i=0; i<nPipes; i++){
     if(pipe(pipes[i]=calloc(2,sizeof(int))) == -1) {
         perror("pipe");
         exit(EXIT_FAILURE);
     }
   }
   return pipes;
 }

/*This function receives an array of pipes and an integer indicating the size of the array.
 The function releases all memory dedicated to the pipes. */
 void releasePipes(int **pipes, int nPipes){
   int i;
   for(i=0; i<nPipes; i++){
     free(pipes[i]);
   }
   free(pipes);
 }

/*This function receives an array of pipes and a pointer to a cmdLine structure.
 It returns the pipe which feeds the process associated with the command.
 That is, the pipe that appears to the left of the process name in the command line.*/
 int *leftPipe(int **pipes, cmdLine *pCmdLine){
   if(pCmdLine->idx == 0){
     return NULL;
   }
   return pipes[pCmdLine->idx-1];
 }

/*This function receives an array of pipes and a pointer to a cmdLine structure.
 It returns the pipe which is the sink of the associated command.
 That is, the pipe that appears to the right of the process name in the command line.*/
 int *rightPipe(int **pipes, cmdLine *pCmdLine){
   if(pCmdLine->next == NULL){
     return NULL;
   }
   return pipes[pCmdLine->idx];
 }

 /* redirects proccess input, if needed */
  void redirect_input(cmdLine *pCmdLine,int* leftPipe){
    /* this is the last command in the command chain
       and there is an input redirection specified */
    if (pCmdLine->inputRedirect != NULL){
       dprint(stderr, "\t@ (child%d CPID: %d>redirecting stdin to %s…)\n",pCmdLine->idx,getpid(),pCmdLine->inputRedirect);
       close(0);
       fopen(pCmdLine->inputRedirect,"r");
    }
    /* this command has an output redirection through a pipe */
    else if(leftPipe != NULL){
      dprint(stderr, "\t@ (child%d CPID: %d>redirecting stdin to the read end of the pipe%d…)\n",pCmdLine->idx,getpid(),pCmdLine->idx-1);
      close(0);         /* close stdin on child i*/
      dup(leftPipe[0]);   /*duplicate pipe(i)s read end */
      close(leftPipe[0]); /* close duplicated file desctiptor */
    }
  }

 /* redirects proccess output, if needed */
  void redirect_output(cmdLine *pCmdLine, int* rightPipe){
    /* this is the first command in the command chain
       and there is an output redirection specified */
    if(pCmdLine->outputRedirect != NULL){
      dprint(stderr, "\t@ (child%d CPID: %d>redirecting stdout to %s…)\n",pCmdLine->idx,getpid(),pCmdLine->outputRedirect);
      close(1);
      fopen(pCmdLine->outputRedirect,"a");
    }
    /* this command has an output redirection through a pipe */
    else if(rightPipe != NULL){
      dprint(stderr, "\t@ (child%d CPID: %d>redirecting stdout to the write end of the pipe%d…)\n",pCmdLine->idx,getpid(),pCmdLine->idx);
      close(1);         /* close stdout on child i*/
      dup(rightPipe[1]);   /*duplicate pipe(i)s write end */
      close(rightPipe[1]); /* close duplicated file desctiptor */
    }
  }

 /*counts the number of commands in the given command chain */
 int count_commands(cmdLine *pCmdLine){
   while(pCmdLine->next != NULL){
     pCmdLine = pCmdLine->next;
   }
   return pCmdLine->idx+1;
 }

/* frees all allocated data and exits */
 void free_and_exit(int ret, cmdLine *pCmdLine){
   releasePipes(pipes, count_commands(pCmdLine)-1);
   freeCmdLines(pCmdLine);
   free(history);
   clear_env();
   _exit(ret);
 }

 /* execute command */
 int execute(cmdLine *pCmdLine) {
   int res;
   /*replaces the current process image with a new process image.*/
   dprint(stderr, "\t@ (child%d CPID: %d>going to execute cmd: %s…)\n",pCmdLine->idx,getpid(), pCmdLine->arguments[0]);
   /* if command is history */
   if (is_cmd(pCmdLine, "history")){
        print_history(history);
        free_and_exit(0, pCmdLine);
   }
   res = execvp(pCmdLine->arguments[0], pCmdLine->arguments);
   if (res == -1) {
     perror("Error Executing command ");
   }
   return res;
 }

/* creates a child process for each command in the given command chain.
   and executes each command in its designate child process,
   while redirecting I/O as needed */
 void create_and_execute_child_processes(cmdLine *pCmdLine){
   int i,res, cStatus;
   cmdLine *command = pCmdLine;
   int command_count = count_commands(pCmdLine);
   pid_t cpids[command_count];
   int *left_pipe;
   int *right_pipe;

   pipes = createPipes(command_count-1);
   for(i = 0; i< command_count; command = command->next, i++ ){
     dprint(stderr, "\t@ (parent_process>forking…)\n");
     cpids[i] = fork();
     if(cpids[i] == -1){
       perror("fork");
       exit(EXIT_FAILURE);
     }
     left_pipe = leftPipe(pipes, command);
     right_pipe = rightPipe(pipes, command);
     if (cpids[i] == 0){ /* Child i */
       redirect_input(command, left_pipe);
       redirect_output(command, right_pipe);
       res = execute(command);
       free_and_exit(res, pCmdLine);
     }
     else{ /* Parent */
       dprint(stderr, "\t@ (parent_process>created process with id: %d)\n",cpids[i]);
       if(left_pipe != NULL){
         dprint(stderr, "\t@ (parent_process>closing the read end of the pipe%d…)\n",i-1);
         close(left_pipe[0]); /* clsose pipe(i-1)s read end */
       }
       if(right_pipe != NULL){
         dprint(stderr, "\t@ (parent_process>closing the write end of the pipe%d…)\n",i);
         close(right_pipe[1]); /* close pipe(i)s write end */
       }
     }
   }
    /* will run only on Parent (child terminates after execute)*/
    command = pCmdLine;
    for(i = 0; i< command_count; command = command->next, i++){
      /* if command(i) in chain is blocking wait for child(i) to teminate */
      if (command->blocking == 1) {
        dprint(stderr, "\t@ (parent_process>waiting for CPID:  %d to terminate…)\n",cpids[i]);
        waitpid(cpids[i], &cStatus, 0);
        dprint(stderr, "\t@ (CPID: %d has finish with status: %d)\n", cpids[i], cStatus);
      }
    }
    /* free pipes */
    releasePipes(pipes, command_count-1);
 }

/* Main */
int main(int argc, char** argv, char *envp[]){
  char entry[MAX_READ];
  cmdLine* command;
  history = calloc(1,sizeof(History));
  int i;
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
      /* if coomand is set */
      else if (is_cmd(command, "set")){
        if(command->argCount < 3)
          fprintf(stderr, "Error not enough arugments supplied: %s",entry);
        else set_var(command->arguments[1],command->arguments[2]);
      }
      /* if command is delete */
      else if (is_cmd(command, "delete")){
        if(command->argCount < 2)
          fprintf(stderr, "Error not enough arugments supplied: %s",entry);
        delete_var(command->arguments[1]);
      }
      /* if command is env */
      else if (is_cmd(command, "env")){
      	print_env();
      }
      else{
        create_and_execute_child_processes(command);
      }
      /* update history */
      strcpy(history->entries[history->current],entry);
      history->current = (history->current+1)%MAX_HISTORY;
      /* free memory */
      freeCmdLines(command);
    }
  }
  free(history);
  clear_env();
  return 0;
}
