
extern int income_multiple;
extern int pop_income_pass_count;
extern int ind_income_pass_count;

#include "c2_data.h"
#include "c2_types.h"

/* Per-industry demand counts accumulated during a census sweep. */
int temp_demand_count[16];
/* Forward declarations (functions defined later in this file). */
void get_census(int quiet);
void take_census(int row_count);
void fire_trouble(void);
void road_trouble(void);
void water_trouble(void);
void wall_trouble(void);
void get_fire_cover(void);
void get_road_cover(void);
void get_wall_cover(void);
void get_water_cover(void);
void hospital_coverage(void);
void library_coverage(void);
void employment(void);
void running_pop_tax(void);
void running_ind_tax(void);
void region_census(void);


// Refresh census, coverage, regional, and rating data without triggering accidents.
// FUNCTION: C2 0x441c9
// FUNCTION: C2WIN 0x004304c0
void forum_update_census(void)
{
    int old_plague_accident;
    int old_fire_accident;
    int old_wall_accident;
    int old_road_accident;

    old_fire_accident = fire_accident;
    fire_accident = 0xf423f;
    old_wall_accident = wall_accident;
    wall_accident = 0xf423f;
    old_road_accident = road_accident;
    road_accident = 0xf423f;
    old_plague_accident = plague_accident;
    plague_accident = 0xf423f;

    evolve_row = 0;
    take_census(0x50);
    get_census(1);
    slave_warning = 0;
    get_fire_cover();
    get_wall_cover();
    get_road_cover();
    get_water_cover();
    get_regroad_web(reg_city_x, reg_city_y);
    region_census();
    adjust_culture_criteria();
    adjust_proserity_criteria();
    adjust_empire_criteria();

    if (evolve_clock >= 0xc2 && evolve_clock < 0xca) {
        evolve_clock = 0xcb;
    }

    if (fire_cover < 0x64) fire_accident = old_fire_accident;
    if (wall_cover < 0x64) wall_accident = old_wall_accident;
    if (road_cover < 0x64) road_accident = old_road_accident;
    plague_accident = old_plague_accident;
}

// Reset census, economy, accident, settlement, and industry state for a new game.
// FUNCTION: C2 0x442b9
// FUNCTION: C2WIN 0x004305eb
void init_census(void)
{
    int i;

    road_accident   = 999999;
    fire_accident   = 999999;
    wall_accident   = 999999;
    region_accident = 999999;
    plague_accident = 999999;
    revolt_accident = 999999;

    pop_tax_last_count         = 0;
    ind_tax_last_count         = 0;
    pop_tax_running_total      = 0;
    pop_tax_counts             = 0;
    ind_tax_running_total      = 0;
    ind_tax_counts             = 0;
    current_construction_cost  = 0;
    current_operating_cost     = 0;
    stolen_denarii             = 0;
    current_gdp                = 0;
    rolling_profit             = 0;
    moving_tribute             = 0;
    average_pop_tax_denariis   = 0;
    average_pop_tax_asses      = 0;
    average_ind_tax_denariis   = 0;
    average_ind_tax_asses      = 0;
    account_total              = 0;
    account_pop_tax            = 0;
    account_ind_tax            = 0;
    account_construction_cost  = 0;
    account_operating_cost     = 0;
    account_tribute            = 0;
    max_population             = 0;
    this_years_population      = 0;
    this_years_denarii         = denarii;
    this_years_pop_tax         = 0;
    this_years_ind_tax         = 0;
    last_years_population      = 0;
    last_years_denarii         = 0;
    last_years_pop_tax         = 0;
    last_years_ind_tax         = 0;

    fire_rate = 0; road_rate = 0; wall_rate = 0;
    water_trouble_rate = 0;

    no_of_workcamps = no_of_warehouses = no_of_shipyards = no_of_ports = 0;
    no_of_trading_posts = no_of_farms = no_of_mines = no_of_quarrys = 0;
    no_of_villages = no_of_towns = no_of_border_towns = 0;

    for (i = 0; i < 16; i++) {
        industry[i].status      = 0;
        industry[i].supply      = 0;
        industry[i].delivered   = 0;
        industry[i].unit_size   = 0;
        industry[i].count       = 0;
        industry[i].has_supply  = 0;
        industry[i].city_supply = 0;
        industry[i].supply_pipeline[0]     = 0;
        industry[i].supply_pipeline[1]     = 0;
        industry[i].supply_pipeline[2]     = 0;

        if (c2inf.peace_mode != 0) {
            industry[i].city_supply = city_level_good_supply[i];
            industry[i].status      = i & 1;
            if (industry[i].status != 0) industry[i].status++;
        }
    }
}

