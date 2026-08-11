
#pragma aux _ds "*"
#include "pcsound.h"
#include "c2_data.h"
#include "smacker.h"
#include <fcntl.h>             /* O_BINARY */
#if PLATFORM_DOS
char __far *MK_FP(int off, int seg);
#pragma aux MK_FP = parm [eax] [edx] value [dx eax];
#elif PLATFORM_WINDOWS
static char __far *MK_FP(unsigned off, unsigned seg) { return 0; }
#endif
extern int  open(const char *path, int flags, ...);

/* Sound module globals. */
extern int _ds;

#include "ail.h"

void AILCALLBACK mood_modfication(int sequence_handle, int log, int data);

char negative_buffer[624];
unsigned char * db_buf[2];
struct sample_slot_rec ss_entries[10];
int S_mdi[2];
int S_dig[6];
int next_sequence;
int smacker_open;
int sequences_running;
int tune2;
int dig_status;
char positive_buffer[532];
int samples_running;
unsigned char tune_buffer[27500];
int db_handle;
int db_playing;
char *db_file;
char *sample_buffer;
int mdi_status;
int next_sample;
int sslot;
int ds;
int tune1;
int dig;
int ms;
int mdi;
int db_recommended_buffer_size;
int db_buffer_size;
struct ambient_rec ambient_list[25];

/* Miles startup hooks return far-pointer status values. */
char __far *start_samples(void);
#pragma aux start_samples modify exact [eax gs];
char __far *start_sequences(void);
#pragma aux start_sequences modify exact [eax gs];
char __far *start_sound(char *sample_data, int loop_count);
char __far *start_tune(unsigned char *sequence_data, int sequence_num, int sequence_idx);
void init_ss_entires(void);

void free(void *p);

extern void _pos_ret3(void);
extern void *malloc(unsigned int size);
/* Forward declarations (functions defined later in this file). */
void stop_samples(void);
void stop_sequences(void);
void get_city_mood(void);
void get_battle_mood(void);
void choose_odd_tune(int branch_base);
void get_new_sslot(char *filename);
void free_up_sslot(int slot_idx);
#if PLATFORM_WINDOWS
void stop_tune_thread(void);
#endif


// Initialize AIL and enable digital samples and MIDI sequences.
// FUNCTION: C2 0x11758
// FUNCTION: C2WIN 0x00401000
void start_sounds(void)
{
    db_playing      = 0;
    smacker_open    = 0;
    c2inf.samples_on = 1;
    c2inf.tunes_on   = 1;
    next_sample = next_sequence = 0;
    samples_running  = 0;
    sequences_running = 0;
    AIL_startup();
    start_samples();
    start_sequences();
}

// Stop the sample and sequence drivers, then shut down AIL.
// FUNCTION: C2 0x117a2
void stop_sounds(void)
{
#if PLATFORM_WINDOWS
    stop_tune_thread();
#endif
    stop_samples();
    stop_sequences();
    AIL_shutdown();
}
// Install the digital driver, preload feedback sounds, and allocate sample handles.
// FUNCTION: C2 0x117b8
// FUNCTION: C2WIN 0x00401085
char __far *start_samples(void)
{
    char __far *result;

    if (!samples_running)
    if (c2inf.samples_on) {
        if (readfile("poscl.wav",  positive_buffer, 0x214, 0) == 0) return MK_FP(4, 0);
        if (readfile("negcl2.wav", negative_buffer, 0x270, 0) == 0) return MK_FP(4, 0);
        if (AIL_install_DIG_INI(&dig)) return MK_FP(1, 0);
        for (ds = 0; ds < 6; ds++) S_dig[ds] = AIL_allocate_sample_handle(dig);
        ds = 0; do { if (S_dig[ds] == 0) return MK_FP(2, 0); ds++; } while (ds < 6);
        samples_running = 1;
        init_ss_entires();
    }
    return result;
}

// Install the MIDI driver, allocate sequence handles, and reset music mood state.
// FUNCTION: C2 0x118a2
// FUNCTION: C2WIN 0x00401250
char __far *start_sequences(void)
{
    char __far *result;

    if (!sequences_running && c2inf.tunes_on) {
        AIL_set_GTL_filename_prefix("CAESAR");
        if (AIL_install_MDI_INI(&mdi) != 0) return (char __far *)1;
        for (ms = 0; ms < 2; ms++) {
            S_mdi[ms] = AIL_allocate_sequence_handle(mdi);
        }
        for (ms = 0; ms < 2; ms++) {
            if (S_mdi[ms] == 0) return (char __far *)MK_FP(2, 1);
        }
        tune1 = 0;
        tune2 = 1;
        tune_mood = 0;
        tune_branch_count = 0;
        last_battle_mood = 0;
        last_city_mood = 0;
        sequences_running = 1;
    }
    return result;
}

// Stop all active digital samples and clear streaming playback state.
// FUNCTION: C2 0x1197b
// FUNCTION: C2WIN 0x0040138f
void stop_samples(void)
{
    if (samples_running == 0) return;
    db_playing = 0;
    for (ds = 0; ds < 6; ds++) {
        dig_status = AIL_sample_status(S_dig[ds]);
        if (dig_status == 4) AIL_end_sample(S_dig[ds]);
    }
}

// Stop all active MIDI sequences and clear city music playback state.
// FUNCTION: C2 0x119e7
void stop_sequences(void)
{
    city_tune_playing = 0;
    if (sequences_running == 0) return;
    for (ms = 0; ms < 2; ms++) {
        mdi_status = AIL_sequence_status(S_mdi[ms]);
        if (mdi_status == 4) AIL_end_sequence(S_mdi[ms]);
    }
}

