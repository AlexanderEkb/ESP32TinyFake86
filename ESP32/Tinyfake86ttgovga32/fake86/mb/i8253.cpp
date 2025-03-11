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
#include "mb/i8259.h"
#include "gbGlobals.h"
#include "cpu/ports.h"
#include <Arduino.h>
#include <string.h>
#include "io/speaker.h"

#include <driver/periph_ctrl.h>
#include <driver/timer.h>
#include <esp_log.h>

#define TAG "PIT"

#define PIT_MODE_LATCHCOUNT 0
#define PIT_MODE_LOBYTE 1
#define PIT_MODE_HIBYTE 2
#define PIT_MODE_TOGGLE 3

typedef struct i8253_s
{
  uint16_t update;
  uint8_t accessmode;
  bool MSB;
  bool active;
  // uint16_t counter;
  uint16_t latch;
  bool isLatched;
} i8253_s;

static const uint32_t CHANNEL_COUNT = 3;
struct i8253_s i8253[CHANNEL_COUNT];

extern uint64_t hostfreq, curtick;

static void initializeHWTimer(timer_group_t group, timer_idx_t idx, void (*isr)(void *));
static void IRAM_ATTR ch0isr(void * p);
static void IRAM_ATTR ch2isr(void * p);

static uint16_t getCounter(uint32_t channel);
static void setCounter(uint32_t channel, uint16_t value);
static void writeCounter(uint32_t address, uint8_t value);
static void writeControl(uint32_t address, uint8_t value);
static uint8_t readCounter(uint32_t address);
static uint8_t readControl(uint32_t address);

IOPort port_040h = IOPort(0x040, 0xFF, readCounter, writeCounter);
IOPort port_041h = IOPort(0x041, 0xFF, readCounter, writeCounter);
IOPort port_042h = IOPort(0x042, 0xFF, readCounter, writeCounter);
IOPort port_043h = IOPort(0x043, 0xFF, readControl, writeControl);

void init8253()
{
  periph_module_enable(PERIPH_TIMG0_MODULE);
  initializeHWTimer(TIMER_GROUP_0, TIMER_0, ch0isr);
  initializeHWTimer(TIMER_GROUP_0, TIMER_1, ch2isr);
  for(uint32_t channel=0; channel<3; channel++)
  {
    i8253[channel].update = 0x0000;
    i8253[channel].accessmode = 0x00;
    i8253[channel].MSB = false;
    i8253[channel].active = false;
  }
}

void gateCh2(bool state)
{
  if(state)
  {
    setCounter(TIMER_1, i8253[2].update);
    timer_start(TIMER_GROUP_0, TIMER_1);
  }
  else
  {
    timer_pause(TIMER_GROUP_0, TIMER_1);
    Speaker_t::driveByTimer(true);
  }
}

static void initializeHWTimer(timer_group_t group, timer_idx_t idx, void (*isr)(void *))
{
  static const uint32_t i8253_NOMINAL_CLOCK_HZ = 1193180;
  static const uint32_t TIMER_DIVIDER = TIMER_BASE_CLK / i8253_NOMINAL_CLOCK_HZ;
  timer_config_t config = {
    .alarm_en = TIMER_ALARM_EN,
    .counter_en = TIMER_PAUSE,
    .counter_dir = TIMER_COUNT_DOWN,
    .auto_reload = TIMER_AUTORELOAD_EN,
    .divider = TIMER_DIVIDER
  }; // default clock source is APB
  timer_init(group, idx, &config);
  timer_set_counter_value(group, idx, 0xFFFF);
  timer_set_alarm_value(group, idx, 0x0000);
  timer_enable_intr(group, idx);
  timer_isr_register(group, idx, isr, nullptr, ESP_INTR_FLAG_IRAM, nullptr);
  timer_start(group, idx);
}

static uint16_t getCounter(uint32_t channel)
{
  timer_idx_t idx;
  switch(channel)
  {
    case 0:
      idx = TIMER_0;
      break;
    case 2:
      idx = TIMER_1;
      break;
    default:
      return 0;
  }
  uint64_t value = 0;
  timer_get_counter_value(TIMER_GROUP_0, idx, &value);
  return static_cast<uint16_t>(value & 0xFFFF);
}

