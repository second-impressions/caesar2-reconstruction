#include "c2_data.h"
#include "c2_types.h"

extern int affected_by_cover1(unsigned char *cell_ptr, int range, char mask);
extern int affected_by_cover2(unsigned char *cell_ptr, int range, char mask);
extern unsigned char *get_ptr_to_corner(unsigned char *base_ptr, int size);

int stretch_ofsets_2x2[4][3] = {
    { 20, 1620, 1600 },
    { -20, 1580, 1600 },
    { -20, -1620, -1600 },
    { 20, -1580, -1600 }
};

int stretch_ofsets_3x3[4][5] = {
    { 40, 1640, 3240, 3220, 3200 },
    { -20, 1580, 3180, 3200, 3220 },
    { -20, 1580, -1620, -1580, -1600 },
    { 40, 1640, -1560, -1580, -1600 }
};

struct int_delta_rec putouts1[4] = {
    { 0, -1 },
    { 1, 0 },
    { 0, 1 },
    { -1, 0 }
};

struct int_delta_rec putouts2[8] = {
    { 0, -1 },
    { 1, -1 },
    { 2, 0 },
    { 2, 1 },
    { 1, 2 },
    { 0, 2 },
    { -1, 1 },
    { -1, 0 }
};

struct int_delta_rec putouts3[12] = {
    { 0, -1 },
    { 1, -1 },
    { 2, -1 },
    { 3, 0 },
    { 3, 1 },
    { 3, 2 },
    { 2, 3 },
    { 1, 3 },
    { 0, 3 },
    { -1, 2 },
    { -1, 1 },
    { -1, 0 }
};

struct int_delta_rec putouts4[16] = {
    { 0, -1 },
    { 1, -1 },
    { 2, -1 },
    { 3, -1 },
    { 4, 0 },
    { 4, 1 },
    { 4, 2 },
    { 4, 3 },
    { 3, 4 },
    { 2, 4 },
    { 1, 4 },
    { 0, 4 },
    { -1, 3 },
    { -1, 2 },
    { -1, 1 },
    { -1, 0 }
};
/* Forward declarations (functions defined later in this file). */
void update_time(void);
void monthly_update(void);
void yearly_update(void);
void evolve_water_supply_baths_industry(int row_count);
void evolve_water_table(int row_count);
void evolve_security_cover(int row_count);
void evolve_amenity_cover(int row_count);
void evolve_land_value(int rows);
void cap_land_value(int row_count);
void evolve_forum_activity(int row_count);
void evolve_fort_activity(int row_count);
void evolve_security_activity(int row_count);
void evolve_industrial_activity(int row_count);
void remove_envoy(void);
void market_image(void);
void business_output(int cell_x, int cell_y);
void spread_fire_and_plague_and_unrest(int row_count);
void evolve_a_cm_row(void);
void devolve_a_building( int tier_idx, int footprint_size, unsigned char base_kind, unsigned char gfx_base, unsigned char gfx_step);
void evolve_a_building( int tier_idx, int footprint_size, unsigned char base_kind, unsigned char gfx_base, unsigned char gfx_step);
void change_house(int tier_idx, int footprint_size, int orientation);
void pad_house_with_domus(int previous_size);
void reduce_villa_to_domus(unsigned char *cell_ptr);
void remove_house(void);
/* Callers before this point treat the return as implicit int;
   only the Windows build sees the prototype. */
#if PLATFORM_WINDOWS
void evolve_a_plaza(signed char land_value, signed char old_kind, int cell_x);
#endif
void clear_fire_zones(void);
void push_shell(int row_count);
void evolve_region(int row_count);
void check_goods_in_region_warehouses(void);


// Reset the city-evolution cycle and refresh the goods available from regional warehouses.
// FUNCTION: C2 0x3fa14
// FUNCTION: C2WIN 0x00461b10
void initiate_evolution(void)
{
    evolve_row = 0;
    evolve_clock = 0;
    evolve_count = 0;
    evolve_tick4 = 0;
    evolve_tick3 = 0;
    check_goods_in_region_warehouses();
}

// Run the next phase of city and region evolution, then advance the calendar after a full cycle.
// FUNCTION: C2 0x3fa3c
// FUNCTION: C2WIN 0x00461b52
void citymap_evolution(void)
{
    if (evolve_clock <= 0x50) {
        water_debar = lv_debar = security_debar = industry_debar = unrest_debar = entertainment_debar = education_debar = admin_debar = health_debar = 0;
    } else if (evolve_clock < 0x92) {
        water_debar = lv_debar = security_debar = industry_debar = unrest_debar = entertainment_debar = education_debar = admin_debar = health_debar = 1;
    } else {
        water_debar = lv_debar = security_debar = industry_debar = unrest_debar = entertainment_debar = education_debar = admin_debar = health_debar = 0;
    }

    sooth_mood();

    if      (evolve_clock == 0)       { clear_fire_zones(); }
    else if (evolve_clock < 0x51)     { evolve_row = evolve_clock - 1;
                                         evolve_a_cm_row(); }
    else if (evolve_clock == 0x51)    { clear_all_cm(0xd); }
    else if (evolve_clock == 0x52)    { clear_all_cm(0xf); }
    else if (evolve_clock == 0x53)    { clear_all_cm(0xe); }
    else if (evolve_clock == 0x54)    { clear_all_cm(0xc); }
    else if (evolve_clock == 0x55)    { }
    else if (evolve_clock < 0x5e)     { evolve_row = (evolve_clock - 0x56) * 10;
                                         evolve_water_supply_baths_industry(10); }
    else if (evolve_clock < 0x66)     { evolve_row = (evolve_clock - 0x5e) * 10;
                                         evolve_security_cover(10); }
    else if (evolve_clock < 0x6e)     { evolve_row = (evolve_clock - 0x66) * 10;
                                         evolve_amenity_cover(10); }
    else if (evolve_clock < 0x76)     { evolve_row = (evolve_clock - 0x6e) * 10;
                                         evolve_water_table(10); }
    else if (evolve_clock < 0x7e)     { evolve_row = (evolve_clock - 0x76) * 10;
                                         evolve_land_value(10); }
    else if (evolve_clock < 0x8e)     { evolve_row = (evolve_clock - 0x7e) * 5;
                                         cap_land_value(5); }
    else if (evolve_clock < 0x92)     { security_debar = 1;
                                         evolve_row = (evolve_clock - 0x8e) * 0x14;
                                         evolve_forum_activity(0x14); }
    else if (evolve_clock < 0x96)     { security_debar = 1;
                                         evolve_row = (evolve_clock - 0x92) * 0x14;
                                         evolve_fort_activity(0x14); }
    else if (evolve_clock < 0x9a)     { security_debar = 1;
                                         evolve_row = (evolve_clock - 0x96) * 0x14;
                                         evolve_security_activity(0x14); }
    else if (evolve_clock < 0x9e)     { security_debar = 1;
                                         evolve_row = (evolve_clock - 0x9a) * 0x14;
                                         evolve_industrial_activity(0x14); }
    else if (evolve_clock < 0xa2)     { security_debar = 1;
                                         evolve_row = (evolve_clock - 0x9e) * 0x14;
                                         spread_fire_and_plague_and_unrest(0x14); }
    else if (evolve_clock < 0xaa)     { security_debar = 1;
                                         evolve_row = (evolve_clock - 0xa2) * 10;
                                         push_shell(10); }
    else if (evolve_clock < 0xb2)     { security_debar = 1;
                                         evolve_row = (evolve_clock - 0xaa) * 10;
                                         push_shell(10); }
    else if (evolve_clock < 0xba)     { security_debar = 1;
                                         evolve_row = (evolve_clock - 0xb2) * 10;
                                         push_shell(10); }
    else if (evolve_clock < 0xc2)     { security_debar = 1;
                                         evolve_row = (evolve_clock - 0xba) * 10;
                                         push_shell(10); }
    else if (evolve_clock < 0xca)     { security_debar = 1;
                                         evolve_row = (evolve_clock - 0xc2) * 10;
                                         take_census(10); }
    else if (evolve_clock == 0xca)    { get_census(0); }
    else if (evolve_clock == 0xcb)    { launch_traders();
                                         unflag_all_rm(3, 0xdf); }
    else if (evolve_clock == 0xcc)    { get_regroad_web(reg_city_x, reg_city_y); }
    else if (evolve_clock == 0xcd)    { region_census(); }
    else if (evolve_clock == 0xce)    { evolve_row = 0;
                                         evolve_region(0x14); }
    else if (evolve_clock == 0xcf)    { evolve_row = 0x14;
                                         evolve_region(0x14); }
    else if (evolve_clock == 0xd0)    { evolve_row = 0x28;
                                         evolve_region(0x14); }
    else if (evolve_clock == 0xd1)    { check_goods_in_region_warehouses(); }
    else if (evolve_clock == 0xd2)    { check_citizen_list();
                                         check_army_list(); }
    else if (evolve_clock == 0xd3)    { get_landfill(0);
                                         update_landfill = 1; }

    evolve_clock++;
    if (evolve_clock <= 0xd6) return;

    evolve_tick4++; if (evolve_tick4 >= 4) evolve_tick4 = 0;
    evolve_tick3++; if (evolve_tick3 >= 3) evolve_tick3 = 0;
    evolve_clock = 0;
    evolve_count++;
    update_time();
    update_map = 1;
    setup_map_screen_refresh();
}

// Recompute city-wide service coverage and land values while preserving the current evolution
// phase.
// FUNCTION: C2 0x3ff68
// FUNCTION: C2WIN 0x00462296
void evolve_to_current_fabric(void)
{
    int saved_evolve_clock = evolve_clock;
    evolve_row = 0;
    clear_all_cm(0xd);
    clear_all_cm(0xf);
    clear_all_cm(0xe);
    clear_all_cm(0xc);
    evolve_water_supply_baths_industry(0x50);
    evolve_security_cover(0x50);
    evolve_amenity_cover(0x50);
    evolve_water_table(0x50);
    evolve_land_value(0x50);
    cap_land_value(0x50);
    if (saved_evolve_clock <= 0x50) {
        evolve_clock = saved_evolve_clock;
    } else if (saved_evolve_clock >= 0x8e) {
        evolve_clock = saved_evolve_clock;
    } else {
        evolve_clock = 0x50;
    }
}

// Advance the calendar, run monthly and yearly bookkeeping, and check whether the game has ended.
// FUNCTION: C2 0x3ffff
// FUNCTION: C2WIN 0x00462355
void update_time(void)
{
    get_population_growth_factor();
    get_industry_growth_factor();
    get_insurrection_factor();
    week++;
    if (week < 1) return;
    week = 0;

    if (population < 500) arena_top_count = 0;
    else if (++arena_top_count > 12) arena_top_count = 0;
    if (population < 1000) {
        colosseum_top_count = 0;
    } else {
        colosseum_top_count++;
        if (colosseum_top_count > 18) colosseum_top_count = 0;
    }

    monthly_update();
    month++;
    if (month < 12) {
        check_game_over();
        return;
    }
    month = 0;
    year++;
    years_elapsed++;
    years_elapsed_in_region++;
    yearly_update();
    check_game_over();
    act_do_year_end();
}

// Run monthly events, labor and military upkeep, salary, trouble, conquest, and emperor messages.
// FUNCTION: C2 0x400d0
// FUNCTION: C2WIN 0x00462456
void monthly_update(void)
{
    game_state = 0;
    check_for_promotion();
    random_event();
    slave_random = rand8;
    slave_welfare();
    slave_costs();
    adjust_slave_usage();
    train_soldiers();
    pay_salary();
    region_trouble();
    city_trouble();
    auto_conquer();
    if (warned_of_emperor_reply_month == month + 1) {
        warned_of_emperor_reply_month = 0;
        put_message((warned_of_emperor_reply_level) + 0x7d, 0, 0);
    }
    if (month == 6) {
        if (warned_of_emperor == 0) {
            warned_of_emperor = 1;
            put_message(0x78, 0, 0xa);
        }
    }
}

// Close the year's accounts, set the next tribute, record history, and roll annual statistics.
// FUNCTION: C2 0x4016e
// FUNCTION: C2WIN 0x0046250c
void yearly_update(void)
{
    year_end_accounts();
    get_new_tribute();
    history_entry[0] = population;
    history_entry[1] = denarii;
    history_entry[2] = account_pop_tax;
    history_entry[3] = account_ind_tax;
    history_entry[4] = year;
    save_history();
    last_years_population = this_years_population;
    last_years_denarii = this_years_denarii;
    last_years_pop_tax = this_years_pop_tax;
    last_years_ind_tax = this_years_ind_tax;
    this_years_population = population;
    this_years_denarii = denarii;
    this_years_pop_tax = account_pop_tax;
    this_years_ind_tax = account_ind_tax;
}

