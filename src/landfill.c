
#include "c2_data.h"

/* Blit encoded landfill sprites in the four supported footprint sizes. */
extern void place_2x2_block(unsigned char *src, int screen_off);
extern void place_4x4_block(unsigned char *src, int screen_off);
extern void place_6x6_block(unsigned char *src, int screen_off);
extern void place_8x8_block(unsigned char *src, int screen_off);
/* Forward declarations (functions defined later in this file). */
void show_city_landfill(int x_start, int y_start);
void get_circus_bodge(unsigned char building_kind);
void show_region_landfill(int x_start, int y_start);


// Clear the city map's per-cell overlay sprite pool.
// FUNCTION: C2 0x3e9c0
// FUNCTION: C2WIN 0x0049d310
void clear_landfill(void)
{
    cm_y = 0;
    cm_dptr = 0;
    for ( ; cm_y < 80; cm_y++) {
        cm_x = 0;
        do {
            landfill_pool[cm_dptr] = 0;
            cm_x++;
            cm_dptr++;
        } while (cm_x < 80);
    }
}

// Rebuild the city map's overlay sprite pool, optionally evolving derived cell state first.
// FUNCTION: C2 0x3ea13
// FUNCTION: C2WIN 0x0049d385
void get_landfill(int evolve)
{
    if (map_mode != 0) return;
    if (evolve != 0) evolve_to_current_fabric();

    cm_y = 0;
    cm_sptr = 0;
    cm_dptr = 0;
    for ( ; cm_y < 80; cm_y++) {
        cm_x = 0;
        do {
            ov_routines[ov_map_mode]();
            cm_x++;
            cm_sptr += 20;
            cm_dptr++;
        } while (cm_x < 80);
    }
}

// Overlay routine for the "no overlay" mode: clears the current cell's landfill byte.
// FUNCTION: C2 0x3ea86
// FUNCTION: C2WIN 0x0049d430
void get_no_ov_image(void)
{
    landfill_pool[cm_dptr] = 0;
}

// Overlay routine for the land-value view: maps the cell's signed land_value (clamped to 0..0x40)
// to a sprite index in the 0x7E + 3*(val/8) ramp; zero land-value clears the byte.
// FUNCTION: C2 0x3ea96
// FUNCTION: C2WIN 0x0049d447
void get_landval_ov_image(void)
{
    signed char land_value;

    land_value = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).land_value;
    if (land_value < 0) land_value = 0;
    else if (land_value >= 0x40) land_value = 0x40;

    if (land_value > 0) {
        landfill_pool[cm_dptr] = (unsigned char)((land_value / 8) * 3 + 0x7e);
    } else {
        landfill_pool[cm_dptr] = 0;
    }
}

// Overlay routine for the water-supply view: 0x96 for water-blocking terrain (terrain bits 0xC0
// set, or aqueduct kinds 0xD7..0xDE); 0x87 for tiles with both running water and a fountain; 0x84
// for water-only; 0x8D for fountain-only.
// FUNCTION: C2 0x3eaea
// FUNCTION: C2WIN 0x0049d4c5
void get_water_ov_image(void)
{
    unsigned char terrain_flag;
    unsigned char water_flags;
    unsigned char water_supply;
    unsigned char has_fountain;
    unsigned char image;
    unsigned char base_kind;

    terrain_flag = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).terrain;
    base_kind = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind;
    water_flags = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).education;
    has_fountain = water_flags & 4;
    water_supply = water_flags & 3;

    if ((terrain_flag & 0xc0) != 0) { landfill_pool[cm_dptr] = 0x96; return; }
    if (base_kind >= 0xd7 && base_kind <= 0xde) { landfill_pool[cm_dptr] = 0x96; return; }
    if (water_supply != 0 && has_fountain != 0) { landfill_pool[cm_dptr] = 0x87; return; }
    if (water_supply != 0) { landfill_pool[cm_dptr] = 0x84; return; }
    if (has_fountain != 0) { landfill_pool[cm_dptr] = 0x8d; return; }
    landfill_pool[cm_dptr] = 0;
}

