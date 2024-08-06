#ifndef _SERVICE_H_
#define _SERVICE_H_

#include <stdint.h>

#define ASSERT(X) if(X == 0) {ESP_LOGE("svc", "Assertion failed at %s line %i", __FILE__, __LINE__); while(1);}

typedef struct DBG_MEM_ADDR
{
  uint16_t segment;
  uint16_t offset;
  DBG_MEM_ADDR() : segment(0), offset(0) {}
  DBG_MEM_ADDR(DBG_MEM_ADDR * src) : segment(src->segment), offset(src->offset) {}

  DBG_MEM_ADDR(uint16_t seg, uint16_t off) : segment(seg),
                                             offset(off) {}
  uint32_t linear() { return ((uint32_t)segment << 4) + offset; }
  void dec()
  {
    offset--;
    if(offset == 0)
      segment--;
  };
  void inc()
  {
    offset++;
    if(offset == 0)
      segment++;
  };
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
    DBG_MEM_ADDR prev = DBG_MEM_ADDR(this);
    this->offset++;
    if (this->offset == 0)
    {
      this->segment++;
    }
    return prev;
  }
  DBG_MEM_ADDR &operator+=(uint16_t rvalue)
  {
    uint32_t off = this->offset + rvalue;
    offset = static_cast<uint16_t>(off);
    if (off & 0xFFFF0000)
      segment++;
    return *this;
  }
  DBG_MEM_ADDR &operator-=(uint16_t rvalue)
  {
    uint32_t off = this->offset - rvalue;
    offset = static_cast<uint16_t>(off);
    if (off & 0xFFFF0000)
      segment--;
    return *this;
  }
  bool operator==(DBG_MEM_ADDR rvalue)
  {
    return (this->segment == rvalue.segment) && (this->offset == rvalue.offset);
  }
  bool operator!=(DBG_MEM_ADDR &rvalue)
  {
    return (this->segment != rvalue.segment) || (this->offset != rvalue.offset);
  }
} DBG_MEM_ADDR;

static const uint32_t SERVICE_FONT_WIDTH = 5;
static const uint32_t SERVICE_FONT_HEIGHT = 7;

static const uint8_t HEADER_BACKGROUND = 0x51;
static const uint8_t SCREEN_BACKGROUND = 0x12;
static const uint32_t DEFAULT_BORDER = 0x17;

static const uint8_t MENU_HEADER_BACKGROUND = 0x0A;
static const uint8_t MENU_HEADER_FOREGROUND = 0x00;

static const uint8_t MENUITEM_DEFAULT_BACKGROUND = DEFAULT_BORDER;
static const uint8_t MENUITEM_DEFAULT_FOREGROUND = 0x90;
static const uint8_t MENUITEM_HIGHLIGHT_FOREGROUND = 0x43;
static const uint8_t MENUITEM_SELECTED_BACKGROUND = HEADER_BACKGROUND;
static const uint8_t MENUITEM_SELECTED_FOREGROUND = 0x0A;

static const uint32_t OSD_VERTICAL_OFFSET = 20;
static const uint32_t EFFECTIVE_HEIGHT = 200;

void svcBar(int orgX, int orgY, int height, int width, uint8_t color);
void svcClearScreen(uint8_t color);
void svcPrintChar(char character, int col, int row, unsigned char color, unsigned char backcolor, int32_t _off = OSD_VERTICAL_OFFSET);
void svcPrintText(const char *cad, int x, int y, unsigned char color, unsigned char backcolor, int32_t _off = OSD_VERTICAL_OFFSET);

void svcPrintCharPetite(char character, int col, int row, unsigned char color, unsigned char backcolor);
void svcPrintTextPetite(const char *cad, int x, int y, unsigned char color, unsigned char backcolor);

#endif /* _SERVICE_H_ */