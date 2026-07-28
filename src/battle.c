
#include "battle.h"
#include "c2_data.h"

/* File-local state. */
int bat_ai_trig_count;
int bat_attack_rate;
int bat_order;
int first_rear;
int bat_no_selected;
int bat_which;
int y_bit;
int first_front;
int which_unit;
int bat_no_of_units;
int x_bit;
int bat_attacker_clock;
int bat_odds;
int bat_spacing;
int first_back;
int xright_front;
int bat_control;
int bat_size;
int yfront;
int bat_hi;
int xleft_front;
int ypos;
int xright_rear;
int bat_morale;
int xleft_back;
int bat_side;
int xpos;
int xleft_rear;
int bat_no;
int bat_width;
int yrear;
int xright_back;
int yback;

extern int get_heading();

void elephant_fire(void);
/* Forward declarations (functions defined later in this file). */
void deselect_all_figures(void);
void deselect_enemy_figures(void);
void get_highlight_position(void);
void generate_battle_map(void);
void setup_battle(void);
void setup_roman_units(void);
void setup_enemy_units(void);
void get_battle_odds(void);
void build_units_figures(int deployment_idx, int figure_kind, int stampede_kind, int unit_sub_kind, int unit_rank, int stance, int army_morale, int target_men, int formation_cols, int row_spacing, unsigned char *arrow_data_ptr, unsigned char *sprite_data_ptr, int sprite_kind);
void find_attack_spot(void);
void find_defensive_spot(void);
void get_start_points(int deployment_idx);
void figure_intelligence(void);
void sf12_rout(void);
void get_arrow_base_image(void);
void reform(int unit_ref, int formation, int force_reform);
void instant_reform(int unit_idx, int formation);
void get_fig_in_unit_position(int formation, int position, int figure_idx);
void get_fig_fight_image(void);
void get_fig_walk_image(void);
void get_fig_still_image(void);
void get_fig_tortoise_image(void);
void get_fig_death_image(void);
void get_fig_missile_image(void);
void set_figure_map_refresh(int grid_x, int grid_y, int offset_x, int offset_y, int radius, int extra_size);
void set_missile_fire_range(int weapon_kind);
void elephant_ai(void);
void do_light_ai(void);
void do_heavy_ai(void);
void set_ai_flank_move(int flank_mode);
void set_ai_unit_move(int offset_x, int offset_y);
void set_ai_unit_withdraw(int offset_x, int offset_y);
void set_ai_unit_beserk(void);
void set_ai_unit_delayed_beserk(void);
void set_ai_unit_auto_fire(void);
void drop_all_units_morale(int match_type, int morale_a_delta, int morale_b_delta);
void raise_all_units_morale(int skip_type, int morale_a_delta, int morale_b_delta);
void set_unit_to_rout(int unit_idx);
void get_units_status(void);
void battle_tune_mood_from_type(int unit_idx);
void move_figure(int figure_idx);
void backtrack_figure(int figure_idx);
void fly_to_target(void);
void bd(int dominant_axis);
void move_arrow_vert(void);
void move_arrow_horiz(void);
void loose_arrow_move(void);
void do_the_fight(void);
void set_attack_count(int figure_idx);
void set_defense_shield(int figure_idx);

// Enter or resume a battle, preparing its map, graphics, units, audio, and main battle screen.
// FUNCTION: C2 0x4afd7
// FUNCTION: C2WIN 0x00472bc0
void do_fight_battle(int continuing)
{
    update_icon         = 0;
    pointer_mode        = 0;
    reg_placing_type    = 0;
    reg_placing_flags   = 0;
    placing_type        = 0;
    placing_flags       = 0;
    battle_state        = 0;
    nomansland_ptr      = 0x1380;
    battle_turbo        = 0;
    c2inf.paused        = 1;
    redraw_icons        = 1;

    if (continuing == 0) {
        return_map_mode   = map_mode;
        return_zoom_level = zoom_level;
        if (map_mode == 0) {
            city_pm_x       = pm_x;
            city_pm_y       = pm_y;
            city_direction  = map_direction;
        } else {
            region_pm_x     = pm_x;
            region_pm_y     = pm_y;
            region_direction = map_direction;
        }
        zoom_level    = 1;
        map_direction = 0;
        pm_x          = 0x1c;
        pm_y          = 0x38;
        map_mode      = 2;
        load_battle_graphics(1);
        refresh_battle_zoom_mode(zoom_level);
        get_pseudo_map(map_direction);
        generate_battle_map();
        setup_battle();
        figure_intelligence();
        battle_screen(1);
        init_battle_ambients();
    }

    play_tune("batest2.xmi", 1);

    while (battle_state < 4) {
        battle_game_loop();
        if (battle_state == 0xa) break;

        if (our_battle_men <= 0) {
            battle_state = 2;
            tune_mood    = 0x12;
            battle_over_count++;
        }
        if (their_battle_men <= 0) {
            battle_state = 2;
            tune_mood    = 0x11;
            battle_over_count++;
        }
        if (our_battle_morale <= 0) {
            battle_state = 2;
            tune_mood    = 0x12;
            battle_over_count++;
        }
        if (their_battle_morale <= 0) {
            battle_state = 2;
            tune_mood    = 0x11;
            battle_over_count++;
        }
        if (battle_over_count > 0x32)
            battle_state = 4;
        if (battle_over_count == 1) {
            if (tune_mood == 0x11) play_speech(3);
            else                   play_speech(4);
        }
    }

    pointer_mode                  = 0;
    c2inf.paused                   = 0;

    if (battle_state != 0xa) {
        zoom_level = return_zoom_level;
        map_mode   = return_map_mode;
        if (map_mode == 0) {
            pm_x          = city_pm_x;
            pm_y          = city_pm_y;
            map_direction = city_direction;
        } else {
            pm_x          = region_pm_x;
            pm_y          = region_pm_y;
            map_direction = region_direction;
        }
        get_pseudo_map(map_direction);
        stop_tune();
    }
}

// Select or toggle every figure belonging to the same unit as `figure_idx`.
// FUNCTION: C2 0x4b267
// FUNCTION: C2WIN 0x00472f8c
void select_a_unit(int figure_idx, int select_mode)
{
    int unit_ref;

    if (figure_list[figure_idx].owner == 0) {
        deselect_all_figures();
    } else {
        deselect_enemy_figures();
    }
    unit_ref = figure_list[figure_idx].unit_ref;
    for (figure_no = 1; figure_no < 0xc9; ++figure_no) {
        if (figure_list[figure_no].unit_ref == unit_ref) {
            battle_stats_type = 0;
            if (figure_list[figure_no].state_idx != 2) {
                if (select_mode == 0) {
                    figure_list[figure_no].selected ^= 1;
                } else {
                    figure_list[figure_no].selected = 1;
                }
            }
        }
    }
}

// Clear the .selected flag on every figure in figure_list[1..200].
// FUNCTION: C2 0x4b2ee
// FUNCTION: C2WIN 0x004730ae
void deselect_all_figures(void)
{
    for (figure_no = 1; figure_no < 201; figure_no++) {
        figure_list[figure_no].selected = 0;
    }
}

// Clear selection from every enemy figure.
// FUNCTION: C2 0x4b31c
// FUNCTION: C2WIN 0x004730fc
void deselect_enemy_figures(void)
{
    for (figure_no = 1; figure_no < 0xc9; ++figure_no) {
        if (figure_list[figure_no].owner == 0) {
            figure_list[figure_no].selected = 0;
        }
    }
}

// Select every active player-controlled figure.
// FUNCTION: C2 0x4b352
// FUNCTION: C2WIN 0x00473169
void select_all_figures(void)
{
    for (figure_no = 1; figure_no < 0xc9; ++figure_no) {
        if (figure_list[figure_no].exists != 0
         && figure_list[figure_no].owner != 0) {
            figure_list[figure_no].selected = 1;
        }
    }
}

// Select units inside the current battle-map drag rectangle.
// FUNCTION: C2 0x4b38f
// FUNCTION: C2WIN 0x004731f5
void select_drag_figures(void)
{
    int left_x;
    int x_bound;
    int first_y;
    int bottom_bound;
    int line_skip;
    int x;
    int y;
    int cell_off;
    int saved_x;
    unsigned char occupant;

    left_x = battle_drag_start_x;
    x_bound = act_start_x;
    first_y = battle_drag_start_y;
    bottom_bound = act_start_y;

    if (x_bound < left_x) { saved_x = left_x; left_x = x_bound; x_bound = saved_x; }
    if (first_y > bottom_bound) { saved_x = first_y; first_y = bottom_bound; bottom_bound = saved_x; }

    cell_off = (first_y * 0x34 + left_x) * 4;
    line_skip = (0x34 - ((x_bound - left_x) + 1)) * 4;

    for (y = first_y; y <= bottom_bound; y++, cell_off += line_skip) {
        for (x = left_x; x <= x_bound; x++, cell_off += 4) {
            occupant = ((unsigned char *)battle_map)[(cell_off) + 1];
            if (occupant != 0) {
                select_a_unit(occupant, 1);
            }
            else {
                ((unsigned char *)battle_map)[(cell_off) + 2] |= 2;
                ((unsigned char *)battle_map)[(cell_off) + 2] &= 0xf3;
                ((unsigned char *)battle_map)[(cell_off) + 2] |= 0xc;
            }
        }
    }
}

// Validate and draw the destination preview for selected moving units.
// FUNCTION: C2 0x4b438
// FUNCTION: C2WIN 0x00473345
int show_move_highlight(void)
{
  int cell_offset;
  int unit_idx;

  if (pm_over == 0)
    return;
  get_highlight_position();
  if (hlite_squares == 0)
    return;
  for (figure_no = 1; figure_no < 0xc9; ++figure_no)
  {
    if ((figure_list[figure_no].exists != 0) && (figure_list[figure_no].selected != 0))
    {
      unit_idx = figure_list[figure_no].unit_ref;
      if (unit_list[unit_idx].first_figure == figure_no)
      {
        unit_list[unit_idx].prev_x = unit_list[unit_idx].x + hlite_off_x;
        unit_list[unit_idx].prev_y = unit_list[unit_idx].y + hlite_off_y;
      }
      if (figure_list[figure_no].state_idx == 0xc)
        continue;
      if (unit_list[unit_idx].prev_x + figure_list[figure_no].offset_x >= 0x34) { hlite_squares = 0; return; }
      if (unit_list[unit_idx].prev_x + figure_list[figure_no].offset_x < 0) { hlite_squares = 0; return; }
      if (unit_list[unit_idx].prev_y + figure_list[figure_no].offset_y >= 0x34) { hlite_squares = 0; return; }
      if (unit_list[unit_idx].prev_y + figure_list[figure_no].offset_y < 0) { hlite_squares = 0; return; }
      cell_offset = ((unit_list[unit_idx].prev_x + figure_list[figure_no].offset_x) + (unit_list[unit_idx].prev_y + figure_list[figure_no].offset_y) * 0x34) * 4;
      if (cell_offset >= nomansland_ptr) { hlite_squares = 0; return; }
      figure_a = ((unsigned char *) battle_map)[cell_offset + 1];
      if (figure_a != 0)
      {
        if ((figure_list[figure_a].owner != 0) && (figure_list[figure_a].selected == 0)) { hlite_squares = 0; return; }
      }
    }
  }

  for (figure_no = 1; figure_no < 0xc9; ++figure_no)
  {
    if ((figure_list[figure_no].exists != 0) && (figure_list[figure_no].selected != 0))
    {
      if (figure_list[figure_no].state_idx == 0xc)
        continue;
      unit_idx = figure_list[figure_no].unit_ref;
      cell_offset = ((unit_list[unit_idx].prev_x + figure_list[figure_no].offset_x) + (figure_list[figure_no].offset_y + unit_list[unit_idx].prev_y) * 0x34) << 2;
      figure_a = ((unsigned char *) battle_map)[cell_offset + 1];
      ((unsigned char *) battle_map)[cell_offset + 2] |= 0x02;
      ((unsigned char *) battle_map)[cell_offset + 2] &= 0xf3;
      if ((figure_a != 0) && (figure_list[figure_no].owner != figure_list[figure_a].owner)) { ((unsigned char *) battle_map)[cell_offset + 2] |= 0x08; }
      else { ((unsigned char *) battle_map)[cell_offset + 2] |= 0x0c; }
    }
  }

  return;
}


// Highlight the target area for selected missile units.
// FUNCTION: C2 0x4b69f
// FUNCTION: C2WIN 0x0047388d
void show_aim_highlight(void)
{
    int eligible_count;
    int x0;
    int x1;
    int y0;
    int y1;
    int x;
    int y;
    int cell_offset;
    int row_skip;

    if (pm_over == 0) return;

    figure_no = 1; eligible_count = 0;
    for (; figure_no < 0xc9; ++figure_no) {
        if (figure_list[figure_no].exists != 0 && figure_list[figure_no].selected != 0) {
            temp_unit = figure_list[figure_no].unit_ref;
            if (unit_list[temp_unit].target_lock == 0) {
                if (unit_list[temp_unit].unit_sub_kind != 0) eligible_count++;
            }
        }
    }
    if (eligible_count == 0) return;

    x0 = act_start_x - 5; hlite_left = x0;
    y0 = act_start_y - 5; hlite_top  = y0;
    x1 = act_start_x + 5;
    y1 = act_start_y + 5;

    if (x0 < 0)    x0 = 0;
    if (x1 >= 0x34) x1 = 0x33;
    if (y0 < 0)    y0 = 0;
    if (y1 >= 0x34) y1 = 0x33;

    cell_offset = (y0 * 0x34 + x0) * 4;
    row_skip = (0x34 - (x1 - x0 + 1)) * 4;

    y = y0;
    for (; y <= y1; y++, cell_offset += row_skip) {
        x = x0;
        for (; x <= x1; x++, cell_offset += 4) {
            ((unsigned char *)battle_map)[(cell_offset) + 2] &= 0xf1;
            ((unsigned char *)battle_map)[(cell_offset) + 2] |= 0xe;
        }
    }
}

// Commit the highlighted destination for the selected units.
// FUNCTION: C2 0x4b7b2
// FUNCTION: C2WIN 0x00473acd
void start_move(void)
{
    int engaged_flag;
    int new_cell_offset;
    int unit_idx;
    engaged_flag = 0;
    if (hlite_squares == 0) {
        pointer_mode = 0;
        redraw_icons = 1;
    } else {
        /* Check whether any selected unit is already engaged. */
        for (figure_no = 1; figure_no < 0xc9; ++figure_no) {
            if (figure_list[figure_no].exists != 0
                && figure_list[figure_no].selected != 0) {
                unit_idx = figure_list[figure_no].unit_ref;
                if (unit_list[unit_idx].type == 0) {
                    deselect_all_figures();
                    return;
                }
                if (unit_list[unit_idx].target_lock != 0) {
                    engaged_flag = 1;
                    break;
                }
            }
        }
        if (engaged_flag) {
            confirm(4, 0xa0, 0xa0);
            if (decision == 0)
                return;
        }

        /* Clear the selected figures' current map cells during setup. */
        for (figure_no = 1; figure_no < 0xc9; ++figure_no) {
            if (figure_list[figure_no].exists != 0
                && figure_list[figure_no].selected != 0
                && battle_state == 0) {
                ((unsigned char *)battle_map)[figure_list[figure_no].map_ref + 1] = 0;
            }
        }

        /* Apply the new destination and movement state. */
        for (figure_no = 1; figure_no < 0xc9; ++figure_no) {
            if (figure_list[figure_no].exists != 0
                && figure_list[figure_no].selected != 0) {
                unit_idx = figure_list[figure_no].unit_ref;
                figure_list[figure_no].selected = 0;
                if (figure_list[figure_no].state_idx != 0xc) {
                    if (battle_state == 0) {
                        pointer_mode = 0;
                        update_map   = 1;
                        get_fig_in_unit_position(unit_list[unit_idx].formation_mode, figure_list[figure_no].unit_type, figure_no);
                        figure_list[figure_no].grid_x = (char)(unit_list[unit_idx].prev_x + x_bit);
                        figure_list[figure_no].grid_y = (char)(unit_list[unit_idx].prev_y + y_bit);
                        figure_list[figure_no].offset_x = x_bit;
                        figure_list[figure_no].offset_y = y_bit;
                        figure_list[figure_no].map_ref =
                            (figure_list[figure_no].grid_x +
                             figure_list[figure_no].grid_y * 0x34) * 4;
                        ((unsigned char *)battle_map)[figure_list[figure_no].map_ref + 1] = figure_no;
                    } else if (unit_list[unit_idx].target_lock != 0) {
                        pointer_mode = 0;
                        figure_list[figure_no].state_idx = 8;
                        get_fig_in_unit_position(unit_list[unit_idx].formation_mode, figure_list[figure_no].unit_type, figure_no);
                        figure_list[figure_no].prev_grid_x = (unit_list[unit_idx].prev_x + x_bit);
                        figure_list[figure_no].prev_grid_y = (unit_list[unit_idx].prev_y + y_bit);
                        figure_list[figure_no].offset_x    = x_bit;
                        figure_list[figure_no].offset_y    = y_bit;
                    } else {
                        pointer_mode = 0;
                        figure_list[figure_no].state_idx = 0xf;
                        figure_list[figure_no].is_visible &= 0xfd;
                        get_fig_in_unit_position(unit_list[unit_idx].formation_mode, figure_list[figure_no].unit_type, figure_no);
                        figure_list[figure_no].prev_grid_x = (unit_list[unit_idx].prev_x + x_bit);
                        figure_list[figure_no].prev_grid_y = (unit_list[unit_idx].prev_y + y_bit);
                        figure_list[figure_no].offset_x    = x_bit;
                        figure_list[figure_no].offset_y    = y_bit;
                    }
                }
            }
        }
    }
    return;
}

// Activate "aim" mode for every selected figure.
// FUNCTION: C2 0x4ba40
// FUNCTION: C2WIN 0x0047410c
void start_aim(void)
{
    int  hit_count;
    int  unit_idx;

    hit_count    = 0;
    pointer_mode = 0;

    for (figure_no = 1; figure_no < 0xc9; ++figure_no) {
        if (figure_list[figure_no].exists != 0
            && figure_list[figure_no].selected != 0) {
            unit_idx = figure_list[figure_no].unit_ref;
            if (unit_list[unit_idx].type == 0) {
                deselect_all_figures();
                return;
            }
            figure_list[figure_no].selected = 0;
            if (figure_list[figure_no].state_idx != 0xc
                && unit_list[unit_idx].target_lock == 0
                && unit_list[unit_idx].unit_sub_kind != 0)
            {
                unit_list[unit_idx].attack_marker_x = hlite_left;
                unit_list[unit_idx].attack_marker_y = hlite_top;
                figure_list[figure_no].state_idx = 0xb;
                figure_list[figure_no].prev_grid_x = figure_list[figure_no].grid_x;
                figure_list[figure_no].prev_grid_y = figure_list[figure_no].grid_y;
                hit_count++;
            }
        }
    }

    if (hit_count == 0) {
        pointer_mode = 0;
        redraw_icons = 1;
    }
}

// Compute the selected figures' bounding box and its offset from the cursor.
// FUNCTION: C2 0x4bb23
// FUNCTION: C2WIN 0x00474311
void get_highlight_position(void)
{
    hlite_centre_x = 0;
    hlite_centre_y = 0;
    hlite_off_x    = 0;
    hlite_off_y    = 0;
    hlite_off_ptr  = 0;
    hlite_squares  = 0;
    hlite_left     = 0x34;
    hlite_right    = 0;
    hlite_top      = 0x34;
    hlite_bottom   = 0;

    for (figure_no = 1; figure_no < 0xc9; ++figure_no) {
        if (figure_list[figure_no].exists == 0)    continue;
        if (figure_list[figure_no].selected == 0)  continue;
        if (figure_list[figure_no].state_idx == 0x0c) continue;

        if (figure_list[figure_no].grid_x < hlite_left)
            hlite_left = figure_list[figure_no].grid_x;
        if (hlite_right < figure_list[figure_no].grid_x)
            hlite_right = figure_list[figure_no].grid_x;

        if (figure_list[figure_no].grid_y < hlite_top)
            hlite_top = figure_list[figure_no].grid_y;
        if (hlite_bottom < figure_list[figure_no].grid_y)
            hlite_bottom = figure_list[figure_no].grid_y;

        hlite_squares++;
    }

    hlite_centre_x = (hlite_right  - hlite_left) / 2 + hlite_left;
    hlite_centre_y = (hlite_bottom - hlite_top)  / 2 + hlite_top;

    hlite_off_x    = act_start_x - hlite_centre_x;
    hlite_off_y    = act_start_y - hlite_centre_y;

    hlite_off_ptr  = hlite_off_x * 4;
    hlite_off_ptr += hlite_off_y * 208;
}

// Clear all highlight flags from the battle map.
// FUNCTION: C2 0x4bcbf
// FUNCTION: C2WIN 0x00474593
void clear_all_highlights_from_battlemap(void)
{
    gmn_y   = 0;
    cm_sptr = 0;
    for ( ; gmn_y < 0x34; gmn_y++) {
        gmn_x = 0;
        do {
            (*(struct battle_cell *)((unsigned char *)battle_map + ((cm_sptr)))).dirty &= 0xf3;
            gmn_x++;
            cm_sptr += 4;
        } while (gmn_x < 0x34);
    }
}

// Build an open-battle map with random terrain, standard dimensions, and a fresh pseudo-map.
// FUNCTION: C2 0x4bd13
// FUNCTION: C2WIN 0x00474618
void generate_battle_map(void)
{
    clear_all_bm(1);
    clear_all_bm(3);

    gmn_y   = 0;
    cm_sptr = 0;
    for ( ; gmn_y < 0x34; gmn_y++) {
        gmn_x = 0;
        do {
            random();
            (*(struct battle_cell *)((unsigned char *)battle_map + ((cm_sptr)))).terrain = rand128 & 0x1f;
            gmn_x++;
            cm_sptr += 4;
        } while (gmn_x < 0x34);
    }

    map_actual_width      = 0x34;
    map_actual_height     = 0x34;
    map_actual_atom       = 4;
    map_width_reduction   = 0xe;
    map_height_reduction  = 0xe;
    com_x                 = 0x1e0;
    com_y                 = 0x30;
    com_w                 = 0xa0;
    com_h                 = 0xa0;

    get_pseudo_map(map_direction);
}

