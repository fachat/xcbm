XCBMLIB = ../bin

CFLAGS = -g -O5 -finline-functions -Wall -pedantic -Wno-parentheses

LIBS = -lc -lncurses -lm

all: c64

ifeq (xcbm.dep,$(wildcard xcbm.dep))
include xcbm.dep
endif

.c.o:
	$(CC) $(CFLAGS) -c -o $*.o $<

 
C64SRC = 	mem.o \
		emu65.o \
		log.o \
		emu6502.o \
		c64io.o \
		timer.o \
		sound.o \
		c64keys.o \
		curses.o \
		trap.o \
		iec.o \
		convert.o\

C64HDR =	c64.h \
		emu65.h \
		emu6502.h \
		log.h



c64: $(C64SRC) 
	cc  -o $(XCBMLIB)/xcbm  $(C64SRC)  $(LIBS)

clean:
	rm -f *.o xcbm.dep 

dep depend:
	cc -M -E *.c > xcbm.dep

