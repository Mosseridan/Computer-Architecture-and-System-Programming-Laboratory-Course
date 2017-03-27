#include <stdio.h>
#include <string.h>

int SIZE = 30;

void filterH(){

	char c;
	
	while(!feof(stdin)){
		switch(c = fgetc(stdin)){
			case 'h':
			case 'H':
			case '\n':
				fputc(c, stdout);
				break;
			default:
				break;
		}		
	}
	fputc('\n', stdout);
}

void filter(char filters[]){
	
	char c ,toPrint;
	int i;
	
	while(!feof(stdin)){
		toPrint = c = fgetc(stdin);
		if(c == '\n'){
			fputc(toPrint, stdout);
		}
		else{
			for(i =0; i<SIZE && filters[i]; i++){
				if(c == filters[i]){
					fputc(toPrint, stdout);
					break;
				}
			}
		}
	}	
}

int main(int argc, char **argv){
	
	FILE * input;
	int i;
	char filters[SIZE];
	for(i =0; i<30; i++){
		filters[i]=0;
	}
	for (i=1; i<argc; i++){
		if(strcmp(argv[i],"-i")==0){
			input = fopen(argv[++i],"r");
			fgets(filters, SIZE, input);
			fclose(input);
			break;
		}
		for(i=0; i<SIZE && argv[1][i]; i++){
			filters[i] = argv[1][i];
		}
	}

	if(argc > 1){
		filter(filters);
	}
	else{
		filterH();
	}
	return 0;
}
