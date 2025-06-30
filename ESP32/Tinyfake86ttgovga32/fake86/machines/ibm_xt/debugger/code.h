#ifndef _DEBUGGER_CODE_H_
#define _DEBUGGER_CODE_H_

#include "../service/widget.h"
#include "../cpu/cpu.h"
#include "dasm.h"

class codeBrowser_t : public widget_t
{
  public:
    codeBrowser_t(widget_t * p)
    {
      area.left   = 0 * ACTUAL_FONT_WIDTH;
      area.top    = 0 * ACTUAL_FONT_WIDTH;
      area.width  = 34 * ACTUAL_FONT_WIDTH;
      area.height = 14 * ACTUAL_FONT_HEIGHT;
    }
    void init(DBG_MEM_ADDR * position);
    virtual bool onKey(uint8_t scancode) override;
    virtual void repaint() override;
  protected:
    static const uint32_t FG_ACTIVE = 0x0F;
    static const uint32_t FG_CHANGED = 0x48;
    static const uint32_t FG_INACTIVE = 0x0C;
    static const uint32_t BG_ACTIVE = 0x70;
    static const uint32_t BG_INACTIVE = 0x00;
    static const uint32_t BG_CSIP = 0xF0;

    DBG_MEM_ADDR * position;
    void printColored(line_t * line, uint32_t pos);
    void nextInstruction();
    void prevInstruction();
  };

#endif /* _DEBUGGER_CODE_H_ */