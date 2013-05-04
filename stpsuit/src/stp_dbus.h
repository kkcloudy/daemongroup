#ifndef _STP_DBUS_H
#define _STP_DBUS_H

#define STP_RUNNING_CFG_MEM (4*1024*1024) 
#define STP_RUNNING_PORT_CFG_MEM (4*1024*1024)

#define STP_OK			0
#define STP_ERR		(STP_OK + 1)

#define STP_DBUS_DEBUG(x)	printf x
#define STP_DBUS_ERR(x) 	printf x

#define STP_DBUS_BUSNAME "aw.stp"

#define STP_DBUS_OBJPATH "/aw/stp"
#define STP_DBUS_INTERFACE "aw.stp"

#define RSTP_DBUS_METHOD_GET_PROTOCOL_STATE	"stp_state"
#define STP_DBUS_METHOD_STPM_ENABLE "stp_en"
#define STP_DBUS_METHOD_INIT_MSTP 			"mstp_init"
#define STP_DBUS_METHOD_INIT_MSTP_V1 			"mstp_init_v1"
#define RSTP_DBUS_METHOD_PORT_STP_ENABLE "stp_port_en"
#define MSTP_DBUS_METHOD_CFG_VLAN_ON_MST "mstp_vlan_mst"
#define MSTP_DBUS_METHOD_CONFIG_REG_NAME "br_reg_name"
#define MSTP_DBUS_METHOD_CONFIG_BR_REVISION "br_revision"
#define STP_DBUS_METHOD_BRG_PRIORITY "brg_prio"
#define STP_DBUS_METHOD_BRG_MAXAGE	"brg_maxage"
#define STP_DBUS_METHOD_BRG_HELTIME       	   "brg_heltime"
#define MSTP_DBUS_METHOD_CONFIG_MAXHOPS       "brg_maxhops"
#define STP_DBUS_METHOD_BRG_FORDELAY      	   "brg_fordelay"
#define STP_DBUS_METHOD_BRG_FORVERSION    	   "brg_forversion"
#define STP_DBUS_METHOD_BRG_NOCONFIG      	   "brg_noconfig"
#define STP_DBUS_METHOD_BRG_PORT_PATHCOST  	   "brg_pathcost"
#define STP_DBUS_METHOD_BRG_PORTPRIO      	   "brg_port_prio"
#define STP_DBUS_METHOD_BRG_NONSTP        	   "brg_nonstp"
#define STP_DBUS_METHOD_BRG_P2P           	   "brg_p2p"
#define STP_DBUS_METHOD_BRG_EDGE               "brg_edge"
#define STP_DBUS_METHOD_BRG_MCHECK             "brg_mcheck"
#define STP_DBUS_METHOD_BRG_PORTNOCONFIG       "brg_port_noconfig"
#define STP_DBUS_METHOD_BRG_SHOWSPANTREE       "brg_spann_tree"
#define STP_DBUS_METHOD_BRG_SHOWSPANTREE_ADMIN_STATE      "brg_spann_tree_admin_state"
#define RSTP_DBUS_METHOD_CHECK_AND_SAVE_PORT_CONFIG "stp_port_save"
#define STP_DBUS_METHOD_SHOW_BRIDGE_INFO         "brg_info"
#define STP_DBUS_METHOD_BRG_SHOWSPANTREEPORT   "brg_tree_port"
#define STP_DBUS_METHOD_BRG_DBUG_SPANNTREE     "brg_stp_debug"
#define STP_DBUS_METHOD_BRG_NO_DBUG_SPANNTREE  "brg_stp_nodebug"
#define MSTP_DBUS_METHOD_SHOW_CIST_INFO  "mstp_cist"
#define MSTP_DBUS_METHOD_SHOW_MSTI_INFO   "mstp_one_msti"
#define MSTP_DBUS_METHOD_SHOW_SELF_INFO		"mstp_self_info"
#define MSTP_DBUS_METHOD_GET_PORT_INFO		"mstp_port_info"
#define MSTP_DBUS_METHOD_GET_VID_BY_MSTID	"brg_vid"
#define RSTP_DBUS_METHOD_GET_STP_RUNNING_CFG "stp_running_cfg"
#define MSTP_DBUS_METHOD_SET_STP_DUPLEX_MODE  "mstp_duplex_mode"
#define MSTP_DBUS_METHOD_PORT_ENDIS_CFG_DIGEST_SNP "stp_port_endis_cfg_digest_snp"
#define MSTP_DBUS_METHOD_CONFIG_BRIDGE_DIGEST_CONTENT	"mstp_config_bridge_digest_confent"

#define STP_DBUS_ERR_BASE						STP_OK
#define STP_DBUS_ERR_INVALID_PARAM			(STP_DBUS_ERR_BASE + 1)
#define STP_DBUS_NO_SUCH_INST    (STP_DBUS_ERR_BASE + 2)

typedef struct slot_port_count{
	int slot_count;
	int port_count;
}SLOT_PORT;

typedef enum{
	DISCARDING,
	LEARNING,
	FORWARDING
} PORT_STATE;
typedef enum stp_link_state{
   STP_LINK_STATE_UP_E = 0,
   STP_LINK_STATE_DOWN_E ,
   STP_LINK_STATE_MAX
}STP_LINK_STATE;
int stp_dbus_init(void);
void * stp_dbus_thread_main(void *arg);

#endif

