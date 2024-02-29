#ifndef _GB_GLOBALS_H
 #define _GB_GLOBALS_H
 #include <stdint.h>
 #include "config/gbConfig.h"
 #include <stdio.h>
 #include "io/keyboard.h" 

#ifdef use_lib_log_serial
#define LOG(...) Serial.printf(__VA_ARGS__)
#else
#define LOG(...) (void)
#endif

extern unsigned char bootdrive;

extern unsigned char cf;

extern unsigned char didbootstrap;

// extern unsigned char speakerenabled;

extern unsigned short int segregs[4];
extern unsigned char gb_video_cga[16384];
static const size_t      RAM_SIZE  = 640 * 1024;

extern unsigned char gb_reset;
 


 extern volatile bool speakerMute;

 extern unsigned char gb_delay_tick_cpu_milis;
 extern unsigned char gb_vga_poll_milis;
 extern unsigned char gb_keyboard_poll_milis;
 extern unsigned char gb_timers_poll_milis;
#endif
