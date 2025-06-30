#include "inputbox.h"
#include "service.h"
#include "../../../host/keyboard/keys.h"

#define TAG "INBOX"

InputBox_t::InputBox_t(widget_t * p)
{
  parent = p;
  string = reinterpret_cast<char *>(malloc(MAX_LENGTH));
  assert(string);
  memset(string, 0, MAX_LENGTH);
  ptr = 0;
  modalState = WIDGET_STATE_ACTIVE;
  children.clear();
}

InputBox_t::~InputBox_t()
{
  free(string);
}

WidgetState_t InputBox_t::state()
{
  return modalState;
}

void InputBox_t::repaint()
{
  svcBar(area.left, area.top, area.height, area.width, 0x07);
  const uint32_t textWidth = ACTUAL_FONT_WIDTH * strlen(string);
  if(textWidth <= area.width)
  {
    svcPrintText(string, area.left, area.top, 0xFF, 0x07, 0);
  }
  else
  {
    char * s = string;
    while((ACTUAL_FONT_WIDTH * strlen(s)) > area.width) s++;
    const uint32_t position = area.left + area.width - (ACTUAL_FONT_WIDTH * strlen(s));
    svcPrintText(s, position, area.top, 0xFF, 0x07, 0);
  }
}

bool InputBox_t::onKey(uint8_t scancode)
{
  switch(scancode)
  {
    case KEY_ESC: modalState = WIDGET_STATE_CANCELLED; return true;
    case KEY_ENTER: modalState = WIDGET_STATE_CONFIRMED; return true;
    case KEY_BACKSPACE: backspace(); return true;

    case KEY_0: put('0'); return true;
    case KEY_1: put('1'); return true;
    case KEY_2: put('2'); return true;
    case KEY_3: put('3'); return true;
    case KEY_4: put('4'); return true;
    case KEY_5: put('5'); return true;
    case KEY_6: put('6'); return true;
    case KEY_7: put('7'); return true;
    case KEY_8: put('8'); return true;
    case KEY_9: put('9'); return true;

    case KEY_Q: put('Q'); return true;
    case KEY_W: put('W'); return true;
    case KEY_E: put('E'); return true;
    case KEY_R: put('R'); return true;
    case KEY_T: put('T'); return true;
    case KEY_Y: put('Y'); return true;
    case KEY_U: put('U'); return true;
    case KEY_I: put('I'); return true;
    case KEY_O: put('O'); return true;
    case KEY_P: put('P'); return true;
    case KEY_A: put('A'); return true;
    case KEY_S: put('S'); return true;
    case KEY_D: put('D'); return true;
    case KEY_F: put('F'); return true;
    case KEY_G: put('G'); return true;
    case KEY_H: put('H'); return true;
    case KEY_J: put('J'); return true;
    case KEY_K: put('K'); return true;
    case KEY_L: put('L'); return true;
    case KEY_Z: put('Z'); return true;
    case KEY_X: put('X'); return true;
    case KEY_C: put('C'); return true;
    case KEY_V: put('V'); return true;
    case KEY_B: put('B'); return true;
    case KEY_N: put('N'); return true;
    case KEY_M: put('M'); return true;

    case KEY_SEMICOLON: put(':'); return true;
  }
  return false;
}

void InputBox_t::put(char c)
{
  if(ptr < MAX_LENGTH)
  {
    string[ptr++] = c;
  }
}

void InputBox_t::backspace()
{
  if(ptr > 0)
    string[--ptr] = '\0';
}
