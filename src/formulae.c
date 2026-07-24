
#include "c2_data.h"
#include "c2_types.h"

char events[5][64] = {
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0 },
    { 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 4, 0, 0, 0 },
    { 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 3, 0, 0, 0, 0, 0, 3, 0, 0, 4, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 4, 0, 0, 0 },
    { 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 2, 0, 4, 0, 0, 3, 0, 0, 3, 0, 0, 0, 0, 0, 3, 0, 0, 4, 0, 0, 0, 3, 0, 0, 4, 0, 0, 3, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 3, 3, 0, 0, 4, 0, 0, 0 }
};

void stop_db(void);
/* Forward declarations (functions defined later in this file). */
void adjust_peace_criteria(void);
void adjust_culture_criteria(void);
void adjust_proserity_criteria(void);
void adjust_empire_criteria(void);
void assign_to_new_province(void);
void do_promotion(int level);
void make_emperor(void);
void get_morale_and_readiness(void);
void get_current_cohort_totals(void);
void set_current_cohort_totals(void);
void fill_cohort_centuries(void);
void get_army_totals(void);
void collect_pop_tax(void);
void collect_ind_tax(void);
void get_pop_tax_estimate(void);
void get_ind_tax_estimate(void);


// Advances the bankruptcy countdown and ends the game if the treasury stays negative.
// FUNCTION: C2 0x55326
// FUNCTION: C2WIN 0x00454cb0
void check_game_over(void) {
    if (denarii >= 0) {
        months_to_game_over = 0;
        return;
    }
    if (months_to_game_over == 0) {
        put_message(0x62, 0, 0xe);
        months_to_game_over = 24;
        return;
    }
    months_to_game_over--;
    if (months_to_game_over == 12) {
        put_message(0x63, 0, 0xd);
    }
    if (months_to_game_over <= 0) {
        game_state = 1;
    }
}

// Recalculates the governor ratings and offers any promotion the player has earned.
// FUNCTION: C2 0x5539d
// FUNCTION: C2WIN 0x00454d39
void check_for_promotion(void) {
    int promotion_level;
    adjust_culture_criteria();
    adjust_proserity_criteria();
    if (c2inf.peace_mode != 0)
        return;
    adjust_empire_criteria();
    adjust_peace_criteria();
    average_rating = (peace_rating + culture_rating + empire_rating + prosperity_rating) / 4;
    if (refused_promotion != 0) refused_promotion--; else {
        if (promotion_cheat == 1)
            goto promote;
        if (peace_rating < promotion_levels[c2inf.skill_level][completed_provinces]) return;
        if (promotion_levels[c2inf.skill_level][completed_provinces] > culture_rating) return;
        if (empire_rating < promotion_levels[c2inf.skill_level][completed_provinces]) return;
        if (promotion_levels[c2inf.skill_level][completed_provinces] > prosperity_rating) return;
        if (average_rating < promotion_av_levels[c2inf.skill_level][completed_provinces]) return;
promote:
        promotion_cheat = 0;
        promotion_level = province_completion_to_promotion[c2inf.skill_level][completed_provinces];
        empire_won[province_is] = years_elapsed_in_region + 1;
        if (want_promotion(promotion_level) == 0)
            return;
        completed_provinces++;
        if (completed_provinces > 19) completed_provinces = 19;
        if (promotion_level == 0) assign_to_new_province();
        else do_promotion(promotion_level);
    }
}

// Recomputes the peace rating and applies its population cap.
// FUNCTION: C2 0x554f3
// FUNCTION: C2WIN 0x00454f27
void adjust_peace_criteria(void) {
    int value;
    pax_romanum += 2;
    if (pax_romanum > 1000) pax_romanum = 1000;
    if (pax_romanum < 0)    pax_romanum = 0;
    peace_rating = pax_romanum / 10;
    value = peace_rating;
    peace_rating = city_pop_limit_10_to_1(peace_rating, 1);
    peace_rating_pop_limit = (value > peace_rating);
    if (population < 10) peace_rating_pop_limit = 0;
}

// Recomputes culture from entertainment, religion, and public-service coverage.
// FUNCTION: C2 0x55573
// FUNCTION: C2WIN 0x00454fdb
void adjust_culture_criteria(void) {
    int population_divisor;
    int uncapped_rating;
    entertainment_level  = theatre_culture_count * 5;
    entertainment_level += odium_culture_count * 8;
    entertainment_level += arena_culture_count * 12;
    entertainment_level += colosseum_culture_count * 16;
    entertainment_level += circus_culture_count * 20;
    entertainment_level += circus_maximus_culture_count * 25;
    entertainment_level *= 100;
    religion_level  = large_temples_culture_count * 12;
    religion_level += med_temples_culture_count * 7;
    religion_level += small_temples_culture_count * 2;
    religion_level *= 100;
    utility_level  = (plaza_culture_count + gardens_culture_count) / 2;
    utility_level += grammaticus_culture_count * 4;
    utility_level += rhetor_culture_count * 7;
    utility_level += accessed_hospitals_count * 10;
    utility_level += accessed_libraries_count * 20;
    utility_level *= 100;
    population_divisor = population / 16 + 2;
    entertainment_level /= population_divisor;
    religion_level /= population_divisor;
    utility_level /= population_divisor;
    if (entertainment_level > 100) entertainment_level = 100;
    if (religion_level > 100) religion_level = 100;
    if (utility_level > 100) utility_level = 100;
    culture_rating = (entertainment_level + religion_level + utility_level) / 3;
    uncapped_rating = culture_rating;
    culture_rating = city_pop_limit_10_to_1(culture_rating, 3);
    culture_rating_pop_limit = (uncapped_rating > culture_rating);
    if (population < 10) culture_rating_pop_limit = 0;
}

// Recomputes prosperity from GDP, population, and recent profit.
// FUNCTION: C2 0x557af
// FUNCTION: C2WIN 0x00455201
void adjust_proserity_criteria(void) {
    int population_rating;
    int uncapped_rating;
    current_gdp = average_pop_tax_denariis * 100 + average_pop_tax_asses;
    current_gdp /= 4;
    if (current_gdp > 60) current_gdp = 60;
    population_rating = population;
    if (population_rating > 2000) population_rating = 2000;
    population_rating /= 60;
    if (month == 11) rolling_profit += account_total;
    if (rolling_profit < -5000) rolling_profit = -5000;
    if (rolling_profit > 5000) rolling_profit = 5000;
    prosperity_rating = current_gdp + population_rating + rolling_profit / 200;
    uncapped_rating = prosperity_rating;
    prosperity_rating = city_pop_limit_10_to_1(prosperity_rating, 4);
    prosperity_rating_pop_limit = (uncapped_rating > prosperity_rating);
    if (population < 10) prosperity_rating_pop_limit = 0;
}

