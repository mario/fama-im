#include "common.h"

typedef struct {
	PANEL *panel;
	WINDOW *window;
	gint w;
	gint h;
} FamaDialog;

FamaDialog *
dialog_new(gint height, gint width)
{
	FamaDialog *d;

	d = g_new(FamaDialog, 1);
	d->w = width;
	d->h = height;
	d->window = newwin(height, width, (get_max_y() - height) / 2, (get_max_x() - width) / 2);
	d->panel = new_panel(d->window);

	return d;
}

void
dialog_draw(FamaDialog *d, wchar_t *title)
{
	top_panel(d->panel);

	box(d->window, 0, 0);
	mvwaddwstr(d->window, 0, (d->w - wcswidth(title, d->w-2)) / 2, title);
	update_panels();
	doupdate();
}

void
dialog_destroy(FamaDialog *d)
{
	del_panel(d->panel);
	delwin(d->window);
	g_free(d);
	window_set_current (window_get_current());
}

void
dialog_yes_no (gchar *title, gchar *message)
{
	FamaDialog *d;

	d = dialog_new(15, 50);
	dialog_draw(d, L"Test dialog");
	g_usleep(6000000);
	dialog_destroy(d);
}

