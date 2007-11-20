#include "common.h"

#include <libtelepathy/tp-conn.h>
#include <libtelepathy/tp-chan.h>
#include <libmissioncontrol/mc-account.h>
#include <libmissioncontrol/mc-protocol.h>
#include <libmissioncontrol/mc-account-monitor.h>
#include <libmissioncontrol/mission-control.h>

#include <libempathy/empathy-debug.h>
#include <libempathy/empathy-idle.h>
#include <libempathy/empathy-chandler.h>
#include <libempathy/empathy-utils.h>
#include <libempathy/empathy-tp-chat.h>

/** remove an account according to its unique name. */
#define ACCTYPE_BYUNINAME	0
/** remove an account according to its display name. */
#define ACCTYPE_BYDISNAME	1

gboolean mc_init();
void mc_uninit();

/** get accounts list*/
gchar **mc_get_account_names();

/** get protocols list*/
gchar **mc_get_protocol_names();

/** remove an account according to the given string and remove type*/
gboolean mc_remove_account(const gchar *accstring, gint removetype);

/** add an account into mc session*/
gboolean mc_account_add(const gchar * profile_name, const gchar * accstring,
	const gchar *passwd);
gboolean mc_account_connect(const gchar *accountstr,gint acctype, gboolean connectenable);
