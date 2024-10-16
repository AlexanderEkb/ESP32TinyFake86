// ~~Port Fake86 to TTGO VGA32 by ackerman~~
// Port Fake86 to ESP32-WROVER by Ochlamonster ;)

//  MODE320x200
//  Single core and dual core


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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/portable.h"
#include "esp_timer.h"

// #include "spiram.h"

///////////////////////////////////////////////////////////////////////////////////////// Local macros

#define TAG "MAIN"
#ifndef use_lib_singlecore
// Video Task Core BEGIN
void videoTask(void *unused);
TaskHandle_t videoTaskHandle;
#endif

// #ifndef use_lib_speaker_cpu
// Ticker gb_ticker_callback;
// #endif

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
  ESP_LOGI(TAG, "Initializing emulated hardware:");
  ESP_LOGI(TAG, "  - Intel 8253 timer:");
  init8253();
  ESP_LOGI(TAG, "    OK");
  ESP_LOGI(TAG, "  - Intel 8259 interrupt controller:");
  init8259();
  ESP_LOGI(TAG, "    OK");
  ESP_LOGI(TAG, "  - Intel 8237 DMA controller:");
  init8237();
  ESP_LOGI(TAG, "OK");
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
  ESP_LOGI(TAG, "HELLO");
  CreateRAM();
  
  renderInit();
  ESP_LOGI(TAG, "VGA OK");
  keyboard->Init();

  reset86();
  Covox_t::getInstance().init();
  inithardware();

#ifndef use_lib_singlecore
  xTaskCreatePinnedToCore(&videoTask, "video", 1024 * 4, NULL, 5, &videoTaskHandle, 1);
#endif

  Speaker_t::init();

  diskInit();

  // Take a snapshot of the number of tasks in case it changes while this
  // function is executing.
  uint32_t count = uxTaskGetNumberOfTasks();
  ESP_LOGI(TAG, "%i tasks are running:", count);

  // Allocate a TaskStatus_t structure for each task.  An array could be
  // allocated statically at compile time.
  TaskStatus_t *pxTaskStatusArray = (TaskStatus_t *)pvPortMalloc( count * sizeof( TaskStatus_t ) );
  uint32_t runtime;

  uxTaskGetSystemState(pxTaskStatusArray, count, &runtime);
  for(uint32_t i=0; i<count; i++)
  {
    ESP_LOGI(TAG, "  %s", pxTaskStatusArray[i].pcTaskName);
  }
  vPortFree(pxTaskStatusArray);
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
  const uint32_t now = esp_timer_get_time() / 1000U;
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

extern "C"
{
  void app_main()
  {
    setup();
    while(true)
      loop();
  }
}