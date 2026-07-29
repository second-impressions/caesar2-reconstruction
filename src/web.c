
#include "web.h"
#include "c2_data.h"

struct web_node web[120];
int web_nof_dircs;
int web_node_count;
int web_y;
int web_x;
int web_first_actual_node;
int web_total_length;
int web_out_of_the_walls;
int web_ptr;
int web_start_y;
int web_start_x;
int web_node;
unsigned char web_from;
unsigned char web_dirc;
unsigned char web_directions;
/* Forward declarations (functions defined later in this file). */
void init_web(void);
void put_new_node(void);
void set_first_nodes_values(int pressure_mask);
void push_nodes_values(char pressure, int source_nodes_only);
void push_node_value(char pressure_value);


// Starts a regional-road traversal and marks the source cell as visited.
// FUNCTION: C2 0x29a68
// FUNCTION: C2WIN 0x00450c50
int get_regroad_start_node(int x, int y)
{
    web_start_x = x;
    web_x = web_start_x;
    web_start_y = y;
    web_y = web_start_y;

    web_ptr = (web_y * REGION_W + web_x) * REGION_CELL_BYTES;

    if ((RM_CELL(web_ptr).terrain & 0x25) == 0) return 0;

    web_nof_dircs = get_web_regroad_dircs();
    web[0].kind     = 4;
    web[0].out_of_walls   = 0;
    web[0].x        = web_x;
    web[0].y        = web_y;
    web[0].dirs     = web_directions;
    RM_CELL(web_ptr).edge_bits |= 0x20;
    web_first_actual_node = 0;
    web_node_count = 1;
    web_total_length = 1;
    web_out_of_the_walls = 0;
    return 1;
}

// Traces every connected regional-road branch from the source cell.
// FUNCTION: C2 0x29b0b
// FUNCTION: C2WIN 0x00450d3b
int get_regroad_web(int x, int y)
{
    unsigned char pending_dirs;

    init_web();
    if (get_regroad_start_node(x, y) == 0) return 0;
    while (get_incomplete_node(0x3c, 8) != 0) {
        web_out_of_the_walls = web[web_node].out_of_walls;
        if (web_node_count >= 0x78) return 0;
        while (run_to_new_regroad_node() != 0) {
            pending_dirs = web[web_node].dirs ^ web[web_node].from_dir;
            if      (pending_dirs & 1) { web_dirc = 1; web_from = 4; }
            else if (pending_dirs & 2) { web_dirc = 2; web_from = 8; }
            else if (pending_dirs & 4) { web_dirc = 4; web_from = 1; }
            else if (pending_dirs & 8) { web_dirc = 8; web_from = 2; }
            else break;
            web[web_node].from_dir |= web_dirc;
        }
    }
    return 1;
}

// Follows one regional-road branch to a junction, dead end, or the starting cell.
// FUNCTION: C2 0x29c0a
// FUNCTION: C2WIN 0x00450e8c
int run_to_new_regroad_node(void)
{
    int steps;

    steps = 0;
    while (1) {
        steps++;
        if (steps >= 0x3e8) break;
        if      (web_dirc == 1) { web_y--; web_ptr -= 0x1e0; }
        else if (web_dirc == 2) { web_x++; web_ptr += 8;     }
        else if (web_dirc == 4) { web_y++; web_ptr += 0x1e0; }
        else if (web_dirc == 8) { web_x--; web_ptr -= 8;     }

        web_nof_dircs = get_web_regroad_dircs();
        if ((RM_CELL(web_ptr).edge_bits & 0x20) == 0) web_total_length++;
        if (RM_CELL(web_ptr).terrain & 2) web_out_of_the_walls = 1;
        RM_CELL(web_ptr).edge_bits |= 0x20;
        RM_CELL(web_ptr).outside &= 0xbf;
        if (web_out_of_the_walls != 0) RM_CELL(web_ptr).outside |= 0x40;

        if (web_nof_dircs > 2) {
            put_new_node();
            web[web_node].out_of_walls = web_out_of_the_walls;
            return 1;
        }
        if (web_x == web_start_x && web_y == web_start_y) {
            web[0].from_dir |= web_from;
            break;
        }
        if (web_nof_dircs <= 1) break;
        web_directions ^= web_from;
        if      (web_directions & 1) { web_dirc = 1; web_from = 4; }
        else if (web_directions & 2) { web_dirc = 2; web_from = 8; }
        else if (web_directions & 4) { web_dirc = 4; web_from = 1; }
        else if (web_directions & 8) { web_dirc = 8; web_from = 2; }
    }
    return 0;
}

