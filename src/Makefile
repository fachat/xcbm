XCBMLIB = ../bin

CFLAGS = -g -O5 -finline-functions -Wall -pedantic -Wno-parentheses

LIBS = -lc -lncurses -lm

all: xcbm64 xcbm8032 xcsa xupet

COMMOBJ = 	curses.o \
		log.o \
		timer.o \
		convert.o\
		devices.o \
		vdrive.o \
		mem.o \
		speed.o \
		config.o \
		stop.o \

C64OBJ = 	c64/mem64.o \
		c64/emu64.o \
		c64/iec64.o \
		c64/c64io.o \
		c64/sound.o \
		c64/c64keys.o \
		c64/vicii.o \
		emu6502.o \
		asm6502.o \

PETOBJ = 	pet/petemu.o \
		pet/petmem.o \
		pet/petio.o \
		pet/petvideo.o \
		pet/petkeys.o \
		pia.o \
		via.o \
		parallel.o \
		pet/piavia.o \
		emu6502.o \
		asm6502.o \

UPETOBJ = 	upet/upetemu.o \
		upet/upetmem.o \
		upet/upetio.o \
		upet/upetvideo.o \
		upet/ctrl.o \
		upet/spi.o \
		pet/petkeys.o \
		pia.o \
		via.o \
		parallel.o \
		pet/piavia.o \
		emu65816.o \
		asm6502.o \
		sdcard.o \

CSAOBJ = 	csa/xcsa.o \
		csa/csamem.o \
		csa/csaio.o \
		pet/petvideo.o \
		pet/petkeys.o \
		pia.o \
		via.o \
		parallel.o \
		pet/piavia.o \
		emu6502.o \
		asm6502.o \

MONOBJ =	mon/mon.o \
		mon/labels.o

ALLOBJ =	$(COMMONOBJ) $(C64OBJ) $(PETOBJ) $(MONOBJ) $(CSAOBJ) $(UPETOBJ)

DEPDIR=.dep

include $(wildcard $(DEPDIR)/*.d)
include $(wildcard $(DEPDIR)/*/*.d)

MAKEDEPEND = $(CC) -E -MM -I. $< > $(DEPDIR)/$*.d
#MAKECSADEPEND = $(CC) -E -MM -I. -Ipet $< > $(DEPDIR)/$*.d

$(DEPDIR):
	@mkdir -p $@ $@/pet $@/c64 $@/csa $@/mon $@/upet

%.o : %.c $(DEPDIR)
	$(MAKEDEPEND)
	$(CC) $(CFLAGS) -I. -c -o $@ $<

emu65816.o : emu65816.c $(DEPDIR)
	$(CC) $(CFLAGS) -I. -I../lib65816 -I../lib65816/build -c -o $@ $<

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

xupet: $(XCBMLIB) $(UPETOBJ) $(COMMOBJ) $(MONOBJ)
	cc  -o $(XCBMLIB)/xupet  $(UPETOBJ) $(COMMOBJ) $(MONOBJ) $(LIBS) ../lib65816.a

clean:
	rm -f $(COMMOBJ) $(C64OBJ) $(PETOBJ) $(MONOBJ) xcbm.dep c64.log
	rm -rf $(DEPDIR)

