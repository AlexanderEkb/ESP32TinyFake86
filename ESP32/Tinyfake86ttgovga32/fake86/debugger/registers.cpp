#include <Arduino.h>
#include <service/service.h>
#include <stdio.h>
#include "registers.h"
#include "keyboard/keys.h"
/*
 * Active/inactive 
 * Selected/unselected
 * Changed/unchanged
 */

constexpr char * regBrowser_t::regNames[static_cast<uint32_t>(_dbgReg__COUNT)] ;

regBrowser_t::regBrowser_t(widget_t * p)
{
  const uint32_t COUNT = static_cast<uint32_t>(_dbgReg__COUNT);
  for(uint32_t i=0; i<COUNT; i++)
  {
    registers[i].value = 0; //_dbgGetRegister(static_cast<_dbgReg_t>(i));
    registers[i].hasChanged = false;
  }
  selection = 0;
  state = STATE_DISPLAY;
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
  switch(state)
  {
    case STATE_DISPLAY:
      return onKeyDisplay(scancode);
    case STATE_EDIT:
      return onKeyEdit(scancode);
    default:
      return false;
  }
}

bool regBrowser_t::onKeyDisplay(uint8_t scancode)
{
  switch (scancode)
  {
  case KEY_CURSOR_UP:     // Scroll up
    if(selection == 0)
      selection = _dbgReg__COUNT - 1;
    else
      selection--;
    repaint();
    return true;
  case KEY_CURSOR_DOWN:   // Scroll down
    if(selection >= (_dbgReg__COUNT - 1))
      selection = 0;
    else
      selection++;
    repaint();
    return true;
  case KEY_ENTER:
    beginEdit();
    return true;
  default:
    return false;
  }
}

void regBrowser_t::beginEdit()
{
  state = STATE_EDIT;

  box = new InputBox_t(this);
  box->setArea(rect_t(4 * ACTUAL_FONT_WIDTH, selection * ACTUAL_FONT_HEIGHT, 4 * ACTUAL_FONT_WIDTH, 8));
  add(box);
  repaint();
}

bool regBrowser_t::onKeyEdit(uint8_t scancode)
{
  box->onKey(scancode);
  switch(box->state())
  {
    case WIDGET_STATE_CONFIRMED:
      updateRegister();
    case WIDGET_STATE_CANCELLED:
      remove(box);
      delete box;
      state = STATE_DISPLAY;
      repaint();
      break;
    case WIDGET_STATE_ACTIVE:
    default:
      break;
  }

  return true;
}

void regBrowser_t::updateRegister()
{
  const uint32_t len = strlen(box->string);
  const bool lenOk = ((len > 0) && (len <= 4));
  if(lenOk)
  {
    uint32_t val;
    uint32_t r = sscanf(box->string, "%X", &val);
    if(r == 1)    
    {
      _dbgSetRegister(static_cast<_dbgReg_t>(selection), val);
      registers[selection].value      = val;
      registers[selection].hasChanged = true;
    }
  }
}

void regBrowser_t::repaint()
{
  if(state == STATE_DISPLAY)
  {
    for(uint32_t i=0; i<_dbgReg__COUNT; i++)
    {
      const uint16_t value    = _dbgGetRegister(static_cast<_dbgReg_t>(i));
      registers[i].hasChanged = value != registers[i].value;
      registers[i].value      = value;
    }

    static const uint32_t FG_ACTIVE   = 0x0A;
    static const uint32_t FG_CHANGED  = 0xB8;
    static const uint32_t FG_INACTIVE = 0x88;
    static const uint32_t BG_ACTIVE   = 0x70;
    static const uint32_t BG_SELECTED = 0x34;
    static const uint32_t BG_INACTIVE = 0x00;

    uint8_t background = isFocused ? BG_ACTIVE : BG_INACTIVE;
    for(uint32_t i=0; i<_dbgReg__COUNT; i++)
    {
      const uint32_t LENGTH = 16;
      char buffer[LENGTH];
      snprintf(buffer, LENGTH, "%s: %04X", regNames[i], registers[i].value);

      const uint32_t foreground = registers[i].hasChanged ? FG_CHANGED : (isFocused ? FG_ACTIVE : FG_INACTIVE);
      const uint32_t background = isFocused ? ((i == selection) ? BG_SELECTED : BG_ACTIVE) : BG_INACTIVE;
      svcPrintText(buffer, area.left, i * ACTUAL_FONT_HEIGHT + area.top, foreground, background, 0);
      registers[i].hasChanged           = false;
    }
  }
  else
  {
    box->repaint();
  }
}