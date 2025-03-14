#ifndef _CONFIG_H
#define _CONFIG_H

#include <stddef.h>

//when CPU_V20 is defined, Fake86's CPU emulator acts like an 80186/V20.
//otherwise, it acts like a true 8086/8088
#define CPU_V20

#define KEYBOARD_POLL_ms (20)
#define DEFAULT_HDD_IMAGE "/sd/PC/HDDs/hdd0.img"

static const size_t RAM_SIZE  = 640 * 1024;

//#define STATS_ON
#endif

