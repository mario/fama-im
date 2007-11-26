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

	werase(clistwin);

	if (groups->len < 1) {
		update_panels();
		doupdate();
		return;
	}


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


