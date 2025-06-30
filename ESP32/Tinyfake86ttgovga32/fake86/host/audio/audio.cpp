#include "audio.h"
#include "../config/config.h"
#include "driver/ledc.h"

#define COVOX_TIMER LEDC_TIMER_0
#define COVOX_MODE LEDC_HIGH_SPEED_MODE
#define COVOX_CHANNEL LEDC_CHANNEL_0
#define COVOX_DUTY_RES LEDC_TIMER_9_BIT // Set duty resolution to 13 bits
#define COVOX_FREQUENCY_HZ (80000)


uint32_t Audio::covox;
uint32_t Audio::speaker;

void Audio::init()
{
  ledc_timer_config_t ledc_timer = {
      .speed_mode = COVOX_MODE,
      .duty_resolution = COVOX_DUTY_RES,
      .timer_num = COVOX_TIMER,
      .freq_hz = COVOX_FREQUENCY_HZ,
      .clk_cfg = LEDC_AUTO_CLK};
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

  // Prepare and then apply the LEDC PWM channel configuration
  ledc_channel_config_t ledc_channel = {
      .gpio_num = AUDIO_OUTPUT_IO,
      .speed_mode = COVOX_MODE,
      .channel = COVOX_CHANNEL,
      .intr_type = LEDC_INTR_DISABLE,
      .timer_sel = COVOX_TIMER,
      .duty = 1,
      .hpoint = 0};
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void Audio::playSample(uint8_t sample)
{
  covox = sample;
  updatePWM();
}

void Audio::updatePWM()
{
  ESP_ERROR_CHECK(ledc_set_duty(COVOX_MODE, COVOX_CHANNEL, speaker | covox));
  ESP_ERROR_CHECK(ledc_update_duty(COVOX_MODE, COVOX_CHANNEL));
}

void Audio::driveSpeaker(bool val)
{
  Audio::speaker = val?0x0100:0x0000;
  updatePWM();
}
