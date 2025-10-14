#include "../../config/config.h"

#if (VIDEO_DRIVER == 0)
#include <string.h>
#include "RawCompositeVideoBlitter.h"
#include "compositeNtsc.h"

void VideoCompositeNtsc_t::init()
{
  RawCompositeVideoBlitter::_lines = createBuffer();
  RawCompositeVideoBlitter::_blitter = RawCompositeVideoBlitter::blitter_1;
  RawCompositeVideoBlitter::_phase = 0;
  RawCompositeVideoBlitter::frame_init(); // The CompositeGraphics lib will do this for us
  RawCompositeVideoBlitter::video_init(RawCompositeVideoBlitter::NTSC);
}

void VideoCompositeNtsc_t::miscCmd(uint32_t cmd, uint32_t param)
{
  switch(cmd)
  {
    case SET_BLITTER:
      setBlitter(param);
      return;
    case SET_PHASE:
      setPhase(param);
      return;
    case SET_COLOR:   
      setColorburstEnabled(param);
      return;
    case PUSH_SETTINGS:
      saveSettings();
      return;
    case POP_SETTINGS:
      restoreSettings();
      return;
  }
}

uint8_t ** VideoCompositeNtsc_t::createBuffer()
{
  bufferNTSC = (uint8_t **)malloc(YRES * sizeof(char *));
  assert(bufferNTSC);
  for (int y = 0; y < YRES; y++)
  {
    bufferNTSC[y] = (uint8_t *)malloc(XRES * 2);
    assert(bufferNTSC[y]);
    memset(bufferNTSC[y], 0x00, XRES * 2);
  }

  return bufferNTSC;
}

void VideoCompositeNtsc_t::setColorburstEnabled(bool bEnabled)
{
  RawCompositeVideoBlitter::bColorburstEnabled = bEnabled;
}

void VideoCompositeNtsc_t::setBlitter(uint32_t blitter)
{
  switch (blitter)
  {
  case 0:
    RawCompositeVideoBlitter::_blitter = RawCompositeVideoBlitter::blitter_0;
    RawCompositeVideoBlitter::_phase = 1;
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

void VideoCompositeNtsc_t::saveSettings(void)
{
  storedBlitter = blitterIndex;
  storedColorburst = RawCompositeVideoBlitter::bColorburstEnabled;
}

void VideoCompositeNtsc_t::restoreSettings(void)
{
  setBlitter(storedBlitter);
  RawCompositeVideoBlitter::bColorburstEnabled = storedColorburst;
}

void VideoCompositeNtsc_t::setPhase(uint32_t phase)
{
  RawCompositeVideoBlitter::_phase = phase % 8;
}

#endif /* VIDEO_DRIVER */