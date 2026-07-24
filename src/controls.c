#include "c2_data.h"
#include "c2_types.h"
#include "refresh.h"

char button_speed_profile[40] = { 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1 };


extern int  get_string_width(char *src, unsigned char *font);
extern void font_list(int idx, int word_count, int x, int y, unsigned char *font, int color);
extern void font_no(int value, char pad_char, char *suffix, int x, int y, unsigned char *font, int color);
extern void font_format_split(int idx, int word_skip, int x, int y_start, int max_width, int line_limit, int x_overflow, int max_width_overflow, unsigned char *font, int color);
extern void place_16x16_block(unsigned char *panel_addr);
extern void place_24x24_block(unsigned char *panel_addr);
extern void place_32x32_block(unsigned char *panel_addr);
/* Forward declarations (functions defined later in this file). */
void show_buttons(int x, int y, struct button_rec *button_list, int button_count);
void show_an_exit_button(int x, int y);
void mid_slider_var(struct slider_rec *slider_ptr, int track_position);
void down_slider_var(struct slider_rec *slider_ptr);
void up_slider_var(struct slider_rec *slider_ptr);
void de_toggle_all_icons(struct icon_rec *icon_list, int icon_count);
void show_menu_items(struct menu_item_rec *, int, int, int, int, int);


// Draw the top-bar menus, record their horizontal hit boxes, and mark the strip for refresh.
// FUNCTION: C2 0x2d4a5
// FUNCTION: C2WIN 0x0041ff90
void show_menus(struct menu_rec *menu_list, int menu_count, int active_menu_idx)
{
    int text_pixel_width;
    int start_x;
    int menu_y;
    int menu_text;
    int menu_no;
    int menu_x;
    int col_no;
    int ry;
    int refresh_width;
    int tile_rows;

#if !C2_FEAT_SOFTWARE_MENUS
    return;
#endif
    cover_mouse_droppings();
    hold_mouse_replace = 1;
    x_is = 0;
    x_is = menu_list->u.start_x;
    start_x = x_is;
    for (menu_no = 1; menu_no <= menu_count; menu_no++) {
        menu_text = menu_list->text;
        menu_list->u.pos.x1 = x_is;
        menu_x = menu_list->u.pos.x1;
        menu_y = menu_list->y;
        if (menu_no == active_menu_idx) {
            get_text_pointer(menu_text, 0);
            text_pixel_width = get_string_width(text_pointer, font1);
            sprite_width = (text_pixel_width + 4) / 16 + 2;
            sprite_height = 0xf;
            show_fast_rect(menu_x - 2, menu_y - 1, 0x10);
            font_list(menu_text, 0, menu_x, menu_y, font1, 0x1a);
        } else {
            get_text_pointer(menu_text, 0);
            text_pixel_width = get_string_width(text_pointer, font1);
            sprite_width = (text_pixel_width + 4) / 16 + 2;
            sprite_height = 0xf;
            show_fast_rect(menu_x - 2, menu_y - 1, 0x1a);
            font_list(menu_text, 0, menu_x, menu_y, font1, 0x10);
        }
        menu_list->u.pos.x2 = x_is;
        x_is += 0x20;
        menu_list++;
    }
    ref_x = (start_x - 2) / 16;
    ref_y = (menu_y - 1) / 16;
    refresh_width = (x_is - start_x - 2) / 16 + 2;
    tile_rows = 2;
    for (ry = ref_y; ry < ref_y + tile_rows; ry++)
        for (col_no = ref_x; col_no < ref_x + refresh_width; col_no++)
            svga_refresh_table[col_no + ry * 40] = 2;
}

// Draw a menu's drop-down items and highlight the active row.
// FUNCTION: C2 0x2d66e
// FUNCTION: C2WIN 0x004201d2
void show_menu_items(struct menu_item_rec *item_list, int x, int y, int text_group, int item_count, int active_item_idx)
{
    int i;
    int item_y;
    int menu_text;
    int col_no;
    int ry;
    int refresh_width;
    int tile_rows;

    sprite_width = 9;
    sprite_height = item_count * 20 + 4;
    show_fast_rect(x, y + 0x12, 0x1a);
    for (i = 1; i <= item_count; i++) {
        menu_text = item_list->text;
        item_y = y + 0x18 + item_list->y;
        if (i == active_item_idx) {
            sprite_width = 9;
            sprite_height = 0xf;
            show_fast_rect(x, item_y - 1, 0x10);
            font_list(text_group, menu_text, x + 0x10, item_y, font1, 0x1a);
        }
        else font_list(text_group, menu_text, x + 0x10, item_y, font1, 0x10);
        item_list++;
    }
    ref_x = x / 16;
    ref_y = (y + 0x12) / 16;
    refresh_width = 10;
    tile_rows = (item_count * 20 + 4) / 16 + 2;
    for (ry = ref_y; ry < ref_y + tile_rows; ry++)
        for (col_no = ref_x; col_no < ref_x + refresh_width; col_no++) svga_refresh_table[col_no + ry * 40] = 2;
}

