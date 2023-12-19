#ifndef _DEBUGGER_BROWSER_H_
#define _DEBUGGER_BROWSER_H_

#include <stdint.h>
#include "service/service.h"

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
typedef struct rect_t
{
  uint32_t left;
  uint32_t top;
  uint32_t width;
  uint32_t height;
  rect_t& operator=(rect_t& rvalue)
  {
    this->left    = rvalue.left;
    this->top     = rvalue.top;
    this->width   = rvalue.width;
    this->height  = rvalue.height;
    return *this;
  }
} rect_t;

class browser_t
{
  public:
    browser_t() {isActive = false;};
    virtual void onEnter() {isActive = true; refresh();};
    virtual void onKey(uint8_t scancode) = 0;
    virtual void onLeave() {isActive = false; refresh();};
    virtual void refresh() = 0;
    virtual void repaint() = 0;
  protected:
    bool isActive;
    rect_t area;
    virtual uint8_t getDefaultBkg() = 0;
};

#endif /* _DEBUGGER_BROWSER_H_ */