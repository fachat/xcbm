
all:
	make -C src
	make -C roms

clean:
	make -C src clean
	rm -f c64.log

distclean: clean
	make -C roms clean

