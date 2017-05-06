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
		int total = strlen ( "/.rfcreader" ) + strlen ( envhome ) ;
		snprintf ( settings_dir, total, "%s/.rfcreader", envhome );
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
	unsigned int dir:1;
	unsigned int txt:1;
	unsigned int pdf:1;
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
			if (*ptr == '='){
				ptr++;
			}
			else if (*ptr == 32){
				while(*ptr == 32)
					ptr++;
			}

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
		if (strncmp(line,"txt",3)==0){
			ptr += 3;
			if ( isalpha ( *ptr ) ) continue;
			if (*ptr == 61){
				ptr++;
			}
			else if (*ptr == 32){
				while(*ptr == 32)
					ptr++;
			}
			{ 
				int length = strlen(ptr) - 1;
				cf->txtviewer = calloc(length + 1, 1);
				strncpy(cf->txtviewer, ptr, length);
			}

			c.txt = 1;
		}
		if (!strncmp(line,"pdf",3)){
			ptr += 3;
			if ( isalpha ( *ptr ) ) continue;
			if (*ptr == 61){
				ptr++;
			}
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
	}
	fclose(conf);

	/* проверка значений и вывод ошибок */
	if ( !c.dir ) {
		fprintf ( stderr, "please fill dir parameter in %s.\n", path );
		exit ( EXIT_FAILURE );
	}
	path = NULL;
	free ( p->cdir );
	free ( p->settings );
	free (p);
	return cf;
}
