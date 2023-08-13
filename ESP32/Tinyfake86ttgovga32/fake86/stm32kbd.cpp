#include "stm32kbd.h"
#include "esp32-hal-gpio.h"
#include "hardware.h"
#include <Arduino.h>

uint32_t bitCounter;
uint8_t scancode;
uint32_t lastCall;
bool bEvent;

static void IRAM_ATTR clkIntHandler();

void stm32keyboard::Init() {
    pinMode(KEYBOARD_DATA, INPUT_PULLUP);
    pinMode(KEYBOARD_CLK, INPUT_PULLUP);
    digitalWrite(KEYBOARD_DATA, true);
    digitalWrite(KEYBOARD_CLK, true);
    attachInterrupt(digitalPinToInterrupt(KEYBOARD_CLK), clkIntHandler, FALLING);
}

void stm32keyboard::Reset()
{
    bitCounter = 0;
    scancode = 0;
    lastCall = 0;
    bEvent = false;
}

void stm32keyboard::Exec()
{
    if (bEvent) {
        gb_portramTiny[fast_tiny_port_0x60] = scancode;
        gb_portramTiny[fast_tiny_port_0x64] |= 2;
        doirq(1);
        bEvent = false;
    }
}

static void IRAM_ATTR clkIntHandler()
{
    int clock = digitalRead(KEYBOARD_CLK);
    if (clock == 1)
        return;

    const uint32_t now_ms = millis();
    if (now_ms - lastCall > 250) {
        bitCounter = 0;
        scancode = 0;
    }
    lastCall = now_ms;

    uint8_t bit = digitalRead(KEYBOARD_DATA);
    scancode <<= 1;
    scancode |= bit;
    bitCounter++;

    if (bitCounter == 8) {
        bEvent = true;
        bitCounter = 0;
        scancode = 0;
    }
}
