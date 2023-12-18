#ifndef _DEBUGGER_REGISTERS_H_
#define _DEBUGGER_REGISTERS_H_

#include "debugger/browser.h"
#include "cpu/cpu.h"

class regBrowser_t : public browser_t
{
  public:
    regBrowser_t();
    virtual void init(rect_t _area) override;
    virtual void onKey(uint8_t scancode) override;
    virtual void refresh() override;
    virtual void repaint() override;
  private:
    typedef struct registerDesc_t
    {
      uint16_t value;
      bool hasChanged;
    } registerDesc_t;
    registerDesc_t registers[static_cast<uint32_t>(_dbgReg__COUNT)];
    static constexpr char *regNames[static_cast<uint32_t>(_dbgReg__COUNT)] = {
    "PC", "AX", "BX", "CX", "DX", "SP", "BP", "SI", "DI", " F", "CS", "DS", "SS", "ES"
  };
};

#endif /* _DEBUGGER_REGISTERS_H_ */