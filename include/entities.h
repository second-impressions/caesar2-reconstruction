/* Entity record types for Caesar II
 *
 * Field names are inferred from Ghidra decompilation of create_*, remove_*,
 * and other entity-management functions.
 *
 * Watcom 10.0a defaults to 1-byte struct packing (PackAmount=1; proven in the
 * compiler binary -- see ReverseEngineering/watcom10.0a/docs/struct-packing.md),
 * so there is NO implicit alignment padding: every gap between named fields is
 * a REAL source-level field whose name we have not yet recovered.  Such gaps
 * are named by their byte offset with one of three prefixes:
 *   _unkNN              -- purpose unknown; no access seen in PS.EXE.
 *   _unused_writeonlyNN -- written by the game (e.g. at init/load) but never
 *                          read back in PS.EXE (dead state, or read only by
 *                          code we have not decompiled -- prime Mac-binary
 *                          cross-check candidates).
 *   _reservedNN         -- trailing struct-tail slack (the bytes between the
 *                          last named field and the struct's true stride).
 * All offsets are exact regardless.
 *
 * The figure_rec, unit_rec, citizen_rec, web_node and c2inf unknown/reserved/
 * write-only fields were cross-checked against the Mac PPC build (same source,
 * CodeWarrior): a global-access scan finds them unread there too, confirming
 * they are genuinely vestigial -- not just unread in our PS decompilation.
 * See docs/mac-deadfield-crosscheck.md (also notes a still-unmodelled live
 * c2inf window-mode field near +0x40 found during the scan).
 *
 * army_rec is defined in formulae.c (it has the most complete field mapping).
 * The structs here cover figure, citizen, unit, and arrow records.
 */
#ifndef ENTITIES_H
#define ENTITIES_H

/* Entity list extern declarations.
 * These override the generic types in c2_data.h. */
extern struct figure_rec figure_list[];
extern struct citizen_rec citizen_list[];
extern struct unit_rec unit_list[];
extern struct arrow_rec arrow_list[];
extern struct army_rec army_list[];
extern struct army_route_rec army_routes[];
extern struct web_node web[];
extern struct province_industry province_industries[];
extern struct mercs_class      mercenary_type[];
extern struct industry_rec     industry[];
/* Map dimensions and strides used pervasively in the code base.
 * The row-stride values appear as raw immediates in PS.EXE often
 * enough (region row-stride 480 = 18x, battle row-stride 208 = 4x)
 * that PS source almost certainly had named macros for them.
 * The map size is the product of W * H * CELL_BYTES, inlined by
 * the preprocessor when used as an array dimension. */
#define CITY_W              80
#define CITY_H              80
#define CITY_CELL_BYTES     20
#define CITY_ROW            (CITY_W * CITY_CELL_BYTES)        /* 1600 */

#define REGION_W            60
#define REGION_H            60
#define REGION_CELL_BYTES    8
#define REGION_ROW          (REGION_W * REGION_CELL_BYTES)    /* 480 */

#define BATTLE_W            52
#define BATTLE_H            52
#define BATTLE_CELL_BYTES    4
#define BATTLE_ROW          (BATTLE_W * BATTLE_CELL_BYTES)    /* 208 */

/* pseudo_map is the *isometric projection* of city_map used by the
 * pm_map0..3 drawing code.  Each cell is a 4-byte packed dword.
 * Dimensions are PM_H × PM_W = 161 × 81, holding the diamond
 * projection of an 80×80 source map.  Total = 52 164 bytes.
 *
 * Declared as a 2D array `int pseudo_map[PM_H][PM_W]` (not a flat
 * 1D int[]) because the PS.EXE codegen emits the row stride
 * baked into the type: a 2D type emits `y * sizeof(arr[0])` =
 * `y * 324` directly, while a 1D type indexed `arr[y*PM_W + x]`
 * would interleave the `+x` before the final `*4`, producing
 * different strength-reduction order.  See pm_map[1-3] scanline
 * functions for the natural 2D access pattern.
 *
 * Each cell holds one of two encodings:
 *
 *   * `value >= 0x0FFF0000` — a *sprite marker*: bottom byte is the
 *     empire-edge sprite kind (0..0xA used currently).  A marker is
 *     built as `0x0FFF0000 | kind`, its kind read back as
 *     `value - 0x0FFF0000`, and tested as `value >= 0x0FFF0000`.
 *   * otherwise — a raw `cm_ptr`: byte offset into city_map[]
 *     pointing at the source city cell this pseudo-cell projects.
 *     Read as `pseudo_map[y][x]`. */
#define PM_W                81
#define PM_H                161

/* city_map / region_map / battle_map are declared as struct <cell>[] after
 * each cell struct's definition (below). */
extern          int  pseudo_map[PM_H][PM_W];

/* Map cells are accessed by inline byte offset, not via macros or a
 * cached struct pointer (a cached `struct *` materialises the cell base
 * in a register and diverges from PS's folded `[base + disp32]`).  The
 * per-map field layouts live in the `struct city_cell` / `struct
 * region_cell` / `struct battle_cell` definitions (city_cell further
 * down in this header):
 *
 *   city_map[(off) + N]   /  cell[N]                       (20-byte cells)
 *   ((struct region_cell *)&region_map[off])->field         (8-byte cells)
 *   battle_map[(off) + N]  /  ((struct battle_cell *)&battle_map[off])->f
 *
 * The cell-offset itself is written explicitly per site as the
 * column-first `(x + y*W)*CELL_BYTES`. */

/* region_map (REGION_CELL_BYTES = 8) */
struct region_cell_fields {
    unsigned char base_kind;   /* +0x00 tile type / building kind (ENUM, not bits) */
    unsigned char terrain;     /* +0x01 tile-CATEGORY bits set by put_rm_area from
                                        regions.dat codes (see docs/cell-bitfields.md):
                                        0x01 placement-block (hut/border/city),
                                        0x02 out-of-walls, 0x04 army-passable,
                                        0x18 decorative land, 0x10 occupied/blocks-
                                        placement, 0x20 structure, 0x40/0x80 terrain
                                        category */
    unsigned char place_state; /* +0x02 flood-fill state: 0=unset, 0xff=blocked,
                                        else step distance -- NOT a bitfield */
    unsigned char edge_bits;   /* +0x03 edge bits (region): 0x01 has-army/citizen,
                                        0x02 overlay, 0x20 road, 0x40 had-goods,
                                        0x80 placement marker */
    unsigned char gfx;         /* +0x04 graphics-tile index */
    unsigned char _unused05;   /* +0x05 only zeroed, never read */
    unsigned char outside;     /* +0x06 single flag: 0x40 = outside-of-walls */
    unsigned char occupant;    /* +0x07 context-dependent packed byte (by base_kind):
                                        warehouse -> 0x0f level, 0xf0>>4 goods;
                                        multi-cell bldg / coast -> 0x03 corner/coast;
                                        evolve decay -> 0x1c>>2 timer, 0x80 in-radius;
                                        army cell -> whole byte = army index */
};
struct region_cell {
    union {
        struct region_cell_fields;                          /* .field named access */
        unsigned char b[sizeof(struct region_cell_fields)]; /* .b[i] indexed access */
    };
};
typedef char region_cell_size_check[sizeof(struct region_cell) == REGION_CELL_BYTES ? 1 : -1];
extern struct region_cell region_map[REGION_W * REGION_H];
/* region_map cell accessors (byte-offset cursor; RM_CELL(off) is the cell at
 * BYTE offset off; directional macros add a neighbour delta). */
#define RM_CELL(off)  (*(struct region_cell *)((unsigned char *)region_map + (off)))
/* No RM_N/RM_S/... directional macros: they were never used (the original
 * reads region neighbours via inline byte offsets, e.g.
 * ((unsigned char *)region_map)[gmn_sptr - 0x1de], confirmed by the Mac
 * build in trace_back_route_elastic) and contradicted the inline-cast rule
 * documented just below. */

/* region_map is a flat unsigned char[]; reach a cell's named fields by
 * casting a byte offset INLINE at each use:
 *     ((struct region_cell *)&region_map[off])->occupant
 * Re-form the cast at every access (do NOT cache it in a local pointer):
 * Watcom folds the `region_map + field` displacement into the disp32
 * exactly as PS.EXE does, whereas a cached `struct region_cell *c`
 * materialises the cell base in a register and diverges (proven byte-
 * for-byte in docs/codegen-experiments/region_cell_struct.py).
 *
 * The only region_map accesses left as raw byte indexing are the ones
 * whose FIELD offset is a runtime variable (clear_all_rm / unflag_all_rm
 * / set_4_rm_neighbours, where `layer` / `field_off` selects the field),
 * which a named struct field cannot express. */

/* battle_map (BATTLE_CELL_BYTES = 4) */
struct battle_cell_fields {
    unsigned char terrain;     /* +0x00 terrain / corpse terrain */
    unsigned char figure;      /* +0x01 occupying figure_no      */
    unsigned char dirty;       /* +0x02 dirty/highlight flags    */
    unsigned char arrow;       /* +0x03 arrow_no / aux layer     */
};
struct battle_cell {
    union {
        struct battle_cell_fields;                          /* .field named access */
        unsigned char b[sizeof(struct battle_cell_fields)]; /* .b[i] indexed access */
    };
};
typedef char battle_cell_size_check[sizeof(struct battle_cell) == BATTLE_CELL_BYTES ? 1 : -1];
extern struct battle_cell battle_map[BATTLE_W * BATTLE_H];
/* No BM_CELL / directional macros: battle_map cells are accessed inline as
 * (*(struct battle_cell *)((unsigned char *)battle_map + (off))) -- the macro
 * was an un-original accessor convenience and is gone. */

/* Game-wide preferences / persistent settings.
 *
 *   c2inf — 64-byte block at 0x0009CFF0 holding everything the
 *   "INF" file persists (sound levels, toggles, player name,
 *   skill level, etc.) plus a couple of runtime fields the
 *   load_inf path saves and restores across the file read.
 *
 *   `basic_inf_settings` (in lib32.c / loadsave.c land) is the
 *   canonical initialiser; it sets the values listed in the
 *   field comments below.
 *
 *   `act_tog_*` action-handlers in action.c are the per-field
 *   GUI toggles; their offsets correspond to one field each.
 *
 *   `region_trouble`, `evolve_region`, `random_event`,
 *   `army_intelligence`, ... 37+ functions branch on
 *   `peace_mode` (+0x35) to skip late-game systems while the
 *   tutorial / "peace" flag is set.
 */
