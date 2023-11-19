#ifndef _RENDER_H
#define _RENDER_H

#include <stdint.h>

#define DUMPER_160x100_4x8  (0)
#define DUMPER_80x25_4x8    (1)
#define DUMPER_160x100_8x8  (2)
#define DUMPER_80x25_8x8    (3)
#define DUMPER_40x25_8x8    (4)
#define DUMPER_320x200      (5)
#define DUMPER_640x200      (6)

static const uint32_t VERTICAL_OFFSET = 20;

class cursor_t {
  public:
    static void updateMSB(uint8_t MSB);
    static void updateLSB(uint8_t LSB);
    static void updateStart(uint32_t startLine);
    static void updateEnd(uint32_t endLine);
    static uint32_t getCol();
    static uint32_t getRow();
    static uint32_t getStart();
    static uint32_t getEnd();
  private:
    static uint32_t row;
    static uint32_t col;
    static uint32_t value;
    static uint32_t start;
    static uint32_t end;
    static void updatePosition();
};

 void renderInit(void);
 void renderExec(void);
 void clearScreen(uint8_t color);
 void renderUpdateSettings(uint8_t settings, uint8_t colors);
 void renderSetCharHeight(uint8_t height);
 void renderSetStartAddr(uint32_t addr);
 void renderUpdateBorder();

 unsigned char initscreen(void);

 extern cursor_t cursor;
#endif