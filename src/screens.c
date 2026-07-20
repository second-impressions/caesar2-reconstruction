
#include "c2_data.h"
#include "c2_types.h"

int history_graph_years[5] = { 10, 20, 50, 100, 200 };

extern int affected_by_cover1();
extern int affected_by_cover2();


extern void put_a_font_string(char *str, int x, int y, unsigned char *font, int color);
extern void font_list(int idx, int word_count, int x, int y, unsigned char *font, int color);
extern void font_no(int value, char pad_char, char *suffix, int x, int y, unsigned char *font, int color);
extern void font_format_split(int idx, int word_skip, int x, int y_start, int max_width, int line_limit, int x_overflow, int max_width_overflow, unsigned char *font, int color);
extern void show_cursor(unsigned char *font);
/* Forward declarations (functions defined later in this file). */
void clip_zoom_level1(void);
void clip_battle_zoom_level2(void);
void clip_map_bottom(void);
void redraw_icon_bits(void);
void show_tribunes_report(int army_idx, int panel_x, int panel_y, int sprite_mode);
void reshow_initreg_box(void);
void show_name_choice(void);
void show_peace_level(void);
void show_skill_level(void);
void show_directory(int scroll_top);
void background_screen(void);
void show_tax_rates(void);
void show_accounts(void);
void show_estimate(void);
void show_personal_cash_stats(void);
void show_donation_level(void);
void show_gift_amount(int gift_index);
void basic_temple_screen(void);
void show_temple_tip(void);
void history_selection(void);
void history_graphs(void);
void basic_empire_screen(void);
void show_regions_in_empire(void);
void show_regions_on_offer(void);
void show_empire_top_slab(void);
void show_empire_bottom_slab(void);
void show_mercs(void);
void show_this_tribune(void);
void update_tribune_flag(int mode);
void show_recruitment(void);
void show_slave_welfare_bill(void);
void show_slave_allocation(void);
void show_top_line(void);
void place_3_legend_blocks(int caption_idx, int top_gfx_idx, int middle_gfx_idx, int bottom_gfx_idx);
void place_3x_legend_blocks(int caption_idx, int top_gfx_idx, int middle_gfx_idx, int bottom_gfx_idx);
void place_9_legend_blocks(void);
void place_legend_block(int sprite_idx, int x, int y);
void show_date(int year, int x, int y, int mode);
void show_query_panel_heading(int y);
void show_general_query_panel(void);
void show_query_house_advice(void);
void show_query_business_advice(void);
void show_people_query_panel(void);
void show_detailed_query_panel(void);
void show_region_query_panel(int y);


// Redraw the complete city screen, including map, controls, overlays, status bar, and palette.
// FUNCTION: C2 0x5b181
// FUNCTION: C2WIN 0x00422530
void city_map_screen(int do_black_out)
{
    int dir;

    if (do_black_out == 1) black_out();
    hold_mouse_replace = 1;
    setup_whole_screen_refresh();
    if (readfile("int_city.pl8", ((void *)scratch_buffer),
                 0x1d4c0, 0) == 0) {
        test_beeps();
        return;
    }
    draw_a_line(0x1de, 0x30, 0x1de, 0xd0, 0x1f);
    draw_a_line(0x1df, 0x30, 0x1df, 0xd0, 0x12);
    restore_picture_part(scratch_buffer, 0);
    restore_picture_part(scratch_buffer, 1);
    restore_picture_part(scratch_buffer, 2);
    restore_picture_part(scratch_buffer, 3);
    clip_zoom_level1();
    clip_map_bottom();
    flush_sb_buffer();
    show_menus(main_menu, 4, 0);
    update_map = 1;
    show_citymap();
    write_image(misc, map_direction / 2, 0x1c4, 0x1a);
    show_landfill(com_x, com_y);
    redraw_topline = 1;
    update_ov_bar = 1;
    update_map = 1;
    redraw_icons = 2;
    show_top_line();
    redraw_icon_bits();
    refresh_svga_screen();
    set_palette(city_palette);
    hold_mouse_replace = 1;
}

// Draw the complete region screen, including its map, controls, status areas, and palette.
// FUNCTION: C2 0x5b2ca
// FUNCTION: C2WIN 0x00422610
void region_map_screen(int do_black_out)
{
    if (do_black_out == 1) {
        black_out();
    }
    hold_mouse_replace = 1;
    setup_whole_screen_refresh();
    if (readfile("int_prov.pl8", ((void *)scratch_buffer), 0x1d4c0, 0) == 0) {
        test_beeps();
        return;
    }

    restore_picture_part(scratch_buffer, 0);
    restore_picture_part(scratch_buffer, 1);
    restore_picture_part(scratch_buffer, 2);
    restore_picture_part(scratch_buffer, 3);
    clip_zoom_level1();
    clip_map_bottom();
    flush_sb_buffer();
    show_menus(main_menu, 4, 0);
    show_regionmap();
    write_image(misc, map_direction / 2, 0x1c4, 0x1a);
    show_landfill(com_x, com_y);
    redraw_topline = 1;
    redraw_icons = 2;
    show_top_line();
    redraw_icon_bits();
    refresh_svga_screen();
    set_palette(region_palette);
    hold_mouse_replace = 1;
}

// Set up the in-battle UI screen.
// FUNCTION: C2 0x5b3cb
// FUNCTION: C2WIN 0x004226d5
void battle_screen(int do_black_out)
{
    if (do_black_out == 1) black_out();

    hold_mouse_replace = 1;
    setup_whole_screen_refresh();

    readfile("batlfix2.256", temp_palette, 0x300, 0);
    if (readfile("int_batl.pl8", ((void *)scratch_buffer), 0x1d4c0, 0) == 0) {
        test_beeps();
        return;
    }

    restore_picture_part(scratch_buffer, 0);
    restore_picture_part(scratch_buffer, 1);
    restore_picture_part(scratch_buffer, 2);
    restore_picture_part(scratch_buffer, 3);

    font_list(7,
              army_list[their_battle_army].source_region,
              0x32, 0x172, font1, 0x10);
    font_list(7, 0, 0x32, 0x1a8, font1, 0x10);

    clip_battle_zoom_level2();
    flush_sb_buffer();
    show_menus(main_menu, 4, 0);

    update_map = 1;
    show_battlemap();

    write_image(misc, map_direction / 2, 0x264, 0x1a);
    show_battle_landfill(0, 0x34, 0xb1, 0x170);

    redraw_topline = 1;
    update_ov_bar  = 1;
    update_map     = 4;
    redraw_icons   = 2;
    redraw_icon_bits();

    refresh_svga_screen();
    set_palette(temp_palette);
    hold_mouse_replace = 1;
}

// Update the battle statistics panel when its unit data or pointer context changes.
// FUNCTION: C2 0x5b53d
// FUNCTION: C2WIN 0x004227f5
void battle_stats_panel(void)
{
    int bar_count;
    int bar_idx;
    int panel_mode;

    panel_mode = 0;

    if (redraw_icons != 0) panel_mode = 1;
    if (request_message.bs_nof_units != battle_stats_nof_units) panel_mode = 1;
    if (request_message.bs_men != battle_stats_men) panel_mode = 1;
    if (request_message.bs_morale != battle_stats_morale) panel_mode = 1;
    if (request_message.bs_type != battle_stats_type) panel_mode = 1;

    if (request_message.prev_mode == 1) {
        if (pointer_mode == 1) panel_mode = 3;
        else if (pointer_mode == 2) panel_mode = 4;
        else if (last_icon_over != 0) panel_mode = 2;
    } else if (request_message.prev_mode == 2) {
        if (pointer_mode == 1) panel_mode = 3;
        else if (pointer_mode == 2) panel_mode = 4;
        else if (last_icon_over == 0) panel_mode = 1;
        if (last_icon_over != 0 && last_icon_over != request_message.icon_over) panel_mode = 2;
    } else if (request_message.prev_mode == 3 && pointer_mode != 1) {
        if (pointer_mode == 2) panel_mode = 4;
        else if (last_icon_over != 0) panel_mode = 2;
        else panel_mode = 1;
    } else if (request_message.prev_mode == 4 && pointer_mode != 2) {
        if (pointer_mode == 1) panel_mode = 3;
        else if (last_icon_over != 0) panel_mode = 2;
        else panel_mode = 1;
    }
    if (panel_mode == 0) return;

    request_message.bs_nof_units = battle_stats_nof_units;
    request_message.bs_men       = battle_stats_men;
    request_message.bs_morale    = battle_stats_morale;
    request_message.bs_type      = battle_stats_type;

    sprite_width = 0xa; sprite_height = 0x68;
    show_fast_rect(0x1db, 0x170, 0x1a);

    if (panel_mode == 3) {
        request_message.prev_mode = 3;
        font_format_split(0x76, 0x11,
                          0x1e2, 0x180, 0x90, 0x64, 0, 0, font1, 0x10);
    } else if (panel_mode == 4) {
        request_message.prev_mode = 4;
        font_format_split(0x76, 0x12,
                          0x1e2, 0x180, 0x90, 0x64, 0, 0, font1, 0x10);
    } else if (panel_mode == 2) {
        request_message.icon_over = last_icon_over;
        request_message.prev_mode = 2;
        font_format_split(0x76, last_icon_over - 4,
                          0x1e2, 0x190, 0x90, 0x64, 0, 0, font1, 0x10);
    } else {
        if (battle_stats_nof_units == 0) {
            request_message.prev_mode = 1;
            font_list(0x2f, 0, 0x1e6, 0x190, font1, 0x10);
            x_is = 0;
            font_no(0, 0x20, " ", 0x1ee, 0x1a0, font1, 0x10);
            font_list(0x2f, 1, x_is + 0x1ee, 0x1a0, font1, 0x10);
        } else {
            request_message.prev_mode = 1;
            if (battle_stats_nof_units == 1) {
                font_list(0x2f, 2, 0x1ee, 0x174, font1, 0x10);
            } else {
                x_is = 0;
                font_no(battle_stats_nof_units, 0x20, " ", 0x1ee, 0x174, font1, 0x10);
                font_list(0x2f, 1, x_is + 0x1ee, 0x174, font1, 0x10);
            }
            if (battle_stats_control == 0) {
                font_list(0x47, battle_stats_type + 0xa,
                          0x1ee, 0x188, font1, 0x10);
            } else {
                font_list(0x2f, battle_stats_type + 2,
                          0x1ee, 0x188, font1, 0x10);
            }
            x_is = 0;
            font_list(0x2f, 8, 0x1ee, 0x198, font1, 0x10);
            font_no(battle_stats_men, 0x20, " ",
                    x_is + 0x1ee, 0x198, font1, 0x10);

            bar_count = valueDIVtotal(battle_stats_men, battle_stats_start_men);
            if (bar_count % 10 != 0)
                bar_count = bar_count / 10 + 1;
            else
                bar_count = bar_count / 10;
            for (bar_idx = 0; bar_idx < bar_count; bar_idx++)
                write_image(game_panels, 0x39,
                            bar_idx * 9 + 0x1ee, 0x1a5);

            x_is = 0;
            font_list(0x2f, 9, 0x1ee, 0x1b8, font1, 0x10);
            font_no(battle_stats_morale, 0x20, " ",
                    x_is + 0x1ee, 0x1b8, font1, 0x10);

            bar_count = battle_stats_morale / 10;
            if (battle_stats_morale % 10 != 0) bar_count++;
            for (bar_idx = 0; bar_idx < bar_count; bar_idx++)
                write_image(game_panels, 0x38,
                            bar_idx * 9 + 0x1ee, 0x1c8);
        }
    }

    setup_refresh_area(0x1e6, 0x170, 0xa, 7, 1);
}

// Update the opposing armies' troop totals and morale bars when their values change.
// FUNCTION: C2 0x5ba21
// FUNCTION: C2WIN 0x00422e53
void battle_totals_panel(void)
{
    int dirty = 0;
    int ratio;
    int bars;
    int rem;
    int i;

    if (request_message.bt_their_men    != their_battle_men)    dirty = 1;
    if (request_message.bt_our_men      != our_battle_men)      dirty = 1;
    if (request_message.bt_their_morale != their_battle_morale) dirty = 1;
    if (request_message.bt_our_morale   != our_battle_morale)   dirty = 1;
    if (redraw_icons != 0)                          dirty = 1;
    if (!dirty) return;

    request_message.bt_their_men    = their_battle_men;
    request_message.bt_our_men      = our_battle_men;
    request_message.bt_their_morale = their_battle_morale;
    request_message.bt_our_morale   = our_battle_morale;

    sprite_width  = 9;
    sprite_height = 0x22;
    show_fast_rect(4, 0x17e, 0x1a);
    sprite_width  = 9;
    sprite_height = 0x22;
    show_fast_rect(0x1d, 0x1b4, 0x1a);

    font_no(their_battle_men, 0x20, " ", 0xc, 0x182, font1, 0x10);
    ratio = valueDIVtotal(their_battle_men, their_battle_start_men);
    rem   = ratio % 10;
    bars  = ratio / 10;
    if (rem != 0) bars = bars + 1;
    for (i = 0; i < bars; i++)
        write_image(game_panels, 0x39,
                    i * 9 + 0x2e, 0x181);

    font_no(their_battle_morale, 0x20, " ", 0xc, 0x194, font1, 0x10);
    bars = their_battle_morale / 10;
    if (their_battle_morale % 10 != 0) bars = bars + 1;
    for (i = 0; i < bars; i++)
        write_image(game_panels, 0x38,
                    i * 8 + 0x30, 0x194);

    font_no(our_battle_men, 0x20, " ", 0x82, 0x1b6, font1, 0x10);
    ratio = valueDIVtotal(our_battle_men, our_battle_start_men);
    rem   = ratio % 10;
    bars  = ratio / 10;
    if (rem != 0) bars = bars + 1;
    for (i = 0; i < bars; i++)
        write_image(game_panels, 0x3a,
                    0x73 - i * 9, 0x1b5);

    font_no(our_battle_morale, 0x20, " ", 0x82, 0x1c8, font1, 0x10);
    bars = our_battle_morale / 10;
    if (our_battle_morale % 10 != 0) bars = bars + 1;
    for (i = 0; i < bars; i++)
        write_image(game_panels, 0x38,
                    0x73 - i * 8, 0x1c8);

    setup_refresh_area(0, 0x180, 0xb, 6, 1);
}

// Show the battle setup message and continue prompt.
// FUNCTION: C2 0x5bd04
// FUNCTION: C2WIN 0x004231d9
void show_battle_setup_box(void)
{
    stone_random_count = 0xf;
    show_a_mosaic_window(0x90, 0x80, 0x16, 0xd);
    font_list(0x4d, 0x15, 0xc0, 0x9c, font2, 0x10);
    font_list(0x4d, 0x16, 0x100, 0xbc, font1, 0x10);
    font_format_split(0x4d, 0x18, 0xb0, 0xd0, 0x120, 0x64, 0, 0, font1, 0x10);
    font_list(9, 1, 0x100, 0x130, font1, 0x10);
}

// Show or clear the paused banner when the pause state changes.
// FUNCTION: C2 0x5bdb6
// FUNCTION: C2WIN 0x00423280
void show_paused(void)
{
    if (c2inf.paused != request_message.paused)
        setup_whole_screen_refresh();
    request_message.paused = c2inf.paused;
    if (c2inf.paused != 0) {
        stone_random_count = 0xf;
        show_a_mosaic_window(0x80, 0x20, 0xe, 4);
        font_list(9, 2, 0xa0, 0x34, font2, 0x10);
    }
}

// Mask the exposed edges of the zoomed city and region maps.
// FUNCTION: C2 0x5be21
// FUNCTION: C2WIN 0x004232df
void clip_zoom_level1(void)
{
    int i;

    if (zoom_level == 1) {
        for (i = 0x18; i < 0x1e0; i++) {
            show_internal_4point(0, i, 0);
        }
        for (i = 0; i < 0x1dc; i += 4) {
            show_internal_4point(i, 0x1d8, 0);
        }
        show_internal_2point(i, 0x1d8, 0);
        for (i = 0; i < 0x1dc; i += 4) {
            show_internal_4point(i, 0x1d9, 0);
        }
        show_internal_2point(i, 0x1d9, 0);
    }
}

// Mask the left edge of the zoomed battle map beside the side panel.
// FUNCTION: C2 0x5bea0
// FUNCTION: C2WIN 0x004232ed
void clip_battle_zoom_level2(void)
{
    int i;
    if (zoom_level == 2) {
        for (i = 0x18; i < 0x168; i++) {
            show_internal_4point(0, i, 0);
            show_internal_2point(4, i, 0);
        }
    }
}

// Mask the exposed rows beneath the map.
// FUNCTION: C2 0x5bedb
// FUNCTION: C2WIN 0x004232fb
void clip_map_bottom(void)
{
    int x;

    for (x = 0; x < 0x1dc; x += 4) show_internal_4point(x, 0x1da, 0);
    show_internal_2point(x, 0x1da, 0);
    for (x = 0; x < 0x1dc; x += 4) show_internal_4point(x, 0x1db, 0);
    show_internal_2point(x, 0x1db, 0);
    for (x = 0; x < 0x1dc; x += 4) show_internal_4point(x, 0x1dc, 0);
    show_internal_2point(x, 0x1dc, 0);
    for (x = 0; x < 0x1dc; x += 4) show_internal_4point(x, 0x1dd, 0);
    show_internal_2point(x, 0x1dd, 0);
    for (x = 0; x < 0x1dc; x += 4) show_internal_4point(x, 0x1de, 0);
    show_internal_2point(x, 0x1de, 0);
    for (x = 0; x < 0x1dc; x += 4) show_internal_4point(x, 0x1df, 0);
    show_internal_2point(x, 0x1df, 0);
}

// Process a pending icon-strip redraw for the current map and tutorial state.
// FUNCTION: C2 0x5bfd2
// FUNCTION: C2WIN 0x00423309
void redraw_icon_bits(void)
{
    if (redraw_icons == 0) return;
    redraw_icons--;

    if (map_mode == 0) {
        draw_city_map_part(2);
        draw_city_map_part(3);
        draw_city_map_part(0xa);
    } else if (map_mode == 1) {
        draw_region_map_part(2);
        draw_region_map_part(3);
        draw_city_map_part(0xc);
    } else if (map_mode == 2) {
        draw_battle_part(3);
        if (zoom_level == 1) draw_battle_part(6);
        else                                draw_battle_part(7);
        if (c2inf.paused == 0) draw_battle_part(8);
        if (pointer_mode == 1)     draw_battle_part(0xa);
        if (pointer_mode == 2)     draw_battle_part(0xb);
    }

    if (tutorial_mode != 0) {
        if (map_mode == 0)      grey_city_map_parts();
        else if (map_mode == 1) grey_region_map_parts();
        show_a_system_window(0x1df, 0x170, 0xa, 7);
        font_list(0x31, 7, 0x1ea, 0x17c, font2, 0x10);
        font_list(0x31, 8, 0x208, 0x19a, font2, 0x10);
        show_an_exit_button(0x258, 0x1b8);
        show_tutorial_timer();
    }

    if (update_icon != 0) {
        if (map_mode == 0) {
            if (update_icon >= 0xe && update_icon != 0x12) draw_city_map_part(update_icon);
        } else if (map_mode == 1) {
            if (update_icon >= 0xe && update_icon != 0x12) draw_region_map_part(update_icon);
        } else if (map_mode == 2) {
            if (update_icon > 0xb) draw_battle_part(update_icon);
        }
    }
#if C2_FEAT_TILE_REFRESH

    setup_whole_screen_refresh();
#endif
    flush_sb_buffer();
}

// Show the selected Roman cohort's commander, composition, strength, morale, and orders.
// FUNCTION: C2 0x5c1a4
// FUNCTION: C2WIN 0x00423580
void show_cohort_box(void)
{
    int auto_fight;

    fill_cohort_centuries();
    readfile("forumbit.pl8", ((void *)scratch_buffer), 0xea60, 0);

    stone_random_count = 0xf;
    show_a_mosaic_frame(0x10, 0x30, 0x1c, 0x14);
    show_a_mosaic_blank(0x20, 0x40, 0x1a, 0x12);
    show_an_exit_button(0x1a4, 0x144);

    write_general_sprite(army_list[tracking_army].cohort_id + 0x21,
                         0x190, 0x44);
    write_image(people_data, 0x12, 0x190, 0x56);

    x_is = 0;
    font_list(5,
              army_list[tracking_army].cohort_id,
              0x38, 0x4a, font2, 0x10);
    font_list(0x2d, 0, x_is + 0x40, 0x52, font1, 0x10);
    show_date(army_list[tracking_army].departure_year,
              x_is + 0x40, 0x52, 1);

    draw_a_dias(0x28, 0x7e, 0x190, 0xa0);
    draw_a_dias(0x28, 0x12a, 0x22, 0x22);
    show_a_32_block(0x29, 0x12b, 0x35);
    font_list(0x2d, 1, 0x50, 0x12a, font1, 0x10);
    font_list(0x2d, 2, 0x50, 0x13a, font1, 0x10);

    draw_a_dias(0xb8, 0x12a, 0x22, 0x22);
    show_a_32_block(0xb9, 0x12b, 0x36);
    font_list(0x2d, 3, 0xe0, 0x12a, font1, 0x10);
    font_list(0x2d, 4, 0xe0, 0x13a, font1, 0x10);

    draw_a_dias(0x148, 0x12a, 0x22, 0x22);
    show_a_32_block(0x149, 0x12b, 0x37);
    font_list(0x2d, 5, 0x170, 0x12a, font1, 0x10);
    font_list(0x2d, 6, 0x170, 0x13a, font1, 0x10);

    show_tribunes_report(tracking_army, 0x30, 0x86, 0);
    update_tribune_flag(0);

    auto_fight = army_list[tracking_army].state_idx;
    if (auto_fight == 0xa) {
        font_list(0x23, 0x1d, 0x120, 0x8a, font1, 0x10);
    } else {
        if      (army_list[tracking_army].cohort_size_class == 0) font_list(0x23, 0x1e, 0x120, 0x8a, font1, 0x10);
        else if (army_list[tracking_army].cohort_size_class == 1) font_list(0x23, 0x25, 0x120, 0x8a, font1, 0x10);
        else if (army_list[tracking_army].cohort_size_class == 2) font_list(0x23, 0x26, 0x120, 0x8a, font1, 0x10);
    }

    setup_map_screen_refresh();
}

