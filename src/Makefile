XCBMLIB = ../bin

CFLAGS = -g -O5 -finline-functions -Wall -pedantic -Wno-parentheses

LIBS = -lc -lncurses -lm

all: xcbm64 xcbm8032 xcsa

COMMOBJ = 	curses.o \
		emu6502.o \
		asm6502.o \
		log.o \
		timer.o \
		convert.o\
		devices.o \
		vdrive.o \
		mem.o \
		speed.o \
		config.o \

C64OBJ = 	c64/mem64.o \
		c64/emu64.o \
		c64/iec64.o \
		c64/c64io.o \
		c64/sound.o \
		c64/c64keys.o \
		c64/vicii.o \

PETOBJ = 	pet/petemu.o \
		pet/petmem.o \
		pet/petio.o \
		pet/petvideo.o \
		pet/petkeys.o \
		pia.o \
		via.o \
		parallel.o \
		pet/piavia.o \

CSAOBJ = 	csa/xcsa.o \
		csa/csamem.o \
		csa/csaio.o \
		pet/petvideo.o \
		pet/petkeys.o \
		pia.o \
		via.o \
		parallel.o \
		pet/piavia.o \

MONOBJ =	mon/mon.o \
		mon/labels.o

ALLOBJ =	$(COMMONOBJ) $(C64OBJ) $(PETOBJ) $(MONOBJ) $(CSAOBJ)

DEPDIR=.dep

include $(wildcard $(DEPDIR)/*.d)
include $(wildcard $(DEPDIR)/*/*.d)

MAKEDEPEND = $(CC) -E -MM -I. $< > $(DEPDIR)/$*.d
#MAKECSADEPEND = $(CC) -E -MM -I. -Ipet $< > $(DEPDIR)/$*.d

$(DEPDIR):
	@mkdir -p $@ $@/pet $@/c64 $@/csa $@/mon

%.o : %.c $(DEPDIR)
	$(MAKEDEPEND)
	$(CC) $(CFLAGS) -I. -c -o $@ $<

#csa/%.o : csa/%.c $(DEPDIR)
#	$(MAKECSADEPEND)
#	$(CC) $(CFLAGS) -I. -Ipet -c -o $@ $<


depfiles: $(DEPDIR) $(DEPFILES)
 
$(XCBMLIB):
	test -d $(XCBMLIB) || mkdir $(XCBMLIB)

xcbm8032: $(XCBMLIB) $(PETOBJ) $(COMMOBJ) $(MONOBJ)
	cc  -o $(XCBMLIB)/xcbm8032  $(PETOBJ) $(COMMOBJ) $(MONOBJ) $(LIBS)

xcbm64: $(XCBMLIB) $(C64OBJ) $(COMMOBJ) $(MONOBJ)
	cc  -o $(XCBMLIB)/xcbm64  $(C64OBJ) $(COMMOBJ) $(MONOBJ) $(LIBS)

xcsa: $(XCBMLIB) $(CSAOBJ) $(COMMOBJ) $(MONOBJ)
	cc  -o $(XCBMLIB)/xcsa  $(CSAOBJ) $(COMMOBJ) $(MONOBJ) $(LIBS)

clean:
	rm -f $(COMMOBJ) $(C64OBJ) $(PETOBJ) $(MONOBJ) xcbm.dep c64.log
	rm -rf $(DEPDIR)

