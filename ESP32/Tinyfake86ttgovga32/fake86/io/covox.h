#ifndef _IO_COVOX_H_
#define _IO_COVOX_H_

#include <stdint.h>

class Covox_t
{
  public:
    static Covox_t & getInstance()
    {
      return instance;
    };
    static void init();
    static void playSample(uint8_t sample);
    static void driveSpeaker(bool val);
  private:
    Covox_t()
    {

    };
    static Covox_t instance;
    static uint32_t covox;
    static uint32_t speaker;
    static void updatePWM();
};


#endif /* _IO_COVOX_H_ */