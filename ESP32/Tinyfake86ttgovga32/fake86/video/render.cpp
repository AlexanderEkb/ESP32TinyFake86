//  Fake86: A portable, open-source 8086 PC emulator.
//  Copyright (C)2010-2012 Mike Chambers
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
// render.c: functions for SDL initialization, as well as video scaling/rendering.
//   it is a bit messy. i plan to rework much of this in the future. i am also
//   going to add hardware accelerated scaling soon.

#include "config/gbConfig.h"
#include <stdio.h>
#include <Arduino.h>
#include "gbGlobals.h"
#include "cpu/cpu.h"
#include "cpu/ports.h"
#include "video/render.h"
#include "fake86.h"
#include "video/gb_sdl_font8x8.h"
#include "video/gb_sdl_font4x8.h"

#define SUPPORT_NTSC 1
#include "video/CompositeColorOutput.h"

#define PENDING_COLORBURST_NO     (0x00)
#define PENDING_COLORBURST_TRUE   (0x01)
#define PENDING_COLORBURST_FALSE  (0x02)

typedef void(*dumper_t)(void);

// static void dump160x100_font4x8(void);
// static void dump80x25_font4x8(void);
// static void dump160x100_font8x8(void);
static void dump80x25(void);
static void dump40x25(void);
static void dump320x200(void);
static void dump640x200(void);

static void SDLprintChar_c(char code, uint32_t x, uint32_t y, uint8_t color, uint8_t backcolor);
static void SDLprintChar(char code, uint32_t x, uint32_t y, uint8_t color, uint8_t backcolor);

static const uint32_t DUMPER_COUNT = 7;
static const uint32_t MODE_COUNT = 8;

typedef struct {
  uint32_t textWidth;
  dumper_t dumper;
  uint32_t blitter;
} videoMode_t;
/*
2: graph dot resolution
1: graph
0: 80-col

0x0: text 40 col
0x1: text 80 col
0x2: graph 320 px
0x3: graph 320 px
0x4: text 40 col
0x5: text 80 col
0x6: graph 640 col
0x7: graph 640 col
*/
const videoMode_t modes[MODE_COUNT] = {
  {40, dump40x25, 1},
  {80, dump80x25, 0},
  {40, dump320x200,       1},
  {40, dump320x200,       1},
  {40, dump40x25, 1},
  {80, dump80x25, 0},
  {80, dump640x200,       0},
  {80, dump640x200,       0}
};

cursor_t cursor;
static uint32_t scanlineBuffer[CompositeColorOutput::XRES * 2];

static const uint8_t paletteBasic[16]={ 
// BLACK    BLUE    GREEN   CYAN    RED     MGNTA   YELLOW  WHITE
    0x00,   0x73,   0xD5,   0xB6,   0x44,   0x65,   0xE7,   0x0A,
    0x05,   0x78,   0xDA,   0xBB,   0x49,   0x6A,   0xEC,   0x0F
};

static const uint32_t GRAPH_PALETTE_COUNT = 4;
static const uint32_t GRAPH_PALETTE_SIZE = 4;
//                                                             Black  Green   Red     Yellow
uint8_t paletteGraphicGRYdim[GRAPH_PALETTE_SIZE]      = {0x00,  0x93,   0x13,   0xB3};

//                                                             Black  Green   Red     Yellow
const uint8_t paletteGraphicGRYdimBW[GRAPH_PALETTE_SIZE]    = {0x00,  0xC3,   0x43,   0x17};

//                                                             Black  Green   Red     Yellow
uint8_t paletteGraphicGRYbright[GRAPH_PALETTE_SIZE]   = {0x00,  0x98,   0x18,   0xB8};

//                                                             Black  Green   Red     Yellow
const uint8_t paletteGraphicGRYbrightBW[GRAPH_PALETTE_SIZE] = {0x00,  0xC8,   0x48,   0x1C};

//                                                             Black   Cyan   Magenta White
uint8_t paletteGraphicCMWdim[GRAPH_PALETTE_SIZE]      = {0x00,  0x86,   0x26,   0x0A};

