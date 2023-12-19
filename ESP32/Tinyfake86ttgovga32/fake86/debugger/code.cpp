#include "io/keys.h"
#include "code.h"
#include <service/service.h>
#include <stdio.h>

codeBrowser_t::codeBrowser_t()
{
}

void codeBrowser_t::init(DBG_MEM_ADDR &position)
{
  area.left   = 8 * ACTUAL_FONT_WIDTH;
  area.top    = 0;
  area.width  = 48 * ACTUAL_FONT_WIDTH;
  area.height = 14 * ACTUAL_FONT_HEIGHT;
  svcBar(area.left, area.top, area.height, area.width, BG_INACTIVE);

  this->position = position;
}

void codeBrowser_t::onKey(uint8_t scancode)
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

void codeBrowser_t::refresh()
{
  repaint();
}

void codeBrowser_t::repaint()
{
}