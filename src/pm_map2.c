
#include "c2_data.h"


extern int  get_string_width(char *src, unsigned char *font);
extern void set_this_ambient(int ambient_idx);
extern void place_diamond(int style);
extern void put_a_font_string(char *str, int x, int y, unsigned char *font, int color);
extern void font_list(int idx, int word_count, int x, int y, unsigned char *font, int color);
extern void font_no(int value, char pad_char, char *suffix, int x, int y, unsigned char *font, int color);
extern void write_i_sprite(unsigned char *sprite_addr);
extern void write_i_left_sprite(unsigned char *sprite_addr);
extern void write_i_right_sprite(unsigned char *sprite_addr);
#if PLATFORM_WINDOWS
extern void *active_window;
extern unsigned char screen_buffer[];
extern void win_bitblt(void *window, unsigned char *buffer, int x, int y,
                       int width, int height, int source_x, int source_y);
#endif
extern void place_lefthalf_diamond(void);
extern void place_righthalf_diamond(void);
/* Forward declarations (functions defined later in this file). */
void show_regionmap_base(void);
void show_regionmap_top(void);
void mid2_line_no_sides_base(void);
void mid2_line_with_sides_base(void);
void mid2_line_no_sides_top(void);
void mid2_line_with_sides_top(void);
void bottom2_line_with_sides(void);
void bottom2_line_no_sides(void);
void place2_a_building_base(int draw_style);
void place2_a_building_top(int draw_style);
void place2_a_building_roof(int draw_style);
void place2_sprite(int draw_style);
void print2_test_info(void);


// Draw the region map's terrain and object layers with a fresh sprite-error count.
// FUNCTION: C2 0x39411
// FUNCTION: C2WIN 0x00445910
void show_regionmap(void)
{
    sprite_error = 0;
    set_this_ambient(1);
    show_regionmap_base();
    show_regionmap_top();
}

