#include "common.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

gchar timebuffer[32];

const gchar *
clock_get_time()
{
	return timebuffer;
}


gboolean
clock_cb (gpointer data)
{
        struct tm *ptr;
        time_t tm;

        tm = time(NULL);
        ptr = localtime(&tm);
        strftime(timebuffer, sizeof (timebuffer) - 1, "%H:%M", ptr);

	statusbar_draw();

	g_timeout_add ((60 - ((guint)tm % 60)) * 1000, clock_cb, NULL);

	return FALSE;
}