// Draw a system window sized for the current selection list.
// FUNCTION: C2 0x2d7bd
// FUNCTION: C2WIN 0x0042036d
void show_selection_box(int selection_count, int x, int y, int text_group)
{
    (void)selection_count;
    (void)text_group;
    cover_mouse_droppings();
    hold_mouse_replace = 1;
    show_a_system_window(x, y,
                         (select_width + select_cost_flag) / 16,
                         select_height / 16);
}

int check_selection_goods_list(short goods_kind);
int check_highlight_list(short goods_kind);
void clear_highlight_goods_list(void);
void get_selection_goods_list(int);
void show_selections(struct selection_rec *, int, int, int, int, int);
void show_confirming_panel(int, int, int);
void show_Xconfirming_panel(int, int, int);
void show_adjusting_panel(int, int, int, int);
void show_warning_panel(int, int, int);
void show_icons(struct icon_rec *, int);
int slider_control(struct slider_rec *, int);

// Determine which choices are available and calculate the selection panel's dimensions.
// FUNCTION: C2 0x2d80c
// FUNCTION: C2WIN 0x004203b8
void get_allowed_selections(struct selection_rec *list, int count, int what)
{
    int current_cost;
    int text_word_no;
    int i;
    int max_cost;
    int text_width;

    select_count = 0;
    select_width = 0;
    select_height = 0;
    select_cost_flag = 0;
    max_cost = 0;
    for (i = 1; i <= count; i++) {
        if (list->max_population > max_population) {
            list->visible = 0;
        } else if (check_selection_goods_list(list->goods_kind) == 0) {
            list->visible = 0;
        } else {
            list->visible = 1;
            select_count++;
            text_word_no = list->text_word;
            get_text_pointer(what, text_word_no);
            text_width = get_string_width(text_pointer, font1);
            if (text_width > select_width) select_width = text_width;
            if (list->cost_kind != 0) {
                if (map_mode == 0) current_cost = city_costs[list->cost_kind];
                else current_cost = region_costs[list->cost_kind];
                if (current_cost > max_cost) max_cost = current_cost;
            }
        }
        list->highlighted = check_highlight_list(list->goods_kind) != 0;
        list++;
    }
    if (max_cost <= 0) select_cost_flag = 0;
    else if (max_cost < 100) select_cost_flag = 0x30;
    else if (max_cost < 1000) select_cost_flag = 0x40;
    else select_cost_flag = 0x50;
    select_height = select_count * 20 + 0x20;
    select_width += 0x30;
}

int control_buttons(int, int, struct button_rec *, int);
int control_selection(struct selection_rec *, int, int, int, int);
int over_selection(int, int, int);

// Clear all goods highlights.
// FUNCTION: C2 0x2d942
// FUNCTION: C2WIN 0x004205ad
void clear_highlight_goods_list(void)
{
    int i;

    for (i = 0; i < 17; i++)
        highlight_goods_list[i] = 0;
}

// Build the allowed-goods list from city industries or the province's selected source group.
// FUNCTION: C2 0x2d958
// FUNCTION: C2WIN 0x004205e6
void get_selection_goods_list(int mode)
{
    int i;

    for (i = 0; i < 17; i++)
        selection_goods_list[i] = 0x10;
    if (mode == 0) {
        for (i = 0; i < 0x10; i++) {
            if (industry[i].status == 1) highlight_goods_list[i] = 1;
            else highlight_goods_list[i] = 0;
            if (industry[i].status != 0) {
                selection_goods_list[i] = i;
            }
        }
    }
    if (mode == 1) {
        for (i = 0; i < 3; i++) {
            selection_goods_list[i] = region_sources[province_is].choices[i];
        }
    } else if (mode == 2) {
        for (i = 0; i < 3; i++) {
            selection_goods_list[i] = region_sources[province_is].choices[3 + i];
        }
    } else if (mode == 3) {
        for (i = 0; i < 3; i++) {
            selection_goods_list[i] = region_sources[province_is].choices[6 + i];
        }
    }
}

// Return whether a goods kind is present in the current selection list.
// FUNCTION: C2 0x2da47
// FUNCTION: C2WIN 0x0042077b
int check_selection_goods_list(short goods_kind)
{
    int i;
    for (i = 0; i < 0x11; i++) {
        if (goods_kind == selection_goods_list[i]) return 1;
    }
    return 0;
}

// Return whether a goods kind should be highlighted.
// FUNCTION: C2 0x2da67
// FUNCTION: C2WIN 0x004207cf
int check_highlight_list(short goods_kind)
{
    if (highlight_goods_list[goods_kind] != 0) {
        return 1;
    }
    return 0;
}

void click_warning(int, int, int);
void confirm(int, int, int);
void extended_confirm(int, int, int);

