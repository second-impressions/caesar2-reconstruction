
#include "c2_data.h"

#if PLATFORM_WINDOWS
extern void *main_window;
extern unsigned char battle_intro_result;
extern int show_native_battle_intro(void *window);
extern void show_native_battle_outtro(void *window);
#endif

/* File-local state. */
int revolt_size;
int barb_ptr;
int barb_entry_count;
int barb_x;
int barb_y;
/* Forward declarations (functions defined later in this file). */
void get_random_start_points_from_dirc(int direction, int map_size, int random_mask);
void do_land_trade(int direction, int cargo, int border_x, int border_y);
void battle_intro(void);
void battle_outtro(void);
void we_surrender(void);
void we_retreat(void);
void battle_auto_resolve(void);
void do_battle_victory(void);

// No-op hook for city-specific trouble.
// FUNCTION: C2 0x52e1b
// FUNCTION: C2WIN 0x00401384
void city_trouble(void)
{
}

// Attempts the monthly revolt, raid, horde, and war events in order.
// FUNCTION: C2 0x52e1c
// FUNCTION: C2WIN 0x0046e7c3
void region_trouble(void)
{
    if (c2inf.peace_mode != 0) return;
    if (revolt_trouble()) return;
    if (raider_trouble()) return;
    if (horde_trouble()) return;
    if (war_trouble()) return;
}

// Resets the cooldown counters for regional trouble events.
// FUNCTION: C2 0x52fa2
// FUNCTION: C2WIN 0x0046e82a
void init_region_trouble(void)
{
    months_since_last_war           = 0;
    months_since_last_horde         = 0;
    months_since_last_raider        = 0;
    months_since_last_revolt        = 0;
    months_since_last_city_attack   = 0;
}

// Attempts a revolt and initializes the resulting rebel army.
// FUNCTION: C2 0x52fc5
// FUNCTION: C2WIN 0x0046e867
int revolt_trouble(void)
{
    months_since_last_revolt++;
    if (chance_of_attack(0, months_since_last_revolt, 0, 0) == 0) return 0;
    if (revolt_in_region(0, 0)) {
        army_list[created_army_no].source_region = province_is;
        army_list[created_army_no].tribe_id = tribe_type[province_is];

        army_list[created_army_no].num_specials      = tribe_to_troop_numbers[army_list[created_army_no].tribe_id].specials * revolt_size;
        army_list[created_army_no].num_horse     = tribe_to_troop_numbers[army_list[created_army_no].tribe_id].horse * revolt_size;
        army_list[created_army_no].num_regulars      = tribe_to_troop_numbers[army_list[created_army_no].tribe_id].regulars * revolt_size;
        army_list[created_army_no].num_irregulars    = tribe_to_troop_numbers[army_list[created_army_no].tribe_id].irregulars * revolt_size;
        army_list[created_army_no].num_auxillaries   = tribe_to_troop_numbers[army_list[created_army_no].tribe_id].auxiliaries * revolt_size;
        army_list[created_army_no].total_troops = army_list[created_army_no].num_specials + army_list[created_army_no].num_horse +
                             army_list[created_army_no].num_regulars + army_list[created_army_no].num_irregulars +
                             army_list[created_army_no].num_auxillaries;

        set_sound("uprise.wav", 1);
        pax_romanum -= 6;
        if (pax_romanum < 0) pax_romanum = 0;
        months_since_last_revolt = 0;
        return 1;
    }
    return 0;
}

// Attempts a raider attack and initializes the resulting army.
// FUNCTION: C2 0x53132
// FUNCTION: C2WIN 0x0046eb23
int raider_trouble(void)
{
    months_since_last_raider++;

    if (chance_of_attack(1, months_since_last_raider, 0, 0) == 0) return 0;
    if (raider_in_region(attack_direction, attack_from_sea)) {

        army_list[created_army_no].source_region = attacking_region;
        army_list[created_army_no].tribe_id = tribe_type[attacking_region];

        army_list[created_army_no].num_specials    = tribe_to_troop_numbers[army_list[created_army_no].tribe_id].specials * 3;
        army_list[created_army_no].num_horse   = tribe_to_troop_numbers[army_list[created_army_no].tribe_id].horse * 3;
        army_list[created_army_no].num_regulars    = tribe_to_troop_numbers[army_list[created_army_no].tribe_id].regulars * 3;
        army_list[created_army_no].num_irregulars  = tribe_to_troop_numbers[army_list[created_army_no].tribe_id].irregulars * 3;
        army_list[created_army_no].num_auxillaries = tribe_to_troop_numbers[army_list[created_army_no].tribe_id].auxiliaries * 3;

        army_list[created_army_no].total_troops = army_list[created_army_no].num_specials + army_list[created_army_no].num_horse
                     + army_list[created_army_no].num_regulars + army_list[created_army_no].num_irregulars
                     + army_list[created_army_no].num_auxillaries;

        set_sound("marchb2.wav", 1);

        pax_romanum -= 12;
        if (pax_romanum < 0) pax_romanum = 0;
        months_since_last_raider = 0;
        return 1;
    }
    return 0;
}

// Attempts a barbarian horde attack and initializes the resulting army.
// FUNCTION: C2 0x5329d
// FUNCTION: C2WIN 0x0046edd3
int horde_trouble(void)
{
    months_since_last_horde++;

    if (chance_of_attack(2, months_since_last_horde, 0, 0) == 0) return 0;
    if (barbarian_in_region(attack_direction, attack_from_sea)) {

        army_list[created_army_no].source_region = attacking_region;
        army_list[created_army_no].tribe_id = tribe_type[attacking_region];

        army_list[created_army_no].num_specials    = tribe_to_troop_numbers[army_list[created_army_no].tribe_id].specials * 6;
        army_list[created_army_no].num_horse   = tribe_to_troop_numbers[army_list[created_army_no].tribe_id].horse * 6;
        army_list[created_army_no].num_regulars    = tribe_to_troop_numbers[army_list[created_army_no].tribe_id].regulars * 6;
        army_list[created_army_no].num_irregulars  = tribe_to_troop_numbers[army_list[created_army_no].tribe_id].irregulars * 6;
        army_list[created_army_no].num_auxillaries = tribe_to_troop_numbers[army_list[created_army_no].tribe_id].auxiliaries * 6;

        army_list[created_army_no].total_troops = army_list[created_army_no].num_specials + army_list[created_army_no].num_horse
                     + army_list[created_army_no].num_regulars + army_list[created_army_no].num_irregulars
                     + army_list[created_army_no].num_auxillaries;

        set_sound("marchb2.wav", 1);

        pax_romanum -= 18;
        if (pax_romanum < 0) pax_romanum = 0;
        months_since_last_horde = 0;
        return 1;
    }
    return 0;
}

