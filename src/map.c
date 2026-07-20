#include "c2_data.h"

/* Temporary coordinates used while tracing a route. */
struct byte_point_rec temp_route[16];

extern void copy(unsigned char *src, unsigned char *dst, int n);
/* Forward declarations (functions defined later in this file). */
void generate_cm_scrub(void);
void flesh_river_atoms(void);
void transform_road_elastic(int radius);
void transform_wall_elastic(int radius);
void transform_aquaduct_elastic(int radius);
void transform_reg_wall_elastic(int radius);
void plague_sized(int sptr, int size);
void clear_sized_to_rubble(int sptr, int size, int rubble_kind);
void clear_to_rubble(int sptr, int rubble_kind);
void clear_to_empty(int sptr);
void clear_basic(int sptr);
void clear_sized_to_reg_basic(int rm_offset, int size);
void clear_reg_basic(int rm_offset);
void plague_it(int sptr);
void unflag_rm_area(int x, int y, int size, unsigned char mask_byte);
void adjust_regions_coastline(int x, int y, int width, int height);
void test_citymap_neighbours_posedge(char mask);
void test_citymap_neighbours_negedge(char mask);
void test_type_citymap_neighbours_negedge(unsigned char type);
void test_regionmap_neighbours_posedge(char mask);
void test_regionmap_neighbours_negedge(char mask);
void test_type_regionmap_neighbours_negedge(unsigned char type);
void init_choices(struct choice_rec *arr, int count);
void invert_gmn(void);
void set_range(int x, int y, int range, unsigned char field_off, unsigned char value);
void set_rm_range(int x, int y, int half_width, char field_offset, char kind_byte);
void set_4_neighbours_if_not_wallortower(int x, int y, int sptr, unsigned char field_off, unsigned char value);
void set_2_neighbours_if_not_wallortower(int x, int y, int sptr, unsigned char field_off, unsigned char value, int north_south);
void set_4_neighbours_if_not_aquaductorresevoir(int x, int y, int sptr, unsigned char field_off, unsigned char value);
void set_2_neighbours_if_not_aquaductorresevoir(int x, int y, int sptr, unsigned char field_off, unsigned char value, int north_south);
void set_4_rm_neighbours_if_not_wallortower(int x, int y, int sptr, unsigned char field_off, unsigned char value);
void inc_elastic_by2(int x, int y, int sptr);
void restore_city_from_undo_buffer(void);
void restore_region_from_undo_buffer(void);
/* Callers before this point treat the return as implicit int;
   only the Windows build sees the prototype. */
#if PLATFORM_WINDOWS
void clear_all_cm(char layer);
#endif
void set_route_elastic_range(int radius);
void clear_city_flag(int val);
void clear_prov_flag(int val);
void count_danger_flags(void);


// Clear the city-map layers, seed scrub terrain, and generate the river layout.
// FUNCTION: C2 0x65f4c
// FUNCTION: C2WIN 0x0049f830
void generate_city_map_geography(void)
{
    int tries;

    tries = 5;
    clear_all_cm(2);
    clear_all_cm(1);
    clear_all_cm(3);
    clear_all_cm(9);
    clear_all_cm(0x10);
    clear_all_cm(tries);
    clear_all_cm(6);
    clear_all_cm(7);
    clear_all_cm(8);
    clear_all_cm(0xf);
    clear_all_cm(0xd);
    clear_all_cm(0xe);
    clear_all_cm(0xa);
    clear_all_cm(0xb);
    clear_all_cm(0xc);
    clear_all_cm(4);
    clear_all_cm(0x11);
    generate_cm_scrub();
    while (generate_cm_river() == 0) {
        generate_cm_scrub();
        tries--;
        if (tries == -1) break;
    }
}

// Trace a southward-biased random river across the city map, then assign its final tiles.
// FUNCTION: C2 0x66014
// FUNCTION: C2WIN 0x0049f923
int generate_cm_river(void)
{
    int current_dir;
    int previous_dir;
    int south_count;
    int budget;
    int candidate_dir;

    budget = 0x3c0;
    random();

    y = 0;
    x = 24;
    x += rand128 & 0x1f;
    cm_sptr = (x + y * 80) * 20;

    gmn_x    = x;
    gmn_y    = y;
    gmn_sptr = cm_sptr;

    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).terrain |= 0x10;
    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind  = 4;

    south_count = 0;
    previous_dir   = 0;
    current_dir    = 4;

    for (;;) {
        if (--budget == -1) break;

        if (current_dir == 0) {
            y--;  cm_sptr -= 0x640;
            (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).terrain |= 0x10;
            (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind  = current_dir;
        } else if (current_dir == 2) {
            x++;  cm_sptr += 0x14;
            (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).terrain |= 0x10;
            (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind  = current_dir;
        } else if (current_dir == 4) {
            y++;  cm_sptr += 0x640;
            (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).terrain |= 0x10;
            (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind  = current_dir;
        } else if (current_dir == 6) {
            x--;  cm_sptr -= 0x14;
            (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).terrain |= 0x10;
            (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind  = current_dir;
        }

        gmn_x    = x;
        gmn_y    = y;
        gmn_sptr = cm_sptr;

        if (x == 0 || y == 0 || x >= 0x4f || y >= 0x4f) break;

        random();
        candidate_dir = (rand128 & 3) * 2;

        if (candidate_dir == 0 && south_count < 4) {
            current_dir = 8;
            continue;
        }
        if (candidate_dir == previous_dir) {
            current_dir = 8;
            continue;
        }

        if (candidate_dir == 0) {
            gmn_y--; gmn_sptr -= 0x640;
        } else if (candidate_dir == 2) {
            gmn_x++; gmn_sptr += 0x14;
        } else if (candidate_dir == 4) {
            gmn_y++; gmn_sptr += 0x640;
        } else if (candidate_dir == 6) {
            gmn_x--; gmn_sptr -= 0x14;
        }

        test_citymap_neighbours_negedge(0x10);
        if (gmn_count > 2) {
            current_dir = 8;
            continue;
        }

        current_dir  = candidate_dir;
        previous_dir = (candidate_dir + 4) % 8;
        if (candidate_dir == 4) south_count++;
        if (candidate_dir == 0) south_count = 0;
    }

    if (budget != 0) {
        flesh_river_atoms();
    }
    return budget;
}

// Fill the city map with randomly selected scrub terrain.
// FUNCTION: C2 0x6623d
// FUNCTION: C2WIN 0x0049fc8b
void generate_cm_scrub(void)
{
    int row;
    int col;

    cm_sptr = 0;
    for (row = 0; row < 80; row++) {
        for (col = 0; col < 80; col++, cm_sptr += 20) {
            int terrain_variant;
            random();
            terrain_variant = rand128 & 0xf;
            (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind = (terrain_variant + 8);
        }
    }
}

// Replace every flagged river atom with the concrete river tile selected by its terrain pattern.
// FUNCTION: C2 0x66281
// FUNCTION: C2WIN 0x0049fd08
void flesh_river_atoms(void)
{
    unsigned char terrain;
    unsigned char variant;

    gmn_y    = 0;
    gmn_sptr = 0;

    for ( ; gmn_y < 0x50; gmn_y++) {
        gmn_x = 0;
        do {
            if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 0x10) {
                test_citymap_neighbours_posedge(0x10);
                terrain = (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind;
                variant = (unsigned char)choose_from(river_data, 6);

                if (variant != 0) {
                    if (terrain != choice_info) {
                        if (first_choice == 0x26) first_choice = 0x2a;
                        else if (first_choice == 0x1e) first_choice = 0x22;
                        else if (first_choice == 0x36) first_choice = 0x2e;
                        else if (first_choice == 0x46) first_choice = 0x42;
                        else if (first_choice == 0x3a) first_choice = 0x32;
                        else if (first_choice == 0x4a) first_choice = 0x3e;
                    }
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind = first_choice + choice_count;
                    if (variant > 2) (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain |= 8;
                }
            }

            gmn_x++;
            gmn_sptr += 0x14;
        } while (gmn_x < 0x50);
    }
}

// Expand a city construction path by one breadth-first search layer.
// FUNCTION: C2 0x663ab
// FUNCTION: C2WIN 0x0049febf
void test_elastic_range(int radius, unsigned char reject_mask)
{
    int x_min;
    int y_min;
    int needs_bounds;
    int x_span;
    int stride;
    int side;
    unsigned char neighbour_value;

    needs_bounds = 0;
    x_min = act_start_x - radius;
    y_min = act_start_y - radius;
    side = 2 * radius + 1;
    x_span = side;
    if (x_min <= 0) {
        x_span += x_min;
        x_min = 0;
        needs_bounds = 1;
    } else if (x_span + x_min > 0x50) {
        x_span -= x_span + x_min - 0x50;
        needs_bounds = 1;
    }
    if (y_min <= 0) {
        side += y_min;
        y_min = 0;
        needs_bounds = 1;
    } else if (side + y_min >= 0x50) {
        side -= side + y_min - 0x50;
        needs_bounds = 1;
    }

    gmn_sptr = ((x_min) + (y_min) * 80) * 20;
    stride = (0x50 - x_span) * 20;

    if (!needs_bounds) {
        gmn_y = y_min;
        for ( ; gmn_y < y_min + side; gmn_y++, gmn_sptr += stride) {
            gmn_x = x_min;
            for ( ; gmn_x < x_min + x_span; gmn_x++, gmn_sptr += 20) {
                if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & reject_mask) {
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = 0xff;
                    continue;
                }
                if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).citizen_a != 0) {
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = 0xff;
                    continue;
                }
                if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).citizen_b != 0) {
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = 0xff;
                    continue;
                }
                if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct != 0)
                    continue;
                neighbour_value = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) - CITY_ROW))).road_aqueduct;
                if (neighbour_value != 0 && neighbour_value < radius + 1) {
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = (unsigned char)(radius + 1);
                    continue;
                }
                neighbour_value = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) + CITY_CELL_BYTES))).road_aqueduct;
                if (neighbour_value != 0 && neighbour_value < radius + 1) {
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = (unsigned char)(radius + 1);
                    continue;
                }
                neighbour_value = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) + CITY_ROW))).road_aqueduct;
                if (neighbour_value != 0 && neighbour_value < radius + 1) {
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = (unsigned char)(radius + 1);
                    continue;
                }
                neighbour_value = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) - CITY_CELL_BYTES))).road_aqueduct;
                if (neighbour_value != 0 && neighbour_value < radius + 1) {
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = (unsigned char)(radius + 1);
                }
            }
        }
    } else {
        gmn_y = y_min;
        for ( ; gmn_y < y_min + side; gmn_y++, gmn_sptr += stride) {
            gmn_x = x_min;
            for ( ; gmn_x < x_min + x_span; gmn_x++, gmn_sptr += 20) {
                if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct != 0)
                    continue;
                if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & reject_mask) {
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = 0xff;
                    continue;
                }
                if (gmn_y > 0)
                    neighbour_value = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) - CITY_ROW))).road_aqueduct;
                else
                    neighbour_value = 0;
                if (neighbour_value != 0 && neighbour_value < radius + 1) {
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = (unsigned char)(radius + 1);
                    continue;
                }
                if (gmn_x < 0x4f)
                    neighbour_value = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) + CITY_CELL_BYTES))).road_aqueduct;
                else
                    neighbour_value = 0;
                if (neighbour_value != 0 && neighbour_value < radius + 1) {
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = (unsigned char)(radius + 1);
                    continue;
                }
                if (gmn_y < 0x4f)
                    neighbour_value = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) + CITY_ROW))).road_aqueduct;
                else
                    neighbour_value = 0;
                if (neighbour_value != 0 && neighbour_value < radius + 1) {
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = (unsigned char)(radius + 1);
                    continue;
                }
                if (gmn_x > 0)
                    neighbour_value = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) - CITY_CELL_BYTES))).road_aqueduct;
                else
                    neighbour_value = 0;
                if (neighbour_value != 0 && neighbour_value < radius + 1) {
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = (unsigned char)(radius + 1);
                }
            }
        }
    }
}

// Select the neighbouring city cell with the lowest nonzero path cost.
// FUNCTION: C2 0x666a4
// FUNCTION: C2WIN 0x004a03c4
void get_best_elastic_value(int x, int y, int cell_offset, int start_dir)
{
    int i;
    int dir = start_dir;
    unsigned char neighbour_value;

    best_elastic_value = 100;
    best_elastic_dirc = 0;
    i = 0;
    while (i++ < 4) {
        if (dir == 0) {
            if (y > 0) {
                neighbour_value = (*(struct city_cell *)((unsigned char *)city_map + ((cell_offset) - CITY_ROW))).road_aqueduct;
                if (neighbour_value != 0 && neighbour_value < best_elastic_value) {
                    best_elastic_value = neighbour_value;
                    best_elastic_dirc = 0;
                }
            }
        } else if (dir == 1) {
            if (x < 79) {
                neighbour_value = (*(struct city_cell *)((unsigned char *)city_map + ((cell_offset) + CITY_CELL_BYTES))).road_aqueduct;
                if (neighbour_value != 0 && neighbour_value < best_elastic_value) {
                    best_elastic_value = neighbour_value;
                    best_elastic_dirc = 1;
                }
            }
        } else if (dir == 2) {
            if (y < 79) {
                neighbour_value = (*(struct city_cell *)((unsigned char *)city_map + ((cell_offset) + CITY_ROW))).road_aqueduct;
                if (neighbour_value != 0 && neighbour_value < best_elastic_value) {
                    best_elastic_value = neighbour_value;
                    best_elastic_dirc = 2;
                }
            }
        } else if (dir == 3) {
            if (x > 0) {
                neighbour_value = (*(struct city_cell *)((unsigned char *)city_map + ((cell_offset) - CITY_CELL_BYTES))).road_aqueduct;
                if (neighbour_value != 0 && neighbour_value < best_elastic_value) {
                    best_elastic_value = neighbour_value;
                    best_elastic_dirc = 3;
                }
            }
        }
        dir++;
        if (dir > 3) dir = 0;
    }
}

// Expand a regional construction path by one breadth-first search layer.
// FUNCTION: C2 0x6675a
// FUNCTION: C2WIN 0x004a0576
void test_rm_elastic_range(int strict, int radius, unsigned char reject_mask)
{
    int x_min;
    int y_min;
    int needs_bounds;
    int x_span;
    int stride;
    int side;
    unsigned char neighbour_value;

    needs_bounds = 0;
    x_min = act_start_x - radius;
    y_min = act_start_y - radius;
    side  = 2 * radius + 1;
    x_span = side;
    if (x_min <= 0) {
        x_span += x_min;
        x_min = 0;
        needs_bounds = 1;
    } else if (x_min + side > 0x3c) {
        x_span -= x_min + side - 0x3c;
        needs_bounds = 1;
    }
    if (y_min <= 0) {
        side += y_min;
        y_min = 0;
        needs_bounds = 1;
    } else if (y_min + side >= 0x3c) {
        side -= y_min + side - 0x3c;
        needs_bounds = 1;
    }

    gmn_sptr = ((x_min) + (y_min) * 60) * 8;
    stride   = (0x3c - x_span) * 8;

    if (!needs_bounds) {
        gmn_y = y_min;
        for ( ; gmn_y < y_min + side; gmn_y++, gmn_sptr += stride) {
            gmn_x = x_min;
            for ( ; gmn_x < x_min + x_span; gmn_x++, gmn_sptr += 8) {
                if ((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain & reject_mask) {
                    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state = 0xff;
                    continue;
                }
                if ((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).occupant != 0) {
                    if (strict == 0) {
                        (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state = 0xff;
                        continue;
                    }
                    if ((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain & 0x17) {
                        (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state = 0xff;
                        continue;
                    }
                }
                if ((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state != 0)
                    continue;
                neighbour_value = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr - 480))).place_state;
                if (neighbour_value != 0 && neighbour_value < radius + 1) {
                    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state = (unsigned char)(radius + 1);
                    continue;
                }
                neighbour_value = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr + 8))).place_state;
                if (neighbour_value != 0 && neighbour_value < radius + 1) {
                    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state = (unsigned char)(radius + 1);
                    continue;
                }
                neighbour_value = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr + 480))).place_state;
                if (neighbour_value != 0 && neighbour_value < radius + 1) {
                    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state = (unsigned char)(radius + 1);
                    continue;
                }
                neighbour_value = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr - 8))).place_state;
                if (neighbour_value != 0 && neighbour_value < radius + 1) {
                    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state = (unsigned char)(radius + 1);
                }
            }
        }
    } else {
        gmn_y = y_min;
        for ( ; gmn_y < y_min + side; gmn_y++, gmn_sptr += stride) {
            gmn_x = x_min;
            for ( ; gmn_x < x_min + x_span; gmn_x++, gmn_sptr += 8) {
                if ((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain & reject_mask) {
                    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state = 0xff;
                    continue;
                }
                if ((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state != 0)
                    continue;
                if (gmn_y > 0)
                    neighbour_value = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr - 480))).place_state;
                else
                    neighbour_value = 0;
                if (neighbour_value != 0 && neighbour_value < radius + 1) {
                    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state = (unsigned char)(radius + 1);
                    continue;
                }
                if (gmn_x < 0x3b)
                    neighbour_value = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr + 8))).place_state;
                else
                    neighbour_value = 0;
                if (neighbour_value != 0 && neighbour_value < radius + 1) {
                    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state = (unsigned char)(radius + 1);
                    continue;
                }
                if (gmn_y < 0x3b)
                    neighbour_value = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr + 480))).place_state;
                else
                    neighbour_value = 0;
                if (neighbour_value != 0 && neighbour_value < radius + 1) {
                    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state = (unsigned char)(radius + 1);
                    continue;
                }
                if (gmn_x > 0)
                    neighbour_value = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr - 8))).place_state;
                else
                    neighbour_value = 0;
                if (neighbour_value != 0 && neighbour_value < radius + 1) {
                    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state = (unsigned char)(radius + 1);
                }
            }
        }
    }
}

// Select the neighbouring region cell with the lowest usable path cost.
// FUNCTION: C2 0x66a53
// FUNCTION: C2WIN 0x004a0a83
void get_best_rm_elastic_value(int x, int y, int cell_offset, int start_dir)
{
    int i;
    int dir = start_dir;
    unsigned char neighbour_value;

    best_elastic_value = 100;
    best_elastic_dirc = 0;
    i = 0;
    while (i++ < 4) {
        if (dir == 0) {
            if (y > 0) {
                neighbour_value = (*(struct region_cell *)((unsigned char *)region_map + (cell_offset - 480))).place_state;
                if (neighbour_value != 0 && neighbour_value < best_elastic_value) {
                    best_elastic_value = neighbour_value;
                    best_elastic_dirc = 0;
                }
            }
        } else if (dir == 1) {
            if (x < 59) {
                neighbour_value = (*(struct region_cell *)((unsigned char *)region_map + (cell_offset + 8))).place_state;
                if (neighbour_value != 0 && neighbour_value < best_elastic_value) {
                    best_elastic_value = neighbour_value;
                    best_elastic_dirc = 1;
                }
            }
        } else if (dir == 2) {
            if (y < 59) {
                neighbour_value = (*(struct region_cell *)((unsigned char *)region_map + (cell_offset + 480))).place_state;
                if (neighbour_value != 0 && neighbour_value < best_elastic_value) {
                    best_elastic_value = neighbour_value;
                    best_elastic_dirc = 2;
                }
            }
        } else if (dir == 3) {
            if (x > 0) {
                neighbour_value = (*(struct region_cell *)((unsigned char *)region_map + (cell_offset - 8))).place_state;
                if (neighbour_value != 0 && neighbour_value < best_elastic_value) {
                    best_elastic_value = neighbour_value;
                    best_elastic_dirc = 3;
                }
            }
        }
        dir++;
        if (dir > 3) dir = 0;
    }
}

// Grow and transform the elastic road preview from the current construction start cell.
// FUNCTION: C2 0x66b05
// FUNCTION: C2WIN 0x004a0c35
void get_road_elastic(void)
{
    int i;

    set_range(act_start_x, act_start_y, 0x15, 2, 0);
    (*(struct city_cell *)((unsigned char *)city_map + (act_start_pm_ptr))).road_aqueduct = 1;
    for (i = 1; i <= 20; i++)
        test_elastic_range(i, 9);
    transform_road_elastic(20);
}

// Mark legal road candidates near the current city construction start.
// FUNCTION: C2 0x66b56
// FUNCTION: C2WIN 0x004a0c9f
void transform_road_elastic(int radius)
{
    int x_min;
    int y_min;
    int x_max;
    int y_max;
    int side;
    int x_span;
    int stride;

    x_min = act_start_x - radius;
    y_min = act_start_y - radius;
    side  = 2 * radius + 1;
    x_span = side;
    x_max = side + x_min;
    if (x_min <= 0)       { x_span = x_max; x_min = 0; }
    else if (x_max > 80)  { x_span -= x_max - 80; }
    y_max = side + y_min;
    if (y_min <= 0)       { side = y_max; y_min = 0; }
    else if (y_max >= 80) { side -= y_max - 80; }

    gmn_sptr = (x_min + y_min * 80) * 20;
    stride   = (80 - x_span) * 20;

    gmn_y = y_min;
    for ( ; gmn_y < y_min + side; gmn_y++, gmn_sptr += stride) {
        gmn_x = x_min;
        for ( ; gmn_x < x_min + x_span; gmn_x++, gmn_sptr += 20) {
            if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct != 0xff) {
                if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 0x02) {
                    if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind <= 0xc2
                        || (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind >= 0xc7) continue;
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = 0xff;
                }
                if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 0x04) {
                    if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 0x20) continue;
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = 0xff;
                }
                if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 0x80) {
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = 0xff;
                }
                if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 0x40) {
                    if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind < 0xd1) continue;
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = 0xff;
                }
                if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind < 8
                    && ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits & 0x80)) {
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = 0xff;
                }
            }
        }
    }
}

// Build the city road represented by the current elastic path.
// FUNCTION: C2 0x66d22
// FUNCTION: C2WIN 0x004a0f4e
void build_road_from_elastic(void)
{
    int over_y_l;
    int over_x_l;
    int cell_ptr;
    int count;
    unsigned char saved_slot;
    unsigned char status;
    int attempt;
    int dir;
    unsigned char neighbour;
    int bridge_count;

    count = (*(struct city_cell *)((unsigned char *)city_map + (pm_over_cm_ptr))).road_aqueduct;
    if (count == 0) { illegal_build = 1; return; }
    if (count == 0xff) { illegal_build = 1; return; }

    over_x_l = over_x;
    over_y_l = over_y;
    cell_ptr = pm_over_cm_ptr;
    dir      = 0;
    bridge_count = 0;
    status   = 0;

    while (count > 0) {
        count--;
        if ((*(struct city_cell *)((unsigned char *)city_map + (cell_ptr))).terrain & 0x02) {
            (*(struct city_cell *)((unsigned char *)city_map + (cell_ptr))).terrain &= 0xf9;
            (*(struct city_cell *)((unsigned char *)city_map + (cell_ptr))).terrain |= 0x04;
        }
        if (!((*(struct city_cell *)((unsigned char *)city_map + (cell_ptr))).terrain & 0x20)) particles_built++;
        if ((*(struct city_cell *)((unsigned char *)city_map + (cell_ptr))).base_kind < 0x1a) particles_cleared++;
        (*(struct city_cell *)((unsigned char *)city_map + (cell_ptr))).terrain |= 0x20;
        (*(struct city_cell *)((unsigned char *)city_map + (cell_ptr))).edge_bits |= 1;

        saved_slot = (*(struct city_cell *)((unsigned char *)city_map + (cell_ptr))).road_aqueduct;
        attempt = 4; dir = 0;
        while (attempt-- > 0) {
            neighbour    = 0;
            if ((*(struct city_cell *)((unsigned char *)city_map + (cell_ptr))).terrain & 0x10) bridge_count = 1; else bridge_count = 0;
            if (++dir > 3) dir = 0;
            if (dir == 0) {
                if (over_y_l > 0) neighbour = (*(struct city_cell *)((unsigned char *)city_map + ((cell_ptr) - 0x640))).road_aqueduct;
                if ((*(struct city_cell *)((unsigned char *)city_map + ((cell_ptr) - 0x640))).terrain & 0x10) bridge_count++;
                if (bridge_count > 1) neighbour = 0;
                if (neighbour != 0 && neighbour < saved_slot) { cell_ptr -= 0x640; over_y_l--; break; }
            } else if (dir == 1) {
                if (over_x_l < 0x4f) neighbour = (*(struct city_cell *)((unsigned char *)city_map + ((cell_ptr) + 0x14))).road_aqueduct;
                if ((*(struct city_cell *)((unsigned char *)city_map + ((cell_ptr) + 0x14))).terrain & 0x10) bridge_count++;
                if (bridge_count > 1) neighbour = 0;
                if (neighbour != 0 && neighbour < saved_slot) { cell_ptr += 0x14; over_x_l++; break; }
            } else if (dir == 2) {
                if (over_y_l < 0x4f) neighbour = (*(struct city_cell *)((unsigned char *)city_map + ((cell_ptr) + 0x640))).road_aqueduct;
                if ((*(struct city_cell *)((unsigned char *)city_map + ((cell_ptr) + 0x640))).terrain & 0x10) bridge_count++;
                if (bridge_count > 1) neighbour = 0;
                if (neighbour != 0 && neighbour < saved_slot) { cell_ptr += 0x640; over_y_l++; break; }
            } else if (dir == 3) {
                if (over_x_l > 0) neighbour = (*(struct city_cell *)((unsigned char *)city_map + ((cell_ptr) - 0x14))).road_aqueduct;
                if ((*(struct city_cell *)((unsigned char *)city_map + ((cell_ptr) - 0x14))).terrain & 0x10) bridge_count++;
                if (bridge_count > 1) neighbour = 0;
                if (neighbour != 0 && neighbour < saved_slot) { cell_ptr -= 0x14; over_x_l--; break; }
            }
        }
        if (neighbour != 0 && neighbour < saved_slot)
            continue;
        if (saved_slot > 1) { status = 1; goto finish; }
        goto phase2;
    }

phase2:
    count    = (*(struct city_cell *)((unsigned char *)city_map + (pm_over_cm_ptr))).road_aqueduct;
    over_x_l = over_x;
    over_y_l = over_y;
    cell_ptr = pm_over_cm_ptr;
    dir      = 0;
    bridge_count = 0;

    while (count > 0) {
        count--;
        if (road_ramifications(over_x_l, over_y_l) == 0) {
            status = 2; goto finish;
        }
        saved_slot = (*(struct city_cell *)((unsigned char *)city_map + (cell_ptr))).road_aqueduct;
        attempt = 4; dir = 0;
        while (attempt-- > 0) {
            neighbour    = 0;
            if ((*(struct city_cell *)((unsigned char *)city_map + (cell_ptr))).terrain & 0x10) bridge_count = 1; else bridge_count = 0;
            if (++dir > 3) dir = 0;
            if (dir == 0) {
                if (over_y_l > 0) neighbour = (*(struct city_cell *)((unsigned char *)city_map + ((cell_ptr) - 0x640))).road_aqueduct;
                if ((*(struct city_cell *)((unsigned char *)city_map + ((cell_ptr) - 0x640))).terrain & 0x10) bridge_count++;
                if (bridge_count > 1) neighbour = 0;
                if (neighbour != 0 && neighbour < saved_slot) { cell_ptr -= 0x640; over_y_l--; break; }
            } else if (dir == 1) {
                if (over_x_l < 0x4f) neighbour = (*(struct city_cell *)((unsigned char *)city_map + ((cell_ptr) + 0x14))).road_aqueduct;
                if ((*(struct city_cell *)((unsigned char *)city_map + ((cell_ptr) + 0x14))).terrain & 0x10) bridge_count++;
                if (bridge_count > 1) neighbour = 0;
                if (neighbour != 0 && neighbour < saved_slot) { cell_ptr += 0x14; over_x_l++; break; }
            } else if (dir == 2) {
                if (over_y_l < 0x4f) neighbour = (*(struct city_cell *)((unsigned char *)city_map + ((cell_ptr) + 0x640))).road_aqueduct;
                if ((*(struct city_cell *)((unsigned char *)city_map + ((cell_ptr) + 0x640))).terrain & 0x10) bridge_count++;
                if (bridge_count > 1) neighbour = 0;
                if (neighbour != 0 && neighbour < saved_slot) { cell_ptr += 0x640; over_y_l++; break; }
            } else if (dir == 3) {
                if (over_x_l > 0) neighbour = (*(struct city_cell *)((unsigned char *)city_map + ((cell_ptr) - 0x14))).road_aqueduct;
                if ((*(struct city_cell *)((unsigned char *)city_map + ((cell_ptr) - 0x14))).terrain & 0x10) bridge_count++;
                if (bridge_count > 1) neighbour = 0;
                if (neighbour != 0 && neighbour < saved_slot) { cell_ptr -= 0x14; over_x_l--; break; }
            }
        }
        if (neighbour != 0 && neighbour < saved_slot)
            continue;
        if (saved_slot > 1) { status = 3; goto finish; }
        break;
    }

finish:
    if (status != 0) {
        illegal_build = 1;
        restore_city_from_undo_buffer();
    }
}

