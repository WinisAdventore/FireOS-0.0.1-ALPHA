typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

u8 *fb = (u8*)0xFD000000;
int sw = 1920, sh = 1080;
int mx = 960, my = 540, omx = 960, omy = 540;

static inline u8 inb(u16 p) { u8 r; __asm__ volatile("inb %1,%0":"=a"(r):"Nd"(p)); return r; }
static inline void outb(u16 p, u8 v) { __asm__ volatile("outb %0,%1"::"a"(v),"Nd"(p)); }
static inline u16 inw(u16 p) { u16 r; __asm__ volatile("inw %1,%0":"=a"(r):"Nd"(p)); return r; }

void pxl(int x, int y, u32 c) {
    if(x>=0 && x<sw && y>=0 && y<sh)
        *((u32*)(fb + y*sw*4 + x*4)) = c;
}

void rect(int x, int y, int w, int h, u32 c) {
    for(int iy=0; iy<h; iy++) {
        u32* line = (u32*)(fb + (y+iy)*sw*4 + x*4);
        for(int ix=0; ix<w; ix++) line[ix] = c;
    }
}

#include "../fonts/rus_font.h"
#include "../gui/fiogui.h"

int my_strlen(const char* s) { int i=0; while(s[i]) i++; return i; }

// Курсор
#define CURSOR_W 11
#define CURSOR_H 18
static const u8 cursor_data[18][11] = {
    {1,0,0,0,0,0,0,0,0,0,0},{1,1,0,0,0,0,0,0,0,0,0},
    {1,2,1,0,0,0,0,0,0,0,0},{1,2,2,1,0,0,0,0,0,0,0},
    {1,2,2,2,1,0,0,0,0,0,0},{1,2,2,2,2,1,0,0,0,0,0},
    {1,2,2,2,2,2,1,0,0,0,0},{1,2,2,2,2,2,2,1,0,0,0},
    {1,2,2,2,2,2,2,2,1,0,0},{1,2,2,2,2,2,2,2,2,1,0},
    {1,2,2,2,2,2,2,1,1,1,1},{1,2,2,1,2,2,1,0,0,0,0},
    {1,2,1,0,1,2,2,1,0,0,0},{1,1,0,0,0,1,2,2,1,0,0},
    {0,0,0,0,0,1,2,2,1,0,0},{0,0,0,0,0,0,1,2,2,1,0},
    {0,0,0,0,0,0,1,2,2,1,0},{0,0,0,0,0,0,0,1,1,0,0}
};
static u32 cursor_bg[CURSOR_H][CURSOR_W];

void save_cursor_bg() { for(int iy=0;iy<CURSOR_H;iy++)for(int ix=0;ix<CURSOR_W;ix++)cursor_bg[iy][ix]=*((u32*)(fb+(my+iy)*sw*4+(mx+ix)*4)); }
void restore_cursor_bg() { for(int iy=0;iy<CURSOR_H;iy++)for(int ix=0;ix<CURSOR_W;ix++)*((u32*)(fb+(omy+iy)*sw*4+(omx+ix)*4))=cursor_bg[iy][ix]; }
void draw_cursor() { for(int iy=0;iy<CURSOR_H;iy++)for(int ix=0;ix<CURSOR_W;ix++){u8 v=cursor_data[iy][ix];if(v==1)pxl(mx+ix,my+iy,0xFFFFFFFF);else if(v==2)pxl(mx+ix,my+iy,0xFF000000);} }

void mouse_init() {
    while(inb(0x64)&2){} outb(0x64,0xA8);
    while(inb(0x64)&2){} outb(0x64,0xD4);
    while(inb(0x64)&2){} outb(0x60,0xF4);
    while(!(inb(0x64)&1)){} inb(0x60);
    while(inb(0x64)&1) inb(0x60);
}

// VFS
#define VFS_MAX 128
typedef struct { char name[32]; int is_dir; char parent[64]; } VFS_Entry;
static VFS_Entry vfs[VFS_MAX];
static int vfs_count = 0;

void vfs_init() {
    vfs_count=0;
    vfs[0].name[0]='/';vfs[0].name[1]=0;vfs[0].is_dir=1;vfs[0].parent[0]=0;vfs_count++;
    vfs[1].name[0]='h';vfs[1].name[1]='o';vfs[1].name[2]='m';vfs[1].name[3]='e';vfs[1].name[4]=0;vfs[1].is_dir=1;vfs[1].parent[0]='/';vfs[1].parent[1]=0;vfs_count++;
    vfs[2].name[0]='e';vfs[2].name[1]='t';vfs[2].name[2]='c';vfs[2].name[3]=0;vfs[2].is_dir=1;vfs[2].parent[0]='/';vfs[2].parent[1]=0;vfs_count++;
}

