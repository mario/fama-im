bin_PROGRAMS = fama

AM_CPPFLAGS = $(GLIB_CFLAGS) $(EMPATHY_CFLAGS)

fama_LDADD = $(GLIB_LIBS) $(EMPATHY_LIBS)

BUILT_SOURCES = empathy-marshal.h

fama_SOURCES = \
    empathy-chat.c \
    empathy-contactliststore.c \
    empathy-private-chat.c \
    main.c \
    mc-accounts.c \
    ui-clock.c \
    ui-color.c \
    ui-command-account.c \
    ui-command.c \
    ui-command-connect.c \
    ui-command-contact.c \
    ui-command-line.c \
    ui-focus.c \
    ui-history.c \
    ui-interface.c \
    ui-keyfile.c \
    ui-log.c \
    ui-logfile.c \
    ui-signal.c \
    ui-statusbar.c \
    ui-stdin-handler.c \
    ui-textwrap.c \
    ui-utf8.c \
    ui-util.c \
    ui-window.c \
    common.h \
    empathy-chat.h \
    empathy-contactliststore.h \
    empathy-private-chat.h \
    hintstring.h \
    mc-accounts.h \
    ui-history.h \
    ui-window.h \
    warningstring.h

%-marshal.h: %-marshal.list Makefile
	$(GLIB_GENMARSHAL) --header --prefix=$(subst -,_,$*)_marshal $< > $*-marshal.h


EXTRA_DIST = empathy-marshal.list indent.sh
CLEANFILES = $(BUILT_SOURCES)
