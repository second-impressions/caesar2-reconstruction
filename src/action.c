
#include "c2_data.h"
#include "c2_types.h"

extern int affected_by_cover1();
extern int colour_cycle_delay1();


/* Local helper declarations. */
void save_a_game(void);
void clear_landfill(void);
int  pause_db(void);
void helping(int help_page_id);
void show_citymap(void);
void show_regionmap(void);
void show_battlemap(void);
void region_map_screen(int do_black_out);
void act_goto_message(void);
void act_goto_city_map(void);
void act_query_do_help(int page_delta);
void init_help_history(void);
void rewind_help_history(void);
void get_next_viewed_cohort(int dir);


/* Selection-box helpers. */
void get_selection_goods_list(int what);
int control_selection(struct selection_rec *list, int count, int x, int y, int width);
#if PLATFORM_WINDOWS
extern int selection_menu;
#endif
void show_fx_box(int what);
void stop_all_sounds(void);
void stop_db(void);

void act_house1(void);

int alter_slave_reqs(int requirement_idx, int allocation_delta);

void rotate_pm_clockwise(void);
void rotate_pm_anticlockwise(void);
void figure_images(void);
void clear_edge_info(void);

void general_reform(int kind);
void select_all_figures(void);
void goto_flag_marker_mode(void);
/* Forward declarations (functions defined later in this file). */
void scroll(void);
void mouse_follow_cohort(void);
void mouse_hunt_enemies(void);
void show_latest_route(void);
void prebuild_city_item(void);
void build_city_item(void);
void prebuild_region_item(void);
void build_region_item(void);
void get_icon_over(void);
void act_exit_turbo_mode(void);
void act_show_ov_legend(void);
void act_select_ov_map(void);
void act_rm_warehouse(void);
void act_rm_workhouse(void);
void act_set_patrol_markers(void);
void act_set_return_home(void);
void act_set_patrol_stop(void);
void do_act_zoom_out(int decayed_click);
void do_act_zoom_in(int decayed_click);
void act_correct_map(void);
void forum_game_loop(void);
void show_forum_screen(void);
void act_query(void);


// Main per-frame action dispatcher. Snapshots `scrolling`, dispatches based on `pointer_mode` and
// the various mouse-button flags, then triggers sounds and the scrolling-stop hook on exit.
// FUNCTION: C2 0x2ede2
// FUNCTION: C2WIN 0x004b0630
void action(void)
{
    int help_page_id;             /* Index into the active map's icon-help table. */
    int saved_reg_placing_type;   /* Preserves the region placement type across the selection dialog. */

    old_scrolling = scrolling;
    scrolling = 0;
    stopped_scrolling = 0;
    illegal_build = 0;

    if (turbo_mode > 1) {
        if (mouse_left_preclick == 0 && mouse_right_preclick == 0) {
            return;
        }
        act_exit_turbo_mode();
        clear_mouse();
        return;
    }

    action_sound = 0;
    if (zoom_in_decay_count != 0) zoom_in_decay_count++;

    if (tutorial_mode != 0 && exit_screen_at(0x250, 0x1b0) != 0) { out4 = 1; goto end_of_action; }

    get_icon_over();

    if (pointer_mode == 0 || pointer_mode == 2 || pointer_mode == 6) {
        if (map_mode == 0) {
            if (control_menus(main_menu, 4, show_citymap) != 0) { pointer_mode = 0; goto end_of_action; }
            if (perform_city_strip_action() != 0) { redraw_icons = 1; update_map = 1; goto end_of_action; }
            if (use_city_overmap_to_move() != 0) { pointer_mode = 0; show_landfill(com_x, com_y); setup_refresh_area(0x1e0, 0x18, 10, 0xb, 1); goto end_of_action; }
        } else if (map_mode == 1) {
            if (control_menus(main_menu, 4, show_regionmap) != 0) { pointer_mode = 0; unflag_all_rm_xwarehouse(); goto end_of_action; }
            if (perform_region_strip_action() != 0) { redraw_icons = 1; update_map = 1; unflag_all_rm_xwarehouse(); goto end_of_action; }
            if (use_region_overmap_to_move() != 0) { pointer_mode = 0; show_landfill(com_x, com_y); setup_refresh_area(0x1e0, 0x18, 10, 0xb, 1); unflag_all_rm_xwarehouse(); goto end_of_action; }
        }
    } else if (pointer_mode == 5) {
        if (perform_cohort_box_action() != 0) { goto end_of_action; }
    } else if (pointer_mode == 1 && last_icon_over == 0xd) {
        if (map_mode == 0) {
            if (perform_city_strip_action() != 0) { redraw_icons = 1; update_map = 1; goto end_of_action; }
        } else if (map_mode == 1) {
            if (perform_region_strip_action() != 0) { redraw_icons = 1; update_map = 1; goto end_of_action; }
        }
    }

    if (mouse_right_preclick != 0) {
        had_clear_sound = 0;
        unflag_all_rm_xwarehouse();
        setup_whole_screen_refresh();
        update_map = 1;
        flag_for_workhouse_request = 0;

        if (!(pointer_mode == 4)) {
            if (pointer_mode >= 6) {
                pointer_mode = 5; gen_refresh1 = 1; goto end_of_action;
            }
            if (pointer_mode >= 1) {
                pointer_mode = 0;
                goto end_of_action;
            }
        }

        if (over_an_army != 0) {
            tracking_army = over_an_army;
            pointer_mode = 5;
            gen_refresh1 = 1;
            setup_whole_screen_refresh();
        } else if (pm_over != 0) {
            act_query();
        } else if (last_icon_over != 0) {
            if (map_mode == 0) {
                help_page_id = city_icons_to_help[last_icon_over];
            } else {
                help_page_id = region_icons_to_help[last_icon_over];
            }
            if (help_page_id != 0) {
                clear_mouse();
                helping(help_page_id);
            }
        }
        goto end_of_action;
    }

    scroll();
    mouse_follow_cohort();
    show_latest_route();
    mouse_hunt_enemies();
    particles_cleared = 0;
    particles_built = 0;

    if (mouse_left_preclick != 0 && pm_over != 0) {
        update_map = 1;
        total_build_cost = 0;
        industry_build_on = 0;
        industry_build_ok = 0;

        if (over_an_army != 0
         && (reg_placing_type != 0x21
             || ((unsigned char)(*(struct region_cell *)((unsigned char *)region_map + (pm_over_cm_ptr))).base_kind) != 0xd2)) {
            tracking_army = over_an_army;
            pointer_mode = 5;
            gen_refresh1 = 1;
            setup_whole_screen_refresh();
            goto end_of_action;
        }

        if (pointer_mode == 1) {
            zoom_in_decay_count = 1;
            goto end_of_action;
        }
        if (pointer_mode == 2) {
            goto end_of_action;
        }
        if (!(pointer_mode != 3)) {
            pointer_mode = 5;
            gen_refresh1 = 1;
            setup_whole_screen_refresh();
            goto end_of_action;
        }
        if (pointer_mode == 4) {
            act_query();
            goto end_of_action;
        }
        if (pointer_mode == 5) {
            goto end_of_action;
        }
        if (pointer_mode == 6 || pointer_mode == 7 || pointer_mode == 8) {
            setup_map_screen_refresh();
            if (((*(struct region_cell *)((unsigned char *)region_map + (pm_over_cm_ptr))).place_state & 0xff) == 0xff) {
                return;
            }
            get_over_coords();

            if (pointer_mode == 7) {
                if (army_list[tracking_army].state_idx == 1) {
                    army_list[tracking_army].flags |= 1;
                }
                army_list[tracking_army].state_idx = 3;
                army_list[tracking_army].dest_y = 0;
                army_list[tracking_army].dest_x = 0;
                army_list[tracking_army].return_flag = 1;
                army_list[tracking_army].wf_active = 0;
                if (this_route_number >= 9) {
                    this_route_number = 10;
                }
                army_list[tracking_army].target_x = army_routes[army_list[tracking_army].cohort_id].points[0][0].x;
                army_list[tracking_army].target_y = army_routes[army_list[tracking_army].cohort_id].points[0][0].y;
                army_routes[army_list[tracking_army].cohort_id].army_x = army_list[tracking_army].x;
                army_routes[army_list[tracking_army].cohort_id].army_y = army_list[tracking_army].y;
                army_routes[army_list[tracking_army].cohort_id].over_x = over_x;
                army_routes[army_list[tracking_army].cohort_id].over_y = over_y;
                army_routes[army_list[tracking_army].cohort_id].row_count = this_route_number;
                army_routes[army_list[tracking_army].cohort_id].chase_row = 0;
                army_routes[army_list[tracking_army].cohort_id].target_army = 0;
                unflag_all_rm_xwarehouse();
                pointer_mode = 2;
                update_map = 1;
                goto end_of_action;
            } else if (pointer_mode == 8) {
                if (army_list[tracking_army].state_idx == 1) {
                    army_list[tracking_army].flags |= 1;
                }
                army_list[tracking_army].state_idx = 3;
                army_list[tracking_army].dest_y = 0;
                army_list[tracking_army].dest_x = 0;
                army_list[tracking_army].wf_active = 0;
                army_list[tracking_army].return_flag = 1;
                army_list[tracking_army].target_x = army_routes[army_list[tracking_army].cohort_id].points[0][0].x;
                army_list[tracking_army].target_y = army_routes[army_list[tracking_army].cohort_id].points[0][0].y;
                army_routes[army_list[tracking_army].cohort_id].army_x = army_list[tracking_army].x;
                army_routes[army_list[tracking_army].cohort_id].army_y = army_list[tracking_army].y;
                army_routes[army_list[tracking_army].cohort_id].over_x = over_x;
                army_routes[army_list[tracking_army].cohort_id].over_y = over_y;
                army_routes[army_list[tracking_army].cohort_id].row_count = this_route_number + 1;
                army_routes[army_list[tracking_army].cohort_id].chase_row = this_route_number;
                army_routes[army_list[tracking_army].cohort_id].target_army = hunting_army;
                if (this_route_number == 0) {
                    enemy_army = hunting_army;
                    army_list[tracking_army].army_id = enemy_army;
                    army_list[tracking_army].target_marker = army_list[enemy_army].evolve_timer;
                    army_list[tracking_army].state_idx = 4;
                }
                unflag_all_rm_xwarehouse();
                pointer_mode = 2;
                update_map = 1;
                goto end_of_action;
            } else {
                if (this_route_number < 9) {
                    this_route_number = this_route_number + 1;
                    set_route_elastic();
                    save_undo_info();
                }
                goto end_of_action;
            }
        }

        if (map_mode == 0) {
            prebuild_city_item();
            if (placing_type == 0xff) {
                goto end_of_action;
            }
        } else if (map_mode == 1) {
            prebuild_region_item();
            if (reg_placing_type == 0xff) {
                goto end_of_action;
            }
        }
        save_undo_info();
    }

    if (mouse_left_preclick != 0 && pointer_mode == 4) {
        if (map_mode == 0 && last_icon_over == 0x17) {
            pointer_mode = 0;
            redraw_icons = 1;
        } else if (map_mode == 1 && last_icon_over == 0x13) {
            pointer_mode = 0;
            redraw_icons = 1;
        } else if (last_icon_over != 0) {
            if (map_mode == 0) {
                help_page_id = city_icons_to_help[last_icon_over];
            } else {
                help_page_id = region_icons_to_help[last_icon_over];
            }
            if (help_page_id != 0) {
                helping(help_page_id);
            }
            goto end_of_action;
        }
    }

    if (mouse_left_button != 0 && pm_over != 0) {
        action_sound = 2;
        total_build_cost = 0;
        if (pointer_mode >= 1 && pointer_mode <= 9) {
            goto end_of_action;
        }
        if (map_mode == 0) {
            build_city_item();
        } else if (map_mode == 1) {
            build_region_item();
        }
        refresh_big_action_square((mouse_x - 0x50) >> 4,
                                  (mouse_y - 0x78) >> 4);
    }

    if (mouse_left_click != 0) {
        had_clear_sound = 0;
        if (pointer_mode >= 2 && pointer_mode <= 9) {
            action_sound = 2;
            goto end_of_action;
        }

        if (any_army_building_adjusts() != 0) {
            confirm(3, 0xa0, 0xa0);
            if (decision == 1) {
                army_building_adjusts();
            } else {
                restore_region_from_undo_buffer();
            }
        }
        clear_all_cm(2);

        if (reg_placing_type >= 0x25 && reg_placing_type <= 0x29) {
            if (industry_build_on != 0 && industry_build_ok == 0) {
                saved_reg_placing_type = reg_placing_type;
                if (reg_placing_type == 0x25) {
                    get_selection_goods_list(1);
                    control_selection(farm_selection, 5,
                                      mouse_x - 0x50, mouse_y - 0x50,
                                      0x11);
                    if (selection_is == 0) {
                        industry_build_ok = 1;
                    }
                } else if (reg_placing_type == 0x26) {
                    get_selection_goods_list(2);
                    control_selection(mine_selection, 5,
                                      mouse_x - 0x50, mouse_y - 0x50,
                                      0x12);
                    if (selection_is == 0) {
                        industry_build_ok = 1;
                    }
                } else if (reg_placing_type == 0x27) {
                    get_selection_goods_list(3);
                    control_selection(quarry_selection, 5,
                                      mouse_x - 0x50, mouse_y - 0x50,
                                      0x13);
                    if (selection_is == 0) {
                        industry_build_ok = 1;
                    }
                }
                if (reg_placing_type == 0) {
                    industry_build_ok = 1;
                } else {
                    industry_build_on = 0;
                }
                reg_placing_type = saved_reg_placing_type;
            }
            if (industry_build_ok != 0) {
                restore_region_from_undo_buffer();
                industry_build_ok = 0;
                industry_build_on = 0;
                total_build_cost = 0;
                denarii = starting_denarii;
                particles_built = 0;
                particles_cleared = 0;
            }
        }

        current_construction_cost = current_construction_cost + total_build_cost;
        if (total_build_cost != 0) {
            set_sound("place.wav", 1);
            if (map_mode == 0) {
                get_landfill(1);
                update_landfill = 1;
            }
            if (reg_placing_type >= 0x25 && reg_placing_type <= 0x29) {
                flag_for_workhouse_request = 0;
                extended_confirm(0xb, 0xa0, 0xa0);
                clear_mouse();
                if (decision == 1) {
                    if (reg_placing_type >= 0x25 && reg_placing_type <= 0x27) {
                        flag_for_workhouse_request = 1;
                    }
                    act_rm_warehouse();
                }
            } else if (reg_placing_type == 0x24
                       && flag_for_workhouse_request != 0) {
                extended_confirm(0xc, 0xa0, 0xa0);
                clear_mouse();
                if (decision == 1) {
                    act_rm_workhouse();
                }
                flag_for_workhouse_request = 0;
            } else {
                flag_for_workhouse_request = 0;
            }
        }
        total_build_cost = 0;
        update_landfill = 1;
        update_map = 1;
        if (pointer_mode == 1 && zoom_in_decay_count != 0) {
            if (zoom_in_decay_count > 3) {
                do_act_zoom_in(1);
            } else {
                do_act_zoom_in(0);
            }
            pointer_mode = 0;
            zoom_in_decay_count = 0;
            goto end_of_action;
        }
    }

    if (action_sound != 1) {
        action_sound = 2;
    }

end_of_action:
    if (action_sound == 1) {
        do_neg();
    } else if (action_sound == 0) {
        do_pos();
    }
    if (old_scrolling != scrolling && old_scrolling == 1) {
        stopped_scrolling = 1;
    }

}

// Per-frame dispatcher used while the player is in flag-marker (banner) placement mode. Mostly
// delegates to the city/region strip-action helpers and lets the user toggle a flag at the cell
// under the cursor on left click.
// FUNCTION: C2 0x2f902
// FUNCTION: C2WIN 0x004b1951
void flag_mode_action(void)
{
    old_scrolling = scrolling;
    scrolling = 0; stopped_scrolling = 0; illegal_build = 0;

    get_icon_over();

    if (map_mode == 0) {
        if (control_menus(main_menu, 4, show_citymap) != 0) {
            flag_mode = 0;
            update_map = 1;
            setup_map_screen_refresh();
            goto flag_done;
        }
        if (perform_city_strip_action() != 0) {
            redraw_icons = 1;
            update_map = 1;
            setup_map_screen_refresh();
            goto flag_done;
        }
        if (use_city_overmap_to_move() != 0) {
            show_landfill(com_x, com_y);
            update_map = 1;
            setup_map_screen_refresh();
            goto flag_done;
        }
    } else if (map_mode == 1) {
        if (control_menus(main_menu, 4, show_regionmap) != 0) {
            flag_mode = 0;
            update_map = 1;
            setup_map_screen_refresh();
            goto flag_done;
        }
        if (perform_region_strip_action() != 0) {
            redraw_icons = 1;
            update_map = 1;
            setup_map_screen_refresh();
            goto flag_done;
        }
        if (use_region_overmap_to_move() != 0) {
            show_landfill(com_x, com_y);
            update_map = 1;
            setup_map_screen_refresh();
            goto flag_done;
        }
    }

    scroll();

    if (mouse_left_preclick != 0 && pm_over != 0) {
        if (map_mode == 0) {
            if (toggle_city_flag(pm_over_cm_ptr) == 0) {
                put_message(0x66, 0, 0);
            }
        } else if (map_mode == 1) {
            if (toggle_prov_flag(pm_over_cm_ptr) == 0) {
                put_message(0x66, 0, 0);
            }
        }
        update_map = 1;
        setup_map_screen_refresh();
    }

    if (mouse_right_preclick != 0) {
        flag_mode_decay_count = 0; flag_mode = 0;
        do_pos();
        setup_map_screen_refresh();
    }

flag_done:
    if (old_scrolling != scrolling && old_scrolling == 1) {
        stopped_scrolling = 1;
    }
}

// Per-frame dispatcher used during a tactical battle. Updates the hover / drag highlights, kicks
// off select/move/aim actions on left click, and exits to the city map on right click.
// FUNCTION: C2 0x2fa7d
// FUNCTION: C2WIN 0x004b1b8f
void battle_action(void)
{
    old_scrolling = scrolling;
    scrolling = 0; stopped_scrolling = 0; illegal_build = 0;

    if (zoom_in_decay_count != 0) zoom_in_decay_count = zoom_in_decay_count + 1;

    if (control_menus(main_menu, 4, show_battlemap) != 0) goto end_battle_action;
    if (perform_battle_strip_action() != 0) { redraw_icons = 1; update_map = 1; goto end_battle_action; }

    scroll();

    if (pm_over != 0) {

        act_start_pm_ptr = pm_over_cm_ptr;
        act_start_ptr = pm_over_cm_ptr / map_actual_atom;
        act_start_x = act_start_ptr % map_actual_width;
        act_start_y = act_start_ptr / map_actual_width;
        if (pointer_mode == 1) show_move_highlight();
        else if (pointer_mode == 2) show_aim_highlight();
    }
    else {

        battle_drag_on = 0;
    }

    if (mouse_left_preclick != 0 && pm_over != 0) {

        if (pointer_mode == 0 && zoom_level == 1) {

            if ((*(struct battle_cell *)((unsigned char *)battle_map + (pm_over_cm_ptr))).figure != 0) select_a_unit((*(struct battle_cell *)((unsigned char *)battle_map + (pm_over_cm_ptr))).figure, 0);
            else {

                deselect_all_figures();
                battle_drag_on = 1;
                battle_drag_start_x = act_start_x;
                battle_drag_start_y = act_start_y;
            }
        }
        if (pointer_mode == 1) start_move();
        else if (pointer_mode == 2) start_aim();
    }

    if (mouse_left_button != 0 && pm_over != 0) {

        if (battle_drag_on != 0) {

            deselect_all_figures();
            select_drag_figures();
        }
    }

    if (mouse_left_click != 0) {

        battle_drag_on = 0;
    }

    if (mouse_left_preclick != 0) {

        if (battle_setup_count != 0) battle_setup_count = 2;
    }

    if (mouse_right_preclick != 0) {

        pointer_mode = 0;
        redraw_icons = 1;
        if (battle_setup_count != 0) battle_setup_count = 2;
    }
end_battle_action:
    if (old_scrolling != scrolling && old_scrolling == 1) stopped_scrolling = 1;
}

// Edge-of-screen panning. When the mouse is at one of the screen edges, advances `pm_x`/`pm_y` by
// `scroll_amount` (or twice that for the top/bottom rows).
// FUNCTION: C2 0x2fc9b
// FUNCTION: C2WIN 0x004b1df5
void scroll(void)
{
    int saved_pm_x = pm_x;
    int saved_pm_y = pm_y;

    /* Province (large) map at zoom 2 doesn't scroll on edges. */
    if (map_mode == 2 && zoom_level == 2) {
        return;
    }
    /* Pointer mode 5 (cohort tracking) suppresses scrolling. */
    if (pointer_mode == 5) {
        return;
    }

    /* Top edge — scroll up. */
    if (mouse_y <= 0 && pm_y > 0) { pm_y = pm_y - scroll_amount * 2; scrolling = 1; update_map = 1; setup_map_screen_refresh(); }
    /* Bottom edge — scroll down. */
    if (mouse_y >= screen_height && (0xa0 - pm_screen_height) > pm_y) { pm_y = pm_y + scroll_amount * 2; scrolling = 1; update_map = 1; setup_map_screen_refresh(); }
    /* Left edge — scroll left. */
    if (mouse_x <= 0 && pm_x > 0) { pm_x = pm_x - scroll_amount; scrolling = 1; update_map = 1; setup_map_screen_refresh(); }
    /* Right edge — scroll right. */
    if (mouse_x >= screen_width && (0x50 - pm_screen_width) > pm_x)
    { pm_x = pm_x + scroll_amount; scrolling = 1; update_map = 1; setup_map_screen_refresh(); }

    if (scrolling != 0) {
        if (scroll_speed() == 0) {
            pm_x = saved_pm_x;
            pm_y = saved_pm_y;
            scrolling = 0;
        }
    }
}

