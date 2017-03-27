 #include <stdio.h>

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

void filter(int size, char filters[]){
	
	char c ,toPrint;
	int i;
	
	while(!feof(stdin)){
		toPrint = c = fgetc(stdin);
		if(c == '\n'){
			fputc(toPrint, stdout);
		}
		else{
			for(i =0; i<size; i++){
				if(c == filters[i]){
					fputc(toPrint, stdout);
					break;
				}
			}
		}
	}	
}


int main(int argc, char **argv){
	
	int size;
	char filters[30];

	if(argc>1){
		for(size=0; size<30 && argv[1][size]; size++){
			filters[size] = argv[1][size];
		}
		filter(size, filters);
	}
	return 0;
}
