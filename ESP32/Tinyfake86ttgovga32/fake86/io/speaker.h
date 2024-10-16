#ifndef SPEAKER_H
#define SPEAKER_H

#include <stdint.h>
#include <esp_attr.h>

class Speaker_t
{
  public:
    static void init();
    static void updateValue(uint16_t value);
    static void attachToTimer(bool enable);
  private:
    static const uint32_t SAMPLE_RATE = 16000;
    static bool isAttachedToTimer;
    static uint32_t period;
    static void IRAM_ATTR timerISR(void * p);
};

#endif /* SPEAKER_H */