// On the region map (map_mode == 1) and only while the player is in pointer_mode 2 or 3 (over-army
// or tracking-army), keep the mouse pulled toward the army's screen coordinates.
// FUNCTION: C2 0x2fdf5
// FUNCTION: C2WIN 0x004b1f7d
void mouse_follow_cohort(void)
{
    static int cohort_tick_gate;
    int army_distance;
    int army_screen_x;
    int army_screen_y;

    if (map_mode != 1) { tracking_army = 0; return; }
    if (pointer_mode <= 1) { tracking_army = 0; return; }
    if (pointer_mode >= 4) return;
    if (pm_over == 0) { tracking_army = 0; pointer_mode = 2; return; }

    if (pointer_mode == 2) army_distance = get_nearest_army_to_track(mouse_x, mouse_y);
    else if (pointer_mode == 3) army_distance = get_tracking_army_distance(tracking_army, mouse_x, mouse_y);

    if (army_distance >= 0x18) { tracking_army = 0; pointer_mode = 2; return; }


    pointer_mode = 3;
    if (cohort_tick_gate >= 2) { cohort_tick_gate = 0; return; }
    cohort_tick_gate = cohort_tick_gate + 1;
    army_screen_x = army_list[tracking_army].map_x; if (army_screen_x < mouse_x) { mse_x = (short)(mouse_x - 1); }
    else if (army_screen_x > mouse_x) { mse_x = (short)(mouse_x + 1); }
    else { mse_x = mouse_x; }
    army_screen_y = army_list[tracking_army].map_y; if (army_screen_y < mouse_y) { mse_y = (short)(mouse_y - 1); }
    else if (army_screen_y > mouse_y) { mse_y = (short)(mouse_y + 1); }
    else { mse_y = mouse_y; }
    set_mouse();
}

// Pulls the cursor toward the hovered enemy army while choosing an attack target.
// FUNCTION: C2 0x2ff3c
// FUNCTION: C2WIN 0x004b2194
void mouse_hunt_enemies(void)
{
    static int enemy_tick_gate;
    int enemy_distance;
    int enemy_screen_x;
    int enemy_screen_y;

    if (map_mode != 1) {
        hunting_army = 0;
        return;
    }
    if (pointer_mode <= 5) {
        hunting_army = 0;
        return;
    }
    if (pointer_mode >= 9) {
        hunting_army = 0;
        return;
    }
    if (pm_over == 0) {
        hunting_army = 0;
        return;
    }

    enemy_distance = get_nearest_enemy_to_track(mouse_x, mouse_y);
    if (enemy_distance >= 0x18) {
        hunting_army = 0;
        return;
    }

    pointer_mode = 8;

    if (enemy_tick_gate >= 2) {
        enemy_tick_gate = 0;
        return;
    }
    enemy_tick_gate = enemy_tick_gate + 1;

    enemy_screen_x = army_list[hunting_army].map_x;
    if (enemy_screen_x < mouse_x) {
        mse_x = (short)(mouse_x - 1);
    } else if (enemy_screen_x > mouse_x) {
        mse_x = (short)(mouse_x + 1);
    } else {
        mse_x = mouse_x;
    }
    enemy_screen_y = army_list[hunting_army].map_y;
    if (enemy_screen_y < mouse_y) {
        mse_y = (short)(mouse_y - 1);
    } else if (enemy_screen_y > mouse_y) {
        mse_y = (short)(mouse_y + 1);
    } else {
        mse_y = mouse_y;
    }

    set_mouse();
}

// While drawing an attack/move route across the region map (pointer modes 6..8), trace and
// highlight the elastic path from the source to the cell under the cursor. Switches to
// pointer_mode 7 when a route segment lands on a destination tile, or 6 otherwise.
// FUNCTION: C2 0x30071
// FUNCTION: C2WIN 0x004b2362
void show_latest_route(void)
{
    unsigned char place_state;

    if (pointer_mode < 6 || pointer_mode > 8) {
        return;
    }
    if (pm_over == 0) {
        return;
    }

    pointer_mode = 6;
    get_over_coords();

    place_state = (*(struct region_cell *)((unsigned char *)region_map + (pm_over_cm_ptr))).place_state;
    if (place_state > 0x20 || place_state < 1) {
        return;
    }

    restore_region_from_undo_buffer();
    trace_back_route_elastic();

    if (this_route_number >= 9) {
        pointer_mode = 7;
        (*(struct region_cell *)((unsigned char *)region_map + (pm_over_cm_ptr))).edge_bits |= 0x80;
    } else if (army_list[tracking_army].x == over_x
            && army_list[tracking_army].y == over_y) {
        if (this_route_number > 0) {
            pointer_mode = 7;
        } else {
            pointer_mode = 6;
        }
    } else {
        unsigned int destination_flag =
            (unsigned char)((*(struct region_cell *)((unsigned char *)region_map + (pm_over_cm_ptr))).edge_bits & 0x80);
        if (destination_flag != 0) {
            pointer_mode = 7;
        } else {
            pointer_mode = 6;
            (*(struct region_cell *)((unsigned char *)region_map + (pm_over_cm_ptr))).edge_bits |= 0x80;
        }
    }

    setup_refresh_area(mouse_x - 0x40, mouse_y - 0x40, 9, 9, 2);
}

// Snapshot the current cursor position and current denarii at the instant the player presses LMB
// to start a build, then dispatch to the per-tool "elastic preview" helper for road/wall/aqueduct.
// FUNCTION: C2 0x30173
// FUNCTION: C2WIN 0x004b24d7
void prebuild_city_item(void)
{
    act_start_pm_ptr = pm_over_cm_ptr;
    act_start_ptr = pm_over_cm_ptr / map_actual_atom;
    act_start_x = act_start_ptr % map_actual_width;
    act_start_y = act_start_ptr / map_actual_width;

    unflag_all_cm(3, 0xdf);
    starting_denarii = denarii;
    hot_key_out_off_build = 0;

    if (placing_type == 2) {
        get_road_elastic();
    } else if (placing_type == 3) {
        get_wall_elastic();
    } else if (placing_type == 4) {
        get_aquaduct_elastic();
        return;
    }
}

// Per-frame "build the placing_type item" handler. Called from `action()` when LMB is held and the
// cursor is over the city map.
// FUNCTION: C2 0x3020c
// FUNCTION: C2WIN 0x004b2585
void build_city_item(void)
{
    int second_gfx_idx;
    unsigned int house_gfx_idx;
    int first_gfx_idx;
    int second_y_offset;
    unsigned int building_edge_bits;
    unsigned int fountain_gfx_idx;
    int first_base_kind;
    int second_base_kind;
    int placement_ok;
    unsigned int building_gfx_idx;
    unsigned int building_shape;
    unsigned int base_kind;
    int has_cover;
    int second_x_offset;

    illegal_build = 2;
    CM_CELL(pm_over_cm_ptr).edge_bits |= 1;

    if (slave_requirements[0].current < slave_requirements[0].max) {
        if (warned_of_not_build != 0) return;
        warned_of_not_build = 1;
        put_message(0x65, 0, 0);
        return;
    }

    if (placing_type != 0) denarii = starting_denarii;
    particles_cleared = particles_built = 0;

    if (placing_type == 0x2) {  /* Road */
        restore_city_from_undo_buffer();
        if (hot_key_out_off_build == 0) build_road_from_elastic();
        if (pm_over != 0 && pm_over != old_pm_over) setup_map_screen_refresh();
    }
    if (placing_type == 0x3) {  /* Wall */
        restore_city_from_undo_buffer();
        if (hot_key_out_off_build == 0) build_wall_from_elastic();
        if (pm_over != 0 && pm_over != old_pm_over) setup_map_screen_refresh();
    }
    if (placing_type == 0x4) {  /* Aqueduct */
        restore_city_from_undo_buffer();
        if (hot_key_out_off_build == 0) build_aquaduct_from_elastic();
        if (pm_over != 0 && pm_over != old_pm_over) setup_map_screen_refresh();
        evolve_row = 0; evolve_water_supply_baths_industry(0x50);
    }

    if (placing_type == 0x1) {  /* Clear / bulldoze (with aqueduct-removal confirm) */
        restore_city_from_undo_buffer();
        /* Aqueduct-removal confirmation: only if cursor stayed put,
         * the cell has the +1 (terrain & 0x20) and (terrain & 0x40)
         * markers, and the player confirms. */
        if (act_start_x == over_x
         && act_start_y == over_y) {
            if ((CM_CELL(pm_over_cm_ptr).terrain & 0x20) != 0) {
                if ((CM_CELL(pm_over_cm_ptr).terrain & 0x40) != 0) {
                    confirm(10, 0xa0, 0xa0);
                    if (decision == 0) {
                        CM_CELL(pm_over_cm_ptr).terrain &= 0xdf;
                        base_kind = CM_CELL(pm_over_cm_ptr).base_kind;
                        if (base_kind == 0xd5) { CM_CELL(pm_over_cm_ptr).base_kind = 0xcf; CM_CELL(pm_over_cm_ptr).extra_edge = 0x79; }
                        else { CM_CELL(pm_over_cm_ptr).base_kind = 0xd0; CM_CELL(pm_over_cm_ptr).extra_edge = 0x76; }
                        aquaduct_ramifications(over_x, over_y);
                        setup_map_screen_refresh();
                        goto after_clear;
                    }
                }
            }
        }
        if (hot_key_out_off_build == 0) clear_an_area(act_start_x, act_start_y, over_x, over_y);
        if (pm_over != 0 && pm_over != old_pm_over) setup_map_screen_refresh();
        if ((cycle_count & 7) == 0) setup_map_screen_refresh();
    }
    if (placing_type == 0x6) {  /* Gardens */
        restore_city_from_undo_buffer();
        if (hot_key_out_off_build == 0) garden_an_area(act_start_x, act_start_y, over_x, over_y);
        if (pm_over != 0 && pm_over != old_pm_over) setup_map_screen_refresh();
    }
    if (placing_type == 0x7) {  /* Plaza */
        restore_city_from_undo_buffer();
        if (hot_key_out_off_build == 0) plaza_an_area(act_start_x, act_start_y, over_x, over_y);
        if (pm_over != 0 && pm_over != old_pm_over) setup_map_screen_refresh();
    }

    /* Houses use the placement type to select their footprint and graphics. */
    if (placing_type >= 0x82 && placing_type <= 0xa1) {
        restore_city_from_undo_buffer();
        base_kind = placing_type;
        house_gfx_idx = house_gfxdat[base_kind * 4 - 0x208];
        building_shape = house_gfxdat[base_kind * 4 - 0x207];
        building_edge_bits = house_gfxdat[base_kind * 4 - 0x206];
        if (hot_key_out_off_build == 0) {
            if (building_shape == 3) put_x3_area(over_x, over_y, base_kind, building_edge_bits, house_gfx_idx);
            else if (building_shape == 2) put_x2_area(over_x, over_y, base_kind, building_edge_bits, house_gfx_idx);
            else build_an_area(act_start_x, act_start_y, over_x, over_y, base_kind, building_edge_bits, house_gfx_idx);
        }
    }

    /* Forums use the placement type to select their footprint and graphics. */
    if (placing_type >= 0xae && placing_type <= 0xb9) {
        restore_city_from_undo_buffer();
        base_kind = placing_type;
        building_gfx_idx = forum_gfxdat[base_kind * 4 - 0x2b8];
        building_shape = forum_gfxdat[base_kind * 4 - 0x2b7];
        building_edge_bits = forum_gfxdat[base_kind * 4 - 0x2b6];
        if (hot_key_out_off_build == 0) {
            if (building_shape == 2) put_x2_area(over_x, over_y, base_kind, building_edge_bits, building_gfx_idx);
            else if (building_shape == 3) put_x3_area(over_x, over_y, base_kind, building_edge_bits, building_gfx_idx);
            else if (building_shape == 4) put_x4_area(over_x, over_y, base_kind, building_edge_bits, building_gfx_idx);
        }
    }

    if (placing_type == 0xa) {  /* Baths */
        restore_city_from_undo_buffer();
        has_cover = affected_by_cover1(CM_CELL(pm_over_cm_ptr).b, 2, 4);
        if (has_cover != 0) building_gfx_idx = 0x20;
        else building_gfx_idx = 99;
        if (hot_key_out_off_build == 0) {
            put_x2_area(over_x, over_y, 0xdf, 8, building_gfx_idx);
            CM_CELL(start_sptr).building = 0x0f;
        }
    }
    if (placing_type == 0xb) {  /* Hospital */
        restore_city_from_undo_buffer();
        if (hot_key_out_off_build == 0) put_x3_area(over_x, over_y, 0xfb, 8, 0x56);
    }
    if (placing_type == 0xe) {  /* Prefecture */
        restore_city_from_undo_buffer();
        if (hot_key_out_off_build == 0) {
            put_x1_area(over_x, over_y, 0xe3, 0, 0x50);
            CM_CELL(start_sptr).edge_bits |= 0x80;
        }
    }
    if (placing_type == 0xd) {  /* Barracks */
        restore_city_from_undo_buffer();
        if (hot_key_out_off_build == 0) put_x3_area(over_x, over_y, 0xe4, 0, 0x51);
    }
    if (placing_type == 0x10) {
        restore_city_from_undo_buffer();
        if (hot_key_out_off_build == 0) {
            if (put_x3_area(over_x, over_y, 0xfa, 0xc, 0x3e) != 0) {
                CM_CELL(start_sptr).edge_bits  |= 0x80;
                CM_CELL(start_sptr).business &= 0xf0;
                CM_CELL(start_sptr).business |= (unsigned char)business_build_type;
                CM_CELL(start_sptr + CITY_CELL_BYTES).edge_bits |= 0x80;
            }
        }
    }
    if (placing_type == 0xf) {  /* Market */
        restore_city_from_undo_buffer();
        if (hot_key_out_off_build == 0) put_x2_area(over_x, over_y, 0xfc, 8, 0x30);
    }
    if (placing_type == 0x11) {  /* Grammaticus (school) */
        restore_city_from_undo_buffer();
        if (hot_key_out_off_build == 0) put_x2_area(over_x, over_y, 0xf3, 8, 0x40);
    }
    if (placing_type == 0x12) {  /* Rhetor (academy) */
        restore_city_from_undo_buffer();
        if (hot_key_out_off_build == 0) put_x3_area(over_x, over_y, 0xf4, 8, 0x44);
    }
    if (placing_type == 0x13) {  /* Library */
        restore_city_from_undo_buffer();
        if (hot_key_out_off_build == 0) put_x3_area(over_x, over_y, 0xf5, 8, 0x4d);
    }
    if (placing_type == 0x14) {  /* Small temple */
        restore_city_from_undo_buffer();
        if (hot_key_out_off_build == 0) put_x1_area(over_x, over_y, 0xa2, 0, 0x3c);
    }
    if (placing_type == 0x15) {  /* Medium temple */
        restore_city_from_undo_buffer();
        if (hot_key_out_off_build == 0) put_x2_area(over_x, over_y, 0xa6, 0, 0x40);
    }
    if (placing_type == 0x16) {  /* Large temple */
        restore_city_from_undo_buffer();
        if (hot_key_out_off_build == 0) put_x3_area(over_x, over_y, 0xaa, 0xc, 0);
    }
    if (placing_type == 0x17) {  /* Theatre */
        restore_city_from_undo_buffer();
        if (hot_key_out_off_build == 0) put_x2_area(over_x, over_y, 0xe5, 0xc, 0x24);
    }
    if (placing_type == 0x18) {  /* Odeum */
        restore_city_from_undo_buffer();
        if (hot_key_out_off_build == 0) put_x2_area(over_x, over_y, 0xe6, 0xc, 0x28);
    }
    if (placing_type == 0x19) {  /* Arena */
        restore_city_from_undo_buffer();
        if (hot_key_out_off_build == 0) {
            put_x3_area(over_x, over_y, 0xe7, 0xc, 0x2c);
            CM_CELL(start_sptr).edge_bits |= 0x80;
        }
    }
    if (placing_type == 0x1a) {  /* Colosseum */
        restore_city_from_undo_buffer();
        if (hot_key_out_off_build == 0) {
            put_x3_area(over_x, over_y, 0xe8, 0xc, 0x35);
            CM_CELL(start_sptr).edge_bits |= 0x80;
        }
    }

    if (placing_type == 0x1b) {
        /* Circus: two 3x3 tiles, orientation chosen from map_direction. */
        restore_city_from_undo_buffer();
        if (map_direction == 0) { first_base_kind = 0xe9; second_base_kind = 0xea; first_gfx_idx = 0; second_gfx_idx = 9; second_x_offset = 0; second_y_offset = 3; }
        else if (map_direction == 4) { first_base_kind = 0xe9; second_base_kind = 0xea; first_gfx_idx = 9; second_gfx_idx = 0; second_x_offset = 0; second_y_offset = -3; }
        else if (map_direction == 2) { first_base_kind = 0xeb; second_base_kind = 0xec; first_gfx_idx = 0x3b; second_gfx_idx = 0x32; second_x_offset = -3; second_y_offset = 0; }
        else if (map_direction == 6) { first_base_kind = 0xeb; second_base_kind = 0xec; first_gfx_idx = 0x32; second_gfx_idx = 0x3b; second_x_offset = 3; second_y_offset = 0; }
        if (hot_key_out_off_build == 0) {
            placement_ok = 1;
            if (put_x3_area(over_x, over_y, first_base_kind, 0x14, first_gfx_idx) == 0) placement_ok = 0;
            if (put_x3_area(over_x + second_x_offset, over_y + second_y_offset, second_base_kind, 0x14, second_gfx_idx) == 0) placement_ok = 0;
            if (placement_ok == 0) { restore_city_from_undo_buffer(); particles_built = 0; }
            else particles_built = 1;
            set_map_ref(over_x, over_y, 3);
            set_map_ref(over_x + second_x_offset, over_y + second_y_offset, 3);
        }
    }

    if (placing_type == 0x1c) {
        /* Circus Maximus: two 4x4 tiles, orientation from map_direction. */
        restore_city_from_undo_buffer();
        if (map_direction == 0) { first_base_kind = 0xed; second_base_kind = 0xee; first_gfx_idx = 0x12; second_gfx_idx = 0x22; second_x_offset = 0; second_y_offset = 4; }
        else if (map_direction == 4) { first_base_kind = 0xed; second_base_kind = 0xee; first_gfx_idx = 0x22; second_gfx_idx = 0x12; second_x_offset = 0; second_y_offset = -4; }
        else if (map_direction == 2) { first_base_kind = 0xef; second_base_kind = 0xf0; first_gfx_idx = 0x54; second_gfx_idx = 0x44; second_x_offset = -4; second_y_offset = 0; }
        else if (map_direction == 6) { first_base_kind = 0xef; second_base_kind = 0xf0; first_gfx_idx = 0x44; second_gfx_idx = 0x54; second_x_offset = 4; second_y_offset = 0; }
        if (hot_key_out_off_build == 0) {
            placement_ok = 1;
            if (put_x4_area(over_x, over_y, first_base_kind, 0x14, first_gfx_idx) == 0) placement_ok = 0;
            if (put_x4_area(over_x + second_x_offset, over_y + second_y_offset, second_base_kind, 0x14, second_gfx_idx) == 0) placement_ok = 0;
            if (placement_ok == 0) { restore_city_from_undo_buffer(); particles_built = 0; }
            else particles_built = 1;
            set_map_ref(over_x, over_y, 4);
            set_map_ref(over_x + second_x_offset, over_y + second_y_offset, 4);
        }
    }

    if (placing_type == 0xbf) {  /* Tower */
        restore_city_from_undo_buffer();
        base_kind = placing_type;
        if (hot_key_out_off_build == 0) {
            CM_CELL(pm_over_cm_ptr).terrain &= 0xfd;
            if (put_x1_area(over_x, over_y, base_kind, 8, 0x94) == 0) {
                restore_city_from_undo_buffer();
            }
            if (wall_ramifications(over_x, over_y) == 0) {
                restore_city_from_undo_buffer();
            }
        }
    }

    if (placing_type == 0xbe) {  /* Reservoir */
        restore_city_from_undo_buffer();
        base_kind = placing_type;
        if (hot_key_out_off_build == 0) {
            CM_CELL(pm_over_cm_ptr).terrain &= 0xbf;
            if (put_x1_area(over_x, over_y, base_kind, 0, 0x5a) == 0) {
                restore_city_from_undo_buffer();
            }
            if (aquaduct_ramifications(over_x, over_y) == 0) {
                restore_city_from_undo_buffer();
            }
        }
        evolve_row = 0; evolve_water_supply_baths_industry(0x50);
    }

    if (placing_type == 0x8) {  /* Well */
        restore_city_from_undo_buffer();
        if (hot_key_out_off_build == 0) {
            if (put_x1_area(over_x, over_y, 0xd7, 8, 0x10) == 0) {
                restore_city_from_undo_buffer();
            }
        }
    }

    if (placing_type == 0xc) {  /* Fountain */
        restore_city_from_undo_buffer();
        base_kind = 0xdb;
        fountain_gfx_idx = fountain_gfxdat[base_kind - 0xdb];
        if ((CM_CELL(pm_over_cm_ptr).education & 4) != 0) fountain_gfx_idx++;
        if (hot_key_out_off_build == 0) {
            if (put_x1_area(over_x, over_y, base_kind, 8, fountain_gfx_idx) == 0) {
                restore_city_from_undo_buffer();
            } else { CM_CELL(pm_over_cm_ptr).building = 0x0f; }
        }
    }

after_clear:
    total_build_cost = particles_cleared * city_costs[1];
    total_build_cost = total_build_cost + particles_built * placing_cost;
    denarii = denarii - total_build_cost;
    update_map = 2;
    return;
}

