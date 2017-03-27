#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    int i;
    char* cp = NULL;
    int* ip = NULL;
    int** ipp = NULL;
    void* vp = NULL;
    for(i=0; i<5 ;i++){
        cp++;
        ip++;
        ipp++;
        vp++;
    }
    printf("cp = %x, ip = %x, ipp = %x, vp = %x",cp,ip,ipp,vp);
    return 0;
}
