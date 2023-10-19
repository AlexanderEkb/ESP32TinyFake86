//  Fake86: A portable, open-source 8086 PC emulator.
//  Copyright (C)2010-2012 Mike Chambers
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
// render.c: functions for SDL initialization, as well as video scaling/rendering.
//   it is a bit messy. i plan to rework much of this in the future. i am also
//   going to add hardware accelerated scaling soon.

#include "gbConfig.h"
//#include <SDL/SDL.h>
#include <stdio.h>
#include <Arduino.h>
//JJ #include "mutex.h"
#include "gbGlobals.h"
#include "cpu.h"
#include "ports.h"
#include "render.h"
#include "fake86.h"
#include "gb_sdl_font8x8.h"
#include "gb_sdl_font4x8.h"
#include "render.h"

#define SUPPORT_NTSC 1
#include "CompositeColorOutput.h"

#define PENDING_COLORBURST_NO     (0x00)
#define PENDING_COLORBURST_TRUE   (0x01)
#define PENDING_COLORBURST_FALSE  (0x02)

typedef void (* dumper_t)(void);

static void dump160x100_font4x8(void);
static void dump80x25_font4x8(void);
static void dump160x100_font8x8(void);
static void dump80x25_font8x8(void);
static void dump40x25_font8x8(void);
static void dump320x200(void);
static void dump640x200(void);

static void SDLprintChar_c(char car,int x,int y,unsigned char color,unsigned char backcolor);
static void SDLprintChar(char car,int x,int y,unsigned char color,unsigned char backcolor);
static void SDLprintChar160x100_font4x8(char car,int x,int y,unsigned char color,unsigned char backcolor);
static void SDLprintChar4x8(char car,int x,int y,unsigned char color,unsigned char backcolor);
static void SDLprintChar160x100_font8x8(char car,int x,int y,unsigned char color,unsigned char backcolor);

static const uint32_t DUMPER_COUNT = 7;

static const uint32_t MODE_COUNT = 7;

typedef enum {COLOR_SHOW, COLOR_JAM} color_t;
typedef struct {
  dumper_t  dumper;
  uint32_t  textWidth;
  uint32_t  palette;
  color_t   color;
} videoMode_t;

const videoMode_t modes[MODE_COUNT] = {
  {}
};

const dumper_t dumpers[DUMPER_COUNT] = {
  dump160x100_font4x8,
  dump80x25_font4x8,
  dump160x100_font8x8,
  dump80x25_font8x8,
  dump40x25_font8x8,
  dump320x200,
  dump640x200
};

cursor_t cursor;
static uint32_t scanlineBuffer[80];

static const uint32_t VERTICAL_OFFSET = 20;

//cga 1
const unsigned char paletteGRYdim[16]={ 
//  Black   Green   Red     Yellow - others don't matter
    0x00,   0xD6,   0x44,   0xFA,   0x34,   0x54,   0xF4,   0x07,
    0x03,   0x8C,   0xCC,   0xAC,   0x3C,   0x5C,   0xFC,   0x0F
};

const unsigned char paletteGRYbright[16]={ 
//  Black   Green   Red     Yellow - others don't matter
    0x00,   0xD9,   0x47,   0xFF,   0x34,   0x54,   0xF4,   0x07,
    0x03,   0x8C,   0xCC,   0xAC,   0x3C,   0x5C,   0xFC,   0x0F
};

//cga 2
const unsigned char paletteCMWdim[16] = {
//  Black   Cyan    Magenta White - others don't matter
    0x00,   0x99,   0x64,   0x06, 0x34, 0x54, 0xF4, 0x07, 0x03,
    0x8C,   0xCC,   0xAC,   0x3C, 0x5C, 0xFC, 0x0F
};

const unsigned char paletteCMWbright[16] = {
//  Black   Cyan    Magenta White - others don't matter
    0x00,   0x9B,   0x67,   0x09, 0x34, 0x54, 0xF4, 0x07, 0x03,
    0x8C,   0xCC,   0xAC,   0x3C, 0x5C, 0xFC, 0x0F
};
//PCJR
const unsigned char gb_color_pcjr[16]={ 
 0x00,0x15,0x2A,0x3F,0x21,0x19,0x10,0x1E,
 0x05,0x01,0x16,0x15,0x15,0x2E,0x25,0x2A 
};