// Captures the starting region cell and funds, then prepares the active elastic build preview.
// FUNCTION: C2 0x30ebf
// FUNCTION: C2WIN 0x004b3568
void prebuild_region_item(void)
{
    act_start_pm_ptr = pm_over_cm_ptr;
    act_start_ptr = pm_over_cm_ptr / map_actual_atom;
    act_start_x = act_start_ptr % map_actual_width;
    act_start_y = act_start_ptr / map_actual_width;

    hot_key_out_off_build = 0;
    starting_denarii = denarii;

    if (reg_placing_type == 0x1e) {
        get_reg_road_elastic();
    } else if (reg_placing_type == 0x1f) {
        get_reg_wall_elastic();
        return;
    }
}

// Builds or clears the selected region-map structure and charges its resulting cost.
// FUNCTION: C2 0x30f3a
// FUNCTION: C2WIN 0x004b35f0
void build_region_item(void)
{


    (*(struct region_cell *)((unsigned char *)region_map + (pm_over_cm_ptr))).edge_bits |= 1;
    illegal_build = 2;

    if (slave_requirements[0].current < slave_requirements[0].max) {
        if (warned_of_not_build != 0) {
            return;
        }
        warned_of_not_build = 1;
        put_message(0x65, 0, 0);
        return;
    }

    denarii = starting_denarii;
    particles_built = 0;
    particles_cleared = 0;
    industry_build_on = 0;

    if (reg_placing_type == 0x1e) {
        /* Region road. */
        restore_region_from_undo_buffer();
        if (hot_key_out_off_build == 0) {
            build_reg_road_from_elastic();
        }
        if (pm_over != 0 && old_pm_over != pm_over) {
            setup_map_screen_refresh();
        }
    }

    if (reg_placing_type == 0x1f) {
        /* Region wall. */
        restore_region_from_undo_buffer();
        if (at_edge_of_map(over_x, over_y) != 0) {
            illegal_build = 1;
        } else if (at_edge_of_map(act_start_x, act_start_y) != 0) {
            illegal_build = 1;
        } else if (hot_key_out_off_build == 0) {
            build_reg_wall_from_elastic();
        }
        if (pm_over != 0 && old_pm_over != pm_over) {
            setup_map_screen_refresh();
        }
    }

    if (reg_placing_type == 0x21) {
        /* Clear region area. */
        restore_region_from_undo_buffer();
        if (hot_key_out_off_build == 0) {
            clear_a_reg_area(act_start_x, act_start_y,
                             over_x, over_y, 0);
        }
        if (pm_over != 0 && old_pm_over != pm_over) {
            setup_map_screen_refresh();
        }
    }

    if (reg_placing_type == 0x23) {
        /* Workhouse */
        restore_region_from_undo_buffer();
        if (at_edge_of_map(over_x, over_y) != 0) {
            illegal_build = 1;
        } else if (hot_key_out_off_build == 0) {
            if (get_reg_industries_in_radius(over_x, over_y) == 0) {
                restore_region_from_undo_buffer();
                illegal_build = 1;
            } else if (put_reg_x1_area(over_x, over_y, 0xd3, 0,
                                        0x3c, 0) == 0) {
                restore_region_from_undo_buffer();
                illegal_build = 1;
            }
        }
    }

    if (reg_placing_type == 0x24) {
        /* Warehouse */
        restore_region_from_undo_buffer();
        if (at_edge_of_map(over_x, over_y) != 0) {
            illegal_build = 1;
        } else if (hot_key_out_off_build == 0) {
            if (get_reg_industries_in_radius(over_x, over_y) == 0) {
                restore_region_from_undo_buffer();
                illegal_build = 1;
            } else if (put_reg_x1_area(over_x, over_y, 0xd4, 0,
                                        0x0b, 0) == 0) {
                restore_region_from_undo_buffer();
                illegal_build = 1;
            }
        }
    }

    if (reg_placing_type == 0x2a) {
        /* Bridge */
        restore_region_from_undo_buffer();
        if (at_edge_of_map(over_x, over_y) != 0) {
            illegal_build = 1;
        } else if (hot_key_out_off_build == 0
                && put_reg_x2_area(over_x, over_y, 0xd5, 0,
                                    0x46, 0) == 0) {
            restore_region_from_undo_buffer();
            illegal_build = 1;
        }
    }

    if (reg_placing_type == 0x25) {
        /* Farm */
        industry_build_on = 1;
        restore_region_from_undo_buffer();
        check_region_map_for_farm_square(over_x, over_y, 0x80);
        if (at_edge_of_map(over_x, over_y) != 0) {
            illegal_build = 1;
        } else if (hot_key_out_off_build == 0) {
            if (put_reg_x2_area(over_x, over_y, 0xdc, 8,
                                0x30, 1) == 0) {
                illegal_build = 1;
                restore_region_from_undo_buffer();
                industry_build_ok = 0;
                industry_build_on = 0;
            }
        }
    }

    if (reg_placing_type == 0x26) {
        /* Mine */
        industry_build_on = 1;
        restore_region_from_undo_buffer();
        check_region_map_for_farm_square(over_x, over_y, 0x40);
        if (at_edge_of_map(over_x, over_y) != 0) {
            illegal_build = 1;
        } else if (hot_key_out_off_build == 0) {
            if (put_reg_x2_area(over_x, over_y, 0xe0, 8,
                                0x40, 1) == 0) {
                illegal_build = 1;
                restore_region_from_undo_buffer();
                industry_build_ok = 0;
                industry_build_on = 0;
            }
        }
    }

    if (reg_placing_type == 0x27) {
        /* Quarry */
        industry_build_on = 1;
        restore_region_from_undo_buffer();
        check_region_map_for_farm_square(over_x, over_y, 0x40);
        if (at_edge_of_map(over_x, over_y) != 0) {
            illegal_build = 1;
        } else if (hot_key_out_off_build == 0) {
            if (put_reg_x2_area(over_x, over_y, 0xe4, 8,
                                0x20, 1) == 0) {
                illegal_build = 1;
                restore_region_from_undo_buffer();
                industry_build_ok = 0;
                industry_build_on = 0;
            }
        }
    }

    if (reg_placing_type == 0x29) {
        /* Logging camp */
        industry_build_on = 1;
        restore_region_from_undo_buffer();
        if (at_edge_of_map(over_x, over_y) != 0) {
            illegal_build = 1;
        } else if (hot_key_out_off_build == 0) {
            if (put_reg_x2_area(over_x, over_y, 0xe8, 8,
                                0x60, 1) == 0) {
                illegal_build = 1;
                restore_region_from_undo_buffer();
                industry_build_ok = 0;
                industry_build_on = 0;
            }
        }
    }

    if (reg_placing_type == 0x28) {
        /* Port */
        industry_build_on = 1;
        restore_region_from_undo_buffer();
        check_region_map_for_port_square(over_x, over_y);
        if (at_edge_of_map(over_x, over_y) != 0) {
            illegal_build = 1;
        } else if (hot_key_out_off_build == 0) {
            if (put_reg_x2_area(over_x, over_y, 0xec, 8,
                                0x50, 2) == 0) {
                illegal_build = 1;
                restore_region_from_undo_buffer();
                industry_build_ok = 0;
                industry_build_on = 0;
            } else if (industry_build_ok == 0) {
                flag_rm_area(over_x, over_y, 2, 8);
                adjust_regions_coastline(start_x_pos - 1,
                                         start_y_pos - 1, 4, 4);
            }
        }
    }

    if (reg_placing_type == 0x22) {
        /* Fortress */
        restore_region_from_undo_buffer();
        if (at_edge_of_map(over_x, over_y) != 0) {
            illegal_build = 1;
        } else if (hot_key_out_off_build == 0) {
            get_cohorts_in_action();
            (*(struct region_cell *)((unsigned char *)region_map + (pm_over_cm_ptr))).terrain &= 0xfd;
            if (put_reg_x1_area(over_x, over_y, 0xd2, 0, 0x46, 1) == 0) {
                restore_region_from_undo_buffer();
                illegal_build = 1;
            } else if (reg_wall_ramifications(over_x, over_y) == 0) {
                restore_region_from_undo_buffer();
                illegal_build = 1;
            } else if (no_of_cohorts_in_action >= 0xa) {
                restore_region_from_undo_buffer();
                if (warned_of_not_build == 0) {
                    warned_of_not_build = 1;
                    put_message(0x5a, 0, 0);
                }
            } else if (create_army(1, over_x, over_y, 0) != 0) {
                int new_army_idx = created_army_no;
                army_list[new_army_idx].state_idx = 1;
                army_list[new_army_idx].saved_state_idx = 1;
                army_list[new_army_idx].exists = 2;
                army_list[new_army_idx].cohort_id = next_cohort_free;
                army_list[new_army_idx].departure_year = year;
                army_list[new_army_idx].morale_timer = 2;
            }
        } else {
            restore_region_from_undo_buffer();
        }
    }

    total_build_cost = particles_cleared * region_costs[1];
    total_build_cost = total_build_cost + particles_built * placing_cost;
    denarii -= total_build_cost;
}

// Stores the selected farm type in the origin cell of the current 2x2 farm.
// FUNCTION: C2 0x31645
// FUNCTION: C2WIN 0x004b3fd6
void act_select_farm(void)
{
    int origin_rm_ptr = get_region_2x2_start(pm_over_cm_ptr);
    (*(struct region_cell *)((unsigned char *)region_map + (origin_rm_ptr))).occupant &= 0x0f;
    para1 <<= 4;
    (*(struct region_cell *)((unsigned char *)region_map + (origin_rm_ptr))).occupant |= (unsigned char)para1;
}

// Returns the origin-cell pointer for the 2x2 region building containing `rm_ptr`.
// FUNCTION: C2 0x3166c
// FUNCTION: C2WIN 0x004b402d
int get_region_2x2_start(int rm_ptr)
{
    int row_offset;
    int col_offset;
    int footprint_width;

    if ((*(struct region_cell *)((unsigned char *)region_map + (rm_ptr))).base_kind == 0xd4) {
        row_offset = 0;
        col_offset = 0;
    } else {
        row_offset = (*(struct region_cell *)((unsigned char *)region_map + (rm_ptr))).occupant & 3;
        col_offset = row_offset;
    }

    footprint_width = 2;
    col_offset = col_offset % footprint_width;
    row_offset = row_offset / footprint_width;
    rm_ptr = rm_ptr - col_offset * 8;
    rm_ptr = rm_ptr - row_offset * 480;
    return rm_ptr;
}

// Determine which icon (if any) is under the mouse pointer.
// FUNCTION: C2 0x316cd
// FUNCTION: C2WIN 0x004b40c5
void get_icon_over(void)
{
    int icon_bottom;
    int icon_idx;
    short icon_x;
    short icon_y;
    short icon_width;
    short icon_height;

    last_icon_over = 0;

    if (mouse_y < 0x18) {
        last_icon_over = 1;
        return;
    }

    if (mouse_x >= com_x && (com_x + com_w) > mouse_x) {
        icon_bottom = com_y + com_h;
        if (map_mode == 0) {
            if (com_y - 0x18 <= mouse_y && icon_bottom > mouse_y) {
                last_icon_over = 2;
                return;
            }
        } else {
            if (mouse_y >= com_y && mouse_y < icon_bottom) {
                last_icon_over = 2;
                return;
            }
        }
    }

    if (mouse_x < 0x1e0) {
        return;
    }

    if (map_mode == 0) {
        for (icon_idx = 4; icon_idx < 0x1c; icon_idx++) {
            if (tutorial_mode == 0 || city_icon_allowed(icon_idx - 4) != 0) {
                icon_width = int_city_header[icon_idx * 8 + 4];
                icon_height = int_city_header[icon_idx * 8 + 5];
                icon_x = int_city_header[icon_idx * 8 + 8] + 0xee;
                icon_y = int_city_header[icon_idx * 8 + 9];
                if (mouse_in_area((unsigned short)icon_x, (unsigned short)icon_y,
                                  (unsigned short)icon_width, (unsigned short)icon_height) != 0) {
                    last_icon_over = icon_idx;
                    return;
                }
            }
        }
    } else {
        for (icon_idx = 4; icon_idx < 0x17; icon_idx++) {
            if (tutorial_mode == 0 || region_icon_allowed(icon_idx - 4) != 0) {
                icon_width = int_region_header[icon_idx * 8 + 4];
                icon_height = int_region_header[icon_idx * 8 + 5];
                icon_x = int_region_header[icon_idx * 8 + 8] + 0xee;
                icon_y = int_region_header[icon_idx * 8 + 9];
                if (mouse_in_area((unsigned short)icon_x, (unsigned short)icon_y,
                                  (unsigned short)icon_width, (unsigned short)icon_height) != 0) {
                    last_icon_over = icon_idx;
                    return;
                }
            }
        }
    }
}

// "Is the cursor over icon `icon_idx`?". Returns 1 if the mouse is in the icon's box (or if `icon_idx==2`,
// which always returns 1 — the command-strip area is special-cased by the caller).
// FUNCTION: C2 0x31850
// FUNCTION: C2WIN 0x004b437f
int is_icon_over(int idx)
{
    short xpos;
    short ypos;
    short w;
    short h;

    if (mouse_x < 0x1e0) {
        return 0;
    }

    if (map_mode == 0) {
        w = int_city_header[idx * 8 + 4];
        h = int_city_header[idx * 8 + 5];
        xpos = int_city_header[idx * 8 + 8] + 0xee;
        ypos = int_city_header[idx * 8 + 9];
    } else {
        w = int_region_header[idx * 8 + 4];
        h = int_region_header[idx * 8 + 5];
        xpos = int_region_header[idx * 8 + 8] + 0xee;
        ypos = int_region_header[idx * 8 + 9];
    }

    if (mouse_in_area((unsigned short)xpos, (unsigned short)ypos,
                      (unsigned short)w, (unsigned short)h) != 0) {
        return 1;
    }
    if (idx == 2) {
        return 1;
    }
    return 0;
}

// On a left-click while the cursor is on the overview-map strip, either pop the legend /
// select-map dialogue or — if the click is on the map proper — re-centre the city view on that
// point.
// FUNCTION: C2 0x318ed
// FUNCTION: C2WIN 0x004b44b1
int use_city_overmap_to_move(void)
{
    int map_x;
    int map_y;
    int target_cm_ptr;

    if (mouse_left_preclick == 0) {
        return 0;
    }
    if (last_icon_over != 2) {
        return 0;
    }

    if (mouse_y < com_y && (com_y - 0x18) <= mouse_y) {
        if (mouse_x >= 0x25c) {
            act_show_ov_legend();
        } else {
            act_select_ov_map();
        }
        return 0;
    }

    map_x = (mouse_x - com_x) / 2;
    map_y = (mouse_y - com_y) / 4 * 2;
    target_cm_ptr = (map_y * map_actual_width + map_x) * map_actual_atom;
    if (jump_to_citymap_ptr(target_cm_ptr) != 0) {
        return 1;
    }
    return 0;
}

// Converts a click on the region overview map into a region cell and recentres the view there.
// FUNCTION: C2 0x31997
// FUNCTION: C2WIN 0x004b459d
int use_region_overmap_to_move(void)
{
    int map_x;
    int map_y;
    int target_rm_ptr;

    if (mouse_left_preclick == 0) {
        return 0;
    }
    if (last_icon_over != 2) {
        return 0;
    }

    map_x = (mouse_x - com_x) / 2;
    map_y = (mouse_y - com_y) / 4 * 2;
    target_rm_ptr = (map_y * map_actual_width + map_x) * map_actual_atom;
    if (jump_to_regionmap_ptr(target_rm_ptr) != 0) {
        return 1;
    }
    return 0;
}

// Re-centre the city view on the cell whose pseudo_map[] entry matches `target_cm_ptr`. If we're
// currently on the region map (map_mode==1), first restore the saved city rotation/zoom and switch
// back to the city map (map_mode=0).
// FUNCTION: C2 0x31a0a
// FUNCTION: C2WIN 0x004b463e
int jump_to_citymap_ptr(int target_cm_ptr)
{
    int row;
    int col;
    int map_switched = 0;

    if (map_mode != 0) {
        prov_rotation = map_direction;
        prov_zoom_level = zoom_level;
        map_direction = city_rotation;
        zoom_level = city_zoom_level;
        map_mode = 0;
        act_correct_map();
        map_switched = 1;
    }

    /* Linear search of pseudo_map for the target cell pointer. */
    for (row = 0; row < 0xa1; row++) {
        for (col = 0; col < 0x51; col++) {
            if (pseudo_map[row][col] == target_cm_ptr) {
                goto found;
            }
        }
    }
    return 0;
found:
    pm_x = col;
    pm_y = row & 0xfffe;
    if (zoom_level == 0) {
        pm_x += -4;
        pm_y += -0xc;
    } else if (zoom_level == 1) {
        pm_x += -8;
        pm_y += -0x1e;
    } else if (zoom_level == 2) {
        pm_x += -0x14;
        pm_y += -0x46;
    }
    pm_limits();
    scrolling = 1;
    update_map = 1;
    if (map_switched) {
        return 1;
    }
    return 2;
}

// Switches to the region map if needed and recentres it on `target_rm_ptr`.
// FUNCTION: C2 0x31b1b
// FUNCTION: C2WIN 0x004b47ba
int jump_to_regionmap_ptr(int target_rm_ptr)
{
    int col;
    int row;
    int map_switched = 0;

    if (map_mode != 1) {
        city_rotation = map_direction;
        city_zoom_level = zoom_level;
        map_direction = prov_rotation;
        zoom_level = prov_zoom_level;
        map_mode = 1;
        act_correct_map();
        map_switched = 1;
    }

    for (row = 0; row < 0xa1; row++) {
        for (col = 0; col < 0x51; col++) {
            if (pseudo_map[row][col] == target_rm_ptr) {
                goto found;
            }
        }
    }
    return 0;
found:
    pm_x = col;
    pm_y = row & 0xfffe;
    if (zoom_level == 0) {
        pm_x += -4;
        pm_y += -0xc;
    } else if (zoom_level == 1) {
        pm_x += -8;
        pm_y += -0x1e;
    } else if (zoom_level == 2) {
        pm_x += -0x14;
        pm_y += -0x46;
    }
    pm_limits();
    scrolling = 1;
    update_map = 1;
    if (map_switched) {
        return 1;
    }
    return 2;
}

// Dispatches a city-screen command-strip click to the selected icon action.
// FUNCTION: C2 0x31bd9
// FUNCTION: C2WIN 0x004b4937
int perform_city_strip_action(void)
{
    int cleared_selection;

    if (mouse_left_preclick == 0) {
        return 0;
    }
    if (last_icon_over < 4) {
        return 0;
    }

    cleared_selection = 0;
    selected_icon_no   = cleared_selection;
    selected_icon_text = cleared_selection;
    icon_strip_toggle  = 0x1f;

    city_actions[last_icon_over - 4]();

    if (last_icon_over >= 0xe && last_icon_over != 0x12) {
        last_icon_used = last_icon_over;
        update_icon    = last_icon_over;
    }
    return 1;
}

// Dispatches a region-screen command-strip click to the selected icon action.
// FUNCTION: C2 0x31c3c
// FUNCTION: C2WIN 0x004b49d3
int perform_region_strip_action(void)
{
    int cleared_selection;

    if (mouse_left_preclick == 0) {
        return 0;
    }
    if (last_icon_over < 4) {
        return 0;
    }

    cleared_selection = 0;
    selected_icon_no   = cleared_selection;
    selected_icon_text = cleared_selection;
    icon_strip_toggle  = 0x1f;

    region_actions[last_icon_over - 4]();

    if (last_icon_over >= 0xe && last_icon_over != 0x12) {
        last_icon_used = last_icon_over;
        update_icon    = last_icon_over;
    }
    return 1;
}

// No-op slot used as a placeholder in the icon-action dispatch tables.
// FUNCTION: C2 0x31c9e
// FUNCTION: C2WIN 0x004b4cab
void act_null(void)
{
}

// Finds the battle-screen icon under the cursor and dispatches its action on a left click.
// FUNCTION: C2 0x31c9f
// FUNCTION: C2WIN 0x004b4a6f REORDERED
int perform_battle_strip_action(void)
{
    int icon_idx;
    short icon_x;
    short icon_y;
    short icon_width;
    short icon_height;

    last_icon_over = 0;
    if (mouse_y < 0x168) {
        return 0;
    }

    for (icon_idx = 4; icon_idx < 0x15; icon_idx++) {
        icon_width = int_battle_header[icon_idx * 8 + 4];
        icon_height = int_battle_header[icon_idx * 8 + 5];
        icon_x = int_battle_header[icon_idx * 8 + 8];
        icon_y = int_battle_header[icon_idx * 8 + 9] + 0xc8;
        if (mouse_in_area((unsigned short)icon_x, (unsigned short)icon_y,
                          (unsigned short)icon_width, (unsigned short)icon_height) != 0) {
            last_icon_over = icon_idx;
            if (mouse_left_preclick == 0) {
                return 0;
            }
            battle_actions[icon_idx - 4]();
            update_icon = icon_idx;
            if (icon_idx >= 9) {
                last_icon_used = icon_idx;
            }
            return 1;
        }
    }
    return 0;
}

