#ifndef _PORTS_H
#define _PORTS_H

#include <stdint.h>
#include "gbGlobals.h"

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
    static IOPortSpace &getInstance()
    {
      return instance;
    }

    void insert(IOPort *newNode)
    {
      IOPort **node = &root;
      while (true)
      {
        IOPort *port = *node;
        if (port == nullptr)
        {
          *node = newNode;
          return;
        }
        else
        {
          if (newNode->address > port->address)
          {
            node = &port->right;
          }
          else
          {
            node = &port->left;
          }
        }
      }
    }

    IOPort *get(uint32_t address)
    {
      IOPort *pNode = root;
      while (true)
      {
        if (pNode == nullptr)
        {
          return nullptr;
        }
        else if (pNode->address == address)
        {
          return pNode;
        }
        else
        {
          if (address > pNode->address)
          {
            pNode = pNode->right;
          }
          else
          {
            pNode = pNode->left;
          }
        }
      }
    }

    void scan()
    {
      LOG("IO port handled in this implementation...\n");
      _scan(root);
      LOG("Scan finished.\n");
    }

    uint8_t read(uint32_t address)
    {
      uint32_t addr = address & MAX_PORT;
      // LOG("IN %03xh ", addr);
      IOPort *port = get(addr);
      if(port == nullptr) {
        // LOG("Error reading port %xh\n", addr);
        // LOG("(00) Err\n");
        return 0x00;
      } else if(port->reader == nullptr) {
        // LOG("(%02xh)\n", port->value);
        return port->value;
      } else {
        uint8_t result = port->reader(addr);;
        // LOG("(%02xh)\n", result);
        return result;
      }
    }

    void write(uint32_t address, uint8_t value)
    {
      uint32_t addr = address & MAX_PORT;
      // LOG("OUT %03xh, %02xh ", addr, value);
      IOPort *port = get(addr);
      if (port == nullptr)
      {
        // LOG("Error writing port %xh\n", addr);
        // LOG("Err\n");
        return;
      }
      port->value = value;
      if (port->writer != nullptr)
        port->writer(addr, value);
        // LOG("\n");
    }

    uint16_t read16(uint32_t address)
    {
      uint16_t LSB = (uint16_t)(read(address));
      uint16_t MSB = (uint16_t)(read(address + 1)) << 16;
      uint16_t result = MSB | LSB;
      // LOG("IN16 %03xh (%x04xh)\n", address, result);
      return result;
    }

    void write16(uint32_t address, uint16_t value)
    {
      // LOG("OUT16:\n");
      write(address, (uint8_t)value);
      write(address + 1, (uint8_t)(value >> 8));
    }

    void setBits(uint32_t address, uint8_t mask)
    {
      uint8_t val = read(address);
      val |= mask;
      write(address, val);
    }

    void resetBits(uint32_t address, uint8_t mask)
    {
      uint8_t val = read(address);
      val &= ~mask;
      write(address, val);
    }
  private:
    static const uint32_t MAX_PORT = 0x3FF;
    static IOPort *root;
    static IOPortSpace instance;

    void _scan(IOPort *startPoint)
    {
    if(startPoint != nullptr)
    {
      LOG("Port %03xh\n", startPoint->address);
      _scan(startPoint->right);
      _scan(startPoint->left);
    }
  }
};

#endif
