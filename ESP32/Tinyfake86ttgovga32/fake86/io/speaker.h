#ifndef SPEAKER_H
#define SPEAKER_H

#include <stdint.h>

class Speaker_t
{
  public:
    static Speaker_t & getInstance() {return instance;};
    static void driveByTimer(bool state);
    static void driveDirectly(bool state);
    static void mute();
    static void unmute();
  private:
    static bool PB1;
    static bool Ch2;
    static bool muted;
    static Speaker_t instance;
};

#endif /* SPEAKER_H */