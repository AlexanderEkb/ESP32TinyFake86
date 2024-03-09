#include "covox.h"
#include "config/hardware.h"
#include "cpu/ports.h"
#include "driver/ledc.h"

#define COVOX_TIMER LEDC_TIMER_0
#define COVOX_MODE LEDC_HIGH_SPEED_MODE
#define COVOX_CHANNEL LEDC_CHANNEL_0
#define COVOX_DUTY_RES LEDC_TIMER_9_BIT // Set duty resolution to 13 bits
#define COVOX_FREQUENCY_HZ (80000)

static void write(uint32_t address, uint8_t value);

Covox_t Covox_t::instance;
uint32_t Covox_t::covox;
uint32_t Covox_t::speaker;
IOPort port_378h = IOPort(0x378, 0x00, nullptr, write);

void Covox_t::init()
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
      .gpio_num = COVOX_OUTPUT_IO,
      .speed_mode = COVOX_MODE,
      .channel = COVOX_CHANNEL,
      .intr_type = LEDC_INTR_DISABLE,
      .timer_sel = COVOX_TIMER,
      .duty = 1,
      .hpoint = 0};
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void Covox_t::playSample(uint8_t sample)
{
  covox = sample;
  updatePWM();
}

void Covox_t::updatePWM()
{
  ESP_ERROR_CHECK(ledc_set_duty(COVOX_MODE, COVOX_CHANNEL, speaker | covox));
  ESP_ERROR_CHECK(ledc_update_duty(COVOX_MODE, COVOX_CHANNEL));
}

void Covox_t::driveSpeaker(bool val)
{
  Covox_t::speaker = val?0x0100:0x0000;
  updatePWM();
}

static void write(uint32_t address, uint8_t value)
{
  (void)address;
  Covox_t::getInstance().playSample(value);
}