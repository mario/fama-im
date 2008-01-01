#include <locale.h>
#include <glib/gstdio.h>

#include "common.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "ui-history.h"
#include "mc-accounts.h"
#include <libempathy/empathy-contact-list.h>
#include <libempathy/empathy-contact-manager.h>
#include "empathy-contactliststore.h"
GMainLoop *loop;
EmpathyContactListStore *list_store;

int
main(int argc, char **argv)
{
	g_type_init();
//	empathy_debug_set_log_file_from_env();

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
	mc_uninit();
	destroy_interface();
	
	g_object_unref(list_store);

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
	EmpathyContactList       *contactlist;
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

	g_message("Fama v%s (c) 2007-2008 Mario Danic", VERSION);

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
		empathy_contactlistwin_set_width(a);
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
	g_free(c);
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

	g_free(c);
	c = g_key_file_get_string(keyfile_get(), "core", "logging", &err);
	if (err != NULL) {
		if (err->code == G_KEY_FILE_ERROR_INVALID_VALUE)
			g_warning("cannot turn logging to: %s", err->message);

		g_clear_error(&err);
	} else {
		set_logging(c);
	}
	g_free(c);

        contactlist = EMPATHY_CONTACT_LIST (empathy_contact_manager_new ());
        list_store = empathy_contact_list_store_new (contactlist);
	g_object_unref(contactlist);

	/*
	 * initialize the mission control
	 */

	mc_init();
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
	 * Initialize history interface
	 */ 
	
	famahistory_command_init();

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

