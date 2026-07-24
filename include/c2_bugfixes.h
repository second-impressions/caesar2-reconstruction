#ifndef C2_BUGFIXES_H
#define C2_BUGFIXES_H

/*
 * Optional source-level repairs are disabled for the shipped reconstruction
 * targets.  Continuation ports may select them explicitly while retaining
 * the same recovered translation units.
 */
#ifndef C2_FIX_MOSAIC_RANDOM_SENTINEL
#define C2_FIX_MOSAIC_RANDOM_SENTINEL 0
#endif

#if C2_FIX_MOSAIC_RANDOM_SENTINEL != 0 && \
    C2_FIX_MOSAIC_RANDOM_SENTINEL != 1
#error "C2_FIX_MOSAIC_RANDOM_SENTINEL must be 0 or 1"
#endif

#endif
