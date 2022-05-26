ARCHIVE=http://www.zimmers.net/anonftp/pub/cbm

all: c64kernl.rom c64basic.rom

c64kernl.rom:
	curl -o c64kernl.rom $(ARCHIVE)/firmware/computers/c64/kernal.901227-03.bin

c64basic.rom:
	curl -o c64basic.rom $(ARCHIVE)/firmware/computers/c64/basic.901226-01.bin

clean:
	rm -f c64kernl.rom c64basic.rom