struct c2inf_rec {
    unsigned char cd_letter;  /* +0x00  CD-ROM drive letter (ASCII)    */
                              /*        Saved/restored across load_inf */
    unsigned char drive_init; /* +0x01  flag: CD path resolved          */
                              /*        Saved/restored across load_inf */
    char  _unk02;             /* +0x02  unused                          */
    unsigned char speech_on;  /* +0x03  bool — Latin speech enabled    */
                              /*        toggled by act_tog_speech       */
    int   game_speed;         /* +0x04  default 100; act_game_speed     */
    int   scroll_speed;       /* +0x08  default 100; act_scroll_speed   */
    unsigned char samples_on; /* +0x0C  bool; act_tog_samples           */
    unsigned char tunes_on;   /* +0x0D  bool; act_tog_tunes             */
    int   samples_level;      /* +0x0E  0..100; act_samples_level       */
                              /*        UNALIGNED int — Watcom -zp1     */
    int   tunes_level;        /* +0x12  0..100; act_tunes_level         */
                              /*        UNALIGNED int — Watcom -zp1     */
    short starting_year;      /* +0x16  default 0x7d5 = 2005            */
    unsigned char paused;     /* +0x18  bool; act_pause / act_stop_go   */
    unsigned char anims_on;   /* +0x19  bool; act_tog_anims             */
    char  player_name[26];    /* +0x1A  null-terminated ("Octavian")    */
                              /*        25-char buffer + null;          */
                              /*        basic_inf_settings strcpy len   */
                              /*        is 0x19, ends at +0x33          */
    signed char skill_level;  /* +0x34  signed (movsx); career skill;   */
                              /*        act_skill_up / act_skill_down   */
    char  peace_mode;         /* +0x35  bool: 1 = no random events;     */
                              /*        act_tog_peace; 37+ readers      */
    char  _unused_writeonly36;             /* +0x36  init=1, only basic_inf_settings */
                              /*        writes; never read.  Mac PPC build */
                              /*        also only stores it -> dead both.  */
    unsigned char config37;   /* +0x37  config byte; load_inf reads     */
    char  _unused_writeonly38;             /* +0x38  init=0; only load_inf stores it; */
                              /*        no reader in PS or Mac build.    */
    unsigned char yearend_on; /* +0x39  bool; act_do_year_end / tog     */
    unsigned char ambients_on;/* +0x3A  bool; act_tog_ambients          */
    unsigned char autosave_on;/* +0x3B  bool; act_tog_autosave          */
    int   max_samples;        /* +0x3C  default 4 (concurrent voices);  */
                              /*        act_nof_samples                 */
    /* DOS struct ENDS at 0x40 (negative_buffer follows at c2inf+0x40).   */
    /* The Mac PPC build (same source) has an extra dual-window-mode flag */
    /* at its c2inf+0x45 (set_window_mode/select_window) -- a Mac-only UI */
    /* feature #ifdef'd out of the full-screen DOS build, so absent here. */
};                            /* total 0x40 = 64 bytes                  */
extern struct c2inf_rec c2inf;

/* Forum department ids used by forum_dept / forum_dept_over /
 * last_forum_dept.  Values come from forum_game_loop() and
 * show_forum_screen() dispatch tables plus the per-department
 * right-click exits in gloops.c. */
enum forum_dept_id {
    FORUM_DEPT_OVERVIEW = 0,  /* base/empty forum view */
    FORUM_DEPT_ADMIN    = 1,  /* administration / tax rates */
    FORUM_DEPT_CAREER   = 2,  /* career / personal cash */
    FORUM_DEPT_CLERKS   = 3,  /* clerks / history */
    FORUM_DEPT_ROME     = 4,  /* emperor / Rome */
    FORUM_DEPT_ADVISOR  = 5,  /* help/advisor entry */
    FORUM_DEPT_ARMY     = 6,  /* army / centurion */
    FORUM_DEPT_INDUSTRY = 7,  /* industry */
    FORUM_DEPT_SLAVES   = 8,  /* plebs / slaves */
    FORUM_DEPT_EXIT     = 9,  /* leave forum */
    FORUM_DEPT_TEMPLE   = 10, /* temple / oracle */
    FORUM_DEPT_EMPIRE   = 11, /* empire */
    FORUM_DEPT_END      = 12  /* one-past-last menu/dept id */
};

/* Per-class slave / labour-pool requirement.
 *
 *   slave_requirements[8] — one entry per industrial class; each
 *   entry tracks the *current* worker count and the *target*
 *   (max) it should converge to.  alter_slave_reqs / set/less/more
 *   in action.c walk slave_requirements[kind].current toward
 *   .max one step at a time.  Storage is owned by formulae.c.
 */
struct slave_req {
    int current;   /* +0x00 */
    int max;       /* +0x04 */
};  /* 8 bytes */
extern struct slave_req slave_requirements[8];

/* Per-cohort patrol/route table.  One route record is selected by
 * army_rec.cohort_id and has a 0x15a-byte stride in PS.EXE. */
struct army_route_point {
    char x;
    char y;
};

struct army_route_rec {
    int row_count;             /* +0x00 number of active route rows */
    int chase_row;             /* +0x04 row/progress for chase route */
    int target_army;           /* +0x08 low word is enemy army id */
    char army_x;               /* +0x0C army tile when route was set */
    char army_y;               /* +0x0D */
    char over_x;               /* +0x0E mouse-over tile when set */
    char over_y;               /* +0x0F */
    unsigned char row_len[10]; /* +0x10 */
    struct army_route_point points[10][16]; /* +0x1A, 0x20-byte rows */
}; /* 0x15A */

/* Century sub-record used within army_rec */
struct century {
    unsigned char type;     /* +0  */
    unsigned char damaged;  /* +1  1 = under-strength */
    unsigned char _u[2];
};

struct army_rec {
    signed char    exists;              /* +0x00 */
    signed char    sprite_image;        /* +0x01  sprite frame index for
                                            place2_sprite (movsx-read);
                                            written by all get_*_image
                                            helpers */
    signed char    sprite_anim;         /* +0x02  per-frame animation
                                            offset (cnt8 + base) added
                                            to sprite_image at draw;
                                            movsx-read (place2_sprite L814) */
    signed char    sprite_dir;          /* +0x03  per-direction sub-frame
                                            offset (walk_dirc result +
                                            base); set by the shared
                                            tail of get_*_image.
                                            movsx-read (place2_sprite L823) */
    signed char    type;                /* +0x04 army type (1=cohort) */
    /* Note: scalar char fields below keep default (unsigned) unless shown
     * otherwise. Fields that PS.EXE reads via `movsx byte` should be
     * promoted to `signed char` here. */
    signed char    world_dir;           /* +0x05  absolute world heading
                                            (0..7); combined with
                                            map_direction in get_*_image
                                            to compute screen sprite
                                            frame.  Always movsx-read. */
    signed char    x;                  /* +0x06  promoted to signed: PS uses
                                            movsx in 12+ functions for signed
                                            arithmetic on tile coords */
    signed char    y;                  /* +0x07  ditto */
    int            map_ref;            /* +0x08 */
    char           pixel_x;            /* +0x0C */
    char           pixel_y;            /* +0x0D */
    signed char    target_x;           /* +0x0E  next target tile X (the army
                                            walks toward target_x/target_y);
                                            written by target_from_army_dirc,
                                            new_army_route_point, sa04_army_
                                            attack, action handlers; movsx-
                                            read by region_go_to_target */
    signed char    target_y;           /* +0x0F  ditto */
    char           saved_state_idx;    /* +0x10  saved state to restore after wait */
    signed char    wait_count;         /* +0x11  ticks remaining in sa01_wait (signed) */
    signed char    state_idx;          /* +0x12  index into army_states[] (movsx) */
    signed char    stuck_timer;        /* +0x13  per-tick stuck/path-find counter
                                                  bumped by sa08_army_stuck;
                                                  > 4 → reset state to 1
                                                  (movsx-read) */
    char           wf_phase;           /* +0x14  walk-phase (0..wf_length) */
    char           wf_active;          /* +0x15  1 = walking ferret-run */
    signed char    wf_step;            /* +0x16  current step within wf_steps[] */
    signed char    wf_length;          /* +0x17  total steps in wf_steps[] */
    char           wf_steps[8];        /* +0x18..+0x1F  16 4-bit headings (2 per byte) */
    char           _unk20[2];          /* Mac PPC: also never accessed -- vestigial */
    char           heading;            /* +0x22  army facing direction */
    signed char    target_kind;        /* +0x23  some path-find/walk flag
                                            (movsx-read by entering_new_
                                            square; cleared with wf state) */
    signed char    target_count;       /* +0x24  another path-find flag
                                            (movsx-read by entering_new_
                                            square; cleared with wf state) */
    signed char    flags;              /* +0x25  army flag bits (movsx in sa##):
                                            0x01 active/moving, 0x02 state bit,
                                            0x08 sailing / sea-voyage; see
                                            docs/cell-bitfields.md */
    char           return_flag;        /* +0x26  set to 1 when target asked to return */
    char           target_flag;        /* +0x27  set when army has a chosen target */
    signed char    cohort_id;          /* +0x28  cohort/route slot id; PS emits movsx on read */
    char           dest_y;             /* +0x29  destination tile Y for sail-to-port */
    char           dest_x;             /* +0x2A  destination tile X for sail-to-port */
    char           _unk2B[1];          /* Mac PPC: also never accessed -- vestigial */
    int            fort_ref;           /* +0x2C */
    char           army_id;              /* +0x30 */
    char           _unk31;              /* Mac PPC: also never accessed -- vestigial */
    short          evolve_timer;       /* +0x32 */
    short          target_marker;      /* +0x34  cached enemy.evolve_timer
                                            from attack lock-on; sa04
                                            aborts attack if live
                                            enemy.evolve_timer no longer
                                            matches */
    short          map_x;              /* +0x36 */
    short          map_y;              /* +0x38 */
    int            departure_year;     /* +0x3A  year the army departed
                                            its home location; set by
                                            build_region_item / do_sea_trade */
    struct century centuries[14];       /* +0x3E..+0x75 (14 x 4 bytes) */
    int            num_horse;          /* +0x76 cavalry/horse troop count */
    int            num_auxillaries;    /* +0x7A */
    int            num_irregulars;     /* +0x7E */
    int            num_regulars;       /* +0x82 */
    int            num_specials;       /* +0x86 */
    int            total_troops;       /* +0x8A */
    int            assigned_needs;     /* +0x8E */
    unsigned char  num_centuries;       /* +0x92 */
    unsigned char  morale_timer;       /* +0x93  cohort morale-recovery countdown */
    unsigned char  morale;             /* +0x94 */
    unsigned char  readiness_level;    /* +0x95 */
    unsigned char  home_x;             /* +0x96  saved home tile X (region
                                            grid); used by sa12_army_sail_
                                            home as the destination to
                                            return to */
    unsigned char  home_y;             /* +0x97  saved home tile Y */
    unsigned char  compass_side;       /* +0x98  trader compass: 0=N 2=E 4=S 6=W */
    unsigned char  trader_brings;      /* +0x99  goods kind a sea trader is
                                            bringing on this voyage; set by
                                            do_sea_trade from
                                            <dir>_trader_brings global */
    unsigned char  battle_disposition; /* +0x9A  set to 10 for generated
                                            hostile-villager battle armies */
    unsigned char  source_region;      /* +0x9B  attacker's home province id
                                                  (set by attack_with_tribe /
                                                  raider_trouble / horde_trouble) */
    char           landed_flag;        /* +0x9C  set when ship has reached shore */
    unsigned char  tribe_id;           /* +0x9D  index into tribe_to_standard[] */
    char           quick_respawn;      /* +0x9E  set by sa13 ground-out; sa15 uses
                                            it to shorten the trader respawn delay
                                            (2 ticks instead of 15) */
    char           order_progress;     /* +0x9F  cleared when state restarts after return */
    unsigned char  cohort_size_class;  /* +0xA0 */
    short          target_timer;       /* +0xA1  ticks to reaching/holding target */
    int            home_ref;           /* +0xA3 */
    char           _reservedA7[0xAF - 0xA7];  /* Mac PPC: also never accessed -- vestigial */
};  /* 175 bytes (0xAF) */