// Recomputes the empire rating from favour, trade links, and provincial infrastructure.
// FUNCTION: C2 0x558a0
// FUNCTION: C2WIN 0x00455342
void adjust_empire_criteria(void) {
    int uncapped_rating;
    empire_rating  = (imperial_favour - 80) / 10;
    empire_rating += no_of_empire_connections * 15;
    empire_rating += no_of_connected_towns * 5;
    empire_rating += (no_of_workcamps + no_of_warehouses) / 2;
    empire_rating += no_of_shipyards * 2;
    empire_rating += no_of_farms;
    empire_rating += no_of_mines;
    empire_rating += no_of_quarrys;
    empire_rating += no_of_trading_posts * 2;
    uncapped_rating = empire_rating;
    empire_rating = city_pop_limit_10_to_1(empire_rating, 1);
    empire_rating_pop_limit = (uncapped_rating > empire_rating);
    if (population < 10) empire_rating_pop_limit = 0;
}

// Caps a rating according to the city's current population tier.
// FUNCTION: C2 0x55992
// FUNCTION: C2WIN 0x00455432
int city_pop_limit_10_to_1(int rating, int population_factor) {
    int rating_limit;
    if (rating < 0) rating = 0;
    if (rating > 100) rating = 100;
    for (rating_limit = 0; rating_limit < 100; rating_limit++) {
        if (rating_limit * 10 * population_factor >= population) {
            if (rating > rating_limit) rating = rating_limit;
            return rating;
        }
    }
    return rating;
}

// Asks whether the player accepts an offered promotion.
#if C2_FEAT_MODAL_PROMOTION
/* Windows-port promotion dialog: the box call is modal and returns the
   player's choice, and any advisor windows the player had open are
   brought back afterwards. */
void act_take_promotion(void);
void act_review_in_10(void);
void act_review_in_25(void);
extern unsigned char advisor_window_up[3];
void show_advisor_window(char advisor_idx);
void redraw_city_window(void);
void redraw_region_window(void);

// Asks whether the player accepts an offered promotion.
// FUNCTION: C2WIN 0x004554ba
int want_promotion(int promotion_level)
{
    int result;

    pointer_mode = 0;
    decision = 0;
    result = show_want_promotion_box(player_rank + promotion_level);
    if (result == 1) {
        act_take_promotion();
    } else if (result == 2) {
        act_review_in_10();
        if (advisor_window_up[0] == 1)
            show_advisor_window(0);
        if (advisor_window_up[1] == 1)
            show_advisor_window(1);
        if (advisor_window_up[2] == 1)
            show_advisor_window(2);
    } else if (result == 3) {
        act_review_in_25();
        if (advisor_window_up[0] == 1)
            show_advisor_window(0);
        if (advisor_window_up[1] == 1)
            show_advisor_window(1);
        if (advisor_window_up[2] == 1)
            show_advisor_window(2);
    }
    if (player_rank + promotion_level >= 10 && decision == 1) {
        if (c2inf.skill_level < 2) {
            make_emperor();
            return 0;
        }
        confirm(13, 160, 160);
        if (decision == 0) {
            make_emperor();
            return 0;
        }
    }
    if (decision == 1) {
        stop_db();
        return 1;
    }
    load_map_graphics(map_mode, zoom_level);
    if (map_mode == 0) {
        city_map_screen(1);
    } else if (map_mode == 1) {
        region_map_screen(1);
    }
    if (result == 2 || result == 3) {
        if (map_mode == 0) {
            redraw_city_window();
        } else if (map_mode == 1) {
            redraw_region_window();
        }
    }
    flush_sb_buffer();
    stop_db();
    return 0;
}
#else
// Asks whether the player accepts an offered promotion.
// FUNCTION: C2 0x559d5
int want_promotion(int promotion_level) {
    int promoted_rank;
    pointer_mode = 0;
    show_want_promotion_box(player_rank + promotion_level);
    clear_mouse();
    out2 = 0;
    decision = 0;
    while (out2 != 1) {
        promotion_game_loop();
    }
    flush_sb_buffer();
    stop_db();
    promoted_rank = promotion_level + player_rank;
    if (promoted_rank >= 10 && decision == 1) {
        if (c2inf.skill_level < 2) {
            make_emperor();
            return 0;
        }
        confirm(13, 160, 160);
        if (decision == 0) {
            make_emperor();
            return 0;
        }
    }
    if (decision == 1) {
        stop_db();
        return 1;
    }
    load_map_graphics(map_mode, zoom_level);
    if (map_mode == 0) {
        city_map_screen(1);
    } else if (map_mode == 1) {
        region_map_screen(1);
    }
    flush_sb_buffer();
    stop_db();
    return 0;
}
#endif


// Accepts the pending promotion and closes the promotion dialog.
// FUNCTION: C2 0x55ac7
// FUNCTION: C2WIN 0x004556eb
void act_take_promotion(void) {
    decision = 1;
    out2 = 1;
}

// Defers the pending promotion for ten months.
// FUNCTION: C2 0x55ad9
// FUNCTION: C2WIN 0x00455707
void act_review_in_10(void) {
    decision = 0;
    out2 = 1;
    refused_promotion = 120;
}

// Defers the pending promotion for twenty-five months.
// FUNCTION: C2 0x55af6
// FUNCTION: C2WIN 0x0045572d
void act_review_in_25(void) {
    decision = 0;
    out2 = 1;
    refused_promotion = 300;
}

// Switches the game to province selection after the current assignment ends.
// FUNCTION: C2 0x55b13
// FUNCTION: C2WIN 0x00455753
void assign_to_new_province(void) {
    game_state = 3;
}

// Applies a rank promotion and returns to province selection.
// FUNCTION: C2 0x55b1e
// FUNCTION: C2WIN 0x00455768
void do_promotion(int level) {
    game_state = 3;
    if (player_rank >= 10)
        return;
    if (level + player_rank > 10)
        return;
    player_rank = level + player_rank;
}

// Promotes the player to emperor and triggers the victory sequence.
// FUNCTION: C2 0x55b42
// FUNCTION: C2WIN 0x004557af
void make_emperor(void) {
    black_out();
    game_state = 2;
}

// Resets legion staffing, reinforcement, morale, and readiness state.
// FUNCTION: C2 0x55b52
// FUNCTION: C2WIN 0x004557c9
void init_legion(void) {
    army_wage_level = 0;
    conscription_rate = 2;
    mercs_in_army = 0;
    total_no_of_cohorts = 0;
    total_no_of_soldiers = 0;
    total_no_of_regulars = total_no_of_irregulars = total_no_of_auxillaries = 0;
    current_no_of_soldiers = 0;
    current_no_of_regulars = current_no_of_irregulars = current_no_of_auxillaries = 0;
    needed_no_of_soldiers = 0;
    needed_no_of_regulars = needed_no_of_irregulars = needed_no_of_auxillaries = 0;
    total_no_of_specials = current_no_of_specials = needed_no_of_specials;
    lacking_auxillaries = extra_auxillaries = 0;
    lacking_irregulars = extra_irregulars = 0;
    lacking_regulars = extra_regulars = 0;
    lacking_specials = extra_specials = 0;
    average_cohort_morale = average_cohort_readiness = 0;
    get_cohorts_in_action();
}

// Reconciles cohort staffing with available recruits and charges the army's monthly costs.
// FUNCTION: C2 0x55c0b
// FUNCTION: C2WIN 0x004558f1
void train_soldiers(void) {
    if (c2inf.peace_mode != 0) return;
    get_cohorts_in_action();
    get_current_cohort_totals();
    get_army_totals();
    set_current_cohort_totals();
    fill_cohort_centuries();
    get_morale_and_readiness();
    current_operating_cost += army_wage_level;
    current_operating_cost += (mercs_in_army / 50) * mercs_cost_per_50;
    denarii -= army_wage_level;
    denarii -= (mercs_in_army / 50) * mercs_cost_per_50;
}

