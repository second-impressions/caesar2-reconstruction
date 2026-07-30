#include "c2_data.h"
#include "c2_types.h"

int mouse_styles[10] = { 0, 1, 2, 3, 9, 0, 2, 3, 4, 4 };

#if PLATFORM_WINDOWS
int game_speed_delays[10] = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90 };
int scroll_speed_delays[10] = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90 };
#define GAME_SPEED_DELAY(delay) game_speed_delays[delay]
#define SCROLL_SPEED_DELAY(delay) scroll_speed_delays[delay]
#else
#define GAME_SPEED_DELAY(delay) ((delay) * 50 + 50)
#define SCROLL_SPEED_DELAY(delay) ((delay) * 50 + 20)
#endif

extern int colour_cycle_delay1();
extern int colour_cycle_delay2();


extern void put_a_font_string(char *text, int text_x, int text_y, unsigned char *font_ptr, int color);
extern void font_list(int string_idx, int word_count, int list_x, int list_y, unsigned char *font_ptr, int color);
extern int  get_fb_width(unsigned char *font_ptr);
extern void show_cursor(unsigned char *font_ptr);

extern void exit_screen_void(void);

/* Forward declarations (functions defined later in this file). */
void explain_forum(void);
void forum_explanations(int department_idx, int highlight_flag);


// Starts a UI frame by advancing the cycle count, restoring the mouse background, polling input,
// and advancing the random-number generator.
// FUNCTION: C2 0x3d399
// FUNCTION: C2WIN 0x0040f7f0
void gloop_start(void)
{
    cycle_count++;
    cover_mouse_droppings();
    get_mouse();
    random();
}

// Finishes a UI frame by drawing the cursor, refreshing the screen, servicing audio, and updating
// the elapsed input time.
// FUNCTION: C2 0x3d9df FOLDED
// FUNCTION: C2WIN 0x0040f810
void gloop_end(void)
{
    get_mouse_droppings();
    show_mouse(pointer_mode);
#if !PLATFORM_WINDOWS
    set_mouse_refresh();
#endif
    refresh_svga_screen();
    continue_db();
    button_time_flag = running_delay1();
}

// Finishes a modal UI frame by drawing the cursor, refreshing the screen, servicing audio, and
// updating the elapsed input time.
// FUNCTION: C2 0x3d9df FOLDED
// FUNCTION: C2WIN 0x0040f844
void mloop_end(void)
{
    get_mouse_droppings();
    show_mouse(pointer_mode);
#if !PLATFORM_WINDOWS
    set_mouse_refresh();
#endif
    refresh_svga_screen();
    continue_db();
    button_time_flag = running_delay1();
}

// Finishes a forum frame, using the forum-hover cursor when a department is under the pointer.
// FUNCTION: C2 0x3d3ae
// FUNCTION: C2WIN 0x0040f878
void floop_end(void)
{
    get_mouse_droppings();
    if (forum_dept_over) show_mouse(0x15);
    else
        show_mouse(pointer_mode);
#if !PLATFORM_WINDOWS
    set_mouse_refresh();
#endif
    refresh_svga_screen();
    continue_db();
    button_time_flag = running_delay1();
}

