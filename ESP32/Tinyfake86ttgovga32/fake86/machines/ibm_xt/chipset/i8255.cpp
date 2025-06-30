#include "../sound/speaker.h"
#include "i8255.h"
#include "i8253.h"
#include "../cpu/ports.h"

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

/**
 * @brief 
 * 
 * @param address 
 * @param val 
 * 
 *       ╓7┬6┬5┬4┬3┬2┬1┬0╖
 *       ║ │ │ │ │ │0│ │ ║
 *       ╙╥┴╥┴╥┴╥┴╥┴─┴╥┴╥╜ bit mask
 *        ║ ║ ║ ║ ║   ║ ╚═► 0: 01H gate timer channel 2 to speaker
 *        ║ ║ ║ ║ ║   ╚═══► 1: 02H pulse speaker 1=out, 0=in
 *        ║ ║ ║ ║ ║            (see Speaker Control for examples)
 *        ║ ║ ║ ║ ╚═══════► 3: 04H 1=read high switches;
 *        ║ ║ ║ ║                  0=read low ones (see 62H)
 *        ║ ║ ║ ╚═════════► 4: 10H 0=enable RAM parity checking; 1=disable
 *        ║ ║ ╚═══════════► 5: 20H 0=enable I/O channel check
 *        ║ ╚═════════════► 6: 40H 0=hold keyboard clock low
 *        ╚═══════════════► 7: 80H 0=enable keyboard; 1=disable keyboard
 */
void onPort0x61Write(uint32_t address, uint8_t val)
{
  (void)address;

  static const uint8_t GATE_CH2 = 0x01;
  static const uint8_t DRIVE_SPEAKER = 0x02;

  const bool gate = (val & GATE_CH2) != 0;
  i8253_gateCh2(gate);
  Speaker_t::gateCh2(gate);
  Speaker_t::driveDirectly((val & DRIVE_SPEAKER) != 0);
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
