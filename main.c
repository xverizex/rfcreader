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
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define TXT 1
#define PDF 2
#ifndef COMMAND_LINE_SIZE
#define COMMAND_LINE_SIZE 128
#endif
#define ESC 27
#define DEL 330
#define NEWLINE 10
#define CARRIAGE 13
#define TERMBACKSPACE 263
#define XBACKSPACE 127
#define KEY_F11 410

struct menu {
	int top;
	int width;
	int height;
	int max;
	int cur;
	char menu[9000][255];
};
struct menu *menu_ptr;

struct winsize ws; /* структура для хранения размера окна терминала */
char *dir;
const char *cur_item; /* указатель на текущую ячейку */
int row;

/* для curses */
ITEM **my_items;
MENU *my_menu;
WINDOW *my_iteml;
WINDOW *botton;
WINDOW *notice;

char ss[64];
/* определяет, читается документ или нет */
enum docs { VIEW, MENU_CURSES } read_doc;

/* функция выбора формата просмотра */
static void select_format ( int number )
{
	ITEM **my_format;
	MENU *smenu;
	WINDOW *formats;
	char *choice[] = { "txt", "pdf", 0 };
	my_format = (ITEM **)calloc(3, sizeof(ITEM *));

	my_format[0] = new_item(choice[0], NULL);
	my_format[1] = new_item(choice[1], NULL);
	my_format[2] = new_item(choice[2], NULL);

	smenu = new_menu((ITEM **)my_format);
	formats = newwin(3,10, ws.ws_row / 2, ws.ws_col / 2);
	set_menu_win(smenu,formats);
	keypad(formats,TRUE);
	keypad(my_iteml,FALSE);
	set_menu_mark(smenu," * ");
	post_menu(smenu);
	wrefresh(formats);
	int c;
	char *document;
	int viewer = 0;
	int srow = 0;
	int ret;
	while ( ( c = wgetch ( formats ) ) != ESC ) {
		switch ( c ) {
			/* выбор формата */
			case CARRIAGE:
			case NEWLINE:
				cur_item = item_name(current_item(smenu));
				if ( !strncmp ( cur_item, "txt", 3 ) ) srow = 0;
				if ( !strncmp ( cur_item, "pdf", 3 ) ) srow = 1;

				document = calloc(180,1);

				memset ( document, 0, COMMAND_LINE_SIZE );

				snprintf( document, COMMAND_LINE_SIZE,
					"%s %s/rfc%d.%s",
					srow == 0 ? cf->txtviewer : cf->pdfviewer  ,
					cf->datadir ,
					number,
					cur_item
					);
					read_doc = VIEW;
				if (system(document) == 512){
					mvwprintw( 
						notice, 
						0, 
						0, 
						"error in choices item");
					free(document);
					read_doc = MENU_CURSES;
					break;
				}
				read_doc = MENU_CURSES;
				if ( document ) free(document);
				keypad(formats,FALSE);
				keypad(my_iteml,TRUE);
				return;
				break;
				/* перемещение по ячейкам */
			case KEY_UP: 
				ret = menu_driver(smenu, REQ_UP_ITEM);
				if ( ret == E_REQUEST_DENIED ) break;
				break;
			case KEY_DOWN:
				ret = menu_driver(smenu, REQ_DOWN_ITEM);
				if ( ret == E_REQUEST_DENIED ) break;
				break;
		}
	}
	return;
}
void menu_show ( struct menu *, char *ss );
/* функция смены размера окна терминала */
static void switch_window ( int signal )
{
	if ( read_doc == VIEW ) return;

	clear ();
	mvprintw ( 0, 0, ">" );
	mvprintw ( 0, 1, ss );
	
	if ( menu_ptr )
	{
		ioctl(0,TIOCGWINSZ,&ws);
		menu_ptr->width = ws.ws_row;
		menu_ptr->width -= 5;
	}

	if ( menu_ptr ) menu_show ( menu_ptr, NULL ); 

//	keypad(stdscr,TRUE);
					
}
#define ECORRECT "\033[5;32mcheck correctness of settings and access rights\033[0m\n"

/* получить настройки */
extern struct configs *getconfig();

/* обновить */
extern int update ( void );

void add_item ( struct menu *m, char *t )
{
	int length = strlen ( t );
	strncpy ( m->menu [ m->max ], t, length );
	m->max++;
}

