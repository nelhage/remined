CFLAGS=`sdl-config --cflags` -Wall
LDFLAGS=`sdl-config --libs`

remined: remined.o

remined.o: remined.c remined.h

clean:
	rm -f remines.o remined