//                                                             Black   Cyan   Magenta White
uint8_t paletteGraphicCMWbright[GRAPH_PALETTE_SIZE]   = {0x00,   0x8B,   0x2B,   0x0F};

static uint8_t * graphPalettes[GRAPH_PALETTE_COUNT] =
{
  paletteGraphicGRYdim,
  paletteGraphicGRYbright,
  paletteGraphicCMWdim,
  paletteGraphicCMWbright
};

static unsigned char palette[16]={
    0x00,   0x73,   0xC3,   0xB6,   0x44,   0x65,   0x17,   0x0A,
    0x05,   0x78,   0xC8,   0xBB,   0x49,   0x6A,   0x1C,   0x0F
};

static struct render {
  dumper_t dumper = dump80x25;
  uint32_t colCount = 80;
  uint32_t frameCount = 0;
  uint32_t paletteIndex = 0;
  uint32_t specialColor = 0;
  uint32_t pendingColorburstValue = PENDING_COLORBURST_NO;

  uint32_t charHeight = 8;
  uint32_t lineCount = 25;

  uint32_t startAddr = 0;
  uint32_t loResPixelOffset = 6;
} render;


CompositeColorOutput composite(CompositeColorOutput::NTSC);

char **gb_buffer_vga;

uint32_t cursor_t::row;
uint32_t cursor_t::col;
uint32_t cursor_t::value;
uint32_t cursor_t::start;
uint32_t cursor_t::end;

void cursor_t::updateMSB(uint8_t MSB)
{
  value = (value & 0x00FF) | (MSB << 8);
  updatePosition();
};

void cursor_t::updateLSB(uint8_t LSB)
{
  value = (value & 0xFF00) | (LSB);
  updatePosition();
};

void cursor_t::updateStart(uint32_t startLine)
{
  start = startLine;
}

void cursor_t::updateEnd(uint32_t endLine)
{
  end = endLine;
}

uint32_t __always_inline cursor_t::getCol()
{
  return col;
};

uint32_t __always_inline cursor_t::getRow()
{
  return row;
};

uint32_t __always_inline cursor_t::getStart()
{
  return start;
};

uint32_t __always_inline cursor_t::getEnd()
{
  return end;
};

void cursor_t::updatePosition()
{
  row = value / render.colCount;
  col = value % render.colCount;
};

void renderInit() {
  gb_buffer_vga = (char **)malloc(CompositeColorOutput::YRES * sizeof(char *));
  for (int y = 0; y < CompositeColorOutput::YRES; y++)
  {
    gb_buffer_vga[y] = (char *)malloc(CompositeColorOutput::XRES * 2);
    memset(gb_buffer_vga[y], DEFAULT_BORDER, CompositeColorOutput::XRES * 2);
  }

  void IRAM_ATTR blitter_0(uint8_t * src, uint16_t * dst);
  void IRAM_ATTR blitter_1(uint8_t * src, uint16_t * dst);
  composite.init(&gb_buffer_vga, &blitter_1);

  render.charHeight = 8;
  render.lineCount = 25;
}

void renderExec()
{
  if (render.pendingColorburstValue != PENDING_COLORBURST_NO)
  {
    composite.setColorburstEnabled(render.pendingColorburstValue == PENDING_COLORBURST_TRUE);
    render.pendingColorburstValue = PENDING_COLORBURST_NO;
  }
}

void renderClearScreen(uint8_t color)
{
  for (int y = 0; y < 200; y++)
  {
    for (int x = 0; x < 8; x++)
      gb_buffer_vga[y + VERTICAL_OFFSET][x] = DEFAULT_BORDER;
    for (int x = 0; x < 320; x++)
      gb_buffer_vga[y + VERTICAL_OFFSET][x + 8] = color ;
    for (int x = 0; x < 8; x++)
      gb_buffer_vga[y + VERTICAL_OFFSET][x + 328] = DEFAULT_BORDER;
  }
}

