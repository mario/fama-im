#include "common.h"
#include "ui-window.h"
#include "mc-accounts.h"
#include <string.h>
#include <config.h>
#include <glib/gprintf.h>
#include "empathy-chat.h"
#include "empathy-private-chat.h"

#define DEBUG_DOMAIN "FamaMain"
#define BUS_NAME "org.gnome.Empathy.Chat"
#define OBJECT_PATH "/org/freedesktop/Telepathy/ChannelHandler"

static void
on_params_foreach (const gchar * key, const GValue * value);

static void new_channel_cb(EmpathyChandler *chandler,
	TpConn          *tp_conn,
	TpChan          *tp_chan,
	MissionControl  *mc);

static McAccount *mc_getaccount_bydisplayname(const gchar *accstring);

typedef struct _empathy_content {
	MissionControl    *mc; 
	McAccountMonitor  *monitor; 
	EmpathyIdle       *idle;
	EmpathyChandler   *chandler;
}MC_CONTENT;

MC_CONTENT telemc;	//telepathy mc

static void service_ended_cb(MissionControl *mc, 
        gpointer        user_data);

static void operation_error_cb(MissionControl *mc, 
	guint           operation_id, 
	guint           error_code, 
	gpointer        user_data);

static void start_mission_control(EmpathyIdle *idle);

static void account_enabled_cb(McAccountMonitor *monitor, 
	gchar            *unique_name, 
	EmpathyIdle      *idle);

static void
create_salut_account (void);

gboolean 
mc_init()
{
	gboolean           no_connect = FALSE;

	/* Setting up MC */
	telemc.monitor = mc_account_monitor_new();
	telemc.mc = empathy_mission_control_new();
	telemc.idle = empathy_idle_new();
	g_signal_connect(telemc.monitor, "account-enabled",
		G_CALLBACK(account_enabled_cb),
		telemc.idle);
	g_signal_connect(telemc.mc, "ServiceEnded",
		G_CALLBACK(service_ended_cb),
		NULL);
	g_signal_connect(telemc.mc, "Error",
		G_CALLBACK(operation_error_cb),
		NULL);

	if (!no_connect) {
		start_mission_control(telemc.idle);
	}
	/* Setting up channel handler  */
	telemc.chandler = empathy_chandler_new(BUS_NAME, OBJECT_PATH);
	g_signal_connect(telemc.chandler, "new-channel",
		G_CALLBACK (new_channel_cb),
		telemc.mc);
	return TRUE;
}

static void 
service_ended_cb(MissionControl *mc, 
                 gpointer        user_data) 
{ 
	fama_debug(DEBUG_DOMAIN, "Mission Control stopped"); 
} 

static void 
operation_error_cb(MissionControl *mc, 
		    guint           operation_id, 
		    guint           error_code, 
		    gpointer        user_data) 
{ 
	fama_debug(DEBUG_DOMAIN, "Error code %d during operation %d", 
		error_code, 
		operation_id); 
} 

static void 
start_mission_control(EmpathyIdle *idle) 
{ 
	McPresence presence; 

	presence = empathy_idle_get_state (idle); 

	if (presence > MC_PRESENCE_OFFLINE) { 
		/* MC is already running and online, nothing to do */ 
		return; 
	} 

	empathy_idle_set_state(idle, MC_PRESENCE_AVAILABLE); 
} 

static void 
account_enabled_cb(McAccountMonitor *monitor, 
                     gchar            *unique_name, 
                     EmpathyIdle      *idle) 
{ 
	empathy_debug(DEBUG_DOMAIN, "Account enabled: %s", unique_name); 
	start_mission_control(idle); 
} 

void
mc_uninit()
{
	GList	*accounts, *l; 
	accounts = mc_accounts_list();
	for (l = accounts; l; l = l->next) {
		McAccount *account;

		account = l->data;
		if (!mc_account_is_complete(account)) {
			/* FIXME: Warn the user the account is not complete
			*        and is going to be removed. */
			mc_account_delete(account);
		}

		g_object_unref(account);
	}
	g_list_free(accounts);

	empathy_idle_set_state(telemc.idle, MC_PRESENCE_OFFLINE);
//	mc_account_finalize(telemc.mc);
	g_object_unref(telemc.chandler); 
	g_object_unref(telemc.monitor); 
	g_object_unref(telemc.mc); 
	g_object_unref(telemc.idle); 
}

