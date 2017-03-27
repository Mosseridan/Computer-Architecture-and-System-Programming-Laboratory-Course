#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>

#define dprint if (DEBUG) fprintf

int DEBUG = 0;

int main(int argc, char **argv) {
  int pipefd1[2];
  int pipefd2[2];
  pid_t cpid1, cpid2, cpid3;
  char *const ls_args[]   = {"ls","-l",0};
  char *const tail_args[]  = {"tail","-n","2",0};
  char *const wc_args[]  = {"wc","-l",0};
  int i, res, cStatus;

  /* chec for debug flag */
  for (i=1; i<argc; i++){
		if(strcmp(argv[i], "-d") == 0){
      DEBUG = 1;
    }
  }

  /* create pipes */
  if (pipe(pipefd1) == -1 || pipe(pipefd2) == -1) {
      perror("pipe");
      exit(EXIT_FAILURE);
  }
  dprint(stderr, "\t@ (parent_process>forking…)\n");
  cpid1 = fork();
  if (cpid1 == -1) {
      perror("fork");
      exit(EXIT_FAILURE);
  }
  if (cpid1 == 0) {    /* Child1 */
    dprint(stderr, "\t@ (child1>redirecting stdout to the write end of the pipe1…)\n");
    close(1);         /* close stdout */
    dup(pipefd1[1]);   /*duplicate pipe1s write end */
    close(pipefd1[1]); /* close duplicated file desctiptor */
    dprint(stderr, "\t@ (child1>going to execute cmd: ls -l…)\n");
    res = execvp(ls_args[0], ls_args); /* execute "ls -l" */
    if (res == -1) {
      perror("Error ");
      _exit(1);
    }
  }
  else {  /* Parent */
    dprint(stderr, "\t@ (parent_process>created process with id: %d)\n",cpid1);
    dprint(stderr, "\t@ (parent_process>closing the write end of the pipe1…)\n");
    close(pipefd1[1]); /* close pipe1s write end */
    cpid2 = fork();
    if (cpid2 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (cpid2 == 0){ /* Child2 */
      dprint(stderr, "\t@ (child2>redirecting stdin to the read end of the pipe1…)\n");
      close(0);      /* close stdin */
      dup(pipefd1[0]); /* duplicat pipe1s read end */
      close(pipefd1[0]); /* close duplicated file descriptor */

      dprint(stderr, "\t@ (child1>redirecting stdout to the write end of the pipe2…)\n");
      close(1);         /* close stdout */
      dup(pipefd2[1]);   /*duplicate pipe2s write end */
      close(pipefd2[1]); /* close duplicated file desctiptor */

      dprint(stderr, "\t@ (child2>going to execute cmd: tail -n 2…)\n");
      res = execvp(tail_args[0],tail_args); /* execute "tail -n 2" */
      if (res == -1) {
        perror("Error ");
        _exit(1);
      }
    }
    else{ /* Parent */
      dprint(stderr, "\t@ (parent_process>created process with id: %d)\n",cpid2);
      dprint(stderr, "\t@ (parent_process>closing the read end of the pipe1…)\n");
      close(pipefd1[0]); /* clsose pipe1s read end */
      dprint(stderr, "\t@ (parent_process>closing the write end of the pipe2…)\n");
      close(pipefd2[1]); /* close pipe2s write end */

      cpid3 = fork();
      if (cpid3 == -1) {
          perror("fork");
          exit(EXIT_FAILURE);
      }
      if (cpid3 == 0){ /*Child3 */
        dprint(stderr, "\t@ (child3>redirecting stdin to the read end of the pipe2…)\n");
        close(0);      /* close stdin */
        dup(pipefd2[0]); /* duplicat pipe2s read end */
        close(pipefd2[0]); /* close duplicated file descriptor */

        dprint(stderr, "\t@ (child3>going to execute cmd: wc -l…)\n");
        res = execvp(wc_args[0],wc_args); /* execute "wc -l" */
        if (res == -1) {
          perror("Error ");
          _exit(1);
        }
      }
      else{ /* Parent */
        dprint(stderr, "\t@ (parent_process>created process with id: %d)\n",cpid3);
        dprint(stderr, "\t@ (parent_process>closing the read end of the pipe2…)\n");
        close(pipefd2[0]); /* clsose pipe2s read end */

        dprint(stderr, "\t@ (parent_process>waiting for child processes to terminate…)\n");
        waitpid(cpid1, &cStatus, 0); /* wait for cpid1 to finish */
        waitpid(cpid2, &cStatus, 0); /* wait for cpid2 to finish */
        waitpid(cpid3, &cStatus, 0); /* wait for cpid3 to finish */
      }



    }
  }dprint(stderr, "\t@ (parent_process>exiting…)\n");
  return 0;
}
