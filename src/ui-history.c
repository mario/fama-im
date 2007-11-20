#include <stdio.h>
#include <errno.h>
#include <readline/history.h>
#include <sys/stat.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <string.h>
#include "common.h"
#include "ui-history.h"
#include "warningstring.h"
#include "hintstring.h"

#define STREQ(a, b) ((a)[0] == (b)[0] && strcmp(a, b) == 0) 
#define LIMITMAXHISTNUMBER		10000
#define COMMAND_STRING_MAX_LENGHT 	512
#define MAX_HISTORY_NUMBER		50

FamaHistory famahis;

void famahistory_really_add __P((gchar *));
void famahistory_reinit __P((gint interact));
void famahistory_disable();
/** check whether the input line is equal to the previous one,
 * force to add this line if varible force is 1*/
gint famahistory_check_add __P((gchar *line, gint force));
/** add this line into history session*/
void famahistory_add __P((gchar *line));
/** remove an indicated line*/
HIST_ENTRY *famahistory_remove(gint offset);	
/** get a history entry by offset,offset as 0 and number is invalid*/
HIST_ENTRY *famahistory_get __P((gint offset));
/** get current line entry*/
HIST_ENTRY *famahistory_get_current();
/** get the last history entry*/
HIST_ENTRY *famahistory_get_last();
/** get current line's offset of the list*/
gint famahistory_current_offset(void);
/** load history file into history list,a line at a time*/
gint famahistory_loadfile(gchar *filename);
/** save history file*/
gint famahistory_savefile(gchar *filename);

void
famahistory_reinit(gint interact)
{
	famahis.history_record_enable = interact != 0;
	history_inhibit_expansion_function = NULL;
}

void
famahistory_enable(gboolean enable)
{
	famahis.history_record_enable = enable;
}

gboolean
famahistory_getenable()
{
	return famahis.history_record_enable;
}

void famahistory_setmax(gint maxhistnumber)
{
	if(maxhistnumber <= 0)
		famahis.maxhistnumber = MAX_HISTORY_NUMBER;
	
	famahis.maxhistnumber = min(LIMITMAXHISTNUMBER, maxhistnumber);
}

/* Check LINE against what HISTCONTROL says to do.  Returns 1 if the line
 *    should be saved; 0 if it should be discarded. */
gint
famahistory_check_control(gchar *line)
{
	HIST_ENTRY *temp;
	gint r;

	if (*line == ' ')
		return 0;
	switch (famahis.history_control){
	case 0:			/* nothing */
		return 1;
	case 1:			/* ignoredups */
		temp = previous_history();

		r = (temp == 0 || STREQ(temp->line, line) == 0);

		return r;
	}

	return 0;
}
/* Just check LINE against HISTCONTROL and add it to the
 *    history if it's OK. Returns 1 if the line was saved in the history, 
 *    0 otherwise. */
gint
famahistory_check_add(gchar *line, gint force)
{
	gint histnumber;
	histnumber = famahistory_number();
	famahis.currentpos = histnumber;
	if (!famahis.history_record_enable)
		return 0;
	if (force || famahistory_check_control(line)){
		famahistory_add(line);
		return 1;
	}	
	return 0;
}

/* Add a line to the history list. */
void
famahistory_add(gchar *line)
{
	gint histnumber, i, n;
	HIST_ENTRY *first;
	histnumber = famahistory_number();
	if (histnumber >= famahis.maxhistnumber - 1){
		n = histnumber - famahis.maxhistnumber + 1;
		for (i = 0; i < n; i++) {
			first = famahistory_remove(history_base);
			free_history_entry(first);
		}
	}
	famahistory_really_add(line);
}

void
famahistory_really_add(gchar *line)
{
	gint histnumber;
	add_history(line);
	histnumber = famahistory_number();
	famahis.history_lines_in_session++;
	using_history();
	famahis.currentpos = histnumber;
}

gint
famahistory_loadfile(gchar *filename)
{
	struct stat buf;
	gint err;
	/* Truncate history file for fama  which desire it.*/
	/* Read the history in maxhistnumber into the history list. */
	if(!filename || !(*filename) || stat(filename, &buf) != 0)
		return -1;
	if((err = read_history(filename)) != 0)
		return err;
	using_history();
	famahis.history_lines_in_file = where_history();
	famahis.currentpos = where_history();
	return 0;
}

gint
famahistory_savefile(gchar *filename)
{
	gint err;
	if(!filename || !(*filename))
		return -1;
	if ((err = write_history(filename)) != 0)
		return err;
	return 0;
}

gint
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

HIST_ENTRY *
famahistory_get(gint offset)
{
	using_history();
	return history_get(offset);
}

HIST_ENTRY *
famahistory_current(void)
{
	using_history();
	return current_history();
}

HIST_ENTRY *
famahistory_remove(gint offset)	
{
	HIST_ENTRY *he =  remove_history(offset);
	using_history();
	famahis.history_lines_in_session--;
	return he;
}

