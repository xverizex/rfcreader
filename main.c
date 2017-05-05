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
#define to_transfer_to_other_line for
#define TXT 1
#define PDF 2
#define SUBWIN 2
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


struct winsize ws; /* структура для хранения размера окна терминала */
char **choices = NULL; /* здесь храняться строки заголовки документов */
char *dir;
const char *cur_item; /* указатель на текущую ячейку */
int row;

/* для curses */
ITEM **my_items;
MENU *my_menu;
WINDOW *my_iteml;
WINDOW *botton;
WINDOW *notice;

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

/* функция смены размера окна терминала */
static void switch_window ( int signal )
{
	if ( read_doc == VIEW ) return;

	if ( ioctl ( 0, TIOCGWINSZ, &ws ) < 0 )
		perror ( "winsize get ioctl" );

	cur_item = item_name(current_item(my_menu));
	row = item_index(current_item(my_menu));
	unpost_menu(my_menu);	
	free_menu(my_menu);
	my_menu = new_menu((ITEM **)my_items);
	my_iteml = derwin(botton, 16, ws.ws_col - 2, SUBWIN, 1);	
	notice = derwin(botton, ws.ws_row - 19 , ws.ws_col - 2, 19, 1);
	wclear ( notice );
	keypad(my_iteml,TRUE);
	set_menu_win(my_menu,my_iteml);
					
	set_menu_mark(my_menu," * ");
	post_menu(my_menu);
	set_top_row(my_menu,row);
	wrefresh(botton);
	wrefresh(my_iteml);
	dir = (char *)&cur_item[0];
	if ( dir ){
		mvwprintw(notice,0,0,"%s",dir);
		length=strlen(dir);
		to_transfer_to_other_line(int line=0;length > ws.ws_col;line++){
			dir += length;
			mvwprintw( notice,line ,0,"%s",dir);
			length=strlen(dir);
		}
	}
	wrefresh(notice);
}
#define ECORRECT "\033[5;32mcheck correctness of settings and access rights\033[0m\n"

/* получить настройки */
extern struct configs *getconfig();

