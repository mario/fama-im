#include "common.h"
#include <tapioca/tpa-client.h>

/*
 * Connect to connection manager
 */
gboolean
command_func_connect(gint argc, gchar ** argv)
{
	TpaConnection *conn;


	if (argc != 3) {
		g_warning("connect: usage: connect <account> <password>");
		return FALSE;
	}

	conn = connection_connect(argv[1], argv[2]);
	if (!conn) {
		g_warning("connect: failed to connect.");
		return FALSE;
	}

	return TRUE;
}
