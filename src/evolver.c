#include "c2_data.h"
#include "c2_types.h"

extern int affected_by_cover1(unsigned char *cell_ptr, int range, unsigned char mask);
extern int affected_by_cover2(unsigned char *cell_ptr, int range, unsigned char mask);
extern unsigned char *get_ptr_to_corner(unsigned char *base_ptr, int size);
void flag_range(int, int, int, int, unsigned char, unsigned char);
void change_sized(int, int, int, int);
void flag_range3(int, int, int, int, int, unsigned char, unsigned char, unsigned char);
void change_lv(int, int, int, int, int);
void destroy_an_atom(int, int);
void put_message(int, int, int);
void spread_fire_atom(int, int);
void spread_plague_atom(int, int);
void fill_warehouses_with(int, int, int, int, int);
void change_reg_sized(int, int, int, int);
int get_best_lv(unsigned char *, int);
void get_population_growth_factor(void);
int get_range1(unsigned char *, int, unsigned char);
int get_range3(unsigned char *, int, unsigned char);
int test_area_for_population(int, int, int, int);
void setup_map_screen_refresh(void);
#if !PLATFORM_WINDOWS
int put_out_a(char, char, char, char, char, char, char);
#endif

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
    if (evolve_clock > 0xd6) {
        evolve_tick4++; if (evolve_tick4 >= 4) evolve_tick4 = 0;
        evolve_tick3++; if (evolve_tick3 >= 3) evolve_tick3 = 0;
        evolve_clock = 0;
        evolve_count++;
        update_time();
        update_map = 1;
#if !PLATFORM_WINDOWS
        setup_map_screen_refresh();
#endif
    }
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
void evolve_water_supply_baths_industry(int rows)
{
    int count;
    int x;
    unsigned char kind;
    unsigned char bits;

    cm_sptr = evolve_row * 1600;
    for (count = 0; count < rows; count++) {
        for (x = 0; x < 80; x++, cm_sptr += 20) {
            kind = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind;
            if (kind == 0xbe) {
                bits = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).range_flag & 3;
                if      (bits == 3) flag_range(0, x, evolve_row + count, 6, 0x0d, 4);
                else if (bits == 2) flag_range(0, x, evolve_row + count, 5, 0x0d, 4);
                else if (bits == 1) flag_range(0, x, evolve_row + count, 4, 0x0d, 4);
            } else if (kind >= 0xfc && kind <= 0xff) {
                flag_range(0, x, evolve_row + count, 2, 0x0d, 0x40);
            } else if (kind == 0xfa) {
                flag_range(0, x, evolve_row + count, 4, 0x0e, 0x20);
                flag_range(0, x, evolve_row + count, 2, 0x0e, 0x10);
                flag_range(0, x, evolve_row + count, 1, 0x0d, 0x80);
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

int create_citizen();

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

int devolve_a_house();
int find_enemy();
int get_reg_buildings_in_radius();

// Stamp health, education, and entertainment coverage around active amenity buildings.
// FUNCTION: C2 0x4077b
// FUNCTION: C2WIN 0x00462e70
void evolve_amenity_cover(int rows)
{
    int count;
    int x;
    unsigned char activity;
    unsigned char kind;

    cm_sptr = evolve_row * 1600;

    for (count = 0; count < rows; count++) {
        for (x = 0; x < 80; x++, cm_sptr += 20) {
            kind = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind;
            activity = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a & 0x0f;
            if (activity != 0) continue;

            if (kind == 0xf3) {
                flag_range(1, x, evolve_row + count, 6, 0xd, 0x10);
            } else if (kind == 0xf4) {
                flag_range(2, x, evolve_row + count, 8, 0xd, 0x20);
            } else if (kind == 0xe5) {
                flag_range3(1, x, evolve_row + count, 9, 0xc, 1, 3, 0xfc);
                flag_range3(1, x, evolve_row + count, 7, 0xc, 2, 3, 0xfc);
                flag_range3(1, x, evolve_row + count, 5, 0xc, 3, 3, 0xfc);
            } else if (kind == 0xe6) {
                flag_range3(1, x, evolve_row + count, 11, 0xc, 1, 3, 0xfc);
                flag_range3(1, x, evolve_row + count,  9, 0xc, 2, 3, 0xfc);
                flag_range3(1, x, evolve_row + count,  7, 0xc, 3, 3, 0xfc);
            } else if (kind == 0xe7) {
                flag_range3(2, x, evolve_row + count, 9, 0xc, 4, 0xc, 0xf3);
                flag_range3(2, x, evolve_row + count, 7, 0xc, 8, 0xc, 0xf3);
                flag_range3(2, x, evolve_row + count, 5, 0xc, 0xc, 0xc, 0xf3);
            } else if (kind == 0xe8) {
                flag_range3(2, x, evolve_row + count, 11, 0xc, 4, 0xc, 0xf3);
                flag_range3(2, x, evolve_row + count,  9, 0xc, 8, 0xc, 0xf3);
                flag_range3(2, x, evolve_row + count,  7, 0xc, 0xc, 0xc, 0xf3);
            } else if (kind == 0xe9 || kind == 0xea || kind == 0xeb || kind == 0xec) {
                flag_range3(2, x, evolve_row + count, 10, 0xc, 0x10, 0x30, 0xcf);
                flag_range3(2, x, evolve_row + count,  8, 0xc, 0x20, 0x30, 0xcf);
                flag_range3(2, x, evolve_row + count,  6, 0xc, 0x30, 0x30, 0xcf);
            } else if (kind == 0xed || kind == 0xee || kind == 0xef || kind == 0xf0) {
                flag_range3(3, x, evolve_row + count, 12, 0xc, 0x10, 0x30, 0xcf);
                flag_range3(3, x, evolve_row + count, 10, 0xc, 0x20, 0x30, 0xcf);
                flag_range3(3, x, evolve_row + count,  8, 0xc, 0x30, 0x30, 0xcf);
            }
        }
    }
}

// Apply each cell's local land-value effects and track the highest-valued point in the city.
// FUNCTION: C2 0x40ac5
// FUNCTION: C2WIN 0x004632ce
void evolve_land_value(int rows)
{
    int count;
    int x;
    unsigned char kind;
    unsigned char map_flags;
    unsigned char building_type;
    unsigned char index;
    unsigned char activity_count;
    int radius;
    int value_change;
    int growth_factor;
    int size;
    signed char curr_lv;
    int value;

    if (evolve_row == 0) top_lv = 0;
    cm_sptr = evolve_row * 1600;

    for (count = 0; count < rows; count++) {
        for (x = 0; x < 80; x++, cm_sptr += 20) {
            map_flags = ((unsigned char *)city_map)[cm_sptr + 1];
            kind  = ((unsigned char *)city_map)[cm_sptr];

            if ((map_flags & 0x20) != 0) {
                if ((map_flags & 0x04) != 0) {
                    change_lv(x, count + evolve_row, 1, 1, 2);
                } else if ((map_flags & 0x10) != 0) {
                    change_lv(x, count + evolve_row, 2, 1, 1);
                } else if (kind == 0x5c) {
                    change_lv(x, count + evolve_row, 1, 1, 2);
                } else if (kind >= 0x58 && kind <= 0x5b) {
                    change_lv(x, count + evolve_row, 1, 1, 1);
                }
            } else {
                if ((map_flags & 0x01) != 0) {
                building_type = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind;
                activity_count = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a & 0xf;
                if (activity_count != 0) continue;
                if (building_type >= 0x82 && building_type <= 0xa1) {
                        building_type -= 0x82;
                        radius = house_lv_effect[building_type].radius;
                        value_change  = house_lv_effect[building_type].delta;
                        size   = house_gfxdat[building_type*4 + 1];
                        value_change += pop_growth_factor;
                        if      (building_type < 0x1a) growth_factor = 0;
                        else if (building_type < 0x1e) growth_factor = pop_growth_factor * 2;
                        else                 growth_factor = pop_growth_factor * 4;
                        if (growth_factor < 0) growth_factor = 0;
                        (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).land_value += growth_factor;
                        change_lv(x, count + evolve_row, radius, size, value_change);
                    } else if (building_type >= 0xdb && building_type <= 0xde) {
                        value_change  = buildings_lv_effect[0];
                        radius = buildings_lv_effect[1];
                        if (((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).education & 4) != 0)
                            change_lv(x, count + evolve_row, radius, 1, value_change);
                    } else if (building_type == 0xe3) {
                        value_change  = buildings_lv_effect[2];
                        radius = buildings_lv_effect[3];
                        change_lv(x, count + evolve_row, radius, 1, value_change);
                    } else if (building_type == 0xe4) {
                        value_change  = buildings_lv_effect[4];
                        radius = buildings_lv_effect[5];
                        change_lv(x, count + evolve_row, radius, 3, value_change);
                    } else if (building_type >= 0xae && building_type <= 0xb9) {
                        building_type -= 0xae;
                        radius = forum_lv_effect[building_type].radius;
                        value_change  = forum_lv_effect[building_type].delta;
                        size   = forum_gfxdat[building_type*4 + 1];
                        change_lv(x, count + evolve_row, radius, size, value_change);
                    } else if (building_type == 0xe5) {
                        value_change  = buildings_lv_effect[6];
                        radius = buildings_lv_effect[7];
                        change_lv(x, count + evolve_row, radius, 2, value_change);
                    } else if (building_type == 0xe6) {
                        value_change  = buildings_lv_effect[8];
                        radius = buildings_lv_effect[9];
                        change_lv(x, count + evolve_row, radius, 2, value_change);
                    } else if (building_type == 0xe7) {
                        value_change  = buildings_lv_effect[10];
                        radius = buildings_lv_effect[11];
                        change_lv(x, count + evolve_row, radius, 3, value_change);
                    } else if (building_type == 0xe8) {
                        value_change  = buildings_lv_effect[12];
                        radius = buildings_lv_effect[13];
                        change_lv(x, count + evolve_row, radius, 3, value_change);
                    } else if (building_type >= 0xe9 && building_type <= 0xec) {
                        value_change  = buildings_lv_effect[14];
                        radius = buildings_lv_effect[15];
                        change_lv(x, count + evolve_row, radius, 3, value_change);
                    } else if (building_type >= 0xed && building_type <= 0xf0) {
                        value_change  = buildings_lv_effect[16];
                        radius = buildings_lv_effect[17];
                        change_lv(x, count + evolve_row, radius, 4, value_change);
                    } else if (building_type == 0xf3) {
                        value_change  = buildings_lv_effect[18];
                        radius = buildings_lv_effect[19];
                        change_lv(x, count + evolve_row, radius, 2, value_change);
                    } else if (building_type == 0xf4) {
                        value_change  = buildings_lv_effect[20];
                        radius = buildings_lv_effect[21];
                        change_lv(x, count + evolve_row, radius, 3, value_change);
                    } else if (building_type == 0xf5) {
                        value_change  = buildings_lv_effect[22];
                        radius = buildings_lv_effect[23];
                        change_lv(x, count + evolve_row, radius, 3, value_change);
                    } else if (building_type >= 0xa2 && building_type <= 0xad) {
                        building_type -= 0xa2;
                        radius = temple_lv_effect[building_type].radius;
                        value_change  = temple_lv_effect[building_type].delta;
                        if      (building_type < 4) size = 1;
                        else if (building_type < 8) size = 2;
                        else              size = 3;
                        change_lv(x, count + evolve_row, radius, size, value_change);
                    } else if (building_type == 0xfa) {
                        index = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building & 0xf0;
                        index >>= 4;
                        if (index <= 4) {
                            value_change  = buildings_lv_effect[24];
                            radius = buildings_lv_effect[25];
                        } else {
                            value_change  = buildings_lv_effect[26];
                            radius = buildings_lv_effect[27];
                        }
                        change_lv(x, count + evolve_row, radius, 3, value_change);
                    } else if (building_type == 0xfc) {
                        value_change  = buildings_lv_effect[28];
                        radius = buildings_lv_effect[29];
                        change_lv(x, count + evolve_row, radius, 2, value_change);
                    } else if (building_type == 0xfd) {
                        value_change  = buildings_lv_effect[30];
                        radius = buildings_lv_effect[31];
                        change_lv(x, count + evolve_row, radius, 2, value_change);
                    } else if (building_type == 0xfe) {
                        value_change  = buildings_lv_effect[32];
                        radius = buildings_lv_effect[33];
                        change_lv(x, count + evolve_row, radius, 2, value_change);
                    } else if (building_type == 0xff) {
                        value_change  = buildings_lv_effect[34];
                        radius = buildings_lv_effect[35];
                        change_lv(x, count + evolve_row, radius, 2, value_change);
                    } else if (building_type == 0xdf) {
                        if (((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).education & 4) != 0) {
                            value_change  = buildings_lv_effect[36];
                            radius = buildings_lv_effect[37];
                            change_lv(x, count + evolve_row, radius, 2, value_change);
                        }
                    } else if (building_type == 0xe0) {
                        if (((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).education & 4) != 0) {
                            value_change  = buildings_lv_effect[38];
                            radius = buildings_lv_effect[39];
                            change_lv(x, count + evolve_row, radius, 2, value_change);
                        }
                    } else if (building_type == 0xe1) {
                        if (((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).education & 4) != 0) {
                            value_change  = buildings_lv_effect[40];
                            radius = buildings_lv_effect[41];
                            change_lv(x, count + evolve_row, radius, 2, value_change);
                        }
                    } else if (building_type == 0xe2) {
                        if (((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).education & 4) != 0) {
                            value_change  = buildings_lv_effect[42];
                            radius = buildings_lv_effect[43];
                            change_lv(x, count + evolve_row, radius, 2, value_change);
                        }
                } else if (building_type == 0xfb) {
                    value_change  = buildings_lv_effect[44];
                    radius = buildings_lv_effect[45];
                    change_lv(x, count + evolve_row, radius, 3, value_change);
                }
                } else if ((map_flags & 0x18) != 0) {
                    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).land_value = 0;
                }
            }

            if (kind >= 0 && kind < 8) {
                if ((((unsigned char *)city_map)[cm_sptr + 3] & 0x80) != 0) {
                    value_change  = buildings_lv_effect[46];
                    radius = buildings_lv_effect[47];
                    change_lv(x, count + evolve_row, radius, 1, value_change);
                } else {
                    value_change  = buildings_lv_effect[48];
                    radius = buildings_lv_effect[49];
                    change_lv(x, count + evolve_row, radius, 1, value_change);
                }
                value_change  = buildings_lv_effect[50];
                radius = buildings_lv_effect[51];
                change_lv(x, count + evolve_row, radius, 1, value_change);
            } else if (kind >= 0x7c && kind <= 0x7e) {
                value_change  = buildings_lv_effect[52];
                radius = buildings_lv_effect[53];
                change_lv(x, count + evolve_row, radius, 1, value_change);
            } else if (kind >= 0x78 && kind <= 0x7b) {
                value_change  = buildings_lv_effect[54];
                radius = buildings_lv_effect[55];
                change_lv(x, count + evolve_row, radius, 1, value_change);
            }

            curr_lv = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).land_value;
            if (curr_lv > top_lv) {
                top_lv      = curr_lv;
                top_lv_spot = cm_sptr;
            }
        }
    }

    if (count + evolve_row >= 0x50) {
        top_lv_ptr = top_lv_spot / 20;
        top_lv_x   = top_lv_ptr  % 80;
        top_lv_y   = top_lv_ptr  / 80;
    }
}

// Cap each cell's land value according to its water, services, security, health, and education.
// FUNCTION: C2 0x41138
// FUNCTION: C2WIN 0x00463f66
void cap_land_value(int rows)
{
    signed char map_land_value;
    unsigned char *this_cell_ptr;
    unsigned char bath_coverage;
    unsigned char prefecture_check;
    int x;
    unsigned char industry;
    unsigned char nearby_market;
    unsigned char admin;
    signed char capped_value;
    unsigned char market_cover;
    unsigned char business_vlow;
    unsigned char hospital_size;
    unsigned char sub_aqueduct_flag;
    unsigned char has_business;
    unsigned char has_barracks;
    unsigned char building_number;
    unsigned char activity;
    unsigned char low_business_flag;
    unsigned char entertainment;
    unsigned char police;
    int y_index;
    unsigned char security;
    unsigned char health;
    signed char security_value;
    unsigned char wall;
    unsigned char aqua;
    unsigned char gate;
    unsigned char theatre;
    unsigned char colosseum;
    unsigned char circus;
    unsigned char water_supply_flag;
    unsigned char grammaticus;
    unsigned char rhetor;
    unsigned int size;

    cm_sptr = evolve_row * 1600;
    for (y_index = 0; y_index < rows; y_index++)
    {
        for (x = 0; x < 80; x++, cm_sptr += 20)
        {
            building_number = ((unsigned char *)city_map)[cm_sptr];
            if (building_number >= 0x82) size = (unsigned char)reg_aquaduct_gfxdat[building_number + 8];
            else size = 1;

            activity = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a & 0xf;

            this_cell_ptr = (unsigned char *)city_map + cm_sptr;
            if (activity) this_cell_ptr = get_ptr_to_corner((unsigned char *)city_map + cm_sptr, size);

            if (building_number >= 0x82 && building_number <= 0xa1) {
            sub_aqueduct_flag = affected_by_cover1(this_cell_ptr, size, 2);
            aqua = affected_by_cover1(this_cell_ptr, size, 1);
            if (aqua == 0 && sub_aqueduct_flag == 0) { capped_value = 0x02; goto emit; }

            admin = get_range1(this_cell_ptr, size, 0x0c);
            if (admin == 0) { capped_value = 0x06; goto emit; }

            has_business = affected_by_cover1(this_cell_ptr, size, 0x80);
            if (has_business != 0) { capped_value = 0x0a; goto emit; }

            market_cover = get_range1(this_cell_ptr, size, 0xc0);
            if (market_cover == 0) { capped_value = 0x0c; goto emit; }

            if (aqua == 0) { capped_value = 0x0e; goto emit; }

            health = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).health;
            low_business_flag = affected_by_cover2(this_cell_ptr, size, 0x10);
            if (low_business_flag != 0) { capped_value = 0x10; goto emit; }

            bath_coverage = affected_by_cover1(this_cell_ptr, size, 0x08);
            if (bath_coverage == 0) { capped_value = 0x12; goto emit; }

            theatre = get_range3(this_cell_ptr, size, 0x03);
            colosseum = get_range3(this_cell_ptr, size, 0x0c);
            colosseum = colosseum >> 2;
            circus = get_range3(this_cell_ptr, size, 0x30);
            circus = circus >> 4;
            entertainment = theatre + colosseum + circus;
            if (entertainment == 0) { capped_value = 0x14; goto emit; }

            has_barracks = affected_by_cover2(this_cell_ptr, size, 0x01);
            if (has_barracks != 0) { capped_value = 0x18; goto emit; }

            security_value = ((unsigned char *)city_map)[cm_sptr + 0x11];
            police = get_range1(this_cell_ptr, size, 0x30);
            if (security_value < 0x10) security = 0;
            else security = 1;

            if (police) security = security + 1;
            if (security == 0) { capped_value = 0x18; goto emit; }

            business_vlow = affected_by_cover2(this_cell_ptr, size, 0x20);
            if (business_vlow != 0) { capped_value = 0x1a; goto emit; }

            if (entertainment <= 1) { capped_value = 0x1a; goto emit; }

            wall = affected_by_cover2(this_cell_ptr, size, 0x08);
            if (wall != 0) { capped_value = 0x1a; goto emit; }

            if (entertainment <= 2) { capped_value = 0x1c; goto emit; }

            gate = affected_by_cover2(this_cell_ptr, size, 0x04);
            if (gate != 0) { capped_value = 0x1e; goto emit; }

            if (hospital_cover < 0x14) { capped_value = 0x1e; goto emit; }

            if (entertainment <= 3) { capped_value = 0x20; goto emit; }

            grammaticus = affected_by_cover1(this_cell_ptr, size, 0x10);
            if (grammaticus == 0) { capped_value = 0x22; goto emit; }

            prefecture_check = affected_by_cover2(this_cell_ptr, size, 0x02);
            if (prefecture_check != 0) { capped_value = 0x22; goto emit; }

            if (hospital_cover < 0x28) { capped_value = 0x24; goto emit; }

            if (entertainment <= 4) { capped_value = 0x26; goto emit; }

            nearby_market = affected_by_cover1(this_cell_ptr, size, 0x40);
            if (nearby_market != 0) { capped_value = 0x28; goto emit; }

            if (security <= 1) { capped_value = 0x2a; goto emit; }

            if (hospital_cover < 0x3c) { capped_value = 0x2c; goto emit; }

            if (entertainment <= 5) { capped_value = 0x2c; goto emit; }

            rhetor = affected_by_cover1(this_cell_ptr, size, 0x20);
            if (rhetor == 0) { capped_value = 0x2e; goto emit; }

            if (library_cover < 0x14) { capped_value = 0x2e; goto emit; }

            if (entertainment <= 6) { capped_value = 0x30; goto emit; }

            if (library_cover < 0x28) { capped_value = 0x32; goto emit; }

            if (hospital_cover < 0x50) { capped_value = 0x34; goto emit; }

            if (library_cover < 0x3c) { capped_value = 0x36; goto emit; }

            if (entertainment <= 7) { capped_value = 0x38; goto emit; }

            if (hospital_cover < 0x64) { capped_value = 0x3a; goto emit; }

            if (library_cover < 0x50) { capped_value = 0x3a; goto emit; }

            if (entertainment <= 8) { capped_value = 0x3c; goto emit; }

            if (library_cover < 0x64) { capped_value = 0x3e; goto emit; }

            capped_value = 0x40;
            } else {
            has_business = affected_by_cover1(this_cell_ptr, size, 0x80);
            if (has_business != 0) { capped_value = 0x0a; goto emit; }

            low_business_flag = affected_by_cover2(this_cell_ptr, size, 0x10);
            if (low_business_flag != 0) { capped_value = 0x10; goto emit; }

            has_barracks = affected_by_cover2(this_cell_ptr, size, 0x01);
            if (has_barracks != 0) { capped_value = 0x18; goto emit; }

            business_vlow = affected_by_cover2(this_cell_ptr, size, 0x20);
            if (business_vlow != 0) { capped_value = 0x1a; goto emit; }

            wall = affected_by_cover2(this_cell_ptr, size, 0x08);
            if (wall != 0) { capped_value = 0x1a; goto emit; }

            gate = affected_by_cover2(this_cell_ptr, size, 0x04);
            if (gate != 0) { capped_value = 0x1e; goto emit; }

            prefecture_check = affected_by_cover2(this_cell_ptr, size, 0x02);
            if (prefecture_check != 0) { capped_value = 0x22; goto emit; }

            nearby_market = affected_by_cover1(this_cell_ptr, size, 0x40);
            if (nearby_market != 0) { capped_value = 0x28; goto emit; }

            capped_value = 0x40;
            }
        emit:
            map_land_value = ((signed char *)city_map)[cm_sptr + 0xf];
            if (capped_value < map_land_value) ((unsigned char *)city_map)[cm_sptr + 0xf] = capped_value;
        } }
}

