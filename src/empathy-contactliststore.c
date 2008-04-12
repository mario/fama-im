#include "common.h"

#include <string.h>

#include <glib.h>

#include <libempathy/empathy-debug.h>
#include <libempathy/empathy-contact.h>
#include <libempathy/empathy-debug.h>
#include <libempathy/empathy-utils.h>
#include <libempathy/empathy-chatroom-manager.h>
#include <libempathy/empathy-chatroom.h>
#include <libempathy/empathy-contact-list.h>
#include <libempathy/empathy-contact-manager.h>

#include "empathy-contactliststore.h"
#include <glib/gprintf.h>

#define DEBUG_DOMAIN "ContactListStore"

/* Active users are those which have recently changed state
 * (e.g. online, offline or from normal to a busy state).
 */

/* Time user is shown as active */
#define ACTIVE_USER_SHOW_TIME 7000

/* Time after connecting which we wait before active users are enabled */
#define ACTIVE_USER_WAIT_TO_ENABLE_TIME 5000

#define GET_PRIV(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EMPATHY_TYPE_CONTACT_LIST_STORE, EmpathyContactListStorePriv))

#define MAXNAMELENGTH	128
#define MAXSTATUSLENGTH	32

WINDOW *em_clistwin = NULL;
PANEL *em_clistpanel = NULL;
gint em_contactlist_width = 30, em_list_marked = 0, em_list_offset = 0;

typedef struct _ContactListItem {
	gshort avatardata;     			/* Avatar pixbuf */
	gboolean avatervisible;			/* Avatar pixbuf visible */
	gchar contactname[MAXNAMELENGTH];	/* Name */
	gchar statusname[MAXSTATUSLENGTH];	/* Status string */
	gboolean showstatus;			/* Show status */
	EmpathyContact *emcontact; 		/* Contact type */
	gboolean is_group;			/* Is group */
	gboolean is_active;			/* Is active */
	gboolean is_online;			/* Is online */
	gboolean is_canvoip;			/* Can VoIP */
	gint attr;				/* clist win show attribute*/
}CONTACTLISTITEM;

struct _EmpathyContactListStorePriv {
	GList			   *contactitemlist;
	EmpathyContactList         *list;
	gboolean                    show_offline;
	gboolean                    show_avatars;
	gboolean                    is_compact;
	gboolean                    show_active;
	EmpathyContactListStoreSort sort_criterium;
	guint                       inhibit_active;

	EmpathyContactGroupsFunc    get_contact_groups;
	gpointer                    get_contact_groups_data;
};

typedef struct {
	CONTACTLISTITEM  *iter;	//group iter in the list
	const gchar *name;
	gboolean     found;
} FindGroup;

typedef struct {
	EmpathyContact *contact;
	gboolean       found;
	GList         *iters;
} FindContact;

typedef struct {
	EmpathyContactListStore *store;
	EmpathyContact          *contact;
	gboolean                remove;
} ShowActiveData;

static void contact_list_store_members_changed_cb(EmpathyContactList *list_iface,
	EmpathyContact          *contact,
	EmpathyContact          *actor,
	guint                    reason,
	gchar                   *message,
	gboolean                 is_member,
	EmpathyContactListStore *store);
static void contact_list_store_setup (EmpathyContactListStore *store);

static void contact_list_store_contact_updated_cb (EmpathyContact *contact,
	GParamSpec              *param,
	EmpathyContactListStore *store);

static void contact_list_store_add_contact (EmpathyContactListStore *store,
	EmpathyContact          *contact);

static void contact_list_store_remove_contact (EmpathyContactListStore *store,
	EmpathyContact          *contact);

static void contact_list_store_contact_update (EmpathyContactListStore *store,
	EmpathyContact          *contact);

static GList *contact_list_store_find_contact (EmpathyContactListStore *store,
	EmpathyContact          *contact);

static void contact_list_store_contact_set_active (EmpathyContactListStore *store,
        EmpathyContact          *contact,
        gboolean                active,
        gboolean                set_changed);

static void empathy_contact_list_store_class_init (gpointer klass,
                      gpointer g_class_data);

static void empathy_contact_list_store_instance_init(GTypeInstance   *instance,
                         gpointer         g_class);

static void contact_list_store_finalize (GObject *object);
static void contact_list_store_get_property (GObject    *object,
	guint       param_id,
	GValue     *value,
	GParamSpec *pspec);
static void contact_list_store_set_property (GObject      *object,
	guint         param_id,
	const GValue *value,
	GParamSpec   *pspec);

static gboolean contact_list_store_contact_active_cb (ShowActiveData *data);

