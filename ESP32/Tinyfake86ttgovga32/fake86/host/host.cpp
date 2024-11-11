#include "host.h"

#define TAG "HOST"

MachineXT_t * Host_t::machine;
KeyboardDriverCustom_t * Host_t::keyboard;
QueueHandle_t Host_t::keyboardEvents;

void Host_t::init()
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

  ESP_LOGI(TAG, "Initializing keyboard");

  keyboardEvents = xQueueCreate(16, 1);
  keyboard = new KeyboardDriverCustom_t(keyboardEvents);
  keyboard->Init();

  machine = new MachineXT_t(keyboard);
  machine->init();
}

void Host_t::run()
{
  machine->run();
}
