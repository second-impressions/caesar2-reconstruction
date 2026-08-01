
#include "c2_data.h"

int elephant_riders[96] = { 16, -10, 15, -9, 15, -10, 16, -10, 15, -9, 15, -9, 18, -7, 17, -7, 17, -7, 18, -7, 17, -7, 17, -7, 16, -6, 16, -6, 15, -7, 16, -6, 15, -6, 15, -7, 11, -4, 11, -5, 11, -5, 12, -3, 12, -4, 12, -5, 7, -5, 7, -5, 7, -6, 7, -5, 7, -5, 7, -6, 5, -7, 6, -7, 6, -7, 5, -7, 6, -7, 6, -7, 6, -9, 7, -9, 7, -9, 6, -9, 7, -9, 7, -9, 12, -10, 12, -10, 12, -10, 12, -10, 12, -10, 12, -10 };


extern void font_no(int value, char pad_char, char *suffix, int x, int y, unsigned char *font, int color);
extern void write_i_sprite(unsigned char *sprite_addr);
extern void write_i_left_sprite(unsigned char *sprite_addr);
extern void write_i_right_sprite(unsigned char *sprite_addr);
/* Forward declarations (functions defined later in this file). */
void show_battlemap_base(void);
void show_battlemap_top(void);
void mid3_line_no_sides_base(void);
void mid3_line_with_sides_base(void);
void mid3_line_no_sides_top(void);
void mid3_line_with_sides_top(void);
void bottom3_line_with_sides(void);
void bottom3_line_no_sides(void);
void place3_sprite(int edge_style);
void print3_test_info(void);


// Render the battle pseudo-map, its figure and arrow overlays, zoom-one top clipping strip,
// pending cell updates, and the active battle-setup dialog.
// FUNCTION: C2 0x3bb88
// FUNCTION: C2WIN 0x0041dc80
void show_battlemap(void)
{
    int screen_y;

    sprite_error = 0;
    show_battlemap_base();
    show_battlemap_top();
    if (zoom_level == 1) {
#if PLATFORM_DOS
        for (screen_y = 0x18; screen_y < 0x164; screen_y += 8)
            show_internal_2x8(0, screen_y, 0);
#endif
#if PLATFORM_WINDOWS
    } else if (zoom_level == 2) {
        memset(internal_screen, 0, 0xb180);
        memset(internal_screen + 0x3f980, 0, 0xb180);
#endif
    }
    if (update_map != 0)
        --update_map;
#if PLATFORM_DOS
    if (battle_setup_count > 1)
        show_battle_setup_box();
#endif
}

