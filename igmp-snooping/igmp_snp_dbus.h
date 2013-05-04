#ifndef _IGMP_SNP_DBUS_H
#define _IGMP_SNP_DBUS_H


#define ISMLD 1
#define ISIGMP 2

#define IGMP_DBUS_DEBUG(x)	printf x
#define IGMP_DBUS_ERR(x) 	printf x

typedef enum {
	VLAN_LIFETIME = 1,
	GROUP_LIFETIME,
	ROBUST_VARIABLE,
	QUERY_INTERVAL,
	RESP_INTERVAL,
	RXMT_INTERVAL
}IGMP_SNP_TIMER_TYPE;

/*
 *IGMP snooping
 *
 */
#define IGMP_DBUS_BUSNAME	"aw.igmp"
#define IGMP_DBUS_OBJPATH	"/aw/igmp"
#define IGMP_DBUS_INTERFACE	"aw.igmp"

#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEBUG_ON		"igmp_snp_debug"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEBUG_OFF		"no_igmp_snp_debug"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_ENABLE	"igmp_snp_en"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_SET_TIMER	"igmp_snp_config_timer"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCGROUP	"igmp_snp_del_group"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCGROUP_VLAN_ONE	"igmp_snp_del_gvlan_one"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCGROUP_VLAN_ALL	"igmp_snp_del_all_gvlan"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_SHOW_TIMER		"igmp_snp_show_timer"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_SHOW_STATE		"igmp_snp_show_config_state"

/*
 *	#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_ADD_DEL_MCROUTE_PORT	"add_del_mcroute_port"
 */
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_VLAN_EN_DIS	"igmp_snp_vlan_add_delete"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_ETH_PORT_EN_DIS	"igmp_snp_port_endis"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_VLAN_COUNT_SHOW		"igmp_snp_show_vlan_cnt"
//#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_GROUP_COUNT_SHOW		"igmp_snp_show_group_cnt"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_TOTAL_GROUP_COUNT_SHOW	"show_igmp_total_group_count"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_TIME_PARAMETER_GET		"igmp_snp_time_parameter_get"
#define IGMP_SNP_DBUS_METHOD_IGMP_SNP_SHOW_ROUTE_PORT			"igmp_snp_show_route_port"


#define IGMP_DBUS_RUNNING_CFG_MEM			(1024*1024)
#define IGMP_DBUS_MAX_CHASSIS_SLOT_COUNT	16
#define IGMP_DBUS_MAX_ETHPORT_PER_BOARD		64

/************************extern Functions*******************************/

int igmp_dbus_init(void);
void * igmp_snp_dbus_thread_main(void *arg);
int igmp_snp_dbus_init(void);

extern int mld_snp_set_enable_dbus();
extern int mld_snp_set_disable_dbus();

#endif

