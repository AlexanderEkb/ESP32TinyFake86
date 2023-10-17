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
#include "gbConfig.h"
#include <stdint.h>
#include <stdio.h>
#include "video.h"
#include "cpu.h"
#include "ports.h"
#include <string.h>
#include "render.h"

#define PORT_3D8_BLINKING			(0x20)
#define PORT_3D8_HIRES_GRAPH	(0x10)
#define PORT_3D8_OE						(0x08)
#define PORT_3D8_NOCOLOR			(0x04)
#define PORT_3D8_GRAPHICS			(0x02)
#define PORT_3D8_80_COL_TEXT	(0x01)

#define VIDEO_MODE_TEXT				(0x00)
#define VIDEO_MODE_GRAPH			(0x01)
#define VIDEO_MODE_40_COLS		(0x00)
#define VIDEO_MODE_80_COLS		(0x02)
#define VIDEO_MODE_320_PX			(0x00)
#define VIDEO_MODE_640_PX			(0x02)
#define VIDEO_MODE_COLOR			(0x04)
#define VIDEO_MODE_GRAY				(0x00)

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

static void     write3D4h(uint32_t portnum, uint8_t value);
static void     write3D5h(uint32_t portnum, uint8_t value);
static uint8_t  read3D5h(uint32_t portnum);
static void     write3D8h(uint32_t portnum, uint8_t value);
static void     write3D9h(uint32_t portnum, uint8_t value);
static uint8_t  read3DAh(uint32_t portnum);

IOPort port_3D4h = IOPort(0x3D4, 0xFF, nullptr,   write3D4h);
IOPort port_3D5h = IOPort(0x3D5, 0xFF, read3D5h,  write3D5h);
IOPort port_3D8h = IOPort(0x3D8, 0xFF, nullptr,   write3D8h);
IOPort port_3D9h = IOPort(0x3D9, 0xFF, nullptr,   write3D9h);
IOPort port_3DAh = IOPort(0x3DA, 0xFF, read3DAh,  nullptr);

static uint8_t    port3da = 0; // Some sort of local cache
static uint8_t    mc6845RegSelector; // 3D4h port writes modifies this var

void setVideoParameters(uint32_t modeDesc)
{
    if(((modeDesc & VIDEO_MODE_GRAPH) && (modeDesc & VIDEO_MODE_640_PX)) || (modeDesc & VIDEO_MODE_80_COLS))
      renderSetColumnCount(80);
    else
      renderSetColumnCount(40);

		const bool enableColour = (modeDesc & VIDEO_MODE_COLOR);
		renderSetColorEnabled(enableColour);
}

static void write3D4h (uint32_t portnum, uint8_t value)
{
  (void)portnum;
  mc6845RegSelector = value;
}

static void write3D5h (uint32_t portnum, uint8_t value)
{
	(void)portnum;
  switch(mc6845RegSelector)
  {
    case MC6845_REG_CURSOS_START:
      cursor.updateStart(value);
      break;
    case MC6845_REG_CURSOR_END:
      cursor.updateEnd(value);
      break;
    case MC6845_REG_CURSOR_ADDR_MSB:
      cursor.updateMSB(value);
      break;
    case MC6845_REG_CURSOR_ADDR_LSB:
      cursor.updateLSB(value);
      break;
  }
}

uint8_t read3D5h (uint32_t portnum)
{
	if (portnum > (gb_max_portram-1))
	 return 0;        
	switch (portnum) {
			case 0x3D5:
				return 0;
		}
	//JJ puerto return (portram[portnum]); //this won't be reached, but without it the compiler gives a warning
	return (0xFF); //this won't be reached, but without it the compiler gives a warning
}

static uint8_t read3DAh(uint32_t portnum)
{
  (void)portnum;
  return (port3da);
}

static void write3D8h(uint32_t portnum, uint8_t value)
{
  (void)portnum;
  uint8_t _mode = value & 0x17;
  switch(_mode)
  {
  case 0x00: // 40x25 text color
    setVideoParameters(VIDEO_MODE_TEXT | VIDEO_MODE_40_COLS | VIDEO_MODE_COLOR);
    renderUpdateDumper(DUMPER_40x25_8x8);
    break;
  case 0x01: // 80x25 text color
    setVideoParameters(VIDEO_MODE_TEXT | VIDEO_MODE_80_COLS | VIDEO_MODE_COLOR);
    renderUpdateDumper(DUMPER_80x25_4x8);
    break;
  case 0x02: // 320x200 graphics color
    setVideoParameters(VIDEO_MODE_GRAPH | VIDEO_MODE_320_PX | VIDEO_MODE_COLOR);
    renderUpdateDumper(DUMPER_320x200);
    break;
  case 0x04: // 40x25 text monochrome
    setVideoParameters(VIDEO_MODE_TEXT | VIDEO_MODE_40_COLS | VIDEO_MODE_GRAY);
    renderUpdateDumper(DUMPER_40x25_8x8);
    break;
  case 0x05: // 80x25 text monochrome
    setVideoParameters(VIDEO_MODE_TEXT | VIDEO_MODE_80_COLS | VIDEO_MODE_GRAY);
    renderUpdateDumper(DUMPER_80x25_4x8);
    break;
  case 0x06: // 320x200 graphics monochrome
    setVideoParameters(VIDEO_MODE_GRAPH | VIDEO_MODE_320_PX | VIDEO_MODE_GRAY);
    renderUpdateDumper(DUMPER_320x200);
    break;
  case 0x0E: // 640x200 graphics monochrome
    setVideoParameters(VIDEO_MODE_GRAPH | VIDEO_MODE_640_PX | VIDEO_MODE_COLOR);
    renderUpdateDumper(DUMPER_640x200);
    break;
  default:
    break;
  }
}

static void write3D9h(uint32_t portnum, uint8_t value)
{
  (void)portnum;
  static const uint8_t COLOR_MASK = 0x0F;
  static const uint8_t PALETTE_POS = 4;
  static const uint8_t PALETTE_MASK = 0x03;
  uint32_t color    = value & COLOR_MASK;
  uint32_t palette  = (value >> PALETTE_POS) & PALETTE_MASK;
  renderUpdateColorSettings(palette, color);
}

void initVideoPorts() 
{
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
      port3da = CGA_VERTICAL_RETRACE;
    else
      port3da = 0;
    if (_counter & 1)
      port3da |= CGA_HORIZONTAL_RETRACE;
    if (_counter > 525)
      _counter = 0;
  }
}