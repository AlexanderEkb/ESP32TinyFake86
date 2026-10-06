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

/* 
 * ┌────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
 * │                                                  MODE SELECTION SUMMARY                                            │
 * ├───────────────────────┬───────┬─────────┬────────────┬─────────┬────────────┬┲━━━━━━━━━━━━┳━━━━━━━━┳━━━━━━━┳━━━━━━━┪
 * │                       │ 'H3D8 │ 'H3D8   │ 'H3DE REG3 │  'H3D8  │ 'H3DE REG3 │┃ 'H3DE REG3 ┃ 'H3DD  ┃ 'H3DF ┃ 'H3DF ┃
 * │     MODE              │ BIT 1 │  BIT 4  │    BIT 3   │  BIT 0  │    BIT 4   │┃    BIT 5   ┃ BIT 0  ┃ BIT 7 ┃ BIT 6 ┃
 * │                       │  GRPH │  HRESAD │   C4COLHR  │  HRESCK │    C16COL  │┃    NVDM    ┃ EXTADR ┃ ADRMl ┃ ADRM0 ┃
 * ├───────────────────────┼───────┼─────────┼────────────┼─────────┼────────────┼╊━━━━━━━━━━━━╋━━━━━━━━╋━━━━━━━╋━━━━━━━┫
 * │40X25 ALPHA     (0, 1) │   0   │     0   │       0    │     0   │       0    │┃     0      ┃    0   ┃    0  ┃   0   ┃ 4?
 * │80X25 ALPHA     (2, 3) │   0   │     0   │       0    │     1   │       0    │┃     0      ┃    0   ┃    0  ┃   0   ┃ 4?
 * ├───────────────────────┼───────┼─────────┼────────────┼─────────┼────────────┼╊━━━━━━━━━━━━╋━━━━━━━━╋━━━━━━━╋━━━━━━━┫
 * │320X200 4 COL   (4, 5) │   1   │     0   │       0    │     0   │       0    │┃     0      ┃    0   ┃    0  ┃   1   ┃ 4
 * │160X200 16 COL  (8)    │   1   │     0   │       0    │     0   │       1    │┃     0      ┃    0   ┃    0  ┃   1   ┃ 2
 * │320X200 16 COL  (9)    │   1   │     0   │       0    │     1   │       1    │┃     0      ┃    0   ┃    1  ┃   1   ┃ 2
 * │640X200 2 COL   (6)    │   1   │     1   │       0    │     0   │       0    │┃     0      ┃    0   ┃    0  ┃   1   ┃ 8
 * │640X200 4 COL   (A)    │   1   │     1   │       1    │     1   │       0    │┃     0      ┃    0   ┃    1  ┃   1   ┃ 4
 * └───────────────────────┴───────┴─────────┴────────────┴─────────┴────────────┴┺━━━━━━━━━━━━┻━━━━━━━━┻━━━━━━━┻━━━━━━━┛
 * 
 * HRESAD:  640 dot graphics. A logical 1 selects 640x200 (2 or 4 color)
 * GRPH:    Graphics selsct. Logical 0 is for alphanumeric modes 1 is for graphics.
 * HRESCK:  High Resolution Dot Clock. This bit controls the operating speed of the
 *          video system. A "0" selects the lower speed for 40 character text or
 *          low resolution graphics modes. A "1" selects high speed for 80 character
 *          text or high resolution graphics modes.
 * NVDM:    Set to 1 for 640x200 secondary pixel organization
 * C16COL:  Set to 1 for 16 color modes
 * C4COLHR: Set to 1 for 4 color 640x200 mode
 */

#include "../../machine_config.h"

#if (IBM_XT_VIDEO_DRIVER == 1)

#include <stdio.h>
#include <string.h>
#include <Esp.h>
#include <esp_attr.h>
#include <esp32-hal-log.h>
#include "../gb_sdl_font8x8.h"
#include "tga_render.h"

#define TAG "render"
#define EFFECTIVE_HEIGHT (200)

uint8_t * tgaBuffer;
uint8_t * tgaCrtPage;
uint8_t * tgaCpuPage;

