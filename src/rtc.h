// Commander X16 Emulator
// Copyright (c) 2019 Michael Steil
// All rights reserved. License: 2-clause BSD
#ifndef _RTC_H
#define _RTC_H
#include <inttypes.h>
#include <stdbool.h>
//#include <SDL.h>

extern bool rtc_attached;
void rtc_set_path(char const *path);
void rtc_attach();
void rtc_init();

void rtc_select(bool select);
uint8_t rtc_handle(uint8_t inbyte);

#endif

