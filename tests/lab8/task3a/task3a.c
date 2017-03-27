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

#define dprint if (DEBUG) fprintf

/* define struct fun_desc*/
typedef struct{
  char *name;
  void (*fun)();
}fun_desc;

typedef struct Elf_Info{
  char* file_name;
  int fd;
  struct stat fd_stat;
  void* map_start;
  Elf32_Ehdr* elf_hdr;
  Elf32_Shdr* s_hdr;
  Elf32_Sym* sym_tab;
  int sym_num;
  char* str_tab;
  Elf32_Sym* dyn_sym_tab;
  int dyn_sym_num;
  char* dyn_str_tab;
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
    free(elfi->file_name);
    if(elfi->map_start != NULL){
      munmap(elfi->map_start, elfi->fd_stat.st_size);
      elfi->map_start = NULL;
      elfi->elf_hdr = NULL;
      elfi->s_hdr = NULL;
      elfi->sym_tab = NULL;
      elfi->sym_num = 0;
      elfi->str_tab = NULL;
      elfi->dyn_sym_tab = NULL;
      elfi->dyn_sym_num = 0;
      elfi->dyn_str_tab = NULL;
      elfi->next = NULL;
    }
    close_fd(elfi);
}

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
void set_file_name(Elf_Info* elfi){
  printf("Please enter <file name>\n> ");
  if (elfi->file_name == NULL){
    elfi->file_name = calloc(FILE_NAME_SIZE,1);
  }
  if(fgets(elfi->file_name,FILE_NAME_SIZE,stdin) == NULL){
      perror("error reading from user");
  }
  elfi->file_name[strlen(elfi->file_name)-1] = '\0';
  dprint(stderr, "Debug: file name set to %s\n",elfi->file_name);
}

/* checkes whether a given Elf32_Ehdr is a valid elf header of a file */
int is_elf(Elf32_Ehdr* elf_hdr){
  return (elf_hdr->e_ident[EI_MAG0] == 0x7f &&
          elf_hdr->e_ident[EI_MAG1] == 'E' &&
          elf_hdr->e_ident[EI_MAG2] == 'L' &&
          elf_hdr->e_ident[EI_MAG3] == 'F');
}