// Show summary information for a non-cohort army.
// FUNCTION: C2 0x5c4e8
// FUNCTION: C2WIN 0x00423934
void show_non_cohort_box(void)
{
    int army_type;

    stone_random_count = 0xf;
    show_a_mosaic_window(0x50, 0xf0, 0x16, 8);
    show_an_exit_button(0x184, 0x144);

    army_type = army_list[tracking_army].type;
    if      (army_type <= 2) font_list(0x2d, 0x19, 0x78, 0x10e, font2, 0x10);
    else if (army_type <= 5) font_list(0x2d, 0x1a, 0x78, 0x10e, font2, 0x10);
    else if (army_type <= 6) font_list(0x2d, 0x1b, 0x78, 0x10e, font2, 0x10);
    else if (army_type <= 7) font_list(0x2d, 0x1c, 0x78, 0x10e, font2, 0x10);
    else                font_list(0x2d, 0x1d, 0x78, 0x10e, font2, 0x10);

    if (army_list[tracking_army].type == 6) {
        x_is = 0;
        font_list(0x2d, 0x1e, 0x78, 0x138, font1, 0x10);
        font_list(0x10,
                  army_list[tracking_army].trader_brings + 1,
                  x_is + 0x78, 0x138, font1, 0x10);
        font_list(0x2d, 0x1f, x_is + 0x78, 0x138, font1, 0x10);
        font_list(6,
                  region_borders[province_is].u.dir[
                      army_list[tracking_army].compass_side / 2] + 1,
                  0x78, 0x148, font1, 0x10);
    } else {
        x_is = 0;
        font_no(army_list[tracking_army].total_troops,
                0x20, " ", 0x78, 0x138, font1, 0x10);
        font_list(7,
                  army_list[tracking_army].source_region,
                  x_is + 0x78, 0x138, font1, 0x10);
    }
}

// Show an army's formation, troop composition, morale, and readiness.
// FUNCTION: C2 0x5c71b
// FUNCTION: C2WIN 0x00423bef
void show_tribunes_report(int army_idx, int panel_x, int panel_y, int sprite_mode)
{
    int sprite_x;
    int drill_step;
    int i;

    stone_random_count = 0xf;
    show_a_mosaic_blank(panel_x, panel_y, 0x15, 9);
    font_list(0x2d, 7, panel_x, panel_y, font1, 0x10);

    draw_a_dias(panel_x, panel_y + 0x10, 0x3e, 0x3e);
    show_cohort_landfill(army_idx, panel_x + 1, panel_y + 0x11);

    sprite_x = 0x44;
    drill_step = cohort_drill_spacing[army_list[army_idx].num_centuries];

    for (i = 0; i < 14; i++) {
        if (army_list[army_idx].centuries[i].type == 1) {
            if (sprite_mode == 0) write_general_sprite(army_list[army_idx].centuries[i].damaged + 0x2b, panel_x + sprite_x, panel_y + 0x14);
            else write_general_sprite(army_list[army_idx].centuries[i].damaged + 0xa, panel_x + sprite_x, panel_y + 0x14);
            sprite_x += drill_step;
        }
    }
    for (i = 0; i < 14; i++) {
        if (army_list[army_idx].centuries[i].type == 2) {
            if (sprite_mode == 0) write_general_sprite(army_list[army_idx].centuries[i].damaged + 0x30, panel_x + sprite_x, panel_y + 0x14);
            else write_general_sprite(army_list[army_idx].centuries[i].damaged + 0xf, panel_x + sprite_x, panel_y + 0x14);
            sprite_x += drill_step;
        }
    }
    for (i = 0; i < 14; i++) {
        if (army_list[army_idx].centuries[i].type == 3) {
            if (sprite_mode == 0) write_general_sprite(army_list[army_idx].centuries[i].damaged + 0x35, panel_x + sprite_x, panel_y + 0x14);
            else write_general_sprite(army_list[army_idx].centuries[i].damaged + 0x14, panel_x + sprite_x, panel_y + 0x14);
            sprite_x += drill_step;
        }
    }
    for (i = 0; i < 14; i++) {
        if (army_list[army_idx].centuries[i].type == 4) {
            if (sprite_mode == 0) write_general_sprite(army_list[army_idx].centuries[i].damaged + 0x2e, panel_x + sprite_x, panel_y + 0x14);
            else write_general_sprite(army_list[army_idx].centuries[i].damaged + 0xd, panel_x + sprite_x, panel_y + 0x14);
            sprite_x += drill_step;
        }
    }

    x_is = 0;
    font_list(0x2d, 8, panel_x, panel_y + 0x52, font1, 0x10);
    font_no(army_list[army_idx].total_troops, 0x20, " ",
            x_is + panel_x, panel_y + 0x52, font1, 0x10);
    font_list(0x2d, 9, x_is + panel_x, panel_y + 0x52, font1, 0x10);

    x_is = 0;
    font_no(army_list[army_idx].num_regulars, 0x20, "", panel_x, panel_y + 0x62, font1, 0x10);
    font_list(0x2d, 0x15, x_is + panel_x, panel_y + 0x62, font1, 0x10);
    font_no(army_list[army_idx].num_irregulars, 0x20, "",
            x_is + panel_x, panel_y + 0x62, font1, 0x10);
    font_list(0x2d, 0x16, x_is + panel_x, panel_y + 0x62, font1, 0x10);
    font_no(army_list[army_idx].num_auxillaries, 0x20, "",
            x_is + panel_x, panel_y + 0x62, font1, 0x10);
    font_list(0x2d, 0x17, x_is + panel_x, panel_y + 0x62, font1, 0x10);
    font_no(army_list[army_idx].num_specials, 0x20, "",
            x_is + panel_x, panel_y + 0x62, font1, 0x10);
    font_list(0x2d, 0x18, x_is + panel_x, panel_y + 0x62, font1, 0x10);

    font_list(0x2d, army_list[army_idx].morale + 0xb, panel_x, panel_y + 0x72, font1, 0x10);

    x_is = 0;
    font_list(0x2d, army_list[army_idx].readiness_level + 0x10, panel_x, panel_y + 0x82, font1, 0x10);
    if (army_list[army_idx].readiness_level == 0 && army_list[army_idx].morale_timer != 0) font_list(0x2d, 0xa, x_is + panel_x, panel_y + 0x82, font1, 0x10);
}


// Show the initial-region selection screen.
// FUNCTION: C2 0x5cb50
// FUNCTION: C2WIN 0x00424275
void show_initreg_box(void)
{
    char *unused_padding;

    unused_padding = "";
    black_out();
    readfile("empire.256", temp_palette, 0x300, 0);
    readfile("e_parts2.pl8", ((void *)scratch_buffer), 0x249f0, 0);
    reshow_initreg_box();
    refresh_svga_screen();
    set_palette(temp_palette);
}

// Redraw the map, available regions, and frame of the initial-region selection screen.
// FUNCTION: C2 0x5cb8f
// FUNCTION: C2WIN 0x004242ef
void reshow_initreg_box(void)
{
    show_pl8file("empire.pl8", 0x1e0);
    show_regions_in_empire();
    show_regions_on_offer();
    show_empire_top_slab();
    show_empire_bottom_slab();
    setup_whole_screen_refresh();
    font_list(0x30, 0, 0xd2, 0x19e, font1, 0x3f);
    hold_mouse_replace = 1;
}

// Show the notification that the player controls no provinces.
// FUNCTION: C2 0x5cbd7
// FUNCTION: C2WIN 0x00424359
void show_no_provinces_box(void)
{
    stone_random_count = 0xf;
    show_a_mosaic_frame(0x90, 0x90, 0x16, 0xc);
    show_a_mosaic_blank(0xa0, 0xa0, 0x14, 0xa);
    font_list(0x4e, 1, 0xc0, 0xb0, font2, 0x10);
    font_format_split(0x4e, 2, 0xb0, 0xd0, 0x120, 0x64, 0, 0,
                      font1, 0x10);
    font_list(9, 1, 0x100, 0x130, font1, 0x10);
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
    hold_mouse_replace = 1;
}

// Show the message that introduces the player's first province.
// FUNCTION: C2 0x5cc82
// FUNCTION: C2WIN 0x00424400
void show_first_region_box(void)
{
    stone_random_count = 0xf;
    show_a_mosaic_frame(0x90, 0x90, 0x16, 0xc);
    show_a_mosaic_blank(0xa0, 0xa0, 0x14, 0xa);
    font_list(0x30, 1, 0xc0, 0xb0, font2, 0x10);
    font_format_split(0x30, 2, 0xb0, 0xd0, 0x120, 0x64, 0, 0,
                      font1, 0x10);
    font_list(9, 1, 0x100, 0x130, font1, 0x10);
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
    hold_mouse_replace = 1;
}

// Show information and available actions for the selected province.
// FUNCTION: C2 0x5cd03
// FUNCTION: C2WIN 0x004244a7
void this_region_box(int confirmation_mode)
{
    int show_media;
    int box_height;
    int region_idx;

    if ((confirmation_mode) == 0)
        box_height = 0x10;
    else
        box_height = 0xe;
    show_media = 1;

    cover_mouse_droppings();
    hold_mouse_replace = 1;
    stone_random_count = 0x11;
    show_a_mosaic_frame(0x70, 0x80, 0x1a, box_height);
    show_a_mosaic_blank(0x80, 0x90, 0x18, box_height - 2);

    font_list(6, region_over, 0x90, 0x9c, font2, 0x10);

    region_idx = region_over; if (empire_won[region_idx - 1] == 0) {
        if (region_idx - show_media == province_is) {
            font_list(0x4d, 6, 0x90, 0xc0, font1, 0x10);
            show_media = 0;
        } else {
            font_list(0x30, 9, 0x90, 0xc0, font1, 0x10);
        }
    } else if (empire_won[region_over - 1] == 0x1869e) {
        font_list(0x30, 5, 0x90, 0xc0, font1, 0x10);
    } else if (empire_won[region_over - 1] == 0x1869f) {
        font_list(0x30, 6, 0x90, 0xc0, font1, 0x10);
        show_media = 0;
    } else {
        x_is = 0;
        font_list(0x30, 7, 0x90, 0xc0, font1, 0x10);
        font_no(empire_won[region_over - 1], 0x20, " ",
                x_is + 0x90, 0xc0, font1, 0x10);
        font_list(0x30, 8, x_is + 0x90, 0xc0, font1, 0x10);
        show_media = 0;
    }

    this_help_page = region_over + 0x47c;
    load_media_entry();
    text_pointer = format_buffer;
    if (region_over != 1 && show_media != 0) {
        media_text_place(0x90, 0xe0, 0x160, 0x64, 0, 0, font1);
        play_speech(region_over + 0x3a);
    }

    if ((confirmation_mode) == 0) {
        font_list(0x30, 3, 0xe0, 0x14c, font1, 0x10);
        show_buttons(0x170, 0x110, confirming_buttons, 2);
    } else {
        font_list(9, 1, 0x100, 0x13c, font1, 0x10);
    }
    setup_whole_screen_refresh();
}

// Show the introductory skill-selection information.
// FUNCTION: C2 0x5cf71
// FUNCTION: C2WIN 0x00424785
void show_skill1_box(void)
{
    cover_mouse_droppings();
    background_screen();
    show_a_system_window(0x50, 0x50, 0x1e, 0x14);
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
    font_list(0x2b, 0, 0x70, 0x68, font2, 0x10);
    font_list(0x2b, 0xd, 0x9a, 0xbe, font1, 0x10);
    font_list(0x2b, 0xc, 0x9a, 0xee, font1, 0x10);
    font_list(0x2b, 0xb, 0x9a, 0x11e, font1, 0x10);
    font_list(0x2b, 0xf, 0x9a, 0x14e, font1, 0x10);
    hold_mouse_replace = 1;
}

// Show the detailed game settings and player choices.
// FUNCTION: C2 0x5d03b
// FUNCTION: C2WIN 0x0042483c
void show_skill2_box(void)
{
    cover_mouse_droppings();
    background_screen();
    show_a_system_window(0x50, 0x50, 0x1e, 0x16);
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
    font_list(0x2b, 0x10, 0x70, 0x68, font2, 0x10);
    font_list(0x2b, 0x11, 0x6a, 0x156, font1, 0x10);
    font_list(0x2b, 0x12, 0x6a, 0x186, font1, 0x10);
    show_skill_level();
    show_peace_level();
    show_name_choice();
    hold_mouse_replace = 1;
}

// Show the selected sound, music, speed, or scroll settings page.
// FUNCTION: C2 0x5d0d7
// FUNCTION: C2WIN 0x004248c2
void show_fx_box(int settings_page)
{
    int box_height;

    if      (settings_page == 0) box_height = 7;
    else if (settings_page == 1) box_height = 0xd;
    else if (settings_page == 2) box_height = 5;
    else if (settings_page == 3) box_height = 7;

    show_a_mosaic_window(0x40, 0x70, 0x13, box_height + 2);
    setup_whole_screen_refresh();

    if (settings_page == 0) {
        font_list(0x39, 0, 0x60, 0x98, font1, 0x10);
        if (c2inf.tunes_on != 0)
            font_list(0x39, 3, 0xf0, 0x98, font1, 0x10);
        else
            font_list(0x39, 2, 0xf0, 0x98, font1, 0x10);
        font_list(0x39, 4, 0x60, 0xb8, font1, 0x10);
        font_no(c2inf.tunes_level, 0x20, "%",
                0xf0, 0xb8, font1, 0x10);
    } else if (settings_page == 1) {
        font_list(0x39, 1, 0x60, 0x98, font1, 0x10);
        if (c2inf.samples_on != 0)
            font_list(0x39, 3, 0xf0, 0x98, font1, 0x10);
        else
            font_list(0x39, 2, 0xf0, 0x98, font1, 0x10);
        font_list(0x39, 7, 0x60, 0xb8, font1, 0x10);
        if (c2inf.ambients_on != 0)
            font_list(0x39, 3, 0xf0, 0xb8, font1, 0x10);
        else
            font_list(0x39, 2, 0xf0, 0xb8, font1, 0x10);
        font_list(0x39, 8, 0x60, 0xd8, font1, 0x10);
        if (c2inf.speech_on != 0)
            font_list(0x39, 3, 0xf0, 0xd8, font1, 0x10);
        else
            font_list(0x39, 2, 0xf0, 0xd8, font1, 0x10);
        font_list(0x39, 4, 0x60, 0xf8, font1, 0x10);
        font_no(c2inf.samples_level, 0x20, "%",
                0xf0, 0xf8, font1, 0x10);
        font_list(0x4e, 3, 0x60, 0x118, font1, 0x10);
        font_no(c2inf.max_samples, 0x20, " ",
                0xf0, 0x118, font1, 0x10);
    } else if (settings_page == 2) {
        font_list(0x39, 5, 0x60, 0x98, font1, 0x10);
        if (c2inf.anims_on != 0)
            font_list(0x39, 3, 0xf0, 0x98, font1, 0x10);
        else
            font_list(0x39, settings_page, 0xf0, 0x98, font1, 0x10);
    } else if (settings_page == 3) {
        font_list(0x39, 6, 0x60, 0x98, font1, 0x10);
        if (c2inf.yearend_on != 0)
            font_list(0x39, settings_page, 0xf0, 0x98, font1, 0x10);
        else
            font_list(0x39, 2, 0xf0, 0x98, font1, 0x10);
        font_list(0x39, 9, 0x60, 0xb8, font1, 0x10);
        if (c2inf.autosave_on != 0)
            font_list(0x39, 3, 0xf0, 0xb8, font1, 0x10);
        else
            font_list(0x39, 2, 0xf0, 0xb8, font1, 0x10);
    }

    font_list(9, 1, 0x90, box_height * 0x10 + 0x68, font1, 0x10);
    setup_whole_screen_refresh();
    hold_mouse_replace = 1;
}

// Show the game's credits and Impressions logo.
// FUNCTION: C2 0x5d4d4
// FUNCTION: C2WIN 0x00424d30
void show_about_box(void)
{
    show_a_mosaic_window(0x80, 0x40, 0x14, 0xa);
    font_list(0xb, 0, 0xa0, 0x58, font1, 0x10);
    font_list(0x3b, 0, 0xa0, 0x68, font1, 0x10);
    write_image(logos, 0, 0x150, 0x78);
    font_list(9, 1, 0xc0, 0xc0, font1, 0x10);
    show_an_exit_button(0x194, 0xb4);
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
    refresh_svga_screen();
    hold_mouse_replace = 1;
}

// Show the player's current name in the new-game settings.
// FUNCTION: C2 0x5d581
// FUNCTION: C2WIN 0x00424dd3
void show_name_choice(void)
{
    show_a_system_blank(0x60, 0x110, 0x1c, 3);
    font_list(0x2b, 0xa, 0x6a, 0x126, font1, 0x10);
    put_a_font_string(c2inf.player_name, 0x140, 0x126, font1, 0x10);
}

// Show whether peaceful play is enabled.
// FUNCTION: C2 0x5d5dc
// FUNCTION: C2WIN 0x00424e2d
void show_peace_level(void)
{
    show_a_system_blank(0x60, 0xe0, 0x1c, 3);
    font_list(0x2b, 7, 0x6a, 0xf6, font1, 0x10);
    font_list(0x2b, (signed char)c2inf.peace_mode + 8, 0x140, 0xf6, font1, 0x10);
    setup_refresh_area(0x60, 0xe0, 0x1c, 3, 1);
}

// Show the selected difficulty level and its explanation.
// FUNCTION: C2 0x5d658
// FUNCTION: C2WIN 0x00424e8b
void show_skill_level(void)
{
    show_a_system_blank(0x60, 0x90, 0x1c, 5);
    draw_a_box(0x68, 0xc0, 0x1b4, 0x1c, 0x10);
    font_list(0x2b, 1, 0x6a, 0xa8, font1, 0x10);
    font_list(0x2c, c2inf.skill_level, 0x160, 0xa2, font2, 0x10);
    font_list(0x2b, c2inf.skill_level + 2, 0x70, 0xc8, font1, 0x10);
    setup_refresh_area(0x60, 0x90, 0x1c, 6, 1);
}

// Show the dialog for entering a new player name.
// FUNCTION: C2 0x5d70c
// FUNCTION: C2WIN 0x00424f21
void show_new_name_box(void)
{
    cover_mouse_droppings();
    show_a_system_window(0xd0, 0xd0, 0x11, 5);
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
    put_a_font_string(c2inf.player_name, 0xe2, 0xe0, font1, 0x10);
    font_list(0x2b, 0xe, 0xe0, 0x100, font1, 0x10);
    hold_mouse_replace = 1;
}

// Show the exit confirmation dialog.
// FUNCTION: C2 0x5d765
// FUNCTION: C2WIN 0x00424f8d
void show_exit_box(void)
{
    cover_mouse_droppings();
    show_a_system_window(0x80, 0xa0, 0xe, 9);
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
    font_list(0x4d, 9, 0x90, 0xb8, font1, 0x10);
    font_list(0x4d, 0xa, 0x90, 0xe0, font1, 0x10);
    font_list(0x4d, 0xb, 0x90, 0x108, font1, 0x10);
    hold_mouse_replace = 1;
}

// Draw the shared frame for the load and save file pickers.
// FUNCTION: C2 0x5d7eb
// FUNCTION: C2WIN 0x00425011
void show_loadsave_box(int title_id)
{
    cover_mouse_droppings();
    show_a_system_window(0x20, 0x50, 0x19, 0x14);
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
    font_list(title_id, 0, 0x30, 0x60, font2, 0x10);
    draw_a_box(0x30, 0x88, 0x170, 0x100, 0x10);
    draw_a_box(0x38, 0x90, 0xc0, 0x20, 0x10);
    draw_a_box(0x38, 0xb8, 0x140, 0xa4, 0x10);
    draw_a_box(0x38, 0x164, 0x160, 0x1c, 0x10);
    show_directory(999);
    hold_mouse_replace = 1;
}

// Draw the scrollable save-file directory and editable filename.
// FUNCTION: C2 0x5d8b0
// FUNCTION: C2WIN 0x004250c4
void show_directory(int scroll_top)
{
    int text_x;
    int text_y;
    int entry_idx;
    int row_idx;

    got_cursx     = 0;
    cursor_x      = 0;
    fb_count      = 0;
    cursor_y      = 0x98;
    allow_padding = 1;
    x_is          = 0;

    show_a_system_blank(0x3c, 0x92, 0xa, 1);
    show_a_system_blank(0x3c, 0x9c, 0xa, 1);
    put_a_font_string(filename, 0x40, 0x98, font1, 0x10);
    if (got_cursx == 0) {
        cursor_x  = x_is;
        got_cursx = 1;
    }
    cursor_x += 0x40;
    show_cursor(font1);

    text_x = 0x40;
    text_y = 0xbc;
    show_a_system_blank(0x3e, 0xba, 0x13, 0xa);

    entry_idx = first_entry;
    row_idx = 0;
    goto check_entries;
    for (;;) {
        char *name;
        name = directory[entry_idx];
        if (entry_idx == scroll_top) {
            sprite_width  = 9;
            sprite_height = 0xf;
            show_fast_rect(text_x - 2, text_y - 2, 0x10);
            put_a_font_string(name, text_x, text_y, font1, 0x20);
        } else {
            put_a_font_string(name, text_x, text_y, font1, 0x10);
        }
        if (row_idx == 9) {
            text_x = 0xe0;
            text_y = 0xbc;
        } else {
            text_y += 0x10;
        }
        if (row_idx >= 0x13) break;
        entry_idx++;
        row_idx++;
check_entries:
        if (entry_idx >= no_of_entries) break;
    }
}

// Load the forum artwork, palette, and department explanation panels.
// FUNCTION: C2 0x5da18
// FUNCTION: C2WIN 0x00425294
void forum_constant_screen(void)
{
    int i;

    black_out();
    readfile("forumbit.pl8", ((void *)scratch_buffer), 0xea60, 0);
    readfile("forum_x.gd8", ((void *)((scratch_buffer) + (0x1d4c0))),
             0xfa0, 0);
    readfile("forum.256", temp_palette, 0x300, 0);
    show_pl8file("forum.pl8", 0x1e0);
    for (i = 0; i < 0xc; i++) {
        forum_explanations(i, 0);
    }
    hold_mouse_replace = 1;
}

