#include <locale.h>
#include <glib/gstdio.h>

#include "common.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

GMainLoop *loop;

int
main(int argc, char **argv)
{
	g_type_init();

	tpa_thread_init(FALSE);

	loop = g_main_loop_new(NULL, FALSE);

	/*
	 * Main looks prettier this way ;-)
	 */
	if (init_all() == FALSE)
		return 0;

	g_main_loop_run(loop);
	g_main_loop_unref(loop);

	/*
	 * Kill the memory!!!!
	 */
	destroy_interface();
	connection_disconnect_all();
	account_destroy();
	manager_factory_destroy();
	contactlist_free();
	tpa_thread_shutdown(FALSE);

	return 0;
}


void
stop_main_loop()
{
	g_main_loop_quit(loop);
}

gboolean
init_all()
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

	g_message("Fama v%s (c) 2007 Mario Danic & Jonas Broms", VERSION);

	/*
	 * Check wether a D-Bus session is available or not
	 */
	if (g_getenv("DBUS_SESSION_BUS_ADDRESS") == NULL) {
		g_warning
			("There is no D-Bus session is available for this session!");
		g_warning
			("Run 'export `dbus-launch --exit-with-session`' to start one.");
		return FALSE;
	}


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
		g_message
			("Redirecting stderr to %s, thus supressing error messages\n",
			 c);
		g_freopen(c, "w", stderr);
	}

	/*
	 * Initialize manager factory
	 */
	manager_factory_init();

	/*
	 * Load account file
	 */
	account_init();

	/*
	 * Initialize the interface
	 */

	init_interface();

	/*
	 * Start the clock for interface use
	 */
	clock_cb(NULL);

	/*
	 * Initialize message-logger
	 */
	log_init();

	/*
	 * Call redraw_interface on SIGWINCH 
	 */

	signal_handler_setup();

	/*
	 * Register commands 
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

	draw_interface();

	return TRUE;
}
