#include "lib32.h"
#include "c2_data.h"
#include <conio.h>             /* inp(), outpw() */
#if PLATFORM_DOS
#include <i86.h>              /* int386, union REGS, sound/nosound/delay */
#else
#include <windows.h>          /* Beep */
#endif
#include <io.h>                /* open, close, read, write */
#include <fcntl.h>             /* O_BINARY */
#include <stdlib.h>            /* free, malloc */
#include <string.h>            /* memset */
#include <dos.h>               /* _dos_setdrive */
#include <direct.h>            /* chdir */
#include <sys/timeb.h>         /* ftime, struct timeb */

struct mouse_cbd cbd = { 0, 0, 0, 0, 0, 0, 0, 0 };
int steves_security_false1[7] = {
    538976288, 2021138464, 2021161080, 1717986936,
    1717986918, 538994278, 538976288 };
char *chipset_names[32] = {
    "Unknown", "Trident", "Tseng", "", "", "", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", ""
};
int multiples[8] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000 };

struct vbe_mode_info vesa_mode_info;
char card_ids[32];
unsigned char greying_data[256];
char mouse_background[576];
struct vbe_info_block vesa_info;
char black_out_data[768];
unsigned char current_palette[768];
char temp_palette[768];
unsigned char logos[5344];
unsigned char font2[28248];
unsigned char system_panel[41672];
unsigned char format_buffer[2000];
char text_buffer[40000];
char path_name[80];
struct dpmi_mem_info memory;
int cards_recognised;
int card_is;
int card_sub_type;
int vid_bank_tech;
int vid_tech;
int vid_error;
int fb_current_char_length;
int fb_max_char_length;
int vid_memory;
int old_mouse_drops_y;
int old_mouse_drops_x;
int bank_ptr;
int vid_no_of_banks;
short highlight_goods_list[18];
int slider_total;
short selection_goods_list[18];
int select_cost_flag;
int scat32000;
int para1;
int para2;
int select_width;
int select_height;
int rand128;
int select_count;
int slidper_on;
int scat128;
unsigned char font1[9460];
int rand32000;
int rand8;
int cnt32;
int out6;
int out7;
int out4;
int out5;
int out2;
int out3;
unsigned int randseed;
int out1;
int turbo_mode;
int out8;
int cnt16;
int cnt64;
unsigned int scatseed;
int cycle_count;
int cnt128;
int cnt8;
int cnt2;
int cnt4;
int cnt256;
int adjust_step;
int test_value4d;
int test_value4a;
int test_value4b;
int test_value4c;
int test_value2d;
int adjust_min;
int test_value2a;
int test_value2b;
int test_value2c;
int test_value3d;
int test_value3a;
int * adjust_var;
int test_value3b;
int test_value3c;
int test_value1d;
int test_value1a;
int test_value1b;
int test_value1c;
int adjust_max;
int font_style;
int absolute_ofset;
int x_wrap;
int dx;
int dy;
int font_screen_limit;
int ex;
int ey;
int sprite_bank;
int D;
int x_start;
int y_start;
int sprite_next_bank_count;
int gx;
int gy;
int y_length;
int ix;
int iy;
int x_length;
int sprite_bank_ofset;
int x_end;
int y_end;
int x_ofset;
int sprite_height;
int old_sprite_y;
int old_sprite_x;
int sprite2_start;
int sprite3_start;
int sprite_colour;
int sprite_error;
int sprite_width;
int sprite_image_no;
int sprite3_image_no;
int data_ptr;
int sprite2_height;
int sprite2_image_no;
int sprite_start;
int sprite2_width;
int sprite3_width;
int sprite_base_y;
int sprite_base_x;
int sprite_x_off;
int sprite_hat_start;
int sprite3_height;
int sprite_size;
int sprite_y_off;
int sprite_x;
int sprite_y;
int fb_max_width_reached;
int at_limit;
int fb_line_length;
int got_cursx;
int cursor_x;
int cursor_y;
char * text_pointer;
char insert_text[100];
int char_count;
int this_letter;
int insert_count;
int insert_place;
int x_is;
int fb_no_of_lines;
int fb_limit;
int fb_count;
int first_entry;
int cscreen;
int screen_width;
char *lang_file;
char directory[100][13];
int gamesource;
int granularity;
int no_of_entries;
int oscreen;
int screen_size;
unsigned char * internal_screen;
char extension[4];
int file_no;
char *media_file;
int screen_height;
int drive_name;
int file_string;
int button_time_flag;
int local_time;
int time_is;
int mouse_y;
int mouse_x;
int old_mouse_x;
int old_mouse_y;
int mouse_installed;
int mouse_movement;
int mouse_was_pressed;
int used_memory;
int free_memory;
int dos_memory;
int avl_memory;
int max_memory;
int scratch_buffer_size;
unsigned char * scratch_buffer;
int allocable_memory;
struct dpmi_real_block VesaModeInfo;
struct dpmi_real_block VesaInfo;
unsigned char mice[8630];
struct media_entry this_media_entry;
short mse_y;
short mse_x;
unsigned char game_panels[23441];
char filename[13];
char vid_new_val;
char old3d5_14;
char old3d5_17;
char vid_old_val;
char old3cf_6;
char old3cf_5;
char vid_val;
char old3c5_4;
unsigned char pre_loaded_status;
unsigned char continue_tutorial_status;
unsigned char file_loaded_status;
char map_gfx_loaded;
unsigned char hot_exit_flag;
unsigned char decision;
unsigned char confirming;
unsigned char exit_flag;
unsigned char restart_flag;
unsigned char test_mode3;
unsigned char test_mode2;
unsigned char test_mode1;
unsigned char test_mode4;
unsigned char develop_mode;
unsigned char xclipped;
unsigned char yclipped;
unsigned char insert_cursor;
char highlight;
unsigned char allow_padding;
unsigned char hot_key_out_off_build;
unsigned char hold_hot_keys;
unsigned char padding_off;
char screen_refresh_flag;
unsigned char hold_mouse_replace;
unsigned char screen_mode;
unsigned char pointer_mode;
unsigned char old_mouse_rb;
unsigned char old_mouse_lb;
unsigned char mouse_left_button;
unsigned char mouse_left_click;
unsigned char debar_fade_click;
unsigned char mouse_right_preclick;
unsigned char mouse_left_preclick;
unsigned char mouse_right_button;
unsigned char mouse_right_click;
unsigned char key_code;
char debug_interupt;
unsigned char key_ascii_was;
unsigned char mse_button;
unsigned char key_ready;
unsigned char key_ascii;

extern void write_i_sprite(unsigned char *sprite_addr);
extern void write_i_left_sprite(unsigned char *sprite_addr);
extern void write_i_right_sprite(unsigned char *sprite_addr);
extern void write_i_font(unsigned char *font);
extern void write_i_left_font(unsigned char *font);
extern void write_i_right_font(unsigned char *font);

extern int _dx;
#pragma aux _dx "*"
extern void __cdecl code_0188AD(void);
extern void __cdecl code_0188B7(void);
extern void __cdecl code_0188ED(void);
extern void __cdecl code_018944(void);
extern void __cdecl code_018964(void);
extern void __cdecl code_018986(void);
extern void __cdecl code_0189AE(void);
extern void __cdecl code_0189D6(void);
extern void __cdecl code_0189ED(void);
extern void __cdecl code_018A04(void);
extern void __cdecl code_018A1D(void);
extern void __cdecl code_018A2F(void);
extern void __cdecl code_018A39(void);
extern void __cdecl code_018A43(void);
extern void __cdecl code_018A4D(void);
extern void __cdecl code_018A79(void);
extern void __cdecl code_018AA0(void);
extern void __cdecl code_018AB9(void);
extern void __cdecl code_018AEE(void);
extern void __cdecl code_018B12(void);
extern void __cdecl code_018B39(void);
extern void __cdecl code_018B60(void);
extern void __cdecl code_018B87(void);
extern void __cdecl code_018BAC(void);
extern void __cdecl code_018BED(void);
extern void __cdecl code_018BFC(void);
extern void __cdecl code_018C03(void);
extern void __cdecl code_018C0A(void);
extern void __cdecl code_018C11(void);
extern void __cdecl code_018C18(void);
extern void __cdecl code_018C1F(void);
extern void __cdecl code_018C26(void);
extern void __cdecl code_018C2D(void);
extern void __cdecl code_018C4A(void);
extern void __cdecl code_018C74(void);
extern void __cdecl code_018C92(void);
extern void __cdecl code_018CCA(void);
extern void __cdecl code_018CEA(void);
extern void __cdecl code_018D13(void);
extern void __cdecl code_018D36(void);
extern void __cdecl code_018D50(void);
char get_insert_letter(void);
unsigned char sim_mouse(void);

extern void __far click_handler(unsigned int ax,
                                unsigned int bx,
                                unsigned int cx,
                                unsigned int dx,
                                unsigned int si,
                                unsigned int di);
/* Forward declarations (functions defined later in this file). */
void set_palette(char *p);
void clear_a_screen(void);
void get_mouse(void);
void clear_keys(void);
void to_fb(void);
void del_fb(void);
void test_for_delimiter(void);
void string_to_upper(unsigned char *s);
int my_strcmp(unsigned char *s1, unsigned char *s2, int n);
void my_strcpy(char *src, char *dst, int n);
void xclip(int clip_left, int clip_right);
int totalXpercent(int total, int percent);
void yclip(int clip_top, int clip_bottom);
void setup_scratch_buffer(void);
void stop_system(void);

#if PLATFORM_WINDOWS
extern char file_buffer[80];
extern char cd_drive[4];
extern int file_exists(char *filename);
#endif

#if PLATFORM_DOS

// Populate directory[] with up to 100 DOS 8.3 filenames matching a wildcard pattern.
// FUNCTION: C2 0x24212
// FUNCTION: C2WIN 0x0044a7e0
void get_directory(char *pattern)
{
    struct find_t find_buf;
    unsigned result;
    unsigned j;

    no_of_entries = 0;

    result = _dos_findfirst(pattern, 0, &find_buf);
    while (result == 0) {
        for (j = 0; j < 13; j++) {
            directory[no_of_entries][j] = find_buf.name[j];
        }
        no_of_entries++;
        result = _dos_findnext(&find_buf);
        if (no_of_entries >= 0x64) break;
    }

    first_entry = 0;
}
#else
void get_directory(char *pattern)
{
    struct _finddata_t find_buf;
    long hFile;
    int j;
    int find_return;

    no_of_entries = 0;
    hFile = _findfirst(pattern, &find_buf);
    if (hFile == -1) return;
    find_return = 0;
    while (find_return == 0) {
        for (j = 0; j < 13; j++) {
            directory[no_of_entries][j] = find_buf.name[j];
        }
        no_of_entries++;
        find_return = _findnext(hFile, &find_buf);
        if (no_of_entries >= 100) break;
    }
    first_entry = 0;
}
#endif /* PLATFORM_DOS */

// Switch to the CD drive and media subdirectory selected by the file extension.
// FUNCTION: C2 0x2426e
// FUNCTION: C2WIN 0x0044a8ae
void cd_path(const char *filename)
{
    char drive_letter;
    unsigned saved_drive;
    int matched;
    char *buf = "c:\\";

    if (c2inf.drive_init != 1) return;

    get_filename_extension(filename);
    string_to_upper(extension);

    matched = 1;
    if (strcmp("PL8", extension) == 0) matched = 0;
    else if (strcmp("RAW", extension) == 0) matched = 0;
    else if (strcmp("XMI", extension) == 0) matched = 0;
    else if (strcmp("SMK", extension) == 0) matched = 0;
    if (matched) return;

#if PLATFORM_DOS
    _dos_setdrive(c2inf.cd_letter - 0x40, &saved_drive);
#else
    _chdrive(c2inf.cd_letter - 0x40);
#endif /* PLATFORM_DOS */
    drive_letter = c2inf.cd_letter;
    buf[0] = drive_letter;
    matched = chdir(buf);

    if      (strcmp("PL8", extension) == 0) chdir("pl8");
    else if (strcmp("RAW", extension) == 0) chdir("raw");
    else if (strcmp("XMI", extension) == 0) chdir("xmi");
    else if (strcmp("SMK", extension) == 0) chdir("smk");
}

// If c2inf.drive_init==1 we're booted from a non-system drive: switch back to the original drive
// and chdir into the install path.
// FUNCTION: C2 0x24382
// FUNCTION: C2WIN 0x0044aa77
void main_path(void)
{
    unsigned drive_count;
    if (c2inf.drive_init != 1) return;
#if PLATFORM_DOS
    _dos_setdrive(drive_name - 0x40, &drive_count);
#else
    _chdrive(drive_name - 0x40);
#endif /* PLATFORM_DOS */
    chdir(path_name);
}

// Walk past the filename to the '.' separator and copy the 3-char extension into the global
// `extension[]` buffer (NUL-terminated).
// FUNCTION: C2 0x243b0
// FUNCTION: C2WIN 0x0044aab8
void get_filename_extension(const char *filename)
{
    unsigned char character;
    do {
        character = *filename;
        filename++;
    } while (character != '.' && character != 0);
    if (character == '.') {
        extension[0] = *filename++;
        extension[1] = *filename++;
        extension[2] = *filename++;
        extension[3] = 0;
    }
}

// Walk past the filename to the '.' separator and overwrite the 3-char extension with the global
// `extension[]` (NUL-terminated).
// FUNCTION: C2 0x243ef
// FUNCTION: C2WIN 0x0044ab28
void put_filename_extension(char *filename)
{
    unsigned char character;
    do {
        character = *filename;
        filename++;
    } while (character != '.' && character != 0);
    if (character == '.') {
        *filename++ = extension[0];
        *filename++ = extension[1];
        *filename++ = extension[2];
        *filename++ = 0;
    }
}

// Length of a filename up to (and including) the first '.' or '\0'.
// FUNCTION: C2 0x2442b
// FUNCTION: C2WIN 0x0044ab9a
char get_filename_length(char *filename)
{
    char length = 0;
    unsigned char character;
    do {
        character = *filename++;
        length++;
    } while (character != '.' && character != 0);
    return length;
}

// Locate a file in the platform's local and media search paths.
// FUNCTION: C2 0x24446
// FUNCTION: C2WIN 0x0044abdd
#if PLATFORM_WINDOWS
int check_file_exists(char *filename)
{
    int fd;
    int found = 0;

    fd = open(filename, O_BINARY);
    if (fd >= 0) {
        found = 1;
        close(fd);
        return 1;
    }
    sprintf(file_buffer, "data0\\%s", filename);
    if (file_exists(file_buffer)) return 1;

    get_filename_extension(filename);
    string_to_upper(extension);
    if (strcmp("PL8", extension) == 0) {
        sprintf(file_buffer, "%s%s\\Pl8\\%s", cd_drive, "C2Win95", filename);
        found = 2;
    } else if (strcmp("RAW", extension) == 0) {
        sprintf(file_buffer, "%s%s\\Raw\\%s", cd_drive, "C2Win95", filename);
        found = 3;
    } else if (strcmp("XMI", extension) == 0) {
        sprintf(file_buffer, "%s%s\\Xmi\\%s", cd_drive, "C2Win95", filename);
        found = 4;
    } else if (strcmp("SMK", extension) == 0) {
        sprintf(file_buffer, "%s%s\\Smk\\%s", cd_drive, "C2Win95", filename);
        found = 5;
    }

    fd = open(file_buffer, O_BINARY);
    if (fd >= 0) {
        close(fd);
        return found;
    }
    return 0;
}
#else
int check_file_exists(char *filename)
{
    int fd;
    int found = 0;

    cd_path(filename);
    fd = open(filename, O_BINARY);
    if (fd >= 0) {
        found = 1;
        close(fd);
    }
    main_path();
    return found;
}
#endif

// Variant of check_file_exists that doesn't cd_path/main_path — just opens the file at its given
// path.
// FUNCTION: C2 0x24477
// FUNCTION: C2WIN 0x0044adc8
int is_file_on_harddrive(char *filename)
{
    int fd;
    int found = 0;
    fd = open(filename, O_RDWR | O_BINARY);
    if (fd >= 0) {
        found = 1;
        close(fd);
    }
#if PLATFORM_WINDOWS
    sprintf(file_buffer, "data0\\%s", filename);
    if (file_exists(file_buffer)) found = 1;
#endif
    return found;
}

