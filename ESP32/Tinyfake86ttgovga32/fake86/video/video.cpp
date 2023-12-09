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
#include "video/video.h"
#include "config/gbConfig.h"
#include "cpu/cpu.h"
#include "cpu/ports.h"
#include "video/render.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

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

static uint8_t  read3D5h(uint32_t portnum);
static void     write3D8h(uint32_t portnum, uint8_t value);
static void     write3D9h(uint32_t portnum, uint8_t value);
static uint8_t  read3DAh(uint32_t portnum);
static uint8_t  readDummy(uint32_t portnum);
static void write3D4h(uint32_t portnum, uint8_t value);
static void write3D5h(uint32_t portnum, uint8_t value);

IOPort port_3D4h = IOPort(0x3D4, 0xFF, nullptr, write3D4h);
IOPort port_3D5h = IOPort(0x3D5, 0xFF, read3D5h,  write3D5h);
IOPort port_3D8h = IOPort(0x3D8, 0xFF, nullptr,   write3D8h);
IOPort port_3D9h = IOPort(0x3D9, 0xFF, nullptr,   write3D9h);
IOPort port_3DAh = IOPort(0x3DA, 0xFF, read3DAh,  nullptr);
IOPort port_3DBh = IOPort(0x3DB, 0xFF, readDummy, nullptr);
IOPort port_3DCh = IOPort(0x3DC, 0xFF, readDummy, nullptr);

static const uint32_t MC6845_REG_TOTAL    = 18;
static const uint32_t MC6845_REG_READABLE = 0x0D;
static uint8_t        port3D8h = 0; // Some sort of local cache
static uint8_t        port3D9h = 0;    // Some sort of local cache
static uint8_t        port3DAh = 0;    // Some sort of local cache
static uint8_t        mc6845RegSelector; // 3D4h port writes modifies this var
static uint8_t        mc6845Registers[MC6845_REG_TOTAL];

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
    LOG("MC6845 write reg %02xh: %02xh\n", mc6845RegSelector, value);
    break;
  case MC6845_REG_HDISP:
    LOG("MC6845 write reg %02xh: %02xh\n", mc6845RegSelector, value);
    break;
  case MC6845_REG_HSYNC:
    LOG("MC6845 write reg %02xh: %02xh\n", mc6845RegSelector, value);
    break;
  case MC6845_REG_VTOTAL:
    LOG("MC6845 write reg %02xh: %02xh\n", mc6845RegSelector, value);
    break;
  case MC6845_REG_VTOTAL_ADJUST:
    LOG("MC6845 write reg %02xh: %02xh\n", mc6845RegSelector, value);
    break;
  case MC6845_REG_VDISP_POS:
    LOG("MC6845 write reg %02xh: %02xh\n", mc6845RegSelector, value);
    break;
  case MC6845_REG_VSYNC_POS:
    LOG("MC6845 write reg %02xh: %02xh\n", mc6845RegSelector, value);
    break;
  case MC6845_REG_MAX_ROWS:
    LOG("MC6845 write reg %02xh: %02xh\n", mc6845RegSelector, value);
    renderSetCharHeight(value);
    break;
  case MC6845_REG_CURSOS_START:
    LOG("MC6845 write reg %02xh: %02xh\n", mc6845RegSelector, value);
    cursor.updateStart(value);
    break;
  case MC6845_REG_CURSOR_END:
    LOG("MC6845 write reg %02xh: %02xh\n", mc6845RegSelector, value);
    cursor.updateEnd(value);
    break;
  case MC6845_REG_START_ADDR_MSB:
    LOG("MC6845 write reg %02xh: %02xh\n", mc6845RegSelector, value);
    renderSetStartAddr((mc6845Registers[MC6845_REG_START_ADDR_MSB] << 8) | mc6845Registers[MC6845_REG_START_ADDR_LSB]);
    break;
  case MC6845_REG_START_ADDR_LSB:
    LOG("MC6845 write reg %02xh: %02xh\n", mc6845RegSelector, value);
    renderSetStartAddr((mc6845Registers[MC6845_REG_START_ADDR_MSB] << 8) | mc6845Registers[MC6845_REG_START_ADDR_LSB]);
    break;
  case MC6845_REG_CURSOR_ADDR_MSB:
    cursor.updateMSB(value);
    break;
  case MC6845_REG_CURSOR_ADDR_LSB:
    cursor.updateLSB(value);
    break;
  case MC6845_REG_LPEN_MSB:
    LOG("MC6845 write reg %02xh: %02xh\n", mc6845RegSelector, value);
    break;
  case MC6845_REG_LPEN_LSB:
    LOG("MC6845 write reg %02xh: %02xh\n", mc6845RegSelector, value);
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
	return result;
}

static uint8_t read3DAh(uint32_t portnum)
{
  (void)portnum;
  return (port3DAh);
}

static void write3D8h(uint32_t portnum, uint8_t value)
{
  /*
  0x0A - graph, lores
  
  */
  (void)portnum;
  LOG("write3D8h(%02x)\n", value);
  port3D8h = value;
  renderUpdateSettings(port3D8h);
}

static void write3D9h(uint32_t portnum, uint8_t value)
{
  (void)portnum;
  LOG("write3D9h(%02x)\n", value);
  port3D9h = value;
  renderUpdateColor(port3D9h);
}

static uint8_t readDummy(uint32_t portnum)
{
  return 0;
}

/// @brief Handles CGA retrace bits in 3DAh port. Gets called each 32th CPU instruction executed.
/// @param  none
void videoExecCpu(void)
{
  static const  uint8_t   CGA_HORIZONTAL_RETRACE = 0x01;
  static const  uint8_t   CGA_VERTICAL_RETRACE   = 0x08;
  static        uint32_t  _counter = 0;

  // Funcion Alleycat y Digger
  {
    _counter++;
    if (_counter > 479)
      port3DAh = CGA_VERTICAL_RETRACE;
    else
      port3DAh = 0;
    if (_counter & 1)
      port3DAh |= CGA_HORIZONTAL_RETRACE;
    if (_counter > 525)
      _counter = 0;
  }
}