// Apply the configured digital-sample volume to the AIL driver.
// FUNCTION: C2 0x11a53
// FUNCTION: C2WIN 0x0040149b
void set_samples_volume(void)
{
    int volume;
#if C2_FEAT_PER_SAMPLE_VOLUME
    int ds;
#endif

    if (c2inf.samples_on == 0)   return;
    if (samples_running == 0)    return;
    volume = totalXpercent(0x7f, c2inf.samples_level);
#if C2_FEAT_PER_SAMPLE_VOLUME
    for (ds = 0; ds < 6; ds++) {
        AIL_set_sample_volume(S_dig[ds], volume);
    }
#else
    AIL_set_digital_master_volume(dig, volume);
#endif
}

// Apply the configured music volume to both MIDI sequence handles.
// FUNCTION: C2 0x11a8c
// FUNCTION: C2WIN 0x00401515
void set_sequences_volume(void)
{
    int volume;

    if (c2inf.tunes_on == 0)     return;
    if (sequences_running == 0)  return;
    volume = totalXpercent(0x7f, c2inf.tunes_level);
    AIL_set_sequence_volume(S_mdi[tune1], volume, 0);
    AIL_set_sequence_volume(S_mdi[tune2], volume, 0);
}

// Fade a MIDI sequence up to the configured music volume over one second.
// FUNCTION: C2 0x11ae9
// FUNCTION: C2WIN 0x0040158e
void fade_sequence_in(int sequence_idx)
{
    int volume;

    if (c2inf.tunes_on == 0)     return;
    if (sequences_running == 0)  return;
    volume = totalXpercent(0x7f, c2inf.tunes_level);
    AIL_set_sequence_volume(S_mdi[sequence_idx],   0, 0);
    AIL_set_sequence_volume(S_mdi[sequence_idx], volume, 1000);
}

// Fade a MIDI sequence to silence over one second.
// FUNCTION: C2 0x11b4b
// FUNCTION: C2WIN 0x00401604
void fade_sequences_out(int sequence_idx)
{
    if (c2inf.tunes_on == 0)      return;
    if (sequences_running == 0)   return;
    AIL_set_sequence_volume(S_mdi[sequence_idx], 0, 1000);
}

// Load or reuse a cached sample and start its playback when a voice is available.
// FUNCTION: C2 0x11b7b
// FUNCTION: C2WIN 0x0040164d
void set_sound(unsigned char *filename, int loop_count)
{
    if (c2inf.samples_on == 0) return;
    if (samples_running == 0) return;
    if (*filename == 0) return;
    if (check_for_free_slot() == 0) return;
    if (check_old_sslots(filename) == 0) {
        get_new_sslot(filename);
        if (readfile(filename, sample_buffer + sslot * 0x4e20,
                     0x4e20, 0) == 0) {
            free_up_sslot(sslot);
            return;
        }
    }
    start_sound(sample_buffer + sslot * 0x4e20, loop_count);
}

// Load or reuse a cached sample and request playback without checking for a free voice first.
// FUNCTION: C2 0x11c35
// FUNCTION: C2WIN 0x00401734
void set_pri_sound(unsigned char *filename, int loop_count)
{
    if (c2inf.samples_on == 0) return;
    if (samples_running == 0) return;
    if (*filename == 0) return;
    if (check_old_sslots(filename) == 0) {
        get_new_sslot(filename);
        if (readfile(filename, sample_buffer + sslot * 0x4e20,
                     0x4e20, 0) == 0) {
            free_up_sslot(sslot);
            return;
        }
    }
    start_sound(sample_buffer + sslot * 0x4e20, loop_count);
}

// Select a digital voice, configure its sample data and loop count, and start playback.
// FUNCTION: C2 0x11ce2
// FUNCTION: C2WIN 0x00401809
char __far *start_sound(char *sample_data, int loop_count)
{
    for (ds = 0; ds < c2inf.max_samples; ds++) {
        dig_status = AIL_sample_status(S_dig[ds]);
        if (dig_status == 2) { next_sample = ds; break; }
        if (dig_status == 8) { next_sample = ds; break; }
    }

    ds = next_sample;
    next_sample++;
    if (next_sample >= c2inf.max_samples) next_sample = 0;

    dig_status = AIL_sample_status(S_dig[ds]);
    if (dig_status == 4) AIL_end_sample(S_dig[ds]);
    AIL_init_sample(S_dig[ds]);
    if (AIL_set_sample_file(S_dig[ds], sample_data, -1) == 0) return (char __far *)3;
    AIL_set_sample_loop_count(S_dig[ds], loop_count);
    return AIL_start_sample(S_dig[ds]);
}

// Returns nonzero if a sample slot is available.
// FUNCTION: C2 0x11df2
// FUNCTION: C2WIN 0x0040198e
int check_for_free_slot(void)
{
    for (ds = 0; ds < c2inf.max_samples; ds++) {
        dig_status = AIL_sample_status(S_dig[ds]);
        if (dig_status != 4) return 1;
    }
    return 0;
}

// Stop every active gameplay sound voice.
// FUNCTION: C2 0x11e3b
void stop_all_sounds(void)
{
    for (ds = 0; ds < 4; ds++) {
        dig_status = AIL_sample_status(S_dig[ds]);
        if (dig_status == 4) AIL_end_sample(S_dig[ds]);
    }
}

#pragma aux _pos_ret3 = "xor edx,edx" "mov eax,3" modify exact [eax edx]