uint8_t tgaRender::palette[16];
uint32_t tgaRender::colorburstOverride;
tgaRender::render_t tgaRender::render;
tgaRender::render_t tgaRender::pendingRender;
cursor_t tgaRender::cursor;
uint8_t const * tgaRender::font;
uint8_t tgaRender::dumpLineBuffer[700];

constexpr uint8_t tgaRender::paletteHiRes[tgaRender::TGA_COLOR_COUNT];
constexpr uint8_t tgaRender::paletteLoRes[tgaRender::TGA_COLOR_COUNT];
constexpr uint8_t tgaRender::paletteBW[tgaRender::TGA_COLOR_COUNT];
constexpr uint8_t tgaRender::paletteGraphicGRYdim[tgaRender::GRAPH_PALETTE_SIZE];
constexpr uint8_t tgaRender::paletteGraphicGRYdimBW[tgaRender::GRAPH_PALETTE_SIZE];
constexpr uint8_t tgaRender::paletteGraphicGRYbright[tgaRender::GRAPH_PALETTE_SIZE];
constexpr uint8_t tgaRender::paletteGraphicGRYbrightBW[tgaRender::GRAPH_PALETTE_SIZE];
constexpr uint8_t tgaRender::paletteGraphicCMWdim[tgaRender::GRAPH_PALETTE_SIZE];
constexpr uint8_t tgaRender::paletteGraphicCMWdimBW[tgaRender::GRAPH_PALETTE_SIZE];
constexpr uint8_t tgaRender::paletteGraphicCMWbright[tgaRender::GRAPH_PALETTE_SIZE];
constexpr uint8_t tgaRender::paletteGraphicCMWbrightBW[tgaRender::GRAPH_PALETTE_SIZE];
constexpr uint8_t const * tgaRender::graphPalettes[tgaRender::GRAPH_PALETTE_COUNT];
constexpr uint8_t const * tgaRender::graphPalettesBW[tgaRender::GRAPH_PALETTE_COUNT];

constexpr tgaRender::videoMode_t tgaRender::modes[tgaRender::MODE_COUNT];

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
  row = value / tgaRender::render.textColCount;
  col = value % tgaRender::render.textColCount;
};

void tgaRender::init(void)
{
  // heap_caps_dump(uint32_t MALLOC_CAP_INTERNAL);
  ESP_LOGI(TAG, "Heap state is %lu (%lu max)", ESP.getFreeHeap(), ESP.getMaxAllocHeap());
  uint32_t const attr = MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT;
  tgaBuffer = reinterpret_cast<uint8_t *>(heap_caps_malloc(VIDEO_MEMORY_SIZE, attr));
  tgaCpuPage = tgaBuffer;
  tgaCrtPage = tgaBuffer;

  font = getFont();
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

  render.hOffset = 0;
  render.pixelsPerLine = 640;
  render.horizontalPosition = 16;
  render.rightBorderPosition = 656;
  render.rightBorderWidth = 16;

  render.blitter = BLITTER_HIRES;
  render.vmode = TEXT_HI;

  render.frameCount = 0;

  colorburstOverride = COLORBURST_NO_CHANGE;

  memcpy(&pendingRender, &render, sizeof(render_t));

  void IRAM_ATTR blitter_0(uint8_t * src, uint16_t * dst);
  void IRAM_ATTR blitter_1(uint8_t * src, uint16_t * dst);
}

void tgaRender::deinit(void)
{

}

