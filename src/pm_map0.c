
#include "c2_data.h"


/* Pseudo-map coordinate conversion and isometric diamond rendering. */
/* Forward declarations (functions defined later in this file). */
void three_by_three(int center_x, int center_y);
void four_by_four(int center_x, int center_y);
void show_one_ptr(int screen_cell_x, int screen_cell_y);


// Finds an actual map offset in the pseudo-map and stores its row and column in x and y.
// FUNCTION: C2 0x352aa
// FUNCTION: C2WIN 0x00484250
void get_pm_from_actual(int actual)
{
    int row;
    int col;

    x = 0;
    y = 0;
    for (row = 0; row < 0xa1; row++) {
        for (col = 0; col < 0x51; col++) {
            if (pseudo_map[row][col] >= 0x0FFF0000U) continue;
            else if (actual == pseudo_map[row][col]) {
                x = row;
                y = col;
                return;
            }
        }
    }
}

#if PLATFORM_WINDOWS
void pm_limits(void);
int get_pm_over_diamond(int);
void rotate_pm_clockwise(void);
void rotate_pm_anticlockwise(void);
void show_diamond_ptr(void);
void place_diamond(int);
void place_lefthalf_diamond(void);
void place_righthalf_diamond(void);
void place_overlay(int);
#endif

