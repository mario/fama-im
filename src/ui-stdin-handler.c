#include "common.h"

gboolean
stdin_handle_input(GIOChannel * source, GIOCondition cond, gpointer d)
{
	GError *err = NULL;
	gunichar unichar;
	gchar mbseq[32], *mbscmd, **argv;
	wchar_t wchar[3], *cmdbuf;
	gsize size;
	gint argc;

	/*
	 * Read from the stdin as long as data is available. The reading end is 
	 * also in non-blocking mode, so if we have consumed all data, 
	 * the read returns G_IO_STATUS_AGAIN. 
	 */
	while (get_wch(&unichar) != ERR) {

		if (unichar == 0x0a || unichar == KEY_ENTER) {
			/*
			 * Do stuff with the buffer 
			 */
			cmdbuf = commandline_get_buffer();

			if (cmdbuf[0] == L'\0') {
				/*
				 * Start conversation with selected contact
				 */
				g_message
					("In the future this will start a new chat"
					 " with the selected contact");
			} else if (cmdbuf[0] != L'/') {
				/*
				 * Send message to the conversation at the current
				 * window.
				 */
				window_add_message(window_get_current(),
						   L"Test output", A_BOLD,
						   cmdbuf);
			} else {
				/*
				 * Convert wchar_t to gchar and then into an argument list
				 */
				mbscmd = utf8_from_wchar(cmdbuf, NULL, 0);
				if (g_shell_parse_argv
				    (&mbscmd[1], &argc, &argv, &err)
				    == FALSE) {
					g_warning("Could not parse command: %s",
						  err->message);
					g_clear_error(&err);
				} else {
					/*
					 * Execute command!!
					 */
					if (command_execute(argc, argv) ==
					    FALSE)
						g_warning
							("Command '%s' not found!",
							 argv[0]);

					g_strfreev(argv);
				}
				g_free(mbscmd);
			}


			/*
			 * Re-initialize the command-line 
			 */
			commandline_init();
			commandline_draw();
		} else if (unichar == KEY_BACKSPACE) {
			commandline_delete();
		} else if (unichar == KEY_LEFT) {
			commandline_move_cursor(-1);
		} else if (unichar == KEY_RIGHT) {
			commandline_move_cursor(1);
		} else if (unichar == KEY_UP) {
			contactlist_scroll(-1);
		} else if (unichar == KEY_DOWN) {
			contactlist_scroll(1);
		} else if (unichar == '\t') {
			/*
			 * Ignore for the time being 
			 */
		} else {
			size = g_unichar_to_utf8(unichar, mbseq);
			mbseq[size] = '\0';

			utf8_to_wchar(mbseq, wchar, 2);

			/*
			 * Add character to command-line buffer 
			 */
			commandline_add_wch(wchar[0]);
		}

	}
	return (TRUE);
}


void
stdin_handler_setup()
{
	GIOChannel *g_stdin;
	GError *err = NULL;

	g_stdin = g_io_channel_unix_new(fileno(stdin));

	g_io_channel_set_encoding(g_stdin, get_interface_encoding(), &err);
	if (err != NULL) {
		g_error("g_io_channel_set_encoding failed %s\n", err->message);
	}

	/*
	 * put the reading end also into non-blocking mode 
	 */
	g_io_channel_set_flags(g_stdin,
			       g_io_channel_get_flags(g_stdin) |
			       G_IO_FLAG_NONBLOCK, &err);

	if (err != NULL) {
		g_error("g_io_set_flags failed %s\n", err->message);
	}

	/*
	 * register the reading end with the event loop 
	 */
	g_io_add_watch(g_stdin, G_IO_IN | G_IO_PRI, stdin_handle_input, NULL);
}
