#include <Esp.h>
#include <string.h>
#include "stats.h"
#include "osd.h"
#include "../../../host/host.h"
#include "../../../host/keyboard/keys.h"
#include "../sound/speaker.h"
#include "../cpu/cpu.h"
#include "../cpu/ports.h"
#include "../io/disk.h"
#include "../video/CGA/render_cga.h"
#include "../video/TGA/tga_render.h"
#include "../video/gb_sdl_font8x8.h"
#include "../service/service.h"
#include "../debugger/debugger.h"

#if (IBM_XT_VIDEO_DRIVER == 0)
extern uint8_t ** graphPalettes;
#endif

#define max_gb_main_menu 5
const char *gb_main_menu[max_gb_main_menu] = {
    "Drive A:",
    "Drive B:",
    "Reset",
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
    svcPrintText(lineOfSpaces,pos,gb_pos_y_menu+8+(i<<3),0,MENUITEM_DEFAULT_BACKGROUND);
    if (currentId < aMax)
    {
      uint8_t foreground = (currentId == highlight) ? MENUITEM_HIGHLIGHT_FOREGROUND : ((i == 0) ? MENUITEM_SELECTED_FOREGROUND : MENUITEM_DEFAULT_FOREGROUND);
      uint8_t background = ((i == 0) ? MENUITEM_SELECTED_BACKGROUND : MENUITEM_DEFAULT_BACKGROUND);

      svcPrintText(ptrValue[currentId], pos + 2, gb_pos_y_menu + 8 + (i << 3), foreground, background);
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
    svcPrintChar(' ', pos + (i << 3), gb_pos_y_menu, MENU_HEADER_FOREGROUND, MENU_HEADER_BACKGROUND);
  svcPrintText(cadTitle,pos,gb_pos_y_menu,MENU_HEADER_FOREGROUND, MENU_HEADER_BACKGROUND);

  OSDMenuRowsDisplayScroll(ptrValue,0,aMax, width, pos, highlight);

  while (!bExit)
  {
    uint8_t scancode = Host::keyboard->Poll();
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
#if (IBM_XT_VIDEO_DRIVER == 0)
    renderSetColorburstOverride(selection);
#elif (IBM_XT_VIDEO_DRIVER == 1)    
  tgaRender::setColorburstOverride(selection);
#endif
    ESP_LOGI("RENDER", "renderSetColorburstOverride(%i)\n", selection);
 }
}

//Menu DSK
void ShowTinyDSKMenu(uint32_t drive)
{
  static int32_t imgIndex[2] = {-1, -1};
  extern SdCard sdcard;
  scandir_t * list = Drive_t::sdCard.getList();
  if(list != nullptr)
  {
    static const uint32_t LENGTH = 256;
    static char * arItems[LENGTH];
    static uint32_t count = 0;
    while(list[count].name[0] != 0)
    {
      arItems[count] = &list[count].name[0];
      count++;
    }

    uint32_t selection = ShowTinyMenu("> Select image:", (const char **)arItems, count, 27, 90, imgIndex[drive]);

    if (selection != 0xFF)
    {
      char fullpath[LENGTH];
      char const *path = RG_STORAGE_FLOPPIES;
      snprintf(fullpath, LENGTH, "%s/%s", path, list[selection].name);
      drives[drive]->openImage(fullpath);
      imgIndex[drive] = selection;
    }
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
        uint8_t scancode = Host::keyboard->Poll();
        switch(scancode)
        {
          case KEY_ESC:
            bExit = true;
            break;
          case KEY_1:
            Host::video->miscCmd(SET_BLITTER, 0);
            break;
          case KEY_2:
            Host::video->miscCmd(SET_BLITTER, 1);
            break;
          case KEY_3:
            Host::video->miscCmd(SET_BLITTER, 2);
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
        Host::video->miscCmd(SET_PHASE, phase);
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
        uint8_t scancode = Host::keyboard->Poll();
        switch (scancode)
        {
        case KEY_ESC:
          bExit = true;
          break;
        case KEY_1:
          Host::video->miscCmd(SET_BLITTER, 0);
          break;
        case KEY_2:
          Host::video->miscCmd(SET_BLITTER, 1);
          break;
        case KEY_3:
          Host::video->miscCmd(SET_BLITTER, 2);
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
void do_tinyOSD()
{
  unsigned char aSelNum;
  uint8_t scancode = Host::keyboard->Poll();
  Host::video->miscCmd(PUSH_SETTINGS, 0);
  Host::video->miscCmd(SET_BLITTER, 1);
  Host::video->miscCmd(SET_COLOR, 1);
  svcClearScreen(SCREEN_BACKGROUND);
  svcBar(8, OSD_VERTICAL_OFFSET, 21, 320, HEADER_BACKGROUND);
  svcPrintText("Port Fake86 by Ackerman", 12, 2, 0xC8, HEADER_BACKGROUND);
  svcPrintText("Extensions by Ochlamonster", 12, 12, 0xF9, HEADER_BACKGROUND);
  svcPrintText(__DATE__, 8, 200, 0xF9, SCREEN_BACKGROUND);

  Speaker_t::mute();

  aSelNum = ShowTinyMenu("MAIN MENU", gb_main_menu, max_gb_main_menu, 10, 10);
  switch (aSelNum)
  {
  case 0:
    ShowTinyDSKMenu(0);
    break;
  case 1:
    ShowTinyDSKMenu(1);
    break;
  case 2:
    ESP.restart();
    break;
  case 3:
    ShowTinyVideoMenu();
    break;
  case 4:
    debugger_t::getInstance().execute();
    break;
  default:
    break;
  }

  Speaker_t::unmute();
  Host::keyboard->Reset();
  osdLeave();
}

static void osdLeave()
{
  Host::video->miscCmd(POP_SETTINGS, 0);
#if (IBM_XT_VIDEO_DRIVER == 0)  
  renderUpdateBorder();
#elif (IBM_XT_VIDEO_DRIVER == 1)  
  tgaRender::updateBorder();
#endif
  Host::keyboard->Reset();
}

void svcDrawTableLoRes(uint32_t p)
{
#if (IBM_XT_VIDEO_DRIVER == 0)
  uint8_t *palette = graphPalettes[p];
#elif (IBM_XT_VIDEO_DRIVER == 1)
  uint8_t *palette = (uint8_t *)tgaRender::graphPaletteColor(p);
#endif

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
#if (IBM_XT_VIDEO_DRIVER == 0)
  return graphPalettes[p];
#elif (IBM_XT_VIDEO_DRIVER == 1)
  return (uint8_t *)tgaRender::graphPaletteColor(p);
#endif
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
