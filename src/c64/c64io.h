

extern uchar pra[256];
extern uchar prb[256];

int iec_init(void);
 
scnt io_peek(meminfo_t *inf, scnt addr);
scnt io_rd(meminfo_t *inf, scnt addr);
void io_wr(meminfo_t *inf, scnt addr, scnt val);

