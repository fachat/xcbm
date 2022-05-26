
XCBMLIB = ../bin

CFLAGS = -g -pedantic -Wall

LIBS = -lncurses -ltermcap -lm


all: uncpk printcbm 

uncpk: uncpk.c
	cc uncpk.c -o $(XCBMLIB)/uncpk

printcbm: printcbm.c
	cc printcbm.c -o $(XCBMLIB)/printcbm

clean:
	rm *.o 