// Render the visible battle terrain rows, honoring dirty-cell updates, clipped edges, and virtual tiles.
// FUNCTION: C2 0x3bbeb
// FUNCTION: C2WIN 0x0041dd0b
void show_battlemap_base(void)
{
    int col_idx;
    int row_idx;
    unsigned char dirty_flags;

    sprite_y   = pm_screen_y_start;
    sprite_x   = pm_screen_x_start;
    pm_shown_y = pm_y;
    pm_y_clip  = 0;

    /* Render the upper clipped row. */
    for (col_idx = 0, pm_shown_x = pm_x;
         col_idx < pm_screen_width; col_idx++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (update_map == 0) {
            if (((pm_shown_ptr) >= 0x0FFF0000)) {
                sprite_image_no = ((pm_shown_ptr) - 0x0FFF0000);
                if (sprite_image_no >= 7) place_diamond(2);
                sprite_x += pm_diamond_width;
                continue;
            }
            dirty_flags = (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).dirty;
            (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).dirty &= 0xf0;
            if (dirty_flags == 0) {
                sprite_x += pm_diamond_width;
                continue;
            }
            if ((dirty_flags & 3) > 1) (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).dirty |= 1;
            if ((dirty_flags & 0xc) != 0) {
                dirty_flags &= 0xc;
                if      (dirty_flags == 4) sprite_image_no = 0xf;
                else if (dirty_flags == 8) sprite_image_no = 0xd;
                else                sprite_image_no = 0xe;
                place_diamond(2);
                refresh_a_square(sprite_x >> 4, sprite_y >> 4, 2);
                sprite_x += pm_diamond_width;
                continue;
            }
        }
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_image_no = ((pm_shown_ptr) - 0x0FFF0000);
            place_diamond(2);
            sprite_x += pm_diamond_width;
            continue;
        } else {
            (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).dirty &= 0xf0;
            sprite_image_no = (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).terrain;
            sprite_image_no += 0x10;
            place_diamond(2);
        }
        sprite_x += pm_diamond_width;
    }
    sprite_y += pm_diamond_half_height;
    pm_shown_y++;

    /* Render alternating interior row layouts. */
    mid3_line_with_sides_base();
    for (row_idx = 0; row_idx < (pm_screen_height - 2) / 2; row_idx++) {
        mid3_line_no_sides_base();
        mid3_line_with_sides_base();
    }

    /* Render the lower clipped row. */
    sprite_x   = pm_screen_x_start;
    for (col_idx = 0, pm_shown_x = pm_x; col_idx < pm_screen_width; col_idx++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (update_map == 0) {
            if (((pm_shown_ptr) >= 0x0FFF0000)) {
                sprite_image_no = ((pm_shown_ptr) - 0x0FFF0000);
                if (sprite_image_no >= 7) place_diamond(1);
                sprite_x += pm_diamond_width;
                continue;
            }
            dirty_flags = (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).dirty;
            (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).dirty &= 0xf0;
            if (dirty_flags == 0) {
                sprite_x += pm_diamond_width;
                continue;
            }
            if ((dirty_flags & 3) > 1) (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).dirty |= 1;
            if ((dirty_flags & 0xc) != 0) {
                dirty_flags &= 0xc;
                if (dirty_flags == 4) sprite_image_no = 0xf;
                else if (dirty_flags == 8) sprite_image_no = 0xd;
                else sprite_image_no = 0xe;
                place_diamond(1);
                refresh_a_square(sprite_x >> 4, sprite_y >> 4, 2);
                sprite_x += pm_diamond_width;
                continue;
            }
        }
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_image_no = ((pm_shown_ptr) - 0x0FFF0000);
            place_diamond(1);
            sprite_x += pm_diamond_width;
            continue;
        } else {
            (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).dirty &= 0xf0;
            sprite_image_no = (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).terrain;
            sprite_image_no += 0x10;
            place_diamond(1);
        }
        sprite_x += pm_diamond_width;
    }
}

// Render the figure and arrow overlays for the visible battle-map rows.
// FUNCTION: C2 0x3bf3c
// FUNCTION: C2WIN 0x0041e1e0
void show_battlemap_top(void)
{
    int col_idx;
    int row_idx;

    sprite_y   = pm_screen_y_start;
    sprite_x   = pm_screen_x_start;
    pm_shown_y = pm_y;
    pm_y_clip  = 0;
    pm_shown_x = pm_x;
    for (col_idx = 0; col_idx < pm_screen_width; col_idx++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_x += pm_diamond_width;
        } else {
            sprite_x += pm_diamond_width;
            place3_sprite(0);
        }
    }
    sprite_y += pm_diamond_half_height;
    pm_shown_y++;

    mid3_line_with_sides_top();
    for (row_idx = 0; row_idx < (pm_screen_height - 2) / 2; row_idx++) {
        mid3_line_no_sides_top();
        mid3_line_with_sides_top();
    }

    /* Render the lower overlay row and its two following edge rows. */
    sprite_x   = pm_screen_x_start;
    col_idx = 0;
    pm_shown_x = pm_x;
    for (; col_idx < pm_screen_width; col_idx++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_x += pm_diamond_width;
        } else {
            sprite_x += pm_diamond_width;
            place3_sprite(0);
        }
    }
    pm_shown_y++;
    sprite_y  += pm_diamond_half_height;
    pm_y_clip  = 0;

    bottom3_line_with_sides();
    bottom3_line_no_sides();
}

