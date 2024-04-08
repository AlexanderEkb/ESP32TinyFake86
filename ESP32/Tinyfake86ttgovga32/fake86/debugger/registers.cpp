#include "registers.h"
#include "io/keys.h"
#include <Arduino.h>
#include <service/service.h>
#include <stdio.h>
/*
 * Active/inactive 
 * Selected/unselected
 * Changed/unchanged
 */

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

bool regBrowser_t::onKey(uint8_t scancode)
{
  switch (scancode)
  {
  case KEY_CURSOR_UP:     // Scroll up
    return true;
  case KEY_CURSOR_DOWN:   // Scroll down
    return true;
  case KEY_ENTER:         // Change value
    return true;
  default:
    return false;
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
  static const uint32_t FG_ACTIVE = 0x0F;
  static const uint32_t FG_CHANGED = 0x16;
  static const uint32_t FG_INACTIVE = 0x88;
  static const uint32_t BG_ACTIVE = 0x70;
  static const uint32_t BG_INACTIVE = 0x00;

  uint8_t background = focused ? BG_ACTIVE : BG_INACTIVE;
  svcBar(area.left, area.top, area.height, area.width, background);
  const uint32_t col = 0;
  for(uint32_t i=0; i<_dbgReg__COUNT; i++)
  {
    const uint32_t LENGTH = 16;
    char buffer[LENGTH];
    snprintf(buffer, LENGTH, "%s: %04X", regNames[i], registers[i].value);

    const uint32_t foreground = registers[i].hasChanged ? FG_CHANGED : (focused ? FG_ACTIVE : FG_INACTIVE);
    svcPrintText(buffer, area.left, (i+1)*(ACTUAL_FONT_HEIGHT)+area.top, foreground, background, 0);
    registers[i].hasChanged           = false;
  }
}