// Show the neutral background used between major screens.
// FUNCTION: C2 0x5da8f
// FUNCTION: C2WIN 0x0042533e
void background_screen(void)
{
    int i;

    black_out();
    readfile("backgrnd.256", temp_palette, 0x300, 0);
    show_pl8file("backgrnd.pl8", 0x1e0);
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
    refresh_svga_screen();
    set_palette(temp_palette);
    hold_mouse_replace = 1;
}

// Show the forum with no department selected.
// FUNCTION: C2 0x5dac6
// FUNCTION: C2WIN 0x00425395
void forum_empty_screen(void)
{
    int i;

    cover_mouse_droppings();
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
    show_pl8file("forum.pl8", 0x1e0);
    for (i = 0; i < 12; i++) {
        forum_explanations(i, 0);
    }
    hold_mouse_replace = 1;
    refresh_svga_screen();
    set_palette(temp_palette);
}

// Show the simplified forum advisor view used when only the city is available.
// FUNCTION: C2 0x5db0b
// FUNCTION: C2WIN 0x004253ff
void forum_city_only_screen(void)
{
    cover_mouse_droppings();
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
    if (forum_repapering[last_forum_dept] != 0) {
        show_pl8file("forum.pl8", forum_repapering[last_forum_dept]);
    }
    explain_forum();
    stone_random_count = 0x14;
    show_a_mosaic_window(0, 0, 0x28, 0xb);
    x_is = 0;
    font_list(0x27, 0, 0x64, 0x38, font2, 0x10);
    font_list(0x27, 1, 0x8c, 0x6a, font1, 0x10);
    hold_mouse_replace = 1;
    refresh_svga_screen();
    set_palette(temp_palette);
}

// Show the administration forum with treasury, population, employment, and tax controls.
// FUNCTION: C2 0x5dbb4
// FUNCTION: C2WIN 0x004254b0
void forum_admin_screen(void)
{
    cover_mouse_droppings();
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif

    if (forum_repapering[last_forum_dept] != 0) {
        show_pl8file("forum.pl8",
                     forum_repapering[last_forum_dept]);
    }

    explain_forum();
    show_a_mosaic_frame(0, 0, 0x28, 0xc);
    stone_random_count = 0xa;
    show_a_mosaic_blank(0x10, 0x10, 0x26, 0xa);

    x_is = 0;
    font_list(0x1e, 0, 0x18, 0x12, font2, 0x10);

    if (denarii < 0) {
        font_no(-denarii, 0x2d, " Dn", x_is + 0x1c, 0x12, font2, 0xb);
    } else {
        font_no(denarii,  0x20, " Dn", x_is + 0x1c, 0x12, font2, 0x10);
    }

    x_is = 0;
    font_no(population,      0x20, " ", 0x18, 0x32, font1, 0x10);
    font_list(0x1e, 1, x_is + 0x18, 0x32, font1, 0x10);
    font_no(employment_rate, 0x20, "%", x_is + 0x18, 0x32, font1, 0x10);
    font_list(0x1e, 2, x_is + 0x18, 0x32, font1, 0x10);

    font_list(0x1e, 3, 0x108, 0x1a, font1, 0x10);
    font_list(0x1e, 4, 0x108, 0x32, font1, 0x10);

    draw_a_dotted_line(0x14, 0x46, 0x26a, 0x46, 0x10);
    draw_a_dotted_line(0x13f, 0x48, 0x13f, 0xac, 0x10);

    show_buttons(0x178, 0x12, admin_buttons, 4);

    show_tax_rates();
    show_accounts();
    show_estimate();

    hold_mouse_replace = 1;
    refresh_svga_screen();
    set_palette(temp_palette);
}

// Show the current and average tax rates.
// FUNCTION: C2 0x5ddcd
// FUNCTION: C2WIN 0x004256cf
void show_tax_rates(void)
{
    get_average_pop_tax();
    get_average_ind_tax();

    stone_random_count = 0x18;
    show_a_mosaic_blank(0x1a8, 0x12, 3, 3);
    font_no(pop_tax_rate, 0x20, "%",  0x1b0, 0x1a, font1, 0x10);
    font_no(ind_tax_rate, 0x20, "%",  0x1b0, 0x32, font1, 0x10);

    show_a_mosaic_blank(0x1ce, 0x12, 0xa, 3);
    x_is = 0;
    font_list(0x1e, 5, 0x1d8, 0x1a, font1, 0x10);
    font_no(average_pop_tax_denariis, 0x20, ".",
            x_is + 0x1d8, 0x1a, font1, 0x10);
    if (average_pop_tax_asses < 0xa) {
        font_no(average_pop_tax_asses, 0x30,
                "Dn )", x_is + 0x1d8, 0x1a, font1, 0x10);
    } else {
        font_no(average_pop_tax_asses, 0x20,
                "Dn )", x_is + 0x1d8, 0x1a, font1, 0x10);
    }
    x_is = 0;
    font_list(0x1e, 5, 0x1d8, 0x32, font1, 0x10);
    font_no(average_ind_tax_denariis, 0x20, ".",
            x_is + 0x1d8, 0x32, font1, 0x10);
    if (average_ind_tax_asses < 0xa) {
        font_no(average_ind_tax_asses, 0x30,
                "Dn )", x_is + 0x1d8, 0x32, font1, 0x10);
    } else {
        font_no(average_ind_tax_asses, 0x20,
                "Dn )", x_is + 0x1d8, 0x32, font1, 0x10);
    }

    setup_refresh_area(0x158, -14, 0x12, 6, 1);
}

// Show last year's treasury income, expenses, and surplus or deficit.
// FUNCTION: C2 0x5dfa2
// FUNCTION: C2WIN 0x004258b9
void show_accounts(void)
{
    stone_random_count = 4;
    show_a_mosaic_blank(0x10, 0x4a, 0x12, 6);
    show_date(year - 1, 0x18, 0x4c, 1);
    font_list(0x1e, 6, x_is + 0x18, 0x4c, font1, 0x10);

    if (account_total < 0) {
        font_no(-account_total, 0x20, " Dn", x_is + 0x18, 0x4c, font1, 0x10);
        font_list(0x1e, 8, x_is + 0x18, 0x4c, font1, 0x10);
    } else {
        font_no(account_total, 0x20, " Dn", x_is + 0x18, 0x4c, font1, 0x10);
        font_list(0x1e, 9, x_is + 0x18, 0x4c, font1, 0x10);
    }

    x_is = 0;
    font_list(0x1e, 0xa, 0x26, 0x5d, font1, 0x10);
    font_no(account_pop_tax, 0x20, " Dn", x_is + 0x26, 0x5e, font1, 0x10);
    font_list(0x1e, 0xc, 0x8c, 0x5e, font1, 0x10);

    x_is = 0;
    font_list(0x1e, 0xa, 0x26, 0x6e, font1, 0x10);
    font_no(account_ind_tax, 0x20, " Dn", x_is + 0x26, 0x6f, font1, 0x10);
    font_list(0x1e, 0xd, 0x8c, 0x6f, font1, 0x10);

    x_is = 0;
    font_list(0x1e, 0xb, 0x26, 0x7f, font1, 0x10);
    font_no(account_construction_cost, 0x20, " Dn",
            x_is + 0x26, 0x80, font1, 0x10);
    font_list(0x1e, 0xe, 0x8c, 0x80, font1, 0x10);

    x_is = 0;
    font_list(0x1e, 0xb, 0x26, 0x90, font1, 0x10);
    font_no(account_operating_cost, 0x20, " Dn",
            x_is + 0x26, 0x91, font1, 0x10);
    font_list(0x1e, 0xf, 0x8c, 0x91, font1, 0x10);

    x_is = 0;
    font_list(0x1e, 0xb, 0x26, 0xa1, font1, 0x10);
    font_no(account_tribute, 0x20, " Dn",
            x_is + 0x26, 0xa2, font1, 0x10);
    font_list(0x1e, 0x10, 0x8c, 0xa2, font1, 0x10);
}

// Show projected year-end income, expenses, and surplus or deficit.
// FUNCTION: C2 0x5e2bd
// FUNCTION: C2WIN 0x00425bae
void show_estimate(void)
{
    get_estimates();
    stone_random_count = 0x11;
    show_a_mosaic_blank(0x144, 0x4a, 0x12, 6);
    show_date(year, 0x148, 0x4c, 1);
    font_list(0x1e, 7, x_is + 0x148, 0x4c, font1, 0x10);

    if (estimate_total < 0) {
        font_no(-estimate_total, 0x20, " Dn", x_is + 0x148, 0x4c, font1, 0x10);
        font_list(0x1e, 8, x_is + 0x148, 0x4c, font1, 0x10);
    } else {
        font_no(estimate_total, 0x20, " Dn", x_is + 0x148, 0x4c, font1, 0x10);
        font_list(0x1e, 9, x_is + 0x148, 0x4c, font1, 0x10);
    }

    x_is = 0;
    font_list(0x1e, 0xa, 0x154, 0x5d, font1, 0x10);
    font_no(estimate_pop_tax, 0x20, " Dn", x_is + 0x154, 0x5e, font1, 0x10);
    font_list(0x1e, 0xc, 0x1ba, 0x5e, font1, 0x10);

    x_is = 0;
    font_list(0x1e, 0xa, 0x154, 0x6e, font1, 0x10);
    font_no(estimate_ind_tax, 0x20, " Dn", x_is + 0x154, 0x6f, font1, 0x10);
    font_list(0x1e, 0xd, 0x1ba, 0x6f, font1, 0x10);

    x_is = 0;
    font_list(0x1e, 0xb, 0x154, 0x7f, font1, 0x10);
    font_no(estimate_construction_cost, 0x20, " Dn",
            x_is + 0x154, 0x80, font1, 0x10);
    font_list(0x1e, 0xe, 0x1ba, 0x80, font1, 0x10);

    x_is = 0;
    font_list(0x1e, 0xb, 0x154, 0x90, font1, 0x10);
    font_no(estimate_operating_cost, 0x20, " Dn",
            x_is + 0x154, 0x91, font1, 0x10);
    font_list(0x1e, 0xf, 0x1ba, 0x91, font1, 0x10);

    x_is = 0;
    font_list(0x1e, 0xb, 0x154, 0xa1, font1, 0x10);
    font_no(estimate_tribute, 0x20, " Dn",
            x_is + 0x154, 0xa2, font1, 0x10);
    font_list(0x1e, 0x10, 0x1ba, 0xa2, font1, 0x10);

    setup_refresh_area(0x140, 0x4c, 0x14, 9, 1);
}

// Show the player's name, rank, promotion progress, and career guidance.
// FUNCTION: C2 0x5e60d
// FUNCTION: C2WIN 0x00425ed0
void forum_career_screen(void)
{
    cover_mouse_droppings();
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif

    if (forum_repapering[last_forum_dept] != 0) {
        show_pl8file("forum.pl8",
                     forum_repapering[last_forum_dept]);
    }

    explain_forum();
    stone_random_count = 0x14;
    show_a_mosaic_frame(0, 0, 0x28, 0xb);
    show_a_mosaic_blank(0x10, 0x10, 0x26, 9);

    x_is = 0;
    font_list(0x1f, 0, 0x18, 0x1c, font2, 0x10);
    put_a_font_string(c2inf.player_name, x_is + 0x1c, 0x1c, font2, 0x10);
    font_list(8, player_rank, x_is + 0x20, 0x1c, font2, 0x10);

    if (c2inf.peace_mode == 0 && player_rank < 0xa) {
        x_is = 0;
        font_list(0x4d, 0x11, 0x18, 0x42, font1, 0x10);
        font_no((unsigned char)promotions_to_win_game[c2inf.skill_level]
                - completed_provinces,
                0x20, " ", x_is + 0x18, 0x42, font1, 0x10);

        font_list(0x4d, 0x12, x_is + 0x18, 0x42, font1, 0x10);

        if (c2inf.skill_level < 2) {
            font_list(0x4d, 0x14, x_is + 0x18, 0x42, font1, 0x10);
        } else {
            font_list(0x4d, 0x13, x_is + 0x18, 0x42, font1, 0x10);
        }
    }

    show_personal_cash_stats();
    show_buttons(0xe0, 0x58, career_buttons, 3);

    hold_mouse_replace = 1;
    refresh_svga_screen();
    set_palette(temp_palette);
}

// Show the player's personal balance and annual salary.
// FUNCTION: C2 0x5e7f3
// FUNCTION: C2WIN 0x004260ba
void show_personal_cash_stats(void)
{
    stone_random_count = 0x14;
    show_a_mosaic_blank(0x10, 0x5c, 0xc, 4);
    x_is = 0;
    font_list(0x1f, 1, 0x12c, 0x5e, font1, 0x10);
    if (players_denarii < 0) {
        font_no(-players_denarii, '-', " Dn", x_is + 0x12c, 0x5e, font1, 0xb);
    } else {
        font_no(players_denarii, ' ', " Dn", x_is + 0x12c, 0x5e, font1, 0x10);
    }
    x_is = 0;
    font_list(0x1f, 2, 0x14, 0x5e, font1, 0x10);
    font_no(players_salary, ' ', " Dn", x_is + 0x14, 0x5e, font1, 0x10);
    x_is = 0;
    font_list(0x1f, 3, 0x14, 0x7a, font1, 0x10);
    setup_refresh_area(0x10, 0x40, 0x11, 6, 1);
}

// Show the controls for donating personal money to the city.
// FUNCTION: C2 0x5e91e
// FUNCTION: C2WIN 0x004261cf
void show_donation_box(void)
{
    cover_mouse_droppings();
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
    stone_random_count = 0x10;
    show_a_mosaic_window(0xa0, 0x20, 0x11, 9);
    show_donation_level();
    font_list(0x1f, 5, 0xc0, 0x5c, font1, 0x10);
    font_list(9, 1, 0xc0, 0x88, font1, 0x10);
    show_an_exit_button(0x180, 0x80);
    hold_mouse_replace = 1;
    refresh_svga_screen();
    set_palette(temp_palette);
}

// Show the currently selected donation amount.
// FUNCTION: C2 0x5e9a4
// FUNCTION: C2WIN 0x00426260
void show_donation_level(void)
{
    stone_random_count = 0x10;
    show_a_mosaic_blank(0xb0, 0x38, 0xb, 2);
    x_is = 0;
    font_list(0x1f, 4, 0xc0, 0x40, font1, 0x10);
    font_no(donation_level, ' ', " Dn", x_is + 0xc0, 0x40, font1, 0x10);
    setup_refresh_area(0xb0, 0x30, 0xa, 3, 1);
}

// Show the forum's imperial-relations department.
// FUNCTION: C2 0x5ea37
// FUNCTION: C2WIN 0x004262d2
void forum_rome_screen(void)
{
    int favour_word;

    cover_mouse_droppings();
    setup_whole_screen_refresh();

    if (forum_repapering[last_forum_dept] != 0)
        show_pl8file("forum.pl8",
                     forum_repapering[last_forum_dept]);
    explain_forum();
    stone_random_count = 0x14;
    show_a_mosaic_frame(0, 0, 0x28, 0xb);
    show_a_mosaic_blank(0x10, 0x10, 0x26, 9);
    font_list(0x26, 0, 0x18, 0x1c, font2, 0x10);

    x_is = 0;
    font_list(0x26, 1, 0x20, 0x40, font1, 0x10);
    if      (imperial_favour <= 0)    favour_word = 3;
    else if (imperial_favour >= 0xc8) favour_word = 0xe;
    else                              favour_word = imperial_favour / 20 + 4;
    font_list(0x26, favour_word, x_is + 0x30, 0x40, font1, 0x10);

    x_is = 0;
    font_list(0x26, 2, 0x20, 0x50, font1, 0x10);
    font_no(tribute, 0x20, " Dn.", x_is + 0x20, 0x50, font1, 0x10);

    x_is = 0;
    if (imperial_req_amount == 0) {
        font_list(0x26, 0x14, 0x20, 0x60, font1, 0x10);
    } else {
        font_list(0x26, 0x15, 0x20, 0x60, font1, 0x10);
        font_no(imperial_req_amount, 0x20, " ",
                x_is + 0x20, 0x60, font1, 0x10);
        font_list(0x26, 0xf, x_is + 0x20, 0x60, font1, 0x10);
        font_list(0x10, imperial_req_goods + 1,
                  x_is + 0x20, 0x60, font1, 0x10);
    }

    draw_a_dias(0x1c, 0x76, 0x248, 0x28);
    if (warned_of_emperor_reply_month != 0) {
        font_list(0x26, 0x1a, 0x20, 0x7c, font1, 0x10);
    } else {
        font_list(0x26, 0x16, 0x20, 0x7c, font1, 0x10);
        show_buttons(0x150, 0x78, rome1_buttons, 1);
        x_is = 0;
        font_list(0x26, 0x1b, 0x180, 0x7c, font1, 0x10);
        font_no(av_imperial_gift_level, 0x20, " Dn)",
                x_is + 0x180, 0x7c, font1, 0x10);
    }

    x_is = 0;
    font_list(0x1f, 1, 0x20, 0x8e, font1, 0x10);
    font_no(players_denarii, 0x20, " Dn",
            x_is + 0x20, 0x8e, font1, 0x10);

    hold_mouse_replace = 1;
    refresh_svga_screen();
    set_palette(temp_palette);
}

// Show the confirmation dialog for sending an imperial gift.
// FUNCTION: C2 0x5ed7c
// FUNCTION: C2WIN 0x00426609
void show_final_bribe_box(void)
{
    cover_mouse_droppings();
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
    stone_random_count = 0x14;
    show_a_mosaic_window(0xa0, 0x80, 0x16, 0xe);
    font_list(0x26, 0x27, 0xc0, 0x98, font1, 0x10);
    x_is = 0;
    font_list(0x26, 0x1b, 0xc0, 0xb0, font1, 0x10);
    font_no(av_imperial_gift_level, ' ', " Dn)", x_is + 0xc0, 0xb0, font1, 0x10);
    x_is = 0;
    font_list(0x1f, 1, 0xc0, 0xc8, font1, 0x10);
    font_no(players_denarii, ' ', " Dn", x_is + 0xc0, 0xc8, font1, 0x10);
    font_list(0x26, 0x17, 0xc0, 0x100, font1, 0x10);
    font_list(0x26, 0x18, 0xc0, 0x118, font1, 0x10);
    font_list(0x26, 0x19, 0xc0, 0x138, font1, 0x10);
    show_buttons(0x160, 0xf8, rome2_buttons, 3);
    show_gift_amount(0x10);
    hold_mouse_replace = 1;
    refresh_svga_screen();
}

// Show the imperial-gift selection panel.
// FUNCTION: C2 0x5ef04
// FUNCTION: C2WIN 0x0042677f
void show_gift_box(void)
{
    cover_mouse_droppings();
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
    show_a_mosaic_frame(0xa0, 0x20, 0x16, 8);
    stone_random_count = 0x14;
    show_a_mosaic_blank(0xb0, 0x30, 0x14, 6);
    font_list(0x26, 0x17, 0xc0, 0x40, font1, 0x10);
    font_list(0x26, 0x18, 0xc0, 0x58, font1, 0x10);
    font_list(0x26, 0x19, 0xc0, 0x78, font1, 0x10);
    show_buttons(0x160, 0x38, rome2_buttons, 3);
    show_gift_amount(4);
    hold_mouse_replace = 1;
    refresh_svga_screen();
}

// Show the gift value beside the selected imperial-gift slot.
// FUNCTION: C2 0x5efcd
// FUNCTION: C2WIN 0x00426836
void show_gift_amount(int gift_index)
{
    stone_random_count = 0x14;
    show_a_mosaic_blank(0x1a0, gift_index * 16 - 4, 5, 1);
    if (imperial_gift_level < 0) {
        imperial_gift_level = 0;
    }
    font_no(imperial_gift_level, ' ', " Dn", 0x1a8, gift_index * 16, font1, 0x10);
    setup_refresh_area(0x140, gift_index * 16 - 0x10, 0xf, 2, 2);
}

// Enter the selected forum advisor department.
// FUNCTION: C2 0x5f03f
// FUNCTION: C2WIN 0x004268a3
void forum_advisor_screen(void)
{
    black_out();
    cover_mouse_droppings();
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
    show_pl8file("forum.pl8", 0x1e0);
    readfile("forum.256", temp_palette, 0x300, 0);
    explain_forum();
    hold_mouse_replace = 1;
    refresh_svga_screen();
    set_palette(temp_palette);
}

// Enter the religion forum and prepare its temple overview.
// FUNCTION: C2 0x5f080
// FUNCTION: C2WIN 0x00426901
void forum_temple_screen(void)
{
    black_out();
    readfile("rat_back.256", temp_palette, 0x300, 0);
    readfile("rat_back.pl8", ((void *)scratch_buffer), 0xc350, 0);
    basic_temple_screen();
    set_palette(temp_palette);
    readfile("forum.256", temp_palette, 0x300, 0);
    readfile("forumbit.pl8", ((void *)scratch_buffer), 0xea60, 0);
}