//Escala grises cambiado orden 1 2 por 2 1
const unsigned char gb_color_cgagray[16]={ 
 0x00,0x2A,0x15,0x3F,0x21,0x19,0x10,0x1E,
 0x05,0x01,0x16,0x15,0x15,0x2E,0x25,0x2A 
};

//Escala Gris static Rapido
static unsigned char palette[16]={
    0x00,   0x84,   0xC4,   0xA4,   0x34,   0x54,   0xF4,   0x07,
    0x03,   0x8C,   0xCC,   0xAC,   0x3C,   0x5C,   0xFC,   0x0F
};

//Color Modo Texto Rapido
static unsigned char gb_color_text_cga[16]={ 
// BLACK    BLUE    GREEN   CYAN    RED     MGNTA   YELLOW  WHITE
    0x00,   0x84,   0xC4,   0x99,   0x44,   0x54,   0xF4,   0x07,
    0x03,   0x87,   0xC7,   0x9B,   0x47,   0x57,   0xF7,   0x0F
};

static struct render {
  uint32_t dumper = 0;
  uint32_t colCount = 80;
  uint32_t frameCount = 0;
  uint32_t paletteIndex = 0;
  uint32_t specialColor = 0;
  uint32_t pendingColorburstValue = PENDING_COLORBURST_NO;
} render;


CompositeColorOutput composite(CompositeColorOutput::NTSC);

char **gb_buffer_vga;

uint32_t cursor_t::row;
uint32_t cursor_t::col;
uint32_t cursor_t::value;
uint32_t cursor_t::start;
uint32_t cursor_t::end;

void cursor_t::updateMSB(uint8_t MSB)
{
  value = (value & 0x00FF) | (MSB << 8);
  updatePosition();
};

void cursor_t::updateLSB(uint8_t LSB)
{
  value = (value & 0xFF00) | (LSB);
  updatePosition();
};

void cursor_t::updateStart(uint32_t startLine)
{
  start = startLine;
}

void cursor_t::updateEnd(uint32_t endLine)
{
  end = endLine;
}

uint32_t __always_inline cursor_t::getCol()
{
  return col;
};

uint32_t __always_inline cursor_t::getRow()
{
  return row;
};

uint32_t __always_inline cursor_t::getStart()
{
  return start;
};

uint32_t __always_inline cursor_t::getEnd()
{
  return end;
};

void cursor_t::updatePosition()
{
  row = value / render.colCount;
  col = value % render.colCount;
};

void renderInit() {
  gb_buffer_vga = (char **)malloc(CompositeColorOutput::YRES * sizeof(char *));
  for (int y = 0; y < CompositeColorOutput::YRES; y++)
  {
    gb_buffer_vga[y] = (char *)malloc(CompositeColorOutput::XRES * 2);
    memset(gb_buffer_vga[y], 0x77, CompositeColorOutput::XRES * 2);
  }

  void IRAM_ATTR blitter_0(uint8_t * src, uint16_t * dst);
  void IRAM_ATTR blitter_1(uint8_t * src, uint16_t * dst);
  composite.init(&gb_buffer_vga, &blitter_1);
}

void renderExec()
{
  if (render.pendingColorburstValue != PENDING_COLORBURST_NO)
  {
    composite.setColorburstEnabled(render.pendingColorburstValue == PENDING_COLORBURST_TRUE);
    render.pendingColorburstValue = PENDING_COLORBURST_NO;
  }
}

void renderClearScreen()
{
  for (int y = 0; y < 200; y++)
    for (int x = 0; x < 320; x++)
      gb_buffer_vga[y + VERTICAL_OFFSET][x] = 0;
}

