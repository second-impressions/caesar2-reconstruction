
#include <fcntl.h>       /* O_BINARY: 0x200 under Watcom, 0x8000 under MSVC */
#if PLATFORM_WINDOWS
#include <sys/stat.h>
#include <windows.h>
#endif
#include "c2_data.h"
#include "c2_types.h"

#if PLATFORM_WINDOWS
extern int savegame_version;
extern unsigned char window_status[];
extern int saved_window_status[];
extern int saved_game_window_status;
extern RECT game_window_rect;
extern HWND game_window;
extern HWND status_window;
extern HWND map_window;
extern int game_window_x;
extern int game_window_y;
extern int game_window_width;
extern int game_window_height;
extern int status_window_x;
extern int status_window_y;
extern int status_window_width;
extern int status_window_height;
extern int map_window_x;
extern int map_window_y;
#endif

struct save_entry model_entries[40] = {
    { skill_to_imperial_request, 20 },
    { skill_to_starting_denarii, 20 },
    { skill_to_denarii_reduction, 20 },
    { skill_to_trouble_honeymoons, 80 },
    { skill_to_trouble_frequency, 80 },
    { skill_to_trouble_debar, 80 },
    { skill_to_city_attacks, 80 },
    { city_costs, 400 },
    { region_costs, 80 },
    { houses_to_people, 128 },
    { houses_to_income, 128 },
    { pop_tax_to_growth_data, 104 },
    { employment_to_pop_growth_factor, 84 },
    { ind_tax_to_growth_data, 104 },
    { tax_to_revolt_data, 104 },
    { conscription_to_revolt_data, 104 },
    { house_type_to_unrest, 128 },
    { unrest_random_data, 256 },
    { house_lv_effect, 256 },
    { forum_lv_effect, 96 },
    { temple_lv_effect, 96 },
    { tribe_to_troop_numbers, 480 },
    { buildings_lv_effect, 224 },
    { init_salary, 8 },
    { promotion_levels, 400 },
    { promotion_av_levels, 400 },
    { init_slave_data, 80 },
    { main_paras, 40 },
    { tax_triggers, 12 },
    { tax_rates, 240 },
    { tribute_adjust, 28 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 }
};

