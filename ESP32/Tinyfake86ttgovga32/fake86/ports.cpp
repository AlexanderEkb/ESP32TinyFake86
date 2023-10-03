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
// ports.c: functions to handle port I/O from the CPU module, as well
//   as functions for emulated hardware components to register their
//   read/write callback functions across the port address range.

#include "ports.h"
#include "cpu.h"
#include "gbConfig.h"
#include "gbGlobals.h"
#include "speaker.h"
#include <Arduino.h>
#include <stdio.h>
#include "ports.h"

// Lista puertos
// 0x60 teclado
// 0x64
// 0x61 speaker
// 0x3D8 video
// 0x3D4
// 0x3D8
// 0x3D9
// 0x3C0
// 0x3C4
// 0x3CE

#ifndef use_lib_not_use_callback_port
void (*do_callback_write)(uint16_t portnum, uint8_t value) = NULL;
uint8_t (*do_callback_read)(uint16_t portnum) = NULL;
#endif

static unsigned char this_getCallbackIndex(unsigned short int numPort);

void portWrite(uint32_t portnum, uint8_t value)
{
  if (portnum >= (gb_max_portram - 1))
    return;
#ifdef use_lib_limit_portram
  if (portnum < gb_max_portram)
#endif
    portWriteTiny(portnum, value);

  switch (portnum)
  {
  case 0x61:
    onPort0x61Write(value);
    return;
  }

#ifndef use_lib_not_use_callback_port
  uint32_t auxIdport = this_getCallbackIndex(portnum);
  if (auxIdport != 0xFF)
  {
    do_callback_write = (void (*)(uint16_t portnum, uint8_t value))gb_portTiny_write_callback[auxIdport];
    if (do_callback_write != (void *)0)
    {
      (*do_callback_write)(portnum, value);
    }
  }
#endif
}

uint8_t portRead(uint16_t portnum)
{
  if (portnum >= (gb_max_portram - 1))
    return 0;
  switch (portnum)
  {
  case 0x62:
    return (0x00);
  case 0x60:
  case 0x61:
  case 0x63:
  case 0x64:
    return portReadTiny(portnum);
  }
#ifndef use_lib_not_use_callback_port
  uint32_t auxIdport = this_getCallbackIndex(portnum);
  if (auxIdport != 0xFF)
  {
    do_callback_read = (uint8_t(*)(uint16_t portnum))gb_portTiny_read_callback[auxIdport];
    if (do_callback_read != (void *)0)
    {
      return ((*do_callback_read)(portnum));
    }
  }
#endif
  return (0xFF);
}

void portout16(uint32_t portnum, uint16_t value)
{
  if (portnum >= (gb_max_portram - 1))
  {
    // printf("portout16 %d\n",portnum);
    // fflush(stdout);
    return;
  }
  // No se usa
  // JJ  #ifndef use_lib_not_use_callback_port
  // JJ	do_callback_write16 = (void (*) (uint16_t portnum, uint16_t value) ) port_write_callback16[portnum];
  // JJ	if (do_callback_write16 != (void *) 0) {
  // JJ			(*do_callback_write16) (portnum, value);
  // JJ			return;
  // JJ		}
  // JJ #endif

  portWrite(portnum, (uint8_t)value);
  portWrite(portnum + 1, (uint8_t)(value >> 8));
}

uint16_t portin16(uint16_t portnum)
{
  uint16_t ret;
  if (portnum >= (gb_max_portram - 1))
  {
    // printf("portin16 %d\n",portnum);
    // fflush(stdout);
    return 0;
  }

  // No se usa
  // JJ  #ifndef use_lib_not_use_callback_port
  // JJ	do_callback_read16 = (uint16_t (*) (uint16_t portnum) ) port_read_callback16[portnum];
  // JJ	if (do_callback_read16 != (void *) 0) return ( (*do_callback_read16) (portnum) );
  // JJ  #endif

  ret = (uint16_t)portRead(portnum);
  ret |= (uint16_t)portRead(portnum + 1) << 8;
  return (ret);
}

// JJ extern void set_port_write_redirector (uint16_t startport, uint16_t endport, void *callback)
extern void set_port_write_redirector(unsigned short int startport, unsigned short int endport, void *callback)
{
#ifndef use_lib_not_use_callback_port
  // uint16_t i;
  int i;
  unsigned char auxIdport;
  for (i = startport; i <= endport; i++)
  {
    if ((i >= 0) && (i < gb_max_portram))
    {
      // JJ port_write_callback[i] = callback;
      auxIdport = this_getCallbackIndex(i);
      if (auxIdport != 0xFF)
      {
        gb_portTiny_write_callback[auxIdport] = callback;
      }
    }
  }
#endif
}

