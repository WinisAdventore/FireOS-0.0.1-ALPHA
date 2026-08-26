#ifndef RTC_FIRE_H
#define RTC_FIRE_H
typedef unsigned char u8;
static int bcd2bin(u8 b) { return ((b>>4)*10)+(b&0x0F); }
static u8 bin2bcd(int v) { return ((v/10)<<4)|(v%10); }
static u8 cmos_read(u8 reg) { extern void outb(u16 p, u8 v); extern u8 inb(u16 p); outb(0x70, reg); return inb(0x71); }
static void cmos_write(u8 reg, u8 val) { extern void outb(u16 p, u8 v); outb(0x70, reg); outb(0x71, val); }
void rtc_get(int* h, int* m, int* s) { while(cmos_read(0x0A)&0x80){} *s=bcd2bin(cmos_read(0x00)); *m=bcd2bin(cmos_read(0x02)); *h=bcd2bin(cmos_read(0x04)); }
void rtc_get_date(int* y, int* mo, int* d) { while(cmos_read(0x0A)&0x80){} *d=bcd2bin(cmos_read(0x07)); *mo=bcd2bin(cmos_read(0x08)); *y=bcd2bin(cmos_read(0x09))+2000; }
void rtc_set_time(int h, int m, int s) { cmos_write(0x00,bin2bcd(s)); cmos_write(0x02,bin2bcd(m)); cmos_write(0x04,bin2bcd(h)); }
void rtc_set_date(int y, int mo, int d) { cmos_write(0x07,bin2bcd(d)); cmos_write(0x08,bin2bcd(mo)); cmos_write(0x09,bin2bcd(y-2000)); }
void rtc_time_str(char* buf) { int h,m,s; rtc_get(&h,&m,&s); buf[0]='0'+h/10;buf[1]='0'+h%10;buf[2]=':';buf[3]='0'+m/10;buf[4]='0'+m%10;buf[5]=':';buf[6]='0'+s/10;buf[7]='0'+s%10;buf[8]=0; }
void rtc_date_str(char* buf) { int y,mo,d; rtc_get_date(&y,&mo,&d); buf[0]='0'+y/1000;buf[1]='0'+(y/100)%10;buf[2]='0'+(y/10)%10;buf[3]='0'+y%10;buf[4]='-';buf[5]='0'+mo/10;buf[6]='0'+mo%10;buf[7]='-';buf[8]='0'+d/10;buf[9]='0'+d%10;buf[10]=0; }
#endif
