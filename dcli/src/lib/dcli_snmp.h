#ifndef DCLI_SNMP_H
#define DCLI_SNMP_H

#define WEB_IPMASK_STRING_MAXLEN	(strlen("255.255.255.255"))
#define WEB_IPMASK_STRING_MINLEN	(strlen("0.0.0.0"))


int trap_index_is_legal_input(const char *str);
int trap_name_is_legal_input(const char *str);
int snmp_ipaddr_is_legal_input(const char *str);
int snmp_community_is_legal_input(const char *str);
int snmp_port_is_legal_input(const char *str);
//int hostname_is_legal_input(const char *str);

#endif

