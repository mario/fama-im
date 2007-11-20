#ifndef _HINTSTRING_H
#define _HINTSTRING_H

#define HINTSTR_HISTORYUSAGE \
"Usage: /history [maxnumber|enable|list]\n\
Examp:/history maxnumber n (n : set max number of record history.) \n\
      /history enable [on|off] (enable or disable history record.) \n\
      /history list n[0-all] (n : list n of the histories,default n=10.\n\
          if n=all,list all of the histories.)" 

#define HINTSTR_HISTORYMAXNUM(num) \
"    Max number of histories is %d.", num

#define HINTSTR_HISTORYENABLE(enable) \
"    History record enable is %d.", enable

#define HINTSTR_HISTORYLISTNUM(number) \
"    History recorded number is %d.", number

#define HINTSTR_ACCOUNTREMOVE \
"Usage: /account remove [OPTION] <account name>\n\
OPTION is optional,Default option is U.\n\
      It could be as below:\n\
      U : the account name is unique name of an account.\n\
      D : the account name is display name of an account.\n\
Examp:/account remove U jabber0 (remove an account the uniquename of which is jabber0)\n\
      /account remove D foo@jabber.org (remove an account foo@jabber.org)\n\
      /account remove gtalk1 (remove an account the uniquename of which is gtalk1)"

#define HINTSTR_ACCOUNTADD \
"Usage: /account add <protocol> <loginname> <passwd>\n\
     Please use command \"/account protocols\" to list all of the supported protocols"

#define HINTSTR_CONNECT \
"Usage: /connect [option] <account name>\n\
OPTION is optional,Default option is U.\n\
      Option could be as below:\n\
      U : the account name is unique name of an account.\n\
      D : the account name is display name of an account.\n\
 Examp:/connect U jabber0 (connect an account the uniquename of which is jabber0)\n\
      /connect D foo@jabber.org (connect an account foo@jabber.org)\n\
      /connect gtalk1 (connect an account the uniquename of which is gtalk1)"

#endif

