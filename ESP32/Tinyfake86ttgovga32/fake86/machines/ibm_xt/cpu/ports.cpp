//  Fake86: A portable, open-source 8086 PC emulator.
//  Copyright (C)2010-2012 Mike Chambers
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
// ports.c: functions to handle port I/O from the CPU module, as well
//   as functions for emulated hardware components to register their
//   read/write callback functions across the port address range.

#include "esp_log.h"
#include "ports.h"
#include "cpu.h"
#include "../io/speaker.h"
#include <Arduino.h>
#include <stdio.h>

#define TAG "PORTS"

IOPortSpace IOPortSpace::instance;
IOPort *    IOPortSpace::root = nullptr;

IOPort::IOPort(uint32_t address, uint8_t defaultValue, portReader_t reader, portWriter_t writer)
{
  this->address = address;
  this->value   = defaultValue;
  this->reader  = reader;
  this->writer  = writer;
  this->left    = nullptr;
  this->right   = nullptr;

  IOPortSpace::getInstance().insert(this);
};

IOPortSpace & IOPortSpace::getInstance()
{
  return instance;
}

void IOPortSpace::insert(IOPort *newNode)
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

IOPort * IOPortSpace::get(uint32_t address)
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

void IOPortSpace::scan()
{
  ESP_LOGI(TAG, "IO port handled in this implementation...\n");
  _scan(root);
  ESP_LOGI(TAG, "Scan finished.\n");
}

uint8_t IOPortSpace::read(uint32_t address)
{
  uint32_t addr = address & MAX_PORT;
  // ESP_LOGI(TAG, "IN %03xh ", addr);
  IOPort *port = get(addr);
  if(port == nullptr) {
    // ESP_LOGE(TAG, "Error reading port %xh\n", addr);
    // ESP_LOGE(TAG, "(00) Err\n");
    return 0xFF;
  } else if(port->reader == nullptr) {
    // ESP_LOGI(TAG, "(%02xh)\n", port->value);
    return port->value;
  } else {
    uint8_t result = port->reader(addr);;
    // ESP_LOGI(TAG, "(%02xh)\n", result);
    return result;
  }
}

void IOPortSpace::write(uint32_t address, uint8_t value)
{
  uint32_t addr = address & MAX_PORT;
  // ESP_LOGI(TAG, "OUT %03xh, %02xh ", addr, value);
  IOPort *port = get(addr);
  if (port == nullptr)
  {
    // ESP_LOGE(TAG, "Error writing port %xh\n", addr);
    // ESP_LOGE(TAG, "Err\n");
    return;
  }
  port->value = value;
  if (port->writer != nullptr)
    port->writer(addr, value);
    // ESP_LOGI(TAG, "\n");
}

uint16_t IOPortSpace::read16(uint32_t address)
{
  uint16_t LSB = (uint16_t)(read(address));
  uint16_t MSB = (uint16_t)(read(address + 1)) << 16;
  uint16_t result = MSB | LSB;
  // ESP_LOGI(TAG, "IN16 %03xh (%x04xh)\n", address, result);
  return result;
}

void IOPortSpace::write16(uint32_t address, uint16_t value)
{
  // ESP_LOGI(TAG, "OUT16:\n");
  write(address, (uint8_t)value);
  write(address + 1, (uint8_t)(value >> 8));
}

void IOPortSpace::setBits(uint32_t address, uint8_t mask)
{
  uint8_t val = read(address);
  val |= mask;
  write(address, val);
}

void IOPortSpace::resetBits(uint32_t address, uint8_t mask)
{
  uint8_t val = read(address);
  val &= ~mask;
  write(address, val);
}

void IOPortSpace::_scan(IOPort *startPoint)
{
  if(startPoint != nullptr)
  {
    ESP_LOGI(TAG, "Port %03xh\n", startPoint->address);
    _scan(startPoint->right);
    _scan(startPoint->left);
  }
}