// Dispatch a click on the floating cohort-control box (patrol / return-home / patrol-stop buttons)
// shown when an army is being tracked. Returns 1 if the click was consumed (and the box was
// dismissed), 0 otherwise.
// FUNCTION: C2 0x31d46
// FUNCTION: C2WIN 0x004b4b98
int perform_cohort_box_action(void)
{
    int army_record_offset;
    int button_size;

    if (exit_screen() != 0) {
        pointer_mode = 0;
        update_map = 1;
        setup_map_screen_refresh();
        return 1;
    }
    army_record_offset = tracking_army * 0xaf;
    if (army_list[tracking_army].type != 1) {
        return 0;
    }

    control_buttons(0x190, 0x82, cohort_buttons, 1);
    if (mouse_left_preclick == 0) {
        return 0;
    }

    /* Three 34x34 buttons at the bottom of the cohort box. */
    button_size = 0x22;
    if (mouse_in_area(0x28, 0x126, button_size, button_size) != 0) {
        act_set_patrol_markers();
        return 1;
    }
    if (mouse_in_area(0xb8, 0x126, button_size, button_size) != 0) {
        act_set_return_home();
        return 1;
    }
    if (mouse_in_area(0x148, 0x126, button_size, button_size) != 0) {
        act_set_patrol_stop();
        return 1;
    }
    return 0;
}

// Enter turbo mode (fast-forward). Disabled on the battle screen.
// FUNCTION: C2 0x31e1a
// FUNCTION: C2WIN 0x004b4cb6
void act_init_turbo_mode(void)
{
    if (map_mode == 2) {
        return;
    }
    turbo_mode   = 1;
    pointer_mode = 0;
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
    update_map = 1;
}

// Leave turbo mode and request a full-screen repaint.
// FUNCTION: C2 0x31e45
// FUNCTION: C2WIN 0x004b4cee
void act_exit_turbo_mode(void)
{
    turbo_mode = 0;
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
    update_map = 1;
}

// "New Game" menu action. In tutorial / demo mode just shows a warning; otherwise asks the user to
// confirm and, if accepted, sets `restart_flag` so the main loop tears the game down.
// FUNCTION: C2 0x31e5c
// FUNCTION: C2WIN 0x004b4d0a
void act_new_game(void)
{
    if (tutorial_mode != 0) {
        click_warning(2, 0x50, 0xa0);
        return;
    }
    if (demo_mode != 0) {
        click_warning(6, 0x50, 0xa0);
        return;
    }
    confirm(1, 0xa0, 0xa0);
    if (decision == 1) {
        restart_flag = 1;
        pre_loaded_status = 0;
        if (map_mode == 2) {
            battle_state = 0xa;
        }
    }
}

// "Load Game" menu action. In tutorial / demo mode shows a warning; otherwise calls
// `load_a_game()`.
// FUNCTION: C2 0x31edd
// FUNCTION: C2WIN 0x004b4da7
void act_load_game(void)
{
    int saved_map_mode = map_mode;
    if (tutorial_mode != 0) {
        click_warning(2, 0x50, 0xa0);
        return;
    }
    if (demo_mode != 0) {
        click_warning(6, 0x50, 0xa0);
        return;
    }
    load_a_game();
    if (saved_map_mode == 2) {
        battle_state = 0xa;
    }
}

// "Save Game" menu action. Tutorial/demo: warning only.
// FUNCTION: C2 0x31f38
// FUNCTION: C2WIN 0x004b4e1e
void act_save_game(void)
{
    if (tutorial_mode != 0) {
        click_warning(2, 0x50, 0xa0);
        return;
    }
    if (demo_mode != 0) {
        click_warning(6, 0x50, 0xa0);
        return;
    }
    save_a_game();
    if (map_mode == 2) {
        battle_screen(0);
    }
}

// "Exit Game" menu action. Tutorial mode: warning.
// FUNCTION: C2 0x31f8f
// FUNCTION: C2WIN 0x004b4e8e
void act_exit_game(void)
{
    if (tutorial_mode != 0) {
        click_warning(2, 0x50, 0xa0);
        return;
    }
    pointer_mode = 0;
    show_exit_box();
    out1 = 0;
    while (out1 == 0) {
        exit_game_loop();
    }

    if (decision == 1) {
        exit_flag = 1;
        if (map_mode == 2) {
            battle_state = 0xa;
        }
    }
#if C2_FEAT_TILE_REFRESH
    setup_map_screen_refresh();
#endif
    update_map = 1;
}

// Exit-confirmation modal: "yes" — commit the exit.
// FUNCTION: C2 0x3200d
// FUNCTION: C2WIN 0x004b4f21
void act_do_exit(void)
{
    decision = 1;
    out1     = 1;
}

// Exit-confirmation modal: save the game and dismiss.
// FUNCTION: C2 0x3201f
// FUNCTION: C2WIN 0x004b4f3d
void act_exit_and_save(void)
{
    save_a_game();
    out1 = 1;
}

// Exit-confirmation modal: "no" — cancel the exit.
// FUNCTION: C2 0x32026
// FUNCTION: C2WIN 0x004b4f57
void act_dont_exit(void)
{
    decision = 0;
    out1     = 1;
}

// Open the FX-options dialog (mode 0 = tunes), pumping `tune_game_loop` until the modal closes. On
// exit refresh the whole screen and re-apply tune volume.
// FUNCTION: C2 0x32030
// FUNCTION: C2WIN 0x004b4f73
void act_toggle_tunes(void)
{
    int tutorial_active = tutorial_mode;
    if (tutorial_active != 0) {
        click_warning(2, 0x50, 0xa0);
        return;
    }
    show_fx_box(0);
    out1 = tutorial_active;
    while (out1 == 0) {
        tune_game_loop();
    }
    setup_whole_screen_refresh();
    set_sequences_volume();
}

// Toggles music and stops or starts the tune appropriate to the current map.
// FUNCTION: C2 0x3207d
// FUNCTION: C2WIN 0x004b4fd1
void act_tog_tunes(void)
{
    c2inf.tunes_on ^= 1;
    show_fx_box(0);
    if (c2inf.tunes_on == 0) {
        stop_tune();
        return;
    }
    if (map_mode == 2) {
        play_tune("batest2.xmi", 1);
    } else {
        play_tune("cityprov.xmi", 0);
    }
}

// Adjust the music-volume slider in the FX dialog. Hands the dialog back to `adjust()` (kind=3,
// target=&c2inf.tunes_level, step 1, max 0x64, min 0, x=0x70, y=0x90, no callback).
// FUNCTION: C2 0x320c2
// FUNCTION: C2WIN 0x004b5040
void act_tunes_level(void)
{
    adjust(3, &c2inf.tunes_level, 1, 0x64, 0, 0x70, 0x90, 0);
    set_sequences_volume();
    out1 = 0;
    show_fx_box(0);
}

// FX-options dialog (mode 1 = sound effects), looping until modal closes. Refresh + reapply sample
// volume on exit.
// FUNCTION: C2 0x320fd
// FUNCTION: C2WIN 0x004b5086
void act_toggle_sound_fx(void)
{
    int tutorial_active = tutorial_mode;
    if (tutorial_active != 0) {
        click_warning(2, 0x50, 0xa0);
        return;
    }
    show_fx_box(1);
    out1 = tutorial_active;
    while (out1 == 0) {
        samples_game_loop();
    }
    setup_whole_screen_refresh();
    set_samples_volume();
}

// Toggle the SFX-enabled flag and re-render the FX dialog.
// FUNCTION: C2 0x3214d
// FUNCTION: C2WIN 0x004b50e4
void act_tog_samples(void)
{
    c2inf.samples_on ^= 1;
    show_fx_box(1);
    if (c2inf.samples_on == 0) stop_samples();
}

// Toggle the ambient-sound flag and re-render the FX dialog.
// FUNCTION: C2 0x3216c
// FUNCTION: C2WIN 0x004b511b
void act_tog_ambients(void)
{
    c2inf.ambients_on ^= 1;
    show_fx_box(1);
    if (c2inf.ambients_on == 0) stop_all_sounds();
}

// Toggle the speech-enabled flag and re-render the FX dialog.
// FUNCTION: C2 0x3218b
// FUNCTION: C2WIN 0x004b5152
void act_tog_speech(void)
{
    c2inf.speech_on ^= 1;
    show_fx_box(1);
    if (c2inf.speech_on == 0) stop_db();
}

// Adjusts the sound-effects volume, applies it, and refreshes the options dialog.
// FUNCTION: C2 0x321aa
// FUNCTION: C2WIN 0x004b5189
void act_samples_level(void)
{
    adjust(4, &c2inf.samples_level, 1, 0x64, 0, 0x70, 0x90, 0);
    set_samples_volume();
    out1 = 0;
    show_fx_box(1);
}

// Adjusts the maximum number of simultaneous sound effects between one and four.
// FUNCTION: C2 0x321ec
// FUNCTION: C2WIN 0x004b51cf
void act_nof_samples(void)
{
    adjust(5, &c2inf.max_samples, 1, 4, 1, 0x70, 0x90, 2);
    out1 = 0;
    show_fx_box(1);
}

// Toggle-animations dialog (mode 2 = anims). Tutorial / demo blocked.
// FUNCTION: C2 0x32215
// FUNCTION: C2WIN 0x004b5210
void act_toggle_anims(void)
{
    int tutorial_active = tutorial_mode;
    if (tutorial_active != 0) {
        click_warning(2, 0x50, 0xa0);
        return;
    }
    if (demo_mode != 0) {
        click_warning(6, 0x50, 0xa0);
        return;
    }
    show_fx_box(2);
    out1 = tutorial_active;
    while (out1 == 0) {
        tog_anims_game_loop();
    }
    setup_whole_screen_refresh();
}

// Toggle the animations flag and re-render the FX dialog (anims tab).
// FUNCTION: C2 0x3227a
// FUNCTION: C2WIN 0x004b528c
void act_tog_anims(void)
{
    c2inf.anims_on ^= 1;
    show_fx_box(2);
}

// FX-options dialog (mode 3 = end-of-year summary toggle). Tutorial blocked, no demo branch.
// FUNCTION: C2 0x3228b
// FUNCTION: C2WIN 0x004b52af
void act_toggle_year_end(void)
{
    int tutorial_active = tutorial_mode;
    if (tutorial_active != 0) {
        click_warning(2, 0x50, 0xa0);
        return;
    }
    show_fx_box(3);
    out1 = tutorial_active;
    while (out1 == 0) {
        tog_yearend_game_loop();
    }
    setup_whole_screen_refresh();
}

// Toggle the end-of-year-summary flag and re-render the FX dialog.
// FUNCTION: C2 0x322d6
// FUNCTION: C2WIN 0x004b5308
void act_tog_yearend(void)
{
    c2inf.yearend_on ^= 1;
    show_fx_box(3);
}

// Toggles year-end autosaving and refreshes the options dialog. Disabled in tutorials and demos.
// FUNCTION: C2 0x322e7
// FUNCTION: C2WIN 0x004b532b
void act_tog_autosave(void)
{
    if (tutorial_mode != 0) {
        click_warning(2, 0x50, 0xa0);
        return;
    }
    if (demo_mode != 0) {
        click_warning(6, 0x50, 0xa0);
        return;
    }
    c2inf.autosave_on ^= 1;
    show_fx_box(3);
}

// Adjusts game speed in ten-point steps. Disabled during tutorials.
// FUNCTION: C2 0x32337
// FUNCTION: C2WIN 0x004b5394
void act_game_speed(void)
{
    if (tutorial_mode != 0) {
        click_warning(2, 0x50, 0xa0);
        return;
    }
    adjust(1, &c2inf.game_speed, 0xa, 0x64, 0, 0xa0, 0xa0, 1);
}

// Adjusts map scrolling speed in ten-point steps. Disabled during tutorials.
// FUNCTION: C2 0x32386
// FUNCTION: C2WIN 0x004b53e7
void act_scroll_speed(void)
{
    if (tutorial_mode != 0) {
        click_warning(2, 0x50, 0xa0);
        return;
    }
    adjust(2, &c2inf.scroll_speed, 0xa, 0x64, 0, 0xa0, 0xa0, 1);
}

// Launch help topic 2 (tips). Blocked in tutorial mode.
// FUNCTION: C2 0x323d5
// FUNCTION: C2WIN 0x004b543a
void act_help_tips(void)
{
    if (tutorial_mode) click_warning(2, 0x50, 0xA0);
    else               helping(2);
}

// In-game F1/help-button: pop the main help index modal.
// FUNCTION: C2 0x32404
// FUNCTION: C2WIN 0x004b5472
void act_help_game(void)
{
    helping(1);
}

// Launch help topic 3 (history). Blocked in tutorial mode.
// FUNCTION: C2 0x3243d
// FUNCTION: C2WIN 0x004b5487
void act_help_history(void)
{
    if (tutorial_mode) click_warning(2, 0x50, 0xA0);
    else               helping(3);
}

// Launch help topic 0x5C (icon legend).
// FUNCTION: C2 0x3246c
// FUNCTION: C2WIN 0x004b54bf
void act_help_icons(void)
{
    helping(0x5c);
}

// Show the "About" / credits modal. Loops `just_idle_game_loop` while the user is reading;
// right-click or any `exit_screen` hit closes the modal.
// FUNCTION: C2 0x32473
// FUNCTION: C2WIN 0x004b54d4
void act_about(void)
{
    show_about_box();
    out1 = 0;
    while (out1 == 0) {
        just_idle_game_loop();
        if (mouse_right_click != 0) {
            out1 = 1;
        }
        if (exit_screen() != 0) {
            out1 = 1;
        }
    }
    clear_mouse();
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
}

// Pop the in-game help/topics modal for `help_page_id`, then refresh whichever main screen we came from
// (city / region / battle) so the help overlay is wiped. Saves and restores `pointer_mode` across
// the call.
// FUNCTION: C2 0x32409 REORDERED
void helping(int help_page_id)
{
    int saved_pointer_mode = pointer_mode;
    pointer_mode = 0;
    launch_help(help_page_id);
    if (map_mode == 0) {
        city_map_screen(1);
    } else if (map_mode == 1) {
        region_map_screen(1);
    } else {
        battle_screen(1);
    }
    flush_sb_buffer();
    pointer_mode = saved_pointer_mode;
}

// Help modal: rewind history and signal the modal to redisplay (out2 = 10).
// FUNCTION: C2 0x324f1
// FUNCTION: C2WIN 0x004b55f2
void act_rewind_help(void)
{
    rewind_help_history();
    out2 = 10;
}

// Help modal: pause speech playback; ignored while the message queue is busy.
// FUNCTION: C2 0x32501
void act_pause_help(void)
{
    if (pause_db() != 0) return;
    help_buttons[1].state = 0;   /* un-toggle the help play/pause button */
}

// Help modal: restart from the beginning of the help history (out2 = 10).
// FUNCTION: C2 0x32513
// FUNCTION: C2WIN 0x004b562b
void act_start_help(void)
{
    init_help_history();
    out2 = 10;
}

// Help modal: exit (set out3 = 1, out2 = 10).
// FUNCTION: C2 0x3251a
// FUNCTION: C2WIN 0x004b5645
void act_exit_help(void)
{
    out3 = 1;
    out2 = 10;
}

// Generic "yes" button: decision = 1, out1 = 100 — dismiss the modal.
// FUNCTION: C2 0x32526
// FUNCTION: C2WIN 0x004b5664
void act_yes(void)
{
    decision = 1;
    out1     = 100;
}

// Generic "no" button: decision = 0, out1 = 100 — dismiss the modal.
// FUNCTION: C2 0x32538
// FUNCTION: C2WIN 0x004b5680
void act_no(void)
{
    decision = 0;
    out1     = 100;
}

// Toggle the global pause flag.
// FUNCTION: C2 0x32542
// FUNCTION: C2WIN 0x004b569c
void act_pause(void)
{
    c2inf.paused ^= 1;
}

// Generic "out" button: set out1 = 10 to break a modal loop.
// FUNCTION: C2 0x3254a
// FUNCTION: C2WIN 0x004b56b5
void act_out(void)
{
    out1 = 0xA;
}

// Shared adjust-slider button: bump *adjust_var up by adjust_step (clamped at adjust_max).
// FUNCTION: C2 0x32555
// FUNCTION: C2WIN 0x004b56ca
void act_adjust_up(void)
{
    if (*adjust_var < adjust_max) *adjust_var += adjust_step;
}

// Shared adjust-slider button: drop *adjust_var down by adjust_step (clamped at adjust_min).
// FUNCTION: C2 0x3256f
// FUNCTION: C2WIN 0x004b56f5
void act_adjust_down(void)
{
    if (*adjust_var > adjust_min) *adjust_var -= adjust_step;
}

// Debug/test placement (placing_type 5) — cheat-only.
// FUNCTION: C2 0x32589
// FUNCTION: C2WIN 0x004b5725
void act_test(void)
{
    placing_type  = 5;
    placing_flags = 0;
}

// Play the "exclaim" beep.
// FUNCTION: C2 0x3259e
// FUNCTION: C2WIN 0x004b5744
void act_exclaim(void)
{
    high_beep();
}

// Undo the last city-map placement and clear the placing context.
// FUNCTION: C2 0x325a3
// FUNCTION: C2WIN 0x004b5754
void act_undo_cm(void)
{
    if (sb_cm_undo_flushed) {
        return;
    }
    restore_city_from_undo_buffer();
#if C2_FEAT_TILE_REFRESH
    setup_map_screen_refresh();
#endif
    placing_type   = 0xFF;
    placing_flags  = 0;
    pm_build_shape = 0;
}

// Pop the houses selection list (or open act_house1 directly when the housing cheat is off).
// FUNCTION: C2 0x325d2
// FUNCTION: C2WIN 0x004b5796
void act_houses(void)
{
    flag_mode = 0;
    if (housing_cheat) {
        get_selection_goods_list(0);
#if PLATFORM_WINDOWS
        selection_menu = 1;
        control_selection(houses_selection, 6, mouse_x, mouse_y, 0x18);
#else
        control_selection(houses_selection, 6, mouse_x - 0x70, mouse_y - 0x30, 0x18);
#endif
    } else {
        act_house1();
    }
}

// Pop the "water structures" selection list (wells / fountains / etc.).
// FUNCTION: C2 0x3261d
// FUNCTION: C2WIN 0x004b57f3
void act_water(void)
{
    flag_mode = 0;
    get_selection_goods_list(0);
#if PLATFORM_WINDOWS
    selection_menu = 2;
    control_selection(water_selection, 5, 0x14, 0x122, 0xD);
#else
    control_selection(water_selection, 5, mouse_x - 0x80, mouse_y - 0x30, 0xD);
#endif
    selected_icon_text = 0xD;
    selected_icon_no = selection_is;
}

// Pop the security-buildings selection list (prefecture etc.).
// FUNCTION: C2 0x3266f
// FUNCTION: C2WIN 0x004b5848
void act_security(void)
{
    flag_mode = 0;
    get_selection_goods_list(0);
#if PLATFORM_WINDOWS
    selection_menu = 3;
    control_selection(security_selection, 5, 0x14, 0x122, 0xE);
#else
    control_selection(security_selection, 5, mouse_x - 0x90, mouse_y - 0x30, 0xE);
#endif
    selected_icon_text = 0xE;
    selected_icon_no = selection_is;
}

// Pop the health-buildings selection list (hospital / baths).
// FUNCTION: C2 0x326b3
// FUNCTION: C2WIN 0x004b589d
void act_health(void)
{
    flag_mode = 0;
    get_selection_goods_list(0);
#if PLATFORM_WINDOWS
    selection_menu = 4;
    control_selection(health_selection, 3, 0x32, 0x11D, 0xF);
#else
    control_selection(health_selection, 3, mouse_x - 0x90, mouse_y - 0x30, 0xF);
#endif
    selected_icon_text = 0xF;
    selected_icon_no = selection_is;
}

// Pop the gardens/plaza selection list.
// FUNCTION: C2 0x326fa
// FUNCTION: C2WIN 0x004b58f2
void act_gardens_plaza(void)
{
    flag_mode = 0;
    get_selection_goods_list(0);
    control_selection(gardens_plaza_selection, 3, mouse_x - 0x70, mouse_y - 0x30, 0x3A);
    selected_icon_text = 0x3A;
    selected_icon_no = selection_is;
}

// Enter city-clear placement mode (cost from city_costs[1]).
// FUNCTION: C2 0x3273e
// FUNCTION: C2WIN 0x004b5947
void act_clear(void) { placing_type = 1; placing_flags = 0; placing_cost = city_costs[1]; pm_build_shape = 0; flag_mode = 0; }

// Enter city-road placement mode.
// FUNCTION: C2 0x32769
// FUNCTION: C2WIN 0x004b5984
void act_road(void)  { placing_type = 2; placing_flags = 0x20; placing_cost = city_costs[2]; pm_build_shape = 0; flag_mode = 0; }

// Enter plaza placement mode.
// FUNCTION: C2 0x32798
// FUNCTION: C2WIN 0x004b59c1
void act_plaza(void)   { placing_type = 7; placing_flags = 0;    placing_cost = city_costs[8];  pm_build_shape = 0; }

