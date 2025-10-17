#ifndef __VIDEO_TGA_RENDER__
#define __VIDEO_TGA_RENDER__

#include <stdint.h>
#include "../../machine_config.h"

#if (IBM_XT_VIDEO_DRIVER == 1)

#include "../../../../host/host.h"

extern uint8_t videoMemory[];

static uint32_t const COLORBURST_NO_CHANGE = 0x00;
static uint32_t const COLORBURST_ENABLE = 0x01;
static uint32_t const COLORBURST_DISABLE = 0x02;

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

class tgaRender
{
  public:
    static uint32_t const TGA_COLOR_COUNT = 16;
    static constexpr uint8_t paletteHiRes[16] = {
    // BLACK   BLUE    GREEN   CYAN    RED     MGNTA   YELLOW  WHITE
        0x00,   0xC3,   0x45,   0xE6,   0x64,   0x95,   0x57,   0x0A,
        0x05,   0xC8,   0x4A,   0xEB,   0x69,   0x9A,   0x5C,   0x0F};
    static constexpr uint8_t paletteLoRes[16] = {
    // BLACK   BLUE    GREEN   CYAN    RED     MGNTA   YELLOW  WHITE
        0x00,   0xE6,   0x66,   0x46,   0x96,   0xB5,   0x84,   0x0A,
        0x05,   0xE8,   0x69,   0x4B,   0x99,   0xBA,   0x8C,   0x0F};
    static constexpr uint8_t paletteBW[16] = {
    // BLACK   BLUE    GREEN   CYAN    RED     MGNTA   YELLOW  WHITE
        0x00,   0x01,   0x05,   0x06,   0x02,   0x04,   0x07,   0x0C,
        0x03,   0x08,   0x0B,   0x0D,   0x09,   0x0A,   0x0E,   0x0F};

    static const uint32_t GREEN   = 2;
    static const uint32_t RED     = 4;
    static const uint32_t YELLOW  = 6;
    static const uint32_t CYAN    = 3;
    static const uint32_t MAGENTA = 5;
    static const uint32_t WHITE   = 7;
    static const uint32_t BRIGHT  = 8;

    static const uint32_t GRAPH_PALETTE_SIZE = 4;
    static constexpr uint8_t paletteGraphicGRYdim[GRAPH_PALETTE_SIZE]      = {0x00, paletteLoRes[GREEN], paletteLoRes[RED], paletteLoRes[YELLOW]};
    static constexpr uint8_t paletteGraphicGRYdimBW[GRAPH_PALETTE_SIZE]    = {0x00, paletteBW[GREEN], paletteBW[RED], paletteBW[YELLOW]};
    static constexpr uint8_t paletteGraphicGRYbright[GRAPH_PALETTE_SIZE]   = {0x00, paletteLoRes[BRIGHT + GREEN], paletteLoRes[BRIGHT + RED], paletteLoRes[BRIGHT + YELLOW]};
    static constexpr uint8_t paletteGraphicGRYbrightBW[GRAPH_PALETTE_SIZE] = {0x00, paletteBW[BRIGHT + GREEN], paletteBW[BRIGHT + RED], paletteBW[BRIGHT + YELLOW]};
    static constexpr uint8_t paletteGraphicCMWdim[GRAPH_PALETTE_SIZE]      = {0x00, paletteLoRes[CYAN], paletteLoRes[MAGENTA], paletteLoRes[WHITE]};
    static constexpr uint8_t paletteGraphicCMWdimBW[GRAPH_PALETTE_SIZE]    = {0x00, paletteBW[CYAN], paletteBW[MAGENTA], paletteBW[WHITE]};
    static constexpr uint8_t paletteGraphicCMWbright[GRAPH_PALETTE_SIZE]   = {0x00, paletteLoRes[BRIGHT + CYAN], paletteLoRes[BRIGHT + MAGENTA], paletteLoRes[BRIGHT + WHITE]};
    static constexpr uint8_t paletteGraphicCMWbrightBW[GRAPH_PALETTE_SIZE] = {0x00, paletteBW[BRIGHT + CYAN], paletteBW[BRIGHT + MAGENTA], paletteBW[BRIGHT + WHITE]};

