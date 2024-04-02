#include "i8255.h"
#include "cpu/ports.h"
#include "io/covox.h"

// static const uint8_t SW1 = 0b10010010;
//                           ┌──────── ⌠ Total diskette
//                           │┌─────── ⌡ drives
//                           ││
//                           ││┌────── ⌠ Active
//                           │││┌───── ⌡ video
//                           ││││
//                           ││││┌──── ⌠ Amount
//                           │││││┌─── ⌡ of RAM
//                           ││││││┌── numeric coprpcessor present
//                           │││││││┌─ diskette drives are present
//                           ││││││││
static const uint8_t SW1 = 0b01101101;
static const uint32_t PB2_HIGH_SWITCHES = 0x08;

extern bool speakerDrivenByTimer;

static uint8_t onPort0x60Read(uint32_t addrress);
static uint8_t onPort0x62Read(uint32_t addrress);
static void onPort0x61Write(uint32_t address, uint8_t val);

IOPort port_060h = IOPort(0x60, 0x00, onPort0x60Read, nullptr);
// IOPort port_060h = IOPort(0x60, 0x00, nullptr, nullptr);
IOPort port_061h = IOPort(0x61, 0xFF, nullptr, onPort0x61Write);
IOPort port_062h = IOPort(0x62, 0x00, onPort0x62Read, nullptr);
IOPort port_063h = IOPort(0x63, 0x00, nullptr, nullptr);

static uint8_t &PA = port_060h.value;
static uint8_t &PB = port_061h.value;
static uint8_t &PC = port_062h.value;

static uint8_t onPort0x60Read(uint32_t addrress)
{
  (void)addrress;
  // LOG("Reading 60h: %s = %02X\n", showSwitches?"SW1":"scancode", result);
  return PA;
}

void onPort0x61Write(uint32_t address, uint8_t val)
{
  (void)address;

  static const uint8_t GATE_TIM_CH2_TO_SPEAKER = 0x01;
  static const uint8_t ENABLE_SPEAKER = 0x02;

  static const uint8_t TIMER_DRIVEN = (GATE_TIM_CH2_TO_SPEAKER | ENABLE_SPEAKER);
  speakerDrivenByTimer = ((val & TIMER_DRIVEN) == TIMER_DRIVEN);
  if (!speakerDrivenByTimer)
  {
    uint8_t level = (val & ENABLE_SPEAKER) ? HIGH : LOW;
    Covox_t::getInstance().driveSpeaker(level);
  }
}

static uint8_t onPort0x62Read(uint32_t addrress)
{
  (void)addrress;
  uint8_t result = 0;

  const bool high = ((PB & PB2_HIGH_SWITCHES) == PB2_HIGH_SWITCHES);
  uint8_t switches = (SW1 >> (high ? 4 : 0)) & 0x0F;

  // TODO: add missing PC lines, at least PC4 (speaker feedback)
  result = switches;
  return result;
}
