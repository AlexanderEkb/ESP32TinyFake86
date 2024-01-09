#include "io/keyboard.h"
#include "cpu/ports.h"
#include "esp32-hal-gpio.h"

IOPort port_060h = IOPort(0x060, 0x00, nullptr, nullptr);
IOPort port_063h = IOPort(0x063, 0x00, nullptr, nullptr);
IOPort port_064h = IOPort(0x064, 0x00, nullptr, nullptr);

uint8_t KeyboardDriverAT::lastKey = 0;
QueueHandle_t KeyboardDriverAT::q;

void IRAM_ATTR kb_interruptHandler(void) {
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
        KeyboardDriverAT::OnKey(shifter);
        digitalWrite(KEYBOARD_RDY, false);
    }
}