// Show temple coverage, favor, funding, and religion statistics.
// FUNCTION: C2 0x5f0f5
// FUNCTION: C2WIN 0x00426989
void basic_temple_screen(void)
{
    int next_threshold;
    int next_av_threshold;
    int i;

    cover_mouse_droppings();
    show_pl8file("rat_fron.pl8", 0x1e0);
    show_a_mosaic_blank(0, 0x160, 0x28, 8);
    font_list(0x22, 3, 0xcc, 0x1d0, font1, 0x20);
    setup_whole_screen_refresh();
    x_is = 0;

    next_threshold    = promotion_levels[c2inf.skill_level][completed_provinces];
    next_av_threshold = promotion_av_levels[c2inf.skill_level][completed_provinces];

    for (i = 0; i < 4; i++) {
        int bar_w;
        int rank_image;
        int bar_w_raw;
        if (i == 0)      bar_w = 0x12c - empire_rating * 3;
        else if (i == 1) bar_w = 0x12c - peace_rating * 3;
        else if (i == 2) bar_w = 0x12c - prosperity_rating * 3;
        else if (i == 3) bar_w = 0x12c - culture_rating * 3;
        if (i == 0)      { if (empire_rating     >= next_threshold) rank_image = 2; else rank_image = 1; }
        else if (i == 1) { if (peace_rating      >= next_threshold) rank_image = 2; else rank_image = 1; }
        else if (i == 2) { if (prosperity_rating >= next_threshold) rank_image = 2; else rank_image = 1; }
        else if (i == 3) { if (culture_rating    >= next_threshold) rank_image = 2; else rank_image = 1; }
        bar_w_raw = bar_w;
        if (bar_w > 0x126) bar_w = 0x126;
        if (bar_w < 0)     bar_w = 0;
        write_general_sprite_with_front_ofset(0, i * 0xa0 + 0x20, 0xc, bar_w + 0xc);
        write_general_sprite(rank_image, i * 0xa0 + 0x20, bar_w_raw);
    }

    for (i = 0; i < 4; i++) {
        int icon_x = i * 0xa0;
        draw_a_dias(icon_x + 0xa, 0x164, 0x8c, 0x21);
    }

    if (c2inf.peace_mode == 0) {
        play_speech(0x1e);
        x_is = 0;
        font_list(0x20, 1, 0x1c, 0x167, font1, 0x10);
        font_no(empire_rating, 0x20, " %", x_is + 0x1c, 0x167, font1, 0x10);
        x_is = 0;
        font_list(0x20, 6, 0x20, 0x177, font1, 0x10);
        font_no(next_threshold, 0x20, " %)", x_is + 0x20, 0x177, font1, 0x10);
        x_is = 0;
        font_list(0x20, 2, 0xbc, 0x167, font1, 0x10);
        font_no(peace_rating, 0x20, " %", x_is + 0xbc, 0x167, font1, 0x10);
        x_is = 0;
        font_list(0x20, 6, 0xbc, 0x177, font1, 0x10);
        font_no(next_threshold, 0x20, " %)", x_is + 0xbc, 0x177, font1, 0x10);
        x_is = 0;
        font_list(0x20, 3, 0x160, 0x167, font1, 0x10);
        font_no(prosperity_rating, 0x20, " %", x_is + 0x160, 0x167, font1, 0x10);
        x_is = 0;
        font_list(0x20, 6, 0x15c, 0x177, font1, 0x10);
        font_no(next_threshold, 0x20, " %)", x_is + 0x15c, 0x177, font1, 0x10);
        x_is = 0;
        font_list(0x20, 4, 0x1fc, 0x167, font1, 0x10);
        font_no(culture_rating, 0x20, " %", x_is + 0x205, 0x167, font1, 0x10);
        x_is = 0;
        font_list(0x20, 6, 0x1fc, 0x177, font1, 0x10);
        font_no(next_threshold, 0x20, " %)", x_is + 0x1fc, 0x177, font1, 0x10);
        x_is = 0;
        font_list(0x20, 5, 0xa0, 0x18a, font1, 0xe);
        font_no(average_rating, 0x20, " %",
                x_is + 0xa0, 0x18a, font1, 0xe);
        font_list(0x20, 6, x_is + 0xa2, 0x18a, font1, 0xe);
        font_no(next_av_threshold, 0x20, " %)",
                x_is + 0xa2, 0x18a, font1, 0xe);
        show_temple_tip();
    } else {
        play_speech(0x2d);
        x_is = 0;
        font_list(0x20, 1, 0x1c, 0x167, font1, 0x10);
        font_no(0, 0x20, " %", x_is + 0x1c, 0x167, font1, 0x10);
        x_is = 0;
        font_list(0x20, 2, 0xbc, 0x167, font1, 0x10);
        font_no(0, 0x20, " %", x_is + 0xbc, 0x167, font1, 0x10);
        x_is = 0;
        font_list(0x20, 3, 0x160, 0x167, font1, 0x10);
        font_no(prosperity_rating, 0x20, " %",
                x_is + 0x160, 0x167, font1, 0x10);
        x_is = 0;
        font_list(0x20, 4, 0x1fc, 0x167, font1, 0x10);
        font_no(culture_rating, 0x20, " %",
                x_is + 0x205, 0x167, font1, 0x10);
        show_temple_tip();
    }

    hold_mouse_replace = 1;
    refresh_svga_screen();
    set_palette(temp_palette);
}

// Show the current temple advice, skipping unsuitable early tips in peaceful play.
// FUNCTION: C2 0x5f743
// FUNCTION: C2WIN 0x00427122
void show_temple_tip(void)
{
    stone_random_count = 0x1e;
    show_a_mosaic_blank(0x10, 0x19c, 0x26, 3);
    draw_a_dias(0xf, 0x19b, 0x262, 0x32);
    if (c2inf.peace_mode != 0) {
        if (current_temple_tip < 9)
            current_temple_tip = 0x11;
    }
    font_format_split(0x20, current_temple_tip + 7, 0x18, 0x19e,
                      0x258, 0x64, 0, 0, font1, 0x10);
    setup_refresh_area(0, 0x190, 0x28, 5, 1);
}

// Show the clerks forum with historical graphs and range controls.
// FUNCTION: C2 0x5f7e2
// FUNCTION: C2WIN 0x004271af
void forum_clerks_screen(void)
{
    int department_idx;

    cover_mouse_droppings();
    setup_whole_screen_refresh();

    get_history_in_buffer(((int *)((scratch_buffer) + 0x1fbd0)));

    department_idx = last_forum_dept;
    if (forum_repapering[department_idx] != 0) {
        show_pl8file("forum.pl8", forum_repapering[department_idx]);
    }

    explain_forum();
    show_a_mosaic_frame(0, 0, 0x28, 0xf);

    stone_random_count = 0x28;

    show_a_mosaic_blank(0x10, 0x10, 0x26, 0xd);
    x_is = 0;
    font_list(0x21, 0, 0x18, 0x1c, font2, 0x10);

    history_graphs();
    history_selection();

    show_buttons(0x40, 0x6c, clerk_buttons, 2);

    hold_mouse_replace = 1;
    refresh_svga_screen();
    set_palette(temp_palette);
}

// Show the selected time span and controls for the history graphs.
// FUNCTION: C2 0x5f89e
// FUNCTION: C2WIN 0x00427286
void history_selection(void)
{
    int years;

    stone_random_count = 0x29;

    show_a_mosaic_blank(0x10, 0x40, 4, 8);
    font_list(0x21, 5, 0x20, 0x44, font1, 0x10);
    font_list(0x21, 6, 0x28, 0x58, font1, 0x10);

    x_is  = 0;
    years = history_graph_years[history_graph_length];

    show_a_mosaic_blank(0x10, 0x8e, 0xa, 5);
    font_no(years, 0x20, " ", 0x38, 0x90, font1, 0x10);
    font_list(0x21, 7, x_is + 0x38, 0x90, font1, 0x10);
    x_is = 0;

    show_date(year - years, 0x12, 0xb0, 1);
    font_list(0x21, 8, x_is + 0x14, 0xb0, font1, 0x10);
    show_date(year, x_is + 0x16, 0xb0, 1);

    setup_refresh_area(0x10, 0x84, 0xb, 7, 1);
}

// Draw the population, treasury, employment, and reputation history graphs.
// FUNCTION: C2 0x5f9f0
// FUNCTION: C2WIN 0x004273b1
void history_graphs(void)
{
    int top;

    stone_random_count = 0x28;
    show_a_mosaic_blank(0xb0, 0x10, 0x1c, 0xd);

    x_is = 0x10;
    font_list(0x21, 1, 0xc0, 0x83, font1, 0x10);
    top = show_history_graph(0xb0, 0x18, 0);
    font_no(top, 0x20, " ", x_is + 0xb0, 0x83, font1, 0x10);

    x_is = 0x18;
    font_list(0x21, 2, 0x1ac, 0x83, font1, 0x10);
    top = show_history_graph(0x194, 0x18, 1);
    font_no(top, 0x20, " ", x_is + 0x194, 0x83, font1, 0x10);

    x_is = 8;
    font_list(0x21, 3, 0xb8, 0xce, font1, 0x10);
    top = show_history_graph(0xb0, 0x96, 2);
    font_no(top, 0x20, " ", x_is + 0xb0, 0xce, font1, 0x10);

    x_is = 0xc;
    font_list(0x21, 4, 0x1a0, 0xce, font1, 0x10);
    top = show_history_graph(0x194, 0x96, 3);
    font_no(top, 0x20, " ", x_is + 0x194, 0xce, font1, 0x10);

    setup_refresh_area(0xb0, 0x10, 0x1d, 0xe, 1);
}

// Plot one historical statistic over the selected number of years.
// FUNCTION: C2 0x5fbba
// FUNCTION: C2WIN 0x004275a1
int show_history_graph(int graph_x, int graph_y, int statistic_idx)
{
    int years   = history_graph_years[history_graph_length];
    int x_step   = 0xc8 / years;
    int sample_idx;
    int max_value = 0;
    int top_value;
    int multiplier;
    int graph_height;
    int divisor;
    int i;
    int value;

    sample_idx = history_end_ptr - years;
    if (sample_idx < 0) sample_idx += 0xc8;
    for (i = 0; i < years; i++) {
        value = get_history_from_buffer(((int *)((scratch_buffer) + 0x1fbd0)),
                                    sample_idx, statistic_idx);
        if (value > max_value) max_value = value;
        if (++sample_idx >= 0xc8) sample_idx = 0;
    }

    multiplier = divisor = 1;
    if (1 >= statistic_idx) {
        if      (max_value <= 0x32)    { top_value = 0x32;    multiplier = 2; }
        else if (max_value <= 0x64)    { top_value = 0x64;    }
        else if (max_value <= 0xc8)    { top_value = 0xc8;    divisor = 2; }
        else if (max_value <= 0x1f4)   { top_value = 0x1f4;   divisor = 5; }
        else if (max_value <= 0x3e8)   { top_value = 0x3e8;   divisor = 0xa; }
        else if (max_value <= 0x9c4)   { top_value = 0x9c4;   divisor = 0x19; }
        else if (max_value <= 0x1388)  { top_value = 0x1388;  divisor = 0x32; }
        else if (max_value <= 0x2710)  { top_value = 0x2710;  divisor = 0x64; }
        else if (max_value <= 0x61a8)  { top_value = 0x61a8;  divisor = 0xfa; }
        else if (max_value <= 0xc350)  { top_value = 0xc350;  divisor = 0x1f4; }
        else if (max_value <= 0x186a0) { top_value = 0x186a0; divisor = 0x3e8; }
        else                         { top_value = 0xf4240; divisor = 0x2710; }
        graph_height = 0x64;
    } else {
        if      (max_value <= 0xa)     { top_value = 0xa;     multiplier = 5; }
        else if (max_value <= 0x19)    { top_value = 0x19;    multiplier = 2; }
        else if (max_value <= 0x32)    { top_value = 0x32;    }
        else if (max_value <= 0x64)    { top_value = 0x64;    divisor = 2; }
        else if (max_value <= 0xc8)    { top_value = 0xc8;    divisor = 4; }
        else if (max_value <= 0x1f4)   { top_value = 0x1f4;   divisor = 0xa; }
        else if (max_value <= 0x3e8)   { top_value = 0x3e8;   divisor = 0x14; }
        else if (max_value <= 0x7d0)   { top_value = 0x7d0;   divisor = 0x28; }
        else if (max_value <= 0xfa0)   { top_value = 0xfa0;   divisor = 0x50; }
        else if (max_value <= 0x1f40)  { top_value = 0x1f40;  divisor = 0xa0; }
        else if (max_value <= 0x2710)  { top_value = 0x2710;  divisor = 0xc8; }
        else if (max_value <= 0x4e20)  { top_value = 0x4e20;  divisor = 0x190; }
        else                         { top_value = 0xc350;  divisor = 0x3e8; }
        graph_height = 0x32;
    }

    sample_idx = history_end_ptr - years;
    if (sample_idx < 0) sample_idx += 0xc8;
    draw_a_dias(graph_x, graph_y, 0xca, graph_height + 2);
    draw_a_rect(graph_x + 1, graph_y + 1, 0xc8, graph_height, 0x20);

    for (i = 0; i < years; i++) {
        int colour;
        int bar_y;
        value = get_history_from_buffer(((int *)((scratch_buffer) + 0x1fbd0)),
                                    sample_idx, statistic_idx);
        if (++sample_idx >= 0xc8) sample_idx = 0;
        if (value <= 0) continue;
        if (value > top_value) continue;
        value = value / divisor;
        value = value * multiplier;
        if (value < 0) continue;
        if (value > 0xc8) continue;
        colour = (i & 1) ? 0xa : 0xd;
        bar_y = graph_y + 1 + graph_height - value;
        if (bar_y < graph_y) continue;
        if (bar_y + value > graph_y + 0xc9) continue;
        draw_a_rect(graph_x + 1 + i * x_step, bar_y, x_step, value, colour);
    }

    return top_value;
}

// Show the empire overview, or the city-only forum during peaceful play.
// FUNCTION: C2 0x5ffc8
// FUNCTION: C2WIN 0x00427ae3
void forum_empire_screen(void)
{
    if (c2inf.peace_mode != 0) {
        forum_city_only_screen();
        return;
    }
    black_out();
    readfile("empire.256", temp_palette, 0x300, 0);
    readfile("e_parts2.pl8", ((void *)scratch_buffer), 0x249f0, 0);
    basic_empire_screen();
    set_palette(temp_palette);
    readfile("forum.256", temp_palette, 0x300, 0);
}

// Draw the empire map, province markers, frame, and headings.
// FUNCTION: C2 0x60038
// FUNCTION: C2WIN 0x00427b6a
void basic_empire_screen(void)
{
    show_pl8file("empire.pl8", 0x1e0);
    show_regions_in_empire();
    show_empire_top_slab();
    show_empire_bottom_slab();
    font_list(0x22, 1, 0xbe, 0x19e, font1, 0x3f);
    font_list(0x22, 3, 0xd0, 0x1ce, font1, 0x20);
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
    hold_mouse_replace = 1;
    refresh_svga_screen();
}

// Mark the player's provinces on the empire map, then draw the remaining offers.
// FUNCTION: C2 0x600a3
// FUNCTION: C2WIN 0x00427bdc
void show_regions_in_empire(void)
{
    int i;

    for (i = 0; i < 0x2c; i++) {
        if (empire[i] == 6) {
            write_general_sprite(i, empire_positions[i].x,
                                    empire_positions[i].y);
        }
    }
    show_regions_on_offer();
}

// Mark provinces that are available, active, won, or completed on the empire map.
// FUNCTION: C2 0x600db
// FUNCTION: C2WIN 0x00427c45
void show_regions_on_offer(void)
{
    int i;

    for (i = 0; i < 0x2c; i++) {
        if (empire[i] == 2) {
            write_image(misc, 7, empire_flag_positions[i].x - 5,
                        empire_flag_positions[i].y - 0x20);
        }
        if (empire[i] == 6) {
            if (empire_won[i] == 0) {
                write_image(misc, 6, empire_flag_positions[i].x - 5,
                            empire_flag_positions[i].y - 0x20);
            }
            if (empire_won[i] == 0x1869f) {
                write_image(misc, 8, empire_flag_positions[i].x - 5,
                            empire_flag_positions[i].y - 0x20);
            }
            if (empire_won[i] < 0x1869e) {
                write_image(misc, 5, empire_flag_positions[i].x - 5,
                            empire_flag_positions[i].y - 0x20);
            }
        }
    }
}

// Draw the upper decorative sections of the empire screen.
// FUNCTION: C2 0x601cc
// FUNCTION: C2WIN 0x00427d86
void show_empire_top_slab(void)
{
    write_general_sprite(0x2c, 0xd2, 0x1a);
    write_general_sprite(0x2e, 0x112, 0x1a);
    write_general_sprite(0x2c, 0x132, 0x1a);
    write_general_sprite(0x2d, 0x16c, 0x1a);
}

// Draw the lower decorative sections of the empire screen.
// FUNCTION: C2 0x60221
// FUNCTION: C2WIN 0x00427dd5
void show_empire_bottom_slab(void)
{
    write_general_sprite(0x2d, 0xb8, 0x199);
    write_general_sprite(0x2c, 0xf8, 0x199);
    write_general_sprite(0x2e, 0x124, 0x199);
    write_general_sprite(0x2c, 0x144, 0x199);
    write_general_sprite(0x2d, 0x184, 0x199);
}

// Show the army forum with cohort, recruitment, tribune, and mercenary information.
// FUNCTION: C2 0x6027f
// FUNCTION: C2WIN 0x00427e44
void forum_army_screen(void)
{
    if (c2inf.peace_mode != 0) {
        forum_city_only_screen();
        return;
    }

    check_viewed_cohort();
    fill_cohort_centuries();
    total_no_of_cohorts = no_of_cohorts_in_action;

    cover_mouse_droppings();
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif

    if (forum_repapering[last_forum_dept] != 0) {
        show_pl8file("forum.pl8",
                     forum_repapering[last_forum_dept]);
    }

    explain_forum();
    show_a_mosaic_frame(0, 0, 0x28, 0xe);
    stone_random_count = 0x3c;
    show_a_mosaic_blank(0x10, 0x10, 0x26, 0xc);

    x_is = 0;
    font_list(0x23, 0, 0x18, 0x14, font2, 0x10);

    show_recruitment();
    show_this_tribune();
    show_mercs();

    no_of_army_buttons = 5;
    if (total_no_of_cohorts > 0) {
        if (forum_viewed_army == 0xa) {
            no_of_army_buttons = 7;
        } else {
            no_of_army_buttons = 8;
        }
    }
    show_buttons(0x18, 0x30, army_buttons, no_of_army_buttons);

    if (max_mercs_allowed != 0) {
        show_buttons(0x18, 0x30, mercenary_buttons, 2);
    }

    hold_mouse_replace = 1;
    refresh_svga_screen();
    set_palette(temp_palette);
}

// Show available mercenaries, their origin and type, and their upkeep cost.
// FUNCTION: C2 0x603c1
// FUNCTION: C2WIN 0x00427f9b
void show_mercs(void)
{
    if (max_mercs_allowed != 0) {
        show_a_mosaic_blank(0x16, 0x9c, 0xa, 3);
        setup_refresh_area(0x16, 0x9c, 0xb, 4, 2);

        x_is = 0;
        font_no(mercs_in_army, 0x20, " ", 0x18, 0x9e, font1, 0x10);
        font_list(0x23, 0x1f, x_is + 0x18, 0x9e, font1, 0x10);

        x_is = 0;
        font_list(7,    mercs_from,        0x18, 0xae, font1, 0x10);
        font_list(0x47, mercs_type + 0xa,  x_is + 0x18, 0xae, font1, 0x10);

        x_is = 0;
        font_list(0x23, 0x20, 0x18, 0xbe, font1, 0x10);
        font_no(mercs_in_army / 50 * mercs_cost_per_50, 0x20, "Dn",
                x_is + 0x18, 0xbe, font1, 0x10);
        font_list(0x23, 0x21, x_is + 0x18, 0xbe, font1, 0x10);
    } else {
        font_list(0x23, 0x22, 0x18, 0x9e, font1, 0x10);
        font_list(0x23, 0x23, 0x18, 0xae, font1, 0x10);
        font_list(0x23, 0x24, 0x18, 0xbe, font1, 0x10);
    }
}

// Show army-wide recruitment totals or the selected cohort's composition and status.
// FUNCTION: C2 0x6058a
// FUNCTION: C2WIN 0x0042813c
void show_this_tribune(void)
{
    stone_random_count = 0xf;

    if (forum_viewed_army == 0xa) {
        show_a_mosaic_blank(0x108, 0x1e, 0x16, 9);
        show_a_mosaic_blank(0x108, 0xa6, 0x16, 1);
        draw_a_dias(0x100, 0x1c, 0x168, 0x98);

        font_list(0x23, 0xc, 0x110, 0x22, font1, 0x10);
        font_list(0x23, 0xd, 0x1c0, 0x22, font1, 0x10);

        x_is = 0;
        font_no(total_no_of_regulars, 0x20, " ",
                0x110, 0x3c, font1, 0x10);
        font_list(0x23, 0xe, x_is + 0x110, 0x3c, font1, 0x10);
        x_is = 0;
        if (total_no_of_regulars >= current_no_of_regulars)
            font_no(current_no_of_regulars, 0x20, " ", 0x190, 0x3c, font1, 0x10);
        else
            font_no(total_no_of_regulars, 0x20, " ", 0x190, 0x3c, font1, 0x10);
        font_list(0x23, 0x11, x_is + 0x190, 0x3c, font1, 0x10);
        font_no(needed_no_of_regulars, 0x20, " ",
                x_is + 0x1a0, 0x3c, font1, 0x10);
        font_list(0x23, 0x12, x_is + 0x1a0, 0x3c, font1, 0x10);

        x_is = 0;
        font_no(total_no_of_irregulars, 0x20, " ",
                0x110, 0x4c, font1, 0x10);
        font_list(0x23, 0xf, x_is + 0x110, 0x4c, font1, 0x10);
        x_is = 0;
        if (total_no_of_irregulars >= current_no_of_irregulars)
            font_no(current_no_of_irregulars, 0x20, " ", 0x190, 0x4c, font1, 0x10);
        else
            font_no(total_no_of_irregulars, 0x20, " ", 0x190, 0x4c, font1, 0x10);
        font_list(0x23, 0x11, x_is + 0x190, 0x4c, font1, 0x10);
        font_no(needed_no_of_irregulars, 0x20, " ",
                x_is + 0x1a0, 0x4c, font1, 0x10);
        font_list(0x23, 0x12, x_is + 0x1a0, 0x4c, font1, 0x10);

        x_is = 0;
        font_no(total_no_of_auxillaries, 0x20, " ",
                0x110, 0x5c, font1, 0x10);
        font_list(0x23, 0x10, x_is + 0x110, 0x5c, font1, 0x10);
        x_is = 0;
        if (total_no_of_auxillaries >= current_no_of_auxillaries)
            font_no(current_no_of_auxillaries, 0x20, " ", 0x190, 0x5c, font1, 0x10);
        else
            font_no(total_no_of_auxillaries, 0x20, " ", 0x190, 0x5c, font1, 0x10);
        font_list(0x23, 0x11, x_is + 0x190, 0x5c, font1, 0x10);
        font_no(needed_no_of_auxillaries, 0x20, " ",
                x_is + 0x1a0, 0x5c, font1, 0x10);
        font_list(0x23, 0x12, x_is + 0x1a0, 0x5c, font1, 0x10);

        x_is = 0;
        font_no(mercs_in_army, 0x20, " ",
                0x110, 0x6c, font1, 0x10);
        font_list(0x23, 0x1f, x_is + 0x110, 0x6c, font1, 0x10);

        if (total_no_of_cohorts != 0) {
            font_list(0x23, average_cohort_morale + 0x13,
                      0x110, 0x8c, font1, 0x10);
            font_list(0x23, average_cohort_readiness + 0x18,
                      0x110, 0x9c, font1, 0x10);
        } else {
            font_format_split(0x4e, 0, 0x110, 0x84,
                              0x140, 0x64, 0, 0, font1, 0x10);
        }
    } else {
        temp_army = (short)get_actual_viewed_army();
        show_a_mosaic_blank(0x108, 0x1e, 0x16, 9);
        show_a_mosaic_blank(0x108, 0xa6, 0x16, 1);
        draw_a_dias(0x100, 0x1c, 0x168, 0x98);

        show_tribunes_report(temp_army, 0x108, 0x22, 1);

        write_general_sprite(army_list[temp_army].cohort_id,
                             0x240, 0x20);
        write_general_sprite(0x19, 0x240, 0x32);
        font_list(5, army_list[temp_army].cohort_id,
                  0x1c0, 0x22, font1, 0x10);
    }

    update_tribune_flag(1);
    setup_refresh_area(0x108, 0x1e, 0x17, 0xb, 1);
}

