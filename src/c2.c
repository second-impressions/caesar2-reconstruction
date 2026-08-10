
#include <stdlib.h>
#include <fcntl.h>             /* O_BINARY */

#include "c2_data.h"

struct gfx_entry c2_map_gfx[48] = {
    { "ltlmen1b.pl8", 60000 },
    { "cityfixt.pl8", 121978 },
    { "houses1.pl8", 168528 },
    { "build1a.pl8", 147740 },
    { "build1b.pl8", 173728 },
    { "build1c.pl8", 101546 },
    { "build1d.pl8", 109856 },
    { "citytop1.pl8", 43800 },
    { "ltlmen2b.pl8", 18000 },
    { "cityfix2.pl8", 29688 },
    { "houses2.pl8", 37422 },
    { "build2a.pl8", 33836 },
    { "build2b.pl8", 39036 },
    { "build2c.pl8", 23976 },
    { "build2d.pl8", 26000 },
    { "citytop2.pl8", 43800 },
    { "ltlmen3b.pl8", 8000 },
    { "cityfix3.pl8", 7288 },
    { "houses3.pl8", 8272 },
    { "build3a.pl8", 7662 },
    { "build3b.pl8", 8090 },
    { "build3c.pl8", 5250 },
    { "build3d.pl8", 5918 },
    { "citytop3.pl8", 43800 },
    { "my_stds.pl8", 158712 },
    { "provfixt.pl8", 128248 },
    { "prvbld1a.pl8", 108800 },
    { "mountns1.pl8", 110250 },
    { "prvbld1b.pl8", 116684 },
    { "build1c.pl8", 0 },
    { "build1d.pl8", 0 },
    { "citytop1.pl8", 18000 },
    { "my_stds2.pl8", 39194 },
    { "provfix2.pl8", 29688 },
    { "prvbld2a.pl8", 23442 },
    { "mountns2.pl8", 25166 },
    { "prvbld2b.pl8", 27038 },
    { "build2c.pl8", 0 },
    { "build2d.pl8", 0 },
    { "citytop2.pl8", 18000 },
    { "my_stds3.pl8", 9024 },
    { "provfix3.pl8", 7288 },
    { "prvbld3a.pl8", 5700 },
    { "mountns3.pl8", 6502 },
    { "prvbld3b.pl8", 6530 },
    { "build3c.pl8", 0 },
    { "build3d.pl8", 0 },
    { "citytop3.pl8", 18000 }
};

struct gfx_entry c2_overlay_gfx[3] = {
    { "overlay1.pl8", 60000 },
    { "overlay2.pl8", 18000 },
    { "overlay3.pl8", 8000 }
};

struct gfx_entry c2_battle_gfx[68] = {
    { "batlfix2.pl8", 14000 },
    { "RO2SWDA.PL8", 90000 },
    { "RO2SPRB.PL8", 105000 },
    { "RO2SLGC.PL8", 98000 },
    { "AF2SPRA.PL8", 102000 },
    { "AF2KNFB.PL8", 100000 },
    { "AF2BOWC.PL8", 98000 },
    { "AR2SPRA.PL8", 115000 },
    { "AR2SWDB.PL8", 118000 },
    { "AR2BOWC.PL8", 95000 },
    { "GM2SWDA.PL8", 95000 },
    { "GM2SPRB.PL8", 105000 },
    { "GM2BOWC.PL8", 105000 },
    { "GL2SWDA.PL8", 110000 },
    { "GL2SPRB.PL8", 105000 },
    { "GL2BOWC.PL8", 98000 },
    { "BR2SWDA.PL8", 110000 },
    { "BR2SWDB.PL8", 105000 },
    { "BR2JAVC.PL8", 98000 },
    { "HN2CAVA1.PL8", 65000 },
    { "HN2CAVA2.PL8", 120000 },
    { "HN2SWDB.PL8", 100000 },
    { "CA2CAVA1.PL8", 235000 },
    { "CA2CAVA2.PL8", 0 },
    { "CA2SPRB.PL8", 110000 },
    { "GK2SPRA.PL8", 110000 },
    { "GK2SPRB.PL8", 105000 },
    { "GK2SLGC.PL8", 95000 },
    { "EG2SPRA.PL8", 102000 },
    { "EG2SWDB.PL8", 100000 },
    { "EG2BOWC.PL8", 98000 },
    { "PA2CAVA1.PL8", 65000 },
    { "PA2CAVA2.PL8", 120000 },
    { "PA2SWDB.PL8", 100000 },
    { "batlfix3.pl8", 14000 },
    { "RO3SWDA.PL8", 90000 },
    { "RO3SPRB.PL8", 105000 },
    { "RO3SLGC.PL8", 98000 },
    { "AF3SPRA.PL8", 102000 },
    { "AF3KNFB.PL8", 100000 },
    { "AF3BOWC.PL8", 98000 },
    { "AR3SPRA.PL8", 115000 },
    { "AR3SWDB.PL8", 118000 },
    { "AR3BOWC.PL8", 95000 },
    { "GM3SWDA.PL8", 95000 },
    { "GM3SPRB.PL8", 105000 },
    { "GM3BOWC.PL8", 105000 },
    { "GL3SWDA.PL8", 110000 },
    { "GL3SPRB.PL8", 105000 },
    { "GL3BOWC.PL8", 98000 },
    { "BR3SWDA.PL8", 110000 },
    { "BR3SWDB.PL8", 105000 },
    { "BR3JAVC.PL8", 98000 },
    { "HN3CAVA1.PL8", 65000 },
    { "HN3CAVA2.PL8", 120000 },
    { "HN3SWDB.PL8", 100000 },
    { "CA3CAVA1.PL8", 235000 },
    { "CA3CAVA2.PL8", 0 },
    { "CA3SPRB.PL8", 110000 },
    { "GK3SPRA.PL8", 110000 },
    { "GK3SPRB.PL8", 105000 },
    { "GK3SLGC.PL8", 95000 },
    { "EG3SPRA.PL8", 102000 },
    { "EG3SWDB.PL8", 100000 },
    { "EG3BOWC.PL8", 98000 },
    { "PA3CAVA1.PL8", 65000 },
    { "PA3CAVA2.PL8", 120000 },
    { "PA3SWDB.PL8", 100000 }
};

