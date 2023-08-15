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
//#include "dataFlash/dsk/dskcompaqdos211cat.h"
#include <string.h>
//#include "dataFlash/dsk/dskmsdos300gameover.h"
//#include "dataFlash/dsk/dskcompaqdos211lastmission.h"
//#include "dataFlash/dsk/dskcompaqdos211madmix.h"
//#include "dataFlash/dsk/dskcompaqdos211solnegro.h"
//#include "dataFlash/dsk/dskcompaqdos211pakupaku.h"
#include <esp_heap_caps.h>
#include <Arduino.h>
#include "sdcard.h"

//JJ extern uint8_t RAM[0x100000]
//uint8_t cf, hdcount;
//extern uint16_t segregs[4];
extern SdCard sdcard;
extern union _bytewordregs_ regs;

extern uint8_t read86 (uint32_t addr32);
extern void write86 (uint32_t addr32, uint8_t value);

unsigned char sectorbuffer[512];
uint8_t * pCache;

typedef struct DRIVE_DESC
{
    uint32_t cylinders;
    uint32_t heads;
		uint32_t sectors;
		uint32_t sectorSize;
		uint32_t capacity;
} DRIVE_DESC;

static DRIVE_DESC drives[2] = {
    {80, 2, 18, 512, 1474560},			// A:
    {80, 2, 18, 512, 1474560},			// B:
};

void diskInit()
{
	pCache = (uint8_t *)heap_caps_malloc(720 * 1024, MALLOC_CAP_SPIRAM); 
	if(pCache == nullptr)
	{
		Serial.println("Disc cache allocation failed!");
	}
}

void readdisk (uint8_t drivenum, uint16_t dstseg, uint16_t dstoff, uint16_t cyl, uint16_t sect, uint16_t head, uint16_t sectcount) {
	uint32_t memdest, lba, fileoffset, cursect, sectoffset;
	if (!sect)
		return;
	lba = ( (uint32_t) cyl * drives[drivenum].heads + (uint32_t) head) * drives[drivenum].sectors + (uint32_t) sect - 1;
	fileoffset = lba * drives[drivenum].sectorSize;
	if (fileoffset>=(drives[drivenum].capacity-1)) 
		return;
	memdest = ( (uint32_t) dstseg << 4) + (uint32_t) dstoff;
	for (cursect=0; cursect<sectcount; cursect++)
	{
		sdcard.Read(drivenum, sectorbuffer, fileoffset, drives[drivenum].sectorSize);
        fileoffset += drives[drivenum].sectorSize;
        if (fileoffset >= (drives[drivenum].capacity-1))
			break;
		for (sectoffset=0; sectoffset<drives[drivenum].sectorSize; sectoffset++)
			write86 (memdest++, sectorbuffer[sectoffset]);
	}
	regs.byteregs[regal] = cursect;
	ExternalSetCF(0);
	regs.byteregs[regah] = 0;
}

void writedisk (uint8_t drivenum, uint16_t dstseg, uint16_t dstoff, uint16_t cyl, uint16_t sect, uint16_t head, uint16_t sectcount) {
 return;
}