// Publish census tallies and update population, services, employment, trouble, and tax state.
// FUNCTION: C2 0x4446d
// FUNCTION: C2WIN 0x00430923
void get_census(int quiet)
{
    int i;

    if (quiet == 0) no_of_census_passes++;

    /* Publish the counts accumulated by take_census. */
    population_running_count       = population_pass_count;
    structure_running_count        = structure_pass_count;
    road_running_count             = road_pass_count;
    fire_running_count             = fire_pass_count;
    plague_running_count           = plague_pass_count;
    fountains_count                = fountains_pass_count;
    baths_count                    = baths_pass_count;
    supplied_fountains_count       = supplied_fountains_pass_count;
    supplied_baths_count           = supplied_baths_pass_count;
    large_forums_count             = large_forums_pass_count;
    medium_forums_count            = medium_forums_pass_count;
    small_forums_count             = small_forums_pass_count;
    forts_count                    = forts_pass_count;
    prefectures_count              = prefectures_pass_count;
    barracks_count                 = barracks_pass_count;
    large_temples_count            = large_temples_pass_count;
    med_temples_count              = med_temples_pass_count;
    small_temples_count            = small_temples_pass_count;
    large_temples_culture_count    = large_temples_culture_pass_count;
    med_temples_culture_count      = med_temples_culture_pass_count;
    small_temples_culture_count    = small_temples_culture_pass_count;
    large_robbery_count            = large_robbery_pass_count;
    med_robbery_count              = med_robbery_pass_count;
    small_robbery_count            = small_robbery_pass_count;
    hospitals_count                = hospitals_pass_count;
    accessed_hospitals_count       = accessed_hospitals_pass_count;
    libraries_count                = libraries_pass_count;
    accessed_libraries_count       = accessed_libraries_pass_count;
    grammaticus_count              = grammaticus_pass_count;
    rhetor_count                   = rhetor_pass_count;
    grammaticus_culture_count      = grammaticus_culture_pass_count;
    rhetor_culture_count           = rhetor_culture_pass_count;
    theatre_count                  = theatre_pass_count;
    odium_count                    = odium_pass_count;
    arena_count                    = arena_pass_count;
    colosseum_count                = colosseum_pass_count;
    circus_count                   = circus_pass_count;
    circus_maximus_count           = circus_maximus_pass_count;
    theatre_culture_count          = theatre_culture_pass_count;
    odium_culture_count            = odium_culture_pass_count;
    arena_culture_count            = arena_culture_pass_count;
    colosseum_culture_count        = colosseum_culture_pass_count;
    circus_culture_count           = circus_culture_pass_count;
    circus_maximus_culture_count   = circus_maximus_culture_pass_count;
    business_count                 = business_pass_count;
    market_count                   = market_pass_count;
    plaza_culture_count            = plaza_culture_pass_count;
    gardens_culture_count          = gardens_culture_pass_count;

    /* Derive combined service totals. */
    water_running_count = supplied_fountains_count + supplied_baths_count;
    temples_count       = large_temples_count + med_temples_count + small_temples_count;
    robbery_count       = large_robbery_count  + med_robbery_count  + small_robbery_count;

    /* Publish population and issue milestone messages. */
    population = population_running_count;
    if (population > max_population) max_population = population;

    if      (population >= 200   && warned_city_size == 0)  { put_message(0x68, 0, 10); warned_city_size++; }
    else if (population >= 500   && warned_city_size == 1)  { put_message(0x69, 0, 10); warned_city_size++; }
    else if (population >= 1000  && warned_city_size == 2)  { put_message(0x6a, 0, 10); warned_city_size++; }
    else if (population >= 2000  && warned_city_size == 3)  { put_message(0x6b, 0, 10); warned_city_size++; }
    else if (population >= 5000  && warned_city_size == 4)  { put_message(0x6c, 0, 10); warned_city_size++; }
    else if (population >= 10000 && warned_city_size == 5)  { put_message(0x6d, 0, 10); warned_city_size++; }
    else if (population >= 20000 && warned_city_size == 6)  { put_message(0x6e, 0, 10); warned_city_size++; }
    else if (population >= 30000 && warned_city_size == 7)  { put_message(0x6f, 0, 10); warned_city_size++; }
    else if (population >= 40000 && warned_city_size == 8)  { put_message(0x70, 0, 10); warned_city_size++; }
    /* Unlock structures at population milestones. */
    if      (population >= 400   && warned_new_struct == 0) { put_message(0x73, 0, 10); new_structure_is = 0xb2; warned_new_struct++; }
    else if (population >= 800   && warned_new_struct == 1) { put_message(0x73, 0, 10); new_structure_is = 0xe6; warned_new_struct++; }
    else if (population >= 1200  && warned_new_struct == 2) { put_message(0x73, 0, 10); new_structure_is = 0xf5; warned_new_struct++; }
    else if (population >= 1800  && warned_new_struct == 3) { put_message(0x73, 0, 10); new_structure_is = 0xb6; warned_new_struct++; }
    else if (population >= 2400  && warned_new_struct == 4) { put_message(0x73, 0, 10); new_structure_is = 0xe8; warned_new_struct++; }
    else if (population >= 4800  && warned_new_struct == 5) { put_message(0x73, 0, 10); new_structure_is = 0xed; warned_new_struct++; }

    /* Calculate service staffing requirements. */
    slave_requirements[1].max = fire_running_count      / 8;
    slave_requirements[2].max = road_running_count      / 8;
    slave_requirements[3].max = water_running_count     * 2;
    slave_requirements[4].max = structure_running_count / 8;
    slave_requirements[5].max = no_of_workcamps         * 30;
    slave_requirements[6].max = 0;

    /* Publish per-industry demand. */
    for (i = 0; i < 16; i++) {
        industry[i].has_supply = temp_demand_count[i];
    }

    if (quiet == 0) {
        /* Refresh industry demand and advance its supply history. */
        for (i = 0; i < 16; i++) {
            industry[i].has_supply = temp_demand_count[i];
            industry[i].unit_size  = temp_demand_count[i];
            if (temp_demand_count[i] != 0) industry[i].supply_pipeline[0] = population / temp_demand_count[i];
            else                           industry[i].supply_pipeline[0] = 0;
            industry[i].supply_pipeline[2] = industry[i].supply_pipeline[1];
            industry[i].supply_pipeline[1] = 0;
        }
    }

    hospital_coverage();
    library_coverage();
    employment();

    if (quiet != 0) return;

    slave_warning = 0;
    fire_trouble();
    road_trouble();
    water_trouble();
    wall_trouble();
    running_pop_tax();
    running_ind_tax();
}

