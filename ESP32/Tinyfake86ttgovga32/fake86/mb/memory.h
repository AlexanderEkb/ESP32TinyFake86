#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdint.h>
#include <esp_attr.h>

class MemoryRegion_t
{
  public:
    MemoryRegion_t(uint32_t start, uint32_t end, uint8_t * mem) :
      start(start),
      end(end),
      mem(mem) {};
    bool IRAM_ATTR belongs(uint32_t addr);
    uint8_t IRAM_ATTR read(uint32_t addr);
    uint16_t IRAM_ATTR readWord(uint32_t addr);
    virtual void readBulk(uint32_t addr, uint8_t * buffer, uint32_t count);
    virtual void IRAM_ATTR write(uint32_t addr, uint8_t byte);
    virtual void IRAM_ATTR writeWord(uint32_t addr, uint16_t word);
    virtual void writeBulk(uint32_t addr, uint8_t * buffer, uint32_t count);
  protected:
    uint32_t start;
    uint32_t end;
    uint8_t * mem;
};

class MemoryROM_t : public MemoryRegion_t
{
  public:
    virtual void IRAM_ATTR write(uint32_t addr, uint8_t byte);
    virtual void IRAM_ATTR writeWord(uint32_t addr, uint16_t word);
  private:
};

class Memory_t
{
  public:
    Memory_t & getInstance() {return instance;};
    uint8_t read(uint32_t addr);
    uint16_t readWord(uint32_t addr);
    void readBulk(uint32_t addr, uint8_t * buffer, uint32_t count);
    void write(uint32_t addr, uint8_t byte);
    void writeWord(uint32_t addr, uint16_t word);
    void writeBulk(uint32_t addr, uint8_t * buffer, uint32_t count);
  private:
    static Memory_t instance;
    static const uint32_t MAX_REGION_COUNT = 4;
    MemoryRegion_t * regions[MAX_REGION_COUNT];
    Memory_t() {};
};

#endif /* __MEMORY_H__ */