// Stamp water, industry, and bath-service coverage around reservoirs, markets, and businesses.
// FUNCTION: C2 0x40200
// FUNCTION: C2WIN 0x004625a8
void evolve_water_supply_baths_industry(int row_count)
{
    int row_idx;
    int col_idx;
    unsigned char building_kind;
    unsigned char supply_level;

    cm_sptr = evolve_row * 1600;
    for (row_idx = 0; row_idx < row_count; row_idx++) {
        for (col_idx = 0; col_idx < 80; col_idx++, cm_sptr += 20) {
            building_kind = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind;
            if (building_kind == 0xbe) {
                supply_level = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).range_flag & 3;
                if      (supply_level == 3) flag_range(0, col_idx, evolve_row + row_idx, 6, 0x0d, 4);
                else if (supply_level == 2) flag_range(0, col_idx, evolve_row + row_idx, 5, 0x0d, 4);
                else if (supply_level == 1) flag_range(0, col_idx, evolve_row + row_idx, 4, 0x0d, 4);
            } else if (building_kind >= 0xfc && building_kind <= 0xff) {
                flag_range(0, col_idx, evolve_row + row_idx, 2, 0x0d, 0x40);
            } else if (building_kind == 0xfa) {
                flag_range(0, col_idx, evolve_row + row_idx, 4, 0x0e, 0x20);
                flag_range(0, col_idx, evolve_row + row_idx, 2, 0x0e, 0x10);
                flag_range(0, col_idx, evolve_row + row_idx, 1, 0x0d, 0x80);
            }
        }
    }
}

// Stamp water coverage and update the supplied state and appearance of fountains and baths.
// FUNCTION: C2 0x40327
// FUNCTION: C2WIN 0x0046277b
void evolve_water_table(int row_count)
{
    unsigned char new_count;
    int map_row;
    int col_no;
    unsigned char has_water_flag;
    int range;
    unsigned char fountain_type;
    unsigned char activity;
    unsigned char image_idx;
    unsigned char building_type;

    cm_sptr = evolve_row * 1600;

    for (map_row = 0; map_row < row_count; map_row++)
        for (col_no = 0; col_no < 80; col_no++, cm_sptr += 20)
        {
            building_type = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind;
            if ((building_type >= 0x1e) && (building_type <= 0x51))
            {
                flag_range(0, col_no, map_row + evolve_row, 3, 0xd, 2);
            }
            else if ((building_type >= 0xd7) && (building_type <= 0xda))
            {
                flag_range(0, col_no, map_row + evolve_row, 2, 0xd, 2);
            }
            else if (building_type == 0xbe)
            {
                has_water_flag = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).range_flag & 3;
                if (has_water_flag == 3) flag_range(0, col_no, map_row + evolve_row, 3, 0xd, 1);
                else if (has_water_flag == 2) flag_range(0, col_no, map_row + evolve_row, 2, 0xd, 1);
                else if (has_water_flag == 1) flag_range(0, col_no, map_row + evolve_row, 1, 0xd, 1);
            }
            else if ((building_type >= 0xdb) && (building_type <= 0xde))
            {
                has_water_flag = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).education & 4;

                if (water_trouble_rate == 0) has_water_flag = 0;
                else if (water_trouble_rate < 0x10)
                {
                    new_count = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building + water_trouble_rate;
                    if (new_count < 0x10) has_water_flag = 0; else new_count = new_count & 0xf;
                    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building = new_count;
                }

                if (has_water_flag)
                {
                    flag_range(0, col_no, map_row + evolve_row, 6, 0xd, 1);
                    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).extra_edge = house_gfxdat[building_type + 0x2d] + 1;
                }
                else (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).extra_edge = house_gfxdat[building_type + 0x2d];
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).edge_bits |= 1;
            }
            else if ((building_type >= 0xdf) && (building_type <= 0xe2))
            {
                activity = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a & 0xf;
                if (activity != 0) continue;
                fountain_type = building_type - 0xdf;
                has_water_flag = affected_by_cover1((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).b, 2, 4);
                if (has_water_flag)
                {
                    range = building_type - 0xda;
                    flag_range(1, col_no, map_row + evolve_row, range, 0xd, 8);
                }

                if (water_trouble_rate == 0) has_water_flag = 0;
                else if (water_trouble_rate < 0x10)
                {
                    new_count = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building + water_trouble_rate;
                    if (new_count < 0x10) has_water_flag = 0; else new_count = new_count & 0xf;
                    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building = new_count;
                }

                image_idx = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).extra_edge;
                if (has_water_flag && (image_idx >= 0x63)) change_sized(building_type, (fountain_type * 4) + 0x20, 2, cm_sptr);
                if ((!has_water_flag) && (image_idx < 0x63)) change_sized(building_type, (fountain_type * 4) + 0x63, 2, cm_sptr);
            }
        }
}

// Stamp security and administrative coverage around patrol buildings, forts, and forums.
// FUNCTION: C2 0x40617
// FUNCTION: C2WIN 0x00462bda
void evolve_security_cover(int row_count)
{
    int row_idx;
    int col_idx;
    unsigned char building_kind;

    cm_sptr = evolve_row * 1600;

    for (row_idx = 0; row_idx < row_count; row_idx++) {
        for (col_idx = 0; col_idx < 80; col_idx++, cm_sptr += 20) {
            building_kind = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind;

            if (building_kind == 0xe4) {
                flag_range(0, col_idx, evolve_row + row_idx, 3, 0xe, 0x01);
                flag_range(0, col_idx, evolve_row + row_idx, 3, 0xa, 0x30);
            } else if (building_kind == 0xe3) {
                flag_range(0, col_idx, evolve_row + row_idx, 2, 0xe, 0x02);
                flag_range(0, col_idx, evolve_row + row_idx, 3, 0xa, 0x30);
            } else if (building_kind == 0xc0) {
                flag_range(0, col_idx, evolve_row + row_idx, 2, 0xe, 0x04);
            } else if (building_kind >= 0xbf && building_kind <= 0xca) {
                flag_range(0, col_idx, evolve_row + row_idx, 2, 0xe, 0x08);
            } else if (building_kind >= 0xae && building_kind <= 0xb1) {
                flag_range(0, col_idx, evolve_row + row_idx, 3, 0xa, 0x0c);
            } else if (building_kind >= 0xb2 && building_kind <= 0xb5) {
                flag_range(0, col_idx, evolve_row + row_idx, 4, 0xa, 0x0c);
            } else if (building_kind >= 0xb6 && building_kind <= 0xb9) {
                flag_range(0, col_idx, evolve_row + row_idx, 5, 0xa, 0x0c);
            } else if (building_kind >= 0xfc && building_kind <= 0xff) {
                flag_range(0, col_idx, evolve_row + row_idx, 3, 0xa, 0xc0);
            }

        }
    }
}

// Stamp health, education, and entertainment coverage around active amenity buildings.
// FUNCTION: C2 0x4077b
// FUNCTION: C2WIN 0x00462e70
void evolve_amenity_cover(int row_count)
{
    int row_idx;
    int col_idx;
    unsigned char building_kind;
    unsigned char activity_state;

    cm_sptr = evolve_row * 1600;

    for (row_idx = 0; row_idx < row_count; row_idx++) {
        for (col_idx = 0; col_idx < 80; col_idx++, cm_sptr += 20) {
            building_kind = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind;
            activity_state = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a & 0x0f;
            if (activity_state != 0) continue;

            if (building_kind == 0xf3) {
                flag_range(1, col_idx, evolve_row + row_idx, 6, 0xd, 0x10);
            } else if (building_kind == 0xf4) {
                flag_range(2, col_idx, evolve_row + row_idx, 8, 0xd, 0x20);
            } else if (building_kind == 0xe5) {
                flag_range3(1, col_idx, evolve_row + row_idx, 9, 0xc, 1, 3, 0xfc);
                flag_range3(1, col_idx, evolve_row + row_idx, 7, 0xc, 2, 3, 0xfc);
                flag_range3(1, col_idx, evolve_row + row_idx, 5, 0xc, 3, 3, 0xfc);
            } else if (building_kind == 0xe6) {
                flag_range3(1, col_idx, evolve_row + row_idx, 11, 0xc, 1, 3, 0xfc);
                flag_range3(1, col_idx, evolve_row + row_idx,  9, 0xc, 2, 3, 0xfc);
                flag_range3(1, col_idx, evolve_row + row_idx,  7, 0xc, 3, 3, 0xfc);
            } else if (building_kind == 0xe7) {
                flag_range3(2, col_idx, evolve_row + row_idx, 9, 0xc, 4, 0xc, 0xf3);
                flag_range3(2, col_idx, evolve_row + row_idx, 7, 0xc, 8, 0xc, 0xf3);
                flag_range3(2, col_idx, evolve_row + row_idx, 5, 0xc, 0xc, 0xc, 0xf3);
            } else if (building_kind == 0xe8) {
                flag_range3(2, col_idx, evolve_row + row_idx, 11, 0xc, 4, 0xc, 0xf3);
                flag_range3(2, col_idx, evolve_row + row_idx,  9, 0xc, 8, 0xc, 0xf3);
                flag_range3(2, col_idx, evolve_row + row_idx,  7, 0xc, 0xc, 0xc, 0xf3);
            } else if (building_kind == 0xe9 || building_kind == 0xea || building_kind == 0xeb || building_kind == 0xec) {
                flag_range3(2, col_idx, evolve_row + row_idx, 10, 0xc, 0x10, 0x30, 0xcf);
                flag_range3(2, col_idx, evolve_row + row_idx,  8, 0xc, 0x20, 0x30, 0xcf);
                flag_range3(2, col_idx, evolve_row + row_idx,  6, 0xc, 0x30, 0x30, 0xcf);
            } else if (building_kind == 0xed || building_kind == 0xee || building_kind == 0xef || building_kind == 0xf0) {
                flag_range3(3, col_idx, evolve_row + row_idx, 12, 0xc, 0x10, 0x30, 0xcf);
                flag_range3(3, col_idx, evolve_row + row_idx, 10, 0xc, 0x20, 0x30, 0xcf);
                flag_range3(3, col_idx, evolve_row + row_idx,  8, 0xc, 0x30, 0x30, 0xcf);
            }
        }
    }
}

