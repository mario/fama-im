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

void
color_init()
{
	gchar *tmp;
	GError *err = NULL;
	gint background;

	tmp = g_key_file_get_string(keyfile_get(), "colors", "background",
				    &err);
	if (tmp == NULL) {
		if (err->code == G_KEY_FILE_ERROR_INVALID_VALUE)
			g_error("invalid value: %s", err->message);
		g_clear_error(&err);
		background = -1;
	} else {
		if ((background = color_str_to_int(tmp)) == 8)
			background = -1;

		g_free(tmp);
	}

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

	tmp = g_key_file_get_string(keyfile_get(), "colors", "borders", &err);
	if (tmp == NULL) {
		if (err->code == G_KEY_FILE_ERROR_INVALID_VALUE)
			g_error("invalid value: %s", err->message);
		g_clear_error(&err);
		settings.borders = COLOR_PAIR(6);
	} else {
		settings.borders = COLOR_PAIR(color_str_to_int(tmp));
		g_free(tmp);
	}

	tmp = g_key_file_get_string(keyfile_get(), "colors", "command_line",
				    &err);
	if (tmp == NULL) {
		if (err->code == G_KEY_FILE_ERROR_INVALID_VALUE)
			g_error("invalid value: %s", err->message);
		g_clear_error(&err);
		settings.command_line = 0;
	} else {
		settings.command_line = COLOR_PAIR(color_str_to_int(tmp));
		g_free(tmp);
	}

	tmp = g_key_file_get_string(keyfile_get(), "colors", "window_title",
				    &err);
	if (tmp == NULL) {
		if (err->code == G_KEY_FILE_ERROR_INVALID_VALUE)
			g_error("invalid value: %s", err->message);
		g_clear_error(&err);
		settings.window_title = COLOR_PAIR(3);
	} else {
		settings.window_title = COLOR_PAIR(color_str_to_int(tmp));
		g_free(tmp);
	}

	tmp = g_key_file_get_string(keyfile_get(), "colors", "message_heading",
				    &err);
	if (tmp == NULL) {
		if (err->code == G_KEY_FILE_ERROR_INVALID_VALUE)
			g_error("invalid value: %s", err->message);
		g_clear_error(&err);
		settings.message_heading = COLOR_PAIR(2);
	} else {
		settings.message_heading = COLOR_PAIR(color_str_to_int(tmp));
		g_free(tmp);
	}

	tmp = g_key_file_get_string(keyfile_get(), "colors", "message_text",
				    &err);
	if (tmp == NULL) {
		if (err->code == G_KEY_FILE_ERROR_INVALID_VALUE)
			g_error("invalid value: %s", err->message);
		g_clear_error(&err);
		settings.message_text = 0;
	} else {
		settings.message_text = COLOR_PAIR(color_str_to_int(tmp));
		g_free(tmp);
	}

	tmp = g_key_file_get_string(keyfile_get(), "colors", "status_available",
				    &err);
	if (tmp == NULL) {
		if (err->code == G_KEY_FILE_ERROR_INVALID_VALUE)
			g_error("invalid value: %s", err->message);
		g_clear_error(&err);
		settings.status_available = COLOR_PAIR(2);
	} else {
		settings.status_available = COLOR_PAIR(color_str_to_int(tmp));
		g_free(tmp);
	}

	tmp = g_key_file_get_string(keyfile_get(), "colors", "status_away",
				    &err);
	if (tmp == NULL) {
		if (err->code == G_KEY_FILE_ERROR_INVALID_VALUE)
			g_error("invalid value: %s", err->message);
		g_clear_error(&err);
		settings.status_away = COLOR_PAIR(6);
	} else {
		settings.status_away = COLOR_PAIR(color_str_to_int(tmp));
		g_free(tmp);
	}

	tmp = g_key_file_get_string(keyfile_get(), "colors", "status_idle",
				    &err);
	if (tmp == NULL) {
		if (err->code == G_KEY_FILE_ERROR_INVALID_VALUE)
			g_error("invalid value: %s", err->message);
		g_clear_error(&err);
		settings.status_idle = COLOR_PAIR(3);
	} else {
		settings.status_idle = COLOR_PAIR(color_str_to_int(tmp));
		g_free(tmp);
	}

	tmp = g_key_file_get_string(keyfile_get(), "colors", "status_busy",
				    &err);
	if (tmp == NULL) {
		if (err->code == G_KEY_FILE_ERROR_INVALID_VALUE)
			g_error("invalid value: %s", err->message);
		g_clear_error(&err);
		settings.status_busy = COLOR_PAIR(5);
	} else {
		settings.status_busy = COLOR_PAIR(color_str_to_int(tmp));
		g_free(tmp);
	}

	tmp = g_key_file_get_string(keyfile_get(), "colors", "status_offline",
				    &err);
	if (tmp == NULL) {
		if (err->code == G_KEY_FILE_ERROR_INVALID_VALUE)
			g_error("invalid value: %s", err->message);
		g_clear_error(&err);
		settings.status_offline = 0;
	} else {
		settings.status_offline = COLOR_PAIR(color_str_to_int(tmp));
		g_free(tmp);
	}

	tmp = g_key_file_get_string(keyfile_get(), "colors", "status_other",
				    &err);
	if (tmp == NULL) {
		if (err->code == G_KEY_FILE_ERROR_INVALID_VALUE)
			g_error("invalid value: %s", err->message);
		g_clear_error(&err);
		settings.status_other = COLOR_PAIR(1);
	} else {
		settings.status_other = COLOR_PAIR(color_str_to_int(tmp));
		g_free(tmp);
	}

}

ColorSettings *
color_get(void)
{
	return &settings;
}