// Draw the visible selection choices, including costs and selection or highlight colours.
// FUNCTION: C2 0x2da7a
// FUNCTION: C2WIN 0x004207ff
void show_selections(struct selection_rec *selection_list, int selection_count, int x, int y, int text_group, int selected_row_idx)
{
    int col_no;
    int i;
    int item_y;
    int refresh_count;
    int cols_count;
    int visible_index;
    int selection_word_no;
    int cost_no;
    int ry;

    sprite_width = select_width / 16 - 1;
    sprite_width += select_cost_flag / 16;
    sprite_height = select_height - 0x18;
    show_fast_rect(x + 8, y + 8, 0x1a);
    visible_index = 0;
    for (i = 1; i <= selection_count; i++) {
        if (selection_list->visible != 0) {
            selection_word_no = selection_list->text_word;
            item_y = y + 0xc + visible_index * 20;
            if (selection_list->cost_kind != 0) {
                if (map_mode == 0) cost_no = city_costs[selection_list->cost_kind];
                else cost_no = region_costs[selection_list->cost_kind];
            } else {
                cost_no = 0;
            }
            if (selected_row_idx - 1 == visible_index) {
                sprite_width = select_width / 16 - 1;
                sprite_width += select_cost_flag / 16;
                sprite_height = 0xf;
                show_fast_rect(x + 8, item_y - 1, 0x10);
                font_list(text_group, selection_word_no, x + 8, item_y, font1, 0x1a);
                if (cost_no != 0)
                    font_no(cost_no, 0x40, "Dn", x + select_width - 0x10, item_y, font1, 0x1a);
            } else {
                if (selection_list->highlighted != 0)
                    font_list(text_group, selection_word_no, x + 8, item_y, font1, 0xb);
                else
                    font_list(text_group, selection_word_no, x + 8, item_y, font1, 0x10);
                if (cost_no != 0) {
                    if (selection_list->highlighted != 0)
                        font_no(cost_no, 0x40, "Dn", x + select_width - 0x10, item_y,
                                font1, 0xb);
                    else
                        font_no(cost_no, 0x40, "Dn", x + select_width - 0x10, item_y,
                                font1, 0x10);
                }
            }
            visible_index++;
        }
        selection_list++;
    }
    ref_x = (x + 8) / 16;
    ref_y = (y + 8) / 16;
    cols_count = (select_width + select_cost_flag) / 16;
    refresh_count = select_height / 16;
    for (ry = ref_y; ry < ref_y + refresh_count; ry++)
        for (col_no = ref_x; col_no < ref_x + cols_count; col_no++)
            svga_refresh_table[col_no + ry * 40] = 2;
}

// Draw a confirmation dialog with its message and yes/no buttons.
// FUNCTION: C2 0x2dcfc
// FUNCTION: C2WIN 0x00420b2e
void show_confirming_panel(int message_idx, int x, int y)
{
    cover_mouse_droppings();
    hold_mouse_replace = 1;
    show_a_mosaic_window(x - 0x10, y - 0x10, 0xc, 8);
    font_list(0xa, message_idx, x + 0x10, y + 0x10, font1, 0x10);
    show_buttons(x, y, confirming_buttons, 2);
}

// Draw a taller confirmation dialog with wrapped message text and yes/no buttons.
// FUNCTION: C2 0x2dd58
// FUNCTION: C2WIN 0x00420b99
void show_Xconfirming_panel(int message_idx, int x, int y)
{
    cover_mouse_droppings();
    hold_mouse_replace = 1;
    show_a_mosaic_window(x - 0x10, y - 0x30, 0xc, 0xa);
    font_format_split(0xa, message_idx, x + 0x10, y - 0x16,
                      0x90, 0x64, 0, 0, font1, 0x10);
    show_buttons(x, y, confirming_buttons, 2);
}

// Draw a numeric adjustment dialog using the requested display format.
// FUNCTION: C2 0x2dda9
// FUNCTION: C2WIN 0x00420c0f
void show_adjusting_panel(int message_idx, int x, int y, int display_kind)
{
    cover_mouse_droppings();
    hold_mouse_replace = 1;
    show_a_mosaic_window(x - 0x10, y - 0x10, 0xf, 7);
    show_an_exit_button(x + 0xb4, y + 0x34);
    font_list(0xc, message_idx, x + 0x10, y + 8, font1, 0x10);
    font_list(0xc, 0, x + 0x10, y + 0x3c, font1, 0x10);
    if (display_kind == 1) {
        font_no(*adjust_var / 10, 0x20, " ", x + 0x64, y + 0x22, font1, 0x10);
    } else if (display_kind == 2) {
        font_no(*adjust_var, 0x20, " ", x + 0x64, y + 0x22, font1, 0x10);
    } else {
        font_no(*adjust_var, 0x20, "%", x + 0x64, y + 0x22, font1, 0x10);
    }
    show_buttons(x, y, adjusting_buttons, 2);
}

