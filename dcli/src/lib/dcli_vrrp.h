#ifndef __DCLI_VRRP_H__
#define __DCLI_VRRP_H__

#include "command.h"
#if 1
//niehy add for ipv6 address print
#define NIP6QUAD(addr) \
	((__u8 *)&addr)[0], \
	((__u8 *)&addr)[1], \
	((__u8 *)&addr)[2], \
	((__u8 *)&addr)[3], \
	((__u8 *)&addr)[4], \
	((__u8 *)&addr)[5], \
	((__u8 *)&addr)[6], \
	((__u8 *)&addr)[7], \
	((__u8 *)&addr)[8], \
	((__u8 *)&addr)[9], \
	((__u8 *)&addr)[10], \
	((__u8 *)&addr)[11], \
	((__u8 *)&addr)[12], \
	((__u8 *)&addr)[13], \
	((__u8 *)&addr)[14], \
	((__u8 *)&addr)[15]

#define NIP6QUAD_FMT "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x"

/***********************niehy add Error  message *****************************/
#define CMD_PARAMETER_ERROR          "%% Failed:command parameter format error\n"

#endif

#define HANSI_STR "High-Available Network Access Service Instance Configuration\n"

typedef enum continous_discrete_type{
	HAD_GRATUITOUS_ARP_CONTINOUS,
	HAD_GRATUITOUS_ARP_DISCRETE
}had_arp_send_mode;

//#define MAX_IFNAME_LEN        20
#define MAX_IPADDR_LEN        20

// niehy add for hansi linkage commands
#define SHOW_RUNNING_CONFIG_LEN (1024 * 1024)
#define SEM_SLOT_COUNT_PATH       "/dbm/product/slotcount"
//niehy add end

#define DCLI_VRRP_RETURN_CODE_BASE                 (0x150000)
#define DCLI_VRRP_RETURN_CODE_OK		              (DCLI_VRRP_RETURN_CODE_BASE + 1)
#define DCLI_VRRP_RETURN_CODE_ERR		          (DCLI_VRRP_RETURN_CODE_BASE + 2)
#define DCLI_VRRP_RETURN_CODE_PROFILE_OUT_OF_RANGE (DCLI_VRRP_RETURN_CODE_BASE + 3)
#define DCLI_VRRP_RETURN_CODE_PROFILE_EXIST        (DCLI_VRRP_RETURN_CODE_BASE + 4)
#define DCLI_VRRP_RETURN_CODE_PROFILE_NOTEXIST        (DCLI_VRRP_RETURN_CODE_BASE + 5)
#define DCLI_VRRP_RETURN_CODE_MALLOC_FAILED        (DCLI_VRRP_RETURN_CODE_BASE + 6)
#define DCLI_VRRP_RETURN_CODE_BAD_PARAM            (DCLI_VRRP_RETURN_CODE_BASE + 7)
#define DCLI_VRRP_RETURN_CODE_PROFILE_NOT_PREPARE  (DCLI_VRRP_RETURN_CODE_BASE + 8)  
#define DCLI_VRRP_RETURN_CODE_SERVICE_NOT_PREPARE  (DCLI_VRRP_RETURN_CODE_BASE + 9)  
#define DCLI_VRRP_RETURN_CODE_IF_NOT_EXIST  		(DCLI_VRRP_RETURN_CODE_BASE + 0x11) 
#define DCLI_VRRP_RETURN_CODE_UP_LIMIT  		(DCLI_VRRP_RETURN_CODE_BASE + 0x12)
#define DCLI_VRRP_RETURN_CODE_VIP_NOT_EXIST				(DCLI_VRRP_RETURN_CODE_BASE + 0xE)
#define DCLI_VRRP_RETURN_CODE_NO_CONFIG				(DCLI_VRRP_RETURN_CODE_BASE + 0x14)
#define DCLI_VRRP_RETURN_CODE_PROCESS_NOTEXIST        (DCLI_VRRP_RETURN_CODE_BASE + 0x15)

#define DCLI_VRRP_RETURN_CODE_IF_EXIST             (VRRP_RETURN_CODE_BASE + 0x10)/*AXSSZFI-1140 */

#define VRRP_DEBUG_FLAG_ALL       0xFF
#define VRRP_DEBUG_FLAG_DBG       0x1
#define VRRP_DEBUG_FLAG_WAR       0x2
#define VRRP_DEBUG_FLAG_ERR       0x4
#define VRRP_DEBUG_FLAG_EVT       0x8
#define VRRP_DEBUG_FLAG_PKT_REV   0x10
#define VRRP_DEBUG_FLAG_PKT_SED   0x20
#define VRRP_DEBUG_FLAG_PKT_ALL   0x30
#define VRRP_DEBUG_FLAG_PROTOCOL  0x40

#define DCLI_VRRP_INSTANCE_CNT		(16)		/* count of VRRP instance allowed to created.
												 * for support V1.2, add instance 0,
												 * so VRRP instance count is 17 now.
												 */
