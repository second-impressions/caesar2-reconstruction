
/* The refresh table stores redraw priorities for a 40×30 grid of 16-pixel tiles. */
#include "refresh.h"
#include "c2_data.h"
#if PLATFORM_WINDOWS
#include <windows.h>
#endif

struct refresh_bank_row refresh_bank_switch_data[30] = {
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 1, 1, 6, 16 },
    { 0, 1, 0, 0 },
    { 0, 1, 0, 0 },
    { 0, 1, 0, 0 },
    { 0, 1, 0, 0 },
    { 0, 1, 0, 0 },
    { 1, 2, 12, 32 },
    { 0, 2, 0, 0 },
    { 0, 2, 0, 0 },
    { 0, 2, 0, 0 },
    { 0, 2, 0, 0 },
    { 0, 2, 0, 0 },
    { 0, 2, 0, 0 },
    { 1, 3, 3, 8 },
    { 0, 3, 0, 0 },
    { 0, 3, 0, 0 },
    { 0, 3, 0, 0 },
    { 0, 3, 0, 0 },
    { 0, 3, 0, 0 },
    { 1, 4, 9, 24 },
    { 0, 4, 0, 0 },
    { 0, 4, 0, 0 },
    { 0, 4, 0, 0 },
    { 0, 4, 0, 0 }
};

/* Refresh-grid state and precomputed tile offsets. */
struct svga_cell svga_refresh_data[1361];
int ref_y;
int ref_x;
int ref_ptr;
int refresh_count;
unsigned char svga_refresh_table[1364];

/* Copies part of a 16×16 tile into the active SVGA bank. */
extern void refresh_16x16_partblock(int screen_off, unsigned short bank_off,
                                    int width);
#if PLATFORM_WINDOWS
extern int pm_diamond_full_height;
extern void resize_pm_screen(void);
extern void update_map_scrollbars(unsigned int mode);
extern void *active_window;
extern unsigned char screen_buffer[];
extern void win_bitblt();
#endif

/* Forward declarations (functions defined later in this file). */
void setup_refresh_area(int screen_x, int screen_y, int width, int height, int refresh_value);

// Mark every clean screen tile for one redraw.
// FUNCTION: C2 0x28e94
// FUNCTION: C2WIN 0x0043a640
void setup_whole_screen_refresh(void)
{
    int tile_idx;
    for (tile_idx = 0; tile_idx < 0x4b0; tile_idx++) {
        if (svga_refresh_table[tile_idx] == 0)
            svga_refresh_table[tile_idx] = 1;
    }
}

// Mark the refresh tiles covered by the mouse pointer at priority 2.
// FUNCTION: C2 0x28eb2
// FUNCTION: C2WIN 0x0043a68c
void set_mouse_refresh(void)
{
    if (pointer_mode == 6 || pointer_mode == 7) {
        setup_refresh_area(mouse_x, mouse_y, 3, 3, 2);
        return;
    }

    ref_x = mouse_x / 16;
    ref_y = mouse_y / 16;

    if (ref_x < 0) return;
    if (ref_y < 0) return;
    if (ref_x >= 40) return;
    if (ref_y >= 30) return;

    (*(unsigned char (*)[30][40])svga_refresh_table)[ref_y][ref_x] = 2;
    if (ref_x < 39)
        (*(unsigned char (*)[30][40])svga_refresh_table)[ref_y + 0][ref_x + 1] = 2;
    if (ref_y < 29)
        (*(unsigned char (*)[30][40])svga_refresh_table)[ref_y + 1][ref_x] = 2;
    if (ref_x < 39 && ref_y < 29)
        (&svga_refresh_table[ref_x])[(ref_y + 1) * 40 + 1] = 2;
}


// Mark a 2×2 cell square in the SVGA refresh table dirty.
// FUNCTION: C2 0x28fb5
// FUNCTION: C2WIN 0x0043a7d2
void refresh_sprite_square(int tile_x, int tile_y)
{
    if (tile_y < 0) tile_y = 0;
    if (tile_x < 0) tile_x = 0;
    ref_ptr = tile_x + tile_y * 40;
    if (ref_ptr >= 0x4b0) return;
    if (svga_refresh_table[ref_ptr] < 2)
        svga_refresh_table[ref_ptr] = 2;
    if (svga_refresh_table[ref_ptr + 1] < 2)
        svga_refresh_table[ref_ptr + 1] = 2;
    if (svga_refresh_table[ref_ptr + 0x28] < 2)
        svga_refresh_table[ref_ptr + 0x28] = 2;
    if (svga_refresh_table[ref_ptr + 0x29] < 2)
        svga_refresh_table[ref_ptr + 0x29] = 2;
}