struct gfx_entry c2_battle_aux_gfx[68] = {
    { "batlfix2.pl8", 14000 },
    { "RO2SWDAX.PL8", 90000 },
    { "RO2SPRBX.PL8", 105000 },
    { "RO2SLGCX.PL8", 98000 },
    { "AF2SPRAX.PL8", 102000 },
    { "AF2KNFBX.PL8", 100000 },
    { "AF2BOWCX.PL8", 98000 },
    { "AR2SPRAX.PL8", 115000 },
    { "AR2SWDBX.PL8", 118000 },
    { "AR2BOWCX.PL8", 95000 },
    { "GM2SWDAX.PL8", 95000 },
    { "GM2SPRBX.PL8", 105000 },
    { "GM2BOWCX.PL8", 105000 },
    { "GL2SWDAX.PL8", 110000 },
    { "GL2SPRBX.PL8", 105000 },
    { "GL2BOWCX.PL8", 98000 },
    { "BR2SWDAX.PL8", 110000 },
    { "BR2SWDBX.PL8", 105000 },
    { "BR2JAVCX.PL8", 98000 },
    { "HN2CAVA3.PL8", 65000 },
    { "HN2CAVA4.PL8", 120000 },
    { "HN2SWDBX.PL8", 100000 },
    { "CA2CAVA3.PL8", 235000 },
    { "CA2CAVA4.PL8", 0 },
    { "CA2SPRBX.PL8", 110000 },
    { "GK2SPRAX.PL8", 110000 },
    { "GK2SPRBX.PL8", 105000 },
    { "GK2SLGCX.PL8", 95000 },
    { "EG2SPRAX.PL8", 102000 },
    { "EG2SWDBX.PL8", 100000 },
    { "EG2BOWCX.PL8", 98000 },
    { "PA2CAVA3.PL8", 65000 },
    { "PA2CAVA4.PL8", 120000 },
    { "PA2SWDBX.PL8", 100000 },
    { "batlfix3.pl8", 14000 },
    { "RO3SWDAX.PL8", 90000 },
    { "RO3SPRBX.PL8", 105000 },
    { "RO3SLGCX.PL8", 98000 },
    { "AF3SPRAX.PL8", 102000 },
    { "AF3KNFBX.PL8", 100000 },
    { "AF3BOWCX.PL8", 98000 },
    { "AR3SPRAX.PL8", 115000 },
    { "AR3SWDBX.PL8", 118000 },
    { "AR3BOWCX.PL8", 95000 },
    { "GM3SWDAX.PL8", 95000 },
    { "GM3SPRBX.PL8", 105000 },
    { "GM3BOWCX.PL8", 105000 },
    { "GL3SWDAX.PL8", 110000 },
    { "GL3SPRBX.PL8", 105000 },
    { "GL3BOWCX.PL8", 98000 },
    { "BR3SWDAX.PL8", 110000 },
    { "BR3SWDBX.PL8", 105000 },
    { "BR3JAVCX.PL8", 98000 },
    { "HN3CAVA3.PL8", 65000 },
    { "HN3CAVA4.PL8", 120000 },
    { "HN3SWDBX.PL8", 100000 },
    { "CA3CAVA3.PL8", 235000 },
    { "CA3CAVA4.PL8", 0 },
    { "CA3SPRBX.PL8", 110000 },
    { "GK3SPRAX.PL8", 110000 },
    { "GK3SPRBX.PL8", 105000 },
    { "GK3SLGCX.PL8", 95000 },
    { "EG3SPRAX.PL8", 102000 },
    { "EG3SWDBX.PL8", 100000 },
    { "EG3BOWCX.PL8", 98000 },
    { "PA3CAVA3.PL8", 65000 },
    { "PA3CAVA4.PL8", 120000 },
    { "PA3SWDBX.PL8", 100000 }
};

/* Persistent game settings and player information. */
struct c2inf_rec c2inf;

#if PLATFORM_WINDOWS
static unsigned char city_gfx_loaded;
static unsigned char city_gfx_zoom;
static unsigned char province_gfx_loaded;
static unsigned char province_gfx_zoom;
static unsigned char windows_game_active;
#endif

extern void *malloc(unsigned int size);
extern void  printf(const char *fmt, ...);
extern void  exit(int status);

extern int read_config();
extern int to_upper();