// Recalculates readiness for each active cohort and updates the army-wide averages.
// FUNCTION: C2 0x55c82
// FUNCTION: C2WIN 0x00455983
void get_morale_and_readiness(void) {
    int n;
    int readiness_sum;
    int morale_sum;
    morale_sum = readiness_sum = n = 0;
    for (temp_army = 1; temp_army < 26; temp_army++) {
        if (army_list[temp_army].exists != 0 && army_list[temp_army].type == 1)
        {
            n++;
            morale_sum += army_list[temp_army].morale;
            if (army_list[temp_army].total_troops < 100)
                army_list[temp_army].readiness_level = 0;
            else if (army_list[temp_army].total_troops < 250)
                army_list[temp_army].readiness_level = 1;
            else if (army_list[temp_army].total_troops < 500)
                army_list[temp_army].readiness_level = 2;
            else if (army_list[temp_army].total_troops < 1000)
                army_list[temp_army].readiness_level = 3;
            else
                army_list[temp_army].readiness_level = 4;
            readiness_sum += army_list[temp_army].readiness_level;
        }
    }
    average_cohort_morale = 0;
    average_cohort_readiness = 0;
    if (n != 0) {
        average_cohort_morale = morale_sum / n;
        average_cohort_readiness = readiness_sum / n;
    }
}

// Totals each troop type currently assigned to active cohorts.
// FUNCTION: C2 0x55d79
// FUNCTION: C2WIN 0x00455b9a
void get_current_cohort_totals(void) {
    current_no_of_auxillaries = current_no_of_irregulars =
        current_no_of_regulars = current_no_of_specials = 0;
    for (temp_army = 1; temp_army < 26; temp_army++) {
        if (army_list[temp_army].exists != 0 && army_list[temp_army].type == 1)
        {
        current_no_of_auxillaries += army_list[temp_army].num_auxillaries;
        current_no_of_irregulars += army_list[temp_army].num_irregulars;
        current_no_of_regulars   += army_list[temp_army].num_regulars;
        current_no_of_specials   += army_list[temp_army].num_specials;
        }
    }
    current_no_of_soldiers = current_no_of_regulars + current_no_of_irregulars
                           + current_no_of_auxillaries + current_no_of_specials;
}

// Removes troop shortfalls, distributes reinforcements, and refreshes active cohort totals.
// FUNCTION: C2 0x55e1d
// FUNCTION: C2WIN 0x00455cce
void set_current_cohort_totals(void) {
    int loop_counter;
    int p;
    int men_per_pass;
    int no_of_specials;
    int needed_regulars;
    int aux_needed;
    int the_irregulars;

    if (no_of_cohorts_in_action <= 0) {
        current_no_of_soldiers = 0;
        current_no_of_regulars = current_no_of_irregulars = current_no_of_auxillaries = 0;
        return;
    }

    loop_counter = 0;
    temp_army = last_adjusted_cohort;

    // Reconcile auxiliaries.
    while (lacking_auxillaries > 0 && ++loop_counter < 40000) {
        if (get_next_temp_cohort(0) == 0)
            break;
        if (army_list[temp_army].num_auxillaries != 0) {
            army_list[temp_army].num_auxillaries--;
            lacking_auxillaries--;
        }
    }
    while (extra_auxillaries > 0 && ++loop_counter < 40000) {
        if (get_next_temp_cohort(1) == 0)
            break;
        if (army_list[temp_army].cohort_size_class == 0)      men_per_pass = 2;
        else if (army_list[temp_army].cohort_size_class == 2) men_per_pass = 4;
        else                                                  men_per_pass = 1;
        for (p = 0; p < men_per_pass; p++) {
            if (extra_auxillaries != 0) {
                army_list[temp_army].num_auxillaries++;
                extra_auxillaries--;
            } else
                break;
        }
    }

    // Reconcile irregulars.
    while (lacking_irregulars > 0 && ++loop_counter < 40000) {
        if (get_next_temp_cohort(0) == 0)
            break;
        if (army_list[temp_army].num_irregulars != 0) {
            army_list[temp_army].num_irregulars--;
            lacking_irregulars--;
        }
    }
    while (extra_irregulars > 0 && ++loop_counter < 40000) {
        if (get_next_temp_cohort(1) == 0)
            break;
        if (army_list[temp_army].cohort_size_class == 0)      men_per_pass = 2;
        else if (army_list[temp_army].cohort_size_class == 2) men_per_pass = 4;
        else                                                  men_per_pass = 1;
        for (p = 0; p < men_per_pass; p++) {
            if (extra_irregulars != 0) {
                army_list[temp_army].num_irregulars++;
                extra_irregulars--;
            } else
                break;
        }
    }

    // Reconcile regulars.
    while (lacking_regulars > 0 && ++loop_counter < 40000) {
        if (get_next_temp_cohort(0) == 0)
            break;
        if (army_list[temp_army].num_regulars != 0) {
            army_list[temp_army].num_regulars--;
            lacking_regulars--;
        }
    }
    while (extra_regulars > 0 && ++loop_counter < 40000) {
        if (get_next_temp_cohort(1) == 0)
            break;
        if (army_list[temp_army].cohort_size_class == 0)      men_per_pass = 2;
        else if (army_list[temp_army].cohort_size_class == 2) men_per_pass = 4;
        else                                                  men_per_pass = 1;
        for (p = 0; p < men_per_pass; p++) {
            if (extra_regulars != 0) {
                army_list[temp_army].num_regulars++;
                extra_regulars--;
            } else
                break;
        }
    }

    // Reconcile special troops.
    while (lacking_specials > 0 && ++loop_counter < 40000) {
        if (get_next_temp_cohort(0) == 0)
            break;
        if (army_list[temp_army].num_specials != 0) {
            army_list[temp_army].num_specials--;
            lacking_specials--;
        }
    }
    while (extra_specials > 0 && ++loop_counter < 40000) {
        if (get_next_temp_cohort(1) == 0)
            break;
        if (army_list[temp_army].cohort_size_class == 0)      men_per_pass = 2;
        else if (army_list[temp_army].cohort_size_class == 2) men_per_pass = 4;
        else                                                  men_per_pass = 1;
        for (p = 0; p < men_per_pass; p++) {
            if (extra_specials != 0) {
                army_list[temp_army].num_specials++;
                extra_specials--;
            } else
                break;
        }
    }

    last_adjusted_cohort = temp_army;

    // Refresh each active cohort's troop total.
    for (temp_army = 1; temp_army < 26; temp_army++) {
        if (army_list[temp_army].exists != 0 && army_list[temp_army].type == 1) {
            army_list[temp_army].total_troops =
                  army_list[temp_army].num_auxillaries
                + army_list[temp_army].num_irregulars
                + army_list[temp_army].num_regulars
                + army_list[temp_army].num_specials;
            army_list[temp_army].assigned_needs = 0;
        }
    }

    // Distribute remaining reinforcement needs among active cohorts.
    aux_needed = needed_no_of_auxillaries;
    the_irregulars = needed_no_of_irregulars;
    needed_regulars = needed_no_of_regulars;
    no_of_specials = needed_no_of_specials;

    while (aux_needed-- > 0 && ++loop_counter < 40000) {
        if (get_next_temp_cohort(1) == 0)
            break;
        army_list[temp_army].assigned_needs++;
    }
    while (the_irregulars-- > 0 && ++loop_counter < 40000) {
        if (get_next_temp_cohort(1) == 0)
            break;
        army_list[temp_army].assigned_needs++;
    }
    while (needed_regulars-- > 0 && ++loop_counter < 40000) {
        if (get_next_temp_cohort(1) == 0)
            break;
        army_list[temp_army].assigned_needs++;
    }
    while (no_of_specials-- > 0 && ++loop_counter < 40000) {
        if (get_next_temp_cohort(1) == 0)
            break;
        army_list[temp_army].assigned_needs++;
    }
}