// JJextern void set_port_read_redirector (uint16_t startport, uint16_t endport, void *callback)
extern void set_port_read_redirector(unsigned short int startport, unsigned short int endport, void *callback)
{
#ifndef use_lib_not_use_callback_port
  // uint16_t i;
  int i;
  unsigned char auxIdport;
  for (i = startport; i <= endport; i++)
  {
    if ((i >= 0) && (i < gb_max_portram))
    {
      // JJ port_read_callback[i] = callback;
      auxIdport = this_getCallbackIndex(i);
      if (auxIdport != 0xFF)
      {
        gb_portTiny_read_callback[auxIdport] = callback;
      }
    }
  }
#endif
}

//********************************************************************
uint32_t this_getPortAddr(unsigned short int numPort)
{
  switch (numPort)
  {
  case 0x001:
    return 0;
  case 0x008:
    return 1;
  case 0x00A:
    return 2;
  case 0x00B:
    return 3;
  case 0x00D:
    return 4;
  case 0x020:
    return 5;
  case 0x021:
    return 6;
  case 0x040:
    return 7;
  case 0x041:
    return 8;
  case 0x042:
    return 9;
  case 0x043:
    return 10;
  case 0x060:
    return fast_tiny_port_0x60; // keyboard
  case 0x061:
    return fast_tiny_port_0x61; // speaker
  case 0x063:
    return 13;
  case 0x064:
    return fast_tiny_port_0x64; // keyboard
  case 0x081:
    return 15;
  case 0x082:
    return 16;
  case 0x083:
    return 17;
  case 0x0A0:
    return 18;
  case 0x0C0:
    return 19;
  case 0x200:
    return 20;
  case 0x201:
    return 21;
  case 0x213:
    return 22;
  case 0x278:
    return 23;
  case 0x2C8:
    return 24;
  case 0x2CB:
    return 25;
  case 0x2FB:
    return 26;
  case 0x378:
    return 27;
  case 0x3B4:
    return 28;
  case 0x3B5:
    return 29;
  case 0x3B8:
    return 30;
  case 0x3B9:
    return fast_tiny_port_0x3B9;
  case 0x3BA:
    return 32; // CGA
  case 0x3BC:
    return 33;
  case 0x3BD:
    return 34;
  case 0x3BE:
    return 35;
  case 0x3BF:
    return 36;
  case 0x3C0:
    return fast_tiny_port_0x3C0; // CGA
  case 0x3C2:
    return 38;
  case 0x3C3:
    return 39;
  case 0x3C4:
    return fast_tiny_port_0x3C4; // CGA
  case 0x3C5:
    return 41;
  case 0x3CB:
    return 42;
  case 0x3CC:
    return 43;
  case 0x3D4:
    return fast_tiny_port_0x3D4; // CGA
  case 0x3D5:
    return 45;
  case 0x3D8:
    return fast_tiny_port_0x3D8; // video CGA
  case 0x3D9:
    return fast_tiny_port_0x3D9; // CGA
  case 0x3DA:
    return 48;
  case 0x3F2:
    return 49;
  case 0x3FB:
    return 50;
  // case 0x3CE: gb_portramTiny[0]; break; //solo VGA
  default:
    return 0xFFFF;
    break;
  }
}

void portWriteTiny(uint32_t numPort, unsigned char aValue)
{
  const uint32_t index = this_getPortAddr(numPort);
  if (index != 0xFFFF)
  {
    gb_portramTiny[index] = aValue;
    if (numPort == 0x042)
    {
      gb_portramTiny[9] = aValue;
      if ((gb_cont_frec_speaker & 1) == 0)
        gb_frec_speaker_low = aValue;
      else
        gb_frec_speaker_high = aValue;
      gb_cont_frec_speaker++;
      // Frecuencia speaker
    }
  }
  // LOG("out 0x%x, 0x%x\n", numPort, gb_portramTiny[aValue]);
}

void __attribute((__always_inline__)) portSet(uint32_t port, uint8_t val)
{
  const uint32_t index = this_getPortAddr(port);
  if (index != 0xFFFF)
  {
    const uint8_t newVal = gb_portramTiny[index] | val;
    gb_portramTiny[index] = newVal;
  }
  // LOG("set 0x%x, 0x%x\n", port, val);
}

void __attribute((__always_inline__)) portReset(uint32_t port, uint8_t val)
{
  const uint32_t index = this_getPortAddr(port);
  if (index != 0xFFFF)
  {
    const uint8_t newVal = gb_portramTiny[index] & ~val;
    gb_portramTiny[index] = newVal;
  }
  // LOG("reset 0x%x, 0x%x\n", port, val);
}

//******************************************************
unsigned char portReadTiny(unsigned short int numPort)
{
  const uint32_t index = this_getPortAddr(numPort);
  return gb_portramTiny[index];
}

//********************************************************************
static unsigned char this_getCallbackIndex(unsigned short int numPort)
{
  if (numPort <= 0x0F)
    return 0; // out8237
  if (numPort >= 0x20 && numPort <= 0x21)
    return 1; // out8259
  if (numPort >= 0x40 && numPort <= 0x43)
    return 2; // out8253
  if (numPort >= 0x80 && numPort <= 0x8F)
    return 3; // out8237
  if (numPort >= 0x3B0 && numPort <= 0x3DA)
    return 4; // outVGA

  return 0xFF;
}
