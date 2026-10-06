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
//
// video.c: many various functions to emulate bits of the video controller.
//   a lot of this code is inefficient, and just plain ugly. i plan to rework
//   large sections of it soon.
/*
 * 0x3D8 port:
 * ===========
 * bit 7:
 * bit 6:
 * bit 5: blinking (1 - enable)
 * bit 4: hi-res graphics. No effect in text modes.
 * bit 3: enable video output
 * bit 2: disable colorburst / select 3rd palette on RGB display
 * bit 1: 1 - graphics, 0 - text
 * bit 0: 80-column text mode
 * 
 * 0x3D9 port:
 * ===========
 * bit 7:
 * bit 6:
 * bit 5: 320x200 modes, palette. 1 - MCW, 0 - RGY
 * bit 4: 320x200 modes, bright foreground.
 * bit 3: | text modes: border
 * bit 2: | 320x200: border/background
 * bit 1: | 640x200: foreground
 * bit 0: |
 * 
 * 0x3DA port:
 * ===========
 * bit 7:
 * bit 6:
 * bit 5: 
 * bit 4: 
 * bit 3: vertical retrace. Meaning is similar to bit 0 whatever it means.
 * bit 2: light pen switch status.
 * bit 1: light pen trigger is set.
 * bit 0: display enable. VRAM may be accesed with no afraid of "snow" effect.
 * 
*/

#include <esp32-hal-log.h>
#include "../../machine_config.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "tga_render.h"
#include "../../cpu/cpu.h"
#include "../../cpu/ports.h"

#define TAG "T_R"

#if (IBM_XT_VIDEO_DRIVER == 1)

#define PORT_3D8_BLINKING			(0x20)
#define PORT_3D8_HIRES_GRAPH	(0x10)
#define PORT_3D8_OE						(0x08)
#define PORT_3D8_NOCOLOR			(0x04)
#define PORT_3D8_GRAPHICS			(0x02)
#define PORT_3D8_80_COL_TEXT	(0x01)

#define MC6845_REG_HTOTAL           (0)
#define MC6845_REG_HDISP            (1)
#define MC6845_REG_HSYNC            (2)

#define MC6845_REG_VTOTAL           (4)
#define MC6845_REG_VTOTAL_ADJUST    (5)
#define MC6845_REG_VDISP_POS        (6)
#define MC6845_REG_VSYNC_POS        (7)

#define MC6845_REG_MAX_ROWS         (9)
#define MC6845_REG_CURSOS_START     (10)
#define MC6845_REG_CURSOR_END       (11)
#define MC6845_REG_START_ADDR_MSB   (12)
#define MC6845_REG_START_ADDR_LSB   (13)
#define MC6845_REG_CURSOR_ADDR_MSB  (14)
#define MC6845_REG_CURSOR_ADDR_LSB  (15)
#define MC6845_REG_LPEN_MSB         (16)
#define MC6845_REG_LPEN_LSB         (17)

extern uint8_t * tgaBuffer;
extern uint8_t * tgaCrtPage;
extern uint8_t * tgaCpuPage;

static uint8_t  readDummy(uint32_t portnum);
static void     write3D4h(uint32_t portnum, uint8_t value);
static uint8_t  read3D5h(uint32_t portnum);
static void     write3D5h(uint32_t portnum, uint8_t value);
static void     write3D8h(uint32_t portnum, uint8_t value);
static void     write3D9h(uint32_t portnum, uint8_t value);
static uint8_t  read3DAh(uint32_t portnum);
static void     write3DAh(uint32_t portnum, uint8_t value);
static void     write3DDh(uint32_t portnum, uint8_t value);
static void     write3DEh(uint32_t portnum, uint8_t value);
static void     write3DFh(uint32_t portnum, uint8_t value);

