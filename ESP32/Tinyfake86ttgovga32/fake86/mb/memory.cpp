#include <string.h>
#include "memory.h"

Memory_t Memory_t::instance;

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

bool IRAM_ATTR MemoryRegion_t::belongs(uint32_t addr)
{
  return ((addr >= start) && (addr <= end));
}

uint8_t IRAM_ATTR MemoryRegion_t::read(uint32_t addr)
{
  return mem[addr - start];
}

uint16_t IRAM_ATTR MemoryRegion_t::readWord(uint32_t addr)
{
  return ( (uint16_t) mem[addr - start] | (uint16_t) mem[addr - start + 1] << 8 );
}

void MemoryRegion_t::readBulk(uint32_t addr, uint8_t * buffer, uint32_t count)
{
  memcpy(buffer, &mem[addr], count);
}

void IRAM_ATTR MemoryRegion_t::write(uint32_t addr, uint8_t byte)
{
  mem[addr - start] = byte;
}

void IRAM_ATTR MemoryRegion_t::writeWord(uint32_t addr, uint16_t word)
{
  mem[addr - start] = (uint8_t)(word & 0x00FF);
  mem[addr - start + 1] = (uint8_t)((word >> 8) & 0x00FF);
}

void MemoryRegion_t::writeBulk(uint32_t addr, uint8_t * buffer, uint32_t count)
{
  memcpy(&mem[addr], buffer, count);
}
