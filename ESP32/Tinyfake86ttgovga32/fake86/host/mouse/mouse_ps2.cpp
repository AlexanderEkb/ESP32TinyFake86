#include <Arduino.h>
#include <esp32-hal-gpio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <esp_log.h>
#include "mouse_ps2.h"
#include "../config/config.h"

#define TAG "MOUSE"

QueueHandle_t MousePs2_t::q;

MousePs2_t::MousePs2_t()
{
  q = xQueueCreate(16, 1);
}

void MousePs2_t::init()
{
  pinMode(PS2_MOUSE_DATA, INPUT_PULLUP);
  pinMode(PS2_MOUSE_CLK, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PS2_MOUSE_CLK), onMouseExti, FALLING);
  ESP_LOGI(TAG, "PS/2 mouse interface init ok");
}

bool MousePs2_t::poll(MouseEvent_t * e)
{

}

void MousePs2_t::onMouseExti()
{
  static uint32_t shifter = 0;
  static uint8_t bitcount = 0;
  static uint32_t prev_ms = 0;
  uint32_t now_ms;
  uint8_t n, val;

  int clock = digitalRead(PS2_MOUSE_CLK);
  if (clock == 1)
    return;

  val = digitalRead(PS2_MOUSE_DATA);
  now_ms = millis();
  if (now_ms - prev_ms > 5) {
    bitcount = 0;
    shifter = 0;
  }
  prev_ms = now_ms;

  shifter >>= 1;
  shifter |= (val ? 0x400 : 0);
  bitcount++;
  static const uint32_t WORDLENGTH = 11;
  if (bitcount == WORDLENGTH) {
    bitcount = 0;
    portBASE_TYPE foo;
    const uint8_t scancode = static_cast<uint8_t>((shifter >> 1) & 0xFF);
    if(scancode != 0)
    {
      xQueueSendFromISR(q, &scancode, &foo);
      ESP_LOGI(TAG, "Received 0x%02X", scancode);
    }
  }
}