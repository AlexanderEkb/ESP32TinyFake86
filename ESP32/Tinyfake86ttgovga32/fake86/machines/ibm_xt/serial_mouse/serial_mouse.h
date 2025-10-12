#ifndef __SERIAL_MOUSE_H__
#define __SERIAL_MOUSE_H__

#include "../chipset/i8250.h"
#include "../../../host/mouse/mouse.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

class SerialMouse_t : public MouseImplementation_t, SerialPeripheral_t
{
  public:
    SerialMouse_t(I8250_t * port);
    virtual void onMouseEvent(int32_t dx, int32_t dy, uint8_t btn) override;
    virtual void onModemControlEvent(uint8_t mcr) override;
  private:
    I8250_t * port;
    uint8_t RTS;
};

#endif /* __SERIAL_MOUSE_H__ */