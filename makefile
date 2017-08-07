CC=gcc
LIBS=`pkg-config --libs libssl,ncurses,menu --cflags libssl,ncurses,menu`
rfcreader:main.c rebuild.c settings.h settings.c update.c
	$(CC) $(LIBS) main.c rebuild.c settings.c update.c -o $@