// Draw terrain and building bases for the visible region-map cells.
// FUNCTION: C2 0x39430
// FUNCTION: C2WIN 0x00445939
void show_regionmap_base(void)
{
    int ptr;
    int x;
    int i;
    int j;

    sprite_y    = pm_screen_y_start;
    sprite_x    = pm_screen_x_start;
    pm_shown_y  = pm_y;
    pm_y_clip   = 0;
#if PLATFORM_WINDOWS
    if (pm_shown_y >= PM_H) return;
#endif
    i = 0;
    pm_shown_x  = pm_x;

    /* top edge */
    for (; i < pm_screen_width; i++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_image_no = ((pm_shown_ptr) - 0x0FFF0000);
            place_diamond(2);
            sprite_x += pm_diamond_width;
        } else if ((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).base_kind > 0x7c) {
            place2_a_building_base(2);
        } else {
            if (((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits & 1) != 0) {
                (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits &= 0xfe;
#if PLATFORM_DOS
                refresh_a_square(sprite_x >> 4, sprite_y >> 4, 2);
#endif
            }
            sprite_image_no = (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).base_kind;
            sprite_image_no = rotated2_map[sprite_image_no].dir[map_direction >> 1];
            sprite_image_no += 0x10;
            place_diamond(2);
            sprite_x += pm_diamond_width;
        }
        if (!((pm_shown_ptr) >= 0x0FFF0000))
            (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits |= 2;
    }
    sprite_y += pm_diamond_half_height;
    pm_shown_y++;

#if PLATFORM_WINDOWS
    if (pm_shown_y >= PM_H) return;
#endif
    /* interior */
    mid2_line_with_sides_base();
    for (j = 0; j < (pm_screen_height - 2) / 2; j++) {
        mid2_line_no_sides_base();
        mid2_line_with_sides_base();
    }

#if PLATFORM_WINDOWS
    if (pm_shown_y >= PM_H) return;
#endif
    /* bottom edge — same as top with style=1 */
    sprite_x   = pm_screen_x_start;
    i = 0;
    pm_shown_x = pm_x;
    for (; i < pm_screen_width; i++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_image_no = ((pm_shown_ptr) - 0x0FFF0000);
            place_diamond(1);
            sprite_x += pm_diamond_width;
        } else if ((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).base_kind > 0x7c) {
            place2_a_building_base(1);
        } else {
            if (((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits & 1) != 0) {
                (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits &= 0xfe;
#if PLATFORM_DOS
                refresh_a_square(sprite_x >> 4, sprite_y >> 4, 2);
#endif
            }
            sprite_image_no = (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).base_kind;
            sprite_image_no = rotated2_map[sprite_image_no].dir[map_direction >> 1];
            sprite_image_no += 0x10;
            place_diamond(1);
            sprite_x += pm_diamond_width;
        }
        if (!((pm_shown_ptr) >= 0x0FFF0000))
            (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits |= 2;
    }
}

// Draw building tops, armies, and bottom-edge roof slices over the region map.
// FUNCTION: C2 0x396c5
// FUNCTION: C2WIN 0x00445cb5
void show_regionmap_top(void)
{
    int ptr;
    int x;
    int i;
    int j;
#if PLATFORM_DOS
    int y;
#else
    unsigned char redraw;
#endif

#if PLATFORM_DOS
    if (zoom_level == 1) {
        for (y = 0x18; y < 0x1bc; y++)
            show_internal_4point(0, y, 0);
    }
#else
    redraw = 0;
#endif

    sprite_y    = pm_screen_y_start;
    sprite_x    = pm_screen_x_start;
    pm_shown_y  = pm_y;
    pm_y_clip   = 0;
#if PLATFORM_WINDOWS
    if (pm_shown_y >= PM_H) return;
#endif

    /* top edge */
    for (i = 0, pm_shown_x = pm_x; i < pm_screen_width; i++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_x += pm_diamond_width;
        } else {
            if ((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).base_kind > 0x7c) {
                place2_a_building_top(2);
                place2_sprite(0);
            } else {
                sprite_x += pm_diamond_width;
                place2_sprite(0);
            }
        }
    }
    sprite_y += pm_diamond_half_height;
    pm_shown_y++;

    /* interior */
    mid2_line_with_sides_top();
#if PLATFORM_WINDOWS
    if (redraw == 1)
        win_bitblt(active_window, screen_buffer, 0, 0, 0x280, 0x1e0, 0, 0);
#endif
    for (j = 0; j < (pm_screen_height - 2) / 2; j++) {
        mid2_line_no_sides_top();
#if PLATFORM_WINDOWS
        if (redraw == 1)
            win_bitblt(active_window, screen_buffer, 0, 0, 0x280, 0x1e0, 0, 0);
#endif
        mid2_line_with_sides_top();
#if PLATFORM_WINDOWS
        if (redraw == 1)
            win_bitblt(active_window, screen_buffer, 0, 0, 0x280, 0x1e0, 0, 0);
#endif
    }

    /* one more no_sides row above the bottom edge */
    sprite_x   = pm_screen_x_start;
#if PLATFORM_WINDOWS
    if (pm_shown_y >= PM_H) return;
#endif
    for (i = 0, pm_shown_x = pm_x; i < pm_screen_width; i++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_x += pm_diamond_width;
        } else {
            if ((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).base_kind > 0x7c) {
                place2_a_building_top(1);
                place2_sprite(0);
            } else {
                sprite_x += pm_diamond_width;
                place2_sprite(0);
            }
        }
    }
    pm_shown_y++;
    sprite_y += pm_diamond_half_height;
    pm_y_clip = 0;

    bottom2_line_with_sides();
    bottom2_line_no_sides();
    bottom2_line_with_sides();
    bottom2_line_no_sides();
}

// Draw an unclipped row of terrain and building bases, then advance the map scan.
// FUNCTION: C2 0x398a6
// FUNCTION: C2WIN 0x00445fa4
void mid2_line_no_sides_base(void)
{
    int i;
#if PLATFORM_DOS
    int half_height;
    int rotation_idx;
    int base_kind;
#endif

#if PLATFORM_WINDOWS
    if (pm_shown_y >= PM_H) return;
#endif
    sprite_x = pm_screen_x_start;
    for (i = 0, pm_shown_x = pm_x; i < pm_screen_width; i++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_image_no = ((pm_shown_ptr) - 0x0FFF0000);
            place_diamond(0);
            sprite_x += pm_diamond_width;
        } else if ((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).base_kind > 0x7c) {
            place2_a_building_base(0);
        } else {
            if (((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits & 1) != 0) {
                (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits &= 0xfe;
#if PLATFORM_DOS
                refresh_a_square(sprite_x >> 4, sprite_y >> 4, 2);
#endif
            }
#if PLATFORM_DOS
            base_kind = (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).base_kind;
            sprite_image_no = base_kind;
            rotation_idx = map_direction >> 1;
            sprite_image_no = rotated2_map[base_kind].dir[rotation_idx];
#else
            sprite_image_no = (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).base_kind;
            sprite_image_no = rotated2_map[sprite_image_no].dir[map_direction >> 1];
#endif
            sprite_image_no += 0x10;
            place_diamond(0);
            sprite_x += pm_diamond_width;
        }
#if PLATFORM_DOS
        print2_test_info();
#endif
    }
#if PLATFORM_DOS
    half_height = pm_diamond_half_height;
    sprite_y  += half_height;
    pm_shown_y++;
    pm_y_clip += half_height;
#else
    sprite_y  += pm_diamond_half_height;
    pm_shown_y++;
    pm_y_clip += pm_diamond_half_height;
#endif
}

// Draw a base row with half-diamonds at the left and right viewport edges.
// FUNCTION: C2 0x399cc
// FUNCTION: C2WIN 0x0044611a
void mid2_line_with_sides_base(void)
{
    int i;
#if PLATFORM_DOS
    int half_height;
#endif

#if PLATFORM_WINDOWS
    if (pm_shown_y >= PM_H) return;
#endif
    pm_shown_x = pm_x;
    sprite_x   = pm_screen_x_start;

    /* leftmost half-diamond */
    pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
    if (((pm_shown_ptr) >= 0x0FFF0000)) {
        sprite_image_no = ((pm_shown_ptr) - 0x0FFF0000);
#if PLATFORM_WINDOWS
        place_lefthalf_diamond();
#endif
    } else if ((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).base_kind > 0x7c) {
        place2_a_building_base(3);
#if PLATFORM_DOS
        goto left_edge_done;
#endif
    } else {
        if (((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits & 1) != 0) {
            (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits &= 0xfe;
#if PLATFORM_DOS
            refresh_a_square(sprite_x >> 4, sprite_y >> 4, 2);
#endif
        }
        sprite_image_no = (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).base_kind;
        sprite_image_no = rotated2_map[sprite_image_no].dir[map_direction >> 1];
        sprite_image_no += 0x10;
#if PLATFORM_WINDOWS
        place_lefthalf_diamond();
#endif
    }
#if PLATFORM_DOS
    place_lefthalf_diamond();
#endif
left_edge_done:
    if (!((pm_shown_ptr) >= 0x0FFF0000))
        (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits |= 2;
    sprite_x += pm_diamond_half_width;

#if PLATFORM_WINDOWS
    if (pm_shown_y >= PM_H) return;
#endif
    /* middle full diamonds */
    for (i = 0; i < pm_screen_width - 1; i++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_image_no = ((pm_shown_ptr) - 0x0FFF0000);
#if PLATFORM_WINDOWS
            place_diamond(0);
            sprite_x += pm_diamond_width;
#endif
        } else if ((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).base_kind > 0x7c) {
            place2_a_building_base(0);
#if PLATFORM_DOS
            print2_test_info();
            continue;
#endif
        } else {
            if (((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits & 1) != 0) {
                (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits &= 0xfe;
#if PLATFORM_DOS
                refresh_a_square(sprite_x >> 4, sprite_y >> 4, 2);
#endif
            }
            sprite_image_no = (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).base_kind;
            sprite_image_no = rotated2_map[sprite_image_no].dir[map_direction >> 1];
            sprite_image_no += 0x10;
#if PLATFORM_WINDOWS
            place_diamond(0);
            sprite_x += pm_diamond_width;
#endif
        }
#if PLATFORM_DOS
        place_diamond(0);
        sprite_x += pm_diamond_width;
        print2_test_info();
#endif
    }

#if PLATFORM_WINDOWS
    if (pm_shown_y >= PM_H) return;
#endif
    /* rightmost half-diamond */
    pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
    if (((pm_shown_ptr) >= 0x0FFF0000)) {
        sprite_image_no = ((pm_shown_ptr) - 0x0FFF0000);
#if PLATFORM_WINDOWS
        place_righthalf_diamond();
#endif
    } else if ((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).base_kind > 0x7c) {
        place2_a_building_base(4);
#if PLATFORM_DOS
        goto right_edge_done;
#endif
    } else {
        if (((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits & 1) != 0) {
            (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits &= 0xfe;
#if PLATFORM_DOS
            refresh_a_square(sprite_x >> 4, sprite_y >> 4, 2);
#endif
        }
        sprite_image_no = (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).base_kind;
#if PLATFORM_DOS
        sprite_image_no = ((unsigned char *)rotated2_map)[(map_direction >> 1) + sprite_image_no * 4];
#else
        sprite_image_no = rotated2_map[sprite_image_no].dir[map_direction >> 1];
#endif
        sprite_image_no += 0x10;
#if PLATFORM_WINDOWS
        place_righthalf_diamond();
#endif
    }
#if PLATFORM_DOS
    place_righthalf_diamond();
#endif
right_edge_done:
    if (!((pm_shown_ptr) >= 0x0FFF0000))
        (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits |= 2;

#if PLATFORM_DOS
    half_height = pm_diamond_half_height;
    sprite_y  += half_height;
    pm_shown_y++;
    pm_y_clip += half_height;
#else
    sprite_y  += pm_diamond_half_height;
    pm_shown_y++;
    pm_y_clip += pm_diamond_half_height;
#endif
}

extern void xclip(int clip_left, int clip_right);
extern void yclip(int clip_top, int clip_bottom);
extern void refresh_region_sprite_square(int tile_x, int tile_y);

// Draw an unclipped row of building tops and sprites, including adjacent spillover sprites.
// FUNCTION: C2 0x39c9f
// FUNCTION: C2WIN 0x004464de
void mid2_line_no_sides_top(void)
{
    int ptr;
    int x;
    int y;
    int col_idx;
#if PLATFORM_WINDOWS
    unsigned char redraw;

    redraw = 0;
    if (pm_shown_y >= PM_H) return;
#endif
    if (pm_x > 0) {
        sprite_x = pm_screen_x_start - pm_diamond_width;
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_x - 1];
        if (!((pm_shown_ptr) >= 0x0FFF0000)) place2_sprite(2);
    }
    sprite_x = pm_screen_x_start;
#if PLATFORM_WINDOWS
    if (pm_shown_y >= PM_H) return;
#endif
    col_idx = 0;
    pm_shown_x = pm_x;
    for (; col_idx < pm_screen_width; col_idx++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_x += pm_diamond_width;
        } else {
            if ((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).base_kind > 0x7c) {
                place2_a_building_top(0);
                place2_sprite(0);
            } else {
                sprite_x += pm_diamond_width;
                place2_sprite(0);
            }
        }
    }

#if PLATFORM_WINDOWS
    if (pm_shown_y >= PM_H) return;
#endif
    if (pm_shown_x < 80) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x];
        if (!((pm_shown_ptr) >= 0x0FFF0000)) place2_sprite(2);
    }

    sprite_y += pm_diamond_half_height;
    pm_shown_y++;
    pm_y_clip += pm_diamond_half_height;
}

// Draw a top-layer row with clipped building and sprite styles at both viewport edges.
// FUNCTION: C2 0x39dd3
// FUNCTION: C2WIN 0x004466a9
void mid2_line_with_sides_top(void)
{
    int ptr;
    int x;
    int y;
    int col_idx;

#if PLATFORM_WINDOWS
    if (pm_shown_y >= PM_H) return;
#endif
    pm_shown_x = pm_x;
    sprite_x   = pm_screen_x_start;

    /* left edge */
    pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
    if (pm_shown_ptr >= 0x0fff0000) {
    } else if ((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).base_kind > 0x7c) {
        place2_a_building_top(3);
        place2_sprite(1);
    } else {
        place2_sprite(1);
    }
    sprite_x += pm_diamond_half_width;

#if PLATFORM_WINDOWS
    if (pm_shown_y >= PM_H) return;
#endif
    /* middle */
    for (col_idx = 0; col_idx < pm_screen_width - 1; col_idx++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_x += pm_diamond_width;
        } else {
            if ((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).base_kind > 0x7c) {
                place2_a_building_top(0);
                place2_sprite(0);
            } else {
                sprite_x += pm_diamond_width;
                place2_sprite(0);
            }
        }
    }

#if PLATFORM_WINDOWS
    if (pm_shown_y >= PM_H) return;
#endif
    /* right edge */
    pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
    if (pm_shown_ptr >= 0x0fff0000) {
    } else if ((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).base_kind > 0x7c) {
        place2_a_building_top(4);
        place2_sprite(2);
    } else {
        place2_sprite(2);
    }

    sprite_y  += pm_diamond_half_height;
    pm_shown_y++;
    pm_y_clip += pm_diamond_half_height;
}

// Draw a clipped bottom row using building roofs, stopping at the region-map boundary.
// FUNCTION: C2 0x39f31
// FUNCTION: C2WIN 0x004468c6
void bottom2_line_with_sides(void)
{
    int i;

    if (pm_shown_y >= PM_H) return;

    pm_shown_x = pm_x;
    sprite_x   = pm_screen_x_start;

    /* left edge */
    pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
    if (pm_shown_ptr >= 0x0fff0000) {
    } else if ((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).base_kind > 0x7c) {
        place2_a_building_roof(3);
        place2_sprite(1);
    } else {
        place2_sprite(1);
    }
    sprite_x += pm_diamond_half_width;

#if PLATFORM_WINDOWS
    if (pm_shown_y >= PM_H) return;
#endif
    /* middle */
    for (i = 0; i < pm_screen_width - 1; i++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_x += pm_diamond_width;
        } else {
            if ((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).base_kind > 0x7c) {
                place2_a_building_roof(0);
                place2_sprite(0);
            } else {
                sprite_x += pm_diamond_width;
                place2_sprite(0);
            }
        }
    }

#if PLATFORM_WINDOWS
    if (pm_shown_y >= PM_H) return;
#endif
    /* right edge */
    pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
    if (pm_shown_ptr >= 0x0fff0000) {
    } else if ((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).base_kind > 0x7c) {
        place2_a_building_roof(4);
        place2_sprite(2);
    } else {
        place2_sprite(2);
    }

    sprite_y  += pm_diamond_half_height;
    pm_shown_y++;
    pm_y_clip += pm_diamond_half_height;
}

extern void refresh_a_bigger_square(int tile_x, int tile_y);
extern void get_text_pointer(int entry_idx, int word_count);
extern void set_prov_ambient(int event);
extern void write_large_diamond_leftroof(unsigned char *sprite_addr);

// Draw an unclipped bottom row using building roofs and adjacent spillover sprites.
// FUNCTION: C2 0x3a096
// FUNCTION: C2WIN 0x00446ae3
void bottom2_line_no_sides(void)
{
    int col_idx;

    if (pm_shown_y >= PM_H) return;

    /* left spillover */
    if (pm_x > 0) {
        sprite_x = pm_screen_x_start - pm_diamond_width;
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_x - 1];
        if (!((pm_shown_ptr) >= 0x0FFF0000)) place2_sprite(2);
    }

#if PLATFORM_WINDOWS
    if (pm_shown_y >= PM_H) return;
#endif
    sprite_x = pm_screen_x_start;
    col_idx = 0;
    pm_shown_x = pm_x;
    for (; col_idx < pm_screen_width; col_idx++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_x += pm_diamond_width;
        } else {
            if ((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).base_kind > 0x7c) {
                place2_a_building_roof(0);
                place2_sprite(0);
            } else {
                sprite_x += pm_diamond_width;
                place2_sprite(0);
            }
        }
    }

#if PLATFORM_WINDOWS
    if (pm_shown_y >= PM_H) return;
#endif

    /* right spillover */
    if (pm_shown_x < 80) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x];
        if (!((pm_shown_ptr) >= 0x0FFF0000)) place2_sprite(2);
    }

    sprite_y  += pm_diamond_half_height;
    pm_shown_y++;
    pm_y_clip += pm_diamond_half_height;
}