// Scan city-map rows, accumulating population, income, service, and accident census data.
// FUNCTION: C2 0x44a94
// FUNCTION: C2WIN 0x0043107a
void take_census(int row_count)
{
    int row_idx;
    int col_idx;
    unsigned char culture_flags;
    int building_tier;
    unsigned char building_kind;
    unsigned char robbery_flags;
    unsigned char wall_terrain;
    unsigned char plague_flags;
    unsigned char activity_flags;
    unsigned char education_flags;
    unsigned char industry_idx;
    int saved_cm_sptr;
    unsigned char cell_flags;

    if (evolve_row == 0) {
        for (row_idx = 0; row_idx < 16; row_idx++) temp_demand_count[row_idx] = 0;
        population_pass_count                = 0;
        pop_income_pass_count                = 0;
        ind_income_pass_count                = 0;
        structure_pass_count                 = 0;
        road_pass_count                      = 0;
        fire_pass_count                      = 0;
        plague_pass_count                    = 0;
        fountains_pass_count                 = 0;
        baths_pass_count                     = 0;
        supplied_fountains_pass_count        = 0;
        supplied_baths_pass_count            = 0;
        large_forums_pass_count              = 0;
        medium_forums_pass_count             = 0;
        small_forums_pass_count              = 0;
        forts_pass_count                     = 0;
        prefectures_pass_count               = 0;
        barracks_pass_count                  = 0;
        large_temples_pass_count             = 0;
        med_temples_pass_count               = 0;
        small_temples_pass_count             = 0;
        large_temples_culture_pass_count     = 0;
        med_temples_culture_pass_count       = 0;
        small_temples_culture_pass_count     = 0;
        large_robbery_pass_count             = 0;
        med_robbery_pass_count               = 0;
        small_robbery_pass_count             = 0;
        hospitals_pass_count                 = 0;
        accessed_hospitals_pass_count        = 0;
        libraries_pass_count                 = 0;
        accessed_libraries_pass_count        = 0;
        grammaticus_pass_count               = 0;
        rhetor_pass_count                    = 0;
        grammaticus_culture_pass_count       = 0;
        rhetor_culture_pass_count            = 0;
        theatre_pass_count                   = 0;
        odium_pass_count                     = 0;
        arena_pass_count                     = 0;
        colosseum_pass_count                 = 0;
        circus_pass_count                    = 0;
        circus_maximus_pass_count            = 0;
        theatre_culture_pass_count           = 0;
        odium_culture_pass_count             = 0;
        arena_culture_pass_count             = 0;
        colosseum_culture_pass_count         = 0;
        circus_culture_pass_count            = 0;
        circus_maximus_culture_pass_count    = 0;
        business_pass_count                  = 0;
        market_pass_count                    = 0;
        plaza_culture_pass_count             = 0;
        gardens_culture_pass_count           = 0;
    }

    cm_sptr = evolve_row * 1600;  /* 80 cols × 20 bytes per cell */

    for (row_idx = 0; row_count > row_idx; row_idx++) {
        for (col_idx = 0; col_idx < 80; col_idx++, cm_sptr += 20) {
            cell_flags = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).terrain & 0x27;
            building_kind  = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind;

            /* Count plaza and garden culture coverage. */
            if (building_kind >= 0x7c && building_kind <= 0x7e) {
                culture_flags = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).range_flag & 0x0c;
                if (culture_flags) plaza_culture_pass_count++;
            } else if (building_kind >= 0x78 && building_kind <= 0x7b) {
                culture_flags = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).range_flag & 0x0c;
                if (culture_flags) gardens_culture_pass_count++;
            }

            if (cell_flags == 0) continue;

            /* Process road, wall, and fort cells. */
            if (cell_flags & 0x20) {
                /* Trigger a road failure at the selected census position. */
                if (road_pass_count != road_accident) road_pass_count++;
                else {
                    destroy_an_atom(cm_sptr, 0); road_accident = 0xf423f;
                }
            }
            else if (cell_flags & 0x02) {
                /* Trigger a wall collapse at the selected census position. */
                if (structure_pass_count != wall_accident) structure_pass_count++;
                else {
                    saved_cm_sptr = cm_sptr;
                    wall_terrain = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).terrain;
                    if (wall_terrain & 0xc0) unflag_all_cm(3, 0xdf);
                    clear_an_area(col_idx, evolve_row + row_idx,
                                  col_idx, evolve_row + row_idx);
                    cm_sptr           = saved_cm_sptr;
                    wall_accident     = 0xf423f;
                    particles_cleared = 0;
                }
            }
            else if (cell_flags & 0x04) {
                building_kind = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind;
                if (building_kind == 0xbf) forts_pass_count++;
            }
            else {

            /* Process fire risk outside the exempt kind range. */
            if (building_kind < 0xbc || building_kind > 0xe2) {
                if (fire_pass_count != fire_accident) fire_pass_count++;
                else {
                    destroy_an_atom(cm_sptr, 1);
                    put_message(0x52, cm_sptr, 20);
                    fire_accident = 0xf423f;
                    continue;
                }
            }

            /* Process plague risk for housing. */
            if (building_kind >= 0x82 && building_kind <= 0xa1) {
                if (plague_accident == plague_pass_count) {
                    plague_an_atom(cm_sptr);
                    put_message(0x51, cm_sptr, 22);
                    plague_accident = 0xf423f;
                    continue;
                }
                plague_flags = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).fpu_flag & 0x30;
                if (plague_flags == 0x30) plague_pass_count++;
            }

            /* Busy or unfinished buildings do not provide services. */
            activity_flags = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a & 0x0f;
            if (activity_flags != 0) continue;

            /* Accumulate census totals for the building kind. */
            if (building_kind >= 0x82 && building_kind <= 0xa1) {
                building_kind -= 0x82;
                population_pass_count += houses_to_people[building_kind];
                culture_flags = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).range_flag & 0x0c;
                if (culture_flags) pop_income_pass_count += houses_to_income[building_kind];
            } else if (building_kind >= 0xae && building_kind <= 0xb9) {
                building_kind -= 0xae;
                if      (building_kind < 4) small_forums_pass_count++;
                else if (building_kind < 8) medium_forums_pass_count++;
                else               large_forums_pass_count++;
            } else if (building_kind >= 0xa2 && building_kind <= 0xa5) {
                small_temples_pass_count++;
                robbery_flags = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).range_flag & 0x30;
                culture_flags = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).range_flag & 0x0c;
                if (culture_flags) small_temples_culture_pass_count++;
                if (robbery_flags == 0) small_robbery_pass_count++;
            } else if (building_kind >= 0xa6 && building_kind <= 0xa9) {
                med_temples_pass_count++;
                robbery_flags = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).range_flag & 0x30;
                if (test_for_any_admin(cm_sptr, 2)) med_temples_culture_pass_count++;
                if (robbery_flags == 0) med_robbery_pass_count++;
            } else if (building_kind >= 0xaa && building_kind <= 0xad) {
                large_temples_pass_count++;
                robbery_flags = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).range_flag & 0x30;
                if (test_for_any_admin(cm_sptr, 3)) large_temples_culture_pass_count++;
                if (robbery_flags == 0) large_robbery_pass_count++;
            } else if (building_kind >= 0xdf && building_kind <= 0xe2) {
                baths_pass_count++;
                education_flags = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).education & 4;
                if (education_flags) supplied_baths_pass_count++;
            } else if (building_kind >= 0xdb && building_kind <= 0xde) {
                fountains_pass_count++;
                education_flags = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).education & 4;
                if (education_flags) supplied_fountains_pass_count++;
            } else if (building_kind == 0xe3) prefectures_pass_count++;
              else if (building_kind == 0xe4) barracks_pass_count++;
              else if (building_kind == 0xf3) {
                grammaticus_pass_count++;
                if (test_for_any_admin(cm_sptr, 2)) grammaticus_culture_pass_count++;
            } else if (building_kind == 0xf4) {
                rhetor_pass_count++;
                if (test_for_any_admin(cm_sptr, 3)) rhetor_culture_pass_count++;
            } else if (building_kind == 0xe5) {
                theatre_pass_count++;
                if (test_for_any_admin(cm_sptr, 2)) theatre_culture_pass_count++;
            } else if (building_kind == 0xe6) {
                odium_pass_count++;
                if (test_for_any_admin(cm_sptr, 2)) odium_culture_pass_count++;
            } else if (building_kind == 0xe7) {
                arena_pass_count++;
                if (test_for_any_admin(cm_sptr, 3)) arena_culture_pass_count++;
            } else if (building_kind == 0xe8) {
                colosseum_pass_count++;
                if (test_for_any_admin(cm_sptr, 3)) colosseum_culture_pass_count++;
            } else if (building_kind == 0xe9 || building_kind == 0xeb) {
                circus_pass_count++;
                if (test_for_any_admin(cm_sptr, 3)) circus_culture_pass_count++;
            } else if (building_kind == 0xed || building_kind == 0xef) {
                circus_maximus_pass_count++;
                if (test_for_any_admin(cm_sptr, 4)) circus_maximus_culture_pass_count++;
            } else if (building_kind >= 0xfc && building_kind <= 0xff) { market_pass_count++;
            } else if (building_kind == 0xfb) {
                hospitals_pass_count++;
                if (test_perimeter_for_road_and_forum(col_idx, evolve_row + row_idx, 3, 0)) accessed_hospitals_pass_count++;
            } else if (building_kind == 0xf5) {
                libraries_pass_count++;
                if (test_perimeter_for_road_and_forum(col_idx, evolve_row + row_idx, 3, 0)) accessed_libraries_pass_count++;
            } else if (building_kind == 0xfa) {
                business_pass_count++;
                industry_idx = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).business & 0x0f;
                temp_demand_count[industry_idx]++;
                culture_flags = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).range_flag & 0x0c;
                if (culture_flags) {
                    building_tier = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building & 0xf0;
                    building_tier >>= 4;
                    ind_income_pass_count += building_tier * 0x46;
                }
            }
            }
        } }
}

