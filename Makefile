
all: lib65816.a
	make -C src
	make -C roms

clean:
	make -C src clean
	make -C roms clean
	rm -f c64.log

distclean: clean
	make -C roms clean

lib65816/build/lib65816.a: lib65816
	(cd lib65816; cmake -S . -B build)
	(cd lib65816/build; make CCOPTS='-DDEBUG')

lib65816.a: lib65816/build/lib65816.a
	cp lib65816/build/lib65816.a .
	
lib65816:
	git clone https://github.com/sam-falvo/lib65816.git
	
