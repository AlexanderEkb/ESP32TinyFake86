#include "io/speaker.h"
#include "config/hardware.h"
#include "cpu/ports.h"
#include "esp32-hal-gpio.h"
#include "covox.h"

static uint32_t period = 0;
bool speakerDrivenByTimer = true;

volatile bool speakerMute = false;

static void calculatePeriod(int freq)
{
  if (freq != 0)
    period = (SAMPLE_RATE / freq) >> 1;
  else
    period = 0;
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
        Covox_t::getInstance().driveSpeaker(speaker);
      }
    }
  }
}

void updateFrequency(uint16_t data)
{
  uint32_t aData = (data != 0) ? (1193180 / data) : 0;
  calculatePeriod(aData);
}