// Recompute road sprites in the clamped 3x3 neighbourhood around (x,y).
// FUNCTION: C2 0x67109
// FUNCTION: C2WIN 0x004a1612
int road_ramifications(int x, int y)
{
    int x_min;
    char kind;
    int x_max;
    int y_max;
    int y_min;

    x_min = (x == 0) ? 0 : x - 1;
    y_min = (y == 0) ? 0 : y - 1;
    x_max = (x == 79) ? 79 : x + 1;
    y_max = (y == 79) ? 79 : y + 1;

    for (gmn_y = y_min; y_max >= gmn_y; gmn_y++) {
        for (gmn_x = x_min; x_max >= gmn_x; gmn_x++) {
            gmn_sptr = ((gmn_x) + (gmn_y) * 80) * 20;
            if (((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 0x20) == 0) continue;
            if (((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 8) != 0) continue;
            (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits |= 1;
            if (((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 0x10) != 0) {
                if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).building != 0) continue;
                kind = (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind;
                (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).building = kind;
                if (kind >= 0x1e && kind < 0x22)
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind = 0x4e;
                else if (kind >= 0x22 && kind < 0x26)
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind = 0x4f;
                else if (kind >= 0x26 && kind < 0x2a)
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind = 0x50;
                else if (kind >= 0x2a && kind < 0x2e)
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind = 0x51;
                continue;
            }
            if (((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 4) != 0) {
                if (one_wall_ramification() != 0) continue;
                (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain &= 0xf9;
                (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain |= 2;
                return 0;
            }
            if (((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 0x40) != 0) {
                if (one_aquaduct_ramification() == 0) {
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain &= 0xdf;
                    return 0;
                }
                (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits |= 0x80;
                continue;
            }
            if (((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind & 0xff) >= 0x7c) continue;
            test_citymap_neighbours_posedge(0x20);
            if (choose_from(road_data, 0x10) == 0) {
                (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain &= 0xdf;
                return 0;
            }
            (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind = first_choice;
        }
    }
    return 1;
}

// Grow and transform the elastic wall preview from the current construction start cell.
// FUNCTION: C2 0x67333
// FUNCTION: C2WIN 0x004a1998
void get_wall_elastic(void)
{
    int i;

    set_range(act_start_x, act_start_y, 0x15, 2, 0);
    (*(struct city_cell *)((unsigned char *)city_map + (act_start_pm_ptr))).road_aqueduct = 1;
    for (i = 1; i <= 20; i++)
        test_elastic_range(i, 0x99);
    transform_wall_elastic(0x14);
    if ((*(struct city_cell *)((unsigned char *)city_map + (act_start_pm_ptr))).road_aqueduct != 0xff)
        (*(struct city_cell *)((unsigned char *)city_map + (act_start_pm_ptr))).road_aqueduct = 1;
    elastic_start_dirc = 0;
}

// Mark legal wall candidates near the current city construction start.
// FUNCTION: C2 0x673a8
// FUNCTION: C2WIN 0x004a1a34
void transform_wall_elastic(int radius)
{
    int x_min;
    int y_min;
    unsigned char kind;
    int stride;
    int x_span;
    int side;
    int needs_bounds;

    needs_bounds = 0;
    x_min = act_start_x - radius;
    y_min = act_start_y - radius;
    side = 2 * radius + 1;
    x_span = side;
    if (x_min <= 0)                  { x_span += x_min; x_min = 0; needs_bounds = 1; }
    else if (x_min + side > 80)      { x_span -= x_min + side - 80; needs_bounds = 1; }
    if (y_min <= 0)                  { side += y_min; y_min = 0; needs_bounds = 1; }
    else if (y_min + side >= 80)     { side -= y_min + side - 80; needs_bounds = 1; }

    gmn_sptr = (x_min + y_min * 80) * 20;
    stride   = (80 - x_span) * 20;

    for (gmn_y = y_min; y_min + side > gmn_y; gmn_y++, gmn_sptr += stride) {
        for (gmn_x = x_min; x_min + x_span > gmn_x; gmn_x++, gmn_sptr += 20) {
            if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct != 0xff) {
                if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 0x02) {
                    kind = (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind;
                    if (!((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 0x40)) {
                        if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind == 0xc1) {
                            set_2_neighbours_if_not_wallortower(gmn_x, gmn_y, gmn_sptr, 2, 0xff, 0);
                            (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = 0xff;
                        } else if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind == 0xc2) {
                            set_2_neighbours_if_not_wallortower(gmn_x, gmn_y, gmn_sptr, 2, 0xff, 1);
                            (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = 0xff;
                        } else if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind < 0xc7) {
                            set_4_neighbours_if_not_wallortower(gmn_x, gmn_y, gmn_sptr, 2, 0xff);
                            (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = 0xff;
                        } else if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind <= 0xca) {
                            if (gmn_x == act_start_x && gmn_y == act_start_y) continue;
                            inc_elastic_by2(gmn_x, gmn_y, gmn_sptr);
                            (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct += 2;
                        }
                    }
                } else if (((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 0x04) && (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct > 1 && (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct != 0xff) {
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct -= 1;
                }

                if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 0x20) {
                    if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 0x04) continue;
                    if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind == 0x52) (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct += 1;
                    else if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind == 0x53) (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct += 1;
                    else (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = 0xff;
                }
                if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 0x40) {
                    if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind <= 0xd0) (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct += 1;
                    else                                   (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = 0xff;
                }
                if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind < 8 && ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits & 0x80)) {
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = 0xff;
                }
            }
        }
    }
}

// Build the city wall represented by the current elastic path and update its connections.
// FUNCTION: C2 0x67653
// FUNCTION: C2WIN 0x004a1ee0
void build_wall_from_elastic(void)
{
    int size;
    int y;
    int x;
    int pm_ptr;

    size = (unsigned char)(*(struct city_cell *)((unsigned char *)city_map + (pm_over_cm_ptr))).road_aqueduct;
    if ((*(struct city_cell *)((unsigned char *)city_map + (pm_over_cm_ptr))).terrain & 4)
        size++;
    if (size == 0) {
        illegal_build = 1;
    } else if (size == 0xff) {
        illegal_build = 1;
    } else {
        unsigned char status;
        unsigned char saved_byte2;

        status = 0;
        x = over_x;
        y = over_y;
        pm_ptr = pm_over_cm_ptr;
        while (size > 0) {
            size--;
            if (((*(struct city_cell *)((unsigned char *)city_map + (pm_ptr))).terrain & 6) == 0)
                particles_built++;
            if ((unsigned char)(*(struct city_cell *)((unsigned char *)city_map + (pm_ptr))).base_kind < 0x1a)
                particles_cleared++;
            (*(struct city_cell *)((unsigned char *)city_map + (pm_ptr))).edge_bits |= 1;
            if (!((*(struct city_cell *)((unsigned char *)city_map + (pm_ptr))).terrain & 4)) {
                if ((*(struct city_cell *)((unsigned char *)city_map + (pm_ptr))).terrain & 0x20)
                    (*(struct city_cell *)((unsigned char *)city_map + (pm_ptr))).terrain |= 4;
                else
                    (*(struct city_cell *)((unsigned char *)city_map + (pm_ptr))).terrain |= 2;
            }
            saved_byte2 = (*(struct city_cell *)((unsigned char *)city_map + (pm_ptr))).road_aqueduct;
            get_best_elastic_value(x, y, pm_ptr, elastic_start_dirc);
            if (saved_byte2 >= best_elastic_value) {
                if (best_elastic_dirc == 0) { pm_ptr -= 0x640; y--; }
                else if (best_elastic_dirc == 1) { pm_ptr += 0x14;  x++; }
                else if (best_elastic_dirc == 2) { pm_ptr += 0x640; y++; }
                else if (best_elastic_dirc == 3) { pm_ptr -= 0x14;  x--; }
                continue;
            }
            if (saved_byte2 > 1) {
                status = 1;
                goto check_outer_state;
            }
            break;
        }

        size = (unsigned char)(*(struct city_cell *)((unsigned char *)city_map + (pm_over_cm_ptr))).road_aqueduct;
        if ((*(struct city_cell *)((unsigned char *)city_map + (pm_over_cm_ptr))).terrain & 4)
            size++;
        x = over_x;
        y = over_y;
        pm_ptr = pm_over_cm_ptr;
        while (size > 0) {
            size--;
            if (!wall_ramifications(x, y)) {
                status = 2;
                goto check_outer_state;
            }
            (*(struct city_cell *)((unsigned char *)city_map + (pm_ptr))).industrial = 0;
            saved_byte2 = (*(struct city_cell *)((unsigned char *)city_map + (pm_ptr))).road_aqueduct;
            get_best_elastic_value(x, y, pm_ptr, elastic_start_dirc);
            if (saved_byte2 >= best_elastic_value) {
                if (best_elastic_dirc == 0) { pm_ptr -= 0x640; y--; }
                else if (best_elastic_dirc == 1) { pm_ptr += 0x14;  x++; }
                else if (best_elastic_dirc == 2) { pm_ptr += 0x640; y++; }
                else if (best_elastic_dirc == 3) { pm_ptr -= 0x14;  x--; }
                continue;
            }
            if (saved_byte2 > 1) {
                status = 3;
                goto check_outer_state;
            }
            break;
        }

    check_outer_state:
        if (status != 0) {
            illegal_build = 1;
            restore_city_from_undo_buffer();
            elastic_start_dirc++;
            if (elastic_start_dirc > 3)
                elastic_start_dirc = 0;
        }
    }
}

// Validate wall connections at (x, y) and across the 3×3 (or smaller, if at edges) neighbourhood.
// FUNCTION: C2 0x678bb
// FUNCTION: C2WIN 0x004a2251
int wall_ramifications(int x, int y)
{
    int x_min;
    int y_min;
    int x_max;
    int y_max;

    if (x == 0) x_min = 0; else x_min = x - 1;
    if (y == 0) y_min = 0; else y_min = y - 1;
    if (x == 79) x_max = x; else x_max = x + 1;
    if (y == 79) y_max = y; else y_max = y + 1;

    gmn_x = x;
    gmn_y = y;
    if (one_wall_ramification() == 0)
        return 0;

    for (gmn_y = y_min; y_max >= gmn_y; gmn_y++) {
        for (gmn_x = x_min; x_max >= gmn_x; gmn_x++) {
            if (one_wall_ramification() == 0)
                return 0;
        }
    }

    return 1;
}

// Choose the wall sprite matching the current cell's neighbouring walls.
// FUNCTION: C2 0x67944
// FUNCTION: C2WIN 0x004a2362
int one_wall_ramification(void)
{
    unsigned char sprite;

    gmn_sptr = ((gmn_x) + (gmn_y) * 80) * 20;
    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits |= 1;

    if (((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 2) != 0) {
        test_citymap_neighbours_negedge(6);
        if (((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 0x40) != 0) {
            if (gmn_polar_count == 0) {
                if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind == 0xcf) first_choice = 0xbc;
                else if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind == 0xd0) first_choice = 0xbd;
                else return 0;
                (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind = first_choice;
                (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits &= 0xe3;
                (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits |= 8;
                if (first_choice == 0xbc) sprite = 3;
                else sprite = 7;
                (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).extra_edge = sprite;
                (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).building = sprite;
                return 1;
            } else {
                if (choose_from(wallaqua_data, 2) != 0) {
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind = first_choice;
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits &= 0xe3;
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits |= 8;
                    if (first_choice == 0xbc) sprite = 3;
                    else sprite = 7;
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).extra_edge = sprite;
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).building = sprite;
                    return 1;
                }
                gmn_err_sptr = gmn_sptr;
                gmn_err_x = gmn_x;
                gmn_err_y = gmn_y;
                return 0;
            }
        }
        if (choose_from(wall_data, 0xe) == 0) {
            gmn_err_sptr = gmn_sptr;
            gmn_err_x = gmn_x;
            gmn_err_y = gmn_y;
            return 0;
        }
        (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind = first_choice;
        (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits &= 0xe3;
        (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits |= wall_gfxdat[first_choice - WALL_GFX_FIRST_TILE].edge_bits;
        (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).extra_edge = wall_gfxdat[first_choice - WALL_GFX_FIRST_TILE].sprite;
        return 1;
    }

    if (((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 4) != 0) {
        if (((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 0x20) != 0) {
            test_type_citymap_neighbours_negedge(0xc0);
            if (gmn_polar_count != 0) {
                gmn_err_sptr = gmn_sptr;
                gmn_err_x = gmn_x;
                gmn_err_y = gmn_y;
                return 0;
            }
            test_citymap_neighbours_negedge(6);
            if (choose_from(gateway_data, 6) != 0) {
                (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).extra_edge = first_choice;
                (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind = 0xc0;
                (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits &= 0x63;
                (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits |= 0x88;
                return 1;
            }
            test_citymap_neighbours_negedge(0x20);
            if (choose_from(gateway2_data, 2) == 0) {
                gmn_err_sptr = gmn_sptr;
                gmn_err_x = gmn_x;
                gmn_err_y = gmn_y;
                return 0;
            }
            (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).extra_edge = first_choice;
            (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind = 0xc0;
            (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits &= 0x63;
            (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits |= 0x88;
            return 1;
        }

        test_citymap_neighbours_negedge(6);
        if (choose_from(tower_data, 0x10) == 0) {
            gmn_err_sptr = gmn_sptr;
            gmn_err_x = gmn_x;
            gmn_err_y = gmn_y;
            return 0;
        }
        (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).extra_edge = first_choice;
        (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind = 0xbf;
        (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits &= 0xe3;
        (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits |= 8;
        return 1;
    }
    return 1;
}

// Grow and transform the elastic aqueduct preview from the current construction start cell.
// FUNCTION: C2 0x67c23
// FUNCTION: C2WIN 0x004a2894
void get_aquaduct_elastic(void)
{
    int i;

    set_range(act_start_x, act_start_y, 0x15, 2, 0);
    (*(struct city_cell *)((unsigned char *)city_map + (act_start_pm_ptr))).road_aqueduct = 1;
    for (i = 1; i <= 20; i++)
        test_elastic_range(i, 0x1d);
    transform_aquaduct_elastic(20);
    if ((*(struct city_cell *)((unsigned char *)city_map + (act_start_pm_ptr))).road_aqueduct != 0xff)
        (*(struct city_cell *)((unsigned char *)city_map + (act_start_pm_ptr))).road_aqueduct = 1;
    elastic_start_dirc = 0;
}

// Mark legal aqueduct candidates near the current city construction start.
// FUNCTION: C2 0x67c75
// FUNCTION: C2WIN 0x004a292d
void transform_aquaduct_elastic(int radius)
{
    int x_min;
    int y_min;
    int x_max;
    int y_max;
    int side;
    int x_span;
    int stride;
    char kind;

    x_min = act_start_x - radius;
    y_min = act_start_y - radius;
    side  = 2 * radius + 1;
    x_span = side;
    x_max = x_min + side;
    if (x_min <= 0)       { x_span = x_max; x_min = 0; }
    else if (x_max > 80)  { x_span -= x_max - 80; }
    y_max = y_min + side;
    if (y_min <= 0)       { side = y_max; y_min = 0; }
    else if (y_max >= 80) { side -= y_max - 80; }

    gmn_sptr = (x_min + y_min * 80) * 20;
    stride   = (80 - x_span) * 20;

    gmn_y = y_min;
    for ( ; gmn_y < y_min + side; gmn_y++, gmn_sptr += stride) {
        gmn_x = x_min;
        for ( ; gmn_x < x_min + x_span; gmn_x++, gmn_sptr += 20) {
            if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct != 0xff) {
                if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 0x40) {
                    kind = (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind;
                    if (kind == 0xcf || kind == 0xd5) {
                        set_2_neighbours_if_not_aquaductorresevoir(gmn_x, gmn_y, gmn_sptr, 2, 0xff, 0);
                        (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = 0xff;
                    } else if (kind == 0xd0 || kind == 0xd6) {
                        set_2_neighbours_if_not_aquaductorresevoir(gmn_x, gmn_y, gmn_sptr, 2, 0xff, 1);
                        (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = 0xff;
                    } else if (kind <= 0xce) {
                        if (gmn_x == act_start_x && gmn_y == act_start_y) continue;
                        inc_elastic_by2(gmn_x, gmn_y, gmn_sptr);
                        (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct += 2;
                    } else if (kind > 0xce) {
                        set_4_neighbours_if_not_aquaductorresevoir(gmn_x, gmn_y, gmn_sptr, 2, 0xff);
                        (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = 0xff;
                    }
                } else if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 0x80) {
                    if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct > 1 && (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct != 0xff) {
                        (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct -= 1;
                    }
                }
                if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 0x20) {
                    if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind == 0x52)
                        (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct++;
                    else if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind == 0x53)
                        (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct++;
                    else
                        (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = 0xff;
                }
                if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 0x02) {
                    if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind <= 0xc2)
                        (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct++;
                    else
                        (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = 0xff;
                }
                if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind < 8 && ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits & 0x80)) {
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).road_aqueduct = 0xff;
                }
            }
        }
    }
}

// Build the aqueduct represented by the current elastic path and update its connections.
// FUNCTION: C2 0x67f13
// FUNCTION: C2WIN 0x004a2d9f
void build_aquaduct_from_elastic(void)
{
    int over_y_l;
    int over_x_l;
    int pm_ptr;
    int size;
    unsigned char path_value;
    unsigned char outcome;

    over_x_l = over_x;
    over_y_l = over_y;
    pm_ptr = pm_over_cm_ptr;

    size = CM_CELL(pm_over_cm_ptr).road_aqueduct;
    if (CM_CELL(pm_over_cm_ptr).terrain & 0x80)
        size++;
    if (size == 0) {
        illegal_build = 1;
    } else if (size == 0xff) {
        illegal_build = 1;
    } else {

        outcome = 0;

        while (size > 0) {
            size--;
            if ((CM_CELL(pm_ptr).terrain & 0xc0) == 0)
                particles_built++;
            if (CM_CELL(pm_ptr).base_kind < 0x1a)
                particles_cleared++;
            CM_CELL(pm_ptr).edge_bits |= 1;
            if (!(CM_CELL(pm_ptr).terrain & 0x80))
                CM_CELL(pm_ptr).terrain |= 0x40;

            path_value = CM_CELL(pm_ptr).road_aqueduct;
            get_best_elastic_value(over_x_l, over_y_l, pm_ptr, elastic_start_dirc);
            if (path_value >= best_elastic_value) {
                if (best_elastic_dirc == 0) { pm_ptr -= 0x640; over_y_l--; }
                else if (best_elastic_dirc == 1) { pm_ptr += 0x14;  over_x_l++; }
                else if (best_elastic_dirc == 2) { pm_ptr += 0x640; over_y_l++; }
                else if (best_elastic_dirc == 3) { pm_ptr -= 0x14;  over_x_l--; }
                continue;
            }
            if (path_value > 1) {
                outcome = 1;
                goto check_outer_state;
            }
            break;
        }

        size = CM_CELL(pm_over_cm_ptr).road_aqueduct;
        if (CM_CELL(pm_over_cm_ptr).terrain & 0x80)
            size++;

        over_x_l = over_x;
        over_y_l = over_y;
        pm_ptr = pm_over_cm_ptr;

        while (size > 0) {
            size--;
            if (!aquaduct_ramifications(over_x_l, over_y_l)) {
                outcome = 2;
                goto check_outer_state;
            }
            path_value = CM_CELL(pm_ptr).road_aqueduct;
            get_best_elastic_value(over_x_l, over_y_l, pm_ptr, elastic_start_dirc);
            if (path_value >= best_elastic_value) {
                if (best_elastic_dirc == 0) { pm_ptr -= 0x640; over_y_l--; }
                else if (best_elastic_dirc == 1) { pm_ptr += 0x14;  over_x_l++; }
                else if (best_elastic_dirc == 2) { pm_ptr += 0x640; over_y_l++; }
                else if (best_elastic_dirc == 3) { pm_ptr -= 0x14;  over_x_l--; }
                continue;
            }
            if (path_value > 1) {
                outcome = 3;
                goto check_outer_state;
            }
            break;
        }

    check_outer_state:
        if (outcome != 0) {
            illegal_build = 1;
            restore_city_from_undo_buffer();
            elastic_start_dirc++;
            if (elastic_start_dirc > 3)
                elastic_start_dirc = 0;
        }
    }
}

// Recompute aqueduct connections at a cell and its surrounding neighbourhood.
// FUNCTION: C2 0x6811e
// FUNCTION: C2WIN 0x004a30d6
int aquaduct_ramifications(int x, int y)
{
    int x_min;
    int y_min;
    int x_max;
    int y_max;

    if (x <= 0) x_min = 0; else x_min = x - 1;
    if (y <= 0) y_min = 0; else y_min = y - 1;
    if (x >= 79) x_max = 79; else x_max = x + 1;
    if (y >= 79) y_max = 79; else y_max = y + 1;

    gmn_x = x;
    gmn_y = y;
    if (one_aquaduct_ramification() == 0)
        return 0;

    for (gmn_y = y_min; y_max >= gmn_y; gmn_y++) {
        for (gmn_x = x_min; x_max >= gmn_x; gmn_x++) {
            if (one_aquaduct_ramification() == 0)
                return 0;
        }
    }

    return 1;
}

// Choose the aqueduct sprite matching the current cell's neighbouring aqueducts.
// FUNCTION: C2 0x681ad
// FUNCTION: C2WIN 0x004a31e8
int one_aquaduct_ramification(void)
{
    unsigned char polar;
    int sel;
    unsigned char sprite;

    gmn_sptr = ((gmn_x) + (gmn_y) * 80) * 20;
    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits |= 1;
    if (((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits & 0x20) == 0)
        get_aqua_web(gmn_x, gmn_y);

    polar = (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).range_flag & 3;
    if (((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 0x40) != 0) {
        test_citymap_neighbours_negedge(0xc0);
        if (((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 2) != 0) {
            if (gmn_polar_count == 0) {
                if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind == 0xc1) first_choice = 0xbd;
                else if ((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind != 0xc2) return 0;
                else first_choice = 0xbc;
                (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind = first_choice;
                (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits &= 0xe3;
                (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits |= 8;
                if (first_choice == 0xbc) sprite = 3;
                else sprite = 7;
                (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).extra_edge = sprite;
                (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).building = sprite;
                return 1;
            } else {
                if (choose_from(aquawall_data, 2) != 0) {
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind = first_choice;
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits &= 0xe3;
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits |= 8;
                    if (first_choice == 0xbc) sprite = 3;
                    else sprite = 7;
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).extra_edge = sprite;
                    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).building = sprite;
                    return 1;
                }
                err:
                gmn_err_sptr = gmn_sptr;
                gmn_err_x = gmn_x;
                gmn_err_y = gmn_y;
                return 0;
            }
        }
        if (((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 0x20) != 0) {
            sel = choose_from(aquaroad_data, 2);
            (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits |= 0x80;
        } else {
            sel = choose_from(aquaduct_data, 0xe);
        }
        if (sel == 0) {
            goto err;
        }
        (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).base_kind = first_choice;
        (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits &= 0xe3;
        (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).edge_bits |= wall_gfxdat[first_choice - WALL_GFX_FIRST_TILE].edge_bits;
        (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).extra_edge = wall_gfxdat[first_choice - WALL_GFX_FIRST_TILE].sprite;
        (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).building = wall_gfxdat[first_choice - WALL_GFX_FIRST_TILE].sprite;
        if (polar == 3) (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).extra_edge += 2;
        else if (polar >= 1) (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).extra_edge += 1;
        return 1;
    }

    if (((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).terrain & 0x80) == 0)
        return 1;
    test_citymap_neighbours_negedge(0xc0);
    if (choose_from(resevoir_data, 0x10) == 0) {
        goto err;
    }
    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).extra_edge = first_choice;
    (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).building = first_choice;
    if (polar == 3) (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).extra_edge += 3;
    if (polar == 2) (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).extra_edge += 2;
    if (polar == 1) (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).extra_edge++;
    return 1;
}

// Grow and transform the regional road preview from the current construction start cell.
// FUNCTION: C2 0x68472
// FUNCTION: C2WIN 0x004a36d5
void get_reg_road_elastic(void)
{
    int i;

    set_rm_range(act_start_x, act_start_y, 0x15, 2, 0);
    (*(struct region_cell *)((unsigned char *)region_map + (act_start_pm_ptr))).place_state = 1;
    for (i = 1; i <= 20; i++)
        test_rm_elastic_range(1, i, 0xd9);
    transform_reg_road_elastic(20);
}

// Mark legal regional-road candidates near the current construction start.
// FUNCTION: C2 0x684c8
// FUNCTION: C2WIN 0x004a3744
int transform_reg_road_elastic(int radius)
{
    int x_min;
    int y_min;
    int x_max;
    int y_max;
    int side;
    int x_span;
    int stride;

    x_min = act_start_x - radius;
    y_min = act_start_y - radius;
    side  = 2 * radius + 1;
    x_span = side;
    x_max = side + x_min;
    if (x_min <= 0)       { x_span = x_max; x_min = 0; }
    else if (x_max > 60)  { x_span -= x_max - 60; }
    y_max = side + y_min;
    if (y_min <= 0)       { side = y_max; y_min = 0; }
    else if (y_max >= 60) { side -= y_max - 60; }

    gmn_sptr = ((x_min) + (y_min) * 60) * 8;
    stride   = (60 - x_span) * 8;

    gmn_y = y_min;
    for ( ; gmn_y < y_min + side; gmn_y++, gmn_sptr += stride) {
        gmn_x = x_min;
        for ( ; gmn_x < x_min + x_span; gmn_x++, gmn_sptr += 8) {
            if ((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state != 0xff) {
                if ((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain & 0x02) {
                    if ((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).base_kind <= 0xb8
                        || (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).base_kind >= 0xbd) continue;
                    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state = 0xff;
                }
                if ((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain & 0x04) {
                    if ((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain & 0x20) continue;
                    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state = 0xff;
                }
                if ((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain & 0x08) {
                    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state = 0xff;
                }
                if ((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain & 0x40) {
                    if ((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).base_kind < 0xc7) continue;
                    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state = 0xff;
                }
            }
        }
    }

    return y_min + side;
}

// Build the regional road represented by the current elastic path.
// FUNCTION: C2 0x68654
// FUNCTION: C2WIN 0x004a39b8
void build_reg_road_from_elastic(void)
{
    int over_y_l;
    int over_x_l;
    int cell_ptr;
    int count;
    unsigned char saved_slot;
    unsigned char status;
    unsigned char neighbour;
    int attempt;
    int dir;

    count = (*(struct region_cell *)((unsigned char *)region_map + (pm_over_cm_ptr))).place_state;
    if (count == 0) {
        illegal_build = 1;
        return;
    }
    if (count == 0xff) {
        illegal_build = 1;
        return;
    }

    over_x_l = over_x;
    over_y_l = over_y;
    cell_ptr = pm_over_cm_ptr;
    dir      = 0;
    status   = 0;

    while (count > 0) {
        count--;
        if (!((*(struct region_cell *)((unsigned char *)region_map + (cell_ptr))).terrain & 0x20)) particles_built++;
        if ((*(struct region_cell *)((unsigned char *)region_map + (cell_ptr))).base_kind < 0x10) particles_cleared++;
        (*(struct region_cell *)((unsigned char *)region_map + (cell_ptr))).terrain |= 0x20;
        (*(struct region_cell *)((unsigned char *)region_map + (cell_ptr))).edge_bits |= 1;

        saved_slot = (*(struct region_cell *)((unsigned char *)region_map + (cell_ptr))).place_state;
        attempt = 4;
        dir = 0;
        while (attempt-- > 0) {
            neighbour = 0;
            if (++dir > 3) dir = 0;
            if (dir == 0) {
                if (over_y_l > 0) neighbour = (*(struct region_cell *)((unsigned char *)region_map + (cell_ptr - 480))).place_state;
                if (neighbour != 0 && neighbour < saved_slot) { cell_ptr -= 0x1e0; over_y_l--; break; }
            } else if (dir == 1) {
                if (over_x_l < 0x3b) neighbour = (*(struct region_cell *)((unsigned char *)region_map + (cell_ptr + 8))).place_state;
                if (neighbour != 0 && neighbour < saved_slot) { cell_ptr += 8; over_x_l++; break; }
            } else if (dir == 2) {
                if (over_y_l < 0x3b) neighbour = (*(struct region_cell *)((unsigned char *)region_map + (cell_ptr + 480))).place_state;
                if (neighbour != 0 && neighbour < saved_slot) { cell_ptr += 0x1e0; over_y_l++; break; }
            } else if (dir == 3) {
                if (over_x_l > 0) neighbour = (*(struct region_cell *)((unsigned char *)region_map + (cell_ptr - 8))).place_state;
                if (neighbour != 0 && neighbour < saved_slot) { cell_ptr -= 8; over_x_l--; break; }
            }
        }
        if (neighbour != 0 && neighbour < saved_slot)
            continue;
        if (saved_slot > 1) {
            status = 1;
            goto finish;
        }
        break;
    }

    count    = (*(struct region_cell *)((unsigned char *)region_map + (pm_over_cm_ptr))).place_state;
    over_x_l = over_x;
    over_y_l = over_y;
    cell_ptr = pm_over_cm_ptr;
    dir      = 0;

    while (count > 0) {
        count--;
        if (reg_road_ramifications(over_x_l, over_y_l) == 0) { status = 2; goto finish; }

        saved_slot = (*(struct region_cell *)((unsigned char *)region_map + (cell_ptr))).place_state;
        attempt = 4;
        dir = 0;
        while (attempt-- > 0) {
            neighbour = 0;
            if (++dir > 3) dir = 0;
            if (dir == 0) {
                if (over_y_l > 0) neighbour = (*(struct region_cell *)((unsigned char *)region_map + (cell_ptr - 480))).place_state;
                if (neighbour != 0 && neighbour < saved_slot) { cell_ptr -= 0x1e0; over_y_l--; break; }
            } else if (dir == 1) {
                if (over_x_l < 0x3b) neighbour = (*(struct region_cell *)((unsigned char *)region_map + (cell_ptr + 8))).place_state;
                if (neighbour != 0 && neighbour < saved_slot) { cell_ptr += 8; over_x_l++; break; }
            } else if (dir == 2) {
                if (over_y_l < 0x3b) neighbour = (*(struct region_cell *)((unsigned char *)region_map + (cell_ptr + 480))).place_state;
                if (neighbour != 0 && neighbour < saved_slot) { cell_ptr += 0x1e0; over_y_l++; break; }
            } else if (dir == 3) {
                if (over_x_l > 0) neighbour = (*(struct region_cell *)((unsigned char *)region_map + (cell_ptr - 8))).place_state;
                if (neighbour != 0 && neighbour < saved_slot) { cell_ptr -= 8; over_x_l--; break; }
            }
        }
        if (neighbour != 0 && neighbour < saved_slot)
            continue;
        if (saved_slot > 1) {
            status = 3;
            goto finish;
        }
        break;
    }

finish:
    if (status != 0) {
        restore_region_from_undo_buffer();
        illegal_build = 1;
    }
}

// Recompute regional road and wall-crossing sprites around a cell.
// FUNCTION: C2 0x688bb
// FUNCTION: C2WIN 0x004a3eb6
int reg_road_ramifications(int x, int y)
{
    int x_min;
    int y_max;
    int x_max;
    int y_min;

    if (x == 0) x_min = 0; else x_min = x - 1;
    if (y == 0) y_min = 0; else y_min = y - 1;
    if (x == 59) x_max = 59; else x_max = x + 1;
    if (y == 59) y_max = 59; else y_max = y + 1;

    for (gmn_y = y_min; y_max >= gmn_y; gmn_y++) {
        for (gmn_x = x_min; x_max >= gmn_x; gmn_x++) {
            gmn_sptr = (gmn_x + gmn_y * 60) * 8;
            if (((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain & 0x20) != 0) {
                (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).edge_bits |= 1;
                if (((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain & 2) != 0) {
                    if (one_reg_wall_ramification() == 0) {
                        (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain &= 0xf9;
                        (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain |= 2;
                        return 0;
                    }
                } else {
                    test_regionmap_neighbours_posedge(0xe5);
                    if (choose_from(road_data, 0x10) != 0) {
                        (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).base_kind = first_choice + 0x4e;
                        (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).edge_bits &= 0xe3;
                        (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).gfx = first_choice - 0x52;
                    } else {
                        (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain &= 0xdf;
                        return 0;
                    }
                }
            }
        }
    }
    return 1;
}

// Grow and transform the regional wall preview from the current construction start cell.
// FUNCTION: C2 0x689f6
// FUNCTION: C2WIN 0x004a40d4
void get_reg_wall_elastic(void)
{
    int i;

    set_rm_range(act_start_x, act_start_y, 0x15, 2, 0);
    (*(struct region_cell *)((unsigned char *)region_map + (act_start_pm_ptr))).place_state = 1;
    for (i = 1; i <= 0x14; i++)
        test_rm_elastic_range(0, i, 0xd9);
    transform_reg_wall_elastic(0x14);
    if ((*(struct region_cell *)((unsigned char *)region_map + (act_start_pm_ptr))).place_state != 0xff)
        (*(struct region_cell *)((unsigned char *)region_map + (act_start_pm_ptr))).place_state = 1;
    elastic_start_dirc = 0;
}

// Mark legal regional-wall candidates near the current construction start.
// FUNCTION: C2 0x68a6a
// FUNCTION: C2WIN 0x004a4172
void transform_reg_wall_elastic(int radius)
{
    int x_min;
    int y_min;
    int x_max;
    int y_max;
    int side;
    int x_span;
    int stride;
    int place_state;
    int base_kind;
    int occupied_base_kind;

    x_min = act_start_x - radius;
    y_min = act_start_y - radius;
    side  = 2 * radius + 1;
    x_span = side;
    x_max = x_min + side;
    if (x_min <= 0)        { x_span = x_max; x_min = 0; }
    else if (x_max > 60)   { x_span -= x_max - 60; }
    y_max = side + y_min;
    if (y_min <= 0)        { side = y_max; y_min = 0; }
    else if (y_max >= 60)  { side -= y_max - 60; }

    gmn_sptr = (x_min + y_min * 60) * 8;
    stride   = (60 - x_span) * 8;

    gmn_y = y_min;
    for ( ; gmn_y < y_min + side; gmn_y++, gmn_sptr += stride) {
        gmn_x = x_min;
        for ( ; gmn_x < x_min + x_span; gmn_x++, gmn_sptr += 8) {
            place_state = ((unsigned char *)region_map)[(gmn_sptr + 2)];
            if (place_state == 0xff) continue;

            if (((unsigned char *)region_map)[(gmn_sptr + 1)] & 0x02) {
                base_kind = ((unsigned char *)region_map)[(gmn_sptr)];
                if (base_kind < 0xbd) {
                    if (!(((unsigned char *)region_map)[(gmn_sptr + 1)] & 0x20)) {
                        set_4_rm_neighbours_if_not_wallortower(
                            gmn_x, gmn_y, gmn_sptr, 2, 0xff);
                        ((unsigned char *)region_map)[(gmn_sptr + 2)] = 0xff;
                    }
                } else if (base_kind <= 0xc0) {
                    if (gmn_x == act_start_x && gmn_y == act_start_y)
                        continue;
                    inc_elastic_by2(gmn_x, gmn_y, gmn_sptr);
                    ((unsigned char *)region_map)[(gmn_sptr + 2)] += 2;
                }
            } else if ((((unsigned char *)region_map)[(gmn_sptr + 1)] & 0x04)
                       && place_state > 1 && place_state != 0xff) {
                ((unsigned char *)region_map)[(gmn_sptr + 2)]--;
            }

            if (((unsigned char *)region_map)[(gmn_sptr + 1)] & 0x20) {
                occupied_base_kind = ((unsigned char *)region_map)[(gmn_sptr)];
                if (occupied_base_kind == 0xa0) {
                    ((unsigned char *)region_map)[(gmn_sptr + 2)]++;
                } else if (occupied_base_kind == 0xa1) {
                    ((unsigned char *)region_map)[(gmn_sptr + 2)]++;
                } else if (((unsigned char *)region_map)[(gmn_sptr + 1)] & 0x02) {
                    ((unsigned char *)region_map)[(gmn_sptr + 2)]++;
                } else {
                    ((unsigned char *)region_map)[(gmn_sptr + 2)] = 0xff;
                }
            }
        }
    }
}

// Build the regional wall represented by the current elastic path.
// FUNCTION: C2 0x68c7c
// FUNCTION: C2WIN 0x004a450a
void build_reg_wall_from_elastic(void)
{
    int over_y_l;
    int over_x_l;
    int pm_ptr;
    unsigned char road_byte;
    unsigned char build_outcome;
    int size;

    size = RM_CELL(pm_over_cm_ptr).place_state;
    if (RM_CELL(pm_over_cm_ptr).terrain & 0x04) size++;

    if (size == 0) {
        illegal_build = 1;
    } else if (size == 0xff) {
        illegal_build = 1;
    } else {
        build_outcome = 0;
        over_x_l = over_x;
        over_y_l = over_y;
        pm_ptr = pm_over_cm_ptr;

        while (size > 0) {
            size--;
            if (!(RM_CELL(pm_ptr).terrain & 0x02)) particles_built++;
            if (RM_CELL(pm_ptr).base_kind < 0x10) particles_cleared++;
            RM_CELL(pm_ptr).edge_bits |= 1;
            if (!(RM_CELL(pm_ptr).terrain & 0x04)) RM_CELL(pm_ptr).terrain |= 0x02;
            road_byte = RM_CELL(pm_ptr).place_state;
            get_best_rm_elastic_value(over_x_l, over_y_l, pm_ptr, elastic_start_dirc);
            if (road_byte >= best_elastic_value) {
                if (best_elastic_dirc == 0) { pm_ptr -= 0x1e0; over_y_l--; }
                else if (best_elastic_dirc == 1) { pm_ptr += 8; over_x_l++; }
                else if (best_elastic_dirc == 2) { pm_ptr += 0x1e0; over_y_l++; }
                else if (best_elastic_dirc == 3) { pm_ptr -= 8; over_x_l--; }
                continue;
            }
            if (road_byte > 1) {
                build_outcome = 1;
                goto finish;
            }
            break;
        }

        size = RM_CELL(pm_over_cm_ptr).place_state;
        if (RM_CELL(pm_over_cm_ptr).terrain & 0x04) size++;

        over_x_l = over_x;
        over_y_l = over_y;
        pm_ptr = pm_over_cm_ptr;

        while (size > 0) {
            size--;
            if (!reg_wall_ramifications(over_x_l, over_y_l)) { build_outcome = 2; goto finish; }
            road_byte = RM_CELL(pm_ptr).place_state;
            get_best_rm_elastic_value(over_x_l, over_y_l, pm_ptr, elastic_start_dirc);
            if (road_byte >= best_elastic_value) {
                if (best_elastic_dirc == 0) { pm_ptr -= 0x1e0; over_y_l--; }
                else if (best_elastic_dirc == 1) { pm_ptr += 8; over_x_l++; }
                else if (best_elastic_dirc == 2) { pm_ptr += 0x1e0; over_y_l++; }
                else if (best_elastic_dirc == 3) { pm_ptr -= 8; over_x_l--; }
                continue;
            }
            if (road_byte > 1) {
                build_outcome = 3;
                goto finish;
            }
            break;
        }

    finish:
        if (build_outcome != 0) {
            illegal_build = 1;
            restore_region_from_undo_buffer();
            elastic_start_dirc++;
            if (elastic_start_dirc > 3) elastic_start_dirc = 0;
        }
    }
}

// Recompute regional wall connections at a cell and its surrounding neighbourhood.
// FUNCTION: C2 0x68ea5
// FUNCTION: C2WIN 0x004a4841
int reg_wall_ramifications(int x, int y)
{
    int x_min;
    int y_min;
    int x_max;
    int y_max;

    if (x == 0) x_min = 0; else x_min = x - 1;
    if (y == 0) y_min = 0; else y_min = y - 1;
    if (x == 59) x_max = x; else x_max = x + 1;
    if (y == 59) y_max = y; else y_max = y + 1;

    gmn_x = x;
    gmn_y = y;
    if (one_reg_wall_ramification() == 0)
        return 0;

    for (gmn_y = y_min; y_max >= gmn_y; gmn_y++) {
        for (gmn_x = x_min; x_max >= gmn_x; gmn_x++) {
            if (one_reg_wall_ramification() == 0)
                return 0;
        }
    }

    return 1;
}

// Choose the regional wall sprite matching the current cell's neighbouring walls.
// FUNCTION: C2 0x68f2e
// FUNCTION: C2WIN 0x004a4953
int one_reg_wall_ramification(void)
{
    gmn_sptr = ((gmn_x) + (gmn_y) * 60) * 8;
    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).edge_bits |= 1;

    if (((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain & 2) != 0) {
        if (((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain & 0x20) != 0) {
            test_type_regionmap_neighbours_negedge(0xb6);
            if (gmn_polar_count != 0) {
                gmn_err_sptr = gmn_sptr;
                gmn_err_x = gmn_x;
                gmn_err_y = gmn_y;
                return 0;
            }
            test_regionmap_neighbours_negedge(6);
            if (choose_from(regwallroad_data, 2) == 0) {
                gmn_err_sptr = gmn_sptr;
                gmn_err_x = gmn_x;
                gmn_err_y = gmn_y;
                return 0;
            }
            (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).gfx = first_choice;
            (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).base_kind = 0xb6;
            (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).edge_bits &= 0xe3;
            return 1;
        }
        test_regionmap_neighbours_negedge(6);
        if (choose_from(wall_data, 0xe) == 0) {
            gmn_err_sptr = gmn_sptr;
            gmn_err_x = gmn_x;
            gmn_err_y = gmn_y;
            return 0;
        }
        (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).base_kind = first_choice - 0xa;
        (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).edge_bits &= 0xe3;
        (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).gfx = house_gfxdat[first_choice + 0x51];
        return 1;
    }

    if (((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain & 4) != 0) {
        test_regionmap_neighbours_negedge(6);
        if (choose_from(tower_data, 0x10) == 0) {
            gmn_err_sptr = gmn_sptr;
            gmn_err_x = gmn_x;
            gmn_err_y = gmn_y;
            return 0;
        }
        (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).gfx = first_choice;
        (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).base_kind = 0xd2;
        (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).edge_bits &= 0xe3;
        return 1;
    }
    return 1;
}

// Fill a city-map rectangle with random garden tiles while preserving stone and flagged cells.
// FUNCTION: C2 0x69093
// FUNCTION: C2WIN 0x004a4bca
void garden_an_area(int x1, int y1, int x2, int y2)
{
    int saved_count = stone_random_count;
    int row_stride;
    int swap_value;
    int y;
    int x;

    if (x1 > x2) {
        swap_value = x2;
        x2 = x1;
        x1 = swap_value;
    }
    if (y1 > y2) {
        swap_value = y2;
        y2 = y1;
        y1 = swap_value;
    }

    cm_sptr = (x1 + y1 * 80) * 20;
    row_stride = (80 - (x2 - x1) - 1) * 20;

    for (y = y1; y <= y2;) {
        for (x = x1; x <= x2;) {
            if ((*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).base_kind < 0x1e) {
                if ((*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).base_kind >= 8 || !((*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).edge_bits & 0x80)) {
                    stone_random_count++;
                    if (stone_random_count >= 0x40)
                        stone_random_count = 0;
                    if ((*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).base_kind < 0x1a)
                        particles_cleared++;
                    particles_built++;
                    (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).base_kind = (unsigned char)((stone_random_data[stone_random_count] >> 2) + 0x78);
                    clear_basic(cm_sptr);
                    (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).extra_edge = (unsigned char)((stone_random_data[stone_random_count] >> 2) + 0x77);
                    (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).edge_bits &= 0xe3;
                    (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).edge_bits |= 4;
                    (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).activity_a = 0;
                }
            }
            x++;
            cm_sptr += 20;
        }
        y++;
        cm_sptr += row_stride;
    }
    stone_random_count = (signed char)saved_count;
    if (particles_built == 0)
        illegal_build = 1;
}

// Pave every clearable cell in a city-map rectangle as plaza.
// FUNCTION: C2 0x6921c
// FUNCTION: C2WIN 0x004a4dd7
void plaza_an_area(int x1, int y1, int x2, int y2)
{
    int row_skip;
    int swap_value;
    int y;
    int x;

    if (x1 > x2) {
        swap_value = x2;
        x2 = x1;
        x1 = swap_value;
    }
    if (y1 > y2) {
        swap_value = y2;
        y2 = y1;
        y1 = swap_value;
    }

    cm_sptr = (x1 + y1 * 80) * 20;
    row_skip = (80 - (x2 - x1) - 1) * 20;

    for (y = y1; y <= y2;) {
        for (x = x1; x <= x2;) {
            if ((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind < 0x1e
                && ((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind >= 8
                    || ((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).edge_bits & 0x80) == 0)) {
                if ((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind < 0x1a) particles_cleared++;
                particles_built++;
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind = 0x7c;
                clear_basic(cm_sptr);
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).extra_edge = 0x74;
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).edge_bits &= 0xe3;
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).edge_bits |= 4;
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a = 0;
                (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).terrain |= 0x20;
            }
            x++;
            cm_sptr += 20;
        }
        y++;
        cm_sptr += row_skip;
    }
    if (particles_built == 0) illegal_build = 1;
}

// Clear/demolish every city-map cell in an inclusive rectangle.
// FUNCTION: C2 0x69344
// FUNCTION: C2WIN 0x004a4fa8
void clear_an_area(int x1, int y1, int x2, int y2)
{
    char size;
    int swap_temp;
    int row_skip;
    int x;
    int y;
    int saved_random;
    unsigned char kind;

    saved_random = stone_random_count;
    if (x1 > x2) { swap_temp = x2; x2 = x1; x1 = swap_temp; }
    if (y1 > y2) { swap_temp = y2; y2 = y1; y1 = swap_temp; }

    cm_sptr = (x1 + y1 * 80) * 20;
    row_skip = (80 - (x2 - x1) - 1) * 20;

    for (y = y1; y <= y2; ) {
        for (x = x1; x <= x2; ) {
            (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).edge_bits &= 0xbf; x++; cm_sptr += 20; } y++; cm_sptr += row_skip; }

    cm_sptr = (x1 + y1 * 80) * 20;
    for (y = y1; y <= y2; ) {
        for (x = x1; x <= x2; ) {

            if (((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).terrain & 0x10) != 0) {

                if (((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).terrain & 0x20) != 0) {

                    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building;
                    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).terrain &= 0xdf;
                    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).building = 0;
                }
            } else {

                if ((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind >= 0x82) {

                    kind = (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind;
                    size = forum_gfxdat[kind + 0x26];
                    if (size == 0) clear_to_empty(cm_sptr);
                    if (size == 4) clear_sized_to_rubble(cm_sptr, 2, 0);
                    else if (size == 9) clear_sized_to_rubble(cm_sptr, 3, 0);
                    else if (size == 0x10) clear_sized_to_rubble(cm_sptr, 4, 0);
                    else if (kind < 0x82) clear_to_empty(cm_sptr);
                    else clear_to_rubble(cm_sptr, 0);

                } else if ((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind < 8) {
                    if (((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).edge_bits & 0x80) == 0)
                        if (((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).edge_bits & 0x40) == 0)

                            clear_to_empty(cm_sptr);
                } else {
                    clear_to_empty(cm_sptr);
                }
            }
            x++; cm_sptr += 20; } y++; cm_sptr += row_skip; }

    cm_sptr = (x1 + y1 * 80) * 20;
    for (y = y1; y <= y2; ) {
        for (x = x1; x <= x2; ) {
            road_ramifications(x, y);
            wall_ramifications(x, y);
            aquaduct_ramifications(x, y);
            x++; cm_sptr += 20; } y++; cm_sptr += row_skip; }
    stone_random_count = (signed char)saved_random;
}

// Demolish a region-map rectangle while preserving protected forts and occupied army ranges.
// FUNCTION: C2 0x695b9
// FUNCTION: C2WIN 0x004a536b
void clear_a_reg_area(int x0, int y0, int x1, int y1, int keep_fortress)
{
    int saved_random;
    int row_skip;
    unsigned char kind;
    int x;
    int y;
    int swap_value;

    saved_random = stone_random_count;

    if (x1 < x0) { swap_value = x1; x1 = x0; x0 = swap_value; }
    if (y0 > y1) { swap_value = y1; y1 = y0; y0 = swap_value; }

    cm_sptr = (x0 + y0 * 60) * 8;
    row_skip = (60 - (x1 - x0)) * 8 - 8;

    for (y = y0; y <= y1; y++, cm_sptr += row_skip) {
        for (x = x0; x <= x1; x++, cm_sptr += 8) {
            if ((RM_CELL(cm_sptr).terrain & 0x10) != 0) continue;
            stone_random_count++;
            if (stone_random_count >= 0x40) stone_random_count = 0;
            kind = RM_CELL(cm_sptr).base_kind;
            if (kind >= 0x92 && kind <= 0x9b) continue;
            if (keep_fortress != 0 && kind == 0xd2) continue;
            if (kind < 0x10) particles_cleared++;
            if (kind >= 0x20 && kind < 0x7c) {
            } else if (kind >= 0xd5 && kind <= 0xeb) {
                clear_sized_to_reg_basic(cm_sptr, 2);
            } else if (kind >= 0xec && kind <= 0xef) {
                clear_sized_to_reg_basic(cm_sptr, 2);
                unflag_rm_area(x - ofset_x, y - ofset_y, 2, 0xf7);
                adjust_regions_coastline(x - ofset_x - 1,
                                         y - ofset_y - 1, 4, 4);
            } else {
                clear_reg_basic(cm_sptr);
            }
            if (kind == 0xd2) clear_army_from_fort_ref(cm_sptr);
        }
    }

    for (y = y0; y <= y1; y++, cm_sptr += row_skip) {
        for (x = x0; x <= x1; x++, cm_sptr += 8) {
            reg_road_ramifications(x, y);
            reg_wall_ramifications(x, y);
        }
    }
    stone_random_count = saved_random;
}

// Destroy a regional structure and refresh the affected terrain and connections.
// FUNCTION: C2 0x697e7
// FUNCTION: C2WIN 0x004a564b
void destroy_reg_atom(int sptr)
{
    int cell = sptr / 8;
    int y;
    int x = cell % 60;
    int saved_random;
    unsigned char kind;
    y = cell / 60;
    saved_random = stone_random_count;
    kind = (*(struct region_cell *)((unsigned char *)region_map + (sptr))).base_kind;

    if (!((kind >= 0x20 && kind < 0x7c) ||
          (kind >= 0x92 && kind <= 0x9b))) {
        if (kind >= 0xd5 && kind <= 0xeb) {
            clear_sized_to_reg_basic(sptr, 2);
        } else if (kind >= 0xec && kind <= 0xef) {
            clear_sized_to_reg_basic(sptr, 2);
            unflag_rm_area(x - ofset_x, y - ofset_y, 2, 0xf7);
            adjust_regions_coastline(x - ofset_x - 1, y - ofset_y - 1, 4, 4);
        } else if (((*(struct region_cell *)((unsigned char *)region_map + (sptr))).terrain & 0x28) == 0) {
            clear_reg_basic(sptr);
        }
    }
    stone_random_count = (signed char)saved_random;
    particles_cleared = 0;
}

// Destroy one city-map atom, clear its footprint correctly, and refresh nearby aqueduct links.
// FUNCTION: C2 0x69907
// FUNCTION: C2WIN 0x004a57bd
void destroy_an_atom(int sptr, int rubble_kind)
{
    int saved_random;
    int kind;
    int size;
    int cell_index;
    int x;
    int y;

    saved_random = stone_random_count;
    kind = (*(struct city_cell *)((unsigned char *)city_map + (sptr))).base_kind & 0xff;
    if (kind >= 0x82) {
        size = forum_gfxdat[kind + 0x26] & 0xff;
        if (size == 4)
            clear_sized_to_rubble(sptr, 2, rubble_kind);
        else if (size == 9)
            clear_sized_to_rubble(sptr, 3, rubble_kind);
        else if (size == 0x10)
            clear_sized_to_rubble(sptr, 4, rubble_kind);
        else if (kind >= 0x82)
            clear_to_rubble(sptr, rubble_kind);
        else
            clear_to_empty(sptr);
    } else if (kind >= 8) {
        clear_to_empty(sptr);
    }

    cell_index = sptr / 20;
    x = cell_index % 80;
    y = cell_index / 80;
    aquaduct_ramifications(x, y);
    stone_random_count = (signed char)saved_random;
    particles_cleared = 0;
}

// Spread fire in one direction and reduce the affected building to burning rubble.
// FUNCTION: C2 0x699d7
// FUNCTION: C2WIN 0x004a590a
void spread_fire_atom(int sptr, int dir)
{
    char kind_byte;
    int kind;
    int size;

    if (dir == 0)
        sptr -= 0x640;
    else if (dir == 4)
        sptr += 0x640;
    else if (dir == 6)
        sptr -= 0x14;
    else if (dir == 2)
        sptr += 0x14;

    kind_byte = (*(struct city_cell *)((unsigned char *)city_map + (sptr))).base_kind;
    kind = (unsigned char)kind_byte;
    if (kind >= 0xbc && kind <= 0xe2) return;
    kind = (unsigned char)kind_byte;
    if (kind < 0x82) return;

    size = forum_gfxdat[kind + 0x26] & 0xff;
    if (size == 4)
        clear_sized_to_rubble(sptr, 2, 1);
    else if (size == 9)
        clear_sized_to_rubble(sptr, 3, 1);
    else if (size == 0x10)
        clear_sized_to_rubble(sptr, 4, 1);
    else
        clear_to_rubble(sptr, 1);
}

// Spread plague in one direction to an eligible neighbouring building.
// FUNCTION: C2 0x69a77
// FUNCTION: C2WIN 0x004a5a2b
void spread_plague_atom(int sptr, int dir)
{
    int kind;
    int tile_size;

    if (dir == 0)
        sptr -= 0x640;
    else if (dir == 4)
        sptr += 0x640;
    else if (dir == 6)
        sptr -= 0x14;
    else if (dir == 2)
        sptr += 0x14;

    kind = (*(struct city_cell *)((unsigned char *)city_map + (sptr))).base_kind & 0xff;
    if (kind < 0x82) return;
    if (kind > 0xa1) return;
    if (((*(struct city_cell *)((unsigned char *)city_map + (sptr))).edge_bits & 0x80) != 0) return;

    tile_size = forum_gfxdat[kind + 0x26] & 0xff;
    if (tile_size == 4)
        plague_sized(sptr, 2);
    else if (tile_size == 9)
        plague_sized(sptr, 3);
    else
        plague_it(sptr);
}

// Mark the building footprint containing a city cell as plague-stricken.
// FUNCTION: C2 0x69afe
// FUNCTION: C2WIN 0x004a5b27
void plague_an_atom(int sptr)
{
    int kind = (*(struct city_cell *)((unsigned char *)city_map + (sptr))).base_kind & 0xff;
    int tile_size;

    if (kind < 0x82) return;
    if (kind > 0xa1) return;

    tile_size = forum_gfxdat[kind + 0x26] & 0xff;
    if (tile_size == 4)
        plague_sized(sptr, 2);
    else if (tile_size == 9)
        plague_sized(sptr, 3);
    else
        plague_it(sptr);
}

// Spread plague across the full `size` by `size` building footprint containing `sptr`.
// FUNCTION: C2 0x69b4b
// FUNCTION: C2WIN 0x004a5bc7
void plague_sized(int sptr, int size)
{
    int x;
    int y;

    x = (*(struct city_cell *)((unsigned char *)city_map + (sptr))).activity_a & 0xf;
    y = x % size;
    x /= size;
    sptr = sptr - y * 20 - x * 80 * 20;
    for (y = 0; y < size; y++, sptr += (80 - size) * 20)
        for (x = 0; x < size; x++, sptr += 20)
            plague_it(sptr);
}

// Reduce an entire building footprint, including linked gatehouse halves, to rubble.
// FUNCTION: C2 0x69bc6
// FUNCTION: C2WIN 0x004a5c9a
void clear_sized_to_rubble(int sptr, int size, int rubble_kind)
{
    int footprint_idx;
    int yoff;
    int xoff;
    unsigned char old_kind;
    int start_sptr;
    int x;
    int y;

    footprint_idx = CM_CELL(sptr).activity_a & 0xf;
    old_kind = CM_CELL(sptr).base_kind;
    xoff = footprint_idx % size; yoff = footprint_idx / size;

    sptr -= xoff * 20;
    sptr -= yoff * 1600;
    start_sptr = sptr;

    for (y = 0; y < size; y++, sptr += (80 - size) * 20)
        for (x = 0; x < size; x++, sptr += 20)
            clear_to_rubble(sptr, rubble_kind);

    if (old_kind < 0xe9) return;
    if (old_kind > 0xf0) return;

    sptr = start_sptr;
    if (old_kind == 0xe9) { if (CM_CELL(sptr + 0x12c0).base_kind == 0xea) sptr += 0x12c0; else sptr -= 0x12c0; }
    if (old_kind == 0xea) { if (CM_CELL(sptr - 0x12c0).base_kind == 0xe9) sptr -= 0x12c0; else sptr += 0x12c0; }
    if (old_kind == 0xeb) { if (CM_CELL(sptr + 0x3c).base_kind == 0xec) sptr += 0x3c; else sptr -= 0x3c; }
    if (old_kind == 0xec) { if (CM_CELL(sptr - 0x3c).base_kind == 0xeb) sptr -= 0x3c; else sptr += 0x3c; }
    if (old_kind == 0xed) { if (CM_CELL(sptr + 0x1900).base_kind == 0xee) sptr += 0x1900; else sptr -= 0x1900; }
    if (old_kind == 0xee) { if (CM_CELL(sptr - 0x1900).base_kind == 0xed) sptr -= 0x1900; else sptr += 0x1900; }
    if (old_kind == 0xef) { if (CM_CELL(sptr + 0x50).base_kind == 0xf0) sptr += 0x50; else sptr -= 0x50; }
    if (old_kind == 0xf0) { if (CM_CELL(sptr - 0x50).base_kind == 0xef) sptr -= 0x50; else sptr += 0x50; }
    for (y = 0; y < size; y++, sptr += (80 - size) * 20)
        for (x = 0; x < size; x++, sptr += 20)
            clear_to_rubble(sptr, rubble_kind);

    if (had_clear_sound == 0 || mouse_left_button == 0) {
        if (size <= 2) set_sound("medrub.wav", 1);
        else set_sound("lrgrub.wav", 1);
        if (mouse_left_button != 0) had_clear_sound = 1;
        else had_clear_sound = 0;
    }
#if C2_FEAT_TILE_REFRESH
    setup_map_screen_refresh();
#endif
}

// Turn one city cell into rubble and start fire effects when requested.
// FUNCTION: C2 0x69e2b
// FUNCTION: C2WIN 0x004a602f
void clear_to_rubble(int sptr, int rubble_kind)
{
    short random_value;

    stone_random_count++;
    if (stone_random_count >= 0x40) stone_random_count = 0;
    random_value = stone_random_data[stone_random_count];
    (*(struct city_cell *)((unsigned char *)city_map + (sptr))).base_kind = (random_value / 2);
    clear_basic(sptr);
    (*(struct city_cell *)((unsigned char *)city_map + (sptr))).edge_bits |= 0x40;

    if (rubble_kind != 0) {
        (*(struct city_cell *)((unsigned char *)city_map + (sptr))).edge_bits |= 0x80;
        stone_random_count += rand8;
        if (stone_random_count >= 0x40) stone_random_count = 0;
        random_value = stone_random_data[stone_random_count];
        (*(struct city_cell *)((unsigned char *)city_map + (sptr))).building = random_value;
        (*(struct city_cell *)((unsigned char *)city_map + (sptr))).fire = (random_value / 4 + 8);
        set_sound("fire.wav", 1);
    } else if (had_clear_sound == 0 || mouse_left_button == 0) {
        set_sound("smrub.wav", 1);
        if (mouse_left_button != 0) had_clear_sound = 1;
        else had_clear_sound = 0;
    }
    particles_cleared++;
}

// Clear a city cell to a randomly varied empty-terrain tile.
// FUNCTION: C2 0x69f41
// FUNCTION: C2WIN 0x004a6183
void clear_to_empty(int sptr)
{
    stone_random_count++;
    if (stone_random_count >= 0x40)
        stone_random_count = 0;
    if ((*(struct city_cell *)((unsigned char *)city_map + (sptr))).base_kind < 0x1a
            || (*(struct city_cell *)((unsigned char *)city_map + (sptr))).base_kind > 0x1d)
        particles_cleared++;
    (*(struct city_cell *)((unsigned char *)city_map + (sptr))).base_kind = ((stone_random_data[stone_random_count] >> 2) + 0x1a);
    clear_basic(sptr);
}

// Reset a city-map cell to its clean basic-terrain state.
// FUNCTION: C2 0x69f9e
// FUNCTION: C2WIN 0x004a6203
void clear_basic(int sptr)
{
    if (((*(struct city_cell *)((unsigned char *)city_map + ((sptr)))).terrain & 0x10) && ((*(struct city_cell *)((unsigned char *)city_map + ((sptr)))).terrain & 0x20)) {
        (*(struct city_cell *)((unsigned char *)city_map + ((sptr)))).base_kind = (*(struct city_cell *)((unsigned char *)city_map + ((sptr)))).building;
    }
    (*(struct city_cell *)((unsigned char *)city_map + ((sptr)))).building = 0;
    (*(struct city_cell *)((unsigned char *)city_map + ((sptr)))).fire = 0;
    (*(struct city_cell *)((unsigned char *)city_map + ((sptr)))).terrain &= 0x18;
    (*(struct city_cell *)((unsigned char *)city_map + ((sptr)))).edge_bits &= 2;
    (*(struct city_cell *)((unsigned char *)city_map + ((sptr)))).range_flag &= 0xfc;
    (*(struct city_cell *)((unsigned char *)city_map + ((sptr)))).fpu_flag &= 0xc0;
    (*(struct city_cell *)((unsigned char *)city_map + ((sptr)))).activity_a = 0;
    (*(struct city_cell *)((unsigned char *)city_map + ((sptr)))).activity_b = 0;
    (*(struct city_cell *)((unsigned char *)city_map + ((sptr)))).edge_bits |= 1;
    (*(struct city_cell *)((unsigned char *)city_map + ((sptr)))).building = 0;
    (*(struct city_cell *)((unsigned char *)city_map + ((sptr)))).business = 0;
}

// Clear every cell in a sized regional building footprint.
// FUNCTION: C2 0x6a018
// FUNCTION: C2WIN 0x004a633c
void clear_sized_to_reg_basic(int rm_offset, int size)
{
    int x;
    int y;

    if (((*(struct region_cell *)((unsigned char *)region_map + (rm_offset))).base_kind) == 0xd4)
        y = x = 0;
    else
        y = x = (*(struct region_cell *)((unsigned char *)region_map + (rm_offset))).occupant & 3;
    y = y % size;
    x = x / size;
    ofset_x = y; ofset_y = x;
    rm_offset -= y * 8;
    rm_offset -= x * 60 * 8;
    for (y = 0; y < size; y++, rm_offset += (60 - size) * 8)
        for (x = 0; x < size; x++, rm_offset += 8)
            clear_reg_basic(rm_offset);
}

// Reset a region cell to terrain appropriate for its underlying land type.
// FUNCTION: C2 0x6a0a6
// FUNCTION: C2WIN 0x004a6430
void clear_reg_basic(int rm_offset)
{
    if ((*(struct region_cell *)((unsigned char *)region_map + (rm_offset))).terrain & 0x40) {
        (*(struct region_cell *)((unsigned char *)region_map + (rm_offset))).base_kind = (char)((stone_random_data[stone_random_count]) / 4 + 0x18);
    } else if ((*(struct region_cell *)((unsigned char *)region_map + (rm_offset))).terrain & 0x80) {
        (*(struct region_cell *)((unsigned char *)region_map + (rm_offset))).base_kind = (char)((stone_random_data[stone_random_count]) / 4 + 0x1c);
    } else {
        (*(struct region_cell *)((unsigned char *)region_map + (rm_offset))).base_kind = (char)((stone_random_data[stone_random_count]) / 4 + 0x10);
    }
    if ((*(struct region_cell *)((unsigned char *)region_map + (rm_offset))).terrain & 1) {
        (*(struct region_cell *)((unsigned char *)region_map + (rm_offset))).occupant = 0;
    }
    (*(struct region_cell *)((unsigned char *)region_map + (rm_offset))).terrain   &= 0xd8;
    (*(struct region_cell *)((unsigned char *)region_map + (rm_offset))).edge_bits &= 2;
    (*(struct region_cell *)((unsigned char *)region_map + (rm_offset)))._unused05  = 0;
    (*(struct region_cell *)((unsigned char *)region_map + (rm_offset))).outside    = 0;
    (*(struct region_cell *)((unsigned char *)region_map + (rm_offset))).edge_bits |= 1;
}

// Mark a city cell as plague-stricken and initialize its plague appearance.
// FUNCTION: C2 0x6a17a
// FUNCTION: C2WIN 0x004a6555
void plague_it(int sptr)
{
    (*(struct city_cell *)((unsigned char *)city_map + (sptr))).fpu_flag &= 0xc0;
    (*(struct city_cell *)((unsigned char *)city_map + (sptr))).edge_bits |= 0x81;
    stone_random_count = stone_random_count + (char)rand8;
    if (stone_random_count >= 0x40)
        stone_random_count = 0;
    (*(struct city_cell *)((unsigned char *)city_map + (sptr))).building = stone_random_data[stone_random_count];
    (*(struct city_cell *)((unsigned char *)city_map + (sptr))).fire = 0x0a;
}

// Fill the legal cells in a city-map rectangle with the selected construction.
// FUNCTION: C2 0x6a1cb
// FUNCTION: C2WIN 0x004a6605
void build_an_area(int x1, int y1, int x2, int y2,
                   int base_kind, int edge_bits, int color)
{
    int row_skip;
    int x;
    int y;
    int swap_value;
    unsigned char old_kind;
    unsigned char building_kind_byte;

    building_kind_byte = (unsigned char)base_kind;
    if (x1 > x2) { swap_value = x2; x2 = x1; x1 = swap_value; }
    if (y1 > y2) { swap_value = y2; y2 = y1; y1 = swap_value; }

    cm_sptr = (x1 + y1 * CITY_W) * CITY_CELL_BYTES;
    row_skip = (CITY_W - (x2 - x1) - 1) * CITY_CELL_BYTES;
    for (y = y1; y <= y2; y++, cm_sptr += row_skip) {
        for (x = x1; x <= x2; x++, cm_sptr += CITY_CELL_BYTES) {
            CM_CELL(cm_sptr).edge_bits |= 1;
            if ((CM_CELL(cm_sptr).terrain & 0x10) == 0 &&
                (CM_CELL(cm_sptr).terrain & 0xe7) == 0 &&
                (CM_CELL(cm_sptr).base_kind >= 8 ||
                 (CM_CELL(cm_sptr).edge_bits & 0x80) == 0) &&
                CM_CELL(cm_sptr).citizen_a == 0 &&
                CM_CELL(cm_sptr).citizen_b == 0) {
                old_kind = CM_CELL(cm_sptr).base_kind;
                particles_built++;
                if (old_kind < 0x1a) particles_cleared++;
                CM_CELL(cm_sptr).base_kind = building_kind_byte;
                CM_CELL(cm_sptr).terrain |= placing_flags;
                CM_CELL(cm_sptr).extra_edge = color;
                CM_CELL(cm_sptr).edge_bits &= 0xe3;
                CM_CELL(cm_sptr).edge_bits |= edge_bits;
                CM_CELL(cm_sptr).activity_a = 0;
            }
        }
    }
    if (particles_built == 0) illegal_build = 1;
}

// Validate and place a single-cell city building.
// FUNCTION: C2 0x6a341
// FUNCTION: C2WIN 0x004a683b
int put_x1_area(int x, int y, char base_kind, int edge_bits, int color)
{

    start_sptr = (x + y * 80) * 20;
    start_x_pos = x;
    start_y_pos = y;
    cm_sptr = start_sptr;

    if (((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).terrain & 0x10) != 0) {
    illegal:
        illegal_build = 1;
        return 0;
    }
    if (((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).terrain & 0xe7) != 0) goto illegal;
    if ((unsigned char)(*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind < 8 && ((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).edge_bits & 0x80) != 0) goto illegal;
    if ((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).citizen_a != 0) goto illegal;
    if ((*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).citizen_b != 0) goto illegal;

    particles_built++;
    if ((unsigned char)(*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind < 0x1a) particles_cleared++;
    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).edge_bits |= 1;
    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).base_kind = base_kind;
    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).terrain |= placing_flags;
    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).extra_edge = (char)color;
    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).edge_bits &= 0xe3;
    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).edge_bits |= (char)edge_bits;
    (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).activity_a = 0;
    return 1;
}

// Validate and place a directionally anchored 2x2 city building.
// FUNCTION: C2 0x6a43f
// FUNCTION: C2WIN 0x004a6a13
int put_x2_area(int x, int y, char base_kind, int edge_bits, int color)
{
    int row_skip;
    int xi;
    int yi;
    int footprint_idx;
    int bad = 0;
    row_skip = (80 - 2) * 20;

    if (map_direction == 2) x -= 1;
    if (map_direction == 6) y -= 1;
    if (map_direction == 4) {
        x -= 1;
        y -= 1;
    }
    if (x < 0) {
    illegal:
        illegal_build = 1;
        return 0;
    }
    if (y < 0) goto illegal;
    if (x + 1 >= 80) goto illegal;
    if (y + 1 >= 80) goto illegal;

    start_x_pos = x;
    start_y_pos = y;
    start_sptr = (x + y * 80) * 20;
    cm_sptr = start_sptr;

    for (yi = y; yi < y + 2; ) {
        for (xi = x; xi < x + 2; ) {
            if (((*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).terrain   & 0x10) != 0) bad = 1;
            if (((*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).terrain   & 0xe7) != 0) bad = 1;
            if ((unsigned char)(*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).base_kind < 8 &&
                ((*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).edge_bits & 0x80) != 0) bad = 1;
            if ((*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).citizen_a != 0) bad = 1;
            if ((*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).citizen_b != 0) bad = 1;
            (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).edge_bits |= 1;
            xi++;
            cm_sptr += 20;
        }
        yi++;
        cm_sptr += row_skip;
    }
    if (bad) goto illegal;

    cm_sptr = start_sptr;
    footprint_idx = 0;
    for (yi = y; yi < y + 2; ) {
        for (xi = x; xi < x + 2; ) {
            if ((unsigned char)(*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).base_kind < 0x1a)
                particles_cleared++;
            (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).base_kind        = base_kind;
            (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).terrain    |= placing_flags;
            (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).extra_edge  = (char)(color + diamond_ofsets_2x[footprint_idx]);
            (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).edge_bits  &= 0xe3;
            (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).edge_bits  |= (char)edge_bits;
            (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).activity_a  = footprint_idx;
            if (map_direction == 2 || map_direction == 4)
                (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).activity_b = 0x20;
            xi++;
            cm_sptr += 20;
            footprint_idx++;
        }
        yi++;
        cm_sptr += row_skip;
    }
    particles_built++;
    return 1;
}

// Validate and place a directionally anchored 3x3 city building.
// FUNCTION: C2 0x6a669
// FUNCTION: C2WIN 0x004a6d79
int put_x3_area(int x, int y, char base_kind, int edge_bits, int color)
{
    int row_skip;
    int xi;
    int yi;
    int footprint_idx;
    int bad = 0;
    row_skip = (80 - 3) * 20;

    if (map_direction == 2) x -= 2;
    if (map_direction == 6) y -= 2;
    if (map_direction == 4) {
        x -= 2;
        y -= 2;
    }
    if (x < 0) {
    illegal:
        illegal_build = 1;
        return 0;
    }
    if (y < 0) goto illegal;
    if (x + 2 >= 80) goto illegal;
    if (y + 2 >= 80) goto illegal;

    start_x_pos = x;
    start_y_pos = y;
    start_sptr = (x + y * 80) * 20;
    cm_sptr = start_sptr;

    for (yi = y; yi < y + 3; ) {
        for (xi = x; xi < x + 3; ) {
            if (((*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).terrain   & 0x10) != 0) bad = 1;
            if (((*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).terrain   & 0xe7) != 0) bad = 1;
            if ((unsigned char)(*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).base_kind < 8 &&
                ((*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).edge_bits & 0x80) != 0) bad = 1;
            if ((*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).citizen_a != 0) bad = 1;
            if ((*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).citizen_b != 0) bad = 1;
            (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).edge_bits |= 1;
            xi++;
            cm_sptr += 20;
        }
        yi++;
        cm_sptr += row_skip;
    }
    if (bad) goto illegal;

    cm_sptr = start_sptr;
    footprint_idx = 0;
    for (yi = y; yi < y + 3; ) {
        for (xi = x; xi < x + 3; ) {
            if ((unsigned char)(*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).base_kind < 0x1a)
                particles_cleared++;
            (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).base_kind        = base_kind;
            (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).terrain    |= placing_flags;
            (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).extra_edge  = (char)(color + diamond_ofsets_3x[footprint_idx]);
            (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).edge_bits  &= 0xe3;
            (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).edge_bits  |= (char)edge_bits;
            (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).activity_a  = footprint_idx;
            if (map_direction == 2 || map_direction == 4)
                (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).activity_b = 0x20;
            xi++;
            cm_sptr += 20;
            footprint_idx++;
        }
        yi++;
        cm_sptr += row_skip;
    }
    particles_built++;
    return 1;
}

// Validate and place a directionally anchored 4x4 city building.
// FUNCTION: C2 0x6a889
// FUNCTION: C2WIN 0x004a70e7
int put_x4_area(int x, int y, char base_kind, int edge_bits, int color)
{
    int row_skip;
    int xi;
    int yi;
    int footprint_idx;
    int bad = 0;
    row_skip = (80 - 4) * 20;

    if (map_direction == 2) x -= 3;
    if (map_direction == 6) y -= 3;
    if (map_direction == 4) {
        x -= 3;
        y -= 3;
    }
    if (x < 0) {
    illegal:
        illegal_build = 1;
        return 0;
    }
    if (y < 0) goto illegal;
    if (x + 3 >= 80) goto illegal;
    if (y + 3 >= 80) goto illegal;

    start_x_pos = x;
    start_y_pos = y;
    start_sptr = (x + y * 80) * 20;
    cm_sptr = start_sptr;

    for (yi = y; yi < y + 4; ) {
        for (xi = x; xi < x + 4; ) {
            if (((*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).terrain   & 0x10) != 0) bad = 1;
            if (((*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).terrain   & 0xe7) != 0) bad = 1;
            if ((unsigned char)(*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).base_kind < 8 &&
                ((*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).edge_bits & 0x80) != 0) bad = 1;
            if ((*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).citizen_a != 0) bad = 1;
            if ((*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).citizen_b != 0) bad = 1;
            (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).edge_bits |= 1;
            xi++;
            cm_sptr += 20;
        }
        yi++;
        cm_sptr += row_skip;
    }
    if (bad) goto illegal;

    cm_sptr = start_sptr;
    footprint_idx = 0;
    for (yi = y; yi < y + 4; ) {
        for (xi = x; xi < x + 4; ) {
            if ((unsigned char)(*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).base_kind < 0x1a)
                particles_cleared++;
            (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).base_kind        = base_kind;
            (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).terrain    |= placing_flags;
            (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).extra_edge  = (char)(color + diamond_ofsets_4x[footprint_idx]);
            (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).edge_bits  &= 0xe3;
            (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).edge_bits  |= (char)edge_bits;
            (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).activity_a  = footprint_idx;
            if (map_direction == 2 || map_direction == 4)
                (*(struct city_cell *)((unsigned char *)city_map + ((cm_sptr)))).activity_b = 0x20;
            xi++;
            cm_sptr += 20;
            footprint_idx++;
        }
        yi++;
        cm_sptr += row_skip;
    }
    particles_built++;
    return 1;
}

// Replace a city building footprint with a new kind and appearance.
// FUNCTION: C2 0x6aaab
// FUNCTION: C2WIN 0x004a7455
void change_sized(int base_kind, int gfx_base_idx, int footprint_size, int cell_offset)
{
    int xi;
    int yi;
    int footprint_idx;
    int row_step = (80 - footprint_size) * 20;

    for (yi = 0, footprint_idx = 0; yi < footprint_size; ) {
        for (xi = 0; xi < footprint_size; ) {
            (*(struct city_cell *)((unsigned char *)city_map + (cell_offset))).base_kind = base_kind;
            (*(struct city_cell *)((unsigned char *)city_map + (cell_offset))).edge_bits |= 1;
            if (footprint_size == 1) {
                (*(struct city_cell *)((unsigned char *)city_map + (cell_offset))).extra_edge = gfx_base_idx;
            } else if (footprint_size == 2) {
                (*(struct city_cell *)((unsigned char *)city_map + (cell_offset))).extra_edge =
                    gfx_base_idx + diamond_ofsets_2x[footprint_idx];
            } else if (footprint_size == 3) {
                (*(struct city_cell *)((unsigned char *)city_map + (cell_offset))).extra_edge =
                    gfx_base_idx + diamond_ofsets_3x[footprint_idx];
            } else if (footprint_size == 4) {
                (*(struct city_cell *)((unsigned char *)city_map + (cell_offset))).extra_edge =
                    gfx_base_idx + diamond_ofsets_4x[footprint_idx];
            }
            xi++;
            cell_offset += 20;
            footprint_idx++;
        }
        yi++;
        cell_offset += row_step;
    }
}

// Mark the directionally anchored city-map footprint used by a placement preview.
// FUNCTION: C2 0x6ab34
// FUNCTION: C2WIN 0x004a7573
void set_map_ref(int x, int y, int size)
{
    int row_skip;
    int sptr;
    int xi;
    int yi;
    int size_minus_one;

    row_skip = (80 - size) * 20;
    if (map_direction == 2)
        x -= size - 1;
    if (map_direction == 6)
        y -= size - 1;
    if (map_direction == 4) {
        x -= size - 1;
        y -= size - 1;
    }
    if (x < 0) return;
    if (y < 0) return;
    size_minus_one = size - 1;
    if (x + size_minus_one >= 80) return;
    if (y + size_minus_one >= 80) return;

    start_x_pos = x;
    start_y_pos = y;
    sptr = (x + y * 80) * 20;
    start_sptr = sptr;
    cm_sptr = sptr;
    for (yi = y; yi < y + size; yi++, cm_sptr += row_skip) {
        for (xi = x; xi < x + size; xi++, cm_sptr += 20) {
            (*(struct city_cell *)((unsigned char *)city_map + (cm_sptr))).edge_bits |= 1;
        }
    }
}

// Validate and place a single-cell regional building.
// FUNCTION: C2 0x6ac09
// FUNCTION: C2WIN 0x004a76d5
int put_reg_x1_area(int x, int y, unsigned char base_kind, int edge_bits,
                    int color, int strict_flags)
{
    start_x_pos = x; start_y_pos = y;
    x = x + y * 60; start_sptr = x * 8;
    cm_sptr = start_sptr;

    if (strict_flags == 1) { if (((*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).terrain & 0x3f) != 0) return 0; }
    else { if (((*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).terrain & 0xff) != 0) return 0; }
    if ((*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).occupant != 0) return 0;

    particles_built++;
    if ((*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).base_kind < 0x10) particles_cleared++;
    (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).edge_bits |= 1;
    (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).base_kind = (unsigned char)base_kind;
    (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).terrain |= reg_placing_flags;
    (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).gfx = (unsigned char)color;
    (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).edge_bits &= 0xe3;
    (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).edge_bits |= (unsigned char)edge_bits;
    (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).edge_bits &= 0xbf;
    (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).occupant = 0;
    return 1;
}

// Validate and place a directionally anchored 2x2 regional building.
// FUNCTION: C2 0x6acd1
// FUNCTION: C2WIN 0x004a7852
int put_reg_x2_area(int x, int y, unsigned char base_kind, int edge_bits,
                    int color, int strict_flags)
{
    int xi;
    int yi;
    int footprint_idx;
    int cell_offset;
    int bad;
    int row_skip;

    bad = 0;
    row_skip = (60 - 2) * 8;

    if (map_direction == 2) x--;
    if (map_direction == 6) y--;
    if (map_direction == 4) { x--; y--; }
    if (x < 0) return 0;
    if (y < 0) return 0;
    if (x + 1 >= 60) return 0;
    if (y + 1 >= 60) return 0;

    start_x_pos = x;
    start_y_pos = y;
    cell_offset = ((x) + (y) * 60) * 8;
    start_sptr = cell_offset;
    cm_sptr = cell_offset;

    for (yi = y; yi < y + 2; yi++, cm_sptr += row_skip) {
        for (xi = x; xi < x + 2; xi++, cm_sptr += 8) {
            if (strict_flags == 1) {
                if (((*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).terrain & 0x3f) != 0) bad = 1;
            } else {
                if ((int)(*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).terrain != 0) bad = 1;
            }
            if ((*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).occupant != 0) bad = 1;
            (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).edge_bits |= 1;
        }
    }
    if (bad) return 0;

    cm_sptr = start_sptr;
    footprint_idx = 0;
    for (yi = y; yi < y + 2; yi++, cm_sptr += row_skip) {
        for (xi = x; xi < x + 2; xi++, cm_sptr += 8, footprint_idx++) {
            if ((*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).base_kind < 0x10) particles_cleared++;
            (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).base_kind = (unsigned char)base_kind;
            (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).terrain |= reg_placing_flags;
            (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).gfx = color + diamond_ofsets_2x[footprint_idx];
            (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).edge_bits &= 0xe3;
            (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).edge_bits |= (unsigned char)edge_bits;
            (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).edge_bits &= 0xbf;
            (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).occupant = (unsigned char)footprint_idx;
        }
    }
    particles_built++;
    return 1;
}

// Replace a regional building footprint with a new kind and appearance.
// FUNCTION: C2 0x6aeac
// FUNCTION: C2WIN 0x004a7b37
void change_reg_sized(int base_kind, int gfx_base_idx, int footprint_size, int cell_offset)
{
    int xi;
    int yi;
    int footprint_idx;
    int row_step = (60 - footprint_size) * 8;

    for (yi = 0, footprint_idx = 0; yi < footprint_size; ) {
        for (xi = 0; xi < footprint_size; ) {
            (*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).base_kind = base_kind;
            (*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).edge_bits |= 1;
            if (footprint_size == 1) {
                (*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).gfx = gfx_base_idx;
            } else if (footprint_size == 2) {
                (*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).gfx =
                    gfx_base_idx + diamond_ofsets_2x[footprint_idx];
            }
            xi++;
            cell_offset += 8;
            footprint_idx++;
        }
        yi++;
        cell_offset += row_step;
    }
}

// Validate and place an arbitrary square regional building.
// FUNCTION: C2 0x6af11
// FUNCTION: C2WIN 0x004a7c06
int put_rm_area(int x, int y, int footprint_size, unsigned char base_kind,
                int edge_bits, int color, int terrain_flags)
{
    int row_skip;
    int cell_offset, xi, yi, footprint_idx;
    unsigned char gfx_base_idx = color;


    row_skip = (60 - footprint_size) * 8;

    if (map_direction == 2) x -= footprint_size - 1;
    if (map_direction == 6) y -= footprint_size - 1;
    if (map_direction == 4) { x -= footprint_size - 1; y -= footprint_size - 1; }
    if (x < 0) return 0;
    if (y < 0) return 0;

    cell_offset = (x + y * 60) * 8;
    for (yi = y; yi < y + footprint_size; yi++, cell_offset += row_skip) {
        for (xi = x; xi < x + footprint_size; xi++, cell_offset += 8) {

            if ((*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).occupant != 0) return 0;
            if (((*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).terrain & 0x10) != 0) return 0;
            if (((*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).terrain & 1) != 0) return 0;
            (*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).edge_bits |= 1;
        }
    }
    cell_offset = (x + y * 60) * 8;
    for (yi = y, footprint_idx = 0; yi < y + footprint_size; yi++, cell_offset += row_skip) {
        for (xi = x; xi < x + footprint_size; xi++, cell_offset += 8, footprint_idx++) {

            (*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).base_kind = base_kind;
            (*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).terrain |= terrain_flags;
            (*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).edge_bits &= 0xe3;
            (*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).edge_bits |= edge_bits;
            if (footprint_size == 1) (*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).gfx = gfx_base_idx;
            else if (footprint_size == 2) (*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).gfx = gfx_base_idx + diamond_ofsets_2x[footprint_idx];
            else if (footprint_size == 3) (*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).gfx = gfx_base_idx + diamond_ofsets_3x[footprint_idx];
            else if (footprint_size == 4) (*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).gfx = gfx_base_idx + diamond_ofsets_4x[footprint_idx];
        }
    }
    return 1;
}

// Set terrain flags across a directionally anchored regional footprint.
// FUNCTION: C2 0x6b08c
// FUNCTION: C2WIN 0x004a7ec5
void flag_rm_area(int x, int y, int size, char mask_byte)
{
    int rowadd = (60 - size) * 8;
    int offset;
    int i;
    int j;

    if (map_direction == 2)
        x -= size - 1;
    if (map_direction == 6)
        y -= size - 1;
    if (map_direction == 4) {
        x -= size - 1;
        y -= size - 1;
    }
    if (x < 0) return;
    if (y < 0) return;

    offset = ((x) + (y) * 60) * 8;
    for (j = y; j < y + size; j++, offset += rowadd) {
        for (i = x; i < x + size; i++, offset += 8) {
            (*(struct region_cell *)((unsigned char *)region_map + (offset))).terrain |= mask_byte;
        }
    }
}

// Mask terrain flags across a regional footprint.
// FUNCTION: C2 0x6b126
// FUNCTION: C2WIN 0x004a7fd4
void unflag_rm_area(int x, int y, int size, unsigned char mask_byte)
{
    int rowadd = (60 - size) * 8;
    int offset;
    int i;
    int j;

    if (x < 0) return;
    if (y < 0) return;

    offset = ((x) + (y) * 60) * 8;

    for (j = y; j < y + size; j++, offset += rowadd) {
        for (i = x; i < x + size; i++, offset += 8) {
            (*(struct region_cell *)((unsigned char *)region_map + (offset))).terrain &= mask_byte;
        }
    }
}

// Check whether a 2x2 regional industry footprint has the required terrain.
// FUNCTION: C2 0x6b18c
// FUNCTION: C2WIN 0x004a808c
void check_region_map_for_farm_square(int x, int y, char mask)
{
    int count;
    int xi;
    int yi;
    int row_skip;

    industry_build_ok = 1;
    count = 0;
    row_skip = (60 - 2) * 8;
    if (map_direction == 2) x--;
    if (map_direction == 6) y--;
    if (map_direction == 4) { x--; y--; }
    if (x < 0 || y < 0 || x + 1 >= 60 || y + 1 >= 60) return;

    cm_sptr = ((x) + (y) * 60) * 8;
    for (yi = y; yi < y + 2; yi++, cm_sptr += row_skip) {
        for (xi = x; xi < x + 2; xi++, cm_sptr += 8) {
            if (((*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).terrain & mask) != 0) count++;
        }
    }
    if (count == 4) industry_build_ok = 0;
    else illegal_build = 1;
}

// Check whether a 2x2 port footprint touches a suitable coastline.
// FUNCTION: C2 0x6b26d
// FUNCTION: C2WIN 0x004a81d2
void check_region_map_for_port_square(int x, int y)
{
    int row_skip;

    industry_build_ok = 1;
    row_skip = (60 - 2) * 8;
    if (map_direction == 2) x--;
    if (map_direction == 6) y--;
    if (map_direction == 4) { x--; y--; }
    if (x < 0 || y < 0 || x + 1 >= 60 || y + 1 >= 60) return;

    gmn_sptr = ((x) + (y) * 60) * 8;
    for (gmn_y = y; gmn_y < y + 2; gmn_y++, gmn_sptr += row_skip) {
        for (gmn_x = x; gmn_x < x + 2; gmn_x++, gmn_sptr += 8) {
            test_regionmap_neighbours_negedge(8);
            if (gmn_polar_count != 0) {
                industry_build_ok = 0;
                return;
            }
        }
    }
    illegal_build = 1;
}

// Rebuild coastline tiles and navigation flags in a regional rectangle.
// FUNCTION: C2 0x6b347
// FUNCTION: C2WIN 0x004a8311
void adjust_regions_coastline(int x, int y, int width, int height)
{
    int x0 = x;
    int y0 = y;
    int x1 = x + width;
    int y1 = y + height;
    int kind;

    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > 60) x1 = 60;
    if (y1 > 60) y1 = 60;

    init_choices(coast_data, 0x30);
    for (gmn_y = y0; gmn_y < y1; gmn_y++) {
        for (gmn_x = x0; gmn_x < x1; gmn_x++) {
            gmn_sptr = ((gmn_x) + (gmn_y) * 60) * 8;
            if (((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain & 8) != 0 &&
                ((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain & 1) == 0) {
                test_regionmap_neighbours_posedge(8);
                invert_gmn();
                if (choose_from(coast_data, 0x30) == 0)
                    high_beep();
                (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).base_kind = first_choice + choice_count - 0x10;
                (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).edge_bits |= 1;
                kind = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).base_kind & 0xff;
                if (kind < 0x7c) {
                    unsigned char sea = sailable_sea[kind - SAILABLE_SEA_FIRST_TILE];
                    if (sea)
                        (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain &= 0xef;
                    else
                        (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain |= 0x10;
                }
            }
        }
    }
}

// Update which regional coast cells are navigable.
// FUNCTION: C2 0x6b474
// FUNCTION: C2WIN 0x004a84fc
void adjust_sailable_area(void)
{
    unsigned char tile;
    unsigned char coast;

    gmn_y = 0;
    gmn_sptr = 0;
    for ( ; gmn_y < 0x3c; gmn_y++) {
    for (gmn_x = 0; gmn_x < 0x3c; gmn_x++, gmn_sptr += 8) {
    if (((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain & 8) != 0) {
        tile = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).base_kind;
        if (tile < 0x7c) {
            coast = sailable_sea[tile - SAILABLE_SEA_FIRST_TILE];
            if (coast != 0)
                (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain &= 0xef;
        }
    }
    }
    }
}

// Measure matching neighbours around the current city-map cell, treating map edges as occupied.
// FUNCTION: C2 0x6b4f3
// FUNCTION: C2WIN 0x004a85d6
void test_citymap_neighbours_posedge(char mask)
{
    int i;

    gmn_count = gmn_polar_count = gmn_density = 0;
    gmn_ns_count = gmn_ew_count = gmn_nesw_count = gmn_nwse_count = 0;
    gmn_run = gmn_max_run = 0;

    if (gmn_y == 0) { gmn[0] = 1; gmn_density = -1; }
    else gmn[0] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) - CITY_ROW))).terrain & mask;
    if (gmn[0]) { gmn_count++; gmn_polar_count++; gmn_ns_count++; gmn_density++; }

    if (gmn_y == 0 || gmn_x == 79) { gmn[1] = 1; gmn_nesw_count--; }
    else gmn[1] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) - CITY_ROW + CITY_CELL_BYTES))).terrain & mask;
    if (gmn[1]) { gmn_count++; gmn_nesw_count++; }

    if (gmn_x == 79) { gmn[2] = 1; gmn_density--; }
    else gmn[2] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) + CITY_CELL_BYTES))).terrain & mask;
    if (gmn[2]) { gmn_count++; gmn_polar_count++; gmn_ew_count++; gmn_density++; }

    if (gmn_x == 79 || gmn_y == 79) { gmn[3] = 1; gmn_nwse_count--; }
    else gmn[3] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) + CITY_ROW + CITY_CELL_BYTES))).terrain & mask;
    if (gmn[3]) { gmn_count++; gmn_nwse_count++; }

    if (gmn_y == 79) { gmn[4] = 1; gmn_density--; }
    else gmn[4] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) + CITY_ROW))).terrain & mask;
    if (gmn[4]) { gmn_count++; gmn_polar_count++; gmn_ns_count++; gmn_density++; }

    if (gmn_x == 0 || gmn_y == 79) { gmn[5] = 1; gmn_nesw_count--; }
    else gmn[5] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) + CITY_ROW - CITY_CELL_BYTES))).terrain & mask;
    if (gmn[5]) { gmn_count++; gmn_nesw_count++; }

    if (gmn_x == 0) { gmn[6] = 1; gmn_density--; }
    else gmn[6] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) - CITY_CELL_BYTES))).terrain & mask;
    if (gmn[6]) { gmn_count++; gmn_polar_count++; gmn_ew_count++; gmn_density++; }

    if (gmn_x == 0 || gmn_y == 0) { gmn[7] = 1; gmn_nwse_count--; }
    else gmn[7] = mask & (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) - CITY_ROW - CITY_CELL_BYTES))).terrain;
    if (gmn[7]) { gmn_count++; gmn_nwse_count++; }

    gmn[8] = gmn[0];
    gmn[9] = gmn[1];
    gmn[10] = gmn[2];
    gmn[11] = gmn[3];
    gmn[12] = gmn[4];
    gmn[13] = gmn[5];
    gmn[14] = gmn[6];
    gmn[15] = gmn[7];
    for (i = 0; i < 16; i++) {
        if (gmn[i]) gmn_run++;
        else gmn_run = 0;
        if (gmn_run > gmn_max_run) gmn_max_run = gmn_run;
    }
}

// Measure matching neighbours around the current city-map cell, treating map edges as empty.
// FUNCTION: C2 0x6b814
// FUNCTION: C2WIN 0x004a89f2
void test_citymap_neighbours_negedge(char mask)
{
    int i;

    gmn_count = gmn_polar_count = gmn_density = 0;
    gmn_ns_count = gmn_ew_count = gmn_nesw_count = gmn_nwse_count = 0;
    gmn_run = gmn_max_run = 0;

    if (gmn_y == 0) gmn[0] = 0;
    else gmn[0] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) - CITY_ROW))).terrain & mask;
    if (gmn[0]) { gmn_count++; gmn_polar_count++; gmn_ns_count++; gmn_density++; }

    if (gmn_y == 0 || gmn_x == 79) gmn[1] = 0;
    else gmn[1] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) - CITY_ROW + CITY_CELL_BYTES))).terrain & mask;
    if (gmn[1]) { gmn_count++; gmn_nesw_count++; }

    if (gmn_x == 79) gmn[2] = 0;
    else gmn[2] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) + CITY_CELL_BYTES))).terrain & mask;
    if (gmn[2]) { gmn_count++; gmn_polar_count++; gmn_ew_count++; gmn_density++; }

    if (gmn_x == 79 || gmn_y == 79) gmn[3] = 0;
    else gmn[3] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) + CITY_ROW + CITY_CELL_BYTES))).terrain & mask;
    if (gmn[3]) { gmn_count++; gmn_nwse_count++; }

    if (gmn_y == 79) gmn[4] = 0;
    else gmn[4] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) + CITY_ROW))).terrain & mask;
    if (gmn[4]) { gmn_count++; gmn_polar_count++; gmn_ns_count++; gmn_density++; }

    if (gmn_x == 0 || gmn_y == 79) gmn[5] = 0;
    else gmn[5] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) + CITY_ROW - CITY_CELL_BYTES))).terrain & mask;
    if (gmn[5]) { gmn_count++; gmn_nesw_count++; }

    if (gmn_x == 0) gmn[6] = 0;
    else gmn[6] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) - CITY_CELL_BYTES))).terrain & mask;
    if (gmn[6]) { gmn_count++; gmn_polar_count++; gmn_ew_count++; gmn_density++; }

    if (gmn_x == 0 || gmn_y == 0) gmn[7] = 0;
    else gmn[7] = mask & (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) - CITY_ROW - CITY_CELL_BYTES))).terrain;
    if (gmn[7]) { gmn_count++; gmn_nwse_count++; }

    gmn[8] = gmn[0];
    gmn[9] = gmn[1];
    gmn[10] = gmn[2];
    gmn[11] = gmn[3];
    gmn[12] = gmn[4];
    gmn[13] = gmn[5];
    gmn[14] = gmn[6];
    gmn[15] = gmn[7];
    for (i = 0; i < 16; i++) {
        if (gmn[i]) gmn_run++;
        else gmn_run = 0;
        if (gmn_run > gmn_max_run) gmn_max_run = gmn_run;
    }
}

