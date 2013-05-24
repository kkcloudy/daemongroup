#ifndef _AC_MANAGE_FIREWALL_INTERFACE_H_
#define _AC_MANAGE_FIREWALL_INTERFACE_H_

int ac_manage_config_firewall_service(DBusConnection *connection, u_long status);

int ac_manage_config_firewall_rule(DBusConnection *connection, fwRule *rule, u_long config_type);

int ac_manage_change_firewall_index(DBusConnection *connection, u_long new_index, u_long rule_type, u_long index);

int ac_manage_del_firewall_rule(DBusConnection *connection, u_long rule_type, u_long index);

#ifndef _VERSION_18SP7_
int ac_manage_config_nat_udp_timeout(DBusConnection *connection, u_long timeout);
#endif

int ac_manage_show_firewall_rule(DBusConnection *connection, 
								u_long *service_status, u_long *timeout,
								fwRule **rule_array, u_long *rule_num);

int
ac_manage_config_strict_access_level(DBusConnection *connection, int level);

int
ac_manage_show_strict_access(DBusConnection *connection, int *level);

#endif
