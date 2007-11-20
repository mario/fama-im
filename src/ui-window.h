#include "common.h"
#include <glib.h>
#include <libmissioncontrol/mc-account.h>
#include <libempathy/empathy-utils.h>
#include "empathy-chat.h"

EmpathyChat *empathy_chat_window_find_chat(McAccount *account, const gchar *id);
FamaWindow *empathy_chat_appendwindow(EmpathyChat *chat);
void empathy_chat_present(EmpathyChat *chat);

FamaWindow *window_new(FamaWindowType);
FamaWindow *window_get_current();
FamaWindow *window_get_index(gint);
FamaWindow *window_find_empathychat(EmpathyChat *chat);
void window_append_rows(FamaWindow *, GPtrArray *, gint);
void window_add_message(FamaWindow *, wchar_t *, gint, wchar_t *);
void window_set_current(FamaWindow *);
void window_set_title(FamaWindow *, wchar_t *);
void window_draw_title_bar();
void window_draw_all_titlebars();
void window_destroy(FamaWindow *);
void window_destroy_all_windows();
void window_resize_all();
gchar *window_create_status_string();
gint get_window_index(FamaWindow *);

#define window_get_main() window_get_index(0)

