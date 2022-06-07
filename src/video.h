
int video_init(CPU *cpu);

void video_wr(scnt,scnt);
scnt video_rd(scnt);

void setvideopage(scnt);

void video_set_status_line(const char *line);

typedef struct rgb_t {
                int     r;
                int     g;
                int     b;
                int     fg;     /* if only the 8 std colors are available */
} rgb_t;

extern int update;
extern rgb_t rgb[16];