// Read `size` bytes from `fname` at byte `offset` into `buf`. Returns the number of bytes actually
// read, or 0 if both the hard-drive and CD attempts failed to read anything.
// FUNCTION: C2 0x2449a
#if PLATFORM_WINDOWS
// FUNCTION: C2WIN 0x0044ae48
int readfile(const char *filename, void *buffer, int size, int offset)
{
    int seek_result;
    int fd;
    int bytes_read;

    bytes_read = 0;
    fd = open(filename, O_BINARY);
    if (fd != -1) {
        seek_result = _lseek(fd, offset, 0);
        if (seek_result != -1) {
            bytes_read = read(fd, buffer, size);
        }
        close(fd);
    }

    if (bytes_read > 0) goto done;

    sprintf(file_buffer, "data0\\%s", filename);
    if (!file_exists(file_buffer)) {
        get_filename_extension(filename);
        string_to_upper(extension);
        if (strcmp("PL8", extension) == 0)
            sprintf(file_buffer, "%s%s\\Pl8\\%s", cd_drive, "C2Win95", filename);
        else if (strcmp("RAW", extension) == 0)
            sprintf(file_buffer, "%s%s\\Raw\\%s", cd_drive, "C2Win95", filename);
        else if (strcmp("XMI", extension) == 0)
            sprintf(file_buffer, "%s%s\\Xmi\\%s", cd_drive, "C2Win95", filename);
        else if (strcmp("SMK", extension) == 0)
            sprintf(file_buffer, "%s%s\\Smk\\%s", cd_drive, "C2Win95", filename);
    }
    fd = open(file_buffer, O_BINARY);
    if (fd != -1) {
        seek_result = _lseek(fd, offset, 0);
        if (seek_result != -1) {
            bytes_read = read(fd, buffer, size);
        }
        close(fd);
    }

done:
    return bytes_read;
}
#else
int readfile(const char *filename, void *buffer, int size, int offset)
{
    int fd;
    int bytes_read;

    bytes_read = 0;
    fd = open(filename, O_BINARY);
    if (fd != -1) {
        if (_lseek(fd, offset, 0) != -1) {
            bytes_read = read(fd, buffer, size);
        }
        close(fd);
    }

    if (bytes_read <= 0) {
        cd_path(filename);
        fd = open(filename, O_BINARY);
        if (fd != -1) {
            if (_lseek(fd, offset, 0) != -1) {
                bytes_read = read(fd, buffer, size);
            }
            close(fd);
        }
        main_path();
    }

    return bytes_read;
}
#endif

// open() the named file (creating it 0644), write `size` bytes, close. Returns the number written,
// or 0 on open failure.
// FUNCTION: C2 0x24541
int writefile(const char *filename, char *buffer, int size)
{
    int fd;
    int bytes_written;
    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0x180);
    if (fd == -1)
        return 0;
    bytes_written = write(fd, buffer, size);
    close(fd);
    return bytes_written;
}

// Write `size` bytes at `offset` in `fname`; returns zero on open or seek failure.
// FUNCTION: C2 0x24572
int write_to_file(char *filename, char *buffer, int size, int offset)
{
    int seek_result;
    int fd;
    int bytes_written;
    fd = open(filename, O_WRONLY | O_CREAT | O_BINARY, 0x180);
    if (fd == -1)
        return 0;
    seek_result = _lseek(fd, offset, 0);
    if (seek_result == -1)
        return 0;
    bytes_written = write(fd, buffer, size);
    close(fd);
    return bytes_written;
}

// Read the "resaud" value from a key-value configuration file.
// FUNCTION: C2 0x245be
char read_config(char *filename, char *buffer)
{
    int search_remaining;

    search_remaining = 1000;
    if (readfile(filename, buffer, 1000, 0) == 0) return 1;

    while (search_remaining > 0) {
        if (my_strcmp(buffer, "resaud", 6) == 0) break;
        search_remaining--;
        buffer++;
    }
    if (search_remaining <= 0) return 0;

    buffer += 7;
    return *buffer;
}

// Decode an IFF ILBM image into a palette and raster buffer.
// FUNCTION: C2 0x2460f
int convert_lbm_file(unsigned char *src, unsigned char *dst, char *pal, int length)
{
    short bitmap_height;
    short file_width;
    int search_left;
    int pixel;
    unsigned char count;
    unsigned char run;
    unsigned char no_of_planes;
    unsigned char pal_component;
    unsigned char *chunk_ptr;
    unsigned int c;
    int b;
    int body_length;

    chunk_ptr = src;
    search_left = 0x64;
    while (search_left > 0) {
        if (my_strcmp((char *)chunk_ptr, "BMHD", 4) == 0) break;
        search_left--; chunk_ptr++;
    }
    if (search_left <= 0) return 2;

    chunk_ptr += 8;
    file_width = (*chunk_ptr++ << 8) + *chunk_ptr++;
    bitmap_height = (*chunk_ptr++ << 8) + *chunk_ptr++;
    if (file_width == 0x140) screen_mode = 1;
    else if (file_width == 0x280) screen_mode = 2;
    else return 4;
    chunk_ptr += 5;
    no_of_planes = *chunk_ptr++;
    if (no_of_planes == 1) return 3;

    search_left = length;
    while (search_left > 0) {
        if (my_strcmp((char *)chunk_ptr, "CMAP", 4) == 0) break;
        search_left--; chunk_ptr++;
    }
    if (search_left <= 0) return 6;
    chunk_ptr += 8;
    for (pixel = 0; pixel < 0x300; pixel++) {
        pal_component = *chunk_ptr++;
        pal_component = pal_component >> 2;
        *pal++ = pal_component;
    }

    search_left = length;
    while (search_left > 0) {
        if (my_strcmp((char *)chunk_ptr, "BODY", 4) == 0) break;
        search_left--; chunk_ptr++;
    }
    if (search_left <= 0) return 7;
    chunk_ptr += 4;
    pixel = *chunk_ptr++;
    b = *chunk_ptr++;
    c = *chunk_ptr++;
    body_length = *chunk_ptr++;
    search_left = body_length + ((c << 8) + (b << 16));
    if (search_left > length) return 8;

    for (pixel = 0; search_left > pixel; ++pixel)
    {
        count = *chunk_ptr++;
        if (count > 0x80) {
            run = *chunk_ptr++;
            for (b = 0; b < 0x100 - count + 1; b++) *dst++ = run;
            ++pixel;
        }
        else {
            for (b = 0; b < count + 1; b++) *dst++ = *chunk_ptr++;
            pixel += count + 1;
        }
    }
    return 0;
}

#if PLATFORM_DOS

// Query VESA information and enter 640×480 or 640×400 banked SVGA mode.
// FUNCTION: C2 0x247b1
int set_svga_640_480(int mode)
{
    union REGS r;
    struct SREGS sr;
    int selector;
    int mode_id;
    int height;
    int mode_granularity;
    int segment;

    if (mode == 0) {
        mode_id = 0x101;
        height = 0x1e0;
    } else if (mode == 1) {
        mode_id = 0x100;
        height = 0x190;
    }
    set_vga_mode(0x12);

    /* VESA 4F00 — VbeInfoBlock */
    memset(&sr, 0, 0xc);
    r.w.ax = 0x100;
    r.w.bx = 0x10;
    int386(0x31, &r, &r);
    selector = r.w.ax;
    segment = r.w.dx;
    VesaInfo.selector = segment;
    VesaInfo.offset   = 0;
    _fmemset(MK_FP(segment, 0), 0xaa, 0x100);
    memset(&RMI, 0, 0x32);
    RMI.eax = 0x4f00;
    RMI.es = selector;
    RMI.edx = 0;
    r.w.ax = 0x300;
    r.h.bl = 0x10;
    r.h.bh = 0;
    r.w.cx = 0;
    sr.es = FP_SEG(&RMI);
    r.x.edi = (int)&RMI;
    int386x(0x31, &r, &r, &sr);
    _fmemcpy((void __far *)&vesa_info, *(void __far **)&VesaInfo, 0x100);
    vid_memory = (short)vesa_info.total_memory << 6;
    r.w.ax = 0x101;
    r.w.bx = segment;
    int386(0x31, &r, &r);
    if (RMI.eax != 0x4f) return 1;

    /* VESA 4F01 — ModeInfoBlock */
    memset(&sr, 0, 0xc);
    r.w.ax = 0x100;
    r.w.bx = 0x10;
    int386(0x31, &r, &r);
    selector = r.w.ax;
    segment = r.w.dx;
    VesaModeInfo.selector = segment;
    VesaModeInfo.offset   = 0;
    _fmemset(MK_FP(segment, 0), 0xaa, 0x100);
    memset(&RMI, 0, 0x32);
    RMI.eax = 0x4f01;
    RMI.ecx = mode_id;
    RMI.es = selector;
    RMI.edx = 0;
    r.w.ax = 0x300;
    r.h.bl = 0x10;
    r.h.bh = 0;
    r.w.cx = 0;
    sr.es = FP_SEG(&RMI);
    r.x.edi = (int)&RMI;
    int386x(0x31, &r, &r, &sr);
    _fmemcpy((void __far *)&vesa_mode_info, *(void __far **)&VesaModeInfo, 0x100);
    bank_ptr = (vesa_mode_info.win_func_seg << 4) + vesa_mode_info.win_func_off;
    r.w.ax = 0x101;
    r.w.bx = segment;
    int386(0x31, &r, &r);
    recognise_card();
    get_video_technique();

    r.w.ax = 0x4f02;
    r.w.bx = mode_id;
    int386(0x10, &r, &r);
    if (r.w.ax != 0x4f) return 2;

    screen_width = 0x280;
    screen_height = height;
    screen_size = height * 0x280;
    mode_granularity = vesa_mode_info.win_granularity;
    if (mode_granularity == 0x40)      granularity = 0;
    else if (mode_granularity == 0x20) granularity = 1;
    else if (mode_granularity == 0x10) granularity = 2;
    else if (mode_granularity == 8)    granularity = 3;
    else if (mode_granularity == 4)    granularity = 4;
    else if (mode_granularity == 2)    granularity = 5;
    else if (mode_granularity == 1)    granularity = 6;
    set_bank(0);
    return 0;
}
#endif /* PLATFORM_DOS */

// Detects the installed VGA chipset and records its capabilities.
// FUNCTION: C2 0x24b0e
// FUNCTION: C2WIN 0x0044b500
#if PLATFORM_WINDOWS
void recognise_card(void)
{
}
#else
void recognise_card(void)
{
    int i;

    cards_recognised = 0;
    card_sub_type    = 0;
    card_is          = 0;
    for (i = 0; i < 0x20; i++)
        card_ids[i] = 0;

    card_ids[1] = check_for_Trident();
    card_ids[2] = check_for_Tseng();

    if (cards_recognised > 1) {
        card_sub_type    = 0;
        card_is          = 0;
        cards_recognised = 0;
    }
}
#endif

#if PLATFORM_DOS

// Detect a Trident VGA chipset and record its model.
// FUNCTION: C2 0x24b69
int check_for_Trident(void)
{
    int unused_result;

    /* Save current SR0E, write 0, read back the low nibble. */
    outp(0x3c4, 0x0e);
    vid_old_val = inp(0x3c5);
    outp(0x3c5, 0);
    vid_val = inp(0x3c5) & 0xf;
    outp(0x3c5, vid_old_val);

    if (vid_val != 2) return 0;

    /* Hardware Version index. */
    outp(0x3c4, 0x0b);
    outp(0x3c5, 0);
    vid_val = inp(0x3c5) & 0xf;

    card_is = 1;
    if (vid_val >= 3) {
        card_sub_type = 0x22c4;
    } else {
        card_sub_type = 0x2260;
    }
    cards_recognised += 1;
    return 1;
}
#endif /* PLATFORM_DOS */

// Report that no supported Tseng VGA chipset was detected.
// FUNCTION: C2 0x24c2c
int check_for_Tseng(void)
{
    return 0;
}

// Set vid_tech / vid_bank_tech / vid_no_of_banks based on the detected VESA memory and recognised
// cards.
// FUNCTION: C2 0x24c2f
// FUNCTION: C2WIN 0x0044b50b
#if PLATFORM_WINDOWS
void get_video_technique(void)
{
}
#else
void get_video_technique(void)
{
    vid_tech         = 0;
    vid_bank_tech    = 0;
    vid_no_of_banks  = 0;
    if (vid_memory >= 0x400)
        vid_tech = 1;
    if (cards_recognised != 0)
        vid_bank_tech = 1;
    if (vid_bank_tech == 0)
        vid_tech = vid_bank_tech;
}
#endif

// Print the detected VESA mode, chipset, memory, and banking information.
// FUNCTION: C2 0x24c7f
// FUNCTION: C2WIN 0x0044b516
#if PLATFORM_WINDOWS
void print_vesa_info(void)
{
}
#else
void print_vesa_info(void)
{
    int oem_string_addr;

    printf("\n--------------------------------------------------------\n");
    if (vid_error == 1) printf("Vesa SVGA not supported by this graphics card.\n");
    else if (vid_error == 2) printf("SVGA mode not supported by this graphics card.\n");
    else if (vid_error == 3) printf("VESA ext bios error :- failed to set bank.\n");
    else printf("VESA Compliant  - Video Card Information.\n");
    printf("--------------------------------------------------------\n");
    if (vid_error != 0) return;

    oem_string_addr = (vesa_info.oem_string_seg << 4) + vesa_info.oem_string_off;
    printf("OEM string      : %s \n", (char *)oem_string_addr);
    printf("VESA Version    : %d.%d,",
           ((short)vesa_info.version & 0xff00) >> 8,
           (short)vesa_info.version & 0xff);
    printf(" %s", chipset_names[card_is]);
    if (card_sub_type != 0) printf(" %d chipset.\n", card_sub_type);
    else printf(" chipset.\n");
    printf("Video memory    : %dk \n", vid_memory);
    printf("Attributes      : %x,%x,",
           vesa_mode_info.win_a_attributes,
           vesa_mode_info.win_b_attributes);
    if (vid_tech == 0) printf("S,"); else printf("C,");
    if (vid_bank_tech == 0) printf("S\n"); else printf("C\n");
    printf("Granularity     : %dk with %dk size ",
           vesa_mode_info.win_granularity,
           vesa_mode_info.win_size);
    printf("at  %x and %x \n",
           vesa_mode_info.win_a_segment,
           vesa_mode_info.win_b_segment);
    printf("Bank function   : %x \n", bank_ptr);
    printf("--------------------------------------------------------\n");
}
#endif

#if PLATFORM_DOS

// Switch the VGA into mode-X (320×200, 4 planes × 80 bytes), saving the touched register values
// into the old3*_* slots so unset_vga_256x can restore them. Read-modify-write each control
// register through `val` then push back via outp.
// FUNCTION: C2 0x24e3c
void set_vga_256x(void)
{
    char val;

    outp(0x3c4, 4);
    old3c5_4 = inp(0x3c5);
    outp(0x3ce, 5);
    old3cf_5 = inp(0x3cf);
    outp(0x3ce, 6);
    old3cf_6 = inp(0x3cf);
    outp(0x3d4, 0x14);
    old3d5_14 = inp(0x3d5);
    outp(0x3d4, 0x17);
    old3d5_17 = inp(0x3d5);

    set_vga_mode(0x13);

    outp(0x3c4, 4);
    val = (inp(0x3c5) & 0xf3) | 4;
    outp(0x3c5, val);

    outp(0x3ce, 5);
    val = inp(0x3cf) & 0xef;
    outp(0x3cf, val);

    outp(0x3ce, 6);
    val = inp(0x3cf) & 0xfd;
    outp(0x3cf, val);

    outp(0x3d4, 0x14);
    val = inp(0x3d5) & 0xbf;
    outp(0x3d5, val);

    outp(0x3d4, 0x17);
    val = inp(0x3d5) | 0x40;
    outp(0x3d5, val);

    clear_all_screens();
    oscreen = 0x4000;
    cscreen = 0;
    swap_screens();
    swap_screens();
    screen_width  = 320;
    screen_height = 200;
    screen_size   = 0xfa00;
}

// Restore the VGA registers saved by set_vga_256x to leave the adapter in a sane mode-X state,
// after wiping all four pages.
// FUNCTION: C2 0x24fef
void unset_vga_256x(void)
{
    clear_all_screens();
    outp(0x3c4, 4);
    outp(0x3c5, old3c5_4);
    outp(0x3ce, 5);
    outp(0x3cf, old3cf_5);
    outp(0x3ce, 6);
    outp(0x3cf, old3cf_6);
    outp(0x3d4, 0x14);
    outp(0x3d5, old3d5_14);
    outp(0x3d4, 0x17);
    outp(0x3d5, old3d5_17);
}

