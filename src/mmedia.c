
#include "mmedia.h"
#include "c2_data.h"


extern void put_a_font_string(char *str, int x, int y, unsigned char *font, int color);
extern int  one_letter(unsigned char *font, unsigned char letter);
extern void font_list(int idx, int word_count, int x, int y, unsigned char *font, int color);
extern void font_no(int value, char pad_char, char *suffix, int x, int y, unsigned char *font, int color);
extern int  get_next_word_length(char *src, unsigned char *font);
void media_text_place(int x, int y, int width, int line_count, int alt_x, int alt_width, unsigned char *font);


#include "c2_types.h"

char help_palname[14] = "xxxxxxxx.256";

char active_tutorial_pages[34] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2, 0, 3, 0, 4, 0, 0, 0, 6, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

unsigned char city_tutorial_icons[30] = { 1, 1, 1, 1, 1, 1, 9, 6, 9, 1, 1, 1, 3, 3, 1, 2, 4, 7, 7, 1, 7, 7, 7, 7, 99, 99, 99, 99, 99, 99 };

unsigned char region_tutorial_icons[30] = { 5, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 8, 5, 5, 8, 5, 8, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99 };

struct tutorial_file_rec tut_files[32] = {
    { "tut_01a.pl8" },
    { "tut_01b.pl8" },
    { "tut_02a.pl8" },
    { "tut_02b.pl8" },
    { "tut_03a.pl8" },
    { "tut_03b.pl8" },
    { "tut_04a.pl8" },
    { "tut_04b.pl8" },
    { "tut_05a.pl8" },
    { "tut_05b.pl8" },
    { "tut_06a.pl8" },
    { "tut_06b.pl8" },
    { "tut_07a.pl8" },
    { "tut_07b.pl8" },
    { "tut_08a.pl8" },
    { "tut_08b.pl8" },
    { "tut_09a.pl8" },
    { "tut_09b.pl8" },
    { "tut_10a.pl8" },
    { "tut_10b.pl8" },
    { "tut_11a.pl8" },
    { "tut_11b.pl8" },
    { "tut_12a.pl8" },
    { "tut_12b.pl8" },
    { "tut_13a.pl8" },
    { "tut_13b.pl8" },
    { "tut_14a.pl8" },
    { "tut_14b.pl8" },
    { "tut_15a.pl8" },
    { "tut_15b.pl8" },
    { "tut_16a.pl8" },
    { "tut_16b.pl8" }
};

struct tutorial_file_rec tut_palfiles[32] = {
    { "tut_01a.256" },
    { "tut_01b.256" },
    { "tut_02a.256" },
    { "tut_02b.256" },
    { "tut_03a.256" },
    { "tut_03b.256" },
    { "tut_04a.256" },
    { "tut_04b.256" },
    { "tut_05a.256" },
    { "tut_05b.256" },
    { "tut_06a.256" },
    { "tut_06b.256" },
    { "tut_07a.256" },
    { "tut_07b.256" },
    { "tut_08a.256" },
    { "tut_08b.256" },
    { "tut_09a.256" },
    { "tut_09b.256" },
    { "tut_10a.256" },
    { "tut_10b.256" },
    { "tut_11a.256" },
    { "tut_11b.256" },
    { "tut_12a.256" },
    { "tut_12b.256" },
    { "tut_13a.256" },
    { "tut_13b.256" },
    { "tut_14a.256" },
    { "tut_14b.256" },
    { "tut_15a.256" },
    { "tut_15b.256" },
    { "tut_16a.256" },
    { "tut_16b.256" }
};

/* File-local state. */
char media_line_buffer[200];
int greyed_out;
int tutorial_correct_timer;
int media_voc;
int media_right_image;
int this_spot;
int tutorial_level;
int last_tutorial_page;
int tutorial_timer;
int media_left_image;
int tutorial_correct;
int linked_text_flag;
/* Forward declarations (functions defined later in this file). */
void load_media_entry(void);
void show_help_page(void);
void put_a_media_string(char *text, int x, int y);
void push_forward_help_history(void);
void init_help_history(void);
void do_a_tutorial_page(void);
void show_please_wait(void);
void act_back_tutorial_page(void);
void act_middle_tutorial_page(void);
void act_forward_tutorial_page(void);


