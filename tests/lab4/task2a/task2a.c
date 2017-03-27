#include "util.h"

#define SYS_EXIT 1
#define SYS_READ 3
#define SYS_WRITE 4
#define SYS_OPEN 5
#define SYS_CLOSE 6
#define SYS_LSEEK 19
#define SYS_GETDENTS 141
#define O_RDONLY 0
#define O_WRONLY 1
#define SEEK_SET 0
#define SEEK_END 2
#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define BUF_SIZE 8192

extern int system_call(int arg1, int arg2, char* arg3, int arg4);

int DEBUG = 0;

typedef struct linux_dirent {
 unsigned long  d_ino;     /* Inode number */
 unsigned long  d_off;     /* Offset to next linux_dirent */
 unsigned short d_reclen;  /* Length of this linux_dirent */
 char           d_name[];  /* Filename (null-terminated) */
                   /* length is actually (d_reclen - 2 -
                      offsetof(struct linux_dirent, d_name)) */
 /*
 char           pad;       // Zero padding byte
 char           d_type;    // File type (only since Linux
                           // 2.6.4); offset is (d_reclen - 1)
 */
} ent;

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

void printEnt(ent* entp){
  char* str;
  str = "\tentp = { d_ino: ";
  system_call(SYS_WRITE, STDERR, str,strlen(str));
  str = itoa(entp->d_ino);
  system_call(SYS_WRITE, STDERR, str,strlen(str));
  str = ", d_off: ";
  system_call(SYS_WRITE, STDERR, str,strlen(str));
  str = itoa(entp->d_off);
  system_call(SYS_WRITE, STDERR, str,strlen(str));
  str = ", d_reclen: ";
  system_call(SYS_WRITE, STDERR, str,strlen(str));
  str = itoa(entp->d_reclen);
  system_call(SYS_WRITE, STDERR, str,strlen(str));
  str = ", d_name: ";
  system_call(SYS_WRITE, STDERR, str,strlen(str));
  str = entp->d_name;
  system_call(SYS_WRITE, STDERR, str,strlen(str));
  system_call(SYS_WRITE, STDERR, " }\n",3);
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

int main (int argc, char* argv[], char* envp[]){
  int i, size,fd, length = 32;
  char* str;
  char buf[BUF_SIZE];
  ent* entp;
  /* get args */
  for (i=1; i<argc; i++){
		if(strcmp(argv[i], "-d") == 0){
      DEBUG =1;
    }
	}
  /* print args */
  if(DEBUG) {
    printArgs(argc, argv, STDERR);
  }
  /*write something to screen*/
  str = "I see you Mohahahaha...\nJust kidding, Here are your Frikin' files:\n";
  dsCall(SYS_WRITE, STDOUT, str, strlen(str));
  /* open current dir file*/
  if((fd = dsCall(SYS_OPEN, (int)".", O_RDONLY, 0777))<0){
    str = "error opening dir file\n";
    system_call(SYS_WRITE, STDERR, str,strlen(str));
    system_call(SYS_EXIT,55 ,0,0);
  }

  while(1){
    size = dsCall(SYS_GETDENTS, fd, buf, BUF_SIZE);
    if(DEBUG){
      str = "\tsize is: ";
      system_call(SYS_WRITE, STDERR, str,strlen(str));
      str = itoa(size);
      system_call(SYS_WRITE, STDERR, str,strlen(str));
      system_call(SYS_WRITE, STDERR, "\n",1);
    }
    if(size<0){
      str = "error with getdents\n";
      system_call(SYS_WRITE, STDERR, str,strlen(str));
      system_call(SYS_EXIT,55 ,0,0);
    }
    if(size == 0){
      break;
    }
    while(length<size){
      entp = (ent*)(buf+length);
      if(DEBUG){
        printEnt(entp)
      }
      dsCall(SYS_WRITE, STDOUT, entp->d_name, strlen(entp->d_name));
      dsCall(SYS_WRITE,STDOUT, "\n",1);
      length += entp->d_reclen;
    }
  }

  /*close file */
  if(dsCall(SYS_CLOSE, fd, 0, 0)<0){
    str = "error closing dir file\n";
    system_call(SYS_WRITE, STDERR, str,strlen(str));
    system_call(SYS_EXIT,55 ,0,0);
  }
  return 0;
}
