
#include "c2_data.h"
#include "c2_types.h"

/* File-local state. */
int age_count;

extern int get_heading();


int city_test_for_road(int cell_x, int cell_y, int cell_offset, signed char heading);


extern void fill_warehouses_with(int cell_x, int cell_y, int amount,
                                 int goods_kind, int refresh_flag);
/* Forward declarations (functions defined later in this file). */
void get_movement_image(int image_base);
void get_army_ship_image(int image_base);
void get_rioter_image(int image_base);
void get_barbarian_image(void);
void get_enemy_image(void);
void get_cohort_image(void);
void get_dirc_from_citizen_wf_run(void);
void copy_ferret_run_to_citizen(void);
void handle_collision(int other_idx);
void fight_centurian(int other_idx);
void fight_barbarian(int other_idx);
void move_citizen(void);
void change_citizen_targs(int target_delta);
void random_target(void);
void target_from_dirc(int direction);
void get_dirc_from_army_wf_run(void);
void copy_ferret_run_to_army(void);
void move_army(void);
void get_population_and_industry_count(int radius, int demand_mode);

// Per-tick AI dispatch for all 201 citizen slots.
// FUNCTION: C2 0x45eb5
// FUNCTION: C2WIN 0x004040b0
void citizen_intelligence(void)
{
    age_count++;
    if (age_count >= 0x40) age_count = 0;
    no_of_rioters    = (no_of_rioters    > 1);
    no_of_barbarians = (no_of_barbarians > 1);
    no_of_citizens = 0;
    for (citizen_no = 0; citizen_no < 0xc9; citizen_no++) {
        if (citizen_list[citizen_no].exists != 0) {
            no_of_citizens++;
            if (citizen_list[citizen_no].type <= 0
             || citizen_list[citizen_no].type >= 8)
                remove_citizen(citizen_no);
            else
                citizen_intelligences[citizen_list[citizen_no].type]();
        }
    }
}

// Per-tick AI dispatch for all 26 army slots.
// FUNCTION: C2 0x45f5f
// FUNCTION: C2WIN 0x004041f9
void army_intelligence(void)
{
    no_of_armies = 0;
    if (c2inf.peace_mode != 0)
        return;
    for (army_no = 0; army_no < 26; army_no++) {
        if (army_list[army_no].exists != 0) {
            no_of_armies++;
            army_list[army_no].map_x = army_list[army_no].map_y = 0;
            if (army_list[army_no].type <= 0
             || army_list[army_no].type >= 9)
                remove_army(army_no);
            else
                army_intelligences[army_list[army_no].type]();
        }
    }
}

// Empty citizen-intelligence slot for the null type.
// FUNCTION: C2 0x45fe2
// FUNCTION: C2WIN 0x00404338
void i00_null(void)
{
}

// Dispatch the tax collector's state, update its sprite, and expire it after 18 age cycles.
// FUNCTION: C2 0x45fe3
// FUNCTION: C2WIN 0x00404343
void i01_tax_man(void)
{
    citizen_states[citizen_list[citizen_no].state_idx]();
    get_movement_image(0x36);
    if (!age_count)
        if (++citizen_list[citizen_no].state_timer >= 0x12)
            citizen_list[citizen_no].state_idx = 2;
}

// Dispatch the market trader's state, update its sprite, and expire it after 30 age cycles.
// FUNCTION: C2 0x46038
// FUNCTION: C2WIN 0x004043d8
void i02_market_man(void)
{
    citizen_states[citizen_list[citizen_no].state_idx]();
    get_movement_image(0x1B);
    if (!age_count)
        if (++citizen_list[citizen_no].state_timer >= 0x1E)
            citizen_list[citizen_no].state_idx = 2;
}

// Dispatch a barbarian's state, update its sprite, flag the emergency, and age it toward expiry.
// FUNCTION: C2 0x4608d
// FUNCTION: C2WIN 0x0040446d
void i03_barbarian_man(void)
{
    emergency_mood    = 2;
    no_of_barbarians  = 2;
    citizen_states[citizen_list[citizen_no].state_idx]();
    get_movement_image(0xA6);
    if (!age_count)
        if (++citizen_list[citizen_no].state_timer >= 0x48)
            citizen_list[citizen_no].state_idx = 2;
}

// Dispatch the centurion's state, update its sprite, and expire it after 35 age cycles.
// FUNCTION: C2 0x460f3
// FUNCTION: C2WIN 0x00404519
void i04_centurian_man(void)
{
    citizen_states[citizen_list[citizen_no].state_idx]();
    get_movement_image(0x6E);
    if (!age_count)
        if (++citizen_list[citizen_no].state_timer >= 0x23)
            citizen_list[citizen_no].state_idx = 2;
}

// Dispatch the vigile's state, update its sprite, and expire it after 20 age cycles.
// FUNCTION: C2 0x46148
// FUNCTION: C2WIN 0x004045ae
void i05_vigile_man(void)
{
    citizen_states[citizen_list[citizen_no].state_idx]();
    get_movement_image(0x51);
    if (!age_count)
        if (++citizen_list[citizen_no].state_timer >= 0x14)
            citizen_list[citizen_no].state_idx = 2;
}

// Dispatch the businessman's state, update its sprite, and expire it after 30 age cycles.
// FUNCTION: C2 0x4619d
// FUNCTION: C2WIN 0x00404643
void i06_business_man(void)
{
    citizen_states[citizen_list[citizen_no].state_idx]();
    get_movement_image(0);
    if (!age_count)
        if (++citizen_list[citizen_no].state_timer >= 0x1E)
            citizen_list[citizen_no].state_idx = 2;
}

// Dispatch a rioter's state, flag the emergency, select its normal or mob sprite, and age it.
// FUNCTION: C2 0x461ef
// FUNCTION: C2WIN 0x004046d8
void i07_rioter_man(void)
{
    emergency_mood = 2;
    no_of_rioters  = 2;
    citizen_states[citizen_list[citizen_no].state_idx]();
    if (citizen_list[citizen_no].state_idx == 12) get_movement_image(0);
    else                                          get_rioter_image(0x89);
    if (!age_count)
        if (++citizen_list[citizen_no].state_timer >= 0x14)
            citizen_list[citizen_no].state_idx = 2;
}

// Empty army-intelligence slot for the null type.
// FUNCTION: C2 0x46273
// FUNCTION: C2WIN 0x004047b5
void a00_null(void)
{
}

// Cohort-army intelligence tick: refresh sprite, dispatch the per-state handler, and tick the
// morale / target timers.
// FUNCTION: C2 0x46274
// FUNCTION: C2WIN 0x004047c0
void a01_cohort(void)
{
    get_cohort_image();
    army_states[army_list[army_no].state_idx]();
    if ((*(struct region_cell *)((unsigned char *)region_map + (army_list[army_no].map_ref))).terrain & 4) {
        army_list[army_no].morale_timer = 0x30;
        army_list[army_no].target_timer = 0x320;
    } else if (army_list[army_no].morale_timer) {
        if (army_list[army_no].target_flag) army_list[army_no].morale_timer -= 2;
        else                                army_list[army_no].morale_timer -= 1;
    }
    if (army_list[army_no].target_timer) army_list[army_no].target_timer--;
}

// Enemy-army intelligence tick: flag a threat mood, refresh the sprite frame, and dispatch the
// per-state handler.
// FUNCTION: C2 0x4631e
// FUNCTION: C2WIN 0x0040493c
void a02_enemy(void)
{
    threat_mood = 2;
    get_enemy_image();
    army_states[army_list[army_no].state_idx]();
}

// Raise the threat mood, update the raider sprite, and dispatch its army-state handler.
// FUNCTION: C2 0x46349
// FUNCTION: C2WIN 0x004049b0
void a04_raider(void)
{
    threat_mood = 2;
    get_barbarian_image();
    army_states[army_list[army_no].state_idx]();
}

// Revolt-army intelligence tick: raises threat_mood. Uses the hostile-empire sprite
// (`get_enemy_image`) in the four named provinces (6, 0xF, 0x12, 0x22) and the generic barbarian
// sprite everywhere else, then dispatches the per-state handler.
// FUNCTION: C2 0x4635a
// FUNCTION: C2WIN 0x004049ea
void a05_revolt(void)
{
    threat_mood = 2;
    if (province_is == 6 || province_is == 0xF || province_is == 0x12 || province_is == 0x22)
        get_enemy_image();
    else
        get_barbarian_image();
    army_states[army_list[army_no].state_idx]();
}

// Roman-trader-ship intelligence tick: ship sprite base 0x4E, then dispatch the per-state handler.
// FUNCTION: C2 0x463a8
// FUNCTION: C2WIN 0x00404a62
void a06_roman_ship(void)
{
    get_army_ship_image(0x4E);
    army_states[army_list[army_no].state_idx]();
}

// Raise the threat mood for an active raider ship.
// FUNCTION: C2 0x463b7
// FUNCTION: C2WIN 0x00404aac
void a08_raider_ship(void)
{
    threat_mood = 2;
}

// Empty citizen-state slot for the null state.
// FUNCTION: C2 0x463c1
// FUNCTION: C2WIN 0x00404ac1
void s00_null(void)
{
}

// Citizen state-1 (wait): tick down wait_count, and on expiry reset the per-citizen speed phase,
// clear action_kind, restore the saved state_idx, and arm speed = 5.
// FUNCTION: C2 0x463c2
// FUNCTION: C2WIN 0x00404acc
void s01_wait(void)
{
    if (--citizen_list[citizen_no].wait_count > 0) {
    } else {
        citizen_list[citizen_no].speed_phase = 0;
        citizen_list[citizen_no].speed_count = 0;
        citizen_list[citizen_no].action_kind = 0;
        citizen_list[citizen_no].state_idx   = citizen_list[citizen_no].saved_state_idx;
        citizen_list[citizen_no].flag_bits  |= 1;
        citizen_list[citizen_no].speed       = 5;
    }
}

// Citizen state-2 (death): remove the citizen from the world.
// FUNCTION: C2 0x46411
// FUNCTION: C2WIN 0x00404bdc
void s02_death(void)
{
    remove_citizen(citizen_no);
}

// Move an administrator along roads while marking the surrounding administration coverage.
// FUNCTION: C2 0x4641d
// FUNCTION: C2WIN 0x00404bf7
void s03_map_admin(void)
{
    unsigned char road_dir;

    if (citizen_go_to_target(0) == 0) {
    } else if (citizen_list[citizen_no].flag_bits & 1) {
        flag_range(0, citizen_list[citizen_no].x, citizen_list[citizen_no].y,
                   3, 0xa, 0xc);
        road_dir = (unsigned char)city_test_for_road(
                citizen_list[citizen_no].x,
                citizen_list[citizen_no].y,
                citizen_list[citizen_no].map_ref,
                citizen_list[citizen_no].world_dir);
        if (road_dir >= 8) {
            citizen_list[citizen_no].state_idx = 2;
            return;
        }
        target_from_dirc(road_dir);
        citizen_list[citizen_no].action_kind = 1;
    }
}

// Citizen state-4 "map markets" tick -- market traders stamp per-cell demand bits into the
// destination cell's `building` byte.
// FUNCTION: C2 0x464ce
// FUNCTION: C2WIN 0x00404d45
void s04_map_markets(void)
{
    int cell_offset;
    unsigned char road_dir;

    if (citizen_go_to_target(0) == 0) {
    } else if (citizen_list[citizen_no].flag_bits & 1) {
        flag_range(0,
                   citizen_list[citizen_no].x,
                   citizen_list[citizen_no].y,
                   3, 0xa, 0xc0);
        get_population_and_industry_count(1, 1);

        cell_offset = citizen_list[citizen_no].target_ref;
        if (((unsigned char *)city_map)[cell_offset] >= 0xfc &&
            ((unsigned char *)city_map)[cell_offset] <= 0xff) {
            if (citizen_list[citizen_no].market_demand_a > 0) {
                ((unsigned char *)city_map)[cell_offset + 9] &= 0xfc;
                if (citizen_list[citizen_no].market_demand_a < 8)
                    ((unsigned char *)city_map)[cell_offset + 9] |= 2;
                else
                    ((unsigned char *)city_map)[cell_offset + 9] |= 3;
            }
            if (citizen_list[citizen_no].market_demand_b > 0) {
                ((unsigned char *)city_map)[cell_offset + 9] &= 0xf3;
                if (citizen_list[citizen_no].market_demand_b < 8)
                    ((unsigned char *)city_map)[cell_offset + 9] |= 8;
                else
                    ((unsigned char *)city_map)[cell_offset + 9] |= 0xc;
            }
        }

        road_dir = (unsigned char)city_test_for_road(citizen_list[citizen_no].x,
                                              citizen_list[citizen_no].y,
                                              citizen_list[citizen_no].map_ref,
                                              citizen_list[citizen_no].world_dir);
        if (road_dir >= 8) {
            citizen_list[citizen_no].state_idx = 2;
            return;
        }
        target_from_dirc(road_dir);
        citizen_list[citizen_no].action_kind = 1;
    }
}

// Citizen state-5 handler: head toward the city's "top spot" (visit attraction). If the path-find
// can't reach a target, fall back to a random nearby tile.
// FUNCTION: C2 0x4663a
// FUNCTION: C2WIN 0x00405005
void s05_maraude_to_top_spot(void)
{
    citizen_list[citizen_no].action_kind = 1;
    citizen_list[citizen_no].saved_state_idx = 5;
    if (citizen_maraude_to_target(1) == 0) {
        random_target();
        citizen_list[citizen_no].wait_count = 0;
        citizen_list[citizen_no].state = 3;
    } else if (citizen_list[citizen_no].flag_bits & 1) {
        if (citizen_list[citizen_no].state != 0) {
            citizen_list[citizen_no].state--;
        } else {
            citizen_list[citizen_no].state = 3;
            citizen_list[citizen_no].dest_x = top_lv_x;
            citizen_list[citizen_no].dest_y = top_lv_y;
        }
    }
}

// Advance a trouble-quelling citizen toward its rioter target and reject recycled target slots.
// FUNCTION: C2 0x466da
// FUNCTION: C2WIN 0x00405143
void s06_quell_trouble(void)
{
    citizen_list[citizen_no].action_kind = 1;
    if (citizen_maraude_to_target(2) == 0) {
    } else if (citizen_list[citizen_no].flag_bits & 1) {
        citizen_a = citizen_list[citizen_no].target_kind;
        if (citizen_list[citizen_a].exists != 0 &&
            citizen_list[citizen_no].target_marker ==
                citizen_list[citizen_a].evolve_timer) {
            citizen_list[citizen_no].dest_x =
                citizen_list[citizen_a].x;
            citizen_list[citizen_no].dest_y =
                citizen_list[citizen_a].y;
            citizen_list[citizen_no].target_count++;
            if (citizen_list[citizen_no].target_count > 4) {
                citizen_list[citizen_no].target_count = 0;
                citizen_list[citizen_no].wf_active = 0;
            }
        } else {
            citizen_a = find_enemy(citizen_list[citizen_no].x,
                                   citizen_list[citizen_no].y, 10);
            if (citizen_a == 0) {
                citizen_list[citizen_no].state_idx = 2;
            } else {
                citizen_list[citizen_no].target_kind = citizen_a;
                citizen_list[citizen_no].target_marker =
                    citizen_list[citizen_a].evolve_timer;
                citizen_list[citizen_no].dest_x =
                    citizen_list[citizen_a].x;
                citizen_list[citizen_no].dest_y =
                    citizen_list[citizen_a].y;
                citizen_list[citizen_no].target_count = 0;
                citizen_list[citizen_no].wf_active = 0;
            }
        }
    }
}