// Run the in-game help browser from the requested page until the user exits.
// FUNCTION: C2 0x57fa8
// FUNCTION: C2WIN 0x004521a0
void launch_help(int page)
{
    int old_pointer_mode;

    if (page <= 0) return;
    if (page >= 0x7d0) return;

    old_pointer_mode = pointer_mode;
    pointer_mode = 0;
    first_help_page = this_help_page = page;
    init_help_history();
    out3 = 0;
    greyed_out = 0;
    while (out3 != 1) {
        stop_db();
        load_media_entry();
        show_help_page();
        if (media_voc != 0) {
            set_db_sound(this_media_entry.voc_file);
        }
        out2 = 0;
        while (out2 != 1) {
            help_game_loop();
            if (exit_screen() != 0) {
                out3 = out2 = 1;
            }
            if (mouse_right_click != 0) {
                out3 = out2 = 1;
            }
            if (out2 > 1) {
                out2 = out2 - 1;
                continue;
            }
            if (mouse_left_preclick != 0) {
                if (get_linked_page() != 0) {
                    out2 = 1;
                }
            }
        }
    }
    stop_db();
    pointer_mode = old_pointer_mode;
}

// Load the current help entry, its text, and the availability of its optional media.
// FUNCTION: C2 0x580a9
// FUNCTION: C2WIN 0x00452332
void load_media_entry(void)
{
    media_voc = 0;
    media_right_image = 0;
    media_left_image = 0;

    readfile(media_file, &this_media_entry, 0x3a,
             this_help_page * 0x3a + 8);
    readfile(media_file, format_buffer, 0x7d0,
             this_media_entry.text_offset);

    if (my_strcmp(this_media_entry.left_file, "null.pl8", 8) != 0) {
        media_left_image = 1;
    }
    if (my_strcmp(this_media_entry.right_file, "null.pl8", 8) != 0) {
        media_right_image = 1;
    }
    if (my_strcmp(this_media_entry.voc_file, "null.voc", 8) != 0) {
        media_voc = 1;
    }
}

