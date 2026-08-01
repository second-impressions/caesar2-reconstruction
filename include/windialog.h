#ifndef WINDIALOG_H
#define WINDIALOG_H

#if PLATFORM_WINDOWS
void show_native_message(void *window, char *message);
int show_native_confirm(void *window, char *message, char *yes_text,
                        char *no_text);
int show_native_year_end(void *window);
void show_native_census(void *window);
#endif

#endif