// Animate the selected cohort's flag in the city or forum panel.
// FUNCTION: C2 0x60b15
// FUNCTION: C2WIN 0x004286d5
void update_tribune_flag(int mode)
{
    short army;

    request_message.tribune_flag_counter += 1;
    if (request_message.tribune_flag_counter >= 0x40) {
        request_message.tribune_flag_counter = 0;
    }
    army = get_actual_viewed_army();
    temp_army = army;
    if (army_list[army].state_idx == 0xa) {
        request_message.tribune_flag_counter = 0;
    }
    stone_random_count = 0xb;
    if (mode == 0) {
        show_a_mosaic_blank(0x190, 0x56, 2, 2);
        write_general_sprite((request_message.tribune_flag_counter >> 3) + 0x3a, 0x190, 0x56);
        setup_refresh_area(0x190, 0x56, 2, 2, 2);
    } else {
        if (forum_viewed_army == 0xa) return;
        show_a_mosaic_blank(0x240, 0x32, 2, 2);
        write_general_sprite((request_message.tribune_flag_counter >> 3) + 0x19, 0x240, 0x32);
        setup_refresh_area(0x240, 0x32, 2, 2, 2);
    }
}

// Show recruitment progress and the selected cohort's engagement status.
// FUNCTION: C2 0x60c04
// FUNCTION: C2WIN 0x004287b8
void show_recruitment(void)
{
    int idx;
    int auto_fight;

    stone_random_count = 0x15;
    show_a_mosaic_blank(0x1e0, 0xb6, 9, 1);
    show_a_mosaic_blank(0x1e0, 0xc0, 9, 1);

    predict_army_totals();
    temp_army = (short)get_actual_viewed_army();

    show_a_mosaic_blank(0xc0, 0x3a, 4, 3);

    x_is = 0;
    font_list(0x23, 1, 0x18, 0x3c, font1, 0x10);
    font_no(army_wage_level, 0x20, "Dn",
            0xcc, 0x3c, font1, 0x10);
    x_is = 0;
    font_list(0x23, 2, 0x18, 0x54, font1, 0x10);
    font_no(conscription_rate, 0x20, "%",
            0xcc, 0x54, font1, 0x10);

    show_a_mosaic_blank(0x18, 0x68, 0xe, 2);
    show_a_mosaic_blank(0x18, 0x80, 0xe, 1);

    x_is = 0;
    font_list(0x23, 4, 0x18, 0x6c, font1, 0x10);
    font_no(total_no_of_soldiers, 0x20, " ",
            x_is + 0x18, 0x6c, font1, 0x10);
    font_list(0x23, 5, x_is + 0x18, 0x6c, font1, 0x10);

    x_is = 0;
    if (total_no_of_soldiers >= current_no_of_soldiers)
        font_no(current_no_of_soldiers, 0x20, " ",
                0x18, 0x7c, font1, 0x10);
    else
        font_no(total_no_of_soldiers, 0x20, " ",
                0x18, 0x7c, font1, 0x10);
    font_list(0x23, 6, x_is + 0x18, 0x7c, font1, 0x10);
    font_no(needed_no_of_soldiers, 0x20, " ",
            x_is + 0x28, 0x7c, font1, 0x10);
    font_list(0x23, 7, x_is + 0x28, 0x7c, font1, 0x10);

    x_is = 0;
    if (total_no_of_cohorts == 0) {
        font_list(0x23, 8, 0x160, 0xba, font1, 0x10);
    } else if (total_no_of_cohorts == 1) {
        font_list(0x23, 9, 0x100, 0xba, font1, 0x10);
    } else {
        font_list(0x23, 0xa, 0x100, 0xba, font1, 0x10);
        font_no(total_no_of_cohorts, 0x20, " ",
                x_is + 0x100, 0xba, font1, 0x10);
        font_list(0x23, 0xb, x_is + 0x100, 0xba, font1, 0x10);
    }

    if (forum_viewed_army != 0xa) {
        idx        = temp_army;
        auto_fight = army_list[idx].state_idx;
        if (auto_fight == 0xa) {
            font_list(0x23, 0x1d, 0x1e8, 0xbc, font1, 0x10);
        } else {
            if      (army_list[idx].cohort_size_class == 0) font_list(0x23, 0x1e, 0x1e8, 0xbc, font1, 0x10);
            else if (army_list[idx].cohort_size_class == 1) font_list(0x23, 0x25, 0x1e8, 0xbc, font1, 0x10);
            else if (army_list[idx].cohort_size_class == 2) font_list(0x23, 0x26, 0x1e8, 0xbc, font1, 0x10);
        }
    }

    setup_refresh_area(-8, 0x10, 0x10, 6, 1);
    setup_refresh_area(0x18, 0x6a, 0xe, 3, 1);
    setup_refresh_area(0x1e0, 0xb6, 9, 3, 1);
}

// Show the province's industries, staffing, supplies, output, and trade status.
// FUNCTION: C2 0x6100b
// FUNCTION: C2WIN 0x00428be0
void forum_industry_screen(void)
{
    int i;
    int hasup;
    int kind;
    int trader;
    int pipe2;
    int supply;
    int diff;
    int citysup;
    int image;

    cover_mouse_droppings();
    setup_whole_screen_refresh();
    if (forum_repapering[last_forum_dept] != 0) show_pl8file("forum.pl8", forum_repapering[last_forum_dept]);
    explain_forum();
    show_a_mosaic_frame(0, 0, 0x28, 0xe);
    stone_random_count = 0x10;
    show_a_mosaic_blank(0x10, 0x10, 0x26, 0xc);

    x_is = 0;
    font_list(0x24, 0, 0x18, 0x14, font2, 0x10);

    if (c2inf.peace_mode == 0) {
        for (i = 0; i < 8; i++) {
            kind   = province_industries[i].kind;
            trader = province_industries[i].is_trader;
            if (i >= 4) diff = ((unsigned char *)provincial_difficulty + 0xac)[i + province_is * 4];
            supply  = industry[kind].supply;
            pipe2   = industry[kind].supply_pipeline[2];
            hasup   = industry[kind].has_supply;
            citysup = industry[kind].city_supply;

            write_image(game_panels, kind + 0x3b, 0x20, i * 0x13 + 0x37);
            font_list(0x10, kind + 1, 0x38, i * 0x13 + 0x39, font1, 0x10);

            if (trader == 0) {
                x_is = 0;
                font_no(pipe2, 0x20, " ", 0x7c, i * 0x13 + 0x39, font1, 0x10);
                font_list(0x24, 1, x_is + 0x7c, i * 0x13 + 0x39, font1, 0x10);
            } else {
                if (trader == 1) image = 0x4b;
                else             image = 0x4c;
                write_image(game_panels, image, 0x7c, i * 0x13 + 0x37);
                font_list(6, diff + 1, 0x90, i * 0x13 + 0x39, font1, 0x10);
            }

            font_no(supply, 0x20, " ", 0x140, i * 0x13 + 0x39, font1, 0x10);
            font_list(0x24, 2, 0x154, i * 0x13 + 0x39, font1, 0x10);
            if (hasup == 0) font_list(0x24, 3, 0x1ae, i * 0x13 + 0x39, font1, 0x10);
            else {
                x_is = 0;
                font_no(hasup, 0x20, " ", 0x1ae, i * 0x13 + 0x39, font1, 0x10);
                if (hasup == 1) font_list(0x24, 4, x_is + 0x1ae, i * 0x13 + 0x39, font1, 0x10);
                else            font_list(0x24, 5, x_is + 0x1ae, i * 0x13 + 0x39, font1, 0x10);
                font_no(citysup, 0x20, "%", x_is + 0x1ae, i * 0x13 + 0x39, font1, 0x10);
                font_list(0x24, 6, x_is + 0x1ae, i * 0x13 + 0x39, font1, 0x10);
            }
        }
    } else {
        for (i = 0; i < 8; i++) {
            kind    = i * 2 + 1;
            hasup   = industry[kind].has_supply;
            citysup = industry[kind].city_supply;

            write_image(game_panels, kind + 0x3b, 0x20, i * 0x13 + 0x37);
            font_list(0x10, kind + 1, 0x38, i * 0x13 + 0x39, font1, 0x10);
            if (hasup == 0) font_list(0x24, 3, 0x8c, i * 0x13 + 0x39, font1, 0x10);
            else {
                x_is = 0;
                font_no(hasup, 0x20, " ", 0x8c, i * 0x13 + 0x39, font1, 0x10);
                if (hasup == 1) font_list(0x24, 4, x_is + 0x8c, i * 0x13 + 0x39, font1, 0x10);
                else            font_list(0x24, 5, x_is + 0x8c, i * 0x13 + 0x39, font1, 0x10);
                font_no(citysup, 0x20, "%", x_is + 0x8c, i * 0x13 + 0x39, font1, 0x10);
                font_list(0x24, 6, x_is + 0x8c, i * 0x13 + 0x39, font1, 0x10);
            }
        }
    }

    hold_mouse_replace = 1;
    refresh_svga_screen();
    set_palette(temp_palette);
}

// Show slave population, welfare, and labor allocation in the forum.
// FUNCTION: C2 0x6147e
// FUNCTION: C2WIN 0x004291b5
void forum_slaves_screen(void)
{
    int delta;

    cover_mouse_droppings();
    setup_whole_screen_refresh();

    if (forum_repapering[last_forum_dept] != 0) {
        show_pl8file("forum.pl8",
                     forum_repapering[last_forum_dept]);
    }

    explain_forum();
    show_a_mosaic_frame(0, 0, 0x28, 0xe);
    stone_random_count = 0x20;
    show_a_mosaic_blank(0x10, 0x10, 0x26, 0xc);

    font_list(0x25, 0, 0x18, 0x1a, font2, 0x10);

    x_is = 0;
    font_no(slaves, 0x20, " ", 0x18, 0x42, font1, 0x10);
    font_list(0x25, 1, x_is + 0x18, 0x42, font1, 0x10);

    x_is = 0;
    if (slave_population_change < 0) {
        font_list(0x25, 2, 0x18, 0x53, font1, 0x10);
        font_no(-slave_population_change, 0x20, " ", x_is + 0x18, 0x53, font1, 0x10);
    } else if (slave_population_change > 0) {
        font_list(0x25, 3, 0x18, 0x53, font1, 0x10);
        font_no(slave_population_change,  0x20, " ", x_is + 0x18, 0x53, font1, 0x10);
    } else {
        font_list(0x25, 4, 0x18, 0x53, font1, 0x10);
    }

    font_list(0x25, 5, x_is + 0x18, 0x53, font1, 0x10);

    show_slave_welfare_bill();
    show_slave_allocation();

    show_buttons(0x40, 0x72, slave1_buttons, 2);
    show_buttons(0x1ae, 0x26, slave2_buttons, 0xc);

    hold_mouse_replace = 1;
    refresh_svga_screen();
    set_palette(temp_palette);
}

// Show current and projected slave welfare costs and population changes.
// FUNCTION: C2 0x61661
// FUNCTION: C2WIN 0x004293ea
void show_slave_welfare_bill(void)
{
    slave_estimate();
    stone_random_count = 0x16;
    show_a_mosaic_blank(0x18, 0x71, 0x10, 1);
    show_a_mosaic_blank(0x18, 0x79, 0x10, 5);

    font_list(0x25, 6, 0x18, 0x7a, font1, 0x10);
    font_no(slave_welfare_bill, 0x20, " Dn",
            0x78, 0x7a, font1, 0x10);
    font_list(0x25, 7, 0x18, 0x8b, font1, 0x10);
    font_list(0x25, 8, 0x18, 0x9c, font1, 0x10);

    x_is = 0;
    if (slave_population_estimate < slaves) {
        font_list(0x25, 9, 0x18, 0xad, font1, 0x10);
        font_no(slave_population_estimate, 0x20, " ",
                x_is + 0x18, 0xad, font1, 0x10);
    } else if (slave_population_estimate > slaves) {
        font_list(0x25, 0xa, 0x18, 0xad, font1, 0x10);
        font_no(slave_population_estimate, 0x20, " ",
                x_is + 0x18, 0xad, font1, 0x10);
    } else {
        font_list(0x25, 0xb, 0x18, 0xad, font1, 0x10);
    }

    x_is = 0;
    font_list(0x4d, 1, 0x18, 0xbe, font1, 0x10);
    font_no(slave_population_final_estimate, 0x20, " ",
            x_is + 0x18, 0xbe, font1, 0x10);

    setup_refresh_area(0x18, 0x62, 0x14, 9, 1);
}

// Show current and maximum slave allocations for each industry.
// FUNCTION: C2 0x6182d
// FUNCTION: C2WIN 0x004295b2
void show_slave_allocation(void)
{
    int i;

    stone_random_count = 0xc;
    show_a_mosaic_blank(0x120, 0x10, 0x15, 0xc);

    font_list(0x25, 0xc, 0x118, 0x16, font1, 0x10);
    font_list(0x25, 0xd, 0x118, 0x2e, font1, 0x10);
    font_list(0x25, 0xe, 0x118, 0x46, font1, 0x10);
    font_list(0x25, 0xf, 0x118, 0x5e, font1, 0x10);
    font_list(0x25, 0x10, 0x118, 0x76, font1, 0x10);
    font_list(0x25, 0x11, 0x118, 0x8e, font1, 0x10);
    font_list(0x25, 0x12, 0x118, 0xa6, font1, 0x10);
    font_list(0x25, 0x13, 0x118, 0xbe, font1, 0x10);

    for (i = 0; i < 7; i++) {
        draw_a_dias(0x216, i * 0x18 + 0x12, 0x50, 0x14);
        if (i >= 5 && c2inf.peace_mode != 0) {
            font_list(0x25, 0x15, 0x1ea, i * 0x18 + 0x16, font1, 0x10);
            font_list(0x25, 0x15, 0x21a, i * 0x18 + 0x16, font1, 0x10);
        } else {
            font_no(slave_requirements[i].current, 0x20, " ",
                    0x1ea, i * 0x18 + 0x16, font1, 0x10);
            x_is = 0;
            font_list(0x25, 0x14, 0x21a, i * 0x18 + 0x16, font1, 0x10);
            font_no(slave_requirements[i].max, 0x20, " ",
                    x_is + 0x21a, i * 0x18 + 0x16, font1, 0x10);
        }
    }

    font_no(slave_requirements[i].current, 0x20, " ",
            0x1ea, i * 0x18 + 0x16, font1, 0x10);

    setup_refresh_area(0x18e, 6, 0x17, 0xe, 1);
}

// Show the annual report, including the governor's appraisal and financial results.
// FUNCTION: C2 0x61a99
// FUNCTION: C2WIN 0x00429825
void show_year_end_screen(void)
{
    cover_mouse_droppings();
    setup_whole_screen_refresh();
    stone_random_count = 0xf;
    show_a_mosaic_frame(0x10, 0x30, 0x1c, 0xf);
    show_a_mosaic_blank(0x20, 0x40, 0x1a, 0xd);
    show_an_exit_button(0x1a4, 0xf4);
    draw_a_dias(0x30, 0x70, 0x180, 0x70);

    x_is = 0;
    font_list(0x49, 0, 0x30, 0x4c, font2, 0x10);
    font_list(0x49, 1, x_is + 0x30, 0x54, font1, 0x10);

    x_is = 0;
    font_list(0x49, 3, 0x40, 0x80, font1, 0x10);
    font_no(this_years_population, 0x20, " ",
            x_is + 0x40, 0x80, font1, 0x10);
    x_is = 0;
    if (this_years_population >= last_years_population) {
        font_list(0x49, 7, 0x120, 0x80, font1, 0x10);
        font_no(this_years_population - last_years_population,
                0x20, ")", x_is + 0x120, 0x80, font1, 0x10);
    } else {
        font_list(0x49, 8, 0x120, 0x80, font1, 0xb);
        font_no(last_years_population - this_years_population,
                0x20, ")", x_is + 0x120, 0x80, font1, 0xb);
    }

    x_is = 0;
    font_list(0x49, 4, 0x40, 0x92, font1, 0x10);
    if (this_years_denarii >= 0)
        font_no(this_years_denarii, 0x20, "Dn",
                x_is + 0x40, 0x92, font1, 0x10);
    else
        font_no(-this_years_denarii, 0x2d, "Dn",
                x_is + 0x40, 0x92, font1, 0xb);
    x_is = 0;
    if (this_years_denarii >= last_years_denarii) {
        font_list(0x49, 7, 0x120, 0x92, font1, 0x10);
        font_no(this_years_denarii - last_years_denarii,
                0x20, "Dn)", x_is + 0x120, 0x92, font1, 0x10);
    } else {
        font_list(0x49, 8, 0x120, 0x92, font1, 0xb);
        font_no(last_years_denarii - this_years_denarii,
                0x20, "Dn)", x_is + 0x120, 0x92, font1, 0xb);
    }

    x_is = 0;
    font_list(0x49, 5, 0x40, 0xa4, font1, 0x10);
    font_no(this_years_pop_tax, 0x20, "Dn",
            x_is + 0x40, 0xa4, font1, 0x10);
    x_is = 0;
    if (this_years_pop_tax >= last_years_pop_tax) {
        font_list(0x49, 7, 0x120, 0xa4, font1, 0x10);
        font_no(this_years_pop_tax - last_years_pop_tax,
                0x20, "Dn)", x_is + 0x120, 0xa4, font1, 0x10);
    } else {
        font_list(0x49, 8, 0x120, 0xa4, font1, 0xb);
        font_no(last_years_pop_tax - this_years_pop_tax,
                0x20, "Dn)", x_is + 0x120, 0xa4, font1, 0xb);
    }

    x_is = 0;
    font_list(0x49, 6, 0x40, 0xb6, font1, 0x10);
    font_no(this_years_ind_tax, 0x20, "Dn",
            x_is + 0x40, 0xb6, font1, 0x10);
    x_is = 0;
    if (this_years_ind_tax >= last_years_ind_tax) {
        font_list(0x49, 7, 0x120, 0xb6, font1, 0x10);
        font_no(this_years_ind_tax - last_years_ind_tax,
                0x20, "Dn)", x_is + 0x120, 0xb6, font1, 0x10);
    } else {
        font_list(0x49, 8, 0x120, 0xb6, font1, 0xb);
        font_no(last_years_ind_tax - this_years_ind_tax,
                0x20, "Dn)", x_is + 0x120, 0xb6, font1, 0xb);
    }

    font_format_split(0x49, 2, 0x30, 0xf0, 0x12c, 0x64,
                      0, 0, font1, 0x10);
    hold_mouse_replace = 1;
    refresh_svga_screen();
}

// Update the date, treasury, and warning indicators in the top status bar.
// FUNCTION: C2 0x61fad
// FUNCTION: C2WIN 0x00429d9f
void show_top_line(void)
{
    char dirty = 0;

    if (--request_message.alarm_blink_timer <= 0) {
        request_message.alarm_blink_state ^= 1;
        request_message.alarm_blink_timer = 0x28;
        dirty = 1;
    }
    if (redraw_topline != 0)                                  dirty = 1;
    if (request_message.cached_month != month)   dirty = 1;
    if (request_message.cached_denarii != denarii) dirty = 1;
    if (!dirty)
        return;

    redraw_topline = 0;
    request_message.cached_month = month;
    request_message.cached_denarii = denarii;

    sprite_width  = 0x15;
    sprite_height = 0xf;
    show_fast_rect(0x12e, 5, 0x1a);

    if (pointer_mode == 5)
        request_message.alarm_blink_state = 1;

    if (slave_warning == 0
        || request_message.alarm_blink_state != 0
        || map_mode == 2) {
        show_date(year, 0x130, 6, 0);
        font_list(0x19, month, x_is + 0x138, 6, font1, 0x10);
    } else {
        font_list(9, 3, 0x138, 6, font1, 0xb);
        if (++request_message.alarm_chime_counter >= 8) {
            request_message.alarm_chime_counter = 0;
            set_pri_sound("a09.wav", 1);
        }
    }
    if (map_mode != 2) {
        if (denarii < 0)
            font_no(-denarii, 0x2d, " Dn",
                    0x230, 6, font1, 0xb);
        else
            font_no(denarii, 0x20, " Dn",
                    0x230, 6, font1, 0x10);
    }

    setup_refresh_area(0xeb, 0, 0x1a, 2, 1);
}

