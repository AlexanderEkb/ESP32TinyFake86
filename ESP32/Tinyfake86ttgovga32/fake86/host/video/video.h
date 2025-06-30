#include <stdint.h>
#include "../config/config.h"

class Video_t
{
  public:
    virtual ~Video_t() {};
    virtual uint32_t width() = 0;
    virtual uint32_t height() = 0;
    virtual uint8_t getLine(uint32_t line) = 0;
};

extern Video_t * video;
#if (VIDEO_DRIVER == 0)
#else
#error "Please choose among supported video drivers."
#endif