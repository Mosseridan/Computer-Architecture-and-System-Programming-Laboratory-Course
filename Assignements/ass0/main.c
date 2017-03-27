#include <stdio.h>
# define MAX_LEN 100     // Maximal line size

extern int strToLeet (char*);

int main(void) {

  char str_buf[MAX_LEN];   
  int str_len = 0;
  
  fgets(str_buf, MAX_LEN, stdin);  // Read user's command line string  
 
  str_len = add_Str_N (str_buf);  	 // Your assembly code function 

  printf("%s\n%d\n",str_buf,str_len); 
} 
