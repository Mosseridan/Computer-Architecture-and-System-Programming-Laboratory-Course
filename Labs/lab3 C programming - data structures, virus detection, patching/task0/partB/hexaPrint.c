#include <stdio.h>
#include <stdlib.h>

void PrintHex(char buffer[], long length){
  int i;
  for (i=0; i<length; i++){
      printf("%02X ",(unsigned char)buffer[i]);
  }
  printf("\n");
}

int main (int argc, char **argv){
  long length;
  size_t size;
  char* buffer = NULL;
  FILE* file;

  if(argc > 1){
    /* open file */
    file = fopen(*(argv+1), "r");
    /* get file length in bytes*/
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);
    /* allocate memory for buffer */
    buffer = (char *) calloc(length,1);
    if (buffer == NULL){
      printf("Error alocating memory for buffer\n");
      fclose(file);
      return 0;
    }
    /*copy file to buffer*/
    size = fread(buffer, 1, length, file);
    if (size != length){
      printf("Error reading file");
      fclose(file);
      return 0;
    }

    /* print the file in hex */
    PrintHex(buffer, length);
    free(buffer);
  }
  else{
    printf("no file entered\n");
  }
  fclose(file);
  return 0;
}