static ShowActiveData * contact_list_store_contact_active_new (EmpathyContactListStore *store,
                                       EmpathyContact          *contact,
                                       gboolean                remove);

static void contact_list_store_contact_active_free (ShowActiveData *data);

static void contact_list_store_groups_changed_cb (EmpathyContactList      *list_iface,
	EmpathyContact          *contact,
	gchar                   *group,
	gboolean                 is_member,
	EmpathyContactListStore *store);

static CONTACTLISTITEM *contact_list_store_get_group (EmpathyContactListStore *store,
	const gchar            *name, gboolean *create);

static gboolean contact_list_free_foreach(gpointer data, gpointer user_data);

static guint empathy_contactlist_presence_to_attr(EmpathyContact *contact);

static guint empathy_contactlist_get_groupattr();

enum {
	PROP_0,
	PROP_SHOW_OFFLINE,
	PROP_SHOW_AVATARS,
	PROP_IS_COMPACT,
	PROP_SORT_CRITERIUM
};


GType empathy_contact_list_store_get_type(void)
{
	static GType type = 0;
	if (type == 0) {
		static const GTypeInfo info = {
			sizeof (EmpathyContactListStoreClass),
			NULL,   /* base_init */
			NULL,   /* base_finalize */
			empathy_contact_list_store_class_init,   /* class_init */
			NULL,   /* class_finalize */
			NULL,   /* class_data */
			sizeof (EmpathyContactListStore),
			0,      /* n_preallocs */
			empathy_contact_list_store_instance_init    /* instance_init */
		};
		type = g_type_register_static (G_TYPE_OBJECT,
			"EmpathyContactListStoreType",
			&info, 0);
	}
	return type;
}

static void
empathy_contact_list_store_instance_init(GTypeInstance   *instance,
                         gpointer         g_class)
{
	//EmpathyContactListStoreClass *self = (EmpathyContactListStoreClass *)instance;
}

static void
empathy_contact_list_store_class_init(gpointer klass,
                      gpointer g_class_data)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = contact_list_store_finalize;
	object_class->get_property = contact_list_store_get_property;
	object_class->set_property = contact_list_store_set_property;

	g_object_class_install_property (object_class,
		PROP_SHOW_OFFLINE,
		g_param_spec_boolean ("show-offline",
			"Show Offline",
			"Whether contact list should display "
			"offline contacts",
			TRUE,//FALSE,
			G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
		PROP_SHOW_AVATARS,
		g_param_spec_boolean ("show-avatars",
			"Show Avatars",
			"Whether contact list should display "
			"avatars for contacts",
			TRUE,
			G_PARAM_READWRITE));
	g_object_class_install_property (object_class,
		PROP_IS_COMPACT,
		g_param_spec_boolean ("is-compact",
			"Is Compact",
			"Whether the contact list is in compact mode or not",
			FALSE,
			G_PARAM_READWRITE));

	g_type_class_add_private (object_class, sizeof (EmpathyContactListStorePriv));
}

EmpathyContactListStore *
empathy_contact_list_store_new (EmpathyContactList *list_iface)
{      
	EmpathyContactListStore     *store;
	EmpathyContactListStorePriv *priv;
	GList                      *contacts, *l;

	g_return_val_if_fail (EMPATHY_IS_CONTACT_LIST (list_iface), NULL);

	store = g_object_new (EMPATHY_TYPE_CONTACT_LIST_STORE, NULL);

	priv = GET_PRIV(store);

	contact_list_store_setup (store);    
	priv->list = g_object_ref (list_iface);

	/* Signal connection. */
	g_signal_connect (priv->list,
		"members-changed",
		G_CALLBACK (contact_list_store_members_changed_cb),
		store);
  	g_signal_connect (priv->list,
		"groups-changed", 
		G_CALLBACK (contact_list_store_groups_changed_cb),
		store);

	/* Add contacts already created. */   
	contacts = empathy_contact_list_get_members (priv->list);
	for (l = contacts; l; l = l->next) {
		contact_list_store_members_changed_cb (priv->list, l->data,
			NULL, 0, NULL,
			TRUE,
			store);

		g_object_unref (l->data);
	}
	g_list_free (contacts);

	return store;
}

