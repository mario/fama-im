#include "common.h"

#define TEXTWRAP_MAX_ROWS 256

GPtrArray *
textwrap_new(wchar_t * title)
{
	wchar_t *first_row;
	GPtrArray *m;
	gint slen;

	m = g_ptr_array_new();

	if (title == NULL)
		return m;

	slen = wcslen(title);

	first_row = g_new(wchar_t, slen + 1);
	wcsncpy(first_row, title, slen);
	first_row[slen] = '\0';

	g_ptr_array_add(m, first_row);

	return m;
}

void
textwrap_destroy(GPtrArray * m)
{
	g_ptr_array_free(m, TRUE);
}

void
textwrap_addrow(GPtrArray * m, const wchar_t * str, gint a, gint b)
{
	wchar_t *row;

	if (a == b)
		return;

	while (m->len >= TEXTWRAP_MAX_ROWS) {
		g_free(g_ptr_array_index(m, 0));
		g_ptr_array_remove_index(m, 0);
	}

	row = g_new(wchar_t, b - a + 1);
	wcsncpy(row, &str[a], b - a);
	row[b - a] = '\0';

	g_ptr_array_add(m, row);
}

gint
textwrap_append(GPtrArray * m, const wchar_t * text, guint max_width)
{
	gint c, width, last_word, start, text_len;

	text_len = wcslen(text);

	for (c = 0, last_word = 0, start = 0, width = 0; c < text_len; c++) {

		if (text[c] == L'\n') {
			textwrap_addrow(m, text, start, c);
			last_word = start = ++c;
			width = 0;
			continue;
		}

		if (text[c] == L' ')
			last_word = c + 1;

		width += wcwidth(text[c]);

		if (width >= max_width) {
			if (start != last_word) {
				textwrap_addrow(m, text, start, last_word);
				width = wcswidth(&text[last_word],
						 c - last_word);
				start = last_word;
			} else {
				textwrap_addrow(m, text, start, c);
				start = last_word = c;
				width = 0;
			}
		}
	}

	textwrap_addrow(m, text, start, c);

	/*
	 * Return number of rows in the textwrap
	 */

	return m->len;
}
