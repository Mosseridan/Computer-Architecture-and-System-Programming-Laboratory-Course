#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <elf.h>

int DEBUG = 0;

#define dprint if (DEBUG) fprintf

typedef struct Elf_Info{
  char* file_name;
  int fd;
  struct stat fd_stat;
  void* map_start;
  Elf32_Ehdr* elf_hdr;
  struct Elf_Info* next;
}Elf_Info;

Elf_Info* ELFI;

/* close CURRENT_FD and set it to -1 */
void close_fd(Elf_Info* elfi){
  if(elfi->fd > 0){
    close(elfi->fd);
    elfi->fd = -1;
  }
}

/* clear elf info aloocated memory memory end set its valuse to NULL*/
void clear_elf_info(Elf_Info* elfi){
    elfi->file_name = NULL;
    if(elfi->fd != -1){
      munmap(elfi->map_start, elfi->fd_stat.st_size);
      elfi->map_start = NULL;
      elfi->elf_hdr = NULL;
    }
    close_fd(elfi);
}

/* checkes whether a given Elf32_Ehdr is a valid elf header of a file */
int is_elf(Elf32_Ehdr* elf_hdr){
  return (elf_hdr->e_ident[EI_MAG0] == 0x7f &&
          elf_hdr->e_ident[EI_MAG1] == 'E' &&
          elf_hdr->e_ident[EI_MAG2] == 'L' &&
          elf_hdr->e_ident[EI_MAG3] == 'F');
}

/* prompt for file name open it mapp it to memry and create a corrospnding elf elf info */
int open_and_map_elf(Elf_Info* elfi, char*file_name){
  elfi->file_name = file_name;
  close_fd(elfi);
  dprint(stderr, "opening file: %s\n",elfi->file_name);
  if((elfi->fd = open(elfi->file_name,O_RDONLY)) == -1){
    perror("Error opening file");
    return 0;
  }
  /* get file stat - inorder to later get the file size*/
  if(fstat(elfi->fd, &elfi->fd_stat) != 0) {
    perror("Error stat failed");
    clear_elf_info(elfi);
    return 0;
  }
  /* mapping file end setting map_start to its starting point */
  if ((elfi->map_start = mmap(0, elfi->fd_stat.st_size, PROT_READ, MAP_SHARED, elfi->fd, 0)) == MAP_FAILED) {
    perror("Error mmap failed");
    clear_elf_info(elfi);
    return 0;
  }
  if(!is_elf(elfi->map_start)){
    perror("Error not an ELF file");
    clear_elf_info(elfi);
    return 0;
  }
  /* set elf header and section header */
  elfi->elf_hdr =  (Elf32_Ehdr*)elfi->map_start;
  return 1;
}

/* print program headers */
void print_program_headers(){
  int i;
  int ph_num = ELFI->elf_hdr->e_phnum;
  Elf32_Phdr* p_hdr = ELFI->map_start+ELFI->elf_hdr->e_phoff;

  printf("\nProgram Headers:\n[Nr] Type      Offset    VirtAddr  PhysAddr  FileSiz   MemSiz    Flg  Align\n");
  for(i =0 ; i<ph_num; i++){
    printf("[%2d] %08x  %08x  %08x  %08x  %08x  %08x  %03x  %05x\n", i, p_hdr[i].p_type, p_hdr[i].p_offset, p_hdr[i].p_vaddr, p_hdr[i].p_paddr, p_hdr[i].p_filesz,p_hdr[i].p_memsz, p_hdr[i].p_flags, p_hdr[i].p_align);
  }
}

/* quits the program */
void quit(){
  Elf_Info* elfi = ELFI;
  dprint(stderr,"quitting\n");
    clear_elf_info(elfi);
    free(elfi);
  ELFI = NULL;
	exit(0);
}

/*  Main */
int main(int argc, char **argv){
  ELFI = calloc(1,sizeof(Elf_Info));
  if(argc != 2){
    fprintf(stderr,"Error: wrong args");
    return 1;
  }
  open_and_map_elf(ELFI,argv[1]);
  print_program_headers();
  quit();
	return 0;
}
