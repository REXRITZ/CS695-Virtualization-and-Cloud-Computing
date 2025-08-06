#ifndef BLOOM_IOCTL_H
#define BLOOM_IOCTL_H
#include <linux/ioctl.h>


#define DEVICE_NAME "bloom_driver"
#define FILE_NAME "/dev/bloom_driver"

#define MODIFY_PARENT _IOR('b', 1, struct PidParam*)
#define KILL_CHILD _IOR('b', 2, struct PidParam*)

#endif