void renderPrintCharOSD(char character, int col, int row, unsigned char color, unsigned char backcolor)
{
  int auxId = character << 3; //*8
  unsigned char pixel;
  for (uint32_t y = 0; y < 8; y++)
  {
    uint8_t aux = gb_sdl_font_8x8[auxId + y];
    for (uint32_t x = 0; x < 8; x++)
    {
      pixel = ((aux >> x) & 0x01);
      const uint32_t line = row + y + VERTICAL_OFFSET;
      const uint32_t column = col + (8 - x);
      gb_buffer_vga[line][column] = (pixel == 1) ? color : backcolor;
    }
  }
}

//uint8_t initscreen (uint8_t *ver) 
unsigned char initscreen() 
{
	// memcpy(palette,paletteGraphicGRYdim,16);
 	// //JJ no SDL SDL_WM_SetCaption ("ESP32 Fake86", NULL);
	// #ifdef use_lib_log_serial
	//  Serial.printf("initscreen SDL_SetVideoMode\n");
	// #endif 
	return (1);
}

void draw();
extern void handleinput();

void VideoThreadPoll()
{
 draw();
}

//*****************************************

static void dump80x25()
{
 uint8_t aColor, aBgColor, aChar;
 uint32_t src = render.startAddr;
 for (uint32_t y = 0; y < render.lineCount; y++)
 {
  for (uint32_t x = 0; x < 80; x++)
  {
   aChar = gb_video_cga[src];
   src++;
   aColor = gb_video_cga[src] & 0x0F;
   aBgColor = ((gb_video_cga[src] >> 4) & 0x07);
   SDLprintChar(aChar, (x << 3), (y * render.charHeight), aColor, aBgColor); // Sin capturadora
   src++;
  }
 }
}

static void dump40x25()
{
 uint32_t src = render.startAddr;
 for (uint32_t y = 0; y < render.lineCount; y++)
 {
  for (uint32_t x = 0; x < 40; x++)
  {
    uint8_t aChar = gb_video_cga[src];
    src++;
    uint8_t aColor = gb_video_cga[src] & 0x0F;
    uint8_t aBgColor = ((gb_video_cga[src] >> 4) & 0x07);
    SDLprintChar(aChar, (x << 3), (y * render.charHeight), aColor, aBgColor); // Sin capturadora
    src++;
  }
 }
}

static void dump320x200()
{
  static const uint32_t INITIAL_OFFSET = 0;
  static uint8_t line[700];
  unsigned short int cont = 0;
  for (uint32_t y = 0; y < 100; y++)
  {
    uint32_t yDest = (y << 1);
    uint32_t offset = INITIAL_OFFSET;
    for (uint32_t x = 0; x < 80; x++)
    {
      uint8_t src = gb_video_cga[cont];
      uint8_t bPixel3 = (src & 0x03); src >>= 2;
      uint8_t bPixel2 = (src & 0x03); src >>= 2;
      uint8_t bPixel1 = (src & 0x03); src >>= 2;
      uint8_t bPixel0 = (src & 0x03);

      line[offset++] = palette[bPixel0];
      line[offset++] = palette[bPixel1];
      line[offset++] = palette[bPixel2];
      line[offset++] = palette[bPixel3];
      cont++;
    }
    uint32_t *dest = (uint32_t *)gb_buffer_vga[yDest + VERTICAL_OFFSET];
    memcpy(dest + 2, line, 320);
  } 

  cont = 0x2000;   
  for (uint32_t y=0;y<100;y++)
  {      
    uint32_t yDest= (y<<1)+1;
    uint32_t offset = INITIAL_OFFSET;
    for (uint32_t x = 0; x < 80; x++)
    {
      uint8_t src = gb_video_cga[cont];
      uint8_t bPixel3 = (src & 0x03); src >>= 2;
      uint8_t bPixel2 = (src & 0x03); src >>= 2;
      uint8_t bPixel1 = (src & 0x03); src >>= 2;
      uint8_t bPixel0 = (src & 0x03);

      line[offset++] = palette[bPixel0];
      line[offset++] = palette[bPixel1];
      line[offset++] = palette[bPixel2];
      line[offset++] = palette[bPixel3];
      cont++;
    }
    uint32_t *dest = (uint32_t *)gb_buffer_vga[yDest + VERTICAL_OFFSET];
    memcpy(dest + 2, line, 320);
  }
}

