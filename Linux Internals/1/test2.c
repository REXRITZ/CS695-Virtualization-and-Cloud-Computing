#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

int main(int argc, char** argv) {
    
    printf("PID: %d\n", getpid());

    unsigned long size = 1024 * 1024 * 16; // 16MB
    
    void *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if(addr == MAP_FAILED) {
        perror("memory alloc failed");
        munmap(addr, size);
        return -1;
    }

    memset(addr, 0, size);
    
    printf("Press anything to exit\n");
    getchar();
    
    munmap(addr, size);
   
    return 0;

}