// Test whether a square footprint borders a qualifying road and, if required, forum coverage.
// FUNCTION: C2 0x452d0
// FUNCTION: C2WIN 0x00431c26
int test_perimeter_for_road_and_forum(int cell_x, int cell_y, int footprint_size, int road_only)
{
    int i;
    int perimeter_sptr;

    if (cell_y > 0) {
        perimeter_sptr = cm_sptr - 1600;
        for (i = 0; i < footprint_size; i++, perimeter_sptr += 20) {
            if (((*(struct city_cell *)((unsigned char *)city_map + (perimeter_sptr))).terrain & 0x20) != 0) {
                if (road_only != 0) return 1;
                if (((*(struct city_cell *)((unsigned char *)city_map + (perimeter_sptr))).range_flag & 0x0c) != 0) return 1;
            }
        }
    }
    if (80 - footprint_size > cell_y) {
        perimeter_sptr = cm_sptr + footprint_size * 1600;
        for (i = 0; i < footprint_size; i++, perimeter_sptr += 20) {
            if (((*(struct city_cell *)((unsigned char *)city_map + (perimeter_sptr))).terrain & 0x20) != 0) {
                if (road_only != 0) return 1;
                if (((*(struct city_cell *)((unsigned char *)city_map + (perimeter_sptr))).range_flag & 0x0c) != 0) return 1;
            }
        }
    }
    if (cell_x > 0) {
        perimeter_sptr = cm_sptr - 20;
        for (i = 0; i < footprint_size; i++, perimeter_sptr += 1600) {
            if (((*(struct city_cell *)((unsigned char *)city_map + (perimeter_sptr))).terrain & 0x20) != 0) {
                if (road_only != 0) return 1;
                if (((*(struct city_cell *)((unsigned char *)city_map + (perimeter_sptr))).range_flag & 0x0c) != 0) return 1;
            }
        }
    }
    if (cell_x < 80 - footprint_size) {
        perimeter_sptr = cm_sptr + footprint_size * 20;
        for (i = 0; i < footprint_size; i++, perimeter_sptr += 1600) {
            if (((*(struct city_cell *)((unsigned char *)city_map + (perimeter_sptr))).terrain & 0x20) != 0) {
                if (road_only != 0) return 1;
                if (((*(struct city_cell *)((unsigned char *)city_map + (perimeter_sptr))).range_flag & 0x0c) != 0) return 1;
            }
        }
    }
    return 0;
}

