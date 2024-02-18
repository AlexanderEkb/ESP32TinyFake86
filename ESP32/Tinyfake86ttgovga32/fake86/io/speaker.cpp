#include "io/speaker.h"
#include "esp32-hal-gpio.h"
#include "config/hardware.h"
#include "cpu/ports.h"

volatile unsigned int gb_pulsos_onda = 0;
volatile unsigned int gb_cont_my_callbackfunc = 0;
volatile unsigned char gb_estado_sonido = 0;
volatile unsigned char speaker_pin_estado = LOW;

uint8_t gb_frec_speaker_low = 0;
uint8_t gb_frec_speaker_high = 0;
static bool speakerDrivenByTimer = true;

volatile uint8_t gb_frecuencia01 = 0;
volatile uint8_t gb_volumen01 = 0;
uint8_t gb_silence = 0;

static void onPort0x61Write(uint32_t address, uint8_t val);

IOPort port_061h = IOPort(0x061, 0xFF, nullptr, onPort0x61Write);

static void CalculaPulsosSonido(int freq)
{
  if (freq != 0)
    gb_pulsos_onda = (SAMPLE_RATE / freq) >> 1;
  else
    gb_pulsos_onda = 0;
}

void onPort0x61Write(uint32_t address, uint8_t val)
{
  static const uint8_t GATE_TIM_CH2_TO_SPEAKER  = 0x01;
  static const uint8_t DRIVE_SPEAKER            = 0x02;
  (void)address;
  speakerDrivenByTimer = (val & GATE_TIM_CH2_TO_SPEAKER);
  if (speakerDrivenByTimer)
  {
    uint32_t data = (gb_frec_speaker_high << 8) | gb_frec_speaker_low;
    updateFrequency(data);
    uint32_t aData = (data != 0) ? (1193180 / data) : 0;
    CalculaPulsosSonido(aData);
    gb_volumen01 = 128;
    gb_frecuencia01 = aData;
  }
  else
  {
    gb_volumen01 = 0;
    gb_frecuencia01 = 0;
    uint8_t level = (val & DRIVE_SPEAKER)?HIGH:LOW;
    digitalWrite(SPEAKER_PIN, level);
  }
}

void my_callback_speaker_func()
{
  gb_cont_my_callbackfunc++;
  if(speakerDrivenByTimer)
  {
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
          digitalWrite(SPEAKER_PIN, (gb_estado_sonido == LOW) ? LOW : HIGH);
          speaker_pin_estado = gb_estado_sonido;
        }
      }
    }
  }
}

void updateFrequency(uint16_t data)
{
  gb_frec_speaker_low = data & 0x00FF;
  gb_frec_speaker_high = static_cast<uint8_t>(data >> 8);
}