#ifndef __EMPATHY_CHAT_H__
#define __EMPATHY_CHAT_H__

#include <glib-object.h>

#include <libempathy/empathy-contact.h>
#include <libempathy/empathy-message.h>
#include <libempathy/empathy-tp-chat.h>

G_BEGIN_DECLS

#define EMPATHY_TYPE_CHAT         (empathy_chat_get_type ())
#define EMPATHY_CHAT(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), EMPATHY_TYPE_CHAT, EmpathyChat))
#define EMPATHY_CHAT_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST ((k), EMPATHY_TYPE_CHAT, EmpathyChatClass))
#define EMPATHY_IS_CHAT(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), EMPATHY_TYPE_CHAT))
#define EMPATHY_IS_CHAT_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), EMPATHY_TYPE_CHAT))
#define EMPATHY_CHAT_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), EMPATHY_TYPE_CHAT, EmpathyChatClass))

typedef struct _EmpathyChat       EmpathyChat;
typedef struct _EmpathyChatClass  EmpathyChatClass;
typedef struct _EmpathyChatPriv   EmpathyChatPriv;

struct _EmpathyChat {
	GObject          parent;

	/* Protected */
	EmpathyTpChat   *tp_chat;
	gboolean         is_first_char;
	McAccount       *account;
};

struct _EmpathyChatClass {
	GObjectClass parent;

	/* VTable */
	const gchar *    (*get_name)            (EmpathyChat    *chat);
	gchar *          (*get_tooltip)         (EmpathyChat    *chat);
	const gchar *    (*get_status_icon_name)(EmpathyChat    *chat);
	gboolean         (*is_group_chat)       (EmpathyChat    *chat);
	void             (*set_tp_chat)         (EmpathyChat    *chat,
						 EmpathyTpChat *tp_chat);
};

GType             empathy_chat_get_type              (void);
void              empathy_chat_clear                 (EmpathyChat       *chat);
const gchar *     empathy_chat_get_name              (EmpathyChat       *chat);
gchar *           empathy_chat_get_tooltip           (EmpathyChat       *chat);
const gchar *     empathy_chat_get_status_icon_name  (EmpathyChat       *chat);
gboolean          empathy_chat_is_group_chat         (EmpathyChat       *chat);
gboolean          empathy_chat_is_connected          (EmpathyChat       *chat);
void              empathy_chat_set_tp_chat           (EmpathyChat       *chat,
						     EmpathyTpChat    *tp_chat);
const gchar *     empathy_chat_get_id                (EmpathyChat       *chat);

/* For spell checker dialog to correct the misspelled word. */
gboolean          empathy_chat_get_is_command        (const gchar      *str);

gboolean          empathy_chat_should_play_sound     (EmpathyChat       *chat);
gboolean          empathy_chat_should_highlight_nick (EmpathyMessage    *message);
void              empathy_chat_send                  (EmpathyChat       *chat,
						      const gchar       *msg);

G_END_DECLS

#endif /* __EMPATHY_CHAT_H__ */
