// ~~Port Fake86 to TTGO VGA32 by ackerman~~
// Port Fake86 to ESP32-WROVER by Ochlamonster ;)

//  MODE320x200
//  Single core and dual core

#include <Arduino.h>
#ifndef use_lib_speaker_cpu
#include <Ticker.h>
#endif
#include "config/gbConfig.h"
#include "cpu/cpu.h"
#include "driver/timer.h"
#include "fake86.h"
#include "gbGlobals.h"
#include "io/disk.h"
// #include "gb_sdl_font8x8.h"
#include "config/hardware.h"
#include "cpu/ports.h"
#include "io/keyboard.h"
#include "io/covox.h"
#include "io/speaker.h"
#include "mb/i8237.h"
#include "mb/i8253.h"
#include "mb/i8259.h"
#include "osd.h"
#include "soc/timer_group_struct.h"
#include "stats.h"
#include "video/render.h"

///////////////////////////////////////////////////////////////////////////////////////// Local macros

#ifndef use_lib_singlecore
// Video Task Core BEGIN
void videoTask(void *unused);
TaskHandle_t videoTaskHandle;
#endif

#ifndef use_lib_speaker_cpu
Ticker gb_ticker_callback;
#endif

unsigned char gb_delay_tick_cpu_milis = use_lib_delay_tick_cpu_milis;
unsigned char gb_vga_poll_milis = use_lib_vga_poll_milis;
unsigned char gb_keyboard_poll_milis = use_lib_keyboard_poll_milis;
unsigned char gb_timers_poll_milis = use_lib_timers_poll_milis;

unsigned char gb_reset = 0;

KeyboardDriver *keyboard = new KeyboardDriverSTM(); // stm32keyboard();
Stats stats;

uint8_t     * ram;
unsigned char gb_video_cga[16384];
unsigned char bootdrive = 0;
unsigned char gb_force_load_com = 0;

unsigned char cf;

// static void ClearRAM();
static void execCPU(uint32_t count);
static void execKeyboard();
static void execVideo();
static void execMisc();

//////////////////////////////////////////////////////////////////////////// Local function prototypes
void setup(void);

///////////////////////////////////////////////////////////////////////// External function prototypes
extern void VideoThreadPoll(void);
extern void draw(void);

uint32_t speed = 0;

void inithardware()
{
  LOG("Initializing emulated hardware:\n");
  LOG("  - Intel 8253 timer: ");
  init8253();
  LOG("OK\n");
  LOG("  - Intel 8259 interrupt controller: ");
  init8259();
  LOG("OK\n");
  LOG("  - Intel 8237 DMA controller: ");
  init8237();
  LOG("OK\n");
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
  LOG("RAM initialized: core #%i, addr:0x%08X\n", coreID, ramAddr);
}

void setup()
{
  disableCore0WDT();
  delay(100);
  disableCore1WDT();

  if (esp_spiram_init() != ESP_OK)
    LOG("This app requires a board with PSRAM!\n");

  esp_spiram_init_cache();

#ifdef use_lib_log_serial
  Serial.begin(115200);
  Serial.printf("\nHEAP BEGIN %d\n", ESP.getFreeHeap());
#endif
  CreateRAM();
  
  renderInit();
  LOG("VGA %d\n", ESP.getFreeHeap());
  keyboard->Init();

  reset86();
  LOG("OK!\n");
  Covox_t::getInstance().init();
  inithardware();

#ifndef use_lib_singlecore
  xTaskCreatePinnedToCore(&videoTask, "videoTask", 1024 * 4, NULL, 5, &videoTaskHandle, 0);
#endif

#ifndef use_lib_speaker_cpu
  float auxTimer = (float)1.0 / (float)SAMPLE_RATE;
  gb_ticker_callback.attach(auxTimer, my_callback_speaker_func);
#endif

  diskInit();

  LOG("END SETUP %d\n", ESP.getFreeHeap());
}

#ifndef use_lib_singlecore
//******************************
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
#endif

unsigned char gb_cpunoexe = 0;
unsigned int gb_cpunoexe_timer_ini;
unsigned int tiempo_ini_cpu, tiempo_fin_cpu;
unsigned int tiene_que_tardar = 0;

// Loop main
void loop()
{
  stats.startIteration();
  execCPU(10000);
  stats.countCPUTime();

  static uint32_t before;
  const uint32_t now = millis();
  if ((now - before) > gb_keyboard_poll_milis)
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

void execCPU(uint32_t const count)
{
#ifdef use_lib_singlecore
  static bool gb_cpunoexe = false;
  static uint32_t gb_cpunoexe_timer_ini;
  static uint32_t tiene_que_tardar = 0;

  if (!gb_cpunoexe)
  {
    exec86(10000);
    gb_cpunoexe = 1;
    gb_cpunoexe_timer_ini = millis();
    tiene_que_tardar = gb_delay_tick_cpu_milis;
  }
  else if ((millis() - gb_cpunoexe_timer_ini) >= tiene_que_tardar)
  {
    gb_cpunoexe = 0;
  }

#else
    exec86(count); // Tarda 22 milis usar 2 cores
#endif
}

void execKeyboard()
{
  const uint8_t scancode = keyboard->Poll();
  if (scancode != 0)
  {
    IOPortSpace::getInstance().get(0x060)->value = scancode;
    doirq(1);
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
  OSD_RESULT_t result = do_tinyOSD();
  if (result == OSD_RESULT_PREPARE)
  {
    vTaskSuspend(videoTaskHandle);
  }
  else if (result == OSD_RESULT_RETURN)
  {
    vTaskResume(videoTaskHandle);
  }
}