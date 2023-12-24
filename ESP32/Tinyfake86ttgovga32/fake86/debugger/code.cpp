#include "code.h"
#include "io/keys.h"
#include <service/service.h>
#include <stdio.h>
#include <string.h>

static disassembler_t dasm;

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
  DBG_MEM_ADDR addr = position;
  const uint32_t lines = (area.height / ACTUAL_FONT_HEIGHT);
  for(uint32_t i=0; i<lines; i++) 
  {
    line_t line;
    addr = dasm.decode(addr, &line);
    // printColored(&line, i);
    char _buf[40];
    snprintf(_buf, 40, "\xB3%04X:%04X %s", line.addr.segment, line.addr.offset, line.buffer);
    svcPrintText(_buf, area.left, area.top + i * ACTUAL_FONT_HEIGHT, FG_INACTIVE, BG_INACTIVE, 0);
  }
}

void codeBrowser_t::printColored(line_t *line, uint32_t pos)
{
  static const uint8_t FG_DEFAULT = isActive ? FG_ACTIVE : FG_INACTIVE;
  static const uint8_t FG_ADDR = isActive ? 0x0A : 0x08;
  static const uint8_t FG_MNEMONIC = isActive ? 0x7A : 0x78;
  static const uint8_t FG_ARGUMENT = isActive ? 0x7A : 0x78;
  static const uint8_t FG_OTHER = isActive ? 0xAA : 0xA8;
  static const uint8_t BG = isActive ? BG_ACTIVE : BG_INACTIVE;
  static const uint32_t ROW = area.top + pos * ACTUAL_FONT_HEIGHT;
  static const uint32_t SEG_COL = area.left + 1 * ACTUAL_FONT_WIDTH;
  static const uint32_t SEMICOLON_COL = area.left + 5 * ACTUAL_FONT_WIDTH;
  static const uint32_t OFF_COL = area.left + 6 * ACTUAL_FONT_WIDTH;
  static const uint32_t MNEMONIC_COL = area.left + 11 * ACTUAL_FONT_WIDTH;

  // svcPrintText("\xB3", area.left, ROW, FG_DEFAULT, BG, 0);
  // char _buf[40];
  // sprintf(_buf, "%04X", line->addr.segment);
  // svcPrintText(_buf, SEG_COL, ROW, FG_ADDR, BG, 0);
  // svcPrintText(":", SEMICOLON_COL, ROW, FG_OTHER, BG, 0);
  // sprintf(_buf, "%04X", line->addr.offset);
  // svcPrintText(_buf, OFF_COL, ROW, FG_ADDR, BG, 0);

  // bool mnemo = true;
  // char * c = line->buffer;
  // uint32_t col = MNEMONIC_COL;
  // uint8_t fg = FG_DEFAULT;
  // uint8_t bg = BG;
  // while(*c)
  // {
  //   if(*c == ' ') mnemo = false;
  //   if(mnemo)
  //   {
  //     fg = FG_MNEMONIC;
  //   }
  //   else
  //   {
  //     const bool punct = (strchr("[]:,. ", *c) == nullptr);
  //     fg = punct?FG_OTHER:FG_ARGUMENT;
  //   }
  //   svcPrintChar(*c, col, ROW, fg, bg, 0);
  //   c++;
  //   col++;
  // }
}