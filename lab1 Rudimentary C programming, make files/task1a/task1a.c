#include <stdio.h>
#include <string.h>

int main(int argc, char **argv){
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
	return 0;
}

