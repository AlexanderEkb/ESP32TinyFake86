#include "registers.h"
#include "io/keys.h"
#include <stdio.h>
#include <service/screen.h>

regBrowser_t::regBrowser_t()
{
  const uint32_t COUNT = static_cast<uint32_t>(_dbgReg__COUNT);
  for(uint32_t i=0; i<COUNT; i++)
  {
    registers[i].value = 0; //_dbgGetRegister(static_cast<_dbgReg_t>(i));
    registers[i].hasChanged = false;
  }
}

void regBrowser_t::init(rect_t _area)
{
  area = _area;
  // Clear screen area
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
  const uint32_t col = 0;
  for(uint32_t i=0; i<_dbgReg__COUNT; i++)
  {
    const uint32_t LENGTH = 16;
    char buffer[LENGTH];
    snprintf(buffer, LENGTH, "%s: %04X", regNames[i], registers[i].value);

    static const uint32_t FG_ACTIVE   = 0x0F;
    static const uint32_t FG_CHANGED  = 0x48;
    static const uint32_t FG_INACTIVE = 0x0C;
    static const uint32_t BG_ACTIVE   = 0x00;
    static const uint32_t BG_INACTIVE = 0x04;
    const uint32_t foreground         = registers[i].hasChanged?FG_CHANGED:(isActive?FG_ACTIVE:FG_INACTIVE);
    const uint32_t background         = isActive?BG_ACTIVE:BG_INACTIVE;
    svcPrintText(buffer, area.left, i*8+area.top, foreground, background);    
  }
}