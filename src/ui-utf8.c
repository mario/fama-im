/* TODO:
 * Make utf8_to_wchar use dynamic allocation!!
 */

#include <stdlib.h>
#include <string.h>

#include "common.h"

#define UNLIMITED 32000

gchar *encoding = NULL;

void
set_interface_encoding(gchar * enc)
{
	if (encoding != NULL)
		g_free(encoding);

	encoding = enc;
}

gchar *
get_interface_encoding(void)
{
	return encoding;
}


wchar_t *
utf8_to_wchar(const gchar * str, wchar_t * outbuf, gsize outbuf_size)
{
	gchar *locale_str;
	gsize bytes_read, bytes_written, numwcs;

	if (str == NULL) {
		*outbuf = L'\0';
		return outbuf;
	}

	g_assert(g_utf8_validate(str, -1, NULL) == TRUE);

	locale_str =
		g_convert_with_fallback(str, -1, encoding, "UTF-8", "?",
					&bytes_read, &bytes_written, NULL);
	g_assert(locale_str != NULL);

	numwcs = mbstowcs(outbuf, locale_str, outbuf_size);
	g_free(locale_str);

	g_assert(numwcs != (size_t) - 1);

	outbuf[numwcs] = L'\0';

	return outbuf;
}


/*
 * If outbuf == NULL, then a buffer will be allocated dynamically
 */

gchar *
utf8_from_wchar(const wchar_t * str, gchar * outbuf, gsize outbuf_size)
{
	gchar *utf8_str, *tmp;
	gsize bytes_read, bytes_written;

	if (str == NULL) {
		if (outbuf == NULL)
			return NULL;

		*outbuf = '\0';
		return outbuf;
	}

	if (outbuf == NULL) {
		bytes_written = wcsrtombs(NULL, &str, 0, NULL);
		tmp = g_new(gchar, bytes_written + 1);
		outbuf_size = UNLIMITED;
	} else {
		tmp = g_new(gchar, outbuf_size + 1);
	}

	bytes_written = wcsrtombs(tmp, &str, outbuf_size, NULL);
	g_assert(bytes_written != (size_t) - 1);

	utf8_str =
		g_convert_with_fallback(tmp, -1, "UTF-8", encoding, "?",
					&bytes_read, &bytes_written, NULL);
	g_assert(utf8_str != NULL);

	g_free(tmp);

	if (outbuf == NULL)
		return utf8_str;

	strncpy(outbuf, utf8_str, outbuf_size);
	outbuf[bytes_written] = L'\0';

	g_free(utf8_str);

	return outbuf;
}