// Measure neighbours of a specific type around a city cell, treating map edges as occupied.
// FUNCTION: C2 0x6bb01
// FUNCTION: C2WIN 0x004a8dde
void test_type_citymap_neighbours_posedge(unsigned char type)
{
    int i;

    gmn_count = gmn_polar_count = gmn_density = 0;
    gmn_ns_count = gmn_ew_count = gmn_nesw_count = gmn_nwse_count = 0;
    gmn_run = gmn_max_run = 0;

    if (gmn_y == 0) { gmn[0] = 0; gmn_density = -1; }
    else gmn[0] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) - CITY_ROW))).base_kind ^ (unsigned char)type;
    if (gmn[0] == 0) { gmn[0] = 1; gmn_count++; gmn_polar_count++; gmn_ns_count++; gmn_density++; }
    else gmn[0] = 0;

    if (gmn_y == 0 || gmn_x == 79) { gmn[1] = 0; gmn_nesw_count--; }
    else gmn[1] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) - CITY_ROW + CITY_CELL_BYTES))).base_kind ^ (unsigned char)type;
    if (gmn[1] == 0) { gmn[1] = 1; gmn_count++; gmn_nesw_count++; }
    else gmn[1] = 0;

    if (gmn_x == 79) { gmn[2] = 0; gmn_density--; }
    else gmn[2] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) + CITY_CELL_BYTES))).base_kind ^ (unsigned char)type;
    if (gmn[2] == 0) { gmn[2] = 1; gmn_count++; gmn_polar_count++; gmn_ew_count++; gmn_density++; }
    else gmn[2] = 0;

    if (gmn_x == 79 || gmn_y == 79) { gmn[3] = 0; gmn_nwse_count--; }
    else gmn[3] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) + CITY_ROW + CITY_CELL_BYTES))).base_kind ^ (unsigned char)type;
    if (gmn[3] == 0) { gmn[3] = 1; gmn_count++; gmn_nwse_count++; }
    else gmn[3] = 0;

    if (gmn_y == 79) { gmn[4] = 0; gmn_density--; }
    else gmn[4] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) + CITY_ROW))).base_kind ^ (unsigned char)type;
    if (gmn[4] == 0) { gmn[4] = 1; gmn_count++; gmn_polar_count++; gmn_ns_count++; gmn_density++; }
    else gmn[4] = 0;

    if (gmn_x == 0 || gmn_y == 79) { gmn[5] = 0; gmn_nesw_count--; }
    else gmn[5] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) + CITY_ROW - CITY_CELL_BYTES))).base_kind ^ (unsigned char)type;
    if (gmn[5] == 0) { gmn[5] = 1; gmn_count++; gmn_nesw_count++; }
    else gmn[5] = 0;

    if (gmn_x == 0) { gmn[6] = 0; gmn_density--; }
    else gmn[6] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) - CITY_CELL_BYTES))).base_kind ^ (unsigned char)type;
    if (gmn[6] == 0) { gmn[6] = 1; gmn_count++; gmn_polar_count++; gmn_ew_count++; gmn_density++; }
    else gmn[6] = 0;

    if (gmn_x == 0 || gmn_y == 0) { gmn[7] = 0; gmn_nwse_count--; }
    else gmn[7] = (unsigned char)type ^ (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) - CITY_ROW - CITY_CELL_BYTES))).base_kind;
    if (gmn[7] == 0) { gmn[7] = 1; gmn_count++; gmn_nwse_count++; }
    else gmn[7] = 0;

    gmn[8] = gmn[0];
    gmn[9] = gmn[1];
    gmn[10] = gmn[2];
    gmn[11] = gmn[3];
    gmn[12] = gmn[4];
    gmn[13] = gmn[5];
    gmn[14] = gmn[6];
    gmn[15] = gmn[7];
    for (i = 0; i < 16; i++) { if (gmn[i]) gmn_run++; else gmn_run = 0; if (gmn_run > gmn_max_run) gmn_max_run = gmn_run; }
}

