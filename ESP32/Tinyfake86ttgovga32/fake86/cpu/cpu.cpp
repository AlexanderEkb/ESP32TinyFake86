//Convertir
//if (dst & 0xFF00) {
//cf = 1;
//}
//else {
//cf = 0;
//}
//
//En
//cf = (dst & 0xFF00) != 0;

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
//
// cpu.c: functions to emulate the 8086/V20 CPU in software. the heart of Fake86.

#include "cpu.h"
#include "config/config.h"
#include "config/gbConfig.h"
#include "config/hardware.h"
#include "dataFlash/bios/biospcxt.h"
#include "dataFlash/rom/rombasic.h"
#include "dataFlash/rom/videorom.h"
#include "io/disk.h"
#include "gbGlobals.h"
#include "mb/i8253.h"
#include "mb/i8259.h"
#include "cpu/ports.h"
#include "video/video.h"
#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>

#define StepIP(x) ip += x
#define getmem8(x, y) read86(segbase(x) + y)
#define getmem16(x, y) readw86(segbase(x) + y)
#define putmem8(x, y, z) write86(segbase(x) + y, z)
#define putmem16(x, y, z) writew86(segbase(x) + y, z)
#define signext(value) (int16_t)(int8_t)(value)
#define signext32(value) (int32_t)(int16_t)(value)
#define getreg16(regid) regs.wordregs[regid]
#define getreg8(regid) regs.byteregs[byteregtable[regid]]
#define putreg16(regid, writeval) regs.wordregs[regid] = writeval
#define putreg8(regid, writeval) regs.byteregs[byteregtable[regid]] = writeval
#define getsegreg(regid) segregs[regid]
#define putsegreg(regid, writeval) segregs[regid] = writeval
#define segbase(x) ((uint32_t)x << 4)

extern struct i8253_s i8253;
extern uint8_t * ram;
extern struct structpic i8259;
uint64_t curtimer, lasttimer, timerfreq;

static unsigned char byteregtable[8] = { regal, regcl, regdl, regbl, regah, regch, regdh, regbh };
unsigned short int segregs[4];

static const uint8_t parity[0x100] = {
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1
};

static unsigned char opcode, segoverride, reptype;
static unsigned short int savecs, saveip, ip, useseg, oldsp;
static unsigned char tempcf, oldcf, pf, af, zf, sf, tf, ifl, df, of, mode, reg, rm;
static unsigned short int oper1, oper2, res16, disp16, temp16, dummy, stacksize, frametemp;
static unsigned char oper1b, oper2b, res8, disp8, temp8, nestlev, addrbyte;
static unsigned int temp1, temp2, temp3, temp4, temp5, temp32, tempaddr32, ea;
uint64_t totalexec;

union _bytewordregs_ regs;
unsigned char didbootstrap = 0;

static IOPortSpace & ports = IOPortSpace::getInstance();

extern uint8_t readVGA (uint32_t addr32);

void ExternalSetCF(unsigned char valor)
{
 cf= valor;
}

void intcall86 (unsigned char intnum);

void SetRegCS(unsigned short int a){ segregs[regcs]= a; }
void SetRegDS(unsigned short int a){ segregs[regds]= a; }
void SetRegSS(unsigned short int a){ segregs[regss]= a; }
void SetRegES(unsigned short int a){ segregs[reges]= a; }

void SetRegIP(unsigned short int a){ ip= a; }
void SetRegSP(unsigned short int a){ regs.wordregs[regsp]= a; }
void SetRegBP(unsigned short int a){ regs.wordregs[regbp]= a; }
void SetRegSI(unsigned short int a){ regs.wordregs[regsi]= a; }
void SetRegDI(unsigned short int a){ regs.wordregs[regdi]= a; }

void SetCF(unsigned short int a){ cf= a; }


unsigned char gb_check_memory_before;

 #define makeflagsword() \
	( \
	2 | (uint16_t) cf | ((uint16_t) pf << 2) | ((uint16_t) af << 4) | ((uint16_t) zf << 6) | ((uint16_t) sf << 7) | \
	((uint16_t) tf << 8) | ((uint16_t) ifl << 9) | ((uint16_t) df << 10) | ((uint16_t) of << 11) \
	)

 static inline void decodeflagsword(unsigned short int x)
 {//Es mas rapido metodo inline que define original
  temp16 = x;
  cf = temp16 & 1;
  pf = (temp16 >> 2) & 1;
  af = (temp16 >> 4) & 1;
  zf = (temp16 >> 6) & 1;
  sf = (temp16 >> 7) & 1;
  tf = (temp16 >> 8) & 1;
  ifl = (temp16 >> 9) & 1;
  df = (temp16 >> 10) & 1;
  of = (temp16 >> 11) & 1;
 }

