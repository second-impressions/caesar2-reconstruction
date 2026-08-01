#ifndef C2_WINGDI_H
#define C2_WINGDI_H

struct win_palette_entry {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char flags;
};

struct win_rgb_quad {
    unsigned char blue;
    unsigned char green;
    unsigned char red;
    unsigned char reserved;
};

struct win_log_palette {
    unsigned short version;
    unsigned short entry_count;
    struct win_palette_entry entries[256];
};

#if PLATFORM_WINDOWS
void direct_palette_set(unsigned char *palette);
#endif

#endif