// Play the preloaded positive-feedback sound on the dedicated feedback voice.
// FUNCTION: C2 0x11e92
// FUNCTION: C2WIN 0x00401a69
void pos_sound(void)
{
    if (c2inf.samples_on != 0 && samples_running != 0) {
        ds = 4;
        dig_status = AIL_sample_status(S_dig[4]);
        if (dig_status == 4) AIL_end_sample(S_dig[ds]);
        AIL_init_sample(S_dig[ds]);
        if (AIL_set_sample_file(S_dig[ds], positive_buffer, -1) == 0) _pos_ret3();
        AIL_start_sample(S_dig[ds]);
    }
}

// Play the negative feedback sample when digital audio is available.
// FUNCTION: C2 0x11f40
// FUNCTION: C2WIN 0x00401b30
void neg_sound(void)
{
    if (c2inf.samples_on != 0 && samples_running != 0) {
        ds = 4;
        dig_status = AIL_sample_status(S_dig[4]);
        if (dig_status == 4) AIL_end_sample(S_dig[ds]);
        AIL_init_sample(S_dig[ds]);
        if (AIL_set_sample_file(S_dig[ds], negative_buffer, -1) == 0) _pos_ret3();
        AIL_start_sample(S_dig[ds]);
    }
}

// Refill the next available half of an AIL double-buffered sample stream.
// FUNCTION: C2 0x11fb9
void serve_sample(int sample_handle, unsigned char **buffers, int buffer_size)
{
    int buffer_idx;
    int count;

    buffer_idx = AIL_sample_buffer_ready(sample_handle);
    if (buffer_idx != -1) {
        count = read(db_handle, buffers[buffer_idx], buffer_size);
        AIL_load_sample_buffer(sample_handle, buffer_idx, buffers[buffer_idx], count);
    }
}

// Open a speech sample and prepare the dedicated double-buffered streaming voice.
// FUNCTION: C2 0x12003
// FUNCTION: C2WIN 0x00401c57
void set_db_sound(char *filename)
{
    if (c2inf.samples_on == 0)   return;
    if (c2inf.speech_on == 0)    return;
    if (samples_running == 0)    return;
    if (db_playing != 0)         return;
    if (*filename == 0)             return;
    if (check_file_exists(filename) == 0) return;

    db_file = filename;
    cd_path(filename);
    db_handle = open(db_file, O_RDONLY | O_BINARY);
    db_playing = 1;

    db_buf[0] = scratch_buffer + 140000;   /* 0x222e0 */
    db_buf[1] = scratch_buffer + 150000;   /* 0x249f0 */

    db_recommended_buffer_size = AIL_minimum_sample_buffer_size(dig, 22050, 0);
    db_buffer_size = 10000;
    ds = 5;

    AIL_init_sample(S_dig[ds]);
    AIL_set_sample_type(S_dig[ds], 0, 0);
    AIL_set_sample_playback_rate(S_dig[ds], 22050);
    main_path();
}

// Refill the active speech stream and close it after playback finishes.
// FUNCTION: C2 0x1211e
// FUNCTION: C2WIN 0x00401da8
void continue_db(void)
{
    if (c2inf.samples_on == 0) return;
    if (samples_running == 0) return;
    if (db_playing == 0) return;
    cd_path(db_file);
    ds = 5;
    serve_sample(S_dig[5], db_buf, db_buffer_size);
    if (AIL_sample_status(S_dig[ds]) == 2) {
        db_playing = 0;
        close(db_handle);
    }
    main_path();
}

// Stop the active speech stream and close its file.
// FUNCTION: C2 0x121a9
// FUNCTION: C2WIN 0x00401e49
void stop_db(void)
{
    if (c2inf.samples_on == 0) return;
    if (samples_running == 0) return;
    if (db_playing == 0) return;
    cd_path(db_file);
    ds = 5;
    AIL_end_sample(S_dig[5]);
    db_playing = 0;
    close(db_handle);
    main_path();
}

// Toggle pause state for the active speech stream.
// FUNCTION: C2 0x121fa
// FUNCTION: C2WIN 0x00401ec1
int pause_db(void)
{
    if (c2inf.samples_on == 0) return 0;
    if (samples_running == 0)  return 0;
    if (db_playing == 0)       return 0;
    ds = 5;
    dig_status = AIL_sample_status(S_dig[ds]);
    if (dig_status == 4)       AIL_stop_sample  (S_dig[ds]);
    else if (dig_status == 8)  AIL_resume_sample(S_dig[ds]);
    return 1;
}

// Load and start an XMI tune unless music or the sequence engine is disabled.
// FUNCTION: C2 0x12279
void play_tune(unsigned char *filename, int loop_count)
{
#if PLATFORM_WINDOWS
    return;
#endif
    if (c2inf.tunes_on == 0) return;
    if (sequences_running == 0) return;
    if (*filename == 0) return;
    if (readfile(filename, tune_buffer, 0x6b6c, 0) == 0) return;
    start_tune(tune_buffer, 0, loop_count);
}

// Initialize, fade in, and start a MIDI sequence on the selected slot.
// FUNCTION: C2 0x122bc
char __far *start_tune(unsigned char *sequence_data, int sequence_num, int sequence_idx)
{
    int result;

    if (sequence_idx == 0) AIL_end_sequence(S_mdi[1]);
    else if (sequence_idx == 1) AIL_stop_sequence(S_mdi[0]);

    mdi_status = AIL_sequence_status(S_mdi[sequence_idx]);
    if (mdi_status == 4) AIL_end_sequence(S_mdi[sequence_idx]);
    else if (mdi_status == 8) {
        return AIL_resume_sequence(S_mdi[sequence_idx]);
    }

    result = AIL_init_sequence(S_mdi[sequence_idx], sequence_data, sequence_num);
    AIL_register_trigger_callback(S_mdi[sequence_idx], mood_modfication);
    if (result < 0) return (char __far *)MK_FP(3, 1);
    if (result == 0) return (char __far *)MK_FP(4, 1);
    fade_sequence_in(sequence_idx);
    return AIL_start_sequence(S_mdi[sequence_idx]);
}

