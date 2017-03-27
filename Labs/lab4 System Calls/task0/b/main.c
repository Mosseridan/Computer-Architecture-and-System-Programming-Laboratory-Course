#include "util.h"

#define SYS_READ 3
#define SYS_WRITE 4
#define SYS_OPEN 5
#define SYS_CLOSE 6
#define SYS_LSEEK 19
#define O_RDRW 2
#define SEEK_SET 0
#define STDOUT 1
#define STDERR 2
#define REPLACE_POSITION 0x291

int main (int argc , char* argv[], char* envp[]){
  int file;
  char * str;
  /* check number of args args */
  if(argc != 3){
    str = "Wrong parameters, expected:\npatch <FILE_NAME> <X_NAME>\n";
    system_call(SYS_WRITE,STDERR, str, strlen(str));
  }
  /* try to open file */
  if((file = system_call(SYS_OPEN,argv[1], O_RDRW, 0777))<0){
    str = "Error opening file ";
    system_call(SYS_WRITE,STDERR, str, strlen(str));
    system_call(SYS_WRITE,STDERR, argv[1], strlen(argv[1]));
    system_call(SYS_WRITE,STDERR,"\n",1);
    return 0;
  }
  /* move cursor to the start of the word "Shira in the file" */
  system_call(SYS_LSEEK, file, REPLACE_POSITION, SEEK_SET);
  /* replace "Shira" with <X_NAME> */
  system_call(SYS_WRITE, file, argv[2],strlen(argv[2]));
  system_call(SYS_WRITE, file, ".\n\0", 3);
  /* close file */
  if(system_call(SYS_CLOSE, file, 0, 0)<0){
    str = "Error closeing file ";
    system_call(SYS_WRITE,STDERR, str, strlen(str));
    system_call(SYS_WRITE,STDERR, argv[1], strlen(argv[1]));
    system_call(SYS_WRITE,STDERR,"\n",1);
  }
  return 0;
}
