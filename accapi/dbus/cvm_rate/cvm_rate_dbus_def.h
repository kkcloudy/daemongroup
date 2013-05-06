#ifndef __CVM_RATE_DBUS_DEF_H__
#define __CVM_RATE_DBUS_DEF_H__


/*MATCH_TYPE only allowed to be uint or ulonglong*/
#define MATCH_TYPE unsigned long long
#define MATCH_TYPE_LEN sizeof(MATCH_TYPE)
#define STATISTIC_TYPE unsigned long long
#define DBUS_MATCH_TYPE DBUS_TYPE_UINT64
#define DBUS_STATISTIC_TYPE DBUS_TYPE_UINT64
#define DBUS_MATCH_TYPE_AS_STRING DBUS_TYPE_UINT64_AS_STRING
#define DBUS_STATISTIC_TYPE_AS_STRING DBUS_TYPE_UINT64_AS_STRING

#define MAX_MATCH_RULES_NUM 32
#define CVM_RATE_LIMIT_DEFAULT_RULE_NUM 24
#define PACKET_MATCH_BYTE_NUM 16
#define PROTOCOL_NAME_LEN 32

#define CVM_RATE_IPTCP 0
#define CVM_RATE_IPUDP 1
#define CVM_RATE_PORT_ANY 65537

#define CVM_RATE_NO_LIMIT 5000001

#define CVM_RATE_LIMIT_DBUS_BUSNAME "aw.cvm_rate"

#define CVM_RATE_LIMIT_DBUS_OBJPATH "/aw/cvm_rate"
#define CVM_RATE_LIMIT_DBUS_INTERFACE "aw.cvm_rate"
#define CVM_RATE_LIMIT_DBUS_METHOD_LOG_LEVEL_SET "cvm_rate_limit_log_level_set"
#define CVM_RATE_LIMIT_DBUS_METHOD_ENABLE_SET "cvm_rate_limit_enable_set"
#define CVM_RATE_LIMIT_DBUS_METHOD_SHOW_RULES "cvm_rate_limit_show_rules"
#define CVM_RATE_LIMIT_DBUS_METHOD_ADD_RULES  "cvm_rate_limit_add_rules"
#define CVM_RATE_LIMIT_DBUS_METHOD_ADD_RULES_SIMP  "cvm_rate_limit_add_rules_simp"
#define CVM_RATE_LIMIT_DBUS_METHOD_DEL_RULES  "cvm_rate_limit_del_rules"
#define CVM_RATE_LIMIT_DBUS_METHOD_MODIFY_RULES  "cvm_rate_limit_modify_rules"
#define CVM_RATE_LIMIT_DBUS_METHOD_SHOW_RUNNING  "cvm_rate_limit_show_running"
#define CVM_RATE_LIMIT_DBUS_METHOD_SHOW_STATISTIC	"cvm_rate_limit_show_statistic"
#define CVM_RATE_LIMIT_DBUS_METHOD_CLEAR_STATISTIC	"cvm_rate_limit_clear_statistic"
#define CVM_RATE_LIMIT_DBUS_METHOD_DMESG_ENABLE_SET	"cvm_rate_limit_dmesg_enable_set"
#define CVM_RATE_LIMIT_DBUS_METHOD_LOAD_RULES_FROM_FILE  "cvm_rate_limit_load_rules_from_file"
#define CVM_RATE_LIMIT_DBUS_METHOD_LOAD_RULES_SOURCE_SET  "cvm_rate_limit_load_rules_source_set"
#define CVM_RATE_LIMIT_DBUS_METHOD_RESTORE_RULES_TO_FILE  "cvm_rate_limit_restore_rules_to_file"
#define CVM_RATE_LIMIT_DBUS_METHOD_CLEAR_RULES			  "cvm_rate_limit_clear_rules"
#define CVM_RATE_LIMIT_DBUS_METHOD_SERVICE_LOAD			  "cvm_rate_limit_service_load"
#define CVM_RATE_LIMIT_DBUS_METHOD_SERVICE_UNLOAD			  "cvm_rate_limit_service_unload"

#endif
