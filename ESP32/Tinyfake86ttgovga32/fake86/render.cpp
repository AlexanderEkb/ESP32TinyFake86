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
#include <stdint.h>
#include <stdio.h>
#include <Arduino.h>
//JJ #include "mutex.h"
#include "gbGlobals.h"
#include "cpu.h"
#include "render.h"
#include "fake86.h"
#include "gb_sdl_font8x8.h"
#include "gb_sdl_font4x8.h"
#include "render.h"

#define uint32_t int

static const uint32_t VERTICAL_OFFSET = 20;

//cga 1
const unsigned char gb_color_cga[16]={ 
//  Black   Green   Red     Yellow - others don't matter
    0x00,   0xD6,   0x45,   0xFA,   0x34,   0x54,   0xF4,   0x07,
    0x03,   0x8C,   0xCC,   0xAC,   0x3C,   0x5C,   0xFC,   0x0F
};

//cga 2
const unsigned char gb_color_cga2[16] = {
//  Black   Cyan    Magenta White - others don't matter
    0x00,   0xB6,   0x64,   0x06, 0x34, 0x54, 0xF4, 0x07, 0x03,
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
static unsigned char gb_color_vga[16]={
    0x00,   0x84,   0xC4,   0xA4,   0x34,   0x54,   0xF4,   0x07,
    0x03,   0x8C,   0xCC,   0xAC,   0x3C,   0x5C,   0xFC,   0x0F
};

//Color Modo Texto Rapido
static unsigned char gb_color_text_cga[16]={ 
// BLACK    BLUE    GREEN   CYAN    RED     MGNTA   YELLOW  WHITE
    0x00,   0x84,   0xC4,   0xA4,   0x34,   0x54,   0xF4,   0x07,
    0x03,   0x8C,   0xCC,   0xAC,   0x3C,   0x5C,   0xFC,   0x0F
};
extern uint16_t cursx, cursy, cols, rows, vgapage, cursorposition, cursorvisible;
extern uint8_t clocksafe, port6, portout16;
extern uint32_t videobase, textbase;
extern uint32_t usefullscreen;
uint64_t totalframes = 0;
char windowtitle[128];

int gb_cont_rgb=0;
//**************************
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

uint32_t nw, nh; //native width and height, pre-stretching (i.e. 320x200 for mode 13h)

extern uint16_t oldw, oldh;
void draw();
extern void handleinput();

void VideoThreadPoll()
{
 draw();
}

void doscrmodechange() 
{
	scrmodechange = 0;
}


inline void jj_fast_putpixel(int x,int y,unsigned char c)
{
 gb_buffer_vga[y + VERTICAL_OFFSET][x]= gb_color_vga[c];
}

extern uint16_t vtotal;

//*************************************************************************************
void SDLprintChar(char car,int x,int y,unsigned char color,unsigned char backcolor)
{ 
// unsigned char bFourPixels = gb_sdl_font_6x8[(car-64)];
 int nBaseOffset = car << 3; //*8
 for (unsigned char j=0;j<8;j++)
 {
     unsigned char bLine = gb_sdl_font_8x8[nBaseOffset + j];
     for (int i = 0; i < 8; i++) {
         unsigned char Pixel = ((bLine >> i) & 0x01);
         gb_buffer_vga[(y + j) + VERTICAL_OFFSET][(x + (7 - i)) + 8] = gb_color_text_cga[(Pixel != 0) ? color : backcolor];
  }
 }
}


//*****************************************
void SDLprintChar160x100_font4x8(char car,int x,int y,unsigned char color,unsigned char backcolor)
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

//*****************************************
void SDLdump160x100_font4x8()
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

//*****************************************
void SDLdump80x25_font4x8()
{
  unsigned char aColor,aBgColor,aChar,swapColor;;
  unsigned int bFourPixelsOffset=0;

  if ( (gb_portramTiny[fast_tiny_port_0x3D8]==9) && (gb_portramTiny[fast_tiny_port_0x3D4]==9) )
  {
    SDLdump160x100_font4x8();
    return;
  }
  
 for (int y=0;y<25;y++)
 {  
    for (int x=0;x<80;x++) //Modo 80x25
    {
      aChar= gb_video_cga[bFourPixelsOffset];
      bFourPixelsOffset++;
      aColor = gb_video_cga[bFourPixelsOffset]&0x0F;
      aBgColor = ((gb_video_cga[bFourPixelsOffset]>>4)&0x07);

   if (gb_invert_color == 1)
   {
    swapColor= aColor;
    aColor= aBgColor;
    aBgColor= swapColor;
   }   

    SDLprintChar4x8(aChar,(x<<2),(y<<3),aColor,aBgColor);//Sin capturadora
    bFourPixelsOffset++;    
  }
 }
}

//*****************************************
void SDLprintChar4x8(char car,int x,int y,unsigned char color,unsigned char backcolor)
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


//*****************************************************
void SDLprintChar160x100_font8x8(char car,int x,int y,unsigned char color,unsigned char backcolor)
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

//*****************************************************
void SDLdump160x100_font8x8()
{
 unsigned char aColor,aBgColor,aChar;
 unsigned int bFourPixelsOffset=0;     
 for (int y=0;y<100;y++)
 {  
  for (unsigned char x=0;x<40;x++) //Modo 40x25
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

//*****************************************
void SDLdump80x25_font8x8()
{//Muestro solo 40 columnas
 unsigned char aColor,aBgColor,aChar;
 unsigned int bOffset=0;
 if ( (gb_portramTiny[fast_tiny_port_0x3D8]==9) && (gb_portramTiny[fast_tiny_port_0x3D4]==9) ) 
 {
  //printf("Modo PAKUPAKU\n");
  //fflush(stdout);
  SDLdump160x100_font8x8();
  return;
 }

 for (unsigned char y=0;y<25;y++)
 {
  //for (unsigned char x=0;x<80;x++)
  for (unsigned char x=0;x<40;x++)
  {
   aChar= gb_video_cga[bOffset];
   bOffset++;
   aColor = gb_video_cga[bOffset]&0x0F;
   aBgColor = ((gb_video_cga[bOffset]>>4)&0x07);
   #ifdef use_lib_capture_usb
    if (x<79){//Para verlo en capturadora    
     SDLprintChar(aChar,((x+1)<<3),(y<<3),aColor,aBgColor);  //Capturadora usb
    }
   #else
    SDLprintChar(aChar,(x<<3),(y<<3),aColor,aBgColor); //Sin capturadora
   #endif
   bOffset++;
  }
  bOffset+= 80;
 } 
}

static unsigned int gb_local_scanline[80];

void jj_sdl_dump_cga_320x200()
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

            uint32_t a32= (gb_color_vga[bPixel0]) | (gb_color_vga[bPixel1]<<8) | (gb_color_vga[bPixel2]<<16) | (gb_color_vga[bPixel3]<<24);
			//ptr32[x]= a32;
			gb_local_scanline[x]= a32;

			cont++;
   }
   memcpy(pLine+2,gb_local_scanline,320);
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

            uint32_t a32 = (gb_color_vga[bPixel0]) | (gb_color_vga[bPixel1] << 8) | (gb_color_vga[bPixel2] << 16) |
                           (gb_color_vga[bPixel3] << 24);
            gb_local_scanline[x] = a32;

            cont++;
   }
   memcpy(pLine+2, gb_local_scanline,320);
  } 
}


