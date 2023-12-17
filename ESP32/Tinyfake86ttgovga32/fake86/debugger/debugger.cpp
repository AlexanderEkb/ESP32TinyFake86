#include "debugger.h"
#include "cpu/cpu.h"
#include "io/keyboard.h"
#include "service/screen.h"

debugger_t debugger_t::instance;

void debugger_t::execute()
{
  while (true)
  {
    refreshRegs();
    refreshCode();
    refreshMem();
    extern KeyboardDriver *keyboard;
    uint8_t scancode = keyboard->getLastKey();
    switch (scancode)
    {
      case (KEY_ESC):
        return;
    }
  }
}

void debugger_t::onEnter()
{

}

void debugger_t::refreshRegs()
{
  static const char *regNames[static_cast<uint32_t>(_dbgReg__COUNT)] = {
    "PC", "AX", "BX", "CX", "DX", "SP", "BP", "SI", "DI", " F", "CS", "DS", "SS", "ES"
  };

  const uint32_t col = 0;
  for(uint32_t i=0; i<_dbgReg__COUNT; i++)
  {
    uint16_t const value = _dbgGetRegister(static_cast<_dbgReg_t>(i));
    const uint32_t LENGTH = 16;
    char buffer[LENGTH];
    snprintf(buffer, LENGTH, "%s: %04X", regNames[i], value);
    svcPrintText(buffer, 0, i*8+16, 0xFF, 0x00);    
  }
}

void debugger_t::refreshMem()
{

}

void debugger_t::refreshCode()
{

}

void debugger_t::refreshPort()
{

}