#include "common.h"

#define BORDER ' '

int max_x, max_y;

void
init_interface()
{
	FamaWindow *mainWin;

	stdscr = initscr();
	keypad(stdscr, TRUE);
	noecho();

	refresh();

	getmaxyx(stdscr, max_y, max_x);

	color_init();
	commandline_init();

	/*
	 * Only create a new main-window if there is no
	 * existing one already.
	 */
	if (window_get_current() == NULL) {
		mainWin = window_new(WindowTypeMain);
		window_set_title(mainWin, L"Console window");
		window_set_current(mainWin);
	}

}

void
destroy_interface()
{
	contactlist_destroy();

	erase();
	refresh();
	endwin();
}

void
draw_interface()
{
	ColorSettings *c = color_get();

	/*
	 * Draw all borders 
	 */

	attron(c->borders);
	mvvline(0, max_x - contactlist_get_width(), 0, max_y);

	attron(A_REVERSE);
	mvhline(0, 0, BORDER, max_x);
	mvhline(max_y - 2, 0, BORDER, max_x);
	mvaddwstr(0, (max_x - 12) / 2, L"F A M A  I M");
	attroff(A_REVERSE | c->borders);
	update_panels();
	doupdate();

	window_draw_title_bar();
	window_resize_all();

	/*
	 * Draw contact list 
	 */
	contactlist_draw();

	/*
	 * Draw command-line
	 */
	commandline_draw();
}

gint
get_max_x()
{
	return max_x;
}

gint
get_max_y()
{
	return max_y;
}

void
redraw_interface()
{
	destroy_interface();
	init_interface();
	draw_interface();
}
