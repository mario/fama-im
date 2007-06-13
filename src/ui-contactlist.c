#include "common.h"
#include <string.h>

WINDOW *clistwin = NULL;
PANEL *clistpanel = NULL;
gint contactlist_width = 30, list_marked = 0, list_offset = 0;
GPtrArray *groups = NULL;

void
contactlist_init()
{
	if (!groups)
		groups = g_ptr_array_new();
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

void
contactlist_draw()
{
	FamaContactListGroup *group = NULL;
	FamaContactListItem *item = NULL;
	gint a, b, i, y;

	if (clistwin == NULL) {
		clistwin = newwin(get_max_y() - 4,
				  contactlist_width - 3,
				  2, get_max_x() - contactlist_width + 2);
		clistpanel = new_panel(clistwin);

		g_assert(clistwin != NULL && clistpanel != NULL);
	}

	if (!groups)
		return;
	if (groups->len < 1)
		return;

	werase(clistwin);

	/*
	 * Determine the start group depending on the offset
	 */

	for (i = 0, a = 0; i < groups->len; i++) {
		group = g_ptr_array_index(groups, i);

		if ((a + group->items->len) > list_offset)
			break;

		a += group->items->len;
	}

	b = list_offset - a;

	for (y = 0; y < get_max_y() - 4 && i < groups->len; y++) {

		item = g_ptr_array_index(group->items, b);

		/*
		 * If we are at the selected item then
		 * * add reversed colors
		 */
		if (y == (list_marked - list_offset) &&
		    focus_get() == FocusContactList)
			item->attr |= A_REVERSE;


		wattron(clistwin, item->attr);

		/*
		 * If colors are reversed or underlined,
		 * * then fill the whole line to give a better effect.
		 */
		if (item->attr & A_REVERSE)
			mvwhline(clistwin, y, 0, ' ', contactlist_width - 3);

		mvwaddwstr_with_maxwidth(clistwin, y, 0, item->text,
					 contactlist_width);
		wattrset(clistwin, A_NORMAL);

		if (y == (list_marked - list_offset) &&
		    focus_get() == FocusContactList)
			item->attr &= ~A_REVERSE;

		if (b >= group->items->len - 1) {
			group = g_ptr_array_index(groups, ++i);
			b = 0;
		} else {
			b++;
		}
	}

	update_panels();
	doupdate();
}

gint
sort_compare_func(gconstpointer a, gconstpointer b)
{
	TpaContactPresence a_pres, b_pres;
	FamaContactListItem *a_item, *b_item;

	a_item = *(FamaContactListItem **) a;
	b_item = *(FamaContactListItem **) b;

	if (a_item->contact == NULL)
		return -1;
	else if (b_item->contact == NULL)
		return 1;

	a_pres = tpa_contact_base_get_presence(TPA_CONTACT_BASE
					       (a_item->contact));
	b_pres = tpa_contact_base_get_presence(TPA_CONTACT_BASE
					       (b_item->contact));

	g_assert(a_pres > 0 && b_pres > 0);

	if (b_pres == TPA_PRESENCE_OFFLINE)
		return -1;
	else if (a_pres > b_pres || a_pres == TPA_PRESENCE_OFFLINE)
		return 1;

	return -1;
}


void
contactlist_sort()
{
	FamaContactListGroup *group;
	gint i;

	for (i = 0; i < groups->len; i++) {
		group = g_ptr_array_index(groups, i);
		g_ptr_array_sort(group->items, sort_compare_func);
	}
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
contactlist_free_group(FamaContactListGroup * group)
{
	FamaContactListItem *item;
	gint i;

	for (i = 0; i < group->items->len; i++) {
		item = g_ptr_array_index(group->items, i);
		g_free(item->text);
		g_free(item);
	}
	g_ptr_array_free(group->items, TRUE);
}

void
contactlist_remove_group(FamaContactListGroup * group)
{
	contactlist_free_group(group);

	if (groups)
		g_ptr_array_remove(groups, group);
}

void
contactlist_free()
{
	FamaContactListGroup *group;
	gint i;

	if (groups) {
		for (i = 0; i < groups->len; i++) {
			group = g_ptr_array_index(groups, i);
			contactlist_free_group(group);
		}

		g_ptr_array_free(groups, TRUE);
		groups = NULL;
	}
}


FamaContactListGroup *
contactlist_get_group(TpaConnection * connection)
{
	FamaContactListGroup *group = NULL;
	gint i;

	/*
	 * Find the group that corresponds with the
	 * connection object.
	 */
	for (i = 0; i < groups->len; i++) {
		group = g_ptr_array_index(groups, i);

		if (group->tpa_connection == connection)
			break;

		group = NULL;
	}
	return group;
}

void
contactlist_presence_updated_cb(TpaContact * contact,
				TpaContactPresence presence, gchar * message)
{
	FamaContactListGroup *group = NULL;
	FamaContactListItem *item = NULL;
	gint i, j;

	for (i = 0; i < groups->len; i++) {
		group = g_ptr_array_index(groups, i);
		for (j = 0; j < group->items->len; j++) {
			item = g_ptr_array_index(group->items, j);
			if (item->contact == contact) {
				item->attr =
					contactlist_presence_to_attr(presence);
				break;
			}
		}
	}

	contactlist_sort();
	contactlist_draw();
}

void
contactlist_alias_changed_cb(TpaContact * contact, gchar * alias)
{
	FamaContactListGroup *group = NULL;
	FamaContactListItem *item = NULL;
	gint i, j;

	for (i = 0; i < groups->len; i++) {
		group = g_ptr_array_index(groups, i);
		for (j = 0; j < group->items->len; j++) {
			item = g_ptr_array_index(group->items, j);
			if (item->contact == contact) {
				g_free(item->text);
				item->text = g_new(wchar_t, strlen(alias) + 1);
				utf8_to_wchar(alias, item->text, strlen(alias));
				break;
			}
		}
	}

	contactlist_draw();
}



void
contactlist_add_contact(FamaContactListGroup * group, TpaContact * contact)
{
	TpaContactPresence contact_presence;
	FamaContactListItem *item = NULL;
	const gchar *contact_alias;

	item = g_new(FamaContactListItem, 1);

	g_signal_connect(G_OBJECT(contact), "presence-updated",
			 G_CALLBACK(contactlist_presence_updated_cb), NULL);

	g_signal_connect(G_OBJECT(contact), "alias-changed",
			 G_CALLBACK(contactlist_alias_changed_cb), NULL);

	contact_alias = tpa_contact_base_get_alias(TPA_CONTACT_BASE(contact));
	contact_presence =
		tpa_contact_base_get_presence(TPA_CONTACT_BASE(contact));

	item->text = g_new(wchar_t, strlen(contact_alias) + 1);
	utf8_to_wchar(contact_alias, item->text, strlen(contact_alias));

	item->contact = contact;
	item->parent_group = group;
	item->attr = contactlist_presence_to_attr(contact_presence);

	g_ptr_array_add(group->items, item);
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
	FamaContactListGroup *group;
	gint i;

	for (i = 0; i < groups->len; i++) {
		group = g_ptr_array_index(groups, i);

		if (group->tpa_contactlist == list)
			break;

		group = NULL;
	}

	if (group)
		contactlist_add_contact(group, contact);
	else
		g_warning("Accepted subscription on non-existant connection?!");
}


FamaContactListGroup *
contactlist_add_group(TpaConnection * connection, TpaContactList * contactlist,
		      const gchar * title)
{
	FamaContactListItem *item;
	FamaContactListGroup *group;

	group = g_new(FamaContactListGroup, 1);
	group->tpa_connection = connection;
	group->tpa_contactlist = contactlist;
	group->items = g_ptr_array_new();

	item = g_new(FamaContactListItem, 1);
	item->contact = NULL;

	item->text = g_new(wchar_t, strlen(title) + 1);
	utf8_to_wchar(title, item->text, strlen(title));

	item->attr = A_BOLD | A_UNDERLINE;
	g_ptr_array_add(group->items, item);
	g_ptr_array_add(groups, group);

	return group;
}

gint
contactlist_count_rows()
{
	FamaContactListGroup *group;
	gint i, c = 0;

	for (i = 0; i < groups->len; i++) {
		group = g_ptr_array_index(groups, i);
		c += group->items->len;
	}

	return c;
}

void
contactlist_scroll(gint m)
{
	gint rows = contactlist_count_rows(), old_list_mark = list_marked;

	list_marked += m;

	if (list_marked >= rows - 1)
		list_marked = rows - 1;
	if (list_marked < 0)
		list_marked = 0;

	while (list_marked >= (list_offset + get_max_y() - 4))
		list_offset++;

	if (list_marked < list_offset)
		list_offset = list_marked;

	if (old_list_mark != list_marked)
		contactlist_draw();
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

FamaContactListItem *
contactlist_get_selected()
{
	FamaContactListGroup *group = NULL;
	FamaContactListItem *item = NULL;
	gint i, len;

	for (i = 0, len = 0; i < groups->len; i++) {
		group = g_ptr_array_index(groups, i);

		if (list_marked < (len + group->items->len)) {
			item = g_ptr_array_index(group->items,
						 list_marked - len);
			break;
		} else {
			len += group->items->len;
		}
	}

	if (item == NULL)
		return NULL;

	if (item->contact == NULL)
		return NULL;

	return item;
}

void
contactlist_reload_from_server(TpaConnection * conn)
{
	FamaContactListGroup *group;
	TpaContactList *list;
	GPtrArray *contacts;
	TpaContact *contact;
	gchar *account;
	gint i;

	group = contactlist_get_group(conn);
	if (group)
		contactlist_remove_group(group);

	list = tpa_connection_get_contactlist(conn);
	contacts = tpa_contact_list_get_known(list);

	g_signal_connect(G_OBJECT(list), "authorization-requested",
			 G_CALLBACK
			 (contactlist_authorization_requested_cb), NULL);

	g_signal_connect(G_OBJECT(list), "subscription-accepted",
			 G_CALLBACK
			 (contactlist_subscription_accepted_cb), NULL);

	account = connection_get_account_from_connection(conn);
	group = contactlist_add_group(conn, list, account);

	for (i = 0; i < contacts->len; i++) {
		contact = g_ptr_array_index(contacts, i);
		contactlist_add_contact(group, contact);
	}

}
