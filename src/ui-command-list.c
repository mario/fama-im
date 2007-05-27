#include "common.h"

#include <string.h>

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


/*
 * List supported profiles and accounts
 */
gboolean
command_func_list(gint argc, gchar ** argv)
{
	GPtrArray *items = NULL;
	wchar_t *outbuf, title[32];
	gchar *mbseq = NULL;

	if (argc < 2) {
		g_warning("list: usage: list [profiles|accounts]");
		return FALSE;
	}

	if (g_ascii_strncasecmp("profiles", argv[1], -1) == 0) {
		wcscpy(title, L"List of available protocols");
		items = tpa_manager_factory_get_all_profiles
			(manager_factory_get());
	} else if (g_ascii_strncasecmp("accounts", argv[1], -1) == 0) {
		g_warning("list: listing of accounts is not implemented");
		return FALSE;
	} else {
		g_warning("list: usage: list [profiles|accounts]");
		return FALSE;
	}

	if (items != NULL)
		g_ptr_array_foreach(items, _concat_ptr_array_items, &mbseq);

	if (mbseq == NULL) {
		window_add_message(window_get_current(), title, A_BOLD,
				   L"None. Please install one or more connection managers..");

		return FALSE;
	}

	outbuf = g_new(wchar_t, strlen(mbseq) + 1);
	utf8_to_wchar(mbseq, outbuf, strlen(mbseq));

	window_add_message(window_get_current(), title, A_BOLD, outbuf);

	g_ptr_array_free(items, TRUE);

	return TRUE;
}
