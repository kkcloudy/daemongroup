#ifndef _AC_MANAGE_ACINFO_H_
#define _AC_MANAGE_ACINFO_H_
#include "ws_acinfo.h"

int ac_manage_set_acinfo_rule(DBusConnection *connection,char *status,char *key,int opt_type) ;
int ac_manage_set_bkacinfo_rule(DBusConnection *connection,char *status,struct bkacinfo_st *rule,int opt_type);
int ac_manage_delete_system_version_file(DBusConnection *connection,char *version_file);

#endif
