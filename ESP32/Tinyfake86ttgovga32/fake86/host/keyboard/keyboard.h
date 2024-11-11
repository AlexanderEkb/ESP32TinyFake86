#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "fake86.h"
#include "config/gbConfig.h"
#include "gbGlobals.h"
#include "config/hardware.h"
#include "mb/i8259.h"
#include "io/keys.h"
#include <Arduino.h>
#include <esp32-hal-gpio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define TAG "keyboard"

void IRAM_ATTR kb_interruptHandler(void);
uint8_t getScancode(void);

class KeyboardDriver {
  public:
    virtual void Init() = 0;
    virtual void Reset() = 0;
    virtual uint8_t getLastKey() = 0;
};

class KeyboardDriverCustom_t : public KeyboardDriver
{
  public:
    KeyboardDriverCustom_t(QueueHandle_t queue);
    virtual void Init() override;
    virtual void Reset() override;
    virtual uint8_t getLastKey() override;
  private:
    static uint8_t lastKey;
    static QueueHandle_t q;
    static void OnKey(uint8_t scancode);
    friend void IRAM_ATTR kb_interruptHandler(void);
};

#endif /* KEYBOARD_H */