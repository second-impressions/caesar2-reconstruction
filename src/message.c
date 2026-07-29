
// Queued modal messages, imperial requests, and their message panels.
#include "c2_data.h"
#include "c2_types.h"


extern void put_a_font_string(char *str, int x, int y, unsigned char *font, int color);
extern void font_list(int idx, int word_count, int x, int y, unsigned char *font, int color);
extern void font_no(int value, char pad_char, char *suffix, int x, int y, unsigned char *font, int color);
extern void font_format_split(int idx, int word_skip, int x, int y_start, int max_width, int line_limit, int x_overflow, int max_width_overflow, unsigned char *font, int color);

// Globals owned by this translation unit
struct request_message  request_message;
char smacks[40][14] = {
    "congrat.smk",  "sick.smk",     "fire.smk",     "warning.smk",
    "congrat.smk",  "warning.smk",  "robbery.smk",  "rioters.smk",
    "robbery.smk",  "robbery.smk",  "warning.smk",  "armywarn.smk",
    "armywarn.smk", "armywarn.smk", "armywarn.smk", "armywarn.smk",
    "armywarn.smk", "warning.smk",  "warning.smk",  "warning.smk",
    "warning.smk",  "warning.smk",  "warning.smk",  "warning.smk",
    "congrat.smk",  "congrat.smk",  "congrat.smk",  "congrat.smk",
    "congrat.smk",  "congrat.smk",  "congrat.smk",  "congrat.smk",
    "congrat.smk",  "armywarn.smk", "armywarn.smk", "congrat.smk",
    "congrat.smk",  "null.smk",     "null.smk",     "null.smk"
};

// ── External functions ────────────────────────────────────────────────────────
extern void region_map_screen(int flag);

/* Forward declarations (functions defined later in this file). */
void clear_messages(void);
void message(int message_idx, int is_emperor, int message_param);
void show_basic_message(int message_idx, int message_param);
void show_emperor_message(int message_idx, int is_emperor);
void show_request_amount(void);
void request_outcome(void);


// Resets message and imperial-request state and queues the opening message.
// FUNCTION: C2 0x5910f
// FUNCTION: C2WIN 0x00459c40
void init_messages(void) {
    clear_messages();
    warned_city_size              = 0;
    warned_new_struct             = 0;
    warned_of_emperor_reply_month = 0;
    warned_of_emperor_reply_level = 0;
    warned_of_emperor             = 0;
    final_bribe                   = 0;
    imperial_request  = skill_to_imperial_request[c2inf.skill_level];
    imperial_review   = 3;
    imperial_req_amount = 0;
    imperial_req_goods  = 0;
    if (develop_mode == 0) {
        message_list[free_message_ptr].msg   = 0x50;
        message_list[free_message_ptr].param = 0;
        free_message_ptr++;
    }
}

// Empties the message queue and resets its warning and cursor state.
// FUNCTION: C2 0x5919f
// FUNCTION: C2WIN 0x00459ce9
void clear_messages(void) {
    int i;
    warned_of_fire     = 0;
    warned_of_cutbacks = 0;
    warned_of_robbery  = 0;
    free_message_ptr   = 0;
    show_message_ptr   = 0;
    put_a_message      = 0;
    for (i = 0; i < 16; i++) {
        message_list[i].param = 0;
        message_list[i].msg   = 0;
    }
}

// Adds a message to the ring buffer and optionally changes the music mood.
// FUNCTION: C2 0x591e8
// FUNCTION: C2WIN 0x00459d67
void put_message(int message_idx, int message_param, int music_mood) {
    message_list[free_message_ptr].msg   = message_idx;
    message_list[free_message_ptr].param = message_param;
    free_message_ptr++;
    if (free_message_ptr >= 16)
        free_message_ptr = 0;
    put_a_message = 1;
    if (music_mood != 0)
        tune_mood = music_mood;
}

// Removes and displays the next queued message when no blocking pointer mode is active.
// FUNCTION: C2 0x59229
// FUNCTION: C2WIN 0x00459dc8
void show_messages(void) {
    int message_idx;
    int message_param;
    if (pointer_mode < 5) {
        put_a_message = 0;
        message_idx   = message_list[show_message_ptr].msg;
        message_param = message_list[show_message_ptr].param;
        message_list[show_message_ptr].msg   = 0;
        message_list[show_message_ptr].param = 0;
        show_message_ptr++;
        if (show_message_ptr >= 16)
            show_message_ptr = 0;
        if (message_idx != 0)
            message(message_idx, message_idx >= 0x78, message_param);
    }
}

