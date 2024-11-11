#include "keyboard.h"
#include "esp32-hal-gpio.h"

#define TAG "KB"

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

uint8_t KeyboardDriverCustom_t::lastKey = 0;
QueueHandle_t KeyboardDriverCustom_t::q;

void IRAM_ATTR kb_interruptHandler(void)
{
  static uint8_t shifter = 0;
  static uint8_t bitcount = 0;
  static uint32_t prev_ms = 0;
  uint32_t now_ms;
  uint8_t n, val;

  int clock = digitalRead(KEYBOARD_CLK);
  if (clock == 1)
    return;

  val = digitalRead(KEYBOARD_DATA);
  now_ms = millis();
  if (now_ms - prev_ms > 5) {
    bitcount = 0;
    shifter = 0;
  }
  prev_ms = now_ms;

  shifter <<= 1;
  shifter |= val;
  bitcount++;
  if (bitcount == 8) {
    bitcount = 0;
    KeyboardDriverCustom_t::OnKey(shifter);
  }
}

KeyboardDriverCustom_t::KeyboardDriverCustom_t(QueueHandle_t queue)
{
  q = queue;
  lastKey  = 0;
}

void KeyboardDriverCustom_t::Init() {
  pinMode(KEYBOARD_DATA, INPUT_PULLUP);
  pinMode(KEYBOARD_CLK, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(KEYBOARD_CLK), kb_interruptHandler, FALLING);
}

void KeyboardDriverCustom_t::Reset() {
  lastKey = 0;
  xQueueReset(q);
}

uint8_t KeyboardDriverCustom_t::getLastKey() {
  uint8_t result = lastKey;
  lastKey = 0;
  return result;
}

void KeyboardDriverCustom_t::OnKey(uint8_t scancode)
{
  portBASE_TYPE foo;
  xQueueSendFromISR(q, &scancode, &foo);
  if(!(scancode & 0x80))
  {
    lastKey = scancode;
  }
  if(foo)
  {
    portYIELD_FROM_ISR ();
  }
}