void con_print(const char* t);
void vfs_ls(const char* path) {
    for(int i=0;i<vfs_count;i++) {
        int match=1,j=0;
        while(vfs[i].parent[j] && path[j]) { if(vfs[i].parent[j]!=path[j]){match=0;break;} j++; }
        if(vfs[i].parent[j]!=0 || path[j]!=0) match=0;
        if(match) {
            char b[40]; int k=0;
            while(vfs[i].name[k]) { b[k]=vfs[i].name[k]; k++; }
            if(vfs[i].is_dir) { b[k]='/'; b[k+1]=0; } else { b[k]=0; }
            con_print(b);
        }
    }
}

int vfs_cd(char* path, const char* arg) {
    if(arg[0]==0 || (arg[0]=='/'&&arg[1]==0)) { path[0]='/'; path[1]=0; return 0; }
    for(int i=0;i<vfs_count;i++) {
        if(vfs[i].is_dir) {
            int match=1,j=0;
            while(vfs[i].name[j] && arg[j]) { if(vfs[i].name[j]!=arg[j]){match=0;break;} j++; }
            if(match && vfs[i].name[j]==0 && arg[j]==0) {
                int k=0; while(path[k]) k++;
                if(path[k-1]!='/') path[k++]='/';
                j=0; while(arg[j] && k<63) { path[k++]=arg[j++]; }
                path[k]=0; return 0;
            }
        }
    }
    return -1;
}

int vfs_mkdir(const char* path, const char* name) {
    if(vfs_count>=VFS_MAX) return -1;
    int i=0; while(name[i] && i<31) { vfs[vfs_count].name[i]=name[i]; i++; }
    vfs[vfs_count].name[i]=0;
    vfs[vfs_count].is_dir=1;
    int j=0; while(path[j] && j<63) { vfs[vfs_count].parent[j]=path[j]; j++; }
    vfs[vfs_count].parent[j]=0;
    vfs_count++;
    return 0;
}

// Консоль
#define MAX_LINES 500
#define INPUT_MAX 256
static char console_buf[MAX_LINES][256];
static int console_lines = 0;
static char input_buf[INPUT_MAX];
static int input_pos = 0;
static char cwd[64] = "/";
static int shift_held = 0;

void con_print(const char* t) {
    int i=0;
    while(t[i] && i<255 && console_lines<MAX_LINES) { console_buf[console_lines][i]=t[i]; i++; }
    if(console_lines<MAX_LINES) { console_buf[console_lines][i]=0; console_lines++; }
}

void con_draw(int x, int y, int w, int h) {
    int lh=18, mv=h/lh;
    if(mv<2) mv=2;
    int start=0;
    if(console_lines>mv-2) start=console_lines-(mv-2);
    int drawn=0;
    for(int i=start; i<console_lines && drawn<mv-1; i++) {
        draw_text(x+5, y+drawn*lh, console_buf[i], 0xFFD0D0D0);
        drawn++;
    }
    int py=y+drawn*lh;
    draw_text(x+5, py, "user@fireos:", 0xFFFFFFFF);
    draw_text(x+5+11*8, py, cwd, 0xFFFFFFFF);
    draw_text(x+5+(11+my_strlen(cwd))*8, py, "$ ", 0xFFFFFFFF);
    int pl=11+my_strlen(cwd)+2;
    draw_text(x+5+pl*8, py, input_buf, 0xFFFFFF00);
    rect(x+5+(pl+input_pos)*8, py, 8, 16, 0xFFFFFFFF);
}

// ============ ДИСК ============
u32 disk_data_start=0, disk_fat_start=0, disk_root_cluster=0;
u8 disk_spc=0; int disk_ready=0;

void disk_read_sector(u32 lba, u8* buf) {
    outb(0x1F6,0xE0|((lba>>24)&0x0F)); outb(0x1F2,1);
    outb(0x1F3,lba&0xFF); outb(0x1F4,(lba>>8)&0xFF);
    outb(0x1F5,(lba>>16)&0xFF); outb(0x1F7,0x20);
    while(inb(0x1F7)&0x80){}
    for(int i=0;i<256;i++) ((u16*)buf)[i]=inw(0x1F0);
}