// Mark a 4×4 tile square at priority 2, replacing any existing priority.
// FUNCTION: C2 0x29041
// FUNCTION: C2WIN 0x0043a8ad
void refresh_figure_square(int tile_x, int tile_y)
{
    if (tile_y < 0) tile_y = 0;
    if (tile_x < 0) tile_x = 0;
    ref_ptr = tile_x + tile_y * 40;
    if (ref_ptr >= 0x4b0) return;
    svga_refresh_table[ref_ptr + 0x00] = 2;
    svga_refresh_table[ref_ptr + 0x01] = 2;
    svga_refresh_table[ref_ptr + 0x02] = 2;
    svga_refresh_table[ref_ptr + 0x03] = 2;
    svga_refresh_table[ref_ptr + 0x28] = 2;
    svga_refresh_table[ref_ptr + 0x29] = 2;
    svga_refresh_table[ref_ptr + 0x2a] = 2;
    svga_refresh_table[ref_ptr + 0x2b] = 2;
    svga_refresh_table[ref_ptr + 0x50] = 2;
    svga_refresh_table[ref_ptr + 0x51] = 2;
    svga_refresh_table[ref_ptr + 0x52] = 2;
    svga_refresh_table[ref_ptr + 0x53] = 2;
    svga_refresh_table[ref_ptr + 0x78] = 2;
    svga_refresh_table[ref_ptr + 0x79] = 2;
    svga_refresh_table[ref_ptr + 0x7a] = 2;
    svga_refresh_table[ref_ptr + 0x7b] = 2;
}

// Mark a 5×5 tile square at priority 2.
// FUNCTION: C2 0x290ce
// FUNCTION: C2WIN 0x0043a9c0
void refresh_figure2_square(int tile_x, int tile_y)
{
    int row_idx;

    if (tile_y < 0) tile_y = 0;
    if (tile_x < 0) tile_x = 0;
    ref_ptr = tile_x + tile_y * 40;
    for (row_idx = 0; row_idx < 5; row_idx++) {
        if (ref_ptr >= 0x4b0) return;
        svga_refresh_table[ref_ptr + 0] = 2;
        svga_refresh_table[ref_ptr + 1] = 2;
        svga_refresh_table[ref_ptr + 2] = 2;
        svga_refresh_table[ref_ptr + 3] = 2;
        svga_refresh_table[ref_ptr + 4] = 2;
        ref_ptr += 40;
    }
}

// Mark a 6×6 tile square at priority 2.
// FUNCTION: C2 0x29131
// FUNCTION: C2WIN 0x0043aa77
void refresh_figure3_square(int tile_x, int tile_y)
{
    int row_idx;

    if (tile_y < 0) tile_y = 0;
    if (tile_x < 0) tile_x = 0;
    ref_ptr = tile_x + tile_y * 40;
    for (row_idx = 0; row_idx < 6; row_idx++) {
        if (ref_ptr >= 0x4b0) return;
        svga_refresh_table[ref_ptr + 0] = 2;
        svga_refresh_table[ref_ptr + 1] = 2;
        svga_refresh_table[ref_ptr + 2] = 2;
        svga_refresh_table[ref_ptr + 3] = 2;
        svga_refresh_table[ref_ptr + 4] = 2;
        svga_refresh_table[ref_ptr + 5] = 2;
        ref_ptr += 40;
    }
}

// Raise a 4×2 tile rectangle to at least priority 2.
// FUNCTION: C2 0x2919a
// FUNCTION: C2WIN 0x0043ab3a
void refresh_sprite2w_square(int tile_x, int tile_y)
{
    if (tile_y < 0) tile_y = 0;
    if (tile_x < 0) tile_x = 0;
    ref_ptr = tile_x + tile_y * 40;
    if (ref_ptr >= 0x4b0) return;
    if (svga_refresh_table[ref_ptr + 0x00] < 2)
        svga_refresh_table[ref_ptr + 0x00] = 2;
    if (svga_refresh_table[ref_ptr + 0x01] < 2)
        svga_refresh_table[ref_ptr + 0x01] = 2;
    if (svga_refresh_table[ref_ptr + 0x02] < 2)
        svga_refresh_table[ref_ptr + 0x02] = 2;
    if (svga_refresh_table[ref_ptr + 0x03] < 2)
        svga_refresh_table[ref_ptr + 0x03] = 2;
    if (svga_refresh_table[ref_ptr + 0x28] < 2)
        svga_refresh_table[ref_ptr + 0x28] = 2;
    if (svga_refresh_table[ref_ptr + 0x29] < 2)
        svga_refresh_table[ref_ptr + 0x29] = 2;
    if (svga_refresh_table[ref_ptr + 0x2a] < 2)
        svga_refresh_table[ref_ptr + 0x2a] = 2;
    if (svga_refresh_table[ref_ptr + 0x2b] < 2)
        svga_refresh_table[ref_ptr + 0x2b] = 2;
}

