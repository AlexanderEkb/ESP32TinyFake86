#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "gbGlobals.h"
#include "PS2Kbd.h"
#include "PS2KeyCode.h"
#include "i8259.h"

class KeyboardDriver
{
  public:
    virtual void Init() = 0;
    virtual void Reset() = 0;
    virtual void Exec() = 0;
};

class KeyboardDriverAT : public KeyboardDriver
{
  public:
    static const uint32_t KEY_COUNT = 53;
    KeyboardDriverAT() {}
    virtual void Init() {
        Reset();
        kb_begin();
    }
    virtual void Reset()
    {
      memset(gb_key_cur, 1, KEY_COUNT);
      memset(gb_key_before, 1, KEY_COUNT);
    }
    virtual void Exec()
    {
      for(uint32_t i=0; i<KEY_COUNT; i++)
      {
        gb_key_cur[i] = ATKeyboard_GetKey(Keymap[i]);
      }

      bool bKeyStateChanged = false;
      unsigned char keyChanged = 0;
      for (unsigned char i = 0; i < KEY_COUNT; i++) {
        if (gb_key_cur[i] != gb_key_before[i]) {
            bKeyStateChanged = true;
            keyChanged = i;
            break;
        }
      }

      memcpy(gb_key_before, gb_key_cur, KEY_COUNT);

      unsigned char keyCode = 0;
      if (bKeyStateChanged) {                  // Hay un cambio en el teclado
        keyCode = TranslateScancode(keyChanged);
        const bool cbKeyPressed = (gb_key_cur[keyChanged] == 0);
        CheckTeclaSDL(keyCode, cbKeyPressed);
      }
    }
  private:
    unsigned char gb_key_cur[KEY_COUNT];
    unsigned char gb_key_before[KEY_COUNT];
    static constexpr unsigned char Keymap[KEY_COUNT] =
    {
      PS2_KC_A, PS2_KC_B, PS2_KC_C, PS2_KC_D, PS2_KC_E, PS2_KC_F, PS2_KC_G, PS2_KC_H,
      PS2_KC_I, PS2_KC_J, PS2_KC_K, PS2_KC_L, PS2_KC_M, PS2_KC_N, PS2_KC_O, PS2_KC_P,
      PS2_KC_Q, PS2_KC_R, PS2_KC_S, PS2_KC_T, PS2_KC_U, PS2_KC_V, PS2_KC_W, PS2_KC_X,
      PS2_KC_Y, PS2_KC_Z, PS2_KC_0, PS2_KC_1, PS2_KC_2, PS2_KC_3, PS2_KC_4, PS2_KC_5,
      PS2_KC_6, PS2_KC_7, PS2_KC_8, PS2_KC_9, PS2_KC_ENTER, PS2_KC_SPACE, KEY_BACKSPACE, PS2_KC_ESC,
      KEY_CURSOR_LEFT, KEY_CURSOR_RIGHT, KEY_CURSOR_UP, KEY_CURSOR_DOWN, PS2_KC_F1, PS2_KC_MINUS, PS2_KC_EQUAL, PS2_KC_COMMA,
      PS2_KC_DOT, PS2_KC_DIV, PS2_KC_L_SHIFT, PS2_KC_R_SHIFT, PS2_KC_SEMI
    };
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
    unsigned char TranslateScancode(int aux)
    {
    unsigned char aRet=0;
    switch (aux)
    {
      case 0: aRet= 30; break; //a
      case 1: aRet= 48; break; //b
      case 2: aRet= 46; break; //c
      case 3: aRet= 32; break; //d
      case 4: aRet= 18; break; //e
      case 5: aRet= 33; break; //f
      case 6: aRet= 34; break; //g
      case 7: aRet= 35; break; //h
      case 8: aRet= 23; break; //i
      case 9: aRet= 36; break; //j
      case 10: aRet= 37; break; //k
      case 11: aRet= 38; break; //l
      case 12: aRet= 50; break; //m
      case 13: aRet= 49; break; //n
      case 14: aRet= 24; break; //o
      case 15: aRet= 25; break; //p
      case 16: aRet= 16; break; //q
      case 17: aRet= 19; break; //r
      case 18: aRet= 31; break; //s
      case 19: aRet= 20; break; //t
      case 20: aRet= 22; break; //u
      case 21: aRet= 47; break; //v
      case 22: aRet= 17; break; //w
      case 23: aRet= 45; break; //x
      case 24: aRet= 21; break; //y
      case 25: aRet= 44; break; //z
      case 26: aRet= 11; break; //0
      case 27: aRet= 2; break; //1
      case 28: aRet= 3; break; //2
      case 29: aRet= 4; break; //3
      case 30: aRet= 5; break; //4
      case 31: aRet= 6; break; //5
      case 32: aRet= 7; break; //6
      case 33: aRet= 8; break; //7
      case 34: aRet= 9; break; //8
      case 35: aRet= 10; break; //9
      case 36: aRet= 28; break; //ENTER
      case 37: aRet= 57; break; //Barra espaciadora
      case 38: aRet= 14; break; //Back space
      case 39: aRet= 1; break; //ESC
      case 40: aRet= 75; break; //izquierda
      case 41: aRet= 77; break; //derecha
      case 42: aRet= 72; break; //arriba 
      case 43: aRet= 80; break; //abajo  
      case 44: aRet= 59; break; //F1

      case 45: aRet= 0x0C; break; // -
      case 46: aRet= 0x0D; break; // =
      case 47: aRet= 0x33; break; // ,
      case 48: aRet= 0x34; break; // .
      case 49: aRet= 0x35; break; // /

      case 50: aRet= 0x2A; break; // LShift
      case 51: aRet= 0x36; break; // RShift
      case 52: aRet= 0x27; break; // :
      default: aRet=0; break;
    }
    return aRet;
    }
    //*********************************************
    void CheckTeclaSDL(int nKeyCode, bool bIsDown)
    {
      gb_portramTiny[fast_tiny_port_0x60] = bIsDown ? nKeyCode : (nKeyCode | 0x80);
      gb_portramTiny[fast_tiny_port_0x64] |= 2;
      doirq(1);
    }
};

class Keyboard {
  public:
    enum KEYBOARD_DRIVER { KEYBOARD_DRIVER_NONE, KEYBOARD_DRIVER_AT };
    Keyboard(KEYBOARD_DRIVER eDriver) {
      m_oDriver = new KeyboardDriverAT();
    }
    void Init()
    {
      m_oDriver->Init();
    }
    void Reset()
    {
      m_oDriver->Reset();
    }
    void Exec()
    {
      m_oDriver->Exec();
    }
  private:
    KeyboardDriver * m_oDriver;
};
#endif /* KEYBOARD_H */