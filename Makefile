CC=gcc
PROG=ctag
CURSES=-lncurses -lmenu -lform
CURSESW=-lncursesw -lmenuw -lformw -D_CURSESW_WIDE
FILES=ctag.c
CFLAGS+=-ansi -pedantic -Wall
CPPFLAGS+=-I/usr/local/include `taglib-config --cflags`
LDFLAGS+=-L/usr/local/lib `taglib-config --libs` -ltag_c -lstdc++

all:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(CURSES) $(FILES) -o $(PROG)
