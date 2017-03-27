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
int MAX_FILES = 2;
char** FILE_NAMES;
int* FDS;
struct stat* FD_STATS; /* this is needed inorder to get the size of the file */
void** MAP_STARTS; /* will point to the start of the memory mapped file */

char* FILE_NAME = NULL;
int CURRENT_FD = -1;
struct stat FD_STAT;
void* MAP_START;
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
void close_fd(int fd){
  if(fd != -1){
    close(fd);
    fd = -1;
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

/* prompt for FILE_NAME save it in buff*/
void set_file_name(char* file_name){
  printf("Please enter <file name>\n> ");
  if(fgets(file_name,FILE_NAME_SIZE,stdin) == NULL){
      perror("error reading from user");
  }
  file_name[strlen(file_name)-1] = '\0';
  dprint(stderr, "Debug: file name set to %s",file_name);
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
  printf("\nELF Header:\n");
  printf("Magic numbers:\t\t\t%c %c %c\n",elf_hdr->e_ident[EI_MAG1],elf_hdr->e_ident[EI_MAG2],elf_hdr->e_ident[EI_MAG3]);
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

int open_and_map_elf(int ndx){
  set_file_name(FILE_NAMES[ndx]);
  close_fd(FDS[ndx]);
  dprint(stderr, "opening file: %s\n",FILE_NAMES[ndx]);
  if((FDS[ndx] = open(FILE_NAMES[ndx],O_RDONLY)) == -1){
    perror("Error opening file");
    return 0;
  }
  /* get file stat - inorder to later get the file size*/
  if(fstat(FDS[ndx], &FD_STATS[ndx]) != 0) {
    perror("Error stat failed");
    close_fd(FDS[ndx]);
    return 0;
  }
  /* mapping file end setting map_start to its starting point */
  if ((MAP_STARTS[ndx] = mmap(0, FD_STATS[ndx].st_size, PROT_READ, MAP_SHARED, FDS[ndx], 0)) == MAP_FAILED) {
    perror("Error mmap failed");
    close_fd(FDS[ndx]);
    return 0;
  }

  if(!is_elf(MAP_STARTS[ndx])){
    perror("Error not an ELF file");
    munmap(MAP_STARTS[ndx], FD_STATS[ndx].st_size);
    close_fd(FDS[ndx]);
    return 0;
  }
  return 1;
}

/* prints info about a given elf file */
void examine_elf_file(){
  if(open_and_map_elf(0) != -1){
    FILE_NAME = FILE_NAMES[0];
    CURRENT_FD = FDS[0];
    FD_STAT = FD_STATS[0];
    MAP_START = MAP_STARTS[0];
    ELF_HDR = (Elf32_Ehdr*)MAP_STARTS[0];
    S_HDR = (Elf32_Shdr*)(ELF_HDR+ELF_HDR->e_shoff); 
    /* print elf information */
    print_elf_header(MAP_STARTS[0]);
  }
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

int get_sym_tab_ndx(Elf32_Ehdr* elf_hdr, Elf32_Word sh_type){
  Elf32_Shdr* s_hdr =(Elf32_Shdr*)(elf_hdr+elf_hdr->e_shoff);
  int i, sh_num = elf_hdr->e_shnum;
  for(i=0; i<sh_num; i++){
    if(s_hdr[i].sh_type == sh_type){
       return i;
    }
  }
  return 0;
}

/* print section names and info */
void print_section_names(){
  int i, sh_num;
  if(CURRENT_FD == -1){
    perror("Error file not set");
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
  int sym_tab_ndx, sym_num, dyn_sym_ndx,dyn_sym_num;
  if(CURRENT_FD == -1){
    perror("Error file not set");
    return;
  }

  sym_tab_ndx = get_sym_tab_ndx(ELF_HDR, SHT_SYMTAB);
  sym_tab = (Elf32_Sym*)(MAP_START+S_HDR[sym_tab_ndx].sh_offset);
  sym_num = S_HDR[sym_tab_ndx].sh_size/sizeof(Elf32_Sym);
  str_tab = MAP_START + S_HDR[S_HDR[sym_tab_ndx].sh_link].sh_offset;

  dyn_sym_ndx = get_sym_tab_ndx(ELF_HDR, SHT_SYMTAB);
  dyn_sym = (Elf32_Sym*)(MAP_START+S_HDR[dyn_sym_ndx].sh_offset);
  dyn_sym_num = S_HDR[dyn_sym_ndx].sh_size/sizeof(Elf32_Sym);
  dyn_str_tab = MAP_START + S_HDR[S_HDR[dyn_sym_ndx].sh_link].sh_offset;
  dprint(stderr,"\nDebug:\tsym_tab: %p\n\tsym_num: %d\n\tstr_tab: %p\n\tdyn_sym: %p\n\tdyn_sym_num: %d\n\tdyn_str_tab%p\n", sym_tab, sym_num, str_tab, dyn_sym, dyn_sym_num,dyn_str_tab);
  printf("\nSymbol table '.dynsym' contains %d entries:\n",dyn_sym_num);
  print_sym_tab(dyn_sym, dyn_sym_num, dyn_str_tab);
  printf("\nSymbol table '.symtab' contains %d entries:\n",sym_num);
  print_sym_tab(sym_tab, sym_num, str_tab);
}

int lookup_symbol(Elf32_Ehdr* elf_hdr, char* symbol){
  Elf32_Shdr* s_hdr =(Elf32_Shdr*)(elf_hdr+elf_hdr->e_shoff);
  Elf32_Sym* sym_tab;
  char* str_tab;
  int i,sym_tab_ndx , sym_num;
  sym_tab_ndx = get_sym_tab_ndx(elf_hdr, SHT_SYMTAB);
  sym_tab = (Elf32_Sym*)(elf_hdr+S_HDR[sym_tab_ndx].sh_offset);
  sym_num = s_hdr[sym_tab_ndx].sh_size/sizeof(Elf32_Sym);
  str_tab = (void*)elf_hdr + s_hdr[s_hdr[sym_tab_ndx].sh_link].sh_offset;
  for(i = 0; i < sym_num; i++){
    if(strcmp(str_tab + sym_tab[i].st_name, symbol) == 0){
      return i;
    }
  }
  return 0;
}

void start_check(Elf32_Ehdr* elf1, Elf32_Ehdr* elf2){
  if(lookup_symbol(elf1,"_start") || lookup_symbol(elf2,"_start")) printf ("_start check: PASSED");
  else printf ("_start check: FAILED");
}

/* checks whether the current ELF file can be linked to another ELF file */
void link_to(){
  Elf32_Ehdr* elf1;
  Elf32_Ehdr* elf2;
  if(CURRENT_FD == -1){
    perror("Error file not set");
    return;
  }
  if (open_and_map_elf(1) == -1){
    return;
  }
  elf1 = (Elf32_Ehdr*)MAP_STARTS[0];
  elf2 = (Elf32_Ehdr*)MAP_STARTS[1];
  start_check(elf1, elf2);


}

/* quits the program */
void quit(){
  int i;
  dprint(stderr,"quitting\n");
  free(INPUT_BUFF);
  for(i=0; i<MAX_FILES; i++){
    free(FILE_NAMES);
    munmap(MAP_STARTS[i], FD_STATS[i].st_size);
    close_fd(FDS[i]);
  }
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
  int input, i;
  fun_desc fdarray[] = {
    create_fun_desc("Toggle Debug Mode",toggle_debug),
    create_fun_desc("Examine ELF File",examine_elf_file),
    create_fun_desc("Print Section Names",print_section_names),
    create_fun_desc("Print Symbols",print_symbols),
    create_fun_desc("Link to",link_to),
    create_fun_desc("Quit",quit),
		create_fun_desc(NULL,NULL),
	};

  INPUT_BUFF = calloc(INPUT_BUFF_SIZE,1);

  FILE_NAMES = calloc(MAX_FILES,sizeof(char**));

  FDS = calloc(MAX_FILES,sizeof(int));
  FD_STATS = calloc(MAX_FILES,sizeof(struct stat));
  MAP_STARTS = calloc(MAX_FILES,sizeof(struct stat));

  for(i=0;i<MAX_FILES;i++){
    FILE_NAMES[i] = calloc(MAX_FILES,sizeof(char*));
  }

  FILE_NAME = FILE_NAMES[0];
  CURRENT_FD = -1;

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
