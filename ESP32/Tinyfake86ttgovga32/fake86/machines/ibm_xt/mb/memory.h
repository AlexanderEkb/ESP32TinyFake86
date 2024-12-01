#ifndef __IBM_XT_MB_MEMORY_H__
#define __IBM_XT_MB_MEMORY_H__

#include <stdint.h>

class MemoryArea_t
{
  public:
    virtual bool  belongs(uint32_t addr) = 0;
    virtual uint8_t  read(uint32_t addr) = 0;
    virtual uint16_t  readWord(uint32_t addr);
    virtual void readBulk(uint32_t addr, uint8_t * buffer, uint32_t count);
    virtual void  write(uint32_t addr, uint8_t byte) = 0;
    virtual void  writeWord(uint32_t addr, uint16_t word);
    virtual void writeBulk(uint32_t addr, uint8_t * buffer, uint32_t count);
};

class MemoryROM_t : public MemoryArea_t
{
  public:
    virtual void  write(uint32_t addr, uint8_t byte) {};
};

class BIOS_t : public MemoryROM_t
{
  public:
    static const uint32_t START_ADDR  = 0xFE000;
    static const uint32_t END_ADDR    = 0xFFFFF;
    virtual bool  belongs(uint32_t addr) override;
    virtual uint8_t  read(uint32_t addr) override;
};

class RomBasic_t : public MemoryROM_t
{
  public:
    static const uint32_t START_ADDR  = 0xF6000;
    static const uint32_t END_ADDR    = 0xFDFFF;
    virtual bool  belongs(uint32_t addr) override;
    virtual uint8_t  read(uint32_t addr) override;
};

class MemoryRAM_t : public MemoryArea_t
{
  public:
    static const uint32_t START_ADDR  = 0x00000;
    static const uint32_t END_ADDR    = 640 * 1024;
    void init();
    virtual bool  belongs(uint32_t addr) override;
    virtual uint8_t  read(uint32_t addr) override;
    virtual void  write(uint32_t addr, uint8_t byte) override;
  private:
    uint8_t * ram;
};

class VideoRAM_t : public MemoryArea_t
{
  public:
    static const uint32_t START_ADDR  = 0xB8000;
    static const uint32_t END_ADDR    = 0xBBFFF;
    void init(uint8_t * bytes);
    virtual bool  belongs(uint32_t addr) override;
    virtual uint8_t  read(uint32_t addr) override;
    virtual void  write(uint32_t addr, uint8_t byte) override;
  private:
    uint8_t * bytes;
};

class Memory_t
{
  public:
    Memory_t() :
      regions({&RAM, &video, &bios, &basic}) {};
    void init(uint8_t * videomemory);
    uint8_t read(uint32_t addr);
    uint16_t readWord(uint32_t addr);
    void readBulk(uint32_t addr, uint8_t * buffer, uint32_t count);
    void write(uint32_t addr, uint8_t byte);
    void writeWord(uint32_t addr, uint16_t word);
    void writeBulk(uint32_t addr, uint8_t * buffer, uint32_t count);
    uint32_t foo();
  private:
    BIOS_t bios;        // 0xFE000 ... 0xFFFFF
    RomBasic_t basic;   // 0xF6000 ... 0xFDFFF
    MemoryRAM_t RAM;    // 0x00000 ... RAM_SIZE
    VideoRAM_t video;   // 0xB8000 ... 0xBBFFF
    static const uint32_t MAX_REGION_COUNT = 4;
    MemoryArea_t * regions[MAX_REGION_COUNT];
};

#endif /* __IBM_XT_MB_MEMORY_H__ */