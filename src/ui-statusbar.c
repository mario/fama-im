#include "common.h"
#include "ui-window.h"
#include <string.h>
#include <libempathy/empathy-contact.h>
#include "empathy-contactliststore.h"
#include "empathy-private-chat.h"

void
statusbar_draw()
{
	const gchar *time_str;
	gchar *status_str;
	wchar_t *wbuf;
	gint wbuf_len;
	ColorSettings *c = color_get();
	EmpathyContact *contact = NULL;
	const gchar *contactname = NULL, *contactid = NULL;
	FamaFocus f;
	FamaWindow *win;
	status_str = window_create_status_string();
	time_str = clock_get_time();

	f = focus_get();
	switch (f) {
	case FocusCommandLine:
		win = window_get_current();
		/**FIXME:Assume it's a private chat arbitrarily,chat room will go wrong*/ 
		if (win && win->empathychat)
			contact = empathy_private_chat_get_contact(EMPATHY_PRIVATE_CHAT(win->empathychat));
		break;
	case FocusContactList:
		contact = empathy_contact_list_store_get_selected(list_store);
		break;
	}
        
	if (contact) {
		contactname = empathy_contact_get_name(contact);
		contactid = empathy_contact_get_id(contact);
		wbuf_len = strlen(status_str) + strlen(time_str) + 
			strlen(contactname) + strlen(contactid) + 15 * 2;
		/** don't use constant number in here,like 15 * 2*/
	} else {
		wbuf_len = strlen(status_str) + strlen(time_str) + 12;
	}
	wbuf = g_new(wchar_t, wbuf_len);
	if (contact)
		swprintf(wbuf, wbuf_len - 1, L"[%s] Act: [%s] contact [%s][%s]", 
			time_str, status_str, contactname, contactid);
	else
		swprintf(wbuf, wbuf_len - 1, L"[%s] Act: [%s]", 
			time_str, status_str);

	attron(c->borders | A_REVERSE);

	attron(A_REVERSE);
	mvhline(get_max_y() - 2, 0, BORDER, get_max_x());
	mvwaddwstr_with_maxwidth(stdscr, get_max_y() - 2, 1, wbuf,
				 get_max_x() - 2);
	attroff(A_REVERSE | c->borders);

	update_panels();
	doupdate();

	g_free(status_str);
	g_free(wbuf);
}
