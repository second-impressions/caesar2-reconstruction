#ifndef C2_TARGET_H
#define C2_TARGET_H

/*
 * Target and feature selection for the Caesar II reconstruction.
 *
 * PLATFORM macros name a shipped platform build of the game:
 *
 *   PLATFORM_DOS       the DOS release (PS.EXE) — the byte-exact
 *                      reconstruction target.
 *   PLATFORM_WINDOWS   the Windows port (reccmp target C2WIN) — a
 *                      source-location/oracle target, not a full rebuild.
 *                      Which Windows build is meant is C2_PATCHLEVEL's job.
 *
 * Exactly one platform is 1 and every other platform is 0.  Windows builds
 * select `PLATFORM_WINDOWS=1` explicitly.  An unspecified build defaults to
 * the original DOS platform, preserving the authentic Watcom command line
 * without inferring a platform from compiler identity.
 */
#if !defined(PLATFORM_DOS) && !defined(PLATFORM_WINDOWS)
#  define PLATFORM_DOS 1
#endif
#ifndef PLATFORM_DOS
#  define PLATFORM_DOS 0
#endif
#ifndef PLATFORM_WINDOWS
#  define PLATFORM_WINDOWS 0
#endif
#if PLATFORM_DOS + PLATFORM_WINDOWS != 1
#  error "exactly one PLATFORM_* must be selected"
#endif

/*
 * PATCHLEVEL leaves room for other witnesses of the same platform
 * (CD rereleases, patches) to enter the source later.  Higher = later
 * build of that platform.  Registry so far:
 *
 *   DOS   1  debug-symbol rerelease build (the pinned PS.EXE ground truth)
 *   WIN   1  Windows build A (the CAESAR2.EXE witness)
 *
 * A feature below may condition on (target, patchlevel) once a second
 * build of a platform is transcribed.
 */
#ifndef C2_PATCHLEVEL
#  define C2_PATCHLEVEL 1
#endif

/*
 * FEATURES name a verified behavioral difference class between the
 * builds.  Guard version-specific function code with these — never with
 * raw compiler macros (`_MSC_VER`), which conflate "which compiler" with
 * "which build of the game".  `grep C2_FEAT_ include src` is the
 * catalogue of known cross-build differences.
 */

/* Dirty-tile renderer: the DOS build marks clean screen tiles for
 * redraw (setup_whole/map/battle_screen_refresh) after UI-state
 * changes; the Windows port repaints differently and dropped these
 * calls at all but a handful of sites (each guarded site verified
 * against CAESAR2.EXE machine code, 2026-07-17). */
#define C2_FEAT_TILE_REFRESH      PLATFORM_DOS

/* The Windows build-A rotate handlers clamp the pseudo-map viewport
 * with a pm_limits() tail call that the DOS build does not make
 * (verified at C2WIN 0x4b6f46 / 0x4b6f99). */
#define C2_FEAT_ROTATE_PM_LIMITS  PLATFORM_WINDOWS

/* The Windows build resets each compression work-table pointer to
 * null after freeing it in free_pumping_memory; the DOS build leaves
 * the pointers dangling (verified at C2WIN 0x43c7e8 vs C2 0x6fffc). */
#define C2_FEAT_PUMP_FREE_NULLS   PLATFORM_WINDOWS

/* The DOS promotion offer spins a nested input loop over the
 * want-promotion box; the Windows port made the box modal (it returns
 * the choice) and re-shows any open advisor windows after a review
 * choice (verified at C2WIN 0x454e88 vs C2 0x554b1). */
#define C2_FEAT_MODAL_PROMOTION   PLATFORM_WINDOWS

/* The Windows audio port applies the configured effects volume to each
 * allocated sample handle; DOS sets the digital driver's master volume
 * (verified at C2WIN 0x40149b vs C2 0x11a53). */

/* DOS Smacker playback changes to the movie's CD path before closing and
 * restores the main path afterwards. The Windows port opens movies through
 * its native file path and omits both calls. */
#define C2_FEAT_SMACK_CD_PATH     PLATFORM_DOS

/* The later Windows map renderer rejects pseudo-map rows past the city-map
 * boundary before each scanline pass. */
#if PLATFORM_WINDOWS
#  define C2_CHECK_PM_ROW() if (pm_shown_y >= PM_H) return
#else
#  define C2_CHECK_PM_ROW()
#endif

#endif /* C2_TARGET_H */