// Builds the oriented pseudo-map lookup and marks cells outside the active map with sentinels.
// FUNCTION: C2 0x35311
// FUNCTION: C2WIN 0x0048430f
void get_pseudo_map(int direction)
{
    int col_x2_delta;
    int x_move;
    int start_y;
    int map_row;
    int x2_origin;
#if PLATFORM_WINDOWS
    int column_idx;
#endif
    int col_row_stride;
    int edge_col;
    int change_row;
    int py;
    int row_edge;
    int p_x2;
#if PLATFORM_WINDOWS
    int num;
    int row_end;
    int mid_col;
#else
    int column_idx;
#endif

#if PLATFORM_WINDOWS
    num = 0xa1;
    row_end = 0xa0;
    mid_col = 0x28;
    for (map_row = 0; map_row < num; map_row++) {
        for (column_idx = 0; column_idx < 0x51; column_idx++) {
            if (map_row <= 0x50) row_edge = map_row; else row_edge = row_end - map_row;
            if (column_idx <= mid_col) edge_col = column_idx; else edge_col = 0x50 - column_idx;
            if (edge_col < 4 && row_edge < 8) {
                pseudo_map[map_row][column_idx] = 0x0FFF0000;
            } else if (edge_col < 8 && row_edge < 0x10) {
                pseudo_map[map_row][column_idx] = 0x0FFF0000 | 1;
            } else if (edge_col < 0xc && row_edge < 0x18) {
                pseudo_map[map_row][column_idx] = 0x0FFF0000 | 2;
            } else if (edge_col < 0x10 && row_edge < 0x20) {
                pseudo_map[map_row][column_idx] = 0x0FFF0000 | 3;
            } else if (edge_col < 0x13 && row_edge < 0x28) {
                pseudo_map[map_row][column_idx] = 0x0FFF0000 | 4;
            } else if (edge_col < 0x1c && row_edge < 0x14) {
                pseudo_map[map_row][column_idx] = 0x0FFF0000 | 5;
            } else if (edge_col < 8 && row_edge < 0x3c) {
                pseudo_map[map_row][column_idx] = 0x0FFF0000 | 6;
            } else {
                pseudo_map[map_row][column_idx] = 0x0FFF0000 | 7;
            }
        }
    }
#else
    for (map_row = 0; map_row < 0xa1; map_row++) {
        for (column_idx = 0; column_idx < 0x51; column_idx++) {
            if (map_row <= 0x50) row_edge = map_row; else row_edge = 0xa0 - map_row;
            if (column_idx <= 0x28) edge_col = column_idx; else edge_col = 0x50 - column_idx;
            if (edge_col < 4 && row_edge < 8) {
                pseudo_map[map_row][column_idx] = 0x0FFF0000;
            } else if (edge_col < 8 && row_edge < 0x10) {
                pseudo_map[map_row][column_idx] = 0x0FFF0000 | 1;
            } else if (edge_col < 0xc && row_edge < 0x18) {
                pseudo_map[map_row][column_idx] = 0x0FFF0000 | 2;
            } else if (edge_col < 0x10 && row_edge < 0x20) {
                pseudo_map[map_row][column_idx] = 0x0FFF0000 | 3;
            } else if (edge_col < 0x13 && row_edge < 0x28) {
                pseudo_map[map_row][column_idx] = 0x0FFF0000 | 4;
            } else if (edge_col < 0x1c && row_edge < 0x14) {
                pseudo_map[map_row][column_idx] = 0x0FFF0000 | 5;
            } else if (edge_col < 8 && row_edge < 0x3c) {
                pseudo_map[map_row][column_idx] = 0x0FFF0000 | 6;
            } else {
                pseudo_map[map_row][column_idx] = 0x0FFF0000 | 7;
            }
        }
    }
#endif

    map_direction = direction;
    if (direction == 0) {
        start_y = map_height_reduction * 2 + 1;
        change_row = 1;
        col_row_stride = 1;
        x2_origin = 0x50;
        x_move = -1;
        col_x2_delta = 1;
    } else if (direction == 2) {
        start_y = 0x50;
        change_row = 1;
        col_row_stride = -1;
        x2_origin = map_width_reduction * 2 + 1;
        x_move = 1;
        col_x2_delta = 1;
    } else if (direction == 4) {
        start_y = (0x50 - map_height_reduction) * 2 - 1;
        change_row = -1;
        col_row_stride = -1;
        x2_origin = 0x50;
        x_move = 1;
        col_x2_delta = -1;
    } else if (direction == 6) {
        start_y = 0x50;
        change_row = -1;
        col_row_stride = 1;
        x2_origin = (0x50 - map_width_reduction) * 2 - 1;
        x_move = -1;
        col_x2_delta = -1;
    }

    for (map_row = 0; map_row < map_actual_height; map_row++) {
        py = start_y;
        p_x2 = x2_origin;
#if PLATFORM_WINDOWS
        num = map_row * map_actual_width;
#endif
        for (column_idx = 0; column_idx < map_actual_width; column_idx++) {
#if PLATFORM_WINDOWS
            pseudo_map[py][p_x2 / 2] = map_actual_atom * (column_idx + num);
#else
            pseudo_map[py][p_x2 / 2] = map_actual_atom * (map_actual_width * map_row + column_idx);
#endif
            py += col_row_stride;
            p_x2 += col_x2_delta;
        }
        start_y += change_row;
        x2_origin += x_move;
    }

    start_y = map_height_reduction * 2 + 1;
    x2_origin = 0x50;
#if PLATFORM_WINDOWS
    num = 0x50 - map_height_reduction * 2;
    for (map_row = 0; map_row < num; map_row++) {
        py = start_y;
        p_x2 = x2_origin;
        py += map_actual_width;
        p_x2 += map_actual_width;
        pseudo_map[py][p_x2 / 2] = 0x0FFF0000 | 0x9;
        start_y++;
        x2_origin--;
    }
    if (map_mode > 0) {
        py = start_y;
        p_x2 = x2_origin;
        py += map_actual_width;
        p_x2 += map_actual_width;
        pseudo_map[py][p_x2 / 2] = 0x0FFF0000 | 0xa;
    }
    py = start_y;
    p_x2 = x2_origin;
    num = 0x50 - map_width_reduction * 2;
    for (column_idx = 0; column_idx < num; column_idx++) {
        pseudo_map[py][p_x2 / 2] = 0x0FFF0000 | 0x8;
        py++;
        p_x2++;
    }
#else
    for (map_row = 0; map_row < 0x50 - map_height_reduction * 2; map_row++) {
        py = start_y;
        p_x2 = x2_origin;
        for (column_idx = 0; column_idx < map_actual_width; column_idx++) {
            py++;
            p_x2++;
        }
        pseudo_map[py][p_x2 / 2] = 0x0FFF0000 | 0x9;
        start_y++;
        x2_origin--;
    }
    if (map_mode > 0) {
        py = start_y;
        p_x2 = x2_origin;
        for (column_idx = 0; column_idx < map_actual_width; column_idx++) {
            py++;
            p_x2++;
        }
        pseudo_map[py][p_x2 / 2] = 0x0FFF0000 | 0xa;
    }
    py = start_y;
    p_x2 = x2_origin;
    for (column_idx = 0; column_idx < 0x50 - map_width_reduction * 2; column_idx++) {
        pseudo_map[py][p_x2 / 2] = 0x0FFF0000 | 0x8;
        py++;
        p_x2++;
    }
#endif
}

