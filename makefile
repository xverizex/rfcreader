CC=gcc
PATHI=/usr/local/bin
LIBS=`pkg-config --libs libssl,ncurses,menu --cflags libssl,ncurses,menu`
rfcreader:main.c rebuild.c settings.h settings.c update.c
	$(CC) main.c rebuild.c settings.c update.c $(LIBS) -o $@
clean:
	rm rfcreader
install:
	strip rfcreader
	install rfcreader $(PATHI)
