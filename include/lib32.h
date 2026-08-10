#ifndef C2_LIB32_H
#define C2_LIB32_H

/* DPMI 0x300 real-mode interrupt-call register block. */
struct dpmi_real_mode_regs {
    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int _reserved0;
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;
    unsigned short flags;
    unsigned short es;
    unsigned short ds;
    unsigned short fs;
    unsigned short gs;
    unsigned short ip;
    unsigned short cs;
    unsigned short sp;
    unsigned short ss;
};

/*
 * lib32 declarations in recovered Watcom symbol-handle page order.  The
 * named lib32 API prototypes occupy the header slots that have no
 * BSS witness; no synthetic padding names are used.
 */

/* Symbol page 0 (PS emission page 11). */
extern void get_directory(char *pattern);
extern void cd_path(const char *fname);
extern void main_path(void);
extern void get_filename_extension(const char *fname);
extern void put_filename_extension(char *fname);

/* Symbol page 1 (PS emission page 10). */
extern int used_memory;
extern int free_memory;
extern int dos_memory;
extern int avl_memory;
extern int max_memory;
extern int scratch_buffer_size;
extern unsigned char * scratch_buffer;
extern int allocable_memory;
extern unsigned char key_ready;
extern unsigned char key_ascii;
extern char get_filename_length(char *s);
extern int check_file_exists(char *fname);
extern int is_file_on_harddrive(char *fname);
extern int readfile(const char *fname, void *buf, int size, int offset);
extern int writefile(const char *fname, char *buf, int size);
extern int write_to_file(char *fname, char *buf, int size, int offset);
extern char read_config(char *fname, char *buf);
extern int convert_lbm_file(unsigned char *src, unsigned char *dst, char *pal, int length);
#if PLATFORM_WINDOWS
extern int set_svga_640_480(void);
#else
extern int set_svga_640_480(int mode);
#endif
extern void recognise_card(void);
extern int check_for_Trident(void);
extern int check_for_Tseng(void);
extern void get_video_technique(void);
extern void print_vesa_info(void);
extern void set_vga_256x(void);

/* Symbol page 2 (PS emission page 9). */
extern int button_time_flag;
extern int local_time;
extern int time_is;
extern int mouse_y;
extern int mouse_x;
extern int old_mouse_x;
extern int old_mouse_y;
extern int mouse_installed;
extern int mouse_movement;
extern int mouse_was_pressed;
extern unsigned char old_mouse_rb;
extern unsigned char old_mouse_lb;
extern unsigned char mouse_left_button;
extern unsigned char mouse_left_click;
extern unsigned char debar_fade_click;
extern unsigned char mouse_right_preclick;
extern unsigned char mouse_left_preclick;
extern unsigned char mouse_right_button;
extern unsigned char mouse_right_click;
extern unsigned char key_code;
extern char debug_interupt;
extern unsigned char key_ascii_was;
extern unsigned char mse_button;
extern void unset_vga_256x(void);
extern void set_mode3(void);

/* Symbol page 3 (PS emission page 8). */
extern int first_entry;
extern int cscreen;
extern char *lang_file;
extern char directory[][13];
extern int gamesource;
extern int granularity;
extern int no_of_entries;
extern int oscreen;
extern int screen_size;
extern unsigned char * internal_screen;
extern char extension[4];
extern int file_no;
extern int screen_width;
extern char *media_file;
extern int drive_name;
extern int file_string;
extern char screen_refresh_flag;
extern unsigned char hold_mouse_replace;
extern unsigned char screen_mode;
extern unsigned char pointer_mode;
extern void set_vga_mode(int mode);
extern void page1_read(void);
extern int screen_height;
extern void set_vga_palette(char *p);
extern void set_vga_palette_range(char *p, int start, int end);

/* Symbol page 4 (PS emission page 7). */
extern int fb_max_width_reached;
extern int at_limit;
extern int fb_line_length;
extern int got_cursx;
extern int cursor_x;
extern int cursor_y;
extern char * text_pointer;
extern char insert_text[];
extern int char_count;
extern int this_letter;
extern int fb_current_char_length;
extern int fb_max_char_length;
extern void cycle_colours(int start_idx, int end_idx);
extern void pulse_red(int idx, int delta);
extern void swap_background_to_red(void);
extern void swap_background_to(int idx);
extern int insert_count;
extern int insert_place;
extern int x_is;
extern int fb_no_of_lines;
extern int fb_limit;
extern int fb_count;
extern struct dpmi_mem_info memory;
extern unsigned char allow_padding;
extern unsigned char hot_key_out_off_build;
extern unsigned char hold_hot_keys;
extern unsigned char padding_off;

