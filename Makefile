
all: lib65816.a
	make -C src
	make -C roms

clone: lib65816
	(cd roms; make clone)

update:
	(cd lib65816; git pull)
	(cd roms; make update)

clean:
	make -C src clean
	rm -f c64.log

distclean: clean
	make -C roms clean

lib65816/build/lib65816.a: lib65816
	(cd lib65816; cmake -S . -B build)
	(cd lib65816/build; make CCOPTS='-DDEBUG')

lib65816.a: lib65816/build/lib65816.a
	cp lib65816/build/lib65816.a .
	
lib65816:
	git clone https://github.com/fachat/lib65816.git
	(cd lib65816; git checkout upet)

upetroms:
	make -C ../roms
	make
	./bin/xupet -S roms/parts.sdcard -8 0=.
	
