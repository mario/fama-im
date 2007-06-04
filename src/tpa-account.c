#include "common.h"

GKeyFile *accounts;
gchar *accounts_path;

gboolean
account_init()
{
	GError *err = NULL;
	gchar *path;

	path = g_strdup_printf("%s/%s", g_get_home_dir(), FAMA_CONFIG_DIR);
	if (g_mkdir_with_parents(path, 0700) != 0) {
		g_warning("Cannot create directories for '%s'", path);
		return FALSE;
	}
	g_free(path);

	accounts_path =
		g_strdup_printf("%s/%s/%s", g_get_home_dir(), FAMA_CONFIG_DIR,
				FAMA_ACCOUNTS);


	accounts = g_key_file_new();
	if (g_file_test(accounts_path, G_FILE_TEST_IS_REGULAR)) {
		if (g_key_file_load_from_file
		    (accounts, accounts_path,
		     G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS,
		     &err)
		    == FALSE) {
			g_warning("Cannot load accounts file %s: %s", path,
				  err->message);
			return FALSE;
		}
	}

	return TRUE;
}

gchar **
account_get_names()
{
	return g_key_file_get_groups(accounts, NULL);
}

void
account_destroy()
{
	g_key_file_free(accounts);
	g_free(accounts_path);
}

gboolean
account_get_profile(gchar * account, TpaProfile ** profile_ptr)
{
	TpaManager *manager;
	TpaParameter *parameter;
	TpaProfile *profile;
	GError *err = NULL;
	GPtrArray *keys;
	gchar *profile_name, *v;
	const gchar *key;
	gint i;

	g_assert(accounts != NULL && accounts_path != NULL);

	profile_name =
		g_key_file_get_string(accounts, account, "profile", &err);
	if (profile_name == NULL) {
		g_warning("could not get profile_name: %s", err->message);
		return FALSE;
	}

	manager =
		tpa_manager_factory_get_manager(manager_factory_get(),
						profile_name);
	if (!manager) {
		g_warning("No connection manager found for '%s'", profile_name);
		return FALSE;
	}

	profile = tpa_manager_get_profile(manager, profile_name);
	if (!profile) {
		g_warning("Cannot get profile for '%s'", profile_name);
		return FALSE;
	}

	g_object_unref(manager);

	keys = tpa_profile_get_all_parameters(profile);
	for (i = 0; i < keys->len; i++) {
		parameter = g_ptr_array_index(keys, i);
		key = tpa_parameter_get_name(parameter);

		if (g_ascii_strcasecmp(key, "account") == 0)
			continue;

		v = g_key_file_get_value(accounts, account, key, &err);

		if (err == NULL) {
			if (g_ascii_strcasecmp(v, "NULL") != 0) {
				if (!tpa_parameter_set_value_as_string
				    (parameter, v))
					g_warning
						("could not set parameter '%s'!",
						 key);
			}
			g_free(v);
		} else {
			g_warning("could not get key '%s': %s", key,
				  err->message);
			g_clear_error(&err);
		}
	}

	parameter = tpa_profile_get_parameter(profile, "account");
	tpa_parameter_set_value_as_string(parameter, account);

	g_ptr_array_free(keys, TRUE);

	*profile_ptr = profile;

	return TRUE;
}

void
account_add(gchar * profile_name, gchar * account)
{
	TpaManager *manager;
	TpaParameter *parameter;
	TpaProfile *profile;
	GError *err = NULL;
	GPtrArray *parameters;
	gsize length;
	gchar *data, *v;
	const gchar *name;
	gint i;

	manager =
		tpa_manager_factory_get_manager(manager_factory_get(),
						profile_name);
	if (!manager) {
		g_warning("No connection manager found for '%s'", profile_name);
		return;
	}

	profile = tpa_manager_get_profile(manager, profile_name);
	if (!profile) {
		g_warning("Cannot get parameter for '%s'", profile_name);
		return;
	}

	g_object_unref(manager);

	g_assert(accounts != NULL && accounts_path != NULL);

	/*
	 * Set the profile key in account file.
	 */

	g_key_file_set_string(accounts, account, "profile", profile_name);

	parameters = tpa_profile_get_all_parameters(profile);
	for (i = 0; i < parameters->len; i++) {
		parameter = g_ptr_array_index(parameters, i);
		name = tpa_parameter_get_name(parameter);

		if (g_ascii_strcasecmp(name, "account") == 0)
			continue;

		v = tpa_parameter_get_default_value_as_string(parameter);

		if (v == NULL)
			g_key_file_set_string(accounts, account, name, "NULL");
		else {
			g_key_file_set_string(accounts, account, name, v);
			g_free(v);
		}
	}

	data = g_key_file_to_data(accounts, &length, &err);
	if (data == NULL) {
		g_warning("Cannot get keyfile data: %s", err->message);
		return;
	}

	if (g_file_set_contents(accounts_path, data, length, &err) == FALSE) {
		g_warning("Cannot write to %s: %s", accounts_path,
			  err->message);
		return;
	}

	g_message("Written account template to '%s'", accounts_path);

	g_free(data);
	g_ptr_array_free(parameters, TRUE);
}
