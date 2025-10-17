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

void Host::init()
{
  ESP_LOGI(TAG, "Host init %d", ESP.getFreeHeap());

#if (KEYBOARD_DRIVER == 0)
  keyboard = new KeyboardDriverSimplifiedXT(); // stm32keyboard();
#elif (KEYBOARD_DRIVER == 1)
  keyboard = new KeyboardDriverAT(); // Regular PS/2 keyboard;
#endif

#if (MOUSE_DRIVER == 0)
  mouse = new MousePs2_t();
#endif

#if (HOST_VIDEO_DRIVER == 0)
  video = new VideoCompositeNtsc_t();
#endif

  disableCore0WDT();
  delay(100);
  disableCore1WDT();

  if (esp_spiram_init() != ESP_OK)
    ESP_LOGE(TAG, "This app requires a board with PSRAM!");

  esp_spiram_init_cache();


  Audio_t::init();
  keyboard->init();
  mouse->init();
  video->init();

  ESP_LOGI(TAG, "Host init ok %d", ESP.getFreeHeap());
}

void Host::run()
{

}
