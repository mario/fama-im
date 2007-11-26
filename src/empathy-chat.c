#include "common.h"
#include "ui-window.h"
#include <string.h>
#include <stdlib.h>

#include <glib/gi18n.h>

#include <libempathy/empathy-contact-manager.h>
#include <libempathy/empathy-log-manager.h>
#include <libempathy/empathy-debug.h>
#include <libempathy/empathy-utils.h>
#include <libempathy/empathy-conf.h>
#include <libempathy/empathy-marshal.h>

#include "empathy-chat.h"

#define GET_PRIV(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EMPATHY_TYPE_CHAT, EmpathyChatPriv))

#define DEBUG_DOMAIN "Chat"

#define CHAT_DIR_CREATE_MODE  (S_IRUSR | S_IWUSR | S_IXUSR)
#define CHAT_FILE_CREATE_MODE (S_IRUSR | S_IWUSR)

#define IS_ENTER(v) (v == GDK_Return || v == GDK_ISO_Enter || v == GDK_KP_Enter)

#define MAX_INPUT_HEIGHT 150

#define COMPOSING_STOP_TIMEOUT 5

struct _EmpathyChatPriv {
	EmpathyContactManager *manager;
	EmpathyLogManager     *log_manager;
	EmpathyTpChat         *tp_chat;
	FamaWindow            *window;
	guint                  composing_stop_timeout_id;
	gboolean               sensitive;
	gchar                 *id;
	GSList                *sent_messages;
	gint                   sent_messages_index;
	GList                 *compositors;
	guint                  scroll_idle_id;
	gboolean               first_tp_chat;
	EmpathyTime            last_log_timestamp;
	/* Used to automatically shrink a window that has temporarily
	 * grown due to long input. 
	 */
	gint                   padding_height;
	gint                   default_window_height;
	gint                   last_input_height;
	gboolean               vscroll_visible;
};

typedef enum {
	EmpathyMessageReceived,
	EmpathyMessageAutoReply,
	EmpathyMessageSent,
	EmpathyMessageError,
} FamaEmapthyMessageType;

static void             empathy_chat_class_init           (EmpathyChatClass              *klass);
static void             empathy_chat_init                 (EmpathyChat                   *chat);
static void             chat_finalize                     (GObject                       *object);
static void             chat_destroy_cb                   (EmpathyTpChat                 *tp_chat,
							   EmpathyChat                   *chat);
static void             chat_message_received_cb          (EmpathyTpChat                 *tp_chat,
							   EmpathyMessage                *message,
							   EmpathyChat                   *chat);
static void             chat_send_error_cb                (EmpathyTpChat                 *tp_chat,
							   EmpathyMessage                *message,
							   TelepathyChannelTextSendError  error_code,
							   EmpathyChat                   *chat);
void                    chat_sent_message_add             (EmpathyChat                   *chat,
							   const gchar                   *str);
const gchar *           chat_sent_message_get_next        (EmpathyChat                   *chat);
const gchar *           chat_sent_message_get_last        (EmpathyChat                   *chat);

static void             chat_composing_start              (EmpathyChat                   *chat);
static void             chat_composing_stop               (EmpathyChat                   *chat);
static void             chat_composing_remove_timeout     (EmpathyChat                   *chat);
static gboolean         chat_composing_stop_timeout_cb    (EmpathyChat                   *chat);
static void             chat_state_changed_cb             (EmpathyTpChat                 *tp_chat,
							   EmpathyContact                *contact,
							   TelepathyChannelChatState      state,
							   EmpathyChat                   *chat);
static void             chat_add_logs                     (EmpathyChat                   *chat);
static gboolean         chat_scroll_down_idle_func        (EmpathyChat                   *chat);
void                    empathy_message_add_text_message  (EmpathyChat   *chat, EmpathyMessage  *msg,  
			                                   FamaEmapthyMessageType type);