// Advance a vigile patrol, mark its coverage area, and acquire nearby rioters or barbarians.
// FUNCTION: C2 0x46830
// FUNCTION: C2WIN 0x0040543e
void s07_army_patrol(void)
{
    unsigned char road_dir;

    if (citizen_go_to_target(0) == 0) {
    } else if (citizen_list[citizen_no].flag_bits & 1) {
        flag_range(0, citizen_list[citizen_no].x,
                   citizen_list[citizen_no].y, 4, 0xa, 0x30);
        if (no_of_rioters != 0 || no_of_barbarians != 0) {
            citizen_a = find_enemy(citizen_list[citizen_no].x,
                                   citizen_list[citizen_no].y, 10);
            if (citizen_a != 0) {
                citizen_list[citizen_no].target_kind = citizen_a;
                citizen_list[citizen_no].target_marker =
                    citizen_list[citizen_a].evolve_timer;
                citizen_list[citizen_no].dest_x =
                    citizen_list[citizen_a].x;
                citizen_list[citizen_no].dest_y =
                    citizen_list[citizen_a].y;
                citizen_list[citizen_no].target_count = 0;
                citizen_list[citizen_no].wf_active = 0;
                citizen_list[citizen_no].state_idx = 6;
                return;
            }
        }
        road_dir = (unsigned char)city_test_for_road(
                    citizen_list[citizen_no].x,
                    citizen_list[citizen_no].y,
                    citizen_list[citizen_no].map_ref,
                    citizen_list[citizen_no].world_dir);
        if (road_dir >= 8) {
            citizen_list[citizen_no].state_idx = 2;
            citizen_list[citizen_no].wait_count = 0x28;
            return;
        }
        target_from_dirc(road_dir);
        citizen_list[citizen_no].action_kind = 1;
    }
}

// Patrol and mark vigile coverage, diverting to nearby fires, rioters, or barbarians.
// FUNCTION: C2 0x46988
// FUNCTION: C2WIN 0x00405711
void s08_vigile_patrol(void)
{
    unsigned char road_dir;

    if (citizen_go_to_target(0) == 0) {
    } else if (citizen_list[citizen_no].flag_bits & 1) {
        flag_range(0, citizen_list[citizen_no].x,
                   citizen_list[citizen_no].y, 3, 0xa, 0x30);
        if (test_fire_zones() != 0) {
            citizen_list[citizen_no].state_idx = 9;
            return;
        }
        if (no_of_rioters != 0 || no_of_barbarians != 0) {
            citizen_a = find_enemy(citizen_list[citizen_no].x,
                                   citizen_list[citizen_no].y, 10);
            if (citizen_a != 0) {
                citizen_list[citizen_no].target_kind = citizen_a;
                citizen_list[citizen_no].target_marker =
                    citizen_list[citizen_a].evolve_timer;
                citizen_list[citizen_no].dest_x =
                    citizen_list[citizen_a].x;
                citizen_list[citizen_no].dest_y =
                    citizen_list[citizen_a].y;
                citizen_list[citizen_no].target_count = 0;
                citizen_list[citizen_no].wf_active = 0;
                citizen_list[citizen_no].state_idx = 6;
                return;
            }
        }
        road_dir = (unsigned char)city_test_for_road(
                    citizen_list[citizen_no].x,
                    citizen_list[citizen_no].y,
                    citizen_list[citizen_no].map_ref,
                    citizen_list[citizen_no].world_dir);
        if (road_dir >= 8) {
            citizen_list[citizen_no].state_idx = 2;
            citizen_list[citizen_no].wait_count = 0x28;
            return;
        }
        target_from_dirc(road_dir);
        citizen_list[citizen_no].action_kind = 1;
    }
}

// Advance a firefighter toward its target, extinguish valid fires, or acquire a replacement target.
// FUNCTION: C2 0x46afe
// FUNCTION: C2WIN 0x00405a0f
void s09_fire_fight(void)
{
    citizen_list[citizen_no].saved_state_idx = 9;
    if (citizen_maraude_to_target(2) == 0) {
    } else {
        if (citizen_list[citizen_no].flag_bits & 2) {
            citizen_list[citizen_no].state_idx = 9;
            citizen_list[citizen_no].target_kind = 0;
            citizen_list[citizen_no].flag_bits &= 0xfd;
        }
        if (citizen_list[citizen_no].flag_bits & 1) {
            if (putting_out_fire() != 0) {
                citizen_list[citizen_no].action_kind = 0;
                citizen_list[citizen_no].target_kind = 0;
            } else {
                if (confirm_fire_target() != 0) {
                    citizen_list[citizen_no].action_kind = 1;
                } else {
                    citizen_list[citizen_no].target_kind = 0;
                    citizen_list[citizen_no].action_kind = 0;
                    citizen_list[citizen_no].target_ref = 0;
                    if (test_fire_zones() != 0) {
                        if (test_zone_for_closest_fire() != 0) {
                            citizen_list[citizen_no].dest_x = z_x;
                            citizen_list[citizen_no].dest_y = z_y;
                            citizen_list[citizen_no].target_ref = z_ptr;
                            citizen_list[citizen_no].wf_active = 0;
                            citizen_list[citizen_no].target_kind = 1;
                            citizen_list[citizen_no].action_kind = 1;
                        }
                    } else {
                        citizen_list[citizen_no].state_idx = 2;
                    }
                }
            }
        }
    }
}

// Update local market demand at a business destination, then choose the trader's next road tile.
// FUNCTION: C2 0x46c41
// FUNCTION: C2WIN 0x00405c88
void s10_get_business(void)
{
    int cell_offset;
    unsigned char road_dir;

    if (citizen_go_to_target(0) == 0) {
    } else if (citizen_list[citizen_no].flag_bits & 1) {
        get_population_and_industry_count(1, 0);
        cell_offset = citizen_list[citizen_no].target_ref;
        if (((unsigned char *)city_map)[cell_offset] == 0xfa) {
            if (citizen_list[citizen_no].market_demand_a > 0) {
                ((unsigned char *)city_map)[cell_offset + 9] &= 0xfc;
                if (citizen_list[citizen_no].market_demand_a < 8)
                    ((unsigned char *)city_map)[cell_offset + 9] |= 2;
                else
                    ((unsigned char *)city_map)[cell_offset + 9] |= 3;
            }
            if (citizen_list[citizen_no].market_demand_b > 0) {
                ((unsigned char *)city_map)[cell_offset + 9] &= 0xf3;
                if (citizen_list[citizen_no].market_demand_b < 8)
                    ((unsigned char *)city_map)[cell_offset + 9] |= 8;
                else
                    ((unsigned char *)city_map)[cell_offset + 9] |= 0xc;
            }
        }
        road_dir = (unsigned char)city_test_for_road(citizen_list[citizen_no].x, citizen_list[citizen_no].y, citizen_list[citizen_no].map_ref, citizen_list[citizen_no].world_dir);
        if (road_dir >= 8) {
            citizen_list[citizen_no].state_idx = 2;
            citizen_list[citizen_no].wait_count = 0x28;
            return;
        }
        target_from_dirc(road_dir);
        citizen_list[citizen_no].action_kind = 1;
    }
}

// Citizen state-11 (rioting in place): tick down wait_count; on expiry transition to state 12
// (goto_riot), aim the citizen at the player's last-viewed tile and reset its walk speed to 5.
// FUNCTION: C2 0x46d86
// FUNCTION: C2WIN 0x00405f03
void s11_riot(void)
{
    citizen_list[citizen_no].action_kind = 0;
    if (--citizen_list[citizen_no].wait_count > 0) {
    } else {
        citizen_list[citizen_no].state_idx   = 12;
        citizen_list[citizen_no].dest_x      = top_lv_x;
        citizen_list[citizen_no].dest_y      = top_lv_y;
        citizen_list[citizen_no].speed_phase = 0;
        citizen_list[citizen_no].speed_count = 0;
        citizen_list[citizen_no].speed       = 5;
    }
}

// Citizen state-12 (walking to riot location): step toward the queued destination via
// citizen_maraude_to_target. On arrival re-aim at the player's last-viewed tile and transition to
// state 11 (riot in place) with a 30-tick rest.
// FUNCTION: C2 0x46de3
// FUNCTION: C2WIN 0x00406003
void s12_goto_riot(void)
{
    citizen_list[citizen_no].action_kind = 1;
    if (citizen_maraude_to_target(1) == 0) {
    } else if (citizen_list[citizen_no].flag_bits & 1) {
        citizen_list[citizen_no].dest_x     = top_lv_x;
        citizen_list[citizen_no].dest_y     = top_lv_y;
        citizen_list[citizen_no].state_idx  = 11;
        citizen_list[citizen_no].wait_count = 30;
    }
}

// Empty army-state slot for the null state.
// FUNCTION: C2 0x46e44
// FUNCTION: C2WIN 0x004060cb
void sa00_null(void)
{
}

// Army state-1 (wait): unless the global pause counter `cnt4` is still ticking, count down
// wait_count and on expiry restore the saved state, clear targeting state, reset heading to 5, and
// run test_for_army_attack to see if the next tick lands on an enemy.
// FUNCTION: C2 0x46e45
// FUNCTION: C2WIN 0x004060d6
void sa01_wait(void)
{
    if (cnt4) return;
    if (--army_list[army_no].wait_count > 0) {
    } else {
        army_list[army_no].wait_count   = 5;
        army_list[army_no].target_count      = 0;
        army_list[army_no].target_kind   = 0;
        army_list[army_no].return_flag  = 0;
        army_list[army_no].state_idx    = army_list[army_no].saved_state_idx;
#if PLATFORM_WINDOWS
        army_list[army_no].flags       &= 0xFD;
#else
        army_list[army_no].flags       &= 0xFC;
#endif
        army_list[army_no].flags       |= 1;
        army_list[army_no].heading      = 5;
        test_for_army_attack();
    }
}

// Army state-2 (death): remove the army from the world.
// FUNCTION: C2 0x46eb9
// FUNCTION: C2WIN 0x00406249
void sa02_death(void)
{
    remove_army(army_no);
}

// Advance a cohort along its patrol route, engaging enemies and cycling through route rows.
// FUNCTION: C2 0x46ec5
// FUNCTION: C2WIN 0x00406264
void sa03_army_move(void)
{
    int i;

    if (region_go_to_target(0) == 0) {
    } else if (army_list[army_no].flags & 1) {
        army_list[army_no].return_flag = 1;
        if (test_for_army_attack() != 0) {
        } else {
            if ((unsigned char)army_list[army_no].dest_x
                    < army_routes[army_list[army_no].cohort_id]
                          .row_len[army_list[army_no].dest_y]) {
                new_army_route_point();
            } else if (army_list[army_no].dest_y
                    < army_routes[army_list[army_no].cohort_id].row_count - 1) {
                army_list[army_no].dest_y += 1;
                army_list[army_no].dest_x = 0;
                new_army_route_point();
                new_army_route_point();
            } else if (army_routes[army_list[army_no].cohort_id].row_count <= 1) {
                army_list[army_no].state_idx = 1;
                army_list[army_no].saved_state_idx = 1;
                army_list[army_no].wait_count = 5;
                army_routes[army_list[army_no].cohort_id].chase_row = 0;
                army_routes[army_list[army_no].cohort_id].target_army = 0;
            } else {
                army_list[army_no].dest_y = 1;
                army_list[army_no].dest_x = 0;
                new_army_route_point();
            }
        }
    }
}

// Pull the next patrol-route waypoint for the current army. First tries to engage a queued enemy
// via `goto_army_attack` (returns 1 if it commits).
// FUNCTION: C2 0x46fb4
// FUNCTION: C2WIN 0x00406517
int new_army_route_point(void)
{
    int ptr;
    int y;
    unsigned char flags;
    unsigned char map_value;
    int x;

    if (goto_army_attack() != 0) return 1;

    x = army_routes[army_list[army_no].cohort_id]
               .points[army_list[army_no].dest_y][army_list[army_no].dest_x].x;
    y = army_routes[army_list[army_no].cohort_id]
               .points[army_list[army_no].dest_y][army_list[army_no].dest_x].y;
    ptr = (x + y * REGION_W) * REGION_CELL_BYTES;
    flags = (*(struct region_cell *)((unsigned char *)region_map + (ptr))).terrain;
    army_list[army_no].dest_x += 1;

    if ((flags & 1) != 0) {
        map_value = (*(struct region_cell *)((unsigned char *)region_map + (ptr))).base_kind;
        if (map_value < 0x93 || map_value > 0x96) return 0;
    } else {
        if ((flags & 0x12) != 0) return 0;
    }

    army_list[army_no].target_x = x;
    army_list[army_no].target_y = y;
    if (army_list[army_no].target_x == army_list[army_no].x) {
        if (army_list[army_no].target_y == army_list[army_no].y) return 0;
    }
    return 1;
}

// Try to switch the current army into the "attack" state, locking onto the queued enemy army from
// its route slot.
// FUNCTION: C2 0x470a2
// FUNCTION: C2WIN 0x0040678c
int goto_army_attack(void)
{
    if (army_routes[army_list[army_no].cohort_id].target_army == 0)
        return 0;
    if (army_list[army_no].dest_y < army_routes[army_list[army_no].cohort_id].chase_row)
        return 0;
    enemy_army = army_routes[army_list[army_no].cohort_id].target_army;
    army_list[army_no].army_id = enemy_army;
    army_list[army_no].target_marker =
        army_list[enemy_army].evolve_timer;
    army_list[army_no].state_idx = 4;
    return 1;
}

// Decide whether the current cohort army should commit to an attack this tick. Returns 1 if it
// engaged (state flipped to 4 = sa04_army_attack), else 0.
// FUNCTION: C2 0x47120
// FUNCTION: C2WIN 0x004068ce
int test_for_army_attack(void)
{
    int distance;

    if (army_list[army_no].type != 1) return 0;
    if (army_list[army_no].order_progress != 0) return 0;
    if (army_list[army_no].total_troops <= 0) return 0;

    if (army_routes[army_list[army_no].cohort_id].target_army != 0) {
        enemy_army = (short)army_routes[army_list[army_no].cohort_id].target_army;
        distance = get_longest_distance(army_list[army_no].x,
                                 army_list[army_no].y,
                                 army_list[enemy_army].x,
                                 army_list[enemy_army].y);
        if (distance > 6) return 0;
        army_list[army_no].army_id = enemy_army;
        army_list[army_no].target_marker  = army_list[enemy_army].evolve_timer;
        army_list[army_no].saved_state_idx = army_list[army_no].state_idx;
        army_list[army_no].state_idx       = 4;
        return 1;
    }
    if (army_list[army_no].readiness_level == 0) return 0;
    enemy_army = find_invading_army(army_list[army_no].x, army_list[army_no].y, 8);
    if (enemy_army != 0) {
        army_list[army_no].army_id = enemy_army;
        army_list[army_no].target_marker  = army_list[enemy_army].evolve_timer;
        army_list[army_no].saved_state_idx = army_list[army_no].state_idx;
        army_list[army_no].state_idx       = 4;
        return 1;
    }
    return 0;
}

