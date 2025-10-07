#ifndef __MOUSE_H__
#define __MOUSE_H__

#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

typedef struct MouseEvent_t
{
  int32_t dx;
  int32_t dy;
  int32_t btn;
} MouseEvent_t;

class Mouse_t
{
  public:
    Mouse_t()
    {
      sink = nullptr;
    }
    virtual ~Mouse_t() {};
    virtual void init() = 0;
    virtual void bind(xQueueHandle q)
    {
      sink = q;
    }
    virtual bool poll(MouseEvent_t * e) = 0;
  protected:
    xQueueHandle sink;
};

#endif /* __MOUSE_H__ */