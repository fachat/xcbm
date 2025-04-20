// Commander X16 Emulator
// Copyright (c) 2019 Michael Steil
// Copyright (c) 2020 Frank van den Hoef
// All rights reserved. License: 2-clause BSD
#ifndef __APPLE__
#define _XOPEN_SOURCE   600
#define _POSIX_C_SOURCE 1
#endif

#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "rtc.h"
#include "files.h"

#include "log.h"

#define VERBOSE 2

static FILE *rtc_file = NULL;
static char rtc_path[PATH_MAX] = "";
bool rtc_attached = false;

static int rtc_ptr = 0;
static char rtc_sram[64];
static char rtc_eeprom[128];

static bool selected = false;

/* This routine is called on init, to set up the
 * actual clock offset.
 *
 * TODO: This is not implemented yet.
 */
void rtc_init() 
{
	memset(rtc_sram, 64, 1);
	memset(rtc_eeprom, 128, 1);
}

static void rtc_detach()
{
	if (rtc_attached) {
		logout(0, "RTC file detached.");
		rtc_attached = false;
	}
}

void
rtc_set_path(char const *path)
{
	rtc_detach();

	strncpy(rtc_path, path, PATH_MAX);
	rtc_path[PATH_MAX-1] = '\0';

	rtc_attach();
}

static bool
rtc_path_is_set()
{
	return strlen(rtc_path) > 0;
}

void
rtc_attach()
{
	if (!rtc_attached && rtc_path_is_set()) {
		logout(0, "RTC file attached.");
		rtc_attached = true;
	}
}

void
rtc_select(bool select)
{
	selected = select;
#if defined(VERBOSE) && VERBOSE >= 2
	if (selected) {
		logout(0, "*** SD card select: %u", select);
	}
#endif
}


uint8_t
rtc_handle(uint8_t inbyte)
{
	if (!selected || (rtc_file == NULL)) {
		return 0xFF;
	}
	// printf("rtc_handle: %02X\n", inbyte);

	uint8_t outbyte = 0xFF;

	if (inbyte == 0xFF) {
		// send response data

	} else {
		// rxbuf[rxbuf_idx++] = inbyte;

	}
	return outbyte;
}

