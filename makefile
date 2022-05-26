CC=gcc
PATHI=/usr/local/bin
LIBS=`pkg-config --libs ncurses,menu,libcurl --cflags ncurses,menu,libcurl`
rfcreader:main.c rebuild.c settings.h settings.c update.c
	$(CC) main.c rebuild.c settings.c update.c $(LIBS) -o $@
clean:
	rm rfcreader
install:
	strip rfcreader
	install rfcreader $(PATHI)