// Overlay routine for the administration view: 0x96 for forum-class building footprints (kind
// 0xAE..0xB9); otherwise reads the 2-bit admin coverage from cell.range_flag bits 2..3 -- level 1
// (0x4) = 0x93, level 2 (0x8) = 0x90, level 3 = 0x8D.
// FUNCTION: C2 0x3eb87
// FUNCTION: C2WIN 0x0049d5d3
void get_admin_ov_image(void)
{
#if PLATFORM_WINDOWS
    unsigned char terrain_flag;
#endif
    unsigned char building_kind;
    unsigned char admin;

#if PLATFORM_WINDOWS
    terrain_flag = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).terrain;
#endif
    building_kind = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind;
    admin = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).range_flag & 0xc;
    if (building_kind >= 0xae && building_kind <= 0xb9) {
        landfill_pool[cm_dptr] = 0x96;
    } else if (admin != 0) {
        if (admin == 4) landfill_pool[cm_dptr] = 0x93;
        else if (admin == 8) landfill_pool[cm_dptr] = 0x90;
        else landfill_pool[cm_dptr] = 0x8d;
    } else {
        landfill_pool[cm_dptr] = 0;
    }
}

// Choose a security-overlay tile from crime, prefect coverage, terrain, and building kind.
// FUNCTION: C2 0x3ec0b
// FUNCTION: C2WIN 0x0049d6a9
void get_security_ov_image(void)
{
    unsigned char terrain;
    unsigned char security;
    signed char crime;
    unsigned char building_kind;
    unsigned char level;

    terrain = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).terrain;
    building_kind = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind;
    crime = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).security;
    security = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).range_flag & 0x30;

    if (crime < 0x10) level = 0;
    else if (crime >= 0x10) level = 1;
    if (security != 0) level++;

    if ((terrain & 6) != 0) { landfill_pool[cm_dptr] = 0x96; return; }
    if (building_kind >= 0x1e && building_kind <= 0x51) { landfill_pool[cm_dptr] = 0x96; return; }
    if (building_kind == 0xe3) { landfill_pool[cm_dptr] = 0x96; return; }
    if (building_kind == 0xe4) { landfill_pool[cm_dptr] = 0x96; return; }
    if (level != 0) {
        if (level == 2) { landfill_pool[cm_dptr] = 0x8d; return; }
        if (security != 0) { landfill_pool[cm_dptr] = 0x93; return; }
        if (level == 1) { landfill_pool[cm_dptr] = 0x90; return; }
    } else {
        landfill_pool[cm_dptr] = level;
    }
}

// Overlay routine for the health view: reads the 2-bit health coverage from cell.fpu_flag bits
// 4..5, mapping 0x10 -> 0x79 (mild), 0x20 -> 0x78 (moderate), 0x30 -> 0x77 (heavy).
// FUNCTION: C2 0x3ecd2
// FUNCTION: C2WIN 0x0049d833
void get_health_ov_image(void)
{
    unsigned char health;

    health = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).fpu_flag & 0x30;
    if (health != 0) {
        if (health == 0x10) landfill_pool[cm_dptr] = 0x79;
        else if (health == 0x20) landfill_pool[cm_dptr] = 0x78;
        else landfill_pool[cm_dptr] = 0x77;
    } else {
        landfill_pool[cm_dptr] = 0;
    }
}

// Overlay routine for the education view: 0x96 for school / academy / library building footprints
// (kind 0xF3..0xF5); otherwise reads cell.education bits 4 (school) and 5 (academy) -- both =
// 0x87, school-only = 0x8D, academy-only = 0x84.
// FUNCTION: C2 0x3ed2a
// FUNCTION: C2WIN 0x0049d8bc
void get_education_ov_image(void)
{
    unsigned char range;
    unsigned char grammaticus;
    unsigned char rhetor;
#if PLATFORM_WINDOWS
    unsigned char terrain_flag;
#endif
    unsigned char building_kind;

#if PLATFORM_WINDOWS
    terrain_flag = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).terrain;
#endif
    building_kind = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind;
    range = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).education;
    grammaticus = range & 0x10;
    rhetor = range & 0x20;
    if (building_kind >= 0xf3 && building_kind <= 0xf5) { landfill_pool[cm_dptr] = 0x96; return; }
    if (grammaticus != 0 && rhetor != 0) { landfill_pool[cm_dptr] = 0x87; return; }
    if (grammaticus != 0) { landfill_pool[cm_dptr] = 0x8d; return; }
    if (rhetor != 0) { landfill_pool[cm_dptr] = 0x84; return; }
    landfill_pool[cm_dptr] = 0;
}

