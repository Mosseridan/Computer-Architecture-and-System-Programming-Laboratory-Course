#include <stdio.h>
#include <stdlib.h>

typedef struct Virus Virus;
typedef struct Link Link;

struct Virus {
    unsigned short length;
    char name[16];
    char signature[];
};

struct Link {
    Virus *v;
    Link *next;
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

/* Print the data of every link in list. Each item followed by a newline character. */
void list_print(Link *virus_list){
	Virus* virus;
	while(virus_list != NULL){
		if((virus = virus_list->v) != NULL){
			PrintVirus(virus);
			virus_list = virus_list->next;
		}
	}
}
     
/* 
 * Add a new link with the given data to the list and a pointer to the last element of a list
 * and appends  a new link with data at the end of the list, and return a pointer to the list (i.e., the first link in the list).
 * If the list is null - create a new entry and return a pointer to the entry. 
 */    
Link* list_append(Link* virus_list, Virus* data){
	Link* link = malloc(sizeof(Link));
	link->v = data;
	link->next = NULL;
	if(virus_list != NULL){
		virus_list->next = link;
	}
	return link;
}

/* Free the memory allocated by the list. */   
void list_free(Link *virus_list){
	if(virus_list->next != NULL){
		list_free(virus_list->next);
	}
	free(virus_list->v);
	free(virus_list);
}

int main(int argc, char** argv){
	FILE* file = fopen("signatures","r");
	Virus* virus = NULL;
	Link* virus_list = NULL;
	Link* tail = NULL;

	while((virus = getSignature(file)) != NULL){
		if(virus_list == NULL){
			virus_list = list_append(virus_list, virus);
			tail = virus_list;
		}
		else{
			tail = list_append(tail, virus);
		}
	}	

	list_print(virus_list);

	list_free(virus_list);
	tail = NULL;
	fclose(file);
	return 0;
}
