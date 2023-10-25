#include "config/gbConfig.h"
#include "fake86.h"
#include "osd.h"
#include "dataFlash/gbcom.h"
#include "gbGlobals.h"
//#include "video/gb_sdl_font8x8.h"
#include "video/render.h"
#include "cpu/cpu.h"
#include "cpu/ports.h"
#include "io/keys.h"
#include "io/keyboard.h"
#include <Esp.h>
#include "io/sdcard.h"
#include "video/render.h"
#include <string.h>

#ifdef COLOR_3B           //       BGR 
 #define BLACK   0x08      // 0000 1000
 #define BLUE    0x0C      // 0000 1100
 #define RED     0x09      // 0000 1001
 #define MAGENTA 0x0D      // 0000 1101
 #define GREEN   0x0A      // 0000 1010
 #define CYAN    0x0E      // 0000 1110
 #define YELLOW  0x0B      // 0000 1011
 #define WHITE   0x0F      // 0000 1111
#endif


//extern int gb_screen_xIni;
//extern int gb_screen_yIni;
//extern unsigned char gb_cache_zxcolor[8];


unsigned char gb_show_osd_main_menu=0;

//extern SDL_Surface * gb_screen;
//extern SDL_Event gb_event;



#define max_gb_delay_cpu_menu 50
const char * gb_delay_cpu_menu[max_gb_delay_cpu_menu]={ 
 "0 (fast)","1","2","3","4","5","6","7","8","9",
 "10","11","12","13","14","15","16","17","18","19",
 "20","21","22","23","24","25","26","27","28","29",
 "30","31","32","33","34","35","36","37","38","39",
 "40","41","42","43","44","45","46","47","48","49"
};

#define max_gb_sound_menu 2
const char * gb_sound_menu[max_gb_sound_menu]={
 "Sound ON"
 ,"Sound OFF"
};

#define max_gb_main_menu 8
const char *gb_main_menu[max_gb_main_menu] = {
    "Load COM",
    "Drive A:",
    "Drive B:",
    "Reset",
    "Speed",
    "Video",
    "Sound",
    "Return"};

#define max_gb_video_menu 4
const char * gb_video_menu[max_gb_video_menu]={
 "Colors",
 "Font 4x8",
 "Font 8x8",  
 "Invert Color"
};

#define max_gb_speed_menu 4
const char * gb_speed_menu[max_gb_speed_menu]={
 "CPU delay",
 "Timer poll",
 "VGA poll",
 "Keyboard poll"
};


#define max_gb_vga_poll_menu 4
const char * gb_vga_poll_menu[max_gb_vga_poll_menu]={
 "20",
 "30",
 "40",
 "50"
};

#define max_gb_keyboard_poll_menu 5
const char * gb_keyboard_poll_menu[max_gb_keyboard_poll_menu]={
 "10",
 "20",
 "30",
 "40",
 "50"
};

#define max_gb_timers_poll_menu 7
const char * gb_timers_poll_menu[max_gb_timers_poll_menu]={
 "216 (4.62)",
 "108 (9.2)",
 "54 (18.5)",
 "27 (37.03)",
 "13 (76.92)",
 "6  (166.66)",
 "1  (fast)"
};

#define max_gb_reset_menu 2
const char * gb_reset_menu[max_gb_reset_menu]={
 "Soft",
 "Hard"
};


#define gb_pos_x_menu 50
#define gb_pos_y_menu 20
#define gb_osd_max_rows 10

//*************************************************************************************
void SDLprintText(const char *cad,int x, int y, unsigned char color,unsigned char backcolor)
{
//SDL_Surface *surface,
// gb_sdl_font_6x8
 int auxLen= strlen(cad);
 if (auxLen>50)
  auxLen=50;
 for (int i=0;i<auxLen;i++)
 {
  renderPrintCharOSD(cad[i],x,y,color,backcolor);
  x+=7;
 }
}

void OSDMenuRowsDisplayScroll(const char **ptrValue,unsigned char currentId,unsigned char aMax)
{//Dibuja varias lineas
 for (int i=0;i<gb_osd_max_rows;i++)
  SDLprintText("                    ",gb_pos_x_menu,gb_pos_y_menu+8+(i<<3),0,0);
 
 for (int i=0;i<gb_osd_max_rows;i++)
 {
  if (currentId >= aMax)
   break;
  //SDLprintText(gb_osd_sdl_surface,ptrValue[currentId],gb_pos_x_menu,gb_pos_y_menu+8+(i<<3),((i==0)?CYAN:WHITE),((i==0)?BLUE:BLACK),1);
  SDLprintText(ptrValue[currentId],gb_pos_x_menu,gb_pos_y_menu+8+(i<<3),((i==0)?0:WHITE),((i==0)?WHITE:0));  
  currentId++;
 }     
}