// Attempts a foreign invasion and initializes the resulting army.
// FUNCTION: C2 0x52e40 REORDERED
// FUNCTION: C2WIN 0x0046f08d
int war_trouble(void)
{
    months_since_last_war++;

    if (chance_of_attack(3, months_since_last_war, 0, 1) == 0) return 0;
    if (empire_in_region(attack_direction, attack_from_sea)) {

        army_list[created_army_no].source_region = attacking_region;
        army_list[created_army_no].tribe_id = tribe_type[attacking_region];

        army_list[created_army_no].num_specials    = tribe_to_troop_numbers[army_list[created_army_no].tribe_id].specials * 8;
        army_list[created_army_no].num_horse   = tribe_to_troop_numbers[army_list[created_army_no].tribe_id].horse * 8;
        army_list[created_army_no].num_regulars    = tribe_to_troop_numbers[army_list[created_army_no].tribe_id].regulars * 8;
        army_list[created_army_no].num_irregulars  = tribe_to_troop_numbers[army_list[created_army_no].tribe_id].irregulars * 8;
        army_list[created_army_no].num_auxillaries = tribe_to_troop_numbers[army_list[created_army_no].tribe_id].auxiliaries * 8;

        army_list[created_army_no].total_troops = army_list[created_army_no].num_specials + army_list[created_army_no].num_horse
                     + army_list[created_army_no].num_regulars + army_list[created_army_no].num_irregulars
                     + army_list[created_army_no].num_auxillaries;

        set_sound("marchb2.wav", 1);

        pax_romanum -= 24;
        if (pax_romanum < 0) pax_romanum = 0;
        months_since_last_war = 0;
        return 1;
    }
    return 0;
}

// Tests an attack event's timing and chance, and optionally selects its origin.
// FUNCTION: C2 0x5341a
// FUNCTION: C2WIN 0x0046f33d
int chance_of_attack(int trouble_type, int months_since,
                     int probe_only, int p4)
{
    int honeymoon;
    int frequency;
    int debar;

    attack_from_sea = 0;
    honeymoon = skill_to_trouble_honeymoons[c2inf.skill_level][trouble_type];
    frequency = skill_to_trouble_frequency[c2inf.skill_level][trouble_type];
    debar = skill_to_trouble_debar[c2inf.skill_level][trouble_type];

    if (honeymoon > years_elapsed_in_region) return 0;
    if (months_since <= debar) return 0;
    random();
    if (rand128 < 20)                        return 0;
    if (frequency + 20 <= rand128)           return 0;

    if (probe_only != 0) return 1;
    attack_direction = get_attackers(rand128 & 3, p4);
    if (attack_direction >= 8) return 0;
    if (attack_direction == 0 && north_trader_is == 1) attack_from_sea = 1;
    if (attack_direction == 2 && east_trader_is  == 1) attack_from_sea = 1;
    if (attack_direction == 4 && south_trader_is == 1) attack_from_sea = 1;
    if (attack_direction == 6 && west_trader_is  == 1) attack_from_sea = 1;
    return 1;
}

// Selects an eligible neighboring region and returns its compass direction.
// FUNCTION: C2 0x534fd
// FUNCTION: C2WIN 0x0046f4d0
int get_attackers(int dir, int scan_all)
{
    int kind;
    int i;

    if (scan_all) {
        /* Scan all four borders; return the first one whose kind is a
           known hostile category (6/f/12/22) and whose empire entry is
           not the friendly value (6). */
        for (i = 0; i < 4; i++) {
            kind = region_borders[province_is].u.dir[i];
            if (kind == 6 || kind == 0xf || kind == 0x12 || kind == 0x22) {
                if (empire[kind] != 6) {
                    attacking_region = kind;
                    return i * 2;
                }
            }
        }
    }
    else {
        /* Single-direction probe with the opposite eligibility test:
           the four "hostile" kinds (6/f/12/22) are rejected here, only
           neighbours outside that set are candidates. */
        kind = region_borders[province_is].u.dir[dir];
        if (kind == 6 || kind == 0xf || kind == 0x12 || kind == 0x22) return 8;
        if (empire[kind] != 6) {
            attacking_region = kind;
            return dir * 2;
        }
    }
    return 8;
}

// Spawns a raider army at a valid land or sea invasion point.
// FUNCTION: C2 0x5358c
// FUNCTION: C2WIN 0x0046f5e3
int raider_in_region(int direction, int from_sea)
{
    int message_map_ref;

    if (get_region_invasion_points(direction, from_sea) == 0) return 0;

    if (from_sea != 0) {
        if (create_army(4, barb_x, barb_y, 1) == 0) return 0;
        army_list[created_army_no].state_idx        = 0xe;
        army_list[created_army_no].wait_count       = 0;
        army_list[created_army_no].morale = 3;
        army_list[created_army_no].saved_state_idx  = 6;
        army_list[created_army_no].army_id = (rand128 & 1);
    } else {
        if (create_army(4, barb_x, barb_y, 2) == 0) return 0;
        army_list[created_army_no].state_idx        = 1;
        army_list[created_army_no].wait_count       = 0x14;
        army_list[created_army_no].morale = 3;
        army_list[created_army_no].saved_state_idx  = 6;
    }

    army_list[created_army_no].target_y = 0;
    army_list[created_army_no].target_x = 0;

    message_map_ref = army_list[created_army_no].map_ref;
    if (message_map_ref == 0) message_map_ref = 8;
    put_message(0x5b, message_map_ref, 0x11);
    return 1;
}