// Enter gardens placement mode.
// FUNCTION: C2 0x327bd
// FUNCTION: C2WIN 0x004b59f4
void act_gardens(void) { placing_type = 6; placing_flags = 0;    placing_cost = city_costs[7];  pm_build_shape = 0; }

// Enter tier-1 house placement mode.
// FUNCTION: C2 0x327d7
// FUNCTION: C2WIN 0x004b5a27
void act_house1(void) { placing_type = 0x82; placing_flags = 1; placing_cost = city_costs[30]; pm_build_shape = 0; }

// Enter tier-2 house placement mode.
// FUNCTION: C2 0x32800
// FUNCTION: C2WIN 0x004b5a5a
void act_house2(void) { placing_type = 0x88; placing_flags = 1; placing_cost = city_costs[36]; pm_build_shape = 0; }

// Enter tier-3 house placement mode.
// FUNCTION: C2 0x3281c
// FUNCTION: C2WIN 0x004b5a8d
void act_house3(void) { placing_type = 0x8C; placing_flags = 1; placing_cost = city_costs[40]; pm_build_shape = 0; }

// Enter tier-4 house placement mode.
// FUNCTION: C2 0x32838
// FUNCTION: C2WIN 0x004b5ac0
void act_house4(void) { placing_type = 0x96; placing_flags = 1; placing_cost = city_costs[50]; pm_build_shape = 0; }

// Enter tier-5 house placement mode.
// FUNCTION: C2 0x32854
// FUNCTION: C2WIN 0x004b5af3
void act_house5(void) { placing_type = 0xA1; placing_flags = 1; placing_cost = city_costs[56]; pm_build_shape = 2; }

// Pop the forum-tier selection list (small/medium/large).
// FUNCTION: C2 0x3287d
// FUNCTION: C2WIN 0x004b5b26
void act_forums(void)
{
    flag_mode = 0;
    get_selection_goods_list(0);
    control_selection(forum_selection, 4, mouse_x - 0x80, mouse_y - 0x30, 0x14);
    selected_icon_text = 0x14;
    selected_icon_no = selection_is;
}

// Forum-selection: pick the small forum (pm_build_shape 1).
// FUNCTION: C2 0x328c4
// FUNCTION: C2WIN 0x004b5b7b
void act_select_small_forum(void)  { placing_type = 0xAE; placing_flags = 1; placing_cost = city_costs[70 + para1]; pm_build_shape = 1; }

// Forum-selection: pick the medium forum (pm_build_shape 2).
// FUNCTION: C2 0x328eb
// FUNCTION: C2WIN 0x004b5bb5
void act_select_medium_forum(void) { placing_type = 0xB2; placing_flags = 1; placing_cost = city_costs[70 + para1]; pm_build_shape = 2; }

// Forum-selection: pick the large forum (pm_build_shape 3).
// FUNCTION: C2 0x32910
// FUNCTION: C2WIN 0x004b5bef
void act_select_large_forum(void)  { placing_type = 0xB6; placing_flags = 1; placing_cost = city_costs[70 + para1]; pm_build_shape = 3; }

// Enter watch-tower placement mode.
// FUNCTION: C2 0x32940
// FUNCTION: C2WIN 0x004b5c29
void act_tower(void)      { placing_type = 0xBF; placing_flags = 4;    placing_cost = city_costs[5];  pm_build_shape = 0; }

// Enter city-wall placement mode.
// FUNCTION: C2 0x3295f
// FUNCTION: C2WIN 0x004b5c5c
void act_wall(void)       { placing_type = 3;    placing_flags = 2;    placing_cost = city_costs[3];  pm_build_shape = 0; }

// Enter barracks placement mode.
// FUNCTION: C2 0x3297e
// FUNCTION: C2WIN 0x004b5c8f
void act_barracks(void) { placing_type = 0xD; placing_flags = 1; placing_cost = city_costs[13]; pm_build_shape = 2; }

// Enter prefecture placement mode.
// FUNCTION: C2 0x3299c
// FUNCTION: C2WIN 0x004b5cc2
void act_prefecture(void) { placing_type = 0xE;  placing_flags = 1;    placing_cost = city_costs[14]; pm_build_shape = 0; }

// Enter reservoir placement mode.
// FUNCTION: C2 0x329bb
// FUNCTION: C2WIN 0x004b5cf5
void act_resevoir(void)   { placing_type = 0xBE; placing_flags = 0x80; placing_cost = city_costs[6];  pm_build_shape = 0; }

// Enter aqueduct placement mode.
// FUNCTION: C2 0x329da
// FUNCTION: C2WIN 0x004b5d28
void act_aquaduct(void)   { placing_type = 4;    placing_flags = 0x40; placing_cost = city_costs[4];  pm_build_shape = 0; }

// Enter fountain placement mode.
// FUNCTION: C2 0x329f9
// FUNCTION: C2WIN 0x004b5d5b
void act_fountain(void)   { placing_type = 0xC;  placing_flags = 1;    placing_cost = city_costs[12]; pm_build_shape = 0; }

// Enter well placement mode.
// FUNCTION: C2 0x32a18
// FUNCTION: C2WIN 0x004b5d8e
void act_well(void)       { placing_type = 8;    placing_flags = 1;    placing_cost = city_costs[9];  pm_build_shape = 0; }

// Pop the industry selection list.
// FUNCTION: C2 0x32a37
// FUNCTION: C2WIN 0x004b5dc1
void act_industries(void)
{
    flag_mode = 0;
    get_selection_goods_list(0);
    control_selection(industry_selection, 0x12, mouse_x - 0x80, mouse_y - 0x30, 0x10);
    if (selection_is == 1) {
        selected_icon_text = 0x10;
        selected_icon_no = selection_is;
    }
}

// Enter business placement mode using business_build_type from para1.
// FUNCTION: C2 0x32a8e
// FUNCTION: C2WIN 0x004b5e23
void act_business(void)
{
    business_build_type = para1;
    placing_type = 0x10;
    placing_flags = 1;
    placing_cost = city_costs[16];
    pm_build_shape = 2;
}

// Enter market placement mode.
// FUNCTION: C2 0x32ab6
// FUNCTION: C2WIN 0x004b5e60
void act_market(void)   { placing_type = 0xF; placing_flags = 1; placing_cost = city_costs[15]; pm_build_shape = 1; }

// Enter hospital placement mode.
// FUNCTION: C2 0x32ad6
// FUNCTION: C2WIN 0x004b5e93
void act_hospital(void) { placing_type = 0xB; placing_flags = 1; placing_cost = city_costs[11]; pm_build_shape = 2; }

// Enter baths placement mode.
// FUNCTION: C2 0x32af4
// FUNCTION: C2WIN 0x004b5ec6
void act_baths(void)    { placing_type = 0xA; placing_flags = 1; placing_cost = city_costs[10]; pm_build_shape = 1; }

// Pop the temple-tier selection list.
// FUNCTION: C2 0x32b14
// FUNCTION: C2WIN 0x004b5ef9
void act_temple(void)
{
    flag_mode = 0;
    get_selection_goods_list(0);
    control_selection(temple_selection, 4, mouse_x - 0x70, mouse_y - 0x30, 0x16);
    selected_icon_text = 0x16;
    selected_icon_no = selection_is;
}

// Temple-selection: pick the small temple.
// FUNCTION: C2 0x32b58
// FUNCTION: C2WIN 0x004b5f4e
void act_select_small_temple(void)  { placing_type = 0x14; placing_flags = 1; placing_cost = city_costs[20]; pm_build_shape = 0; }

// Temple-selection: pick the medium temple.
// FUNCTION: C2 0x32b77
// FUNCTION: C2WIN 0x004b5f81
void act_select_medium_temple(void) { placing_type = 0x15; placing_flags = 1; placing_cost = city_costs[21]; pm_build_shape = 1; }

// Temple-selection: pick the large temple.
// FUNCTION: C2 0x32b97
// FUNCTION: C2WIN 0x004b5fb4
void act_select_large_temple(void)  { placing_type = 0x16; placing_flags = 1; placing_cost = city_costs[22]; pm_build_shape = 2; }

// Pop the education-buildings selection list.
// FUNCTION: C2 0x32bb5
// FUNCTION: C2WIN 0x004b5fe7
void act_education(void)
{
    flag_mode = 0;
    get_selection_goods_list(0);
    control_selection(education_selection, 4, mouse_x - 0xA0, mouse_y - 0x30, 0x15);
    selected_icon_text = 0x15;
    selected_icon_no = selection_is;
}

// Education-selection: pick the grammaticus.
// FUNCTION: C2 0x32bfc
// FUNCTION: C2WIN 0x004b603c
void act_select_grammaticus(void) { placing_type = 0x11; placing_flags = 1; placing_cost = city_costs[17]; pm_build_shape = 1; }

// Education-selection: pick the rhetor school.
// FUNCTION: C2 0x32c1c
// FUNCTION: C2WIN 0x004b606f
void act_select_rhetor(void)      { placing_type = 0x12; placing_flags = 1; placing_cost = city_costs[18]; pm_build_shape = 2; }

// Education-selection: pick the library.
// FUNCTION: C2 0x32c3a
// FUNCTION: C2WIN 0x004b60a2
void act_select_library(void)     { placing_type = 0x13; placing_flags = 1; placing_cost = city_costs[19]; pm_build_shape = 2; }

// Pop the entertainment-buildings selection list.
// FUNCTION: C2 0x32c58
// FUNCTION: C2WIN 0x004b60d5
void act_entertainment(void)
{
    flag_mode = 0;
    get_selection_goods_list(0);
    control_selection(entertainment_selection, 7, mouse_x - 0x90, mouse_y - 0x30, 0x17);
    selected_icon_text = 0x17;
    selected_icon_no = selection_is;
}

// Entertainment-selection: pick the theatre.
// FUNCTION: C2 0x32c9f
// FUNCTION: C2WIN 0x004b612a
void act_select_theatre(void)   { placing_type = 0x17; placing_flags = 1; placing_cost = city_costs[23]; pm_build_shape = 1; }

// Entertainment-selection: pick the odium.
// FUNCTION: C2 0x32cbf
// FUNCTION: C2WIN 0x004b615d
void act_select_odium(void)     { placing_type = 0x18; placing_flags = 1; placing_cost = city_costs[24]; pm_build_shape = 1; }

// Entertainment-selection: pick the arena.
// FUNCTION: C2 0x32cdf
// FUNCTION: C2WIN 0x004b6190
void act_select_arena(void)     { placing_type = 0x19; placing_flags = 1; placing_cost = city_costs[25]; pm_build_shape = 2; }

// Entertainment-selection: pick the colosseum.
// FUNCTION: C2 0x32cfd
// FUNCTION: C2WIN 0x004b61c3
void act_select_colosseum(void) { placing_type = 0x1A; placing_flags = 1; placing_cost = city_costs[26]; pm_build_shape = 2; }

// Entertainment-selection: pick the circus.
// FUNCTION: C2 0x32d1b
// FUNCTION: C2WIN 0x004b61f6
void act_select_circus(void)     { placing_type = 0x1B; placing_flags = 1; placing_cost = city_costs[27]; pm_build_shape = 4; }

// Entertainment-selection: pick the circus maximus.
// FUNCTION: C2 0x32d44
// FUNCTION: C2WIN 0x004b6229
void act_select_circus_max(void) { placing_type = 0x1C; placing_flags = 1; placing_cost = city_costs[28]; pm_build_shape = 5; }

// Selection modal: cancel — clear the placing context.
// FUNCTION: C2 0x32d6d
// FUNCTION: C2WIN 0x004b625c
void act_select_cancel(void)
{
    reg_placing_type = 0;
    placing_type     = 0;
    placing_flags    = 0;
    placing_cost     = 0;
}

// Pop the overview-map legend panel. Loads the legend overlay, shows it, then idles in `read_mouse
// + colour_cycle_delay1` until the user releases the mouse button.
// FUNCTION: C2 0x32d8a
// FUNCTION: C2WIN 0x004b628f
void act_show_ov_legend(void)
{
    get_landfill(1);
    load_overlay_graphics(1);
    show_ov_legend_panel();
    do {
        read_mouse();
        if (mse_button == 0) {
            break;
        }
        if (colour_cycle_delay1(0x3c) != 0) {
            pulse_red(0x48, 6);
        }
    } while (1);
    load_overlay_graphics(0);
    show_landfill(com_x, com_y);
    setup_refresh_area(0x1e0, 0x30, 0xa, 0xb, 1);
    setup_whole_screen_refresh();
}

// Pop the overview-map "select map type" selection. `get_selection_goods_list(0)` builds the
// option list, then `control_selection` runs the modal at fixed coordinates.
// FUNCTION: C2 0x32e0e
// FUNCTION: C2WIN 0x004b62d1
void act_select_ov_map(void)
{
    get_selection_goods_list(0);
    control_selection(ovmap_selection, 0xb, 0x1f4, 0x36, 0x35);
    update_ov_bar = 1;
    redraw_icons = 1;
    update_map = 1;
    get_landfill(1);
    update_landfill = 1;
}

// Empty hook for undoing the last region-map placement.
// FUNCTION: C2 0x32e60
void act_undo_rm(void)
{
}

// Switch the overview map to mode 0 (geography) and trigger a landfill rebuild.
// FUNCTION: C2 0x32e61
// FUNCTION: C2WIN 0x004b632e
void act_ov_geography(void)
{
    ov_map_mode = 0;
    need_glf    = 1;
    clear_landfill();
}

// Switch the overview map to mode 1 (land value) and trigger a landfill rebuild.
// FUNCTION: C2 0x32e75
// FUNCTION: C2WIN 0x004b634c
void act_ov_landval(void)
{
    ov_map_mode = 1;
    need_glf    = 1;
    clear_landfill();
}

// Switch the overview map to mode 2 (water coverage).
// FUNCTION: C2 0x32e88
// FUNCTION: C2WIN 0x004b636a
void act_ov_water(void)    { ov_map_mode = 2; need_glf = 1; clear_landfill(); }

// Switch the overview map to mode 3 (security).
// FUNCTION: C2 0x32e91
// FUNCTION: C2WIN 0x004b6388
void act_ov_security(void) { ov_map_mode = 3; need_glf = 1; clear_landfill(); }

// Switch the overview map to mode 4 (unrest).
// FUNCTION: C2 0x32e9a
// FUNCTION: C2WIN 0x004b63a6
void act_ov_unrest(void)
{
    ov_map_mode = 4;
    need_glf    = 1;
    clear_landfill();
}

// Switch the overview map to mode 5 (administration).
// FUNCTION: C2 0x32ea3
// FUNCTION: C2WIN 0x004b63c4
void act_ov_admin(void)
{
    ov_map_mode = 5;
    need_glf    = 1;
    clear_landfill();
}

// Switch the overview map to mode 6 (entertainment).
// FUNCTION: C2 0x32eac
// FUNCTION: C2WIN 0x004b63e2
void act_ov_entertainment(void)
{
    ov_map_mode = 6;
    need_glf    = 1;
    clear_landfill();
}

// Switch the overview map to mode 7 (education).
// FUNCTION: C2 0x32eb5
// FUNCTION: C2WIN 0x004b6400
void act_ov_education(void)
{
    ov_map_mode = 7;
    need_glf    = 1;
    clear_landfill();
}

// Switch the overview map to mode 8 (health).
// FUNCTION: C2 0x32ebe
// FUNCTION: C2WIN 0x004b641e
void act_ov_health(void)
{
    ov_map_mode = 8;
    need_glf    = 1;
    clear_landfill();
}

// Switch the overview map to mode 9 (industry).
// FUNCTION: C2 0x32ec7
// FUNCTION: C2WIN 0x004b643c
void act_ov_industry(void) { ov_map_mode = 9; need_glf = 1; clear_landfill(); }

// Open the three-entry region security selection list and store the chosen icon.
// FUNCTION: C2 0x32ed0
// FUNCTION: C2WIN 0x004b645a
void act_rm_security(void)
{
    flag_mode = 0;
    pointer_mode = 0;
    get_selection_goods_list(0);
    control_selection(rm_security_selection, 3,
                      mouse_x - 0x90, mouse_y - 0x20, 0x36);
    selected_icon_text = 0x36;
    selected_icon_no = selection_is;
}

// Open the seven-entry region industry selection list and store the chosen icon.
// FUNCTION: C2 0x32f1f
// FUNCTION: C2WIN 0x004b64b6
void act_rm_industry(void)
{
    flag_mode = 0;
    pointer_mode = 0;
    get_selection_goods_list(0);
    control_selection(rm_industry_selection, 7,
                      mouse_x - 0x90, mouse_y - 0x30, 0x37);
    selected_icon_text = 0x37;
    selected_icon_no = selection_is;
}

// Enter region-map clear placement mode.
// FUNCTION: C2 0x32f6e
// FUNCTION: C2WIN 0x004b651d
void act_clear_rm(void) { reg_placing_type = 0x21; reg_placing_flags = 0;    placing_cost = region_costs[1]; pm_build_shape = 0; flag_mode = 0; pointer_mode = 0; }

// Enter region-map road placement mode.
// FUNCTION: C2 0x32f9c
// FUNCTION: C2WIN 0x004b6561
void act_road_rm(void)  { reg_placing_type = 0x1E; reg_placing_flags = 0x20; placing_cost = region_costs[2]; pm_build_shape = 0; flag_mode = 0; pointer_mode = 0; }

// Enter region-map wall placement mode.
// FUNCTION: C2 0x32fcb
// FUNCTION: C2WIN 0x004b65a5
void act_wall_rm(void)       { reg_placing_type = 0x1F; reg_placing_flags = 2; placing_cost = region_costs[3]; pm_build_shape = 0; pointer_mode = 0; }

// Enter region-map warehouse placement mode.
// FUNCTION: C2 0x32ffc
// FUNCTION: C2WIN 0x004b65df
void act_rm_warehouse(void)  { reg_placing_type = 0x24; reg_placing_flags = 1; placing_cost = region_costs[8]; pm_build_shape = 0; pointer_mode = 0; }

// Enter region-map workhouse placement mode.
// FUNCTION: C2 0x33018
// FUNCTION: C2WIN 0x004b6619
void act_rm_workhouse(void)  { reg_placing_type = 0x23; reg_placing_flags = 1; placing_cost = region_costs[5]; pm_build_shape = 0; pointer_mode = 0; }

// Enter region-map port placement mode.
// FUNCTION: C2 0x33034
// FUNCTION: C2WIN 0x004b6653
void act_rm_port(void)
{
    reg_placing_type  = 0x28;
    reg_placing_flags = 1;
    placing_cost      = region_costs[7];
    pm_build_shape    = 1;
    flag_mode         = 0;
    pointer_mode      = 0;
}

// Enter region-map shipyard placement mode.
// FUNCTION: C2 0x3306e
// FUNCTION: C2WIN 0x004b6697
void act_rm_shipyard(void)
{
    pointer_mode      = 0;
    reg_placing_type  = 0x2A;
    reg_placing_flags = 1;
    placing_cost      = region_costs[9];
    pm_build_shape    = 1;
}

// Enter region-map farm placement mode.
// FUNCTION: C2 0x33096
// FUNCTION: C2WIN 0x004b66d1
void act_rm_farm(void)   { reg_placing_type = 0x25; reg_placing_flags = 1; placing_cost = region_costs[6]; pm_build_shape = 1; pointer_mode = 0; }

// Enter region-map mine placement mode.
// FUNCTION: C2 0x330c6
// FUNCTION: C2WIN 0x004b670b
void act_rm_mine(void)   { reg_placing_type = 0x26; reg_placing_flags = 1; placing_cost = region_costs[6]; pm_build_shape = 1; pointer_mode = 0; }

// Enter region-map quarry placement mode.
// FUNCTION: C2 0x330d3
// FUNCTION: C2WIN 0x004b6745
void act_rm_quarry(void) { reg_placing_type = 0x27; reg_placing_flags = 1; placing_cost = region_costs[6]; pm_build_shape = 1; pointer_mode = 0; }

// Enter region-map trading-post placement mode.
// FUNCTION: C2 0x330e0
// FUNCTION: C2WIN 0x004b677f
void act_rm_trading_post(void)
{
    reg_placing_type  = 0x29;
    reg_placing_flags = 1;
    placing_cost      = region_costs[10];
    pm_build_shape    = 1;
    flag_mode         = 0;
    pointer_mode      = 0;
}

// Enter region-map fortress placement mode.
// FUNCTION: C2 0x33101
// FUNCTION: C2WIN 0x004b67c3
void act_rm_fort(void)   { reg_placing_type = 0x22; reg_placing_flags = 4; placing_cost = region_costs[4]; pm_build_shape = 0; pointer_mode = 0; }

// "Order cohort" entry — clears the placing context, switches the pointer to selection-mode (2),
// and snapshots `denarii` so any subsequent build can be priced.
// FUNCTION: C2 0x33120
// FUNCTION: C2WIN 0x004b67fd
void act_order_cohort(void)
{
    flag_mode = 0;
    reg_placing_type = 0;
    reg_placing_flags = 0;
    pointer_mode = 2;
    starting_denarii = denarii;
}