struct figure_rec {
    char           exists;           /* +0x00 */
    signed char    owner;            /* +0x01  movsx-read by missile target
                                                friend/foe filters. */
    char           sprite_anim;      /* +0x02  written by the shared tail
                                                of get_fig_*_image (cnt8 +
                                                base offset); every read is
                                                unsigned (image index). */
    signed char    sprite_dir;       /* +0x03  per-direction sub-frame
                                                set by the get_fig_*_image
                                                shared tail (1 = facing). */
    unsigned char  fight_state;      /* +0x04  enum-like fight/animation state.
                                                build_units_figures sets
                                                1 (walk) or 2 (still);
                                                get_fig_*_image switches
                                                on this to pick which
                                                sprite-base to use. */
    signed char    sprite_type;      /* +0x05  dispatch/table index; PS uses
                                                signed compares for range
                                                checks. */
    signed char    direction;        /* +0x06  PS movsx-reads at every use
                                                site (heading 0..7; passed
                                                signed to get_heading). */
    signed char    fight_direction;  /* +0x07  facing direction during a fight
                                                (set by sf09 look_for_fight
                                                for both attacker and defender).
                                                movsx-read (get_fig_fight_image
                                                0x4F75E), never movzx; sits beside
                                                the signed `direction`. */
    signed char    grid_x;           /* +0x08 -- movsx on read in all
                                                consumers (get_highlight_
                                                position, get_fire_target,
                                                figure_go_to_target, ...). */
    signed char    grid_y;           /* +0x09 -- movsx on read */
    unsigned char *arrow_data_ptr;   /* +0x0A  per-figure sprite-data
                                                pointer; assigned from
                                                figure[1-7]_data by
                                                rebuild_figures_image_data
                                                based on sprite_kind, then
                                                passed as the first arg
                                                ('owner' slot) of create_arrow.
                                                Despite the slot's name in the
                                                ABI, this is sprite-data, not
                                                an owner_id. */
    unsigned char *sprite_data_ptr;  /* +0x0E  pointer into the
                                                figureN_data sprite block this
                                                figure renders from; assigned
                                                from figure5_data..figure8_data
                                                by rebuild_figures_image_data
                                                for elephant-class units. */
    int            map_ref;          /* +0x12 */
    signed char    archer_tick_a;    /* +0x16  per-tick AI counter for the
                                                primary elephant-archer slot;
                                                bumped every sf14_opertunist
                                                _fire tick and reset after
                                                firing. */
    signed char    archer_tick_b;    /* +0x17  per-tick AI counter for the
                                                secondary elephant-archer
                                                slot. */
    signed char    prev_grid_x;      /* +0x18 */
    signed char    prev_grid_y;      /* +0x19 */
    signed char    next_state_idx;  /* +0x1A  state to switch into after a
                                                wait expires — sf01_wait,
                                                figure_go_to_target, and
                                                set_ai_unit_delayed_beserk
                                                stash here while state_idx=1. */
    signed char    wait_counter;    /* +0x1B  decrementing tick counter
                                                consumed by sf01_wait; expiry
                                                triggers restore of
                                                next_state_idx to state_idx. */
    signed char    state_idx;        /* +0x1C  index into figure_states[]
                                                (movsx-read by f01_regular et al
                                                before tail-calling
                                                figure_states[state_idx]) */
    signed char    wf_searching;     /* +0x1D  bool: walk-finder search active
                                                (get_wf_dirc sweeps directions
                                                looking for a free square);
                                                signed because state_idx and
                                                surrounding bytes are signed-
                                                compared (jg/jl). */
    signed char    wf_orient;        /* +0x1E  XOR'd 0/1 each search
                                                (alternates which way the
                                                fallback inc/dec sweeps) */
    signed char    wf_dirc;          /* +0x1F  current candidate direction
                                                (0..7) being probed */
    signed char    wf_ttl;           /* +0x20  search lifetime counter
                                                (init 2; decremented per call;
                                                cleared resets wf_searching) */
    char           _unk21[1];  /* Mac PPC: also never accessed -- vestigial */
    signed char    wf_step_x;        /* +0x22  cleared by sf01_wait alongside
                                                wf_step_y; signed per PS movsx-read
                                                (figure_go_to_target wf_step_y/x
                                                post-inc compare). */
    signed char    wf_step_y;        /* +0x23 */
    signed char    is_visible;       /* +0x24  signed: PS movsx-reads
                                                the &2 result (sf03_move) */
    char           is_routing;       /* +0x25  set to 1 by sf12_rout when
                                                a unit panics; checked by
                                                figure_go_to_target so a
                                                routing figure walks off
                                                the map edge instead of
                                                pursuing a target. */
    unsigned char  sprite_kind;      /* +0x26  sprite-data-table selector
                                                (rebuild_figures_image_data
                                                dispatches on this).  Copied
                                                to arrow_rec.sprite_kind by
                                                get_arrow_base_image.
                                                Values 1..8 select figureN_data
                                                tables. */
    char           fight_swing_active; /* +0x27  set during the explicit swing
                                                  sub-state of
                                                  get_fig_fight_image so the
                                                  cap-8 swing animation runs
                                                  instead of the cap-12 idle
                                                  wobble. */
    signed char    death_timer;      /* +0x28  signed-byte timer initialised
                                                to 0x1e (30) by sf02_death
                                                and try_this_battlemap_square,
                                                decremented per frame; the
                                                halved value (>>1, capped at 7)
                                                indexes the death animation
                                                pose. */
    unsigned char  missile_target;   /* +0x29  latched enemy figure index
                                                for missile-fire targeting
                                                (sf10_hunt_for_fight,
                                                sf11_fire_missile). */
    char           _unk2A[1];  /* Mac PPC: also never accessed -- vestigial */
    unsigned char  unit_ref;         /* +0x2B */
    unsigned char  unit_type;        /* +0x2C */
    signed char    offset_x;         /* +0x2D */
    signed char    offset_y;         /* +0x2E */
    unsigned char  is_defending;     /* +0x2F  bool gating set_defense_shield's
                                                bonus +2 add */
    signed char    unit_position;    /* +0x30  fight-line position passed as
                                                p4 to get_y_spacing
                                                (multiplied with the
                                                row-number * spacing
                                                product) by
                                                get_fig_in_unit_position */
    signed char    kill_counter;     /* +0x31  per-tick combat tally; bumps
                                                by 1 each defense and once it
                                                reaches 10 a kill is awarded
                                                (do_the_fight). */
    signed char    defense;          /* +0x32  shield/defense accumulator;
                                                bumped by set_defense_shield */
    signed char    stampede_flag;    /* +0x33  set to 1 by sf02_death when an
                                                elephant figure begins its
                                                stampede death animation. */
    char           _unk34[1];  /* Mac PPC: also never accessed -- vestigial */
    char           opponent;         /* +0x35  current combat opponent's
                                                figure index (read by
                                                sf04_fight to look up the
                                                enemy figure_rec). */
    signed char    attack_count;     /* +0x36  per-figure attack-tally
                                                counter — init'd to .anim_kind
                                                by set_attack_count, modulated
                                                by .figure_rank / .is_defending
                                                / .shield_class / .owner;
                                                decremented per-tick by
                                                do_the_fight which gates the
                                                next combat resolution */
    unsigned char  fight_role;       /* +0x37  1 = attacker, 2 = defender
                                                (set by sf09_look_for_fight). */
    char           missile_max;     /* +0x38  per-figure missile cooldown
                                                threshold; set to 0x20 by
                                                sf06_defend / sf11_fire_missile
                                                and compared against
                                                missile_timer. */
    unsigned char  selected;         /* +0x39  set/cleared by
                                                select_all_figures /
                                                deselect_all_figures /
                                                select_a_unit; checked by
                                                show_aim_highlight,
                                                show_move_highlight,
                                                start_move, start_aim, etc.;
                                                zero-extended by the WIN build. */
    unsigned char  anim_counter;     /* +0x3A  per-figure animation tick
                                                counter (bumped by
                                                get_fig_walk_image, divided
                                                by 2 to index the walk
                                                cycle).  Signed-read by
                                                walk-image to drive
                                                `sar dl, 1`. */
    unsigned char  unit_grid_x;      /* +0x3B  per-figure column index within
                                                the parent unit's grid (read
                                                by get_fig_in_unit_position). */
    unsigned char  unit_grid_y;      /* +0x3C  per-figure row index within
                                                the parent unit's grid. */
    unsigned char  shield_class;     /* +0x3D  troop class — when == 2,
                                                set_defense_shield adds an
                                                extra +2 to .defense (heavy
                                                shield bonus). */
    char           anim_state;       /* +0x3E */
    char           backtrack_flag;   /* +0x3F  set by figure_go_to_target
                                                after a backtrack; checked
                                                by set_unit_to_fight. */
    unsigned char  missile_timer;    /* +0x40  per-tick missile-firing timer
                                                indexed into bow/sling/horsebow
                                                _images[] by
                                                get_fig_missile_image. */
    char           backtrack_dirc;   /* +0x41  heading copied to backtracking
                                                figures by set_unit_to_fight. */
    char           _unk42[2];  /* Mac PPC: also never accessed -- vestigial */
    char           stampede_kind;    /* +0x44  set to 1 by sf02_death for
                                                stampeding elephants. */
    char           anim_kind;        /* +0x45  per-figure-type animation
                                                slot set by f01_regular,
                                                f02_irregular, f03_auxillary,
                                                f05_barb_sword, etc. */
    unsigned char  sub_state;        /* +0x46  paired sub-state (3, 2, 1, …)
                                                set by the same dispatchers */
    unsigned char  figure_rank;      /* +0x47  troop role/class within its
                                                unit; values seen so far:
                                                  1 = front-rank back row
                                                  2 = back row (zeros .+0x4F
                                                       in sf08_withdraw)
                                                  4 = leader/centurion
                                                       (gates the centuries
                                                       count in
                                                       get_battle_centuries_left)
                                                Mirrored to unit_rec.+0x1A
                                                during build_units_figures.
                                                set_attack_count subtracts 2
                                                from .attack_count when the
                                                rank is 1 or 2. */
    signed char    archer_heading_a; /* +0x48  latched heading toward the
                                                 primary elephant-archer
                                                 target figure. */
    signed char    archer_heading_b; /* +0x49  latched heading toward the
                                                 secondary elephant-archer
                                                 target figure. */
    signed char    archer_image_a;  /* +0x4A  written by elephant_fire from
                                                elephant_archer_images[0]. */
    signed char    archer_image_b;  /* +0x4B  paired with archer_image_a. */
    signed char    archer_target_a; /* +0x4C  latched primary target figure
                                                 index for elephant archers. */
    signed char    archer_target_b; /* +0x4D  latched secondary target figure
                                                 index for elephant archers. */
    signed char    engaged_count;   /* +0x4E  nearby enemy engagement pressure */
    signed char    morale;          /* +0x4F  per-figure morale; written by
                                                sf08_withdraw from
                                                tribe_ai_data[bat_tribe*10+5],
                                                halved when figure_rank==1,
                                                zeroed when figure_rank==2. */
    char           _reserved50[8];  /* Mac PPC: also never accessed -- vestigial */
};  /* 88 bytes (0x58) */

