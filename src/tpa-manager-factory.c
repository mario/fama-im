
#include "common.h"

TpaManagerFactory *factory = NULL;

TpaManagerFactory *
manager_factory_get()
{
	return factory;
}

void
manager_factory_init()
{
	factory = tpa_manager_factory_new();

	if (!factory)
		g_error("Failed to create Connection Manager Factory!\n"
			"You probably have to start a D-Bus session..");
		
}

void
manager_factory_destroy()
{
	GPtrArray *managers = tpa_manager_factory_get_all_managers(factory);

	if (managers != NULL) {
		int i;

		for (i = 0; i < managers->len; i++)
			g_object_unref(g_ptr_array_index(managers, i));

		g_ptr_array_free(managers, TRUE);
	}

	if (factory != NULL) {
		g_object_unref(factory);
		factory = NULL;
	}
}