/*uint8_t insertdisk (uint8_t drivenum, char *filename) {
	if (disk[drivenum].inserted) fclose (disk[drivenum].diskfile);
	else disk[drivenum].inserted = 1;
	disk[drivenum].diskfile = fopen (filename, "r+b");
	if (disk[drivenum].diskfile==NULL) {
			disk[drivenum].inserted = 0;
			return (1);
		}
	fseek (disk[drivenum].diskfile, 0L, SEEK_END);
	disk[drivenum].filesize = ftell (disk[drivenum].diskfile);
	fseek (disk[drivenum].diskfile, 0L, SEEK_SET);
	if (drivenum >= 0x80) { //it's a hard disk image
			disk[drivenum].sects = 63;
			disk[drivenum].heads = 16;
			disk[drivenum].cyls = disk[drivenum].filesize / (disk[drivenum].sects * disk[drivenum].heads * 512);
			hdcount++;
		}
	else {   //it's a floppy image
			disk[drivenum].cyls = 80;
			disk[drivenum].sects = 18;
			disk[drivenum].heads = 2;
			if (disk[drivenum].filesize <= 1228800) disk[drivenum].sects = 15;
			if (disk[drivenum].filesize <= 737280) disk[drivenum].sects = 9;
			if (disk[drivenum].filesize <= 368640) {
					disk[drivenum].cyls = 40;
					disk[drivenum].sects = 9;
				}
			if (disk[drivenum].filesize <= 163840) {
					disk[drivenum].cyls = 40;
					disk[drivenum].sects = 8;
					disk[drivenum].heads = 1;
				}
		}
	return (0);
}

void ejectdisk (uint8_t drivenum) {
	disk[drivenum].inserted = 0;
	if (disk[drivenum].diskfile != NULL) fclose (disk[drivenum].diskfile);
}


void readdisk (uint8_t drivenum, uint16_t dstseg, uint16_t dstoff, uint16_t cyl, uint16_t sect, uint16_t head, uint16_t sectcount) {
	uint32_t memdest, lba, fileoffset, cursect, sectoffset;
	if (!sect || !disk[drivenum].inserted) return;
	lba = ( (uint32_t) cyl * (uint32_t) disk[drivenum].heads + (uint32_t) head) * (uint32_t) disk[drivenum].sects + (uint32_t) sect - 1;
	fileoffset = lba * 512;
	if (fileoffset>disk[drivenum].filesize) return;
	fseek (disk[drivenum].diskfile, fileoffset, SEEK_SET);
	memdest = ( (uint32_t) dstseg << 4) + (uint32_t) dstoff;
	//for the readdisk function, we need to use write86 instead of directly fread'ing into
	//the RAM array, so that read-only flags are honored. otherwise, a program could load
	//data from a disk over BIOS or other ROM code that it shouldn't be able to.
	for (cursect=0; cursect<sectcount; cursect++) {
			if (fread (sectorbuffer, 1, 512, disk[drivenum].diskfile) < 512) break;
			for (sectoffset=0; sectoffset<512; sectoffset++) {
					write86 (memdest++, sectorbuffer[sectoffset]);
				}
		}
	regs.byteregs[regal] = cursect;
	cf = 0;
	regs.byteregs[regah] = 0;
}

void writedisk (uint8_t drivenum, uint16_t dstseg, uint16_t dstoff, uint16_t cyl, uint16_t sect, uint16_t head, uint16_t sectcount) {
	uint32_t memdest, lba, fileoffset, cursect, sectoffset;
	if (!sect || !disk[drivenum].inserted) return;
	lba = ( (uint32_t) cyl * (uint32_t) disk[drivenum].heads + (uint32_t) head) * (uint32_t) disk[drivenum].sects + (uint32_t) sect - 1;
	fileoffset = lba * 512;
	if (fileoffset>disk[drivenum].filesize) return;
	fseek (disk[drivenum].diskfile, fileoffset, SEEK_SET);
	memdest = ( (uint32_t) dstseg << 4) + (uint32_t) dstoff;
	for (cursect=0; cursect<sectcount; cursect++) {
			for (sectoffset=0; sectoffset<512; sectoffset++) {
					sectorbuffer[sectoffset] = read86 (memdest++);
				}
			fwrite (sectorbuffer, 1, 512, disk[drivenum].diskfile);
		}
	regs.byteregs[regal] = (uint8_t) sectcount;
	cf = 0;
	regs.byteregs[regah] = 0;
}
*/


/*
//Original soporta disco duro 256 entradas y varias disqueteras
void diskhandler()
{    
	static uint8_t lastdiskah[256], lastdiskcf[256];
	switch (regs.byteregs[regah]) {
			case 0: //reset disk system
				regs.byteregs[regah] = 0;
				cf = 0; //useless function in an emulator. say success and return.
				break;
			case 1: //return last status
				regs.byteregs[regah] = lastdiskah[regs.byteregs[regdl]];
				cf = lastdiskcf[regs.byteregs[regdl]];
				return;
			case 2: //read sector(s) into memory
				if (disk[regs.byteregs[regdl]].inserted) {
						readdisk (regs.byteregs[regdl], segregs[reges], getreg16 (regbx), regs.byteregs[regch] + (regs.byteregs[regcl]/64) *256, regs.byteregs[regcl] & 63, regs.byteregs[regdh], regs.byteregs[regal]);
						cf = 0;
						regs.byteregs[regah] = 0;
					}
				else {
						cf = 1;
						regs.byteregs[regah] = 1;
					}
				break;
			case 3: //write sector(s) from memory
				if (disk[regs.byteregs[regdl]].inserted) {
						writedisk (regs.byteregs[regdl], segregs[reges], getreg16 (regbx), regs.byteregs[regch] + (regs.byteregs[regcl]/64) *256, regs.byteregs[regcl] & 63, regs.byteregs[regdh], regs.byteregs[regal]);
						cf = 0;
						regs.byteregs[regah] = 0;
					}
				else {
						cf = 1;
						regs.byteregs[regah] = 1;
					}
				break;
			case 4:
			case 5: //format track
				cf = 0;
				regs.byteregs[regah] = 0;
				break;
			case 8: //get drive parameters
				if (disk[regs.byteregs[regdl]].inserted) {
						cf = 0;
						regs.byteregs[regah] = 0;
						regs.byteregs[regch] = disk[regs.byteregs[regdl]].cyls - 1;
						regs.byteregs[regcl] = disk[regs.byteregs[regdl]].sects & 63;
						regs.byteregs[regcl] = regs.byteregs[regcl] + (disk[regs.byteregs[regdl]].cyls/256) *64;
						regs.byteregs[regdh] = disk[regs.byteregs[regdl]].heads - 1;
						//segregs[reges] = 0; regs.wordregs[regdi] = 0x7C0B; //floppy parameter table
						if (regs.byteregs[regdl]<0x80) {
								regs.byteregs[regbl] = 4; //else regs.byteregs[regbl] = 0;
								regs.byteregs[regdl] = 2;
							}
						else regs.byteregs[regdl] = hdcount;
					}
				else {
						cf = 1;
						regs.byteregs[regah] = 0xAA;
					}
				break;
			default:
				cf = 1;
		}
	lastdiskah[regs.byteregs[regdl]] = regs.byteregs[regah];
	lastdiskcf[regs.byteregs[regdl]] = cf;
	if (regs.byteregs[regdl] & 0x80) RAM[0x474] = regs.byteregs[regah];
}
*/


