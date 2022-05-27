ARCHIVE=http://www.zimmers.net/anonftp/pub/cbm

ROMS=petbasic4.rom petkernel4.rom petedit4.rom

all: $(ROMS)

%.bin:
	curl -o $@ $(ARCHIVE)/firmware/computers/pet/$@

petbasic4.rom: basic-4-b000.901465-23.bin basic-4-c000.901465-20.bin basic-4-d000.901465-21.bin
	cat $^ > $@

petkernel4.rom: kernal-4.901465-22.bin
	cat $^ > $@

# BASIC4 N-keyboard, no CRTC
petedit4.rom: edit-4-n.901447-29.bin
	cat $^ > $@

clean:
	rm -f $(ROMS) *.bin


