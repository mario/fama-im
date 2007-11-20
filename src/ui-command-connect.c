#include "common.h"
#include "ui-window.h"
//#include <tapioca/tpa-client.h>
#include "hintstring.h"
#include "mc-accounts.h"
#include <string.h>
#include <glib/gprintf.h>

/*
 * Connect to connection manager
 */
gboolean
command_func_connect(gint argc, gchar ** argv)
{
	//TpaConnection *conn;
	gchar string[256];
	wchar_t *outbuf;
	const gchar *accstr;
	gint	acctype;

	if (argc < 2 || argc >= 4) {
	//	g_warning("connect: usage: connect <account> <password>");
		g_warning(HINTSTR_CONNECT);
		return FALSE;
	}
	g_assert((g_ascii_strcasecmp(argv[0], "connect") == 0));
	/** Remove by displayname will be implement in future*/

	if(argc == 2) {
		acctype = ACCTYPE_BYUNINAME;
		accstr = argv[1];
	} else if (argc == 3) {
		/* unique name*/
		if(g_ascii_strcasecmp(argv[1], "U") == 0)
			acctype = ACCTYPE_BYUNINAME;
		else if(g_ascii_strcasecmp(argv[1], "D") == 0) //display
			acctype = ACCTYPE_BYDISNAME;
		else {
			g_warning("Option is false!");
			return FALSE;
		}
		accstr = argv[2];
	} else {
		g_warning("Paramters's number is out of range!");
		return FALSE;
	}
	
	//conn = connection_connect(argv[1], argv[2]);
	mc_account_connect(accstr, acctype, TRUE);
	/*if (!conn) {
		g_warning("connect: failed to connect.");
		return FALSE;
	}*/
	g_sprintf(string, "Connect account %s successfully!", accstr);
	outbuf = g_new(wchar_t, strlen(string) + 1);

	utf8_to_wchar(string, outbuf, strlen(string));
	window_add_message(window_get_main(), L"Connect account:",
			   A_BOLD, outbuf);

	g_free(outbuf);
	return TRUE;
}