// Advances the city simulation when due, renders the active map and interface, handles player
// actions, and services animation and ambient audio for one frame.
// FUNCTION: C2 0x3d3ca
// FUNCTION: C2WIN 0x0040fb4a
void main_game_loop(void)
{
    int simulation_step_count;
    int simulation_step_idx;

    cycle_count++;
    button_time_flag = running_delay1();

    if (game_speed() != 0) {
        if (turbo_mode != 0)
            simulation_step_count = 4;
        else
            simulation_step_count = 1;
        for (simulation_step_idx = 0; simulation_step_idx < simulation_step_count; simulation_step_idx++) {
            do_32_count();
            random();
            citymap_evolution();
            if (game_state == 3) return;
            if (game_state == 2) return;
            if (game_state == 1) return;
            citizen_intelligence();
            army_intelligence();
        }
    }
    cover_mouse_droppings();
    get_mouse();

    if (turbo_mode > 1) {
        show_turbo_panel();
    } else if (map_mode == 0) {
        show_citymap();
    } else if (map_mode == 1 && pointer_mode != 5) {
        show_regionmap();
    }

    if (turbo_mode < 2) {
        write_image(misc, map_direction / 2, 0x1c4, 0x1a);
        old_pm_over = pm_over;
        pm_over = get_pm_over_diamond(0);
        if (pm_over != 0 && pointer_mode == 0 && particles_built == 0) {
            show_diamond_ptr();
        }
        if (pm_over != 0) {
            over_ptr = pm_over_cm_ptr / map_actual_atom;
            over_x = over_ptr % map_actual_width;
            over_y = over_ptr / map_actual_width;
        }
        get_over_army();
    }

    show_top_line();
    show_icon_strip();
    show_tutorial_timer();
    show_ov_bar();
    show_paused();
    show_querymode_panel();

    if (pointer_mode == 5) {
        if (gen_refresh1) {
            if (army_list[tracking_army].type == 1) {
                show_cohort_box();
            } else {
                show_non_cohort_box();
            }
            gen_refresh1 = 0;
        }
        if (army_list[tracking_army].type == 1)
            update_tribune_flag(0);
        if (army_list[tracking_army].type == 1)
            show_buttons(0x190, 0x82, cohort_buttons, 1);
    }

    if (scrolling)
        setup_map_screen_refresh();

    if (update_landfill) {
        show_landfill(com_x, com_y);
        setup_refresh_area(0x1e0, 0x30, 0xa, 0xb, 1);
        update_landfill--;
    }

    redraw_icon_bits();
    get_mouse_droppings();

    if (pm_over != 0)
        refresh_a_square(pm_over_x >> 4, pm_over_y >> 4, 2);

    /* Select the cursor for the active tool and hovered map object. */
    if (flag_mode) {
        show_mouse(0xa);
    } else if (illegal_build == 1) {
        show_mouse(0x14);
    } else if (illegal_build == 2) {
        show_mouse(0x13);
    } else if (over_an_army) {
        show_mouse(0x11);
    } else if ((pointer_mode == 2 || pointer_mode == 6) && pm_over == 0) {
        show_mouse(mouse_styles[0]);
    } else {
        show_mouse(mouse_styles[pointer_mode]);
    }
    set_mouse_refresh();
    refresh_svga_screen();

    if (!scrolling && colour_cycle_delay1(0x3c) != 0 && c2inf.paused == 0) {
        if (map_mode == 0) {
            cycle_colours(0x40, 0x47);
        } else {
            cycle_colours(0x41, 0x43);
        }
        pulse_red(0x48, 6);
    }
    if (!scrolling && colour_cycle_delay2(0x96) != 0 && c2inf.paused == 0 && map_mode == 0) {
        cycle_colours(0x50, 0x55);
    }

    if (stopped_scrolling) {
        update_landfill = 1;
        clear_edge_info();
        setup_whole_screen_refresh();
        update_map = 1;
    }

    if (turbo_mode != 0)
        turbo_mode = turbo_mode + 1;

    if (flag_mode != 0)
        flag_mode_action();
    else
        action();

    if (hot_exit_flag) {
        act_exit_game();
        hot_exit_flag = 0;
    }
    if (restart_flag != 0)
        return;

    if (flag_mode_decay_count != 0) {
        flag_mode_decay_count = flag_mode_decay_count - 1;
        if (flag_mode_decay_count <= 0) {
            flag_mode = 0;
            setup_map_screen_refresh();
        }
    }
    if (!mouse_left_button)
        show_messages();
    pm_limits();
    play_ambient_fx();
    continue_db();
}

