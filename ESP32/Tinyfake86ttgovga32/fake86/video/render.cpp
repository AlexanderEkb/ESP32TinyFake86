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

#include "video/render.h"
#include "config/gbConfig.h"
#include "cpu/cpu.h"
#include "cpu/ports.h"
#include "fake86.h"
#include "gbGlobals.h"
#include "video/CompositeColorOutput.h"
#include "video/gb_sdl_font8x8.h"
#include <Arduino.h>
#include <stdio.h>

#define EFFECTIVE_HEIGHT (200)

#define BLITTER_HIRES (0)
#define BLITTER_LORES (1)

typedef void (*dumper_t)(void);

static void dump80x25(void);
static void dump40x25(void);
static void dump320x200(void);
static void dump640x200(void);

static void printChar_c(char code, uint32_t x, uint32_t y, uint8_t color, uint8_t backcolor);
static void printChar(char code, uint32_t x, uint32_t y, uint8_t color, uint8_t backcolor);

static const uint32_t DUMPER_COUNT = 7;
static const uint32_t MODE_COUNT = 8;

static uint8_t const *const font = getFont();

typedef enum vmode_t
{
  TEXT,
  GRAPH_LO,
  GRAPH_HI
} vmode_t;

typedef struct
{
  uint32_t textWidth;
  dumper_t dumper;
  uint32_t blitter;
  uint32_t hOffset;
} videoMode_t;

const videoMode_t modes[MODE_COUNT] = {
    {40, dump40x25, BLITTER_LORES, 16},
    {80, dump80x25, BLITTER_HIRES, 22},
    {40, dump320x200, BLITTER_LORES, 2},
    {40, dump320x200, BLITTER_LORES, 2},
    {40, dump40x25, BLITTER_LORES, 16},
    {80, dump80x25, BLITTER_HIRES, 22},
    {80, dump640x200, BLITTER_HIRES, 4},
    {80, dump640x200, BLITTER_HIRES, 4}};

cursor_t cursor;
static uint32_t scanlineBuffer[CompositeColorOutput::XRES * 2];

static const uint8_t paletteBasic[16] = {
    // BLACK    BLUE    GREEN   CYAN    RED     MGNTA   YELLOW  WHITE
    0x00, 0x73, 0xD5, 0xB6, 0x44, 0x65, 0xE7, 0x0A,
    0x05, 0x78, 0xDA, 0xBB, 0x49, 0x6A, 0xEC, 0x0F};
static const uint8_t paletteBasicBW[16] = {
    // BLACK    BLUE    GREEN   CYAN    RED     MGNTA   YELLOW  WHITE
    0x00, 0x01, 0x04, 0x05, 0x02, 0x03, 0x06, 0x08,
    0x07, 0x02, 0x08, 0x0A, 0x04, 0x06, 0x0E, 0x0F};

static const uint32_t GRAPH_PALETTE_COUNT = 4;
static const uint32_t GRAPH_PALETTE_SIZE = 4;
//                                                             Black  Green   Red     Yellow
uint8_t paletteGraphicGRYdim[GRAPH_PALETTE_SIZE] = {0x00, 0x93, 0x13, 0xB3};
const uint8_t paletteGraphicGRYdimBW[GRAPH_PALETTE_SIZE] = {0x00, 0x06, 0x02, 0x07};
uint8_t paletteGraphicGRYbright[GRAPH_PALETTE_SIZE] = {0x00, 0x98, 0x18, 0xB8};
const uint8_t paletteGraphicGRYbrightBW[GRAPH_PALETTE_SIZE] = {0x00, 0x0B, 0x03, 0x0F};

//                                                             Black   Cyan   Magenta White
uint8_t paletteGraphicCMWdim[GRAPH_PALETTE_SIZE] = {0x00, 0x86, 0x26, 0x0A};
const uint8_t paletteGraphicCMWdimBW[GRAPH_PALETTE_SIZE] = {0x00, 0x06, 0x02, 0x08};
uint8_t paletteGraphicCMWbright[GRAPH_PALETTE_SIZE] = {0x00, 0x8B, 0x2B, 0x0F};
const uint8_t paletteGraphicCMWbrightBW[GRAPH_PALETTE_SIZE] = {0x00, 0x0D, 0x05, 0x0F};

