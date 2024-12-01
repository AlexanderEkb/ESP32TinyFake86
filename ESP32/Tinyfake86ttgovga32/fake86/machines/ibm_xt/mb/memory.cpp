#include <freertos/portmacro.h>
#include <esp32-hal-log.h>
#include "memory.h"
#include "../dataFlash/bios/biospcxt.inc"
#include "../dataFlash/rom/rombasic.inc"

#define TAG "MEM"
// =============================================================[ MemoryArea_t ]
uint16_t  MemoryArea_t::readWord(uint32_t addr)
{
  return ((uint16_t)read(addr) | ((uint16_t)read(addr + 1) << 8 ));
}

void MemoryArea_t::readBulk(uint32_t addr, uint8_t * buffer, uint32_t count)
{
  for(uint32_t i=0; i<count; i++)
  {
    buffer[i] = read(addr + i);
  }
}

void  MemoryArea_t::writeWord(uint32_t addr, uint16_t word)
{
  write(addr, (uint8_t)word);
  write(addr + 1, (uint8_t)(word >> 8));
}

void MemoryArea_t::writeBulk(uint32_t addr, uint8_t * buffer, uint32_t count)
{
  for(uint32_t i=0; i<count; i++)
  {
    write(addr + i, buffer[i]);
  }
}

// ===================================================================[ BIOS_t ]
bool BIOS_t::belongs(uint32_t addr)
{
  return ((addr >= START_ADDR) && (addr <= END_ADDR));
}

uint8_t BIOS_t::read(uint32_t addr)
{
  return bios_pcxt[addr - START_ADDR];
}

// ===============================================================[ RomBasic_t ]
bool RomBasic_t::belongs(uint32_t addr)
{
  return ((addr >= START_ADDR) && (addr <= END_ADDR));
}

uint8_t RomBasic_t::read(uint32_t addr)
{
  return rom_basic[addr - START_ADDR];
}

// ==============================================================[ MemoryRam_t ]
void MemoryRAM_t::init()
{
  const uint32_t coreID = xPortGetCoreID();
  const uint32_t ramAddr = SOC_EXTRAM_DATA_LOW + (coreID == 1 ? 2 * 1024 * 1024 : 0);
  ram = reinterpret_cast<uint8_t *>(ramAddr);
  ESP_LOGI(TAG, "RAM initialized: core #%i, addr:0x%08X", coreID, ramAddr);
}

bool MemoryRAM_t::belongs(uint32_t addr)
{
  return ((addr >= START_ADDR) && (addr <= END_ADDR));
}

uint8_t MemoryRAM_t::read(uint32_t addr)
{
  return ram[addr - START_ADDR];
}

void  MemoryRAM_t::write(uint32_t addr, uint8_t byte)
{
  ram[addr - START_ADDR] = byte;
}

// ===============================================================[ VideoRam_t ]
void VideoRAM_t::init(uint8_t * bytes)
{
  this->bytes = bytes;
}

bool VideoRAM_t::belongs(uint32_t addr)
{
  return ((addr >= START_ADDR) && (addr <= END_ADDR));
}

uint8_t VideoRAM_t::read(uint32_t addr)
{
  // ESP_LOGI(TAG, "%05Xh", addr);
  return bytes[addr - START_ADDR];
}

void  VideoRAM_t::write(uint32_t addr, uint8_t byte)
{
  // ESP_LOGI(TAG, "%05Xh", addr);
  bytes[addr - START_ADDR] = byte;
}

// =================================================================[ Memory_t ]
void Memory_t::init(uint8_t * videomemory)
{
  ESP_LOGW(TAG, "Memory init");
  ESP_LOGI(TAG, "RAM init");
  RAM.init();
  ESP_LOGI(TAG, "Video memory init");
  video.init(videomemory);
}

uint8_t Memory_t::read(uint32_t addr)
{
  // ESP_LOGI(TAG, "read from %05X", addr);
  for(uint32_t i=0; i<MAX_REGION_COUNT; i++)
  {
    if(regions[i]->belongs(addr))
    {
      return regions[i]->read(addr);
    }
  }
  return 0xFF;
}

uint16_t Memory_t::readWord(uint32_t addr)
{
  // ESP_LOGI(TAG, "readWord from %05X", addr);
  for(uint32_t i=0; i<MAX_REGION_COUNT; i++)
  {
    if(regions[i]->belongs(addr))
    {
      return regions[i]->readWord(addr);
    }
  }
  return 0xFFFF;
}

void Memory_t::readBulk(uint32_t addr, uint8_t * buffer, uint32_t count)
{
  // ESP_LOGI(TAG, "readBulk %X bytes  from %05X", count, addr);
  for(uint32_t i=0; i<MAX_REGION_COUNT; i++)
  {
    if(regions[i]->belongs(addr))
    {
      regions[i]->readBulk(addr, buffer, count);
      return;
    }
  }
}

void Memory_t::write(uint32_t addr, uint8_t byte)
{
  // ESP_LOGI(TAG, "write %02Xh to %05Xh", byte, addr);
  for(uint32_t i=0; i<MAX_REGION_COUNT; i++)
  {
    if(regions[i]->belongs(addr))
    {
      regions[i]->write(addr, byte);
      return;
    }
  }
}

void Memory_t::writeWord(uint32_t addr, uint16_t word)
{
  // ESP_LOGI(TAG, "write %04Xh to %05Xh", word, addr);
  for(uint32_t i=0; i<MAX_REGION_COUNT; i++)
  {
    if(regions[i]->belongs(addr))
    {
      regions[i]->writeWord(addr, word);
      return;
    }
  }
}

void Memory_t::writeBulk(uint32_t addr, uint8_t * buffer, uint32_t count)
{
  // ESP_LOGI(TAG, "writeBulk %X bytes to %05X", count, addr);
  for(uint32_t i=0; i<MAX_REGION_COUNT; i++)
  {
    if(regions[i]->belongs(addr))
    {
      regions[i]->writeBulk(addr, buffer, count);
      return;
    }
  }
  ESP_LOGE(TAG, "Unhandled write: %05Xh", addr);
}
