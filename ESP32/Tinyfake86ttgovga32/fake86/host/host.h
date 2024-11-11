#ifndef __HOST_HOST_H__
#define __HOST_HOST_H__

#include "machine_xt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

class Host_t
{
  public:
    static void init();
    static void run();
  private:
    static QueueHandle_t keyboardEvents;
    static MachineXT_t * machine;
    static KeyboardDriverCustom_t * keyboard;
};

#endif /* __HOST_HOST_H__ */