uint8_t *graphPalettes[GRAPH_PALETTE_COUNT] =
    {
        paletteGraphicGRYdim,
        paletteGraphicGRYbright,
        paletteGraphicCMWdim,
        paletteGraphicCMWbright};

uint8_t const *graphPalettesBW[GRAPH_PALETTE_COUNT] =
    {
        paletteGraphicGRYdimBW,
        paletteGraphicGRYbrightBW,
        paletteGraphicCMWdimBW,
        paletteGraphicCMWbrightBW};

static unsigned char palette[16] = {
    0x00, 0x73, 0xC3, 0xB6, 0x44, 0x65, 0x17, 0x0A,
    0x05, 0x78, 0xC8, 0xBB, 0x49, 0x6A, 0x1C, 0x0F};

typedef struct render_t
{
  bool pendingChanges;
  dumper_t dumper;
  uint32_t colCount;
  uint32_t frameCount;
  uint32_t paletteIndex;
  uint32_t specialColor;
  uint32_t hasColor;

  uint32_t charHeight;
  uint32_t lineCount;

  uint32_t startAddr;
  uint32_t hOffset;
  uint32_t blitter;
  vmode_t vmode;

  uint32_t colorburstOverride;
} render_t;

static render_t render;
static render_t pendingRender;

CompositeColorOutput composite;

char **bufferNTSC;

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

void renderInit()
{
  render.pendingChanges = false;
  render.dumper = dump80x25;
  render.colCount = 80;
  render.frameCount = 0;
  render.paletteIndex = 0;
  render.specialColor = 0;
  render.hasColor = COLORBURST_NO_CHANGE;
  render.charHeight = 8;
  render.lineCount = 25;
  render.startAddr = 0;
  render.hOffset = 22;
  render.vmode = TEXT;
  render.colorburstOverride = COLORBURST_NO_CHANGE;

  memcpy(&pendingRender, &render, sizeof(render_t));

  bufferNTSC = (char **)malloc(CompositeColorOutput::YRES * sizeof(char *));
  for (int y = 0; y < CompositeColorOutput::YRES; y++)
  {
    bufferNTSC[y] = (char *)malloc(CompositeColorOutput::XRES * 2);
    memset(bufferNTSC[y], 0x00, CompositeColorOutput::XRES * 2);
  }

  void IRAM_ATTR blitter_0(uint8_t * src, uint16_t * dst);
  void IRAM_ATTR blitter_1(uint8_t * src, uint16_t * dst);
  composite.init(&bufferNTSC);
}

void renderExec()
{
  if (render.hasColor != COLORBURST_NO_CHANGE)
  {
    composite.setColorburstEnabled(render.hasColor == COLORBURST_ENABLE);
    render.hasColor = COLORBURST_NO_CHANGE;
  }
}

void draw();
extern void handleinput();

void VideoThreadPoll()
{
  draw();
}

//*****************************************