// Advances battle units when due, renders the battle map and panels, handles battle input, and
// services screen and audio updates for one frame.
// FUNCTION: C2 0x3d816
// FUNCTION: C2WIN 0x004101e4
void battle_game_loop(void)
{
    cycle_count++;
    button_time_flag = running_delay1();
    figure_update();
    get_units_status();
    random();

    if (game_speed() != 0 || battle_turbo != 0) {
        if (zoom_level == 1) {
            arrow_update();
            update_units_morale();
            update_units_ai();
            figure_intelligence();
            arrow_intelligence();
            do_32_count();
        }
    }
    cover_mouse_droppings();
    get_mouse();

    if (battle_setup_count != 0) {
        battle_setup_count--;
        if (battle_setup_count <= 1) {
            setup_battle_screen_refresh();
            update_map = 1;
        }
    }

    if (battle_turbo == 0 || scrolling != 0) {
        show_battlemap();
    } else {
        battle_turbo_count++;
        if (battle_turbo_count > 0x14) {
            battle_turbo_count = 0;
            show_battlemap();
            setup_battle_screen_refresh();
        }
    }

    write_image(misc, map_direction / 2, 0x264, 0x1a);
    old_pm_over = pm_over;
    pm_over = get_pm_over_diamond(0);
    show_top_line();
    battle_totals_panel();
    battle_stats_panel();

    if (scrolling)
        setup_battle_screen_refresh();

    if (update_landfill) {
        show_battle_landfill(0, 0x34, 0xb1, 0x170);
        update_landfill--;
    } else {
        update_battle_landfill();
    }

    redraw_icon_bits();
    get_mouse_droppings();
    show_mouse(mouse_styles[0]);
    set_mouse_refresh();
    refresh_svga_screen();

    if (stopped_scrolling) {
        update_landfill = 1;
        clear_edge_info();
        setup_whole_screen_refresh();
    }
    battle_action();

    if (hot_exit_flag) {
        act_exit_game();
        hot_exit_flag = 0;
    }
    if (restart_flag == 0) {
        pm_limits();
        if (battle_turbo == 0)
            play_ambient_fx();
        continue_db();
    }
}

// Provides an empty hook for drawing content above the mouse cursor.
// FUNCTION: C2 0x3d9d9
void show_mouse_top(void)
{
}

// Polls input and refreshes one otherwise idle UI frame.
// FUNCTION: C2 0x3d9da FOLDED
void just_idle_game_loop(void)
{
    gloop_start();
    gloop_end();
}

// Handles the administration forum controls and refreshes the tax-rate and estimate displays.
// FUNCTION: C2 0x3da0a
// FUNCTION: C2WIN 0x0041048d
void forum_admin_game_loop(void)
{
    gloop_start();
    show_buttons(0x178, 0x12, admin_buttons, 4);
    if (gen_refresh1) {
        gen_refresh1 = 0;
        show_tax_rates();
        show_estimate();
    }
    control_buttons(0x178, 0x12, admin_buttons, 4);
    floop_end();
    if (mouse_right_click) {
        last_forum_dept = FORUM_DEPT_ADMIN;
        forum_dept = FORUM_DEPT_OVERVIEW;
        out1 = 2;
    }
}

// Handles the career forum controls and refreshes the personal-cash display.
// FUNCTION: C2 0x3da8a
// FUNCTION: C2WIN 0x00410515
void forum_career_game_loop(void)
{
    gloop_start();
    show_buttons(0xe0, 0x58, career_buttons, 3);
    if (gen_refresh1) {
        gen_refresh1 = 0;
        show_personal_cash_stats();
    }
    floop_end();
    control_buttons(0xe0, 0x58, career_buttons, 3);
    if (mouse_right_click) {
        last_forum_dept = FORUM_DEPT_CLERKS;
        forum_dept = FORUM_DEPT_OVERVIEW;
        out1 = 2;
    }
}

// Handles donation controls, refreshes the selected donation level, and closes on cancel or exit.
// FUNCTION: C2 0x3db05
// FUNCTION: C2WIN 0x00410598
void donation_game_loop(void)
{
    gloop_start();
    show_buttons(0x160, 0x38, donation_buttons, 3);
    if (gen_refresh1) {
        gen_refresh1 = 0;
        show_donation_level();
    }
    gloop_end();
    control_buttons(0x160, 0x38, donation_buttons, 3);
    if (mouse_right_click)
        out1 = 1;
    if (exit_screen())
        out1 = 1;
}