// Apply each cell's local land-value effects and track the highest-valued point in the city.
// FUNCTION: C2 0x40ac5
// FUNCTION: C2WIN 0x004632ce
void evolve_land_value(int rows)
{
    int row;
    int col;
    unsigned char kind;
    unsigned char flags;
    unsigned char bkind;
    unsigned char idx;
    unsigned char cooldown;
    int radius;
    int delta;
    int growth;
    int size;
    signed char curr_lv;

    if (evolve_row == 0) top_lv = 0;
    cm_sptr = evolve_row * 1600;

    for (row = 0; row < rows; row++) {
        for (col = 0; col < 80; col++, cm_sptr += 20) {
            flags = ((unsigned char *)city_map)[cm_sptr + 1];
            kind  = ((unsigned char *)city_map)[cm_sptr];

            if ((flags & 0x20) != 0) {
                if ((flags & 0x04) != 0) {
                    change_lv(col, row + evolve_row, 1, 1, 2);
                } else if ((flags & 0x10) != 0) {
                    change_lv(col, row + evolve_row, 2, 1, 1);
                } else if (kind == 0x5c) {
                    change_lv(col, row + evolve_row, 1, 1, 2);
                } else if (kind >= 0x58 && kind <= 0x5b) {
                    change_lv(col, row + evolve_row, 1, 1, 1);
                }
            } else {
                if ((flags & 0x01) != 0) {
                bkind = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind;
                cooldown = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a & 0xf;
                if (cooldown != 0) continue;
                if (bkind >= 0x82 && bkind <= 0xa1) {
                        bkind -= 0x82;
                        radius = house_lv_effect[bkind].radius;
                        delta  = house_lv_effect[bkind].delta;
                        size   = house_gfxdat[bkind*4 + 1];
                        delta += pop_growth_factor;
                        if      (bkind < 0x1a) growth = 0;
                        else if (bkind < 0x1e) growth = pop_growth_factor * 2;
                        else                 growth = pop_growth_factor * 4;
                        if (growth < 0) growth = 0;
                        (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).land_value += growth;
                        change_lv(col, row + evolve_row, radius, size, delta);
                    } else if (bkind >= 0xdb && bkind <= 0xde) {
                        delta  = buildings_lv_effect[0];
                        radius = buildings_lv_effect[1];
                        if (((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).education & 4) != 0)
                            change_lv(col, row + evolve_row, radius, 1, delta);
                    } else if (bkind == 0xe3) {
                        delta  = buildings_lv_effect[2];
                        radius = buildings_lv_effect[3];
                        change_lv(col, row + evolve_row, radius, 1, delta);
                    } else if (bkind == 0xe4) {
                        delta  = buildings_lv_effect[4];
                        radius = buildings_lv_effect[5];
                        change_lv(col, row + evolve_row, radius, 3, delta);
                    } else if (bkind >= 0xae && bkind <= 0xb9) {
                        bkind -= 0xae;
                        radius = forum_lv_effect[bkind].radius;
                        delta  = forum_lv_effect[bkind].delta;
                        size   = forum_gfxdat[bkind*4 + 1];
                        change_lv(col, row + evolve_row, radius, size, delta);
                    } else if (bkind == 0xe5) {
                        delta  = buildings_lv_effect[6];
                        radius = buildings_lv_effect[7];
                        change_lv(col, row + evolve_row, radius, 2, delta);
                    } else if (bkind == 0xe6) {
                        delta  = buildings_lv_effect[8];
                        radius = buildings_lv_effect[9];
                        change_lv(col, row + evolve_row, radius, 2, delta);
                    } else if (bkind == 0xe7) {
                        delta  = buildings_lv_effect[10];
                        radius = buildings_lv_effect[11];
                        change_lv(col, row + evolve_row, radius, 3, delta);
                    } else if (bkind == 0xe8) {
                        delta  = buildings_lv_effect[12];
                        radius = buildings_lv_effect[13];
                        change_lv(col, row + evolve_row, radius, 3, delta);
                    } else if (bkind >= 0xe9 && bkind <= 0xec) {
                        delta  = buildings_lv_effect[14];
                        radius = buildings_lv_effect[15];
                        change_lv(col, row + evolve_row, radius, 3, delta);
                    } else if (bkind >= 0xed && bkind <= 0xf0) {
                        delta  = buildings_lv_effect[16];
                        radius = buildings_lv_effect[17];
                        change_lv(col, row + evolve_row, radius, 4, delta);
                    } else if (bkind == 0xf3) {
                        delta  = buildings_lv_effect[18];
                        radius = buildings_lv_effect[19];
                        change_lv(col, row + evolve_row, radius, 2, delta);
                    } else if (bkind == 0xf4) {
                        delta  = buildings_lv_effect[20];
                        radius = buildings_lv_effect[21];
                        change_lv(col, row + evolve_row, radius, 3, delta);
                    } else if (bkind == 0xf5) {
                        delta  = buildings_lv_effect[22];
                        radius = buildings_lv_effect[23];
                        change_lv(col, row + evolve_row, radius, 3, delta);
                    } else if (bkind >= 0xa2 && bkind <= 0xad) {
                        bkind -= 0xa2;
                        radius = temple_lv_effect[bkind].radius;
                        delta  = temple_lv_effect[bkind].delta;
                        if      (bkind < 4) size = 1;
                        else if (bkind < 8) size = 2;
                        else              size = 3;
                        change_lv(col, row + evolve_row, radius, size, delta);
                    } else if (bkind == 0xfa) {
                        idx = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building & 0xf0;
                        idx >>= 4;
                        if (idx <= 4) {
                            delta  = buildings_lv_effect[24];
                            radius = buildings_lv_effect[25];
                        } else {
                            delta  = buildings_lv_effect[26];
                            radius = buildings_lv_effect[27];
                        }
                        change_lv(col, row + evolve_row, radius, 3, delta);
                    } else if (bkind == 0xfc) {
                        delta  = buildings_lv_effect[28];
                        radius = buildings_lv_effect[29];
                        change_lv(col, row + evolve_row, radius, 2, delta);
                    } else if (bkind == 0xfd) {
                        delta  = buildings_lv_effect[30];
                        radius = buildings_lv_effect[31];
                        change_lv(col, row + evolve_row, radius, 2, delta);
                    } else if (bkind == 0xfe) {
                        delta  = buildings_lv_effect[32];
                        radius = buildings_lv_effect[33];
                        change_lv(col, row + evolve_row, radius, 2, delta);
                    } else if (bkind == 0xff) {
                        delta  = buildings_lv_effect[34];
                        radius = buildings_lv_effect[35];
                        change_lv(col, row + evolve_row, radius, 2, delta);
                    } else if (bkind == 0xdf) {
                        if (((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).education & 4) != 0) {
                            delta  = buildings_lv_effect[36];
                            radius = buildings_lv_effect[37];
                            change_lv(col, row + evolve_row, radius, 2, delta);
                        }
                    } else if (bkind == 0xe0) {
                        if (((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).education & 4) != 0) {
                            delta  = buildings_lv_effect[38];
                            radius = buildings_lv_effect[39];
                            change_lv(col, row + evolve_row, radius, 2, delta);
                        }
                    } else if (bkind == 0xe1) {
                        if (((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).education & 4) != 0) {
                            delta  = buildings_lv_effect[40];
                            radius = buildings_lv_effect[41];
                            change_lv(col, row + evolve_row, radius, 2, delta);
                        }
                    } else if (bkind == 0xe2) {
                        if (((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).education & 4) != 0) {
                            delta  = buildings_lv_effect[42];
                            radius = buildings_lv_effect[43];
                            change_lv(col, row + evolve_row, radius, 2, delta);
                        }
                } else if (bkind == 0xfb) {
                    delta  = buildings_lv_effect[44];
                    radius = buildings_lv_effect[45];
                    change_lv(col, row + evolve_row, radius, 3, delta);
                }
                } else if ((flags & 0x18) != 0) {
                    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).land_value = 0;
                }
            }

            if (kind >= 0 && kind < 8) {
                if ((((unsigned char *)city_map)[cm_sptr + 3] & 0x80) != 0) {
                    delta  = buildings_lv_effect[46];
                    radius = buildings_lv_effect[47];
                    change_lv(col, row + evolve_row, radius, 1, delta);
                } else {
                    delta  = buildings_lv_effect[48];
                    radius = buildings_lv_effect[49];
                    change_lv(col, row + evolve_row, radius, 1, delta);
                }
                delta  = buildings_lv_effect[50];
                radius = buildings_lv_effect[51];
                change_lv(col, row + evolve_row, radius, 1, delta);
            } else if (kind >= 0x7c && kind <= 0x7e) {
                delta  = buildings_lv_effect[52];
                radius = buildings_lv_effect[53];
                change_lv(col, row + evolve_row, radius, 1, delta);
            } else if (kind >= 0x78 && kind <= 0x7b) {
                delta  = buildings_lv_effect[54];
                radius = buildings_lv_effect[55];
                change_lv(col, row + evolve_row, radius, 1, delta);
            }

            curr_lv = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).land_value;
            if (curr_lv > top_lv) {
                top_lv      = curr_lv;
                top_lv_spot = cm_sptr;
            }
        }
    }

    if (row + evolve_row >= 0x50) {
        top_lv_ptr = top_lv_spot / 20;
        top_lv_x   = top_lv_ptr  % 80;
        top_lv_y   = top_lv_ptr  / 80;
    }
}

// Cap each cell's land value according to its water, services, security, health, and education.
// FUNCTION: C2 0x41138
// FUNCTION: C2WIN 0x00463f66
void cap_land_value(int row_count)
{
    unsigned char coverage_hit;
    int col_idx;
    signed char land_value_cap;
    int row_idx;
    unsigned char building_kind;
    int sub_aqueduct_cover;
    unsigned char activity_state;
    unsigned int footprint_size;
    unsigned char security_score;
    signed char security;
    char entertainment_high;
    char entertainment_mid;
    unsigned char *cell_ptr;
    int range_value;
    char entertainment_low;
    unsigned char entertainment_rank;
    signed char current_land_value;
    signed char aqueduct_cover;

    cm_sptr = evolve_row * 1600;
    for (row_idx = 0; row_idx < row_count; row_idx++)
    {
        for (col_idx = 0; col_idx < 80; col_idx++, cm_sptr += 20)
        {
            building_kind = ((unsigned char *)city_map)[cm_sptr];
            if (building_kind >= 0x82) footprint_size = reg_aquaduct_gfxdat[building_kind + 8];
            else footprint_size = 1;

            activity_state = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a & 0xf;

            cell_ptr = (unsigned char *)city_map + cm_sptr;
            if (activity_state) cell_ptr = get_ptr_to_corner(cell_ptr, footprint_size);

            if ((building_kind < 0x82) || (building_kind > 0xa1))
                goto non_housing;

            sub_aqueduct_cover = affected_by_cover1(cell_ptr, footprint_size, 2);
            aqueduct_cover = affected_by_cover1(cell_ptr, footprint_size, 1);
            if (aqueduct_cover == 0 && (char)sub_aqueduct_cover == 0) { land_value_cap = 0x02; goto emit; }

            coverage_hit = get_range1(cell_ptr, footprint_size, 0x0c);
            if (coverage_hit == 0) { land_value_cap = 0x06; goto emit; }

            coverage_hit = affected_by_cover1(cell_ptr, footprint_size, 0x80);
            if (coverage_hit != 0) { land_value_cap = 0x0a; goto emit; }

            coverage_hit = get_range1(cell_ptr, footprint_size, 0xc0);
            if (coverage_hit == 0) { land_value_cap = 0x0c; goto emit; }

            if (aqueduct_cover == 0) { land_value_cap = 0x0e; goto emit; }

            coverage_hit = affected_by_cover2(cell_ptr, footprint_size, 0x10);
            if (coverage_hit != 0) { land_value_cap = 0x10; goto emit; }

            coverage_hit = affected_by_cover1(cell_ptr, footprint_size, 0x08);
            if (coverage_hit == 0) { land_value_cap = 0x12; goto emit; }

            entertainment_low = get_range3(cell_ptr, footprint_size, 0x03);
            range_value = get_range3(cell_ptr, footprint_size, 0x0c);
            entertainment_mid = (range_value & 0xff) >> 2;
            range_value = get_range3(cell_ptr, footprint_size, 0x30);
            entertainment_high = (range_value & 0xff) >> 4;
            entertainment_rank = entertainment_low + entertainment_mid + entertainment_high;
            if (entertainment_rank == 0) { land_value_cap = 0x14; goto emit; }

            coverage_hit = affected_by_cover2(cell_ptr, footprint_size, 0x01);
            if (coverage_hit != 0) { land_value_cap = 0x18; goto emit; }

            security = ((unsigned char *)city_map)[cm_sptr + 0x11];
            coverage_hit = get_range1(cell_ptr, footprint_size, 0x30);
            security_score = (security >= 0x10);

            if (coverage_hit) { security_score &= 0xff; security_score++; }
            if (security_score == 0) { land_value_cap = 0x18; goto emit; }

            coverage_hit = affected_by_cover2(cell_ptr, footprint_size, 0x20);
            if (coverage_hit != 0) { land_value_cap = 0x1a; goto emit; }

            if (entertainment_rank <= 1) { land_value_cap = 0x1a; goto emit; }

            coverage_hit = affected_by_cover2(cell_ptr, footprint_size, 0x08);
            if (coverage_hit != 0) { land_value_cap = 0x1a; goto emit; }

            if (entertainment_rank <= 2) { land_value_cap = 0x1c; goto emit; }

            coverage_hit = affected_by_cover2(cell_ptr, footprint_size, 0x04);
            if (coverage_hit != 0) { land_value_cap = 0x1e; goto emit; }

            if (hospital_cover < 0x14) { land_value_cap = 0x1e; goto emit; }

            if (entertainment_rank <= 3) { land_value_cap = 0x20; goto emit; }

            coverage_hit = affected_by_cover1(cell_ptr, footprint_size, 0x10);
            if (coverage_hit == 0) { land_value_cap = 0x22; goto emit; }

            coverage_hit = affected_by_cover2(cell_ptr, footprint_size, 0x02);
            if (coverage_hit != 0) { land_value_cap = 0x22; goto emit; }

            if (hospital_cover < 0x28) { land_value_cap = 0x24; goto emit; }

            if (entertainment_rank <= 4) { land_value_cap = 0x26; goto emit; }

            coverage_hit = affected_by_cover1(cell_ptr, footprint_size, 0x40);
            if (coverage_hit != 0) { land_value_cap = 0x28; goto emit; }

            if (security_score <= 1) { land_value_cap = 0x2a; goto emit; }

            if (hospital_cover < 0x3c) { land_value_cap = 0x2c; goto emit; }

            if (entertainment_rank <= 5) { land_value_cap = 0x2c; goto emit; }

            coverage_hit = affected_by_cover1(cell_ptr, footprint_size, 0x20);
            if (coverage_hit == 0) { land_value_cap = 0x2e; goto emit; }

            if (library_cover < 0x14) { land_value_cap = 0x2e; goto emit; }

            if (entertainment_rank <= 6) { land_value_cap = 0x30; goto emit; }

            if (library_cover < 0x28) { land_value_cap = 0x32; goto emit; }

            if (hospital_cover < 0x50) { land_value_cap = 0x34; goto emit; }

            if (library_cover < 0x3c) { land_value_cap = 0x36; goto emit; }

            if (entertainment_rank <= 7) { land_value_cap = 0x38; goto emit; }

            if (hospital_cover < 0x64) { land_value_cap = 0x3a; goto emit; }

            if (library_cover < 0x50) { land_value_cap = 0x3a; goto emit; }

            if (entertainment_rank <= 8) { land_value_cap = 0x3c; goto emit; }

            if (library_cover < 0x64) { land_value_cap = 0x3e; goto emit; }

            land_value_cap = 0x40;
            goto emit;

        non_housing:
            coverage_hit = affected_by_cover1(cell_ptr, footprint_size, 0x80);
            if (coverage_hit != 0) { land_value_cap = 0x0a; goto emit; }

            coverage_hit = affected_by_cover2(cell_ptr, footprint_size, 0x10);
            if (coverage_hit != 0) { land_value_cap = 0x10; goto emit; }

            coverage_hit = affected_by_cover2(cell_ptr, footprint_size, 0x01);
            if (coverage_hit != 0) { land_value_cap = 0x18; goto emit; }

            coverage_hit = affected_by_cover2(cell_ptr, footprint_size, 0x20);
            if (coverage_hit != 0) { land_value_cap = 0x1a; goto emit; }

            coverage_hit = affected_by_cover2(cell_ptr, footprint_size, 0x08);
            if (coverage_hit != 0) { land_value_cap = 0x1a; goto emit; }

            coverage_hit = affected_by_cover2(cell_ptr, footprint_size, 0x04);
            if (coverage_hit != 0) { land_value_cap = 0x1e; goto emit; }

            coverage_hit = affected_by_cover2(cell_ptr, footprint_size, 0x02);
            if (coverage_hit != 0) { land_value_cap = 0x22; goto emit; }

            coverage_hit = affected_by_cover1(cell_ptr, footprint_size, 0x40);
            if (coverage_hit != 0) { land_value_cap = 0x28; goto emit; }

            land_value_cap = 0x40;
        emit:
            current_land_value = ((signed char *)city_map)[cm_sptr + 0xf];
            if (land_value_cap < current_land_value) ((unsigned char *)city_map)[cm_sptr + 0xf] = land_value_cap;
        }
    }
}