static void dump640x200()
{
  static uint8_t line[700];
  static uint32_t *dest;
  static const uint32_t INITIAL_OFFSET = 1;
  unsigned short int srcAddr;
  unsigned int yDest; 
  unsigned int x;
  unsigned int a32;

  srcAddr = 0x0000;
  for (uint32_t y = 0; y < 100; y++)
  {
    yDest= (y<<1);
    uint32_t offset = INITIAL_OFFSET;
    for (x=0;x<80;x++)   
    {
      unsigned char src = gb_video_cga[srcAddr];
      uint8_t a7 = (src & 0x01); src >>= 1;
      uint8_t a6 = (src & 0x01); src >>= 1;
      uint8_t a5 = (src & 0x01); src >>= 1;
      uint8_t a4 = (src & 0x01); src >>= 1;
      uint8_t a3 = (src & 0x01); src >>= 1;
      uint8_t a2 = (src & 0x01); src >>= 1;
      uint8_t a1 = (src & 0x01); src >>= 1;
      uint8_t a0 = (src & 0x01);

      uint32_t off = x << 1;
      line[offset++] = palette[a0];
      line[offset++] = palette[a1];
      line[offset++] = palette[a2];
      line[offset++] = palette[a3];
      line[offset++] = palette[a4];
      line[offset++] = palette[a5];
      line[offset++] = palette[a6];
      line[offset++] = palette[a7];

      srcAddr++;
    }
    dest = (uint32_t *)gb_buffer_vga[yDest + VERTICAL_OFFSET];
    memcpy(dest + 4, line, 640);
  } 

  srcAddr = 0x2000;   
  for (uint32_t y=0;y<100;y++)
  {      
    yDest= (y<<1) + 1;
    uint32_t offset = INITIAL_OFFSET;
    for (x = 0; x < 80; x++)
    {
    unsigned char src = gb_video_cga[srcAddr];
    uint8_t a7 = (src & 0x01); src >>= 1;
    uint8_t a6 = (src & 0x01); src >>= 1;
    uint8_t a5 = (src & 0x01); src >>= 1;
    uint8_t a4 = (src & 0x01); src >>= 1;
    uint8_t a3 = (src & 0x01); src >>= 1;
    uint8_t a2 = (src & 0x01); src >>= 1;
    uint8_t a1 = (src & 0x01); src >>= 1;
    uint8_t a0 = (src & 0x01);

    uint32_t off = x << 1;
    line[offset++] = palette[a0];
    line[offset++] = palette[a1];
    line[offset++] = palette[a2];
    line[offset++] = palette[a3];
    line[offset++] = palette[a4];
    line[offset++] = palette[a5];
    line[offset++] = palette[a6];
    line[offset++] = palette[a7];

    srcAddr++;
    }
    dest = (unsigned int *)gb_buffer_vga[yDest + VERTICAL_OFFSET];
    memcpy(dest + 4, line, 640);
  } 
}

//*************************************************************************************
static void SDLprintChar_c(char code, uint32_t x, uint32_t y, uint8_t color, uint8_t backcolor)
{ 
 int nBaseOffset = code << 3;
 const bool cbTime = render.frameCount & 0x04;
 for (unsigned int row=0; row<render.charHeight; row++)
 {
    const bool cbFill = (row >= cursor.getStart()) && (row <= cursor.getEnd()) && cbTime;
     unsigned char bLine = ((row >= 6) && (cbTime))?0xFF:gb_sdl_font_8x8[nBaseOffset + row];
     for (int col = 0; col < 8; col++) {
      unsigned char Pixel = ((bLine >> col) & 0x01);
      const uint32_t vgaLine = y + row + VERTICAL_OFFSET;
      const uint32_t vgaCol = x - col + 22;
      gb_buffer_vga[vgaLine][vgaCol] = paletteBasic[(Pixel != 0) ? color : backcolor];
  }
 }
}

