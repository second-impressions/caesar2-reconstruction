#include "pump.h"
#include "c2_data.h"

unsigned char p_len[64] = { 3, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 };

unsigned char p_code[64] = { 0, 32, 48, 64, 80, 88, 96, 104, 112, 120, 128, 136, 144, 148, 152, 156, 160, 164, 168, 172, 176, 180, 184, 188, 192, 194, 196, 198, 200, 202, 204, 206, 208, 210, 212, 214, 216, 218, 220, 222, 224, 226, 228, 230, 232, 234, 236, 238, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255 };

unsigned char d_code[256] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17, 17, 17, 17, 18, 18, 18, 18, 19, 19, 19, 19, 20, 20, 20, 20, 21, 21, 21, 21, 22, 22, 22, 22, 23, 23, 23, 23, 24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 31, 32, 32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 38, 38, 39, 39, 40, 40, 41, 41, 42, 42, 43, 43, 44, 44, 45, 45, 46, 46, 47, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63 };

unsigned char d_len[256] = { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 };

/* File-local compression state. */
short *dad;
unsigned char *pmp_inbuff;
int pmp_length;
unsigned short *freq;
int pmp_optr;
int pmp_iptr;
int textsize;
short *rson;
short *lson;
unsigned char *pmp_outbuff;
short *prnt;
unsigned char *text_buf;
int codesize;
int pmp_mem_needed;
short *son;
short match_length;
unsigned short getbuf;
unsigned short putbuf;
short match_position;
unsigned char putlen;
unsigned char getlen;

/* Okumura/Yoshizaki LZHUF parameters. */
#define N           4096        /* size of string buffer */
#define F           60          /* lookahead buffer size */
#define THRESHOLD   2
#define NIL         N           /* tree leaf sentinel */
#define N_CHAR      (256 - THRESHOLD + F)
#define T           (N_CHAR * 2 - 1)
#define R           (T - 1)
#define MAX_FREQ    0x8000

extern void  memmove(void *dst, const void *src, unsigned int n);
extern void *calloc(unsigned int nmemb, unsigned int size);
extern void free(void *);
/* Forward declarations (functions defined later in this file). */
void InsertNode(short r);
void DeleteNode(short p);
void Putcode(short l, unsigned short c);
void update(short c);
void EncodeChar(unsigned short c);
void EncodePosition(unsigned short c);
int evacuate(unsigned char *src, unsigned char *dst);
void free_pumping_memory(void);


// Initialize the LZSS match tree with every node detached.
// FUNCTION: C2 0x6f535
// FUNCTION: C2WIN 0x0043b720
void InitTree(void)
{
    short i;
    for (i = 0x1001; i <= 0x1100; i++) {
        rson[i] = 0x1000;
    }
    for (i = 0; i < 0x1000; i++) {
        dad[i] = 0x1000;
    }
}

// Insert a window position into the LZSS search tree and record its best match.
// FUNCTION: C2 0x6f575
// FUNCTION: C2WIN 0x0043b794
void InsertNode(short r)
{
    unsigned short c;
    short cmp;
    short i;
    short p;
    unsigned char *key;

    cmp = 1;
    key = &text_buf[r];
    p = (N + 1) + key[0];
    rson[r] = lson[r] = NIL;
    match_length = 0;
    for (;;) {
        if (cmp >= 0) {
            if (rson[p] != NIL)
                p = rson[p];
            else {
                rson[p] = r;
                dad[r] = p;
                return;
            }
        } else {
            if (lson[p] != NIL)
                p = lson[p];
            else {
                lson[p] = r;
                dad[r] = p;
                return;
            }
        }
        for (i = 1; i < F; i++)
            if ((cmp = key[i] - text_buf[p + i]) != 0)
                break;
        if (i > THRESHOLD) {
            if (i > match_length) {
                match_position = ((r - p) & (N - 1)) - 1;
                if ((match_length = i) >= F)
                    break;
            }
            if (i == match_length) {
                if ((c = ((r - p) & (N - 1)) - 1) < match_position)
                    match_position = c;
            }
        }
    }
    dad[r] = dad[p];
    lson[r] = lson[p];
    rson[r] = rson[p];
    dad[lson[p]] = r;
    dad[rson[p]] = r;
    if (rson[dad[p]] == p)
        rson[dad[p]] = r;
    else
        lson[dad[p]] = r;
    dad[p] = 0x1000;
}

