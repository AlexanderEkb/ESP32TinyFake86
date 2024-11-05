#ifndef __MACHINE_XT__
#define __MACHINE_XT__

#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <Ticker.h>

#include "io/keyboard.h"
#include "stats.h"

class MachineXT_t
{
  public:
    static MachineXT_t & getInstance() {return instance;};
    void init();
    void run();
    void suspend();
    void resume();

    uint8_t * getRAM();

    // CRUTCH!! Remove ASAP!!!
    KeyboardDriver * getKeyboard() {return keyboard;};
  private:
    MachineXT_t();
    static const uint32_t SAMPLE_RATE = 16000;
    static const uint32_t KEYB_POLL_PERIOD_ms = 20;
    uint8_t * ram;
    static MachineXT_t instance;
    KeyboardDriver *keyboard;
    TaskHandle_t videoTaskHandle;
    Ticker ticker;
    Stats stats;

    bool createRAM();
    void execKeyboard();
};

#endif /* __MACHINE_XT__ */