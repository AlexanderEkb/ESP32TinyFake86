// ~~Port Fake86 to TTGO VGA32 by ackerman~~
// Port Fake86 to ESP32-WROVER by Ochlamonster ;)

#include "host/host.h"

///////////////////////////////////////////////////////////////////////////////////////// Local macros

//****************************
void setup()
{
  Host_t::init();
}

// Loop main
void loop()
{
  Host_t::run();
}
