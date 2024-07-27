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

static const uint32_t VERTICAL_OFFSET = 20;

typedef void (*dumper_t)(void);

static void dump80x25(void);
static void dump40x25(void);
static void dump320x200(void);
static void dump640x200(void);

static void printChar_c(char code, uint32_t x, uint32_t y, uint8_t color, uint8_t backcolor);
static void printChar(char code, uint32_t x, uint32_t y, uint8_t color, uint8_t backcolor);

static const uint32_t MODE_COUNT = 8;

static uint8_t const *const font = getFont();

static const uint8_t paletteHiRes[16] = {
 // BLACK   BLUE    GREEN   CYAN    RED     MGNTA   YELLOW  WHITE
    0x00,   0x86,   0xC8,   0xB9,   0x36,   0x57,   0xE7,   0x0A,
    0x05,   0x88,   0xCA,   0xBB,   0x39,   0x5A,   0xEB,   0x0F};
static const uint8_t paletteLoRes[16] = {
// BLACK   BLUE    GREEN   CYAN    RED     MGNTA   YELLOW  WHITE
    0x00,   0x46,   0x97,   0x79,   0xC4,   0x15,   0xA7,   0x0A,
    0x05,   0x48,   0x99,   0x7B,   0xC9,   0x1A,   0xAC,   0x0F};
static const uint8_t paletteBW[16] = {
// BLACK   BLUE    GREEN   CYAN    RED     MGNTA   YELLOW  WHITE
    0x00,   0x01,   0x05,   0x06,   0x02,   0x04,   0x07,   0x0C,
    0x03,   0x08,   0x0B,   0x0D,   0x09,   0x0A,   0x0E,   0x0F};
/*
    00      17      44      5B      24      3A      65      A9
    00      01      05      06      02      04      07      0C

    39      78      A4      BD      84      9B      C7      FF
    03      08      0B      0D      09      0A      0E      0F
*/

static const uint32_t GRAPH_PALETTE_COUNT = 4;
static const uint32_t GRAPH_PALETTE_SIZE = 4;

static const uint32_t GREEN   = 2;
static const uint32_t RED     = 4;
static const uint32_t YELLOW  = 6;
static const uint32_t CYAN    = 3;
static const uint32_t MAGENTA = 5;
static const uint32_t WHITE   = 7;
static const uint32_t BRIGHT  = 8;
const uint8_t paletteGraphicGRYdim[GRAPH_PALETTE_SIZE]      = {0x00, paletteLoRes[GREEN], paletteLoRes[RED], paletteLoRes[YELLOW]};
const uint8_t paletteGraphicGRYdimBW[GRAPH_PALETTE_SIZE]    = {0x00, paletteBW[GREEN], paletteBW[RED], paletteBW[YELLOW]};
const uint8_t paletteGraphicGRYbright[GRAPH_PALETTE_SIZE]   = {0x00, paletteLoRes[BRIGHT + GREEN], paletteLoRes[BRIGHT + RED], paletteLoRes[BRIGHT + YELLOW]};
const uint8_t paletteGraphicGRYbrightBW[GRAPH_PALETTE_SIZE] = {0x00, paletteBW[BRIGHT + GREEN], paletteBW[BRIGHT + RED], paletteBW[BRIGHT + YELLOW]};
const uint8_t paletteGraphicCMWdim[GRAPH_PALETTE_SIZE]      = {0x00, paletteLoRes[CYAN], paletteLoRes[MAGENTA], paletteLoRes[WHITE]};
const uint8_t paletteGraphicCMWdimBW[GRAPH_PALETTE_SIZE]    = {0x00, paletteBW[CYAN], paletteBW[MAGENTA], paletteBW[WHITE]};
const uint8_t paletteGraphicCMWbright[GRAPH_PALETTE_SIZE]   = {0x00, paletteLoRes[BRIGHT + CYAN], paletteLoRes[BRIGHT + MAGENTA], paletteLoRes[BRIGHT + WHITE]};
const uint8_t paletteGraphicCMWbrightBW[GRAPH_PALETTE_SIZE] = {0x00, paletteBW[BRIGHT + CYAN], paletteBW[BRIGHT + MAGENTA], paletteBW[BRIGHT + WHITE]};

uint8_t const* graphPalettes[GRAPH_PALETTE_COUNT] =
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

typedef enum vmode_t
{
  TEXT_LO,
  TEXT_HI,
  GRAPH_LO,
  GRAPH_HI
} vmode_t;

