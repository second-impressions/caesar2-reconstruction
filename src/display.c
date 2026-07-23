
#include <stdio.h>      /* printf */
#include <stdlib.h>     /* exit   */

#include "c2_data.h"
#include "c2_types.h"     /* struct request_message */

extern void place_32x32_block(unsigned char *panel_data_ptr);
extern void place_16x16_block(unsigned char *panel_data_ptr);
extern int  depress(unsigned char *destination_ptr, unsigned char *source_ptr);
extern void copy(unsigned char *source_ptr, unsigned char *destination_ptr, int byte_count);
extern int convert_lbm_file(unsigned char *source_ptr, unsigned char *destination_ptr, char *palette_ptr, int byte_count);
extern void place_i_sprite(unsigned char *sprite_data_ptr);  /* sprites.asm */
extern void write_i_sprite(unsigned char *sprite_data_ptr);   /* sprites.asm */
extern void write_i_left_sprite(unsigned char *sprite_data_ptr);
extern void write_i_right_sprite(unsigned char *sprite_data_ptr);

/* Forward declarations (functions defined later in this file). */
void show_a_mosaic_frame(int window_x, int window_y, int column_count, int row_count);
void show_a_mosaic_blank(int window_x, int window_y, int column_count, int row_count);


// Load rows of PL8 pixels into the internal screen, skipping the file header.
// FUNCTION: C2 0x5a25c
// FUNCTION: C2WIN 0x0045f920
void show_pl8file(const char *filename, int row_count)
{
    if (readfile(filename, internal_screen,
                 screen_width * row_count, 0x18) == 0) {
        test_beeps();
        return;
    }
    flush_sb_buffer();
}

// Load a full-screen PL8 image and palette, display it, and fade the palette in.
// FUNCTION: C2 0x5a28b
// FUNCTION: C2WIN 0x0045f960
int display_pl8file(char *pl8_filename, char *palette_filename)
{
    if (readfile(pl8_filename, internal_screen, 0x4b000, 0x18) == 0)
        return 0;
    if (readfile(palette_filename, &temp_palette, 0x300, 0) == 0)
        return 0;
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
    refresh_svga_screen();
    fade_to_palette(temp_palette);
    return 1;
}

// Load and decode an LBM image into the internal screen.
// FUNCTION: C2 0x5a2dd
// FUNCTION: C2WIN 0x0045f9d6
void show_lbm(const char *filename)
{
    int file_size;

    file_size = readfile(filename, ((void *)scratch_buffer),
                  scratch_buffer_size, 0);
    if (file_size >= scratch_buffer_size) {
        no_high_beeps(1);
        stop_system();
        printf("Exit from c2 tutorial mode .lbm file too large.\n");
        exit(100);
    }
    if (file_size != 0) {
        convert_lbm_file(scratch_buffer, internal_screen, temp_palette, file_size);
    }
    flush_sb_buffer();
}

// Load a compressed PIC image into the internal screen without changing the palette.
// FUNCTION: C2 0x5a345
// FUNCTION: C2WIN 0x0045fa67
void show_picfile(char *filename)
{
    if (readfile(filename, ((void *)scratch_buffer), 0x4e200, 0) == 0) {
        test_beeps();
        return;
    }
    if (evacuate(((scratch_buffer) + 2), internal_screen) > 0x4e200) {
        test_beeps();
        return;
    }
    if (depress(internal_screen, scratch_buffer) > 0x4e200) {
        test_beeps();
        return;
    }
    copy(scratch_buffer + 0x300, internal_screen, screen_size);
    flush_sb_buffer();
}

// Load a compressed PIC image and install its embedded palette.
// FUNCTION: C2 0x5a3bc
// FUNCTION: C2WIN 0x0045fb16
void display_picfile(char *filename)
{
    if (readfile(filename, ((void *)scratch_buffer), 0x4e200, 0) == 0) {
        test_beeps();
        return;
    }
    if (evacuate(((scratch_buffer) + 2), internal_screen) > 0x4e200) {
        test_beeps();
        return;
    }
    if (depress(internal_screen, scratch_buffer) > 0x4e200) {
        test_beeps();
        return;
    }
    copy(scratch_buffer + 0x300, internal_screen, screen_size);
    set_palette(((char *)scratch_buffer));
    flush_sb_buffer();
}

