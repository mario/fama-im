#include "common.h"
#include <string.h>

static void
  status_changed_cb(TpaConnection *, TpaConnectionStatus, TpaConnectionStatusReason);

GPtrArray *connections = NULL;

GPtrArray *
connection_get_connections()
{
	return connections;
}

void
connection_disconnect_all()
{
	TpaConnection *a;
	gint i;

	if (connections == NULL)
		return;

	for (i = 0; i < connections->len; i++) {
		a = g_ptr_array_index(connections, i);

		if (a != NULL)
			tpa_connection_disconnect(a);
	}
}

TpaConnection *
connection_connect(gchar * account, gchar * password)
{
	TpaParameter *parameter;
	TpaProfile *profile;
	TpaConnection *conn;
	TpaManager *manager;

	/*
	 * setup the profile for connection 
	 */

	if (!account_get_profile(account, &profile)) {
		g_warning("failed to get account profile!");
		return FALSE;
	}

	parameter = tpa_profile_get_parameter(profile, "password");
	tpa_parameter_set_value_as_string(parameter, password);

	manager =
		tpa_manager_factory_get_manager(manager_factory_get(),
						tpa_profile_get_protocol
						(profile));
	if (!manager) {
		g_warning("failed to create Connection Manager!");
		return FALSE;
	}

	/*
	 * Get a Connection 
	 */
	conn = tpa_manager_request_connection(manager, profile);

	if (!conn) {
		g_warning("failed to create Connection!");
		return NULL;
	}

	/*
	 * Set up connection callbacks 
	 */
	g_signal_connect(G_OBJECT(conn), "status-changed",
			 G_CALLBACK(status_changed_cb), NULL);
	g_signal_connect(G_OBJECT(conn), "channel-created",
			 G_CALLBACK(channel_created_cb), NULL);

	/*
	 * Connect! 
	 */
	tpa_connection_connect(conn);

	if (connections == NULL)
		connections = g_ptr_array_new();

	g_ptr_array_add(connections, conn);

	return conn;
}

void
connection_handle_reason(TpaConnectionStatusReason reason)
{
	switch (reason) {

	case TPA_CONNECTION_STATUS_REASON_REQUESTED:
		g_message("Requested by user.");
		break;
	case TPA_CONNECTION_STATUS_REASON_NETWORK_ERROR:
		g_warning("Network error.");
		break;
	case TPA_CONNECTION_STATUS_REASON_AUTHENTICATION_FAILED:
		g_warning("Authentication failed.");
		break;
	case TPA_CONNECTION_STATUS_REASON_ENCRYPTION_ERROR:
		g_warning("Encryption error.");
		break;
	case TPA_CONNECTION_STATUS_REASON_NAME_IN_USE:
		g_warning("Name in use.");
		break;
	case TPA_CONNECTION_STATUS_REASON_CERT_NOT_PROVIDED:
	case TPA_CONNECTION_STATUS_REASON_CERT_UNTRUSTED:
	case TPA_CONNECTION_STATUS_REASON_CERT_EXPIRED:
	case TPA_CONNECTION_STATUS_REASON_CERT_NOT_ACTIVATED:
	case TPA_CONNECTION_STATUS_REASON_CERT_HOSTNAME_MISMATCH:
	case TPA_CONNECTION_STATUS_REASON_CERT_FINGERPRINT_MISMATCH:
	case TPA_CONNECTION_STATUS_REASON_CERT_OTHER_ERROR:
		g_warning("Certificate error (%d).", reason);
		break;
	case TPA_CONNECTION_STATUS_REASON_NONE_SPECIFIED:
	default:
		g_warning("Unknown reason.");
		break;
	}
}

static void
status_changed_cb(TpaConnection * conn, TpaConnectionStatus status,
		  TpaConnectionStatusReason reason)
{
	if (status == TPA_CONNECTION_STATUS_CONNECTING) {
		g_message("Connecting..");
	} else if (status == TPA_CONNECTION_STATUS_CONNECTED) {
		TpaUserContact *user;

		g_message("%s: Connected!", tpa_connection_get_protocol(conn));

		contactlist_reload_from_server(conn);
		contactlist_sort();
		contactlist_draw();

		user = tpa_connection_get_user_contact(conn);
		tpa_user_contact_set_capabilities(user, TPA_CAPABILITY_TEXT);
	}
	if (status == TPA_CONNECTION_STATUS_DISCONNECTED) {
		g_message("Disconnected.");
		connection_handle_reason(reason);
		contactlist_remove_from_connection(conn);
		contactlist_draw();
		g_ptr_array_remove(connections, conn);
		tpa_connection_disconnect(conn);
	}
}