    static const uint32_t GRAPH_PALETTE_COUNT = 4;

    static constexpr uint8_t const * graphPalettes[GRAPH_PALETTE_COUNT] = {
            paletteGraphicGRYdim,
            paletteGraphicGRYbright,
            paletteGraphicCMWdim,
            paletteGraphicCMWbright};

    static constexpr uint8_t const * graphPalettesBW[GRAPH_PALETTE_COUNT] = {
            paletteGraphicGRYdimBW,
            paletteGraphicGRYbrightBW,
            paletteGraphicCMWdimBW,
            paletteGraphicCMWbrightBW};


    static void init(void);
    static void deinit(void);
    static void updateSettings(uint8_t settings, uint8_t colors);
    static void setCharHeight(uint8_t height);
    static void setColorburstOverride(uint32_t value);
    static void setStartAddr(uint32_t addr);
    static void updateBorder();
    static void setCursorStart(uint8_t line);
    static void setCursorEnd(uint8_t line);
    static void setCursorAddrMSB(uint8_t addr);
    static void setCursorAddrLSB(uint8_t addr);
    static void draw()
    {
      render.frameCount++;
      render.dumper();
    }

    static uint8_t const * graphPaletteColor(uint32_t n)
    {
      return graphPalettes[n];
    }

  private:
    typedef void (*dumper_t)(void);

    typedef enum vmode_t
    {
      TEXT_LO,
      TEXT_HI,
      GRAPH_LO,
      GRAPH_HI
    } vmode_t;

    typedef struct render_t
    {
      /// @brief true if some changes are made by the 'video' part. In such a case
      ///        OnDumpDone() function performs these changes and pendingRender contents
      //         is copied to the render var.
      bool pendingChanges;

      /// @brief pointer to a dumper function, which transforms CGA video memory contents
      ///        to the format accepted by the rendering module.
      dumper_t dumper;
      uint32_t paletteIndex;
      uint32_t specialColor;
      uint32_t hasColor;

      uint32_t textCharHeight;
      uint32_t textRowCount;
      uint32_t textColCount;

      uint32_t startAddr;
      uint32_t pixelsPerLine;
      uint32_t horizontalPosition;
      uint32_t rightBorderPosition;
      uint32_t rightBorderWidth;
      uint32_t hOffset;
      uint32_t blitter;
      vmode_t vmode;

      uint32_t frameCount;
    } render_t;

    static uint32_t const VERTICAL_OFFSET = 20;

    static uint32_t const BLITTER_HIRES = 0;
    static uint32_t const BLITTER_LORES = 1;

    static uint32_t const MODE_COUNT = 8;

    static uint8_t palette[16];
    static uint8_t dumpLineBuffer[700];

    static uint32_t colorburstOverride;
    static render_t render;
    static render_t pendingRender;
    static cursor_t cursor;
    static uint8_t const * font;

    static void dump80x25()
    {
      uint8_t aColor, aBgColor, aChar;
      uint32_t src = render.startAddr;
      for (uint32_t y = 0; y < render.textRowCount; y++)
      {
        for (uint32_t x = 0; x < 80; x++)
        {
          aChar = videoMemory[src];
          src++;
          aColor = videoMemory[src] & 0x0F;
          aBgColor = ((videoMemory[src] >> 4) & 0x0F);
          printChar(aChar, (x << 3), (y * render.textCharHeight), aColor, aBgColor); // Sin capturadora
          src++;
        }
      }
      OnDumpDone();
    }

    static void dump40x25()
    {
      uint32_t src = render.startAddr;
      for (uint32_t y = 0; y < render.textRowCount; y++)
      {
        for (uint32_t x = 0; x < 40; x++)
        {
          uint8_t aChar = videoMemory[src];
          src++;
          uint8_t aColor = videoMemory[src] & 0x0F;
          uint8_t aBgColor = ((videoMemory[src] >> 4) & 0x07);
          printChar(aChar, (x << 3), (y * render.textCharHeight), aColor, aBgColor); // Sin capturadora
          src++;
        }
      }
      OnDumpDone();
    }

