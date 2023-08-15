//Port Fake86 to TTGO VGA32 by ackerman
//Load COM, DSK
// MODE320x200
// Single core and dual core
// Only SRAM
// Tiny mod VGA library bitluni
// gbConfig options configuration compile

#include <Arduino.h>
#ifndef use_lib_speaker_cpu
 #include <Ticker.h>
#endif 
#include "gbConfig.h"
#include "gbGlobals.h"
#include "fake86.h"
#include "hardware.h"
#include "driver/timer.h"
#include "soc/timer_group_struct.h"
#include "i8253.h"
#include "i8259.h"
#include "i8237.h"
#include "cpu.h"
#include "render.h"
#include "video.h"
#include "timing.h"
#include "disk.h"
#include "dataFlash/gbcom.h"
#include "gb_sdl_font8x8.h"
#include "osd.h"
#include "keyboard.h"
#include "sdcard.h"

#define SUPPORT_NTSC 1
#include "CompositeColorOutput.h"

#ifndef use_lib_singlecore
//Video Task Core BEGIN
 void videoTask(void *unused);
 QueueHandle_t vidQueue;
 TaskHandle_t videoTaskHandle;
 volatile bool videoTaskIsRunning = false;
 uint16_t *param;
//Video Task Core END
#endif

#ifndef use_lib_speaker_cpu
 Ticker gb_ticker_callback;
#endif 

unsigned char gb_invert_color=0;
unsigned char gb_silence=0;

unsigned char gb_delay_tick_cpu_milis= use_lib_delay_tick_cpu_milis;
unsigned char gb_auto_delay_cpu= use_lib_delay_tick_cpu_auto;
unsigned char gb_vga_poll_milis= use_lib_vga_poll_milis;
unsigned char gb_keyboard_poll_milis= use_lib_keyboard_poll_milis;
unsigned char gb_timers_poll_milis= use_lib_timers_poll_milis;

unsigned char gb_frec_speaker_low=0;
unsigned char gb_frec_speaker_high=0;
unsigned char gb_cont_frec_speaker=0;
volatile int gb_frecuencia01=0;
volatile int gb_volumen01=0;

unsigned char gb_use_remap_cartdridge=0;
unsigned char gb_use_snarare_madmix=0;

unsigned char gb_font_8x8=1;

unsigned int lasttick; //16 bits timer
unsigned char gb_reset=0;
unsigned char gb_id_cur_com=0;

unsigned char tiempo_vga= 0;
unsigned int jj_ini_cpu,jj_end_cpu,jj_ini_vga,jj_end_vga;
unsigned int gb_cpu_timer_before,gb_cpu_timer_cur;
unsigned int gb_max_cpu_ticks,gb_min_cpu_ticks,gb_cur_cpu_ticks;
unsigned int gb_max_vga_ticks,gb_min_vga_ticks,gb_cur_vga_ticks;
unsigned long tickgap=0;
unsigned char port3da=0;

char **gb_buffer_vga;

CompositeColorOutput composite(CompositeColorOutput::NTSC);
KeyboardDriver * keyboard = new KeyboardDriverAT(); // stm32keyboard();
SdCard sdcard;

unsigned char *gb_ram_bank[PAGE_COUNT];
unsigned char gb_video_cga[16384];
//unsigned char gb_video_hercules[16384];
unsigned char slowsystem=0;
unsigned short int constantw = 0, constanth = 0;
unsigned char bootdrive=0;
unsigned char speakerenabled=0;
unsigned char renderbenchmark= 0;
unsigned int gb_keyboard_time_before, gb_keyboard_time_cur;
unsigned int gb_ini_vga;
unsigned int gb_cur_vga;
unsigned char scrmodechange = 0;

//unsigned char updatedscreen=0;
unsigned int x,y;
unsigned short int segregs[4];

const unsigned char palettecga[16]={
 0x00,0xAA,0x00,0xAA,
 0x00,0xAA,0x00,0xAA,
 0x55,0xFF,0x55,0xFF,
 0x55,0xFF,0x55,0xFF
};

unsigned char gbKeepAlive=0;

#define uint8_t unsigned char

unsigned char vidmode=5;
unsigned char gb_force_set_cga=0; //fuerzo modo cga Digger
unsigned char gb_force_load_com=0;
unsigned char gb_force_boot=0;
unsigned char gb_force_load_dsk=0;
unsigned char gb_id_cur_dsk=0;

unsigned char gb_portramTiny[51]; //Solo 51 puertos
void * gb_portTiny_write_callback[5]; //Solo 5
void * gb_portTiny_read_callback[5];

unsigned char cf;
unsigned char hdcount=0;
unsigned char running=0;

extern void VideoThreadPoll(void);
extern void draw(void);
extern void doscrmodechange();

extern uint64_t totalexec, totalframes;
uint64_t starttick, endtick;

uint32_t speed = 0;

