#ifndef __MOUSE_PS2_H__
#define __MOUSE_PS2_H__

#include "mouse.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

class MousePs2_t : public Mouse_t
{
  public:
    static const uint32_t RESULT_OK       = 0;
    static const uint32_t RESULT_ERROR    = 1;
    static const uint32_t RESULT_TIMEOUT  = 2;
    MousePs2_t();
    virtual ~MousePs2_t() override;
    virtual void init() override;
  private:
    typedef enum {
      UNKNOWN,
      RESET,
      SET_MODE,
      STARTUP,
      RUNNING
    } State_t;
    static State_t state;
    static QueueHandle_t q;
    static void reset();
    static void setMode();
    static void enable();
    static uint32_t sendByte(uint8_t d);
    static uint32_t sendBit(uint8_t b);
    static void IRAM_ATTR onMouseExti();
    // static void mouseInitTask(void * p);
    static void mouseTask(void * p);
};

#endif /* __MOUSE_PS2_H__ */