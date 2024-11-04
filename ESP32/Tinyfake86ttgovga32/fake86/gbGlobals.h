#ifndef _GB_GLOBALS_H
#define _GB_GLOBALS_H
#include <stdint.h>
#include <Arduino.h>
#include "config/gbConfig.h"

#ifdef use_lib_log_serial
#define LOG(...) Serial.printf(__VA_ARGS__)
#else
#define LOG(...) (void)(__VA_ARGS__)
#endif

extern unsigned char cf;

extern unsigned short int segregs[4];
static const size_t      RAM_SIZE  = 640 * 1024;

extern volatile bool speakerMute;

#endif