/* 
 * ┌────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
 * │                                                  MODE SELECTION SUMMARY                                            │
 * ├───────────────────────┬───────┬─────────┬────────────┬─────────┬────────────┬┲━━━━━━━━━━━━┳━━━━━━━━┳━━━━━━━┳━━━━━━━┪
 * │                       │ 'H3D8 │ 'H3D8   │ 'H3DE REG3 │  'H3D8  │ 'H3DE REG3 │┃ 'H3DE REG3 ┃ 'H3DD  ┃ 'H3DF ┃ 'H3DF ┃
 * │     MODE              │ BIT 1 │  BIT 4  │    BIT 3   │  BIT 0  │    BIT 4   │┃    BIT 5   ┃ BIT 0  ┃ BIT 7 ┃ BIT 6 ┃
 * │                       │  GRPH │  HRESAD │   C4COLHR  │  HRESCK │    C16COL  │┃    NVDM    ┃ EXTADR ┃ ADRMl ┃ ADRM0 ┃
 * ├───────────────────────┼───────┼─────────┼────────────┼─────────┼────────────┼╊━━━━━━━━━━━━╋━━━━━━━━╋━━━━━━━╋━━━━━━━┫
 * │40X25 ALPHA     (0, 1) │   0   │     0   │       0    │     0   │       0    │┃     0      ┃    0   ┃    0  ┃   0   ┃ 0x00 40X25 ALPHA     (0, 1)
 * │80X25 ALPHA     (2, 3) │   0   │     0   │       0    │     1   │       0    │┃     0      ┃    0   ┃    0  ┃   0   ┃ 0x02 80X25 ALPHA     (2, 3)
 * ├───────────────────────┼───────┼─────────┼────────────┼─────────┼────────────┼╊━━━━━━━━━━━━╋━━━━━━━━╋━━━━━━━╋━━━━━━━┫ 
 * │320X200 4 COL   (4, 5) │   1   │     0   │       0    │     0   │       0    │┃     0      ┃    0   ┃    0  ┃   1   ┃ 0x10 320X200 4 COL   (4, 5)
 * │160X200 16 COL  (8)    │   1   │     0   │       0    │     0   │       1    │┃     0      ┃    0   ┃    0  ┃   1   ┃ 0x11 160X200 16 COL  (8)   
 * │320X200 16 COL  (9)    │   1   │     0   │       0    │     1   │       1    │┃     0      ┃    0   ┃    1  ┃   1   ┃ 0x13 320X200 16 COL  (9)   
 * │640X200 2 COL   (6)    │   1   │     1   │       0    │     0   │       0    │┃     0      ┃    0   ┃    0  ┃   1   ┃ 0x18 640X200 2 COL   (6)   
 * │640X200 4 COL   (A)    │   1   │     1   │       1    │     1   │       0    │┃     0      ┃    0   ┃    1  ┃   1   ┃ 0x1E 640X200 4 COL   (A)   
 * └───────────────────────┴───────┴─────────┴────────────┴─────────┴────────────┴┺━━━━━━━━━━━━┻━━━━━━━━┻━━━━━━━┻━━━━━━━┛
 * 
 * HRESAD:  640 dot graphics. A logical 1 selects 640x200 (2 or 4 color)
 * GRPH:    Graphics selsct. Logical 0 is for alphanumeric modes 1 is for graphics.
 * HRESCK:  High Resolution Dot Clock. This bit controls the operating speed of the
 *          video system. A "0" selects the lower speed for 40 character text or
 *          low resolution graphics modes. A "1" selects high speed for 80 character
 *          text or high resolution graphics modes.
 * NVDM:    Set to 1 for 640x200 secondary pixel organization
 * C16COL:  Set to 1 for 16 color modes
 * C4COLHR: Set to 1 for 4 color 640x200 mode
 */