// Handles the Rome forum panel, disabling its action after the emperor has been warned this month.
// FUNCTION: C2 0x3db84
// FUNCTION: C2WIN 0x00410624
void forum_rome_game_loop(void)
{
    gloop_start();
    if (!warned_of_emperor_reply_month)
        show_buttons(0x150, 0x78, rome1_buttons, 1);
    floop_end();
    if (!warned_of_emperor_reply_month)
        control_buttons(0x150, 0x78, rome1_buttons, 1);
    if (mouse_right_click) {
        last_forum_dept = FORUM_DEPT_ROME;
        forum_dept = FORUM_DEPT_OVERVIEW;
        out1 = 2;
    }
}

// Handles the emperor-gift controls and displays the gift amount selected by `gift_index`.
// FUNCTION: C2 0x3dbfb
// FUNCTION: C2WIN 0x004106aa
void gift_game_loop(int gift_index)
{
    gloop_start();
    show_buttons(0x160, gift_index * 16 - 8, rome2_buttons, 3);
    if (gen_refresh1) {
        gen_refresh1 = 0;
        show_gift_amount(gift_index);
    }
    gloop_end();
    control_buttons(0x160, gift_index * 16 - 8, rome2_buttons, 3);
    if (mouse_right_click)
        out1 = 1;
}

// Displays temple advice and lets the player choose which temple receives the selected tip.
// FUNCTION: C2 0x3dc73
// FUNCTION: C2WIN 0x00410736
void forum_temple_game_loop(void)
{
    int temple_idx;

    gloop_start();
    if (gen_refresh1) {
        gen_refresh1--;
        show_temple_tip();
    }
    gloop_end();
    if (mouse_left_preclick) {
        for (temple_idx = 0; temple_idx < 4; temple_idx++) {
            if (mouse_in_area(temple_idx * 160 + 10, 0x164, 0x8c, 0x21)) {
                act_set_temple_tips(temple_idx);
                break;
            }
        }
    }
    if (mouse_right_click) {
        last_forum_dept = FORUM_DEPT_TEMPLE;
        forum_dept = FORUM_DEPT_OVERVIEW;
        out1 = 2;
    }
}

// Handles the clerks forum controls and refreshes the selected history graph.
// FUNCTION: C2 0x3dd00
// FUNCTION: C2WIN 0x004107f6
void forum_clerks_game_loop(void)
{
    gloop_start();
    show_buttons(0x40, 0x6c, clerk_buttons, 2);
    if (gen_refresh1) {
        gen_refresh1 = 0;
        history_graphs();
        history_selection();
    }
    control_buttons(0x40, 0x6c, clerk_buttons, 2);
    floop_end();
    if (mouse_right_click) {
        last_forum_dept = FORUM_DEPT_CAREER;
        forum_dept = FORUM_DEPT_OVERVIEW;
        out1 = 2;
    }
}

// Refreshes the advisor forum and returns to the forum overview on a right-click.
// FUNCTION: C2 0x3dd80
// FUNCTION: C2WIN 0x00410878
void forum_advisor_game_loop(void)
{
    gloop_start();
    gloop_end();
    if (mouse_right_click) {
        last_forum_dept = FORUM_DEPT_ADVISOR;
        forum_dept = FORUM_DEPT_OVERVIEW;
        out1 = 2;
    }
}

