#ifndef __EMPATHY_CONTACT_LIST_STORE_H__
#define __EMPATHY_CONTACT_LIST_STORE_H__

#include <libempathy/empathy-contact-list.h>
#include <libempathy/empathy-contact.h>

G_BEGIN_DECLS

/*
 * EmpathyContactListStoreSort
 */ 
typedef enum {
	EMPATHY_CONTACT_LIST_STORE_SORT_STATE,
	EMPATHY_CONTACT_LIST_STORE_SORT_NAME
} EmpathyContactListStoreSort;

/*
 * EmpathyContactListStore 
 */ 
#define EMPATHY_TYPE_CONTACT_LIST_STORE         (empathy_contact_list_store_get_type ())
#define EMPATHY_CONTACT_LIST_STORE(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), EMPATHY_TYPE_CONTACT_LIST_STORE, EmpathyContactListStore))
#define EMPATHY_CONTACT_LIST_STORE_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), EMPATHY_TYPE_CONTACT_LIST_STORE, EmpathyContactListStoreClass))
#define EMPATHY_IS_CONTACT_LIST_STORE(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), EMPATHY_TYPE_CONTACT_LIST_STORE))
#define EMPATHY_IS_CONTACT_LIST_STORE_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), EMPATHY_TYPE_CONTACT_LIST_STORE))
#define EMPATHY_CONTACT_LIST_STORE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), EMPATHY_TYPE_CONTACT_LIST_STORE, EmpathyContactListStoreClass))

typedef struct _EmpathyContactListStore      EmpathyContactListStore;
typedef struct _EmpathyContactListStoreClass EmpathyContactListStoreClass;
typedef struct _EmpathyContactListStorePriv  EmpathyContactListStorePriv;

enum {
	COL_ICON_STATUS,
	COL_PIXBUF_AVATAR,
	COL_PIXBUF_AVATAR_VISIBLE,
	COL_NAME,
	COL_STATUS,
	COL_STATUS_VISIBLE,
	COL_CONTACT,
	COL_IS_GROUP,
	COL_IS_ACTIVE,
	COL_IS_ONLINE,
	COL_IS_SEPARATOR,
	COL_CAN_VOIP,
	COL_COUNT
} EmpathyContactListStoreCol;

struct _EmpathyContactListStore {
	GObject		parent;
};

struct _EmpathyContactListStoreClass {
	GObjectClass 	parent;
};
typedef GList *            (*EmpathyContactGroupsFunc)                   (EmpathyContact              *contact,
									 gpointer                    user_data);

GType                      empathy_contact_list_store_get_type           (void) G_GNUC_CONST;
EmpathyContactListStore *   empathy_contact_list_store_new                (EmpathyContactList         *list_iface);
EmpathyContactList *       empathy_contact_list_store_get_list_iface     (EmpathyContactListStore     *store);
gboolean                   empathy_contact_list_store_get_show_offline   (EmpathyContactListStore     *store);
void                       empathy_contact_list_store_set_show_offline   (EmpathyContactListStore     *store,
									 gboolean                    show_offline);
gboolean                   empathy_contact_list_store_get_show_avatars   (EmpathyContactListStore     *store);
void                       empathy_contact_list_store_set_show_avatars   (EmpathyContactListStore     *store,
									 gboolean                    show_avatars);
gboolean                   empathy_contact_list_store_get_is_compact     (EmpathyContactListStore     *store);
void                       empathy_contact_list_store_set_is_compact     (EmpathyContactListStore     *store,
									 gboolean                    is_compact);
EmpathyContactListStoreSort empathy_contact_list_store_get_sort_criterium (EmpathyContactListStore     *store);
void                       empathy_contact_list_store_set_sort_criterium (EmpathyContactListStore     *store,
									 EmpathyContactListStoreSort  sort_criterium);
void empathy_contactlistwin_draw(EmpathyContactListStore *store);

void empathy_contactlistwin_set_width(gint a);

gint empathy_contactlistwin_get_width();

void empathy_contactlistwin_destroy();

gint empathy_contactlistwin_count_rows(EmpathyContactListStore *store);

void empathy_contactlistwin_scroll(EmpathyContactListStore *store, gint m);

EmpathyContact *empathy_contact_list_store_get_selected();

void empathy_contact_list_launch_channel(EmpathyContact *contact);

gint empathy_contactlistwin_get_current_selected_index();

extern EmpathyContactListStore *list_store;

G_END_DECLS

#endif /* __EMPATHY_CONTACT_LIST_STORE_H__ */

