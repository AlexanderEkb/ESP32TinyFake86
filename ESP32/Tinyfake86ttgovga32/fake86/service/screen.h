#ifndef _SERVICE_SCREEH_H_
#define _SERVICE_SCREEH_H_

#include <stdint.h>

static const uint8_t HEADER_BACKGROUND = 0x31;
static const uint8_t SCREEN_BACKGROUND = 0x60;
static const uint32_t DEFAULT_BORDER = 0x77;
static const uint32_t OSD_VERTICAL_OFFSET = 20;
static const uint32_t EFFECTIVE_HEIGHT = 200;

void svcBar(int orgX, int orgY, int height, int width, uint8_t color);
void svcClearScreen(uint8_t color);
void svcPrintChar(char character, int col, int row, unsigned char color, unsigned char backcolor);
void svcPrintText(const char *cad, int x, int y, unsigned char color, unsigned char backcolor);

#endif /* _SERVICE_SCREEH_H_ */