// Draw a tiled system window with corners, edges, and an interior.
// FUNCTION: C2 0x5a43d
// FUNCTION: C2WIN 0x0045fbd3
void show_a_system_window(int window_x, int window_y, int column_count, int row_count)
{
    int row;
    int column;

    for (row = 0; row < row_count; row++) {
        for (column = 0; column < column_count; column++) {
            if (row == 0)
                sprite_image_no = 0;
            else if (row == row_count - 1)
                sprite_image_no = 6;
            else
                sprite_image_no = 3;
            if (column == 0)
                goto draw_system_tile;
            if (column == column_count - 1)
                sprite_image_no += 2;
            else
                sprite_image_no++;
draw_system_tile:
            sprite_x = window_x + column * 16;
            sprite_y = window_y + row * 16;
            place_16x16_block(system_panel);
        }
    }
}

// Fill a rectangular area with the system window's interior tile.
// FUNCTION: C2 0x5a4cf
// FUNCTION: C2WIN 0x0045fcb7
void show_a_system_blank(int window_x, int window_y, int column_count, int row_count)
{
    int row_idx;
    int column_idx;
    for (row_idx = 0; row_idx < row_count; row_idx++) {
        for (column_idx = 0; column_idx < column_count; column_idx++) {
            sprite_image_no = 4;
            sprite_x = window_x + column_idx * 16;
            sprite_y = window_y + row_idx * 16;
            place_16x16_block(system_panel);
        }
    }
}


// Draw a deterministic stone mosaic window with a frame and tiled interior.
// FUNCTION: C2 0x5a523
// FUNCTION: C2WIN 0x0045fd38
void show_a_mosaic_window(int window_x, int window_y, int column_count, int row_count)
{
    stone_random_count = 0xa;
    show_a_mosaic_frame(window_x, window_y, column_count, row_count);
    show_a_mosaic_blank(window_x + 16, window_y + 16, column_count - 2, row_count - 2);
}

// Draw the corner and edge tiles of a stone mosaic frame.
// FUNCTION: C2 0x5a559
// FUNCTION: C2WIN 0x0045fd86
void show_a_mosaic_frame(int window_x, int window_y, int column_count, int row_count)
{
    int row;
    int column;

    for (row = 0; row < row_count; row++) {
        for (column = 0; column < column_count; column++) {
            if (stone_random_count++ >= 0x40)
                stone_random_count = 0;

            if (row == 0 && column == 0) {
                sprite_image_no = 0;
            } else if (row == row_count - 1 && column == 0) {
                sprite_image_no = 3;
            } else if (row == 0 && column == column_count - 1) {
                sprite_image_no = 1;
            } else if (row == row_count - 1 && column == column_count - 1) {
                sprite_image_no = 2;
            } else if (row == 0) {
                sprite_image_no =
                    stone_random_data[stone_random_count] / 2 + 4;
            } else if (row == row_count - 1) {
                sprite_image_no =
                    stone_random_data[stone_random_count] / 2 + 0xc;
            } else if (column == 0) {
                sprite_image_no =
                    stone_random_data[stone_random_count] / 2 + 0x14;
            } else if (column == column_count - 1) {
                sprite_image_no =
                    stone_random_data[stone_random_count] / 2 + 0x1c;
            } else {
                continue;
            }
            sprite_x = window_x + column * 16;
            sprite_y = window_y + row * 16;
            place_16x16_block(game_panels);
        }
    }
}

// Draw a horizontal stone divider with capped ends.
// FUNCTION: C2 0x5a6d5
// FUNCTION: C2WIN 0x0045ff6a
void mosaic_frame_divider(int divider_x, int divider_y, int column_count, int unused_row_count)
{
    int column_idx;

    (void)unused_row_count;
    for (column_idx = 0; column_idx < column_count; column_idx++) {
        if (stone_random_count++ >= 0x40)
            stone_random_count = 0;
        if (column_idx == 0)
            sprite_image_no = 0x4d;
        else if (column_idx == column_count - 1)
            sprite_image_no = 0x4e;
        else
            sprite_image_no =
                stone_random_data[stone_random_count] / 2 + 4;
        sprite_x = divider_x + column_idx * 16;
        sprite_y = divider_y;
        place_16x16_block(game_panels);
    }
}

// Fill a mosaic interior with a deterministic sequence of stone tiles.
// FUNCTION: C2 0x5a774
// FUNCTION: C2WIN 0x0046002d
void show_a_mosaic_blank(int window_x, int window_y, int column_count, int row_count)
{
    int row;
    int column;

    for (row = 0; row < row_count; row++) {
        for (column = 0; column < column_count; column++) {
            if (stone_random_count++ >= 0x40)
                stone_random_count = 0;
            sprite_image_no = stone_random_data[stone_random_count] + 0x24;
            sprite_x = window_x + column * 16;
            sprite_y = window_y + row * 16;
            place_16x16_block(game_panels);
        }
    }
}