static __always_inline void OnDumpDone()
{
  if (pendingRender.pendingChanges)
  {
    pendingRender.pendingChanges = false;
    memcpy(&render, &pendingRender, sizeof(render_t));
    renderUpdateBorder();
  }
}

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
      printChar(aChar, (x << 3), (y * render.charHeight), aColor, aBgColor); // Sin capturadora
      src++;
    }
  }
  OnDumpDone();
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
      printChar(aChar, (x << 3), (y * render.charHeight), aColor, aBgColor); // Sin capturadora
      src++;
    }
  }
  OnDumpDone();
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
      uint8_t bPixel3 = (src & 0x03);
      src >>= 2;
      uint8_t bPixel2 = (src & 0x03);
      src >>= 2;
      uint8_t bPixel1 = (src & 0x03);
      src >>= 2;
      uint8_t bPixel0 = (src & 0x03);

      line[offset++] = palette[bPixel0];
      line[offset++] = palette[bPixel1];
      line[offset++] = palette[bPixel2];
      line[offset++] = palette[bPixel3];
      cont++;
    }
    uint32_t *dest = (uint32_t *)bufferNTSC[yDest + VERTICAL_OFFSET];
    memcpy(dest + render.hOffset, line, 320);
  }

  cont = 0x2000;
  for (uint32_t y = 0; y < 100; y++)
  {
    uint32_t yDest = (y << 1) + 1;
    uint32_t offset = INITIAL_OFFSET;
    for (uint32_t x = 0; x < 80; x++)
    {
      uint8_t src = gb_video_cga[cont];
      uint8_t bPixel3 = (src & 0x03);
      src >>= 2;
      uint8_t bPixel2 = (src & 0x03);
      src >>= 2;
      uint8_t bPixel1 = (src & 0x03);
      src >>= 2;
      uint8_t bPixel0 = (src & 0x03);

      line[offset++] = palette[bPixel0];
      line[offset++] = palette[bPixel1];
      line[offset++] = palette[bPixel2];
      line[offset++] = palette[bPixel3];
      cont++;
    }
    uint32_t *dest = (uint32_t *)bufferNTSC[yDest + VERTICAL_OFFSET];
    memcpy(dest + render.hOffset, line, 320);
  }
  OnDumpDone();
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
    yDest = (y << 1);
    uint32_t offset = INITIAL_OFFSET;
    for (x = 0; x < 80; x++)
    {
      unsigned char src = gb_video_cga[srcAddr];
      uint8_t a7 = (src & 0x01);
      src >>= 1;
      uint8_t a6 = (src & 0x01);
      src >>= 1;
      uint8_t a5 = (src & 0x01);
      src >>= 1;
      uint8_t a4 = (src & 0x01);
      src >>= 1;
      uint8_t a3 = (src & 0x01);
      src >>= 1;
      uint8_t a2 = (src & 0x01);
      src >>= 1;
      uint8_t a1 = (src & 0x01);
      src >>= 1;
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
    dest = (uint32_t *)bufferNTSC[yDest + VERTICAL_OFFSET];
    memcpy(dest + render.hOffset, line, 640);
  }

  srcAddr = 0x2000;
  for (uint32_t y = 0; y < 100; y++)
  {
    yDest = (y << 1) + 1;
    uint32_t offset = INITIAL_OFFSET;
    for (x = 0; x < 80; x++)
    {
      unsigned char src = gb_video_cga[srcAddr];
      uint8_t a7 = (src & 0x01);
      src >>= 1;
      uint8_t a6 = (src & 0x01);
      src >>= 1;
      uint8_t a5 = (src & 0x01);
      src >>= 1;
      uint8_t a4 = (src & 0x01);
      src >>= 1;
      uint8_t a3 = (src & 0x01);
      src >>= 1;
      uint8_t a2 = (src & 0x01);
      src >>= 1;
      uint8_t a1 = (src & 0x01);
      src >>= 1;
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
    dest = (unsigned int *)bufferNTSC[yDest + VERTICAL_OFFSET];
    memcpy(dest + render.hOffset, line, 640);
  }
  OnDumpDone();
}

//*************************************************************************************
static void printChar_c(char code, uint32_t x, uint32_t y, uint8_t color, uint8_t backcolor)
{
  int nBaseOffset = code << 3;
  const bool cbTime = render.frameCount & 0x04;
  for (unsigned int row = 0; row < render.charHeight; row++)
  {
    const bool cbFill = (row >= cursor.getStart()) && (row <= cursor.getEnd()) && cbTime;
    unsigned char bLine = ((row >= 6) && (cbTime)) ? 0xFF : font[nBaseOffset + row];
    for (int col = 0; col < render.charHeight; col++)
    {
      unsigned char Pixel = ((bLine >> col) & 0x01);
      const uint32_t vgaLine = y + row + VERTICAL_OFFSET;
      const uint32_t vgaCol = x - col + render.hOffset;
      bufferNTSC[vgaLine][vgaCol] = palette[(Pixel != 0) ? color : backcolor];
    }
  }
}

static void printChar(char code, uint32_t x, uint32_t y, uint8_t color, uint8_t backcolor)
{
  if ((x == (cursor.getCol() << 3)) && (y == (cursor.getRow() << 3)))
  {
    printChar_c(code, x, y, color, backcolor);
  }
  else
  {
    int nBaseOffset = code << 3; //*8
    for (unsigned int row = 0; row < render.charHeight; row++)
    {
      unsigned char bLine = font[nBaseOffset + row];
      for (int col = 0; col < 8; col++)
      {
        unsigned char Pixel = ((bLine >> col) & 0x01);
        const uint32_t vgaLine = y + row + VERTICAL_OFFSET;
        const uint32_t vgaCol = x - col + render.hOffset;
        bufferNTSC[vgaLine][vgaCol] = palette[(Pixel != 0) ? color : backcolor];
      }
    }
  }
}

