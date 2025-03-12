#ifndef _GB_GLOBALS_H
 #define _GB_GLOBALS_H
 #include <stdint.h>
 #include <stdio.h>
 #include "Arduino.h"
 #include "keyboard/keyboard.h" 

extern unsigned char bootdrive;

extern unsigned char cf;

// extern unsigned char speakerenabled;

extern unsigned short int segregs[4];
extern unsigned char gb_video_cga[16384];
static const size_t      RAM_SIZE  = 640 * 1024;

extern unsigned char gb_reset;
 
#endif
