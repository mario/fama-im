#include "common.h"

#include <tapioca/tpa-base.h>
#include <tapioca/tpa-client.h>

static void status_changed_cb(TpaConnection * conn, guint status, guint reason);
static void channel_created_cb(TpaConnection * conn, TpaChannel * channel);

TpaConnection *
connection_connect(gchar * account, gchar * password)
{
	TpaParameters *parameters;
	TpaConnection *conn;
	TpaManager *manager;

	manager =
		tpa_manager_factory_get_manager(manager_factory_get(),
						"jabber");
	if (!manager) {
		g_warning("failed to create Connection Manager!");
		return FALSE;
	}

	/*
	 * setup the parameters for connection 
	 */
	parameters = tpa_manager_get_protocol_parameters(manager, "jabber");

	tpa_parameters_set_value_as_string(parameters, "account", account);
	tpa_parameters_set_value_as_string(parameters, "password", password);
	tpa_parameters_set_value_as_string(parameters, "server",
					   "talk.google.com");
	tpa_parameters_set_value_as_string(parameters, "resource", "tapioca");
	tpa_parameters_set_value_as_uint(parameters, "port", 5223);
	tpa_parameters_set_value_as_boolean(parameters, "old-ssl", TRUE);
	tpa_parameters_set_value_as_boolean(parameters, "ignore-ssl-errors",
					    TRUE);

	/*
	 * Get a Connection 
	 */
	conn = tpa_manager_request_connection(manager, "jabber", parameters);

	if (!conn) {
		g_warning("failed to create Connection!\n");
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
		TpaContactList *list;
		GPtrArray *contacts;

		g_message("%s: Connected!", tpa_connection_get_protocol(conn));

		tpa_connection_get_open_channels(conn);
		list = tpa_connection_get_contactlist(conn);
		contacts = tpa_contact_list_get_known(list);
		user = tpa_connection_get_user_contact(conn);
		tpa_user_contact_set_capabilities(user, TPA_CAPABILITY_TEXT);
		contactlist_add_contacts (tpa_connection_get_protocol(conn), contacts);
	}
	if (status == TPA_CONNECTION_STATUS_DISCONNECTED) {
		g_message("Disconnected.");
		connection_handle_reason(reason);
	}
}

static gboolean
channel_play_cb(gpointer data)
{
	TpaStreamChannel *stream = TPA_STREAM_CHANNEL(data);

	tpa_stream_channel_join(stream);
	return FALSE;
}

static void
channel_created_cb(TpaConnection * conn, TpaChannel * channel)
{
	g_message("Got channel-created signal from TpaConnection\nchannel: %p",
		  channel);

	if (tpa_channel_get_channel_type(channel) == TPA_CHANNEL_TYPE_STREAM) {
		g_timeout_add(10000, channel_play_cb, channel);
	}
}