struct save_entry savegame_entries[500] = {
    { &map_mode, 1 },
    { &ov_map_mode, 1 },
    { &zoom_level, 1 },
    { &zoom_level, 1 },
    { &map_direction, 4 },
    { &pm_x, 4 },
    { &pm_y, 4 },
    { army_list, 4550 },
    { citizen_list, 11658 },
    { unit_list, 3978 },
    { figure_list, 17688 },
    { arrow_list, 9045 },
    { army_routes, 3460 },
    { &city_map, 128000 },
    { &region_map, 28800 },
    { fire_zones, 100 },
    { (char *)&c2inf + 52, 1 },
    { &randseed, 4 },
    { &top_lv_spot, 4 },
    { &top_lv, 4 },
    { &top_lv_x, 4 },
    { &top_lv_y, 4 },
    { &evolve_tick4, 4 },
    { &evolve_row, 4 },
    { &evolve_clock, 4 },
    { &year, 4 },
    { &month, 4 },
    { &week, 4 },
    { &denarii, 4 },
    { &pop_tax_rate, 4 },
    { &ind_tax_rate, 4 },
    { &employment_rate, 4 },
    { &population, 4 },
    { &account_total, 4 },
    { &account_pop_tax, 4 },
    { &account_ind_tax, 4 },
    { &account_construction_cost, 4 },
    { &account_operating_cost, 4 },
    { &estimate_total, 4 },
    { &estimate_pop_tax, 4 },
    { &estimate_ind_tax, 4 },
    { &estimate_construction_cost, 4 },
    { &estimate_operating_cost, 4 },
    { &no_of_census_passes, 4 },
    { &population_running_count, 4 },
    { &employment_running_count, 4 },
    { &structure_running_count, 4 },
    { &road_running_count, 4 },
    { &fire_running_count, 4 },
    { &plague_running_count, 4 },
    { &water_running_count, 4 },
    { &region_running_count, 4 },
    { &slaves, 4 },
    { &slave_population_change, 4 },
    { &slave_welfare_bill, 4 },
    { &slave_population_estimate, 4 },
    { slave_requirements, 64 },
    { &slave_random, 4 },
    { &population_pass_count, 4 },
    { &employment_pass_count, 4 },
    { &structure_pass_count, 4 },
    { &road_pass_count, 4 },
    { &fire_pass_count, 4 },
    { &plague_pass_count, 4 },
    { &region_pass_count, 4 },
    { &fountains_pass_count, 4 },
    { &fountains_count, 4 },
    { &supplied_fountains_pass_count, 4 },
    { &supplied_fountains_count, 4 },
    { &baths_pass_count, 4 },
    { &baths_count, 4 },
    { &supplied_baths_pass_count, 4 },
    { &supplied_baths_count, 4 },
    { &large_forums_pass_count, 4 },
    { &large_forums_count, 4 },
    { &medium_forums_pass_count, 4 },
    { &medium_forums_count, 4 },
    { &small_forums_pass_count, 4 },
    { &small_forums_count, 4 },
    { &forts_pass_count, 4 },
    { &forts_count, 4 },
    { &prefectures_pass_count, 4 },
    { &prefectures_count, 4 },
    { &barracks_pass_count, 4 },
    { &barracks_count, 4 },
    { &large_temples_pass_count, 4 },
    { &large_temples_count, 4 },
    { &med_temples_pass_count, 4 },
    { &med_temples_count, 4 },
    { &small_temples_pass_count, 4 },
    { &small_temples_count, 4 },
    { &hospitals_pass_count, 4 },
    { &hospitals_count, 4 },
    { &accessed_hospitals_pass_count, 4 },
    { &accessed_hospitals_count, 4 },
    { &libraries_pass_count, 4 },
    { &libraries_count, 4 },
    { &accessed_libraries_pass_count, 4 },
    { &accessed_libraries_count, 4 },
    { &grammaticus_pass_count, 4 },
    { &grammaticus_count, 4 },
    { &rhetor_pass_count, 4 },
    { &rhetor_count, 4 },
    { &theatre_pass_count, 4 },
    { &theatre_count, 4 },
    { &odium_pass_count, 4 },
    { &odium_count, 4 },
    { &arena_pass_count, 4 },
    { &arena_count, 4 },
    { &colosseum_pass_count, 4 },
    { &colosseum_count, 4 },
    { &circus_pass_count, 4 },
    { &circus_count, 4 },
    { &circus_maximus_pass_count, 4 },
    { &circus_maximus_count, 4 },
    { &business_pass_count, 4 },
    { &business_count, 4 },
    { &market_pass_count, 4 },
    { &market_count, 4 },
    { &water_trouble_rate, 4 },
    { &road_rate, 4 },
    { &road_accident, 4 },
    { &fire_rate, 4 },
    { &fire_accident, 4 },
    { &wall_rate, 4 },
    { &wall_accident, 4 },
    { &region_rate, 4 },
    { &region_accident, 4 },
    { &fire_spread_direction, 4 },
    { &fire_spread_count, 4 },
    { &fire_spread_target, 4 },
    { &plague_spread_direction, 4 },
    { &plague_spread_count, 4 },
    { &plague_spread_target, 4 },
    { &hospital_cover, 4 },
    { &library_cover, 4 },
    { &plague_accident, 4 },
    { &revolt_accident, 4 },
    { &shell_push_direction, 4 },
    { &pop_growth_factor, 4 },
    { &ind_growth_factor, 4 },
    { &insurrection_factor, 4 },
    { &pop_growth_future, 4 },
    { &ind_growth_future, 4 },
    { &insurrection_future, 4 },
    { &conscription_rate, 4 },
    { &employees, 4 },
    { &pop_income_pass_count, 4 },
    { &pop_tax_running_total, 4 },
    { &pop_tax_last_count, 4 },
    { &pop_tax_counts, 4 },
    { &ind_income_pass_count, 4 },
    { &ind_tax_running_total, 4 },
    { &ind_tax_last_count, 4 },
    { &ind_tax_counts, 4 },
    { &current_construction_cost, 4 },
    { &current_operating_cost, 4 },
    { &account_tribute, 4 },
    { &estimate_tribute, 4 },
    { &income_multiple, 4 },
    { &average_pop_tax_denariis, 4 },
    { &average_pop_tax_asses, 4 },
    { &average_ind_tax_denariis, 4 },
    { &average_ind_tax_asses, 4 },
    { &tribute, 4 },
    { &last_tribute, 4 },
    { &unrest_random_count, 4 },
    { &no_of_rioters, 4 },
    { &no_of_barbarians, 4 },
    { &free_message_ptr, 4 },
    { &show_message_ptr, 4 },
    { message_list, 64 },
    { &warned_of_fire, 1 },
    { &warned_of_cutbacks, 1 },
    { &warned_of_robbery, 1 },
    { &warned_of_emperor, 1 },
    { &warned_of_emperor_reply_month, 1 },
    { &warned_of_emperor_reply_level, 1 },
    { &warned_of_free4, 1 },
    { &warned_of_free5, 1 },
    { &large_robbery_pass_count, 4 },
    { &large_robbery_count, 4 },
    { &med_robbery_pass_count, 4 },
    { &med_robbery_count, 4 },
    { &small_robbery_pass_count, 4 },
    { &small_robbery_count, 4 },
    { &temples_count, 4 },
    { &robbery_count, 4 },
    { &stolen_denarii, 4 },
    { &reg_city_x, 4 },
    { &reg_city_y, 4 },
    { &our_battle_army, 4 },
    { &their_battle_army, 4 },
    { &forum_viewed_army, 4 },
    { &army_wage_level, 4 },
    { &conscription_rate, 4 },
    { &total_no_of_cohorts, 4 },
    { &total_no_of_centuries, 4 },
    { &total_no_of_soldiers, 4 },
    { &total_no_of_regulars, 4 },
    { &total_no_of_irregulars, 4 },
    { &total_no_of_auxillaries, 4 },
    { &current_no_of_centuries, 4 },
    { &current_no_of_soldiers, 4 },
    { &current_no_of_regulars, 4 },
    { &current_no_of_irregulars, 4 },
    { &current_no_of_auxillaries, 4 },
    { &needed_no_of_centuries, 4 },
    { &needed_no_of_soldiers, 4 },
    { &needed_no_of_regulars, 4 },
    { &needed_no_of_irregulars, 4 },
    { &needed_no_of_auxillaries, 4 },
    { &average_cohort_morale, 4 },
    { &average_cohort_readiness, 4 },
    { &last_adjusted_cohort, 4 },
    { &west_border_x, 4 },
    { &west_border_y, 4 },
    { &east_border_x, 4 },
    { &east_border_y, 4 },
    { &north_border_x, 4 },
    { &north_border_y, 4 },
    { &south_border_x, 4 },
    { &south_border_y, 4 },
    { &province_is, 4 },
    { &north_trader_count0, 4 },
    { &east_trader_count0, 4 },
    { &south_trader_count0, 4 },
    { &west_trader_count0, 4 },
    { &north_trader_brings, 4 },
    { &east_trader_brings, 4 },
    { &south_trader_brings, 4 },
    { &west_trader_brings, 4 },
    { &north_trader_is, 4 },
    { &east_trader_is, 4 },
    { &south_trader_is, 4 },
    { &west_trader_is, 4 },
    { &north_trader_count1, 4 },
    { &east_trader_count1, 4 },
    { &south_trader_count1, 4 },
    { &west_trader_count1, 4 },
    { &no_of_workcamps, 4 },
    { &no_of_warehouses, 4 },
    { &no_of_shipyards, 4 },
    { &no_of_ports, 4 },
    { &no_of_trading_posts, 4 },
    { &no_of_farms, 4 },
    { &no_of_mines, 4 },
    { &no_of_quarrys, 4 },
    { &no_of_villages, 4 },
    { &no_of_towns, 4 },
    { &no_of_border_towns, 4 },
    { &months_to_game_over, 4 },
    { &old_map_mode, 1 },
    { &old_zoom_level, 1 },
    { &battle_state, 4 },
    { &battle_map, 10816 },
    { &city_pm_x, 4 },
    { &city_pm_y, 4 },
    { &region_pm_x, 4 },
    { &region_pm_y, 4 },
    { &battle_pm_x, 4 },
    { &battle_pm_y, 4 },
    { &city_direction, 4 },
    { &region_direction, 4 },
    { &battle_direction, 4 },
    { &battle_scale, 4 },
    { &our_battle_men, 4 },
    { &our_battle_start_men, 4 },
    { &their_battle_men, 4 },
    { &their_battle_start_men, 4 },
    { &our_battle_routs, 4 },
    { &their_battle_routs, 4 },
    { &our_battle_stance, 4 },
    { &their_battle_stance, 4 },
    { &battle_npc_retreat_count, 4 },
    { &retreat_flag, 4 },
    { &no_of_empire_connections, 4 },
    { empire_connections, 4 },
    { &battle_over_count, 4 },
    { &our_battle_morale, 4 },
    { &their_battle_morale, 4 },
    { &our_battle_units, 4 },
    { &their_battle_units, 4 },
    { empire, 50 },
    { &completed_provinces, 4 },
    { &auto_conquered, 4 },
    { &empire_rating, 4 },
    { &peace_rating, 4 },
    { &prosperity_rating, 4 },
    { &culture_rating, 4 },
    { &pax_romanum, 4 },
    { &player_rank, 4 },
    { &large_temples_culture_pass_count, 4 },
    { &large_temples_culture_count, 4 },
    { &med_temples_culture_pass_count, 4 },
    { &med_temples_culture_count, 4 },
    { &small_temples_culture_pass_count, 4 },
    { &small_temples_culture_count, 4 },
    { &theatre_culture_pass_count, 4 },
    { &theatre_culture_count, 4 },
    { &odium_culture_pass_count, 4 },
    { &odium_culture_count, 4 },
    { &arena_culture_pass_count, 4 },
    { &arena_culture_count, 4 },
    { &colosseum_culture_pass_count, 4 },
    { &colosseum_culture_count, 4 },
    { &circus_culture_pass_count, 4 },
    { &circus_culture_count, 4 },
    { &circus_maximus_culture_pass_count, 4 },
    { &circus_maximus_culture_count, 4 },
    { &grammaticus_culture_pass_count, 4 },
    { &grammaticus_culture_count, 4 },
    { &plaza_culture_count, 4 },
    { &plaza_culture_pass_count, 4 },
    { &gardens_culture_count, 4 },
    { &gardens_culture_pass_count, 4 },
    { &rhetor_culture_pass_count, 4 },
    { &rhetor_culture_count, 4 },
    { &refused_promotion, 4 },
    { &entertainment_level, 4 },
    { &religion_level, 4 },
    { &utility_level, 4 },
    { &rolling_profit, 4 },
    { &current_gdp, 4 },
    { &no_of_connected_towns, 4 },
    { &start_year, 4 },
    { &years_elapsed, 4 },
    { &months_since_last_war, 4 },
    { &months_since_last_horde, 4 },
    { &months_since_last_raider, 4 },
    { &months_since_last_revolt, 4 },
    { &years_elapsed_in_region, 4 },
    { hut_list, 12 },
    { &roman_name_count, 4 },
    { &barbarian_name_count, 4 },
    { province_industries, 256 },
    { &history_end_ptr, 4 },
    { &history_start_ptr, 4 },
    { &history_entries, 4 },
    { industry, 768 },
    { &imperial_favour, 4 },
    { &imperial_request, 4 },
    { &imperial_review, 4 },
    { &imperial_req_goods, 4 },
    { &imperial_req_amount, 4 },
    { &av_imperial_gift_level, 4 },
    { &our_battle_specials, 4 },
    { &our_battle_horse, 4 },
    { &our_battle_regs, 4 },
    { &our_battle_irregs, 4 },
    { &our_battle_auxs, 4 },
    { &their_battle_specials, 4 },
    { &their_battle_horse, 4 },
    { &their_battle_regs, 4 },
    { &their_battle_irregs, 4 },
    { &their_battle_auxs, 4 },
    { &mercs_in_army, 4 },
    { &max_mercs_allowed, 4 },
    { &mercs_type, 4 },
    { &mercs_from, 4 },
    { &mercs_cost_per_50, 4 },
    { &current_no_of_specials, 4 },
    { &needed_no_of_specials, 4 },
    { &total_no_of_specials, 4 },
    { &mercs_catagory, 4 },
    { &mercs_tribe, 4 },
    { &mercs_speed, 4 },
    { &mercs_missile, 4 },
    { &battle_type, 4 },
    { &battle2_ptr, 4 },
    { &game_state, 4 },
    { &return_zoom_level, 4 },
    { &return_map_mode, 4 },
    { &shell_wall_flag, 4 },
    { &shell_last_value, 4 },
    { message_list, 128 },
    { &average_rating, 4 },
    { &empire_rating_pop_limit, 4 },
    { &peace_rating_pop_limit, 4 },
    { &prosperity_rating_pop_limit, 4 },
    { &culture_rating_pop_limit, 4 },
    { &no_of_city_flags, 4 },
    { &no_of_prov_flags, 4 },
    { &no_of_danger_flags, 4 },
    { &last_city_flag, 4 },
    { &last_prov_flag, 4 },
    { &last_danger_flag, 4 },
    { city_flag_list, 80 },
    { prov_flag_list, 80 },
    { danger_flag_list, 80 },
    { &danger_flag_map_mode, 4 },
    { &flag_mode_decay_count, 4 },
    { &last_years_population, 4 },
    { &this_years_population, 4 },
    { &last_years_denarii, 4 },
    { &this_years_denarii, 4 },
    { &last_years_pop_tax, 4 },
    { &this_years_pop_tax, 4 },
    { &last_years_ind_tax, 4 },
    { &this_years_ind_tax, 4 },
    { &pop_growth_extra, 4 },
    { &pop_growth_extra_count, 4 },
    { &players_denarii, 4 },
    { &players_salary, 4 },
    { &donation_level, 4 },
    { &evolve_tick3, 4 },
    { (char *)&c2inf + 53, 1 },
    { &months_since_last_city_attack, 4 },
    { &auto_conquered_months, 4 },
    { &max_population, 4 },
    { &warned_city_size, 4 },
    { &warned_new_struct, 4 },
    { &last_city_mood, 4 },
    { &last_battle_mood, 4 },
    { empire_won, 200 },
    { &final_bribe, 4 },
    { &province_difficulty, 4 },
    { &total_amount_of_bribes, 4 },
    { &total_no_of_bribes, 4 },
    { &bat_tribe, 4 },
    { &battle_ai_count, 4 },
    { &moving_tribute, 4 },
    { &imperial_tax, 4 },
    { &last_imperial_tax_amount, 4 },
    { &total_imperial_taxes, 4 },
    { &last_imperial_tax_percent, 4 },
    { &city_rotation, 4 },
    { &city_zoom_level, 4 },
    { &prov_rotation, 4 },
    { &prov_zoom_level, 4 },
    { &arena_top_count, 4 },
    { &colosseum_top_count, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 },
    { &dummy_sav, 4 }
};

