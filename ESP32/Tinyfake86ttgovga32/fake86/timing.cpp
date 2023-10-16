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
// simulateCGARetrace.c: critical functions to provide accurate timing for the
//   system timer interrupt, and to generate new audio output samples.

#include "gbConfig.h"
#include "config.h"
#include "gbGlobals.h"
#include "timing.h"
//#include <SDL/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <Arduino.h>
//#ifdef _WIN32
// #include <Windows.h>
// LARGE_INTEGER queryperf;
//#else
// #include <sys/time.h>
// struct timeval tv;
//#endif
#include "i8253.h"
#include "i8259.h"
#include "cpu.h"
//JJ #include "blaster.h"

//JJ extern struct blaster_s blaster;
extern struct i8253_s i8253;
//extern void doirq (uint8_t irqnum);
//JJ extern void tickaudio(); //no audio
#ifdef use_lib_disneysound
 //JJ extern void tickssource();
#endif 
#ifdef use_lib_adlib
 //JJ extern void tickadlib();
#endif 
//JJ extern void tickBlaster(); //no audio

unsigned long hostfreq = 1000000;

unsigned int jj_cur_ms_tick, jj_last_ms_tick;

//uint16_t pit0counter = 65535; //No lo necesito
extern uint64_t totalexec;
extern uint32_t speed;

void inittiming() {
  jj_last_ms_tick= jj_cur_ms_tick= millis();
}


void simulateCGARetrace()
{
 //Fuerzo siempre 54 ms 18.2 ticks 
 //auxCurTick= (jj_last_ms_tick - jj_cur_ms_tick);
 //if (auxCurTick >= 54)
 jj_cur_ms_tick = millis();
 unsigned int auxCurTick = (jj_last_ms_tick - jj_cur_ms_tick);
 if (auxCurTick >= gb_timers_poll_milis)
 {
  jj_last_ms_tick = jj_cur_ms_tick;
  updateBIOSDataArea();  //Cada 54 milis
  if (i8253.active[0])
  {
   doirq(0);
  }

  for (uint32_t i8253chan=0; i8253chan<3; i8253chan++)
  {
   if (i8253.active[i8253chan]) 
   {
    if (i8253.counter[i8253chan] < 10)
     i8253.counter[i8253chan] = i8253.chandata[i8253chan];
    i8253.counter[i8253chan] -= 10;
   }
  }   
 }

 //auxCurTick= (jj_lasti8253_ms_tick - jj_cur_ms_tick);
 //if (auxCurTick >= 54) 
 //{
 // jj_lasti8253_ms_tick = jj_cur_ms_tick;   
 // for (i8253chan=0; i8253chan<3; i8253chan++)
 // {
 //  if (i8253.active[i8253chan]) 
 //  {
 //   if (i8253.counter[i8253chan] < 10)
 //    i8253.counter[i8253chan] = i8253.chandata[i8253chan];
 //   i8253.counter[i8253chan] -= 10;
 //  }
 // }   
 //}

}