// Draw one 32-by-32 game-panel sprite at the requested position.
// FUNCTION: C2 0x5a7f7
// FUNCTION: C2WIN 0x004600de
void show_a_32_block(int sprite_x_pos, int sprite_y_pos, int sprite_idx)
{
    sprite_image_no = sprite_idx;
    sprite_x        = sprite_x_pos;
    sprite_y        = sprite_y_pos;
    place_32x32_block(game_panels);
}

// Read a sprite's dimensions from its header in the scratch buffer.
// FUNCTION: C2 0x5a812
// FUNCTION: C2WIN 0x0046010e
void get_general_sprite_sizes(int sprite_idx)
{
    unsigned char *header_ptr;

    data_ptr = sprite_idx * 16 + 8;
    header_ptr = (scratch_buffer) + data_ptr;
    sprite_width  = header_ptr[0] + (header_ptr[1] << 8);
    sprite_height = header_ptr[2] + (header_ptr[3] << 8);
}

// Validate and draw an unclipped sprite from the scratch buffer.
// FUNCTION: C2 0x5a858
// FUNCTION: C2WIN 0x00460180
void general_sprite(int sprite_idx, int sprite_x_pos, int sprite_y_pos)
{
    unsigned char *sprite_data_ptr;
    unsigned char *header_ptr;

    data_ptr = sprite_idx * 16 + 8;
    sprite_image_no = sprite_idx;
    sprite_data_ptr = (scratch_buffer);
    header_ptr = sprite_data_ptr + data_ptr;
    sprite_width  = header_ptr[0] + (header_ptr[1] << 8);
    sprite_height = header_ptr[2] + (header_ptr[3] << 8);
    sprite_start  = header_ptr[4] + (header_ptr[5] << 8) + (header_ptr[6] << 16);
    if (sprite_start > 0x4baf0) return;
    if (sprite_width <= 0)      return;
    if (sprite_width > 0x280)   return;
    if (sprite_height <= 0)     return;
    if (sprite_height > 0x1e0)  return;
    sprite_x = sprite_x_pos;
    sprite_y = sprite_y_pos;
    x_wrap = 0x280 - sprite_width;
    place_i_sprite(sprite_data_ptr);
}

// Validate, clip, and draw a sprite from the scratch buffer.
// FUNCTION: C2 0x5a915
// FUNCTION: C2WIN 0x004602ce
void write_general_sprite(int sprite_idx, int sprite_x_pos, int sprite_y_pos)
{
    unsigned char *header_ptr;

    data_ptr = sprite_idx * 16 + 8;
    sprite_image_no = sprite_idx;
    header_ptr = (scratch_buffer) + data_ptr;
    sprite_width  = header_ptr[0] + (header_ptr[1] << 8);
    sprite_height = header_ptr[2] + (header_ptr[3] << 8);
    sprite_start  = header_ptr[4] + (header_ptr[5] << 8) + (header_ptr[6] << 16);
    if (sprite_start > 0x4baf0) return;
    if (sprite_width <= 0)      return;
    if (sprite_width > 0x280)   return;
    if (sprite_height <= 0)     return;
    if (sprite_height > 0x1e0)  return;
    sprite_x = sprite_x_pos;
    sprite_y = sprite_y_pos;
    x_wrap = 0x280 - sprite_width;
    xclip(0, screen_width);
    yclip(0, screen_height);
    if (yclipped == 5) return;
    if (xclipped == 1) {
        write_i_left_sprite(scratch_buffer);
        return;
    }
    if (xclipped == 2) {
        write_i_right_sprite(scratch_buffer);
        return;
    }
    write_i_sprite(scratch_buffer);
}