// Spawns a barbarian army at a valid land or sea invasion point.
// FUNCTION: C2 0x53688
// FUNCTION: C2WIN 0x0046f7fd
int barbarian_in_region(int direction, int from_sea)
{
    int message_map_ref;

    if (get_region_invasion_points(direction, from_sea) == 0) return 0;

    if (from_sea != 0) {
        if (create_army(3, barb_x, barb_y, 1) == 0) return 0;
        army_list[created_army_no].state_idx        = 0xe;
        army_list[created_army_no].wait_count       = 0;
        army_list[created_army_no].morale = 4;
        army_list[created_army_no].saved_state_idx  = 7;
        army_list[created_army_no].army_id = (rand128 & 1);
    } else {
        if (create_army(3, barb_x, barb_y, 2) == 0) return 0;
        army_list[created_army_no].state_idx        = 1;
        army_list[created_army_no].wait_count       = 0x14;
        army_list[created_army_no].morale = 4;
        army_list[created_army_no].saved_state_idx  = 7;
    }

    message_map_ref = army_list[created_army_no].map_ref;
    message_map_ref = message_map_ref == 0 ? 8 : message_map_ref;
    put_message(0x5d, message_map_ref, 0x11);
    return 1;
}

// Spawns a hostile empire army at a valid land or sea invasion point.
// FUNCTION: C2 0x53776
// FUNCTION: C2WIN 0x0046f9ce
int empire_in_region(int direction, int from_sea)
{
    int message_map_ref;

    if (get_region_invasion_points(direction, from_sea) == 0) return 0;

    if (from_sea != 0) {
        if (create_army(2, barb_x, barb_y, 1) == 0) return 0;
        army_list[created_army_no].state_idx        = 0xe;
        army_list[created_army_no].wait_count       = 0;
        army_list[created_army_no].morale = 4;
        army_list[created_army_no].saved_state_idx  = 7;
        army_list[created_army_no].army_id = (rand128 & 1);
    } else {
        if (create_army(2, barb_x, barb_y, 2) == 0) return 0;
        army_list[created_army_no].state_idx        = 1;
        army_list[created_army_no].wait_count       = 0x14;
        army_list[created_army_no].morale = 4;
        army_list[created_army_no].saved_state_idx  = 7;
    }

    message_map_ref = army_list[created_army_no].map_ref;
    message_map_ref = message_map_ref == 0 ? 8 : message_map_ref;
    put_message(0x5e, message_map_ref, 0x11);
    return 1;
}

// Selects a revolt point and spawns a rebel army there.
// FUNCTION: C2 0x53861
// FUNCTION: C2WIN 0x0046fb9f
int revolt_in_region(int direction, int from_sea)
{
    (void)direction;
    (void)from_sea;
    revolt_size = get_region_revolt_points();
    if (revolt_size == 0) return 0;
    if (create_army(5, barb_x, barb_y, 2) == 0) return 0;
    army_list[created_army_no].state_idx       = 1;
    army_list[created_army_no].wait_count      = 0x14;
    army_list[created_army_no].morale          = 2;
    army_list[created_army_no].saved_state_idx = 7;
    put_message(0x5c, army_list[created_army_no].map_ref, 0x11);
    return 1;
}

// Searches random map-edge positions for a valid invasion point.
// FUNCTION: C2 0x538d6
// FUNCTION: C2WIN 0x0046fc82
int get_region_invasion_points(int direction, int from_sea)
{
    int i;
    unsigned char t;

    i = 0;
    while (i++ < 20) {
        get_random_start_points_from_dirc(direction, 0x3c, 0x3f);
        barb_ptr = (barb_x + barb_y * REGION_W) * REGION_CELL_BYTES;
        t = (*(struct region_cell *)((unsigned char *)region_map + (barb_ptr))).terrain;
        if (from_sea != 0) {
            if ((t & 8) != 0)
                return 1;
        } else {
            if ((t & 0x1c) == 0) {
                clear_a_reg_area(barb_x, barb_y, barb_x, barb_y, 1);
                return 1;
            }
        }
    }
    return 0;
}

// Selects a rebel hut and an adjacent clear tile for a revolt spawn.
// FUNCTION: C2 0x5395c
// FUNCTION: C2WIN 0x0046fd4f
int get_region_revolt_points(void)
{
    int hut_no;
    int hut_offset;
    unsigned char hut_kind;
    unsigned char occupant;
    unsigned char square;

    hut_no = rand128 & 3;
    hut_offset = hut_list[hut_no].x * REGION_CELL_BYTES;
    hut_offset += hut_list[hut_no].y * REGION_W * REGION_CELL_BYTES;
    hut_kind = RM_CELL(hut_offset).base_kind;
    if (hut_kind >= 0x93 && hut_kind <= 0x96) {
        square   = (*(struct region_cell *)((unsigned char *)region_map + (barb_ptr - 480))).terrain;
        occupant = (*(struct region_cell *)((unsigned char *)region_map + (barb_ptr - 480))).occupant;
        if ((square & 8) == 0 && occupant == 0) { barb_x = hut_list[hut_no].x; barb_y = hut_list[hut_no].y - 1; return hut_kind - 0x92; }
        square   = (*(struct region_cell *)((unsigned char *)region_map + (barb_ptr + 8))).terrain;
        occupant = (*(struct region_cell *)((unsigned char *)region_map + (barb_ptr + 8))).occupant;
        if ((square & 8) == 0 && occupant == 0) { barb_x = hut_list[hut_no].x + 1; barb_y = hut_list[hut_no].y; return hut_kind - 0x92; }
        square   = (*(struct region_cell *)((unsigned char *)region_map + (barb_ptr + 480))).terrain;
        occupant = (*(struct region_cell *)((unsigned char *)region_map + (barb_ptr + 480))).occupant;
        if ((square & 8) == 0 && occupant == 0) { barb_x = hut_list[hut_no].x; barb_y = hut_list[hut_no].y + 1; return hut_kind - 0x92; }
        square   = (*(struct region_cell *)((unsigned char *)region_map + (barb_ptr - 8))).terrain;
        occupant = (*(struct region_cell *)((unsigned char *)region_map + (barb_ptr - 8))).occupant;
        if ((square & 8) == 0 && occupant == 0) { barb_x = hut_list[hut_no].x - 1; barb_y = hut_list[hut_no].y; return hut_kind - 0x92; }
    }
    return 0;
}


