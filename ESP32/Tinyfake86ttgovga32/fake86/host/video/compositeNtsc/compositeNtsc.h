#include "../../config/config.h"

#if (VIDEO_DRIVER == 0)

#ifndef __VIDEO_COMPOSITE_NTSC_H__
#define __VIDEO_COMPOSITE_NTSC_H__
#include "../video.h"

static uint32_t const SET_BLITTER   = 0;
static uint32_t const SET_PHASE     = 1;  // Get rid of!
static uint32_t const SET_COLOR     = 2;
static uint32_t const PUSH_SETTINGS = 3;
static uint32_t const POP_SETTINGS  = 4;

class VideoCompositeNtsc_t : public Video_t
{
  public:
    virtual ~VideoCompositeNtsc_t() {};
    virtual void init() override;
    virtual uint32_t width() override {return XRES;};
    virtual uint32_t height() override {return YRES;};
    virtual uint8_t * scanline(uint32_t n) override {return bufferNTSC[n];};
    virtual void miscCmd(uint32_t cmd, uint32_t param) override;
  private:
    static constexpr uint32_t XRES = 336;
    static constexpr uint32_t YRES = 240;
    uint8_t **bufferNTSC;
    uint32_t blitterIndex;
    uint32_t storedBlitter;
    bool storedColorburst;
    uint8_t ** createBuffer();
    void setColorburstEnabled(bool bEnabled);
    void setBlitter(uint32_t blitter);
    void saveSettings(void);
    void restoreSettings(void);
    void setPhase(uint32_t phase);
};

#endif /* __VIDEO_COMPOSITE_NTSC_H__ */

#endif /* VIDEO_DRIVER */