// Clamps the pseudo-map viewport to the active map bounds.
// FUNCTION: C2 0x356f7
// FUNCTION: C2WIN 0x00484805
void pm_limits(void)
{
    if (pm_x < 0) pm_x = 0;
    if (pm_y < 0) pm_y = 0;
    if (0x50 - pm_screen_width <= pm_x) pm_x = 0x50 - pm_screen_width;
    if (0xa0 - pm_screen_height <= pm_y) pm_y = 0xa0 - pm_screen_height;
}

// Resolves the mouse position to a visible pseudo-map diamond and updates the selection globals.
// FUNCTION: C2 0x3574e
// FUNCTION: C2WIN 0x0048488c
int get_pm_over_diamond(int force_zero_offset)
{
#if PLATFORM_WINDOWS
    int row;
    int x;
    int xp;
    int y;
    int part;
    int odd;
    int y_mod;
    int odd_y;
    int xpos;
    int grid_x;
#else
    int x;
    int y;
    int rel_x;
    int rel_y;
    int grid_x;
    int xpos;
    int y_mod;
    int part;
    int xp;
    int odd_y;
    int row;
    int next_x;
    int next_y;
    int odd;
#endif

    if (mouse_x + 0 < pm_screen_x_start) return 0;
    if (pm_screen_x_start + pm_screen_width * (pm_diamond_width + 0) <= mouse_x) return 0;
    if ((pm_screen_y_start - 0) + pm_diamond_half_height > mouse_y) return 0;
    if ((pm_screen_y_start - 0) + pm_diamond_half_height + pm_screen_height * pm_diamond_half_height <= mouse_y) return 0;

    if (map_mode == 2) {
        x = 0;
        y = 0x10;
    } else if (force_zero_offset) {
        x = 0;
        y = 0;
    } else if (pointer_mode > 0 && map_mode < 2) {
        x = 8;
        y = 8;
    } else if (pm_build_shape < 1) {
        x = 0;
        y = 0;
    } else if (pm_build_shape < 2) {
        x = 0;
        y = -pm_diamond_half_height;
    } else if (pm_build_shape < 3) {
        x = 0;
        y = -pm_diamond_half_height * 2;
    } else if (pm_build_shape < 4) {
        x = 0;
        y = -pm_diamond_half_height * 3;
    } else if (pm_build_shape < 5) {
        x = pm_diamond_width;
        y = -pm_diamond_half_height * 4;
    } else {
        x = pm_diamond_width;
        y = -pm_diamond_half_height * 5;
    }

#if PLATFORM_WINDOWS
    row = (mouse_y + y - ((pm_screen_y_start - 0) + pm_diamond_half_height)) / pm_diamond_half_height;
    grid_x = (mouse_x + x - pm_screen_x_start) / pm_diamond_half_width;
#else
    rel_y = mouse_y + y - ((pm_screen_y_start - 0) + pm_diamond_half_height);
    row = rel_y / pm_diamond_half_height;
    rel_x = mouse_x + x - pm_screen_x_start;
    grid_x = rel_x / pm_diamond_half_width;
#endif
    part = (grid_x + row) & 1;
    xp = grid_x & 1;
    odd_y = row & 1;
#if PLATFORM_WINDOWS
    xpos = (mouse_x + x - pm_screen_x_start) % pm_diamond_half_width;
#else
    xpos = rel_x % pm_diamond_half_width;
#endif
    xpos /= 2;
#if PLATFORM_WINDOWS
    y_mod = (mouse_y + y - ((pm_screen_y_start - 0) + pm_diamond_half_height)) % pm_diamond_half_height;
#else
    y_mod = rel_y % pm_diamond_half_height;
#endif

    pm_y_coord = row;
    pm_x_coord = grid_x / 2;
#if PLATFORM_DOS
    next_y = row + 1;
    next_x = pm_x_coord + 1;
#endif
    if (part == 0) {
        if (y_mod > xpos) {
#if PLATFORM_WINDOWS
            pm_y_coord++;
#else
            pm_y_coord = next_y;
#endif
        } else if (xp != 0 && odd_y != 0) {
#if PLATFORM_WINDOWS
            pm_x_coord++;
#else
            pm_x_coord = next_x;
#endif
        }
    } else if (part == 1) {
        if (y_mod + xpos >= pm_diamond_half_height - 1) {
#if PLATFORM_WINDOWS
            pm_y_coord++;
#else
            pm_y_coord = next_y;
#endif
            if (xp != 0 && odd_y == 0) {
#if PLATFORM_WINDOWS
                pm_x_coord++;
#else
                pm_x_coord = next_x;
#endif
            }
        }
    }

    odd = pm_y_coord & 1;
    pm_over_x = pm_screen_x_start + pm_x_coord * pm_diamond_width;
    if (odd)
        pm_over_x -= pm_diamond_half_width;
    pm_over_y = pm_screen_y_start + pm_y_coord * pm_diamond_half_height;
    pm_over_cm_ptr = pseudo_map[(pm_y_coord + pm_y)][pm_x_coord + pm_x + 0];
    if (pm_over_cm_ptr >= 0x0FFF0000) return 0;

    pm_x_edge = pm_y_edge = 0;
    if (pm_y_coord == 0) {
        pm_y_edge = 2;
    } else if (pm_y_coord >= pm_screen_height) {
        pm_y_edge = 1;
    }
    if (odd) {
        if (pm_x_coord == 0) {
            pm_x_edge = 2;
        } else if (pm_x_coord >= pm_screen_width) {
            pm_x_edge = 1;
        }
    }
    return 1;
}

