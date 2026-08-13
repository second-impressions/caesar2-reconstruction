
#include "pm_map1.h"
#include "c2_data.h"

struct byte_delta_rec fire_offs[16] = {
    { 0, 0 },
    { 10, -3 },
    { 21, -2 },
    { 31, 3 },
    { 44, 0 },
    { 34, -5 },
    { 16, 2 },
    { 37, -3 },
    { 28, -8 },
    { 39, -3 },
    { 22, -12 },
    { 18, -6 },
    { 36, -2 },
    { 8, -3 },
    { 37, 1 },
    { 10, 2 }
};

int plague_offs[4] = { -49675264, 52100629, -31981554, -49872368 };

int aquaduct_tops[10] = { 56, 57, 58, 53, 54, 55, 25, 25, 25, 25 };

int aquaduct_tops2[10] = { 53, 54, 55, 56, 57, 58, 25, 25, 25, 25 };

struct int_delta_rec arena_top_data[3][4] = {
    { { 0, 4 },  { 61, -27 }, { 1, -57 },  { -59, -27 } },
    { { 4, 1 },  { 31, -13 }, { 4, -27 },  { -24, -13 } },
    { { 2, 1 },  { 13, -5 },  { 2, -12 },  { -11, -5 } }
};

struct int_delta_rec colos_top_data[3][4] = {
    { { -2, -18 }, { 59, -48 }, { -1, -78 }, { -61, -48 } },
    { { 0, -10 },  { 27, -24 }, { 0, -38 },  { -28, -24 } },
    { { -2, -2 },  { 11, -9 },  { 0, -14 },  { -11, -9 } }
};

int city_anim64;
int city_anim128;
int city_anim32;
int city_anim8;
int city_anim16;
int overlay0_empty_mode;
int cmu_count[5];


extern void font_no(int value, char pad_char, char *suffix, int x, int y, unsigned char *font, int color);
extern void set_this_ambient(int ambient_idx);
extern void place_diamond(int);
extern void place_lefthalf_diamond(void);
extern void place_righthalf_diamond(void);
#if PLATFORM_WINDOWS
extern unsigned char game_paused;
#endif
/* Forward declarations (functions defined later in this file). */
void show_citymap_base(void);
void show_citymap_sprites(void);
void show_citymap_top(void);
void sprites_no_sides(void);
void sprites_with_sides(void);
void mid_line_no_sides_base(void);
void mid_line_with_sides_base(void);
void place_a_building_base(int edge_style);
void place_sprite(int edge_side);


// Render the city map for the current frame, updating animation and dirty-map state.
// FUNCTION: C2 0x364f5
// FUNCTION: C2WIN 0x0045b1e0
void show_citymap(void)
{
    sprite_error = 0;
#if PLATFORM_WINDOWS
    if (game_paused == 0) {
#endif
    cmu_count[0]++;
    if (cmu_count[0] > 3) {
        cmu_count[0] = 0;
    }

    city_anim128++;
    if (city_anim128 >= 0x80) {
        city_anim128 = 0;
    }
    city_anim64 = city_anim128 >> 1;
#if PLATFORM_WINDOWS
    city_anim32 = city_anim64 >> 1;
    city_anim16 = city_anim32 >> 1;
    city_anim8  = city_anim16 >> 1;
    }
#else
    city_anim32 = city_anim128 >> 2;
    city_anim16 = city_anim128 >> 3;
    city_anim8  = city_anim128 >> 4;
#endif

    if (ov_map_mode == 0 || ov_map_mode == 1 ||
        ov_map_mode == 4 || ov_map_mode == 8) {
        overlay0_empty_mode = 1;
    } else {
        overlay0_empty_mode = 0;
    }

    if (update_landfill != 0) {
        update_map = 1;
    }
    show_citymap_base();
    if (overlays_on != 1) {
        show_citymap_sprites();
    }
    show_citymap_top();
    if (update_map != 0) {
        update_map--;
    }
}

