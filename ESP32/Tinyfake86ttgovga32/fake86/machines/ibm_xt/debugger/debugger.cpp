#include "debugger.h"
#include "../cpu/cpu.h"
#include "../../../host/host.h"
#include "../../../host/keyboard/keys.h"
#include "../service/service.h"

debugger_t debugger_t::instance = debugger_t();

void debugger_t::nextBrowser()
{
  screen.next();
}

void debugger_t::doSingleStep()
{
    exec86(1);
    codePosition.segment = _dbgGetRegister(_dbgReg_CS);
    codePosition.offset = _dbgGetRegister(_dbgReg_IP);
}

void debugger_t::execute()
{
  onEnter();
  isRunning = true;
  while (isRunning)
  {
    uint8_t scancode = 0;
    while (!(scancode = Host::keyboard->Poll()));
    if(onKey(scancode))
    {
      screen.repaint();
    }
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

  codeBrowser.setFocus();
  screen.repaint();
}

bool debugger_t::onKey(uint8_t scancode)
{
  const bool handled = screen.onKey(scancode);
  if(handled)
  {
    return true;
  }
  else
  {
    switch (scancode)
    {
    case KEY_I:
    {
      uint16_t _if = _dbgGetRegister(_dbgReg_F);
      _if ^= (1 << 9);
      _dbgSetRegister(_dbgReg_F, _if);
      regBrowser.repaint();
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