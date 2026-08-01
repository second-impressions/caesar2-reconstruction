
#include "common.h"
#include "c2_data.h"
#include "c2_types.h"

/* File-local state. */
int ferret_targ_x;
int ferret_targ_y;
int anti_ferret_moves;
int last_anti_ferret_dirc;
int tb_ptr;
int anti_ferret_running;
int clock_ferret_moves;
int clock_ferret_ptr;
int tb_x;
int tb_y;
int ferret_map_hi;
int clock_ferret_count;
int clock_ferret_running;
int ferret_map_wi;
unsigned char * ferret_map;
int anti_ferret_count;
int anti_ferret_ptr;
int ferret_vert_off;
int ferret_home;
int clock_ferret_y;
int anti_ferret_x;
int anti_ferret_y;
int last_clock_ferret_dirc;
int ferret_targ_ptr;
int ferret_horiz_off;
int clock_ferret_x;
int ferret_energy;
unsigned char tb_occ_b_flag;
unsigned char tb_road_flag;
unsigned char tb_prev_flag;
unsigned char tb_occ_a_flag;

/* Forward declarations */
char get_heading(int start_x, int start_y, int end_x, int end_y, unsigned char still_offset);
void init_bd(int start_x, int start_y, int end_x, int end_y);
signed char check_clock_ferret_move(signed char direction);
signed char check_anti_ferret_move(signed char direction);
unsigned char ferret_heading(int current_x, int current_y);
unsigned char get_tb_value(int direction);
unsigned char get_ferret2(int direction);
/* Forward declarations (functions defined later in this file). */
void clear_army(struct army_rec *record_ptr);
void load_ferret_run(int start_x, int start_y, int max_length);
void smooth_ferret_run(int margin, unsigned char *map_base, int map_width, int map_height, int cell_size, int start_x, int start_y, int end_x, int end_y);
void clear_ferret_map(int margin, unsigned char *map_base, int map_width, int map_height, int cell_size, int start_x, int start_y, int end_x, int end_y);
void run_clock_ferret(void);
void run_anti_ferret(void);
void move_clock_ferret(signed char direction, char update_existing);
void move_anti_ferret(signed char direction, char update_existing);
void move_to_tb_value(int direction);


// Allocates and initializes a citizen in an available city-map slot.
// FUNCTION: C2 0x2a907
// FUNCTION: C2WIN 0x004691b0
int create_citizen(int type, int x, int y, unsigned char is_barb)
{
    int ref;
    unsigned char terrain;

    if (x < 0)
        return 0;
    if (x >= 0x50)
        return 0;
    if (y < 0)
        return 0;
    if (y >= 0x50)
        return 0;

    ref = (y * 0x50 + x) * 0x14;
    terrain = CM_CELL((ref)).terrain;
    citizen_a = (unsigned char)CM_CELL((ref)).citizen_a;
    citizen_b = (unsigned char)CM_CELL((ref)).citizen_b;
    if (citizen_a != 0 && citizen_b != 0)
        return 0;
    if ((terrain & 0x8b) != 0)
        return 0;
    if (is_barb != 0) {
        if ((terrain & 0x20) == 0)
            return 0;
    } else {
        if ((terrain & 0x54) != 0)
            return 0;
    }
    for (created_citizen_no = 1; created_citizen_no < 0xC9; created_citizen_no++) {
        if (citizen_list[created_citizen_no].exists == 0) {
            citizen_list[created_citizen_no].exists = 1;
            citizen_list[created_citizen_no].evolve_timer = (evolve_count + rand128) & 0x7fff;
            citizen_list[created_citizen_no].type = type;
            citizen_list[created_citizen_no].dest_x = citizen_list[created_citizen_no].x = x;
            citizen_list[created_citizen_no].dest_y = citizen_list[created_citizen_no].y = y;
            citizen_list[created_citizen_no].map_ref = ref;
            citizen_list[created_citizen_no].pixel_x = x << 4;
            citizen_list[created_citizen_no].pixel_y = y << 4;
            citizen_list[created_citizen_no].world_dir = 1;
            citizen_list[created_citizen_no].speed = 5;
            citizen_list[created_citizen_no].state = 0;
            if (citizen_a == 0) {
                CM_CELL((ref)).citizen_a = created_citizen_no;
            } else {
                CM_CELL((ref)).citizen_b = created_citizen_no;
            }
            CM_CELL((ref)).edge_bits = CM_CELL((ref)).edge_bits | 1;
            if (is_barb != 0) {
                citizen_list[created_citizen_no].is_barbarian = 1;
            } else {
                citizen_list[created_citizen_no].is_barbarian = 0;
            }
            if (type == 3) {
                citizen_list[created_citizen_no].name_id = barbarian_name_count;
            } else {
                citizen_list[created_citizen_no].name_id = roman_name_count;
            }
            roman_name_count++;
            if (roman_name_count >= 0x20)
                roman_name_count = 0;
            barbarian_name_count++;
            if (barbarian_name_count >= 0x10)
                barbarian_name_count = 0;
            return 1;
        }
    }
    return 0;
}

// Allocates an army on an unoccupied region cell allowed by the requested mode.
// FUNCTION: C2 0x2ab1a
// FUNCTION: C2WIN 0x004695b9
int create_army(int army_type, int region_x, int region_y, unsigned char placement_mode)
{
    int map_ptr;
    unsigned char terrain_type;

    if (region_x < 0)
        return 0;
    if (region_x >= 0x3C)
        return 0;
    if (region_y < 0)
        return 0;
    if (region_y >= 0x3C)
        return 0;

    map_ptr = (region_x + region_y * 0x3C) * 8;
    terrain_type = RM_CELL(map_ptr).terrain;
    army_a = (unsigned char)RM_CELL(map_ptr).occupant;
    if (army_a != 0)
        return 0;

    if (placement_mode == 1) {
        if ((terrain_type & 8) == 0)
            return 0;
    } else if (placement_mode == 2) {
        if ((terrain_type & 0x1F) != 0)
            return 0;
    }

    for (created_army_no = 1; created_army_no < 0x19; created_army_no++) {
        if (army_list[created_army_no].exists == 0) {
            clear_army(&army_list[created_army_no]);
            army_list[created_army_no].exists = 1;
            army_list[created_army_no].morale = 2;
            army_list[created_army_no].evolve_timer = (evolve_count + rand128) & 0x7fff;
            army_list[created_army_no].type = army_type;
            army_list[created_army_no].target_x = army_list[created_army_no].x = region_x;
            army_list[created_army_no].target_y = army_list[created_army_no].y = region_y;
            army_list[created_army_no].map_ref = map_ptr;
            army_list[created_army_no].home_ref = map_ptr;
            army_list[created_army_no].fort_ref = map_ptr;
            army_list[created_army_no].pixel_x = region_x << 4;
            army_list[created_army_no].pixel_y = region_y << 4;
            army_list[created_army_no].world_dir = 1;
            army_list[created_army_no].heading = 5;
            army_list[created_army_no].flags |= 1;
            RM_CELL(map_ptr).occupant = created_army_no;
            RM_CELL(map_ptr).edge_bits |= 1;
            return 1;
        }
    }
    return 0;
}

// Allocates and initializes a battle unit.
// FUNCTION: C2 0x2ac8b
// FUNCTION: C2WIN 0x0046993d
int create_unit(int owner, int x, int y, int type)
{
    for (created_unit_no = 1; created_unit_no < 0x33; created_unit_no++) {
        if (unit_list[created_unit_no].exists == 0) {
            unit_list[created_unit_no].exists = 1;
            unit_list[created_unit_no].owner = owner;
            unit_list[created_unit_no].prev_x = unit_list[created_unit_no].x = x;
            unit_list[created_unit_no].prev_y = unit_list[created_unit_no].y = y;
            unit_list[created_unit_no].state = 0;
            unit_list[created_unit_no].type = type;
            return 1;
        }
    }
    return 0;
}

// Allocates a battle figure and claims its destination map cell.
// FUNCTION: C2 0x2acfb
// FUNCTION: C2WIN 0x00469aa3
int create_figure(int sprite_type, int base_x, int off_x, int base_y, int off_y, int owner, int unit_no)
{
    for (created_figure_no = 1; created_figure_no < 0xC9; created_figure_no++) {
        if (figure_list[created_figure_no].exists == 0) {
            figure_list[created_figure_no].exists = 1;
            figure_list[created_figure_no].owner = owner;
            base_x = base_x + off_x;
            base_y = base_y + off_y;
            figure_list[created_figure_no].grid_x = base_x;
            figure_list[created_figure_no].grid_y = base_y;
            figure_list[created_figure_no].map_ref = (base_x + base_y * 0x34) * 4;
            if (((unsigned char *)battle_map)[figure_list[created_figure_no].map_ref + 1] != 0) {
                return 0;
            } else {
                ((unsigned char *)battle_map)[figure_list[created_figure_no].map_ref + 1] = created_figure_no;
            }
            figure_list[created_figure_no].sprite_type = sprite_type;
            figure_list[created_figure_no].unit_ref = unit_no;
            figure_list[created_figure_no].unit_type = unit_list[unit_no].fig_count;
            figure_list[created_figure_no].offset_x = off_x;
            figure_list[created_figure_no].offset_y = off_y;
            if (unit_list[unit_no].heading == -1)
                figure_list[created_figure_no].direction = 4;
            else
                figure_list[created_figure_no].direction = 0;
            figure_list[created_figure_no].anim_state = figure_list[created_figure_no].direction;
            figure_list[created_figure_no].prev_grid_x = base_x;
            figure_list[created_figure_no].prev_grid_y = base_y;
            figure_list[created_figure_no].is_visible = 1;
            return 1;
        }
    }
    return 0;
}