/* Symbol page 5 (PS emission page 6). */
extern int sprite_height;
extern int old_sprite_y;
extern int old_sprite_x;
extern int sprite2_start;
extern int sprite3_start;
extern int sprite_colour;
extern int sprite_error;
extern int sprite_width;
extern int sprite_image_no;
extern int sprite3_image_no;
extern int sprite2_height;
extern int sprite2_image_no;
extern int sprite_start;
extern int sprite2_width;
extern int data_ptr;
extern int sprite3_width;
extern int sprite_base_y;
extern int sprite_base_x;
extern int sprite_x_off;
extern int sprite_hat_start;
extern int sprite3_height;
extern int sprite_size;
extern int sprite_y_off;
extern int sprite_x;
extern int sprite_y;

/* Symbol page 6 (PS emission page 5). */
extern int font_style;
extern int absolute_ofset;
extern int x_wrap;
extern int font_screen_limit;
extern int dy;
extern int sprite_bank;
extern int x_start;
extern int ex;
extern int dx;
extern int D;
extern int y_start;
extern int sprite_next_bank_count;
extern int ey;
extern int gx;
extern int gy;
extern int y_length;
extern int ix;
extern int x_length;
extern int sprite_bank_ofset;
extern int x_end;
extern int y_end;
extern int x_ofset;
extern unsigned char insert_cursor;
extern char highlight;

/* Symbol page 7 (PS emission page 4). */
extern int adjust_step;
extern int test_value4d;
extern int test_value4a;
extern int test_value4b;
extern int test_value4c;
extern int iy;
extern int test_value2d;
extern int adjust_min;
extern int test_value2a;
extern int test_value2b;
extern int test_value2c;
extern int test_value3d;
extern int * adjust_var;
extern int test_value3a;
extern int test_value3b;
extern int test_value3c;
extern int test_value1d;
extern int test_value1a;
extern int test_value1b;
extern int test_value1c;
extern int adjust_max;
extern char text_buffer[];
extern char path_name[];
extern unsigned char xclipped;
extern unsigned char yclipped;
extern void copy_palette(char *src, char *dst);

/* Symbol page 8 (PS emission page 3). */
extern int cnt32;
extern int out6;
extern int out7;
extern int out4;
extern int out5;
extern int out2;
extern int out3;
extern unsigned int randseed;
extern int out1;
extern int turbo_mode;
extern int out8;
extern int cnt16;
extern int cnt64;
extern unsigned int scatseed;
extern int cycle_count;
extern int cnt128;
extern int cnt8;
extern int cnt2;
extern int cnt4;
extern int cnt256;
extern unsigned char test_mode3;
extern unsigned char test_mode2;
extern unsigned char test_mode1;
extern unsigned char test_mode4;
extern unsigned char develop_mode;

/* Symbol page 9 (PS emission page 2). */
extern short highlight_goods_list[];
extern int slider_total;
extern short selection_goods_list[];
extern int select_cost_flag;
extern int scat32000;
extern int para1;
extern int para2;
extern int select_width;
extern int select_height;
extern int rand128;
extern int select_count;
extern int slidper_on;
extern int scat128;
extern unsigned char font1[];
extern int rand32000;
extern int rand8;
extern unsigned char format_buffer[];
extern unsigned char hot_exit_flag;
extern unsigned char decision;
extern unsigned char confirming;
extern unsigned char exit_flag;
extern unsigned char restart_flag;
extern void go_64k_palette(unsigned char *p);
extern void go_16m_palette(unsigned char *p);
extern void load_to_temp_palette(char *fname);

/* Symbol page 10 (PS emission page 1). */
extern int vid_bank_tech;
extern int vid_tech;
extern int vid_error;
extern int fb_current_char_length;
extern int fb_max_char_length;
extern int vid_memory;
extern int old_mouse_drops_y;
extern int old_mouse_drops_x;
extern int vid_no_of_banks;
extern int bank_ptr;
extern char temp_palette[];
extern unsigned char logos[];
extern unsigned char font2[];
extern unsigned char system_panel[];
extern unsigned char game_panels[];
extern char filename[];
extern unsigned char pre_loaded_status;
extern unsigned char continue_tutorial_status;
extern unsigned char file_loaded_status;
extern char map_gfx_loaded;
extern struct mouse_cbd cbd;
extern int steves_security_false1[];
extern char *chipset_names[32];
extern int multiples[];
extern void fade_to_temp_palette(char *fname);

/* Symbol page 11 (PS emission page 0). */
extern int cards_recognised;
extern int card_is;
extern int card_sub_type;
extern struct vbe_mode_info vesa_mode_info;
extern char card_ids[];
extern unsigned char greying_data[256];
extern char mouse_background[];
extern struct vbe_info_block vesa_info;
extern char black_out_data[];
extern unsigned char current_palette[];
extern struct dpmi_real_block VesaModeInfo;
extern struct dpmi_real_block VesaInfo;
static struct dpmi_real_mode_regs RMI;
extern unsigned char mice[];
extern struct media_entry this_media_entry;
extern short mse_y;
extern short mse_x;
extern char vid_new_val;
extern char old3d5_14;
extern char old3d5_17;
extern char vid_old_val;
extern char old3cf_6;
extern char old3cf_5;
extern char vid_val;
extern char old3c5_4;

#endif /* C2_LIB32_H */
