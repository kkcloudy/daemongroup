#ifndef _HAD_DBUS_H
#define _HAD_DBUS_H

#include "had_vrrpd.h"

#define DEFAULT_HANSI_PROFILE 0
#define MAX_HANSI_PROFILE     (16 + 1)		/* for support V1.2, add instance 0,
											 * so VRRP instance count is 17 now.*/
#define VRRP_MAX_VRID		255
#define MAX_IFNAME_LEN        20
#define MAX_IPADDR_LEN        20
#define VRRP_SAVE_CFG_MEM     (16*248)

#define VRRP_OBJPATH_LEN		(64)		/* vrrp obj path length of special vrrp instance */
#define VRRP_DBUSNAME_LEN		(64)		/* vrrp dbus name length of special vrrp instance */


#define NEED_CONVERT_IF_NUM 2       /* num of NEED_CONVERT_IFNAMES ifname need to convert , eg. wlan1,ebr1,...*/
#define NEED_CONVERT_IFNAMES {"wlan","ebr"}  /* ifnames need to convert */



/*RETURN VALUE*/
#define COMMON_SUCCESS						  0
#define VRRP_RETURN_CODE_BASE                 (0x150000)
#define VRRP_RETURN_CODE_SUCCESS			  (COMMON_SUCCESS)
#define VRRP_RETURN_CODE_OK		              (VRRP_RETURN_CODE_BASE + 1)
#define VRRP_RETURN_CODE_ERR		          (VRRP_RETURN_CODE_BASE + 2)
#define VRRP_RETURN_CODE_PROFILE_OUT_OF_RANGE (VRRP_RETURN_CODE_BASE + 3)
#define VRRP_RETURN_CODE_PROFILE_EXIST        (VRRP_RETURN_CODE_BASE + 4)
#define VRRP_RETURN_CODE_PROFILE_NOTEXIST     (VRRP_RETURN_CODE_BASE + 5)
#define VRRP_RETURN_CODE_MALLOC_FAILED        (VRRP_RETURN_CODE_BASE + 6)
#define VRRP_RETURN_CODE_BAD_PARAM            (VRRP_RETURN_CODE_BASE + 7)  
#define VRRP_RETURN_CODE_PROFILE_NOT_PREPARE  (VRRP_RETURN_CODE_BASE + 8)  
#define VRRP_RETURN_CODE_SERVICE_NOT_PREPARE  (VRRP_RETURN_CODE_BASE + 9)
#define VRRP_RETURN_CODE_VGATEWAY_NO_SETTED   (VRRP_RETURN_CODE_BASE + 0xA)
#define VRRP_RETURN_CODE_LINKMODE_NO_SETTED   (VRRP_RETURN_CODE_BASE + 0xB)
#define VRRP_RETURN_CODE_IFNAME_ERROR         (VRRP_RETURN_CODE_BASE + 0xC)
#define VRRP_RETURN_CODE_VIP_EXIST            (VRRP_RETURN_CODE_BASE + 0xD)
#define VRRP_RETURN_CODE_VIP_NOT_EXIST        (VRRP_RETURN_CODE_BASE + 0xE)
#define VRRP_RETURN_CODE_VIP_LAST_ONE         (VRRP_RETURN_CODE_BASE + 0xF)
#define VRRP_RETURN_CODE_IF_EXIST             (VRRP_RETURN_CODE_BASE + 0x10)
#define VRRP_RETURN_CODE_IF_NOT_EXIST         (VRRP_RETURN_CODE_BASE + 0x11)
#define VRRP_RETURN_CODE_IF_UP_LIMIT          (VRRP_RETURN_CODE_BASE + 0x12)
#define VRRP_RETURN_CODE_VMAC_NOT_PREPARE     (VRRP_RETURN_CODE_BASE + 0x13)
#define VRRP_RETURN_CODE_NO_CONFIG			  (VRRP_RETURN_CODE_BASE + 0x14)

#define VRRP_DBUS_BUSNAME "aw.vrrpcli"
#define VRRP_NOTIFY_DBUS_BUSNAME	"aw.vrrpnoti"

#define VRRP_DBUS_OBJPATH "/aw/vrrp"
#define VRRP_DBUS_INTERFACE "aw.vrrp"

typedef enum
{
	VRRP_LINK_TYPE_INVALID = 0,
	VRRP_LINK_TYPE_UPLINK,
	VRRP_LINK_TYPE_DOWNLINK,
	VRRP_LINK_TYPE_VGATEWAY,
	VRRP_LINK_TYPE_L2_UPLINK
}VRRP_LINK_TYPE;

#define VRRP_LINK_TYPE_DESCANT(linktype)		\
({												\
	(linktype == VRRP_LINK_TYPE_UPLINK) ? "uplink" :		\
	(linktype == VRRP_LINK_TYPE_DOWNLINK) ? "downlink" :	\
	(linktype == VRRP_LINK_TYPE_VGATEWAY) ? "vgateway" :	\
	(linktype == VRRP_LINK_TYPE_L2_UPLINK) ? "l2_uplink":     \
	"error link type";							\
})

/* add/delete virtual ip */
typedef enum
{
	VRRP_VIP_OPT_TYPE_INVALID = 0,
	VRRP_VIP_OPT_TYPE_ADD,
	VRRP_VIP_OPT_TYPE_DEL
}VRRP_VIP_OPT_TYPE;

