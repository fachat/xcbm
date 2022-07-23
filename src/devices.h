
#define PAR_STATUS_OK                   0x00
#define PAR_STATUS_TIMEOUT              0x02
#define PAR_STATUS_EOI                  0x40
#define PAR_STATUS_DEVICE_NOT_PRESENT   0x80
#define PAR_STATUS_LISTENING		0x2000
#define PAR_STATUS_TALKING		0x4000

void devices_init(void);

typedef struct device {
                int     timeout;
                void    (*out)(uchar, int isatn, struct device*);   /* output */
                uchar   (*get)(struct device*, uchar *status, uchar ack); /* input */
                int     type;           /* type of device */
} device;

// set the device for a unit number (e.g. use a VC1541 device type for device unit 8)
void device_set(int unit, device *dev);

// get the current device for a unit
device *device_get(int unit);


#define MODE_FREE       0       /* bus is free, next should be listen or talk */
#define MODE_LISTEN     1       /* received a Listen command */
#define MODE_TALK       2       /* received a talk command */
#define MODE_OPEN       3
#define MODE_READ       4
#define MODE_WRITE      5
#define MODE_DIR        6