// Handles the empire map forum, including region details, known-world labels, and returning to the
// forum overview.
// FUNCTION: C2 0x3ddaf
// FUNCTION: C2WIN 0x004108b4
void forum_empire_game_loop(void)
{
    if (c2inf.peace_mode) {
        forum_advisor_game_loop();
        return;
    }
    gloop_start();
    get_region_over();
    show_empire_top_slab();

    if (region_over != 0) {
        if (known_world(region_over - 1)) {
            font_centre(6, region_over, 0xd8, 0x1e, 0xdc, font1, 0x3f);
        } else {
            font_centre(0x30, 4, 0xd8, 0x1e, 0xdc, font1, 0x3f);
        }
    } else {
        x_is = 0;
        font_list(0x22, 2, 0xd8, 0x1e, font1, 0x3f);
        show_date(year, x_is + 0xd8, 0x1e, 2);
    }
    setup_refresh_area(0xd2, 0x1a, 0x12, 2, 1);
    gloop_end();

    if (mouse_left_preclick && region_over != 0 && known_world(region_over - 1)) {
        this_region_box(1);
        out1 = 0;
        while (out1 == 0) {
            just_idle_game_loop();
            if (mouse_right_click)
                out1 = 1;
        }
        stop_db();
        clear_mouse();
        out1 = 0;
        basic_empire_screen();
    }

    if (mouse_right_click) {
        last_forum_dept = FORUM_DEPT_EMPIRE;
        forum_dept = FORUM_DEPT_OVERVIEW;
        out1 = 2;
        readfile("forumbit.pl8", ((void *)scratch_buffer), 0xea60, 0);
        readfile("forum_x.gd8", ((void *)((scratch_buffer) + (0x1d4c0))), 0xfa0, 0);
    }
}

// Handles army recruitment, tribune, and mercenary controls in the forum.
// FUNCTION: C2 0x3df63
// FUNCTION: C2WIN 0x00410a8d
void forum_army_game_loop(void)
{
    if (c2inf.peace_mode) {
        forum_advisor_game_loop();
        return;
    }
    no_of_army_buttons = 5;
    if (total_no_of_cohorts > 0) {
        if (forum_viewed_army == 10)
            no_of_army_buttons = 7;
        else
            no_of_army_buttons = 8;
    }
    gloop_start();
    update_tribune_flag(1);
    show_buttons(0x18, 0x30, army_buttons, no_of_army_buttons);
    if (max_mercs_allowed) {
        show_buttons(0x18, 0x30, mercenary_buttons, 2);
    }
    if (gen_refresh1) {
        gen_refresh1 = 0;
        show_recruitment();
    }
    if (gen_refresh2) {
        gen_refresh2 = 0;
        show_this_tribune();
    }
    if (gen_refresh3) {
        gen_refresh3 = 0;
        show_mercs();
    }
    control_buttons(0x18, 0x30, army_buttons, no_of_army_buttons);
    if (max_mercs_allowed) {
        control_buttons(0x18, 0x30, mercenary_buttons, 2);
    }
    floop_end();
    if (mouse_right_click) {
        last_forum_dept = FORUM_DEPT_ARMY;
        forum_dept = FORUM_DEPT_OVERVIEW;
        out1 = 2;
    }
}

// Refreshes the industry forum and returns to the forum overview on a right-click.
// FUNCTION: C2 0x3e09e
// FUNCTION: C2WIN 0x00410be8
void forum_industry_game_loop(void)
{
    gloop_start();
    floop_end();
    if (mouse_right_click) {
        last_forum_dept = FORUM_DEPT_INDUSTRY;
        forum_dept = FORUM_DEPT_OVERVIEW;
        out1 = 2;
    }
}

// Handles slave welfare and allocation controls, including assigning a workforce to its need
// level.
// FUNCTION: C2 0x3e0cd
// FUNCTION: C2WIN 0x00410c24
void forum_slaves_game_loop(void)
{
    int need_level;

    gloop_start();
    if (gen_refresh1) {
        gen_refresh1 = 0;
        show_slave_welfare_bill();
    }
    if (gen_refresh2) {
        gen_refresh2 = 0;
        show_slave_allocation();
    }
    show_buttons(0x40,  0x72, slave1_buttons, 2);
    show_buttons(0x1ae, 0x26, slave2_buttons, 0xc);
    control_buttons(0x40,  0x72, slave1_buttons, 2);
    control_buttons(0x1ae, 0x26, slave2_buttons, 0xc);
    floop_end();
    if (mouse_left_preclick) {
        for (need_level = 1; need_level < 7; need_level++) {
            if (mouse_in_area(0x216, need_level * 24 + 18, 0x50, 0x14)) {
                act_set_slaves_to_need_level(need_level);
                break;
            }
        }
    }
    if (mouse_right_click) {
        last_forum_dept = FORUM_DEPT_SLAVES;
        forum_dept = FORUM_DEPT_OVERVIEW;
        out1 = 2;
    }
}