struct citizen_rec {
    char           exists;           /* +0x00 */
    signed char    xp;               /* +0x01  fight experience (signed; bumps on win) */
    signed char    type;             /* +0x02  citizen kind (1..7); index
                                                into citizen_intelligences[]
                                                (movsx-read by
                                                citizen_intelligence) */
    signed char    world_dir;        /* +0x03  facing direction (0..7);
                                                movsx-read by
                                                get_movement_image and
                                                citizen state handlers */
    signed char    x;                /* +0x04  cell-grid X (random_target
                                                does signed compares) */
    signed char    y;                /* +0x05  cell-grid Y */
    int            map_ref;          /* +0x06 */
    char           pixel_x;          /* +0x0A */
    char           pixel_y;          /* +0x0B */
    signed char    dest_x;           /* +0x0C  wander/path target X
                                                (signed compare in
                                                random_target) */
    signed char    dest_y;           /* +0x0D  wander/path target Y */
    char           saved_state_idx; /* +0x0E  prior state to restore after wait */
    signed char    wait_count;      /* +0x0F  ticks remaining in s01_wait (signed; ≤0 expires) */
    signed char    state_idx;        /* +0x10  index into citizen_states[] (movsx) */
    char           wf_active;        /* +0x11  1 = walking ferret-run */
    signed char    wf_step;          /* +0x12  current step within wf_steps[] */
    signed char    wf_length;        /* +0x13  total steps in wf_steps[] */
    char           wf_steps[8];      /* +0x14..+0x1B  16 4-bit headings (2 per byte) */
    char           _unk1C[2];  /* Mac PPC: also never accessed -- vestigial */
    signed char    speed;           /* +0x1E  movsx-read (signed per PS) */
    signed char    speed_count;     /* +0x1F  ticks at current speed */
    signed char    speed_phase;     /* +0x20  fractional walk-phase */
    signed char    flag_bits;       /* +0x21  citizen flag bits: 0x01 on-road /
                                                pathfind-ok / active (movsx),
                                                0x02 transient */
    char           action_kind;     /* +0x22  s12_goto_riot=1, s11_riot=0 */
    char           is_barbarian;     /* +0x23 */
    signed char    state_timer;     /* +0x24  per-state tick counter (movsx) */
    char           _unk25;           /* +0x25 ; Mac PPC: also never accessed -- vestigial */
    signed char    market_demand_a; /* +0x26  > 0 ⇒ stage-A demand level
                                          stamped into city_cell.+9 bits
                                          0/1 by s10_get_business
                                          (1..7 → bit 1 set; ≥8 → bits
                                          0|1 set) */
    signed char    market_demand_b; /* +0x27  > 0 ⇒ stage-B demand level
                                          (bits 2/3 of city_cell.+9) */
    int            target_ref;       /* +0x28  cm_ptr of action target
                                         (fire / patrol / quell / etc.) */
    char           target_kind;      /* +0x2C  non-zero ⇒ target_ref valid
                                         (fire-fight, vigile-patrol, …) */
    char           target_count;     /* +0x2D  per-target tick (inc'd by
                                         s06_quell_trouble) */
    short          evolve_timer;     /* +0x2E */
    short          target_marker;    /* +0x30  saved target.evolve_timer
                                         (recycled-slot guard); set by
                                         s06_quell_trouble + s08_vigile_patrol
                                         when locking a target */
    char           name_id;          /* +0x32 */
    char           state;            /* +0x33 */
    short          image_id;         /* +0x34  current sprite image ID */
    char           wobble_counter;   /* +0x36  per-citizen vertical-wobble
                                                counter consumed in
                                                place_normal_node when the
                                                node is drawn at zoom 0;
                                                seeded to 0x10 from the
                                                node's flag-byte bit 6. */
    char           _reserved37[3];  /* Mac PPC: also never accessed -- vestigial */
};  /* 58 bytes (0x3A) */

struct unit_rec {
    unsigned char  x;                /* +0x00  zero-extended by the WIN build */
    unsigned char  y;                /* +0x01  zero-extended by the WIN build */
    int            current_men;      /* +0x02  current strength (men) of this
                                                unit; decremented as figures
                                                die, aggregated into
                                                battle_stats_men. */
    int            start_men;        /* +0x06  starting strength at build
                                                time; aggregated into
                                                battle_stats_start_men. */
    char           _unk0A[3];  /* Mac PPC: also never accessed -- vestigial */
    char           withdraw_flag;    /* +0x0D  set when AI orders unit withdrawal */
    int            target_lock;     /* +0x0E  4-byte gate: when zero, the
                                                unit is eligible to receive a
                                                new attack target via start_aim. */
    int            prev_attack_off;  /* +0x12  best previous attack cell offset
                                                seen by find_attack_target;
                                                latched to gate weaker targets. */
    unsigned char  type;             /* +0x16 */
    unsigned char  loss_tier;        /* +0x17  count of 20% losses bands the
                                                unit has crossed (incremented
                                                by update_units_morale once per
                                                additional 20% strength lost). */
    char           state;            /* +0x18 */
    char           _unk19[1];  /* Mac PPC: also never accessed -- vestigial */
    unsigned char  unit_rank;        /* +0x1A  mirrored from leader figure's
                                                .figure_rank by
                                                build_units_figures.  Drives
                                                AI period in do_light_ai
                                                (rank==2 → 60-tick period, else
                                                30) and morale gating in
                                                drop_all_units_morale. */
    unsigned char  exists;           /* +0x1B */
    char           owner;            /* +0x1C */
    signed char    morale_a;         /* +0x1D  signed-byte morale axis A,
                                                bumped by raise/drop_all_units
                                                _morale; capped to 100. */
    signed char    morale_b;         /* +0x1E  signed-byte morale axis B
                                                (paired with morale_a). */
    char           fatigue;          /* +0x1F  per-unit fatigue gauge; bumped
                                                by each successful arrow hit
                                                against one of the unit's
                                                figures, then bled off by
                                                update_units_morale (-5 per
                                                tick when >0x14, capped at
                                                0x32). */
    char           combat_order;     /* +0x20  AI/combat order: 3=move, 6=withdraw,
                                                7/3=flank, 8=withdraw, 0xa=berserk,
                                                0xc=rout.  Set by set_ai_unit_*
                                                helpers in battle.c. */
    char           _unk21[1];  /* Mac PPC: also never accessed -- vestigial */
    unsigned char  prev_x;           /* +0x22  zero-extended by the WIN build */
    unsigned char  prev_y;           /* +0x23  zero-extended by the WIN build */
    char           _unk24;            /* +0x24 ; Mac PPC: also never accessed -- vestigial */
    char           fatigue_alert;     /* +0x25  set to 1 by update_units_morale
                                                whenever fatigue is bled off
                                                this tick (consumer TBD). */
    char           has_selected_figs;/* +0x26  set by update_battle_stats when
                                                any of this unit's figures has
                                                .selected != 0; consumed by the
                                                same pass to populate the
                                                battle_stats_* panel readings. */
    char           is_target;        /* +0x27 */
    char           _unk28[2];  /* Mac PPC: also never accessed -- vestigial */
    unsigned char  regen_tick;        /* +0x2A  per-tick morale-regen counter;
                                                ticks up while target_lock == 0
                                                (unit disengaged) and on every
                                                26th tick bumps morale_a one
                                                step back toward morale_b. */
    char           _unk2B[1];  /* Mac PPC: also never accessed -- vestigial */
    unsigned char  first_figure;     /* +0x2C  byte index of first figure_list
                                                entry belonging to this unit */
    unsigned char  last_figure;      /* +0x2D  byte index of last figure_list
                                                entry (inclusive) */
    char           fig_count;        /* +0x2E  per-unit running count of
                                                alive figures, recomputed each
                                                tick by update_battle_stats.
                                                Each new figure assigned this
                                                value into its unit_type, so
                                                figures within a unit carry
                                                their position-index (0..N-1).
                                                Also read by set_ai_flank_move
                                                as the unit's grid width. */
    char           _unk2F;            /* +0x2F ; Mac PPC: also never accessed -- vestigial */
    char           formation_width;   /* +0x30  set in build_units_figures
                                                from bat_width; no current
                                                readers (PS may have used
                                                this for layout/save). */
    char           formation_cols;    /* +0x31  set in build_units_figures
                                                from cols; no current readers */
    char           _init32;            /* +0x32  set to 1 in build_units_figures;
                                                no current readers */
    char           _unk33;            /* +0x33 ; Mac PPC: also never accessed -- vestigial */
    signed char    heading;           /* +0x34  movsx-read, never movzx; signed. */
    char           stage_slot;        /* +0x35  set in build_units_figures
                                                from stage_slot arg; no
                                                current readers */
    unsigned char  formation_mode;   /* +0x36  formation kind 0..3; written
                                                by reform / instant_reform. */
    char           _unk37[2];  /* Mac PPC: also never accessed -- vestigial */
    unsigned char  unit_sub_kind;     /* +0x39  unit sub-kind tag set by
                                                build_units_figures from
                                                sub_kind2 argument; readers
                                                check non-zero to gate
                                                attack-marker logic in
                                                sf06_defend, show_aim_highlight,
                                                start_aim, update_units_ai. */
    unsigned char  attack_marker_x;   /* +0x3A  hlite_left at the moment of aim */
    unsigned char  attack_marker_y;   /* +0x3B  hlite_top  at the moment of aim */
    char           _unk3C;            /* +0x3C ; Mac PPC: also never accessed -- vestigial */
    unsigned char  flank_pending;     /* +0x3D  pending flank manoeuvre kind
                                                (1..4 dispatch index); cleared
                                                after do_heavy_ai fires
                                                set_ai_flank_move. */
    short          ai_period;        /* +0x3E  ticks between AI evaluations
                                                for this unit; set in
                                                build_units_figures, compared
                                                by do_light_ai / do_heavy_ai /
                                                elephant_ai */
    short          ai_tick;          /* +0x40  current AI-tick counter,
                                                bumped each call and reset to
                                                0 when it crosses ai_period */
    char           manoeuvre_done;    /* +0x42  set by set_ai_flank_move to
                                                gate further wedge/forward
                                                manoeuvres in do_heavy_ai. */
    char           _reserved43[11];  /* Mac PPC: also never accessed -- vestigial */
};  /* 78 bytes (0x4E) */