static void
contact_list_store_members_changed_cb (EmpathyContactList      *list_iface,
	EmpathyContact          *contact,
	EmpathyContact          *actor,
	guint                    reason,
	gchar                   *message,
	gboolean                 is_member,
	EmpathyContactListStore *store)
{
	EmpathyContactListStorePriv *priv;

	priv = GET_PRIV(store);

	fama_debug(DEBUG_DOMAIN,
		"Contact %s (%d) %s",
		empathy_contact_get_id (contact),
		empathy_contact_get_handle (contact),
		is_member ? "added" : "removed");

	if (is_member) {
		g_signal_connect (contact, "notify::presence",
		G_CALLBACK (contact_list_store_contact_updated_cb),
		store);
	g_signal_connect (contact, "notify::name",
		G_CALLBACK (contact_list_store_contact_updated_cb),
		store);
	g_signal_connect (contact, "notify::avatar",
		G_CALLBACK (contact_list_store_contact_updated_cb),
		store);
	g_signal_connect (contact, "notify::capabilities",
		G_CALLBACK (contact_list_store_contact_updated_cb),
		store);

		contact_list_store_add_contact (store, contact);
	} else {
		g_signal_handlers_disconnect_by_func (contact,
			G_CALLBACK (contact_list_store_contact_updated_cb),
			store);

		contact_list_store_remove_contact (store, contact);
	}
}

static void
contact_list_store_setup (EmpathyContactListStore *store)
{
	EmpathyContactListStorePriv *priv;
	priv = GET_PRIV(store);

	priv->show_offline = TRUE;
	priv->sort_criterium = EMPATHY_CONTACT_LIST_STORE_SORT_NAME;
	empathy_contact_list_store_set_sort_criterium (store, priv->sort_criterium);
}

static void
contact_list_store_contact_updated_cb (EmpathyContact          *contact,
	GParamSpec              *param,
	EmpathyContactListStore *store)
{
	fama_debug (DEBUG_DOMAIN,
		"Contact:'%s' updated, checking roster is in sync...",
		empathy_contact_get_name (contact));

	contact_list_store_contact_update (store, contact);
}

static void
contact_list_store_add_contact (EmpathyContactListStore *store,
	EmpathyContact          *contact)
{
	EmpathyContactListStorePriv *priv;
	CONTACTLISTITEM		    *clitem;
	GList                       *groups, *l;

	priv = GET_PRIV(store);

	if (!priv->show_offline && !empathy_contact_is_online (contact)) {
		return;
	}

	groups = empathy_contact_list_get_groups (priv->list, contact);

	/* If no groups just add it at the top level. */
	if (!groups) {
		clitem = g_new0(CONTACTLISTITEM, 1);	
		g_stpcpy(clitem->contactname, empathy_contact_get_name (contact));	/* Name */
		clitem->emcontact = contact; 		/* Contact type */
		clitem->is_group = FALSE;			/* Is group */
		clitem->is_canvoip = empathy_contact_can_voip(contact);
		priv->contactitemlist = g_list_append(priv->contactitemlist, clitem);
	}

	/* Else add to each group. */
	for (l = groups; l; l = l->next) {
		CONTACTLISTITEM* iter_group;
		gint groupiteridx;
		iter_group = contact_list_store_get_group(store, l->data, NULL);

		clitem = g_new0(CONTACTLISTITEM, 1);	
		g_stpcpy(clitem->contactname, empathy_contact_get_name (contact));	/* Name */
		clitem->emcontact = contact; 		/* Contact type */
		clitem->is_group = FALSE;			/* Is group */
		clitem->is_canvoip = empathy_contact_can_voip(contact);
	
	
		groupiteridx = g_list_index(priv->contactitemlist, iter_group);
		g_assert(groupiteridx >= 0);
		priv->contactitemlist = g_list_insert(priv->contactitemlist, clitem, groupiteridx + 1);
		g_free (l->data);
	}
	g_list_free (groups);

	contact_list_store_contact_update (store, contact);

}

static guint
empathy_contactlist_get_groupattr()
{
	ColorSettings *c = color_get();
	return c->status_group;
}

static guint
empathy_contactlist_presence_to_attr(EmpathyContact *contact)
{
	ColorSettings *c = color_get();
	McPresence state;

	state = empathy_contact_get_presence(contact);
	switch (state) {
	case MC_PRESENCE_AVAILABLE:
		return c->status_available;
	case MC_PRESENCE_DO_NOT_DISTURB:
		return c->status_busy;
	case MC_PRESENCE_AWAY:
	case MC_PRESENCE_EXTENDED_AWAY:
		return c->status_away;
	case MC_PRESENCE_HIDDEN:
	//	return _("Hidden");
	case MC_PRESENCE_OFFLINE:
	case MC_PRESENCE_UNSET:
		return c->status_offline;
	default:
		g_assert_not_reached ();
		break;
	}             
	return c->status_other;
}

static gboolean
contact_list_store_get_group_foreach(gpointer data,
	gpointer user_data)
{
	FindGroup *fg = (FindGroup *)user_data;
	CONTACTLISTITEM *clitem = (CONTACTLISTITEM *)data;
	
	g_assert(clitem);
	if (clitem->is_group && strcmp (clitem->contactname, fg->name) == 0) {
		fg->found = TRUE;
		fg->iter = clitem;
	}

	return fg->found;
}