gchar **
mc_get_account_names()
{
	gchar	**strlist;
	GList	*accounts, *l; 
	gint	totallen, i = 0;
	gchar	tempstr[256], statusstr[256];
	accounts = mc_accounts_list(); 
	totallen = g_list_length(accounts);
	strlist = g_new0(gchar *, totallen + 1); 

	for (l = accounts; l; l = l->next) { 
		McAccount	*account; 
		const gchar	*name, *protname; 
		GValue		*accval = NULL;
		gboolean	accenable;
		GHashTable	*params;
		TelepathyConnectionStatus  status; 
		McProfile	*accprofile;
		McProtocol	*accproto;
		account = l->data; 

		name = mc_account_get_display_name(account); 
		if (!name) { 
			continue;
		} 
		/*get its protocol*/
		accprofile = mc_account_get_profile(account);
	 	accproto = mc_profile_get_protocol(accprofile);
 		protname = mc_protocol_get_name(accproto);

		status = mission_control_get_connection_status(telemc.mc, account, NULL); 
		accenable = mc_account_is_enabled(account);
		switch(status) {
		case TP_CONN_STATUS_CONNECTED:
			g_sprintf(statusstr, "%s", "TP_CONN_STATUS_CONNECTED"); break;
		case TP_CONN_STATUS_CONNECTING:
			g_sprintf(statusstr, "%s", "TP_CONN_STATUS_CONNECTING"); break;
		case TP_CONN_STATUS_DISCONNECTED:
			g_sprintf(statusstr, "%s", "TP_CONN_STATUS_DISCONNECTED"); break;
		case NUM_TP_CONN_STATUSES: 
			g_sprintf(statusstr, "%s", "NUM_TP_CONN_STATUSES"); break;
		}

		params = mc_account_get_params (account);
		if (params == NULL) {
			g_warning ("Error: Failed to retreive params.");
		}
		else {
			accval = g_hash_table_lookup(params, "account");
			//g_hash_table_foreach (params, (GHFunc) on_params_foreach, NULL);
		}

		g_sprintf(tempstr, "DisplayName=%s UniName=%s status=%s %s proto=%s", 
			g_value_get_string(accval), name, statusstr, 
			accenable ? "Connected" : "Disconnected", protname);
		strlist[i] = g_strdup_printf(tempstr);
		if(params)
			g_hash_table_destroy(params);
			
		g_object_unref(accprofile);
		g_object_unref(accproto);
		//g_object_unref(account);
		i++;
	}
	mc_accounts_list_free(accounts);
	return strlist;
}

static void
on_params_foreach (const gchar * key, const GValue * value)
{
  switch (G_VALUE_TYPE (value))
    {
    case G_TYPE_INT:
      printf ("        (int) %s = %d\n", key, g_value_get_int (value));
      break;
    case G_TYPE_UINT:
      printf ("        (int) %s = %d\n", key, g_value_get_uint (value));
      break;
    case G_TYPE_BOOLEAN:
      printf ("       (bool) %s = %s\n", key,
              g_value_get_boolean (value) ? "true" : "false");
      break;
    case G_TYPE_STRING:
      printf ("     (string) %s = %s\n", key, g_value_get_string (value));
      break;
    default:
      g_warning ("Unknown account setting type.");
    }
}

gchar **
mc_get_protocol_names()
{
	gchar	**strlist;
	GList	*l, *profiles; 
	gint	totallen, i = 0;
	profiles = mc_profiles_list();

	totallen = g_list_length(profiles);
	strlist = g_new0(gchar *, totallen + 1); 
	
	for(l = profiles; l; l = l->next) {
		const gchar 	*mcdisname, *mcuniname;
		McProfile 	*mcprofile;
		McProtocol	*protocol;
		mcprofile = l->data;
		g_assert(mcprofile);
		/* Check if the CM is installed, otherwise skip that profile.
		* Workaround SF bug #1688779 */
		protocol = mc_profile_get_protocol (mcprofile);
		if (!protocol) {
			continue;
		}
		g_object_unref (protocol);

		mcdisname = mc_profile_get_display_name(mcprofile);
		mcuniname = mc_profile_get_unique_name(mcprofile);
		strlist[i] = g_strdup_printf("%s [Display : %s]", 
			mcuniname, mcdisname);
		i++;
	}
	mc_protocols_free_list(profiles);
	return strlist;
}

gboolean
mc_remove_account(const gchar *accstring, gint removetype)
{
	McAccount *account; 

	if(ACCTYPE_BYUNINAME == removetype) {
		account = mc_account_lookup(accstring);
	}else if(ACCTYPE_BYDISNAME == removetype) {
		account = mc_getaccount_bydisplayname(accstring);
	}
	if(!account)
		return FALSE;
	if (!mc_account_is_complete(account)) { 
		g_object_unref(account);
		return FALSE;
	}
	mc_account_delete(account); 

	return TRUE; 
} 

