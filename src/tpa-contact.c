#include "common.h"

gboolean
contact_add(gchar * dummy, gchar * uri)
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
	conn->account = connection_get_connection_from_account(dummy);
	if (conn->account == NULL) {
		g_warning("The specified account is not active");
		return FALSE;
	}
	list = tpa_connection_get_contactlist(conn->connection);
	if (list == NULL) {
		g_warning("Could not retrieve the contact list");
		return FALSE;
	}
	contact = tpa_contact_list_add(list, uri);
	if (contact == NULL) {
		g_warning("No such user");
		return FALSE;
	}

	if (dummy == uri)
		return FALSE;

/* This will send a authorization request to the user */
	tpa_contact_authorize(contact, TRUE);
	tpa_contact_subscribe(contact, TRUE);

	return TRUE;
}

gboolean
contact_remove(const gchar * dummy, const gchar * uri)
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
	conn->account = connection_get_connection_from_account(dummy);
	if (conn->account == NULL) {
		g_warning("The specified account is not active");
		return FALSE;
	}
	list = tpa_connection_get_contactlist(conn->connection);
	if (list == NULL) {
		g_warning("Could not retrieve the contact list");
		return FALSE;
	}
	contact = tpa_contact_list_add(list, uri);
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
	tpa_contact_set_authorization_status(contact, 0);
	tpa_contact_set_subscription_status(contact, 0);

	return TRUE;
}

gboolean
contact_authorize(const gchar * dummy, const gchar * uri)
{
	TpaContactList *list = NULL;
	TpaContact *contact = NULL;
	FamaConnection *conn = NULL;
	GPtrArray *connections = NULL;

	connections = connection_get_connections();
	if (connections == NULL) {
		g_warning("There are no active connections");
		return FALSE;
	}
	conn = g_ptr_array_index(connection_get_connections(), 0);
	conn->account = connection_get_connection_from_account(dummy);
	if (conn->account == NULL) {
		g_warning("The specified account is not active");
		return FALSE;
	}
	list = tpa_connection_get_contactlist(conn->connection);
	if (list == NULL) {
		g_warning("Could not retrieve the contact list");
		return FALSE;
	}
	contact = tpa_contact_list_add(list, uri);
	if (contact == NULL) {
		g_warning("No such user");
		return FALSE;
	}

/* Authorize */
	tpa_contact_authorize(contact, TRUE);
	tpa_contact_subscribe(contact, TRUE);
	tpa_contact_set_authorization_status(contact, 2);
	tpa_contact_set_subscription_status(contact, 2);

/* Not sure if this is needed, removes block if any */
	tpa_contact_block(contact, 1);

	return TRUE;
}
