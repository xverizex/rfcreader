#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main ( int argc, char *argv[] ) 
{
	if ( argc < 1 ) exit ( EXIT_FAILURE );
	FILE *file = fopen ( argv[1], "r" );

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
	out = fopen ( "index", "w" );
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
				rfcdoc [ strlen ( rfcdoc ) - 1] = 0;
				strncat ( rfcdoc, space, strlen ( space ) - 1 );
			}
			/* вывести строку в новый файл */
			fprintf ( out, "%s\n", rfcdoc );
		}
	}
	fclose ( out );
	fclose ( file );

}

