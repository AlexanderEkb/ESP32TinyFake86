#include "keyboard.h"

volatile uint8_t KeyboardDriverAT::incoming = 0;
volatile uint8_t KeyboardDriverAT::bitcount = 0;
volatile uint8_t KeyboardDriverAT::lastScanCode = 0;

void IRAM_ATTR kb_interruptHandler(void) {
    static uint32_t prev_ms = 0;
    uint32_t now_ms;
    uint8_t n, val;

    int clock = digitalRead(KEYBOARD_CLK);
    if (clock == 1)
        return;

    val = digitalRead(KEYBOARD_DATA);
    now_ms = millis();
    if (now_ms - prev_ms > 5) {
        KeyboardDriverAT::bitcount = 0;
        KeyboardDriverAT::incoming = 0;
    }
    prev_ms = now_ms;

    KeyboardDriverAT::incoming <<= 1;
    KeyboardDriverAT::incoming |= val;
    KeyboardDriverAT::bitcount++;
    if (KeyboardDriverAT::bitcount == 8) {
        KeyboardDriverAT::bitcount = 0;
        if (!(KeyboardDriverAT::incoming & 0x80)) {
            KeyboardDriverAT::lastScanCode = KeyboardDriverAT::incoming;
        }
    }
}