// Draw a warning window with two message lines and an OK prompt.
// FUNCTION: C2 0x2de9c
// FUNCTION: C2WIN 0x00420d5d
void show_warning_panel(int message_idx, int x, int y)
{
    cover_mouse_droppings();
    hold_mouse_replace = 1;
    show_a_system_window(x, y, 0x14, 5);
    font_list(0xb, message_idx,     x + 0x10, y + 0x10, font1, 0x10);
    font_list(0xb, message_idx + 1, x + 0x10, y + 0x20, font1, 0x10);
    font_list(9,   0,        x + 0x60, y + 0x3c, font1, 0x10);
}

// Load each icon's position and current pressed or released image into the sprite state.
// FUNCTION: C2 0x2df1d
// FUNCTION: C2WIN 0x00420df0
void show_icons(struct icon_rec *icon_list, int icon_count)
{
    int i;
    cover_mouse_droppings();
    hold_mouse_replace = 1;
    for (i = 0; i < icon_count; i++) {
        sprite_x = icon_list->x;
        sprite_y = icon_list->y;
        sprite_image_no = icon_list->sprite;
        if (icon_list->down != 0)
            sprite_image_no++;
        icon_list++;
    }
}

// Draw a set of buttons with images selected from their type and state, then mark them for refresh.
// FUNCTION: C2 0x2df68
// FUNCTION: C2WIN 0x00420e66
void show_buttons(int x, int y, struct button_rec *button_list, int button_count)
{
    struct button_rec *button_ptr = button_list;
    int button_count_copy = button_count;
    int i;
    int button_size;
    int refresh_width;
    int refresh_height;
    int refresh_x;
    int refresh_y;

    for (i = 0; i < button_count_copy; i++) {
        sprite_x = x + button_ptr->x;
        sprite_y = y + button_ptr->y;
        if (button_ptr->type == 4) {
            sprite_image_no = button_ptr->sprite;
            if (button_ptr->down != 0)
                sprite_image_no = button_ptr->sprite + 1;
        } else {
            if (button_ptr->state == 0)
                sprite_image_no = button_ptr->sprite;
            else
                sprite_image_no = button_ptr->sprite + 1;
        }

        button_size = button_ptr->size;
        if (button_size == 0x10)      place_16x16_block(system_panel);
        else if (button_size == 0x18) place_24x24_block(system_panel);
        else if (button_size == 0x20) place_32x32_block(system_panel);

        button_ptr++;
        ref_x = sprite_x / 16;
        ref_y = sprite_y / 16;

        if (button_size == 0x10) {
            refresh_width = 2;
            refresh_height = 2;
        } else {
            refresh_width = 3;
            refresh_height = 3;
        }
        for (refresh_y = ref_y; refresh_y < ref_y + refresh_height; refresh_y++) {
            for (refresh_x = ref_x; refresh_x < ref_x + refresh_width; refresh_x++) {
                svga_refresh_table[refresh_x + refresh_y * 40] = 2;
            }
        }
    }
}


// Draw the standard exit button and record its hit-test position.
// FUNCTION: C2 0x2e0a1
// FUNCTION: C2WIN 0x0042102b
void show_an_exit_button(int x, int y)
{
    exit_x_at = x;
    exit_y_at = y;
    sprite_image_no = 0x33;
    sprite_x = x;
    sprite_y = y;
    place_24x24_block(system_panel);
}

// Handle clicks on slider decrement buttons, tracks, and increment buttons.
// FUNCTION: C2 0x2e0cb
// FUNCTION: C2WIN 0x0042106d
int slider_control(struct slider_rec *slider_list, int slider_count)
{
    int index;
    int spacing;
    int slider_range;
    int minimum;
    int maximum;
    int knob_width;

    if (mouse_left_button == 0) return 0;
    for (index = 0; index < slider_count; index++) {
        spacing = slider_list->step_pixels;
        slider_range = slider_list->slider_range;
        minimum = slider_list->min_pixel;
        maximum = slider_list->max_pixel;
        knob_width = (maximum - minimum) * spacing / slider_range;
        if (slider_list->y <= mouse_y && slider_list->y + 10 > mouse_y) {
            if (slider_list->x <= mouse_x && slider_list->x + 12 > mouse_x) {
                down_slider_var(slider_list);
                return index + 1;
            }
            if (slider_list->x + 12 <= mouse_x && slider_list->x + knob_width + 12 > mouse_x) {
                mid_slider_var(slider_list, mouse_x - 12 - slider_list->x);
                return index + 1;
            }
            if (slider_list->x + 12 + knob_width <= mouse_x && slider_list->x + knob_width + 0x18 > mouse_x) {
                up_slider_var(slider_list);
                return index + 1;
            }
        }
        slider_list++;
    }
    return 0;
}

