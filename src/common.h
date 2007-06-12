#ifndef COMMON_H
#define COMMON_H 1

#define _GNU_SOURCE 1

#include <glib.h>
#include <glib-object.h>

#include <wchar.h>
#include <ncurses.h>
#include <panel.h>

#include <tapioca/tpa-client.h>

/**
 * FamaWindowType enumeration.
 * Specifies the type of content of a window.
 */
typedef enum {
	WindowTypeMain,
	WindowTypeConversation,
} FamaWindowType;

/**
 * FamaWindow.
 */
typedef struct {
	WINDOW *ncwin; /**< NCurses WINDOW pointer */
	PANEL *ncpanel; /**< NCurses PANEL pointer */

	wchar_t *title; /**< The title of the window */
	GPtrArray *messages; /**< An array of FamaMessage data-types */
	FamaWindowType type; /**< The FamaWindowType of the window */
	gboolean is_updated; /**< True if window has unread messages */

	TpaChannel *channel; /**< The channel associated with the window */
} FamaWindow;

/**
 * FamaFocus enumeration.
 * Specifies where the focus of the cursor will be set.
 */
typedef enum {
	FocusContactList,
	FocusCommandLine,
} FamaFocus;

/**
 * FamaMessage.
 * A window's content consists of a GPtrArray of this type.
 */
typedef struct {
	gint attr; /**< Attribute of the title */
	wchar_t *title; /**< Title of the message */
	wchar_t *message; /**< Message string */
} FamaMessage;

/**
 * FamaContactListGroup.
 * The contact-list contains an array of groups, which in turn
 * contain the actual items.
 */
typedef struct _FamaContactListGroup {
	TpaConnection *tpa_connection; /**< TpaConnection associated with the group */
	TpaContactList *tpa_contactlist; /**< TpaContactList associated with the group */
	GPtrArray *items; /**< An array of FamaContactListItem's */
} FamaContactListGroup;

/**
 * FamaContactListItem.
 * An item in the contact-list, returned by contactlist_get_selected() 
 */
typedef struct _FamaContactListItem {
	FamaContactListGroup *parent_group; /**< Parent group of the item */
	TpaContact *contact; /**< TpaContact associated with the item */
	wchar_t *text; /**< Text displayed in the contact-list */
	gint attr; /**< Attribute of the text displayed in the contact-list */
} FamaContactListItem;


/**
 * ColorSettings.
 * Each element contains an attribute for use with wattron()
 */
typedef struct {
	gint borders; /**< Bars and borders */
	gint command_line; /**< Command line */
	gint window_title; /**< Title of the conversation windows */
	gint outgoing_message; /**< Outgoing message title */
	gint incoming_message; /**< Incoming message title */
	gint status_available; /** Contact-list status 'available' */
	gint status_away; /** Contact-list status 'away' */
	gint status_busy; /** Contact-list status 'busy' */
	gint status_idle; /** Contact-list status 'idle' */
	gint status_offline; /** Contact-list status 'offline' */
	gint status_other; /** Contact-list other or unrecognized statuses */
} ColorSettings;

/**
 * CommandFunc.
 * Used by command-parser
 */
typedef gboolean(*CommandFunc) (gint argc, gchar ** argv);

/* Main.c */
gboolean init_all();
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
void contactlist_sort();
void contactlist_destroy();
void contactlist_free();
void contactlist_scroll(gint);
void contactlist_presence_updated_cb(TpaContact *, TpaContactPresence, gchar *);
void contactlist_authorization_requested_cb(TpaContactList *, TpaContact *);
void contactlist_subscription_accepted_cb(TpaContactList *, TpaContact *);
guint contactlist_presence_to_attr(TpaContactPresence);
FamaContactListItem *contactlist_get_selected();
FamaContactListGroup *contactlist_get_group(TpaConnection *);
void contactlist_reload_from_server(TpaConnection *);
void contactlist_remove_group(FamaContactListGroup *);



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
gboolean keyfile_read();
gboolean keyfile_write();
GKeyFile *keyfile_get();

#define FAMA_CONFIG_DIR                 ".fama"
#define FAMA_CONFIG_FILE                "config"
#define FAMA_ACCOUNTS			"accounts"

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

#define window_get_main() window_get_index(0)

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
gchar *connection_get_account(TpaConnection * conn);



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

/* Account.c */
gboolean account_init();
void account_destroy();
gchar **account_get_names();
gboolean account_get_profile(gchar *, TpaProfile **);
void account_add(gchar *, gchar *);

/* Focus.c */
void focus_set(FamaFocus);
FamaFocus focus_get();


#endif
