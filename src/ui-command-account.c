#include <string.h>
#include <stdlib.h>
#include "common.h"

void
_concat_ptr_array_items(gpointer data, gpointer user_data)
{
	gchar *tmp;
	gchar **input = (gchar **) user_data;

	tmp = g_strjoin(", ", (gchar *) data, *input, NULL);

	if (*input != NULL)
		g_free(*input);

	*input = tmp;
}

gboolean
command_func_account(gint argc, gchar ** argv)
{
	if (argc < 2) {
		g_warning("usage: /account [add|remove|list|protocols]");
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
	} else if (g_ascii_strcasecmp("protocols", argv[1]) == 0) {
		GPtrArray *items = NULL;
		wchar_t *outbuf, title[32];
		gchar *mbseq = NULL;


		wcscpy(title, L"List of available protocols");
		items = tpa_manager_factory_get_all_profiles
			(manager_factory_get());

		if (items != NULL)
			g_ptr_array_foreach(items, _concat_ptr_array_items,
					    &mbseq);

		if (mbseq == NULL) {
			window_add_message(window_get_current(), title, A_BOLD,
					   L"None. Please install one or more connection managers..");
			return FALSE;
		}

		outbuf = g_new(wchar_t, strlen(mbseq) + 1);
		utf8_to_wchar(mbseq, outbuf, strlen(mbseq));

		window_add_message(window_get_current(), title, A_BOLD, outbuf);

		g_ptr_array_free(items, TRUE);
	}
	return TRUE;
}
