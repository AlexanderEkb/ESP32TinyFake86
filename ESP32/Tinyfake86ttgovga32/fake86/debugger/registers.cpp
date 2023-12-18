#include "registers.h"
#include <stdio.h>
#include <service/screen.h>

void regBrowser_t::init()
{

}

void regBrowser_t::onKey(uint8_t scancode)
{

}

void regBrowser_t::refresh()
{
  const uint32_t col = 0;
  for(uint32_t i=0; i<_dbgReg__COUNT; i++)
  {
    uint16_t const value = _dbgGetRegister(static_cast<_dbgReg_t>(i));
    const uint32_t LENGTH = 16;
    char buffer[LENGTH];
    snprintf(buffer, LENGTH, "%s: %04X", regNames[i], value);
    const uint32_t colors = isActive?1:0;
    svcPrintText(buffer, area.left, i*8+area.top, foreground[colors], background[colors]);    
  }
}