// Switch to BIOS text mode 3.
// FUNCTION: C2 0x2509c
void set_mode3(void)
{
    union REGS regs;
    regs.w.ax = 3;
    int386(0x10, &regs, &regs);
}

// Sets vga mode.
// FUNCTION: C2 0x250bb
void set_vga_mode(int mode)
{
    union REGS r;
    r.w.ax = mode;
    int386(0x10, &r, &r);
}

// Switches the EGA/VGA Graphics Controller to read from page 1.
// FUNCTION: C2 0x250c6
void page1_read(void)
{
    outpw(0x3CE, 4);
}

// Push a 256×3 palette buffer to the VGA DAC. Sends 0xFF to the pixel-mask register (0x3C6) once,
// then for each colour writes the index to 0x3C8 and the R/G/B triple to 0x3C9.
// FUNCTION: C2 0x250d8
void set_vga_palette(char *p)
{
    int i;
    outp(0x3c6, 0xff);
    for (i = 0; i < 256; i++) {
        outp(0x3c8, i);
        outp(0x3c9, *p++);
        outp(0x3c9, *p++);
        outp(0x3c9, *p++);
    }
}

// Program the VGA DAC entries [start, end] (inclusive) from the 3-bytes-per-entry palette buffer
// at `p`.
// FUNCTION: C2 0x25134
void set_vga_palette_range(char *p, int start, int end)
{
    int i;
    for (i = start; i <= end; i++) {
        outp(0x3c8, i);
        outp(0x3c9, *p++);
        outp(0x3c9, *p++);
        outp(0x3c9, *p++);
    }
}
#endif /* PLATFORM_DOS */

#if PLATFORM_WINDOWS
extern void direct_palette_set(char *p);

// FUNCTION: C2WIN 0x0044b56e
void set_vga_palette(char *p)
{
    direct_palette_set(p);
}
#endif

// Rotate the colour entries in current_palette[start_idx..end_idx] (inclusive) by one slot toward
// higher indices: the colour that was at end_idx moves to start_idx, and every entry in between
// shifts up by one.
// FUNCTION: C2 0x2517f
void cycle_colours(int start_idx, int end_idx)
{
    int i;
    char saved[3];

    saved[0] = current_palette[3 * end_idx + 0];
    saved[1] = current_palette[3 * end_idx + 1];
    saved[2] = current_palette[3 * end_idx + 2];

    for (i = end_idx; i > start_idx; i--) {
        current_palette[3 * i + 0] = current_palette[3 * (i - 1) + 0];
        current_palette[3 * i + 1] = current_palette[3 * (i - 1) + 1];
        current_palette[3 * i + 2] = current_palette[3 * (i - 1) + 2];
    }

    current_palette[3 * start_idx + 0] = saved[0];
    current_palette[3 * start_idx + 1] = saved[1];
    current_palette[3 * start_idx + 2] = saved[2];

    set_vga_palette_range(&current_palette[3 * start_idx], start_idx, end_idx);
}

// Three-slot red flash effect. Increments slot[idx]'s red channel by `delta`, capping wraparound
// to 0x10 (so it never blows past 0x3F into invalid territory).
// FUNCTION: C2 0x25226
// FUNCTION: C2WIN 0x0044b659
void pulse_red(int idx, int delta)
{
    unsigned char red;

    red = current_palette[3 * idx + 0];
    red += delta;
    if (red > 0x3f) red = 0x10;

    current_palette[3 * idx + 0] = red;
    current_palette[3 * idx + 1] = 0;
    current_palette[3 * idx + 2] = 0;

    current_palette[3 * (idx + 1) + 0] = red;
    current_palette[3 * (idx + 1) + 1] = red / 2;
    current_palette[3 * (idx + 1) + 2] = 0;

    current_palette[3 * (idx + 2) + 0] = red;
    current_palette[3 * (idx + 2) + 1] = (3 * red) / 4;
    current_palette[3 * (idx + 2) + 2] = 0;

    set_vga_palette_range(&current_palette[3 * idx], idx, idx + 2);
}

// Toggle palette entry 0 between black and full-red, set the rest of the entry to 0, and reload
// the VGA palette.
// FUNCTION: C2 0x252de
// FUNCTION: C2WIN 0x0044b730
void swap_background_to_red(void)
{
    if (current_palette[0] == 0x3f)
        current_palette[0] = 0;
    else
        current_palette[0] = 0x3f;
    current_palette[1] = 0;
    current_palette[2] = 0;
    set_vga_palette_range(current_palette, 0, 0);
}

// Copy palette entry `idx` (RGB triple) into entry 0 and reload the VGA palette.
// FUNCTION: C2 0x2531c
// FUNCTION: C2WIN 0x0044b77d
void swap_background_to(int idx)
{
    int r = current_palette[idx*3];
    int g = current_palette[idx*3 + 1];
    int b = current_palette[idx*3 + 2];
    current_palette[0] = r;
    current_palette[1] = g;
    current_palette[2] = b;
    set_vga_palette_range(current_palette, 0, 0);
}

// Copies the active palette into the caller's buffer.
// FUNCTION: C2 0x2534f
void copy_palette(char *src, char *dst)
{
    int i;

    for (i = 0; i < 256; i++) {
        int ptr;
        ptr = i * 3;
        dst[i*3]   = src[ptr];
        dst[i*3+1] = src[ptr+1];
        dst[i*3+2] = src[ptr+2];
    }
}

// Right-shift every channel of a 256-entry palette by 2 (6-bit VGA → 4-bit hi-color downsample).
// FUNCTION: C2 0x25384
// FUNCTION: C2WIN 0x0044b8f7
void go_64k_palette(unsigned char *p)
{
    int i;

    for (i = 0; i < 256; i++) {
        int ptr;
        ptr = i * 3;
        p[ptr] >>= 2;
        p[ptr + 1] >>= 2;
        p[ptr + 2] >>= 2;
    }
}

// Convert every channel of a 256-entry palette between 6-bit and 8-bit values.
// FUNCTION: C2 0x253ab
// FUNCTION: C2WIN 0x0044b94f
void go_16m_palette(unsigned char *p)
{
    int i;

    for (i = 0; i < 256; i++) {
        int ptr;
        ptr = i * 3;
        p[ptr] <<= 2;
        p[ptr + 1] <<= 2;
        p[ptr + 2] <<= 2;
    }
}

// Fade current_palette one step at a time toward `p`, allowing mouse clicks to skip the inter-step
// delay.
// FUNCTION: C2 0x255b1
// FUNCTION: C2WIN 0x0044b9a7
void fade_to_palette(char *p)
{
    short i;
    short loop_counter;
    int delay;
    int wait_limit;
    unsigned char changed_flag;
    unsigned char wanted;
    unsigned char current_color;
    short fade_no;
#if !PLATFORM_DOS
    int pal_ptr;
#endif

    wait_limit = 5;
    for (fade_no = 0; fade_no < 300; fade_no++) {
        changed_flag = 0;
        loop_counter = 0;
        get_mouse();
        if ((mouse_left_preclick || mouse_right_preclick) && !debar_fade_click) wait_limit = 0;
        delay = 0;
#if PLATFORM_DOS
        while (loop_counter < 20000) {
            delay += running_delay1();
            if (delay >= wait_limit) break;
            loop_counter++;
        }
#endif

        for (i = 0; i < 256; i++) {
#if PLATFORM_DOS
            wanted = p[i * 3];
            current_color = current_palette[i * 3];
#else
            pal_ptr = i * 3;
            wanted = p[pal_ptr];
            current_color = current_palette[pal_ptr];
#endif
            if (current_color > wanted) {
                current_color--;
                changed_flag = 1;
            } else if (current_color < wanted) {
                if (fade_no >= (unsigned char)current_color) {
                    current_color++;
                    changed_flag = 1;
                }
            }
#if PLATFORM_DOS
            current_palette[i * 3] = current_color;
#else
            current_palette[pal_ptr] = current_color;
#endif

#if PLATFORM_DOS
            wanted = p[i * 3 + 1];
            current_color = current_palette[i * 3 + 1];
#else
            wanted = p[pal_ptr + 1];
            current_color = current_palette[pal_ptr + 1];
#endif
            if (current_color > wanted) {
                current_color--;
                changed_flag = 1;
            } else if (current_color < wanted) {
                if (fade_no >= (unsigned char)current_color) {
                    current_color++;
                    changed_flag = 1;
                }
            }
#if PLATFORM_DOS
            current_palette[i * 3 + 1] = current_color;
#else
            current_palette[pal_ptr + 1] = current_color;
#endif

#if PLATFORM_DOS
            wanted = p[i * 3 + 2];
            current_color = current_palette[i * 3 + 2];
#else
            wanted = p[pal_ptr + 2];
            current_color = current_palette[pal_ptr + 2];
#endif
            if (current_color > wanted) {
                current_color--;
                changed_flag = 1;
            } else if (current_color < wanted) {
                if (fade_no >= (unsigned char)current_color) {
                    current_color++;
                    changed_flag = 1;
                }
            }
#if PLATFORM_DOS
            current_palette[i * 3 + 2] = current_color;
#else
            current_palette[pal_ptr + 2] = current_color;
#endif
        }

        if (!changed_flag) break;
        set_vga_palette(current_palette);
    }

    set_palette(p);
    debar_fade_click = 0;
}
// Read 0x300 bytes (256 RGB triples) into temp_palette and apply.
// FUNCTION: C2 0x2550b REORDERED
// FUNCTION: C2WIN 0x0044bbce
void load_to_temp_palette(char *fname)
{
    readfile(fname, temp_palette, 0x300, 0);
    set_palette(temp_palette);
}

// Same as load_to_temp_palette but fades into the new palette.
// FUNCTION: C2 0x2552d
// FUNCTION: C2WIN 0x0044bbfe
void fade_to_temp_palette(char *fname)
{
    readfile(fname, temp_palette, 0x300, 0);
    fade_to_palette(temp_palette);
}

// Replace the active palette with black.
// FUNCTION: C2 0x2554f
// FUNCTION: C2WIN 0x0044bc2e
void black_out(void)
{
    set_palette(black_out_data);
}

// Copy a 256×3 palette buffer to current_palette and push it to the VGA DAC. The trailing
// assignments to current_palette[0..2] force VGA colour 0 to black regardless of the source
// palette.
// FUNCTION: C2 0x25554
// FUNCTION: C2WIN 0x0044b7e1 REORDERED
void set_palette(char *p)
{
    int i;

    {
        int ptr;

        for (i = 0; i < 256; i++) {
            ptr = i * 3;
            current_palette[i * 3]     = p[ptr];
            current_palette[i * 3 + 1] = p[ptr + 1];
            current_palette[i * 3 + 2] = p[ptr + 2];
        }
    }
    current_palette[0] = current_palette[1] = current_palette[2] = 0;
    set_vga_palette(current_palette);
}

// Fade the active palette to black.
// FUNCTION: C2 0x255ac
// FUNCTION: C2WIN 0x0044bc46
void fade_to_black_out(void)
{
    fade_to_palette(black_out_data);
}


#if PLATFORM_DOS

// Wait for a complete vertical-blank cycle on port 0x3DA bit 3.
// FUNCTION: C2 0x255cb
// FUNCTION: C2WIN 0x0044bc5e
void wvbl2(void)
{
    while (inp(0x3DA) & 8) ;
    while (!(inp(0x3DA) & 8)) ;
}
#else
void wvbl2(void)
{
}
#endif /* PLATFORM_DOS */

#if PLATFORM_DOS
// Toggle the active framebuffer page (cscreen / oscreen) by reprogramming the CRTC start-address
// registers and flipping a private page flag.
// FUNCTION: C2 0x255e8
// FUNCTION: C2WIN 0x0044bc69
void swap_screens(void)
{
    static int page_flag;

    outpw(0x3d4, 0x0d);
    outpw(0x3d4, cscreen + 0xc);
    page_flag ^= 1;
    if (page_flag != 0) {
        oscreen = 0x4000;
        cscreen = 0;
    } else {
        oscreen = 0;
        cscreen = 0x4000;
    }
}
#else
void swap_screens(void)
{
}
#endif /* PLATFORM_DOS */

// De-interleave a 4-plane mode-X buffer (4 × 0x3E80 bytes) back into a contiguous 256×N raster.
// FUNCTION: C2 0x25645
// FUNCTION: C2WIN 0x0044bc74
void convert256x_to_256screen(char *src, char *dst)
{
    int i;
    for (i = 0; i < 0x3e80; i++) {
        dst[i*4]     = src[i];
        dst[i*4 + 1] = src[i + 0x3e80];
        dst[i*4 + 2] = src[i + 0x7d00];
        dst[i*4 + 3] = src[i + 0xbb80];
    }
}

// Repack a 320×200 linear-256 framebuffer (`src` is 64,000 bytes of one byte per pixel in
// row-major order) into the four-plane VGA mode-X layout used by `dst`. Mode-X interleaves pixels
// in groups of four across four 16,000-byte planes (a, b, c, d).
// FUNCTION: C2 0x25686
// FUNCTION: C2WIN 0x0044bcfa
void convert256_to_256xscreen(unsigned char *src, unsigned char *dst)
{
    int i;
    int s;

    i = 0;
    s = 0;
    for (; i < 0x3e80; i += 2, s += 8) {
        dst[i]            = src[s];
        dst[i + 0x3e80]   = src[s + 1];
        dst[i + 0x7d00]   = src[s + 2];
        dst[i + 0xbb80]   = src[s + 3];
        dst[i + 1]        = src[s + 4];
        dst[i + 0x3e81]   = src[s + 5];
        dst[i + 0x7d01]   = src[s + 6];
        dst[i + 0xbb81]   = src[s + 7];
    }
}

// Dispatch on screen_mode to the right physical-screen blitter.
// FUNCTION: C2 0x256f8
void copy_to_physical_screen(int source_addr, int screen_offset)
{
#if !PLATFORM_WINDOWS
    if (screen_mode == 1) {
        convert_and_copy_to_256xscreen(source_addr, screen_offset);
        return;
    }
    copy_to_640_480_screen(source_addr);
#endif
}

// Clear all mode-X pages, or the active linear framebuffer in an SVGA mode.
// FUNCTION: C2 0x25714
void clear_screens(void)
{
#if PLATFORM_WINDOWS
    if (internal_screen != 0)
        memset(internal_screen, 0, 0x4b000);
#else
    if (screen_mode == 1) {
        cls_256x(0,       0xfa00);
        cls_256x(0x4000,  0xfa00);
        cls_256x(0x8000,  0xfa00);
        cls_256x(0xc000,  0xfa00);
    } else {
        clear_a_screen();
    }
#endif
}

// Wipe the current visible page. In mode 1 (320×200 mode-X) defer to cls_256x; in modes 2/3
// (640×480 / 640×400 linear) zero the internal_screen buffer byte-by-byte.
// FUNCTION: C2 0x25763
// FUNCTION: C2WIN 0x0044be1d
void clear_a_screen(void)
{
#if PLATFORM_WINDOWS
    if (internal_screen != 0)
        memset(internal_screen, 0, 0x4b000);
#else
    int i;
    if (screen_mode == 1) {
        cls_256x(cscreen, 0xfa00);
        return;
    }
    if (screen_mode == 2) {
        if (internal_screen == 0)
            return;
        for (i = 0; i < 0x4b000; i++)
            internal_screen[i] = 0;
        return;
    }
    if (screen_mode == 3) {
        if (internal_screen == 0)
            return;
        for (i = 0; i < 0x3e800; i++)
            internal_screen[i] = 0;
    }
#endif
}

// Remap the 640×480 framebuffer through a lookup derived from the active palette.
// FUNCTION: C2 0x257c9
// FUNCTION: C2WIN 0x0044be4a
void grey_a_screen(void)
{
    int i;
    int offset;
    int value;
    int colour;

    if (screen_mode != 2)         return;
    if (internal_screen == 0)     return;

    for (i = 0; i < 0x100; i++)
    {
        offset = i * 3; value = (unsigned char)current_palette[offset];
        value += (unsigned char)current_palette[offset];
        value += (unsigned char)current_palette[offset]; value /= 3;
        greying_data[i] = (unsigned char)(0x3f - (value >> 1));
    }

    for (i = 0; i < 0x4b000; i++)
    {
        colour = internal_screen[i];
        internal_screen[i] = greying_data[colour];
    }
}