IOPort port_3D4h = IOPort(0x3D4, 0xFF, nullptr, write3D4h);
IOPort port_3D5h = IOPort(0x3D5, 0xFF, read3D5h,  write3D5h);
IOPort port_3D8h = IOPort(0x3D8, 0xFF, nullptr,   write3D8h);
IOPort port_3D9h = IOPort(0x3D9, 0xFF, nullptr,   write3D9h);
IOPort port_3DAh = IOPort(0x3DA, 0xFF, read3DAh,  write3DAh);
IOPort port_3DBh = IOPort(0x3DB, 0xFF, readDummy, nullptr);
IOPort port_3DCh = IOPort(0x3DC, 0xFF, readDummy, nullptr);
IOPort port_3DDh = IOPort(0x3DD, 0xFF, nullptr, write3DDh);
IOPort port_3DEh = IOPort(0x3DE, 0xFF, nullptr, write3DEh);
IOPort port_3DFh = IOPort(0x3DF, 0xFF, nullptr, write3DFh);

static const uint32_t MC6845_REG_TOTAL    = 18;
static const uint32_t MC6845_REG_READABLE = 0x0D;
static uint8_t        port3D8h = 0;       // Some sort of local cache
static uint8_t        port3D9h = 0;       // Some sort of local cache
static uint8_t        port3DAh = 0;       // Some sort of local cache
static uint8_t        mc6845RegSelector;  // 3D4h port writes modify this var
static uint8_t        mc6845Registers[MC6845_REG_TOTAL];
static uint8_t        tgaRegSelector = 0;
static uint8_t        tgaPaletteMask = 0x0F;
static uint8_t        tgaBorderColor = 0x00;
static uint8_t        tgaModeControl = 0x00;
static uint8_t        tgaExtRamPageReg = 0x00;
static uint8_t        tgaCRT_CPUPageReg = 0x00;

static void write3D4h (uint32_t portnum, uint8_t value)
{
  (void)portnum;
  mc6845RegSelector = value;
}

static void write3D5h (uint32_t portnum, uint8_t value)
{
	(void)portnum;
  if(mc6845RegSelector < MC6845_REG_TOTAL)
    mc6845Registers[mc6845RegSelector] = value;
  switch (mc6845RegSelector)
  {
  case MC6845_REG_HTOTAL:
    // ESP_LOGI(TAG, "MC6845 write H_TOTAL: %02xh", value);
    break;
  case MC6845_REG_HDISP:
    // ESP_LOGI(TAG, "MC6845 write H_DISP %02xh", value);
    break;
  case MC6845_REG_HSYNC:
    // ESP_LOGI(TAG, "MC6845 write H_SYNC_W %02xh", value);
    break;
  case MC6845_REG_VTOTAL:
    // ESP_LOGI(TAG, "MC6845 write V_TOTAL %02xh", value);
    break;
  case MC6845_REG_VTOTAL_ADJUST:
    // ESP_LOGI(TAG, "MC6845 write V_TOTAL_ADJ %02xh", value);
    break;
  case MC6845_REG_VDISP_POS:
    // ESP_LOGI(TAG, "MC6845 write V_DISP %02xh", value);
    break;
  case MC6845_REG_VSYNC_POS:
    // ESP_LOGI(TAG, "MC6845 write V_SYNC_P %02xh", value);
    break;
  case MC6845_REG_MAX_ROWS:
    // ESP_LOGI(TAG, "MC6845 write MAX_ROWS %02xh", value);
    tgaRender::setCharHeight(value);
    break;
  case MC6845_REG_CURSOS_START:
    // ESP_LOGI(TAG, "MC6845 write CUR_START %02xh", value);
    tgaRender::setCursorStart(value);
    break;
  case MC6845_REG_CURSOR_END:
    // ESP_LOGI(TAG, "MC6845 write CUR_END %02xh", value);
    tgaRender::setCursorEnd(value);
    break;
  case MC6845_REG_START_ADDR_MSB:
    // ESP_LOGI(TAG, "MC6845 write START_M %02xh", value);
    tgaRender::setStartAddr((mc6845Registers[MC6845_REG_START_ADDR_MSB] << 8) | mc6845Registers[MC6845_REG_START_ADDR_LSB]);
    break;
  case MC6845_REG_START_ADDR_LSB:
    // ESP_LOGI(TAG, "MC6845 write START_L %02xh", value);
    tgaRender::setStartAddr((mc6845Registers[MC6845_REG_START_ADDR_MSB] << 8) | mc6845Registers[MC6845_REG_START_ADDR_LSB]);
    break;
  case MC6845_REG_CURSOR_ADDR_MSB:
    tgaRender::setCursorAddrMSB(value);
    break;
  case MC6845_REG_CURSOR_ADDR_LSB:
    tgaRender::setCursorAddrLSB(value);
    break;
  case MC6845_REG_LPEN_MSB:
    // ESP_LOGI(TAG, "MC6845 write LPEN_M %02xh", value);
    break;
  case MC6845_REG_LPEN_LSB:
    // ESP_LOGI(TAG, "MC6845 write LPEN_L %02xh", value);
    break;
  }
}

