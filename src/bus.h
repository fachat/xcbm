
typedef struct BUS {
		struct CPU	*cpu;
		CLOCK		clk;
		alarm_context_t	actx;
		int		msperframe;
		int		cyclesperframe;
} BUS;
		

