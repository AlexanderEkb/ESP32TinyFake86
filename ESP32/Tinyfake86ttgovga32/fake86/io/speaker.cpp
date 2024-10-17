#include "io/speaker.h"
#include "config/hardware.h"
#include "cpu/ports.h"
#include "covox.h"
#include <driver/periph_ctrl.h>
#include <esp_err.h>
#include <esp_timer.h>

bool Speaker_t::isAttachedToTimer;
uint32_t Speaker_t::period;

volatile bool speakerMute = false;

void Speaker_t::init()
{
  esp_timer_create_args_t config = {
    .callback = &timerISR,
    .arg = nullptr,
    .dispatch_method = ESP_TIMER_ISR,
    .name = "speaker",
    .skip_unhandled_events = true
  };

  esp_timer_handle_t timer;
  ESP_ERROR_CHECK(esp_timer_create(&config, &timer));
  ESP_ERROR_CHECK(esp_timer_start_periodic(timer, 1000000UL / SAMPLE_RATE));
}

void Speaker_t::updateValue(uint16_t value)
{
  uint32_t freq = (value != 0) ? (1193180 / value) : 0;
  if (freq != 0)
    period = (SAMPLE_RATE / freq) >> 1;
  else
    period = 0;
}

void Speaker_t::attachToTimer(bool enable)
{
  isAttachedToTimer = enable;
}

void IRAM_ATTR Speaker_t::timerISR(void * p)
{
  static uint32_t counter = 0;
  static bool speaker;

  if(isAttachedToTimer)
  {
    counter++;
    if (counter >= period)
    {
      counter = 0;
      speaker ^= true;
      if (!speakerMute)
      {
        Covox_t::getInstance().driveSpeaker(speaker);
      }
    }
  }
}