//Lo saco fuera de Read86. Se ejecuta al inicio y en timer 54 ms.
void updateBIOSDataArea()
{ 
	if (!didbootstrap)
	{
		ram[0x410]= 0x61;	// Equipment word: no FPU, no mouse, two floppies, EGA or better
		ram[0x475]= 1;			// Number of HDDs intalled

		unsigned char ram_size_low  = (static_cast<uint8_t>((RAM_SIZE / 1024) >> 0));
		unsigned char ram_size_high = (static_cast<uint8_t>((RAM_SIZE / 1024) >> 8));
		ram[0x413] = ram_size_low;	// Amount of RAM, in Kbytes
		ram[0x414] = ram_size_high;    
 }
}

//********************************************************
void write86 (unsigned int addr32, unsigned char value)
{
 unsigned char idRAM;
 unsigned short int auxOffs;

 //Primero video
 //Primero CGA
 //if ((addr32 >= 0xB8000) && (addr32 < (0xB8000+16384)))
 if ((addr32 >= 0xB8000) && (addr32 < 0xBC000))
 {  
  gb_video_cga[(addr32-0xB8000)]= value;
  //updatedscreen = 1;
  return;
 }

 //Segundo memoria
 if ((addr32>=0) && (addr32<RAM_SIZE))
 {
  ram[addr32]= value;
  return;
 }

 if (addr32 > 1048575)
 {
  addr32 = addr32 & 0xFFFFF; //FIX EXPAND MICROSOFT ERROR MADMIX GAME

  ram[addr32] = value;  
 }

}

 static inline void writew86 (unsigned int addr32, unsigned short int value)
 {
  write86 (addr32, (unsigned char) value);
  write86 (addr32 + 1, (unsigned char) (value >> 8) );
 } 

