#include "common.h"

ColorSettings settings;

gint
color_str_to_int(gchar * str)
{
	if (g_ascii_strcasecmp(str, "black") == 0)
		return 0;
	if (g_ascii_strcasecmp(str, "red") == 0)
		return 1;
	if (g_ascii_strcasecmp(str, "green") == 0)
		return 2;
	if (g_ascii_strcasecmp(str, "yellow") == 0)
		return 3;
	if (g_ascii_strcasecmp(str, "blue") == 0)
		return 4;
	if (g_ascii_strcasecmp(str, "magenta") == 0)
		return 5;
	if (g_ascii_strcasecmp(str, "cyan") == 0)
		return 6;
	if (g_ascii_strcasecmp(str, "white") == 0)
		return 7;
	if (g_ascii_strcasecmp(str, "default") == 0)
		return 8;

	g_error("No such color: %s!\n", str);
	return -1;
}

gint
color_with_fallback(gchar * item, gint default_color)
{
	GError *err = NULL;
	gchar *tmp;
	gint ret = 0;

	tmp = g_key_file_get_string(keyfile_get(), "colors", item, &err);
	if (tmp == NULL) {
		if (err->code == G_KEY_FILE_ERROR_INVALID_VALUE)
			g_error("invalid value: %s", err->message);
		g_clear_error(&err);
		ret = default_color;
	} else {
		ret = COLOR_PAIR(color_str_to_int(tmp));
		g_free(tmp);
	}

	return ret;
}

void
color_init()
{
	gint background;

	background = color_with_fallback("background", -1);
	if (background == 8)
		background = -1;

	start_color();
	use_default_colors();

	init_pair(0, COLOR_BLACK, background);
	init_pair(1, COLOR_RED, background);
	init_pair(2, COLOR_GREEN, background);
	init_pair(3, COLOR_YELLOW, background);
	init_pair(4, COLOR_BLUE, background);
	init_pair(5, COLOR_MAGENTA, background);
	init_pair(6, COLOR_CYAN, background);
	init_pair(7, COLOR_WHITE, background);
	init_pair(8, -1, background);

	settings.borders = color_with_fallback("borders", COLOR_PAIR(6));
	settings.command_line = color_with_fallback("command_line", 0);
	settings.window_title =
		color_with_fallback("window_title", COLOR_PAIR(6));
	settings.window_title_uploaded =
		color_with_fallback("window_title_uploaded", COLOR_PAIR(5));
	settings.status_active_window = 
		color_with_fallback("status_active_window", COLOR_PAIR(6));
	settings.outgoing_message =
		color_with_fallback("outgoing_message", COLOR_PAIR(2));
	settings.incoming_message =
		color_with_fallback("incoming_message", COLOR_PAIR(6));
	settings.incoming_message =
		color_with_fallback("incoming_automsg", COLOR_PAIR(4));
	settings.status_available =
		color_with_fallback("status_available", COLOR_PAIR(1));
	settings.status_away =
		color_with_fallback("status_away", COLOR_PAIR(6));
	settings.status_idle =
		color_with_fallback("status_idle", COLOR_PAIR(3));
	settings.status_busy =
		color_with_fallback("status_busy", COLOR_PAIR(5));
	settings.status_group =
		color_with_fallback("status_group", COLOR_PAIR(6));
	settings.status_offline = color_with_fallback("status_offline", COLOR_PAIR(8));
	settings.status_other =
		color_with_fallback("status_available", COLOR_PAIR(1));
}

ColorSettings *
color_get(void)
{
	return &settings;
}