// Measure neighbours of a specific type around a city cell, treating map edges as empty.
// FUNCTION: C2 0x6beb5
// FUNCTION: C2WIN 0x004a9272
void test_type_citymap_neighbours_negedge(unsigned char type)
{
    int i;

    gmn_count = gmn_polar_count = gmn_density = 0;
    gmn_ns_count = gmn_ew_count = gmn_nesw_count = gmn_nwse_count = 0;
    gmn_run = gmn_max_run = 0;

    if (gmn_y == 0) gmn[0] = 1;
    else gmn[0] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) - CITY_ROW))).base_kind ^ (unsigned char)type;
    if (gmn[0] == 0) { gmn[0] = 1; gmn_count++; gmn_polar_count++; gmn_ns_count++; gmn_density++; }
    else gmn[0] = 0;

    if (gmn_y == 0 || gmn_x == 79) gmn[1] = 1;
    else gmn[1] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) - CITY_ROW + CITY_CELL_BYTES))).base_kind ^ (unsigned char)type;
    if (gmn[1] == 0) { gmn[1] = 1; gmn_count++; gmn_nesw_count++; }
    else gmn[1] = 0;

    if (gmn_x == 79) gmn[2] = 1;
    else gmn[2] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) + CITY_CELL_BYTES))).base_kind ^ (unsigned char)type;
    if (gmn[2] == 0) { gmn[2] = 1; gmn_count++; gmn_polar_count++; gmn_ew_count++; gmn_density++; }
    else gmn[2] = 0;

    if (gmn_x == 79 || gmn_y == 79) gmn[3] = 1;
    else gmn[3] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) + CITY_ROW + CITY_CELL_BYTES))).base_kind ^ (unsigned char)type;
    if (gmn[3] == 0) { gmn[3] = 1; gmn_count++; gmn_nwse_count++; }
    else gmn[3] = 0;

    if (gmn_y == 79) gmn[4] = 1;
    else gmn[4] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) + CITY_ROW))).base_kind ^ (unsigned char)type;
    if (gmn[4] == 0) { gmn[4] = 1; gmn_count++; gmn_polar_count++; gmn_ns_count++; gmn_density++; }
    else gmn[4] = 0;

    if (gmn_x == 0 || gmn_y == 79) gmn[5] = 1;
    else gmn[5] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) + CITY_ROW - CITY_CELL_BYTES))).base_kind ^ (unsigned char)type;
    if (gmn[5] == 0) { gmn[5] = 1; gmn_count++; gmn_nesw_count++; }
    else gmn[5] = 0;

    if (gmn_x == 0) gmn[6] = 1;
    else gmn[6] = (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) - CITY_CELL_BYTES))).base_kind ^ (unsigned char)type;
    if (gmn[6] == 0) { gmn[6] = 1; gmn_count++; gmn_polar_count++; gmn_ew_count++; gmn_density++; }
    else gmn[6] = 0;

    if (gmn_x == 0 || gmn_y == 0) gmn[7] = 1;
    else gmn[7] = (unsigned char)type ^ (*(struct city_cell *)((unsigned char *)city_map + ((gmn_sptr) - CITY_ROW - CITY_CELL_BYTES))).base_kind;
    if (gmn[7] == 0) { gmn[7] = 1; gmn_count++; gmn_nwse_count++; }
    else gmn[7] = 0;

    gmn[8] = gmn[0];
    gmn[9] = gmn[1];
    gmn[10] = gmn[2];
    gmn[11] = gmn[3];
    gmn[12] = gmn[4];
    gmn[13] = gmn[5];
    gmn[14] = gmn[6];
    gmn[15] = gmn[7];
    for (i = 0; i < 16; i++) { if (gmn[i]) gmn_run++; else gmn_run = 0; if (gmn_run > gmn_max_run) gmn_max_run = gmn_run; }
}