/* File-local state. */
int dummy_sav;


extern void font_list(int idx, int word_count, int x, int y, unsigned char *font, int color);

extern int read(int fd, void *buf, unsigned int size);
#if PLATFORM_WINDOWS
extern void set_raw_tune_volume(void);
#endif
/* Forward declarations (functions defined later in this file). */
void set_language(int language);
void test_inf_settings(void);
void basic_inf_settings(void);
void clear_huts(void);
void put_a_hut(int hut_x, int hut_y, int hut_kind);

// Open the load dialog, validate the selected .SAV file, and load it before restarting the game.
// FUNCTION: C2 0x70461
// FUNCTION: C2WIN 0x00482ac0
void load_a_game(void)
{
    int done;
#if PLATFORM_WINDOWS
    int oldmode;
#endif
    int i;

    file_loaded_status = 0;
    done = 0;
#if PLATFORM_WINDOWS
    oldmode = map_mode;
#endif
    get_directory("*.sav");
    show_loadsave_box(0x28);
    in_format_buffer(filename, 0xc, 0xa0, 1);

    while (done == 0) {
        if (select_filename(0x28) != 0) {
            done = check_file_exists(filename);
            if (done != 0) {
                get_filename_extension(filename);
                if (strcmp("SAV", extension) != 0) done = 0;
            }

            show_a_system_blank(0x3c, 0x168, 0x15, 1);
            show_a_system_blank(0x3c, 0x16c, 0x15, 1);
            if (done == 0) {
                font_list(0x2a, 0, 0x40, 0x16c, font1, 0x10);
            } else {
                font_list(0x2a, 1, 0x40, 0x16c, font1, 0x10);
                setup_whole_screen_refresh();
                refresh_svga_screen();
                loadgame(filename);
#if !PLATFORM_WINDOWS
                for (i = 0; i < 200; i++) just_idle_game_loop();
#endif
                pre_loaded_status = 2;
                restart_flag = 1;
            }
        } else {
            done = 1;
        }
        setup_whole_screen_refresh();
    }

    get_landfill(1);
    update_landfill = 1;
    hold_mouse_replace = 1;
}