// Render the in-game F1 help dialog.
// FUNCTION: C2 0x58162
// FUNCTION: C2WIN 0x0045241f
void show_help_page(void)
{
    int  text_lines;
    int  text_x;
    int  text_width;
    int  left_loaded;
    int  right_loaded;

    /* Use each illustration's matching .256 palette file. */
    my_strcpy("256", extension, 4);
    my_strcpy(city_palette, temp_palette, 0x300);

    cover_mouse_droppings();
    if (!greyed_out) {
        grey_a_screen();
        greyed_out = 1;
    }

    stone_random_count = 0x11;
    show_a_mosaic_frame(8, 0x20, 0x1d, 0x1b);
    setup_whole_screen_refresh();
    show_a_mosaic_blank(0x18, 0x30, 0x1b, 0x19);
    show_an_exit_button(0x1a8, 0x1a0);
    show_buttons(0x168, 0x1a0, help_buttons, 2);

    text_pointer = format_buffer;

    text_x     = 0x28;
    text_width     = 0x190;
    text_lines = 1;

    /* Optional left illustration ---------------------------------- */
    left_loaded = 0;
    if (media_left_image) {
        left_loaded = readfile(this_media_entry.left_file,
                           ((void *)scratch_buffer), 0x186a0, 0);
        my_strcpy(this_media_entry.left_file, help_palname, 0xd);
        my_strcpy("256", extension, 4);
        put_filename_extension(help_palname);
        readfile(help_palname, temp_palette, 0x300, 0);
        if (left_loaded) {
            int line_capacity;
            general_sprite(this_media_entry.left_sprite, 0x1e, 0x38);
            draw_a_dias(0x1d, 0x37, sprite_width + 2, sprite_height + 2);
            draw_a_dias(0x1c, 0x36, sprite_width + 4, sprite_height + 4);
            text_width -= sprite_width + 8;
            text_x  = sprite_width + 0x28;
            line_capacity     = (sprite_height - 0x18) / 0x10;
            if (line_capacity > text_lines) text_lines = line_capacity;
        }
    }

    /* Optional right illustration --------------------------------- */
    right_loaded = 0;
    if (media_right_image) {
        right_loaded = readfile(this_media_entry.right_file,
                            ((void *)scratch_buffer), 0x186a0, 0);
        my_strcpy(this_media_entry.right_file, help_palname, 0xd);
        my_strcpy("256", extension, 4);
        put_filename_extension(help_palname);
        readfile(help_palname, temp_palette, 0x300, 0);
        if (right_loaded) {
            int line_capacity;
            get_general_sprite_sizes(this_media_entry.right_sprite);
            general_sprite(this_media_entry.right_sprite,
                           0x1b8 - sprite_width + 6, 0x38);
            draw_a_dias(0x1b8 - sprite_width + 5, 0x37,
                        sprite_width + 2, sprite_height + 2);
            draw_a_dias(0x1b8 - sprite_width + 4, 0x36,
                        sprite_width + 4, sprite_height + 4);
            text_width -= sprite_width + 8;
            line_capacity     = (sprite_height - 0x18) / 0x10;
            if (line_capacity > text_lines) text_lines = line_capacity;
        }
    }

    put_a_font_string(text_pointer, text_x, 0x38, font2, 0x10);
    media_text_place(text_x, 0x5a, text_width, text_lines,
                     0x28, 0x190, font1);

    refresh_svga_screen();
    set_palette(temp_palette);
    hold_mouse_replace = 1;
}

// Word-wrap the loaded help or tutorial text and render it across the supplied text regions.
// FUNCTION: C2 0x584a9
// FUNCTION: C2WIN 0x0045296c
void media_text_place(int x, int y, int width, int line_count,
                      int alt_x, int alt_width, unsigned char *font)
{
    int    i;
    int    escape_flag;
    int    line_idx;
    int    current_y;
    int    alt_offset;
    int    loop_active;
    int    line_width;
    int    buffer_pos;
    int    skip_leading;
    signed char character;
    int    character_idx;

    this_spot        = 0;
    linked_text_flag = 0;
    nof_hot_spots    = 0;
    for (i = 0; i < 20; i++) {
        help_page_hot_spots[i].page = 0;
        help_page_hot_spots[i].x1 = help_page_hot_spots[i].x2 = 0;
        help_page_hot_spots[i].x3 = help_page_hot_spots[i].x4 = 0;
        help_page_hot_spots[i].y  = help_page_hot_spots[i].unused = 0;
    }

    /* Skip the entry heading and start at the paragraph text. */
    text_pointer = format_buffer;
    while (*text_pointer > 0) text_pointer++;
    text_pointer++;

    font_screen_limit = 0;
    loop_active = 1;
    line_idx    = 0;
    current_y       = y;
    escape_flag = 0;

    while (loop_active) {
        for (i = 0; i < 200; i++) media_line_buffer[i] = 0;
        line_width  = 0;
        buffer_pos     = 0;
        skip_leading = 1;

        if (escape_flag) {
            alt_offset     = 0;
            escape_flag = 0;
        } else {
            alt_offset = 0;
        }

        while (loop_active && line_width < width - alt_offset) {
            line_width += get_next_word_length(text_pointer, font);
            if (line_width >= width - alt_offset) continue;

            for (character_idx = 0; character_idx < char_count; ) {
                character = *text_pointer++;
                if (skip_leading && character == ' ') {
                    /* drop */
                } else if (character == '$') {
                    escape_flag = 1;
                    line_width  = width;
                    break;
                } else {
                    media_line_buffer[buffer_pos++] = character;
                    skip_leading = 0;
                }
                character_idx++;
            }
            if (*text_pointer == 0) {
                loop_active = 0;
                continue;
            }
        }

        insert_place  = 1;
        x_is          = 0;
        allow_padding = 1;
        put_a_media_string(media_line_buffer, x + alt_offset, current_y);
        line_idx++;
        current_y += 0x12;

        if (line_idx >= line_count) {
            /* Switch over to the continuation rectangle. */
            x     = alt_x;
            width = alt_width;
        }
    }
}