// Spawns a city invasion wave sized from the attacking army's strength.
// FUNCTION: C2 0x53ac3
// FUNCTION: C2WIN 0x0046ff71
int barbarian_invades_city(int army_idx)
{
    int   invader_count;
    int   direction;
    int   placed_count;
    int   attempts;
    unsigned char terrain;

    if      (army_list[army_idx].total_troops >= 0x320) invader_count = 9;
    else if (army_list[army_idx].total_troops >= 0x258) invader_count = 7;
    else if (army_list[army_idx].total_troops >= 0x190) invader_count = 5;
    else if (army_list[army_idx].total_troops >=  0xc8) invader_count = 3;
    else                                                invader_count = 2;

    direction = (army_list[army_idx].world_dir + 4) % 8;

    for (placed_count = 0; placed_count < invader_count; placed_count++) {
        for (attempts = 0; attempts++ < 10; ) {
            get_random_start_points_from_dirc(direction, 0x50, 0x3f);
            barb_ptr = (barb_y * CITY_W + barb_x) * CITY_CELL_BYTES;
            terrain = (*(struct city_cell *)((unsigned char *)city_map + (barb_ptr))).terrain;
            if (terrain & 0xe7) {
                clear_an_area(barb_x, barb_y, barb_x, barb_y);
            }
            if (create_citizen(3, barb_x, barb_y, 0) == 0) goto finished;
            break;
        }

        citizen_list[created_citizen_no].state_idx       = 1;
        citizen_list[created_citizen_no].saved_state_idx = 5;
        citizen_list[created_citizen_no].wait_count      = 0x14;
        citizen_list[created_citizen_no].dest_x          = top_lv_x;
        citizen_list[created_citizen_no].dest_y          = top_lv_y;
    }

finished:
    if (placed_count == 0) return 0;

    put_message(0x53, citizen_list[created_citizen_no].map_ref, 0x17);

    pax_romanum -= 0x40;
    if (pax_romanum < 0) pax_romanum = 0;
    return 1;
}

// Drops a requested number of barbarian citizens along a city-map edge.
// FUNCTION: C2 0x53c43
// FUNCTION: C2WIN 0x00470206
void barbarians_drop_by_city(int direction, int invader_count)
{
    int placed_count;
    int attempts;
    unsigned char terrain;

    for (placed_count = 0; placed_count < invader_count; placed_count++) {
        for (attempts = 0; attempts++ < 10; ) {
            get_random_start_points_from_dirc(direction, 0x50, 0x3f);
            barb_ptr = (barb_y * CITY_W + barb_x) * CITY_CELL_BYTES;
            terrain = (*(struct city_cell *)((unsigned char *)city_map + (barb_ptr))).terrain;
            if (terrain & 0xe7) {
                clear_an_area(barb_x, barb_y, barb_x, barb_y);
            }
            if (create_citizen(3, barb_x, barb_y, 0) == 0) goto finished;
            break;
        }
        citizen_list[created_citizen_no].state_idx       = 1;
        citizen_list[created_citizen_no].saved_state_idx = 5;
        citizen_list[created_citizen_no].wait_count      = 0x14;
        citizen_list[created_citizen_no].dest_x          = top_lv_x;
        citizen_list[created_citizen_no].dest_y          = top_lv_y;
    }

finished:
    if (placed_count != 0) {
        put_message(0x53, citizen_list[created_citizen_no].map_ref, 0x17);
        pax_romanum -= 0x40;
        if (pax_romanum < 0) pax_romanum = 0;
    }
}

// Chooses a pseudo-random point on the map edge indicated by `direction`.
// FUNCTION: C2 0x53d4e
// FUNCTION: C2WIN 0x004703be
void get_random_start_points_from_dirc(int direction, int map_size, int random_mask)
{
    int wrapped_flag;
    int random_offset;
    int edge_x;
    int edge_y;

    barb_entry_count++;
    random_offset = (rand128 + barb_entry_count) & random_mask;
    edge_x = (map_size - random_mask) / 2;
    if (edge_x < 0) edge_x = 0;
    edge_y = map_size - random_mask / 4;
    if (edge_y < 0) edge_y = 0;
    edge_x += random_offset;
    edge_y += random_offset / 2;
    wrapped_flag = 0;
    if (edge_x >= map_size) edge_x /= 2;
    if (edge_y >= map_size) { edge_y -= map_size; wrapped_flag = 1; }

    if (direction == 0) { barb_x = edge_x; barb_y = 0; }
    else if (direction == 1) {
        if (wrapped_flag) { barb_x = map_size - 1; barb_y = edge_y; }
        else { barb_x = edge_y; barb_y = 0; }
    } else if (direction == 2) { barb_x = map_size - 1; barb_y = edge_x; }
    else if (direction == 3) {
        if (wrapped_flag) { barb_x = map_size - 1 - edge_y; barb_y = map_size - 1; }
        else { barb_x = map_size - 1; barb_y = edge_y; }
    } else if (direction == 4) { barb_x = edge_x; barb_y = map_size - 1; }
    else if (direction == 5) {
        if (wrapped_flag) { barb_x = 0; barb_y = map_size - 1 - edge_y; }
        else { barb_x = map_size - 1 - edge_y; barb_y = map_size - 1; }
    } else if (direction == 6) { barb_x = 0; barb_y = edge_x; }
    else if (direction == 7) {
        if (wrapped_flag) { barb_x = edge_y; barb_y = 0; }
        else { barb_x = 0; barb_y = map_size - 1 - edge_y; }
    }
}

// Advances `temp_army` to the next eligible cohort in round-robin order.
// FUNCTION: C2 0x53e8e
// FUNCTION: C2WIN 0x00470607
int get_next_temp_cohort(int strict)
{
    int retries;

    for (retries = 0; retries < 26; retries++) {
        temp_army++;
        if (temp_army >= 26)
            temp_army = 1;

        if (army_list[temp_army].exists != 0 &&
            army_list[temp_army].type == 1) {
            if (strict != 0) {
                if (army_list[temp_army].state_idx == 10) continue;
                if (army_list[temp_army].target_timer == 0) continue;
            }
            return 1;
        }
    }
    return 0;
}

// Rebuilds the active-cohort bitmap, count, and next free cohort slot.
// FUNCTION: C2 0x53f0c
// FUNCTION: C2WIN 0x004706fe
void get_cohorts_in_action(void)
{
    int i;

    i = 0;
    no_of_cohorts_in_action = 0;
    for (; i < 10; i++) cohort_in_action[i] = 0;

    for (army_no = 1; army_no < 26; army_no++) {
        if (army_list[army_no].exists != 0 &&
            army_list[army_no].type   == 1) {
            if (army_list[army_no].cohort_id >= 10) {
                test_beeps();
                continue;
            }
            cohort_in_action[army_list[army_no].cohort_id] = 1;
            no_of_cohorts_in_action++;
        }
    }

    i = 0;
    next_cohort_free = 0;
    for (; i < 10; i++, next_cohort_free++) {
        if (cohort_in_action[i] == 0) break;
    }
}

