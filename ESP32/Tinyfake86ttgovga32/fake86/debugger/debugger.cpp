#include "debugger.h"
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
  codePosition.segment = _dbgGetRegister(_dbgReg_CS);
  codePosition.offset   = _dbgGetRegister(_dbgReg_PC);
  regBrowser.init();
  memBrowser.init(memPosition);
  regBrowser.refresh();
  memBrowser.refresh();
}

// void debugger_t::refreshRegs()
// {
//   static const char *regNames[static_cast<uint32_t>(_dbgReg__COUNT)] = {
//     "PC", "AX", "BX", "CX", "DX", "SP", "BP", "SI", "DI", " F", "CS", "DS", "SS", "ES"
//   };

//   const uint32_t col = 0;
//   for(uint32_t i=0; i<_dbgReg__COUNT; i++)
//   {
//     uint16_t const value = _dbgGetRegister(static_cast<_dbgReg_t>(i));
//     const uint32_t LENGTH = 16;
//     char buffer[LENGTH];
//     snprintf(buffer, LENGTH, "%s: %04X", regNames[i], value);
//     svcPrintText(buffer, 0, i*8, 0xFF, 0x00);
//   }
// }

// void debugger_t::refreshMem()
// {
//   static const uint32_t MEM_ROW_COUNT = 9;
//   static const uint32_t MEM_COL_COUNT = 8;
//   DBG_MEM_ADDR position = memPosition;
//   for (uint32_t row = 0; row < MEM_ROW_COUNT; row++)
//   {
//     static const uint32_t LENGTH = 16;
//     char buffer[LENGTH];
//     snprintf(buffer, LENGTH, "%04X:%04X", position.segment, position.offset);
//     static const uint32_t MEM_START_ROW = 14;
//     svcPrintText(buffer, 0, (MEM_START_ROW + row) * 8, 0x88, 0x00);
//     for(uint32_t col=0; col<MEM_COL_COUNT; col++)
//     {
//       const uint8_t value = read86(position.linear());
//       snprintf(buffer, LENGTH, "%02X", value);
//       svcPrintText(buffer, (10 + col * 3) * 8, (MEM_START_ROW + row) * 8, 0xA8, 0x00);
//       svcPrintChar(value, (241 + col) * 8, (MEM_START_ROW + row) * 8, 0xA8, 0x00);
//       position++;
//     }
//   }
// }