// Raise a 5×6 tile rectangle to at least priority 2.
// FUNCTION: C2 0x2928e
// FUNCTION: C2WIN 0x0043ac9d
void refresh_region_sprite_square(int tile_x, int tile_y)
{
    int row_idx;

    if (tile_y < 0) tile_y = 0;
    if (tile_x < 0) tile_x = 0;
    ref_ptr = tile_x + tile_y * 40;
    if (ref_ptr >= 0x4b0) return;
    for (row_idx = 0; row_idx < 6; row_idx++) {
        if (svga_refresh_table[ref_ptr + 0] < 2)
            svga_refresh_table[ref_ptr + 0] = 2;
        if (svga_refresh_table[ref_ptr + 1] < 2)
            svga_refresh_table[ref_ptr + 1] = 2;
        if (svga_refresh_table[ref_ptr + 2] < 2)
            svga_refresh_table[ref_ptr + 2] = 2;
        if (svga_refresh_table[ref_ptr + 3] < 2)
            svga_refresh_table[ref_ptr + 3] = 2;
        if (svga_refresh_table[ref_ptr + 4] < 2)
            svga_refresh_table[ref_ptr + 4] = 2;
        ref_ptr += 0x28;
        if (ref_ptr >= 0x4b0) return;
    }
}

// Marks one map square for redraw at the requested refresh priority.
// FUNCTION: C2 0x29361
// FUNCTION: C2WIN 0x0043add7
void refresh_a_square(int tile_x, int tile_y, char refresh_value)
{
    ref_ptr = tile_x + tile_y * 0x28;
    svga_refresh_table[ref_ptr + 0x00] = refresh_value;
    svga_refresh_table[ref_ptr + 0x01] = refresh_value;
    svga_refresh_table[ref_ptr + 0x02] = refresh_value;
    svga_refresh_table[ref_ptr + 0x03] = refresh_value;
    svga_refresh_table[ref_ptr + 0x04] = refresh_value;
    svga_refresh_table[ref_ptr + 0x28] = refresh_value;
    svga_refresh_table[ref_ptr + 0x29] = refresh_value;
    svga_refresh_table[ref_ptr + 0x2a] = refresh_value;
    svga_refresh_table[ref_ptr + 0x2b] = refresh_value;
    svga_refresh_table[ref_ptr + 0x2c] = refresh_value;
    svga_refresh_table[ref_ptr + 0x50] = refresh_value;
    svga_refresh_table[ref_ptr + 0x51] = refresh_value;
    svga_refresh_table[ref_ptr + 0x52] = refresh_value;
    svga_refresh_table[ref_ptr + 0x53] = refresh_value;
    svga_refresh_table[ref_ptr + 0x54] = refresh_value;
}

// Mark a five-tile-wide footprint at priority 2, clipping rows above the screen.
// FUNCTION: C2 0x293d2
// FUNCTION: C2WIN 0x0043aed4
void refresh_a_bigger_square(int tile_x, int tile_y)
{
    int i;
    int count;

    count = 6;
    if (tile_y < -0x20) {
        tile_y = 0;
        count = 3;
    } else if (tile_y < -0x10) {
        tile_y = 0;
        count = 4;
    } else if (tile_y < 0) {
        tile_y = 0;
        count = 5;
    }
    ref_ptr = tile_x + tile_y * 40;
    for (i = 0; i < count; ++i) {
        if (ref_ptr >= 0x4b0) return;
        svga_refresh_table[ref_ptr + 0] = 2;
        svga_refresh_table[ref_ptr + 1] = 2;
        svga_refresh_table[ref_ptr + 2] = 2;
        svga_refresh_table[ref_ptr + 3] = 2;
        svga_refresh_table[ref_ptr + 4] = 2;
        ref_ptr += 0x28;
    }
}


