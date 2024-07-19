

/* called from mem module to tell the CRTC emu about the video memory */
void vmem_set(uchar *vram, scnt mask);

/* write functions for the whole 4k at $8xxx, i.e. video + col RAM */
void vmem_wr(meminfo_t *inf, scnt,scnt);

/* write to video memory */
void wrvid(scnt, scnt);

/* CRTC emulation */
void crtc_wr(scnt, scnt);
scnt crtc_rd(scnt);

