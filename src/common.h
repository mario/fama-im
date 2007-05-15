#ifndef COMMON_H
#define COMMON_H 1

#define _GNU_SOURCE 1

#include <glib.h>
#include <glib-object.h>

#include <wchar.h>
#include <ncurses.h>
#include <panel.h>

#include <tapioca/tpa-base.h>
#include <tapioca/tpa-client.h>

typedef enum {
	WindowTypeMain,
	WindowTypeConversation,
} FamaWindowType;

typedef struct {
	WINDOW *ncwin;
	PANEL *ncpanel;

	wchar_t *title;
	GPtrArray *messages;
	FamaWindowType type;
	gboolean is_updated;

	TpaChannel *channel;
} FamaWindow;

typedef struct {
	gint attr;
	wchar_t *title;
	wchar_t *message;
} FamaMessage;

typedef struct _FamaContactListItem {
	TpaContact *contact;
	TpaConnection *connection;
	wchar_t *text;
	gint attr;
} FamaContactListItem;


typedef struct {
	gint borders;
	gint command_line;
	gint window_title;
	gint message_heading;
	gint message_text;
	gint status_available;
	gint status_away;
	gint status_busy;
	gint status_idle;
	gint status_offline;
	gint status_other;
} ColorSettings;

typedef gboolean(*CommandFunc) (gint argc, gchar ** argv);

/* Main.c */
gboolean init_all(gpointer data);
void stop_main_loop(void);

/* Interface.c */

void destroy_interface();
void init_interface();
void draw_interface();
void redraw_interface();
gboolean interface_is_initialized();
int get_max_y();
int get_max_x();

#define BORDER ' '

/* Contactlist.c */
void contactlist_init();
void contactlist_set_width(gint);
gint contactlist_get_width();
void contactlist_draw();
void contactlist_destroy();
void contactlist_add_item(TpaConnection *, TpaContact *, const wchar_t *, int);
void contactlist_scroll(gint);
void contactlist_presence_updated_cb(TpaContact *, TpaContactPresence, gchar *);
guint contactlist_presence_to_attr(TpaContactPresence);
FamaContactListItem *contactlist_get_selected();


/* Utf8.c */
wchar_t *utf8_to_wchar(const gchar *, wchar_t *, gsize);
gchar *utf8_from_wchar(const wchar_t *, gchar *, gsize);
void set_interface_encoding(gchar *);
gchar *get_interface_encoding(void);


/* Message.c */
GPtrArray *textwrap_new(wchar_t *);
void textwrap_destroy();
gint textwrap_append(GPtrArray *, const wchar_t *, guint);

/* Signal.c */
void signal_handler_setup();

/* Stdin-handler.c */
void stdin_handler_setup();

/* Command-line.c */
void commandline_init();
void commandline_add_wch(wchar_t);
void commandline_update_width();
wchar_t *commandline_get_buffer();
void commandline_draw();
void commandline_move_cursor(gint);
void commandline_delete();

/* Keyfile.c */
gint keyfile_read();
gint keyfile_write();
GKeyFile *keyfile_get();

/* Window.c */
FamaWindow *window_new(FamaWindowType);
FamaWindow *window_get_current();
FamaWindow *window_get_index(gint);
FamaWindow *window_find_channel(TpaChannel * c);
void window_append_rows(FamaWindow *, GPtrArray *, gint);
void window_add_message(FamaWindow *, wchar_t *, gint, wchar_t *);
void window_set_current(FamaWindow *);
void window_set_title(FamaWindow *, wchar_t *);
void window_draw_title_bar();
void window_destroy(FamaWindow *);
void window_resize_all();
gchar *window_create_status_string();


/* Log.c */
void log_init();
void log_get_time(gchar *, gsize);

/* Command.c */
void command_init();
void command_add(gchar *, CommandFunc);
gboolean command_execute(gint, gchar **);

/* Color.c */
void color_init();
ColorSettings *color_get();

/* Connection.c */
void connection_disconnect_all();
TpaConnection *connection_connect(gchar *, gchar *);


/* Factory-manager.c */
TpaManagerFactory *manager_factory_get();
void manager_factory_init();
void manager_factory_destroy();

/* Channel.c */
void channel_send_message(TpaTextChannel *, gchar *);
void channel_created_cb(TpaConnection * conn, TpaChannel * channel);

/* Clock.c */
gboolean clock_cb(gpointer data);
const gchar *clock_get_time();

/* Statusbar */
void statusbar_draw();

/* Util.c */
int mvwaddwstr_with_maxwidth(WINDOW *, int, int, const wchar_t *, int);

#endif