void diskhandler()
{    
 //Solo una disquetera
	switch (regs.byteregs[regah]) {
			case 0: //reset disk system
				regs.byteregs[regah] = 0;
				//cf = 0; //useless function in an emulator. say success and return.
				ExternalSetCF(0);
				break;
			case 1: //return last status
				return;
			case 2: //read sector(s) into memory
				//JJ if (disk[regs.byteregs[regdl]].inserted) {
						readdisk (regs.byteregs[regdl], segregs[reges], getreg16 (regbx), regs.byteregs[regch] + (regs.byteregs[regcl]/64) *256, regs.byteregs[regcl] & 63, regs.byteregs[regdh], regs.byteregs[regal]);
						//cf = 0;
						ExternalSetCF(0);
						regs.byteregs[regah] = 0;
				//JJ 	}
				//JJ else {
				//JJ 		//cf = 1;
				//JJ 		ExternalSetCF(1);
				//JJ 		regs.byteregs[regah] = 1;
				//JJ 	}
				break;
			case 3: //write sector(s) from memory
				//JJ if (disk[regs.byteregs[regdl]].inserted) {
						writedisk (regs.byteregs[regdl], segregs[reges], getreg16 (regbx), regs.byteregs[regch] + (regs.byteregs[regcl]/64) *256, regs.byteregs[regcl] & 63, regs.byteregs[regdh], regs.byteregs[regal]);
						//cf = 0;
						ExternalSetCF(0);
						regs.byteregs[regah] = 0;
				//JJ 	}
				//JJ else {
				//JJ 		//cf = 1;
				//JJ 		ExternalSetCF(1);
				//JJ 		regs.byteregs[regah] = 1;
				//JJ 	}
				break;
			case 4:
			case 5: //format track
				//cf = 0;
				ExternalSetCF(0);
				regs.byteregs[regah] = 0;
				break;
			case 8: //get drive parameters
				//JJ if (disk[regs.byteregs[regdl]].inserted) {
						cf = 0;
						ExternalSetCF(0);
						regs.byteregs[regah] = 0;
						regs.byteregs[regch] = static_cast<uint8_t>(drives[regs.byteregs[regdl]].cylinders) - 1;
						regs.byteregs[regcl] = static_cast<uint8_t>(drives[regs.byteregs[regdl]].sectors) & 63;
						regs.byteregs[regcl] = regs.byteregs[regcl] + static_cast<uint8_t>(drives[regs.byteregs[regdl]].sectors / 256) * 63;
						regs.byteregs[regdh] = static_cast<uint8_t>(drives[regs.byteregs[regdl]].heads) & 63;
						//segregs[reges] = 0; regs.wordregs[regdi] = 0x7C0B; //floppy parameter table
						if (regs.byteregs[regdl]<0x80) {
								regs.byteregs[regbl] = 4; //else regs.byteregs[regbl] = 0;
								regs.byteregs[regdl] = 2;
							}
						else regs.byteregs[regdl] = hdcount;
				//JJ 	}
				//JJ else {
				//JJ 		//cf = 1;
				//JJ 		ExternalSetCF(1);
				//JJ 		regs.byteregs[regah] = 0xAA;
				//JJ 	}
				break;
			default:
				//cf = 1;
				ExternalSetCF(1);
		}
	if (regs.byteregs[regdl] & 0x80)
    {
     //RAM[0x474] = regs.byteregs[regah];
     //write86(0x474,regs.byteregs[regah]);
	 #ifdef use_lib_sna_rare
	  jj_write86_remap(0x474,regs.byteregs[regah]);
	 #else
	  gb_ram_bank[0][0x474]= regs.byteregs[regah];
	 #endif
     //if (gb_use_remap_cartdridge==1)
     //{
     // jj_write86_remap(0x474,regs.byteregs[regah]);
     //}	 
	 //else
	 //{
	 // gb_ram_bank[0][0x474]= regs.byteregs[regah];
	 //}
    }
}