unsigned char read86 (unsigned int addr32) 
{
 //BIOS ADDR
 unsigned short int auxOffs;
 unsigned char idRAM;

 //Primero video
 //Primero CGA
 //if ((addr32 >= 0xB8000) && (addr32 < (0xB8000+16384)))
 if ((addr32 >= 0xB8000) && (addr32 < 0xBC000))
 {   
  return (gb_video_cga[(addr32-0xB8000)]);
 } 

 //Segundo memoria RAM
 if ((addr32>=0) && (addr32<RAM_SIZE))
 {
  return (ram[addr32]);
 }
 //if ((addr32 >= 0xFE000) && (addr32 < (0xFE000 + gb_size_rom_bios_pcxt)))
 if ((addr32 >= 0xFE000) && (addr32 < 0x100000))
 {
  return gb_bios_pcxt[(addr32-0xFE000)];
 }
 //if ((addr32 >= 0xF6000) && (addr32 < (0xF6000 + gb_size_rom_basic)))
 if ((addr32 >= 0xF6000) && (addr32 < 0xFE000))
 {
  return gb_rom_basic[(addr32-0xF6000)];
 }
 //if ((addr32 >= 0xC0000) && (addr32 < (0xC0000 + gb_size_rom_videorom)))
 if ((addr32 >= 0xC0000) && (addr32 < 0xC8000))
 {
  return gb_rom_videorom[(addr32-0xC0000)];
 }
 if (addr32 > 1048575)
 {
  addr32 = addr32 & 0xFFFFF; //FIX EXPAND MICROSOFT ERROR MADMIX GAME
  return (ram[addr32]);
 }
 return 0xFF; 
}

 static inline unsigned short int readw86 (unsigned int addr32)
 {
  return ( (unsigned short int) read86 (addr32) | (unsigned short int) (read86 (addr32 + 1) << 8) );
 }



 static inline void flag_szp8(unsigned char value)
 {
  zf= (!value)?1:0; //set or clear zero flag  
  sf= (value & 0x80)?1:0; //set or clear sign flag
  pf = parity[value]; //retrieve parity state from lookup table
 }


 static inline void flag_szp16(unsigned short int value)
 {	 
  zf= (!value)?1:0; //set or clear zero flag
  sf= (value & 0x8000)?1:0; //set or clear sign flag
  pf = parity[value & 255];	//retrieve parity state from lookup table
 }

 static inline void flag_log8(unsigned char value)
 {
  flag_szp8(value);
  cf=0;
  of=0; //bitwise logic ops always clear carry and overflow
 }


 static inline void flag_log16(unsigned short int value)
 {
  flag_szp16(value);
  cf=0;
  of=0; //bitwise logic ops always clear carry and overflow
 }

 static inline void flag_adc8 (unsigned char v1, unsigned char v2, unsigned char v3)
{//v1 = destination operand, v2 = source operand, v3 = carry flag 
 unsigned short int dst;
 dst = (unsigned short int) v1 + (unsigned short int) v2 + (unsigned short int) v3;
 flag_szp8 ( (unsigned char) dst);
 of= ( ( (dst ^ v1) & (dst ^ v2) & 0x80) == 0x80)?1:0; //set or clear overflow flag
 cf= (dst & 0xFF00)?1:0; // set or clear carry flag 
 af= ( ( (v1 ^ v2 ^ dst) & 0x10) == 0x10)?1:0; // set or clear auxilliary flag
}

 static inline void flag_adc16 (unsigned short int v1, unsigned short int v2, unsigned short int v3)
 {
  unsigned int dst;
  dst = (unsigned int) v1 + (unsigned int) v2 + (unsigned int) v3;
  flag_szp16 ( (unsigned short int) dst);
  of= ( ( ( (dst ^ v1) & (dst ^ v2) ) & 0x8000) == 0x8000)?1:0;
  cf= (dst & 0xFFFF0000)?1:0;
  af= ( ( (v1 ^ v2 ^ dst) & 0x10) == 0x10)?1:0;
 }


 static inline void flag_add8 (unsigned char v1, unsigned char v2)
 {
  //v1 = destination operand, v2 = source operand
  unsigned short int dst;
  dst = (unsigned short int) v1 + (unsigned short int) v2;
  flag_szp8 ( (unsigned char) dst);
  cf= (dst & 0xFF00)?1:0;
  of= ( ( (dst ^ v1) & (dst ^ v2) & 0x80) == 0x80)?1:0;
  af= ( ( (v1 ^ v2 ^ dst) & 0x10) == 0x10)?1:0;
 }


 static inline void flag_add16 (uint16_t v1, uint16_t v2)
 {
  //v1 = destination operand, v2 = source operand
  unsigned int dst;
  dst = (unsigned int) v1 + (unsigned int) v2;
  flag_szp16 ( (unsigned short int) dst);  
  cf= (dst & 0xFFFF0000)?1:0;
  of= ( ( (dst ^ v1) & (dst ^ v2) & 0x8000) == 0x8000)?1:0;
  af= ( ( (v1 ^ v2 ^ dst) & 0x10) == 0x10)?1:0;
 }


 static inline void flag_sbb8 (unsigned char v1, unsigned char v2, unsigned char v3)
 {
  //v1 = destination operand, v2 = source operand, v3 = carry flag
  unsigned short int dst;
  v2 += v3;
  dst = (unsigned short int) v1 - (unsigned short int) v2;
  flag_szp8 ( (unsigned short int) dst);
  cf= (dst & 0xFF00)?1:0;
  of= ( (dst ^ v1) & (v1 ^ v2) & 0x80)?1:0;
  af= ( (v1 ^ v2 ^ dst) & 0x10)?1:0;
 }


 static inline void flag_sbb16 (unsigned short int v1, unsigned short int v2, unsigned short int v3)
 {
  //v1 = destination operand, v2 = source operand, v3 = carry flag
  unsigned int dst;
  v2 += v3;
  dst = (unsigned int) v1 - (unsigned int) v2;
  flag_szp16 ( (uint16_t) dst);
  cf= (dst & 0xFFFF0000)?1:0;
  of= ( (dst ^ v1) & (v1 ^ v2) & 0x8000)?1:0;
  af= ( (v1 ^ v2 ^ dst) & 0x10)?1:0;
 }
 

 static inline void flag_sub8 (unsigned char v1, unsigned char v2)
 {//v1 = destination operand, v2 = source operand
  unsigned short int dst;
  dst = (unsigned short int) v1 - (unsigned short int) v2;
  flag_szp8 ( (unsigned char) dst);
  cf= (dst & 0xFF00)?1:0;
  of= ( (dst ^ v1) & (v1 ^ v2) & 0x80)?1:0; 
  af= ( (v1 ^ v2 ^ dst) & 0x10)?1:0;
 }

 static inline void flag_sub16 (unsigned short int v1, unsigned short int v2)
 {//v1 = destination operand, v2 = source operand
  unsigned int dst;
  dst = (unsigned int) v1 - (unsigned int) v2;
  flag_szp16 ( (unsigned short int) dst);
  cf= (dst & 0xFFFF0000)?1:0;
  of= ( (dst ^ v1) & (v1 ^ v2) & 0x8000)?1:0;
  af= ( (v1 ^ v2 ^ dst) & 0x10)?1:0;
 }

 static inline void op_adc8()
 {
  res8 = oper1b + oper2b + cf;
  flag_adc8 (oper1b, oper2b, cf);
 }

 static inline void op_adc16()
 {
  res16 = oper1 + oper2 + cf;
  flag_adc16 (oper1, oper2, cf);
 }

 static inline void op_add8()
 {
  res8 = oper1b + oper2b;
  flag_add8 (oper1b, oper2b);
 }

 static inline void op_add16()
 {
  res16 = oper1 + oper2;
  flag_add16 (oper1, oper2);
 }


 static inline void op_and8()
 {
  res8 = oper1b & oper2b;
  flag_log8 (res8);
 }


 static inline void op_and16()
 {
  res16 = oper1 & oper2;
  flag_log16 (res16);
 }



 static inline void op_or8()
 {
  res8 = oper1b | oper2b;
  flag_log8 (res8);
 } 


 static inline void op_or16()
 {
  res16 = oper1 | oper2;
  flag_log16 (res16);
 }


 static inline void op_xor8()
 {
  res8 = oper1b ^ oper2b;
  flag_log8 (res8);
 }


 static inline void op_xor16()
 {
  res16 = oper1 ^ oper2;
  flag_log16((unsigned short int)res16);
 }

 static inline void op_sub8()
 {
  res8 = oper1b - oper2b;
  flag_sub8 (oper1b, oper2b);
 }

 static inline void op_sub16()
 {
  res16 = oper1 - oper2;
  flag_sub16 (oper1, oper2);
 }


 static inline void op_sbb8()
 {
  res8 = oper1b - (oper2b + cf);
  flag_sbb8 (oper1b, oper2b, cf);
 }

 static inline void op_sbb16()
 {
  res16 = oper1 - (oper2 + cf);
  flag_sbb16 (oper1, oper2, cf);
 }

 void modregrm()
 { // Es mas rapido la funcion que la macro original
   addrbyte = getmem8(segregs[regcs], ip);
   StepIP(1);
   mode = addrbyte >> 6;
   reg = (addrbyte >> 3) & 7;
   rm = addrbyte & 7;
   switch (mode)
   {
   case 0:
     if (rm == 6)
     {
       disp16 = getmem16(segregs[regcs], ip);
       StepIP(2);
     }
     if (((rm == 2) || (rm == 3)) && !segoverride)
     {
       useseg = segregs[regss];
     }
     break;
   case 1:
     disp16 = signext(getmem8(segregs[regcs], ip));
     StepIP(1);
     if (((rm == 2) || (rm == 3) || (rm == 6)) && !segoverride)
     {
       useseg = segregs[regss];
     }
     break;
   case 2:
     disp16 = getmem16(segregs[regcs], ip);
     StepIP(2);
     if (((rm == 2) || (rm == 3) || (rm == 6)) && !segoverride)
     {
       useseg = segregs[regss];
     }
     break;
   default:
     disp8 = 0;
     disp16 = 0;
   }
 }

