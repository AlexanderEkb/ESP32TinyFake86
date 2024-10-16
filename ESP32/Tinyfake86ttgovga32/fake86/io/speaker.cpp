#include "io/speaker.h"
#include "config/hardware.h"
#include "cpu/ports.h"
#include "covox.h"
#include <driver/periph_ctrl.h>
#include <esp_err.h>

Speaker_t Speaker_t::instance;
bool Speaker_t::isAttachedToTimer;
uint32_t Speaker_t::period;

volatile bool speakerMute = false;

Speaker_t::Speaker_t()
{

}

void Speaker_t::init()
{
  
  static const uint32_t TIMER_DIVIDER = 2;
  periph_module_enable(PERIPH_TIMG0_MODULE);
  gptimer_config_t conf = {
    GPTIMER_CLK_SRC_APB,
    GPTIMER_COUNT_UP,
    SAMPLE_RATE,
    0,
    {0, 0}
  };
  gptimer_handle_t timer = nullptr;
  ESP_ERROR_CHECK(gptimer_new_timer(&conf, &timer));
  gptimer_event_callbacks_t cbs = {
    .on_alarm = timerISR,
  };
  ESP_ERROR_CHECK(gptimer_register_event_callbacks(timer, &cbs, nullptr));
  ESP_ERROR_CHECK(gptimer_enable(timer));
  gptimer_alarm_config_t alarm_config = {
        .alarm_count = 1,
        .reload_count = 0,
        .flags = {true}
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(timer, &alarm_config));
    ESP_ERROR_CHECK(gptimer_start(timer));
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

bool IRAM_ATTR Speaker_t::timerISR(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
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

  return false;
}