// Update the bottom strip with construction costs, icon help, or current map information.
// FUNCTION: C2 0x62177
// FUNCTION: C2WIN 0x00429f9c
void show_icon_strip(void)
{
    int any_text;
    int has_sel;
    int has_cost;
    int cost;
    int text_id;

    if (pointer_mode > 0)
        icon_strip_toggle = 0x21;

    any_text = 0;
    has_sel  = 0;
    has_cost = 0;
    cost     = 0;
    if (total_build_cost != 0) {
        has_cost = 1;
        cost     = total_build_cost;
    } else if (pm_over != 0) {
        if (placing_cost != 0) {
            icon_strip_toggle = icon_strip_toggle + 1;
            if (icon_strip_toggle > 0x40) icon_strip_toggle = 0;
            if (icon_strip_toggle == 0x20) any_text = 1;
            if (icon_strip_toggle < 0x20) {
                has_cost = 1;
                cost     = placing_cost;
            } else if (selected_icon_no != 0) {
                int sel = selected_icon_no;
                text_id  = sel - 1;
                has_sel  = 1;
            } else {
                text_id = last_icon_used;
            }
        } else {
            text_id = last_icon_used;
        }
    } else if (redraw_topline != 0) {
        text_id = last_icon_used;
    } else if (last_icon_over == 0) {
        text_id = last_icon_used;
    } else {
        if (last_icon_over == 0) {
        } else {
            text_id = last_icon_over;
        }
    }

    if (!(any_text == 1)) {
        if (has_cost != 0
            && cost == request_message.cached_cost)
            return;
        if (text_id == request_message.cached_text_id)
            return;
    }

    sprite_width  = 9;
    sprite_height = 0xf;
    show_fast_rect(0x1e8, 0x10b, 0x1a);

    if (has_cost != 0) {
        request_message.cached_text_id = -1;
        x_is = 0;
        font_list(0x34, 0, 0x1ea, 0x10c, font1, 0x10);
        font_no(cost, 0x20, " Dn",
                x_is + 0x1ea, 0x10d, font1, 0x10);
        request_message.cached_cost = cost;
    } else {
        request_message.cached_cost = -1;
        if (map_mode == 0) {
            if (has_sel != 0)
                font_list(selected_icon_text, text_id, 0x1ea,
                          0x10c, font1, 0x10);
            else
                font_list(0x32, text_id, 0x1ea, 0x10c, font1, 0x10);
        } else if (has_sel != 0) {
            font_list(selected_icon_text, text_id, 0x1ea,
                      0x10c, font1, 0x10);
        } else {
            font_list(0x33, text_id, 0x1ea, 0x10c, font1, 0x10);
        }
        request_message.cached_text_id = text_id;
    }

    setup_refresh_area(0x1e0, 0x10b, 0xa, 2, 1);
}

// Update the city overview-mode selector when its state changes.
// FUNCTION: C2 0x62366
// FUNCTION: C2WIN 0x0042a299
void show_ov_bar(void)
{
    int h;
    int w;

    if (map_mode != 0) return;
    if (redraw_topline == 0 && update_ov_bar == 0) return;

    if (update_ov_bar != 0) {
        update_ov_bar--;
    }
    sprite_width = 6;
    h = 0xf;
    sprite_height = h;
    show_fast_rect(0x1e2, 0x1c, 0x1a);
    w = 1;
    sprite_width = w;
    sprite_height = h;
    show_fast_rect(0x23c, 0x1c, 0x1a);
    font_list(0x35, ov_map_mode, 0x1e4, 0x1d, font1, 0x10);
    draw_a_line(0x1de, 0x30, 0x1de, 0xd0, 0x1f);
    draw_a_line(0x1df, 0x30, 0x1df, 0xd0, 0x12);
    setup_refresh_area(0x1e0, 0x1c, 0xa, 2, w);
}

// Draw the legend for the active city analysis overlay.
// FUNCTION: C2 0x62462
// FUNCTION: C2WIN 0x0042a52e
void show_ov_legend_panel(void)
{
    cover_mouse_droppings();
    stone_random_count = 0xa;
    overlays_on = 1;
    show_citymap();

    show_a_system_blank(0x1e0, 0x30, 0xa, 0xa);
    draw_a_box(0x1e1, 0x31, 0x9e, 0x9e, 0x10);
    draw_a_box(0x1e5, 0x35, 0x96, 0x14, 0x10);

    x_is = 0;
    font_list(0x35, ov_map_mode, 0x1ec, 0x38, font1, 0x10);
    font_list(0x35, 0xb, x_is + 0x1ec, 0x38, font1, 0x10);
    font_format_split(0x35, ov_map_mode + 0xc, 0x1e5, 0x50, 0x96, 0x64,
                      0, 0, font1, 0x10);

    if (ov_map_mode == 1) {
        place_9_legend_blocks();
    } else if (ov_map_mode == 2) {
        place_3_legend_blocks(0x19, 0x84, 0x8d, 0x87);
    } else if (ov_map_mode == 3) {
        place_3x_legend_blocks(0x20, 0x93, 0x90, 0x8d);
    } else if (ov_map_mode == 4) {
        place_3_legend_blocks(0x16, 0x79, 0x78, 0x77);
    } else if (ov_map_mode == 5) {
        place_3_legend_blocks(0x16, 0x93, 0x90, 0x8d);
    } else if (ov_map_mode == 6) {
        place_9_legend_blocks();
    } else if (ov_map_mode == 7) {
        place_3_legend_blocks(0x1c, 0x84, 0x8d, 0x87);
    } else if (ov_map_mode == 8) {
        place_3_legend_blocks(0x16, 0x79, 0x78, 0x77);
    } else if (ov_map_mode == 9) {
        place_3_legend_blocks(0x16, 0x93, 0x90, 0x8d);
    }

    overlays_on = 0;
    hold_mouse_replace = 1;
    setup_whole_screen_refresh();
    refresh_svga_screen();
}

// Draw three vertically stacked legend entries with ascending captions.
// FUNCTION: C2 0x62634
// FUNCTION: C2WIN 0x0042a5f3
void place_3_legend_blocks(int caption_idx, int top_gfx_idx, int middle_gfx_idx, int bottom_gfx_idx)
{
    place_legend_block(top_gfx_idx, 0x1e8, 0x92);
    place_legend_block(middle_gfx_idx, 0x1e8, 0xa6);
    place_legend_block(bottom_gfx_idx, 0x1e8, 0xba);
    font_list(0x35, caption_idx,     0x200, 0x94, font1, 0x10);
    font_list(0x35, caption_idx + 1, 0x200, 0xa8, font1, 0x10);
    font_list(0x35, caption_idx + 2, 0x200, 0xbc, font1, 0x10);
}

// Draw three vertically stacked legend entries with descending captions.
// FUNCTION: C2 0x626c9
// FUNCTION: C2WIN 0x0042a6a1
void place_3x_legend_blocks(int caption_idx, int top_gfx_idx, int middle_gfx_idx, int bottom_gfx_idx)
{
    place_legend_block(top_gfx_idx, 0x1e8, 0x92);
    place_legend_block(middle_gfx_idx, 0x1e8, 0xa6);
    place_legend_block(bottom_gfx_idx, 0x1e8, 0xba);
    font_list(0x35, caption_idx,     0x200, 0x94, font1, 0x10);
    font_list(0x35, caption_idx - 1, 0x200, 0xa8, font1, 0x10);
    font_list(0x35, caption_idx - 2, 0x200, 0xbc, font1, 0x10);
}

// Draw the nine-color legend used by the population-density overlay.
// FUNCTION: C2 0x6274c
// FUNCTION: C2WIN 0x0042a74f
void place_9_legend_blocks(void)
{
    int i;

    for (i = 0; i < 9; i++) {
        place_legend_block(i * 3 + 0x7e, i * 16 + 0x1e8, 0x92);
    }
    font_list(0x35, 0x16, 0x1f0, 0xaa, font1, 0x10);
    font_list(0x35, 0x18, 0x250, 0xaa, font1, 0x10);
}

// Draw and frame one legend color tile.
// FUNCTION: C2 0x627b6
// FUNCTION: C2WIN 0x0042a7d8
void place_legend_block(int sprite_idx, int x, int y)
{
    int xo;
    int yo;
    sprite_start = landfill[sprite_idx * 16 + 0xC]
                 + (landfill[sprite_idx * 16 + 0xD] << 8);
    for (yo = 0; yo < 16; yo += 2) {
        for (xo = 0; xo < 16; xo += 2) {
            place_2x2_block(landfill + sprite_start,
                            (x + xo) + (y + yo) * screen_width);
        }
    }
    draw_a_box(x - 1, y - 1, 0x12, 0x12, 0x10);
}

// Print a year with the appropriate BC or AD label and display color.
// FUNCTION: C2 0x62828
// FUNCTION: C2WIN 0x0042a88d
void show_date(int year, int x, int y, int mode)
{
    x_is = 0;
    if (mode == 0) {
        if (year < 0) {
            font_no(-year, 0x20, " ", x, y, font1, 0x10);
            font_list(0x1a, 0, x + x_is, y, font1, 0x10);
        } else {
            font_no(year, 0x20, " ", x, y, font1, 0x10);
            font_list(0x1a, 1, x + x_is, y, font1, 0x10);
        }
    } else if (mode == 1) {
        if (year < 0) {
            font_no(-year, 0x20, " ", x, y, font1, 0x10);
            font_list(0x1a, 0, x + x_is, y, font1, 0x10);
        } else {
            font_no(year, 0x20, " ", x, y, font1, 0x10);
            font_list(0x1a, 1, x + x_is, y, font1, 0x10);
        }
    } else if (mode == 2) {
        if (year < 0) {
            font_no(-year, 0x20, " ", x, y, font1, 0x3f);
            font_list(0x1a, 0, x + x_is, y, font1, 0x3f);
        } else {
            font_no(year, 0x20, " ", x, y, font1, 0x3f);
            font_list(0x1a, 1, x + x_is, y, font1, 0x3f);
        }
    }
}

// Show the census mini-panel with the city's population and employment rate.
// FUNCTION: C2 0x62911
// FUNCTION: C2WIN 0x0042aa89
void show_census_panel(void)
{
    cover_mouse_droppings();
    stone_random_count = 0x1c;

    show_a_mosaic_window(0x50, 0x60, 0x14, 0xb);
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
    show_an_exit_button(0x160, 0xe0);

    font_list(0x4b, 0,    0x70, 0x78, font2, 0x10);
    font_list(0x4b, 1,    0x70, 0x94, font1, 0x10);

    x_is = 0;
    font_list(0x4b, 2,    0x70, 0xb8, font1, 0x10);
    font_no(population,      0x20, " ",
            x_is + 0x70, 0xb8, font1, 0x10);

    x_is = 0;
    font_list(0x4b, 3,    0x70, 0xd0, font1, 0x10);
    font_no(employment_rate, 0x20, "%",
            x_is + 0x70, 0xd0, font1, 0x10);

    font_list(9,    1,    0x90, 0xf0, font1, 0x10);

    refresh_svga_screen();
    hold_mouse_replace = 1;
}

// Update the population display used during turbo mode.
// FUNCTION: C2 0x62a51
// FUNCTION: C2WIN 0x0042abb8
void show_turbo_panel(void)
{
    if (request_message.cached_population == population) {
        if (turbo_mode > 2) return;
    }

    stone_random_count = 0x1c;
    show_a_mosaic_window(0x50, 0x60, 0x14, 0xb);
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
    show_an_exit_button(0x160, 0xe0);
    font_list(0x4c, 0, 0x70, 0x78, font2, 0x10);
    font_format_split(0x4c, 1, 0x70, 0xa0, 0xf0, 0x64, 0, 0, font1, 0x10);
    x_is = 0;
    font_no(population, 0x20, " ", 0x70, 0xe4, font1, 0x10);
    font_list(0x1e, 1, x_is + 0x70, 0xe4, font1, 0x10);
    request_message.cached_population = population;
}

// Reset the query panel and selected person.
// FUNCTION: C2 0x62b49
// FUNCTION: C2WIN 0x0042aca8
void init_queery_panel(void)
{
    queery_buttons[3].state = 1;
    query_mode = 0;
    queried_person = 0;
}

// Draw the small lower-right help panel shown while the pointer is in query mode.
// FUNCTION: C2 0x62b61
// FUNCTION: C2WIN 0x0042acce
void show_querymode_panel(void)
{
    if (pointer_mode == 4) {
        show_a_system_window(0x1df, 0x170, 0xa, 7);
        font_list(0x4a, 0, 0x1e8, 0x17c, font2, 0x10);
        font_list(0x4a, 1, 0x21a, 0x19a, font2, 0x10);
        font_format_split(0x4a, 2, 0x1e8, 0x1b8, 0x98, 0x64, 0, 0, font1, 0x10);
        setup_refresh_area(0x1de, 0x16e, 0xb, 8, 1);
    }
}

// Draw the query panel and dispatch to its active city, resident, detail, or region view.
// FUNCTION: C2 0x62c14
// FUNCTION: C2WIN 0x0042ad66
void show_query_panel(void)
{
    int heading_y;

    cover_mouse_droppings();
    stone_random_count = 0x1c;

    if (map_mode == 1) {
        query_panel_reduction = 9;
    }
    else if (query_mode == 2) {
        query_panel_reduction = 0;
    }
    else if (query_mode == 1) {
        query_panel_reduction = 7;
    }
    else if (q_type == 0xfa) {
        query_panel_reduction = 5;
    }
    else {
        query_panel_reduction = 9;
    }

    show_a_mosaic_frame(8, 0x20 + query_panel_reduction * 16,
                        0x1c, 0x1b - query_panel_reduction);
    setup_whole_screen_refresh();
    show_a_mosaic_blank(0x18, 0x30 + query_panel_reduction * 16,
                        0x1a, 0x19 - query_panel_reduction);
    show_an_exit_button(0x198, 0x1a0);
    show_buttons(8, 0x20, queery_buttons, nof_query_buttons);
    draw_a_dias(0x20, 0xa0 + query_panel_reduction * 16, 0x190,
                (0xf - query_panel_reduction) * 16);

    heading_y = query_panel_reduction * 16 + 0x48;
    if (map_mode == 0) {
        show_query_panel_heading(heading_y);
        if (query_mode == 2) {
            show_detailed_query_panel();
        }
        else if (query_mode == 1) {
            show_people_query_panel();
        }
        else {
            show_general_query_panel();
        }
    }
    else {
        show_region_query_panel(heading_y);
    }

    refresh_svga_screen();
    hold_mouse_replace = 1;
}

// Draw the queried building's category and development-level heading.
// FUNCTION: C2 0x62d6e
// FUNCTION: C2WIN 0x0042af17
void show_query_panel_heading(int y)
{
    int qt;
    int bucket;
    int y2;

    y2 = y;
    qt = q_type;
    if      (qt < 8)    bucket = 7;
    else if (qt < 0x1e) bucket = 8;
    else if (qt < 0x4e) bucket = 6;
    else if (qt < 0x78) bucket = 1;
    else if (qt < 0x7c) bucket = 0;
    else if (qt < 0x7d) bucket = 4;
    else if (qt < 0x82) bucket = 5;
    else if (qt == 0xfa) bucket = 2;
    else                bucket = qt - 0x78;

    if (q_type == 0xfa) {
        font_list(0x3e, q_goods, 0x98, y2, font2, 0x10);
    } else {
        font_list(0x3c, bucket, 0x98, y2, font2, 0x10);
    }

    this_help_page = city_mm_enties[bucket];
    y2 += 0x20;

    if (q_lv <= 0) {
        font_list(0x3d, 0, 0x98, y2, font2, 0x10);
        return;
    }
    x_is = 0;
    font_list(0x3d, 1, 0x98, y2, font2, 0x10);
    font_no(q_lv, 0x20, " ", x_is + 0x98, y2, font2, 0x10);
}

// Show general, residential, or business details for the queried city tile.
// FUNCTION: C2 0x62eb6
// FUNCTION: C2WIN 0x0042b109
void show_general_query_panel(void)
{
    int q;
    int word;

    q = q_type;
    if (q >= 0x82 && q < 0xa2) {
        show_query_house_advice();
        return;
    }

    q = q_type;
    if (q == 0xfb || q == 0xf5) {
        if (!q_road_access)        word = 0x5a;
        else if (q_hospital_access) word = 0x52;
        else                        word = 0x53;
    } else if (q == 0xfa) {
        show_query_business_advice();
        return;
    } else if (q == 0x7c) {
        word = 0x24;
    } else if (q == 0x7d) {
        word = 0x25;
    } else if (q == 0x7e) {
        word = 0x26;
    } else if (q >= 0x82) {
        if (q <= 0xa5) {
            word = (unsigned char)q;
            word -= 0x7b;
        } else if (q <= 0xa9) {
            word = (unsigned char)q;
            word -= 0x7f;
        } else if (q <= 0xad) {
            word = (unsigned char)q;
            word -= 0x83;
        } else if (q <= 0xb1) {
            if (!q_road_access)        word = 0x5a;
            else {                     word = (unsigned char)q; word -= 0x87; }
        } else if (q <= 0xb5) {
            if (!q_road_access)        word = 0x5a;
            else {                     word = (unsigned char)q; word -= 0x8b; }
        } else if (q <= 0xb9) {
            if (!q_road_access)        word = 0x5a;
            else {                     word = (unsigned char)q; word -= 0x8f; }
        } else if (q >= 0xd7) {
            if (q <= 0xda) {
                word = (unsigned char)q;
                word -= 0xb0;
            } else if (q <= 0xe2) {
                if (!q_supply) {
                    word = 0x2b;
                } else if (q <= 0xde) {
                    word = q - 0xb4;
                } else if (q <= 0xe2) {
                    word = q - 0xb8;
                }
            } else if (q < 0xe5) {
                if (!q_road_access) word = 0x5a;
                else word = 0x23;
            } else if (q >= 0xfc && q <= 0xff) {
                if (!q_road_access) word = 0x5a;
                else {              word = q - 0xd0; }
            } else {
                word = 0x23;
            }
        } else {
            word = 0x23;
        }
    } else {
        word = 0x23;
    }

    font_format_split(0x3d, word, 0x38,
                      (query_panel_reduction + 9) * 0x10 + 0x20, 0x160, 0x64,
                      0, 0, font1, 0x10);

    if (word == 0x52) {
        x_is = 0;
        if (q_type == 0xfb) {
            font_no(0x3e8, 0x20, " ", 0x38,
                    (query_panel_reduction + 0xa) * 0x10 + 0x20, font1, 0x10);
        } else {
            font_no(0x4b0, 0x20, " ", 0x38,
                    (query_panel_reduction + 0xa) * 0x10 + 0x20, font1, 0x10);
        }
        font_list(0x3d, 0x5b, x_is + 0x38,
                  (query_panel_reduction + 0xa) * 0x10 + 0x20
                      + (font1[0] - font1[0]), font1, 0x10);
    }
}

// Explain which services or conditions are limiting the queried house.
// FUNCTION: C2 0x63169
// FUNCTION: C2WIN 0x0042b5ad
void show_query_house_advice(void)
{
    int q = q_type;
    int next_lv;
    int demote_lv;
    int word;

    next_lv = (((signed char *)promotion_av_levels)[(q) * 2 + 0x186]);
    demote_lv = (((signed char *)promotion_av_levels)[(q) * 2 + 0x187]);

    if (q_lv > demote_lv)              { word = 0x57; goto render; }
    if (!q_aqua && !q_sub_aqua) {
        if (q_lv == 2)                  word = 0x3d;
        else                                word = 0x3c;
        goto render;
    }
    if (!q_admin) {
        if (q_lv == 6)                  word = 0x3e;
        else                                word = 0x3c;
        goto render;
    }
    if (q_business) {
        if (q_lv == 0xa)                word = 0x40;
        else                                word = 0x3c;
        goto render;
    }
    if (!q_market) {
        if (q_lv == 0xc)                word = 0x3f;
        else                                word = 0x3c;
        goto render;
    }
    if (!q_aqua) {
        if (q_lv == 0xe)                word = 0x41;
        else                                word = 0x3c;
        goto render;
    }
    if (q_business_low) {
        if (q_lv == 0x10)               word = 0x4f;
        else                                word = 0x3c;
        goto render;
    }
    if (!q_baths) {
        if (q_lv == 0x12)               word = 0x42;
        else                                word = 0x3c;
        goto render;
    }
    if (!q_entertainment) {
        if (q_lv == 0x14)               word = 0x43;
        else                                word = 0x3c;
        goto render;
    }
    if (q_barracks) {
        if (q_lv == 0x18)               word = 0x44;
        else                                word = 0x3c;
        goto render;
    }
    if (!q_security) {
        if (q_lv == 0x18)               word = 0x45;
        else                                word = 0x3c;
        goto render;
    }
    if (q_business_vlow) {
        if (q_lv == 0x1a)               word = 0x50;
        else                                word = 0x3c;
        goto render;
    }
    if (q_entertainment <= 1) {
        if (q_lv == 0x1a)               word = 0x43;
        else                                word = 0x3c;
        goto render;
    }
    if (q_wall) {
        if (q_lv == 0x1a)               word = 0x46;
        else                                word = 0x3c;
        goto render;
    }
    if (q_entertainment <= 2) {
        if (q_lv == 0x1c)               word = 0x43;
        else                                word = 0x3c;
        goto render;
    }
    if (q_gate) {
        if (q_lv == 0x1e)               word = 0x47;
        else                                word = 0x3c;
        goto render;
    }
    if (hospital_cover < 0x14) {
        if (q_lv == 0x1e)               word = 0x4a;
        else                                word = 0x3c;
        goto render;
    }
    if (q_entertainment <= 3) {
        if (q_lv == 0x20)               word = 0x43;
        else                                word = 0x3c;
        goto render;
    }
    if (!q_grammaticus) {
        if (q_lv == 0x22)               word = 0x48;
        else                                word = 0x3c;
        goto render;
    }
    if (q_prefecture) {
        if (q_lv == 0x22)               word = 0x49;
        else                                word = 0x3c;
        goto render;
    }
    if (hospital_cover < 0x28) {
        if (q_lv == 0x24)               word = 0x4a;
        else                                word = 0x3c;
        goto render;
    }
    if (q_entertainment <= 4) {
        if (q_lv == 0x26)               word = 0x43;
        else                                word = 0x3c;
        goto render;
    }
    if (q_near_market) {
        if (q_lv == 0x28)               word = 0x4c;
        else                                word = 0x3c;
        goto render;
    }
    if (q_security <= 1) {
        if (q_lv == 0x2a)               word = 0x4d;
        else                                word = 0x3c;
        goto render;
    }
    if (hospital_cover < 0x3c) {
        if (q_lv == 0x2c)               word = 0x4a;
        else                                word = 0x3c;
        goto render;
    }
    if (q_entertainment <= 5) {
        if (q_lv == 0x2c)               word = 0x43;
        else                                word = 0x3c;
        goto render;
    }
    if (!q_rhetor) {
        if (q_lv == 0x2e)               word = 0x4b;
        else                                word = 0x3c;
        goto render;
    }
    if (library_cover < 0x14) {
        if (q_lv == 0x2e)               word = 0x4e;
        else                                word = 0x3c;
        goto render;
    }
    if (q_entertainment <= 6) {
        if (q_lv == 0x30)               word = 0x43;
        else                                word = 0x3c;
        goto render;
    }
    if (library_cover < 0x28) {
        if (q_lv == 0x32)               word = 0x4e;
        else                                word = 0x3c;
        goto render;
    }
    if (hospital_cover < 0x50) {
        if (q_lv == 0x34)               word = 0x4a;
        else                                word = 0x3c;
        goto render;
    }
    if (library_cover < 0x3c) {
        if (q_lv == 0x36)               word = 0x4e;
        else                                word = 0x3c;
        goto render;
    }
    if (q_entertainment <= 7) {
        if (q_lv == 0x38)               word = 0x43;
        else                                word = 0x3c;
        goto render;
    }
    if (hospital_cover < 0x64) {
        if (q_lv == 0x3a)               word = 0x4a;
        else                                word = 0x3c;
        goto render;
    }
    if (library_cover < 0x50) {
        if (q_lv == 0x3a)               word = 0x4e;
        else                                word = 0x3c;
        goto render;
    }
    if (q_entertainment <= 8) {
        if (q_lv == 0x3c)               word = 0x43;
        else                                word = 0x3c;
        goto render;
    }
    if (library_cover < 0x64) {
        if (q_lv == 0x3e)               word = 0x4e;
        else                                word = 0x3c;
        goto render;
    }
    if (q_lv < 0x40) {
        word = 0x3c;
        goto render;
    }
    word = 0x51;

render:
    font_format_split(0x3d, word, 0x38,
                      (query_panel_reduction + 9) * 0x10 + 0x20,
                      0x160, 0x64, 0, 0, font1, 0x10);

    if (q_lv < next_lv) {
        font_list(0x3d, 0x56, 0x48,
                  query_panel_reduction * 0x10 + 0x88, font1, 0xd);
    }
}

