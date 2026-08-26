#ifndef FONT_DRIVER_LINUX_H
#define FONT_DRIVER_LINUX_H
typedef unsigned char u8;
typedef unsigned int u32;
#include "../ruscii_8x16.h"
static const unsigned char* current_font = (const unsigned char*)ruscii_8x16_data;
static int font_w = 8, font_h = 16;
extern void vesa_pixel(int x, int y, u32 color);
void draw_text(int x, int y, const char* text, u32 color) {
    while(*text) {
        unsigned char c = (unsigned char)*text;
        const u8* glyph = &current_font[c * font_h];
        for(int iy=0; iy<font_h; iy++) { u8 row=glyph[iy]; for(int ix=0; ix<font_w; ix++) if(row&(1<<(7-ix))) vesa_pixel(x+ix, y+iy, color); }
        x+=font_w; text++;
    }
}
void draw_char(int x, int y, unsigned char code, u32 color) {
    const u8* glyph = &current_font[code * font_h];
    for(int iy=0; iy<font_h; iy++) { u8 row=glyph[iy]; for(int ix=0; ix<font_w; ix++) if(row&(1<<(7-ix))) vesa_pixel(x+ix, y+iy, color); }
}
#endif
