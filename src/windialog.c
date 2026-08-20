#include "windialog.h"

#if PLATFORM_WINDOWS

extern void *application_instance;
extern int flag_mode;
extern char *native_dialog_message;
extern char *native_dialog_yes_text;
extern char *native_dialog_no_text;

extern int (__stdcall *DialogBoxParamA)(void *instance, char *name,
                                       void *parent, void *dialog_proc,
                                       int parameter);

void set_modal_window_state(int active);
long __stdcall message_dialog_proc(void *window, unsigned int message,
                                   unsigned int wparam, long lparam);
long __stdcall confirm_dialog_proc(void *window, unsigned int message,
                                   unsigned int wparam, long lparam);
long __stdcall year_end_dialog_proc(void *window, unsigned int message,
                                    unsigned int wparam, long lparam);
long __stdcall census_dialog_proc(void *window, unsigned int message,
                                  unsigned int wparam, long lparam);

// FUNCTION: C2WIN 0x004880df
void show_native_message(void *window, char *message)
{
    char text[24];

    set_modal_window_state(0);
    native_dialog_message = message;
    DialogBoxParamA(application_instance, "MESSAGE", window,
                    message_dialog_proc, 0);
    if (flag_mode != 0) {
        flag_mode = 0;
    }
    clear_mouse_input();
    set_modal_window_state(1);
}

// FUNCTION: C2WIN 0x004883f6
int show_native_confirm(void *window, char *message, char *yes_text,
                        char *no_text)
{
    int result;

    set_modal_window_state(0);
    native_dialog_message = message;
    native_dialog_yes_text = yes_text;
    native_dialog_no_text = no_text;
    result = DialogBoxParamA(application_instance, "OK_CANCEL", window,
                             confirm_dialog_proc, 0);
    if (flag_mode != 0) {
        flag_mode = 0;
    }
    clear_mouse_input();
    set_modal_window_state(1);
    return result;
}

// FUNCTION: C2WIN 0x0048a1d5
void show_native_year_end_options(void *window)
{
    int result;

    set_modal_window_state(0);
    result = DialogBoxParamA(application_instance, "YEAREND", window,
                             year_end_dialog_proc, 0);
    if (flag_mode != 0) {
        flag_mode = 0;
    }
    clear_mouse_input();
    set_modal_window_state(1);
}

// FUNCTION: C2WIN 0x0048a4c1
void show_native_census(void *window)
{
    int result;

    set_modal_window_state(0);
    result = DialogBoxParamA(application_instance, "CENSUS", window,
                             census_dialog_proc, 0);
    if (flag_mode != 0) {
        flag_mode = 0;
    }
    clear_mouse_input();
    set_modal_window_state(1);
}

#endif