// Initialise battle state for the combatants named in our_battle_army / their_battle_army.
// FUNCTION: C2 0x4bde3
// FUNCTION: C2WIN 0x00474716
void setup_battle(void)
{
    int our_men;
    int their_men;
    int total_men;

    total_men = army_list[our_battle_army].total_troops
              + army_list[their_battle_army].total_troops;

    battle_scale = 0;
    if (total_men >= 0x1c20) battle_scale = 8;
    else if (total_men >= 0xe10) battle_scale = 4;
    else if (total_men >= 0x708) battle_scale = 2;
    else if (total_men >= 0x384) battle_scale = 1;

    their_battle_routs            = 0;
    our_battle_routs              = 0;
    bat_attacker_clock            = 0;
    battle_ai_count               = 0;
    battle_npc_retreat_count      = 0;
    battle_over_count             = 0;
    bat_enemy_left_flank_unit     = 0;
    bat_enemy_right_flank_unit    = 0;
    bat_enemy_first_fan_unit      = 0;
    bat_enemy_second_fan_unit     = 0;
    bat_ai_trig_count             = 12;

    get_battle_odds();

    /* Re-read the troop totals after the odds calculation. */
    our_men   = army_list[our_battle_army].total_troops;
    their_men = army_list[their_battle_army].total_troops;

    their_battle_stance = 0;
    our_battle_stance   = 0;
    if (our_men   > (their_men + their_men / 3)) their_battle_stance = 1;
    if (their_men > (our_men   + our_men   / 3)) our_battle_stance   = 1;

    bat_no_selected = 0;
    retreat_flag    = 0;

    clear_unit_list();
    clear_figure_list();
    clear_arrow_list();
    setup_roman_units();
    setup_enemy_units();
    get_units_status();

    our_battle_start_men   = our_battle_men;
    their_battle_start_men = their_battle_men;
    which_unit             = 0;
}

// Snapshot troop counts from the two participating armies into the battle globals used by the
// setup UI and battle resolution code.
// FUNCTION: C2 0x4bf69
// FUNCTION: C2WIN 0x0047491f
void get_battle_men(void)
{
    our_battle_men      = army_list[our_battle_army].total_troops;
    their_battle_men    = army_list[their_battle_army].total_troops;
    our_battle_specials = army_list[our_battle_army].num_specials;
    our_battle_horse    = army_list[our_battle_army].num_horse;
    our_battle_regs     = army_list[our_battle_army].num_regulars;
    our_battle_irregs   = army_list[our_battle_army].num_irregulars;
    our_battle_auxs     = army_list[our_battle_army].num_auxillaries;
    their_battle_specials = army_list[their_battle_army].num_specials;
    their_battle_horse    = army_list[their_battle_army].num_horse;
    their_battle_regs     = army_list[their_battle_army].num_regulars;
    their_battle_irregs   = army_list[their_battle_army].num_irregulars;
    their_battle_auxs     = army_list[their_battle_army].num_auxillaries;
}

// Create and deploy the Roman units for the battle.
// FUNCTION: C2 0x4c016
// FUNCTION: C2WIN 0x00474a86
void setup_roman_units(void)
{
    int light_count;
    int heavy_count;
    int archer_count;
    int mercenary_count;
    int target_unit_size;
    int units_made;

    bat_which   = 0;
    bat_spacing = 1;
    bat_side    = -1;
    bat_control = 1;

    if (our_battle_stance != 0)
        find_attack_spot();
    else
        find_defensive_spot();

    heavy_count = army_list[our_battle_army].num_regulars;
    light_count = army_list[our_battle_army].num_irregulars;
    archer_count = army_list[our_battle_army].num_auxillaries;
    mercenary_count = army_list[our_battle_army].num_specials;

    if (battle_scale == 4)
        target_unit_size = 0x3c0;
    else if (battle_scale == 3)
        target_unit_size = 0x1e0;
    else if (battle_scale == 2)
        target_unit_size = 0xf0;
    else if (battle_scale == 1)
        target_unit_size = 0x78;
    else
        target_unit_size = 0x3c;

    units_made = 0;

    /* Stage 1: heavy infantry */
    while (target_unit_size / 12 <= heavy_count) {
        if (target_unit_size <= heavy_count)
            build_units_figures(units_made++, 1, 3, 0, 0, our_battle_stance,
                army_list[our_battle_army].morale, target_unit_size, 2, 1,
                figure1_data, 0, 1);
        else
            build_units_figures(units_made++, 1, 3, 0, 0, our_battle_stance,
                army_list[our_battle_army].morale, heavy_count, 2, 1,
                figure1_data, 0, 1);
        if (target_unit_size <= heavy_count)
            heavy_count -= target_unit_size;
        else
            heavy_count = 0;
    }

    /* Stage 2: light infantry */
    while (light_count >= target_unit_size / 12) {
        if (light_count >= target_unit_size)
            build_units_figures(units_made++, 2, 2, 0, 1, our_battle_stance,
                army_list[our_battle_army].morale, target_unit_size, 2, 1,
                figure2_data, 0, 2);
        else
            build_units_figures(units_made++, 2, 2, 0, 1, our_battle_stance,
                army_list[our_battle_army].morale, light_count, 2, 1,
                figure2_data, 0, 2);
        if (light_count >= target_unit_size)
            light_count -= target_unit_size;
        else
            light_count = 0;
    }

    /* Stage 3: archers */
    while (target_unit_size / 12 <= archer_count) {
        if (target_unit_size <= archer_count)
            build_units_figures(units_made++, 3, 2, 1, 2, our_battle_stance,
                army_list[our_battle_army].morale, target_unit_size, 2, 1,
                figure3_data, 0, 3);
        else
            build_units_figures(units_made++, 3, 2, 1, 2, our_battle_stance,
                army_list[our_battle_army].morale, archer_count, 2, 1,
                figure3_data, 0, 3);
        if (target_unit_size <= archer_count)
            archer_count -= target_unit_size;
        else
            archer_count = 0;
    }

    /* Stage 4: mercenary cavalry */
    while (target_unit_size / 12 <= mercenary_count) {
        if (target_unit_size <= mercenary_count)
            build_units_figures(units_made++, mercs_type, mercs_speed, mercs_missile,
                4, our_battle_stance, army_list[our_battle_army].morale,
                target_unit_size, 2, 1, figure7_data, figure8_data, 7);
        else
            build_units_figures(units_made++, mercs_type, mercs_speed, mercs_missile,
                4, our_battle_stance, army_list[our_battle_army].morale,
                mercenary_count, 2, 1, figure7_data, figure8_data, 7);
        if (target_unit_size <= mercenary_count)
            mercenary_count -= target_unit_size;
        else
            mercenary_count = 0;
    }
}

// Build the enemy army from its tribe's formation, unit-kind, and sprite-set configuration.
// FUNCTION: C2 0x4c399
// FUNCTION: C2WIN 0x00474f9f
void setup_enemy_units(void)
{
    int middle_figure_kind;
    int cavalry_unit_size;
    int front_figure_kind;
    int rear_stampede_kind;
    int rear_figure_kind;
    int rear_heavy_count;
    int middle_sprite_kind;
    int archer_unit_size;
    int front_sprite_kind;
    int cavalry_missile_flag;
    int front_unit_size;
    int rear_sprite_kind;
    int middle_heavy_count;
    int front_heavy_count;
    int archer_count;
    int cavalry_figure_kind;
    int cavalry_count;
    int middle_unit_size;
    unsigned char *sprite_data_ptr;
    int units_made;

    front_heavy_count = middle_heavy_count = rear_heavy_count = archer_count = cavalry_count = 0;
    front_heavy_count  = army_list[their_battle_army].num_regulars;
    middle_heavy_count = army_list[their_battle_army].num_irregulars;
    rear_heavy_count   = army_list[their_battle_army].num_auxillaries;
    cavalry_count      = army_list[their_battle_army].num_horse;
    archer_count       = army_list[their_battle_army].num_specials;

    if (battle_scale == 4) { front_unit_size = 0x500; middle_unit_size = 0x640; cavalry_unit_size = 0x1e0; archer_unit_size = 0xa0; }
    else if (battle_scale == 3) { front_unit_size = 0x280; middle_unit_size = 0x320; cavalry_unit_size = 0xf0; archer_unit_size = 0x50; }
    else if (battle_scale == 2) { front_unit_size = 0x140; middle_unit_size = 0x190; cavalry_unit_size = 0x78; archer_unit_size = 0x28; }
    else if (battle_scale == 1) { front_unit_size = 0xa0; middle_unit_size = 0xc8; cavalry_unit_size = 0x3c; archer_unit_size = 0x14; }
    else { front_unit_size = 0x50; middle_unit_size = 0x64; cavalry_unit_size = 0x1e; archer_unit_size = 0xa; }

    bat_tribe = army_list[their_battle_army].tribe_id;
    cavalry_figure_kind = tribe_battle_setup[bat_tribe].u.raw[0];
    front_figure_kind   = tribe_battle_setup[bat_tribe].u.raw[1];
    middle_figure_kind  = tribe_battle_setup[bat_tribe].u.raw[2];
    rear_figure_kind    = tribe_battle_setup[bat_tribe].u.raw[3];
    front_sprite_kind = tribe_battle_setup[bat_tribe].u.raw[4];
    middle_sprite_kind = tribe_battle_setup[bat_tribe].u.raw[5];
    rear_sprite_kind  = tribe_battle_setup[bat_tribe].u.raw[6];

    if (tribe_ai_data[bat_tribe].no_flanks == 0) {
        bat_enemy_left_flank_unit = bat_enemy_right_flank_unit = 1;
    }
    if (tribe_ai_data[bat_tribe].no_fans == 0) {
        bat_enemy_first_fan_unit = bat_enemy_second_fan_unit = 1;
    }

    bat_which = 0; bat_spacing = 3; bat_side = 1; bat_control = 0;

    if (their_battle_stance != 0) find_attack_spot();
    else find_defensive_spot();

    cavalry_missile_flag = (cavalry_figure_kind == 0xd);
    rear_stampede_kind   = (rear_figure_kind >= 9);
    units_made = 0;

    /* ---- Stage 1: front rank ---- */
    while (front_unit_size / 10 <= front_heavy_count) {
        if (front_sprite_kind == 2) sprite_data_ptr = figure5_data;
        else if (front_sprite_kind == 3) sprite_data_ptr = figure6_data;
        else sprite_data_ptr = figure4_data;
        if (front_unit_size <= front_heavy_count) build_units_figures(units_made++, front_figure_kind, 3, 0, 0, their_battle_stance, army_list[their_battle_army].morale, front_unit_size, 2, 1, sprite_data_ptr, 0, front_sprite_kind + 3);
        else build_units_figures(units_made++, front_figure_kind, 3, 0, 0, their_battle_stance, army_list[their_battle_army].morale, front_heavy_count, 2, 1, sprite_data_ptr, 0, front_sprite_kind + 3);
        if (front_unit_size <= front_heavy_count) front_heavy_count -= front_unit_size; else front_heavy_count = 0;
    }

    /* ---- Stage 2: middle rank ---- */
    while (middle_unit_size / 10 <= middle_heavy_count) {
        if (middle_sprite_kind == 2) sprite_data_ptr = figure5_data;
        else if (middle_sprite_kind == 3) sprite_data_ptr = figure6_data;
        else sprite_data_ptr = figure4_data;
        if (middle_unit_size <= middle_heavy_count) build_units_figures(units_made++, middle_figure_kind, 2, 0, 1, their_battle_stance, army_list[their_battle_army].morale, middle_unit_size, 2, 1, sprite_data_ptr, 0, middle_sprite_kind + 3);
        else build_units_figures(units_made++, middle_figure_kind, 2, 0, 1, their_battle_stance, army_list[their_battle_army].morale, middle_heavy_count, 2, 1, sprite_data_ptr, 0, middle_sprite_kind + 3);
        if (middle_unit_size <= middle_heavy_count) middle_heavy_count -= middle_unit_size; else middle_heavy_count = 0;
    }

    /* ---- Stage 3: rear rank ---- */
    while (middle_unit_size / 10 <= rear_heavy_count) {
        if (rear_sprite_kind == 2) sprite_data_ptr = figure5_data;
        else if (rear_sprite_kind == 3) sprite_data_ptr = figure6_data;
        else sprite_data_ptr = figure4_data;
        if (middle_unit_size <= rear_heavy_count) build_units_figures(units_made++, rear_figure_kind, 2, rear_stampede_kind, 2, their_battle_stance, army_list[their_battle_army].morale, middle_unit_size, 2, 1, sprite_data_ptr, 0, rear_sprite_kind + 3);
        else build_units_figures(units_made++, rear_figure_kind, 2, rear_stampede_kind, 2, their_battle_stance, army_list[their_battle_army].morale, rear_heavy_count, 2, 1, sprite_data_ptr, 0, rear_sprite_kind + 3);
        if (middle_unit_size <= rear_heavy_count) rear_heavy_count -= middle_unit_size; else rear_heavy_count = 0;
    }

    /* ---- Stage 4: mercenary cavalry ---- */
    while (cavalry_unit_size / 10 <= cavalry_count) {
        if (cavalry_count >= cavalry_unit_size) build_units_figures(units_made++, cavalry_figure_kind, 0, cavalry_missile_flag, 3, their_battle_stance, army_list[their_battle_army].morale, cavalry_unit_size, 1, 1, figure4_data, figure5_data, 4);
        else build_units_figures(units_made++, cavalry_figure_kind, 0, cavalry_missile_flag, 3, their_battle_stance, army_list[their_battle_army].morale, cavalry_count, 1, 1, figure4_data, figure5_data, 4);
        if (cavalry_count >= cavalry_unit_size) cavalry_count -= cavalry_unit_size; else cavalry_count = 0;
    }

    /* ---- Stage 5: archers ---- */
    while (archer_unit_size / 5 <= archer_count) {
        if (archer_count >= archer_unit_size) build_units_figures(units_made++, 0xf, 2, 0, 4, their_battle_stance, army_list[their_battle_army].morale, archer_unit_size, 1, 2, figure4_data, 0, 4);
        else build_units_figures(units_made++, 0xf, 2, 0, 4, their_battle_stance, army_list[their_battle_army].morale, 5, 1, 2, figure4_data, 0, 4);
        if (archer_count >= archer_unit_size) archer_count -= archer_unit_size; else archer_count = 0;
    }
}

// Re-tally per-rank HP totals into each side's army_list record.
// FUNCTION: C2 0x4c9c0
// FUNCTION: C2WIN 0x00475835
void get_battle_centuries_left(void)
{

    army_list[our_battle_army].num_specials = 0; army_list[our_battle_army].num_horse = 0; army_list[our_battle_army].num_regulars = 0; army_list[our_battle_army].num_irregulars = 0; army_list[our_battle_army].num_auxillaries = 0;
    army_list[their_battle_army].num_specials = 0; army_list[their_battle_army].num_horse = 0; army_list[their_battle_army].num_regulars = 0; army_list[their_battle_army].num_irregulars = 0; army_list[their_battle_army].num_auxillaries = 0;

    for (temp_figure = 1; temp_figure < 0xc9; temp_figure++) {
        if (figure_list[temp_figure].exists == 0) continue;

        if (figure_list[temp_figure].owner != 0) {

            if (figure_list[temp_figure].figure_rank == 4)      army_list[our_battle_army].num_specials    += figure_list[temp_figure].stampede_flag;
            else if (figure_list[temp_figure].figure_rank == 3) army_list[our_battle_army].num_horse       += figure_list[temp_figure].stampede_flag;
            else if (figure_list[temp_figure].figure_rank == 0) army_list[our_battle_army].num_regulars    += figure_list[temp_figure].stampede_flag;
            else if (figure_list[temp_figure].figure_rank == 1) army_list[our_battle_army].num_irregulars  += figure_list[temp_figure].stampede_flag;
            else if (figure_list[temp_figure].figure_rank == 2) army_list[our_battle_army].num_auxillaries += figure_list[temp_figure].stampede_flag;
        }
        else {

            if (figure_list[temp_figure].figure_rank == 4)      army_list[their_battle_army].num_specials    += figure_list[temp_figure].stampede_flag;
            else if (figure_list[temp_figure].figure_rank == 3) army_list[their_battle_army].num_horse       += figure_list[temp_figure].stampede_flag;
            else if (figure_list[temp_figure].figure_rank == 0) army_list[their_battle_army].num_regulars    += figure_list[temp_figure].stampede_flag;
            else if (figure_list[temp_figure].figure_rank == 1) army_list[their_battle_army].num_irregulars  += figure_list[temp_figure].stampede_flag;
            else if (figure_list[temp_figure].figure_rank == 2) army_list[their_battle_army].num_auxillaries += figure_list[temp_figure].stampede_flag;
        }
    }

    army_list[our_battle_army].total_troops = army_list[our_battle_army].num_auxillaries + army_list[our_battle_army].num_irregulars + army_list[our_battle_army].num_regulars + army_list[our_battle_army].num_horse + army_list[our_battle_army].num_specials;
    army_list[their_battle_army].total_troops = army_list[their_battle_army].num_auxillaries + army_list[their_battle_army].num_irregulars + army_list[their_battle_army].num_regulars + army_list[their_battle_army].num_horse + army_list[their_battle_army].num_specials;
}

// Compute the battle-odds rating into bat_odds (+5 = we vastly outnumber them, 0 = roughly equal,
// -5 = they vastly outnumber us) from their_battle_men and our_battle_men, then — unless
// tune_mood_hold is set — fold that into tune_mood (1..5).
// FUNCTION: C2 0x4cbd9
// FUNCTION: C2WIN 0x00475e2d
void get_battle_odds(void)
{
    if      (their_battle_men * 4 < our_battle_men)                      bat_odds =  5;
    else if (their_battle_men * 3 < our_battle_men)                      bat_odds =  4;
    else if (their_battle_men * 2 < our_battle_men)                      bat_odds =  3;
    else if (their_battle_men + their_battle_men / 2 < our_battle_men)   bat_odds =  2;
    else if (their_battle_men + their_battle_men / 4 < our_battle_men)   bat_odds =  1;
    else if (our_battle_men   * 4 < their_battle_men)                    bat_odds = -5;
    else if (our_battle_men   * 3 < their_battle_men)                    bat_odds = -4;
    else if (our_battle_men   * 2 < their_battle_men)                    bat_odds = -3;
    else if (our_battle_men   + our_battle_men   / 2 < their_battle_men) bat_odds = -2;
    else if (our_battle_men   + our_battle_men   / 4 < their_battle_men) bat_odds = -1;
    else                                                                 bat_odds =  0;

    if (tune_mood_hold != 0) return;

    if      (bat_odds >=  4) tune_mood = 5;
    else if (bat_odds >=  2) tune_mood = 4;
    else if (bat_odds <= -4) tune_mood = 3;
    else if (bat_odds <= -2) tune_mood = 2;
    else                     tune_mood = 1;
}

// Create a battle unit and place its figures in the current deployment slot.
// FUNCTION: C2 0x4cd76
// FUNCTION: C2WIN 0x0047605c
void build_units_figures(int deployment_idx, int figure_kind, int stampede_kind,
                         int unit_sub_kind, int unit_rank, int stance,
                         int army_morale, int target_men, int formation_cols,
                         int row_spacing, unsigned char *arrow_data_ptr,
                         unsigned char *sprite_data_ptr, int sprite_kind)
{
    signed char morale_adjustment;
    int i;

    random();

    if      (battle_scale == 4) bat_size = target_men / 0x50;
    else if (battle_scale == 3) bat_size = target_men / 0x28;
    else if (battle_scale == 2) bat_size = target_men / 0x14;
    else if (battle_scale == 1) bat_size = target_men / 0xa;
    else                        bat_size = target_men / 5;

    if (bat_size < 0xf && formation_cols == 4) formation_cols = 3;
    if (bat_size < 0xa && formation_cols == 3) formation_cols = 2;
    if (bat_size < 5   && formation_cols == 2) formation_cols = 1;

    bat_width = bat_size / formation_cols * row_spacing;

    get_start_points(deployment_idx);
    create_unit(figure_kind, x, y, bat_control);

    unit_list[created_unit_no].morale_a = army_morale * 10 + 0x32;
    morale_adjustment = stampede_kind;
    unit_list[created_unit_no].unit_sub_kind = unit_sub_kind;
    unit_list[created_unit_no].stage_slot = stance; morale_adjustment = bat_odds * 5;
    if (bat_control != 0) unit_list[created_unit_no].morale_a += morale_adjustment;
    else unit_list[created_unit_no].morale_a -= morale_adjustment;
    if (unit_list[created_unit_no].morale_a >= 0x64) unit_list[created_unit_no].morale_a = 0x64;
    if (unit_list[created_unit_no].morale_a < 0x19) unit_list[created_unit_no].morale_a = 0x19;
    unit_list[created_unit_no].morale_b = unit_list[created_unit_no].morale_a;
    unit_list[created_unit_no].formation_width = bat_width;
    unit_list[created_unit_no].formation_cols = formation_cols;
    unit_list[created_unit_no]._init32 = 1;
    unit_list[created_unit_no].formation_mode = 0;
    unit_list[created_unit_no].heading = bat_side;
    unit_list[created_unit_no].start_men = target_men; unit_list[created_unit_no].current_men = target_men;
    if (bat_control != 0) unit_list[created_unit_no].ai_period = 0;
    else {
        unit_list[created_unit_no].ai_period = bat_ai_trig_count;
        bat_ai_trig_count += (rand128 & 7) + 6;
    }
    unit_list[created_unit_no].ai_tick = 0;
    unit_list[created_unit_no].unit_rank = unit_rank;

    /* Assign eligible enemy units to flank or fan manoeuvres. */
    if (bat_control == 0 && unit_rank == 1) {
        if (bat_enemy_left_flank_unit == 0 && x <= 0x1a) {
            unit_list[created_unit_no].flank_pending = 1; bat_enemy_left_flank_unit = unit_rank;
        }
        if (bat_enemy_right_flank_unit == 0 && x > 0x1a) {
            unit_list[created_unit_no].flank_pending = 2; bat_enemy_right_flank_unit = 1;
        }
        if (unit_list[created_unit_no].flank_pending == 0) {
            if (bat_enemy_first_fan_unit == 0) {
                unit_list[created_unit_no].flank_pending = 4; bat_enemy_first_fan_unit = 1;
            } else if (bat_enemy_second_fan_unit == 0) {
                unit_list[created_unit_no].flank_pending = 3; bat_enemy_second_fan_unit = 1;
            }
        }
    }

    /* Create and configure the unit's figures. */
    for (i = 0; i < bat_size; i++) {
        random();
        x_bit = get_x_spacing(row_spacing, formation_cols, i);
        y_bit = get_y_spacing(row_spacing, formation_cols, i, bat_side);

        if (create_figure(figure_kind, x, x_bit, y, y_bit, bat_control,
                          created_unit_no) == 0)
            break;

        figure_list[created_figure_no].state_idx     = 6;
        figure_list[created_figure_no].unit_position = bat_side;
        figure_list[created_figure_no].figure_rank   = unit_rank;
        figure_list[created_figure_no].unit_grid_x   = row_spacing;
        figure_list[created_figure_no].unit_grid_y   = formation_cols;
        figure_list[created_figure_no].fight_swing_active = unit_sub_kind;

        if (bat_control == 0) figure_list[created_figure_no].morale = tribe_ai_data[bat_tribe].aggression;
        if (figure_list[created_figure_no].figure_rank == 1) figure_list[created_figure_no].morale = figure_list[created_figure_no].morale / 2;
        if (figure_list[created_figure_no].figure_rank == 2) figure_list[created_figure_no].morale = 0;

        figure_list[created_figure_no].stampede_kind = stampede_kind;
        figure_list[created_figure_no].is_defending = 1;
        figure_list[created_figure_no].shield_class = 0;

        if      (battle_scale == 0) figure_list[created_figure_no].stampede_flag = 5;
        else if (battle_scale == 1) figure_list[created_figure_no].stampede_flag = 0xa;
        else if (battle_scale == 2) figure_list[created_figure_no].stampede_flag = 0x14;
        else if (battle_scale == 3) figure_list[created_figure_no].stampede_flag = 0x28;
        else if (battle_scale == 4) figure_list[created_figure_no].stampede_flag = 0x50;

        figure_list[created_figure_no].arrow_data_ptr = arrow_data_ptr;
        figure_list[created_figure_no].sprite_data_ptr = sprite_data_ptr;
        figure_list[created_figure_no].sprite_kind = sprite_kind;

        if (sprite_data_ptr != 0) figure_list[created_figure_no].fight_state = 1;
        else if (figure_kind == 0xf) figure_list[created_figure_no].fight_state = 2;
        figure_list[created_figure_no].missile_timer = rand128 & 0x1f;

        if (i == 0) unit_list[created_unit_no].first_figure = created_figure_no;
        unit_list[created_unit_no].fig_count++;
    }

    unit_list[created_unit_no].last_figure = created_figure_no;
}

