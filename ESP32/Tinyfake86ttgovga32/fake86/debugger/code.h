#ifndef _DEBUGGER_CODE_H_
#define _DEBUGGER_CODE_H_

#include "cpu/cpu.h"
#include "debugger/browser.h"
#include "debugger/dasm.h"

class codeBrowser_t : public browser_t
{
  public:
      codeBrowser_t() {isActive = false;};
      void init(DBG_MEM_ADDR * position);
      virtual void onKey(uint8_t scancode) override;
      virtual void refresh() override;
      virtual void repaint() override;
  protected:
    virtual uint8_t getDefaultBkg() { return BG_INACTIVE; };
  private:
    static const uint32_t FG_ACTIVE = 0x0F;
    static const uint32_t FG_CHANGED = 0x48;
    static const uint32_t FG_INACTIVE = 0x0C;
    static const uint32_t BG_ACTIVE = 0x70;
    static const uint32_t BG_INACTIVE = 0x70;

    DBG_MEM_ADDR * position;
    void printColored(line_t * line, uint32_t pos);
};

#endif /* _DEBUGGER_CODE_H_ */