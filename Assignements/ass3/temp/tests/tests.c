#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test(char *file, char* cmd);
char* getStr(FILE* fp);

int main(int argc, char **argv) {

	printf("These tests ignore any newline character!!!\n");
	test("input_12x16 12 16 10 768"			,"../ass3 -d ../input/input_12x16 12 16 10 768 | cat > tmp");
	test("corners_20x30 20 30 3 1200"		,"../ass3 -d ../input/corners_20x30 20 30 3 1200 | cat > tmp");
	test("edges_20x30 20 30 3 1200"			,"../ass3 -d ../input/edges_20x30 20 30 3 1200 | cat > tmp");
	test("edges_60x60 60 60 2 7200"			,"../ass3 -d ../input/edges_60x60 60 60 2 7200 | cat > tmp");
	test("Oscillator_22x24 22 24 4 1056"	,"../ass3 -d ../input/Oscillator_22x24 22 24 4 1056 | cat > tmp");

	system("rm -f tmp");
	return 0;
}

void test(char *file, char *cmd){

	printf("-d %-40s - ", file);
	FILE *fp = fopen(file,"r");
	char* str1 = getStr(fp);
	fclose(fp);
	system(cmd);
	fp = fopen("./tmp","r");
	char* str2 = getStr(fp);
	fclose(fp);

	if(strcmp(str1, str2)==0)
		printf("PASS\n");
	else{
		printf("FAIL\n");
		printf("%s\n\n\n\n%s", str1, str2);
	}

	free(str1);
	free(str2);

	system("rm -f tmp");

}
char* getStr(FILE *fp){
	fseek(fp,0,SEEK_END);
	int len = ftell(fp);
	rewind(fp);
	char *buf = malloc(len+1);
	
	fread(buf,1,len,fp);
	char *str = malloc(len+1);
	size_t b,s;
	
	for(b=0, s=0; b<len; b++, s++){
		str[s]=buf[b];
		if(buf[b]=='\n' && buf[b+1]=='\n')
			b++; /* skip double '\n' */
	}
	str[s]=0;
	free(buf);
	return str;

}