    static void dump320x200x4()
    {
      static const uint32_t INITIAL_OFFSET = 0;
      unsigned short int cont = 0;
      for (uint32_t y = 0; y < 100; y++)
      {
        uint32_t offset = INITIAL_OFFSET;
        for (uint32_t x = 0; x < 80; x++)
        {
          uint8_t src = videoMemory[cont];
          uint8_t bPixel3 = (src & 0x03);
          src >>= 2;
          uint8_t bPixel2 = (src & 0x03);
          src >>= 2;
          uint8_t bPixel1 = (src & 0x03);
          src >>= 2;
          uint8_t bPixel0 = (src & 0x03);

          dumpLineBuffer[offset++] = palette[bPixel0];
          dumpLineBuffer[offset++] = palette[bPixel1];
          dumpLineBuffer[offset++] = palette[bPixel2];
          dumpLineBuffer[offset++] = palette[bPixel3];
          cont++;
        }
        uint32_t yDest = (y << 1);
        uint32_t *dest = (uint32_t *)Host::video->scanline(yDest + VERTICAL_OFFSET);
        memcpy((void *)dest + render.horizontalPosition, dumpLineBuffer, render.pixelsPerLine);
      }

      cont = 0x2000;
      for (uint32_t y = 0; y < 100; y++)
      {
        uint32_t offset = INITIAL_OFFSET;
        for (uint32_t x = 0; x < 80; x++)
        {
          uint8_t src = videoMemory[cont];
          uint8_t bPixel3 = (src & 0x03);
          src >>= 2;
          uint8_t bPixel2 = (src & 0x03);
          src >>= 2;
          uint8_t bPixel1 = (src & 0x03);
          src >>= 2;
          uint8_t bPixel0 = (src & 0x03);

          dumpLineBuffer[offset++] = palette[bPixel0];
          dumpLineBuffer[offset++] = palette[bPixel1];
          dumpLineBuffer[offset++] = palette[bPixel2];
          dumpLineBuffer[offset++] = palette[bPixel3];
          cont++;
        }
        uint32_t yDest = (y << 1) + 1;
        uint32_t *dest = (uint32_t *)Host::video->scanline(yDest + VERTICAL_OFFSET);
        memcpy((void *)dest + render.horizontalPosition, dumpLineBuffer, render.pixelsPerLine);
      }
      OnDumpDone();
    }

    static void dump320x200x16()
    {
      static const uint32_t INITIAL_OFFSET = 0;
      unsigned short int cont = 0;
      for (uint32_t y = 0; y < 100; y++)
      {
        uint32_t offset = INITIAL_OFFSET;
        for (uint32_t x = 0; x < 160; x++)
        {
          uint8_t src = videoMemory[cont];
          uint8_t bPixel1 = (src & 0x0F);
          src >>= 4;
          uint8_t bPixel0 = (src & 0x0F);

          dumpLineBuffer[offset++] = palette[bPixel0];
          dumpLineBuffer[offset++] = palette[bPixel1];
          cont++;
        }
        uint32_t yDest = (y << 1);
        uint32_t *dest = (uint32_t *)Host::video->scanline(yDest + VERTICAL_OFFSET);
        memcpy((void *)dest + render.horizontalPosition, dumpLineBuffer, render.pixelsPerLine);
      }

      cont = 0x4000;
      for (uint32_t y = 0; y < 100; y++)
      {
        uint32_t offset = INITIAL_OFFSET;
        for (uint32_t x = 0; x < 160; x++)
        {
          uint8_t src = videoMemory[cont];
          uint8_t bPixel1 = (src & 0x0F);
          src >>= 4;
          uint8_t bPixel0 = (src & 0x0F);

          dumpLineBuffer[offset++] = palette[bPixel0];
          dumpLineBuffer[offset++] = palette[bPixel1];
          cont++;
        }
        uint32_t yDest = (y << 1) + 1;
        uint32_t *dest = (uint32_t *)Host::video->scanline(yDest + VERTICAL_OFFSET);
        memcpy((void *)dest + render.horizontalPosition, dumpLineBuffer, render.pixelsPerLine);
      }
      OnDumpDone();
    }