// Collects the compass directions of regional roads adjoining the current cell.
// FUNCTION: C2 0x29ddc
// FUNCTION: C2WIN 0x004510fc
int get_web_regroad_dircs(void)
{
    int count = 0;
    web_directions = 0;
    if (web_y > 0) {
        if (RM_CELL(web_ptr - REGION_ROW).terrain & 0x25) { web_directions += 1; count++; }
    }
    if (web_x < 0x3b) {
        if (RM_CELL(web_ptr + REGION_CELL_BYTES).terrain   & 0x25) { web_directions += 2; count++; }
    }
    if (web_y < 0x3b) {
        if (RM_CELL(web_ptr + REGION_ROW).terrain & 0x25) { web_directions += 4; count++; }
    }
    if (web_x > 0) {
        if (RM_CELL(web_ptr - REGION_CELL_BYTES).terrain   & 0x25) { web_directions += 8; count++; }
    }
    return count;
}

// Starts a water-network traversal from an aqueduct or reservoir cell.
// FUNCTION: C2 0x29e8d
// FUNCTION: C2WIN 0x004511f2
int get_aqua_start_node(int x, int y)
{
    web_start_x = x;
    web_x       = web_start_x;
    web_start_y = y;
    web_y       = web_start_y;
    web_ptr     = (web_x + web_y * 80) * 20;
    if ((CM_CELL(web_ptr).terrain & 0xc0) == 0) return 0;
    web_nof_dircs = get_web_aqua_dircs();
    web[0].kind   = 4;
    web[0].x      = web_x;
    web[0].y      = web_y;
    web[0].dirs   = web_directions;
    CM_CELL(web_ptr).edge_bits |= 0x20;
    CM_CELL(web_ptr).range_flag &= 0xfc;
    CM_CELL(web_ptr).extra_edge = CM_CELL(web_ptr).building;
    if (CM_CELL(web_ptr).terrain & 0x80) web_first_actual_node = 0;
    else web_first_actual_node = 1;
    web_node_count   = 1;
    web_total_length = 1;
    return 1;
}

// Traces the connected water network and propagates pressure from river-fed reservoirs.
// FUNCTION: C2 0x29f4e
// FUNCTION: C2WIN 0x00451323
int get_aqua_web(int x, int y)
{
    unsigned char pending_dirs;

    init_web();
    if (get_aqua_start_node(x, y) == 0) return 0;
    while (get_incomplete_node(0x50, 0x14) != 0) {
        while (run_to_new_aqua_node() != 0) {
            pending_dirs = web[web_node].dirs ^ web[web_node].from_dir;
            if      (pending_dirs & 1) { web_dirc = 1; web_from = 4; }
            else if (pending_dirs & 2) { web_dirc = 2; web_from = 8; }
            else if (pending_dirs & 4) { web_dirc = 4; web_from = 1; }
            else if (pending_dirs & 8) { web_dirc = 8; web_from = 2; }
            else break;
            web[web_node].from_dir |= web_dirc;
        }
    }
    set_first_nodes_values(3);
    push_nodes_values(3, 1);
    push_nodes_values(3, 0);
    push_nodes_values(2, 0);
    return 1;
}

// Follows one aqueduct branch until it reaches a reservoir, dead end, or the starting cell.
// FUNCTION: C2 0x2a05a
// FUNCTION: C2WIN 0x0045147a
int run_to_new_aqua_node(void)
{
    int           steps;
    unsigned char unused_dx;

    steps = 0;
    while (1) {
        steps++;
        if (steps >= 0x3e8) break;
        if      (web_dirc == 1) { web_y--; web_ptr -= 0x640; }
        else if (web_dirc == 2) { web_x++; web_ptr += 0x14;  }
        else if (web_dirc == 4) { web_y++; web_ptr += 0x640; }
        else if (web_dirc == 8) { web_x--; web_ptr -= 0x14;  }
        web_nof_dircs = get_web_aqua_dircs();
        if ((CM_CELL(web_ptr).edge_bits & 0x20) == 0) web_total_length++;
        CM_CELL(web_ptr).edge_bits   |= 0x21;
        CM_CELL(web_ptr).range_flag &= 0xfc;
        CM_CELL(web_ptr).extra_edge    = CM_CELL(web_ptr).building;
        if (CM_CELL(web_ptr).terrain & 0x80) {
            put_new_node();
            return 1;
        }
        if (web_x == web_start_x && web_y == web_start_y) {
            web[0].from_dir |= web_from;
            break;
        }
        if (web_nof_dircs <= 1) break;
        web_directions ^= web_from;
        if      (web_directions & 1) { web_dirc = 1; web_from = 4; }
        else if (web_directions & 2) { web_dirc = 2; web_from = 8; }
        else if (web_directions & 4) { web_dirc = 4; web_from = 1; }
        else if (web_directions & 8) { web_dirc = 8; web_from = 2; }
    }
    return 0;
}

