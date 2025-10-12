#ifndef __MOUSE_H__
#define __MOUSE_H__

#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

static uint8_t const MOUSE_BUTTON_L = 0x01;
static uint8_t const MOUSE_BUTTON_R = 0x02;
static uint8_t const MOUSE_BUTTON_M = 0x04;
static uint8_t const BTN_MASK = (
  MOUSE_BUTTON_L |
  MOUSE_BUTTON_M |
  MOUSE_BUTTON_R);

class MouseImplementation_t
{
  public:
    virtual void onMouseEvent(int32_t dx, int32_t dy, uint8_t btn) = 0;
};

class Mouse_t
{
  public:
    Mouse_t()
    {
      sink = nullptr;
    }
    virtual ~Mouse_t() {};
    virtual void init() = 0;
    virtual void bind(MouseImplementation_t * q)
    {
      sink = q;
    }
  protected:
    MouseImplementation_t * sink;
};

#endif /* __MOUSE_H__ */