#include <stdio.h>

int ntsc(char* str){
	int i, count = 0;
	for(i=0; str[i] != '\0' && str[i] != '\n'; i++){
		if(str[i] >= '0' && str[i] <= '9'){
			count++;
		}	
	}
	return count;
}


int main(int argc, char** argv){
	int count = ntsc(argv[1]);
	printf("%d\n",count);
	return count;
}