#include "debugger/debugger.h"
#include "cpu/cpu.h"
#include "io/keyboard.h"
#include "service/service.h"
#include <algorithm>

debugger_t debugger_t::instance;

debugger_t::debugger_t()
{
  add(&codeBrowser);
  add(&regBrowser);
  add(&memBrowser);
  memPosition = DBG_MEM_ADDR(0, 0);
  codePosition = DBG_MEM_ADDR(0, 0);
}

void debugger_t::nextBrowser()
{
  auto i = std::find(children.begin(), children.end(), browser);
  browser_t ** b = reinterpret_cast<browser_t **>(&i);
  i++;
  if(i == children.end())
    i = children.begin()++;
  browser = reinterpret_cast<browser_t *>(*i);
  browser->setFocus();
}

void debugger_t::doSingleStep()
{
    exec86(1);
    codePosition.segment = _dbgGetRegister(_dbgReg_CS);
    codePosition.offset = _dbgGetRegister(_dbgReg_IP);
    memPosition.segment = _dbgGetRegister(_dbgReg_CS);
    memPosition.offset = _dbgGetRegister(_dbgReg_IP) & 0xFFF8;
}

void debugger_t::execute()
{
  onEnter();
  isRunning = true;
  while (isRunning)
  {
    memBrowser.refresh();
    codeBrowser.refresh();
    regBrowser.refresh();

    extern KeyboardDriver *keyboard;
    uint8_t scancode = 0;
    while (!(scancode = keyboard->getLastKey()));
    onKey(scancode);
  }
}

void debugger_t::onEnter()
{
  codePosition.segment = _dbgGetRegister(_dbgReg_CS);
  codePosition.offset   = _dbgGetRegister(_dbgReg_IP);
  memPosition.segment = _dbgGetRegister(_dbgReg_CS);
  memPosition.offset = _dbgGetRegister(_dbgReg_IP) & 0xFFF8;
  regBrowser.init();
  memBrowser.init(&memPosition);
  codeBrowser.init(&codePosition);

  browser = &codeBrowser;
  codeBrowser.setFocus();
}

bool debugger_t::onKey(uint8_t scancode)
{
  const bool handled = widget_t::onKey(scancode);
  if(!handled)
  {
    switch (scancode)
    {
    case KEY_I:
    {
      uint16_t _if = _dbgGetRegister(_dbgReg_F);
      _if ^= (1 << 9);
      _dbgSetRegister(_dbgReg_F, _if);
      regBrowser.refresh();
    }
    break;
    case KEY_5:
      doSingleStep();
      return true;
    case KEY_TAB:
      nextBrowser();
      return true;
    case (KEY_ESC):
      isRunning = false;
      return true;
    }
  }
  return false;
}