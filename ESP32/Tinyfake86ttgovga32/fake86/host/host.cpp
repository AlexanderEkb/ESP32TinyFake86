#include <Arduino.h>
#include <esp32-hal-log.h>
#include "host.h"
#include "keyboard/keyboard_AT.h"
#include "keyboard/keyboard_simplifiedXT.h"
#include "mouse/mouse_ps2.h"

#define TAG "HOST"

Audio_t * Host::audio;
Video_t * Host::video;
Keyboard_t * Host::keyboard;
Mouse_t * Host::mouse;

Host::Host()
{
}

void Host::init()
{
}

void Host::run()
{

}