// Rebind every live figure and projectile to the sprite tables loaded for the current map and zoom.
// FUNCTION: C2 0x4d272
// FUNCTION: C2WIN 0x004769af
void rebuild_figures_image_data(void)
{
    unsigned char *figure_data_ptr;
    unsigned char *arrow_data_ptr;
    int sprite_kind;

    for (figure_no = 1; figure_no < 0xc9; ++figure_no) {
        if (figure_list[figure_no].exists == 0) continue;
        sprite_kind = figure_list[figure_no].sprite_kind;
        if (sprite_kind == 7) {
            figure_list[figure_no].arrow_data_ptr = figure7_data;
            if (figure_list[figure_no].sprite_data_ptr != 0)
                figure_list[figure_no].sprite_data_ptr = figure8_data;
        } else if (sprite_kind == 1) {
            figure_list[figure_no].arrow_data_ptr = figure1_data;
        } else if (sprite_kind == 2) {
            figure_list[figure_no].arrow_data_ptr = figure2_data;
        } else if (sprite_kind == 3) {
            figure_list[figure_no].arrow_data_ptr = figure3_data;
        } else if (sprite_kind == 4) {
            figure_list[figure_no].arrow_data_ptr = figure4_data;
            if (figure_list[figure_no].sprite_data_ptr != 0)
                figure_list[figure_no].sprite_data_ptr = figure5_data;
        } else if (sprite_kind == 5) {
            figure_list[figure_no].arrow_data_ptr = figure5_data;
        } else if (sprite_kind == 6) {
            figure_list[figure_no].arrow_data_ptr = figure6_data;
        }
    }

    for (arrow_no = 1; arrow_no < 0xc9; arrow_no++) {
        if (arrow_list[arrow_no].exists == 0) continue;
        sprite_kind = arrow_list[arrow_no].sprite_kind;
        if (sprite_kind == 1) arrow_list[arrow_no].arrow_data_ptr = figure1_data;
        else if (sprite_kind == 2) arrow_list[arrow_no].arrow_data_ptr = figure2_data;
        else if (sprite_kind == 3) arrow_list[arrow_no].arrow_data_ptr = figure3_data;
        else if (sprite_kind == 4) arrow_list[arrow_no].arrow_data_ptr = figure4_data;
        else if (sprite_kind == 5) arrow_list[arrow_no].arrow_data_ptr = figure5_data;
        else if (sprite_kind == 6) arrow_list[arrow_no].arrow_data_ptr = figure6_data;
        else if (sprite_kind == 7) arrow_list[arrow_no].arrow_data_ptr = figure7_data;
        else if (sprite_kind == 8) arrow_list[arrow_no].arrow_data_ptr = figure8_data;
    }
}

// Initialize the deployment lanes for an attacking army.
// FUNCTION: C2 0x4d404
// FUNCTION: C2WIN 0x00476d34
void find_attack_spot(void)
{
    first_rear = 1;
    first_back = 1;
    first_front = 1;
    xright_rear = 0x1a;
    xleft_rear = 0x1a;
    xright_back = 0x1a;
    xleft_back = 0x1a;
    xright_front = 0x1a;
    xleft_front = 0x1a;
    if (bat_side == -1) {
        yfront = 0x12;
        yback = 0xe;
        yback = 0xa;
    } else {
        yfront = 0x22;
        yback = 0x26;
        yrear = 0x2a;
    }
}

// Initialize the deployment lanes for a defending army.
// FUNCTION: C2 0x4d491
// FUNCTION: C2WIN 0x00476de7
void find_defensive_spot(void)
{
    first_rear = 1;
    first_back = 1;
    first_front = 1;
    xright_rear = 0x1a;
    xleft_rear = 0x1a;
    xright_back = 0x1a;
    xleft_back = 0x1a;
    xright_front = 0x1a;
    xleft_front = 0x1a;
    if (bat_side == -1) {
        yfront = 0xe;
        yback = 0xa;
        yrear = 6;
    } else {
        yfront = 0x26;
        yback = 0x2a;
        yrear = 0x2e;
    }
}

// Pick the next (x, y) deployment slot for a unit being placed on the battle map.
// FUNCTION: C2 0x4d51e
// FUNCTION: C2WIN 0x00476e9a
void get_start_points(int deployment_idx)
{
    xpos = attack_pos_data[deployment_idx].xpos;
    ypos = attack_pos_data[deployment_idx].ypos;

    if (ypos == 0 && first_front != 0) {
        first_front = 0;
        x = xleft_front;
        y = yfront;
        xright_front += bat_width + bat_spacing;
        return;
    }
    if (ypos == 1 && first_back != 0) {
        first_back = 0;
        x = xleft_back;
        y = yback;
        xright_back += bat_width + bat_spacing;
        return;
    }
    if (ypos == 2 && first_rear != 0) {
        first_rear = 0;
        x = xleft_rear;
        y = yrear;
        xright_rear += bat_width + bat_spacing;
        return;
    }

    if (ypos == 0) {
        y = yfront;
        if (xpos == 1 && xright_front + bat_width >= 0x34) xpos = 0;
        else if (xpos == 0 && xleft_front - bat_width - bat_spacing < 0) xpos = 1;
        if (xpos == 1) {
            x = xright_front;
            xright_front += bat_width + bat_spacing;
        } else {
            x = xleft_front - bat_width - bat_spacing;
            xleft_front = x;
        }
    } else if (ypos == 1) {
        y = yback;
        if (xpos == 1 && xright_back + bat_width >= 0x34) xpos = 0;
        else if (xpos == 0 && xleft_back - bat_width - bat_spacing < 0) xpos = 1;
        if (xpos == 1) {
            x = xright_back;
            xright_back += bat_width + bat_spacing;
        } else {
            x = xleft_back - bat_width - bat_spacing;
            xleft_back = x;
        }
    } else { /* ypos == 2 */
        y = yrear;
        if (xpos == 1 && xright_rear + bat_width >= 0x34) xpos = 0;
        else if (xpos == 0 && xleft_rear - bat_width - bat_spacing < 0) xpos = 1;
        if (xpos == 1) {
            x = xright_rear;
            xright_rear += bat_width + bat_spacing;
        } else {
            x = xleft_rear - bat_width - bat_spacing;
            xleft_rear = x;
        }
    }
}

// Return a figure's horizontal offset within a formation.
// FUNCTION: C2 0x4d7d3
// FUNCTION: C2WIN 0x004771f6
int get_x_spacing(int row_spacing, int formation_cols, int position)
{
    if (formation_cols <= 1)
        return position * row_spacing;
    if (formation_cols <= 2)
        return (position / 2) * row_spacing;
    if (formation_cols <= 3)
        return (position / 3) * row_spacing;
    else
        return (position / 4) * row_spacing;
}

// Return a figure's vertical offset within a formation.
// FUNCTION: C2 0x4d821
// FUNCTION: C2WIN 0x0047726b
int get_y_spacing(int row_spacing, int formation_cols, int position, int side)
{
    if (formation_cols <= 1)
        return 0;
    if (formation_cols <= 2)
        return (position % 2) * row_spacing * side;
    if (formation_cols <= 3)
        return (position % 3) * row_spacing * side;
    else
        return (position % 4) * row_spacing * side;
}

// Count active figures and mark their map footprints for redraw.
// FUNCTION: C2 0x4d861
// FUNCTION: C2WIN 0x004772f2
void figure_update(void)
{
    int refresh_radius;
    int direction_offset;

    no_of_figures = 0;

    for (figure_no = 1; figure_no < 0xc9; ++figure_no) {
        if (figure_list[figure_no].exists != 0) {
            no_of_figures++;
            if (figure_list[figure_no].sprite_type == 0xf) {
                refresh_radius = 3;
                direction_offset = 3;
            } else if (figure_list[figure_no].fight_state != 0) {
                refresh_radius = 2;
                direction_offset = 2;
            } else {
                refresh_radius = 2;
                direction_offset = 2;
            }
            set_figure_map_refresh(figure_list[figure_no].grid_x,
                                   figure_list[figure_no].grid_y,
                                   0, 0, refresh_radius, 0);

            if (map_direction == 0)
                set_figure_map_refresh(figure_list[figure_no].grid_x,
                                       figure_list[figure_no].grid_y,
                                       -direction_offset, -direction_offset, direction_offset, 1);
            else if (map_direction == 2)
                set_figure_map_refresh(figure_list[figure_no].grid_x,
                                       figure_list[figure_no].grid_y,
                                        direction_offset, -direction_offset, direction_offset, 1);
            else if (map_direction == 4)
                set_figure_map_refresh(figure_list[figure_no].grid_x,
                                       figure_list[figure_no].grid_y,
                                        direction_offset,  direction_offset, direction_offset, 1);
            else if (map_direction == 6)
                set_figure_map_refresh(figure_list[figure_no].grid_x,
                                       figure_list[figure_no].grid_y,
                                       -direction_offset,  direction_offset, direction_offset, 1);

            if (figure_list[figure_no].selected != 0) {
                temp_unit = (short)figure_list[figure_no].unit_ref;
                if (pointer_mode == 2 && unit_list[temp_unit].unit_sub_kind == 0) {
                    figure_list[figure_no].selected = 0;
                } else {
                    ((unsigned char *)battle_map)[figure_list[figure_no].map_ref + 2] |= 4;
                }
            }
        }
    }
}

// Clear projectile occupancy and mark active projectiles' map areas for redraw.
// FUNCTION: C2 0x4da05
// FUNCTION: C2WIN 0x0047763d
void arrow_update(void)
{
    for (arrow_no = 1; arrow_no < 0xc9; arrow_no++) {
        if (arrow_list[arrow_no].exists != 0) {
            ((unsigned char *)battle_map)[(arrow_list[arrow_no].map_ref) + 3] = 0;
            if (map_direction == 0)
                set_figure_map_refresh(arrow_list[arrow_no].grid_x,
                                       arrow_list[arrow_no].grid_y,
                                       -2, -2, 2, 1);
            else if (map_direction == 2)
                set_figure_map_refresh(arrow_list[arrow_no].grid_x,
                                       arrow_list[arrow_no].grid_y,
                                        2, -2, 2, 1);
            else if (map_direction == 4)
                set_figure_map_refresh(arrow_list[arrow_no].grid_x,
                                       arrow_list[arrow_no].grid_y,
                                        2,  2, 2, 1);
            else if (map_direction == 6)
                set_figure_map_refresh(arrow_list[arrow_no].grid_x,
                                       arrow_list[arrow_no].grid_y,
                                       -2,  2, 2, 1);
        }
    }
}

// Updates every active battle figure for the current simulation tick.
// FUNCTION: C2 0x4dae8
// FUNCTION: C2WIN 0x004777c2
void figure_intelligence(void)
{
    for (figure_no = 1; figure_no < 0xc9; ++figure_no) {
        if (figure_list[figure_no].exists != 0) {
            if (figure_list[figure_no].engaged_count != 0) {
                figure_list[figure_no].engaged_count--;
            }
            if (figure_list[figure_no].sprite_type <= 0
             || figure_list[figure_no].sprite_type >= 0x12) {
                remove_figure(figure_no);
            } else {
                figure_intelligences[figure_list[figure_no].sprite_type]();
            }
        }
    }
}

// Refresh the still-frame sprite for every active figure.
// FUNCTION: C2 0x4db5e
// FUNCTION: C2WIN 0x004778bf
void figure_images(void)
{
    for (figure_no = 1; figure_no < 0xc9; ++figure_no) {
        if (figure_list[figure_no].exists != 0) {
            get_fig_still_image();
        }
    }
}

// Provide the unused figure-type handler.
// FUNCTION: C2 0x4db8f
// FUNCTION: C2WIN 0x0047791a
void f00_null(void)
{
}

// Initialize a regular soldier's animation and run its current state handler.
// FUNCTION: C2 0x4db90
// FUNCTION: C2WIN 0x00477925
void f01_regular(void)
{
    figure_list[figure_no].anim_kind = 0xf;
    figure_list[figure_no].sub_state = 3;
    figure_states[figure_list[figure_no].state_idx]();
}

// Irregular-troop figure-state init (anim_kind = 0xa, sub_state = 2).
// FUNCTION: C2 0x4dbaa
// FUNCTION: C2WIN 0x0047797c
void f02_irregular(void)
{
    figure_list[figure_no].anim_kind = 0xa;
    figure_list[figure_no].sub_state = 2;
    figure_states[figure_list[figure_no].state_idx]();
}

// Initialize an auxiliary soldier's animation and run its current state handler.
// FUNCTION: C2 0x4dbc4 FOLDED
void f03_auxillary(void)
{
    figure_list[figure_no].anim_kind = 4;
    figure_list[figure_no].sub_state = 1;
    figure_states[figure_list[figure_no].state_idx]();
}

// Empty state-handler slot in `figure_states[]` (state_idx 0).
// FUNCTION: C2 0x4dbea
void sf00_null(void)
{
}

// Initializes a barbarian swordsman's animation state and dispatches its current state handler.
// FUNCTION: C2 0x4dbeb
// FUNCTION: C2WIN 0x00477a2a
void f05_barb_sword(void)
{
    figure_list[figure_no].anim_kind = 0xe;
    figure_list[figure_no].sub_state = 3;
    figure_states[figure_list[figure_no].state_idx]();
}

// Barbarian-spear figure-state init (anim_kind = 0xc, sub_state = 2).
// FUNCTION: C2 0x4dbfe
// FUNCTION: C2WIN 0x00477a81
void f06_barb_spear(void)
{
    figure_list[figure_no].anim_kind = 0xc;
    figure_list[figure_no].sub_state = 2;
    figure_states[figure_list[figure_no].state_idx]();
}

// Barbarian-axe figure-state init (anim_kind = 0x10, sub_state = 1).
// FUNCTION: C2 0x4dc11
// FUNCTION: C2WIN 0x00477ad8
void f07_barb_axe(void)
{
    figure_list[figure_no].anim_kind = 0x10;
    figure_list[figure_no].sub_state = 1;
    figure_states[figure_list[figure_no].state_idx]();
}

// Barbarian-pike figure-state init (anim_kind = 0xa, sub_state = 5).
// FUNCTION: C2 0x4dc24
// FUNCTION: C2WIN 0x00477b2f
void f08_barb_pike(void)
{
    figure_list[figure_no].anim_kind = 0xa;
    figure_list[figure_no].sub_state = 5;
    figure_states[figure_list[figure_no].state_idx]();
}

// Initialize a barbarian javelin soldier and run its current state handler.
// FUNCTION: C2 0x4dbc4 FOLDED
void f09_barb_javalin(void)
{
    figure_list[figure_no].anim_kind = 4;
    figure_list[figure_no].sub_state = 1;
    figure_states[figure_list[figure_no].state_idx]();
}

// Initialize a barbarian slinger and run its current state handler.
// FUNCTION: C2 0x4dbc4 FOLDED
void f10_barb_sling(void)
{
    figure_list[figure_no].anim_kind = 4;
    figure_list[figure_no].sub_state = 1;
    figure_states[figure_list[figure_no].state_idx]();
}

// Heavy barbarian-cavalry figure-state init (anim_kind = 0x10, sub_state = 5).
// FUNCTION: C2 0x4dc3e
// FUNCTION: C2WIN 0x00477c34
void f11_barb_horse_heavy(void)
{
    figure_list[figure_no].anim_kind = 0x10;
    figure_list[figure_no].sub_state = 5;
    figure_states[figure_list[figure_no].state_idx]();
}

// Initialize light barbarian cavalry and run its current state handler.
// FUNCTION: C2 0x4dc51 FOLDED
// FUNCTION: C2WIN 0x00477c8b
void f12_barb_horse_light(void)
{
    figure_list[figure_no].anim_kind = 0xe;
    figure_list[figure_no].sub_state = 4;
    figure_states[figure_list[figure_no].state_idx]();
}

// Initialize a mounted archer and run its current state handler.
// FUNCTION: C2 0x4dc51 FOLDED
// FUNCTION: C2WIN 0x00477ce2
void f13_barb_horse_archer(void)
{
    figure_list[figure_no].anim_kind = 0xe;
    figure_list[figure_no].sub_state = 4;
    figure_states[figure_list[figure_no].state_idx]();
}

// Initialize a camel rider and run its current state handler.
// FUNCTION: C2 0x4dc51 FOLDED
// FUNCTION: C2WIN 0x00477d39
void f14_barb_camel(void)
{
    figure_list[figure_no].anim_kind = 0xe;
    figure_list[figure_no].sub_state = 4;
    figure_states[figure_list[figure_no].state_idx]();
}


// Initializes a barbarian elephant's animation state and dispatches its current state handler.
// FUNCTION: C2 0x4dc6e
// FUNCTION: C2WIN 0x00477d90
void f15_barb_elephant(void)
{
    figure_list[figure_no].anim_kind = 0x14;
    figure_list[figure_no].sub_state = 6;
    figure_states[figure_list[figure_no].state_idx]();
    elephant_fire();
}

// Initializes a barbarian archer's animation state and dispatches its current state handler.
// FUNCTION: C2 0x4dcca
// FUNCTION: C2WIN 0x00477dec
void f16_barb_bow(void)
{
    figure_list[figure_no].anim_kind = 5;
    figure_list[figure_no].sub_state = 1;
    figure_states[figure_list[figure_no].state_idx]();
}

// Initialize a barbarian knife fighter and run its current state handler.
// FUNCTION: C2 0x4dbc4 FOLDED
void f17_barb_knife(void)
{
    figure_list[figure_no].anim_kind = 4;
    figure_list[figure_no].sub_state = 1;
    figure_states[figure_list[figure_no].state_idx]();
}

// Hold a waiting figure still, then advance it to its queued state.
// FUNCTION: C2 0x4dce0
// FUNCTION: C2WIN 0x00477ea5
void sf01_wait(void)
{
    get_fig_still_image();
    if (cnt4 != 0) return;
    if (--figure_list[figure_no].wait_counter > 0) return;

    figure_list[figure_no].wait_counter = 5;
    figure_list[figure_no].wf_step_y    = 0;
    figure_list[figure_no].wf_step_x    = 0;
    figure_list[figure_no].is_routing   = 0;
    figure_list[figure_no].state_idx    = figure_list[figure_no].next_state_idx;
#if C2_FEAT_WAIT_KEEP_VISIBLE
    figure_list[figure_no].is_visible  &= 0xfd;
#else
    figure_list[figure_no].is_visible  &= 0xfc;
#endif
    figure_list[figure_no].is_visible  |= 1;
}

// Animate and remove a dead figure, or send a dying elephant into a stampede.
// FUNCTION: C2 0x4dd4a
// FUNCTION: C2WIN 0x00477fe7
void sf02_death(void)
{
    int sprite_type;
    signed char death_count;
    unsigned char terrain;
    unsigned char stampede_value = 1;
    int terrain_value;

    sprite_type = figure_list[figure_no].sprite_type;
    if (sprite_type == 0xf) {
        int stampede_idx;
        figure_list[figure_no].stampede_kind = stampede_value;
        figure_list[figure_no].stampede_flag = stampede_value;
        stampede_idx = figure_no & 7;
        figure_list[figure_no].prev_grid_x = elephant_stampede[stampede_idx].dx;
        figure_list[figure_no].prev_grid_y = elephant_stampede[stampede_idx].dy;
        sf12_rout();
        if (figure_list[figure_no].death_timer <= 0) {
            set_battle_death_fx(figure_list[figure_no].sprite_type);
        }
        ++figure_list[figure_no].death_timer;
        if (figure_list[figure_no].death_timer > 0x40)
            figure_list[figure_no].death_timer =
                ((unsigned char)rand8 + (unsigned char)rand8);
        return;
    }

    if (figure_list[figure_no].death_timer <= 0)
        set_battle_death_fx(sprite_type);

    get_fig_death_image();
    ++figure_list[figure_no].death_timer;
    if (figure_list[figure_no].death_timer <= 0x1e)
        return;

    figure_list[figure_no].death_timer = 0x1e;
    terrain  = (*(struct battle_cell *)((unsigned char *)battle_map + ((figure_list[figure_no].map_ref)))).terrain;
    terrain_value = terrain;
    if (terrain_value < 0x28) {
        if (terrain_value >= 0x24) {
            terrain = terrain + 4;
        } else if (terrain_value >= 0x20) {
            terrain = terrain + 4;
        } else {
            terrain = (unsigned char)((terrain & 3) + 0x24);
        }
    }
    (*(struct battle_cell *)((unsigned char *)battle_map + ((figure_list[figure_no].map_ref)))).terrain = terrain;
    remove_figure(figure_no);
}

// Move state (state_idx 3): step the figure toward its current target; on arrival drop the routing
// flag and return to defend state 6.
// FUNCTION: C2 0x4de81
// FUNCTION: C2WIN 0x00478269
void sf03_move(void)
{
    figure_list[figure_no].is_routing = 1;
    get_fig_walk_image();
    if (figure_go_to_target() == 0)
        return;
    if ((figure_list[figure_no].is_visible & 2) != 0) {
        figure_list[figure_no].is_visible &= 0xfd;
        figure_list[figure_no].state_idx = 6;
    }
}

// Move-and-reform state (state_idx 15): step the figure toward its current target; once it
// arrives, call reform() on its parent unit with the saved formation.
// FUNCTION: C2 0x4decb
// FUNCTION: C2WIN 0x00478307
void sf15_move_and_reform(void)
{
    int moved;
    int unit_ref;
    int formation;

    figure_list[figure_no].is_routing = 1;
    get_fig_walk_image();
    moved = figure_go_to_target();
    if (moved == 0)
        return;
    if ((figure_list[figure_no].is_visible & 2) == 0)
        return;
    figure_list[figure_no].is_visible &= 0xfd;
    formation = figure_list[figure_no].shield_class;
    unit_ref  = figure_list[figure_no].unit_ref;
    reform(unit_ref, formation, 1);
}