// Starts patrol-route placement for the tracked cohort, unless it has no troops while its morale
// timer is active or is already inactive.
// FUNCTION: C2 0x33148
// FUNCTION: C2WIN 0x004b6837
void act_set_patrol_markers(void)
{
    int route_row;
    int point_idx;

    if ((army_list[tracking_army].total_troops == 0
            && army_list[tracking_army].morale_timer != 0)
            || army_list[tracking_army].state_idx == 0xa) {
        put_message(0x61, 0, 0);
        pointer_mode = 0;
        setup_map_screen_refresh();
        update_map = 1;
        clear_mouse();
        return;
    }

    pointer_mode = 6;
    army_list[tracking_army].dest_y = 0;
    army_list[tracking_army].dest_x = 0;
    unflag_all_rm_xwarehouse();

    /* Clear all 10 patrol-route slots' 15 entries. */
    for (route_row = 0; route_row < 10; route_row++) {
        for (point_idx = 0; point_idx < 15; point_idx++) {
            army_routes[(signed char)
                army_list[tracking_army].cohort_id]
                .points[route_row][point_idx].x = 0;
            army_routes[(signed char)
                army_list[tracking_army].cohort_id]
                .points[route_row][point_idx].y = 0;
        }
    }
    for (route_row = 0; route_row < 10; route_row++) army_routes[(signed char)army_list[tracking_army].cohort_id].row_len[route_row] = 0;

    this_route_number = 0;
    over_x = army_list[tracking_army].x;
    over_y = army_list[tracking_army].y;
    if ((signed char)army_list[tracking_army].state_idx == 4
            || (signed char)army_list[tracking_army].state_idx == 8) {
        army_list[tracking_army].order_progress = 1;
    } else {
        army_list[tracking_army].order_progress = 0;
    }
    set_route_elastic();
    save_undo_info();
    setup_map_screen_refresh();
    clear_mouse();
}

// Clears the tracked cohort's patrol route and orders it back to its fortress.
// FUNCTION: C2 0x3329b
void act_set_return_home(void)
{
    int fortress_cell_idx;
    int fortress_x;
    int route_row;

    pointer_mode = 0;
    army_list[tracking_army].dest_y = 0;
    army_list[tracking_army].dest_x = 0;
    unflag_all_rm_xwarehouse();

    for (route_row = 0; route_row < 10; route_row++) {
        army_routes[(signed char)
            army_list[tracking_army].cohort_id].row_len[route_row] = 0;
    }
    army_routes[(signed char)
        army_list[tracking_army].cohort_id].row_count = 0;
    army_routes[(signed char)
        army_list[tracking_army].cohort_id].chase_row = 0;
    army_routes[(signed char)
        army_list[tracking_army].cohort_id].target_army = 0;

    fortress_cell_idx = army_list[tracking_army].fort_ref / 8;
    fortress_x = fortress_cell_idx % 60;
    army_list[tracking_army].target_x = fortress_x;
    army_list[tracking_army].target_y = (fortress_cell_idx / 60);
    army_list[tracking_army].state_idx = 5;
    army_list[tracking_army].flags &= ~2;
    army_list[tracking_army].order_progress = 1;
    setup_map_screen_refresh();
    clear_mouse();
}

// Stop the selected cohort's patrol, return it to idle state, and refresh the map controls.
// FUNCTION: C2 0x33360
// FUNCTION: C2WIN 0x004b6ce8
void act_set_patrol_stop(void)
{
    int route_row;
    int army_state;

    pointer_mode = 0;
    army_list[tracking_army].dest_y = 0;
    army_list[tracking_army].dest_x = 0;
    unflag_all_rm_xwarehouse();

    for (route_row = 0; route_row < 10; route_row++) {
        army_routes[(signed char)
            army_list[tracking_army].cohort_id].row_len[route_row] = 0;
    }
    army_routes[(signed char)
        army_list[tracking_army].cohort_id].row_count = 0;
    army_routes[(signed char)
        army_list[tracking_army].cohort_id].chase_row = 0;
    army_routes[(signed char)
        army_list[tracking_army].cohort_id].target_army = 0;

    army_list[tracking_army].target_x =
        army_list[tracking_army].x;
    army_list[tracking_army].target_y =
        army_list[tracking_army].y;
    army_state = (signed char)army_list[tracking_army].state_idx;
    if (army_state == 4 || army_state == 8) {
        army_list[tracking_army].order_progress = 1;
    } else {
        army_list[tracking_army].order_progress = 0;
    }
    army_list[tracking_army].state_idx = 3;
    army_list[tracking_army].flags &= ~2;

    setup_map_screen_refresh();
    clear_mouse();
}

// Rotate the map view clockwise by 90 degrees and refresh.
// FUNCTION: C2 0x3342f
// FUNCTION: C2WIN 0x004b6f46
void act_rotate_clockwise(void)
{
    rotate_pm_anticlockwise();
    if (map_mode == 2) {
        if (c2inf.paused) figure_images();
#if C2_FEAT_TILE_REFRESH
        setup_battle_screen_refresh();
#endif
    }
#if C2_FEAT_TILE_REFRESH
    else {
        setup_map_screen_refresh();
    }
#endif
    clear_edge_info();
    update_landfill = 1;
    update_map      = 1;
    pointer_mode    = 0;
#if C2_FEAT_ROTATE_PM_LIMITS
    pm_limits();
#endif
}

// Rotate the map view counter-clockwise by 90 degrees and refresh.
// FUNCTION: C2 0x3347a
// FUNCTION: C2WIN 0x004b6f99
void act_rotate_anticlockwise(void)
{
    rotate_pm_clockwise();
    if (map_mode == 2) {
        if (c2inf.paused) figure_images();
#if C2_FEAT_TILE_REFRESH
        setup_battle_screen_refresh();
#endif
    }
#if C2_FEAT_TILE_REFRESH
    else {
        setup_map_screen_refresh();
    }
#endif
    clear_edge_info();
    update_landfill = 1;
    update_map      = 1;
    pointer_mode    = 0;
#if C2_FEAT_ROTATE_PM_LIMITS
    pm_limits();
#endif
}

// Handles a zoom-out click.
// FUNCTION: C2 0x33483
// FUNCTION: C2WIN 0x004b6fec
void act_zoom_out(void)
{
    do_act_zoom_out(0);
}

// Zoom out one step. At zoom 2 nothing happens.
// FUNCTION: C2 0x33485
// FUNCTION: C2WIN 0x004b7001
void do_act_zoom_out(int decayed_click)
{
    if (zoom_level == 2) {
        return;
    }
    if (zoom_level == 1 || decayed_click != 0) {
        pm_x -= 0xc;
        pm_y -= 0x28;
        refresh_zoom_mode(2);
    } else if (zoom_level == 0) {
        pm_x -= 4;
        pm_y -= 0x10;
        refresh_zoom_mode(1);
    }
    pm_limits();
    setup_map_screen_refresh();
    clip_zoom_level1();
    clear_edge_info();
    update_landfill = 1;
    update_map = 1;
    load_map_graphics(map_mode, zoom_level);
    pointer_mode = 0;
}

// Zoom-in click handler. At zoom 0 it just sets `action_sound` (the click is filtered later).
// FUNCTION: C2 0x33513
// FUNCTION: C2WIN 0x004b7196
void act_zoom_in(void)
{
    if (zoom_level == 0) {
        action_sound = 1;
        return;
    }
    if (pointer_mode == 1) {
        if (zoom_level == 1) {
            pm_x_coord = 8;
            pm_y_coord = 0x1c;
        } else if (zoom_level == 2) {
            pm_x_coord = 0x10;
            pm_y_coord = 0x3c;
        }
        do_act_zoom_in(0);
        return;
    }
    if (zoom_level > 0) {
        pointer_mode = 1;
    }
}

// Zoom in one step. At zoom 1 we always shift; at zoom 0 we shift only if `decayed_click==1`.
// FUNCTION: C2 0x33583
// FUNCTION: C2WIN 0x004b72fc
void do_act_zoom_in(int decayed_click)
{
    if (zoom_level == 1 || decayed_click == 1) {
        pm_x = pm_x_coord + pm_x - 4;
        pm_y = ((pm_y_coord + pm_y) & 0xfffe) - 0xe;
        refresh_zoom_mode(0);
    } else if (zoom_level == 2) {
        pm_x = pm_x_coord + pm_x - 8;
        pm_y = ((pm_y_coord + pm_y) & 0xfffe) - 0x1e;
        refresh_zoom_mode(1);
    }
    pm_limits();
    setup_map_screen_refresh();
    clip_zoom_level1();
    clear_edge_info();
    update_landfill = 1;
    update_map = 1;
    load_map_graphics(map_mode, zoom_level);
    pointer_mode = 0;
}

// Jump to the city map and dismiss the current modal.
// FUNCTION: C2 0x33640
// FUNCTION: C2WIN 0x004b74bd
void act_goto_city(void)
{
    act_goto_city_map();
    out3 = 1;
}

// Toggle between city and region maps. Saves the current view's rotation/zoom into its slot
// (city_rotation/zoom or prov_rotation/zoom), restores the other side's, flips `map_mode`, then
// calls `act_correct_map` to adjust pm_x/pm_y.
// FUNCTION: C2 0x33650
// FUNCTION: C2WIN 0x004b74e9
void act_swap_maps(void)
{
    if (c2inf.peace_mode != 0) {
        click_warning(4, 0x50, 0xa0);
        return;
    }
    pointer_mode = 0;
    pm_build_shape = 0;
    placing_type = 0;
    placing_flags = 0;

    if (map_mode == 0) {
        map_mode = 1;
        city_rotation = map_direction;
        city_zoom_level = zoom_level;
        map_direction = prov_rotation;
        zoom_level = prov_zoom_level;
    } else {
        map_mode = 0;
        prov_rotation = map_direction;
        prov_zoom_level = zoom_level;
        map_direction = city_rotation;
        zoom_level = city_zoom_level;
    }
    act_correct_map();
}

// Switch to the city map (no-op if already there). Saves the region pm_x/pm_y into region_pm_x/_y
// so they survive the swap.
// FUNCTION: C2 0x336fc
// FUNCTION: C2WIN 0x004b75b9
void act_goto_city_map(void)
{
    if (map_mode == 0) {
        return;
    }
    pointer_mode = 0;
    pm_build_shape = 0;
    placing_type = 0;
    placing_flags = 0;

    prov_rotation = map_direction;
    prov_zoom_level = zoom_level;
    map_direction = city_rotation;
    zoom_level = city_zoom_level;
    map_mode = 0;

    region_pm_x = pm_x;
    region_pm_y = pm_y;
    pm_x = city_pm_x;
    pm_y = city_pm_y;
    act_correct_map();
}

// Switches to the region map unless peaceful mode disables it.
// FUNCTION: C2 0x33783
// FUNCTION: C2WIN 0x004b7722
void act_goto_prov_map(void)
{
    if (c2inf.peace_mode != 0) {
        click_warning(4, 0x50, 0xa0);
        return;
    }
    if (map_mode == 1) {
        return;
    }
    pointer_mode = 0;
    pm_build_shape = 0;
    placing_type = 0;
    placing_flags = 0;

    city_rotation = map_direction;
    city_zoom_level = zoom_level;
    map_direction = prov_rotation;
    zoom_level = prov_zoom_level;
    map_mode = 1;

    city_pm_x = pm_x;
    city_pm_y = pm_y;

    if (region_pm_x == -1) {
        map_actual_width = 0x3c;
        map_actual_height = 0x3c;
        map_actual_atom = 8;
        map_width_reduction = 0xa;
        map_height_reduction = 0xa;
        get_pseudo_map(map_direction);
        jump_to_regionmap_ptr(reg_city_ptr);
        region_pm_x = pm_x;
        region_pm_y = pm_y;
    }
    map_mode = 1;
    pm_x = region_pm_x;
    pm_y = region_pm_y;
    act_correct_map();
}

// After a map-mode change (city/region/battle), set the map_actual_* dimensions, command-strip
// rectangle, reset placing state, rebuild the pseudo_map, refresh the zoom, reload the graphic
// tiles, and finally show the destination screen.
// FUNCTION: C2 0x33899
// FUNCTION: C2WIN 0x004b78ff
void act_correct_map(void)
{
    if (map_mode == 1) {
        /* Region */
        map_actual_width  = 0x3c;
        map_actual_height = 0x3c;
        map_actual_atom   = 8;
        map_width_reduction  = 0xa;
        map_height_reduction = 0xa;
        com_x = 0x1f4; com_y = 0x44;
        com_w = 0x78;  com_h = 0x78;
    } else if (map_mode == 0) {
        /* City */
        map_actual_width  = 0x50;
        map_actual_height = 0x50;
        map_actual_atom   = 0x14;
        map_width_reduction = map_height_reduction = 0;
        com_x = 0x1e0; com_y = 0x30;
        com_w = 0xa0;  com_h = 0xa0;
    } else if (map_mode == 2) {
        /* Battle */
        map_actual_width  = 0x34;
        map_actual_height = 0x34;
        map_actual_atom   = 4;
        map_width_reduction  = 0xe;
        map_height_reduction = 0xe;
        com_x = 0x1e0; com_y = 0x30;
        com_w = 0xa0;  com_h = 0xa0;
    }

    update_icon = 0;
    overlays_on = 0;
    reg_placing_type = 0;
    reg_placing_flags = 0;
    placing_type = 0;
    placing_flags = 0;
    pm_build_shape = 0;

    get_pseudo_map(map_direction);

    if (map_mode == 2) {
        refresh_battle_zoom_mode(zoom_level);
    } else {
        refresh_zoom_mode(zoom_level);
    }
    pm_limits();
    setup_whole_screen_refresh();
    clear_edge_info();
    update_landfill = 1;

    if (map_mode == 2) {
        load_battle_graphics(zoom_level);
    } else {
        load_map_graphics(map_mode, zoom_level);
    }

    if (pre_loaded_status != 0 && map_mode == 2) {
        rebuild_figures_image_data();
    }

    if (map_mode == 0) {
        init_city_ambients();
        tune_mood = last_city_mood;
        if (city_tune_playing == 0) {
            play_tune("cityprov.xmi", 0);
        }
        city_tune_playing = 1;
    } else if (map_mode == 1) {
        init_prov_ambients();
        tune_mood = last_city_mood;
        if (city_tune_playing == 0) {
            play_tune("cityprov.xmi", 0);
        }
        city_tune_playing = 1;
    } else if (map_mode == 2) {
        init_battle_ambients();
        play_tune("batest2.xmi", 1);
        city_tune_playing = 0;
    }

    if (map_mode == 0) {
        city_map_screen(1);
    } else if (map_mode == 1) {
        region_map_screen(1);
    } else if (map_mode == 2) {
        battle_screen(1);
    }
    flush_sb_buffer();
    pointer_mode = 0;
}

// Enter flag-marker pointer mode and clear the placing context.
// FUNCTION: C2 0x33b1c
// FUNCTION: C2WIN 0x004b7c74
void act_goto_flags(void)
{
    pointer_mode   = 0;
    goto_flag_marker_mode();
    placing_type   = 0;
    placing_flags  = 0;
    pm_build_shape = 0;
}

// Cycle to the next "city flag" marker. If `next_city_flag` returns 0 the city has none and a "no
// markers" message (id 0x67) pops up; otherwise enter flag-marker pointer mode (with a 10-tick
// decay) and pan the city map to that flag.
// FUNCTION: C2 0x33b40
// FUNCTION: C2WIN 0x004b7ca9
void act_set_marker1(void)
{
    pointer_mode = 0;
    placing_type = 0;
    placing_flags = 0;
    pm_build_shape = 0;
    if (next_city_flag() == 0) {
        put_message(0x67, 0, 0);
        return;
    }
    if (flag_mode == 0) {
        goto_flag_marker_mode();
        flag_mode_decay_count = 0xa;
    }
    jump_to_citymap_ptr(city_flag_list[last_city_flag]);
}

// Cycles to the next province flag and recentres the region map on it.
// FUNCTION: C2 0x33ba2
// FUNCTION: C2WIN 0x004b7d38
void act_set_marker2(void)
{
    pointer_mode = 0;
    placing_type = 0;
    placing_flags = 0;
    pm_build_shape = 0;
    if (next_prov_flag() == 0) {
        put_message(0x67, 0, 0);
        return;
    }
    if (flag_mode == 0) {
        goto_flag_marker_mode();
        flag_mode_decay_count = 0xa;
    }
    jump_to_regionmap_ptr(prov_flag_list[last_prov_flag]);
}

// Danger-flag cycle. `danger_flag_map_mode` selects whether the flag is on the city map (0) or
// region map (non-zero).
// FUNCTION: C2 0x33c04
// FUNCTION: C2WIN 0x004b7dc7
void act_set_marker3(void)
{
    int target_map_ptr;

    pointer_mode = 0;
    placing_type = 0;
    placing_flags = 0;
    pm_build_shape = 0;
    if (next_danger_flag() == 0) {
        put_message(0x67, 0, 0);
        return;
    }
    if (flag_mode == 0) {
        goto_flag_marker_mode();
        flag_mode_decay_count = 0xa;
    }
    target_map_ptr = danger_flag_list[last_danger_flag];
    if (danger_flag_map_mode == 0) {
        jump_to_citymap_ptr(target_map_ptr);
    } else {
        jump_to_regionmap_ptr(target_map_ptr);
    }
}

// Open the forum (advisor) screen. Picks an entry tune based on `rand8`, resets `tracking_army`,
// primes the chosen department (slave_warning forces dept 8), then loops on `forum_game_loop`
// while the modal is up.
// FUNCTION: C2 0x33c77
void act_forum(void)
{
    pointer_mode = 0;
    stop_all_sounds();

    /* Entry tune choice based on rand8 buckets. */
    if (rand8 <= 1) play_tune("forum1.xmi", 1);
    else if (rand8 <= 4) play_tune("forum2.xmi", 1);
    else play_tune("forum3.xmi", 1);
    city_tune_playing = 0;
    tracking_army = 0;
    last_forum_dept = FORUM_DEPT_OVERVIEW;

    if (slave_warning != 0) { forum_dept = FORUM_DEPT_SLAVES; last_forum_dept = FORUM_DEPT_SLAVES; }
    evolve_to_current_fabric();
    forum_update_census(); current_temple_tip = 0;
    forum_constant_screen();

    show_forum_screen();
    out1 = 0;
    while (out1 == 0) {
        in_the_forum = 1;
        forum_game_loop();
        in_the_forum = 0;
        if (out1 == 2) { out1 = 0; stop_db(); show_forum_screen(); }
        forum_dept_over = FORUM_DEPT_OVERVIEW;

        if (c2inf.peace_mode == 0 || forum_dept != 0xb) {
            if (mouse_y < forum_repapering[forum_dept]) {
                continue;
            }
        }

        if (mouse_x >= 0x280) mouse_x = 0x27f;
        if (mouse_y >= 0x198) forum_dept_over = (char)over_forum_menu();
        else if (mouse_y >= 0xb0) {
            /* Pixel-pick from the dept-strip lookup table. */
            unsigned char *dept_strip_ptr;
            dept_strip_ptr = scratch_buffer; dept_strip_ptr += mouse_x / 8;
            forum_dept_over = dept_strip_ptr[(mouse_y - 0xb0) / 8 * 0x50 + 0x1d4c0];
        } else forum_dept_over = FORUM_DEPT_OVERVIEW;

        if (mouse_left_preclick == 0) continue;
        last_forum_dept = forum_dept;
        forum_dept = forum_dept_over;
        if (forum_dept == FORUM_DEPT_ADVISOR) launch_help(4);
        show_forum_screen();
    }

    forum_update_census();
    forum_dept = FORUM_DEPT_OVERVIEW;
    forum_dept_over = FORUM_DEPT_OVERVIEW;
    if (map_mode == 0) city_map_screen(1);
    else if (map_mode == 1) region_map_screen(1);
    play_tune("cityprov.xmi", 0);
    city_tune_playing = 1;
    flush_sb_buffer();
    in_the_forum = 0;
}

// Runs the interaction loop for the active forum department.
// FUNCTION: C2 0x33ea7
// FUNCTION: C2WIN 0x004b7e89
void forum_game_loop(void)
{
    int dept = forum_dept;
    if (dept == FORUM_DEPT_ADMIN) { forum_admin_game_loop();    return; }
    if (dept == FORUM_DEPT_CAREER) { forum_career_game_loop();   return; }
    if (dept == FORUM_DEPT_ROME) { forum_rome_game_loop();     return; }
    if (dept == FORUM_DEPT_CLERKS) { forum_clerks_game_loop();   return; }
    if (dept == FORUM_DEPT_ARMY) { forum_army_game_loop();     return; }
    if (dept == FORUM_DEPT_INDUSTRY) { forum_industry_game_loop(); return; }
    if (dept == FORUM_DEPT_SLAVES) { forum_slaves_game_loop();   return; }
    if (dept == FORUM_DEPT_EXIT) { out1 = 1; return; }
    if (dept == FORUM_DEPT_TEMPLE) { forum_temple_game_loop();  return; }
    if (dept == FORUM_DEPT_EMPIRE) { forum_empire_game_loop();  return; }
    forum_idle_game_loop();
}

// Forum-internal "go to message" hook — sets out1 to 1 so the outer modal cycles around and
// re-displays the message screen.
// FUNCTION: C2 0x33ef2
void act_goto_message(void)
{
    out1 = 1;
}

