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

#endif

