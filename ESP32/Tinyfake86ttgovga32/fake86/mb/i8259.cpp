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


// i8259.c: functions to emulate the Intel 8259 prioritized interrupt controller.
//   note: this is not a very complete 8259 implementation, but for the purposes
//   of a PC, it's all we need.

#include <stdint.h>
#include <string.h>
#include "mb/i8259.h"
#include "cpu/ports.h"
#include "config/gbConfig.h"
#include "gbGlobals.h"

struct structpic i8259;

uint8_t keyboardwaitack;
//  set_port_write_redirector(0x20, 0x21, (void *)&out8259);
//  set_port_read_redirector(0x20, 0x21, (void *)&in8259);
static uint8_t in8259(uint32_t portnum);
static void out8259(uint32_t portnum, uint8_t value);

IOPort port_020h = IOPort(0x020, 0xFF, in8259, out8259);
IOPort port_021h = IOPort(0x021, 0xFF, in8259, out8259);
IOPort port_0A0h = IOPort(0x0A0, 0xFF, nullptr, nullptr);

uint8_t in8259(uint32_t portnum)
{
  switch (portnum & 1) {
		   case 0:
			if (i8259.readmode==0) return(i8259.irr); else return(i8259.isr);
		   case 1: //read mask register
			return(i8259.imr);
	}
	return (0);
}

extern uint32_t makeupticks;
void out8259(uint32_t portnum, uint8_t value) {
	 uint8_t i;
	 switch (portnum & 1) {
		case 0:
		 if (value & 0x10) { //begin initialization sequence
			i8259.icwstep = 1;
			i8259.imr = 0; //clear interrupt mask register
			i8259.icw[i8259.icwstep++] = value;
			return;
		 }
		 if ((value & 0x98)==8) { //it's an OCW3
			if (value & 2) i8259.readmode = value & 2;
		 }
		 if (value & 0x20) { //EOI command
	keyboardwaitack = 0;
			for (i=0; i<8; i++)
			if ((i8259.isr >> i) & 1) {
			   i8259.isr ^= (1 << i);
        // Figure out why this shit is here: 
			  //  if ((i==0) && (makeupticks>0)) { makeupticks = 0; i8259.irr |= 1; }
			   return;
			}
		 }
		 break;
		case 1:
		 if ((i8259.icwstep==3) && (i8259.icw[1] & 2)) i8259.icwstep = 4; //single mode, so don't read ICW3
		 if (i8259.icwstep<5) { i8259.icw[i8259.icwstep++] = value; return; }
		 //if we get to this point, this is just a new IMR value
		 i8259.imr = value;
		 break;
	 }
}

uint8_t nextintr() {
	uint8_t i, tmpirr;
	tmpirr = i8259.irr & (~i8259.imr); //XOR request register with inverted mask register
	for (i=0; i<8; i++)
		if ((tmpirr >> i) & 1) {
		   i8259.irr ^= (1 << i);
		   i8259.isr |= (1 << i);
		   return(i8259.icw[2] + i);
		}
	return(0); //this won't be reached, but without it the compiler gives a warning
}

#ifdef use_lib_fast_doirq

#else
 void doirq(unsigned char irqnum)
 {
  static uint32_t counter_1C = 0;
  static uint32_t timestamp_1C = millis();
  if (irqnum == 0x00)
  {
    counter_1C++;
    if (counter_1C >= 1000)
    {
       uint32_t now = millis();
       uint32_t period = (now - timestamp_1C);
       LOG("INT 1Ch: 1000 per %i ms\n", period);
       timestamp_1C = now;
       counter_1C = 0;
    }
  }

  i8259.irr |= (1 << irqnum);
  if (irqnum == 1)
  {
    keyboardwaitack = 1;
  }  
 } 
#endif 

void init8259() {
	 memset((void *)&i8259, 0, sizeof(i8259));
	//  set_port_write_redirector(0x20, 0x21, (void *)&out8259);
	//  set_port_read_redirector(0x20, 0x21, (void *)&in8259);
}
