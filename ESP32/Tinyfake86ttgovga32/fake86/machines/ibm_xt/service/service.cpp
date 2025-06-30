#include <string.h>
#include "service.h"
#include "../extras/osd.h"
#include "../video/gb_sdl_font8x8.h"
#include "../video/render.h"
#include "../../../host/video/composite_ntsc/CompositeColorOutput.h"

extern char **bufferNTSC;
static uint8_t const *const font = getFont();

void svcBar(int orgX, int orgY, int height, int width, uint8_t color)
{
  for (int y = 0; y < height; y++)
  {
    int scanline = orgY + y;
    for (int x = 0; x < width; x++)
    {
      int col = orgX + x;
      bufferNTSC[scanline][col] = color;
    }
  }
}

void svcClearScreen(uint8_t color)
{
  const uint32_t BORDER_WIDTH = 8;
  const uint32_t FIELD_WIDTH = CompositeColorOutput::XRES - 2 * BORDER_WIDTH;
  const uint32_t RIGHT_POS = CompositeColorOutput::XRES - BORDER_WIDTH;
  for (int y = 0; y < OSD_VERTICAL_OFFSET; y++)
  {
    for (uint32_t x = 0; x < CompositeColorOutput::XRES; x++)
    {
      bufferNTSC[y][x] = DEFAULT_BORDER;
      bufferNTSC[y + EFFECTIVE_HEIGHT + OSD_VERTICAL_OFFSET][x] = DEFAULT_BORDER;
    }
  }
  for (int y = 0; y < EFFECTIVE_HEIGHT; y++)
  {
    for (int x = 0; x < BORDER_WIDTH; x++)
    {
      bufferNTSC[y + OSD_VERTICAL_OFFSET][x] = DEFAULT_BORDER;
      bufferNTSC[y + OSD_VERTICAL_OFFSET][x + RIGHT_POS] = DEFAULT_BORDER;
    }
    for (int x = 0; x < FIELD_WIDTH; x++)
      bufferNTSC[y + OSD_VERTICAL_OFFSET][x + BORDER_WIDTH] = color;
  }
}

void svcPrintChar(char character, int col, int row, unsigned char color, unsigned char backcolor, int32_t _off)
{
  int origin = character << 3; //*8
  unsigned char pixel;
  for (uint32_t y = 0; y < 8; y++)
  {
    const uint32_t line = row + y + _off;
    uint8_t aux = font[origin + y];
    for (uint32_t x = 0; x < 8; x++)
    {
      pixel = ((aux >> x) & 0x01);
      const uint32_t column = col + (8 - x);
      bufferNTSC[line][column] = (pixel == 1) ? color : backcolor;
    }
  }
}

//*************************************************************************************
void svcPrintText(const char *cad, int x, int y, unsigned char color, unsigned char backcolor, int32_t _off)
{
  // SDL_Surface *surface,
  //  gb_sdl_font_6x8
  int auxLen = strlen(cad);
  if (auxLen > 50)
    auxLen = 50;
  for (int i = 0; i < auxLen; i++)
  {
    svcPrintChar(cad[i], x, y, color, backcolor, _off);
    x += 8;
  }
}

void svcPrintTextColored(const char *cad, int x, int y, unsigned char color, unsigned char backcolor, int32_t _off)
{
  uint8_t fg = color;
  uint8_t bg = backcolor;
  char const * ptr = cad;
  uint8_t parameter;
  bool ESC = false;
  while (*ptr != '\0')
  {
    unsigned char c = *ptr;
    ptr++;
    if(c == '\a') // stands for 'attribute' :)
    {
      ESC = true;
      parameter = 0;
    } 
    else if(ESC)
    {
      switch(c)
      {
        case 0x30 ... 0x39:
          parameter <<= 4;
          parameter |= (c - 0x30);
          break;
        case 'A':
          parameter <<= 4;
          parameter |= 0x0A;
          break;
        case 'B':
          parameter <<= 4;
          parameter |= 0x0B;
          break;
        case 'C':
          parameter <<= 4;
          parameter |= 0x0C;
          break;
        case 'D':
          parameter <<= 4;
          parameter |= 0x0D;
          break;
        case 'E':
          parameter <<= 4;
          parameter |= 0x0E;
          break;
        case 'F':
          parameter <<= 4;
          parameter |= 0x0F;
          break;
        case 'f':
          fg = parameter;
          ESC = false;
          break;
        case 'b':
          bg = parameter;
          ESC = false;
          break;
        default:
          ESC = false;
          break;
      }
    }
    else
    {
      svcPrintChar(c, x, y, fg, bg, _off);
      x += getFontWidth();;
      if(CompositeColorOutput::XRES - x < getFontWidth())
        return;
    }
  }
}

#include "serviceFont.inc"

void svcPrintCharPetite(char character, int col, int row, unsigned char color, unsigned char backcolor)
{
  int origin = character * SERVICE_FONT_WIDTH;
  for (uint32_t x = 0; x < SERVICE_FONT_WIDTH; x++)
  {
    const uint32_t column = col + x;
    uint8_t line = serviceFont[origin + x];
    for (uint32_t y = 0; y < SERVICE_FONT_HEIGHT; y++)
    {
      uint8_t pixel = ((line >> y) & 0x01);
      const uint32_t line = row + y;
      bufferNTSC[line][column] = (pixel == 1) ? color : backcolor;
    }
  }
}

void svcPrintTextPetite(const char *cad, int x, int y, unsigned char color, unsigned char backcolor)
{
  int auxLen = strlen(cad);
  if (auxLen > 50)
    auxLen = 50;
  for (int i = 0; i < auxLen; i++)
  {
    svcPrintCharPetite(cad[i], x, y, color, backcolor);
    x += SERVICE_FONT_WIDTH + 1;
  }
}