#if PLATFORM_WINDOWS
void place_diamond(int style);
void place_overlay(int style);
void place_lefthalf_overlay(int style);
void place_righthalf_overlay(int style);
#endif

// Rotates the pseudo-map orientation clockwise.
// FUNCTION: C2 0x35a37
// FUNCTION: C2WIN 0x00484cb9
void rotate_pm_clockwise(void)
{
    int next_x;
    int next_y;

    map_direction += 2;
    if (map_direction > 6) map_direction = 0;
    get_pseudo_map(map_direction);

    if (zoom_level == 0) {
        next_y = (pm_y + 0xe) / 2;
        next_x = (0x50 - (pm_x + 4)) * 2;
        pm_x = next_y - 4;
        pm_y = next_x - 0xe;
    } else if (zoom_level == 1) {
        if (map_mode == 2) {
            next_y = (pm_y + 0x16) / 2;
            next_x = (0x50 - (pm_x + 0xb)) * 2;
            pm_x = next_y - 0xb;
            pm_y = next_x - 0x16;
        } else {
            next_y = (pm_y + 0x1e) / 2;
            next_x = (0x50 - (pm_x + 8)) * 2;
            pm_x = next_y - 8;
            pm_y = next_x - 0x1e;
        }
    } else if (zoom_level == 2) {
        if (map_mode == 2) {
            pm_x = 0xd;
            pm_y = 0x18;
#if PLATFORM_DOS
            return;
#endif
        }
#if PLATFORM_WINDOWS
        else
#endif
        {
            next_y = (pm_y + 0x46) / 2;
            next_x = (0x50 - (pm_x + 0xa)) * 2;
            pm_x = next_y - 0x14;
            pm_y = next_x - 0x46;
        }
    }
#if PLATFORM_WINDOWS
    if (map_mode == 0) city_rotation = map_direction;
    else               prov_rotation = map_direction;
#endif
}

// Rotates the pseudo-map orientation anticlockwise.
// FUNCTION: C2 0x35b80
// FUNCTION: C2WIN 0x00484e82
void rotate_pm_anticlockwise(void)
{
    int next_x;
    int next_y;

    map_direction -= 2;
    if (map_direction < 0) map_direction = 6;
    get_pseudo_map(map_direction);

    if (zoom_level == 0) {
        next_y = (0xa1 - (pm_y + 0xe)) / 2;
        next_x = (pm_x + 4) * 2;
        pm_x = next_y - 4;
        pm_y = next_x - 0xe;
    } else if (zoom_level == 1) {
        if (map_mode == 2) {
            next_y = (0xa1 - (pm_y + 0x16)) / 2;
            next_x = (pm_x + 0xb) * 2;
            pm_x = next_y - 0xb;
            pm_y = next_x - 0x16;
        } else {
            next_y = (0xa1 - (pm_y + 0x1e)) / 2;
            next_x = (pm_x + 8) * 2;
            pm_x = next_y - 8;
            pm_y = next_x - 0x1e;
        }
    } else if (zoom_level == 2) {
        if (map_mode == 2) {
            pm_x = 0xd;
            pm_y = 0x18;
#if PLATFORM_DOS
            return;
#endif
        }
#if PLATFORM_WINDOWS
        else
#endif
        {
            next_y = (0xa1 - (pm_y + 0x46)) / 2;
            next_x = (pm_x + 0x14) * 2;
            pm_x = next_y - 0x14;
            pm_y = next_x - 0x46;
        }
    }
#if PLATFORM_WINDOWS
    if (map_mode == 0) city_rotation = map_direction;
    else               prov_rotation = map_direction;
#endif
}