// Update forum activity and periodically send citizens from occupied forum buildings.
// FUNCTION: C2 0x415bb
// FUNCTION: C2WIN 0x004647a0
void evolve_forum_activity(int row_count)
{
    unsigned char flags;
    int row_idx;
    unsigned char building_kind;
    unsigned char activity_state;
    int col_idx;
    unsigned char gfx_template;
    unsigned char cooldown;
    unsigned char shoppers;
    int spawn_result;

    cm_sptr = evolve_row * 1600;

    for (row_idx = 0; row_idx < row_count; row_idx++) {
        for (col_idx = 0; col_idx < 80; col_idx++, cm_sptr += 20) {
            building_kind = ((unsigned char *)city_map)[cm_sptr];
            if (building_kind < 0xae) continue; if (building_kind > 0xb9) continue;

            gfx_template = forum_gfxdat[building_kind + 0x26];
            if (gfx_template == 0) continue;
            activity_state = ((unsigned char *)city_map)[cm_sptr + 5] & 0x0f;
            if (activity_state != 0) continue;

            cooldown = ((unsigned char *)city_map)[cm_sptr + 6] & 0x0f;
            flags = ((unsigned char *)city_map)[cm_sptr + 6] & 0x10;
            shoppers = (((unsigned char *)city_map)[cm_sptr + 5] & 0xf0) >> 4;

            if (cooldown == 0) {
                if (population < 2) continue;
                spawn_result = put_out_a(1, (unsigned char)col_idx, (unsigned char)(evolve_row + row_idx), flags,
                                   shoppers, gfx_template, 0x20);
                if (spawn_result != 0) {
                    shoppers = spawn_result & 0xff;
                    cooldown = 3;
                    if (gfx_template == 9) cooldown -= 1;
                    else if (gfx_template == 0x10) cooldown -= 2;
                    citizen_list[created_citizen_no].state_idx = 1; citizen_list[created_citizen_no].wait_count = 0x14;
                    citizen_list[created_citizen_no].saved_state_idx = 3;
                }
                if (shoppers >= gfx_template) shoppers = 0;
            } else {
                cooldown--;
            }

            ((unsigned char *)city_map)[cm_sptr + 6] &= 0xf0;
            ((unsigned char *)city_map)[cm_sptr + 6] |= cooldown;
            ((unsigned char *)city_map)[cm_sptr + 5] &= 0x0f;
            ((unsigned char *)city_map)[cm_sptr + 5] |= shoppers << 4;
        }
    }
}

// Dispatch soldiers from forts to engage nearby enemy citizens.
// FUNCTION: C2 0x4176e
// FUNCTION: C2WIN 0x00464a1f
void evolve_fort_activity(int row_count)
{
    int row_idx;
    int col_idx;
    unsigned char building_kind;
    unsigned char counter;
    short enemy_idx;
    int spawn_result;

    cm_sptr = evolve_row * 1600;

    for (row_idx = 0; row_idx < row_count; row_idx++) {
        for (col_idx = 0; col_idx < 80; col_idx++, cm_sptr += 20) {
            building_kind = ((unsigned char *)city_map)[cm_sptr];
            if (building_kind == 0xbf) {
                counter = ((unsigned char *)city_map)[cm_sptr + 6] & 0x0f;
                if (counter == 0) {
                    if (population < 2) continue;
                    enemy_idx = find_enemy(col_idx, evolve_row + row_idx, 6);
                    if (enemy_idx == 0) continue;
                    counter = 3;
                    spawn_result = put_out_a(4, (unsigned char)col_idx, (unsigned char)(evolve_row + row_idx), 0, 0, 1, 0);
                    if (spawn_result != 0) {
                        citizen_a = enemy_idx;
                        citizen_list[created_citizen_no].target_kind = citizen_a;
                        citizen_list[created_citizen_no].target_marker = citizen_list[enemy_idx].evolve_timer;
                        citizen_list[created_citizen_no].dest_x = citizen_list[enemy_idx].x; citizen_list[created_citizen_no].dest_y = citizen_list[enemy_idx].y;
                        citizen_list[created_citizen_no].state_idx = 1;
                        citizen_list[created_citizen_no].saved_state_idx = 6;
                        citizen_list[created_citizen_no].wait_count = 0x14;
                    }
                } else {
                    counter--;
                }
                ((unsigned char *)city_map)[cm_sptr + 6] &= 0xf0;
                ((unsigned char *)city_map)[cm_sptr + 6] |= counter;
            }
        }
    }
}

