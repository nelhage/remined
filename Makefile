CC=gcc
CFLAGS=`sdl-config --cflags` -Wall -O3
LDFLAGS=`sdl-config --libs`

remined: remined.o

remined.o: remined.c remined.h

clean:
	rm -f remined.o remined
