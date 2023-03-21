

#define	MP_VRAM		0	
#define	MP_EXT9		1
#define	MP_EXTA		2
#define	MP_BASIC	3	/* 11, 12, 13 */
#define	MP_EDIT		6
#define	MP_KERNEL	7

#define	CSAPAGES	256
#define	CSAPAGESMASK	0xfffff


extern uchar *colram;

void mmu_wr(scnt addr, scnt val);
scnt mmu_rd(scnt addr);

