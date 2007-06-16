#include "common.h"

TpaContactPresence
_string_to_presence(gchar *string)
{
	if (g_ascii_strcasecmp(string, "available") == 0)
		return TPA_PRESENCE_AVAILABLE;
	else if (g_ascii_strcasecmp(string, "offline") == 0)
		return TPA_PRESENCE_OFFLINE;
	else if (g_ascii_strcasecmp(string, "away") == 0)
		return TPA_PRESENCE_AWAY;
	else if (g_ascii_strcasecmp(string, "hidden") == 0)
		return TPA_PRESENCE_HIDDEN;
	else if (g_ascii_strcasecmp(string, "busy") == 0)
		return TPA_PRESENCE_BUSY;

	return 0;
}


/*
 * Set status for the current user
 */
gboolean
command_func_status(gint argc, gchar ** argv)
{
	TpaContactPresence presence;
	TpaUserContact *user;
	GPtrArray *connections;
	FamaConnection *conn;
	gboolean success;
	gint i;

	if (argc < 2) {
		g_warning("usage: status <status> <message>\n"
			  "where <status> is available, away, busy, hidden or offline.\n"
			  "The <message> parameter is optional.");

		return FALSE;
	}

	connections = connection_get_connections();
	if (!connections) {
		g_warning("no connections available!");
		return FALSE;
	}

	presence = _string_to_presence (argv[1]);
	if (!presence) {
		g_warning("Unrecognized presence string '%s'", argv[1]);
		return FALSE;
	}

	for (i = 0; i < connections->len; i++) {
	conn = g_ptr_array_index(connections, i);

	user = tpa_connection_get_user_contact (conn->connection);
	g_assert(user);

	if (argc >= 3) {
		success = tpa_user_contact_set_presence_with_message(user, presence, argv[2]);
		if (success)
			g_message("%s: Presence set to %s - '%s'", conn->account, argv[1], argv[2]);
	} else {
		success = tpa_user_contact_set_presence(user, presence);
		if (success)
			g_message("%s: Presence set to '%s'", conn->account, argv[1]);
	}

	if (!success) {
		g_warning("Could not set contact presence.");
		return FALSE;
	}

	}

	return TRUE;
}
