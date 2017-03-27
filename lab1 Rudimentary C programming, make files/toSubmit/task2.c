#include <stdio.h>
#include <string.h>

const int SIZE = 30;

void filterH(FILE * OUTPUT){

	char c;
	
	puts("Enter text to filter:");
	while(!feof(stdin)){
		switch(c = fgetc(stdin)){
			case 'h':
			case 'H':
			case '\n':
				fputc(c, OUTPUT);
				break;
			default:
				break;
		}		
	}
	fputc('\n', OUTPUT);
}


void filter(char filters[],FILE * OUTPUT){
	
	char c ,toPrint;
	int i;
	
	puts("Enter text to filter:");
	while(!feof(stdin)){
		toPrint = c = fgetc(stdin);
		if(c == '\n'){
			fputc(toPrint, OUTPUT);
		}
		else{
			for(i =0; i<SIZE && filters[i]; i++){
				if(c == filters[i]){
					fputc(toPrint, OUTPUT);
					break;
				}
			}
		}
	}	
}


int main(int argc, char **argv){
	
	
	FILE * OUTPUT = stdout;
	int i;
	char filters[SIZE];
	char file[SIZE];
	for(i =0; i<30; i++){
		filters[i]=0;
		file[i]=0;
	}

	for (i=1; i<argc; i++){
		if(strcmp(argv[i],"-i")==0){
			FILE * input = fopen(argv[++i],"r");
			fgets(filters, SIZE, input);
			fclose(input);
		}
		else if(strcmp(argv[i],"-o")==0){
			puts("Enter output file:");
			gets(file);
			OUTPUT = fopen(file, "w+");
		}
		else{
			for(i=0; i<SIZE && argv[1][i]; i++){
				filters[i] = argv[1][i];
			}	
		}	
	}

	if(argc > 1){
		filter(filters,OUTPUT);
	}
	else{
		filterH(OUTPUT);
	}
	if(OUTPUT != stdout){
		fclose(OUTPUT);
	}
	return 0;
}
