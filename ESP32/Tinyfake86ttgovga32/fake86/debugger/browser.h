#ifndef _DEBUGGER_BROWSER_H_
#define _DEBUGGER_BROWSER_H_

#include <stdint.h>

/*
 * Registers
 * Disassembly
 * Memory
 * Ports
 * Breakpoints (?)
 * Stack (?)
 */

class browser_t
{
  public:
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

    browser_t() {isActive = false;};
    virtual void init(rect_t _area) = 0;
    virtual void onEnter() {isActive = true; refresh();};
    virtual void onKey(uint8_t scancode) = 0;
    virtual void onLeave() {isActive = false; refresh();};
    virtual void refresh() = 0;
    virtual void repaint() = 0;
  protected:
    bool isActive;
    rect_t area;
};

#endif /* _DEBUGGER_BROWSER_H_ */