// Fight state (state_idx 4): if the opponent is still alive and also fighting us, resolve one tick
// of melee; otherwise fall back to look-for-fight (state 9).
// FUNCTION: C2 0x4df2d
// FUNCTION: C2WIN 0x004783ca
void sf04_fight(void)
{
    int not_visible;

    figure_list[figure_no].is_routing = 0;
    get_fig_fight_image();
    enemy_figure = (short)figure_list[figure_no].opponent;
    if (figure_list[figure_list[figure_no].opponent].exists == 0) {
        figure_list[figure_no].state_idx = 9;
        return;
    }
    if (figure_list[enemy_figure].state_idx != 4) {
        figure_list[figure_no].state_idx = 9;
        return;
    }
    do_the_fight();
    not_visible = (figure_list[figure_no].is_visible == 0);
    if (not_visible)
        figure_go_to_target();
}

// Empty state-handler slot for the "mop-up" battle state.
// FUNCTION: C2 0x4dfb7
void sf05_mop_up(void)
{
}

// Defend state (state == 6). Hold position; harass any enemy in firing range.
// FUNCTION: C2 0x4dfb8
// FUNCTION: C2WIN 0x004784c6
void sf06_defend(void)
{
    int fire_tick;

    get_fig_still_image();
    temp_unit = (short)figure_list[figure_no].unit_ref;

    if (unit_list[figure_list[figure_no].unit_ref].unit_sub_kind != 0) {
        figure_list[figure_no].missile_max = 0x20;
        fire_tick = figure_list[figure_no].missile_max / 2;
        figure_list[figure_no].missile_timer++;
        if (figure_list[figure_no].missile_timer == fire_tick) {
            if (find_nearest_target(5)) {
                figure_list[figure_no].missile_target = enemy_figure;
                figure_list[figure_no].direction = (char)get_heading(
                    figure_list[figure_no].grid_x,
                    figure_list[figure_no].grid_y,
                    figure_list[enemy_figure].grid_x,
                    figure_list[enemy_figure].grid_y,
                    figure_list[figure_no].direction);
            } else {
                figure_list[figure_no].missile_target = 0;
                figure_list[figure_no].missile_timer = 4;
            }
        }

        if (figure_list[figure_no].missile_timer > figure_list[figure_no].missile_max) {
            figure_list[figure_no].missile_timer = 0;
            enemy_figure = (short)figure_list[figure_no].missile_target;
            if (figure_list[enemy_figure].exists != 0) {
                create_arrow(
                    figure_list[figure_no].arrow_data_ptr,
                    figure_list[figure_no].owner,
                    figure_list[figure_no].grid_x,
                    figure_list[figure_no].grid_y,
                    figure_list[enemy_figure].grid_x,
                    figure_list[enemy_figure].grid_y);
                arrow_list[created_arrow_no].weapon_kind = figure_list[figure_no].sprite_type;
                get_arrow_base_image();
                set_missile_fire_fx(arrow_list[created_arrow_no].weapon_kind);
                set_missile_fire_range(arrow_list[created_arrow_no].weapon_kind);
            }
        }

        if (figure_list[figure_no].missile_target != 0)
            get_fig_missile_image();
    }
}

// Reform state (state_idx 7): walk the figure toward its formation slot; on arrival switch to
// defend state 6 with .is_defending set, snapping its facing to anim_state.
// FUNCTION: C2 0x4e1bf
// FUNCTION: C2WIN 0x00478820
void sf07_reform(void)
{
    int moved;

    figure_list[figure_no].is_routing = 1;
    get_fig_walk_image();
    moved = figure_go_to_target();
    if (moved == 0)
        return;
    if ((figure_list[figure_no].is_visible & 2) == 0)
        return;
    figure_list[figure_no].is_visible   &= 0xfd;
    figure_list[figure_no].state_idx     = 6;
    figure_list[figure_no].is_defending  = 1;
    figure_list[figure_no].direction     = figure_list[figure_no].anim_state;
}

// Withdraw state (state_idx 8): step the figure backward to its target tile; on arrival drop into
// defend state 6 and reset morale (halved for rank-1, zeroed for rank-2) based on the tribe's base
// morale.
// FUNCTION: C2 0x4e21c
// FUNCTION: C2WIN 0x00478901
void sf08_withdraw(void)
{
    int moved;
    int unit_idx;
    int unit_rank;
    int morale;

    figure_list[figure_no].is_routing   = 1;
    figure_list[figure_no].is_defending = 0;
    get_fig_walk_image();
    moved = figure_go_to_target();
    if (moved == 0)
        return;
    if ((figure_list[figure_no].is_visible & 2) == 0)
        return;

    unit_idx = figure_list[figure_no].unit_ref;
    if (unit_list[unit_idx].combat_order == 8)
        unit_list[unit_idx].combat_order = 6;

    figure_list[figure_no].is_visible &= 0xfd;
    figure_list[figure_no].state_idx    = 6;
    figure_list[figure_no].is_defending = 1;
    figure_list[figure_no].morale =
        tribe_ai_data[bat_tribe].aggression;

    unit_rank = figure_list[figure_no].figure_rank;
    if (unit_rank == 1) {
        morale = figure_list[figure_no].morale;
        figure_list[figure_no].morale = (morale / 2);
    }
    if (figure_list[figure_no].figure_rank == 2)
        figure_list[figure_no].morale = 0;
    figure_list[figure_no].direction = figure_list[figure_no].anim_state;
}

// Look-for-fight state (state_idx 9): if defending, scan the eight neighbour cells via
// `nearest_formation_enemy` and engage the first hostile found; otherwise drop into the
// hunt-for-fight state (0xa).
// FUNCTION: C2 0x4e31d
// FUNCTION: C2WIN 0x00478afc
void sf09_look_for_fight(void)
{
    int direction;

    get_fig_still_image();
    if (figure_list[figure_no].is_defending == 0) {
        figure_list[figure_no].state_idx = 0xa;
        return;
    }

    direction = nearest_formation_enemy();
    if (direction >= 8)
        return;

    figure_list[figure_no].state_idx       = 4;
    figure_list[figure_no].fight_direction = direction;
    figure_list[figure_no].opponent        = enemy_figure;
    figure_list[figure_no].fight_role      = 1;
    set_attack_count(figure_no);

    if (figure_list[enemy_figure].state_idx == 4)
        return;
    figure_list[enemy_figure].state_idx       = 4;
    figure_list[enemy_figure].fight_direction = ((direction + 4) % 8);
    figure_list[enemy_figure].opponent        = figure_no;
    figure_list[enemy_figure].fight_role      = 2;
    set_defense_shield(enemy_figure);
}

// Empty state-handler slot for the "berserk" battle state.
// FUNCTION: C2 0x4e3df
void sf16_beserk(void)
{
}

// Hunt-for-fight state (state_idx 10): pick a fresh missile_target (the previous one if still
// alive, otherwise the nearest enemy), then walk toward it via figure_go_to_target.
// FUNCTION: C2 0x4e3e0
// FUNCTION: C2WIN 0x00478c7b
void sf10_hunt_for_fight(void)
{
    short target_idx;

    figure_list[figure_no].is_routing   = 1;
    figure_list[figure_no].wf_searching = 0;
    figure_list[figure_no].is_defending = 0;
    enemy_figure = figure_list[figure_no].missile_target;

    if ((figure_list[figure_no].is_visible & 1) != 0) {
        target_idx = enemy_figure;
        if (target_idx != 0) {
            if (figure_list[target_idx].exists != 0)
                goto have_target;
        }
        if (find_nearest_enemy() != 0)
            goto tail;
        figure_list[figure_no].state_idx = 6;
        goto tail;
    have_target:
        figure_list[figure_no].prev_grid_x = figure_list[enemy_figure].grid_x;
        figure_list[figure_no].prev_grid_y = figure_list[enemy_figure].grid_y;
    }

tail:
    if (figure_list[enemy_figure].state_idx == 2) {
        figure_list[figure_no].missile_target = 0;
    }
    if (figure_list[enemy_figure].state_idx == 0xc) {
        figure_list[figure_no].missile_target = 0;
    }
    get_fig_walk_image();
    figure_go_to_target();
}

// Fire-missile state (state == 11). While the figure hasn't reached its firing stand yet
// (is_visible bit 1 clear), keep walking via figure_go_to_target.
// FUNCTION: C2 0x4e4cc
// FUNCTION: C2WIN 0x00479337
void sf11_fire_missile(void)
{
    if ((figure_list[figure_no].is_visible & 1) == 0) {
        get_fig_walk_image();
        figure_go_to_target();
        return;
    }

    get_fig_still_image();
    figure_list[figure_no].missile_max = 0x20;
    figure_list[figure_no].missile_timer++;
    if (figure_list[figure_no].missile_timer
        <= figure_list[figure_no].missile_max)
        goto tail;

    if (get_fire_target(figure_no) != 0) {
        figure_list[figure_no].missile_target = enemy_figure;
        figure_list[figure_no].missile_timer = 0;

        figure_list[figure_no].direction = (char)get_heading(
            figure_list[figure_no].grid_x,
            figure_list[figure_no].grid_y,
            figure_list[enemy_figure].grid_x,
            figure_list[enemy_figure].grid_y,
            figure_list[figure_no].direction);
        create_arrow(figure_list[figure_no].arrow_data_ptr,
                     figure_list[figure_no].owner,
                     figure_list[figure_no].grid_x,
                     figure_list[figure_no].grid_y,
                     figure_list[enemy_figure].grid_x,
                     figure_list[enemy_figure].grid_y);
        arrow_list[created_arrow_no].weapon_kind = figure_list[figure_no].sprite_type;
        get_arrow_base_image();
        set_missile_fire_fx(arrow_list[created_arrow_no].weapon_kind);
        set_missile_fire_range(arrow_list[created_arrow_no].weapon_kind);
    } else if (find_nearest_target(5) != 0) {
        figure_list[figure_no].missile_target = enemy_figure;
        figure_list[figure_no].missile_timer = 0;

        figure_list[figure_no].direction = (char)get_heading(
            figure_list[figure_no].grid_x,
            figure_list[figure_no].grid_y,
            figure_list[enemy_figure].grid_x,
            figure_list[enemy_figure].grid_y,
            figure_list[figure_no].direction);
        create_arrow(figure_list[figure_no].arrow_data_ptr,
                     figure_list[figure_no].owner,
                     figure_list[figure_no].grid_x,
                     figure_list[figure_no].grid_y,
                     figure_list[enemy_figure].grid_x,
                     figure_list[enemy_figure].grid_y);
        arrow_list[created_arrow_no].weapon_kind = figure_list[figure_no].sprite_type;
        get_arrow_base_image();
        set_missile_fire_fx(arrow_list[created_arrow_no].weapon_kind);
        set_missile_fire_range(arrow_list[created_arrow_no].weapon_kind);
    } else {
        figure_list[figure_no].missile_timer = 0xa;
        figure_list[figure_no].missile_target = 0;
    }

tail:
    if (figure_list[figure_no].missile_target != 0)
        get_fig_missile_image();
}

// Routing state handler: mark the figure as routing, update its walk image, advance toward the
// (panic) target, and despawn it once it reaches the map edge.
// FUNCTION: C2 0x4e6b4
// FUNCTION: C2WIN 0x004792ec REORDERED
void sf12_rout(void)
{
    figure_list[figure_no].is_routing = 1;
    get_fig_walk_image();
    figure_go_to_target();
    if (fig_at_edge != 0) {
        remove_figure(figure_no);
    }
}

// Hold position and automatically fire at nearby enemies.
// FUNCTION: C2 0x4e6e4
// FUNCTION: C2WIN 0x004795f0
void sf13_autofire_missile(void)
{
    if ((figure_list[figure_no].is_visible & 1) == 0) {
        get_fig_walk_image();
        figure_go_to_target();
        return;
    }

    get_fig_still_image();
    figure_list[figure_no].missile_max = 0x20;
    figure_list[figure_no].missile_timer++;
    if (figure_list[figure_no].missile_timer
        <= figure_list[figure_no].missile_max)
        goto tail;

    figure_list[figure_no].missile_timer = 0;
    if (find_nearest_target(5) != 0) {
        figure_list[figure_no].missile_target = enemy_figure;

        figure_list[figure_no].direction = (char)get_heading(
            figure_list[figure_no].grid_x,
            figure_list[figure_no].grid_y,
            figure_list[enemy_figure].grid_x,
            figure_list[enemy_figure].grid_y,
            figure_list[figure_no].direction);
        create_arrow(figure_list[figure_no].arrow_data_ptr,
                     figure_list[figure_no].owner,
                     figure_list[figure_no].grid_x,
                     figure_list[figure_no].grid_y,
                     figure_list[enemy_figure].grid_x,
                     figure_list[enemy_figure].grid_y);
        arrow_list[created_arrow_no].weapon_kind = figure_list[figure_no].sprite_type;
        get_arrow_base_image();
        set_missile_fire_fx(arrow_list[created_arrow_no].weapon_kind);
        set_missile_fire_range(arrow_list[created_arrow_no].weapon_kind);
    } else {
        figure_list[figure_no].missile_target = 0;
    }

tail:
    if (figure_list[figure_no].missile_target != 0)
        get_fig_missile_image();
}

// Move into firing position and periodically attack nearby enemies.
// FUNCTION: C2 0x4e895
// FUNCTION: C2WIN 0x0047995d
void sf14_opertunist_fire(void)
{
    if ((figure_list[figure_no].is_visible & 1) == 0) {
        get_fig_walk_image();
        figure_go_to_target();
        return;
    }

    get_fig_still_image();
    figure_list[figure_no].missile_max = 0x30;
    figure_list[figure_no].missile_timer += 1;
    if (figure_list[figure_no].missile_timer > figure_list[figure_no].missile_max) {
        figure_list[figure_no].missile_timer = 0;
        if (find_nearest_target(0xf)) {
            figure_list[figure_no].missile_target = enemy_figure;
            figure_list[figure_no].direction = get_heading(figure_list[figure_no].grid_x, figure_list[figure_no].grid_y,
                                        figure_list[enemy_figure].grid_x, figure_list[enemy_figure].grid_y,
                                        figure_list[figure_no].direction);
            create_arrow(figure_list[figure_no].arrow_data_ptr, figure_list[figure_no].owner,
                         figure_list[figure_no].grid_x, figure_list[figure_no].grid_y,
                         figure_list[enemy_figure].grid_x, figure_list[enemy_figure].grid_y);
            arrow_list[created_arrow_no].weapon_kind = figure_list[figure_no].sprite_type;
            get_arrow_base_image();
            set_missile_fire_fx(arrow_list[created_arrow_no].weapon_kind);
            set_missile_fire_range(arrow_list[created_arrow_no].weapon_kind);
        } else if (figure_list[figure_no].grid_y > 0xa) {
            figure_list[figure_no].missile_target = 0;
            figure_list[figure_no].prev_grid_y = figure_list[figure_no].grid_y - 2; figure_list[figure_no].prev_grid_x = figure_list[figure_no].grid_x;
            figure_list[figure_no].is_routing = 1;
            figure_go_to_target();
        } else figure_list[figure_no].missile_target = 0;
    }

    if (figure_list[figure_no].missile_target != 0) get_fig_missile_image();
}

// Updates an elephant archer's two firing slots and selects nearby targets.
// FUNCTION: C2 0x4dc94 REORDERED
void elephant_fire(void)
{
    int distance;

    if (figure_list[figure_no].state_idx == 2) {
        figure_list[figure_no].archer_image_a = elephant_archer_images[0];
        figure_list[figure_no].archer_image_b = elephant_archer_images[0];
    } else {
        figure_list[figure_no].archer_tick_a++;
        figure_list[figure_no].archer_tick_b++;
        if (figure_list[figure_no].archer_tick_a == 0xb) {
            if (find_nearest_target(0x1e)) {
                target_unit_debar = figure_list[enemy_figure].unit_ref;
                if (our_battle_units <= 1) target_unit_debar = 0;
                figure_list[figure_no].archer_target_a = enemy_figure;
                figure_list[figure_no].archer_heading_a = get_heading(figure_list[figure_no].grid_x, figure_list[figure_no].grid_y,
                                                   figure_list[enemy_figure].grid_x, figure_list[enemy_figure].grid_y,
                                                   figure_list[figure_no].direction);
            } else figure_list[figure_no].archer_tick_a = 0;
        }

        if (figure_list[figure_no].archer_tick_b == 0x15) {
            if (find_nearest_target(0x1e)) {
                target_unit_debar = figure_list[enemy_figure].unit_ref;
                if (our_battle_units <= 1) target_unit_debar = 0;
                figure_list[figure_no].archer_target_b = enemy_figure;
                figure_list[figure_no].archer_heading_b = get_heading(figure_list[figure_no].grid_x, figure_list[figure_no].grid_y,
                                                   figure_list[enemy_figure].grid_x, figure_list[enemy_figure].grid_y,
                                                   figure_list[figure_no].direction);
            } else figure_list[figure_no].archer_tick_b = 0xa;
        }

        if (figure_list[figure_no].archer_tick_a > 0x14) {
            enemy_figure = figure_list[figure_no].archer_target_a;
            create_arrow(figure_list[figure_no].arrow_data_ptr, figure_list[figure_no].owner,
                         figure_list[figure_no].grid_x, figure_list[figure_no].grid_y,
                         figure_list[enemy_figure].grid_x, figure_list[enemy_figure].grid_y);
            distance = get_longest_distance(figure_list[figure_no].grid_x, figure_list[figure_no].grid_y,
                                            figure_list[enemy_figure].grid_x, figure_list[enemy_figure].grid_y);
            arrow_list[created_arrow_no].weapon_kind = figure_list[figure_no].sprite_type;
            get_arrow_base_image();
            arrow_list[created_arrow_no].anim_count = 0x3c;
            set_missile_fire_fx(arrow_list[created_arrow_no].weapon_kind);
            set_missile_fire_range(arrow_list[created_arrow_no].weapon_kind);
            if      (distance <= 2) arrow_list[created_arrow_no].anim_delta = 0xa;
            else if (distance <= 4) arrow_list[created_arrow_no].anim_delta = 6;
            else if (distance <= 8) arrow_list[created_arrow_no].anim_delta = 3;
            else                arrow_list[created_arrow_no].anim_delta = 1;
            figure_list[figure_no].archer_tick_a = rand128 & 3;
        }

        if (figure_list[figure_no].archer_tick_b > 0x1e) {
            enemy_figure = figure_list[figure_no].archer_target_b;
            create_arrow(figure_list[figure_no].arrow_data_ptr, figure_list[figure_no].owner,
                         figure_list[figure_no].grid_x, figure_list[figure_no].grid_y,
                         figure_list[enemy_figure].grid_x, figure_list[enemy_figure].grid_y);
            distance = get_longest_distance(figure_list[figure_no].grid_x, figure_list[figure_no].grid_y,
                                            figure_list[enemy_figure].grid_x, figure_list[enemy_figure].grid_y);
            arrow_list[created_arrow_no].weapon_kind = figure_list[figure_no].sprite_type;
            set_missile_fire_fx(arrow_list[created_arrow_no].weapon_kind);
            get_arrow_base_image();
            arrow_list[created_arrow_no].anim_count = 0x3c;
            set_missile_fire_range(arrow_list[created_arrow_no].weapon_kind);
            if      (distance <= 2) arrow_list[created_arrow_no].anim_delta = 0xa;
            else if (distance <= 4) arrow_list[created_arrow_no].anim_delta = 6;
            else if (distance <= 8) arrow_list[created_arrow_no].anim_delta = 3;
            else                arrow_list[created_arrow_no].anim_delta = 1;
            figure_list[figure_no].archer_tick_b = 0xa;
        }

        figure_list[figure_no].archer_image_a = elephant_archer_images[figure_list[figure_no].archer_tick_a];
        if      (map_direction == 0) figure_list[figure_no].archer_image_a += ((figure_list[figure_no].archer_heading_a % 8) * 4);
        else if (map_direction == 2) figure_list[figure_no].archer_image_a += (((figure_list[figure_no].archer_heading_a + 6) % 8) * 4);
        else if (map_direction == 4) figure_list[figure_no].archer_image_a += (((figure_list[figure_no].archer_heading_a + 4) % 8) * 4);
        else if (map_direction == 6) figure_list[figure_no].archer_image_a += (((figure_list[figure_no].archer_heading_a + 2) % 8) * 4);

        /* Animate the elephant's second archer once its firing cycle begins. */
        if (figure_list[figure_no].archer_tick_b < 0xa) figure_list[figure_no].archer_image_b = elephant_archer_images[0];
        else figure_list[figure_no].archer_image_b = elephant_archer_images[figure_list[figure_no].archer_tick_b - 10];

        if      (map_direction == 0) figure_list[figure_no].archer_image_b += ((figure_list[figure_no].archer_heading_b % 8) * 4);
        else if (map_direction == 2) figure_list[figure_no].archer_image_b += (((figure_list[figure_no].archer_heading_b + 6) % 8) * 4);
        else if (map_direction == 4) figure_list[figure_no].archer_image_b += (((figure_list[figure_no].archer_heading_b + 4) % 8) * 4);
        else if (map_direction == 6) figure_list[figure_no].archer_image_b += (((figure_list[figure_no].archer_heading_b + 2) % 8) * 4);
    }
}

// Set a newly created projectile's sprite from the firing figure's type.
// FUNCTION: C2 0x4f096
// FUNCTION: C2WIN 0x0047a4ba
void get_arrow_base_image(void)
{
    if (figure_list[figure_no].sprite_type == 3) {
        arrow_list[created_arrow_no].sprite_base = 0xaa;
    } else if (figure_list[figure_no].sprite_type == 9) {
        arrow_list[created_arrow_no].sprite_base = 0xaa;
    } else if (figure_list[figure_no].sprite_type == 10) {
        arrow_list[created_arrow_no].sprite_base = 0xaa;
    } else if (figure_list[figure_no].sprite_type == 13) {
        arrow_list[created_arrow_no].sprite_base = 0x28;
    } else if (figure_list[figure_no].sprite_type == 15) {
        arrow_list[created_arrow_no].sprite_base = 0x50;
    } else if (figure_list[figure_no].sprite_type == 16) {
        arrow_list[created_arrow_no].sprite_base = 0xaa;
    } else if (figure_list[figure_no].sprite_type == 17) {
        arrow_list[created_arrow_no].sprite_base = 0xaa;
    } else {
        arrow_list[created_arrow_no].sprite_base = 0;
    }
    arrow_list[created_arrow_no].sprite_kind =
        figure_list[figure_no].sprite_kind;
}

