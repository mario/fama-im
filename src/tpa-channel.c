#include "common.h"

void
message_received_cb (TpaTextChannel *channel, TpaTextMessage *message)
{
}

void
message_sent_cb (TpaTextChannel *channel, TpaTextMessage *message)
{
}

void
channel_send_message (TpaTextChannel *text_channel, gchar *contents)
{
	tpa_text_channel_send (text_channel, contents, TPA_MESSAGE_TYPE_NORMAL);
}

void
channel_created_cb (TpaConnection *conn, TpaChannel *channel)
{
	if (tpa_channel_get_channel_type (channel) == TPA_CHANNEL_TYPE_TEXT) {
		int i;
		GPtrArray *messages;
		TpaTextMessage *msg;
		TpaTextChannel *text_channel;
		FamaWindow *win;

		text_channel = TPA_TEXT_CHANNEL (channel);	
		messages = tpa_text_channel_get_pending (text_channel, TRUE);

                win = window_new(WindowTypeConversation);
		win->channel = channel;
                window_set_current(win);

		for (i = 0; i < messages->len; i++) {
			msg = g_ptr_array_index (messages, i);			



		}

		g_signal_connect (G_OBJECT (text_channel), "message-sent",
					G_CALLBACK (message_sent_cb), NULL);
		g_signal_connect (G_OBJECT (text_channel), "message-received",
					G_CALLBACK (message_received_cb), NULL);
	}
}