void LoadCOMFlash(const unsigned char *ptr,int auxSize,int seg_load)
{
 int dir_load= seg_load*16;
 for (int i=0;i<auxSize;i++)
 {
  write86((dir_load+0x100+i),ptr[i]);
 }
 SetRegCS(seg_load);
 SetRegDS(seg_load);
 SetRegES(seg_load);
 SetRegSS(seg_load);
 SetRegIP(0x100);//0x100; 

 SetRegSP(0);
 SetRegBP(0);
 SetRegSI(0);
 SetRegDI(0);
 SetCF(0);     
}

#ifdef use_lib_log_serial
#define LOG(...) Serial.printf(__VA_ARGS__)
#else
#define LOG(...) (void)
#endif

void inithardware()
{
	LOG("Initializing emulated hardware:\n");
   #ifndef use_lib_not_use_callback_port
   memset(gb_portramTiny,0,sizeof(gb_portramTiny));
   memset(gb_portTiny_write_callback,0,sizeof(gb_portTiny_write_callback));
   memset(gb_portTiny_read_callback, 0, sizeof (gb_portTiny_read_callback));
   #endif
   LOG("  - Intel 8253 timer: ");
	init8253();
	LOG ("OK\n");
	LOG ("  - Intel 8259 interrupt controller: ");
	init8259();
   LOG ("OK\n");
   LOG ("  - Intel 8237 DMA controller: ");
	init8237();
   LOG ("OK\n");
	initVideoPorts();
	inittiming();
	initscreen();
}


void PerformSpecialActions()
{
 if (gb_reset == 1)
 {
  gb_reset=0;
  ClearRAM();
  memset(gb_video_cga,0,16384);
  keyboard->Reset();
  running = 1;
  reset86();	
  inithardware();
  return;
 }
 if (gb_force_load_com == 1)
 {
  gb_force_load_com=0;
  int auxOffs= 0;
  if (gb_list_seg_load[gb_id_cur_com] == 0)
   auxOffs= 0x07C0;
  else
   auxOffs= 0x0051;       
  LoadCOMFlash(gb_list_com_data[gb_id_cur_com],gb_list_com_size[gb_id_cur_com],auxOffs);
  return;
 }
}

//****************************
void ClearRAM()
{
 int i;
 for (i=0;i<gb_max_ram;i++)
 {
  write86(i,0);
 }
}

//****************************
void CreateRAM()
{
   for(uint32_t nIndex=0; nIndex<PAGE_COUNT; nIndex++)
   {
      unsigned char * pPage =  (unsigned char *)heap_caps_malloc(PAGE_SIZE, MALLOC_CAP_SPIRAM);
      memset(pPage, 0, PAGE_SIZE);
      gb_ram_bank[nIndex] = pPage;

   }
}

//Funciones
void setup(void);
void SDL_DumpVGA(void);
//Setup principal
void setup()
{ 
 pinMode(SPEAKER_PIN, OUTPUT);
 //REG_WRITE(GPIO_OUT_W1TC_REG , BIT25); //LOW clear
 digitalWrite(SPEAKER_PIN, LOW);  

#ifdef use_lib_log_serial
  Serial.begin(115200);
   Serial.printf("\nHEAP BEGIN %d\n", ESP.getFreeHeap());
#endif
 sdcard.Init();
 CreateRAM();
 ClearRAM();
 SetRAMTruco();//Primero pongo RAM 
 updateBIOSDataArea(); //Al inicio
 //insertdisk (0, "COMPAQDOS211cat.img");
 FuerzoParityRAM(); //Fuerzo que Parity sea en RAM
 
 // TODO: Initialize video here
 composite.init();
 gb_buffer_vga = (char **)malloc(CompositeColorOutput::YRES * sizeof(char *));
 for (int y = 0; y < CompositeColorOutput::YRES; y++) {
      gb_buffer_vga[y] = (char *)malloc(CompositeColorOutput::XRES);
      memset(gb_buffer_vga[y], 0, CompositeColorOutput::XRES);
 }

  LOG("VGA %d\n", ESP.getFreeHeap()); 
 PreparaColorVGA();
   keyboard->Init();
 

	running = 1;
	reset86();
   LOG ("OK!\n");    

	inithardware();

	lasttick = starttick = 0; //JJ millis();
	gb_ini_vga= lasttick;	
    //Serial.printf("Memoria:0x%02X 0x%02X\n",read86(0x413),read86(0x414));
    gb_keyboard_time_before= gb_keyboard_time_cur = gb_cpu_timer_before= gb_cpu_timer_cur= millis();

 
 #ifndef use_lib_singlecore
 //BEGIN TASK video
  vidQueue = xQueueCreate(1, sizeof(uint16_t *));
  xTaskCreatePinnedToCore(&videoTask, "videoTask", 1024 * 4, NULL, 5, &videoTaskHandle, 0); 
 //END Task video 
 #endif

 #ifndef use_lib_speaker_cpu
  float auxTimer= (float)1.0/(float)SAMPLE_RATE;
  gb_ticker_callback.attach(auxTimer,my_callback_speaker_func); 
 #endif 

 diskInit();

LOG("END SETUP %d\n", ESP.getFreeHeap()); 
}