// Raise a 14×12 tile rectangle to priority 2, clipping at the top and left edges.
// FUNCTION: C2 0x29458
// FUNCTION: C2WIN 0x0043afc4
void refresh_big_action_square(int tile_x, int tile_y)
{
    int i;
    int j;
    int ymax;
    int xmax;

    ymax = 12;
    xmax = 14;
    if (tile_x < 0) {
        tile_x = 0;
        xmax = 8;
    }
    if (tile_y < 0) {
        tile_y = 0;
        ymax = 8;
    }
    ref_ptr = tile_x + tile_y * 40;
    for (i = 0; i < ymax; i++, ref_ptr += 40) {
        for (j = 0; j < xmax; j++) {
            if (ref_ptr + j >= 0x4b0) return;
            if (svga_refresh_table[ref_ptr + j] < 2) {
                svga_refresh_table[ref_ptr + j] = 2;
            }
        }
    }
}

// Mark clean tiles in the 30×29 map viewport for one redraw.
// FUNCTION: C2 0x294d0
// FUNCTION: C2WIN 0x0043b0aa
void setup_map_screen_refresh(void)
{
    int row_idx = 1;
    int tile_idx = 0x28;
    int col_idx;
    for (; row_idx < 0x1e; row_idx++, tile_idx += 0xa) {
        for (col_idx = 0; col_idx < 0x1e; col_idx++, tile_idx++) {
            if (svga_refresh_table[tile_idx] == 0)
                svga_refresh_table[tile_idx] = 1;
        }
    }
}

// Fill the 30×29 map viewport with the requested refresh priority.
// FUNCTION: C2 0x29506
// FUNCTION: C2WIN 0x0043b11f
void setup_map_screen_long_refresh(int refresh_value)
{
    int row_idx = 1;
    int tile_idx = 0x28;
    int col_idx;
    for (; row_idx < 0x1e; row_idx++, tile_idx += 0xa) {
        for (col_idx = 0; col_idx < 0x1e; col_idx++) {
            svga_refresh_table[tile_idx] = refresh_value;
            tile_idx++;
        }
    }
}

// Mark refresh-grid rows 1 through 22 for one battle-screen redraw.
// FUNCTION: C2 0x29533
// FUNCTION: C2WIN 0x0043b183
void setup_battle_screen_refresh(void)
{
    int i;
    int row_idx;
    int tile_idx = 0x28;
    for (row_idx = 1; row_idx < 0x17; row_idx++) {
        for (i = 0; i < 0x28; i++) {
            svga_refresh_table[tile_idx] = 1;
            tile_idx++;
        }
    }
}

// Fill a tile rectangle whose origin is given in screen pixels.
// FUNCTION: C2 0x2955b
// FUNCTION: C2WIN 0x0043b1e1
void setup_refresh_area(int screen_x, int screen_y, int width, int height, int refresh_value)
{
    if (screen_x < 0) screen_x = 0;
    if (screen_y < 0) screen_y = 0;
    screen_x /= 16;
    screen_y /= 16;

    ref_ptr = screen_x + screen_y * 40;

    for (ref_y = screen_y; ref_y < screen_y + height; ref_y++) {
        if (ref_ptr >= 0x4b0) break;
        for (ref_x = screen_x; ref_x < screen_x + width; ref_x++) {
            svga_refresh_table[ref_ptr] = refresh_value;
            ref_ptr++;
        }
        ref_ptr += 40 - width;
    }
}

// Precompute screen offsets and bank-switch metadata for all 40×30 refresh tiles.
// FUNCTION: C2 0x2960d
// FUNCTION: C2WIN 0x0043b2c9
void setup_svga_refresh_data(void)
{
    int screen_y;
    int screen_x;
    int tile_idx;

    for (tile_idx = 0; tile_idx < 0x4b0; tile_idx++)
        svga_refresh_table[tile_idx] = 0;

    screen_y = 0;
    tile_idx = 0;
    for ( ; screen_y < 0x1e0; screen_y += 0x10) {
    for (screen_x = 0; screen_x < 0x280; screen_x += 0x10, tile_idx++) {
        svga_refresh_data[tile_idx].screen_off = screen_y * 5 * 128 + screen_x;
        svga_refresh_data[tile_idx].bank_off   = (unsigned short)(screen_y * 0x280 + screen_x) % 0x10000;
        svga_refresh_data[tile_idx].split_off  = 0;

        if (refresh_bank_switch_data[screen_y / 16].split != 0) {
            int split_col = refresh_bank_switch_data[screen_y / 16].split_col;
            if (screen_x >= split_col * 16) {
                svga_refresh_data[tile_idx].split_off = (unsigned short)(screen_x - split_col * 16);
            } else {
                svga_refresh_data[tile_idx].split_off = (unsigned short)((40 - split_col) * 16 + screen_x);
            }
        }
    }
    }
}

