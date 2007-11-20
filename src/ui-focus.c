#include "common.h"

FamaFocus current_focus = FocusCommandLine;

void
focus_set(FamaFocus f)
{
	FamaFocus old_focus;

	old_focus = current_focus;
	current_focus = f;

	switch (old_focus) {
	case FocusCommandLine:
		mvaddch(get_max_y() - 1, 0, ' ');
		break;
	case FocusContactList:
		contactlist_draw();
		break;
	}

	switch (current_focus) {
	case FocusCommandLine:
		mvaddch(get_max_y() - 1, 0, '>');
		break;
	case FocusContactList:
		contactlist_draw();
		break;
	}

	update_panels();
	doupdate();
}

FamaFocus
focus_get()
{
	return current_focus;
}