// Choose an entertainment-overlay tile from theatre, arena, and colosseum coverage.
// FUNCTION: C2 0x3edb3
// FUNCTION: C2WIN 0x0049d9ac
void get_entertainment_ov_image(void)
{
    unsigned char theatre;
    unsigned char colosseum;
    unsigned char circus;
    unsigned char entertainment;
#if PLATFORM_WINDOWS
    unsigned char terrain_flag;
#endif
    unsigned char building_kind;
    unsigned char range;

#if PLATFORM_WINDOWS
    terrain_flag = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).terrain;
#endif
    building_kind = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind;
    range = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).entertainment;
    theatre = range & 3;
    colosseum = (range & 0xc) >> 2;
    circus = (range & 0x30) >> 4;
    entertainment = theatre + colosseum + circus;
    if (building_kind >= 0xe5 && building_kind <= 0xf0) { landfill_pool[cm_dptr] = 0x96; return; }
    if (entertainment == 0) landfill_pool[cm_dptr] = 0;
    else landfill_pool[cm_dptr] = (entertainment - 1) * 3 + 0x7e;
}

// Overlay routine for the industry/market view: 0x96 for market and large-industry footprints
// (kind 0xFC..0xFF or 0xFA); otherwise reads the 2-bit industry coverage from cell.range_flag bits
// 6..7 -- 0x40 -> 0x93, 0x80 -> 0x90, 0xC0 -> 0x8D.
// FUNCTION: C2 0x3ee33
// FUNCTION: C2WIN 0x0049da87
void get_industry_ov_image(void)
{
    unsigned char industry;
#if PLATFORM_WINDOWS
    unsigned char terrain_flag;
#endif
    unsigned char building_kind;
#if PLATFORM_WINDOWS
    terrain_flag = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).terrain;
#endif
    building_kind = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind;
    industry = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).range_flag & 0xc0;
    if (building_kind >= 0xfc && building_kind <= 0xff) landfill_pool[cm_dptr] = 0x96;
    else if (building_kind == 0xfa) landfill_pool[cm_dptr] = 0x96;
    else if (industry != 0) {
        if (industry == 0x40) landfill_pool[cm_dptr] = 0x93;
        else if (industry == 0x80) landfill_pool[cm_dptr] = 0x90;
        else landfill_pool[cm_dptr] = 0x8d;
    } else landfill_pool[cm_dptr] = 0;
}

// Overlay routine for the unrest view: reads cell.fpu_flag bits 0..3, bucketing 1..4 -> 0x79
// (mild), 5..0xA -> 0x78 (moderate), >= 0xB -> 0x77 (severe). Zero is the cleared default.
// FUNCTION: C2 0x3eebe
// FUNCTION: C2WIN 0x0049db80
void get_unrest_ov_image(void)
{
    unsigned char flags;
    unsigned char unrest;

    flags = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).fpu_flag;
    unrest = flags & 0xf;
    if (unrest == 0) {
        landfill_pool[cm_dptr] = 0;
    } else if (unrest >= 0xb) {
        landfill_pool[cm_dptr] = 0x77;
    } else if (unrest >= 5) {
        landfill_pool[cm_dptr] = 0x78;
    } else {
        landfill_pool[cm_dptr] = 0x79;
    }
}