// Render one interior terrain row with full diamonds and optional debug labels.
// FUNCTION: C2 0x3c0af
// FUNCTION: C2WIN 0x0041e3ca
void mid3_line_no_sides_base(void)
{
    int col_idx;
    unsigned char dirty_flags;

    sprite_x = pm_screen_x_start;
    for (col_idx = 0, pm_shown_x = pm_x; col_idx < pm_screen_width; col_idx++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (update_map == 0) {
            if (((pm_shown_ptr) >= 0x0FFF0000)) {
                sprite_image_no = ((pm_shown_ptr) - 0x0FFF0000);
                if (sprite_image_no >= 7) place_diamond(0);
                sprite_x += pm_diamond_width; continue;
            }
            dirty_flags = (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).dirty;
            (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).dirty &= 0xf0;
            if (dirty_flags == 0) {
                sprite_x += pm_diamond_width;
                continue;
            }
            if ((dirty_flags & 3) > 1) (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).dirty |= 1;
            if ((dirty_flags & 0xc) != 0) {
                dirty_flags &= 0xc;
                if      (dirty_flags == 4) sprite_image_no = 0xf;
                else if (dirty_flags == 8) sprite_image_no = 0xd;
                else                sprite_image_no = 0xe;
                place_diamond(0);
                refresh_a_square(sprite_x >> 4, sprite_y >> 4, 2);
                sprite_x += pm_diamond_width;
                continue;
            }
        }
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_image_no = ((pm_shown_ptr) - 0x0FFF0000);
            place_diamond(0);
        } else {
            (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).dirty &= 0xf0;
            sprite_image_no = (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).terrain;
            sprite_image_no += 0x10;
            place_diamond(0);
        }
        sprite_x += pm_diamond_width;
        print3_test_info();
    }
    sprite_y  += pm_diamond_half_height;
    pm_shown_y++;
    pm_y_clip += pm_diamond_half_height;
}

// Render one terrain row with half-diamond clipping at the left and right map edges.
// FUNCTION: C2 0x3c244
// FUNCTION: C2WIN 0x0041e622
void mid3_line_with_sides_base(void)
{
    int col_idx;
    unsigned char dirty_flags;

    pm_shown_x = pm_x;
    sprite_x   = pm_screen_x_start;

    /* Render the clipped left edge cell. */
    pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
    if (update_map == 0) {
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_image_no = ((pm_shown_ptr) - 0x0FFF0000);
            if (sprite_image_no >= 7) place_lefthalf_diamond();
        } else {
            dirty_flags = (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).dirty;
            (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).dirty = (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).dirty & 0xf0;
            if (dirty_flags != 0) {
                if ((dirty_flags & 3) > 1) (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).dirty |= 1;
                if ((dirty_flags & 0xc) != 0) {
                    dirty_flags &= 0xc;
                    if      (dirty_flags == 4) sprite_image_no = 0xf;
                    else if (dirty_flags == 8) sprite_image_no = 0xd;
                    else                sprite_image_no = 0xe;
                    place_lefthalf_diamond();
                    refresh_a_square(sprite_x >> 4, sprite_y >> 4, 2);
                } else {
terrain_left:
                    if (((pm_shown_ptr) >= 0x0FFF0000)) {
                        sprite_image_no = ((pm_shown_ptr) - 0x0FFF0000);
                        place_lefthalf_diamond();
                    } else {
                        (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).dirty &= 0xf0;
                        sprite_image_no = (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).terrain;
                        sprite_image_no += 0x10;
                        place_lefthalf_diamond();
                    }
                }
            }
        }
    } else goto terrain_left;
    sprite_x += pm_diamond_half_width;

    /* Render the full interior cells. */
    for (col_idx = 0; col_idx < pm_screen_width - 1; col_idx++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (update_map == 0) {
            if (((pm_shown_ptr) >= 0x0FFF0000)) {
                sprite_image_no = ((pm_shown_ptr) - 0x0FFF0000);
                if (sprite_image_no >= 7) place_diamond(0);
                sprite_x += pm_diamond_width; continue;
            }
            dirty_flags = (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).dirty;
            (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).dirty = (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).dirty & 0xf0;
            if (dirty_flags == 0) {
                sprite_x += pm_diamond_width;
                continue;
            }
            if ((dirty_flags & 3) > 1) (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).dirty |= 1;
            if ((dirty_flags & 0xc) != 0) {
                dirty_flags &= 0xc;
                if      (dirty_flags == 4) sprite_image_no = 0xf;
                else if (dirty_flags == 8) sprite_image_no = 0xd;
                else                sprite_image_no = 0xe;
                place_diamond(0);
                refresh_a_square(sprite_x >> 4, sprite_y >> 4, 2);
                sprite_x += pm_diamond_width;
                continue;
            }
        }
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_image_no = ((pm_shown_ptr) - 0x0FFF0000);
            place_diamond(0);
        } else {
            (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).dirty &= 0xf0;
            sprite_image_no = (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).terrain;
            sprite_image_no += 0x10;
            place_diamond(0);
        }
        sprite_x += pm_diamond_width;
        print3_test_info();
    }

    /* Render the clipped right edge cell. */
    pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
    if (update_map == 0) {
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_image_no = ((pm_shown_ptr) - 0x0FFF0000);
            if (sprite_image_no >= 7) place_righthalf_diamond();
            goto mid_done;
        }
        dirty_flags = (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).dirty;
        (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).dirty &= 0xf0;
        if (dirty_flags == 0) goto mid_done;
        if ((dirty_flags & 3) > 1) (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).dirty |= 1;
        if ((dirty_flags & 0xc) != 0) {
            dirty_flags &= 0xc;
            if      (dirty_flags == 4) sprite_image_no = 0xf;
            else if (dirty_flags == 8) sprite_image_no = 0xd;
            else                sprite_image_no = 0xe;
            place_righthalf_diamond();
            refresh_a_square(sprite_x >> 4, sprite_y >> 4, 2);
            goto mid_done;
        }
    }
    if (((pm_shown_ptr) >= 0x0FFF0000)) {
        sprite_image_no = ((pm_shown_ptr) - 0x0FFF0000);
        place_righthalf_diamond();
    } else {
        (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).dirty &= 0xf0;
        sprite_image_no = (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).terrain;
        sprite_image_no += 0x10;
        place_righthalf_diamond();
    }

