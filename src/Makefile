XCBMLIB = ../bin

CFLAGS = -g -O5 -finline-functions -Wall -pedantic -Wno-parentheses

LIBS = -lc -lncurses -lm

all: c64

ifeq (xcbm.dep,$(wildcard xcbm.dep))
include xcbm.dep
endif

.c.o:
	$(CC) $(CFLAGS) -I. -c -o $*.o $<

COMMOBJ = 	curses.o \
		trap.o \
		emu6502.o \
		log.o \
		timer.o \
		convert.o\
		iec.o \

C64OBJ = 	c64/mem64.o \
		c64/emu64.o \
		c64/iec64.o \
		c64/c64io.o \
		c64/sound.o \
		c64/c64keys.o \


c64: $(C64OBJ) $(COMMOBJ)
	cc  -o $(XCBMLIB)/xcbm  $(C64OBJ) $(COMMOBJ) $(LIBS)

clean:
	rm -f $(COMMOBJ) $(C64OBJ) xcbm.dep c64.log

dep depend:
	cc -M -E *.c > xcbm.dep

