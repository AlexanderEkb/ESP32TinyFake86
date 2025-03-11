#ifndef SPEAKER_H
#define SPEAKER_H

#include <stdint.h>
#include <esp_attr.h>
#include "covox.h"

class Speaker_t
{
  public:
    static const uint32_t SAMPLE_RATE = 16000;
    static void __attribute__((optimize("-Ofast"))) IRAM_ATTR onTimer();
    static void __attribute__((optimize("-Ofast"))) IRAM_ATTR gateCh2(bool state);
    static void __attribute__((optimize("-Ofast"))) IRAM_ATTR driveDirectly(bool state);
    static void __attribute__((optimize("-Ofast"))) IRAM_ATTR updateFrequency(uint16_t data);
    static void mute();
    static void unmute();
  private:
    static bool PB0;
    static bool PB1;
    static bool Ch2;
    static bool muted;
    static uint32_t 
    period;
};

void __attribute__((optimize("-Ofast"))) IRAM_ATTR my_callback_speaker_func();

#endif /* SPEAKER_H */