struct arrow_rec {
    short          start_x;          /* +0x00 */
    short          start_y;          /* +0x02 */
    short          end_x;            /* +0x04 */
    short          end_y;            /* +0x06 */
    unsigned char *arrow_data_ptr;   /* +0x08  sprite-data pointer copied
                                                from figure_rec.arrow_data_ptr
                                                via create_arrow's 'owner'
                                                parameter; later overwritten
                                                by rebuild_figures_image_data
                                                from figure[1-8]_data. */
    signed char    grid_x;           /* +0x0C -- movsx on read */
    signed char    grid_y;           /* +0x0D -- movsx on read */
    int            step_x;           /* +0x0E Bresenham x-axis delta */
    int            step_y;           /* +0x12 Bresenham y-axis delta */
    int            step_error;       /* +0x16 Bresenham error term */
    char           sprite_anim;      /* +0x1A  per-tick sprite frame to draw
                                                (sprite_base + rotated heading);
                                                set by arrow_intelligence. */
    unsigned char  sprite_kind;      /* +0x1B  sprite-data-table selector
                                                copied from figure_rec.sprite_kind
                                                by get_arrow_base_image. */
    unsigned char  sprite_base;      /* +0x1C  base sprite frame for the
                                                arrow's weapon class; one of
                                                0xAA/0x28/0x50/0 written by
                                                get_arrow_base_image. */
    unsigned char  anim_count;       /* +0x1D */
    unsigned char  anim_delta;       /* +0x1E  subtracted from anim_count per tick */
    char           _unk1F[1];        /* +0x1F  reserved slack: NO access anywhere
                                                in PS.EXE (whole-binary disasm +
                                                byte scan; every other arrow_rec
                                                field is touched) and 0 in all 41
                                                sample saves.  arrow_rec is not a
                                                Mac TOC global, so PS is the sole
                                                witness (see mac-deadfield-
                                                crosscheck.md). */
    signed char    fire_range;       /* +0x20  per-arrow range in cells
                                                (set by set_missile_fire_range
                                                 from weapon kind; read by
                                                 fly_to_target via movsx).
                                                Values seen: 30, 40, 60, 70. */
    char           flight_done;      /* +0x21  cleared before fly_to_target */
    char           axis_dominant;     /* +0x22  Bresenham axis tag set by
                                                init_bd: 2 = vertical-dominant
                                                (dy>dx), 1 = horizontal-or-
                                                equal.  Read by bd() to choose
                                                which axis to step on. */
    unsigned char  exists;           /* +0x23 */
    unsigned char  flight_age;       /* +0x24  per-tick age counter bumped
                                                each call to fly_to_target;
                                                when it exceeds fire_speed the
                                                arrow expires (clear_arrow). */
    unsigned char  fire_speed;       /* +0x25  per-arrow flight pacing
                                                (set by set_missile_fire_range
                                                 from weapon kind; read by
                                                 fly_to_target as unsigned).
                                                Values seen: 30, 50, 100, 120. */
    char           weapon_kind;      /* +0x26  weapon-class tag copied from
                                                the firing figure's sprite_kind
                                                (figure_rec.sprite_kind at +5)
                                                and consumed by
                                                set_missile_fire_fx /
                                                set_missile_fire_range to drive
                                                fire_range (+0x20) and
                                                fire_speed (+0x25). */
    char           heading;          /* +0x27 */
    char           owner;            /* +0x28  owner_id of the figure that
                                                fired this arrow; checked in
                                                fly_to_target friend/foe
                                                filter (fig->owner ==
                                                ar->owner -> skip).  Copied
                                                from the firing figure_rec's
                                                .owner by create_arrow. */
    int            map_ref;          /* +0x29 */
};  /* 45 bytes (0x2D) */

/* Per-province industry slot (set by set_new_province in empire.c).
 *
 *   province_industries[8] — 4 slots per province + 4 neighbour rows;
 *   each entry is 4 ints (16 bytes), of which only kind/is_trader
 *   are referenced so far.
 */
struct province_industry {
    int kind;        /* +0x00  industry kind 0..15 */
    int is_trader;   /* +0x04  0=produce, 1=missing-trader, 2=neighbour-source */
    int _unk08;      /* +0x08  reserved slack: NO read or write anywhere in
                              PS.EXE (confirmed by a whole-binary capstone
                              disasm scan + an alignment-independent byte
                              scan; only +0x00/+0x04 are ever touched) and 0
                              in all 41 sample saves.  province_industries is
                              not a Mac TOC global, so PS is the only witness. */
    int _unk0C;      /* +0x0C  reserved slack: ditto (no access in PS.EXE). */
};  /* 16 bytes */

/* Per-industry book-keeping (one entry per industry kind).
 *
 * Field meanings consolidated from message.c (which previously had a
 * file-local copy named struct industry_rec).  set_new_province in
 * empire.c writes industry[kind].status = 1 when the province has
 * that industry; other fields track the supply pipeline.
 */
struct industry_rec {
    int status;        /* +0x00  1 = this industry is set up somewhere */
    int supply;        /* +0x04  (= data_042D28 anchor)                 */
    int delivered;     /* +0x08 */
    int unit_size;     /* +0x0C */
    int count;         /* +0x10 */
    int has_supply;    /* +0x14 */
    int city_supply;   /* +0x18 */
    int supply_pipeline[5]; /* +0x1C..+0x2F  census supply/demand pipeline:
                               [0]=+0x1C population/demand ratio (read by
                               queries as q_local); [1]=+0x20 / [2]=+0x24 a
                               two-stage shift window updated each census;
                               [3]/[4] init-only.  (was _f1C; real fields) */
};  /* 48 bytes */

/* Mercenary class entry, indexed by province ("mercs_from").
 *
 *   mercenary_type[44] — 8 shorts per entry (16 bytes); fields named
 *   from set_new_province / similar references.
 */
struct mercs_class {
    short mercs_from;          /* +0x00 source province / tribe */
    short category;            /* +0x02 */
    short max_allowed;         /* +0x04 */
    short cost_per_50;         /* +0x06 */
    short _unk08;              /* +0x08  reserved slack: NO access anywhere in
                                        PS.EXE (whole-binary disasm scan; only
                                        +0x00/+0x02/+0x04/+0x06 are touched).
                                        Config table (mercenary_type[44]), not
                                        a savegame block. */
    short _unk0A;              /* +0x0A  reserved slack: ditto */
    short _unk0C;              /* +0x0C  reserved slack: ditto */
    short _unk0E;              /* +0x0E  reserved slack: ditto */
};  /* 16 bytes */

/* City-map field layout (20 bytes per cell, 80x80 = 128 000 bytes).
 *
 * city_map is declared above as `unsigned char[CITY_W*CITY_H*
 * CITY_CELL_BYTES]`.  All 832 instruction-level city_map accesses
 * in PS.EXE use byte addressing; there are zero word/dword reads,
 * confirming the byte-array type.  Indexed in byte-offset units:
 * `cm_off = (y * CITY_W + x) * CITY_CELL_BYTES`.  Row stride
 * CITY_ROW = 1600 bytes.
 *
 * `city_map` is `struct city_cell[CITY_W*CITY_H]`.  Map cursors
 * (cm_sptr, gmn_sptr, sptr, ptr, ...) are BYTE offsets (cm_sptr +=
 * CITY_CELL_BYTES per column, += CITY_ROW per row).  Access every cell
 * through the CM_* macros below, which cast `(unsigned char *)city_map +
 * off` to the typed cell so the byte-offset cursor folds into PS's
 * `[base + disp32]` addressing:
 *     CM_CELL(off).field        current cell, named field
 *     CM_S(off).terrain         neighbour cell, named field
 *     CM_S(off).b[field_off]    neighbour cell, RUNTIME field index
 *     CM_CELL(off).b            byte pointer to the cell (helper args)
 * All of these are byte-for-byte identical to raw `city_map[off + N]`
 * indexing (proven in docs/codegen-experiments/city-map-*.py).
 *
 * Do NOT *cache* a `struct city_cell *cell = &CM_CELL(off)` and read
 * `cell->field`: that materialises the cell base in a register and
 * diverges from PS.  Re-form CM_CELL inline at every access.
 *
 * The cell carries an anonymous-union byte overlay `b[]` so a field
 * chosen at RUNTIME (the generic flag/range engine: set_4_neighbours,
 * flag_range, set_range, unflag_all_cm) reads as `CM_*(off).b[field_off]`
 * instead of raw byte indexing. */
