// ~~Port Fake86 to TTGO VGA32 by ackerman~~
// Port Fake86 to ESP32-WROVER by Ochlamonster ;)

//  MODE320x200
//  Single core and dual core

#include <Arduino.h>
#include <WiFi.h>
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
#include "io/sdcard.h"
#include "io/speaker.h"
#include "mb/i8237.h"
#include "mb/i8253.h"
#include "mb/i8259.h"
#include "osd.h"
#include "soc/timer_group_struct.h"
#include "stats.h"
#include "video/render.h"
#include "video/video.h"

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

KeyboardDriver *keyboard = new KeyboardDriverAT(); // stm32keyboard();
SdCard sdcard;
Stats stats;

uint8_t     * ram;
unsigned char gb_video_cga[16384];
unsigned char bootdrive = 0;
unsigned char gb_force_load_com = 0;

unsigned char cf;

// static void ClearRAM();
static void execCPU();
static void execKeyboard();
static void execVideo();
static void execMisc();

uint8_t defaultReader_00(uint32_t address)
{
  (void)address;
  return 0x00;
}

uint8_t defaultReader_FF(uint32_t address)
{
  (void)address;
  return 0xFF;
}

uint8_t defaultReader_Stub(uint32_t address)
{
  // LOG("Reading stub port %03xh\n", address & 0x3FF);
  return IOPortSpace::getInstance().get(address)->value;
}

void defaultWriter(uint32_t address, uint8_t value)
{
  (void)value;
  // LOG("Writing stub port %03xh\n", address & 0x3FF);
}

IOPort port_062h = IOPort(0x62, 0x00, defaultReader_00, defaultWriter);

IOPort port_200h = IOPort(0x200, 0x00, defaultReader_Stub, defaultWriter);
IOPort port_201h = IOPort(0x201, 0x00, defaultReader_FF, defaultWriter);

IOPort port_213h = IOPort(0x213, 0x00, defaultReader_Stub, defaultWriter);

IOPort port_241h = IOPort(0x241, 0x00, defaultReader_FF, defaultWriter);
IOPort port_2C1h = IOPort(0x2C1, 0x00, defaultReader_FF, defaultWriter);
IOPort port_341h = IOPort(0x341, 0x00, defaultReader_FF, defaultWriter);

IOPort port_278h = IOPort(0x278, 0x00, defaultReader_FF, defaultWriter);
IOPort port_378h = IOPort(0x378, 0x00, defaultReader_FF, defaultWriter);

IOPort port_2E8h = IOPort(0x2E8, 0x00, defaultReader_FF, defaultWriter);
IOPort port_2EBh = IOPort(0x2EB, 0x00, defaultReader_FF, defaultWriter);
IOPort port_2F8h = IOPort(0x2F8, 0x00, defaultReader_FF, defaultWriter);
IOPort port_2FBh = IOPort(0x2FB, 0x00, defaultReader_FF, defaultWriter);
IOPort port_3E8h = IOPort(0x3E8, 0x00, defaultReader_FF, defaultWriter);
IOPort port_3EBh = IOPort(0x3EB, 0x00, defaultReader_FF, defaultWriter);
IOPort port_3F8h = IOPort(0x3F8, 0x00, defaultReader_Stub, defaultWriter);
IOPort port_3FBh = IOPort(0x3FB, 0x00, defaultReader_FF, defaultWriter);

/*
IOPort port_3B0h = IOPort(0x3B0, 0x00, defaultReader_FF, defaultWriter);
IOPort port_3B1h = IOPort(0x3B1, 0x00, defaultReader_FF, defaultWriter);
IOPort port_3B2h = IOPort(0x3B2, 0x00, defaultReader_FF, defaultWriter);
IOPort port_3B3h = IOPort(0x3B3, 0x00, defaultReader_FF, defaultWriter);
IOPort port_3B4h = IOPort(0x3B4, 0x00, defaultReader_FF, defaultWriter);
IOPort port_3B5h = IOPort(0x3B5, 0x00, defaultReader_FF, defaultWriter);
IOPort port_3B6h = IOPort(0x3B6, 0x00, defaultReader_FF, defaultWriter);
IOPort port_3B7h = IOPort(0x3B7, 0x00, defaultReader_FF, defaultWriter);
IOPort port_3B8h = IOPort(0x3B8, 0x00, defaultReader_FF, defaultWriter);
IOPort port_3B9h = IOPort(0x3B9, 0x00, defaultReader_FF, defaultWriter);
IOPort port_3BAh = IOPort(0x3BA, 0x00, defaultReader_FF, defaultWriter);
IOPort port_3BBh = IOPort(0x3BB, 0x00, defaultReader_FF, defaultWriter);
IOPort port_3BCh = IOPort(0x3BC, 0x00, defaultReader_FF, defaultWriter);
IOPort port_3BDh = IOPort(0x3BD, 0x00, defaultReader_FF,   defaultWriter);
IOPort port_3BEh = IOPort(0x3BE, 0x00, defaultReader_FF,   defaultWriter);
IOPort port_3BFh = IOPort(0x3BF, 0x00, defaultReader_FF,   defaultWriter);

*/
IOPort port_3B4h = IOPort(0x3B4, 0x00, defaultReader_Stub, defaultWriter);
IOPort port_3B5h = IOPort(0x3B5, 0x00, defaultReader_Stub, defaultWriter);
IOPort port_3B8h = IOPort(0x3B8, 0x00, defaultReader_Stub, defaultWriter);
IOPort port_3B9h = IOPort(0x3B9, 0x00, defaultReader_Stub, defaultWriter);
IOPort port_3BAh = IOPort(0x3BA, 0x00, defaultReader_Stub, defaultWriter);
IOPort port_3BCh = IOPort(0x3BC, 0x00, defaultReader_FF,   defaultWriter);
IOPort port_3BDh = IOPort(0x3BD, 0x00, defaultReader_FF,   defaultWriter);
IOPort port_3BEh = IOPort(0x3BE, 0x00, defaultReader_FF,   defaultWriter);
IOPort port_3BFh = IOPort(0x3BF, 0x00, defaultReader_FF,   defaultWriter);