// Render a help-text line and record the clickable regions delimited by link tags.
// FUNCTION: C2 0x58684
// FUNCTION: C2WIN 0x00452c66
void put_a_media_string(char *text, int x, int y)
{
    char character;
    int  character_width;

    sprite_x   = x;
    font_style = 0;
    character = *text;
    if (linked_text_flag)
        help_page_hot_spots[this_spot].x3 = sprite_x;
    while (character != 0) {
        sprite_y = y;
        if (character == '#') {
            if (linked_text_flag) {
                /* Closing tag: write the right edge into either
                 * the primary or the continuation slot, depending
                 * on whether this spot has already wrapped. */
                linked_text_flag = 0;
                if (help_page_hot_spots[this_spot].x2 == 0)
                    help_page_hot_spots[this_spot].x2 = sprite_x;
                else
                    help_page_hot_spots[this_spot].x4 = sprite_x;
            } else {
                /* Opening tag.  Allocate a fresh hotspot, parse
                 * the id digits, and stamp the rectangle origin. */
                text++;
                this_spot = nof_hot_spots;
                help_page_hot_spots[this_spot].page = get_number_from_text(text);
                help_page_hot_spots[this_spot].x1 = sprite_x;
                help_page_hot_spots[this_spot].y = sprite_y - 2;
                nof_hot_spots++;
                linked_text_flag = 1;
                /* Skip the remaining page-number digits. */
                if (help_page_hot_spots[this_spot].page > 9) {
                    if (help_page_hot_spots[this_spot].page <= 99)
                        text += 1;
                    else if (help_page_hot_spots[this_spot].page <= 999)
                        text += 2;
                    else
                        text += 3;
                }
            }
            character = 1;
        }
        if (character >= 0x20) {
            if (linked_text_flag)
                sprite_colour = 0x0d;
            else
                sprite_colour = 0x10;
            character -= 0x20;
            if (letter_table[character] > 0)
                character_width = one_letter(font1, character);
            else
                character_width = 4;
            sprite_x += character_width;
            x_is     += character_width;
        }
        text++;
        character = *text;
    }
    if (linked_text_flag)
        help_page_hot_spots[this_spot].x2 = sprite_x;
    x_is += 4;
}

// Hit-test the current mouse position against the help-screen hot spots.
// FUNCTION: C2 0x58828
// FUNCTION: C2WIN 0x00452f00
int get_linked_page(void)
{
    int i;
    int hotspot_width;
    int hotspot_y;

    for (i = 0; i < nof_hot_spots; i++) {
        hotspot_width   = help_page_hot_spots[i].x2 - help_page_hot_spots[i].x1;
        hotspot_y = help_page_hot_spots[i].y;
        if (mouse_in_area(help_page_hot_spots[i].x1, hotspot_y, hotspot_width, 0x12)) {
            push_forward_help_history();
            this_help_page = help_page_hot_spots[i].page;
            return 1;
        }

        if (help_page_hot_spots[i].x3 == 0) continue;
        hotspot_width = help_page_hot_spots[i].x4 - help_page_hot_spots[i].x3;
        if (mouse_in_area(help_page_hot_spots[i].x3, hotspot_y += 0x12, hotspot_width, 0x12)) {
            push_forward_help_history();
            this_help_page = help_page_hot_spots[i].page;
            return 1;
        }
    }
    return 0;
}

