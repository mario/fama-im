#include <string.h>
#include <stdlib.h>
#include "common.h"

gboolean
command_func_account(gint argc, gchar ** argv)
{
	if (argc < 2) {
		g_warning("usage: /account [add|remove|list]");
		return FALSE;
	}

	if (g_ascii_strcasecmp("add", argv[1]) == 0) {
		if (argc < 4) {
			g_warning("usage: /account add <protocol> <login>");
			return FALSE;
		}

		account_add(argv[2], argv[3]);
	} else if (g_ascii_strcasecmp("remove", argv[1]) == 0) {
		g_warning
			("Not implemented, do it manually by editing the accounts file.");
		return FALSE;
	} else if (g_ascii_strcasecmp("list", argv[1]) == 0) {
		gchar **names;
		gchar *string;
		wchar_t *outbuf;

		names = account_get_names();
		string = g_strjoinv("\n", names);
		outbuf = g_new(wchar_t, strlen(string) + 1);

		utf8_to_wchar(string, outbuf, strlen(string));
		window_add_message(window_get_main(), L"List of accounts:",
				   A_BOLD, outbuf);

		g_strfreev(names);
		g_free(string);
		g_free(outbuf);
	}

	return TRUE;
}
