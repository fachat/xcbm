
void colram_wr(scnt,scnt);

extern uchar colram[];

#define	colram_rd(a) 	colram[(a)]

void video_wr(scnt,scnt);
scnt video_rd(scnt);

void setvideopage(scnt);

