#include "osd.h"
#include "config/gbConfig.h"
#include "cpu/cpu.h"
#include "cpu/ports.h"
#include "fake86.h"
#include "gbGlobals.h"
#include "io/keyboard.h"
#include "io/keys.h"
#include "io/sdcard.h"
#include "video/CompositeColorOutput.h"
#include "video/render.h"
#include "video/gb_sdl_font8x8.h"
#include "service/service.h"
#include <Esp.h>
#include <string.h>
#include "stats.h"
#include "debugger/debugger.h"

static unsigned char palette[16] = {
    0x00, 0x73, 0xC3, 0xB6, 0x44, 0x65, 0x17, 0x0A,
    0x05, 0x78, 0xC8, 0xBB, 0x49, 0x6A, 0x1C, 0x0F};

#define BLACK         0x00
#define BLUE          0x73
#define RED           0x44
#define MAGENTA       0x65
#define GREEN         0xC3
#define CYAN          0xB6
#define YELLOW        0x17
#define WHITE         0x0A
#define GRAY          0x05
#define LIGHTBLUE     0x78
#define LIGHTRED      0x49
#define LIGHTMAGENTA  0x6A
#define LIGHTGREEN    0xC8
#define LIGHTCYAN     0xBB
#define LIGHTYELLOW   0x1C
#define LIGHTWHITE    0x0F

static struct osd {
  bool active       = false;
} osd;

extern char **bufferNTSC;
extern CompositeColorOutput composite;
extern uint8_t ** graphPalettes;

#define max_gb_delay_cpu_menu 50
const char * gb_delay_cpu_menu[max_gb_delay_cpu_menu]={ 
 "0 (fast)","1","2","3","4","5","6","7","8","9",
 "10","11","12","13","14","15","16","17","18","19",
 "20","21","22","23","24","25","26","27","28","29",
 "30","31","32","33","34","35","36","37","38","39",
 "40","41","42","43","44","45","46","47","48","49"
};

#define max_gb_main_menu 6
const char *gb_main_menu[max_gb_main_menu] = {
    "Drive A:",
    "Drive B:",
    "Reset",
    "Speed",
    "Video",
    "Debug"};

#define max_gb_video_menu 3
const char * gb_video_menu[max_gb_video_menu]={
 "Color",
 "Palette",
 "LO_RES"
};

#define COLOR_MENU_ITEM_COUNT 3
const char * colorMenu[COLOR_MENU_ITEM_COUNT]={
 "As set by SW",
 "Enable",
 "Disable"
};

#define max_gb_speed_menu 2
const char * gb_speed_menu[max_gb_speed_menu]={
 "CPU delay",
 "Timer poll",
};