// Test whether any cell in a square has administrative range coverage.
// FUNCTION: C2 0x45422
// FUNCTION: C2WIN 0x00431e5e
int test_for_any_admin(int start_sptr, int footprint_size)
{
    int row_idx;
    int col_idx;
    int sptr_skip = 1600 - footprint_size * 20;
    unsigned char range_flags;
    for (row_idx = 0; row_idx < footprint_size; row_idx++, start_sptr += sptr_skip) {
        for (col_idx = 0; col_idx < footprint_size; col_idx++, start_sptr += 20) {
            range_flags = (*(struct city_cell *)((unsigned char *)city_map + (start_sptr))).range_flag & 0x0c;
            if (range_flags) {
                return 1;
            }
        }
    }
    return 0;
}

// Accumulate fire risk from staffing shortfalls and select an accident site at the threshold.
// FUNCTION: C2 0x4546c
// FUNCTION: C2WIN 0x00431ef8
void fire_trouble(void)
{
    fire_accident = 0xf423f;
    if (slave_requirements[1].max == 0) return;
    get_fire_cover();
    fire_rate += 0x64 - fire_cover;
    if (fire_rate <= 0) fire_rate = 0;
    if (fire_rate >= 0x64) {
        fire_rate = fire_rate % 0x64;
        fire_accident = get_rand_max(fire_pass_count);
    }
}

