

/* called from mem module to tell the CRTC emu about the video memory */
void viccy2_init();

/* CRTC emulation */
void viccy2_wr(scnt, scnt);
scnt viccy2_rd(scnt);

