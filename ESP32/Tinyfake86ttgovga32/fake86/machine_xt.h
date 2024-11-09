#ifndef __MACHINE_XT__
#define __MACHINE_XT__

#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <Ticker.h>

#include "io/keyboard.h"
#include "mb/memory.h"
#include "stats.h"

class MachineXT_t
{
  public:
    static MachineXT_t & getInstance() {return instance;};
    void init();
    void run();
    void suspend();
    void resume();

    Memory_t memory;

    // CRUTCH!! Remove ASAP!!!
    KeyboardDriver * getKeyboard() {return keyboard;};
  private:
    MachineXT_t();
    static const uint32_t SAMPLE_RATE = 16000;
    static const uint32_t KEYB_POLL_PERIOD_ms = 20;
    static MachineXT_t instance;
    KeyboardDriver *keyboard;
    TaskHandle_t videoTaskHandle;
    Ticker ticker;
    Stats stats;

    void execKeyboard();
};

#endif /* __MACHINE_XT__ */