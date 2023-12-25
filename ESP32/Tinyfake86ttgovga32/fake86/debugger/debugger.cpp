#include "debugger/debugger.h"
#include "cpu/cpu.h"
#include "io/keyboard.h"
#include "service/service.h"

debugger_t debugger_t::instance;

debugger_t::debugger_t()
{
  memPosition = DBG_MEM_ADDR(0, 0);
  codePosition = DBG_MEM_ADDR(0, 0);
}

void debugger_t::execute()
{
  onEnter();
  while (true)
  {
    codePosition.segment = _dbgGetRegister(_dbgReg_CS);
    codePosition.offset = _dbgGetRegister(_dbgReg_PC);

    codeBrowser.refresh();
    regBrowser.refresh();
    memBrowser.refresh();

    extern KeyboardDriver *keyboard;
    uint8_t scancode = 0;
    while (!(scancode = keyboard->getLastKey()));
    switch (scancode)
    {
      case KEY_5:
        exec86(1);
        break;
      case (KEY_ESC):
        return;
    }
  }
}

void debugger_t::onEnter()
{
  codePosition.segment = _dbgGetRegister(_dbgReg_CS);
  codePosition.offset   = _dbgGetRegister(_dbgReg_PC);
  regBrowser.init();
  memBrowser.init(memPosition);
  codeBrowser.init(&codePosition);
}