// Refreshes an otherwise idle forum frame and displays contextual department help.
// FUNCTION: C2 0x3e1d3
// FUNCTION: C2WIN 0x00410d4f
void forum_idle_game_loop(void)
{
    gloop_start();
    explain_forum();
    floop_end();
    if (mouse_right_click)
        out1 = 1;
}

// Updates each forum department label, highlighting the department under the pointer.
// FUNCTION: C2 0x3e1f6
// FUNCTION: C2WIN 0x00410d82
void explain_forum(void)
{
    int department_idx;

    if (out1 == 1) return;
    for (department_idx = 0; department_idx < FORUM_DEPT_END; department_idx++) {
        if (department_idx == forum_dept_over)
            forum_explanations(department_idx, 1);
        else
            forum_explanations(department_idx, 0);
    }
}

// Draws one forum-department label at its menu position, using the highlighted color when selected.
// FUNCTION: C2 0x3e227
// FUNCTION: C2WIN 0x00410df1
void forum_explanations(int department_idx, int highlight_flag)
{
    int label_x;
    int label_y;

    label_x = forum_menu[department_idx].x + 8;
    label_y = forum_menu[department_idx].y + 5;
    stone_random_count = 0xb;
    show_a_mosaic_blank(label_x, label_y, 9, 1);
    if (highlight_flag == 0)
        font_list(0x1d, department_idx, label_x + 4, label_y + 2, font1, 0x10);
    else
        font_list(0x1d, department_idx, label_x + 4, label_y + 2, font1, 0xb);
    setup_refresh_area(label_x, label_y, 0xa, 2, 1);
}

// Polls input and refreshes one otherwise idle year-end frame.
// FUNCTION: C2 0x3d9da FOLDED
void year_end_game_loop(void)
{
    gloop_start();
    gloop_end();
}

// Displays and handles the battle-introduction confirmation controls.
// FUNCTION: C2 0x3e2a3
// FUNCTION: C2WIN 0x00410ea4
void battle_intro_game_loop(void)
{
    gloop_start();
    show_buttons(0x100, 0x104, confirming_buttons, 2);
    gloop_end();
    control_buttons(0x100, 0x104, confirming_buttons, 2);
}

// Displays and handles music-selection controls, closing on a right-click.
// FUNCTION: C2 0x3e2e2
// FUNCTION: C2WIN 0x00410eeb
void tune_game_loop(void)
{
    gloop_start();
    show_buttons(0x50, 0x80, tunes_buttons, 2);
    gloop_end();
    control_buttons(0x50, 0x80, tunes_buttons, 2);
    if (mouse_right_click)
        out1 = 1;
}

// Displays and handles sound-sample controls, closing on a right-click.
// FUNCTION: C2 0x3e338
// FUNCTION: C2WIN 0x00410f45
void samples_game_loop(void)
{
    gloop_start();
    show_buttons(0x50, 0x80, samples_buttons, 5);
    gloop_end();
    control_buttons(0x50, 0x80, samples_buttons, 5);
    if (mouse_right_click)
        out1 = 1;
}

// Displays and handles the animation toggle, closing on a right-click.
// FUNCTION: C2 0x3e38e
// FUNCTION: C2WIN 0x00410f9f
void tog_anims_game_loop(void)
{
    gloop_start();
    show_buttons(0x50, 0x80, tog_anims_buttons, 1);
    gloop_end();
    control_buttons(0x50, 0x80, tog_anims_buttons, 1);
    if (mouse_right_click)
        out1 = 1;
}

