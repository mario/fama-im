#include "common.h"
#include <string.h>
#include "ui-window.h"
#include "empathy-contactliststore.h"
#define WINDOW_WIDTH (get_max_x() - empathy_contactlistwin_get_width())

GPtrArray *window_list = NULL;
FamaWindow *current_window = NULL;

/*
 * Create a status-string for use in the status-bar
 */

gchar *
window_create_status_string()
{
	FamaWindow *win;
	gchar **list;
	gchar *status_str;
	gint i, j;

	list = g_new(gchar *, window_list->len + 1);

	for (i = 0, j = 0; i < window_list->len; i++) {
		win = g_ptr_array_index(window_list, i);

		if (win->is_updated)
			list[j++] = g_strdup_printf("%d", i);
	}

	list[j] = NULL;

	status_str = g_strjoinv(", ", list);
	g_strfreev(list);

	return status_str;
}


/*
 * Return the window associated with the channel 'c'
 */

FamaWindow *
window_find_empathychat(EmpathyChat *chat)
{
	FamaWindow *win = NULL;
	gint i;

	if (window_list == NULL)
		return NULL;

	for (i = 0; i < window_list->len; i++) {
		win = g_ptr_array_index(window_list, i);

		if (win->empathychat == chat)
			break;

		win = NULL;
	}

	if (!win) {
		win = empathy_chat_appendwindow(chat);
	}
	return win;
}

void
window_draw_title_bar()
{
	ColorSettings *c = color_get();

	attron(A_UNDERLINE | A_BOLD | c->window_title);
	mvhline(1, 0, ' ', WINDOW_WIDTH);

	if (current_window->title != NULL)
		mvwaddwstr_with_maxwidth(stdscr, 1, 2, current_window->title,
					 WINDOW_WIDTH - 2);

	attrset(A_NORMAL);
}

void
window_draw_all_titlebars()
{
	gint i, strlength, pos = 0, n = 0xff, attr;
	ColorSettings *c = color_get();
	FamaWindow *win;
	EmpathyChat *chat;

	if (window_list == NULL)
		return;
	/**FIXME: refresh screen doesn't work*/
	//wredrawln(stdscr, 0, 10);
	for (i = 0; i < window_list->len; i++) {
		win = g_ptr_array_index(window_list, i);

		if (win == current_window) {
			attr = c->status_active_window | A_BOLD;
		} else {
			if (win->is_updated)
				attr = c->window_title_uploaded | A_BOLD;
			else
				attr = c->window_title;
		}

		attron(A_UNDERLINE | attr);
		/** FIXME: window's bounder is not checked*/
		if (win->title != NULL) {
			strlength = wcswidth(win->title, n);
			mvwaddwstr_with_maxwidth(stdscr, 1, pos, win->title,
					 WINDOW_WIDTH - 2);
			pos += strlength + 4;
		}

		attrset(A_NORMAL);
	}
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
	current_window = w;
	g_assert(w);

	if (w != NULL) {
		w->is_updated = FALSE;

		top_panel(w->ncpanel);
		//window_draw_title_bar();
		window_draw_all_titlebars();
		statusbar_draw();
	}

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
	w->empathychat = NULL;
	w->is_updated = TRUE;

	g_ptr_array_add(window_list, w);

	bottom_panel(w->ncpanel);

	return w;
}

void
window_append_rows(FamaWindow * w, GPtrArray * m, gint attr)
{
	gint i;

	for (i = 0; i < m->len; i++) {
		waddwstr(w->ncwin, L"\n");

		if (i == 0)
			wattron(w->ncwin, attr);

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

	if (current_window == w) {
		w->is_updated = FALSE;
	} else {
		w->is_updated = TRUE;
		/** refresh the window titles display*/
		window_draw_all_titlebars();
		statusbar_draw();
	}

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

	g_object_unref(w->empathychat);
	g_ptr_array_remove(window_list, w);
	g_free(w->title);
	g_free(w);

	if (window_list->len)
		window_set_current(g_ptr_array_index(window_list, 0));
}

void
window_destroy_all_windows()
{
	int i = 0;
	FamaWindow *win;
	for (i = 0; i < window_list->len; i++) {
		win = g_ptr_array_index(window_list, 0);
		window_destroy(win);
	}
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

gint
get_window_index(FamaWindow * window)
{
	gint i;

	for (i = 0; i < window_list->len; i++)
		if (g_ptr_array_index(window_list, i) == window)
			return i;
	return 0;
}

EmpathyChat *
empathy_chat_window_find_chat (McAccount   *account, const gchar *id)
{
	gint i;
	EmpathyChat *chat;
	FamaWindow *win = NULL;

	g_return_val_if_fail (MC_IS_ACCOUNT (account), NULL);
	g_return_val_if_fail (!G_STR_EMPTY (id), NULL);

	for (i = 0; i < window_list->len; i++) {
		win = g_ptr_array_index(window_list, i);
		chat = win->empathychat;
		if (!chat)
			continue;

		if (empathy_account_equal (account, chat->account) &&
			strcmp (id, empathy_chat_get_id (chat)) == 0) {
			return chat;
		}
	}
	return NULL;
}