// Runs a modal message panel, then restores the map and handles any requested map jump.
// FUNCTION: C2 0x59292
// FUNCTION: C2WIN 0x00459e8d
void message(int message_idx, int is_emperor, int message_param) {
    int jump_result;
    int old_pointer_mode;
    decision = 0;
    if (tutorial_mode == 0 &&
        ((message_idx != 0x56 && message_idx != 0x59) || stolen_denarii > 0)) {
        clear_map_gfx_buffers();
        clear_battle_gfx_buffers();
        clear_keys();
        warned_of_not_build  = 0;
        stop_db();
        hold_hot_keys        = 1;
        old_pointer_mode     = pointer_mode;
        pointer_mode         = 0;
        turbo_mode           = 0;
        local_time           = time_is;
        message_goto_ptr     = message_param;
        request_message.active = 0;
        imperial_send_amount = 0;
        if (is_emperor == 0)
            show_basic_message(message_idx, message_param);
        else
            show_emperor_message(message_idx, is_emperor);
        out1 = 0;
        while (out1 != 1) {
            hold_hot_keys = 1;
            gloop_start();
            continue_smacking(0x60, 0x50, 1);
            if (request_message.active != 0) {
                if (gen_refresh1 != 0) {
                    gen_refresh1 = 0;
                    show_request_amount();
                }
                show_buttons(0x110, 0x18e, request_buttons, 2);
            }
            if (message_param != 0)
                show_buttons(0x130, 0x170, goto_mess_buttons, 1);
            gloop_end();
            if (request_message.active != 0)
                control_buttons(0x110, 0x18e, request_buttons, 2);
            if (message_param != 0)
                control_buttons(0x130, 0x170, goto_mess_buttons, 1);
            if (mouse_right_click != 0) {
                out1 = 1;
                message_goto_ptr = 0;
            }
            if (exit_screen() != 0) {
                out1 = 1;
                message_goto_ptr = 0;
            }
            if (local_time + 12 < time_is) {
                out1 = 1;
                message_goto_ptr = 0;
            }
        }
        if (request_message.active != 0) {
            request_outcome();
        } else {
            if (message_idx >= 0x7d && message_idx <= 0x84 && final_bribe == 2)
                game_state = 1;
        }
        stop_smacking();
        draw_a_rect(0x60, 0x50, 0x140, 0x98, 16);
        setup_refresh_area(0x60, 0x50, 20, 10, 1);
        refresh_svga_screen();
        pointer_mode = old_pointer_mode;
        init_map_gfx_buffers();
        init_battle_gfx_buffers();
        load_map_graphics(map_mode, zoom_level);
        jump_result = 0;
        if (message_goto_ptr != 0) {
            turbo_mode = 0;
            if (mess_goto_map[message_idx - 80] == 1)
                jump_result = jump_to_regionmap_ptr(message_goto_ptr);
            else
                jump_result = jump_to_citymap_ptr(message_goto_ptr);
        }
        if (map_mode == 0)
            set_palette(&city_palette);
        else if (map_mode == 1)
            set_palette(&region_palette);
        else
            set_palette(&temp_palette);
        if (jump_result != 1) {
            if (map_mode == 0)
                city_map_screen(0);
            else if (map_mode == 1)
                region_map_screen(0);
            else
                battle_screen(0);
        }
        if (message_param != 0) {
            danger_flag_map_mode = (mess_goto_map[message_idx - 80] == 1);
            put_danger_flag(message_param);
        }
        if (turbo_mode != 0)
            act_init_turbo_mode();
        setup_map_screen_long_refresh(4);
        gen_refresh1 = 1;
        flush_sb_buffer();
    }
}