// Accumulate road-failure risk from staffing shortfalls and select a site at the threshold.
// FUNCTION: C2 0x454dd
// FUNCTION: C2WIN 0x00431f7f
void road_trouble(void)
{
    road_accident = 0xf423f;
    if (slave_requirements[2].max == 0) return;
    get_road_cover();
    road_rate += 0x64 - road_cover;
    if (road_rate <= 0) road_rate = 0;
    if (road_rate >= 0x64) {
        road_rate = road_rate % 0x64;
        road_accident = get_rand_max(road_pass_count);
    }
}

// Convert the current water coverage into the 0–16 water-trouble rating.
// FUNCTION: C2 0x4554e
// FUNCTION: C2WIN 0x00432006
void water_trouble(void)
{
    get_water_cover();
    if (water_cover <= 0x0a) { water_trouble_rate = 0;    return; }
    if (water_cover <= 0x16) { water_trouble_rate = 2;    return; }
    if (water_cover <= 0x1c) { water_trouble_rate = 3;    return; }
    if (water_cover <= 0x22) { water_trouble_rate = 4;    return; }
    if (water_cover <= 0x28) { water_trouble_rate = 5;    return; }
    if (water_cover <= 0x2e) { water_trouble_rate = 6;    return; }
    if (water_cover <= 0x34) { water_trouble_rate = 7;    return; }
    if (water_cover <= 0x3a) { water_trouble_rate = 8;    return; }
    if (water_cover <= 0x40) { water_trouble_rate = 9;    return; }
    if (water_cover <= 0x46) { water_trouble_rate = 0xa;  return; }
    if (water_cover <= 0x4c) { water_trouble_rate = 0xb;  return; }
    if (water_cover <= 0x52) { water_trouble_rate = 0xc;  return; }
    if (water_cover <= 0x58) { water_trouble_rate = 0xd;  return; }
    if (water_cover <= 0x5e) { water_trouble_rate = 0xe;  return; }
    if (water_cover <  0x64) { water_trouble_rate = 0xf;  return; }
    water_trouble_rate = 0x10;
}

// Accumulate wall-collapse risk from staffing shortfalls and select a site at the threshold.
// FUNCTION: C2 0x45674
// FUNCTION: C2WIN 0x004321c4
void wall_trouble(void)
{
    wall_accident = 0xf423f;
    if (slave_requirements[4].max == 0) return;
    get_wall_cover();
    wall_rate += 0x64 - wall_cover;
    if (wall_rate <= 0) wall_rate = 0;
    if (wall_rate >= 0x64) {
        wall_rate = wall_rate % 0x64;
        wall_accident = get_rand_max(structure_pass_count);
    }
}