// Append the current page to the help-history buffer and advance its cursor.
// FUNCTION: C2 0x588b9
// FUNCTION: C2WIN 0x0045307e
void push_forward_help_history(void)
{
    help_history[this_help_action] = this_help_page;
    if (this_help_action < 0xc7)
        this_help_action++;
}

// Move back one help-history entry and restore that page.
// FUNCTION: C2 0x588e2
// FUNCTION: C2WIN 0x004530b7
void rewind_help_history(void)
{
    if (this_help_action > 0) {
        this_help_action--;
        this_help_page = help_history[this_help_action];
    }
}

// Reset help navigation to the first page and clear its history.
// FUNCTION: C2 0x58907
// FUNCTION: C2WIN 0x004530ec
void init_help_history(void)
{
    int i;

    this_help_page = first_help_page;
    this_help_action = 0;
    for (i = 0; i < 0xc8; i++)
        help_history[i] = 0;
}

// Run the tutorial campaign with beginner settings, then restore skill and peace mode.
// FUNCTION: C2 0x5892d
// FUNCTION: C2WIN 0x0045313c
void do_tutorial(void)
{
    int saved_skill = c2inf.skill_level;
    int saved_peace = (signed char)c2inf.peace_mode;

    c2inf.skill_level   = 0;
    c2inf.peace_mode    = 1;
    tutorial_page       = 0;
    tutorial_mode       = 1;
    province_difficulty = 1;
    year                = -200;
    start_year          = -200;
    month               = 0;
    week                = 0;
    players_denarii     = 0;
    players_salary      = init_salary[0].welfare_bill;
    init_tribute();
    years_elapsed       = 0;
    completed_provinces = 0;
    player_rank         = 0;

    clear_empire();
    setup_history_data();
    new_province();
    black_out();

    while (tutorial_page < 0x20) {
        do_a_tutorial_page();
    }

    tutorial_mode = 0;
    c2inf.skill_level = (signed char)saved_skill;
    c2inf.peace_mode  = saved_peace;

    confirm(0xe, 0xa0, 0xa0);
    if (decision == 1) {
        continue_tutorial_status = 1;
    }

    black_out();
    cover_mouse_droppings();
    clear_a_screen();
    hold_mouse_replace = 1;
}

// Display the current tutorial page, handle navigation, and run its interactive objective.
// FUNCTION: C2 0x58a24
// FUNCTION: C2WIN 0x004532dc
void do_a_tutorial_page(void)
{
    char *image_path;
    char *pal_path;
    int text_x;
    int text_y;
    int text_width;

    image_path = tut_files[tutorial_page].name;
    pal_path   = tut_palfiles[tutorial_page].name;

    if (!check_file_exists(image_path)) { out4 = 0;
        tutorial_page++;
    } else {

    last_tutorial_page = tutorial_page;
    setup_whole_screen_refresh();
    show_pl8file(image_path, 0x1e0);
    readfile(pal_path, temp_palette, 0x300, 0);

    this_help_page = tutorial_page + 0x3c;
    load_media_entry();
    text_pointer = format_buffer;

    text_x = this_media_entry.left_sprite;
    text_y = this_media_entry.right_sprite;
    text_width = this_media_entry.width;
    put_a_font_string(text_pointer, text_x, text_y, font2, 0x10);
    media_text_place(text_x, text_y + 0x1e, text_width, 1, text_x, text_width, font1);

    /* Bottom-row navigation arrows */
    font_list(0x31, 1, 0xa0, 0x1a0, font1, 0x10);
    font_list(0x31, 2, 0xa0, 0x1b0, font1, 0x10);
    font_list(0x31, 3, 0xa0, 0x1c0, font1, 0x10);
    font_list(0x31, 4, 0x1b0, 0x1b4, font1, 0x10);

    show_an_exit_button(0x250, 0x1b0);
    hold_mouse_replace = 1;
    refresh_svga_screen();
    set_palette(temp_palette);

    if (media_voc) set_db_sound(this_media_entry.voc_file);

    out1 = 0;
    out4 = 0;
  out1_loop:
    if (out1 != 0) goto out1_done;
    just_idle_game_loop();
    if (exit_screen()) {
        out1          = 1;
        tutorial_page = 0x20;
        do_pos();
    }
    if (mouse_left_preclick) {
        if (mouse_in_area(0x18, 0x1a8, 0x20, 0x20)) act_back_tutorial_page();
        if (mouse_in_area(0x40, 0x1a8, 0x20, 0x20)) act_middle_tutorial_page();
        if (mouse_in_area(0x68, 0x1a8, 0x20, 0x20)) act_forward_tutorial_page();
    }
    goto out1_loop;
  out1_done: ;

    }

    /* Run the interactive challenge associated with this page. */
    tutorial_level = active_tutorial_pages[tutorial_page] & 0xff;
    if (tutorial_level != 0 && out4 == 0) {
        tutorial_timer         = 0x13b9;
        tutorial_correct       = 0;
        tutorial_correct_timer = 0;
        if (tutorial_level == 5)      act_goto_prov_map();
        else if (tutorial_level == 8) act_goto_prov_map();
        else                          act_goto_city_map();

        if (map_mode == 0)      city_map_screen(0);
        else if (map_mode == 1) region_map_screen(0);

        while (out4 == 0) {
            main_game_loop();
            if (c2inf.paused == 0) tutorial_timer--;
            if (tutorial_timer < 0) out4 = 1;
            if (tutorial_correct) {
                tutorial_correct_timer++;
                if (tutorial_correct_timer > 0x64) {
                    show_please_wait();
                    wait_click();
                    out4 = 1;
                }
            }
        }
    }

    stop_db();
}

