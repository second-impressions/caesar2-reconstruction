#ifndef WINAUDIO_H
#define WINAUDIO_H

#if PLATFORM_WINDOWS
void set_cityprov_tune(unsigned char tune);
void pause_cityprov_tune(void);
void set_forum_tune(unsigned char tune);
void set_battle_tune(unsigned char tune);
int CallbackProc(void);
#endif

#endif
