#include <stdio.h>
#include <errno.h>
#include <readline/history.h>
#include <sys/stat.h>
#include <string.h>
#include "ui-history.h"

#define STREQ(a, b) ((a)[0] == (b)[0] && strcmp(a, b) == 0) 
#define DEFAULTMAXHISTNUMBER	500
#define LIMITMAXHISTNUMBER	10000

FamaHistory famahis;

void famahistory_really_add __P((char *));

void
famahistory_initialize(int maxhistnumber)
{
	history_search_delimiter_chars = ";&()|<>";
	history_inhibit_expansion_function = NULL;
	famahis.history_control = OMIT_SAMELINE;
	history_base = 0;
	famahistory_setmax(maxhistnumber);
	using_history();
}

void
famahistory_reinit(int interact)
{
	famahis.history_record_enable = interact != 0;
	history_inhibit_expansion_function = NULL;
}

void
famahistory_disable()
{
	famahis.history_record_enable = 0;
}

void famahistory_setmax(int maxhistnumber)
{
	if(maxhistnumber <= 0)
		famahis.maxhistnumber = DEFAULTMAXHISTNUMBER;
	
	famahis.maxhistnumber = min(LIMITMAXHISTNUMBER, maxhistnumber);
}

/* Check LINE against what HISTCONTROL says to do.  Returns 1 if the line
 *    should be saved; 0 if it should be discarded. */
int
famahistory_check_control(char *line)
{
	HIST_ENTRY *temp;
	int r;

	if (*line == ' ')
		return 0;
	switch (famahis.history_control){
	case 0:			/* nothing */
		return 1;
	case 1:			/* ignoredups */
		temp = current_history();

		r = (temp == 0 || STREQ(temp->line, line) == 0);

		return r;
	}

	return 0;
}
/* Just check LINE against HISTCONTROL and HISTIGNORE and add it to the
 *    history if it's OK.  Used by `history -s' as well as maybe_add_history().
 *       Returns 1 if the line was saved in the history, 0 otherwise. */
int
famahistory_check_add(char *line, int force)
{
	if (force || famahistory_check_control(line)){
		famahistory_add(line);
		return 1;
	}	
	return 0;
}

/* Add a line to the history list.
 * The variable COMMAND_ORIENTED_HISTORY controls the style of history
 * remembering;  when non-zero, and LINE is not the first line of a
 * complete parser construct, append LINE to the last history line instead
 * of adding it as a new line. */
void
famahistory_add(char *line)
{
	int histnumber;
	HIST_ENTRY *first;
	histnumber = famahistory_number();
	if (histnumber >= famahis.maxhistnumber){
		first = famahistory_remove(history_base);
		free_history_entry(first);
	}
	famahistory_really_add(line);
}

void
famahistory_really_add(char *line)
{
	add_history(line);
	famahis.history_lines_in_session++;
	using_history();
}

int
famahistory_loadfile(char *filename)
{
	struct stat buf;
	int err;
	/* Truncate history file for fama  which desire it.*/
	/* Read the history in maxhistnumber into the history list. */
	if(!filename || !(*filename) || stat(filename, &buf) != 0)
		return -1;
	if((err = read_history(filename)) != 0)
		return err;
	using_history();
	famahis.history_lines_in_file = where_history();
	strcpy(famahis.filename, filename);
	return 0;
}

int
famahistory_savefile(char *filename)
{
	int err;
	if(!filename || !(*filename))
		return -1;
	if ((err = write_history(filename)) != 0)
		return err;
	return 0;
}

int
famahistory_number()
{
	using_history();
	return history_length;
}

HIST_ENTRY *
famahistory_get_last()
{
	HIST_ENTRY *he;
	using_history();
	he = history_get(history_length - 1 + history_base);
	return he;
}

HIST_ENTRY *famahistory_get(int offset)
{
	using_history();
	return history_get(offset);
}

HIST_ENTRY *famahistory_current(void)
{
	using_history();
	return current_history();
}

HIST_ENTRY *famahistory_remove(int offset)	
{
	HIST_ENTRY *he =  remove_history(offset);
	using_history();
	famahis.history_lines_in_session--;
	return he;
}