// Wipe all four 64K mode-X memory regions.
// FUNCTION: C2 0x25845
void clear_all_screens(void)
{
#if PLATFORM_WINDOWS
    extern HWND hWnd;
    extern void clear_window(HWND window, int x, int y, int width, int height);

    clear_window(hWnd, 0, 0, 0x280, 0x1e0);
#else
    cls_256x(0,      0x10000);
    cls_256x(0x4000, 0x10000);
    cls_256x(0x8000, 0x10000);
    cls_256x(0xa000, 0x10000);
#endif
}

// Handle CBC completion without performing any action.
// FUNCTION: C2 0x25880
void cbc_end(void)
{
}

#if PLATFORM_DOS

// BIOS int 10h fn 6: scroll up, full text screen with attribute 7 (effective text-mode CLS).
// FUNCTION: C2 0x25881
void dos_cls(void)
{
    union REGS r;
    r.w.cx  = 0;
    r.w.dx  = 0x1850;
    r.h.bh  = 7;
    r.w.ax  = 0x600;
    int386(0x10, &r, &r);
}
#else
extern void dos_cls(void);
#endif /* PLATFORM_DOS */

#pragma aux click_handler __loadds parm [eax] [ebx] [ecx] [edx] [esi] [edi];

// Real-mode mouse-callback (DOS int 0x33 function 0x0C). Installed by `install_mouse`; the mouse
// driver calls this with AX = event mask, BX = button state, CX = X, DX = Y, SI = mickey-X, DI =
// mickey-Y.
// FUNCTION: C2 0x258a4
void __far click_handler(unsigned int ax, unsigned int bx,
                         unsigned int cx, unsigned int dx,
                         unsigned int si, unsigned int di)
{
#if !PLATFORM_WINDOWS
    cbd.pending = 1;
    cbd.ax = ax;
    cbd.bx = bx;
    cbd.cx = cx;
    cbd.dx = dx;
    cbd.si = si;
    cbd.di = di;
    if ((cbd.ax & 8) != 0)
        cbd.click_flag = 1;
#endif
}

#pragma on(check_stack);

#if PLATFORM_DOS

// Initializes the mouse driver and installs the click callback.
// FUNCTION: C2 0x258f3
void install_mouse(void)
{
    union REGS out;
    union REGS in;
    struct SREGS sr;

    mouse_installed = 0;
    segread(&sr);
    in.w.ax = 0;
    int386(0x33, &in, &out);
    mouse_installed = (out.w.ax == -1);
    if (mouse_installed == 0) return;

    if (lock_region((int)&cbd, 0x14) == 0 ||
        lock_region((int)click_handler,
                    (char *)cbc_end - (char *)click_handler) == 0) {
        exit_flag = 1; return;
    }
    in.w.ax = 1;
    int386(0x33, &in, &out);
    in.w.ax = 9;
    in.w.bx = 0;
    in.w.cx = 0;
    in.x.edx = FP_OFF(mouse_ptr);
    sr.es = FP_SEG(mouse_ptr);
    int386x(0x33, &in, &out, &sr);
    in.w.ax = 0xc;
    in.w.cx = 1;
    in.x.edx = FP_OFF(click_handler);
    sr.es = FP_SEG(click_handler);
    int386x(0x33, &in, &out, &sr);
}
#else
extern void install_mouse(void);
#endif /* PLATFORM_DOS */

// Drain the deferred mouse-call buffer (cbd) into mse_x/mse_y/mse_button.
// FUNCTION: C2 0x25a0c
// FUNCTION: C2WIN 0x0044bf7b
void read_installed_mouse(void)
{
    if (mouse_installed == 0) return;
    if (cbd.pending != 0) {
        mse_x      = cbd.cx;
        mse_y      = cbd.dx;
        mse_button = (char)cbd.bx;
        cbd.pending = 0;
    }
}

#if PLATFORM_DOS

// Hide the mouse cursor (int 33h fn 2) then reset the mouse driver (int 33h fn 0) when one is
// installed.
// FUNCTION: C2 0x25a55
void de_install_mouse(void)
{
    union REGS in;
    union REGS out;
    if (mouse_installed == 0)
        return;
    in.w.ax = 2;
    int386(0x33, &in, &out);
    in.w.ax = 0;
    int386(0x33, &in, &out);
}

// Reset the mouse driver (int 33h fn 0); cache the result in `mouse_installed` and return it.
// FUNCTION: C2 0x25aa1
int init_mouse(void)
{
    union REGS in;
    union REGS out;
    memset(&in, 0, sizeof(in));
    in.w.ax = 0;
    int386(0x33, &in, &out);
    mouse_installed = out.w.ax;
    if (mouse_installed < 0)
        mouse_installed = 0;
    return mouse_installed;
}
#else
extern void de_install_mouse(void);
extern int init_mouse(void);
#endif /* PLATFORM_DOS */

// Constrain the mouse cursor to the active screen mode's resolution. No-op if no mouse is
// installed or screen_mode isn't 1/2/3.
// FUNCTION: C2 0x25af3
void set_mouse_limits(void)
{
    if (mouse_installed == 0)
        return;
    if (screen_mode == 1)
        mouserange(0, 0, 320, 200);
#if !PLATFORM_WINDOWS
    else if (screen_mode == 3)
        mouserange(0, 0, 640, 400);
#endif
    else if (screen_mode == 2)
        mouserange(0, 0, 640, 480);
}

#if PLATFORM_DOS

// Configure the int 33h driver's horizontal (fn 7) and vertical (fn 8) cursor limits. Zeroes the
// full REGS union before each int386 call so any reserved register slots are left clear.
// FUNCTION: C2 0x25b4e
void mouserange(int xmin, int ymin, int xmax, int ymax)
{
    union REGS r;
    int hi_x = xmax;
    memset(&r, 0, 0x1c);
    r.w.ax = 7;
    r.w.cx = xmin;
    r.w.dx = hi_x;
    int386(0x33, &r, &r);
    r.w.ax = 8;
    r.w.cx = ymin;
    r.w.dx = ymax;
    int386(0x33, &r, &r);
}

// Read the mouse position via int 33h fn 3 and update mse_x, mse_y and mse_button.
// FUNCTION: C2 0x25bb9
void read_mouse(void)
{
    union REGS r;
    memset(&r, 0, 0x1c);
    r.w.ax = 3;
    int386(0x33, &r, &r);
    mse_x = r.w.cx;
    mse_y = r.w.dx;
    mse_button = r.w.bx;
}
#else
extern int mouserange();

// FUNCTION: C2WIN 0x0044c055
void set_mouse(void)
{
    extern HWND hWnd;
    POINT point;
    point.x = mse_x;
    point.y = mse_y;
    ClientToScreen(hWnd, &point);
    SetCursorPos(point.x, point.y);
}

// FUNCTION: C2WIN 0x0044c0fb
void read_mouse(void)
{
    extern HWND main_window;
    extern HACCEL accelerator;
    extern void process_windows_message(MSG *message);
    MSG message;
    RECT rect;
    POINT point;
    int w;
    int h;

    if (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
        if (TranslateAccelerator(main_window, accelerator, &message) == 0) {
            TranslateMessage(&message);
            DispatchMessage(&message);
            process_windows_message(&message);
        }
    }
}
#endif /* PLATFORM_DOS */

// Poll the mouse until the user clicks any button.
// FUNCTION: C2 0x25c0c
// FUNCTION: C2WIN 0x0044c161
void wait_click(void)
{
    for (;;) {
        read_mouse();
        if (mse_button != 0)
            break;
    }
}

// Drain any held mouse buttons and reset the click/preclick latches.
// FUNCTION: C2 0x25c25
// FUNCTION: C2WIN 0x0044c18a
void clear_mouse(void)
{
    do {
        get_mouse();
    } while (mouse_left_button != 0 || mouse_right_button != 0);
    mouse_left_click = mouse_right_click = 0;
    mouse_left_preclick = mouse_right_preclick = 0;
}

// Update the cached (mouse_x, mouse_y) and push them to the driver via set_mouse (int 33h fn 4).
// FUNCTION: C2 0x25c60
// FUNCTION: C2WIN 0x0044c1da
void position_mouse(short x, short y)
{
    mouse_x = mse_x = x;
    mouse_y = mse_y = y;
    set_mouse();
}

#if PLATFORM_DOS
// Push the cached (mse_x, mse_y) to the mouse driver via int 33h fn 4.
// FUNCTION: C2 0x25c85
void set_mouse(void)
{
    union REGS r;
    memset(&r, 0, sizeof(r));
    r.w.ax = 4;
    r.w.cx = mse_x;
    r.w.dx = mse_y;
    int386(0x33, &r, &r);
}
#endif

// Pump the mouse driver and update the engine's mouse state. Tries sim_mouse() first
// (replay-from-recording / inter-net sync feeder); on no replay frame falls through to
// read_mouse() which polls INT 33h.
// FUNCTION: C2 0x25ccc
// FUNCTION: C2WIN 0x0044c216
void get_mouse(void)
{
#if !C2_FEAT_PRESERVE_MOUSE_SAMPLE
    int button_state;
#endif

    if (sim_mouse() == 0) {
        read_mouse();
    }
    mouse_movement = 0;
    old_mouse_x = mouse_x;
    old_mouse_y = mouse_y;
    old_mouse_lb = mouse_left_button;
    old_mouse_rb = mouse_right_button;
    mouse_x = mse_x;
    mouse_y = mse_y;

#if C2_FEAT_PRESERVE_MOUSE_SAMPLE
    mouse_left_button = mouse_left_preclick = mouse_left_click = 0;
    mouse_right_button = mouse_right_preclick = mouse_right_click = 0;
#else
    mouse_left_button   = 0;
    mouse_left_preclick = 0;
    mouse_left_click    = 0;
    mouse_right_button   = 0;
    mouse_right_preclick = 0;
    mouse_right_click    = 0;
#endif

    if ((mse_button & 2) != 0) mouse_right_button = 1;
    if ((mse_button & 1) != 0) mouse_left_button  = 1;

    if (old_mouse_x != mouse_x) mouse_movement = 1;
    if (old_mouse_y != mouse_y) mouse_movement = 1;

#if C2_FEAT_PRESERVE_MOUSE_SAMPLE
    if (mouse_left_button != old_mouse_lb) {
        mouse_movement    = 1;
        mouse_was_pressed = 1;
        if (mouse_left_button == 1) {
            mouse_left_preclick = 1;
        } else if (mouse_left_button == 0) {
            mouse_left_click = 1;
        }
    }
#else
    if (mouse_left_button != old_mouse_lb) {
        button_state = 1;
        mouse_movement    = button_state;
        mouse_was_pressed = button_state;
        if (mouse_left_button == button_state) {
            mouse_left_preclick = 1;
        } else if (mouse_left_button == 0) {
            mouse_left_click = 1;
        }
    }
#endif

#if C2_FEAT_PRESERVE_MOUSE_SAMPLE
    if (mouse_right_button != old_mouse_rb) {
        mouse_movement    = 1;
        mouse_was_pressed = 1;
        if (mouse_right_button == 1) {
            mouse_right_preclick = 1;
        } else if (mouse_right_button == 0) {
            mouse_right_click = 1;
        }
    }
#else
    if (mouse_right_button != old_mouse_rb) {
        button_state = 1;
        mouse_movement    = button_state;
        mouse_was_pressed = button_state;
        if (mouse_right_button == button_state) {
            mouse_right_preclick = 1;
        } else if (mouse_right_button == 0) {
            mouse_right_click = 1;
        }
    }
#endif

#if !C2_FEAT_PRESERVE_MOUSE_SAMPLE
    mse_button = 0;
#endif
}

// Mouse-cursor variant of write_image().
// FUNCTION: C2 0x25e33
// FUNCTION: C2WIN 0x0044c3d3
void show_mouse(int image_idx)
{
#if PLATFORM_WINDOWS
    extern void set_windows_mouse(char image_idx);

    if (image_idx == 0 || image_idx == 1 || image_idx == 2 ||
        image_idx == 3 || image_idx == 4 || image_idx == 6 ||
        image_idx == 7 || image_idx == 9 || image_idx == 0x11 ||
        image_idx == 0x13 || image_idx == 0x14) {
        set_windows_mouse(image_idx);
        return;
    }
#endif

    data_ptr = image_idx * 16 + 8;

    sprite_width  = mice[data_ptr]     + (mice[data_ptr + 1] << 8);
    sprite_height = mice[data_ptr + 2] + (mice[data_ptr + 3] << 8);
    sprite_start  = mice[data_ptr + 4] + (mice[data_ptr + 5] << 8);

    if (sprite_start > 0x4baf0) return;
    if (sprite_width  <= 0)     return;
    if (sprite_width  > 300)    return;
    if (sprite_height <= 0)     return;
    if (sprite_height > 300)    return;

    sprite_x = mouse_x;
    sprite_y = mouse_y;

    xclip(0, screen_width);
    yclip(0, screen_height);

    if (yclipped == 5) return;
    else if (xclipped == 1) {
        write_i_left_sprite(mice);
    } else if (xclipped == 2) {
        write_i_right_sprite(mice);
    } else {
        write_i_sprite(mice);
    }
}

// Snapshot the current mouse position into sprite_x / sprite_y, clamp to (0, 0) — (screen_w-24,
// screen_h-24), stash the clamped coords as the next-frame `old_mouse_drops_*`, and pick up the
// background tile under the cursor.
// FUNCTION: C2 0x25f52
// FUNCTION: C2WIN 0x0044c5dc
void get_mouse_droppings(void)
{
    sprite_x = mouse_x;
    sprite_y = mouse_y;
    if (sprite_x < 0)
        sprite_x = 0;
    if (sprite_y < 0)
        sprite_y = 0;
    if (screen_width  - 0x18 < sprite_x)
        sprite_x = screen_width  - 0x18;
    if (screen_height - 0x18 < sprite_y)
        sprite_y = screen_height - 0x18;
    old_mouse_drops_x = sprite_x;
    old_mouse_drops_y = sprite_y;
    pick_up_mouse_background(mouse_background);
}

// Restore the mouse-background sprite at the last drop coordinates; no-op while hold_mouse_replace
// is set.
// FUNCTION: C2 0x25fdf
// FUNCTION: C2WIN 0x0044c68c
void cover_mouse_droppings(void)
{
    if (hold_mouse_replace != 0) {
        hold_mouse_replace = 0;
        return;
    }
    sprite_x = old_mouse_drops_x;
    sprite_y = old_mouse_drops_y;
    put_down_mouse_background(mouse_background);
}

#if PLATFORM_WINDOWS
int get_windows_key(char *key);
#endif
void wait_key(void);

// 1 if (mouse_x, mouse_y) is inside the half-open rectangle [x, x+w) x [y, y+h), 0 otherwise.
// FUNCTION: C2 0x2601d
// FUNCTION: C2WIN 0x0044c6d3
int mouse_in_area(int x, int y, int w, int h)
{
    if (mouse_x >= x && w + x > mouse_x) {
        if (mouse_y >= y && h + y > mouse_y)
            return 1;
    }
    return 0;
}

#if PLATFORM_WINDOWS
// Poll for a Windows character message and return its byte value through key.
// FUNCTION: C2WIN 0x0044c731
int get_windows_key(char *key)
{
    extern HWND main_window;
    extern HACCEL accelerator;
    MSG message;

    if (PeekMessage(&message, 0, WM_CHAR, WM_CHAR, PM_REMOVE)) {
        if (TranslateAccelerator(main_window, accelerator, &message) == 0) {
            TranslateMessage(&message);
            DispatchMessage(&message);
            *key = (char)message.wParam;
            return 1;
        }
    }
    return 0;
}
#endif

// Non-blocking keyboard poll.
// FUNCTION: C2 0x26056
// FUNCTION: C2WIN 0x0044c7aa
void get_key(void)
{
#if PLATFORM_WINDOWS
    char key;

    key_ready = 0;
    key_ascii = 0;
    if (get_windows_key(&key)) {
        key_ascii_was = key_ascii;
        key_ready = 1;
        key_code = 0;
        key_ascii = key;
    }
#else
    key_ready = 0;
    key_ascii = 0;
    if (kbhit()) {
        key_ascii_was = key_ascii;
        key_ready = 1;
        key_code = 0;
        key_ascii = getch();
        if (key_ascii == 0) {
            key_code = getch();
        }
    }
#endif
}

