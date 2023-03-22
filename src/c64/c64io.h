

extern uchar pra[256];
extern uchar prb[256];

int iec_init(void);
 
scnt io_peek(scnt addr);
scnt io_rd(scnt addr);
void io_wr(scnt addr, scnt val);

