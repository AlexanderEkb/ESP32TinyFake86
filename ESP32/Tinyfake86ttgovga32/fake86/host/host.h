#ifndef __HOST_HOST_H__
#define __HOST_HOST_H__

#include "config/config.h"

#include "audio/audio.h"

#if (KEYBOARD_DRIVER == 0)
#include "keyboard/keyboard_simplifiedXT.h"
#elif (KEYBOARD_DRIVER == 1)
#include "keyboard/keyboard_AT.h"
#else
#error "Choose any supported keyboard driver!"
#endif

#if (MOUSE_DRIVER == 0)
#include "mouse/mouse_ps2.h"
#else
#error "Choose any supported mouse driver!"
#endif

#if (VIDEO_DRIVER == 0)
#include "video/compositeNtsc/compositeNtsc.h"
#else
#error "Choose any supported video driver!"
#endif


class Host {
  public:
    static Audio_t * audio;
    static Video_t * video;
    static Keyboard_t * keyboard;
    static Mouse_t * mouse;
    static void init();
    static void run();
};

#endif /* __HOST_HOST_H__ */