#ifndef __HOST_HOST_H__
#define __HOST_HOST_H__

#include "../machines/machine.h"
#include "keyboard/keyboard.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

class Host_t
{
  public:
    static void init();
    static void run();
  private:
    static QueueHandle_t keyboardEvents;
    static Machine_t * machine;
    static Keyboard_t * keyboard;
};

#endif /* __HOST_HOST_H__ */