#ifndef use_lib_singlecore
//******************************
void videoTask(void *unused)
{
 videoTaskIsRunning = true;   
 uint16_t *param;
 while (1) 
 {        
  xQueuePeek(vidQueue, &param, portMAX_DELAY);
  if ((int)param == 1)
   break; 

  draw();
  composite.sendFrameHalfResolution(&gb_buffer_vga);

  xQueueReceive(vidQueue, &param, portMAX_DELAY);
  videoTaskIsRunning = false;   
 } 
 videoTaskIsRunning = false;
 vTaskDelete(NULL);

 while (1) {
 }     
}
#endif

unsigned char gb_cpunoexe=0;
unsigned int gb_cpunoexe_timer_ini;
unsigned int tiempo_ini_cpu,tiempo_fin_cpu;
unsigned int total_tiempo_ms_cpu;
unsigned int tiene_que_tardar=0;

//Loop main
void loop() 
{ 
   jj_ini_cpu = micros();
#ifdef use_lib_singlecore
  if (gb_cpunoexe == 0)
  {
    exec86 (10000);
  }
#else 
  exec86 (10000); //Tarda 22 milis usar 2 cores
#endif 
  
   jj_end_cpu = micros();
   gb_cur_cpu_ticks= (jj_end_cpu-jj_ini_cpu);
   total_tiempo_ms_cpu= gb_cur_cpu_ticks/1000;
   if (gb_cur_cpu_ticks>gb_max_cpu_ticks)
     gb_max_cpu_ticks= gb_cur_cpu_ticks;
   if (gb_cur_cpu_ticks<gb_min_cpu_ticks)   
     gb_min_cpu_ticks= gb_cur_cpu_ticks;

   gb_keyboard_time_cur= millis();
   if ((gb_keyboard_time_cur- gb_keyboard_time_before) > gb_keyboard_poll_milis)
   {
      gb_keyboard_time_before= gb_keyboard_time_cur;
      const uint8_t scancode = keyboard->Exec();
      if (scancode != 0)
      {
         gb_portramTiny[fast_tiny_port_0x60] = scancode;
         gb_portramTiny[fast_tiny_port_0x64] |= 2;
         doirq(1);
      }

      PerformSpecialActions();    
      do_tinyOSD();    
   }

  if (scrmodechange)
  {
   //ClearSDL(); //Para borra modo texto grande
   doscrmodechange();
  }

  #ifdef use_lib_singlecore
  //Un solo CORE BEGIN
  gb_cur_vga= millis();
  //if ((gb_cur_vga - gb_ini_vga)>=41)
  if ((gb_cur_vga - gb_ini_vga) >= gb_vga_poll_milis)
  {     
   draw();
   composite.sendFrameHalfResolution(&gb_buffer_vga);
   gb_ini_vga = gb_cur_vga;   
  }
  #endif
  
#ifndef use_lib_singlecore
   xQueueSend(vidQueue, &param, portMAX_DELAY);
#endif
   if (gb_cpunoexe == 0)
   {
      gb_cpunoexe=1;
      gb_cpunoexe_timer_ini= millis();
      tiene_que_tardar= gb_delay_tick_cpu_milis;
   }
   else if ((millis()-gb_cpunoexe_timer_ini) >= tiene_que_tardar)
   {
      gb_cpunoexe=0;
   }
     


   gb_cpu_timer_cur= millis();
   if ((gb_cpu_timer_cur-gb_cpu_timer_before)>1000)
   {
      gb_cpu_timer_before= gb_cpu_timer_cur;
      if (tiempo_vga == 1)
      {
         LOG("c:%u m:%u mx:%u vc:%u m:%u mx:%u\n",gb_cur_cpu_ticks,gb_min_cpu_ticks,gb_max_cpu_ticks, gb_cur_vga_ticks,gb_min_vga_ticks,gb_max_vga_ticks);
         //Reseteo VGA
         gb_min_vga_ticks= 1000000;
         gb_max_vga_ticks= 0;
         gb_cur_vga_ticks= 0;   
         tiempo_vga=0;
      }
      else
      { // Imprimo CPU
         LOG("c:%u m:%u mx:%u\n",gb_cur_cpu_ticks,gb_min_cpu_ticks,gb_max_cpu_ticks);
      }

     //Reseteo CPU a 1 segundo
     gb_min_cpu_ticks= 1000000;
     gb_max_cpu_ticks= 0;
     gb_cur_cpu_ticks= 0;
    }

#ifndef use_lib_singlecore
      // TASK video BEGIN
      TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
      TIMERG0.wdt_feed = 1;
      TIMERG0.wdt_wprotect = 0;
      vTaskDelay(0); // important to avoid task watchdog timeouts - change this to slow down emu
// TASK video END
#endif

      // }//fin if running
   }
