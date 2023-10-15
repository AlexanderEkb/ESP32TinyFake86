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

extern union _bytewordregs_ regs;
uint16_t cursx, cursy, cursorPosition, cols = 80, rows = 25, vgapage, cursorposition, cursorvisible;
uint8_t clocksafe, port6, portout16;
uint32_t videobase= 0xB8000;
uint32_t usefullscreen = 0;

uint8_t latchRGB = 0, latchPal = 0, stateDAC = 0;
uint8_t latchReadRGB = 0, latchReadPal = 0;
uint32_t tempRGB;
uint16_t oldw, oldh; //used when restoring screen mode

extern uint32_t nw, nh;
extern uint32_t pendingColorburstValue;

void setVideoParameters(uint32_t modeDesc, int32_t videoBase)
{
    videobase = videoBase;
		if(modeDesc & VIDEO_MODE_GRAPH)
		{
			portSet(0x3D8, PORT_3D8_GRAPHICS);
			if(modeDesc & VIDEO_MODE_640_PX)
			{
				cols = 80;
				portSet(0x3D8, PORT_3D8_HIRES_GRAPH);
			}
			else
			{
				cols = 40;
				portReset(0x3D8, PORT_3D8_HIRES_GRAPH);
			}
		}
		else
		{
			if (modeDesc & VIDEO_MODE_80_COLS)
			{
				cols = 80;
				portSet(0x3D8, PORT_3D8_80_COL_TEXT);
			}
			else
			{
				cols = 40;
				portSet(0x3D8, PORT_3D8_80_COL_TEXT);
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
			portReset(0x3D8, PORT_3D8_NOCOLOR);
		}
		else
		{
    	pendingColorburstValue = PENDING_COLORBURST_FALSE;
			portSet(0x3D8, PORT_3D8_NOCOLOR);
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
    renderSetBlitter(0);
    break;
  case VIDEO_MODE_80x25_COLOR: // 80x25 color text
		setVideoParameters(VIDEO_MODE_TEXT | VIDEO_MODE_80_COLS | VIDEO_MODE_COLOR, CGA_BASE_MEMORY);
    renderSetBlitter(0);
    break;
  case VIDEO_MODE_320x200_COLOR: // 320x200 color
		setVideoParameters(VIDEO_MODE_GRAPH | VIDEO_MODE_320_PX | VIDEO_MODE_COLOR, CGA_BASE_MEMORY);
		gb_portramTiny[fast_tiny_port_0x3D9] = 48;
    renderSetBlitter(1);
    break;
  case VIDEO_MODE_320x200_BW: // 320x200 BW
		setVideoParameters(VIDEO_MODE_GRAPH | VIDEO_MODE_320_PX | VIDEO_MODE_GRAY, CGA_BASE_MEMORY);
		gb_portramTiny[fast_tiny_port_0x3D9] = 0;
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
      portReset(0x3D8, PORT_3D8_80_COL_TEXT);
      pendingColorburstValue = PENDING_COLORBURST_TRUE;
			break;
	case VIDEO_MODE_0x09: // 320x200 16-color
			videobase = CGA_BASE_MEMORY;
			cols = 40;
			if ((regs.byteregs[regal] & 0x80) == 0) {
					memset(gb_video_cga, 0, 16384);
			}
      portReset(0x3D8, PORT_3D8_80_COL_TEXT);
      pendingColorburstValue = PENDING_COLORBURST_TRUE;
			break;
	case VIDEO_MODE_0x0D: // 320x200 16-color
	case VIDEO_MODE_0x12: // 640x480 16-color
	case VIDEO_MODE_0x13: // 320x200 256-color
			videobase = VGA_BASE_MEMORY;
			cols = 40;
      portReset(0x3D8, PORT_3D8_80_COL_TEXT);
      pendingColorburstValue = PENDING_COLORBURST_TRUE;
			break;
	}
}

void vidinterrupt()
{
	switch (regs.byteregs[regah]) 
	{ //what video interrupt function?
			case 0: //set video mode
			  setVideoMode(regs.byteregs[regal] & 0x7F);

				vidmode = regs.byteregs[regal] & 0x7F;

				gb_ram_bank[0][0x449]= vidmode;
				gb_ram_bank[0][0x44A]= cols;
				gb_ram_bank[0][0x44B]= 0;
				gb_ram_bank[0][0x484]= (rows - 1);				
				cursx = 0;
				cursy = 0;
        cursorPosition = 0;
				switch (vidmode) {
						case 127: //hercules
							nw = oldw = 720;
							nh = oldh = 348;
							break;
						case 0x12:
							nw = oldw = 640;
							nh = oldh = 480;
							break;
						case 0x13:
							oldw = 640;
							oldh = 400;
							nw = 320;
							nh = 200;
							break;
						default:
							nw = oldw = 640;
							nh = oldh = 400;
							break;
					}
				break;
			case 0x1A: //get display combination code (ps, vga/mcga)
				regs.byteregs[regal] = 0x1A;
				regs.byteregs[regbl] = 0x8;
				break;
		}
}

uint16_t vtotal = 0;
void outVGA (unsigned short int portnum, unsigned char value)
{
	static uint8_t oldah, oldal;
	uint8_t flip3c0 = 0;
	if (portnum > (gb_max_portram-1))
	 return;
	
	//updatedscreen = 1;
	switch (portnum) {
			case 0x3B8: //hercules support
				if ( ( (value & 2) == 2) && (vidmode != 127) ) {
						oldah = regs.byteregs[regah];
						oldal = regs.byteregs[regal];
						regs.byteregs[regah] = 0;
						regs.byteregs[regal] = 127;
						vidinterrupt();
						regs.byteregs[regah] = oldah;
						regs.byteregs[regal] = oldal;
					}
				if (value & 0x80) videobase = 0xB8000;
				else videobase = 0xB0000;
				break;
			case 0x3C0:
				if (flip3c0) {
						flip3c0 = 0;
						//JJ puerto portram[0x3C0] = value & 255;
						gb_portramTiny[fast_tiny_port_0x3C0]= value & 255;
						return;
					}
				else {
						flip3c0 = 1;
						//JJVGA VGA_ATTR[portram[0x3C0]] = value & 255;
						return;
					}
			case 0x3C4: //sequence controller index
				//JJ puerto portram[0x3C4] = value & 255;
				gb_portramTiny[fast_tiny_port_0x3C4]= value & 255;
				//if (portout16) VGA_SC[value & 255] = value >> 8;
				break;
			case 0x3C5: //sequence controller data
				//JJVGA VGA_SC[portram[0x3C4]] = value & 255;
				/*if (portram[0x3C4] == 2) {
				printf("VGA_SC[2] = %02X\n", value);
				}*/
				break;
			case 0x3D4: //CRT controller index
				//JJ portram[0x3D4] = value & 255;
				gb_portramTiny[fast_tiny_port_0x3D4] = value & 255;
				//if (portout16) VGA_CRTC[value & 255] = value >> 8;
				break;
			case 0x3C7: //color index register (read operations)
				latchReadPal = value & 255;
				latchReadRGB = 0;
				stateDAC = 0;
				break;
			case 0x3C8: //color index register (write operations)
				latchPal = value & 255;
				latchRGB = 0;
				tempRGB = 0;
				stateDAC = 3;
				break;
			case 0x3C9: //RGB data register
				value = value & 63;
				switch (latchRGB) {
//JJ #ifdef __BIG_ENDIAN__
//JJ 						case 0: //red
//JJ 							tempRGB = value << 26;
//JJ 							break;
//JJ 						case 1: //green
//JJ 							tempRGB |= value << 18;
//JJ 							break;
//JJ 						case 2: //blue
//JJ 							tempRGB |= value << 10;
//JJ 							palettevga[latchPal] = tempRGB;
//JJ 							latchPal = latchPal + 1;
//JJ 							break;
//JJ #else
						case 0: //red
							tempRGB = value << 2;
							break;
						case 1: //green
							tempRGB |= value << 10;
							break;
						case 2: //blue
							tempRGB |= value << 18;
							//JJ palettevga[latchPal] = tempRGB;
							latchPal = latchPal + 1;
							break;
//JJ #endif
					}
				latchRGB = (latchRGB + 1) % 3;
				break;
			case 0x3D5: //cursor position latch
				//JJVGA VGA_CRTC[portram[0x3D4]] = value & 255;
				//JJ puerto if (portram[0x3D4]==0xE)
				if (gb_portramTiny[fast_tiny_port_0x3D4] == 0xE)
				{
					cursorposition = (cursorposition&0xFF) | (value<<8);
				}
				else
				{
				 //JJ puerto if (portram[0x3D4]==0xF)
				 if (gb_portramTiny[fast_tiny_port_0x3D4] == 0xF)
				 {
				  cursorposition = (cursorposition&0xFF00) |value;
				 }
				}
				cursy = cursorposition/cols;
				cursx = cursorposition%cols;
        cursorPosition = (cursy << 8) | cursx;
				//JJ if (portram[0x3D4] == 6) 
				//JJ {
				 //JJVGA vtotal = value | ( ( (uint16_t) VGA_GC[7] & 1) << 8) | ( ( (VGA_GC[7] & 32) ? 1 : 0) << 9);
				 //printf("Vertical total: %u\n", vtotal);
				//JJ }
				break;
			case 0x3CF:
				//JJVGA VGA_GC[portram[0x3CE]] = value;
				break;
			default:
				//JJ puerto portram[portnum] = value;
				portWriteTiny(portnum,value);
		}
}

uint8_t inVGA (uint16_t portnum) {
	if (portnum > (gb_max_portram-1))
	 return 0;        
	switch (portnum) {
			case 0x3C1:
				//JJVGA return ( (uint8_t) VGA_ATTR[portram[0x3C0]]);
				return 0;
			case 0x3C5:
				//JJVGA return ( (uint8_t) VGA_SC[portram[0x3C4]]);
				return 0;
			case 0x3D5:
				//JJVGA return ( (uint8_t) VGA_CRTC[portram[0x3D4]]);
				return 0;
			case 0x3C7: //DAC state
				return (stateDAC);				
			case 0x3C8: //palette index
				return (latchReadPal);
			case 0x3C9: //RGB data register
				switch (latchReadRGB++) {
//JJ #ifdef __BIG_ENDIAN__
//JJ 						case 0: //blue
//JJ 							return ( (palettevga[latchReadPal] >> 26) & 63);
//JJ 						case 1: //green
//JJ 							return ( (palettevga[latchReadPal] >> 18) & 63);
//JJ 						case 2: //red
//JJ 							latchReadRGB = 0;
//JJ 							return ( (palettevga[latchReadPal++] >> 10) & 63);
//JJ #else
						case 0: //blue
							//JJ return ( (palettevga[latchReadPal] >> 2) & 63);
							return 0; //no necesito vga
						case 1: //green
							//JJ return ( (palettevga[latchReadPal] >> 10) & 63);
							return 0; //no necesito vga
						case 2: //red
							latchReadRGB = 0;
							//JJ return ( (palettevga[latchReadPal++] >> 18) & 63);
							return 0; //no necesito vga
//JJ #endif
					}
			case 0x3DA:
				return (port3da);
		}
	//JJ puerto return (portram[portnum]); //this won't be reached, but without it the compiler gives a warning
	return (portReadTiny(portnum)); //this won't be reached, but without it the compiler gives a warning
}

#define shiftVGA(value) {\
	for (cnt=0; cnt<(VGA_GC[3] & 7); cnt++) {\
		value = (value >> 1) | ((value & 1) << 7);\
	}\
}

