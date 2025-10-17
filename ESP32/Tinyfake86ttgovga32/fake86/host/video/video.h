#include <stdint.h>
#include "../config/config.h"

class Video_t
{
  public:
    virtual ~Video_t() {};
    virtual void init() = 0;
    virtual uint32_t width() = 0;
    virtual uint32_t height() = 0;
    virtual uint8_t * scanline(uint32_t n) = 0;
    virtual void miscCmd(uint32_t cmd, uint32_t param) = 0;
};

#if (HOST_VIDEO_DRIVER == 0)
#else
#error "Please choose any supported video driver."
#endif