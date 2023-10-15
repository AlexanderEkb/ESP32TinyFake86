#ifndef _RENDER_H
 #define _RENDER_H

 void renderInit();
 void renderExec();
 void renderSetBlitter(unsigned int blitter);
 void jj_fast_putpixel(int x, int y, unsigned char c);
 unsigned char initscreen ();
 void SDLprintChar4x8(char car,int x,int y,unsigned char color,unsigned char backcolor);
 void InitPaletaCGA(void);
 void InitPaletaCGA2(void);
 void InitPaletaCGAgray(void);
 void InitPaletaPCJR(void);

#endif