// Set a slider from a track position while balancing its complementary value.
// FUNCTION: C2 0x2e1a1
// FUNCTION: C2WIN 0x004211f2
void mid_slider_var(struct slider_rec *slider_ptr, int track_position)
{
    int slider_range, max_value, min_value;
    int step_pixels, new_value;
    int old_value;

    old_value = (signed char)*slider_ptr->value;
    max_value = slider_ptr->max;
    min_value = slider_ptr->min;
    step_pixels = slider_ptr->step_pixels;
    slider_range = slider_ptr->slider_range;
    slider_ptr->refresh_flag = 2;
    new_value = slider_range * track_position;
    new_value /= step_pixels;
    *slider_ptr->value = new_value;
    if ((signed char)*slider_ptr->value >= max_value) *slider_ptr->value = max_value;
    if ((signed char)*slider_ptr->value < min_value) *slider_ptr->value = min_value;
    if (new_value < old_value) *slider_ptr->complement += (char)(old_value - new_value);
    else if (new_value > old_value) {
        if ((signed char)*slider_ptr->complement < new_value - old_value) {
            *slider_ptr->value = (char)(old_value + (signed char)*slider_ptr->complement);
            *slider_ptr->complement = 0;
        }
        else *slider_ptr->complement -= (char)(new_value - old_value);
    }
}

// Decrease a slider by one effective step, respecting its minimum and complement.
// FUNCTION: C2 0x2e24a
// FUNCTION: C2WIN 0x0042131c
void down_slider_var(struct slider_rec *slider_ptr)
{
    int old_value;
    int min_value;
    int step_value;
    int current_value;
    int previous_percent;
    int attempts_remaining;
    int current_percent;

    if (mouse_left_preclick == 0) return;
    old_value = (signed char)*slider_ptr->value;
    min_value = slider_ptr->min;
    step_value = slider_ptr->step;
    slider_ptr->refresh_flag = 2;
    slider_ptr->down_anim = 4;
    if (slidper_on == 0) {
        *slider_ptr->value -= (char)step_value;
    } else {
        attempts_remaining = 100;
        current_percent = totalXpercent(slider_total, old_value);
        previous_percent = current_percent;
        while (previous_percent == current_percent && attempts_remaining != 0 && current_percent >= min_value) {
            attempts_remaining--;
            *slider_ptr->value -= (char)step_value;
            current_percent = totalXpercent(slider_total, (signed char)*slider_ptr->value);
        }
        if (current_percent == 0) *slider_ptr->value = min_value;
    }
    if ((signed char)*slider_ptr->value < min_value)
        *slider_ptr->value = min_value;
    current_value = (signed char)*slider_ptr->value;
    if (current_value < old_value)
        *slider_ptr->complement += (char)(old_value - current_value);
    else if (current_value > old_value)
        *slider_ptr->complement -= (char)(current_value - old_value);
}

// Increase a slider by one effective step when its complement permits it.
// FUNCTION: C2 0x2e327
// FUNCTION: C2WIN 0x00421495
void up_slider_var(struct slider_rec *slider_ptr)
{
    int step_value;
    int old_value;
    int max_value;
    int min_value;
    int current_value;
    int previous_percent;
    int current_percent;
    int attempts_remaining;

    if (mouse_left_preclick == 0) return;
    old_value = (signed char)*slider_ptr->value;
    max_value = slider_ptr->max;
    min_value = slider_ptr->min;
    step_value = slider_ptr->step;
    if ((signed char)*slider_ptr->complement < min_value + step_value) return;
    slider_ptr->refresh_flag = 2;
    slider_ptr->up_anim = 4;
    if (slidper_on == 0) {
        *slider_ptr->value += (char)step_value;
    } else {
        attempts_remaining = 100;
        current_percent = totalXpercent(slider_total, old_value);
        previous_percent = current_percent;
        while (previous_percent == current_percent && attempts_remaining != 0) {
            attempts_remaining--;
            *slider_ptr->value += (char)step_value;
            current_percent = totalXpercent(slider_total, (signed char)*slider_ptr->value);
        }
    }
    if ((signed char)*slider_ptr->value > max_value)
        *slider_ptr->value = max_value;
    current_value = (signed char)*slider_ptr->value;
    if (current_value < old_value)
        *slider_ptr->complement += (char)(old_value - current_value);
    else if (current_value > old_value)
        *slider_ptr->complement -= (char)(current_value - old_value);
}

// Activate the clicked icon, release the others, and invoke its callback parameters.
// FUNCTION: C2 0x2e406
// FUNCTION: C2WIN 0x00421613
int control_icons(struct icon_rec *icon_list, int icon_count)
{
    struct icon_rec *icon_base_ptr;
    int i;

    if (mouse_left_preclick == 0) return 0;
    icon_base_ptr = icon_list;
    for (i = 0; i < icon_count; i++) {
        if (icon_list->x <= mouse_x && icon_list->x + 0x20 > mouse_x &&
            icon_list->y <= mouse_y && icon_list->y + 0x20 > mouse_y) {
            de_toggle_all_icons(icon_base_ptr, icon_count);
            para1 = icon_list->para1;
            para2 = icon_list->para2;
            icon_list->down = 1;
            icon_list->callback();
            return i + 1;
        }
        icon_list++;
    }
    return 0;
}

