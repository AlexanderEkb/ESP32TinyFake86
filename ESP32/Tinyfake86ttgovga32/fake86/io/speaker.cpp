#include "io/speaker.h"
#include "config/hardware.h"
#include "cpu/ports.h"
#include "esp32-hal-gpio.h"
#include "covox.h"

static uint32_t period = 0;
static uint8_t gb_frec_speaker_low = 0;
static uint8_t gb_frec_speaker_high = 0;
static bool speakerDrivenByTimer = true;

volatile bool speakerMute = false;

static void onPort0x61Write(uint32_t address, uint8_t val);
static void __always_inline driveSpeaker(bool state);

IOPort port_061h = IOPort(0x061, 0xFF, nullptr, onPort0x61Write);

static void calculatePeriod(int freq)
{
  if (freq != 0)
    period = (SAMPLE_RATE / freq) >> 1;
  else
    period = 0;
}

void onPort0x61Write(uint32_t address, uint8_t val)
{
  (void)address;

  static const uint8_t GATE_TIM_CH2_TO_SPEAKER = 0x01;
  static const uint8_t ENABLE_SPEAKER          = 0x02;

  static const uint8_t TIMER_DRIVEN            = (GATE_TIM_CH2_TO_SPEAKER | ENABLE_SPEAKER);
  speakerDrivenByTimer                         = ((val & TIMER_DRIVEN) == TIMER_DRIVEN);
  if (!speakerDrivenByTimer)
  {
    uint8_t level = (val & ENABLE_SPEAKER) ? HIGH : LOW;
    driveSpeaker(level);
  }
}

void my_callback_speaker_func()
{
  static uint32_t counter = 0;
  static bool speaker;

  if(speakerDrivenByTimer)
  {
    counter++;
    if (counter >= period)
    {
      counter = 0;
      speaker ^= true;
      if (!speakerMute)
      {
        driveSpeaker(speaker);
      }
    }
  }
}

void updateFrequency(uint16_t data)
{
  gb_frec_speaker_low = data & 0x00FF;
  gb_frec_speaker_high = static_cast<uint8_t>(data >> 8);
  uint32_t aData = (data != 0) ? (1193180 / data) : 0;
  calculatePeriod(aData);
}

static void __always_inline driveSpeaker(bool state)
{
  // digitalWrite(SPEAKER_PIN, state);
  Covox_t::getInstance().driveSpeaker(state);
}