#ifndef _WS_DBUS_DEF_
#define _WS_DBUS_DEF_

#define MAX_NAME_LEN	32
#define WIRELESS_MAX_NUM    40000

#define LOCAL_SLOTID_FILE   "/dbm/local_board/slot_id"
#define DISTRIBUTED_FILE    "/dbm/product/is_distributed"
#define SLOT_COUNT_FILE     "/dbm/product/slotcount"
#define BOARD_MASK_FILE     "/dbm/product/board_on_mask"


#define SNMPD_DBUS_DBUSNAME     "aw.snmp.mib"
#define SNMPD_DBUS_OBJPATH      "/aw/snmp/mib"
#define SNMPD_DBUS_INTERFACE    "aw.snmp.mib"

#define SNMPD_DBUS_CONF_METHOD_SHOW_DBUS_CONNECTION_LIST    "show_dbus_connection_list"
#define SNMPD_DBUS_CONF_METHOD_CONFIG_SNMP_LOG_DEBUG        "snmp_config_log_debug"

/**********TRAP DBUS********/
#define TRAP_DBUS_BUSNAME      "aw.traphelper"
#define TRAP_DBUS_OBJPATH      "/aw/traphelper"
#define TRAP_DBUS_INTERFACE    "aw.trap"

#define TRAP_DBUS_METHOD_GET_INSTANCE_STATES	"trap_dbus_method_get_instance_states"
/*********END**********/


#define SLOT_MAX_NUM    16
#define INSTANCE_NUM	16
#define VRRP_TYPE_NUM   2

#define BOARD_ENABLE    1
#define BOARD_ABNORMAL  (BOARD_ENABLE + 0x1)
#define BOARD_NULL     (BOARD_ENABLE + 0x2)

#define SNMPD_DBUS_SUCCESS      0
#define SNMPD_DBUS_ERROR        -1
#define SNMPD_CONNECTION_ERROR  3

#define LOCAL_SLOT_NUM      0

#define LOCAL_CONNECTION    0
#define REMOTE_CONNECTION   1

#define SNMPD_NO_VRRP       0
#define SNMPD_VRRP_MASTER   (SNMPD_NO_VRRP + 0x1)
#define SNMPD_VRRP_BACK     (SNMPD_NO_VRRP + 0x2)
  
#define SNMPD_REMOTE_INSTANCE   0
#define SNMPD_LOCAL_INSTANCE    1

#define SNMPD_INSTANCE_ALL          0
#define SNMPD_INSTANCE_MASTER       1
#define SNMPD_INSTANCE_BACK         2       
#define SNMPD_INSTANCE_MASTER_V2    3       /*find first master instance in slot*/
#define SNMPD_INSTANCE_MASTER_V3    4       /*the paraHead is dbus connection*/
#define SNMPD_SLOT_CONNECT          5       /*list all slot connection*/
#define SNMPD_SLOT_MASTER_CONNECT   6       /*list the masetr slot connection*/

#endif
