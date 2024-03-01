#include "debugger/debugger.h"
#include "cpu/cpu.h"
#include "io/keyboard.h"
#include "service/service.h"

debugger_t debugger_t::instance;

debugger_t::debugger_t()
{
  memPosition   = DBG_MEM_ADDR(0, 0);
  codePosition  = DBG_MEM_ADDR(0, 0);
  activeBrowser = 0;
}

void debugger_t::cycleActive()
{
  browsers[activeBrowser]->isActive = false;
  browsers[activeBrowser]->repaint();
  if(++activeBrowser >= BROWSER_COUNT) activeBrowser = 0;
  browsers[activeBrowser]->isActive = true;
  browsers[activeBrowser]->repaint();
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
  while (true)
  {
    memBrowser.refresh();
    codeBrowser.refresh();
    regBrowser.refresh();

    extern KeyboardDriver *keyboard;
    uint8_t scancode = 0;
    while (!(scancode = keyboard->getLastKey()));
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
        break;
      case KEY_TAB:
        cycleActive();
        break;
      case (KEY_ESC):
        return;
      default:
        browsers[activeBrowser]->onKey(scancode);
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

  activeBrowser = 2;
  codeBrowser.isActive = true;
}
