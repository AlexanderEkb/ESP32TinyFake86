#ifndef _IO_AUDIO_H_
#define _IO_AUDIO_H_

#include <stdint.h>

class Audio
{
  public:
    static void init();
    static void playSample(uint8_t sample);
    static void driveSpeaker(bool val);
  private:
    static uint32_t covox;
    static uint32_t speaker;
    static void updatePWM();
};


#endif /* _IO_AUDIO_H_ */