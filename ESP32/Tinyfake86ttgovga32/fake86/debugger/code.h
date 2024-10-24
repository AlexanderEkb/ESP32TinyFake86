#ifndef _DEBUGGER_CODE_H_
#define _DEBUGGER_CODE_H_

#include "cpu/cpu.h"
#include "debugger/browser.h"
#include "debugger/dasm.h"

class codeBrowser_t : public browser_t
{
  public:
      codeBrowser_t() {
        position = nullptr;
      };
      void init(DBG_MEM_ADDR * position);
      virtual bool onKey(uint8_t scancode) override;
      virtual void refresh() override;
      virtual void repaint() override;
      DBG_MEM_ADDR getNextInstruction();
  private:
    static const uint32_t FG_ACTIVE = 0x0F;
    static const uint32_t FG_CHANGED = 0x48;
    static const uint32_t FG_INACTIVE = 0x0C;
    static const uint32_t BG_ACTIVE = 0x70;
    static const uint32_t BG_INACTIVE = 0x00;
    static const uint32_t BG_CSIP = 0xF0;

    DBG_MEM_ADDR * position;
    void printColored(line_t * line, uint32_t pos);
};

#endif /* _DEBUGGER_CODE_H_ */