// Advance and animate every active projectile.
// FUNCTION: C2 0x4f174
// FUNCTION: C2WIN 0x0047a67f
void arrow_intelligence(void)
{
    int direction_idx;

    for (arrow_no = 1; arrow_no < 0xc9; arrow_no++) {
        if (arrow_list[arrow_no].exists != 0) {
            arrow_list[arrow_no].flight_done = 0;
            fly_to_target();

            if (arrow_list[arrow_no].anim_count != 0) arrow_list[arrow_no].anim_count -= arrow_list[arrow_no].anim_delta;

            direction_idx = (unsigned char)arrow_list[arrow_no].heading;
            if (map_direction == 0) direction_idx += 1;
            if (map_direction == 2) direction_idx += 7;
            if (map_direction == 4) direction_idx += 5;
            if (map_direction == 6) direction_idx += 3;
            if (direction_idx >= 8) direction_idx = direction_idx % 8;

            arrow_list[arrow_no].sprite_anim =
                (arrow_list[arrow_no].sprite_base + direction_idx);
        }
    }
}

// Deselect the player's selected unit or reform a selected enemy unit for the current map mode.
// FUNCTION: C2 0x4f27d
// FUNCTION: C2WIN 0x0047a7db
void general_reform(int formation)
{
    /* Process each selected unit once. */
    int previous_unit_ref = 0;

    for (figure_no = 1; figure_no < 201; figure_no++) {
        if (figure_list[figure_no].selected != 0
            && figure_list[figure_no].exists != 0
            && (unsigned char)figure_list[figure_no].unit_ref != previous_unit_ref) {
            if (figure_list[figure_no].owner == 0) {
                deselect_all_figures();
                return;
            }
            if (battle_state == 0) {
                if (test_reform_pattern(figure_list[figure_no].unit_ref, formation) != 0) {
                    instant_reform(figure_list[figure_no].unit_ref, formation);
                }
            } else {
                reform(figure_list[figure_no].unit_ref, formation, 0);
            }
            previous_unit_ref = (unsigned char)figure_list[figure_no].unit_ref;
        }
    }
}

// Assign a formation and destination slots to every figure in a unit.
// FUNCTION: C2 0x4f33d
// FUNCTION: C2WIN 0x0047a94b
void reform(int unit_ref, int mode, int force)
{
  int unit_x;
  int pos;
  int unit_y;
  unit_list[unit_ref].formation_mode = mode;
  pos = 0;
  unit_x = unit_list[unit_ref].x;
  unit_y = unit_list[unit_ref].y;
  for (temp_figure = unit_list[unit_ref].first_figure; temp_figure <= unit_list[unit_ref].last_figure; temp_figure++)
  {
    if (figure_list[temp_figure].exists != 0)
    {
      if (mode != 3)
      {
        get_fig_in_unit_position(mode, pos, temp_figure);
        figure_list[temp_figure].prev_grid_x = unit_x + x_bit;
        figure_list[temp_figure].prev_grid_y = unit_y + y_bit;
        figure_list[temp_figure].offset_x = x_bit;
        figure_list[temp_figure].offset_y = y_bit;
        figure_list[temp_figure].shield_class = mode;
        if (force != 0)
          figure_list[temp_figure].state_idx = 7;
        if (figure_list[temp_figure].state_idx == 0xc)
          figure_list[temp_figure].state_idx = 7;
      }
      else
      {
        figure_list[temp_figure].state_idx = 0xa;
      }
      figure_list[temp_figure].is_defending = 0;
      pos++;
    }
  }

}


// Immediately place a unit's figures into the requested formation.
// FUNCTION: C2 0x4f44d
// FUNCTION: C2WIN 0x0047ab55
void instant_reform(int unit_no, int formation)
{
    int base_y;
  int base_x;
  int position;
  unit_list[unit_no].formation_mode = formation;
  if (formation == 3)
    return;
  position = 0;
  base_x = unit_list[unit_no].x;
  base_y = unit_list[unit_no].y;
  for (temp_figure = unit_list[unit_no].first_figure; temp_figure <= unit_list[unit_no].last_figure; temp_figure++)
  {
    if (figure_list[temp_figure].exists != 0)
    {
      ((unsigned char *) battle_map)[figure_list[temp_figure].map_ref + 1] = 0;
    }
  }

  for (temp_figure = unit_list[unit_no].first_figure; temp_figure <= unit_list[unit_no].last_figure; temp_figure++)
  {
    if (figure_list[temp_figure].exists != 0)
    {
      get_fig_in_unit_position(formation, position, temp_figure);
      update_map = 1;
      figure_list[temp_figure].grid_x = base_x + x_bit;
      figure_list[temp_figure].grid_y = base_y + y_bit;
      figure_list[temp_figure].map_ref = (figure_list[temp_figure].grid_x + (figure_list[temp_figure].grid_y * 0x34)) * 4;
      ((unsigned char *) battle_map)[figure_list[temp_figure].map_ref + 1] = temp_figure;
      figure_list[temp_figure].offset_x = x_bit;
      figure_list[temp_figure].offset_y = y_bit;
      figure_list[temp_figure].shield_class = formation;
      figure_list[temp_figure].is_defending = 1;
      figure_list[temp_figure].state_idx = 6;
      figure_list[temp_figure].wf_step_y = 0;
      figure_list[temp_figure].wf_step_x = 0;
      figure_list[temp_figure].is_routing = 0;
      figure_list[temp_figure].is_visible &= 0xfd;
      figure_list[temp_figure].is_visible |= 1;
      position++;
    }
  }

}


// Test whether a unit can use the requested `formation` at its current position.
// FUNCTION: C2 0x4f5e0
// FUNCTION: C2WIN 0x0047aeb4
int test_reform_pattern(int unit_ref, int dir)
{
    int position;
    int unit_x;
    int unit_y;
    int occupant_idx;

    if (dir == 3)
        return 1;
    position = 0;
    unit_x = unit_list[unit_ref].x;
    unit_y = unit_list[unit_ref].y;
    for (temp_figure = unit_list[unit_ref].first_figure;
         temp_figure <= unit_list[unit_ref].last_figure;
         temp_figure++) {
        if (figure_list[temp_figure].exists != 0) {
            int cell_off;

            get_fig_in_unit_position(dir, position, temp_figure);
            cell_off = (unit_x + x_bit) * BATTLE_CELL_BYTES;
            cell_off += (unit_y + y_bit) * BATTLE_ROW;
            occupant_idx = ((unsigned char *)battle_map)[cell_off + 1];
            position++;
            if (cell_off >= nomansland_ptr)
                return 0;
            if (occupant_idx == 0)
                continue;
            if (figure_list[occupant_idx].unit_ref != figure_list[temp_figure].unit_ref) {
                return 0;
            }
        }
    }
    return 1;
}


// Compute the (x_bit, y_bit) offsets for `figure_idx` in the requested `formation`.
// FUNCTION: C2 0x4f6c1
// FUNCTION: C2WIN 0x0047b036
void get_fig_in_unit_position(int formation, int position, int figure_idx)
{
    int side;
    int row_spacing;
    int column;

    row_spacing = figure_list[figure_idx].unit_grid_x;
    column = figure_list[figure_idx].unit_grid_y;
    side = figure_list[figure_idx].unit_position;

    if (formation == 0) {
        x_bit = get_x_spacing(row_spacing, column, position);
        y_bit = get_y_spacing(row_spacing, column, position, side);
    } else if (formation == 1) {
        y_bit = get_x_spacing(row_spacing, column, position);
        x_bit = get_y_spacing(row_spacing, column, position, side);
    } else if (formation == 2) {
        x_bit = get_x_spacing(row_spacing, column + 1, position);
        y_bit = get_y_spacing(row_spacing, column + 1, position, side);
    } else {
        x_bit = y_bit = 0;
    }
}

// Pick the sprite-frame for figure_no while it is in a fight state. fight_state == 2 (stopped)
// delegates to get_fig_still_image and returns.
// FUNCTION: C2 0x4f74e
// FUNCTION: C2WIN 0x0047b16d
void get_fig_fight_image(void)
{
    int dir;
    int cnt8;
    int defend_frame;
    int attacker;
    int sprite_idx;
    int frame_index;

    dir = figure_list[figure_no].fight_direction;
    if (figure_list[figure_no].fight_state == 2) {
        get_fig_still_image();
        return;
    }
    if (figure_list[figure_no].fight_state != 0) {
        cnt8 = 9;
        defend_frame = 0;
        attacker = 0;
        figure_list[figure_no].sprite_dir = 1;
    } else {
        cnt8 = 20;
        defend_frame = 16;
        attacker = 10;
    }
    if (figure_list[figure_no].fight_state != 0) {
        dir = (dir + 2) % 8;
    }
    if (map_direction == 0) {
        sprite_idx = (dir % 8) * cnt8;
    } else if (map_direction == 2) {
        sprite_idx = ((dir + 6) % 8) * cnt8;
    } else if (map_direction == 4) {
        sprite_idx = ((dir + 4) % 8) * cnt8;
    } else if (map_direction == 6) {
        sprite_idx = ((dir + 2) % 8) * cnt8;
    }
    if (figure_list[figure_no].fight_role == 1) {
        if (figure_list[figure_no].fight_state != 0) {
            figure_list[figure_no].anim_counter++;
            if (figure_list[figure_no].anim_counter >= 12)
                figure_list[figure_no].anim_counter = 0;
            frame_index = ((unsigned char)figure_list[figure_no].anim_counter) >> 1;
            sprite_idx += frame_index;
        } else if (figure_list[figure_no].fight_swing_active != 0) {
            sprite_idx += attacker + 3;
            figure_list[figure_no].anim_counter++;
            if (figure_list[figure_no].anim_counter >= 8)
                figure_list[figure_no].anim_counter = 0;
            frame_index = ((unsigned char)figure_list[figure_no].anim_counter) >> 1;
            if (frame_index == 3)
                frame_index = 1;
            sprite_idx += frame_index;
        } else {
            sprite_idx += attacker;
            figure_list[figure_no].anim_counter++;
            if (figure_list[figure_no].anim_counter >= 12)
                figure_list[figure_no].anim_counter = 0;
            sprite_idx += ((unsigned char)figure_list[figure_no].anim_counter) >> 1;
        }
    } else {
        if (figure_list[figure_no].fight_state != 0) {
            figure_list[figure_no].anim_counter++;
            if (figure_list[figure_no].anim_counter >= 12)
                figure_list[figure_no].anim_counter = 0;
            frame_index = ((unsigned char)figure_list[figure_no].anim_counter) >> 1;
            sprite_idx += frame_index;
        } else {
            sprite_idx += defend_frame;
            figure_list[figure_no].anim_counter++;
            if (figure_list[figure_no].anim_counter >= 8)
                figure_list[figure_no].anim_counter = 0;
            sprite_idx += ((unsigned char)figure_list[figure_no].anim_counter) >> 1;
        }
    }
    figure_list[figure_no].sprite_anim = sprite_idx;
}


// Select and advance the current figure's walking animation frame.
// FUNCTION: C2 0x4f902
// FUNCTION: C2WIN 0x0047b600
void get_fig_walk_image(void)
{
    int stride;
    int dir_base;

    figure_list[figure_no].sprite_dir = 0;
    if (figure_list[figure_no].fight_state == 2) {
        stride = 6;
    } else if (figure_list[figure_no].fight_state != 0) {
        stride = 5;
    } else {
        stride = 0x14;
    }
    if (map_direction == 0)      dir_base = (figure_list[figure_no].direction % 8) * stride;
    else if (map_direction == 2) dir_base = ((figure_list[figure_no].direction + 6) % 8) * stride;
    else if (map_direction == 4) dir_base = ((figure_list[figure_no].direction + 4) % 8) * stride;
    else if (map_direction == 6) dir_base = ((figure_list[figure_no].direction + 2) % 8) * stride;
    dir_base += (unsigned char)figure_list[figure_no].anim_counter >> 1;
    if (figure_list[figure_no].fight_state == 2) {
        figure_list[figure_no].anim_counter++;
        if (figure_list[figure_no].anim_counter >= 0xc)
            figure_list[figure_no].anim_counter = 0;
    } else if (figure_list[figure_no].fight_state != 0) {
        figure_list[figure_no].anim_counter++;
        if (figure_list[figure_no].anim_counter >= 0xa)
            figure_list[figure_no].anim_counter = 0;
    } else {
        figure_list[figure_no].anim_counter++;
        if (figure_list[figure_no].anim_counter >= 0x14)
            figure_list[figure_no].anim_counter = 0;
    }
    figure_list[figure_no].sprite_anim = dir_base;
}

// Select the current figure's still frame, including tortoise-formation poses.
// FUNCTION: C2 0x4fa48
// FUNCTION: C2WIN 0x0047b8e5
void get_fig_still_image(void)
{
    int stride;
    int dir_base;

    figure_list[figure_no].sprite_dir = 0;
    if (figure_list[figure_no].fight_state == 2) {
        stride = 6;
    } else if (figure_list[figure_no].fight_state != 0) {
        stride = 5;
    } else if (figure_list[figure_no].is_defending != 0
               && figure_list[figure_no].shield_class == 2) {
        get_fig_tortoise_image();
        return;
    } else {
        stride = 0x14;
    }
    if (map_direction == 0)      dir_base = (figure_list[figure_no].direction % 8) * stride;
    else if (map_direction == 2) dir_base = ((figure_list[figure_no].direction + 6) % 8) * stride;
    else if (map_direction == 4) dir_base = ((figure_list[figure_no].direction + 4) % 8) * stride;
    else if (map_direction == 6) dir_base = ((figure_list[figure_no].direction + 2) % 8) * stride;
    figure_list[figure_no].sprite_anim = dir_base;
}

// Pick the facing for a tortoise figure (the locked-shield Roman formation): prefer to face the
// same-army figure that's one step E (4), N (2), S (6), or W (0); fall back to E (4) when no
// neighbour matches.
// FUNCTION: C2 0x4fb34
// FUNCTION: C2WIN 0x0047bac7
void get_fig_tortoise_image(void)
{
    int sprite_frame;

    if (test_for_same_fig_to(4) == 0) {
        figure_list[figure_no].direction = 4;
    } else {
        if (test_for_same_fig_to(2) == 0)
            figure_list[figure_no].direction = 2;
        else {
            if (test_for_same_fig_to(6) == 0)
                figure_list[figure_no].direction = 6;
            else {
                if (test_for_same_fig_to(0) == 0)
                    figure_list[figure_no].direction = 0;
                else
                    figure_list[figure_no].direction = 4;
            }
        }
    }

    if (map_direction == 0)      sprite_frame = (figure_list[figure_no].direction % 8) * 20;
    else if (map_direction == 2) sprite_frame = ((figure_list[figure_no].direction + 6) % 8) * 20;
    else if (map_direction == 4) sprite_frame = ((figure_list[figure_no].direction + 4) % 8) * 20;
    else if (map_direction == 6) sprite_frame = ((figure_list[figure_no].direction + 2) % 8) * 20;
    sprite_frame = sprite_frame + 0x10;
    figure_list[figure_no].sprite_anim = sprite_frame;
}

// Direction-checked neighbour test on the battle map. Returns 1 if the neighbour cell in `direction`
// (0/2/4/6) holds a figure that shares figure_no's unit_ref, 0 otherwise.
// FUNCTION: C2 0x4fc4e
// FUNCTION: C2WIN 0x0047bcbd
int test_for_same_fig_to(int direction)
{
    int neighbor_idx;

    if (direction == 4) {
        if (figure_list[figure_no].grid_y >= 0x33) return 0;
        neighbor_idx = ((unsigned char *)battle_map)[figure_list[figure_no].map_ref + BATTLE_ROW + 1];
    } else if (direction == 2) {
        if (figure_list[figure_no].grid_y >= 0x33) return 0;
        neighbor_idx = ((unsigned char *)battle_map)[figure_list[figure_no].map_ref + 5];
    } else if (direction == 6) {
        if (figure_list[figure_no].grid_y <= 0) return 0;
        neighbor_idx = ((unsigned char *)battle_map)[figure_list[figure_no].map_ref - 3];
    } else if (direction == 0) {
        if (figure_list[figure_no].grid_y <= 0) return 0;
        neighbor_idx = ((unsigned char *)battle_map)[figure_list[figure_no].map_ref - 0xcf];
    }
    if (neighbor_idx == 0) return 0;
    if (figure_list[neighbor_idx].unit_ref != figure_list[figure_no].unit_ref)
        return 0;
    return 1;
}

// Pick the sprite frame for a dying figure (figure_no).
// FUNCTION: C2 0x4fd25
// FUNCTION: C2WIN 0x0047be76
void get_fig_death_image(void)
{
    int frame;

    if (figure_list[figure_no].fight_state == 2) {
        get_fig_still_image();
        return;
    }
    frame = figure_list[figure_no].death_timer >> 1;
    if (frame > 7) frame = 7;
    if (figure_list[figure_no].fight_state != 0) {
        figure_list[figure_no].sprite_anim = (frame + 0x48);
        figure_list[figure_no].sprite_dir  = 1;
    } else {
        figure_list[figure_no].sprite_anim = (frame + 0xa0);
    }
}

// Missile-attack frame picker for figure_no. Non-zero fight_state uses the short 9-frame stride
// and sprite_dir=1; idle uses the normal 20-frame stride.
// FUNCTION: C2 0x4fd87
// FUNCTION: C2WIN 0x0047bf53
void get_fig_missile_image(void)
{
    int dir_base;
    int stride;
    int cnt8;

    figure_list[figure_no].sprite_dir = 0;
    if (figure_list[figure_no].fight_state != 0) {
        stride = 9;
        figure_list[figure_no].sprite_dir = 1;
    } else {
        stride = 0x14;
    }

    if (map_direction == 0)
        dir_base = (figure_list[figure_no].direction % 8) * stride;
    else if (map_direction == 2)
        dir_base = ((figure_list[figure_no].direction + 6) % 8) * stride;
    else if (map_direction == 4)
        dir_base = ((figure_list[figure_no].direction + 4) % 8) * stride;
    else if (map_direction == 6)
        dir_base = ((figure_list[figure_no].direction + 2) % 8) * stride;

    cnt8 = figure_list[figure_no].missile_timer;
    if (0x20 < cnt8)
        cnt8 = 0x20;

    if (figure_list[figure_no].sprite_type == 10)
        dir_base += sling_images[cnt8];
    else if (figure_list[figure_no].sprite_type == 3)
        dir_base += sling_images[cnt8];
    else if (figure_list[figure_no].fight_state != 0)
        dir_base += horsebow_images[cnt8];
    else
        dir_base += bow_images[cnt8];
    figure_list[figure_no].sprite_anim = dir_base;
}

int try_a_battlemap_square(int direction);
int try_this_battlemap_square(int cell_offset);
void target_from_figure_dirc(int direction);
int get_wf_dirc(int search_mode);
int arrow_off_map(void);
int find_adjacent_target(void);

// Mark a clipped rectangle of battle-map cells dirty for redraw.
// FUNCTION: C2 0x4fea9
// FUNCTION: C2WIN 0x0047c1a2
void set_figure_map_refresh(int grid_x, int grid_y, int offset_x, int offset_y,
                            int radius, int extra_size)
{
    int x_from;
    int last_bound;
    int x;
    int y_begin;
    int bottom_row;
    int y;
    int map_pos;
    int row_stride;

    x_from = grid_x + offset_x - radius;
    y_begin = grid_y + offset_y - radius;
    last_bound = (grid_x + offset_x + extra_size) + radius;
    bottom_row = (grid_y + offset_y + extra_size) + radius;

    if (x_from < 0) x_from = 0;
    if (last_bound >= 0x34) last_bound = 0x33;
    if (y_begin < 0) y_begin = 0;
    if (bottom_row >= 0x34) bottom_row = 0x33;

    map_pos = (y_begin * 0x34 + x_from) * 4;
    row_stride = (0x34 - ((last_bound - x_from) + 1)) * 4;
    for (y = y_begin; y <= bottom_row; ++y, map_pos += row_stride) {
        for (x = x_from; x <= last_bound; ++x, map_pos += 4) {
            ((unsigned char *)battle_map)[map_pos + 2] |= 2;
        }
    }
}

// Set the new projectile's firing range and speed from its `weapon_kind`.
// FUNCTION: C2 0x4ff22
// FUNCTION: C2WIN 0x0047c2b4
void set_missile_fire_range(int weapon_kind)
{
    if (weapon_kind <= 3) {
        arrow_list[created_arrow_no].fire_range = 60;
        arrow_list[created_arrow_no].fire_speed = 50;
    } else if (weapon_kind <= 9) {
        arrow_list[created_arrow_no].fire_range = 30;
        arrow_list[created_arrow_no].fire_speed = 120;
    } else if (weapon_kind <= 10) {
        arrow_list[created_arrow_no].fire_range = 60;
        arrow_list[created_arrow_no].fire_speed = 50;
    } else if (weapon_kind <= 16) {
        arrow_list[created_arrow_no].fire_range = 40;
        arrow_list[created_arrow_no].fire_speed = 100;
    } else if (weapon_kind <= 17) {
        arrow_list[created_arrow_no].fire_range = 70;
        arrow_list[created_arrow_no].fire_speed = 30;
    }
}

// Update the AI-controlled enemy units for the current battle tick.
// FUNCTION: C2 0x4ffd0
// FUNCTION: C2WIN 0x0047c3b9
void update_units_ai(void)
{
    if (cnt32 == 0)
        battle_ai_count++;
    for (temp_unit = 1; temp_unit < 0x33; temp_unit++) {
        if (unit_list[temp_unit].exists == 0) continue;
        if (unit_list[temp_unit].type != 0) continue;
        if (unit_list[temp_unit].combat_order == 0xc) continue;
        if (unit_list[temp_unit].unit_sub_kind != 0) {
            do_light_ai();
        } else if (unit_list[temp_unit].owner == 0xf) {
            elephant_ai();
        } else {
            do_heavy_ai();
        }
    }
}

// Per-tick AI driver for elephant units.
// FUNCTION: C2 0x5004e
// FUNCTION: C2WIN 0x0047c4cb
void elephant_ai(void)
{
    ++unit_list[temp_unit].ai_tick;
    if (unit_list[temp_unit].ai_tick >= unit_list[temp_unit].ai_period) {
        unit_list[temp_unit].ai_tick = 0;
        if ((rand128 & 7) <= 4) {
            set_ai_unit_move(0, 0);
        } else {
            set_ai_unit_move(0, -1);
        }
    }
}

// Choose movement or firing orders for the current light enemy unit.
// FUNCTION: C2 0x5009b
// FUNCTION: C2WIN 0x0047c57e
void do_light_ai(void)
{
    int berserk_tick;

    ++unit_list[temp_unit].ai_tick;
    if (unit_list[temp_unit].ai_tick >= unit_list[temp_unit].ai_period) {
        unit_list[temp_unit].ai_tick = 0;
        if ((unit_list[temp_unit].unit_rank & 0xff) == 2) {
            berserk_tick = 60;
        } else {
            berserk_tick = 30;
        }
        if (berserk_tick <= battle_ai_count) {
            set_ai_unit_beserk();
        } else {
            set_ai_unit_auto_fire();
        }
    }
}

