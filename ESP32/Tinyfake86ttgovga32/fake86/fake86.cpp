// ~~Port Fake86 to TTGO VGA32 by ackerman~~
// Port Fake86 to ESP32-WROVER by Ochlamonster ;)

#include "machine_xt.h"

///////////////////////////////////////////////////////////////////////////////////////// Local macros
#define TAG "HOST"

//****************************
void setup()
{
  ESP_LOGI(TAG, "Entering setup");
  disableCore0WDT();
  delay(100);
  disableCore1WDT();
  ESP_LOGI(TAG, "HEAP BEGIN %d", ESP.getFreeHeap());
  if (esp_spiram_init() != ESP_OK)
    ESP_LOGE(TAG, "This app requires a board with PSRAM!");
  ESP_LOGI(TAG, "Init cache");
  esp_spiram_init_cache();

  MachineXT_t::getInstance().init();
}

// Loop main
void loop()
{
  MachineXT_t::getInstance().run();
}