void tgaRender::updateSettings(uint8_t reg3D8h, uint8_t reg3D9h, uint8_t reg3DEh_3)
{
  uint32_t const width = Host::video->width();
  // LOG("renderUpdateSettings(%02X, %02X)\n", settings, colors);
  // uint8_t _mode                     = (reg3D8 & 0x03) | ((reg3D8 >> 2) & 0x04);
  uint8_t mode = 0;
  if (reg3DEh_3 & 0x10) mode |= 0x01;
  if (reg3D8h   & 0x01) mode |= 0x02;
  if (reg3DEh_3 & 0x08) mode |= 0x04;
  if (reg3D8h   & 0x10) mode |= 0x08;
  if (reg3D8h   & 0x02) mode |= 0x10;
  ESP_LOGI(TAG, "mode=%lu", mode);
  const bool colorSuppressed        = (reg3D8h & 0x04);
  pendingRender.hasColor            = colorSuppressed ? COLORBURST_DISABLE : COLORBURST_ENABLE;
  pendingRender.dumper              = modes[mode].dumper;
  pendingRender.textColCount        = modes[mode].textColCount;
  pendingRender.hOffset             = modes[mode].hOffset;
  pendingRender.blitter             = modes[mode].blitter;

  const uint32_t pixelsPerLine      = (modes[mode].blitter == BLITTER_HIRES) ? 640 : 320;
  const uint32_t effectiveWidth     = width * ((modes[mode].blitter == BLITTER_HIRES) ? 2 : 1);
  pendingRender.pixelsPerLine       = pixelsPerLine;
  pendingRender.horizontalPosition  = ((effectiveWidth - pixelsPerLine) >> 1) + modes[mode].hOffset;
  pendingRender.rightBorderPosition = pendingRender.horizontalPosition + pixelsPerLine;
  pendingRender.rightBorderWidth    = effectiveWidth - pendingRender.rightBorderPosition;

  Host::video->miscCmd(SET_BLITTER, modes[mode].blitter);

  // Colors
  static const uint8_t COLOR_MASK = 0x0F;
  pendingRender.specialColor = reg3D9h & COLOR_MASK;

  static const uint8_t PALETTE_POS = 4;
  static const uint8_t PALETTE_MASK = 0x03;

  pendingRender.paletteIndex = (reg3D9h >> PALETTE_POS) & PALETTE_MASK;
  if(!(reg3D8h & 0x02))
  {
    pendingRender.vmode = (reg3D8h & 0x01) ? TEXT_HI : TEXT_LO;
  }
  else if(reg3D8h & 0x10)
  {
    pendingRender.vmode = (reg3DEh_3 & 0x08) ? GRAPH_HI_4 : GRAPH_HI_2;
  }
  else
  {
    pendingRender.vmode = (reg3DEh_3 & 0x10) ? ((reg3D8h & 0x01) ? GRAPH_LO_16 : GRAPH_ULTRALO) : GRAPH_LO_4;
  }
  pendingRender.pendingChanges = true;
}

void tgaRender::setCharHeight(uint8_t height)
{
  pendingRender.textCharHeight = height + 1;
  pendingRender.textRowCount = EFFECTIVE_HEIGHT / pendingRender.textCharHeight;
}

void tgaRender::setColorburstOverride(uint32_t value)
{
  colorburstOverride = value;
  memcpy(&render, &pendingRender, sizeof(render_t));
  pendingRender.pendingChanges = true;
}

void tgaRender::setStartAddr(uint32_t addr)
{
  render.startAddr = addr * 2;
}

void tgaRender::updateBorder()
{
  uint32_t barHeight = 20;
  uint32_t barColor = 0;

  switch(render.vmode)
  {
    case TEXT_HI:
      barColor = render.hasColor ? paletteHiRes[render.specialColor] : paletteBW[render.specialColor];
      break;
    case GRAPH_HI_2:
    case GRAPH_HI_4:
      barColor = 0;
      break;
    default:
      barColor = render.hasColor ? paletteLoRes[render.specialColor] : paletteBW[render.specialColor];
  }

  uint32_t const width = Host::video->width();
  for (int y = 0; y < barHeight; y++)
  {
    uint8_t * topLine = Host::video->scanline(y);
    uint8_t * botLine = Host::video->scanline(y + VERTICAL_OFFSET + EFFECTIVE_HEIGHT);
    for (int x = 0; x < width << 1; x++)
    {
      topLine[x] = barColor;
      botLine[x] = barColor;
    }
  }
  for (int y = 0; y < EFFECTIVE_HEIGHT; y++)
  {
    uint8_t * line = Host::video->scanline(y + VERTICAL_OFFSET);
    for (int x = 0; x < render.horizontalPosition; x++)
      // bufferNTSC[y + VERTICAL_OFFSET][x] = barColor;
      line[x] = barColor;
    for (int x = 0; x < render.rightBorderWidth; x++)
      // bufferNTSC[y + VERTICAL_OFFSET][x + render.rightBorderPosition] = barColor;
      line[x + render.rightBorderPosition] = barColor;
  }
}

void tgaRender::setCursorStart(uint8_t line)
{
  cursor.updateStart(line);
}

void tgaRender::setCursorEnd(uint8_t line)
{
  cursor.updateEnd(line);
}

void tgaRender::setCursorAddrMSB(uint8_t addr)
{
  cursor.updateMSB(addr);
}

void tgaRender::setCursorAddrLSB(uint8_t addr)
{
  cursor.updateLSB(addr);
}


// void draw();

// void VideoThreadPoll()
// {
//   draw();
// }

#endif /* IBM_XT_VIDEO_DRIVER */