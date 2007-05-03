
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
		g_error("failed to create Connection Manager Factory!");
}
