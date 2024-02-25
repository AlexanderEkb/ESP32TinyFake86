/*
  Fake86: A portable, open-source 8086 PC emulator.
  Copyright (C)2010-2012 Mike Chambers

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/* i8253.c: functions to emulate the Intel 8253 programmable interval timer.
   these are required for the timer interrupt and PC speaker to be
   properly emulated! */

#include <stdint.h>
#include <stdio.h>
#include "mb/i8253.h"
#include "gbGlobals.h"
#include "cpu/ports.h"
#include <Arduino.h>
#include <string.h>
#include "io/speaker.h"

#define PIT_MODE_LATCHCOUNT 0
#define PIT_MODE_LOBYTE 1
#define PIT_MODE_HIBYTE 2
#define PIT_MODE_TOGGLE 3

typedef struct i8253_s
{
  uint16_t update[3];
  uint8_t accessmode[3];
  bool MSB[3];
  bool active[3];
  uint16_t counter[3];
} i8253_s;

struct i8253_s i8253;

extern uint64_t hostfreq, curtick;

static void out8253(uint32_t portnum, uint8_t value);
static uint8_t in8253(uint32_t portnum);

static void writeCounter(uint32_t address, uint8_t value);
static void writeControl(uint32_t address, uint8_t value);
static uint8_t readCounter(uint32_t address);
static uint8_t readControl(uint32_t address);

// IOPort port_040h = IOPort(0x040, 0xFF, in8253, out8253);
// IOPort port_041h = IOPort(0x041, 0xFF, in8253, out8253);
// IOPort port_042h = IOPort(0x042, 0xFF, in8253, out8253);
// IOPort port_043h = IOPort(0x043, 0xFF, in8253, out8253);
IOPort port_040h = IOPort(0x040, 0xFF, readCounter, writeCounter);
IOPort port_041h = IOPort(0x041, 0xFF, readCounter, writeCounter);
IOPort port_042h = IOPort(0x042, 0xFF, readCounter, writeCounter);
IOPort port_043h = IOPort(0x043, 0xFF, readControl, writeControl);

static void writeCounter(uint32_t address, uint8_t value)
{
  const uint32_t channel = address & 0x03;
  // LOG("Write ch%i: %02x\n", channel, value);
  const bool lobyte = 
    (i8253.accessmode[channel] == PIT_MODE_LOBYTE) || 
    ((i8253.accessmode[channel] == PIT_MODE_TOGGLE) && !i8253.MSB[channel]);
  const bool hibyte = 
    (i8253.accessmode[channel] == PIT_MODE_HIBYTE) || 
    ((i8253.accessmode[channel] == PIT_MODE_TOGGLE) && i8253.MSB[channel]);

  uint16_t & update = i8253.update[channel];
  if (lobyte)
    update = (update & 0xFF00) | value; // Lower byte
  else if (hibyte)
    update = (update & 0x00FF) | ((uint16_t)value << 8); // Upper byte

  i8253.active[channel] = true;

  if (i8253.accessmode[channel] == PIT_MODE_TOGGLE)
    i8253.MSB[channel] = !i8253.MSB[channel];

  if (channel == 0x02)
  {
    updateFrequency(i8253.update[2]);
  }
}

static void writeControl(uint32_t address, uint8_t value)
{
  const uint32_t channel = value >> 6;
  const uint8_t accessmode = (value >> 4) & 3;
  const uint8_t mode = (value >> 1) & 7;
  LOG("Write CR %02x: ch%02xh a%02x m%02x\n", value,  channel, accessmode, mode);

  i8253.accessmode[channel] = accessmode;
  if (i8253.accessmode[channel] == PIT_MODE_TOGGLE)
    i8253.MSB[channel] = false;
}

static uint8_t readCounter(uint32_t address)
{
  const uint32_t channel = address & 0x03;
  uint8_t curbyte = 0;

  if ((i8253.accessmode[channel] == 0) || (i8253.accessmode[channel] == PIT_MODE_LOBYTE) || ((i8253.accessmode[channel] == PIT_MODE_TOGGLE) && (i8253.MSB[channel] == 0)))
    curbyte = 0;
  else if ((i8253.accessmode[channel] == PIT_MODE_HIBYTE) || ((i8253.accessmode[channel] == PIT_MODE_TOGGLE) && (i8253.MSB[channel] == 1)))
    curbyte = 1;
  if ((i8253.accessmode[channel] == 0) || (i8253.accessmode[channel] == PIT_MODE_LOBYTE) || ((i8253.accessmode[channel] == PIT_MODE_TOGGLE) && (i8253.MSB[channel] == 0)))
    curbyte = 0;
  else if ((i8253.accessmode[channel] == PIT_MODE_HIBYTE) || ((i8253.accessmode[channel] == PIT_MODE_TOGGLE) && (i8253.MSB[channel] == 1)))
    curbyte = 1;
  if ((i8253.accessmode[channel] == 0) || (i8253.accessmode[channel] == PIT_MODE_TOGGLE))
    i8253.MSB[channel] = !i8253.MSB[channel];

  const uint8_t result = (curbyte == 0) ? ((uint8_t)i8253.counter[channel]) : ((uint8_t)(i8253.counter[channel] >> 8));
  LOG("Read ch%i: %02x\n", address, result);
  return result;
}

static uint8_t readControl(uint32_t address)
{
  (void)address;
  return 0;
}

void init8253()
{
  for(uint32_t channel=0; channel<3; channel++)
  {
    i8253.update[channel] = 0x0001;
    i8253.accessmode[channel] = 0x00;
    i8253.MSB[channel] = false;
    i8253.active[channel] = false;
    i8253.counter[channel] = 0x0001;
  }
}

/// @brief Feeds clock to the timer. Gets called each 4th CPU instruction executed.
/// @param  none
void __attribute__((optimize("-Ofast"))) IRAM_ATTR i8253Exec()
{
  for (uint32_t channel = 0; channel < 3; channel++)
  {
    if (i8253.active[channel])
    {
      uint16_t & CNTR = i8253.counter[channel];
      CNTR -= 0x10;
 
      if(CNTR < 0x10)
      {
        CNTR = i8253.update[channel];
        if (channel == 0) doirq(0);
      }
    }
  }
}
