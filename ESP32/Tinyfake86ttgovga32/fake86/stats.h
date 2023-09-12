#ifndef STATS_H
#define STATS_H

#include <stdint.h>
#include "esp32-hal.h"

#ifdef use_lib_log_serial
#define LOG(...) Serial.printf(__VA_ARGS__)
#else
#define LOG(...) (void)
#endif

class Stats {
  public:
    Stats()
    {

    };

    void Initialize() {

    };

    void StartIteration() {jj_ini_cpu = micros();};

    void CountCPUTime() {
      jj_end_cpu = micros();
      gb_cur_cpu_ticks = (jj_end_cpu - jj_ini_cpu);
      uint32_t total_tiempo_ms_cpu = gb_cur_cpu_ticks / 1000;
      if (gb_cur_cpu_ticks > gb_max_cpu_ticks)
          gb_max_cpu_ticks = gb_cur_cpu_ticks;
      if (gb_cur_cpu_ticks < gb_min_cpu_ticks)
          gb_min_cpu_ticks = gb_cur_cpu_ticks;
    };

    void PrintAndReset() {
      LOG("c:%u m:%u mx:%u\n", gb_cur_cpu_ticks, gb_min_cpu_ticks, gb_max_cpu_ticks);
      gb_min_cpu_ticks = 1000000;
      gb_max_cpu_ticks = 0;
      gb_cur_cpu_ticks = 0;
    }

  private:
    uint32_t jj_ini_cpu;
    uint32_t jj_end_cpu;
    uint32_t jj_ini_vga;
    uint32_t jj_end_vga;
    uint32_t gb_max_cpu_ticks;
    uint32_t gb_min_cpu_ticks;
    uint32_t gb_cur_cpu_ticks;
    uint32_t gb_max_vga_ticks;
    uint32_t gb_min_vga_ticks;
    uint32_t gb_cur_vga_ticks;

};

#endif /** STATS_H */