#if PLATFORM_WINDOWS
int put_out_a(char, char, char, char, char, char, char);
int evolve_a_house();
int stretch_house();
int stretch_to_2x2_house();
int stretch_to_3x3_house();
int get_pop_level();
#endif

// Update forum activity and periodically send citizens from occupied forum buildings.
// FUNCTION: C2 0x415bb
// FUNCTION: C2WIN 0x004647a0
void evolve_forum_activity(int rows)
{
    unsigned char status;
    int row;
    unsigned char kind;
    unsigned char occupancy;
    int x;
    unsigned char gfx_value;
    unsigned char counter;
    unsigned char buyers;
    int people;

    cm_sptr = evolve_row * 1600;

    for (row = 0; row < rows; row++) {
        for (x = 0; x < 80; x++, cm_sptr += 20) {
            kind = ((unsigned char *)city_map)[cm_sptr];
            if (kind >= 0xae && kind <= 0xb9) {
                gfx_value = forum_gfxdat[kind + 0x26];
                if (gfx_value == 0) continue;
                occupancy = ((unsigned char *)city_map)[cm_sptr + 5] & 0x0f;
                if (occupancy == 0) {
                    counter = ((unsigned char *)city_map)[cm_sptr + 6] & 0x0f;
                    status = ((unsigned char *)city_map)[cm_sptr + 6] & 0x10;
                    buyers = (((unsigned char *)city_map)[cm_sptr + 5] & 0xf0) >> 4;

                    if (counter == 0) {
                        if (population < 2) continue;
                        people = put_out_a(1, (unsigned char)x, (unsigned char)(evolve_row + row), status,
                                           buyers, gfx_value, 0x20);
                        if (people != 0) {
                            buyers = people & 0xff;
                            counter = 3;
                            if (gfx_value == 9) counter -= 1;
                            else if (gfx_value == 0x10) counter -= 2;
                            citizen_list[created_citizen_no].state_idx = 1; citizen_list[created_citizen_no].wait_count = 0x14;
                            citizen_list[created_citizen_no].saved_state_idx = 3;
                        }
                        if (buyers >= gfx_value) buyers = 0;
                    } else {
                        counter--;
                    }

                    ((unsigned char *)city_map)[cm_sptr + 6] &= 0xf0;
                    ((unsigned char *)city_map)[cm_sptr + 6] |= counter;
                    ((unsigned char *)city_map)[cm_sptr + 5] &= 0x0f;
                    ((unsigned char *)city_map)[cm_sptr + 5] |= buyers << 4;
                }
            }
        } }
}