static void SDLprintChar(char code, uint32_t x, uint32_t y, uint8_t color, uint8_t backcolor)
{
  if((x == (cursor.getCol() << 3)) && (y == (cursor.getRow() << 3)))
  {
    SDLprintChar_c(code, x, y, color, backcolor);
  }
  else
  {
    int nBaseOffset = code << 3; //*8
    for (unsigned int row = 0; row < render.charHeight; row++)
    {
      unsigned char bLine = gb_sdl_font_8x8[nBaseOffset + row];
      for (int col = 0; col < 8; col++)
      {
          unsigned char Pixel = ((bLine >> col) & 0x01);
          const uint32_t vgaLine = y + row + VERTICAL_OFFSET;
          const uint32_t vgaCol = x - col + 22;
          gb_buffer_vga[vgaLine][vgaCol] = paletteBasic[(Pixel != 0) ? color : backcolor];
      }
    }
  }
}

void draw()
{
  render.frameCount++;
  render.dumper();
}

void IRAM_ATTR blitter_0(uint8_t *src, uint16_t *dst)
{
  const uint32_t *destPaletteEven = RawCompositeVideoBlitter::ntsc_palette_even();
  const uint16_t *destPaletteOdd  = RawCompositeVideoBlitter::ntsc_palette_odd();
  static const uint32_t STEP = 4;
  static const uint32_t MASK_EVEN = 0x0000FFFF;
  static const uint32_t MASK_ODD  = 0xFFFF0000;

  uint32_t *d = (uint32_t *)dst + 16;
  for (int i = 0; i < RawCompositeVideoBlitter::NTSC_DEFAULT_WIDTH; i += STEP) // 84 steps, 4 pixels per step
  {
    d[0] = (destPaletteEven[src[0]] | destPaletteOdd[src[1]]);
    d[1] = (destPaletteEven[src[2]] | destPaletteOdd[src[3]]) << 8;
    d[2] = (destPaletteEven[src[4]] | destPaletteOdd[src[5]]);
    d[3] = (destPaletteEven[src[6]] | destPaletteOdd[src[7]]) << 8;
    d += STEP;
    src += STEP<<1;
  }
}

void renderSetCharHeight(uint8_t height)
{
  render.charHeight = height + 1;
  render.lineCount  = 200 / render.charHeight;
}

void IRAM_ATTR blitter_1(uint8_t *src, uint16_t *dst)
{
  const unsigned int *destPalette = RawCompositeVideoBlitter::_palette;
  static const uint32_t STEP = 4;

  uint32_t *d = (uint32_t *)dst + 16;
  for (int i = 0; i < RawCompositeVideoBlitter::NTSC_DEFAULT_WIDTH; i += STEP) // 84 steps, 4 pixels per step
  {
    d[0] = destPalette[src[0]];
    d[1] = destPalette[src[1]] << 8;
    d[2] = destPalette[src[2]];
    d[3] = destPalette[src[3]] << 8;
    d += STEP;
    src += STEP;
  }
}

void IRAM_ATTR blitter_2(uint8_t *src, uint16_t *dst)
{
  // static uint32_t line[RawCompositeVideoBlitter::NTSC_DEFAULT_WIDTH];
  const uint32_t *destPalette = RawCompositeVideoBlitter::_palette;
  const uint32_t *destPaletteEven = RawCompositeVideoBlitter::ntsc_palette_even();
  const uint16_t *destPaletteOdd = RawCompositeVideoBlitter::ntsc_palette_odd();
  static const uint32_t STEP = 4;

  uint32_t *d = (uint32_t *)(dst + 35);
  for (int i = 0; i < RawCompositeVideoBlitter::NTSC_DEFAULT_WIDTH; i += STEP) // 84 steps, 4 pixels per step
  {
    d[0] = destPalette[src[0]];
    d[1] = destPalette[src[1]] << 8;
    d[2] = destPalette[src[2]];
    d[3] = destPalette[src[3]] << 8;
    d += STEP;
    src += STEP;
  }
  // uint8_t *dest = (uint8_t *)((uint32_t *)dst + 16) + render.loResPixelOffset;
  // memcpy(dest, line, RawCompositeVideoBlitter::NTSC_DEFAULT_WIDTH * sizeof(uint32_t));
}

void renderSetPixelOffset(uint32_t const offset)
{
  render.loResPixelOffset = offset;
}

void renderSaveBlitter()
{
  composite.saveBlitter();
}

void renderSetPhase(uint32_t phase)
{
  composite.setPhase(phase);
}