#define VRRP_VIP_OPT_TYPE_DESCANT(opttype)		\
({												\
	(opttype == VRRP_VIP_OPT_TYPE_ADD) ? "add" :	\
	(opttype == VRRP_VIP_OPT_TYPE_DEL) ? "delete" :	\
	"error virtual ip operate type";				\
})


#if 0
#define VRRP_DBUS_METHOD_BRG_NO_DBUG_VRRP "vrrp_no_debug"
#define VRRP_DBUS_METHOD_BRG_DBUG_VRRP    "vrrp_debug"
#define VRRP_DBUS_METHOD_START_VRRP   "vrrp_start"
#define VRRP_DBUS_METHOD_SET_VRRPID   "vrrp_vrid"
#define VRRP_DBUS_METHOD_VRRP_REAL_IP   "vrrp_real_ip"
#define VRRP_DBUS_METHOD_VRRP_HEARTBEAT_LINK   "vrrp_heartbeat_link"
#define VRRP_DBUS_METHOD_VRRP_REAL_IP_DOWNLINK   "vrrp_real_ip_downlink"
#define VRRP_DBUS_METHOD_START_VRRP_DOWNLINK   "vrrp_start_downlink"
#define VRRP_DBUS_METHOD_VRRP_SERVICE_ENABLE   "vrrp_service_enable"
#define VRRP_DBUS_METHOD_VRRP_WANT_STATE   "vrrp_want_state"
#define VRRP_DBUS_METHOD_V_GATEWAY   "vrrp_vgateway"
#define VRRP_DBUS_METHOD_NO_V_GATEWAY   "vrrp_no_vgateway"
#define VRRP_DBUS_METHOD_NO_TRANSFER   "vrrp_no_transfer"
#define VRRP_DBUS_METHOD_END_VRRP     "vrrp_end"
#define VRRP_DBUS_METHOD_PROFILE_VALUE "vrrp_profile"
#define VRRP_DBUS_METHOD_GLOBAL_VMAC_ENABLE "vrrp_global_vmac"
#define VRRP_DBUS_METHOD_PREEMPT_VALUE "vrrp_preempt"
#define VRRP_DBUS_METHOD_ADVERT_VALUE "vrrp_advert"
#define VRRP_DBUS_METHOD_VIRTUAL_MAC_VALUE "vrrp_virtual_mac"
#define VRRP_DBUS_METHOD_MS_DOWN_PACKT_COUNT "vrrp_ms_count"
#define VRRP_DBUS_METHOD_MULTI_LINK_DETECT "vrrp_multi_link_detect"
#define VRRP_DBUS_METHOD_CONFIG_HANSI_PROFILE "vrrp_hansi_profile"
#define VRRP_DBUS_METHOD_SHOW                "vrrp_show"
#define VRRP_DBUS_METHOD_SHOW_DETAIL                "vrrp_show_detail"
#define VRRP_DBUS_METHOD_SHOW_RUNNING                "vrrp_show_running"
#define VRRP_DBUS_METHOD_GET_IFNAME          "vrrp_show_ifname"
#define VRRP_DBUS_METHOD_SET_PROTAL          "vrrp_set_protal"
#define VRRP_DBUS_METHOD_SET_TRANSFER_STATE          "vrrp_set_transfer_state"
#define VRRP_DBUS_METHOD_SET_PORTAL_TRANSFER_STATE          "vrrp_set_portal_transfer_state"
#define VRRP_DBUS_METHOD_START_SEND_ARP   "vrrp_send_arp"
#endif

/********************************************************
*	function declare									*
*********************************************************/
int had_dbus_init
(
	void
);

void * had_dbus_thread_main
(
	void *arg
);

/**********************************************************************************
 *  had_profile_create
 *
 *	DESCRIPTION:
 * 		This method create HANSI profile with global HANSI profile id
 *
 *	INPUT:
 *		profileId - global HANSI profile id
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *
 **********************************************************************************/
unsigned int had_profile_create
(
	unsigned int profileId,
	int  vrid
);

/**********************************************************************************
 *  had_profile_destroy
 *
 *	DESCRIPTION:
 * 		This method create mirror profile with global HANSI profile id
 *
 *	INPUT:
 *		profileId - global HANSI profile id
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *
 **********************************************************************************/
unsigned int had_profile_destroy
(
	unsigned int profileId
);

int had_check_global_enable
(
	void
);

int had_check_vrrp_configure
(
    int profile
);

void had_profile_config_save
(
	char* showStr
);

hansi_s* had_get_profile_node
(
	unsigned int profile
);

int had_dbus_init2
(
	void
) ;


/*
 *******************************************************************************
 *had_splice_objpath_string()
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
void had_splice_objpath_string
(
	char *common_objpath,
	unsigned int profile,
	char *splice_objpath
);
/*********************************************
 * DESCRIPTION:
 *   this function drop ifname like wlanx-y,
 *           and convert ifname like wlany,ebry ... 
 *            to wlanx-y,ebrx-y ... , x is had profile num.
 *
 *********************************************/
int had_check_and_convert_ifname(unsigned int profile,char * ifname);

/**************************************************
 * DESCRIPTION:
 *       convert ifname wlanx-y,ebrx-y to wlany,ebry
 *
 **************************************************/
 int had_ifname_convert_back(char * ifname);


/********************************************************
*	extern Functions									*
*********************************************************/

#endif