// Compute firefighter staffing coverage and flag an insufficient workforce.
// FUNCTION: C2 0x456e5
// FUNCTION: C2WIN 0x0043224b
void get_fire_cover(void)
{
    if (slave_requirements[1].max == 0) {
        fire_cover = 100;
    } else if (tutorial_mode != 0 && tutorial_page < 22) {
        fire_cover = 100;
    } else {
        fire_cover = valueDIVtotal(slave_requirements[1].current,
                                   slave_requirements[1].max);
        if (fire_cover >= 100) {
            fire_cover = 100;
        } else {
            ++slave_warning;
        }
        if (fire_cover < 0) fire_cover = 0;
    }
}

// Compute road-maintenance staffing coverage and flag an insufficient workforce.
// FUNCTION: C2 0x4574b
// FUNCTION: C2WIN 0x004322ed
void get_road_cover(void)
{
    if (slave_requirements[2].max == 0) {
        road_cover = 100;
    } else if (tutorial_mode != 0 && tutorial_page < 22) {
        road_cover = 100;
    } else {
        road_cover = valueDIVtotal(slave_requirements[2].current,
                                   slave_requirements[2].max);
        if (road_cover >= 100) {
            road_cover = 100;
        } else {
            ++slave_warning;
        }
        if (road_cover < 0) road_cover = 0;
    }
}

// Compute wall-maintenance staffing coverage and flag an insufficient workforce.
// FUNCTION: C2 0x457b1
// FUNCTION: C2WIN 0x0043238f
void get_wall_cover(void)
{
    if (slave_requirements[4].max == 0) {
        wall_cover = 100;
    } else if (tutorial_mode != 0 && tutorial_page < 22) {
        wall_cover = 100;
    } else {
        wall_cover = valueDIVtotal(slave_requirements[4].current,
                                   slave_requirements[4].max);
        if (wall_cover >= 100) {
            wall_cover = 100;
        } else {
            ++slave_warning;
        }
        if (wall_cover < 0) wall_cover = 0;
    }
}

// Compute water-service staffing coverage and flag an insufficient workforce.
// FUNCTION: C2 0x45817
// FUNCTION: C2WIN 0x00432431
void get_water_cover(void)
{
    if (slave_requirements[3].max == 0) {
        water_cover = 100;
    } else if (tutorial_mode != 0 && tutorial_page < 22) {
        water_cover = 100;
    } else {
        water_cover = valueDIVtotal(slave_requirements[3].current,
                                    slave_requirements[3].max);
        if (water_cover >= 100) {
            water_cover = 100;
        } else {
            ++slave_warning;
        }
        if (water_cover < 0) water_cover = 0;
    }
}

// Compute hospital capacity coverage, allowing 1,000 residents per accessible hospital.
// FUNCTION: C2 0x4587d
// FUNCTION: C2WIN 0x004324d3
void hospital_coverage(void)
{
    hospital_cover = 0;
    if (accessed_hospitals_count <= 0) return;

    if (population < 100) {
        hospital_cover = 100;
    } else {
        hospital_cover = valueDIVtotal(accessed_hospitals_count * 1000,
                                        population);
    }
}

// Compute library capacity coverage, allowing 1,200 residents per accessible library.
// FUNCTION: C2 0x458ca
// FUNCTION: C2WIN 0x0043253b
void library_coverage(void)
{
    library_cover = 0;
    if (accessed_libraries_count <= 0) return;

    if (population < 100) {
        library_cover = 100;
    } else {
        library_cover = valueDIVtotal(accessed_libraries_count * 1200,
                                       population);
    }
}

// Recompute employees and the employment rate from service buildings and conscription.
// FUNCTION: C2 0x45919
// FUNCTION: C2WIN 0x004325a3
void employment(void)
{
    employees = 0;
    employees += small_forums_pass_count          *  40;
    employees += medium_forums_pass_count         *  80;
    employees += large_forums_pass_count          * 120;
    employees += small_temples_pass_count         *  10;
    employees += med_temples_pass_count           *  20;
    employees += large_temples_pass_count         *  30;
    employees += supplied_baths_pass_count        *  20;
    employees += prefectures_pass_count           *  25;
    employees += barracks_pass_count              *  30;
    employees += grammaticus_pass_count           *  30;
    employees += rhetor_pass_count                *  80;
    employees += accessed_libraries_pass_count    *  60;
    employees += theatre_pass_count               *  25;
    employees += odium_pass_count                 *  30;
    employees += arena_pass_count                 *  50;
    employees += colosseum_pass_count             *  60;
    employees += circus_pass_count                *  80;
    employees += circus_maximus_pass_count        * 100;
    employees += accessed_hospitals_pass_count    *  80;
    employees += market_pass_count                *  20;
    employees += business_pass_count              *  60;

    employment_rate  = valueDIVtotal(employees, population);
    employment_rate += conscription_rate;

    if (population < 50) employment_rate = 100;
    if (employment_rate > 100) employment_rate = 100;
}

