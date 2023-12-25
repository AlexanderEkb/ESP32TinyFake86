#include "code.h"
#include "io/keys.h"
#include <service/service.h>
#include <stdio.h>
#include <string.h>

static disassembler_t dasm;

void codeBrowser_t::init(DBG_MEM_ADDR * position)
{
  area.left   = 8 * ACTUAL_FONT_WIDTH;
  area.top    = 1 * ACTUAL_FONT_WIDTH;
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
  DBG_MEM_ADDR addr = *position;
  const uint32_t lines = (area.height / ACTUAL_FONT_HEIGHT);
  for(uint32_t i=0; i<lines; i++) 
  {
    line_t line;
    addr = dasm.decode(addr, &line);
    printColored(&line, i);
  }
}

void codeBrowser_t::printColored(line_t *line, uint32_t pos)
{
  const uint8_t FG_DEFAULT = isActive ? FG_ACTIVE : FG_INACTIVE;
  const uint8_t FG_ADDR = isActive ? 0x0A : 0x08;
  const uint8_t FG_MNEMONIC = isActive ? 0x7A : 0x78;
  const uint8_t FG_ARGUMENT = isActive ? 0xAA : 0xA8;
  const uint8_t FG_OTHER = isActive ? 0x7A : 0x78;
  const uint8_t BG = isActive ? BG_ACTIVE : BG_INACTIVE;
  const uint32_t ROW = area.top + pos * ACTUAL_FONT_HEIGHT;
  const uint32_t SEG_COL = area.left + 1 * ACTUAL_FONT_WIDTH;
  const uint32_t SEMICOLON_COL = area.left + 5 * ACTUAL_FONT_WIDTH;
  const uint32_t OFF_COL = area.left + 6 * ACTUAL_FONT_WIDTH;
  const uint32_t MNEMONIC_COL = area.left + 11 * ACTUAL_FONT_WIDTH;

  svcPrintText("\xB3", area.left, ROW, FG_DEFAULT, BG, 0);

  char _buf[40];
  sprintf(_buf, "%04X", line->addr.segment);
  svcPrintText(_buf, SEG_COL, ROW, FG_ADDR, BG, 0);
  svcPrintText(":", SEMICOLON_COL, ROW, FG_OTHER, BG, 0);
  sprintf(_buf, "%04X", line->addr.offset);
  svcPrintText(_buf, OFF_COL, ROW, FG_ADDR, BG, 0);

  bool mnemo = true;
  char * c = line->buffer;
  uint32_t col = MNEMONIC_COL;
  uint8_t fg = FG_DEFAULT;
  uint8_t bg = BG;
  while(*c)
  {
    if(*c == ' ') mnemo = false;
    if(mnemo)
    {
      fg = FG_MNEMONIC;
    }
    else
    {
      const bool punct = (strchr("[]:,.+- ", *c) != nullptr);
      fg = punct?FG_OTHER:FG_ARGUMENT;
    }
    svcPrintChar(*c, col, ROW, fg, bg, 0);
    c++;
    col += ACTUAL_FONT_WIDTH;
  }
  const uint32_t _width = 336-col;
  svcBar(col, ROW, ACTUAL_FONT_HEIGHT, _width, BG);
}