// Stop the primary tune and end the secondary tune when they are playing.
// FUNCTION: C2 0x1239c
void stop_tune(void)
{
    mdi_status = AIL_sequence_status(S_mdi[0]);
    if (mdi_status == 4) AIL_stop_sequence(S_mdi[0]);
    mdi_status = AIL_sequence_status(S_mdi[1]);
    if (mdi_status == 4) AIL_end_sequence(S_mdi[1]);
}

// Stop the primary MIDI sequence when it is playing.
// FUNCTION: C2 0x123f5
void stop_tune0(void)
{
    mdi_status = AIL_sequence_status(S_mdi[0]);
    if (mdi_status == 4) AIL_stop_sequence(S_mdi[0]);
}

// Recalculate the current music mood and branch the active sequence.
// FUNCTION: C2 0x12424
void AILCALLBACK mood_modfication(int sequence_handle, int log, int data)
{
    tune_branch_count++;
    tune_mood_hold = 0;
    if (map_mode == 2) get_battle_mood();
    else               get_city_mood();
    AIL_branch_index(sequence_handle, tune_branch);
}

// Restore the last music mood for the current map mode.
// FUNCTION: C2 0x12461
// FUNCTION: C2WIN 0x004021fd
void get_old_mood(void)
{
    if (map_mode == 2) tune_mood = last_battle_mood;
    else               tune_mood = last_city_mood;
}

// Choose the music branch from the current mood, including temporary threat and emergency states.
// FUNCTION: C2 0x1247f
// FUNCTION: C2WIN 0x00402231
void get_city_mood(void)
{
    if (tune_mood == 10) { tune_mood = 0; tune_branch = 0x28; }
    else if (tune_mood == 11) { tune_mood = 0; tune_branch = 0x29; }
    else if (tune_mood == 12) { tune_mood = 0; tune_branch = 0x2a; }
    else if (tune_mood == 13) { tune_mood = 1; tune_branch = 0x2d; bad_mood = 0xc8; }
    else if (tune_mood == 14) { tune_mood = 1; tune_branch = 0x2b; bad_mood = 0xc8; }
    else if (tune_mood == 15) { tune_mood = 1; tune_branch = 0x2c; bad_mood = 0xc8; }
    else if (tune_mood == 16) { tune_mood = 1; tune_branch = 0x2e; bad_mood = 0xc8; }
    else if (tune_mood == 17) { tune_mood = 2; tune_branch = 0x2f; threat_mood = 0xc8; }
    else if (tune_mood == 18) { tune_mood = 2; tune_branch = 0x30; threat_mood = 0xc8; }
    else if (tune_mood == 19) { tune_mood = 2; tune_branch = 0x31; threat_mood = 0xc8; }
    else if (tune_mood == 20) { tune_mood = 3; tune_branch = 0x32; emergency_mood = 0xc8; }
    else if (tune_mood == 21) { tune_mood = 3; tune_branch = 0x33; emergency_mood = 0xc8; }
    else if (tune_mood == 22) { tune_mood = 3; tune_branch = 0x34; emergency_mood = 0xc8; }
    else if (tune_mood == 23) { tune_mood = 3; tune_branch = 0x35; emergency_mood = 0xc8; }
    else if (tune_mood == 0) { tune_branch = rand128 & 7; if (tune_branch > 6) tune_branch = 6; }
    else if (tune_mood == 1) { tune_branch = (rand128 & 7) + 0xa; if (tune_branch > 0x10) tune_branch = 0x10; }
    else if (tune_mood == 2) { tune_branch = (rand128 & 7) + 0x14; if (tune_branch > 0x1a) tune_branch = 0x1a; }
    else if (tune_mood == 3) { tune_branch = (rand128 & 7) + 0x1e; if (tune_branch > 0x24) tune_branch = 0x24; }
    else { tune_mood = 0; tune_branch = 0; }
    if (bad_mood != 0)       tune_mood = 1;
    if (threat_mood != 0)    tune_mood = 2;
    if (emergency_mood != 0) tune_mood = 3;

    if (((bad_mood == 0) && (threat_mood == 0)) && (emergency_mood == 0)) {
        if (last_city_mood == 3)      tune_mood = 2;
        else if (last_city_mood == 2) tune_mood = 1;
        else                          tune_mood = 0;
    }
    last_city_mood = tune_mood;
}

// Select the battle-music branch for the current mood.
// FUNCTION: C2 0x12751
// FUNCTION: C2WIN 0x0040266b
void get_battle_mood(void)
{
    if      (tune_mood == 1)  choose_odd_tune(0xe);
    else if (tune_mood == 2)  choose_odd_tune(0x16);
    else if (tune_mood == 3)  choose_odd_tune(0x1e);
    else if (tune_mood == 4)  choose_odd_tune(0x26);
    else if (tune_mood == 5)  choose_odd_tune(0x2e);
    else if (tune_mood == 6)  tune_branch = 0;
    else if (tune_mood == 7)  tune_branch = 1;
    else if (tune_mood == 8)  tune_branch = 2;
    else if (tune_mood == 9)  tune_branch = 3;
    else if (tune_mood == 10) tune_branch = 4;
    else if (tune_mood == 11) tune_branch = 5;
    else if (tune_mood == 12) tune_branch = 6;
    else if (tune_mood == 13) tune_branch = 7;
    else if (tune_mood == 14) tune_branch = 9;
    else if (tune_mood == 15) tune_branch = 10;
    else if (tune_mood == 16) tune_branch = 11;
    else if (tune_mood == 17) tune_branch = 12;
    else if (tune_mood == 18) tune_branch = 13;
    else                      tune_branch = (rand128 + 0xe) & 7;
    last_battle_mood = tune_mood;
}

