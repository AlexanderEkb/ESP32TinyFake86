#ifndef _DEBUGGER_REGISTERS_H_
#define _DEBUGGER_REGISTERS_H_

#include "browser.h"
#include "cpu/cpu.h"

class regBrowser_t : public browser_t
{
  public:
    regBrowser_t();
    void init();
    virtual void onKey(uint8_t scancode) override;
    virtual void refresh() override;
    virtual void repaint() override;
  protected:
      virtual uint8_t getDefaultBkg() {return BG_INACTIVE;};

  private:
      static const uint32_t FG_ACTIVE = 0x0F;
      static const uint32_t FG_CHANGED = 0x15;
      static const uint32_t FG_INACTIVE = 0x0C;
      static const uint32_t BG_ACTIVE = 0x00;
      static const uint32_t BG_INACTIVE = 0x04;

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