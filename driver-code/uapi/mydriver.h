#pragma once
#include <asm/ioctl.h>

#define MY_IOCTL_IN _IOW('w', 1, my_ioctl_data)

typedef struct my_ioctl_data {
     int newCounterVal;
}my_ioctl_data;