// Alternate battle-music branches within a mood group.
// FUNCTION: C2 0x128ad
// FUNCTION: C2WIN 0x00402888
void choose_odd_tune(int branch_base)
{
    if (odd_battle_tune) {
        tune_branch = branch_base + (rand128 & 6);
        odd_battle_tune = 0;
    } else {
        tune_branch += 1;
        odd_battle_tune = 1;
    }
}

// Decay the temporary emergency, threat, and bad-mood counters.
// FUNCTION: C2 0x128ea
// FUNCTION: C2WIN 0x004028cf
void sooth_mood(void)
{
    if (emergency_mood) emergency_mood -= 1;
    if (threat_mood)    threat_mood    -= 1;
    if (bad_mood)       bad_mood       -= 1;
}

// Initialize the cached-sample slots and their replacement order.
// FUNCTION: C2 0x12932
// FUNCTION: C2WIN 0x00402913
void init_ss_entires(void)
{
    int i;
    for (i = 0; i < 10; i++) ss_entries[i].hits = i;
    sslot = 0;
}

// Choose a cached-sample slot for a new file and update its replacement state.
// FUNCTION: C2 0x12953
// FUNCTION: C2WIN 0x00402959
void get_new_sslot(char *filename)
{
    int highest_hit_count;
    int slot;
    int count;
    slot = highest_hit_count = 0;
    for (count = 0; count < 10; count++) {
        ++ss_entries[count].hits;
        if (highest_hit_count <= ss_entries[count].hits) {
            slot = count; highest_hit_count = ss_entries[count].hits;
        }
    }
    sslot = slot;
    ss_entries[sslot].hits = 0;
    strcpy(ss_entries[sslot].name, filename);
}

// Mark a cached-sample slot unused so it will be replaced next.
// FUNCTION: C2 0x129b1
// FUNCTION: C2WIN 0x00402a06
void free_up_sslot(int slot_idx)
{
    ss_entries[slot_idx].hits = 1000;
    strcpy(ss_entries[slot_idx].name, "unused.wav");
}

// Find a cached sample by filename and mark its slot recently used.
// FUNCTION: C2 0x129db
// FUNCTION: C2WIN 0x00402a41
int check_old_sslots(char *filename)
{
    int i;
    for (i = 0; i < 10; i++) {
        if (strcmp(ss_entries[i].name, filename) == 0) {
            ss_entries[i].hits = 0;
            sslot = i;
            return 1;
        }
    }
    return 0;
}

// Connect Smacker audio playback to the active AIL digital driver.
// FUNCTION: C2 0x12a25
// FUNCTION: C2WIN 0x00402abd
int link_to_smacker(void)
{
#if !PLATFORM_WINDOWS
    int driver_flag;
#endif
    if (smacker_open) return 1;
#if PLATFORM_WINDOWS
    SmackSoundUseMSS(dig);
#else
    driver_flag = smacker_open;
    SetSmackAILDigDriver(dig, driver_flag);
#endif
    smacker_open = 1;
    return 1;
}

// True when the AIL digital subsystem is running.
// FUNCTION: C2 0x12a5c
// FUNCTION: C2WIN 0x00402b01
int allow_samples(void)
{
    if (samples_running) {
        return 1;
    }
    return 0;
}

// Flag ambient slot `idx` for playback on the next play_ambient_fx tick.
// FUNCTION: C2 0x12a6c
// FUNCTION: C2WIN 0x00402b2a
void set_this_ambient(int ambient_idx)
{
    ambient_list[ambient_idx].active = 1;
}

// Raise an ambient slot's delay counter to the requested minimum.
// FUNCTION: C2 0x12a77
// FUNCTION: C2WIN 0x00402b4a
void set_ambient_minimum(int ambient_idx, int minimum_delay)
{
    if (ambient_list[ambient_idx].delay_counter < minimum_delay)
        ambient_list[ambient_idx].delay_counter = minimum_delay;
}