// Render the active forum department's full-screen layout. When transitioning out of the temple
// (0xa) or empire (0xb) views (the latter only outside tutorial mode) we first fade to black to
// mask the screen-tearing the new department's repaint would cause.
// FUNCTION: C2 0x33f14
// FUNCTION: C2WIN 0x004b7fa2
void show_forum_screen(void)
{
    int dept;

    if (last_forum_dept == FORUM_DEPT_TEMPLE) {
        black_out();
    }
    if (c2inf.peace_mode == 0 && last_forum_dept == FORUM_DEPT_EMPIRE) {
        black_out();
    }

    dept = forum_dept;
    if (dept == FORUM_DEPT_ADMIN) { forum_admin_screen();    return; }
    if (dept == FORUM_DEPT_CAREER) { forum_career_screen();   return; }
    if (dept == FORUM_DEPT_ROME) { forum_rome_screen();     return; }
    if (dept == FORUM_DEPT_CLERKS) { forum_clerks_screen();   return; }
    if (dept == FORUM_DEPT_ADVISOR) { forum_advisor_screen();  return; }
    if (dept == FORUM_DEPT_ARMY) { forum_army_screen();     return; }
    if (dept == FORUM_DEPT_INDUSTRY) { forum_industry_screen(); return; }
    if (dept == FORUM_DEPT_SLAVES) { forum_slaves_screen();   return; }
    if (dept == FORUM_DEPT_TEMPLE) { forum_temple_screen();  return; }
    if (dept == FORUM_DEPT_EMPIRE) { forum_empire_screen();  return; }
    forum_empty_screen();
}

// Hit-test the bottom-of-screen forum menu strip (FORUM_DEPT_END entries, 0..FORUM_DEPT_EMPIRE).
// Each is 0x18 wide x 0xa0 tall — coordinates from forum_menu[i*2] for x and forum_menu[i*2+1] for
// y.
// FUNCTION: C2 0x33fa5
// FUNCTION: C2WIN 0x004b80ef
int over_forum_menu(void)
{
    int menu_x;
    int menu_y;
    int i;
    int j;
    for (i = 0; i < FORUM_DEPT_END; i++) {
        menu_x = forum_menu[i].x;
        menu_y = forum_menu[i].y;
        if (mouse_in_area(menu_x, menu_y, 0xa0, 0x18) != 0) {
            return i;
        }
    }
    return 0;
}

// Updates the empire region under the mouse using each region's recorded screen bounds.
// FUNCTION: C2 0x33fde
// FUNCTION: C2WIN 0x004b8163
void get_region_over(void)
{
    int region_x;
    int region_idx;
    int region_height;
    int region_width;
    int bitmap_offset;
    int region_y;
    unsigned char pixel;

    region_over = 0;
    for (region_idx = 0; region_idx < 0x2c; region_idx++) {
        data_ptr = region_idx * 16 + 8;

        region_width = ((scratch_buffer)[data_ptr + 1] << 8) + (scratch_buffer)[data_ptr];
        region_height = (scratch_buffer)[data_ptr + 2] + ((scratch_buffer)[data_ptr + 3] << 8);
        region_x = empire_positions[region_idx].x;
        region_y = empire_positions[region_idx].y;
        bitmap_offset = (scratch_buffer)[data_ptr + 4] + ((scratch_buffer)[data_ptr + 5] << 8)
                + (scratch_buffer)[data_ptr + 6] * 0x10000;

        if (mouse_x < region_x) continue;
        if (region_y > mouse_y) continue;
        if (((region_x) + (region_width)) <= mouse_x) continue;
        if ((region_height + region_y) <= mouse_y) continue;

        region_x = mouse_x - region_x; region_y = mouse_y - region_y;
        pixel = *(scratch_buffer + bitmap_offset + region_x + region_y * region_width);
        if (pixel != 0) {
            region_over = region_idx + 1; return;
        }
    }
}

// "Final bribe to Caesar" modal. Loops gift_game_loop(0x10) until the player commits or aborts; if
// accepted (decision==1) calls `bribe_emperor`, otherwise sets `game_state = 1` (resignation).
// FUNCTION: C2 0x340c2
// FUNCTION: C2WIN 0x004b82fa
void act_final_bribe(void)
{
    final_bribe = 1;
    show_final_bribe_box();
    out1 = 0;
    decision = 0;
    if (players_denarii <= 0) {
        imperial_gift_level = 0;
    }
    while (out1 == 0) {
        gift_game_loop(0x10);
    }
    if (decision == 1) {
        bribe_emperor();
    } else {
        game_state = 1;
    }
    clear_mouse();
    out1 = 1;
}

// Drop Caesar's requested-tribute slider by 1 (floored at 0).
// FUNCTION: C2 0x34134
// FUNCTION: C2WIN 0x004b838b
void act_request_down(void) { if (imperial_send_amount > 0) imperial_send_amount--; gen_refresh1 = 1; }

// Raise Caesar's requested-tribute slider (clamped at the goods supply).
// FUNCTION: C2 0x34152
// FUNCTION: C2WIN 0x004b83b0
void act_request_up(void)
{
    if (imperial_send_amount < industry[imperial_req_goods].supply)
        imperial_send_amount++;
    gen_refresh1 = 1;
}

// Raise the population-tax rate one step (clamped at 0x19).
// FUNCTION: C2 0x34185
// FUNCTION: C2WIN 0x004b83f8
void act_pop_tax_up(void)   { if (pop_tax_rate < 0x19) pop_tax_rate++;   gen_refresh1 = 1; }

// Drop the population-tax rate one step (floored at 0).
// FUNCTION: C2 0x341a4
// FUNCTION: C2WIN 0x004b841d
void act_pop_tax_down(void) { if (pop_tax_rate > 0)    pop_tax_rate--;   gen_refresh1 = 1; }

// Raise the industry-tax rate one step (clamped at 0x19).
// FUNCTION: C2 0x341b9
// FUNCTION: C2WIN 0x004b8442
void act_ind_tax_up(void)   { if (ind_tax_rate < 0x19) ind_tax_rate++;   gen_refresh1 = 1; }

// Drop the industry-tax rate one step (floored at 0).
// FUNCTION: C2 0x341d8
// FUNCTION: C2WIN 0x004b8467
void act_ind_tax_down(void) { if (ind_tax_rate > 0)    ind_tax_rate--;   gen_refresh1 = 1; }

// Raise the player salary slider by 1 (clamped at 0x3E8).
// FUNCTION: C2 0x341ed
// FUNCTION: C2WIN 0x004b848c
void act_salary_up(void)    { if (players_salary < 0x3E8) players_salary++; gen_refresh1 = 1; }

// Drop the player salary slider by 1 (floored at 0).
// FUNCTION: C2 0x3420f
// FUNCTION: C2WIN 0x004b84b4
void act_salary_down(void)  { if (players_salary > 0)     players_salary--; gen_refresh1 = 1; }

// Open the "make a donation to Rome" modal. Clamps the donation to the player's available denarii
// first.
// FUNCTION: C2 0x34224
// FUNCTION: C2WIN 0x004b84d9
void act_donation(void)
{
    if (donation_level > players_denarii) {
        donation_level = players_denarii;
    }
    show_donation_box();
    out1 = 0;
    while (out1 == 0) {
        donation_game_loop();
    }
    forum_career_screen();
    out1 = 0;
    clear_mouse();
}

// "Donation +" button: bump the slider by 0xa if there's enough headroom (>= 10 below
// players_denarii), otherwise step by 1.
// FUNCTION: C2 0x3426f
// FUNCTION: C2WIN 0x004b8539
void act_donation_up(void)
{
    if ((players_denarii - 0xa) > donation_level) {
        donation_level += 0xa;
    } else if (donation_level < players_denarii) {
        donation_level += 1;
    }
    gen_refresh1 = 1;
}

// "Donation –" button: drop by 0xa if >=10, by 1 otherwise. Floors at 0 — clicks are ignored once
// we've hit it.
// FUNCTION: C2 0x342ab
// FUNCTION: C2WIN 0x004b8582
void act_donation_down(void)
{
    if (donation_level > 0xa) {
        donation_level -= 0xa;
    } else if (donation_level > 0) {
        donation_level -= 1;
    }
    gen_refresh1 = 1;
}

// Commit the chosen donation amount: transfer denarii from the player to Rome.
// FUNCTION: C2 0x342cd
// FUNCTION: C2WIN 0x004b85c0
void act_send_donation(void)
{
    denarii        += donation_level;
    players_denarii -= donation_level;
    act_goto_message();
}

// Extend the income-history graph window by one bucket (up to 4).
// FUNCTION: C2 0x342e3
// FUNCTION: C2WIN 0x004b85f0
void act_history_graph_longer(void)  { if (history_graph_length < 4) history_graph_length++; gen_refresh1 = 1; }

// Shrink the income-history graph window by one bucket (floored at 0).
// FUNCTION: C2 0x34302
// FUNCTION: C2WIN 0x004b8615
void act_history_graph_shorter(void) { if (history_graph_length > 0) history_graph_length--; gen_refresh1 = 1; }

// "Help" inside the army-box: launch help topic 0xb, then return to the army (forum dept 6)
// advisor view.
// FUNCTION: C2 0x34317
// FUNCTION: C2WIN 0x004b863a
void act_army_box_help(void)
{
    launch_help(0xb);
    forum_dept = 6;
    last_forum_dept = forum_dept;
    forum_constant_screen();
    show_forum_screen();
    hold_mouse_replace = 0;
    clear_mouse();
}

// Raise the army-wage slider by 5 denarii (clamped at 0x3E8).
// FUNCTION: C2 0x34349
// FUNCTION: C2WIN 0x004b8676
void act_army_wage_up(void)    { if (army_wage_level < 0x3E8) army_wage_level += 5; gen_refresh1 = 1; gen_refresh2 = 1; }

// Drop the army-wage slider by 5 denarii (floored at 0).
// FUNCTION: C2 0x34364
// FUNCTION: C2WIN 0x004b86a6
void act_army_wage_down(void)  { if (army_wage_level > 0)     army_wage_level -= 5; gen_refresh1 = 1; gen_refresh2 = 1; }

// Raise the conscription rate by 1 (clamped at 0x32).
// FUNCTION: C2 0x34375
// FUNCTION: C2WIN 0x004b86d3
void act_conscription_up(void)   { if (conscription_rate < 0x32) conscription_rate++; gen_refresh1 = 1; gen_refresh2 = 1; }

// Drop the conscription rate by 1 (floored at 0).
// FUNCTION: C2 0x3439c
// FUNCTION: C2WIN 0x004b86ff
void act_conscription_down(void) { if (conscription_rate > 0)    conscription_rate--; gen_refresh1 = 1; gen_refresh2 = 1; }

// Cycle the army-box view forward to the next cohort and request a refresh.
// FUNCTION: C2 0x343ad
// FUNCTION: C2WIN 0x004b872b
void act_next_cohort(void)
{
    get_next_viewed_cohort(0);
    gen_refresh2 = 1;
    gen_refresh1 = 1;
}

// Cycle the army-box view back to the previous cohort and request a refresh.
// FUNCTION: C2 0x343c3
// FUNCTION: C2WIN 0x004b874e
void act_prev_cohort(void)
{
    get_next_viewed_cohort(1);
    gen_refresh2 = 1;
    gen_refresh1 = 1;
}

// Cycles the viewed cohort through its demobilisation states, preserving and restoring its prior
// activity state when necessary.
// FUNCTION: C2 0x343ca
// FUNCTION: C2WIN 0x004b8771
void act_demob_cohort(void)
{
    short army_idx = (short)get_actual_viewed_army();
    temp_army = army_idx;

    if (army_list[army_idx].cohort_size_class == 0) {
        army_list[army_idx].cohort_size_class = 1;
    } else {
        unsigned int size_class = army_list[army_idx].cohort_size_class;
        if (size_class == 1) {
            army_list[army_idx].cohort_size_class = 2;
        } else if (size_class == 2) {
            army_list[army_idx].cohort_size_class = 3;
            army_list[army_idx].saved_state_idx = army_list[army_idx].state_idx;
            army_list[army_idx].state_idx = 0xa;
        } else {
            army_list[army_idx].cohort_size_class = 0;
            army_list[army_idx].state_idx = army_list[army_idx].saved_state_idx;
        }
    }
    gen_refresh1 = 1;
    gen_refresh2 = 1;
}

// "Hire +50 mercs" button: bumps mercs_in_army by 0x32, clamped at max_mercs_allowed.
// FUNCTION: C2 0x34448
// FUNCTION: C2WIN 0x004b88e9
void act_more_mercs(void)
{
    if (mercs_in_army < max_mercs_allowed) {
        mercs_in_army += 0x32;
        if (mercs_in_army >= max_mercs_allowed) {
            mercs_in_army = max_mercs_allowed;
        }
        gen_refresh3 = 1;
        gen_refresh1 = 1;
        gen_refresh2 = 1;
    }
}

// "Hire -50 mercs" button: rounds mercs_in_army down to the nearest multiple of 50, then subtracts
// 50 if already aligned. Floors at 0.
// FUNCTION: C2 0x34483
// FUNCTION: C2WIN 0x004b8941
void act_less_mercs(void)
{
    if (mercs_in_army > 0) {
        int remainder = mercs_in_army % 0x32;
        if (remainder != 0) {
            mercs_in_army -= remainder;
        } else {
            mercs_in_army -= 0x32;
        }
        if (mercs_in_army < 0) {
            mercs_in_army = 0;
        }
        gen_refresh3 = 1;
        gen_refresh1 = 1;
        gen_refresh2 = 1;
    }
}

// Raise the slave welfare bill by 1 (clamped at 0x61A8).
// FUNCTION: C2 0x344d8
// FUNCTION: C2WIN 0x004b89c0
void act_slave_welfare_up(void)   { if (slave_welfare_bill < 0x61A8) slave_welfare_bill++; gen_refresh1 = 1; }

// Drop the slave welfare bill by 1 (floored at 0).
// FUNCTION: C2 0x344fb
// FUNCTION: C2WIN 0x004b89e8
void act_slave_welfare_down(void) { if (slave_welfare_bill > 0)     slave_welfare_bill--; gen_refresh1 = 1; }

// Allocate one more slave to the fire-brigade category.
// FUNCTION: C2 0x3450c
// FUNCTION: C2WIN 0x004b8a0d
void act_slave_fire_up(void)        { alter_slave_reqs(1,  1); gen_refresh2 = 1; }

// Take one slave away from the fire-brigade category.
// FUNCTION: C2 0x34516
// FUNCTION: C2WIN 0x004b8a2b
void act_slave_fire_down(void)      { alter_slave_reqs(1, -1); gen_refresh2 = 1; }

// Add one slave to the city-road upkeep category.
// FUNCTION: C2 0x34523
// FUNCTION: C2WIN 0x004b8a49
void act_slave_city_road_up(void)   { alter_slave_reqs(2,  1); gen_refresh2 = 1; }

// Take one slave away from the city-road upkeep category.
// FUNCTION: C2 0x34530
// FUNCTION: C2WIN 0x004b8a67
void act_slave_city_road_down(void) { alter_slave_reqs(2, -1); gen_refresh2 = 1; }

// Add one slave to the city-water upkeep category.
// FUNCTION: C2 0x34538
// FUNCTION: C2WIN 0x004b8a85
void act_slave_city_water_up(void)  { alter_slave_reqs(3,  1); gen_refresh2 = 1; }

// Take one slave away from the city-water upkeep category.
// FUNCTION: C2 0x34545
// FUNCTION: C2WIN 0x004b8aa3
void act_slave_city_water_down(void){ alter_slave_reqs(3, -1); gen_refresh2 = 1; }

// Add one slave to the city-wall upkeep category.
// FUNCTION: C2 0x3454d
// FUNCTION: C2WIN 0x004b8ac1
void act_slave_city_wall_up(void)   { alter_slave_reqs(4,  1); gen_refresh2 = 1; }

// Take one slave away from the city-wall upkeep category.
// FUNCTION: C2 0x34566
// FUNCTION: C2WIN 0x004b8adf
void act_slave_city_wall_down(void) { alter_slave_reqs(4, -1); gen_refresh2 = 1; }

// Add one slave to the regional-work category. Gated on !c2inf.peace_mode.
// FUNCTION: C2 0x3456e
// FUNCTION: C2WIN 0x004b8afd
void act_slave_reg_work_up(void)    { if (!c2inf.peace_mode) { alter_slave_reqs(5,  1); gen_refresh2 = 1; } }

// Take one slave away from the regional-work category. Gated on !c2inf.peace_mode.
// FUNCTION: C2 0x34590
// FUNCTION: C2WIN 0x004b8b2f
void act_slave_reg_work_down(void)  { if (!c2inf.peace_mode) { alter_slave_reqs(5, -1); gen_refresh2 = 1; } }

// Add one slave to the regional-upkeep category. Gated on !c2inf.peace_mode.
// FUNCTION: C2 0x345b2
// FUNCTION: C2WIN 0x004b8b61
void act_slave_reg_upkeep_up(void)  { if (!c2inf.peace_mode) { alter_slave_reqs(6,  1); gen_refresh2 = 1; } }

// Take one slave away from the regional-upkeep category. Gated on !c2inf.peace_mode.
// FUNCTION: C2 0x345d4
// FUNCTION: C2WIN 0x004b8b93
void act_slave_reg_upkeep_down(void){ if (!c2inf.peace_mode) { alter_slave_reqs(6, -1); gen_refresh2 = 1; } }

// Moves a slave-work category's allocation toward its required level.
// FUNCTION: C2 0x345f6
// FUNCTION: C2WIN 0x004b8bc5
void act_set_slaves_to_need_level(int requirement_idx)
{
    int allocation_delta;

    while (slave_requirements[requirement_idx].current
            != slave_requirements[requirement_idx].max) {
        if (slave_requirements[requirement_idx].current
                < slave_requirements[requirement_idx].max) {
            allocation_delta = 1;
        } else if (slave_requirements[requirement_idx].current
                > slave_requirements[requirement_idx].max) {
            allocation_delta = -1;
        }
        if (alter_slave_reqs(requirement_idx, allocation_delta) == 0) {
            break;
        }
    }
    gen_refresh2 = 1;
}

// Adjusts one slave-work category while conserving the total through the free-slave pool.
// FUNCTION: C2 0x3463a
// FUNCTION: C2WIN 0x004b8c5d
int alter_slave_reqs(int requirement_idx, int allocation_delta)
{
    int donor_idx;

    if (allocation_delta == -1) {
        if (slave_requirements[requirement_idx].current <= 0) {
            return 0;
        }
        slave_requirements[requirement_idx].current -= 1;
        slave_requirements[7].current += 1;
        return 1;
    }

    if (allocation_delta == 1) {
        if (slave_requirements[7].current != 0) {
            slave_requirements[requirement_idx].current += 1;
            slave_requirements[7].current -= 1;
            return 1;
        }
        /* Take from another category, kinds 6 → 1. */
        for (donor_idx = 6; donor_idx > 0; donor_idx--) {
            if (donor_idx == requirement_idx) continue;
            if (slave_requirements[donor_idx].current == 0) continue;
            slave_requirements[requirement_idx].current += 1;
            slave_requirements[donor_idx].current -= 1;
            return 1;
        }
        return 0;
    }

    return 0;
}

// Open the "send a gift to Caesar" modal. Loops gift_game_loop(4) until the user accepts or
// cancels; if accepted commits the gift via `bribe_emperor`.
// FUNCTION: C2 0x346c7
// FUNCTION: C2WIN 0x004b8d3b
void act_send_gift(void)
{
    show_gift_box();
    out1 = 0;
    decision = 0;
    if (players_denarii <= 0) {
        imperial_gift_level = 0;
    }
    while (out1 == 0) {
        gift_game_loop(4);
    }
    if (decision == 1) {
        bribe_emperor();
    }
    forum_rome_screen();
    clear_mouse();
    out1 = 0;
}

// Raise the imperial-gift slider by 1, clamped at the player's denarii.
// FUNCTION: C2 0x34728
// FUNCTION: C2WIN 0x004b8db8
void act_gift_up(void)
{
    if (players_denarii > 0) {
        imperial_gift_level++;
        if (imperial_gift_level > players_denarii) imperial_gift_level = players_denarii;
        gen_refresh1 = 1;
    }
}

// Drop the imperial-gift slider by 1 (floored at 0).
// FUNCTION: C2 0x34755
// FUNCTION: C2WIN 0x004b8dfd
void act_gift_down(void)
{
    imperial_gift_level--;
    if (imperial_gift_level < 0) imperial_gift_level = 0;
    gen_refresh1 = 1;
}

// Commit the imperial gift (decision = (level != 0); dismiss modal).
// FUNCTION: C2 0x34779
// FUNCTION: C2WIN 0x004b8e25
void act_gift_send(void)
{
    decision = (imperial_gift_level != 0);
    out1     = 1;
}

// Refresh the temple-tips strip with the new tip_kind selection.
// FUNCTION: C2 0x34796
// FUNCTION: C2WIN 0x004b8e5a
void act_set_temple_tips(int tip_kind)
{
    get_temple_tip(tip_kind);
    gen_refresh1 = 1;
}

// Battle-screen zoom-level 1. No-op when already at level 1; otherwise re-centre on (0x1c, 0x38)
// and reload battle graphics.
// FUNCTION: C2 0x347a3
// FUNCTION: C2WIN 0x004b8e78
void act_zoom_level1(void)
{
    if (zoom_level == 1) {
        return;
    }
    pm_x = 0x1c;
    pm_y = 0x38;
    refresh_battle_zoom_mode(1);
#if C2_FEAT_TILE_REFRESH
    setup_battle_screen_refresh();
#endif
    clear_edge_info();
    update_landfill = 1;
    update_map = 1;
    load_battle_graphics(zoom_level);
    rebuild_figures_image_data();
    clip_battle_zoom_level2();
}

