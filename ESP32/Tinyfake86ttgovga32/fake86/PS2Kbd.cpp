#include "gbConfig.h"
#include "gbGlobals.h"
#include "fake86.h"
#include "hardware.h"
#include "keys.h"
#include "PS2Kbd.h"
#include <Arduino.h>

#ifndef LOG
#ifdef use_lib_log_serial
#define LOG(...) Serial.printf(__VA_ARGS__)
#else
#define LOG(...) (void)
#endif
#endif
volatile unsigned char keymap[256];
volatile static uint8_t incoming = 0;
volatile static uint8_t bitcount = 0;

boolean keyup = false;

// #define DEBUG_LOG_KEYSTROKES 1

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
        bitcount = 0;
        incoming = 0;
    }
    prev_ms = now_ms;
    // n = bitcount - 1;
    // if (n <= 7) {
    //     incoming |= (val << n);
    // }
    incoming <<= 1;
    incoming |= val;
    bitcount++;
    if (bitcount == 8)
    {
        bitcount = 0;
        keymap[incoming] = 0;
    }
//     if (bitcount == 11) {

//         if (1) {
//             if (keyup == true) {
//                 if (keymap[incoming] == 0) {
//                     keymap[incoming] = 1;
//                 } else {
//                     // Serial.println("WARNING: Keyboard cleaned");
//                     for (int gg = 0; gg < 256; gg++)
//                         keymap[gg] = 1;
//                 }
//                 keyup = false;
//             } else
//                 keymap[incoming] = 0;

// #ifdef DEBUG_LOG_KEYSTROKES
//                 #ifdef use_lib_log_serial
//                 Serial.printf("PS2Kbd[%s]: %02X\n",
//                     keyup ? " up " : "down", incoming);
//                 #endif    
// #endif

//             if (incoming == 240)
//                 keyup = true;
//             else
//                 keyup = false;
//         }
//         bitcount = 0;
//         incoming = 0;
//     }
}

//#define FIX_PERIBOARD_NOT_INITING
#ifdef FIX_PERIBOARD_NOT_INITING
 #include "PS2KeyAdvanced.h"
 PS2KeyAdvanced ps2boot;
#endif

void kb_begin()
{
 #ifdef FIX_PERIBOARD_NOT_INITING
  //Configure the keyboard library
  ps2boot.begin( KEYBOARD_DATA, KEYBOARD_CLK );
  ps2boot.echo( );              // ping keyboard to see if there
  delay( 6 );
  ps2boot.read( );
  delay( 6 );
  ps2boot.terminate();
 #endif

    pinMode(KEYBOARD_DATA, INPUT_PULLUP);
    pinMode(KEYBOARD_CLK, INPUT_PULLUP);
    digitalWrite(KEYBOARD_DATA, true);
    digitalWrite(KEYBOARD_CLK, true);
    attachInterrupt(digitalPinToInterrupt(KEYBOARD_CLK), kb_interruptHandler, FALLING);

    memset((void *)keymap, 1, sizeof(keymap));
}

// Check if key is pressed and clean it
boolean checkAndCleanKey(uint8_t scancode) {
    if (keymap[scancode] == 0) {
        keymap[scancode] = 1;
        return true;
    }
    return false;
}

//*****************************************
boolean checkKey(uint8_t scancode) {
    if (keymap[scancode] == 0)        
        return true;
    return false;
}

void emulateKeyChange(uint8_t scancode, uint8_t isdown)
{
    keymap[scancode] = isdown ? 0 : 1;
}

unsigned char ATKeyboard_GetKey(uint8_t nIndex)
{
    return keymap[nIndex];
}

uint8_t getScancode(void)
{
    uint8_t _incoming = 0;
    if(bitcount == 0)
    {
      _incoming = incoming;
      incoming = 0;
    }
    return _incoming;
}