/*
 * rfcreader - навигация по документам rfc.
 *
 * Copyright (C) 2016 Naidolinsky Dmitry <naidv88@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY of FITNESS for A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * -------------------------------------------------------------------/
 */
#include "settings.h"

typedef struct {
	char *cdir;
	char *settings;
} Paths;
/* получить путь к файлу настроек */
static Paths * getpath()
{
	char *envhome = getenv("HOME");
	char settings_dir[255] = { 0 };
	{
		int total = strlen ( "/.rfcreader" ) + strlen ( envhome ) + 2;
		snprintf ( settings_dir, total -1, "%s/.rfcreader", envhome );
	}
	mkdir ( settings_dir, S_IRWXU );

	char *string = NULL;

	{
		int home = strlen ( envhome );
		int dir = strlen ( "/.rfcreader" );
		int cfile = strlen ( "/rfcreader" );
		int total = home + dir + cfile + 1;
		string = calloc ( total + 1, 1 );
		snprintf(string, total,"%s/.rfcreader/rfcreader",envhome);

		envhome = NULL;
	}


	/* копировать в структуру */
	Paths *p = calloc ( 1, sizeof ( Paths ) );
	{
		int sdir = strlen ( settings_dir );
		int ss = strlen ( string );
		p->cdir = calloc ( sdir + 1, 1 );
		p->settings = calloc ( ss + 1, 1 );
		strncpy ( p->cdir, settings_dir, sdir );
		strncpy ( p->settings, string, ss );
	}

	/* блок для создания файла */
	{
		/* если файла настроек нет */
		struct stat st;
		if (stat(string,&st) == -1){
			perror("config");

			/* если файла нет, создать файл */
			if (errno & ENOENT){
				fprintf(stderr,"file settings created");

				FILE *newconfig;

				if ( ( newconfig = fopen(string,"w")) == NULL){
					perror("fopen");
					exit(-1);
				}
				fprintf(stderr,"%s\n", "file created");

				if (newconfig != NULL){
					fprintf(newconfig,
							"# dir - path your rfc documents.\n"
							"# Example:\n"
							"# dir=~/rfc\n"
							"# dir=/home/user/rfc\n"
							"dir=\n"
							"\n"
							"# txt - view rfc document.\n"
							"# Example:\n"
							"# txt=less\n"
							"# txt=vi\n"
							"txt=\n"
							"\n"
							"# pdf - view rfc document.\n"
							"# Example:\n"
							"# pdf=\n"
							"# pdf=evince\n"
							"pdf=\n"
							"\n"
							"# reg - to consider the register.\n"
							"# if the option is 'on', then search of keywords is carried\n"
							"# irrespective of a letter case.\n"
							"# if the option is 'off', search works quicker.\n"
							"# Example:\n"
							"# reg=on\n"
							"# reg=off\n"
							"reg=on\n"
							"\n"
							"# colors.\n"
							"# available colors.\n"
							"# black, red, green, yellow, blue, magenta, cyan, white.\n"
							"# ------------------------------------------------------\n"
							"# foreground color.\n"
							"# Example:\n"
							"# fgcolor = black\n"
							"fgcolor = white\n"
							"\n"
							"# background color.\n"
							"bgcolor = black\n"
							"\n"
							"# selection foreground color.\n"
							"sfgcolor = yellow\n"
							"\n"
							"# selection background color.\n"
							"sbgcolor = black\n"

							);
					fclose(newconfig);
					fprintf(stderr,"need fill file settings -> %s\n", string);
					exit(-1);
				}
	
				fclose(newconfig);
			}
		}

	}
	free ( string );
	return p;
}
typedef struct {
	unsigned int dir:1 __attribute__ ((aligned(4)));
	unsigned int txt:1 __attribute__ ((aligned(4)));
	unsigned int pdf:1 __attribute__ ((aligned(4)));
	unsigned int reg:1 __attribute__ ((aligned(4)));
	unsigned int fg:1  __attribute__ ((aligned(4)));
	unsigned int bg:1  __attribute__ ((aligned(4)));
	unsigned int sfg:1 __attribute__ ((aligned(4)));
	unsigned int sbg:1 __attribute__ ((aligned(4)));
	
}conf;

struct configs * getconfig()
{
	Paths *p = getpath();
	cf = calloc(1,sizeof(struct configs));

