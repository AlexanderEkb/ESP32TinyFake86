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

#include "cpu/ports.h"
#include "cpu/cpu.h"
#include "config/gbConfig.h"
#include "gbGlobals.h"
#include "io/speaker.h"
#include <Arduino.h>
#include <stdio.h>

IOPortSpace IOPortSpace::instance;
IOPort *    IOPortSpace::root = nullptr;

IOPort::IOPort(uint32_t address, uint8_t defaultValue, portReader_t reader, portWriter_t writer)
{
  this->address = address;
  this->value   = defaultValue;
  this->reader  = reader;
  this->writer  = writer;
  this->left    = nullptr;
  this->right   = nullptr;

  IOPortSpace::getInstance().insert(this);
};

//********************************************************************
// uint32_t this_getPortAddr(unsigned short int numPort)
// {
//   switch (numPort)
//   {
//   // case 0x001:
//   //   return 0;
//   // case 0x008:
//   //   return 1;
//   // case 0x00A:
//   //   return 2;
//   // case 0x00B:
//   //   return 3;
//   // case 0x00D:
//   //   return 4;
//   // case 0x020:
//   //   return 5;
//   // case 0x021:
//   //   return 6;
//   // case 0x040:
//   //   return 7;
//   // case 0x041:
//   //   return 8;
//   // case 0x042:
//   //   return 9;
//   // case 0x043:
//   //   return 10;
//   // case 0x060:
//   //   return fast_tiny_port_0x60; // keyboard
//   // case 0x061:
//   //   return fast_tiny_port_0x61; // speaker
//   // case 0x063:
//   //   return 13;
//   // case 0x064:
//   //   return fast_tiny_port_0x64; // keyboard
//   case 0x081:
//     return 15;
//   case 0x082:
//     return 16;
//   // case 0x083:
//   //   return 17;
//   case 0x0A0:
//     return 18;
//   case 0x0C0:
//     return 19;
//   case 0x200:
//     return 20;
//   case 0x201:
//     return 21;
//   case 0x213:
//     return 22;
//   case 0x278:
//     return 23;
//   case 0x2C8:
//     return 24;
//   case 0x2CB:
//     return 25;
//   case 0x2FB:
//     return 26;
//   case 0x378:
//     return 27;
//   case 0x3B4:
//     return 28;
//   case 0x3B5:
//     return 29;
//   // case 0x3B8:
//   //   return 30;
//   case 0x3B9:
//     return fast_tiny_port_0x3B9;
//   case 0x3BA:
//     return 32; // CGA
//   case 0x3BC:
//     return 33;
//   case 0x3BD:
//     return 34;
//   case 0x3BE:
//     return 35;
//   case 0x3BF:
//     return 36;
//   // case 0x3C0:
//   //   return fast_tiny_port_0x3C0; // CGA
//   case 0x3C2:
//     return 38;
//   case 0x3C3:
//     return 39;
//   // case 0x3C4:
//   //   return fast_tiny_port_0x3C4; // CGA
//   // case 0x3C5:
//   //   return 41;
//   case 0x3CB:
//     return 42;
//   case 0x3CC:
//     return 43;
//   // case 0x3D4:
//   //   return fast_tiny_port_0x3D4; // CGA
//   // case 0x3D5:
//   //   return 45;
//   // case 0x3D8:
//   //   return fast_tiny_port_0x3D8; // video CGA
//   case 0x3D9:
//     return fast_tiny_port_0x3D9; // CGA
//   case 0x3DA:
//     return 48;
//   case 0x3F2:
//     return 49;
//   case 0x3FB:
//     return 50;
//   // case 0x3CE: gb_portramTiny[0]; break; //solo VGA
//   default:
//     return 0xFFFF;
//     break;
//   }
// }
//********************************************************************