void draw()
{
  render.frameCount++;
  render.dumper();
}

void renderSetCharHeight(uint8_t height)
{
  pendingRender.charHeight = height + 1;
  pendingRender.lineCount = EFFECTIVE_HEIGHT / pendingRender.charHeight;
  LOG("CharHeight=%i LineCount=%i\n", pendingRender.charHeight, pendingRender.lineCount);
}

void renderSetColorburstOverride(uint32_t value)
{
  render.colorburstOverride = value;
}
void renderSetStartAddr(uint32_t addr)
{
  render.startAddr = addr * 2;
}

void renderUpdateSettings(uint8_t settings, uint8_t colors)
{
  uint8_t _mode = (settings & 0x3) | ((settings >> 2) & 0x04);
  const bool colorEnabled = !(settings & 0x04); // || (settings & 10);
  const uint32_t requestedColor = colorEnabled ? COLORBURST_ENABLE : COLORBURST_DISABLE;
  pendingRender.hasColor = render.colorburstOverride == COLORBURST_NO_CHANGE ? requestedColor : render.colorburstOverride;
  pendingRender.dumper = modes[_mode].dumper;
  pendingRender.colCount = modes[_mode].textWidth;
  pendingRender.hOffset = modes[_mode].hOffset;
  pendingRender.blitter = modes[_mode].blitter;

  composite.setBlitter(modes[_mode].blitter);

  // Colors
  static const uint8_t COLOR_MASK = 0x0F;
  static const uint8_t PALETTE_POS = 4;
  static const uint8_t PALETTE_MASK = 0x03;
  uint32_t specialColor = colors & COLOR_MASK;
  uint32_t paletteIndex = (colors >> PALETTE_POS) & PALETTE_MASK;
  pendingRender.paletteIndex = paletteIndex;
  pendingRender.specialColor = specialColor;

  vmode_t vmode = !(settings & 0x02) ? TEXT : ((settings & 0x10) ? GRAPH_HI : GRAPH_LO);
  switch (vmode)
  {
  case TEXT:
    memcpy(palette, colorEnabled ? paletteBasic : paletteBasicBW, 16);
    break;
  case GRAPH_LO:
    memcpy(palette, colorEnabled ? graphPalettes[paletteIndex] : graphPalettesBW[paletteIndex], GRAPH_PALETTE_SIZE);
    palette[0] = paletteBasic[specialColor];
    break;
  case GRAPH_HI:
    palette[0] = 0;
    for (uint32_t item = 1; item < 16; item++)
      palette[item] = colorEnabled ? paletteBasic[specialColor] : paletteBasicBW[specialColor];
    break;
  }

  pendingRender.pendingChanges = true;
}

void renderUpdateBorder()
{
  uint32_t rightOrg = (render.blitter == 0) ? 655 : 328;
  uint32_t barWidthL = (render.blitter == 0) ? 15 : 8;
  uint32_t barWidthR = (render.blitter == 0) ? 17 : 8;
  uint32_t barHeight = 20;
  uint32_t barColor = 0;

  switch (render.vmode)
  {
  case TEXT:
  case GRAPH_LO:
    barColor = render.hasColor ? paletteBasic[render.specialColor] : paletteBasicBW[render.specialColor];
    break;
  case GRAPH_HI:
    barColor = 0;
    break;
  }

  for (int y = 0; y < barHeight; y++)
  {
    for (int x = 0; x < CompositeColorOutput::XRES << 1; x++)
      bufferNTSC[y][x] = barColor;
    for (int x = 0; x < CompositeColorOutput::XRES << 1; x++)
      bufferNTSC[y + VERTICAL_OFFSET + EFFECTIVE_HEIGHT][x] = barColor;
  }
  for (int y = 0; y < EFFECTIVE_HEIGHT; y++)
  {
    for (int x = 0; x < barWidthL; x++)
    {
      bufferNTSC[y + VERTICAL_OFFSET][x] = barColor;
    }
    for (int x = 0; x < barWidthR; x++)
    {
      bufferNTSC[y + VERTICAL_OFFSET][x + rightOrg] = barColor;
    }
  }
}
