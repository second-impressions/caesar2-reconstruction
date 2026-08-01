#include "c2_target.h"
#include "wingdi.h"

#if PLATFORM_WINDOWS

extern struct win_log_palette windows_palette;
extern struct win_rgb_quad dib_palette[256];
extern void *animated_palette;
extern void *logical_palette;
extern void *screen_dc;
extern void *game_dc;
extern void *status_dc;
extern void *map_dc;

extern int (__stdcall *AnimatePalette)(void *palette, unsigned int start,
                                      unsigned int count, void *entries);
extern int (__stdcall *SetPaletteEntries)(void *palette, unsigned int start,
                                         unsigned int count, void *entries);
extern int __stdcall WinGSetDIBColorTable(void *dc, unsigned int start,
                                         unsigned int count, void *entries);

void set_palette_entry_flags(unsigned char animate);

// FUNCTION: C2WIN 0x004ba803
void direct_palette_set(unsigned char *palette)
{
    int rgb_offset;
    int i;

    for (i = 10; i < 246; i++) {
        rgb_offset = i * 3;
        windows_palette.entries[i].red = palette[rgb_offset] * 4;
        windows_palette.entries[i].green = palette[rgb_offset + 1] * 4;
        windows_palette.entries[i].blue = palette[rgb_offset + 2] * 4;
    }
    for (i = 10; i < 246; i++) {
        dib_palette[i].red = windows_palette.entries[i].red;
        dib_palette[i].green = windows_palette.entries[i].green;
        dib_palette[i].blue = windows_palette.entries[i].blue;
    }
    AnimatePalette(animated_palette, 10, 246, &windows_palette.entries[10]);
    set_palette_entry_flags(1);
    SetPaletteEntries(logical_palette, 10, 246, &windows_palette.entries[10]);
    set_palette_entry_flags(0);
    WinGSetDIBColorTable(screen_dc, 10, 246, &dib_palette[10]);
    WinGSetDIBColorTable(game_dc, 10, 246, &dib_palette[10]);
    WinGSetDIBColorTable(status_dc, 10, 246, &dib_palette[10]);
    WinGSetDIBColorTable(map_dc, 10, 246, &dib_palette[10]);
}

#endif
