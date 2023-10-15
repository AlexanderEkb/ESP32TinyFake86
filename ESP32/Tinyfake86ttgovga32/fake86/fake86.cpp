// ~~Port Fake86 to TTGO VGA32 by ackerman~~
// Port Fake86 to ESP32-WROVER by Ochlamonster ;)

//  MODE320x200
//  Single core and dual core

#include <Arduino.h>
#ifndef use_lib_speaker_cpu
#include <Ticker.h>
#endif
#include "cpu.h"
#include "dataFlash/gbcom.h"
#include "disk.h"
#include "driver/timer.h"
#include "fake86.h"
#include "gbConfig.h"
#include "gbGlobals.h"
#include "gb_sdl_font8x8.h"
#include "hardware.h"
#include "i8237.h"
#include "i8253.h"
#include "i8259.h"
#include "keyboard.h"
#include "osd.h"
#include "render.h"
#include "sdcard.h"
#include "soc/timer_group_struct.h"
#include "timing.h"
#include "video.h"
#include "stats.h"
#include "speaker.h"
#include "ports.h"

///////////////////////////////////////////////////////////////////////////////////////// Local macros

#ifndef use_lib_singlecore
// Video Task Core BEGIN
void videoTask(void *unused);
QueueHandle_t vidQueue;
TaskHandle_t videoTaskHandle;
volatile bool videoTaskIsRunning = false;
// Video Task Core END
#endif

#ifndef use_lib_speaker_cpu
Ticker gb_ticker_callback;
#endif

unsigned char gb_invert_color = 0;

unsigned char gb_delay_tick_cpu_milis = use_lib_delay_tick_cpu_milis;
unsigned char gb_vga_poll_milis = use_lib_vga_poll_milis;
unsigned char gb_keyboard_poll_milis = use_lib_keyboard_poll_milis;
unsigned char gb_timers_poll_milis = use_lib_timers_poll_milis;

unsigned char gb_font_8x8 = 1;

unsigned char gb_reset = 0;
unsigned char gb_id_cur_com = 0;

KeyboardDriver *keyboard = new KeyboardDriverAT(); // stm32keyboard();
SdCard sdcard;
Stats stats;

unsigned char *gb_ram_bank[PAGE_COUNT];
unsigned char gb_video_cga[16384];
unsigned char bootdrive = 0;
unsigned char gb_force_load_com = 0;

unsigned char cf;
unsigned char running = 0;

uint8_t defaultReader_00(uint32_t address)
{
  (void)address;
  return 0x00;
}

uint8_t defaultReader_FF(uint32_t address)
{
  (void)address;
  return 0;
}

IOPort port_062 = IOPort(0x200, 0x00, defaultReader_00, nullptr);

IOPort port_200 = IOPort(0x200, 0x00, defaultReader_FF, nullptr);
IOPort port_201 = IOPort(0x201, 0x00, defaultReader_FF, nullptr);
IOPort port_3F2 = IOPort(0x3F2, 0x00, defaultReader_FF, nullptr);

IOPort port_3B4 = IOPort(0x3B4, 0x00, defaultReader_FF, nullptr);
IOPort port_3B5 = IOPort(0x3B5, 0x00, defaultReader_FF, nullptr);
IOPort port_3B8 = IOPort(0x3B8, 0x00, defaultReader_FF, nullptr);
IOPort port_3B9 = IOPort(0x3B9, 0x00, defaultReader_FF, nullptr);
IOPort port_3BA = IOPort(0x3BA, 0x00, defaultReader_FF, nullptr);
IOPort port_3BC = IOPort(0x3BC, 0x00, defaultReader_FF, nullptr);
IOPort port_3BD = IOPort(0x3BD, 0x00, defaultReader_FF, nullptr);
IOPort port_3BE = IOPort(0x3BE, 0x00, defaultReader_FF, nullptr);
IOPort port_3BF = IOPort(0x3BF, 0x00, defaultReader_FF, nullptr);

IOPort port_3C0 = IOPort(0x3C0, 0x00, defaultReader_FF, nullptr);
IOPort port_3C2 = IOPort(0x3C2, 0x00, defaultReader_FF, nullptr);
IOPort port_3C3 = IOPort(0x3C3, 0x00, defaultReader_FF, nullptr);
IOPort port_3C4 = IOPort(0x3C4, 0x00, defaultReader_FF, nullptr);
IOPort port_3C5 = IOPort(0x3C5, 0x00, defaultReader_FF, nullptr);
IOPort port_3CB = IOPort(0x3CB, 0x00, defaultReader_FF, nullptr);
IOPort port_3CC = IOPort(0x3CC, 0x00, defaultReader_FF, nullptr);

//////////////////////////////////////////////////////////////////////////// Local function prototypes
void setup(void);
void SDL_DumpVGA(void);