	/* для обозначения обработанных данных */
	conf c;
	c.dir = 0;
	c.txt = 0;
	c.pdf = 0;
	c.reg = 0;

	cf->flm = calloc ( strlen ( p->cdir ) + 8, 1 );

	/* копировать значения */
	sprintf ( cf->flm, "%s/update", p->cdir );

	const char *path = p->settings;
	FILE * conf;
	char *ptr;
	char line[128];
	if ( ( conf = fopen(path,"r")) == NULL){
		perror("fopen");
		exit(-1);
	}
	while(fgets(line,127,conf)!=NULL){
		ptr = &line[0];

		if ( line[0] == '#' ) continue;
		if ( line[0] == 0xa ) continue;

		if (strncmp(line,"dir",3)==0){
			ptr += 3;
			if ( isalpha ( *ptr ) ) continue;
			if ( *ptr == 32 ) while ( *ptr == 32 ) ptr++;
			if ( *ptr == 61 ) ptr++;
			if ( *ptr == 32 ) while ( *ptr == 32 ) ptr++;

			/* блок для составляения строки cf->datadir */
			{
				/* узнать написано ли ~/, и добавить длину, если определено */
				char *envhome = NULL;
				if ( !strncmp ( ptr, "~/", 2 ) ) {
					envhome = getenv ( "HOME" );
					ptr += 2;
				}
				int home = envhome ? strlen ( envhome ) : 0;

				int datadir = strlen ( ptr ) - 1;
				length = home + datadir;
				/* for +/ */
				if ( envhome ) length++;

				cf->datadir = calloc(length + 1, 1);
				/* добавить строку, если определено */
				if ( envhome ) {
					char *ph = cf->datadir;
					strncpy ( ph, envhome, home );
					ph += home;
					*ph = '/'; ph++;
					strncpy( ph, ptr, datadir );
					envhome = NULL;
				}else
				strncpy(cf->datadir, ptr, length);
			}

			/* если не существует, создать */
			if ( access ( cf->datadir, F_OK ) ) {
				mkdir ( cf->datadir, S_IRWXU );
				fprintf ( stderr, "please run rfcreader -update for downoad rfc tar\n" );
			}

			/* 6 - /index */
			cf->index = calloc( length + 6 + 1 , 1 );
			snprintf(cf->index, length + 7,"%s/index",cf->datadir);

			/* запись dir обработана */
			c.dir = 1;
		}
		if (!strncmp(line,"txt",3)){
			ptr += 3;
			if ( isalpha ( *ptr ) ) continue;
			if ( *ptr == 32 ) while ( *ptr == 32 ) ptr++;
			if ( *ptr == 61 ) ptr++;
			if ( *ptr == 32 ) while ( *ptr == 32 ) ptr++;
			{ 
				int length = strlen(ptr) - 1;
				if ( length > 0 ) {
					cf->txtviewer = calloc(length + 1, 1);
					strncpy(cf->txtviewer, ptr, length);
				} else cf->txtviewer = NULL;
			}

			c.txt = 1;
		}
		if (!strncmp(line,"pdf",3)){
			ptr += 3;
			if ( isalpha ( *ptr ) ) continue;
			if (*ptr == 61) ptr++;
			
			else if (*ptr == 32){
				while(*ptr == 32)
					ptr++;
			}
			int length = strlen(ptr) - 1;
			if ( length == 0 )
				cf->pdfviewer = NULL;
			else{
				cf->pdfviewer = calloc(length + 1, 1);
				strncpy(cf->pdfviewer, ptr, length);
			}
			c.pdf = 1;
		}

		/* если опция поиска ключевых слов в независимости от регистра */
		if ( !strncmp ( line, "reg", 3 ) ) {
			ptr += 3;
			if ( isalpha ( *ptr ) ) continue;
			if ( *ptr == 32 ) while ( *ptr == 32 ) ptr++;
			if ( *ptr == 61 ) ptr++;
			if ( *ptr == 32 ) while ( *ptr == 32 ) ptr++;
				while ( *ptr == 32 ) ptr++;

			int length = strlen ( ptr ) - 1;
			if ( length == 0 ){
				cf->reg = 0;
				c.reg = 1;
			}
			else {
				*(ptr + length) = 0;
				if ( !strncmp ( ptr, "on\0", 3 ) ) {
					cf->reg = 1;
					c.reg = 1;
				}else
				if ( !strncmp ( ptr, "off\0", 4 ) ) {
					cf->reg = 0;
					c.reg = 1;
				}else {
					fprintf ( stderr, "error in option reg, please choose ( on, off,\\ )\n" );
					c.reg = 0;
				}
			}
		}
		if ( !strncmp ( line, "fgcolor", 7 ) ) {
			ptr += 7;
			if ( isalpha ( *ptr ) ) continue;
			if ( *ptr == 32 ) while ( *ptr == 32 ) ptr++;
			if ( *ptr == 61 ) ptr++;
			if ( *ptr == 32 ) while ( *ptr == 32 ) ptr++;

			int length = strlen ( ptr ) - 1;
			if ( !strncmp ( ptr, "black", 5 ) ) {
				if ( isalpha ( *(ptr+5) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->fgcolor = BLACK;
				c.fg = 1;
			}
			if ( !strncmp ( ptr, "red", 3 ) ) {
				if ( isalpha ( *(ptr+3) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->fgcolor = RED;
				c.fg = 1;
			}
			if ( !strncmp ( ptr, "green", 5 ) ) {
				if ( isalpha ( *(ptr+5) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->fgcolor = GREEN;
				c.fg = 1;
			}
			if ( !strncmp ( ptr, "yellow", 6 ) ) {
				if ( isalpha ( *(ptr+6) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->fgcolor = YELLOW;
				c.fg = 1;
			}
			if ( !strncmp ( ptr, "blue", 4 ) ) {
				if ( isalpha ( *(ptr+4) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->fgcolor = BLUE;
				c.fg = 1;
			}
			if ( !strncmp ( ptr, "magenta", 7 ) ) {
				if ( isalpha ( *(ptr+7) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->fgcolor = MAGENTA;
				c.fg = 1;
			}
			if ( !strncmp ( ptr, "cyan", 4 ) ) {
				if ( isalpha ( *(ptr+4) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->fgcolor = CYAN;
				c.fg = 1;
			}
			if ( !strncmp ( ptr, "white", 5 ) ) {
				if ( isalpha ( *(ptr+5) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->fgcolor = WHITE;
				c.fg = 1;
			}
		}
		if ( !strncmp ( line, "bgcolor", 7 ) ) {
			ptr += 7;
			if ( isalpha ( *ptr ) ) continue;
			if ( *ptr == 32 ) while ( *ptr == 32 ) ptr++;
			if ( *ptr == 61 ) ptr++;
			if ( *ptr == 32 ) while ( *ptr == 32 ) ptr++;

			int length = strlen ( ptr ) - 1;
			if ( !strncmp ( ptr, "black", 5 ) ) {
				if ( isalpha ( *(ptr+5) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->bgcolor = BLACK;
				c.bg = 1;
			}
			if ( !strncmp ( ptr, "red", 3 ) ) {
				if ( isalpha ( *(ptr+3) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->bgcolor = RED;
				c.bg = 1;
			}
			if ( !strncmp ( ptr, "green", 5 ) ) {
				if ( isalpha ( *(ptr+5) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->bgcolor = GREEN;
				c.bg = 1;
			}
			if ( !strncmp ( ptr, "yellow", 6 ) ) {
				if ( isalpha ( *(ptr+6) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->bgcolor = YELLOW;
				c.bg = 1;
			}
			if ( !strncmp ( ptr, "blue", 4 ) ) {
				if ( isalpha ( *(ptr+4) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->bgcolor = BLUE;
				c.bg = 1;
			}
			if ( !strncmp ( ptr, "magenta", 7 ) ) {
				if ( isalpha ( *(ptr+7) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->bgcolor = MAGENTA;
				c.bg = 1;
			}
			if ( !strncmp ( ptr, "cyan", 4 ) ) {
				if ( isalpha ( *(ptr+4) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->bgcolor = CYAN;
				c.bg = 1;
			}
			if ( !strncmp ( ptr, "white", 5 ) ) {
				if ( isalpha ( *(ptr+5) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->bgcolor = WHITE;
				c.bg = 1;
			}
		}
		if ( !strncmp ( line, "sfgcolor", 8 ) ) {
			ptr += 8;
			if ( isalpha ( *ptr ) ) continue;
			if ( *ptr == 32 ) while ( *ptr == 32 ) ptr++;
			if ( *ptr == 61 ) ptr++;
			if ( *ptr == 32 ) while ( *ptr == 32 ) ptr++;

			int length = strlen ( ptr ) - 1;
			if ( !strncmp ( ptr, "black", 5 ) ) {
				if ( isalpha ( *(ptr+5) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->sfgcolor = BLACK;
				c.sfg = 1;
			}
			if ( !strncmp ( ptr, "red", 3 ) ) {
				if ( isalpha ( *(ptr+3) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->sfgcolor = RED;
				c.sfg = 1;
			}
			if ( !strncmp ( ptr, "green", 5 ) ) {
				if ( isalpha ( *(ptr+5) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->sfgcolor = GREEN;
				c.sfg = 1;
			}
			if ( !strncmp ( ptr, "yellow", 6 ) ) {
				if ( isalpha ( *(ptr+6) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->sfgcolor = YELLOW;
				c.sfg = 1;
			}
			if ( !strncmp ( ptr, "blue", 4 ) ) {
				if ( isalpha ( *(ptr+4) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->sfgcolor = BLUE;
				c.sfg = 1;
			}
			if ( !strncmp ( ptr, "magenta", 7 ) ) {
				if ( isalpha ( *(ptr+7) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->sfgcolor = MAGENTA;
				c.sfg = 1;
			}
			if ( !strncmp ( ptr, "cyan", 4 ) ) {
				if ( isalpha ( *(ptr+4) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->sfgcolor = CYAN;
				c.sfg = 1;
			}
			if ( !strncmp ( ptr, "white", 5 ) ) {
				if ( isalpha ( *(ptr+5) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->sfgcolor = WHITE;
				c.sfg = 1;
			}
		}
		if ( !strncmp ( line, "sbgcolor", 8 ) ) {
			ptr += 8;
			if ( isalpha ( *ptr ) ) continue;
			if ( *ptr == 32 ) while ( *ptr == 32 ) ptr++;
			if ( *ptr == 61 ) ptr++;
			if ( *ptr == 32 ) while ( *ptr == 32 ) ptr++;

			int length = strlen ( ptr ) - 1;
			if ( !strncmp ( ptr, "black", 5 ) ) {
				if ( isalpha ( *(ptr+5) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->sbgcolor = BLACK;
				c.sbg = 1;
			}
			if ( !strncmp ( ptr, "red", 3 ) ) {
				if ( isalpha ( *(ptr+3) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->sbgcolor = RED;
				c.sbg = 1;
			}
			if ( !strncmp ( ptr, "green", 5 ) ) {
				if ( isalpha ( *(ptr+5) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->sbgcolor = GREEN;
				c.sbg = 1;
			}
			if ( !strncmp ( ptr, "yellow", 6 ) ) {
				if ( isalpha ( *(ptr+6) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->sbgcolor = YELLOW;
				c.sbg = 1;
			}
			if ( !strncmp ( ptr, "blue", 4 ) ) {
				if ( isalpha ( *(ptr+4) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->sbgcolor = BLUE;
				c.sbg = 1;
			}
			if ( !strncmp ( ptr, "magenta", 7 ) ) {
				if ( isalpha ( *(ptr+7) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->sbgcolor = MAGENTA;
				c.sbg = 1;
			}
			if ( !strncmp ( ptr, "cyan", 4 ) ) {
				if ( isalpha ( *(ptr+4) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->sbgcolor = CYAN;
				c.sbg = 1;
			}
			if ( !strncmp ( ptr, "white", 5 ) ) {
				if ( isalpha ( *(ptr+5) ) ){
					fprintf ( stderr, "error in color, maybe cyan\n" );
				}
				cf->sbgcolor = WHITE;
				c.sbg = 1;
			}
		}
	}
	fclose(conf);

	/* проверка значений и вывод ошибок */
	if ( !c.dir ) {
		fprintf ( stderr, "please fill dir parameter in %s.\n", path );
		exit ( EXIT_FAILURE );
	}
	/* установить цвета по умолчанию, если не определены */
	if ( !c.fg ) cf->fgcolor = WHITE;
	if ( !c.bg ) cf->bgcolor = BLACK;
	if ( !c.sfg ) cf->sfgcolor = BLACK;
	if ( !c.sbg ) cf->sbgcolor = WHITE;

	path = NULL;
	free ( p->cdir );
	free ( p->settings );
	free (p);
	return cf;
}