#define logicVGA(curval, latchval) {\
	switch ((VGA_GC[3]>>3) & 3) {\
		   case 1: curval &= latchval; break;\
		   case 2: curval |= latchval; break;\
		   case 3: curval ^= latchval; break;\
	}\
}

uint8_t lastmode = 0;
void writeVGA (uint32_t addr32, uint8_t value) {
	uint32_t planesize;
	uint8_t curval, tempand, cnt;
	//updatedscreen = 1;
	planesize = 0x10000;
	//JJVGA shiftVGA (value);
	/*JJVGA
	switch (VGA_GC[5] & 3) { //get write mode
			case 0:
				if (VGA_SC[2] & 1) {
						if (VGA_GC[1] & 1)
							if (VGA_GC[0] & 1) curval = 255;
							else curval = 0;
						else curval = value;
						logicVGA (curval, VGA_latch[0]);
						curval = (~VGA_GC[8] & curval) | (VGA_GC[8] & VGA_latch[0]);
						//JJ VRAM[addr32] = curval;
					}
				if (VGA_SC[2] & 2) {
						if (VGA_GC[1] & 2)
							if (VGA_GC[0] & 2) curval = 255;
							else curval = 0;
						else curval = value;
						logicVGA (curval, VGA_latch[1]);
						curval = (~VGA_GC[8] & curval) | (VGA_GC[8] & VGA_latch[1]);
						//JJ VRAM[addr32+planesize] = curval;
					}
				if (VGA_SC[2] & 4) {
						if (VGA_GC[1] & 4)
							if (VGA_GC[0] & 4) curval = 255;
							else curval = 0;
						else curval = value;
						logicVGA (curval, VGA_latch[2]);
						curval = (~VGA_GC[8] & curval) | (VGA_GC[8] & VGA_latch[2]);
						//JJ VRAM[addr32+planesize*2] = curval;
					}
				if (VGA_SC[2] & 8) {
						if (VGA_GC[1] & 8)
							if (VGA_GC[0] & 8) curval = 255;
							else curval = 0;
						else curval = value;
						logicVGA (curval, VGA_latch[3]);
						curval = (~VGA_GC[8] & curval) | (VGA_GC[8] & VGA_latch[3]);
						//JJ VRAM[addr32+planesize*3] = curval;
					}
				break;
			case 1:
				//JJ if (VGA_SC[2] & 1) VRAM[addr32] = VGA_latch[0];
				//JJ if (VGA_SC[2] & 2) VRAM[addr32+planesize] = VGA_latch[1];
				//JJ if (VGA_SC[2] & 4) VRAM[addr32+planesize*2] = VGA_latch[2];
				//JJ if (VGA_SC[2] & 8) VRAM[addr32+planesize*3] = VGA_latch[3];
				break;
			case 2:
				if (VGA_SC[2] & 1) {
						if (VGA_GC[1] & 1)
							if (value & 1) curval = 255;
							else curval = 0;
						else curval = value;
						logicVGA (curval, VGA_latch[0]);
						curval = (~VGA_GC[8] & curval) | (VGA_GC[8] & VGA_latch[0]);
						//JJ VRAM[addr32] = curval;
					}
				if (VGA_SC[2] & 2) {
						if (VGA_GC[1] & 2)
							if (value & 2) curval = 255;
							else curval = 0;
						else curval = value;
						logicVGA (curval, VGA_latch[1]);
						curval = (~VGA_GC[8] & curval) | (VGA_GC[8] & VGA_latch[1]);
						//JJ VRAM[addr32+planesize] = curval;
					}
				if (VGA_SC[2] & 4) {
						if (VGA_GC[1] & 4)
							if (value & 4) curval = 255;
							else curval = 0;
						else curval = value;
						logicVGA (curval, VGA_latch[2]);
						curval = (~VGA_GC[8] & curval) | (VGA_GC[8] & VGA_latch[2]);
						//JJ VRAM[addr32+planesize*2] = curval;
					}
				if (VGA_SC[2] & 8) {
						if (VGA_GC[1] & 8)
							if (value & 8) curval = 255;
							else curval = 0;
						else curval = value;
						logicVGA (curval, VGA_latch[3]);
						curval = (~VGA_GC[8] & curval) | (VGA_GC[8] & VGA_latch[3]);
						//JJ VRAM[addr32+planesize*3] = curval;
					}
				break;
			case 3:
				tempand = value & VGA_GC[8];
				if (VGA_SC[2] & 1) {
						if (VGA_GC[0] & 1) curval = 255;
						else curval = 0;
						logicVGA (curval, VGA_latch[0]);
						curval = (~tempand & curval) | (tempand & VGA_latch[0]);
						//JJ VRAM[addr32] = curval;
					}
				if (VGA_SC[2] & 2) {
						if (VGA_GC[0] & 2) curval = 255;
						else curval = 0;
						logicVGA (curval, VGA_latch[1]);
						curval = (~tempand & curval) | (tempand & VGA_latch[1]);
						//JJ VRAM[addr32+planesize] = curval;
					}
				if (VGA_SC[2] & 4) {
						if (VGA_GC[0] & 4) curval = 255;
						else curval = 0;
						logicVGA (curval, VGA_latch[2]);
						curval = (~tempand & curval) | (tempand & VGA_latch[2]);
						//JJ VRAM[addr32+planesize*2] = curval;
					}
				if (VGA_SC[2] & 8) {
						if (VGA_GC[0] & 8) curval = 255;
						else curval = 0;
						logicVGA (curval, VGA_latch[3]);
						curval = (~tempand & curval) | (tempand & VGA_latch[3]);
						//JJ VRAM[addr32+planesize*3] = curval;
					}
				break;
		}
	*/
}

uint8_t readVGA (uint32_t addr32) {
	uint32_t planesize;
	planesize = 0x10000;
	//JJ VGA_latch[0] = VRAM[addr32];
	//JJ VGA_latch[1] = VRAM[addr32+planesize];
	//JJ VGA_latch[2] = VRAM[addr32+planesize*2];
	//JJ VGA_latch[3] = VRAM[addr32+planesize*3];
	//JJ if (VGA_SC[2] & 1) return (VRAM[addr32]);
	//JJ if (VGA_SC[2] & 2) return (VRAM[addr32+planesize]);
	//JJ if (VGA_SC[2] & 4) return (VRAM[addr32+planesize*2]);
	//JJ if (VGA_SC[2] & 8) return (VRAM[addr32+planesize*3]);
	return (0); //this won't be reached, but without it some compilers give a warning
}

void initVideoPorts() 
{
 //JJ set_port_write_redirector (0x3B0, 0x3DA, &outVGA); 
 //JJ set_port_read_redirector (0x3B0, 0x3DA, &inVGA);
 set_port_write_redirector (0x3B0, 0x3DA, (void *)&outVGA);
 set_port_read_redirector (0x3B0, 0x3DA, (void *)&inVGA);
}