///////////////////////////////////////////////////////////////////////// External function prototypes
extern void VideoThreadPoll(void);
extern void draw(void);

uint32_t speed = 0;

void LoadCOMFlash(const unsigned char *ptr, int auxSize, int seg_load) {
    int dir_load = seg_load * 16;
    for (int i = 0; i < auxSize; i++) {
        write86((dir_load + 0x100 + i), ptr[i]);
    }
    SetRegCS(seg_load);
    SetRegDS(seg_load);
    SetRegES(seg_load);
    SetRegSS(seg_load);
    SetRegIP(0x100); // 0x100;

    SetRegSP(0);
    SetRegBP(0);
    SetRegSI(0);
    SetRegDI(0);
    SetCF(0);
}

void inithardware() {
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
    initVideoPorts();
    inittiming();
    initscreen();
}

void PerformSpecialActions() {
    if (gb_reset == 1) {
        gb_reset = 0;
        ClearRAM();
        memset(gb_video_cga, 0, 16384);
        keyboard->Reset();
        running = 1;
        reset86();
        inithardware();
        return;
    }
    if (gb_force_load_com == 1) {
        gb_force_load_com = 0;
        int auxOffs = 0;
        if (gb_list_seg_load[gb_id_cur_com] == 0)
            auxOffs = 0x07C0;
        else
            auxOffs = 0x0051;
        LoadCOMFlash(gb_list_com_data[gb_id_cur_com], gb_list_com_size[gb_id_cur_com], auxOffs);
        return;
    }
    renderExec();
}

//****************************
void ClearRAM() {
    int i;
    for (i = 0; i < gb_max_ram; i++) {
        write86(i, 0);
    }
}

//****************************
void CreateRAM() {
    for (uint32_t nIndex = 0; nIndex < PAGE_COUNT; nIndex++) {
        unsigned char *pPage = (unsigned char *)heap_caps_malloc(PAGE_SIZE, MALLOC_CAP_SPIRAM);
        memset(pPage, 0, PAGE_SIZE);
        gb_ram_bank[nIndex] = pPage;
    }
}

// Setup principal
void setup() {
    pinMode(SPEAKER_PIN, OUTPUT);
    digitalWrite(SPEAKER_PIN, LOW);

#ifdef use_lib_log_serial
    Serial.begin(115200);
    Serial.printf("\nHEAP BEGIN %d\n", ESP.getFreeHeap());
#endif
    sdcard.Init();
    CreateRAM();
    ClearRAM();
    updateBIOSDataArea(); // Al inicio
    FuerzoParityRAM();    // Fuerzo que Parity sea en RAM

    renderInit();
    LOG("VGA %d\n", ESP.getFreeHeap());
    keyboard->Init();

    running = 1;
    reset86();
    LOG("OK!\n");

    inithardware();

#ifndef use_lib_singlecore
    // BEGIN TASK video
    vidQueue = xQueueCreate(1, sizeof(uint16_t *));
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
void videoTask(void *unused) {
    videoTaskIsRunning = true;
    uint16_t *param;
    while (1) {
        xQueuePeek(vidQueue, &param, portMAX_DELAY);
        if ((int)param == 1)
            break;
        draw();
        xQueueReceive(vidQueue, &param, portMAX_DELAY);
        videoTaskIsRunning = false;
    }
    videoTaskIsRunning = false;
    vTaskDelete(NULL);

    while (1) {
    }
}
#endif

unsigned char gb_cpunoexe = 0;
unsigned int gb_cpunoexe_timer_ini;
unsigned int tiempo_ini_cpu, tiempo_fin_cpu;
unsigned int tiene_que_tardar = 0;

// Loop main
void loop()
{
    static uint32_t gb_cpu_timer_before;

    stats.StartIteration();
    execCPU();
    stats.CountCPUTime();
    execKeyboard();
    execMisc();
    execVideo();

    const uint32_t gb_cpu_timer_cur = millis();
    if ((gb_cpu_timer_cur - gb_cpu_timer_before) > 1000)
    {
        gb_cpu_timer_before = gb_cpu_timer_cur;
        stats.PrintAndReset();
    }

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
            IOPortSpace::getInstance().write(0x060, scancode);
            uint8_t val = IOPortSpace::getInstance().read(0x064);
            IOPortSpace::getInstance().write(0x064, val |= 2);
            doirq(1);
            Serial.printf("key: 0x%02x\n", scancode);
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
#else
    static uint16_t *param;
    xQueueSend(vidQueue, &param, portMAX_DELAY);
#endif
}

void execMisc()
{
    static uint32_t before;
    const uint32_t now = millis();
    if ((now - before) > gb_keyboard_poll_milis)
    {
        PerformSpecialActions();
        do_tinyOSD();
    }
}