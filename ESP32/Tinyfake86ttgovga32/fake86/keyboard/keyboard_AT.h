#ifndef __KEYBOARD_KEYBOARD_AT__
#define __KEYBOARD_KEYBOARD_AT__

#include "config/config.h"

#if (KEYBOARD_DRIVER == 1)

#include "keyboard.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

class KeyboardDriverAT : public KeyboardDriver
{
  public:
    static const uint32_t KEY_COUNT = 53;
    KeyboardDriverAT();
    virtual void Init() override;
    virtual void Reset() override;
    uint8_t Poll();
  private:
    static QueueHandle_t q;
    static void onExti();
    static uint8_t translateScancode(uint8_t code);
};
#endif /* KEYBOARD_DRIVER */

#endif /* __KEYBOARD_KEYBOARD_AT__ */