// Measure matching neighbours around the current region cell, treating map edges as occupied.
// FUNCTION: C2 0x6c22e
// FUNCTION: C2WIN 0x004a96d6
void test_regionmap_neighbours_posedge(char mask)
{
    int i;

    gmn_count = gmn_polar_count = gmn_density = 0;
    gmn_ns_count = gmn_ew_count = gmn_nesw_count = gmn_nwse_count = 0;
    gmn_run = gmn_max_run = 0;

    if (gmn_y == 0) { gmn[0] = 1; gmn_density = -1; }
    else gmn[0] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr - 480))).terrain & mask;
    if (gmn[0]) { gmn_count++; gmn_polar_count++; gmn_ns_count++; gmn_density++; }

    if (gmn_y == 0 || gmn_x == 59) { gmn[1] = 1; gmn_nesw_count--; }
    else gmn[1] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr - 472))).terrain & mask;
    if (gmn[1]) { gmn_count++; gmn_nesw_count++; }

    if (gmn_x == 59) { gmn[2] = 1; gmn_density--; }
    else gmn[2] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr + 8))).terrain & mask;
    if (gmn[2]) { gmn_count++; gmn_polar_count++; gmn_ew_count++; gmn_density++; }

    if (gmn_x == 59 || gmn_y == 59) { gmn[3] = 1; gmn_nwse_count--; }
    else gmn[3] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr + 488))).terrain & mask;
    if (gmn[3]) { gmn_count++; gmn_nwse_count++; }

    if (gmn_y == 59) { gmn[4] = 1; gmn_density--; }
    else gmn[4] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr + 480))).terrain & mask;
    if (gmn[4]) { gmn_count++; gmn_polar_count++; gmn_ns_count++; gmn_density++; }

    if (gmn_x == 0 || gmn_y == 59) { gmn[5] = 1; gmn_nesw_count--; }
    else gmn[5] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr + 472))).terrain & mask;
    if (gmn[5]) { gmn_count++; gmn_nesw_count++; }

    if (gmn_x == 0) { gmn[6] = 1; gmn_density--; }
    else gmn[6] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr - 8))).terrain & mask;
    if (gmn[6]) { gmn_count++; gmn_polar_count++; gmn_ew_count++; gmn_density++; }

    if (gmn_x == 0 || gmn_y == 0) { gmn[7] = 1; gmn_nwse_count--; }
    else gmn[7] = mask & (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr - 488))).terrain;
    if (gmn[7]) { gmn_count++; gmn_nwse_count++; }

    gmn[8] = gmn[0];
    gmn[9] = gmn[1];
    gmn[10] = gmn[2];
    gmn[11] = gmn[3];
    gmn[12] = gmn[4];
    gmn[13] = gmn[5];
    gmn[14] = gmn[6];
    gmn[15] = gmn[7];
    for (i = 0; i < 16; i++) {
        if (gmn[i]) gmn_run++;
        else gmn_run = 0;
        if (gmn_run > gmn_max_run) gmn_max_run = gmn_run;
    }
}