// Displays and handles year-end option toggles, closing on a right-click.
// FUNCTION: C2 0x3e3e4
// FUNCTION: C2WIN 0x00410ff9
void tog_yearend_game_loop(void)
{
    gloop_start();
    show_buttons(0x50, 0x80, tog_yearend_buttons, 2);
    gloop_end();
    control_buttons(0x50, 0x80, tog_yearend_buttons, 2);
    if (mouse_right_click)
        out1 = 1;
}

// Displays and handles the exit-game confirmation controls.
// FUNCTION: C2 0x3e43a
// FUNCTION: C2WIN 0x00411053
void exit_game_loop(void)
{
    gloop_start();
    show_buttons(0, 0, exit_buttons, 3);
    gloop_end();
    control_buttons(0, 0, exit_buttons, 3);
}

// Displays and handles the initial skill-selection controls.
// FUNCTION: C2 0x3e46a
// FUNCTION: C2WIN 0x0041108e
void skill1_game_loop(void)
{
    gloop_start();
    show_buttons(0x50, 0x50, skill1_buttons, 4);
    gloop_end();
    control_buttons(0x50, 0x50, skill1_buttons, 4);
}

// Handles the detailed skill and peace-level controls, refreshing their displayed values as
// needed.
// FUNCTION: C2 0x3e4a7
// FUNCTION: C2WIN 0x004110c9
void skill2_game_loop(void)
{
    gloop_start();
    if (gen_refresh1) {
        gen_refresh1 = 0;
        show_skill_level();
    }
    if (gen_refresh2) {
        gen_refresh2 = 0;
        show_peace_level();
    }
    show_buttons(0x50, 0x50, skill2_buttons, 6);
    gloop_end();
    control_buttons(0x50, 0x50, skill2_buttons, 6);
}

// Handles initial province selection on the empire map and records the chosen province and
// difficulty.
// FUNCTION: C2 0x3e502
// FUNCTION: C2WIN 0x0041113a
void initreg_game_loop(void)
{


    gloop_start();
    if (out2 == 0) {
        get_region_over();
        show_empire_top_slab();
        if (region_over != 0) {
            x_is = 0;
            if (known_world(region_over - 1)) {
                font_centre(6, region_over, 0xd2, 0x1e, 0xdc, font1, 0x3f);
            } else {
                font_centre(0x30, 4, 0xd2, 0x1e, 0xdc, font1, 0x3f);
            }
        } else {
            x_is = 0;
            font_list(0x22, 2, 0xd8, 0x1e, font1, 0x3f);
            show_date(year, x_is + 0xd8, 0x1e, 2);
        }
#if C2_FEAT_TILE_REFRESH
        setup_refresh_area(0xd2, 0x1a, 0x12, 2, 1);
#endif
    }
    gloop_end();

    if (out2 > 1) {
        out2 = out2 - 1;
        goto end;
    }
    if (!mouse_left_click)
        goto end;
    if (region_over == 0)
        goto end;
    if ((empire[region_over - 1] & 0xff) != 2)
        goto end;

    this_region();
    if (decision == 1) {
        out2 = 0x64;
        province_is = region_over - 1;
        province_difficulty = empire_region_order[region_over + 10];
        empire[region_over - 1] = 6;
    }
    reshow_initreg_box();
end:;
}

// Edits and redraws the province name, closing the entry field on Escape, Enter, or a right-click.
// FUNCTION: C2 0x3e673
// FUNCTION: C2WIN 0x004112b7
void new_name_game_loop(void)
{
    int field_y;

    hold_hot_keys = 1;
    gloop_start();
    if (edit_format_buffer())
        out2 = 1;
    get_fb_length();
    out_format_buffer(c2inf.player_name);
    got_cursx = 0;
    cursor_x = 0;
    fb_count = 0;
    field_y = 0xe0;
    cursor_y = field_y;
    allow_padding = 1;
    x_is = 0;
    show_a_system_blank(field_y, 0xd8, 0xc, 2);
    put_a_font_string(c2inf.player_name, 0xe2, field_y, font1, 0x10);
    fb_max_width_reached = (get_fb_width(font1) > fb_line_length);
    if (got_cursx == 0) {
        cursor_x = x_is;
        got_cursx = 1;
    }
    cursor_x += 0xe2;
    show_cursor(font1);
    setup_refresh_area(0xe0, 0xd0, 0xc, 3, 1);
    if (key_ascii == 0x1b) out2 = 1;
    if (key_ascii == 0xd)  out2 = 1;
    if (mouse_right_click) out2 = 1;
    gloop_end();
}

