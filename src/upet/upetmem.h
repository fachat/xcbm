

#define MP_RAM0		0	/* RAM1-7= MP_RAM0+{1-7} */
#define	MP_VRAM		8	

#define	MP_ROM_OFFSET	9

#define	MP_EXT9		0
#define	MP_EXTA		1
#define	MP_BASIC	2	/* 3, 4 */
#define	MP_EDIT		5
#define	MP_KERNEL	6

void mem_set_map(byte bank);
void mem_set_bank(byte bank);
void mem_set_vidblk(byte bank);