// Allocates an arrow and initializes its flight path between two battle cells.
// FUNCTION: C2 0x2ae0e
// FUNCTION: C2WIN 0x00469d49
int create_arrow(unsigned char *arrow_data_ptr, int owner, int sx, int sy, int ex, int ey)
{
    for (created_arrow_no = 1; created_arrow_no < 0xC9; created_arrow_no++) {
        if (arrow_list[created_arrow_no].exists == 0) {
            arrow_list[created_arrow_no].exists = 1;
            arrow_list[created_arrow_no].owner = owner;
            arrow_list[created_arrow_no].end_x = ex * 7;
            arrow_list[created_arrow_no].end_y = ey * 7;
            arrow_list[created_arrow_no].start_x = sx * 7;
            arrow_list[created_arrow_no].start_y = sy * 7;
            arrow_list[created_arrow_no].grid_x = sx;
            arrow_list[created_arrow_no].grid_y = sy;
            arrow_list[created_arrow_no].map_ref = (arrow_list[created_arrow_no].grid_x + arrow_list[created_arrow_no].grid_y * 0x34) * 4;
            arrow_list[created_arrow_no].heading = get_heading(sx, sy, ex, ey, 0);
            arrow_list[created_arrow_no].arrow_data_ptr = arrow_data_ptr;
            arrow_list[created_arrow_no].anim_count = 0;
            arrow_no = created_arrow_no;
            init_bd(arrow_list[created_arrow_no].start_x, arrow_list[created_arrow_no].start_y,
                    arrow_list[created_arrow_no].end_x, arrow_list[created_arrow_no].end_y);
            return 1;
        }
    }
    high_beep();
    return 0;
}

// Zeros a citizen record.
// FUNCTION: C2 0x2af5a
// FUNCTION: C2WIN 0x00469f63
void clear_citizen(struct citizen_rec *record_ptr)
{
#if PLATFORM_WINDOWS
    memset(record_ptr, 0, sizeof(struct citizen_rec));
#else
    unsigned int i;

    for (i = 0; i < sizeof(struct citizen_rec); i++) {
        ((char *)record_ptr)[i] = 0;
    }
#endif
}

// Zeros an army record.
// FUNCTION: C2 0x2af6d
// FUNCTION: C2WIN 0x00469f7e
void clear_army(struct army_rec *record_ptr)
{
#if PLATFORM_WINDOWS
    memset(record_ptr, 0, sizeof(struct army_rec));
#else
    unsigned int i;
    char *record_bytes = (char *)record_ptr;
    for (i = 0; i < sizeof(struct army_rec); i++) {
        record_bytes[i] = 0;
    }
#endif
}

// Clears every field in a unit record.
// FUNCTION: C2 0x2afbc
// FUNCTION: C2WIN 0x00469f9c
void clear_unit(struct unit_rec *record_ptr)
{
#if PLATFORM_WINDOWS
    memset(record_ptr, 0, sizeof(struct unit_rec));
#else
    unsigned int i;
    char *record_bytes = (char *)record_ptr;
    for (i = 0; i < sizeof(struct unit_rec); i++) {
        record_bytes[i] = 0;
    }
#endif
}

// Zeros a battle-figure record.
// FUNCTION: C2 0x2af8e REORDERED
// FUNCTION: C2WIN 0x00469fb7
void clear_figure(struct figure_rec *record_ptr)
{
#if PLATFORM_WINDOWS
    memset(record_ptr, 0, sizeof(struct figure_rec));
#else
    unsigned int i;
    char *record_bytes = (char *)record_ptr;
    for (i = 0; i < sizeof(struct figure_rec); i++) {
        record_bytes[i] = 0;
    }
#endif
}

// Zeros an arrow record.
// FUNCTION: C2 0x2afa1
// FUNCTION: C2WIN 0x00469fd2
void clear_arrow(struct arrow_rec *record_ptr)
{
#if PLATFORM_WINDOWS
    memset(record_ptr, 0, sizeof(struct arrow_rec));
#else
    unsigned int i;
    char *record_bytes = (char *)record_ptr;
    for (i = 0; i < sizeof(struct arrow_rec); i++) {
        record_bytes[i] = 0;
    }
#endif
}

// Removes a battle unit by clearing its record.
// FUNCTION: C2 0x2afb4
// FUNCTION: C2WIN 0x00469fed
void remove_unit(int unit_idx)
{
    clear_unit(&unit_list[unit_idx]);
}

// Releases a figure's battle-map cell and clears its record.
// FUNCTION: C2 0x2afc3
// FUNCTION: C2WIN 0x0046a016
void remove_figure(int figure_idx)
{
    ((unsigned char *)battle_map)[figure_list[figure_idx].map_ref + 1] = 0;
    clear_figure(&figure_list[figure_idx]);
}

// Releases a citizen's city-map slot and clears its record.
// FUNCTION: C2 0x2afe3
// FUNCTION: C2WIN 0x0046a055
void remove_citizen(int n)
{
    if (CM_CELL(citizen_list[n].map_ref).citizen_a == n) {
        CM_CELL(citizen_list[n].map_ref).citizen_a = 0;
    } else if (CM_CELL(citizen_list[n].map_ref).citizen_b == n) {
        CM_CELL(citizen_list[n].map_ref).citizen_b = 0;
    }
    clear_citizen(&citizen_list[n]);
}

// Releases an army's region-map cell and clears its record.
// FUNCTION: C2 0x2b02a
// FUNCTION: C2WIN 0x0046a102
void remove_army(int army_idx)
{
    RM_CELL(army_list[army_idx].map_ref).occupant = 0;
    clear_army(&army_list[army_idx]);
}

// Clears every usable battle-unit record.
// FUNCTION: C2 0x2b04d
// FUNCTION: C2WIN 0x0046a146
void clear_unit_list(void)
{
    for (unit_no = 1; unit_no < 0x33; unit_no++) {
        clear_unit(&unit_list[unit_no]);
    }
}

// Clears every usable battle-figure record.
// FUNCTION: C2 0x2b079
// FUNCTION: C2WIN 0x0046a19d
void clear_figure_list(void)
{
    for (figure_no = 1; figure_no < 0xC9; figure_no++) {
        clear_figure(&figure_list[figure_no]);
    }
}

// Clears every usable arrow record.
// FUNCTION: C2 0x2b0a7
// FUNCTION: C2WIN 0x0046a1f3
void clear_arrow_list(void)
{
    for (arrow_no = 1; arrow_no < 0xC9; arrow_no++) {
        clear_arrow(&arrow_list[arrow_no]);
    }
}

// Rebuilds city-map citizen occupancy from the active citizen list.
// FUNCTION: C2 0x2b0e3
// FUNCTION: C2WIN 0x0046a245
void check_citizen_list(void)
{
    int i;
    /* Clear citizen slots in city_map (unrolled 4x, 20 bytes/cell) */
    for (i = 0; i < 128000; i += 80) {
        CM_CELL((i)).citizen_a = 0;
        CM_CELL((i)).citizen_b = 0;
        CM_CELL((i + 1 * CITY_CELL_BYTES)).citizen_a = 0;
        CM_CELL((i + 1 * CITY_CELL_BYTES)).citizen_b = 0;
        CM_CELL((i + 2 * CITY_CELL_BYTES)).citizen_a = 0;
        CM_CELL((i + 2 * CITY_CELL_BYTES)).citizen_b = 0;
        CM_CELL((i + 3 * CITY_CELL_BYTES)).citizen_a = 0;
        CM_CELL((i + 3 * CITY_CELL_BYTES)).citizen_b = 0;
    }
    /* Re-assign citizens to their map cells */
    for (citizen_no = 1; citizen_no < 201; citizen_no++) {
        if (citizen_list[citizen_no].exists != 0) {
            citizen_a = (unsigned char)CM_CELL(citizen_list[citizen_no].map_ref).citizen_a;
            citizen_b = (unsigned char)CM_CELL(citizen_list[citizen_no].map_ref).citizen_b;
            if (citizen_a == 0) {
                CM_CELL(citizen_list[citizen_no].map_ref).citizen_a = citizen_no;
                continue;
            }
            if (citizen_b == 0) {
                CM_CELL(citizen_list[citizen_no].map_ref).citizen_b = citizen_no;
                continue;
            }
            clear_citizen(&citizen_list[citizen_no]);
        }
    }
}