// Draws a standard message panel with its text, animation, exit control, and optional map link.
// FUNCTION: C2 0x595d1
// FUNCTION: C2WIN 0x0045a701
void show_basic_message(int message_idx, int message_param) {
    cover_mouse_droppings();
    grey_a_screen();
    setup_whole_screen_refresh();
    refresh_svga_screen();
    show_a_mosaic_frame(0x50, 0x40, 22, 23);
    show_a_mosaic_blank(0x60, 0x50, 20, 21);
    mosaic_frame_divider(0x50, 0xe8, 22, 23);
    if (message_idx == 0x56 || message_idx == 0x59) {
        x_is = 0;
        font_no(stolen_denarii, 0x20, "Dn", 0x70, 0xfc, font2, 16);
        font_list(message_idx, 0, x_is + 0x78, 0xfc, font2, 16);
    } else if (message_idx == 0x50) {
        x_is = 0;
        font_list(message_idx, 0, 0x70, 0xfc, font2, 16);
        put_a_font_string(&c2inf.player_name, x_is + 0x70, 0xfc, font2, 16);
    } else if (message_idx == 0x73) {
        int structure_name_idx = new_structure_is - 0x78;
        font_list(0x3c, structure_name_idx, 0x70, 0xfc, font2, 16);
    } else {
        font_list(message_idx, 0, 0x70, 0xfc, font2, 16);
    }
    font_list(0x4f, 0, 0x80, 0x190, font1, 16);
    font_format_split(message_idx, 1, 0x68, 0x11c, 0x138, 0x64, 0x68, 0x138, font1, 16);
    show_an_exit_button(0x180, 0x180);
    if (message_param != 0) {
        font_list(0x4f, 1, 0xd0, 0x178, font1, 16);
        show_buttons(0x130, 0x170, goto_mess_buttons, 1);
    }
    setup_whole_screen_refresh();
    start_smacking(smacks[message_idx - 0x50], 0x60, 0x50, 1);
    hold_mouse_replace = 1;
}

// Draws an emperor message, including request, favour, tribute, or tax details.
// FUNCTION: C2 0x597dc
// FUNCTION: C2WIN 0x0045a935
void show_emperor_message(int message_idx, int is_emperor) {
    int favour_text_idx;
    (void)is_emperor;
    cover_mouse_droppings();
    grey_a_screen();
    setup_whole_screen_refresh();
    refresh_svga_screen();
    show_a_mosaic_frame(0x50, 0x40, 22, 25);
    show_a_mosaic_blank(0x60, 0x50, 20, 23);
    mosaic_frame_divider(0x50, 0xe8, 22, 23);
    x_is = 0;
    font_list(message_idx, 0, 0x70, 0xfc, font2, 16);
    put_a_font_string(c2inf.player_name, x_is + 0x74, 0xfc, font2, 16);
    font_format_split(message_idx, 1, 0x68, 0x11c, 0x138, 0x64, 0x68, 0x138, font1, 16);
    show_an_exit_button(0x180, 0x1a0);
    if (message_idx >= 0x87 && message_idx <= 0x8a) {
        /* Goods request */
        request_message.active = 1;
        if (imperial_req_amount == 0)
            imperial_req_amount = 1;
        x_is = 0;
        font_no(imperial_req_amount, 0x20, " ", 0x88, 0x154, font2, 16);
        font_list(0x26, 0x0f, x_is + 0x88, 0x154, font2, 16);
        font_list(0x10, imperial_req_goods + 1, x_is + 0x8c, 0x154, font2, 16);
        x_is = 0;
        font_list(0x26, 0x10, 0x68, 0x178, font1, 16);
        font_no(industry[imperial_req_goods].supply,
                0x20, " ", x_is + 0x68, 0x178, font1, 16);
        font_list(0x26, 0x11, x_is + 0x68, 0x178, font1, 16);
        show_request_amount();
        show_buttons(0x110, 0x18e, request_buttons, 2);
        font_list(0x26, 0x12, 0x88, 0x1b0, font1, 16);
    } else if (message_idx <= 0x8f) {
        /* Tribute / imperial favour */
        x_is = 0;
        font_list(0x26, 1, 0x68, 0x188, font1, 0);
        if (imperial_favour <= 0)
            favour_text_idx = 3;
        else if (imperial_favour >= 200)
            favour_text_idx = 14;
        else
            favour_text_idx = imperial_favour / 20 + 4;
        font_list(0x26, favour_text_idx, x_is + 0x68, 0x188, font1, 16);
        x_is = 0;
        font_list(0x26, 2, 0x68, 0x198, font1, 16);
        font_no(tribute, 0x20, " Dn.", x_is + 0x68, 0x198, font1, 16);
        font_list(0x4f, 0, 0x80, 0x1b0, font1, 16);
    } else {
        /* Tax demand */
        x_is = 0;
        font_list(0x26, 1, 0x68, 0x188, font1, 0);
        if (imperial_favour <= 0)
            favour_text_idx = 3;
        else if (imperial_favour >= 200)
            favour_text_idx = 14;
        else
            favour_text_idx = imperial_favour / 20 + 4;
        font_list(0x26, favour_text_idx, x_is + 0x68, 0x188, font1, 16);
        x_is = 0;
        font_list(0x26, 0x28, 0x68, 0x198, font1, 16);
        font_no(last_imperial_tax_amount, 0x20, " Dn.", x_is + 0x68, 0x198, font1, 16);
        font_list(0x4f, 0, 0x80, 0x1b0, font1, 16);
        players_denarii -= last_imperial_tax_amount;
        if (players_denarii < 0)
            players_denarii = 0;
    }
    setup_whole_screen_refresh();
    start_smacking("message.smk", 0x60, 0x50, 1);
    hold_mouse_replace = 1;
}

