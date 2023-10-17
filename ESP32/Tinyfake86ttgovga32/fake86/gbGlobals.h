#ifndef _GB_GLOBALS_H
 #define _GB_GLOBALS_H
 #include <stdint.h>
 #include "gbConfig.h"
 #include <stdio.h>
 #include "keyboard.h" 

#ifdef use_lib_log_serial
#define LOG(...) Serial.printf(__VA_ARGS__)
#else
#define LOG(...) (void)
#endif

#define CGA_BASE_MEMORY (0xB8000)

#define VIDEO_MODE_40x25_BW       (0x00)
#define VIDEO_MODE_40x25_COLOR    (0x01)
#define VIDEO_MODE_80x25_BW       (0x02)
#define VIDEO_MODE_80x25_COLOR    (0x03)
#define VIDEO_MODE_320x200_COLOR  (0x04)
#define VIDEO_MODE_320x200_BW     (0x05)
#define VIDEO_MODE_640x200_COLOR  (0x06)

extern unsigned char bootdrive;
// noscale, usessource, useconsole,doaudio,cgaonly,nosmooth,ethif

extern unsigned char gb_force_load_com;
extern unsigned char gb_id_cur_com;
#ifdef use_lib_snapshot
// extern unsigned char gb_memory_write[1048576];
extern unsigned char gb_force_snapshot_begin;
extern unsigned char gb_force_snapshot_end;
#endif

#ifdef use_lib_limit_256KB
extern unsigned char gb_check_memory_before;
#endif

// extern unsigned char RAM[0x100000];
// extern unsigned char RAM[gb_max_ram];  //Ya no uso RAM sino bloques de RAM
// JJ extern unsigned char VRAM[262144]; //quito vga por ahora
// extern unsigned char cf; //Lo dejo static optimizado
extern unsigned char cf;

// #ifdef use_lib_limit_portram
//  extern unsigned char portram[gb_max_portram]; //limito a 1023 bytes de puertos
// #else
//  extern unsigned char portram[0x10000];
// #endif
// JJ extern unsigned char portram[gb_max_portram]; //limito a 1023 bytes de puertos
extern unsigned char running;

#ifdef use_lib_debug_interrupt
extern unsigned char gb_interrupt_before;
#endif

extern unsigned char didbootstrap;

extern unsigned char speakerenabled;

extern const unsigned char fontcga[];
extern unsigned long int gb_jj_cont_timer;
// JJVGA extern unsigned short int VGA_SC[0x100], VGA_CRTC[0x100], VGA_ATTR[0x100], VGA_GC[0x100]; //no necesito VGA
 
 extern unsigned short int segregs[4];
 //extern unsigned char VGA_latch[4]; //Solo CGA

 //extern unsigned char updatedscreen;
 
 extern unsigned int usegrabmode;
 
 extern unsigned int x,y;
 
 extern unsigned char gb_video_cga[16384];
 //extern unsigned char gb_video_hercules[16384];
 
 static const uint32_t   PAGE_COUNT  = 20;
 static const size_t     PAGE_SIZE   = 32768;
 extern unsigned char *gb_ram_bank[PAGE_COUNT];
 
// extern const unsigned char gb_reserved_memory[16];
 extern volatile unsigned char oldKeymap[256];

 //extern unsigned char keydown[0x100];

/* extern unsigned char gb_current_ms_poll_sound; //milisegundos muestreo
 extern unsigned char gb_current_ms_poll_keyboard; //milisegundos muestreo teclado
 extern unsigned char gb_current_frame_crt_skip; //el actual salto de frame
 extern unsigned char gb_current_delay_emulate_ms; //la espera en cada iteracion
 extern unsigned char gb_sdl_blit;
 extern unsigned char gb_screen_xOffset;

 extern unsigned char gb_show_osd_main_menu;*/


 extern unsigned char gb_font_8x8;

  extern unsigned char gb_reset;
 

 //Medicion tiempos

 extern unsigned char keyboardwaitack;

 extern unsigned char gb_ram_truco_low;
 extern unsigned char gb_ram_truco_high;

 extern unsigned char gb_frec_speaker_low;
 extern unsigned char gb_frec_speaker_high;
 extern unsigned char gb_cont_frec_speaker;
 extern volatile int gb_frecuencia01;
 extern volatile int gb_volumen01;
 extern volatile unsigned int gb_pulsos_onda;
 extern volatile unsigned int gb_cont_my_callbackfunc; 
 extern volatile unsigned char speaker_pin_estado;

 extern unsigned char gb_delay_tick_cpu_milis;
 extern unsigned char gb_vga_poll_milis;
 extern unsigned char gb_keyboard_poll_milis;
 extern unsigned char gb_timers_poll_milis;

 //fast bitluni
 extern unsigned char gb_sync_bits;

 extern unsigned char gb_invert_color;
 extern unsigned char gb_silence;
#endif
