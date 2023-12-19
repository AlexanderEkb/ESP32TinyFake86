#ifndef _SERVICE_H_
#define _SERVICE_H_

#include <stdint.h>

typedef struct rect_t
{
  uint32_t left;
  uint32_t top;
  uint32_t width;
  uint32_t height;
  rect_t() : left(0), top(0), width(0), height(0){};
  rect_t(uint32_t left, uint32_t top, uint32_t width, uint32_t height) : left(left), top(top), width(width), height(height){};
  rect_t& operator=(rect_t &rvalue)
  {
    this->left = rvalue.left;
    this->top = rvalue.top;
    this->width = rvalue.width;
    this->height = rvalue.height;
    return *this;
  }
} rect_t;

typedef struct DBG_MEM_ADDR
{
  uint16_t segment;
  uint16_t offset;
  DBG_MEM_ADDR() : segment(0), offset(0) {}
  DBG_MEM_ADDR(uint16_t seg, uint16_t off) : segment(seg),
                                             offset(off) {}
  uint32_t linear() { return ((uint32_t)segment << 4) + offset; }
  DBG_MEM_ADDR &operator=(const DBG_MEM_ADDR &rvalue)
  {
    this->segment = rvalue.segment;
    this->offset = rvalue.offset;
    return *this;
  }
  DBG_MEM_ADDR &operator++()
  {
    this->offset++;
    if (this->offset == 0)
    {
      this->segment++;
    }
    return *this;
  }
  DBG_MEM_ADDR operator++(int)
  {
    DBG_MEM_ADDR prev = DBG_MEM_ADDR(this->segment, this->offset);
    this->offset++;
    if (this->offset == 0)
    {
      this->segment++;
    }
    return prev;
  }
} DBG_MEM_ADDR;

static const uint32_t SERVICE_FONT_WIDTH = 5;
static const uint32_t SERVICE_FONT_HEIGHT = 7;

static const uint8_t HEADER_BACKGROUND = 0x31;
static const uint8_t SCREEN_BACKGROUND = 0x60;
static const uint32_t DEFAULT_BORDER = 0x77;
static const uint32_t OSD_VERTICAL_OFFSET = 20;
static const uint32_t EFFECTIVE_HEIGHT = 200;

void svcBar(int orgX, int orgY, int height, int width, uint8_t color);
void svcClearScreen(uint8_t color);
void svcPrintChar(char character, int col, int row, unsigned char color, unsigned char backcolor);
void svcPrintText(const char *cad, int x, int y, unsigned char color, unsigned char backcolor);

void svcPrintCharPetite(char character, int col, int row, unsigned char color, unsigned char backcolor);
void svcPrintTextPetite(const char *cad, int x, int y, unsigned char color, unsigned char backcolor);

#endif /* _SERVICE_H_ */