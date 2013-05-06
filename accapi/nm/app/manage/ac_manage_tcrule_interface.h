#ifndef _AC_MANAGE_TCRULE_INTERFACE_H_
#define _AC_MANAGE_TCRULE_INTERFACE_H_


int ac_manage_service_flow_control(DBusConnection *connection, unsigned int status);

int ac_manage_add_tcrule(DBusConnection *connection, TCRule *tcRuleNew);

int ac_manage_offset_tcrule(DBusConnection *connection, struct tcrule_offset_s *offset);

int ac_manage_delete_tcrule(DBusConnection *connection, unsigned int index);

int ac_manage_show_flow_control_service(DBusConnection *connection, unsigned int *status);

int ac_manage_show_tcrule(DBusConnection *connection, TCRule **rule_array, unsigned int *count);

int ac_manage_show_tcrule_offset(DBusConnection *connection, struct tcrule_offset_s **offset_array, unsigned int *count);



#endif