// Choose manoeuvre, attack, or withdrawal orders for the current heavy enemy unit.
// FUNCTION: C2 0x500fc
// FUNCTION: C2WIN 0x0047c653
void do_heavy_ai(void)
{
    int wedge_move;
    int berserk_count;
    int delayed_berserk;
    int base_morale;
    int forward_move;
    int order;
    int unit_x;

    delayed_berserk = tribe_ai_data[bat_tribe].delayed_berserk;
    berserk_count = tribe_ai_data[bat_tribe].berserk_count;
    base_morale = tribe_ai_data[bat_tribe].base_morale;
    wedge_move = tribe_ai_data[bat_tribe].wedge_move;
    forward_move = tribe_ai_data[bat_tribe].forward_move;

    /* Wait until the unit's next AI decision tick. */
    unit_list[temp_unit].ai_tick = (unit_list[temp_unit].ai_tick + 1);
    if (unit_list[temp_unit].ai_tick < unit_list[temp_unit].ai_period)
        return;
    unit_list[temp_unit].ai_tick = 0;

    if (unit_list[temp_unit].target_lock > 2) {
        if (base_morale != 0 && unit_list[temp_unit].withdraw_flag == 0) set_ai_unit_withdraw(0, 8);
        return;
    }

    order = unit_list[temp_unit].combat_order;
    if (order == 0xa || order == 8) return;

    if (delayed_berserk == 1) { set_ai_unit_delayed_beserk(); return; }
    if (berserk_count <= battle_ai_count) { set_ai_unit_beserk(); return; }

    order = unit_list[temp_unit].flank_pending;
    if (order == 1) { set_ai_flank_move(order); unit_list[temp_unit].flank_pending = 0; return; }
    if (order == 2) { set_ai_flank_move(order); unit_list[temp_unit].flank_pending = 0; return; }
    if (order == 3) { set_ai_flank_move(order); unit_list[temp_unit].flank_pending = 0; return; }
    if (order == 4) { set_ai_flank_move(order); unit_list[temp_unit].flank_pending = 0; return; }

    if (unit_list[temp_unit].manoeuvre_done == 0 && wedge_move != 0) {
        unit_x = unit_list[temp_unit].x; if (unit_x < 0x12) set_ai_unit_move(8, -12);
        else if (unit_x > 0x1e) set_ai_unit_move(-10, -12);
        else set_ai_unit_move(0, -18);
        return;
    }

    if (unit_list[temp_unit].manoeuvre_done != 0) return; if (forward_move == 0) return; set_ai_unit_move(0, -4); unit_list[temp_unit].manoeuvre_done = 0;
}

// Position every figure of temp_unit in a flank-line or flank-column formation. `flank_mode` selects the
// anchor column: 1 = left flank (roman_left_edge - 6), 2 = right flank (roman_right_edge + 8), 3 =
// centre (fixed at 0x2c); the column is clamped to [0, 0x33].
// FUNCTION: C2 0x502f3
// FUNCTION: C2WIN 0x0047ca6a
void set_ai_flank_move(int flank_mode)
{
    int target_col;
    int formation;
    int figure_position;

    if (flank_mode == 1) {
        target_col = roman_left_edge - 6;
    } else if (flank_mode == 2) {
        target_col = roman_right_edge + 8;
    } else if (flank_mode >= 3) {
        target_col = 0x2c;
    }
    if (target_col < 0) {
        target_col = 0;
    }
    if (target_col >= 0x34) {
        target_col = 0x33;
    }
    figure_position = 0;
    formation = tribe_ai_data[bat_tribe].prefer_column;
    if (flank_mode >= 3) {
        formation = 0;
    }
    if (formation == 1) {
        unit_list[temp_unit].combat_order = 7;
    } else {
        unit_list[temp_unit].combat_order = 3;
        if (flank_mode == 2) {
            target_col -= unit_list[temp_unit].fig_count / 2;
        }
    }
    unit_list[temp_unit].manoeuvre_done = 1;
    for (temp_figure = unit_list[temp_unit].first_figure;
         temp_figure <= unit_list[temp_unit].last_figure;
         temp_figure++) {
        if (figure_list[temp_figure].exists == 0) continue;
        if (figure_list[temp_figure].state_idx == 2) continue;
        if (figure_list[temp_figure].state_idx == 0xc) continue;
        if (figure_list[temp_figure].state_idx == 0xa) continue;
        if (formation == 1) {
            figure_list[temp_figure].state_idx = 7;
            figure_list[temp_figure].shield_class = 1;
            figure_list[temp_figure].prev_grid_x = target_col + col_flank_data[figure_position].dx;
            figure_list[temp_figure].prev_grid_y = col_flank_data[figure_position].dy + (unit_list[temp_unit].y - 0x14);
        } else if (flank_mode >= 3) {
            figure_list[temp_figure].state_idx = 7;
            figure_list[temp_figure].prev_grid_x = target_col + line_flank_data[figure_position].dx;
            figure_list[temp_figure].prev_grid_y = line_flank_data[figure_position].dy + (unit_list[temp_unit].y - flank_mode * 2);
            target_col -= 2;
        } else {
            figure_list[temp_figure].state_idx = 7;
            figure_list[temp_figure].prev_grid_x = target_col + line_flank_data[figure_position].dx;
            figure_list[temp_figure].prev_grid_y = line_flank_data[figure_position].dy + (unit_list[temp_unit].y - 0x10);
        }
        figure_position++;
    }
}


// Order temp_unit to move by (offset_x, offset_y), set combat order 3, and arm its AI flag.
// FUNCTION: C2 0x504df
// FUNCTION: C2WIN 0x0047ce22
void set_ai_unit_move(int offset_x, int offset_y)
{
    int y_offset;
    signed char new_y;

    y_offset = offset_y;

    unit_list[temp_unit].combat_order = 3;
    unit_list[temp_unit].manoeuvre_done = 1;
    for (temp_figure = unit_list[temp_unit].first_figure;
         temp_figure <= unit_list[temp_unit].last_figure;
         temp_figure++) {
        if (figure_list[temp_figure].exists == 0) continue;
        figure_list[temp_figure].is_defending = 0;
        if (figure_list[temp_figure].state_idx == 2
            || figure_list[temp_figure].state_idx == 0xc
            || figure_list[temp_figure].state_idx == 0xa)
            continue;
        if (figure_list[temp_figure].grid_y < 6) {
            figure_list[temp_figure].state_idx = 0xa;
            return;
        }
        figure_list[temp_figure].state_idx = 3;
        figure_list[temp_figure].prev_grid_x = figure_list[temp_figure].grid_x + offset_x;
        new_y = (signed char)(figure_list[temp_figure].grid_y + y_offset);
        figure_list[temp_figure].prev_grid_y = new_y;
        if (new_y <= 2) {
            figure_list[temp_figure].state_idx = 0xa;
            return;
        }
    }
}

// Order temp_unit to withdraw by (offset_x, offset_y), set combat order 8, and arm its withdraw flag.
// FUNCTION: C2 0x505aa
// FUNCTION: C2WIN 0x0047d04d
void set_ai_unit_withdraw(int offset_x, int offset_y)
{
    int figure_state;

    unit_list[temp_unit].combat_order = 8;
    unit_list[temp_unit].withdraw_flag = 1;
    for (temp_figure = unit_list[temp_unit].first_figure;
         unit_list[temp_unit].last_figure >= temp_figure;
         ++temp_figure) {
        if (figure_list[temp_figure].exists != 0) {
            figure_list[temp_figure].is_defending = 0;
            figure_state = figure_list[temp_figure].state_idx;
            if (figure_state != 0xc) {
                figure_list[temp_figure].state_idx = 8;
                figure_list[temp_figure].prev_grid_x = figure_list[temp_figure].grid_x + (char)offset_x;
                figure_list[temp_figure].prev_grid_y = figure_list[temp_figure].grid_y + (char)offset_y;
            }
        }
    }
}

// Order every eligible figure in the current unit to attack berserk.
// FUNCTION: C2 0x50646
// FUNCTION: C2WIN 0x0047d1b6
void set_ai_unit_beserk(void)
{
    int figure_state;

    unit_list[temp_unit].combat_order = 10;
    for (temp_figure = unit_list[temp_unit].first_figure;
         unit_list[temp_unit].last_figure >= temp_figure;
         ++temp_figure) {
        if (figure_list[temp_figure].exists != 0) {
            figure_list[temp_figure].is_defending = 0;
            figure_state = figure_list[temp_figure].state_idx;
            if (figure_state != 2 && figure_state != 0xc && figure_state != 4) {
                figure_list[temp_figure].state_idx = 10;
            }
        }
    }
}

// Queue a staggered berserk attack for every eligible figure in the current unit.
// FUNCTION: C2 0x506c5
// FUNCTION: C2WIN 0x0047d2f0
void set_ai_unit_delayed_beserk(void)
{
    int figure_state;

    unit_list[temp_unit].combat_order = 10;
    for (temp_figure = unit_list[temp_unit].first_figure;
         unit_list[temp_unit].last_figure >= temp_figure;
         ++temp_figure) {
        if (figure_list[temp_figure].exists != 0) {
            figure_list[temp_figure].is_defending = 0;
            figure_state = figure_list[temp_figure].state_idx;
            if (figure_state != 2 && figure_state != 0xc && figure_state != 4) {
                figure_list[temp_figure].state_idx = 1;
                figure_list[temp_figure].wait_counter =
                    ((temp_figure & 3) + 2);
                figure_list[temp_figure].next_state_idx = 10;
            }
        }
    }
}

// Put every active figure in temp_unit into auto-fire state unless it is already in one of the
// excluded combat states. Also clears the per-figure defense flag and snapshots current grid
// position into prev_grid_x/prev_grid_y for the auto-fire animation.
// FUNCTION: C2 0x50760
// FUNCTION: C2WIN 0x0047d464
void set_ai_unit_auto_fire(void)
{
    temp_figure = unit_list[temp_unit].first_figure;
    for ( ; temp_figure <= unit_list[temp_unit].last_figure; temp_figure++) {
    if (figure_list[temp_figure].exists != 0) {
            figure_list[temp_figure].is_defending = 0;
            if (figure_list[temp_figure].state_idx != 2) {
                if (figure_list[temp_figure].state_idx != 4) {
                    if (figure_list[temp_figure].state_idx != 8) {
                        if (figure_list[temp_figure].state_idx != 3) {
                            if (figure_list[temp_figure].state_idx != 0xc) {
                                if (figure_list[temp_figure].state_idx != 0xe) {
                                    figure_list[temp_figure].state_idx = 0xe;
                                    figure_list[temp_figure].prev_grid_y = figure_list[temp_figure].grid_y - 1;
                                    figure_list[temp_figure].prev_grid_x = figure_list[temp_figure].grid_x;
                                }
                            }
                        }
                    }
                }
            }
    }
    }
}

// Update unit morale, fatigue, recovery, and routing decisions.
// FUNCTION: C2 0x50806
// FUNCTION: C2WIN 0x0047d64e
void update_units_morale(void)
{
    struct unit_rec *unit_ptr;
    int losses_pct;
    int loss_tier;
    unsigned char morale_drop;

    for (temp_unit = 1; temp_unit < 0x33; temp_unit++) {
        if (unit_list[temp_unit].exists == 0)        continue;
        if (unit_list[temp_unit].combat_order == 0xc) continue;

        losses_pct = valueDIVtotal(
            unit_list[temp_unit].start_men - unit_list[temp_unit].current_men,
            unit_list[temp_unit].start_men);
        loss_tier = losses_pct / 5;
        if (loss_tier > unit_list[temp_unit].loss_tier) {
            morale_drop = losses_to_morale[unit_list[temp_unit].loss_tier
                                           + (unit_list[temp_unit].owner - 1) * 5 * 4];
            unit_list[temp_unit].morale_a -= morale_drop;
            unit_list[temp_unit].morale_b -= morale_drop / 3;
            unit_list[temp_unit].loss_tier++;
        }

        if (unit_list[temp_unit].fatigue > 0x14) {
            unit_list[temp_unit].fatigue -= 5;
            if (unit_list[temp_unit].fatigue > 0x32) unit_list[temp_unit].fatigue = 0x32;
            unit_list[temp_unit].morale_a--;
            unit_list[temp_unit].fatigue_alert = 1;
        }

        if (unit_list[temp_unit].target_lock == 0) {
            unit_list[temp_unit].regen_tick++;
            if (unit_list[temp_unit].regen_tick > 0x19) {
                unit_list[temp_unit].regen_tick = 0;
                if (unit_list[temp_unit].morale_a < unit_list[temp_unit].morale_b) unit_list[temp_unit].morale_a++;
            }
        }

        if (unit_list[temp_unit].morale_a <= 10) {
            set_unit_to_rout(temp_unit);
            drop_all_units_morale(unit_list[temp_unit].type, 16, 6);
            raise_all_units_morale(unit_list[temp_unit].type, 10, 8);
        }
    }
}

// Lower morale for active units of the specified type.
// FUNCTION: C2 0x509c4
// FUNCTION: C2WIN 0x0047da3d
void drop_all_units_morale(int match_type, int morale_a_delta, int morale_b_delta)
{
    int unit_idx;
    for (unit_idx = 1; unit_idx < 0x33; unit_idx++) {
        if (unit_list[unit_idx].exists == 0) continue;
        if ((unsigned char)unit_list[unit_idx].type != match_type) continue;
        if (unit_list[unit_idx].unit_rank == 2)
            unit_list[unit_idx].morale_a -= 10;
        unit_list[unit_idx].morale_a -= (char)morale_a_delta;
        unit_list[unit_idx].morale_b -= (char)morale_b_delta;
        if (unit_list[unit_idx].morale_a < 0) unit_list[unit_idx].morale_a = 0;
        if (unit_list[unit_idx].morale_b < 0) unit_list[unit_idx].morale_b = 0;
    }
}

// Raise both morale values for every active unit whose type differs from `skip_type`.
// FUNCTION: C2 0x50a44
// FUNCTION: C2WIN 0x0047dbad
void raise_all_units_morale(int skip_type, int morale_a_delta, int morale_b_delta)
{
    int unit_idx;
    for (unit_idx = 1; unit_idx < 0x33; unit_idx++) {
        if (unit_list[unit_idx].exists == 0) continue;
        if ((unsigned char)unit_list[unit_idx].type == skip_type) continue;
        unit_list[unit_idx].morale_a += (char)morale_a_delta;
        unit_list[unit_idx].morale_b += (char)morale_b_delta;
        if (unit_list[unit_idx].morale_a > 0x64)
            unit_list[unit_idx].morale_a = 0x64;
        if (unit_list[unit_idx].morale_b > 0x64)
            unit_list[unit_idx].morale_b = 0x64;
    }
}

// Force `unit_idx` to rout: zero its primary morale axis, set combat_order=0x0c, and walk all
// member figures.
// FUNCTION: C2 0x50ab5
// FUNCTION: C2WIN 0x0047dcd4
void set_unit_to_rout(int unit_idx)
{
    int figure_state;

    unit_list[unit_idx].morale_a = 0;
    unit_list[unit_idx].combat_order = 0xc;
    for (temp_figure = unit_list[unit_idx].first_figure;
         unit_list[unit_idx].last_figure >= temp_figure;
         ++temp_figure) {
        if (figure_list[temp_figure].exists != 0) {
            figure_list[temp_figure].is_defending = 0;
            figure_state = figure_list[temp_figure].state_idx;
            if (figure_state != 2) {
                figure_list[temp_figure].state_idx = 0xc;
                figure_list[temp_figure].prev_grid_x = figure_list[temp_figure].grid_x;
                if (figure_list[temp_figure].unit_position == -1)
                    figure_list[temp_figure].prev_grid_y = -1;
                else
                    figure_list[temp_figure].prev_grid_y = 0x34;
            }
        }
    }
    battle_tune_mood_from_type(unit_idx);
}

// Recompute unit membership, army totals, morale, selection statistics, and Roman map bounds.
// FUNCTION: C2 0x50b57
// FUNCTION: C2WIN 0x0047de58
void get_units_status(void)
{
    int unit_idx;

    our_battle_men         = 0;
    their_battle_men       = 0;
    our_battle_morale      = 0;
    their_battle_morale    = 0;
    our_battle_units       = 0;
    their_battle_units     = 0;
    battle_stats_nof_units = 0;
    battle_stats_men       = 0;
    battle_stats_start_men = 0;
    battle_stats_morale    = 0;
    battle_stats_type      = 0;

    roman_left_edge  = 0x33;
    roman_right_edge = 0;
    roman_back_edge  = 0x33;
    roman_front_edge = 0;

    /* ---- Pass 1: clear per-unit fields for alive units. ---- */
    for (temp_unit = 1; temp_unit < 0x33; temp_unit++) {
        if (unit_list[temp_unit].exists == 0) continue;
        unit_list[temp_unit].target_lock       = 0;
        unit_list[temp_unit].has_selected_figs = 0;
        unit_list[temp_unit].first_figure      = 0;
        unit_list[temp_unit].fig_count         = 0;
        unit_list[temp_unit].last_figure       = 0;
        unit_list[temp_unit].current_men       = 0;
    }

    /* ---- Pass 2: per-figure accumulate. ---- */
    for (temp_figure = 1; temp_figure < 0xc9; temp_figure++) {
        if (figure_list[temp_figure].exists == 0) continue;

        unit_idx = figure_list[temp_figure].unit_ref;
        figure_list[temp_figure].unit_type = unit_list[unit_idx].fig_count;

        if (figure_list[temp_figure].state_idx == 4)
            unit_list[unit_idx].target_lock++;

        if (figure_list[temp_figure].selected != 0)
            unit_list[unit_idx].has_selected_figs = 1;

        if (unit_list[unit_idx].first_figure == 0) {
            unit_list[unit_idx].first_figure = temp_figure;
            unit_list[unit_idx].x = figure_list[temp_figure].grid_x;
            unit_list[unit_idx].y = figure_list[temp_figure].grid_y;
        }

        unit_list[unit_idx].fig_count++;
        unit_list[unit_idx].last_figure = temp_figure;
        unit_list[unit_idx].current_men += figure_list[temp_figure].stampede_flag;

        if (figure_list[temp_figure].owner != 0) {
            if (figure_list[temp_figure].grid_x < roman_left_edge)  roman_left_edge  = figure_list[temp_figure].grid_x;
            if (figure_list[temp_figure].grid_x > roman_right_edge) roman_right_edge = figure_list[temp_figure].grid_x;
            if (figure_list[temp_figure].grid_y < roman_back_edge)  roman_back_edge  = figure_list[temp_figure].grid_y;
            if (figure_list[temp_figure].grid_y > roman_front_edge) roman_front_edge = figure_list[temp_figure].grid_y;
        }
    }

    /* ---- Pass 3: per-unit finalize + battle_stats. ---- */
    for (temp_unit = 1; temp_unit < 0x33; temp_unit++) {
        if (unit_list[temp_unit].exists == 0) continue;

        if (unit_list[temp_unit].fig_count == 0) {
            unit_list[temp_unit].exists = 0;
            battle_tune_mood_from_type(temp_unit);
        }

        if (unit_list[temp_unit].has_selected_figs != 0 && unit_list[temp_unit].type != 0) {
            battle_stats_control = 1;
            battle_stats_nof_units += 1;
            battle_stats_men       += unit_list[temp_unit].current_men;
            battle_stats_start_men += unit_list[temp_unit].start_men;
            battle_stats_morale    += unit_list[temp_unit].morale_a;
            if (battle_stats_type == 0) {
                battle_stats_type = unit_list[temp_unit].owner;
                if (battle_stats_type > 4)
                    battle_stats_type = 5;
            } else if (unit_list[temp_unit].owner != battle_stats_type) {
                battle_stats_type = 4;
            }
        }
        if (unit_list[temp_unit].has_selected_figs != 0 && unit_list[temp_unit].type == 0) {
            battle_stats_nof_units = 1;
            battle_stats_men       = unit_list[temp_unit].current_men;
            battle_stats_start_men = unit_list[temp_unit].start_men;
            battle_stats_morale    = unit_list[temp_unit].morale_a;
            battle_stats_type      = unit_list[temp_unit].owner;
            battle_stats_control   = 0;
        }

        if (unit_list[temp_unit].type != 0) {
            our_battle_men    += unit_list[temp_unit].current_men;
            our_battle_morale += unit_list[temp_unit].morale_a;
            our_battle_units++;
        } else {
            their_battle_men    += unit_list[temp_unit].current_men;
            their_battle_morale += unit_list[temp_unit].morale_a;
            their_battle_units++;
        }
    }

    get_battle_odds();

    if (our_battle_units != 0)
        our_battle_morale /= our_battle_units;
    if (their_battle_units != 0)
        their_battle_morale /= their_battle_units;
    if (battle_stats_nof_units != 0)
        battle_stats_morale /= battle_stats_nof_units;

}

// Convert unit type/owner into the battle music mood bucket and hold it. Units with non-zero .type
// use moods 6..9 by .owner; zero-type barbarian / animal / siege unit classes map to the later
// mood range.
// FUNCTION: C2 0x50f6d
// FUNCTION: C2WIN 0x0047e676
void battle_tune_mood_from_type(int unit_idx)
{
    int owner;

    if (unit_list[unit_idx].type != 0) {
        owner = unit_list[unit_idx].owner;
        if (owner == 1) tune_mood = 6;
        else if (owner == 2) tune_mood = 7;
        else if (owner == 3) tune_mood = 8;
        else tune_mood = 9;
    } else {
        owner = unit_list[unit_idx].owner;
        if (owner == 5) tune_mood = 0xd;
        else if (owner == 7) tune_mood = 0xd;
        else if (owner == 8) tune_mood = 0xd;
        else if (owner == 6) tune_mood = 0xc;
        else if (owner == 0xb) tune_mood = 0xe;
        else if (owner == 0xc) tune_mood = 0xe;
        else if (owner == 0xd) tune_mood = 0xe;
        else if (owner == 0xe) tune_mood = 0xf;
        else if (owner == 0xf) tune_mood = 0x10;
        else tune_mood = 0xb;
    }
    tune_mood_hold = 1;
}

// Prepare every eligible figure in a unit to seek or enter combat.
// FUNCTION: C2 0x5105b
// FUNCTION: C2WIN 0x0047e8ea
void set_unit_to_fight(int figure_idx)
{
    int   figure_state;

    temp_unit = figure_list[figure_idx].unit_ref;

    for (temp_figure = unit_list[temp_unit].first_figure;
         temp_figure <= unit_list[temp_unit].last_figure;
         temp_figure++) {
        if (figure_list[temp_figure].exists != 0) {
            if (figure_list[temp_figure].owner == 0) {
                figure_list[figure_no].is_defending = 0;
            }
            if (figure_list[temp_figure].is_defending != 0
                && unit_list[temp_unit].target_lock == 0
                && figure_list[temp_figure].backtrack_flag != 0)
            {
                figure_list[temp_figure].is_visible |= 1;
                backtrack_figure(temp_figure);
                figure_list[temp_figure].direction =
                    figure_list[figure_no].backtrack_dirc;
                figure_list[temp_figure].wf_step_x = 0;
                figure_list[temp_figure].backtrack_flag = 0;
            }
            figure_state = figure_list[temp_figure].state_idx;
            if (figure_state != 4 && figure_state != 2 && figure_state != 0xc) {
                if (figure_list[temp_figure].is_defending != 0) {
                    figure_list[temp_figure].state_idx = 9;
                } else {
                    figure_list[temp_figure].state_idx = 0xa;
                }
            }
        }
    }
}