static McAccount *
mc_getaccount_bydisplayname(const gchar *accstring)
{
	GList	*accounts, *l; 
	gint	i = 0;
	McAccount	*account = NULL, *tempaccount;
	accounts = mc_accounts_list(); 

	for (l = accounts; l; l = l->next) { 
		const gchar	*name; 
		GValue		*accval = NULL;
		GHashTable	*params;
		tempaccount = l->data; 

		name = mc_account_get_display_name(tempaccount); 
		if (!name) { 
			continue;
		} 

		params = mc_account_get_params(tempaccount);
		if (params == NULL) {
			g_warning ("%sError: Failed to retreive params.", __FUNCTION__);
		} else {
			accval = g_hash_table_lookup(params, "account");
		}
		if (g_ascii_strcasecmp(g_value_get_string(accval), accstring) == 0)
			account = tempaccount;
		if(params)
			g_hash_table_destroy(params);
		if(account)
			break;
		i++;
	}
	mc_accounts_list_free(accounts);
	return account;
}	

gboolean
mc_account_add(const gchar * profile_name, const gchar * accstring, const gchar *passwd)
{
	McProfile   *profile, *tmpprofile;
	McAccount   *account, *tmpaccount;
	gchar       displaystr[256];
	const gchar *str;
	wchar_t *outbuf;
	profile = mc_profile_lookup(profile_name);
	if (!profile) {
		g_warning("Cannot get profile for '%s'", profile_name);
		return FALSE;
	}
	tmpaccount = mc_getaccount_bydisplayname(accstring);
	if (tmpaccount) {
		/** it means maybe this account has existed*/
		tmpprofile = mc_account_get_profile(tmpaccount);
		if (tmpprofile) {
			if (mc_profile_get_unique_name(tmpprofile) ==
				mc_profile_get_unique_name(profile)) {
		
				g_object_unref(tmpaccount);
				g_object_unref(tmpprofile);
				g_warning("This account %s has existed", accstring);
				return FALSE;	
			}
			g_object_unref(tmpprofile);
		}
		g_object_unref(tmpaccount);
	}
	/* Create account */
	account = mc_account_create(profile);
	if (!account) {
		g_warning("Cannot create account for '%s'", profile_name);
	}
	str = mc_account_get_unique_name(account);
	mc_account_set_display_name(account, str);

	mc_account_set_param_string(account, "account", accstring);
	mc_account_set_param_string(account, "password", passwd);
	
	g_sprintf(displaystr, "Create account %s uniquename=%s successfully!",
		accstring, str);

	outbuf = g_new(wchar_t, strlen(displaystr) + 1);

	utf8_to_wchar(displaystr, outbuf, strlen(displaystr));
	window_add_message(window_get_main(), L"Add account:", 
			   A_BOLD, outbuf);

	g_object_unref(account);
	g_object_unref(profile);
	g_free(outbuf);
	return TRUE;
}

gboolean
mc_account_connect(const gchar *accountstr,
		gint acctype, gboolean connectenable)
{
	McAccount *account;
	gboolean   enable;

	if(ACCTYPE_BYUNINAME == acctype) {
		account = mc_account_lookup(accountstr);
	}else if(ACCTYPE_BYDISNAME == acctype) {
		account = mc_getaccount_bydisplayname(accountstr);
		if(account)		
			g_object_ref(account);
	}
	if(!account)
	{
		g_warning("Cannot find account %s", accountstr);
		return FALSE;
	}
	
	enable = mc_account_is_enabled(account);
	if(connectenable != enable)
		mc_account_set_enabled(account, connectenable);

	g_object_unref (account);
	return TRUE;
}

