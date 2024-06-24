#include "video/RawCompositeVideoBlitter.h"
#include "video/CompositeColorOutput.h"

void CompositeColorOutput::init(char ***frame)
{
  RawCompositeVideoBlitter::_lines = (uint8_t **)*frame;
  RawCompositeVideoBlitter::_blitter = RawCompositeVideoBlitter::blitter_1;
  RawCompositeVideoBlitter::_phase = 0;
  RawCompositeVideoBlitter::frame_init(); // The CompositeGraphics lib will do this for us
  RawCompositeVideoBlitter::video_init(RawCompositeVideoBlitter::NTSC);
}

void CompositeColorOutput::sendFrameHalfResolution(char ***frame)
{
  RawCompositeVideoBlitter::_lines = (uint8_t **)*frame;
}

void CompositeColorOutput::setColorburstEnabled(bool bEnabled)
{
  RawCompositeVideoBlitter::bColorburstEnabled = bEnabled;
}

void CompositeColorOutput::setBlitter(uint32_t blitter)
{
  switch (blitter)
  {
  case 0:
    RawCompositeVideoBlitter::_blitter = RawCompositeVideoBlitter::blitter_0;
    RawCompositeVideoBlitter::_phase = 0;
    break;
  case 1:
    RawCompositeVideoBlitter::_blitter = RawCompositeVideoBlitter::blitter_1;
    RawCompositeVideoBlitter::_phase = 0;
    break;
  case 2:
    RawCompositeVideoBlitter::_blitter = RawCompositeVideoBlitter::blitter_2;
    RawCompositeVideoBlitter::_phase = 0;
    break;
  }
  blitterIndex = blitter;
}

void CompositeColorOutput::saveSettings(void)
{
  storedBlitter = blitterIndex;
  storedColorburst = RawCompositeVideoBlitter::bColorburstEnabled;
}

void CompositeColorOutput::restoreSettings(void)
{
  setBlitter(storedBlitter);
  RawCompositeVideoBlitter::bColorburstEnabled = storedColorburst;
}

void CompositeColorOutput::setPhase(uint32_t phase)
{
  RawCompositeVideoBlitter::_phase = phase % 8;
}
