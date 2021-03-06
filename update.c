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

	int sock;
	if ( ( sock = socket ( AF_INET, SOCK_STREAM, 0 ) ) == -1 ) {
		perror ( "socket" );
		exit ( EXIT_FAILURE );
	}

	/* настройки для соединения */ 
	struct hostent *ht = gethostbyname ( "www.rfc-editor.org" );
	struct sockaddr_in dst;
	dst.sin_family = AF_INET;
	dst.sin_port = htons (443);
	dst.sin_addr = *( ( struct in_addr *) ht->h_addr_list[0] );
	if ( connect ( sock, ( struct sockaddr *)&dst, sizeof ( dst ) ) == -1 ){
		perror ( "connect" );
		exit ( EXIT_FAILURE );
	}

	const SSL_METHOD *method;
	SSL_CTX *ctx;
	SSL_SESSION *session;
	SSL *ssl;
	X509 *x509;

	SSL_library_init ( );


	char buffer [ 4096 ] = {0};
	sprintf ( buffer, 
			"GET /in-notes/tar/RFC-all.tar.gz HTTP/1.1\r\n"
			"Host: www.rfc-editor.org\r\n"
			"User-Agent: rfcreader/0.5\r\n"
			"Accept: */*\r\n"
			"\r\n"
		 	);

	method = SSLv23_client_method();
	ctx = SSL_CTX_new ( method );
	ssl = SSL_new ( ctx );
	SSL_set_fd ( ssl, sock );
	SSL_connect ( ssl );

	int ret;

	ret = SSL_write ( ssl, buffer, strlen ( buffer ) );
	/* блок обработки ошибки */
	{
		if ( ret < 0 ) {
			fprintf ( stderr, "ssl read error code %d\n", SSL_get_error ( ssl, ret ) );
			SSL_shutdown ( ssl );
			SSL_free ( ssl );
			SSL_CTX_free ( ctx );
			shutdown ( sock, SHUT_RDWR );
			close ( sock );
			exit ( EXIT_FAILURE );
		}
		if ( ret == 0 ) {
			fprintf ( stderr, "error\n" );
			SSL_shutdown ( ssl );
			SSL_free ( ssl );
			SSL_CTX_free ( ctx );
			shutdown ( sock, SHUT_RDWR );
			close ( sock );
			exit ( EXIT_FAILURE );
		}
	}

	ret = SSL_read ( ssl, buffer, 4096 );
	/* блок обработки ошибки */
	{
		if ( ret < 0 ) {
			fprintf ( stderr, "ssl read error code %d\n", SSL_get_error ( ssl, ret ) );
			SSL_shutdown ( ssl );
			SSL_free ( ssl );
			SSL_CTX_free ( ctx );
			shutdown ( sock, SHUT_RDWR );
			close ( sock );
			exit ( EXIT_FAILURE );
		}
		if ( ret == 0 ) {
			fprintf ( stderr, "error\n" );
			SSL_shutdown ( ssl );
			SSL_free ( ssl );
			SSL_CTX_free ( ctx );
			shutdown ( sock, SHUT_RDWR );
			close ( sock );
			exit ( EXIT_FAILURE );
		}
	}

	/* если запрос неправильный */
	if ( strstr ( buffer, "400 BHTTP/1.1 400 Bad Request" ) ) {
		fprintf ( stderr, "error, bad request\n" );
		SSL_shutdown ( ssl );
		SSL_free ( ssl );
		SSL_CTX_free ( ctx );
		shutdown ( sock, SHUT_RDWR );
		close ( sock );
		exit ( EXIT_FAILURE );
	}

	/* получить строку последней модификации */
	char *llm;
	char *lm;
	lm = strstr ( buffer, "Last-Modified:" );
	if ( lm != NULL ) {
		/* вычислить длину строки и записать в буфер */
		fprintf ( stderr, "getting last modified.\n" );
		char *lend;
		lm += 15;
		lend = lm;
		int len = 0;
		while (  *lend != 0xa && *lend != 0xd ) {
			lend++;
			len++;
		}
		llm = calloc ( len + 1, 1 );
		strncpy ( llm, lm, len );

		/* если файл существуeт, сравнить даты */
		if ( get_lm ( ) == 0 ) {
			if ( !strncmp ( llm, cf->lm, strlen ( llm ) ) ) {
				fprintf ( stderr, "documents correspond to the latest version.\n" );
				exit ( EXIT_SUCCESS );
			}
		}
	}

	/* получить размер файла */
	unsigned int total_size = 0;
	char size[20] = { 0 };
	char *pt;
	pt = strstr ( buffer, "Content-Length:" );
	if ( pt ) {
		pt += 16;
		total_size = atoi ( pt );
	}

	/* блок для сохранения файла */
	{
		fprintf ( stderr, "downloading...\n" );
		/* открыть файл для записи */
		const char name[] = "/tmp/RFC-all.tar.gz";
		FILE *out = fopen ( name, "wb" );

		/* максимальный приём файла 16384 */
		const unsigned int max_recv_size = 16384;
		char file [ max_recv_size + 1 ];
		int get = 0;

		int cur_time = 0;
		/* записать в файл */
		unsigned long percent = 0;
		printf ( "total downloaded: %4ld%%\r", percent );
		do 
		{
			get = SSL_read ( ssl, file, max_recv_size );
			if ( get <= 0 ) break;
			percent += get;
			long result = percent * 100.0 / total_size;
			printf ( "total downloaded: %4ld%%", result );
			fflush ( stdout );
			printf ("\r");
			fwrite ( file, 1, get, out );
		}
		while ( percent < total_size );
		printf ( "\n" );

		fclose ( out );
	}


	SSL_shutdown ( ssl );
	SSL_free ( ssl );
	SSL_CTX_free ( ctx );
	shutdown ( sock, SHUT_RDWR );
	close ( sock );

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

	/* сохранить дату */
	{
		fprintf ( stderr, "saving date in file.\n" );
		FILE *out_lm = fopen ( cf->flm, "w" );
		fwrite ( llm, 1, strlen ( llm ), out_lm );
		fclose ( out_lm );
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