// Render the base diamond of a building cell on the region map.
// FUNCTION: C2 0x3a1cc
// FUNCTION: C2WIN 0x00446caa
void place2_a_building_base(int draw_style)
{
#if PLATFORM_WINDOWS
    unsigned char flags;
    int mode_no;
    unsigned char height;
    int width;
#else
    char bank_kind;
    char header_high_byte;
    int header_byte;
    int rotation_idx;
#endif
    unsigned char *sprite_bank_ptr;

#if PLATFORM_WINDOWS
    if (screen_mode > 1) mode_no = 0;
    else mode_no = screen_mode;
#endif
    sprite_image_no = (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).gfx;
#if PLATFORM_WINDOWS
    flags = (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits & 0x1c;

    if (flags == 0) { sprite_bank_ptr = (&house_data)[mode_no]; sprite_image_no = ((struct rotated_sprite_rec *)rotated2_bank0)[sprite_image_no].dir[map_direction >> 1];
    } else if (flags == 4) { sprite_bank_ptr = (&building_data1)[mode_no]; sprite_image_no = ((struct rotated_sprite_rec *)rotated2_bank1)[sprite_image_no].dir[map_direction >> 1];
    } else if (flags == 8) { sprite_bank_ptr = (&building_data2)[mode_no]; sprite_image_no = ((struct rotated_sprite_rec *)rotated2_bank2)[sprite_image_no].dir[map_direction >> 1];
    } else if (flags == 0xc) { sprite_bank_ptr = (&building_data3)[mode_no]; sprite_image_no = ((struct rotated_sprite_rec *)rotated2_bank3)[sprite_image_no].dir[map_direction >> 1];
    } else if (flags == 0x10) {
        sprite_bank_ptr = (&fixt_data)[mode_no]; sprite_image_no = ((unsigned char *)rotated2_map)[(map_direction >> 1) + sprite_image_no * 4] + 0x10; } else { return; }
#else
    bank_kind = (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits & 0x1c; rotation_idx = (map_direction >> 1) + sprite_image_no * 4;

    if (bank_kind == 0) { sprite_bank_ptr = house_data; sprite_image_no = rotated2_bank0[rotation_idx];
    } else if (bank_kind == 4) { sprite_bank_ptr = building_data1; sprite_image_no = rotated2_bank1[rotation_idx];
    } else if (bank_kind == 8) { sprite_bank_ptr = building_data2; sprite_image_no = rotated2_bank2[rotation_idx];
    } else if (bank_kind == 0xc) { sprite_bank_ptr = building_data3; sprite_image_no = rotated2_bank3[rotation_idx];
    } else if (bank_kind == 0x10) {
        sprite_bank_ptr = fixt_data; sprite_image_no = rotated2_map[sprite_image_no - 0x10].dir[map_direction >> 1] + 0x10; } else { return; }
#endif

    set_prov_ambient((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).base_kind);
    data_ptr     = sprite_image_no * 16 + 8;
    y_length     = sprite_bank_ptr[data_ptr + 0xd];
#if PLATFORM_WINDOWS
    height       = sprite_bank_ptr[data_ptr + 0xc];
    sprite_start = (sprite_bank_ptr[data_ptr + 6] << 16)
                 + (sprite_bank_ptr[data_ptr + 5] << 8)
                 + sprite_bank_ptr[data_ptr + 4];
#else
    sprite_start = ((header_byte = sprite_bank_ptr[data_ptr + 5]) << 8)
                 + (header_byte = sprite_bank_ptr[data_ptr + 4])
                 + ((header_high_byte = sprite_bank_ptr[data_ptr + 6]) << 16);
#endif
    if (sprite_start > 0x4baf0) { sprite_error++; return; }
    if (sprite_start < 0) { sprite_error++; return; }
    if (y_length > 0xc8) { sprite_error++; return; }
    if (y_length < 0) { sprite_error++; return; }

    if (((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits & 1) != 0) {
        (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits &= 0xfe;
#if PLATFORM_DOS
        refresh_a_bigger_square(sprite_x >> 4, (sprite_y - 0x30) >> 4);
#endif
    }

    if (draw_style == 3) {
        if      (zoom_level == 0) place_i_large_diamond_lefthalf(sprite_bank_ptr, 0);
        else if (zoom_level == 1) place_i_medium_diamond_lefthalf(sprite_bank_ptr, 0);
        else                       place_i_small_diamond_lefthalf(sprite_bank_ptr, 0);
    } else if (draw_style == 4) {
        if      (zoom_level == 0) place_i_large_diamond_righthalf(sprite_bank_ptr, 0);
        else if (zoom_level == 1) place_i_medium_diamond_righthalf(sprite_bank_ptr, 0);
        else                       place_i_small_diamond_righthalf(sprite_bank_ptr, 0);
    } else {
        if      (zoom_level == 0) place_i_large_diamond(sprite_bank_ptr, draw_style);
        else if (zoom_level == 1) place_i_medium_diamond(sprite_bank_ptr, draw_style);
        else                       place_i_small_diamond(sprite_bank_ptr, draw_style);
        sprite_x += pm_diamond_width;
    }
}

extern void write_large_diamond_hat(unsigned char *sprite_addr, int y_clip);
extern void write_medium_diamond_hat(unsigned char *sprite_addr, int y_clip);
extern void write_small_diamond_hat(unsigned char *sprite_addr, int y_clip);
extern void write_large_diamond_lefthat(unsigned char *sprite_addr, int y_clip);
extern void write_large_diamond_roof(unsigned char *sprite_addr);

// Draw a building's visible upper slice, province label, and any regional marker.
// FUNCTION: C2 0x3a402
// FUNCTION: C2WIN 0x0044707e
void place2_a_building_top(int draw_style)
{
#if PLATFORM_WINDOWS
    unsigned char bank_index;
    int mode;
    int rotation;
#else
    unsigned char bank_index;
    int rotation_idx;
#endif
    unsigned char *bank;
    unsigned char sprite_kind;
    unsigned char sprite_class;
    int edge_class;
    int label_x;
    int y_offset;
    int label_width;
    unsigned char province_no;
    unsigned char goods_amount;
    unsigned char qty;
    int goods_x;
    int sprite_y_off;
#if PLATFORM_DOS
    int header_byte;
    int header_high_byte;
#endif

#if PLATFORM_WINDOWS
    if (screen_mode > 1) mode = 0;
    else mode = screen_mode;
#endif
    sprite_image_no = (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).gfx;

#if PLATFORM_WINDOWS
    bank_index = (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits & 0x1c;

    if (bank_index == 0) { bank = (&house_data)[mode]; sprite_image_no = ((struct rotated_sprite_rec *)rotated2_bank0)[sprite_image_no].dir[map_direction >> 1]; }
    else if (bank_index == 4) { bank = (&building_data1)[mode]; sprite_image_no = ((struct rotated_sprite_rec *)rotated2_bank1)[sprite_image_no].dir[map_direction >> 1]; }
    else if (bank_index == 8) { bank = (&building_data2)[mode]; sprite_image_no = ((struct rotated_sprite_rec *)rotated2_bank2)[sprite_image_no].dir[map_direction >> 1]; }
    else if (bank_index == 0xc) { bank = (&building_data3)[mode]; sprite_image_no = ((struct rotated_sprite_rec *)rotated2_bank3)[sprite_image_no].dir[map_direction >> 1]; }
    else if (bank_index == 0x10) {
        bank = (&fixt_data)[mode]; sprite_image_no = ((unsigned char *)rotated2_map)[(map_direction >> 1) + sprite_image_no * 4] + 0x10; }
    else return;
#else
    bank_index = (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits & 0x1c; rotation_idx = (map_direction >> 1) + sprite_image_no * 4;

    if (bank_index == 0) { bank = house_data; sprite_image_no = rotated2_bank0[rotation_idx] & 0xff; }
    else if (bank_index == 4) { bank = building_data1; sprite_image_no = rotated2_bank1[rotation_idx] & 0xff; }
    else if (bank_index == 8) { bank = building_data2; sprite_image_no = rotated2_bank2[rotation_idx] & 0xff; }
    else if (bank_index == 0xc) { bank = building_data3; sprite_image_no = (unsigned char)rotated2_bank3[rotation_idx]; }
    else if (bank_index == 0x10) {
        bank = fixt_data; sprite_image_no = (rotated2_map[sprite_image_no - 0x10].dir[map_direction >> 1] & 0xff) + 0x10; }
    else return;
#endif

    data_ptr     = sprite_image_no * 16 + 8;
    y_length     = bank[data_ptr + 0xd];
    sprite_class = bank[data_ptr + 0xc];
#if PLATFORM_WINDOWS
    sprite_start = bank[data_ptr + 4]
                 + (bank[data_ptr + 5] << 8)
                 + (bank[data_ptr + 6] << 16);
#else
    sprite_start = bank[data_ptr + 4]
                 + (bank[data_ptr + 5] << 8)
                 + (bank[data_ptr + 6] << 16);
#endif
    if (zoom_level == 0)      sprite_hat_start = sprite_start + 0x384;
    else if (zoom_level == 1) sprite_hat_start = sprite_start + 0xc4;
    else                      sprite_hat_start = sprite_start + 0x24;

    if (sprite_start > 0x4baf0) { sprite_error++; return; }
    if (sprite_start < 0) { sprite_error++; return; }
    if (y_length > 0xc8) { sprite_error++; return; }
    if (y_length < 0) { sprite_error++; return; }

    if (draw_style == 3) {
        if (y_length != 0) {
            if (sprite_class == 2) {
                if      (zoom_level == 0) write_large_diamond_lefthat(bank, pm_y_clip);
                else if (zoom_level == 1) write_medium_diamond_lefthat(bank, pm_y_clip);
                else                      write_small_diamond_lefthat(bank, pm_y_clip);
            } else if (sprite_class == 4) {
                if      (zoom_level == 0) write_large_diamond_righthalfhat(bank, pm_y_clip, 2);
                else if (zoom_level == 1) write_medium_diamond_righthalfhat(bank, pm_y_clip, 2);
                else                      write_small_diamond_righthalfhat(bank, pm_y_clip, 2);
            }
        }
    } else if (draw_style == 4) {
        if (y_length != 0) {
            if (sprite_class == 2) {
                if      (zoom_level == 0) write_large_diamond_righthat(bank, pm_y_clip);
                else if (zoom_level == 1) write_medium_diamond_righthat(bank, pm_y_clip);
                else                      write_small_diamond_righthat(bank, pm_y_clip);
            }
            if (sprite_class == 3) {
                if      (zoom_level == 0) write_large_diamond_lefthalfhat(bank, pm_y_clip, 2);
                else if (zoom_level == 1) write_medium_diamond_lefthalfhat(bank, pm_y_clip, 2);
                else                      write_small_diamond_lefthalfhat(bank, pm_y_clip, 2);
            }
        }
    } else {
        if (draw_style != 2 && y_length != 0) {
            if (sprite_class == 2) {
                if      (zoom_level == 0) write_large_diamond_hat(bank, pm_y_clip);
                else if (zoom_level == 1) write_medium_diamond_hat(bank, pm_y_clip);
                else                      write_small_diamond_hat(bank, pm_y_clip);
            } else if (sprite_class == 3) {
                if      (zoom_level == 0) write_large_diamond_lefthalfhat(bank, pm_y_clip, 0);
                else if (zoom_level == 1) write_medium_diamond_lefthalfhat(bank, pm_y_clip, 0);
                else                      write_small_diamond_lefthalfhat(bank, pm_y_clip, 0);
            } else if (sprite_class == 4) {
                if      (zoom_level == 0) write_large_diamond_righthalfhat(bank, pm_y_clip, 0);
                else if (zoom_level == 1) write_medium_diamond_righthalfhat(bank, pm_y_clip, 0);
                else                      write_small_diamond_righthalfhat(bank, pm_y_clip, 0);
            }
        }
        sprite_x += pm_diamond_width;
    }

    sprite_kind = (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).base_kind;
    if (sprite_kind >= 0x98 && sprite_kind <= 0x9f) {
        old_sprite_y = sprite_y; old_sprite_x = sprite_x;
        if (sprite_kind == 0x98 || sprite_kind == 0x9c)      edge_class = 0;
        else if (sprite_kind == 0x99 || sprite_kind == 0x9d) edge_class = 2;
        else if (sprite_kind == 0x9a || sprite_kind == 0x9e) edge_class = 4;
        else                                   edge_class = 6;

        province_no = region_borders[province_is].u.dir[edge_class >> 1];
        if (province_no != 0) province_no++;
        get_text_pointer(6, province_no);
        label_width = get_string_width(text_pointer, font1);

        if (map_direction == 2)      edge_class += 6;
        else if (map_direction == 4) edge_class += 4;
        else if (map_direction == 6) edge_class += 2;
        if (edge_class >= 8) edge_class %= 8;

        if (edge_class == 0)      { label_x = -8; y_offset = -8; }
        else if (edge_class == 2) { label_x = 0; y_offset = 0x28; }
        else if (edge_class == 4) { label_x = -(label_width + 0x3c); y_offset = 0x2c; }
        else                      { label_x = -(label_width + 0x3c); y_offset = -0xc; }

        font_screen_limit = 1;
        put_a_font_string(text_pointer, sprite_x + label_x, sprite_y + y_offset, font1, 0x20);
        font_screen_limit = 1;
        sprite_y = old_sprite_y; sprite_x = old_sprite_x;
        font_list(0x1b, 0, sprite_x + label_x, sprite_y + y_offset - 0x14, font1, 0x20);
        sprite_y = old_sprite_y; sprite_x = old_sprite_x;
    }
    else if (sprite_kind == 0xd4) {
        goods_amount = (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).occupant & 0xf0;
        qty = (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).occupant & 0xf;
        goods_amount >>= 4;
        if (((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits & 0x40) == 0) return;
        sprite_image_no = goods_amount + 9;
#if PLATFORM_WINDOWS
        goods_x = *(int *)((unsigned char *)reg_type_x_off
                               + (map_direction / 2) * 4 + zoom_level * 16);
        sprite_y_off = *(int *)((unsigned char *)reg_type_y_off
                               + (map_direction / 2) * 4 + zoom_level * 16);
#else
        goods_x = reg_type_x_off[zoom_level][map_direction / 2];
        sprite_y_off = reg_type_y_off[zoom_level][map_direction / 2];
#endif
        if (draw_style == 1) goods_x -= pm_diamond_half_width;

        data_ptr = sprite_image_no * 16 + 8;
#if PLATFORM_WINDOWS
        sprite_start = ((&tops_data)[mode][data_ptr + 6] << 16)
                     + ((&tops_data)[mode][data_ptr + 5] << 8)
                     + (&tops_data)[mode][data_ptr + 4];
        sprite_width = ((&tops_data)[mode][data_ptr + 1] << 8)
                     + (&tops_data)[mode][data_ptr];
        sprite_height = ((&tops_data)[mode][data_ptr + 3] << 8)
                      + (&tops_data)[mode][data_ptr + 2];
#else
        bank = tops_data + data_ptr; sprite_start = ((header_byte = bank[5]) << 8)
                     + (header_byte = bank[4])
                     + ((header_high_byte = bank[6]) << 16);
        sprite_width = (header_byte = bank[0]) + ((header_byte = bank[1]) << 8);
        sprite_height = (bank[2]) + (bank[3] << 8);
#endif

        if (sprite_start > 0x4baf0) { sprite_error++; return; }
        if (sprite_start < 0)       { sprite_error++; return; }
        if (sprite_width > 0x280)   { sprite_error++; return; }
        if (sprite_height > 0x1e0)  { sprite_error++; return; }

        old_sprite_x = sprite_x; old_sprite_y = sprite_y;
        sprite_x += goods_x; sprite_y += sprite_y_off;

#if PLATFORM_WINDOWS
        xclip(pm_screen_x_start, pm_screen_x_end);
        yclip(pm_screen_y_start + pm_diamond_height, pm_screen_y_end);
#else
        xclip(pm_screen_x_start, 0x1de);
        yclip(0x18, 0x1da);
#endif
        if (yclipped == 5) goto put_back;
#if PLATFORM_WINDOWS
        if      (xclipped == 1) write_i_left_sprite((&tops_data)[mode]);
        else if (xclipped == 2) write_i_right_sprite((&tops_data)[mode]);
        else                    write_i_sprite((&tops_data)[mode]);
#else
        if      (xclipped == 1) write_i_left_sprite(tops_data);
        else if (xclipped == 2) write_i_right_sprite(tops_data);
        else                    write_i_sprite(tops_data);
#endif
put_back: sprite_x = old_sprite_x; sprite_y = old_sprite_y;
    }
}

extern void write_large_diamond_righthalfroof(unsigned char *sprite_addr, int side);

// Draw the portion of a building roof that crosses the bottom of the visible map slice.
// FUNCTION: C2 0x3ab26
// FUNCTION: C2WIN 0x00447bf3
void place2_a_building_roof(int draw_style)
{
#if PLATFORM_WINDOWS
    unsigned char flags;
    int mode;
#else
    int rotation_idx;
    unsigned char flags;
#endif
    unsigned char *bank;
    unsigned char height;

#if PLATFORM_WINDOWS
    if (screen_mode > 1) mode = 0;
    else mode = screen_mode;
#endif
    old_sprite_y = sprite_y;
    sprite_image_no = (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).gfx;
#if PLATFORM_WINDOWS
    flags = (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits & 0x1c;

    if (flags == 0) {
        bank = (&house_data)[mode];
        sprite_image_no = ((struct rotated_sprite_rec *)rotated2_bank0)[sprite_image_no].dir[map_direction >> 1];
    } else if (flags == 4) {
        bank = (&building_data1)[mode];
        sprite_image_no = ((struct rotated_sprite_rec *)rotated2_bank1)[sprite_image_no].dir[map_direction >> 1];
    } else if (flags == 8) {
        bank = (&building_data2)[mode];
        sprite_image_no = ((struct rotated_sprite_rec *)rotated2_bank2)[sprite_image_no].dir[map_direction >> 1];
    } else if (flags == 0xc) {
        bank = (&building_data3)[mode];
        sprite_image_no = ((struct rotated_sprite_rec *)rotated2_bank3)[sprite_image_no].dir[map_direction >> 1];
    } else if (flags == 0x10) {
        bank = (&fixt_data)[mode];
        sprite_image_no = ((unsigned char *)rotated2_map)[(map_direction >> 1) + sprite_image_no * 4] + 0x10;
    } else {
        return;
    }
#else
    flags = (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits & 0x1c;
    rotation_idx = (map_direction >> 1) + sprite_image_no * 4;

    if (flags == 0) {
        bank = house_data;
        sprite_image_no = rotated2_bank0[rotation_idx];
    } else if (flags == 4) {
        bank = building_data1;
        sprite_image_no = rotated2_bank1[rotation_idx];
    } else if (flags == 8) {
        bank = building_data2;
        sprite_image_no = rotated2_bank2[rotation_idx];
    } else if (flags == 0xc) {
        bank = building_data3;
        sprite_image_no = rotated2_bank3[rotation_idx];
    } else if (flags == 0x10) {
        bank = fixt_data;
        sprite_image_no = (rotated2_map[sprite_image_no - 0x10].dir[map_direction >> 1] & 0xff) + 0x10;
    } else {
        return;
    }
#endif

    data_ptr     = sprite_image_no * 16 + 8;
    y_length     = bank[data_ptr + 0xd];
    if (y_length <= pm_y_clip) {
        if (draw_style < 3)
            sprite_x += pm_diamond_width;
        return;
    }

    height = bank[data_ptr + 0xc];
#if PLATFORM_WINDOWS
    sprite_start = (bank[data_ptr + 5] << 8)
                 + (bank[data_ptr + 6] << 16)
                 + bank[data_ptr + 4];
#else
    sprite_start = bank[data_ptr + 4]
                 + (bank[data_ptr + 5] << 8)
                 + (bank[data_ptr + 6] << 16);
#endif
    if      (zoom_level == 0) sprite_hat_start = sprite_start + 0x384;
    else if (zoom_level == 1) sprite_hat_start = sprite_start + 0xc4;
    else                       sprite_hat_start = sprite_start + 0x24;

    if (sprite_start > 0x4baf0) {
        sprite_error++;
    } else if (sprite_start < 0) {
        sprite_error++;
    } else if (y_length > 0xc8) {
        sprite_error++;
    } else if (y_length < 0) {
        sprite_error++;
    } else {
        sprite_y = pm_screen_y_end - 1;
        if (((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits & 1) != 0) {
            (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits &= 0xfe;
#if PLATFORM_DOS
            refresh_a_bigger_square(sprite_x >> 4, (sprite_y - 0x30) >> 4);
#endif
        }
        y_length -= pm_y_clip;

        if (zoom_level == 0) {
            if (height == 2) sprite_hat_start += pm_y_clip * 0x3a;
            else                   sprite_hat_start += pm_y_clip * 30;
        } else if (zoom_level == 1) {
            if (height == 2) sprite_hat_start += pm_y_clip * 0x1a;
            else                   sprite_hat_start += pm_y_clip * 14;
        } else {
            if (height == 2) sprite_hat_start += pm_y_clip * 10;
            else                   sprite_hat_start += pm_y_clip * 6;
        }

        if (draw_style == 3) {
            if (height == 2) {
                if      (zoom_level == 0) write_large_diamond_leftroof(bank);
                else if (zoom_level == 1) write_medium_diamond_leftroof(bank);
                else                       write_small_diamond_leftroof(bank);
            } else if (height == 4) {
                if      (zoom_level == 0) write_large_diamond_righthalfroof(bank, 2);
                else if (zoom_level == 1) write_medium_diamond_righthalfroof(bank, 2);
                else                       write_small_diamond_righthalfroof(bank, 2);
            }
        } else if (draw_style == 4) {
            if (height == 2) {
                if      (zoom_level == 0) write_large_diamond_rightroof(bank);
                else if (zoom_level == 1) write_medium_diamond_rightroof(bank);
                else                       write_small_diamond_rightroof(bank);
            }
            if (height == 3) {
                if      (zoom_level == 0) write_large_diamond_lefthalfroof(bank, 2);
                else if (zoom_level == 1) write_medium_diamond_lefthalfroof(bank, 2);
                else                       write_small_diamond_lefthalfroof(bank, 2);
            }
        } else {
            if (height == 2) {
                if      (zoom_level == 0) write_large_diamond_roof(bank);
                else if (zoom_level == 1) write_medium_diamond_roof(bank);
                else                       write_small_diamond_roof(bank);
            } else if (height == 3) {
                if      (zoom_level == 0) write_large_diamond_lefthalfroof(bank, 0);
                else if (zoom_level == 1) write_medium_diamond_lefthalfroof(bank, 0);
                else                       write_small_diamond_lefthalfroof(bank, 0);
            } else if (height == 4) {
                if      (zoom_level == 0) write_large_diamond_righthalfroof(bank, 0);
                else if (zoom_level == 1) write_medium_diamond_righthalfroof(bank, 0);
                else                       write_small_diamond_righthalfroof(bank, 0);
            }
            sprite_x += pm_diamond_width;
        }
        sprite_y = old_sprite_y;
    }
}

// Draw a cell's army layers together with its flag and event overlays.
// FUNCTION: C2 0x3af6b
// FUNCTION: C2WIN 0x004482d9
void place2_sprite(int draw_style)
{
    int marker_kind;
    int y_offset;
    int x_offset;
    int relative_direction;
    unsigned char *sprite_header_ptr;
    unsigned char overlay_flags;
    int sprite_top_y;

    army_a = (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).occupant;
    marker_kind = 0;
    if (flag_mode != 0)
        marker_kind = (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).place_state;

    if (army_a != 0) {
        if (army_a < 0) return;
        if (army_a >= 0x1a) return;
        if (army_list[army_a].exists == 0) return;
        if (army_list[army_a].exists == 3) return;
        if (((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).terrain & 1) != 0) return;

        if      (army_list[army_a].type <= 1) set_this_ambient(9);
        else if (army_list[army_a].type <= 4) set_this_ambient(8);
        else if (army_list[army_a].type == 5) set_this_ambient(0xa);

        relative_direction = army_list[army_a].world_dir - map_direction;
        if (relative_direction < 0) relative_direction += 8;

        if (zoom_level == 0) {
            x_offset = walking_x_ofsets_zoom0[relative_direction * 16 + army_list[army_a].target_kind];
            y_offset = walking_y_ofsets_zoom0[relative_direction * 16 + army_list[army_a].target_kind];
        } else if (zoom_level == 1) {
            x_offset = walking_x_ofsets_zoom1[relative_direction * 16 + army_list[army_a].target_kind];
            y_offset = walking_y_ofsets_zoom1[relative_direction * 16 + army_list[army_a].target_kind];
        } else {
            x_offset = walking_x_ofsets_zoom2[relative_direction * 16 + army_list[army_a].target_kind];
            y_offset = walking_y_ofsets_zoom2[relative_direction * 16 + army_list[army_a].target_kind];
        }

        if (draw_style == 1) {
            x_offset -= 2;
        } else if (draw_style == 2) {
            x_offset += pm_diamond_half_width - 2;
        } else {
            x_offset += pm_diamond_half_width - 2 - pm_diamond_width;
        }

        if      (zoom_level == 0) y_offset += 0x18;
        else if (zoom_level == 1) y_offset += 8;
        else                       y_offset += 2;

        /* --- Pass 1: main body --- */
        sprite_image_no = army_list[army_a].sprite_image;
        data_ptr        = sprite_image_no * 16 + 8;
        sprite_header_ptr          = people_data + data_ptr; sprite_start = sprite_header_ptr[4] + (sprite_header_ptr[5] << 8) + (sprite_header_ptr[6] << 16);
        sprite_width    = (sprite_header_ptr[0]) + (sprite_header_ptr[1] << 8);
        sprite_height   = sprite_header_ptr[2] + (sprite_header_ptr[3] << 8);
        if (sprite_start > 0x4baf0) { sprite_error++; return; }
        if (sprite_width <= 0)      { sprite_error++; return; }
        if (sprite_width > 0x12c)   { sprite_error++; return; }
        if (sprite_height <= 0)     { sprite_error++; return; }
        if (sprite_height > 0x12c)  { sprite_error++; return; }

        /* --- Read sprite2 header (optional) --- */
        if (army_list[army_a].sprite_anim != 0) {
            sprite2_image_no = army_list[army_a].sprite_anim;
            data_ptr         = sprite2_image_no * 16 + 8;
            sprite_header_ptr           = people_data + data_ptr; sprite2_start = (sprite_header_ptr[6] << 16) + ((sprite_header_ptr[4]) + (sprite_header_ptr[5] << 8));
            sprite2_height   = (sprite_header_ptr[2]) + (sprite_header_ptr[3] << 8);
        } else {
            sprite2_height = 0;
        }

        /* --- Read sprite3 header (optional) --- */
        if (army_list[army_a].sprite_dir != 0) {
            sprite3_image_no = army_list[army_a].sprite_dir;
            data_ptr         = sprite3_image_no * 16 + 8;
            sprite_header_ptr           = people_data + data_ptr; sprite3_start = sprite_header_ptr[4] + (sprite_header_ptr[5] << 8) + (sprite_header_ptr[6] << 16);
            sprite3_height   = sprite_header_ptr[2] + (sprite_header_ptr[3] << 8);
        } else {
            sprite3_height = 0;
        }

        /* --- Snapshot + centre + draw body --- */
        old_sprite_x = sprite_x; old_sprite_y = sprite_y;
        sprite_x    += x_offset; sprite_y += y_offset;
        sprite_x    -= sprite_width >> 1;
        sprite_y    -= sprite_height + sprite2_height + sprite3_height;
        sprite_base_x = sprite_x;
        sprite_base_y = sprite_y;
        refresh_region_sprite_square(sprite_x >> 4, sprite_y >> 4);
        xclip(pm_screen_x_start, 0x1de);
        if (zoom_level == 1) yclip(0x18, 0x1d8);
        else                  yclip(0x18, 0x1da);
        if (yclipped == 5) goto after_body;
        if      (xclipped == 1) write_i_left_sprite(people_data);
        else if (xclipped == 2) write_i_right_sprite(people_data);
        else                    write_i_sprite(people_data);
after_body:

        /* --- Pass 2: sprite2 stacked above body --- */
        if (army_list[army_a].sprite_anim != 0) {
            sprite_x        = sprite_base_x;
            sprite_y        = sprite_base_y;
            sprite_y       += sprite_height;
            sprite_base_y   = sprite_y;
            sprite_image_no = sprite2_image_no; sprite_start = sprite2_start; sprite_height = sprite2_height;
            if (sprite_start > 0x4baf0) { sprite_error++; return; }
            if (sprite_height <= 0)     { sprite_error++; return; }
            if (sprite_height > 0x12c)  { sprite_error++; return; }
            xclip(pm_screen_x_start, 0x1de);
            if (zoom_level == 1) yclip(0x18, 0x1d8);
            else                  yclip(0x18, 0x1da);
            if (yclipped == 5) goto after_sprite2;
            if      (xclipped == 1) write_i_left_sprite(people_data);
            else if (xclipped == 2) write_i_right_sprite(people_data);
            else                    write_i_sprite(people_data);
after_sprite2:

            if      (zoom_level == 0) army_list[army_a].map_x = sprite_x + 8;
            else if (zoom_level == 1) army_list[army_a].map_x = sprite_x + 4;
            else                       army_list[army_a].map_x = sprite_x;
            army_list[army_a].map_y = sprite_y;
        }

        /* --- Pass 3: sprite3 stacked above sprite2 --- */
        if (army_list[army_a].sprite_dir != 0) {
            sprite_x        = sprite_base_x;
            sprite_y        = sprite_base_y;
            sprite_y       += sprite_height;
            sprite_image_no = sprite3_image_no; sprite_start = sprite3_start; sprite_height = sprite3_height;
            if (sprite_start > 0x4baf0) { sprite_error++; return; }
            if (sprite_height <= 0)     { sprite_error++; return; }
            if (sprite_height > 0x12c)  { sprite_error++; return; }
            xclip(pm_screen_x_start, 0x1de);
            if (zoom_level == 1) yclip(0x18, 0x1d8);
            else                  yclip(0x18, 0x1da);
            if (army_list[army_a].morale_timer != 0) goto after_sprite3;
            if (yclipped == 5) goto after_sprite3;
            if      (xclipped == 1) write_i_left_sprite(people_data);
            else if (xclipped == 2) write_i_right_sprite(people_data);
            else                    write_i_sprite(people_data);
after_sprite3:
            if      (zoom_level == 0) army_list[army_a].map_x = sprite_x + 8;
            else if (zoom_level == 1) army_list[army_a].map_x = sprite_x + 4;
            else                       army_list[army_a].map_x = sprite_x;
            army_list[army_a].map_y = sprite_y;
        }

        sprite_x = old_sprite_x;
        sprite_y = old_sprite_y;
    }

    overlay_flags  = (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits & 0xc0;
    if (((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).base_kind) == 0xd4)
        overlay_flags = 0;
    if (overlay_flags != 0) {
        y_offset = 0;
        if (draw_style == 1) {
            x_offset = -2;
        } else {
            x_offset = pm_diamond_half_width - 2;
            if (draw_style != 2)
                x_offset -= pm_diamond_width;
        }
        y_offset += pm_diamond_half_height;

        if ((overlay_flags & 0x80) != 0) {
            sprite_image_no = 6;
            sprite_width    = 0x10;
            sprite_height   = 0x1c;
        } else if ((overlay_flags & 0x40) != 0) {
            sprite_image_no = 8;
            sprite_width    = 7;
            sprite_height   = 7;
        }
        data_ptr     = sprite_image_no * 16 + 8;
        sprite_start = (mice[data_ptr + 5] << 8) + (mice[data_ptr + 4]) + (mice[data_ptr + 6] << 16);
        if (sprite_start > 0x4baf0) { sprite_error++; return; }
        if (sprite_start < 0)       { sprite_error++; return; }
        old_sprite_x = sprite_x; old_sprite_y = sprite_y;
        sprite_x    += x_offset; sprite_y += y_offset; sprite_top_y = sprite_y - sprite_height;
        if      (zoom_level == 0) { sprite_x -= sprite_width >> 1; sprite_y = sprite_top_y; }
        else if (zoom_level == 1) { sprite_x -= sprite_width >> 2; sprite_y = sprite_top_y; }
        else                      { sprite_x -= sprite_width >> 3; sprite_y = sprite_top_y; }
        refresh_sprite_square(sprite_x >> 4, sprite_y >> 4);
        xclip(pm_screen_x_start, 0x1de);
        if (zoom_level == 1) yclip(0x18, 0x1d8);
        else                  yclip(0x18, 0x1da);
        if (yclipped == 5) goto restore_overlay;
        if      (xclipped == 1) write_i_left_sprite(mice);
        else if (xclipped == 2) write_i_right_sprite(mice);
        else                    write_i_sprite(mice);
restore_overlay:
        sprite_x = old_sprite_x;
        sprite_y = old_sprite_y;
    }

    if (marker_kind != 0) {
        y_offset = 0;
        if (draw_style == 1) {
            x_offset = -2;
        } else {
            x_offset = pm_diamond_half_width - 2;
            if (draw_style != 2)
                x_offset -= pm_diamond_width;
        }
        y_offset += pm_diamond_half_height;

        if (marker_kind == 3)
            sprite_image_no = zoom_level + 0xe;
        else
            sprite_image_no = zoom_level + 0xb;
        data_ptr      = sprite_image_no * 16 + 8;
        sprite_start  = mice[data_ptr + 4] + (mice[data_ptr + 5] << 8) + (mice[data_ptr + 6] << 16);
        sprite_width  = (mice[data_ptr + 0]) + (mice[data_ptr + 1] << 8);
        sprite_height = (mice[data_ptr + 2]) + (mice[data_ptr + 3] << 8);
        if (sprite_start > 0x4baf0) { sprite_error++; return; }
        if (sprite_width <= 0)      { sprite_error++; return; }
        if (sprite_width > 0x12c)   { sprite_error++; return; }
        if (sprite_height <= 0)     { sprite_error++; return; }
        if (sprite_height > 0x12c)  { sprite_error++; return; }
        old_sprite_x = sprite_x; old_sprite_y = sprite_y;
        sprite_x    += x_offset;
        sprite_y    += y_offset;
        sprite_y    -= sprite_height;
        refresh_sprite_square(sprite_x >> 4, sprite_y >> 4);
        xclip(pm_screen_x_start, 0x1de);
        if (zoom_level == 1) yclip(0x18, 0x1d8);
        else                  yclip(0x18, 0x1da);
        if (yclipped == 5) goto restore_event;
        if      (xclipped == 1)
            write_i_left_sprite(mice);
        else if (xclipped == 2)
            write_i_right_sprite(mice);
        else                    write_i_sprite(mice);
restore_event:
        sprite_x = old_sprite_x;
        sprite_y = old_sprite_y;
    }
}

// Print the selected per-cell diagnostic value over a region-map diamond.
// FUNCTION: C2 0x3ba9d
// FUNCTION: C2WIN 0x00449419
void print2_test_info(void)
{
    int v;
    int col;
    int fig;

    if (test_mode1 != 0) {
        if (((pm_shown_ptr) >= 0x0FFF0000)) return;
        old_sprite_x = sprite_x;
        old_sprite_y = sprite_y;
        v = ((*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).outside &= 0x40);
        col = (*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).edge_bits & 0x20;
        if (v != 0 && col != 0) v = 1;
        else if (col != 0) v = 2;
        else v = 0;
        col = 0x20;
        font_no(v, 0x20, " ",
                sprite_x + 0x14 - pm_diamond_width, sprite_y + 0xa,
                font1, col);
        sprite_x = old_sprite_x;
        sprite_y = old_sprite_y;
    } else if (test_mode2 != 0) {
        if (((pm_shown_ptr) >= 0x0FFF0000)) return;
        old_sprite_x = sprite_x;
        old_sprite_y = sprite_y;
        v = (signed char)(*(struct region_cell *)((unsigned char *)region_map + (pm_shown_ptr))).place_state;
        col = 0x20;
        font_no(v, 0x20, " ",
                sprite_x + 0x14 - pm_diamond_width, sprite_y + 0xa,
                font1, col);
        sprite_x = old_sprite_x;
        sprite_y = old_sprite_y;
    }
}
