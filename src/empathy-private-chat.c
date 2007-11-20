#include <string.h>

#include <glib/gi18n.h>

#include <libempathy/empathy-debug.h>
#include <libempathy/empathy-tp-chat.h>
#include <libempathy/empathy-tp-contact-list.h>
#include <libempathy/empathy-contact-factory.h>

#include "empathy-private-chat.h"
#include "empathy-chat.h"
#include "common.h"

#define DEBUG_DOMAIN "PrivateChat"

#define GET_PRIV(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EMPATHY_TYPE_PRIVATE_CHAT, EmpathyPrivateChatPriv))

struct _EmpathyPrivateChatPriv {   
	EmpathyContactFactory *factory;
	EmpathyContact        *contact;
	gchar                 *name;
	gboolean               is_online;
};

static void           empathy_private_chat_class_init            (EmpathyPrivateChatClass *klass);
static void           empathy_private_chat_init                  (EmpathyPrivateChat      *chat);
static void           private_chat_finalize                     (GObject                *object);
static void           private_chat_contact_presence_updated_cb  (EmpathyContact          *contact,
								 GParamSpec             *param,
								 EmpathyPrivateChat      *chat);
static void           private_chat_contact_updated_cb           (EmpathyContact          *contact,
								 GParamSpec             *param,
								 EmpathyPrivateChat      *chat);
static const gchar *  private_chat_get_name                     (EmpathyChat             *chat);
static gchar *        private_chat_get_tooltip                  (EmpathyChat             *chat);

G_DEFINE_TYPE (EmpathyPrivateChat, empathy_private_chat, EMPATHY_TYPE_CHAT);

static void
empathy_private_chat_class_init (EmpathyPrivateChatClass *klass)
{
	GObjectClass    *object_class = G_OBJECT_CLASS (klass);
	EmpathyChatClass *chat_class = EMPATHY_CHAT_CLASS (klass);

	object_class->finalize = private_chat_finalize;

	chat_class->get_name             = private_chat_get_name;
	chat_class->get_tooltip          = private_chat_get_tooltip;
	chat_class->set_tp_chat          = NULL;

	g_type_class_add_private (object_class, sizeof (EmpathyPrivateChatPriv));
}

static void
empathy_private_chat_init (EmpathyPrivateChat *chat)
{
	EmpathyPrivateChatPriv *priv;

	priv = GET_PRIV (chat);

	priv->is_online = FALSE;
}

static void
private_chat_finalize (GObject *object)
{
	EmpathyPrivateChat     *chat;
	EmpathyPrivateChatPriv *priv;
	
	chat = EMPATHY_PRIVATE_CHAT (object);
	priv = GET_PRIV (chat);

	g_signal_handlers_disconnect_by_func (priv->contact,
					      private_chat_contact_updated_cb,
					      chat);
	g_signal_handlers_disconnect_by_func (priv->contact,
					      private_chat_contact_presence_updated_cb,
					      chat);

	if (priv->contact) {
		g_object_unref (priv->contact);
	}
	if (priv->factory) {
		g_object_unref (priv->factory);
	}
	g_free (priv->name);

	G_OBJECT_CLASS (empathy_private_chat_parent_class)->finalize (object);
}

static void
private_chat_contact_presence_updated_cb (EmpathyContact     *contact,
					  GParamSpec        *param,
					  EmpathyPrivateChat *chat)
{
	EmpathyPrivateChatPriv *priv;

	priv = GET_PRIV (chat);

	fama_debug (DEBUG_DOMAIN, "Presence update for contact: %s",
		      empathy_contact_get_id (contact));

	if (!empathy_contact_is_online (contact)) {
		if (priv->is_online) {
			gchar *msg;

			msg = g_strdup_printf (_("%s went offline"),
					       empathy_contact_get_name (priv->contact));
//r			empathy_chat_view_append_event (EMPATHY_CHAT (chat)->view, msg);
			g_free (msg);
		}

		priv->is_online = FALSE;

		g_signal_emit_by_name (chat, "composing", FALSE);

	} else {
		if (!priv->is_online) {
			gchar *msg;

			msg = g_strdup_printf (_("%s has come online"),
					       empathy_contact_get_name (priv->contact));
//r			empathy_chat_view_append_event (EMPATHY_CHAT (chat)->view, msg);
			g_free (msg);
		}

		priv->is_online = TRUE;
	}

	g_signal_emit_by_name (chat, "status-changed");
}

