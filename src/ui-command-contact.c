#include <string.h>
#include <stdlib.h>
#include "common.h"

gboolean
command_func_contact(gint argc, gchar ** argv)
{
	if (argc < 2) {
		g_warning("usage: /contact [add|remove|auth]");
		return FALSE;
	}

	if (g_ascii_strcasecmp("add", argv[1]) == 0) {
		if (argc < 4) {
			g_warning
				("usage: /account add <contact to add> <account to add to>");
			return FALSE;
		}
//r		contact_add(argv[2], argv[3]);

	} else if (g_ascii_strcasecmp("remove", argv[1]) == 0) {
		if (argc < 4) {
			g_warning
				("usage: /account remove <contact to add> <account to add to>");
			return FALSE;
		}
//r		contact_remove(argv[2], argv[3]);
		return FALSE;

	} else if (g_ascii_strcasecmp("auth", argv[1]) == 0) {
		if (argc < 4) {
			g_warning
				("usage: /account auth <contact to add> <account to add to>");
			return FALSE;
		}
//r		contact_authorize(argv[2], argv[3]);
	}
	return TRUE;
}
