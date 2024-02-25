#include "io/speaker.h"
#include "esp32-hal-gpio.h"
#include "config/hardware.h"
#include "cpu/ports.h"

static uint32_t period = 0;

uint8_t gb_frec_speaker_low = 0;
uint8_t gb_frec_speaker_high = 0;
static volatile bool speakerDrivenByTimer = true;
static volatile bool speakerEnabled = true;

volatile uint8_t gb_frecuencia01 = 0;
volatile uint8_t gb_volumen01 = 0;

static void onPort0x61Write(uint32_t address, uint8_t val);

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

  LOG("61h: %02xh\n", val);
  static const uint8_t GATE_TIM_CH2_TO_SPEAKER = 0x01;
  static const uint8_t ENABLE_SPEAKER          = 0x02;
  static const uint8_t TIMER_DRIVEN            = (GATE_TIM_CH2_TO_SPEAKER | ENABLE_SPEAKER);
  speakerDrivenByTimer = ((val & TIMER_DRIVEN) == TIMER_DRIVEN);
  if (speakerDrivenByTimer)
  {
    uint32_t data = (gb_frec_speaker_high << 8) | gb_frec_speaker_low;
    updateFrequency(data);
    uint32_t aData = (data != 0) ? (1193180 / data) : 0;
    calculatePeriod(aData);
    gb_volumen01 = 128;
    gb_frecuencia01 = aData;
  }
  else
  {
    uint8_t level = (val & ENABLE_SPEAKER)?HIGH:LOW;
    digitalWrite(SPEAKER_PIN, level);
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
      if ((gb_volumen01 != 0) && (gb_frecuencia01 != 0))
      {
        digitalWrite(SPEAKER_PIN, speaker); // ? HIGH : LOW);
      }
    }
  }
}

void updateFrequency(uint16_t data)
{
  gb_frec_speaker_low = data & 0x00FF;
  gb_frec_speaker_high = static_cast<uint8_t>(data >> 8);
}