//Maximo 256 elementos
unsigned char ShowTinyMenu(const char *cadTitle,const char **ptrValue,unsigned char aMax)
{
  unsigned char aReturn=0;
  bool bExit = false;
  renderClearScreen();
  SDLprintText("Port Fake86 by Ackerman",gb_pos_x_menu-(4<<3),gb_pos_y_menu-16,WHITE,0);
  for (int i=0;i<20;i++)
    renderPrintCharOSD(' ',gb_pos_x_menu+(i<<3),gb_pos_y_menu,0,WHITE);
  SDLprintText(cadTitle,gb_pos_x_menu,gb_pos_y_menu,0,WHITE);

  OSDMenuRowsDisplayScroll(ptrValue,0,aMax);

  while (!bExit)
  {
    extern KeyboardDriver *keyboard;
    uint8_t scancode = keyboard->getLastKey();
    switch (scancode)
    {
    case (KEY_CURSOR_LEFT):
      if (aReturn>10) aReturn-=10;
      OSDMenuRowsDisplayScroll(ptrValue,aReturn,aMax);       
      break;
    case (KEY_CURSOR_RIGHT):
      if (aReturn<(aMax-10)) aReturn+=10;
      OSDMenuRowsDisplayScroll(ptrValue,aReturn,aMax);       
      break;     

    case (KEY_CURSOR_UP):
      if (aReturn>0) aReturn--;
      OSDMenuRowsDisplayScroll(ptrValue,aReturn,aMax);
      break;
    case (KEY_CURSOR_DOWN):
      if (aReturn < (aMax-1)) aReturn++;
      OSDMenuRowsDisplayScroll(ptrValue,aReturn,aMax);
      break;
    case (KEY_ENTER):
      bExit = true;
      break;
    //case SDLK_KP_ENTER: case SDLK_RETURN: bExit= 1;break;
    case (KEY_ESC):
      bExit = true; 
      aReturn= 255;    
      break;
    }
 } 
 gb_show_osd_main_menu= 0;
 return aReturn;
}

//Menu DSK
void ShowTinyDSKMenu(uint32_t drive)
{
  extern SdCard sdcard;
  scandir_t * pResult = sdcard.scandir();
  if(pResult != nullptr)
  {
    static const uint32_t LENGTH = 256;
    static char * arItems[LENGTH];
    static uint32_t count = 0;
    while(pResult[count].name[0] != 0)
    {
      arItems[count] = &pResult[count].name[0];
      count++;
    }

    uint32_t selection = ShowTinyMenu("DSK", (const char **)arItems, count);

    if (selection > (count - 1))
      selection = count - 1;
    sdcard.OpenImage(drive, arItems[selection]);
    // running= 0;

    free((void *)pResult);
  }
}


void ShowTinyCPUDelayMenu()
{
 unsigned char aSelNum;
 aSelNum = ShowTinyMenu("Delay CPU ms",gb_delay_cpu_menu,max_gb_delay_cpu_menu);
 if (aSelNum == 255)
  return;
 gb_delay_tick_cpu_milis = aSelNum;  
}

void ShowTinyTimerDelayMenu()
{
 unsigned char aSelNum;
 aSelNum = ShowTinyMenu("Timers poll ms",gb_timers_poll_menu,max_gb_timers_poll_menu);
 switch (aSelNum)
 {
  case 0: gb_timers_poll_milis= 216; break;
  case 1: gb_timers_poll_milis= 108; break;
  case 2: gb_timers_poll_milis= 54; break;
  case 3: gb_timers_poll_milis= 27; break;
  case 4: gb_timers_poll_milis= 13; break;
  case 5: gb_timers_poll_milis= 6; break;
  case 6: gb_timers_poll_milis= 1; break;
 }
}

void ShowTinyVGApollMenu()
{
 unsigned char aSelNum;
 aSelNum = ShowTinyMenu("VGA poll ms",gb_vga_poll_menu,max_gb_vga_poll_menu);
 switch (aSelNum)
 {
  case 0: gb_vga_poll_milis= 20; break;
  case 1: gb_vga_poll_milis=30; break;
  case 2: gb_vga_poll_milis=40; break;
  case 3: gb_vga_poll_milis=50; break;
 }
}