// Draws the pointer footprint for the current building shape.
// FUNCTION: C2 0x35cc0
// FUNCTION: C2WIN 0x0048504c
void show_diamond_ptr(void)
{
    int odd_row = (pm_y_coord & 1) != 0;

    if (pm_build_shape == 0) {
        show_one_ptr(pm_x_coord, pm_y_coord);
        return;
    }
    if (pm_build_shape == 1) {
        show_one_ptr(pm_x_coord, pm_y_coord);
        show_one_ptr(pm_x_coord - odd_row, pm_y_coord + 1);
        show_one_ptr(pm_x_coord - odd_row + 1, pm_y_coord + 1);
        show_one_ptr(pm_x_coord, pm_y_coord + 2);
        return;
    }
    if (pm_build_shape == 2) {
        three_by_three(pm_x_coord, pm_y_coord);
        return;
    }
    if (pm_build_shape == 3) {
        four_by_four(pm_x_coord, pm_y_coord);
        return;
    }
    if (pm_build_shape == 4) {
        three_by_three(pm_x_coord, pm_y_coord);
        if (odd_row) three_by_three(pm_x_coord - 2, pm_y_coord + 3);
        else        three_by_three(pm_x_coord - 1, pm_y_coord + 3);
        return;
    }
    if (pm_build_shape == 5) {
        four_by_four(pm_x_coord, pm_y_coord);
        four_by_four(pm_x_coord - 2, pm_y_coord + 4);
        return;
    }
    return;
}

// Draws a nine-tile isometric pointer footprint centered at (x, y).
// FUNCTION: C2 0x35dc0
// FUNCTION: C2WIN 0x00485206
void three_by_three(int center_x, int center_y)
{
    int odd_row = (center_y & 1) != 0;

    show_one_ptr(center_x, center_y);
    show_one_ptr(center_x - odd_row, center_y + 1);
    show_one_ptr(center_x - odd_row + 1, center_y + 1);
    show_one_ptr(center_x - 1, center_y + 2);
    show_one_ptr(center_x, center_y + 2);
    show_one_ptr(center_x + 1, center_y + 2);
    show_one_ptr(center_x - odd_row, center_y + 3);
    show_one_ptr(center_x - odd_row + 1, center_y + 3);
    show_one_ptr(center_x, center_y + 4);
}

// Draws a sixteen-tile isometric pointer footprint centered at (x, y).
// FUNCTION: C2 0x35e3f
// FUNCTION: C2WIN 0x004852e5
void four_by_four(int center_x, int center_y)
{
    int odd_row = (center_y & 1) != 0;

    show_one_ptr(center_x, center_y);
    show_one_ptr(center_x - odd_row, center_y + 1);
    show_one_ptr(center_x - odd_row + 1, center_y + 1);
    show_one_ptr(center_x - 1, center_y + 2);
    show_one_ptr(center_x, center_y + 2);
    show_one_ptr(center_x + 1, center_y + 2);
    show_one_ptr(center_x - odd_row - 1, center_y + 3);
    show_one_ptr(center_x - odd_row, center_y + 3);
    show_one_ptr(center_x - odd_row + 1, center_y + 3);
    show_one_ptr(center_x - odd_row + 2, center_y + 3);
    show_one_ptr(center_x - 1, center_y + 4);
    show_one_ptr(center_x, center_y + 4);
    show_one_ptr(center_x + 1, center_y + 4);
    show_one_ptr(center_x - odd_row, center_y + 5);
    show_one_ptr(center_x - odd_row + 1, center_y + 5);
    show_one_ptr(center_x, center_y + 6);
}

