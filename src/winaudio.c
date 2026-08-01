#include "winaudio.h"

#if PLATFORM_WINDOWS

extern int cityprov_tune_count;
extern unsigned char cityprov_tune;
extern int forum_tune_count;
extern unsigned char forum_tune;
extern int battle_tune_count;
extern unsigned char battle_tune;
extern int streamed_sample;
extern int streamed_tune_playing;
extern int streamed_tune_file;
extern int cityprov_tune_position;
extern unsigned char tune_thread_running;
extern int application_active;
extern unsigned char streamed_tune_ready;
extern unsigned char streamed_tune_inhibit;

long _tell(int handle);
void play_next_tune(void);
extern void (__stdcall *Sleep)(unsigned long milliseconds);

// FUNCTION: C2WIN 0x0043303a
void set_cityprov_tune(unsigned char tune)
{
    if (tune < 0) {
        cityprov_tune = tune;
    } else if (tune >= cityprov_tune_count) {
        tune = cityprov_tune_count - 1;
    } else {
        cityprov_tune = tune;
    }
}

// FUNCTION: C2WIN 0x004330fa
void pause_cityprov_tune(void)
{
    cityprov_tune_position = 0;
    if (streamed_sample == 0) return;
    if (streamed_tune_playing == 0) return;
    if (streamed_tune_file == -1) return;
    cityprov_tune_position = _tell(streamed_tune_file);
    if (cityprov_tune_position == -1) {
        cityprov_tune_position = 0;
    }
}

// FUNCTION: C2WIN 0x0043320d
void set_forum_tune(unsigned char tune)
{
    if (tune < 0) {
        forum_tune = tune;
    } else if (tune >= forum_tune_count) {
        tune = forum_tune_count - 1;
    } else {
        forum_tune = tune;
    }
}

// FUNCTION: C2WIN 0x0043336a
void set_battle_tune(unsigned char tune)
{
    if (tune < 0 || tune >= battle_tune_count) {
    } else {
        battle_tune = tune;
    }
}

// FUNCTION: C2WIN 0x0043398c
int CallbackProc(void)
{
    while (tune_thread_running != 0) {
        if (application_active == 1 &&
            streamed_tune_ready == 1 &&
            streamed_tune_inhibit == 0) {
            play_next_tune();
        }
        Sleep(50);
    }
    return 0;
}

#endif
