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

 void renderInit();
 void renderExec();
 void renderClearScreen(void);
 void renderPrintCharOSD(char character, int col, int row, unsigned char color, unsigned char backcolor);
 void renderSetBlitter(unsigned int blitter);
 void renderSetColumnCount(uint32_t columnCount);
 void renderSetColorEnabled(bool bEnabled);
 void renderUpdateColorSettings(uint32_t palette, uint32_t color);
 void renderUpdateDumper(uint32_t dumper);
 
 unsigned char initscreen ();
 void InitPaletaCGA(void);
 void InitPaletaCGA2(void);
 void InitPaletaCGAgray(void);
 void InitPaletaPCJR(void);

extern cursor_t cursor;
#endif