// Draw a clipped sprite after omitting the requested number of top rows.
// FUNCTION: C2 0x5aa34
// FUNCTION: C2WIN 0x0046049b
void write_general_sprite_with_front_ofset(int sprite_idx, int sprite_x_pos, int sprite_y_pos, int front_offset)
{
    unsigned char *header_ptr;

    data_ptr = sprite_idx * 16 + 8;
    sprite_image_no = sprite_idx;
    header_ptr = (scratch_buffer) + data_ptr;
    sprite_width  = header_ptr[0] + (header_ptr[1] << 8);
    sprite_height = header_ptr[2] + (header_ptr[3] << 8);
    sprite_start  = header_ptr[4] + (header_ptr[5] << 8) + (header_ptr[6] << 16);
    if (sprite_start > 0x4baf0) return;
    if (sprite_width <= 0)      return;
    if (sprite_width > 0x280)   return;
    if (sprite_height <= 0)     return;
    if (sprite_height > 0x1e0)  return;
    sprite_x = sprite_x_pos;
    sprite_y = sprite_y_pos;
    sprite_y      = sprite_y_pos + front_offset;
    sprite_height = sprite_height - front_offset;
    sprite_start += front_offset * sprite_width;
    x_wrap = 0x280 - sprite_width;
    xclip(0, screen_width);
    yclip(0, screen_height);
    if (yclipped == 5) return;
    if (xclipped == 1) {
        write_i_left_sprite(scratch_buffer);
        return;
    }
    if (xclipped == 2) {
        write_i_right_sprite(scratch_buffer);
        return;
    }
    write_i_sprite(scratch_buffer);
}

// Restore a saved screen region using the position stored in its sprite header.
// FUNCTION: C2 0x5ab70
// FUNCTION: C2WIN 0x0046068e
void restore_picture_part(unsigned char *sprite_data_ptr, int sprite_idx)
{
    data_ptr = sprite_idx * 16 + 8;
    sprite_width  = sprite_data_ptr[data_ptr] + (sprite_data_ptr[data_ptr + 1] << 8);
    sprite_height = sprite_data_ptr[data_ptr + 2] + (sprite_data_ptr[data_ptr + 3] << 8);
    sprite_start  = sprite_data_ptr[data_ptr + 4] + (sprite_data_ptr[data_ptr + 5] << 8) + (sprite_data_ptr[data_ptr + 6] << 16);
    if (sprite_start > 0x4baf0) return;
    if (sprite_width <= 0) return;
    if (sprite_width > 0x280) return;
    if (sprite_height <= 0) return;
    if (sprite_height > 0x1e0) return;
    sprite_x = sprite_data_ptr[data_ptr + 8] + (sprite_data_ptr[data_ptr + 9] << 8);
    sprite_y = sprite_data_ptr[data_ptr + 10] + (sprite_data_ptr[data_ptr + 11] << 8);
    x_wrap = 0x280 - sprite_width;
    place_i_sprite(sprite_data_ptr);
}

// Load and draw one rectangular component of the city interface.
// FUNCTION: C2 0x5ac44
// FUNCTION: C2WIN 0x004607fb
void draw_city_map_part(int part_idx)
{
    int file_offset;

    file_offset = int_city_header[part_idx * 8 + 6];
    file_offset += (int_city_header[part_idx * 8 + 7]) << 16;
    sprite_start  = 0;
    sprite_width  = int_city_header[part_idx * 8 + 4];
    sprite_height = int_city_header[part_idx * 8 + 5];
    sprite_x      = int_city_header[part_idx * 8 + 8];
    if (part_idx >= 4) {
        sprite_x += 0xee;
    }
    sprite_y      = int_city_header[part_idx * 8 + 9];
    readfile("int_city.pl8", ((void *)scratch_buffer), sprite_width * sprite_height, file_offset);
    x_wrap = 0x280 - sprite_width;
    place_i_sprite(scratch_buffer);
}

// Load and draw one rectangular component of the region interface.
// FUNCTION: C2 0x5acf9
// FUNCTION: C2WIN 0x004608f1
void draw_region_map_part(int part_idx)
{
    int file_offset;

    file_offset = int_region_header[part_idx * 8 + 6];
    file_offset += (int_region_header[part_idx * 8 + 7]) << 16;
    sprite_start  = 0;
    sprite_width  = int_region_header[part_idx * 8 + 4];
    sprite_height = int_region_header[part_idx * 8 + 5];
    sprite_x      = int_region_header[part_idx * 8 + 8];
    if (part_idx >= 4) {
        sprite_x += 0xee;
    }
    sprite_y      = int_region_header[part_idx * 8 + 9];
    readfile("int_prov.pl8", ((void *)scratch_buffer), sprite_width * sprite_height, file_offset);
    x_wrap = 0x280 - sprite_width;
    place_i_sprite(scratch_buffer);
}

