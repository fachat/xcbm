
/*
#include	<stdio.h>
#include	<stdlib.h>
#include 	<string.h>
#include 	<errno.h>

#include	"log.h"
#include 	"config.h"
#include	"types.h"
#include	"alarm.h"
#include	"bus.h"
#include	"emu6502.h"
#include 	"mem.h"
#include 	"upetmem.h"
#include 	"petio.h"
#include 	"petvideo.h"
#include 	"mon.h"
*/



void spi_init();

void spi_start();

void spi_ipl(uchar *iplblk);

void spi_wr(scnt addr, scnt val);

scnt spi_rd(scnt addr);

