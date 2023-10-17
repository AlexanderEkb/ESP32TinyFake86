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
#include "gbGlobals.h"
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

static void     write3D5h(uint32_t portnum, uint8_t value);
static uint8_t  inVGA(uint32_t portnum);
static uint8_t  read3DAh(uint32_t portnum);
static void     write3D8h(uint32_t portnum, uint8_t value);

IOPort port_3D4h = IOPort(0x3D4, 0xFF, inVGA,     nullptr);
IOPort port_3D5h = IOPort(0x3D5, 0xFF, inVGA,     write3D5h);
IOPort port_3D8h = IOPort(0x3D8, 0xFF, nullptr,   write3D8h);
IOPort port_3D9h = IOPort(0x3D9, 0xFF, nullptr,   nullptr);
IOPort port_3DAh = IOPort(0x3DA, 0xFF, read3DAh,  nullptr);

static unsigned char port3da = 0;

extern union _bytewordregs_ regs;
uint16_t cursx, cursy, cursorPosition, cols = 80, rows = 25, vgapage, cursorposition, cursorvisible;
uint8_t clocksafe, port6, portout16;
uint32_t videobase= 0xB8000;
uint32_t usefullscreen = 0;

uint16_t oldw, oldh; //used when restoring screen mode

extern uint32_t nw, nh;
extern uint32_t pendingColorburstValue;

void setVideoParameters(uint32_t modeDesc, int32_t videoBase)
{
    videobase = videoBase;
		if(modeDesc & VIDEO_MODE_GRAPH)
		{
			// IOPortSpace::getInstance().setBits(0x3D8, PORT_3D8_GRAPHICS);
			if(modeDesc & VIDEO_MODE_640_PX)
			{
				cols = 80;
				// IOPortSpace::getInstance().setBits(0x3D8, PORT_3D8_HIRES_GRAPH);
			}
			else
			{
				cols = 40;
				// IOPortSpace::getInstance().resetBits(0x3D8, PORT_3D8_HIRES_GRAPH);
			}
		}
		else
		{
			if (modeDesc & VIDEO_MODE_80_COLS)
			{
				cols = 80;
				// IOPortSpace::getInstance().setBits(0x3D8, PORT_3D8_80_COL_TEXT);
			}
			else
			{
				cols = 40;
				// IOPortSpace::getInstance().resetBits(0x3D8, PORT_3D8_80_COL_TEXT);
			}
		}
    
    for (uint32_t i = 0; i < 16384; i += 2) {
			gb_video_cga[i] = 0;
			gb_video_cga[i + 1] = 7;
    }
		const uint32_t hiresGraphMask = VIDEO_MODE_GRAPH | VIDEO_MODE_640_PX;
		const bool hiresGraph = ((modeDesc & hiresGraphMask) == hiresGraphMask);
		const bool colour = (modeDesc & VIDEO_MODE_COLOR);
		const bool enableColour = hiresGraph || colour;
		if(enableColour)
		{
			pendingColorburstValue = PENDING_COLORBURST_TRUE;
			// IOPortSpace::getInstance().resetBits(0x3D8, PORT_3D8_NOCOLOR);
		}
		else
		{
    	pendingColorburstValue = PENDING_COLORBURST_FALSE;
			// IOPortSpace::getInstance().setBits(0x3D8, PORT_3D8_NOCOLOR);
		}
}