// Distributes cohort troops across its century records.
// FUNCTION: C2 0x56322
// FUNCTION: C2WIN 0x00456551
void fill_cohort_centuries(void) {
    int no_auxillaries;
    int no_of_irregulars;
    int regulars;
    int i;
    int specials;

    for (temp_army = 1; temp_army < 26; temp_army++) {
        if (army_list[temp_army].exists != 0 && army_list[temp_army].type == 1) {

        army_list[temp_army].num_centuries = 0;
        no_auxillaries = army_list[temp_army].num_auxillaries;
        no_of_irregulars = army_list[temp_army].num_irregulars;
        regulars = army_list[temp_army].num_regulars;
        specials = army_list[temp_army].num_specials;
        army_list[temp_army].total_troops =
              army_list[temp_army].num_auxillaries
            + army_list[temp_army].num_irregulars
            + army_list[temp_army].num_regulars
            + army_list[temp_army].num_specials;

        // Pass 1: re-stock existing centuries by type.
        for (i = 0; i < 14; i++) {
            if (army_list[temp_army].centuries[i].type == 1) {                       // regulars
                if (regulars <= 0) {
                    army_list[temp_army].centuries[i].type = 0;
                } else if (regulars >= 60) {
                    regulars -= 60;
                    if (army_list[temp_army].centuries[i].damaged == 0)
                        army_list[temp_army].centuries[i].damaged = 1;
                } else {
                    army_list[temp_army].centuries[i].damaged = 0;
                    regulars = 0;
                }
            } else if (army_list[temp_army].centuries[i].type == 2) {                // no_of_irregulars
                if (no_of_irregulars <= 0) {
                    army_list[temp_army].centuries[i].type = 0;
                } else if (no_of_irregulars >= 60) {
                    no_of_irregulars -= 60;
                    if (army_list[temp_army].centuries[i].damaged == 0)
                        army_list[temp_army].centuries[i].damaged = 1;
                } else {
                    army_list[temp_army].centuries[i].damaged = 0;
                    no_of_irregulars = 0;
                }
            } else if (army_list[temp_army].centuries[i].type == 3) {                // no_auxillaries
                if (no_auxillaries <= 0) {
                    army_list[temp_army].centuries[i].type = 0;
                } else if (no_auxillaries >= 60) {
                    no_auxillaries -= 60;
                    if (army_list[temp_army].centuries[i].damaged == 0)
                        army_list[temp_army].centuries[i].damaged = 1;
                } else {
                    army_list[temp_army].centuries[i].damaged = 0;
                    no_auxillaries = 0;
                }
            } else if (army_list[temp_army].centuries[i].type == 4) {                // specials
                if (specials <= 0) {
                    army_list[temp_army].centuries[i].type = 0;
                } else if (specials >= 60) {
                    specials -= 60;
                    if (army_list[temp_army].centuries[i].damaged == 0)
                        army_list[temp_army].centuries[i].damaged = 1;
                } else {
                    army_list[temp_army].centuries[i].damaged = 0;
                    specials = 0;
                }
            }
        }

        // Pass 2: fill empty centuries from leftovers.
        for (i = 0; i < 14; i++) {
            if (army_list[temp_army].centuries[i].type == 0) {
            if (regulars >= 60) {
                regulars -= 60;
                army_list[temp_army].centuries[i].type = 1;
                army_list[temp_army].centuries[i].damaged = 1;
            } else if (no_of_irregulars >= 60) {
                no_of_irregulars -= 60;
                army_list[temp_army].centuries[i].type = 2;
                army_list[temp_army].centuries[i].damaged = 1;
            } else if (no_auxillaries >= 60) {
                no_auxillaries -= 60;
                army_list[temp_army].centuries[i].type = 3;
                army_list[temp_army].centuries[i].damaged = 1;
            } else if (specials >= 60) {
                specials -= 60;
                army_list[temp_army].centuries[i].type = 4;
                army_list[temp_army].centuries[i].damaged = 1;
            } else if (regulars != 0) {
                regulars = 0;
                army_list[temp_army].centuries[i].type = 1;
                army_list[temp_army].centuries[i].damaged = 0;
            } else if (no_of_irregulars != 0) {
                no_of_irregulars = 0;
                army_list[temp_army].centuries[i].type = 2;
                army_list[temp_army].centuries[i].damaged = 0;
            } else if (no_auxillaries != 0) {
                no_auxillaries = 0;
                army_list[temp_army].centuries[i].type = 3;
                army_list[temp_army].centuries[i].damaged = 0;
            } else if (specials != 0) {
                specials = 0;
                army_list[temp_army].centuries[i].type = 4;
                army_list[temp_army].centuries[i].damaged = 0;
            }
            }
        }

        // Pass 3: count non-empty centuries.
        for (i = 0; i < 14; i++) {
            if (army_list[temp_army].centuries[i].type != 0)
                army_list[temp_army].num_centuries++;
        }
        }
    }
}