// Dispatch soldiers from forts to engage nearby enemy citizens.
// FUNCTION: C2 0x4176e
// FUNCTION: C2WIN 0x00464a1f
void evolve_fort_activity(int rows)
{
    int row;
    int new_counter;
    int col;
    unsigned char counter;

    cm_sptr = evolve_row * 1600;

    for (row = 0; row < rows; row++) {
        for (col = 0; col < 80; col++, cm_sptr += 20) {
            unsigned char kind;

            kind = ((unsigned char *)city_map)[cm_sptr];
            if (kind == 0xbf) {
                counter = ((unsigned char *)city_map)[cm_sptr + 6] & 0x0f;
                if (counter == 0) {
                    int res;
                    short enemy_target;

                    if (population < 2) continue;
                    enemy_target = find_enemy(col, evolve_row + row, 6);
                    if (enemy_target == 0) continue;
                    counter = 3;
                    res = put_out_a(4, (unsigned char)col, (unsigned char)(evolve_row + row), 0, 0, 1, 0);
                    if (res != 0) {
                        citizen_a = enemy_target;
                        citizen_list[created_citizen_no].target_kind = citizen_a;
                        citizen_list[created_citizen_no].target_marker = citizen_list[citizen_a].evolve_timer;
                        citizen_list[created_citizen_no].dest_x = citizen_list[citizen_a].x; citizen_list[created_citizen_no].dest_y = citizen_list[citizen_a].y;
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
void evolve_security_activity(int rows)
{
    int row;
    unsigned char occupancy;
    unsigned char kind;
    int x;
    unsigned char status;
    unsigned char counter;
    unsigned char patrols;
    int people;

    cm_sptr = evolve_row * 1600;

    for (row = 0; row < rows; row++) {
        for (x = 0; x < 80; x++, cm_sptr += 20) {
            kind = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind;
            if (kind == 0xe3) {
                occupancy = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a & 0x0f;
                if (occupancy != 0) continue;
                counter = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_b & 0x0f;
                status = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_b & 0x10;
                patrols = ((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a & 0xf0) >> 4;
                if (counter == 0) {
                    if (population < 2) continue;
                    people = put_out_a(5, (unsigned char)x, (unsigned char)(evolve_row + row), status,
                                       patrols, 1, 0x20);
                    if (people != 0) {
                        patrols = (unsigned char)people;
                        counter = 3;
                        citizen_list[created_citizen_no].state_idx = 1; citizen_list[created_citizen_no].wait_count = 0x14;
                        citizen_list[created_citizen_no].saved_state_idx = 8;
                    }
                    patrols++; if (patrols >= 4) patrols = 0;
                } else {
                    counter--;
                }
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_b &= 0xf0;
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_b |= counter;
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a &= 0x0f;
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a |= patrols << 4;
            } else if (kind == 0xe4) {
                occupancy = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a & 0x0f;
                if (occupancy != 0) continue;
                counter = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_b & 0x0f;
                status = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_b & 0x10;
                patrols = ((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a & 0xf0) >> 4;
                if (counter == 0) {
                    if (population < 2) continue;
                    people = put_out_a(4, (unsigned char)x, (unsigned char)(evolve_row + row), status,
                                       patrols, 9, 0x20);
                    if (people != 0) {
                        patrols = (unsigned char)people;
                        counter = 3;
                        citizen_list[created_citizen_no].state_idx = 1; citizen_list[created_citizen_no].wait_count = 0x14;
                        citizen_list[created_citizen_no].saved_state_idx = 7;
                    }
                    patrols++; if (patrols >= 9) patrols = 0;
                } else {
                    counter--;
                }
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_b &= 0xf0;
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_b |= counter;
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a &= 0x0f;
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a |= patrols << 4;
            }
        }
    }
}


// Update markets and businesses, refresh their output, and dispatch shopper or trader citizens.
// FUNCTION: C2 0x41b49
// FUNCTION: C2WIN 0x0046503e
void evolve_industrial_activity(int rows)
{
    unsigned char kind;
    int row;
    unsigned char occupancy;
    unsigned char status;
    unsigned char buyers;
    unsigned char counter;
    int x;
    int people;

    cm_sptr = evolve_row * 1600;

    for (row = 0; rows > row; row++) {
        for (x = 0; x < 80; x++, cm_sptr += 20) {
            kind = ((unsigned char *)city_map)[cm_sptr];
            if (kind >= 0xfc && kind <= 0xff) {
                occupancy = ((unsigned char *)city_map)[cm_sptr + 5] & 0xf;
                if (occupancy != 0) continue;
                market_image();

                counter = ((unsigned char *)city_map)[cm_sptr + 6] & 0xf;
                status = ((unsigned char *)city_map)[cm_sptr + 6] & 0x10;
                buyers  = (((unsigned char *)city_map)[cm_sptr + 5] & 0xf0) >> 4;
                if (counter == 0) {
                    if (population < 2) continue;
                    people = put_out_a(2, (char)x, (char)(evolve_row + row), status,
                                       (char)buyers, 4, 0x20);
                    if (people != 0) {
                        buyers = (unsigned char)people;
                        counter = 3;
                        citizen_list[created_citizen_no].state_idx = 1; citizen_list[created_citizen_no].wait_count = 0x14;
                        citizen_list[created_citizen_no].saved_state_idx = 4;
                        citizen_list[created_citizen_no].target_ref = cm_sptr;
                        remove_envoy();
                        ((unsigned char *)city_map)[cm_sptr + 0x12] = (unsigned char)created_citizen_no;
                    }
                    buyers++; if (buyers >= 4) buyers = 0;
                } else {
                    counter--;
                }
                ((unsigned char *)city_map)[cm_sptr + 6] &= 0xf0;
                ((unsigned char *)city_map)[cm_sptr + 6] |= counter;
                ((unsigned char *)city_map)[cm_sptr + 5] &= 0x0f;
                ((unsigned char *)city_map)[cm_sptr + 5] |= buyers << 4;
            } else if (kind == 0xfa) {
                occupancy = ((unsigned char *)city_map)[cm_sptr + 5] & 0xf;
                ((unsigned char *)city_map)[cm_sptr + 3] |= 1;
                if (occupancy != 0) continue;
                business_output(x, evolve_row + row);

                counter = ((unsigned char *)city_map)[cm_sptr + 6] & 0xf;
                status = ((unsigned char *)city_map)[cm_sptr + 6] & 0x10;
                buyers  = (((unsigned char *)city_map)[cm_sptr + 5] & 0xf0) >> 4;
                if (counter == 0) {
                    if (population < 2) continue;
                    people = put_out_a(6, (char)x, (char)(evolve_row + row), status,
                                       (char)buyers, 9, 0x20);
                    if (people != 0) {
                        buyers = (unsigned char)people;
                        counter = 3;
                        citizen_list[created_citizen_no].state_idx = 1; citizen_list[created_citizen_no].wait_count = 0x14;
                        citizen_list[created_citizen_no].saved_state_idx = 0xa;
                        citizen_list[created_citizen_no].target_ref = cm_sptr;
                        remove_envoy();
                        ((unsigned char *)city_map)[cm_sptr + 0x12] = (unsigned char)created_citizen_no;
                    }
                    buyers++; if (buyers >= 9) buyers = 0;
                } else {
                    counter--;
                }
                ((unsigned char *)city_map)[cm_sptr + 6] &= 0xf0;
                ((unsigned char *)city_map)[cm_sptr + 6] |= counter;
                ((unsigned char *)city_map)[cm_sptr + 5] &= 0x0f;
                ((unsigned char *)city_map)[cm_sptr + 5] |= buyers << 4;
            }
        }
    }
}

// Send a cell's previous envoy home when it is still assigned to that cell.
// FUNCTION: C2 0x41e3a
// FUNCTION: C2WIN 0x004654b5
void remove_envoy(void)
{
    citizen_a = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).industrial;
    if (citizen_list[citizen_a].exists == 0)
        return;
    if (citizen_list[citizen_a].target_ref != cm_sptr)
        return;
    citizen_list[citizen_a].state_idx = 2;
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
  unsigned char building = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building;
  unsigned char good = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).business & 0xf;
  unsigned char low_half = building & 3;
  unsigned char upper_level = building & 0xc;
  int city_stock = industry[good].city_supply;
  int supply_pipeline = industry[good].supply_pipeline[0];
  int factor = ind_growth_factor;
  int num_people = test_area_for_population(2, cell_x, cell_y, 2);
  int max_supply;
  if (num_people > 0x82)
    low_half += 4;
  else if (num_people > 0x5a)
    low_half += 3;
  else if (num_people > 0x32)
    low_half += 2;
  else if (num_people > 10)
    low_half += 1;
  if (low_half <= 0)
    max_supply = 0;
  else
    if (low_half <= 1)
    max_supply = 3;
  else
    if (low_half <= 2)
    max_supply = 5;
  else
    if (low_half <= 3)
    max_supply = 7;
  else
    max_supply = 7;
  if (upper_level == 0)
  {
    factor -= 2;
    if (max_supply > 4)
      max_supply = 4;
  }
  if (supply_pipeline <= 0)
    max_supply = 0;
  else
    if (supply_pipeline <= 0x32)
  {
    factor -= 3;
    if (max_supply > 1)
      max_supply = 1;
  }
  else
    if (supply_pipeline <= 0xc8)
  {
    factor -= 2;
    if (max_supply > 3)
      max_supply = 3;
  }
  else
    if (supply_pipeline <= 0x190)
  {
    factor -= 1;
    if (max_supply > 5)
      max_supply = 5;
  }
  else
  {
    if (supply_pipeline <= 600)
      goto pipeline_done;
    if (supply_pipeline <= 0x320)
      factor += 1;
    else
      if (supply_pipeline <= 0x3e8)
      factor += 2;
    else
      if (supply_pipeline > 1000)
      factor += 3;
  }
pipeline_done:
  if (no_of_empire_connections <= 0)
  {
    if (max_supply > 4)
      max_supply = 4;
  }
  else
    if (no_of_empire_connections <= 1)
    factor += 1;
  else
    if (no_of_empire_connections > 1)
    factor += 2;
  if (city_stock <= 0)
    max_supply = 0;
  else
    if (city_stock <= 0x14)
  {
    if (max_supply > 1)
      max_supply = 1;
  }
  else
    if (city_stock <= 0x22)
  {
    if (max_supply > 2)
      max_supply = 2;
  }
  else
    if (city_stock <= 0x32)
  {
    if (max_supply > 3)
      max_supply = 3;
  }
  else
    if (city_stock <= 0x43)
  {
    if (max_supply > 4)
      max_supply = 4;
  }
  else
    if (city_stock <= 0x4b)
  {
    if (max_supply > 5)
      max_supply = 5;
  }
  else
    if ((city_stock <= 99) && (max_supply > 6))
    max_supply = 6;
  if (max_supply < factor)
    factor = max_supply;
  if (factor < 0)
    factor = 0;
  if (factor > 7)
    factor = 7;
  (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building &= 0xf;
  factor <<= 4;
  (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building |= factor;
  if (low_half != 0)
  {
    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building &= 0xfc;
    if (low_half == 2)
      (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building |= 1;
    else
      if (low_half == 3)
      (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building |= 2;
  }
  if (upper_level != 0)
  {
    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building &= 0xf3;
    if (upper_level == 8)
      (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building |= 4;
    else
      if (upper_level == 0xc)
      (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building |= 8;
  }
}

// Advance fires and plagues, spread them to neighboring cells, and turn severe housing unrest
// into destruction and rioters.
// FUNCTION: C2 0x42204
// FUNCTION: C2WIN 0x00465bbb
void spread_fire_and_plague_and_unrest(int row_count)
{
    int map_row;
    int col_no;
    unsigned char cell_kind;
    unsigned char fire;
    signed char unrest;
    unsigned char range_flag;
    unsigned char health_level;
    unsigned char rand_num;
    unsigned char status;

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

    for (map_row = 0; map_row < row_count; map_row++) {
        for (col_no = 0; col_no < 80; col_no++, cm_sptr += 20) {

            if ((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).edge_bits & 0x80) {
                cell_kind = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind;
                if (cell_kind < 8) {
                    fire = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).fire - 1;
                    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).fire = fire;
                    if (fire == 0) {
                        (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).edge_bits &= 0x7f;
                    } else if (fire_spread_count++ >= fire_spread_target) {
                        fire_spread_target += 2;
                        if (fire_spread_direction == 0 && map_row + evolve_row <= 0)
                            continue;
                        else if (fire_spread_direction == 4 && map_row + evolve_row >= 0x4f)
                            continue;
                        else if (fire_spread_direction == 6 && col_no <= 0)
                            continue;
                        else if (fire_spread_direction == 2 && col_no >= 0x4f)
                            continue;
                        spread_fire_atom(cm_sptr, fire_spread_direction);
                    }
                } else if (cell_kind >= 0x82 && cell_kind <= 0xa1) {
                    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).fpu_flag &= 0xcf;
                    fire = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).fire;
                    fire = fire - 1; (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).fire = fire;
                    if (fire == 0) {
                        destroy_an_atom(cm_sptr, 0);
                    } else if (plague_spread_count++ >= plague_spread_target) {
                        plague_spread_target++;
                        if (plague_spread_direction == 0 && map_row + evolve_row <= 0)
                            continue;
                        else if (plague_spread_direction == 4 && map_row + evolve_row >= 0x4f)
                            continue;
                        else if (plague_spread_direction == 6 && col_no <= 0)
                            continue;
                        else if (plague_spread_direction == 2 && col_no >= 0x4f)
                            continue;
                        if (fire == 9)
                            continue;
                        spread_plague_atom(cm_sptr, plague_spread_direction);
                    }
                }
            } else {
                cell_kind = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind;
                if (cell_kind >= 0x82 && cell_kind <= 0x9b) {
                    status = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a & 0xf;
                    if (status != 0) continue;

                    unrest = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).fpu_flag & 0xf;
                    range_flag = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).range_flag;
                    health_level = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).health;
                    if ((range_flag & 0x0c) == 0) unrest = unrest - 1;
                    if ((range_flag & 0x30) != 0) unrest = unrest - 1;
                    if ((health_level & 0x03) != 0) unrest = unrest - 1;
                    unrest += insurrection_factor;

                    unrest_random_count++;
                    if (unrest_random_count >= 0x40) unrest_random_count = 0;
                    rand_num = unrest_random_data[unrest_random_count];
                    if (rand_num == 9)
                        unrest += house_type_to_unrest[cell_kind - 0x82];
                    else
                        unrest = unrest + rand_num;
                    if (unrest < 0) unrest = 0;
                    else if (unrest > 0xf) {
                        if (insurrection_factor > 6)
                            insurrection_factor = 6;
                        else if (insurrection_factor > 2)
                            insurrection_factor = 2;
                        destroy_an_atom(cm_sptr, 0);
                        if (put_out_a(7, (unsigned char)col_no, (unsigned char)(evolve_row + map_row), 0, 0, 0, 0) != 0) {
                            citizen_list[created_citizen_no].speed_phase = 0;
                            citizen_list[created_citizen_no].speed_count = 0;
                            citizen_list[created_citizen_no].state_idx = 1;
                            citizen_list[created_citizen_no].wait_count = 0x14;
                            citizen_list[created_citizen_no].saved_state_idx = 0xb;
                            if (put_a_message == 0)
                                put_message(0x57, cm_sptr, 0x15);
                        }
                        continue;
                    }
                    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).fpu_flag &= 0xf0;
                    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).fpu_flag |= unrest;
                }
            }
        }
    }
}

// Create a citizen at the requested cell or at the first usable perimeter position.
// FUNCTION: C2 0x42666
// FUNCTION: C2WIN 0x00466232
int put_out_a(char citizen_kind, char cell_x, char cell_y, char unused, char start_idx,
              char perimeter_size, char barbarian_flag)
{
  int idx;
  int position_count;
  int counter;
  int offx;
  int offy;
  (void) unused;
  idx = (unsigned char)start_idx;
  if ((unsigned char)perimeter_size == 1)
    position_count = 4;
  else
    if ((unsigned char)perimeter_size == 4)
    position_count = 8;
  else
    if ((unsigned char)perimeter_size == 9)
    position_count = 12;
  else
    if ((unsigned char)perimeter_size == 0x10)
    position_count = 0x10;
  else
  {
    if (create_citizen((unsigned char)citizen_kind, (unsigned char)cell_x,
                       (unsigned char)cell_y, 0) != 0)
      return 1;
    else
      return 0;
  }
  for (counter = 0; counter < position_count; counter++, idx++)
  {
    if (position_count <= idx)
      idx = 0;
    if ((unsigned char)perimeter_size == 1)
    {
      offx = putouts1[idx].dx;
      offy = putouts1[idx].dy;
    }
    else
      if ((unsigned char)perimeter_size == 4)
    {
      offx = putouts2[idx].dx;
      offy = putouts2[idx].dy;
    }
    else
      if ((unsigned char)perimeter_size == 9)
    {
      offx = putouts3[idx].dx;
      offy = putouts3[idx].dy;
    }
    else
      if ((unsigned char)perimeter_size == 0x10)
    {
      offx = putouts4[idx].dx;
      offy = putouts4[idx].dy;
    }
    if (create_citizen((unsigned char)citizen_kind, (unsigned char)cell_x + offx,
                       (unsigned char)cell_y + offy, (unsigned char)barbarian_flag) != 0)
      return idx + 1;
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
    unsigned int next;
    unsigned int curr;
    int delta;

    curr = house_gfxdat[tier_idx*4 + 1];
    if (++tier_idx >= 0x20) return 0;
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
        if (stretch_to_2x2_house(tier_idx, footprint_size, 1)) return 2;
        if (stretch_to_2x2_house(tier_idx, footprint_size, 2)) return 3;
        if (stretch_to_2x2_house(tier_idx, footprint_size, 3)) return 4;
    } else if (footprint_size == 3) {
        if (stretch_to_3x3_house(tier_idx, footprint_size, 0)) return 1;
        if (stretch_to_3x3_house(tier_idx, footprint_size, 1)) return 2;
        if (stretch_to_3x3_house(tier_idx, footprint_size, 2)) return 3;
        if (stretch_to_3x3_house(tier_idx, footprint_size, 3)) return 4;
    }
    return 0;
}

// Check whether the cells needed for a 2x2 house expansion are clear or compatible housing.
// FUNCTION: C2 0x430af
// FUNCTION: C2WIN 0x004674b5
int stretch_to_2x2_house(int tier_idx, int unused, int orientation)
{
    int count;
    int cell_ofset;
    unsigned char bits;
    unsigned char cell_kind;
    unsigned char fire_status;
    unsigned char plague_count;

    (void)unused;
    if (evolve_col == 0x4f) return 0;
    if (evolve_row == 0x4f) return 0;
    if (evolve_col == 0) return 0;
    if (evolve_row == 0) return 0;
    for (count = 0; count < 3; count++) {
        cell_ofset = stretch_ofsets_2x2[orientation][count];
        bits = city_qptr[cell_ofset + 1];
        cell_kind = city_qptr[cell_ofset];
        fire_status = city_qptr[cell_ofset + 7];
        plague_count = city_qptr[cell_ofset + 8];
        if (fire_status != 0 || plague_count != 0) return 0;
        if ((bits & 0xfe) != 0) return 0;
        if ((bits & 1) != 0 && cell_kind >= tier_idx + 0x82) return 0;
    }
    return 1;
}

// Check a 3x3 house expansion and reduce overlapping villas to individual domus cells.
// FUNCTION: C2 0x4313d
// FUNCTION: C2WIN 0x004675ef
int stretch_to_3x3_house(int tier_idx, int unused, int orientation)
{
    int cell_ofset;
    unsigned char plague_count;
    unsigned char bits;
    unsigned char fire_status;
    int villa_row_no;
    int villa_col_no;
    int count;
    unsigned char cell_kind;
    unsigned char *qptr;

    if (evolve_col >= 0x4e) return 0;
    if (evolve_row >= 0x4e) return 0;
    if (evolve_col <= 0) return 0;
    if (evolve_row <= 0) return 0;
    for (count = 0; count < 5; count++) {
        cell_ofset = stretch_ofsets_3x3[orientation][count];
        bits = city_qptr[cell_ofset + 1];
        cell_kind = city_qptr[cell_ofset];
        fire_status = city_qptr[cell_ofset + 7];
        plague_count = city_qptr[cell_ofset + 8];
        if (fire_status != 0 || plague_count != 0) return 0;
        if ((bits & 0xfe) != 0) return 0;
        if ((bits & 1) != 0 && cell_kind >= tier_idx + 0x82) return 0;
    }
    for (count = 0; count < 5; count++) {
        cell_ofset = stretch_ofsets_3x3[orientation][count];
        cell_kind = city_qptr[cell_ofset];
        if (cell_kind >= 0x9c && cell_kind <= 0x9f) {
            villa_row_no = city_qptr[cell_ofset + 5] & 0xf;
            villa_col_no = villa_row_no;
            villa_col_no %= 2;
            villa_row_no /= 2;
            qptr = city_qptr + cell_ofset;
            qptr -= villa_col_no * 20;
            qptr -= villa_row_no * 1600;
            reduce_villa_to_domus(qptr);
        }
    }
    return 1;
}

// Stamp a house tier and its graphics across a selected map footprint.
// FUNCTION: C2 0x43255
// FUNCTION: C2WIN 0x00467804
void change_house(int tier_idx, int footprint_size, int orientation)
{
    int gfx = (unsigned char)house_gfxdat[tier_idx * 4];
    int rowadd = (80 - footprint_size) * 20;
    unsigned char *cm_ptr = city_qptr;
    int x;
    int i;
    int y;

    if (orientation == 0) {
    } else if (orientation == 1) cm_ptr -= 20;
    else if (orientation == 2) cm_ptr -= 0x654;
    else if (orientation == 3) cm_ptr -= 0x640;

    for (y = 0, i = 0; y < footprint_size; y++, cm_ptr += rowadd) {
        for (x = 0; x < footprint_size; x++, cm_ptr += 20, i++) {
            if ((unsigned char)cm_ptr[0] < 0x82) cm_ptr[3] &= 0x7f;
            cm_ptr[0] = (char)(tier_idx + 0x82);
            cm_ptr[1] |= 1;
            cm_ptr[3] &= 0xe3;
            cm_ptr[3] |= 1;
            cm_ptr[3] &= 0xdf;
            cm_ptr[5] = i;
            if (footprint_size == 1) {
                cm_ptr[4] = gfx;
            } else if (footprint_size == 2) {
                cm_ptr[4] = (char)(gfx + diamond_ofsets_2x[i]);
            } else if (footprint_size == 3) {
                cm_ptr[4] = (char)(gfx + diamond_ofsets_3x[i]);
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

    for (zone_x = 0; zone_x < 10; zone_x++) {
        for (zone_y = 0; zone_y < 10; zone_y++) {
            if (fire_zones[zone_x * 10 + zone_y] > 2)        fire_zones[zone_x * 10 + zone_y] = 2;
            else if (fire_zones[zone_x * 10 + zone_y] > 1)   fire_zones[zone_x * 10 + zone_y] = 1;
            else if (fire_zones[zone_x * 10 + zone_y] <= 1)  fire_zones[zone_x * 10 + zone_y] = 0;
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
    else if (population > 8000) return 6;
    else if (population > 4000) return 5;
    else if (population > 2000) return 4;
    else if (population > 1000) return 3;
    else if (population > 500) return 2;
    else if (population > 250) return 1;
    return 0;
}

// Recompute each industry's city supply from regional warehouses and update unloading warehouses.
// FUNCTION: C2 0x44013
// FUNCTION: C2WIN 0x00468e5d
void check_goods_in_region_warehouses(void)
{
    int count;
    int column_no;
    unsigned char delivered_now;
    int supplied;
    unsigned char goods_idx;
    unsigned char type;
    unsigned char remaining;
    unsigned char sprite;
    int delivered;

    if (c2inf.peace_mode) {
        for (count = 0; count < 16; count++) {
            industry[count].city_supply = city_level_good_supply[count];
            industry[count].status = 0;
            industry[count].status = count & 1;
            if (industry[count].status) industry[count].status++;
        }
        return;
    }

    for (count = 0; count < 16; count++) {
        industry[count].count     = 0;
        industry[count].supply    = 0;
        industry[count].delivered = 0;
    }

    count = 0; cm_sptr = 0;
    for (; count < 60; count++) {
        for (column_no = 0; column_no < 60; column_no++, cm_sptr += 8) {
            type = ((unsigned char *)region_map)[cm_sptr];
            if (type == 0xd4) {
                delivered_now = ((unsigned char *)region_map)[cm_sptr + 7] & 0x0f;
                goods_idx = ((unsigned char *)region_map)[cm_sptr + 7] & 0xf0;
                goods_idx >>= 4;
                goods_idx &= 0xf;
                industry[goods_idx].count++;
                if (delivered_now != 0) {
                    industry[goods_idx].status = 2;
                    remaining = industry[goods_idx].unit_size;
                    if (delivered_now <= remaining) {
                        remaining -= delivered_now;
                        industry[goods_idx].delivered += delivered_now;
                        delivered_now = 0;
                    } else {
                        delivered_now -= remaining;
                        industry[goods_idx].delivered += remaining;
                        industry[goods_idx].supply += delivered_now;
                        remaining = 0;
                    }
                    industry[goods_idx].unit_size = remaining;
                    ((unsigned char *)region_map)[cm_sptr + 7] &= 0xf0;
                    ((unsigned char *)region_map)[cm_sptr + 7] |= delivered_now;
                    if (delivered_now < 0xf) sprite = delivered_now + 11;
                    else sprite = 0x24;
                    change_reg_sized(type, sprite, 1, cm_sptr);
                }
            } } }

    for (count = 0; count < 16; count++) {
        delivered = industry[count].delivered;
        supplied = industry[count].has_supply;
        if (supplied) industry[count].city_supply = valueDIVtotal(delivered, supplied);
        else industry[count].city_supply = 0;
    }
}