// Remove a window position from the LZSS search tree before reusing its slot.
// FUNCTION: C2 0x6f75a
// FUNCTION: C2WIN 0x0043bab6
void DeleteNode(short p)
{
    short q;

    if (dad[p] == NIL)
        return;
    if (rson[p] == NIL) {
        q = lson[p];
    } else if (lson[p] == NIL) {
        q = rson[p];
    } else {
        q = lson[p];
        if (rson[q] != NIL) {
            do {
                q = rson[q];
            } while (rson[q] != NIL);
            rson[dad[q]] = lson[q];
            dad[lson[q]] = dad[q];
            lson[q] = lson[p];
            dad[lson[p]] = q;
        }
        rson[q] = rson[p];
        dad[rson[p]] = q;
    }
    dad[q] = dad[p];
    if (rson[dad[p]] == p) {
        rson[dad[p]] = q;
    } else {
        lson[dad[p]] = q;
    }
    dad[p] = NIL;
}

// Read one bit from the compressed input stream.
// FUNCTION: C2 0x6f8b5
// FUNCTION: C2WIN 0x0043bce1
short GetBit(void)
{
    short i;

    while (getlen <= 8) {
        i = pmp_inbuff[pmp_iptr++];
        i &= 0xff;
        getbuf |= i << (8 - getlen);
        getlen += 8;
    }
    i = getbuf;
    getbuf <<= 1;
    getlen--;
    return (i < 0);
}

// Read one byte from the compressed input stream.
// FUNCTION: C2 0x6f923
// FUNCTION: C2WIN 0x0043bd95
int GetByte(void)
{
    unsigned short i;

    while (getlen <= 8) {
        i = pmp_inbuff[pmp_iptr++];
        i &= 0xff;
        getbuf |= i << (8 - getlen);
        getlen += 8;
    }
    i = getbuf;
    getbuf <<= 8;
    getlen -= 8;
    return i >> 8;
}

// Append the high `l` bits of `c` to the compressed output stream.
// FUNCTION: C2 0x6f993
// FUNCTION: C2WIN 0x0043be48
void Putcode(short l, unsigned short c)
{
    putbuf |= c >> putlen;
    if ((putlen += (unsigned char)l) >= 8) {
        pmp_outbuff[pmp_optr++] = putbuf >> 8;
        if ((putlen -= 8) >= 8) {
            pmp_outbuff[pmp_optr++] = putbuf;
            codesize += 2;
            putlen -= 8;
            putbuf = c << (l - putlen);
        } else {
            putbuf <<= 8;
            codesize++;
        }
    }
}

// Initialize the adaptive Huffman tree with equal symbol frequencies.
// FUNCTION: C2 0x6fa66
// FUNCTION: C2WIN 0x0043bf35
void StartHuff(void)
{
    short i;
    short j;

    for (i = 0; i < N_CHAR; i++) {
        freq[i] = 1;
        son[i] = i + T;
        prnt[i + T] = i;
    }
    i = 0;
    j = N_CHAR;
    while (j <= R) {
        freq[j] = freq[i] + freq[i + 1];
        son[j] = i;
        prnt[i] = prnt[i + 1] = j;
        i += 2;
        j++;
    }
    freq[T] = 0xffff;
    prnt[R] = 0;
}

// Rebuild the adaptive Huffman tree with halved frequencies.
// FUNCTION: C2 0x6fb1e
// FUNCTION: C2WIN 0x0043c061
void reconst(void)
{
    short i;
    short k;
    short j;
    unsigned short f;
    unsigned short l;

    j = 0;
    for (i = 0; i < T; i++) {
        if (son[i] >= T) {
            freq[j] = (freq[i] + 1) / 2;
            son[j] = son[i];
            j++;
        }
    }
    for (i = 0, j = N_CHAR; j < T; i += 2, j++) {
        k = i + 1;
        f = freq[j] = (freq[i] + freq[k]);
        for (k = j - 1; f < freq[k]; k--)
            ;
        k++;
        l = (j - k) * 2;
        memmove(&freq[k + 1], &freq[k], l);
        freq[k] = f;
        memmove(&son[k + 1], &son[k], l);
        son[k] = i;
    }
    for (i = 0; i < T; i++) {
        if ((k = son[i]) >= T) {
            prnt[k] = i;
        } else {
            prnt[k] = prnt[k + 1] = i;
        }
    }
}

