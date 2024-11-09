#ifndef STATS_H
#define STATS_H

#include "esp32-hal.h"
#include <stdint.h>
#include "config.h"

#define TAG "STATS"

class Stats
{
  public:
  Stats(){

  };

  void initialize(){

  };

  void exec()
  {
    const uint32_t now = millis();
    if ((now - timestampExec_ms) > PERIOD_ms)
    {
      timestampExec_ms = now;
      printAndReset();
    }
  };

  void startIteration() { timestamp = micros(); };

  void countCPUTime()
  {
    uint32_t now = micros();
    CPU_TIME.instant = (now - timestamp);
    if (CPU_TIME.instant > CPU_TIME.max)
      CPU_TIME.max = CPU_TIME.instant;
    if (CPU_TIME.instant < CPU_TIME.min)
      CPU_TIME.min = CPU_TIME.instant;
  };

  void printAndReset()
  {
  #ifdef STATS_ON
    ESP_LOGI(TAG, "c:%u m:%u mx:%u", CPU_TIME.instant, CPU_TIME.min, CPU_TIME.max);
    CPU_TIME.min = 1000000;
    CPU_TIME.max = 0;
    CPU_TIME.instant = 0;
  #endif
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