// Region-map army state-4: attack the locked-on enemy. Aborts to state 1 (wait 5 ticks) if the
// enemy has vanished (exists == 0) or its evolve_timer no longer matches the cached
// `target_marker` from `goto_army_attack`.
// FUNCTION: C2 0x47257
// FUNCTION: C2WIN 0x00406c08
void sa04_army_attack(void)
{
    if (army_list[army_no].wf_step >= 2) army_list[army_no].wf_active = 0;
    enemy_army = army_list[army_no].army_id;
    if (army_list[army_list[army_no].army_id].exists == 0
     || army_list[enemy_army].evolve_timer
            != army_list[army_no].target_marker) {
        army_list[army_no].state_idx = 1;
        army_list[army_no].wait_count = 5;
        return;
    }
    army_list[army_no].target_x = army_list[enemy_army].x;
    army_list[army_no].target_y = army_list[enemy_army].y;
    if (region_go_to_target(0) != 0
     && (army_list[army_no].flags & 1) != 0)
        army_list[army_no].return_flag = 1;
}

// Return an army to its waypoint, then enter the wait state and clear its pending order.
// FUNCTION: C2 0x47332
// FUNCTION: C2WIN 0x00406db1
void sa05_army_return(void)
{
    int i;

    army_list[army_no].return_flag = 1;
    if (region_go_to_target(0) == 0) {
    } else if (army_list[army_no].flags & 2) {
        army_list[army_no].state_idx      = 1;
        army_list[army_no].order_progress = 0;
    }
}

// Region-map army state-6: the raid handler.
// FUNCTION: C2 0x47382
// FUNCTION: C2WIN 0x00406e42
void sa06_army_land_raid(void)
{
    if ((army_list[army_no].target_x == 0 && army_list[army_no].target_y == 0)
     || (army_list[army_no].flags & 2) != 0) {
        if (get_nearest_reg_building() != 0) {
            army_list[army_no].target_x = gmn_x;
            army_list[army_no].target_y = gmn_y;
        } else {
            army_list[army_no].target_x = reg_city_x;
            army_list[army_no].target_y = reg_city_y;
        }
    }
    army_list[army_no].return_flag = 1;
    if (region_go_to_target(0) == 0) return;
}

// Region-map army state-7: barbarian/raider land-invasion handler. Always targets the player's
// city (reg_city_x/reg_city_y), sets return_flag, and walks one tick via region_go_to_target.
// FUNCTION: C2 0x4742a
// FUNCTION: C2WIN 0x00406f66
void sa07_army_land_invade(void)
{
    army_list[army_no].target_x      = reg_city_x;
    army_list[army_no].target_y      = reg_city_y;
    army_list[army_no].return_flag = 1;
    if (region_go_to_target(0) == 0) return;
}

// Army state-8 "stuck" tick -- recovery handler when the straight path to target is blocked. On
// entering a fresh region cell, spirals outward from the desired heading (alternating right /
// left, wrapping 0..7) for up to 3 iterations.
// FUNCTION: C2 0x47452
// FUNCTION: C2WIN 0x00406fdb
void sa08_army_stuck(void)
{
    int left_dir;
    int right_dir;
    int i;
    signed char stuck_count;

    if (entering_new_square() != 0) {
        left_dir  = get_heading(
                        army_list[army_no].x,
                        army_list[army_no].y,
                        army_list[army_no].target_x,
                        army_list[army_no].target_y,
                        army_list[army_no].world_dir);
        right_dir = left_dir;
        for (i = 0; i < 3; i++) {
            right_dir++;
            if (right_dir >= 8) right_dir = 0;
            left_dir--;
            if (left_dir < 0)   left_dir = 7;
            if (try_a_regionmap_square(right_dir, 0, 0) != 0) {
                army_list[army_no].target_x +=
                    gmn_ofsets[right_dir].dx;
                army_list[army_no].target_y +=
                    gmn_ofsets[right_dir].dy;
                goto post_loop;
            }
            if (try_a_regionmap_square(left_dir, 0, 0) != 0) {
                army_list[army_no].target_x +=
                    gmn_ofsets[left_dir].dx;
                army_list[army_no].target_y +=
                    gmn_ofsets[left_dir].dy;
                goto post_loop;
            }
            if (i == 2) {
                army_list[army_no].saved_state_idx = 1;
                army_list[army_no].wait_count      = 5;
                army_list[army_no].order_progress  = 1;
            }
        }
post_loop:
        stuck_count = ++army_list[army_no].stuck_timer;
        if (stuck_count > 4) {
            army_list[army_no].state_idx     = 1;
            army_list[army_no].wait_count    = 0xa;
            army_list[army_no].order_progress = 0;
            army_list[army_no].wf_active     = 0;
            return;
        }
    }
    if (region_go_to_target(0) == 0) return;
    if ((army_list[army_no].flags & 1) == 0) return;
    army_list[army_no].state_idx = army_list[army_no].saved_state_idx;
}

// Run a siege countdown based on troop strength, then clear the stronghold area and resume
// invasion.
// FUNCTION: C2 0x475d2
// FUNCTION: C2WIN 0x0040735d
void sa09_army_siege(void)
{
    int siege_period;
    unsigned char stuck_count;
    unsigned char siege_count;

    army_list[army_no].sprite_anim = 0x66;

    if (army_list[army_no].total_troops <= 10)       siege_period = 40;
    else if (army_list[army_no].total_troops <= 50)  siege_period = 30;
    else if (army_list[army_no].total_troops <= 100) siege_period = 25;
    else if (army_list[army_no].total_troops <= 200) siege_period = 20;
    else if (army_list[army_no].total_troops <= 400) siege_period = 15;
    else if (army_list[army_no].total_troops <= 800) siege_period = 12;
    else                                             siege_period = 8;

    stuck_count = ++army_list[army_no].stuck_timer;
    if ((signed char)stuck_count < 50) return;
    army_list[army_no].stuck_timer = 0;
    siege_count = ++army_list[army_no].wf_phase;
    if ((signed char)siege_count < siege_period) return;

    clear_a_reg_area(army_list[army_no].x - 1,
                     army_list[army_no].y - 1,
                     army_list[army_no].x + 1,
                     army_list[army_no].y + 1, 1);
    army_list[army_no].state_idx = 7;
    put_message(0x60, army_list[army_no].map_ref, 0x13);
}

// Demobilise the army at its home fort and reset its movement and targeting state.
// FUNCTION: C2 0x476cf
// FUNCTION: C2WIN 0x004075e1
void sa10_army_demobed(void)
{
    army_list[army_no].return_flag = 0;
    (*(struct region_cell *)((unsigned char *)region_map + (army_list[army_no].map_ref))).occupant = 0;
    (*(struct region_cell *)((unsigned char *)region_map + (army_list[army_no].fort_ref))).occupant = army_no;
    army_list[army_no].map_ref = army_list[army_no].fort_ref;
    army_list[army_no].saved_state_idx = 1;
    army_list[army_no].target_count = 0;
    army_list[army_no].target_kind  = 0;
    army_list[army_no].flags |= 1;
    army_list[army_no].pixel_x = 0;
    army_list[army_no].pixel_y = 0;
    army_list[army_no].world_dir = 0;
    army_list[army_no].x = (char)((army_list[army_no].fort_ref / 8) % 60);
    army_list[army_no].y = (char)((army_list[army_no].fort_ref / 8) / 60);
}

// Army state-11 (sail to player's port): target the city tile and walk via sail_to_target. On
// arrival (flags bit 3 set) reset wf_phase, transition to state 0xD (sail-round-coast), and stash
// the city as the new dest.
// FUNCTION: C2 0x4777f
// FUNCTION: C2WIN 0x004077c2
void sa11_army_sail_to_port(void)
{
    army_list[army_no].target_x      = reg_city_x;
    army_list[army_no].target_y      = reg_city_y;
    army_list[army_no].return_flag = 1;
    if (sail_to_target(0) == 0) {
    } else {
        if (army_list[army_no].flags & 8) {
            army_list[army_no].wf_phase  = 0;
            army_list[army_no].state_idx = 0xD;
            army_list[army_no].dest_x    = reg_city_x;
            army_list[army_no].dest_y    = reg_city_y;
        }
        if (army_list[army_no].flags & 4) {
        }
    }
}

// Army state-12 (sail home) tick: moves a sea-borne army back to its launch port (home_x/home_y).
// FUNCTION: C2 0x477fd
// FUNCTION: C2WIN 0x004078e5
void sa12_army_sail_home(void)
{
    int distance;

    army_list[army_no].target_x = army_list[army_no].home_x;
    army_list[army_no].target_y = army_list[army_no].home_y;
    distance = get_longest_distance(army_list[army_no].x,
                                army_list[army_no].y,
                                army_list[army_no].home_x,
                                army_list[army_no].home_y);
    if (distance > 1) {
        if (army_list[army_no].x != army_list[army_no].target_x) {
            if (army_list[army_no].home_x <= 0)
                army_list[army_no].target_x++;
            else if (army_list[army_no].home_x >= 0x3b)
                army_list[army_no].target_x--;
        }
        if (army_list[army_no].y != army_list[army_no].target_y) {
            if (army_list[army_no].home_y <= 0)
                army_list[army_no].target_y++;
            else if (army_list[army_no].home_y >= 0x3b)
                army_list[army_no].target_y--;
        }
    }
    army_list[army_no].return_flag = 1;
    if (sail_to_target(0) == 0) return;
    if ((army_list[army_no].flags & 8) != 0)
        army_list[army_no].state_idx = 0xf;
    if ((army_list[army_no].flags & 2) != 0)
        army_list[army_no].state_idx = 0xf;
    if ((army_list[army_no].flags & 4) != 0)
        army_list[army_no].state_idx = 0xf;
}

// Search along the coast for a beaching cell, returning the sea army home after a timeout.
// FUNCTION: C2 0x47967
// FUNCTION: C2WIN 0x00407c18
void sa13_army_sail_round_coast(void)
{
    int heading;
    int attempts;
    int cell_result;
    signed char phase_count;

    army_list[army_no].return_flag = 1;
    if (sail_to_target(0) == 0) return;
    if ((army_list[army_no].flags & 0xa) == 0) return;
    army_list[army_no].flags &= 0xfd;
    phase_count = ++army_list[army_no].wf_phase;
    if (phase_count > 0x14) {
        army_list[army_no].state_idx = 0xc;
        return;
    }
    army_list[army_no].target_x = army_list[army_no].dest_x;
    army_list[army_no].target_y = army_list[army_no].dest_y;
    heading = get_heading(army_list[army_no].x,
                          army_list[army_no].y,
                          army_list[army_no].target_x,
                          army_list[army_no].target_y,
                          army_list[army_no].world_dir);
    for (attempts = 0; attempts < 8; attempts++) {
        cell_result = try_a_seamap_square(heading, 0, 0);
        if (cell_result == 1) {
            army_list[army_no].target_x =
                army_list[army_no].x +
                gmn_ofsets[heading].dx;
            army_list[army_no].target_y =
                army_list[army_no].y +
                gmn_ofsets[heading].dy;
            return;
        }
        if (cell_result == 2) {
            army_list[army_no].state_idx = 0xf;
            return;
        }
        if (cell_result == 3 && (heading & 1) == 0) {
            army_list[army_no].state_idx       = 1;
            army_list[army_no].saved_state_idx = 0xc;
            if (dock_the_ship_in_good_port(heading) != 0)
                army_list[army_no].wait_count = 0xa;
            else
                army_list[army_no].wait_count = 0x64;
            army_list[army_no].quick_respawn = 1;
        } else {
            army_list[army_no].dest_x =
                army_list[army_no].x +
                gmn_ofsets[heading].dx;
            army_list[army_no].dest_y =
                army_list[army_no].y +
                gmn_ofsets[heading].dy;
        }
        if (army_list[army_no].army_id != 0) {
            heading--;
            if (heading < 0) heading = 7;
        } else {
            heading++;
            if (heading >= 8) heading = 0;
        }
        if (attempts >= 7)
            army_list[army_no].state_idx = 0xf;
    }
}

// Army state-14 (sail to shore for an invasion): target the city, sail one tick; on landfall
// (landed_flag set) transition to state 0x10 (sa16_lurk) with the city as dest, or otherwise fall
// back to the saved state on a successful waypoint.
// FUNCTION: C2 0x47b80
// FUNCTION: C2WIN 0x00408076
void sa14_army_sail_to_shore(void)
{
    army_list[army_no].target_x      = reg_city_x;
    army_list[army_no].target_y      = reg_city_y;
    army_list[army_no].return_flag = 1;
    sail_to_target(0);
    if (army_list[army_no].landed_flag) {
        army_list[army_no].state_idx = 0x10;
        army_list[army_no].flags    &= 0xFD;
        army_list[army_no].dest_x    = reg_city_x;
        army_list[army_no].dest_y    = reg_city_y;
        return;
    }
    if (army_list[army_no].flags & 0xA)
        army_list[army_no].state_idx = army_list[army_no].saved_state_idx;
}

// Naval trader sink handler. Schedules respawn of the appropriate trader (via the corresponding
// compass-side `*_trader_count*` global) and removes the army.
// FUNCTION: C2 0x47c10
// FUNCTION: C2WIN 0x004081dc
void sa15_sink(void)
{
    int delay;

    delay = (army_list[army_no].quick_respawn != 0) ? 2 : 15;
    if (army_list[army_no].compass_side == 0) {            /* north */
        if (army_list[army_no].army_id == 0)
            north_trader_count0 = delay;
        else
            north_trader_count1 = delay;
    } else if (army_list[army_no].compass_side == 2) {     /* east  */
        if (army_list[army_no].army_id == 0)
            east_trader_count0 = delay;
        else
            east_trader_count1 = delay;
    } else if (army_list[army_no].compass_side == 4) {     /* south */
        if (army_list[army_no].army_id == 0)
            south_trader_count0 = delay;
        else
            south_trader_count1 = delay;
    } else if (army_list[army_no].compass_side == 6) {     /* west  */
        if (army_list[army_no].army_id == 0)
            west_trader_count0 = delay;
        else
            west_trader_count1 = delay;
    }
    remove_army(army_no);
}

