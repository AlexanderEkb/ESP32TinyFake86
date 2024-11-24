#include <string.h>
#include "memory.h"
#include "../dataFlash/bios/biospcxt.h"
#include "../dataFlash/rom/rombasic.h"

// TODO: Move to some kind of machine_config.h ASAP
static const size_t      RAM_SIZE  = 640 * 1024;

void Memory_t::init()
{
  // const uint32_t coreID = xPortGetCoreID();
  // const uint32_t ramAddr = SOC_EXTRAM_DATA_LOW + (coreID == 1 ? 2 * 1024 * 1024 : 0);
  // ram = reinterpret_cast<uint8_t *>(ramAddr);
  // ESP_LOGI(TAG, "RAM initialized: core #%i, addr:0x%08X", coreID, ramAddr);
  // return true; // We allocate RAM statically, so it is always successful.
}

uint8_t Memory_t::read(uint32_t addr)
{
  for(uint32_t i=0; i<MAX_REGION_COUNT; i++)
  {
    if(regions[i]->belongs(addr))
    {
      return regions[i]->read(addr);
    }
  }
}

uint16_t Memory_t::readWord(uint32_t addr)
{
  for(uint32_t i=0; i<MAX_REGION_COUNT; i++)
  {
    if(regions[i]->belongs(addr))
    {
      return regions[i]->readWord(addr);
    }
  }
}

void Memory_t::readBulk(uint32_t addr, uint8_t * buffer, uint32_t count)
{
  for(uint32_t i=0; i<MAX_REGION_COUNT; i++)
  {
    if(regions[i]->belongs(addr))
    {
      return regions[i]->readBulk(addr, buffer, count);
    }
  }
}

void Memory_t::write(uint32_t addr, uint8_t byte)
{
  for(uint32_t i=0; i<MAX_REGION_COUNT; i++)
  {
    if(regions[i]->belongs(addr))
    {
      regions[i]->write(addr, byte);
    }
  }
}

void Memory_t::writeWord(uint32_t addr, uint16_t word)
{
  for(uint32_t i=0; i<MAX_REGION_COUNT; i++)
  {
    if(regions[i]->belongs(addr))
    {
      regions[i]->writeWord(addr, word);
    }
  }
}

void Memory_t::writeBulk(uint32_t addr, uint8_t * buffer, uint32_t count)
{
  for(uint32_t i=0; i<MAX_REGION_COUNT; i++)
  {
    if(regions[i]->belongs(addr))
    {
      regions[i]->writeBulk(addr, buffer, count);
    }
  }
}

bool IRAM_ATTR MemoryArea_t::belongs(uint32_t addr)
{
  return ((addr >= start) && (addr <= end));
}

uint8_t IRAM_ATTR MemoryArea_t::read(uint32_t addr)
{
  return mem[addr - start];
}

uint16_t IRAM_ATTR MemoryArea_t::readWord(uint32_t addr)
{
  return ( (uint16_t) mem[addr - start] | (uint16_t) mem[addr - start + 1] << 8 );
}

void MemoryArea_t::readBulk(uint32_t addr, uint8_t * buffer, uint32_t count)
{
  memcpy(buffer, &mem[addr], count);
}

void IRAM_ATTR MemoryRAM_t::write(uint32_t addr, uint8_t byte)
{
  mem[addr - start] = byte;
}

void IRAM_ATTR MemoryRAM_t::writeWord(uint32_t addr, uint16_t word)
{
  mem[addr - start] = (uint8_t)(word & 0x00FF);
  mem[addr - start + 1] = (uint8_t)((word >> 8) & 0x00FF);
}

void MemoryRAM_t::writeBulk(uint32_t addr, uint8_t * buffer, uint32_t count)
{
  memcpy(&mem[addr], buffer, count);
}
