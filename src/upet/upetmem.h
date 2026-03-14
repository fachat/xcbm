
/* use a 2M mapping, so $f0xxxx wraps to zero again, 
   but without bank maping */

#define	UPETPAGES	512		/* 2M for now */
#define	UPETPAGESMASK	0x1fffff	/* 2M for now */

void mem_set_map(byte b);
void mem_set_bank(byte b);
void mem_set_vidblk(byte b);
void mem_set_vctrl(byte b);


