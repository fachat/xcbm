
BASE=$(shell git remote -v | cut -f 2- | sed -e "s%fachat/.*%fachat%g" | uniq )
ARCHIVE=$(BASE)/upet_roms.git

ROMS=spi.rom

all: $(ROMS)

clone: upet_roms

update:
	(cd upet_roms; git pull)

upet_roms: 
	test -d ../../../roms && ln -s ../../../roms upet_roms || true
	test -d upet_roms || git clone ${ARCHIVE}

upet_roms/spiimg: upet_roms
	(cd upet_roms; make)

${ROMS}: upet_roms/spiimg
	cp $< $@

clean:
	rm -f $(ROMS) 