// Sea-army state-16 (lurk round coast): a sailing army that's blocked from its current target
// tries up to 8 alternate headings around the coastline before giving up. Only runs when
// sail_to_target reports progress with the "stuck" or "land/grounded" flag bits set.
// FUNCTION: C2 0x47cd8
// FUNCTION: C2WIN 0x004083a5
void sa16_army_lurk_round_coast(void)
{
    int heading;
    int attempts;
    int cell_result;

    army_list[army_no].return_flag = 1;
    if (sail_to_target(0) == 0) return;
    if ((army_list[army_no].flags & 0xa) == 0) return;
    army_list[army_no].flags &= 0xfd;
    army_list[army_no].target_x = army_list[army_no].dest_x;
    army_list[army_no].target_y = army_list[army_no].dest_y;
    heading = get_heading(army_list[army_no].x,
                          army_list[army_no].y,
                          army_list[army_no].target_x,
                          army_list[army_no].target_y,
                          army_list[army_no].world_dir);
    for (attempts = 0; attempts < 8; attempts++) {
        cell_result = try_a_seamap_square(heading, 0, 0);
        if (cell_result == 1) {
            army_list[army_no].target_x =
                army_list[army_no].x +
                gmn_ofsets[heading].dx;
            army_list[army_no].target_y =
                army_list[army_no].y +
                gmn_ofsets[heading].dy;
            return;
        }
        if (cell_result == 2) {
            army_list[army_no].state_idx = 0xf;
            return;
        }
        if (cell_result == 0 && army_list[army_no].landed_flag == 0) {
            army_list[army_no].state_idx =
                army_list[army_no].saved_state_idx;
            return;
        }
        army_list[army_no].dest_x =
            army_list[army_no].x +
            gmn_ofsets[heading].dx;
        army_list[army_no].dest_y =
            army_list[army_no].y +
            gmn_ofsets[heading].dy;
        if (army_list[army_no].army_id != 0) {
            heading--;
            if (heading < 0) heading = 7;
        } else {
            heading++;
            if (heading >= 8) heading = 0;
        }
        if (attempts >= 7)
            army_list[army_no].state_idx = 0xf;
    }
}

// Select the current citizen's walking sprite from its facing direction and animation phase.
// FUNCTION: C2 0x47e9d
// FUNCTION: C2WIN 0x0040875d
void get_movement_image(int image_base)
{
    int screen_dir;

    screen_dir = citizen_list[citizen_no].world_dir - map_direction;
    if (screen_dir < 0) screen_dir += 8;
    switch (screen_dir) {
    case 0: break;
    case 1: image_base += 3; break;
    case 2: image_base += 6; break;
    case 3: image_base += 9; break;
    case 4: image_base += 12; break;
    case 5: image_base += 15; break;
    case 6: image_base += 18; break;
    case 7: image_base += 21; break;
    }
    switch (citizen_list[citizen_no].speed_count & 3) {
    case 1:
    case 3: image_base += 1; break;
    case 2: image_base += 2; break;
    }
    citizen_list[citizen_no].image_id = image_base;
}

// Compute screen-direction sprite frame for the current army's ship. Rotates the army's absolute
// world heading by `map_direction`, wraps to the [0..7] range, and stores `img_base + dir` into
// the army's sprite_image slot.
// FUNCTION: C2 0x47f29
// FUNCTION: C2WIN 0x0040889c
void get_army_ship_image(int image_base)
{
    int screen_dir;
    screen_dir = army_list[army_no].world_dir - map_direction + 1;
    if (screen_dir < 0)  screen_dir += 8;
    if (screen_dir >= 8) screen_dir %= 8;
    image_base += screen_dir;
    army_list[army_no].sprite_image = image_base;
}

// Pick the rioter sprite based on the current wait_count: chunked into 5-tick bands, even bands
// use img + 0x1B and odd bands use img + 0x1C. Used by i07_rioter_man for the mob-walking sprite.
// FUNCTION: C2 0x47f7a
// FUNCTION: C2WIN 0x00408918
void get_rioter_image(int image_base)
{
    if (citizen_list[citizen_no].wait_count < 5) {
        citizen_list[citizen_no].image_id = (image_base + 0x1B);
        return;
    }
    if (citizen_list[citizen_no].wait_count < 10) {
        citizen_list[citizen_no].image_id = (image_base + 0x1C);
        return;
    }
    if (citizen_list[citizen_no].wait_count < 15) {
        citizen_list[citizen_no].image_id = (image_base + 0x1B);
        return;
    }
    if (citizen_list[citizen_no].wait_count < 20) {
        citizen_list[citizen_no].image_id = (image_base + 0x1C);
        return;
    }
    if (citizen_list[citizen_no].wait_count < 25) {
        citizen_list[citizen_no].image_id = (image_base + 0x1B);
        return;
    }
    citizen_list[citizen_no].image_id = (image_base + 0x1C);
}

// Select the barbarian army's sprite and animation from its heading, tribe, and state.
// FUNCTION: C2 0x47fc7
// FUNCTION: C2WIN 0x00408aa0
void get_barbarian_image(void)
{
    int dir;
    int i;

    army_list[army_no].sprite_dir = 0;
    dir = army_list[army_no].world_dir - map_direction + 1;
    if (dir < 0)  dir += 8;
    if (dir >= 8) dir %= 8;

    if (army_list[army_no].state_idx >= 14) {
        army_list[army_no].sprite_image = (char)(dir + 0x56);
        army_list[army_no].sprite_anim = 0;
    } else {
        army_list[army_no].sprite_image = tribe_to_standard[
            army_list[army_no].tribe_id];
        army_list[army_no].sprite_anim = (get_army_walk_dirc(dir,
            army_list[army_no].target_kind) + 0x2a);
    }
}

// Compute the sprite frame for an enemy (barbarian / brigand) army. At advanced state (state_idx
// >= 14, charging/dead) the frame is a fixed `dir + 0x5e` direction-only image with no animation
// cycle.
// FUNCTION: C2 0x48079
// FUNCTION: C2WIN 0x00408beb
void get_enemy_image(void)
{
    int dir;
    int i;

    dir = army_list[army_no].world_dir - map_direction + 1;
    if (dir < 0)  dir += 8;
    if (dir >= 8) dir %= 8;

    if (army_list[army_no].state_idx >= 14) {
        army_list[army_no].sprite_image = (char)(dir + 0x5e);
        army_list[army_no].sprite_anim = 0;
        army_list[army_no].sprite_dir  = 0;
    } else {
        army_list[army_no].sprite_image = tribe_to_standard[
            army_list[army_no].tribe_id];
        army_list[army_no].sprite_anim = (cnt8 + 0x1a);
        army_list[army_no].sprite_dir  = (get_army_walk_dirc(dir,
            army_list[army_no].target_kind) + 0x42);
    }
}

// Select the cohort army's sprite direction and animation from its heading, cohort, and state.
// FUNCTION: C2 0x48130
// FUNCTION: C2WIN 0x00408d56
void get_cohort_image(void)
{
    int dir;
    int i;

    dir = army_list[army_no].world_dir - map_direction + 1;
    if (dir < 0)  dir += 8;
    if (dir >= 8) dir %= 8;

    army_list[army_no].sprite_image = army_list[army_no].cohort_id;
    if (army_list[army_no].state_idx == 10)
        army_list[army_no].sprite_anim = 0x12;
    else
        army_list[army_no].sprite_anim = (cnt8 + 0x12);
    army_list[army_no].sprite_dir = (get_army_walk_dirc(dir,
        army_list[army_no].target_kind) + 0x36);
}

// Convert a screen-relative heading and animation phase into a directional sprite offset.
// FUNCTION: C2 0x481ce
// FUNCTION: C2WIN 0x00408e82
int get_army_walk_dirc(int screen_dir, int anim_phase)
{
    int frame_offset;
    if (screen_dir == 0)      frame_offset = 6;
    else if (screen_dir == 4) frame_offset = 0;
    else if (screen_dir < 4)  frame_offset = 9;
    else               frame_offset = 3;
    anim_phase &= 3;
    if (anim_phase == 0) {
    } else {
        if (anim_phase == 2) frame_offset += 2;
        else          frame_offset++;
    }
    return frame_offset;
}

// Scan the citizen list for the nearest enemy citizen (type 3 or 7) within a Chebyshev radius of
// (cx, cy). Returns the citizen index of the best (closest) match, or 0 if none.
// FUNCTION: C2 0x481ff
// FUNCTION: C2WIN 0x00408f06
int find_enemy(int center_x, int center_y, int radius)
{
    int x_min;
    int x_max;
    int y_max;
    int best_idx;
    int best_dist;
    int y_min;
    int distance;

    x_min = center_x - radius;        if (x_min < 0)    x_min = 0;
    x_max = center_x + radius;        if (x_max >= 0x50) x_max = 0x4f;
    y_min = center_y - radius;        if (y_min < 0)    y_min = 0;
    y_max = center_y + radius;        if (y_max >= 0x50) y_max = 0x4f;
    best_dist = radius + 1;
    best_idx = 0;
    for (enemy_citizen = 0; enemy_citizen < 0xc9; enemy_citizen++) {
        if (citizen_list[enemy_citizen].exists == 0) continue; if (citizen_list[enemy_citizen].type != 3 && citizen_list[enemy_citizen].type != 7) continue;
        if (citizen_list[enemy_citizen].x < x_min || citizen_list[enemy_citizen].x >= x_max) continue; if (citizen_list[enemy_citizen].y < y_min || citizen_list[enemy_citizen].y >= y_max) continue;
        distance = get_longest_distance(center_x, center_y, citizen_list[enemy_citizen].x, citizen_list[enemy_citizen].y);
        if (distance < best_dist) { best_dist = distance; best_idx = enemy_citizen; }
    }
    return best_idx;
}

// Find the nearest active invading army of type 2 through 5 within the requested radius.
// FUNCTION: C2 0x482f1
// FUNCTION: C2WIN 0x0040910e
int find_invading_army(int center_x, int center_y, int radius)
{
    int x_min;
    int y_min;
    int y_max;
    int x_max;
    int best_idx;
    int best_dist;
    int army_x;
    int army_y;
    int distance;
    int army_type;

    x_min = center_x - radius;        if (x_min < 0)    x_min = 0;
    x_max = center_x + radius;        if (x_max >= 0x3c) x_max = 0x3b;
    y_min = center_y - radius;        if (y_min < 0)    y_min = 0;
    y_max = center_y + radius;        if (y_max >= 0x3c) y_max = 0x3b;
    best_dist = radius + 1;
    best_idx = 0;
    for (enemy_army = 0; enemy_army < 0x1a; enemy_army++) {
        if (army_list[enemy_army].exists == 0) continue;
        army_type = army_list[enemy_army].type;
        if (army_type < 2 || army_type > 5) continue;
        if (army_list[enemy_army].state_idx >= 14) continue;
        army_x = army_list[enemy_army].x;
        if (army_x < x_min || army_x >= x_max) continue;
        army_y = army_list[enemy_army].y;
        if (army_y < y_min || army_y >= y_max) continue;
        distance = get_longest_distance(center_x, center_y, army_x, army_y);
        if (distance < best_dist) {
            best_dist = distance;
            best_idx = enemy_army;
        }
    }
    return best_idx;
}

// Advance the current citizen directly toward its destination, respecting its movement speed.
// FUNCTION: C2 0x483df
// FUNCTION: C2WIN 0x00409336
int citizen_go_to_target(int movement_kind)
{
    int movement_value;

    if ((citizen_list[citizen_no].flag_bits & 1) == 0) {
        if (citizen_list[citizen_no].is_barbarian) movement_value = citizen_speed_on_road[citizen_list[citizen_no].type];
        else movement_value = citizen_speed_off_road[citizen_list[citizen_no].type];
        citizen_list[citizen_no].speed_phase = citizen_list[citizen_no].speed_phase + 1;
        if (citizen_list[citizen_no].speed_phase > movement_value) {
            citizen_list[citizen_no].speed_phase = 0; citizen_list[citizen_no].speed_count++; if (citizen_list[citizen_no].speed_count > 0xf) citizen_list[citizen_no].flag_bits |= 1;
        }
        return 0;
    }

    citizen_list[citizen_no].speed_count = 0; citizen_list[citizen_no].speed_phase = 0;
    if (citizen_list[citizen_no].action_kind == 0) return 1;

    w_dirc = (char)get_heading(citizen_list[citizen_no].x,
                               citizen_list[citizen_no].y,
                               citizen_list[citizen_no].dest_x,
                               citizen_list[citizen_no].dest_y,
                               citizen_list[citizen_no].world_dir);
    if (w_dirc >= 8) { citizen_list[citizen_no].action_kind = 0; citizen_list[citizen_no].flag_bits |= 2; return 1; }
    movement_value = try_a_citymap_square(w_dirc, 0, 0);
    if (movement_value > 2) {
        citizen_list[citizen_no].state_idx = 1; citizen_list[citizen_no].wait_count = 0x14; citizen_list[citizen_no].world_dir = (citizen_list[citizen_no].world_dir + 4) & 7;
        return 1;
    }
    if (movement_kind == 0 && movement_value == 2) {
        citizen_list[citizen_no].state_idx = 1; citizen_list[citizen_no].wait_count = 0x14; citizen_list[citizen_no].world_dir = (citizen_list[citizen_no].world_dir + 4) & 7;
        return 1;
    }
    if (movement_value == 0) {
        citizen_list[citizen_no].state_idx = 1; citizen_list[citizen_no].wait_count = 0x14; citizen_list[citizen_no].world_dir = (citizen_list[citizen_no].world_dir + 4) & 7;
        return 1;
    }
    citizen_list[citizen_no].is_barbarian = 1;
    citizen_list[citizen_no].flag_bits &= 0xfe;
    citizen_list[citizen_no].world_dir = w_dirc;
    citizen_list[citizen_no].speed_count = 1;
    move_citizen();
    return 1;
}

// Advance the current citizen toward its destination, using a calculated route when blocked.
// FUNCTION: C2 0x48569
// FUNCTION: C2WIN 0x00409815
int citizen_maraude_to_target(int movement_kind)
{
    int movement_value;

    if ((citizen_list[citizen_no].flag_bits & 1) == 0) {
        if (citizen_list[citizen_no].is_barbarian) movement_value = citizen_speed_on_road[citizen_list[citizen_no].type];
        else movement_value = citizen_speed_off_road[citizen_list[citizen_no].type];
        citizen_list[citizen_no].speed_phase = citizen_list[citizen_no].speed_phase + 1;
        if (citizen_list[citizen_no].speed_phase > movement_value) {
            citizen_list[citizen_no].speed_phase = 0; citizen_list[citizen_no].speed_count++; if (citizen_list[citizen_no].speed_count > 0xf) citizen_list[citizen_no].flag_bits |= 1;
        }
        return 1;
    }

    citizen_list[citizen_no].speed_count = 0; citizen_list[citizen_no].speed_phase = 0;

    if (citizen_list[citizen_no].action_kind == 0) return 1;

    w_dirc = (char)get_heading(
        citizen_list[citizen_no].x,
        citizen_list[citizen_no].y,
        citizen_list[citizen_no].dest_x,
        citizen_list[citizen_no].dest_y,
        citizen_list[citizen_no].world_dir);

    if (w_dirc >= 8) { citizen_list[citizen_no].state_idx = 1; citizen_list[citizen_no].wait_count = 0x78; citizen_list[citizen_no].flag_bits |= 2; return 1; }

    if (citizen_list[citizen_no].wf_active) get_dirc_from_citizen_wf_run();

    movement_value = try_a_citymap_square(w_dirc, movement_kind, 0);

    if (movement_value == 0x3e7 || (movement_value == 0 && citizen_list[citizen_no].wf_active)) {
        citizen_list[citizen_no].world_dir = (char)((citizen_list[citizen_no].world_dir + 1) & 7);
        if (citizen_list[citizen_no].wf_active) citizen_list[citizen_no].wf_active = 0;
        else { citizen_list[citizen_no].state_idx = 1; citizen_list[citizen_no].wait_count = 0x10; }
        return 1;
    }

    if (movement_value == 0) {
        citizen_list[citizen_no].wf_active = 0; citizen_list[citizen_no].speed = 0x14;
        clear_ferret_map(citizen_list[citizen_no].speed,
                         (unsigned char *)city_map,
                         0x50, 0x50,
                         0x14,
                         citizen_list[citizen_no].x,
                         citizen_list[citizen_no].y,
                         citizen_list[citizen_no].dest_x,
                         citizen_list[citizen_no].dest_y);
        if (run_2_map_ferrets(citizen_list[citizen_no].speed,
                              (unsigned char *)city_map,
                              0x50, 0x50,
                              0x14,
                              citizen_list[citizen_no].x,
                              citizen_list[citizen_no].y,
                              citizen_list[citizen_no].dest_x,
                              citizen_list[citizen_no].dest_y)
            == 0) {
            change_citizen_targs(0x12);
            citizen_list[citizen_no].state_idx = 1; citizen_list[citizen_no].wait_count = 0x14; citizen_list[citizen_no].world_dir = (char)((citizen_list[citizen_no].world_dir + 1) & 7);
            return 0;
        }
        copy_ferret_run_to_citizen();
        return 1;
    }

    if (movement_value == 1) citizen_list[citizen_no].is_barbarian = 1;
    else citizen_list[citizen_no].is_barbarian = 0;
    citizen_list[citizen_no].flag_bits &= 0xfe;
    citizen_list[citizen_no].world_dir = w_dirc;
    citizen_list[citizen_no].speed_count = 1;
    move_citizen();
    return 1;
}

