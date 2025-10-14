/*
 * How it was implemented in the glorious IBM PC:
 * ==============================================
 *                 PIT, Ch#2
 *              ╭─────────────╮
 * 1.193 MHz ──>┤Clk          │
 * CPU_bus ───<>┤Cnt       Out├>────┐         ╭─────╮               
 * PB1 ────────>┤Gate         │     └─────────┤ &   │
 *              ╰─────────────╯               │     ├───── Audio
 *                                  ┌─────────┤     │
 * PB0 ─────────────────────────────┘         ╰─────╯
 * 
 * How it is implemented here:
 * ===========================
 * 
 *                     TG0_T1 - to make conter readable
 *                  ╭────────────────────╮
 * 1.193 MHz ──────>┤Clk                 │
 * PB1 ──────┬─────>┤Gate             Out├>─X 
 * CPU_bus ────┬──<>┤Counter reg (R/W)   │ 
 *           │ │    ╰────────────────────╯ 
 *           │ │       'Ticker' - to play sound and not overloading this we-eak CPU
 *           │ │    ╭────────────────╮
 *           │ └───>┤Cnt (R/O)       │
 *           └─────>┤Gate         Out├>─┐ 
 * Some clock ─────>┤Clk             │  │
 *                  ╰────────────────╯  │
 *                                      │         ╭─────╮           
 *                                      └─────────┤ &   │
 *                                                │     ├───── Audio
 *                                      ┌─────────┤     │
 *           PB0 ───────────────────────┘         ╰─────╯
 */

#include "speaker.h"
#include "../../../host/audio/audio.h"

bool Speaker_t::PB0 = false;
bool Speaker_t::PB1 = false;
bool Speaker_t::Ch2 = false;
bool Speaker_t::muted = false;
uint32_t Speaker_t::period = 0;

void __attribute__((optimize("-Ofast"))) IRAM_ATTR Speaker_t::onTimer()
{
  static uint32_t counter = 0;

  if (++counter >= period)
  {
    counter = 0;
    if(PB0)
      Ch2 ^= true;
    if(!muted)
      Audio_t::driveSpeaker(Ch2 && PB1);
    }
}

void __attribute__((optimize("-Ofast"))) IRAM_ATTR Speaker_t::gateCh2(bool state)
{
  if(PB0 != state)
  {
    PB0 = state;
    if(!state)
      Ch2 = true;
  }
}

void __attribute__((optimize("-Ofast"))) IRAM_ATTR Speaker_t::driveDirectly(bool state)
{
  PB1 = state;
  if(!muted)
    Audio_t::driveSpeaker(Ch2 && PB1);
}

void Speaker_t::mute()
{
  muted = true;
  Audio_t::driveSpeaker(false);
}

void Speaker_t::unmute()
{
  muted = false;
}

void __attribute__((optimize("-Ofast"))) IRAM_ATTR Speaker_t::updateFrequency(uint16_t data)
{
  uint32_t freq = (data != 0) ? (1193180 / data) : 0;
  if (freq != 0)
    period = (SAMPLE_RATE / freq) >> 1;
  else
    period = 0;    
}

void __attribute__((optimize("-Ofast"))) IRAM_ATTR my_callback_speaker_func()
{
  Speaker_t::onTimer();
}
