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
int FILE_NAME_SIZE = 101;
int INPUT_BUFF_SIZE = 101;
char* INPUT_BUFF = NULL;
char* FILE_NAME = NULL;
int CURRENT_FD = -1;
struct stat FD_STAT; /* this is needed inorder to get the size of the file */
void *MAP_START; /* will point to the start of the memory mapped file */
#define dprint if (DEBUG) fprintf

/* define struct fun_desc*/
typedef struct{
  char *name;
  void (*fun)();
}fun_desc;

/* Gets a printer to a char (string) and aprinter to a function,
   and returns a fun_desc with these ointers as its elements. */
fun_desc create_fun_desc(char* name, void (*fun)()){
  fun_desc fd;
  fd.name = name;
  fd.fun = fun;
  return fd;
}

/* get onput from user as a string */
char* get_input(){
  char* input = fgets(INPUT_BUFF,INPUT_BUFF_SIZE,stdin);
  if(input == NULL){
    perror("Error: read error");
  }
  return input;
}

/* get input from user as an int between 0-9 returns the number or -1 on fail*/
int get_input_int(){
  char* input = get_input();
  int int_input;
  if(input != NULL && sscanf(input, "%d", &int_input) == 1){
      return int_input;
  }
  printf("Error: expected number <0-9> but given %s",input);
  return -1;
}

/* close CURRENT_FD and set it to -1 */
void close_cfd(){
  if(CURRENT_FD != -1){
    close(CURRENT_FD);
    CURRENT_FD = -1;
  }
}

/* toogle DEBUG FLAG on and of */
void toggle_debug(){
  if(DEBUG){
    DEBUG = 0;
    fprintf(stderr, "Debug flag now off\n");
  }
  else{
    DEBUG = 1;
    fprintf(stderr, "Debug flag now on\n");
  }
}

/* set  FILE_NAME */
void set_file_name(){
  printf("Please enter <file name>\n> ");
  if(fgets(FILE_NAME,FILE_NAME_SIZE,stdin) == NULL){
      perror("error reading from user");
  }
  FILE_NAME[strlen(FILE_NAME)-1] = '\0';
  dprint(stderr, "Debug: file name set to %s",FILE_NAME);
}

/* checkes whether a given Elf32_Ehdr is a valid elf header of a file */
int is_elf(Elf32_Ehdr* elf_hdr){
  return (elf_hdr->e_ident[EI_MAG0] == 0x7f &&
          elf_hdr->e_ident[EI_MAG1] == 'E' &&
          elf_hdr->e_ident[EI_MAG2] == 'L' &&
          elf_hdr->e_ident[EI_MAG3] == 'F');
}

/* prints some of the elf headers info */
void print_elf_header(Elf32_Ehdr* elf_hdr){
  printf("\nMagic numbers:\t\t\t%c %c %c\n",elf_hdr->e_ident[EI_MAG1],elf_hdr->e_ident[EI_MAG2],elf_hdr->e_ident[EI_MAG3]);
  printf("Data encoding:\t\t\t");
  if(elf_hdr->e_ident[EI_DATA] == 1) printf("2's complement, little endian\n");
  else if(elf_hdr->e_ident[EI_DATA] == 2) printf("2's complement, big endian\n");
  else printf("Invalid encoding.\n");
  printf("Entry point:\t\t\t%p\n", (void*)elf_hdr->e_entry);
  printf("Section headers offset:\t\t%d (bytes into file)\n",elf_hdr->e_shoff);
  printf("Number of section headers:\t%d\n",elf_hdr->e_shnum);
  printf("Size of section header:\t\t%d (bytes)\n",elf_hdr->e_shentsize);
  printf("Program headers offset:\t\t%d (bytes into file)\n",elf_hdr->e_phoff);
  printf("Number of program headers:\t%d\n",elf_hdr->e_phnum);
  printf("Size of program header:\t\t%d (bytes)\n",elf_hdr->e_phentsize);
}