// Drain the keyboard buffer then block until the next key event.
// FUNCTION: C2 0x260ab
// FUNCTION: C2WIN 0x0044c7fa
void wait_key(void)
{
    clear_keys();
    do {
        get_key();
    } while (key_ready == 0);
}

// Set key_ready and drain via get_key() until it clears.
// FUNCTION: C2 0x260c9
// FUNCTION: C2WIN 0x0044c81e
void clear_keys(void)
{
    key_ready = 1;
    while (key_ready != 0)
        get_key();
}

void pull_string_left(char *p, char *end);
void push_string_right(char *start, char *end);
void strip_fb_spaces(void);
unsigned char to_upper(unsigned char c);

// Bounded byte-compare of two strings: returns 1-based index of first mismatch, or 0 if all `n`
// bytes are equal.
// FUNCTION: C2 0x260ea
int my_strcmp(unsigned char *s1, unsigned char *s2, int n)
{
    int i;
    for (i = 0; i < n; i++) {
        if (s2[i] != s1[i]) return i + 1; }
    return 0;
}

// Copy exactly `n` bytes from src to dst.
// FUNCTION: C2 0x26118
// FUNCTION: C2WIN 0x0044c8a5
void my_strcpy(char *src, char *dst, int n)
{
    int i;
    for (i = 0; i < n; i++) dst[i] = src[i];
}

// Uppercase a single ASCII letter; non-letters pass through unchanged.
// FUNCTION: C2 0x2613e
// FUNCTION: C2WIN 0x0044c8e5
unsigned char to_upper(unsigned char c)
{
    if (c >= 'a' && c <= 'z')
        c -= 0x20;
    return c;
}

// Uppercase every ASCII letter in `s` in place.
// FUNCTION: C2 0x2615b
// FUNCTION: C2WIN 0x0044c91f
void string_to_upper(unsigned char *s)
{
    unsigned char c;
    while (*s != 0) {
        c = *s;
        if (c >= 'a' && c <= 'z')
            c -= 0x20;
        *s = c;
        s++;
    }
}

// Shift bytes of [p, end) one position left and null-terminate at *end.
// FUNCTION: C2 0x26187
// FUNCTION: C2WIN 0x0044c97b
void pull_string_left(char *p, char *end)
{
    while (p < end) { *p = *(p + 1); p++; }
    *p = 0;
}

// Shift bytes from `start` through `end - 1` one position right and terminate after `end`.
// FUNCTION: C2 0x261a5
// FUNCTION: C2WIN 0x0044c9ab
void push_string_right(char *start, char *end)
{
    end[1] = 0;
    while (end > start) { *end = end[-1]; end--; }
}

// Drop leading spaces by repeatedly shifting the string left until the first character is
// non-space.
// FUNCTION: C2 0x261c2
// FUNCTION: C2WIN 0x0044c9dc
void strip_leading_space(signed char *s)
{
    int i;
    while (*s == ' ') {
        i = 0;
        while (s[i] != 0) {
            s[i] = s[i + 1];
            i++;
        }
    }
}

// Strip trailing spaces from a string in place.
// FUNCTION: C2 0x261ef
// FUNCTION: C2WIN 0x0044ca32
void strip_trailing_space(signed char *s)
{
    int i;
    i = 0;
    while (s[i] != 0)
        i++;
    i--;
    while (s[i] == ' ') {
        s[i] = 0;
        i--;
    }
}

// Collapse repeated spaces in a NUL-terminated string.
// FUNCTION: C2 0x2621e
// FUNCTION: C2WIN 0x0044ca89
void strip_spaces(char *s)
{
    int i;
    int previous_was_space = 0;
    int len;

    for (i = 0; i < 0xfa00; i++) {
        if (s[i] == 0) {
            len = i;
            break;
        }
    }
    for (i = 0; i < len; i++) {
        if (s[i] == 0) return;
        if ((signed char)s[i] == ' ') {
            if (previous_was_space) {
                i--;
                pull_string_left(s + i, s + len);
            } else {
                previous_was_space = 1;
            }
        } else {
            previous_was_space = 0;
        }
    }
}

// Measure a NUL-terminated string in pixels using a variable-width game font.
// FUNCTION: C2 0x26284
// FUNCTION: C2WIN 0x0044cb61
int get_string_width(char *src, unsigned char *font)
{
    int total;
    int remaining_chars;
    unsigned char c;

    remaining_chars = 0x2710;
    total = 0;
    while (remaining_chars > 0) {
        c = *src;
        src++;
        if (c == 0) return total;
        if (c == ' ') {
            total += 4;
        } else {
            sprite_image_no = letter_table[c - ' '];
            if (sprite_image_no != 0) {
                sprite_image_no--;
                data_ptr = sprite_image_no * 16 + 8;
                total += font[data_ptr] + font[data_ptr + 1] * 256;
                total++;
            }
        }
        remaining_chars--;
    }
    return total;
}

// Pixel width of a single ASCII character in the given font.
// FUNCTION: C2 0x26305
// FUNCTION: C2WIN 0x0044cc2a
int get_letter_width(unsigned char letter, unsigned char *font)
{
    int width;

    width = 0;
    if (letter == 0) return 0;
    if (letter == ' ') return 4;
    sprite_image_no = letter_table[letter - 0x20];
    if (sprite_image_no == 0) return width;
    sprite_image_no = sprite_image_no - 1;
    data_ptr = sprite_image_no * 16 + 8;
    width = font[data_ptr] + font[data_ptr + 1] * 0x100;
    width++;
    return width;
}

// Parse a leading run of ASCII digits from `text` and return its decimal value. Walks the text
// twice: first to count digits, then back from the start applying `multiples[]` (1, 10, 100, …)
// right-to-left so each digit gets the right place value.
// FUNCTION: C2 0x26367
// FUNCTION: C2WIN 0x0044ccd7
int get_number_from_text(char *text)
{
    unsigned char *digit_ptr;
    int total;
    int digits;

    digit_ptr = (unsigned char *)text;
    digits = total = 0;
    while (*digit_ptr >= '0' && *digit_ptr <= '9') {
        digits = digits + 1;
        digit_ptr = digit_ptr + 1;
    }
    digit_ptr = (unsigned char *)text;
    while (digits != 0) {
        int digit_value;
        digits = digits - 1;
        digit_value = (unsigned char)(*digit_ptr) - '0';
        total = total + digit_value * multiples[digits];
        digit_ptr = digit_ptr + 1;
    }
    return total;
}

// Copy `n` bytes from `src` into the `idx`-th text-buffer slot. The slot's payload offset lives in
// two big-endian bytes at text_buffer[idx*4 + 0x1e..0x1f]; the actual data starts at
// text_buffer[0x1c + offset].
// FUNCTION: C2 0x263af
// FUNCTION: C2WIN 0x0044cd64
void load_to_text_buffer(char *src, int entry_idx, int word_count, int copy_len)
{
    int text_offset;
    char *buffer_ptr;
    unsigned char char_idx;
    unsigned char offset_byte;

    buffer_ptr = text_buffer;
    offset_byte = text_buffer[entry_idx * 4 + 0x1e];
    text_offset  = offset_byte;
    text_offset  = text_offset << 8;
    offset_byte = text_buffer[entry_idx * 4 + 0x1f];
    text_offset  = text_offset + offset_byte;
    text_offset = text_offset + 0x1c;
    buffer_ptr = buffer_ptr + text_offset;
    while (word_count > 0) {
        if (*buffer_ptr == 0 && (signed char)*(buffer_ptr - 1) >= ' ')
            word_count--;
        buffer_ptr++;
    }
    while ((signed char)*buffer_ptr < ' ')
        buffer_ptr++;
    for (char_idx = 0; char_idx < copy_len; char_idx++)
        buffer_ptr[char_idx] = src[char_idx];
}

// Copy bytes from a selected word position in a text-buffer entry into `dst`.
// FUNCTION: C2 0x2641a
// FUNCTION: C2WIN 0x0044ce30
void load_from_text_buffer(char *dst, int entry_idx, int word_count, int copy_len)
{
    int text_offset;
    char *buffer_ptr;
    unsigned char char_idx;
    unsigned char offset_byte;

    buffer_ptr = text_buffer;
    offset_byte = text_buffer[entry_idx * 4 + 0x1e];
    text_offset  = offset_byte;
    text_offset  = text_offset << 8;
    offset_byte = text_buffer[entry_idx * 4 + 0x1f];
    text_offset  = text_offset + offset_byte;
    text_offset = text_offset + 0x1c;
    buffer_ptr = buffer_ptr + text_offset;
    while (word_count > 0) {
        if (*buffer_ptr == 0 && (signed char)*(buffer_ptr - 1) >= ' ')
            word_count--;
        buffer_ptr++;
    }
    while ((signed char)*buffer_ptr < ' ')
        buffer_ptr++;
    for (char_idx = 0; char_idx < copy_len; char_idx++)
        dst[char_idx] = buffer_ptr[char_idx];
}

// Returns the 24-bit little-endian value at text_buffer[idx*4 + 8..10]: (high<<16) + (mid<<8) +
// low.
// FUNCTION: C2 0x26485
// FUNCTION: C2WIN 0x0044cefc
int get_buffer_ofset(int entry_idx)
{
    int offset;
    int offset_byte;

    offset_byte  = (unsigned char)text_buffer[entry_idx * 4 + 0xa];
    offset_byte <<= 16;
    offset  = offset_byte;
    offset_byte  = (unsigned char)text_buffer[entry_idx * 4 + 9];
    offset_byte <<= 8;
    offset += offset_byte;
    offset_byte  = (unsigned char)text_buffer[entry_idx * 4 + 8];
    offset += offset_byte;
    return offset;
}

// Walk the text buffer from the entry's offset, skipping `word_count` tokens (anything preceded by
// a NUL terminator), then strip leading control characters. Returns the resulting `text_pointer`.
// FUNCTION: C2 0x264bd
// FUNCTION: C2WIN 0x0044cf59
void get_text_pointer(int entry_idx, int word_count)
{
    text_pointer = text_buffer;
    text_pointer += get_buffer_ofset(entry_idx);

    while (word_count > 0) {
        if ((unsigned char)*text_pointer == 0 &&
            ((unsigned char)*(text_pointer - 1) >= ' ' ||
             (unsigned char)*(text_pointer - 1) == 0)) word_count--;
        text_pointer++;
    }

    while ((unsigned char)*text_pointer < ' ')
        text_pointer++;

}

// One-iteration handler for the in-place format_buffer text editor (used by the "enter a name"
// prompts). Reads one keystroke from the keyboard polling globals (key_ready / key_ascii /
// key_code) and updates the editor cursor and buffer contents.
// FUNCTION: C2 0x26518
// FUNCTION: C2WIN 0x0044cfeb
int edit_format_buffer(void)
{
    int i;

    if (this_letter > fb_current_char_length)
        this_letter = fb_current_char_length;
    if (this_letter < 0)
        this_letter = 0;

    if (key_ready != 1) return 0;

    if (key_ascii == 0x1b) return 0;
    if (key_ascii == 0x0d) return 1;

    test_for_delimiter();
    if (key_ascii == 0) {
        /* Function keys / arrows / Home / End / Insert. */
        if (key_code == 0x53) del_fb();           /* Delete */
        if (key_code == 0x52) insert_cursor ^= 1; /* Insert */
        if (key_code == 0x47) this_letter = 0;    /* Home   */

        if (key_code == 0x4b && this_letter > 0)  /* Left   */
            this_letter--;

        if (key_code == 0x48) {                    /* Up     */
            for (i = 0; i < 0xa; i++)
                if (this_letter > 0) this_letter--;
        }
        if (at_limit != 0) return 0;

        if (key_code == 0x4d                       /* Right  */
            && this_letter < fb_current_char_length)
            this_letter++;

        if (fb_limit == 1) return 0;

        if (key_code == 0x50) {                    /* Down   */
            for (i = 0; i < 0xa; i++)
                if (this_letter < fb_current_char_length) this_letter++;
        }
        if (key_code == 0x4f)                      /* End    */
            this_letter = fb_current_char_length;
        return 0;
    }

    /* Printable / control: backspace and the per-codepage whitelist. */
    if (key_ascii == 8) {                          /* BS     */
        if (this_letter > 0) {
            this_letter--;
            at_limit = 0;
            del_fb();
            return 0;
        }
    }
    if (at_limit == 2) return 0;

    if (key_ascii == ' ') to_fb();
    else if (key_ascii == '\\') to_fb();
    else if (key_ascii == ',') to_fb();
    else if (key_ascii == '?') to_fb();
    else if (key_ascii == '\'') to_fb();
    else if (key_ascii == '!') to_fb();
    else if (key_ascii == '"') to_fb();
    else if (key_ascii >= '0' && key_ascii <= '9') to_fb();
    else if (key_ascii >= 'a' && key_ascii <= 'z') to_fb();
    else if (key_ascii >= 'A' && key_ascii <= 'Z') to_fb();
    else if (key_ascii >= 0x80 && key_ascii <= 0x9a) to_fb();
    else if (key_ascii >= 0xa0 && key_ascii <= 0xa7) to_fb();
    else if (key_ascii == 0xe1) to_fb();

    if (this_letter >= fb_max_char_length)
        this_letter = fb_max_char_length;
    if (this_letter < 0)
        this_letter = 0;
    return 0;
}

// Insert / overwrite the current `key_ascii` byte at `this_letter` inside `format_buffer`, advance
// the cursor, and shuffle subsequent characters right when in insert mode. Used by the on-screen
// text editor.
// FUNCTION: C2 0x267b3
// FUNCTION: C2WIN 0x0044d41a
void to_fb(void)
{
    int cursor_idx;
    int last_idx;

    if (fb_limit == 2) { if (this_letter > fb_current_char_length) return;
    } else {
        if (this_letter >= fb_current_char_length) return;
    }

    cursor_idx = this_letter;
    if (insert_cursor || at_limit == 1) {
        /* Insert a character and shift the remaining text right. */
        if (fb_max_width_reached) return;
        if (fb_current_char_length < fb_max_char_length) {
            push_string_right(&format_buffer[this_letter],
                              &format_buffer[fb_current_char_length + 1]);
            format_buffer[this_letter] = key_ascii;
            this_letter = this_letter + 1;
        }
    } else {
        /* Overwrite the current character and advance when space permits. */
        format_buffer[this_letter] = key_ascii;
        if (fb_max_width_reached) return;
        if (fb_current_char_length >= fb_max_char_length) {
            if (this_letter < fb_current_char_length)
                this_letter = this_letter + 1;
            return;
        }
        if (fb_limit == 2)
            this_letter = this_letter + 1;
        else if (this_letter < fb_current_char_length - 1)
            this_letter = this_letter + 1;
    }
}

// Delete a single character from the format buffer, shifting the tail left by one. Skipped when
// at_limit is set.
// FUNCTION: C2 0x2689d
// FUNCTION: C2WIN 0x0044d568
void del_fb(void)
{
    if (at_limit != 0) return;
    pull_string_left(&format_buffer[this_letter],
                     &format_buffer[fb_current_char_length]);
}

// Copy 2000 bytes (the full format buffer) from src to dst.
// FUNCTION: C2 0x268ce
// FUNCTION: C2WIN 0x0044d5a3
void copy_fb(char *src, char *dst)
{
    int i;
    for (i = 0; i < 2000; i++)
        dst[i] = src[i];
}

// Update at_limit based on the character at format_buffer[this_letter] and the current fb_limit
// setting.
// FUNCTION: C2 0x268f1
// FUNCTION: C2WIN 0x0044d5e4
void test_for_delimiter(void)
{
    at_limit = 0;
    if (format_buffer[this_letter] == 0)
        at_limit = 1;
    if (fb_limit != 1) return;
    if (format_buffer[this_letter] != '.') return;
    if (this_letter < 8)
        at_limit = 1;
    else
        at_limit = 2;
}

// Walk the format buffer and pull-string-left every space character up to fb_current_char_length,
// returning early on the first NUL.
// FUNCTION: C2 0x26955
// FUNCTION: C2WIN 0x0044d66b
void strip_fb_spaces(void)
{
    int i;
    for (i = 0; i <= fb_current_char_length; i++) {
        if (format_buffer[i] == 0)
            return;
        if (format_buffer[i] == ' ')
            pull_string_left(&format_buffer[i],
                             &format_buffer[fb_current_char_length]);
    }
}