// Selects the previous or next active cohort in the forum view.
// FUNCTION: C2 0x53f9d
// FUNCTION: C2WIN 0x00470843
void get_next_viewed_cohort(int direction)
{
    int attempts;

    get_cohorts_in_action();
    for (attempts = 0; attempts < 11; attempts++) {
        if (direction == 0) {
            --forum_viewed_army;
        } else {
            ++forum_viewed_army;
        }
        if (forum_viewed_army <  0) forum_viewed_army = 10;
        if (forum_viewed_army > 10) forum_viewed_army = 0;
        if (forum_viewed_army == 10) return;
        if (cohort_in_action[forum_viewed_army] == 1) return;
    }
    forum_viewed_army = 10;
}

// Ensures the forum's selected cohort is still active.
// FUNCTION: C2 0x54012
// FUNCTION: C2WIN 0x004708f4
void check_viewed_cohort(void)
{
    get_cohorts_in_action();
    if (forum_viewed_army <  0) forum_viewed_army = 10;
    if (forum_viewed_army > 10) forum_viewed_army = 0;
    if (forum_viewed_army == 10) return;
    if (cohort_in_action[forum_viewed_army] == 1) return;
    get_next_viewed_cohort(1);
}

// Resolves the cohort selected in the forum to its army-list index.
// FUNCTION: C2 0x54065
// FUNCTION: C2WIN 0x00470969
int get_actual_viewed_army(void)
{
    int i;

    i = 0;
    if (tracking_army != 0) {
        if (army_list[tracking_army].type == 1)
            return tracking_army;
    }
    for (temp_army = 0; temp_army < 26; ++temp_army) {
        if (army_list[temp_army].exists != 0
            && army_list[temp_army].type == 1
            && army_list[temp_army].cohort_id == forum_viewed_army)
        {
            return temp_army;
        }
    }
    forum_viewed_army = 0xa;
    return 0;
}

// Initialize sea traders for each border using the neighbouring province's primary resource.
// FUNCTION: C2 0x540e4
// FUNCTION: C2WIN 0x00470a6a
void init_traders(void)
{
    int neighbor_region;

    north_trader_count0 = 2;
    north_trader_count1 = 0xa;
    east_trader_count0  = 4;
    east_trader_count1  = 0xc;
    south_trader_count0 = 6;
    south_trader_count1 = 0xe;
    west_trader_count0  = 8;
    west_trader_count1  = 0x10;

    neighbor_region = region_borders[province_is].u.side.north;
    north_trader_brings = region_sources[neighbor_region].primary;
    neighbor_region = region_borders[province_is].u.side.east;
    east_trader_brings  = region_sources[neighbor_region].primary;
    neighbor_region = region_borders[province_is].u.side.south;
    south_trader_brings = region_sources[neighbor_region].primary;
    neighbor_region = region_borders[province_is].u.side.west;
    west_trader_brings  = region_sources[neighbor_region].primary;
}

// Advances the border trade timers and launches due land or sea traders.
// FUNCTION: C2 0x541c8
// FUNCTION: C2WIN 0x00470b60
void launch_traders(void)
{
    /* ---- North ---- */
    north_trader_count0--;
    if (north_trader_count0 <= 0) {
        if (north_trader_is == 0) {
            do_land_trade(0, north_trader_brings,
                          north_border_x, north_border_y);
            north_trader_count0 = 4;
        } else if (do_sea_trade(0, north_trader_brings,
                                north_border_x, north_border_y, 0)) {
            north_trader_count0 = 0xf423f;
        } else {
            north_trader_count0 = 2;
        }
    }
    north_trader_count1--;
    if (north_trader_count1 <= 0 && north_trader_is != 0) {
        if (do_sea_trade(0, north_trader_brings,
                         north_border_x, north_border_y, 1)) {
            north_trader_count1 = 0xf423f;
        } else {
            north_trader_count1 = 2;
        }
    }

    /* ---- East ---- */
    east_trader_count0--;
    if (east_trader_count0 <= 0) {
        if (east_trader_is == 0) {
            do_land_trade(2, east_trader_brings,
                          east_border_x, east_border_y);
            east_trader_count0 = 4;
        } else if (do_sea_trade(2, east_trader_brings,
                                east_border_x, east_border_y, 0)) {
            east_trader_count0 = 0xf423f;
        } else {
            east_trader_count0 = 2;
        }
    }
    east_trader_count1--;
    if (east_trader_count1 <= 0 && east_trader_is != 0) {
        if (do_sea_trade(2, east_trader_brings,
                         east_border_x, east_border_y, 1)) {
            east_trader_count1 = 0xf423f;
        } else {
            east_trader_count1 = 2;
        }
    }

    /* ---- South ---- */
    south_trader_count0--;
    if (south_trader_count0 <= 0) {
        if (south_trader_is == 0) {
            do_land_trade(4, south_trader_brings,
                          south_border_x, south_border_y);
            south_trader_count0 = 4;
        } else if (do_sea_trade(4, south_trader_brings,
                                south_border_x, south_border_y, 0)) {
            south_trader_count0 = 0xf423f;
        } else {
            south_trader_count0 = 2;
        }
    }
    south_trader_count1--;
    if (south_trader_count1 <= 0 && south_trader_is != 0) {
        if (do_sea_trade(4, south_trader_brings,
                         south_border_x, south_border_y, 1)) {
            south_trader_count1 = 0xf423f;
        } else {
            south_trader_count1 = 2;
        }
    }

    /* ---- West ---- */
    west_trader_count0--;
    if (west_trader_count0 <= 0) {
        if (west_trader_is == 0) {
            do_land_trade(6, west_trader_brings,
                          west_border_x, west_border_y);
            west_trader_count0 = 4;
        } else if (do_sea_trade(6, west_trader_brings,
                                west_border_x, west_border_y, 0)) {
            west_trader_count0 = 0xf423f;
        } else {
            west_trader_count0 = 2;
        }
    }
    west_trader_count1--;
    if (west_trader_count1 <= 0 && west_trader_is != 0) {
        if (do_sea_trade(6, west_trader_brings,
                         west_border_x, west_border_y, 1)) {
            west_trader_count1 = 0xf423f;
        } else {
            west_trader_count1 = 2;
        }
    }
}

