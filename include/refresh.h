#ifndef REFRESH_H
#define REFRESH_H

struct svga_cell {
    int screen_off;
    unsigned short bank_off;
    unsigned short split_off;
};

extern unsigned char svga_refresh_table[1364];
extern unsigned char gen_refresh1;
extern unsigned char gen_refresh2;
extern unsigned char gen_refresh3;
extern char gen_refresh4;

extern struct svga_cell svga_refresh_data[1361];
extern int ref_y;
extern int ref_x;
extern int ref_ptr;
extern int refresh_count;

#endif /* REFRESH_H */