void getea (uint8_t rmval)
{
	uint32_t	tempea;

	tempea = 0;
	switch (mode) {
			case 0:
				switch (rmval) {
						case 0:
							tempea = regs.wordregs[regbx] + regs.wordregs[regsi];
							break;
						case 1:
							tempea = regs.wordregs[regbx] + regs.wordregs[regdi];
							break;
						case 2:
							tempea = regs.wordregs[regbp] + regs.wordregs[regsi];
							break;
						case 3:
							tempea = regs.wordregs[regbp] + regs.wordregs[regdi];
							break;
						case 4:
							tempea = regs.wordregs[regsi];
							break;
						case 5:
							tempea = regs.wordregs[regdi];
							break;
						case 6:
							tempea = disp16;
							break;
						case 7:
							tempea = regs.wordregs[regbx];
							break;
					}
				break;

			case 1:
			case 2:
				switch (rmval) {
						case 0:
							tempea = regs.wordregs[regbx] + regs.wordregs[regsi] + disp16;
							break;
						case 1:
							tempea = regs.wordregs[regbx] + regs.wordregs[regdi] + disp16;
							break;
						case 2:
							tempea = regs.wordregs[regbp] + regs.wordregs[regsi] + disp16;
							break;
						case 3:
							tempea = regs.wordregs[regbp] + regs.wordregs[regdi] + disp16;
							break;
						case 4:
							tempea = regs.wordregs[regsi] + disp16;
							break;
						case 5:
							tempea = regs.wordregs[regdi] + disp16;
							break;
						case 6:
							tempea = regs.wordregs[regbp] + disp16;
							break;
						case 7:
							tempea = regs.wordregs[regbx] + disp16;
							break;
					}
				break;
		}

	ea = (tempea & 0xFFFF) + (useseg << 4);
}

 #define push(x) \ 
  putreg16(regsp,getreg16(regsp)-2); \
  putmem16(segregs[regss],getreg16(regsp),x); \
   
 static inline unsigned short int pop()
 {
  uint16_t tempval = getmem16 (segregs[regss], getreg16 (regsp) );
  putreg16 (regsp, getreg16 (regsp) + 2);
  return tempval;
 }  

