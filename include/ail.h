/* Miles AIL (Audio Interface Library) — third-party imports.
 *
 * The Miles source declares these with their plain C names (e.g.
 * `AIL_shutdown`) — NO leading underscore.  The `_AIL_shutdown` LINKER
 * symbol in PS.EXE is just Watcom's cdecl name-mangling (a prepended
 * `_`); it is not part of the source identifier.  So we keep the
 * faithful source name `AIL_shutdown` and use `#pragma aux NAME "_*"`
 * to re-add the mangled `_` prefix (matching the PS.EXE symbol) and to
 * select cdecl-style stack parameter passing.
 *
 * Include this header in every translation unit that calls into the
 * library so link symbols and call-site conventions stay in sync.
 */

#ifndef AIL_H
#define AIL_H

/* The Miles AIL libraries are cdecl: arguments are pushed by caller
 * (right-to-left) and EAX/EBX/ECX/EDX are caller-save.  The `modify
 * [...]` clause tells Watcom these registers may be clobbered, so any
 * calling __watcall function will save them in its prologue and skip
 * tail-call optimization for direct AIL calls. */
#pragma aux AIL_shutdown                  "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_sample_status             "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_end_sample                "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_sequence_status           "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_end_sequence              "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_stop_sequence             "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_set_sequence_volume       "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_set_digital_master_volume "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_startup                   "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_stop_sample               "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_resume_sample             "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_sample_buffer_ready       "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_load_sample_buffer        "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_set_GTL_filename_prefix   "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_install_MDI_INI           "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_allocate_sequence_handle  "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_branch_index              "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_init_sample               "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_set_sample_file           "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_set_sample_loop_count     "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_start_sample              "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_set_sample_type           "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_set_sample_playback_rate  "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_minimum_sample_buffer_size "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_install_DIG_INI            "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_init_sequence              "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_start_sequence             "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_resume_sequence            "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_register_trigger_callback  "_*" parm caller [] modify [eax ebx ecx edx]
#pragma aux AIL_allocate_sample_handle     "_*" parm caller [] modify [eax ebx ecx edx]

/* The Windows build links the Miles library as wail32.dll: every AIL
 * entry point is a DLL import, so CAESAR2.EXE call sites are indirect
 * (`call [__imp_...]`).  MSVC reproduces that with dllimport. */
#include "c2_target.h"
#if PLATFORM_WINDOWS
#define AILIMPORT __declspec(dllimport)
#else
#define AILIMPORT
#endif

AILIMPORT void AIL_shutdown(void);
AILIMPORT int  AIL_sample_status  (int handle);
AILIMPORT void AIL_end_sample     (int handle);
AILIMPORT int  AIL_sequence_status(int handle);
AILIMPORT void AIL_end_sequence   (int handle);
AILIMPORT void AIL_stop_sequence  (int handle);
AILIMPORT void AIL_set_sequence_volume(int handle, int volume, int ms);
AILIMPORT void AIL_set_digital_master_volume(int dig, int volume);
AILIMPORT int  AIL_startup(void);
AILIMPORT void AIL_stop_sample  (int handle);
AILIMPORT void AIL_resume_sample(int handle);
AILIMPORT int  AIL_sample_buffer_ready(int handle);
AILIMPORT void AIL_load_sample_buffer(int handle, int slot, void *buf, int size);
AILIMPORT void AIL_set_GTL_filename_prefix(char *prefix);
AILIMPORT int  AIL_install_MDI_INI(int *mdi_handle_out);
AILIMPORT int  AIL_allocate_sequence_handle(int mdi);
AILIMPORT void AIL_branch_index(int seq, int marker);
AILIMPORT void AIL_init_sample(int sample);
AILIMPORT int  AIL_set_sample_file(int sample, void *buf, int block);
AILIMPORT void AIL_set_sample_loop_count(int sample, int loops);
AILIMPORT char __far *AIL_start_sample(int sample);
AILIMPORT void AIL_set_sample_type(int sample, int format, int flags);
AILIMPORT void AIL_set_sample_playback_rate(int sample, int rate_hz);
AILIMPORT int  AIL_minimum_sample_buffer_size(int dig, int rate_hz, int bits);
AILIMPORT int  AIL_install_DIG_INI(int *dig_handle_out);
AILIMPORT int  AIL_init_sequence(int seq, void *bytes, int sequence_num);
AILIMPORT char __far *AIL_start_sequence(int seq);
AILIMPORT char __far *AIL_resume_sequence(int seq);
AILIMPORT void AIL_register_trigger_callback(int seq, void (*cb)());
AILIMPORT int  AIL_allocate_sample_handle(int dig);

#endif /* AIL_H */
