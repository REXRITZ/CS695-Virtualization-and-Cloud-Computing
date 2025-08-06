#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "spock_ioctl.h"

#define FILE_NAME "/dev/spock_driver"

int main(int argc, char** argv) {

    if(argc != 2) {
        printf("Usage: ./spock_user <count>");
        return -1
    }

    printf("PID: %ld\n", getpid());

    int count = atoi(argv[1]);
    int value = 104;
    struct data val[count];
    char *baseAddr = malloc(count);

    for(char* addr = baseAddr; addr < baseAddr + count; ++addr) {
        *addr = value;
        value++;
        printf("VA: 0x%lx Value: %d\n", (unsigned long)addr, *addr);
    }

    int fd = open(FILE_NAME, O_RDWR);
    if(fd < 0) {
        perror("error in opening file\n");
        free(baseAddr);
        return -1;
    }

    printf("VA to PA translation\n");
    int i = 0;
    for(char* addr = baseAddr; addr < baseAddr + count; ++addr) {
        
        // struct data info;
        // info.vaddr = (unsigned long)addr;
        val[i].value = *addr;
        val[i].vaddr = (unsigned long)addr;
        if(ioctl(fd, V2P, &val[i]) == -1) {
            perror("VA to PA: ioctl failed\n");
            free(baseAddr);
            return -1;
        }
        printf("VA: 0x%lx\tPA: 0x%lx\n", val[i].vaddr, val[i].phyaddr);
        i++;
    }
    
    char newValue = 53;
    i = 0;
    for(char* addr = baseAddr; addr < baseAddr + count; ++addr) {
        // old value
        printf("PA: 0x%lx\tValue: %d\n", val[i].phyaddr, val[i].value);
        //new value
        val[i].value = newValue;
        if(ioctl(fd, WRITE_PHY, &val[i]) == -1) {
            perror("Write to physical address: ioctl failed\n");
            free(baseAddr);
            return -1;
        }
        i++;
        newValue++;
    }

    i = 0;
    for(char* addr = baseAddr; addr < baseAddr + count; ++addr) {
        
        printf("VA: 0x%lx\tPA: 0x%lx\tUpdated Value: %d\n", val[i].vaddr, val[i].phyaddr, *addr);
        i++;
    }

    close(fd);
    free(baseAddr);
    return 0;
}