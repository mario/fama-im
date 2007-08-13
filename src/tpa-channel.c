#include "common.h"
#include <string.h>

typedef enum {
	MessageReceived,
	MessageSent,
	MessageError,
} FamaMessageType;

void
message_add_text_message(TpaTextChannel * channel, TpaTextMessage * message,
			 FamaMessageType type)
{
	ColorSettings *c;
	FamaWindow *win;
	const gchar *contents, *uri, *time, *account, *contact;
	wchar_t *contents_w, *uri_w, *title;
	gint title_len, attr;

	c = color_get();

	if ((win = window_find_channel(TPA_CHANNEL(channel))) == NULL) {
		g_warning("Message on non-existant channel!");
		return;
	}

	contents = tpa_text_message_get_contents(message);
	uri = tpa_text_message_get_uri(message);
	time = clock_get_time();
	account =
		tpa_channel_target_get_uri(TPA_CHANNEL_TARGET
					   (tpa_channel_get_owner
					    (&channel->parent)));
	contact =
		tpa_channel_target_get_uri(tpa_channel_get_target
					   (&channel->parent));

	contents_w = g_new(wchar_t, strlen(contents) + 1);
	uri_w = g_new(wchar_t, strlen(uri) + 1);
	utf8_to_wchar(contents, contents_w, strlen(contents));
	utf8_to_wchar(uri, uri_w, strlen(uri));

	title_len = wcslen(uri_w) + strlen(time) + 5;
	title = g_new(wchar_t, title_len);
	swprintf(title, title_len - 1, L"[%s] %ls", time, uri_w);

	if (type == MessageSent)
		attr = c->outgoing_message;
	else if (type == MessageReceived)
		attr = c->incoming_message;
	else
		attr = COLOR_PAIR(1);

	window_add_message(win, title, A_BOLD | attr, contents_w);

	if (get_logging() == TRUE)
		write_to_log(account, contact, uri, contents, time);

	g_free(title);
	g_free(contents_w);
	g_free(uri_w);

}

void
message_received_cb(TpaTextChannel * channel, TpaTextMessage * message)
{
	message_add_text_message(channel, message, MessageReceived);
}

void
message_sent_cb(TpaTextChannel * channel, TpaTextMessage * message)
{
	message_add_text_message(channel, message, MessageSent);
}

void
message_delivery_error_cb(TpaTextChannel * channel, TpaTextMessage * message,
			  TpaTextMessageDeliveryError error)
{
	message_add_text_message(channel, message, MessageError);
}

void
channel_send_message(TpaTextChannel * text_channel, gchar * contents)
{
	tpa_text_channel_send(text_channel, contents, TPA_MESSAGE_TYPE_NORMAL);
}

void
channel_created_cb(TpaConnection * conn, TpaChannel * channel)
{
	const gchar *target;

	target = tpa_channel_target_get_uri(tpa_channel_get_target(channel));

	g_message("New channel created with '%s'", target);

	if (tpa_channel_get_channel_type(channel) == TPA_CHANNEL_TYPE_TEXT) {
		GPtrArray *messages;
		TpaTextChannel *text_channel;
		FamaWindow *win;
		wchar_t *target_w;

		tpa_channel_join(channel);
		text_channel = TPA_TEXT_CHANNEL(channel);

		win = window_new(WindowTypeConversation);
		win->channel = channel;
		win->is_updated = TRUE;

		/*
		 * uncomment to automatically change current window 
		 */
		/*
		 * window_set_current(win); 
		 */

		target_w = g_new(wchar_t, strlen(target) + 1);
		utf8_to_wchar(target, target_w, strlen(target));
		window_set_title(win, target_w);
		g_free(target_w);
		window_draw_title_bar();
		statusbar_draw();

		messages = tpa_text_channel_get_pending(text_channel, TRUE);
		if (messages) {
			TpaTextMessage *message;
			gint i;

			for (i = 0; i < messages->len; i++) {
				message = g_ptr_array_index(messages, i);
				message_add_text_message(text_channel, message,
							 MessageReceived);
			}
		}


		g_signal_connect(G_OBJECT(text_channel), "message-sent",
				 G_CALLBACK(message_sent_cb), NULL);
		g_signal_connect(G_OBJECT(text_channel), "message-received",
				 G_CALLBACK(message_received_cb), NULL);
		g_signal_connect(G_OBJECT(text_channel),
				 "message-delivery-error",
				 G_CALLBACK(message_delivery_error_cb), NULL);

		update_panels();
		doupdate();
	}
}