// Computes available troop totals, recruitment changes, and remaining army needs.
// FUNCTION: C2 0x5654e
// FUNCTION: C2WIN 0x00456d85
void get_army_totals(void) {
    lacking_auxillaries = lacking_irregulars =
        lacking_regulars = lacking_specials = 0;
    extra_auxillaries = extra_irregulars =
        extra_regulars = extra_specials = 0;

    total_no_of_auxillaries = slave_requirements[6].current;
    total_no_of_irregulars = totalXpercent(population, conscription_rate);
    total_no_of_regulars = army_wage_level * (main_paras[1] + 1 - province_difficulty);
    total_no_of_specials = mercs_in_army;

    if (total_no_of_auxillaries < current_no_of_auxillaries) lacking_auxillaries = current_no_of_auxillaries - total_no_of_auxillaries;
    else if (current_no_of_auxillaries + main_paras[2] <= total_no_of_auxillaries) extra_auxillaries = main_paras[2];
    else extra_auxillaries = total_no_of_auxillaries - current_no_of_auxillaries;
    current_no_of_auxillaries = current_no_of_auxillaries - lacking_auxillaries;
    current_no_of_auxillaries = current_no_of_auxillaries + extra_auxillaries;

    if (total_no_of_irregulars < current_no_of_irregulars) lacking_irregulars = current_no_of_irregulars - total_no_of_irregulars;
    else if (current_no_of_irregulars + main_paras[3] <= total_no_of_irregulars) extra_irregulars = main_paras[3];
    else extra_irregulars = total_no_of_irregulars - current_no_of_irregulars;
    current_no_of_irregulars = current_no_of_irregulars - lacking_irregulars;
    current_no_of_irregulars = current_no_of_irregulars + extra_irregulars;

    if (total_no_of_regulars < current_no_of_regulars) lacking_regulars = current_no_of_regulars - total_no_of_regulars;
    else if (current_no_of_regulars + main_paras[4] <= total_no_of_regulars) extra_regulars = main_paras[4];
    else extra_regulars = total_no_of_regulars - current_no_of_regulars;
    current_no_of_regulars = current_no_of_regulars - lacking_regulars;
    current_no_of_regulars = current_no_of_regulars + extra_regulars;

    if (total_no_of_specials < current_no_of_specials) lacking_specials = current_no_of_specials - total_no_of_specials;
    else if (current_no_of_specials + 1000 <= total_no_of_specials) extra_specials = 1000;
    else extra_specials = total_no_of_specials - current_no_of_specials;
    current_no_of_specials = current_no_of_specials - lacking_specials;
    current_no_of_specials = current_no_of_specials + extra_specials;

    needed_no_of_auxillaries = total_no_of_auxillaries - current_no_of_auxillaries;
    needed_no_of_irregulars = total_no_of_irregulars - current_no_of_irregulars;
    needed_no_of_regulars = total_no_of_regulars - current_no_of_regulars;
    needed_no_of_specials = total_no_of_specials - current_no_of_specials;

    total_no_of_soldiers = total_no_of_regulars + total_no_of_irregulars + total_no_of_auxillaries + total_no_of_specials;
    needed_no_of_soldiers = needed_no_of_regulars + needed_no_of_irregulars + needed_no_of_auxillaries + needed_no_of_specials;
    current_no_of_soldiers = current_no_of_regulars + current_no_of_irregulars + current_no_of_auxillaries + current_no_of_specials;
}

// Predict total and still-needed troop counts from the province's current army parameters.
// FUNCTION: C2 0x567fc
// FUNCTION: C2WIN 0x00457094
void predict_army_totals(void) {
    get_current_cohort_totals();

    total_no_of_auxillaries = slave_requirements[6].current;
    total_no_of_irregulars  = totalXpercent(population, conscription_rate);
    total_no_of_regulars    = army_wage_level * (main_paras[1] + 1 - province_difficulty);
    total_no_of_specials    = mercs_in_army;

    needed_no_of_auxillaries = total_no_of_auxillaries - current_no_of_auxillaries;
    needed_no_of_irregulars  = total_no_of_irregulars  - current_no_of_irregulars;
    needed_no_of_regulars    = total_no_of_regulars    - current_no_of_regulars;
    needed_no_of_specials    = total_no_of_specials    - current_no_of_specials;

    if (needed_no_of_auxillaries < 0) {
        needed_no_of_auxillaries = 0;
        total_no_of_auxillaries  = current_no_of_auxillaries;
    }
    if (needed_no_of_irregulars < 0) {
        needed_no_of_irregulars = 0;
        total_no_of_irregulars  = current_no_of_irregulars;
    }
    if (needed_no_of_regulars < 0) {
        needed_no_of_regulars = 0;
        total_no_of_regulars  = current_no_of_regulars;
    }
    if (needed_no_of_specials < 0) {
        needed_no_of_specials = 0;
        total_no_of_specials  = current_no_of_specials;
    }

    total_no_of_soldiers  = total_no_of_regulars + total_no_of_irregulars
                          + total_no_of_auxillaries + total_no_of_specials;
    needed_no_of_soldiers = needed_no_of_regulars + needed_no_of_irregulars
                          + needed_no_of_auxillaries + needed_no_of_specials;
}

// Initialize the slave economy and workforce requirements for a new province.
// FUNCTION: C2 0x56943
// FUNCTION: C2WIN 0x004571e5
void init_slaves(void) {
    slave_welfare_bill = init_slave_data[province_difficulty - 1].welfare_bill;
    slaves             = init_slave_data[province_difficulty - 1].slaves;

    slave_requirements[0].max     = 0x14;
    slave_requirements[1].current = 0x0C;
    slave_requirements[2].current = 4;
    slave_requirements[3].current = 4;
    slave_requirements[4].current = 0;
    slave_requirements[5].current = 0;
    slave_requirements[6].current = 0;
}

// Advance the slave population by one welfare-driven growth and mortality tick.
// FUNCTION: C2 0x569a1
// FUNCTION: C2WIN 0x00457258
void slave_welfare(void) {
    int start_slaves = slaves;
    int welfare    = (main_paras[0] - province_difficulty / 3) * slave_welfare_bill;
    int qual     = valueDIVtotal(welfare, slaves);
    int growth_percent;
    int death_rate;
    int gained;
    int death_toll;

    if      (qual <   10) { death_rate = 50; growth_percent =   1; }
    else if (qual <   25) { death_rate = 30; growth_percent =   2; }
    else if (qual <   50) { death_rate = 20; growth_percent =   3; }
    else if (qual <   75) { death_rate = 15; growth_percent =   4; }
    else if (qual <   95) { death_rate =  9; growth_percent =   5; }
    else if (qual > 2000) { death_rate =    2; growth_percent = 200; }
    else if (qual > 1500) { death_rate =    2; growth_percent = 150; }
    else if (qual > 1000) { death_rate =    2; growth_percent = 100; }
    else if (qual >  750) { death_rate =    2; growth_percent =  60; }
    else if (qual >  500) { death_rate =    2; growth_percent =  40; }
    else if (qual >  300) { death_rate =    2; growth_percent =  20; }
    else if (qual >  200) { death_rate =    3; growth_percent =  15; }
    else if (qual >  150) { death_rate =    4; growth_percent =  11; }
    else if (qual >  125) { death_rate =    5; growth_percent =   9; }
    else if (qual >  105) { death_rate =    6; growth_percent =   8; }
    else {
        // A balanced welfare level leaves the slave population unchanged.
        slave_population_change = 0;
        return;
    }

    gained    = totalXpercent(slaves, growth_percent);
    death_toll = totalXpercent(slaves, death_rate);
    slaves += gained + 1;
    slaves -= death_toll;
    if (slaves < 1) slaves = 1;
    slave_population_change = slaves - start_slaves;
}

// Charges the monthly slave welfare bill to the provincial treasury.
// FUNCTION: C2 0x56b5a
// FUNCTION: C2WIN 0x004574e1
void slave_costs(void) {
    denarii -= slave_welfare_bill;
    current_operating_cost += slave_welfare_bill;
}

// Estimate short- and long-term slave populations without changing the live simulation state.
// FUNCTION: C2 0x56b6c
// FUNCTION: C2WIN 0x00457507
void slave_estimate(void) {
    int saved_slaves = slaves;
    int saved_change = slave_population_change;
    int p;

    slave_welfare();
    slave_population_estimate = slaves;

    for (p = 0; p < 100; p++) {
        slave_welfare();
    }
    slave_population_final_estimate = slaves;

    slaves = saved_slaves;
    slave_population_change = saved_change;
}

