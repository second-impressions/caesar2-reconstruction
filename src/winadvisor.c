#include "c2_target.h"
#include "windialog.h"

#if PLATFORM_WINDOWS

extern void *application_instance;
extern int flag_mode;
extern int (__stdcall *DialogBoxParamA)(void *instance, char *name,
                                       void *parent, void *dialog_proc,
                                       int parameter);

void set_modal_window_state(int active);
long __stdcall annual_dialog_proc(void *window, unsigned int message,
                                  unsigned int wparam, long lparam);

// FUNCTION: C2WIN 0x0049a9e7
int show_native_year_end(void *window)
{
    int result;

    set_modal_window_state(0);
    result = DialogBoxParamA(application_instance, "ANNUAL", window,
                             annual_dialog_proc, 0);
    if (flag_mode != 0) {
        flag_mode = 0;
    }
    clear_mouse_input();
    set_modal_window_state(1);
    return result;
}

#endif