// Marks one map cell and draws its clipped pointer diamond at the current zoom level.
// FUNCTION: C2 0x35f0f
// FUNCTION: C2WIN 0x0048545c
void show_one_ptr(int screen_cell_x, int screen_cell_y)
{
#if PLATFORM_WINDOWS
    int cell_offset;
#else
    int map_cell_y;
    int map_cell_x;
    int cell_offset;
#endif

#if PLATFORM_WINDOWS
    if (screen_cell_y + pm_y < 0) return;
    if (screen_cell_y + pm_y >= 0xa1) return;
    if (screen_cell_x + pm_x < 0) return;
    if (screen_cell_x + pm_x >= 0x51) return;

    cell_offset = pseudo_map[screen_cell_y + pm_y][screen_cell_x + pm_x];
#else
    map_cell_y = pm_y + screen_cell_y;
    if (map_cell_y < 0 || map_cell_y >= 0xa1) return;
    map_cell_x = pm_x + screen_cell_x;
    if (map_cell_x < 0 || map_cell_x >= 0x51) return;

    cell_offset = pseudo_map[map_cell_y][map_cell_x];
#endif
    if (cell_offset >= 0x0FFF0000) return;

    if (map_mode == 0) {
        CM_CELL(cell_offset).edge_bits |= 1;
    } else if (map_mode == 1) {
        RM_CELL(cell_offset).edge_bits |= 1;
    }

    lib_para1 = pm_screen_x_start + pm_diamond_width * screen_cell_x;
    if (screen_cell_y & 1) lib_para1 -= pm_diamond_half_width;
    lib_para2 = pm_screen_y_start + pm_diamond_half_height * screen_cell_y;

    pm_x_edge = pm_y_edge = 0;
    if (screen_cell_y == 0) {
        pm_y_edge = 2;
    } else if (screen_cell_y < 0) {
        return;
    } else if (screen_cell_y == pm_screen_height) {
        pm_y_edge = 1;
    } else if (screen_cell_y > pm_screen_height) {
        return;
    }

    if (screen_cell_x < 0) return;
    if (screen_cell_y & 1) {
        if (screen_cell_x == 0) {
            pm_x_edge = 2;
        } else if (screen_cell_x == pm_screen_width) {
            pm_x_edge = 1;
        } else if (screen_cell_x > pm_screen_width) {
            return;
        }
    } else if (screen_cell_x >= pm_screen_width) {
        return;
    }

    if (zoom_level == 0) {
        if (pm_x_edge == 0)      write_i_large_diamond_ptr(15, pm_y_edge);
        else if (pm_x_edge == 2) write_i_large_diamond_ptr_left(15, pm_y_edge);
        else if (pm_x_edge == 1) write_i_large_diamond_ptr_right(15, pm_y_edge);
    } else if (zoom_level == 1) {
        if (pm_x_edge == 0)      write_i_medium_diamond_ptr(15, pm_y_edge);
        else if (pm_x_edge == 2) write_i_medium_diamond_ptr_left(15, pm_y_edge);
        else if (pm_x_edge == 1) write_i_medium_diamond_ptr_right(15, pm_y_edge);
    } else if (zoom_level == 2) {
        if (pm_x_edge == 0)      write_i_small_diamond_ptr(15, pm_y_edge);
        else if (pm_x_edge == 2) write_i_small_diamond_ptr_left(15, pm_y_edge);
        else if (pm_x_edge == 1) write_i_small_diamond_ptr_right(15, pm_y_edge);
    }
}

// Loads a fixture sprite and draws its full diamond at the current zoom level.
// FUNCTION: C2 0x36165
// FUNCTION: C2WIN 0x004857da
void place_diamond(int style)
{
#if PLATFORM_WINDOWS
    int mode;

    if (map_mode > 1) mode = 0;
    else mode = map_mode;
    if ((&fixt_data)[mode] == 0) {
        sprite_error++;
        return;
    }
#endif
    data_ptr = sprite_image_no * 16 + 8;
#if PLATFORM_WINDOWS
    sprite_start = ((&fixt_data)[mode][data_ptr + 5] << 8)
                 + ((&fixt_data)[mode][data_ptr + 6] << 16)
                 + (&fixt_data)[mode][data_ptr + 4];
#else
    sprite_start = *(fixt_data + data_ptr + 4)
                 + (*(fixt_data + data_ptr + 5) << 8)
                 + (*(fixt_data + data_ptr + 6) << 16);
#endif

    if (sprite_start > 0x4baf0) {
        sprite_error++;
        return;
    }
    if (sprite_start < 0) {
        sprite_error++;
        return;
    }
    if (zoom_level == 0) {
#if PLATFORM_WINDOWS
        place_i_large_diamond((&fixt_data)[mode], style);
#else
        place_i_large_diamond(fixt_data, style);
#endif
        return;
    }
    if (zoom_level == 1) {
#if PLATFORM_WINDOWS
        place_i_medium_diamond((&fixt_data)[mode], style);
#else
        place_i_medium_diamond(fixt_data, style);
#endif
        return;
    }
#if PLATFORM_WINDOWS
    place_i_small_diamond((&fixt_data)[mode], style);
#else
    place_i_small_diamond(fixt_data, style);
#endif
}