// Explain the queried business's output using supplies, workforce, market, trade, and road access.
// FUNCTION: C2 0x63638
// FUNCTION: C2WIN 0x0042be42
void show_query_business_advice(void)
{
    int word;

    font_list(0x3f, q_ind_output, 0x38,
              (query_panel_reduction + 9) * 0x10 + 0x20,
              font1, 0x10);

    if (q_ind_output >= 7) {
        word = 0x30;
    } else if (q_ind_output == 6) {
        if (q_supplies <= 0x63)        word = 0x31;
        else                            word = general_business_cause();
    } else if (q_ind_output == 5) {
        if (q_supplies <= 0x4b)        word = 0x31;
        else if (q_ind_pop <= 2) word = 0x38;
        else                            word = general_business_cause();
    } else if (q_ind_output == 4) {
        if (q_supplies <= 0x43) {
            word = 0x31;
        } else if (!q_ind_market) {
            word = 0x36;
        }
        if (no_of_empire_connections <= 0) word = 0x37;
        else                               word = general_business_cause();
    } else if (q_ind_output == 3) {
        if (q_supplies <= 0x32)        word = 0x31;
        else if (q_ind_pop <= 1) word = 0x38;
        else                            word = general_business_cause();
    } else if (q_ind_output == 2) {
        if (q_supplies <= 0x22)        word = 0x31;
        else                            word = general_business_cause();
    } else if (q_ind_output == 1) {
        if (q_supplies <= 0x14)        word = 0x31;
        else if (q_local <= 0x32)      word = 0x35;
        else                            word = general_business_cause();
    } else {
        if (!q_road_access)            word = 0x5a;
        else if (q_supplies <= 0)      word = 0x31;
        else if (q_ind_pop <= 0) word = 0x38;
        else if (q_local <= 0)         word = 0x35;
        else                            word = general_business_cause();
    }

    font_format_split(0x3d, word, 0x38,
                      (query_panel_reduction + 0xa) * 0x10 + 0x20,
                      0x160, 0x64, 0, 0, font1, 0x10);

    x_is = 0;
    font_list(0x3f, 8, 0x38,
              (query_panel_reduction + 0xe) * 0x10 + 0x20,
              font1, 0x10);
    font_no(q_ind_output, 0x20, " ",
            x_is + 0x38,
            (query_panel_reduction + 0xe) * 0x10 + 0x20,
            font1, 0x10);
    font_list(0x3f, 9, x_is + 0x38,
              (query_panel_reduction + 0xe) * 0x10 + 0x20,
              font1, 0x10);

    x_is = 0;
    font_list(0x3f, 0xa, 0x38,
              (query_panel_reduction + 0xf) * 0x10 + 0x20,
              font1, 0x10);
    font_no(industry[q_goods].supply, 0x20, " ",
            x_is + 0x38,
            (query_panel_reduction + 0xf) * 0x10 + 0x20,
            font1, 0x10);
    font_list(0x3f, 0xb, x_is + 0x38,
              (query_panel_reduction + 0xf) * 0x10 + 0x20,
              font1, 0x10);
    font_list(0x10, q_goods + 1,
              x_is + 0x38,
              (query_panel_reduction + 0xf) * 0x10 + 0x20,
              font1, 0x10);
    font_list(0x3f, 0xc, x_is + 0x38,
              (query_panel_reduction + 0xf) * 0x10 + 0x20,
              font1, 0x10);
}

// Select the explanation for the queried business's most important obstacle.
// FUNCTION: C2 0x63945
// FUNCTION: C2WIN 0x0042c280
int general_business_cause(void)
{
    if (ind_growth_factor < 0)
        return 0x32;
    if (q_local < 0xc8 && population >= 0xc8)
        return 0x3a;
    if (q_ind_market == 0)
        return 0x36;
    if (no_of_empire_connections <= 0)
        return 0x34;
    if (q_local < 0x258 && population >= 0x258)
        return 0x33;
    return 0x39;
}

// List the people living in the queried house, or report that it is empty.
// FUNCTION: C2 0x639b4
// FUNCTION: C2WIN 0x0042c330
void show_people_query_panel(void)
{
    int i;
    int ratio1;
    int ratio2;
    int word_health;

    if (q_no_of_people == 0) {
        font_list(0x40, 0, 0x58,
                  (query_panel_reduction + 0xb) * 0x10 + 0x20,
                  font1, 0x10);
        return;
    }

    x_is = 0;
    font_no(q_no_of_people, 0x20, " ",
            q_no_of_people * 0x28 + 0x30,
            (query_panel_reduction + 9) * 0x10 + 0x1a,
            font1, 0x10);
    if (q_no_of_people == 1)
        font_list(0x40, 1,
                  x_is + (q_no_of_people * 0x28 + 0x30),
                  (query_panel_reduction + 9) * 0x10 + 0x1a,
                  font1, 0x10);
    else
        font_list(0x40, 2,
                  x_is + (q_no_of_people * 0x28 + 0x30),
                  (query_panel_reduction + 9) * 0x10 + 0x1a,
                  font1, 0x10);
    font_list(0x40, 3,
              q_no_of_people * 0x28 + 0x30,
              (query_panel_reduction + 0xa) * 0x10 + 0x1a,
              font1, 0x10);

    for (i = 0; i < q_no_of_people; i++) {
        draw_a_dias(i * 0x28 + 0x30,
                    (query_panel_reduction + 9) * 0x10 + 0x18,
                    0x18, 0x20);
        draw_a_rect(i * 0x28 + 0x31,
                    (query_panel_reduction + 9) * 0x10 + 0x19,
                    0x16, 0x1e, 0x14);
        citizen_a = (short)(unsigned char)q_people_list[i];
        write_image(people_data,
                    citizen_list[citizen_a].image_id,
                    i * 0x28 + 0x37,
                    (query_panel_reduction + 9) * 0x10 + 0x20);
    }

    citizen_a = (short)(unsigned char)q_people_list[queried_person];

    draw_a_box(queried_person * 0x28 + 0x2e,
               (query_panel_reduction + 9) * 0x10 + 0x16,
               0x1c, 0x24, 0xa);

    x_is = 0;
    if (citizen_list[citizen_a].type == 3)
        font_list(0x42, citizen_list[citizen_a].name_id, 0x58,
                  (query_panel_reduction + 0xc) * 0x10 + 0x1c,
                  font1, 0x10);
    else
        font_list(0x41, citizen_list[citizen_a].name_id, 0x58,
                  (query_panel_reduction + 0xc) * 0x10 + 0x1c,
                  font1, 0x10);
    font_list(0x43, citizen_list[citizen_a].type - 1, x_is + 0x58,
              (query_panel_reduction + 0xc) * 0x10 + 0x1c,
              font1, 0x10);

    word_health = 0xf;
    if (citizen_list[citizen_a].type == 1) {
        test_range_for(citizen_list[citizen_a].x,
                       citizen_list[citizen_a].y, 5, 0);
        ratio1 = valueDIVtotal(test_result2, test_result1);
        if      (ratio1 > 0x32) word_health = 4;
        else if (ratio1 > 0xa)  word_health = 5;
        else if (ratio1 != 0)   word_health = 6;
        else {
            ratio2 = valueDIVtotal(test_result3, test_result1 * 3);
            if      (ratio2 < 0x3c) word_health = 7;
            else if (ratio2 < 0x5a) word_health = 8;
            else                     word_health = 9;
        }
    }
    if (citizen_list[citizen_a].type == 2) {
        if      (citizen_list[citizen_a].market_demand_a < 1) word_health = 0xa;
        else if (citizen_list[citizen_a].market_demand_a < 8) word_health = 0xb;
        else if (citizen_list[citizen_a].market_demand_b < 1) word_health = 0xc;
        else                                                   word_health = 0xd;
    }
    if (citizen_list[citizen_a].type == 3)
        word_health = 0xe;
    if (citizen_list[citizen_a].type == 4) {
        if (citizen_list[citizen_a].state_idx == 6) {
            word_health = 0xf;
        } else {
            test_range_for(citizen_list[citizen_a].x,
                           citizen_list[citizen_a].y, 5, 1);
            ratio1 = valueDIVtotal(test_result2, test_result1);
            if      (ratio1 > 0x32) word_health = 0x10;
            else if (ratio1 > 0xa)  word_health = 0x11;
            else                     word_health = 0x12;
        }
    }
    if (citizen_list[citizen_a].type == 5) {
        if (citizen_list[citizen_a].state_idx == 6) {
            word_health = 0xf;
        } else if (citizen_list[citizen_a].state_idx == 9) {
            word_health = 0x13;
        } else {
            test_range_for(citizen_list[citizen_a].x,
                           citizen_list[citizen_a].y, 5, 2);
            ratio1 = valueDIVtotal(test_result2, test_result1 << 4);
            if      (ratio1 > 0x50) word_health = 0x14;
            else if (ratio1 > 0x3c) word_health = 0x15;
            else if (ratio1 > 0x28) word_health = 0x16;
            else if (ratio1 > 0x14) word_health = 0x17;
            else                     word_health = 0x18;
        }
    }
    if (citizen_list[citizen_a].type == 6) {
        if      (citizen_list[citizen_a].market_demand_a < 1) word_health = 0x19;
        else if (citizen_list[citizen_a].market_demand_a < 8) word_health = 0x1a;
        else if (citizen_list[citizen_a].market_demand_b < 1) word_health = 0x1b;
        else                                                   word_health = 0x1c;
    }
    if (citizen_list[citizen_a].type == 7) {
        if (pop_tax_rate > 0xa) word_health = 0x1d;
        else                    word_health = 0x1e;
    }
    font_format_split(0x40, word_health, 0x58,
                      (query_panel_reduction + 0xd) * 0x10 + 0x1c,
                      0x150, 0x64, 0, 0, font1, 0x10);
}

// Select the person clicked in the query-panel resident list.
// FUNCTION: C2 0x63ef3
// FUNCTION: C2WIN 0x0042cba3
void get_queried_person(void)
{
    int i;

    if (query_mode != 1) return;
    if (mouse_left_preclick == 0) return;
    for (i = 0; i < q_no_of_people; i++) {
        if (mouse_in_area(i * 40 + 0x30,
                          (query_panel_reduction + 9) * 16 + 0x18,
                          0x18, 0x20)) {
            queried_person = i;
            show_query_panel();
        }
    }
}

// Show detailed requirement, access, and service statuses for the queried building or citizen.
// FUNCTION: C2 0x63f55
// FUNCTION: C2WIN 0x0042cc37
void show_detailed_query_panel(void)
{
    int row;
    int text_idx;
    int colour;

    for (row = 0; row < 0xb; row++) {
        colour = 0x10;
        if (row == 0) { if (q_aqua) text_idx = 2; else if (q_sub_aqua) text_idx = 3; else { text_idx = 4; colour = 0xb; } }
        if (row == 1) { if (q_admin) text_idx = 5; else { text_idx = 6; colour = 0xb; } }
        if (row == 2) {
            if (q_security > 1) text_idx = 0x5c;
            else if (q_patrol) text_idx = 7;
            else if (q_security > 0) text_idx = 8;
            else { text_idx = 9; colour = 0xb; }
        }
        if (row == 3) { if (q_market) text_idx = 0xa; else { text_idx = 0xb; colour = 0xb; } }
        if (row == 4) { if (q_grammaticus) text_idx = 0xc; else { text_idx = 0xd; colour = 0xb; } }
        if (row == 5) { if (q_rhetor) text_idx = 0xe; else { text_idx = 0xf; colour = 0xb; } }
        if (row == 6) text_idx = 0x10;
        if (row == 7) { if (q_baths) text_idx = 0x11; else { text_idx = 0x12; colour = 0xb; } }
        if (row == 8) {
            if (hospital_cover >= 0x64) text_idx = 0x13;
            else { if (hospital_cover <= 0) text_idx = 0x54;
                else text_idx = 0x14; colour = 0xb; }
        }
        if (row == 9) {
            if (library_cover >= 0x64) text_idx = 0x15;
            else { if (library_cover <= 0) text_idx = 0x55;
                else text_idx = 0x16; colour = 0xb; }
        }
        if (row == 0xa) {
            if (q_road_access) text_idx = 0x58; else { text_idx = 0x59; colour = 0xb; }
        }

        if (not_pertinant_statistic1(row) != 0) colour = 0x29;

        x_is = 0;
        font_list(0x3d, text_idx, 0x38, row * 0x10 + 0xac, font1, colour);
        if (row == 6) font_no(q_entertainment, 0x20, " ", x_is + 0x38, row * 0x10 + 0xac, font1, colour);
        if (row == 8 && text_idx == 0x14) font_no(hospital_cover, 0x20, "%", x_is + 0x38, row * 0x10 + 0xac, font1, colour);
        if (row == 9 && text_idx == 0x16) font_no(library_cover, 0x20, "%", x_is + 0x38, row * 0x10 + 0xac, font1, colour);
    }

    for (row = 0; row < 6; row++) {
        colour = 0x10;
        if (row == 0) {
            if (q_business) { text_idx = 0x17; colour = 0xb; }
            else if (q_business_low) { text_idx = 0x17; colour = 0xb; }
            else if (q_business_vlow) { text_idx = 0x17; colour = 0xb; }
            else text_idx = 0x18;
        }
        if (row == 1) { if (!q_barracks) text_idx = 0x1a; else { text_idx = 0x19; colour = 0xb; } }
        if (row == 2) { if (!q_wall) text_idx = 0x1c; else { text_idx = 0x1b; colour = 0xb; } }
        if (row == 3) { if (!q_prefecture) text_idx = 0x1e; else { text_idx = 0x1d; colour = 0xb; } }
        if (row == 4) { if (!q_near_market) text_idx = 0x20; else { text_idx = 0x1f; colour = 0xb; } }
        if (row == 5) { if (!q_gate) text_idx = 0x22; else { text_idx = 0x21; colour = 0xb; } }
        font_list(0x3d, text_idx, 0xf8, row * 0x10 + 0xac, font1, colour);
    }

    draw_a_dias(0x28, 0x160, 0x180, 0x28);
    font_list(0x26, 0x25, 0x38, 0x164, font1, 0x10);
    font_list(0x26, pop_growth_factor + 0x20, 0xd0, 0x164, font1, 0x10);
    font_list(0x26, 0x26, 0x38, 0x176, font1, 0x10);
    font_list(0x26, ind_growth_factor + 0x20, 0xd0, 0x176, font1, 0x10);
}

// Explain the queried regional building, unit, terrain, or industry.
// FUNCTION: C2 0x64371
// FUNCTION: C2WIN 0x0042d278
void show_region_query_panel(int y)
{
    int paragraph;
    int quote_kind;
    int extra_kind = 0;
    int quote;

    if (q_type < 0x10) { paragraph = 0; quote_kind = 0; }
    else if (q_type < 0x18) { paragraph = 1; quote_kind = 0; }
    else if (q_type < 0x1c) { paragraph = 2; quote_kind = 0; }
    else if (q_type < 0x20) { paragraph = 3; quote_kind = 0; }
    else if (q_type < 0x7d) { paragraph = 4; quote_kind = 0; }
    else if (q_type < 0x85) { paragraph = 5; quote_kind = 0; }
    else if (q_type < 0x8d) { paragraph = 6; quote_kind = 0; }
    else if (q_type < 0x91) { paragraph = 7; quote_kind = 0; }
    else if (q_type < 0x92) { paragraph = 8; quote_kind = 0; }
    else if (q_type < 0x93) { paragraph = 9; quote_kind = 1; extra_kind = 1; }
    else if (q_type < 0x94) { paragraph = 0xa; quote_kind = 2; }
    else if (q_type < 0x95) { paragraph = 0xb; quote_kind = 3; }
    else if (q_type < 0x96) { paragraph = 0xc; quote_kind = 4; }
    else if (q_type < 0x97) { paragraph = 0xd; quote_kind = 5; }
    else if (q_type < 0x98) { paragraph = 0xe; quote_kind = 6; }
    else if (q_type < 0x9c) { paragraph = 0xf; quote_kind = 7; }
    else if (q_type < 0xa0) { paragraph = 0x10; quote_kind = 0; }
    else if (q_type < 0xb5) { paragraph = 0x11; quote_kind = 0; }
    else if (q_type < 0xb6) { paragraph = 0x12; quote_kind = 0; }
    else if (q_type < 0xd2) { paragraph = 0x13; quote_kind = 0; }
    else if (q_type < 0xd3) { paragraph = 0x14; quote_kind = 0; extra_kind = 2; }
    else if (q_type < 0xd4) { paragraph = 0x15; quote_kind = 8; }
    else if (q_type < 0xd5) { paragraph = 0x16; quote_kind = 9; extra_kind = 4; }
    else if (q_type < 0xdc) { paragraph = 0x17; quote_kind = 0; }
    else if (q_type < 0xe0) { paragraph = 0x18; quote_kind = 0xa; extra_kind = 3; }
    else if (q_type < 0xe4) { paragraph = 0x19; quote_kind = 0xb; extra_kind = 3; }
    else if (q_type < 0xe8) { paragraph = 0x1a; quote_kind = 0xc; extra_kind = 3; }
    else if (q_type < 0xec) { paragraph = 0x1b; quote_kind = 0xd; }
    else { paragraph = 0x1c; quote_kind = 0xe; }

    x_is = 0;
    if (extra_kind != 4) {
        font_list(0x44, paragraph, 0x98, y, font2, 0x10);
    }
    if (extra_kind == 1) {
        font_list(0x32, 0xa, 0x98, y + 0x34, font1, 0x10);
    }
    if (extra_kind == 2) {
        int army_name = get_army_name_from_fort_ref(q_ptr);
        font_list(5, army_name, x_is + 0x98, y, font2, 0x10);
    }
    if (extra_kind == 3) {
        font_list(0x10, q_goods + 1, x_is + 0x98, y, font2, 0x10);
    }
    if (extra_kind == 4) {
        if (q_had_goods != 0) {
            font_list(0x10, q_goods + 1, 0x68, y, font2, 0x10);
            font_list(0x44, paragraph, x_is + 0x68, y, font2, 0x10);
        } else {
            font_list(0x44, paragraph, x_is + 0x98, y, font2, 0x10);
        }
    }

    this_help_page = ((short *)region_mm_enties)[paragraph];

    if (quote_kind == 0) {
        quote = 0;
    } else if (quote_kind == 1) {
        quote = q_gfx / 4 + 1;
    } else if (quote_kind == 2) {
        quote = 2;
    } else if (quote_kind == 3) {
        quote = 4;
    } else if (quote_kind == 4) {
        quote = 6;
    } else if (quote_kind == 5) {
        quote = 8;
    } else if (quote_kind == 6) {
        if (!q_road) quote = 0x1c;
        else quote = q_gfx - 0x31;
    } else if (quote_kind == 7) {
        if (!q_road) quote = 0x1c;
        else quote = q_gfx - 0x4f;
    } else if (quote_kind == 8) {
        quote = q_gfx - 0x33;
    } else if (quote_kind == 9) {
        if (q_wh_level <= 0) quote = 0x1e;
        else if (q_wh_level < 4) quote = 0xd;
        else if (q_wh_level < 8) quote = 0xe;
        else if (q_wh_level < 0xf) quote = 0xf;
        else quote = 0x10;
    } else if (quote_kind == 0xa) {
        quote = reg_industry_quote(q_type - 0xdc);
    } else if (quote_kind == 0xb) {
        quote = reg_industry_quote(q_type - 0xe0);
    } else if (quote_kind == 0xc) {
        quote = reg_industry_quote(q_type - 0xe4);
    } else if (quote_kind == 0xd) {
        quote = reg_tpost_quote(q_type - 0xe8);
    } else if (quote_kind == 0xe) {
        quote = reg_port_quote(q_type - 0xec);
    } else {
        quote = 0;
    }

    font_format_split(0x45, quote, 0x38,
                      (query_panel_reduction + 9) * 0x10 + 0x20,
                      0x150, 0x64, 0, 0, font1, 0x10);
}

