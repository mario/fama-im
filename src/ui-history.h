#ifndef FAMAHISTORY_H
#define FAMAHISTORY_H

#include <stdio.h>
#include <errno.h>
#include <readline/history.h>
#include <glib.h>

#define MAXFILEPATHLEN 256

typedef struct _famahistory{
	/** Declarations of fama history.*/
	gboolean history_record_enable; 
	/** The number of lines that has been added to this history session.*/
	gint history_lines_in_session; 
	/** The number of lines that has been read from the history file.*/
	gint history_lines_in_file;
	/** TRUE means to append the history to the history file at fama
	 exit.*/
	gint history_force_append;
	/** A nit for picking at history saving.
	 *    Value of 0 means save all lines parsed by the shell on the history.
	 *    Value of 1 means save all lines that do not match the last line
	 *    saved. */
	gint history_control;
	/** filename of current history session*/
	gchar histfilename[MAXFILEPATHLEN];
	/** max number of this history session,default value is 500*/
	gint maxhistnumber;
	/** current pos*/
	gint currentpos;
}FamaHistory;

/** argc is arguments' number,argv is array of arguments*/
void famahistory_command_add __P((gint argc, gchar ** argv));

/** Initialize a history session and load the max number of history lines
 * from config file*/
gboolean famahistory_command_init();

/** Load previous history and fill it in cmdline*/
gboolean famahistory_command_loadpre();

/** Load previous history and fill it in cmdline*/
gboolean famahistory_command_loadnext();

/** save current session to history file*/
void famahistory_command_savesession();

/** get basic infomation of history according the input combination*/
gchar **famahistory_info_get(gulong infomask);

/** get array of history lines*/
gchar **famahistory_info_getlist(gint listnum);

/** set enable flag of this session*/
void famahistory_enable(gboolean enable);

/** get enable flag of this session*/
gboolean famahistory_getenable();
/** Control type of history_control.*/

/** set max number of history lines*/
void famahistory_setmax(gint maxhistnumber);

/** get max number of history lines*/
gint famahistory_getmax();

/** return number of history lines*/
int famahistory_number();

#define OMIT_SAMELINE	1
/*
#define min(x,y)  \
({    \
        typeof(x) _x = (x);     \
        typeof(y) _y = (y);     \
        (void) (&_x == &_y);    \
        _x < _y ? _x : _y;      \
})*/

#define max(a,b) ( ( (a) >= (b) ) ? (a) : (b))
#define min(a,b) ( ( (a) <= (b) ) ? (a) : (b))

/** famahistory_info_get use them*/
#define HISINFO_MAXNUMBER	(0x1)
#define	HISINFO_ENABLE		(0x1 << 1)
#define	HISINFO_LISTNUM		(0x1 << 2) 
#define	HISINFO_USAGE		(0x1 << 3) 
	
#endif

