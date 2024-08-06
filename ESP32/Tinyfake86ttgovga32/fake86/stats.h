#ifndef STATS_H
#define STATS_H

#include <stdint.h>
#include "config/config.h"
#include "esp_timer.h"

#ifdef STATS_ON
#define PRINT_STATS(...) Serial.printf(__VA_ARGS__)
#else
#define PRINT_STATS(...) (void)(__VA_ARGS__)
#endif

class Stats
{
  public:
  Stats(){

  };

  void initialize(){

  };

  void exec()
  {
    const uint32_t now = esp_timer_get_time() / 1000U;
    if ((now - timestampExec_ms) > PERIOD_ms)
    {
      timestampExec_ms = now;
      printAndReset();
    }
  };

  void startIteration() { timestamp = esp_timer_get_time(); };

  void countCPUTime()
  {
    uint32_t now = esp_timer_get_time();
    CPU_TIME.instant = (now - timestamp);
    if (CPU_TIME.instant > CPU_TIME.max)
      CPU_TIME.max = CPU_TIME.instant;
    if (CPU_TIME.instant < CPU_TIME.min)
      CPU_TIME.min = CPU_TIME.instant;
  };

  void printAndReset()
  {
    PRINT_STATS("c:%u m:%u mx:%u\n", CPU_TIME.instant, CPU_TIME.min, CPU_TIME.max);
    CPU_TIME.min = 1000000;
    CPU_TIME.max = 0;
    CPU_TIME.instant = 0;
  }

  private:
  static const uint32_t PERIOD_ms = 1000;
  struct CPU_TIME
  {
    uint32_t instant;
    uint32_t max;
    uint32_t min;
  } CPU_TIME;
  uint32_t timestamp;
  uint32_t timestampExec_ms;
};

#endif /** STATS_H */