// Recompute fb_current_char_length from the format_buffer's NUL.
// FUNCTION: C2 0x2699e
// FUNCTION: C2WIN 0x0044d6e4
void get_fb_length(void)
{
    int i = 0;
    fb_current_char_length = 0;
    while (format_buffer[i++] != 0)
        fb_current_char_length++;
}

// Sums get_letter_width() across every char in the null-terminated `format_buffer`.
// FUNCTION: C2 0x269c8
// FUNCTION: C2WIN 0x0044d72a
int get_fb_width(unsigned char *font)
{
    int char_idx;
    int width;
    unsigned char c;

    char_idx = 0;
    width = 0;
    while ((c = format_buffer[char_idx++]) != 0) {
        width = width + get_letter_width(c, font);
    }
    return width;
}

void in_format_buffer(char *src, int max_chars, int line_width, int edit_limit);

// Word-wrap measurement: walk the format_buffer and return the number of display lines the text
// would occupy given a maximum of fb_line_length pixels per line, 0x32 (=50) characters per line,
// and 0x64 (=100) total lines.
// FUNCTION: C2 0x269fb
// FUNCTION: C2WIN 0x0044d782
int get_fb_lines(void)
{
    int line_no;
    int pixels_across;
    int num_chars;
    int last_space;
    unsigned char c;
    int keep_going;
    int line_offset;
    char *fb = format_buffer;
    int linehead;
    int letter_width;

    keep_going = 1; line_no = 0; linehead = 0;
    while (line_no < 0x64 && keep_going) {
        line_offset = linehead; num_chars = pixels_across = last_space = 0;
        while (num_chars < 0x32 && keep_going
               && pixels_across < fb_line_length) {
            c = (unsigned char)fb[line_offset];
            if (c == 0) { keep_going = 0; last_space = line_offset;
            } else if (c == ' ') { last_space = line_offset; pixels_across += 4;
            } else if (c == '#') { pixels_across += 0x64;
            }
            num_chars++; line_offset++;
        }
        if (!last_space) { line_offset--; last_space = line_offset;
        }
        line_no++; linehead = last_space + 1;
    }
    return line_no;
}

// Initialise format_buffer for editing. Wipes all 0x7D0 bytes (= 2000) of the buffer to 0, then
// copies the source string into it (NUL-terminator excluded).
// FUNCTION: C2 0x26a8d
// FUNCTION: C2WIN 0x0044d88b
void in_format_buffer(char *src, int max_chars, int line_width, int edit_limit)
{
    int i;

    for (i = 0x7cf; i >= 0; i--) format_buffer[i] = 0;
    while (*src) format_buffer[++i] = *src++;
    fb_current_char_length = i + 1;
    fb_max_char_length = max_chars;
    fb_line_length = line_width;
    fb_limit = edit_limit;
    fb_max_width_reached = 0;
    cursor_y = 0;
}

// Copy format_buffer (a NUL-terminated string) into `out`, null terminator included.
// FUNCTION: C2 0x26af5
// FUNCTION: C2WIN 0x0044d91d
void out_format_buffer(char *out)
{
    int i = 0;
    while (format_buffer[i] != 0) {
        *out = format_buffer[i];
        i++;
        out++;
    }
    *out = 0;
}

void put_a_font_string(char *str, int x, int y, unsigned char *font, int color);

// Read the entry's offset word from the table at file offset idx*4 + 0x1E (big-endian on disk),
// then load 0x7D0 bytes from (offset + 0x1C) into format_buffer.
// FUNCTION: C2 0x26b1a
// FUNCTION: C2WIN 0x0044d964
void load_format_buffer_from_disk(char *filename, int entry_idx)
{
    int word_value;
    unsigned char *p;

    readfile(filename, (char *)&word_value, 2, entry_idx * 4 + 0x1e);

    /* Byte-swap the 16-bit word (BE on disk → LE in memory). */
    word_value = ((word_value & 0xff) << 8)
               + ((word_value & 0xff00) >> 8);

    readfile(filename, format_buffer, 0x7d0, word_value + 0x1c);

    fb_max_char_length = 0;
    p = format_buffer;
    while (*p != 0) { p++; fb_max_char_length++; }

    fb_line_length  = 0xe0;
    fb_no_of_lines  = 4;
    fb_limit        = 0;
}

// Read the entry's offset word from the table at file offset idx*4 + 0x1E (big-endian on disk),
// pad the format_buffer past its first NUL with spaces, and write fb_max_char_length bytes back at
// (offset + 0x1C).
// FUNCTION: C2 0x26bab
// FUNCTION: C2WIN 0x0044da0c
void save_format_buffer_to_disk(char *filename, int entry_idx)
{
    int word_value;
    int buffer_idx;
    int found_zero;

    readfile(filename, (char *)&word_value, 2, entry_idx * 4 + 0x1e);

    /* Byte-swap the 16-bit word (BE on disk → LE in memory). */
    word_value = ((word_value & 0xff) << 8)
               + ((word_value & 0xff00) >> 8);

    /* Replace anything past the first NUL with spaces. */
    buffer_idx = found_zero = 0;
    while (buffer_idx < fb_max_char_length) {
        if (format_buffer[buffer_idx] == 0) found_zero = 1;
        if (found_zero) format_buffer[buffer_idx] = ' ';
        buffer_idx++;
    }

    write_to_file(filename, format_buffer,
                  fb_max_char_length, word_value + 0x1c);
}

int one_letter(unsigned char *font, unsigned char letter);

// Render a printable string `str` at pixel (x, y) using the bitmap font (font1 / font2). `color`
// is the palette index for solid pixels; 0 selects the shadow font_style with the default colour.
// FUNCTION: C2 0x26c2e
// FUNCTION: C2WIN 0x0044dabc
void put_a_font_string(char *str, int x, int y, unsigned char *font, int color)
{
    unsigned char prev_char;
    unsigned char ch;
    unsigned char letter_count;
    int letter_width;

    font_style = 1;
    if (color != 0) { font_style = 0; }
    sprite_colour = color;
    sprite_x      = x;

    ch = *str;
    letter_count = 0;
    while (ch != 0) {
        prev_char = ch;

        if (ch == '#') {
            if (insert_place == 1) { ch = get_insert_letter();
            } else { ch = 0x20; }
        }

        if (ch == '_') { ch = 0x20; }

        if (ch < 0x20)
            goto skip_letter;
        ch -= 0x20;

        if (fb_count == this_letter && got_cursx == 0) { cursor_x = x_is; got_cursx = 1; }

        sprite_y = y;

        if (letter_table[(unsigned char)ch] > 0) { letter_width = one_letter(font, ch);
        } else { letter_width = 4; }

        sprite_x += letter_width;
        x_is     += letter_width;

skip_letter:
        if (insert_count != 0) { ch = get_insert_letter();
        } else {
            str++;
            ch = *str;
        }
        letter_count++;
        fb_count++;

        if (allow_padding == 0 && padding_off != 0) {
            if (prev_char == ch && (ch == 0x20 || ch == '_')) {
                x_is -= letter_width; sprite_x -= letter_width; } }
    }

    x_is += 4;
    insert_place      = 0;
    insert_count      = 0;
    allow_padding     = 0;
    font_screen_limit = 0;
}

// Render a single bitmap-font glyph for `letter` from the font table (font1 or font2) at the
// current sprite position with the supplied clipping box.
// FUNCTION: C2 0x26d83
// FUNCTION: C2WIN 0x0044dcb8
int one_letter(unsigned char *font, unsigned char letter)
{
    sprite_image_no = letter_table[letter];
    if (sprite_image_no == 0) {
        return 0;
    }

    sprite_image_no = sprite_image_no - 1;
    data_ptr = (sprite_image_no << 4) + 8;

    sprite_y += (unsigned char)font[data_ptr + 13];

    if (font == font1) {
        if (letter >= 'a' && letter <= 'm') {
            sprite_y -= 1;
        }
        if (letter >= 's' && letter <= 'w') {
            sprite_y -= 1;
        }
        if (letter >= 0x80 && letter <= 0x84) {
            sprite_y -= 1;
        }
    }

    sprite_width  = font[data_ptr] + (font[data_ptr + 1] << 8);
    sprite_height = font[data_ptr + 2] + (font[data_ptr + 3] << 8);
    sprite_start  = font[data_ptr + 4] + (font[data_ptr + 5] << 8) +
                    (font[data_ptr + 6] << 16);

    if (font_screen_limit != 0) {
#if PLATFORM_WINDOWS
        xclip(0, 0x280);
        yclip(pm_diamond_height + pm_screen_y_start, pm_screen_y_end);
#else
        xclip(pm_screen_x_start, 0x1de);
        yclip(0x18, pm_screen_y_end);
#endif
    } else {
        xclip(0, screen_width);
        yclip(0, screen_height);
    }

    if (yclipped == 5)
        goto done;
    if (xclipped == 1) {
        write_i_left_font(font);
    } else if (xclipped == 2) {
        write_i_right_font(font);
    } else {
        write_i_font(font);
    }

done:
    return sprite_width + 1;
}

// Pull the next typed-insert character, rewinding and returning a space at the terminator.
// FUNCTION: C2 0x26efe
// FUNCTION: C2WIN 0x0044dedb
char get_insert_letter(void)
{
    unsigned char c = insert_text[insert_count];
    if (c == 0) {
        insert_count = 0;
        return ' ';
    }
    insert_count++;
    return c;
}

// Select a text-buffer substring, center it within `total_width`, and render it.
// FUNCTION: C2 0x26f2e
// FUNCTION: C2WIN 0x0044df23
void font_centre(int entry_idx, int word_count, int x_left, int y,
                 int total_width, unsigned char *font, int color)
{
    int width;
    int offset;

    text_pointer = text_buffer;
    text_pointer += get_buffer_ofset(entry_idx);

    while (word_count > 0) {
        if ((unsigned char)*text_pointer == 0 && ((unsigned char)*(text_pointer - 1) >= ' ' || (unsigned char)*(text_pointer - 1) == 0)) word_count--;
        text_pointer++;
    }

    while ((unsigned char)*text_pointer < ' ') text_pointer++;

    width  = get_string_width(text_pointer, font);
    offset = (total_width - width) / 2;
    if (offset < 0) offset = 0;

    put_a_font_string(text_pointer, x_left + offset, y, font, color);
    font_screen_limit = 0;
}

// Select a text-buffer substring and render it at the requested position.
// FUNCTION: C2 0x26fcf
// FUNCTION: C2WIN 0x0044e018
void font_list(int entry_idx, int word_count, int x, int y, unsigned char *font, int color)
{
    text_pointer = text_buffer;
    text_pointer += get_buffer_ofset(entry_idx);

    while (word_count > 0) {
        if ((unsigned char)*text_pointer == 0 && ((unsigned char)*(text_pointer - 1) >= ' ' || (unsigned char)*(text_pointer - 1) == 0)) word_count--;
        text_pointer++;
    }

    while ((unsigned char)*text_pointer < ' ') text_pointer++;

    put_a_font_string(text_pointer, x, y, font, color);
    font_screen_limit = 0;
}

// Render an integer right-aligned into a static 16-byte scratch buffer (filled with `pad_char` for
// the digit field, then `suffix` at positions 10..15), strip the leading padding, and hand the
// result to put_a_font_string at (x, y) in the supplied font.
// FUNCTION: C2 0x2704e
// FUNCTION: C2WIN 0x0044e0d2
void font_no(int value, char pad_char, char *suffix, int x,
             int y, unsigned char *font, int color)
{
    char *buffer;
    int i;
    char zero;

    zero = 0;
    buffer = "                ";  /* 16 spaces plus NUL */
    if (pad_char != 0) for (i = 9; i >= 0; i--) buffer[i] = pad_char;

    i = 10;
    while (*suffix != 0) {
        buffer[i] = *suffix++;
        i++; if (i >= 16) break;
    }
    buffer[i] = 0;

    for (i = 9; i >= 0; i--) {
        if ((value <= 0) && (i != 9) && !zero) zero = 1;
        else if ((value <= 0) && (i != 9) && zero) buffer[i] = ' ';
        else buffer[i] = (char)((value % 10) + '0');
        value = value / 10;
    }

    strip_leading_space((signed char *)buffer);
    put_a_font_string(buffer, x, y, font, color);
    font_screen_limit = 0;
}

int get_next_word_length(char *src, unsigned char *font);
void font_format_split(int idx, int word_skip, int x, int y_start,
                       int max_width, int line_limit,
                       int x_overflow, int max_width_overflow,
                       unsigned char *font, int color);

// Word-wrap a text-buffer entry and render the requested number of lines within `max_width`.
// FUNCTION: C2 0x2712a
// FUNCTION: C2WIN 0x0044e235
void font_format_split(int idx, int word_skip, int x, int y_start,
                       int max_width, int line_limit,
                       int x_overflow, int max_width_overflow,
                       unsigned char *font, int color)
{
    int   line_count;
    int   draw_y;
    int   looping;
    int   x_count;
    int   format_count;
    int   first_char;
    int   counter;
    unsigned char c;

    font_screen_limit = 0;
    text_pointer      = text_buffer;
    text_pointer     += get_buffer_ofset(idx);

    /* Skip word_skip whole words from the entry's offset. */
    while (word_skip > 0) {
        if ((unsigned char)*text_pointer == 0 && ((unsigned char)*(text_pointer - 1) >= ' ' || (unsigned char)*(text_pointer - 1) == 0)) { word_skip--; }
        text_pointer++;
    }

    /* Strip leading control bytes (any byte < ' '). */
    while ((unsigned char)*text_pointer < ' ') { text_pointer++; }

    looping = 1; line_count = 0; draw_y = y_start;

    while (looping) {
        for (counter = 0; counter < 0x7d0; counter++)
            format_buffer[counter] = 0;

        x_count = 0; format_count = 0;
        first_char = 1;

        while (looping && x_count < max_width) {
            x_count += get_next_word_length(text_pointer,
                                            font);
            if (x_count < max_width) {

                for (counter = 0; counter < char_count; counter++) {
                    c = *text_pointer++;
                    if (first_char != 0) {
                        if (c == ' ') continue;
                    }
                    format_buffer[format_count++] = c;
                    first_char = 0;
                }

                if ((unsigned char)*text_pointer == 0) { looping = 0; }
            }
        }

        insert_place  = 1;
        x_is          = 0;       allow_padding = 1;
        put_a_font_string(format_buffer, x, draw_y, font, color);

        line_count++; draw_y += 0x10;

        if (line_count >= line_limit) { x = x_overflow; max_width = max_width_overflow; }
    }
}

// Returns the rendered pixel width of the next word in a text string.
// FUNCTION: C2 0x2729b
// FUNCTION: C2WIN 0x0044e448
int get_next_word_length(char *src, unsigned char *font)
{
    int width;
    int found_char;
    int n;
    int loops;
    unsigned char c;

    char_count = 0;
    width = 0; n = 0; found_char = 0;
    while (++n < 0x7d0) {
        c = *src; src++;
        if (c == 0) return width;
        if (c == ' ') { if (found_char != 0) return width; else width += 4;
        } else if (c == '$') { if (found_char != 0) return width; else { }
        } else {
            if (c < ' ') goto next;
            width += get_letter_width(c, font); found_char = 1;
        }
    next: char_count++;
    }
    return width;
}

// If the user just pre-clicked inside the text rect [x,x+w)×[y,y+h), remember the click's
// y-coordinate.
// FUNCTION: C2 0x27309
// FUNCTION: C2WIN 0x0044e53d
void click_on_text(int x, int y, int w, int h)
{
    if (cursor_y != 0)                  return;
    if (mouse_left_preclick == 0)       return;
    if (x > mouse_x || x + w <= mouse_x) return;
    if (y > mouse_y || y + h <= mouse_y) return;
    cursor_y = y;
}

// Like clicked_delay() but runs the inner poll for a fixed 8000 iterations and returns void: a
// settle (warmup) loop of 1000 get_mouse() calls followed by `delay` outer ticks of 8000 inner
// polls.
// FUNCTION: C2 0x2734d
// FUNCTION: C2WIN 0x0044e5c2
void click_delay(int delay)
{
    int i;
    int j;
    int k;
    for (i = 0; i < 1000; i++) get_mouse();
    for (j = 0; j < delay; j++) {
        for (k = 0; k < 8000; k++) {
            get_mouse();
            if (mouse_left_click != 0 || mouse_right_click != 0) { clear_mouse(); return; }
        } }
}

