#ifndef __MACHINE_XT__
#define __MACHINE_XT__

#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <Ticker.h>

#include "io/keyboard.h"
#include "io/drive.h"
#include "mb/memory.h"
#include "stats.h"

class MachineXT_t
{
  public:
    MachineXT_t(KeyboardDriver * keyboard) :
      ticker(Ticker()),
      keyboard(keyboard),
      videoTaskHandle(nullptr),
      stats(Stats())
    {};
    void init();
    void run();
    void suspend();
    void resume();

    Memory_t memory;

    // To boot the machine we must have these two methods public:
    void boot();
    uint8_t getBootDrive();
    void __attribute__((optimize("-Ofast"))) IRAM_ATTR readdisk(DISK_ADDR &src, MEM_ADDR &dst);

    // CRUTCH!! Remove ASAP!!!
    KeyboardDriver * getKeyboard() {return keyboard;};
  private:
    MachineXT_t();
    static const uint32_t SAMPLE_RATE = 16000;
    static const uint32_t KEYB_POLL_PERIOD_ms = 20;
    KeyboardDriver *keyboard;
    TaskHandle_t videoTaskHandle;
    Ticker ticker;
    Stats stats;

    FloppyDrive_t driveA;
    FloppyDrive_t driveB;
    HDD_t driveC;
    static const uint32_t DRIVE_COUNT = 3;
    Drive_t * drives[DRIVE_COUNT] = {&driveA, &driveB, &driveC};
    uint8_t lastDiskIOResult = 0;


    void execKeyboard();
    
    void diskInit(void);
    void diskhandler();
    void writedisk (DISK_ADDR & dst, MEM_ADDR & src);
    void setDiskIOResult(uint8_t _result);
    void getDriveParameters(uint8_t drive);
};

#endif /* __MACHINE_XT__ */