    static void dump640x200x2()
    {
      static uint32_t *dest;
      static const uint32_t INITIAL_OFFSET = 0;
      unsigned short int srcAddr;
      // unsigned int yDest;
      unsigned int x;
      unsigned int a32;

      srcAddr = 0x0000;
      for (uint32_t y = 0; y < 100; y++)
      {
        uint32_t offset = INITIAL_OFFSET;
        for (x = 0; x < 80; x++)
        {
          unsigned char src = videoMemory[srcAddr];
          uint8_t a7 = (src & 0x01);
          src >>= 1;
          uint8_t a6 = (src & 0x01);
          src >>= 1;
          uint8_t a5 = (src & 0x01);
          src >>= 1;
          uint8_t a4 = (src & 0x01);
          src >>= 1;
          uint8_t a3 = (src & 0x01);
          src >>= 1;
          uint8_t a2 = (src & 0x01);
          src >>= 1;
          uint8_t a1 = (src & 0x01);
          src >>= 1;
          uint8_t a0 = (src & 0x01);

          uint32_t off = x << 1;
          dumpLineBuffer[offset++] = palette[a0];
          dumpLineBuffer[offset++] = palette[a1];
          dumpLineBuffer[offset++] = palette[a2];
          dumpLineBuffer[offset++] = palette[a3];
          dumpLineBuffer[offset++] = palette[a4];
          dumpLineBuffer[offset++] = palette[a5];
          dumpLineBuffer[offset++] = palette[a6];
          dumpLineBuffer[offset++] = palette[a7];

          srcAddr++;
        }
        uint32_t yDest = (y << 1);
        // dest = (uint32_t *)bufferNTSC[yDest + VERTICAL_OFFSET];
        dest = (uint32_t *)Host::video->scanline(yDest + VERTICAL_OFFSET);
        memcpy((void *)dest + render.horizontalPosition, dumpLineBuffer, render.pixelsPerLine);
      }

      srcAddr = 0x2000;
      for (uint32_t y = 0; y < 100; y++)
      {
        uint32_t offset = INITIAL_OFFSET;
        for (x = 0; x < 80; x++)
        {
          unsigned char src = videoMemory[srcAddr];
          uint8_t a7 = (src & 0x01);
          src >>= 1;
          uint8_t a6 = (src & 0x01);
          src >>= 1;
          uint8_t a5 = (src & 0x01);
          src >>= 1;
          uint8_t a4 = (src & 0x01);
          src >>= 1;
          uint8_t a3 = (src & 0x01);
          src >>= 1;
          uint8_t a2 = (src & 0x01);
          src >>= 1;
          uint8_t a1 = (src & 0x01);
          src >>= 1;
          uint8_t a0 = (src & 0x01);

          uint32_t off = x << 1;
          dumpLineBuffer[offset++] = palette[a0];
          dumpLineBuffer[offset++] = palette[a1];
          dumpLineBuffer[offset++] = palette[a2];
          dumpLineBuffer[offset++] = palette[a3];
          dumpLineBuffer[offset++] = palette[a4];
          dumpLineBuffer[offset++] = palette[a5];
          dumpLineBuffer[offset++] = palette[a6];
          dumpLineBuffer[offset++] = palette[a7];

          srcAddr++;
        }
        // dest = (uint32_t *)bufferNTSC[yDest + VERTICAL_OFFSET];
        uint32_t yDest = (y << 1) + 1;
        dest = (uint32_t *)Host::video->scanline(yDest + VERTICAL_OFFSET);
        memcpy((void *)dest + render.horizontalPosition, dumpLineBuffer, render.pixelsPerLine);
      }
      OnDumpDone();
    }

    typedef struct
    {
      /// @brief Characters per line. Only has effect in text modes
      uint32_t textColCount;
      /// @brief Pointer to an appropriate dumper function
      dumper_t dumper;
      /// @brief Index of an appropriate blitter function
      uint32_t blitter;
      /// @brief Horizontal image offset. Very important for both horizontal position and accuracy of artifact colors.
      uint32_t hOffset;
    } videoMode_t;

