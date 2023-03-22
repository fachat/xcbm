

/* called from mem module to tell the CRTC emu about the video memory */
void vmem_set(uchar *vram, scnt mask);

/* write functions for the whole 4k at $8xxx, i.e. video + col RAM */
void vmem_wr(scnt,scnt);

/* CRTC emulation */
void crtc_wr(scnt, scnt);
scnt crtc_rd(scnt);