void ShowTinyKeyboardPollMenu()
{
 unsigned char aSelNum;
 aSelNum = ShowTinyMenu("Keyboard poll ms",gb_keyboard_poll_menu,max_gb_keyboard_poll_menu);
 switch (aSelNum)
 {
  case 0: gb_keyboard_poll_milis= 10; break;
  case 1: gb_keyboard_poll_milis= 20; break;
  case 2: gb_keyboard_poll_milis= 30; break;
  case 3: gb_keyboard_poll_milis= 40; break;
  case 4: gb_keyboard_poll_milis= 50; break;
 } 
}

//Menu velocidad emulador
void ShowTinySpeedMenu()
{
 unsigned char aSelNum;
 aSelNum = ShowTinyMenu("Speed",gb_speed_menu,max_gb_speed_menu);
 switch (aSelNum)
 {
  case 0: ShowTinyCPUDelayMenu(); break;
  case 1: ShowTinyTimerDelayMenu(); break;
  case 2: ShowTinyVGApollMenu(); break;
  case 3: ShowTinyKeyboardPollMenu(); break;
 } 
}

//Menu sonido
void ShowTinySoundMenu()
{
 unsigned char aSelNum;
 aSelNum = ShowTinyMenu("Sound",gb_sound_menu,max_gb_sound_menu);
 gb_silence= (aSelNum==0)?0:1;
}

//Menu resetear
void ShowTinyResetMenu()
{
 unsigned char aSelNum;
 aSelNum= ShowTinyMenu("Reset",gb_reset_menu,max_gb_reset_menu);
 if (aSelNum == 1)
 {
  ESP.restart();
 }
 else
 {
   gb_reset= 1;
 } 
}

void ShowTinyCOMMenu()
{
 unsigned char aSelNum;     
 aSelNum = ShowTinyMenu("COM",gb_list_com_title,max_list_com);

 //gb_cartfilename= (char *)gb_list_rom_title[aSelNum];
 gb_force_load_com= 1;
 gb_id_cur_com= aSelNum;
 //running= 0;
}

void ShowTinyVideoMenu()
{
 unsigned char aSelNum;
 aSelNum = ShowTinyMenu("Video",gb_video_menu,max_gb_video_menu);
 switch (aSelNum)
 {
   case 0: 
    {
      svcShowColorTable(); 
      
      bool bExit = false;
      while (!bExit)
      {
        extern KeyboardDriver *keyboard;
        uint8_t scancode = keyboard->getLastKey();
        switch(scancode)
        {
          case KEY_ESC:
            bExit = true;
            break;
          case KEY_1:
            renderSetBlitter(0);
            break;
          case KEY_2:
            renderSetBlitter(1);
            break;
          case KEY_3:
            renderSetBlitter(2);
            break;
        }
      }
    }
    break;
   case 1: gb_font_8x8= 0; break; //font 4x8
   case 2: gb_font_8x8= 1; break; //font 8x8 
   case 3: gb_invert_color= ((~gb_invert_color)&0x01); break; //Invertir color
 }
}


//*******************************************
void SDLActivarOSDMainMenu()
{     
 gb_show_osd_main_menu= 1;   
}

//Very small tiny osd
void do_tinyOSD() 
{
 int auxVol;
 int auxFrec;  
 unsigned char aSelNum;
 extern KeyboardDriver *keyboard;
 uint8_t scancode = keyboard->getLastKey();
 if (scancode == KEY_F12)
 {
  gb_show_osd_main_menu= 1;
  return;
 }

 if (gb_show_osd_main_menu == 1)
 {
  auxVol= gb_volumen01;
  auxFrec= gb_frecuencia01;
  gb_volumen01= gb_frecuencia01=0;

  aSelNum = ShowTinyMenu("MAIN MENU",gb_main_menu,max_gb_main_menu);
  switch (aSelNum)
  {
   case 0:
    ShowTinyCOMMenu();
    gb_show_osd_main_menu=0;
    break;
   case 1:
    ShowTinyDSKMenu(0);
    gb_show_osd_main_menu=0;
    break;
   case 2:
    ShowTinyDSKMenu(1);
    gb_show_osd_main_menu = 0;
    break;
   case 3:
    ShowTinyResetMenu();
    gb_show_osd_main_menu=0;    
    break;
   case 4:
    ShowTinySpeedMenu(); 
    gb_show_osd_main_menu=0;   
    break;
   case 5: ShowTinyVideoMenu(); 
    gb_show_osd_main_menu=0;
    break;   
   case 6:
    ShowTinySoundMenu();        
    gb_show_osd_main_menu=0; 
    break;
   default: break;
  }

  gb_volumen01= auxVol;
  gb_frecuencia01= auxFrec;
  keyboard->Reset();
 }
 #ifdef use_lib_sound_ay8912
  gb_silence_all_channels = 0;
 #endif 
}

