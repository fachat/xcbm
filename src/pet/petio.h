
#define	PIA1_INT_MASK	0x01
#define	PIA2_INT_MASK	0x02
#define	VIA_INT_MASK	0x04


scnt io_rd(scnt addr);
void io_wr(scnt addr, scnt val);


void io_set_vdrive(uchar flag);

