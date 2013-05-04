#ifndef WBMD_DBUS_HANDLE_H
#define WBMD_DBUS_HANDLE_H
int wbmd_wbridge_create(int ID, int IP);
struct wbridge_info * wbridge_get(int ip);
WBMDBool wbmd_id_check(int WBID);
int wbmd_wbridge_snmp_init(int ID, char ** argv, int argvn);
int wbmd_wbridge_delete(int ID);
#endif