/* prints info about a given elf file */
void examine_elf_file(){
  Elf32_Ehdr *elf_hdr; /* this will point to the header structure */

  set_file_name();
  close_cfd();
  dprint(stderr, "opening file: %s\n",FILE_NAME);
  if((CURRENT_FD = open(FILE_NAME,O_RDONLY)) == -1){
    perror("Error opening file");
    return;
  }
  /* get file stat - inorder to later get the file size*/
  if(fstat(CURRENT_FD, &FD_STAT) != 0) {
    perror("Error stat failed");
    close_cfd();
    return;
  }
  /* mapping file end setting map_start to its starting point */
  if ((MAP_START = mmap(0, FD_STAT.st_size, PROT_READ, MAP_SHARED, CURRENT_FD, 0)) == MAP_FAILED) {
    perror("Error mmap failed");
    close_cfd();
    return;
  }
  /* seting elf_header to the to point to the files elf header (which is its starting point)*/
  elf_hdr = (Elf32_Ehdr *) MAP_START;
  if(!is_elf(elf_hdr)){
    perror("Error not an ELF file");
    munmap(MAP_START, FD_STAT.st_size);
    close_cfd();
    return;
  }
  /* print elf information */
  print_elf_header(elf_hdr);
}

char* sh_type_str(uint32_t  sh_type){
  char* sh_types_arr[] = {"NULL","PROGBITS","SYMTAB","STRTAB","RELA","HASH",
                         "DYNAMIC","NOTE","NOBITS","REL","SHLIB","DYNSYM",};
  if(sh_type >= 0 && sh_type <= 11){
    return sh_types_arr[sh_type];
  }
  else if(sh_type == 0x70000000) return "LOPROC";
  else if(sh_type == 0x7fffffff) return "HIPROC";
  else if(sh_type == 0x80000000) return "LOUSER";
  else if(sh_type == 0xffffffff) return "HIUSER";
  else return "INVALID";
}

void print_section_names(){
  Elf32_Ehdr* elf_hdr; /* will point to the elf header structure */
  Elf32_Shdr* s_hdr; /* will point to the section header structure */
  int sh_num, i;
  char* sh_strtab;
  if(CURRENT_FD == -1){
    perror("Error opening file");
    return;
  }

  elf_hdr = (Elf32_Ehdr *)MAP_START;
  sh_num = elf_hdr->e_shnum;
  s_hdr = (Elf32_Shdr *)(MAP_START+elf_hdr->e_shoff);
  sh_strtab = (char*)(MAP_START+ (s_hdr[elf_hdr->e_shstrndx]).sh_offset);

  dprint(stderr,"\nDEBUG: elf_hdr: %p\n\tsh_num: %d\n\t s_hdr: %p\n\tsh_strtab: %p\n",elf_hdr,sh_num,s_hdr,sh_strtab);
  printf("\n[Nr] Name\t\tAddr\t\tOff\t\tSize\t\tType\n");
  for(i = 0; i< sh_num; i++){
    printf("[%2d] %-16s\t%08x\t%08x\t%08x\t%s\n", i, &sh_strtab[s_hdr[i].sh_name], s_hdr[i].sh_addr, s_hdr[i].sh_offset, s_hdr[i].sh_size, sh_type_str(s_hdr[i].sh_type));
  }

}
/* quits the program */
void quit(){
  dprint(stderr,"quitting\n");
  free(FILE_NAME);
  free(INPUT_BUFF);
  munmap(MAP_START, FD_STAT.st_size);
  close_cfd();
	exit(0);
}

/* prompts the menu to screen */
void prompt_menu(fun_desc* fdarray){
  int i;
  dprint(stderr,"\nDebug:\tfile name: %s\n\tcurrent fd: %d\n\tmapping start: %p\n",FILE_NAME,CURRENT_FD,MAP_START);
  printf("\nChoose action:\n");
  for(i =0 ; fdarray[i].name != NULL ; i++){
    printf(" %i-%s\n", i, fdarray[i].name);
  }
  printf("> ");
}

/* determins wether the given index is within the menues bounds */
int valid_index(unsigned int index, fun_desc* fdarray){
  int i;
  for (i = 0; i<=index ;i++){
    if(fdarray[i].name == NULL){
      fprintf(stderr,"Error: index out of bounds: %d\n",index);
      return 0;
    }
  }
  return 1;
}

/*  Main */
int main(int argc, char **argv){
  int input;
  fun_desc fdarray[] = {
    create_fun_desc("Toggle Debug Mode",toggle_debug),
    create_fun_desc("Examine ELF File",examine_elf_file),
    create_fun_desc("Print Section Names",print_section_names),
    create_fun_desc("Quit",quit),
		create_fun_desc(NULL,NULL),
	};

  FILE_NAME = calloc(FILE_NAME_SIZE,1);
  INPUT_BUFF = calloc(INPUT_BUFF_SIZE,1);

  while (1){
    prompt_menu(fdarray);
    input = get_input_int();
    if(input >= 0 && valid_index(input, fdarray)){
      fdarray[input].fun();
    }
	}
  quit();
	return 0;
}
