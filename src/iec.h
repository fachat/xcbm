
int iec_init(void);
int iec_setdrive(int device, int drive, const char *pathname);

void iec_exit(void);

typedef struct device {
                int     timeout;
                void    (*out)(scnt, int isatn, struct device*);   /* output, uses global actdev */
                scnt    (*get)(struct device*, int *iseof); /* input, uses global actdev */
                int     type;           /* type of device */
} device;

#define MODE_FREE       0       /* bus is free, next should be listen or talk */
#define MODE_LISTEN     1       /* received a Listen command */
#define MODE_TALK       2       /* received a talk command */
#define MODE_OPEN       3
#define MODE_READ       4
#define MODE_WRITE      5
#define MODE_DIR        6