// Open the save dialog, confirm overwrites, and save the game under the selected filename.
// FUNCTION: C2 0x705ba
// FUNCTION: C2WIN 0x00482c33
void save_a_game(void)
{
    int done;
    int i;

    file_loaded_status = 0;
    done = 0;
    decision = 0;
    get_directory("*.sav");
    show_loadsave_box(0x29);
    in_format_buffer(filename, 0xc, 0xa0, 1);

    while (done == 0) {
        if (select_filename(0x29) != 0) {
            done = check_file_exists(filename);
            if (done != 0) {
                confirm(2, 0xa0, 0xa0);
                if (decision == 0) done = 0;
            } else {
                done = 1;
            }

            show_loadsave_box(0x29);
            if (done == 0) {
                font_list(0x2a, 2, 0x40, 0x16c, font1, 0x10);
            } else {
                font_list(0x2a, 3, 0x40, 0x16c, font1, 0x10);
                setup_whole_screen_refresh();
                refresh_svga_screen();
                savegame(filename);
#if !PLATFORM_WINDOWS
                for (i = 0; i < 1000; i++) just_idle_game_loop();
#endif
                decision = 1;
            }
        } else {
            done = 1;
        }
        setup_whole_screen_refresh();
        hold_mouse_replace = 1;
    }
}