// Updates the nearest trading post's route strength and incoming direction.
// FUNCTION: C2 0x54503
// FUNCTION: C2WIN 0x00470f23
void do_land_trade(int direction, int cargo, int border_x, int border_y)
{
    int distance;
    char new_level;
    char current_level;
    char road_flag;
    (void)cargo;

    road_flag = (*(struct region_cell *)((unsigned char *)region_map + ((border_x + border_y * REGION_W) * REGION_CELL_BYTES))).edge_bits & 0x20;

    if (road_flag == 0) return;
    distance = get_closest_trading_post(border_x, border_y, 0x10);
    if (distance > 0x10) return;
    current_level = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).occupant & 0x1c;
    current_level >>= 2;
    if (distance <= 2)       new_level = 7;
    else if (distance <= 4)  new_level = 6;
    else if (distance <= 6)  new_level = 5;
    else if (distance <= 8)  new_level = 4;
    else if (distance <= 10) new_level = 3;
    else if (distance <= 12) new_level = 2;
    else                     new_level = 1;
    if (new_level > current_level) current_level = new_level;
    current_level <<= 2;
    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).occupant &= 0xe3;
    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).occupant |= current_level;

    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).occupant &= 0x9f;
    if (direction == 0) return;
    if (direction == 2) { (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).occupant |= 0x20; return; }
    if (direction == 4) { (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).occupant |= 0x40; return; }
    if (direction == 6) { (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).occupant |= 0x60; return; }
}

// Spawns and initializes a sea-trader army.
// FUNCTION: C2 0x545e8
// FUNCTION: C2WIN 0x0047110f
int do_sea_trade(int compass_side, int cargo,
                 int home_x, int home_y, int army_id)
{
    int army_idx;

    if (create_army(6, home_x, home_y, 1) == 0) return 0;
    army_idx = created_army_no;
    army_list[army_idx].home_x          = home_x;
    army_list[army_idx].home_y          = home_y;
    army_list[army_idx].compass_side    = compass_side;
    army_list[army_idx].trader_brings   = cargo;
    army_list[army_idx].state_idx       = 0xb;
    army_list[army_idx].saved_state_idx = 0xb;
    army_list[army_idx].exists          = 1;
    army_list[army_idx].departure_year  = year;
    army_list[army_idx].target_count = 0; army_list[army_idx].target_kind = 0;
    army_list[army_idx].return_flag = 0;
    army_list[army_idx].army_id     = army_id;
    army_list[army_idx].flags |= 1;
    return 1;
}

// Runs a new or resumed battle and returns to the appropriate map afterward.
// FUNCTION: C2 0x5468f
// FUNCTION: C2WIN 0x004712b4
void continue_battle(int pre_loaded)
{
    unsigned char saved_map_mode;

    battle_setup_count = 0;
    if (pre_loaded == 0) {
        battle_setup_count = 0x64;
        battle_intro();
        get_battle_men();
        battle_state = 5;
    } else {
        decision = 1;
    }

    stop_db();
    act_exit_turbo_mode();

    if (decision == 1) {
        do_fight_battle(pre_loaded);
        if (battle_state != 0xa) get_battle_centuries_left();
    }

    if (battle_state == 6) {
        we_retreat();
        battle_victor = 1;
    } else if (battle_state == 8) {
        battle_victor = 0;
    } else if (battle_state == 7) {
        we_surrender();
        battle_victor = 1;
    } else if (battle_state == 9) {
        battle_victor = 0;
    } else if (battle_state == 4) {
        if (our_battle_men <= 0) {
            battle_victor = 1;
        } else if (our_battle_morale <= 0) {
            battle_victor = 1;
        } else {
            battle_victor = 0;
        }
    } else if (battle_state == 5) {
        battle_auto_resolve();
    } else if (battle_state == 0xa) {
        return;
    } else {
        battle_victor = 0;
    }

    if (battle_victor == 0) {
        do_vga_smacked_anim("battwon.smk");
    } else {
        do_vga_smacked_anim("battlost.smk");
    }

    refresh_zoom_mode(zoom_level);
    act_correct_map();
    battle_outtro();
    do_battle_victory();

    saved_map_mode = map_mode;
    if (saved_map_mode == 0)      city_map_screen(0);
    else if (saved_map_mode == 1) region_map_screen(0);

    play_tune("cityprov.xmi", 0);
    city_tune_playing = 1;
    update_map        = 1;
    setup_map_screen_long_refresh(8);
}

// Displays the pre-battle sequence and waits for it to finish.
// FUNCTION: C2 0x547fd
// FUNCTION: C2WIN 0x004716c9
void battle_intro(void)
{
#if PLATFORM_WINDOWS
    pointer_mode = 0;
    last_icon_over = 0;
    selected_icon_text = 0;
    selected_icon_no = 0;
    last_icon_used = 0;
    stop_tune();
    if (c2inf.tunes_on) {
        set_db_sound("prebatle.raw");
    }
    jump_to_regionmap_ptr(army_list[our_battle_army].map_ref + 0x7a0);
    show_regionmap();
    refresh_svga_screen();
    set_palette(&city_palette);
    if (show_native_battle_intro(main_window) != 0) {
        battle_intro_result = 1;
        act_yes();
    } else {
        battle_intro_result = 0;
        act_no();
    }
#else
    pointer_mode = 0;
    stop_tune();
    if (c2inf.tunes_on) {
        set_db_sound("prebatle.raw");
    }
    jump_to_regionmap_ptr(army_list[our_battle_army].map_ref + 0x7a0);
    show_regionmap();
    show_battle_intro_screen();
    out1 = 0;
    while (out1 == 0) {
        battle_intro_game_loop();
    }
    clear_mouse();
#endif
}

// Displays the battle outcome until the player dismisses it.
// FUNCTION: C2 0x54864
// FUNCTION: C2WIN 0x00471794
void battle_outtro(void)
{
#if PLATFORM_WINDOWS
    pointer_mode = 0;
    last_icon_over = 0;
    selected_icon_text = 0;
    selected_icon_no = 0;
    last_icon_used = 0;
    refresh_svga_screen();
    show_native_battle_outtro(main_window);
    stop_db();
#else
    pointer_mode = 0;
    show_battle_outtro_screen();
    out1 = 0;
    while (out1 == 0) {
        just_idle_game_loop();
        if (mouse_right_click) {
            out1 = 1;
        }
    }
    stop_db();
#endif
}