// Distribute the available slave workforce across requirement buckets in priority order.
// FUNCTION: C2 0x56bb5
// FUNCTION: C2WIN 0x00457571
void adjust_slave_usage(void) {
    int available_slaves = slaves;
    int i;

    slave_requirements[0].current = slave_requirements[0].max;

    for (i = 0; i < 7; i++) {
        if (available_slaves >= slave_requirements[i].current) {
            available_slaves -= slave_requirements[i].current;
        } else {
            slave_requirements[i].current = available_slaves;
            available_slaves = 0;
        }
    }
    slave_requirements[i].current = available_slaves;              /* Unassigned workforce. */
}

// Draw and apply a skill-dependent random province event for the current turn.
#if PLATFORM_WINDOWS
/* The Windows build rewrote this handler: OR-combined quiet-outcome
   gates, reordered temple sums, split argument locals, and a dead
   20000-denarii tier storing 50 where the DOS build stores 20 (both
   stores are shadowed by the 10000 tier, so behavior is unchanged). */
// FUNCTION: C2WIN 0x004575f9
void random_event(void) {
    int event;
    int modifier;
    int cash;
    int temples;
    int chance;
    int value;
    int average;

    event = (unsigned char)events[c2inf.skill_level][rand128 & 63];
    plague_accident = 999999;
    revolt_accident = 999999;

    if (event == 0) {
        if (denarii < 1000 || population < 100)
            return;
        temples = med_temples_count * 3
                     + large_temples_count * 4
                     + small_temples_count;
        if (temples == 0) temples = 1;
        cash = denarii / temples;
        if (cash >= 20000) modifier = 50;
        if      (cash >= 10000) modifier = 20;
        else if (cash >=  4000) modifier = 14;
        else if (cash >=  2000) modifier =  8;
        else if (cash >=  1000) modifier =  4;
        else if (cash >=   500) modifier =  0;
        else                                  modifier = -4;
        modifier += c2inf.skill_level;
        if (rand128 >= modifier)
            return;
        robbery_count = 1;
        event = 3;
    }

    if (event == 4) {
        if (plague_running_count < 4)
            return;
        plague_accident = get_rand_max(plague_running_count);
    }

    if (event == 2) {
        if (denarii < 1000 || population < 100)
            return;
        if (temples_count == 0) {
            if (!warned_of_robbery) {
                warned_of_robbery = 1;
                put_message(88, 0, 14);
            } else {
                put_message(89, 0, 16);
                stolen_denarii = denarii / 4;
                denarii -= stolen_denarii;
            }
        }
    }

    if (event == 3) {
        if (denarii < 1000 || population < 100)
            return;
        if (temples_count == 0) {
            if (!warned_of_robbery) {
                warned_of_robbery = 1;
                put_message(88, 0, 14);
            } else {
                put_message(89, 0, 16);
                stolen_denarii = denarii / 4;
                denarii -= stolen_denarii;
            }
        } else if (robbery_count != 0) {
            value = large_temples_count * 4
                          + med_temples_count * 2
                          + small_temples_count;
            average = (large_robbery_count * 4
                           + med_robbery_count * 2
                           + small_robbery_count) / robbery_count;
            chance = valueDIVtotal(average, value);
            if (chance < 10) chance = 10;
            if (chance > 80) chance = 80;
            stolen_denarii = totalXpercent(denarii / 4, chance);
            if (stolen_denarii > 0) {
                denarii -= stolen_denarii;
                put_message(86, 0, 16);
            }
        }
    }
}
#else
// Draw and apply a skill-dependent random province event for the current turn.
// FUNCTION: C2 0x56bf6
void random_event(void) {
    int event_kind;
    int temple_score;
    int denarii_per_temple;
    int robbery_bonus;
    int temple_weight;
    int robbery_weight;

    // Draw an event from the current difficulty's event table.
    event_kind = (unsigned char)events[c2inf.skill_level][rand128 & 63];

    plague_accident = 999999;
    revolt_accident = 999999;

    if (event_kind == 0) {
        // Convert a favorable roll into a robbery check when the city is wealthy enough.
        if (denarii < 1000) return;
        if (population < 100) return;
        temple_score = large_temples_count * 4
                     + med_temples_count   * 3
                     + small_temples_count;
        if (temple_score == 0) temple_score = 1;
        denarii_per_temple = denarii / temple_score;
        if (denarii_per_temple >= 20000) robbery_bonus = 20;
        if      (denarii_per_temple >= 10000) robbery_bonus =  20;
        else if (denarii_per_temple >=  4000) robbery_bonus =  14;
        else if (denarii_per_temple >=  2000) robbery_bonus =   8;
        else if (denarii_per_temple >=  1000) robbery_bonus =   4;
        else if (denarii_per_temple >=   500) robbery_bonus =   0;
        else                                  robbery_bonus =  -4;
        if (c2inf.skill_level + robbery_bonus <= rand128) return;
        robbery_count = 1;
        event_kind = 3;
    }

    // Schedule a plague accident.
    if (event_kind == 4) {
        if (plague_running_count < 4) return;
        plague_accident = get_rand_max(plague_running_count);
    }

    // Warn about temple neglect, then take a quarter of the treasury on recurrence.
    if (event_kind == 2) {
        if (denarii < 1000)   return;
        if (population < 100) return;
        if (temples_count == 0) {
            if (!warned_of_robbery) {
                warned_of_robbery = 1;
                put_message(88, 0, 14);
            } else {
                put_message(89, 0, 16);
                stolen_denarii = denarii / 4;
                denarii -= stolen_denarii;
            }
        }
        // Cities with temples continue into the robbery calculation.
    }

    if (event_kind != 3) return;
    if (denarii < 1000)   return;
    if (population < 100) return;
    if (temples_count == 0) {
        if (!warned_of_robbery) {
            warned_of_robbery = 1;
            put_message(88, 0, 14);
        } else {
            put_message(89, 0, 16);
            stolen_denarii = denarii / 4;
            denarii -= stolen_denarii;
        }
        return;
    }
    if (robbery_count == 0) return;
    temple_weight = large_temples_count * 4
             + med_temples_count   * 2
             + small_temples_count;
    robbery_weight = (large_robbery_count * 4
           + med_robbery_count   * 2
           + small_robbery_count) / robbery_count;
    temple_score = valueDIVtotal(robbery_weight, temple_weight);
    if (temple_score < 10)  temple_score = 10;
    if (temple_score > 80)  temple_score = 80;
    stolen_denarii = totalXpercent(denarii / 4, temple_score);
    if (stolen_denarii <= 0) return;
    denarii -= stolen_denarii;
    put_message(86, 0, 16);
}
#endif

// Pays the governor's salary for the current rank.
// FUNCTION: C2 0x56eb8
// FUNCTION: C2WIN 0x00457984
void pay_salary(void) {
    current_operating_cost += players_salary;
    players_denarii += players_salary;
    denarii -= players_salary;
}

