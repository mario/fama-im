#include "common.h"
#include <string.h>
#include <stdlib.h>

GHashTable *table = NULL;


gboolean command_func_window(gint argc, gchar ** argv);
gboolean command_func_help(gint argc, gchar ** argv);
gboolean command_func_connect(gint argc, gchar ** argv);
gboolean command_func_quit(gint argc, gchar ** argv);
gboolean command_func_account(gint argc, gchar ** argv);
gboolean command_func_contact(gint argc, gchar ** argv);
gboolean command_func_status(gint argc, gchar ** argv);
gboolean command_func_log(gint argc, gchar ** argv);

/*
 * This function is for command_func_help()'s private use
 */
void
_concat_command_str(gpointer key, gpointer value, gpointer user_data)
{
	gchar *tmp;
	gchar **input = (gchar **) user_data;

	tmp = g_strjoin(", ", (gchar *) key, *input, NULL);

	if (*input != NULL)
		g_free(*input);

	*input = tmp;
}

/*
 * List all registered commands
 */
gboolean
command_func_help(gint argc, gchar ** argv)
{
	wchar_t *outbuf;
	gchar *mbseq;

	if (argc > 1) {
		g_warning("help: at this time, displaying help for other "
			  "commands is not implemented");
		return FALSE;
	}

	/*
	 * Display a list of available commands 
	 */
	mbseq = NULL;
	g_hash_table_foreach(table, _concat_command_str, &mbseq);
	outbuf = g_new(wchar_t, strlen(mbseq) + 1);
	utf8_to_wchar(mbseq, outbuf, strlen(mbseq));

	window_add_message(window_get_current(), L"List of commands", A_BOLD,
			   outbuf);

	return TRUE;
}

/*
 * Exit the main-loop
 */
gboolean
command_func_quit(gint argc, gchar ** argv)
{
	stop_main_loop();
	return TRUE;
}


/*
 * Turn logging on and off
 */
gboolean
command_func_log(gint argc, gchar ** argv)
{
     if (argc > 2)
          g_warning("usage: /log [on|off]");
     if (argc == 1)
          if (get_logging() == TRUE)
               g_message("logging is on");
          else
               g_message("logging is off");
     if (argc == 2)
          set_logging(argv[1]);
     return 0;
}


/*
 * Switch between windows
 */
gboolean
command_func_window(gint argc, gchar ** argv)
{
	FamaWindow *w;

	if (argc != 2) {
		g_warning("usage: /window <n>");
		return FALSE;
	}

	if (g_ascii_strcasecmp(argv[1], "close") == 0) {
		g_assert((w = window_get_current()) != NULL);

		if (w->type == WindowTypeMain) {
			g_warning("cannot close the main window!");
			return FALSE;
		}

		window_destroy(w);
		return TRUE;
	}

	w = window_get_index(atoi(argv[1]));

	if (w == NULL) {
		g_warning("No such window!");
		return FALSE;
	}

	window_set_current(w);

	return TRUE;
}

/*
 * Add a new command
 */
void
command_add(gchar * command, CommandFunc func)
{
	g_hash_table_insert(table, (gpointer) command, (gpointer) func);
}

/*
 * Initialize the command hash-table
 */
void
command_init()
{
	table = g_hash_table_new(g_str_hash, g_str_equal);
	command_add("help", command_func_help);
	command_add("quit", command_func_quit);
	command_add("window", command_func_window);
	command_add("connect", command_func_connect);
	command_add("account", command_func_account);
	command_add("status", command_func_status);
	command_add("contact", command_func_contact);
        command_add("log", command_func_log);
}

/*
 * Execute argv[0] as a command
 */
gboolean
command_execute(gint argc, gchar ** argv)
{
	CommandFunc func;

	if (argc < 1)
		return FALSE;

	func = (CommandFunc) g_hash_table_lookup(table, argv[0]);

	if (func == NULL)
		return FALSE;

	func(argc, argv);

	return TRUE;
}