// Activate the ambient sound associated with a city building type.
// FUNCTION: C2 0x12a8f
// FUNCTION: C2WIN 0x00402b8b
void set_city_ambient(int building_kind)
{
    int ambient_idx = 0;
    unsigned char building_type;

    if (building_kind < 0x78) return;
    else if (building_kind < 0x7c) ambient_idx = 1;
    else if (building_kind < 0x82) ambient_idx = 0xe;
    else if (building_kind < 0xa2) return;
    else if (building_kind < 0xae) ambient_idx = 0x12;
    else if (building_kind < 0xbc) ambient_idx = 7;
    else if (building_kind < 0xbe) ambient_idx = 0x11;
    else if (building_kind < 0xbf) ambient_idx = 0x10;
    else if (building_kind < 0xcb) return;
    else if (building_kind < 0xd7) ambient_idx = 0x11;
    else if (building_kind < 0xdb) ambient_idx = 0xf;
    else if (building_kind < 0xdf) ambient_idx = 8;
    else if (building_kind < 0xe3) ambient_idx = 4;
    else if (building_kind < 0xe4) ambient_idx = 0x17;
    else if (building_kind < 0xe5) ambient_idx = 3;
    else if (building_kind < 0xe7) ambient_idx = 0x13;
    else if (building_kind < 0xe9) ambient_idx = 5;
    else if (building_kind < 0xf3) ambient_idx = 2;
    else if (building_kind < 0xf4) ambient_idx = 0xa;
    else if (building_kind < 0xf5) ambient_idx = 0xb;
    else if (building_kind < 0xfa) return;
    else if (building_kind < 0xfb) {
        building_type = (*(struct city_cell *)((unsigned char *)city_map + (pm_shown_ptr))).building & 0xf0;
        building_type >>= 4;
        if (building_type > 3) ambient_idx = 0x14;
        else                   ambient_idx = 0x15;
    } else if (building_kind < 0xfc) ambient_idx = 0x16;
    else if (building_kind < 0xfe) ambient_idx = 0xd;
    else ambient_idx = 0xc;

    ambient_list[ambient_idx].active = 1;
}

// Activate the ambient sound associated with a province event.
// FUNCTION: C2 0x12c22
// FUNCTION: C2WIN 0x00402e37
void set_prov_ambient(int event)
{
    if (event <  0xd2) return;
    if (event <  0xd3) return;
    if      (event <  0xd4) event = 0xc;
    else if (event <  0xd5) event = 5;
    else if (event <  0xdc) event = 4;
    else if (event <  0xe0) event = 0xb;
    else if (event <  0xe4) event = 2;
    else if (event <  0xe8) event = 6;
    else if (event <  0xec) event = 7;
    else                    event = 3;

    ambient_list[event].active = 1;
}

// Schedule the death sound associated with a battle unit type.
// FUNCTION: C2 0x12cad
// FUNCTION: C2WIN 0x00402f45
void set_battle_death_fx(int unit_type)
{
    int ambient_idx = 0;

    if      (unit_type <=  3) ambient_idx = 0xc;
    else if (unit_type <= 14) ambient_idx = 0xd;
    else if (unit_type <= 15) ambient_idx = 0xe;
    else if (unit_type <= 17) ambient_idx = 0xd;

    ambient_list[ambient_idx].active = 1;
    ambient_list[ambient_idx].delay_counter = 0xc8;
}

// Schedule a unit-march sound for the given unit-type id.
// FUNCTION: C2 0x12cec
// FUNCTION: C2WIN 0x00402fd9
void set_battle_march_fx(int unit_type)
{
    int ambient_idx = 0;

    if      (unit_type <=  3) ambient_idx = 15;
    else if (unit_type <= 10) ambient_idx = 15;
    else if (unit_type <= 15) ambient_idx = 16;
    else if (unit_type <= 17) ambient_idx = 15;

    ambient_list[ambient_idx].active = 1;
    if (marching_fx == 0) set_ambient_minimum(ambient_idx, 0xc7);
    else                  set_ambient_minimum(ambient_idx, 0xb9);
    marching_fx = 10;
}

// Schedule the melee sound associated with a battle event.
// FUNCTION: C2 0x12d41
// FUNCTION: C2WIN 0x00403094
void set_battle_fight_fx(int event)
{
    int ambient_idx = 0;

    if      (event <= 2)    ambient_idx = 8;
    else if (event <= 3)    ambient_idx = 9;
    else if (event <= 5)    ambient_idx = 8;
    else if (event <= 6)    ambient_idx = 0xa;
    else if (event <= 7)    ambient_idx = 7;
    else if (event <= 8)    ambient_idx = 0xa;
    else if (event <= 0xa)  ambient_idx = 9;
    else if (event <= 0xd)  ambient_idx = 8;
    else if (event <= 0x11) ambient_idx = 9;

    ambient_list[ambient_idx].active = 1;
    ambient_list[ambient_idx].delay_counter += 0x19;
}

// Schedule the missile-impact sound associated with a battle event.
// FUNCTION: C2 0x12d9f
// FUNCTION: C2WIN 0x004031ac
void set_missile_fight_fx(int event)
{
    int ambient_idx = 0;

    if      (event <= 3)    ambient_idx = 6;
    else if (event <= 9)    ambient_idx = 0xb;
    else if (event <= 0xa)  ambient_idx = 6;
    else if (event <= 0x10) ambient_idx = 3;
    else if (event <= 0x11) ambient_idx = 0xb;

    ambient_list[ambient_idx].active = 1;
    ambient_list[ambient_idx].delay_counter += 0x19;
}

// Schedule the launch sound associated with a missile type.
// FUNCTION: C2 0x12de2
// FUNCTION: C2WIN 0x0040326c
void set_missile_fire_fx(int missile_type)
{
    int ambient_idx = 0;

    if      (missile_type <=  3) ambient_idx = 4;
    else if (missile_type <=  9) ambient_idx = 0;
    else if (missile_type <= 10) ambient_idx = 4;
    else if (missile_type <= 16) ambient_idx = 1;
    else if (missile_type <= 17) ambient_idx = 0;

    ambient_list[ambient_idx].active = 1;
    ambient_list[ambient_idx].delay_counter += 0x28;
}

