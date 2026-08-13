#include "c2_data.h"

#if PLATFORM_WINDOWS

extern unsigned char window_mouse_state_1;
extern unsigned char window_mouse_state_2;
extern unsigned char window_mouse_state_3;
extern unsigned char window_mouse_state_4;
extern unsigned char window_mouse_state_5;
extern unsigned char window_mouse_state_6;
extern void *main_window;
extern unsigned int window_timer;

extern int (__stdcall *KillTimer)(void *window, unsigned int timer);

void close_window_graphics(void);
void free_window_images(void);
void free_window_image_list(void);
void delete_window_fonts(void);

// FUNCTION: C2WIN 0x0043a1ad
void close_windows(void)
{
    int i;

    if (window_timer != 0) {
        KillTimer(main_window, window_timer);
        window_timer = 0;
    }
    close_window_graphics();
    free_window_images();
    free_window_image_list();
    delete_window_fonts();
}

// FUNCTION: C2WIN 0x0041dbab
void clear_mouse_input(void)
{
    window_mouse_state_1 = 0;
    window_mouse_state_3 = window_mouse_state_2 = 0;
    window_mouse_state_5 = window_mouse_state_4 = 0;
    mouse_left_preclick = window_mouse_state_6 = 0;
}

int prev_mode;
int icon_over;
int bs_type;
int bs_morale;
int bs_men;
int bs_nof_units;
int bt_their_men;
int bt_our_men;
int bt_their_morale;
int bt_our_morale;
int old_paused;
int tribune_flag_counter;
int old_population;
int old_cost;

#endif
