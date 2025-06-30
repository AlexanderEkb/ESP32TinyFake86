#include "../cpu/ports.h"
#include "../../../host/audio/audio.h"

static void write(uint32_t address, uint8_t value);
IOPort port_378h = IOPort(0x378, 0x00, nullptr, write);

static void write(uint32_t address, uint8_t value)
{
  (void)address;
  Audio::playSample(value);
}