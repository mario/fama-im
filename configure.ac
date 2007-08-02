AC_PREREQ(2.59)

AC_INIT([fama], [0.0.1], [http://www.fama-im.org/])
AM_INIT_AUTOMAKE(1.9)
AC_CONFIG_HEADERS([config.h])

AM_MAINTAINER_MODE

AC_PROG_CC
AC_PROG_CPP
AC_PROG_MAKE_SET
AC_PROG_INSTALL

AM_PATH_GLIB_2_0(2.0.0, [], [AC_MSG_FAILURE([You need the libglib-2.0])], [gobject])
PKG_CHECK_MODULES(TAPIOCA_CLIENT_GLIB, libtapioca-client-glib-0.14 >= 0.14.0.2, [], [AC_MSG_FAILURE([You need libtapioca-client-glib >= 0.14.0.2])])
AC_SUBST(TAPIOCA_CLIENT_GLIB_CFLAGS)
AC_SUBST(TAPIOCA_CLIENT_GLIB_LIBS)

AC_CHECK_LIB([ncurses], [stdscr], [], [AC_MSG_FAILURE([You need libncurse5])])
AC_CHECK_HEADERS([ncurses.h], [], [AC_MSG_FAILURE([ncurses.h missing])])

AC_CHECK_LIB([panel], [update_panels], [], [AC_MSG_FAILURE([You need libncurse5])])
AC_CHECK_HEADERS(panel.h, [], [AC_MSG_FAILURE([panel.h missing])])

AC_CHECK_LIB([ncursesw], stdscr, [], [AC_MSG_FAILURE([You need libncursew5])])
AC_CHECK_HEADERS([ncursesw/ncurses.h], [], [AC_MSG_FAILURE([ncursesw/ncurses.h missing])])


AC_CONFIG_FILES([Makefile
		 src/Makefile])
AC_OUTPUT