/* обновить */
extern int update ( void );

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
	char *line = calloc(1024,sizeof(char));
	int lens = 0 ;
	choices = (char **) calloc(65535, sizeof(char));
	int si = 0;
	int f = 0;
	char *ptr;
	char *ptline;

	/* эти строки переводятся в нижний регистр,
		 такое нужно, чтобы найти искомые подстроки не зависимо от регистра */
	char *query = calloc ( 64, 1 );/* строка запроса */
	char *query_line = calloc ( 1024, 1 ); /* cтрока меню */

	/* составление меню упрощено из-за rebuild */
	while( fgets(line,1023,rfd)!=NULL){
		choices[si] = (char *) calloc ( strlen ( line ), 1 );
		strncpy ( choices[si], line, strlen ( line ) - 1 );
		si++;

		memset ( line, 0, 1023 );

	}
	fclose(rfd);
	si++;
	choices[si] = (char *)NULL;
	

	setlocale(LC_ALL, "");
	initscr();
	cbreak();
	noecho();
	nonl();
	int c;

	struct termios term;
	tcgetattr(1, &term);
	term.c_lflag &= ~(ICANON|ECHO);
	term.c_cc[VMIN] = 1;
	term.c_cc[VTIME] = 0;
	tcsetattr(0,TCSANOW,&term);

	ioctl(0,TIOCGWINSZ,&ws);

	char * ss = calloc(64,1);
	int a = 0;
	int len = 0;
	char *document = NULL;
	char *rfc = NULL;
	int number = 0;

	
	my_items = (ITEM **)calloc(si, sizeof(ITEM *));
	int i = 0;

	for(; i <= si; i++){
		if(!choices[i]){
			si = i;
		}
		my_items[i] = new_item(choices[i], NULL);
	}

	my_menu = new_menu((ITEM **)my_items);
	botton = newwin(0,0,0,0);
	my_iteml = derwin(botton, 16, ws.ws_col - 2, SUBWIN, 1);
	notice = derwin(botton, ws.ws_row - 19 , ws.ws_col - 2, 19, 1);
	set_menu_win(my_menu,my_iteml);
	keypad(my_iteml,TRUE);
	set_menu_mark(my_menu," * ");
	post_menu(my_menu);
	mvwaddstr(botton,0,0,">");
	cur_item = item_name(current_item(my_menu));
		dir = (char *)&cur_item[0];
		mvwprintw(notice,0,0,"%s",dir);
		int length=strlen(dir);
		to_transfer_to_other_line(int line=0;length > ws.ws_col;line++){
			dir += length;
			mvwprintw( notice,line ,0,"%s",dir);
			length=strlen(dir);
		}
	wrefresh(botton);
	wrefresh(my_iteml);
	wrefresh(notice);
	int ret = 0;
	while ( ( c = wgetch(my_iteml)) != ESC){
		static int pos = 0;
		switch(c){
			case CARRIAGE:
			case NEWLINE:
				row = item_index(current_item(my_menu));
				document = calloc(180,1);
				rfc = calloc(10,1);
				int viewer = 0;
				sscanf(cur_item, "%s ", rfc);
				number = atoi(rfc);

				if (strstr(cur_item," PDF="))
					viewer += PDF;
				if(strstr(cur_item," TXT="))
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
					if (system(document) == 512){
						mvwprintw( 
							notice, 
							0, 
							0, 
							"error in choices item");
						free(document);
						wrefresh(notice);
						read_doc = MENU_CURSES;
						break;
					}
				}
				read_doc = MENU_CURSES;
				if ( document )
				free(document);
#if 0
				wclear(botton);
				wclear(my_iteml);
#endif
				cur_item = item_name(current_item(my_menu));
				dir = (char *)&cur_item[0];
					mvwprintw(notice,0 ,0,"%s",dir);
					length=strlen(dir);
					to_transfer_to_other_line(int line=0;length > ws.ws_col;line++){
						dir += length;
						mvwprintw( notice, line ,0,"%s",dir);
						length=strlen(dir);
					}
				mvwaddstr(botton,0,0,">");
				wrefresh(botton);
				wrefresh(my_iteml);
				wrefresh(notice);
				goto print;
				break;
				
			case KEY_DOWN:
				wclear(notice);
				ret = menu_driver(my_menu, REQ_DOWN_ITEM);
				cur_item = item_name(current_item(my_menu));
				dir = (char *)&cur_item[0];
					mvwprintw(notice,0 ,0,"%s",dir);
					length=strlen(dir);
					to_transfer_to_other_line(int line=0;length > ws.ws_col;line++){
						dir += length;
						mvwprintw( notice, 0 ,0,"%s",dir);
						length=strlen(dir);
					}
				mvwaddstr(botton,0,0,">");
				wrefresh(botton);
				wrefresh(my_iteml);
				wrefresh(notice);
				break;
			case KEY_NPAGE: 
				wclear(notice);
				ret = menu_driver(my_menu, REQ_SCR_DPAGE);
        if ( ret == E_REQUEST_DENIED ){
					ret = menu_driver(my_menu, REQ_LAST_ITEM);
				}
        cur_item = item_name(current_item(my_menu));
        dir = (char *)&cur_item[0];
          mvwprintw(notice,0,0,"%s",dir);
          length=strlen(dir);
          to_transfer_to_other_line(int line=0;length > ws.ws_col;line++){
            dir += length;
            mvwprintw( notice, line ,0,"%s",dir);
          	length=strlen(dir);
          }
				mvwaddstr(botton,0,0,">");
				wrefresh(my_iteml);
				wrefresh(botton);
				wrefresh(notice);
        break;
			case KEY_PPAGE:
				wclear(notice);
        ret = menu_driver(my_menu, REQ_SCR_UPAGE);
        if ( ret == E_REQUEST_DENIED ){
					ret = menu_driver(my_menu, REQ_FIRST_ITEM);
				}
        cur_item = item_name(current_item(my_menu));
        dir = (char *)&cur_item[0];
          mvwprintw(notice, 0,0,"%s",dir);
          length=strlen(dir);
          to_transfer_to_other_line(int line=0;length > ws.ws_col;line++){
            dir += length;
            mvwprintw( notice, line ,0,"%s",dir);
          	length=strlen(dir);
          }
				mvwaddstr(botton,0,0,">");
				wrefresh(my_iteml);
				wrefresh(botton);
				wrefresh(notice);
        break;
			case KEY_UP: 
				wclear(notice);
				ret = menu_driver(my_menu, REQ_UP_ITEM);
				if ( ret == E_REQUEST_DENIED )
					break;
				cur_item = item_name(current_item(my_menu));
				dir = (char *)&cur_item[0];
					mvwprintw(notice, 0,0,"%s",dir);
					length=strlen(dir);
					to_transfer_to_other_line(int line=0;length > ws.ws_col;line++){
						dir += length;
						mvwprintw( notice, line ,0,"%s",dir);
						length=strlen(dir);
					}
				mvwaddstr(botton,0,0,">");
				wrefresh(my_iteml);
				wrefresh(botton);
				wrefresh(notice);
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
				mvwaddstr(botton,0,0,">");
				wrefresh(my_iteml);
				wrefresh(botton);
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
				wclear(botton);
					mvwaddstr(botton,0,0,">");
					mvwaddstr(botton,0,1,ss);
					i = 0;
					a = 0;

					/* перевести запроса  в нижний регистр */
					memset ( query, 0, 64 );
					strncpy ( query, ss, 63 );
					char *ptr = &query[0];
					for ( int i = 0; i <= 63; i++, ptr++ ) {
						if ( *ptr >= 0x41 && *ptr <= 0x5a )
							*ptr += 0x20;
					}


					for (i = 0; i <= si - 1; i++){
						/* перевести строку меню в нижний регистр */
						memset ( query_line, 0, 1024 );
						strncpy ( query_line, choices[i], strlen ( choices[i] ) - 1 );
						int len = strlen ( choices[i] );
						ptr = &query_line[0];
						for ( int i = 0; i <= len; i++, ptr++ ) {
							if ( *ptr >= 0x41 && *ptr <= 0x5a )
								*ptr += 0x20;
						}


						if (pos == 0){
							my_items[i] = new_item(choices[i],NULL);
							a = i + 1;
						}
						else 
							if ( strstr(query_line, query ) ) {
							my_items[a] =  new_item(choices[i],NULL);
							a++;

						}
					}
					for (; a <= i; a++){
						my_items[a] = new_item(NULL,NULL);
					}
						unpost_menu(my_menu);	
						free_menu(my_menu);
						my_menu = new_menu((ITEM **)my_items);
						my_iteml = derwin(botton, 16, ws.ws_col - 2, SUBWIN, 1);	
						keypad(my_iteml,TRUE);
						set_menu_win(my_menu,my_iteml);
						
						set_menu_mark(my_menu," * ");
						post_menu(my_menu);
						wrefresh(botton);
						wrefresh(my_iteml);
						set_top_row(my_menu,row);
						cur_item = item_name(current_item(my_menu));
							dir = (char *)&cur_item[0];
						if ( dir ){
							mvwprintw(notice,0,0,"%s",dir);
							length=strlen(dir);
							to_transfer_to_other_line(int line=0;length > ws.ws_col;line++){
							dir += length;
							mvwprintw( notice,line ,0,"%s",dir);
							length=strlen(dir);
							}
						}
						wrefresh(notice);
					break;
		}
		clear();
		post_menu(my_menu);
		wrefresh(my_iteml);
		wrefresh(botton);
	}


	
	
	free(cf);
	unpost_menu(my_menu);
	free_menu(my_menu);
	endwin();
	exit(0);
}