// Run the modal filename picker shared by the load and save dialogs.
// FUNCTION: C2 0x706d9
// FUNCTION: C2WIN 0x00482d77
int select_filename(int dialog_mode)
{
    int old_pointer_mode;
    int control_result;

    old_pointer_mode = pointer_mode;
    pointer_mode = 0;
    this_letter = insert_cursor = out1 = 0;
    clear_keys();
    adjust_var = &first_entry;
    adjust_step = 2;
    adjust_max = no_of_entries - 2;
    adjust_min = 0;

    while (out1 != 1) {
        cover_mouse_droppings();
        hold_hot_keys = 1;
        get_mouse();
        if (key_ascii == 0x20) key_ready = 0;
        show_buttons(0x20, 0x50, loadsave_buttons, 4);
        get_mouse_droppings();

        if (edit_format_buffer() != 0) {
            decision = 1;
            out1 = 1;
        }
        get_fb_length();
        out_format_buffer(filename);
        show_directory(file_no);

        show_mouse(0);
        set_mouse_refresh();
        if (mouse_movement != 0) {
            setup_refresh_area(0x3e, 0xb0, 0x15, 0xe, 2);
        }
        setup_refresh_area(0x30, 0x90, 0xc, 2, 2);
        refresh_svga_screen();
#if PLATFORM_WINDOWS
        if (control_buttons(0x20, 0x50, loadsave_buttons, 4) != 0);
#else
        control_buttons(0x20, 0x50, loadsave_buttons, 4);
#endif
        setup_refresh_area(0x3e, 0xb0, 0x15, 0xe, 2);

        file_no = 0x3e7;
        if (mouse_x >= 0x40 && mouse_x < 0x180) {
            if (mouse_y >= 0xbc && mouse_y < 0x15c) {
                if (mouse_x < 0xe0) {
                    file_no = first_entry;
                } else {
                    file_no = first_entry + 10;
                }
                file_no += (mouse_y - 0xbc) / 16;
            }
        }

        if (mouse_left_preclick != 0) {
            if (file_no < 0x3e7) {
                my_strcpy(directory[file_no], filename, 0xd);
                in_format_buffer(filename, 0xc, 0xa0, 1);
            }
        }

        if (out1 >= 1) {
            pointer_mode = old_pointer_mode;
            if (decision == 1) return 1;
            return 0;
        }
        button_time_flag = running_delay1();
    }

    cover_mouse_droppings();
    hold_mouse_replace = 1;
    pointer_mode = old_pointer_mode;
    return 0;
}

// Scroll the directory listing down one row without moving beyond the final visible page.
// FUNCTION: C2 0x7093b
// FUNCTION: C2WIN 0x0048302f
void act_down_directory(void)
{
    if (no_of_entries - 0xe > first_entry) first_entry++;
}

// Scroll the directory listing up by one entry, floored at 0.
// FUNCTION: C2 0x7095b
// FUNCTION: C2WIN 0x00483054
void act_up_directory(void)
{
    if (first_entry != 0) first_entry--;
}

// Select the dialog's file-operation action.
// FUNCTION: C2 0x70973
// FUNCTION: C2WIN 0x00483072
void act_do_file_op(void)     { out1 = 2; }

// Cancel the current file dialog.
// FUNCTION: C2 0x7097e
// FUNCTION: C2WIN 0x00483087
void act_cancel_file_op(void) { out1 = 1; }

// Write the registered game-state blocks and history data to a save file.
// FUNCTION: C2 0x70989
// FUNCTION: C2WIN 0x0048309c
int savegame(char *save_filename)
{
    int save_fd;
    int history_file;
    int i;
#if PLATFORM_WINDOWS
    RECT window_rect;

    c2inf.restore_window_positions = 1;
    savegame_version = 999;
    saved_window_status[0] = window_status[0];
    saved_window_status[1] = window_status[1];
    saved_window_status[2] = window_status[2];

    GetWindowRect(game_window, &window_rect);
    game_window_x = window_rect.left;
    game_window_y = window_rect.top;
    game_window_width = window_rect.right - window_rect.left;
    game_window_height = window_rect.bottom - window_rect.top;

    GetWindowRect(status_window, &window_rect);
    status_window_x = window_rect.left;
    status_window_y = window_rect.top;
    status_window_width = window_rect.right - window_rect.left;
    status_window_height = window_rect.bottom - window_rect.top;

    GetWindowRect(map_window, &window_rect);
    map_window_x = window_rect.left;
    map_window_y = window_rect.top;

    if (map_mode == 2 && saved_game_window_status != 0) {
        game_window_x = game_window_rect.left;
        game_window_y = game_window_rect.top;
        game_window_width = game_window_rect.right - game_window_rect.left;
        game_window_height = game_window_rect.bottom - game_window_rect.top;
    }
#endif

    save_fd = open(save_filename, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0x180);
    if (save_fd == -1) return 0;

    history_file = open("history.dat", O_BINARY);
    if (history_file == -1) {
        close(save_fd);
        return 0;
    }

    for (i = 0; i < 500; i++) {
        if (savegame_entries[i].size == 0) break;
        write(save_fd, savegame_entries[i].buf, savegame_entries[i].size);
    }

    read(history_file, ((void *)scratch_buffer), 0xfa0);
    write(save_fd, ((void *)scratch_buffer), 0xfa0);
    close(save_fd);
    close(history_file);

    map_gfx_loaded = 0;
    setup_map_screen_refresh();
    unflag_all_cm(3, 0xfd);
    update_landfill = 1;
    return 1;
}