// Pull the next walking-ferret-run direction for the current citizen. `wf_steps[]` packs two 4-bit
// headings per byte: low nibble for even steps, high nibble for odd steps.
// FUNCTION: C2 0x48847
// FUNCTION: C2WIN 0x00409ec7
void get_dirc_from_citizen_wf_run(void)
{
    if (citizen_list[citizen_no].wf_step
        >= citizen_list[citizen_no].wf_length) {
        citizen_list[citizen_no].wf_active = 0;
        return;
    }
    w_dirc = citizen_list[citizen_no].wf_steps[
        citizen_list[citizen_no].wf_step >> 1];
    if ((citizen_list[citizen_no].wf_step & 1) != 0)
        w_dirc >>= 4;
    else
        w_dirc &= 0xf;
    citizen_list[citizen_no].wf_step++;
}

// Pack the calculated route into the current citizen's nibble-encoded walking steps and activate it.
// FUNCTION: C2 0x488bb
// FUNCTION: C2WIN 0x00409fb6
void copy_ferret_run_to_citizen(void)
{
    int i;
    int j;
    unsigned char value;

    citizen_list[citizen_no].wf_active = 1; citizen_list[citizen_no].wf_step = 0; citizen_list[citizen_no].wf_length = ferret_run_length;
    w_dirc = ferret_run[0];
    i = 0; j = 0;
    for (; i < ferret_run_length; i++) {
        value = ferret_run[i];
        if ((i & 1) == 0) citizen_list[citizen_no].wf_steps[j] = value;
        else { value <<= 4; citizen_list[citizen_no].wf_steps[j] += value; j++; }
    }
}

// Test the neighbouring city-map cell in compass direction `dir` for the current citizen path.
// FUNCTION: C2 0x48955
// FUNCTION: C2WIN 0x0040a0c4
int try_a_citymap_square(int direction, int movement_kind, int unused_arg)
{
    int result;

    result = 0;
    enemy_figure = 0;

    switch ((unsigned int)direction) {
    case 0:
        if (citizen_list[citizen_no].y <= 0)
            result = 0;
        else
            result = try_this_citymap_square(citizen_list[citizen_no].map_ref - 0x640, movement_kind, unused_arg);
        break;
    case 1:
        if (citizen_list[citizen_no].x < 0x4f) {
            if (citizen_list[citizen_no].y <= 0)
                result = 0;
            else
                result = try_this_citymap_square(citizen_list[citizen_no].map_ref - 0x62c, movement_kind, unused_arg);
        } else {
            result = 0;
        }
        break;
    case 2:
        if (citizen_list[citizen_no].x < 0x4f)
            result = try_this_citymap_square(citizen_list[citizen_no].map_ref + 0x14, movement_kind, unused_arg);
        else
            result = 0;
        break;
    case 3:
        if (citizen_list[citizen_no].x < 0x4f) {
            if (citizen_list[citizen_no].y < 0x4f)
                result = try_this_citymap_square(citizen_list[citizen_no].map_ref + 0x654, movement_kind, unused_arg);
            else
                result = 0;
        } else {
            result = 0;
        }
        break;
    case 4:
        if (citizen_list[citizen_no].y < 0x4f)
            result = try_this_citymap_square(citizen_list[citizen_no].map_ref + 0x640, movement_kind, unused_arg);
        else
            result = 0;
        break;
    case 5:
        if (citizen_list[citizen_no].x <= 0)
            result = 0;
        else if (citizen_list[citizen_no].y < 0x4f)
            result = try_this_citymap_square(citizen_list[citizen_no].map_ref + 0x62c, movement_kind, unused_arg);
        else
            result = 0;
        break;
    case 6:
        if (citizen_list[citizen_no].x <= 0)
            result = 0;
        else
            result = try_this_citymap_square(citizen_list[citizen_no].map_ref - 0x14, movement_kind, unused_arg);
        break;
    case 7:
        if (citizen_list[citizen_no].x <= 0)
            result = 0;
        else if (citizen_list[citizen_no].y <= 0)
            result = 0;
        else
            result = try_this_citymap_square(citizen_list[citizen_no].map_ref - 0x654, movement_kind, unused_arg);
        break;
    }
    return result;
}

// Test whether the current citizen can enter a city-map cell, resolving collisions and trampling.
// FUNCTION: C2 0x48aeb
// FUNCTION: C2WIN 0x0040a4c0
int try_this_citymap_square(int cell_offset, int movement_kind, int unused_arg)
{
    unsigned char terrain;
    unsigned char industry;
    int cell_x;
    int cell_y;
    int cell_idx;

    (void)unused_arg;
    citizen_a = (short)(unsigned char)(*(struct city_cell *)((unsigned char *)city_map + (cell_offset))).citizen_a;
    citizen_b = (short)(unsigned char)(*(struct city_cell *)((unsigned char *)city_map + (cell_offset))).citizen_b;
    terrain   = (*(struct city_cell *)((unsigned char *)city_map + (cell_offset))).terrain;

    if (citizen_a != 0) handle_collision(citizen_a);
    if (citizen_b != 0) handle_collision(citizen_b);

    if (citizen_a != 0 && citizen_b != 0) return 0x3e7;

    if ((terrain & 0x20) != 0) return 1;

    if (movement_kind != 1) {
        if (terrain == 0) goto ret2;
        return 0;
    }

    if ((terrain & 4) != 0) return 0;
    if ((terrain & 0x18) != 0) return 0;

    if ((terrain & 2) != 0) {
        industry = (*(struct city_cell *)((unsigned char *)city_map + (cell_offset))).industrial;
        industry++;
        if (industry > 0xc) {
            destroy_an_atom(cell_offset, 0);
            goto ret2;
        }
        (*(struct city_cell *)((unsigned char *)city_map + (cell_offset))).industrial = industry;
        return 0;
    }

    if (terrain != 0) {
        if ((terrain & 0xc0) != 0) {
            unflag_all_cm(3, 0xdf);
        }

        cell_idx = cell_offset / 20;
        cell_x = cell_idx % 80;
        cell_y = cell_idx / 80;
        clear_an_area(cell_x, cell_y, cell_x, cell_y);
#if C2_FEAT_TILE_REFRESH
        setup_map_screen_refresh();
#endif
        particles_cleared = 0;
        citizen_list[citizen_no].state_idx = 1;
        citizen_list[citizen_no].wait_count = 0x20;
        citizen_list[citizen_no].wf_active = 1;
        return 0;
    }
    destroy_an_atom(cell_offset, 0);
    return 2;
ret2:
    return 2;
}

// Two citizens have ended up on the same map cell.
// FUNCTION: C2 0x48c6a
// FUNCTION: C2WIN 0x0040a702
void handle_collision(int other_idx)
{
    int citizen_type;

    if (citizen_list[citizen_no].state_idx == 2) return;
    citizen_type = citizen_list[citizen_no].type;
    if (citizen_type == 4 || citizen_type == 5) {
        if (citizen_list[other_idx].type == 3) {
            fight_barbarian(other_idx);
            return;
        }
        if (citizen_list[other_idx].type == 7) {
            citizen_list[other_idx].state_idx = 2;
            return;
        }
        return;
    }
    if (citizen_type == 7) {
        if (citizen_list[other_idx].type == 3) {
            citizen_list[citizen_no].state_idx = 2;
            return;
        }
        if (citizen_list[other_idx].type == 4) {
            citizen_list[citizen_no].state_idx = 2;
            return;
        }
        if (citizen_list[other_idx].type == 5) {
            citizen_list[citizen_no].state_idx = 2;
            return;
        }
        if (citizen_list[other_idx].type != citizen_type)
            citizen_list[other_idx].state_idx = 2;
        return;
    }
    if (citizen_type == 3) {
        if (citizen_list[other_idx].type == citizen_type) return;
        if (citizen_list[other_idx].type == 4) {
            fight_centurian(other_idx);
            return;
        }
        if (citizen_list[other_idx].type == 5) {
            fight_centurian(other_idx);
            return;
        }
        citizen_list[other_idx].state_idx = 2;
        return;
    }
    if (citizen_list[other_idx].type == 3) {
        citizen_list[citizen_no].state_idx = 2;
        return;
    }
    if (citizen_list[other_idx].type == 7) {
        citizen_list[citizen_no].state_idx = 2;
        return;
    }
}

// Resolve a centurion or vigile fighting a barbarian and award the patrol citizen experience.
// FUNCTION: C2 0x48d32
// FUNCTION: C2WIN 0x0040aa40
void fight_centurian(int other_idx)
{
    int strength;

    strength = citizen_list[citizen_no].xp + 2;
    if (strength < rand8) citizen_list[other_idx].state_idx        = 2;
    else                                          citizen_list[citizen_no].state_idx = 2;
    citizen_list[citizen_no].xp++;
}

// Resolve a barbarian's skirmish with a centurian and award the barbarian experience.
// FUNCTION: C2 0x48d6f
// FUNCTION: C2WIN 0x0040aac6
void fight_barbarian(int other_idx)
{
    int strength;

    strength = citizen_list[other_idx].xp + 2;
    if (strength < rand8) citizen_list[citizen_no].state_idx = 2;
    else                                  citizen_list[other_idx].state_idx        = 2;
    citizen_list[other_idx].xp++;
}

// Move citizen one cell in citizen.world_dir (0..7).
// FUNCTION: C2 0x48dc1
// FUNCTION: C2WIN 0x0040ab44
void move_citizen(void)
{
    int cell_offset;

    cell_offset = citizen_list[citizen_no].map_ref;
    if ((*(struct city_cell *)((unsigned char *)city_map + (cell_offset))).citizen_a == citizen_no) {
        (*(struct city_cell *)((unsigned char *)city_map + (cell_offset))).citizen_a = 0;
    } else if ((*(struct city_cell *)((unsigned char *)city_map + (cell_offset))).citizen_b == citizen_no) {
        (*(struct city_cell *)((unsigned char *)city_map + (cell_offset))).citizen_b = 0;
    }

    switch ((unsigned char)citizen_list[citizen_no].world_dir) {
    case 0:
        citizen_list[citizen_no].y--;
        citizen_list[citizen_no].map_ref -= 0x640;
        break;
    case 1:
        citizen_list[citizen_no].y--;
        citizen_list[citizen_no].x++;
        citizen_list[citizen_no].map_ref -= 0x62c;
        break;
    case 2:
        citizen_list[citizen_no].x++;
        citizen_list[citizen_no].map_ref += 0x14;
        break;
    case 3:
        citizen_list[citizen_no].y++;
        citizen_list[citizen_no].x++;
        citizen_list[citizen_no].map_ref += 0x654;
        break;
    case 4:
        citizen_list[citizen_no].y++;
        citizen_list[citizen_no].map_ref += 0x640;
        break;
    case 5:
        citizen_list[citizen_no].y++;
        citizen_list[citizen_no].x--;
        citizen_list[citizen_no].map_ref += 0x62c;
        break;
    case 6:
        citizen_list[citizen_no].x--;
        citizen_list[citizen_no].map_ref -= 0x14;
        break;
    case 7:
        citizen_list[citizen_no].y--;
        citizen_list[citizen_no].x--;
        citizen_list[citizen_no].map_ref -= 0x654;
        break;
    default:
        return;
    }

    cell_offset = citizen_list[citizen_no].map_ref;
    if ((*(struct city_cell *)((unsigned char *)city_map + (cell_offset))).citizen_a == 0) {
        (*(struct city_cell *)((unsigned char *)city_map + (cell_offset))).citizen_a = citizen_no;
        return;
    }
    if ((*(struct city_cell *)((unsigned char *)city_map + (cell_offset))).citizen_b == 0) {
        (*(struct city_cell *)((unsigned char *)city_map + (cell_offset))).citizen_b = citizen_no;
        return;
    }
    high_beep();
    remove_citizen(citizen_no);
}

// Clamp the current citizen's wander target to a nearby valid city-map cell.
// FUNCTION: C2 0x48f2e
// FUNCTION: C2WIN 0x0040af92
void change_citizen_targs(int target_delta)
{
    int unclamped_axes = 2;            /* hits 0 only when both axes are within delta */
    int cell_idx;

    /* ---- X axis ---- */
    if (citizen_list[citizen_no].dest_x >
        citizen_list[citizen_no].x + target_delta) {
        citizen_list[citizen_no].dest_x =
            citizen_list[citizen_no].x + target_delta;
    } else if (citizen_list[citizen_no].dest_x <
               citizen_list[citizen_no].x - target_delta) {
        citizen_list[citizen_no].dest_x =
            citizen_list[citizen_no].x - target_delta;
    } else {
        unclamped_axes = 1;
    }

    /* ---- Y axis ---- */
    if (citizen_list[citizen_no].dest_y >
        citizen_list[citizen_no].y + target_delta) {
        citizen_list[citizen_no].dest_y =
            citizen_list[citizen_no].y + target_delta;
    } else if (citizen_list[citizen_no].dest_y <
               citizen_list[citizen_no].y - target_delta) {
        citizen_list[citizen_no].dest_y =
            citizen_list[citizen_no].y - target_delta;
    } else {
        unclamped_axes--;
    }

    /* ---- Cell occupancy nudge / random retarget ---- */
    if (unclamped_axes == 0) {
        cell_idx = citizen_list[citizen_no].dest_x
                 + citizen_list[citizen_no].dest_y * 80;
        if (((*(struct city_cell *)((unsigned char *)city_map + ((cell_idx * CITY_CELL_BYTES)))).terrain & 0xDF) != 0) {
            /* Cell is non-empty — step target back one tile
             * toward home on whichever axis still differs. */
            if (citizen_list[citizen_no].dest_x >
                citizen_list[citizen_no].x)
                citizen_list[citizen_no].dest_x--;
            if (citizen_list[citizen_no].dest_x <
                citizen_list[citizen_no].x)
                citizen_list[citizen_no].dest_x++;
            if (citizen_list[citizen_no].dest_y >
                citizen_list[citizen_no].y)
                citizen_list[citizen_no].dest_y--;
            if (citizen_list[citizen_no].dest_y <
                citizen_list[citizen_no].y)
                citizen_list[citizen_no].dest_y++;
        } else {
            random_target();
        }
    }

    /* ---- Final 0..0x4F clamp ---- */
    if (citizen_list[citizen_no].dest_x < 0)
        citizen_list[citizen_no].dest_x = 0;
    if (citizen_list[citizen_no].dest_x > 0x4f)
        citizen_list[citizen_no].dest_x = 0x4f;
    if (citizen_list[citizen_no].dest_y < 0)
        citizen_list[citizen_no].dest_y = 0;
    if (citizen_list[citizen_no].dest_y > 0x4f)
        citizen_list[citizen_no].dest_y = 0x4f;
}

