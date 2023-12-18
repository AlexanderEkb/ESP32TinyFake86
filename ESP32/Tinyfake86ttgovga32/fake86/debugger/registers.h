#ifndef _DEBUGGER_REGISTERS_H_
#define _DEBUGGER_REGISTERS_H_

#include "debugger/browser.h"
#include "cpu/cpu.h"

class regBrowser_t : public browser_t
{
  public:
    virtual void init() override;
    virtual void onKey(uint8_t scancode) override;
    virtual void refresh() override;
  private:
    constexpr uint8_t foreground[2] = {0x80, 0xFF};
    constexpr uint8_t background[2] = {0x40, 0x00};
    static constexpr char *regNames[static_cast<uint32_t>(_dbgReg__COUNT)] = {
    "PC", "AX", "BX", "CX", "DX", "SP", "BP", "SI", "DI", " F", "CS", "DS", "SS", "ES"
  };
};

#endif /* _DEBUGGER_REGISTERS_H_ */