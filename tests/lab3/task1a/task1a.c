#include <stdio.h>
#include <stdlib.h>

typedef struct virus virus;

struct virus{
  unsigned short length;
  char name[16];
  char signature[];
};

FILE* file = NULL;

void quit(char* message, int status){
  printf("%s\n", message);
  /*if (file != NULL){
    fclose(file);
  }*/
  exit(status);
}

/* reads a binary file of virus signatures in to a "struct virus" and returns a pointer to it */
virus* getSignature(){
  const int LEN_SIZE = 2, LEN_AND_NAME_SIZE = 18;
  size_t result;
  virus* vir = NULL;
  char len[2];
  unsigned short length;
  /* get length and switch from big indian */
  result = fread(len, LEN_SIZE, 1, file);
  if(result != 1){
    printf("Error reading file\n");
    return NULL;
  }
  length = len[1]+256*len[0];
  /* allocate space for vir */
  vir = calloc(1,length);
  if (vir == NULL) {
    printf("Error allocating memory for struct virus\n");
    return NULL;
  }
  /* import rest of vir from file */
  vir->length = (length - LEN_AND_NAME_SIZE);
  result = fread(&(vir->name), sizeof(char), length - LEN_SIZE, file);
  if(result != length - LEN_SIZE){
    printf("Error reading file\n");
    return NULL;
  }
  return vir;
}

void PrintHex(char buffer[], long length){
  int i;
  for (i=0; i<length; i++){
    printf("%02X ",(unsigned char)buffer[i]);
    if (i%20==19) printf("\n");
  }
  printf("\n");
}

int main (int argc, char** argv){
  file = fopen("signatures", "r");
  virus* vir;
  while((vir = getSignature()) != NULL){
    printf("Virus name: %s\nVirus size: %hu\nsignature:\n",vir->name,vir->length);
    PrintHex(vir->signature,(vir->length));
    printf("\n");
  }
  fclose(file);
  return 0;
}
