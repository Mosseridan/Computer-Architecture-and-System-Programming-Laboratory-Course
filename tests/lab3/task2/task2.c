#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct virus virus;
typedef struct link link;

struct virus{
	unsigned short length;
	char name[16];
	char signature[];
};

struct link {
	virus *v;
	link *next;
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

void printSignature(virus* vir){
	printf("Virus name: %s\nVirus size: %hu\nsignature:\n",vir->name,vir->length);
	PrintHex(vir->signature,(vir->length));
	printf("\n");
}

/* Print the data of every link in list. Each item followed by a newline character. */
void list_print(link *virus_list){
	while(virus_list != NULL){
		printSignature(virus_list->v);
		virus_list = virus_list->next;
	}
}

/* Add a new link with the given data to the list
(either at the end or the beginning, depending on what your TA tells you),
and return a pointer to the list (i.e., the first link in the list).
If the list is null - create a new entry and return a pointer to the entry. */
link* list_append(link* virus_list, virus* data){
	link* lk = calloc(sizeof(link),1);
	lk->v = data;
	lk->next = NULL;
	link* current = NULL;
	if(virus_list == NULL){
		return lk;
	}
	else{
		current = virus_list;
		/* go to end of  list */
		while(current->next != NULL){
			current = current->next;
		}
		/*set the last link of virus list to be lk */
		current->next = lk;
		return virus_list;
	}
}

/* Free the memory allocated by the list. */
void list_free(link *virus_list){
	if(virus_list->next != NULL){
		list_free(virus_list->next);
	}
	free(virus_list->v);
	free(virus_list);
}

void detect_virus(char *buffer, link *virus_list, unsigned int size){
	int i,j, result;
	const char NOP = 144;
	link* current = NULL;
	virus* vir = NULL;
	for(i = 0 ; i < size; i++){
		current = virus_list;
		while(current != NULL){
			vir = current->v;
			if(vir != NULL && vir->length <= size-i){
				result = memcmp(buffer+i, vir->signature, vir->length);
				if(result == 0){
					printf("Virus found!:\n"
					"At starting Byte: DEC - %d, HEX - %02X\n"
					"Virus Name: %s\n"
					"Virus signature size: %d\n\n", i, i,vir->name, vir->length);

					/*delete virus */
					fseek(file, i, SEEK_SET);
					for (j=0; j < vir->length; j++){
							fwrite(&NOP, sizeof(char), 1, file);
					}
				}
			}
			current = current->next;
		}
	}
}

int main (int argc, char** argv){
	virus* vir;
	link* virus_list = NULL;
	char* buffer = calloc(sizeof(char),10000);
	if(buffer == NULL){
		printf("Error alocating memory for buffer");
		return 0;
	}
	int size = 10000;
	/* open file */
	file = fopen("signatures", "r");
	while((vir = getSignature()) != NULL){
		virus_list = list_append(virus_list,vir);
	}

	link* toPrint = virus_list;
	while(toPrint != NULL){
		printSignature(toPrint->v);
		toPrint = toPrint->next;
	}

	/*open infected */
	fclose(file);
	file = fopen("infected", "r+");
	/* get file length in bytes*/
	fseek(file, 0, SEEK_END);
	if(ftell(file) < size) size = ftell(file);
	rewind(file);
	/*fill buffer*/
	if(fread(buffer,1,size, file) == 0){
		printf("Error reading file to buffer");
	}
	else{
		detect_virus(buffer, virus_list, size);
	}
	list_free(virus_list);
	free(buffer);
	fclose(file);
	return 0;
}
