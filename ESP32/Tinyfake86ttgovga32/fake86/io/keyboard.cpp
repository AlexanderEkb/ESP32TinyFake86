#include "io/keyboard.h"
#include "esp_timer.h"

uint8_t KeyboardDriverSTM::lastKey = 0;
QueueHandle_t KeyboardDriverSTM::q;

void IRAM_ATTR kb_interruptHandler(void * p) {
    static uint8_t shifter = 0;
    static uint8_t bitcount = 0;
    static uint32_t prev_ms = 0;
    uint32_t now_ms;
    uint8_t n, val;

    int clock = gpio_get_level(KEYBOARD_CLK);
    if (clock == 1)
        return;

    val = gpio_get_level(KEYBOARD_DATA);
    now_ms = esp_timer_get_time() / 1000ULL;
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
        KeyboardDriverSTM::OnKey(shifter);
    }
}
