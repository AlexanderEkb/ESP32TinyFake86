#include "speaker.h"
#include "hardware.h"
#include "esp32-hal-gpio.h"

volatile unsigned int gb_pulsos_onda = 0;
volatile unsigned int gb_cont_my_callbackfunc = 0;
volatile unsigned char gb_estado_sonido = 0;
volatile unsigned char speaker_pin_estado = LOW;

unsigned char speakerenabled = 0;
unsigned char gb_frec_speaker_low = 0;
unsigned char gb_frec_speaker_high = 0;
unsigned char gb_cont_frec_speaker = 0;
volatile int gb_frecuencia01 = 0;
volatile int gb_volumen01 = 0;
unsigned char gb_silence = 0;

static void CalculaPulsosSonido(int freq)
{
  if (freq != 0)
    gb_pulsos_onda = (SAMPLE_RATE / freq) >> 1;
  else
    gb_pulsos_onda = 0;
}

void onPort0x61Write(uint8_t val)
{
  speakerenabled = ((val & 3) == 3) ? 1 : 0;
  unsigned int aData = (gb_frec_speaker_high << 8) | gb_frec_speaker_low;
  aData = (aData != 0) ? (1193180 / aData) : 0;
  CalculaPulsosSonido(aData);
  if (speakerenabled)
  {
    gb_volumen01 = 128;
    gb_frecuencia01 = aData;
  }
  else
  {
    gb_volumen01 = 0;
    gb_frecuencia01 = 0;
  }
}

void my_callback_speaker_func()
{
  gb_cont_my_callbackfunc++;
  if (gb_cont_my_callbackfunc >= gb_pulsos_onda)
  {
    gb_cont_my_callbackfunc = 0;
    gb_estado_sonido = (~gb_estado_sonido) & 0x1;
    if ((gb_volumen01 == 0) || (gb_frecuencia01 == 0) || (gb_silence == 1))
    {
      digitalWrite(SPEAKER_PIN, LOW);
    }
    else
    {
      if (speaker_pin_estado != gb_estado_sonido)
      {
        digitalWrite(SPEAKER_PIN, (gb_estado_sonido == LOW)?LOW:HIGH);
        speaker_pin_estado = gb_estado_sonido;
      }
    }
  }
}
