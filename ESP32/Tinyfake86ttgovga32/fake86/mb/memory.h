#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdint.h>
#include <esp_attr.h>

class MemoryArea_t
{
  public:
    MemoryArea_t(uint32_t start, uint32_t end, uint8_t * mem) :
      start(start),
      end(end),
      mem(mem) {};
    bool IRAM_ATTR belongs(uint32_t addr);
    uint8_t IRAM_ATTR read(uint32_t addr);
    uint16_t IRAM_ATTR readWord(uint32_t addr);
    void readBulk(uint32_t addr, uint8_t * buffer, uint32_t count);
    virtual void IRAM_ATTR write(uint32_t addr, uint8_t byte) = 0;
    virtual void IRAM_ATTR writeWord(uint32_t addr, uint16_t word) = 0;
    virtual void writeBulk(uint32_t addr, uint8_t * buffer, uint32_t count) = 0;
  protected:
    uint32_t start;
    uint32_t end;
    uint8_t * mem;
};

class MemoryRAM_t : public MemoryArea_t
{
  public:
    virtual void IRAM_ATTR write(uint32_t addr, uint8_t byte) override;
    virtual void IRAM_ATTR writeWord(uint32_t addr, uint16_t word) override;
    virtual void writeBulk(uint32_t addr, uint8_t * buffer, uint32_t count) override;
};

class MemoryROM_t : public MemoryArea_t
{
  public:
    virtual void IRAM_ATTR write(uint32_t addr, uint8_t byte) {return;};
    virtual void IRAM_ATTR writeWord(uint32_t addr, uint16_t word) {};
    virtual void writeBulk(uint32_t addr, uint8_t * buffer, uint32_t count) {};
  private:
};

class BIOS_t : public MemoryROM_t
{

};

class RomBasic_t : public MemoryROM_t
{

};

class Memory_t
{
  public:
    Memory_t() {};
    void init();
    uint8_t read(uint32_t addr);
    uint16_t readWord(uint32_t addr);
    void readBulk(uint32_t addr, uint8_t * buffer, uint32_t count);
    void write(uint32_t addr, uint8_t byte);
    void writeWord(uint32_t addr, uint16_t word);
    void writeBulk(uint32_t addr, uint8_t * buffer, uint32_t count);
  private:
    static const uint32_t MAX_REGION_COUNT = 4;
    MemoryArea_t * regions[MAX_REGION_COUNT];
};

#endif /* __MEMORY_H__ */