typedef struct
{
  /// @brief Characters per line. Only has effect in text modes
  uint32_t textColCount;
  /// @brief Pointer to an appropriate dumper function
  dumper_t dumper;
  /// @brief Index of an appropriate blitter function
  uint32_t blitter;
  /// @brief Horizontal image offset. Very important for both horizontal position and accuracy of artifact colors.
  uint32_t hOffset;
} videoMode_t;

const videoMode_t modes[MODE_COUNT] = {
    {40, dump40x25, BLITTER_LORES,    16},
    {80, dump80x25, BLITTER_HIRES,    22},
    {40, dump320x200, BLITTER_LORES,  8},
    {40, dump320x200, BLITTER_LORES,  8},
    {40, dump40x25, BLITTER_LORES,    16},
    {80, dump80x25, BLITTER_HIRES,    22},
    {80, dump640x200, BLITTER_HIRES,  1},
    {80, dump640x200, BLITTER_HIRES,  1}};

class cursor_t {
  public:
    static void updateMSB(uint8_t MSB);
    static void updateLSB(uint8_t LSB);
    static void updateStart(uint32_t startLine);
    static void updateEnd(uint32_t endLine);
    static uint32_t getCol();
    static uint32_t getRow();
    static uint32_t getStart();
    static uint32_t getEnd();
  private:
    static uint32_t row;
    static uint32_t col;
    static uint32_t value;
    static uint32_t start;
    static uint32_t end;
    static void updatePosition();
};

cursor_t cursor;
static uint32_t scanlineBuffer[CompositeColorOutput::XRES * 2];

typedef struct render_t
{
  /// @brief true if some changes are made by the 'video' part. In such a case
  ///        OnDumpDone() function performs these changes and pendingRender contents
  //         is copied to the render var.
  bool pendingChanges;

  /// @brief pointer to a dumper function, which transforms CGA video memory contents
  ///        to the format accepted by the rendering module.
  dumper_t dumper;
  uint32_t paletteIndex;
  uint32_t specialColor;
  uint32_t hasColor;

  uint32_t textCharHeight;
  uint32_t textRowCount;
  uint32_t textColCount;

  uint32_t startAddr;
  uint32_t hOffset;
  uint32_t blitter;
  vmode_t vmode;

  uint32_t frameCount;
} render_t;

static uint32_t colorburstOverride;
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
  row = value / render.textColCount;
  col = value % render.textColCount;
};

void renderInit()
{
  memcpy(palette, paletteHiRes, sizeof(palette));

  render.pendingChanges = false;
  render.dumper = dump80x25;
  render.paletteIndex = 0;
  render.specialColor = 0;
  render.hasColor = COLORBURST_ENABLE;

  render.textCharHeight = 8;
  render.textRowCount = 25;
  render.textColCount = 80;

  render.startAddr = 0;
  render.hOffset = 22;
  render.blitter = BLITTER_HIRES;
  render.vmode = TEXT_HI;

  render.frameCount = 0;

  colorburstOverride = COLORBURST_NO_CHANGE;

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

    bool colorEnabled = (colorburstOverride == COLORBURST_NO_CHANGE) ? (render.hasColor != COLORBURST_DISABLE) : (colorburstOverride != COLORBURST_DISABLE);
    composite.setColorburstEnabled(colorEnabled);
    switch (render.vmode)
    {
    case TEXT_LO:
      memcpy(palette, colorEnabled ? paletteLoRes : paletteBW, 16);
      break;
    case TEXT_HI:
      memcpy(palette, colorEnabled ? paletteHiRes : paletteBW, 16);
      break;
    case GRAPH_LO:
      memcpy(palette, colorEnabled ? graphPalettes[render.paletteIndex] : graphPalettesBW[render.paletteIndex], GRAPH_PALETTE_SIZE);
      palette[0] = paletteLoRes[render.specialColor];
      break;
    case GRAPH_HI:
      palette[0] = 0;
      palette[1] = colorEnabled ? paletteHiRes[render.specialColor] : paletteBW[render.specialColor];
      break;
    }

    renderUpdateBorder();
  }
}

static void dump80x25()
{
  uint8_t aColor, aBgColor, aChar;
  uint32_t src = render.startAddr;
  for (uint32_t y = 0; y < render.textRowCount; y++)
  {
    for (uint32_t x = 0; x < 80; x++)
    {
      aChar = gb_video_cga[src];
      src++;
      aColor = gb_video_cga[src] & 0x0F;
      aBgColor = ((gb_video_cga[src] >> 4) & 0x07);
      printChar(aChar, (x << 3), (y * render.textCharHeight), aColor, aBgColor); // Sin capturadora
      src++;
    }
  }
  OnDumpDone();
}