// Measure matching neighbours around the current region cell, treating map edges as empty.
// FUNCTION: C2 0x6c54f
// FUNCTION: C2WIN 0x004a9af2
void test_regionmap_neighbours_negedge(char mask)
{
    int i;

    gmn_count = gmn_polar_count = gmn_density = 0;
    gmn_ns_count = gmn_ew_count = gmn_nesw_count = gmn_nwse_count = 0;
    gmn_run = gmn_max_run = 0;

    if (gmn_y == 0) gmn[0] = 0;
    else gmn[0] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr - 480))).terrain & mask;
    if (gmn[0]) { gmn_count++; gmn_polar_count++; gmn_ns_count++; gmn_density++; }

    if (gmn_y == 0 || gmn_x == 59) gmn[1] = 0;
    else gmn[1] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr - 472))).terrain & mask;
    if (gmn[1]) { gmn_count++; gmn_nesw_count++; }

    if (gmn_x == 59) gmn[2] = 0;
    else gmn[2] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr + 8))).terrain & mask;
    if (gmn[2]) { gmn_count++; gmn_polar_count++; gmn_ew_count++; gmn_density++; }

    if (gmn_x == 59 || gmn_y == 59) gmn[3] = 0;
    else gmn[3] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr + 488))).terrain & mask;
    if (gmn[3]) { gmn_count++; gmn_nwse_count++; }

    if (gmn_y == 59) gmn[4] = 0;
    else gmn[4] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr + 480))).terrain & mask;
    if (gmn[4]) { gmn_count++; gmn_polar_count++; gmn_ns_count++; gmn_density++; }

    if (gmn_x == 0 || gmn_y == 59) gmn[5] = 0;
    else gmn[5] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr + 472))).terrain & mask;
    if (gmn[5]) { gmn_count++; gmn_nesw_count++; }

    if (gmn_x == 0) gmn[6] = 0;
    else gmn[6] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr - 8))).terrain & mask;
    if (gmn[6]) { gmn_count++; gmn_polar_count++; gmn_ew_count++; gmn_density++; }

    if (gmn_x == 0 || gmn_y == 0) gmn[7] = 0;
    else gmn[7] = mask & (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr - 488))).terrain;
    if (gmn[7]) { gmn_count++; gmn_nwse_count++; }

    gmn[8] = gmn[0];
    gmn[9] = gmn[1];
    gmn[10] = gmn[2];
    gmn[11] = gmn[3];
    gmn[12] = gmn[4];
    gmn[13] = gmn[5];
    gmn[14] = gmn[6];
    gmn[15] = gmn[7];
    for (i = 0; i < 16; i++) {
        if (gmn[i]) gmn_run++;
        else gmn_run = 0;
        if (gmn_run > gmn_max_run) gmn_max_run = gmn_run;
    }
}

// Measure neighbours of a specific type around a region cell, treating map edges as occupied.
// FUNCTION: C2 0x6c83c
// FUNCTION: C2WIN 0x004a9ede
void test_type_regionmap_neighbours_posedge(unsigned char type)
{
    int i;

    gmn_count = gmn_polar_count = gmn_density = 0;
    gmn_ns_count = gmn_ew_count = gmn_nesw_count = gmn_nwse_count = 0;
    gmn_run = gmn_max_run = 0;

    if (gmn_y == 0) { gmn[0] = 0; gmn_density = -1; }
    else gmn[0] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr - 480))).base_kind ^ (unsigned char)type;
    if (gmn[0] == 0) { gmn[0] = 1; gmn_count++; gmn_polar_count++; gmn_ns_count++; gmn_density++; }
    else gmn[0] = 0;

    if (gmn_y == 0 || gmn_x == 59) { gmn[1] = 0; gmn_nesw_count--; }
    else gmn[1] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr - 472))).base_kind ^ (unsigned char)type;
    if (gmn[1] == 0) { gmn[1] = 1; gmn_count++; gmn_nesw_count++; }
    else gmn[1] = 0;

    if (gmn_x == 59) { gmn[2] = 0; gmn_density--; }
    else gmn[2] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr + 8))).base_kind ^ (unsigned char)type;
    if (gmn[2] == 0) { gmn[2] = 1; gmn_count++; gmn_polar_count++; gmn_ew_count++; gmn_density++; }
    else gmn[2] = 0;

    if (gmn_x == 59 || gmn_y == 59) { gmn[3] = 0; gmn_nwse_count--; }
    else gmn[3] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr + 488))).base_kind ^ (unsigned char)type;
    if (gmn[3] == 0) { gmn[3] = 1; gmn_count++; gmn_nwse_count++; }
    else gmn[3] = 0;

    if (gmn_y == 59) { gmn[4] = 0; gmn_density--; }
    else gmn[4] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr + 480))).base_kind ^ (unsigned char)type;
    if (gmn[4] == 0) { gmn[4] = 1; gmn_count++; gmn_polar_count++; gmn_ns_count++; gmn_density++; }
    else gmn[4] = 0;

    if (gmn_x == 0 || gmn_y == 59) { gmn[5] = 0; gmn_nesw_count--; }
    else gmn[5] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr + 472))).base_kind ^ (unsigned char)type;
    if (gmn[5] == 0) { gmn[5] = 1; gmn_count++; gmn_nesw_count++; }
    else gmn[5] = 0;

    if (gmn_x == 0) { gmn[6] = 0; gmn_density--; }
    else gmn[6] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr - 8))).base_kind ^ (unsigned char)type;
    if (gmn[6] == 0) { gmn[6] = 1; gmn_count++; gmn_polar_count++; gmn_ew_count++; gmn_density++; }
    else gmn[6] = 0;

    if (gmn_x == 0 || gmn_y == 0) { gmn[7] = 0; gmn_nwse_count--; }
    else gmn[7] = (unsigned char)type ^ (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr - 488))).base_kind;
    if (gmn[7] == 0) { gmn[7] = 1; gmn_count++; gmn_nwse_count++; }
    else gmn[7] = 0;

    gmn[8] = gmn[0];
    gmn[9] = gmn[1];
    gmn[10] = gmn[2];
    gmn[11] = gmn[3];
    gmn[12] = gmn[4];
    gmn[13] = gmn[5];
    gmn[14] = gmn[6];
    gmn[15] = gmn[7];
    for (i = 0; i < 16; i++) { if (gmn[i]) gmn_run++; else gmn_run = 0; if (gmn_run > gmn_max_run) gmn_max_run = gmn_run; }
}

// Measure neighbours of a specific type around a region cell, treating map edges as empty.
// FUNCTION: C2 0x6cbf0
// FUNCTION: C2WIN 0x004aa372
void test_type_regionmap_neighbours_negedge(unsigned char type)
{
    int i;

    gmn_count = gmn_polar_count = gmn_density = 0;
    gmn_ns_count = gmn_ew_count = gmn_nesw_count = gmn_nwse_count = 0;
    gmn_run = gmn_max_run = 0;

    if (gmn_y == 0) gmn[0] = 1;
    else gmn[0] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr - 480))).base_kind ^ (unsigned char)type;
    if (gmn[0] == 0) { gmn[0] = 1; gmn_count++; gmn_polar_count++; gmn_ns_count++; gmn_density++; }
    else gmn[0] = 0;

    if (gmn_y == 0 || gmn_x == 59) gmn[1] = 1;
    else gmn[1] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr - 472))).base_kind ^ (unsigned char)type;
    if (gmn[1] == 0) { gmn[1] = 1; gmn_count++; gmn_nesw_count++; }
    else gmn[1] = 0;

    if (gmn_x == 59) gmn[2] = 1;
    else gmn[2] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr + 8))).base_kind ^ (unsigned char)type;
    if (gmn[2] == 0) { gmn[2] = 1; gmn_count++; gmn_polar_count++; gmn_ew_count++; gmn_density++; }
    else gmn[2] = 0;

    if (gmn_x == 59 || gmn_y == 59) gmn[3] = 1;
    else gmn[3] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr + 488))).base_kind ^ (unsigned char)type;
    if (gmn[3] == 0) { gmn[3] = 1; gmn_count++; gmn_nwse_count++; }
    else gmn[3] = 0;

    if (gmn_y == 59) gmn[4] = 1;
    else gmn[4] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr + 480))).base_kind ^ (unsigned char)type;
    if (gmn[4] == 0) { gmn[4] = 1; gmn_count++; gmn_polar_count++; gmn_ns_count++; gmn_density++; }
    else gmn[4] = 0;

    if (gmn_x == 0 || gmn_y == 59) gmn[5] = 1;
    else gmn[5] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr + 472))).base_kind ^ (unsigned char)type;
    if (gmn[5] == 0) { gmn[5] = 1; gmn_count++; gmn_nesw_count++; }
    else gmn[5] = 0;

    if (gmn_x == 0) gmn[6] = 1;
    else gmn[6] = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr - 8))).base_kind ^ (unsigned char)type;
    if (gmn[6] == 0) { gmn[6] = 1; gmn_count++; gmn_polar_count++; gmn_ew_count++; gmn_density++; }
    else gmn[6] = 0;

    if (gmn_x == 0 || gmn_y == 0) gmn[7] = 1;
    else gmn[7] = (unsigned char)type ^ (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr - 488))).base_kind;
    if (gmn[7] == 0) { gmn[7] = 1; gmn_count++; gmn_nwse_count++; }
    else gmn[7] = 0;

    gmn[8] = gmn[0];
    gmn[9] = gmn[1];
    gmn[10] = gmn[2];
    gmn[11] = gmn[3];
    gmn[12] = gmn[4];
    gmn[13] = gmn[5];
    gmn[14] = gmn[6];
    gmn[15] = gmn[7];
    for (i = 0; i < 16; i++) { if (gmn[i]) gmn_run++; else gmn_run = 0; if (gmn_run > gmn_max_run) gmn_max_run = gmn_run; }
}

// Choose the first pattern record matching the current neighbour configuration.
// FUNCTION: C2 0x6cf69
// FUNCTION: C2WIN 0x004aa7d6
int choose_from(struct choice_rec *records, int count)
{
    int rec_idx;
    int byte_idx;
    for (rec_idx = 0; rec_idx < count; records++, rec_idx++) {
        for (byte_idx = 0; byte_idx < 8; byte_idx++) {
            if (records->match[byte_idx] == 2) continue;
            if (records->match[byte_idx] != 0 && gmn[byte_idx] != 0) continue;
            if (records->match[byte_idx] != 0) break;
            if (gmn[byte_idx] != 0) break;
        }
        if (byte_idx >= 8) {
            first_choice = records->value;
            choice_info  = records->info;
            records->counter++;
            if (records->counter >= records->max_count) {
                records->counter = 0;
            }
            choice_count = records->counter;
            return rec_idx + 1;
        }
    }
    return 0;
}

// Reset the selection state of a choice table.
// FUNCTION: C2 0x6cfed
// FUNCTION: C2WIN 0x004aa8fd
void init_choices(struct choice_rec *arr, int count)
{
    int i;
    for (i = 0; i < count; i++, arr++)
        arr->counter = 0;
}

// Invert the current neighbour flags.
// FUNCTION: C2 0x6d002
void invert_gmn(void)
{
    int i;
    for (i = 0; i < 16; i++) {
        gmn[i] = (gmn[i] == 0);
    }
}

// Clear one data layer across the region map.
// FUNCTION: C2 0x6d01e
// FUNCTION: C2WIN 0x004aaa92
void clear_all_rm(char layer)
{
    cm_sptr = 0;
    for (gmn_y = 0; gmn_y < 60; gmn_y++) {
        for (gmn_x = 0; gmn_x < 6; gmn_x++, cm_sptr += 0x50) {
            int idx = cm_sptr + (unsigned char)layer;
            ((unsigned char *)region_map)[(idx + 0x00)] = 0;
            ((unsigned char *)region_map)[(idx + 0x08)] = 0;
            ((unsigned char *)region_map)[(idx + 0x10)] = 0;
            ((unsigned char *)region_map)[(idx + 0x18)] = 0;
            ((unsigned char *)region_map)[(idx + 0x20)] = 0;
            ((unsigned char *)region_map)[(idx + 0x28)] = 0;
            ((unsigned char *)region_map)[(idx + 0x30)] = 0;
            ((unsigned char *)region_map)[(idx + 0x38)] = 0;
            ((unsigned char *)region_map)[(idx + 0x40)] = 0;
            ((unsigned char *)region_map)[(idx + 0x48)] = 0;
        }
    }
}

// Clear one data layer across the battle map.
// FUNCTION: C2 0x6d0b7
// FUNCTION: C2WIN 0x004aabba
void clear_all_bm(char layer)
{
    cm_sptr = 0;
    for (gmn_y = 0; gmn_y < 52; gmn_y++) {
        for (gmn_x = 0; gmn_x < 13; gmn_x++, cm_sptr += 0x10) {
            int idx = cm_sptr + (unsigned char)layer;
            (*(struct battle_cell *)((unsigned char *)battle_map + (idx))).terrain = 0;
            (*(struct battle_cell *)((unsigned char *)battle_map + (idx + 0x4))).terrain = 0;
            (*(struct battle_cell *)((unsigned char *)battle_map + (idx + 0x8))).terrain = 0;
            (*(struct battle_cell *)((unsigned char *)battle_map + (idx + 0xc))).terrain = 0;
        }
    }
}

// Mask one data field across the city map.
// FUNCTION: C2 0x6d12c
// FUNCTION: C2WIN 0x004aac70
void unflag_all_cm(char field_off, int mask)
{
    cm_sptr = 0;
    gmn_y = 0;
    do {
        gmn_x = 0;
        do {
            ((unsigned char *)city_map)[cm_sptr + field_off + 0x00] &= mask;
            ((unsigned char *)city_map)[cm_sptr + field_off + 0x14] &= mask;
            ((unsigned char *)city_map)[cm_sptr + field_off + 0x28] &= mask;
            ((unsigned char *)city_map)[cm_sptr + field_off + 0x3c] &= mask;
            ((unsigned char *)city_map)[cm_sptr + field_off + 0x50] &= mask;
            ((unsigned char *)city_map)[cm_sptr + field_off + 0x64] &= mask;
            ((unsigned char *)city_map)[cm_sptr + field_off + 0x78] &= mask;
            ((unsigned char *)city_map)[cm_sptr + field_off + 0x8c] &= mask;
            gmn_x++;
            cm_sptr += 0xa0;
        } while (gmn_x < 10);
        gmn_y++;
    } while (gmn_y < 80);
}

// Mask one data field across the region map.
// FUNCTION: C2 0x6d1b7
// FUNCTION: C2WIN 0x004aae45
void unflag_all_rm(char field_off, int mask)
{
    cm_sptr = 0;
    gmn_y = 0;
    do {
        gmn_x = 0;
        do {
            ((unsigned char *)region_map)[(cm_sptr + field_off + 0x00)] &= (char)mask;
            ((unsigned char *)region_map)[(cm_sptr + field_off + 0x08)] &= (char)mask;
            ((unsigned char *)region_map)[(cm_sptr + field_off + 0x10)] &= (char)mask;
            ((unsigned char *)region_map)[(cm_sptr + field_off + 0x18)] &= (char)mask;
            ((unsigned char *)region_map)[(cm_sptr + field_off + 0x20)] &= (char)mask;
            ((unsigned char *)region_map)[(cm_sptr + field_off + 0x28)] &= (char)mask;
            ((unsigned char *)region_map)[(cm_sptr + field_off + 0x30)] &= (char)mask;
            ((unsigned char *)region_map)[(cm_sptr + field_off + 0x38)] &= (char)mask;
            ((unsigned char *)region_map)[(cm_sptr + field_off + 0x40)] &= (char)mask;
            ((unsigned char *)region_map)[(cm_sptr + field_off + 0x48)] &= (char)mask;
            gmn_x++;
            cm_sptr += 0x50;
        } while (gmn_x < 6);
        gmn_y++;
    } while (gmn_y < 60);
}

// Clear temporary edge flags from all regional cells except warehouses.
// FUNCTION: C2 0x6d24b
// FUNCTION: C2WIN 0x004ab0be
void unflag_all_rm_xwarehouse(void)
{
    cm_sptr = 0;
    gmn_y = 0;
    do {
        gmn_x = 0;
        do {
            if ((*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).base_kind != 0xd4)
                (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr))).edge_bits &= 0x3f;
            if ((*(struct region_cell *)((unsigned char *)region_map + (cm_sptr + 8))).base_kind != 0xd4)
                (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr + 8))).edge_bits &= 0x3f;
            if ((*(struct region_cell *)((unsigned char *)region_map + (cm_sptr + 16))).base_kind != 0xd4)
                (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr + 16))).edge_bits &= 0x3f;
            if ((*(struct region_cell *)((unsigned char *)region_map + (cm_sptr + 24))).base_kind != 0xd4)
                (*(struct region_cell *)((unsigned char *)region_map + (cm_sptr + 24))).edge_bits &= 0x3f;
            gmn_x++;
            cm_sptr += 0x20;
        } while (gmn_x < 15);
        gmn_y++;
    } while (gmn_y < 60);
}

// Set a data field throughout a clipped square of the city map.
// FUNCTION: C2 0x6d309
// FUNCTION: C2WIN 0x004ab1f8
void set_range(int x, int y, int range, unsigned char field_off, unsigned char value)
{
    int width;
    int height;
    int xend;
    int yend;
    int row_skip;

    x -= range;
    y -= range;
    width = height = range * 2 + 1;

    xend = width + x;
    if (x < 0) {
        width = xend;
        x = 0;
    } else if (xend > 80) {
        width -= xend - 80;
    }
    yend = height + y;
    if (y < 0) {
        height = yend;
        y = 0;
    } else if (yend > 80) {
        height -= yend - 80;
    }

    gmn_sptr = ((x) + (y) * 80) * 20;
    row_skip  = (80 - width) * 20;
    for (gmn_y = y; gmn_y < y + height; gmn_y++, gmn_sptr += row_skip) {
        for (gmn_x = x; gmn_x < x + width; gmn_x++, gmn_sptr += 20) {
            ((unsigned char *)city_map)[gmn_sptr + field_off] = value;
        }
    }
}

// Set a data field throughout a clipped square of the region map.
// FUNCTION: C2 0x6d3ed
// FUNCTION: C2WIN 0x004ab344
void set_rm_range(int x, int y, int half_width, char field_offset,
                  char kind_byte)
{
    int width;
    int height;
    int row_skip;

    x -= half_width;
    y -= half_width;
    height = 2 * half_width + 1;
    width = height;

    if (x < 0) {
        width = x + height;
        x = 0;
    } else if (x + height > 60) {
        width -= (x + height - 60);
    }

    if (y < 0) {
        height = y + height;
        y = 0;
    } else if (y + height > 60) {
        height -= (y + height - 60);
    }

    gmn_sptr = ((x) + (y) * 60) * 8;
    row_skip = (60 - width) * 8;

    for (gmn_y = y; gmn_y < y + height; gmn_y++, gmn_sptr += row_skip) {
        for (gmn_x = x; gmn_x < x + width; gmn_x++, gmn_sptr += 8) {
            ((unsigned char *)region_map)[(gmn_sptr + field_offset)] = kind_byte;
        }
    }
}

