#include "util.h"

#define SYS_EXIT 1
#define SYS_READ 3
#define SYS_WRITE 4
#define SYS_OPEN 5
#define SYS_CLOSE 6
#define SYS_LSEEK 19
#define SYS_GETDENTS 141
#define DT_REG 8
#define O_RDONLY 0
#define O_WRONLY 1
#define SEEK_SET 0
#define SEEK_END 2
#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define BUF_SIZE 8192

extern int system_call(int arg1, int arg2, char* arg3, int arg4);
extern void infection();
extern void infector(char * name);

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

void dprint(char* msg){
  system_call(SYS_WRITE, STDERR, "\t",1);
  system_call(SYS_WRITE, STDERR, msg,strlen(msg));
}

void dprintI(char* msg, int param){
  system_call(SYS_WRITE, STDERR, "\t",1);
  system_call(SYS_WRITE, STDERR, msg,strlen(msg));
  char* str = itoa(param);
  system_call(SYS_WRITE, STDERR, str,strlen(str));
}

void dprintS(char* msg, char* param){
  system_call(SYS_WRITE, STDERR, "\t",1);
  system_call(SYS_WRITE, STDERR, msg,strlen(msg));
  system_call(SYS_WRITE, STDERR, param,strlen(param));
}

int dsCall(int arg1, int arg2, char* arg3, int arg4){
  int ret = system_call(arg1,arg2,arg3,arg4);
  if(DEBUG){
    dprintI("system call ID: ", arg1);
    dprintI("was called, and it's return code is: ", ret);
    dprint("\n");
  }
  return ret;
}

void fprint(char* msg, int fd){
  dsCall(SYS_WRITE,STDERR,msg, strlen(msg));
}

void print(char* msg){
  fprint(msg, STDIN);
}

void fprintI(char* msg,int param, int fd){
  dsCall(SYS_WRITE,STDERR,msg, strlen(msg));
  char* str = itoa(param);
  dsCall(SYS_WRITE,STDERR, str, strlen(str));
}

void printI(char* msg,int param){
  fprintI (msg, param, STDOUT);
}

void printEnt(ent* entp){
  dprintI("entp = { d_ino: ",entp->d_ino);
  dprintI(",d_off: ",entp->d_off);
  dprintI(",d_reclen: ",entp->d_reclen);
  dprintS(",d_name: ",entp->d_name);
  dprintI(",d_type: ",(int)*((char*)entp+entp->d_reclen - 1));
  dprint("}\n");
}

void printArgs(int argc, char* argv[], int output){
  int i;
  dprintI("number of args: ", argc);
  dprint("\n");
  for (i = 0 ; i < argc ; i++){
      dprintI("argv[",i);
      dprintS("] = ",argv[i]);
      dprint("\n");
  }
}

void quit(char* msg){
  dprint(msg);
  system_call(SYS_EXIT, 55,0,0); 
}

int sufcmp(char* fName,char suffix){
  int i;
  char suf;
  for(i =0; *(i+fName)!=0;i++){
    suf = *(i+fName);
  }
  if(DEBUG){
    dprintS("file suffix is: ",&suf);
    dprint("\n");
  }
  return(suffix == suf);
}

int main (int argc, char* argv[], char* envp[]){
  int i, size,fd, length = 32;
  char buf[BUF_SIZE];
  ent* entp;
  /* get args */
  for (i=1; i<argc; i++){
    if(strcmp(argv[i], "-d") == 0){
      DEBUG = 1;
    }  
  }
  /* print args */
  if(DEBUG) {
    printArgs(argc, argv, STDERR);
  }
  /*write something to screen*/
  print("I see you Mohahahaha...\nJust kidding, Here are your Frikin' files:\n");
  /* open current dir file*/
  if((fd = dsCall(SYS_OPEN, (int)".", O_RDONLY, 0777))<0){
    quit("error opening dir file\n");
  }

  while(1){
    size = dsCall(SYS_GETDENTS, fd, buf, BUF_SIZE);
    if(DEBUG){
      dprintI("size is: ",size);
      dprint("\n");
    }
    if(size<0){
      quit("error with getdents\n");
    }
    if(size == 0){
      break;
    }
    while(length<size){
      entp = (ent*)(buf+length);
      if(DEBUG){
        printEnt(entp);
      }

      print(entp->d_name);
      print("\n");

      length += entp->d_reclen;
    }
  }
  /*close dir file */
  if(dsCall(SYS_CLOSE, fd, 0, 0)<0){
    quit("error closing dir file\n");
  }
  return 0;
}
