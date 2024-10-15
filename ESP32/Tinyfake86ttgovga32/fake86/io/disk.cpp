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

#include "config/gbConfig.h"
#include "cpu/cpu.h"
#include "gbGlobals.h"
#include "io/disk.h"
#include "io/sdcard.h"
#include <esp_heap_caps.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define TAG "disk"

extern SdCard sdcard;
extern union _bytewordregs_ regs;
extern uint8_t * ram;

extern uint8_t read86 (uint32_t addr32);
extern void write86 (uint32_t addr32, uint8_t value);

static void getDriveParameters(uint8_t drive);

static uint8_t lastResult = 0;

FloppyDrive_t driveA;
FloppyDrive_t driveB;
HDD_t driveC;

static const uint32_t DRIVE_COUNT = 3;
Drive_t * drives[DRIVE_COUNT] = {&driveA, &driveB, &driveC};

void setResult(uint8_t _result)
{
  lastResult = _result;
  regs.byteregs[regah] = _result;
  ExternalSetCF((_result == 0)?0:1);
}

void diskInit()
{
  gpio_config_t config = {
    .pin_bit_mask = 1 << DISK_LED,          /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
    .mode = GPIO_MODE_OUTPUT_OD,            /*!< GPIO mode: set input/output mode                     */
    .pull_up_en = GPIO_PULLUP_DISABLE,      /*!< GPIO pull-up                                         */
    .pull_down_en = GPIO_PULLDOWN_DISABLE,  /*!< GPIO pull-down                                       */
    .intr_type = GPIO_INTR_DISABLE          /*!< GPIO interrupt type                                  */
  };
  gpio_config(&config);
  gpio_set_level(DISK_LED, 0);
  const bool sdCardOk = Drive_t::sdCard.Init();
  if(sdCardOk)
  {
    driveC.openImage(DEFAULT_HDD_IMAGE);
  }
  gpio_set_level(DISK_LED, 1);
  lastResult = RESULT_OK;
}

void __attribute__((optimize("-Ofast"))) IRAM_ATTR readdisk(DISK_ADDR &src, MEM_ADDR &dst)
{
  // LOG("Reading D%i C%i H%i S%i L%i %04X:%04X ", src.drive, src.cylinder, src.head, src.sector, src.sectorCount, dst.segment, dst.offset);
  if(src.drive >= DRIVE_COUNT)
  {
    setResult(RESULT_WRONG_PARAM);
    return;
  }
  gpio_set_level(DISK_LED, 0);
  Drive_t *drive = drives[src.drive];
  uint8_t result = drive->read(src, getramloc(dst.linear()));

  setResult(result);
  gpio_set_level(DISK_LED, 1);
  if (result == RESULT_OK)
  {
    regs.byteregs[regal] = src.sectorCount;
    // LOG("OK\n");
  }
  else
  {
    ESP_LOGE(TAG, "Reading error: D%i C%i H%i S%i L%i %04X:%04X ", src.drive, src.cylinder, src.head, src.sector, src.sectorCount, dst.segment, dst.offset);
  }
}

void writedisk (DISK_ADDR & dst, MEM_ADDR & src)
{
  // LOG("Writing D%i C%i H%i S%i L%i %04X:%04X ", dst.drive, dst.cylinder, dst.head, dst.sector, dst.sectorCount, src.segment, src.offset);
  gpio_set_level(DISK_LED, 0);
  Drive_t *drive = drives[dst.drive];
  uint8_t result = drive->write(getramloc(src.linear()), dst);

  if(result == RESULT_OK)
  {
    regs.byteregs[regal] = dst.sectorCount;
    // LOG("OK\n");
  }
  else
  {
    ESP_LOGE(TAG, "Writing error: D%i C%i H%i S%i L%i %04X:%04X ", dst.drive, dst.cylinder, dst.head, dst.sector, dst.sectorCount, src.segment, src.offset);
  }
  setResult(result);
  gpio_set_level(DISK_LED, 1);
}

void diskhandler()
{ 
  const uint8_t  drive = regs.byteregs[regdl];
  const uint16_t cylinder = 
    (static_cast<uint16_t>(regs.byteregs[regcl] & 0xC0) << 2) |
    regs.byteregs[regch];
  const uint16_t sector = regs.byteregs[regcl] & 0x3F;
  const uint16_t head = regs.byteregs[regdh];
  const uint16_t sectorCount = regs.byteregs[regal];
  const uint32_t translatedDrive = (drive >= 0x80) ? (drive - 0x7E) : drive;
  DISK_ADDR diskAddr = DISK_ADDR(translatedDrive, cylinder, head, sector, sectorCount);
  MEM_ADDR buffer = {segregs[reges], regs.wordregs[regbx]};
  const uint8_t serviceNum = regs.byteregs[regah];
 //Solo una disquetera
	switch (serviceNum)
  {
    case 0: //reset disk system
      setResult(RESULT_OK);
      break;
    case 1: // return last status
      setResult(lastResult);
      return;
    case 2: // read sector(s) into memory
      readdisk(diskAddr, buffer);
      break;
    case 3: // write sector(s) from memory
      writedisk(diskAddr, buffer);
      break;
    case 4: // verify sector(s)
    case 5: // format track
      setResult(RESULT_OK);
      break;
    case 8: //get drive parameters
      cf = 0;
      getDriveParameters(translatedDrive);
      setResult(RESULT_OK);
      break;
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
      break;
    default:
      setResult(RESULT_GENERAL_FAILURE);
	}

	if (regs.byteregs[regdl] & 0x80)
  {
	  ram[0x474]= regs.byteregs[regah];
  }
}

static void getDriveParameters(uint8_t drive)
{
  if(drive >= DRIVE_COUNT)
  {
    setResult(RESULT_WRONG_PARAM);
  }
  else
  {
    Geometry_t geometry;
    drives[drive]->getGeometry(&geometry);

    const uint32_t maxCylIndex = geometry.cylinders - 1;
    regs.byteregs[regch] = static_cast<uint8_t>(maxCylIndex & 0xFF);
    regs.byteregs[regcl] = static_cast<uint8_t>(geometry.sectors) & 0x3F;
    regs.byteregs[regcl] = regs.byteregs[regcl] | static_cast<uint8_t>((maxCylIndex >> 2) & 0xC0);
    regs.byteregs[regdh] = static_cast<uint8_t>(geometry.heads - 1) & 0x3F;

    if (drive < 4)
    {
      regs.byteregs[regbl] = 4; // Floppy type. 04h means 3.5" 1.44M
      regs.byteregs[regdl] = 2; // Drive count
    }
    else
    {
      regs.byteregs[regbl] = 0; // Floppy type. Don't know what has to be returned for a HDD.
      regs.byteregs[regdl] = 1; // Drive count
    }
    setResult(RESULT_OK);
  }
}

uint8_t getBootDrive()
{
  if(driveA.isReady())
  {
    ESP_LOGI(TAG, "Booting from drive A:\n");
    return 0x00;
  }
  else if(driveC.isReady())
  {
    ESP_EARLY_LOGI(TAG, "Booting from drive C:\n");
    return 0x02;
  }
  else
    ESP_LOGI(TAG, "Booting to BASIC\n");

  return 0xFF;
}