struct city_cell_fields {
    unsigned char base_kind;     /* +0x00 base structure / category byte;
                                          road_ramifications copies this to
                                          +9 ("intrinsic" kind) */
    unsigned char terrain;       /* +0x01 placed-structure / feature BITFIELD; each
                                          bit = the act_* placing_flags stamped on
                                          placement (see docs/cell-bitfields.md):
                                            0x01 b0 building (house/forum/temple/...)
                                            0x02 b1 wall      (act_wall)
                                            0x04 b2 tower     (act_tower; wall-gate)
                                            0x08 b3 natural water/river variant
                                            0x10 b4 river / bridge tile
                                            0x20 b5 road      (act_road)
                                            0x40 b6 aqueduct  (act_aquaduct)
                                            0x80 b7 reservoir (act_resevoir)
                                          composites: 0x18=water, 0xe7=any structure
                                          (illegal to build; =~0x18), 0x8b=citizen-
                                          blocked, 0x54=roman-spawn-blocked */
    unsigned char road_aqueduct; /* +0x02 road / aqueduct placement bitmap */
    unsigned char edge_bits;     /* +0x03 edge / render bits (see docs/cell-bitfields.md):
                                            0x01 b0  on-road/has-citizen/needs-redraw
                                            0x02 b1  drawn (render-pass) marker
                                            0x1c b2-4 wall/riverbank graphic sub-kind
                                                      (from wall_gfxdat[].edge_bits)
                                            0x20 b5  on water-network / web path
                                            0x40 b6  graphic orientation flag
                                            0x80 b7  draw-on-top / tall (wall/tower) */
    unsigned char extra_edge;    /* +0x04 secondary sprite/graphic index (overlay
                                          tile from house_gfxdat/wall_gfxdat) --
                                          a value, NOT a bitfield */
    unsigned char activity_a;    /* +0x05 cumulative activity counter
                                          (evolve_security/industrial_activity) */
    unsigned char activity_b;    /* +0x06 second activity counter */
    unsigned char citizen_a;     /* +0x07 citizen slot A (index into citizen_list) */
    unsigned char citizen_b;     /* +0x08 citizen slot B */
    unsigned char building;      /* +0x09 active building / business type
                                          (business_output, market_image, ...) */
    unsigned char range_flag;    /* +0x0A transient range marker
                                          (test_range_for, push_*) */
    unsigned char fpu_flag;      /* +0x0B fire/plague/unrest, nibble-packed:
                                            0x0f (b0-3) unrest level (0..15)
                                            0x30 (b4-5) plague/health (0x30=plague)
                                            0xc0 (b6-7) fire level (0..3) */
    unsigned char entertainment; /* +0x0C entertainment overlay */
    unsigned char education;     /* +0x0D education / population (set by census) */
    unsigned char health;        /* +0x0E health overlay */
    unsigned char land_value;    /* +0x0F land value (evolve_land_*,
                                          cap_land_value, change_lv) */
    unsigned char fire;          /* +0x10 fire state */
    unsigned char security;      /* +0x11 security overlay */
    unsigned char industrial;    /* +0x12 industrial activity overlay */
    unsigned char business;      /* +0x13 business index */
};  /* 20 bytes = CITY_CELL_BYTES */

struct city_cell {
    union {
        struct city_cell_fields;                          /* .field named access */
        unsigned char b[sizeof(struct city_cell_fields)]; /* .b[i] indexed access */
    };
};
/* compile-time guard: cell size must stay == CITY_CELL_BYTES */
typedef char city_cell_size_check[sizeof(struct city_cell) == CITY_CELL_BYTES ? 1 : -1];

/* city_map: an array of cells (same 128000 bytes as the old unsigned char[]).
 * Cursors remain BYTE offsets; access via the CM_* macros below. */
extern struct city_cell city_map[CITY_W * CITY_H];

/* city_map cell accessors (see the comment block above).  CM_CELL(off) is the
 * typed cell at BYTE offset off; the directional macros add a neighbour delta. */
#define CM_CELL(off)  (*(struct city_cell *)((unsigned char *)city_map + (off)))
/* No directional CM_* macros: city-map neighbours are accessed inline with an
 * explicit byte delta, e.g.
 * (*(struct city_cell *)((unsigned char *)city_map + ((off) - CITY_ROW))). */

/* Region-map field layout (8 bytes per cell, 60x60 = 28 800 bytes).
 *
 * region_map is declared above as `unsigned char[REGION_W*REGION_H*
 * REGION_CELL_BYTES]`.  All 84 instruction-level region_map accesses
 * in PS.EXE use byte addressing; there are zero word/dword reads,
 * confirming the byte-array type.  Indexed in byte-offset units:
 * `rm_off = (y * REGION_W + x) * REGION_CELL_BYTES`.  Row stride
 * REGION_ROW = 480 bytes appears 18x as a raw immediate.
 *
 * Per-cell field layout (byte offset within the 8-byte cell):
 *
 *   +0  base_kind     tile type / building kind
 *                     0x98..0x9F = region edge marker
 *                     0xD4       = barbarian
 *                     0xD5..0xFF = multi-cell building tiles
 *                     (q_type in queries)
 *   +1  terrain       passability / road bitfield
 *                     bit 0x02 = road bit
 *                     bit 0x04 = water adjacent
 *                     bit 0x20 = road tile
 *                     mask 0x25 in web.c = walkable
 *   +2  place_state   placement / cursor state
 *                     0xff = blocked-by-cursor
 *                     1    = active placement
 *                     (act_start_pm_ptr +2 toggle)
 *   +3  edge_bits     edge / road / has-goods
 *                     bit 0x20 = road
 *                     bit 0x40 = had goods
 *                     cleared mask 0xd8 +2 set (map.c destroy/build)
 *   +4  gfx           graphics-tile index / house variant / color
 *                     (q_gfx in queries; set to first_choice in
 *                      build paths, color in place_a_building)
 *   +5  _unused       only zeroed by clear_reg_basic; no reads
 *                     found in 464 region_map source references
 *   +6  outside       outside-of-walls flag (bit 0x40)
 *                     (web.c web_out_of_walls)
 *   +7  occupant      packed cell payload — semantics depend on
 *                     base_kind of the cell:
 *                       * army cells: = army_no
 *                         (int_c2.c 1237–1238, common.c 93/126/327)
 *                       * multi-cell building tiles (kind >= 0xD5):
 *                         bit 0 = +1 X subcell, bit 1 = +1 Y
 *                         (mask & 3 = corner; int_c2 3788–3789)
 *                       * warehouse cells: low nibble = level,
 *                         high nibble = goods kind
 *                         (q_wh_level, q_goods)
 *                       * land/sea tagging: bit 0x80 = was_sea
 *                         (int_c2.c 3800)
 */
/* The 8-byte cell layout above is modelled by `struct region_cell`
 * (defined with the RM_* accessors near the top of this header). */

/* Battle-map field layout (4 bytes per cell, 52x52 = 10 816 bytes).
 *
 * battle_map is declared above as `unsigned char[BATTLE_W*BATTLE_H*
 * BATTLE_CELL_BYTES]`.  Indexed in byte-offset units `bm_off =
 * (y * BATTLE_W + x) * BATTLE_CELL_BYTES`.  Row stride BATTLE_ROW =
 * 208 bytes appears 4x as a raw immediate.
 *
 * Per-cell field layout (byte offset within the 4-byte cell):
 *
 *   +0  terrain       base terrain / tile kind.  landfill masks
 *                     low 3 bits (& 7) for landfill class;
 *                     pm_map3.c branches on this byte's value.
 *   +1  figure_no     figure slot on this cell; 0 = empty.
 *                     Stamped by common.c create_a_figure (171/174)
 *                     and cleared in destroy paths.
 *   +2  dirty_flags   dirty / overlay bits for battle redraw
 *                     (battle.c 302/1813 |= 4).
 *   +3  arrow_no      active arrow slot index (battle.c 5369+;
 *                     arrow_a / arrow_no).
 */

struct refresh_bank_row {
    int split;       /* +0x00 non-zero when row crosses an SVGA bank */
    int bank;        /* +0x04 bank id for the row/split */
    int part_rows;   /* +0x08 number of 16-pixel chunks in lower bank */
    int split_col;   /* +0x0C first 16-pixel column in upper bank */
}; /* 0x10 */
extern struct refresh_bank_row refresh_bank_switch_data[];

struct hut_rec {
    unsigned char x;
    unsigned char y;
    unsigned char kind;
}; /* 0x03 */
extern struct hut_rec hut_list[4];

struct point_rec {
    int x;
    int y;
};
extern struct point_rec forum_menu[];

struct short_point_rec {
    short x;
    short y;
};
extern struct short_point_rec empire_positions[];
extern struct short_point_rec empire_flag_positions[];

struct byte_point_rec {
    unsigned char x;
    unsigned char y;
};
extern struct byte_point_rec temp_route[];

struct byte_delta_rec {
    signed char dx;
    signed char dy;
};
extern struct byte_delta_rec gmn_ofsets[];
extern struct byte_delta_rec elephant_stampede[];

struct attack_pos_rec {
    unsigned char kind;
    unsigned char xpos;
    unsigned char ypos;
};
extern struct attack_pos_rec attack_pos_data[];

struct int_delta_rec {
    int dx;
    int dy;
};
extern struct int_delta_rec line_flank_data[];
extern struct int_delta_rec col_flank_data[];
extern struct int_delta_rec putouts1[];
extern struct int_delta_rec putouts2[];
extern struct int_delta_rec putouts3[];
extern struct int_delta_rec putouts4[];
extern struct byte_delta_rec fire_offs[];

extern signed char walking_x_ofsets_zoom0[];
extern signed char walking_y_ofsets_zoom0[];
extern signed char walking_x_ofsets_zoom1[];
extern signed char walking_y_ofsets_zoom1[];
extern signed char walking_x_ofsets_zoom2[];
extern signed char walking_y_ofsets_zoom2[];
extern signed char fig_walking_x_ofsets_z1[];
extern signed char fig_walking_y_ofsets_z1[];
extern signed char fig_walking_x_ofsets_z2[];
extern signed char fig_walking_y_ofsets_z2[];
extern struct int_delta_rec arena_top_data[][4];
extern struct int_delta_rec colos_top_data[][4];

struct help_redirect_rec {
    int page;
    int replacement;
};
extern struct help_redirect_rec help_redir_ent_history[];

struct lv_effect_rec {
    int delta;
    int radius;
};
extern struct lv_effect_rec house_lv_effect[];
extern struct lv_effect_rec forum_lv_effect[];
extern struct lv_effect_rec temple_lv_effect[];

