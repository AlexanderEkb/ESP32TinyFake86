#ifndef SPEAKER_H
#define SPEAKER_H

#include <stdint.h>

#ifdef use_lib_speaker_cpu
#define SAMPLE_RATE 10000
#else
#define SAMPLE_RATE 16000
#endif

void onPort0x61Write(uint8_t val);
void my_callback_speaker_func();

#endif /* SPEAKER_H */