// Play eligible ambient sounds and rotate through each slot's sample names.
// FUNCTION: C2 0x12e1e
// FUNCTION: C2WIN 0x0040332c
void play_ambient_fx(void)
{
    int i;

    if (marching_fx != 0) marching_fx--;
    for (i = 1; i < 0x19; i++) {
        if (ambient_list[i].active == 0) continue;
        ambient_list[i].active = 0;
        (ambient_list[i].delay_counter)++;
        if (ambient_list[i].delay_counter >= 0xc8) {
            ambient_list[i].delay_counter = rand128;
            if (c2inf.ambients_on == 0) break;
            if (ambient_list[i].name_idx == 0) {
                set_sound(ambient_list[i].names[0], ambient_list[i].volume);
            } else if (ambient_list[i].name_idx == 1) {
                set_sound(ambient_list[i].names[1], ambient_list[i].volume);
            } else if (ambient_list[i].name_idx == 2) {
                set_sound(ambient_list[i].names[2], ambient_list[i].volume);
            } else if (ambient_list[i].name_idx == 3) {
                set_sound(ambient_list[i].names[3], ambient_list[i].volume);
            }
            ambient_list[i].name_idx++;
            if (ambient_list[i].name_idx >= ambient_list[i].name_count) {
                ambient_list[i].name_idx = 0;
            }
        }
    }
}

// Initialize the city ambient-sound bank.
// FUNCTION: C2 0x12f2a
// FUNCTION: C2WIN 0x004035c9
void init_city_ambients(void)
{
    int i;

    for (i = 0; i < 0x19; i++) {
        ambient_list[i].active = 0;
        ambient_list[i].delay_counter = (i * 8);
        ambient_list[i].name_idx = 0;
        ambient_list[i].volume = 1;
    }
    ambient_list[1].name_count = 3;
    strcpy(ambient_list[1].names[0], "gardenb.wav");
    strcpy(ambient_list[1].names[1], "gardenc.wav");
    strcpy(ambient_list[1].names[2], "gardend.wav");
    ambient_list[2].name_count = 1;
    strcpy(ambient_list[2].names[0], "circus1.wav");
    ambient_list[3].name_count = 1;
    strcpy(ambient_list[3].names[0], "barrack2.wav");
    ambient_list[4].name_count = 1;
    ambient_list[4].volume = 1;
    strcpy(ambient_list[4].names[0], "bathhs.wav");
    ambient_list[5].name_count = 2;
    strcpy(ambient_list[5].names[0], "colisum5.wav");
    strcpy(ambient_list[5].names[1], "colisum6.wav");
    ambient_list[6].name_count = 1;
    strcpy(ambient_list[6].names[0], "fire.wav");
    ambient_list[7].name_count = 1;
    strcpy(ambient_list[7].names[0], "forum.wav");
    ambient_list[8].name_count = 1;
    ambient_list[8].volume = 1;
    strcpy(ambient_list[8].names[0], "fountn.wav");
    ambient_list[9].name_count = 1;
    strcpy(ambient_list[9].names[0], "fountnx.wav");
    ambient_list[10].name_count = 1;
    strcpy(ambient_list[10].names[0], "grammat2.wav");
    ambient_list[11].name_count = 1;
    strcpy(ambient_list[11].names[0], "rhetor.wav");
    ambient_list[12].name_count = 1;
    ambient_list[12].volume = 1;
    strcpy(ambient_list[12].names[0], "marketh.wav");
    ambient_list[13].name_count = 1;
    ambient_list[13].volume = 1;
    strcpy(ambient_list[13].names[0], "marketl.wav");
    ambient_list[14].name_count = 1;
    strcpy(ambient_list[14].names[0], "plazab.wav");
    ambient_list[15].name_count = 1;
    ambient_list[15].volume = 1;
    strcpy(ambient_list[15].names[0], "well.wav");
    ambient_list[16].name_count = 1;
    ambient_list[16].volume = 1;
    strcpy(ambient_list[16].names[0], "reserv.wav");
    ambient_list[17].name_count = 1;
    ambient_list[17].volume = 1;
    strcpy(ambient_list[17].names[0], "aquadct.wav");
    ambient_list[18].name_count = 1;
    strcpy(ambient_list[18].names[0], "temple1.wav");
    ambient_list[19].name_count = 1;
    strcpy(ambient_list[19].names[0], "theatre.wav");
    ambient_list[20].name_count = 1;
    ambient_list[20].volume = 1;
    strcpy(ambient_list[20].names[0], "hbiz.wav");
    ambient_list[21].name_count = 1;
    ambient_list[21].volume = 1;
    strcpy(ambient_list[21].names[0], "lbiz.wav");
    ambient_list[22].name_count = 1;
    strcpy(ambient_list[22].names[0], "null.wav");
    ambient_list[23].name_count = 1;
    strcpy(ambient_list[23].names[0], "null.wav");
}

// Empty sound-error hook.
// FUNCTION: C2 0x13186
void sound_error(void)
{
}

