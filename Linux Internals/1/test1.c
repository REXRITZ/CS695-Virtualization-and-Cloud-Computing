#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

#define PAGE_SIZE 4096

int main(int argc, char** argv) {
    
    if(argc != 3) {
        printf("Usage: ./test <num_pages> <stride_in_bytes>\n");
        return -1;
    }

    printf("PID: %d\n", getpid());

    unsigned long numPages = strtoul(argv[1], NULL, 10);
    unsigned long stride = strtoul(argv[2], NULL, 10);
    unsigned long totalSz = numPages * PAGE_SIZE;
    char* baseAddr = malloc(totalSz);
    
    for(unsigned long i = 0; i < totalSz; i+=stride) {
        unsigned long offset = i;
        
        // baseAddr[offset] = i;
        baseAddr[offset] = 'X';
    }
    
    printf("Press anything to exit\n");
    getchar();
    
    free(baseAddr);
    return 0;

}