#include "io/speaker.h"
#include "config/hardware.h"
#include "cpu/ports.h"
#include "esp32-hal-gpio.h"
#include "covox.h"

Speaker_t Speaker_t::instance;
bool Speaker_t::PB1 = false;
bool Speaker_t::Ch2 = false;
bool Speaker_t::muted = false;

void Speaker_t::driveByTimer(bool state)
{
  Ch2 = state;
  Covox_t::getInstance().driveSpeaker(Ch2 && PB1);
}

void Speaker_t::driveDirectly(bool state)
{
  PB1 = state;
  Covox_t::getInstance().driveSpeaker(Ch2 && PB1);
}

void Speaker_t::mute()
{
  muted = true;
  Covox_t::getInstance().driveSpeaker(false);
}

void Speaker_t::unmute()
{
  muted = false;
}
