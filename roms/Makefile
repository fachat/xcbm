
all: romcheck rompatch parts.sdcard
	make -C c64
	make -C pet
	make -C upet
	make -C csa

clone:
	(cd upet; make clone)

update:
	(cd upet; make update)

romcheck: romcheck.c
	cc $< -o $@

rompatch: rompatch.c
	cc $< -o $@

parts.sdcard: parts.sdcard.gz
	gzip -c $< > $@

clean:
	make -C c64 clean
	make -C pet clean
	make -C csa clean
	make -C upet clean
	rm -f romcheck rompatch