extern int   _getdrive(void);
extern int   getch(void);
extern void  demo_lead_in_slideshow(void);
extern void  free_tune_buffer(void);
#if PLATFORM_WINDOWS
extern void  close_windows(void);
extern void  show_map_window(int mode);
extern void  update_window_menu(int mode);
extern void  update_sound_menu(void);
extern void  start_windows_game(void);
#endif
void *load_a_battle_gfx_file(int battle_zoom, int troop_gfx_idx, int use_aux);
extern void get_pseudo_map(int n);
extern unsigned _dos_setdrive(unsigned drive, unsigned *total);
extern unsigned _dos_getdrive(unsigned *drive);
extern int      chdir(const char *path);
extern int      open(const char *path, int flags, ...);
extern int      close(int fd);
#if PLATFORM_WINDOWS
extern int      _chdrive(int drive);
extern int      _getdrive(void);
#endif
/* Forward declarations (functions defined later in this file). */
void deal_with_battles(void);
void start_a_new_game(void);
void start_a_promotion(void);
void new_province(void);
void setup_game(void);
void init_map_gfx_buffers();
void clear_map_gfx_buffers();
void init_battle_gfx_buffers(void);
void clear_battle_gfx_buffers(void);
void flush_sb_buffer(void);


// Initializes the game, runs campaign sessions until exit, then releases resources.
// FUNCTION: C2 0x10010
// FUNCTION: C2WIN 0x00443733
void main(int argc, char *argv[])
{
    unsigned int cd_drive_config;
    int cd_error;
    int graphics_error;
    int init_error;

    demo_mode = 1;
    demo_mode = 0;

    drive_name = _getdrive() + 0x40;

    if (getcwd(path_name, 0x50) == NULL) goto end;

    c2inf.cd_letter  = 0;
    c2inf.drive_init = 1;

    cd_drive_config = (unsigned char)read_config("resource.cfg", misc);
    c2inf.cd_letter = to_upper(cd_drive_config);
    if (cd_drive_config == 1) {
        high_beep();
        c2inf.drive_init = 0;
    } else {
        cd_error = 1;
        while (cd_error != 0) {
            cd_error = test_cd_drive();
            if (cd_error == 1) {
                printf("\nNo CD information found.\n");
            } else if (cd_error == 2) {
                printf("\nThe drive entered is not a valid drive.\n");
            } else if (cd_error == 3) {
                printf("\nThe drive could not be accessed.\n");
            } else if (cd_error == 4) {
                printf("\nNo CD check info found..\n");
            } else if (cd_error == 5) {
                printf("\nCould not reset to current drive.\n");
            } else if (cd_error == 6) {
                printf("\nCould not reset to current path.\n");
            } else {
                printf("\nCD version installed OK.\n");
                break;
            }
            printf("    1) Enter a valid drive letter.\n");
            printf("    2) Enter SPACE to play from hard-drive only.\n");
            printf("    3) Enter ESC to exit to DOS.\n");
            c2inf.cd_letter = to_upper(getch() & 0xff);
            if (c2inf.cd_letter == 0x1b) {
                printf("\nCAESAR 2 aborted.\n");
                exit(0);
            }
            if (c2inf.cd_letter >= 'A' && c2inf.cd_letter <= 'Z') continue;
            c2inf.drive_init = 0;
            break;
        }
    }

    map_mode   = 0;
    zoom_level = 0;
    init_map_gfx_buffers();
    init_battle_gfx_buffers();
    graphics_error = load_start_graphics();
    if (graphics_error != 0) {
        printf("File not found - code%d.\n", graphics_error);
        exit(0);
    }
    load_map_graphics(0, 0);
    screen_mode = 2;

    if (init_mouse() == 0) {
        printf("No mouse driver found.\n");
        exit(0);
    }

    scratch_buffer_size = 0x27100;
    init_error = 0;
    if (start_system() == 0) init_error = 1;
    if (internal_screen == 0) init_error = 2;
    if (scratch_buffer == 0)  init_error = 3;
    if (init_sample_buffer(0xa) == 0) init_error = 4;
    if (init_tune_buffer() == 0)      init_error = 5;

    if (init_error != 0) {
        no_high_beeps(init_error);
        stop_system();
        printf("Not enough free memory to run Caesar2.\n");
        exit(100);
    }

    flush_sb_buffer();
    init_queery_panel();
    refresh_zoom_mode(0);
    setup_svga_refresh_data();
    load_inf();
    lead_in_logos();
    do_svga_smacked_anim("intro.smk");
    play_tune("forum1.xmi", 1);
    demo_lead_in_slideshow();

    hold_hot_keys = 1;
    background_screen();
    click_warning(0, 0xa0, 0x100);

    turbo_mode = 0;
    exit_flag  = 0;

    while (exit_flag == 0) {
        if (game_state == 3) {
            start_a_promotion();
        } else if (pre_loaded_status == 0) {
            start_a_new_game();
        }

        if (exit_flag != 0) break;

        city_tune_playing = 0;
        hold_hot_keys     = 0;

        if (pre_loaded_status == 0) clear_landfill();
        act_correct_map();
        last_icon_used   = 0;
        placing_type     = 0;
        placing_cost     = 0;
        total_build_cost = 0;
        update_map       = 1;
        restart_flag     = 0;

        if (pre_loaded_status == 0) clear_landfill();

        turbo_mode = 0;
        while (1) {
            if (restart_flag != 0 || exit_flag != 0) break;
            if (pre_loaded_status == 0) {
                main_game_loop();
                if (restart_flag != 0) break;
            }
            deal_with_battles();
            if (restart_flag != 0) break;
            pre_loaded_status = 0;
            if (game_state != 0) {
                restart_flag = 1;
            }
        }

        if (game_state == 1) do_lose_game();
        if (exit_flag == 0) play_tune("forum1.xmi", 1);
    }

    demo_lead_in_slideshow();
    save_inf();
    free_tune_buffer();
    free_sample_buffer(10);
    clear_map_gfx_buffers();
    clear_battle_gfx_buffers();
    exit_game();

end:;
}

