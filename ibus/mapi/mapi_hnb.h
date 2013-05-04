#ifndef _MAPI_HNB_H
#define _MAPI_HNB_H

int mapi_iuh_auto_hnb_login(int index, int localid, char *name, unsigned char policy, DBusConnection *dbus_connection, char *DBUS_METHOD);
int mapi_set_iuh_daemonlog(int index, int localid, unsigned int daemonlogtype, unsigned int daemonloglevel, DBusConnection *dbus_connection, char *DBUS_METHOD);
int mapi_delete_hnb_id(int index, int localid, unsigned int hnbid, DBusConnection *dbus_connection, char *DBUS_METHOD);
Iuh_HNB * mapi_get_hnb_info_by_hnbid(int index, int localid, unsigned int hnb_id, int *ret, DBusConnection *dbus_connection, char *DBUS_METHOD);
HNBLIST * mapi_get_hnb_list(int index, int localid, DBusConnection *dbus_connection, char *DBUS_METHOD);
int mapi_delete_ue_id(int index, int localid, unsigned int ueid, DBusConnection *dbus_connection, char *DBUS_METHOD);
Iuh_HNB_UE * mapi_get_ue_info_by_uebid(int index, int localid, unsigned int ue_id, int *ret, DBusConnection *dbus_connection, char *DBUS_METHOD);
UELIST * mapi_get_ue_list(int index, int localid, DBusConnection *dbus_connection, char *DBUS_METHOD);
int mapi_set_rncid(int index, int localid, unsigned short int rncid, DBusConnection *dbus_connection, char *DBUS_METHOD);
int mapi_set_asn_debug_switch(int index, int localid, unsigned int debug_switch, DBusConnection *dbus_connection, char *DBUS_METHOD);
//int mapi_get_running_cfg_lib(char *showStr,int index, DBusConnection *dbus_connection, char *DBUS_METHOD);
int mapi_femto_acl_white_list(int index, int localid, unsigned int op_type, unsigned int hnb_id, unsigned char* imsi, DBusConnection *dbus_connection, char *DBUS_METHOD);

#endif
