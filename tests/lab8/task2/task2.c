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
void* MAP_START; /* will point to the start of the memory mapped file */
Elf32_Ehdr* ELF_HDR;
Elf32_Shdr* S_HDR;
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
int is_elf(Elf32_Ehdr* ELF_HDR){
  return (ELF_HDR->e_ident[EI_MAG0] == 0x7f &&
          ELF_HDR->e_ident[EI_MAG1] == 'E' &&
          ELF_HDR->e_ident[EI_MAG2] == 'L' &&
          ELF_HDR->e_ident[EI_MAG3] == 'F');
}

/* prints some of the elf headers info */
void print_elf_header(Elf32_Ehdr* ELF_HDR){
  printf("\nELF Header:\n");
  printf("Magic numbers:\t\t\t%c %c %c\n",ELF_HDR->e_ident[EI_MAG1],ELF_HDR->e_ident[EI_MAG2],ELF_HDR->e_ident[EI_MAG3]);
  printf("Data encoding:\t\t\t");
  if(ELF_HDR->e_ident[EI_DATA] == 1) printf("2's complement, little endian\n");
  else if(ELF_HDR->e_ident[EI_DATA] == 2) printf("2's complement, big endian\n");
  else printf("Invalid encoding.\n");
  printf("Entry point:\t\t\t%p\n", (void*)ELF_HDR->e_entry);
  printf("Section headers offset:\t\t%d (bytes into file)\n",ELF_HDR->e_shoff);
  printf("Number of section headers:\t%d\n",ELF_HDR->e_shnum);
  printf("Size of section header:\t\t%d (bytes)\n",ELF_HDR->e_shentsize);
  printf("Program headers offset:\t\t%d (bytes into file)\n",ELF_HDR->e_phoff);
  printf("Number of program headers:\t%d\n",ELF_HDR->e_phnum);
  printf("Size of program header:\t\t%d (bytes)\n",ELF_HDR->e_phentsize);
}

/* prints info about a given elf file */
void examine_elf_file(){
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
  ELF_HDR = (Elf32_Ehdr *) MAP_START;
  S_HDR = (Elf32_Shdr*)(MAP_START+ELF_HDR->e_shoff);

  if(!is_elf(ELF_HDR)){
    perror("Error not an ELF file");
    munmap(MAP_START, FD_STAT.st_size);
    close_cfd();
    return;
  }
  /* print elf information */
  print_elf_header(ELF_HDR);
}

/* get sh_type as a string */
char* get_sh_type(int ndx){
  Elf32_Word sh_type = S_HDR[ndx].sh_type;
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

/* retreves the name of the section at index ndx in the section header */
char* get_sh_name(int ndx){
  char* sh_strtab = MAP_START + S_HDR[ELF_HDR->e_shstrndx].sh_offset;
  return (&sh_strtab[S_HDR[ndx].sh_name]);
}

/* print section names and info */
void print_section_names(){
  int i, sh_num;
  if(CURRENT_FD == -1){
    perror("Error opening file");
    return;
  }
  sh_num = ELF_HDR->e_shnum;
  dprint(stderr,"\nDEBUG:\tS_HDR: %p\n\tsh_num: %d\n\tsh_strtab: %p\n", S_HDR, sh_num, MAP_START+ S_HDR[ELF_HDR->e_shstrndx].sh_offset);
  printf("\nSection Headers:\n[Nr] Name\t\tAddr\t\tOff\t\tSize\t\tType\n");
  for(i = 0; i< sh_num; i++){
    printf("[%2d] %-16s\t%08x\t%08x\t%08x\t%s\n", i, get_sh_name(i), S_HDR[i].sh_addr, S_HDR[i].sh_offset, S_HDR[i].sh_size, get_sh_type(i));
  }
}

/* prints a given symbol table */
void print_sym_tab(Elf32_Sym* sym_tab, int sym_num,char* str_tab){
  int i, sym_val, sym_shndx;
  char* sym_name;
  printf("Num: Value     Ndx  Sec_Name          Sym_Name\n");
  for(i = 0; i<sym_num; i++){
    sym_name = str_tab + sym_tab[i].st_name;
    sym_val = sym_tab[i].st_value;
    sym_shndx = sym_tab[i].st_shndx;
    if(sym_shndx == SHN_UNDEF) printf("%3d: %08x  %-3s  %-16s  %s\n",i,sym_val,"UND","UND",sym_name);
    else if(sym_shndx == SHN_ABS) printf("%3d: %08x  %-3s  %-16s  %s\n",i,sym_val,"ABS","ABS",sym_name);
    else printf("%3d: %08x  %3d  %-16s  %-16s\n",i,sym_val,sym_shndx,get_sh_name(sym_shndx),sym_name);
  }
}


/* print symbols and their info */
void print_symbols(){
  Elf32_Sym* sym_tab;
  Elf32_Sym* dyn_sym;
  char* str_tab;
  char* dyn_str_tab;
  int i, sh_num, sym_num, dyn_sym_num;
  if(CURRENT_FD == -1){
    perror("Error opening file");
    return;
  }
  sh_num = ELF_HDR->e_shnum;
  for(i=0; i<sh_num; i++){
    if(S_HDR[i].sh_type == SHT_SYMTAB){
      sym_tab = (Elf32_Sym*)(MAP_START+S_HDR[i].sh_offset);
      sym_num = S_HDR[i].sh_size/sizeof(Elf32_Sym);
      str_tab = MAP_START + S_HDR[S_HDR[i].sh_link].sh_offset;
    }
    else if(S_HDR[i].sh_type == SHT_DYNSYM){
      dyn_sym = (Elf32_Sym*)(MAP_START+S_HDR[i].sh_offset);
      dyn_sym_num = S_HDR[i].sh_size/sizeof(Elf32_Sym);
      dyn_str_tab = MAP_START + S_HDR[S_HDR[i].sh_link].sh_offset;
    }
  }
  dprint(stderr,"\nDebug:\tsym_tab: %p\n\tsym_num: %d\n\tstr_tab: %p\n\tdyn_sym: %p\n\tdyn_sym_num: %d\n\tdyn_str_tab%p\n", sym_tab, sym_num, str_tab, dyn_sym, dyn_sym_num,dyn_str_tab);
  printf("\nSymbol table '.dynsym' contains %d entries:\n",dyn_sym_num);
  print_sym_tab(dyn_sym, dyn_sym_num, dyn_str_tab);
  printf("\nSymbol table '.symtab' contains %d entries:\n",sym_num);
  print_sym_tab(sym_tab, sym_num, str_tab);
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
  dprint(stderr,"\nDebug:\tfile name: %s\n\tcurrent fd: %d\n\tmapping start: %p\n\tELF_HDR: %p\n\tS_HDR: %p\n",FILE_NAME,CURRENT_FD,MAP_START,ELF_HDR,S_HDR);
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
    create_fun_desc("Print Symbols",print_symbols),
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
