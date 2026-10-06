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

/**
 * This is a representation of a mouse for the Machine object.
 * To emulate a particular kind of a mouse, you should prepare a driver
 * for it and somehow handle onMouseEvent() method in your Machine.
 */
class GenericMouse_t
{
  public:
    virtual void onMouseEvent(int32_t dx, int32_t dy, uint8_t btn) = 0;
};

/**
 * This is a representation of a mouse for the Host object. 
 * To make your particular mouse working you should prepare a driver
 * for the particular mouse connected to your build, inherited from
 * this class, and provide it to the Host;
 * Now there kinds of mices are supported:
 *   - PS/2 Mouse
 *   - Sun Microsystems Mouse (also serial)
 */
class Mouse_t
{
  public:
    Mouse_t()
    {
      sink = nullptr;
    }
    virtual ~Mouse_t() {};
    virtual void init() = 0;
    virtual void bind(GenericMouse_t * q)
    {
      sink = q;
    }
  protected:
    GenericMouse_t * sink;
};

#endif /* __MOUSE_H__ */