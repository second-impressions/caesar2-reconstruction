#ifndef C2_BATTLE_DECLS_H
#define C2_BATTLE_DECLS_H

/*
 * Watcom allocates file-scope symbols from 25-entry symbol pages.  These
 * declarations reconstruct the four pages visible in battle.c's two PS BSS
 * hash runs.  The non-owned declarations are real battle state consumed by
 * this translation unit; they occupy the otherwise invisible header slots.
 */

/* Page 0: high-hash tail of the second owned dword run. */
extern int bat_no;
extern int bat_width;
extern int yrear;
extern int xright_back;
extern int yback;

/* Page 1: low-hash head of the second run plus battle-state header slots. */
extern int xright_front;
extern int bat_control;
extern int bat_size;
extern int yfront;
extern int bat_hi;
extern int xleft_front;
extern int ypos;
extern int xright_rear;
extern int bat_morale;
extern int xleft_back;
extern int bat_side;
extern int xpos;
extern int xleft_rear;
extern int battle_ai_count;
extern int battle_drag_start_x;
extern int battle_drag_start_y;
extern int battle_npc_retreat_count;
extern int battle_over_count;
extern int battle_scale;
extern int battle_state;
extern int battle_stats_control;
extern int battle_stats_men;
extern int battle_stats_morale;
extern int battle_stats_nof_units;
extern int battle_stats_start_men;

/* Page 2: remainder of the first run plus both armies' live state. */
extern int bat_order;
extern int bat_ai_trig_count;
extern int first_rear;
extern int bat_no_selected;
extern int bat_no_of_units;
extern int bat_which;
extern int y_bit;
extern int first_front;
extern int which_unit;
extern int x_bit;
extern int bat_attacker_clock;
extern int bat_odds;
extern int bat_spacing;
extern int first_back;
extern int battle_stats_type;
extern int battle_turbo;
extern int our_battle_men;
extern int our_battle_morale;
extern int our_battle_units;
extern int our_battle_stance;
extern int their_battle_men;
extern int their_battle_morale;
extern int their_battle_units;
extern int their_battle_stance;
extern int retreat_flag;

/* Page 3: low-hash head emitted first. */
extern int bat_attack_rate;

extern short figure_a;
extern short figure_no;
extern short temp_figure;

#endif /* C2_BATTLE_DECLS_H */