// Rebuilds region-map army occupancy from the active army list.
// FUNCTION: C2 0x2b1ba
// FUNCTION: C2WIN 0x0046a3ed
void check_army_list(void)
{
    int i;
    unsigned char terrain_flag;

    /* Clear army slots in region_map where terrain bit 0 is clear (unrolled 4x, 8 bytes/cell) */
    for (i = 0; i < 28800; i += 32) {
        terrain_flag = RM_CELL(i).terrain;
        if ((terrain_flag & 1) == 0) RM_CELL(i).occupant = 0;
        terrain_flag = RM_CELL(i + 8).terrain;
        if ((terrain_flag & 1) == 0) RM_CELL(i + 8).occupant = 0;
        terrain_flag = RM_CELL(i + 16).terrain;
        if ((terrain_flag & 1) == 0) RM_CELL(i + 16).occupant = 0;
        terrain_flag = RM_CELL(i + 24).terrain;
        if ((terrain_flag & 1) == 0) RM_CELL(i + 24).occupant = 0;
    }
    /* Re-assign armies to their map cells */
    for (army_no = 1; army_no < 26; army_no++) {
        if (army_list[army_no].exists != 0) {
            army_a = (unsigned char)RM_CELL(army_list[army_no].map_ref).occupant;
            if (army_a == 0) {
                RM_CELL(army_list[army_no].map_ref).occupant = army_no;
                continue;
            }
        }
    }
}

// Removes every citizen and releases its city-map occupancy.
// FUNCTION: C2 0x2b282
// FUNCTION: C2WIN 0x0046a551
void clear_citizen_list(void)
{
    int i;

    for (citizen_no = 1; citizen_no < 0xC9; citizen_no++) {
        remove_citizen(citizen_no);
    }
}

// Removes every army and releases its region-map occupancy.
// FUNCTION: C2 0x2b2a8
// FUNCTION: C2WIN 0x0046a59b
void clear_army_list(void)
{
    int i;

    for (army_no = 1; army_no < 0x1A; army_no++) {
        remove_army(army_no);
    }
}

// Normalizes temporary army existence states after restoring a game.
// FUNCTION: C2 0x2b2cc
// FUNCTION: C2WIN 0x0046a5e3
void army_restoring_adjusts(void)
{
    for (army_no = 0; army_no < 0x1A; army_no++) {
        if (army_list[army_no].exists != 0) {
            if (army_list[army_no].exists == 2) {
                army_list[army_no].exists = 0;
            } else if (army_list[army_no].exists == 3) {
                army_list[army_no].exists = 1;
            }
        }
    }
}

// Normalizes temporary army states and reports whether any army is still being built.
// FUNCTION: C2 0x2b31d
// FUNCTION: C2WIN 0x0046a6b4
int any_army_building_adjusts(void)
{
    int any_building = 0;
    for (army_no = 0; army_no < 0x1A; army_no++) {
        if (army_list[army_no].exists == 2) {
            army_list[army_no].exists = 1;
        }
        if (army_list[army_no].exists == 3) {
            any_building = 1;
        }
    }
    return any_building;
}

// Removes armies left in the temporary building state.
// FUNCTION: C2 0x2b37b
// FUNCTION: C2WIN 0x0046a75f
void army_building_adjusts(void)
{
    for (army_no = 0; army_no < 0x1A; army_no++) {
        if (army_list[army_no].exists == 3) {
            remove_army(army_no);
        }
    }
}

// Marks the active army assigned to a fort for removal.
// FUNCTION: C2 0x2b3b3
// FUNCTION: C2WIN 0x0046a7c6
void clear_army_from_fort_ref(int fort_ref)
{
    for (army_no = 0; army_no < 26; army_no++) {
        if (army_list[army_no].exists != 0 && fort_ref == army_list[army_no].fort_ref) {
            army_list[army_no].exists = 3;
            return;
        }
    }
}

// Returns the cohort identifier of the army assigned to a fort.
// FUNCTION: C2 0x2b3f9
// FUNCTION: C2WIN 0x0046a85b
int get_army_name_from_fort_ref(int fort_ref)
{
    for (army_no = 0; army_no < 26; army_no++) {
        if (army_list[army_no].exists != 0 && fort_ref == army_list[army_no].fort_ref) {
            return army_list[army_no].cohort_id;
        }
    }
}

// Selects the nearest friendly field army and returns its distance.
// FUNCTION: C2 0x2b442
// FUNCTION: C2WIN 0x0046a8f0
int get_nearest_army_to_track(int map_x, int map_y)
{
    int distance;
    int best_distance;

    best_distance = 9999;
    for (army_no = 0; army_no < 26; army_no++) {
        if (army_list[army_no].exists != 0) {
            if (army_list[army_no].type == 1) {
                if (army_list[army_no].map_x == 0 || army_list[army_no].map_y == 0) continue;
                distance = get_longest_distance(army_list[army_no].map_x,
                                            army_list[army_no].map_y, map_x, map_y);
                if (distance < best_distance) { tracking_army = army_no; best_distance = distance; }
            }
        }
    }
    return best_distance;
}

// Selects the nearest enemy field army and returns its distance.
// FUNCTION: C2 0x2b4cb
// FUNCTION: C2WIN 0x0046aa26
int get_nearest_enemy_to_track(int map_x, int map_y)
{
    int distance;
    int best_distance;

    best_distance = 9999;
    for (army_no = 0; army_no < 26; army_no++) {
        if (army_list[army_no].exists != 0) {
            if (army_list[army_no].type >= 2 && army_list[army_no].type <= 5) {
                if (army_list[army_no].map_x == 0 || army_list[army_no].map_y == 0) continue;
                distance = get_longest_distance(army_list[army_no].map_x,
                                            army_list[army_no].map_y, map_x, map_y);
                if (distance < best_distance) { hunting_army = army_no; best_distance = distance; }
            }
        }
    }
    return best_distance;
}

// Returns a tracked army's distance from a point, or 999 if it is off-map.
// FUNCTION: C2 0x2b557
// FUNCTION: C2WIN 0x0046ab7e
int get_tracking_army_distance(int army_idx, int map_x, int map_y)
{
    int distance;

    if (army_list[army_idx].map_x == 0 || army_list[army_idx].map_y == 0) {
        return 999;
    }
    distance = get_longest_distance(army_list[army_idx].map_x,
                                    army_list[army_idx].map_y, map_x, map_y);
    return distance;
}

// Finds the battle unit under a left-click, if any.
// FUNCTION: C2 0x2b593
// FUNCTION: C2WIN 0x0046ac17
int get_a_unit_centered_on_mouse(void)
{
    if (mouse_left_preclick == 0) {
        return 0;
    }
    temp_unit = find_figure(1);
    return temp_unit;
}

// Reports that no battle figure was found.
// FUNCTION: C2 0x2b59c
int find_figure(int selection_mode)
{
    return 0;
}

// Clears unit target flags, finds a figure, and marks its unit as the target.
// FUNCTION: C2 0x2b5b1
// FUNCTION: C2WIN 0x0046ac54
int get_a_shootable_unit(void)
{
    for (temp_unit = 1; temp_unit < 0x33; temp_unit++) {
        unit_list[temp_unit].is_target = 0;
    }
    temp_unit = find_figure(0);
    unit_list[temp_unit].is_target = 1;
    return temp_unit;
}

// Returns the eight-way heading from one point to another.
// FUNCTION: C2 0x2b5f5
// FUNCTION: C2WIN 0x0046ad14
char get_heading(int start_x, int start_y, int end_x, int end_y, unsigned char still_offset)
{
    char heading;
    if (start_x > end_x) {
        if (start_y > end_y) heading = HEADING_NW;
        else if (start_y == end_y) heading = HEADING_W;
        else if (start_y < end_y) heading = HEADING_SW;
    } else if (start_x == end_x) {
        if (start_y > end_y) heading = HEADING_N;
        else if (start_y == end_y) heading = still_offset + HEADING_STILL;
        else if (start_y < end_y) heading = HEADING_S;
    } else if (start_x < end_x) {
        if (start_y > end_y) heading = HEADING_NE;
        else if (start_y == end_y) heading = HEADING_E;
        else if (start_y < end_y) heading = HEADING_SE;
    }
    return heading;
}

// Initializes a city-map corridor with path costs, barriers, and road classes.
// FUNCTION: C2 0x2b662
// FUNCTION: C2WIN 0x0046ae0d
void clear_ferret_map(int margin, unsigned char *map_base, int map_width, int map_height,
                      int cell_size, int start_x, int start_y, int end_x, int end_y)
{
    int lower_y;
    int y_max;
    int left_x;
    int max_x;
    int left;
    int top;
    int hi_y;
    int right_edge;
    int ptr;
    int cell_x;
    int ypos;
    int map_ptr;
    int tmp;
    unsigned char terrain;
    unsigned char value;
    int first_row;

    if (start_x <= end_x) { left_x = start_x; max_x = end_x; }
    else { left_x = end_x; max_x = start_x; }
    if (start_y <= end_y) { lower_y = start_y; y_max = end_y; }
    else { lower_y = end_y; y_max = start_y; }
    left = left_x - margin;
    top = lower_y - margin;
    right_edge = max_x + margin;
    hi_y = y_max + margin;
    if (left < 0) left = 0;
    if (top < 0) top = 0;
    if (right_edge >= map_width) right_edge = map_width - 1;
    if (hi_y >= map_height) hi_y = map_height - 1;

    ptr = cell_size * (left + top * map_width);
    for (ypos = top; ypos <= hi_y; ypos++, ptr += map_width * cell_size) {
        for (cell_x = left, map_ptr = ptr; cell_x <= right_edge; cell_x++, map_ptr += cell_size) {
            if (ypos == top) value = 0xFF;
            else if (ypos == hi_y) value = 0xFF;
            else if (cell_x == left) value = 0xFF;
            else if (cell_x == right_edge) value = 0xFF;
            else {
                value = 0;
                terrain = *(map_base + map_ptr + 1);
                if ((terrain & 1) != 0) { value = 0xFE;
                } else if ((terrain & 4) != 0) {
                    if (citizen_list[citizen_no].type == 3) { value = 0xFE;
                    } else if ((terrain & 0x20) != 0) {
                        *(map_base + map_ptr + 6) = 0;
                        *(map_base + map_ptr + 5) = 1;
                    } else {
                        value = 0xFE;
                    }
                } else {
                    *(map_base + map_ptr + 6) = 0;
                    if ((terrain & 0x20) != 0) {
                        *(map_base + map_ptr + 5) = 1;
                    } else if (terrain != 0) { value = 0xFE; *(map_base + map_ptr + 5) = 0;
                    } else { *(map_base + map_ptr + 5) = 2;
                    }
                }
            }
            *(map_base + map_ptr + 2) = value;
        } }
}