//cga6 rapido
void jj_sdl_dump_640x200()
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

	a32= (gb_color_vga[a0]) | (gb_color_vga[a2]<<8) | (gb_color_vga[a4]<<16) | (gb_color_vga[a6]<<24);
	//ptr32[x]= a32;
	gb_local_scanline[x]= a32;

    cont++;
   }
   memcpy(ptr32 + 2,gb_local_scanline,320);
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
   	
	a32= (gb_color_vga[a0]) | (gb_color_vga[a6]<<2) | (gb_color_vga[a4]<<16) | (gb_color_vga[a6]<<24);
	//ptr32[x]= a32;
	gb_local_scanline[x]= a32;

    cont++;
   }
   memcpy(ptr32 + 2,gb_local_scanline,320);
  } 
}

void draw () 
{
 int x,y;
 
	uint32_t planemode, vgapage, color, chary, charx, vidptr, divx, divy, curchar, curpixel, usepal, intensity, blockw, curheight, x1, y1;

	switch (vidmode) 
    {
        case 0:
        case 1:
        case 2: //text modes
        case 3:
        case 7:
   	    case 0x82:
			 if (gb_font_8x8 == 1)
			  SDLdump80x25_font8x8();
			 else 
			  SDLdump80x25_font4x8();
             break;
        case 4:
        case 5:
            jj_sdl_dump_cga_320x200();
            break;
        case 6:
            jj_sdl_dump_640x200();
            break;
        case 127:
            nw = 720;
            nh = 348;
            for (y = 0; y < 348; y++) {
                for (x = 0; x < 720; x++) {
                    charx = x;
                    chary = y >> 1;
                    vidptr = videobase + ((y & 3) << 13) + (y >> 2) * 90 + (x >> 3);
                    curpixel = (read86(vidptr) >> (7 - (charx & 7))) & 1;
                    color = curpixel ? 0x00FFFFFF : 0x00000000;
                    jj_fast_putpixel((x >> 2), (y >> 1), color);
                }
            }
            break;
        case 0x8: //160x200 16-color (PCjr)
            nw = 640; // fix this
            nh = 400; //part later
            for (y=0; y<400; y++)
                for (x=0; x<640; x++) {
                    vidptr = 0xB8000 + (y>>2) *80 + (x>>3) + ( (y>>1) &1) *8192;
                    if ( ( (x>>1) &1) ==0)
                    {
                        //color = palettecga[RAM[vidptr] >> 4];
                        color = palettecga[read86(vidptr) >> 4];
                    }
                    else
                    {
                        //color = palettecga[RAM[vidptr] & 15];
                        color = palettecga[read86(vidptr) & 15];
                    }
                    //JJ prestretch[y][x] = color; //no necesito escalar
                    jj_fast_putpixel((x>>1),(y>>1),color);
                }
            break;
        case 0x9: //320x200 16-color (Tandy/PCjr)
            nw = 640; // fix this
            nh = 400; //part later
            for (y=0; y<400; y++)
                for (x=0; x<640; x++) {
                    vidptr = 0xB8000 + (y>>3) *160 + (x>>2) + ( (y>>1) &3) *8192;
                    if ( ( (x>>1) &1) ==0)
                    {
                        //color = palettecga[RAM[vidptr] >> 4];
                        color = palettecga[read86(vidptr) >> 4];
                    }
                    else
                    {
                        //color = palettecga[RAM[vidptr] & 15];
                        color = palettecga[read86(vidptr) & 15];
                    }
                    jj_fast_putpixel((x>>1),(y>>1),color);
                }
				break;
			case 0xD:
			case 0xE:
                nw = 640; // fix this
                nh = 400; //part later
				for (y=0; y<400; y++)
					for (x=0; x<640; x++) {
							divx = x>>1;
							divy = y>>1;
							vidptr = divy*40 + (divx>>3);
							x1 = 7 - (divx & 7);
							jj_fast_putpixel((x>>1),(y>>1),color);
						}
				break;
			case 0x10:
				nw = 640;
				nh = 350;
				for (y=0; y<350; y++)
					for (x=0; x<640; x++) {
							vidptr = y*80 + (x>>3);
							x1 = 7 - (x & 7);
							jj_fast_putpixel((x>>1),(y>>1),color);
						}
				break;
      default:
       break;
	} //Fin switch vidmode
}



//******************************************
void PreparaColorVGA()
{
 #ifdef use_lib_bitluni_fast
  //Modo grafico CGA
  for (unsigned char i=0;i<16;i++)
  {
   gb_color_vga[i] = gb_color_vga[i] | gb_sync_bits;
  }

  //Modo texto cga
  for (unsigned char i=0;i<16;i++)
  {
   gb_color_text_cga[i] = gb_color_text_cga[i] | gb_sync_bits; 
  } 
 #endif 
}

void InitPaletaCGA()
{
 memcpy(gb_color_vga,gb_color_cga,16);
 PreparaColorVGA();
}

void InitPaletaCGA2()
{
 memcpy(gb_color_vga,gb_color_cga2,16);
 PreparaColorVGA();
}

void InitPaletaCGAgray()
{
 memcpy(gb_color_vga,gb_color_cgagray,16);
 PreparaColorVGA();
}

void InitPaletaPCJR()
{
 memcpy(gb_color_vga,gb_color_pcjr,16);
 PreparaColorVGA();
}