// Battle zoom-level 2. No-op at level 2; otherwise centre on (0xd, 0x18) and run the same refresh
// as act_zoom_level1 (refresh_battle_zoom_mode + load_battle_graphics + clip).
// FUNCTION: C2 0x347fb
// FUNCTION: C2WIN 0x004b8ee3
void act_zoom_level2(void)
{
    if (zoom_level == 2) {
        return;
    }
    pm_x = 0xd;
    pm_y = 0x18;
    refresh_battle_zoom_mode(2);
#if C2_FEAT_TILE_REFRESH
    setup_battle_screen_refresh();
#endif
    clear_edge_info();
    update_landfill = 1;
    update_map = 1;
    load_battle_graphics(zoom_level);
    rebuild_figures_image_data();
    clip_battle_zoom_level2();
}

// Starts an unstarted battle or toggles its paused state, then resets battle timing controls.
// FUNCTION: C2 0x34822
// FUNCTION: C2WIN 0x004b8f4e
void act_stop_go(void)
{
    last_icon_used = 8;
    redraw_icons = 1;
    if (battle_state == 0) {
        battle_state = 1;
    }
    c2inf.paused ^= 1;
    nomansland_ptr = 0x65900;
    battle_turbo = 0;
    battle_setup_count = 0;
}

// Toggle battle-screen turbo mode (no-op while battle_state == 0).
// FUNCTION: C2 0x34868
// FUNCTION: C2WIN 0x004b8fad
void act_turbo(void)
{
    if (battle_state) {
        battle_turbo ^= 1;
        battle_turbo_count = 0;
    }
}

// Battle: enter "move" pointer mode (1) when stats-control is on; otherwise deselect.
// FUNCTION: C2 0x34883
// FUNCTION: C2WIN 0x004b8fdb
void act_move_unit(void)
{
    if (zoom_level == 2) return;
    if (!battle_stats_control) deselect_all_figures();
    else pointer_mode = 1;
}

// Battle: enter "target" pointer mode (2) when stats-control is on; otherwise deselect.
// FUNCTION: C2 0x348a4
// FUNCTION: C2WIN 0x004b9019
void act_target_unit(void)
{
    if (zoom_level == 2) return;
    if (!battle_stats_control) deselect_all_figures();
    else pointer_mode = 2;
}

// Battle "retreat" button: confirm and, on yes, advance battle_state to 6.
// FUNCTION: C2 0x348c5
// FUNCTION: C2WIN 0x004b9057
void act_battle_retreat(void)    { confirm(5, 0xA0, 0xA0); if (decision == 1) battle_state = 6; pointer_mode = 0; }

// Battle "surrender" button: confirm and, on yes, advance battle_state to 7.
// FUNCTION: C2 0x348f9
// FUNCTION: C2WIN 0x004b9097
void act_battle_surrender(void)  { confirm(6, 0xA0, 0xA0); if (decision == 1) battle_state = 7; pointer_mode = 0; }

// Battle "auto-calculate" button: confirm and, on yes, advance battle_state to 5.
// FUNCTION: C2 0x34924
// FUNCTION: C2WIN 0x004b90d7
void act_battle_autocalc(void)   { confirm(7, 0xA0, 0xA0); if (decision == 1) battle_state = 5; pointer_mode = 0; }

// Battle: select every figure on the field (no-op at zoom 2).
// FUNCTION: C2 0x3494f
// FUNCTION: C2WIN 0x004b9117
void act_battle_select_all(void)
{
    if (zoom_level == 2) {
        return;
    }
    select_all_figures();
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
    update_map = 1;
}

// Battle: launch help topic 0x33.
// FUNCTION: C2 0x3496d
// FUNCTION: C2WIN 0x004b9143
void act_battle_help(void)
{
    helping(0x33);
}

// Battle: order selected units into a line formation (general_reform(0)).
// FUNCTION: C2 0x34977
// FUNCTION: C2WIN 0x004b9162
void act_unit_line_formation(void)
{
    if (zoom_level == 2) return;
    general_reform(0);
    if (!battle_state) return;
    if (!battle_stats_control) deselect_all_figures();
    else pointer_mode = 1;
}

// Battle: order selected units into a column formation (general_reform(1)).
// FUNCTION: C2 0x349a8
// FUNCTION: C2WIN 0x004b91bc
void act_unit_column_formation(void)
{
    if (zoom_level == 2) return;
    general_reform(1);
    if (!battle_state) return;
    if (!battle_stats_control) deselect_all_figures();
    else pointer_mode = 1;
}

// Battle: order selected units into a testudo / tortoise formation (general_reform(2)).
// FUNCTION: C2 0x349dc
// FUNCTION: C2WIN 0x004b9216
void act_unit_tortoise_formation(void)
{
    if (zoom_level == 2) return;
    general_reform(2);
    if (!battle_state) return;
    if (!battle_stats_control) deselect_all_figures();
    else pointer_mode = 1;
}

// Orders selected battle units to mop up remaining enemies.
// FUNCTION: C2 0x34a10
// FUNCTION: C2WIN 0x004b9270
void act_unit_mop_up_formation(void)
{
    if (zoom_level == 2) return;
    pointer_mode = 0;
    general_reform(3);
}

// "Choose skill levels" startup dialog. Loops show_skill1_box + skill1_game_loop until accepted,
// then show_skill2_box (unless the user already exited / preloaded a save / continues a tutorial).
// FUNCTION: C2 0x34a2e
// FUNCTION: C2WIN 0x004b92a1
void act_set_skill_levels(void)
{
    pre_loaded_status = 0;
    tutorial_mode = 0;
    continue_tutorial_status = 0;

choose_skill1:
    show_skill1_box();
    out1 = 0;
    while (out1 == 0) {
        skill1_game_loop();
    }
    if (continue_tutorial_status == 0
            && exit_flag == 0
            && pre_loaded_status == 0) {
        show_skill2_box();
        out1 = 0;
        while (out1 == 0) {
            skill2_game_loop();
        }
        if (out1 == 0x42a) goto choose_skill1;
    }
    flush_sb_buffer();
}

// Runs province selection for a promotion, or derives the peaceful-mode province difficulty from
// the chosen skill level.
// FUNCTION: C2 0x34ab0
// FUNCTION: C2WIN 0x004b9357
void act_choose_init_region(void)
{
    if (tutorial_mode != 0) {
        return;
    }
    if (c2inf.peace_mode != 0) {
        province_is = 0;
        province_difficulty = c2inf.skill_level * 2 + 1;
        return;
    }
    clear_mouse();
    get_new_province_options();
    if (provinces_on_offer == 0) {
        do_vga_smacked_anim("wingame.smk");
    }
    if (player_rank == 0) {
        show_initreg_box();
        show_first_region_box();
        out2 = 0;
        while (out2 != 1) {
            just_idle_game_loop();
            if (mouse_right_preclick != 0) {
                out2 = 1;
            }
        }
        reshow_initreg_box();
    } else {
        show_initreg_box();
    }
    if (provinces_on_offer == 0) {
        show_no_provinces_box();
    }
    out2 = 0;
    if (provinces_on_offer == 0) {
        while (out2 != 1) {
            just_idle_game_loop();
            if (mouse_right_click != 0) {
                out2 = 1;
            }
        }
        restart_flag = 1;
    } else {
        while (out2 != 1) {
            initreg_game_loop();
        }
    }
    flush_sb_buffer();
}

// Pop the "this region: <name>" confirmation modal during the new-province flow. Loops
// show_buttons + control_buttons (the Yes / No pair `confirming_buttons`) until out1 == 1.
// FUNCTION: C2 0x34bb1
// FUNCTION: C2WIN 0x004b94a6
void this_region(void)
{
    this_region_box(0);
    out1 = 0; decision = 0;
    while (out1 != 1) {
        gloop_start();
        show_buttons(0x170, 0x110, confirming_buttons, 2);
        gloop_end();
        control_buttons(0x170, 0x110, confirming_buttons, 2);
        if (out1 > 0xa) { out1 = 0xa; }
        if (out1 > 1) out1 -= 1;
    }
    if (decision == 0) { out2 = 0; }
    else if (decision == 1) { out2 = 1; }
    clear_mouse(); out1 = 0;
    stop_db();
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
}

// Run a tutorial-mode game once (`do_tutorial`). After it returns, either go to the skill1 dialog
// (regular flow) or just set out1=1 (`continue_tutorial_status` was set, meaning the player chose
// "continue" and we should not reset).
// FUNCTION: C2 0x34c7b
// FUNCTION: C2WIN 0x004b9590
void act_tutorial(void)
{
    do_tutorial();
    if (continue_tutorial_status == 0) {
        show_skill1_box();
        out1 = 0;
    } else {
        out1 = 1;
    }
}

// New-game flow: "quit to DOS" — set exit_flag and dismiss the modal.
// FUNCTION: C2 0x34ca5
// FUNCTION: C2WIN 0x004b9647
void act_dos(void)
{
    exit_flag = 1;
    act_goto_message();
}

// Raise the difficulty slider by 1 (clamped at 4).
// FUNCTION: C2 0x34cb1
// FUNCTION: C2WIN 0x004b9663
void act_skill_up(void)
{
    if (c2inf.skill_level < 4) {
        c2inf.skill_level++;
        gen_refresh1 = 1;
    }
}

// Drop the difficulty slider by 1 (floored at 0).
// FUNCTION: C2 0x34ccc
// FUNCTION: C2WIN 0x004b968b
void act_skill_down(void)
{
    if (c2inf.skill_level > 0) {
        c2inf.skill_level--;
        gen_refresh1 = 1;
    }
}

// Toggle the peaceful-mode (no-region) flag.
// FUNCTION: C2 0x34ceb
// FUNCTION: C2WIN 0x004b96b2
void act_tog_peace(void)
{
    c2inf.peace_mode ^= 1;
    gen_refresh2   = 1;
}

// "Edit player name" modal during the new-game flow. Idles new_name_game_loop while the user
// types, then re-shows skill2 box.
// FUNCTION: C2 0x34cfa
// FUNCTION: C2WIN 0x004b96d2
void act_choose_name(void)
{
    insert_cursor = 0;
#if C2_FEAT_NAME_EDIT_FB_COUNT
    fb_count = insert_cursor;
#else
    this_letter = 0;
#endif
    in_format_buffer(c2inf.player_name, 0x18, 0xa0, 2);
    show_new_name_box();
    out2 = 0;
    while (out2 == 0) {
        new_name_game_loop();
    }
    show_skill2_box();
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
}

// Close the current modal back to the new-game front panel (out1 = 1066).
// FUNCTION: C2 0x34d4d
// FUNCTION: C2WIN 0x004b9735
void act_back_to_front_panel(void)
{
    out1 = 1066;
}

// "Load saved game" entry from the new-game flow. On success (file_loaded_status set), close the
// new-game flow with pre_loaded_status and out1=1; otherwise reopen the skill1 box.
// FUNCTION: C2 0x34d58
// FUNCTION: C2WIN 0x004b974a
void act_preload(void)
{
    load_a_game();
    if (file_loaded_status != 0) {
        out1 = 1;
        pre_loaded_status = 1;
    } else {
        out1 = 0;
        show_skill1_box();
    }
#if C2_FEAT_TILE_REFRESH
    setup_whole_screen_refresh();
#endif
    hold_mouse_replace = 1;
}

// Pop the census panel and idle until the user dismisses it. Battle mode is silently ignored.
// FUNCTION: C2 0x34d95
// FUNCTION: C2WIN 0x004b9795
void act_census(void)
{
    if (map_mode == 2) {
        return;
    }
    in_census_mode = 1;
    clear_mouse();
    show_census_panel();
    out1 = 0;
    while (out1 != 1) {
        just_idle_game_loop();
        if (exit_screen() != 0) {
            out1 = 1;
        }
        if (mouse_right_click != 0) {
            out1 = 1;
        }
    }
    cover_mouse_droppings();
    setup_map_screen_refresh();
    in_census_mode = 0;
}

// Opens the city or region query panel for the cell under the cursor and restores the prior
// pointer mode after the panel closes.
// FUNCTION: C2 0x34e10
// FUNCTION: C2WIN 0x004b97d7
void act_query(void)
{
    int saved_pointer_mode;
    if (map_mode > 1) {
        return;
    }
    get_pm_over_diamond(1);
    saved_pointer_mode = pointer_mode;
    pointer_mode = 0;
    act_start_pm_ptr = pm_over_cm_ptr;
    act_start_ptr = pm_over_cm_ptr / map_actual_atom;
    act_start_x = act_start_ptr % map_actual_width;
    act_start_y = act_start_ptr / map_actual_width;

    evolve_to_current_fabric();
    if (map_mode == 0) {
        get_query_info();
    } else {
        get_region_query_info();
    }

    /* Reset the three "active dot" markers in the query button bar. */
    queery_buttons[3].state = 0;
    queery_buttons[4].state = 0;
    queery_buttons[5].state = 0;

    if (q_type >= 0x82
            && q_type <= 0xa1) {
        query_mode = last_house_query_mode;
        queery_buttons[last_house_query_mode + 3].state = 1;
    } else {
        unsigned int people_query_flag = q_flag & 0x20;
        if (people_query_flag != 0) {
            query_mode = 1;
            queery_buttons[4].state = 1;
        } else {
            query_mode = 0;
            queery_buttons[3].state = 1;
        }
    }

    if (map_mode == 0) {
        nof_query_buttons = 6;
    } else {
        nof_query_buttons = 3;
    }
    show_query_panel();
    clear_mouse();
    out3 = 0;
    while (out3 != 1) {
        queery_game_loop();
        if (exit_screen() != 0) {
            out2 = 1;
            out3 = 1;
        }
        if (mouse_right_click != 0) {
            out3 = 1;
        }
    }
    if (q_type >= 0x82
            && q_type <= 0xa1) {
        last_house_query_mode = query_mode;
    }
    setup_map_screen_refresh();
    update_map = 1;
    pointer_mode = saved_pointer_mode;
    clear_mouse();
}

// Query-panel "General" tab. Only acts when query_mode != 0: resets the other two button dots,
// clears query_mode, and re-renders the panel (city map first when in city view).
// FUNCTION: C2 0x34fb3
// FUNCTION: C2WIN 0x004b99d4
void act_general_query(void)
{
    if (query_mode == 0) {
        queery_buttons[3].state = 1;
    } else {
        queery_buttons[4].state = 0;
        queery_buttons[5].state = 0;
        query_mode = 0;
    }
    if (map_mode == 0) {
        show_citymap();
    }
    show_query_panel();
}

// Selects the query panel's people tab and refreshes the panel.
// FUNCTION: C2 0x34ff1
// FUNCTION: C2WIN 0x004b9a29
void act_people_query(void)
{
    if (query_mode == 1) {
        queery_buttons[4].state = 1;
    } else {
        queery_buttons[3].state = 0;
        queery_buttons[5].state = 0;
        query_mode = 1;
    }
    if (map_mode == 0) {
        show_citymap();
    }
    show_query_panel();
}

// Select the detailed query tab and repaint the query panel.
// FUNCTION: C2 0x35032
// FUNCTION: C2WIN 0x004b9a7e
void act_detailed_query(void)
{
    if (query_mode == 2) {
        queery_buttons[5].state = 1;
    } else {
        queery_buttons[3].state = 0;
        queery_buttons[4].state = 0;
        query_mode = 2;
    }
    if (map_mode == 0) {
        show_citymap();
    }
    show_query_panel();
}

// Query panel: navigate to the help page (delta 0).
// FUNCTION: C2 0x3505e
// FUNCTION: C2WIN 0x004b9ad3
void act_query_help(void)
{
    act_query_do_help(0);
}

// Query panel: navigate to the tips page (delta +1).
// FUNCTION: C2 0x35062
// FUNCTION: C2WIN 0x004b9ae8
void act_query_tips(void)
{
    act_query_do_help(1);
}

// Query panel: navigate to the history page (delta +2).
// FUNCTION: C2 0x35069
// FUNCTION: C2WIN 0x004b9afd
void act_query_history(void)
{
    act_query_do_help(2);
}

// Pop a help-page modal for the current query. Adds `page_delta` to `this_help_page` (so help-cursor
// arrows page through topics), then routes through three redirect tables (temple-tips,
// temple-history, ent-history) before checking the debar list.
// FUNCTION: C2 0x3506e
// FUNCTION: C2WIN 0x004b9b12
void act_query_do_help(int page_delta)
{
    int is_debarred;
    int redirect_idx;

    this_help_page += page_delta;
    is_debarred = 0;

    /* Temple-tips redirect: 2 entries, page 0xec is the canonical. */
    for (redirect_idx = 0; redirect_idx < 2; redirect_idx++) {
        if (this_help_page == help_redir_temple_tips[redirect_idx]) {
            this_help_page = 0xec;
        }
    }

    /* Temple-history redirect: 2 entries, page 0xed canonical. */
    for (redirect_idx = 0; redirect_idx < 2; redirect_idx++) {
        if (this_help_page == help_redir_temple_history[redirect_idx]) {
            this_help_page = 0xed;
        }
    }

    /* Entertainment-history redirect: 5 pairs (page → replacement). */
    for (redirect_idx = 0; redirect_idx < 5; redirect_idx++) {
        if (this_help_page == help_redir_ent_history[redirect_idx].page) {
            this_help_page = help_redir_ent_history[redirect_idx].replacement;
        }
    }

    /* Debar list — empty by default. */
    for (redirect_idx = 0; redirect_idx < 0; redirect_idx++) {
        if (this_help_page == help_debar[redirect_idx]) {
            is_debarred = 1;
            this_help_page -= page_delta;
        }
    }

    if (!is_debarred) {
        launch_help(this_help_page);
        if (map_mode == 0) {
            city_map_screen(1);
        } else if (map_mode == 1) {
            region_map_screen(1);
        } else {
            battle_screen(1);
        }
        show_query_panel();
    }
    flush_sb_buffer();
    pointer_mode = 0;
    out2 = 0;
    out3 = 0;
}

// Toggle query pointer mode (between 4 and 0).
// FUNCTION: C2 0x35170
// FUNCTION: C2WIN 0x004b9c74
void act_query_mode(void)
{
    if (pointer_mode == 4) pointer_mode = 0;
    else                   pointer_mode = 4;
}

// Runs year-end autosave and summary handling outside tutorial mode.
// FUNCTION: C2 0x35190
// FUNCTION: C2WIN 0x004b9ca2
void act_do_year_end(void)
{
    int saved_pointer_mode;

    if (tutorial_mode != 0) {
        return;
    }
    swap_circus_gfx();

    if (c2inf.yearend_on == 0) {
        if (c2inf.autosave_on != 0) {
            savegame("lastyear.sav");
        }
        return;
    }

    if (game_state == 3 || game_state == 1 || game_state == 2) {
        return;
    }

    saved_pointer_mode = pointer_mode;
    turbo_mode = 0;
    local_time = time_is;
    pointer_mode = 0;
    show_top_line();
    show_year_end_screen();
    out1 = 0;

    if (c2inf.autosave_on != 0) {
        savegame("lastyear.sav");
    }

    while (out1 == 0) {
        just_idle_game_loop();
        if (mouse_right_click != 0) {
            out1 = 1;
        }
        if (exit_screen() != 0) {
            out1 = 1;
        }
    }

    if (map_mode == 0) {
        city_map_screen(1);
    } else if (map_mode == 1) {
        region_map_screen(1);
    }
    flush_sb_buffer();
    pointer_mode = saved_pointer_mode;
    if (turbo_mode != 0) {
        act_init_turbo_mode();
    }
}

void (*city_actions[24])(void) = {
    act_rotate_clockwise,
    act_rotate_anticlockwise,
    act_goto_flags,
    act_set_marker1,
    act_set_marker2,
    act_set_marker3,
    act_goto_city_map,
    act_forum,
    act_goto_prov_map,
    act_zoom_in,
    act_clear,
    act_houses,
    act_road,
    act_forums,
    act_zoom_out,
    act_water,
    act_security,
    act_industries,
    act_health,
    act_query_mode,
    act_entertainment,
    act_temple,
    act_education,
    act_gardens_plaza
};

void (*region_actions[19])(void) = {
    act_rotate_clockwise,
    act_rotate_anticlockwise,
    act_goto_flags,
    act_set_marker1,
    act_set_marker2,
    act_set_marker3,
    act_goto_city_map,
    act_forum,
    act_goto_prov_map,
    act_zoom_in,
    act_clear_rm,
    act_road_rm,
    act_rm_security,
    act_rm_trading_post,
    act_zoom_out,
    act_query_mode,
    act_rm_industry,
    act_order_cohort,
    act_rm_port
};

void (*battle_actions[17])(void) = {
    act_rotate_clockwise,
    act_rotate_anticlockwise,
    act_zoom_level1,
    act_zoom_level2,
    act_stop_go,
    act_turbo,
    act_move_unit,
    act_target_unit,
    act_unit_line_formation,
    act_unit_column_formation,
    act_unit_tortoise_formation,
    act_unit_mop_up_formation,
    act_battle_retreat,
    act_battle_surrender,
    act_battle_autocalc,
    act_battle_select_all,
    act_battle_help
};

int help_redir_house_tips[5] = { 161, 176, 191, 206, 221 };

int help_redir_house_history[5] = { 162, 177, 192, 207, 222 };

int help_redir_temple_tips[2] = { 251, 266 };

int help_redir_temple_history[2] = { 252, 267 };

struct help_redirect_rec help_redir_ent_history[5] = {
    { 507, 492 },
    { 537, 522 },
    { 567, 552 },
    { 597, 582 },
    { 612, 582 }
};

int help_debar[31] = { 372, 477, 627, 671, 672, 686, 687, 701, 702, 716, 717, 731, 732, 746, 747, 762, 776, 777, 792, 806, 807, 821, 822, 897, 912, 926, 927, 942, 957, 972, 987 };