// Updates population growth pressure from taxes, employment, difficulty, and tutorial mode.
// FUNCTION: C2 0x56ed0
// FUNCTION: C2WIN 0x004579b5
void get_population_growth_factor(void) {
    pop_growth_future += pop_tax_to_growth_data[pop_tax_rate];
    pop_growth_future += employment_to_pop_growth_factor[employment_rate / 5];
    pop_growth_future -= province_difficulty / 3;
    if (pop_growth_future >  36) pop_growth_future =  36;
    if (pop_growth_future < -36) pop_growth_future = -36;
    if (tutorial_mode != 0) pop_growth_future = 36;
    pop_growth_factor = pop_growth_future / 8;
}

// Updates industry growth pressure from taxation and the current business population.
// FUNCTION: C2 0x56f74
// FUNCTION: C2WIN 0x00457a5d
void get_industry_growth_factor(void) {
    ind_growth_future += ind_tax_to_growth_data[ind_tax_rate];
    if (ind_growth_future >  36) ind_growth_future =  36;
    if (ind_growth_future < -36) ind_growth_future = -36;
    if (business_count == 0) ind_growth_future = 0;
    ind_growth_factor = ind_growth_future / 8;
}

// Converts accumulated tax, difficulty, and conscription unrest into an insurrection factor.
// FUNCTION: C2 0x56fdd
// FUNCTION: C2WIN 0x00457ad2
void get_insurrection_factor(void) {
    insurrection_future += tax_to_revolt_data[pop_tax_rate];
    insurrection_future += (province_difficulty - 4) / 2;
    if (province_difficulty <= 2) insurrection_future -= 1;
    insurrection_future += conscription_to_revolt_data[conscription_rate / 2];
    if      (insurrection_future >= 100) { insurrection_factor = 10; insurrection_future -= 100; }
    else if (insurrection_future >=  90) { insurrection_factor =  9; insurrection_future -=  90; }
    else if (insurrection_future >=  80) { insurrection_factor =  8; insurrection_future -=  80; }
    else if (insurrection_future >=  70) { insurrection_factor =  7; insurrection_future -=  70; }
    else if (insurrection_future >=  60) { insurrection_factor =  6; insurrection_future -=  60; }
    else if (insurrection_future >=  50) { insurrection_factor =  5; insurrection_future -=  50; }
    else if (insurrection_future >=  40) { insurrection_factor =  4; insurrection_future -=  40; }
    else if (insurrection_future >=  30) { insurrection_factor =  3; insurrection_future -=  30; }
    else if (insurrection_future >   20) { insurrection_factor =  2; insurrection_future -=  20; }
    else if (insurrection_future <  -10) { insurrection_factor = -2; insurrection_future +=  10; }
    else                                 { insurrection_factor =  0; }
    if (tutorial_mode != 0) insurrection_factor = -2;
}

// Closes the annual accounts and rolls yearly financial totals forward.
// FUNCTION: C2 0x5717d
// FUNCTION: C2WIN 0x00457cad
void year_end_accounts(void) {
    collect_pop_tax();
    collect_ind_tax();
    account_construction_cost = current_construction_cost;
    account_operating_cost    = current_operating_cost + stolen_denarii;
    denarii      -= tribute;
    account_tribute = tribute;
    current_construction_cost = 0;
    current_operating_cost    = 0;
    stolen_denarii            = 0;
    account_total = (account_pop_tax + account_ind_tax)
                  - account_construction_cost
                  - account_operating_cost
                  - account_tribute;
    if (account_total > 0) months_to_game_over = 0;
}

// Collects population tax and records the assessed and paid amounts.
// FUNCTION: C2 0x57200
// FUNCTION: C2WIN 0x00457d4d
void collect_pop_tax(void) {
    if (pop_tax_counts == 0)
        return;
    account_pop_tax       = pop_tax_running_total / pop_tax_counts;
    account_pop_tax      /= 100;
    denarii              += account_pop_tax;
    pop_tax_running_total = 0;
    pop_tax_counts        = 0;
}

// Collects industry tax and records the assessed and paid amounts.
// FUNCTION: C2 0x5724d
// FUNCTION: C2WIN 0x00457dac
void collect_ind_tax(void) {
    if (ind_tax_counts == 0)
        return;
    account_ind_tax       = ind_tax_running_total / ind_tax_counts;
    account_ind_tax      /= 100;
    denarii              += account_ind_tax;
    ind_tax_running_total = 0;
    ind_tax_counts        = 0;
}

// Projects the current year's income, expenses, tribute, and final balance.
// FUNCTION: C2 0x5729a
// FUNCTION: C2WIN 0x00457e0b
void get_estimates(void) {
    int months_left;
    get_pop_tax_estimate();
    get_ind_tax_estimate();
    estimate_construction_cost  = current_construction_cost;
    months_left = 12 - month;
    estimate_operating_cost     = current_operating_cost + stolen_denarii;
    estimate_operating_cost    += slave_welfare_bill * months_left;
    estimate_operating_cost    += army_wage_level    * months_left;
    estimate_operating_cost    += (mercs_in_army / 50) * mercs_cost_per_50 * months_left;
    estimate_operating_cost    += players_salary     * months_left;
    estimate_tribute            = tribute;
    estimate_total              = (estimate_pop_tax + estimate_ind_tax)
                                - estimate_construction_cost
                                - estimate_operating_cost
                                - tribute;
}

// Estimates annual population-tax income using collected and projected monthly receipts.
// FUNCTION: C2 0x57356
// FUNCTION: C2WIN 0x00457ec2
void get_pop_tax_estimate(void) {
    int projected_tax = 0;
    if (pop_tax_counts < 12) {
        int months_left = 12 - pop_tax_counts;
        projected_tax = totalXpercent(pop_tax_last_count * income_multiple, pop_tax_rate);
        projected_tax *= months_left;
    }
    projected_tax += pop_tax_running_total;
    estimate_pop_tax  = projected_tax / 12;
    estimate_pop_tax /= 100;
}

// Estimates annual industry-tax income using collected and projected monthly receipts.
// FUNCTION: C2 0x573b5
// FUNCTION: C2WIN 0x00457f44
void get_ind_tax_estimate(void) {
    int projected_tax = 0;
    if (ind_tax_counts < 12) {
        int months_left = 12 - ind_tax_counts;
        projected_tax = totalXpercent(ind_tax_last_count * income_multiple, ind_tax_rate);
        projected_tax *= months_left;
    }
    projected_tax += ind_tax_running_total;
    estimate_ind_tax  = projected_tax / 12;
    estimate_ind_tax /= 100;
}

// Computes average pop tax per person in denarii and asses (100 asses = 1 denarius).
// FUNCTION: C2 0x57414
// FUNCTION: C2WIN 0x00457fc6
void get_average_pop_tax(void) {
    int per_person;
    if (population == 0) {
        average_pop_tax_denariis = average_pop_tax_asses = 0;
        return;
    }
    per_person = totalXpercent(pop_tax_last_count * income_multiple, pop_tax_rate);
    per_person = per_person / population;
    average_pop_tax_denariis  = per_person / 100;
    average_pop_tax_asses     = per_person % 100;
}

