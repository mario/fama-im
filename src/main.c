#include <locale.h>
#include <glib/gstdio.h>

#include "common.h"
#include "fama-config.h"

GMainLoop *loop;

int
main(int argc, char **argv)
{
	g_type_init();

	loop = g_main_loop_new(NULL, FALSE);

	g_idle_add(init_all, NULL);

	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	destroy_interface();

	return 0;
}


void
stop_main_loop()
{
	g_main_loop_quit(loop);
}

gboolean
init_all(gpointer data)
{
	GError *err = NULL;
	gint a;
	gchar *c;
	const gchar *charset;

	/*
	 * Set locale
	 */

	if (!setlocale(LC_ALL, "")) {
		g_error("Can't set the specified locale!\n");
		return FALSE;
	}

	/*
	 * Print version information
	 */

	g_message("Fama v%s (c) 2007 Jonas Broms", VERSION_STRING);

	/*
	 * Read configuration file (and act accordingly)
	 */

	if (keyfile_read() == FALSE)
		return FALSE;

	a = g_key_file_get_integer(keyfile_get(), "core",
				   "contact_list_width", &err);
	if (err != NULL) {
		if (err->code == G_KEY_FILE_ERROR_INVALID_VALUE)
			g_warning("cannot get contact_list_width: %s",
				  err->message);

		g_clear_error(&err);
	} else {
		contactlist_set_width(a);
	}

	g_get_charset(&charset);
	set_interface_encoding(g_strdup(charset));
	c = g_key_file_get_string(keyfile_get(), "core", "charset", &err);
	if (err != NULL) {
		if (err->code == G_KEY_FILE_ERROR_INVALID_VALUE)
			g_warning("cannot get character charset: %s",
				  err->message);

		g_clear_error(&err);
	} else {
		set_interface_encoding(c);
	}
	g_message("Using charset '%s'", get_interface_encoding());

	c = g_key_file_get_string(keyfile_get(), "core",
				  "redirect_stderr", &err);
	if (err) {
		if (err->code == G_KEY_FILE_ERROR_INVALID_VALUE)
			g_warning("cannot get 'redirect_stderr': %s",
				  err->message);

		g_clear_error(&err);
	} else if (!err && c) {
		g_message("Redirecting stderr to %s", c);
		g_freopen(c, "w", stderr);
	}

	/*
	 * Initialize manager factory
	 */
	manager_factory_init();

	/*
	 * Initialize the interface
	 */

	init_interface();

	/*
	 * Initialize message-logger
	 */
	log_init();

	/*
	 * Call redraw_interface on SIGWINCH 
	 */

	signal_handler_setup();

	/*
	 * Registering commands 
	 */

	command_init();

	/*
	 * Call stdin_handle_input when there's input from stdin
	 */

	stdin_handler_setup();

	/*
	 * Initialize the command-line buffer
	 */

	commandline_init();

	/*
	 * Draw the interface
	 */

	/*
	 * Test stuff 
	 */
	contactlist_add_category(L"MSN");
	contactlist_add_item(0, L"A 0 只B只C只只只只只只只只只", 0);
	contactlist_add_item(0, L"Hello", 0);

	contactlist_add_category(L"Jabber");
	contactlist_add_item(1, L"Lalalal alal", 0);
	contactlist_add_item(1, L"Bababab", 0);

	contactlist_add_category(L"ICQ");
	contactlist_add_item(2, L"asdasddlo", 0);
	contactlist_add_item(2, L"Mahatmao", 0);
	contactlist_add_item(2, L"Leningrad", 0);
	contactlist_add_item(2, L"asdasddlo", 0);
	contactlist_add_item(2, L"Mahatmao", 0);
	contactlist_add_item(2, L"Leningrad", 0);
	contactlist_add_item(2, L"asdasddlo", 0);
	contactlist_add_item(2, L"Mahatmao", 0);
	contactlist_add_item(2, L"Leningrad", 0);
	contactlist_add_item(2, L"asdasddlo", 0);
	contactlist_add_item(2, L"Mahatmao", 0);
	contactlist_add_item(2, L"Leningrad", 0);
	contactlist_add_item(2, L"asdasddlo", 0);
	contactlist_add_item(2, L"Mahatmao", 0);
	contactlist_add_item(2, L"Leningrad", 0);
	contactlist_add_item(2, L"asdasddlo", 0);
	contactlist_add_item(2, L"Mahatmao", 0);
	contactlist_add_item(2, L"Leningrad", 0);

	draw_interface();

	/*
	 * Return FALSE so that the function isn't ran again
	 */
	return FALSE;
}
