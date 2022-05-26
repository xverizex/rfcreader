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

extern struct configs *cf;
extern int length;

int rebuild ( )
{
	char datadir[255];
	sprintf ( datadir, "%s/rfc-index.txt", cf->datadir );
	FILE *file = fopen ( datadir, "r" );

	/* читать строки, пока не найдётся второй раз строка RFC INDEX */
	char line[255];
	for ( int index = 0; index <= 1; ) {
		fgets ( line, 254, file );
		if ( strstr ( line, "RFC INDEX" ) ) {
			index++;
		}
	}

	/* считывать остальные строки */
	FILE *out;
	char dt[255] = { 0 };
	sprintf ( dt, "%s/index", cf->datadir );
	out = fopen ( dt, "w" );
	char rfcdoc[1024];
	char *space;
	while ( fgets ( line, 254, file ) != NULL ) {
		if ( isalnum ( line[0] ) ) {
			memset ( &rfcdoc, 0, 1024 );
			strncpy ( rfcdoc, line, strlen ( line ) - 1 );
			while ( line[0] != 10 ) {
				space = fgets ( line, 254, file );
				if ( space == NULL ) break;
				while ( *space == 0x20 ) space++;
				rfcdoc [ strlen ( rfcdoc ) ] = 32;
				strncat ( rfcdoc, space, strlen ( space ) - 1);
			}
			/* вывести строку в новый файл */
			fprintf ( out, "%s\n", rfcdoc );
		}
	}
	fclose ( out );
	fclose ( file );

}