// Continues an active battle and returns to normal play unless the battle requests a restart.
// FUNCTION: C2 0x10409
// FUNCTION: C2WIN 0x00443c91
void deal_with_battles(void)
{
    if (game_state != 4) return;

    if (battle_type == 2) {
        continue_battle(pre_loaded_status);
        if (battle_victor == 0) {
            RM_CELL(battle2_ptr).base_kind     = 0x97;
            RM_CELL(battle2_ptr).gfx = 0x32;
        }
    } else if (army_list[our_battle_army].state_idx != 2 &&
               army_list[their_battle_army].state_idx != 2) {
        continue_battle(pre_loaded_status);
    }

    if (restart_flag == 0) {
        game_state = 0;
    }
}

// Initializes a new campaign and opens its first province unless a saved game or tutorial was loaded.
// FUNCTION: C2 0x1049b
// FUNCTION: C2WIN 0x00443d62
void start_a_new_game(void)
{
    setup_game();
    if (!develop_mode) act_set_skill_levels();
    if (exit_flag)     exit_game();
    if (pre_loaded_status)        return;
    if (continue_tutorial_status) return;
    start_year = year = -300;          /* 300 BC */
    month = 0;
    week = 0;
    years_elapsed = 0;
    completed_provinces = 0;
    province_is = 0;
    player_rank = 0;
    players_denarii = 0;
    players_salary = init_salary[0];
    init_tribute();
    clear_empire();
    new_province();
}

// Advances the calendar as needed and starts the province awarded by a promotion.
// FUNCTION: C2 0x10529
// FUNCTION: C2WIN 0x00443e35
void start_a_promotion(void)
{
#if PLATFORM_WINDOWS
    windows_game_active = 0;
    show_map_window(0);
    show_map_window(1);
    show_map_window(2);
    update_window_menu(0);
    update_window_menu(1);
    update_sound_menu();
#endif
    setup_game();
#if PLATFORM_WINDOWS
    if (month) {
        month = 0;
        start_year++;
    }
#else
    if (month) { month = 0; year++; }
#endif
    imperial_tax = 0;
    new_province();
#if PLATFORM_WINDOWS
    if (restart_flag) {
        windows_game_active = 1;
        start_windows_game();
    }
#else
    if (restart_flag) start_a_new_game();
#endif
}

// Initializes a province's map, population, armies, economy, ratings, and regional systems.
// FUNCTION: C2 0x10565
// FUNCTION: C2WIN 0x00443eca
void new_province(void)
{
    int skill;
    int denarii_reduction;

    setup_game();
    restart_flag = 0;
    setup_history_data();
    clear_citizen_list();
    clear_army_list();

    skill = c2inf.skill_level;
    auto_conquered = pompous_conquests[skill];
    auto_conquered_months = 0;
    if (skill <= 4) {
        pop_growth_future = 0x1c;
    }
    pop_growth_factor = pop_growth_future / 8;

    ind_growth_future   = 0;
    ind_growth_factor   = 0;
    insurrection_future = 0;
    insurrection_factor = 0;
    employment_rate     = 0;
    population          = 0;

    skill = c2inf.skill_level;
    denarii  = skill_to_starting_denarii[skill];
    denarii_reduction = skill_to_denarii_reduction[skill];
    denarii -= denarii_reduction * completed_provinces;

    income_multiple = 0x258;
    pop_tax_rate    = 5;
    ind_tax_rate    = 5;

    init_messages();
    init_flag_markers();
    act_choose_init_region();

    if (restart_flag != 0) return;

    clear_region_map();
    load_region_map(province_is);
    adjust_sailable_area();
    generate_city_map_geography();
    initiate_evolution();
    init_traders();
    init_region_trouble();
    set_new_province();
    init_slaves();
    init_legion();
    init_census();
    monthly_update();
    init_census();
    set_new_province();

    years_elapsed_in_region = 0;
    culture_rating          = 0;
    prosperity_rating       = 0;
    peace_rating            = 0;
    empire_rating           = 0;
    pax_romanum             = 0;
}

// Resets map, camera, command-window, placement, and cheat state for a game session.
// FUNCTION: C2 0x106bb
// FUNCTION: C2WIN 0x004440a8
void setup_game(void)
{
    pm_y       = 0x50;
    pm_x       = 0x28;
    city_pm_x  = 0x28;
    city_pm_y  = 0x50;
    zoom_level = 0;

    city_rotation   = 0;
    city_zoom_level = 0;
    prov_rotation   = 0;
    prov_zoom_level = 0;
    map_mode        = 0;
    in_the_forum    = 0;

    com_x = 0x1e0;
    com_y = 0x30;
    com_w = 0xa0;
    com_h = 0xa0;

    map_actual_width     = 0x50;
    map_actual_height    = 0x50;
    map_actual_atom      = 0x14;
    map_height_reduction = 0;
    map_width_reduction  = 0;

    get_pseudo_map(0);
    refresh_zoom_mode(zoom_level);
    load_map_graphics(map_mode,
                      zoom_level);

    ov_map_mode      = 0;
    ov2_map_mode     = 1;
    last_icon_used   = 4;
    promotion_cheat  = 0;
    housing_cheat    = 0;
    slave_warning    = 0;
    clear_highlight_goods_list();
    reg_placing_type  = 0;
    reg_placing_flags = 0;
    placing_type      = 0;
    placing_flags     = 0;
    pm_build_shape    = 0;
}

