#include "host.h"
#include "machines/ibm_xt/machine_xt.h"

#define TAG "HOST"

Machine_t * Host_t::machine;
Keyboard_t * Host_t::keyboard;
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
  uint8_t result;
  if(xQueueReceive(keyboardEvents, &result, 0) == pdTRUE)
  {
    Message_t msg = Message_t(EVENT_KEY, result);
    machine->onEvent(&msg);
  }
  machine->run();
}
