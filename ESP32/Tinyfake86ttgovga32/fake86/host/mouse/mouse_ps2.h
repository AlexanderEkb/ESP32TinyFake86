#ifndef __MOUSE_PS2_H__
#define __MOUSE_PS2_H__

#include "mouse.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

class MousePs2_t : public Mouse_t
{
  public:
    MousePs2_t();
    virtual void init() override;
    virtual bool poll(MouseEvent_t * e) override;
  private:
    static QueueHandle_t q;
    static void onMouseExti();
};

#endif /* __MOUSE_PS2_H__ */