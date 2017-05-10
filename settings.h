#ifndef SETTINGS
#define SETTINGS
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <locale.h>
#include <menu.h>
#include <curses.h>
#include <stdlib.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <netdb.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
struct configs {
	char *lm;				/* дата последней модификации */
	char *flm;			/* файл обновления */
	char *cdir; 		/* каталог настроек */
	char *index; 		/* файл список */
	char *datadir; 	/* каталог документов rfc */
	char *txtviewer;/* просмотр текстовых файлов */
	char *pdfviewer;/* просмотр pdf файлов */
	unsigned int reg:1 __attribute__ ((aligned(32)));/* регистро независимый поиск */
	enum fgcolor {
		BLACK,
		RED,
		GREEN,
		YELLOW,
		BLUE,
		MAGENTA,
		CYAN,
		WHITE
	}fgcolor,bgcolor,sfgcolor,sbgcolor;
};
struct configs *cf;

#define if_have_path_and_file_of_list if (( cf->datadir ) && ( cf->list ))
#define read_every_line_in_config_file  while(fgets(line,127,conf)!=NULL)
int length; /* длина для информационног окна, если длина превышает, написать на следующей строке */
#endif