// Update and reorder the adaptive Huffman tree after processing a symbol.
// FUNCTION: C2 0x6fc6e
// FUNCTION: C2WIN 0x0043c2d1
void update(short c)
{
    short i;
    short j;
    short k;
    short l;

    if (freq[R] == MAX_FREQ)
        reconst();
    c = prnt[c + T];
    do {
        k = ++freq[c];
        if (k > freq[l = (c + 1)]) {
            while (k > freq[++l])
                ;
            l--;
            freq[c] = freq[l];
            freq[l] = k;
            i = son[c];
            prnt[i] = l;
            if (i < T)
                prnt[i + 1] = l;
            j = son[l];
            son[l] = i;
            prnt[j] = c;
            if (j < T)
                prnt[j + 1] = c;
            son[c] = j;
            c = l;
        }
        c = prnt[c];
    } while (c);
}

// Encode one literal or match-length token with the adaptive Huffman tree.
// FUNCTION: C2 0x6fdb3
// FUNCTION: C2WIN 0x0043c486
void EncodeChar(unsigned short c)
{
    short j;
    unsigned short i;
    short k;

    i = j = 0;
    k = prnt[(unsigned short)c + 0x273];

    do {
        i >>= 1;
        if (k & 1) {
            i += 0x8000;
        }
        j++;
        k = prnt[k];
    } while (k != 0x272);

    Putcode((int)j, i);
    update((short)c);
}

// Encode an LZSS match distance using a table-coded prefix and six raw bits.
// FUNCTION: C2 0x6fe17
// FUNCTION: C2WIN 0x0043c51f
void EncodePosition(unsigned short c)
{
    unsigned short i;

    i = c >> 6;
    Putcode(p_len[i], (unsigned short)(p_code[i] << 8));
    Putcode(6, (unsigned short)((c & 0x3f) << 10));
}

// Flush the final partial byte from the compressed output bit buffer.
// FUNCTION: C2 0x6fe63
// FUNCTION: C2WIN 0x0043c57d
void EncodeEnd(void)
{
    if (putlen) {
        pmp_outbuff[pmp_optr++] = putbuf >> 8;
        codesize++;
    }
}

// Decode one literal or match-length token with the adaptive Huffman tree.
// FUNCTION: C2 0x6fe9d
// FUNCTION: C2WIN 0x0043c5ba
short DecodeChar(void)
{
    unsigned short c;

    c = son[R];
    while (c < T) {
        c += GetBit();
        c = son[c];
    }
    c -= T;
    update(c);
    return (short)c;
}

// Decode an LZSS match distance from its prefix and trailing bits.
// FUNCTION: C2 0x6fee1
// FUNCTION: C2WIN 0x0043c643
short DecodePosition(void)
{
    unsigned short i;
    unsigned short j;
    unsigned short c;

    i = GetByte();
    c = d_code[i] << 6;
    j = d_len[i];
    j -= 2;
    while (j--) {
        i = (i << 1) + GetBit();
    }
    return (short)(c | i & 0x3f);
}

// Allocate the LZSS and Huffman work tables, rolling back on failure.
// FUNCTION: C2 0x6ff25
// FUNCTION: C2WIN 0x0043c6e0
int get_pumping_memory(void)
{
    text_buf = calloc(0x103b, 1);
    lson    = calloc(0x1001, 2);
    rson    = calloc(0x1101, 2);
    dad     = calloc(0x1001, 2);
    freq    = calloc(0x274,  2);
    prnt    = calloc(0x3ad,  2);
    son     = calloc(0x273,  2);

    if (text_buf == 0 || lson == 0 || rson == 0 || dad == 0 ||
        freq == 0 || prnt == 0 || son == 0) {
        free_pumping_memory();
        return 0;
    }
    return 1;
}

// Free the LZSS and Huffman work tables.
// FUNCTION: C2 0x6fffc
// FUNCTION: C2WIN 0x0043c7e8
void free_pumping_memory(void)
{
#if C2_FEAT_PUMP_FREE_NULLS
    if (text_buf) { free(text_buf); text_buf = 0; }
    if (lson)     { free(lson);     lson = 0; }
    if (rson)     { free(rson);     rson = 0; }
    if (dad)      { free(dad);      dad = 0; }
    if (freq)     { free(freq);     freq = 0; }
    if (prnt)     { free(prnt);     prnt = 0; }
    if (son)      { free(son);      son = 0; }
#else
    if (text_buf) free(text_buf);
    if (lson)     free(lson);
    if (rson)     free(rson);
    if (dad)      free(dad);
    if (freq)     free(freq);
    if (prnt)     free(prnt);
    if (son)      free(son);
#endif
}

