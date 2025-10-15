#include <Arduino.h>
#include <esp32-hal-gpio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <esp_log.h>
#include "mouse_ps2.h"
#include "../config/config.h"

#define TAG "MOUSE"

QueueHandle_t MousePs2_t::q;
MousePs2_t::State_t MousePs2_t::state;

MousePs2_t::MousePs2_t()
{
  q = xQueueCreate(128, 1);
  state = UNKNOWN;
}

MousePs2_t::~MousePs2_t()
{
  vQueueDelete(q);
}

void MousePs2_t::init()
{
  uint8_t byte;
  uint32_t rstRespCount;
  uint32_t attemptCount = 0;
  static uint32_t const MAX_ATTEMPTS = 3;

  static uint32_t const RST_RESP_LENGTH = 3;
  static uint8_t const RST_RESP[RST_RESP_LENGTH] = {0xFA, 0xAA, 0x00};

  while(state != RUNNING)
  {
    if(state == UNKNOWN)
    {
      if(++attemptCount >= MAX_ATTEMPTS)
      {
        ESP_LOGE(TAG, "Mouse not found");
        return;
      }
      rstRespCount = 0;
      reset();     
      continue;   
    }

    if(xQueueReceive(q, &byte, 1000) != pdPASS)
    {
      // ESP_LOGE(TAG, "timeout");
      state = UNKNOWN;
      continue;
    }

    // ESP_LOGI(TAG, "recv 0x%02X", byte);
    switch(state)
    {
      case RESET:
        if(byte != RST_RESP[rstRespCount++])
        {
          rstRespCount = 0;
          reset();
        }
        else if(rstRespCount == RST_RESP_LENGTH)
          setMode();
        break;
      case SET_MODE:
        if(byte == 0xFA)
          enable();
        else
        {
          rstRespCount = 0;
          reset();
        }
        break;
      case STARTUP:
        if(byte == 0xFA)
        {
          state = RUNNING;
          ESP_LOGI(TAG, "PS/2 mouse interface init ok");
        } else
        {
          rstRespCount = 0;
          reset();
        }
        break;
    }
  }

  xTaskHandle stub;
  if(xTaskCreate(mouseTask, "mouse", 2048, reinterpret_cast<void * const>(this), tskIDLE_PRIORITY, &stub) != pdPASS)
  {
    ESP_LOGE(TAG, "Unable to initialize mouse!");
  }
}

void IRAM_ATTR MousePs2_t::onMouseExti()
{
  static uint32_t shifter = 0;
  static uint8_t bitcount = 0;
  static uint32_t prev_ms = 0;

  int clock = digitalRead(PS2_MOUSE_CLK);
  if (clock != 0)
    return;

  uint8_t val = digitalRead(PS2_MOUSE_DATA);
  uint32_t const now_us = micros();
  if (now_us - prev_ms > 120) {
    bitcount = 0;
    shifter = 0;
  }
  prev_ms = now_us;

  shifter >>= 1;
  shifter |= (val ? 0x400 : 0);
  bitcount++;
  static const uint32_t WORDLENGTH = 11;
  if (bitcount == WORDLENGTH) {
    bitcount = 0;
    portBASE_TYPE bar;
    const uint8_t scancode = static_cast<uint8_t>((shifter >> 1) & 0xFF);
    xQueueSendFromISR(q, &scancode, &bar);
    portYIELD_FROM_ISR(bar);
  }
}

void MousePs2_t::reset()
{
  state = RESET;
  delay(100);
  uint32_t r = sendByte(0xFF);
}

void MousePs2_t::setMode()
{
  state = SET_MODE;
  uint32_t r = sendByte(0xEA);
}

void MousePs2_t::enable()
{
  state = STARTUP;
  uint32_t r = sendByte(0xF4);
}

uint32_t MousePs2_t::sendByte(uint8_t d)
{
  // Calculating parity
  uint8_t t = d;
  bool parity = false;
  for(uint32_t i=0; i<8; i++)
  {
    parity ^= (t & 0x01 == 0x01);
    t >>= 1;
  }
  // Composing the word to be sent
  uint16_t word = d;
  word |= parity ? (0 << 8) : (1 << 8);
  word |= 1 << 9;

  detachInterrupt(digitalPinToInterrupt(PS2_MOUSE_CLK));

  digitalWrite(PS2_MOUSE_CLK, 0);
  digitalWrite(PS2_MOUSE_DATA, 1);
  pinMode(PS2_MOUSE_DATA, OUTPUT_OPEN_DRAIN);
  pinMode(PS2_MOUSE_CLK, OUTPUT_OPEN_DRAIN);
  delayMicroseconds(120);
  digitalWrite(PS2_MOUSE_DATA, 0);
  delayMicroseconds(120);
  pinMode(PS2_MOUSE_CLK, INPUT_PULLUP);

  static const uint32_t WORDLENGTH = 10;
  for(uint32_t i=0; i<WORDLENGTH; i++)
  {
    const uint8_t bit = (word & 0x01);
    uint32_t const r = sendBit(bit);
    if(r != RESULT_OK)
    {
      return r;
    }
    word >>= 1;
  }

  pinMode(PS2_MOUSE_DATA, INPUT_PULLUP);
  pinMode(PS2_MOUSE_CLK, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PS2_MOUSE_CLK), onMouseExti, FALLING);

  return RESULT_OK;
}

uint32_t MousePs2_t::sendBit(uint8_t b)
{
  static uint32_t const TIMEOUT = 1000;
  uint64_t now = micros();
  while(digitalRead(PS2_MOUSE_CLK) != 0)
  {
    if((micros() - now) > TIMEOUT)
      return RESULT_TIMEOUT;
  }
  digitalWrite(PS2_MOUSE_DATA, b);
  now = micros();
  while(digitalRead(PS2_MOUSE_CLK) == 0)
    {
    if((micros() - now) > TIMEOUT)
      return RESULT_TIMEOUT;
  }

  return RESULT_OK;
}

void MousePs2_t::mouseTask(void * p)
{
  int8_t byte;
  static uint32_t const PKT_LENGTH = 3;
  int8_t bytes[PKT_LENGTH];
  uint32_t ptr = 0;

  static uint32_t lastRX;
  static uint32_t const TIMEOUT_ms = 10;
  while(true)
  {
    xQueueReceive(q, &byte, portMAX_DELAY);
    // ESP_LOGI(TAG, "recv 0x%02X", byte);

    uint32_t const now = millis();
    if(now > lastRX + TIMEOUT_ms)
      ptr = 0;
    lastRX = now;
    bytes[ptr++] = byte;
    if(ptr >= PKT_LENGTH)
    {
      ptr = 0;
      MousePs2_t * instance = reinterpret_cast<MousePs2_t * const>(p);
      if(instance->sink != nullptr)
      {
        uint8_t const ZERO_BITS = 0xC0;
        uint8_t const ONE_BIT = 0x08;
        if(((bytes[0] & ZERO_BITS) == 0) && ((bytes[0] & ONE_BIT) == ONE_BIT))
        {
          // ESP_LOGI(TAG, "0x%02X", static_cast<uint8_t>(bytes[0]));
          int32_t const dx = bytes[1];
          int32_t const dy = bytes[2];
          uint8_t const btn = (bytes[0] & BTN_MASK);
          instance->sink->onMouseEvent(dx, dy, btn);
        }
      }
    }
  }
}