// Complete tutorial level 3 when housing gains forum access.
// FUNCTION: C2 0x58d3b
// FUNCTION: C2WIN 0x004537a1
void tutorial_test_for_forum_access(void)
{
    int map_y;
    int map_x;

    if (tutorial_level != 3) return;
    if (tutorial_timer % 50 != 0) return;
    if (tutorial_correct) return;

    map_y = 0;
    cm_sptr = 0;
    for ( ; map_y < 80; map_y++) {
    for (map_x = 0; map_x < 80; map_x++, cm_sptr += 20) {
        unsigned char building_kind = CM_CELL(cm_sptr).base_kind;
        if (building_kind >= 0x82 && building_kind <= 0xa1
            && (CM_CELL(cm_sptr).range_flag & 0x0c) != 0) {
            tutorial_correct_timer = 50;
            tutorial_correct       = 1;
            return;
        }
    }
    }
}

// Complete tutorial level 2 when housing receives water.
// FUNCTION: C2 0x58ddc
// FUNCTION: C2WIN 0x00453897
void tutorial_test_for_water_distribution(void)
{
    int map_y;
    int map_x;

    if (tutorial_level != 2) return;
    if (tutorial_timer % 50 != 0) return;
    if (tutorial_correct) return;

    map_y = 0;
    cm_sptr = 0;
    for ( ; map_y < 80; map_y++) {
    for (map_x = 0; map_x < 80; map_x++, cm_sptr += 20) {
        unsigned char building_kind = CM_CELL(cm_sptr).base_kind;
        if (building_kind >= 0x82 && building_kind <= 0xa1
            && (CM_CELL(cm_sptr).education & 0x01) != 0) {
            tutorial_correct = 1;
            return;
        }
    }
    }
}

// Draw and refresh the tutorial's "please wait" panel.
// FUNCTION: C2 0x58e6f
// FUNCTION: C2WIN 0x00453983
void show_please_wait(void)
{
    show_a_system_window(0x50, 0xc0, 0x14, 8);
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
    font_list(0x31, 0,   0x90, 0xd8, font2, 0x10);
    font_list(0x31, 9,   0x80, 0x100, font1, 0x10);
    font_list(0x31, 0xa, 0xb0, 0x110, font1, 0x10);
    font_list(0x31, 0xb, 0xa0, 0x128, font1, 0x10);
    refresh_svga_screen();
}

