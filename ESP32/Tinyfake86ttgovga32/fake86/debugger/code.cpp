#include "code.h"
#include "keyboard/keys.h"
#include <service/service.h>
#include <stdio.h>
#include <string.h>
#include <esp_log.h>

#define TAG "CODE"

static disassembler_t dasm;

void codeBrowser_t::init(DBG_MEM_ADDR * position)
{
  svcBar(area.left, area.top, area.height, area.width, BG_INACTIVE);

  this->position = position;
}

bool codeBrowser_t::onKey(uint8_t scancode)
{
  switch (scancode)
  {
  case KEY_CURSOR_UP:
    prevInstruction();
    return true;
  case KEY_CURSOR_DOWN:
    nextInstruction();
    return true;

  default:
    return false;
  }
}

void codeBrowser_t::nextInstruction()
{
  line_t line;
  DBG_MEM_ADDR next = dasm.decode(position, &line);
  position->segment = next.segment;
  position->offset = next.offset;
}

void codeBrowser_t::prevInstruction()
{
  // ESP_LOGI(TAG, "hi!");
  if(position->offset == 0)
    return;
  DBG_MEM_ADDR a = DBG_MEM_ADDR(position);
  static const uint32_t EURISTIC_DEPTH = 20;
  const uint32_t BACKSTEP = (a.offset > EURISTIC_DEPTH) ? EURISTIC_DEPTH : a.offset;
  // ESP_LOGI(TAG, "Backstep is %lu", BACKSTEP);
  a.offset -= BACKSTEP;
  DBG_MEM_ADDR next;
  while(next.offset < position->offset)
  {
    line_t line;
    line.s.clear();
    next = dasm.decode(a, &line);
    // ESP_LOGI(TAG, "%04X %s", a.offset, line.s.c_str());
    if(next == *position)
    {
      // ESP_LOGI(TAG, "match!");
      position->offset = a.offset;
      return;
    }
    a.offset = next.offset;
  }
  // ESP_LOGI(TAG, "no match...");
  position->dec();
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
  const uint8_t FG_DEFAULT = isFocused ? FG_ACTIVE : FG_INACTIVE;
  const uint8_t FG_ADDR = isFocused ? 0x0A : 0x08;
  const uint8_t FG_MNEMONIC = isFocused ? 0x7A : 0x78;
  const uint8_t FG_ARGUMENT = isFocused ? 0xAA : 0xA8;
  const uint8_t FG_OTHER = isFocused ? 0x7A : 0x78;
  const bool isCurrentPos = (line->addr == DBG_MEM_ADDR(_dbgGetRegister(_dbgReg_CS), _dbgGetRegister(_dbgReg_IP)));
  const uint8_t BG = isCurrentPos ? BG_CSIP : (isFocused ? BG_ACTIVE : BG_INACTIVE);
  const uint32_t ROW = area.top + pos * ACTUAL_FONT_HEIGHT;
  const uint32_t SEG_COL        = area.left + 0 * ACTUAL_FONT_WIDTH;
  const uint32_t SEMICOLON_COL  = area.left + 4 * ACTUAL_FONT_WIDTH;
  const uint32_t OFF_COL        = area.left + 5 * ACTUAL_FONT_WIDTH;
  const uint32_t MNEMONIC_COL   = area.left + 10 * ACTUAL_FONT_WIDTH;
  //  const uint32_t OFF_COL        = area.left + 0 * ACTUAL_FONT_WIDTH;
  //  const uint32_t MNEMONIC_COL   = area.left + 5 * ACTUAL_FONT_WIDTH;

  char _buf[40];
  sprintf(_buf, "%04X", line->addr.segment);
  svcPrintText(_buf, SEG_COL, ROW, FG_ADDR, BG, 0);
  svcPrintChar(':', SEMICOLON_COL, ROW, FG_ADDR, BG, 0);
  sprintf(_buf, "%04X", line->addr.offset);
  svcPrintText(_buf, OFF_COL, ROW, FG_ADDR, BG, 0);

  bool mnemo = true;
  char const * c = line->s.data();
  uint32_t col = MNEMONIC_COL;
  uint8_t fg = FG_DEFAULT;
  uint8_t bg = BG;
  while(*c)
  {
    if(*c == ' ') mnemo = false;
    if(mnemo) {
      fg = FG_MNEMONIC;
    } else {
      const bool punct = (strchr("[]:,.+- ", *c) != nullptr);
      fg = punct?FG_OTHER:FG_ARGUMENT;
    }
    svcPrintChar(*c, col, ROW, fg, bg, 0);
    c++;
    col += ACTUAL_FONT_WIDTH;
  }
  const uint32_t _width = area.width - col + 1;
  svcBar(col, ROW, ACTUAL_FONT_HEIGHT, _width, BG);
}