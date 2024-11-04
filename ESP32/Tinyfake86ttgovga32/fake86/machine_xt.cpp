#include "machine_xt.h"
#include <Arduino.h>
#include "cpu/cpu.h"
#include "cpu/ports.h"
#include "mb/i8253.h"
#include "mb/i8259.h"
#include "mb/i8237.h"
#include "io/covox.h"
#include "io/disk.h"
#include "video/render.h"
#include "osd.h"

#define TAG "XT"

MachineXT_t MachineXT_t::instance;

static void videoTask(void *unused);

MachineXT_t::MachineXT_t()
{
  ram = (uint8_t *)0x3F820000;
}

void MachineXT_t::init()
{
  const uint32_t coreID = xPortGetCoreID();
  ESP_LOGI(TAG, "Machine init@core#%i", coreID);

  // ESP_LOGI(TAG, "Create RAM");
  // ram = (uint8_t *)0x3F800000;
  // createRAM();

  ESP_LOGI(TAG, "Initializing keyboard");
  keyboard = new KeyboardDriverSTM();
  keyboard->Init();

  ESP_LOGI(TAG, "Reset CPU");
  init86();
  reset86();
  Covox_t::getInstance().init();
  ESP_LOGI(TAG, "Initializing emulated hardware:");
  ESP_LOGI(TAG, "  - Intel 8253 timer");
  init8253();
  ESP_LOGI(TAG, "  - Intel 8259 interrupt controller");
  init8259();
  ESP_LOGI(TAG, "  - Intel 8237 DMA controller");
  init8237();

  float auxTimer = (float)1.0 / (float)SAMPLE_RATE;
  ticker.attach(auxTimer, my_callback_speaker_func);

  diskInit();

  ESP_LOGI(TAG, "Init render");
  renderInit();
  xTaskCreatePinnedToCore(&videoTask, "videoTask", 1024 * 4, NULL, 5, &videoTaskHandle, 0);

  ESP_LOGI(TAG, "END SETUP %d", ESP.getFreeHeap());
}

bool MachineXT_t::createRAM()
{
  const uint32_t coreID = xPortGetCoreID();
  const uint32_t ramAddr = SOC_EXTRAM_DATA_LOW + (coreID == 1 ? 2 * 1024 * 1024 : 0);
  ram = reinterpret_cast<uint8_t *>(ramAddr);
  ESP_LOGI(TAG, "RAM initialized: core #%i, addr:0x%08X", coreID, ramAddr);
}

void MachineXT_t::run()
{
  stats.startIteration();
  exec86(10000);
  stats.countCPUTime();

  static uint32_t before;
  const uint32_t now = millis();
  if ((now - before) > KEYB_POLL_PERIOD_ms)
  {
    before = now;
    execKeyboard();
    execMisc();
  }
#ifdef use_lib_singlecore
  execVideo();
#endif
  stats.exec();
}

void MachineXT_t::execKeyboard()
{
  const uint8_t scancode = keyboard->Poll();
  if (scancode != 0)
  {
    IOPortSpace::getInstance().get(0x060)->value = scancode;
    doirq(1);
  }
}

void MachineXT_t::execMisc()
{
  OSD_RESULT_t result = do_tinyOSD();
  if (result == OSD_RESULT_PREPARE)
  {
    suspend();
  }
  else if (result == OSD_RESULT_RETURN)
  {
    resume();
  }
}

uint8_t * MachineXT_t::getRAM()
{
  return ram;
}

uint8_t * MachineXT_t::getVideoRAM()
{
  return ram + 0xB8000;
}

void MachineXT_t::suspend()
{
  vTaskSuspend(videoTaskHandle);
}

void MachineXT_t::resume()
{
  vTaskResume(videoTaskHandle);
}

//******************************
static void videoTask(void *unused)
{
  (void)unused;
  while (1)
  {
    extern void draw(void);
    draw();
    vTaskDelay(40 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}
