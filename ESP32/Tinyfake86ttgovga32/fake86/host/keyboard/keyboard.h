#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

class Keyboard_t {
  public:
    virtual void init() = 0;
    virtual void Reset() = 0;
    virtual uint8_t Poll() = 0;
};

#endif /* KEYBOARD_H */