// Select the geography-overlay sprite for the current region cell and clear bit 1 of its gfx flags.
// FUNCTION: C2 0x3ef16
// FUNCTION: C2WIN 0x0049dc0e
int get_reg_geog_ov_image(void)
{
    unsigned char map_kind;
    unsigned char edge_bits;

    map_kind = ((unsigned char *)region_map)[cm_sptr];
    edge_bits = ((unsigned char *)region_map)[cm_sptr + 3] & 2;
    ((unsigned char *)region_map)[cm_sptr + 3] &= 0xfd;
    if (edge_bits != 0) sprite_image_no = 0x93;
    else if (map_kind < 4) sprite_image_no = 0xb4;
    else if (map_kind < 8) sprite_image_no = 0xb5;
    else if (map_kind < 0xc) sprite_image_no = 0xb6;
    else if (map_kind < 0x10) sprite_image_no = 0xb7;
    else if (map_kind < 0x20) sprite_image_no = (map_kind & 7) + 0x10;
    else if (map_kind < 0x7c) sprite_image_no = (map_kind & 7) + 0xb8;
    else if (map_kind >= 0x7d && map_kind < 0x85) sprite_image_no = (map_kind & 7) + 0xc0;
    else if (map_kind >= 0x85 && map_kind < 0x8d) sprite_image_no = (map_kind & 3) + 0xc8;
    else if (map_kind >= 0x8d && map_kind < 0x91) sprite_image_no = (map_kind & 3) + 0xd1;
    else if (map_kind == 0x91) sprite_image_no = 0xd5;
    else if (map_kind == 0x92) sprite_image_no = 0xcc;
    else if (map_kind >= 0x93 && map_kind <= 0x96) sprite_image_no = 0x48;
    else if (map_kind >= 0x97 && map_kind <= 0x9b) sprite_image_no = 0x4a;
    else if (map_kind >= 0x9c && map_kind <= 0x9f) sprite_image_no = 0xb8;
    else if (map_kind >= 0xa0 && map_kind <= 0xaa) sprite_image_no = lf_tiles[map_kind - 0x4e];
    else if (map_kind >= 0xb7 && map_kind <= 0xc0) sprite_image_no = lf_tiles[map_kind + 0xa];
    else if (map_kind == 0xd5) sprite_image_no = 0xce;
    else if (map_kind >= 0xdc && map_kind <= 0xe7) sprite_image_no = 0xcf;
    else if (map_kind >= 0xe8 && map_kind <= 0xeb) sprite_image_no = 0xd0;
    else if (map_kind >= 0xec && map_kind <= 0xef) sprite_image_no = 0xcd;
    else sprite_image_no = 0x96;
    return 0;
}

// Draw the city or region overlay for the active map mode.
// FUNCTION: C2 0x3f187
void show_landfill(int x_start, int y_start)
{
    if (map_mode == 0) {
        show_city_landfill(x_start, y_start);
        return;
    }
    if (map_mode == 1) {
        show_region_landfill(x_start, y_start);
    }
}

// Draw the 80x80 city overlay, selecting each cell's sprite and footprint-sized blitter.
// FUNCTION: C2 0x3f1ac
// FUNCTION: C2WIN 0x0049e068
void show_city_landfill(int x_start, int y_start)
{
    unsigned char building_kind;
    int block_idx;
    unsigned char activity_flag;
    unsigned char overlay_flag;
    unsigned char terrain_flag;
    int pool_idx;
    unsigned char in_range;

    sprite_y = y_start * screen_width;
    cm_y = 0;
    pool_idx = 0;
    cm_sptr = 0;
    for ( ; cm_y < 80; cm_y++, sprite_y += 0x500) {
    sprite_x = x_start;
    cm_x = 0;
    do {
        building_kind = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind;
        activity_flag = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a & 0xf;
        terrain_flag = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).terrain & 1;
        if (building_kind >= 0x82 && building_kind <= 0xa1) in_range = 1;
        else in_range = 0;
        overlay_flag = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).edge_bits & 2;
        (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).edge_bits &= 0xfd;
        block_idx = 0;
        if (overlay_flag != 0) {
            sprite_image_no = 0x93;
        } else if (landfill_pool[pool_idx] != 0) {
            sprite_image_no = (unsigned char)landfill_pool[pool_idx];
            if (in_range) {
                if (sprite_image_no >= 0x7e) sprite_image_no += 2;
            } else if (terrain_flag != 0) {
                sprite_image_no++;
            }
        } else {
                if (activity_flag != 0 && terrain_flag != 0) goto next_cell;
                sprite_image_no = lf_tiles[building_kind];
                if (building_kind >= 0xe9 && building_kind <= 0xf0) get_circus_bodge(building_kind);
                if (sprite_image_no < 0x5a) block_idx = 0;
                else if (sprite_image_no < 0x63) block_idx = 1;
                else if (sprite_image_no < 0x71) block_idx = 2;
                else block_idx = 3;
            }
            sprite_start = landfill[sprite_image_no * 16 + 0xc]
                       + (landfill[sprite_image_no * 16 + 0xd] << 8);
            if (block_idx == 0) place_2x2_block(landfill + sprite_start, sprite_x + sprite_y);
            else if (block_idx == 1) place_4x4_block(landfill + sprite_start, sprite_x + sprite_y);
            else if (block_idx == 2) place_6x6_block(landfill + sprite_start, sprite_x + sprite_y);
            else if (block_idx == 3) place_8x8_block(landfill + sprite_start, sprite_x + sprite_y);
next_cell:
            cm_x++;
            pool_idx++;
            cm_sptr += 20;
        sprite_x += 2;
    } while (cm_x < 80);
    }
    write_image(misc, 4, x_start + 2, y_start + 2);
}

