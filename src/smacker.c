#include "pcsound.h"
#include <stdlib.h>      /* malloc, free */

#include "c2_data.h"
#include "c2_types.h"
#include "smacker.h"     /* RAD Smacker entry-point pragmas */

int vgawintab[2] = { 65536, 196610 };

/* File-local Smacker playback state. */
int smk_ref_hi;
int smack_frame;
int smk_height;
int smack_from_cd;
char * smack_filename;
int smksumy[15];
int smacker_on;
struct smk_handle *smk;
int smk_ref_wi;

#pragma aux high_beep modify exact [eax ebx ecx];

// Empty display callback used by Smacker's direct-screen path.
// FUNCTION: C2 0x135be
void vgawinrout(void)
{
}

// Allocates memory on behalf of the Smacker library.
// FUNCTION: C2 0x135bf
void *__pascal radmalloc(unsigned int size)
{
    if (size == 0) return (void *)size;
    return malloc(size);
}

// Releases memory on behalf of the Smacker library.
// FUNCTION: C2 0x135d2
// FUNCTION: C2WIN 0x0044b56e
void __pascal radfree(void *ptr)
{
    free(ptr);
}

// Opens a Smacker movie and displays its first frame. Modes 0 and 1 decode to the internal
// screen, with a still-image fallback for mode 1; mode 2 uses direct-screen playback.
// FUNCTION: C2 0x135de
void start_smacking(char *file_or_palette_ptr, int left, int top, int mode)
{
    int sample_flags;

    smacker_on = 0;
    if (link_to_smacker() == 0) return;

    my_strcpy("SMK", extension, 4);
    put_filename_extension(file_or_palette_ptr);
    smack_filename = file_or_palette_ptr;
    smack_from_cd  = 1;
    if (is_file_on_harddrive(file_or_palette_ptr) != 0)
        smack_from_cd = 0;
    if (smack_from_cd != 0)
        cd_path(smack_filename);
    free_scratch_buffer();

    if (allow_samples() == 0) {
        sample_flags = 0;
        high_beep();
    } else if (c2inf.samples_on == 0) {
        sample_flags = 0;
    } else {
        sample_flags = 0x200;
        if (mode == 1) sample_flags = 0x240;
    }
    if (c2inf.anims_on != 0)
        smk = SmackOpen(smack_filename, sample_flags, -1);
    else
        smk = 0;

    if (smk == 0) {
        if (smack_from_cd != 0) main_path();
        setup_scratch_buffer();
        if (mode != 1) return;

        my_strcpy("pl8", extension, 4);
        put_filename_extension(file_or_palette_ptr);
        if (readfile(file_or_palette_ptr, ((void *)scratch_buffer), 0x186a0, 0) == 0) return;

        my_strcpy("256", extension, 4);
        put_filename_extension(file_or_palette_ptr);
        if (readfile(file_or_palette_ptr, temp_palette, 0x300, 0) == 0) return;

        set_palette(temp_palette);
        general_sprite(0, left, top);
        setup_refresh_area(left, top, 0x14, 0xa, 1);
        refresh_svga_screen();
        return;
    }

    /* Select the refresh region dimensions from the movie height. */
    smk_height = smk->Height;
    if (smk_height == 0xc8) {
        smk_ref_hi = 0x0d;
        smk_ref_wi = 0x14;
    } else {
        if (smk_height == 0xc8) {
            smk_ref_hi = 0x19;
            smk_ref_wi = 0x28;
        } else {
            smk_ref_hi = 0x1e;
            smk_ref_wi = 0x28;
        }
    }
    smacker_on = 1;
    stop_samples();

    if (mode != 2) {
        SmackToBuffer(smk, left, top, 0x280, 0x1e0,
                      internal_screen, 0);
    } else {
        SmackToScreen(smk, left, top, 0x140,
                      (const unsigned short *)vgawintab, vgawinrout);
    }

    if (smk->NewPalette != 0) {
        file_or_palette_ptr = (char *)smk;
        if (smk->PalType == 1)
            file_or_palette_ptr += 0x70;
        else
            file_or_palette_ptr += 0x374;
        PaletteSet((unsigned char *)file_or_palette_ptr);
    }
    SmackDoFrame(smk);
    SmackNextFrame(smk);

    if (mode == 0) {
        setup_refresh_area(0, 0, smk_ref_wi, smk_ref_hi, 1);
        refresh_svga_screen();
    } else if (mode == 1) {
        setup_refresh_area(left, top, 0x14, 0xa, 1);
        refresh_svga_screen();
    }

    while ((short)SmackWait(smk) != 0) { }

    smack_frame = 2;
    if (smack_from_cd != 0) main_path();
}

// Advances a ready movie frame, applies palette changes, schedules any required refresh region,
// and closes the movie after its final frame.
// FUNCTION: C2 0x138bc
int continue_smacking(int left, int top, int mode)
{
    int result = 0;

#if PLATFORM_DOS
    if (link_to_smacker() == 0) return 0;
#endif
    if (smacker_on == 0) return 0;
#if PLATFORM_DOS
    if ((short)SmackWait(smk) != 0) return 0;
#else
    if (SmackWait(smk) != 0) return 0;
#endif

    if (smk->NewPalette != 0) {
#if PLATFORM_WINDOWS
        SmackBufferNewPalette(internal_screen, &smk->PalType, 0);
        set_palette((char *)&smk->PalType);
#else
        unsigned char *palette_ptr;
        if (smk->PalType == 1) palette_ptr = smk->Palette;
        else                   palette_ptr = smk->Palette2;
        PaletteSet(palette_ptr);
#endif
    }

    SmackDoFrame(smk);
    if (smack_frame < smk->Frames) {
        SmackNextFrame(smk);
#if PLATFORM_DOS
        if (mode == 0)
            setup_refresh_area(0, 0, smk_ref_wi, smk_ref_hi, 1);
        else if (mode == 1)
            setup_refresh_area(left, top, 0x14, 0xa, mode);
#endif
        result = 1;
    }
    smack_frame = smack_frame + 1;
    if (smack_frame >= smk->Frames) {
        SmackClose(smk);
        smacker_on = 0;
        result = 1;
        setup_scratch_buffer();
    }
    return result;
}

// Closes the active movie and restores the normal scratch-buffer and path state.
// FUNCTION: C2 0x139ab
void stop_smacking(void)
{
    if (smacker_on == 0) return;
#if C2_FEAT_SMACK_CD_PATH
    if (smack_from_cd) cd_path(smack_filename);
#endif
    SmackClose(smk);
    setup_scratch_buffer();
    smacker_on = 0;
#if C2_FEAT_SMACK_CD_PATH
    if (smack_from_cd) main_path();
#endif
}

// Reports whether a Smacker movie is active.
// FUNCTION: C2 0x139f7
int are_smacking(void)
{
    if (smacker_on == 0) return 0;
    return 1;
}

// Intentionally performs no work.
// FUNCTION: C2 0x13a06
void show_smksum_screen(void)
{
}