// Reloads the eight map graphics buffers for the selected map mode and zoom level.
// FUNCTION: C2 0x107db
// FUNCTION: C2WIN 0x0044421c
int load_map_graphics(int gfx_mode, int gfx_level)
{
    int   gfx_base_idx;
    int   i;
    int   file_size;
    char *filename;
    int   result;

    clear_map_gfx_buffers();
    init_map_gfx_buffers();
    clear_battle_gfx_buffers();
    init_battle_gfx_buffers();

    if (gfx_mode > 1) gfx_mode = 0;
    gfx_base_idx = gfx_mode * 24 + gfx_level * 8;

    for (i = 0; i < 8; i++) {
        file_size = c2_map_gfx[i + gfx_base_idx].size;
        filename = c2_map_gfx[i + gfx_base_idx].filename;

        if (file_size == 0) continue;

        if (i == 0) {
            people_data = malloc((unsigned)file_size);
            if (people_data == NULL) goto alloc_fail;
            if (!readfile(filename, people_data, file_size, 0)) goto file_fail;
        }
        else if (i == 1) {
            fixt_data = malloc((unsigned)file_size);
            if (fixt_data == NULL) goto alloc_fail;
            if (!readfile(filename, fixt_data, file_size, 0)) goto file_fail;
        }
        else if (i == 2) {
            house_data = malloc((unsigned)file_size);
            if (house_data == NULL) goto alloc_fail;
            if (!readfile(filename, house_data, file_size, 0)) goto file_fail;
        }
        else if (i == 3) {
            building_data1 = malloc((unsigned)file_size);
            if (building_data1 == NULL) goto alloc_fail;
            if (!readfile(filename, building_data1, file_size, 0)) goto file_fail;
        }
        else if (i == 4) {
            building_data2 = malloc((unsigned)file_size);
            if (building_data2 == NULL) goto alloc_fail;
            if (!readfile(filename, building_data2, file_size, 0)) goto file_fail;
        }
        else if (i == 5) {
            building_data3 = malloc((unsigned)file_size);
            if (building_data3 == NULL) goto alloc_fail;
            if (!readfile(filename, building_data3, file_size, 0)) goto file_fail;
        }
        else if (i == 6) {
            building_data4 = malloc((unsigned)file_size);
            if (building_data4 == NULL) goto alloc_fail;
            if (!readfile(filename, building_data4, file_size, 0)) goto file_fail;
        }
        else if (i == 7) {
            tops_data = malloc((unsigned)file_size);
            if (tops_data == NULL) goto alloc_fail;
            if (!readfile(filename, tops_data, file_size, 0)) goto file_fail;
        }
    }

    result = 1;
    goto done;

file_fail:
    stop_system();
    printf("\nError loading graphics data - code %d - file not found.\n",
           i + gfx_base_idx);
    exit(100);

alloc_fail:
    stop_system();
    printf("\nError loading graphics data - code %d  - cannot allocate memory.\n",
           i + gfx_base_idx);
    exit(100);

done:
    return result;
}

// Reloads circus sprites for a populous city, alternating the graphics by year.
// FUNCTION: C2 0x10944
// FUNCTION: C2WIN 0x0044474a
void swap_circus_gfx(void)
{
    if (population < 2000) return;
    if (map_mode != 0)     return;
    if (game_state == 3)   return;
    if (game_state == 1)   return;
    if (game_state == 2)   return;

    if (zoom_level == 0) {
        if (year & 1)
            readfile("build1f.pl8", building_data4, 0x1ad20, 0);
        else
            readfile("build1d.pl8", building_data4, 0x1ad20, 0);
    } else if (zoom_level == 1) {
        if (year & 1)
            readfile("build2f.pl8", building_data4, 0x6590, 0);
        else
            readfile("build2d.pl8", building_data4, 0x6590, 0);
    } else if (zoom_level == 2) {
        if (year & 1)
            readfile("build3f.pl8", building_data4, 0x171e, 0);
        else
            readfile("build3d.pl8", building_data4, 0x171e, 0);
    }
}

// Loads the people or overlay graphics for the current zoom level into people_data.
// FUNCTION: C2 0x10a40
// FUNCTION: C2WIN 0x004448fb
#if PLATFORM_WINDOWS
int load_overlay_graphics(int use_overlay)
{
    int   gfx_idx;
    int   result;
    int   map_kind;
    char *filename;
    int   size;

    map_kind = map_mode;
    if (map_kind > 1) map_kind = 0;

    if (use_overlay == 0) {
        gfx_idx = zoom_level * 8;
        size = c2_map_gfx[gfx_idx].size;
        filename = c2_map_gfx[gfx_idx].filename;
    } else {
        size = c2_overlay_gfx[zoom_level].size;
        filename = c2_overlay_gfx[zoom_level].filename;
    }

    if (!readfile(filename, (&people_data)[map_kind], size, 0)) goto file_error;
    return 1;

file_error:
    stop_system();
    printf("\nError loading overlay data - file not found.\n");
    close_windows();
    exit(100);
}
#else
int load_overlay_graphics(int use_overlay)
{
    int   file_size;
    char *filename;
    int   gfx_idx;
    int   result;

    if (use_overlay == 0) {
        gfx_idx = zoom_level * 8;
        file_size = c2_map_gfx[gfx_idx].size;
        filename = c2_map_gfx[gfx_idx].filename;
    } else {
        gfx_idx = zoom_level; file_size = c2_overlay_gfx[gfx_idx].size;
        filename = c2_overlay_gfx[gfx_idx].filename;
    }

    if (readfile(filename, people_data, file_size, 0)) {
        result = 1; } else {
        stop_system();
        printf("\nError loading overlay data - file not found.\n");
        exit(100);
    }
    return result;
}
#endif