// Accumulate population tax from the current census income and tax rate.
// FUNCTION: C2 0x45b7b
// FUNCTION: C2WIN 0x00432787
void running_pop_tax(void)
{
    int tax;

    tax = totalXpercent(pop_income_pass_count * income_multiple, pop_tax_rate);
    pop_tax_running_total += tax;
    pop_tax_last_count = pop_income_pass_count;
    pop_tax_counts++;
}

// Accumulate industry tax from the current census income and tax rate.
// FUNCTION: C2 0x45bab
// FUNCTION: C2WIN 0x004327cc
void running_ind_tax(void)
{
    int tax;

    tax = totalXpercent(ind_income_pass_count * income_multiple, ind_tax_rate);
    ind_tax_running_total += tax;
    ind_tax_last_count = ind_income_pass_count;
    ind_tax_counts++;
}

// Tally regional buildings, connected towns, and empire links across the region map.
// FUNCTION: C2 0x45bdb
// FUNCTION: C2WIN 0x00432811
void region_census(void)
{
    int i;
    int j;
    unsigned char image;
    unsigned char value;
    unsigned char connection;
    unsigned char occupant;

    no_of_workcamps = no_of_warehouses = no_of_shipyards = no_of_ports = 0;
    no_of_trading_posts = no_of_farms = no_of_mines = no_of_quarrys = 0;
    no_of_villages = no_of_towns = no_of_border_towns = 0;
    no_of_connected_towns     = 0;
    no_of_empire_connections  = 0;

    for (i = 0; i < 4; i++)
        empire_connections[i] = 0;

    if (c2inf.peace_mode) {
        no_of_empire_connections = 4;
        for (i = 0; i < 4; i++)
            empire_connections[i] = 1;
        return;
    }

    i = 0;
    cm_sptr = 0;
    for ( ; i < 0x3c; i++) {
    for (j = 0; j < 0x3c; j++, cm_sptr += 8) {
    value = (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).occupant & 3;
    /* Warehouses (kind 0xD4) ignore the low-bit ignore mask. */
    if ((*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).base_kind == 0xd4) value = 0;
    if (value != 0) continue;

    image = (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).base_kind;
    if      (image == 0xd3) { no_of_workcamps++; }
    else if (image == 0xd4) { no_of_warehouses++; }
    else if (image == 0xd5) { no_of_shipyards++; }
    else if (image >= 0xdc && image <= 0xdf) { no_of_farms++; }
    else if (image >= 0xe0 && image <= 0xe3) { no_of_mines++; }
    else if (image >= 0xe4 && image <= 0xe7) { no_of_quarrys++; }
    else if (image >= 0xe8 && image <= 0xeb) { no_of_trading_posts++; }
    else if (image >= 0xec && image <= 0xef) {
        no_of_ports++;
        connection = (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).edge_bits & 0x20;
        if (connection) {
            occupant = (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).occupant & 0x60;
            if (occupant == 0 && empire_connections[0] == 0) {
                empire_connections[0] = 1;
                no_of_empire_connections++;
            } else if (occupant == 0x20 && empire_connections[1] == 0) {
                empire_connections[1] = 1;
                no_of_empire_connections++;
            } else if (occupant == 0x40 && empire_connections[2] == 0) {
                empire_connections[2] = 1;
                no_of_empire_connections++;
            } else if (occupant == 0x60 && empire_connections[3] == 0) {
                empire_connections[3] = 1;
                no_of_empire_connections++;
            }
        }
    }
    else if (image >= 0x93 && image <= 0x96) { no_of_villages++; }
    else if (image >= 0x98 && image <= 0x9b) {
        no_of_border_towns++;
        connection = (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).edge_bits & 0x20;
        if (connection) {
            no_of_empire_connections++;
            empire_connections[image - 0x98] = 1;
        }
    }
    else if (image == 0x97) {
        connection = (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).edge_bits & 0x20;
        if (connection) no_of_connected_towns++;
        no_of_towns++;
    }
    }
    }
}
