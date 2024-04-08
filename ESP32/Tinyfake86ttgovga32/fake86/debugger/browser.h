#ifndef _DEBUGGER_BROWSER_H_
#define _DEBUGGER_BROWSER_H_

#include <stdint.h>
#include "service/service.h"
#include "service/widget.h"

#define FONT_HEIGHT (8)
#define FONT_WIDTH  (8)
/*
 * Registers
 * Disassembly
 * Memory
 * Ports
 * Breakpoints (?)
 * Stack (?)
 */

static const uint32_t ACTUAL_FONT_WIDTH = 8; //SERVICE_FONT_WIDTH + 1;
static const uint32_t ACTUAL_FONT_HEIGHT = 8; //SERVICE_FONT_HEIGHT + 1;

class browser_t : public widget_t
{
  public:
    virtual void refresh()
    {
      for(widget_t * w : children)
        w->repaint();
    }
    virtual bool onKey(uint8_t scancode)
    {
      (void)scancode;
      return false;
    }
};

#endif /* _DEBUGGER_BROWSER_H_ */