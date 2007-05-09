#include "common.h"
#include <string.h>

GPtrArray *contactlist = NULL;
WINDOW *clistwin = NULL;
PANEL *clistpanel = NULL;

gint contactlist_width = 30;
gint list_offset = 0, list_marked = 0;

void
contactlist_add_item(TpaConnection *connection, TpaContact * contact, const wchar_t * text, gint attr)
{
	FamaContactListItem *a = g_new(FamaContactListItem, 1);

	a->contact = contact;
	a->connection = connection;
	a->text = g_new(wchar_t, wcslen(text) + 1);
	wcscpy(a->text, text);
	a->attr = attr;

	g_ptr_array_add(contactlist, a);
}

FamaContactListItem *
contactlist_get_selected()
{
	return g_ptr_array_index(contactlist, list_marked);
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
		contactlist = g_ptr_array_new();

		g_assert(clistwin != NULL && clistpanel != NULL &&
			 contactlist != NULL);
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
		if (i == (list_marked - list_offset))
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

		if (i == (list_marked - list_offset))
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

	if (list_marked < 0)
		list_marked = 0;
	if (list_marked >= rows - 1)
		list_marked = rows - 1;

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
		}
	}

	contactlist_draw();
}
