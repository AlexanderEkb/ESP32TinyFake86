#ifndef SPEAKER_H
#define SPEAKER_H

#include <stdint.h>
#include <esp_attr.h>
#include <driver/gptimer.h>

class Speaker_t
{
  public:
    static Speaker_t & getInstance()
    {
      return instance;
    }
    static void init();
    static void updateValue(uint16_t value);
    static void attachToTimer(bool enable);
  private:
    static const uint32_t SAMPLE_RATE = 16000;
    static Speaker_t instance;
    static bool isAttachedToTimer;
    static uint32_t period;

    Speaker_t();
    static bool IRAM_ATTR timerISR(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx);
};

#endif /* SPEAKER_H */