// Select the individual circus-footprint sprite when activity_b bit 5 is clear.
// FUNCTION: C2 0x3f3cb
// FUNCTION: C2WIN 0x0049e741
void get_circus_bodge(unsigned char building_kind)
{
    unsigned char activity_flag = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_b & 0x20;
    if (activity_flag != 0) return;
    if (building_kind == 0xe9) sprite_image_no = 0x6d;
    if (building_kind == 0xea) sprite_image_no = 0x6e;
    if (building_kind == 0xeb) sprite_image_no = 0x6f;
    if (building_kind == 0xec) sprite_image_no = 0x70;
    if (building_kind == 0xed) sprite_image_no = 0x72;
    if (building_kind == 0xee) sprite_image_no = 0x73;
    if (building_kind == 0xef) sprite_image_no = 0x74;
    if (building_kind == 0xf0) sprite_image_no = 0x75;
}

// Draw the 60x60 region overlay, skipping hidden footprint cells and coastal large sprites.
// FUNCTION: C2 0x3f493
// FUNCTION: C2WIN 0x0049e844
void show_region_landfill(int x_start, int y_start)
{
    unsigned char corner_flags;
    unsigned char region_kind;
    unsigned char region_gfx_idx;
    int block_idx;

    sprite_y = y_start * screen_width;
    cm_y = 0; cm_sptr = 0;
    for ( ; cm_y < 60; cm_y++, sprite_y += 0x500) {
    sprite_x = x_start;
    cm_x = 0;
    do {
        corner_flags = (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).occupant & 3;
        region_kind = (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).base_kind;
        region_gfx_idx = (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).gfx;
        if (region_kind == 0x92 && (region_gfx_idx & 3) != 0) goto reg_next;
        if (region_kind >= 0x85 && region_kind < 0x8d && (region_gfx_idx & 3) != 0) goto reg_next;
        if (region_kind == 0x8d && region_gfx_idx != 0x28) goto reg_next;
        if (region_kind == 0x8e && region_gfx_idx != 0x31) goto reg_next;
        if (region_kind == 0x8f && region_gfx_idx != 0x3a) goto reg_next;
        if (region_kind == 0x90 && region_gfx_idx != 0x43) goto reg_next;
        if (region_kind == 0x91 && region_gfx_idx != 0x4c) goto reg_next;
        get_reg_geog_ov_image();
        if (sprite_image_no < 0xc8) block_idx = 0;
        else if (sprite_image_no < 0xd1) block_idx = 1;
        else if (sprite_image_no < 0xd5) block_idx = 2;
        else block_idx = 3;
        if (block_idx > 0 && corner_flags != 0) goto reg_next;
        sprite_start = landfill[sprite_image_no * 16 + 0xc]
                     + (landfill[sprite_image_no * 16 + 0xd] << 8);
        if (block_idx == 0) place_2x2_block(landfill + sprite_start, sprite_x + sprite_y);
        else if (block_idx == 1) place_4x4_block(landfill + sprite_start, sprite_x + sprite_y);
        else if (block_idx == 2) place_6x6_block(landfill + sprite_start, sprite_x + sprite_y);
        else if (block_idx == 3) place_8x8_block(landfill + sprite_start, sprite_x + sprite_y);
reg_next:
        cm_x++;
        cm_sptr += 8;
        sprite_x += 2;
    } while (cm_x < 60);
    }
    write_image(misc, 4, x_start + 2, y_start + 2);
}

