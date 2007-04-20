#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "common.h"

void
log_get_time(gchar *buf, gsize size)
{
	struct tm *ptr;
	time_t tm;

	tm = time(NULL);
	ptr = localtime(&tm);
	strftime(buf, size, "%H:%M:%S", ptr);
}

void
log_function(const gchar * log_domain, GLogLevelFlags log_level,
	     const gchar * message, gpointer user_data)
{
	FamaWindow *w;
	gchar time[32];
	wchar_t *wcs, title[128];
	gint slen, wlen;

	log_get_time(time, sizeof(time) - 1);

	/*
	 * Log to stderr 
	 */
	if ((w = window_get_index(0)) == NULL) {
		if (log_level == G_LOG_LEVEL_WARNING)
			fprintf(stderr, "** %s Warning: %s\n", time, message);
		else
			fprintf(stderr, "** %s Message: %s\n", time, message);

		return;
	}

	/*
	 * Log to NCurses main window 
	 */

	slen = strlen(message);
	wcs = g_new(wchar_t, slen + 1);
	wlen = mbstowcs(wcs, message, slen);
	g_assert(wlen != (size_t) - 1);

	wcs[wlen] = L'\0';

	if (log_level == G_LOG_LEVEL_WARNING) {
		swprintf(title, sizeof(title) - 1, L"[%s] Warning", time);
		window_add_message(w, title, A_BOLD | COLOR_PAIR(1), wcs);
	} else {
		swprintf(title, sizeof(title) - 1, L"[%s] Message", time);
		window_add_message(w, title, A_BOLD | COLOR_PAIR(3), wcs);
	}

	g_free(wcs);
}


void
log_init()
{
	/*
	 * Ncurses must be initialized before calling this function 
	 */
	g_assert(stdscr != NULL);

	g_log_set_handler(G_LOG_DOMAIN,
			  G_LOG_LEVEL_WARNING | G_LOG_LEVEL_MESSAGE,
			  log_function, NULL);
}