void clear_sea_ferret_map(int, int, unsigned char *, int, int, int, int, int, int, int);
int run_2_map_ferrets(int, unsigned char *, int, int, int, int, int, int, int);
int trace_back_ferret(void);

// Initializes a region-map corridor for the current army's movement rules.
// FUNCTION: C2 0x2b7e0
// FUNCTION: C2WIN 0x0046b083
void clear_region_ferret_map(int movement_mode, int margin, unsigned char *map_base, int map_width,
                             int map_height, int cell_size, int start_x, int start_y,
                             int end_x, int end_y)
{
    int y_max;
    int first_y;
    int left_col;
    int max_x;
    int left;
    int top;
    int hi_y;
    int right_edge;
    int first_row;
    int ptr;
    int map_ptr;
    int cell_x;
    int ypos;
    int tmp;
    unsigned char terrain;
    unsigned char path_value;
    unsigned char terrain_type;
    unsigned char cell7;

    if (start_x <= end_x) { left_col = start_x; max_x = end_x; }
    else { left_col = end_x; max_x = start_x; }
    if (start_y <= end_y) { first_y = start_y; y_max = end_y; }
    else { first_y = end_y; y_max = start_y; }
    left = left_col - margin;
    top = first_y - margin;
    right_edge = max_x + margin;
    hi_y = y_max + margin;
    if (left < 0) left = 0;
    if (top < 0) top = 0;
    if (right_edge >= map_width) right_edge = map_width - 1;
    if (hi_y >= map_height) hi_y = map_height - 1;

    ptr = cell_size * (left + top * map_width);
    for (ypos = top; ypos <= hi_y; ypos++, ptr += map_width * cell_size) {
        for (cell_x = left, map_ptr = ptr; cell_x <= right_edge; cell_x++, map_ptr += cell_size) {
            if (ypos == top) path_value = 0xFF;
            else if (ypos == hi_y) path_value = 0xFF;
            else if (cell_x == left) path_value = 0xFF;
            else if (cell_x == right_edge) path_value = 0xFF;
            else {
                path_value = 0;
                terrain = *(map_base + map_ptr + 1);
                terrain_type = *(map_base + map_ptr);
                cell7 = *(map_base + map_ptr + 7);

                *(map_base + map_ptr + 6) = 0;
                *(map_base + map_ptr + 5) = 0;
                if ((terrain & 1) != 0) {
                    if (army_list[army_no].type == 1) { path_value = 0xFE;
                    } else
                    if (terrain_type >= 0x93 && terrain_type <= 0x9B) { path_value = 0xFE;
                    } else {
                        *(map_base + map_ptr + 5) = 1; }
                } else if ((terrain & 4) != 0) {
                    if (army_list[army_no].type == 1) {
                        *(map_base + map_ptr + 5) = 1;
                    } else {
                        path_value = 0xFE;
                    }
                } else if ((terrain & 2) != 0) {
                    if (army_list[army_no].type == 1) {
                        if ((terrain & 0x20) != 0) {
                            *(map_base + map_ptr + 5) = 1;
                        } else {
                            path_value = 0xFE;
                        }
                    } else if (movement_mode != 0) { *(map_base + map_ptr + 5) = 1;
                    } else {
                        path_value = 0xFE;
                    }
                } else if ((terrain & 0x20) != 0) {
                    *(map_base + map_ptr + 5) = 1;
                } else if ((terrain & 8) != 0) {
                    if ((terrain & 0x10) != 0) {
                        path_value = 0xFE;
                    } else if (army_list[army_no].type != 1) {
                        path_value = 0xFE;
                    } else if (army_list[army_no].state_idx != 4) {
                        path_value = 0xFE;
                    } else if (cell7 == 0) {
                        path_value = 0xFE;
                    } else if (cell7 != army_list[army_no].army_id) {
                        path_value = 0xFE;
                    } else {
                        *(map_base + map_ptr + 5) = 2;
                    }
                } else if ((terrain & 0x10) != 0) {
                    path_value = 0xFE;
                } else if ((terrain & 8) != 0) {
                    path_value = 0xFE;
                } else {
                    *(map_base + map_ptr + 5) = 2;
                }
            }
            *(map_base + map_ptr + 2) = path_value;
        } }
}

// Initializes a map corridor so only navigable sea cells remain passable.
// FUNCTION: C2 0x2ba5e
// FUNCTION: C2WIN 0x0046b4a9
void clear_sea_ferret_map(int unused, int margin, unsigned char *map_base, int map_wi,
                          int map_hi, int cell_size, int x1, int y1,
                          int x2, int y2)
{
    int first_y;
    int highy;
    int left_col;
    int max_x;
    int x_begin;
    int min_y;
    int end_line;
    int high_col;
    int ptr;
    int map_ptr;
    int x;
    int cy;
    int cell_no;
    int j;
    unsigned char terrain_type;
    unsigned char path_value;

    if (x1 <= x2) {
        left_col = x1;
        max_x = x2;
    } else {
        left_col = x2;
        max_x = x1;
    }
    if (y1 <= y2) {
        first_y = y1;
        highy = y2;
    } else {
        first_y = y2;
        highy = y1;
    }
    x_begin = left_col - margin;
    min_y = first_y - margin;
    high_col = max_x + margin;
    end_line = highy + margin;
    if (x_begin < 0) x_begin = 0;
    if (min_y < 0) min_y = 0;
    if (high_col >= map_wi) high_col = map_wi - 1;
    if (end_line >= map_hi) end_line = map_hi - 1;

    ptr = cell_size * (x_begin + min_y * map_wi);
    for (cy = min_y; cy <= end_line; cy++, ptr += map_wi * cell_size) {
        for (x = x_begin, map_ptr = ptr; x <= high_col; x++, map_ptr += cell_size) {
            if (cy == min_y)
                path_value = 0xFF;
            else if (cy == end_line)
                path_value = 0xFF;
            else if (x == x_begin)
                path_value = 0xFF;
            else if (x == high_col)
                path_value = 0xFF;
            else {
                path_value = 0xFE;
                terrain_type = *(map_base + map_ptr + 1);
                *(map_base + map_ptr + 6) = 0;
                *(map_base + map_ptr + 5) = 0;
                if ((terrain_type & 0x10) != 0) {
                    if ((terrain_type & 8) != 0) {
                        path_value = 0;
                        *(map_base + map_ptr + 5) = 1;
                    }
                }
            }
            *(map_base + map_ptr + 2) = path_value;
        }
    }
}

// Searches clockwise and anticlockwise for a route, then traces a successful path.
// FUNCTION: C2 0x2bb7b
// FUNCTION: C2WIN 0x0046b68b
int run_2_map_ferrets(int margin, unsigned char *map_base, int map_width, int map_height,
                      int cell_size, int start_x, int start_y,
                      int target_x, int target_y)
{
    int i;

    anti_ferret_x = start_x;
    clock_ferret_x = start_x;
    anti_ferret_y = start_y;
    clock_ferret_y = start_y;
    anti_ferret_ptr = cell_size * (start_y * map_width + start_x);
    clock_ferret_ptr = anti_ferret_ptr;
    anti_ferret_running = 1;
    clock_ferret_running = 1;
    anti_ferret_count = *(map_base + clock_ferret_ptr + 5) + 1;
    clock_ferret_count = anti_ferret_count;
    *(map_base + clock_ferret_ptr + 2) = 1;
    ferret_energy = 200;
    ferret_home = 0;
    ferret_horiz_off = cell_size;
    ferret_vert_off = cell_size * map_width;
    ferret_targ_x = target_x;
    ferret_targ_y = target_y;
    ferret_targ_ptr = cell_size * (target_y * map_width + target_x);
    ferret_map_wi = map_width;
    ferret_map_hi = map_height;
    ferret_map = map_base;
    last_clock_ferret_dirc = 0;
    last_anti_ferret_dirc = 0;

    while (clock_ferret_running != 0 || anti_ferret_running != 0) {
        run_clock_ferret();
        run_anti_ferret();
        if (ferret_home != 0) break;
        ferret_energy--;
        if (ferret_energy <= 0) break;
    }

    for (i = 0; i < 20; i++)
        ferret_run[i] = 0;

    if (ferret_home != 0) {
        smooth_ferret_run(margin, map_base, map_width, map_height,
                          cell_size, start_x, start_y, target_x, target_y);
        if (trace_back_ferret() != 0) {
            load_ferret_run(start_x, start_y, 20);
            return 1;
        }
    }
    return 0;
}