// Update prefecture and watchtower cooldowns and dispatch their next patrol citizens.
// FUNCTION: C2 0x418d9
// FUNCTION: C2WIN 0x00464c75
void evolve_security_activity(int row_count)
{
    int row_idx;
    unsigned char activity_state;
    unsigned char building_kind;
    int col_idx;
    unsigned char flags;
    unsigned char cooldown;
    unsigned char patrol_count;
    int spawn_result;

    cm_sptr = evolve_row * 1600;

    for (row_idx = 0; row_idx < row_count; row_idx++) {
        for (col_idx = 0; col_idx < 80; col_idx++, cm_sptr += 20) {
            building_kind = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind;
            if (building_kind == 0xe3) {
                activity_state = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a & 0x0f;
                if (activity_state != 0) continue;
                cooldown     = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_b & 0x0f;
                flags        = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_b & 0x10;
                patrol_count = ((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a & 0xf0) >> 4;
                if (cooldown == 0) {
                    if (population < 2) continue;
                    spawn_result = put_out_a(5, (unsigned char)col_idx, (unsigned char)(evolve_row + row_idx), flags,
                                       patrol_count, 1, 0x20);
                    if (spawn_result != 0) {
                        patrol_count = (unsigned char)spawn_result;
                        cooldown = 3;
                        citizen_list[created_citizen_no].state_idx = 1; citizen_list[created_citizen_no].wait_count = 0x14;
                        citizen_list[created_citizen_no].saved_state_idx = 8;
                    }
                    patrol_count++; if (patrol_count >= 4) patrol_count = 0;
                } else {
                    cooldown--;
                }
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_b &= 0xf0;
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_b |= cooldown;
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a &= 0x0f;
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a |= patrol_count << 4;
            } else if (building_kind == 0xe4) {
                activity_state = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a & 0x0f;
                if (activity_state != 0) continue;
                cooldown     = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_b & 0x0f;
                flags        = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_b & 0x10;
                patrol_count = ((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a & 0xf0) >> 4;
                if (cooldown == 0) {
                    if (population < 2) continue;
                    spawn_result = put_out_a(4, (unsigned char)col_idx, (unsigned char)(evolve_row + row_idx), flags,
                                       patrol_count, 9, 0x20);
                    if (spawn_result != 0) {
                        patrol_count = (unsigned char)spawn_result;
                        cooldown = 3;
                        citizen_list[created_citizen_no].state_idx = 1; citizen_list[created_citizen_no].wait_count = 0x14;
                        citizen_list[created_citizen_no].saved_state_idx = 7;
                    }
                    patrol_count++; if (patrol_count >= 9) patrol_count = 0;
                } else {
                    cooldown--;
                }
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_b &= 0xf0;
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_b |= cooldown;
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a &= 0x0f;
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a |= patrol_count << 4;
            }
        }
    }
}


// Update markets and businesses, refresh their output, and dispatch shopper or trader citizens.
// FUNCTION: C2 0x41b49
// FUNCTION: C2WIN 0x0046503e
void evolve_industrial_activity(int row_count)
{
    unsigned char building_kind;
    int row_idx;
    unsigned char activity_state;
    unsigned char flags;
    unsigned char patrons;
    unsigned char cooldown;
    int col_idx;
    int spawn_result;

    cm_sptr = evolve_row * 1600;

    for (row_idx = 0; row_count > row_idx; row_idx++) {
        for (col_idx = 0; col_idx < 80; col_idx++, cm_sptr += 20) {
            building_kind = ((unsigned char *)city_map)[cm_sptr];
            if (building_kind >= 0xfc && building_kind <= 0xff) {
                activity_state = ((unsigned char *)city_map)[cm_sptr + 5] & 0xf;
                if (activity_state == 0) {
                    market_image();

                    cooldown = ((unsigned char *)city_map)[cm_sptr + 6] & 0xf;
                    flags = ((unsigned char *)city_map)[cm_sptr + 6] & 0x10;
                    patrons  = (((unsigned char *)city_map)[cm_sptr + 5] & 0xf0) >> 4;
                    if (cooldown == 0) {
                        if (population < 2) goto next;
                        spawn_result = put_out_a(2, (char)col_idx, (char)(evolve_row + row_idx), flags,
                                           (char)patrons, 4, 0x20);
                        if (spawn_result != 0) {
                            patrons = (unsigned char)spawn_result;
                            cooldown = 3;
                            citizen_list[created_citizen_no].state_idx = 1; citizen_list[created_citizen_no].wait_count = 0x14;
                            citizen_list[created_citizen_no].saved_state_idx = 4;
                            citizen_list[created_citizen_no].target_ref = cm_sptr;
                            remove_envoy();
                            ((unsigned char *)city_map)[cm_sptr + 0x12] = (unsigned char)created_citizen_no;
                        }
                        patrons++; if (patrons >= 4) patrons = 0;
                    } else {
                        cooldown--;
                    }
                    ((unsigned char *)city_map)[cm_sptr + 6] &= 0xf0;
                    ((unsigned char *)city_map)[cm_sptr + 6] |= cooldown;
                    ((unsigned char *)city_map)[cm_sptr + 5] &= 0x0f;
                    ((unsigned char *)city_map)[cm_sptr + 5] |= patrons << 4;
                }
            } else if (building_kind == 0xfa) {
                activity_state = ((unsigned char *)city_map)[cm_sptr + 5] & 0xf;
                ((unsigned char *)city_map)[cm_sptr + 3] |= 1;
                if (activity_state == 0) {
                    business_output(col_idx, evolve_row + row_idx);

                    cooldown = ((unsigned char *)city_map)[cm_sptr + 6] & 0xf;
                    flags = ((unsigned char *)city_map)[cm_sptr + 6] & 0x10;
                    patrons  = (((unsigned char *)city_map)[cm_sptr + 5] & 0xf0) >> 4;
                    if (cooldown == 0) {
                        if (population < 2) goto next;
                        spawn_result = put_out_a(6, (char)col_idx, (char)(evolve_row + row_idx), flags,
                                           (char)patrons, 9, 0x20);
                        if (spawn_result != 0) {
                            patrons = (unsigned char)spawn_result;
                            cooldown = 3;
                            citizen_list[created_citizen_no].state_idx = 1; citizen_list[created_citizen_no].wait_count = 0x14;
                            citizen_list[created_citizen_no].saved_state_idx = 0xa;
                            citizen_list[created_citizen_no].target_ref = cm_sptr;
                            remove_envoy();
                            ((unsigned char *)city_map)[cm_sptr + 0x12] = (unsigned char)created_citizen_no;
                        }
                        patrons++; if (patrons >= 9) patrons = 0;
                    } else {
                        cooldown--;
                    }
                    ((unsigned char *)city_map)[cm_sptr + 6] &= 0xf0;
                    ((unsigned char *)city_map)[cm_sptr + 6] |= cooldown;
                    ((unsigned char *)city_map)[cm_sptr + 5] &= 0x0f;
                    ((unsigned char *)city_map)[cm_sptr + 5] |= patrons << 4;
                }
            }
        next:
            ;
        }
    }
}

// Send a cell's previous envoy home when it is still assigned to that cell.
// FUNCTION: C2 0x41e3a
// FUNCTION: C2WIN 0x004654b5
void remove_envoy(void)
{
    citizen_a = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).industrial;
    if (citizen_list[(*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).industrial].exists) {
        if (citizen_list[citizen_a].target_ref == cm_sptr)
            citizen_list[citizen_a].state_idx = 2;
    }
}

// Select a market's sprite from its activity state and gradually decay both activity counters.
// FUNCTION: C2 0x41e7e
// FUNCTION: C2WIN 0x0046553b
void market_image(void)
{
    unsigned char shape = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind;
    unsigned char building = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building;
    unsigned char low_half = building & 0x03;
    unsigned char second_half = building & 0x0c;
    unsigned char target = low_half;
    if (second_half == 0) target = 1;

    if (shape != 0xfc + target) {
        change_sized(target + 0xfc, target * 4 + 0x30, 2, cm_sptr);
    }

    if (low_half != 0 && (evolve_tick4 & 1)) {
        (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building &= 0xfc;
        if (low_half == 2) {
            (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building |= 1;
        } else if (low_half == 3) {
            (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building |= 2;
        }
    }

    if (second_half != 0 && (evolve_tick4 & 1)) {
        (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building &= 0xf3;
        if (second_half == 8) {
            (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building |= 4;
        } else if (second_half == 0xc) {
            (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building |= 8;
        }
    }
}

// Recompute a business's production tier from nearby population, goods supply, and trade links.
// FUNCTION: C2 0x41f63
// FUNCTION: C2WIN 0x004656e6
void business_output(int cell_x, int cell_y)
{
  unsigned char building_state = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building;
  unsigned char good_idx = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).business & 0xf;
  unsigned char production_level = building_state & 3;
  unsigned char staged_level = building_state & 0xc;
  int city_supply = industry[good_idx].city_supply;
  int pipeline = industry[good_idx].supply_pipeline[0];
  int growth = ind_growth_factor;
  int nearby_population = test_area_for_population(2, cell_x, cell_y, 2);
  int supply;
  if (nearby_population > 0x82)
    production_level += 4;
  else if (nearby_population > 0x5a)
    production_level += 3;
  else if (nearby_population > 0x32)
    production_level += 2;
  else if (nearby_population > 10)
    production_level += 1;
  if (production_level <= 0)
    supply = 0;
  else
    if (production_level <= 1)
    supply = 3;
  else
    if (production_level <= 2)
    supply = 5;
  else
    if (production_level <= 3)
    supply = 7;
  else
    supply = 7;
  if (staged_level == 0)
  {
    growth -= 2;
    if (supply > 4)
      supply = 4;
  }
  if (pipeline <= 0)
    supply = 0;
  else
    if (pipeline <= 0x32)
  {
    growth -= 3;
    if (supply > 1)
      supply = 1;
  }
  else
    if (pipeline <= 0xc8)
  {
    growth -= 2;
    if (supply > 3)
      supply = 3;
  }
  else
    if (pipeline <= 0x190)
  {
    growth -= 1;
    if (supply > 5)
      supply = 5;
  }
  else
    if (pipeline > 600)
  {
    if (pipeline <= 0x320)
      growth += 1;
    else
      if (pipeline <= 0x3e8)
      growth += 2;
    else
      if (pipeline > 1000)
      growth += 3;
  }
  if (no_of_empire_connections <= 0)
  {
    if (supply > 4)
      supply = 4;
  }
  else
    if (no_of_empire_connections <= 1)
    growth += 1;
  else
    if (no_of_empire_connections > 1)
    growth += 2;
  if (city_supply <= 0)
    supply = 0;
  else
    if (city_supply <= 0x14)
  {
    if (supply > 1)
      supply = 1;
  }
  else
    if (city_supply <= 0x22)
  {
    if (supply > 2)
      supply = 2;
  }
  else
    if (city_supply <= 0x32)
  {
    if (supply > 3)
      supply = 3;
  }
  else
    if (city_supply <= 0x43)
  {
    if (supply > 4)
      supply = 4;
  }
  else
    if (city_supply <= 0x4b)
  {
    if (supply > 5)
      supply = 5;
  }
  else
    if ((city_supply <= 99) && (supply > 6))
    supply = 6;
  if (supply < growth)
    growth = supply;
  if (growth < 0)
    growth = 0;
  if (growth > 7)
    growth = 7;
  (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building &= 0xf;
  growth <<= 4;
  (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building |= growth;
  if (production_level != 0)
  {
    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building &= 0xfc;
    if (production_level == 2)
      (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building |= 1;
    else
      if (production_level == 3)
      (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building |= 2;
  }
  if (staged_level != 0)
  {
    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building &= 0xf3;
    if (staged_level == 8)
      (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building |= 4;
    else
      if (staged_level == 0xc)
      (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building |= 8;
  }
}

// Advance fires and plagues, spread them to neighboring cells, and turn severe housing unrest
// into destruction and rioters.
// FUNCTION: C2 0x42204
// FUNCTION: C2WIN 0x00465bbb
void spread_fire_and_plague_and_unrest(int row_count)
{
    int row_idx;
    int col_idx;
    unsigned char building_kind;
    unsigned char damage_count;
    signed char unrest;
    unsigned char range_flags;
    unsigned char health_flags;
    unsigned char random_value;

    cm_sptr = evolve_row * 1600;

    if (evolve_row == 0) {
        if (rand8 > 4) {
            stone_random_count++;
            if (stone_random_count >= 0x40) stone_random_count = 0;
            fire_spread_direction =
                stone_random_data[stone_random_count] & 6;
        }
        stone_random_count++;
        if (stone_random_count >= 0x40) stone_random_count = 0;
        plague_spread_direction =
            stone_random_data[stone_random_count] & 6;
        fire_spread_count    = 0;
        fire_spread_target   = rand8 & 1;
        plague_spread_count  = 0;
        plague_spread_target = rand8 & 1;
        unrest_random_count += rand8;
        if (unrest_random_count >= 0x40) unrest_random_count -= 0x40;
    }

    for (row_idx = 0; row_idx < row_count; row_idx++) {
        for (col_idx = 0; col_idx < 80; col_idx++, cm_sptr += 20) {

            if ((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).edge_bits & 0x80) {
                building_kind = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind;
                if (building_kind < 8) {
                    damage_count = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).fire - 1;
                    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).fire = damage_count;
                    if (damage_count == 0) {
                        (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).edge_bits &= 0x7f;
                    } else {
                        if (fire_spread_count++ < fire_spread_target) goto next;
                        fire_spread_target += 2;
                        if (fire_spread_direction == 0 && row_idx + evolve_row <= 0)
                            goto next;
                        if (fire_spread_direction == 4 && row_idx + evolve_row >= 0x4f)
                            goto next;
                        if (fire_spread_direction == 6 && col_idx <= 0)
                            goto next;
                        if (fire_spread_direction == 2 && col_idx >= 0x4f)
                            goto next;
                        spread_fire_atom(cm_sptr, fire_spread_direction);
                    }
                } else if (building_kind >= 0x82 && building_kind <= 0xa1) {
                    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).fpu_flag &= 0xcf;
                    damage_count = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).fire;
                    damage_count--; (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).fire = damage_count;
                    if (damage_count == 0) {
                        destroy_an_atom(cm_sptr, 0);
                    } else {
                        if (plague_spread_count++ < plague_spread_target) goto next;
                        plague_spread_target++;
                        if (plague_spread_direction == 0 && row_idx + evolve_row <= 0)
                            goto next;
                        if (plague_spread_direction == 4 && row_idx + evolve_row >= 0x4f)
                            goto next;
                        if (plague_spread_direction == 6 && col_idx <= 0)
                            goto next;
                        if (plague_spread_direction == 2 && col_idx >= 0x4f)
                            goto next;
                        if (damage_count == 9)
                            goto next;
                        spread_plague_atom(cm_sptr, plague_spread_direction);
                    }
                }
            } else {
                building_kind = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind;
                if (building_kind < 0x82 || building_kind > 0x9b) goto next;
                unrest = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a & 0xf;
                if (unrest != 0) goto next;

                unrest = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).fpu_flag & 0xf;
                range_flags = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).range_flag;
                health_flags = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).health;
                if ((range_flags & 0x0c) == 0) unrest--;
                if ((range_flags & 0x30) != 0) unrest--;
                if ((health_flags & 0x03) != 0) unrest--;
                unrest += insurrection_factor;

                unrest_random_count++;
                if (unrest_random_count >= 0x40) unrest_random_count = 0;
                random_value = unrest_random_data[unrest_random_count];
                if (random_value == 9)
                    unrest += house_type_to_unrest[building_kind - 0x82].unrest_delta;
                else
                    unrest += random_value;
                if (unrest < 0) unrest = 0;
                else if (unrest > 0xf) {
                    if (insurrection_factor > 6)
                        insurrection_factor = 6;
                    else if (insurrection_factor > 2)
                        insurrection_factor = 2;
                    destroy_an_atom(cm_sptr, 0);
                    if (put_out_a(7, (unsigned char)col_idx, (unsigned char)(evolve_row + row_idx), 0, 0, 0, 0) != 0) {
                        citizen_list[created_citizen_no].speed_phase = 0;
                        citizen_list[created_citizen_no].speed_count = 0;
                        citizen_list[created_citizen_no].state_idx = 1;
                        citizen_list[created_citizen_no].wait_count = 0x14;
                        citizen_list[created_citizen_no].saved_state_idx = 0xb;
                        if (put_a_message == 0)
                            put_message(0x57, cm_sptr, 0x15);
                    }
                    goto next;
                }
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).fpu_flag &= 0xf0;
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).fpu_flag |= unrest;
            }
next:
            ;
        }
    }
}

// Create a citizen at the requested cell or at the first usable perimeter position.
// FUNCTION: C2 0x42666
// FUNCTION: C2WIN 0x00466232
int put_out_a(char citizen_kind, char cell_x, char cell_y, int unused, char start_idx,
              char perimeter_size, char barbarian_flag)
{
  int attempt_idx;
  int position_count;
  int position_idx;
  int offset_x;
  int offset_y;
  (void) unused;
  position_idx = start_idx;
  if (perimeter_size == 1)
    position_count = 4;
  else
    if (perimeter_size == 4)
    position_count = 8;
  else
    if (perimeter_size == 9)
    position_count = 12;
  else
    if (perimeter_size == 0x10)
    position_count = 0x10;
  else
  {
    if (create_citizen(citizen_kind, cell_x, cell_y, 0) == 0)
      return 0;
    return 1;
  }
  for (attempt_idx = 0; attempt_idx < position_count; attempt_idx++)
  {
    if (position_count <= position_idx)
      position_idx = 0;
    if (perimeter_size == 1)
    {
      offset_x = putouts1[position_idx].dx;
      offset_y = putouts1[position_idx].dy;
    }
    else
      if (perimeter_size == 4)
    {
      offset_x = putouts2[position_idx].dx;
      offset_y = putouts2[position_idx].dy;
    }
    else
      if (perimeter_size == 9)
    {
      offset_x = putouts3[position_idx].dx;
      offset_y = putouts3[position_idx].dy;
    }
    else
      if (perimeter_size == 0x10)
    {
      offset_x = putouts4[position_idx].dx;
      offset_y = putouts4[position_idx].dy;
    }
    if (create_citizen(citizen_kind, cell_x + offset_x, cell_y + offset_y, barbarian_flag) != 0)
      return position_idx + 1;
    position_idx++;
  }

  return 0;
}

// Decay service coverage, update fire risk, and evolve houses, civic buildings, and plazas in one
// city-map row.
// FUNCTION: C2 0x42790
// FUNCTION: C2WIN 0x0046640d
void evolve_a_cm_row(void)
{
    unsigned char fire_zone_row;
    signed char tier_idx;
    int odd_tick_flag;
    signed char land_value_rank;
    int fourth_tick_flag;
    unsigned char amenity_low_mask;
    unsigned char building_state;
    unsigned char amenity_high_mask;
    int random_flag;
    int third_tick_flag;
    unsigned char amenity_flags;
    unsigned char amenity_mid_mask;
    unsigned char building_kind;
    signed char health_score;
    unsigned char health_flags;
    unsigned char range_flags;
    unsigned char health_flag8;
    unsigned char health_flag1;
    unsigned char health_flag2;
    unsigned char fire_zone_col;

    odd_tick_flag = third_tick_flag = fourth_tick_flag = random_flag = 0;
    if (evolve_tick4 & 1) odd_tick_flag = 1;
    if (evolve_tick3 == 0) third_tick_flag = 1;
    if (evolve_tick4 == 0) fourth_tick_flag = 1;
    if (rand8 >= 6) random_flag = 1;
    fire_zone_row = evolve_row / 8;

    city_qptr = (unsigned char *)city_map;
    city_qptr += evolve_row * 1600;
    city_ptr  = evolve_row * 1600;

    for (evolve_col = 0; evolve_col < 80; evolve_col++, city_qptr += 20) {
        building_state = city_qptr[5] & 0xf;
        building_kind = city_qptr[0];
        land_value_rank = city_qptr[0xf];

        if (third_tick_flag) {
            amenity_flags = city_qptr[0xa];
            amenity_low_mask = amenity_flags & 0xc;
            amenity_mid_mask = amenity_flags & 0x30;
            amenity_high_mask = amenity_flags & 0xc0;
            if (amenity_mid_mask) {
                city_qptr[0xa] &= 0xcf;
                if (amenity_mid_mask == 0x30) city_qptr[0xa] |= 0x20;
                else if (amenity_mid_mask == 0x20) city_qptr[0xa] |= 0x10;
            }
            if (amenity_high_mask) {
                city_qptr[0xa] &= 0x3f;
                if (amenity_high_mask == 0xc0) city_qptr[0xa] |= 0x80;
                else if (amenity_high_mask == 0x80) city_qptr[0xa] |= 0x40;
            }
            if (amenity_low_mask) {
                city_qptr[0xa] &= 0xf3;
                if (amenity_low_mask == 0xc) city_qptr[0xa] |= 8;
                else if (amenity_low_mask == 8) city_qptr[0xa] |= 4;
            }
        }

        if (fourth_tick_flag) {
            if (building_kind >= 0x82 && building_kind <= 0xa1) {
                health_flags  = city_qptr[0xd];
                range_flags = city_qptr[0xb];
                health_score = range_flags & 0x30;
                health_score >>= 4;
                health_flag8 = health_flags & 8;
                health_flag1 = health_flags & 1;
                health_flag2 = health_flags & 2;
                health_score += random_flag;
                if (health_flag8 && health_flag1) health_score -= 3;
                else if (health_flag8 && health_flag2) health_score -= 2;
                else if (health_flag1 || health_flag8) health_score -= 1;
                else if (!health_flag2) health_score += 1;
                if (hospital_cover <= 0) health_score += 1;
                else if (hospital_cover >= 0x64) health_score -= 2;
                else if (hospital_cover >= 0x4b) { if (evolve_count & 1) health_score -= 2; else health_score -= 1; }
                else if (hospital_cover >= 0x32) health_score -= 1;
                else if (hospital_cover >= 0x19 && (evolve_count & 1)) health_score -= 1;
                if (health_score <= 0) health_score = 0;
                else if (health_score == 1) health_score = 0x10;
                else if (health_score == 2) health_score = 0x20;
                else health_score = 0x30;
                city_qptr[0xb] &= 0xcf;
                city_qptr[0xb] |= health_score;
            }
        }

        if (building_kind < 8) {
            if (city_qptr[3] & 0x80) {
                fire_zone_col = evolve_col / 8;
                fire_zones[fire_zone_row * 10 + fire_zone_col] += 2;
            }
        }

        if (building_state == 0) {
            if (building_kind >= 0x82 && building_kind <= 0xa1) {
                tier_idx  = building_kind - 0x82;
                if (tier_idx >= 0x1e) land_value_rank = get_best_lv(city_qptr, 3);
                else if (tier_idx >= 0x1a) land_value_rank = get_best_lv(city_qptr, 2);
                if      (land_value_rank <  house_evolution[tier_idx].devolve_below) devolve_a_house(tier_idx);
                else if (land_value_rank >  house_evolution[tier_idx].evolve_above)  evolve_a_house(tier_idx);
            } else if (building_kind >= 0xd7 && building_kind <= 0xda) {
                tier_idx  = building_kind - 0xd7;
                if      (land_value_rank <  well_evolution[tier_idx].devolve_below) devolve_a_building(tier_idx, 1, 0xd7, 0x10, 1);
                else if (land_value_rank >  well_evolution[tier_idx].evolve_above)  evolve_a_building(tier_idx, 1, 0xd7, 0x10, 1);
            } else if (building_kind >= 0xdb && building_kind <= 0xde) {
                tier_idx  = building_kind - 0xdb;
                if      (land_value_rank <  fountain_evolution[tier_idx].devolve_below) devolve_a_building(tier_idx, 1, 0xdb, 0, 0);
                else if (land_value_rank >  fountain_evolution[tier_idx].evolve_above)  evolve_a_building(tier_idx, 1, 0xdb, 0, 0);
            } else if (building_kind >= 0xdf && building_kind <= 0xe2) {
                tier_idx  = building_kind - 0xdf;
                land_value_rank = get_best_lv(city_qptr, 2);
                if      (land_value_rank <  baths_evolution[tier_idx].devolve_below) devolve_a_building(tier_idx, 2, 0xdf, 0, 0);
                else if (land_value_rank >  baths_evolution[tier_idx].evolve_above)  evolve_a_building(tier_idx, 2, 0xdf, 0, 0);
            } else if (building_kind >= 0xae && building_kind <= 0xb1) {
                tier_idx  = building_kind - 0xae;
                land_value_rank = get_best_lv(city_qptr, 2);
                if      (land_value_rank <  forum_evolution[tier_idx].devolve_below) devolve_a_building(tier_idx, 2, 0xae, forum_gfxdat[0], 4);
                else if (land_value_rank >  forum_evolution[tier_idx].evolve_above)  evolve_a_building(tier_idx, 2, 0xae, forum_gfxdat[0], 4);
            } else if (building_kind >= 0xb2 && building_kind <= 0xb5) {
                tier_idx  = building_kind - 0xb2;
                land_value_rank = get_best_lv(city_qptr, 3);
                if      (land_value_rank <  forum_evolution[tier_idx].devolve_below) devolve_a_building(tier_idx, 3, 0xb2, forum_gfxdat[0x10], 9);
                else if (land_value_rank >  forum_evolution[tier_idx].evolve_above)  evolve_a_building(tier_idx, 3, 0xb2, forum_gfxdat[0x10], 9);
            } else if (building_kind >= 0xb6 && building_kind <= 0xb9) {
                tier_idx  = building_kind - 0xb6;
                land_value_rank = get_best_lv(city_qptr, 4);
                if      (land_value_rank <  forum_evolution[tier_idx].devolve_below) devolve_a_building(tier_idx, 4, 0xb6, forum_gfxdat[0x20], 0x10);
                else if (land_value_rank >  forum_evolution[tier_idx].evolve_above)  evolve_a_building(tier_idx, 4, 0xb6, forum_gfxdat[0x20], 0x10);
            } else if (building_kind >= 0xa2 && building_kind <= 0xa5) {
                tier_idx  = building_kind - 0xa2;
                if (land_value_rank <  temple_evolution[tier_idx].devolve_below ||
                    population <  temple_populations1[tier_idx].devolve_below) {
                    devolve_a_building(tier_idx, 1, 0xa2, 0x3c, 1);
                } else if (land_value_rank >  temple_evolution[tier_idx].evolve_above &&
                           population > temple_populations1[tier_idx].evolve_above) {
                    evolve_a_building(tier_idx, 1, 0xa2, 0x3c, 1);
                }
            } else if (building_kind >= 0xa6 && building_kind <= 0xa9) {
                tier_idx  = building_kind - 0xa6;
                land_value_rank = get_best_lv(city_qptr, 2);
                if (land_value_rank <  temple_evolution[tier_idx].devolve_below ||
                    population <  temple_populations2[tier_idx].devolve_below) {
                    devolve_a_building(tier_idx, 2, 0xa6, 0x40, 4);
                } else if (land_value_rank >  temple_evolution[tier_idx].evolve_above &&
                           population > temple_populations2[tier_idx].evolve_above) {
                    evolve_a_building(tier_idx, 2, 0xa6, 0x40, 4);
                }
            } else if (building_kind >= 0xaa && building_kind <= 0xad) {
                tier_idx  = building_kind - 0xaa;
                land_value_rank = get_best_lv(city_qptr, 3);
                if ((temple_evolution[tier_idx].devolve_below) > (land_value_rank) ||
                    population <  temple_populations3[tier_idx].devolve_below) {
                    devolve_a_building(tier_idx, 3, 0xaa, 0, 9);
                } else if ((temple_evolution[tier_idx].evolve_above) < (land_value_rank) &&
                           population > temple_populations3[tier_idx].evolve_above) {
                    evolve_a_building(tier_idx, 3, 0xaa, 0, 9);
                }
            } else if (building_kind >= 0x7c && building_kind <= 0x7e) evolve_a_plaza(land_value_rank, building_kind, evolve_col);
        }

    }
}

// Move a multi-tier civic building down one level and update its map graphics.
// FUNCTION: C2 0x42eac
// FUNCTION: C2WIN 0x0046707d
void devolve_a_building(
    int tier_idx, int footprint_size, unsigned char base_kind, unsigned char gfx_base,
    unsigned char gfx_step)
{
    unsigned char offs;
    unsigned char fflag;

    if (tier_idx <= 0)
        return;
    --tier_idx;
    if (base_kind == 0xDB) {
        fflag = city_qptr[0xD] & 4;
        offs = fountain_gfxdat[tier_idx];
        if (fflag) ++offs;
    } else if (base_kind == 0xDF) {
        fflag = city_qptr[0xD] & 4;
        if (fflag) offs = tier_idx * 4 + 0x20;
        else offs = tier_idx * 4 + 0x63;
    } else {
        offs = gfx_base + tier_idx * gfx_step;
    }
    change_sized(base_kind + tier_idx, offs, footprint_size,
                 city_ptr + evolve_col * 20);
}

// Move a multi-tier civic building up one level and update its map graphics.
// FUNCTION: C2 0x42f46
// FUNCTION: C2WIN 0x0046716d
void evolve_a_building(
    int tier_idx, int footprint_size, unsigned char base_kind, unsigned char gfx_base,
    unsigned char gfx_step)
{
    unsigned char offs;
    unsigned char fflag;

    if (tier_idx >= 3)
        return;
    ++tier_idx;
    if (base_kind == 0xDB) {
        fflag = city_qptr[0xD] & 4;
        offs = fountain_gfxdat[tier_idx];
        if (fflag)
            ++offs;
    } else if (base_kind == 0xDF) {
        fflag = city_qptr[0xD] & 4;
        if (fflag)
            offs = tier_idx * 4 + 0x20;
        else
            offs = tier_idx * 4 + 0x63;
    } else {
        offs = gfx_base + tier_idx * gfx_step;
    }
    change_sized(base_kind + tier_idx, offs, footprint_size,
                 city_ptr + evolve_col * 20);
}

// Demote a house, shrinking its footprint when necessary or removing the lowest tier.
// FUNCTION: C2 0x42f5d
// FUNCTION: C2WIN 0x0046725d
int devolve_a_house(int tier_idx)
{
    unsigned int new_size;
    unsigned int old_size;

    old_size = house_gfxdat[tier_idx*4 + 1];
    --tier_idx;
    if (tier_idx < 0) {
        remove_house();
        return 0;
    }
    new_size = house_gfxdat[tier_idx*4 + 1] & 0xff;
    change_house(tier_idx, new_size, 0);
    if (old_size != new_size) {
        pad_house_with_domus(new_size);
    }
    return 1;
}

// Promote a house when the next tier's footprint can fit on the surrounding cells.
// FUNCTION: C2 0x42fa5
// FUNCTION: C2WIN 0x004672d2
int evolve_a_house(int tier_idx)
{
    unsigned int curr;
    unsigned int next;
    int delta;

    curr = house_gfxdat[tier_idx*4 + 1];
    ++tier_idx;
    if (tier_idx >= 0x20) return 0;
    next = house_gfxdat[tier_idx*4 + 1];
    delta = 0;
    if (curr != next) {
        delta = stretch_house(tier_idx, next);
        if (delta == 0) return 0;
        delta--;
    }
    change_house(tier_idx, next, delta);
    return 1;
}

// Find an orientation in which a house can expand to the requested 2x2 or 3x3 footprint.
// FUNCTION: C2 0x42ff4
// FUNCTION: C2WIN 0x0046736a
int stretch_house(int tier_idx, int footprint_size)
{
    if (footprint_size == 2) {
        if (stretch_to_2x2_house(tier_idx, footprint_size, 0)) return 1;
        if (stretch_to_2x2_house(tier_idx, footprint_size, 1)) return footprint_size;
        if (stretch_to_2x2_house(tier_idx, footprint_size, footprint_size)) return 3;
        if (stretch_to_2x2_house(tier_idx, footprint_size, 3)) return 4;
    } else if (footprint_size == 3) {
        if (stretch_to_3x3_house(tier_idx, footprint_size, 0)) return 1;
        if (stretch_to_3x3_house(tier_idx, footprint_size, 1)) return 2;
        if (stretch_to_3x3_house(tier_idx, footprint_size, 2)) return footprint_size;
        if (stretch_to_3x3_house(tier_idx, footprint_size, footprint_size)) return 4;
    }
    return 0;
}

// Check whether the cells needed for a 2x2 house expansion are clear or compatible housing.
// FUNCTION: C2 0x430af
// FUNCTION: C2WIN 0x004674b5
int stretch_to_2x2_house(int tier_idx, int unused, int orientation)
{
    unsigned char *cell_ptr;
    int offset_idx;
    int cell_offset;
    unsigned char cell_flags;
    unsigned char cell_kind;
    unsigned char cell_fire;
    unsigned char cell_plague;

    (void)unused;
    if (evolve_col == 0x4f) { fail: return 0; }
    if (evolve_row == 0x4f) goto fail;
    if (evolve_col == 0) goto fail;
    if (evolve_row == 0) goto fail;
    for (offset_idx = 0; offset_idx < 3; offset_idx++) {
        cell_offset = stretch_ofsets_2x2[orientation][offset_idx];
        cell_ptr = city_qptr + cell_offset;
        cell_flags = (unsigned char)cell_ptr[1];
        cell_kind = (unsigned char)cell_ptr[0];
        cell_fire = (unsigned char)cell_ptr[7];
        cell_plague = (unsigned char)cell_ptr[8];
        if (cell_fire != 0 || cell_plague != 0) goto fail;
        if ((cell_flags & 0xfe) != 0) goto fail;
        if ((cell_flags & 1) != 0 && cell_kind >= tier_idx + 0x82) goto fail;
    }
    return 1;
}

// Check a 3x3 house expansion and reduce overlapping villas to individual domus cells.
// FUNCTION: C2 0x4313d
// FUNCTION: C2WIN 0x004675ef
int stretch_to_3x3_house(int tier_idx, int unused, int orientation)
{
    int cell_offset;
    unsigned char cell_plague;
    unsigned char cell_flags;
    unsigned char cell_fire;
    int villa_y;
    int villa_x;
    int offset_idx;
    unsigned char cell_kind;
    unsigned char *cell_ptr;

    if (evolve_col >= 0x4e) return 0;
    if (evolve_row >= 0x4e) return 0;
    if (evolve_col <= 0) return 0;
    if (evolve_row <= 0) return 0;
    for (offset_idx = 0; offset_idx < 5; offset_idx++) {
        cell_offset = *((int *)stretch_ofsets_3x3 + (unsigned int)(orientation * 5) + offset_idx);
        cell_flags = city_qptr[cell_offset + 1];
        cell_kind = city_qptr[cell_offset];
        cell_fire = city_qptr[cell_offset + 7];
        cell_plague = city_qptr[cell_offset + 8];
        if (cell_fire != 0 || cell_plague != 0) return 0;
        if ((cell_flags & 0xfe) != 0) return 0;
        if ((cell_flags & 1) != 0 && cell_kind >= tier_idx + 0x82) return 0;
    }
    for (offset_idx = 0; offset_idx < 5; offset_idx++) {
        cell_offset = stretch_ofsets_3x3[orientation][offset_idx];
        cell_kind = city_qptr[cell_offset];
        if (cell_kind >= 0x9c && cell_kind <= 0x9f) {
            villa_y = city_qptr[cell_offset + 5] & 0xf;
            villa_x = villa_y;
            villa_x %= 2;
            villa_y /= 2;
            cell_ptr = city_qptr + cell_offset;
            cell_ptr -= villa_x * 20;
            cell_ptr -= villa_y * 1600;
            reduce_villa_to_domus(cell_ptr);
        }
    }
    return 1;
}

// Stamp a house tier and its graphics across a selected map footprint.
// FUNCTION: C2 0x43255
// FUNCTION: C2WIN 0x00467804
void change_house(int tier_idx, int footprint_size, int orientation)
{
    int base_gfx_idx = (unsigned char)house_gfxdat[tier_idx * 4];
    int row_skip = (80 - footprint_size) * 20;
    unsigned char *cell_ptr = city_qptr;
    int row_idx;
    int col_idx;
    int footprint_idx;
    unsigned char gfx_offset;

    if (orientation != 0) {
        if (orientation == 1) cell_ptr -= 20;
        else if (orientation == 2) cell_ptr -= 0x654;
        else if (orientation == 3) cell_ptr -= 0x640;
    }

    for (row_idx = 0, footprint_idx = 0; row_idx < footprint_size; row_idx++, cell_ptr += row_skip) {
        for (col_idx = 0; col_idx < footprint_size; col_idx++, cell_ptr += 20, footprint_idx++) {
            if ((unsigned char)cell_ptr[0] < 0x82) cell_ptr[3] &= 0x7f;
            cell_ptr[0] = (char)(tier_idx + 0x82);
            cell_ptr[1] |= 1;
            cell_ptr[3] |= 1;
            cell_ptr[3] &= 0xc3;
            cell_ptr[5] = footprint_idx;
            if (footprint_size == 1) {
                cell_ptr[4] = base_gfx_idx;
            } else if (footprint_size == 2) {
                gfx_offset = diamond_ofsets_2x[footprint_idx];
                cell_ptr[4] = (char)(base_gfx_idx + gfx_offset);
            } else if (footprint_size == 3) {
                gfx_offset = diamond_ofsets_3x[footprint_idx];
                cell_ptr[4] = (char)(base_gfx_idx + gfx_offset);
            }
        }
    }
}

// Fill the row and column freed by a house shrink with individual domus cells.
// FUNCTION: C2 0x4332a
// FUNCTION: C2WIN 0x004679ae
void pad_house_with_domus(int previous_size)
{
    int gfx_idx;
    int row_stride;
    unsigned char *cm;
    int y;
    int x;

    gfx_idx    = house_gfxdat[0x64];
    row_stride = (80 - previous_size) * 20;
    cm      = city_qptr;

    for (y = 0; y < previous_size; y++) {
        x = 0;
        for ( ; x < previous_size; x++, cm += 20)
            ;
        cm[0]  = 0x9b;
        cm[3] |= 1;
        cm[4]  = gfx_idx;
        cm[5]  = 0;
        cm    += row_stride;
    }

    for (x = 0; x <= previous_size; x++, cm += 20) {
        cm[0]  = 0x9b;
        cm[3] |= 1;
        cm[4]  = gfx_idx;
        cm[5]  = 0;
    }
}

// Convert a 2x2 villa rooted at `cm` into four 1x1 domus cells.
// FUNCTION: C2 0x433a1
// FUNCTION: C2WIN 0x00467a9b
void reduce_villa_to_domus(unsigned char *cell_ptr)
{
    int row_idx;
    int col_idx;

    for (row_idx = 0; row_idx < 2; row_idx++, cell_ptr += CITY_ROW - 2 * CITY_CELL_BYTES) {
        for (col_idx = 0; col_idx < 2; col_idx++, cell_ptr += CITY_CELL_BYTES) {
            ((cell_ptr)[0])        = 0x9b;
            ((cell_ptr)[3])  |= 1;
            ((cell_ptr)[4])  = house_gfxdat[0x64];
            ((cell_ptr)[5])  = 0;
        }
    }
}

// Clear a house cell's state and return it to undeveloped ground.
// FUNCTION: C2 0x433d4
// FUNCTION: C2WIN 0x00467b19
void remove_house(void)
{
    city_qptr[0x00] = 0x1a;
    city_qptr[0x09] = 0;
    city_qptr[0x01] &= 0x18;
    city_qptr[0x03] &= 0xe3;
    city_qptr[0x03] &= 0xdf;
    city_qptr[0x03] &= 0x7f;
    city_qptr[0x0a] &= 0xfc;
    city_qptr[0x0b] &= 0xcf;
    city_qptr[0x0b] &= 0xf0;
    city_qptr[0x03] |= 1;
    city_qptr[0x05] = 0;
}

// Select a plaza tier from land value and update its surrounding coverage when the tier changes.
// FUNCTION: C2 0x43437
// FUNCTION: C2WIN 0x00467be6
void evolve_a_plaza(signed char land_value, signed char old_kind, int cell_x)
{
    if ((evolve_row & 1) != 0) return;
    if ((cell_x & 1) != 0) return;
    if (city_qptr[7] != 0) return;
    if (city_qptr[8] != 0) return;
    if ((signed char)land_value > 0x28) {
        city_qptr[0] = 0x7e;
        city_qptr[4] = 0x76;
    } else if ((signed char)land_value > 0x14) {
        city_qptr[0] = 0x7d;
        city_qptr[4] = 0x75;
    } else {
        city_qptr[0] = 0x7c;
        city_qptr[4] = 0x74;
    }
    if ((unsigned char)old_kind != city_qptr[0]) {
        flag_range(0, cell_x, evolve_row, 1, 3, 1);
    }
}

// Decay and clamp every entry in the city's 10x10 fire-risk grid.
// FUNCTION: C2 0x434bb
// FUNCTION: C2WIN 0x00467cc7
void clear_fire_zones(void)
{
    int zone_x;
    int zone_y;
    int zone_idx;

    for (zone_x = 0; zone_x < 10; zone_x++) {
        for (zone_y = 0; zone_y < 10; zone_y++) {
            zone_idx = zone_x * 10 + zone_y;
            if (fire_zones[zone_idx] > 2)        fire_zones[zone_idx] = 2;
            else if (fire_zones[zone_idx] > 1)   fire_zones[zone_idx] = 1;
            else if (fire_zones[zone_idx] <= 1)  fire_zones[zone_idx] = 0;
        }
    }
}

// Propagate wall security values across the city map in one rotating sweep direction.
// FUNCTION: C2 0x4350a
// FUNCTION: C2WIN 0x00467d9c
void push_shell(int row_count)
{
    int cell_stride;
    unsigned char neighbour_found;
    int direction;
    int row_idx;
    int row_stride;
    int cell_idx;
    unsigned char neighbour_security;
    unsigned char neighbour_wall;
    int start_offset;
    char new_security;
    unsigned char propagated_security;
    unsigned char wall_flags;

    if (evolve_row == 0) {
        shell_push_direction++;
        if (shell_push_direction >= 4) shell_push_direction = 0;
    }
    if (shell_push_direction == 0)      { row_stride = 20;   cell_stride =  1600; start_offset = 0;       }
    else if (shell_push_direction == 1) { row_stride = 1600; cell_stride =   -20; start_offset = 0x62c;   }
    else if (shell_push_direction == 2) { row_stride = 20;   cell_stride = -1600; start_offset = 0x1edc0; }
    else if (shell_push_direction == 3) { row_stride = 1600; cell_stride =    20; start_offset = 0;       }

    for (row_idx = 0; row_idx < row_count; row_idx++) {
        cm_sptr = start_offset + (evolve_row + row_idx) * row_stride;
        propagated_security      = 0xf8;
        neighbour_found  = 0;
        for (cell_idx = 0; cell_idx < 80; cell_idx++, cm_sptr += cell_stride) {
            wall_flags = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).terrain & 0x1e;
            if (wall_flags != 0) {
                neighbour_wall   = 1;
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).security    = 100;
                direction = shell_push_direction;
                if (direction == 0) {
                    if (cell_idx > 0) {
                        neighbour_wall = (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr) - 20))).terrain & 0x1e;
                        neighbour_security = (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr) - 20))).security;
                    }
                } else if (direction == 1) {
                    if (cell_idx > 0) {
                        neighbour_wall = (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr) - 1600))).terrain & 0x1e;
                        neighbour_security = (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr) - 1600))).security;
                    }
                } else if (direction == 2) {
                    if (cell_idx < 0x4f) {
                        neighbour_wall = (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr) + 20))).terrain & 0x1e;
                        neighbour_security = (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr) + 20))).security;
                    }
                } else if (direction == 3) {
                    if (cell_idx < 0x4f) {
                        neighbour_wall = (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr) + 1600))).terrain & 0x1e;
                        neighbour_security = (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr) + 1600))).security;
                    }
                }
                if (neighbour_wall == 0) {
                    propagated_security = neighbour_security;
                } else {
                    neighbour_found = 1;
                    propagated_security = 100;
                }
            } else if (neighbour_found) {
                new_security = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).security;
                if ((signed char)new_security < 100) new_security++;
                if ((signed char)new_security > (signed char)propagated_security) new_security = propagated_security;
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).security = new_security;
                propagated_security  = new_security;
            } else {
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).security = propagated_security;
            }
        }
    }
}

