
#include "c2_data.h"

#if !PLATFORM_WINDOWS

extern void put_a_font_string(char *str, int x, int y, unsigned char *font, int color);
extern void font_no(int value, char pad_char, char *suffix, int x, int y, unsigned char *font, int color);
/* Forward declarations (functions defined later in this file). */
void show_debug_screen(void);


// Displays the debug screen until a key is pressed, then restores the active map.
// FUNCTION: C2 0x713cf
void debug_screen(void) {
    show_debug_screen();
    wait_key();
    if (map_mode == 0)
        city_map_screen(1);
    else if (map_mode == 1)
        region_map_screen(1);
}

// Displays city services, growth, industry supply, population, military, and connection diagnostics.
// FUNCTION: C2 0x713fd
void show_debug_screen(void) {
    cover_mouse_droppings();
    show_a_system_window(0, 0, 0x28, 0x1e);

    // Fountains
    put_a_font_string("Fountains ", 8, 8, font1, 0x3f);
    font_no(fountains_count, 0x20, " ", 100, 8, font1, 0x3f);
    // -supplied
    put_a_font_string("-supplied ", 8, 0x17, font1, 0x3f);
    font_no(supplied_fountains_count, 0x20, " ", 100, 0x17, font1, 0x3f);
    // Baths
    put_a_font_string("Baths ", 8, 0x26, font1, 0x3f);
    font_no(baths_count, 0x20, " ", 100, 0x26, font1, 0x3f);
    // -supplied
    put_a_font_string("-supplied ", 8, 0x35, font1, 0x3f);
    font_no(supplied_baths_count, 0x20, " ", 100, 0x35, font1, 0x3f);

    // Hospitals
    put_a_font_string("Hospitals ", 8, 0x44, font1, 0x3f);
    font_no(hospitals_count, 0x20, " ", 100, 0x44, font1, 0x3f);
    // -accessed
    put_a_font_string("-accessed ", 8, 0x53, font1, 0x3f);
    font_no(accessed_hospitals_count, 0x20, " ", 100, 0x53, font1, 0x3f);
    // -cover
    put_a_font_string("-cover ", 8, 0x62, font1, 0x3f);
    font_no(hospital_cover, 0x20, " %", 100, 0x62, font1, 0x3f);
    // -disease
    put_a_font_string("-disease ", 8, 0x71, font1, 0x3f);
    font_no(plague_running_count, 0x20, " ", 100, 0x71, font1, 0x3f);

    // Libraries
    put_a_font_string("Libraries ", 8, 0x80, font1, 0x3f);
    font_no(libraries_count, 0x20, " ", 100, 0x80, font1, 0x3f);
    // -accessed
    put_a_font_string("-accessed ", 8, 0x8f, font1, 0x3f);
    font_no(accessed_libraries_count, 0x20, " ", 100, 0x8f, font1, 0x3f);
    // -cover
    put_a_font_string("-cover ", 8, 0x9e, font1, 0x3f);
    font_no(library_cover, 0x20, " %", 100, 0x9e, font1, 0x3f);

    // Temples (sum of small+med+large)
    put_a_font_string("Temples ", 8, 0xad, font1, 0x3f);
    font_no(large_temples_count + med_temples_count + small_temples_count, 0x20, " ", 100, 0xad, font1, 0x3f);
    // - culture (sum)
    put_a_font_string("- culture ", 8, 0xbc, font1, 0x3f);
    font_no(large_temples_culture_count + med_temples_culture_count + small_temples_culture_count, 0x20, " ", 100, 0xbc, font1, 0x3f);

    // Pleasure (sum of entertainment venues)
    put_a_font_string("Pleasure ", 8, 0xcb, font1, 0x3f);
    font_no(theatre_count + odium_count + arena_count + colosseum_count + circus_count + circus_maximus_count, 0x20, " ", 100, 0xcb, font1, 0x3f);
    // - culture (culture sum)
    put_a_font_string("- culture ", 8, 0xda, font1, 0x3f);
    font_no(theatre_culture_count + odium_culture_count + arena_culture_count + colosseum_culture_count + circus_culture_count + circus_maximus_culture_count, 0x20, " ", 100, 0xda, font1, 0x3f);

    // Entertain.
    put_a_font_string("Entertain. ", 8, 0xe9, font1, 0x3f);
    font_no(entertainment_level, 0x20, " ", 100, 0xe9, font1, 0x3f);
    // Religion
    put_a_font_string("Religion ", 8, 0xf8, font1, 0x3f);
    font_no(religion_level, 0x20, " ", 100, 0xf8, font1, 0x3f);
    // Utilities
    put_a_font_string("Utilities ", 8, 0x107, font1, 0x3f);
    font_no(utility_level, 0x20, " ", 100, 0x107, font1, 0x3f);

    // GDP
    put_a_font_string("GDP ", 8, 0x116, font1, 0x3f);
    font_no(current_gdp, 0x20, " ", 100, 0x116, font1, 0x3f);
    // rolling profit (signed)
    put_a_font_string("rolling ", 8, 0x125, font1, 0x3f);
    if (rolling_profit < 0) font_no(-rolling_profit, 0x2d, " ", 100, 0x125, font1, 3);
    else font_no(rolling_profit, 0x20, " ", 100, 0x125, font1, 0x3f);

    // Fire rate
    put_a_font_string("Fire rate ", 8, 0x143, font1, 0x3f);
    font_no(fire_rate, 0x20, " ", 100, 0x143, font1, 0x3f);
    // Road rate
    put_a_font_string("Road rate ", 8, 0x152, font1, 0x3f);
    font_no(road_rate, 0x20, " ", 100, 0x152, font1, 0x3f);
    // Wall rate
    put_a_font_string("Wall rate ", 8, 0x161, font1, 0x3f);
    font_no(wall_rate, 0x20, " ", 100, 0x161, font1, 0x3f);
    // Water rate
    put_a_font_string("Water rate ", 8, 0x170, font1, 0x3f);
    font_no(water_trouble_rate, 0x20, " ", 100, 0x170, font1, 0x3f);

    // Road acc
    put_a_font_string("Road acc", 8, 0x17f, font1, 0x3f);
    font_no(road_accident, 0x20, " ", 100, 0x17f, font1, 0x3f);
    // Wall acc
    put_a_font_string("Wall acc", 8, 0x18e, font1, 0x3f);
    font_no(wall_accident, 0x20, " ", 100, 0x18e, font1, 0x3f);
    // Fire acc
    put_a_font_string("Fire acc", 8, 0x19d, font1, 0x3f);
    font_no(fire_accident, 0x20, " ", 100, 0x19d, font1, 0x3f);

    // Wall cov
    put_a_font_string("Wall cov", 8, 0x1ac, font1, 0x3f);
    font_no(wall_cover, 0x20, " ", 100, 0x1ac, font1, 0x3f);

    // pop growth (signed)
    put_a_font_string("pop growth ", 0xc8, 8, font1, 0x3f);
    if (pop_growth_factor < 0) font_no(-pop_growth_factor, 0x2d, " ", 300, 8, font1, 3);
    else font_no(pop_growth_factor, 0x20, " ", 300, 8, font1, 0x3f);
    // - future (signed)
    put_a_font_string("- future ", 0xc8, 0x17, font1, 0x3f);
    if (pop_growth_future < 0) font_no(-pop_growth_future, 0x2d, " ", 300, 0x17, font1, 3);
    else font_no(pop_growth_future, 0x20, " ", 300, 0x17, font1, 0x3f);
    // ind growth (signed)
    put_a_font_string("ind growth ", 0xc8, 0x26, font1, 0x3f);
    if (ind_growth_factor < 0) font_no(-ind_growth_factor, 0x2d, " ", 300, 0x26, font1, 3);
    else font_no(ind_growth_factor, 0x20, " ", 300, 0x26, font1, 0x3f);
    // - future (signed)
    put_a_font_string("- future ", 0xc8, 0x35, font1, 0x3f);
    if (ind_growth_future < 0) font_no(-ind_growth_future, 0x2d, " ", 300, 0x35, font1, 3);
    else font_no(ind_growth_future, 0x20, " ", 300, 0x35, font1, 0x3f);
    // insurrection (signed)
    put_a_font_string("insurrection ", 0xc8, 0x44, font1, 0x3f);
    if (insurrection_factor < 0) font_no(-insurrection_factor, 0x2d, " ", 300, 0x44, font1, 3);
    else font_no(insurrection_factor, 0x20, " ", 300, 0x44, font1, 0x3f);
    // - future (signed)
    put_a_font_string("- future ", 0xc8, 0x53, font1, 0x3f);
    if (insurrection_future < 0) font_no(-insurrection_future, 0x2d, " ", 300, 0x53, font1, 3);
    else font_no(insurrection_future, 0x20, " ", 300, 0x53, font1, 0x3f);

    // conscription rate
    put_a_font_string("conscription rate", 0xc8, 0x62, font1, 0x3f);
    font_no(conscription_rate, 0x20, " ", 300, 0x62, font1, 0x3f);
    // employment
    put_a_font_string("employment", 0xc8, 0x71, font1, 0x3f);
    font_no(employment_rate, 0x20, " ", 300, 0x71, font1, 0x3f);
    // employees
    put_a_font_string("employees", 0xc8, 0x80, font1, 0x3f);
    font_no(employees, 0x20, " ", 300, 0x80, font1, 0x3f);
    // population
    put_a_font_string("population", 0xc8, 0x8f, font1, 0x3f);
    font_no(population, 0x20, " ", 300, 0x8f, font1, 0x3f);

    // grain sup/sat
    put_a_font_string("grain sup/sat", 0xc8, 0x9e, font1, 0x3f);
    x_is = 0;
    font_no(industry[0].city_supply, 0x20, " ", 300, 0x9e, font1, 0x3f);
    font_no(industry[0].supply_pipeline[0], 0x2f, " ", x_is + 300, 0x9e, font1, 0x3f);
    // grapes
    put_a_font_string("grapes sup/sat", 0xc8, 0xad, font1, 0x3f);
    x_is = 0;
    font_no(industry[1].city_supply, 0x20, " ", 300, 0xad, font1, 0x3f);
    font_no(industry[1].supply_pipeline[0], 0x2f, " ", x_is + 300, 0xad, font1, 0x3f);
    // cattle
    put_a_font_string("cattle sup/sat", 0xc8, 0xbc, font1, 0x3f);
    x_is = 0;
    font_no(industry[2].city_supply, 0x20, " ", 300, 0xbc, font1, 0x3f);
    font_no(industry[2].supply_pipeline[0], 0x2f, " ", x_is + 300, 0xbc, font1, 0x3f);
    // timber
    put_a_font_string("timber sup/sat", 0xc8, 0xcb, font1, 0x3f);
    x_is = 0;
    font_no(industry[3].city_supply, 0x20, " ", 300, 0xcb, font1, 0x3f);
    font_no(industry[3].supply_pipeline[0], 0x2f, " ", x_is + 300, 0xcb, font1, 0x3f);
    // gems
    put_a_font_string("gems sup/sat", 0xc8, 0xda, font1, 0x3f);
    x_is = 0;
    font_no(industry[4].city_supply, 0x20, " ", 300, 0xda, font1, 0x3f);
    font_no(industry[4].supply_pipeline[0], 0x2f, " ", x_is + 300, 0xda, font1, 0x3f);
    // lead
    put_a_font_string("lead sup/sat", 0xc8, 0xe9, font1, 0x3f);
    x_is = 0;
    font_no(industry[5].city_supply, 0x20, " ", 300, 0xe9, font1, 0x3f);
    font_no(industry[5].supply_pipeline[0], 0x2f, " ", x_is + 300, 0xe9, font1, 0x3f);
    // iron
    put_a_font_string("iron sup/sat", 0xc8, 0xf8, font1, 0x3f);
    x_is = 0;
    font_no(industry[6].city_supply, 0x20, " ", 300, 0xf8, font1, 0x3f);
    font_no(industry[6].supply_pipeline[0], 0x2f, " ", x_is + 300, 0xf8, font1, 0x3f);
    // copper
    put_a_font_string("copper sup/sat", 0xc8, 0x107, font1, 0x3f);
    x_is = 0;
    font_no(industry[7].city_supply, 0x20, " ", 300, 0x107, font1, 0x3f);
    font_no(industry[7].supply_pipeline[0], 0x2f, " ", x_is + 300, 0x107, font1, 0x3f);
    // clay
    put_a_font_string("clay sup/sat", 0xc8, 0x116, font1, 0x3f);
    x_is = 0;
    font_no(industry[8].city_supply, 0x20, " ", 300, 0x116, font1, 0x3f);
    font_no(industry[8].supply_pipeline[0], 0x2f, " ", x_is + 300, 0x116, font1, 0x3f);
    // sand
    put_a_font_string("sand sup/sat", 0xc8, 0x125, font1, 0x3f);
    x_is = 0;
    font_no(industry[9].city_supply, 0x20, " ", 300, 0x125, font1, 0x3f);
    font_no(industry[9].supply_pipeline[0], 0x2f, " ", x_is + 300, 0x125, font1, 0x3f);
    // marble
    put_a_font_string("marble sup/sat", 0xc8, 0x134, font1, 0x3f);
    x_is = 0;
    font_no(industry[10].city_supply, 0x20, " ", 300, 0x134, font1, 0x3f);
    font_no(industry[10].supply_pipeline[0], 0x2f, " ", x_is + 300, 0x134, font1, 0x3f);
    // stone
    put_a_font_string("stone sup/sat", 0xc8, 0x143, font1, 0x3f);
    x_is = 0;
    font_no(industry[11].city_supply, 0x20, " ", 300, 0x143, font1, 0x3f);
    font_no(industry[11].supply_pipeline[0], 0x2f, " ", x_is + 300, 0x143, font1, 0x3f);
    // silk
    put_a_font_string("silk sup/sat", 0xc8, 0x152, font1, 0x3f);
    x_is = 0;
    font_no(industry[12].city_supply, 0x20, " ", 300, 0x152, font1, 0x3f);
    font_no(industry[12].supply_pipeline[0], 0x2f, " ", x_is + 300, 0x152, font1, 0x3f);
    // spices
    put_a_font_string("spices sup/sat", 0xc8, 0x161, font1, 0x3f);
    x_is = 0;
    font_no(industry[13].city_supply, 0x20, " ", 300, 0x161, font1, 0x3f);
    font_no(industry[13].supply_pipeline[0], 0x2f, " ", x_is + 300, 0x161, font1, 0x3f);
    // ivory
    put_a_font_string("ivory sup/sat", 0xc8, 0x170, font1, 0x3f);
    x_is = 0;
    font_no(industry[14].city_supply, 0x20, " ", 300, 0x170, font1, 0x3f);
    font_no(industry[14].supply_pipeline[0], 0x2f, " ", x_is + 300, 0x170, font1, 0x3f);
    // fish
    put_a_font_string("fish sup/sat", 0xc8, 0x17f, font1, 0x3f);
    x_is = 0;
    font_no(industry[15].city_supply, 0x20, " ", 300, 0x17f, font1, 0x3f);
    font_no(industry[15].supply_pipeline[0], 0x2f, " ", x_is + 300, 0x17f, font1, 0x3f);

    // Workcamps (column 2)
    put_a_font_string("Workcamps", 0xc8, 0x18e, font1, 0x3f);
    font_no(no_of_workcamps, 0x20, " ", 300, 0x18e, font1, 0x3f);
    // No of citizens (column 3)
    put_a_font_string("No of citizens", 0x190, 8, font1, 0x3f);
    font_no(no_of_citizens, 0x20, " ", 500, 8, font1, 0x3f);
    // No of armies
    put_a_font_string("No of armies", 0x190, 0x17, font1, 0x3f);
    font_no(no_of_armies, 0x20, " ", 500, 0x17, font1, 0x3f);
    // No of units
    put_a_font_string("No of units", 0x190, 0x26, font1, 0x3f);
    font_no(no_of_units, 0x20, " ", 500, 0x26, font1, 0x3f);
    // No of figures
    put_a_font_string("No of figures", 0x190, 0x35, font1, 0x3f);
    font_no(no_of_figures, 0x20, " ", 500, 0x35, font1, 0x3f);
    // No of arrows
    put_a_font_string("No of arrows", 0x190, 0x44, font1, 0x3f);
    font_no(no_of_arrows, 0x20, " ", 500, 0x44, font1, 0x3f);

    // Connections
    put_a_font_string("Connections", 0x190, 0x53, font1, 0x3f);
    x_is = 0;

    // N connection
    if (empire_connections[0] != 0)
        put_a_font_string("N", 0x1f4, 0x53, font1, 0x3f);
    // E connection
    if (empire_connections[1] != 0)
        put_a_font_string("E", x_is + 0x1f4, 0x53, font1, 0x3f);
    // S connection
    if (empire_connections[2] != 0)
        put_a_font_string("S", x_is + 0x1f4, 0x53, font1, 0x3f);
    // W connection
    if (empire_connections[3] != 0)
        put_a_font_string("W", x_is + 0x1f4, 0x53, font1, 0x3f);

    setup_whole_screen_refresh();
    refresh_svga_screen();
    hold_mouse_replace = 1;
}
#endif
