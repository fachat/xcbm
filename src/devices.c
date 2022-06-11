
/************************************************************************************
 * devices handling
 *
 * Holds the table for the virtual devices, so IEC/IEEE commands can be 
 * dispatched accordingly just from the unit number (IEEE address)
 */

#include <stddef.h>

#include "log.h"
#include "types.h"
#include "devices.h"

#define	NUM_DEVICES	30

static device *devs[NUM_DEVICES];

void devices_init(void) {
	for (int i = 0; i < NUM_DEVICES; i++) {
		devs[i] = NULL;
	}
}


// set the device for a unit number (e.g. use a VC1541 device type for device unit 8)
void device_set(int unit, device *dev) {
	logout(0, "set device for unit %d -> %p", unit, dev);
	if (unit >= 0 && unit < NUM_DEVICES) {
		devs[unit] = dev;
	}
}

// get the current device for a unit
device *device_get(int unit) {
	if (unit < 0 || unit >= NUM_DEVICES) {
		return NULL;
	}
	logout(0, "get device for unit %d -> %p", unit, devs[unit]);
	return devs[unit];
}


