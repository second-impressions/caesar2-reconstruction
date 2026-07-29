#ifndef WEB_H
#define WEB_H

/* Regional-road pathfinding node: 120 records, 8 bytes each. */
struct web_node {
    unsigned char kind;
    unsigned char out_of_walls;
    unsigned char _unused_writeonly02[2];
    unsigned char dirs;
    unsigned char from_dir;
    unsigned char x;
    unsigned char y;
};

typedef struct web_node web_node_rec;

int get_regroad_start_node(int, int);
int get_regroad_web(int, int);
int run_to_new_regroad_node(void);
int get_web_regroad_dircs(void);
int get_aqua_start_node(int, int);
int get_aqua_web(int, int);
int run_to_new_aqua_node(void);
int get_web_aqua_dircs(void);
int test_next_to_river(void);
void init_web(void);
void put_new_node(void);
int get_incomplete_node(int, int);
void set_first_nodes_values(int);
void push_nodes_values(char, int);
void push_node_value(char);

/* Page 0: high-hash tail values emitted after page 1 within each size class. */
extern struct web_node web[120];
extern int web_start_y;
extern int web_start_x;
extern int web_node;
extern unsigned char web_directions;

/* Page 1: low-hash heads of the dword and byte runs. */
extern int web_nof_dircs;
extern int web_node_count;
extern int web_y;
extern int web_x;
extern int web_first_actual_node;
extern int web_total_length;
extern int web_out_of_the_walls;
extern int web_ptr;
extern unsigned char web_from;
extern unsigned char web_dirc;

#endif /* WEB_H */