IOPort port_3C0h = IOPort(0x3C0, 0x00, defaultReader_Stub, defaultWriter);
IOPort port_3C2h = IOPort(0x3C2, 0x00, defaultReader_Stub, defaultWriter);
IOPort port_3C3h = IOPort(0x3C3, 0x00, defaultReader_Stub, defaultWriter);
IOPort port_3C4h = IOPort(0x3C4, 0x00, defaultReader_Stub, defaultWriter);
IOPort port_3C5h = IOPort(0x3C5, 0x00, defaultReader_Stub, defaultWriter);
IOPort port_3CBh = IOPort(0x3CB, 0x00, defaultReader_Stub, defaultWriter);
IOPort port_3CCh = IOPort(0x3CC, 0x00, defaultReader_Stub, defaultWriter);

IOPort port_3F2h = IOPort(0x3F2, 0x00, defaultReader_Stub, nullptr);

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

void PerformSpecialActions()
{
  if (gb_reset == 1)
  {
    gb_reset = 0;
    // ClearRAM();
    memset(gb_video_cga, 0, 16384);
    keyboard->Reset();
    reset86();
    inithardware();
    return;
  }
}

//****************************
void CreateRAM()
{
  const uint32_t coreID = xPortGetCoreID();
  const uint32_t ramAddr = SOC_EXTRAM_DATA_LOW + (coreID == 1 ? 2 * 1024 * 1024 : 0);
  ram = (uint8_t *)(ramAddr);
  LOG("RAM initialized: core #%i, addr:0x%08X\n", coreID, ramAddr);
}

// Setup principal
void setup()
{
  // disableCore0WDT();
  // delay(100);
  // disableCore1WDT();

  // WiFi.mode(WIFI_OFF);
  // btStop();

  if (esp_spiram_init() != ESP_OK)
    LOG("This app requires a board with PSRAM!\n");

  esp_spiram_init_cache();

  pinMode(SPEAKER_PIN, OUTPUT);
  digitalWrite(SPEAKER_PIN, LOW);

#ifdef use_lib_log_serial
  Serial.begin(115200);
  Serial.printf("\nHEAP BEGIN %d\n", ESP.getFreeHeap());
#endif
  sdcard.Init();
  CreateRAM();
  // ClearRAM();
  
  renderInit();
  LOG("VGA %d\n", ESP.getFreeHeap());
  keyboard->Init();

  reset86();
  LOG("OK!\n");

  inithardware();

#ifndef use_lib_singlecore
  // BEGIN TASK video
  xTaskCreatePinnedToCore(&videoTask, "videoTask", 1024 * 4, NULL, 5, &videoTaskHandle, 0);
// END Task video
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
  uint16_t *param;
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
  execCPU();
  stats.countCPUTime();
  execKeyboard();
  execMisc();
  execVideo();
  stats.exec();

#ifndef use_lib_singlecore
  // TASK video BEGIN
  TIMERG0.wdt_wprotect = TIMG_WDT_WKEY_VALUE;
  TIMERG0.wdt_feed = 1;
  TIMERG0.wdt_wprotect = 0;
  vTaskDelay(0); // important to avoid task watchdog timeouts - change this to slow down emu
#endif
}

void execCPU()
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
  exec86(10000); // Tarda 22 milis usar 2 cores
#endif
}

void execKeyboard()
{
  static uint32_t before;
  const uint32_t now = millis();
  if ((now - before) > gb_keyboard_poll_milis)
  {
    before = now;
    const uint8_t scancode = keyboard->Poll();
    if (scancode != 0)
    {
      IOPortSpace::getInstance().get(0x060)->value = scancode;
      uint8_t val = IOPortSpace::getInstance().get(0x064)->value;
      IOPortSpace::getInstance().get(0x064)->value = val |= 2;
      doirq(1);
      // Serial.printf("key: 0x%02x\n", scancode);
    }
  }
}

void execVideo()
{
#ifdef use_lib_singlecore
  static uint32_t gb_ini_vga, gb_cur_vga;
  gb_cur_vga = millis();
  if ((gb_cur_vga - gb_ini_vga) >= gb_vga_poll_milis)
  {
    draw();
    gb_ini_vga = gb_cur_vga;
  }
#endif
}

void execMisc()
{
  static uint32_t before;
  const uint32_t now = millis();
  if ((now - before) > gb_keyboard_poll_milis)
  {
    PerformSpecialActions();
    OSD_RESULT_t result = do_tinyOSD();
    if (result == OSD_RESULT_PREPARE)
    {
      vTaskSuspend(videoTaskHandle);
    }
    else if (result == OSD_RESULT_RETURN)
    {
      renderExec();
      renderUpdateBorder();
      vTaskResume(videoTaskHandle);
    }
  }
}