#ifndef _AC_MANAGE_FIREWALL_H_
#define _AC_MANAGE_FIREWALL_H_


int manage_config_firewall_service(u_long status);

void manage_free_firewall_rule(fwRule **rule);

int manage_add_firewall_rule(fwRule *rule);

int manage_modify_firewall_rule(fwRule *rule);

int manage_chanage_firewall_rule_index(u_long new_index, u_long rule_type, u_long index);

int manage_del_firewall_rule(u_long rule_type, u_long rule_id);

int manage_modify_nat_udp_timeout(unsigned int timeout);

unsigned int manage_show_nat_udp_timeout(void);

int manage_show_firewall_service(void);

int manage_show_firewall_rule(fwRule **rule_array, u_long *rule_num);

int
manage_config_strict_access_level(int level);

int
manage_show_strict_access_level(void);

#endif
