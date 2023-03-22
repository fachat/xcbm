ARCHIVE=http://www.zimmers.net/anonftp/pub/cbm

ROMS=pet.rom 

BASIC=basic-4-b000.901465-23.bin basic-4-c000.901465-20.bin basic-4-d000.901465-21.bin
KERNEL=kernal-4.901465-22.bin
EDIT=edit-4-80-n_unk.bin


all: csa.rom

%.bin:
	curl -o $@ $(ARCHIVE)/firmware/computers/pet/$@

empty4k:
	dd if=/dev/zero of=empty4k bs=4096 count=1

pet.rom: $(BASIC) $(KERNEL) $(EDIT) empty4k
	cat empty4k empty4k empty4k $(BASIC) $(EDIT) $(KERNEL) > pet.rom

csa.rom: pet.rom initmmu reset
	../rompatch -o csa.rom -p 0x7e00 initmmu -p 0x7ffc reset pet.rom

initmmu: initmmu.a65
	xa -o $@ $<

reset: reset.a65
	xa -o $@ $<

clean:
	rm -f $(ROMS) *.bin empty2k empty4k initmmu pet.rom csa.rom reset



