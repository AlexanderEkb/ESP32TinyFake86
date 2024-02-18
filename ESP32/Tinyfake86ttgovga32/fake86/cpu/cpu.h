//
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
#ifndef _CPU_H
 #define _CPU_H

// #include <time.h>
#include <stdint.h>

#define regax 0
#define regcx 1
#define regdx 2
#define regbx 3
#define regsp 4
#define regbp 5
#define regsi 6
#define regdi 7
#define reges 0
#define regcs 1
#define regss 2
#define regds 3

#define regal 0
#define regah 1
#define regcl 2
#define regch 3
#define regdl 4
#define regdh 5
#define regbl 6
#define regbh 7

typedef enum : uint32_t
{
  _dbgReg_IP,
  _dbgReg_AX,
  _dbgReg_BX,
  _dbgReg_CX,
  _dbgReg_DX,
  _dbgReg_SP,
  _dbgReg_BP,
  _dbgReg_SI,
  _dbgReg_DI,
  _dbgReg_F,
  _dbgReg_CS,
  _dbgReg_DS,
  _dbgReg_SS,
  _dbgReg_ES,

  _dbgReg__COUNT
} _dbgReg_t;

union _bytewordregs_ {
	uint16_t wordregs[8];
	uint8_t byteregs[8];
};

void intcall86 (uint8_t intnum);

void SetRegCS(unsigned short int a);
void SetRegIP(unsigned short int a);
void SetRegDS(unsigned short int a);
void SetRegSS(unsigned short int a);
void SetRegES(unsigned short int a);

void SetRegSP(unsigned short int a);
void SetRegBP(unsigned short int a);
void SetRegSI(unsigned short int a);
void SetRegDI(unsigned short int a);

void SetCF(unsigned short int a);

unsigned char read86 (unsigned int addr32);
void write86 (unsigned int addr32, unsigned char value);
void reset86(void);
void exec86 (uint32_t count);

void updateBIOSDataArea(void);
void ExternalSetCF(unsigned char valor);

void my_callback_speaker_func(void);

uint16_t _dbgGetRegister(_dbgReg_t reg);
void _dbgSetRegister(_dbgReg_t reg, uint16_t val);
#endif
