#ifndef __MOUSE_SUN_H__
#define __MOUSE_SUN_H__

#include "../config/config.h"

#if (MOUSE_DRIVER == 1)
#include "mouse.h"
#include "driver/uart.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

class MouseSun_t : public Mouse_t
{
  public:
    MouseSun_t()
    {
      sink = nullptr;
    }
    virtual ~MouseSun_t() {};
    virtual void init() override;
  private:
    static uint32_t const PKT_LENGTH = 5;
    static const uart_port_t uart_num = UART_NUM_2;
    static const uint32_t uart_buffer_size = 256;
    // static QueueHandle_t uart_queue;
    static void IRAM_ATTR rx_task(void *arg);

};

#endif /* MOUSE_DRIVER */

#endif /* __MOUSE_SUN_H__ */