static void setCounter(uint32_t channel, uint16_t value)
{
  // ESP_LOGI(TAG, "setReload %i, %04X", channel, value);
  timer_idx_t idx;
  uint64_t counter = (value == 0) ? 0x10000ULL : static_cast<uint64_t>(value);
  switch(channel)
  {
    case 0:
      idx = TIMER_0;
      break;
    case 2:
      idx = TIMER_1;
      counter >>= 1;
      if(counter < 0x10ULL)
        counter = 0x10ULL;
      break;
    default:
      return;
  }
  timer_set_counter_value(TIMER_GROUP_0, idx, counter);
}

static void writeCounter(uint32_t address, uint8_t value)
{
  // ESP_LOGI(TAG, "%02X: %02X", static_cast<unsigned int>(address), static_cast<unsigned int>(value));
  const uint32_t channel = address & 0x03;

  const bool lobyte =
      (i8253[channel].accessmode == PIT_MODE_LOBYTE) ||
      ((i8253[channel].accessmode == PIT_MODE_TOGGLE) && !i8253[channel].MSB);
  const bool hibyte = 
    (i8253[channel].accessmode == PIT_MODE_HIBYTE) || 
    ((i8253[channel].accessmode == PIT_MODE_TOGGLE) && i8253[channel].MSB);

  uint16_t & update = i8253[channel].update;
  if (lobyte)
    update = (update & 0xFF00) | value; // Lower byte
  else if (hibyte)
    update = (update & 0x00FF) | ((uint16_t)value << 8); // Upper byte

  i8253[channel].active = true;

  if (i8253[channel].accessmode == PIT_MODE_TOGGLE)
    i8253[channel].MSB = !i8253[channel].MSB;

  setCounter(channel, i8253[channel].update);

  // if(channel == 2)
  //   updateFrequency(i8253[2].update);
}

static void writeControl(uint32_t address, uint8_t value)
{
  // ESP_LOGI(TAG, "%02X: %02X", static_cast<unsigned int>(address), static_cast<unsigned int>(value));
  const uint32_t channel = value >> 6;
  const uint8_t accessmode = (value >> 4) & 3;
  const uint8_t mode = (value >> 1) & 7;
  const uint16_t counter = getCounter(channel);

  i8253[channel].accessmode = accessmode;
  if(accessmode == PIT_MODE_LATCHCOUNT)
  {
    i8253[channel].latch = counter;
  }
  i8253[channel].isLatched = (accessmode == PIT_MODE_LATCHCOUNT);

  if ((accessmode == PIT_MODE_TOGGLE) || (accessmode == PIT_MODE_LATCHCOUNT))
    i8253[channel].MSB = false;
}

static uint8_t readCounter(uint32_t address)
{
  const uint32_t channel = address & 0x03;
  uint8_t & accessMode = i8253[channel].accessmode;
  uint16_t counter = getCounter(channel);

  const bool interleavedRead = (accessMode == PIT_MODE_LATCHCOUNT) || (accessMode == PIT_MODE_TOGGLE);
  const bool readMSB = (accessMode == PIT_MODE_HIBYTE) || (interleavedRead && i8253[channel].MSB);

  uint16_t & value = (i8253[channel].isLatched) ? i8253[channel].latch : counter;
  if ((accessMode == 0) || (accessMode == PIT_MODE_TOGGLE))
    i8253[channel].MSB = !i8253[channel].MSB;

  const uint8_t result = readMSB ? ((uint8_t)value) : ((uint8_t)(value >> 8));
  // LOG("Read ch%i: %02x\n", channel, result);
  return result;
}

static uint8_t readControl(uint32_t address)
{
  (void)address;
  return 0;
}

static void IRAM_ATTR ch0isr(void * p)
{
  timer_spinlock_take(TIMER_GROUP_0);
  doirq(0);
  timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_0);
  timer_group_enable_alarm_in_isr(TIMER_GROUP_0, TIMER_0);
  timer_spinlock_give(TIMER_GROUP_0);
}

static void IRAM_ATTR ch2isr(void * p)
{
  static bool spk = false;

  timer_spinlock_take(TIMER_GROUP_0);
  spk = !spk;
  Speaker_t::driveByTimer(spk);
  timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_1);
  timer_group_enable_alarm_in_isr(TIMER_GROUP_0, TIMER_1);
  timer_spinlock_give(TIMER_GROUP_0);
}