void disk_init() {
    u8 sec[512];
    outb(0x1F6,0xE0); outb(0x1F2,1);
    outb(0x1F3,0); outb(0x1F4,0); outb(0x1F5,0);
    outb(0x1F7,0x20);
    while(inb(0x1F7)&0x80){}
    if(inb(0x1F7)&1) return;
    for(int i=0;i<256;i++) ((u16*)sec)[i]=inw(0x1F0);
    if(sec[0x52]=='F'&&sec[0x53]=='A'&&sec[0x54]=='T') {
        disk_spc=sec[13];
        u16 res=*(u16*)(sec+14); u8 fats=sec[16];
        u32 fat_sz=*(u32*)(sec+36);
        disk_root_cluster=*(u32*)(sec+44);
        disk_fat_start=res;
        disk_data_start=res+fats*fat_sz;
        disk_ready=1;
    }
}

// ============ ЗАПУСК ПРИЛОЖЕНИЙ С ДИСКА ============
void run_disk_app(const char* name) {
    if(!disk_ready) {
        con_print("[ERROR] No disk");
        return;
    }
    
    static u8 app_buf[16384];
    u8 sec[512];
    u32 cl = disk_root_cluster;
    
    while(cl >= 2 && cl < 0x0FFFFFF8) {
        for(int s=0; s<disk_spc; s++) {
            disk_read_sector(disk_data_start + (cl-2)*disk_spc + s, sec);
            for(int e=0; e<16; e++) {
                u8* ent = sec + e*32;
                if(ent[0]==0) return;
                if(ent[0]==0xE5) continue;
                
                char fname[12]={0};
                int ni=0;
                for(int n=0; n<8 && ent[n]!=' '; n++) fname[ni++]=ent[n];
                if(ent[8]!=' ') {
                    fname[ni++]='.';
                    for(int n=0; n<3 && ent[8+n]!=' '; n++) fname[ni++]=ent[8+n];
                }
                fname[ni]=0;
                
                int match=1, j=0;
                while(fname[j] && name[j]) {
                    if(fname[j]!=name[j]) { match=0; break; }
                    j++;
                }
                
                if(match && fname[j]==0 && name[j]==0) {
                    u32 fc = ((u32)*(u16*)(ent+20)<<16) | *(u16*)(ent+26);
                    disk_read_sector(disk_data_start + (fc-2)*disk_spc, app_buf);
                    
                    con_print("[RUN] Executing app...");
                    
                    if(app_buf[0]==0x7F && app_buf[1]=='E' && app_buf[2]=='L') {
                        u32 entry = *(u32*)(app_buf + 24);
                        void (*entry_func)() = (void(*)())(app_buf + entry);
                        entry_func();
                        con_print("[DONE] App finished");
                    } else {
                        con_print("[ERROR] Not ELF");
                    }
                    return;
                }
            }
        }
        u8 fs[512];
        u32 off = cl*4;
        disk_read_sector(disk_fat_start + off/512, fs);
        cl = *(u32*)(fs + (off%512)) & 0x0FFFFFFF;
    }
    con_print("[ERROR] App not found");
}

// Команды
void cmd_exec(const char* cmd) {
    char comm[64]={0}, arg[128]={0};
    int ci=0;
    while(cmd[ci]==' ') ci++;
    int cs=ci;
    while(cmd[ci] && cmd[ci]!=' ' && (ci-cs)<63) { comm[ci-cs]=cmd[ci]; ci++; }
    comm[ci-cs]=0;
    while(cmd[ci]==' ') ci++;
    int ai=0;
    while(cmd[ci] && ai<127) arg[ai++]=cmd[ci++];
    arg[ai]=0;
    
    if(comm[0]==0) return;
    
    if(comm[0]=='l'&&comm[1]=='s') vfs_ls(cwd);
    else if(comm[0]=='c'&&comm[1]=='d') { if(vfs_cd(cwd,arg)!=0) con_print("No such directory"); }
    else if(comm[0]=='p'&&comm[1]=='w'&&comm[2]=='d') con_print(cwd);
    else if(comm[0]=='m'&&comm[1]=='k'&&comm[2]=='d') { if(arg[0]){ if(vfs_mkdir(cwd,arg)==0) con_print("Created"); else con_print("Error"); } else con_print("Usage: mkdir <name>"); }
    else if(comm[0]=='c'&&comm[1]=='l'&&comm[2]=='e') console_lines=0;
    else if(comm[0]=='h'&&comm[1]=='e'&&comm[2]=='l') con_print("ls cd pwd mkdir clear help echo run");
    else if(comm[0]=='e'&&comm[1]=='c'&&comm[2]=='h') { if(arg[0]) con_print(arg); }
    else if(comm[0]=='r'&&comm[1]=='u'&&comm[2]=='n') { if(arg[0]) run_disk_app(arg); else con_print("Usage: run <app.exe>"); }
    else con_print("Command not found");
}