// Reconfigure pseudo-map (PM) viewport globals for the city screen at the given zoom level.
// FUNCTION: C2 0x296e5
// FUNCTION: C2WIN 0x0043b412
void refresh_zoom_mode(int zoom)
{
#if PLATFORM_WINDOWS
    if (map_mode == 0) {
        city_zoom_level = zoom;
    } else if (map_mode == 1) {
        prov_zoom_level = zoom;
    }
    zoom_level = zoom;
    if (zoom_level == 0) {
        scroll_amount = 1;
        pm_screen_x_start = 0;
        pm_screen_y_start = -15;
        pm_diamond_width = 60;
        pm_diamond_full_height = 32;
        pm_diamond_half_width = 30;
        pm_diamond_half_height = 15;
        resize_pm_screen();
    } else if (zoom_level == 1) {
        scroll_amount = 2;
        pm_screen_x_start = -2;
        pm_screen_y_start = -7;
        pm_diamond_width = 28;
        pm_diamond_full_height = 14;
        pm_diamond_half_width = 14;
        pm_diamond_half_height = 7;
        resize_pm_screen();
    } else if (zoom_level == 2) {
        scroll_amount = 4;
        pm_screen_x_start = 0;
        pm_screen_y_start = -3;
        pm_diamond_width = 12;
        pm_diamond_full_height = 6;
        pm_diamond_half_width = 6;
        pm_diamond_half_height = 3;
        resize_pm_screen();
    }
    pm_screen_x_end = pm_diamond_width * pm_screen_width + pm_screen_x_start;
    pm_screen_y_end = (pm_screen_height + 1) * pm_diamond_half_height
                      + pm_screen_y_start;
    update_map_scrollbars(map_mode);
#else
    zoom_level = zoom;
    if (zoom_level == 0) {
        scroll_amount = 1;
        pm_screen_width = 8;
        pm_screen_height = 0x1e;
        pm_screen_x_start = 0;
        pm_screen_y_start = 9;
        pm_diamond_width = 0x3c;
        pm_diamond_height = 0x1e;
        pm_diamond_half_width = 0x1e;
        pm_diamond_half_height = 0xf;
    } else if ((zoom & 0xff) == 1) {
        scroll_amount = 2;
        pm_screen_width = 0x11;
        pm_screen_height = 0x40;
        pm_screen_x_start = 4;
        pm_screen_y_start = 0x11;
        pm_diamond_width = 0x1c;
        pm_diamond_height = 0xe;
        pm_diamond_half_width = 0xe;
        pm_diamond_half_height = 7;
    } else if ((zoom & 0xff) == 2) {
        scroll_amount = 4;
        pm_screen_width = 0x28;
        pm_screen_height = 0x96;
        pm_screen_x_start = 0;
        pm_screen_y_start = 0x15;
        pm_diamond_width = 0xc;
        pm_diamond_height = 6;
        pm_diamond_half_width = 6;
        pm_diamond_half_height = 3;
    }
    pm_screen_x_end = pm_screen_x_start + pm_screen_width * pm_diamond_width;
    pm_screen_y_end = pm_screen_y_start
                      + (pm_screen_height + 1) * pm_diamond_half_height;
#endif
}

