#include <stdio.h>
#include "memory.h"
#include "../../../host/keyboard/keys.h"
#include "../service/service.h"

#define TAG "MEM_BR"

void memBrowser_t::init(DBG_MEM_ADDR * position)
{
  area.left = 0;
  area.top = 21 * ACTUAL_FONT_HEIGHT;
  area.width = 43 * ACTUAL_FONT_WIDTH;
  area.height = 9 * ACTUAL_FONT_HEIGHT;
  
  this->position = position;
}

bool memBrowser_t::onKey(uint8_t scancode)
{
  if(box != nullptr)
  {
    bool result = box->onKey(scancode);
    if(result)
    {
      switch(box->state())
      {
        case WIDGET_STATE_CONFIRMED:
          parseAddressString(box->string);
        case WIDGET_STATE_CANCELLED:
          remove(box);
          delete box;
          box = nullptr;
          repaint();
          break;
      }
    }
    return result;
  } else {
    switch (scancode)
    {
    case KEY_CURSOR_UP:
      *position -= 8;
      repaint();
      return true;
    case KEY_CURSOR_DOWN:
      *position += 8;
      repaint();
      return true;
    case KEY_A:
      createAddressBox();
      return true;
    case KEY_PAGE_UP:
    case KEY_PAGE_DOWN:
    default:
      return false;
    }
  }
}

void memBrowser_t::repaint()
{
  if(box != nullptr)
  {
    box->repaint();
  }
  else
  {
    uint8_t background = isFocused ? BG_ACTIVE : BG_INACTIVE;
    // svcBar(area.left, area.top, area.height, area.width, background);

    const uint32_t MEM_ROW_COUNT = area.height / ACTUAL_FONT_HEIGHT;
    const uint32_t MEM_COL_COUNT = 8;
    const uint32_t ADDR_WIDTH = 9;
    const uint32_t NUMBERS_OFF = ADDR_WIDTH + 1;
    const uint32_t NUMBER_WIDTH = 3;
    const uint32_t CHARS_OFF = MEM_COL_COUNT * NUMBER_WIDTH + NUMBERS_OFF;
    const uint8_t FG_ADDR = isFocused ? 0x7A : 0x78;

    DBG_MEM_ADDR _pos = *position;
    for (uint32_t row = 0; row < MEM_ROW_COUNT; row++)
    {
      static const uint32_t LENGTH = 16;
      char buffer[LENGTH];
      snprintf(buffer, LENGTH, "%04X:%04X ", _pos.segment, _pos.offset);
      svcPrintText(buffer, area.left, area.top + row * ACTUAL_FONT_HEIGHT, FG_ADDR, background, 0);
      for(uint32_t col=0; col<MEM_COL_COUNT; col++)
      {
        const char value = read86(_pos.linear());
        svcPrintChar(value, area.left + (CHARS_OFF + col) * ACTUAL_FONT_WIDTH, area.top + row * ACTUAL_FONT_HEIGHT, FG_INACTIVE, background, 0);
        snprintf(buffer, LENGTH, "%02X ", value);
        svcPrintText(buffer, (NUMBERS_OFF + col * 3) * ACTUAL_FONT_WIDTH, area.top + row * ACTUAL_FONT_HEIGHT, FG_MEM_CONTENT, background, 0);
        _pos.inc();
      }
    }
  }
}

void memBrowser_t::createAddressBox()
{
  if(box == nullptr)
  {
    box = new InputBox_t(this);
    box->setArea(rect_t(0, 0, 72, 8));
    add(box);
  }
}

void memBrowser_t::parseAddressString(char * string)
{
  char * seg = nullptr;
  char * off = nullptr;
  char * delimiter = strstr(box->string, ":");
  seg = (delimiter != nullptr) ? box->string : nullptr;
  off = (delimiter != nullptr) ? (delimiter + 1) : (box->string);
  if(delimiter != nullptr) *delimiter = '\0';

  uint32_t segment = position->segment;
  uint32_t offset = position->offset;
  if(seg != nullptr)
  {
    if(strcmp(seg, "CS") == 0)
      segment = _dbgGetRegister(_dbgReg_CS);
    else if(strcmp(seg, "DS") == 0)
      segment = _dbgGetRegister(_dbgReg_DS);
    else if(strcmp(seg, "SS") == 0)
      segment = _dbgGetRegister(_dbgReg_SS);
    else if(strcmp(seg, "ES") == 0)
      segment = _dbgGetRegister(_dbgReg_ES);
    else
    {
      uint32_t val;
      uint32_t r = sscanf(seg, "%X", &val);
      if(r == 1)
        segment = val;
      else
        return;
    }
  }

  if(strcmp(off, "SP") == 0)
    offset = _dbgGetRegister(_dbgReg_SP);
  else if(strcmp(off, "BP") == 0)
    offset = _dbgGetRegister(_dbgReg_BP);
  else if(strcmp(off, "SI") == 0)
    offset = _dbgGetRegister(_dbgReg_SI);
  else if(strcmp(off, "DI") == 0)
    offset = _dbgGetRegister(_dbgReg_DI);
  else
  {
    uint32_t val;
    uint32_t r = sscanf(off, "%X", &val);
    if(r == 1)
      offset = val;
    else
      return;
  }

  position->segment = segment;
  position->offset = offset & 0xFFF0;
}