// Collects the compass directions of aqueducts or reservoirs adjoining the current cell.
// FUNCTION: C2 0x2a1f3
// FUNCTION: C2WIN 0x004516cb
int get_web_aqua_dircs(void)
{
    int count = 0;
    web_directions = 0;
    if (web_y > 0) {
        if (CM_CELL((web_ptr - 80 * CITY_CELL_BYTES)).terrain & 0xC0) {
            web_directions += 1; count++;
        }
    }
    if (web_x < 0x4f) {
        if (CM_CELL((web_ptr + 1 * CITY_CELL_BYTES)).terrain & 0xC0) {
            web_directions += 2; count++;
        }
    }
    if (web_y < 0x4f) {
        if (CM_CELL((web_ptr + 80 * CITY_CELL_BYTES)).terrain & 0xC0) {
            web_directions += 4; count++;
        }
    }
    if (web_x > 0) {
        if (CM_CELL((web_ptr - 1 * CITY_CELL_BYTES)).terrain & 0xC0) {
            web_directions += 8; count++;
        }
    }
    return count;
}

// Tests whether the current water-network node borders a river cell.
// FUNCTION: C2 0x2a2a4
// FUNCTION: C2WIN 0x004517c1
int test_next_to_river(void)
{
    if (web_y > 0) {
        if (CM_CELL((web_ptr - 80 * CITY_CELL_BYTES)).terrain & 0x18) return 1;
    }
    if (web_x < 0x4f) {
        if (CM_CELL((web_ptr + 1 * CITY_CELL_BYTES)).terrain & 0x18) return 1;
    }
    if (web_y < 0x4f) {
        if (CM_CELL((web_ptr + 80 * CITY_CELL_BYTES)).terrain & 0x18) return 1;
    }
    if (web_x > 0) {
        if (CM_CELL((web_ptr - 1 * CITY_CELL_BYTES)).terrain & 0x18) return 1;
    }
    return 0;
}

// Clears the temporary nodes used to trace a road or water network.
// FUNCTION: C2 0x2a321
// FUNCTION: C2WIN 0x00451887
void init_web(void)
{
    int i;
    for (i = 0; i < 120; i++) {
        web[i].kind = web[i].out_of_walls = web[i]._unused_writeonly02[0] = web[i]._unused_writeonly02[1] = 0;
        web[i].dirs = web[i].from_dir = 0;
        web[i].x    = web[i].y        = 0;
    }
}

// Adds the current junction to the web, or records another incoming branch for an existing node.
// FUNCTION: C2 0x2a368
// FUNCTION: C2WIN 0x00451938
void put_new_node(void)
{
    for (web_node = 0; web_node < web_node_count; web_node++) {
        if (web[web_node].kind == 0) break;
        if (web[web_node].x == web_x) {
            if (web[web_node].y == web_y) {
                web[web_node].from_dir |= web_from;
                return;
            }
        }
    }

    web_node_count++;
    web[web_node].kind = 4;
    web[web_node].x    = web_x;
    web[web_node].y    = web_y;
    web[web_node].dirs = web_directions;
    web[web_node].from_dir += web_from;
    return;
}

// Selects the first web node with an unexplored outgoing branch.
// FUNCTION: C2 0x2a418
// FUNCTION: C2WIN 0x00451a53
int get_incomplete_node(int row_stride, int cell_stride)
{
    unsigned char pending_dirs;

    for (web_node = 0; web_node < web_node_count; web_node++) {
        if (web[web_node].dirs == web[web_node].from_dir) continue;
        web_x = web[web_node].x;
        web_y = web[web_node].y;
        web_ptr = (row_stride * web_y + web_x) * cell_stride;
        pending_dirs = web[web_node].dirs ^ web[web_node].from_dir;
        if      (pending_dirs & 1) { web_dirc = 1; web_from = 4; }
        else if (pending_dirs & 2) { web_dirc = 2; web_from = 8; }
        else if (pending_dirs & 4) { web_dirc = 4; web_from = 1; }
        else if (pending_dirs & 8) { web_dirc = 8; web_from = 2; }
        web[web_node].from_dir |= web_dirc;
        return 1;
    }
    return 0;
}