// Update the visible tutorial countdown while tutorial mode is active.
// FUNCTION: C2 0x58f16
// FUNCTION: C2WIN 0x00453a1a
void show_tutorial_timer(void)
{
    if (tutorial_mode != 0) {
        show_a_system_blank(0x1f4, 0x1b3, 6, 2);
        font_no(tutorial_timer / 50, ' ', " ", 0x206, 0x1bc, font2, 0x10);
        setup_refresh_area(0x1f4, 0x1b3, 6, 3, 1);
    }
}

// Navigate to the preceding available tutorial page.
// FUNCTION: C2 0x58f8b
// FUNCTION: C2WIN 0x00453ad1
void act_back_tutorial_page(void)
{
    while (tutorial_page > 0) {
        tutorial_page--;
        if (check_file_exists(tut_files[tutorial_page].name)) {
            out1 = 1;
            out4 = 1;
            do_pos();
            return;
        }
    }
    do_neg();
}

// Jump to the tutorial's middle page.
// FUNCTION: C2 0x58fdb
// FUNCTION: C2WIN 0x00453b44
void act_middle_tutorial_page(void)
{
    tutorial_page = 7;
    out1          = 1;
    do_pos();
}

// Advance to the next tutorial page.
// FUNCTION: C2 0x58ff4
// FUNCTION: C2WIN 0x00453b68
void act_forward_tutorial_page(void)
{
    tutorial_page += 1;
    out1 = 1;
    do_pos();
}

// Return whether the current tutorial stage permits a city-map icon.
// FUNCTION: C2 0x5900d
// FUNCTION: C2WIN 0x00453b88
int city_icon_allowed(int icon_idx)
{
    if (city_tutorial_icons[icon_idx] > tutorial_level) return 0;
    return 1;
}

// Return whether the current tutorial stage permits a region-map icon.
// FUNCTION: C2 0x59027
// FUNCTION: C2WIN 0x00453bbb
int region_icon_allowed(int icon_idx)
{
    if (region_tutorial_icons[icon_idx] > tutorial_level) return 0;
    return 1;
}

// Grey out city-map controls that the current tutorial stage has not unlocked.
// FUNCTION: C2 0x5902f
// FUNCTION: C2WIN 0x00453bee
void grey_city_map_parts(void)
{
    int icon_idx;
    int header_offset;
    unsigned short icon_width;
    unsigned short icon_height;
    unsigned short icon_x;
    unsigned short icon_y;

    if (tutorial_mode == 0) return;
    for (icon_idx = 4; icon_idx < 0x1c; icon_idx++) {
        if (city_icon_allowed(icon_idx - 4) == 0) {
            header_offset = icon_idx * 8;
            icon_width = int_city_header[header_offset + 4];
            icon_height = int_city_header[header_offset + 5];
            icon_x = int_city_header[header_offset + 8] + 0xee;
            icon_y = int_city_header[header_offset + 9];
            draw_a_rect(icon_x, icon_y, icon_width, icon_height, 0x1a);
        }
    }
}

// Grey out region-map controls that the current tutorial stage has not unlocked.
// FUNCTION: C2 0x5909f
// FUNCTION: C2WIN 0x00453c56
void grey_region_map_parts(void)
{
    int icon_idx;
    int header_offset;

    unsigned short icon_width;
    unsigned short icon_height;
    unsigned short icon_x;
    unsigned short icon_y;

    if (tutorial_mode == 0) return;
    for (icon_idx = 4; icon_idx < 0x17; icon_idx++) {
        if (region_icon_allowed(icon_idx - 4) == 0) {
            header_offset = icon_idx * 8;
            icon_width = int_region_header[header_offset + 4];
            icon_height = int_region_header[header_offset + 5];
            icon_x = int_region_header[header_offset + 8] + 0xee;
            icon_y = int_region_header[header_offset + 9];
            draw_a_rect(icon_x, icon_y, icon_width, icon_height, 0x1a);
        }
    }
}