// Load and draw one rectangular component of the battle interface.
// FUNCTION: C2 0x5ad8d
// FUNCTION: C2WIN 0x004609e7
void draw_battle_part(int header_idx)
{
    int part_idx = header_idx;
    int file_offset;

    header_idx *= 8;
    file_offset = int_battle_header[header_idx + 6];
    file_offset += (int_battle_header[header_idx + 7]) << 16;
    sprite_start  = 0;
    sprite_width  = int_battle_header[header_idx + 4];
    sprite_height = int_battle_header[header_idx + 5];
    sprite_x      = int_battle_header[header_idx + 8];
    sprite_y      = int_battle_header[header_idx + 9];
    if (part_idx >= 4) sprite_y += 0xc8;
    readfile("int_batl.pl8", ((void *)scratch_buffer), sprite_width * sprite_height, file_offset);
    x_wrap = 0x280 - sprite_width;
    place_i_sprite(scratch_buffer);
}

// Blink either the insertion bar or underline caret at the text cursor.
// FUNCTION: C2 0x5ae3e
// FUNCTION: C2WIN 0x00460add
void show_cursor(unsigned char *font_ptr)
{
    int letter_width;

    request_message.caret_count++;
    if (request_message.caret_count > 0x10) request_message.caret_count = 0;
    if (cursor_y == 0) return;
    letter_width = get_letter_width((signed char)format_buffer[this_letter], font_ptr);
    if (request_message.caret_count <= 8) return;
    if (insert_cursor != 0) {
        draw_a_rect(cursor_x - 2, cursor_y - 2, 1, 0xf, 3);
        draw_a_rect(cursor_x - 4, cursor_y - 3, 2, 1, 3);
        draw_a_rect(cursor_x - 4, cursor_y + 0xe, 2, 1, 3);
        draw_a_rect(cursor_x - 1, cursor_y - 3, 2, 1, 3);
        draw_a_rect(cursor_x - 1, cursor_y + 0xe, 2, 1, 3);
    } else {
        draw_a_rect(cursor_x - 1, cursor_y + 0xe, letter_width + 2, 2, 3);
    }
}

// Play a mouse-skippable Smacker animation in VGA mode, then restore the SVGA game display.
// FUNCTION: C2 0x5af5d
// FUNCTION: C2WIN 0x00460c28
void do_vga_smacked_anim(char *filename)
{
    if (check_file_exists(filename) == 0) return;
    black_out();
    wvbl2();
    clear_a_screen();
    clear_all_screens();
    setup_whole_screen_refresh();
    refresh_svga_screen();
    set_bank(0);
    screen_mode = 1;
    set_vga_mode(0x13);
    screen_width  = 0x140;
    screen_height = 0xc8;
    screen_size   = 0xfa00;
    clear_map_gfx_buffers();
    clear_battle_gfx_buffers();
    start_smacking(filename, 0, 0x18, 2);
    if (are_smacking()) {
        out2 = 0;
        out1 = 0;
        while (out1 != 1) {
            hold_hot_keys = 1;
            get_mouse();
            continue_smacking(0, 0x18, 2);
            if (mouse_right_click) {
                out1 = 1;
                out2 = 1;
            }
            if (mouse_left_click) {
                out1 = 1;
                out2 = 1;
            }
            if (!are_smacking()) out1 = 1;
        }
        stop_smacking();
    }
    black_out();
    clear_all_screens();
    screen_mode = 2;
    set_svga_640_480(0);
    clear_a_screen();
    set_mouse_limits();
    init_map_gfx_buffers();
    init_battle_gfx_buffers();
}

// Play a mouse-skippable Smacker animation in SVGA mode and restore the graphics buffers.
// FUNCTION: C2 0x5b0a0
// FUNCTION: C2WIN 0x00460db5
void do_svga_smacked_anim(char *filename)
{
    int saved_anims_on;

    saved_anims_on = c2inf.anims_on;
    c2inf.anims_on = 1;
    clear_map_gfx_buffers();
    clear_battle_gfx_buffers();
    start_smacking(filename, 0, 0, 0);
    out2 = 0;
    out1 = 0;
    while (out1 != 1) {
        hold_hot_keys = 1;
        get_mouse();
        if (continue_smacking(0, 0, 0)) refresh_svga_screen();
        if (mouse_right_preclick) {
            out1 = 1;
            out2 = 1;
        }
        if (mouse_left_preclick) {
            out1 = 1;
            out2 = 1;
        }
        if (!are_smacking()) out1 = 1;
    }
    stop_smacking();
    black_out();
    clear_all_screens();
    clear_a_screen();
    c2inf.anims_on = saved_anims_on;
    init_map_gfx_buffers();
    init_battle_gfx_buffers();
}
