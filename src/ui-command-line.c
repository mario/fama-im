#include "common.h"
#include <string.h>

#define COMMAND_LINE_MAX_LENGHT 512

gint offset, len, ptr, max_width;

wchar_t cmdbuf[COMMAND_LINE_MAX_LENGHT];

wchar_t *
commandline_get_buffer()
{
	return cmdbuf;
}


void
commandline_update_width()
{
	max_width = get_max_x() - 2;
}

void
commandline_init()
{
	cmdbuf[0] = L'\0';
	offset = ptr = len = 0;
	commandline_update_width();
}

void
commandline_draw()
{
	ColorSettings *c = color_get();
	gint i, width, cursor_pos = 0;

	/*
	 * clear 
	 */
	mvhline(get_max_y() - 1, 1, ' ', get_max_x() - 1);

	while (ptr < offset)
		offset -= (max_width / 2);


	for (i = offset, width = 0; i <= len && width <= max_width; i++) {
		if (ptr == i)
			cursor_pos = width;

		width += wcwidth(cmdbuf[i]);
	}

	attron(c->command_line);
	mvaddnwstr(get_max_y() - 1, 1, &cmdbuf[offset], i);
	attroff(c->command_line);

	if (cursor_pos >= max_width - 1)
		offset += (max_width / 2);

	update_panels();
	move(get_max_y() - 1, 1 + cursor_pos);
	doupdate();
}

void
commandline_move_cursor(gint m)
{
	ptr += m;

	if (ptr < 0)
		ptr = 0;
	if (ptr >= len)
		ptr = len;

	commandline_draw();
}

void
commandline_delete()
{
	gint i;

	if (ptr <= 0)
		return;

	ptr--;
	len--;

	for (i = ptr; i < len; i++)
		cmdbuf[i] = cmdbuf[i + 1];

	cmdbuf[i] = 0x0;

	commandline_draw();

}

void
commandline_add_wch(wchar_t c)
{
	gint i;

	if (len >= COMMAND_LINE_MAX_LENGHT - 1)
		return;


	for (i = len; i > ptr; i--)
		cmdbuf[i] = cmdbuf[i - 1];

	cmdbuf[ptr] = c;
	cmdbuf[len + 1] = L'\0';

	len++;
	ptr++;

	commandline_draw();
}

void
commandline_set_cmd(gchar *line)
{
	gint strlength;
	if (!line) {
		cmdbuf[0] = L'\0';
		len = ptr = 0;
		return;
	}
	cmdbuf[0] = L'/';	
	strlength = strlen(line);
	utf8_to_wchar(line, cmdbuf + 1, strlength);
	cmdbuf[strlength + 1] = L'\0';
	len = strlength;
	ptr = len;
}

