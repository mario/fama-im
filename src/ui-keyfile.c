#include "common.h"
#include <string.h>
#include <glib/gstdio.h>

const gchar *default_keyfile = "[core]\n"
	"contact_list_width=30\n"
	"\n# If commented, Fama uses the charset of the current locale\n"
	"#charset=UTF-8\n"
	"\n# Turn logging on or off\n"
	"logging=off\n"
	"\n# Suppress messages to stderr\n"
	"redirect_stderr=/dev/null\n"
	"\n[colors]\n"
	"# Available colors are\n"
	"# red, green, blue, black, white\n"
	"# cyan, yellow, magenta and default\n"
	"borders=green\n"
	"command_line=default\n"
	"window_title=cyan\n"
	"outgoing_message=green\n"
	"incoming_message=cyan\n"
	"status_available=green\n"
	"status_away=cyan\n"
	"status_busy=magenta\n"
	"status_idle=yellow\n" "status_offline=default\n" "status_other=red\n"
	"\n[history]\n"
	"history_filename=history\n"
	"history_maxnumber=1024\n";

GKeyFile *keyFile = NULL;

GKeyFile *
keyfile_get()
{
	return keyFile;
}

gboolean
keyfile_read()
{
	gchar *path;
	GError *err = NULL;
	gint flags = G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS;

	path = g_strdup_printf("%s/%s/%s", g_get_home_dir(), FAMA_CONFIG_DIR,
			       FAMA_CONFIG_FILE);
	g_assert(path != NULL);

	keyFile = g_key_file_new();

	g_key_file_load_from_file(keyFile, path, flags, &err);

	if (err != NULL) {
		if (err->code == G_KEY_FILE_ERROR_NOT_FOUND ||
		    err->code == G_FILE_ERROR_NOENT) {
			g_clear_error(&err);
			g_warning("Keyfile not found, creating a new one");
			g_key_file_load_from_data(keyFile, default_keyfile,
						  strlen(default_keyfile),
						  flags, &err);

			keyfile_write();
		}

		if (err != NULL) {
			g_warning("Unable to read config: %s\n", err->message);
			g_clear_error(&err);
			return FALSE;
		}
	}

	return TRUE;
}

gboolean
keyfile_write()
{
	GError *err = NULL;
	gchar *data, *path;
	gsize length;

	path = g_strdup_printf("%s/%s", g_get_home_dir(), FAMA_CONFIG_DIR);
	if (g_mkdir_with_parents(path, 0700) != 0) {
		g_warning("Cannot create directories for '%s'", path);
		g_free(path);
		return FALSE;
	}
	g_free(path);


	data = g_key_file_to_data(keyFile, &length, &err);
	if (data == NULL) {
		g_warning("Cannot get keyfile data: %s", err->message);
		g_free(path);
		return FALSE;
	}

	path = g_strdup_printf("%s/%s/%s", g_get_home_dir(), FAMA_CONFIG_DIR,
			       FAMA_CONFIG_FILE);
	if (g_file_set_contents(path, data, length, &err) == FALSE) {
		g_warning("Cannot write to %s: %s", path, err->message);
		g_free(data);
		g_free(path);
		return FALSE;
	}

	g_free(data);
	g_free(path);

	return TRUE;
}
