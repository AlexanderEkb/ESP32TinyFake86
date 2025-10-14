// ~~Port Fake86 to TTGO VGA32 by ackerman~~
//   Port Fake86 to ESP32-WROVER by Ochlamonster ;)

#include <Arduino.h>
#include <Ticker.h>
#include "../../host/host.h"
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
#include "chipset/i8250.h"
#include "io/extensions.h"
#include "serial_mouse/serial_mouse.h"
#include "extras/osd.h"
#include "soc/timer_group_struct.h"
#include "extras/stats.h"
#include "video/render_cga.h"

#define TAG "FAKE86"

TaskHandle_t videoTaskHandle;

Ticker gb_ticker_callback;

unsigned char gb_reset = 0;
I8250_t * com1 = new I8250_t(0x3F8, 4);
SerialMouse_t * serMouse;
Stats stats;

uint8_t     * ram;
unsigned char gb_video_cga[16384];

//////////////////////////////////////////////////////////////////////////// Local function prototypes
static void createRAM();
static void inithardware();
static void execKeyboard();
static void execVideo();
static void execMisc();
void videoTask(void *unused);

///////////////////////////////////////////////////////////////////////// External function prototypes
extern void VideoThreadPoll(void);
extern void draw(void);

uint32_t speed = 0;

void setup()
{
  ESP_LOGI(TAG, "Ok, let's rock!");
  Host::init();

  createRAM();
  renderInit();
  inithardware();
  serMouse = new SerialMouse_t(com1);
#ifndef use_lib_singlecore
  xTaskCreatePinnedToCore(&videoTask, "videoTask", 1024 * 4, NULL, 5, &videoTaskHandle, 0);
#endif

#ifndef use_lib_speaker_cpu
  float auxTimer = (float)1.0 / (float)Speaker_t::SAMPLE_RATE;
  gb_ticker_callback.attach(auxTimer, my_callback_speaker_func);
#endif

  diskInit();
  Extensions_t::init();
  reset86();
  ESP_LOGI(TAG, "END SETUP %d", ESP.getFreeHeap());
}

static void inithardware()
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
  Host::keyboard->Reset();
  reset86();
  inithardware();
  return;
}

//****************************
static void createRAM()
{
  const uint32_t coreID = xPortGetCoreID();
  const uint32_t ramAddr = SOC_EXTRAM_DATA_LOW + (coreID == 1 ? 2 * 1024 * 1024 : 0);
  ram = reinterpret_cast<uint8_t *>(ramAddr);
  ESP_LOGI(TAG, "RAM initialized: core #%i, addr:0x%08X", coreID, ramAddr);
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
  const uint8_t scancode = Host::keyboard->Poll();
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