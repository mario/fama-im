#include "common.h"

#define WINDOW_WIDTH (get_max_x() - contactlist_get_width())

GPtrArray *window_list = NULL;
FamaWindow *current_window = NULL;

void
window_draw_title_bar()
{
	gint l, w;
	ColorSettings *c = color_get();

	attron(A_UNDERLINE | A_BOLD | c->window_title);
	mvhline(1, 0, ' ', WINDOW_WIDTH);

	if (current_window->title != NULL) {

		for (l = 0, w = 2;
		     w < WINDOW_WIDTH && current_window->title[l] != L'\0'; l++)
			w += wcwidth(current_window->title[l]);

		mvaddnwstr(1, 2, current_window->title, l);
	}
	attrset(A_NORMAL);
}

void
window_set_title(FamaWindow * w, wchar_t * title)
{
	if (w->title != NULL)
		g_free(w->title);

	w->title = g_new(wchar_t, wcslen(title) + 1);
	wcscpy(w->title, title);
}

void
window_set_current(FamaWindow * w)
{
	g_assert(w != NULL);

	current_window = w;
	w->is_updated = FALSE;

	top_panel(w->ncpanel);
	window_draw_title_bar();

	update_panels();
	doupdate();
}

FamaWindow *
window_get_index(gint i)
{
	if (i < 0 || i >= window_list->len)
		return NULL;

	return g_ptr_array_index(window_list, i);
}

FamaWindow *
window_get_current()
{
	return current_window;
}


FamaWindow *
window_new(FamaWindowType type)
{
	FamaWindow *w;

	if (window_list == NULL)
		window_list = g_ptr_array_new();

	w = g_new(FamaWindow, 1);
	w->ncwin = newwin(get_max_y() - 4, WINDOW_WIDTH, 2, 0);
	g_assert(w->ncwin != NULL);

	scrollok(w->ncwin, TRUE);

	w->ncpanel = new_panel(w->ncwin);
	g_assert(w->ncpanel != NULL);

	w->messages = g_ptr_array_new();
	w->type = type;
	w->title = NULL;
	w->is_updated = TRUE;

	g_ptr_array_add(window_list, w);

	bottom_panel(w->ncpanel);

	return w;
}

void
window_append_rows(FamaWindow * w, GPtrArray * m, gint attr)
{
	ColorSettings *c = color_get();
	gint i;

	for (i = 0; i < m->len; i++) {
		waddwstr(w->ncwin, L"\n");

		if (i == 0)
			wattron(w->ncwin, attr);
		else 
			wattron(w->ncwin, c->message_text);

		waddwstr(w->ncwin, g_ptr_array_index(m, i));

		wattrset(w->ncwin, A_NORMAL);

	}
}

/*
 * window_add_message is a wrapper function to window_append_rows
 */
void
window_add_message(FamaWindow * w, wchar_t * title, gint attr, wchar_t * str)
{
	GPtrArray *rows;
	FamaMessage *message;

	message = g_new(FamaMessage, 1);

	message->title = g_new(wchar_t, wcslen(title) + 1);
	message->message = g_new(wchar_t, wcslen(str) + 1);
	wcscpy(message->title, title);
	wcscpy(message->message, str);

	message->attr = attr;

	g_ptr_array_add(w->messages, message);

	w->is_updated = (current_window == w) ? TRUE : FALSE;

	rows = textwrap_new(title);
	textwrap_append(rows, str, WINDOW_WIDTH - 2);
	window_append_rows(w, rows, attr);
	textwrap_destroy(rows);

	update_panels();
	doupdate();
}

void
window_destroy_helper_func(gpointer data, gpointer user_data)
{
	FamaMessage *m = (FamaMessage *) data;

	g_free(m->message);
	g_free(m->title);
}

void
window_destroy(FamaWindow * w)
{
	if (w->type == WindowTypeMain || w->type == WindowTypeConversation) {
		g_ptr_array_foreach(w->messages, window_destroy_helper_func,
				    NULL);
		g_ptr_array_free(w->messages, FALSE);
	}

	del_panel(w->ncpanel);
	delwin(w->ncwin);

	g_ptr_array_remove(window_list, w);
	window_set_current(g_ptr_array_index(window_list, 0));
}

void
window_resize_helper_func(gpointer data, gpointer user_data)
{
	FamaWindow *w = (FamaWindow *) data;
	FamaMessage *m;
	GPtrArray *rows;
	WINDOW *new_win;
	gint i;

	new_win = newwin(get_max_y() - 4, WINDOW_WIDTH, 2, 0);
	g_assert(new_win != NULL);
	scrollok(new_win, TRUE);

	replace_panel(w->ncpanel, new_win);
	delwin(w->ncwin);
	w->ncwin = new_win;

	werase(w->ncwin);


	/*
	 * Resize messages
	 */

	if ((w->type != WindowTypeMain &&
	     w->type != WindowTypeConversation) || w->messages == NULL)
		return;

	for (i = 0; i < w->messages->len; i++) {
		m = g_ptr_array_index(w->messages, i);
		rows = textwrap_new(m->title);
		textwrap_append(rows, m->message, WINDOW_WIDTH - 2);
		window_append_rows(w, rows, m->attr);
		textwrap_destroy(rows);
	}
}

void
window_resize_all()
{
	g_ptr_array_foreach(window_list, window_resize_helper_func, NULL);
	update_panels();
	doupdate();
}