// Initialize the province ambient-sound bank.
// FUNCTION: C2 0x13187
// FUNCTION: C2WIN 0x00403999
void init_prov_ambients(void)
{
    int i;

    for (i = 0; i < 0x19; i++) {
        ambient_list[i].active = 0;
        ambient_list[i].delay_counter = (i * 8);
        ambient_list[i].name_idx = 0;
        ambient_list[i].volume = 1;
    }
    ambient_list[1].name_count = 4;
    strcpy(ambient_list[1].names[0], "birdsp2.wav");
    strcpy(ambient_list[1].names[1], "birdsp3.wav");
    strcpy(ambient_list[1].names[2], "birdsp4.wav");
    strcpy(ambient_list[1].names[3], "birdsp5.wav");
    ambient_list[2].name_count = 1;
    strcpy(ambient_list[2].names[0], "mining3.wav");
    ambient_list[3].name_count = 4;
    strcpy(ambient_list[3].names[0], "surf1.wav");
    strcpy(ambient_list[3].names[1], "surf2.wav");
    strcpy(ambient_list[3].names[2], "shore1.wav");
    strcpy(ambient_list[3].names[3], "shore2.wav");
    ambient_list[4].name_count = 2;
    strcpy(ambient_list[4].names[0], "shipyrd1.wav");
    strcpy(ambient_list[4].names[1], "shipyrd2.wav");
    ambient_list[5].name_count = 3;
    strcpy(ambient_list[5].names[0], "warehse1.wav");
    strcpy(ambient_list[5].names[1], "warehse2.wav");
    strcpy(ambient_list[5].names[2], "warehse3.wav");
    ambient_list[6].name_count = 1;
    strcpy(ambient_list[6].names[0], "quarry.wav");
    ambient_list[7].name_count = 1;
    strcpy(ambient_list[7].names[0], "trading.wav");
    ambient_list[8].name_count = 1;
    strcpy(ambient_list[8].names[0], "marchb2.wav");
    ambient_list[9].name_count = 1;
    strcpy(ambient_list[9].names[0], "marchr.wav");
    ambient_list[10].name_count = 1;
    strcpy(ambient_list[10].names[0], "uprise.wav");
    ambient_list[11].name_count = 4;
    strcpy(ambient_list[11].names[0], "farm3.wav");
    strcpy(ambient_list[11].names[1], "farm4.wav");
    strcpy(ambient_list[11].names[2], "farm5.wav");
    strcpy(ambient_list[11].names[3], "farm6.wav");
    ambient_list[12].name_count = 1;
    strcpy(ambient_list[12].names[0], "null.wav");
}

// Initialize the battle ambient-sound bank.
// FUNCTION: C2 0x13351
// FUNCTION: C2WIN 0x00403cab
void init_battle_ambients(void)
{
    int i;

    for (i = 0; i < 0x19; i++) {
        ambient_list[i].active = 0;
        ambient_list[i].delay_counter = (i * 8);
        ambient_list[i].name_idx = 0;
        ambient_list[i].volume = 1;
    }
    ambient_list[1].name_count = 2;
    strcpy(ambient_list[1].names[0], "bowlau.wav");
    strcpy(ambient_list[1].names[1], "bowslau.wav");
    ambient_list[3].name_count = 1;
    strcpy(ambient_list[3].names[0], "bowhit.wav");
    ambient_list[4].name_count = 2;
    strcpy(ambient_list[4].names[0], "sling4.wav");
    strcpy(ambient_list[4].names[1], "sling5.wav");
    ambient_list[6].name_count = 1;
    strcpy(ambient_list[6].names[0], "slinght.wav");
    ambient_list[7].name_count = 1;
    strcpy(ambient_list[7].names[0], "axe.wav");
    ambient_list[8].name_count = 1;
    strcpy(ambient_list[8].names[0], "swordht.wav");
    ambient_list[9].name_count = 1;
    strcpy(ambient_list[9].names[0], "clubht.wav");
    ambient_list[10].name_count = 1;
    strcpy(ambient_list[10].names[0], "spearht.wav");
    ambient_list[11].name_count = 1;
    strcpy(ambient_list[11].names[0], "knieht.wav");
    ambient_list[12].name_count = 1;
    strcpy(ambient_list[12].names[0], "deathr.wav");
    ambient_list[13].name_count = 1;
    strcpy(ambient_list[13].names[0], "deathb.wav");
    ambient_list[14].name_count = 1;
    strcpy(ambient_list[14].names[0], "elephant.wav");
    ambient_list[15].name_count = 2;
    ambient_list[15].volume = 3;
    strcpy(ambient_list[15].names[0], "soldadv.wav");
    strcpy(ambient_list[15].names[1], "armadv.wav");
    ambient_list[16].name_count = 2;
    ambient_list[16].volume = 2;
    strcpy(ambient_list[16].names[0], "singcav.wav");
    strcpy(ambient_list[16].names[1], "armcav.wav");
    ambient_list[17].name_count = 1;
    ambient_list[17].volume = 1;
    strcpy(ambient_list[17].names[0], "mobadv.wav");
    ambient_list[18].name_count = 1;
    ambient_list[18].volume = 1;
    strcpy(ambient_list[18].names[0], "mobcav2.wav");
    ambient_list[19].name_count = 2;
    ambient_list[19].volume = 3;
    strcpy(ambient_list[19].names[0], "meleeam.wav");
    strcpy(ambient_list[19].names[1], "melee2a.wav");
}


// Allocate storage for the requested number of cached 20,000-byte samples.
// FUNCTION: C2 0x13546
// FUNCTION: C2WIN 0x00403fd7
int init_sample_buffer(int sample_count)
{
    sample_buffer = 0;
    if (samples_running != 0 && c2inf.samples_on != 0) {
        sample_buffer = malloc(sample_count * 20000);
        if (sample_buffer == 0) return 0;
    }
    return 1;
}

// Release the cached-sample storage.
// FUNCTION: C2 0x1358a
void free_sample_buffer(int sample_count)
{
    if (sample_buffer == 0) return;
    free(sample_buffer);
#if PLATFORM_WINDOWS
    sample_buffer = 0;
#endif
}

// Report successful initialization of the statically allocated tune buffer.
// FUNCTION: C2 0x1359e
// FUNCTION: C2WIN 0x0040408f
int init_tune_buffer(void)
{
    return 1;
}

// Leave the statically allocated tune buffer in place.
// FUNCTION: C2 0x135a3
void free_tune_buffer(void)
{
}
