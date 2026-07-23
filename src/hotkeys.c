
#include "c2_data.h"

#include <io.h>          /* open, close, write            */
#include <fcntl.h>       /* O_BINARY / O_TRUNC / O_CREAT  */
#include <sys/stat.h>    /* S_IRUSR, S_IWUSR              */

// Backing buffer for the eight-key debug-cheat ring.
char *old_key_buffer = "        ";

int LBM_HEADER1[12] = { 1297239878, 817038336, 541934160, 1145589058, 335544320, -536772606, 0, 8, 16908032, -536772606, 1346456899, 196608 };

int LBM_HEADER2[2] = { 1497648962, 11535360 };

int LBM_PADDING = 0;
#ifndef S_IRUSR          /* MSVC's <sys/stat.h> lacks the POSIX _USR names. */
#define S_IRUSR 0400
#define S_IWUSR 0200
#endif

extern void __cdecl code_01871D(void);
extern void __cdecl code_018738(void);
extern void __cdecl code_0187A9(void);
extern void __cdecl code_0187BF(void);
/* Forward declarations (functions defined later in this file). */
void capture_shot(char *filename);


// Translate keyboard shortcuts into the same actions as mouse/UI input.
// FUNCTION: C2 0x2881b
unsigned char sim_mouse(void)
{
    int i;
    int one;

    get_key();
    if (key_ready != 1) {
        hold_hot_keys = 0;
        return 0;
    }
    if (1 < turbo_mode) {
        act_exit_turbo_mode();
        return 1;
    }
    if (hold_hot_keys != 0) {
        hold_hot_keys = 0;
        return 1;
    }

    for (i = 7; i > 0; i--) {
        old_key_buffer[i] = old_key_buffer[i - 1];
    }
    old_key_buffer[0] = key_ascii + 3;

    switch (key_ascii) {
    case ';':
        mse_button |= 1;
        break;
    case 0x0d:
        if (cursor_y != 0) {
            cursor_y = 0;
        } else {
            mse_button |= 2;
            if (confirming != 0) {
                decision = 0;
                out1 = 1;
            }
        }
        break;
    case '+':
    case '=':
        if (map_mode == 2) act_zoom_level1();
        else if (zoom_level != 0) {
            pm_x_coord = pm_screen_width / 2;
            pm_y_coord = pm_screen_height / 2;
            do_act_zoom_in(0);
        }
        break;
    case '-':
        if (map_mode == 2) act_zoom_level2();
        else act_zoom_out();
        break;
    case '1':
        if (in_the_forum == 0) {
            if (map_mode == 2) act_zoom_level1();
            else do_act_zoom_in(1);
        }
        break;
    case '2':
        if (in_the_forum == 0) {
            if (map_mode == 2) act_zoom_level1();
            else if (zoom_level == 0) act_zoom_out();
            else do_act_zoom_in(0);
        }
        break;
    case '3':
        if (in_the_forum == 0) {
            if (map_mode == 2) act_zoom_level2();
            else do_act_zoom_out(1);
        }
        break;
    case ',':
    case '<':
        if (in_the_forum == 0) act_rotate_anticlockwise();
        break;
    case '.':
    case '>':
        if (in_the_forum == 0) act_rotate_clockwise();
        break;
    case 0:
        switch (key_code) {
        case 0x48:
            mse_y -= 8;
            set_mouse();
            break;
        case 0x50:
            mse_y += 8;
            set_mouse();
            break;
        case 0x4b:
            mse_x -= 8;
            set_mouse();
            break;
        case 0x4d:
            mse_x += 8;
            set_mouse();
            break;
        case 0x3b:
            if (map_mode != 2) {
                map_mode = 0;
                act_correct_map();
                out1 = 1;
            }
            break;
        case 0x3c:
            if (map_mode != 2 && in_the_forum == 0) act_forum();
            break;
        case 0x3d:
            if (map_mode != 2) {
                map_mode = 1;
                act_correct_map();
                out1 = 1;
            }
            break;
        case 0x3e:
            if (in_the_forum == 0) {
                if (tutorial_mode == 0) load_a_game();
                if (pre_loaded_status == 2) black_out();
            }
            break;
        case 0x3f:
            if (in_the_forum == 0 && tutorial_mode == 0) save_a_game();
            break;
        case 0x21:
            if (in_the_forum == 0 && map_mode != 2) act_goto_flags();
            break;
        case 0x68:
            if (in_the_forum == 0 && map_mode != 2) act_set_marker1();
            break;
        case 0x6a:
            if (in_the_forum == 0 && map_mode != 2) act_set_marker2();
            break;
        case 0x20:
            if (in_the_forum == 0 && map_mode != 2) act_set_marker3();
            break;
        case 0x2d:
            hot_exit_flag = 1;
            out1 = 1;
            out2 = 1;
            out3 = 1;
            out4 = 1;
            out5 = 1;
            out6 = 1;
            out7 = 1;
            out8 = 1;
            break;
        case 0x78: capture_shot("shot1.lbm"); break;
        case 0x79: capture_shot("shot2.lbm"); break;
        case 0x7a: capture_shot("shot3.lbm"); break;
        case 0x7b: capture_shot("shot4.lbm"); break;
        case 0x7c: capture_shot("shot5.lbm"); break;
        case 0x7d: capture_shot("shot6.lbm"); break;
        case 0x7e: capture_shot("shot7.lbm"); break;
        case 0x7f: capture_shot("shot8.lbm"); break;
        }
        break;
    case 'y':
    case 'Y':
        decision = 1;
        out1 = 1;
        out2 = 1;
        break;
    case 0x1b:
        decision = 0;
        out1 = 1;
        out2 = 1;
        out3 = 1;
        out4 = 1;
        break;
    case 'n':
    case 'N':
        decision = 0;
        out1 = 1;
        out2 = 1;
        break;
    case 'p':
    case 'P':
        if (in_the_forum == 0) {
            if (map_mode == 2) act_stop_go();
            else c2inf.paused ^= 1;
            update_map = 1;
            setup_whole_screen_refresh();
        }
        break;
    case 'f':
    case 'F':
        if (in_the_forum == 0 && map_mode != 2) act_forum();
        break;
    case 'c':
    case 'C':
        if (in_the_forum == 0 && map_mode != 2) {
            if (in_census_mode != 0) {
                out1 = 1;
            } else {
                act_census();
            }
        }
        break;
    case 'a':
    case 'A':
        if (in_the_forum == 0 && map_mode != 2) {
            c2inf.paused = 0;
            act_init_turbo_mode();
        }
        break;
    case ' ':
        hot_key_out_off_build = 1;
        mse_button |= 1;
        if (test_mode4 != 0) debug_screen();
        break;
    }
    /*
     * Cheat chords are matched newest-first after adding three to each key.
     * They toggle the four test modes, grant denarii, or enable promotions.
     */
    if (strcmp(old_key_buffer, "GE78DuhQ") == 0) test_mode1 ^= 1;
    if (strcmp(old_key_buffer, "GE78HuhQ") == 0) test_mode2 ^= 1;
    if (strcmp(old_key_buffer, "GE78LuhQ") == 0) test_mode3 ^= 1;
    if (strcmp(old_key_buffer, "GE78RuhQ") == 0) test_mode4 ^= 1;
    if (strcmp(old_key_buffer, "WIVqNSPS") == 0) {
        if (last_forum_dept == FORUM_DEPT_INDUSTRY && denarii < 100000) {
            denarii += 10000;
        }
    }
    if (strcmp(old_key_buffer, "eoxJloef") == 0) {
        promotion_cheat = 1;
    }
    return 1;
}

// Captures the current screen to an image file.
// FUNCTION: C2 0x28e13
void capture_shot(char *filename)
{
    int screenshot_fd;

    go_16m_palette(&current_palette);
    screenshot_fd = open(filename,
              O_BINARY | O_TRUNC | O_CREAT | O_WRONLY,
              S_IRUSR | S_IWUSR);
    if (screenshot_fd != -1) {
        write(screenshot_fd, &LBM_HEADER1, 0x30);
        write(screenshot_fd, &current_palette, 0x300);
        write(screenshot_fd, &LBM_HEADER2, 8);
        write(screenshot_fd, internal_screen, 0x4b000);
        close(screenshot_fd);
        go_64k_palette(&current_palette);
    }
}
