
-------------------------------------------------------------------------------

             README File for 'xcbm', the Commodore C64 emulator

          (c) 1994-98 Andre Fachat, fachat@galileo.rhein-neckar.de

                       Version 0.1.2 / 13. Jul. 1998

-------------------------------------------------------------------------------


xcbm is an emulator for the commodore 8 bit computer cbm 3032 and c64.
(at least it will be someday ;-))

The programm is copyrighted by Andre Fachat and is covered by the
GNU Public license. For copying this programm see the file 'COPYING'
included in this package.

This programm comes with absolutely no warranty of any kind for what 
happens due to this programm. 

The current version number is 0.1.2. This means, that not much has been
tested and it definitely has lots of bugs. And it lacks lots of
stuff to really emulate. 
On the other hand the programm can run some BASIC and machine language
programms in c64 mode that do not require that much hardware simulation.

To run the emulator, you have to get the ROM images from the C64.
For text mode you need the basic and the kernel ROM. They have to 
have a leading load address, i.e. these two address bytes to tell
a normal C64 where to load the stuff. However, they have to be there
but are ignored by the emulator. To get them try (on an original C64):

10 open 1,8,1,"c64basic.rom"
30 for i=10*4096 to 12*4096-1
40 print#1,chr$(peek(i));
50 next
60 close1

(This is from my mind, but I think it is correct). For the kernel,
change the loop to "for i=14*4096 to 65535" and of course the filename. 
The character ROM is also not included, but for curses modes, it is not 
necessary.

I don't do anything on this emulator anymore (since 1994 actually)
as I have joined the VICE emulator team. However, people have expressed
interest in a curses-based emulator. So I fixed some little annoyances
and a bug now and released 0.1.2.
However, this emulator is horribly slow as it lacks all necessary
optimization. Especially the main CPU loop could be made into a 
big "case" switch() as it is much faster etc.

A cross assembler for 6502 computer is also available, so you can
develop programms without even having a 6502. For more info, mail me.

Andre Fachat

email: fachat@physik.tu-chemnitz.de

-------------------------------------------------------------------------------

BUGS/FEATURES (don't expect too much): 

- no doc available yet. see the file "MODES" on what is planned.
  see the file TODO for a brief introduction what has to be done.
  unfortunately the source is not documented very well.
- no undocumented opcodes (still exits) and no CMOS opcodes (remind me
  to create a CMOS command line switch)
- curses is hard to try: block graphic characters are not emulated,
  colors only very buggy, I don't understand it that much at the moment.
- no X yet, no cbm3032 yet, no 'fancy' mode yet (If you call trapping the
  IRQ address at ea31 fancy, compared to just trap hardware access, it is
  somehow fancy. I tried to just use hardware access but I got much worse
  keypress response time.)
- CIA only partially emulated, VIC only regs 24 and 33, no SID
- no real timing. IRQ timer gets decremented by ten for each executed
  opcode. This gives a curser blink rate at roughly original speed (may
  be a bit slower, but I have long time no real C64 seen).
- Can use Un*x filesystem subdirectories as disks. At the moment, the 
  current directory for unit 8, drive 0 is ".". You can set this at
  command line. no real disk command ("R","S",...) is emulated
  (what is a second window for? ;-), no just kidding, it should be
  possible in the future, but low priority for now)
- The current ROM directory is "/var/lib/xcbm" 
- sigint not yet trapped, i.e. no run/stop possible. curses gave me some 
  more trouble when trying to get some special keys (e.g. shift pos1 to
  emulated the clear key doesn't work ????)
- I have a function "logout" that gives output to the file "c64.log".
  here you can read some info on what this programm is doing and you
  can put some debug info there, if you want.
- exrom and game lines not yet used in memory management.
- the misc directory contains a programm to list basic files to stdout

- compiling: go to the src directory and type 'make clean;make dep; make'. 
  This puts the binary in the now empty bin directory.


