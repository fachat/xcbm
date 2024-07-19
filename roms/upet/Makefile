ARCHIVE=https://github.com/fachat/MicroPET.git

ROMS=spi.rom

all: $(ROMS)

MicroPET: 
	git clone ${ARCHIVE}

MicroPET/ROM/spiimg: MicroPET
	(cd MicroPET/ROM; make)

${ROMS}: MicroPET/ROM/spiimg
	cp $< $@

clean:
	rm -f $(ROMS) 