// Loads a fixture sprite and draws its left half at the current zoom level.
// FUNCTION: C2 0x361fd
// FUNCTION: C2WIN 0x0048592d
void place_lefthalf_diamond(void)
{
#if PLATFORM_WINDOWS
    int mode;

    if (map_mode > 1) mode = 0;
    else mode = map_mode;
#endif
    data_ptr = sprite_image_no * 16 + 8;
#if PLATFORM_WINDOWS
    sprite_start = (&fixt_data)[mode][data_ptr + 4]
                 + ((&fixt_data)[mode][data_ptr + 5] << 8)
                 + ((&fixt_data)[mode][data_ptr + 6] << 16);
#else
    sprite_start = *(fixt_data + data_ptr + 4)
                 + (*(fixt_data + data_ptr + 5) << 8)
                 + (*(fixt_data + data_ptr + 6) << 16);
#endif

    if (sprite_start > 0x4baf0) {
        sprite_error++;
        return;
    }
    if (sprite_start < 0) {
        sprite_error++;
        return;
    }
    if (zoom_level == 0) {
#if PLATFORM_WINDOWS
        place_i_large_diamond_lefthalf((&fixt_data)[mode], 0);
#else
        place_i_large_diamond_lefthalf(fixt_data, 0);
#endif
        return;
    }
    if (zoom_level == 1) {
#if PLATFORM_WINDOWS
        place_i_medium_diamond_lefthalf((&fixt_data)[mode], 0);
#else
        place_i_medium_diamond_lefthalf(fixt_data, 0);
#endif
        return;
    }
#if PLATFORM_WINDOWS
    place_i_small_diamond_lefthalf((&fixt_data)[mode], 0);
#else
    place_i_small_diamond_lefthalf(fixt_data, 0);
#endif
}

// Loads a fixture sprite and draws its right half at the current zoom level.
// FUNCTION: C2 0x36295
// FUNCTION: C2WIN 0x00485a5e
void place_righthalf_diamond(void)
{
#if PLATFORM_WINDOWS
    int mode;

    if (map_mode > 1) mode = 0;
    else mode = map_mode;
#endif
    data_ptr = sprite_image_no * 16 + 8;
#if PLATFORM_WINDOWS
    sprite_start = ((&fixt_data)[mode][data_ptr + 5] << 8)
                 + ((&fixt_data)[mode][data_ptr + 6] << 16)
                 + (&fixt_data)[mode][data_ptr + 4];
#else
    sprite_start = *(fixt_data + data_ptr + 4)
                 + (*(fixt_data + data_ptr + 5) << 8)
                 + (*(fixt_data + data_ptr + 6) << 16);
#endif

    if (sprite_start > 0x4baf0) {
        sprite_error++;
        return;
    }
    if (sprite_start < 0) {
        sprite_error++;
        return;
    }
    if (zoom_level == 0) {
#if PLATFORM_WINDOWS
        place_i_large_diamond_righthalf((&fixt_data)[mode], 0);
#else
        place_i_large_diamond_righthalf(fixt_data, 0);
#endif
        return;
    }
    if (zoom_level == 1) {
#if PLATFORM_WINDOWS
        place_i_medium_diamond_righthalf((&fixt_data)[mode], 0);
#else
        place_i_medium_diamond_righthalf(fixt_data, 0);
#endif
        return;
    }
#if PLATFORM_WINDOWS
    place_i_small_diamond_righthalf((&fixt_data)[mode], 0);
#else
    place_i_small_diamond_righthalf(fixt_data, 0);
#endif
}