void renderRestoreBlitter()
{
  composite.restoreBlitter();
}

void renderSetBlitter(unsigned int blitter)
{
  switch(blitter)
  {
    case 0:
      composite.setBlitter(blitter_0);
      composite.setPhase(3);
      break;
    case 1:
      composite.setBlitter(blitter_1);
      composite.setPhase(0);
      break;
    case 2:
      composite.setBlitter(blitter_2);
      composite.setPhase(0);
      break;
  }
}

void renderSetStartAddr(uint32_t addr)
{
  render.startAddr = addr * 2;
}

/*
updateColorSettings(00, 30)
MC6845 write reg 00h: 63h
MC6845 write reg 01h: 00h
MC6845 write reg 02h: 80h
MC6845 write reg 04h: 04h
MC6845 write reg 05h: eeh
MC6845 write reg 06h: 88h
MC6845 write reg 07h: 1eh
MC6845 write reg 09h: 00h
MC6845 write reg 0ah: 1eh
MC6845 write reg 0bh: 33h
MC6845 write reg 0ch: c0h
MC6845 write reg 0dh: 8eh
updateColorSettings(80, 30)
updateColorSettings(80, 30)
*/
void renderUpdateSettings(uint8_t settings, uint8_t colors)
{
  uint8_t _mode = (settings & 0x3) | ((settings >> 2) & 0x04);
  render.dumper = modes[_mode].dumper;
  render.colCount = modes[_mode].textWidth;
  const bool colorEnabled = !(settings & 0x04) || (settings & 10);
  render.pendingColorburstValue = colorEnabled ? PENDING_COLORBURST_TRUE : PENDING_COLORBURST_FALSE;
  renderSetBlitter(modes[_mode].blitter);

  // Colors
  static const uint8_t COLOR_MASK = 0x0F;
  static const uint8_t PALETTE_POS = 4;
  static const uint8_t PALETTE_MASK = 0x03;
  uint32_t specialColor = colors & COLOR_MASK;
  uint32_t paletteIndex = (colors >> PALETTE_POS) & PALETTE_MASK;
  render.paletteIndex = paletteIndex;
  render.specialColor = specialColor;

  enum mode {TEXT, GRAPH_LO, GRAPH_HI} mode = !(settings & 0x02) ? TEXT : ((settings & 0x10) ? GRAPH_HI : GRAPH_LO);
  switch (mode)
  {
  case TEXT:
    memcpy(palette, paletteBasic, 16);
    break;
  case GRAPH_LO:
    memcpy(palette, graphPalettes[paletteIndex], GRAPH_PALETTE_SIZE);
    palette[0] = paletteBasic[specialColor];
    break;
  case GRAPH_HI:
    palette[0] = 0;
    for(uint32_t item=1; item<16; item++)
      palette[item] = paletteBasic[specialColor];
    break;
  }
}

void svcBar(int orgX, int orgY, int height, int width, uint8_t color)
{
  for (int y = 0; y < height; y++)
  {
    int scanline = orgY + y + VERTICAL_OFFSET;
    for (int x = 0; x < width; x++)
    {
      int col = orgX + x;
      gb_buffer_vga[scanline][col] = color;
    }
  }
}
void svcDrawTableLoRes(uint32_t p)
{
  uint8_t *palette = graphPalettes[p];

  static const uint32_t BAR_WIDTH = 20;
  for(uint32_t bg = 0; bg<4; bg++)
  {
    for (uint32_t fg = 0; fg < 4; fg++)
    {
      for (uint32_t off=0; off<BAR_WIDTH; off++)
      {
        uint8_t color = palette[(off & 0x01)?fg:bg];
        uint32_t pos = (bg*4+fg)*BAR_WIDTH+off + 8;
        svcBar(pos, 0, 100, 1, color);
      }
    }
    svcBar(bg*80 + 8, 100, 100, 80, palette[bg]);
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
      int orgY = hue * HEIGHT;
      uint8_t color = ((uint8_t)hue << 4) | ((uint8_t)luma & 0x0F);
      svcBar(orgX + 8, orgY, HEIGHT, WIDTH, color);
    }
  }
}