// Select the regional-industry advice matching its access, labor, and supply state.
// FUNCTION: C2 0x64880
// FUNCTION: C2WIN 0x0042dabd
int reg_industry_quote(int x)
{
    if (q_road == 0)        return 0x19;
    if (q_workhouse == 0)   return 0x1a;
    if (q_outside)          return 0x1d;
    if (q_workhouse <= 1)   return 0x1b;
    return x + 0x11;
}

// Select trading-post advice based on road access.
// FUNCTION: C2 0x648c0
// FUNCTION: C2WIN 0x0042db4b
int reg_tpost_quote(int base)
{
    if (!q_road) return 0x19;
    return base + 0x15;
}

// Select port advice based on road access.
// FUNCTION: C2 0x648d3
// FUNCTION: C2WIN 0x0042db85
int reg_port_quote(int base)
{
    if (!q_road) return 0x19;
    return base + 0x15;
}

// Collect building, service, business, and resident data for the queried city tile.
// FUNCTION: C2 0x648e6
// FUNCTION: C2WIN 0x0042dbbf
void get_query_info(void)
{
    unsigned char b;
    int xc, yc;
    int stride;
    int ptr;
    int pop;
    int dx;
    int dy;
    unsigned char a;
    int ax, ay;
    unsigned char footprint;

    q_type = ((unsigned char *)city_map)[pm_over_cm_ptr];
    if (q_type < 0x82)
        footprint = 1;
    else
        footprint = reg_aquaduct_gfxdat[q_type + 8];

    dx = dy = 0;
    if (footprint > 1) {
        dx = dy = ((unsigned char *)city_map)[pm_over_cm_ptr + 5] & 0xf;
        dx = dx % footprint;
        dy = dy / footprint;
        ptr = pm_over_cm_ptr - dx * 20;
        ptr -= dy * 20 * 80;
    } else {
        ptr = pm_over_cm_ptr;
    }

    q_type = ((unsigned char *)city_map)[ptr];
    q_flag = ((unsigned char *)city_map)[ptr + 1];
    q_lv = (char)get_best_lv((unsigned char *)city_map + ptr, footprint);
    q_cover1 = ((unsigned char *)city_map)[ptr + 0xd];
    q_cover2 = ((unsigned char *)city_map)[ptr + 0xe];
    q_range1 = ((unsigned char *)city_map)[ptr + 0xa];
    q_range3 = ((unsigned char *)city_map)[ptr + 0xc];
    q_supply       = (unsigned char)affected_by_cover1((unsigned char *)city_map + ptr, footprint, 4);
    q_sub_aqua     = (unsigned char)affected_by_cover1((unsigned char *)city_map + ptr, footprint, 2);
    q_aqua         = (char)affected_by_cover1((unsigned char *)city_map + ptr, footprint, 1);
    q_baths        = (unsigned char)affected_by_cover1((unsigned char *)city_map + ptr, footprint, 8);
    q_grammaticus  = (unsigned char)affected_by_cover1((unsigned char *)city_map + ptr, footprint, 0x10);
    q_rhetor       = (unsigned char)affected_by_cover1((unsigned char *)city_map + ptr, footprint, 0x20);
    q_admin        = (char)get_range1((unsigned char *)city_map + ptr, footprint, 0xc);
    q_shell        = ((unsigned char *)city_map)[ptr + 0x11];
    q_patrol       = (unsigned char)get_range1((unsigned char *)city_map + ptr, footprint, 0x30);
    if ((signed char)q_shell >= 0x10)
        q_security = 1;
    else
        q_security = 0;
    if (q_patrol)
        q_security = q_security + 1;
    q_theatre      = get_range3((unsigned char *)city_map + ptr, footprint, 3);
    q_colosseum    = get_range3((unsigned char *)city_map + ptr, footprint, 0xc);
    q_colosseum    = q_colosseum >> 2;
    q_circus       = get_range3((unsigned char *)city_map + ptr, footprint, 0x30);
    q_circus       = q_circus >> 4;
    q_entertainment = q_theatre + q_colosseum + q_circus;
    q_market       = (unsigned char)get_range1((unsigned char *)city_map + ptr, footprint, 0xc0);
    q_business     = (char)affected_by_cover1((unsigned char *)city_map + ptr, footprint, 0x80);
    q_business_low = (char)affected_by_cover2((unsigned char *)city_map + ptr, footprint, 0x10);
    q_business_vlow = (char)affected_by_cover2((unsigned char *)city_map + ptr, footprint, 0x20);
    q_barracks     = (char)affected_by_cover2((unsigned char *)city_map + ptr, footprint, 1);
    q_wall         = (char)affected_by_cover2((unsigned char *)city_map + ptr, footprint, 8);
    q_gate         = (unsigned char)affected_by_cover2((unsigned char *)city_map + ptr, footprint, 4);
    q_prefecture   = (unsigned char)affected_by_cover2((unsigned char *)city_map + ptr, footprint, 2);
    q_near_market  = (unsigned char)affected_by_cover1((unsigned char *)city_map + ptr, footprint, 0x40);

    q_goods = ((unsigned char *)city_map)[ptr + 0x13] & 0xf;
    q_ind_output = ((unsigned char *)city_map)[ptr + 0x9] & 0xf0;
    q_ind_output >>= 4;
    q_ind_pop    = ((unsigned char *)city_map)[ptr + 0x9] & 3;
    pop = test_area_for_population(2,
                                   act_start_x - dx,
                                   act_start_y - dy, 2);
    if      (pop > 0x82) q_ind_pop = q_ind_pop + 4;
    else if (pop > 0x5a) q_ind_pop = q_ind_pop + 3;
    else if (pop > 0x32) q_ind_pop = q_ind_pop + 2;
    else if (pop > 0xa)  q_ind_pop = q_ind_pop + 1;
    q_ind_market = ((unsigned char *)city_map)[ptr + 0x9] & 0xc;

    q_supplies = industry[q_goods].city_supply;
    q_local    = industry[q_goods].supply_pipeline[0];
    q_hospital_access = 0;
    cm_sptr = ptr;
    if (q_type == 0xfb || q_type == 0xf5) {
        if (test_perimeter_for_road_and_forum(
                act_start_x - dx, act_start_y - dy, 3, 0) != 0)
            q_hospital_access = 1;
    }
    q_road_access = (unsigned char)test_perimeter_for_road_and_forum(
            act_start_x - dx, act_start_y - dy, footprint, 1);
    if (q_type >= 0x82 && q_type <= 0xa1 && q_road_access == 0)
        q_road_access = (unsigned char)test_range_for_road(
            act_start_x - dx, act_start_y - dy, 3);

    queried_person = 0;
    q_no_of_people = 0;
    a = ((unsigned char *)city_map)[ptr + 7];
    b = ((unsigned char *)city_map)[ptr + 8];
    if (a) {
        q_people_list[q_no_of_people] = a;
        q_no_of_people++;
    }
    if (b) {
        q_people_list[q_no_of_people] = b;
        q_no_of_people++;
    }

    ax = act_start_x - 1;
    ay = act_start_y - 1;
    xc = yc = 3;
    if (ax < 0) {
        xc = 2;
        ax = 0;
    } else if (xc + ax > 0x50) {
        xc = 2;
    }
    if (ay < 0) {
        yc = 2;
        ay = 0;
    } else if (yc + ay > 0x3c) {
        yc = 2;
    }
    ptr = (ay * 80 + ax) * 20;
    stride = (0x50 - xc) * 20;
    for (gmn_y = ay; gmn_y < ay + yc; gmn_y++, ptr += stride) {
        for (gmn_x = ax; gmn_x < ax + xc; gmn_x++, ptr += 20) {
            if (q_no_of_people >= 6) break;
            if (gmn_x == act_start_x && gmn_y == act_start_y) continue;
            a = ((unsigned char *)city_map)[ptr + 7];
            b = ((unsigned char *)city_map)[ptr + 8];
            if (a) {
                q_people_list[q_no_of_people] = a;
                q_no_of_people++;
            }
            if (b) {
                q_people_list[q_no_of_people] = b;
                q_no_of_people++;
            }
        }
    }
}

// Collect building, access, labor, and goods data for the queried region tile.
// FUNCTION: C2 0x64e92
// FUNCTION: C2WIN 0x0042e3ed
void get_region_query_info(void)
{
    int slave_per_camp;
    int ptr;
    int map_pos;
    int map_x;
    int map_y;
    int workhouse;

    slave_per_camp = slave_requirements[5].current;
    if (no_of_workcamps != 0)
        slave_per_camp = slave_per_camp / no_of_workcamps;
    else
        slave_per_camp = 0;
    slave_per_camp = slave_per_camp / 10;
    if (slave_per_camp > 3) slave_per_camp = 3;
    if (slave_per_camp < 0) slave_per_camp = 0;

    ptr = pm_over_cm_ptr;
    q_type = (*(struct region_cell *)((unsigned char *)region_map + (ptr))).base_kind;

    if (q_type >= 0xd5) {
        int corner = (*(struct region_cell *)((unsigned char *)region_map + (ptr))).occupant & 3;
        int dx     = corner % 2;
        int dy     = corner / 2;
        ptr -= dx * 8;
        ptr -= dy * 0x1e0;
    }
    q_ptr = ptr;

    map_pos = ptr / map_actual_atom;
    map_y = map_pos % 0x3c;
    map_x = map_pos / 0x3c;

    q_occa     = (*(struct region_cell *)((unsigned char *)region_map + (ptr))).occupant;
    q_gfx      = (*(struct region_cell *)((unsigned char *)region_map + (ptr))).gfx;
    q_wh_level = (*(struct region_cell *)((unsigned char *)region_map + (ptr))).occupant & 0xf;
    q_pop_level = get_pop_level();
    q_road     = (*(struct region_cell *)((unsigned char *)region_map + (ptr))).edge_bits & 0x20;
    q_outside  = (*(struct region_cell *)((unsigned char *)region_map + (ptr))).outside & 0x40;

    workhouse = get_reg_buildings_in_radius(map_y, map_x,
                                            2, 1, 0xd3);
    q_workhouse = workhouse;
    if (workhouse != 0)
        q_workhouse = workhouse * slave_per_camp + 1;

    q_goods = (*(struct region_cell *)((unsigned char *)region_map + (ptr))).occupant & 0xf0;
    q_goods >>= 4;
    q_had_goods = (*(struct region_cell *)((unsigned char *)region_map + (ptr))).edge_bits & 0x40;
}

// Report whether a detailed statistic applies to the current query type.
// FUNCTION: C2 0x65001
// FUNCTION: C2WIN 0x0042e5dc
int not_pertinant_statistic1(int statistic_idx)
{
    if (q_type >= 0x82 && q_type < 0xa2) {
        if (statistic_idx != 0xa) goto not_pert;
        return 1;
    }
    if (q_type >= 0xae && q_type <= 0xb9) {
        if (statistic_idx == 0xa) goto not_pert;
        return 1;
    }
    if (q_type >= 0xdb && q_type <= 0xe2) {
        if (statistic_idx == 0)   goto not_pert;
        return 1;
    }
    if (q_type >= 0xe3 && q_type <= 0xe4) {
        if (statistic_idx == 0xa) goto not_pert;
        return 1;
    }
    if (q_type == 0xfb || q_type == 0xf5) {
        if (statistic_idx == 0xa) return 0;
        if (statistic_idx == 1) return 0;
        return 1;
    }
    if (q_type == 0xfa) {
        if (statistic_idx == 0xa) goto not_pert;
        return 1;
    }
    if (q_type >= 0xfc) {
        if (q_type <= 0xff) {
            if (statistic_idx == 0xa) goto not_pert;
            return 1;
        }
        return 1;
    }
    return 1;
not_pert:
    return 0;
}

// Report whether the secondary statistic applies to the current query type.
// FUNCTION: C2 0x650c3
// FUNCTION: C2WIN 0x0042e7a9
int not_pertinant_statistic2(void)
{
    if (q_type >= 0x7c && q_type <= 0xb9) return 0;
    if (q_type >= 0xd7 && q_type <= 0xe2) return 0;
    return 1;
}

// Show the pre-battle briefing, armies, terrain, and tactical choices.
// FUNCTION: C2 0x650f4
// FUNCTION: C2WIN 0x0042e817
void show_battle_intro_screen(void)
{
    int tribe_idx;

    cover_mouse_droppings();
    setup_whole_screen_refresh();
    stone_random_count = 0x32;
    show_a_mosaic_window(8, 0xb0, 0x27, 0x13);

    x_is = 0;
    font_list(0x47, 0, 0xf0, 0xdc, font2, 0x10);
    font_list(0x47, 1, 0x30, 0x104, font1, 0x10);
    font_list(0x47, 3, 0x30, 0x116, font1, 0x10);
    font_list(0x47, 4, 0x30, 0x13e, font1, 0x10);
    show_buttons(0x100, 0x104, confirming_buttons, 2);
    font_list(0x47, 5, 0x30, 0x170, font1, 0x10);

    x_is = 0;
    font_no(army_list[our_battle_army].num_regulars, 0x20, " ", 0x50, 0x182, font1, 0x10);
    font_list(0x47, 6, x_is + 0x50, 0x182, font1, 0x10);
    x_is = 0;
    font_no(army_list[our_battle_army].num_irregulars, 0x20, " ", 0x50, 0x194, font1, 0x10);
    font_list(0x47, 7, x_is + 0x50, 0x194, font1, 0x10);
    x_is = 0;
    font_no(army_list[our_battle_army].num_auxillaries, 0x20, " ", 0x50, 0x1a6, font1, 0x10);
    font_list(0x47, 8, x_is + 0x50, 0x1a6, font1, 0x10);
    x_is = 0;
    font_no(army_list[our_battle_army].num_specials, 0x20, " ", 0x50, 0x1b8, font1, 0x10);
    font_list(0x47, 9, x_is + 0x50, 0x1b8, font1, 0x10);

    x_is = 0;
    font_list(7, army_list[their_battle_army].source_region, 0x100, 0x170, font1, 0x10);
    font_list(0x47, 0xa, x_is + 0x100, 0x170, font1, 0x10);

    if (army_list[their_battle_army].num_specials != 0) {
        x_is = 0;
        font_no(army_list[their_battle_army].num_specials, 0x20, " ", 0x120, 0x182, font1, 0x10);
        font_list(0x47, 0xb, x_is + 0x120, 0x182, font1, 0x10);
        font_list(0x47, 0x19, x_is + 0x120, 0x182, font1, 0x10);
    } else {
        x_is = 0;
        tribe_idx = tribe_battle_setup[army_list[their_battle_army].tribe_id].u.raw[0];
        font_no(army_list[their_battle_army].num_horse, 0x20, " ", 0x120, 0x182, font1, 0x10);
        font_list(0x47, 0xb, x_is + 0x120, 0x182, font1, 0x10);
        if (army_list[their_battle_army].num_horse != 0) font_list(0x47, tribe_idx + 0xa, x_is + 0x120, 0x182, font1, 0x10);
    }

    x_is = 0;
    tribe_idx = tribe_battle_setup[army_list[their_battle_army].tribe_id].u.raw[1];
    font_no(army_list[their_battle_army].num_regulars, 0x20, " ", 0x120, 0x194, font1, 0x10);
    font_list(0x47, 0xc, x_is + 0x120, 0x194, font1, 0x10);
    font_list(0x47, tribe_idx + 0xa, x_is + 0x120, 0x194, font1, 0x10);

    x_is = 0;
    tribe_idx = tribe_battle_setup[army_list[their_battle_army].tribe_id].u.raw[2];
    font_no(army_list[their_battle_army].num_irregulars, 0x20, " ", 0x120, 0x1a6, font1, 0x10);
    font_list(0x47, 0xd, x_is + 0x120, 0x1a6, font1, 0x10);
    font_list(0x47, tribe_idx + 0xa, x_is + 0x120, 0x1a6, font1, 0x10);

    x_is = 0;
    tribe_idx = tribe_battle_setup[army_list[their_battle_army].tribe_id].u.raw[3];
    font_no(army_list[their_battle_army].num_auxillaries, 0x20, " ", 0x120, 0x1b8, font1, 0x10);
    font_list(0x47, 0xe, x_is + 0x120, 0x1b8, font1, 0x10);
    font_list(0x47, tribe_idx + 0xa, x_is + 0x120, 0x1b8, font1, 0x10);

    hold_mouse_replace = 1;
    refresh_svga_screen();
    set_palette(city_palette);
}

// Show the battle result, losses, morale changes, and follow-up message.
// FUNCTION: C2 0x656cd
// FUNCTION: C2WIN 0x0042eeac
void show_battle_outtro_screen(void)
{
    cover_mouse_droppings();
    setup_whole_screen_refresh();
    stone_random_count = 0x32;
    show_a_mosaic_window(8, 0xb0, 0x27, 0x13);

    x_is = 0;
    if (battle_victor == 0) {
        font_list(0x48, 0, 0xc0, 0xdc, font2, 0x10);
        font_list(0x48, 2, 0x80, 0x104, font1, 0x10);
        play_speech(5);
    } else {
        font_list(0x48, 1, 0xc0, 0xdc, font2, 0x10);
        if (battle_state == 6) {
            font_list(0x48, 3, 0x80, 0x104, font1, 0x10);
            play_speech(6);
        } else {
            font_list(0x48, 4, 0x80, 0x104, font1, 0x10);
            play_speech(7);
        }
    }

    font_list(0x48, 5, 0xe0, 0x128, font1, 0x10);
    font_list(0x47, 5, 0x30, 0x170, font1, 0x10);

    x_is = 0;
    font_no(army_list[our_battle_army].num_regulars, 0x20, " ", 0x30, 0x182, font1, 0x10);
    font_list(0x47, 6, x_is + 0x30, 0x182, font1, 0x10);
    font_no(our_battle_regs - army_list[our_battle_army].num_regulars,
            0x28, " ", x_is + 0x30, 0x182, font1, 0x10);
    font_list(0x48, 6, x_is + 0x30, 0x182, font1, 0x10);

    x_is = 0;
    font_no(army_list[our_battle_army].num_irregulars, 0x20, " ", 0x30, 0x194, font1, 0x10);
    font_list(0x47, 7, x_is + 0x30, 0x194, font1, 0x10);
    font_no(our_battle_irregs - army_list[our_battle_army].num_irregulars,
            0x28, " ", x_is + 0x30, 0x194, font1, 0x10);
    font_list(0x48, 6, x_is + 0x30, 0x194, font1, 0x10);

    x_is = 0;
    font_no(army_list[our_battle_army].num_auxillaries, 0x20, " ", 0x30, 0x1a6, font1, 0x10);
    font_list(0x47, 8, x_is + 0x30, 0x1a6, font1, 0x10);
    font_no(our_battle_auxs - army_list[our_battle_army].num_auxillaries,
            0x28, " ", x_is + 0x30, 0x1a6, font1, 0x10);
    font_list(0x48, 6, x_is + 0x30, 0x1a6, font1, 0x10);

    x_is = 0;
    font_no(army_list[our_battle_army].num_specials, 0x20, " ", 0x30, 0x1b8, font1, 0x10);
    font_list(0x47, 9, x_is + 0x30, 0x1b8, font1, 0x10);
    font_no(our_battle_specials - army_list[our_battle_army].num_specials,
            0x28, " ", x_is + 0x30, 0x1b8, font1, 0x10);
    font_list(0x48, 6, x_is + 0x40, 0x1b8, font1, 0x10);

    x_is = 0;
    font_list(7, army_list[their_battle_army].source_region, 0x150, 0x170, font1, 0x10);
    font_list(0x47, 0xa, x_is + 0x150, 0x170, font1, 0x10);

    if (their_battle_specials != 0) {
        x_is = 0;
        font_no(army_list[their_battle_army].num_specials, 0x20, " ", 0x150, 0x182, font1, 0x10);
        font_list(0x47, 0xb, x_is + 0x150, 0x182, font1, 0x10);
        font_no(their_battle_specials - army_list[their_battle_army].num_specials,
                0x28, " ", x_is + 0x150, 0x182, font1, 0x10);
        font_list(0x48, 6, x_is + 0x150, 0x182, font1, 0x10);
    } else {
        x_is = 0;
        font_no(army_list[their_battle_army].num_horse, 0x20, " ", 0x150, 0x182, font1, 0x10);
        font_list(0x47, 0xb, x_is + 0x150, 0x182, font1, 0x10);
        font_no(their_battle_horse - army_list[their_battle_army].num_horse,
                0x28, " ", x_is + 0x150, 0x182, font1, 0x10);
        font_list(0x48, 6, x_is + 0x150, 0x182, font1, 0x10);
    }

    x_is = 0;
    font_no(army_list[their_battle_army].num_regulars, 0x20, " ", 0x150, 0x194, font1, 0x10);
    font_list(0x47, 0xc, x_is + 0x150, 0x194, font1, 0x10);
    font_no(their_battle_regs - army_list[their_battle_army].num_regulars,
            0x28, " ", x_is + 0x150, 0x194, font1, 0x10);
    font_list(0x48, 6, x_is + 0x150, 0x194, font1, 0x10);

    x_is = 0;
    font_no(army_list[their_battle_army].num_irregulars, 0x20, " ", 0x150, 0x1a6, font1, 0x10);
    font_list(0x47, 0xd, x_is + 0x150, 0x1a6, font1, 0x10);
    font_no(their_battle_irregs - army_list[their_battle_army].num_irregulars,
            0x28, " ", x_is + 0x150, 0x1a6, font1, 0x10);
    font_list(0x48, 6, x_is + 0x150, 0x1a6, font1, 0x10);

    x_is = 0;
    font_no(army_list[their_battle_army].num_auxillaries, 0x20, " ", 0x150, 0x1b8, font1, 0x10);
    font_list(0x47, 0xe, x_is + 0x150, 0x1b8, font1, 0x10);
    font_no(their_battle_auxs - army_list[their_battle_army].num_auxillaries,
            0x28, " ", x_is + 0x150, 0x1b8, font1, 0x10);
    font_list(0x48, 6, x_is + 0x150, 0x1b8, font1, 0x10);

    hold_mouse_replace = 1;
    refresh_svga_screen();
    set_palette(city_palette);
}
