CC=gcc
PROG=ctag
CURSES=-lncurses -lmenu -lform
CURSESW=-lncursesw -lmenuw -lformw -D_CURSESW_WIDE
FILES=ctag.c
CFLAGS+=-std=c89 -pedantic -Wall
LDFLAGS+=-L/usr/local/lib

all:
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(CURSES) $(FILES) -o $(PROG)
