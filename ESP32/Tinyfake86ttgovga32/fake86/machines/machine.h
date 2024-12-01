#ifndef __MACHINES_MACHINE_H__
#define __MACHINES_MACHINE_H__

#include <stdint.h>
#include "../host/keyboard/keyboard.h"

typedef enum Event_t
{
  EVENT_KEY,
} Event_t;

typedef struct Message_t
{
  Message_t(Event_t event, uint32_t param) :
    event(event), param(param) {};
  Event_t event;
  uint32_t param;
} Message_t;

class Machine_t
{
  public:
    virtual void init() = 0;
    virtual void run() = 0;
    virtual void suspend() = 0;
    virtual void resume() = 0;
    virtual void onEvent(Message_t * msg) = 0;

    virtual Keyboard_t * getKeyboard() = 0;
};

#endif /* __MACHINES_MACHINE_H__ */