static CONTACTLISTITEM*
contact_list_store_get_group (EmpathyContactListStore *store,
	const gchar            *name,
	gboolean               *created)
{
	EmpathyContactListStorePriv *priv;
	FindGroup                    fg;
	CONTACTLISTITEM		    *iter_group_to_set;
	CONTACTLISTITEM		    *clitem;

	priv = GET_PRIV (store);

	memset (&fg, 0, sizeof (fg));

	fg.name = name;

	g_list_foreach (priv->contactitemlist,
		(GFunc) contact_list_store_get_group_foreach,
		&fg);

	if (!fg.found) {
		if (created) {
			*created = TRUE;
		}

		clitem = g_new0(CONTACTLISTITEM, 1);	
		g_stpcpy(clitem->contactname, name);	/* Name */
		clitem->is_group = TRUE;		/* Is group */
		clitem->is_active = FALSE;
		priv->contactitemlist = g_list_append(priv->contactitemlist, clitem);
		clitem->attr = empathy_contactlist_get_groupattr();

		if (iter_group_to_set) {
			iter_group_to_set = clitem;
		}

	} else {
		if (created) {
			*created = FALSE;
		}

		if (iter_group_to_set) {
			iter_group_to_set = fg.iter;
		}
	}
	return iter_group_to_set;
}

static void
contact_list_store_remove_contact (EmpathyContactListStore *store,
	EmpathyContact          *contact)
{
	EmpathyContactListStorePriv *priv;
	GList                      *iters, *l;

	priv = GET_PRIV(store);

	iters = contact_list_store_find_contact (store, contact);
	if (!iters) {
		return;
	}

	/* Clean up model */
	for (l = iters; l; l = l->next) {
		CONTACTLISTITEM *clitem;
		GList *iter;
		guint pos;
		clitem = l->data;
		iter = g_list_find(priv->contactitemlist, l->data);
		g_assert(iter);
		clitem = iter->data;
		g_free(clitem);
		pos = g_list_position(priv->contactitemlist, iter);
		priv->contactitemlist = g_list_remove(priv->contactitemlist, iter->data);
	}

	//g_list_foreach (iters, (GFunc) contact_list_free_foreach, NULL);
	g_list_free(iters);
}

static gboolean
contact_list_free_foreach(gpointer data, gpointer user_data)
{
	CONTACTLISTITEM *clitem;
	clitem = (CONTACTLISTITEM *)data;
	
	g_free(clitem);
	return TRUE;
}