#define DCLI_VRRP_OBJPATH_LEN		(64)		/* vrrp obj path length of special vrrp instance */
#define DCLI_VRRP_DBUSNAME_LEN		(64)		/* vrrp dbus name length of special vrrp instance */
#define DCLI_VRRP_SYS_COMMAND_LEN	(128)		/* vrrp used shell command length */

#define DCLI_VRRP_INSTANCE_CHECK_FAILED	(-1)
#define DCLI_VRRP_INSTANCE_CREATED		(1)		/* VRRP instance have created.		*/
#define DCLI_VRRP_INSTANCE_NO_CREATED	(0)		/* VRRP instance have not created.	*/

#define DCLI_VRRP_NOTIFY_ON				(0)		/* notify wid/portal	*/
#define DCLI_VRRP_NOTIFY_OFF			(1)		/* no notify wid/portal */
#define DCLI_VRRP_ON					(1)
#define DCLI_VRRP_OFF					(0)

#define DCLI_VRRP_SHOW_RUNNING_CFG_LEN	(2048 * 2048)

/* eight threads in the current version of HAD module. */
#define DCLI_VRRP_THREADS_CNT			(8)

#define DCLI_VRRP_VGATEWAY_TF_FLG_OFF	(0)
#define DCLI_VRRP_VGATEWAY_TF_FLG_ON	(1)


typedef enum
{
	DCLI_VRRP_NOTIFY_OBJ_TYPE_WID = 0,			/* WID: wireless-control */
	DCLI_VRRP_NOTIFY_OBJ_TYPE_PORTAL,			/* PORTAL: easy-access-gateway */
	DCLI_VRRP_NOTIFY_OBJ_TYPE_DHCP,				/* DHCP: Dynamic Host Configuration Protocol */
	DCLI_VRRP_NOTIFY_OBJ_TYPE_PPPOE,
	DCLI_VRRP_NOTIFY_OBJ_TYPE_INVALID,
}DCLI_VRRP_NOTIFY_OBJ_TYPE;

typedef enum
{
	DCLI_VRRP_LINK_TYPE_INVALID = 0,
	DCLI_VRRP_LINK_TYPE_UPLINK,
	DCLI_VRRP_LINK_TYPE_DOWNLINK,
	DCLI_VRRP_LINK_TYPE_VGATEWAY,
	DCLI_VRRP_LINK_TYPE_L2_UPLINK
}DCLI_VRRP_LINK_TYPE;

/* add/delete virtual ip */
typedef enum
{
	DCLI_VRRP_VIP_OPT_TYPE_INVALID = 0,
	DCLI_VRRP_VIP_OPT_TYPE_ADD,
	DCLI_VRRP_VIP_OPT_TYPE_DEL
}DCLI_VRRP_VIP_OPT_TYPE;

extern unsigned long dcli_ip2ulong(char *str);

void dcli_vrrp_notify_to_npd
(
    struct vty* vty,
    char* ifname1,
    char* ifname2,
    int   add
);

/*
 *******************************************************************************
 *dcli_vrrp_splice_objpath_string()
 *
 *  DESCRIPTION:
 *		use of common path and vrrp instance profile,
 *		splicing obj-path string of special vrrp instance.
 *  INPUTS:
 * 	 	char *common_objpath,	- vrrp obj common path
 *		unsigned int profile,	- profile of vrrp instance
 *
 *  OUTPUTS:
 * 	 	char *splice_objpath	- special obj-path string 
 *
 *  RETURN VALUE:
 *		void
 *
 *******************************************************************************
 */

void dcli_vrrp_splice_objpath_string
(
	char *common_objpath,
	unsigned int profile,
	char *splice_objpath
);

/*
 *******************************************************************************
 *dcli_vrrp_splice_dbusname()
 *
 *  DESCRIPTION:
 *		use of common dbus name and vrrp instance profile,
 *		splicing dbus name of special vrrp instance.
 *  INPUTS:
 * 	 	char *common_dbusname,	- vrrp common dbus name
 *		unsigned int profile,	- profile of vrrp instance
 *
 *  OUTPUTS:
 * 	 	char *splice_dbusname	- special dbus name 
 *
 *  RETURN VALUE:
 *		void
 *
 *******************************************************************************
 */
void dcli_vrrp_splice_dbusname
(
	char *common_dbusname,
	unsigned int profile,
	char *splice_dbusname
);
/***********************************************************************
 *
 * dcli_vrrp_config_continous_discrete_value()
 * DESCRIPTION:
 *		config gratuitous arp send mode for hansi
 * INPUTS:
 * 		unsigned int profile
 *		unsigned int mode
 *		unsigned int value
 * OUTPUTS:
 *		unsigned int op_ret
 * RETURN:
 *		CMD_WARNING
 *		CMD_SUCCESS
 * NOTE:
 *
 *************************************************************************/
int dcli_vrrp_config_continous_discrete_value
(
	unsigned int profile,
	unsigned int mode,
	unsigned value
);
int dcli_vrrp_hansi_is_running
(
	struct vty* vty,
	unsigned int profileId
);

#endif