static void dump40x25()
{
  uint32_t src = render.startAddr;
  for (uint32_t y = 0; y < render.textRowCount; y++)
  {
    for (uint32_t x = 0; x < 40; x++)
    {
      uint8_t aChar = gb_video_cga[src];
      src++;
      uint8_t aColor = gb_video_cga[src] & 0x0F;
      uint8_t aBgColor = ((gb_video_cga[src] >> 4) & 0x07);
      printChar(aChar, (x << 3), (y * render.textCharHeight), aColor, aBgColor); // Sin capturadora
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
    memcpy((void *)dest + render.hOffset, line, 320);
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
    memcpy((void *)dest + render.hOffset, line, 320);
  }
  OnDumpDone();
}

static void dump640x200()
{
  static uint8_t line[700];
  static uint32_t *dest;
  static const uint32_t INITIAL_OFFSET = 0;
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
    memcpy((void *)dest + render.hOffset, line, 640);
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
    memcpy((void *)dest + render.hOffset, line, 640);
  }
  OnDumpDone();
}

//*************************************************************************************
static void printChar_c(char code, uint32_t x, uint32_t y, uint8_t color, uint8_t backcolor)
{
  int nBaseOffset = code << 3;
  const bool cbTime = render.frameCount & 0x04;
  for (unsigned int row = 0; row < render.textCharHeight; row++)
  {
    const bool cbFill = (row >= cursor.getStart()) && (row <= cursor.getEnd()) && cbTime;
    unsigned char bLine = ((row >= 6) && (cbTime)) ? 0xFF : font[nBaseOffset + row];
    for (int col = 0; col < render.textCharHeight; col++)
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
    for (unsigned int row = 0; row < render.textCharHeight; row++)
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
  pendingRender.textCharHeight = height + 1;
  pendingRender.textRowCount = EFFECTIVE_HEIGHT / pendingRender.textCharHeight;
  // LOG("CharHeight=%i LineCount=%i\n", pendingRender.textCharHeight, pendingRender.textRowCount);
}

void renderSetColorburstOverride(uint32_t value)
{
  colorburstOverride = value;
  memcpy(&render, &pendingRender, sizeof(render_t));
  pendingRender.pendingChanges = true;
}

void renderSetStartAddr(uint32_t addr)
{
  render.startAddr = addr * 2;
}

void renderSetCursorStart(uint8_t line)
{
  cursor.updateStart(line);
}

void renderSetCursorEnd(uint8_t line)
{
  cursor.updateEnd(line);
}

void renderSetCursorAddrMSB(uint8_t addr)
{
  cursor.updateMSB(addr);
}

void renderSetCursorAddrLSB(uint8_t addr)
{
  cursor.updateLSB(addr);
}

void renderUpdateSettings(uint8_t settings, uint8_t colors)
{
  // LOG("renderUpdateSettings(%02X, %02X)\n", settings, colors);
  uint8_t _mode                 = (settings & 0x03) | ((settings >> 2) & 0x04);
  const bool colorSuppressed    = (settings & 0x04);
  pendingRender.hasColor        = colorSuppressed ? COLORBURST_DISABLE : COLORBURST_ENABLE;
  pendingRender.dumper          = modes[_mode].dumper;
  pendingRender.textColCount    = modes[_mode].textColCount;
  pendingRender.hOffset         = modes[_mode].hOffset;
  pendingRender.blitter         = modes[_mode].blitter;

  composite.setBlitter(modes[_mode].blitter);

  // Colors
  static const uint8_t COLOR_MASK = 0x0F;
  pendingRender.specialColor = colors & COLOR_MASK;

  static const uint8_t PALETTE_POS = 4;
  static const uint8_t PALETTE_MASK = 0x03;

  pendingRender.paletteIndex = (colors >> PALETTE_POS) & PALETTE_MASK;
  pendingRender.vmode        = !(settings & 0x02) ? ((settings & 0x01) ? TEXT_HI : TEXT_LO) : ((settings & 0x10) ? GRAPH_HI : GRAPH_LO);

  pendingRender.pendingChanges = true;
}

void renderUpdateBorder()
{
  uint32_t rightOrg = (render.blitter == 0) ? 655 : 328;
  uint32_t barWidthL = (render.blitter == 0) ? 15 : 8;
  uint32_t barWidthR = (render.blitter == 0) ? 17 : 8;
  uint32_t barHeight = 20;
  uint32_t barColor = 0;

  switch(render.vmode)
  {
    case TEXT_HI:
      barColor = render.hasColor ? paletteHiRes[render.specialColor] : paletteBW[render.specialColor];
      break;
    case GRAPH_HI:
      barColor = 0;
      break;
    default:
      barColor = render.hasColor ? paletteLoRes[render.specialColor] : paletteBW[render.specialColor];
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
