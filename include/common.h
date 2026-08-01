#ifndef COMMON_H
#define COMMON_H

/* Page 0: final dword plus the complete ascending byte run. */
extern int ferret_energy;
extern unsigned char tb_occ_b_flag;
extern unsigned char tb_road_flag;
extern unsigned char tb_prev_flag;
extern unsigned char tb_occ_a_flag;

/* Page 1: the penultimate dword and globals consumed by common.c. */
extern int clock_ferret_x;
extern int D;
extern short army_a;
extern short army_no;
extern short arrow_no;
extern int barbarian_name_count;
extern short citizen_a;
extern short citizen_b;
extern short citizen_no;
extern short created_army_no;
extern short created_arrow_no;
extern short created_citizen_no;
extern short created_figure_no;
extern short created_unit_no;
extern int evolve_count;
extern int ex;
extern int ey;
extern char ferret_run[];
extern short figure_no;
extern short hunting_army;
extern int ferret_run_length;
extern int map_actual_atom;
extern int map_actual_height;
extern int map_actual_width;
extern unsigned char map_mode;

/* Page 2: the remaining 24 members of the first dword run.  Equal-hash
 * declarations are reversed so Watcom's hash chain emits PS order. */
extern int anti_ferret_moves;
extern int ferret_targ_y;
extern int last_anti_ferret_dirc;
extern int tb_y;
extern int tb_x;
extern int tb_ptr;
extern int anti_ferret_running;
extern int clock_ferret_moves;
extern int clock_ferret_ptr;
extern int clock_ferret_count;
extern int ferret_map_hi;
extern int clock_ferret_running;
extern int anti_ferret_count;
extern int ferret_map_wi;
extern unsigned char *ferret_map;
extern int anti_ferret_ptr;
extern int ferret_vert_off;
extern int ferret_home;
extern int clock_ferret_y;
extern int anti_ferret_x;
extern int anti_ferret_y;
extern int last_clock_ferret_dirc;
extern int ferret_targ_ptr;
extern int ferret_horiz_off;
extern int over_an_army;

/* Page 3: low-hash head emitted first. */
extern int ferret_targ_x;

#endif /* COMMON_H */
