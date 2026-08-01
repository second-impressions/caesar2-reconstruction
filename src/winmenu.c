#include "c2_target.h"

#if PLATFORM_WINDOWS

extern void *main_window;
extern void *(__stdcall *GetMenu)(void *window);
extern int (__stdcall *EnableMenuItem)(void *menu, unsigned int item,
                                      unsigned int flags);
extern int (__stdcall *DrawMenuBar)(void *window);

// FUNCTION: C2WIN 0x004b0278
void set_main_menu_enabled(unsigned char enabled)
{
    void *menu;

    menu = GetMenu(main_window);
    if (enabled == 0) {
        EnableMenuItem(menu, 0, 0x401);
        EnableMenuItem(menu, 1, 0x401);
        EnableMenuItem(menu, 2, 0x401);
        EnableMenuItem(menu, 3, 0x401);
        EnableMenuItem(menu, 4, 0x401);
    } else if (enabled == 1) {
        EnableMenuItem(menu, 0, 0x400);
        EnableMenuItem(menu, 1, 0x400);
        EnableMenuItem(menu, 2, 0x400);
        EnableMenuItem(menu, 3, 0x400);
        EnableMenuItem(menu, 4, 0x400);
    }
    DrawMenuBar(main_window);
}

#endif
