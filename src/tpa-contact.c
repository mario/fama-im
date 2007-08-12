#include "common.h"

gboolean
contact_add(gchar * uri, gchar * account)
{
	FamaConnection *conn = NULL;
	TpaContactList *list = NULL;
	TpaContact *contact = NULL;
	GPtrArray *connections = NULL;

	connections = connection_get_connections();
	if (connections == NULL) {
		g_warning("There are no active connections");
		return FALSE;
	}
	conn = g_ptr_array_index(connection_get_connections(), 0);
	conn->connection = connection_get_connection_from_account(account);

	if (conn->connection == NULL) {
		g_warning("The specified account is not active");
		return FALSE;
	}
	list = tpa_connection_get_contactlist(conn->connection);
	if (list == NULL) {
		g_warning("Could not retrieve the contact list");
		return FALSE;
	}

        tpa_contact_list_add(list, uri);
        contact = tpa_contact_list_get_contact(list, uri);
        if (contact == NULL) {
		g_warning("No such user");
		return FALSE;
	}

	if (account == uri)
		return FALSE;

	tpa_contact_authorize(contact, TRUE);
	tpa_contact_subscribe(contact, TRUE);

	return TRUE;
}

gboolean
contact_remove(gchar * uri, gchar * account)
{
	FamaConnection *conn = NULL;
	TpaContactList *list = NULL;
	TpaContact *contact = NULL;
	GPtrArray *connections = NULL;

	connections = connection_get_connections();
	if (connections == NULL) {
		g_warning("There are no active connections");
		return FALSE;
	}
	conn = g_ptr_array_index(connection_get_connections(), 0);
	conn->connection = connection_get_connection_from_account(account);
	if (conn->connection == NULL) {
		g_warning("The specified account is not active");
		return FALSE;
	}
	list = tpa_connection_get_contactlist(conn->connection);
	if (list == NULL) {
		g_warning("Could not retrieve the contact list");
		return FALSE;
	}

        contact = tpa_contact_list_get_contact(list, uri);
	if (contact == NULL) {
		g_warning("No such user");
		return FALSE;
	}

	/*
	 * resets all for the user in question 
	 */
	tpa_contact_list_remove(list, contact);
	tpa_contact_authorize(contact, FALSE);
	tpa_contact_subscribe(contact, FALSE);

	return TRUE;
}


gboolean
contact_authorize(gchar * uri, gchar * account)
{
	TpaContactList *list = NULL;

	FamaConnection *conn = NULL;
	GPtrArray *connections = NULL;
        TpaContact *contact = NULL;
	connections = connection_get_connections();
	if (connections == NULL) {
		g_warning("There are no active connections");
		return FALSE;
	}
	conn = g_ptr_array_index(connection_get_connections(), 0);
	conn->connection = connection_get_connection_from_account(account);
	if (conn->connection == NULL) {
		g_warning("The specified account is not active");
		return FALSE;
	}
	list = tpa_connection_get_contactlist(conn->connection);
	if (list == NULL) {
		g_warning("Could not retrieve the contact list");
		return FALSE;
	}

        contact = tpa_contact_list_get_contact(list, uri);
	if (contact == NULL) {
		g_warning("No such user");
		return FALSE;
	}

/* Authorize */
	tpa_contact_authorize(contact, TRUE);
	tpa_contact_subscribe(contact, TRUE);

	return TRUE;
}