mid_done:
    sprite_y  += pm_diamond_half_height;
    pm_shown_y++;
    pm_y_clip += pm_diamond_half_height;
}

// Render an overlay row plus any visible neighboring cells beyond its left and right edges.
// FUNCTION: C2 0x3c61e
// FUNCTION: C2WIN 0x0041ebe0
void mid3_line_no_sides_top(void)
{
    int col_idx;

    if (pm_x > 0) {
        sprite_x = pm_screen_x_start - pm_diamond_width;
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_x - 1];
        if (!((pm_shown_ptr) >= 0x0FFF0000)) place3_sprite(2);
    }
    sprite_x = pm_screen_x_start;
    col_idx = 0;
    pm_shown_x = pm_x;
    for (; col_idx < pm_screen_width; col_idx++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_x += pm_diamond_width;
        } else {
            sprite_x += pm_diamond_width;
            place3_sprite(0);
        }
    }

    if (pm_shown_x < 80) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x];
        if (!((pm_shown_ptr) >= 0x0FFF0000)) place3_sprite(2);
    }

    sprite_y += pm_diamond_half_height;
    pm_shown_y++;
    pm_y_clip += pm_diamond_half_height;
}

// Render an overlay row whose first and last cells are clipped to the visible half-diamonds.
// FUNCTION: C2 0x3c733
// FUNCTION: C2WIN 0x0041ed78
void mid3_line_with_sides_top(void)
{
    int col_idx;

    pm_shown_x = pm_x;
    sprite_x   = pm_screen_x_start;

    pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
    if (!((pm_shown_ptr) >= 0x0FFF0000)) place3_sprite(1);
    sprite_x += pm_diamond_half_width;

    for (col_idx = 0; col_idx < pm_screen_width - 1; col_idx++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_x += pm_diamond_width;
        } else {
            sprite_x += pm_diamond_width;
            place3_sprite(0);
        }
    }

    pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
    if (!((pm_shown_ptr) >= 0x0FFF0000)) place3_sprite(2);

    sprite_y  += pm_diamond_half_height;
    pm_shown_y++;
    pm_y_clip += pm_diamond_half_height;
}

// Render a bounded lower overlay row with clipped first and last cells.
// FUNCTION: C2 0x3c846
// FUNCTION: C2WIN 0x0041ef08
void bottom3_line_with_sides(void)
{
    int col_idx;

    if (pm_shown_y >= PM_H) return;

    pm_shown_x = pm_x;
    sprite_x   = pm_screen_x_start;

    pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
    if (!((pm_shown_ptr) >= 0x0FFF0000)) place3_sprite(1);
    sprite_x += pm_diamond_half_width;

    for (col_idx = 0; col_idx < pm_screen_width - 1; col_idx++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_x += pm_diamond_width;
        } else {
            sprite_x += pm_diamond_width;
            place3_sprite(0);
        }
    }

    pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
    if (!((pm_shown_ptr) >= 0x0FFF0000)) place3_sprite(2);

    sprite_y  += pm_diamond_half_height;
    pm_shown_y++;
    pm_y_clip += pm_diamond_half_height;
}