// Release all icons and mark their screen regions for repainting.
// FUNCTION: C2 0x2e47b
// FUNCTION: C2WIN 0x004216f6
void de_toggle_all_icons(struct icon_rec *icon_list, int icon_count)
{
    int i;
    int refresh_x;
    int refresh_y;
    for (i = 0; i < icon_count; i++) {
        icon_list->down = 0;
        ref_x = icon_list->x >> 4;
        ref_y = icon_list->y >> 4;
        for (refresh_y = ref_y; refresh_y < ref_y + 3; refresh_y++)
            for (refresh_x = ref_x; refresh_x < ref_x + 3; refresh_x++)
                if (refresh_x + refresh_y * 40 <= 0x4b0)
                    svga_refresh_table[refresh_x + refresh_y * 40] = 1;
        icon_list++;
    }
}

// Update button repeat and visual states, then handle toggle, momentary, and repeating clicks.
// FUNCTION: C2 0x2e4f3
// FUNCTION: C2WIN 0x004217c7
int control_buttons(int x, int y, struct button_rec *button_list, int button_count)
{
    int i;
    int button_size;
    int button_type;
    struct button_rec *button_ptr;

    button_ptr = button_list;
    for (i = 0; i < button_count; i++) {
        if (button_ptr->down != 0) {
            if (button_ptr->repeat == 0) button_ptr->repeat = 1;
            else if (button_time_flag != 0) button_ptr->repeat++;
        } else {
            button_ptr->repeat = 0;
        }
        button_ptr->down = 0;
        button_type = button_ptr->type;
        if (button_type == 4) {
            if (button_ptr->repeat == 0) button_ptr->state = 0;
            if (button_ptr->state != 0) { para1 = button_ptr->para1; para2 = button_ptr->para2; button_ptr->callback(); }
        }
        if (button_ptr->type == 3) button_ptr->state = 0;
        button_ptr++;
    }

    if (mouse_left_button == 0) return 0;
    for (i = 0; i < button_count; i++) {
        button_size = button_list->size;
        if (button_list->x + x <= mouse_x && button_list->x + button_size + x > mouse_x)
        if (button_list->y + y <= mouse_y && button_list->y + button_size + y > mouse_y) {
            int button_type = button_list->type;
            if (button_type == 4) {
                button_list->down = 1;
                if (button_list->repeat == 0) button_list->state = 1;
                else if (button_time_flag != 0) {
                    int repeat_count = button_list->repeat;
                    if (repeat_count >= 0x30) { button_list->state = 1; button_list->repeat = 0x30; }
                    else if (repeat_count < 8) button_list->state = 0;
                    else if (button_speed_profile[repeat_count - 8] != 0) button_list->state = 1;
                    else button_list->state = 0;
                } else button_list->state = 0;
                return i + 1;
            }
            if (button_type == 2) {
                button_list->down = 1;
                if (button_list->repeat == 0) {
                    button_list->state ^= 1;
                    para1 = button_list->para1;
                    para2 = button_list->para2;
                    button_list->callback();
                }
                return i + 1;
            }
            if (button_type == 3) {
                button_list->down = 1;
                button_list->state = 1;
                if (button_list->repeat == 0) {
                    para1 = button_list->para1;
                    para2 = button_list->para2;
                    button_list->callback();
                }
                return i + 1;
            }
        }
        button_list++;
    }
    return 0;
}

// Run top-bar menu interaction and invoke the selected item's action.
// FUNCTION: C2 0x2e67d
// FUNCTION: C2WIN 0x00421ae2
int control_menus(struct menu_rec *menu_list, int menu_count, void (*draw_map_fn)(void))
{
    int item_number;
    int old_mode;
    int over;
    int active_menu;
    int finish;
    int menu_tracking;
    struct menu_item_rec *current_item;
    struct menu_rec *current_menu;

    if (mouse_left_preclick == 0) return 0;
    if (tutorial_mode != 0) return 0;
    active_menu = over_menu(menu_list, menu_count);
    item_number = 0;
    if (active_menu == 0) return 0;
    old_mode = pointer_mode;
    pointer_mode = 0;
    current_menu = menu_list + active_menu - 1;
    show_menus(menu_list, menu_count, active_menu);
    setup_map_screen_refresh();
    mouse_left_preclick = 0;
    menu_tracking = 1;
    finish = 0;
    while (finish == 0) {
        if (mouse_left_button == 0) menu_tracking = 0;
        gloop_start();
        update_map = 1;
        draw_map_fn();
        show_menu_items(current_menu->items, current_menu->u.pos.x1, current_menu->y, current_menu->text, current_menu->item_count, item_number);
        gloop_end();
        if (menu_tracking != 0) {
            over = over_menu(menu_list, menu_count);
            if (over != 0 && active_menu != over) {
                active_menu = over;
                current_menu = menu_list + active_menu - 1;
                show_menus(menu_list, menu_count, active_menu);
            }
        }
        item_number = over_item(current_menu->items, current_menu->item_count, current_menu->u.pos.x1, current_menu->y);
        if (mouse_left_click != 0 && item_number != 0) finish = 1;
        if (mouse_left_preclick != 0) finish = 1;
        if (mouse_right_preclick != 0) { finish = 1; item_number = 0; }
    }
    show_menus(menu_list, menu_count, 0);
    if (item_number != 0) {
        current_item = current_menu->items;
        current_item += item_number - 1;
        current_item->action();
    }
    setup_map_screen_refresh();
    update_map = 1;
    pointer_mode = old_mode;
    clear_mouse();
    return 1;
}

