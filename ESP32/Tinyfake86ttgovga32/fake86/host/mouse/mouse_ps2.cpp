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
QueueHandle_t MousePs2_t::foo;
xTaskHandle MousePs2_t::mainTask;
MousePs2_t::State_t MousePs2_t::state;

MousePs2_t::MousePs2_t()
{
  q = xQueueCreate(16, 1);
  foo = xQueueCreate(16, 1);
  state = UNKNOWN;
}

MousePs2_t::~MousePs2_t()
{
  vQueueDelete(q);
}

void MousePs2_t::init()
{
  mainTask = xTaskGetCurrentTaskHandle();
  xTaskHandle stub;
  if(xTaskCreate(mouseInitTask, "mouseInit", 2048, reinterpret_cast<void * const>(this), tskIDLE_PRIORITY + 2, &stub) != pdPASS)
  {
    ESP_LOGE(TAG, "Unable to initialize mouse!");
  }
  else vTaskSuspend(mainTask);
}

bool MousePs2_t::poll(MouseEvent_t * e)
{
  return false;
}

void MousePs2_t::onMouseExti()
{
  static uint32_t shifter = 0;
  static uint8_t bitcount = 0;
  static uint32_t prev_ms = 0;
  uint32_t now_us;
  uint8_t n, val;

  int clock = digitalRead(PS2_MOUSE_CLK);
  if (clock == 1)
    return;

  val = digitalRead(PS2_MOUSE_DATA);
  now_us = micros();
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
    switch (state)
    {
      case RUNNING:
        xQueueSendFromISR(q, &scancode, &bar);
        break;
      default:
        xQueueSendFromISR(foo, &scancode, &bar);
        break;
    }
  }
}

void MousePs2_t::reset()
{
  state = RESET;
  delay(100);
  uint32_t r = sendByte(0xFF);
  if(r == RESULT_OK)
    ESP_LOGI(TAG, "rst ok");
  else
    ESP_LOGE(TAG, "rst failed");
}

void MousePs2_t::setMode()
{
  state = SET_MODE;
  uint32_t r = sendByte(0xEA);
  if(r == RESULT_OK)
    ESP_LOGI(TAG, "mod ok");
  else
    ESP_LOGE(TAG, "mod failed");
}

void MousePs2_t::enable()
{
  uint32_t r = sendByte(0xF4);
  if(r == RESULT_OK)
    ESP_LOGI(TAG, "ena ok");
  else
    ESP_LOGE(TAG, "ena failed");
  state = STARTUP;
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
  // uint16_t word = (d << 1) & 0x01FE;
  // word |= parity ? (0 << 9) : (1 << 9);
  // word |= 1 << 10;
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

void MousePs2_t::mouseInitTask(void * p)
{
  uint8_t byte;
  uint32_t rst;
  while(state != RUNNING)
  {
    if(state == UNKNOWN)
    {
      rst = 0;
      reset();        
    } else {
      if(xQueueReceive(foo, &byte, 1000) != pdPASS)
      {
        ESP_LOGE(TAG, "timeout");
        state = UNKNOWN;
      } else {
        ESP_LOGI(TAG, "recv 0x%02X", byte);
        switch(state)
        {
          case RESET:
            if((rst == 0) && (byte == 0xFA))
              rst = 1;
            else if((rst == 1) && (byte == 0xAA))
            {
              rst = 2;
            } else if ((rst == 2)) {
              setMode();
            }
            else {
              ESP_LOGE(TAG, "rst=%lu byte=0x%02X", rst, byte);
              rst = 0;
              reset();
            }
            break;
          case SET_MODE:
            if(byte == 0xFA)
              enable();
            else
            {
              rst = 0;
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
              rst = 0;
              reset();
            }
            break;
        }
      }
    }
  }
  xTaskHandle stub;
  if(xTaskCreate(mouseDbgTask, "mouseDbg", 2048, nullptr, tskIDLE_PRIORITY, &stub) != pdPASS)
  {
    ESP_LOGE(TAG, "Unable to initialize mouse!");
  }
  vQueueDelete(foo);
  vTaskDelete(xTaskGetCurrentTaskHandle());
  vTaskResume(mainTask);
}

void MousePs2_t::mouseDbgTask(void * p)
{
  uint8_t byte;
  while(true)
  {
    xQueueReceive(q, &byte, portMAX_DELAY);
    ESP_LOGI(TAG, "recv 0x%02X", byte);
  }
}