// Compress a buffer with LZSS and adaptive Huffman coding.
// The output header stores compressed and uncompressed sizes.
// FUNCTION: C2 0x7007f
// FUNCTION: C2WIN 0x0043c8f6
int pump(unsigned char *source_ptr, unsigned char *dest_ptr, int source_size)
{
    short i;
    short c;
    int length;
    short r;
    short s;
    int last_match_length;

    pmp_inbuff  = source_ptr;
    pmp_outbuff = dest_ptr;
    pmp_iptr    = 0;
    pmp_optr    = 8;
    pmp_length  = source_size;
    textsize    = 0;
    codesize    = 0;

    getbuf = 0;
    putbuf = 0;
    getlen = 0;
    putlen = 0;
    match_position = 0;
    match_length   = 0;

    if (!get_pumping_memory()) {
        return 0;
    }

    StartHuff();
    InitTree();

    s = 0;
    r = N - F;
    for (i = s; i < r; i++)
        text_buf[i] = ' ';
    length = 0;
    for (;;) {
        if (length >= F)
            break;
        c = pmp_inbuff[pmp_iptr++];
        if (pmp_iptr >= pmp_length)
            break;
        text_buf[r + length] = c;
        length++;
    }
    textsize = length;
    for (i = 1; i <= F; i++)
        InsertNode(r - i);
    InsertNode(r);

    lib_ret4 = 0;

    do {
        if (match_length > length)
            match_length = length;
        if (match_length <= THRESHOLD) {
            match_length = 1;
            EncodeChar(text_buf[r]);
        } else {
            EncodeChar((255 - THRESHOLD) + match_length);
            EncodePosition(match_position);
        }
        last_match_length = match_length;
        for (i = 0; i < last_match_length; i++) {
            if (pmp_iptr >= pmp_length)
                break;
            c = pmp_inbuff[pmp_iptr++];
            DeleteNode(s);
            text_buf[s] = c;
            if (s < F - 1)
                text_buf[s + N] = c;
            s = (s + 1) & (N - 1);
            r = (r + 1) & (N - 1);
            InsertNode(r);
        }
        while (i++ < last_match_length) {
            DeleteNode(s);
            s = (s + 1) & (N - 1);
            r = (r + 1) & (N - 1);
            if (--length)
                InsertNode(r);
        }
    } while (length > 0);

    EncodeEnd();

    my_strcpy((char *)&pmp_optr, (char *)dest_ptr, 4);
    my_strcpy((char *)&source_size, (char *)(dest_ptr + 4), 4);

    free_pumping_memory();
    return pmp_optr;
}

// Expand an LZSS/adaptive-Huffman buffer into `dst`.
// The input header stores compressed and uncompressed sizes.
// FUNCTION: C2 0x702df
// FUNCTION: C2WIN 0x0043cc5b
int evacuate(unsigned char *source_ptr, unsigned char *dest_ptr)
{
    short c;
    short r;
    short k;
    short i;
    short j;
    int length;
    unsigned int count;

    /* Header: byte 4..7 = uncompressed length. */
    my_strcpy((char *)(source_ptr + 4), (char *)&length, 4);
    pmp_inbuff  = source_ptr;
    pmp_outbuff = dest_ptr;
    pmp_iptr    = 8;
    pmp_optr    = 0;

    pmp_length = length;
    textsize   = length;
    codesize   = 0;

    getbuf = 0;
    putbuf = 0;
    getlen = 0;
    putlen = 0;
    match_position = 0;
    match_length   = 0;

    if (!get_pumping_memory()) {
        return 0;
    }

    StartHuff();

    for (i = 0; i < (N - F); i++) {
        text_buf[i] = ' ';
    }
    r = (N - F);
    for (count = 0; count < textsize; ) {
        c = DecodeChar();
        if (c < 256) {
            pmp_outbuff[pmp_optr++] = c;
            text_buf[r++] = c;
            r &= (N - 1);
            count++;
        } else {
            i = (r - DecodePosition() - 1) & (N - 1);
            j = c - (255 - THRESHOLD);
            for (k = 0; k < j; k++) {
                c = text_buf[(i + k) & (N - 1)];
                pmp_outbuff[pmp_optr++] = c;
                text_buf[r++] = c;
                r &= (N - 1);
                count++;
            }
        }
    }

    free_pumping_memory();
    return pmp_optr;
}