// Render a bounded lower overlay row plus any visible neighboring edge cells.
// FUNCTION: C2 0x3c960
// FUNCTION: C2WIN 0x0041f098
void bottom3_line_no_sides(void)
{
    int col_idx;

    if (pm_shown_y >= PM_H) return;

    if (pm_x > 0) {
        sprite_x = pm_screen_x_start - pm_diamond_width;
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_x - 1];
        if (!((pm_shown_ptr) >= 0x0FFF0000)) place3_sprite(2);
    }
    sprite_x = pm_screen_x_start;
    col_idx = 0;
    pm_shown_x = pm_x;
    for (; col_idx < pm_screen_width; col_idx++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_x += pm_diamond_width;
        } else {
            sprite_x += pm_diamond_width;
            place3_sprite(0);
        }
    }

    if (pm_shown_x < 80) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x];
        if (!((pm_shown_ptr) >= 0x0FFF0000)) place3_sprite(2);
    }

    sprite_y += pm_diamond_half_height;
    pm_shown_y++;
    pm_y_clip += pm_diamond_half_height;
}

// Render a cell's figure, elephant riders, and linked arrow sprites with directional offsets
// and screen clipping.
// FUNCTION: C2 0x3ca7f
// FUNCTION: C2WIN 0x0041f231
void place3_sprite(int edge_style)
{
    int base_x_offset;
    int base_y_offset;
    int direction_idx;
    int x_offset;
    int y_offset;
    unsigned char *sprite_header_ptr;
    unsigned char *sprite_data_ptr;
    int rider_idx;
    int elephant_anim_idx;
    int arrow_subcell_x;
    int arrow_subcell_y;
    int direction_quadrant;

    figure_a = (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).figure;
    arrow_a  = (*(struct battle_cell *)((unsigned char *)battle_map + ((pm_shown_ptr)))).arrow;

    if (figure_a != 0) {
        direction_idx = figure_list[figure_a].direction - map_direction;
        if (direction_idx < 0) direction_idx += 8;
        if (zoom_level == 1) {
            x_offset = fig_walking_x_ofsets_z1[direction_idx * 8 + figure_list[figure_a].wf_step_x];
            y_offset = fig_walking_y_ofsets_z1[direction_idx * 8 + figure_list[figure_a].wf_step_x];
        } else {
            x_offset = fig_walking_x_ofsets_z2[direction_idx * 8 + figure_list[figure_a].wf_step_x];
            y_offset = fig_walking_y_ofsets_z2[direction_idx * 8 + figure_list[figure_a].wf_step_x];
        }
        if      (edge_style == 1) x_offset -= 2;
        else if (edge_style == 2) x_offset += pm_diamond_half_width;
        else                 x_offset += pm_diamond_half_width - pm_diamond_width;
        y_offset += pm_diamond_half_height;
        base_x_offset = x_offset; base_y_offset = y_offset;

        if (figure_list[figure_a].sprite_dir != 0) sprite_data_ptr = figure_list[figure_a].sprite_data_ptr;
        else sprite_data_ptr = figure_list[figure_a].arrow_data_ptr;
        sprite_image_no = figure_list[figure_a].sprite_anim;
        data_ptr        = sprite_image_no * 0x10 + 8;
        sprite_header_ptr             = sprite_data_ptr + data_ptr; sprite_start = sprite_header_ptr[4] + (sprite_header_ptr[5] << 8) + (sprite_header_ptr[6] << 16);
        sprite_width    = sprite_header_ptr[0] + (sprite_header_ptr[1] << 8);
        sprite_height   = sprite_header_ptr[2] + (sprite_header_ptr[3] << 8);
        if (sprite_start > 0x4baf0) { sprite_error++; return; }
        if (sprite_width <= 0)      { sprite_error++; return; }
        if (sprite_width > 0x12c)   { sprite_error++; return; }
        if (sprite_height <= 0)     { sprite_error++; return; }
        if (sprite_height > 0x12c)  { sprite_error++; return; }
        sprite_x_off = (signed char)sprite_header_ptr[0xe];
        sprite_y_off = (signed char)sprite_header_ptr[0xd];
        x_offset = x_offset - sprite_x_off;
        y_offset = y_offset - sprite_y_off;
        old_sprite_x = sprite_x; old_sprite_y = sprite_y;
        sprite_x += x_offset;
        sprite_y += y_offset;
        if (figure_list[figure_a].fight_state == 2) {
            if (zoom_level == 1) { sprite_x -= 0x18; sprite_y -= 0x40; }
            else                 { sprite_x -= 0xc;  sprite_y -= 0x20; }
            refresh_figure3_square((sprite_x - 4) >> 4, sprite_y >> 4);
        } else if (figure_list[figure_a].fight_state != 0) {
            if (zoom_level == 1) { sprite_x -= 0x14; sprite_y -= 0x2a; }
            else                 { sprite_x -= 0xa;  sprite_y -= 0x14; }
            refresh_figure2_square((sprite_x - 4) >> 4, sprite_y >> 4);
        } else {
            if (zoom_level == 1) { sprite_x -= 0xa;  sprite_y -= 0x20; }
            else                 { sprite_x -= 4;    sprite_y -= 0x10; }
            refresh_figure_square((sprite_x - 0x14) >> 4, sprite_y >> 4);
        }
        xclip(pm_screen_x_start, 0x280);
        yclip(0x18, 0x168);
        if (yclipped != 5) {
            if      (xclipped == 1) write_i_left_sprite(sprite_data_ptr);
            else if (xclipped == 2) write_i_right_sprite(sprite_data_ptr);
            else                    write_i_sprite(sprite_data_ptr);
        }
        sprite_x = old_sprite_x; sprite_y = old_sprite_y;

        if (figure_list[figure_a].fight_state == 2) {
            for (rider_idx = 1; rider_idx >= 0; rider_idx--) {
                if (rider_idx == 1) sprite_image_no = figure_list[figure_a].archer_image_a;
                else sprite_image_no = figure_list[figure_a].archer_image_b;
                data_ptr      = sprite_image_no * 0x10 + 8;
                sprite_header_ptr           = sprite_data_ptr + data_ptr; sprite_start = sprite_header_ptr[4] + (sprite_header_ptr[5] << 8) + (sprite_header_ptr[6] << 16);
                sprite_width  = sprite_header_ptr[0] + (sprite_header_ptr[1] << 8);
                sprite_height = sprite_header_ptr[2] + (sprite_header_ptr[3] << 8);
                if (sprite_start > 0x4baf0) { sprite_error++; return; }
                if (sprite_width <= 0)      { sprite_error++; return; }
                if (sprite_width > 0x12c)   { sprite_error++; return; }
                if (sprite_height <= 0)     { sprite_error++; return; }
                if (sprite_height > 0x12c)  { sprite_error++; return; }
                sprite_x_off = (signed char)sprite_header_ptr[0xe];
                sprite_y_off = (signed char)sprite_header_ptr[0xd];
                x_offset = base_x_offset - sprite_x_off;
                y_offset = base_y_offset - sprite_y_off;
                old_sprite_x = sprite_x; old_sprite_y = sprite_y;
                sprite_x += x_offset;
                sprite_y += y_offset;
                sprite_x -= 0x18;
                sprite_y -= 0x40;
                elephant_anim_idx = figure_list[figure_a].sprite_anim; sprite_x += elephant_riders[elephant_anim_idx * 2];
                sprite_y += elephant_riders[elephant_anim_idx * 2 + 1];
                sprite_x += rider_idx * 6;
                sprite_y -= rider_idx * 6;
                if (rider_idx <= 0) sprite_height -= 8;
                refresh_figure_square((sprite_x - 4) >> 4, sprite_y >> 4);
                xclip(pm_screen_x_start, 0x280);
                yclip(0x18, 0x168);
                if (yclipped != 5) {
                    if      (xclipped == 1) write_i_left_sprite(sprite_data_ptr);
                    else if (xclipped == 2) write_i_right_sprite(sprite_data_ptr);
                    else                    write_i_sprite(sprite_data_ptr);
                }
                sprite_x = old_sprite_x; sprite_y = old_sprite_y;
            }
        }
    }

    if (arrow_a != 0) {
        do {
            direction_idx = (unsigned char)arrow_list[arrow_a].heading - map_direction;
            if (direction_idx < 0) direction_idx += 8;
            arrow_subcell_x = arrow_list[arrow_a].start_x % 7; arrow_subcell_y = arrow_list[arrow_a].start_y % 7;
            if (zoom_level == 1) {
                direction_quadrant = map_direction / 2; x_offset = arrow_xr_x_ofset[arrow_subcell_x + 7 * direction_quadrant];
                x_offset += arrow_yr_x_ofset[arrow_subcell_y + 7 * direction_quadrant];
                y_offset = arrow_xr_y_ofset[arrow_subcell_x + 7 * direction_quadrant];
                y_offset += arrow_yr_y_ofset[arrow_subcell_y + 7 * direction_quadrant];
            }
            if      (edge_style == 1) x_offset -= 2;
            else if (edge_style == 2) x_offset += pm_diamond_half_width;
            else                 x_offset += pm_diamond_half_width - pm_diamond_width;
            y_offset += pm_diamond_half_height;

            sprite_data_ptr = arrow_list[arrow_a].arrow_data_ptr;
            if (sprite_data_ptr == 0) return;
            sprite_image_no = arrow_list[arrow_a].sprite_anim;
            data_ptr        = sprite_image_no * 0x10 + 8;
            sprite_header_ptr             = sprite_data_ptr + data_ptr; sprite_start = sprite_header_ptr[4] + (sprite_header_ptr[5] << 8) + (sprite_header_ptr[6] << 16);
            sprite_width    = (sprite_header_ptr[0]) + (sprite_header_ptr[1] << 8);
            sprite_height   = sprite_header_ptr[2] + (sprite_header_ptr[3] << 8);
            if (sprite_start > 0x4baf0) { sprite_error++; return; }
            if (sprite_width <= 0)      { sprite_error++; return; }
            if (sprite_width > 0x12c)   { sprite_error++; return; }
            if (sprite_height <= 0)     { sprite_error++; return; }
            if (sprite_height > 0x12c)  { sprite_error++; return; }
            old_sprite_x = sprite_x; old_sprite_y = sprite_y;
            sprite_x += x_offset;
            sprite_y += y_offset;
            sprite_x -= sprite_width >> 1;
            sprite_y -= sprite_height;
            sprite_y -= (unsigned char)arrow_list[arrow_a].anim_count / 2 + 0x20;
            refresh_figure_square((sprite_x - 4) >> 4, sprite_y >> 4);
            xclip(pm_screen_x_start, 0x280);
            yclip(0x18, 0x168);
            if (yclipped != 5) {
                if      (xclipped == 1) write_i_left_sprite(sprite_data_ptr);
                else if (xclipped == 2) write_i_right_sprite(sprite_data_ptr);
                else                    write_i_sprite(sprite_data_ptr);
            }
            sprite_x = old_sprite_x; sprite_y = old_sprite_y;
            arrow_a = (unsigned char)arrow_list[arrow_a].flight_done;
        } while (arrow_a != 0);
    }
}

