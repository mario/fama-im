#include "common.h"
#include "ui-window.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <glib.h>
#include <glib/gstdio.h>
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
		    log_level == G_LOG_LEVEL_CRITICAL ||
		    log_level == G_LOG_LEVEL_DEBUG)
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
	    log_level == G_LOG_LEVEL_CRITICAL ||
	    log_level == G_LOG_LEVEL_DEBUG) {
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
			  G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_DEBUG, 
			  log_function, NULL);
}

void
fama_debug_set_log_file(const gchar *debugfile)
{
	const gchar *output_file;
	gint         out;

	output_file = debugfile;//g_getenv ("EMPATHY_LOGFILE");
	if (output_file == NULL) {
		return;
	}

	out = g_open(output_file, O_WRONLY | O_CREAT, 0644);
	if (out == -1) {
		g_warning ("Can't open logfile '%s': %s", output_file,
		g_strerror (errno));
		return;
	}
/*
	if (dup2 (out, STDOUT_FILENO) == -1) {
	g_warning ("Error when duplicating stdout file descriptor: %s",
	g_strerror (errno));
	return;
	}

	if (dup2 (out, STDERR_FILENO) == -1) {
	g_warning ("Error when duplicating stderr file descriptor: %s",
	g_strerror (errno));
	return;
	}
*/
}

void
fama_debug(const gchar *domain, const gchar *msg, ...)
{
	g_return_if_fail (msg != NULL);

	//debug_init ();

	return;
	va_list args;

	g_print ("%s: ", domain);

	va_start (args, msg);
	g_vprintf (msg, args);
	va_end (args);

	g_print ("\n");
//	break;
}