// Restore the registered game-state blocks and history data, then rebuild the map display state.
// FUNCTION: C2 0x70a5c
// FUNCTION: C2WIN 0x004832e3
int loadgame(char *save_filename)
{
    int save_fd;
    int history_fd;
    int i;

    file_loaded_status = 0;
    clear_messages();

    save_fd = open(save_filename, O_BINARY);
    if (save_fd == -1) return 0;

    history_fd = open("history.dat", 0x261, 0x180);
    if (history_fd == -1) {
        close(save_fd);
        return 0;
    }

    for (i = 0; i < 500; i++) {
        if (savegame_entries[i].size == 0) break;
        read(save_fd, savegame_entries[i].buf, savegame_entries[i].size);
    }

    read(save_fd, ((void *)scratch_buffer), 0xfa0);
    write(history_fd, ((void *)scratch_buffer), 0xfa0);
    close(save_fd);
    close(history_fd);

    file_loaded_status = 1;
    map_gfx_loaded = 0;
    get_pseudo_map(map_direction);
    refresh_zoom_mode(zoom_level);
    load_map_graphics(map_mode, zoom_level);
    setup_map_screen_refresh();
    unflag_all_cm(3, 0xfd);
    pm_build_shape = 0;
    placing_type = 0;
    placing_flags = 0;
    if (city_rotation >= 8 || city_rotation < 0) city_rotation = 0;
    if (prov_rotation >= 8 || prov_rotation < 0) prov_rotation = 0;
    get_old_mood();
    if (no_of_warehouses != 0) c2inf.peace_mode = 0;
    return 1;
}

// Save the persistent preferences block to caesar2.inf.
// FUNCTION: C2 0x70bba
// FUNCTION: C2WIN 0x00483554
void save_inf(void)
{
    int inf_fd;

#if PLATFORM_WINDOWS
    c2inf.restore_window_positions = 1;
#endif
    inf_fd = open("caesar2.inf", O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0x180);
    if (inf_fd == -1) return;
    write(inf_fd, &c2inf, sizeof(c2inf));
    close(inf_fd);
}

// Load caesar2.inf while preserving the runtime drive settings initialized before the read.
// FUNCTION: C2 0x70bf5
// FUNCTION: C2WIN 0x004835b1
void load_inf(void)
{
    int old_cd;
    int old_drive;
    int file;
#if PLATFORM_WINDOWS
    struct stat file_stat;
#endif

    old_cd = c2inf.cd_letter;
    old_drive = c2inf.drive_init;

    get_directory("*.sav");
    if (no_of_entries != 0) {
        my_strcpy(directory, filename, 0xd);
    } else {
        my_strcpy("caesar2.sav", filename, 0xd);
    }
    first_entry = 0;
    file_no = 0;

    basic_inf_settings();
    set_language(c2inf.config37);

    file = open("caesar2.inf", 0x8404);
    if (file == -1) return;
#if PLATFORM_WINDOWS
    if (fstat(file, &file_stat) != 0) return;
#endif
    read(file, &c2inf, sizeof(c2inf));
    close(file);
    test_inf_settings();
#if PLATFORM_WINDOWS
    c2inf.restore_window_positions = file_stat.st_size > 0x40U;
#endif
    set_language(c2inf.config37);
    c2inf._unused_writeonly38 = 0;
    c2inf.cd_letter = old_cd;
    c2inf.drive_init = old_drive;
    set_samples_volume();
    set_sequences_volume();
#if PLATFORM_WINDOWS
    set_raw_tune_volume();
#endif
}

// Select language and help/media table filenames. English is the default; language ids 2/3/4
// replace it with German/French/Spanish.
// FUNCTION: C2 0x70cd1
// FUNCTION: C2WIN 0x004836fb
void set_language(int language)
{
    my_strcpy("c2.eng", lang_file, 0xc);
#if !PLATFORM_WINDOWS
    if (language == 2) {
        my_strcpy("c2.ger", lang_file, 0xc);
    } else if (language == 3) {
        my_strcpy("c2.fre", lang_file, 0xc);
    } else if (language == 4) {
        my_strcpy("c2.spa", lang_file, 0xc);
    }
#endif

    my_strcpy("help.eng", media_file, 0xc);
#if !PLATFORM_WINDOWS
    if (language == 2) {
        my_strcpy("help.ger", media_file, 0xc);
    } else if (language == 3) {
        my_strcpy("help.fre", media_file, 0xc);
    } else if (language == 4) {
        my_strcpy("help.spa", media_file, 0xc);
    }
#endif
}

// Validate the persistent settings and restore defaults when their year marker is invalid.
// FUNCTION: C2 0x70d94
// FUNCTION: C2WIN 0x0048372e
void test_inf_settings(void)
{
    if ((unsigned short)c2inf.starting_year != 0x7d5)
        basic_inf_settings();
    c2inf.skill_level = 0;
    c2inf.peace_mode = 1;
}