int clicked_delay(int delay);

// Waits for a click or timeout and reports whether a click occurred.
// FUNCTION: C2 0x273a4
// FUNCTION: C2WIN 0x0044e664
int clicked_delay(int delay)
{
    int i;
    int j;
    int k;
    for (i = 0; i < 1000; i++) get_mouse();
    for (j = 0; j < delay; j++) {
        for (k = 0; k < 8000; k++) {
            get_mouse();
            if (mouse_left_click != 0 || mouse_right_click != 0) { clear_mouse(); return 1; }
        } }
    return 0;
}

// Crude busy-wait: `n` * 25 vertical-blank cycles.
// FUNCTION: C2 0x27402
// FUNCTION: C2WIN 0x0044e712
void do_delay(int n)
{
    int i;
    int j;
    for (i = 0; i < n; i++)
        for (j = 0; j < 25; j++) wvbl2();
}

// Returns the number of milliseconds since the previous call, or 999 if the wallclock went
// backwards (clock skew / wrap). Latches `tb.time` into the global `time_is` so the rest of the
// engine can read it without re-calling ftime().
// FUNCTION: C2 0x2742b
// FUNCTION: C2WIN 0x0044e763
int running_delay1(void)
{
    static int running_delay_last;
    struct timeb tb;
    int current;
    int elapsed;

    ftime(&tb);
    time_is = current = tb.time;
    current = current * 1000;
    current = current + tb.millitm;
    if (current >= running_delay_last)
        elapsed = current - running_delay_last;
    else
        elapsed = 999;
    running_delay_last = current;
    return elapsed;
}

// Report when `delay_ms` has elapsed since this palette-cycle gate last fired.
// FUNCTION: C2 0x27483
char colour_cycle_delay1(int delay_ms)
{
    struct timeb tb;
    short ms;
    static short last_cycle_ms1;
    short elapsed;

    ftime(&tb);
    ms = tb.millitm;

    if (ms > last_cycle_ms1) { elapsed = ms - last_cycle_ms1;
    } else if (ms < last_cycle_ms1) { elapsed = (ms + 1000) - last_cycle_ms1;
    } else {
        elapsed = 0;
    }

    if ((int)elapsed >= delay_ms) { last_cycle_ms1 = ms; return 1;
    }
    return 0;
}

// Independently report when `delay_ms` has elapsed for a second palette-cycle gate.
// FUNCTION: C2 0x274d8
char colour_cycle_delay2(int delay_ms)
{
    static short last_cycle_ms2;
    struct timeb tb;
    short ms;
    short delta;

    ftime(&tb);
    ms = tb.millitm;

    if (ms > last_cycle_ms2) { elapsed = ms - last_cycle_ms2;
    } else if (ms < last_cycle_ms2) { elapsed = (ms + 1000) - last_cycle_ms2;
    } else {
        elapsed = 0;
    }

    if ((int)elapsed >= delay_ms) { last_cycle_ms2 = ms;
        return 1;
    }
    return 0;
}

// Start a millisecond stopwatch with mode 0 and read its elapsed time with mode 1.
// FUNCTION: C2 0x27522
// FUNCTION: C2WIN 0x0044e926
int timer(int mode)
{
    static int unused_timer_state;
    static int start_ms;
    static int start_sec;
    struct timeb tbuff;
    int now_ms;
    int now_sec;
    int delta;

    ftime(&tbuff);
    if (mode == 0) {
        start_ms = (int)(unsigned short)tbuff.millitm;
        start_sec = tbuff.time;
        return 0;
    } else if (mode == 1) {
        now_ms = (int)(unsigned short)tbuff.millitm;
        now_sec = tbuff.time;
        delta = (now_sec - start_sec) * 1000;
        if (now_ms >= start_ms) {
            delta += now_ms - start_ms;
        } else {
            delta += (now_ms + 0x3e8) - start_ms;
        }
        return delta;
    }
    return 0;
}

#if PLATFORM_DOS

// Play a short 880 Hz beep.
// FUNCTION: C2 0x2759c
// FUNCTION: C2WIN 0x0044e9f8
void high_beep(void)
{
    sound(0x370);
    delay(50);
    nosound();
}

// Short beep at 220 Hz (0xDC).
// FUNCTION: C2 0x275bf
// FUNCTION: C2WIN 0x0044ea10
void low_beep(void)
{
    sound(0xdc);
    delay(50);
    nosound();
}
#else
void vhigh_beep(void);

void high_beep(void)
{
    Beep(0x370, 50);
}

void low_beep(void)
{
    Beep(0xdc, 50);
}
#endif /* PLATFORM_DOS */

// Emit `n` high beeps with a 1-tick delay between each.
// FUNCTION: C2 0x275d0
// FUNCTION: C2WIN 0x0044ea28
void no_high_beeps(int count)
{
    while (count != 0) {
        high_beep();
        count--;
        do_delay(1);
    }
}

// Emit `n` low beeps with a 1-tick delay between each.
// FUNCTION: C2 0x275f7
// FUNCTION: C2WIN 0x0044ea54
void no_low_beeps(int count)
{
    while (count != 0) {
        low_beep();
        count--;
        do_delay(1);
    }
}

// Play the sequence of tones used by the audio test.
// FUNCTION: C2 0x2761e
// FUNCTION: C2WIN 0x0044ea80
void test_beeps(void)
{
    vhigh_beep();
    high_beep();
    low_beep();
    high_beep();
    vhigh_beep();
}

#if PLATFORM_DOS

// Short beep at 1720 Hz (0x6b8) for 150 ms.
// FUNCTION: C2 0x2763c
// FUNCTION: C2WIN 0x0044e9dd
void vhigh_beep(void)
{
    sound(0x6b8);
    delay(150);
    nosound();
}
#else
void vhigh_beep(void)
{
    Beep(0x6b8, 150);
}
#endif /* PLATFORM_DOS */

// Sets up Bresenham state for the line from (x1, y1) to (x2, y2). dx / dy hold the absolute
// extents along each axis; the longer of the two becomes the major axis (so the line loop in
// draw_a_line steps along it).
// FUNCTION: C2 0x2765a
// FUNCTION: C2WIN 0x0044eaa4
void get_longest_side(int x1, int y1, int x2, int y2)
{
    gx = gy = 1;

    if (x1 > x2) dx = x1 - x2;
    else         dx = x2 - x1;

    if (y1 > y2) dy = y1 - y2;
    else         dy = y2 - y1;

    if (dy > dx) {
        if (y1 > y2) {
            iy = y2; ey = y1;
            ix = x2; ex = x1;
        } else {
            iy = y1; ey = y2;
            ix = x1; ex = x2;
        }
    } else {
        if (x1 > x2) {
            iy = y2; ey = y1;
            ix = x2; ex = x1;
        } else {
            iy = y1; ey = y2;
            ix = x1; ex = x2;
        }
    }

    if (ex < ix) gx = -1;
    if (ey < iy) gy = -1;
}

// Per-step Bresenham accumulator update. draw_a_line passes mode = 0 in the x-major branch and
// mode = 1 in the y-major branch.
// FUNCTION: C2 0x27714
// FUNCTION: C2WIN 0x0044ebff
void Bresenham_decision(int mode)
{
    if (mode == 0) {
        if (D < 0) D = D + 2 * dy;
        else       D = D + 2 * (dy - dx);
    } else {
        if (D < 0) D = D + 2 * dx;
        else       D = D + 2 * (dx - dy);
    }
}

// Plot one pixel when `(x, y)` lies inside the internal screen buffer.
// FUNCTION: C2 0x2779b
// FUNCTION: C2WIN 0x0044ec7d
void draw_a_point(int x, int y, int colour)
{
    if (x < 0) return;
    if (x >= screen_width) return;
    if (y < 0) return;
    if (y >= screen_height) return;
    show_internal_point(x, y, colour);
}

// Bounds-checked two-pixel point plot. If (x, y) is inside the screen rectangle, dispatches to
// show_internal_2point which writes the colour byte at (x, y) AND (x + 1, y).
// FUNCTION: C2 0x277d2
// FUNCTION: C2WIN 0x0044ece2
void draw_a_2point(int x, int y, int colour)
{
    if (x < 0) return;
    if (x >= screen_width) return;
    if (y < 0) return;
    if (y >= screen_height) return;
    show_internal_2point(x, y, colour);
}

// Draw a clipped horizontal, vertical, or diagonal Bresenham line.
// FUNCTION: C2 0x27809
// FUNCTION: C2WIN 0x0044ed47
void draw_a_line(int x1, int y1, int x2, int y2, int colour)
{
    int x;
    int y;

    get_longest_side(x1, y1, x2, y2);
    y = iy;
    x = ix;

    if (x1 == x2) {
        while (y <= ey) {
            draw_a_point(x1, y, colour);
            y++;
        }
    } else if (y1 == y2) {
        while (x <= ex) {
            draw_a_point(x, y1, colour);
            x++;
        }
    } else if (dy > dx) {
        D = 2 * dx - dy;
        x = ix;
        for (y = iy; y <= ey; y++) {
            draw_a_point(x, y, colour);
            Bresenham_decision(1);
            dy--;
            if (D >= 1) {
                dx--;
                x += gx;
            }
        }
    } else {
        D = 2 * dy - dx;
        x = ix;
        y = iy;
        for ( ; x <= ex; x++) {
            draw_a_point(x, y, colour);
            Bresenham_decision(0);
            dx--;
            if (D >= 1) {
                dy--;
                y += gy;
            }
        }
    }
}

// Vertical / horizontal dotted line: plots one dot every two pixels along the major axis. Diagonal
// lines are silently dropped.
// FUNCTION: C2 0x27923
// FUNCTION: C2WIN 0x0044ef01
void draw_a_dotted_line(int x1, int y1, int x2, int y2, int colour)
{
    int x;
    int y;

    get_longest_side(x1, y1, x2, y2);
    y = iy;
    x = ix;

    if (x1 == x2) {
        while (y <= ey) {
            draw_a_point(x1, y, colour);
            y += 2;
        }
    } else if (y1 == y2) {
        while (x <= ex) {
            draw_a_point(x, y1, colour);
            x += 2;
        }
    }
}

// Outline a rectangle with four draw_a_line calls -- top edge, bottom edge, left edge, right edge
// -- in that order. The inclusive lower-right corner is (x + w - 1, y + h - 1).
// FUNCTION: C2 0x27996
// FUNCTION: C2WIN 0x0044efab
void draw_a_box(int x, int y, int w, int h, int colour)
{
    draw_a_line(x,             y,             x + (w - 1), y,             colour);
    draw_a_line(x,             y + (h - 1),   x + (w - 1), y + (h - 1),   colour);
    draw_a_line(x,             y,             x,           y + (h - 1),   colour);
    draw_a_line(x + (w - 1),   y,             x + (w - 1), y + (h - 1),   colour);
}

// Outline a beveled rectangle ("dias" = dais / pedestal): light edges on top + right (colour
// 0x1f), dark edges on bottom + left (colour 0x12). Inclusive lower-right corner is (x + w - 1, y
// + h - 1).
// FUNCTION: C2 0x27a0a
// FUNCTION: C2WIN 0x0044f046
void draw_a_dias(int x, int y, int w, int h)
{
    draw_a_line(x,           y,           x + (w - 1), y,           0x1f);
    draw_a_line(x + (w - 1), y,           x + (w - 1), y + (h - 1), 0x1f);
    draw_a_line(x,           y + (h - 1), x + (w - 1), y + (h - 1), 0x12);
    draw_a_line(x,           y,           x,           y + (h - 1), 0x12);
}

// Outline a tile-shaped diamond using draw_a_2point (which plots two mirror-image pixels per
// call). Walks top-half rows widening, then bottom-half rows narrowing, with the centre column at
// `xcentre`.
// FUNCTION: C2 0x27a74
// FUNCTION: C2WIN 0x0044f0d9
void draw_a_diamond(int x, int y, int width, int height, int colour)
{
    int i;
    int j;
    int k;

    width += 2;
    i = 0; j = height / 2 - 1; k = height / 2;
    for ( ; i < width / 2; i += 2, j--, k++) {
        draw_a_2point(x + i, y + j, colour);
        draw_a_2point(x + i, y + k, colour);
    }

    i = width / 2; j = height - 2; k = 1;
    for ( ; i < width - 2; i += 2, j--, k++) {
        draw_a_2point(x + i, y + j, colour);
        draw_a_2point(x + i, y + k, colour);
    }
}

void xor_a_diamond_lhs_top(int x, int y, int width, int height, int color);

// XOR-plot the top half of a tile diamond (both quadrants). Like draw_a_diamond but routes through
// xor_internal_2point and stops at the equator (height/2).
// FUNCTION: C2 0x27b35
// FUNCTION: C2WIN 0x0044f1d8
void xor_a_diamond_top(int x, int y, int width, int height, int colour)
{
    int i;
    int j;
    int k;

    width += 2;
    i = 0; j = height / 2 - 1; k = height / 2;
    for ( ; i < width / 2; i += 2, j--, k++)
        xor_internal_2point(x + i, y + j, colour);

    i = width / 2; j = height - 2; k = 1;
    for ( ; i < width - 2; i += 2, j--, k++)
        xor_internal_2point(x + i, y + k, colour);
}

// Draws the left-hand-side top quadrant of a diamond outline by stepping through (height/2) rows,
// advancing x by 2 per iteration and dropping y by 1. Each step calls xor_internal_2point which
// XOR-plots a mirrored point pair about a vertical axis.
// FUNCTION: C2 0x27bc3
// FUNCTION: C2WIN 0x0044f2a3
void xor_a_diamond_lhs_top(int x, int y, int width, int height, int color)
{
    int i;
    int j;
    int k;

    width += 2;
    i = 0; j = height / 2 - 1; k = height / 2;

    for ( ; i < width / 2; i += 2, j--, k++) {
        xor_internal_2point(x + i, y + j, color); }
}

void draw_a_rect(int x, int y, int w, int h, int colour);
void write_image(unsigned char *sprite_data, int image_idx, int x, int y);
void yclip(int clip_top, int clip_bottom);
void do_32_count(void);

// Draw the top-right half of an XOR diamond outline.
// FUNCTION: C2 0x27c1b
// FUNCTION: C2WIN 0x0044f315
void xor_a_diamond_rhs_top(int x, int y, int width, int height, int color)
{
    int i;
    int j;
    int k;

    width += 2;
    i = width / 2; j = height - 2; k = 1;
    for ( ; i < width - 2; i += 2, j--, k++) {
        xor_internal_2point(x + i, y + k, color); }
}

void xclip(int clip_left, int clip_right);

// Filled (solid) rectangle by stacking horizontal draw_a_line calls from y to y + h - 1. Returns
// early if w or h is non-positive.
// FUNCTION: C2 0x27c66
// FUNCTION: C2WIN 0x0044f380
void draw_a_rect(int x, int y, int w, int h, int colour)
{
    int cur_y;

    if (w <= 0 || h <= 0) return;

    for (cur_y = y; cur_y < y + h; cur_y++) {
        draw_a_line(x, cur_y, x + (w - 1), cur_y, colour); }
}

// Sprite-blit dispatcher. `buf` is a packed sprite-bank: an 8-byte header followed by 16-byte
// descriptors (width at +0, height at +2, pixel-data byte-offset at +4..+6 all LE).
// FUNCTION: C2 0x27cb3
// FUNCTION: C2WIN 0x0044f3e9
void write_image(unsigned char *sprite_data, int image_idx, int x, int y)
{
    data_ptr = image_idx * 16 + 8;

    sprite_width  = sprite_data[data_ptr]     + (sprite_data[data_ptr + 1] << 8);
    sprite_height = sprite_data[data_ptr + 2] + (sprite_data[data_ptr + 3] << 8);
    sprite_start  = sprite_data[data_ptr + 4] + sprite_data[data_ptr + 5] * 0x100
                  + sprite_data[data_ptr + 6] * 0x10000;
    sprite_x = x;
    sprite_y = y;

    xclip(0, screen_width);
    yclip(0, screen_height);

    if (yclipped == 5) return;
    else if (xclipped == 1) { write_i_left_sprite(sprite_data); }
    else if (xclipped == 2) { write_i_right_sprite(sprite_data); }
    else {
        write_i_sprite(sprite_data);
    }
}

