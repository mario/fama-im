#include "common.h"

int
mvwaddwstr_with_maxwidth(WINDOW *win, int y, int x, const wchar_t *wstr, int n)
{
	int width, i;

	for (width=0, i=0; i<wcslen(wstr) && width < n; i++)
		width += wcwidth(wstr[i]);

	return mvwaddnwstr(win, y, x, wstr, i);
}