char kbd_read() {
    if(inb(0x64)&1) {
        u8 sc=inb(0x60);
        if(sc==0x2A||sc==0x36){shift_held=1;return 0;}
        if(sc==0xAA||sc==0xB6){shift_held=0;return 0;}
        if(sc==0xE0||sc==0xF0||(sc&0x80))return 0;
        static const char lo[]={0,0,'1','2','3','4','5','6','7','8','9','0','-','=','\b','\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',0,'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\','z','x','c','v','b','n','m',',','.','/',0,'*',0,' '};
        static const char up[]={0,0,'!','@','#','$','%','^','&','*','(',')','_','+','\b','\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',0,'A','S','D','F','G','H','J','K','L',':','"','~',0,'|','Z','X','C','V','B','N','M','<','>','?',0,'*',0,' '};
        if(sc<128){char ch=shift_held?up[sc]:lo[sc];if(ch)return ch;}
    }
    return 0;
}

void redraw_console() {
    if(fiogui_count>0 && fiogui_windows[0].visible) {
        rect(fiogui_windows[0].x+1, fiogui_windows[0].y+29,
             fiogui_windows[0].width-2, fiogui_windows[0].height-30, 0xFF002B36);
        con_draw(fiogui_windows[0].x+8, fiogui_windows[0].y+32,
                 fiogui_windows[0].width-16, fiogui_windows[0].height-40);
    }
}

void redraw_all() {
    rect(0, 0, sw, sh, 0xFF0044AA);
    fiogui_draw_all();
    redraw_console();
}

void key_handle(char key) {
    if(key=='\n'){ cmd_exec(input_buf); input_pos=0; input_buf[0]=0; }
    else if(key=='\b'){ if(input_pos>0){input_pos--;input_buf[input_pos]=0;} }
    else if(key>=32&&key<=126&&input_pos<INPUT_MAX-1){ input_buf[input_pos]=key; input_pos++; input_buf[input_pos]=0; }
    redraw_console();
}

void kernel_main() {
    rect(0,0,sw,sh,0xFF0044AA);
    
    vfs_init();
    disk_init();
    mouse_init();
    
    fiogui_create_window(200, 100, 700, 500, "Terminal");
    fiogui_draw_all();
    redraw_console();
    
    if(disk_ready) {
        con_print("[OK] Disk ready");
        con_print("Apps: hello.exe gui.exe");
        con_print("Use: run hello.exe");
    }
    
    save_cursor_bg();
    draw_cursor();
    omx=mx; omy=my;
    
    for(;;) {
        u8 st=inb(0x64);
        if((st&1)&&(st&0x20)) {
            static u8 cyc=0,pkt[3];
            pkt[cyc]=inb(0x60); cyc++;
            if(cyc==3) {
                int dx=pkt[1],dy=pkt[2];
                if(pkt[0]&0x10)dx|=~0xFF;
                if(pkt[0]&0x20)dy|=~0xFF;
                int btn=pkt[0]&0x07;
                cyc=0;
                
                if(btn==1 && fiogui_drag_win<0) {
                    int id=fiogui_find_window(mx,my);
                    if(id>=0 && fiogui_hit_title(id,mx,my)) {
                        fiogui_start_drag(id,mx,my);
                    }
                }
                if(btn==0 && fiogui_drag_win>=0) fiogui_end_drag();
                
                if(fiogui_drag_win>=0) {
                    fiogui_update_drag(mx,my);
                    restore_cursor_bg();
                    redraw_all();
                    save_cursor_bg();
                    draw_cursor();
                    continue;
                }
                
                restore_cursor_bg();
                mx+=dx; my-=dy;
                if(mx<0)mx=0; if(mx>sw-CURSOR_W)mx=sw-CURSOR_W;
                if(my<0)my=0; if(my>sh-CURSOR_H)my=sh-CURSOR_H;
                save_cursor_bg();
                draw_cursor();
                omx=mx; omy=my;
            }
        }
        else if((st&1)&&!(st&0x20)) {
            char key=kbd_read();
            if(key)key_handle(key);
        }
    }
}