// Converts a marked ferret path into a bounded sequence of directions.
// FUNCTION: C2 0x2bceb
// FUNCTION: C2WIN 0x0046b875
void load_ferret_run(int start_x, int start_y, int max_length)
{
    unsigned char direction;
    unsigned char reverse_direction;

    ferret_run_length = 0;
    tb_x = start_x;
    tb_y = start_y;
    tb_ptr = (start_x + start_y * ferret_map_wi) * ferret_horiz_off;
    reverse_direction = 8;
    while (max_length > ferret_run_length) {
        for (direction = 0; direction < 8; direction++) {
            if (get_ferret2(direction) == 1 && direction != reverse_direction) break;
        }
        if (direction >= 8) return;
        reverse_direction = (direction + 4) % 8;
        ferret_run[ferret_run_length++] = direction;
        move_to_tb_value(direction);
    }
}

// Relaxes pathfinding costs within the corridor around the current ferret route.
// FUNCTION: C2 0x2bd7c
// FUNCTION: C2WIN 0x0046b96b
void smooth_ferret_run(int margin, unsigned char *map_base, int map_width, int map_height,
                       int cell_size, int start_x, int start_y, int end_x, int end_y)
{
    int min_x;
    int min_y;
    int max_x;
    int max_y;
    int corridor_start_x;
    int corridor_start_y;
    int corridor_end_y;
    int corridor_end_x;
    int row_offset;
    int direction;
    unsigned char saved_target_value;
    unsigned char current_value;
    unsigned char best_value;
    unsigned char neighbor_value;
    unsigned char occupant_b;
    unsigned char occupant_a;

    saved_target_value = *(map_base + ferret_targ_ptr + 2);

    if (start_x <= end_x) {
        min_x = start_x;
        max_x = end_x;
    } else {
        min_x = end_x;
        max_x = start_x;
    }
    if (start_y <= end_y) {
        min_y = start_y;
        max_y = end_y;
    } else {
        min_y = end_y;
        max_y = start_y;
    }
    corridor_start_x = min_x - margin;
    corridor_start_y = min_y - margin;
    corridor_end_x = max_x + margin;
    corridor_end_y = max_y + margin;
    if (corridor_start_x < 0) corridor_start_x = 0;
    if (corridor_start_y < 0) corridor_start_y = 0;
    if (corridor_end_x >= map_width) corridor_end_x = map_width - 1;
    if (corridor_end_y >= map_height) corridor_end_y = map_height - 1;

    row_offset = (corridor_start_x + corridor_start_y * map_width) * cell_size;
    for (tb_y = corridor_start_y; tb_y <= corridor_end_y; tb_y++, row_offset += map_width * cell_size) {
        for (tb_x = corridor_start_x, tb_ptr = row_offset; tb_x <= corridor_end_x; tb_x++, tb_ptr += cell_size) {
            current_value = *(map_base + tb_ptr + 2);
            occupant_a = *(map_base + tb_ptr + 7);
            occupant_b = *(map_base + tb_ptr + 8);
            if (current_value < 0xFE) {
                if (current_value == 0) {
                    best_value = 0xFA;
                } else {
                    best_value = current_value;
                }
                for (direction = 0; direction < 8; direction++) {
                    neighbor_value = get_tb_value(direction);
                    if (neighbor_value < 0xFE && neighbor_value != 0) {
                        if (tb_road_flag == 1) {
                            neighbor_value++;
                        } else if (tb_road_flag == 2) {
                            neighbor_value += 2;
                        } else {
                            continue;
                        }
                        if (neighbor_value < best_value) {
                            best_value = neighbor_value;
                        }
                    }
                }
                if (current_value == 0 || best_value != current_value) {
                    *(map_base + tb_ptr + 2) = best_value;
                    if (occupant_a != 0 && occupant_b != 0) {
                        *(map_base + tb_ptr + 2) = 0xFE;
                    }
                }
            }
        }
    }
    *(map_base + ferret_targ_ptr + 2) = saved_target_value;
}

// Traces a ferret route backward from the target along decreasing path costs.
// FUNCTION: C2 0x2bf23
// FUNCTION: C2WIN 0x0046bc3b
int trace_back_ferret(void)
{
    unsigned char current_value;
    unsigned char new_value;
    int best_direction;
    int direction;
    int steps_left;

    tb_x = ferret_targ_x;
    tb_y = ferret_targ_y;
    tb_ptr = (ferret_targ_x + ferret_targ_y * ferret_map_wi) * ferret_horiz_off;
    current_value = *(ferret_map + tb_ptr + 2);
    *(ferret_map + tb_ptr + 6) = 1;
    steps_left = 100;

    while (current_value > 1) {
        unsigned char best_value;

        steps_left--; if (steps_left < 0) return 0;
        current_value++;
        best_value = current_value;
        best_direction = 0;
        for (direction = 0; direction < 8; direction++) {
            new_value = get_tb_value(direction);
            if (new_value == 0) continue;
            if (new_value < best_value) { best_value = new_value;
                best_direction = direction;
            } else if (new_value == best_value && tb_road_flag == 1) { best_direction = direction;
            }
        }
        if (best_value == current_value) return 0;      /* cur_val was pre-incremented */
        current_value = best_value;
        move_to_tb_value(best_direction);
    }
    return 1;
}

// Traces a ferret route forward for a bounded number of steps.
// FUNCTION: C2 0x2bfca
// FUNCTION: C2WIN 0x0046bd96
int trace_forward_ferret(int steps_remaining)
{
    unsigned char current_value;
    unsigned char best_value;
    int best_direction;
    int direction;

    tb_x = clock_ferret_x;
    tb_y = clock_ferret_y;
    tb_ptr = clock_ferret_ptr;
    current_value = *(ferret_map + tb_ptr + 2);
    *(ferret_map + tb_ptr + 6) = 1;

    while (steps_remaining-- > 0) {
        best_value = current_value;
        best_direction = 0;
        for (direction = 0; direction < 8; direction++) {
            unsigned char neighbor_value = get_tb_value(direction);
            if (neighbor_value != 0 && neighbor_value < 0xFE) {
                if (neighbor_value > best_value) { best_value = neighbor_value; best_direction = direction; }
                else if (neighbor_value == best_value && tb_road_flag == 1) best_direction = direction;
            }
        }
        if (best_value == current_value) return 0;
        current_value = best_value;
        move_to_tb_value(best_direction);
    }
    return 1;
}

// Advances the clockwise path probe toward the target.
// FUNCTION: C2 0x2c062
// FUNCTION: C2WIN 0x0046bedd
void run_clock_ferret(void)
{
    unsigned char heading;
    int move_result;
    int attempt_count;
    int direction;

    if (clock_ferret_running == 0) return;

    heading = ferret_heading(clock_ferret_x, clock_ferret_y);
    if (heading == 8) {
        ferret_home = 1;
        return;
    }

    direction = heading;
    attempt_count = 0;
    do {
        move_result = (unsigned char)check_clock_ferret_move((signed char)direction);
        if (tb_occ_a_flag != 0 && tb_occ_b_flag != 0) move_result = 0xFE;
        if (move_result == 0xFF) { clock_ferret_running = 0; return; }
        if (move_result == 0) {
            move_clock_ferret((signed char)direction, 0);
            last_clock_ferret_dirc = direction;
            return;
        }
        if (++direction >= 8) direction = 0;
    } while (++attempt_count < 8);

    direction = last_clock_ferret_dirc;
    attempt_count = 0;
    do {
        move_result = (unsigned char)check_clock_ferret_move((signed char)direction);
        if (tb_occ_a_flag != 0 && tb_occ_b_flag != 0) move_result = 0xFE;
        if (move_result < 0xFE && tb_prev_flag == 0) {
            move_clock_ferret((signed char)direction, 1);
            *(ferret_map + clock_ferret_ptr + 5) |= 0x80;
            return;
        }
        if (++direction >= 8) direction = 0;
    } while (++attempt_count < 8);

    attempt_count = 0;
    do {
        move_result = (unsigned char)check_clock_ferret_move((signed char)direction);
        if (move_result < 0xFE) {
            move_clock_ferret((signed char)direction, 1);
            return;
        }
        if (++direction >= 8) direction = 0;
    } while (++attempt_count < 8);
    clock_ferret_running = 0;
}

