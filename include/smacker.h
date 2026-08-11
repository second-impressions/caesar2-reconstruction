/* RAD Smacker video library — third-party imports.
 *
 * Why the public entry points are UPPERCASE (`SMACKOPEN`, `SMACKCLOSE`,
 * …) while the internal decoder helpers in PS.EXE are CamelCase with a
 * leading underscore (`_SmackDoTables`, `_SmackDoFrameToBuffer`,
 * `_SmackTimerRead`, …):
 *
 *   The shipped RAD `smack.h` declares every API as
 *   `RADEXPFUNC ... RADEXPLINK SmackOpen(...)`.  In `rad.h`, the DOS
 *   build path (`__RADDOS__`, selected by Watcom's `__DOS__`) defines
 *   `RADEXPLINK __pascal`.  Watcom's `__pascal` convention
 *   (a) UPPERCASES the symbol with no leading/trailing underscore,
 *   (b) pushes args left-to-right, and (c) makes the callee clean the
 *   stack (`ret N`).  So the C source wrote `SmackOpen(...)` but it
 *   linked as `SMACKOPEN`.  The internal helpers were ordinary
 *   register/cdecl functions, so they kept CamelCase + leading `_`.
 *   Confirmed against symbols.json (both forms present) and the real
 *   rad.h (grayj/Jedi-Outcast code/win32/rad.h).
 *
 * We therefore model the original `rad.h` layer directly rather than
 * hand-rolling per-function `#pragma aux` clauses: `RADEXPLINK` is just
 * `__pascal`, and Watcom's `__pascal` already performs the uppercasing,
 * left-to-right push, callee-pop (`ret N`) and EAX return.  The
 * leading-underscore helpers (`_PaletteSet`, `_SetSmackAILDigDriver`)
 * are `__cdecl` (underscore prefix, caller-pop), matching their PS.EXE
 * `add esp, N` epilogues.
 *
 * Include this header in every translation unit that calls into the
 * library so link symbols and call-site conventions stay in sync.
 */

#ifndef SMACKER_H
#define SMACKER_H

/* RAD Smacker SMK handle.
 *
 * Layout reverse-engineered from offsets PS.EXE reads/writes:
 * Field names follow RAD's `Smack` struct (smack.h); offsets are what
 * PS.EXE reads/writes.  Caesar's Smacker build orders PalType before
 * the palette (unlike the public 2.0y header), and carries a second
 * palette slot at +0x374:
 *   +0x08  Height
 *   +0x0C  Frames        (total frame count)
 *   +0x68  NewPalette    (non-zero => palette changed this frame)
 *   +0x6C  PalType       (1 => use inline Palette at +0x70, else Palette2)
 *   +0x70  Palette[772]  (VGA palette data; decays to a pointer)
 *   +0x374 Palette2[772] (alternate / streaming palette slot)
 * Other fields belong to the proprietary Smacker driver; the leading
 * pads keep them addressable without naming. */
struct smk_handle {
    unsigned char _pad00[0x08];        /* +0x00 */
    int           Height;              /* +0x08 */
    unsigned int  Frames;              /* +0x0C total frame count */
    unsigned char _pad10[0x68 - 0x10]; /* +0x10..+0x67 */
    int           NewPalette;          /* +0x68 */
    int           PalType;             /* +0x6C */
    unsigned char Palette[772];        /* +0x70 */
    unsigned char Palette2[772];       /* +0x374 */
};

extern struct smk_handle *smk;

/* On the DOS/Watcom build, rad.h's `RADEXPLINK` macro expands to
 * `__pascal`; we spell it out directly here.  Watcom's `__pascal`
 * uppercases the symbol (so `SmackOpen` links as `SMACKOPEN`), pushes
 * args left-to-right and callee-pops with `ret N`, returning in EAX —
 * exactly the ABI PS.EXE imports.  (smack.h's signatures here are the
 * Smacker 2.0 ones the binary actually calls, e.g. the 6-arg
 * SmackToScreen with a u32 SetBank.) */

/* Memory callbacks supplied by the game (defined in smacker.c).  rad.h
 * declares these lowercase `radmalloc`/`radfree`; __pascal uppercases
 * them to the RADMALLOC/RADFREE symbols seen in PS.EXE. */
extern void *__pascal radmalloc(unsigned int size);
extern void  __pascal radfree (void *ptr);

/* Smacker library entry points (resolved in 3rd-party object code). */
extern struct smk_handle *__pascal SmackOpen(char *fname, unsigned flags, unsigned extrabuf);
extern void     __pascal SmackClose    (struct smk_handle *smk);
extern unsigned __pascal SmackWait     (struct smk_handle *smk);
extern unsigned __pascal SmackDoFrame  (struct smk_handle *smk);
extern void     __pascal SmackNextFrame(struct smk_handle *smk);
extern void     __pascal SmackToBuffer (struct smk_handle *smk, unsigned left, unsigned top,
                                        unsigned pitch, unsigned destheight,
                                        const void *buf, unsigned flags);
extern void     __pascal SmackToScreen (struct smk_handle *smk, unsigned left, unsigned top,
                                        unsigned byteps, const unsigned short *wintbl,
                                        void *setbank);
#if PLATFORM_WINDOWS
extern void     __pascal SmackBufferNewPalette(void *buffer, const void *palette,
                                               unsigned paltype);
extern unsigned char __cdecl SmackSoundUseMSS(int dig_driver);
#endif

/* Internal RAD helpers — __cdecl (leading underscore, caller-pop). */
extern void __cdecl PaletteSet(unsigned char *pal);
extern void __cdecl SetSmackAILDigDriver(int unused, int dig);

#endif /* SMACKER_H */