// Set flag bits throughout a clipped square of the city map.
// FUNCTION: C2 0x6d4c1
// FUNCTION: C2WIN 0x004ab48d
void flag_range(int extra, int x, int y, int range, unsigned char field_off, unsigned char mask)
{
    int width;
    int height;
    int xend;
    int yend;
    int row_stride;
    unsigned char fo;

    fo = field_off;
    x -= range;
    y -= range;
    width = height = 2 * range + 1;
    if (extra != 0)
        height = width = height + extra;

    xend = width + x;
    if (x < 0) {
        width = xend;
        x = 0;
    } else if (xend > 80) {
        width -= xend - 80;
    }
    yend = height + y;
    if (y < 0) {
        height = yend;
        y = 0;
    } else if (yend > 80) {
        height -= yend - 80;
    }

    gmn_sptr   = ((x) + (y) * 80) * 20;
    row_stride = (80 - width) * 20;
    for (gmn_y = y; gmn_y < y + height; gmn_y++, gmn_sptr += row_stride) {
        for (gmn_x = x; gmn_x < x + width; gmn_x++, gmn_sptr += 20) {
            ((unsigned char *)city_map)[gmn_sptr + fo] |= mask;
        }
    }
}

// Raise an entertainment coverage band throughout a clipped city-map square.
// FUNCTION: C2 0x6d5aa
// FUNCTION: C2WIN 0x004ab603
void flag_range3(int extra, int x, int y, int range, int unused_field_off,
                 unsigned char threshold, unsigned char query_mask,
                 unsigned char clear_mask)
{
    int height;
    int width;
    int row_stride;
    int xend;
    int yend;

    (void)unused_field_off;

    x -= range;
    y -= range;
    width = height = 2 * range + 1;
    if (extra != 0) {
        width = height = (2 * range + 1) + extra;
    }

    xend = width + x;
    if (x < 0) {
        width = xend;
        x = 0;
    } else if (xend > 80) {
        width -= xend - 80;
    }

    yend = height + y;
    if (y < 0) {
        height = yend;
        y = 0;
    } else if (yend > 80) {
        height -= yend - 80;
    }

    gmn_sptr   = (x + y * 80) * 20;
    row_stride = (80 - width) * 20;

    for (gmn_y = y; gmn_y < y + height; gmn_y++, gmn_sptr += row_stride) {
        for (gmn_x = x; gmn_x < x + width; gmn_x++, gmn_sptr += 20) {
            if (threshold > (unsigned char)((*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).entertainment & query_mask)) {
                (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).entertainment &= clear_mask;
                (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).entertainment |= threshold;
            }
        }
    }
}

// Set a data field on each cardinal city-map neighbour.
// FUNCTION: C2 0x6d6b5
// FUNCTION: C2WIN 0x004ab7b3
void set_4_neighbours(int x, int y, int sptr, unsigned char field_off, unsigned char value)
{
    if (x > 0)  ((unsigned char *)city_map)[sptr - 20 + field_off] = value;
    if (x < 79) ((unsigned char *)city_map)[sptr + 20 + field_off] = value;
    if (y > 0)  ((unsigned char *)city_map)[sptr - 1600 + field_off] = value;
    if (y < 79) ((unsigned char *)city_map)[sptr + 1600 + field_off] = value;
}

// Set a data field on cardinal city neighbours that are not walls or towers.
// FUNCTION: C2 0x6d6fb
// FUNCTION: C2WIN 0x004ab82e
void set_4_neighbours_if_not_wallortower(int x, int y, int sptr,
                                         unsigned char field_off, unsigned char value)
{
    if (x > 0 && (((*(struct city_cell *)((unsigned char *)city_map + ((sptr) - CITY_CELL_BYTES))).terrain & 6) == 0))
        ((unsigned char *)city_map)[sptr - 20 + field_off] = value;
    if (x < 79 && (((*(struct city_cell *)((unsigned char *)city_map + ((sptr) + CITY_CELL_BYTES))).terrain & 6) == 0))
        ((unsigned char *)city_map)[sptr + 20 + field_off] = value;
    if (y > 0 && (((*(struct city_cell *)((unsigned char *)city_map + ((sptr) - CITY_ROW))).terrain & 6) == 0))
        ((unsigned char *)city_map)[sptr - 1600 + field_off] = value;
    if (y < 79 && (((*(struct city_cell *)((unsigned char *)city_map + ((sptr) + CITY_ROW))).terrain & 6) == 0))
        ((unsigned char *)city_map)[sptr + 1600 + field_off] = value;
}

// Set a data field on one axis of city neighbours that are not walls or towers.
// FUNCTION: C2 0x6d785
// FUNCTION: C2WIN 0x004ab8f9
void set_2_neighbours_if_not_wallortower(int x, int y, int sptr,
                                         unsigned char field_off, unsigned char value,
                                         int north_south)
{
    if (north_south == 0) {
        if (x > 0 && (((*(struct city_cell *)((unsigned char *)city_map + ((sptr) - CITY_CELL_BYTES))).terrain & 6) == 0))
            ((unsigned char *)city_map)[sptr - 20 + field_off] = value;
        if (x < 79 && (((*(struct city_cell *)((unsigned char *)city_map + ((sptr) + CITY_CELL_BYTES))).terrain & 6) == 0))
            ((unsigned char *)city_map)[sptr + 20 + field_off] = value;
    } else {
        if (y > 0 && (((*(struct city_cell *)((unsigned char *)city_map + ((sptr) - CITY_ROW))).terrain & 6) == 0))
            ((unsigned char *)city_map)[sptr - 1600 + field_off] = value;
        if (y < 79 && (((*(struct city_cell *)((unsigned char *)city_map + ((sptr) + CITY_ROW))).terrain & 6) == 0))
            ((unsigned char *)city_map)[sptr + 1600 + field_off] = value;
    }
}

// Set a data field on cardinal city neighbours that are not aqueducts or reservoirs.
// FUNCTION: C2 0x6d80a
// FUNCTION: C2WIN 0x004ab9d3
void set_4_neighbours_if_not_aquaductorresevoir(int x, int y, int sptr,
                                                unsigned char field_off, unsigned char value)
{
    if (x > 0 && (((*(struct city_cell *)((unsigned char *)city_map + ((sptr) - CITY_CELL_BYTES))).terrain & 0xc0) == 0))
        ((unsigned char *)city_map)[sptr - 20 + field_off] = value;
    if (x < 79 && (((*(struct city_cell *)((unsigned char *)city_map + ((sptr) + CITY_CELL_BYTES))).terrain & 0xc0) == 0))
        ((unsigned char *)city_map)[sptr + 20 + field_off] = value;
    if (y > 0 && (((*(struct city_cell *)((unsigned char *)city_map + ((sptr) - CITY_ROW))).terrain & 0xc0) == 0))
        ((unsigned char *)city_map)[sptr - 1600 + field_off] = value;
    if (y < 79 && (((*(struct city_cell *)((unsigned char *)city_map + ((sptr) + CITY_ROW))).terrain & 0xc0) == 0))
        ((unsigned char *)city_map)[sptr + 1600 + field_off] = value;
}

// Set a data field on one axis of city neighbours that are not aqueducts or reservoirs.
// FUNCTION: C2 0x6d894
// FUNCTION: C2WIN 0x004aba9e
void set_2_neighbours_if_not_aquaductorresevoir(int x, int y, int sptr,
                                                unsigned char field_off, unsigned char value,
                                                int north_south)
{
    if (north_south == 0) {
        if (x > 0 && ((*(struct city_cell *)((unsigned char *)city_map + ((sptr) - CITY_CELL_BYTES))).terrain & 0xc0) == 0)
            ((unsigned char *)city_map)[sptr - 20 + field_off] = value;
        if (x < 79 && ((*(struct city_cell *)((unsigned char *)city_map + ((sptr) + CITY_CELL_BYTES))).terrain & 0xc0) == 0)
            ((unsigned char *)city_map)[sptr + 20 + field_off] = value;
    } else {
        if (y > 0 && ((*(struct city_cell *)((unsigned char *)city_map + ((sptr) - CITY_ROW))).terrain & 0xc0) == 0)
            ((unsigned char *)city_map)[sptr - 1600 + field_off] = value;
        if (y < 79 && ((*(struct city_cell *)((unsigned char *)city_map + ((sptr) + CITY_ROW))).terrain & 0xc0) == 0)
            ((unsigned char *)city_map)[sptr + 1600 + field_off] = value;
    }
}

// Set a data field on cardinal region neighbours that are not walls or towers.
// FUNCTION: C2 0x6d919
// FUNCTION: C2WIN 0x004abb78
void set_4_rm_neighbours_if_not_wallortower(int x, int y, int sptr,
                                            unsigned char field_off, unsigned char value)
{
    if (x > 0 && (((*(struct region_cell *)((unsigned char *)region_map + (sptr - 8))).terrain & 6) == 0))
        ((unsigned char *)region_map)[(sptr - 8 + field_off)] = (unsigned char)value;
    if (x < 59 && (((*(struct region_cell *)((unsigned char *)region_map + (sptr + 8))).terrain & 6) == 0))
        ((unsigned char *)region_map)[(sptr + 8 + field_off)] = (unsigned char)value;
    if (y > 0 && (((*(struct region_cell *)((unsigned char *)region_map + (sptr - 480))).terrain & 6) == 0))
        ((unsigned char *)region_map)[(sptr - 480 + field_off)] = (unsigned char)value;
    if (y < 59 && (((*(struct region_cell *)((unsigned char *)region_map + (sptr + 480))).terrain & 6) == 0))
        ((unsigned char *)region_map)[(sptr + 480 + field_off)] = (unsigned char)value;
}

// Set a data field on cardinal region neighbours that are not aqueducts or reservoirs.
// FUNCTION: C2 0x6d9a3
// FUNCTION: C2WIN 0x004abc43
void set_4_rm_neighbours_if_not_aquaductorresevoir(int x, int y, int sptr,
                                                   unsigned char field_off, unsigned char value)
{
    if (x > 0 && ((*(struct region_cell *)((unsigned char *)region_map + (sptr - 8))).terrain & 0x40) == 0)
        ((unsigned char *)region_map)[(sptr - 8 + field_off)] = value;
    if (x < 59 && ((*(struct region_cell *)((unsigned char *)region_map + (sptr + 8))).terrain & 0x40) == 0)
        ((unsigned char *)region_map)[(sptr + 8 + field_off)] = value;
    if (y > 0 && ((*(struct region_cell *)((unsigned char *)region_map + (sptr - 480))).terrain & 0x40) == 0)
        ((unsigned char *)region_map)[(sptr - 480 + field_off)] = value;
    if (y < 59 && ((*(struct region_cell *)((unsigned char *)region_map + (sptr + 480))).terrain & 0x40) == 0)
        ((unsigned char *)region_map)[(sptr + 480 + field_off)] = value;
}

// Increase the path cost of each cardinal city neighbour without changing blocked cells.
// FUNCTION: C2 0x6da2d
// FUNCTION: C2WIN 0x004abd0e
void inc_elastic_by2(int x, int y, int sptr)
{
    if (x > 0) {
        if ((*(struct city_cell *)((unsigned char *)city_map + ((sptr) - CITY_CELL_BYTES))).road_aqueduct != 0xff)
            (*(struct city_cell *)((unsigned char *)city_map + ((sptr) - CITY_CELL_BYTES))).road_aqueduct += 2;
    }
    if (x < 79) {
        if ((*(struct city_cell *)((unsigned char *)city_map + ((sptr) + CITY_CELL_BYTES))).road_aqueduct != 0xff)
            (*(struct city_cell *)((unsigned char *)city_map + ((sptr) + CITY_CELL_BYTES))).road_aqueduct += 2;
    }
    if (y > 0) {
        if ((*(struct city_cell *)((unsigned char *)city_map + ((sptr) - CITY_ROW))).road_aqueduct != 0xff)
            (*(struct city_cell *)((unsigned char *)city_map + ((sptr) - CITY_ROW))).road_aqueduct += 2;
    }
    if (y < 79) {
        if ((*(struct city_cell *)((unsigned char *)city_map + ((sptr) + CITY_ROW))).road_aqueduct != 0xff)
            (*(struct city_cell *)((unsigned char *)city_map + ((sptr) + CITY_ROW))).road_aqueduct += 2;
    }
}

// Test whether the north or south city neighbour is a wall.
// FUNCTION: C2 0x6daa6
// FUNCTION: C2WIN 0x004abdf9
int test_for_ns_polar_walls(int _eax_unused, int y, int sptr)
{
    (void)_eax_unused;
    if (y > 0     && ((*(struct city_cell *)((unsigned char *)city_map + ((sptr) - CITY_ROW))).terrain & 0x06)) return 1;
    if (y < 0x4f  && ((*(struct city_cell *)((unsigned char *)city_map + ((sptr) + CITY_ROW))).terrain & 0x06)) return 1;
    return 0;
}

// Test whether the east or west city neighbour is a wall.
// FUNCTION: C2 0x6dad6
// FUNCTION: C2WIN 0x004abe5b
int test_for_ew_polar_walls(int x, int _edx_unused, int sptr)
{
    (void)_edx_unused;
    if (x > 0     && ((*(struct city_cell *)((unsigned char *)city_map + ((sptr) - CITY_CELL_BYTES))).terrain & 0x06)) return 1;
    if (x < 0x4f  && ((*(struct city_cell *)((unsigned char *)city_map + ((sptr) + CITY_CELL_BYTES))).terrain & 0x06)) return 1;
    return 0;
}

// Test whether any adjacent regional cell, including diagonals, is a wall.
// FUNCTION: C2 0x6db08
// FUNCTION: C2WIN 0x004abebd
int test_for_next_to_region_wall(int x, int y)
{
    int sptr = ((x) + (y) * 60) * 8;

    if (y > 0) {
        if ((*(struct region_cell *)((unsigned char *)region_map + (sptr - 480))).terrain & 0x02) return 1;
        if (x > 0    && ((*(struct region_cell *)((unsigned char *)region_map + (sptr - 488))).terrain & 0x02)) return 1;
        if (x < 0x3b && ((*(struct region_cell *)((unsigned char *)region_map + (sptr - 472))).terrain & 0x02)) return 1;
    }
    if (y < 0x3b) {
        if ((*(struct region_cell *)((unsigned char *)region_map + (sptr + 480))).terrain & 0x02) return 1;
        if (x > 0    && ((*(struct region_cell *)((unsigned char *)region_map + (sptr + 472))).terrain & 0x02)) return 1;
        if (x < 0x3b && ((*(struct region_cell *)((unsigned char *)region_map + (sptr + 488))).terrain & 0x02)) return 1;
    }
    if (x > 0    && ((*(struct region_cell *)((unsigned char *)region_map + (sptr - 8))).terrain & 0x02)) return 1;
    if (x < 0x3b && ((*(struct region_cell *)((unsigned char *)region_map + (sptr + 8))).terrain & 0x02)) return 1;
    return 0;
}

// Count cells of `building_kind` in a clipped rectangular region-map search area.
// FUNCTION: C2 0x6dbda
// FUNCTION: C2WIN 0x004ac027
int get_reg_buildings_in_radius(int x, int y, int span, int radius,
                                unsigned char building_kind)
{
    int width;
    int height;
    int row_skip;
    int count;
    unsigned char kind;

    x = x - radius;
    y = y - radius;
    height = radius * 2 + 1;
    span--;
    width = height + span;
    height = width;
    if (x < 0) { width += x; x = 0; }
    else if (x + width > 60) width -= x + width - 60;
    if (y < 0) { height += y; y = 0; }
    else if (y + height > 60) height -= y + height - 60;

    gmn_sptr = (x + y * 60) * 8;
    row_skip = (60 - width) * 8;

    count = 0;
    for (gmn_y = y; gmn_y < y + height; gmn_y++, gmn_sptr += row_skip) {
        for (gmn_x = x; gmn_x < x + width; gmn_x++, gmn_sptr += 8) {
            kind = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).base_kind;
            if (kind == building_kind) count++;
        }
    }
    return count;
}

// Count regional industry and port cells around a location.
// FUNCTION: C2 0x6dcb4
// FUNCTION: C2WIN 0x004ac19b
int get_reg_industries_in_radius(int x, int y)
{
    int w;
    int h;
    int row_skip;
    int count;
    unsigned char type;

    x = x - 1; y = y - 1;
    h = 3; w = h;

    if (x < 0) { w += x; x = 0; }
    else if (x + w > 60) w -= x + w - 60;
    if (y < 0) { h += y; y = 0; }
    else if (y + h > 60) h -= y + h - 60;

    gmn_sptr = (x + y * 60) * 8;
    row_skip = (60 - w) * 8;

    count = 0;
    for (gmn_y = y; gmn_y < y + h; gmn_y++, gmn_sptr += row_skip) {
        for (gmn_x = x; gmn_x < x + w; gmn_x++, gmn_sptr += 8) {
            type = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).base_kind;
            if (type >= 0xdc && type <= 0xef) count++;
        }
    }
    return count;
}

// Find the nearest available trading-post building within a regional search radius.
// FUNCTION: C2 0x6dd8c
// FUNCTION: C2WIN 0x004ac2ff
int get_closest_trading_post(int x, int y, int radius)
{
    unsigned char kind;
    unsigned char occ;
    int y2;
    int x2;
    int width;
    int height;
    int row_skip;
    int best;
    int best_sptr;
    int dist;

    best_sptr = 0;
    best = radius + 1;
    x2 = x;
    y2 = y;
    x -= radius;
    y -= radius;
    width = height = radius * 2 + 1;
    if (x < 0) {
        width += x;
        x = 0;
    } else if (x + width > 60) {
        width -= x + width - 60;
    }
    if (y < 0) {
        height += y;
        y = 0;
    } else if (y + height > 60) {
        height -= y + height - 60;
    }
    gmn_sptr = (y * 60 + x) * 8;
    row_skip = (60 - width) * 8;

    for (gmn_y = y; gmn_y < y + height; gmn_y++, gmn_sptr += row_skip) {
        for (gmn_x = x; gmn_x < x + width; gmn_x++, gmn_sptr += 8) {
            kind = ((unsigned char *)region_map)[gmn_sptr];
            occ  = ((unsigned char *)region_map)[gmn_sptr + 7] & 3;
            if (occ != 0) continue;
            if (kind >= 0xe8 && kind <= 0xeb) {
                dist = get_longest_distance(gmn_x, gmn_y, x2, y2);
                if (dist < best) {
                    best = dist;
                    best_sptr = gmn_sptr;
                }
            }
        }
    }
    gmn_sptr = best_sptr;
    return best;
}

// Distribute goods among compatible warehouses near a regional location.
// FUNCTION: C2 0x6dedf
// FUNCTION: C2WIN 0x004ac4e9
void fill_warehouses_with(int x, int y, int amount, int goods, int refresh)
{
    unsigned char stored_goods;
    unsigned char qty;
    unsigned char cell_kind;
    unsigned char refresh_qty;
    int row_skip;
    int i;
    int height;
    int width;

    if (amount == 0) return;
    x -= 1; y--;
    width = height = 4;
    if (x < 0)             { width = x + 4; x = 0; }
    else if (x + 4 > 0x3c) width = 4 - (x - 0x38);
    if (y < 0)             { height += y; y = 0; }
    else if (y + height > 0x3c) height -= y + height - 0x3c;

    row_skip = (60 - width) * 8;
    i = 0;
    for ( ; i < amount; i++) {
        gmn_sptr = ((x) + (y) * 60) * 8;
        gmn_y = y;
        for ( ; gmn_y < y + height; gmn_y++, gmn_sptr += row_skip) {
            gmn_x = x;
            for ( ; gmn_x < x + width; gmn_x++, gmn_sptr += 8) {
                cell_kind = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).base_kind;
                if (cell_kind == 0xd4) {
                    stored_goods = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).occupant & 0xf0;
                    stored_goods >>= 4;
                    qty = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).occupant & 0xf;
                    if (qty == 0) {
                        (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).edge_bits |= 0x40;
                        (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).occupant &= 0xf;
                        (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).occupant |= (goods << 4);
                    } else if (stored_goods != goods) continue;
                    else if (qty >= 0xf) continue;
                    qty++;
                    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).occupant &= 0xf0;
                    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).occupant |= qty;
                    goto next_unit;
                }
            }
        }
next_unit:
        ;
    }

    if (refresh == 1) {
        gmn_sptr = ((x) + (y) * 60) * 8;
        gmn_y = y;
        for ( ; gmn_y < y + height; ) {
            gmn_x = x;
            for ( ; gmn_x < x + width; ) {
                cell_kind = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).base_kind;
                if (cell_kind == 0xd4) {
                    qty = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).occupant & 0xf;
                    if (qty < 0xf) refresh_qty = qty + 0xb;
                    else refresh_qty = 0x24;
                    change_reg_sized(cell_kind, refresh_qty, 1, gmn_sptr);
                }
                gmn_x++; gmn_sptr += 8; } gmn_y++; gmn_sptr += row_skip; }
    }
}

// Remove the requested goods from warehouses across the region map.
// FUNCTION: C2 0x6e10d
// FUNCTION: C2WIN 0x004ac84e
void take_from_warehouses(int amount, int goods)
{
    unsigned char qty;
    unsigned char stored_goods;

    if (amount <= 0) return;
    gmn_y = 0;
    gmn_sptr = 0;
    for ( ; gmn_y < 60; gmn_y++) {
    gmn_x = 0;
    do {
        if (((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).base_kind & 0xff) == 0xd4) {
                stored_goods = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).occupant & 0xf0;
                stored_goods >>= 4;
                qty = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).occupant & 0xf;
                if (stored_goods == goods && qty != 0) {
                    if (amount >= qty) {
                        amount -= qty;
                        qty = 0;
                    } else {
                        qty -= amount;
                        amount = 0;
                    }
                    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).occupant &= 0xf0;
                    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).occupant |= qty;
                    if (amount <= 0) return;
                }
            }
        gmn_x++;
        gmn_sptr += 8;
    } while (gmn_x < 60);
    }
}

// Set a data field on the north and south city neighbours.
// FUNCTION: C2 0x6e1cb
// FUNCTION: C2WIN 0x004ac9b3
void set_ns_polar(int x, int y, int sptr, unsigned char field_off, unsigned char value)
{
    (void)x;
    if (y > 0)  ((unsigned char *)city_map)[sptr - 1600 + field_off] = value;
    if (y < 79) ((unsigned char *)city_map)[sptr + 1600 + field_off] = value;
}

// Set a data field on the east and west city neighbours.
// FUNCTION: C2 0x6e1f6
// FUNCTION: C2WIN 0x004ac9f6
void set_ew_polar(int x, int y, int sptr, unsigned char field_off, unsigned char value)
{
    (void)y;
    if (x > 0)  ((unsigned char *)city_map)[sptr - 20 + field_off] = value;
    if (x < 79) ((unsigned char *)city_map)[sptr + 20 + field_off] = value;
}

// Adjust land values in a clipped city-map area, clamping them to the valid range.
// FUNCTION: C2 0x6e221
// FUNCTION: C2WIN 0x004aca39
void change_lv(int x, int y, int radius, int extra, int delta)
{
    int width;
    int height;
    int row_skip;
    int land_value;

    if (extra == 0) return;
    x -= radius;
    y -= radius;
    height = extra + radius * 2;
    width = height;
    if (x < 0) { width += x; x = 0; }
    else if (x + width > 80) width -= x + width - 80;
    if (y < 0) { height += y; y = 0; }
    else if (y + height > 80) height -= y + height - 80;

    gmn_sptr = ((x) + (y) * 80) * 20;
    row_skip = (80 - width) * 20;
    for (gmn_y = y; gmn_y < y + height;) {
        for (gmn_x = x; gmn_x < x + width;) {
            signed char nv = (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).land_value;
            nv += (char)delta;
            land_value = nv;
            if (land_value > 0x40) nv = 0x40;
            else if (land_value < -0x40) nv = -0x40;
            (*(struct city_cell *)((unsigned char *)city_map + (gmn_sptr))).land_value = nv;
            gmn_x++;
            gmn_sptr += 20;
        }
        gmn_y++;
        gmn_sptr += row_skip;
    }
}

// Return the highest land value in a square city-map footprint.
// FUNCTION: C2 0x6e31b
// FUNCTION: C2WIN 0x004acbcf
int get_best_lv(unsigned char *base, int bp)
{
    int best;
    int i;
    int j;
    int value;

    best = 0;
    for (i = 0; i < bp; i++) {
        for (j = 0; j < bp; j++) {
            value = (base + j * CITY_CELL_BYTES + i * CITY_ROW)[15];
            if (value > best) best = value;
        }
    }
    return best;
}

// Return the top-left cell of the building footprint containing a city-map cell.
// FUNCTION: C2 0x6e36a
// FUNCTION: C2WIN 0x004acc61
unsigned char *get_ptr_to_corner(unsigned char *base_ptr, int size)
{
    int packed;
    int x_off;

    packed = base_ptr[5] & 0xf;
    x_off = packed % size; packed /= size;
    base_ptr -= x_off * 20;
    packed *= 1600; return base_ptr - packed;
}

// Test whether a city building footprint has any requested education coverage.
// FUNCTION: C2 0x6e3b5
// FUNCTION: C2WIN 0x004accc6
char affected_by_cover1(unsigned char *p, int range, char mask)
{
    int xi;
    int yi;

    if (range == 1)
        return (p)[13] & mask;
    for (yi = 0; yi < range; yi++) {
        for (xi = 0; xi < range; xi++) {
            if ((p + xi * CITY_CELL_BYTES + yi * CITY_ROW)[13] & mask)
                return 1;
        }
    }
    return 0;
}

