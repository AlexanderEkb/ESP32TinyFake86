#include "io/keys.h"
#include "memory.h"
#include <service/service.h>
#include <stdio.h>

void memBrowser_t::init(DBG_MEM_ADDR * position)
{
  area.left = 0;
  area.top = 15 * FONT_HEIGHT;
  area.width = 43 * FONT_WIDTH;
  area.height = 9 * FONT_HEIGHT;
  
  this->position = position;
}

void memBrowser_t::onKey(uint8_t scancode)
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

void memBrowser_t::refresh()
{
  repaint();
}

void memBrowser_t::repaint()
{
  uint8_t background = isActive ? BG_ACTIVE : BG_INACTIVE;
  svcBar(area.left, area.top, area.height, area.width, background);

  const uint32_t MEM_ROW_COUNT = area.height / FONT_HEIGHT;
  const uint32_t MEM_COL_COUNT = 8;
  const uint32_t ADDR_WIDTH = 9;
  const uint32_t NUMBERS_OFF = ADDR_WIDTH + 1;
  const uint32_t NUMBER_WIDTH = 3;
  const uint32_t CHARS_OFF = MEM_COL_COUNT * NUMBER_WIDTH + NUMBERS_OFF;
  const uint8_t  FG_ADDR = isActive ? 0x7A : 0x78;

  DBG_MEM_ADDR * _pos = position;
  for (uint32_t row = 0; row < MEM_ROW_COUNT; row++)
  {
    static const uint32_t LENGTH = 16;
    char buffer[LENGTH];
    snprintf(buffer, LENGTH, "%04X:%04X", _pos->segment, _pos->offset);
    svcPrintText(buffer, area.left, area.top + row * ACTUAL_FONT_HEIGHT, FG_ADDR, background, 0);
    for(uint32_t col=0; col<MEM_COL_COUNT; col++)
    {
      const char value = read86(_pos->linear());
      svcPrintChar(value, area.left + (CHARS_OFF + col) * ACTUAL_FONT_WIDTH, area.top + row * ACTUAL_FONT_HEIGHT, FG_INACTIVE, background, 0);
      snprintf(buffer, LENGTH, "%02X", value);
      svcPrintText(buffer, (NUMBERS_OFF + col * 3) * ACTUAL_FONT_WIDTH, area.top + row * ACTUAL_FONT_HEIGHT, FG_MEM_CONTENT, background, 0);
      _pos->inc();
    }
  }
}