enum {
	COMPOSING,
	NEW_MESSAGE,
	NAME_CHANGED,
	STATUS_CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (EmpathyChat, empathy_chat, G_TYPE_OBJECT);

static void
empathy_chat_class_init (EmpathyChatClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = chat_finalize;

	signals[COMPOSING] =
		g_signal_new ("composing",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      0,
			      NULL, NULL,
			      g_cclosure_marshal_VOID__BOOLEAN,
			      G_TYPE_NONE,
			      1, G_TYPE_BOOLEAN);

	signals[NEW_MESSAGE] =
		g_signal_new ("new-message",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      0,
			      NULL, NULL,
			      empathy_marshal_VOID__OBJECT_BOOLEAN,
			      G_TYPE_NONE,
			      2, EMPATHY_TYPE_MESSAGE, G_TYPE_BOOLEAN);

	signals[NAME_CHANGED] =
		g_signal_new ("name-changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      0,
			      NULL, NULL,
			      g_cclosure_marshal_VOID__POINTER,
			      G_TYPE_NONE,
			      1, G_TYPE_POINTER);

	signals[STATUS_CHANGED] =
		g_signal_new ("status-changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      0,
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE,
			      0);

	g_type_class_add_private (object_class, sizeof (EmpathyChatPriv));
}

static void
empathy_chat_init (EmpathyChat *chat)
{
	EmpathyChatPriv *priv;
	chat->is_first_char = TRUE;

	priv = GET_PRIV (chat);

	priv->manager = empathy_contact_manager_new ();
	priv->log_manager = empathy_log_manager_new ();
	priv->default_window_height = -1;
	priv->vscroll_visible = FALSE;
	priv->sensitive = TRUE;
	priv->sent_messages = NULL;
	priv->sent_messages_index = -1;
	priv->first_tp_chat = TRUE;
}

static void
chat_finalize (GObject *object)
{
	EmpathyChat     *chat;
	EmpathyChatPriv *priv;

	chat = EMPATHY_CHAT (object);
	priv = GET_PRIV (chat);

	fama_debug (DEBUG_DOMAIN, "Finalized: %p", object);

	g_slist_foreach (priv->sent_messages, (GFunc) g_free, NULL);
	g_slist_free (priv->sent_messages);

	g_list_foreach (priv->compositors, (GFunc) g_object_unref, NULL);
	g_list_free (priv->compositors);

	chat_composing_remove_timeout (chat);
	g_object_unref (chat->account);
	g_object_unref (priv->manager);
	g_object_unref (priv->log_manager);

	if (priv->tp_chat) {
		g_object_unref (priv->tp_chat);
	}

	if (priv->scroll_idle_id) {
		g_source_remove (priv->scroll_idle_id);
	}

	g_free (priv->id);

	G_OBJECT_CLASS (empathy_chat_parent_class)->finalize (object);
}

static void
chat_destroy_cb (EmpathyTpChat *tp_chat,
		 EmpathyChat    *chat)
{
	EmpathyChatPriv *priv;

	priv = GET_PRIV (chat);

	if (priv->tp_chat) {
		g_object_unref (priv->tp_chat);
		priv->tp_chat = NULL;
	}
	priv->sensitive = FALSE;

	if (EMPATHY_CHAT_GET_CLASS (chat)->set_tp_chat) {
		EMPATHY_CHAT_GET_CLASS (chat)->set_tp_chat (chat, NULL);
	}
}

void
empathy_chat_send (EmpathyChat  *chat,
	   const gchar *msg)
{
	EmpathyChatPriv *priv;
	EmpathyMessage  *message;

	priv = GET_PRIV (chat);

	if (G_STR_EMPTY (msg)) {
		return;
	}
	/** for recently historical message*/
	chat_sent_message_add (chat, msg);

	/* FIXME: add here something to let group/privrate chat handle
	 *        some special messages */

	message = empathy_message_new (msg);

	empathy_tp_chat_send (priv->tp_chat, message);

	g_object_unref (message);
	chat->is_first_char = TRUE;
}

static void
chat_message_received_cb (EmpathyTpChat  *tp_chat,
			  EmpathyMessage *message,
			  EmpathyChat    *chat)
{
	EmpathyChatPriv *priv;
	EmpathyContact  *sender;
	EmpathyTime      timestamp;
	FamaEmpathyMessageType messagetype;
	EmpathyMessageType empathymessagetype;
	priv = GET_PRIV (chat);

	timestamp = empathy_message_get_timestamp (message);
	if (timestamp <= priv->last_log_timestamp) {
		/* Do not take care of messages anterior of the last
		 * logged message. Some Jabber chatroom sends messages
		 * received before we joined the room, this avoid
		 * displaying those messages if we already logged them
		 * last time we joined that room. */
		fama_debug (DEBUG_DOMAIN, "Skipping message because it is "
			       "anterior of last logged message.");
		return;
	}

	sender = empathy_message_get_sender (message);
	fama_debug (DEBUG_DOMAIN, "Appending message ('%s')",
		      empathy_contact_get_name (sender));

	empathy_log_manager_add_message (priv->log_manager,
					 empathy_chat_get_id (chat),
					 empathy_chat_is_group_chat (chat),
					 message);

	empathymessagetype = empathy_message_get_type(message);
	switch (empathymessagetype) {
	case EMPATHY_MESSAGE_TYPE_ACTION:/** FIXME: action isn't supported*/
		messagetype = EmpathyMessageError;	break;
	case EMPATHY_MESSAGE_TYPE_NORMAL:
		if (empathy_contact_is_user(sender)) {
			messagetype = EmpathyMessageSent;
		} else {
			messagetype = EmpathyMessageReceived;
		}
		break;
	case EMPATHY_MESSAGE_TYPE_NOTICE:
		messagetype = EmpathyMessageError;	break;
	case EMPATHY_MESSAGE_TYPE_AUTO_REPLY:
		messagetype = EmpathyMessageAutoReply;	break;
	default:	
		messagetype = EmpathyMessageError;	break;
	}
	empathy_message_add_text_message(chat, message, messagetype);

	if (empathy_chat_should_play_sound (chat)) {
		// FIXME: empathy_sound_play (EMPATHY_SOUND_CHAT);
	}

	g_signal_emit (chat, signals[NEW_MESSAGE], 0, message, FALSE);
}

static void
chat_send_error_cb (EmpathyTpChat                 *tp_chat,
		    EmpathyMessage                *message,
		    TelepathyChannelTextSendError  error_code,
		    EmpathyChat                   *chat)
{
	const gchar *error;
	gchar       *str;

	switch (error_code) {
	case TP_CHANNEL_TEXT_SEND_ERROR_OFFLINE:
		error = _("offline");
		break;
	case TP_CHANNEL_TEXT_SEND_ERROR_INVALID_CONTACT:
		error = _("invalid contact");
		break;
	case TP_CHANNEL_TEXT_SEND_ERROR_PERMISSION_DENIED:
		error = _("permission denied");
		break;
	case TP_CHANNEL_TEXT_SEND_ERROR_TOO_LONG:
		error = _("too long message");
		break;
	case TP_CHANNEL_TEXT_SEND_ERROR_NOT_IMPLEMENTED:
		error = _("not implemented");
		break;
	default:
		error = _("unknown");
		break;
	}

	str = g_strdup_printf (_("Error sending message '%s': %s"),
			       empathy_message_get_body (message),
			       error);
	g_free (str);
}

void 
chat_sent_message_add (EmpathyChat  *chat,
		       const gchar *str)
{
	EmpathyChatPriv *priv;
	GSList         *list;
	GSList         *item;

	priv = GET_PRIV (chat);

	/* Save the sent message in our repeat buffer */
	list = priv->sent_messages;
	
	/* Remove any other occurances of this msg */
	while ((item = g_slist_find_custom (list, str, (GCompareFunc) strcmp)) != NULL) {
		list = g_slist_remove_link (list, item);
		g_free (item->data);
		g_slist_free1 (item);
	}

	/* Trim the list to the last 10 items */
	while (g_slist_length (list) > 10) {
		item = g_slist_last (list);
		if (item) {
			list = g_slist_remove_link (list, item);
			g_free (item->data);
			g_slist_free1 (item);
		}
	}

	/* Add new message */
	list = g_slist_prepend (list, g_strdup (str));

	/* Set list and reset the index */
	priv->sent_messages = list;
	priv->sent_messages_index = -1;
}

const gchar *
chat_sent_message_get_next (EmpathyChat *chat)
{
	EmpathyChatPriv *priv;
	gint            max;
	
	priv = GET_PRIV (chat);

	if (!priv->sent_messages) {
		empathy_debug (DEBUG_DOMAIN, 
			      "No sent messages, next message is NULL");
		return NULL;
	}

	max = g_slist_length (priv->sent_messages) - 1;

	if (priv->sent_messages_index < max) {
		priv->sent_messages_index++;
	}
	
	empathy_debug (DEBUG_DOMAIN, 
		      "Returning next message index:%d",
		      priv->sent_messages_index);

	return g_slist_nth_data (priv->sent_messages, priv->sent_messages_index);
}

const gchar *
chat_sent_message_get_last (EmpathyChat *chat)
{
	EmpathyChatPriv *priv;

	g_return_val_if_fail (EMPATHY_IS_CHAT (chat), NULL);

	priv = GET_PRIV (chat);
	
	if (!priv->sent_messages) {
		empathy_debug (DEBUG_DOMAIN, 
			      "No sent messages, last message is NULL");
		return NULL;
	}

	if (priv->sent_messages_index >= 0) {
		priv->sent_messages_index--;
	}

	empathy_debug (DEBUG_DOMAIN, 
		      "Returning last message index:%d",
		      priv->sent_messages_index);

	return g_slist_nth_data (priv->sent_messages, priv->sent_messages_index);
}

static void
chat_composing_start (EmpathyChat *chat)
{
	EmpathyChatPriv *priv;

	priv = GET_PRIV (chat);

	if (priv->composing_stop_timeout_id) {
		/* Just restart the timeout */
		chat_composing_remove_timeout (chat);
	} else {
		empathy_tp_chat_set_state (priv->tp_chat,
					   TP_CHANNEL_CHAT_STATE_COMPOSING);
	}

	priv->composing_stop_timeout_id = g_timeout_add (
		1000 * COMPOSING_STOP_TIMEOUT,
		(GSourceFunc) chat_composing_stop_timeout_cb,
		chat);
}

static void
chat_composing_stop (EmpathyChat *chat)
{
	EmpathyChatPriv *priv;

	priv = GET_PRIV (chat);

	chat_composing_remove_timeout (chat);
	empathy_tp_chat_set_state (priv->tp_chat,
				   TP_CHANNEL_CHAT_STATE_ACTIVE);
}

static void
chat_composing_remove_timeout (EmpathyChat *chat)
{
	EmpathyChatPriv *priv;

	priv = GET_PRIV (chat);

	if (priv->composing_stop_timeout_id) {
		g_source_remove (priv->composing_stop_timeout_id);
		priv->composing_stop_timeout_id = 0;
	}
}

static gboolean
chat_composing_stop_timeout_cb (EmpathyChat *chat)
{
	EmpathyChatPriv *priv;

	priv = GET_PRIV (chat);

	priv->composing_stop_timeout_id = 0;
	empathy_tp_chat_set_state (priv->tp_chat,
				   TP_CHANNEL_CHAT_STATE_PAUSED);

	return FALSE;
}

static void
chat_state_changed_cb (EmpathyTpChat             *tp_chat,
		       EmpathyContact             *contact,
		       TelepathyChannelChatState  state,
		       EmpathyChat                *chat)
{
	EmpathyChatPriv *priv;
	GList          *l;
	gboolean        was_composing;

	priv = GET_PRIV (chat);

	if (empathy_contact_is_user (contact)) {
		/* We don't care about our own chat state */
		return;
	}

	was_composing = (priv->compositors != NULL);

	/* Find the contact in the list. After that l is the list elem or NULL */
	for (l = priv->compositors; l; l = l->next) {
		if (empathy_contact_equal (contact, l->data)) {
			break;
		}
	}

	switch (state) {
	case TP_CHANNEL_CHAT_STATE_GONE:
	case TP_CHANNEL_CHAT_STATE_INACTIVE:
	case TP_CHANNEL_CHAT_STATE_PAUSED:
	case TP_CHANNEL_CHAT_STATE_ACTIVE:
		/* Contact is not composing */
		if (l) {
			priv->compositors = g_list_remove_link (priv->compositors, l);
			g_object_unref (l->data);
			g_list_free1 (l);
		}
		break;
	case TP_CHANNEL_CHAT_STATE_COMPOSING:
		/* Contact is composing */
		if (!l) {
			priv->compositors = g_list_prepend (priv->compositors,
							    g_object_ref (contact));
		}
		break;
	default:
		g_assert_not_reached ();
	}

	fama_debug (DEBUG_DOMAIN, "Was composing: %s now composing: %s",
		      was_composing ? "yes" : "no",
		      priv->compositors ? "yes" : "no");

	if ((was_composing && !priv->compositors) ||
	    (!was_composing && priv->compositors)) {
		/* Composing state changed */
		g_signal_emit (chat, signals[COMPOSING], 0,
			       priv->compositors != NULL);
	}
}

static void
chat_add_logs (EmpathyChat *chat)
{
	EmpathyChatPriv *priv;
	GList          *messages, *l;
	guint           num_messages;
	guint           i;

	priv = GET_PRIV (chat);

	/* Add messages from last conversation */
	messages = empathy_log_manager_get_last_messages (priv->log_manager,
							  chat->account,
							  empathy_chat_get_id (chat),
							  empathy_chat_is_group_chat (chat));
	num_messages  = g_list_length (messages);

	for (l = messages, i = 0; l; l = l->next, i++) {
		EmpathyMessage *message;

		message = l->data;

		/* Only add 10 last messages */
		if (num_messages - i > 10) {
			g_object_unref (message);
			continue;
		}

		priv->last_log_timestamp = empathy_message_get_timestamp (message);

		g_object_unref (message);
	}
	g_list_free (messages);

	/* Scroll to the most recent messages, we reference the chat
	 * for the duration of the scroll func.
	 */
	priv->scroll_idle_id = g_idle_add ((GSourceFunc) chat_scroll_down_idle_func, 
					   g_object_ref (chat));
}

/* Scroll down after the back-log has been received. */
static gboolean
chat_scroll_down_idle_func (EmpathyChat *chat)
{
	EmpathyChatPriv *priv;

	priv = GET_PRIV (chat);

	g_object_unref (chat);

	priv->scroll_idle_id = 0;

	return FALSE;
}

gboolean
empathy_chat_get_is_command (const gchar *str)
{
	g_return_val_if_fail (str != NULL, FALSE);

	if (str[0] != '/') {
		return FALSE;
	}

	if (g_str_has_prefix (str, "/me")) {
		return TRUE;
	}
	else if (g_str_has_prefix (str, "/nick")) {
		return TRUE;
	}
	else if (g_str_has_prefix (str, "/topic")) {
		return TRUE;
	}

	return FALSE;
}

const gchar *
empathy_chat_get_name (EmpathyChat *chat)
{
	g_return_val_if_fail (EMPATHY_IS_CHAT (chat), NULL);

	if (EMPATHY_CHAT_GET_CLASS (chat)->get_name) {
		return EMPATHY_CHAT_GET_CLASS (chat)->get_name (chat);
	}

	return NULL;
}

gchar *
empathy_chat_get_tooltip (EmpathyChat *chat)
{
	g_return_val_if_fail (EMPATHY_IS_CHAT (chat), NULL);

	if (EMPATHY_CHAT_GET_CLASS (chat)->get_tooltip) {
		return EMPATHY_CHAT_GET_CLASS (chat)->get_tooltip (chat);
	}

	return NULL;
}

const gchar *
empathy_chat_get_status_icon_name (EmpathyChat *chat)
{
	g_return_val_if_fail (EMPATHY_IS_CHAT (chat), NULL);

	if (EMPATHY_CHAT_GET_CLASS (chat)->get_status_icon_name) {
		return EMPATHY_CHAT_GET_CLASS (chat)->get_status_icon_name (chat);
	}

	return NULL;
}

gboolean
empathy_chat_is_group_chat (EmpathyChat *chat)
{
	g_return_val_if_fail (EMPATHY_IS_CHAT (chat), FALSE);

	if (EMPATHY_CHAT_GET_CLASS (chat)->is_group_chat) {
		return EMPATHY_CHAT_GET_CLASS (chat)->is_group_chat (chat);
	}

	return FALSE;
}

gboolean 
empathy_chat_is_connected (EmpathyChat *chat)
{
	EmpathyChatPriv *priv;

	g_return_val_if_fail (EMPATHY_IS_CHAT (chat), FALSE);

	priv = GET_PRIV (chat);

	return (priv->tp_chat != NULL);
}

void
empathy_chat_set_tp_chat (EmpathyChat   *chat,
			  EmpathyTpChat *tp_chat)
{
	EmpathyChatPriv *priv;
	GList           *messages, *l;

	g_return_if_fail (EMPATHY_IS_CHAT (chat));
	g_return_if_fail (EMPATHY_IS_TP_CHAT (tp_chat));

	priv = GET_PRIV (chat);

	if (tp_chat == priv->tp_chat) {
		return;
	}

	if (priv->tp_chat) {
		g_signal_handlers_disconnect_by_func (priv->tp_chat,
						      chat_message_received_cb,
						      chat);
		g_signal_handlers_disconnect_by_func (priv->tp_chat,
						      chat_send_error_cb,
						      chat);
		g_signal_handlers_disconnect_by_func (priv->tp_chat,
						      chat_destroy_cb,
						      chat);
		g_object_unref (priv->tp_chat);
	}

	g_free (priv->id);
	priv->tp_chat = g_object_ref (tp_chat);
	priv->id = g_strdup (empathy_tp_chat_get_id (tp_chat));

	if (priv->first_tp_chat) {
		chat_add_logs (chat);
		priv->first_tp_chat = FALSE;
	}

	g_signal_connect (tp_chat, "message-received",
			  G_CALLBACK (chat_message_received_cb),
			  chat);
	g_signal_connect (tp_chat, "send-error",
			  G_CALLBACK (chat_send_error_cb),
			  chat);
	g_signal_connect (tp_chat, "chat-state-changed",
			  G_CALLBACK (chat_state_changed_cb),
			  chat);
	g_signal_connect (tp_chat, "destroy",
			  G_CALLBACK (chat_destroy_cb),
			  chat);

	/* Get pending messages */
	empathy_tp_chat_set_acknowledge (tp_chat, TRUE);
	messages = empathy_tp_chat_get_pendings (tp_chat);
	for (l = messages; l; l = l->next) {
		chat_message_received_cb (tp_chat, l->data, chat);
		g_object_unref (l->data);
	}
	g_list_free (messages);

	if (!priv->sensitive) {
		priv->sensitive = TRUE;
	}

	if (EMPATHY_CHAT_GET_CLASS (chat)->set_tp_chat) {
		EMPATHY_CHAT_GET_CLASS (chat)->set_tp_chat (chat, tp_chat);
	}

}

const gchar *
empathy_chat_get_id (EmpathyChat *chat)
{
	EmpathyChatPriv *priv;

	priv = GET_PRIV (chat);

	return priv->id;
}

void
empathy_chat_present(EmpathyChat *chat)
{
	FamaWindow *window;
	g_return_if_fail (EMPATHY_IS_CHAT (chat));
	/** priv->window is filled at here*/
	window = empathy_chat_appendwindow(chat);

	window_set_current(window);
	window_draw_all_titlebars();
}

FamaWindow *
empathy_chat_appendwindow(EmpathyChat *chat)
{
	EmpathyChatPriv *priv;
	const gchar     *title;
	FamaWindow      *window = 0;
	wchar_t         *wtitle;

	g_return_val_if_fail (EMPATHY_IS_CHAT (chat), NULL);

	priv = GET_PRIV (chat);
	title = empathy_chat_get_name(chat);

	if (priv->window == NULL) {
		window = window_new(WindowTypeConversation);
		if (!window)
			return NULL;
		/** unref it when closing a window*/
		priv->window = window;
		window->empathychat = chat;
		g_object_ref(window->empathychat);
		wtitle = g_new(wchar_t, strlen(title) + 1);
		utf8_to_wchar(title, wtitle, strlen(title));
		
		window_set_title(window, wtitle);
		g_free(wtitle);
	}
	return priv->window;
}

gboolean
empathy_chat_should_play_sound (EmpathyChat *chat)
{
	gboolean          play = TRUE;
	return play;
}

gboolean
empathy_chat_should_highlight_nick (EmpathyMessage *message)
{
	EmpathyContact *contact;
	const gchar   *msg, *to;
	gchar         *cf_msg, *cf_to;
	gchar         *ch;
	gboolean       ret_val;

	g_return_val_if_fail (EMPATHY_IS_MESSAGE (message), FALSE);

	empathy_debug (DEBUG_DOMAIN, "Highlighting nickname");

	ret_val = FALSE;

	msg = empathy_message_get_body (message);
	if (!msg) {
		return FALSE;
	}

	contact = empathy_message_get_receiver (message);
	if (!contact || !empathy_contact_is_user (contact)) {
		return FALSE;
	}

	to = empathy_contact_get_name (contact);
	if (!to) {
		return FALSE;
	}

	cf_msg = g_utf8_casefold (msg, -1);
	cf_to = g_utf8_casefold (to, -1);

	ch = strstr (cf_msg, cf_to);
	if (ch == NULL) {
		goto finished;
	}

	if (ch != cf_msg) {
		/* Not first in the message */
		if ((*(ch - 1) != ' ') &&
		    (*(ch - 1) != ',') &&
		    (*(ch - 1) != '.')) {
			goto finished;
		}
	}

	ch = ch + strlen (cf_to);
	if (ch >= cf_msg + strlen (cf_msg)) {
		ret_val = TRUE;
		goto finished;
	}

	if ((*ch == ' ') ||
	    (*ch == ',') ||
	    (*ch == '.') ||
	    (*ch == ':')) {
		ret_val = TRUE;
		goto finished;
	}

finished:
	g_free (cf_msg);
	g_free (cf_to);

	return ret_val;
}

void
empathy_message_add_text_message(EmpathyChat   *chat, EmpathyMessage  *msg,  
			 FamaEmapthyMessageType type)
{
	ColorSettings *c;
	FamaWindow *win;
	EmpathyContact     *sender;
	const gchar        *name;

	const gchar *contents, *time, *account, *contact;
	wchar_t *contents_w, *uri_w, *title;
	gint title_len, attr;

	g_return_if_fail (EMPATHY_IS_MESSAGE (msg));

	contents = empathy_message_get_body (msg);
	if (!contents) {
		return;
	}
	sender = empathy_message_get_sender (msg);
	name = empathy_contact_get_name (sender);

	c = color_get();

	if ((win = window_find_empathychat(chat)) == NULL) {
		g_warning("Message on non-existant channel!");
		return;
	}

	time = clock_get_time();
	contents_w = g_new(wchar_t, strlen(contents) + 1);
	uri_w = g_new(wchar_t, strlen(name) + 1);
	utf8_to_wchar(contents, contents_w, strlen(contents));
	utf8_to_wchar(name, uri_w, strlen(name));

	title_len = wcslen(uri_w) + strlen(time) + 5;
	title = g_new(wchar_t, title_len);
	swprintf(title, title_len - 1, L"[%s] %ls", time, uri_w);

	if (type == EmpathyMessageSent)
		attr = c->outgoing_message;
	else if (type == EmpathyMessageReceived)
		attr = c->incoming_message;
	else if (type == EmpathyMessageAutoReply)
		attr = c->incoming_automsg;
	else
		attr = COLOR_PAIR(1);

	window_add_message(win, title, A_BOLD | attr, contents_w);

	if (get_logging() == TRUE)
		write_to_log(account, contact, name, contents, time);

	g_free(title);
	g_free(contents_w);
	g_free(uri_w);
}

