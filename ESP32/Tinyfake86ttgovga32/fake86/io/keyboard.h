#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "fake86.h"
#include "config/gbConfig.h"
#include "gbGlobals.h"
#include "config/hardware.h"
#include "mb/i8259.h"
#include "io/keys.h"
#include <Arduino.h>
#include <esp32-hal-gpio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

void IRAM_ATTR kb_interruptHandler(void);
uint8_t getScancode(void);

class KeyboardDriver {
  public:
    virtual void Init() = 0;
    virtual void Reset() = 0;
    virtual uint8_t Poll() = 0;
    virtual uint8_t getLastKey() = 0;
    virtual void resetRdy();
};

class KeyboardDriverSTM : public KeyboardDriver
{
  public:
    static const uint32_t KEY_COUNT = 53;
    KeyboardDriverSTM() {
      lastKey  = 0;
      q = xQueueCreate(16, 1);
    }
    virtual void Init() {
      pinMode(KEYBOARD_DATA, INPUT_PULLUP);
      pinMode(KEYBOARD_CLK, INPUT_PULLUP);
      // pinMode(KEYBOARD_RDY, OUTPUT_OPEN_DRAIN);
      // digitalWrite(KEYBOARD_DATA, true);
      // digitalWrite(KEYBOARD_CLK, true);
      // digitalWrite(KEYBOARD_RDY, true);
      attachInterrupt(digitalPinToInterrupt(KEYBOARD_CLK), kb_interruptHandler, FALLING);
    }

    virtual void Reset() {
      lastKey = 0;
      resetRdy();
      xQueueReset(q);
    }

    virtual uint8_t getLastKey() {
      uint8_t result = lastKey;
      lastKey = 0;
      resetRdy();
      return result;
    }

    uint8_t Poll() {
      uint8_t result;
      if(xQueueReceive(q, &result, 0) != pdTRUE)
        result = 0;
      return result;
    }

    virtual void resetRdy()
    {
      // digitalWrite(KEYBOARD_RDY, true);
    }

  private:
    static uint8_t lastKey;
    static QueueHandle_t q;
    static void OnKey(uint8_t scancode) {
      portBASE_TYPE foo;
      xQueueSendFromISR(q, &scancode, &foo);
      if(!(scancode & 0x80))
      {
        lastKey = scancode;
      }
    }
    friend void IRAM_ATTR kb_interruptHandler(void);
    /*
    // https://homepages.cwi.nl/~aeb/linux/kbd/scancodes-1.html
    //  00    01    02    03    04    05    06    07    08    09    0a    0b    0c    0d    0e    0f
    // (ERR)  ESC   1!    2@    3#    4$    5%    6^    7&    8*    9(    0)    -_    =+    BS    Tab

    //  10    11    12    13    14    15    16    17    18    19    1a    1b    1c    1d    1e    1f
    //  Q     W     E     R     T     Y     U     I     O     P     [{    ]}  Enter  LCtrl  A     S

    //  20    21    22    23    24    25    26    27    28    29    2a    2b    2c    2d    2e    2f
    //  D     F     G     H     J     K     L     ;:    '"    `~  LShift  \|    Z     X     C     V

    //  30    31    32    33    34    35    36    37    38    39    3a    3b    3c    3d    3e    3f
    //  B     N     M     ,<    .>    /?  RShift KP-*  LAlt Space  Caps   F1    F2    F3    F4    F5
    //                                   or (* / PrtScn)
    //                                   on a 83/84-key

    //  40    41    42    43    44    45    46    47    48    49    4a    4b    4c    4d    4e    4f
    //  F6    F7    F8    F9    F10 NumLk ScrlLk KP-7  KP-8  KP-9  KP--  KP-4  KP-5  KP-6  KP-+  KP-1
    //                                           Home   Up   PgUp        Left        Right       End
    //
    //  50    51    52    53    54    55    56    57    58    59    5a    5b    5c    5d    5e    5f
    // KP-2  KP-3  KP-0  KP-.  Alt-  (??)  (??)  F11   (??)  (??)  (??)  (??)  (??)  (??)  (??)  (??)
    // Down  PgDn   Ins   Del SysRq
    //
    //  58
    // F12
        */
    //*********************************************
};

#endif /* KEYBOARD_H */