// Computes average industry tax per business in denarii and asses.
// FUNCTION: C2 0x5747e
// FUNCTION: C2WIN 0x00458046
void get_average_ind_tax(void) {
    int per_business;
    if (business_count == 0) {
        average_ind_tax_denariis = average_ind_tax_asses = 0;
        return;
    }
    per_business = totalXpercent(ind_tax_last_count * income_multiple, ind_tax_rate);
    per_business = per_business / business_count;
    average_ind_tax_denariis  = per_business / 100;
    average_ind_tax_asses     = per_business % 100;
}

// Updates imperial favour, requests, tribute reviews, and personal tax demands.
// FUNCTION: C2 0x574e8
// FUNCTION: C2WIN 0x004580c6
void get_new_tribute(void) {
    int favour_delta;
    int province_count;
    int request_amount;
    last_tribute = tribute;

    favour_delta = rand128 & 7;
    favour_delta -= 3;
    favour_delta -= c2inf.skill_level;
    if      (imperial_favour <  25) favour_delta += 2;
    else if (imperial_favour <  70) favour_delta += 1;
    else if (imperial_favour > 175) favour_delta -= 2;
    else if (imperial_favour > 120) favour_delta -= 1;

    if (population < 50) favour_delta = 0;
    imperial_favour += favour_delta;
    if (imperial_favour <   0) imperial_favour =   0;
    if (imperial_favour > 200) imperial_favour = 200;

    if (player_rank >= 10) {
        imperial_favour  = 200;
        tribute          =   0;
        imperial_request = 100;
        imperial_review  = 100;
        return;
    }

    imperial_request -= 1; if (imperial_request <= 0) {
        if (c2inf.peace_mode != 0) { imperial_request = 100; return; }
        if (max_population < 2000) { imperial_request = 2;
        } else if (imperial_request == 0) {
                imperial_req_goods  = province_industries[rand128 & 3].kind;
                request_amount = c2inf.skill_level + 1 + completed_provinces / 2; request_amount += years_elapsed_in_region / 10; request_amount += rand128 & 1; imperial_req_amount = request_amount;
                put_message(135, 0, 10);
            } else if (imperial_request == -1) { put_message(136, 0, 11);
            } else if (imperial_request == -2) { put_message(137, 0, 14);
            } else {
                put_message(138, 0, 13);
            }
    }

    imperial_review -= 1; if (imperial_review <= 0) {
        imperial_review = (rand8 & 3) + 2;
        tribute = (220 - imperial_favour) / 2;
        if      (rolling_profit >=  4000) moving_tribute += tribute_adjust[0];
        else if (rolling_profit >=  2000) moving_tribute += tribute_adjust[1];
        else if (rolling_profit <= -4000) moving_tribute += tribute_adjust[2];
        else if (rolling_profit <= -2000) moving_tribute += tribute_adjust[3];
        if      (denarii >= 40000) moving_tribute += tribute_adjust[4];
        else if (denarii >= 20000) moving_tribute += tribute_adjust[5];
        else if (denarii <  1000) moving_tribute += tribute_adjust[6];
        if (moving_tribute <    0) moving_tribute =    0;
        if (moving_tribute > 2000) moving_tribute = 2000;
        tribute += moving_tribute;
        if (tribute <    0) tribute =    0;
        if (tribute > 2000) tribute = 2000;
        if (last_tribute > tribute) {
            if      (rolling_profit <= -4000) put_message(143, 0, 11);
            else if (denarii        <  1000) put_message(142, 0, 11);
            else if (imperial_favour <   80) put_message(124, 0, 11);
            else                             put_message(123, 0, 11);
        } else if (last_tribute < tribute) {
            if      (denarii >= 50000)        put_message(141, 0, 14);
            else if (rolling_profit >= 4000) put_message(140, 0, 14);
            else if (imperial_favour > 130)  put_message(122, 0, 14);
            else                             put_message(121, 0, 14);
        }
    }

    imperial_tax -= 1; if (imperial_tax <= 0) {
        imperial_tax = main_paras[5] + (rand8 & 3); province_count = completed_provinces;
        if (players_denarii >= tax_triggers[0]) {
            last_imperial_tax_percent = tax_rates[province_count];
            last_imperial_tax_amount = totalXpercent(players_denarii, last_imperial_tax_percent);
            total_imperial_taxes += last_imperial_tax_amount;
            put_message(146, 0, 14);
        } else if (players_denarii >= tax_triggers[1]) {
            last_imperial_tax_percent = tax_rates[20 + province_count];
            last_imperial_tax_amount = totalXpercent(players_denarii, last_imperial_tax_percent);
            total_imperial_taxes += last_imperial_tax_amount;
            put_message(145, 0, 14);
        } else if (players_denarii >= tax_triggers[2]) {
            last_imperial_tax_percent = tax_rates[40 + province_count];
            last_imperial_tax_amount = totalXpercent(players_denarii, last_imperial_tax_percent);
            total_imperial_taxes += last_imperial_tax_amount;
            put_message(144, 0, 14);
        }
    }
}

// Resets imperial favour, tribute, bribe, gift, and personal-tax state.
// FUNCTION: C2 0x57958
// FUNCTION: C2WIN 0x004586ae
void init_tribute(void) {
    imperial_favour         = 110;
    tribute                 = (200 - imperial_favour) / 2;
    moving_tribute          = 0;
    last_tribute            = 0;
    total_amount_of_bribes  = 0;
    total_no_of_bribes      = 0;
    imperial_gift_level     = 0;
    av_imperial_gift_level  = 0;
    imperial_tax            = 1;
    last_imperial_tax_amount  = 0;
    last_imperial_tax_percent = 0;
    total_imperial_taxes      = 0;
}

// Selects and speaks a rating-specific temple advisor tip.
// FUNCTION: C2 0x579b1
// FUNCTION: C2WIN 0x0045873d
void get_temple_tip(int rating_kind) {
    int i, j, k;
    if (rating_kind == 0) {
        if (empire_rating_pop_limit != 0)       { current_temple_tip = 1; play_speech(31); }
        else if (imperial_favour < 80)          { current_temple_tip = 2; play_speech(32); }
        else if (no_of_empire_connections == 0) { current_temple_tip = 3; play_speech(33); }
        else                                    { current_temple_tip = 4; play_speech(34); }
        return;
    }
    if (rating_kind == 1) {
        if (peace_rating_pop_limit != 0) { current_temple_tip = 5; play_speech(35); }
        else                             { current_temple_tip = 6; play_speech(36); }
        return;
    }
    if (rating_kind == 2) {
        if (prosperity_rating_pop_limit != 0) { current_temple_tip =  9; play_speech(37); }
        else if (rolling_profit < 0)          { current_temple_tip = 10; play_speech(38); }
        else if (current_gdp < 10)            { current_temple_tip = 11; play_speech(39); }
        else                                  { current_temple_tip = 12; play_speech(40); }
        return;
    }
    // Culture advice identifies the weakest service category.
    if (culture_rating_pop_limit != 0) { current_temple_tip = 13; play_speech(41); }
    else if (entertainment_level <= religion_level && entertainment_level <= utility_level)
                                       { current_temple_tip = 14; play_speech(42); }
    else if (religion_level <= entertainment_level && religion_level <= utility_level)
                                       { current_temple_tip = 15; play_speech(43); }
    else                               { current_temple_tip = 16; play_speech(44); }
}
