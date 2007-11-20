#ifndef FAMAHISTORY_H
#define FAMAHISTORY_H

#include <stdio.h>
#include <errno.h>
#include <readline/history.h>

#define MAXFILEPATHLEN 256

typedef struct _famahistory{
	/** Declarations of fama history.*/
	int history_record_enable; 
	/** The number of lines that has been added to this history session.*/
	int history_lines_in_session; 
	/** The number of lines that has been read from the history file.*/
	int history_lines_in_file;
	/** TRUE means to append the history to the history file at fama
	 exit.*/
	int history_force_append;
	/** A nit for picking at history saving.
	 *    Value of 0 means save all lines parsed by the shell on the history.
	 *    Value of 1 means save all lines that do not match the last line
	 *    saved. */
	int history_control;
	/** filename of current history session*/
	char filename[MAXFILEPATHLEN];
	/** max number of this history session,default value is 500*/
	int maxhistnumber;
}FamaHistory;

/** Initialize a history session and indicate the max number of history lines*/
void famahistory_initialize __P((int maxhistnumber));
void famahistory_reinit __P((int interact));
void famahistory_disable();
/** set max number of history lines*/
void famahistory_setmax(int maxhistnumber);
/** check whether the input line is equal to the previous one,
 * force to add this line if varible force is 1*/
int famahistory_check_add __P((char *line, int force));
/** add this line into history session*/
void famahistory_add __P((char *line));
/** remove an indicated line*/
HIST_ENTRY *famahistory_remove(int offset);	
/** return number of history lines*/
int famahistory_number();
int famahistory_currrent_offset(void);
/** get a history entry by offset,offset as 0 and number is invalid*/
HIST_ENTRY *famahistory_get __P((int offset));
/** get current line entry*/
HIST_ENTRY *famahistory_get_current();
/** get the last history entry*/
HIST_ENTRY *famahistory_get_last();
/** get current line's offset of the list*/
int famahistory_currrent_offset(void);
/** load history file into history list,a line at a time*/
int famahistory_loadfile(char *filename);
/** save history file*/
int famahistory_savefile(char *filename);

/** Control type of history_control.*/
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

#endif

