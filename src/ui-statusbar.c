#include "common.h"
#include <string.h>

void
statusbar_draw()
{
	const gchar *time_str;
	gchar *status_str;
	wchar_t *wbuf;
	gint wbuf_len;
	ColorSettings *c = color_get();

	if (!interface_is_initialized())
		return;

	status_str = window_create_status_string();
	time_str = clock_get_time();
	wbuf_len = strlen(status_str) + strlen(time_str) + 12;

	wbuf = g_new(wchar_t, wbuf_len);

	swprintf(wbuf, wbuf_len - 1, L"[%s] Act: [%s]", time_str, status_str);

	attron(c->borders | A_REVERSE);

	attron(A_REVERSE);
	mvhline(get_max_y() - 2, 0, BORDER, get_max_x());
	mvwaddwstr_with_maxwidth(stdscr, get_max_y() - 2, 1, wbuf,
				 get_max_x() - 2);
	attroff(A_REVERSE | c->borders);

	update_panels();
	doupdate();

	g_free(status_str);
	g_free(wbuf);
}