// Draw the terrain and building-base layer across the visible pseudo-map.
// FUNCTION: C2 0x365da
// FUNCTION: C2WIN 0x0045b314
void show_citymap_base(void)
{
    int ptr;
    int x;
    int i;
    int j;

    sprite_y    = pm_screen_y_start;
    sprite_x    = pm_screen_x_start;
    pm_shown_y  = pm_y;
    pm_y_clip   = 0;
    C2_CHECK_PM_ROW();
    i = 0;
    pm_shown_x  = pm_x;

    /* top edge — style 2 */
    for (; i < pm_screen_width; i++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_image_no = ((pm_shown_ptr) - 0x0FFF0000);
            place_diamond(2);
            sprite_x += pm_diamond_width;
        } else if (show_overlay(2)) {
            /* overlay handled this cell */
        } else if ((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind >= 0x78) {
            place_a_building_base(2);
        } else {
            if (((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits & 1) != 0) {
                (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits &= 0xfe;
#if PLATFORM_DOS
                refresh_a_square(sprite_x >> 4, sprite_y >> 4, 2);
#endif
            }
            sprite_image_no = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind;
            sprite_image_no = rotated_map[sprite_image_no].dir[map_direction >> 1];
            sprite_image_no += 0x10;
            place_diamond(2);
            sprite_x += pm_diamond_width;
        }
        if (!((pm_shown_ptr) >= 0x0FFF0000))
            (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits |= 2;
    }
    sprite_y += pm_diamond_half_height;
    pm_shown_y++;

    C2_CHECK_PM_ROW();
    /* interior */
    mid_line_with_sides_base();
    for (j = 0; j < (pm_screen_height - 2) / 2; j++) {
        mid_line_no_sides_base();
        mid_line_with_sides_base();
    }

    C2_CHECK_PM_ROW();
    /* bottom edge — style 1 */
    sprite_x   = pm_screen_x_start;
    i = 0;
    pm_shown_x = pm_x;
    for (; i < pm_screen_width; i++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_image_no = ((pm_shown_ptr) - 0x0FFF0000);
            place_diamond(1);
            sprite_x += pm_diamond_width;
        } else if (show_overlay(1)) {
            /* overlay handled */
        } else if ((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind >= 0x78) {
            place_a_building_base(1);
        } else {
            if (((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits & 1) != 0) {
                (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits &= 0xfe;
#if PLATFORM_DOS
                refresh_a_square(sprite_x >> 4, sprite_y >> 4, 2);
#endif
            }
            sprite_image_no = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind;
            sprite_image_no = rotated_map[sprite_image_no].dir[map_direction >> 1];
            sprite_image_no += 0x10;
            place_diamond(1);
            sprite_x += pm_diamond_width;
        }
        if (!((pm_shown_ptr) >= 0x0FFF0000))
            (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits |= 2;
    }
}

// Draw citizens and other cell sprites across the visible pseudo-map.
// FUNCTION: C2 0x3689e
// FUNCTION: C2WIN 0x0045b6bc
void show_citymap_sprites(void)
{
    int ptr;
    int x;
    int i;
    int j;

    sprite_y    = pm_screen_y_start;
    sprite_x    = pm_screen_x_start;
    pm_shown_y  = pm_y;
    pm_y_clip   = 0;
    C2_CHECK_PM_ROW();
    i = 0;
    pm_shown_x  = pm_x;

    for (; i < pm_screen_width; i++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (!((pm_shown_ptr) >= 0x0FFF0000))
            place_sprite(0);
        sprite_x += pm_diamond_width;
    }
    sprite_y += pm_diamond_half_height;
    pm_shown_y++;

    C2_CHECK_PM_ROW();
    sprites_with_sides();
    for (j = 0; j < (pm_screen_height - 2) / 2; j++) {
        sprites_no_sides();
        sprites_with_sides();
    }

    sprite_x   = pm_screen_x_start;
    C2_CHECK_PM_ROW();
    i = 0;
    pm_shown_x = pm_x;
    for (; i < pm_screen_width; i++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (!((pm_shown_ptr) >= 0x0FFF0000))
            place_sprite(0);
        sprite_x += pm_diamond_width;
    }
}

extern void write_i_sprite(unsigned char *sprite_addr);
extern void write_i_left_sprite(unsigned char *sprite_addr);
extern void write_i_right_sprite(unsigned char *sprite_addr);
void mid_line_no_sides_top(void);
void mid_line_with_sides_top(void);
void bottom_line_with_sides(void);
void bottom_line_no_sides(void);
void place_a_building_top(int edge_style);
void top_it(int edge_side);
void print_test_info(void);

// Draw building tops and overhead effects above the city-map base and sprite layers.
// FUNCTION: C2 0x369ca
// FUNCTION: C2WIN 0x0045b874
void show_citymap_top(void)
{
    int ptr;
    int x;
    int i;
    int j;
#if PLATFORM_DOS
    int k;
#endif

#if PLATFORM_DOS
    if (zoom_level == 1) for (k = 0x18; k < 0x1bc; k++) show_internal_4point(0, k, 0);
#endif

    sprite_y    = pm_screen_y_start;
    sprite_x    = pm_screen_x_start;
    pm_shown_y  = pm_y;
    pm_y_clip   = 0;
    C2_CHECK_PM_ROW();
    i = 0;
    pm_shown_x  = pm_x;

    /* top edge */
    for (; i < pm_screen_width; i++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];

        if (are_overlays_on() != 0) {
            sprite_x += pm_diamond_width;
            continue;
        }
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_x += pm_diamond_width;
            continue;
        } else if ((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind >= 0x78) {
            place_a_building_top(2);
        }
        if (((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits & 0x80) != 0) top_it(0);
        sprite_x += pm_diamond_width;
    }
    sprite_y += pm_diamond_half_height;
    pm_shown_y++;

    C2_CHECK_PM_ROW();
    /* interior */
    mid_line_with_sides_top();
    for (j = 0; j < (pm_screen_height - 2) / 2; j++) {
        mid_line_no_sides_top();
        mid_line_with_sides_top();
    }

    /* one more pre-bottom row */
    C2_CHECK_PM_ROW();
    sprite_x   = pm_screen_x_start;
    i = 0; pm_shown_x = pm_x;
    for (; i < pm_screen_width; i++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];

        if (are_overlays_on() != 0) {
            sprite_x += pm_diamond_width;
            continue;
        }
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_x += pm_diamond_width;
            continue;
        } else if ((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind >= 0x78) {
            place_a_building_top(1);
        }
        if (((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits & 0x80) != 0) top_it(0);
        sprite_x += pm_diamond_width;
    }
    pm_shown_y++;
    sprite_y += pm_diamond_half_height;
    pm_y_clip = 0;

    bottom_line_with_sides();
    bottom_line_no_sides();
    bottom_line_with_sides();
    bottom_line_no_sides();
}

void place_a_building_roof(int);

// Draw an interior sprite row, including neighboring cells whose sprites spill into view.
// FUNCTION: C2 0x36bfe
// FUNCTION: C2WIN 0x0045bb2b
void sprites_no_sides(void)
{
    int i;

    C2_CHECK_PM_ROW();
    /* left spillover */
    if (pm_x > 0) {
        sprite_x = pm_screen_x_start - pm_diamond_width;
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_x - 1];
        if (!((pm_shown_ptr) >= 0x0FFF0000))
            place_sprite(0);
    }

    sprite_x   = pm_screen_x_start;
    C2_CHECK_PM_ROW();
    i = 0;
    pm_shown_x = pm_x;
    for (; i < pm_screen_width; i++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (!((pm_shown_ptr) >= 0x0FFF0000))
            place_sprite(0);
        sprite_x += pm_diamond_width;
    }

    C2_CHECK_PM_ROW();
    /* right spillover */
    if (pm_shown_x < 0x50) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x];
        if (!((pm_shown_ptr) >= 0x0FFF0000))
            place_sprite(0);
    }

    sprite_y  += pm_diamond_half_height;
    pm_shown_y++;
    pm_y_clip += pm_diamond_half_height;
}

// Draw a sprite row with clipped cells along the visible side edges.
// FUNCTION: C2 0x36cfc
// FUNCTION: C2WIN 0x0045bcb4
void sprites_with_sides(void)
{
    int i;

    C2_CHECK_PM_ROW();
    pm_shown_x = pm_x;
    sprite_x = pm_screen_x_start;
    pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
    if (!((pm_shown_ptr) >= 0x0FFF0000)) place_sprite(1);
    sprite_x += pm_diamond_half_width;

    C2_CHECK_PM_ROW();
    for (i = 0; i < pm_screen_width - 1; i++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (!((pm_shown_ptr) >= 0x0FFF0000)) place_sprite(0);
        sprite_x += pm_diamond_width;
    }
    C2_CHECK_PM_ROW();
    pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
    if (!((pm_shown_ptr) >= 0x0FFF0000)) place_sprite(0);
    sprite_y += pm_diamond_half_height;
    pm_shown_y++;
    pm_y_clip += pm_diamond_half_height;
}

// Draw an unclipped interior row of terrain and building bases.
// FUNCTION: C2 0x36dfb
// FUNCTION: C2WIN 0x0045be2a
void mid_line_no_sides_base(void)
{
    int i;
#if PLATFORM_DOS
    int dir;
#endif

    C2_CHECK_PM_ROW();
    sprite_x = pm_screen_x_start;
    i = 0;
    pm_shown_x = pm_x;
    for (; i < pm_screen_width; i++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_image_no = ((pm_shown_ptr) - 0x0FFF0000);
            if (update_map != 0 || sprite_image_no >= 7) place_diamond(0);
            sprite_x += pm_diamond_width;
        } else if (show_overlay(0)) {
        } else if ((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind >= 0x78) {
            place_a_building_base(0);
        } else {
            if (((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits & 1) != 0) {
                (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits &= 0xfe;
#if C2_FEAT_TILE_REFRESH
                refresh_a_square(sprite_x >> 4, sprite_y >> 4, 2);
#endif
            }
            sprite_image_no = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind;
#if PLATFORM_DOS
            dir = map_direction >> 1;
            sprite_image_no = rotated_map[sprite_image_no].dir[dir];
#else
            sprite_image_no = rotated_map[sprite_image_no].dir[map_direction >> 1];
#endif
            sprite_image_no += 0x10;
            place_diamond(0);
            sprite_x += pm_diamond_width;
        }
    }
    sprite_y  += pm_diamond_half_height;
    pm_shown_y++;
    pm_y_clip += pm_diamond_half_height;
}

// Draw a base-layer row with half-diamonds clipped at the visible side edges.
// FUNCTION: C2 0x36f49
// FUNCTION: C2WIN 0x0045bfd3
void mid_line_with_sides_base(void)
{
    int i;
#if PLATFORM_DOS
    int dir;
#endif

    C2_CHECK_PM_ROW();
    pm_shown_x = pm_x;
    sprite_x   = pm_screen_x_start;

    /* leftmost half-diamond */
    pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
    if (((pm_shown_ptr) >= 0x0FFF0000)) {
        sprite_image_no = ((pm_shown_ptr) - 0x0FFF0000);
        if (update_map != 0 || sprite_image_no >= 7) place_lefthalf_diamond();
    } else if (show_left_overlay(2)) {
        /* overlay handled this cell */
    } else if ((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind >= 0x78) {
        place_a_building_base(3);
    } else {
        if (((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits & 1) != 0) {
            (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits &= 0xfe;
#if C2_FEAT_TILE_REFRESH
            refresh_a_square(sprite_x >> 4, sprite_y >> 4, 2);
#endif
        }
        sprite_image_no = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind;
#if PLATFORM_DOS
        dir = map_direction >> 1;
        sprite_image_no = rotated_map[sprite_image_no].dir[dir];
#else
        sprite_image_no = rotated_map[sprite_image_no].dir[map_direction >> 1];
#endif
        sprite_image_no += 0x10;
        place_lefthalf_diamond();
    }
    if (!((pm_shown_ptr) >= 0x0FFF0000))
        (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits |= 2;
    sprite_x += pm_diamond_half_width;

    C2_CHECK_PM_ROW();
    /* middle full diamonds */
    for (i = 0; i < pm_screen_width - 1; i++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_image_no = ((pm_shown_ptr) - 0x0FFF0000);
            if (update_map != 0 || sprite_image_no >= 7) place_diamond(0);
            sprite_x += pm_diamond_width;
        } else if (show_overlay(0)) {
        } else if ((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind >= 0x78) {
            place_a_building_base(0);
        } else {
            if (((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits & 1) != 0) {
                (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits &= 0xfe;
#if C2_FEAT_TILE_REFRESH
                refresh_a_square(sprite_x >> 4, sprite_y >> 4, 2);
#endif
            }
            sprite_image_no = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind;
#if PLATFORM_DOS
            dir = map_direction >> 1;
            sprite_image_no = rotated_map[sprite_image_no].dir[dir];
#else
            sprite_image_no = rotated_map[sprite_image_no].dir[map_direction >> 1];
#endif
            sprite_image_no += 0x10;
            place_diamond(0);
            sprite_x += pm_diamond_width;
        }
    }

    C2_CHECK_PM_ROW();
    /* rightmost half-diamond */
    pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
    if (((pm_shown_ptr) >= 0x0FFF0000)) {
        sprite_image_no = ((pm_shown_ptr) - 0x0FFF0000);
        if (update_map != 0 || sprite_image_no >= 7) place_righthalf_diamond();
    } else if (show_right_overlay(0)) {
        /* overlay handled this cell */
    } else if ((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind >= 0x78) {
        place_a_building_base(4);
    } else {
        if (((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits & 1) != 0) {
            (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits &= 0xfe;
#if C2_FEAT_TILE_REFRESH
            refresh_a_square(sprite_x >> 4, sprite_y >> 4, 2);
#endif
        }
        sprite_image_no = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind;
#if PLATFORM_DOS
        dir = map_direction >> 1;
        sprite_image_no = rotated_map[sprite_image_no].dir[dir];
#else
        sprite_image_no = rotated_map[sprite_image_no].dir[map_direction >> 1];
#endif
        sprite_image_no += 0x10;
        place_righthalf_diamond();
    }
    if (!((pm_shown_ptr) >= 0x0FFF0000))
        (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits |= 2;
    sprite_y  += pm_diamond_half_height;
    pm_shown_y++;
    pm_y_clip += pm_diamond_half_height;
}

extern void write_large_diamond_hat(unsigned char *, int);
extern void write_large_diamond_lefthat(unsigned char *, int);
extern void write_large_diamond_righthat(unsigned char *, int);
extern void write_large_diamond_lefthalfhat(unsigned char *, int, int);
extern void write_large_diamond_righthalfhat(unsigned char *, int, int);
extern void write_medium_diamond_hat(unsigned char *, int);
extern void write_medium_diamond_lefthat(unsigned char *, int);
extern void write_medium_diamond_righthat(unsigned char *, int);
extern void write_medium_diamond_lefthalfhat(unsigned char *, int, int);
extern void write_medium_diamond_righthalfhat(unsigned char *, int, int);
extern void write_small_diamond_hat(unsigned char *, int);
extern void write_small_diamond_lefthat(unsigned char *, int);
extern void write_small_diamond_righthat(unsigned char *, int);
extern void write_small_diamond_lefthalfhat(unsigned char *, int, int);
extern void write_small_diamond_righthalfhat(unsigned char *, int, int);

// Draw building tops and overhead effects across an unclipped interior row.
// FUNCTION: C2 0x372a9
// FUNCTION: C2WIN 0x0045c430
void mid_line_no_sides_top(void)
{
    int col_idx;
    int next_screen_x;
    int overlay_flag;

    sprite_x   = pm_screen_x_start;
    col_idx = 0;
    pm_shown_x = pm_x;
    for (; col_idx < pm_screen_width; col_idx++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        overlay_flag = are_overlays_on();
        next_screen_x   = sprite_x + pm_diamond_width;
        if (overlay_flag != 0) {
            sprite_x = next_screen_x;
        } else if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_x = next_screen_x;
        } else {
            if ((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind >= 0x78) place_a_building_top(0);
            if (((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits & 0x80) != 0) top_it(0);
            sprite_x += pm_diamond_width;
            print_test_info();
        }
    }
    sprite_y  += pm_diamond_half_height;
    pm_shown_y++;
    pm_y_clip += pm_diamond_half_height;
}

// Draw a top-layer row with building and effect sprites clipped at the side edges.
// FUNCTION: C2 0x3738b
// FUNCTION: C2WIN 0x0045c558
void mid_line_with_sides_top(void)
{
    int i;
#if PLATFORM_DOS
    int next_screen_x;
    int overlay_flag;
#endif

    C2_CHECK_PM_ROW();
    pm_shown_x = pm_x;
    sprite_x   = pm_screen_x_start;

    /* left edge */
    pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
    if (are_overlays_on() == 0) {
        if (!((pm_shown_ptr) >= 0x0FFF0000)) {
            if ((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind >= 0x78) place_a_building_top(3);
            if (((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits & 0x80) != 0) top_it(1);
        }
    }
    sprite_x += pm_diamond_half_width;

    C2_CHECK_PM_ROW();
    /* middle */
    for (i = 0; i < pm_screen_width - 1; i++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
#if PLATFORM_DOS
        overlay_flag = are_overlays_on();
        next_screen_x   = sprite_x + pm_diamond_width;
        if (overlay_flag != 0) {
            sprite_x = next_screen_x;
        } else if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_x = next_screen_x;
        } else {
            if ((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind >= 0x78) place_a_building_top(0);
#else
        if (are_overlays_on() != 0) {
            sprite_x += pm_diamond_width;
            continue;
        }
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_x += pm_diamond_width;
            continue;
        } else if ((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind >= 0x78) {
            place_a_building_top(0);
        }
#endif
            if (((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits & 0x80) != 0) top_it(0);
            sprite_x += pm_diamond_width;
#if PLATFORM_DOS
            print_test_info();
        }
#endif
    }

    C2_CHECK_PM_ROW();
    /* right edge */
    pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
    if (are_overlays_on() == 0) {
        if (!((pm_shown_ptr) >= 0x0FFF0000)) {
            if ((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind >= 0x78) place_a_building_top(4);
            if (((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits & 0x80) != 0) top_it(2);
        }
    }

    sprite_y  += pm_diamond_half_height;
    pm_shown_y++;
    pm_y_clip += pm_diamond_half_height;
}

// Draw building roof slices along the lower viewport edge, including clipped side cells.
// FUNCTION: C2 0x37556
// FUNCTION: C2WIN 0x0045c7bc
void bottom_line_with_sides(void)
{
    int col_idx;

    if (pm_shown_y >= PM_H) return;

    pm_shown_x = pm_x;
    sprite_x   = pm_screen_x_start;

    /* left edge */
    pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
    if (((pm_shown_ptr) >= 0x0FFF0000)) {
    } else if (are_overlays_on() != 0) {
    } else if ((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind >= 0x78) {
        place_a_building_roof(3);
    }
    sprite_x += pm_diamond_half_width;

    C2_CHECK_PM_ROW();
    /* middle */
    for (col_idx = 0; col_idx < pm_screen_width - 1; col_idx++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_x += pm_diamond_width;
        }
        else if (are_overlays_on() != 0) sprite_x += pm_diamond_width;
        else if ((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind >= 0x78) place_a_building_roof(0);
        else sprite_x += pm_diamond_width;
    }

    C2_CHECK_PM_ROW();
    /* right edge */
    pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
    if (((pm_shown_ptr) >= 0x0FFF0000)) {
    } else if (are_overlays_on() != 0) {
    } else if ((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind >= 0x78) {
        place_a_building_roof(4);
    }

    pm_shown_y++;
    pm_y_clip += pm_diamond_half_height;
}

// Draw building roof slices along an unclipped lower viewport row.
// FUNCTION: C2 0x376cf
// FUNCTION: C2WIN 0x0045c9c9
void bottom_line_no_sides(void)
{
    int i;
#if PLATFORM_DOS
    int next_screen_x;
    int overlay_flag;
#endif

    if (pm_shown_y >= PM_H) return;
    sprite_x   = pm_screen_x_start;
    i = 0;
    pm_shown_x = pm_x;
    for (; i < pm_screen_width; i++) {
        pm_shown_ptr = pseudo_map[pm_shown_y][pm_shown_x++];
        if (((pm_shown_ptr) >= 0x0FFF0000)) {
            sprite_x += pm_diamond_width;
        } else {
#if PLATFORM_DOS
            overlay_flag = are_overlays_on();
            next_screen_x   = sprite_x + pm_diamond_width;
            if (overlay_flag == 0 && (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind >= 0x78) {
                place_a_building_roof(0);
            } else {
                sprite_x = next_screen_x;
            }
#else
            if (are_overlays_on() != 0) {
                sprite_x += pm_diamond_width;
            } else if ((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind >= 0x78) {
                place_a_building_roof(0);
            } else {
                sprite_x += pm_diamond_width;
            }
#endif
        }
    }
    pm_shown_y++;
    pm_y_clip += pm_diamond_half_height;
}

extern void set_city_ambient(int kind);

// Select the current building's rotated sprite and draw its base at the requested edge style.
// FUNCTION: C2 0x3779d
// FUNCTION: C2WIN 0x0045cac7
void place_a_building_base(int edge_style)
{
#if PLATFORM_WINDOWS
    unsigned char flags;
    int mode_no;
    int width;
#else
    char bank_kind;
    int rotation_idx;
#endif
    unsigned char *sprite_bank_ptr;

#if PLATFORM_WINDOWS
    if (map_mode > 1) mode_no = 0;
    else mode_no = map_mode;
#endif

    sprite_image_no = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).extra_edge;
#if PLATFORM_WINDOWS
    flags = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits & 0x1c;

    if (flags == 0) {
        sprite_bank_ptr = (&house_data)[mode_no];
        sprite_image_no = ((struct rotated_sprite_rec *)rotated_bank0)[sprite_image_no].dir[map_direction >> 1];
    } else if (flags == 4) {
        sprite_bank_ptr = (&building_data1)[mode_no];
        sprite_image_no = ((struct rotated_sprite_rec *)rotated_bank1)[sprite_image_no].dir[map_direction >> 1];
    } else if (flags == 8) {
        sprite_bank_ptr = (&building_data2)[mode_no];
        sprite_image_no = ((struct rotated_sprite_rec *)rotated_bank2)[sprite_image_no].dir[map_direction >> 1];
    } else if (flags == 0xc) {
        sprite_bank_ptr = (&building_data3)[mode_no];
        sprite_image_no = ((struct rotated_sprite_rec *)rotated_bank3)[sprite_image_no].dir[map_direction >> 1];
    } else if (flags == 0x10) {
        sprite_bank_ptr = (&fixt_data)[mode_no];
        sprite_image_no = ((unsigned char *)rotated_map)[(map_direction >> 1) + (sprite_image_no - 0x10) * 4] + 0x10;
    } else if (flags == 0x14) {
        sprite_bank_ptr = (&building_data4)[mode_no];
        sprite_image_no = ((struct rotated_sprite_rec *)rotated_bank4)[sprite_image_no].dir[map_direction >> 1];
    } else {
        return;
    }
#else
    bank_kind = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits & 0x1c;
    rotation_idx = (map_direction >> 1) + sprite_image_no * 4;

    if (bank_kind == 0) {
        sprite_bank_ptr = house_data;
        sprite_image_no = rotated_bank0[rotation_idx];
    } else if (bank_kind == 4) {
        sprite_bank_ptr = building_data1;
        sprite_image_no = rotated_bank1[rotation_idx];
    } else if (bank_kind == 8) {
        sprite_bank_ptr = building_data2;
        sprite_image_no = rotated_bank2[rotation_idx];
    } else if (bank_kind == 0xc) {
        sprite_bank_ptr = building_data3;
        sprite_image_no = rotated_bank3[rotation_idx];
    } else if (bank_kind == 0x10) {
        sprite_bank_ptr = fixt_data;
        sprite_image_no = rotated_map[sprite_image_no - 0x10].dir[map_direction >> 1] + 0x10;
    } else if (bank_kind == 0x14) {
        sprite_bank_ptr = building_data4;
        sprite_image_no = rotated_bank4[rotation_idx];
    } else {
        return;
    }
#endif

    set_city_ambient((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind);
    data_ptr     = sprite_image_no * 16 + 8;
    y_length     = sprite_bank_ptr[data_ptr + 0xd];
    sprite_start = sprite_bank_ptr[data_ptr + 4]
                 + (sprite_bank_ptr[data_ptr + 5] << 8)
                 + (sprite_bank_ptr[data_ptr + 6] << 16);
    if (sprite_start > 0x4baf0) {
        sprite_error++;
        return;
    }
    if (sprite_start < 0) {
        sprite_error++;
        return;
    }
    if (y_length > 0xc8) {
        sprite_error++;
        return;
    }
    if (y_length < 0) {
        sprite_error++;
        return;
    }

    if (((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits & 1) != 0) {
        (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits &= 0xfe;
#if C2_FEAT_TILE_REFRESH
        refresh_a_bigger_square(sprite_x >> 4, (sprite_y - 0x30) >> 4);
#endif
    }

    if (edge_style == 3) {
        if      (zoom_level == 0) place_i_large_diamond_lefthalf(sprite_bank_ptr, 0);
        else if (zoom_level == 1) place_i_medium_diamond_lefthalf(sprite_bank_ptr, 0);
        else                       place_i_small_diamond_lefthalf(sprite_bank_ptr, 0);
    } else if (edge_style == 4) {
        if      (zoom_level == 0) place_i_large_diamond_righthalf(sprite_bank_ptr, 0);
        else if (zoom_level == 1) place_i_medium_diamond_righthalf(sprite_bank_ptr, 0);
        else                       place_i_small_diamond_righthalf(sprite_bank_ptr, 0);
    } else {
        if      (zoom_level == 0) place_i_large_diamond(sprite_bank_ptr, edge_style);
        else if (zoom_level == 1) place_i_medium_diamond(sprite_bank_ptr, edge_style);
        else                       place_i_small_diamond(sprite_bank_ptr, edge_style);
        sprite_x += pm_diamond_width;
    }
}

// Draw the visible upper portion of the current building at the requested zoom and edge style.
// FUNCTION: C2 0x379eb
// FUNCTION: C2WIN 0x0045cec9
void place_a_building_top(int edge_style)
{
#if PLATFORM_WINDOWS
    unsigned char flags;
    int mode_no;
    unsigned char height;
    int width;
    unsigned char *sprite_bank_ptr;
#else
    char height;
    unsigned char *sprite_bank_ptr;
    int map_dir_idx;
#endif

#if PLATFORM_WINDOWS
    if (map_mode > 1) mode_no = 0;
    else mode_no = map_mode;
#endif

    sprite_image_no = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).extra_edge;
#if PLATFORM_WINDOWS
    flags = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits & 0x1c;

    if (flags == 0) {
        sprite_bank_ptr = (&house_data)[mode_no];
        sprite_image_no = ((struct rotated_sprite_rec *)rotated_bank0)[sprite_image_no].dir[map_direction >> 1];
    } else if (flags == 4) {
        sprite_bank_ptr = (&building_data1)[mode_no];
        sprite_image_no = ((struct rotated_sprite_rec *)rotated_bank1)[sprite_image_no].dir[map_direction >> 1];
    } else if (flags == 8) {
        sprite_bank_ptr = (&building_data2)[mode_no];
        sprite_image_no = ((struct rotated_sprite_rec *)rotated_bank2)[sprite_image_no].dir[map_direction >> 1];
    } else if (flags == 0xc) {
        sprite_bank_ptr = (&building_data3)[mode_no];
        sprite_image_no = ((struct rotated_sprite_rec *)rotated_bank3)[sprite_image_no].dir[map_direction >> 1];
    } else if (flags == 0x10) {
        sprite_bank_ptr = (&fixt_data)[mode_no];
        sprite_image_no = ((unsigned char *)rotated_map)[(map_direction >> 1) + (sprite_image_no - 0x10) * 4] + 0x10;
    } else if (flags == 0x14) {
        sprite_bank_ptr = (&building_data4)[mode_no];
        sprite_image_no = ((struct rotated_sprite_rec *)rotated_bank4)[sprite_image_no].dir[map_direction >> 1];
    } else {
        return;
    }
#else
    height = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits & 0x1c; map_dir_idx = map_direction >> 1;

    if (height == 0) { sprite_bank_ptr = house_data; sprite_image_no = rotated_bank0[sprite_image_no * 4 + map_dir_idx]; }
    else if (height == 4) { sprite_bank_ptr = building_data1; sprite_image_no = rotated_bank1[sprite_image_no * 4 + map_dir_idx]; }
    else if (height == 8) { sprite_bank_ptr = building_data2; sprite_image_no = rotated_bank2[sprite_image_no * 4 + map_dir_idx]; }
    else if (height == 0xc) { sprite_bank_ptr = building_data3; sprite_image_no = rotated_bank3[sprite_image_no * 4 + map_dir_idx]; }
    else if (height == 0x10) { sprite_bank_ptr = fixt_data; sprite_image_no = rotated_map[sprite_image_no - 0x10].dir[map_dir_idx] + 0x10; }
    else if (height == 0x14) {
        sprite_bank_ptr = building_data4; sprite_image_no = rotated_bank4[sprite_image_no * 4 + map_dir_idx]; } else return;
#endif

    data_ptr     = sprite_image_no * 16 + 8;
    y_length     = sprite_bank_ptr[data_ptr + 0xd];
    height = sprite_bank_ptr[data_ptr + 0xc];
    sprite_start = sprite_bank_ptr[data_ptr + 4]
                 + (sprite_bank_ptr[data_ptr + 5] << 8)
                 + (sprite_bank_ptr[data_ptr + 6] << 16);
    if      (zoom_level == 0) sprite_hat_start = sprite_start + 0x384;
    else if (zoom_level == 1) sprite_hat_start = sprite_start + 0xc4;
    else                       sprite_hat_start = sprite_start + 0x24;

    if (sprite_start > 0x4baf0) { sprite_error++; return; }
    if (sprite_start < 0) { sprite_error++; return; }
    if (y_length > 0xc8) { sprite_error++; return; }
    if (y_length < 0) { sprite_error++; return; }

    if (edge_style == 3) {
        if (y_length != 0) {
            if (height == 2) {
                if      (zoom_level == 0) write_large_diamond_lefthat(sprite_bank_ptr, pm_y_clip);
                else if (zoom_level == 1) write_medium_diamond_lefthat(sprite_bank_ptr, pm_y_clip);
                else                       write_small_diamond_lefthat(sprite_bank_ptr, pm_y_clip);
            } else if (height == 4) {
                if      (zoom_level == 0) write_large_diamond_righthalfhat(sprite_bank_ptr, pm_y_clip, 2);
                else if (zoom_level == 1) write_medium_diamond_righthalfhat(sprite_bank_ptr, pm_y_clip, 2);
                else                       write_small_diamond_righthalfhat(sprite_bank_ptr, pm_y_clip, 2);
            }
        }
    } else if (edge_style == 4) {
        if (y_length != 0) {
            if (height == 2) {
                if      (zoom_level == 0) write_large_diamond_righthat(sprite_bank_ptr, pm_y_clip);
                else if (zoom_level == 1) write_medium_diamond_righthat(sprite_bank_ptr, pm_y_clip);
                else                       write_small_diamond_righthat(sprite_bank_ptr, pm_y_clip);
            }
            if (height == 3) {
                if      (zoom_level == 0) write_large_diamond_lefthalfhat(sprite_bank_ptr, pm_y_clip, 2);
                else if (zoom_level == 1) write_medium_diamond_lefthalfhat(sprite_bank_ptr, pm_y_clip, 2);
                else                       write_small_diamond_lefthalfhat(sprite_bank_ptr, pm_y_clip, 2);
            }
        }
    } else if (edge_style != 2) {
        if (y_length != 0) {
            if (height == 2) {
                if      (zoom_level == 0) write_large_diamond_hat(sprite_bank_ptr, pm_y_clip);
                else if (zoom_level == 1) write_medium_diamond_hat(sprite_bank_ptr, pm_y_clip);
                else                       write_small_diamond_hat(sprite_bank_ptr, pm_y_clip);
            } else if (height == 3) {
                if      (zoom_level == 0) write_large_diamond_lefthalfhat(sprite_bank_ptr, pm_y_clip, 0);
                else if (zoom_level == 1) write_medium_diamond_lefthalfhat(sprite_bank_ptr, pm_y_clip, 0);
                else                       write_small_diamond_lefthalfhat(sprite_bank_ptr, pm_y_clip, 0);
            } else if (height == 4) {
                if      (zoom_level == 0) write_large_diamond_righthalfhat(sprite_bank_ptr, pm_y_clip, 0);
                else if (zoom_level == 1) write_medium_diamond_righthalfhat(sprite_bank_ptr, pm_y_clip, 0);
                else                       write_small_diamond_righthalfhat(sprite_bank_ptr, pm_y_clip, 0);
            }
        }
    }
}

extern void write_large_diamond_rightroof(unsigned char *sprite_addr);
extern void write_large_diamond_righthalfroof(unsigned char *sprite_addr, int side);
extern void write_large_diamond_leftroof(unsigned char *);
extern void write_large_diamond_roof(unsigned char *);
extern void write_large_diamond_lefthalfroof(unsigned char *, int);
extern void write_medium_diamond_leftroof(unsigned char *);
extern void write_medium_diamond_rightroof(unsigned char *);
extern void write_medium_diamond_roof(unsigned char *);
extern void write_medium_diamond_lefthalfroof(unsigned char *, int);
extern void write_medium_diamond_righthalfroof(unsigned char *, int);
extern void write_small_diamond_leftroof(unsigned char *);
extern void write_small_diamond_rightroof(unsigned char *);
extern void write_small_diamond_roof(unsigned char *);
extern void write_small_diamond_lefthalfroof(unsigned char *, int);
extern void write_small_diamond_righthalfroof(unsigned char *, int);
extern void refresh_a_bigger_square(int x, int y);

// Draw the portion of the current building roof exposed at the lower viewport boundary.
// FUNCTION: C2 0x37dc4
// FUNCTION: C2WIN 0x0045d530
void place_a_building_roof(int edge_style)
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
    if (map_mode > 1) mode = 0;
    else mode = map_mode;
#endif

    sprite_image_no = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).extra_edge;
#if PLATFORM_WINDOWS
    flags = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits & 0x1c;

    if (flags == 0) {
        bank = (&house_data)[mode];
        sprite_image_no = ((struct rotated_sprite_rec *)rotated_bank0)[sprite_image_no].dir[map_direction >> 1];
    } else if (flags == 4) {
        bank = (&building_data1)[mode];
        sprite_image_no = ((struct rotated_sprite_rec *)rotated_bank1)[sprite_image_no].dir[map_direction >> 1];
    } else if (flags == 8) {
        bank = (&building_data2)[mode];
        sprite_image_no = ((struct rotated_sprite_rec *)rotated_bank2)[sprite_image_no].dir[map_direction >> 1];
    } else if (flags == 0xc) {
        bank = (&building_data3)[mode];
        sprite_image_no = ((struct rotated_sprite_rec *)rotated_bank3)[sprite_image_no].dir[map_direction >> 1];
    } else if (flags == 0x10) {
        bank = (&fixt_data)[mode];
        sprite_image_no = ((unsigned char *)rotated_map)[(map_direction >> 1) + (sprite_image_no - 0x10) * 4] + 0x10;
    } else if (flags == 0x14) {
        bank = (&building_data4)[mode];
        sprite_image_no = ((struct rotated_sprite_rec *)rotated_bank4)[sprite_image_no].dir[map_direction >> 1];
    } else {
        return;
    }
#else
    flags = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits & 0x1c;
    rotation_idx = (map_direction >> 1) + sprite_image_no * 4;

    if (flags == 0) {
        bank = house_data;
        sprite_image_no = rotated_bank0[rotation_idx];
    } else if (flags == 4) {
        bank = building_data1;
        sprite_image_no = rotated_bank1[rotation_idx];
    } else if (flags == 8) {
        bank = building_data2;
        sprite_image_no = rotated_bank2[rotation_idx];
    } else if (flags == 0xc) {
        bank = building_data3;
        sprite_image_no = rotated_bank3[rotation_idx];
    } else if (flags == 0x10) {
        bank = fixt_data;
        sprite_image_no = (rotated_map[sprite_image_no - 0x10].dir[map_direction >> 1] & 0xff) + 0x10;
    } else if (flags == 0x14) {
        bank = building_data4;
        sprite_image_no = rotated_bank4[rotation_idx];
    } else {
        return;
    }
#endif

    data_ptr     = sprite_image_no * 16 + 8;
    y_length     = bank[data_ptr + 0xd];
    if (y_length <= pm_y_clip) {
        if (edge_style < 3)
            sprite_x += pm_diamond_width;
        return;
    }

    height = bank[data_ptr + 0xc];
    sprite_start = bank[data_ptr + 4]
                 + (bank[data_ptr + 5] << 8)
                 + (bank[data_ptr + 6] << 16);
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
        if (((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits & 1) != 0) {
            (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).edge_bits &= 0xfe;
#if C2_FEAT_TILE_REFRESH
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

        if (edge_style == 3) {
            if (height == 2) {
                if      (zoom_level == 0) write_large_diamond_leftroof(bank);
                else if (zoom_level == 1) write_medium_diamond_leftroof(bank);
                else                       write_small_diamond_leftroof(bank);
            } else if (height == 4) {
                if      (zoom_level == 0) write_large_diamond_righthalfroof(bank, 2);
                else if (zoom_level == 1) write_medium_diamond_righthalfroof(bank, 2);
                else                       write_small_diamond_righthalfroof(bank, 2);
            }
        } else if (edge_style == 4) {
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
    }
}

extern void set_ambient_minimum(int ambient_idx, int minimum_delay);

// Draw the current cell's animated fire, unrest, smoke, decoration, or landmark effect.
// FUNCTION: C2 0x3820d
// FUNCTION: C2WIN 0x0045dc3f
void top_it(int edge_side)
{
    int ambient;
#if PLATFORM_WINDOWS
    int mode;
#endif
    int direction_index;
    unsigned char tile_type;
    int image_off_x;
    unsigned char building_animation;
    unsigned char activity_value;
    unsigned char jar_num;
    unsigned char sprite_num;
#if !PLATFORM_WINDOWS
    unsigned char *t;
#endif
    int y_delta;
    int scale;

    ambient = 0;
#if PLATFORM_WINDOWS
    mode = map_mode;
    if (mode > 1) mode = 0;
    direction_index = map_direction / 2;
#endif
    tile_type          = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind;
    building_animation = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).building;
#if PLATFORM_DOS
    image_off_x        = 0;
#endif

    if (tile_type < 8) {
        /* Fire flames */
        sprite_image_no = (building_animation + city_anim64) & 7;
        image_off_x     = fire_offs[((building_animation + city_anim16) & 0xf)].dx;
        y_delta         = fire_offs[((building_animation + city_anim16) & 0xf)].dy;
        if (zoom_level == 1) {
            image_off_x >>= 1;
            y_delta >>= 1;
        } else if (zoom_level == 2) {
            image_off_x >>= 2;
            y_delta >>= 2;
        }
        ambient = 1;
        set_this_ambient(6);
        set_ambient_minimum(6, 0xbe);
        emergency_mood = 0xa;
    } else if (tile_type >= 0x82 && tile_type <= 0xa1) {
        /* Riot */
        sprite_image_no = 8;
        image_off_x    = 0x14;
        y_delta        = -2;
        emergency_mood  = 0xa;
        if (zoom_level == 1) {
            image_off_x >>= 1;
            y_delta >>= 1;
        } else if (zoom_level == 2) {
            image_off_x >>= 2;
            y_delta >>= 2;
        }
    } else {
        if (tile_type == 0xfa) {
            activity_value = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).activity_a & 0xf;
            if (activity_value != 0) {
                /* Flower jars use the neighboring cell's variant. */
                jar_num = (*(struct city_cell *)((unsigned char *)city_map + ((pm_shown_ptr) - 0x14))).building & 0xf0;
                jar_num >>= 4;
                if (jar_num == 0)
                    return;
                sprite_image_no = jar_num + 0x18;
#if PLATFORM_WINDOWS
                image_off_x = city_jars_x_off[zoom_level][direction_index];
                y_delta     = city_jars_y_off[zoom_level][direction_index];
#else
                image_off_x = city_jars_x_off[zoom_level][map_direction / 2];
                y_delta     = city_jars_y_off[zoom_level][map_direction / 2];
#endif
            } else {
                /* Plain jars */
#if PLATFORM_WINDOWS
                jar_num = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).business & 0xf;
                sprite_image_no = jar_num + 9;
                image_off_x = city_type_x_off[zoom_level][direction_index];
                y_delta     = city_type_y_off[zoom_level][direction_index];
#else
                sprite_image_no = ((*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).business & 0xf) + 9;
                image_off_x = city_type_x_off[zoom_level][map_direction / 2];
                y_delta     = city_type_y_off[zoom_level][map_direction / 2];
#endif
            }
        } else if (tile_type == 0xe3) {
            /* Smoke / steam */
            sprite_image_no = ((building_animation + city_anim64) & 7) + 0x21;
            image_off_x = 0x1c;
            y_delta     = -0x1e;
            if (zoom_level == 1) {
                image_off_x = 0xe;
                y_delta = -0xf;
            } else if (zoom_level == 2) {
                image_off_x = 3;
                y_delta = -6;
            }
            ambient = 1;
        } else if (tile_type == 0xe7) {
            if (arena_top_count < 6)
                return;
            sprite_image_no = 0x3b;
#if PLATFORM_WINDOWS
            image_off_x = arena_top_data[zoom_level][direction_index].dx;
            y_delta     = arena_top_data[zoom_level][direction_index].dy;
#else
            image_off_x = arena_top_data[zoom_level][map_direction / 2].dx;
            y_delta     = arena_top_data[zoom_level][map_direction / 2].dy;
#endif
        } else if (tile_type == 0xe8) {
            if (colosseum_top_count < 9)
                return;
            sprite_image_no = 0x3c;
#if PLATFORM_WINDOWS
            image_off_x = colos_top_data[zoom_level][direction_index].dx;
            y_delta     = colos_top_data[zoom_level][direction_index].dy;
#else
            image_off_x = colos_top_data[zoom_level][map_direction / 2].dx;
            y_delta     = colos_top_data[zoom_level][map_direction / 2].dy;
#endif
#if PLATFORM_DOS
        } else {
            direction_index = map_direction >> 1;
            if (tile_type == 0xc0) {
#else
        } else if (tile_type == 0xc0) {
#endif
            /* Aqueduct top */
            sprite_num = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).extra_edge;
            sprite_image_no = rotated_bank2[sprite_num * 4 + direction_index] & 0xff;
            sprite_image_no = sprite_image_no + 0x1d;
            image_off_x = 0;
            y_delta     = -0x17;
            if (zoom_level == 1)      y_delta = -0xb;
            else if (zoom_level == 2) y_delta = -5;
        } else if (tile_type >= 0xd5 && tile_type <= 0xd6) {
#if PLATFORM_WINDOWS
            if (zoom_level > 1)
                return;
#else
            scale = zoom_level;
            if (scale > 1)
                return;
#endif
            sprite_num = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).extra_edge;
            sprite_image_no = (unsigned char)rotated_map[sprite_num - 0x10].dir[direction_index];
#if PLATFORM_WINDOWS
            sprite_image_no = sprite_image_no - 0x60;
#else
            sprite_image_no -= 0x60;
#endif
            if (sprite_image_no > 2) {
                image_off_x = 0;
#if PLATFORM_WINDOWS
            } else if (zoom_level == 1) {
#else
            } else if (scale == 1) {
#endif
                image_off_x = 8;
            } else {
                image_off_x = 0x10;
            }
            if (zoom_level == 0) {
                sprite_image_no = aquaduct_tops2[sprite_image_no];
            } else {
                sprite_image_no = aquaduct_tops2[sprite_image_no];
            }
            y_delta = 0;
        } else {
            return;
        }
#if PLATFORM_DOS
        }
#endif
    }

    if (edge_side == 1)
        image_off_x -= pm_diamond_half_width;

    data_ptr     = sprite_image_no * 16 + 8;
#if PLATFORM_WINDOWS
    sprite_start  = ((&tops_data)[mode][data_ptr + 5] << 8)
                  + ((&tops_data)[mode][data_ptr + 6] << 16)
                  + (&tops_data)[mode][data_ptr + 4];
    sprite_width  = ((&tops_data)[mode][data_ptr + 1] << 8)
                  + (&tops_data)[mode][data_ptr];
    sprite_height = ((&tops_data)[mode][data_ptr + 3] << 8)
                  + (&tops_data)[mode][data_ptr + 2];
#else
    t = tops_data + data_ptr;
    sprite_start  = (t[5] << 8) + t[4] + (t[6] << 16);
    sprite_width  = t[0] + (t[1] << 8);
    sprite_height = (t[2]) + (t[3] << 8);
#endif
    if (sprite_start > 0x4baf0) {
        sprite_error++;
        return;
    }
    if (sprite_start < 0) {
        sprite_error++;
        return;
    }
    if (sprite_width > 0x280) {
        sprite_error++;
        return;
    }
    if (sprite_height > 0x1e0) {
        sprite_error++;
        return;
    }

    old_sprite_x = sprite_x;
    old_sprite_y = sprite_y;
    sprite_x    += image_off_x;
    sprite_y    += y_delta;

#if C2_FEAT_TILE_REFRESH
    if (ambient == 1)
        refresh_sprite_square(sprite_x >> 4, sprite_y >> 4);
    else if (ambient == 2)
        refresh_sprite2w_square(sprite_x >> 4, sprite_y >> 4);
#endif

#if PLATFORM_WINDOWS
    xclip(pm_screen_x_start, pm_screen_x_end);
    yclip(pm_screen_y_start + pm_diamond_height, pm_screen_y_end);
#else
    xclip(pm_screen_x_start, 0x1de);
    if (zoom_level == 1)
        yclip(0x18, 0x1d8);
    else
        yclip(0x18, 0x1da);
#endif

#if PLATFORM_WINDOWS
    if (yclipped == 5)
        goto put_back;
    if      (xclipped == 1) write_i_left_sprite((&tops_data)[mode]);
    else if (xclipped == 2) write_i_right_sprite((&tops_data)[mode]);
    else                    write_i_sprite((&tops_data)[mode]);
put_back:
#else
    if (yclipped != 5) {
        if      (xclipped == 1) write_i_left_sprite(tops_data);
        else if (xclipped == 2) write_i_right_sprite(tops_data);
        else                    write_i_sprite(tops_data);
    }
#endif

    sprite_x = old_sprite_x;
    sprite_y = old_sprite_y;
}

// Draw the current cell's citizens and optional flag-mode marker with movement and clipping.
// FUNCTION: C2 0x386f9
// FUNCTION: C2WIN 0x0045e38c
void place_sprite(int edge_side)
{
    unsigned char terrain_flags;
    int marker_kind;
#if PLATFORM_WINDOWS
    int mode;
#endif
    int sprite_offset_x;
    int sprite_offset_y;
    int direction;
#if !PLATFORM_WINDOWS
    unsigned char *sprite_header_ptr;
#endif
#if PLATFORM_DOS
    int data_byte;
#endif

#if PLATFORM_WINDOWS
    mode = map_mode;
    if (mode > 1) mode = 0;
    if (pm_shown_y >= 0xa1) return;
#endif
    marker_kind = 0;
#if PLATFORM_WINDOWS
    citizen_a = citizen_b = marker_kind;
#else
    citizen_b = 0;
    citizen_a = 0;
#endif
    citizen_a = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).citizen_a;
    citizen_b = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).citizen_b;
    terrain_flags = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).terrain;
    if (flag_mode != 0)
        marker_kind = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).road_aqueduct;

    if (citizen_a != 0) {
        direction = citizen_list[citizen_a].world_dir - map_direction;
        if (direction < 0) direction += 8;
        if (zoom_level == 0) {
            sprite_offset_x = walking_x_ofsets_zoom0[direction * 16 + citizen_list[citizen_a].speed_count];
            sprite_offset_y = walking_y_ofsets_zoom0[direction * 16 + citizen_list[citizen_a].speed_count];
        } else if (zoom_level == 1) {
            sprite_offset_x = walking_x_ofsets_zoom1[direction * 16 + citizen_list[citizen_a].speed_count];
            sprite_offset_y = walking_y_ofsets_zoom1[direction * 16 + citizen_list[citizen_a].speed_count];
        } else {
            sprite_offset_x = walking_x_ofsets_zoom2[direction * 16 + citizen_list[citizen_a].speed_count];
            sprite_offset_y = walking_y_ofsets_zoom2[direction * 16 + citizen_list[citizen_a].speed_count];
        }
        if (edge_side == 1) {
            sprite_offset_x -= 2;
        } else if (edge_side == 2) {
            sprite_offset_x += pm_diamond_half_width - 2;
        } else {
            sprite_offset_x += pm_diamond_half_width;
        }
#if PLATFORM_WINDOWS
        sprite_offset_y += pm_diamond_height;
#else
        sprite_offset_y += pm_diamond_half_height;
#endif
        if (zoom_level == 0) {
            if (direction <= 3) {
                if ((terrain_flags & 0x40) != 0) {
#if PLATFORM_WINDOWS
                    sprite_offset_y += 2;
                    citizen_list[citizen_a].wobble_counter = 0x10;
#else
                    citizen_list[citizen_a].wobble_counter = 0x10;
                    sprite_offset_y += 2;
#endif
                } else if (citizen_list[citizen_a].wobble_counter != 0) {
                    sprite_offset_y += 2;
                    citizen_list[citizen_a].wobble_counter--;
                } else {
                    sprite_offset_y -= 2;
                }
            } else {
                sprite_offset_y += 6;
            }
        }
        sprite_image_no = citizen_list[citizen_a].image_id;
        data_ptr        = sprite_image_no * 16 + 8;
#if PLATFORM_WINDOWS
        sprite_start  = ((&people_data)[mode][data_ptr + 5] << 8)
                      + ((&people_data)[mode][data_ptr + 6] << 16)
                      + (&people_data)[mode][data_ptr + 4];
        sprite_width  = ((&people_data)[mode][data_ptr + 1] << 8)
                      + (&people_data)[mode][data_ptr];
        sprite_height = ((&people_data)[mode][data_ptr + 3] << 8)
                      + (&people_data)[mode][data_ptr + 2];
#else
        sprite_header_ptr = people_data + data_ptr; sprite_start = (sprite_header_ptr[5] << 8) + sprite_header_ptr[4] + (sprite_header_ptr[6] << 16);
        sprite_width    = sprite_header_ptr[0] + (sprite_header_ptr[1] << 8);
        sprite_height   = sprite_header_ptr[2] + (sprite_header_ptr[3] << 8);
#endif
        if (sprite_start > 0x4baf0) {
            sprite_error++;
            return;
        }
        if (sprite_width <= 0) {
            sprite_error++;
            return;
        }
        if (sprite_width > 0x12c) {
            sprite_error++;
            return;
        }
        if (sprite_height <= 0) {
            sprite_error++;
            return;
        }
        if (sprite_height > 0x12c) {
            sprite_error++;
            return;
        }
        old_sprite_x = sprite_x;
        old_sprite_y = sprite_y;
        sprite_x    += sprite_offset_x;
        sprite_y    += sprite_offset_y;
        sprite_x    -= sprite_width >> 1;
        sprite_y    -= sprite_height;
#if C2_FEAT_TILE_REFRESH
        refresh_sprite_square(sprite_x >> 4, sprite_y >> 4);
#endif
#if PLATFORM_WINDOWS
        xclip(pm_screen_x_start, pm_screen_x_end);
        yclip(pm_screen_y_start + pm_diamond_height, pm_screen_y_end);
#else
        xclip(pm_screen_x_start, 0x1de);
        if (zoom_level == 1) yclip(0x18, 0x1d8);
        else                  yclip(0x18, 0x1da);
#endif
        if (yclipped == 5) goto after_citizen_a;
#if PLATFORM_WINDOWS
            if (xclipped == 1)
                write_i_left_sprite((&people_data)[mode]);
            else if (xclipped == 2)
                write_i_right_sprite((&people_data)[mode]);
            else
                write_i_sprite((&people_data)[mode]);
#else
            if (xclipped == 1)
                write_i_left_sprite(people_data);
            else if (xclipped == 2)
                write_i_right_sprite(people_data);
            else
                write_i_sprite(people_data);
#endif
after_citizen_a:
        sprite_x = old_sprite_x;
        sprite_y = old_sprite_y;
    }

    if (citizen_b != 0) {
        direction = citizen_list[citizen_b].world_dir - map_direction;
        if (direction < 0) direction += 8;
        if (zoom_level == 0) {
            sprite_offset_x = walking_x_ofsets_zoom0[direction * 16 + citizen_list[citizen_b].speed_count];
            sprite_offset_y = walking_y_ofsets_zoom0[direction * 16 + citizen_list[citizen_b].speed_count];
        } else if (zoom_level == 1) {
            sprite_offset_x = walking_x_ofsets_zoom1[direction * 16 + citizen_list[citizen_b].speed_count];
            sprite_offset_y = walking_y_ofsets_zoom1[direction * 16 + citizen_list[citizen_b].speed_count];
        } else {
            sprite_offset_x = walking_x_ofsets_zoom2[direction * 16 + citizen_list[citizen_b].speed_count];
            sprite_offset_y = walking_y_ofsets_zoom2[direction * 16 + citizen_list[citizen_b].speed_count];
        }
        if (edge_side == 1) {
            sprite_offset_x -= 2;
        } else if (edge_side == 2) {
            sprite_offset_x += pm_diamond_half_width - 2;
        } else {
            sprite_offset_x += pm_diamond_half_width;
        }
#if PLATFORM_WINDOWS
        sprite_offset_y += pm_diamond_height;
#else
        sprite_offset_y += pm_diamond_half_height;
#endif
        if (zoom_level == 0) {
            if (direction <= 3) {
                if ((terrain_flags & 0x40) != 0) {
#if PLATFORM_WINDOWS
                    sprite_offset_y += 2;
                    citizen_list[citizen_b].wobble_counter = 0x10;
#else
                    citizen_list[citizen_b].wobble_counter = 0x10;
                    sprite_offset_y += 2;
#endif
                } else if (citizen_list[citizen_b].wobble_counter != 0) {
                    sprite_offset_y += 2;
                    citizen_list[citizen_b].wobble_counter--;
                } else {
                    sprite_offset_y -= 2;
                }
            } else {
                sprite_offset_y += 6;
            }
        }
        sprite_image_no = citizen_list[citizen_b].image_id;
        data_ptr        = sprite_image_no * 16 + 8;
#if PLATFORM_WINDOWS
        sprite_start  = ((&people_data)[mode][data_ptr + 5] << 8)
                      + ((&people_data)[mode][data_ptr + 6] << 16)
                      + (&people_data)[mode][data_ptr + 4];
        sprite_width  = ((&people_data)[mode][data_ptr + 1] << 8)
                      + (&people_data)[mode][data_ptr];
        sprite_height = ((&people_data)[mode][data_ptr + 3] << 8)
                      + (&people_data)[mode][data_ptr + 2];
#else
        sprite_header_ptr = people_data + data_ptr; sprite_start = (sprite_header_ptr[6] << 16) + (sprite_header_ptr[4] + (sprite_header_ptr[5] << 8));
        sprite_width    = sprite_header_ptr[0] + (sprite_header_ptr[1] << 8);
        sprite_height   = sprite_header_ptr[2] + (sprite_header_ptr[3] << 8);
#endif
        if (sprite_start > 0x4baf0) {
            sprite_error++;
            return;
        }
        if (sprite_width <= 0) {
            sprite_error++;
            return;
        }
        if (sprite_width > 0x12c) {
            sprite_error++;
            return;
        }
        if (sprite_height <= 0) {
            sprite_error++;
            return;
        }
        if (sprite_height > 0x12c) {
            sprite_error++;
            return;
        }
        old_sprite_x = sprite_x;
        old_sprite_y = sprite_y;
        sprite_x    += sprite_offset_x;
        sprite_y    += sprite_offset_y;
        sprite_x    -= sprite_width >> 1;
        sprite_y    -= sprite_height;
#if C2_FEAT_TILE_REFRESH
        refresh_sprite_square(sprite_x >> 4, sprite_y >> 4);
#endif
#if PLATFORM_WINDOWS
        xclip(pm_screen_x_start, pm_screen_x_end);
        yclip(pm_screen_y_start + pm_diamond_height, pm_screen_y_end);
#else
        xclip(pm_screen_x_start, 0x1de);
        if (zoom_level == 1) yclip(0x18, 0x1d8);
        else                  yclip(0x18, 0x1da);
#endif
        if (yclipped == 5) goto after_citizen_b;
#if PLATFORM_WINDOWS
            if (xclipped == 1)
                write_i_left_sprite((&people_data)[mode]);
            else if (xclipped == 2)
                write_i_right_sprite((&people_data)[mode]);
            else
                write_i_sprite((&people_data)[mode]);
#else
            if (xclipped == 1)
                write_i_left_sprite(people_data);
            else if (xclipped == 2)
                write_i_right_sprite(people_data);
            else
                write_i_sprite(people_data);
#endif
after_citizen_b:
        sprite_x = old_sprite_x;
        sprite_y = old_sprite_y;
    }

    if (marker_kind != 0) {
#if PLATFORM_DOS
    sprite_offset_y = 0;
    if (edge_side == 1)      sprite_offset_x = -2;
    else if (edge_side == 2) sprite_offset_x = pm_diamond_half_width - 2;
    else                sprite_offset_x = pm_diamond_half_width;
    sprite_offset_y += pm_diamond_half_height;
#else
    sprite_offset_x = sprite_offset_y = 0;
    if (edge_side == 1)      sprite_offset_x -= 2;
    else if (edge_side == 2) sprite_offset_x += pm_diamond_half_width - 2;
    else                     sprite_offset_x += pm_diamond_half_width;
    sprite_offset_y += pm_diamond_height;
#endif

    if (marker_kind == 3) sprite_image_no = zoom_level + 0xe;
    else                 sprite_image_no = zoom_level + 0xb;
    data_ptr        = sprite_image_no * 16 + 8;
#if PLATFORM_DOS
    sprite_start    = (mice[data_ptr + 4] + (mice[data_ptr + 5] << 8)) + ((data_byte = mice[data_ptr + 6]) << 16);
    sprite_width    = (data_byte = mice[data_ptr + 0]) + ((data_byte = mice[data_ptr + 1]) << 8);
    sprite_height   = mice[data_ptr + 2] + (mice[data_ptr + 3] << 8);
#else
    sprite_start  = (mice[data_ptr + 6] << 16)
                  + (mice[data_ptr + 5] << 8)
                  + mice[data_ptr + 4];
    sprite_width  = (mice[data_ptr + 1] << 8) + mice[data_ptr];
    sprite_height = (mice[data_ptr + 3] << 8) + mice[data_ptr + 2];
#endif

    old_sprite_x = sprite_x;
    old_sprite_y = sprite_y;
    sprite_x    += sprite_offset_x;
    sprite_y    += sprite_offset_y;
    sprite_y    -= sprite_height;
#if C2_FEAT_TILE_REFRESH
    refresh_sprite_square(sprite_x >> 4, sprite_y >> 4);
#endif
#if PLATFORM_WINDOWS
    xclip(pm_screen_x_start, pm_screen_x_end);
    yclip(pm_screen_y_start + pm_diamond_height, pm_screen_y_end);
#else
    xclip(pm_screen_x_start, 0x1de);
    if (zoom_level == 1) yclip(0x18, 0x1d8);
    else                  yclip(0x18, 0x1da);
#endif
    if (yclipped == 5) goto after_marker;
    if (xclipped == 1)
        write_i_left_sprite(mice);
    else if (xclipped == 2)
        write_i_right_sprite(mice);
    else
        write_i_sprite(mice);
after_marker:
    sprite_x = old_sprite_x;
    sprite_y = old_sprite_y;
    }
}

// Display the active diagnostic value over the current city-map cell.
// FUNCTION: C2 0x38e80
// FUNCTION: C2WIN 0x0045ef7d
void print_test_info(void)
{
    int v;
    int col;
    int fig;

    if (test_mode1 != 0) {
        if (((pm_shown_ptr) >= 0x0FFF0000)) return;
        old_sprite_x = sprite_x;
        old_sprite_y = sprite_y;
        v = (signed char)(*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).terrain;
#if PLATFORM_WINDOWS
        if (v < 0) { v = -v; col = 0xf9; }
#else
        if (v < 0) { v = -v; col = 3; }
#endif
        else       col = 0x3f;
        font_no(v, 0x20, " ",
                sprite_x + 0x14 - pm_diamond_width, sprite_y + 0xa,
                font1, col);
        sprite_x = old_sprite_x;
        sprite_y = old_sprite_y;
    } else if (test_mode2 != 0) {
        if (((pm_shown_ptr) >= 0x0FFF0000)) return;
        old_sprite_x = sprite_x;
        old_sprite_y = sprite_y;
        v = (signed char)(*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).road_aqueduct;
#if PLATFORM_WINDOWS
        if (v < 0) { v = -v; col = 0xf9; }
#else
        if (v < 0) { v = -v; col = 3; }
#endif
        else       col = 0x3f;
        font_no(v, 0x20, " ",
                sprite_x + 0x14 - pm_diamond_width, sprite_y + 0xa,
                font1, col);
        sprite_x = old_sprite_x;
        sprite_y = old_sprite_y;
    }
}

// Draw the current cell's full-diamond overlay and report whether it replaces the base layer.
// FUNCTION: C2 0x38f5b
// FUNCTION: C2WIN 0x0045f0f8
int show_overlay(int style)
{
    int map_ptr;
    unsigned char terrain_flags;
    unsigned char tile;
    int in_range = 0;

    if (overlays_on != 1)
        return 0;
    map_ptr = pm_shown_ptr / 20;

    if (landfill_pool[map_ptr] == 0) {
        terrain_flags = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).terrain & 0xe7;
        tile = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind;
        if (terrain_flags == 0)
            return 0;
        if (overlay0_empty_mode != 0)
            return 0;
        if (tile >= 0x82 && tile <= 0xa1)
            sprite_image_no = 7;
        else
            sprite_image_no = 0;
    } else if (landfill_pool[map_ptr] == 0x96) {
        if (ov_map_mode == 1) {
            sprite_image_no = landfill_pool[map_ptr] - 0x76;
        } else if (ov_map_mode == 6) {
            tile = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind;
            if (tile >= 0xe5 && tile <= 0xf0)
                return 0;
            sprite_image_no = landfill_pool[map_ptr] - 0x76;
        } else {
            return 0;
        }
    } else {
        sprite_image_no = landfill_pool[map_ptr] - 0x76;
    }

    if (sprite_image_no >= 8) {
        tile                  = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind;
        terrain_flags         = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).terrain & 1;
        in_range = (tile >= 0x82 && tile <= 0xa1);
        if (in_range && sprite_image_no >= 8) sprite_image_no += 2;
        else if (terrain_flags) sprite_image_no++;
    }
    if (sprite_image_no < 0x23)
        place_overlay(style);
    sprite_x += pm_diamond_width;
    return 1;
}

// Draw the current cell's overlay clipped to the left edge.
// FUNCTION: C2 0x390c3
// FUNCTION: C2WIN 0x0045f34e
int show_left_overlay(int style)
{
#if PLATFORM_DOS
    unsigned char tile;
    unsigned char overlay_value;
    int map_ptr;
    unsigned char terrain_flags;
    int in_range = 0;
    int overlay_image_idx;
#else
    int map_ptr;
    unsigned char terrain_flags;
    unsigned char tile;
    int in_range = 0;
#endif

    if (overlays_on != 1) return 0;
    map_ptr = pm_shown_ptr / 20;
#if PLATFORM_DOS
    if ((overlay_value = landfill_pool[map_ptr]) == 0) {
#else
    if (landfill_pool[map_ptr] == 0) {
#endif
        terrain_flags = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).terrain & 0xe7;
        tile = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind;
        if (terrain_flags == 0)
            return 0;
        if (overlay0_empty_mode != 0)
            return 0;
        if (tile >= 0x82 && tile <= 0xa1) sprite_image_no = 7;
        else sprite_image_no = 0;
#if PLATFORM_DOS
    } else {
        overlay_image_idx = overlay_value - 0x76; if (overlay_value == 0x96) {
            if (ov_map_mode == 1) sprite_image_no = overlay_image_idx;
            else if (ov_map_mode == 6) {
                tile = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind;
                if (tile >= 0xe5 && tile <= 0xf0)
                    return 0;
                sprite_image_no = landfill_pool[map_ptr] - 0x76;
            } else {
                return 0;
            }
        } else {
            sprite_image_no = overlay_image_idx;
        }
#else
    } else if (landfill_pool[map_ptr] == 0x96) {
        if (ov_map_mode == 1) {
            sprite_image_no = landfill_pool[map_ptr] - 0x76;
        } else if (ov_map_mode == 6) {
            tile = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind;
            if (tile >= 0xe5 && tile <= 0xf0)
                return 0;
            sprite_image_no = landfill_pool[map_ptr] - 0x76;
        } else {
            return 0;
        }
    } else {
        sprite_image_no = landfill_pool[map_ptr] - 0x76;
#endif
    }

    if (sprite_image_no >= 8) {
        tile          = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind;
        terrain_flags = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).terrain & 1;
        in_range      = (tile >= 0x82 && tile <= 0xa1);
        if (in_range && sprite_image_no >= 8) sprite_image_no += 2;
        else if (terrain_flags) sprite_image_no++;
    }
    if (sprite_image_no < 0x23) place_lefthalf_overlay(style);
    return 1;
}

// Draw the current cell's overlay clipped to the right edge.
// FUNCTION: C2 0x3921d
// FUNCTION: C2WIN 0x0045f599
int show_right_overlay(int style)
{
#if PLATFORM_DOS
    unsigned char tile;
    unsigned char overlay_value;
    int map_ptr;
    unsigned char terrain_flags;
    int in_range = 0;
    int overlay_image_idx;
#else
    int map_ptr;
    unsigned char terrain_flags;
    unsigned char tile;
    int in_range = 0;
#endif

    if (overlays_on != 1) return 0;
    map_ptr = pm_shown_ptr / 20;
#if PLATFORM_DOS
    if ((overlay_value = landfill_pool[map_ptr]) == 0) {
#else
    if (landfill_pool[map_ptr] == 0) {
#endif
        terrain_flags = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).terrain & 0xe7;
        tile = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind;
        if (terrain_flags == 0)
            return 0;
        if (overlay0_empty_mode != 0)
            return 0;
        if (tile >= 0x82 && tile <= 0xa1) sprite_image_no = 7;
        else sprite_image_no = 0;
#if PLATFORM_DOS
    } else {
        overlay_image_idx = overlay_value - 0x76; if (overlay_value == 0x96) {
            if (ov_map_mode == 1) sprite_image_no = overlay_image_idx;
            else if (ov_map_mode == 6) {
                tile = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind;
                if (tile >= 0xe5 && tile <= 0xf0)
                    return 0;
                sprite_image_no = landfill_pool[map_ptr] - 0x76;
            } else {
                return 0;
            }
        } else {
            sprite_image_no = overlay_image_idx;
        }
#else
    } else if (landfill_pool[map_ptr] == 0x96) {
        if (ov_map_mode == 1) {
            sprite_image_no = landfill_pool[map_ptr] - 0x76;
        } else if (ov_map_mode == 6) {
            tile = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind;
            if (tile >= 0xe5 && tile <= 0xf0)
                return 0;
            sprite_image_no = landfill_pool[map_ptr] - 0x76;
        } else {
            return 0;
        }
    } else {
        sprite_image_no = landfill_pool[map_ptr] - 0x76;
#endif
    }

    if (sprite_image_no >= 8) {
        tile          = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind;
        terrain_flags = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).terrain & 1;
        in_range      = (tile >= 0x82 && tile <= 0xa1);
        if (in_range && sprite_image_no >= 8) sprite_image_no += 2;
        else if (terrain_flags) sprite_image_no++;
    }
    if (sprite_image_no < 0x23) place_righthalf_overlay(style);
    return 1;
}

// Report whether an overlay on the current cell suppresses its normal rendering.
// FUNCTION: C2 0x39377
// FUNCTION: C2WIN 0x0045f7e4
int are_overlays_on(void)
{
    int map_ptr;
    unsigned char terrain_flags;
    unsigned char building_kind;

    if (overlays_on != 1) return 0;
    if (((pm_shown_ptr) >= 0x0FFF0000)) return 0;
    map_ptr = pm_shown_ptr / 20;
    if (landfill_pool[map_ptr] == 0) {
        terrain_flags = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).terrain & 0xe7;
        if (terrain_flags == 0) return 0;
        if (overlay0_empty_mode != 0) return 0;
        return 1;
    }
    if (landfill_pool[map_ptr] == 0x96) {
        if (ov_map_mode == 1) {
            return 1;
        } else if (ov_map_mode == 6) {
            building_kind = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).base_kind;
            if (building_kind >= 0xe5 && building_kind <= 0xf0) {
                return 0;
            }
            return 1;
        } else {
            return 0;
        }
    }
    return 1;
}
