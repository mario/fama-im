#include "common.h"
#include "ui-window.h"
#include "ui-history.h"
#include "empathy-contactliststore.h"
#include "empathy-chat.h"

void
handle_input_on_command_line(gunichar c)
{
	GError *err = NULL;

	if (c == 0x0a || c == KEY_ENTER) {
		wchar_t *cmdbuf = commandline_get_buffer();

		if (cmdbuf[0] != L'/') {
			FamaWindow *win;

			/*
			 * Send message to the conversation at the current
			 * window.
			 */

			if (cmdbuf[0] != L'\0') {
				win = window_get_current();
				
				if (win->empathychat != NULL) {
					gchar *contents;
					contents =
						utf8_from_wchar(cmdbuf, NULL, 0);
					empathy_chat_send(win->empathychat, contents);
				}
			}
		} else {
			gchar *mbscmd, **argv;
			gint argc;

			/*
			 * Convert wchar_t to gchar and then into an argument list
			 */
			mbscmd = utf8_from_wchar(cmdbuf, NULL, 0);
			if (g_shell_parse_argv(&mbscmd[1], &argc, &argv, &err)
			    == FALSE) {
				g_warning("Could not parse command: %s",
					  err->message);
				g_clear_error(&err);
			} else {
				/*
 				 * add to history session
 				 */
				famahistory_command_add(argc, argv);

				/*
				 * Execute command!!
				 */
				if (command_execute(argc, argv) == FALSE)
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

	} else if (c == KEY_BACKSPACE) {
		commandline_delete();
	} else if (c == KEY_LEFT) {
		commandline_move_cursor(-1);
	} else if (c == KEY_RIGHT) {
		commandline_move_cursor(1);
	} else if (c == KEY_UP) {
		famahistory_command_loadpre();
		commandline_draw();
	} else if (c == KEY_DOWN) {
		famahistory_command_loadnext();
		commandline_draw();
	} else if (c == '\t') {
		/*
		 * Ignore for the time being 
		 */
	} else {
		gchar mbseq[32];
		wchar_t wchar[3];
		gsize size;

                size = g_unichar_to_utf8(c, mbseq);
		mbseq[size] = '\0';

		utf8_to_wchar(mbseq, wchar, 2);

		/*
		 * Add character to command-line buffer 
		 */
		commandline_add_wch(wchar[0]);
	}
}

void
handle_input_on_contact_list(gunichar c)
{
	if (c == 0x0a || c == KEY_ENTER) {

		//FamaContactListItem *a;
		EmpathyContact *contact;
		/*
		 * Start conversation with selected contact
		 */
		if ((contact = empathy_contact_list_store_get_selected(list_store)) == NULL)
			return;
		
		empathy_contact_list_launch_channel(contact);
		g_object_unref(contact);
                /*
                 * Change focus to command-line after initializing a conversation
                 */

                focus_set(FocusCommandLine);
	} else if (c == KEY_UP) {
		empathy_contactlistwin_scroll(list_store, -1);
		statusbar_draw();
	} else if (c == KEY_DOWN) {
		empathy_contactlistwin_scroll(list_store, 1);
		statusbar_draw();
	}
}

gboolean
stdin_handle_input(GIOChannel * source, GIOCondition cond, gpointer d)
{
	gunichar unichar;
	gint index = 0, highestindex=0;
        FamaFocus f;
        FamaWindow * curwindow, *mainwindow;
        gchar *winclose[] = {"window", "close", NULL};

	while (get_wch(&unichar) != ERR) {
		f = focus_get();
                switch (unichar) {
                     case 0x06: //change focus, ctrl+f
			  f = (f ==
			     FocusCommandLine) ? FocusContactList :
			  FocusCommandLine;
			  focus_set(f);
                        break;
                     case 0x17: //destroy window, ctrl+w
                          command_execute(2, winclose);
                          break;
                    case 0x0E: //next window, ctrl+n
                          mainwindow = window_get_index(0);
                          curwindow = window_get_current();
                          index = get_window_index(curwindow);
                          if (window_get_index(index+1) != NULL)
                               window_set_current(window_get_index(index+1));
                          else
                               window_set_current(mainwindow);
                          break;
                     case 0x02: //previous window, ctrl+b
                          mainwindow = window_get_index(0);
                          curwindow = window_get_current();
                          index = get_window_index(curwindow);
                          while (window_get_index(highestindex) != NULL)
                               highestindex++;
                          if (window_get_index(index-1) != NULL)
                               window_set_current(window_get_index(index-1));
                          else
                               window_set_current(window_get_index(highestindex-1));
                          break;
                }
		statusbar_draw();
		
                if (unichar < 31 && unichar != 0x0a)
                     continue;

		switch (f) {
		case FocusCommandLine:
			handle_input_on_command_line(unichar);
			break;
		case FocusContactList:
			handle_input_on_contact_list(unichar);
			break;
		}
        }

	return TRUE;
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