// Return the one-based top-bar menu index under the mouse, or zero.
// FUNCTION: C2 0x2e81c
// FUNCTION: C2WIN 0x00421d3c
int over_menu(struct menu_rec *menu_list, int menu_count)
{
    int i;
    for (i = 1; i <= menu_count; i++) {
        if (menu_list->u.pos.x1 <= mouse_x && menu_list->u.pos.x2 > mouse_x
         && menu_list->y <= mouse_y && menu_list->y + 0xc > mouse_y) {
            return i;
        }
        menu_list++;
    }
    return 0;
}

// Return the one-based drop-down item index under the mouse, or zero.
// FUNCTION: C2 0x2e868
// FUNCTION: C2WIN 0x00421dcb
int over_item(struct menu_item_rec *item_list, int item_count, int x, int y)
{
    int i;
    int item_y;

    for (i = 1; i <= item_count; i++) {
        item_y = (long)(y + 0x17) + item_list->y;
        if (x <= mouse_x && x + 0x60 > mouse_x
         && item_y <= mouse_y && item_y + 0xf > mouse_y) {
            return i;
        }
        item_list++;
    }
    return 0;
}

void adjust(int message_idx, int *value_ptr, int step_value, int max_value, int min_value, int x, int y, int display_kind);
int exit_screen(void);
int exit_screen_at(int, int);

// Run a selection dialog and invoke the callback for the chosen visible entry.
// FUNCTION: C2 0x2e8bb
// FUNCTION: C2WIN 0x00421e5e
int control_selection(struct selection_rec *selection_list, int selection_count, int x, int y, int text_group)
{
    int i;
    int visible;

    selection_is = 0;
    get_allowed_selections(selection_list, selection_count, text_group);
#if C2_FEAT_SELECTION_WIDTH_OFFSET
    x -= select_width;
#else
    x -= select_cost_flag;
#endif
    if (x < 0) x = 0;
    if (y < 0x18) y = 0x18;
#if C2_FEAT_SELECTION_WIDTH_OFFSET
    if (x + select_cost_flag >= 0x26c) x = 0x26c - select_cost_flag;
#else
    if (select_width + x >= 0x26c) x = 0x26c - select_width;
#endif
    if (select_height + y >= 0x1cc) y = 0x1cc - select_height;
    show_selection_box(selection_count, x, y, text_group);
    setup_whole_screen_refresh();
    mouse_left_preclick = 0;
    for (;;) {
        gloop_start();
        show_selections(selection_list, selection_count, x, y, text_group, selection_is);
        gloop_end();
        selection_is = over_selection(select_count, x, y);
        if (mouse_left_click != 0 && selection_is != 0) break;
        if (mouse_left_click != 0 && is_icon_over(last_icon_over) == 0) {
            selection_is = 0;
            break;
        }
        if (mouse_left_preclick != 0 && selection_is != 0) break;
        if (mouse_right_preclick != 0) { selection_is = 0; break; }
    }
    if (selection_is != 0 && selection_is <= select_count) {
        i = 1;
        visible = 1;
        for (; i <= selection_count; i++) {
            if (selection_list->visible != 0) {
                if (visible++ == selection_is) break;
            }
            selection_list++;
        }
        para1 = selection_list->para1;
        selection_list->callback();
        if (selection_is == select_count) selection_is = 0;
        else selection_is = i;
    } else {
        selection_is = 0;
    }
    setup_whole_screen_refresh();
    clear_mouse();
    clear_highlight_goods_list();
    return 1;
}

// Return the one-based selection row under the mouse, or zero.
// FUNCTION: C2 0x2ea4f
// FUNCTION: C2WIN 0x004220d2
int over_selection(int row_count, int x, int base_y)
{
    int i;
    int item_y;

    for (i = 0; i < row_count; ++i) {
        item_y = base_y + 7 + i * 20;
        if (x <= mouse_x &&
            x + select_width + select_cost_flag > mouse_x &&
            item_y <= mouse_y &&
            item_y + 20 > mouse_y) {
            return i + 1;
        }
    }
    return 0;
}

#if C2_FEAT_NATIVE_DIALOGS
extern char warning_text_1[];
extern char warning_text_2[];
extern char warning_message[];
extern void *main_window;
void get_text_string(int text_group, int text_word, char *destination);
void show_native_message(void *window, char *message);
extern unsigned char native_dialog_extended;
extern char *confirm_yes_text;
extern char *confirm_no_text;
int show_native_confirm(void *window, char *message, char *yes_text, char *no_text);
void act_yes(void);
void act_no(void);
#endif