// Print the cell's figure state or signed arrow value when the corresponding debug mode is active.
// FUNCTION: C2 0x3d2d5
// FUNCTION: C2WIN 0x0041fe23
void print3_test_info(void)
{
    int v;
    int col;
    int fig;

    if (test_mode1 != 0) {
        if (((pm_shown_ptr) >= 0x0FFF0000)) return;
        old_sprite_x = sprite_x;
        old_sprite_y = sprite_y;
        v = ((unsigned char *)battle_map)[(pm_shown_ptr) + 1];
        if (v != 0) v = figure_list[v].state_idx;
        else v = 0;
        col = 0x3f;
        font_no(v, 0x20, " ",
                sprite_x + 0x14 - pm_diamond_width, sprite_y + 0xa,
                font1, col);
        sprite_x = old_sprite_x;
        sprite_y = old_sprite_y;
    } else if (test_mode2 != 0) {
        if (((pm_shown_ptr) >= 0x0FFF0000)) return;
        old_sprite_x = sprite_x;
        old_sprite_y = sprite_y;
        v = (signed char)((unsigned char *)battle_map)[(pm_shown_ptr) + 3];
        col = 0x3f;
        font_no(v, 0x20, " ",
                sprite_x + 0x14 - pm_diamond_width, sprite_y + 0xa,
                font1, col);
        sprite_x = old_sprite_x;
        sprite_y = old_sprite_y;
    }
}