// Initialize the persistent player, speed, sound, autosave, and game-mode settings.
// FUNCTION: C2 0x70db8
// FUNCTION: C2WIN 0x0048375f
void basic_inf_settings(void)
{
    c2inf.starting_year = 0x7d5;
    my_strcpy("Octavian                ", c2inf.player_name, 0x19);
    c2inf.game_speed = 0x64;
    c2inf.scroll_speed = 0x64;
    c2inf.anims_on = 1;
    c2inf.yearend_on = 1;
    c2inf.ambients_on = 1;
    c2inf.speech_on = 1;
    c2inf.tunes_on = 1;
    c2inf.samples_on = 1;
    c2inf.autosave_on = 1;
    c2inf.samples_level = 0x64;
    c2inf.max_samples = 4;
    c2inf.tunes_level = 0x64;
    c2inf.paused = 0;
    c2inf._unused_writeonly36 = 1;
    c2inf.skill_level = 0;
    c2inf.peace_mode = 1;
    c2inf.config37 = 1;
#if PLATFORM_WINDOWS
    c2inf.restore_window_positions = 1;
    c2inf.invalidate_background = 0;
    c2inf.wallpaper = 0;
    c2inf.mouse_window_on = 1;
    c2inf.dual_window_mode = 1;
#endif
}

// Load registered model blocks until the first zero-size entry.
// FUNCTION: C2 0x70e57
// FUNCTION: C2WIN 0x00483834
int loadmodel(char *model_filename)
{
    int model_file;
    int i;

    model_file = open(model_filename, O_BINARY);
    if (model_file == -1) return 0;

    for (i = 0; i < 100; i++) {
        if (model_entries[i].size == 0) break;
        read(model_file, model_entries[i].buf, model_entries[i].size);
    }
    close(model_file);
    return 1;
}

// Replace control bytes in the first 24 player-name characters with spaces and append a terminator.
// FUNCTION: C2 0x70eae
// FUNCTION: C2WIN 0x004838d3
void fix_plyr_name(unsigned char *name_buf)
{
    int i;
    for (i = 0; i < 0x18; i++) {
        if (name_buf[i] < 0x20)
            name_buf[i] = 0x20;
        if (name_buf[i] == 0) break;
    }
    name_buf[i] = 0;
}

// Decode a province's 60x60 region map and record its city, huts, and border routes.
// FUNCTION: C2 0x70ed8
// FUNCTION: C2WIN 0x0048393f
void load_region_map(int province_idx)
{
    int size;
    int map_y;
    int x;
    int i;
    int j;
    unsigned char image;

    map_direction = 0;

    size = 0xe10;
    readfile("regions.dat", ((void *)scratch_buffer), size, province_idx * size);
    clear_huts();
    cm_dptr = 0;

    for (map_y = 0; map_y < 60; map_y++) {
        for (x = 0; x < 60; x++, cm_dptr++) {
            image = *(((char *)scratch_buffer) + cm_dptr);

            if (image >= 0x7d && image < 0x85) {
                put_rm_area(x, map_y, 1, image, 4, image - 0x7d, 0x10);
            } else if (image >= 0x85 && image < 0x8d) {
                put_rm_area(x, map_y, 2, image, 4, (image - 0x85) * 4 + 8, 0x10);
            } else if (image >= 0x8d && image < 0x91) {
                put_rm_area(x, map_y, 3, image, 4,
                            (image - 0x8d) * 9 + 0x28, 0x10);
            } else if (image == 0x91) {
                put_rm_area(x, map_y, 4, image, 4, 0x4c, 0x10);
            } else if (image >= 0x20 && image < 0x7c) {
                put_rm_area(x, map_y, 1, image, 0, image, 0x18);
            } else if (image == 0x92) {
                put_rm_area(x, map_y, 2, image, 8, 0, 1);
                reg_city_x = x;
                reg_city_y = map_y;
                reg_city_ptr = (map_y * REGION_W + x) * REGION_CELL_BYTES;
                region_pm_x = region_pm_y = -1;
            } else if (image == 0x93) {
                put_rm_area(x, map_y, 1, image, 0, 0x2e, 1);
                put_a_hut(x, map_y, 2);
            } else if (image == 0x94) {
                put_rm_area(x, map_y, 1, image, 0, 0x2f, 1);
                put_a_hut(x, map_y, 3);
            } else if (image == 0x95) {
                put_rm_area(x, map_y, 1, image, 0, 0x30, 1);
                put_a_hut(x, map_y, 4);
            } else if (image == 0x96) {
                put_rm_area(x, map_y, 1, image, 0, 0x31, 1);
                put_a_hut(x, map_y, 5);
            } else if (image == 0x97) {
                put_rm_area(x, map_y, 1, image, 0, 0x32, 1);
                put_a_hut(x, map_y, 1);
            } else if (image == 0x98) {
                image += get_border_position(x, map_y, 0);
                put_rm_area(x, map_y, 1, image, 0, 0x50, 1);
            } else if (image == 0x9c) {
                image += get_border_position(x, map_y, 1);
                put_rm_area(x, map_y, 1, image, 0,
                            image - 0x72, 0x18);
            } else if (image >= 0x18 && image <= 0x1b) {
                put_rm_area(x, map_y, 1, image, 0, image, 0x40);
            } else if (image >= 0x1c && image <= 0x1f) {
                put_rm_area(x, map_y, 1, image, 0, image, 0x80);
            } else {
                put_rm_area(x, map_y, 1, image, 0, image, 0);
            }
        }
    }
}

