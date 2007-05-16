#include "common.h"

FamaFocus current_focus = FocusCommandLine;

void
focus_move_cursor()
{

}

void
focus_set(FamaFocus f)
{
	current_focus = f;
	focus_move_cursor();
}

FamaFocus
focus_get()
{
	return current_focus;
}
