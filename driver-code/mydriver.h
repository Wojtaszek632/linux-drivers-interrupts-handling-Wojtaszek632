#pragma once
#include <asm/ioctl.h>
#include "../../include/uapi/misc/mydriver.h"

#define sysbusAddr 0xf000B000

#define GPIO_STATE_REG 0x0
#define INTERRUPT_STATUS_REG 0x0c
#define INTERRUPT_PENDING_REG 0x10
#define INTERRUPT_ENABLE_REG 0x14
/*
0x0 - GPIO state
0x0c - interrupt status
0x10 - interrupt pending
0x14 - interrupt enable
*/