// Advances the anticlockwise path probe toward the target.
// FUNCTION: C2 0x2c1ab
// FUNCTION: C2WIN 0x0046c10f
void run_anti_ferret(void)
{
    unsigned char heading;
    int move_result;
    int attempt_count;
    int direction;

    if (anti_ferret_running == 0) return;

    heading = ferret_heading(anti_ferret_x, anti_ferret_y);
    if (heading == 8) {
        ferret_home = 1;
        return;
    }

    direction = heading;
    attempt_count = 0;
    do {
        move_result = (unsigned char)check_anti_ferret_move((signed char)direction);
        if (tb_occ_a_flag != 0 && tb_occ_b_flag != 0) move_result = 0xFE;
        if (move_result == 0xFF) { anti_ferret_running = 0; return; }
        if (move_result == 0) {
            move_anti_ferret((signed char)direction, 0);
            last_anti_ferret_dirc = direction;
            return;
        }
        if (--direction < 0) direction = 7;
    } while (++attempt_count < 8);

    direction = last_anti_ferret_dirc;
    attempt_count = 0;
    do {
        move_result = (unsigned char)check_anti_ferret_move((signed char)direction);
        if (tb_occ_a_flag != 0 && tb_occ_b_flag != 0) move_result = 0xFE;
        if (move_result < 0xFE && tb_prev_flag == 0) {
            move_anti_ferret((signed char)direction, 1);
            *(ferret_map + anti_ferret_ptr + 5) |= 0x40;
            return;
        }
        if (--direction < 0) direction = 7;
    } while (++attempt_count < 8);

    attempt_count = 0;
    do {
        move_result = (unsigned char)check_anti_ferret_move((signed char)direction);
        if (move_result < 0xFE) {
            move_anti_ferret((signed char)direction, 1);
            return;
        }
        if (--direction < 0) direction = 7;
    } while (++attempt_count < 8);
    anti_ferret_running = 0;
}

// Reads a clockwise probe neighbor's cost, occupancy, and visited state.
// FUNCTION: C2 0x2c31b
// FUNCTION: C2WIN 0x0046c335
signed char check_clock_ferret_move(signed char direction)
{
    switch (direction) {
    case 0:
        if (clock_ferret_y <= 0) return -1;
        tb_prev_flag = *(ferret_map + clock_ferret_ptr - ferret_vert_off + 5) & 0x80;
        tb_occ_a_flag = *(ferret_map + clock_ferret_ptr - ferret_vert_off + 7);
        tb_occ_b_flag = *(ferret_map + clock_ferret_ptr - ferret_vert_off + 8);
        if (clock_ferret_ptr - ferret_vert_off == ferret_targ_ptr) tb_occ_a_flag = 0;
        return *(ferret_map + clock_ferret_ptr - ferret_vert_off + 2);
    case 1:
        if (clock_ferret_y <= 0) return -1;
        if (ferret_map_wi - 1 <= clock_ferret_x) return -1;
        tb_prev_flag = *(ferret_map + clock_ferret_ptr - ferret_vert_off + ferret_horiz_off + 5) & 0x80;
        tb_occ_a_flag = *(ferret_map + clock_ferret_ptr - ferret_vert_off + ferret_horiz_off + 7);
        tb_occ_b_flag = *(ferret_map + clock_ferret_ptr - ferret_vert_off + ferret_horiz_off + 8);
        if (clock_ferret_ptr - ferret_vert_off + ferret_horiz_off == ferret_targ_ptr) tb_occ_a_flag = 0;
        return *(ferret_map + clock_ferret_ptr - ferret_vert_off + ferret_horiz_off + 2);
    case 2:
        if (ferret_map_wi - 1 <= clock_ferret_x) return -1;
        tb_prev_flag = *(ferret_map + clock_ferret_ptr + ferret_horiz_off + 5) & 0x80;
        tb_occ_a_flag = *(ferret_map + clock_ferret_ptr + ferret_horiz_off + 7);
        tb_occ_b_flag = *(ferret_map + clock_ferret_ptr + ferret_horiz_off + 8);
        if (clock_ferret_ptr + ferret_horiz_off == ferret_targ_ptr) tb_occ_a_flag = 0;
        return *(ferret_map + clock_ferret_ptr + ferret_horiz_off + 2);
    case 3:
        if (ferret_map_hi - 1 <= clock_ferret_y) return -1;
        if (ferret_map_wi - 1 <= clock_ferret_x) return -1;
        tb_prev_flag = *(ferret_map + clock_ferret_ptr + ferret_vert_off + ferret_horiz_off + 5) & 0x80;
        tb_occ_a_flag = *(ferret_map + clock_ferret_ptr + ferret_vert_off + ferret_horiz_off + 7);
        tb_occ_b_flag = *(ferret_map + clock_ferret_ptr + ferret_vert_off + ferret_horiz_off + 8);
        if (clock_ferret_ptr + ferret_vert_off + ferret_horiz_off == ferret_targ_ptr) tb_occ_a_flag = 0;
        return *(ferret_map + clock_ferret_ptr + ferret_vert_off + ferret_horiz_off + 2);
    case 4:
        if (ferret_map_hi - 1 <= clock_ferret_y) return -1;
        tb_prev_flag = *(ferret_map + clock_ferret_ptr + ferret_vert_off + 5) & 0x80;
        tb_occ_a_flag = *(ferret_map + clock_ferret_ptr + ferret_vert_off + 7);
        tb_occ_b_flag = *(ferret_map + clock_ferret_ptr + ferret_vert_off + 8);
        if (clock_ferret_ptr + ferret_vert_off == ferret_targ_ptr) tb_occ_a_flag = 0;
        return *(ferret_map + clock_ferret_ptr + ferret_vert_off + 2);
    case 5:
        if (ferret_map_hi - 1 <= clock_ferret_y) return -1;
        if (clock_ferret_x <= 0) return -1;
        tb_prev_flag = *(ferret_map + clock_ferret_ptr + ferret_vert_off - ferret_horiz_off + 5) & 0x80;
        tb_occ_a_flag = *(ferret_map + clock_ferret_ptr + ferret_vert_off - ferret_horiz_off + 7);
        tb_occ_b_flag = *(ferret_map + clock_ferret_ptr + ferret_vert_off - ferret_horiz_off + 8);
        if (clock_ferret_ptr + ferret_vert_off - ferret_horiz_off == ferret_targ_ptr) tb_occ_a_flag = 0;
        return *(ferret_map + clock_ferret_ptr + ferret_vert_off - ferret_horiz_off + 2);
    case 6:
        if (clock_ferret_x <= 0) return -1;
        tb_prev_flag = *(ferret_map + clock_ferret_ptr - ferret_horiz_off + 5) & 0x80;
        tb_occ_a_flag = *(ferret_map + clock_ferret_ptr - ferret_horiz_off + 7);
        tb_occ_b_flag = *(ferret_map + clock_ferret_ptr - ferret_horiz_off + 8);
        if (clock_ferret_ptr - ferret_horiz_off == ferret_targ_ptr) tb_occ_a_flag = 0;
        return *(ferret_map + clock_ferret_ptr - ferret_horiz_off + 2);
    case 7:
        if (clock_ferret_y <= 0) return -1;
        if (clock_ferret_x <= 0) return -1;
        tb_prev_flag = *(ferret_map + clock_ferret_ptr - ferret_vert_off - ferret_horiz_off + 5) & 0x80;
        tb_occ_a_flag = *(ferret_map + clock_ferret_ptr - ferret_vert_off - ferret_horiz_off + 7);
        tb_occ_b_flag = *(ferret_map + clock_ferret_ptr - ferret_vert_off - ferret_horiz_off + 8);
        if (clock_ferret_ptr - ferret_vert_off - ferret_horiz_off == ferret_targ_ptr) tb_occ_a_flag = 0;
        return *(ferret_map + clock_ferret_ptr - ferret_vert_off - ferret_horiz_off + 2);
    }
    return -1;
}

// Moves the clockwise path probe one cell and stamps its accumulated route cost.
// FUNCTION: C2 0x2c70b
// FUNCTION: C2WIN 0x0046c965
void move_clock_ferret(signed char direction, char update_existing)
{
    unsigned char path_value;
    unsigned char road_cost;

    switch (direction) {
    case 0:
        --clock_ferret_y;
        clock_ferret_ptr -= ferret_vert_off;
        break;
    case 1:
        --clock_ferret_y;
        ++clock_ferret_x;
        clock_ferret_ptr -= ferret_vert_off;
        clock_ferret_ptr += ferret_horiz_off;
        break;
    case 2:
        ++clock_ferret_x;
        clock_ferret_ptr += ferret_horiz_off;
        break;
    case 3:
        ++clock_ferret_y;
        ++clock_ferret_x;
        clock_ferret_ptr += ferret_vert_off;
        clock_ferret_ptr += ferret_horiz_off;
        break;
    case 4:
        ++clock_ferret_y;
        clock_ferret_ptr += ferret_vert_off;
        break;
    case 5:
        ++clock_ferret_y;
        --clock_ferret_x;
        clock_ferret_ptr += ferret_vert_off;
        clock_ferret_ptr -= ferret_horiz_off;
        break;
    case 6:
        --clock_ferret_x;
        clock_ferret_ptr -= ferret_horiz_off;
        break;
    case 7:
        --clock_ferret_y;
        --clock_ferret_x;
        clock_ferret_ptr -= ferret_vert_off;
        clock_ferret_ptr -= ferret_horiz_off;
        break;
    }
    path_value = *(ferret_map + clock_ferret_ptr + 2);
    road_cost = *(ferret_map + clock_ferret_ptr + 5) & 3;
    if (path_value == 0) {
        *(ferret_map + clock_ferret_ptr + 2) = clock_ferret_count;
        clock_ferret_count += road_cost;
        clock_ferret_moves++;
    } else if (update_existing != 0) {
        clock_ferret_count = path_value + road_cost;
    }
}