// Pull the current citizen's wander target a few steps closer to home, with a small per-axis
// random jitter.
// FUNCTION: C2 0x4910b
// FUNCTION: C2WIN 0x0040b422
void random_target(void)
{
    if (citizen_list[citizen_no].dest_x > citizen_list[citizen_no].x)
        citizen_list[citizen_no].dest_x =
            citizen_list[citizen_no].x + rand8 - 3;
    else if (citizen_list[citizen_no].dest_x <= citizen_list[citizen_no].x)
        citizen_list[citizen_no].dest_x =
            citizen_list[citizen_no].x + rand8 - 5;
    if (citizen_list[citizen_no].dest_y > citizen_list[citizen_no].y)
        citizen_list[citizen_no].dest_y =
            citizen_list[citizen_no].y + rand8 - 2;
    else if (citizen_list[citizen_no].dest_y <= citizen_list[citizen_no].y)
        citizen_list[citizen_no].dest_y =
            citizen_list[citizen_no].y + rand8 - 6;
}

// Choose a neighbouring road, preferring an unoccupied direction that does not reverse course.
// FUNCTION: C2 0x49184
// FUNCTION: C2WIN 0x0040b607
int city_test_for_road(int cell_x, int cell_y, int cell_offset, signed char heading)
{
    unsigned char road_slots[8][3];
    signed char reverse_dir;
    signed char candidate_dir;
    int i;
    int road_count;
    int empty_road_count;

    for (i = 0; i < 8; i += 2) road_slots[i][0] = road_slots[i][1] = road_slots[i][2] = 0;
    reverse_dir = ((char)heading + 4) & 7;

    if (cell_y > 0) {
        if ((*(struct city_cell *)((unsigned char *)city_map + ((cell_offset - 1600)))).terrain & 0x20) {
            road_slots[0][0] = 1;
            road_slots[0][1] = (*(struct city_cell *)((unsigned char *)city_map + ((cell_offset - 1600)))).citizen_a;
            road_slots[0][2] = (*(struct city_cell *)((unsigned char *)city_map + ((cell_offset - 1600)))).citizen_b;
        }
    }
    if (cell_x < 0x4f) {
        if ((*(struct city_cell *)((unsigned char *)city_map + ((cell_offset + 20)))).terrain & 0x20) {
            road_slots[2][0] = 1;
            road_slots[2][1] = (*(struct city_cell *)((unsigned char *)city_map + ((cell_offset + 20)))).citizen_a;
            road_slots[2][2] = (*(struct city_cell *)((unsigned char *)city_map + ((cell_offset + 20)))).citizen_b;
        }
    }
    if (cell_y < 0x4f) {
        if ((*(struct city_cell *)((unsigned char *)city_map + ((cell_offset + 1600)))).terrain & 0x20) {
            road_slots[4][0] = 1;
            road_slots[4][1] = (*(struct city_cell *)((unsigned char *)city_map + ((cell_offset + 1600)))).citizen_a;
            road_slots[4][2] = (*(struct city_cell *)((unsigned char *)city_map + ((cell_offset + 1600)))).citizen_a;
        }
    }
    if (cell_x > 0) {
        if ((*(struct city_cell *)((unsigned char *)city_map + ((cell_offset - 20)))).terrain & 0x20) {
            road_slots[6][0] = 1;
            road_slots[6][1] = (*(struct city_cell *)((unsigned char *)city_map + ((cell_offset - 20)))).citizen_a;
            road_slots[6][2] = (*(struct city_cell *)((unsigned char *)city_map + ((cell_offset - 20)))).citizen_b;
        }
    }

    road_count = 0; empty_road_count = 0;
    for (i = 0; i < 8; i += 2) {
        if (road_slots[i][0] == 0) continue;
        road_count++; if (road_slots[i][1] != 0) continue; if (road_slots[i][2] != 0) continue; empty_road_count++;
    }

    candidate_dir = (unsigned char)rand8 & 6;
    if (road_count != 0) {
        if (road_count == 1) {
            for (i = 0; i < 8; i += 2) if (road_slots[i][0] != 0) return i;
        }
        if (empty_road_count != 0) {
            for (i = 0; i < 4; i++) {
                if (road_slots[candidate_dir][0] != 0) {
                    if (road_slots[candidate_dir][1] == 0 && road_slots[candidate_dir][2] == 0) {
                        if (candidate_dir != reverse_dir) return candidate_dir;
                    }
                }
                candidate_dir += 2;
                if (candidate_dir > 6) candidate_dir = 0;
            }
        }
        for (i = 0; i < 4; i++) {
            if (road_slots[candidate_dir][0] != 0) {
                if (candidate_dir != reverse_dir) return candidate_dir;
            }
            candidate_dir += 2;
            if (candidate_dir > 6) candidate_dir = 0;
        }
    }
    return 8;
}

// Set citizen.dest_x / dest_y to the city-map tile adjacent to the current citizen in the given
// direction.
// FUNCTION: C2 0x4933e
// FUNCTION: C2WIN 0x0040b930
void target_from_dirc(int direction)
{
    if (direction == 0) {
        citizen_list[citizen_no].dest_x = citizen_list[citizen_no].x;
        citizen_list[citizen_no].dest_y = citizen_list[citizen_no].y - 1;
    } else if (direction == 2) {
        citizen_list[citizen_no].dest_x = citizen_list[citizen_no].x + 1;
        citizen_list[citizen_no].dest_y = citizen_list[citizen_no].y;
    } else if (direction == 4) {
        citizen_list[citizen_no].dest_x = citizen_list[citizen_no].x;
        citizen_list[citizen_no].dest_y = citizen_list[citizen_no].y + 1;
    } else if (direction == 6) {
        citizen_list[citizen_no].dest_x = citizen_list[citizen_no].x - 1;
        citizen_list[citizen_no].dest_y = citizen_list[citizen_no].y;
    } else if (direction == 1) {
        citizen_list[citizen_no].dest_x = citizen_list[citizen_no].x + 1;
        citizen_list[citizen_no].dest_y = citizen_list[citizen_no].y - 1;
    } else if (direction == 3) {
        citizen_list[citizen_no].dest_x = citizen_list[citizen_no].x + 1;
        citizen_list[citizen_no].dest_y = citizen_list[citizen_no].y + 1;
    } else if (direction == 5) {
        citizen_list[citizen_no].dest_x = citizen_list[citizen_no].x - 1;
        citizen_list[citizen_no].dest_y = citizen_list[citizen_no].y + 1;
    } else if (direction == 7) {
        citizen_list[citizen_no].dest_x = citizen_list[citizen_no].x - 1;
        citizen_list[citizen_no].dest_y = citizen_list[citizen_no].y - 1;
    }
}

// Test whether the current army has just entered a new square that should trigger a
// path-find/walk-state event.
// FUNCTION: C2 0x4944a
// FUNCTION: C2WIN 0x0040bcc6
int entering_new_square(void)
{
    int step_threshold;
    if ((army_list[army_no].flags & 1) != 0) return 1;
    if (army_list[army_no].target_flag) step_threshold = 1;
    else step_threshold = 2;
    if (army_list[army_no].target_count >= step_threshold
     && army_list[army_no].target_kind >= 15)
        return 1;
    return 0;
}


// Advance the current army toward its target, calculating a regional route when blocked.
// FUNCTION: C2 0x494ac
// FUNCTION: C2WIN 0x0040bd88
int region_go_to_target(int unused_kind)
{
    int step_limit;
    int cell_result;
    signed char ready_flag;
    (void)unused_kind;

    ready_flag = army_list[army_no].flags & 1;
    army_list[army_no].flags &= 0xf7;
    step_limit = ready_flag;
    if (step_limit != 0) {
        army_list[army_no].target_kind = 0;
        army_list[army_no].target_count = 0;
    } else {
        if (army_list[army_no].target_flag == 0) {
            if (army_list[army_no].type == 1)
                step_limit = 2;
            else
                step_limit = 3;
        }
        army_list[army_no].target_count++;
        if (army_list[army_no].target_count > step_limit) {
            army_list[army_no].target_count = 0;
            army_list[army_no].target_kind++;
            if (army_list[army_no].target_kind > 0xf) {
                army_list[army_no].flags |= 1;
                army_list[army_no].target_kind = 0;
                goto branch_b;
            }
        }
        return 0;
    branch_b: ;
    }

    if (army_list[army_no].return_flag == 0) {
        return 1;
    }

    w_dirc = (char)get_heading(
        army_list[army_no].x,
        army_list[army_no].y,
        army_list[army_no].target_x,
        army_list[army_no].target_y,
        army_list[army_no].world_dir);

    if (w_dirc >= 8) {
        army_list[army_no].return_flag = 0;
        army_list[army_no].flags |= 2;
        return 1;
    }

    if (army_list[army_no].wf_active) {
        get_dirc_from_army_wf_run();
    }

    cell_result = try_a_regionmap_square(w_dirc, 0, 0);

    if (cell_result == 0x3e7) {
        if (army_list[army_no].state_idx == 2) return 0;
        army_list[army_no].saved_state_idx =
            army_list[army_no].state_idx;
        army_list[army_no].state_idx = 1;
        army_list[army_no].wait_count = 5;
        army_list[army_no].world_dir =
            (army_list[army_no].world_dir + 1) & 7;
        return 0;
    }

    if (cell_result == 0) {
        army_list[army_no].wf_active = 0;
        if (army_list[army_no].state_idx == 8) return 1;

        clear_region_ferret_map(0, 0x3c,
                                (unsigned char *)region_map,
                                0x3c, 0x3c, 8,
                                army_list[army_no].x,
                                army_list[army_no].y,
                                army_list[army_no].target_x,
                                army_list[army_no].target_y);
        if (run_2_map_ferrets(0x3c,
                              (unsigned char *)region_map,
                              0x3c, 0x3c, 8,
                              army_list[army_no].x,
                              army_list[army_no].y,
                              army_list[army_no].target_x,
                              army_list[army_no].target_y)
            == 0) {
            if (army_list[army_no].type == 1) {
                army_list[army_no].saved_state_idx =
                    army_list[army_no].state_idx;
                army_list[army_no].state_idx = 8;
                army_list[army_no].stuck_timer = 0;
                return 0;
            }
            if (army_list[army_no].flags & 8) {
                army_list[army_no].state_idx = 9;
                army_list[army_no].stuck_timer = 0;
                army_list[army_no].wf_phase = 0;
                put_message(0x5f, army_list[army_no].map_ref, 0x12);
                return 0;
            }
            clear_region_ferret_map(1, 0x3c,
                                    (unsigned char *)region_map,
                                    0x3c, 0x3c, 8,
                                    army_list[army_no].x,
                                    army_list[army_no].y,
                                    army_list[army_no].target_x,
                                    army_list[army_no].target_y);
            if (run_2_map_ferrets(0x3c,
                                  (unsigned char *)region_map,
                                  0x3c, 0x3c, 8,
                                  army_list[army_no].x,
                                  army_list[army_no].y,
                                  army_list[army_no].target_x,
                                  army_list[army_no].target_y)
                == 0) {
                army_list[army_no].state_idx = 2;
                return 0;
            }
            copy_ferret_run_to_army();
            return 0;
        }
        copy_ferret_run_to_army();
        return 0;
    }

    if (cell_result == 1)
        army_list[army_no].target_flag = 1;
    else
        army_list[army_no].target_flag = 0;

    army_list[army_no].flags &= 0xfe;
    army_list[army_no].world_dir = w_dirc;
    army_list[army_no].target_kind = 1;
    move_army();
    return 1;
}

// Advance the current ship toward its target, calculating a sea route when blocked.
// FUNCTION: C2 0x4987b
// FUNCTION: C2WIN 0x0040c603
int sail_to_target(int unused_kind)
{
    int cell_result;
    (void)unused_kind;

    army_list[army_no].flags &= 0xf3;

    if ((army_list[army_no].flags & 1) != 0) {
        army_list[army_no].target_kind = 0;
        army_list[army_no].target_count = 0;
    } else {
        army_list[army_no].target_count++;
        if (army_list[army_no].target_count > 2) {
            army_list[army_no].target_count = 0;
            army_list[army_no].target_kind++;
            if (army_list[army_no].target_kind <= 0xf) {
                return 0;
            }
            army_list[army_no].flags |= 1;
            army_list[army_no].target_kind = 0;
        } else {
            return 0;
        }
    }

    if (army_list[army_no].return_flag == 0) {
        return 1;
    }

    w_dirc = (char)get_heading(
        army_list[army_no].x,
        army_list[army_no].y,
        army_list[army_no].target_x,
        army_list[army_no].target_y,
        army_list[army_no].world_dir);

    if (w_dirc >= 8) {
        army_list[army_no].return_flag = 0;
        army_list[army_no].flags |= 2;
        return 1;
    }

    if (army_list[army_no].wf_active) {
        get_dirc_from_army_wf_run();
    }

    cell_result = try_a_seamap_square(w_dirc, 0, 0);

    if (cell_result == 0x3e7) {
        army_list[army_no].flags |= 4;
    }

    if (cell_result != 1) {
        army_list[army_no].wf_active = 0;
        clear_sea_ferret_map(0, 0x3c,
                             (unsigned char *)region_map,
                             0x3c, 0x3c, 8,
                             army_list[army_no].x,
                             army_list[army_no].y,
                             army_list[army_no].target_x,
                             army_list[army_no].target_y);
        if (run_2_map_ferrets(0x3c,
                              (unsigned char *)region_map,
                              0x3c, 0x3c, 8,
                              army_list[army_no].x,
                              army_list[army_no].y,
                              army_list[army_no].target_x,
                              army_list[army_no].target_y)
            == 0) {
            return 1;
        }
        copy_ferret_run_to_army();
        return 0;
    }

    army_list[army_no].target_flag = 0;
    army_list[army_no].flags &= 0xfe;
    army_list[army_no].world_dir = w_dirc;
    army_list[army_no].target_kind = 1;
    move_army();
    return 1;
}