// Reloads terrain, troop, and optional mercenary graphics for the active battle.
// FUNCTION: C2 0x10ac9
// FUNCTION: C2WIN 0x004449df
int load_battle_graphics(int battle_zoom)
{
    int troop_gfx_idx;
    int mercenary_gfx_idx;

    clear_map_gfx_buffers();
    init_map_gfx_buffers();
    clear_battle_gfx_buffers();
    init_battle_gfx_buffers();

    troop_gfx_idx = tribe_to_troops[
        army_list[their_battle_army].tribe_id];

    fixt_data    = load_a_battle_gfx_file(battle_zoom, 0, 0);
    figure1_data = load_a_battle_gfx_file(battle_zoom, 1, 0);
    figure2_data = load_a_battle_gfx_file(battle_zoom, 2, 0);
    figure3_data = load_a_battle_gfx_file(battle_zoom, 3, 0);
    figure4_data = load_a_battle_gfx_file(battle_zoom, troop_gfx_idx, 0);
    figure5_data = load_a_battle_gfx_file(battle_zoom, troop_gfx_idx + 1, 0);
    figure6_data = load_a_battle_gfx_file(battle_zoom, troop_gfx_idx + 2, 0);

    if (max_mercs_allowed != 0) {
        troop_gfx_idx = tribe_to_troops[mercs_tribe];
        mercenary_gfx_idx = troop_gfx_idx + 1;

        if (mercs_catagory == 0) {
            figure7_data = load_a_battle_gfx_file(battle_zoom, troop_gfx_idx, 1);
            figure8_data = load_a_battle_gfx_file(battle_zoom, mercenary_gfx_idx, 1);
        } else if (mercs_catagory == 1) {
            figure7_data = load_a_battle_gfx_file(battle_zoom, troop_gfx_idx, 1);
        } else if (mercs_catagory == 2) {
            figure7_data = load_a_battle_gfx_file(battle_zoom, mercenary_gfx_idx, 1);
        } else if (mercs_catagory == 3) {
            figure7_data = load_a_battle_gfx_file(battle_zoom, troop_gfx_idx + 2, 1);
        }
    }

    return 1;
}

// Allocates and loads one troop graphics file from the primary or auxiliary battle table.
// FUNCTION: C2 0x10c03
// FUNCTION: C2WIN 0x00444cf6
void *load_a_battle_gfx_file(int battle_zoom, int troop_gfx_idx, int use_aux)
{
    int    gfx_idx;
    int    size;
    char  *filename;
    void  *data;

#if PLATFORM_WINDOWS
    gfx_idx = (battle_zoom - 1) * 34;
    if (use_aux) {
        size = c2_battle_aux_gfx[gfx_idx + troop_gfx_idx].size;
        filename = c2_battle_aux_gfx[gfx_idx + troop_gfx_idx].filename;
    } else {
        size = c2_battle_gfx[gfx_idx + troop_gfx_idx].size;
        filename = c2_battle_gfx[gfx_idx + troop_gfx_idx].filename;
    }
#else
    battle_zoom--; gfx_idx = battle_zoom * 34 + troop_gfx_idx;
    if (use_aux) {
        size = c2_battle_aux_gfx[gfx_idx].size;
        filename = c2_battle_aux_gfx[gfx_idx].filename;
    } else {
        size = c2_battle_gfx[gfx_idx].size;
        filename = c2_battle_gfx[gfx_idx].filename;
    }
#endif

    if (size == 0) return NULL;

    data = malloc((unsigned)size);
    if (!data) {
        stop_system();
        printf("\nError loading battle data - code %d - cannot allocate memory.\n",
               troop_gfx_idx);
#if PLATFORM_WINDOWS
        close_windows();
#endif
        exit(100);
    }

    if (!readfile(filename, data, size, 0)) {
        stop_system();
        if (size == 0) printf("\nError loading battle data - 0 sized file.\n");
        else printf("\nError loading battle data - %s not found.\n", filename);
#if PLATFORM_WINDOWS
        close_windows();
#endif
        exit(100);
    }

    return data;
}

// Marks all map graphics buffers as empty.
// FUNCTION: C2 0x10cb9
// FUNCTION: C2WIN 0x00444e27
#if PLATFORM_WINDOWS
void init_map_gfx_buffers(int mode)
{
    if (mode > 1) mode = 0;

    if ((&people_data)[mode]) free((&people_data)[mode]);
    (&people_data)[mode] = 0;
    if ((&fixt_data)[mode]) free((&fixt_data)[mode]);
    (&fixt_data)[mode] = 0;
    if ((&house_data)[mode]) free((&house_data)[mode]);
    (&house_data)[mode] = 0;
    if ((&building_data1)[mode]) free((&building_data1)[mode]);
    (&building_data1)[mode] = 0;
    if ((&building_data2)[mode]) free((&building_data2)[mode]);
    (&building_data2)[mode] = 0;
    if ((&building_data3)[mode]) free((&building_data3)[mode]);
    (&building_data3)[mode] = 0;
    if ((&building_data4)[mode]) free((&building_data4)[mode]);
    (&building_data4)[mode] = 0;
    if ((&tops_data)[mode]) free((&tops_data)[mode]);
    (&tops_data)[mode] = 0;
}
#else
void init_map_gfx_buffers(void)
{
    people_data    = 0;
    fixt_data      = 0;
    house_data     = 0;
    building_data1 = 0;
    building_data2 = 0;
    building_data3 = 0;
    building_data4 = 0;
    tops_data      = 0;
}
#endif