static void
private_chat_contact_updated_cb (EmpathyContact     *contact,
				 GParamSpec        *param,
				 EmpathyPrivateChat *chat)
{
	EmpathyPrivateChatPriv *priv;

	priv = GET_PRIV (chat);

	if (strcmp (priv->name, empathy_contact_get_name (contact)) != 0) {
		g_free (priv->name);
		priv->name = g_strdup (empathy_contact_get_name (contact));
		g_signal_emit_by_name (chat, "name-changed", priv->name);
	}
}

static const gchar *
private_chat_get_name (EmpathyChat *chat)
{
	EmpathyPrivateChatPriv *priv;

	g_return_val_if_fail (EMPATHY_IS_PRIVATE_CHAT (chat), NULL);

	priv = GET_PRIV (chat);

	return priv->name;
}

static gchar *
private_chat_get_tooltip (EmpathyChat *chat)
{
	EmpathyPrivateChatPriv *priv;
	const gchar           *status;

	g_return_val_if_fail (EMPATHY_IS_PRIVATE_CHAT (chat), NULL);

	priv = GET_PRIV (chat);

	status = empathy_contact_get_status (priv->contact);

	return g_strdup_printf ("%s\n%s",
				empathy_contact_get_id (priv->contact),
				status);
}

EmpathyContact *
empathy_private_chat_get_contact (EmpathyPrivateChat *chat)
{
	EmpathyPrivateChatPriv *priv;

	g_return_val_if_fail (EMPATHY_IS_PRIVATE_CHAT (chat), NULL);

	priv = GET_PRIV (chat);

	return priv->contact;
}

static void
private_chat_setup (EmpathyPrivateChat *chat,
		    EmpathyContact     *contact,
		    EmpathyTpChat     *tp_chat)
{
	EmpathyPrivateChatPriv *priv;

	priv = GET_PRIV (chat);

	EMPATHY_CHAT (chat)->account = g_object_ref (empathy_contact_get_account (contact));
	priv->contact = g_object_ref (contact);
	priv->name = g_strdup (empathy_contact_get_name (contact));

	empathy_chat_set_tp_chat (EMPATHY_CHAT (chat), tp_chat);

	g_signal_connect (priv->contact, 
			  "notify::name",
			  G_CALLBACK (private_chat_contact_updated_cb),
			  chat);
	g_signal_connect (priv->contact, 
			  "notify::presence",
			  G_CALLBACK (private_chat_contact_presence_updated_cb),
			  chat);

	priv->is_online = empathy_contact_is_online (priv->contact);
}

EmpathyPrivateChat *
empathy_private_chat_new (McAccount *account,
			  TpChan    *tp_chan)
{
	EmpathyPrivateChat     *chat;
	EmpathyPrivateChatPriv *priv;
	EmpathyTpChat          *tp_chat;
	EmpathyContact         *contact;

	g_return_val_if_fail (MC_IS_ACCOUNT (account), NULL);
	g_return_val_if_fail (TELEPATHY_IS_CHAN (tp_chan), NULL);

	chat = g_object_new (EMPATHY_TYPE_PRIVATE_CHAT, NULL);
	priv = GET_PRIV (chat);

	priv->factory = empathy_contact_factory_new ();
	contact = empathy_contact_factory_get_from_handle (priv->factory,
							   account,
							   tp_chan->handle);

	tp_chat = empathy_tp_chat_new (account, tp_chan);
	private_chat_setup (chat, contact, tp_chat);

	g_object_unref (tp_chat);
	g_object_unref (contact);

	return chat;
}

EmpathyPrivateChat *
empathy_private_chat_new_with_contact (EmpathyContact *contact)
{
	EmpathyPrivateChat     *chat;
	EmpathyPrivateChatPriv *priv;
	EmpathyTpChat          *tp_chat;

	g_return_val_if_fail (EMPATHY_IS_CONTACT (contact), NULL);

	chat = g_object_new (EMPATHY_TYPE_PRIVATE_CHAT, NULL);

	priv = GET_PRIV (chat);
	priv->factory = empathy_contact_factory_new ();

	tp_chat = empathy_tp_chat_new_with_contact (contact);
	private_chat_setup (chat, contact, tp_chat);
	g_object_unref (tp_chat);

	return chat;
}

