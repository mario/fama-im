#include "common.h"

struct list_item {
	wchar_t *text;
	gint attr;
	// There should be a FamaContact datatype here in the future
};

GSList *categories = NULL;
WINDOW *clistwin = NULL;
PANEL *clistpanel = NULL;

gint contactlist_width = 30;
int list_offset = 0, list_marked = 0;

gint
contactlist_count_rows()
{
	GSList *categoryTmp;
	gint count = 0;

	categoryTmp = categories;
	do {
		count += ((GPtrArray *) categoryTmp->data)->len;
	} while ((categoryTmp = g_slist_next(categoryTmp)) != NULL);

	return count;
}



void
contactlist_draw()
{
	GSList *categoryTmp;
	GPtrArray *array;
	struct list_item *item;
	int i, a, str_len, str_width;

	if (clistwin == NULL) {
		clistwin = newwin(get_max_y() - 3,
				  contactlist_width - 3,
				  1, get_max_x() - contactlist_width + 2);
		clistpanel = new_panel(clistwin);

		g_assert(clistwin != NULL && clistpanel != NULL);
	}

	/*
	 * Clear the window 
	 */
	werase(clistwin);

	if ((categoryTmp = categories) == NULL)
		return;

	/*
	 * Skip a number of categories and items depending on offset 
	 */
	i = a = 0;
	do {
		array = (GPtrArray *) categoryTmp->data;
		if (i + array->len > list_offset) {
			a = list_offset - i;
			break;
		}
		i += array->len;
	} while ((categoryTmp = g_slist_next(categoryTmp)) != NULL);

	/*
	 * Now categoryTmp will point to the category of which we will
	 * start to print items from, the integer 'a' is the index in
	 * the GPtrArray where we start.
	 */

	i = 0;
	while (categoryTmp != NULL && i < get_max_y() - 2) {
		array = (GPtrArray *) categoryTmp->data;
		for (; a < array->len; a++, i++) {
			item = g_ptr_array_index(array, a);
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
			if ((item->attr & A_REVERSE) ||
			    (item->attr & A_UNDERLINE))
				mvwhline(clistwin, i, 0, ' ',
					 contactlist_width - 3);


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

		/*
		 * reset for next category
		 */
		a = 0;

		categoryTmp = g_slist_next(categoryTmp);
	}

	update_panels();
	doupdate();
}

void
contactlist_scroll(gint m)
{
	gint rows = contactlist_count_rows();

	list_marked += m;

	if (list_marked < 0)
		list_marked = 0;
	if (list_marked >= rows - 1)
		list_marked = rows - 1;

	while (list_marked >= (list_offset + get_max_y() - 3))
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

gint
sort_function(gconstpointer a, gconstpointer b)
{
	gchar *aNew, *bNew;
	gint r;

	if (a == b)
		return TRUE;

	if (!a || !b)
		return FALSE;

	aNew = g_utf8_casefold((gchar *) a, -1);
	bNew = g_utf8_casefold((gchar *) b, -1);

	r = g_utf8_collate(aNew, bNew);

	g_free(aNew);
	g_free(bNew);

	return r;
}

/* Make a new GPtrArray of list_items and append the
 * array to the GSList 'categories'.
 */

void
contactlist_add_category(const wchar_t * title)
{
	struct list_item *c;
	GPtrArray *array = NULL;

	c = (struct list_item *)g_malloc(sizeof(struct list_item));
	c->text = g_new(wchar_t, wcslen(title) + 1);
	wcscpy(c->text, title);

	c->attr = A_UNDERLINE | A_BOLD;

	array = g_ptr_array_new();
	g_ptr_array_add(array, (gpointer) c);
	categories = g_slist_append(categories, array);
}

void
contactlist_add_item(guint category_index, const wchar_t * text, int attr)
{
	struct list_item *c;
	GPtrArray *array = NULL;

	c = g_new(struct list_item, 1);

	c->text = g_new(wchar_t, wcslen(text) + 1);
	wcscpy(c->text, text);

	c->attr = attr;

	array = (GPtrArray *) g_slist_nth_data(categories, category_index);
	g_assert(array != NULL);
	g_ptr_array_add(array, (gpointer) c);
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
