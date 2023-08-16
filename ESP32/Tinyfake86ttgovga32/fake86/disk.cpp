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
// disk.c: disk emulation routines for Fake86. works at the BIOS interrupt 13h level.

#include "gbConfig.h"
#include <stdint.h>
#include <stdio.h>
#include "disk.h"
#include "cpu.h"
#include "gbGlobals.h"
#include <string.h>
#include <esp_heap_caps.h>
#include <Arduino.h>
#include "sdcard.h"

extern SdCard sdcard;
extern union _bytewordregs_ regs;

extern uint8_t read86 (uint32_t addr32);
extern void write86 (uint32_t addr32, uint8_t value);

unsigned char sectorbuffer[SECTOR_SIZE];
uint8_t * pCache;

DRIVE_DESC drives[DRIVE_COUNT] = {
    {80, 2, 18, 1474560},			// A:
    {80, 2, 18, 1474560},			// B:
};

typedef struct NODE
{
  uint32_t sectorNumber;
  uint8_t * data;
  NODE * left;
  NODE * right;
  NODE(uint32_t sectorNumber)
  {
    this->sectorNumber = sectorNumber;
    data = (uint8_t *) heap_caps_malloc(SECTOR_SIZE, MALLOC_CAP_SPIRAM);
    left = nullptr;
    right = nullptr;
  }
  ~NODE()
  {
    free(data);
  }
} NODE;

void diskInit()
{
	pCache = (uint8_t *)heap_caps_malloc(720 * 1024, MALLOC_CAP_SPIRAM); 
	if(pCache == nullptr)
	{
		Serial.println("Disc cache allocation failed!");
	}
}

// bool readdisk (uint8_t drivenum, uint16_t dstseg, uint16_t dstoff, uint16_t cyl, uint16_t sect, uint16_t head, uint16_t sectcount)
bool readdisk (DISK_ADDR & src, MEM_ADDR & dst)
{
  const bool isValid = src.isValid();
  if(isValid)
  {
    uint32_t filePos = src.lba() * SECTOR_SIZE;
    uint32_t memdest = dst.linear();
    uint32_t sector;
    for (sector=0; sector<src.sectorCount; sector++)
    {
      sdcard.Read(src.drive, sectorbuffer, filePos, SECTOR_SIZE);
      filePos += SECTOR_SIZE;
      if (filePos >= (drives[src.drive].capacity-1))
        break;
      for (uint32_t sectoffset=0; sectoffset<SECTOR_SIZE; sectoffset++)
        write86 (memdest++, sectorbuffer[sectoffset]);
    }
    regs.byteregs[regal] = sector;
    ExternalSetCF(0);
    regs.byteregs[regah] = 0;
  }
  return isValid;
}

bool writedisk (DISK_ADDR & dst, MEM_ADDR & src)
{
  const bool isValid = dst.isValid();
  if(isValid)
  {
    uint32_t filePos = dst.lba() * SECTOR_SIZE;
    uint32_t srcAddr = src.linear();
    uint32_t sector;
    for (sector=0; sector<dst.sectorCount; sector++)
    {
      for (uint32_t i=0; i<SECTOR_SIZE; i++)
        sectorbuffer[i] = read86(srcAddr++);
      sdcard.Write(dst.drive, sectorbuffer, filePos, SECTOR_SIZE);
      filePos += SECTOR_SIZE;
      if (filePos >= (drives[dst.drive].capacity-1))
        break;
    }
    regs.byteregs[regal] = sector;
    ExternalSetCF(0);
    regs.byteregs[regah] = 0;
  }
  return isValid;
}

void diskhandler()
{ 
  MEM_ADDR buffer = {segregs[reges], getreg16 (regbx)};
  const uint8_t  drive = regs.byteregs[regdl];
  const uint16_t dstSeg = segregs[reges];
  const uint16_t dstOff = getreg16 (regbx);
  const uint16_t cylinder = 
    (static_cast<uint16_t>(regs.byteregs[regcl] & 0xC0) << 2) |
    regs.byteregs[regch];
  const uint16_t sector = regs.byteregs[regcl] & 0x3F;
  const uint16_t head = regs.byteregs[regdh];
  const uint16_t sectorCount = regs.byteregs[regal];
  DISK_ADDR diskAddr = DISK_ADDR(drive, cylinder, head, sector, sectorCount);

 //Solo una disquetera
	switch (regs.byteregs[regah])
  {
    case 0: //reset disk system
      regs.byteregs[regah] = 0;
      ExternalSetCF(0);
      break;
    case 1: //return last status
      return;
    case 2: //read sector(s) into memory
      {
        const bool isOk = readdisk(diskAddr, buffer);
        if(isOk)
        {
          ExternalSetCF(0);
          regs.byteregs[regah] = 0;
        }
        else
        {
          ExternalSetCF(1);
          regs.byteregs[regah] = 1;
        }
      }
      break;
    case 3: //write sector(s) from memory
    {
      const bool isOk = writedisk(diskAddr, buffer);
      if(isOk)
      {
        ExternalSetCF(0);
        regs.byteregs[regah] = 0;
      }
      else
      {
        ExternalSetCF(1);
        regs.byteregs[regah] = 1;
      }
    }
      break;
    case 4:
    case 5: //format track
      ExternalSetCF(0);
      regs.byteregs[regah] = 0;
      break;
    case 8: //get drive parameters
      cf = 0;
      ExternalSetCF(0);
      regs.byteregs[regah] = 0;
      regs.byteregs[regch] = static_cast<uint8_t>(drives[drive].cylinders) - 1;
      regs.byteregs[regcl] = static_cast<uint8_t>(drives[drive].sectors) & 63;
      regs.byteregs[regcl] = regs.byteregs[regcl] + static_cast<uint8_t>(drives[drive].sectors / 256) * 63;
      regs.byteregs[regdh] = static_cast<uint8_t>(drives[drive].heads) & 63;

      if (regs.byteregs[regdl]<0x80) {
          regs.byteregs[regbl] = 4; //else regs.byteregs[regbl] = 0;
          regs.byteregs[regdl] = 2;
        }
      else regs.byteregs[regdl] = hdcount;
    break;
    default:
      ExternalSetCF(1);
	}

	if (regs.byteregs[regdl] & 0x80)
  {
	  gb_ram_bank[0][0x474]= regs.byteregs[regah];
  }
}