void renderPrintCharOSD(char character, int col, int row, unsigned char color, unsigned char backcolor)
{
  // unsigned char aux = gb_sdl_font_6x8[(car-64)];
  int auxId = character << 3; //*8
  unsigned char pixel;
  for (uint32_t y = 0; y < 8; y++)
  {
    uint8_t aux = gb_sdl_font_8x8[auxId + y];
    for (uint32_t x = 0; x < 8; x++)
    {
      pixel = ((aux >> x) & 0x01);
      const uint32_t line = row + y + VERTICAL_OFFSET;
      const uint32_t column = col + (8 - x);
      gb_buffer_vga[line][column] = (pixel == 1) ? color : backcolor;
    }
  }
}

void PreparaPaleta()
{
 InitPaletaCGA();
}

//uint8_t initscreen (uint8_t *ver) 
unsigned char initscreen() 
{
	#ifdef use_lib_force_sdl_8bpp
 	 //JJ no SDL screen = SDL_SetVideoMode (640, 400, 8, SDL_SWSURFACE | SDL_DOUBLEBUF); 	 
     PreparaPaleta();     
 	#else 
 	 screen = SDL_SetVideoMode (640, 400, 32, SDL_SWSURFACE | SDL_DOUBLEBUF);
 	#endif
 	//JJ no SDL SDL_WM_SetCaption ("ESP32 Fake86", NULL);
	#ifdef use_lib_log_serial
	 Serial.printf("initscreen SDL_SetVideoMode\n");
	#endif 
	return (1);
}

void draw();
extern void handleinput();

void VideoThreadPoll()
{
 draw();
}

//*****************************************
static void dump160x100_font4x8()
{
 unsigned char aColor,aBgColor,aChar;
 unsigned int bFourPixelsOffset=0;     
 for (int y=0;y<100;y++)
 {  
  for (unsigned char x=0;x<80;x++) //Modo 80x25
  {
   aChar= gb_video_cga[bFourPixelsOffset];
   bFourPixelsOffset++;
   aColor = gb_video_cga[bFourPixelsOffset]&0x0F;
   aBgColor = ((gb_video_cga[bFourPixelsOffset]>>4)&0x0F);   
   SDLprintChar160x100_font4x8(aChar,(x<<2),(y*2),aColor,aBgColor);  //40x25
   bFourPixelsOffset++;
  } 
 }
}

static void dump80x25_font4x8()
{
  unsigned char aColor,aBgColor,aChar,swapColor;;
  unsigned int nOffset=0;

  // if ( (port_3D8->value==9) && (port_3D4->value==9))
  // {
  //   SDLdump160x100_font4x8();
  //   return;
  // }
  
 for (int y=0;y<25;y++)
 {  
    for (int x=0;x<80;x++) //Modo 80x25
    {
      aChar= gb_video_cga[nOffset];
      nOffset++;
      aColor = gb_video_cga[nOffset]&0x0F;
      aBgColor = ((gb_video_cga[nOffset]>>4)&0x07);

   if (gb_invert_color == 1)
   {
    swapColor= aColor;
    aColor= aBgColor;
    aBgColor= swapColor;
   }   

    SDLprintChar4x8(aChar,(x<<2),(y<<3),aColor,aBgColor);//Sin capturadora
    nOffset++;    
  }
 }
}

static void dump160x100_font8x8()
{
 unsigned char aColor,aBgColor,aChar;
 unsigned int bFourPixelsOffset=0;     
 for (int y=0;y<100;y++)
 {  
  for (unsigned char x=0;x<80;x++) //Modo 40x25
  {
   aChar= gb_video_cga[bFourPixelsOffset];
   bFourPixelsOffset++;
   aColor = gb_video_cga[bFourPixelsOffset]&0x0F;   
   aBgColor = ((gb_video_cga[bFourPixelsOffset]>>4)&0x0F);   
   SDLprintChar160x100_font8x8(aChar,(x<<3),(y*2),aColor,aBgColor);  //40x25
   bFourPixelsOffset++;
  }
  bFourPixelsOffset+= 80; //40x25  
 }
}