#define max_gb_vga_poll_menu 4
const char * gb_vga_poll_menu[max_gb_vga_poll_menu]={
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

static uint8_t const * const font = getFont();

#define gb_pos_x_menu 10
#define gb_pos_y_menu 25
#define gb_osd_max_rows 20

static void osdLeave();
static void svcDrawTableLoRes(uint32_t p);
static void svcShowColorTable(void);
static uint8_t *svcGetPalette(uint32_t p);

static void showColorMenu();

void OSDMenuRowsDisplayScroll(const char **ptrValue, unsigned char currentId, unsigned char aMax, uint32_t width, uint32_t pos, int32_t highlight = -1)
{//Dibuja varias lineas
  const uint32_t MAX_WIDTH = 64;
  if(width > MAX_WIDTH) width = MAX_WIDTH;
  char lineOfSpaces[MAX_WIDTH + 1];
  memset(lineOfSpaces, ' ', MAX_WIDTH + 1);
  lineOfSpaces[width] = 0x00;

  const int rowCount = (aMax > gb_osd_max_rows) ? gb_osd_max_rows : aMax;
  for (int i = 0; i < rowCount; i++)
  {
    svcPrintText(lineOfSpaces,pos,gb_pos_y_menu+8+(i<<3),0,DEFAULT_BORDER);
    if (currentId < aMax)
    {
      uint8_t foreground = (currentId == highlight) ? 0x35 : ((i == 0) ? DEFAULT_BORDER : SCREEN_BACKGROUND);
      uint8_t background = ((i == 0) ? SCREEN_BACKGROUND : DEFAULT_BORDER);

      svcPrintText(ptrValue[currentId], pos + 1, gb_pos_y_menu + 8 + (i << 3), foreground, background);
      currentId++;
    }
  }
}

//Maximo 256 elementos
uint8_t ShowTinyMenu(const char *cadTitle, const char **ptrValue, unsigned char aMax, uint32_t width, uint32_t pos, int32_t highlight = -1)
{
  unsigned char aReturn=0;
  bool bExit = false;
  for (int i = 0; i < width; i++)
  svcPrintChar(' ', pos + (i << 3), gb_pos_y_menu, 0, WHITE);
  svcPrintText(cadTitle,pos,gb_pos_y_menu,0,WHITE);

  OSDMenuRowsDisplayScroll(ptrValue,0,aMax, width, pos, highlight);

  while (!bExit)
  {
    extern KeyboardDriver *keyboard;
    uint8_t scancode = keyboard->getLastKey();
    switch (scancode)
    {
    case (KEY_CURSOR_LEFT):
      if (aReturn>10) aReturn-=10;
      OSDMenuRowsDisplayScroll(ptrValue, aReturn, aMax, width, pos, highlight);
      break;
    case (KEY_CURSOR_RIGHT):
      if (aReturn<(aMax-10)) aReturn+=10;
      OSDMenuRowsDisplayScroll(ptrValue, aReturn, aMax, width, pos, highlight);
      break;     

    case (KEY_CURSOR_UP):
      if (aReturn>0) aReturn--;
      OSDMenuRowsDisplayScroll(ptrValue, aReturn, aMax, width, pos, highlight);
      break;
    case (KEY_CURSOR_DOWN):
      if (aReturn < (aMax-1)) aReturn++;
      OSDMenuRowsDisplayScroll(ptrValue, aReturn, aMax, width, pos, highlight);
      break;
    case (KEY_ENTER):
      bExit = true;
      break;
    case (KEY_ESC):
      bExit = true; 
      aReturn= 255;    
      break;
    }
 } 
 return aReturn;
}

static void showColorMenu()
{
  uint32_t selection = ShowTinyMenu("Color", colorMenu, COLOR_MENU_ITEM_COUNT, 13, 170);
 if(selection <= COLORBURST_DISABLE)
 {
    renderSetColorburstOverride(selection);
    LOG("renderSetColorburstOverride(%i)\n", selection);
 }
}

//Menu DSK
void ShowTinyDSKMenu(uint32_t drive)
{
  extern SdCard sdcard;
  scandir_t * list = sdcard.getList();
  if(list != nullptr)
  {
    const int32_t imgIndex = sdcard.getImageIndex(drive);
    static const uint32_t LENGTH = 256;
    static char * arItems[LENGTH];
    static uint32_t count = 0;
    while(list[count].name[0] != 0)
    {
      arItems[count] = &list[count].name[0];
      count++;
    }

    uint32_t selection = ShowTinyMenu("> Select image:", (const char **)arItems, count, 27, 90, imgIndex);

    if (selection != 0xFF)
      sdcard.OpenImage(drive, selection);

  }
}


void ShowTinyCPUDelayMenu()
{
 unsigned char aSelNum;
 aSelNum = ShowTinyMenu("> Delay CPU ms",gb_delay_cpu_menu,max_gb_delay_cpu_menu, 14, 202);
 if (aSelNum == 255)
  return;
 gb_delay_tick_cpu_milis = aSelNum;  
}

void ShowTinyTimerDelayMenu()
{
 unsigned char aSelNum;
 aSelNum = ShowTinyMenu("> Timers poll",gb_timers_poll_menu,max_gb_timers_poll_menu, 14, 202);
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
 aSelNum = ShowTinyMenu("> VGA poll ms",gb_vga_poll_menu,max_gb_vga_poll_menu, 14, 202);
 switch (aSelNum)
 {
  case 0: gb_vga_poll_milis= 20; break;
  case 1: gb_vga_poll_milis=30; break;
  case 2: gb_vga_poll_milis=40; break;
  case 3: gb_vga_poll_milis=50; break;
 }
}

//Menu velocidad emulador
void ShowTinySpeedMenu()
{
 unsigned char aSelNum;
 aSelNum = ShowTinyMenu("> Speed",gb_speed_menu,max_gb_speed_menu, 14, 90);
 switch (aSelNum)
 {
  case 0: ShowTinyCPUDelayMenu(); break;
  case 1: ShowTinyTimerDelayMenu(); break;
 } 
}

//Menu resetear
void ShowTinyResetMenu()
{
 unsigned char aSelNum;
 aSelNum= ShowTinyMenu("Reset",gb_reset_menu,max_gb_reset_menu, 10, 90);
 if (aSelNum == 1)
 {
  ESP.restart();
 }
 else
 {
   gb_reset= 1;
 } 
}

void ShowTinyVideoMenu()
{
 unsigned char aSelNum = ShowTinyMenu("Video",gb_video_menu,max_gb_video_menu, 10, 90);
 switch (aSelNum)
 {
   case 0:
     showColorMenu();
     break;
   case 1: 
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
            composite.setBlitter(0);
            break;
          case KEY_2:
            composite.setBlitter(1);
            break;
          case KEY_3:
            composite.setBlitter(2);
            break;
        }
      }
    }
    break;
  case 2:
    {
      uint32_t paletteIndex = 0;
      uint8_t selection = 1;
      uint32_t phase = 0;
      bool bExit = false;
      while (!bExit)
      {
        char buffer[40];
        svcDrawTableLoRes(paletteIndex);
        composite.setPhase(phase);
        uint8_t * palette = svcGetPalette(paletteIndex);
        for(uint32_t c=1; c<4;c++)
        {
          sprintf(buffer, "%02X", palette[c]);
          svcPrintText(buffer, c*80+40, 146, (c == selection)?15:8, 0);
        }
        sprintf(buffer, "pal:   %i", paletteIndex);
        svcPrintText(buffer, 24, 120, 15, 0);
        sprintf(buffer, "phase: %i", phase);
        svcPrintText(buffer, 24, 128, 15, 0);
        extern KeyboardDriver *keyboard;
        uint8_t scancode = keyboard->getLastKey();
        switch (scancode)
        {
        case KEY_ESC:
          bExit = true;
          break;
        case KEY_1:
          composite.setBlitter(0);
          break;
        case KEY_2:
          composite.setBlitter(1);
          break;
        case KEY_3:
          composite.setBlitter(2);
          break;
        case KEY_F1:
          selection = 0;
          break;
        case KEY_F2:
          selection = 1;
          break;
        case KEY_F3:
          selection = 2;
          break;
        case KEY_F4:
          selection = 3;
          break;
        case KEY_F5:
          paletteIndex = (paletteIndex - 1) % 4;
          break;
        case KEY_F6:
          paletteIndex = (paletteIndex + 1) % 4;
          break;
        case KEY_F7:
          phase = (phase - 1) % 8;
          break;
        case KEY_F8:
          phase = (phase + 1) % 8;
          break;
        case KEY_CURSOR_UP:
          palette[selection] += 0x10;
          break;
        case KEY_CURSOR_DOWN:
          palette[selection] -= 0x10;
          break;
        case KEY_CURSOR_RIGHT:
          palette[selection] += 0x01;
          break;
        case KEY_CURSOR_LEFT:
          palette[selection] -= 0x01;
          break;
        }
      }
    }
    break;
 }
}