// Reads an anticlockwise probe neighbor's cost, occupancy, and visited state.
// FUNCTION: C2 0x2c883
// FUNCTION: C2WIN 0x0046cb49
signed char check_anti_ferret_move(signed char direction)
{
    switch (direction) {
    case 0:
        if (anti_ferret_y <= 0) return -1;
        tb_prev_flag = *(ferret_map + anti_ferret_ptr - ferret_vert_off + 5) & 0x40;
        tb_occ_a_flag = *(ferret_map + anti_ferret_ptr - ferret_vert_off + 7);
        tb_occ_b_flag = *(ferret_map + anti_ferret_ptr - ferret_vert_off + 8);
        if (anti_ferret_ptr - ferret_vert_off == ferret_targ_ptr) tb_occ_a_flag = 0;
        return *(ferret_map + anti_ferret_ptr - ferret_vert_off + 2);
    case 1:
        if (anti_ferret_y <= 0) return -1;
        if (ferret_map_wi - 1 <= anti_ferret_x) return -1;
        tb_prev_flag = *(ferret_map + anti_ferret_ptr - ferret_vert_off + ferret_horiz_off + 5) & 0x40;
        tb_occ_a_flag = *(ferret_map + anti_ferret_ptr - ferret_vert_off + ferret_horiz_off + 7);
        tb_occ_b_flag = *(ferret_map + anti_ferret_ptr - ferret_vert_off + ferret_horiz_off + 8);
        if (anti_ferret_ptr - ferret_vert_off + ferret_horiz_off == ferret_targ_ptr) tb_occ_a_flag = 0;
        return *(ferret_map + anti_ferret_ptr - ferret_vert_off + ferret_horiz_off + 2);
    case 2:
        if (ferret_map_wi - 1 <= anti_ferret_x) return -1;
        tb_prev_flag = *(ferret_map + anti_ferret_ptr + ferret_horiz_off + 5) & 0x40;
        tb_occ_a_flag = *(ferret_map + anti_ferret_ptr + ferret_horiz_off + 7);
        tb_occ_b_flag = *(ferret_map + anti_ferret_ptr + ferret_horiz_off + 8);
        if (anti_ferret_ptr + ferret_horiz_off == ferret_targ_ptr) tb_occ_a_flag = 0;
        return *(ferret_map + anti_ferret_ptr + ferret_horiz_off + 2);
    case 3:
        if (ferret_map_hi - 1 <= anti_ferret_y) return -1;
        if (ferret_map_wi - 1 <= anti_ferret_x) return -1;
        tb_prev_flag = *(ferret_map + anti_ferret_ptr + ferret_vert_off + ferret_horiz_off + 5) & 0x40;
        tb_occ_a_flag = *(ferret_map + anti_ferret_ptr + ferret_vert_off + ferret_horiz_off + 7);
        tb_occ_b_flag = *(ferret_map + anti_ferret_ptr + ferret_vert_off + ferret_horiz_off + 8);
        if (anti_ferret_ptr + ferret_vert_off + ferret_horiz_off == ferret_targ_ptr) tb_occ_a_flag = 0;
        return *(ferret_map + anti_ferret_ptr + ferret_vert_off + ferret_horiz_off + 2);
    case 4:
        if (ferret_map_hi - 1 <= anti_ferret_y) return -1;
        tb_prev_flag = *(ferret_map + anti_ferret_ptr + ferret_vert_off + 5) & 0x40;
        tb_occ_a_flag = *(ferret_map + anti_ferret_ptr + ferret_vert_off + 7);
        tb_occ_b_flag = *(ferret_map + anti_ferret_ptr + ferret_vert_off + 8);
        if (anti_ferret_ptr + ferret_vert_off == ferret_targ_ptr) tb_occ_a_flag = 0;
        return *(ferret_map + anti_ferret_ptr + ferret_vert_off + 2);
    case 5:
        if (ferret_map_hi - 1 <= anti_ferret_y) return -1;
        if (anti_ferret_x <= 0) return -1;
        tb_prev_flag = *(ferret_map + anti_ferret_ptr + ferret_vert_off - ferret_horiz_off + 5) & 0x40;
        tb_occ_a_flag = *(ferret_map + anti_ferret_ptr + ferret_vert_off - ferret_horiz_off + 7);
        tb_occ_b_flag = *(ferret_map + anti_ferret_ptr + ferret_vert_off - ferret_horiz_off + 8);
        if (anti_ferret_ptr + ferret_vert_off - ferret_horiz_off == ferret_targ_ptr) tb_occ_a_flag = 0;
        return *(ferret_map + anti_ferret_ptr + ferret_vert_off - ferret_horiz_off + 2);
    case 6:
        if (anti_ferret_x <= 0) return -1;
        tb_prev_flag = *(ferret_map + anti_ferret_ptr - ferret_horiz_off + 5) & 0x40;
        tb_occ_a_flag = *(ferret_map + anti_ferret_ptr - ferret_horiz_off + 7);
        tb_occ_b_flag = *(ferret_map + anti_ferret_ptr - ferret_horiz_off + 8);
        if (anti_ferret_ptr - ferret_horiz_off == ferret_targ_ptr) tb_occ_a_flag = 0;
        return *(ferret_map + anti_ferret_ptr - ferret_horiz_off + 2);
    case 7:
        if (anti_ferret_y <= 0) return -1;
        if (anti_ferret_x <= 0) return -1;
        tb_prev_flag = *(ferret_map + anti_ferret_ptr - ferret_vert_off - ferret_horiz_off + 5) & 0x40;
        tb_occ_a_flag = *(ferret_map + anti_ferret_ptr - ferret_vert_off - ferret_horiz_off + 7);
        tb_occ_b_flag = *(ferret_map + anti_ferret_ptr - ferret_vert_off - ferret_horiz_off + 8);
        if (anti_ferret_ptr - ferret_vert_off - ferret_horiz_off == ferret_targ_ptr) tb_occ_a_flag = 0;
        return *(ferret_map + anti_ferret_ptr - ferret_vert_off - ferret_horiz_off + 2);
    }
    return -1;
}

// Moves the anticlockwise path probe one cell and stamps its accumulated route cost.
// FUNCTION: C2 0x2cc73
// FUNCTION: C2WIN 0x0046d179
void move_anti_ferret(signed char direction, char update_existing)
{
    unsigned char path_value;
    unsigned char road_cost;

    switch (direction) {
    case 0:
        --anti_ferret_y;
        anti_ferret_ptr -= ferret_vert_off;
        break;
    case 1:
        --anti_ferret_y;
        ++anti_ferret_x;
        anti_ferret_ptr -= ferret_vert_off;
        anti_ferret_ptr += ferret_horiz_off;
        break;
    case 2:
        ++anti_ferret_x;
        anti_ferret_ptr += ferret_horiz_off;
        break;
    case 3:
        ++anti_ferret_y;
        ++anti_ferret_x;
        anti_ferret_ptr += ferret_vert_off;
        anti_ferret_ptr += ferret_horiz_off;
        break;
    case 4:
        ++anti_ferret_y;
        anti_ferret_ptr += ferret_vert_off;
        break;
    case 5:
        ++anti_ferret_y;
        --anti_ferret_x;
        anti_ferret_ptr += ferret_vert_off;
        anti_ferret_ptr -= ferret_horiz_off;
        break;
    case 6:
        --anti_ferret_x;
        anti_ferret_ptr -= ferret_horiz_off;
        break;
    case 7:
        --anti_ferret_y;
        --anti_ferret_x;
        anti_ferret_ptr -= ferret_vert_off;
        anti_ferret_ptr -= ferret_horiz_off;
        break;
    }
    path_value = *(ferret_map + anti_ferret_ptr + 2);
    road_cost = *(ferret_map + anti_ferret_ptr + 5) & 3;
    if (path_value == 0) {
        *(ferret_map + anti_ferret_ptr + 2) = anti_ferret_count;
        anti_ferret_count += road_cost;
        anti_ferret_moves++;
    } else if (update_existing != 0) {
        anti_ferret_count = path_value + road_cost;
    }
}

// Returns the compass direction from a point toward the ferret target.
// FUNCTION: C2 0x2cdcd
// FUNCTION: C2WIN 0x0046d35d
unsigned char ferret_heading(int current_x, int current_y)
{
    if (current_x > ferret_targ_x) {
        if (current_y > ferret_targ_y) return 7;
        if (current_y == ferret_targ_y) return 6;
        if (current_y < ferret_targ_y) return 5;
    } else if (current_x == ferret_targ_x) {
        if (current_y > ferret_targ_y) return 0;
        if (current_y == ferret_targ_y) return 8;
        if (current_y < ferret_targ_y) return 4;
    } else if (current_x < ferret_targ_x) {
        if (current_y > ferret_targ_y) return 1;
        if (current_y == ferret_targ_y) return 2;
        if (current_y < ferret_targ_y) return 3;
    }
    return 8;
}

