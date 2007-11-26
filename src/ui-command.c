#include "common.h"
#include "ui-window.h"
#include <string.h>
#include <stdlib.h>
#include <glib/gprintf.h>
#include "ui-history.h"

GHashTable *table = NULL;

gboolean command_func_window(gint argc, gchar ** argv);
gboolean command_func_help(gint argc, gchar ** argv);
gboolean command_func_connect(gint argc, gchar ** argv);
gboolean command_func_quit(gint argc, gchar ** argv);
gboolean command_func_account(gint argc, gchar ** argv);
gboolean command_func_contact(gint argc, gchar ** argv);
gboolean command_func_log(gint argc, gchar ** argv);
gboolean command_func_history(gint argc, gchar ** argv);

/** argc is number of arguments in a command line,
 * argv is array of the arguments*/
void command_hist_add(gint argc, gchar ** argv);
/** load history setting in profile(keyfile)*/
gboolean command_hist_loadprofile();
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
	famahistory_command_savesession();
	return TRUE;
}


/*
 * Turn logging on and off
 */
gboolean
command_func_log(gint argc, gchar ** argv)
{
	if (argc != 2)
		g_warning("usage: /log [on|off|status]");

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
        gint index;

	if (argc != 2) {
		g_warning("usage: /window <n>");
		return FALSE;
	}

	if (g_ascii_strcasecmp(argv[1], "close") == 0) {
		g_assert((w = window_get_current()) != NULL);

		if (w->type == WindowTypeMain) {
                       g_warning("Cannot close the main window!");
                       return FALSE;
		}

                index = get_window_index(w);

		window_destroy(w);

                if (window_get_index(index) != NULL){
                     window_set_current(window_get_index(index));
                } else {
                     window_set_current(window_get_index(index-1));
                }
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

gboolean
command_func_history(gint argc, gchar ** argv)
{
	gchar **infostring;
	gchar *string, strtitle[256];
	wchar_t *outbuf, wtitle[256];
	gint mask = 0;
	if (argc == 0)
		return FALSE;
	g_sprintf(strtitle, "History infomation : ");
	if (argc == 1) {
		infostring = famahistory_info_get(HISINFO_USAGE | HISINFO_MAXNUMBER | 
			HISINFO_ENABLE | HISINFO_LISTNUM);

		g_assert(infostring);
		string = g_strjoinv("\n", infostring);
		outbuf = g_new(wchar_t, strlen(string) + 1);

		utf8_to_wchar(string, outbuf, strlen(string));
		utf8_to_wchar(strtitle, wtitle, strlen(strtitle));
		window_add_message(window_get_main(), 
				   wtitle,
				   A_BOLD, outbuf);

		g_strfreev(infostring);
		g_free(string);
		g_free(outbuf);
		return TRUE;
	}

	if (g_ascii_strcasecmp("maxnumber", argv[1]) == 0) {
		if (argc == 3) {
			gint max = atoi(argv[2]);
			famahistory_setmax(max);
			g_sprintf(strtitle, "History infomation : %s",
				"Succeed in setting max number of history.");
		} else 
			mask |= HISINFO_USAGE;
		mask |= HISINFO_MAXNUMBER;
		infostring = famahistory_info_get(mask);
		string = g_strjoinv("\n", infostring);
		outbuf = g_new(wchar_t, strlen(string) + 1);
		utf8_to_wchar(string, outbuf, strlen(string));
		utf8_to_wchar(strtitle, wtitle, strlen(strtitle));
		window_add_message(window_get_main(),
				   wtitle, A_BOLD, outbuf);
		g_strfreev(infostring);
		g_free(string);
		g_free(outbuf);
		return TRUE;
	}
	if (g_ascii_strcasecmp("enable", argv[1]) == 0) {
		if (argc == 3) {
			gint enab = -1;
			if (g_ascii_strcasecmp("on", argv[2]) == 0) {
				enab = 1;
			} else if (g_ascii_strcasecmp("off", argv[2]) == 0) {
				enab = 0;
			} else
				mask |= HISINFO_USAGE;
			if (enab >= 0) {
				famahistory_enable(enab);
				g_sprintf(strtitle, "History infomation : %s",
					  "Succeed in setting of history enable flag.");
			}
		} else 
			mask |= HISINFO_USAGE;
		mask |= HISINFO_ENABLE;
		infostring = famahistory_info_get(mask);
		string = g_strjoinv("\n", infostring);
		outbuf = g_new(wchar_t, strlen(string) + 1);
		utf8_to_wchar(string, outbuf, strlen(string));
		utf8_to_wchar(strtitle, wtitle, strlen(strtitle));
		window_add_message(window_get_main(),
				   wtitle, A_BOLD, outbuf);
		g_strfreev(infostring);
		g_free(string);
		g_free(outbuf);
		return TRUE;
	}
	if (g_ascii_strcasecmp("list", argv[1]) == 0) {
		gint listnum;
		g_sprintf(strtitle, "History infomation : has %d records",
			  famahistory_number());
		if (argc == 3) {
			if (g_ascii_strcasecmp("all", argv[2]) == 0)
				listnum = famahistory_number();
			else
				listnum = atoi(argv[2]);
		} else { 
			listnum = DEFAULTLISTNUMBER;
		}
		infostring = famahistory_info_getlist(listnum);
		string = g_strjoinv("\n", infostring);
		outbuf = g_new(wchar_t, strlen(string) + 1);
		utf8_to_wchar(string, outbuf, strlen(string));
		utf8_to_wchar(strtitle, wtitle, strlen(strtitle));
		window_add_message(window_get_main(),
				   wtitle, A_BOLD, outbuf);
		g_strfreev(infostring);
		g_free(string);
		g_free(outbuf);
		return TRUE;
	}return TRUE;
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
	command_add("history", command_func_history);
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

