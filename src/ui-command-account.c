#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "ui-window.h"
#include <glib/gprintf.h>
#include "mc-accounts.h"
#include "hintstring.h"

static gboolean command_analyze_account_remove(gint argc, gchar **argv);

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
		if (argc < 5) {
			g_warning(HINTSTR_ACCOUNTADD);
			return FALSE;
		}

		//account_add(argv[2], argv[3]);
		mc_account_add(argv[2], argv[3], argv[4]);
	} else if (g_ascii_strcasecmp("remove", argv[1]) == 0) {
		//g_warning
		//	("Not implemented, do it manually by editing the accounts file.");
		return command_analyze_account_remove(argc, argv);
	} else if (g_ascii_strcasecmp("list", argv[1]) == 0) {
		gchar **names;
		gchar *string;
		wchar_t *outbuf;

		//names = account_get_names();
		names = mc_get_account_names();
		string = g_strjoinv("\n", names);
		outbuf = g_new(wchar_t, strlen(string) + 1);

		utf8_to_wchar(string, outbuf, strlen(string));
		window_add_message(window_get_main(), L"List of accounts:",
				   A_BOLD, outbuf);

		g_strfreev(names);
		g_free(string);
		g_free(outbuf);
	} else if (g_ascii_strcasecmp("protocols", argv[1]) == 0) {
		//GPtrArray *items = NULL;
		wchar_t *outbuf, title[32];
		gchar *string;
		//gchar *mbseq = NULL;
		gchar **protonames;

		wcscpy(title, L"List of available protocols");
		protonames = mc_get_protocol_names();
		string = g_strjoinv("\n", protonames);
		outbuf = g_new(wchar_t, strlen(string) + 1);

		utf8_to_wchar(string, outbuf, strlen(string));
		window_add_message(window_get_main(), title, 
				   A_BOLD, outbuf);

		g_strfreev(protonames);
		g_free(string);
		g_free(outbuf);
		/*
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
		g_free(outbuf);
		*/
	}else {
		g_warning("usage: /account [add|remove|list|protocols]");
	}

	return TRUE;
}

static gboolean
command_analyze_account_remove(gint argc, gchar **argv)
{
	gchar string[256];
	wchar_t *outbuf;
	const gchar *accstr;
	gint	acctype;
	g_assert((g_ascii_strcasecmp(argv[1], "remove") == 0));
	/** Remove by displayname will be implement in future*/

	if (argc == 2) {
		//show hints
		g_warning(HINTSTR_ACCOUNTREMOVE);
		return FALSE;
	} else if (argc == 3) {
		acctype = ACCTYPE_BYUNINAME;
		accstr = argv[2];
	} else if (argc == 4) {
		//unique name
		if(g_ascii_strcasecmp(argv[2], "U") == 0)
			acctype = ACCTYPE_BYUNINAME;
		else if(g_ascii_strcasecmp(argv[2], "D") == 0) //display
			acctype = ACCTYPE_BYDISNAME;
		else {
			g_warning("Option is false!");
			return FALSE;
		}
		accstr = argv[3];
	} else {
		g_warning("Paramters's number is out of range!");
		return FALSE;
	}

	if(!mc_remove_account(accstr, acctype)) {
		g_warning("Can't remove account %s!does it exist?", accstr);
		return FALSE;
	}
	g_sprintf(string, "Remove account %s successfully!", accstr);
	outbuf = g_new(wchar_t, strlen(string) + 1);

	utf8_to_wchar(string, outbuf, strlen(string));
	window_add_message(window_get_main(), L"Remove account:",
			   A_BOLD, outbuf);

	g_free(outbuf);
	return TRUE;
}