static void dump80x25_font8x8()
{
//  if ((port_3D8->value == 9) && (port_3D4->value == 9))
//  {
//   SDLdump160x100_font8x8();
//   return;
//  }

 unsigned char aColor, aBgColor, aChar;
 uint32_t bOffset = 0;
 for (uint32_t y = 0; y < 25; y++)
 {
  for (uint32_t x = 0; x < 80; x++)
  {
   aChar = gb_video_cga[bOffset];
   bOffset++;
   aColor = gb_video_cga[bOffset] & 0x0F;
   aBgColor = ((gb_video_cga[bOffset] >> 4) & 0x07);
   SDLprintChar(aChar, (x << 3), (y << 3), aColor, aBgColor); // Sin capturadora
   bOffset++;
  }
 }
}

static void dump40x25_font8x8()
{
 uint32_t bOffset = 0;
 for (uint32_t y = 0; y < 25; y++)
 {
  for (uint32_t x = 0; x < 40; x++)
  {
    uint8_t aChar = gb_video_cga[bOffset];
    bOffset++;
    uint8_t aColor = gb_video_cga[bOffset] & 0x0F;
    uint8_t aBgColor = ((gb_video_cga[bOffset] >> 4) & 0x07);
    SDLprintChar(aChar, (x << 3), (y << 3), aColor, aBgColor); // Sin capturadora
    bOffset++;
  }
 }
}

static void dump320x200()
{
    unsigned short int cont=0;
  for (uint32_t y=0; y<100; y++)
  {        
		uint32_t yDest= (y<<1);
        uint32_t *pLine = (uint32_t *)gb_buffer_vga[yDest + VERTICAL_OFFSET];
        for (uint32_t x=0;x<80;x++)   
		{//Lineas impares
			unsigned char bFourPixels = gb_video_cga[cont];   
			unsigned char bPixel3 = (bFourPixels & 0x03); //empieza izquierda derecha pixel
            unsigned char bPixel2 = ((bFourPixels >> 2) & 0x03);
            unsigned char bPixel1 = ((bFourPixels >> 4) & 0x03);
            unsigned char bPixel0 = ((bFourPixels >> 6) & 0x03);

            uint32_t a32= (palette[bPixel0]) | (palette[bPixel1]<<8) | (palette[bPixel2]<<16) | (palette[bPixel3]<<24);
			//ptr32[x]= a32;
			scanlineBuffer[x]= a32;

			cont++;
   }
   memcpy(pLine+2,scanlineBuffer,320);
  } 

  cont = 0x2000;   
  for (uint32_t y=0;y<100;y++)
  {      
   uint32_t yDest= (y<<1)+1;
   uint32_t *pLine = (uint32_t *)gb_buffer_vga[yDest + VERTICAL_OFFSET];
   for (uint32_t x=0;x<80;x++)
   {//Lineas impares
            unsigned char bFourPixels = gb_video_cga[cont];
            unsigned char bPixel3 = (bFourPixels & 0x03); // empieza izquierda derecha pixel
            unsigned char bPixel2 = ((bFourPixels >> 2) & 0x03);
            unsigned char bPixel1 = ((bFourPixels >> 4) & 0x03);
            unsigned char bPixel0 = ((bFourPixels >> 6) & 0x03);

            uint32_t a32 = (palette[bPixel0]) | (palette[bPixel1] << 8) | (palette[bPixel2] << 16) |
                           (palette[bPixel3] << 24);
            scanlineBuffer[x] = a32;

            cont++;
   }
   memcpy(pLine+2, scanlineBuffer,320);
  } 
}


