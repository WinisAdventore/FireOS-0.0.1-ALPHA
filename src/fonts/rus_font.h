#ifndef RUS_FONT_H
#define RUS_FONT_H

// Подключаем сконвертированный русский шрифт
#include "ruscii_8x16.h"

static const unsigned char* current_font = (const unsigned char*)ruscii_8x16_data;
static int font_width = 8;
static int font_height = 16;

// Нарисовать символ
void draw_char(int x, int y, unsigned char c, unsigned int color) {
    extern void pxl(int x, int y, unsigned int color);
    extern int sw, sh;
    
    if(c >= RUSCII_8X16_COUNT) c = '?';
    
    const unsigned char* glyph = &current_font[c * font_height];
    
    for(int iy = 0; iy < font_height; iy++) {
        unsigned char row = glyph[iy];
        for(int ix = 0; ix < font_width; ix++) {
            if(row & (1 << (7 - ix))) {
                int px = x + ix;
                int py = y + iy;
                if(px >= 0 && px < sw && py >= 0 && py < sh)
                    pxl(px, py, color);
            }
        }
    }
}

// Нарисовать строку
void draw_text(int x, int y, const char* text, unsigned int color) {
    while(*text) {
        draw_char(x, y, (unsigned char)*text, color);
        x += font_width;
        text++;
    }
}

#endif
