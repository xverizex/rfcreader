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
#include <curl/curl.h>

extern struct configs *cf;
extern int length;

/* создать меню из файла */
extern int rebuild ( );

/* получить строку последней модификации */
static int get_lm ( )
{
	{
		char *ret;
		/* существует ли файл */
		if ( !access ( cf->flm, F_OK ) ){
			char date[64];
			FILE *in = fopen ( cf->flm, "r" );
			ret = fgets ( date, 63, in );

			if ( ret == NULL ) return -1;

			int len = strlen ( date );
			cf->lm = calloc ( len + 1, 1 );
			strncpy ( cf->lm, date, len );
			fclose ( in );
			return 0;
		}
	}
	return -1;
}

int update ( )
{
	fprintf ( stderr, "Updating...\n" );

	/* если каталог на написан, то выйти */
	if ( !strlen ( cf->datadir ) ){
		fprintf ( stderr, "please fill dir in file settings.\n" );
		exit ( EXIT_FAILURE );
	}

	CURL *handle = curl_easy_init ();
	struct curl_slist *slist = NULL;

	slist = curl_slist_append (slist, "User-Agent: rfcreader/0.6");

	curl_easy_setopt (handle, CURLOPT_HTTPHEADER, slist);

	curl_easy_setopt (handle, CURLOPT_URL, "https://www.rfc-editor.org/in-notes/tar/RFC-all.tar.gz");

	FILE *fp = fopen ("/tmp/RFC-all.tar.gz", "w");

	curl_easy_setopt (handle, CURLOPT_WRITEFUNCTION, NULL);
	curl_easy_setopt (handle, CURLOPT_WRITEDATA, fp);

	curl_easy_perform (handle);
	
	fclose (fp);

	/* удалить документы rfc */
	{
		fprintf ( stderr, "deleting rfc documents.\n" );
		struct dirent *dr;
		char delete[512];
		DIR *dir_rfc = opendir ( cf->datadir );
		while ( ( dr = readdir ( dir_rfc ) ) != NULL ) {
			snprintf ( delete, 512, "%s/%s", cf->datadir, dr->d_name );
			unlink ( delete );
		}

		closedir ( dir_rfc );
	}

	/* распаковать новые документы */
	{
		fprintf ( stderr, "unpack rfc archive.\n" );
		int ret;
		char extract[255];
		sprintf ( extract, "tar zxf /tmp/RFC-all.tar.gz -C %s\n", cf->datadir );
		if ( system ( extract ) == -1 ) {
			perror ( "system" );
		}
		unlink ( "/tmp/RFC-all.tar.gz" );
	}

	/* перенастроить меню */
	{
		fprintf ( stderr, "rebuild menu\n" );
		rebuild ( );
	}

	free ( cf->lm );
	free ( cf->flm );
	free ( cf->cdir );
	free ( cf->datadir );
	free ( cf->txtviewer );
	free ( cf->pdfviewer );
	free ( cf );

	exit ( EXIT_SUCCESS );
}