// Advance the current figure toward its destination and handle obstacles or combat.
// FUNCTION: C2 0x51189
// FUNCTION: C2WIN 0x0047eb78
int figure_go_to_target(void)
{
    int move_result;
    int swap_result;

    fig_at_edge = 0;

    if (figure_list[figure_no].is_visible & 1) {
        figure_list[figure_no].wf_step_x = 0;
        figure_list[figure_no].wf_step_y = 0;
    } else {
        figure_list[figure_no].backtrack_flag = 0;
        move_result = (unsigned char)figure_list[figure_no].stampede_kind;
        set_battle_march_fx(figure_list[figure_no].sprite_type);

        figure_list[figure_no].wf_step_y++;
        if (figure_list[figure_no].wf_step_y <= move_result)
            goto movement_wait;
        figure_list[figure_no].wf_step_y = 0;
        figure_list[figure_no].wf_step_x++;
        if (figure_list[figure_no].wf_step_x <= 7)
            goto movement_wait;
        figure_list[figure_no].is_visible |= 1;
        figure_list[figure_no].wf_step_x = 0;
        goto movement_ready;
movement_wait:
        return 0;
movement_ready:
        ;
    }

    /* Choose a heading toward the destination. */
    if (figure_list[figure_no].is_routing == 0)
        return 1;

    if (figure_list[figure_no].wf_searching != 0
        && --figure_list[figure_no].wf_ttl <= 0)
        figure_list[figure_no].wf_searching = 0;
    if (figure_list[figure_no].wf_searching == 0) {
        w_dirc = get_heading(figure_list[figure_no].grid_x, figure_list[figure_no].grid_y,
                             figure_list[figure_no].prev_grid_x, figure_list[figure_no].prev_grid_y,
                             figure_list[figure_no].direction);
    } else {
        w_dirc = figure_list[figure_no].wf_dirc;
    }

    if (w_dirc >= 8) {
        figure_list[figure_no].is_routing = 0;
        figure_list[figure_no].is_visible |= 2;
        return 1;
    }

    /* Test the next step and resolve any obstruction. */
    move_result = try_a_battlemap_square(w_dirc);
    if (move_result == 0) {
        if (figure_list[enemy_figure].state_idx == 2) {
            move_result = get_wf_dirc(1);
        } else if (figure_list[figure_no].is_defending != 0) {
            /* Same unit: do not fight self. */
            if ((figure_list[enemy_figure].unit_ref) != (figure_list[figure_no].unit_ref)) {
                if (figure_list[enemy_figure].state_idx != 0xf && figure_list[enemy_figure].next_state_idx != 0xf) {
                    move_result = get_wf_dirc(2);
                } else {
                    figure_list[figure_no].next_state_idx = figure_list[figure_no].state_idx;
                    figure_list[figure_no].state_idx = 1;
                    figure_list[figure_no].wait_counter = 1;
                    get_fig_still_image();
                    return 0;
                }
            } else {
                figure_list[figure_no].next_state_idx = figure_list[figure_no].state_idx;
                figure_list[figure_no].state_idx = 1;
                figure_list[figure_no].wait_counter = 1;
                get_fig_still_image();
                return 0;
            }
        } else if (figure_list[figure_no].state_idx == 7) {
            swap_result = swap_2_figures();
            if (swap_result == 2) {
                figure_list[enemy_figure].state_idx = 1;
                figure_list[enemy_figure].next_state_idx = 7;
                figure_list[enemy_figure].wait_counter = 2;
                figure_list[enemy_figure].is_defending = 0;
                figure_list[enemy_figure].prev_grid_x = figure_list[figure_no].grid_x;
                figure_list[enemy_figure].prev_grid_y = figure_list[figure_no].grid_y;
                return 0;
            }
            if (swap_result != 0)
                goto cap_wander;
            figure_list[figure_no].state_idx = 1;
            figure_list[figure_no].next_state_idx = 7;
            figure_list[figure_no].wait_counter = 1;
            get_fig_still_image();
            return 0;
cap_wander:
            move_result = get_wf_dirc(2);
        } else {
            move_result = get_wf_dirc(1);
            if (move_result == 0) {
                get_fig_still_image();
            }
        }
    }

    if (move_result == 0x3e7) {
        /* Engage the blocking enemy. */
        if (figure_list[figure_no].state_idx == 2)
            return 0;
        if (figure_list[figure_no].state_idx == 7) {
            move_result = get_wf_dirc(0);
        } else if (figure_list[figure_no].state_idx == 8) {
            move_result = get_wf_dirc(0);
        } else if (figure_list[enemy_figure].state_idx == 2) {
            move_result = get_wf_dirc(0);
        } else {
            set_unit_to_fight(figure_no);
            figure_list[figure_no].state_idx = 4;
            figure_list[figure_no].fight_direction = w_dirc;
            figure_list[figure_no].opponent = enemy_figure;
            figure_list[figure_no].fight_role = 1;
            set_attack_count(figure_no);
            if (figure_list[enemy_figure].state_idx != 4) {
                set_unit_to_fight(enemy_figure);
                figure_list[enemy_figure].state_idx = 4;
                figure_list[enemy_figure].fight_direction = (w_dirc + 4) % 8;
                figure_list[enemy_figure].opponent = figure_no;
                figure_list[enemy_figure].fight_role = 2;
                set_defense_shield(enemy_figure);
            }
            return 0;
        }
    }

    if (move_result == 0) {
        if (figure_list[figure_no].state_idx == 2)
            return 0;
        figure_list[figure_no].next_state_idx = figure_list[figure_no].state_idx;
        figure_list[figure_no].state_idx = 1;
        figure_list[figure_no].wait_counter = 5;
        return 0;
    }
    if (move_result == 0x3e7)
        return 0;

    /* Commit the movement step. */
    figure_list[figure_no].is_visible &= 0xfe;
    figure_list[figure_no].backtrack_dirc = figure_list[figure_no].direction;
    figure_list[figure_no].direction = w_dirc;
    figure_list[figure_no].wf_step_x = 1;
    move_figure(figure_no);
    figure_list[figure_no].backtrack_flag = 1;
    return 1;
}

// Swap positions when two compatible figures from the same unit block each other.
// FUNCTION: C2 0x515e0
// FUNCTION: C2WIN 0x0047f57d
int swap_2_figures(void)
{
    int   temp_y;
    int   temp_x;
    int   temp_map_ref;

    if (figure_list[enemy_figure].unit_ref != figure_list[figure_no].unit_ref) return 1;

    if (figure_list[enemy_figure].state_idx == 1) return 1;
    if (figure_list[enemy_figure].state_idx != 6) return 0;

    temp_x       = figure_list[figure_no].grid_x;
    temp_y       = figure_list[figure_no].grid_y;
    temp_map_ref = figure_list[figure_no].map_ref;

    figure_list[figure_no].grid_x  = figure_list[enemy_figure].grid_x;
    figure_list[figure_no].grid_y  = figure_list[enemy_figure].grid_y;
    figure_list[figure_no].map_ref = figure_list[enemy_figure].map_ref;

    figure_list[enemy_figure].grid_x  = temp_x;
    figure_list[enemy_figure].grid_y  = temp_y;
    figure_list[enemy_figure].map_ref = temp_map_ref;

    (*(struct battle_cell *)((unsigned char *)battle_map + (figure_list[figure_no].map_ref))).figure = figure_no;
    (*(struct battle_cell *)((unsigned char *)battle_map + (figure_list[enemy_figure].map_ref))).figure = enemy_figure;
    return 2;
}

// Test the battle-map destination one tile from `figure_no`, recording when the move reaches an edge.
// FUNCTION: C2 0x516cb
// FUNCTION: C2WIN 0x0047f786
int try_a_battlemap_square(int direction)
{
    int result;
    result = 0;
    switch (direction) {
    case 0:  /* N */
        if (figure_list[figure_no].grid_y <= 0) {
            fig_at_edge = 1;
            result = 0;
        } else {
            result = try_this_battlemap_square(figure_list[figure_no].map_ref - 0xd0);
        }
        break;
    case 1:  /* NE */
        if (figure_list[figure_no].grid_x < 0x33) {
            if (figure_list[figure_no].grid_y <= 0) {
                fig_at_edge = 1;
                result = 0;
            } else {
                result = try_this_battlemap_square(figure_list[figure_no].map_ref - 0xcc);
            }
        } else {
            fig_at_edge = 1;
            result = 0;
        }
        break;
    case 2:  /* E */
        if (figure_list[figure_no].grid_x < 0x33) {
            result = try_this_battlemap_square(figure_list[figure_no].map_ref + 4);
        } else {
            fig_at_edge = 1;
            result = 0;
        }
        break;
    case 3:  /* SE */
        if (figure_list[figure_no].grid_x < 0x33) {
            if (figure_list[figure_no].grid_y < 0x33) {
                result = try_this_battlemap_square(figure_list[figure_no].map_ref + 0xd4);
            } else {
                fig_at_edge = 1;
                result = 0;
            }
        } else {
            fig_at_edge = 1;
            result = 0;
        }
        break;
    case 4:  /* S */
        if (figure_list[figure_no].grid_y < 0x33) {
            result = try_this_battlemap_square(figure_list[figure_no].map_ref + 0xd0);
        } else {
            fig_at_edge = 1;
            result = 0;
        }
        break;
    case 5:  /* SW */
        if (figure_list[figure_no].grid_x <= 0) {
            fig_at_edge = 1;
            result = 0;
        } else if (figure_list[figure_no].grid_y < 0x33) {
            result = try_this_battlemap_square(figure_list[figure_no].map_ref + 0xcc);
        } else {
            fig_at_edge = 1;
            result = 0;
        }
        break;
    case 6:  /* W */
        if (figure_list[figure_no].grid_x <= 0) {
            fig_at_edge = 1;
            result = 0;
        } else {
            result = try_this_battlemap_square(figure_list[figure_no].map_ref - 4);
        }
        break;
    case 7:  /* NW */
        if (figure_list[figure_no].grid_x <= 0) {
            fig_at_edge = 1;
            result = 0;
        } else if (figure_list[figure_no].grid_y <= 0) {
            fig_at_edge = 1;
            result = 0;
        } else {
            result = try_this_battlemap_square(figure_list[figure_no].map_ref - 0xd4);
        }
        break;
    }
    return result;
}

// Test whether a battle-map cell is free, friendly, or occupied by an enemy.
// FUNCTION: C2 0x5185c
// FUNCTION: C2WIN 0x0047fb65
int try_this_battlemap_square(int cell_offset)
{
    int sprite_type;
    int figure_state;

    enemy_figure = (*(struct battle_cell *)((unsigned char *)battle_map + ((cell_offset)))).figure;
    if (enemy_figure != 0) {
        sprite_type = figure_list[figure_no].sprite_type;
        if (sprite_type == 0xf) {
            figure_state = figure_list[figure_no].state_idx;
            if (figure_state == 2) {
                figure_list[enemy_figure].state_idx   = 2;
                figure_list[enemy_figure].death_timer = 0x1e;
                return 0;
            }
            if (figure_list[enemy_figure].owner == figure_list[figure_no].owner) {
                return 0;
            }
            figure_list[enemy_figure].state_idx   = 2;
            figure_list[enemy_figure].death_timer = 0x1e;
            return 0;
        }
        if (figure_list[enemy_figure].owner == figure_list[figure_no].owner) {
            return 0;
        }
        return 0x3e7;
    }
    return 1;
}

// Move a figure forward one cell, removing it if the destination is already occupied.
// FUNCTION: C2 0x51927
// FUNCTION: C2WIN 0x0047fccd
void move_figure(int figure_idx)
{
    int   old_cell_offset = figure_list[figure_idx].map_ref;
    int   new_cell_offset;
    int   previous_occupant;

    previous_occupant = ((unsigned char *)battle_map)[(old_cell_offset) + 1];
    if (previous_occupant == figure_idx) {
        ((unsigned char *)battle_map)[(old_cell_offset) + 1] = 0;
    }

    switch ((unsigned char)figure_list[figure_idx].direction) {
    case 0:
        figure_list[figure_idx].grid_y--;
        figure_list[figure_idx].map_ref -= 0xd0;
        break;
    case 1:
        figure_list[figure_idx].grid_y--;
        figure_list[figure_idx].grid_x++;
        figure_list[figure_idx].map_ref -= 0xcc;
        break;
    case 2:
        figure_list[figure_idx].grid_x++;
        figure_list[figure_idx].map_ref += 0x04;
        break;
    case 3:
        figure_list[figure_idx].grid_y++;
        figure_list[figure_idx].grid_x++;
        figure_list[figure_idx].map_ref += 0xd4;
        break;
    case 4:
        figure_list[figure_idx].grid_y++;
        figure_list[figure_idx].map_ref += 0xd0;
        break;
    case 5:
        figure_list[figure_idx].grid_y++;
        figure_list[figure_idx].grid_x--;
        figure_list[figure_idx].map_ref += 0xcc;
        break;
    case 6:
        figure_list[figure_idx].grid_x--;
        figure_list[figure_idx].map_ref -= 0x04;
        break;
    case 7:
        figure_list[figure_idx].grid_y--;
        figure_list[figure_idx].grid_x--;
        figure_list[figure_idx].map_ref -= 0xd4;
        break;
    default:
        return;
    }
    new_cell_offset = figure_list[figure_idx].map_ref;

    if (((unsigned char *)battle_map)[(new_cell_offset) + 1] == 0) {
        ((unsigned char *)battle_map)[(new_cell_offset) + 1] = figure_idx;
        return;
    }
    low_beep();
    remove_figure(figure_idx);
}

// Step a figure backward one cell along its current direction (used when a forward move was
// illegal).
// FUNCTION: C2 0x51a5f
// FUNCTION: C2WIN 0x0047ffbe
void backtrack_figure(int figure_idx)
{
    int   old_cell_offset = figure_list[figure_idx].map_ref;
    int   new_cell_offset;
    int   previous_occupant;

    previous_occupant = ((unsigned char *)battle_map)[(old_cell_offset) + 1];
    if (previous_occupant == figure_idx) {
        ((unsigned char *)battle_map)[(old_cell_offset) + 1] = 0;
    }

    switch ((unsigned char)figure_list[figure_idx].direction) {
    case 4:
        figure_list[figure_idx].grid_y--;
        figure_list[figure_idx].map_ref -= 0xd0;
        break;
    case 5:
        figure_list[figure_idx].grid_y--;
        figure_list[figure_idx].grid_x++;
        figure_list[figure_idx].map_ref -= 0xcc;
        break;
    case 6:
        figure_list[figure_idx].grid_x++;
        figure_list[figure_idx].map_ref += 0x04;
        break;
    case 7:
        figure_list[figure_idx].grid_y++;
        figure_list[figure_idx].grid_x++;
        figure_list[figure_idx].map_ref += 0xd4;
        break;
    case 0:
        figure_list[figure_idx].grid_y++;
        figure_list[figure_idx].map_ref += 0xd0;
        break;
    case 1:
        figure_list[figure_idx].grid_y++;
        figure_list[figure_idx].grid_x--;
        figure_list[figure_idx].map_ref += 0xcc;
        break;
    case 2:
        figure_list[figure_idx].grid_x--;
        figure_list[figure_idx].map_ref -= 0x04;
        break;
    case 3:
        figure_list[figure_idx].grid_y--;
        figure_list[figure_idx].grid_x--;
        figure_list[figure_idx].map_ref -= 0xd4;
        break;
    default:
        return;
    }
    new_cell_offset = figure_list[figure_idx].map_ref;
    ((unsigned char *)battle_map)[(new_cell_offset) + 1] = figure_idx;
}

// Set the current figure's destination to the adjacent cell in an eight-way direction.
// FUNCTION: C2 0x51b58
// FUNCTION: C2WIN 0x00480277
void target_from_figure_dirc(int direction)
{
    if (direction == 0) {
        figure_list[figure_no].prev_grid_x = figure_list[figure_no].grid_x;
        figure_list[figure_no].prev_grid_y = (figure_list[figure_no].grid_y - 1);
    } else if (direction == 2) {
        figure_list[figure_no].prev_grid_x = (figure_list[figure_no].grid_x + 1);
        figure_list[figure_no].prev_grid_y = figure_list[figure_no].grid_y;
    } else if (direction == 4) {
        figure_list[figure_no].prev_grid_x = figure_list[figure_no].grid_x;
        figure_list[figure_no].prev_grid_y = (figure_list[figure_no].grid_y + 1);
    } else if (direction == 6) {
        figure_list[figure_no].prev_grid_x = (figure_list[figure_no].grid_x - 1);
        figure_list[figure_no].prev_grid_y = figure_list[figure_no].grid_y;
    } else if (direction == 1) {
        figure_list[figure_no].prev_grid_x = (figure_list[figure_no].grid_x + 1);
        figure_list[figure_no].prev_grid_y = (figure_list[figure_no].grid_y - 1);
    } else if (direction == 3) {
        figure_list[figure_no].prev_grid_x = (figure_list[figure_no].grid_x + 1);
        figure_list[figure_no].prev_grid_y = (figure_list[figure_no].grid_y + 1);
    } else if (direction == 5) {
        figure_list[figure_no].prev_grid_x = (figure_list[figure_no].grid_x - 1);
        figure_list[figure_no].prev_grid_y = (figure_list[figure_no].grid_y + 1);
    } else if (direction == 7) {
        figure_list[figure_no].prev_grid_x = (figure_list[figure_no].grid_x - 1);
        figure_list[figure_no].prev_grid_y = (figure_list[figure_no].grid_y - 1);
    }
}

// Choose a passable direction toward the current figure's destination.
// FUNCTION: C2 0x51c64
// FUNCTION: C2WIN 0x004805cd
int get_wf_dirc(int search_mode)
{
  int attempt;
  int reverse_direction;
  int move_result;
  int initial_direction;
  int desired_direction;
  int search_direction;
  attempt = 0;
  initial_direction = figure_list[figure_no].direction;
  reverse_direction = (initial_direction + 4) % 8;
  desired_direction = get_heading(figure_list[figure_no].grid_x, figure_list[figure_no].grid_y, figure_list[figure_no].prev_grid_x, figure_list[figure_no].prev_grid_y, initial_direction);
  move_result = try_a_battlemap_square(desired_direction);
  if (move_result == 1)
  {
    w_dirc = desired_direction;
    figure_list[figure_no].wf_dirc = desired_direction;
    figure_list[figure_no].wf_searching = 0;
    return 1;
  }
  if (figure_list[figure_no].wf_searching == 0)
  {
    figure_list[figure_no].wf_ttl = 2;
    figure_list[figure_no].wf_searching = 1;
    figure_list[figure_no].wf_orient ^= 1;
  }
  else
  {
    figure_list[figure_no].wf_ttl--;
    if (figure_list[figure_no].wf_ttl <= 0)
    {
      figure_list[figure_no].wf_searching = 0;
    }
  }
  search_direction = w_dirc;
  figure_list[figure_no].wf_dirc = search_direction;
  while (attempt < 8)
  {
    if (search_mode == 1)
    {
      figure_list[figure_no].wf_dirc = wf_battle_dircs[attempt] + search_direction;
    }
    else
      if (search_mode == 2)
    {
      figure_list[figure_no].wf_dirc = wf_battle_dircs[attempt] + desired_direction;
    }
    else
    {
      if (figure_list[figure_no].wf_orient == 1)
      {
        figure_list[figure_no].wf_dirc++;
      }
      else
      {
        figure_list[figure_no].wf_dirc--;
      }
    }
    if (figure_list[figure_no].wf_dirc >= 8)
      figure_list[figure_no].wf_dirc = 0;
    if (figure_list[figure_no].wf_dirc < 0)
      figure_list[figure_no].wf_dirc = 7;
    if (figure_list[figure_no].wf_dirc != reverse_direction)
    {
      w_dirc = figure_list[figure_no].wf_dirc;
      move_result = try_a_battlemap_square(w_dirc);
      if (search_mode != 0)
      {
        if (move_result != 0)
          return move_result;
      }
      else
      {
        if ((move_result != 0) && (move_result < 0x3e7))
          return 1;
      }
    }
    attempt++;
    if ((attempt >= 2) && (figure_list[figure_no].state_idx == 0xa))
      break;
  }

  return 0;
}


// Advance the current projectile and resolve expiry or impact.
// FUNCTION: C2 0x51e5a
// FUNCTION: C2WIN 0x004809d7
void fly_to_target(void)
{
    int step_idx;
    int range_bonus;
    int damage;

    arrow_list[arrow_no].flight_age = arrow_list[arrow_no].flight_age + 1;
    if (arrow_list[arrow_no].flight_age > arrow_list[arrow_no].fire_speed) {
        clear_arrow(&arrow_list[arrow_no]);
        return;
    }

    for (step_idx = 0; step_idx < 2; step_idx++) {
        if (arrow_list[arrow_no].step_x + arrow_list[arrow_no].step_y <= 0) {
            loose_arrow_move();
        } else {
            bd(arrow_list[arrow_no].axis_dominant);
            if (arrow_list[arrow_no].axis_dominant == 2) {
                move_arrow_vert();
                if (arrow_list[arrow_no].step_error >= 0) {
                    arrow_list[arrow_no].step_x--;
                    move_arrow_horiz();
                }
            } else {
                move_arrow_horiz();
                if (arrow_list[arrow_no].step_error >= 0) {
                    arrow_list[arrow_no].step_y--;
                    move_arrow_vert();
                }
            }
        }

        if (arrow_off_map() != 0) {
            clear_arrow(&arrow_list[arrow_no]);
            return;
        }

        arrow_list[arrow_no].grid_x = arrow_list[arrow_no].start_x / 7;
        arrow_list[arrow_no].grid_y = arrow_list[arrow_no].start_y / 7;
        arrow_list[arrow_no].map_ref = (arrow_list[arrow_no].grid_y * 0x34 + arrow_list[arrow_no].grid_x) * 4;

        enemy_figure = ((unsigned char *)battle_map)[arrow_list[arrow_no].map_ref + 1];
        if (enemy_figure == 0) continue;
        if (figure_list[enemy_figure].state_idx == 2) continue;
        if (figure_list[enemy_figure].owner == arrow_list[arrow_no].owner) continue;

        temp_unit = figure_list[enemy_figure].unit_ref;
        unit_list[temp_unit].fatigue++;

        range_bonus = (arrow_list[arrow_no].fire_speed - arrow_list[arrow_no].flight_age) / 4;
        damage = (arrow_list[arrow_no].fire_range + range_bonus) / 0x14;

        if (figure_list[enemy_figure].defense > 0) {
            if (figure_list[enemy_figure].sub_state > 2) damage--;
            if (figure_list[enemy_figure].is_defending != 0 && figure_list[enemy_figure].shield_class == 2)
                damage--;
        }
        if (damage > 0) {
            figure_list[enemy_figure].kill_counter += damage;
            if (figure_list[enemy_figure].kill_counter >= 0xa) {
                figure_list[enemy_figure].kill_counter = 0;
                figure_list[enemy_figure].stampede_flag--;
                set_missile_fight_fx(arrow_list[arrow_no].weapon_kind);
            }
            if (figure_list[enemy_figure].stampede_flag <= 0) {
                figure_list[enemy_figure].stampede_flag = 0;
                figure_list[enemy_figure].state_idx = 2;
            }
        }

        clear_arrow(&arrow_list[arrow_no]);
    }

    {
        int cell_offset;
        cell_offset = arrow_list[arrow_no].map_ref; arrow_a = ((unsigned char *)battle_map)[cell_offset + 3];
        if (arrow_a != 0) {
            arrow_list[arrow_a].flight_done = arrow_no;
        } else {
            ((unsigned char *)battle_map)[cell_offset + 3] = arrow_no;
        }
    }
}