//cga6 rapido
static void dump640x200()
{//640x200 1 bit Escalado a la mitad
 unsigned short int cont=0;
 unsigned int yDest; 
 unsigned int x;
 unsigned char y;
 unsigned int *ptr32;
 unsigned int a32;
   

  for (y=0;y<100;y++)
  {        
   yDest= (y<<1);
   ptr32= (unsigned int *)gb_buffer_vga[yDest + VERTICAL_OFFSET];
   for (x=0;x<80;x++)   
   {
    unsigned char bOneByte = gb_video_cga[cont];
    uint8_t a6 = ((bOneByte >> 1) & 0x01); // empieza izquierda derecha pixel
    uint8_t a4 = ((bOneByte >> 3) & 0x01);
    uint8_t a2 = ((bOneByte >> 5) & 0x01);
    uint8_t a0= ((bOneByte>>7)& 0x01);

    a0= (a0==0?0:3); //Deberia ser 15, por ahora 3 de 4 colores
    a2= (a2==0?0:3);
    a4= (a4==0?0:3);
    a6= (a6==0?0:3);

	a32= (palette[a0]) | (palette[a2]<<8) | (palette[a4]<<16) | (palette[a6]<<24);
	//ptr32[x]= a32;
	scanlineBuffer[x]= a32;

    cont++;
   }
   memcpy(ptr32 + 2,scanlineBuffer,320);
  } 

  cont = 0x2000;   
  for (y=0;y<100;y++)
  {      
   yDest= (y<<1)+1;
   ptr32= (unsigned int *)gb_buffer_vga[yDest + VERTICAL_OFFSET];  
   for (x=0;x<80;x++)
   {//Lineas impares
    unsigned char bOneByte = gb_video_cga[cont];
    uint8_t a6 = ((bOneByte >> 1) & 0x01); // empieza izquierda derecha pixel
    uint8_t a4 = ((bOneByte >> 3) & 0x01);
    uint8_t a2 = ((bOneByte >> 5) & 0x01);
    uint8_t a0 = ((bOneByte >> 7) & 0x01);

    a0= (a0==0?0:3);//Deberia ser 15, por ahora 3 de 4 colores
    a2= (a2==0?0:3);
    a4= (a4==0?0:3);
    a6= (a6==0?0:3);
   	
	a32= (palette[a0]) | (palette[a6]<<2) | (palette[a4]<<16) | (palette[a6]<<24);
	//ptr32[x]= a32;
	scanlineBuffer[x]= a32;

    cont++;
   }
   memcpy(ptr32 + 2,scanlineBuffer,320);
  } 
}

//*************************************************************************************
static void SDLprintChar_c(char code,int x,int y,unsigned char color,unsigned char backcolor)
{ 
 int nBaseOffset = code << 3;
 const bool cbTime = render.frameCount & 0x04;
 for (unsigned int row=0; row<8; row++)
 {
    const bool cbFill = (row >= cursor.getStart()) && (row <= cursor.getEnd()) && cbTime;
     unsigned char bLine = ((row >= 6) && (cbTime))?0xFF:gb_sdl_font_8x8[nBaseOffset + row];
     for (int col = 0; col < 8; col++) {
      unsigned char Pixel = ((bLine >> col) & 0x01);
      const uint32_t vgaLine = y + row + VERTICAL_OFFSET;
      const uint32_t vgaCol = x - col + 15;
      gb_buffer_vga[vgaLine][vgaCol] = gb_color_text_cga[(Pixel != 0) ? color : backcolor];
  }
 }
}

static void SDLprintChar(char code, int x, int y, unsigned char color, unsigned char backcolor)
{
  if((x == (cursor.getCol() << 3)) && (y == (cursor.getRow() << 3)))
  {
    SDLprintChar_c(code, x, y, color, backcolor);
  }
  else
  {
    // unsigned char bFourPixels = gb_sdl_font_6x8[(car-64)];
    int nBaseOffset = code << 3; //*8
    for (unsigned int row = 0; row < 8; row++)
    {
      unsigned char bLine = gb_sdl_font_8x8[nBaseOffset + row];
      for (int col = 0; col < 8; col++)
      {
          unsigned char Pixel = ((bLine >> col) & 0x01);
          const uint32_t vgaLine = y + row + VERTICAL_OFFSET;
          const uint32_t vgaCol = x - col + 15;
          gb_buffer_vga[vgaLine][vgaCol] = gb_color_text_cga[(Pixel != 0) ? color : backcolor];
      }
    }
  }
}

