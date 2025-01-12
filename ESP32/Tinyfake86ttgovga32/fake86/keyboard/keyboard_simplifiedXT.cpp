#include "config/hardware.h"

#if (KEYBOARD_DRIVER == 0)

#include "keyboard_simplifiedXT.h"
#include "mb/i8259.h"
#include "keyboard/keys.h"
#include <Arduino.h>
#include <esp32-hal-gpio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"


QueueHandle_t KeyboardDriverSimplifiedXT::q;

void IRAM_ATTR KeyboardDriverSimplifiedXT::onExti()
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
    OnKey(shifter);
  }
}

KeyboardDriverSimplifiedXT::KeyboardDriverSimplifiedXT() {
  q = xQueueCreate(16, 1);
}

void KeyboardDriverSimplifiedXT::Init()
{
  pinMode(KEYBOARD_DATA, INPUT_PULLUP);
  pinMode(KEYBOARD_CLK, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(KEYBOARD_CLK), onExti, FALLING);
}

void KeyboardDriverSimplifiedXT::Reset()
{
  xQueueReset(q);
}

uint8_t KeyboardDriverSimplifiedXT::Poll()
{
  uint8_t result;
  if(xQueueReceive(q, &result, 0) != pdTRUE)
    result = 0;
  return result;
}

void KeyboardDriverSimplifiedXT::OnKey(uint8_t scancode)
{
  portBASE_TYPE foo;
  xQueueSendFromISR(q, &scancode, &foo);
}

#endif /* KEYBOARD_DRIVER */