// Update the current projectile's Bresenham error term.
// FUNCTION: C2 0x521ab
// FUNCTION: C2WIN 0x00480f19
void bd(int dominant_axis)
{

    if (dominant_axis == 1) {
        if (arrow_list[arrow_no].step_error < 0) {
            arrow_list[arrow_no].step_error = 2 * arrow_list[arrow_no].step_y + arrow_list[arrow_no].step_error;
        } else {
            arrow_list[arrow_no].step_error = 2 * (arrow_list[arrow_no].step_y - arrow_list[arrow_no].step_x) + arrow_list[arrow_no].step_error;
        }
        arrow_list[arrow_no].step_x--;
    } else {
        if (arrow_list[arrow_no].step_error < 0) {
            arrow_list[arrow_no].step_error = 2 * arrow_list[arrow_no].step_x + arrow_list[arrow_no].step_error;
        } else {
            arrow_list[arrow_no].step_error = 2 * (arrow_list[arrow_no].step_x - arrow_list[arrow_no].step_y) + arrow_list[arrow_no].step_error;
        }
        arrow_list[arrow_no].step_y--;
    }
}

// Initialize a new projectile's Bresenham deltas, error term, and dominant axis.
// FUNCTION: C2 0x5227d
// FUNCTION: C2WIN 0x00481041
void init_bd(int start_x, int start_y, int end_x, int end_y)
{
    if (start_x > end_x) {
        arrow_list[arrow_no].step_x = start_x - end_x;
    } else {
        arrow_list[arrow_no].step_x = end_x - start_x;
    }
    if (start_y > end_y) {
        arrow_list[arrow_no].step_y = start_y - end_y;
    } else {
        arrow_list[arrow_no].step_y = end_y - start_y;
    }

    if (arrow_list[arrow_no].step_y > arrow_list[arrow_no].step_x) {
        arrow_list[arrow_no].step_error =
            arrow_list[arrow_no].step_x * 2 - arrow_list[arrow_no].step_y;
    } else if (arrow_list[arrow_no].step_x > arrow_list[arrow_no].step_y) {
        arrow_list[arrow_no].step_error =
            arrow_list[arrow_no].step_y * 2 - arrow_list[arrow_no].step_x;
    } else {
        arrow_list[arrow_no].step_error = 0;
    }

    /* vertical dominant: dx*2 < dy */
    if ((arrow_list[arrow_no].step_x << 1) < arrow_list[arrow_no].step_y) {
        if (arrow_list[arrow_no].heading == 1)
            arrow_list[arrow_no].heading = 0;
        else if (arrow_list[arrow_no].heading == 3)
            arrow_list[arrow_no].heading = 4;
        else if (arrow_list[arrow_no].heading == 5)
            arrow_list[arrow_no].heading = 4;
        else if (arrow_list[arrow_no].heading == 7)
            arrow_list[arrow_no].heading = 0;
    }

    /* horizontal dominant: dy*2 < dx */
    if ((arrow_list[arrow_no].step_y << 1) < arrow_list[arrow_no].step_x) {
        if (arrow_list[arrow_no].heading == 1)
            arrow_list[arrow_no].heading = 2;
        else if (arrow_list[arrow_no].heading == 3)
            arrow_list[arrow_no].heading = 2;
        else if (arrow_list[arrow_no].heading == 5)
            arrow_list[arrow_no].heading = 6;
        else if (arrow_list[arrow_no].heading == 7)
            arrow_list[arrow_no].heading = 6;
    }

    if (arrow_list[arrow_no].step_y > arrow_list[arrow_no].step_x) {
        arrow_list[arrow_no].axis_dominant = 2;
    } else {
        arrow_list[arrow_no].axis_dominant = 1;
    }
}

// Return 1 when the current projectile lies outside the 52×52 battle grid.
// FUNCTION: C2 0x52410
// FUNCTION: C2WIN 0x004813d6
int arrow_off_map(void)
{
    if (arrow_list[arrow_no].grid_x < 0) return 1;
    if (arrow_list[arrow_no].grid_y < 0) return 1;
    if (arrow_list[arrow_no].grid_x >= 0x34) return 1;
    if (arrow_list[arrow_no].grid_y >= 0x34) return 1;
    return 0;
}

// Step the current projectile's `start_y` one pixel toward `end_y`.
// FUNCTION: C2 0x52458
// FUNCTION: C2WIN 0x0048147a
void move_arrow_vert(void)
{
    if (arrow_list[arrow_no].start_y < arrow_list[arrow_no].end_y)
        arrow_list[arrow_no].start_y++;
    else if (arrow_list[arrow_no].start_y > arrow_list[arrow_no].end_y)
        arrow_list[arrow_no].start_y--;
}

// Step the current projectile's `start_x` one pixel toward `end_x`.
// FUNCTION: C2 0x524a4
// FUNCTION: C2WIN 0x00481506
void move_arrow_horiz(void)
{
    if (arrow_list[arrow_no].start_x < arrow_list[arrow_no].end_x)
        arrow_list[arrow_no].start_x++;
    else if (arrow_list[arrow_no].start_x > arrow_list[arrow_no].end_x)
        arrow_list[arrow_no].start_x--;
}

// Move the current projectile one grid step along its heading.
// FUNCTION: C2 0x524f0
// FUNCTION: C2WIN 0x00481592
void loose_arrow_move(void)
{
    if (arrow_list[arrow_no].heading == 0) {
        arrow_list[arrow_no].start_y--;
    } else if (arrow_list[arrow_no].heading == 1) {
        arrow_list[arrow_no].start_x++;
        arrow_list[arrow_no].start_y--;
    } else if (arrow_list[arrow_no].heading == 2) {
        arrow_list[arrow_no].start_x++;
    } else if (arrow_list[arrow_no].heading == 3) {
        arrow_list[arrow_no].start_x++;
        arrow_list[arrow_no].start_y++;
    } else if (arrow_list[arrow_no].heading == 4) {
        arrow_list[arrow_no].start_y++;
    } else if (arrow_list[arrow_no].heading == 5) {
        arrow_list[arrow_no].start_x--;
        arrow_list[arrow_no].start_y++;
    } else if (arrow_list[arrow_no].heading == 6) {
        arrow_list[arrow_no].start_x--;
    } else if (arrow_list[arrow_no].heading == 7) {
        arrow_list[arrow_no].start_x--;
        arrow_list[arrow_no].start_y--;
    }
}

// Resolve one tick of melee combat between figure_no (attacker) and enemy_figure.
// FUNCTION: C2 0x52582
// FUNCTION: C2WIN 0x0048185b
void do_the_fight(void)
{
    temp_figure = (short)(unsigned char)figure_list[enemy_figure].opponent;

    if (temp_figure == figure_no
        && figure_list[enemy_figure].fight_role == 1)
    {
        figure_list[figure_no].fight_role = 2;
    }

    if (figure_list[figure_no].defense <= 0) {
        figure_list[figure_no].kill_counter++;
        set_defense_shield(figure_no);
    }

    if (figure_list[figure_no].kill_counter >= 0x0a) {
        figure_list[figure_no].kill_counter -= 0x0a;
        figure_list[figure_no].stampede_flag--;
        set_battle_fight_fx(figure_list[figure_no].sprite_type);
    }

    if (figure_list[figure_no].stampede_flag <= 0) {
        figure_list[figure_no].state_idx = 2;
        return;
    }

    if (figure_list[figure_no].fight_role == 1) {
        if (figure_list[figure_no].morale != 0) {
            figure_list[enemy_figure].stampede_flag -= figure_list[figure_no].morale;
            if (figure_list[enemy_figure].stampede_flag < 0)
                figure_list[enemy_figure].stampede_flag = 0;
            figure_list[figure_no].morale = 0;
        }
        figure_list[enemy_figure].defense--;
        figure_list[figure_no].attack_count--;
        if (figure_list[figure_no].attack_count <= 0) {
            if (temp_figure == figure_no) {
                figure_list[enemy_figure].fight_role = 1;
                set_attack_count(enemy_figure);
                figure_list[figure_no].fight_role = 2;
            } else {
                set_attack_count(figure_no);
            }
        }
    }

    set_this_ambient(0x13);
}

// Derive a figure's attack count from its animation, rank, and defensive posture.
// FUNCTION: C2 0x526f9
// FUNCTION: C2WIN 0x00481b69
void set_attack_count(int figure_idx)
{
    temp_unit = figure_list[figure_idx].unit_ref;
    figure_list[figure_idx].attack_count = figure_list[figure_idx].anim_kind;

    if (figure_list[figure_idx].figure_rank == 1)
        figure_list[figure_idx].attack_count -= 2;
    if (figure_list[figure_idx].figure_rank == 2)
        figure_list[figure_idx].attack_count -= 2;

    if (figure_list[figure_idx].is_defending != 0) {
        if (figure_list[figure_idx].shield_class == 0) {
            if (figure_list[figure_idx].owner != 0)
                figure_list[figure_idx].attack_count += 6;
            else
                figure_list[figure_idx].attack_count += 4;
        }
        if (figure_list[figure_idx].shield_class == 1)
            figure_list[figure_idx].attack_count += 6;
    }
}

// Refresh a figure's defense value from its type and formation.
// FUNCTION: C2 0x5278e
// FUNCTION: C2WIN 0x00481d29
void set_defense_shield(int figure_idx)
{
    temp_unit = figure_list[figure_idx].unit_ref;
    figure_list[figure_idx].defense += figure_list[figure_idx].sub_state;
    if (figure_list[figure_idx].is_defending != 0
     && figure_list[figure_idx].shield_class == 2) {
        figure_list[figure_idx].defense += 2;
    }
}

// Return the direction of the first living enemy adjacent to the current figure.
// FUNCTION: C2 0x527cc
// FUNCTION: C2WIN 0x00481dea
int nearest_formation_enemy(void)
{
    if (figure_list[figure_no].grid_y > 0) {
        enemy_figure = ((unsigned char *)battle_map)[figure_list[figure_no].map_ref - 0xcf];   /* N */
        if (enemy_figure != 0
         && figure_list[enemy_figure].owner != figure_list[figure_no].owner
         && figure_list[enemy_figure].state_idx != 2)
            return 0;
        if (figure_list[figure_no].grid_x > 0) {                            /* NW */
            enemy_figure = ((unsigned char *)battle_map)[figure_list[figure_no].map_ref - 0xd3];
            if (enemy_figure != 0
             && figure_list[enemy_figure].owner != figure_list[figure_no].owner
             && figure_list[enemy_figure].state_idx != 2)
                return 7;
        }
        if (figure_list[figure_no].grid_x < 0x33) {                         /* NE */
            enemy_figure = ((unsigned char *)battle_map)[figure_list[figure_no].map_ref - 0xcb];
            if (enemy_figure != 0
             && figure_list[enemy_figure].owner != figure_list[figure_no].owner
             && figure_list[enemy_figure].state_idx != 2)
                return 1;
        }
    }
    if (figure_list[figure_no].grid_x > 0) {                                /* W */
        enemy_figure = ((unsigned char *)battle_map)[figure_list[figure_no].map_ref - 3];
        if (enemy_figure != 0
         && figure_list[enemy_figure].owner != figure_list[figure_no].owner
         && figure_list[enemy_figure].state_idx != 2)
            return 6;
    }
    if (figure_list[figure_no].grid_x < 0x33) {                             /* E */
        enemy_figure = ((unsigned char *)battle_map)[figure_list[figure_no].map_ref + 5];
        if (enemy_figure != 0
         && figure_list[enemy_figure].owner != figure_list[figure_no].owner
         && figure_list[enemy_figure].state_idx != 2)
            return 2;
    }
    if (figure_list[figure_no].grid_y < 0x33) {
        if (figure_list[figure_no].grid_x > 0) {                            /* SW */
            enemy_figure = ((unsigned char *)battle_map)[figure_list[figure_no].map_ref + 0xcd];
            if (enemy_figure != 0
             && figure_list[enemy_figure].owner != figure_list[figure_no].owner
             && figure_list[enemy_figure].state_idx != 2)
                return 5;
        }
        if (figure_list[figure_no].grid_x < 0x33) {                         /* SE */
            enemy_figure = ((unsigned char *)battle_map)[figure_list[figure_no].map_ref + 0xd5];
            if (enemy_figure != 0
             && figure_list[enemy_figure].owner != figure_list[figure_no].owner
             && figure_list[enemy_figure].state_idx != 2)
                return 3;
        }
        enemy_figure = ((unsigned char *)battle_map)[figure_list[figure_no].map_ref + 0xd1];   /* S */
        if (enemy_figure != 0
         && figure_list[enemy_figure].owner != figure_list[figure_no].owner
         && figure_list[enemy_figure].state_idx != 2)
            return 4;
    }
    return 8;
}

// Pick the closest still-fightable hostile figure for figure_no and set it up as our melee target.
// Distance is the Chebyshev metric (get_longest_distance), capped at 0x68 = "no candidate".
// FUNCTION: C2 0x52a8c
// FUNCTION: C2WIN 0x00482390
int find_nearest_enemy(void)
{
    int distance;
    int best_figure_idx;
    int prefer_cohesion;
    int best_distance;

    best_distance = 0x68;
    best_figure_idx = 0;
    prefer_cohesion = tribe_ai_data[bat_tribe].prefer_cohesion;

    if (figure_list[figure_no].owner != 0) prefer_cohesion = 0;

    for (temp_figure = 1; temp_figure < 0xc9; temp_figure++) {
        if (figure_list[temp_figure].exists == 0) continue;
        if (figure_list[temp_figure].owner == figure_list[figure_no].owner) continue;
        if (figure_list[temp_figure].state_idx == 2) continue;
        if (figure_list[temp_figure].state_idx == 0xc) continue;
        if (figure_list[temp_figure].engaged_count > 1) continue;

        distance = get_longest_distance(figure_list[figure_no].grid_x,
                                        figure_list[figure_no].grid_y,
                                        figure_list[temp_figure].grid_x,
                                        figure_list[temp_figure].grid_y);
        if (figure_list[temp_figure].sprite_type != 3 && prefer_cohesion == 1) distance += 10;

        if (distance < best_distance) {
            best_distance = distance;
            best_figure_idx = temp_figure;
        }
    }

    if (best_figure_idx == 0) return 0;

    temp_figure = best_figure_idx;
    figure_list[figure_no].prev_grid_x = figure_list[temp_figure].grid_x;
    figure_list[figure_no].prev_grid_y = figure_list[temp_figure].grid_y;
    figure_list[figure_no].state_idx = 0x0a;
    figure_list[figure_no].missile_target = temp_figure;
    figure_list[temp_figure].engaged_count++;
    return 1;
}

// Find the closest active enemy figure to figure_no within `max_distance`. Skips same-owner figures,
// death/rout states (2/12), and figures whose unit_ref matches target_unit_debar.
// FUNCTION: C2 0x52be9
// FUNCTION: C2WIN 0x00482654
int find_nearest_target(int max_distance)
{
    int best_distance = 0x68;
    int best_figure_idx = 0;
    int distance;

    for (temp_figure = 1; temp_figure < 0xc9; temp_figure++) {
        if (figure_list[temp_figure].exists == 0) continue;
        if (figure_list[temp_figure].owner == figure_list[figure_no].owner) continue;
        if (figure_list[temp_figure].state_idx == 2) continue;
        if (figure_list[temp_figure].state_idx == 0xc) continue;
        if (figure_list[temp_figure].unit_ref == target_unit_debar) continue;
        distance = get_distance(figure_list[figure_no].grid_x,
                                figure_list[figure_no].grid_y,
                                figure_list[temp_figure].grid_x,
                                figure_list[temp_figure].grid_y);
        if (distance <= max_distance && distance < best_distance) {
            best_distance = distance;
            best_figure_idx = temp_figure;
        }
    }
    if (best_figure_idx == 0) return 0;
    enemy_figure = best_figure_idx;
    return 1;
}

// Empty adjacent-target hook; always returns 0.
// FUNCTION: C2 0x52cc0
int find_adjacent_target(void)
{
    return 0;
}

// Scan an 11x11 box of battle_map cells around the firing unit for a hostile figure to shoot at.
// Anchor is the unit's grid; the scan span is clamped to [0, 0x33].
// FUNCTION: C2 0x52cc3
// FUNCTION: C2WIN 0x00482837
int get_fire_target(int figure_idx)
{

    int marker_x;
    int marker_y;
    int start_x;
    int end_x;
    int start_y;
    int x;
    int end_y;
    int y;
    int cell_offset;
    int fallback_offset;
    int row_skip;
    int fallback_figure_idx;
    int previous_offset;
    int target_idx;

    fallback_figure_idx = 0;

    temp_unit = figure_list[figure_idx].unit_ref;
    marker_x = unit_list[temp_unit].attack_marker_x;
    marker_y = unit_list[temp_unit].attack_marker_y;
    previous_offset = unit_list[temp_unit].prev_attack_off;

    start_x = marker_x; start_y = marker_y;
    end_x = marker_x + 0xb;
    end_y = marker_y + 0xb;
    if (start_x < 0) start_x = 0;
    if (end_x  >= 0x34) end_x = 0x33;
    if (start_y < 0) start_y = 0;
    if (end_y  >= 0x34) end_y = 0x33;

    cell_offset = (start_y * 0x34 + start_x) * 4;
    row_skip = (0x34 - (end_x - start_x + 1)) * 4;

    for (y = start_y; y <= end_y; y++, cell_offset += row_skip) {
        x = start_x;
        for (; x <= end_x; x++, cell_offset += 4) {
            enemy_figure = ((unsigned char *)battle_map)[cell_offset + 1];
            if (enemy_figure != 0 && figure_list[(target_idx = enemy_figure)].exists != 0) {
                if (figure_list[target_idx].owner != figure_list[figure_idx].owner) {
                if (cell_offset > previous_offset) { unit_list[temp_unit].prev_attack_off = cell_offset; return 1; }
                if (fallback_figure_idx == 0) { fallback_figure_idx = target_idx; fallback_offset = cell_offset; }
                }
            }
        } }

    if (fallback_figure_idx == 0) return 0;
    unit_list[temp_unit].prev_attack_off = fallback_offset;
    enemy_figure = fallback_figure_idx;
    return 1;
}

struct attack_pos_rec attack_pos_data[20] = {
    { 3, 0, 0 },
    { 3, 1, 0 },
    { 4, 0, 1 },
    { 4, 1, 1 },
    { 3, 0, 0 },
    { 3, 1, 0 },
    { 4, 0, 1 },
    { 4, 1, 1 },
    { 3, 0, 2 },
    { 3, 1, 2 },
    { 4, 0, 2 },
    { 4, 1, 2 },
    { 3, 0, 0 },
    { 3, 1, 0 },
    { 4, 0, 1 },
    { 4, 1, 1 },
    { 3, 0, 2 },
    { 3, 1, 2 },
    { 4, 0, 0 },
    { 4, 1, 0 }
};

int steves_security_false2[7] = { 538976288, 2021138464, 2021161080, 538998904, 2021138464, 538998904, 538976288 };

struct byte_delta_rec elephant_stampede[8] = {
    { 53, 53 },
    { 26, 53 },
    { -1, 53 },
    { 53, 53 },
    { 26, 53 },
    { -1, 53 },
    { 26, 53 },
    { 26, 53 }
};

unsigned char sling_images[33] = { 10, 10, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 10, 10, 10, 11, 11, 12, 12, 11, 12, 11, 12 };

unsigned char bow_images[33] = { 10, 10, 10, 10, 10, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 11, 11, 11, 12, 12, 12, 12, 12 };

unsigned char horsebow_images[33] = { 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 7, 7, 8, 8, 8, 8, 8 };

char elephant_archer_images[21] = "111111111122223333000";

void (*figure_intelligences[18])(void) = {
    f00_null,
    f01_regular,
    f02_irregular,
    f03_auxillary,
    f00_null,
    f05_barb_sword,
    f06_barb_spear,
    f07_barb_axe,
    f08_barb_pike,
    f03_auxillary,
    f03_auxillary,
    f11_barb_horse_heavy,
    f12_barb_horse_light,
    f12_barb_horse_light,
    f12_barb_horse_light,
    f15_barb_elephant,
    f16_barb_bow,
    f03_auxillary
};

void (*figure_states[17])(void) = {
    sf00_null,
    sf01_wait,
    sf02_death,
    sf03_move,
    sf04_fight,
    sf05_mop_up,
    sf06_defend,
    sf07_reform,
    sf08_withdraw,
    sf09_look_for_fight,
    sf10_hunt_for_fight,
    sf11_fire_missile,
    sf12_rout,
    sf13_autofire_missile,
    sf14_opertunist_fire,
    sf15_move_and_reform,
    sf16_beserk
};

struct int_delta_rec line_flank_data[20] = {
    { 0, 0 },
    { 0, 1 },
    { 1, 0 },
    { 1, 1 },
    { 2, 0 },
    { 2, 1 },
    { 3, 0 },
    { 3, 1 },
    { 4, 0 },
    { 4, 1 },
    { 5, 0 },
    { 5, 1 },
    { 6, 0 },
    { 6, 1 },
    { 7, 0 },
    { 7, 1 },
    { 8, 0 },
    { 8, 1 },
    { 9, 0 },
    { 9, 1 }
};

struct int_delta_rec col_flank_data[20] = {
    { 0, 0 },
    { 1, 0 },
    { 0, 1 },
    { 1, 1 },
    { 0, 2 },
    { 1, 2 },
    { 0, 3 },
    { 1, 3 },
    { 0, 4 },
    { 1, 4 },
    { 0, 5 },
    { 1, 5 },
    { 0, 6 },
    { 1, 6 },
    { 0, 7 },
    { 1, 7 },
    { 0, 8 },
    { 1, 8 },
    { 0, 9 },
    { 1, 9 }
};

signed char wf_battle_dircs[8] = { 1, -1, 2, -2, 3, -3, 4, 4 };