static void SDLprintChar160x100_font4x8(char car,int x,int y,unsigned char color,unsigned char backcolor)
{
 unsigned char bFourPixels;
 unsigned char bFourPixelsBit,bFourPixelsColor;
 unsigned char aColor;
 unsigned char nibble0;
 unsigned char nibble1;
 switch (car)
 {
  case 221: nibble0=color; nibble1=backcolor; break;
  case 222: nibble0=backcolor; nibble1=color; break;
  default: nibble0=0; nibble1=0; break;  
 }
 for (unsigned char j=0;j<2;j++)
 {
  //bFourPixels = gb_sdl_font_8x8[bFourPixelsId + j];  
  for (int i=0;i<2;i++)
  {//4 primeros pixels
         gb_buffer_vga[(y + j) + VERTICAL_OFFSET][(x + i)] = gb_color_text_cga[nibble0];
  }
  for (int i=2;i<4;i++)
  {//4 segundos pixels
         gb_buffer_vga[(y + j) + VERTICAL_OFFSET][(x + i)] = gb_color_text_cga[nibble1];
  }  
 }
}     

static void SDLprintChar4x8(char car,int x,int y,unsigned char color,unsigned char backcolor)
{ 
// unsigned char bFourPixels = gb_sdl_font_6x8[(car-64)];
 int nBaseOffset = car << 3; //*8
 for (unsigned char row=0;row<8;row++)
 {  
  uint8_t Line = gb_sdl_font_4x8[nBaseOffset + row];  
  for (int i=4;i<8;i++)
  {
   uint8_t Pixel = ((Line>>i) & 0x01);
   //jj_fast_putpixel(x+(7-i),y+j,(bFourPixelsColor==1)?color:backcolor);
   gb_buffer_vga[(y+row) + VERTICAL_OFFSET][(x+(7-i)) + 8]= gb_color_text_cga[((Pixel == 0)?color:backcolor)];
  }
 }
}

static void SDLprintChar160x100_font8x8(char car,int x,int y,unsigned char color,unsigned char backcolor)
{
 unsigned char bFourPixels;
 unsigned char bFourPixelsBit,bFourPixelsColor;
 unsigned char aColor;
 unsigned char nibble0;
 unsigned char nibble1; 
 switch (car)
 {
  case 221: nibble0=color; nibble1=backcolor; break;
  case 222: nibble0=backcolor; nibble1=color; break;
  default: nibble0=0; nibble1=0; break;  
 }
 for (unsigned char j=0;j<2;j++)
 {
  //bFourPixels = gb_sdl_font_8x8[bFourPixelsId + j];  
  for (int i=0;i<4;i++)
  {//4 primeros pixels      
   gb_buffer_vga[(y+j) + VERTICAL_OFFSET][(x+i)]= gb_color_text_cga[nibble0];
  }
  for (int i=4;i<8;i++)
  {//4 segundos pixels
   gb_buffer_vga[(y+j) + VERTICAL_OFFSET][(x+i)]= gb_color_text_cga[nibble1];
  }  
 }     
}

void draw()
{
  render.frameCount++;
  dumpers[render.dumper]();
}

//******************************************
void InitPaletaCGA()
{
 memcpy(palette,paletteGRYdim,16);
}

void InitPaletaCGA2()
{
 memcpy(palette,paletteCMWdim,16);
}

void InitPaletaCGAgray()
{
 memcpy(palette,gb_color_cgagray,16);
}

void InitPaletaPCJR()
{
 memcpy(palette,gb_color_pcjr,16);
}

void IRAM_ATTR blitter_0(uint8_t *src, uint16_t *dst)
{
  const unsigned int *destPalette = RawCompositeVideoBlitter::_palette;
  static const uint32_t STEP = 2;

  uint16_t *dest_16 = dst + 32;
  for (int i = 0; i < RawCompositeVideoBlitter::NTSC_DEFAULT_WIDTH << 1; i += STEP)
  {
    dest_16[0] = (uint16_t)(destPalette[src[0]]);
    dest_16[1] = (uint16_t)(destPalette[src[1]] << 8);
    dest_16 += STEP;
    src += STEP;
  }
}

void IRAM_ATTR blitter_1(uint8_t *src, uint16_t *dst)
{
  const unsigned int *destPalette = RawCompositeVideoBlitter::_palette;
  static const uint32_t STEP = 4;

  uint32_t *d = (uint32_t *)dst + 16;
  for (int i = 0; i < RawCompositeVideoBlitter::NTSC_DEFAULT_WIDTH; i += STEP)  // 84 steps, 4 pixels per step
  {
   d[0] = destPalette[src[0]];
   d[1] = destPalette[src[1]] << 8;
   d[2] = destPalette[src[2]];
   d[3] = destPalette[src[3]] << 8;
   d += STEP;
   src += STEP;
  }
}