// Draw a row range of the battle terrain overlay, including occupied-cell highlights.
// FUNCTION: C2 0x3f6b6
// FUNCTION: C2WIN 0x0049ef17
void show_battle_landfill(int start_row, int row_count, int screen_x, int screen_y)
{
    int col_idx;
    int battle_offset;
    int row_idx;
    unsigned char terrain_kind;

    battle_offset = start_row * BATTLE_ROW;
    sprite_y = (screen_y + start_row * 2) * screen_width;
    row_idx = start_row;
    for ( ; row_idx < (start_row + row_count); row_idx++, sprite_y += 0x500) {
    sprite_x = screen_x;
    col_idx = 0;
    do {
        terrain_kind = ((unsigned char *)battle_map)[battle_offset];
        temp_figure = ((unsigned char *)battle_map)[battle_offset + 1];
        sprite_image_no = (terrain_kind & 7) + 0x10;
        if (temp_figure != 0) {
            if (figure_list[temp_figure].state_idx == 2) goto battle_next;
            if (figure_list[temp_figure].owner != 0) sprite_image_no = 0x8a;
            else sprite_image_no = 0x93;
        }
        sprite_start = landfill[sprite_image_no * 16 + 0xc]
                     + (landfill[sprite_image_no * 16 + 0xd] << 8);
        place_2x2_block(landfill + sprite_start, sprite_x + sprite_y);
battle_next:
        col_idx++;
        battle_offset += BATTLE_CELL_BYTES;
        sprite_x += 2;
    } while (col_idx < BATTLE_W);
    }

    setup_refresh_area(screen_x, screen_y + (start_row * 2), 8, ((start_row * 2) / 16) + 2, 1);
}


// Advance and draw the eleven-step battle terrain animation.
// FUNCTION: C2 0x3f7f5
// FUNCTION: C2WIN 0x0049f217
void update_battle_landfill(void)
{
    ++cmu_count[3];
    if (cmu_count[3] > 10) {
        cmu_count[3] = 0;
    }
    if (cmu_count[3] < 10) {
        show_battle_landfill(cmu_count[3] * 5, 5, 0xb1, 0x170);
    } else {
        show_battle_landfill(50, 2, 0xb1, 0x170);
    }
}

// Draw a terrain-coloured region mini-map and mark the selected cohort army.
// FUNCTION: C2 0x3f854
// FUNCTION: C2WIN 0x0049f284
void show_cohort_landfill(int army_idx, int x_start, int y_start)
{
    int terrain_colour;
    unsigned char terrain_kind;

    cm_y = 0;
    cm_sptr = 0;
    for ( ; cm_y < 60; cm_y++) {
    cm_x = 0;
    do {
        terrain_kind = (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).base_kind;
            if (terrain_kind < 0x20) terrain_colour = 2;
            else if (terrain_kind < 0x7c) terrain_colour = 1;
            else if (terrain_kind >= 0x7d && terrain_kind < 0x85) terrain_colour = 0x30;
            else if (terrain_kind >= 0x85 && terrain_kind < 0x8d) terrain_colour = 0x2a;
            else if (terrain_kind >= 0x8d && terrain_kind < 0x91) terrain_colour = 0x24;
            else if (terrain_kind == 0x91) terrain_colour = 0x20;
            else terrain_colour = 2;
            draw_a_point(x_start + cm_x, y_start + cm_y, terrain_colour);
            cm_x++;
        cm_sptr += 8;
    } while (cm_x < 60);
    }
    draw_a_rect(x_start + army_list[army_idx].x - 1, y_start + army_list[army_idx].y - 2, 3, 1, 0x3f);
    draw_a_rect(x_start + army_list[army_idx].x - 1, y_start + army_list[army_idx].y + 2, 3, 1, 0x3f);
    draw_a_rect(x_start + army_list[army_idx].x - 2, y_start + army_list[army_idx].y - 1, 1, 3, 0x3f);
    draw_a_rect(x_start + army_list[army_idx].x + 2, y_start + army_list[army_idx].y - 1, 1, 3, 0x3f);
    draw_a_rect(x_start + army_list[army_idx].x - 1, y_start + army_list[army_idx].y - 1, 3, 3, 3);
}

extern void get_no_ov_image(void);
extern void get_landval_ov_image(void);
extern void get_water_ov_image(void);
extern void get_security_ov_image(void);
extern void get_unrest_ov_image(void);
extern void get_admin_ov_image(void);
extern void get_entertainment_ov_image(void);
extern void get_education_ov_image(void);
extern void get_health_ov_image(void);
extern void get_industry_ov_image(void);

void (*ov_routines[10])(void) = {
    get_no_ov_image,
    get_landval_ov_image,
    get_water_ov_image,
    get_security_ov_image,
    get_unrest_ov_image,
    get_admin_ov_image,
    get_entertainment_ov_image,
    get_education_ov_image,
    get_health_ov_image,
    get_industry_ov_image
};