// Evolve regional settlements and industries, process warehouse deliveries, and add produced goods
// to city supply pipelines.
// FUNCTION: C2 0x436ab
// FUNCTION: C2WIN 0x00468079
void evolve_region(int row_count)
{
    int test_value;
    int col_idx;
    unsigned char corner_state;
    unsigned char road_flag;
    int pop_level;
    unsigned char outside_walls_flag;
    unsigned char gfx_idx;
    unsigned char trader_direction;
    unsigned char current_tier;
    int row_idx;
    int workforce_level;
    int level_value;
    int production_penalty;
    unsigned char building_kind;
    unsigned char good_idx;
    int shifted_value;

    if (c2inf.peace_mode != 0) {
        return;
    }

    pop_level = get_pop_level();
    workforce_level = slave_requirements[5].current;
    if (no_of_workcamps != 0) workforce_level /= no_of_workcamps;
    else workforce_level = 0;
    workforce_level /= 10;
    if (workforce_level > 3) workforce_level = 3;
    if (workforce_level < 0) workforce_level = 0;

    if (evolve_row == 0)
        cmu_count[4] = 0;

    test_value = (province_difficulty + rand8) * 4;
    if      (test_value > 0x3c) production_penalty = 3;
    else if (test_value > 0x30) production_penalty = 2;
    else if (test_value > 0x20) production_penalty = 1;
    else               production_penalty = 0;

    cm_sptr = evolve_row * 480;

    for (row_idx = 0; row_idx < row_count; row_idx++)
        for (col_idx = 0; col_idx < 60; col_idx++, cm_sptr += 8) {
            corner_state = ((unsigned char *)region_map)[cm_sptr + 7] & 3;
            building_kind = ((unsigned char *)region_map)[cm_sptr];
            if (building_kind == 0xd4) corner_state = 0;
            if (corner_state != 0) continue;

            gfx_idx = ((unsigned char *)region_map)[cm_sptr + 4];
            road_flag  = ((unsigned char *)region_map)[cm_sptr + 3] & 0x20;
            outside_walls_flag  = ((unsigned char *)region_map)[cm_sptr + 6] & 0x40;

            if (building_kind == 0x92 && cmu_count[4] == 0) {
                level_value = pop_level;
                cmu_count[4] = 1;
                current_tier = gfx_idx / 4;
                if (current_tier < level_value) change_reg_sized(building_kind, gfx_idx + 4, 2, cm_sptr);
                if (level_value < current_tier) change_reg_sized(building_kind, gfx_idx - 4, 2, cm_sptr);
            } else if (building_kind >= 0x98 && building_kind <= 0x9b) {
                level_value = (pop_level - 1) / 2;
                if (road_flag && outside_walls_flag) level_value += 2;
                else if (road_flag)   level_value += 4;
                current_tier = gfx_idx - 0x50;
                if (level_value > current_tier) change_reg_sized(building_kind, gfx_idx + 1, 1, cm_sptr);
                if (level_value < current_tier) change_reg_sized(building_kind, gfx_idx - 1, 1, cm_sptr);
            } else if (building_kind == 0x97) {
                level_value = (pop_level - 1) / 2;
                if (road_flag && outside_walls_flag) level_value += 2;
                else if (road_flag)   level_value += 4;
                current_tier = gfx_idx - 0x32;
                if (level_value > current_tier) change_reg_sized(building_kind, gfx_idx + 1, 1, cm_sptr);
                if (level_value < current_tier) change_reg_sized(building_kind, gfx_idx - 1, 1, cm_sptr);
            } else if (building_kind == 0xd3) {
                level_value = workforce_level;
                current_tier = gfx_idx - 0x3c;
                if (current_tier < level_value) change_reg_sized(building_kind, gfx_idx + 1, 1, cm_sptr);
                if (level_value < current_tier) change_reg_sized(building_kind, gfx_idx - 1, 1, cm_sptr);
            } else if (building_kind == 0xd4) {
                ;
            } else if (building_kind >= 0xdc && building_kind <= 0xdf) {
                level_value = get_reg_buildings_in_radius(col_idx, evolve_row + row_idx, 2, 1, 0xd3);
                level_value = workforce_level * level_value;
                if (road_flag && outside_walls_flag) level_value--;
                else if (!road_flag) level_value = 0;
                if (level_value > 3) level_value = 3; else if (level_value < 0) level_value = 0;
                current_tier = building_kind - 0xdc;
                if (level_value > current_tier) change_reg_sized(building_kind + 1, gfx_idx + 4, 2, cm_sptr);
                if (level_value < current_tier) change_reg_sized(building_kind - 1, gfx_idx - 4, 2, cm_sptr);
                good_idx = ((unsigned char *)region_map)[cm_sptr + 7] & 0xf0;
                good_idx >>= 4;
                if (level_value > production_penalty) {
                    level_value -= production_penalty;
                    fill_warehouses_with(col_idx, evolve_row + row_idx, level_value, good_idx, 0);
                    industry[good_idx].supply_pipeline[1] += level_value;
                }
            } else if (building_kind >= 0xe0 && building_kind <= 0xe3) {
                level_value = get_reg_buildings_in_radius(col_idx, evolve_row + row_idx, 2, 1, 0xd3);
                level_value = workforce_level * level_value;
                if (road_flag && outside_walls_flag) level_value--;
                else if (!road_flag) level_value = 0;
                if (level_value > 3) level_value = 3; else if (level_value < 0) level_value = 0;
                current_tier = building_kind - 0xe0;
                if (level_value > current_tier) change_reg_sized(building_kind + 1, gfx_idx + 4, 2, cm_sptr);
                if (level_value < current_tier) change_reg_sized(building_kind - 1, gfx_idx - 4, 2, cm_sptr);
                good_idx = ((unsigned char *)region_map)[cm_sptr + 7] & 0xf0;
                good_idx >>= 4;
                if (good_idx == 0) {
                    good_idx = region_sources[province_is].choices[3];
                    good_idx <<= 4;
                    ((unsigned char *)region_map)[cm_sptr + 7] |= good_idx;
                    good_idx >>= 4;
                }
                if (level_value > production_penalty) {
                    level_value -= production_penalty;
                    fill_warehouses_with(col_idx, evolve_row + row_idx, level_value, good_idx, 0);
                    industry[good_idx].supply_pipeline[1] += level_value;
                }
            } else if (building_kind >= 0xe4 && building_kind <= 0xe7) {
                level_value = get_reg_buildings_in_radius(col_idx, evolve_row + row_idx, 2, 1, 0xd3);
                level_value = workforce_level * level_value;
                if (road_flag && outside_walls_flag) level_value--;
                else if (!road_flag) level_value = 0;
                if (level_value > 3) level_value = 3; else if (level_value < 0) level_value = 0;
                current_tier = building_kind - 0xe4;
                if (level_value > current_tier) change_reg_sized(building_kind + 1, gfx_idx + 4, 2, cm_sptr);
                if (level_value < current_tier) change_reg_sized(building_kind - 1, gfx_idx - 4, 2, cm_sptr);
                good_idx = ((unsigned char *)region_map)[cm_sptr + 7] & 0xf0;
                good_idx >>= 4;
                if (good_idx == 0) {
                    good_idx = region_sources[province_is].choices[6];
                    good_idx <<= 4;
                    ((unsigned char *)region_map)[cm_sptr + 7] |= good_idx;
                    good_idx >>= 4;
                }
                if (level_value > production_penalty) {
                    level_value -= production_penalty;
                    fill_warehouses_with(col_idx, evolve_row + row_idx, level_value, good_idx, 0);
                    industry[good_idx].supply_pipeline[1] += level_value;
                }
            } else if (building_kind >= 0xe8 && building_kind <= 0xeb) {
                level_value = ((unsigned char *)region_map)[cm_sptr + 7] & 0x1c;
                level_value >>= 2;
                if (level_value != 0) level_value--;
                shifted_value = level_value; shifted_value <<= 2;
                ((unsigned char *)region_map)[cm_sptr + 7] &= 0xe3;
                ((unsigned char *)region_map)[cm_sptr + 7] |= shifted_value;
                if (road_flag && outside_walls_flag) level_value--;
                else if (!road_flag) level_value = 0;
                if (level_value > 3) level_value = 3; else if (level_value < 0) level_value = 0;
                current_tier = building_kind - 0xe8;
                if (level_value > current_tier) change_reg_sized(building_kind + 1, gfx_idx + 4, 2, cm_sptr);
                if (level_value < current_tier) change_reg_sized(building_kind - 1, gfx_idx - 4, 2, cm_sptr);
                trader_direction = ((unsigned char *)region_map)[cm_sptr + 7] & 0x60;
                if      (trader_direction == 0)    good_idx = north_trader_brings;
                else if (trader_direction == 0x20) good_idx = east_trader_brings;
                else if (trader_direction == 0x40) good_idx = south_trader_brings;
                else if (trader_direction == 0x60) good_idx = west_trader_brings;
                fill_warehouses_with(col_idx, evolve_row + row_idx, level_value, good_idx, 0);
            } else if (building_kind >= 0xec && building_kind <= 0xef) {
                test_value = get_reg_buildings_in_radius(col_idx, evolve_row + row_idx, 2, 1, 0xd5);
                if (test_value != 0) ((unsigned char *)region_map)[cm_sptr + 7] |= 0x80;
                else ((unsigned char *)region_map)[cm_sptr + 7] &= 0x7f;
                level_value = ((unsigned char *)region_map)[cm_sptr + 7] & 0x1c;
                level_value >>= 2;
                if (level_value != 0 && (evolve_tick4 & 1)) level_value--;
                shifted_value = level_value; shifted_value <<= 2;
                ((unsigned char *)region_map)[cm_sptr + 7] &= 0xe3;
                ((unsigned char *)region_map)[cm_sptr + 7] |= shifted_value;
                if (road_flag && outside_walls_flag) level_value--;
                else if (!road_flag) level_value = 0;
                if (level_value > 3) level_value = 3; else if (level_value < 0) level_value = 0;
                current_tier = building_kind - 0xec;
                if (level_value > current_tier) change_reg_sized(building_kind + 1, gfx_idx + 4, 2, cm_sptr);
                if (level_value < current_tier) change_reg_sized(building_kind - 1, gfx_idx - 4, 2, cm_sptr);
            }
        }
}

