#define TAG "XT"

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

static void videoTask(void *unused);
#define TAG "XT"

void MachineXT_t::init()
{
  ESP_LOGI(TAG, "START SETUP %d", ESP.getFreeHeap());
  ESP_LOGI(TAG, "Initializing emulated hardware:");
  init8253();
  init8259();
  init8237();

  float auxTimer = (float)1.0 / (float)SAMPLE_RATE;
  ticker.attach(auxTimer, my_callback_speaker_func);

  Covox_t::getInstance().init();

  diskInit();

  renderInit();
  ESP_LOGI(TAG, "Starting dumper");
  xTaskCreatePinnedToCore(&videoTask, "videoTask", 1024 * 4, NULL, 5, &videoTaskHandle, 0);
  extern uint8_t videomem[];
  memory->init(videomem);
  init86(this);

  ESP_LOGI(TAG, "END SETUP %d", ESP.getFreeHeap());
}

void MachineXT_t::run()
{
  if(isMachineRunning)
  {
    stats.startIteration();
    exec86(10000);
    stats.countCPUTime();
    stats.exec();
  }
  else
  {
    osd.execute();
  }
}

void MachineXT_t::onEvent(Message_t * msg)
{
  switch(msg->event)
  {
    case EVENT_KEY:
      ESP_LOGI(TAG, "EVENT_KEY 0x%X", msg->param);
      IOPortSpace::getInstance().get(0x060)->value = (uint8_t)msg->param;
      doirq(1);
      break;
  }
}

void MachineXT_t::suspend()
{
  isMachineRunning = false;
  vTaskSuspend(videoTaskHandle);
}

void MachineXT_t::resume()
{
  isMachineRunning = true;
  vTaskResume(videoTaskHandle);
}

// TODO: Consider getting rid of this crutch
void MachineXT_t::boot()
{
  extern union _bytewordregs_ regs;
  extern unsigned short int segregs[4];

  uint8_t bootdrive = getBootDrive();
  if (bootdrive < 255)
  { // read first sector of boot drive into 07C0:0000 and execute it
    regs.byteregs[regdl] = bootdrive;
    DISK_ADDR src = DISK_ADDR(bootdrive, 0, 0, 1, 1);
    MEM_ADDR dst = MEM_ADDR(0x07C0, 0x0000);
    readdisk(src, dst);
    segregs[regcs] = 0x0000;
    SetRegIP(0x7C00);
  }
  else
  {
    segregs[regcs] = 0xF600; // start ROM BASIC at bootstrap if requested
    SetRegIP(0x0000);
  }
}

Keyboard_t * MachineXT_t::getKeyboard()
{
  return keyboard;
}

Drive_t * MachineXT_t::getDrive(uint32_t index)
{
  return drives[index];
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