//*******************************************
//Very small tiny osd
OSD_RESULT_t do_tinyOSD(  )
{
 unsigned char aSelNum;
 extern KeyboardDriver *keyboard;
 uint8_t scancode = keyboard->getLastKey();
 if (scancode == KEY_F12)
 {
  osd.active = true;
  return OSD_RESULT_PREPARE;
 }

 if (osd.active)
 {
  composite.saveSettings();
  composite.setBlitter(1);
  composite.setColorburstEnabled(true);
  svcClearScreen(SCREEN_BACKGROUND);
  svcBar(8, OSD_VERTICAL_OFFSET, 21, 320, HEADER_BACKGROUND);
  svcPrintText("Port Fake86 by Ackerman", 12, 2, 0xC8, HEADER_BACKGROUND);
  svcPrintText("Extensions by Ochlamonster", 12, 12, 0xF9, HEADER_BACKGROUND);

  speakerMute = true;

  aSelNum = ShowTinyMenu("MAIN MENU",gb_main_menu,max_gb_main_menu, 10, 10);
  switch (aSelNum)
  {
   case 0:
    ShowTinyDSKMenu(0);
    break;
   case 1:
    ShowTinyDSKMenu(1);
    break;
   case 2:
    ShowTinyResetMenu();
    break;
   case 3:
    ShowTinySpeedMenu();
    break;
   case 4: 
    ShowTinyVideoMenu();
    break;
   case 5:
    debugger_t::getInstance().execute();
    break;
   default:
    break;
  }

  speakerMute= false;
  keyboard->Reset();
  osdLeave();
  return OSD_RESULT_RETURN;
 }

 return OSD_RESULT_NONE;
}

static void osdLeave()
{
  osd.active = false;
  composite.restoreSettings();
  renderUpdateBorder();
  extern KeyboardDriver *keyboard;
  keyboard->Reset();
}

void svcDrawTableLoRes(uint32_t p)
{
  uint8_t *palette = graphPalettes[p];

  static const uint32_t BAR_WIDTH = 20;
  for (uint32_t bg = 0; bg < 4; bg++)
  {
    for (uint32_t fg = 0; fg < 4; fg++)
    {
      for (uint32_t off = 0; off < BAR_WIDTH; off++)
      {
        uint8_t color = palette[(off & 0x01) ? fg : bg];
        uint32_t pos = (bg * 4 + fg) * BAR_WIDTH + off + 8;
        svcBar(pos, OSD_VERTICAL_OFFSET, 100, 1, color);
      }
    }
    svcBar(bg * 80 + 8, 100 + OSD_VERTICAL_OFFSET, 100, 80, palette[bg]);
  }
}

uint8_t *svcGetPalette(uint32_t p)
{
  return graphPalettes[p];
}

void svcShowColorTable()
{
  static const int WIDTH = 20;
  static const int HEIGHT = 10;
  for (int hue = 0; hue < 16; hue++)
  {
    for (int luma = 0; luma < 16; luma++)
    {
      int orgX = luma * WIDTH;
      int orgY = hue * HEIGHT + OSD_VERTICAL_OFFSET;
      uint8_t color = ((uint8_t)hue << 4) | ((uint8_t)luma & 0x0F);
      svcBar(orgX + 8, orgY, HEIGHT, WIDTH, color);
    }
  }
}