// Loads a people sprite and draws its full overlay diamond at the current zoom level.
// FUNCTION: C2 0x3632d
// FUNCTION: C2WIN 0x00485b8f
void place_overlay(int style)
{
#if PLATFORM_WINDOWS
    int mode;

    if (map_mode > 1) mode = 0;
    else mode = map_mode;
#endif
    data_ptr = sprite_image_no * 16 + 8;
#if PLATFORM_WINDOWS
    sprite_start = (&people_data)[mode][data_ptr + 4]
                 + (((&people_data)[mode][data_ptr + 5] << 8)
                 + (((&people_data)[mode][data_ptr + 6] & 0xff) << 16));
#else
    sprite_start = *(people_data + data_ptr + 4)
                 + (*(people_data + data_ptr + 5) << 8)
                 + (*(people_data + data_ptr + 6) << 16);
#endif

    if (sprite_start > 0x4baf0) {
        sprite_error++;
        return;
    }
    if (sprite_start < 0) {
        sprite_error++;
        return;
    }
    if (zoom_level == 0) {
#if PLATFORM_WINDOWS
        place_i_large_diamond((&people_data)[mode], style);
#else
        place_i_large_diamond(people_data, style);
#endif
        return;
    }
    if (zoom_level == 1) {
#if PLATFORM_WINDOWS
        place_i_medium_diamond((&people_data)[mode], style);
#else
        place_i_medium_diamond(people_data, style);
#endif
        return;
    }
#if PLATFORM_WINDOWS
    place_i_small_diamond((&people_data)[mode], style);
#else
    place_i_small_diamond(people_data, style);
#endif
}

// Loads a people sprite and draws the left half of its overlay at the current zoom level.
// FUNCTION: C2 0x363c5
// FUNCTION: C2WIN 0x00485cc6
void place_lefthalf_overlay()
{
#if PLATFORM_WINDOWS
    int mode;

    if (map_mode > 1) mode = 0;
    else mode = map_mode;
#endif
    data_ptr = sprite_image_no * 16 + 8;
#if PLATFORM_WINDOWS
    sprite_start = (&people_data)[mode][data_ptr + 4]
                 + (((&people_data)[mode][data_ptr + 5] & 0xff) << 8)
                 + ((&people_data)[mode][data_ptr + 6] << 16);
#else
    sprite_start = *(people_data + data_ptr + 4)
                 + (*(people_data + data_ptr + 5) << 8)
                 + (*(people_data + data_ptr + 6) << 16);
#endif

    if (sprite_start > 0x4baf0) {
        sprite_error++;
        return;
    }
    if (sprite_start < 0) {
        sprite_error++;
        return;
    }
    if (zoom_level == 0) {
#if PLATFORM_WINDOWS
        place_i_large_diamond_lefthalf((&people_data)[mode], 0);
#else
        place_i_large_diamond_lefthalf(people_data, 0);
#endif
        return;
    }
    if (zoom_level == 1) {
#if PLATFORM_WINDOWS
        place_i_medium_diamond_lefthalf((&people_data)[mode], 0);
#else
        place_i_medium_diamond_lefthalf(people_data, 0);
#endif
        return;
    }
#if PLATFORM_WINDOWS
    place_i_small_diamond_lefthalf((&people_data)[mode], 0);
#else
    place_i_small_diamond_lefthalf(people_data, 0);
#endif
}

// Loads a people sprite and draws the right half of its overlay at the current zoom level.
// FUNCTION: C2 0x3645d
// FUNCTION: C2WIN 0x00485df7
void place_righthalf_overlay()
{
#if PLATFORM_WINDOWS
    int mode;

    if (map_mode > 1) mode = 0;
    else mode = map_mode;
#endif
    data_ptr = sprite_image_no * 16 + 8;
#if PLATFORM_WINDOWS
    sprite_start = (&people_data)[mode][data_ptr + 4]
                 + ((&people_data)[mode][data_ptr + 5] << 8)
                 + (((&people_data)[mode][data_ptr + 6] & 0xff) << 16);
#else
    sprite_start = *(people_data + data_ptr + 4)
                 + (*(people_data + data_ptr + 5) << 8)
                 + (*(people_data + data_ptr + 6) << 16);
#endif

    if (sprite_start > 0x4baf0) {
        sprite_error++;
        return;
    }
    if (sprite_start < 0) {
        sprite_error++;
        return;
    }
    if (zoom_level == 0) {
#if PLATFORM_WINDOWS
        place_i_large_diamond_righthalf((&people_data)[mode], 0);
#else
        place_i_large_diamond_righthalf(people_data, 0);
#endif
        return;
    }
    if (zoom_level == 1) {
#if PLATFORM_WINDOWS
        place_i_medium_diamond_righthalf((&people_data)[mode], 0);
#else
        place_i_medium_diamond_righthalf(people_data, 0);
#endif
        return;
    }
#if PLATFORM_WINDOWS
    place_i_small_diamond_righthalf((&people_data)[mode], 0);
#else
    place_i_small_diamond_righthalf(people_data, 0);
#endif
}
