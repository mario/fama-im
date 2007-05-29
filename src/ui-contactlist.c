#include "common.h"
#include <string.h>

/* The base of how the contact-list is sorted
 * See TpaContactPresence.
 */
const gint statusrank[8] = { 6, 1, 2, 3, 4, 5 };

GPtrArray *contactlist = NULL;
GHashTable *contactlist_table = NULL;

WINDOW *clistwin = NULL;
PANEL *clistpanel = NULL;

gint contactlist_width = 30;
gint list_offset = 0, list_marked = 0;

void
contactlist_init()
{
	if (!contactlist)
		contactlist = g_ptr_array_new();

	if (!contactlist_table)
		contactlist_table =
			g_hash_table_new(g_direct_hash, g_direct_equal);
}

FamaContactListItem *
contactlist_get_selected()
{
	if (contactlist->len < 1)
		return NULL;

	return g_ptr_array_index(contactlist, list_marked);
}

gint
sort_compare_func(gconstpointer a, gconstpointer b)
{
	TpaContactPresence a_pres, b_pres;

	a_pres = tpa_contact_base_get_presence(TPA_CONTACT_BASE
					       ((*(FamaContactListItem **) a)->
						contact));
	b_pres = tpa_contact_base_get_presence(TPA_CONTACT_BASE
					       ((*(FamaContactListItem **) b)->
						contact));

	g_assert(a_pres > 0 && b_pres > 0);

	if (statusrank[a_pres - 1] < statusrank[b_pres - 1])
		return -1;

	return 1;
}

void
contactlist_sort()
{
	g_ptr_array_sort(contactlist, sort_compare_func);
}

void
contactlist_draw()
{
	FamaContactListItem *item;
	int a, i, str_len, str_width;

	if (clistwin == NULL) {
		clistwin = newwin(get_max_y() - 4,
				  contactlist_width - 3,
				  2, get_max_x() - contactlist_width + 2);
		clistpanel = new_panel(clistwin);

		g_assert(clistwin != NULL && clistpanel != NULL);
	}

	/*
	 * Clear the window 
	 */
	werase(clistwin);

	for (a = list_offset, i = 0;
	     a < contactlist->len && i < (get_max_y() - 4); i++, a++) {
		item = g_ptr_array_index(contactlist, a);
		g_assert(item != NULL);

		/*
		 * If we are at the selected item then
		 * * add reversed colors
		 */
		if (i == (list_marked - list_offset) &&
		    focus_get() == FocusContactList)
			item->attr |= A_REVERSE;

		wattron(clistwin, item->attr);

		/*
		 * If colors are reversed or underlined,
		 * * then fill the whole line to give a better effect.
		 */
		if ((item->attr & A_REVERSE) || (item->attr & A_UNDERLINE))
			mvwhline(clistwin, i, 0, ' ', contactlist_width - 3);


		/*
		 * Make sure we don't print outside the window
		 */
		for (str_len = 0, str_width = 0;
		     str_width <= contactlist_width - 3 &&
		     item->text[str_len] != L'\0'; str_len++)
			str_width += wcwidth(item->text[str_len]);

		mvwaddnwstr(clistwin, i, 0, item->text, str_len);
		wattrset(clistwin, A_NORMAL);

		if (i == (list_marked - list_offset) &&
		    focus_get() == FocusContactList)
			item->attr &= ~A_REVERSE;
	}

	update_panels();
	doupdate();
}

void
contactlist_scroll(gint m)
{
	gint rows = contactlist->len;

	list_marked += m;

	if (list_marked >= rows - 1)
		list_marked = rows - 1;
	if (list_marked < 0)
		list_marked = 0;

	while (list_marked >= (list_offset + get_max_y() - 4))
		list_offset++;

	if (list_marked < list_offset)
		list_offset = list_marked;

	contactlist_draw();
}

void
contactlist_destroy()
{
	werase(clistwin);
	update_panels();
	doupdate();
	del_panel(clistpanel);
	delwin(clistwin);
	clistpanel = NULL;
	clistwin = NULL;
}

void
contactlist_set_width(gint a)
{
	contactlist_width = a;
}

gint
contactlist_get_width()
{
	return contactlist_width;
}

