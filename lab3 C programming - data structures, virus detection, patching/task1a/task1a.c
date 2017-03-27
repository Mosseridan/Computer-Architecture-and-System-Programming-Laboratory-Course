#include <stdio.h>
#include <stdlib.h>

typedef struct Virus Virus;
 
struct Virus {
    unsigned short length;
    char name[16];
    char signature[];
};


void PrintHex(char buffer[], long length){
  int i;
  for (i=0; i<length; i++){
      printf("%02X ",(unsigned char)buffer[i]);
      if(i%20 == 19){
      	printf("\n");
      }
  }
  printf("\n");
}

void PrintVirus(Virus* virus){
	printf("Virus name: %s\n"
			"Virus size: %hu\n"
			"Signature:\n",virus->name,virus->length);
    PrintHex(virus->signature,(virus->length));
    printf("\n");
}

Virus* getSignature(FILE* file){
	const int LEN_SIZE = 2, LEN_AND_NAME_SIZE = 18;
	char buffer[2];
	unsigned short length = 0;
	Virus* virus;

	if(file == NULL){
		return NULL;
	}
	/* get signature length and convert from big indian */
	if(fread(buffer, LEN_SIZE, 1, file) != 1){
		return NULL;
	}

	length = buffer[1]+256*buffer[0];
	virus = calloc (1, length);
	if (virus == NULL) {
    	printf("Error allocating memory for struct virus\n");
    	return NULL;
  	}
	virus->length = (length - LEN_AND_NAME_SIZE);
	if(fread(&(virus->name), 1, length - LEN_SIZE, file) != length - LEN_SIZE){
		printf("Error reading file\n");
    	return NULL;
	}
	return virus;
}

int main(int argc, char** argv){
	FILE* file = fopen("signatures","r");
	Virus* virus = NULL;
	while((virus = getSignature(file)) != NULL){
		PrintVirus(virus);
		free(virus);
	}
	fclose(file);
	return 0;
}