// Marks river-adjacent network nodes as water sources and assigns their initial pressure.
// FUNCTION: C2 0x2a51f
// FUNCTION: C2WIN 0x00451bb9
void set_first_nodes_values(int pressure_mask)
{
    char pressure_mask_byte;
    char source_kind;

    pressure_mask_byte = pressure_mask;
    for (web_node = web_first_actual_node, source_kind = 5; web_node < web_node_count; web_node++) {
        web_x = web[web_node].x;
        web_y = web[web_node].y;
        web_ptr = (web_y * CITY_W + web_x) * CITY_CELL_BYTES;
        if (test_next_to_river()) {
            CM_CELL(web_ptr).range_flag |= pressure_mask_byte;
            web[web_node].kind = source_kind;
            CM_CELL(web_ptr).extra_edge += 3;
        }
    }
}

// Propagates a pressure value outward along every eligible branch of the water network.
// FUNCTION: C2 0x2a5b0
// FUNCTION: C2WIN 0x00451c83
void push_nodes_values(char pressure, int source_nodes_only)
{
    unsigned char directions;
    int  node_x;
    int  node_y;
    int  cell_offset;

    for (web_node = web_first_actual_node; web_node < web_node_count; web_node++) {
        if (source_nodes_only != 0) {
            if ((unsigned char)web[web_node].kind != 5) continue;
        }
        directions = web[web_node].dirs;
        node_x = web[web_node].x;
        node_y = web[web_node].y;
        cell_offset = (node_x + node_y * CITY_W) * CITY_CELL_BYTES;
        if ((char)(CM_CELL(cell_offset).range_flag & 3) != pressure) continue;
        if (directions & 1) {
            web_dirc = 1; web_from = 4;
            web_x = node_x; web_y = node_y; web_ptr = cell_offset;
            if (source_nodes_only == 0) push_node_value((char)(pressure - 1));
            else           push_node_value(pressure);
        }
        if (directions & 2) {
            web_dirc = 2; web_from = 8;
            web_x = node_x; web_y = node_y; web_ptr = cell_offset;
            if (source_nodes_only == 0) push_node_value((char)(pressure - 1));
            else           push_node_value(pressure);
        }
        if (directions & 4) {
            web_dirc = 4; web_from = 1;
            web_x = node_x; web_y = node_y; web_ptr = cell_offset;
            if (source_nodes_only == 0) push_node_value((char)(pressure - 1));
            else           push_node_value(pressure);
        }
        if (directions & 8) {
            web_dirc = 8; web_from = 2;
            web_x = node_x; web_y = node_y; web_ptr = cell_offset;
            if (source_nodes_only == 0) push_node_value((char)(pressure - 1));
            else           push_node_value(pressure);
        }
    }
}

// Propagates pressure along one water-network branch until it reaches a reservoir or stronger flow.
// FUNCTION: C2 0x2a74d
// FUNCTION: C2WIN 0x00451ebf
void push_node_value(char pressure_value)
{
    int steps;
    unsigned char current_pressure;
    int is_reservoir;

    steps = 0;
    while (1) {
        steps++;
        if (steps >= 0x3e8) return;
        if      (web_dirc == 1) { web_y--; web_ptr -= 0x640; }
        else if (web_dirc == 2) { web_x++; web_ptr += 0x14;  }
        else if (web_dirc == 4) { web_y++; web_ptr += 0x640; }
        else if (web_dirc == 8) { web_x--; web_ptr -= 0x14;  }
        web_nof_dircs = get_web_aqua_dircs();
        current_pressure = CM_CELL(web_ptr).range_flag & 3;
        is_reservoir = CM_CELL(web_ptr).terrain & 0x80;

        if (is_reservoir) {
            if ((char)current_pressure >= pressure_value) return;
            CM_CELL(web_ptr).range_flag |= pressure_value;
            if (pressure_value == 3) CM_CELL(web_ptr).extra_edge += 3;
            if (pressure_value == 2) CM_CELL(web_ptr).extra_edge += 2;
            if (pressure_value == 1) CM_CELL(web_ptr).extra_edge += 1;
            return;
        }
        if ((char)current_pressure >= pressure_value) return;
        CM_CELL(web_ptr).range_flag |= pressure_value;
        if ((CM_CELL(web_ptr).terrain & 2) == 0) {
            if (pressure_value == 3) CM_CELL(web_ptr).extra_edge += 2;
            else if (pressure_value >= 1) CM_CELL(web_ptr).extra_edge++;
        }
        web_directions ^= web_from;
        if      (web_directions & 1) { web_dirc = 1; web_from = 4; }
        else if (web_directions & 2) { web_dirc = 2; web_from = 8; }
        else if (web_directions & 4) { web_dirc = 4; web_from = 1; }
        else if (web_directions & 8) { web_dirc = 8; web_from = 2; }
        else return;
    }
}