// Releases all allocated map graphics buffers.
// FUNCTION: C2 0x10cee
// FUNCTION: C2WIN 0x00444fd3
#if PLATFORM_WINDOWS
void clear_map_gfx_buffers(unsigned char mode)
{
    int map_kind;

    map_kind = mode;
    if (map_kind > 1) map_kind = 0;

    if (map_kind == 0) {
        city_gfx_loaded = 0;
        city_gfx_zoom = 0xff;
    } else if (map_kind == 1) {
        province_gfx_loaded = 0;
        province_gfx_zoom = 0xff;
    }

    if ((&people_data)[map_kind]) {
        free((&people_data)[map_kind]);
        (&people_data)[map_kind] = 0;
    }
    if ((&fixt_data)[map_kind]) {
        free((&fixt_data)[map_kind]);
        (&fixt_data)[map_kind] = 0;
    }
    if ((&house_data)[map_kind]) {
        free((&house_data)[map_kind]);
        (&house_data)[map_kind] = 0;
    }
    if ((&building_data1)[map_kind]) {
        free((&building_data1)[map_kind]);
        (&building_data1)[map_kind] = 0;
    }
    if ((&building_data2)[map_kind]) {
        free((&building_data2)[map_kind]);
        (&building_data2)[map_kind] = 0;
    }
    if ((&building_data3)[map_kind]) {
        free((&building_data3)[map_kind]);
        (&building_data3)[map_kind] = 0;
    }
    if ((&building_data4)[map_kind]) {
        free((&building_data4)[map_kind]);
        (&building_data4)[map_kind] = 0;
    }
    if ((&tops_data)[map_kind]) {
        free((&tops_data)[map_kind]);
        (&tops_data)[map_kind] = 0;
    }
}
#else
void clear_map_gfx_buffers(void)
{
    if (people_data)    free(people_data);
    if (fixt_data)      free(fixt_data);
    if (house_data)     free(house_data);
    if (building_data1) free(building_data1);
    if (building_data2) free(building_data2);
    if (building_data3) free(building_data3);
    if (building_data4) free(building_data4);
    if (tops_data)      free(tops_data);
}
#endif

// Marks all battle graphics buffers as empty.
// FUNCTION: C2 0x10d80
// FUNCTION: C2WIN 0x004451bf
#if PLATFORM_WINDOWS
void init_battle_gfx_buffers(void)
{
    if (fixt_data) free(fixt_data);
    fixt_data = 0;
    if (figure1_data) free(figure1_data);
    figure1_data = 0;
    if (figure2_data) free(figure2_data);
    figure2_data = 0;
    if (figure3_data) free(figure3_data);
    figure3_data = 0;
    if (figure4_data) free(figure4_data);
    figure4_data = 0;
    if (figure5_data) free(figure5_data);
    figure5_data = 0;
    if (figure6_data) free(figure6_data);
    figure6_data = 0;
    if (figure7_data) free(figure7_data);
    figure7_data = 0;
    if (figure8_data) free(figure8_data);
    figure8_data = 0;
    if (figure9_data) free(figure9_data);
    figure9_data = 0;
    if (figure10_data) free(figure10_data);
    figure10_data = 0;
    city_gfx_loaded = 0;
    city_gfx_zoom = 0xff;
}
#else
void init_battle_gfx_buffers(void)
{
    fixt_data     = 0;
    figure1_data  = 0;
    figure2_data  = 0;
    figure3_data  = 0;
    figure4_data  = 0;
    figure5_data  = 0;
    figure6_data  = 0;
    figure7_data  = 0;
    figure8_data  = 0;
    figure9_data  = 0;
    figure10_data = 0;
}
#endif

// Releases all allocated battle graphics buffers.
// FUNCTION: C2 0x10dc7
// FUNCTION: C2WIN 0x0044536f
#if PLATFORM_WINDOWS
void clear_battle_gfx_buffers(void)
{
    if (fixt_data) {
        free(fixt_data);
        fixt_data = 0;
    }
    if (figure1_data) {
        free(figure1_data);
        figure1_data = 0;
    }
    if (figure2_data) {
        free(figure2_data);
        figure2_data = 0;
    }
    if (figure3_data) {
        free(figure3_data);
        figure3_data = 0;
    }
    if (figure4_data) {
        free(figure4_data);
        figure4_data = 0;
    }
    if (figure5_data) {
        free(figure5_data);
        figure5_data = 0;
    }
    if (figure6_data) {
        free(figure6_data);
        figure6_data = 0;
    }
    if (figure7_data) {
        free(figure7_data);
        figure7_data = 0;
    }
    if (figure8_data) {
        free(figure8_data);
        figure8_data = 0;
    }
    if (figure9_data) {
        free(figure9_data);
        figure9_data = 0;
    }
    if (figure10_data) {
        free(figure10_data);
        figure10_data = 0;
    }
    city_gfx_loaded = 0;
    city_gfx_zoom = 0xff;
}
#else
void clear_battle_gfx_buffers(void)
{
    if (fixt_data)     free(fixt_data);
    if (figure1_data)  free(figure1_data);
    if (figure2_data)  free(figure2_data);
    if (figure3_data)  free(figure3_data);
    if (figure4_data)  free(figure4_data);
    if (figure5_data)  free(figure5_data);
    if (figure6_data)  free(figure6_data);
    if (figure7_data)  free(figure7_data);
    if (figure8_data)  free(figure8_data);
    if (figure9_data)  free(figure9_data);
    if (figure10_data) free(figure10_data);
}
#endif

