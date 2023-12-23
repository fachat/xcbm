
all: romcheck rompatch
	make -C c64
	make -C pet

romcheck: romcheck.c
	cc $< -o $@

rompatch: rompatch.c
	cc $< -o $@

clean:
	make -C c64 clean
	make -C pet clean
	make -C csa clean
	rm -f romcheck rompatch