void setVideoMode(uint8_t mode)
{
	switch (mode)
	{
	case VIDEO_MODE_40x25_BW: // 40x25 mono text
		setVideoParameters(VIDEO_MODE_TEXT | VIDEO_MODE_40_COLS | VIDEO_MODE_GRAY, CGA_BASE_MEMORY);
    renderSetBlitter(1);
		break;
	case VIDEO_MODE_40x25_COLOR: // 40x25 color text
		setVideoParameters(VIDEO_MODE_TEXT | VIDEO_MODE_40_COLS | VIDEO_MODE_COLOR, CGA_BASE_MEMORY);
    renderSetBlitter(1);
    break;
  case VIDEO_MODE_80x25_BW: // 80x25 mono text
		setVideoParameters(VIDEO_MODE_TEXT | VIDEO_MODE_80_COLS | VIDEO_MODE_GRAY, CGA_BASE_MEMORY);
    renderSetBlitter(1 /*0*/);
    break;
  case VIDEO_MODE_80x25_COLOR: // 80x25 color text
		setVideoParameters(VIDEO_MODE_TEXT | VIDEO_MODE_80_COLS | VIDEO_MODE_COLOR, CGA_BASE_MEMORY);
    renderSetBlitter(1 /*0*/);
    break;
  case VIDEO_MODE_320x200_COLOR: // 320x200 color
		setVideoParameters(VIDEO_MODE_GRAPH | VIDEO_MODE_320_PX | VIDEO_MODE_COLOR, CGA_BASE_MEMORY);
		IOPortSpace::getInstance().write(0x3D9, 48);
    renderSetBlitter(1);
    break;
  case VIDEO_MODE_320x200_BW: // 320x200 BW
		setVideoParameters(VIDEO_MODE_GRAPH | VIDEO_MODE_320_PX | VIDEO_MODE_GRAY, CGA_BASE_MEMORY);
    IOPortSpace::getInstance().write(0x3D9, 0);
    renderSetBlitter(1);
    break;
  case VIDEO_MODE_640x200_COLOR: // 640x200 color
	  setVideoParameters(VIDEO_MODE_GRAPH | VIDEO_MODE_640_PX | VIDEO_MODE_COLOR, CGA_BASE_MEMORY);
    renderSetBlitter(1);
		break;
	case VIDEO_MODE_0x7F:
			videobase = CGA_BASE_MEMORY;
			cols = 90;
			memset(gb_video_cga, 0, 16384);
      // IOPortSpace::getInstance().resetBits(0x3D8, PORT_3D8_80_COL_TEXT);
      pendingColorburstValue = PENDING_COLORBURST_TRUE;
			break;
	case VIDEO_MODE_0x09: // 320x200 16-color
			videobase = CGA_BASE_MEMORY;
			cols = 40;
			if ((regs.byteregs[regal] & 0x80) == 0) {
					memset(gb_video_cga, 0, 16384);
			}
      // IOPortSpace::getInstance().resetBits(0x3D8, PORT_3D8_80_COL_TEXT);
      pendingColorburstValue = PENDING_COLORBURST_TRUE;
			break;
	case VIDEO_MODE_0x0D: // 320x200 16-color
	case VIDEO_MODE_0x12: // 640x480 16-color
	case VIDEO_MODE_0x13: // 320x200 256-color
			videobase = VGA_BASE_MEMORY;
			cols = 40;
      // IOPortSpace::getInstance().resetBits(0x3D8, PORT_3D8_80_COL_TEXT);
      pendingColorburstValue = PENDING_COLORBURST_TRUE;
			break;
	}
}

uint16_t vtotal = 0;
void write3D5h (uint32_t portnum, uint8_t value)
{
	(void)portnum;
  if (IOPortSpace::getInstance().get(0x3D4)->value == 0xE)
  {
    cursorposition = (cursorposition&0xFF) | (value<<8);
  }
  else if (IOPortSpace::getInstance().get(0x3D4)->value == 0xF)
  {
    cursorposition = (cursorposition&0xFF00) |value;
  }
  cursy = cursorposition/cols;
  cursx = cursorposition%cols;
  cursorPosition = (cursy << 8) | cursx;
}

uint8_t inVGA (uint32_t portnum)
{
	if (portnum > (gb_max_portram-1))
	 return 0;        
	switch (portnum) {
			case 0x3D5:
				return 0;
			case 0x3DA:
				return (port3da);
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
    setVideoParameters(VIDEO_MODE_TEXT | VIDEO_MODE_40_COLS | VIDEO_MODE_COLOR, CGA_BASE_MEMORY);
    vidmode = 1;
    break;
  case 0x01: // 80x25 text color
    setVideoParameters(VIDEO_MODE_TEXT | VIDEO_MODE_80_COLS | VIDEO_MODE_COLOR, CGA_BASE_MEMORY);
    vidmode = 3;
    break;
  case 0x02: // 320x200 graphics color
    setVideoParameters(VIDEO_MODE_GRAPH | VIDEO_MODE_320_PX | VIDEO_MODE_COLOR, CGA_BASE_MEMORY);
    vidmode = 4;
    break;
  case 0x04: // 40x25 text monochrome
    setVideoParameters(VIDEO_MODE_TEXT | VIDEO_MODE_40_COLS | VIDEO_MODE_GRAY, CGA_BASE_MEMORY);
    vidmode = 0;
    break;
  case 0x05: // 80x25 text monochrome
    setVideoParameters(VIDEO_MODE_TEXT | VIDEO_MODE_80_COLS | VIDEO_MODE_GRAY, CGA_BASE_MEMORY);
    vidmode = 2;
    break;
  case 0x06: // 320x200 graphics monochrome
    setVideoParameters(VIDEO_MODE_GRAPH | VIDEO_MODE_320_PX | VIDEO_MODE_GRAY, CGA_BASE_MEMORY);
    vidmode = 5;
    break;
  case 0x0E: // 640x200 graphics monochrome
    setVideoParameters(VIDEO_MODE_GRAPH | VIDEO_MODE_640_PX | VIDEO_MODE_COLOR, CGA_BASE_MEMORY);
    vidmode = 6;
    break;
  default:
    break;
  }
}

uint8_t lastmode = 0;
void initVideoPorts() 
{
}

void videoExecCpu(void)
{
  static const uint8_t CGA_HORIZONTAL_RETRACE = 0x01;
  static const uint8_t CGA_VERTICAL_RETRACE = 0x08;
  static uint32_t localscanline = 0;

  // Funcion Alleycat y Digger
  {
        localscanline++;
        if (localscanline > 479)
              port3da = CGA_VERTICAL_RETRACE;
        else
              port3da = 0;
        if (localscanline & 1)
              port3da |= CGA_HORIZONTAL_RETRACE;
        if (localscanline > 525)
              localscanline = 0;
  }
}