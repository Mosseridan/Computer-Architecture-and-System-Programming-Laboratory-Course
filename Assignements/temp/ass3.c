#include <stdio.h>

extern void build_structures();
extern void init_co_from_c(int ndx);
extern void start_co_from_c();
extern int NumCo;


int main(int argc, char** argv){
  int i;
  build_structures();

  for(i = 0; i < NumCo; i++){
    init_co_from_c(i);
  }

  start_co_from_c();

 return 0;
}
