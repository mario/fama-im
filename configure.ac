AC_PREREQ(2.59)
AC_INIT([fama], 0.0.4, [http://www.fama-im.org/])
AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(1.9)

EMPATHY_REQUIRED=0.21.3

AM_MAINTAINER_MODE

AC_PROG_CC

AM_PATH_GLIB_2_0(2.10.0, , , [gobject])
PKG_CHECK_MODULES(EMPATHY, [libempathy >= $EMPATHY_REQUIRED])

AC_CHECK_LIB([ncursesw], [newwin])
AC_CHECK_LIB([panelw], [update_panels])
AC_CHECK_LIB([history], [using_history])

AC_CHECK_HEADERS([ncursesw/panel.h])
AC_CHECK_HEADERS([ncursesw/ncurses.h])
AC_CHECK_HEADERS([readline/history.h])

GLIB_GENMARSHAL=`$PKG_CONFIG glib-2.0 --variable=glib_genmarshal`
AC_SUBST(GLIB_GENMARSHAL)

AC_OUTPUT([
    Makefile
    src/Makefile
])