// Returns a neighboring path cost and records its road class.
// FUNCTION: C2 0x2ce5b
// FUNCTION: C2WIN 0x0046d47e
unsigned char get_tb_value(int direction)
{
    switch (direction) {
    case 0:
        if (tb_y <= 0) return 0xFF;
        tb_road_flag = *(ferret_map + tb_ptr - ferret_vert_off + 5) & 3;
        return *(ferret_map + tb_ptr - ferret_vert_off + 2);
    case 1:
        if (tb_y <= 0) return 0xFF;
        if (ferret_map_wi - 1 <= tb_x) return 0xFF;
        tb_road_flag = *(ferret_map + tb_ptr - ferret_vert_off + ferret_horiz_off + 5) & 3;
        return *(ferret_map + tb_ptr - ferret_vert_off + ferret_horiz_off + 2);
    case 2:
        if (ferret_map_wi - 1 <= tb_x) return 0xFF;
        tb_road_flag = *(ferret_map + tb_ptr + ferret_horiz_off + 5) & 3;
        return *(ferret_map + tb_ptr + ferret_horiz_off + 2);
    case 3:
        if (ferret_map_hi - 1 <= tb_y) return 0xFF;
        if (ferret_map_wi - 1 <= tb_x) return 0xFF;
        tb_road_flag = *(ferret_map + tb_ptr + ferret_vert_off + ferret_horiz_off + 5) & 3;
        return *(ferret_map + tb_ptr + ferret_vert_off + ferret_horiz_off + 2);
    case 4:
        if (ferret_map_hi - 1 <= tb_y) return 0xFF;
        tb_road_flag = *(ferret_map + tb_ptr + ferret_vert_off + 5) & 3;
        return *(ferret_map + tb_ptr + ferret_vert_off + 2);
    case 5:
        if (ferret_map_hi - 1 <= tb_y) return 0xFF;
        if (tb_x <= 0) return 0xFF;
        tb_road_flag = *(ferret_map + tb_ptr + ferret_vert_off - ferret_horiz_off + 5) & 3;
        return *(ferret_map + tb_ptr + ferret_vert_off - ferret_horiz_off + 2);
    case 6:
        if (tb_x <= 0) return 0xFF;
        tb_road_flag = *(ferret_map + tb_ptr - ferret_horiz_off + 5) & 3;
        return *(ferret_map + tb_ptr - ferret_horiz_off + 2);
    case 7:
        if (tb_y <= 0) return 0xFF;
        if (tb_x <= 0) return 0xFF;
        tb_road_flag = *(ferret_map + tb_ptr - ferret_vert_off - ferret_horiz_off + 5) & 3;
        return *(ferret_map + tb_ptr - ferret_vert_off - ferret_horiz_off + 2);
    }
    return 0xFF;
}

// Returns a neighboring path marker, rejecting blocked and out-of-bounds cells.
// FUNCTION: C2 0x2cfef
// FUNCTION: C2WIN 0x0046d7dd
unsigned char get_ferret2(int direction)
{
    switch (direction) {
    case 0:
        if (tb_y <= 0) return 0xFF;
        if (*(ferret_map + tb_ptr - ferret_vert_off + 2) == 0xFE) return 0xFF;
        return *(ferret_map + tb_ptr - ferret_vert_off + 6);
    case 1:
        if (tb_y <= 0) return 0xFF;
        if (ferret_map_wi - 1 <= tb_x) return 0xFF;
        if (*(ferret_map + tb_ptr - ferret_vert_off + ferret_horiz_off + 2) == 0xFE) return 0xFF;
        return *(ferret_map + tb_ptr - ferret_vert_off + ferret_horiz_off + 6);
    case 2:
        if (ferret_map_wi - 1 <= tb_x) return 0xFF;
        if (*(ferret_map + tb_ptr + ferret_horiz_off + 2) == 0xFE) return 0xFF;
        return *(ferret_map + tb_ptr + ferret_horiz_off + 6);
    case 3:
        if (ferret_map_hi - 1 <= tb_y) return 0xFF;
        if (ferret_map_wi - 1 <= tb_x) return 0xFF;
        if (*(ferret_map + tb_ptr + ferret_vert_off + ferret_horiz_off + 2) == 0xFE) return 0xFF;
        return *(ferret_map + tb_ptr + ferret_vert_off + ferret_horiz_off + 6);
    case 4:
        if (ferret_map_hi - 1 <= tb_y) return 0xFF;
        if (*(ferret_map + tb_ptr + ferret_vert_off + 2) == 0xFE) return 0xFF;
        return *(ferret_map + tb_ptr + ferret_vert_off + 6);
    case 5:
        if (ferret_map_hi - 1 <= tb_y) return 0xFF;
        if (tb_x <= 0) return 0xFF;
        if (*(ferret_map + tb_ptr + ferret_vert_off - ferret_horiz_off + 2) == 0xFE) return 0xFF;
        return *(ferret_map + tb_ptr + ferret_vert_off - ferret_horiz_off + 6);
    case 6:
        if (tb_x <= 0) return 0xFF;
        if (*(ferret_map + tb_ptr - ferret_horiz_off + 2) == 0xFE) return 0xFF;
        return *(ferret_map + tb_ptr - ferret_horiz_off + 6);
    case 7:
        if (tb_y <= 0) return 0xFF;
        if (tb_x <= 0) return 0xFF;
        if (*(ferret_map + tb_ptr - ferret_vert_off - ferret_horiz_off + 2) == 0xFE) return 0xFF;
        return *(ferret_map + tb_ptr - ferret_vert_off - ferret_horiz_off + 6);
    }
    return 0xFF;
}

// Advances the current ferret trace and marks the destination cell.
// FUNCTION: C2 0x2d1ef
// FUNCTION: C2WIN 0x0046db8c
void move_to_tb_value(int direction)
{
    switch (direction) {
    case 0:
        --tb_y;
        tb_ptr -= ferret_vert_off;
        break;
    case 1:
        --tb_y;
        ++tb_x;
        tb_ptr -= ferret_vert_off;
        tb_ptr += ferret_horiz_off;
        break;
    case 2:
        ++tb_x;
        tb_ptr += ferret_horiz_off;
        break;
    case 3:
        ++tb_y;
        ++tb_x;
        tb_ptr += ferret_vert_off;
        tb_ptr += ferret_horiz_off;
        break;
    case 4:
        ++tb_y;
        tb_ptr += ferret_vert_off;
        break;
    case 5:
        ++tb_y;
        --tb_x;
        tb_ptr += ferret_vert_off;
        tb_ptr -= ferret_horiz_off;
        break;
    case 6:
        --tb_x;
        tb_ptr -= ferret_horiz_off;
        break;
    case 7:
        --tb_y;
        --tb_x;
        tb_ptr -= ferret_vert_off;
        tb_ptr -= ferret_horiz_off;
        break;
    }
    *(ferret_map + tb_ptr + 6) = 1;
}

// Converts the overview-map pointer offset into map coordinates.
// FUNCTION: C2 0x2d305
// FUNCTION: C2WIN 0x0046dd00
void get_over_coords(void)
{
    over_ptr = pm_over_cm_ptr / map_actual_atom;
    over_x = over_ptr % map_actual_width;
    over_y = over_ptr / map_actual_width;
}

// Returns whether the current overview coordinates lie on a map edge.
// FUNCTION: C2 0x2d349
// FUNCTION: C2WIN 0x0046dd3f
int at_edge_of_map(int map_x, int map_y)
{
    if (map_x <= 0) return 1;
    if (map_x >= map_actual_width - 1) return 1;
    if (map_y <= 0) return 1;
    if (map_y >= map_actual_height - 1) return 1;
    return 0;
}

// Finds the army under the overview pointer or occupying an adjacent home cell.
// FUNCTION: C2 0x2d372
// FUNCTION: C2WIN 0x0046ddab
void get_over_army(void)
{
    int start_y;
    int start_x;
    int end_y;
    int end_x;
    int cell_offset;
    int army_idx;

    over_an_army = 0;
    if (map_mode != 1 || pointer_mode != 0 || pm_over == 0)
        return;
    if ((RM_CELL(pm_over_cm_ptr).terrain & 1) != 0)
        return;

    army_a = (unsigned char)RM_CELL(pm_over_cm_ptr).occupant;
    if (army_a != 0 && pm_over_cm_ptr == army_list[army_a].map_ref) {
        over_an_army = army_a;
        return;
    }

    start_x = over_x - 1;
    if (start_x < 0) start_x = 0;
    start_y = over_y - 1;
    if (start_y < 0) start_y = 0;
    end_x = over_x + 1;
    if (end_x >= map_actual_width) end_x = map_actual_width - 1;
    end_y = over_y + 1;
    if (end_y >= map_actual_height) end_y = map_actual_height - 1;

    for (; start_y <= end_y; start_y++) {
        int cell_x;
        for (cell_x = start_x; cell_x <= end_x; cell_x++) {
            cell_offset = (map_actual_width * start_y + cell_x) * map_actual_atom;
            army_a = (unsigned char)RM_CELL(cell_offset).occupant;
            if ((RM_CELL(cell_offset).terrain & 1) == 0
                && army_a != 0
                && army_a >= 0) {
                army_idx = army_a;
                if (army_idx < 26 && army_list[army_idx].home_ref == pm_over_cm_ptr) {
                    over_an_army = army_idx;
                }
            }
        }
    }
}