// Loads boot-time graphics, text, palettes, and interface data into their fixed buffers.
// Returns zero on success or the one-based index of the first file that failed.
// FUNCTION: C2 0x10e89
// FUNCTION: C2WIN 0x0044551f
int load_start_graphics(void)
{
    if (!readfile("cityfixt.256", city_palette,        0x300,  0)) return 1;
    if (!readfile("landfill.pl8", landfill,            0x1540, 0)) return 2;
    if (!readfile("font_c2.pl8", font1,                0x24f4, 0)) return 3;
    if (!readfile("font3c2.pl8", font2,                0x6e58, 0)) return 4;
    if (!readfile("mouse.pl8", mice,                   0x21b6, 0)) return 5;
    if (!readfile("system.pl8", system_panel,                  0xa2c8, 0)) return 6;
    if (!readfile("panels.pl8", game_panels,           0x5b91, 0)) return 7;
    if (!readfile("smacker.pl8", logos,                0x14e0, 0)) return 8;
    if (!readfile("misc.pl8", misc,                    0xe00,  0)) return 9;
    if (!readfile("c2.eng", text_buffer,               0x9c40, 0)) return 10;
    if (!readfile("int_city.pl8", int_city_header,     0x1c8,  0)) return 11;
    if (!readfile("provfixt.256", region_palette,      0x300,  0)) return 12;
    if (!readfile("int_prov.pl8", int_region_header,   0x1c8,  0)) return 13;
    if (!readfile("int_batl.pl8", int_battle_header,   0x1c8,  0)) return 14;
    return 0;
}

// Marks both map undo buffers as flushed.
// FUNCTION: C2 0x1107c
// FUNCTION: C2WIN 0x0044578b
void flush_sb_buffer(void)
{
    sb_cm_undo_flushed = 1;
    sb_rm_undo_flushed = 1;
}

// Plays the positive-action feedback sound.
// FUNCTION: C2 0x1108b
// FUNCTION: C2WIN 0x004457a4
void do_pos(void)
{
    pos_sound();
}

// Plays the negative-action feedback sound.
// FUNCTION: C2 0x11090
// FUNCTION: C2WIN 0x004457b4
void do_neg(void)
{
    neg_sound();
}

// Checks the configured CD drive for cd.dat and restores the startup drive and path.
// Returns zero on success or an error code identifying the failed step.
// FUNCTION: C2 0x11095
// FUNCTION: C2WIN 0x004457c4
#if PLATFORM_WINDOWS
int test_cd_drive(void)
{
    char  cdrive;
    int   drive;
    int   no_of_drives;
    int   file;
    int   result;
    int   error;
    char *root_path = "c:\\";
    int   current_drive;

    error = 0;
    if (c2inf.cd_letter < 'A') {
        error = 1;
    } else {
        drive = c2inf.cd_letter - 0x40;
        _chdrive(drive);
        current_drive = _getdrive();
        if (current_drive != drive) {
            error = 2;
        } else {
            cdrive = c2inf.cd_letter;
            root_path[0] = cdrive;
            result = chdir(root_path);
            if (result != 0) error = 3;

            getcwd(path_name, 0x50);
            file = open("cd.dat", O_BINARY);
            if (file >= 0) close(file);
            else error = 4;

            drive = drive_name - 0x40;
            _chdrive(drive);
            current_drive = _getdrive();
            if (current_drive != drive) error = 5;
            result = chdir(path_name);
            if (result != 0) error = 6;
        }
    }
    return error;
}
#else
int test_cd_drive(void)
{
    int            error_code;
    int            drive;
    int            drive_count;
    int            current_drive;
    int            cd_fd;
    int            result;
    char           cdrive;
    char          *cd_root = "c:\\";

    error_code = 0;

    if (c2inf.cd_letter < 'A') { error_code = 1;
        goto end;
    }

    drive = c2inf.cd_letter - 0x40;
    _dos_setdrive((unsigned)drive, (unsigned *)&drive_count);
    _dos_getdrive((unsigned *)&current_drive);
    if (drive != current_drive) { error_code = 2;
        goto end;
    }

    cdrive = c2inf.cd_letter;
    cd_root[0] = cdrive;
    result = chdir(cd_root);
    if (result != 0) error_code = 3;

    getcwd(path_name, 0x50);

    cd_fd = open("cd.dat", O_BINARY); if (cd_fd >= 0) close(cd_fd);
    else error_code = 4;

    drive = drive_name - 0x40;
    _dos_setdrive((unsigned)drive, (unsigned *)&drive_count);
    _dos_getdrive((unsigned *)&current_drive);
    if (drive != current_drive) error_code = 5;

    result = chdir(path_name);
    if (result != 0) error_code = 6;

end:
    return error_code;
}
#endif