    static constexpr videoMode_t modes[MODE_COUNT] = {
      {40, dump40x25,       BLITTER_LORES, 0},
      {80, dump80x25,       BLITTER_HIRES, 0},
      {40, dump320x200x4,   BLITTER_LORES, 0},
      {40, dump320x200x4,   BLITTER_LORES, 0},
      {40, dump40x25,       BLITTER_LORES, 0},
      {80, dump80x25,       BLITTER_HIRES, 0},
      {80, dump640x200x2,   BLITTER_HIRES, 2},
      {80, dump640x200x2,   BLITTER_HIRES, 2}};

    static __always_inline void OnDumpDone()
    {
      if (pendingRender.pendingChanges)
      {
        pendingRender.pendingChanges = false;
        memcpy(&render, &pendingRender, sizeof(render_t));

        bool colorEnabled = (colorburstOverride == COLORBURST_NO_CHANGE) ? (render.hasColor != COLORBURST_DISABLE) : (colorburstOverride != COLORBURST_DISABLE);
        Host::video->miscCmd(SET_COLOR, colorEnabled);
        switch (render.vmode)
        {
        case TEXT_LO:
          memcpy(palette, colorEnabled ? paletteLoRes : paletteBW, 16);
          break;
        case TEXT_HI:
          memcpy(palette, colorEnabled ? paletteHiRes : paletteBW, 16);
          break;
        case GRAPH_LO:
          memcpy(palette, colorEnabled ? graphPalettes[render.paletteIndex] : graphPalettesBW[render.paletteIndex], GRAPH_PALETTE_SIZE);
          palette[0] = paletteLoRes[render.specialColor];
          break;
        case GRAPH_HI:
          palette[0] = 0;
          palette[1] = colorEnabled ? paletteHiRes[render.specialColor] : paletteBW[render.specialColor];
          break;
        }

        updateBorder();
      }
    }

    static void printChar_c(char code, uint32_t x, uint32_t y, uint8_t color, uint8_t backcolor)
    {
      int nBaseOffset = code << 3;
      const bool blink = render.frameCount & 0x04;
      for (unsigned int row = 0; row < render.textCharHeight; row++)
      {
        const bool cbFill = (row >= cursor.getStart()) && (row <= cursor.getEnd()) && blink;
        unsigned char src = ((row >= 6) && (blink)) ? 0xFF : font[nBaseOffset + row];
        const uint32_t vgaLine = y + row + VERTICAL_OFFSET;
        uint32_t vgaCol = x + render.horizontalPosition;
        uint8_t * line = Host::video->scanline(vgaLine);
        for (int col = 0; col < render.textCharHeight; col++)
        {
          // bufferNTSC[vgaLine][vgaCol] = palette[((src & 0x80) != 0) ? color : backcolor];
          line[vgaCol] = palette[((src & 0x80) != 0) ? color : backcolor];
          vgaCol++;
          src <<= 1;
        }
      }
    }

    static void printChar(char code, uint32_t x, uint32_t y, uint8_t color, uint8_t backcolor)
    {
      if ((x == (cursor.getCol() << 3)) && (y == (cursor.getRow() << 3)))
      {
        printChar_c(code, x, y, color, backcolor);
      }
      else
      {
        int nBaseOffset = code << 3; //*8
        for (unsigned int row = 0; row < render.textCharHeight; row++)
        {
          unsigned char src = font[nBaseOffset + row];
          const uint32_t vgaLine = y + row + VERTICAL_OFFSET;
          uint32_t vgaCol = x + render.horizontalPosition;
          uint8_t * line = Host::video->scanline(vgaLine);
          for (int col = 0; col < 8; col++)
          {
            // bufferNTSC[vgaLine][vgaCol] = palette[((src & 0x80) != 0) ? color : backcolor];
            line[vgaCol] = palette[((src & 0x80) != 0) ? color : backcolor];
            src <<= 1;
            vgaCol++;
          }
        }
      }
    }
    friend class cursor_t;
};

#endif /* IBM_XT_VIDEO_DRIVER */
#endif /* __VIDEO_TGA_RENDER__ */