// Removes all troops from the player's army after a surrender.
// FUNCTION: C2 0x548a7
// FUNCTION: C2WIN 0x004717e6
void we_surrender(void)
{
    army_list[our_battle_army].num_specials    = 0;
    army_list[our_battle_army].num_horse   = 0;
    army_list[our_battle_army].num_regulars    = 0;
    army_list[our_battle_army].num_irregulars  = 0;
    army_list[our_battle_army].num_auxillaries = 0;
}

// Applies the player's troop losses after a retreat.
// FUNCTION: C2 0x548d7
// FUNCTION: C2WIN 0x0047187d
void we_retreat(void)
{
    army_list[our_battle_army].num_specials    = 0;
    army_list[our_battle_army].num_horse   = 0;
    army_list[our_battle_army].num_regulars    /= 2;
    army_list[our_battle_army].num_irregulars  /= 3;
    army_list[our_battle_army].num_auxillaries /= 4;
}

// Resolves a battle from army strength, morale, aggression, and randomness.
// FUNCTION: C2 0x5493f
// FUNCTION: C2WIN 0x00471967
void battle_auto_resolve(void)
{
    int loss_base;
    int our_strength;
    int their_strength;
    int our_score;
    int their_score;
    int loss_percent;
    int survivor_percent;
    int aggression;

    aggression = tribe_ai_data[army_list[their_battle_army].tribe_id].aggression;
    random();   /* discarded — only the side-effect on rand128 matters */

    our_strength = army_list[our_battle_army].num_regulars + army_list[our_battle_army].num_irregulars
                 + army_list[our_battle_army].num_auxillaries + army_list[our_battle_army].num_specials;

    if      (our_strength < 50)   our_score = army_list[our_battle_army].morale * 20;
    else if (our_strength <= 100) our_score = army_list[our_battle_army].morale * 50;
    else if (our_strength <= 200) our_score = army_list[our_battle_army].morale * 80;
    else                          our_score = army_list[our_battle_army].morale * 100;
    our_score += army_list[our_battle_army].num_regulars * 5;
    our_score += army_list[our_battle_army].num_irregulars * 3;
    our_score += army_list[our_battle_army].num_auxillaries * 2;
    our_score += army_list[our_battle_army].num_specials * 3;
    our_score += rand128;

    random();   /* discarded */

    their_strength = army_list[their_battle_army].num_regulars + army_list[their_battle_army].num_irregulars
                   + army_list[their_battle_army].num_auxillaries + army_list[their_battle_army].num_horse
                   + army_list[their_battle_army].num_specials;

    if      (their_strength < 50)   their_score = army_list[their_battle_army].morale * 20;
    else if (their_strength <= 100) their_score = army_list[their_battle_army].morale * 50;
    else if (their_strength <= 200) their_score = army_list[their_battle_army].morale * 80;
    else                            their_score = army_list[their_battle_army].morale * 100;
    their_score += army_list[their_battle_army].num_regulars * 4;
    their_score += army_list[their_battle_army].num_irregulars * 3;
    their_score += army_list[their_battle_army].num_auxillaries;
    their_score += army_list[their_battle_army].num_horse * 4;
    their_score += army_list[their_battle_army].num_specials * 10;
    their_score += rand128;

    if      (their_strength < 50)   their_score += (aggression - 1) * 10;
    else if (their_strength <= 100) their_score += (aggression - 1) * 20;
    else if (their_strength <= 200) their_score += (aggression - 1) * 30;
    else if (their_strength <= 400) their_score += (aggression - 1) * 40;
    else if (their_strength <= 600) their_score += (aggression - 1) * 50;
    else                            their_score += (aggression - 1) * 60;

    random();   /* discarded */

    if (our_score >= their_score) {
        /* We won. */
        battle_victor = 0;
        tune_mood     = 0x11;

        if      (our_strength >= their_strength * 10) loss_base = our_strength / 20;
        else if (our_strength >= their_strength *  5) loss_base = our_strength / 10;
        else if (our_strength >= their_strength *  3) loss_base = our_strength /  5;
        else if (our_strength >= their_strength *  2) loss_base = our_strength /  4;
        else if (our_strength >= their_strength + their_strength / 2) loss_base = our_strength /  3;
        else if (our_strength >= their_strength)      loss_base = our_strength /  2;
        else                                          loss_base = (our_strength * 3) / 4;

        if      (our_score > their_score * 5) loss_base /= 5;
        else if (our_score > their_score * 4) loss_base /= 4;
        else if (our_score > their_score * 3) loss_base /= 3;
        else if (our_score > their_score * 2) loss_base /= 2;

        loss_base += rand128 & 7;
        loss_percent = valueDIVtotal(loss_base, our_strength);
        loss_percent += aggression;
        if (their_strength <= 0) loss_percent = 0;
        if (loss_percent      <  0) loss_percent = 0;
        if (loss_percent      >= 90) loss_percent = 90;
        survivor_percent = 100 - loss_percent;

        army_list[our_battle_army].num_regulars = totalXpercent(army_list[our_battle_army].num_regulars, survivor_percent);
        army_list[our_battle_army].num_irregulars = totalXpercent(army_list[our_battle_army].num_irregulars, survivor_percent);
        army_list[our_battle_army].num_auxillaries = totalXpercent(army_list[our_battle_army].num_auxillaries, survivor_percent);
        army_list[our_battle_army].num_specials = totalXpercent(army_list[our_battle_army].num_specials, survivor_percent);

        army_list[their_battle_army].num_regulars = 0; army_list[their_battle_army].num_irregulars = 0; army_list[their_battle_army].num_auxillaries = 0; army_list[their_battle_army].num_horse = 0; army_list[their_battle_army].num_specials = 0;
    } else {
        /* We lost. */
        battle_victor = 1;
        tune_mood     = 0x12;

        if      (their_strength >= our_strength * 10) loss_base = their_strength / 20;
        else if (their_strength >= our_strength *  5) loss_base = their_strength / 10;
        else if (their_strength >= our_strength *  3) loss_base = their_strength /  5;
        else if (their_strength >= our_strength *  2) loss_base = their_strength /  4;
        else if (their_strength >= our_strength + our_strength / 2) loss_base = their_strength /  3;
        else if (their_strength >= our_strength)      loss_base = their_strength /  2;
        else                                          loss_base = (their_strength * 3) / 4;

        if      (their_score > our_score * 5) loss_base /= 5;
        else if (their_score > our_score * 4) loss_base /= 4;
        else if (their_score > our_score * 3) loss_base /= 3;
        else if (their_score > our_score * 2) loss_base /= 2;

        loss_base += rand128 & 7;
        loss_percent = valueDIVtotal(loss_base, their_strength);
        if (our_strength <= 0) loss_percent = 0;
        if (loss_percent      <  0) loss_percent = 0;
        if (loss_percent      >= 90) loss_percent = 90;
        survivor_percent = 100 - loss_percent;

        army_list[their_battle_army].num_regulars = totalXpercent(army_list[their_battle_army].num_regulars, survivor_percent);
        army_list[their_battle_army].num_irregulars = totalXpercent(army_list[their_battle_army].num_irregulars, survivor_percent);
        army_list[their_battle_army].num_auxillaries = totalXpercent(army_list[their_battle_army].num_auxillaries, survivor_percent);
        army_list[their_battle_army].num_horse = totalXpercent(army_list[their_battle_army].num_horse, survivor_percent);
        army_list[their_battle_army].num_specials = totalXpercent(army_list[their_battle_army].num_specials, survivor_percent);

        army_list[our_battle_army].num_regulars = 0; army_list[our_battle_army].num_irregulars = 0; army_list[our_battle_army].num_auxillaries = 0; army_list[our_battle_army].num_horse = 0; army_list[our_battle_army].num_specials = 0;
    }
}