struct evolution_threshold_rec {
    signed char devolve_below;
    signed char evolve_above;
};
extern struct evolution_threshold_rec house_evolution[];
extern struct evolution_threshold_rec well_evolution[];
extern struct evolution_threshold_rec fountain_evolution[];
extern struct evolution_threshold_rec baths_evolution[];
extern struct evolution_threshold_rec forum_evolution[];
extern struct evolution_threshold_rec temple_evolution[];

struct population_threshold_rec {
    int devolve_below;
    int evolve_above;
};
extern struct population_threshold_rec temple_populations1[];
extern struct population_threshold_rec temple_populations2[];
extern struct population_threshold_rec temple_populations3[];

struct house_unrest_rec {
    unsigned char unrest_delta;
    char _reserved01[3];
};
extern struct house_unrest_rec house_type_to_unrest[];

extern int stretch_ofsets_2x2[][3];
extern int stretch_ofsets_3x3[][5];
extern char directory[][13];

/* VESA VBE 1.x SuperVGA information block (function 4F00h). */
struct vbe_info_block {
    char           signature[4];        /* +0x00 "VESA" */
    unsigned short version;             /* +0x04 BCD: hi=major, lo=minor */
    unsigned short oem_string_off;      /* +0x06 OEM string real-mode off */
    unsigned short oem_string_seg;      /* +0x08 OEM string real-mode seg */
    unsigned int   capabilities;        /* +0x0A */
    unsigned short video_mode_off;      /* +0x0E mode list real-mode off */
    unsigned short video_mode_seg;      /* +0x10 mode list real-mode seg */
    unsigned short total_memory;        /* +0x12 in 64K units */
    char           reserved[236];       /* +0x14 padding to 256 bytes */
};
extern struct vbe_info_block vesa_info;

/* VESA VBE 1.x ModeInfoBlock (function 4F01h). */
struct vbe_mode_info {
    unsigned short mode_attributes;     /* +0x00 */
    unsigned char  win_a_attributes;    /* +0x02 */
    unsigned char  win_b_attributes;    /* +0x03 */
    unsigned short win_granularity;     /* +0x04 in KB */
    unsigned short win_size;            /* +0x06 in KB */
    unsigned short win_a_segment;       /* +0x08 */
    unsigned short win_b_segment;       /* +0x0A */
    unsigned short win_func_off;        /* +0x0C bank-switch fn real-mode off */
    unsigned short win_func_seg;        /* +0x0E bank-switch fn real-mode seg */
    unsigned short bytes_per_scan_line; /* +0x10 */
    char           reserved[238];       /* +0x12 padding to 256 bytes */
};
extern struct vbe_mode_info vesa_mode_info;

extern int promotion_levels[][20];
extern int promotion_av_levels[][20];
extern unsigned char province_completion_to_promotion[][45];
extern char events[][64];

struct region_border_rec {
    union {
        struct {
            unsigned char north;
            unsigned char east;
            unsigned char south;
            unsigned char west;
        } side;
        unsigned char dir[4];
    } u;
};
extern struct region_border_rec region_borders[];

struct troop_numbers_rec {
    int specials;
    int horse;
    int regulars;
    int irregulars;
    int auxiliaries;
};
extern struct troop_numbers_rec tribe_to_troop_numbers[];

struct region_source_rec {
    unsigned char primary;
    unsigned char choices[9];
};
extern struct region_source_rec region_sources[];

struct tribe_battle_setup_rec {
    union {
        struct {
            unsigned char front_quirk;
            unsigned char middle_figure;
            unsigned char rear_figure;
            unsigned char flank_quirk;
            unsigned char front_kind;
            unsigned char middle_kind;
            unsigned char rear_kind;
        } f;
        unsigned char raw[7];
    } u;
};
extern struct tribe_battle_setup_rec tribe_battle_setup[];

struct tribe_ai_rec {
    unsigned char aggression;       /* +0 auto-resolve score bias */
    unsigned char berserk_count;    /* +1 battle_ai_count threshold for berserk */
    unsigned char delayed_berserk;  /* +2 == 1 → use delayed-berserk dispatch */
    unsigned char wedge_move;       /* +3 enables position-based wedge move */
    unsigned char forward_move;     /* +4 enables straight forward move */
    unsigned char base_morale;      /* +5 initial figure morale / withdraw threshold */
    unsigned char prefer_cohesion;  /* +6 == 1 → penalise mixed-species targets */
    unsigned char prefer_column;    /* +7 == 1 → column formation flank moves */
    unsigned char no_flanks;        /* +8 == 0 → suppress enemy flank units */
    unsigned char no_fans;          /* +9 == 0 → suppress enemy fan units */
};
extern struct tribe_ai_rec tribe_ai_data[];

/* 12-byte record consumed by choose_from / init_choices: 8 match-flag
 * bytes, then chosen sprite ID, info byte, max counter, current counter. */
struct choice_rec {
    unsigned char match[8];
    unsigned char value;
    unsigned char info;
    unsigned char max_count;
    unsigned char counter;
};
extern struct choice_rec river_data[];
extern struct choice_rec road_data[];
extern struct choice_rec wall_data[];
extern struct choice_rec tower_data[];
extern struct choice_rec gateway_data[];
extern struct choice_rec gateway2_data[];
extern struct choice_rec wallaqua_data[];
extern struct choice_rec aquawall_data[];
extern struct choice_rec aquaroad_data[];
extern struct choice_rec aquaduct_data[];
extern struct choice_rec regwallroad_data[];
extern struct choice_rec resevoir_data[];
extern struct choice_rec coast_data[];

enum {
    WALL_GFX_FIRST_TILE = 0xc1,
    SAILABLE_SEA_FIRST_TILE = 0x20
};

/* 4-byte wall/aqueduct graphics metadata.  wall_gfxdat and the adjacent
 * aquaduct_gfxdat form one tile-id-indexed table for ids 0xC1..0xD4. */
struct wall_gfx_rec {
    unsigned char sprite;       /* +0x00  base sprite/tile id (read by
                                          set_*_elastic via
                                          wall_gfxdat[id].sprite). */
    unsigned char count;        /* +0x01  always 1 in every wall/aqueduct
                                          entry; never read by code (a
                                          tile/variant count in the source
                                          table, but no consumer indexes it). */
    unsigned char edge_bits;    /* +0x02  edge bitmask OR'd into cm[3] by
                                          map.c (8 = wall, 16 = aqueduct). */
    unsigned char _reserved3;        /* +0x03  always 0; never read — trailing
                                          padding to a 4-byte stride. */
};
extern struct wall_gfx_rec wall_gfxdat[];
extern struct wall_gfx_rec aquaduct_gfxdat[];
extern unsigned char sailable_sea[];

/* Barbarian-trouble chance tables: [skill_level][trouble_type]. */
extern int skill_to_trouble_honeymoons[][4];
extern int skill_to_trouble_frequency[][4];
extern int skill_to_trouble_debar[][4];

/* 4-byte rotated sprite-ID record: one sprite per 90° camera rotation. */
struct rotated_sprite_rec {
    unsigned char dir[4];
};
extern struct rotated_sprite_rec rotated_map[];
extern struct rotated_sprite_rec rotated2_map[];

/* Starting pleb values for one province-difficulty level. */
struct slave_init_rec {
    int welfare_bill;
    int slaves;
};
extern int init_salary[2];
extern struct slave_init_rec init_slave_data[10];

/* request_message — 0x68-byte mixed-purpose scratch buffer at 0x117E64.
 * Originally just an `int` flag (the first slot), the area is reused as
 * a dirty-check / cached-state stash by several panel drawers.  Layout
 * reconstructed from the (offset → field) accesses in screens.c /
 * display.c / message.c.
 *
 *   +0x00  active                 message.c request-pending flag
 *   +0x04  caret_count            display.c text-entry caret blink
 *   +0x0C..+0x20  battle-stats panel last-drawn state cache
 *   +0x24..+0x30  battle-totals panel last-drawn state cache
 *   +0x34  paused                 show_paused last-drawn state
 *   +0x38  tribune_flag_counter   update_tribune_flag tick (wraps at 0x40)
 *   +0x3C  alarm_chime_counter    show_top_line chime tick (wraps at 8)
 *   +0x40  alarm_blink_timer      show_top_line toggle countdown (28h)
 *   +0x44  alarm_blink_state      show_top_line on/off toggle
 *   +0x50  cached_denarii         show_top_line dirty cache
 *   +0x58  cached_month           show_top_line dirty cache
 *   +0x5C  cached_cost            text-status dirty cache
 *   +0x60  cached_text_id         text-status dirty cache
 *   +0x64  cached_population      show_turbo_panel dirty cache
 */
struct request_message {
    int  active;                /* +0x00 */
    int  caret_count;           /* +0x04 */
    int  _unk08;                /* +0x08 */
    int  prev_mode;             /* +0x0C  battle_stats_panel */
    int  icon_over;             /* +0x10  battle_stats_panel */
    int  bs_type;               /* +0x14 */
    int  bs_morale;             /* +0x18 */
    int  bs_men;                /* +0x1C */
    int  bs_nof_units;          /* +0x20 */
    int  bt_their_morale;       /* +0x24  battle_totals_panel */
    int  bt_our_morale;         /* +0x28 */
    int  bt_their_men;          /* +0x2C */
    int  bt_our_men;            /* +0x30 */
    int  paused;                /* +0x34 */
    int  tribune_flag_counter;  /* +0x38 */
    int  alarm_chime_counter;   /* +0x3C */
    int  alarm_blink_timer;     /* +0x40 */
    int  alarm_blink_state;     /* +0x44 (read as char in ^=1) */
    int  _unk48[2];             /* +0x48..+0x4F */
    int  cached_denarii;        /* +0x50 */
    int  _unk54;                /* +0x54 */
    int  cached_month;          /* +0x58 */
    int  cached_cost;           /* +0x5C */
    int  cached_text_id;        /* +0x60 */
    int  cached_population;     /* +0x64 */
};  /* 0x68 bytes */

extern struct request_message request_message;

/* UI button descriptor arrays.  Each entry is 0x18 bytes, consumed by
 * show_buttons/control_buttons in controls.c. */
struct selection_rec {
    short para1;          /* +0x00 copied to global para1 before callback */
    short text_word;      /* +0x02 word in string group `what` */
    void (*callback)(void); /* +0x04 */
    short _unk08;         /* +0x08 */
    short max_population; /* +0x0A availability population threshold */
    short goods_kind;     /* +0x0C required/highlighted goods kind */
    short visible;        /* +0x0E computed by get_allowed_selections */
    short highlighted;    /* +0x10 computed by check_highlight_list */
    short cost_kind;      /* +0x12 city_costs/region_costs index */
}; /* 0x14 */

