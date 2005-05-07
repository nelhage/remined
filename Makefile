CFLAGS=`sdl-config --cflags` -Wall
LDFLAGS=`sdl-config --libs`

mines: mines.o

mines.o: mines.c mines.h

clean:
	rm -f mines.o mines
