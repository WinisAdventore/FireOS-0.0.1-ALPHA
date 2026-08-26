#ifndef FIOGUI_H
#define FIOGUI_H

typedef unsigned char u8;
typedef unsigned int u32;

#define FIOGUI_MAX_WINDOWS 16

typedef struct {
    int x, y, width, height;
    const char* title;
    int visible;
    int dragging;
    int drag_off_x, drag_off_y;
} FIOGUI_Window;

static FIOGUI_Window fiogui_windows[FIOGUI_MAX_WINDOWS];
static int fiogui_count = 0;
static int fiogui_drag_win = -1;

int fiogui_create_window(int x, int y, int w, int h, const char* title) {
    if(fiogui_count >= FIOGUI_MAX_WINDOWS) return -1;
    FIOGUI_Window* win = &fiogui_windows[fiogui_count];
    win->x = x; win->y = y;
    win->width = w; win->height = h;
    win->title = title;
    win->visible = 1;
    win->dragging = 0;
    fiogui_count++;
    return fiogui_count - 1;
}

int fiogui_find_window(int mx, int my) {
    for(int i = fiogui_count - 1; i >= 0; i--) {
        if(!fiogui_windows[i].visible) continue;
        if(mx >= fiogui_windows[i].x && mx < fiogui_windows[i].x + fiogui_windows[i].width &&
           my >= fiogui_windows[i].y && my < fiogui_windows[i].y + fiogui_windows[i].height)
            return i;
    }
    return -1;
}

int fiogui_hit_title(int id, int mx, int my) {
    if(id < 0 || id >= fiogui_count) return 0;
    return (mx >= fiogui_windows[id].x && mx < fiogui_windows[id].x + fiogui_windows[id].width &&
            my >= fiogui_windows[id].y && my < fiogui_windows[id].y + 28);
}

void fiogui_start_drag(int id, int mx, int my) {
    if(id < 0) return;
    fiogui_drag_win = id;
    fiogui_windows[id].dragging = 1;
    fiogui_windows[id].drag_off_x = mx - fiogui_windows[id].x;
    fiogui_windows[id].drag_off_y = my - fiogui_windows[id].y;
}

void fiogui_update_drag(int mx, int my) {
    if(fiogui_drag_win < 0) return;
    fiogui_windows[fiogui_drag_win].x = mx - fiogui_windows[fiogui_drag_win].drag_off_x;
    fiogui_windows[fiogui_drag_win].y = my - fiogui_windows[fiogui_drag_win].drag_off_y;
}

void fiogui_end_drag() {
    if(fiogui_drag_win >= 0) fiogui_windows[fiogui_drag_win].dragging = 0;
    fiogui_drag_win = -1;
}

// Сглаженный пиксель
static void fiogui_pset_aa(int x, int y, u32 color, float alpha) {
    extern void pxl(int x, int y, u32 color);
    extern u8* fb;
    extern int sw, sh;
    
    if(x < 0 || x >= sw || y < 0 || y >= sh) return;
    if(alpha >= 1.0f) { pxl(x, y, color); return; }
    
    u32* pixel = (u32*)(fb + y*sw*4 + x*4);
    u32 bg = *pixel;
    u32 br = (bg >> 16) & 0xFF, bg_g = (bg >> 8) & 0xFF, bb = bg & 0xFF;
    u32 fr = (color >> 16) & 0xFF, fg = (color >> 8) & 0xFF, fb2 = color & 0xFF;
    u32 r = (u32)(fr * alpha + br * (1.0f - alpha));
    u32 g = (u32)(fg * alpha + bg_g * (1.0f - alpha));
    u32 b = (u32)(fb2 * alpha + bb * (1.0f - alpha));
    *pixel = 0xFF000000 | (r << 16) | (g << 8) | b;
}

// Скруглённый угол (сглаженный)
static void fiogui_round_tl(int cx, int cy, u32 color) {
    float r = 6.0f;
    for(int iy = 0; iy <= (int)r; iy++) {
        for(int ix = 0; ix <= (int)r; ix++) {
            float dx = (float)ix - r;
            float dy = (float)iy - r;
            float dist = dx*dx + dy*dy;
            float r2 = r*r;
            if(dist <= r2) {
                float a = r2 - dist;
                if(a > 1.0f) a = 1.0f;
                fiogui_pset_aa(cx+ix, cy+iy, color, a);
            }
        }
    }
}

static void fiogui_round_tr(int cx, int cy, u32 color) {
    float r = 6.0f;
    for(int iy = 0; iy <= (int)r; iy++) {
        for(int ix = 0; ix <= (int)r; ix++) {
            float dx = (float)ix;
            float dy = (float)iy - r;
            float dist = dx*dx + dy*dy;
            float r2 = r*r;
            if(dist <= r2) {
                float a = r2 - dist;
                if(a > 1.0f) a = 1.0f;
                fiogui_pset_aa(cx+ix, cy+iy, color, a);
            }
        }
    }
}

// Круг
void fiogui_draw_circle(int cx, int cy, int r, u32 fill, u32 border) {
    extern void pxl(int x, int y, u32 color);
    for(int iy = -r; iy <= r; iy++) {
        for(int ix = -r; ix <= r; ix++) {
            int dist = ix*ix + iy*iy;
            if(dist <= r*r) {
                if(dist >= (r-1)*(r-1)) pxl(cx+ix, cy+iy, border);
                else pxl(cx+ix, cy+iy, fill);
            }
        }
    }
}

// Рисуем окно
void fiogui_draw_window(int id) {
    if(id < 0 || id >= fiogui_count || !fiogui_windows[id].visible) return;
    
    extern void rect(int x, int y, int w, int h, u32 color);
    extern void pxl(int x, int y, u32 color);
    extern void draw_text(int x, int y, const char* text, u32 color);
    
    FIOGUI_Window* w = &fiogui_windows[id];
    int x = w->x, y = w->y, ww = w->width, wh = w->height;
    int r = 6;
    
    // Тело окна (белое с скруглением сверху)
    rect(x + r, y, ww - 2*r, wh, 0xFFFFFFFF);
    rect(x, y + r, r, wh - r, 0xFFFFFFFF);
    rect(x + ww - r, y + r, r, wh - r, 0xFFFFFFFF);
    fiogui_round_tl(x, y, 0xFFFFFFFF);
    fiogui_round_tr(x + ww - r, y, 0xFFFFFFFF);
    
    // Заголовок (серый с скруглением)
    rect(x + r, y, ww - 2*r, 28, 0xFFD0D0D0);
    rect(x, y + r, r, 28 - r, 0xFFD0D0D0);
    rect(x + ww - r, y + r, r, 28 - r, 0xFFD0D0D0);
    fiogui_round_tl(x, y, 0xFFD0D0D0);
    fiogui_round_tr(x + ww - r, y, 0xFFD0D0D0);
    
    // Линия
    rect(x, y + 28, ww, 1, 0xFFBBBBBB);
    
    // Кнопки
    fiogui_draw_circle(x + 14, y + 14, 6, 0xFFFF5555, 0xFFCC2222);
    fiogui_draw_circle(x + 38, y + 14, 6, 0xFFFFCC55, 0xFFCCAA22);
    fiogui_draw_circle(x + 62, y + 14, 6, 0xFF55CC55, 0xFF228822);
    
    // Название
    if(w->title) {
        draw_text(x + 80, y + 6, w->title, 0xFF000000);
    }
}

void fiogui_draw_all(void) {
    for(int i = 0; i < fiogui_count; i++) {
        fiogui_draw_window(i);
    }
}

#endif