static void
new_channel_cb(EmpathyChandler *chandler,
	TpConn          *tp_conn,
	TpChan          *tp_chan,
	MissionControl  *mc)
{
	McAccount  *account;
	EmpathyChat *chat;
	gchar      *id;

	account = mission_control_get_account_for_connection (mc, tp_conn, NULL);
	id = empathy_inspect_channel (account, tp_chan);
	chat = empathy_chat_window_find_chat (account, id);
	g_free (id);

	if (chat) {
		/* The chat already exists */
		if (!empathy_chat_is_connected (chat)) {
			EmpathyTpChat *tp_chat;

			/* The chat died, give him the new text channel */
			if (empathy_chat_is_group_chat (chat)) {
				/** FIXME:start chat room at here */
				/*tp_chat = EMPATHY_TP_CHAT (empathy_tp_chatroom_new (account, tp_chan));*/
			} else {
				tp_chat = empathy_tp_chat_new (account, tp_chan);
			}
			empathy_chat_set_tp_chat (chat, tp_chat);
			g_object_unref (tp_chat);
		}
		empathy_chat_present (chat);

		g_object_unref (account);
		return;
	}

	if (tp_chan->handle_type == TP_HANDLE_TYPE_CONTACT) {
		/* We have a new private chat channel */
		chat = EMPATHY_CHAT (empathy_private_chat_new (account, tp_chan));
	}
	else if (tp_chan->handle_type == TP_HANDLE_TYPE_ROOM) {
		/* We have a new group chat channel */
		/* chat = EMPATHY_CHAT (empathy_group_chat_new (account, tp_chan));*/
	}

	empathy_chat_present (EMPATHY_CHAT (chat));

	g_object_unref (chat);
	g_object_unref (account);
}
/*FIXME: start salut account at here*/
//static void
//create_salut_account (void)
//{
//	McProfile  *profile;
//	McProtocol *protocol;
//	gboolean    salut_created;
//	McAccount  *account;
//	GList      *accounts;
//	EBook      *book;
//	EContact   *contact;
//	gchar      *nickname = NULL;
//	gchar      *published_name = NULL;
//	gchar      *first_name = NULL;
//	gchar      *last_name = NULL;
//	gchar      *email = NULL;
//	gchar      *jid = NULL;

//	/* Check if we already created a salut account */
//	if (!empathy_conf_get_bool (empathy_conf_get(),
//		EMPATHY_PREFS_SALUT_ACCOUNT_CREATED,
//		&salut_created)) {
//			return;
//		}
//		if (salut_created) {
//			return;
//	}

//	empathy_debug (DEBUG_DOMAIN, "Try to add a salut account...");

//	/* Check if the salut CM is installed */
//	profile = mc_profile_lookup ("salut");
//	protocol = mc_profile_get_protocol (profile);
//	if (!protocol) {
//		empathy_debug (DEBUG_DOMAIN, "Salut not installed");
//		g_object_unref (profile);
//		return;
//	}
//	g_object_unref (protocol);

//	/* Get self EContact from EDS */
//	if (!e_book_get_self (&contact, &book, NULL)) {
//		empathy_debug (DEBUG_DOMAIN, "Failed to get self econtact");
//		g_object_unref (profile);
//		return;
//	}

//	/* Check if there is already a salut account */
//	accounts = mc_accounts_list_by_profile (profile);
//	if (accounts) {
//		empathy_debug (DEBUG_DOMAIN, "There is already a salut account");
//		mc_accounts_list_free (accounts);
//		g_object_unref (profile);
//		return;
//	}

//	account = mc_account_create (profile);
//	mc_account_set_display_name (account, _("People nearby"));

//	nickname = e_contact_get (contact, E_CONTACT_NICKNAME);
//	published_name = e_contact_get (contact, E_CONTACT_FULL_NAME);
//	first_name = e_contact_get (contact, E_CONTACT_GIVEN_NAME);
//	last_name = e_contact_get (contact, E_CONTACT_FAMILY_NAME);
//	email = e_contact_get (contact, E_CONTACT_EMAIL_1);
//	jid = e_contact_get (contact, E_CONTACT_IM_JABBER_HOME_1);
//
//	if (G_STR_EMPTY (nickname) || !empathy_strdiff (nickname, "nickname")) {
//		g_free (nickname);
//		nickname = g_strdup (g_get_user_name ());
//	}
//	if (G_STR_EMPTY (published_name)) {
//		g_free (published_name);
//		published_name = g_strdup (g_get_real_name ());
//	}

//	empathy_debug (DEBUG_DOMAIN, "Salut account created:\n"
//		"  nickname=%s\n"
//		"  published-name=%s\n"
//		"  first-name=%s\n"
//		"  last-name=%s\n"
//		"  email=%s\n"
//		"  jid=%s\n",
//		nickname, published_name, first_name, last_name, email, jid);
//
//	mc_account_set_param_string (account, "nickname", nickname ? nickname : "");
//	mc_account_set_param_string (account, "published-name", published_name ? published_name : "");
//	mc_account_set_param_string (account, "first-name", first_name ? first_name : "");
//	mc_account_set_param_string (account, "last-name", last_name ? last_name : "");
//	mc_account_set_param_string (account, "email", email ? email : "");
//	mc_account_set_param_string (account, "jid", jid ? jid : "");

//	g_free (nickname);
//	g_free (published_name);
//	g_free (first_name);
//	g_free (last_name);
//	g_free (email);
//	g_free (jid);

//	g_object_unref (account);
//	g_object_unref (profile);
//	g_object_unref (contact);
//	g_object_unref (book);
//}