// Plot one sprite from the sprite-bank `buf` at (x, y) with X/Y clipping against a rectangular
// window.
// FUNCTION: C2 0x27d7f
// FUNCTION: C2WIN 0x0044f521
void write_clipped_image(unsigned char *sprite_data, int image_idx, int x, int y,
                         int clip_x_lo, int clip_x_hi,
                         int clip_y_lo, int clip_y_hi)
{
    data_ptr = image_idx * 16 + 8;
    sprite_width  = *(sprite_data + data_ptr) + (*(sprite_data + data_ptr + 1) << 8);
    sprite_height = *(sprite_data + data_ptr + 2) + (*(sprite_data + data_ptr + 3) << 8);
    sprite_start  = *(sprite_data + data_ptr + 4) + (*(sprite_data + data_ptr + 5) << 8)
                  + (*(sprite_data + data_ptr + 6) << 16);
    sprite_x = x;
    sprite_y = y;
    xclip(clip_x_lo, clip_y_lo);
    yclip(clip_x_hi, clip_y_hi);
    if (yclipped == 5) return;
    else if (xclipped == 1) { write_i_left_sprite(sprite_data); }
    else if (xclipped == 2) { write_i_right_sprite(sprite_data); }
    else { write_i_sprite(sprite_data); }
}

// Clip a sprite's horizontal extent against the [clip_left, clip_right] window. Reads sprite_x /
// sprite_width, writes the global clip-state quintet (xclipped, x_start, x_end, x_length, x_ofset,
// x_wrap, plus an in-place adjustment of sprite_start and sprite_x on a left-clip).
// FUNCTION: C2 0x27e54
// FUNCTION: C2WIN 0x0044f659
void xclip(int clip_left, int clip_right)
{
#if PLATFORM_WINDOWS
    if (clip_left < 0) clip_left = 0;
    if (screen_width <= clip_right) clip_right = screen_width;
#endif

    xclipped = 0;
    x_start = 0;
    x_end = sprite_width;

    if (sprite_width <= 0) {
        xclipped = 5;
    } else if (clip_left > sprite_x) {
        if (sprite_x + sprite_width <= clip_left) {
            xclipped = 5;
        } else {
            xclipped = 1;
            x_start = clip_left - sprite_x;
            sprite_start += x_start;
            sprite_x = clip_left;
        }
    } else if (clip_right - sprite_width < sprite_x) {
        if (clip_right <= sprite_x) {
            xclipped = 5;
        } else {
            xclipped = 2;
            x_end = clip_right - sprite_x;
        }
    }

    if (xclipped == 5) {
        x_length = 0;
    } else {
        x_length = x_end - x_start;
    }
    x_ofset = sprite_width - x_length;
    x_wrap = screen_width - x_length;
}

// Vertical sibling of xclip(). Clips a sprite's Y-extent against the [clip_top, clip_bottom]
// window, leaving the result in the global y-clip state (yclipped, y_start, y_end, y_length, plus
// an in-place adjustment of sprite_start and sprite_y on a top-clip).
// FUNCTION: C2 0x27f20
// FUNCTION: C2WIN 0x0044f7ad
void yclip(int clip_top, int clip_bottom)
{
#if PLATFORM_WINDOWS
    if (clip_top < 0) clip_top = 0;
#endif

    yclipped = 0;
    y_start = 0;
    y_end = sprite_height;

    if (sprite_height <= 0) {
        yclipped = 5;
    } else if (clip_top > sprite_y) {
        if (sprite_y + sprite_height <= clip_top) {
            yclipped = 5;
        } else {
            yclipped = 3;
            y_start = clip_top - sprite_y;
            sprite_start += y_start * sprite_width;
            sprite_y = clip_top;
        }
    } else if (clip_bottom - sprite_height < sprite_y) {
        if (clip_bottom <= sprite_y) {
            yclipped = 5;
        } else {
            yclipped = 4;
            y_end = clip_bottom - sprite_y;
        }
    }

    if (yclipped == 5) {
        y_length = 0;
    } else {
        y_length = y_end - y_start;
    }

    if (xclipped == 5) {
        yclipped = 5;
    }
}

// Tick the seven free-running frame counters used to drive periodic UI animations.
// FUNCTION: C2 0x27fea
// FUNCTION: C2WIN 0x0044f8e9
void do_32_count(void)
{
    ++cnt4;   if (cnt4   >= 4)   cnt4   = 0;
    ++cnt8;   if (cnt8   >= 8)   cnt8   = 0;
    ++cnt16;  if (cnt16  >= 16)  cnt16  = 0;
    ++cnt32;  if (cnt32  >= 32)  cnt32  = 0;
    ++cnt64;  if (cnt64  >= 64)  cnt64  = 0;
    ++cnt128; if (cnt128 >= 128) cnt128 = 0;
    ++cnt256; if (cnt256 >= 256) cnt256 = 0;
}

// 31-bit linear-feedback shift register (taps 0 and 4) folding into bit 30 of randseed; returns
// the low 15 bits.
// FUNCTION: C2 0x280bc
// FUNCTION: C2WIN 0x0044f9c5
int big_random(void)
{
    int i;
    unsigned int bit;
    for (i = 0; i < 0x1f; i++) {
        bit = (randseed & 1) ^ ((randseed & 0x10) >> 4);
        randseed >>= 1;
        if (bit != 0)
            randseed |= 0x40000000;
    }
    return randseed & 0x7fff;
}

// Refresh the per-frame random caches (rand32000, rand128, rand8) from a single big_random() draw.
// FUNCTION: C2 0x28105
// FUNCTION: C2WIN 0x0044fa34
void random(void)
{
    rand32000 = big_random();
    rand128   = rand32000 & 0x7f;
    rand8     = rand32000 & 7;
}

// Same LFSR as big_random but mixes scatseed and stores the low 7 bits to scat128 — used for
// sprite scatter offsets.
// FUNCTION: C2 0x2812f
// FUNCTION: C2WIN 0x0044fa63
void scatter(void)
{
    int i;
    unsigned int bit;
    for (i = 0; i < 0x1f; i++) {
        bit = (scatseed & 1) ^ ((scatseed & 0x10) >> 4);
        scatseed >>= 1;
        if (bit != 0)
            scatseed |= 0x40000000;
    }
    scat128 = scatseed & 0x7f;
}

int totalXpercentX100(int total, int percent_x100);
int valueDIVtotal(int value, int total);

// Uniformly-distributed bounded random in [0, max]. Pulls a sample from the cached rand32000 word,
// masks it down to the smallest 2^k - 1 mask >= max, and rejects/retries (calling random() to
// reseed rand32000) if the masked sample exceeds max.
// FUNCTION: C2 0x2817b
// FUNCTION: C2WIN 0x0044fad0
int get_rand_max(int max)
{
    int i;
    unsigned int mask;
    int v;

    if (max <= 0) return 0;

    if      (max <=     1) mask =     1;
    else if (max <=     3) mask =     3;
    else if (max <=     7) mask =     7;
    else if (max <=   0xf) mask =   0xf;
    else if (max <=  0x1f) mask =  0x1f;
    else if (max <=  0x3f) mask =  0x3f;
    else if (max <=  0x7f) mask =  0x7f;
    else if (max <=  0xff) mask =  0xff;
    else if (max <= 0x1ff) mask = 0x1ff;
    else if (max <= 0x3ff) mask = 0x3ff;
    else if (max <= 0x7ff) mask = 0x7ff;
    else if (max <= 0xfff) mask = 0xfff;
    else if (max <= 0x1fff) mask = 0x1fff;
    else if (max <= 0x3fff) mask = 0x3fff;
    else if (max <= 0x7fff) mask = 0x7fff;
    else                   mask = 0xffff;

    i = 0;
    while (i++ < 10) {
        v = rand32000 & mask;
        if (v <= max) return v;
        random();
    }
    return 0;
}

// Returns (a * b) / 100.
// FUNCTION: C2 0x28298
// FUNCTION: C2WIN 0x0044fca2
int totalXpercent(int total, int percent)
{
    total = percent * total;
    total = total / 100;
    return total;
}

// Returns (a * b) / 10000.
// FUNCTION: C2 0x282ba
// FUNCTION: C2WIN 0x0044fccd
int totalXpercentX100(int total, int percent_x100)
{
    total = percent_x100 * total;
    total = total / 10000;
    return total;
}

// Percentage helper: (value * 100) / total, with a guard for total==0.
// FUNCTION: C2 0x282d2
// FUNCTION: C2WIN 0x0044fcf8
int valueDIVtotal(int value, int total)
{
    value *= 100;
    if (total != 0)
        value = value / total;
    else
        value = 0;
    return value;
}

// Manhattan distance between (x1,y1) and (x2,y2).
// FUNCTION: C2 0x28300
// FUNCTION: C2WIN 0x0044fd3a
int get_distance(int x1, int y1, int x2, int y2)
{
    int x_diff;
    int y_diff;
    if (x1 > x2)      x_diff = x1 - x2;
    else if (x1 < x2) x_diff = x2 - x1;
    else              x_diff = 0;
    if (y1 > y2)      y_diff = y1 - y2;
    else if (y1 < y2) y_diff = y2 - y1;
    else              y_diff = 0;
    return x_diff + y_diff;
}

int get_longest_distance(int x1, int y1, int x2, int y2);
int get_shortest_distance(int x1, int y1, int x2, int y2);
int lock_region(unsigned int address, unsigned int size);
void start_game(void);
void exit_game(void);
void stop_system(void);
void free_scratch_buffer(void);

// Chebyshev (king-move) distance: max(|dx|, |dy|).
// FUNCTION: C2 0x28333
// FUNCTION: C2WIN 0x0044fdc9
int get_longest_distance(int x1, int y1, int x2, int y2)
{
    int x_diff;
    int y_diff;
    if (x1 > x2)      x_diff = x1 - x2;
    else if (x1 < x2) x_diff = x2 - x1;
    else              x_diff = 0;
    if (y1 > y2)      y_diff = y1 - y2;
    else if (y1 < y2) y_diff = y2 - y1;
    else              y_diff = 0;
    if (x_diff < y_diff) return y_diff;
    return x_diff;
}

// min(|dx|, |dy|) — the shortest leg of the bounding rectangle.
// FUNCTION: C2 0x28368
// FUNCTION: C2WIN 0x0044fe69
int get_shortest_distance(int x1, int y1, int x2, int y2)
{
    int x_diff;
    int y_diff;
    if (x1 > x2)      x_diff = x1 - x2;
    else if (x1 < x2) x_diff = x2 - x1;
    else              x_diff = 0;
    if (y1 > y2)      y_diff = y1 - y2;
    else if (y1 < y2) y_diff = y2 - y1;
    else              y_diff = 0;
    if (x_diff < y_diff) return x_diff;
    return y_diff;
}

#if PLATFORM_DOS

// Issue DPMI service 0x600 (lock linear region) for the byte range [addr, addr+size). Returns
// nonzero on success (carry clear).
// FUNCTION: C2 0x2839e
// FUNCTION: C2WIN 0x0044ff09
int lock_region(unsigned int address, unsigned int size)
{
    union REGS r;
    unsigned int hi;
    r.w.ax = 0x600;
    hi = address >> 16;
    r.w.bx = hi;
    r.w.cx = address;
    hi = size >> 16;
    r.w.si = hi;
    r.w.di = size;
    int386(0x31, &r, &r);
    return r.w.cflag == 0;
}
#endif /* PLATFORM_DOS */

// Start the game runtime from the program entry path.
// FUNCTION: C2 0x283f0
// FUNCTION: C2WIN 0x0044ff14
void start_game(void)
{
    start_system();
}

#if PLATFORM_WINDOWS
void close_windows(void);
#endif

// Tear-down counterpart of start_system.
// FUNCTION: C2 0x28470
// FUNCTION: C2WIN 0x0044ff24
void exit_game(void)
{
    stop_system();
#if PLATFORM_WINDOWS
    close_windows();
#else
    printf("\nExiting Caesar II.\n");
    exit(0);
#endif
}

// Initialize random state, video, framebuffers, audio, scratch memory, and mouse limits.
// FUNCTION: C2 0x283fa REORDERED
// FUNCTION: C2WIN 0x0044ff39
#if PLATFORM_WINDOWS
extern unsigned char *game_screen;
#endif

int start_system(void)
{
    int i;

    randseed = 0x54657687;
    scatseed = 0x34518632;
    test_mode1 = test_mode2 = test_mode3 = test_mode4 = 0;
    used_memory = 0;
    for (i = 0; i < 0x300; i++)
        black_out_data[i] = 0;
    key_ascii = 0;
    if (screen_mode == 1) {
        clear_all_screens();
        set_vga_256x();
    }
#if PLATFORM_WINDOWS
    else if (screen_mode == 2) vid_error = set_svga_640_480();
#else
    else if (screen_mode == 2) vid_error = set_svga_640_480(0);
    else if (screen_mode == 3) vid_error = set_svga_640_480(1);
#endif

    screen_refresh_flag = 0;
#if PLATFORM_WINDOWS
    if (screen_mode == 1) { screen_width = 0x140; screen_height = 0xc8; }
    else                  { screen_width = 0x280; screen_height = 0x1e0; }
#else
    if (screen_mode == 1)      { screen_width = 0x140; screen_height = 0xc8; }
    else if (screen_mode == 2) { screen_width = 0x280; screen_height = 0x1e0; }
    else                       { screen_width = 0x280; screen_height = 0x190; }
#endif
    screen_size = screen_width * screen_height;

    internal_screen = 0;
#if PLATFORM_WINDOWS
    internal_screen = game_screen;
#else
    internal_screen = malloc(screen_size);
#endif
    if (internal_screen != 0) used_memory += (unsigned int)screen_size / 0x400;
    setup_scratch_buffer();
    start_sounds();
    clear_screens();
    set_mouse_limits();
    pointer_mode = 0;
    dos_memory = 0;
    if (exit_flag != 0) return 0;
    else return 1;
}

#if PLATFORM_DOS

// Query the DPMI host and runtime heap for current memory availability.
// FUNCTION: C2 0x28579
// FUNCTION: C2WIN 0x004500c8
void get_dos_memory(void)
{
    union REGS r;
    struct SREGS sr;

    r.x.eax = 0x500;
    memset(&sr, 0, sizeof sr);
    sr.es = FP_SEG((void __far *)&memory);
    r.x.edi = (int)&memory;
    int386x(0x31, &r, &r, &sr);
    dos_memory = memory.free_linear_pages * 4;
    avl_memory = _memavl();
    max_memory = _memmax();
}
#endif /* PLATFORM_DOS */

// Allocate scratch_buffer with the size in scratch_buffer_size and charge the (KB-rounded) cost to
// used_memory.
// FUNCTION: C2 0x285ee
// FUNCTION: C2WIN 0x004500d3
void setup_scratch_buffer(void)
{
    scratch_buffer = 0;
    if (scratch_buffer_size != 0)
        scratch_buffer = malloc(scratch_buffer_size);
    if (scratch_buffer != 0)
        used_memory += scratch_buffer_size / 1024;
}

// Release the scratch buffer if any, and decrement used_memory by the buffer's KB size.
// FUNCTION: C2 0x2863c
// FUNCTION: C2WIN 0x0045012c
void free_scratch_buffer(void)
{
    if (scratch_buffer != 0) {
        free((void *)scratch_buffer);
#if PLATFORM_WINDOWS
        scratch_buffer = 0;
#endif
        used_memory -= scratch_buffer_size / 1024;
    }
}

// Probe the largest allocatable heap block and store its size in KiB.
// FUNCTION: C2 0x28672
void get_free_memory(void)
{
    void *block_ptr;

    block_ptr = 0;
    allocable_memory = 0x400;
    block_ptr = malloc(allocable_memory);
    while (block_ptr != NULL) {
        free(block_ptr);
        block_ptr = 0;
        allocable_memory += 0x400;
        block_ptr = malloc(allocable_memory);
    }
    allocable_memory -= 0x400;
    allocable_memory = allocable_memory / 0x400;
}

// Release graphics memory and scratch storage, restore display state, and stop audio.
// FUNCTION: C2 0x286da
// FUNCTION: C2WIN 0x00450206
void stop_system(void)
{
#if !PLATFORM_WINDOWS
    if (internal_screen != 0) free(internal_screen);
#endif
    free_scratch_buffer();
    if (screen_mode == 1) unset_vga_256x();
    dos_cls();
    set_mode3();
    stop_sounds();
}