// Assigns the player and enemy contenders for the upcoming battle.
// FUNCTION: C2 0x54f9e
// FUNCTION: C2WIN 0x004724e7
void get_contenders(void)
{
    if (game_state == 4) return;
    if (army_list[army_no].type == 1) {
        our_battle_army = army_no;
        their_battle_army = army_a;
    } else {
        our_battle_army = army_a;
        their_battle_army = army_no;
    }
}

// Builds a temporary tribal militia for an undefended province.
// FUNCTION: C2 0x54fed
// FUNCTION: C2WIN 0x0047255b
void get_villagers(int village_level)
{
    int   tribe_idx;

    if (game_state == 4) return;

    our_battle_army     = army_no;
    army_a              = 0x19;
    their_battle_army   = 0x19;
    clear_army(&army_list[0x19]);

    army_list[army_a].morale = 3;
    army_list[army_a].source_region = province_is;

    army_list[army_a].total_troops = village_level * 200;

    army_list[army_a].tribe_id = (unsigned char)tribe_type[province_is];
    tribe_idx = army_list[army_a].tribe_id;

    if (tribe_battle_setup[tribe_idx].u.f.middle_kind != 0) {
        army_list[army_a].num_irregulars = village_level * 150;
    } else if (tribe_battle_setup[tribe_idx].u.f.rear_kind != 0) {
        army_list[army_a].num_auxillaries = village_level * 150;
    } else if (tribe_battle_setup[tribe_idx].u.f.front_kind != 0) {
        army_list[army_a].num_regulars = village_level * 150;
    }

    army_list[army_a].battle_disposition = 0xa;
}

// Applies battle results to both armies, Pax Romana, mercenaries, and slaves.
// FUNCTION: C2 0x550d9
// FUNCTION: C2WIN 0x0047274e
void do_battle_victory(void)
{
    struct army_rec *our_army_ptr;
    int   auxiliary_loss;
    int   specialist_loss;

    if (battle_victor == 0) {
        /* We won. */
        if (army_list[our_battle_army].morale < 4) army_list[our_battle_army].morale++;

        if (their_battle_army == 0x19) {
            pax_romanum += 12;
            if (pax_romanum > 1000) pax_romanum = 1000;
        } else {
            army_list[their_battle_army].state_idx = 2;
            pax_romanum += 24;
            if (pax_romanum > 1000) pax_romanum = 1000;
        }

        army_list[our_battle_army].total_troops = army_list[our_battle_army].num_auxillaries + army_list[our_battle_army].num_irregulars
                             + army_list[our_battle_army].num_regulars + army_list[our_battle_army].num_horse
                             + army_list[our_battle_army].num_specials;
        army_list[our_battle_army].state_idx = army_list[our_battle_army].saved_state_idx;
    } else {
        /* We lost. */
        if (army_list[their_battle_army].morale < 4) army_list[their_battle_army].morale++;
        army_list[our_battle_army].morale = 0;
        army_list[our_battle_army].readiness_level = 0;
        army_list[our_battle_army].total_troops = army_list[our_battle_army].num_auxillaries + army_list[our_battle_army].num_irregulars
                             + army_list[our_battle_army].num_regulars + army_list[our_battle_army].num_horse
                             + army_list[our_battle_army].num_specials;
        army_list[our_battle_army].morale_timer = 2;
        army_no = our_battle_army;
        sa10_army_demobed();
        army_list[our_battle_army].state_idx = 1;
        army_list[our_battle_army].saved_state_idx = 1;
        pax_romanum -= 12;
        if (pax_romanum < 0) pax_romanum = 0;
        army_list[their_battle_army].total_troops = army_list[their_battle_army].num_auxillaries + army_list[their_battle_army].num_irregulars
                               + army_list[their_battle_army].num_regulars + army_list[their_battle_army].num_horse
                               + army_list[their_battle_army].num_specials;
    }

    /* Settle specialist and auxiliary losses against their resource pools. */
    auxiliary_loss  = our_battle_auxs     - army_list[our_battle_army].num_auxillaries;
    specialist_loss = our_battle_specials - army_list[our_battle_army].num_specials;

    mercs_in_army -= specialist_loss;
    if (mercs_in_army < 0) mercs_in_army = 0;
    max_mercs_allowed -= specialist_loss;
    if (max_mercs_allowed < 0) max_mercs_allowed = 0;

    if (auxiliary_loss > slave_requirements[6].current) {
        slaves -= slave_requirements[6].current;
        slave_requirements[6].current = 0;
    } else {
        slaves -= auxiliary_loss;
        slave_requirements[6].current -= auxiliary_loss;
    }
    if (slaves <= 0) slaves = 4;
}
