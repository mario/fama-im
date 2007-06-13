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
	TpaConnection *conn;
	TpaContactPresence presence;
	TpaUserContact *user;
	gboolean success;

	if (argc < 3) {
		g_warning("usage: status <account> <status> <message>\n"
			  "where <status> is available, away, busy, hidden or offline.\n"
			  "The <message> parameter is optional.");

		return FALSE;
	}

	conn = connection_get_connection_from_account(argv[1]);
	if (!conn) {
		g_warning("%s is not connected! Connect with /connect <account>.", argv[1]);
		return FALSE;
	}

	user = tpa_connection_get_user_contact (conn);
	g_assert(user);

	presence = _string_to_presence (argv[2]);
	if (!presence) {
		g_warning("Unrecognized presence string '%s'", argv[2]);
		return FALSE;
	}

	if (argc >= 4) {
		success = tpa_user_contact_set_presence_with_message(user, presence, argv[3]);
		if (success)
			g_message("Presence set to %s - '%s'", argv[2], argv[3]);
	} else {
		success = tpa_user_contact_set_presence(user, presence);
		if (success)
			g_message("Presence set to '%s'", argv[2]);
	}

	if (!success) {
		g_warning("Could not set contact presence.");
		return FALSE;
	}

	return TRUE;
}