static void
contact_list_store_contact_update (EmpathyContactListStore *store,
	EmpathyContact          *contact)
{
	EmpathyContactListStorePriv *priv;
	ShowActiveData             *data;
	GList                      *iters, *l;
	gboolean                    in_list;
	gboolean                    should_be_in_list;
	gboolean                    was_online = TRUE;
	gboolean                    now_online = FALSE;
	gboolean                    set_model = FALSE;
	gboolean                    do_remove = FALSE;
	gboolean                    do_set_active = FALSE;
	gboolean                    do_set_refresh = FALSE;

	priv = GET_PRIV(store);

	iters = contact_list_store_find_contact (store, contact);
	if (!iters) {
		in_list = FALSE;
	} else {
		in_list = TRUE;
	}

	/* Get online state now. */
	now_online = empathy_contact_is_online (contact);

	if (priv->show_offline || now_online) {
		should_be_in_list = TRUE;
	} else {
		should_be_in_list = FALSE;
	}

	if (!in_list && !should_be_in_list) {
		/* Nothing to do. */
		fama_debug (DEBUG_DOMAIN,
			"Contact:'%s' in list:NO, should be:NO",
			empathy_contact_get_name (contact));

	//	g_list_foreach (iters, (GFunc) contact_list_free_foreach, NULL);
		g_list_free (iters);
		return;
	}
	else if (in_list && !should_be_in_list) {
		fama_debug (DEBUG_DOMAIN,
			"Contact:'%s' in list:YES, should be:NO",
			empathy_contact_get_name (contact));

		if (priv->show_active) {
			do_remove = TRUE;
			do_set_active = TRUE;
			do_set_refresh = TRUE;

			set_model = TRUE;
			fama_debug (DEBUG_DOMAIN, "Remove item (after timeout)");
		} else {
			fama_debug (DEBUG_DOMAIN, "Remove item (now)!");
			contact_list_store_remove_contact (store, contact);
		}
	}
	else if (!in_list && should_be_in_list) {
		fama_debug (DEBUG_DOMAIN,
			"Contact:'%s' in list:NO, should be:YES",
			empathy_contact_get_name (contact));

		contact_list_store_add_contact (store, contact);

		if (priv->show_active) {
			do_set_active = TRUE;

			fama_debug (DEBUG_DOMAIN, "Set active (contact added)");
		}
	} else {
		fama_debug (DEBUG_DOMAIN,
			"Contact:'%s' in list:YES, should be:YES",
			empathy_contact_get_name (contact));

		/* Get online state before. */
		if (iters && g_list_length (iters) > 0) {
			CONTACTLISTITEM *clitem = iters->data;
			was_online = clitem->is_online;
		}

		/* Is this really an update or an online/offline. */
		if (priv->show_active) {
			if (was_online != now_online) {
				do_set_active = TRUE;
				do_set_refresh = TRUE;

				fama_debug (DEBUG_DOMAIN, "Set active (contact updated %s)",
					was_online ? "online  -> offline" :
					"offline -> online");
			} else {
					/* Was TRUE for presence updates. */
				/* do_set_active = FALSE;  */
				do_set_refresh = TRUE;

				fama_debug (DEBUG_DOMAIN, "Set active (contact updated)");
			}
		}

		set_model = TRUE;
	}

	//g_list_foreach (iters, (GFunc)contact_list_free_foreach, NULL);
	g_list_free (iters);
	iters = contact_list_store_find_contact (store, contact);
	for (l = iters; l && set_model; l = l->next) {
		CONTACTLISTITEM *clitem;
		clitem = l->data;
		g_stpcpy(clitem->contactname, empathy_contact_get_name(contact));	/* Name */
		g_stpcpy(clitem->statusname, empathy_contact_get_status(contact));	/* Status string */
		clitem->showstatus = !priv->is_compact;			/* Show status */
		g_assert(clitem->emcontact == contact);	/* Contact type */
		clitem->is_group = FALSE;			/* Is group */
		clitem->is_online = now_online;			/* Is online */
		clitem->is_canvoip = empathy_contact_can_voip(contact);		/* Can VoIP */
		clitem->attr = empathy_contactlist_presence_to_attr(contact);	/* clist win show attribute*/
	}
	empathy_contactlistwin_draw(store);

	if (priv->show_active && do_set_active) {
		contact_list_store_contact_set_active (store, contact, do_set_active, do_set_refresh);

		if (do_set_active) {
			data = contact_list_store_contact_active_new (store, contact, do_remove);
			g_timeout_add (ACTIVE_USER_SHOW_TIME,
				(GSourceFunc) contact_list_store_contact_active_cb,
				data);
		}
	}

	/* FIXME: when someone goes online then offline quickly, the
	* first timeout sets the user to be inactive and the second
	* timeout removes the user from the contact list, really we
	* should remove the first timeout.
	*/
//	g_list_foreach (iters, (GFunc)contact_list_free_foreach, NULL);
	g_list_free (iters);
}

static gboolean
contact_list_store_find_contact_foreach(gpointer data,
	gpointer user_data)
{
	CONTACTLISTITEM *clitem = (CONTACTLISTITEM *)data;
	FindContact  *fc = (FindContact *)user_data;
	EmpathyContact *contact;
	
	contact = clitem->emcontact;

	if (!contact) {
		return FALSE;
	}
	g_object_ref(contact);
	if (!EMPATHY_IS_CONTACT(contact))
		g_assert(0);
	g_assert(fc->contact);
	if (empathy_contact_equal (contact, fc->contact)) {
		fc->found = TRUE;
		fc->iters = g_list_append(fc->iters, clitem);
	}
	g_object_unref (contact);

	return FALSE;
}

static GList *
contact_list_store_find_contact (EmpathyContactListStore *store,
	EmpathyContact          *contact)
{
	EmpathyContactListStorePriv *priv;
	GList                     *l = NULL;
	FindContact                fc;

	priv = GET_PRIV(store);

	memset (&fc, 0, sizeof (fc));

	fc.contact = contact;

	g_list_foreach(priv->contactitemlist, (GFunc)contact_list_store_find_contact_foreach,
		&fc);
	if (fc.found) {
		l = fc.iters;
	}

	return l;
}

static void
contact_list_store_contact_set_active (EmpathyContactListStore *store,
        EmpathyContact          *contact,
        gboolean                active,
        gboolean                set_changed)
{
        EmpathyContactListStorePriv *priv;
        GList                      *iters, *l;

	priv = GET_PRIV(store);

        iters = contact_list_store_find_contact (store, contact);
        for (l = iters; l; l = l->next) {

		fama_debug (DEBUG_DOMAIN, "Set item %s", active ? "active" : "inactive");
	}

	//g_list_foreach (iters, (GFunc)contact_list_free_foreach, NULL);
        g_list_free (iters);

}