// Return the population bracket used to determine regional settlement growth.
// FUNCTION: C2 0x43f9f
// FUNCTION: C2WIN 0x00468d77
int get_pop_level(void)
{
    if (population > 12000) return 7;
    if (population >  8000) return 6;
    if (population >  4000) return 5;
    if (population >  2000) return 4;
    if (population >  1000) return 3;
    if (population >   500) return 2;
    if (population >   250) return 1;
    return 0;
}

// Recompute each industry's city supply from regional warehouses and update unloading warehouses.
// FUNCTION: C2 0x44013
// FUNCTION: C2WIN 0x00468e5d
void check_goods_in_region_warehouses(void)
{
    int i;
    int col_idx;
    unsigned char delivered_now;
    int supply_capacity;
    unsigned char goods_idx;
    unsigned char building_kind;
    unsigned char remaining_unit;
    unsigned char warehouse_gfx_idx;
    int delivered_count;

    if (c2inf.peace_mode) {
        for (i = 0; i < 16; i++) {
            industry[i].city_supply = city_level_good_supply[i];
            industry[i].status = 0;
            industry[i].status = i & 1;
            if (industry[i].status) industry[i].status++;
        }
        return;
    }

    for (i = 0; i < 16; i++) {
        industry[i].count     = 0;
        industry[i].supply    = 0;
        industry[i].delivered = 0;
    }

    i = 0; cm_sptr = 0;
    for (; i < 60; i++) {
        for (col_idx = 0; col_idx < 60; col_idx++, cm_sptr += 8) {
            building_kind = ((unsigned char *)region_map)[cm_sptr];
            if (building_kind == 0xd4) {
                delivered_now = ((unsigned char *)region_map)[cm_sptr + 7] & 0x0f;
                goods_idx = ((unsigned char *)region_map)[cm_sptr + 7] & 0xf0;
                goods_idx >>= 4;
                goods_idx &= 0xf;
                industry[goods_idx].count++;
                if (delivered_now != 0) {
                    industry[goods_idx].status = 2;
                    remaining_unit = industry[goods_idx].unit_size;
                    if (delivered_now <= remaining_unit) {
                        remaining_unit -= delivered_now;
                        industry[goods_idx].delivered += delivered_now;
                        delivered_now = 0;
                    } else {
                        delivered_now -= remaining_unit;
                        industry[goods_idx].delivered += remaining_unit;
                        industry[goods_idx].supply += delivered_now;
                        remaining_unit = 0;
                    }
                    industry[goods_idx].unit_size = remaining_unit;
                    ((unsigned char *)region_map)[cm_sptr + 7] &= 0xf0;
                    ((unsigned char *)region_map)[cm_sptr + 7] |= delivered_now;
                    if (delivered_now < 0xf) warehouse_gfx_idx = delivered_now + 11;
                    else warehouse_gfx_idx = 0x24;
                    change_reg_sized(building_kind, warehouse_gfx_idx, 1, cm_sptr);
                }
            } } }

    for (i = 0; i < 16; i++) {
        delivered_count = industry[i].delivered;
        supply_capacity = industry[i].has_supply;
        if (supply_capacity) industry[i].city_supply = valueDIVtotal(delivered_count, supply_capacity);
        else industry[i].city_supply = 0;
    }
}