void reset86() {
	segregs[regcs] = 0xFFFF;
	ip = 0x0000;
  updateBIOSDataArea();
}

 static inline unsigned short int readrm16 (unsigned char rmval)
 {
  if (mode < 3)
  {
   getea (rmval);
   return read86 (ea) | ( (unsigned short int) read86 (ea + 1) << 8);
  }
  else
  {
   return getreg16 (rmval);
  }
}

 static inline unsigned char readrm8 (unsigned char rmval) 
 {
  if (mode < 3)
  {
   getea (rmval);
   return read86 (ea);
  }
  else
  {
   return getreg8 (rmval);
  }
}

 static inline void writerm16 (unsigned char rmval, unsigned short int value)
 {
  if (mode < 3)
  {
   getea (rmval);
   write86 (ea, value & 0xFF);
   write86 (ea + 1, value >> 8);
  }
  else
  {
   putreg16 (rmval, value);
  }
}

 static inline void writerm8 (unsigned char rmval, unsigned char value)
 {
  if (mode < 3)
  {
   getea (rmval);
   write86 (ea, value);
  }
  else
  {
   putreg8 (rmval, value);
  }
}

extern void diskhandler();

void intcall86(unsigned char intnum)
{
  switch (intnum)
  {
  /************************************/
  /******** INT19H : Bootstrap ********/
  /************************************/
  case 0x19: // bootstrap
    didbootstrap = 1;
    bootdrive = getBootDrive();
    if (bootdrive < 255)
    { // read first sector of boot drive into 07C0:0000 and execute it
      regs.byteregs[regdl] = bootdrive;
      DISK_ADDR src = DISK_ADDR(bootdrive, 0, 0, 1, 1);
      MEM_ADDR dst = MEM_ADDR(0x07C0, 0x0000);
      readdisk(src, dst);
      segregs[regcs] = 0x0000;
      ip = 0x7C00;
    }
    else
    {
      segregs[regcs] = 0xF600; // start ROM BASIC at bootstrap if requested
      ip = 0x0000;
    }
    return;
  /************************************/
  /********** INT13H : Disks **********/
  /************************************/
  case 0x13:
  case 0xFD:
    diskhandler();
    return;
  }

  push(makeflagsword());
  push(segregs[regcs]);
  push(ip);
  segregs[regcs] = getmem16(0, (uint16_t)intnum * 4 + 2);
  ip = getmem16(0, (uint16_t)intnum * 4);
  ifl = 0;
  tf = 0;
}

extern uint8_t	nextintr();
extern void	i8253Exec();

static uint32_t loopcount;
static uint16_t firstip;

#include "cpu-instructions.inc"

