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