// Pull the next heading from the current army's nibble-packed walking route.
// FUNCTION: C2 0x49a96
// FUNCTION: C2WIN 0x0040cb2d
void get_dirc_from_army_wf_run(void)
{
    if (army_list[army_no].wf_step
        >= army_list[army_no].wf_length) {
        army_list[army_no].wf_active = 0;
        return;
    }
    w_dirc = army_list[army_no].wf_steps[
        army_list[army_no].wf_step >> 1];
    if ((army_list[army_no].wf_step & 1) != 0)
        w_dirc >>= 4;
    else
        w_dirc &= 0xf;
    army_list[army_no].wf_step++;
}

// Pack the calculated route into the current army's nibble-encoded walking steps and activate it.
// FUNCTION: C2 0x49b10
// FUNCTION: C2WIN 0x0040cc1f
void copy_ferret_run_to_army(void)
{
    int i, j;
    char step;

    army_list[army_no].wf_active = 1; army_list[army_no].wf_step = 0; army_list[army_no].wf_length = ferret_run_length;
    w_dirc = ferret_run[0];
    i = 0; j = 0;
    for (; i < ferret_run_length; i++) {
        step = ferret_run[i];
        if ((i & 1) == 0) army_list[army_no].wf_steps[j] = step;
        else { step <<= 4; army_list[army_no].wf_steps[j] += step; j++; }
    }
}

// Test the neighbouring region-map cell in compass direction `dir` for the current army.
// FUNCTION: C2 0x49bb1
// FUNCTION: C2WIN 0x0040cd36
int try_a_regionmap_square(int direction, int movement_kind, int unused_arg)
{
    int result;

    result = 0;
    enemy_figure = 0;
    switch ((unsigned int)direction) {
    case 0:                                /* N */
        if (army_list[army_no].y <= 0)
            result = 0;
        else
            result = try_this_regionmap_square(army_list[army_no].map_ref - 0x1e0, movement_kind, unused_arg);
        break;
    case 1:                                /* NE */
        if (army_list[army_no].x < 0x3b) {
            if (army_list[army_no].y <= 0)
                result = 0;
            else
                result = try_this_regionmap_square(army_list[army_no].map_ref - 0x1d8, movement_kind, unused_arg);
        } else {
            result = 0;
        }
        break;
    case 2:                                /* E */
        if (army_list[army_no].x < 0x3b)
            result = try_this_regionmap_square(army_list[army_no].map_ref + 8, movement_kind, unused_arg);
        else
            result = 0;
        break;
    case 3:                                /* SE */
        if (army_list[army_no].x < 0x3b) {
            if (army_list[army_no].y < 0x3b)
                result = try_this_regionmap_square(army_list[army_no].map_ref + 0x1e8, movement_kind, unused_arg);
            else
                result = 0;
        } else {
            result = 0;
        }
        break;
    case 4:                                /* S */
        if (army_list[army_no].y < 0x3b)
            result = try_this_regionmap_square(army_list[army_no].map_ref + 0x1e0, movement_kind, unused_arg);
        else
            result = 0;
        break;
    case 5:                                /* SW */
        if (army_list[army_no].x <= 0)
            result = 0;
        else if (army_list[army_no].y < 0x3b)
            result = try_this_regionmap_square(army_list[army_no].map_ref + 0x1d8, movement_kind, unused_arg);
        else
            result = 0;
        break;
    case 6:                                /* W */
        if (army_list[army_no].x <= 0)
            result = 0;
        else
            result = try_this_regionmap_square(army_list[army_no].map_ref - 8, movement_kind, unused_arg);
        break;
    case 7:                                /* NW */
        if (army_list[army_no].x <= 0)
            result = 0;
        else if (army_list[army_no].y <= 0)
            result = 0;
        else
            result = try_this_regionmap_square(army_list[army_no].map_ref - 0x1e8, movement_kind, unused_arg);
        break;
    }
    return result;
}

// Test whether the current army can enter a region-map cell and resolve battles or destruction.
// FUNCTION: C2 0x49d62
// FUNCTION: C2WIN 0x0040d132
int try_this_regionmap_square(int target_offset, int unused_kind, int unused_arg)
{
    unsigned char terr_bit_1;
    unsigned char terrain;
    unsigned char terr_bit_4;
    unsigned char terr_bit_2;
    unsigned char base_kind;
    int army_type;
    unsigned char terr_bit_20;
    int i;

    (void)unused_kind; (void)unused_arg;

    army_a      = (short)(*(struct region_cell *)((unsigned char *)region_map + (target_offset))).occupant;
    terrain     = (*(struct region_cell *)((unsigned char *)region_map + (target_offset))).terrain;
    army_type        = army_list[army_no].type;

    terr_bit_2  = terrain & 2;
    terr_bit_20 = terrain & 0x20;
    terr_bit_4  = terrain & 4;
    terr_bit_1  = terrain & 1;

    if (army_type == 1) {
        if ((terrain & 0x10) != 0) {
            if (army_a == 0) goto ret0;
            if (army_list[army_a].state_idx == 2) goto ret0;
            if (army_list[army_a].type != army_type) { get_contenders(); game_state = 4; battle_type = army_type; }
            goto ret999;
        }
        if ((int)terr_bit_1) {
            base_kind = (*(struct region_cell *)((unsigned char *)region_map + (target_offset))).base_kind;
            if (base_kind >= 0x93 && base_kind <= 0x96) {
                confirm(9, 0xa0, 0xa0);
                if (decision == 1) {
                    battle_type   = 2;
                    battle2_ptr   = target_offset;
                    get_villagers(base_kind - 0x92);
                    game_state    = 4;
                    army_list[army_no].target_x = army_list[army_no].x;
                    army_list[army_no].target_y = army_list[army_no].y;
                }
                else {
                    army_list[army_no].dest_y = 0;
                    army_list[army_no].dest_x = 0;
                    for (i = 0; i < 10; i++) {
                        army_routes[army_list[army_no].cohort_id].row_len[i] = 0;
                    }
                    army_routes[army_list[army_no].cohort_id].row_count = 0;
                    army_routes[army_list[army_no].cohort_id].chase_row = 0;
                    army_routes[army_list[army_no].cohort_id].target_army = 0;
                    army_list[army_no].target_x = army_list[army_no].x;
                    army_list[army_no].target_y = army_list[army_no].y;
                    army_list[army_no].order_progress = 0;
                }
            }
ret0:
            return 0;
        }
        if ((int)terr_bit_4) return 2;
        if (army_a == 0) {
            if ((int)terr_bit_20) return 1;
            if ((int)terr_bit_2) return 0;
            return 2;
        }
        if (army_list[army_a].type != 1) {
            if (army_list[army_a].state_idx == 2) return 0;
            get_contenders(); game_state = 4; battle_type = 1;
        }
ret999:
        return 0x3e7;
    }

    if (army_type < 2 || army_type > 5) return 0;

    if ((terrain & 0x10) != 0) return 0;
    if ((int)terr_bit_2) {
        army_list[army_no].flags |= 8;
        return 0;
    }
    if ((int)terr_bit_1) {
        base_kind = (*(struct region_cell *)((unsigned char *)region_map + (target_offset))).base_kind;
        if (base_kind >= 0x93 && base_kind <= 0x96) return 0;
        if (base_kind == 0x92) {
            barbarian_invades_city(army_no);
            army_list[army_no].state_idx = 2;
            return 0;
        }
        if (base_kind == 0x97) {
            (*(struct region_cell *)((unsigned char *)region_map + (target_offset))).base_kind = 0x93;
            (*(struct region_cell *)((unsigned char *)region_map + (target_offset))).gfx = 0x2e;
            army_list[army_no].state_idx = 2;
            put_message(0x71, target_offset, 0x13);
            pax_romanum -= 0xc;
            if (pax_romanum < 0) pax_romanum = 0;
            return 0;
        }
        destroy_reg_atom(target_offset);
        army_list[army_no].target_x = army_list[army_no].x;
        army_list[army_no].target_y = army_list[army_no].y;
        pax_romanum -= 6;
        if (pax_romanum < 0) pax_romanum = 0;
        put_message(0x72, target_offset, 0x13);
        return 0;
    }
    if ((int)terr_bit_4) return 0;
    if (army_a == 0) {
        destroy_reg_atom(target_offset);
        if ((int)terr_bit_20) return 1;
        return 2;
    }
    if (army_list[army_a].type == 1) {
        if (army_list[army_a].state_idx == 2) return 0;
        get_contenders();
        game_state  = 4;
        battle_type = 1;
    }
    goto ret999;
}

// Test the neighbouring sea-map cell in compass direction `dir` for the current ship.
// FUNCTION: C2 0x4a1b5
// FUNCTION: C2WIN 0x0040d869
int try_a_seamap_square(int direction, int movement_kind, int unused_arg)
{
    int result;

    result = 0;
    enemy_figure = 0;
    switch (direction) {
    case 0:                                /* N */
        if (army_list[army_no].y <= 0)
            result = 2;
        else
            result = try_this_seamap_square(army_list[army_no].map_ref - 0x1e0, movement_kind, unused_arg);
        break;
    case 1:                                /* NE */
        if (army_list[army_no].x < 0x3b) {
            if (army_list[army_no].y <= 0)
                result = 2;
            else
                result = try_this_seamap_square(army_list[army_no].map_ref - 0x1d8, movement_kind, unused_arg);
        } else {
            result = 2;
        }
        break;
    case 2:                                /* E */
        if (army_list[army_no].x < 0x3b)
            result = try_this_seamap_square(army_list[army_no].map_ref + 8, movement_kind, unused_arg);
        else
            result = 2;
        break;
    case 3:                                /* SE */
        if (army_list[army_no].x < 0x3b) {
            if (army_list[army_no].y < 0x3b)
                result = try_this_seamap_square(army_list[army_no].map_ref + 0x1e8, movement_kind, unused_arg);
            else
                result = 2;
        } else {
            result = 2;
        }
        break;
    case 4:                                /* S */
        if (army_list[army_no].y < 0x3b)
            result = try_this_seamap_square(army_list[army_no].map_ref + 0x1e0, movement_kind, unused_arg);
        else
            result = 2;
        break;
    case 5:                                /* SW */
        if (army_list[army_no].x <= 0)
            result = 2;
        else if (army_list[army_no].y < 0x3b)
            result = try_this_seamap_square(army_list[army_no].map_ref + 0x1d8, movement_kind, unused_arg);
        else
            result = 2;
        break;
    case 6:                                /* W */
        if (army_list[army_no].x <= 0)
            result = 2;
        else
            result = try_this_seamap_square(army_list[army_no].map_ref - 8, movement_kind, unused_arg);
        break;
    case 7:                                /* NW */
        if (army_list[army_no].x <= 0)
            result = 2;
        else if (army_list[army_no].y <= 0)
            result = 2;
        else
            result = try_this_seamap_square(army_list[army_no].map_ref - 0x1e8, movement_kind, unused_arg);
        break;
    }
    return result;
}

// Test a sea-route cell, handling coast destruction, landing, and blocked-water flags.
// FUNCTION: C2 0x4a369
// FUNCTION: C2WIN 0x0040dc89
int try_this_seamap_square(int cell_offset, int unused_kind, int unused_arg)
{
    char tile_flags;
    (void)unused_kind; (void)unused_arg;

    army_a = (*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).occupant;
    tile_flags = (*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).terrain;
    if ((tile_flags & 8) != 0) {
        if ((tile_flags & 0x10) != 0)
            return 1;
        if ((tile_flags & 1) != 0) {
            if (army_list[army_no].type < 6) {
                destroy_reg_atom(cell_offset);
                return 0;
            }
            army_list[army_no].flags |= 8;
            return 3;
        }
    }
    army_list[army_no].landed_flag = tile_flags & 0x17;
    army_list[army_no].flags |= 8;
    return 0;
}

// Dock a trader ship at a port-adjacent tile and deliver its cargo to nearby warehouses.
// FUNCTION: C2 0x4a3f5
// FUNCTION: C2WIN 0x0040ddb0
int dock_the_ship_in_good_port(int heading)
{
    unsigned char occupant_flags;
    unsigned char corner_flags;
    unsigned char was_sea_flag;
    unsigned char edge_flag;
    int cell_x;
    int cell_y;
    int cell_offset;
    int cell_idx;
    int goods_kind;

    if (heading == 0)      cell_offset = army_list[army_no].map_ref - 0x1e0;
    else if (heading == 1) cell_offset = army_list[army_no].map_ref - 0x1d8;
    else if (heading == 2) cell_offset = army_list[army_no].map_ref + 8;
    else if (heading == 3) cell_offset = army_list[army_no].map_ref + 0x1e8;
    else if (heading == 4) cell_offset = army_list[army_no].map_ref + 0x1e0;
    else if (heading == 5) cell_offset = army_list[army_no].map_ref + 0x1d8;
    else if (heading == 6) cell_offset = army_list[army_no].map_ref - 8;
    else if (heading == 7) cell_offset = army_list[army_no].map_ref - 0x1e8;

    occupant_flags = (*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).occupant;
    corner_flags = occupant_flags & 3;
    if (corner_flags & 1) cell_offset -= 8;
    if (corner_flags & 2) cell_offset -= 0x1e0;

    cell_idx = cell_offset / 8;
    cell_x = cell_idx % 60;
    cell_y = cell_idx / 60;

    occupant_flags = (*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).occupant;
    edge_flag = (*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).edge_bits & 0x20;
    was_sea_flag = occupant_flags & 0x80;

    (*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).occupant |= 0x1c;
    (*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).occupant &= 0x9f;

    if (army_list[army_no].compass_side != 0) {
        if (army_list[army_no].compass_side == 2)      (*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).occupant |= 0x20;
        else if (army_list[army_no].compass_side == 4) (*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).occupant |= 0x40;
        else                                           (*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).occupant |= 0x60;
    }

    goods_kind = army_list[army_no].trader_brings;
    if (edge_flag != 0) {
        fill_warehouses_with(cell_x, cell_y, 0xf, goods_kind, 1);
    }

    return was_sea_flag != 0;
}

// Step the current army one tile in its `world_dir` heading on the region map.
// FUNCTION: C2 0x4a621
// FUNCTION: C2WIN 0x0040e131
void move_army(void)
{
    if ((*(struct region_cell *)((unsigned char *)region_map + (army_list[army_no].map_ref))).occupant == army_no)
        (*(struct region_cell *)((unsigned char *)region_map + (army_list[army_no].map_ref))).occupant = 0;
    army_list[army_no].home_ref = army_list[army_no].map_ref;

    switch ((unsigned char)army_list[army_no].world_dir) {
    default: return;
    case 0:
        army_list[army_no].y--;
        army_list[army_no].map_ref -= 0x1e0;
        break;
    case 1:
        army_list[army_no].y--;
        army_list[army_no].x++;
        army_list[army_no].map_ref -= 0x1d8;
        break;
    case 2:
        army_list[army_no].x++;
        army_list[army_no].map_ref += 8;
        break;
    case 3:
        army_list[army_no].y++;
        army_list[army_no].x++;
        army_list[army_no].map_ref += 0x1e8;
        break;
    case 4:
        army_list[army_no].y++;
        army_list[army_no].map_ref += 0x1e0;
        break;
    case 5:
        army_list[army_no].y++;
        army_list[army_no].x--;
        army_list[army_no].map_ref += 0x1d8;
        break;
    case 6:
        army_list[army_no].x--;
        army_list[army_no].map_ref -= 8;
        break;
    case 7:
        army_list[army_no].y--;
        army_list[army_no].x--;
        army_list[army_no].map_ref -= 0x1e8;
        break;
    }
    if ((*(struct region_cell *)((unsigned char *)region_map + (army_list[army_no].map_ref))).occupant == 0)
        (*(struct region_cell *)((unsigned char *)region_map + (army_list[army_no].map_ref))).occupant = army_no;
}