void IRAM_ATTR blitter_2(uint8_t *src, uint16_t *dst)
{
  const unsigned int *destPalette = RawCompositeVideoBlitter::_palette;
  static const uint32_t STEP = 4;

  uint32_t *dest_32 = (uint32_t *)dst + 16;
  for (int i = 0; i < RawCompositeVideoBlitter::NTSC_DEFAULT_WIDTH; i += STEP) // 84 steps, 4 pixels per step
  {
    uint32_t c = *((uint32_t *)src); // screen may be in 32 bit mem
    dest_32[0] = destPalette[(uint8_t)(c >>  0)];
    dest_32[1] = destPalette[(uint8_t)(c >>  8)] << 8;
    dest_32[2] = destPalette[(uint8_t)(c >> 16)];
    dest_32[3] = destPalette[(uint8_t)(c >> 24)] << 8;
   dest_32 += STEP;
   src += STEP;
  }
}

void renderSetBlitter(unsigned int blitter)
{
  switch(blitter)
  {
    case 0:
      composite.setBlitter(blitter_0);
      break;
    case 1:
      composite.setBlitter(blitter_1);
      break;
    case 2:
      composite.setBlitter(blitter_2);
      break;
  }
}

void renderSetColorEnabled(bool bEnabled)
{
  render.pendingColorburstValue = bEnabled?PENDING_COLORBURST_TRUE:PENDING_COLORBURST_FALSE;
  LOG("SetColorEnabled(%i)\n", bEnabled);
}

void renderSetColumnCount(uint32_t columnCount)
{
  render.colCount = columnCount;
}

static void bar(int orgX, int orgY, int height, int width, uint8_t color)
{
  for (int y = 0; y < height; y++)
  {
    int scanline = orgY + y + VERTICAL_OFFSET;
    for (int x = 0; x < width; x++)
    {
      int col = orgX + x;
      gb_buffer_vga[scanline][col] = color;
    }
  }
}

void renderUpdateColorSettings(uint32_t paletteIndex, uint32_t color)
{
  render.paletteIndex = paletteIndex;
  render.specialColor = color;

  LOG("updateColorSettings(%02x, %02x)\n", paletteIndex, color);
  enum mode {TEXT, GRAPH_LO, GRAPH_HI} mode = (render.dumper <= 4)?TEXT:((render.dumper == 5)?GRAPH_LO:GRAPH_HI);
  switch(mode)
  {
    case TEXT:
      memcpy(palette, gb_color_text_cga, 16);
      break;
    case GRAPH_LO:
      if(paletteIndex == 0)
        memcpy(palette, paletteGRYdim, 4);
      else if(paletteIndex == 1)
        memcpy(palette, paletteGRYbright, 4);
      else if(paletteIndex == 2)
        memcpy(palette, paletteCMWdim, 4);
      else if(paletteIndex == 3)
        memcpy(palette, paletteCMWbright, 4);
      palette[0] = gb_color_text_cga[color];
      break;
    case GRAPH_HI:
      palette[0] = 0;
      palette[1] = color;
      break;
  }

}

void renderUpdateDumper(uint32_t dumper)
{
  if(render.dumper != dumper)
  {
    if(dumper < DUMPER_COUNT)
    {
      renderUpdateColorSettings(render.paletteIndex, render.specialColor);
      render.dumper = dumper;
      LOG("Dumper #%i\n", dumper);
    }
    else
      LOG("Dumper #%i - FAIL!\n", dumper);
  }
}

void ShowColorTable()
{
  static const int WIDTH = 20;
  static const int HEIGHT = 10;
  for (int hue = 0; hue < 16; hue++)
  {
    for (int luma = 0; luma < 16; luma++)
    {
      int orgX = luma * WIDTH;
      int orgY = hue * HEIGHT;
      uint8_t color = ((uint8_t)hue << 4) | ((uint8_t)luma & 0x0F);
      bar(orgX, orgY, HEIGHT, WIDTH, color);
    }
  }
}