// Test whether a city building footprint has any requested health coverage.
// FUNCTION: C2 0x6e41c
// FUNCTION: C2WIN 0x004acd6f
char affected_by_cover2(unsigned char *p, int range, char mask)
{
    int xi;
    int yi;

    if (range == 1)
        return (p)[14] & mask;
    for (yi = 0; yi < range; yi++) {
        for (xi = 0; xi < range; xi++) {
            if ((p + xi * CITY_CELL_BYTES + yi * CITY_ROW)[14] & mask)
                return 1;
        }
    }
    return 0;
}

// Return the highest masked service-range value in a city building footprint.
// FUNCTION: C2 0x6e47b
// FUNCTION: C2WIN 0x004ace18
int get_range1(unsigned char *start, int range, char mask)
{
    int best;
    int row;
    int col;
    int range_value;
    unsigned char *cell_ptr;

    if (range == 1) {
        range_value = (start)[10];
        range_value &= mask;
        return range_value;
    }
    best = 0;
    for (row = 0; row < range; row++) {
        for (col = 0; col < range; col++) {
            cell_ptr = start + col * CITY_CELL_BYTES + row * CITY_ROW;
            range_value = (cell_ptr)[10];
            range_value &= mask;
            if (range_value > best)
                best = range_value;
        }
    }
    return best;
}

// Return the highest masked entertainment coverage in a city building footprint.
// FUNCTION: C2 0x6e4f3
// FUNCTION: C2WIN 0x004aced1
int get_range3(unsigned char *start, int range, char mask)
{
    int best;
    int row;
    int col;
    int range_value;
    unsigned char *cell_ptr;

    if (range == 1) {
        range_value = (start)[12];
        range_value &= mask;
        return range_value;
    }
    best = 0;
    for (row = 0; row < range; row++) {
        for (col = 0; col < range; col++) {
            cell_ptr = start + col * CITY_CELL_BYTES + row * CITY_ROW;
            range_value = (cell_ptr)[12];
            range_value &= mask;
            if (range_value > best)
                best = range_value;
        }
    }
    return best;
}

// Summarize housing and service coverage within a city-map radius.
// FUNCTION: C2 0x6e563
// FUNCTION: C2WIN 0x004acf8a
void test_range_for(int x, int y, int radius, int mode)
{
    unsigned char kind_byte;
    int kind;
    int height;
    int yend;
    int row_skip;
    int sptr;
    char val;
    int width;
    int xend;
    char sub;
    int diff;

    x -= radius;
    y -= radius;
    width = height = radius * 2 + 1;

    xend = width + x;
    if (x < 0) {
        width = xend;
        x = 0;
    } else if (xend > 80) {
        width -= xend - 80;
    }
    yend = height + y;
    if (y < 0) {
        height = yend;
        y = 0;
    } else if (yend > 80) {
        height -= yend - 80;
    }

    sptr = ((x) + (y) * 80) * 20;
    diff = (80 - width) * 4;
    diff += (80 - width);
    row_skip = diff * 4;
    test_result1 = 0;
    test_result2 = 0;
    test_result3 = 0;
    test_result4 = 0;
    for (gmn_y = y; gmn_y < y + height; gmn_y++, sptr += row_skip) {
        for (gmn_x = x; gmn_x < x + width; gmn_x++, sptr += 20) {
            sub = (*(struct city_cell *)((unsigned char *)city_map + (sptr))).activity_a & 0xf;
            kind_byte = (*(struct city_cell *)((unsigned char *)city_map + (sptr))).base_kind;
            kind = kind_byte;
            if (kind >= 0x82 && kind <= 0xa1)
                if (sub == 0) {
                    test_result1++;
                    if (mode == 0) {
                        val = (*(struct city_cell *)((unsigned char *)city_map + (sptr))).range_flag & 0xc;
                        val >>= 2;
                        if (val == 0)
                            test_result2++;
                        else
                            test_result3 += val;
                    } else if (mode == 1) {
                        val = (*(struct city_cell *)((unsigned char *)city_map + (sptr))).range_flag & 0x30;
                        if (val == 0)
                            test_result2++;
                    } else if (mode == 2) {
                        val = (*(struct city_cell *)((unsigned char *)city_map + (sptr))).fpu_flag & 0xf;
                        test_result2 += val;
                    }
                }
        }
    }
}

// Test whether a city-map radius contains a road or plaza.
// FUNCTION: C2 0x6e6d6
// FUNCTION: C2WIN 0x004ad1db
int test_range_for_road(int x, int y, int radius)
{
    int side;
    int width;
    int sptr;
    int row_skip;
    unsigned char road_flag;
    x -= radius;
    y -= radius;
    side = radius * 2 + 1;
    width = side;

    if (x < 0) { width = side + x; x = 0; }
    else if (x + side > 80) { width -= (x + side) - 80; }
    if (y < 0) { side = side + y; y = 0; }
    else if (y + side > 80) { side -= (y + side) - 80; }

    sptr = ((x) + (y) * 80) * 20;
    row_skip = (80 - width) * 20;

    for (gmn_y = y; gmn_y < y + side; gmn_y++, sptr += row_skip) {
        for (gmn_x = x; gmn_x < x + width; gmn_x++, sptr += 20) {

            road_flag = (*(struct city_cell *)((unsigned char *)city_map + (sptr))).terrain & 0x20;
            if (road_flag != 0) return 1;
        }
    }
    return 0;
}

// Count the population represented by housing within a city-map area.
// FUNCTION: C2 0x6e7a2
// FUNCTION: C2WIN 0x004ad339
int test_area_for_population(int extra, int x, int y, int radius)
{
    int width;
    int height;
    int xend;
    int yend;
    int row_skip;
    int sptr;
    int total;
    int kind;
    char kind_byte;
    char flag;

    x -= radius;
    y -= radius;
    width = height = radius * 2 + 1;
    if (extra != 0)
        height = width = height + extra;

    xend = width + x;
    if (x < 0) {
        width = xend;
        x = 0;
    } else if (xend > 80) {
        width -= xend - 80;
    }
    yend = height + y;
    if (y < 0) {
        height = yend;
        y = 0;
    } else if (yend > 80) {
        height -= yend - 80;
    }

    sptr = ((x) + (y) * 80) * 20;
    row_skip = (80 - width) * 20;
    total = 0;
    for (gmn_y = y; gmn_y < y + height; gmn_y++, sptr += row_skip) {
        for (gmn_x = x; gmn_x < x + width; gmn_x++, sptr += 20) {
            flag = (*(struct city_cell *)((unsigned char *)city_map + (sptr))).activity_a & 0xf;
            kind_byte = (*(struct city_cell *)((unsigned char *)city_map + (sptr))).base_kind;
            kind = kind_byte & 0xff;
            if (kind >= 0x82 && kind <= 0xa1 && flag == 0)
                total += houses_to_people[kind - 0x82];
        }
    }
    return total;
}

// Save the active city or region map for a possible undo.
// FUNCTION: C2 0x6e898
// FUNCTION: C2WIN 0x004ad4f0
void save_undo_info(void)
{
    if (map_mode == 0) {
        sb_cm_undo_flushed = 0;
        copy((unsigned char *)city_map, scratch_buffer, 0x1f400);
    } else if (map_mode == 1) {
        sb_rm_undo_flushed = 0;
        copy((unsigned char *)region_map, scratch_buffer + 0x1f400, 0x7080);
    }
}


// Restore the city map from the pending undo snapshot.
// FUNCTION: C2 0x6e8eb
// FUNCTION: C2WIN 0x004ad562
void restore_city_from_undo_buffer(void)
{
    if (sb_cm_undo_flushed != 0) return;
    copy(scratch_buffer, (unsigned char *)city_map, 0x1f400);
    particles_cleared = particles_built = 0;
}

// Restore the region map from the pending undo snapshot and repair army positions.
// FUNCTION: C2 0x6e91b
// FUNCTION: C2WIN 0x004ad5ad
void restore_region_from_undo_buffer(void)
{
    if (sb_rm_undo_flushed != 0) return;
    copy(scratch_buffer + 0x1f400, (unsigned char *)region_map, 0x7080);
    particles_cleared = particles_built = 0;
    army_restoring_adjusts();
}

// Reset every region-map layer and the city-map layers shared with regional state.
// FUNCTION: C2 0x6e955
// FUNCTION: C2WIN 0x004ad602
void clear_region_map(void)
{
    clear_all_rm(2);
    clear_all_rm(0);
    clear_all_rm(1);
    clear_all_rm(3);
    clear_all_cm(5);
    clear_all_cm(6);
    clear_all_rm(7);
    clear_all_cm(4);
}

// Clear one data layer across the city map.
// FUNCTION: C2 0x6e99d
void clear_all_cm(char layer)
{
    cm_sptr = 0;
    for (gmn_y = 0; gmn_y < 80; gmn_y++) {
        for (gmn_x = 0; gmn_x < 10; gmn_x++, cm_sptr += 0xa0) {
            int idx = cm_sptr + (unsigned char)layer;
            (*(struct city_cell *)((unsigned char *)city_map + (idx))).base_kind = 0;
            (*(struct city_cell *)((unsigned char *)city_map + ((idx) + CITY_CELL_BYTES))).base_kind = 0;
            (*(struct city_cell *)((unsigned char *)city_map + (idx + 0x28))).base_kind = 0;
            (*(struct city_cell *)((unsigned char *)city_map + (idx + 0x3c))).base_kind = 0;
            (*(struct city_cell *)((unsigned char *)city_map + (idx + 0x50))).base_kind = 0;
            (*(struct city_cell *)((unsigned char *)city_map + (idx + 0x64))).base_kind = 0;
            (*(struct city_cell *)((unsigned char *)city_map + (idx + 0x78))).base_kind = 0;
            (*(struct city_cell *)((unsigned char *)city_map + (idx + 0x8c))).base_kind = 0;
        }
    }
}

// Clear temporary edge information from the active map.
// FUNCTION: C2 0x6ea2d
// FUNCTION: C2WIN 0x004ad65d
void clear_edge_info(void)
{
    if (map_mode == 0) {
        unflag_all_cm(3, 0xfd);
    } else if (map_mode == 1) {
        unflag_all_rm(3, 0xfd);
    }
}

// Build the regional movement-cost field used to preview an army route.
// FUNCTION: C2 0x6ea65
// FUNCTION: C2WIN 0x004ad6aa
void set_route_elastic(void)
{
    int i;
    clear_all_rm(2);
    for (i = 1; i <= 0xf; i++)
        set_route_elastic_range(i);
}

// Expand the regional army route search by one range band.
// FUNCTION: C2 0x6ea84
// FUNCTION: C2WIN 0x004ad6ec
void set_route_elastic_range(int radius)
{
    int x_span;
    int y_min;
    unsigned char n_nw;
    unsigned char best;
    unsigned char n_sw;
    unsigned char saved;
    unsigned char n_w;
    int stride;
    unsigned char n_e;
    unsigned char t;
    unsigned char n_n;
    unsigned char n_ne;
    unsigned char cost;
    unsigned char n_se;
    int side;
    int x_min;
    unsigned char n_s;

    gmn_sptr = ((over_x) + (over_y) * 60) * 8;
    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state = 1;

    x_min = over_x - radius;
    y_min = over_y - radius;
    side  = 2 * radius + 1;
    x_span = side;
    if (x_min <= 0) {
        x_span += x_min;
        x_min = 0;
    } else if (x_min + side > 0x3c) {
        x_span -= x_min + side - 0x3c;
    }
    if (y_min <= 0) {
        side += y_min; y_min = 0;
    } else if (y_min + side >= 0x3c) {
        side -= y_min + side - 0x3c;
    }

    gmn_sptr = ((x_min) + (y_min) * 60) * 8;
    stride   = (0x3c - x_span) * 8;
    gmn_y = y_min;
    for ( ; gmn_y < y_min + side; gmn_y++, gmn_sptr += stride) {
        gmn_x = x_min;
        for ( ; gmn_x < x_min + x_span; gmn_x++, gmn_sptr += 8) {
            if ((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state == 0xff)
                continue;
            if ((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain & 0x11) {
                t = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).base_kind;
                if (t < 0x93 || t > 0x96) {
                    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state = 0xff;
                    continue;
                }
            }
            if ((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain & 0x04) {
                if ((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).occupant != 0 &&
                    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).occupant != tracking_army) {
                    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state = 0xff;
                    continue;
                }
                cost = 2;
            } else if ((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain & 0x20) {
                cost = 1;
            } else {
                if ((*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).terrain & 0x02) {
                    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state = 0xff;
                    continue;
                }
                cost = 2;
            }

            saved = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state;
            n_n = n_e = n_s = n_w = n_ne = n_se = n_sw = n_nw = 0;
            best = 0xc7;
            if (gmn_y > 0)
                n_n = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr - 480))).place_state;
            if (gmn_x < 0x3b)
                n_e = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr + 8))).place_state;
            if (gmn_y < 0x3b)
                n_s = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr + 480))).place_state;
            if (gmn_x > 0)
                n_w = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr - 8))).place_state;
            if (gmn_y > 0 && gmn_x < 0x3b)
                n_ne = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr - 472))).place_state;
            if (gmn_x < 0x3b && gmn_y < 0x3b)
                n_se = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr + 488))).place_state;
            if (gmn_y < 0x3b && gmn_x > 0)
                n_sw = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr + 472))).place_state;
            if (gmn_x > 0 && gmn_y > 0)
                n_nw = (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr - 488))).place_state;
            if (n_n != 0 && n_n != 0xff && n_n < best)
                best = n_n;
            if (n_e != 0 && n_e != 0xff && n_e < best)
                best = n_e;
            if (n_s != 0 && n_s != 0xff && n_s < best)
                best = n_s;
            if (n_w != 0 && n_w != 0xff && n_w < best)
                best = n_w;
            if (n_ne != 0 && n_ne != 0xff && n_ne < best)
                best = n_ne;
            if (n_se != 0 && n_se != 0xff && n_se < best)
                best = n_se;
            if (n_sw != 0 && n_sw != 0xff && n_sw < best)
                best = n_sw;
            if (n_nw != 0 && n_nw != 0xff && n_nw < best)
                best = n_nw;
            {
                int value;
                value = best;
                if (cost + value < saved) {
                    n_sw = best + cost;
                    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state = n_sw;
                } else if (saved == 0 && best != 0) {
                    n_sw = best + cost;
                    (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).place_state = n_sw;
                }
            }
        }
    }
}

// Trace the lowest-cost regional path back into the selected army route.
// FUNCTION: C2 0x6ee0b
// FUNCTION: C2WIN 0x004adcc4
void trace_back_route_elastic(void)
{
    unsigned char orig;
    int idx;
    unsigned char n_n;
    unsigned char n_nw;
    unsigned char n_e;
    unsigned char n_se;
    unsigned char dir;
    unsigned char n_w;
    unsigned char n_s;
    unsigned char n_ne;
    int iters;
    unsigned char best;
    int i;
    unsigned char n_sw;

    idx   = 0;
    iters = 0;
    for (i = 0; i < 16; i++) temp_route[i].x = temp_route[i].y = 0;

    gmn_x = over_x; gmn_y = over_y;
    gmn_sptr = ((gmn_x) + (gmn_y) * 60) * 8;
    temp_route[idx].x = gmn_x;
    temp_route[idx].y = gmn_y;

    while (++iters < 1000) {
        orig = ((unsigned char *)region_map)[gmn_sptr + 2];
        n_n = n_w = n_se = n_ne = n_sw = n_nw = 0; n_e = n_s = 0;
        best = orig;
        if (gmn_y > 0) n_n = ((unsigned char *)region_map)[gmn_sptr - 0x1de];
        if (gmn_x < 0x3b) n_e = ((unsigned char *)region_map)[gmn_sptr + 0xa];
        if (gmn_y < 0x3b) n_s = ((unsigned char *)region_map)[gmn_sptr + 0x1e2];
        if (gmn_x > 0) n_w = ((unsigned char *)region_map)[gmn_sptr - 6];
        if (gmn_y > 0 && gmn_x < 0x3b) n_ne = ((unsigned char *)region_map)[gmn_sptr - 0x1d6];
        if (gmn_x < 0x3b && gmn_y < 0x3b) n_se = ((unsigned char *)region_map)[gmn_sptr + 0x1ea];
        if (gmn_y < 0x3b && gmn_x > 0) n_sw = ((unsigned char *)region_map)[gmn_sptr + 0x1da];
        if (gmn_x > 0 && gmn_y > 0) n_nw = ((unsigned char *)region_map)[gmn_sptr - 0x1e6];

        if (n_n  != 0 && best > n_n) { best = n_n;  dir = 0; }
        if (n_e  != 0 && n_e  < best) { best = n_e;  dir = 2; }
        if (n_s  != 0 && n_s  < best) { best = n_s;  dir = 4; }
        if (n_w  != 0 && n_w  < best) { best = n_w;  dir = 6; }
        if (n_ne != 0 && n_ne < best) { best = n_ne; dir = 1; }
        if (n_se != 0 && n_se < best) { best = n_se; dir = 3; }
        if (n_sw != 0 && n_sw < best) { best = n_sw; dir = 5; }
        if (n_nw != 0 && best > n_nw) { best = n_nw; dir = 7; }

        if (orig != best) {
            gmn_x += gmn_ofsets[dir].dx;
            gmn_y += gmn_ofsets[dir].dy;
            idx++;
            temp_route[idx].x = gmn_x;
            temp_route[idx].y = gmn_y;
            if (best == 1) break;
            gmn_sptr = ((gmn_x) + (gmn_y) * 60) * 8;
            (*(struct region_cell *)((unsigned char *)region_map + (gmn_sptr))).edge_bits |= 0x40;
        }
    }

    for (i = 0; i <= idx; i++) {
        army_routes[army_list[tracking_army].cohort_id]
            .points[this_route_number][i].x = temp_route[idx - i].x;
        army_routes[army_list[tracking_army].cohort_id]
            .points[this_route_number][i].y = temp_route[idx - i].y;
    }
    army_routes[army_list[tracking_army].cohort_id]
        .row_len[this_route_number] = (unsigned char)idx + 1;
}

// Reset city, province, and danger map markers.
// FUNCTION: C2 0x6f0fa
// FUNCTION: C2WIN 0x004ae1a9
void init_flag_markers(void)
{
    int i;

    flag_mode = 0;
    flag_mode_decay_count = 0;
    danger_flag_map_mode = 0;
    no_of_danger_flags = 0;
    no_of_prov_flags = 0;
    no_of_city_flags = 0;
    last_danger_flag = 0;
    last_prov_flag = 0;
    last_city_flag = 0;
    for (i = 0; i < 0x14; i++)
        city_flag_list[i] = -1;
    for (i = 0; i < 0x14; i++)
        prov_flag_list[i] = -1;
    for (i = 0; i < 0x14; i++)
        danger_flag_list[i] = -1;
}

// Update the number of active city markers.
// FUNCTION: C2 0x6f173
// FUNCTION: C2WIN 0x004ae2e2
void count_city_flags(void)
{
    int i;
    no_of_city_flags = 0;
    for (i = 0; i < 0x14; i++) {
        if (city_flag_list[i] != -1)
            no_of_city_flags++;
    }
}

// Update the number of active province markers.
// FUNCTION: C2 0x6f196
// FUNCTION: C2WIN 0x004ae32f
void count_prov_flags(void)
{
    int i;
    no_of_prov_flags = 0;
    for (i = 0; i < 0x14; i++) {
        if (prov_flag_list[i] != -1)
            no_of_prov_flags++;
    }
}

// Add or remove the city marker at a map location.
// FUNCTION: C2 0x6f1b9
// FUNCTION: C2WIN 0x004ae37c
int toggle_city_flag(int val)
{
    int i;
    for (i = 0; i < 0x14; i++) {
        if (val == city_flag_list[i]) {
            clear_city_flag(val);
            return 1;
        }
    }
    if (put_city_flag(val)) return 1;
    return 0;
}

// Add or remove the province marker at a map location.
// FUNCTION: C2 0x6f1ed
// FUNCTION: C2WIN 0x004ae3f6
int toggle_prov_flag(int val)
{
    int i;
    for (i = 0; i < 0x14; i++) {
        if (val == prov_flag_list[i]) {
            clear_prov_flag(val);
            return 1;
        }
    }
    if (put_prov_flag(val)) return 1;
    return 0;
}

// Add a city marker if a marker slot is available.
// FUNCTION: C2 0x6f221
// FUNCTION: C2WIN 0x004ae470
int put_city_flag(int val)
{
    int i;

    count_city_flags();
    if (val == danger_flag_list[0]) return 1;
    if (no_of_city_flags >= 0x14) return 0;
    for (i = 0; i < 0x14; i++) {
        last_city_flag++;
        if (last_city_flag >= 0x14) last_city_flag = 0;
        if (city_flag_list[last_city_flag] == -1) {
            city_flag_list[last_city_flag] = val;
            (*(struct city_cell *)((unsigned char *)city_map + (val))).road_aqueduct = 1;
            return 1;
        }
    }
    return 0;
}

// Add a province marker if a marker slot is available.
// FUNCTION: C2 0x6f290
// FUNCTION: C2WIN 0x004ae529
int put_prov_flag(int val)
{
    int i;

    count_prov_flags();
    if (val == danger_flag_list[0]) return 1;
    if (no_of_prov_flags >= 0x14) return 0;
    for (i = 0; i < 0x14; i++) {
        last_prov_flag++;
        if (last_prov_flag >= 0x14) last_prov_flag = 0;
        if (prov_flag_list[last_prov_flag] == -1) {
            prov_flag_list[last_prov_flag] = val;
            (*(struct region_cell *)((unsigned char *)region_map + (val))).place_state = 1;
            return 1;
        }
    }
    return 0;
}

// Set the current danger marker.
// FUNCTION: C2 0x6f2ff
// FUNCTION: C2WIN 0x004ae5e2
int put_danger_flag(int val)
{
    danger_flag_list[0] = val;
    last_danger_flag = 0;
    return 1;
}

// Remove all city markers at a map location.
// FUNCTION: C2 0x6f312
// FUNCTION: C2WIN 0x004ae609
void clear_city_flag(int val)
{
    int i;
    for (i = 0; i < 0x14; i++) {
        if (city_flag_list[i] == val) {
            city_flag_list[i] = -1;
            (*(struct city_cell *)((unsigned char *)city_map + (val))).road_aqueduct = 0;
        }
    }
    count_city_flags();
}

// Remove all province markers at a map location.
// FUNCTION: C2 0x6f34c
// FUNCTION: C2WIN 0x004ae665
void clear_prov_flag(int val)
{
    int i;
    for (i = 0; i < 0x14; i++) {
        if (prov_flag_list[i] == val) {
            prov_flag_list[i] = -1;
            (*(struct region_cell *)((unsigned char *)region_map + (val))).place_state = 0;
        }
    }
    count_prov_flags();
}

// Clear the current danger marker.
// FUNCTION: C2 0x6f386
// FUNCTION: C2WIN 0x004ae6c1
void clear_danger_flag(void)
{
    danger_flag_list[0] = -1;
    count_danger_flags();
}

// Update the number of active danger markers.
// FUNCTION: C2 0x6f390
// FUNCTION: C2WIN 0x004ae295 REORDERED
void count_danger_flags(void)
{
    int i;
    no_of_danger_flags = 0;
    for (i = 0; i < 0x14; i++) {
        if (danger_flag_list[i] != -1)
            no_of_danger_flags++;
    }
}

// Select the next active city marker.
// FUNCTION: C2 0x6f3b3
// FUNCTION: C2WIN 0x004ae761
int next_city_flag(void)
{
    int i;

    count_city_flags();
    if (no_of_city_flags <= 0) return 0;
    for (i = 0; i < 0x14; i++) {
        last_city_flag++;
        if (last_city_flag >= 0x14) last_city_flag = 0;
        if (city_flag_list[last_city_flag] != -1) return 1;
    }
    return 0;
}

// Select the next active province marker.
// FUNCTION: C2 0x6f405
// FUNCTION: C2WIN 0x004ae7e7
int next_prov_flag(void)
{
    int i;

    count_prov_flags();
    if (no_of_prov_flags <= 0) return 0;
    for (i = 0; i < 0x14; i++) {
        last_prov_flag++;
        if (last_prov_flag >= 0x14) last_prov_flag = 0;
        if (prov_flag_list[last_prov_flag] != -1) return 1;
    }
    return 0;
}

// Select the next active danger marker.
// FUNCTION: C2 0x6f457
// FUNCTION: C2WIN 0x004ae6db REORDERED
int next_danger_flag(void)
{
    int i;

    count_danger_flags();
    if (no_of_danger_flags <= 0) return 0;
    for (i = 0; i < 0x14; i++) {
        last_danger_flag++;
        if (last_danger_flag >= 0x14) last_danger_flag = 0;
        if (danger_flag_list[last_danger_flag] != -1) return 1;
    }
    return 0;
}

// Switch the UI into flag-marker mode (used by the city- and province-level alert overlays).
// FUNCTION: C2 0x6f4a9
// FUNCTION: C2WIN 0x004ae86d
void goto_flag_marker_mode(void)
{
    int i;
    int cell_offset;

    flag_mode = 1;
    clear_all_cm(2);
    clear_all_rm(2);

    for (i = 0; i < 20; i++) {
        if (city_flag_list[i] != -1) {
            cell_offset = city_flag_list[i];
            (*(struct city_cell *)((unsigned char *)city_map + (cell_offset))).road_aqueduct = 1;
        }
        if (prov_flag_list[i] != -1) {
            cell_offset = prov_flag_list[i];
            (*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).place_state = 2;
        }
        if (danger_flag_list[i] != -1) {
            cell_offset = danger_flag_list[i];
            if (danger_flag_map_mode == 0)
                (*(struct city_cell *)((unsigned char *)city_map + (cell_offset))).road_aqueduct = 3;
            else
                (*(struct region_cell *)((unsigned char *)region_map + (cell_offset))).place_state = 3;
        }
    }
}