/* print all elf infos chearted (if debug flaf is on) */
void dprint_elfis(){
  int i;
  Elf_Info* elfi = ELFI;
  if(DEBUG == 1){
    printf("\nDebug:\n");
    for(i=0; elfi != NULL; elfi = elfi->next, i++){
        printf("Elf_Info #%d:\n",i);
        printf("\tfile_name: %s\n",elfi->file_name);
        printf("\tfd: %d\n",elfi->fd);
        printf("\tmap_start: %p\n",elfi->map_start);
        printf("\telf_hdr: %p\n",elfi->elf_hdr);
        printf("\ts_hdr: %p\n",elfi->s_hdr);
        printf("\tsym_tab: %p\n",elfi->sym_tab);
        printf("\tsym_num: %d\n",elfi->sym_num);
        printf("\tstr_tab: %p\n",elfi->str_tab);
        printf("\tdyn_sym_tab: %p\n",elfi->dyn_sym_tab);
        printf("\tdyn_sym_num: %d\n",elfi->dyn_sym_num);
        printf("\tdyn_str_tab: %p\n",elfi->dyn_str_tab);
    }
  }
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

/* get the index int to section table of the sh_type symbol table of a given elf info */
int get_sym_tab_ndx(Elf_Info* elfi, Elf32_Word sh_type){
  int i, sh_num = elfi->elf_hdr->e_shnum;
  for(i=0; i<sh_num; i++){
    if(elfi->s_hdr[i].sh_type == sh_type){
       return i;
    }
  }
  return 0;
}

/* prompt for file name open it mapp it to memry and create a corrospnding elf elf info */
int open_and_map_elf(Elf_Info* elfi){
  int sym_tab_ndx, dyn_sym_tab_ndx;
  set_file_name(elfi);
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
  elfi->s_hdr = (Elf32_Shdr*)(elfi->map_start + elfi->elf_hdr->e_shoff);
  /* set symbol table, string table and its size */
  sym_tab_ndx = get_sym_tab_ndx(elfi, SHT_SYMTAB);
  elfi->sym_tab = (Elf32_Sym*)(elfi->map_start + elfi->s_hdr[sym_tab_ndx].sh_offset);
  elfi->sym_num = elfi->s_hdr[sym_tab_ndx].sh_size/sizeof(Elf32_Sym);
  elfi->str_tab = elfi->map_start + elfi->s_hdr[elfi->s_hdr[sym_tab_ndx].sh_link].sh_offset;
  /* set dynamic symbol table, string table and its size */
  dyn_sym_tab_ndx = get_sym_tab_ndx(elfi, SHT_DYNSYM);
  elfi->dyn_sym_tab = (Elf32_Sym*)(elfi->map_start + elfi->s_hdr[dyn_sym_tab_ndx].sh_offset);
  elfi->dyn_sym_num = elfi->s_hdr[dyn_sym_tab_ndx].sh_size/sizeof(Elf32_Sym);
  elfi->dyn_str_tab = elfi->map_start + elfi->s_hdr[elfi->s_hdr[dyn_sym_tab_ndx].sh_link].sh_offset;
  return 1;
}

/* prints info about a given elf file */
void examine_elf_file(){
  if(open_and_map_elf(ELFI)){
    /* print elf information */
    print_elf_header(ELFI->elf_hdr);
  }
}

/* get sh_type as a string */
char* get_sh_type(int ndx, Elf_Info* elfi){
  Elf32_Word sh_type = elfi->s_hdr[ndx].sh_type;
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
char* get_sh_name(int ndx,Elf_Info* elfi){
  char* sh_strtab = elfi->map_start + elfi->s_hdr[elfi->elf_hdr->e_shstrndx].sh_offset;
  return (&sh_strtab[elfi->s_hdr[ndx].sh_name]);
}

/* print section names and info */
void print_section_names(){
  int i, sh_num;
  if(ELFI->fd == -1){
    perror("Error file not set");
    return;
  }
  sh_num = ELFI->elf_hdr->e_shnum;
  dprint(stderr,"\nDEBUG:\tS_HDR: %p\n\tsh_num: %d\n\tsh_strtab: %p\n", ELFI->s_hdr, sh_num, ELFI->map_start + ELFI->s_hdr[ELFI->elf_hdr->e_shstrndx].sh_offset);
  printf("\nSection Headers:\n[Nr] Name\t\tAddr\t\tOff\t\tSize\t\tType\n");
  for(i = 0; i< sh_num; i++){
    printf("[%2d] %-16s\t%08x\t%08x\t%08x\t%s\n", i, get_sh_name(i,ELFI), ELFI->s_hdr[i].sh_addr, ELFI->s_hdr[i].sh_offset, ELFI->s_hdr[i].sh_size, get_sh_type(i,ELFI));
  }
}

/* prints a given symbol table */
void print_sym_tab(Elf_Info* elfi ,Elf32_Word sh_type){
  int i, sym_val, sym_num, sym_shndx;
  Elf32_Sym* sym_tab;
  char* str_tab;
  char* sym_name;
  if(sh_type == SHT_SYMTAB){
    sym_tab = elfi->sym_tab;
    sym_num = elfi->sym_num;
    str_tab = elfi->str_tab;
  }
  else if(sh_type == SHT_DYNSYM){
    sym_tab = elfi->dyn_sym_tab;
    sym_num = elfi->dyn_sym_num;
    str_tab = elfi->dyn_str_tab;
  }
  else{
    fprintf(stderr,"Error given wrong sh_type\n");
    return;
  }
  printf("Num: Value     Ndx  Sec_Name          Sym_Name\n");
  for(i = 0; i<sym_num; i++){
    sym_name = str_tab + sym_tab[i].st_name;
    sym_val = sym_tab[i].st_value;
    sym_shndx = sym_tab[i].st_shndx;
    if(sym_shndx == SHN_UNDEF) printf("%3d: %08x  %-3s  %-16s  %s\n",i,sym_val,"UND","UND",sym_name);
    else if(sym_shndx == SHN_ABS) printf("%3d: %08x  %-3s  %-16s  %s\n",i,sym_val,"ABS","ABS",sym_name);
    else printf("%3d: %08x  %3d  %-16s  %-16s\n",i,sym_val,sym_shndx,get_sh_name(sym_shndx, elfi),sym_name);
  }
}

/* print symbols and their info */
void print_symbols(){
  if(ELFI->fd == -1){
    perror("Error file not set");
    return;
  }
  dprint(stderr,"\nDebug:\tsym_tab: %p\n\tsym_num: %d\n\tstr_tab: %p\n\tdyn_sym: %p\n\tdyn_sym_num: %d\n\tdyn_str_tab%p\n", ELFI->sym_tab, ELFI->sym_num, ELFI->str_tab, ELFI->dyn_sym_tab, ELFI->dyn_sym_num, ELFI->dyn_str_tab);
  printf("\nSymbol table '.dynsym' contains %d entries:\n",ELFI->dyn_sym_num);
  print_sym_tab(ELFI, SHT_DYNSYM);
  printf("\nSymbol table '.symtab' contains %d entries:\n",ELFI->sym_num);
  print_sym_tab(ELFI, SHT_SYMTAB);
}

/* lookup a given symbol in the given elfi */
int lookup_symbol(Elf_Info* elfi, char* symbol){
    int i;
    for(i = 0; i < elfi->sym_num; i++){
    if(strcmp(elfi->str_tab + elfi->sym_tab[i].st_name, symbol) == 0){
      return i;
    }
  }
  return 0;
}

/* determines whether the _start symbol is defined in one of the given files */
void start_check(Elf_Info* elfi){
  while(elfi != NULL){
    if(lookup_symbol(elfi,"_start")){
      printf ("\t_start check: PASSED\n");
      return;
    }
    elfi = elfi->next;
  }
  printf ("\t_start check: FAILED\n");
}

/* checks whether the current ELF file can be linked to another ELF file */
void link_to(){
  if(ELFI == NULL || ELFI->fd == -1){
    perror("Error file not set");
    return;
  }
  ELFI->next = calloc(1,sizeof(Elf_Info));
  if (open_and_map_elf(ELFI->next) == -1){
    return;
  }
  printf("\nTesting files:\n");
  start_check(ELFI);
}

/* quits the program */
void quit(){
  Elf_Info* elfi = ELFI;
  Elf_Info* next;
  dprint(stderr,"quitting\n");
  free(INPUT_BUFF);
  while(elfi != NULL){
    next = elfi->next;
    clear_elf_info(elfi);
    elfi = next;
  }
  ELFI = NULL;
	exit(0);
}

/* prompts the menu to screen */
void prompt_menu(fun_desc* fdarray){
  int i;
  dprint_elfis();
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
    create_fun_desc("Link to",link_to),
    create_fun_desc("Quit",quit),
		create_fun_desc(NULL,NULL),
	};

  INPUT_BUFF = calloc(INPUT_BUFF_SIZE,1);
  ELFI = calloc(1,sizeof(Elf_Info));

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
