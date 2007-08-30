#include "common.h"

gboolean logging = TRUE;

gboolean
write_to_log(const gchar * account, const gchar * contact, const gchar * author,
	     const gchar * message, const gchar * time)
{
	GError *err = NULL;
	gchar *path;
	gsize length;
	gchar *output;
	GIOChannel *channel;

/* Create a log directory if there is none */
	path = g_strdup_printf("%s/%s/%s/%s", g_get_home_dir(),
			       FAMA_CONFIG_DIR, FAMA_LOGFILE_DIR, account);
	if (g_mkdir_with_parents(path, 0700) != 0) {
		g_warning("Cannot create directories for '%s'", path);
		g_free(path);
		return FALSE;
	}
	g_free(path);

/* Create a logfile if there is none */
	path = g_strdup_printf("%s/%s/%s/%s/%s", g_get_home_dir(),
			       FAMA_CONFIG_DIR, FAMA_LOGFILE_DIR,
			       account, contact);

/* add information to logfile*/
	channel = g_io_channel_new_file(path, "a", &err);
	output = g_strconcat(time, " ", author, "\n", message, "\n", NULL);
	g_io_channel_write_chars(channel, output, strlen(output), &length,
				 &err);

	if (err)
		g_warning("Could not write to logfile: %s", err->message);
	g_io_channel_flush(channel, NULL);
	g_free(path);
	return TRUE;
}

void
set_logging(gchar * c)
{
	if (g_ascii_strcasecmp(c, "on") == 0){
		logging = TRUE;
		g_message("Logging is on!");
	}
	else if (g_ascii_strcasecmp(c, "off") == 0){
		logging = FALSE;
		g_message("Logging is off!");
	}	
	else if (g_ascii_strcasecmp(c,"status") == 0){
		if (logging == TRUE)
			g_message("Logging is on!");
		else
			g_message("Logging is off!");
		}
	else
		g_warning("usage: /log [on|off|status]");
}

gboolean
get_logging()
{
	return logging;
}