// Reconfigure pseudo-map (PM) viewport globals for the battle screen at the given zoom level.
// FUNCTION: C2 0x29833
// FUNCTION: C2WIN 0x0043b5b2
void refresh_battle_zoom_mode(int zoom)
{
#if PLATFORM_WINDOWS
    zoom_level = zoom;
    if (zoom_level == 1) {
        scroll_amount = 2;
        pm_screen_width = 22;
        pm_screen_height = 70;
        pm_screen_x_start = 0;
        pm_screen_y_start = -7;
        pm_diamond_width = 28;
        pm_diamond_full_height = 14;
        pm_diamond_half_width = 14;
        pm_diamond_half_height = 7;
    } else if (zoom_level == 2) {
        scroll_amount = 4;
        pm_screen_width = 50;
        pm_screen_height = 120;
        pm_screen_x_start = 0;
        pm_screen_y_start = 58;
        pm_diamond_width = 12;
        pm_diamond_full_height = 6;
        pm_diamond_half_width = 6;
        pm_diamond_half_height = 3;
    }
    pm_screen_x_end = pm_diamond_width * pm_screen_width + pm_screen_x_start;
    pm_screen_y_end = (pm_screen_height + 1) * pm_diamond_half_height
                      + pm_screen_y_start;
#else
    zoom_level = zoom;
    if ((zoom & 0xff) == 1) {
        scroll_amount = 2;
        pm_screen_width = 0x17;
        pm_screen_height = 0x30;
        pm_screen_x_start = 0;
        pm_screen_y_start = 0x11;
        pm_diamond_width = 0x1c;
        pm_diamond_height = 0xe;
        pm_diamond_half_width = 0xe;
        pm_diamond_half_height = 7;
    } else if ((zoom & 0xff) == 2) {
        scroll_amount = 4;
        pm_screen_width = 0x35;
        pm_screen_height = 0x70;
        pm_screen_x_start = 6;
        pm_screen_y_start = 0x15;
        pm_diamond_width = 0xc;
        pm_diamond_height = 6;
        pm_diamond_half_width = 6;
        pm_diamond_half_height = 3;
    }
    pm_screen_x_end = pm_screen_x_start + pm_screen_width * pm_diamond_width;
    pm_screen_y_end = pm_screen_y_start
                      + (pm_screen_height + 1) * pm_diamond_half_height;
#endif
}

// Redraw dirty tiles, splitting copies that cross an SVGA bank boundary.
// FUNCTION: C2 0x2992d
// FUNCTION: C2WIN 0x0043b6cd
void refresh_svga_screen(void)
{
#if PLATFORM_WINDOWS
    RECT rect;
    int w;
    int h;

    GetClientRect(active_window, &rect);
    w = rect.right - rect.left;
    h = rect.bottom - rect.top;
    win_bitblt(active_window, screen_buffer, 0, 0, w, h, 0, 0);
#else
    int row_idx;
    int col_idx;
    int tile_idx;
    int part_height;
    int bank_idx;
    int screen_offset;
    int saved_tile_idx;

    tile_idx = 0;
    for (row_idx = 0; row_idx < 30; row_idx++) {
        if (refresh_bank_switch_data[row_idx].split != 0) {
            saved_tile_idx = tile_idx;
            /* Copy each tile's portion held in the preceding bank. */
            for (col_idx = 0; col_idx < 40; col_idx++, tile_idx++) {
                if (svga_refresh_table[tile_idx] != 0) {
                    set_bank(refresh_bank_switch_data[row_idx].bank - 1);
                    part_height = refresh_bank_switch_data[row_idx].part_rows;
                    if (col_idx < refresh_bank_switch_data[row_idx].split_col)
                        part_height++;
                    refresh_16x16_partblock(
                        svga_refresh_data[tile_idx].screen_off,
                        svga_refresh_data[tile_idx].bank_off,
                        part_height);
                }
            }
            /* Copy each tile's remaining portion from the selected bank. */
            tile_idx = saved_tile_idx;
            for (col_idx = 0; col_idx < 40; col_idx++, tile_idx++) {
                if (svga_refresh_table[tile_idx] != 0) {
                    set_bank(refresh_bank_switch_data[row_idx].bank);
                    refresh_count++;
                    svga_refresh_table[tile_idx]--;
                    part_height = refresh_bank_switch_data[row_idx].part_rows;
                    if (col_idx < refresh_bank_switch_data[row_idx].split_col)
                        part_height++;
                    screen_offset = part_height * 5 * 128;
                    refresh_16x16_partblock(
                        svga_refresh_data[tile_idx].screen_off + screen_offset,
                        svga_refresh_data[tile_idx].split_off,
                        16 - part_height);
                }
            }
        } else {
            /* Rows contained in one bank can be copied as full tiles. */
            for (col_idx = 0; col_idx < 40; col_idx++, tile_idx++) {
                if (svga_refresh_table[tile_idx] != 0) {
                    refresh_count++;
                    svga_refresh_table[tile_idx]--;
                    bank_idx = refresh_bank_switch_data[row_idx].bank;
                    set_bank(bank_idx);
                    refresh_16x16_block(
                        svga_refresh_data[tile_idx].screen_off,
                        svga_refresh_data[tile_idx].bank_off);
                }
            }
        }
    }
#endif
}
