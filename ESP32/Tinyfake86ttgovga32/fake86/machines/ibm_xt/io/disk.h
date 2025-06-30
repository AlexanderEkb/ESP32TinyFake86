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
#ifndef _DISK_H
 #define _DISK_H

#include <stdint.h>
#include <stdio.h>
#include "sdcard.h"
#include "drive.h"

extern Drive_t * drives[];

void diskInit(void);
void __attribute__((optimize("-Ofast"))) IRAM_ATTR readdisk(DISK_ADDR &src, MEM_ADDR &dst);
void writedisk (DISK_ADDR & dst, MEM_ADDR & src);
uint8_t getBootDrive();

#endif