uint8_t read3D5h (uint32_t portnum)
{
  (void)portnum;
  uint8_t result;
  if(mc6845RegSelector < MC6845_REG_READABLE)
    result = 0;
  else
    result = mc6845Registers[mc6845RegSelector];
  // ESP_LOGI(TAG, "read 3D5h: %02xh", result);
	return result;
}

static void write3D8h(uint32_t portnum, uint8_t value)
{
  /*
  0x0A - graph, lores
  
  */
  (void)portnum;
  ESP_LOGI(TAG, "CGA_MODE = %02Xh", value);
  port3D8h = value;
  tgaRender::updateSettings(port3D8h, port3D9h, tgaModeControl);
}

static void write3D9h(uint32_t portnum, uint8_t value)
{
  (void)portnum;
  // ESP_LOGI(TAG, "3D9h = %02Xh", value);
  port3D9h = value;
  tgaRender::updateSettings(port3D8h, port3D9h, tgaModeControl);
}

static uint8_t read3DAh(uint32_t portnum)
{
  (void)portnum;
  static uint32_t const V_RETRACE = 0x08;
  static uint32_t const H_RETRACE = 0x01;
  static uint32_t retraceCounter = 0;
  uint8_t flags = 0;

  ++retraceCounter;
  retraceCounter %= 320;
  flags |= (retraceCounter & 0x04) ? H_RETRACE : 0x00;
  flags |= (retraceCounter == 0x04) ? V_RETRACE : 0x00;
  uint8_t const result = (port3DAh & 0xF6 | flags);
  // ESP_LOGI(TAG, "read 3DAh = %02X", result);
  return result;
}

void write3DAh(uint32_t portnum, uint8_t value)
{
  (void)portnum;
  tgaRegSelector = value & 0x1F;
}

void write3DDh(uint32_t portnum, uint8_t value)
{
  (void)portnum;
  ESP_LOGI(TAG, "EXT_RAM_Page = %02Xh (Unused?)", value);
  tgaExtRamPageReg = value;
}

void write3DEh(uint32_t portnum, uint8_t value)
{
  (void)portnum;
  switch(tgaRegSelector)
  {
    case 0x01:
      // ESP_LOGI(TAG, "PAL_MASK %02Xh", value);
      tgaPaletteMask = value;
      return;
    case 0x02:
      // ESP_LOGI(TAG, "BORDER %02Xh", value);
      tgaBorderColor = value & 0xDF;
      return;
    case 0x03:
      // ESP_LOGI(TAG, "TGA_MODE (3DEh_3) = %02Xh", tgaRegSelector, value);
      tgaModeControl = value & 0xFD;
      ESP_LOGI(TAG, "TGA_MODE = %02Xh", value);
      tgaRender::updateSettings(port3D8h, port3D9h, tgaModeControl);
      return;
    case 0x10 ... 0x1F:
      ESP_LOGI(TAG, "TGA_PAL[%02X] = %02Xh", tgaRegSelector, value);
      tgaRender::updatePalette(tgaRegSelector - 0x10, value);
  }
}

void write3DFh(uint32_t portnum, uint8_t value)
{
  (void)portnum;
  ESP_LOGI(TAG, "CRT_CPU_page %02Xh", value);
  tgaCrtPage = tgaBuffer + (4000 * (value & 0x07));
  tgaCpuPage = tgaBuffer + (4000 * ((value >> 3) & 0x07));
  tgaCRT_CPUPageReg = value;
}

static uint8_t readDummy(uint32_t portnum)
{
  return 0;
}

#endif /* IBM_XT_VIDEO_DRIVER */