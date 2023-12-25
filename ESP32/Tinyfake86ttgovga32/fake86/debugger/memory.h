#ifndef _DEBUGGER_MEMORY_H_
#define _DEBUGGER_MEMORY_H_

#include "cpu/cpu.h"
#include "debugger/browser.h"

class memBrowser_t : public browser_t
{
  public:
    memBrowser_t();
    void init(DBG_MEM_ADDR& position);
    virtual void onKey(uint8_t scancode) override;
    virtual void refresh() override;
    virtual void repaint() override;

  protected:
    virtual uint8_t getDefaultBkg(){return BG_INACTIVE;};

  private:
    static const uint32_t FG_ACTIVE = 0x0F;
    static const uint32_t FG_CHANGED = 0x48;
    static const uint32_t FG_INACTIVE = 0x08;
    static const uint32_t BG_ACTIVE = 0x00;
    static const uint32_t BG_INACTIVE = 0x00;
    static const uint32_t FG_MEM_CONTENT = 0xA8;

    DBG_MEM_ADDR position;
};

#endif /* _DEBUGGER_MEMORY_H_ */