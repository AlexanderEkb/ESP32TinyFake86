#include "registers.h"
#include "io/keys.h"
#include <Arduino.h>
#include <service/service.h>
#include <stdio.h>

constexpr char * regBrowser_t::regNames[static_cast<uint32_t>(_dbgReg__COUNT)] ;

regBrowser_t::regBrowser_t()
{
  const uint32_t COUNT = static_cast<uint32_t>(_dbgReg__COUNT);
  for(uint32_t i=0; i<COUNT; i++)
  {
    registers[i].value = 0; //_dbgGetRegister(static_cast<_dbgReg_t>(i));
    registers[i].hasChanged = false;
  }
}

void regBrowser_t::init()
{
  area.left = 34 * ACTUAL_FONT_WIDTH;
  area.top = 0;
  area.width = 8 * ACTUAL_FONT_WIDTH;
  area.height = _dbgReg__COUNT * ACTUAL_FONT_HEIGHT;
}

void regBrowser_t::onKey(uint8_t scancode)
{
  switch (scancode)
  {
  case KEY_CURSOR_UP:
    break;
  case KEY_CURSOR_DOWN:
    break;
  
  default:
    break;
  }
}

void regBrowser_t::refresh()
{
  const uint32_t col = 0;
  for(uint32_t i=0; i<_dbgReg__COUNT; i++)
  {
    const uint16_t value    = _dbgGetRegister(static_cast<_dbgReg_t>(i));
    registers[i].hasChanged = value != registers[i].value;
    registers[i].value      = value;
  }
  repaint();
}

void regBrowser_t::repaint()
{
  uint8_t background = isActive ? BG_ACTIVE : BG_INACTIVE;
  svcBar(area.left, area.top, area.height, area.width, background);
  const uint32_t col = 0;
  for(uint32_t i=0; i<_dbgReg__COUNT; i++)
  {
    const uint32_t LENGTH = 16;
    char buffer[LENGTH];
    snprintf(buffer, LENGTH, "%s: %04X", regNames[i], registers[i].value);

    const uint32_t foreground         = registers[i].hasChanged?FG_CHANGED:(isActive?FG_ACTIVE:FG_INACTIVE);
    svcPrintText(buffer, area.left, (i+1)*(ACTUAL_FONT_HEIGHT)+area.top, foreground, background, 0);
    registers[i].hasChanged           = false;
  }
}