guint
contactlist_presence_to_attr(TpaContactPresence p)
{
	ColorSettings *c = color_get();

	switch (p) {
	case TPA_PRESENCE_OFFLINE:
	case TPA_PRESENCE_HIDDEN:
		return c->status_offline;
	case TPA_PRESENCE_AVAILABLE:
		return c->status_available;
	case TPA_PRESENCE_AWAY:
		return c->status_away;
	case TPA_PRESENCE_XA:
		return c->status_idle;
	case TPA_PRESENCE_BUSY:
		return c->status_busy;
	default:
		break;
	}

	return c->status_other;
}

void
contactlist_alias_changed_cb(TpaContact * contact, gchar * alias)
{
}

void
contactlist_presence_updated_cb(TpaContact * contact,
				TpaContactPresence presence, gchar * message)
{
	int i;
	FamaContactListItem *a;
	const gchar *contact_alias, *contact_status;

	contact_alias = tpa_contact_base_get_alias(TPA_CONTACT_BASE(contact));
	contact_status =
		tpa_contact_base_get_presence_as_string(TPA_CONTACT_BASE
							(contact));

	for (i = 0; i < contactlist->len; i++) {
		if ((a = g_ptr_array_index(contactlist, i))->contact == contact) {
			a->attr = contactlist_presence_to_attr(presence);
			break;
		}
	}

	contactlist_sort();
	contactlist_draw();
}

void
contactlist_add_contact(TpaConnection * connection, TpaContact * contact)
{
	TpaContactPresence contact_presence;
	FamaContactListItem *a;
	const gchar *contact_alias;
	wchar_t *alias;

	a = g_new(FamaContactListItem, 1);

	g_signal_connect(G_OBJECT(contact), "presence-updated",
			 G_CALLBACK(contactlist_presence_updated_cb), NULL);

	g_signal_connect(G_OBJECT(contact), "alias-changed",
			 G_CALLBACK(contactlist_alias_changed_cb), NULL);

	contact_alias = tpa_contact_base_get_alias(TPA_CONTACT_BASE(contact));
	contact_presence =
		tpa_contact_base_get_presence(TPA_CONTACT_BASE(contact));

	alias = g_new(wchar_t, strlen(contact_alias) + 1);
	utf8_to_wchar(contact_alias, alias, strlen(contact_alias));

	a->contact = contact;
	a->connection = connection;
	a->text = alias;
	a->attr = contactlist_presence_to_attr(contact_presence);

	g_ptr_array_add(contactlist, a);
}

void
contactlist_authorization_requested_cb(TpaContactList * list,
				       TpaContact * contact)
{
	g_message("%s has requested your authorization",
		  tpa_contact_base_get_alias(TPA_CONTACT_BASE(contact)));
}

void
contactlist_subscription_accepted_cb(TpaContactList * list,
				     TpaContact * contact)
{
	TpaConnection *conn;

	conn = g_hash_table_lookup(contactlist_table, list);
	contactlist_add_contact(conn, contact);
}

void
contactlist_remove_from_connection(TpaConnection * conn)
{
	FamaContactListItem *item;
	gint i;

	for (i = 0; i < contactlist->len; i++) {
		item = g_ptr_array_index(contactlist, i);

		if (item->connection == conn) {
			g_ptr_array_remove_index(contactlist, i);
			g_free(item->text);
			g_free(item);
			i--;
		}
	}
}

void
contactlist_reload_from_server(TpaConnection * conn)
{
	TpaContactList *list;
	GPtrArray *contacts;
	TpaContact *contact;
	gint i;

	contactlist_remove_from_connection(conn);

	list = tpa_connection_get_contactlist(conn);
	contacts = tpa_contact_list_get_known(list);

	g_hash_table_insert(contactlist_table, list, conn);

	g_signal_connect(G_OBJECT(list), "authorization-requested",
			 G_CALLBACK
			 (contactlist_authorization_requested_cb), NULL);

	g_signal_connect(G_OBJECT(list), "subscription-accepted",
			 G_CALLBACK
			 (contactlist_subscription_accepted_cb), NULL);


	for (i = 0; i < contacts->len; i++) {
		contact = g_ptr_array_index(contacts, i);
		contactlist_add_contact(conn, contact);
	}

}
