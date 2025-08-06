#ifndef SPOCK_IOCTL_H
#define SPOCK_IOCTL_H
#include <linux/ioctl.h>

struct data {
    unsigned long vaddr;
    unsigned long phyaddr;
    unsigned char value;
};

#define DEVICE_NAME "spock_driver"
#define V2P _IOWR('v', 1, struct data*)
#define WRITE_PHY _IOW('v', 2, struct data*)

#endif