#ifndef _RENDER_H
#define _RENDER_H

#include <stdint.h>

static const uint32_t COLORBURST_NO_CHANGE = 0x00;
static const uint32_t COLORBURST_ENABLE = 0x01;
static const uint32_t COLORBURST_DISABLE = 0x02;

class Render_t
{
  public:
    void init(void);
    void deinit(void);
    void updateSettings(uint8_t settings, uint8_t color);
    void setCharHeight(uint8_t height);
    void setColorburstOverride(uint32_t value);
    void setStartAddr(uint32_t addr);
    void updateBorder();
    void setCursorStart(uint8_t line);
    void setCursorEnd(uint8_t line);
    void setCursorAddrMSB(uint8_t addr);
    void setCursorAddrLSB(uint8_t addr);
  private:
    static const uint32_t VERTICAL_OFFSET = 20;
    static const uint32_t COLORBURST_NO_CHANGE = 0x00;
    static const uint32_t COLORBURST_ENABLE = 0x01;
    static const uint32_t COLORBURST_DISABLE = 0x02;
};

void renderInit(void);
void renderUpdateSettings(uint8_t settings, uint8_t color);
void renderSetCharHeight(uint8_t height);
void renderSetColorburstOverride(uint32_t value);
void renderSetStartAddr(uint32_t addr);
void renderUpdateBorder();
void renderSetCursorStart(uint8_t line);
void renderSetCursorEnd(uint8_t line);
void renderSetCursorAddrMSB(uint8_t addr);
void renderSetCursorAddrLSB(uint8_t addr);

#endif
