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

#define DRIVE_COUNT (2)
#define SECTOR_SIZE (512)


typedef struct DRIVE_DESC
{
    uint32_t cylinders;
    uint32_t heads;
		uint32_t sectors;
		uint32_t capacity;
} DRIVE_DESC;

extern DRIVE_DESC drives[DRIVE_COUNT];

typedef struct MEM_ADDR
{
  uint16_t segment;
  uint16_t offset;
  MEM_ADDR(uint16_t seg, uint16_t off) :
    segment(seg),
    offset(off) {}
  uint32_t linear() {return ((uint32_t)segment << 4) + offset;}
} MEM_ADDR;

typedef struct DISK_ADDR
{
  uint32_t drive;
  uint32_t cylinder;
  uint32_t sector;
  uint32_t head;
  uint32_t sectorCount;
  DISK_ADDR(uint32_t drv, uint32_t cyl, uint32_t hd, uint32_t sect, uint32_t cnt) :
    drive(drv),
    cylinder(cyl),
    sector(sect),
    head(hd),
    sectorCount(cnt)
    {}
  uint32_t lba() {return (cylinder * drives[drive].heads + head) * drives[drive].sectors + sector - 1;}
  bool isValid() {return (sector != 0) && ((lba() * SECTOR_SIZE < (drives[drive].capacity - 1)));};
} DISK_ADDR;

void diskInit(void);
bool readdisk (DISK_ADDR & src, MEM_ADDR & dst);

#endif
