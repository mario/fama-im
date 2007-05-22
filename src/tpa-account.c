#include "common.h"

gboolean
account_add_line_to_file(GIOChannel * file, TpaParameters * param, gchar * name,
			 GValue * val)
{
	GError *err = NULL;
	gchar *line = NULL;

	switch (G_VALUE_TYPE(val)) {
	case G_TYPE_STRING:{
			gchar *v =
				tpa_parameters_get_default_value_as_string
				(param, name);
			line = g_strdup_printf("%s: %s\n", name, v);
			break;
		}
	case G_TYPE_UINT:{
			guint v =
				tpa_parameters_get_default_value_as_uint(param,
									 name);

			line = g_strdup_printf("%s: %u\n", name, v);
			break;
		}
	case G_TYPE_INT:{
			gint v = tpa_parameters_get_default_value_as_int(param,
									 name);

			line = g_strdup_printf("%s: %d\n", name, v);
			break;
		}
	case G_TYPE_BOOLEAN:{
			gboolean v =
				tpa_parameters_get_default_value_as_boolean
				(param, name);
			line = g_strdup_printf("%s: %d\n", name, v);
			break;
		}
	default:
		break;
	}


	if (line) {
		if (g_io_channel_write_chars(file, line, -1, NULL, &err) !=
		    G_IO_STATUS_NORMAL) {
			g_warning("cannot write account parameter: %s",
				  err->message);
			g_clear_error(&err);
			return FALSE;
		}

		g_free(line);
	}

	return TRUE;
}

void
account_add(gchar * protocol, gchar * identifier)
{
	TpaManager *manager;
	TpaParameters *parameters;
	GError *err = NULL;
	GIOChannel *file;
	GPtrArray *names;
	GValue *value;
	gchar *name, *path;
	gint i;

	manager =
		tpa_manager_factory_get_manager(manager_factory_get(),
						protocol);
	if (!manager) {
		g_warning("No connection manager found for '%s'", protocol);
		return;
	}

	parameters = tpa_manager_get_protocol_parameters(manager, protocol);
	if (!parameters) {
		g_warning("Cannot get parameters for '%s'", protocol);
		return;
	}

	names = tpa_parameters_get_required(parameters);

	path = g_strdup_printf("%s/%s/%s", g_get_home_dir(), FAMA_CONFIG_DIR,
			       FAMA_CONFIG_ACCOUNTS_DIR);

	if (g_mkdir_with_parents(path, 0700) != 0) {
		g_warning("Cannot create directories for '%s'", path);
		return;
	}

	g_free(path);

	path = g_strdup_printf("%s/%s/%s/%s", g_get_home_dir(), FAMA_CONFIG_DIR,
			       FAMA_CONFIG_ACCOUNTS_DIR, identifier);


	file = g_io_channel_new_file(path, "w", &err);
	if (!file) {
		g_warning("cannot open file %s: %s", path, err->message);
		g_free(path);
		g_clear_error(&err);
		return;
	}

	for (i = 0; i < names->len; i++) {
		name = g_ptr_array_index(names, i);
		value = tpa_parameters_get_default_value(parameters, name);
		if (account_add_line_to_file(file, parameters, name, value) ==
		    FALSE)
			return;
	}

	if (g_io_channel_shutdown(file, TRUE, &err) != G_IO_STATUS_NORMAL) {
		g_warning("cannot close channel: %s", err->message);
		g_clear_error(&err);
		return;
	}
	g_io_channel_unref(file);

	g_message("Written account template to '%s'", path);

	g_free(path);
	g_ptr_array_free(names, TRUE);
}