// Displays and handles the help-screen controls for one frame.
// FUNCTION: C2 0x3e7b1
// FUNCTION: C2WIN 0x00411428
void help_game_loop(void)
{
    gloop_start();
    show_buttons(0x168, 0x1a0, help_buttons, 2);
    control_buttons(0x168, 0x1a0, help_buttons, 2);
    gloop_end();
}

// Handles query controls and updates the queried person, adding the region-specific control when
// required.
// FUNCTION: C2 0x3e7f4
// FUNCTION: C2WIN 0x0041146f
void queery_game_loop(void)
{
    gloop_start();
    show_buttons(8, 0x20, queery_buttons, nof_query_buttons);
    if (map_mode == 1 && q_type == 0x92)
        show_buttons(8, 0x20, query_buttons2, 1);
    gloop_end();
    control_buttons(8, 0x20, queery_buttons, nof_query_buttons);
    if (map_mode == 1 && q_type == 0x92)
        control_buttons(8, 0x20, query_buttons2, 1);
    get_queried_person();
}

// Displays and handles the promotion-decision controls.
// FUNCTION: C2 0x3e8aa
// FUNCTION: C2WIN 0x00411521
void promotion_game_loop(void)
{
    gloop_start();
    show_buttons(0x80, 0x70, promotion_buttons, 3);
    gloop_end();
    control_buttons(0x80, 0x70, promotion_buttons, 3);
}

// Returns whether enough unpaused, non-scrolling input time has elapsed to advance the simulation.
// FUNCTION: C2 0x3e8e9
// FUNCTION: C2WIN 0x00411562
int game_speed(void)
{
    int speed_delay;

    cmu_count[1] += button_time_flag;
    if (turbo_mode > 1) return 1;

    speed_delay = (100 - c2inf.game_speed) / 10;
#if PLATFORM_WINDOWS
    if (speed_delay >= 10)
        return 0;
    if (c2inf.paused)
        return 0;
    if (scrolling)
        return 0;
    if (flag_mode != 0)
        return 0;
    if (pointer_mode >= 5)
        return 0;
    if (mouse_left_button)
        return 0;
    if (GAME_SPEED_DELAY(speed_delay) <= cmu_count[1]) {
        cmu_count[1] = 0;
        return 1;
    }
    return 0;
#else
    if (speed_delay < 10) {
        if (!c2inf.paused) {
            if (!scrolling) {
                if (flag_mode == 0) {
                    if (pointer_mode < 5) {
                        if (!mouse_left_button) {
                            if (GAME_SPEED_DELAY(speed_delay) <= cmu_count[1]) {
                                cmu_count[1] = flag_mode;
                                return 1;
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
#endif
}

// Returns whether enough input time has elapsed for another automatic map-scroll step.
// FUNCTION: C2 0x3e972
// FUNCTION: C2WIN 0x00411657
int scroll_speed(void)
{
    int scroll_delay;

    cmu_count[2] += button_time_flag;
    scroll_delay = (100 - c2inf.scroll_speed) / 10;
#if PLATFORM_WINDOWS
    if (scroll_delay >= 10)
        return 0;
    if (SCROLL_SPEED_DELAY(scroll_delay) <= cmu_count[2]) {
        cmu_count[2] = 0;
        return 1;
    }
    return 0;
#else
    if (scroll_delay < 10) {
        if (SCROLL_SPEED_DELAY(scroll_delay) <= cmu_count[2]) {
            cmu_count[2] = 0;
            return 1;
        }
    }
    return 0;
#endif
}

#undef SCROLL_SPEED_DELAY
#undef GAME_SPEED_DELAY
