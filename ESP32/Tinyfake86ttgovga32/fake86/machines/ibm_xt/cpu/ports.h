#ifndef _PORTS_H
#define _PORTS_H

#include <stdint.h>

typedef uint8_t(* portReader_t)(uint32_t address);
typedef void (* portWriter_t)(uint32_t address, uint8_t value);

class IOPort
{
  public:
    IOPort() : 
      left(nullptr), 
      right(nullptr), 
      leftHeight(0), 
      rightHeight(0), 
      address(0x00), 
      reader(nullptr),
      writer(nullptr),
      value(0xFF)
    {};
    IOPort(uint32_t address, uint8_t defaultValue, portReader_t reader, portWriter_t writer);
    IOPort *left;
    IOPort *right;
    int32_t leftHeight;
    int32_t rightHeight;
    uint32_t address;

    portReader_t reader;
    portWriter_t writer;
    uint8_t value;
  };

  class IOPortSpace
  {
    public:
    static IOPortSpace &getInstance();
    void insert(IOPort *newNode);
    IOPort *get(uint32_t address);
    void scan();
    uint8_t read(uint32_t address);
    void write(uint32_t address, uint8_t value);
    uint16_t read16(uint32_t address);
    void write16(uint32_t address, uint16_t value);
    void setBits(uint32_t address, uint8_t mask);
    void resetBits(uint32_t address, uint8_t mask);
  private:
    static const uint32_t MAX_PORT = 0x3FF;
    static IOPort *root;
    static IOPortSpace instance;

    void _scan(IOPort *startPoint);
};

#endif