// Show a modal warning until either mouse button is clicked.
// FUNCTION: C2 0x2eaac
// FUNCTION: C2WIN 0x0042216b
void click_warning(int message_idx, int x, int y)
{
#if C2_FEAT_NATIVE_DIALOGS
    get_text_string(0xb, message_idx, warning_text_1);
    get_text_string(0xb, message_idx + 1, warning_text_2);
    sprintf(warning_message, "%s %s", warning_text_1, warning_text_2);
    show_native_message(main_window, warning_message);
#else
    clear_mouse();
    show_warning_panel(message_idx, x, y);
    setup_whole_screen_refresh();
    out1 = 0;
    while (out1 != 1) {
        gloop_start();
        gloop_end();
        if (mouse_left_click != 0 || mouse_right_click != 0) {
            out1 = 1;
        }
    }
    setup_whole_screen_refresh();
#endif
}

// Run a yes/no confirmation dialog and leave the result in the global decision state.
// FUNCTION: C2 0x2eb01
// FUNCTION: C2WIN 0x004221cc
void confirm(int message_idx, int x, int y)
{
#if C2_FEAT_NATIVE_DIALOGS
    int result;

    get_text_string(10, message_idx, warning_message);
    native_dialog_extended = 0;
    result = show_native_confirm(main_window, warning_message,
                                 confirm_yes_text, confirm_no_text);
    if (result == 1) act_yes();
    else act_no();
#else
    decision = 0;
    pointer_mode = 0;
    show_confirming_panel(message_idx, x, y);
    setup_whole_screen_refresh();
    refresh_svga_screen();
    clear_mouse();
    out1 = 0;
    while (out1 != 1) {
        gloop_start();
        show_buttons(x, y, confirming_buttons, 2);
        gloop_end();
        control_buttons(x, y, confirming_buttons, 2);
        if (out1 > 1) {
            out1 = out1 - 1;
        }
    }
    setup_whole_screen_refresh();
    update_map = 1;
#endif
}

// Run a taller yes/no confirmation dialog for wrapped messages.
// FUNCTION: C2 0x2eb95
// FUNCTION: C2WIN 0x0042222f
void extended_confirm(int message_idx, int x, int y)
{
    decision = 0;
    pointer_mode = 0;
    show_Xconfirming_panel(message_idx, x, y);
    setup_whole_screen_refresh();
    refresh_svga_screen();
    clear_mouse();
    out1 = 0;
    while (out1 != 1) {
        gloop_start();
        show_buttons(x, y, confirming_buttons, 2);
        gloop_end();
        control_buttons(x, y, confirming_buttons, 2);
        if (out1 > 1) {
            out1 = out1 - 1;
        }
    }
    setup_whole_screen_refresh();
    update_map = 1;
}

// Run a modal numeric adjustment dialog.
// FUNCTION: C2 0x2ec18
// FUNCTION: C2WIN 0x004222ac
void adjust(int message_idx, int *value_ptr, int step_value, int max_value, int min_value, int x, int y, int display_kind)
{
    adjust_var = value_ptr;
    adjust_step = step_value;
    adjust_max = max_value;
    adjust_min = min_value;
    show_adjusting_panel(message_idx, x, y, display_kind);
    setup_whole_screen_refresh();
    out1 = 0;
    while (out1 != 1) {
        gloop_start();
        show_buttons(x, y, adjusting_buttons, 2);
        stone_random_count = 0xa;
        show_a_mosaic_blank(x + 0x5c, y + 0x1e, 3, 1);
        if (display_kind == 1) {
            font_no(*adjust_var / 10, 0x20, " ", x + 0x64, y + 0x22, font1, 0x10);
        } else if (display_kind == 2) {
            font_no(*adjust_var, 0x20, " ", x + 0x64, y + 0x22, font1, 0x10);
        } else {
            font_no(*adjust_var, 0x20, "%", x + 0x64, y + 0x22, font1, 0x10);
        }
        refresh_a_square((x + 0x64) >> 4, (y + 0x22) >> 4, 1);
        gloop_end();
        control_buttons(x, y, adjusting_buttons, 2);
        if (mouse_right_click != 0) out1 = 4;
        if (exit_screen() != 0) out1 = 1;
        if (out1 > 1) out1--;
    }
    setup_whole_screen_refresh();
}

// Consume a left click on the recorded exit-button region.
// FUNCTION: C2 0x2ed7a
// FUNCTION: C2WIN 0x00422479
int exit_screen(void)
{
    if (!mouse_left_preclick) {
        return 0;
    }
    if (mouse_in_area(exit_x_at, exit_y_at, 24, 24)) {
        clear_mouse();
        return 1;
    }
    return 0;
}

// Consume a left click on an exit-button region at the supplied position.
// FUNCTION: C2 0x2edb5
// FUNCTION: C2WIN 0x004224d0
int exit_screen_at(int x, int y)
{
    if (!mouse_left_preclick)
        return 0;
    if (mouse_in_area(x, y, 0x18, 0x18)) {
        clear_mouse();
        return 1;
    }
    return 0;
}
