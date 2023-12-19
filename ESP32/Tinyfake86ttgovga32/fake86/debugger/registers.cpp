#include "registers.h"
#include "io/keys.h"
#include <stdio.h>
#include <service/service.h>

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
  area.left = 0;
  area.top = 0;
  area.width = 8*(SERVICE_FONT_WIDTH + 1);
  area.height = _dbgReg__COUNT*(SERVICE_FONT_HEIGHT + 1);
  svcBar(area.left, area.top, area.height, area.width, BG_INACTIVE);
  void svcClearScreen(uint8_t color);
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

    const uint32_t foreground         = registers[i].hasChanged?FG_CHANGED:(isActive?FG_ACTIVE:FG_INACTIVE);
    const uint32_t background         = isActive?BG_ACTIVE:BG_INACTIVE;
    svcPrintTextPetite(buffer, area.left, i*(SERVICE_FONT_HEIGHT+1)+area.top, foreground, background);    
  }
}