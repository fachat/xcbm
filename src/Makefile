XCBMLIB = ../bin

CFLAGS = -g -O5 -finline-functions -Wall -pedantic -Wno-parentheses

LIBS = -lc -lncurses -lm

all: c64

ifeq (xcbm.dep,$(wildcard xcbm.dep))
include xcbm.dep
endif

.c.o:
	$(CC) $(CFLAGS) -I. -g -c -o $*.o $<

COMMOBJ = 	curses.o \
		trap.o \
		emu6502.o \
		log.o \
		timer.o \
		convert.o\
		iec.o \
		mem.o \

C64OBJ = 	c64/mem64.o \
		c64/emu64.o \
		c64/iec64.o \
		c64/c64io.o \
		c64/sound.o \
		c64/c64keys.o \
		c64/vicii.o \

PETOBJ = 	pet/petemu.o \
		pet/petmem.o \
		pet/petieee.o \
		pet/petio.o \
		pet/petvideo.o \
#		pet/petkeys.o \


xcbm4032: $(PETOBJ) $(COMMOBJ)
	cc  -o $(XCBMLIB)/xcbm4032  $(PETOBJ) $(COMMOBJ) $(LIBS)

xcbm64: $(C64OBJ) $(COMMOBJ)
	cc  -o $(XCBMLIB)/xcbm64  $(C64OBJ) $(COMMOBJ) $(LIBS)

clean:
	rm -f $(COMMOBJ) $(C64OBJ) $(PETOBJ) xcbm.dep c64.log

dep depend:
	cc -M -E -I. */*.c *.c > xcbm.dep