void __attribute__((optimize("-Ofast"))) IRAM_ATTR exec86(uint32_t execloops)
{

	uint8_t	docontinue;
  static uint16_t trap_toggle = 0;

	for (loopcount = 0; loopcount < execloops; loopcount++)
	{

   #ifdef use_lib_speaker_cpu
      my_callback_speaker_func();
	 #endif 

			if ( (totalexec & 31) == 0)
      {
        videoExecCpu();
        i8253Exec();
      }

			if (trap_toggle) {
					intcall86 (1);
				}
            trap_toggle=  (tf)?1:0;
			if (!trap_toggle && (ifl && (i8259.irr & (~i8259.imr) ) ) ) {
					intcall86 (nextintr() );	/* get next interrupt from the i8259, if any */
				}

			reptype = 0;
			segoverride = 0;
			useseg = segregs[regds];
			docontinue = 0;
			firstip = ip;

			if ( (segregs[regcs] == 0xF000) && (ip == 0xE066) ) {
        didbootstrap = 0; //detect if we hit the BIOS entry point to clear didbootstrap because we've rebooted
        updateBIOSDataArea();
      }

			while (!docontinue) {
					segregs[regcs] = segregs[regcs] & 0xFFFF;
					ip = ip & 0xFFFF;
					savecs = segregs[regcs];
					saveip = ip;
					opcode = getmem8 (segregs[regcs], ip);
					StepIP (1);

					switch (opcode) {
								/* segment prefix check */
							case 0x2E:	/* segment segregs[regcs] */
								useseg = segregs[regcs];
								segoverride = 1;
								break;

							case 0x3E:	/* segment segregs[regds] */
								useseg = segregs[regds];
								segoverride = 1;
								break;

							case 0x26:	/* segment segregs[reges] */
								useseg = segregs[reges];
								segoverride = 1;
								break;

							case 0x36:	/* segment segregs[regss] */
								useseg = segregs[regss];
								segoverride = 1;
								break;

								/* repetition prefix check */
							case 0xF3:	/* REP/REPE/REPZ */
								reptype = 1;
								break;

							case 0xF2:	/* REPNE/REPNZ */
								reptype = 2;
								break;

							default:
								docontinue = 1;
								break;
						}
				}

			totalexec++;

      typedef void (* opcode_t)(void);
      #ifdef CPU_V20
      static const opcode_t opcodes[256] = {
          opcode0x00, opcode0x01, opcode0x02, opcode0x03, opcode0x04, opcode0x05, opcode0x06, opcode0x07, opcode0x08, opcode0x09, opcode0x0A, opcode0x0B, opcode0x0C, opcode0x0D, opcode0x0E, opcode0x0F,
          opcode0x10, opcode0x11, opcode0x12, opcode0x13, opcode0x14, opcode0x15, opcode0x16, opcode0x17, opcode0x18, opcode0x19, opcode0x1A, opcode0x1B, opcode0x1C, opcode0x1D, opcode0x1E, opcode0x1F,
          opcode0x20, opcode0x21, opcode0x22, opcode0x23, opcode0x24, opcode0x25, opcodeStub, opcode0x27, opcode0x28, opcode0x29, opcode0x2A, opcode0x2B, opcode0x2C, opcode0x2D, opcodeStub, opcode0x2F,
          opcode0x30, opcode0x31, opcode0x32, opcode0x33, opcode0x34, opcode0x35, opcodeStub, opcode0x37, opcode0x38, opcode0x39, opcode0x3A, opcode0x3B, opcode0x3C, opcode0x3D, opcodeStub, opcode0x3F,
          opcode0x40, opcode0x41, opcode0x42, opcode0x43, opcode0x44, opcode0x45, opcode0x46, opcode0x47, opcode0x48, opcode0x49, opcode0x4A, opcode0x4B, opcode0x4C, opcode0x4D, opcode0x4E, opcode0x4F,
          opcode0x50, opcode0x51, opcode0x52, opcode0x53, opcode0x54, opcode0x55, opcode0x56, opcode0x57, opcode0x58, opcode0x59, opcode0x5A, opcode0x5B, opcode0x5C, opcode0x5D, opcode0x5E, opcode0x5F,
          opcode0x60, opcode0x61, opcode0x62, opcodeInv , opcodeInv , opcodeInv , opcodeInv , opcodeInv , opcode0x68, opcode0x69, opcode0x6A, opcode0x6B, opcode0x6C, opcode0x6D, opcode0x6E, opcode0x6F,
          opcode0x70, opcode0x71, opcode0x72, opcode0x73, opcode0x74, opcode0x75, opcode0x76, opcode0x77, opcode0x78, opcode0x79, opcode0x7A, opcode0x7B, opcode0x7C, opcode0x7D, opcode0x7E, opcode0x7F,
          opcode0x80, opcode0x81, opcode0x82, opcode0x83, opcode0x84, opcode0x85, opcode0x86, opcode0x87, opcode0x88, opcode0x89, opcode0x8A, opcode0x8B, opcode0x8C, opcode0x8D, opcode0x8E, opcode0x8F,
          opcode0x90, opcode0x91, opcode0x92, opcode0x93, opcode0x94, opcode0x95, opcode0x96, opcode0x97, opcode0x98, opcode0x99, opcode0x9A, opcode0x9B, opcode0x9C, opcode0x9D, opcode0x9E, opcode0x9F,
          opcode0xA0, opcode0xA1, opcode0xA2, opcode0xA3, opcode0xA4, opcode0xA5, opcode0xA6, opcode0xA7, opcode0xA8, opcode0xA9, opcode0xAA, opcode0xAB, opcode0xAC, opcode0xAD, opcode0xAE, opcode0xAF,
          opcode0xB0, opcode0xB1, opcode0xB2, opcode0xB3, opcode0xB4, opcode0xB5, opcode0xB6, opcode0xB7, opcode0xB8, opcode0xB9, opcode0xBA, opcode0xBB, opcode0xBC, opcode0xBD, opcode0xBE, opcode0xBF,
          opcode0xC0, opcode0xC1, opcode0xC2, opcode0xC3, opcode0xC4, opcode0xC5, opcode0xC6, opcode0xC7, opcode0xC8, opcode0xC9, opcode0xCA, opcode0xCB, opcode0xCC, opcode0xCD, opcode0xCE, opcode0xCF,
          opcode0xD0, opcode0xD1, opcode0xD2, opcode0xD3, opcode0xD4, opcode0xD5, opcode0xD6, opcode0xD7, opcodeSkip, opcodeSkip, opcodeSkip, opcodeSkip, opcodeSkip, opcodeSkip, opcodeSkip, opcodeSkip,
          opcode0xE0, opcode0xE1, opcode0xE2, opcode0xE3, opcode0xE4, opcode0xE5, opcode0xE6, opcode0xE7, opcode0xE8, opcode0xE9, opcode0xEA, opcode0xEB, opcode0xEC, opcode0xED, opcode0xEE, opcode0xEF,
          opcode0xF0, opcodeInv , opcodeStub, opcodeStub, opcode0xF4, opcode0xF5, opcode0xF6, opcode0xF7, opcode0xF8, opcode0xF9, opcode0xFA, opcode0xFB, opcode0xFC, opcode0xFD, opcode0xFE, opcode0xFF,
      };
      #else
      static const opcode_t opcodes[256] = {
          opcode0x00, opcode0x01, opcode0x02, opcode0x03, opcode0x04, opcode0x05, opcode0x06, opcode0x07, opcode0x08, opcode0x09, opcode0x0A, opcode0x0B, opcode0x0C, opcode0x0D, opcode0x0E, opcode0x0F,
          opcode0x10, opcode0x11, opcode0x12, opcode0x13, opcode0x14, opcode0x15, opcode0x16, opcode0x17, opcode0x18, opcode0x19, opcode0x1A, opcode0x1B, opcode0x1C, opcode0x1D, opcode0x1E, opcode0x1F,
          opcode0x20, opcode0x21, opcode0x22, opcode0x23, opcode0x24, opcode0x25, opcodeStub, opcode0x27, opcode0x28, opcode0x29, opcode0x2A, opcode0x2B, opcode0x2C, opcode0x2D, opcodeStub, opcode0x2F,
          opcode0x30, opcode0x31, opcode0x32, opcode0x33, opcode0x34, opcode0x35, opcodeStub, opcode0x37, opcode0x38, opcode0x39, opcode0x3A, opcode0x3B, opcode0x3C, opcode0x3D, opcodeStub, opcode0x3F,
          opcode0x40, opcode0x41, opcode0x42, opcode0x43, opcode0x44, opcode0x45, opcode0x46, opcode0x47, opcode0x48, opcode0x49, opcode0x4A, opcode0x4B, opcode0x4C, opcode0x4D, opcode0x4E, opcode0x4F,
          opcode0x50, opcode0x51, opcode0x52, opcode0x53, opcode0x54, opcode0x55, opcode0x56, opcode0x57, opcode0x58, opcode0x59, opcode0x5A, opcode0x5B, opcode0x5C, opcode0x5D, opcode0x5E, opcode0x5F,
          opcodeInv , opcodeInv , opcodeInv , opcodeInv , opcodeInv , opcodeInv , opcodeInv , opcodeInv , opcodeInv , opcodeInv , opcodeInv , opcodeInv , opcodeInv , opcodeInv , opcodeInv , opcodeInv ,
          opcode0x70, opcode0x71, opcode0x72, opcode0x73, opcode0x74, opcode0x75, opcode0x76, opcode0x77, opcode0x78, opcode0x79, opcode0x7A, opcode0x7B, opcode0x7C, opcode0x7D, opcode0x7E, opcode0x7F,
          opcode0x80, opcode0x81, opcode0x82, opcode0x83, opcode0x84, opcode0x85, opcode0x86, opcode0x87, opcode0x88, opcode0x89, opcode0x8A, opcode0x8B, opcode0x8C, opcode0x8D, opcode0x8E, opcode0x8F,
          opcode0x90, opcode0x91, opcode0x92, opcode0x93, opcode0x94, opcode0x95, opcode0x96, opcode0x97, opcode0x98, opcode0x99, opcode0x9A, opcode0x9B, opcode0x9C, opcode0x9D, opcode0x9E, opcode0x9F,
          opcode0xA0, opcode0xA1, opcode0xA2, opcode0xA3, opcode0xA4, opcode0xA5, opcode0xA6, opcode0xA7, opcode0xA8, opcode0xA9, opcode0xAA, opcode0xAB, opcode0xAC, opcode0xAD, opcode0xAE, opcode0xAF,
          opcode0xB0, opcode0xB1, opcode0xB2, opcode0xB3, opcode0xB4, opcode0xB5, opcode0xB6, opcode0xB7, opcode0xB8, opcode0xB9, opcode0xBA, opcode0xBB, opcode0xBC, opcode0xBD, opcode0xBE, opcode0xBF,
          opcode0xC0, opcode0xC1, opcode0xC2, opcode0xC3, opcode0xC4, opcode0xC5, opcode0xC6, opcode0xC7, opcode0xC8, opcode0xC9, opcode0xCA, opcode0xCB, opcode0xCC, opcode0xCD, opcode0xCE, opcode0xCF,
          opcode0xD0, opcode0xD1, opcode0xD2, opcode0xD3, opcode0xD4, opcode0xD5, opcode0xD6, opcode0xD7, opcodeSkip, opcodeSkip, opcodeSkip, opcodeSkip, opcodeSkip, opcodeSkip, opcodeSkip, opcodeSkip,
          opcode0xE0, opcode0xE1, opcode0xE2, opcode0xE3, opcode0xE4, opcode0xE5, opcode0xE6, opcode0xE7, opcode0xE8, opcode0xE9, opcode0xEA, opcode0xEB, opcode0xEC, opcode0xED, opcode0xEE, opcode0xEF,
          opcode0xF0, opcodeInv , opcodeStub, opcodeStub, opcode0xF4, opcode0xF5, opcode0xF6, opcode0xF7, opcode0xF8, opcode0xF9, opcode0xFA, opcode0xFB, opcode0xFC, opcode0xFD, opcode0xFE, opcode0xFF,
      };
      #endif
      opcodes[opcode]();
		}
}

uint16_t _dbgGetRegister(_dbgReg_t reg)
{
  switch (reg)
  {
    case _dbgReg_IP:
      return ip;
    case _dbgReg_AX:
      return regs.wordregs[regax];
    case _dbgReg_BX:
      return regs.wordregs[regbx];
    case _dbgReg_CX:
      return regs.wordregs[regcx];
    case _dbgReg_DX:
      return regs.wordregs[regdx];
    case _dbgReg_SI:
      return regs.wordregs[regsi];
    case _dbgReg_DI:
      return regs.wordregs[regdi];
    case _dbgReg_F:
      return makeflagsword();
    case _dbgReg_SP:
      return regs.wordregs[regsp];
    case _dbgReg_BP:
      return regs.wordregs[regbp];
    case _dbgReg_CS:
      return segregs[regcs];
    case _dbgReg_DS:
      return segregs[regds];
    case _dbgReg_SS:
      return segregs[regss];
    case _dbgReg_ES:
      return segregs[reges];
    default:
      return 0;
  }
}