static gboolean
contact_list_store_contact_active_cb (ShowActiveData *data)
{
        EmpathyContactListStorePriv *priv;

	priv = GET_PRIV(data->store);

        if (data->remove &&
            !priv->show_offline &&
            !empathy_contact_is_online (data->contact)) {
                empathy_debug (DEBUG_DOMAIN,
                              "Contact:'%s' active timeout, removing item",
                              empathy_contact_get_name (data->contact));
                contact_list_store_remove_contact (data->store, data->contact);
        }

        fama_debug (DEBUG_DOMAIN,
                      "Contact:'%s' no longer active",
                      empathy_contact_get_name (data->contact));

        contact_list_store_contact_set_active (data->store,
                                               data->contact,
                                               FALSE,
                                               TRUE);

        contact_list_store_contact_active_free (data);

        return FALSE;
}

static ShowActiveData *
contact_list_store_contact_active_new (EmpathyContactListStore *store,
                                       EmpathyContact          *contact,
                                       gboolean                remove)
{
        ShowActiveData *data;

        fama_debug (DEBUG_DOMAIN,
                      "Contact:'%s' now active, and %s be removed",
                      empathy_contact_get_name (contact),
                      remove ? "WILL" : "WILL NOT");

        data = g_slice_new0 (ShowActiveData);

        data->store = g_object_ref (store);
        data->contact = g_object_ref (contact);
        data->remove = remove;

        return data;
}

static void
contact_list_store_contact_active_free (ShowActiveData *data)
{
        g_object_unref (data->contact);
        g_object_unref (data->store);

        g_slice_free (ShowActiveData, data);
}

static void
contact_list_store_groups_changed_cb (EmpathyContactList      *list_iface,
	EmpathyContact          *contact,
	gchar                   *group,
	gboolean                 is_member,
	EmpathyContactListStore *store)
{
	EmpathyContactListStorePriv *priv;
	gboolean                     show_active;
	const gchar                 *contactstr;

	priv = GET_PRIV(store);

	empathy_debug (DEBUG_DOMAIN, "Updating groups for contact %s (%d)",
		empathy_contact_get_id (contact),
		empathy_contact_get_handle (contact));

	/* We do this to make sure the groups are correct, if not, we
	 * would have to check the groups already set up for each
	 * contact and then see what has been updated.
	 */
	show_active = priv->show_active;
	priv->show_active = FALSE;
	contactstr = empathy_contact_get_name(contact);
	contact_list_store_remove_contact (store, contact);
	contact_list_store_add_contact (store, contact);
	priv->show_active = show_active;
}

static void
contact_list_store_get_property (GObject    *object,
	guint       param_id,
	GValue     *value,
	GParamSpec *pspec)
{
	EmpathyContactListStorePriv *priv;

	priv = GET_PRIV (object);

	switch (param_id) {
	case PROP_SHOW_OFFLINE:
		g_value_set_boolean (value, priv->show_offline);
		break;
	case PROP_SHOW_AVATARS:
		g_value_set_boolean (value, priv->show_avatars);
		break;
	case PROP_IS_COMPACT:
		g_value_set_boolean (value, priv->is_compact);
		break;
	case PROP_SORT_CRITERIUM:
		g_value_set_enum (value, priv->sort_criterium);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	};
}

