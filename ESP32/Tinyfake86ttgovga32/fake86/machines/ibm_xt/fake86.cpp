// ~~Port Fake86 to TTGO VGA32 by ackerman~~
//   Port Fake86 to ESP32-WROVER by Ochlamonster ;)

#include <Arduino.h>
#include <Ticker.h>
#include "../../host/config/config.h"
#include "../../host/audio/audio.h"
#include "../../host/keyboard/keyboard_simplifiedXT.h"
#include "../../host/keyboard/keyboard_AT.h"
#include "../../host/keyboard/keys.h"
#include "sound/speaker.h"
#include "machine_config.h"
#include "cpu/cpu.h"
#include "driver/timer.h"
#include "io/disk.h"
#include "cpu/ports.h"
#include "chipset/i8237.h"
#include "chipset/i8253.h"
#include "chipset/i8259.h"
#include "io/extensions.h"
#include "extras/osd.h"
#include "soc/timer_group_struct.h"
#include "extras/stats.h"
#include "video/render.h"

#define TAG "FAKE86"

TaskHandle_t videoTaskHandle;

Ticker gb_ticker_callback;

unsigned char gb_reset = 0;
#if (KEYBOARD_DRIVER == 0)
KeyboardDriver *keyboard = new KeyboardDriverSimplifiedXT(); // stm32keyboard();
#elif (KEYBOARD_DRIVER == 1)
KeyboardDriver *keyboard = new KeyboardDriverAT(); // Regular PS/2 keyboard;
#endif
Stats stats;

uint8_t     * ram;
unsigned char gb_video_cga[16384];

//////////////////////////////////////////////////////////////////////////// Local function prototypes
static void execKeyboard();
static void execVideo();
static void execMisc();
void videoTask(void *unused);

///////////////////////////////////////////////////////////////////////// External function prototypes
extern void VideoThreadPoll(void);
extern void draw(void);

uint32_t speed = 0;

void inithardware()
{
  ESP_LOGI(TAG, "Initializing emulated hardware:");
  i8253_init();
  ESP_LOGI(TAG, "- Intel 8253 timer: OK");
  init8259();
  ESP_LOGI(TAG, "- Intel 8259 interrupt controller: OK");
  init8237();
  ESP_LOGI(TAG, "- Intel 8237 DMA controller: OK");
}

void DoSoftReset()
{
  gb_reset = 0;
  // ClearRAM();
  memset(gb_video_cga, 0, 16384);
  keyboard->Reset();
  reset86();
  inithardware();
  return;
}

//****************************
void CreateRAM()
{
  const uint32_t coreID = xPortGetCoreID();
  const uint32_t ramAddr = SOC_EXTRAM_DATA_LOW + (coreID == 1 ? 2 * 1024 * 1024 : 0);
  ram = reinterpret_cast<uint8_t *>(ramAddr);
  ESP_LOGI(TAG, "RAM initialized: core #%i, addr:0x%08X", coreID, ramAddr);
}

void setup()
{
  // To prevent any unwanted squeaks, initialize sound first.
  Audio::init();
  
  disableCore0WDT();
  delay(100);
  disableCore1WDT();

  if (esp_spiram_init() != ESP_OK)
    ESP_LOGE(TAG, "This app requires a board with PSRAM!");

  esp_spiram_init_cache();

  CreateRAM();
  
  renderInit();
  ESP_LOGI(TAG, "VGA %d", ESP.getFreeHeap());
  keyboard->Init();

  reset86();
  ESP_LOGI(TAG, "OK!");
  inithardware();

#ifndef use_lib_singlecore
  xTaskCreatePinnedToCore(&videoTask, "videoTask", 1024 * 4, NULL, 5, &videoTaskHandle, 0);
#endif

#ifndef use_lib_speaker_cpu
  float auxTimer = (float)1.0 / (float)Speaker_t::SAMPLE_RATE;
  gb_ticker_callback.attach(auxTimer, my_callback_speaker_func);
#endif

  diskInit();
  Extensions_t::init();
  ESP_LOGI(TAG, "END SETUP %d", ESP.getFreeHeap());
}

void videoTask(void *unused)
{
  (void)unused;
  while (1)
  {
    draw();
    vTaskDelay(40 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}

unsigned char gb_cpunoexe = 0;
unsigned int gb_cpunoexe_timer_ini;
unsigned int tiempo_ini_cpu, tiempo_fin_cpu;
unsigned int tiene_que_tardar = 0;

// Loop main
void loop()
{
  stats.startIteration();
  exec86(10000);
  stats.countCPUTime();

  static uint32_t before;
  const uint32_t now = millis();
  if ((now - before) > KEYBOARD_POLL_ms)
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

void execKeyboard()
{
  const uint8_t scancode = keyboard->Poll();
  if (scancode != 0)
  {
    if(scancode == KEY_F12)
    {
      vTaskSuspend(videoTaskHandle);
      do_tinyOSD();
      vTaskResume(videoTaskHandle);
    }
    else
    {
      IOPortSpace::getInstance().get(0x060)->value = scancode;
      doirq(1);
    }
  }
}

#ifdef use_lib_singlecore
void execVideo()
{
  static uint32_t gb_ini_vga, gb_cur_vga;
  gb_cur_vga = millis();
  if ((gb_cur_vga - gb_ini_vga) >= gb_vga_poll_milis)
  {
    draw();
    gb_ini_vga = gb_cur_vga;
  }
}
#endif

void execMisc()
{
  if (gb_reset == 1)
  {
    DoSoftReset();
  }
}