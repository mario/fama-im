#include "common.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void
log_function(const gchar * log_domain, GLogLevelFlags log_level,
	     const gchar * message, gpointer user_data)
{
	FamaWindow *w;
	const gchar *time;
	wchar_t *wcs, title[128];
	gint slen, wlen;

	time = clock_get_time();

	/*
	 * Log to stdout 
	 */
	if ((w = window_get_main()) == NULL) {
		if (log_level == G_LOG_LEVEL_WARNING ||
		    log_level == G_LOG_LEVEL_CRITICAL)
			printf("** %s Warning: %s\n", time, message);
		else
			printf("** %s Message: %s\n", time, message);

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

	if (log_level == G_LOG_LEVEL_WARNING ||
	    log_level == G_LOG_LEVEL_CRITICAL) {
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
			  G_LOG_LEVEL_WARNING | G_LOG_LEVEL_MESSAGE |
			  G_LOG_LEVEL_CRITICAL, log_function, NULL);
}