// Set the current army's target to the adjacent region-map cell in direction `dir`.
// FUNCTION: C2 0x4a759
// FUNCTION: C2WIN 0x0040e4f2
void target_from_army_dirc(int direction)
{
    if (direction == 0) {
        army_list[army_no].target_x = army_list[army_no].x;
        army_list[army_no].target_y = army_list[army_no].y - 1;
    } else if (direction == 2) {
        army_list[army_no].target_x = army_list[army_no].x + 1;
        army_list[army_no].target_y = army_list[army_no].y;
    } else if (direction == 4) {
        army_list[army_no].target_x = army_list[army_no].x;
        army_list[army_no].target_y = army_list[army_no].y + 1;
    } else if (direction == 6) {
        army_list[army_no].target_x = army_list[army_no].x - 1;
        army_list[army_no].target_y = army_list[army_no].y;
    } else if (direction == 1) {
        army_list[army_no].target_x = army_list[army_no].x + 1;
        army_list[army_no].target_y = army_list[army_no].y - 1;
    } else if (direction == 3) {
        army_list[army_no].target_x = army_list[army_no].x + 1;
        army_list[army_no].target_y = army_list[army_no].y + 1;
    } else if (direction == 5) {
        army_list[army_no].target_x = army_list[army_no].x - 1;
        army_list[army_no].target_y = army_list[army_no].y + 1;
    } else if (direction == 7) {
        army_list[army_no].target_x = army_list[army_no].x - 1;
        army_list[army_no].target_y = army_list[army_no].y - 1;
    }
}

// Find a fire-zone bit set in the 3×3 neighbourhood of the current citizen's zone (zone = 8×8
// cells). Sets the global (zone_x, zone_y) to the chosen zone and returns 1, or returns 0 if no
// neighbour zone has a fire.
// FUNCTION: C2 0x4a880
// FUNCTION: C2WIN 0x0040e888
int test_fire_zones(void)
{
    zone_x = citizen_list[citizen_no].x / 8;
    zone_y = citizen_list[citizen_no].y / 8;

    if (fire_zones[zone_x + zone_y * 10] != 0) {
        return 1;
    }

    if (zone_y > 0) {
        if (fire_zones[zone_x + (zone_y - 1) * 10] != 0) {
            zone_y = zone_y - 1;
            return 1;
        }
        if (zone_x > 0) {
            if (fire_zones[(zone_x - 1) + (zone_y - 1) * 10] != 0) {
                zone_x = zone_x - 1;
                zone_y = zone_y - 1;
                return 1;
            }
        }
        if (zone_x < 9) {
            if (fire_zones[(zone_x + 1) + (zone_y - 1) * 10] != 0) {
                zone_x = zone_x + 1;
                zone_y = zone_y - 1;
                return 1;
            }
        }
    }

    if (zone_y < 9) {
        if (fire_zones[zone_x + (zone_y + 1) * 10] != 0) {
            zone_y = zone_y + 1;
            return 1;
        }
        if (zone_x > 0) {
            if (fire_zones[(zone_x - 1) + (zone_y + 1) * 10] != 0) {
                zone_x = zone_x - 1;
                zone_y = zone_y + 1;
                return 1;
            }
        }
        if (zone_x < 9) {
            if (fire_zones[(zone_x + 1) + (zone_y + 1) * 10] != 0) {
                zone_x = zone_x + 1;
                zone_y = zone_y + 1;
                return 1;
            }
        }
    }

    if (zone_x > 0) {
        if (fire_zones[zone_y * 10 + (zone_x - 1)] != 0) {
            zone_x = zone_x - 1;
            return 1;
        }
    }
    if (zone_x < 9) {
        if (fire_zones[zone_y * 10 + (zone_x + 1)] != 0) {
            zone_x = zone_x + 1;
            return 1;
        }
    }

    return 0;
}

// Choose a fire in the selected 8×8 zone, preferring uncovered fires unless a covered one is much
// closer.
// FUNCTION: C2 0x4aa68
// FUNCTION: C2WIN 0x0040eb06
int test_zone_for_closest_fire(void)
{
  int covered_offset;
  int uncovered_x;
  int uncovered_y;
  int cell_offset;
  int min_covered_distance;
  int uncovered_offset;
  int min_uncovered_distance;
  int cell_y;
  int cell_x;
  int covered_x;
  int covered_y;
  int zone_idx;
  cell_offset = (zone_y * ((8 * CITY_W) * CITY_CELL_BYTES));
  cell_offset += (zone_x * (8 * CITY_CELL_BYTES));
  min_uncovered_distance = 100;
  min_covered_distance = 100;
  for (cell_y = zone_y * 8; cell_y < ((zone_y * 8) + 8); cell_y++, cell_offset += 1440)
  {
    for (cell_x = zone_x * 8; cell_x < ((zone_x * 8) + 8); cell_x++, cell_offset += 20)
    {
      unsigned char cell_value;
      cell_value = (*(struct city_cell *)((unsigned char *)city_map + (cell_offset))).base_kind;
      if (cell_value < 8)
      {
        cell_value = (*(struct city_cell *)((unsigned char *)city_map + (cell_offset))).edge_bits;
        if ((cell_value & 0x80) != 0)
        {
          int distance;
          distance = get_distance(cell_x, cell_y, citizen_list[citizen_no].x, citizen_list[citizen_no].y);
          if (distance < min_uncovered_distance)
          {
            if (is_fire_covered(cell_offset) != 0)
            {
              min_covered_distance = distance;
              covered_x = cell_x;
              covered_y = cell_y;
              covered_offset = cell_offset;
              continue;
            }
            min_uncovered_distance = distance;
            uncovered_x = cell_x;
            uncovered_y = cell_y;
            uncovered_offset = cell_offset;
          }
        }
      }
    }
  }

  if ((min_uncovered_distance > 0x28) && (min_covered_distance < 0x24))
  {
    z_x = covered_x;
    z_y = covered_y;
    z_ptr = covered_offset;
    return 1;
  }
  if ((min_uncovered_distance > 0xc) && (min_covered_distance < 6))
  {
    z_x = covered_x;
    z_y = covered_y;
    z_ptr = covered_offset;
    return 1;
  }
  if ((min_uncovered_distance > 8) && (min_covered_distance < 4))
  {
    z_x = covered_x;
    z_y = covered_y;
    z_ptr = covered_offset;
    return 1;
  }
  if (min_uncovered_distance < 100)
  {
    z_x = uncovered_x;
    z_y = uncovered_y;
    z_ptr = uncovered_offset;
    return 1;
  }
  if (min_covered_distance >= 100)
  {
    zone_idx = (zone_y * 10) + zone_x;
    fire_zones[zone_idx] = 0;
  }
  return 0;
}


// Reduce the fire at the current citizen's cell, clearing its active flag when extinguished.
// FUNCTION: C2 0x4abff
// FUNCTION: C2WIN 0x0040ed63
int putting_out_fire(void)
{
    int cell_offset;
    unsigned char cell_value;

    cell_offset = citizen_list[citizen_no].map_ref;
    cell_value = (*(struct city_cell *)((unsigned char *)city_map + ((cell_offset)))).base_kind;
    if (cell_value < 8) {
        cell_value = (*(struct city_cell *)((unsigned char *)city_map + ((cell_offset)))).edge_bits;
        if ((cell_value & 0x80) != 0) {
            cell_value = (*(struct city_cell *)((unsigned char *)city_map + ((cell_offset)))).fire;
            if (--cell_value != 0)
                (*(struct city_cell *)((unsigned char *)city_map + ((cell_offset)))).fire--;
            else
                (*(struct city_cell *)((unsigned char *)city_map + ((cell_offset)))).edge_bits &= 0x7f;
            return 1;
        }
    }
    return 0;
}

// Re-validate the current citizen's fire target.
// FUNCTION: C2 0x4ac56
// FUNCTION: C2WIN 0x0040ee6f
int confirm_fire_target(void)
{
    int cell_offset;

    if (citizen_list[citizen_no].target_kind == 0) return 0;
    cell_offset = citizen_list[citizen_no].target_ref;
    if ((*(struct city_cell *)((unsigned char *)city_map + ((cell_offset)))).base_kind >= 8)
        return 0;
    if (((*(struct city_cell *)((unsigned char *)city_map + ((cell_offset)))).edge_bits & 0x80) == 0)
        return 0;
    return 1;
}

// Walk citizens 1..200 and return 1 if any active citizen is in the fire-fight state (state_idx ==
// 9 == s09_fire_fight) with target_ref matching the supplied cm_ptr. Otherwise return 0.
// FUNCTION: C2 0x4ac97
// FUNCTION: C2WIN 0x0040ef01
int is_fire_covered(int cell_offset)
{
    for (temp_citizen = 1; temp_citizen < 0xc9; temp_citizen++) {
        if (citizen_list[temp_citizen].exists != 0
         && citizen_list[temp_citizen].state_idx == 9
         && citizen_list[temp_citizen].target_ref == cell_offset) {
            return 1;
        }
    }
    return 0;
}

// Survey the square neighbourhood around a citizen and update its industry and population demand.
// FUNCTION: C2 0x4ace8
// FUNCTION: C2WIN 0x0040efad
void get_population_and_industry_count(int radius, int demand_mode)
{
    int y_start;
    int x_start;
    int diameter;
    int width;
    int height;
    int row_inc;
    unsigned char count_pop;
    unsigned char count_industry;
    unsigned char count_other;
    unsigned char base_kind;
    unsigned char education_flags;

    x_start = citizen_list[citizen_no].x - radius;
    y_start = citizen_list[citizen_no].y - radius;
    diameter = 2 * radius;
    height = diameter + 1;
    width  = diameter + 1;

    if (x_start < 0) {
        width += x_start;
        x_start = 0;
    } else if (x_start + width > 0x50) {
        width -= (x_start + width) - 0x50;
    }
    if (y_start < 0) {
        height += y_start;
        y_start = 0;
    } else if (y_start + height > 0x50) {
        height -= (y_start + height) - 0x50;
    }

    gmn_sptr = 20 * (x_start + y_start * 0x50);
    row_inc  = 20 * (0x50 - width);

    count_other    = 0;
    count_pop      = 0;
    count_industry = 0;

    for (gmn_y = y_start; gmn_y < y_start + height; gmn_y++, gmn_sptr += row_inc) {
        for (gmn_x = x_start; gmn_x < x_start + width; gmn_x++, gmn_sptr += 20) {
            base_kind = ((struct city_cell *)((unsigned char *)city_map + gmn_sptr))->base_kind;
            education_flags  = ((struct city_cell *)((unsigned char *)city_map + gmn_sptr))->education;
            if (base_kind >= 0x82 && base_kind <= 0xa1) count_industry++;
            if (education_flags & 0x80) count_pop   += 2;
            if (education_flags & 0x40) count_other += 3;
        }
    }

    /* Saturating bump for market_demand_a. */
    citizen_list[citizen_no].market_demand_a += count_industry;
    if (citizen_list[citizen_no].market_demand_a > 4) {
        citizen_list[citizen_no].market_demand_a -= 2;
    } else if (citizen_list[citizen_no].market_demand_a > 0) {
        citizen_list[citizen_no].market_demand_a--;
    }
    if (citizen_list[citizen_no].market_demand_a > 0x64) {
        citizen_list[citizen_no].market_demand_a = 0x64;
    }

    /* Bump for market_demand_b. */
    if (demand_mode != 0) {
        citizen_list[citizen_no].market_demand_b += count_pop;
    } else {
        citizen_list[citizen_no].market_demand_b += count_other;
    }
    if (citizen_list[citizen_no].market_demand_b > 0) {
        citizen_list[citizen_no].market_demand_b--;
    }
    if (citizen_list[citizen_no].market_demand_b > 0x64) {
        citizen_list[citizen_no].market_demand_b = 0x64;
    }
}

// Find the nearest unoccupied regional building that the current raiding army can target.
// FUNCTION: C2 0x4aedb
// FUNCTION: C2WIN 0x0040f3c1
int get_nearest_reg_building(void)
{
    int min_dist;
    int best_x;
    int best_y;
    int building_kind;
    int distance;
    unsigned char base_kind;
    unsigned char terrain_flags;
    unsigned char occupant_flags;

    min_dist = 0x3e8; best_x = best_y = 0;
    gmn_sptr = gmn_y = 0;
    for ( ; gmn_y < 0x3c; gmn_y++) {
    for (gmn_x = 0; gmn_x < 0x3c; gmn_x++, gmn_sptr += 8) {
    base_kind = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).base_kind;
    terrain_flags  = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain;
    occupant_flags  = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).occupant & 3;
    if ((terrain_flags & 1) != 0) {
        if (occupant_flags == 0) {
            if ((building_kind = base_kind) >= 0x97) {
                if (building_kind < 0x98 || building_kind > 0xd2) {
                    distance = get_longest_distance(army_list[army_no].x, army_list[army_no].y, gmn_x, gmn_y);
                    if (distance < min_dist) { min_dist = distance; best_x = gmn_x; best_y = gmn_y; }
                }
            }
        }
    }
    }
    }
    if (min_dist < 0x3e8) { gmn_x = best_x; gmn_y = best_y; return 1; }
    return 0;
}

void (*citizen_intelligences[8])(void) = {
    i00_null,
    i01_tax_man,
    i02_market_man,
    i03_barbarian_man,
    i04_centurian_man,
    i05_vigile_man,
    i06_business_man,
    i07_rioter_man
};

void (*army_intelligences[9])(void) = {
    a00_null,
    a01_cohort,
    a02_enemy,
    a04_raider,
    a04_raider,
    a05_revolt,
    a06_roman_ship,
    a08_raider_ship,
    a08_raider_ship
};

void (*citizen_states[13])(void) = {
    s00_null,
    s01_wait,
    s02_death,
    s03_map_admin,
    s04_map_markets,
    s05_maraude_to_top_spot,
    s06_quell_trouble,
    s07_army_patrol,
    s08_vigile_patrol,
    s09_fire_fight,
    s10_get_business,
    s11_riot,
    s12_goto_riot
};

void (*army_states[17])(void) = {
    sa00_null,
    sa01_wait,
    sa02_death,
    sa03_army_move,
    sa04_army_attack,
    sa05_army_return,
    sa06_army_land_raid,
    sa07_army_land_invade,
    sa08_army_stuck,
    sa09_army_siege,
    sa10_army_demobed,
    sa11_army_sail_to_port,
    sa12_army_sail_home,
    sa13_army_sail_round_coast,
    sa14_army_sail_to_shore,
    sa15_sink,
    sa16_army_lurk_round_coast
};