// Refreshes the amount selected for an imperial goods shipment.
// FUNCTION: C2 0x59c55
// FUNCTION: C2WIN 0x0045adce
void show_request_amount(void) {
    stone_random_count = 13;
    show_a_mosaic_blank(0x60, 0x188, 18, 2);
    font_list(0x26, 0x13, 0x68, 0x194, font1, 0);
    font_no(imperial_send_amount, 0x20, " ", 0x160, 0x194, font1, 0);
#if !PLATFORM_WINDOWS
    setup_refresh_area(0x60, 0x188, 20, 2, 1);
#endif
}

// Ships the selected goods and updates the request balance, favour, and final-bribe state.
// FUNCTION: C2 0x59cdc
// FUNCTION: C2WIN 0x0045ae33
void request_outcome(void) {
    int delivery_percent;
    take_from_warehouses(imperial_send_amount, imperial_req_goods);
    delivery_percent = valueDIVtotal(imperial_send_amount, imperial_req_amount);
    imperial_req_amount -= imperial_send_amount;
    if (imperial_request < -1 || delivery_percent >= 75) {
        /* Adjust imperial favour based on delivery percentage */
        if (delivery_percent <= 0)
            imperial_favour -= 20;
        else if (delivery_percent < 25)
            imperial_favour -= 15;
        else if (delivery_percent < 50)
            imperial_favour -= 10;
        else if (delivery_percent < 75)
            imperial_favour -= 5;
        else if (delivery_percent < 100)
            imperial_favour += (rand128 & 3) - 1;
        else
            imperial_favour += 20;
        if (imperial_favour < 0)
            imperial_favour = 0;
        if (imperial_favour > 200)
            imperial_favour = 200;
        if (imperial_req_amount <= 0) {
            imperial_request = skill_to_imperial_request[c2inf.skill_level];
            imperial_req_amount = 0;
            return;
        }
        if (imperial_request <= -2) {
            if (imperial_favour > 10) {
                imperial_request = -1;
                final_bribe = 0;
                return;
            }
            if (imperial_request <= -3)
                act_final_bribe();
        }
    }
}

// Applies the selected imperial bribe and updates the emperor's opinion.
// FUNCTION: C2 0x59dfd
// FUNCTION: C2WIN 0x0045afba
void bribe_emperor(void) {
    int rating;

    if (imperial_gift_level < av_imperial_gift_level)
        rating = -1;
    else if (imperial_gift_level == av_imperial_gift_level)
        rating = 0;
    else
        rating = 1;

    if (imperial_gift_level < tribute) {
        rating--;
    } else if (imperial_gift_level != tribute) {
        if (imperial_gift_level >= tribute * 10)
            rating += 6;
        else if (imperial_gift_level >= tribute * 7)
            rating += 5;
        else if (imperial_gift_level >= tribute * 5)
            rating += 4;
        else if (imperial_gift_level >= tribute * 3)
            rating += 3;
        else if (imperial_gift_level >= tribute * 2)
            rating += 2;
        else
            rating++;
    }

    players_denarii -= imperial_gift_level;
    imperial_favour += rating * 10;
    if (imperial_favour < 0)
        imperial_favour = 0;
    if (imperial_favour > 200)
        imperial_favour = 200;

    if (rating <= -1) rating = -1;
    if (rating >= 5)  rating = 5;

    warned_of_emperor_reply_month = month + 2;
    if (warned_of_emperor_reply_month >= 12)
        warned_of_emperor_reply_month -= 12;
    warned_of_emperor_reply_month++;
    warned_of_emperor_reply_level = rating + 1;

    total_amount_of_bribes += imperial_gift_level;
    total_no_of_bribes++;
    av_imperial_gift_level = total_amount_of_bribes / total_no_of_bribes;

    if (final_bribe != 0) {
        if (rating <= -1) {
            final_bribe = 2;
            imperial_favour = 0;
        } else {
            final_bribe = 0;
        }
    }
}