extern struct selection_rec ovmap_selection[];
extern struct selection_rec rm_security_selection[];
extern struct selection_rec rm_industry_selection[];
extern struct selection_rec education_selection[];
extern struct selection_rec entertainment_selection[];
extern struct selection_rec farm_selection[];
extern struct selection_rec forum_selection[];
extern struct selection_rec gardens_plaza_selection[];
extern struct selection_rec health_selection[];
extern struct selection_rec houses_selection[];
extern struct selection_rec industry_selection[];
extern struct selection_rec mine_selection[];
extern struct selection_rec quarry_selection[];
extern struct selection_rec security_selection[];
extern struct selection_rec temple_selection[];
extern struct selection_rec water_selection[];

/* Adjustable slider record consumed by slider_control / mid_slider_var /
 * up_slider_var / down_slider_var.  Stride is 0x1B bytes. */
struct slider_rec {
    short        x;             /* +0x00 */
    short        y;             /* +0x02 */
    char        *value;         /* +0x04 pointer to controlled byte */
    signed char  step_pixels;   /* +0x08 px per slider notch */
    signed char  slider_range;  /* +0x09 logical range mapped to px */
    signed char  step;          /* +0x0A logical step per click */
    char        *complement;    /* +0x0B pointer to complement byte */
    short        min_pixel;     /* +0x0F */
    short        max_pixel;     /* +0x11 */
    short        min;           /* +0x13 */
    short        max;           /* +0x15 */
    char         down_anim;     /* +0x17 */
    char         up_anim;       /* +0x18 */
    short        refresh_flag;  /* +0x19 */
}; /* 0x1B */

/* Runtime pointer globals (stored as 32-bit data symbols in PS.EXE). */
extern char *speech_filaname;
extern char *lang_file;
extern char *media_file;

/* LHARC workspace pointers owned by pump.c. */
extern unsigned char *text_buf;
extern short *lson;
extern short *rson;
extern short *dad;
extern unsigned short *freq;   /* LZHUF cumulative-frequency table (USHORT) */
extern short *prnt;
extern short *son;
extern unsigned char *pmp_inbuff;
extern unsigned char *pmp_outbuff;

/* 20-byte icon/control records used by controls.c show_icons/control_icons. */
struct icon_rec {
    short x;              /* +0x00 */
    short y;              /* +0x02 */
    short sprite;         /* +0x04 */
    void (*callback)(void); /* +0x06 */
    short down;           /* +0x0A */
    int   para1;          /* +0x0C */
    int   para2;          /* +0x10 */
}; /* 0x14 */

/* Fixed-width tutorial asset filename record (14 bytes). */
struct tutorial_file_rec {
    char name[14];
};
extern struct tutorial_file_rec tut_files[];
extern struct tutorial_file_rec tut_palfiles[];

/* Menu records used by controls.c.  The first dword of the menu array is
 * also used as the initial x position before show_menus overwrites x1/x2. */
struct menu_item_rec {
    short y;          /* +0x00 */
    short text;       /* +0x02 */
    void (*action)(void); /* +0x04 */
    short reserved;   /* +0x08 */
}; /* 0x0A */

struct menu_rec {
    union {
        short start_x;        /* +0x00 before show_menus initializes x1/x2 (signed; movsx) */
        struct {
            short x1;         /* +0x00 */
            short x2;         /* +0x02 */
        } pos;
    } u;
    short y;                  /* +0x04 */
    short text;               /* +0x06 */
    struct menu_item_rec *items; /* +0x08 */
    short item_count;         /* +0x0C */
}; /* 0x0E */
extern struct menu_rec main_menu[];

/* Per-submenu item arrays pointed at by main_menu[].items. */
extern struct menu_item_rec file_items[4];
extern struct menu_item_rec options_items[5];
extern struct menu_item_rec speed_items[3];
extern struct menu_item_rec help_items[5];

/* Help/media table entry loaded from media file (58 bytes). */
struct media_entry {
    int   text_offset;       /* +0x00 */
    short left_sprite;       /* +0x04 */
    short right_sprite;      /* +0x06 */
    short width;             /* +0x08 */
    char  left_file[16];     /* +0x0A */
    char  right_file[16];    /* +0x1A */
    char  voc_file[16];      /* +0x2A */
}; /* 0x3A */
extern struct media_entry this_media_entry;

/* Help-page clickable hotspot record (14 bytes). */
struct help_hotspot {
    short page;      /* +0x00 */
    short x1;        /* +0x02 */
    short x2;        /* +0x04 */
    short y;         /* +0x06 */
    short x3;        /* +0x08 second-line left */
    short x4;        /* +0x0A second-line right */
    short unused;    /* +0x0C */
}; /* 0x0E */
extern struct help_hotspot help_page_hot_spots[];

/* Save/load and model-load descriptor — pointer-stored-as-int followed
 * by a length.  Both savegame_entries and model_entries iterate until
 * size == 0. */
struct save_entry {
    void *buf;   /* +0x00 */
    int   size;  /* +0x04 */
};
extern struct save_entry savegame_entries[];
extern struct save_entry model_entries[];

/* Message queue slot — message_list is an array of these (anchor at the
 * slot-0 msg_id).  Each slot is { int msg; int param }.  Typed here at the
 * definition level so accessors index it directly instead of casting. */
struct msg_slot {
    int msg;    /* +0x00 */
    int param;  /* +0x04 */
};
extern struct msg_slot message_list[];

/* Graphics-table entry used by the per-zoom gfx loaders.
 * Records are 20 bytes: 16-byte filename followed by 4-byte size. */
struct gfx_entry {
    char filename[16];
    int  size;
};
extern struct gfx_entry c2_map_gfx[];
extern struct gfx_entry c2_overlay_gfx[];
extern struct gfx_entry c2_battle_gfx[];
extern struct gfx_entry c2_battle_aux_gfx[];

extern void (*city_actions[])(void);
extern void (*region_actions[])(void);

/* Per-citizen-state and per-citizen-intelligence dispatch tables.
 * Indexed by `citizen_list[i].state_idx` and `.type` respectively;
 * every entry is the entry point of an `s0X_*` / `i0X_*` handler. */
extern void (*citizen_states[13])(void);
extern void (*citizen_intelligences[8])(void);

/* Per-army-state and per-army-intelligence dispatch tables.  Indexed
 * by `army_list[i].state_idx` and `.intelligence` respectively;
 * every entry is the entry point of an `sa0X_*` / `a0X_*` handler
 * (LE fixups confirm pure function pointers). */
extern void (*army_states[17])(void);
extern void (*army_intelligences[9])(void);

/* Per-figure-state and per-figure-intelligence dispatch tables.
 * Indexed by `figure_list[i].state_idx` / `.intelligence`; entries
 * are `sf0X_*` / `f0X_*` handlers. */
extern void (*figure_states[17])(void);
extern void (*figure_intelligences[18])(void);

/* Battle-screen action callbacks and overlay-map image-getter
 * routines (indexed by `ov_map_mode`). */
extern void (*battle_actions[17])(void);
extern void (*ov_routines[10])(void);

/* VGA-chipset display names indexed by `card_is`.  Most slots point
 * at empty strings (unknown chipsets); a handful name real ones
 * ("Trident", "Tseng", ...). */
extern char *chipset_names[32];

/* Bound function-pointer slots written by the AIL DLL loader during
 * startup so its callback path can `call dword ptr [MEM_alloc]` /
 * `[MEM_free]` without going through the host symbol table. */
extern void *(*MEM_alloc)(unsigned int size);
extern void  (*MEM_free)(void *ptr);

/* Per-direction arrow-sprite offset tables (4 directions × 7 steps).
 * Watcom emits `movsx ecx, byte ptr [base + idx]` against these, so
 * the element type is signed char. */
extern signed char arrow_xr_x_ofset[28];
extern signed char arrow_xr_y_ofset[28];
extern signed char arrow_yr_x_ofset[28];
extern signed char arrow_yr_y_ofset[28];

struct button_rec {
    short x;              /* +0x00 */
    short y;              /* +0x02 */
    short sprite;         /* +0x04 */
    short size;           /* +0x06 */
    void (*callback)(void); /* +0x08 */
    unsigned char state;  /* +0x0C toggled/pressed state */
    unsigned char down;   /* +0x0D currently held */
    unsigned char repeat; /* +0x0E repeat timer */
    unsigned char type;   /* +0x0F button type */
    int   para1;          /* +0x10 */
    int   para2;          /* +0x14 */
}; /* 0x18 */

extern struct button_rec confirming_buttons[];
extern struct button_rec help_buttons[];
extern struct button_rec queery_buttons[];
extern struct button_rec query_buttons2[];
extern struct button_rec adjusting_buttons[];
extern struct button_rec skill1_buttons[];
extern struct button_rec skill2_buttons[];
extern struct button_rec exit_buttons[];
extern struct button_rec loadsave_buttons[];
extern struct button_rec tunes_buttons[];
extern struct button_rec samples_buttons[];
extern struct button_rec tog_anims_buttons[];
extern struct button_rec tog_yearend_buttons[];
extern struct button_rec promotion_buttons[];
extern struct button_rec request_buttons[];
extern struct button_rec goto_mess_buttons[];
extern struct button_rec admin_buttons[];
extern struct button_rec career_buttons[];
extern struct button_rec donation_buttons[];
extern struct button_rec clerk_buttons[];
extern struct button_rec army_buttons[];
extern struct button_rec cohort_buttons[];
extern struct button_rec mercenary_buttons[];
extern struct button_rec slave1_buttons[];
extern struct button_rec slave2_buttons[];
extern struct button_rec rome1_buttons[];
extern struct button_rec rome2_buttons[];

/* 20-byte real-mode mouse callback data block.
 *
 * Populated by the int 0x33 callback `click_handler` (see lib32.c)
 * and drained by `read_installed_mouse`.  The block must be DPMI-
 * locked because the real-mode driver writes it from an interrupt
 * context.  PS reads `bx`'s low byte directly as the button state. */
struct mouse_cbd {
    int   click_flag;       /* +0x00 set when a click bit fires */
    int   pending;          /* +0x04 non-zero if new data is waiting */
    unsigned short ax;      /* +0x08 event mask */
    unsigned short bx;      /* +0x0A button state (low byte = mse_button) */
    unsigned short cx;      /* +0x0C mouse x */
    unsigned short dx;      /* +0x0E mouse y */
    unsigned short si;      /* +0x10 mickey x */
    unsigned short di;      /* +0x12 mickey y */
};

extern struct mouse_cbd cbd;

#endif /* ENTITIES_H */
