
int  lowloaded;
int  sounds;
int  trackbuf;
int  simspeed;

#if PLATFORM_DOS
/* Initializes a four-byte local buffer with two spaces. */
void __smackw32_text_pad1(void) { char text_padding[4] = "  "; (void)text_padding; }

/* Initializes a second four-byte local buffer with two spaces. */
void __smackw32_text_pad2(void) { char text_padding[4] = "  "; (void)text_padding; }
#endif
