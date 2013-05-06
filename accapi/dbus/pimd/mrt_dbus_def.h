#ifndef __MRT_DBUS_DEF_H__
#define __MRT_DBUS_DEF_H__

/*added by scx 2009.12.17 for pimd dbus*/
#define PIMD_DBUS_BUSNAME "aw.pimd"
#define PIMD_DBUS_OBJPATH "/aw/pimd"
#define PIMD_DBUS_INTERFACE "aw.pimd"

#define PIMD_DBUS_INTERFACE_METHOD_IPMRT_ENABLE         "ip_mrt_enable"
#define PIMD_DBUS_INTERFACE_METHOD_IPPIM_ENABLE         "ip_pim_enable"
#define PIMD_DBUS_INTERFACE_METHOD_IPMRT_STATIC         "ip_mrt_static"
#define PIMD_DBUS_INTERFACE_METHOD_NO_IPMRT_STATIC         "no_ip_mrt_static"
#define PIMD_DBUS_INTERFACE_METHOD_BSR_CANDIDATE         "ip_pim_bsr_candidate"
#define PIMD_DBUS_INTERFACE_METHOD_NO_BSR_CANDIDATE         "no_ip_pim_bsr_candidate"
#define PIMD_DBUS_INTERFACE_METHOD_SHOW_BSR         "ip_pim_show_bsr"

#define PIMD_DBUS_INTERFACE_METHOD_SHOW_RP         "ip_pim_show_rp"
#define PIMD_DBUS_INTERFACE_METHOD_SHOW_RP_CANDIDATE         "ip_pim_show_rp_candidate"
#define PIMD_DBUS_INTERFACE_METHOD_SET_RP_CANDIDATE         "ip_pim_set_rp_candidate"
#define PIMD_DBUS_INTERFACE_METHOD_SET_RP_STATIC            "ip_pim_set_rp_static"


#define PIMD_DBUS_INTERFACE_METHOD_SHOW_IF         "ip_pim_show_interface"
#define PIMD_DBUS_INTERFACE_METHOD_SHOW_IF1         "ip_pim_show_interface1"

#define PIMD_DBUS_INTERFACE_METHOD_SHOW_NBR         "ip_pim_show_neighbor"

#define PIMD_DBUS_INTERFACE_METHOD_SHOW_MRT        "ip_pim_show_mrt"

#define PIMD_DBUS_INTERFACE_METHOD_DEBUG_MRT        "debug_ip_mrt"
#define PIMD_DBUS_INTERFACE_METHOD_DEBUG_WRITE        "ip_pimd_show_running"
#define PIMD_DBUS_INTERFACE_METHOD_BSR_BORDER        "ip_pimd_bsr_border"
#define PIMD_DBUS_INTERFACE_METHOD_DR_PRIORITY        "ip_pimd_dr_priority"
#define PIMD_DBUS_INTERFACE_METHOD_MSG_INTERVAL        "ip_pimd_message_interval"
#define PIMD_DBUS_INTERFACE_METHOD_QUERY_INTERVAL        "ip_pimd_query_interval"


/*define error code*/

#define PIM_STATIC_RP_NO_EXIST 2
#define MRT_DISABLE 3
#define MRT_PIM_DISABLE 4
#define MRT_CONFIG_REPEAT 5
#define PIM_INTERFACE_DISABLE 6
#define MRT_ERR_IPADDR 7
#define MRT_ERR_MIPADDR 8


#endif /*end __MRT_DBUS_DEF_H__*/
