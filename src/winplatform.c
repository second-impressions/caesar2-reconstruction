#include "c2_data.h"

#if PLATFORM_WINDOWS

extern unsigned char window_mouse_state_1;
extern unsigned char window_mouse_state_2;
extern unsigned char window_mouse_state_3;
extern unsigned char window_mouse_state_4;
extern unsigned char window_mouse_state_5;
extern unsigned char window_mouse_state_6;

// FUNCTION: C2WIN 0x0041dbab
void clear_mouse_input(void)
{
    window_mouse_state_1 = 0;
    window_mouse_state_3 = window_mouse_state_2 = 0;
    window_mouse_state_5 = window_mouse_state_4 = 0;
    mouse_left_preclick = window_mouse_state_6 = 0;
}

#endif