void menu_show ( struct menu *m, char *ss )
{
	if ( ioctl ( 0, TIOCGWINSZ, &ws ) < 0 )
		perror ( "winsize get ioctl" );

	for ( int i = 0; i < m->width; i++ ) {
		length = strlen ( m->menu[i + m->top] );
		if ( m->cur == i ) 
		{
			attroff ( COLOR_PAIR (2) );
			attron ( COLOR_PAIR (1) | A_BOLD );
		}
		else 
		{
			attroff ( COLOR_PAIR (1) | A_BOLD );
			attron ( COLOR_PAIR (2) );
		}

#if 1
		for ( int index = 0; index < ws.ws_col && index < length ; index++ ) {
			mvaddch ( i + 1, index, m->menu[i + m->top][index] );
		}
#endif


		if ( i == m->max - 1 ) break;
	}
}

int main(int argc, char *argv[])
{


	signal ( SIGWINCH, switch_window );
	read_doc = MENU_CURSES;
	struct configs * cf = getconfig();

	/* если нужно обновить */
	if ( argc >= 2 )
		if ( !strncmp ( argv[1], "-update\0", 8 ) ) {
			update ( );
			exit ( EXIT_SUCCESS );
		}

	FILE *rfd;
	if ( ( rfd = fopen(cf->index,"r"))==NULL){
		fprintf(stderr, ECORRECT );
		perror("fopen");
		return 0;
	}
	free ( cf->index );
	char line[285];
	int lens = 0 ;
	int si = 0;
	int f = 0;
	char *ptr;
	char *ptline;

	char list[9000][285];

	/* эти строки переводятся в нижний регистр,
		 такое нужно, чтобы найти искомые подстроки не зависимо от регистра */
	char query[64];
	char query_line[1024];

	struct menu m = { 0, 0, 0, 0 };

	/* получить размеры экрана */
	ioctl(0,TIOCGWINSZ,&ws);
	m.width = ws.ws_row;
	m.width -= 5;

	menu_ptr = &m;

	/* составление меню упрощено из-за rebuild */
	while( fgets(line,284,rfd) != NULL){

		int length = strlen ( line );
		/* создать список */
		strncpy ( list[si], line, length );
		si++;
		memset ( line, 0, 285 );
	}

	fclose(rfd);

	setlocale(LC_ALL, "");
	initscr();
	cbreak();
	noecho();
	clear();
	nonl();
	int c;
	start_color ();
	init_pair ( 1, cf->sfgcolor, cf->sbgcolor );
	init_pair ( 2, cf->fgcolor, cf->bgcolor );

	struct termios term;
	tcgetattr(1, &term);
	term.c_lflag &= ~(ICANON|ECHO);
	term.c_cc[VMIN] = 1;
	term.c_cc[VTIME] = 0;
	tcsetattr(0,TCSANOW,&term);

	ioctl(0,TIOCGWINSZ,&ws);

	int a = 0;
	int len = 0;
	char *document = NULL;
	char *rfc = NULL;
	int number = 0;
	mvprintw ( 0, 0, ">" );

	int i = 0;
#if 1
	/* заполнить меню списком */
	for ( int i = 0; i < si; i++ ) {
		add_item ( &m, &list[i][0] );
	}
#endif
	int page = 0;

	menu_show (  &m, NULL );
	mvprintw ( m.width + 2, 0, m.menu[m.cur + m.top] );
	int x, y = 1;

	keypad(stdscr,TRUE);
	int ret = 0;
	while ( ( c = getch()) != ESC){
		static int pos = 0;
		switch(c){
			case CARRIAGE:
			case NEWLINE:
				document = calloc(180,1);
				rfc = calloc(10,1);
				int viewer = 0;
				sscanf(m.menu[m.cur + m.top], "%s ", rfc);
				number = atoi(rfc);
				free ( rfc );

				if (strstr(m.menu[m.cur]," PDF="))
					viewer += PDF;
				if(strstr(m.menu[m.cur]," TXT="))
					viewer += TXT;

				if ( viewer == 3 ) {
					select_format ( number );
				} else {
					memset ( document, 0, COMMAND_LINE_SIZE );

					snprintf( document, COMMAND_LINE_SIZE,
						"%s %s/rfc%d.%s",
						viewer==TXT ? cf->txtviewer : cf->pdfviewer  ,
						cf->datadir ,
						number,
						viewer==TXT ? "txt" : "pdf"
						);
					read_doc = VIEW;
					if ( viewer == 0 ) {
						mvwprintw( 
							notice, 
							0, 
							0, 
							"error in choices item");
						free(document);
						read_doc = MENU_CURSES;
						break;
					}
					if (system(document) == 512){
						mvwprintw( 
							notice, 
							0, 
							0, 
							"error in choices item");
						free(document);
						read_doc = MENU_CURSES;
						break;
					}
				}
				read_doc = MENU_CURSES;
				if ( document )
				free(document);

				clear();
				mvprintw ( 0, 0, ">" );
				mvprintw ( 0, 1, ss );
				menu_show(&m, &ss[0]);
				mvprintw ( m.width + 2, 0, m.menu[m.cur] );
				break;
			case KEY_UP:

				if ( m.cur > 10 && m.cur <= ws.ws_row - 5 ) 
				{
					m.cur--;
				}
				else if ( m.cur >= 10 && m.top > 0 )
				{
					m.top--;
					if ( m.top < 0 ) m.top = 0;
				}
				else if ( y <= 11 ) 
				{
					m.cur--;
				}
				else 
				{
					m.cur--;
				}

				y = m.cur; 
				if ( y < 0 ) y = m.cur = 0;

				clear ();
				mvprintw ( 0, 0, ">" );
				mvprintw ( 0, 1, ss );
				menu_show( &m, &ss[0] );
				mvprintw ( m.width + 2, 0, m.menu[m.cur + m.top] );
				break;
				
			case KEY_DOWN:
				
				if ( m.cur >= 10 && m.top < ( m.max - ( ws.ws_row - 5 ) ) ) 
				{
					m.top++;
				}
				else if ( y >= m.max - 10 ) 
				{
					m.cur++;
				}
				else 
				{
					m.cur++;
				}

				y = m.cur;
				if ( y > ws.ws_row - 6  ) y = m.cur = ws.ws_row - 6; 

				clear();
				mvprintw ( 0, 0, ">" );
				mvprintw ( 0, 1, ss );
				menu_show( &m, &ss[0] );
				mvprintw ( m.width + 2, 0, m.menu[m.cur + m.top] );
				break;
			case KEY_NPAGE: 
        break;
			case KEY_PPAGE:
        break;
			case TERMBACKSPACE:
			case XBACKSPACE:
				len = strlen(ss);
				if ( len > 0 ){
					ss[len-1]='\0';
					pos = len - 1;
					if ( pos < 0 ){
						pos = 0;
						break;
					}
					goto  print;
				}
				goto print;
				
				break;
			default:
					if (c == KEY_F11) 
						break;
					
					if ( c == DEL )
						break;

					ss[pos]=c;
					pos++;
print:
					for ( int i = 0; i < 9000; i++ ) memset ( m.menu[i], 0, 255 );
					i = 0;
					a = 0;

					/* перевести запроса  в нижний регистр если включена опция */
						memset ( query, 0, 64 );
						strncpy ( query, ss, 63 );
					if ( cf->reg ) {
						char *ptr = &query[0];
						for ( int i = 0; i <= 63; i++, ptr++ ) 
							if ( *ptr >= 0x41 && *ptr <= 0x5a )
								*ptr += 0x20;
					}

						m.max = 0;

					for (i = 0; i <= si - 1; i++){
						/* перевести строку меню в нижний регистр */
							memset ( query_line, 0, 1024 );
							strncpy ( query_line, list[i], strlen ( list[i] ) );
							int len = strlen ( list[i] );
						if ( cf->reg ) {
							ptr = &query_line[0];
							for ( int i = 0; i <= len; i++, ptr++ ) 
								if ( *ptr >= 0x41 && *ptr <= 0x5a )
									*ptr += 0x20;
						}
						


						if (pos == 0){
							add_item ( &m, &list[i][0] );
							a = i + 1;
						}
						else if ( strstr(query_line, query ) ) {
							add_item ( &m, &list[i][0] );
							a++;

						}
					}
					clear ();
					mvprintw ( 0, 0, ">" );
					mvprintw ( 0, 1, ss );
					m.cur = m.top = 0;
					menu_show ( &m, &ss[0] );
					mvprintw ( m.width + 2, 0, m.menu[m.cur] );
		}
		keypad ( stdscr, TRUE );
	}


	/* завершение, освобождение участков памяти */
	free ( cf->lm );
	free ( cf->flm );
	free ( cf->cdir );
	free ( cf->datadir );
	free ( cf->txtviewer );
	free ( cf->pdfviewer );
	free ( cf );
	

	unpost_menu(my_menu);
	free_menu(my_menu);
	endwin();
	exit(0);
}
