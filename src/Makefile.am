bin_PROGRAMS = fama

fama_SOURCES = \
		main.c tpa-account.c tpa-channel.c \
		tpa-connection.c tpa-manager-factory.c ui-clock.c \
		ui-color.c ui-command-account.c ui-command.c \
		ui-command-connect.c ui-command-line.c ui-command-status.c \
		ui-contactlist.c ui-focus.c ui-interface.c \
		ui-keyfile.c ui-log.c ui-signal.c \
		ui-statusbar.c ui-stdin-handler.c ui-textwrap.c \
		ui-utf8.c ui-util.c ui-window.c

fama_CPPFLAGS = $(GLIB_CFLAGS) $(TAPIOCA_CLIENT_GLIB_CFLAGS)

fama_LDADD = $(GLIB_LIBS) $(TAPIOCA_CLIENT_GLIB_LIBS)

EXTRA_DIST = indent.sh