// Clear the 4-entry hut list (3 bytes per entry).
// FUNCTION: C2 0x711a6
// FUNCTION: C2WIN 0x00483e56
void clear_huts(void)
{
    int i;
    for (i = 0; i < 4; i++) {
        hut_list[i].x    = 0;
        hut_list[i].y    = 0;
        hut_list[i].kind = 0;
    }
}

// Store a hut's position and kind in the first empty hut-list entry.
// FUNCTION: C2 0x711ce
// FUNCTION: C2WIN 0x00483ea3
void put_a_hut(int hut_x, int hut_y, int hut_kind)
{
    int i;
    for (i = 0; i < 4; i++) {
        if (hut_list[i].x == 0) {
            hut_list[i].x    = hut_x;
            hut_list[i].y    = hut_y;
            hut_list[i].kind = hut_kind;
            return;
        }
    }
}

// Record a region-map border position and return its compass direction.
// FUNCTION: C2 0x71216
// FUNCTION: C2WIN 0x00483f0f
int get_border_position(int border_x, int border_y, int trader_flag)
{
    if (border_x == 0) {
        west_border_x = border_x;
        west_border_y = border_y;
        west_trader_is = trader_flag;
        return 3;
    }
    if (border_x >= 59) {
        east_border_x = border_x;
        east_border_y = border_y;
        east_trader_is = trader_flag;
        return 1;
    }
    if (border_y == 0) {
        north_border_x = border_x;
        north_border_y = border_y;
        north_trader_is = trader_flag;
        return 0;
    }
    if (border_y >= 59) {
        south_border_x = border_x;
        south_border_y = border_y;
        south_trader_is = trader_flag;
        return 2;
    }
    test_beeps();
    return 0;
}

// Always report that key input is allowed.
// FUNCTION: C2 0x71247
// FUNCTION: C2WIN 0x0048423b
int allowed_keys(void)
{
    return 1;
}

// Report that the mouse recorder is inactive.
// FUNCTION: C2 0x71286 FOLDED
// FUNCTION: C2WIN 0x004841eb REORDERED
int mouse_recorder(void)
{
    return 0;
}

// Report that the game state is synchronized.
// FUNCTION: C2 0x71286 FOLDED
// FUNCTION: C2WIN 0x00484213
int out_of_sync(void)
{
    return 0;
}

// Create an empty 200-entry history.dat file and reset its ring-buffer state.
// FUNCTION: C2 0x71289
// FUNCTION: C2WIN 0x00483fd3 REORDERED
void setup_history_data(void)
{
    int history_fd;
    int i;

    for (i = 0; i < 5; i++)
        history_entry[i] = 0;
    history_fd = open("history.dat", O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0x180);
    if (history_fd == -1) return;
    for (i = 0; i < 200; ++i) {
        write(history_fd, history_entry, 0x14);
    }
    close(history_fd);
    history_start_ptr = 0;
    history_end_ptr = 0;
    history_entries = 0;
}

// Write the current five-value history entry at the next slot in the 200-entry ring file.
// FUNCTION: C2 0x712f8
// FUNCTION: C2WIN 0x00484094
void save_history(void)
{
    int history_fd;
    int file_offset;
    int seek_result;
    int write_result;

    file_offset = history_end_ptr * 20;
    history_fd = open("history.dat", O_WRONLY | O_CREAT | O_BINARY, 0x180);
    if (history_fd == -1) return;
    seek_result = _lseek(history_fd, file_offset, 0);
    write(history_fd, history_entry, 0x14);
    close(history_fd);
    history_entries++;
    if (history_entries > 0xc8)
        history_entries = 0xc8;
    history_end_ptr++;
    if (history_end_ptr >= 0xc8)
        history_end_ptr = 0;
}


// Read all 200 five-value history entries into the caller's buffer when history.dat exists.
// FUNCTION: C2 0x7138b
// FUNCTION: C2WIN 0x0048414d
void get_history_in_buffer(int *history_buf)
{
    int history_fd;

    history_fd = open("history.dat", O_BINARY);
    if (history_fd == -1) return;
    read(history_fd, history_buf, 0xfa0);
    close(history_fd);
}

// Return one value from a buffered five-column history entry.
// FUNCTION: C2 0x713be
// FUNCTION: C2WIN 0x004841a0
int get_history_from_buffer(int *history_buf, int row_idx, int col_idx)
{
    int ptr;
    int result;

    ptr = row_idx * 5 + col_idx;
    result = history_buf[ptr];
    return result;
}

// No-op hook for stopping mouse recording.
// FUNCTION: C2 0x713ce FOLDED
// FUNCTION: C2WIN 0x00484230
void stop_mouse_recorder(void)
{
}

// No-op callback for starting demo mode.
// FUNCTION: C2 0x713ce FOLDED
// FUNCTION: C2WIN 0x00484208 REORDERED
void start_demo(void)
{
}

// No-op callback for starting mouse recording.
// FUNCTION: C2 0x713ce FOLDED
// FUNCTION: C2WIN 0x00484225
void start_mouse_recorder(void)
{
}

// No-op callback for entering demo playback mode.
// FUNCTION: C2 0x713ce FOLDED
// FUNCTION: C2WIN 0x004841fd REORDERED
void go_demo_play_mode(void)
{
}

// No-op callback for entering demo build mode.
// FUNCTION: C2 0x713ce FOLDED
// FUNCTION: C2WIN 0x004841ce REORDERED
void go_demo_build_mode(void)
{
}