static void
contact_list_store_set_property (GObject      *object,
	guint         param_id,
	const GValue *value,
	GParamSpec   *pspec)
{
	EmpathyContactListStorePriv *priv;

	priv = GET_PRIV (object);

	switch (param_id) {
	case PROP_SHOW_OFFLINE:
		empathy_contact_list_store_set_show_offline (EMPATHY_CONTACT_LIST_STORE (object),
			g_value_get_boolean (value));
		break;
	case PROP_SHOW_AVATARS:
		empathy_contact_list_store_set_show_avatars (EMPATHY_CONTACT_LIST_STORE (object),
			g_value_get_boolean (value));
		break;
	case PROP_IS_COMPACT:
		empathy_contact_list_store_set_is_compact (EMPATHY_CONTACT_LIST_STORE (object),
			g_value_get_boolean (value));
		break;
	case PROP_SORT_CRITERIUM:
		empathy_contact_list_store_set_sort_criterium (EMPATHY_CONTACT_LIST_STORE (object),
			g_value_get_enum (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	};
}

static gboolean
contact_list_store_finalize_foreach (gpointer data,
	gpointer user_data)
{
	EmpathyContactListStore *store = user_data;
	EmpathyContact          *contact = NULL;
	CONTACTLISTITEM *clitem = (CONTACTLISTITEM *)data;

	contact = clitem->emcontact;

	if (contact) {
		g_signal_handlers_disconnect_by_func (contact,
			G_CALLBACK (contact_list_store_contact_updated_cb),
			store);
	}
	g_free(clitem);
	return FALSE;
}

static void
contact_list_store_finalize (GObject *object)
{
	EmpathyContactListStorePriv *priv;

	priv = GET_PRIV (object);

	g_list_foreach(priv->contactitemlist,(GFunc) contact_list_store_finalize_foreach,
		object);
	if (priv->list) {
		g_signal_handlers_disconnect_by_func (priv->list,
			G_CALLBACK (contact_list_store_members_changed_cb),
			object);
		g_signal_handlers_disconnect_by_func (priv->list,
			G_CALLBACK (contact_list_store_groups_changed_cb),
			object);
		g_object_unref (priv->list);
	}

	if (priv->inhibit_active) {
		g_source_remove (priv->inhibit_active);
	}

//	G_OBJECT_CLASS (empathy_contact_list_store_parent_class)->finalize (object);
}

void
empathy_contact_list_store_set_sort_criterium (EmpathyContactListStore     *store,
	EmpathyContactListStoreSort  sort_criterium)
{
	EmpathyContactListStorePriv *priv;

	g_return_if_fail (EMPATHY_IS_CONTACT_LIST_STORE (store));

	priv = GET_PRIV (store);

	priv->sort_criterium = sort_criterium;

	switch (sort_criterium) {
	case EMPATHY_CONTACT_LIST_STORE_SORT_STATE:
		break;

	case EMPATHY_CONTACT_LIST_STORE_SORT_NAME:
		break;
	}
}

gboolean
empathy_contact_list_store_get_show_offline (EmpathyContactListStore *store)
{
	EmpathyContactListStorePriv *priv;

	g_return_val_if_fail (EMPATHY_IS_CONTACT_LIST_STORE (store), FALSE);

	priv = GET_PRIV (store);

	return priv->show_offline;
}

void
empathy_contact_list_store_set_show_offline (EmpathyContactListStore *store,
	gboolean                show_offline)
{
	EmpathyContactListStorePriv *priv;
	GList                      *contacts, *l;
	gboolean                    show_active;

	g_return_if_fail (EMPATHY_IS_CONTACT_LIST_STORE (store));

	priv = GET_PRIV (store);

	priv->show_offline = show_offline;
	show_active = priv->show_active;

	/* Disable temporarily. */
	priv->show_active = FALSE;

	contacts = empathy_contact_list_get_members (priv->list);
	for (l = contacts; l; l = l->next) {
		contact_list_store_contact_update (store, l->data);

		g_object_unref (l->data);
	}
	g_list_free (contacts);

	/* Restore to original setting. */
	priv->show_active = show_active;
}

gboolean
empathy_contact_list_store_get_show_avatars (EmpathyContactListStore *store)
{
	EmpathyContactListStorePriv *priv;

	g_return_val_if_fail (EMPATHY_IS_CONTACT_LIST_STORE (store), TRUE);

	priv = GET_PRIV (store);

	return priv->show_avatars;
}

void
empathy_contact_list_store_set_show_avatars (EmpathyContactListStore *store,
	gboolean                show_avatars)
{
	EmpathyContactListStorePriv *priv;

	/** this function does nothing*/
	g_return_if_fail (EMPATHY_IS_CONTACT_LIST_STORE (store));

	priv = GET_PRIV (store);

	priv->show_avatars = show_avatars;
}

gboolean
empathy_contact_list_store_get_is_compact (EmpathyContactListStore *store)
{
	EmpathyContactListStorePriv *priv;

	g_return_val_if_fail (EMPATHY_IS_CONTACT_LIST_STORE (store), TRUE);

	priv = GET_PRIV (store);

	return priv->is_compact;
}

static gboolean
contact_list_store_update_list_mode_foreach(gpointer data,
	gpointer user_data)
{
	EmpathyContactListStorePriv *priv;
	gboolean                    show_avatar = FALSE;
	EmpathyContactListStore *store = (EmpathyContactListStore *)data;
	priv = GET_PRIV(store);

	if (priv->show_avatars && !priv->is_compact) {
		show_avatar = TRUE;
	}

	return FALSE;
}

void
empathy_contact_list_store_set_is_compact (EmpathyContactListStore *store,
	gboolean                is_compact)
{
	EmpathyContactListStorePriv *priv;

	g_return_if_fail (EMPATHY_IS_CONTACT_LIST_STORE (store));

	priv = GET_PRIV (store);

	priv->is_compact = is_compact;

	g_list_foreach(priv->contactitemlist, 
		(GFunc)contact_list_store_update_list_mode_foreach,
		store);
}

void
empathy_contactlistwin_draw(EmpathyContactListStore *store)
{
	gint i;
	gint clcnt;
	GList *l;
	gchar display[512] = {0};
	EmpathyContactListStorePriv *priv;

	priv = GET_PRIV (store);

	if (em_clistwin == NULL) {
		em_clistwin = newwin(get_max_y() - 4,
				  em_contactlist_width - 3,
				  2, get_max_x() - em_contactlist_width + 2);
		em_clistpanel = new_panel(em_clistwin);

		g_assert(em_clistwin != NULL && em_clistpanel != NULL);
	}

	werase(em_clistwin);
	clcnt = g_list_length(priv->contactitemlist);
	if (clcnt < 1) {
		update_panels();
		doupdate();
		return;
	}

	for (l = priv->contactitemlist, i = 0; l; l = l->next, i++) {
		CONTACTLISTITEM *clitem;
		wchar_t *wcontactname;
		clitem = l->data;
		if (i < em_list_offset || i > em_list_offset + get_max_y() - 4)
			continue;
		/*
		 * If we are at the selected item then
		 * * add reversed colors
		 */
		if (em_list_marked == i && focus_get() == FocusContactList)
			clitem->attr |= A_REVERSE;
		if (clitem->is_group) {
			g_sprintf(display, "   [%s] <->", clitem->contactname);
		} else {
			if(!clitem->showstatus)
				g_sprintf(display, "%s", clitem->contactname);
			else
				g_sprintf(display, "%s (%.4s)", clitem->contactname, clitem->statusname);
		}
			
		wattron(em_clistwin, clitem->attr);

		/*
		 * If colors are reversed or underlined,
		 * * then fill the whole line to give a better effect.
		 */

		wcontactname = g_new(wchar_t, strlen(display) + 1);
		utf8_to_wchar(display, wcontactname, strlen(display));

		mvwaddwstr_with_maxwidth(em_clistwin, i - em_list_offset, 0, wcontactname,
			 em_contactlist_width);
		wattrset(em_clistwin, A_NORMAL);

		if (em_list_marked == i && focus_get() == FocusContactList)
			clitem->attr &= ~A_REVERSE;
		g_free(wcontactname);
	}
	update_panels();
	doupdate();
}

void
empathy_contactlistwin_set_width(gint a)
{
	em_contactlist_width = a;
}

gint
empathy_contactlistwin_get_width()
{
	return em_contactlist_width;
}

void
empathy_contactlistwin_destroy()
{
	werase(em_clistwin);
	update_panels();
	doupdate();
	del_panel(em_clistpanel);
	delwin(em_clistwin);
	em_clistpanel = NULL;
	em_clistwin = NULL;
}

gint
empathy_contactlistwin_count_rows(EmpathyContactListStore *store)
{
	EmpathyContactListStorePriv *priv;

	priv = GET_PRIV (store);

	return g_list_length(priv->contactitemlist);
}

void
empathy_contactlistwin_scroll(EmpathyContactListStore *store, gint m)
{
	gint rows = empathy_contactlistwin_count_rows(store), 
		old_list_mark = em_list_marked;

	em_list_marked += m;

	if (em_list_marked >= rows - 1)
		em_list_marked = rows - 1;
	if (em_list_marked < 0)
		em_list_marked = 0;

	while (em_list_marked >= (em_list_offset + get_max_y() - 4))
		em_list_offset++;

	if (em_list_marked < em_list_offset)
		em_list_offset = em_list_marked;

	if (old_list_mark != em_list_marked)
		empathy_contactlistwin_draw(store);
}

EmpathyContact *
empathy_contact_list_store_get_selected(EmpathyContactListStore *store)
{
	EmpathyContactListStorePriv *priv;
	CONTACTLISTITEM             *clitem;

	priv = GET_PRIV(store);
	clitem = g_list_nth_data(priv->contactitemlist, 
		empathy_contactlistwin_get_current_selected_index());
	if (!clitem) {
		return NULL;
	}
	return clitem->emcontact;
}

void
empathy_contact_list_launch_channel(EmpathyContact *contact)
{
	MissionControl *mc;

	mc = empathy_mission_control_new ();
	mission_control_request_channel (mc,
		empathy_contact_get_account (contact),
		TP_IFACE_CHANNEL_TYPE_TEXT,
		empathy_contact_get_handle (contact),
		TP_HANDLE_TYPE_CONTACT,
		NULL, NULL);
	g_object_unref (mc);
}

gint 
empathy_contactlistwin_get_current_selected_index()
{
	return em_list_marked + em_list_offset;
}