void
famahistory_command_add(gint argc, gchar ** argv)
{
	gint i, offset = 0, arglen = 0;
	gchar cmdstring[COMMAND_STRING_MAX_LENGHT] = {0};
	g_assert(argc >= 1);
	//to avoid pasword being recorded in history file
	//So discard the last argument
	if (g_ascii_strcasecmp(argv[0], "connect") == 0 && argc >= 3)
		argc = 2;
	for(i = 0; i < argc; i++) {
		arglen = strlen(argv[i]);
		g_strlcpy(cmdstring + offset, argv[i], arglen + 1);
		offset += arglen;
		g_strlcpy(cmdstring + offset, " ", 2);
		offset += 1;
	}
	famahistory_check_add(cmdstring, FALSE);
}

gboolean
famahistory_command_loadprofile()
{
	GError *err = NULL;

	famahis.maxhistnumber = MAX_HISTORY_NUMBER;

	g_sprintf(famahis.histfilename, "%s/%s/%s", g_get_home_dir(),
	       FAMA_CONFIG_DIR, FAMA_HISTORY_FILE);
	//get the max history number
	famahis.maxhistnumber = g_key_file_get_integer(keyfile_get(), 
		"history", "history_maxnumber", &err);
	famahis.maxhistnumber = max(MAX_HISTORY_NUMBER, famahis.maxhistnumber);
	return TRUE;	
}

gboolean
famahistory_command_init()
{
	famahistory_command_loadprofile();
	/** famahistory has its default max number,if input histmaxnumber is 0
 	 * famahistory will use its default value*/
	history_search_delimiter_chars = ";&()|<>";
	history_inhibit_expansion_function = NULL;
	famahis.history_control = OMIT_SAMELINE;
	famahis.history_record_enable = TRUE;
	history_base = 0;
	if (famahistory_loadfile(famahis.histfilename) != 0)
		return FALSE;
	using_history();
	return TRUE;
}

gboolean
famahistory_command_loadpre()
{
	HIST_ENTRY *he;
	gint offset, totalcnt;	
	totalcnt = famahistory_number();
	offset = famahis.currentpos;// where_history();
	offset--;
	if (offset == history_base - 1)
		offset = totalcnt + history_base;
	he = famahistory_get(offset);
	if (he)
		commandline_set_cmd(he->line);
	else
		commandline_set_cmd(NULL);
	famahis.currentpos = offset;
	return TRUE;
}

gboolean
famahistory_command_loadnext()
{
	HIST_ENTRY *he;
	gint offset, totalcnt;	
	totalcnt = famahistory_number();
	offset = famahis.currentpos;// where_history();
	offset++;
	if (offset == history_base + totalcnt + 1)
		offset = history_base;
	he = famahistory_get(offset);
	if (he)
		commandline_set_cmd(he->line);
	else
		commandline_set_cmd(NULL);
	famahis.currentpos = offset;
	return TRUE;
}

/** save current session to history file*/
void 
famahistory_command_savesession()
{
	gint rt;
	rt = famahistory_savefile(famahis.histfilename);
	if (rt != 0)
		g_warning(WARNSTRING_CANTSAVEHISTORY);
}

gchar **
famahistory_info_get(gulong infomask)
{
	gint count = 0, i = 0;
	gchar **strlist;
	while (i < 32) {
		count += (infomask >> i) & 0x00000001;
		i++;
	}
	/* the first line is prompt of history usage*/
	count++; 
	strlist = g_new(gchar *, count + 1); 
	count = 0;
	if (infomask & HISINFO_USAGE) {
		strlist[count] = g_strdup_printf(HINTSTR_HISTORYUSAGE);
		count++;
	}
	if (infomask & HISINFO_MAXNUMBER) {
		strlist[count] = 
			g_strdup_printf(HINTSTR_HISTORYMAXNUM(famahis.maxhistnumber));
		count++;
	}
	if (infomask & HISINFO_ENABLE) {
		strlist[count] = 
			g_strdup_printf(HINTSTR_HISTORYENABLE(famahis.history_record_enable));
		count++;
	}		
	if (infomask & HISINFO_LISTNUM) {
		strlist[count] = 
			g_strdup_printf(HINTSTR_HISTORYLISTNUM(famahistory_number()));
		count++;
	}	
	strlist[count] = NULL;
	return strlist;
}

gchar **
famahistory_info_getlist(gint listnum)
{
	gint totaln, i;
	gchar **strlist;
	HIST_ENTRY *he;
	totaln = famahistory_number();
	g_assert(listnum >= 0 && listnum <= totaln);
	strlist = g_new(gchar *, listnum + 1);
	for (i = 0; i < listnum; i++) {
		he = famahistory_get(totaln - listnum + i);
		strlist[i] = g_strdup_printf("    (%d)%s",
			(totaln - listnum + 1 + i), he->line);
	}
	strlist[listnum] = NULL;
	return strlist;
}

