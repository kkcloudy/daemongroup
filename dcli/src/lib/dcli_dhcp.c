/*******************************************************************************
Copyright (C) Autelan Technology

This software file is owned and distributed by Autelan Technology 
********************************************************************************

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* dcli_dhcp.c
*
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		CLI definition for dhcp module.
*
* DATE:
*		11/26/2009
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.33 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include <zebra.h>
#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <errno.h>
#include "dbus/dhcp/dhcp_dbus_def.h"
#include "dcli_main.h"
#include "vty.h"
#include "command.h"
#include "if.h"
#include "sysdef/returncode.h"
#include "dcli_dhcp.h"
#include "dcli_vrrp.h"
#include <sys/socket.h>
#include <dbus/sem/sem_dbus_def.h>



/*********************hanhui upgrade***************
#define SIOCGIFPRIVFLAGS 0x89b0
***************************************************/
#define SIOCGIFPRIVFLAGS 0x89b1
/**************************end*********************/

#define IFF_RPA 0x20

struct cmd_node pool_node = 
{
	POOL_NODE,
	"%s(config-pool)# ",
	1
};

struct cmd_node dhcrelay_node = 
{
	DHCRELAY_NODE,
	" ",
	1
};
struct cmd_node hansi_pool_node =
{
	HANSI_POOL_NODE,
	"%s(hansi%d-%d pool)# ",
	1
};
struct cmd_node local_hansi_pool_node =
{
	LOCAL_HANSI_POOL_NODE,
	"%s(local hansi%d-%d pool)# ",
	1
};
unsigned int dhcp_relay_enable;
#define ALIAS_NAME_SIZE 		0x15
#define MAC_ADDRESS_LEN			6
#define MAX_IP_STRING_LEN		16
#define MAX_OPTION43_LIST		16
#define MAX_SUB_NET			1024
#define DELETE_OPTION		1
#define ADD_OPTION			0

#define SET_INTERFACE_IP_POOL			(1)		/* interface bind ip pool */
#define CLR_INTERFACE_IP_POOL			(0)		/* interface no ip pool */

#define INTERFACE_BIND_FLAG				(0)		/* interface bind flag */
#define INTERFACE_UNBIND_FLAG			(1)		/* interface unbind flag */

#define DOMAIN_NAME		20


extern char vlan_eth_port_ifname [INTERFACE_NAMSIZ];

unsigned int
dcli_config_ip_pool_name
(
	char *poolName,	
	struct vty *vty
);


extern int 
ip_address_format2ulong
(
	char ** buf,
	unsigned long *ipAddress,
	unsigned int *mask
);

extern unsigned long 
dcli_ip2ulong
(
	char *str
);

typedef struct
{
    unsigned char arEther[MAC_ADDRESS_LEN];
}FC_ETHERADDR;

extern int 
parse_mac_addr
(
	char* input,
	FC_ETHERADDR *  macAddr
);

struct option43_s
{
	unsigned int type;
	unsigned int len;
	unsigned int ipnum;
	unsigned int ip[255];
};

struct dhcp_option_show
{
	char* domainname;
	char* option43;
	unsigned int dns[3];
	char *option138[16];
	char *option60_id[DHCP_OPTION60_DEFAULT_NUMBER];
	unsigned int option60_flag;		/* 1 : enable, 0 : disable */
	unsigned int routers;
	unsigned int wins;
	unsigned int defaulttime;
	unsigned int maxtime;
};

struct dhcp_sub_show 
{
	unsigned int iplow;
	unsigned int iphigh;
	unsigned int mask;
};

struct dhcp_pool_show
{
	char *poolname;
	char *interfacename;
	struct dhcp_sub_show* sub_show;
	unsigned int sub_count;
	struct dhcp_option_show option_show;
};

struct dhcp_global_show
{
	unsigned int enable;
	unsigned int unicast;
	unsigned int staticarp;
	struct dhcp_option_show option_show;
};

struct dhcp_failover_show
{
	char name[ALIAS_NAME_SIZE];
	unsigned int primary;/*bool 0 for primary 1 for secondary*/
	unsigned int split;
	unsigned int mclt;
	unsigned int dstport;
	unsigned int dstip;
	unsigned int srcport;
	unsigned int srcip;
};

struct dhcp_static_show
{
	unsigned int ipaddr;
	unsigned char mac[MAC_ADDRESS_LEN];
	char *ifname;
};

struct dhcp_relay_show
{
	char downifname[21];
	char upifname[21];
	unsigned int ipaddr;
};
char*
ip_long2string(unsigned long ipAddress, unsigned char *buff)
{
	unsigned long	cnt = 0;
	unsigned char *tmpPtr = buff;
	
	cnt = sprintf((char*)tmpPtr,"%ld.%ld.%ld.%ld\n",(ipAddress>>24) & 0xFF, \
			(ipAddress>>16) & 0xFF,(ipAddress>>8) & 0xFF,ipAddress & 0xFF);
	
	return tmpPtr;
}

int get_slotid_index(struct vty *vty, int *index, int *slotid, int *localid)
{
	if(vty->node == CONFIG_NODE){
		*index = 0;

	}else if((HANSI_NODE == vty->node) || (HANSI_POOL_NODE == vty->node) || (HANSI_POOLV6_NODE == vty->node)){
		*index = (int)vty->index;
		if (localid) {
			*localid = vty->local;
		}
		if (slotid) {
			*slotid = vty->slotindex;
			#ifdef DCLI_DHCP_DEBUG
			vty_out(vty, "hansi node: get_slotid_index: slot id %d\n", *slotid);
			#endif
		}		
	}
	else if((LOCAL_HANSI_NODE == vty->node) || (LOCAL_HANSI_POOL_NODE == vty->node) || (LOCAL_HANSI_POOLV6_NODE == vty->node)) {
		*index = (int)vty->index;
		if (localid) {
			*localid = vty->local;
		}
		if (slotid) {
			*slotid = vty->slotindex;
			#ifdef DCLI_DHCP_DEBUG			
			vty_out(vty, "local hansi node: get_slotid_index: slot id %d\n", *slotid);	
			#endif			
		}		
	}
	/* zhangshu modify for interface node slotindex */
	else if(INTERFACE_NODE == vty->node) {
	    *index = (int)vty->index;
		if (localid) {
			*localid = vty->local;
		}
		if(slotid) {
		    if((vty->index_sub) && (vty->slotindex != 0)) {
		        *slotid = vty->slotindex;
		        #ifdef DCLI_DHCP_DEBUG
    			vty_out(vty, "hansi interface node: get_slotid_index: slot id %d\n", *slotid);
    			#endif
		    }
		    else {
		        *slotid = HostSlotId;
    		    #ifdef DCLI_DHCP_DEBUG
    			vty_out(vty, "interface node: get_slotid_index: slot id %d\n", *slotid);
    			#endif
			}
		}
	}
	return 0;
}

#if 0
/**********************************************************************************
 *  dcli_wtp_trace_script_exec
 *
 *	DESCRIPTION:
 * 		This function calls wtp trace scripts reside on file system
 *
 *	INPUT:
 *		ipAddr - ip address in ascii format which probably along with L4 port #
 *		id - identifier in ascii format which is along with ip address
 *		flag - flag indicate this is the first/middle/last call, which has the follow value:
 *			'F' or 'f' - this is the first call during the call sequence
 *			'M' or 'm' - this is the middle call during the call sequence
 *			'L' or 'l' - this is the last call during the call sequence
 *			'A' or 'a' - this option means only one item call with both header and tail infomation
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *		
 **********************************************************************************/
void ReInitDbusConnection(DBusConnection **dcli_dbus_connection,int slot_id,int distributFag){
	//DBusConnection *dcli_dbus_connection = NULL;
	if((distributFag)&&(dbus_connection_dcli[slot_id])&&(NULL != dbus_connection_dcli[slot_id]->dcli_dbus_connection))
	{
		*dcli_dbus_connection = dbus_connection_dcli[slot_id]->dcli_dbus_connection;
	}else{
		*dcli_dbus_connection = dcli_dbus_connection_local;
		//vty_out(vty,"the slot has not connected\n");
	}
}

#endif


inline int check_mask(unsigned int mask)
{
	int i = 0;
	
	if (mask < (1<<31)) {
		return -1;
	}

	for (i=30; i>=0; i--) {
		if ((mask & (1<<i)) && !(mask & 1<<(i+1))) {
			return -1;
		}
	}
	return 0;
}

int dcli_dhcp_check_mask(char *addr, void *out) 
{	
	unsigned int mask = 0;

	if(1 == inet_pton(AF_INET, addr, &mask)) {
		if (check_mask(mask)) {
			return -1;
		}
		if (out) {
			*(unsigned int *)out = mask;
		}
		return 0;
	}
	return -1;
}


int dcli_dhcp_check_ip_address(char *addr, void *out) 
{	
	unsigned int ipaddr = 0;

	if(1 == inet_pton(AF_INET, addr, &ipaddr)) {
		if ((0xffffffff == ipaddr)
			|| (0 == (0xff000000 & ipaddr))) {
			return -1;
		}
		if (out) {
			*(unsigned int *)out = ipaddr;
		}
		return 0;
	}
	return -1;
}
const char *dcli_dhcp_failover_state_print (enum failover_state state)
{
	switch (state) {
	      default:
	      case unknown_state:
		return "unknown-state";

	      case partner_down:
		return "partner-down";

	      case normal:
		return "normal";

	      case conflict_done:
		return "conflict-done";

	      case communications_interrupted:
		return "communications-interrupted";

	      case resolution_interrupted:
		return "resolution-interrupted";

	      case potential_conflict:
		return "potential-conflict";

	      case recover:
		return "recover";

	      case recover_done:
		return "recover-done";

	      case recover_wait:
		return "recover-wait";

	      case shut_down:
		return "shutdown";

	      case paused:
		return "paused";

	      case startup:
		return "startup";
	}
}
int dcli_dhcp_convert_ifname(struct vty *vty, char *ifname)
{
	int slotid = 0, vrrpid = 0, ebrid = 0, wlanid = 0;

	if (!vty || !ifname) {
		return -1;
	}

	if (!distributFag) {
		return 0;
	}
	
	if ((strncmp(ifname, "ebr", 3))  && strncmp(ifname, "wlan", 4)) {
		return 0;
	}

	if (CONFIG_NODE == vty->node) {
		if ((slotid = dcli_dhcp_get_board_slot_id()) < 0) {
			return -1;
		}
		
		if (!strncmp(ifname, "ebr", 3)) {
			sscanf(ifname, "ebr%d", &ebrid);
			memset(ifname, 0, strlen(ifname));
			sprintf(ifname, "ebrl%d-%d-%d", slotid, 0, ebrid);
		} else if (!strncmp(ifname, "wlan", strlen("wlan"))) {
			sscanf(ifname, "wlan%d", &wlanid);
			memset(ifname, 0, strlen(ifname));
			sprintf(ifname, "wlanl%d-%d-%d", slotid, 0, wlanid);
		}

	} else if ((HANSI_NODE == vty->node) || (LOCAL_HANSI_NODE == vty->node)) {
		if (vty->local) {
			if (!(strncmp(ifname, "ebr", 3))) {
				sscanf(ifname, "ebr%d", &ebrid);
				memset(ifname, 0, strlen(ifname));
				sprintf(ifname, "ebrl%d-%d-%d", vty->slotindex, (unsigned int)vty->index, ebrid);
			} else if (!strncmp(ifname, "wlan", strlen("wlan"))) {
				sscanf(ifname, "wlan%d", &wlanid);
				memset(ifname, 0, strlen(ifname));
				sprintf(ifname, "wlanl%d-%d-%d", vty->slotindex, (unsigned int)vty->index, wlanid);
			}
		} else {
			if (!strncmp(ifname, "ebr", 3)) {
				sscanf(ifname, "ebr%d", &ebrid);
				memset(ifname, 0, strlen(ifname));
				sprintf(ifname, "ebr%d-%d-%d", vty->slotindex, (unsigned int)vty->index, ebrid);
			} else if (!strncmp(ifname, "wlan", strlen("wlan"))) {
				sscanf(ifname, "wlan%d", &wlanid);
				memset(ifname, 0, strlen(ifname));
				sprintf(ifname, "wlan%d-%d-%d", vty->slotindex, (unsigned int)vty->index, wlanid);
			}
		}
	}
	return 0;
}


/*
		if (dcli_dhcp_check_ip_address((char *)argv[i], NULL)) {
			vty_out(vty, "invalid ip address %s\n", (char *)argv[i]);
			return CMD_WARNING;
		}
*/


int
dcli_dhcp_check_server_interface_iSbusy
(
	struct vty *vty,
	char *upifname,
	char *downifname
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	int detect = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);	
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_CHECK_SERVER_INTERFACE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &upifname,					
							DBUS_TYPE_STRING, &downifname,
							DBUS_TYPE_UINT32, &detect,					 
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID)) {
        if(op_ret){
            /*vty_out(vty,"saved dhcp lease success\n");*/
		}
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	
	return op_ret;
}

int
dcli_dhcp_check_relay_interface_iSbusy
(
	struct vty *vty,
	char *ifname
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	int detect = 0;
	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
	
	query = dbus_message_new_method_call(DHCRELAY_DBUS_BUSNAME, 
									DHCRELAY_DBUS_OBJPATH, 
									DHCRELAY_DBUS_INTERFACE, 
									DHCRELAY_DBUS_METHOD_CHECK_RELAY_INTERFACE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &ifname,
							DBUS_TYPE_UINT32, &detect,					 
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID)) {
        if(op_ret){
            /*vty_out(vty,"saved dhcp lease success\n");*/
		}
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	
	return op_ret;
}

DEFUN(save_dhcp_lease_cmd_func,
	save_dhcp_lease_cmd,
	"save dhcp-lease",
	"Save dhcp-lease\n"
	"Save dhcp-lease information\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	unsigned int profile = 0;
	int detect = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
			
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SAVE_DHCP_LEASE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &profile,	
							 DBUS_TYPE_UINT32, &detect,					 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID)) {
        if(op_ret){
            vty_out(vty,"saved dhcp lease success\n");
		}
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
}

DEFUN(show_dhcp_lease_cmd_func,
	show_dhcp_lease_cmd,
	"show dhcp-lease",
	"Show dhcp-lease\n"
	"Show dhcp-lease information\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned int op_ret = 0,ret = 0,i = 0,j = 0;
	unsigned int count = 0, value = 0;
	unsigned int ipaddr = 0;
	unsigned char mac[MAC_ADDRESS_LEN] = {0, 0, 0, 0, 0,0};

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
		
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SHOW_DHCP_LEASE);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&value,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query, 200000, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&count);
	dbus_message_iter_next(&iter);	
	
	if(CMD_SUCCESS == op_ret) {
		if(count > 0) {
			dbus_message_iter_recurse(&iter,&iter_array);
			for(i = 0; i < count; i++) {
				dbus_message_iter_recurse(&iter_array,&iter_struct);
			
				dbus_message_iter_get_basic(&iter_struct,&ipaddr);
				dbus_message_iter_next(&iter_struct);
				/*release dhcp lease*/
				if(0 == ipaddr){					
					continue;
				}
				/*vty_out(vty,"ipaddr %d\n",ipaddr);*/
				vty_out(vty,"%-3d.%-3d.%-3d.%-3d ",((ipaddr & 0xff000000) >> 24),((ipaddr & 0xff0000) >> 16),	\
					((ipaddr & 0xff00) >> 8),(ipaddr & 0xff));

				for(j = 0; j< MAC_ADDRESS_LEN; j++) {
		 			dbus_message_iter_get_basic(&iter_struct,&mac[j]);
					/*vty_out(vty,"mac[j] %d\n",mac[j]);*/
					dbus_message_iter_next(&iter_struct);
				}
				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x ",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

				vty_out(vty,"\n");
				dbus_message_iter_next(&iter_array);
			}
		}

	}
	dbus_message_unref(reply);

	return CMD_SUCCESS;
}

DEFUN(dhcp_client_lease_time_func,
	dhcp_client_lease_time,	
	"ip dhcp server client-lease-time (enable|disable)",
	SETT_STR
	"ip dhcp server client-lease-time\n"
	"client-lease-time enable\n"
	"client-lease-time disable\n"
)
{
	
	DBusMessage *query=NULL, *reply=NULL;
	DBusError err;
	DBusMessageIter  iter;
	unsigned int client_ls_time_en=0;
	unsigned int ret_ls_time_en=0;
	if(argc != 1)
	{
		vty_out(vty,"parameter number is error\n");
		return CMD_WARNING;
	}
	if( 0 == strncmp(argv[0],"enable",strlen(argv[0])))
	{
		client_ls_time_en=1;
	}
	else if(0 == strncmp(argv[0],"disable",strlen(argv[0])))
	{
		client_ls_time_en=0;
	}
	else
	{
		vty_out(vty,"parameter number is error\n");
		return CMD_WARNING;
	}

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
			
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_CONFIG_DHCP_LEASE_ENABLE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
					 DBUS_TYPE_UINT32,&client_ls_time_en,
					 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);

	
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	
	if (!dbus_message_get_args (reply, &err,
		DBUS_TYPE_UINT32,&ret_ls_time_en,
		DBUS_TYPE_INVALID)){
		vty_out(vty,"can't get the route entry msg.\n");
	}
	dbus_message_unref(reply);
	
	if(client_ls_time_en != ret_ls_time_en)
	{
		vty_out(vty,"failure.\n");
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;	

}


int dcli_show_lease_by_ip(unsigned int ipaddr, unsigned int ipnums, struct vty *vty)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError 		err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned int    ip_Addr = 0, ip_Nums = 0;
	unsigned int	op_ret = 0, ret = 0, i = 0, j = 0;
	unsigned int 	count = 0, value = 0, ip_addr = 0;
	unsigned char 	mac[MAC_ADDRESS_LEN] = {0, 0, 0, 0, 0, 0};

	ip_Addr = ipaddr;
	ip_Nums = ipnums;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SHOW_DHCP_LEASE_BY_IP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &ip_Addr,
							 DBUS_TYPE_UINT32, &ip_Nums,
							 DBUS_TYPE_INVALID);
		   
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&count);
	dbus_message_iter_next(&iter);	
	
	if(CMD_SUCCESS == op_ret) {
		if(count > 0) {
			dbus_message_iter_recurse(&iter,&iter_array);
			for(i = 0; i < count; i++) {
				dbus_message_iter_recurse(&iter_array,&iter_struct);
			

				dbus_message_iter_get_basic(&iter_struct,&ip_addr);
				dbus_message_iter_next(&iter_struct);
				/*vty_out(vty,"ipaddr %d\n",ipaddr);*/
				vty_out(vty,"%-3d.%-3d.%-3d.%-3d ",((ip_addr & 0xff000000) >> 24),((ip_addr & 0xff0000) >> 16),	\
					((ip_addr & 0xff00) >> 8),(ip_addr & 0xff));

				for(j = 0; j< MAC_ADDRESS_LEN; j++) {
					dbus_message_iter_get_basic(&iter_struct,&mac[j]);
					/*vty_out(vty,"mac[j] %d\n",mac[j]);*/
					dbus_message_iter_next(&iter_struct);
				}
				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x ",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

				vty_out(vty,"\n");
				dbus_message_iter_next(&iter_array);
			}
		}

	}
	dbus_message_unref(reply);

	return CMD_SUCCESS;
}

DEFUN(show_dhcp_lease_by_ip_cmd_func,
	show_dhcp_lease_by_ip_cmd,
	"show dhcp-lease by A.B.C.D",
	"Show config information\n"
	"Show dhcp-lease by ip\n"
	"Show dhcp-lease information\n"
)
{
	unsigned int ipAddr = 0, ipMaskLen = 0, ip_Nums = 0;

	ipAddr = dcli_ip2ulong((char*)argv[0]);
	/*no mask, set ipAddrLen = 32*/
	ipMaskLen = 32;
	ip_Nums = 1<<(32 - ipMaskLen);

	dcli_show_lease_by_ip(ipAddr, ip_Nums, vty);
	
	return CMD_SUCCESS;
}

DEFUN(show_dhcp_lease_by_ip_diff_cmd_func,
	show_dhcp_lease_by_ip_diff_cmd,
	"show dhcp-lease ip_range A.B.C.D A.B.C.D",
	"Show config information\n"
	"Show dhcp-lease information\n"
	"Ipve address range\n"
	"The low ip range\n"
	"The high ip range\n"
)
{
	unsigned int ipAddrl = 0, ipAddrb = 0, ip_Nums = 0, ret = 0;
	
	ret = inet_pton (AF_INET, (char*)argv[0], &ipAddrl);
	if (ret == 0) {
		vty_out (vty, "malformed ip address : %s\n", argv[0]);
		return CMD_SUCCESS;
	 }

	ret = inet_pton (AF_INET, (char*)argv[1], &ipAddrb);
	if (ret == 0) {
		vty_out (vty, "malformed ip address : %s\n", argv[1]);
		return CMD_SUCCESS;
	}

	if (ipAddrl > ipAddrb) {
		vty_out(vty, "wrong ip range\n");
		return CMD_WARNING;
	}	

	ip_Nums = ipAddrb - ipAddrl;
	if(ip_Nums > 255){
		vty_out(vty, "Can display up to 255 leases\n");
		return CMD_WARNING;
	}

	dcli_show_lease_by_ip(ipAddrl, ip_Nums, vty);
	
	return CMD_SUCCESS;
}


/*
DEFUN(show_dhcp_lease_by_ip_mask_cmd_func,
	show_dhcp_lease_by_ip_mask_cmd,
	"show dhcp-lease by subnet A.B.C.D/M",
	"Show config information\n"
	"Show dhcp-lease by ip\n"
	"Show dhcp-lease information\n"
)
{

	unsigned int    ipAddr = 0, ipMaskLen = 0, ip_Nums = 0;
	int Val = 0;

	Val = ip_address_format2ulong((char**)&argv[0], &ipAddr, &ipMaskLen);
	if(CMD_WARNING == Val) {
		vty_out(vty, "%% Bad parameter %s\n", argv[0]);
		return CMD_WARNING;
	}
	
	ip_Nums = 1<<(32 - ipMaskLen);
	dcli_show_lease_by_ip(ipAddr, ip_Nums, vty);
	
	return CMD_SUCCESS;
}
*/

DEFUN(show_dhcp_lease_by_ip_mask_cmd_func,
	show_dhcp_lease_by_ip_mask_cmd,
	"show dhcp-lease by subnet A.B.C.D/M",
	"Show config information\n"
	"Show dhcp-lease by ip\n"
	"Show dhcp-lease information\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned int op_ret = 0,ret = 0,i = 0,j = 0;
	unsigned int count = 0, value = 0;
	unsigned int ipaddr = 0;
	unsigned char mac[MAC_ADDRESS_LEN] = {0, 0, 0, 0, 0,0};
	
	unsigned int    subnet = 0, mask = 0, ip_Nums = 0;
	int Val = 0;

	Val = ip_address_format2ulong((char **)&argv[0], &subnet, &mask);
	if(CMD_WARNING == Val) {
		vty_out(vty, "%% Bad parameter %s\n", argv[0]);
		return CMD_WARNING;
	}

	mask = ~((1 << (32 - mask)) - 1); /* exp: 24 -> 0xffffff00 */
	subnet = subnet & mask;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
		
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SHOW_DHCP_LEASE);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&value,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&count);
	dbus_message_iter_next(&iter);	
	
	if(CMD_SUCCESS == op_ret) {
		if(count > 0) {
			dbus_message_iter_recurse(&iter,&iter_array);
			for(i = 0; i < count; i++) {
				dbus_message_iter_recurse(&iter_array,&iter_struct);
			
				dbus_message_iter_get_basic(&iter_struct,&ipaddr);
				dbus_message_iter_next(&iter_struct);
				/*release dhcp lease*/
				if(0 == ipaddr){					
					continue;
				}
				if ((ipaddr & mask) == (subnet & mask)) 
				{
					/*vty_out(vty,"ipaddr %#x\n",ipaddr);*/
					vty_out(vty,"%-3d.%-3d.%-3d.%-3d ",((ipaddr & 0xff000000) >> 24),((ipaddr & 0xff0000) >> 16),	\
						((ipaddr & 0xff00) >> 8),(ipaddr & 0xff));

					for(j = 0; j< MAC_ADDRESS_LEN; j++) {
			 			dbus_message_iter_get_basic(&iter_struct,&mac[j]);
						/*vty_out(vty,"mac[j] %d\n",mac[j]);*/
						dbus_message_iter_next(&iter_struct);
					}
					
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x ",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

					vty_out(vty,"\n");
				}
				dbus_message_iter_next(&iter_array);
			}
		}

	}
	dbus_message_unref(reply);

	return CMD_SUCCESS;
}

DEFUN(show_dhcp_lease_by_mac_cmd_func,
	show_dhcp_lease_by_mac_cmd,
	"show dhcp-lease by MAC",
	"Show config information\n"
	"Show dhcp-lease by mac\n"
	"Show dhcp-lease information\n"
)
{
	DBusMessage 	*query = NULL, *reply = NULL;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	DBusError		err;
	unsigned char	macAddr[MAC_ADDRESS_LEN], mac[MAC_ADDRESS_LEN];
	unsigned int	macType = 0,op_ret = 0, ipaddr = 0, j = 0;
	FC_ETHERADDR	fc_mac;
	
/*	
	if((argc < 2)||(argc > 3))
	{
		vty_out(vty, "bad parameters number!\n");
		return CMD_WARNING;
	}	
*/
	memset(&macAddr, 0, MAC_ADDRESS_LEN);
	memset(&mac, 0, MAC_ADDRESS_LEN);
	memset(&fc_mac, 0, sizeof(FC_ETHERADDR));
	
	op_ret = parse_mac_addr((char *)argv[0], &fc_mac);
	if (DHCP_SERVER_RETURN_CODE_SUCCESS != op_ret) {
    	vty_out(vty," %% Unknow mac addr format.\n");
		return CMD_SUCCESS;
	}
	memcpy(macAddr, fc_mac.arEther, MAC_ADDRESS_LEN);
	/*vty_out(vty," %% %x%x%x%x%x%x\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);*/

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
		
	/*query*/
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SHOW_DHCP_LEASE_BY_MAC);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&macType,
							 DBUS_TYPE_BYTE,  &macAddr[0],
							 DBUS_TYPE_BYTE,  &macAddr[1],
							 DBUS_TYPE_BYTE,  &macAddr[2],
							 DBUS_TYPE_BYTE,  &macAddr[3],
							 DBUS_TYPE_BYTE,  &macAddr[4],
							 DBUS_TYPE_BYTE,  &macAddr[5],						
							 DBUS_TYPE_INVALID);
      
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &op_ret);

	dbus_message_iter_next(&iter);	
	
	if(op_ret) {
		dbus_message_iter_recurse(&iter, &iter_array);
		dbus_message_iter_recurse(&iter_array, &iter_struct);
	

		dbus_message_iter_get_basic(&iter_struct, &ipaddr);
		dbus_message_iter_next(&iter_struct);
		/*vty_out(vty,"ipaddr %d\n",ipaddr);*/
		vty_out(vty,"%-3d.%-3d.%-3d.%-3d ",((ipaddr & 0xff000000) >> 24),((ipaddr & 0xff0000) >> 16),	\
			((ipaddr & 0xff00) >> 8),(ipaddr & 0xff));

		for(j = 0; j< MAC_ADDRESS_LEN; j++) {
 			dbus_message_iter_get_basic(&iter_struct,&mac[j]);
			/*vty_out(vty,"mac[j] %d\n",mac[j]);*/
			dbus_message_iter_next(&iter_struct);
		}
		
		vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x ",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

		vty_out(vty,"\n");
		dbus_message_iter_next(&iter_array);
	}
	else {
		vty_out(vty, "no lease for this mac \n");
	}

	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
}

unsigned int
dcli_show_static_host
(
	struct vty *vty,
	struct dhcp_static_show **static_host,
	unsigned int *host_num
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned int op_ret = 0,ret = 0,i = 0,j = 0;
	unsigned int ipaddr = 0, count = 0, value = 0;
	unsigned char mac[MAC_ADDRESS_LEN] = {0, 0, 0, 0, 0,0};
	struct dhcp_static_show *host = NULL;
	char *ifname = NULL;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SHOW_STATIC_HOST);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&value,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&count);
	dbus_message_iter_next(&iter);	
	
	if (!op_ret) {
		if(count > 0) {	
			host = malloc(sizeof(struct dhcp_static_show) * count);
			memset(host, 0, (sizeof(struct dhcp_static_show)*count));
			
			dbus_message_iter_recurse(&iter,&iter_array);
			for(i = 0; i < count; i++) {
				dbus_message_iter_recurse(&iter_array,&iter_struct);
			
				dbus_message_iter_get_basic(&iter_struct,&ipaddr);
				dbus_message_iter_next(&iter_struct);
				host[i].ipaddr = ipaddr;
				for(j = 0; j< MAC_ADDRESS_LEN; j++) {
		 			dbus_message_iter_get_basic(&iter_struct,&mac[j]);
					host[i].mac[j] = mac[j];
					dbus_message_iter_next(&iter_struct);
				}
				dbus_message_iter_get_basic(&iter_struct,&ifname);
				dbus_message_iter_next(&iter_struct);
				host[i].ifname = ifname;
				
				dbus_message_iter_next(&iter_array);
			}
		}

	}
	
	*host_num = count;
	*static_host = host;
	dbus_message_unref(reply);

	return CMD_SUCCESS;

}

DEFUN(show_ip_dhcp_static_cmd_func,
	show_ip_dhcp_static_cmd,
	"show ip dhcp static",
	"Show config information\n"
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Show ip dhcp static information\n"
)
{
	struct dhcp_static_show *host = NULL;
	unsigned int i = 0, count = 0;

	dcli_show_static_host(vty, &host, &count);
	
	vty_out(vty, "========================================\n");
	vty_out(vty, "          dhcp static host\n");
	for (i = 0; i < count; i++) {
		vty_out(vty,"%-3d.%-3d.%-3d.%-3d ",(((host[i].ipaddr) & 0xff000000) >> 24),(((host[i].ipaddr) & 0xff0000) >> 16),	\
			(((host[i].ipaddr) & 0xff00) >> 8),((host[i].ipaddr) & 0xff));
		
		vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  ",host[i].mac[0], host[i].mac[1], host[i].mac[2], host[i].mac[3], host[i].mac[4], host[i].mac[5]);
		vty_out(vty,"%s\n",host[i].ifname);
	}
	vty_out(vty, "========================================\n");
	return CMD_SUCCESS;
}

unsigned int 
dcli_show_ip_dhcp_server
(
	struct vty *vty,
	struct dhcp_global_show *global_show
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0, value = 0, enable = 0, staticarp = 0;
	unsigned int	dns[3] = {0, 0, 0}, routers = 0, wins = 0, default_time = 0, max_time = 0, unicast = 0;
	char* domainname = NULL, *option43 = NULL;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SHOW_DHCP_GLOBAL_CONF);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &value, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &enable,
					DBUS_TYPE_UINT32, &unicast,			
					DBUS_TYPE_UINT32, &staticarp,
					DBUS_TYPE_STRING, &domainname,
					DBUS_TYPE_STRING, &option43,
					DBUS_TYPE_UINT32, &dns[0],
					DBUS_TYPE_UINT32, &dns[1],
					DBUS_TYPE_UINT32, &dns[2],
					DBUS_TYPE_UINT32, &routers,
					DBUS_TYPE_UINT32, &wins,
					DBUS_TYPE_UINT32, &default_time,
					DBUS_TYPE_UINT32, &max_time,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
			global_show->enable = enable;
			global_show->unicast= unicast;		
			global_show->staticarp= staticarp;
			global_show->option_show.domainname = domainname;
			global_show->option_show.option43 = option43;
			global_show->option_show.dns[0] = dns[0];
			global_show->option_show.dns[1] = dns[1];
			global_show->option_show.dns[2] = dns[2];
			global_show->option_show.routers = routers;
			global_show->option_show.wins = wins;
			global_show->option_show.defaulttime = default_time;
			global_show->option_show.maxtime = max_time;
			
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}

}

DEFUN(show_ip_dhcp_server_cmd_func,
	show_ip_dhcp_server_cmd,
	"show ip dhcp",
	"Show config information\n"
	"Show ip dhcp server entity\n"
	"Specify pool name begins with char, and name length no more than 20 characters\n"
)
{
	unsigned int ret = 0, op_ret = 0;
	struct dhcp_global_show global_show;
	char tmp1[MAX_IP_STRING_LEN*3 + 1], tmp2[MAX_IP_STRING_LEN], tmp3[MAX_IP_STRING_LEN];
	
	memset(tmp1, 0, MAX_IP_STRING_LEN*3 + 1);
	memset(tmp2, 0, MAX_IP_STRING_LEN);
	memset(tmp3, 0, MAX_IP_STRING_LEN);
	memset(&global_show, 0, sizeof(struct dhcp_global_show));
	
	ret = dcli_show_ip_dhcp_server(vty, &global_show);
	
	vty_out(vty, "===============================================\n");
	vty_out(vty, "          dhcp server global configure\n");
	vty_out(vty, "dhcp server is %s\n", (global_show.enable) ? "enable" : "disable");
	vty_out(vty, "dhcp server reply mode is  %s\n", (global_show.unicast) ? "unicast" : "default");
	vty_out(vty, "dhcp server set kernel static arp is %s\n", (global_show.staticarp) ? "enable" : "disable");
	if (global_show.option_show.domainname) {
		vty_out(vty, "dhcp server global domain name is : %s\n", global_show.option_show.domainname);		
	}
	if (global_show.option_show.option43) {
		vty_out(vty, "dhcp server global option43 is: %s\n", global_show.option_show.option43);		
	}
	vty_out(vty, "dhcp server global dns ip is :");	
	vty_out(vty, "%s", (global_show.option_show.dns[0]&global_show.option_show.dns[1]) ? (strcat(strcat(ip_long2string(global_show.option_show.dns[0], tmp1), ip_long2string(global_show.option_show.dns[1], tmp2)), (global_show.option_show.dns[2] ? ip_long2string(global_show.option_show.dns[2], tmp3) : ""))) : ((global_show.option_show.dns[0] ? ip_long2string(global_show.option_show.dns[0], tmp1) : "\n")));
	memset(tmp2, 0, MAX_IP_STRING_LEN);
	vty_out(vty, "dhcp server global routers ip is : %s", global_show.option_show.routers ? ip_long2string(global_show.option_show.routers, tmp2) : "\n");	
	memset(tmp2, 0, MAX_IP_STRING_LEN);
	vty_out(vty, "dhcp server global wins ip is : %s", global_show.option_show.wins ? ip_long2string(global_show.option_show.wins, tmp2) : "\n");
	vty_out(vty, "dhcp server global lease time is : %u seconds\n", global_show.option_show.defaulttime ? global_show.option_show.defaulttime : 86400);
	//vty_out(vty, "dhcp server global lease time is : %u %s\n", global_show.option_show.defaulttime? global_show.option_show.defaulttime : 1, (global_show.option_show.defaulttime > 1) ? "days" : "day");
	//vty_out(vty, "dhcp server global max lease time is : %u %s\n", global_show.option_show.maxtime? global_show.option_show.maxtime: 1, (global_show.option_show.maxtime> 1) ? "days" : "day");
	vty_out(vty,"===============================================\n");

	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

unsigned int
dcli_show_ip_pool
(
	struct vty *vty,
	unsigned int mode,
	unsigned int index
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	struct dhcp_pool_show* pool = NULL; 
	unsigned int i = 0, j = 0,ret = 0, count=0, pool_count = 0;
	char *tmpstr = NULL;


	unsigned int op_ret = 0;	
	char tmp1[MAX_IP_STRING_LEN*3 + 1] = {0}, tmp2[MAX_IP_STRING_LEN] = {0}, tmp3[MAX_IP_STRING_LEN] = {0};
	char len[3] = {0}, optio43_list[16] = {0};
	char* endstr = NULL;
	int ipnum;
	unsigned char ip[9] = {0}, ip_list[16] = {0};
	unsigned int ip43;
	
	memset(tmp1, 0, MAX_IP_STRING_LEN*3 + 1);
	memset(tmp2, 0, MAX_IP_STRING_LEN);
	memset(tmp3, 0, MAX_IP_STRING_LEN);

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SHOW_IP_POOL_CONF);
	dbus_error_init(&err);
	dbus_message_append_args(query, 
						     DBUS_TYPE_UINT32, &mode,
						     DBUS_TYPE_UINT32, &index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	 }
	 dbus_message_iter_init(reply,&iter);
 
	 dbus_message_iter_get_basic(&iter,&ret);
 
	 if (!ret) {
		dbus_message_iter_next(&iter);  
		dbus_message_iter_get_basic(&iter, &pool_count); 
		pool = malloc(sizeof(struct dhcp_pool_show)*pool_count);
		memset(pool, 0, sizeof(struct dhcp_pool_show)*pool_count);
		dbus_message_iter_next(&iter);  
		dbus_message_iter_recurse(&iter,&iter_array);

		for (i = 0; i < pool_count; i++) {
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array;

			dbus_message_iter_recurse(&iter_array,&iter_struct);			  

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option_show.defaulttime));
			dbus_message_iter_next(&iter_struct); 
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option_show.maxtime));
			dbus_message_iter_next(&iter_struct); 
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option_show.dns[0]));
			dbus_message_iter_next(&iter_struct); 
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option_show.dns[1]));
			dbus_message_iter_next(&iter_struct); 
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option_show.dns[2]));
			dbus_message_iter_next(&iter_struct); 

			/*show option138*/
			for(j = 0; j < 16 ; j++){
				dbus_message_iter_get_basic(&iter_struct, &(pool[i].option_show.option138[j]));
				dbus_message_iter_next(&iter_struct); 
			}			
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option_show.routers));
			dbus_message_iter_next(&iter_struct); 
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option_show.wins));
			dbus_message_iter_next(&iter_struct); 
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option_show.domainname));
			dbus_message_iter_next(&iter_struct); 
			/*
			pool[i].option_show.domainname = malloc(strlen(tmpstr) + 1);
			memset(pool[i].option_show.domainname, 0, (strlen(tmpstr) + 1));
			memcpy(pool[i].option_show.domainname, tmpstr, strlen(tmpstr));
			*/
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option_show.option43));
			dbus_message_iter_next(&iter_struct); 	
			/*
			pool[i].option_show.option43 = malloc(strlen(tmpstr) + 1);
			memset(pool[i].option_show.option43, 0, (strlen(tmpstr) + 1));
			memcpy(pool[i].option_show.option43, tmpstr, strlen(tmpstr));
			*/
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option_show.option60_flag));
			dbus_message_iter_next(&iter_struct); 	
			for (j = 0; j < DHCP_OPTION60_DEFAULT_NUMBER; j++) {
				dbus_message_iter_get_basic(&iter_struct, &(pool[i].option_show.option60_id[j]));
				dbus_message_iter_next(&iter_struct); 		
			}
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].poolname));
			dbus_message_iter_next(&iter_struct); 
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].interfacename));
			dbus_message_iter_next(&iter_struct); 
			/*
			pool[i].poolname = malloc(strlen(tmpstr) + 1);
			memset(pool[i].poolname, 0, (strlen(tmpstr) + 1));
			memcpy(pool[i].poolname, tmpstr, strlen(tmpstr));
			*/
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].sub_count));

			count = pool[i].sub_count;
			dbus_message_iter_next(&iter_struct); 		  
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);	

			pool[i].sub_show = malloc(sizeof(struct dhcp_sub_show)*count);
			memset(pool[i].sub_show, 0, sizeof(struct dhcp_sub_show)*count);
			for (j = 0; j < count; j++){

				DBusMessageIter iter_sub_struct;
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);

				dbus_message_iter_get_basic(&iter_sub_struct,&(pool[i].sub_show[j].iplow));
				dbus_message_iter_next(&iter_sub_struct);
				
				dbus_message_iter_get_basic(&iter_sub_struct,&(pool[i].sub_show[j].iphigh));
				dbus_message_iter_next(&iter_sub_struct);
				
				dbus_message_iter_get_basic(&iter_sub_struct,&(pool[i].sub_show[j].mask));
				dbus_message_iter_next(&iter_sub_struct);


				dbus_message_iter_next(&iter_sub_array);
			}			  	  
			dbus_message_iter_next(&iter_array);

		}/*for*/
	}  /*if*/

	for (i = 0; i < pool_count; i++) {		
		vty_out(vty, "===============================================\n");
		vty_out(vty, "        dhcp server ip pool configure\n");
		vty_out(vty, "ip pool name %s\n", pool[i].poolname);
#if 1
	
		if (pool[i].option_show.domainname) {
			vty_out(vty, "dhcp server domain name is : %s\n", pool[i].option_show.domainname); 	
		}
		if (pool[i].option_show.option43) {

			vty_out(vty, "dhcp server option43 is : ");
			memcpy(len, &(pool[i].option_show.option43[2]), 2);
			ipnum = (strtoul(len, &endstr, 16)) / 4;			
			
			for(j = 0; j < ipnum; ++j){
				memset(ip, 0, 9);				
				memcpy(ip, &(pool[i].option_show.option43[4 + j*8]), 8);
				ip43 = strtoul(ip, &endstr, 16);
				
				ip_list[0] = ip43>>24 & 0xff;
				ip_list[1] = ip43>>16 & 0xff;
				ip_list[2] = ip43>>8 & 0xff;
				ip_list[3] = ip43 & 0xff;
				inet_ntop(AF_INET, ip_list, optio43_list, sizeof(optio43_list));
				vty_out(vty, "%s ", optio43_list);
				memset(optio43_list, 0, 16);
			}
			vty_out(vty, "\n");
		}

		vty_out(vty, "dhcp server option138 is : ");
		for(j = 0;  j < 16 ; j++){
			if(pool[i].option_show.option138[j][0] != '\0'){
				vty_out(vty, "%s ", pool[i].option_show.option138[j]);	
			}else{
				break;
			}
		}
		vty_out(vty, "\n"); 
		
		if (pool[i].option_show.option60_flag) {
			vty_out(vty, "dhcp server option60 is : enable\n"); 	
		}
		for(j = 0;  j < DHCP_OPTION60_DEFAULT_NUMBER; j++){
			if (pool[i].option_show.option60_id[j] && pool[i].option_show.option60_id[j][0]) {
				vty_out(vty, "dhcp server option60 id [%d] : %s\n", j, pool[i].option_show.option60_id[j]);
			}
		}
		
		vty_out(vty, "dhcp server dns ip is :");	
		vty_out(vty, "%s", (pool[i].option_show.dns[0] && pool[i].option_show.dns[1]) ? (strcat(strcat(ip_long2string(pool[i].option_show.dns[0], tmp1), ip_long2string(pool[i].option_show.dns[1], tmp2)), (pool[i].option_show.dns[2] ? ip_long2string(pool[i].option_show.dns[2], tmp3) : ""))) : ((pool[i].option_show.dns[0] ? ip_long2string(pool[i].option_show.dns[0], tmp1) : "\n")));
		memset(tmp2, 0, MAX_IP_STRING_LEN);
		vty_out(vty, "dhcp server routers ip is : %s", pool[i].option_show.routers ? ip_long2string(pool[i].option_show.routers, tmp2) : "\n");	
		memset(tmp2, 0, MAX_IP_STRING_LEN);
		vty_out(vty, "dhcp server wins ip is : %s", pool[i].option_show.wins ? ip_long2string(pool[i].option_show.wins, tmp2) : "\n");
		memset(tmp2, 0, MAX_IP_STRING_LEN);
		vty_out(vty, "dhcp server global lease time is : %u seconds\n", pool[i].option_show.defaulttime ? pool[i].option_show.defaulttime : 86400);
//		vty_out(vty, "dhcp server lease time is : %u %s\n", poolshow[i].option_show.defaulttime? poolshow[i].option_show.defaulttime : 1, (poolshow[i].option_show.defaulttime > 1) ? "days" : "day");
//		vty_out(vty, "dhcp server max lease time is : %u %s\n", poolshow[i].option_show.maxtime? poolshow[i].option_show.maxtime: 1, (poolshow[i].option_show.maxtime> 1) ? "days" : "day");
		if(pool[i].interfacename != NULL && pool[i].interfacename[0] != '\0'){
			vty_out(vty, "binded by interface name  : %s\n", pool[i].interfacename);
		}
		for (j = 0; j < pool[i].sub_count; j++) {
			vty_out(vty, "ip range low %s", pool[i].sub_show[j].iplow ? ip_long2string(pool[i].sub_show[j].iplow, tmp2) : "");
			memset(tmp2, 0, MAX_IP_STRING_LEN);
			vty_out(vty, "ip range high %s", pool[i].sub_show[j].iphigh ? ip_long2string(pool[i].sub_show[j].iphigh, tmp2) : "");
			memset(tmp2, 0, MAX_IP_STRING_LEN);
			vty_out(vty, "ip range mask %s", pool[i].sub_show[j].mask ? ip_long2string(pool[i].sub_show[j].mask, tmp2) : "");
		}
		#endif
		vty_out(vty, "===============================================\n");
	}

	
	dbus_message_unref(reply);
	 return CMD_SUCCESS;		 
}

DEFUN(show_ip_pool_cmd_func,
	show_ip_pool_cmd,
	"show ip pool",
	"Show config information\n"
	"Show ip pool entity\n"
	"Specify pool name begins with char, and name length no more than 20 characters\n"
)
{
	unsigned int ret = 0, mode = 0, index = 0;	
	
	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;
	}
	else if((POOL_NODE == vty->node) || (HANSI_POOL_NODE == vty->node) || (LOCAL_HANSI_POOL_NODE  == vty->node)) {
		mode = 1;		
		index = (unsigned int *)(vty->index_sub);
	}
	
	ret = dcli_show_ip_pool(vty, mode, index);
#if 0	
	for (i = 0; i < count; i++) {
		
		vty_out(vty, "===============================================\n");
		vty_out(vty, "        dhcp server ip pool configure\n");
		vty_out(vty, "ip pool name %s\n", poolshow[i].poolname);
		if (poolshow[i].option_show.domainname) {
			vty_out(vty, "dhcp server domain name is : %s\n", poolshow[i].option_show.domainname); 	
		}
		if (poolshow[i].option_show.option43) {

			vty_out(vty, "dhcp server option43 is : ");
			memcpy(len, &(poolshow[i].option_show.option43[2]), 2);
			ipnum = (strtoul(len, &endstr, 16)) / 4;			
			
			for(j = 0; j < ipnum; ++j){
				memset(ip, 0, 9);				
				memcpy(ip, &(poolshow[i].option_show.option43[4 + j*8]), 8);
				ip43 = strtoul(ip, &endstr, 16);
				
				ip_list[0] = ip43>>24 & 0xff;
				ip_list[1] = ip43>>16 & 0xff;
				ip_list[2] = ip43>>8 & 0xff;
				ip_list[3] = ip43 & 0xff;
				inet_ntop(AF_INET, ip_list, optio43_list, sizeof(optio43_list));
				vty_out(vty, "%s ", optio43_list);
				memset(optio43_list, 0, 16);
			}
			vty_out(vty, "\n");
		}

		if (poolshow[i].option_show.option60_flag) {
			vty_out(vty, "dhcp server option60 is : enable\n"); 	
		}
		for(j = 0;  j < DHCP_OPTION60_DEFAULT_NUMBER; j++){
			if (poolshow[i].option_show.option60_id[j] && poolshow[i].option_show.option60_id[j][0]) {
				vty_out(vty, "dhcp server option60 id [%d] : %s\n", j, poolshow[i].option_show.option60_id[j]);
			}
		}
		vty_out(vty, "dhcp server option138 is : ");
		for(j = 0;  j < 16 ; j++){
			if(poolshow[i].option_show.option138[j][0] != '\0'){
				vty_out(vty, "%s ", poolshow[i].option_show.option138[j]);	
			}else{
				break;
			}
		}
		vty_out(vty, "\n");
		vty_out(vty, "dhcp server dns ip is :");	
		vty_out(vty, "%s", (poolshow[i].option_show.dns[0]&poolshow[i].option_show.dns[1]) ? (strcat(strcat(ip_long2string(poolshow[i].option_show.dns[0], tmp1), ip_long2string(poolshow[i].option_show.dns[1], tmp2)), (poolshow[i].option_show.dns[2] ? ip_long2string(poolshow[i].option_show.dns[2], tmp3) : ""))) : ((poolshow[i].option_show.dns[0] ? ip_long2string(poolshow[i].option_show.dns[0], tmp1) : "\n")));
		memset(tmp2, 0, MAX_IP_STRING_LEN);
		vty_out(vty, "dhcp server routers ip is : %s", poolshow[i].option_show.routers ? ip_long2string(poolshow[i].option_show.routers, tmp2) : "\n");	
		memset(tmp2, 0, MAX_IP_STRING_LEN);
		vty_out(vty, "dhcp server wins ip is : %s", poolshow[i].option_show.wins ? ip_long2string(poolshow[i].option_show.wins, tmp2) : "\n");
		memset(tmp2, 0, MAX_IP_STRING_LEN);
		vty_out(vty, "dhcp server global lease time is : %u seconds\n", poolshow[i].option_show.defaulttime ? poolshow[i].option_show.defaulttime : 86400);
//		vty_out(vty, "dhcp server lease time is : %u %s\n", poolshow[i].option_show.defaulttime? poolshow[i].option_show.defaulttime : 1, (poolshow[i].option_show.defaulttime > 1) ? "days" : "day");
//		vty_out(vty, "dhcp server max lease time is : %u %s\n", poolshow[i].option_show.maxtime? poolshow[i].option_show.maxtime: 1, (poolshow[i].option_show.maxtime> 1) ? "days" : "day");
		if(poolshow[i].interfacename != NULL && poolshow[i].interfacename[0] != '\0'){
			vty_out(vty, "binded by interface name  : %s\n", poolshow[i].interfacename);
		}
		for (j = 0; j < poolshow[i].sub_count; j++) {
			vty_out(vty, "ip range low %s", poolshow[i].sub_show[j].iplow ? ip_long2string(poolshow[i].sub_show[j].iplow, tmp2) : "");
			memset(tmp2, 0, MAX_IP_STRING_LEN);
			vty_out(vty, "ip range high %s", poolshow[i].sub_show[j].iphigh ? ip_long2string(poolshow[i].sub_show[j].iphigh, tmp2) : "");
			memset(tmp2, 0, MAX_IP_STRING_LEN);
			vty_out(vty, "ip range mask %s", poolshow[i].sub_show[j].mask ? ip_long2string(poolshow[i].sub_show[j].mask, tmp2) : "");
		}
		vty_out(vty, "===============================================\n");
	}
#endif	
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

unsigned int
dcli_create_ip_pool_name
(
	unsigned int del,
	char *poolName,	
	struct vty *vty
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0, index = 0;

	if (!poolName) {
		return 1;
	}
	int indextmp = 0, localid = 1, slot_id = HostSlotId;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_CREATE_POOL_NODE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,	
							 DBUS_TYPE_UINT32,&del,
							 DBUS_TYPE_STRING,&poolName, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &index,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
			if (del) {	/* 0: add   1: del*/
			}
			else {
				if(CONFIG_NODE == vty->node) {
					vty->node = POOL_NODE;
					vty->index_sub = (void *)index;
					/*vty_out(vty, "create pool index is %d \n", vty->index);*/
				} else if (HANSI_NODE == vty->node){
					vty->node = HANSI_POOL_NODE;
					vty->index_sub = (void *)index;
				} else if (LOCAL_HANSI_NODE == vty->node){
					vty->node = LOCAL_HANSI_POOL_NODE;
					vty->index_sub = (void *)index;					
				} else {
					vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
					//dbus_message_unref(reply);
					//return CMD_WARNING;
				}
			}
		}
		else {
			if (del) {			
				if(DHCP_NOT_FOUND_POOL == op_ret){
					vty_out (vty, "delete ip pool fail because of not found pool\n");
				}else if(DHCP_UNBIND_BY_INTERFACE == op_ret){
					vty_out (vty, "pool has already bind to interface, please first unbind pool\n");
				}else{
					vty_out (vty, "delete ip poolfail\n");
				}				
			}
			else if(2 == op_ret){
				/* pool already exists, config ip pool */
				op_ret = dcli_config_ip_pool_name(poolName, vty);
				if(op_ret)
				    vty_out (vty, "enter ip pool fail \n");
			}
			else {
			    vty_out (vty, "create ip pool fail \n"); 
				dbus_message_unref(reply);
				return 1;
			}
		}
	} 
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		//dbus_message_unref(reply);
	
		//return CMD_WARNING;
	}
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(create_ip_pool_name_cmd_func,
	create_ip_pool_name_cmd,
	"ip pool POOLNAME",
	"Ipv4 address of portal\n"
	"Create ip pool entity\n"
	"Specify pool name begins with char, and name length no more than 20 characters\n"
)
{
	char* poolName = NULL;
	char *ptr = NULL;
	unsigned int nameSize = 0;
	int ret = 0;
	unsigned int op_ret = 0;

	nameSize = strlen(argv[0]);
	if (nameSize >= ALIAS_NAME_SIZE) {
		vty_out(vty, "pool name is too long\n");
		return CMD_WARNING;
	}

	if (NULL == (poolName = (char*)malloc(ALIAS_NAME_SIZE))) {
		vty_out(vty, "create ip pool failed\n");
		return CMD_WARNING;
	}
	memset(poolName, 0, ALIAS_NAME_SIZE);
	
	memcpy(poolName, argv[0], nameSize);
	//poolName[nameSize+1] = '\0';

	ptr = poolName;
	while (*ptr) {

		/* allow ab..z, AB...Z, 01...9, _, -, @	 */
		if ((*ptr >= 'a' && *ptr <= 'z') || (*ptr >= 'A' && *ptr <= 'Z')
			|| (*ptr >= '0' && *ptr <= '9')
			|| (*ptr == '_') || (*ptr == '-') || (*ptr == '.') || (*ptr == '@')) 
		{
			ptr++;
		} else {
			vty_out(vty, "pool name contains invalid character : '%c'\n", *ptr);
			free(poolName);
			poolName = NULL;	
			return CMD_WARNING;
		}
	}
	
	
	ret = dcli_create_ip_pool_name(0, poolName, vty);

	if(poolName) {
	    free(poolName);
	    poolName = NULL;
	}
	if(ret){
		return CMD_FAILURE;
	}
	return CMD_SUCCESS;
	
}

DEFUN(delete_ip_pool_name_cmd_func,
	delete_ip_pool_name_cmd,
	"no ip pool POOLNAME",
	"Delete old Configuration\n"
	"Ipv4 address of portal\n"
	"Create ip pool entity\n"
	"Specify pool name begins with char, and name length no more than 20 characters\n"
)
{
	char* poolName = NULL;
	unsigned int nameSize = 0;
	int ret = 0;
	unsigned int op_ret = 0;

	nameSize = strlen(argv[0]);
	if (nameSize >= ALIAS_NAME_SIZE) {
		vty_out(vty, "pool name is too long\n");
		return CMD_WARNING;
	}

	poolName = (char*)malloc(ALIAS_NAME_SIZE);
	memset(poolName, 0, ALIAS_NAME_SIZE);

	memcpy(poolName, argv[0], nameSize);
	//poolName[nameSize+1] = '\0';
	ret = dcli_create_ip_pool_name(1, poolName, vty);
	if (!ret) {
		free(poolName);
		poolName = NULL;
		return CMD_SUCCESS;
	}
	else {
		vty_out(vty, "delete ip pool fail\n");
		free(poolName);
		poolName = NULL;	
		return CMD_WARNING;
	}
}

unsigned int
dcli_config_ip_pool_name
(
	char *poolName,	
	struct vty *vty
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0, index = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_ENTRY_POOL_NODE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING,&poolName, 
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &index,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
			if(CONFIG_NODE == vty->node) {
				vty->node = POOL_NODE;
				vty->index_sub = (void*)index;
			} else if (HANSI_NODE == vty->node) {
				vty->node = HANSI_POOL_NODE;
				vty->index_sub = (void*)index;
			} else if (LOCAL_HANSI_NODE == vty->node) {
				vty->node = LOCAL_HANSI_POOL_NODE;
				vty->index_sub = (void*)index;	
			} else {
				vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
				return CMD_WARNING;
			}
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}else{
			if(DHCP_NOT_FOUND_POOL == op_ret){
				vty_out (vty, "Don't found the pool\n");
			}
			return CMD_WARNING;
		}
	} 
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}
}
DEFUN(config_ip_pool_name_cmd_func,
	config_ip_pool_name_cmd,
	"config ip pool POOLNAME",
	CONFIG_STR
	"Ipv4 address of portal\n"
	"Config ip pool entity\n"
	"Specify pool name begins with char, and name length no more than 20 characters\n"
)
{

	char* poolName = NULL;
	unsigned int nameSize = 0, nodeSave = 0;
	int ret = 0, index = 0;
	unsigned int op_ret = 0;

	poolName = (char*)malloc(ALIAS_NAME_SIZE);
	memset(poolName, 0, ALIAS_NAME_SIZE);

	nameSize = strlen(argv[0]);
	if (nameSize >= ALIAS_NAME_SIZE) {
		vty_out(vty, "pool name is too long\n");
		free(poolName);
		poolName = NULL;	
		return CMD_WARNING;
	}	
	memcpy(poolName, argv[0], nameSize);
	ret = dcli_config_ip_pool_name(poolName, vty);
	
	if (!ret) {
		free(poolName);		
		poolName = NULL;
		return CMD_SUCCESS;
	}
	else {	
		vty_out(vty, "ip pool %s not exist\n", poolName);		
		free(poolName);
		poolName = NULL;
		return CMD_WARNING;
	}
}

/**********************************************************************************
 *  get_slot_id_by_ifname
 *
 *	DESCRIPTION:
 * 		get board slot id by ifname
 *
 *	INPUT:
 *		ifname -> interface name
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		slot_id -> slot id
 *		
 **********************************************************************************/ 

unsigned int get_slot_id_by_ifname(const char *ifname)
{
	unsigned int slotnum = 0;
	int i = 0;
	int count = 0;
	char tmp[32];

	memset(tmp, 0, sizeof(tmp));
	memcpy(tmp, ifname, strlen(ifname));

	/* eth : cpu */
	if (0 == strncmp(ifname, "eth", 3)) {
		sscanf(ifname, "eth%d-%*d", &slotnum);
	}

	/* ve */
	else if (0 == strncmp(ifname, "ve", 2)) {
		sscanf(ifname, "ve%d.%*d", &slotnum);
	} 

	/* vlan */
	else if (0 == strncmp(ifname, "vlan", 4)) {
		slotnum = 0xffff;	/* invalid slot number */
	} 

	/* radio */
	else if (0 == strncmp(ifname, "r", 1)) {
		for (i = 0; i < strlen(ifname); i++) {
			/*use '-' to make sure this radio is local board or remote board */
			if (tmp[i] == '-') {
				count++;
			}			
		}
		
		if (2 == count) {	/*local board*/
			slotnum = dcli_dhcp_get_board_slot_id();
		} else if(3 == count) {	/*remote board*/
			sscanf(ifname, "r%d-%*d-%*d-%d.%*d", &slotnum);
		}
	}

	#if 1
	/* wlan */
	else if(0 == strncmp(ifname, "wlanl", 5)) {
		sscanf(ifname, "wlanl%d-%*d-%*d", &slotnum);
	}
	else if ((0 == strncmp(ifname, "wlan", 4)) && (strncmp(ifname, "wlanl", 5))) {
		sscanf(ifname, "wlan%d-%*d-%*d", &slotnum);
	}
	#else
	else if (0 == strncmp(ifname, "wlan", 4)) {
		for (i = 0; i < strlen(ifname); i++) {
			if(tmp[i] == '-') {
				count++;
			}
		}
		
		if (1 == count) {	/*local board*/
			slotnum = dcli_dhcp_get_board_slot_id();
		} else if (2 == count) {	/*remote board*/
			sscanf(ifname, "wlan%d-%*d-%*d", &slotnum);
		}
	}
	#endif

	/* ebr */
	#if 1
	/* ebrl */
	else if (0 == strncmp(ifname, "ebrl", 4)) {
		sscanf(ifname, "ebrl%d-%*d-%*d", &slotnum);
	}

	/* ebr */
	else if ((0 == strncmp(ifname, "ebr", 3)) && (strncmp(ifname, "ebrl", 4))) {
		sscanf(ifname, "ebr%d-%*d-%*d", &slotnum);
	}
	else if (0 == strncmp(ifname, "mng", 3)) {
		sscanf(ifname, "mng%d-%*d", &slotnum);
	}
	#else
	else if (0 == strncmp(ifname, "ebr", 3)) {
		for (i = 0; i < strlen(ifname); i++) {
			if (tmp[i] == '-') {
				count++;
			}
		}
		if (1 == count) {	/*local board*/
			slotnum = dcli_dhcp_get_board_slot_id();
		} else if (2 == count) {	/*remote board*/
			sscanf(ifname, "ebr%d-%*d-%*d", &slotnum);
		}
	}
	#endif
	return slotnum;
}


/**********************************************************************************
 *  dcli_dhcp_get_board_slot_id
 *
 *	DESCRIPTION:
 * 		get board slot id
 *
 *	INPUT:
 *		NULL
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		-1 -> failed
 *		slot_id -> slot id
 *		
 **********************************************************************************/ 
int dcli_dhcp_get_board_slot_id(void)
{
	int fd = -1;
	int length = -1;
	int slot_id = -1;
	int ret = -1;
	char buf[8];

	memset(buf, 0, sizeof(buf));

	if ((fd = open(DISTRIBUTED_BOARD_SLOT_ID_FILE, O_RDONLY)) < 0) {
		return ret;
	}

	if ((length = read(fd, buf, sizeof(buf))) < 0) {
		close(fd);
		return ret;
	}

	if (buf[length-1] == '\n') {
		buf[length-1] = '\0';
		length--;
	}

	slot_id = atoi(buf);
	close(fd);
	
	return slot_id;
}


/**********************************************************************************
 *  is_local_board_interface
 *
 *	DESCRIPTION:
 * 		whether local board interface
 *
 *	INPUT:
 *		NULL
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		0 -> remote
 *		1 -> local board interface
 *		
 **********************************************************************************/ 
unsigned int is_local_board_interface(const char *ifname)
{
	int sock = -1;
	int board_slot = -1;
	int slot_id = -1;
	struct ifreq tmp;

	if (NULL == ifname) {
		return -1;
	}

	/* ve */
	if (0 == strncmp(ifname, "ve", 2)) {
		board_slot = dcli_dhcp_get_board_slot_id();
		slot_id = get_slot_id_by_ifname(ifname);

		return (board_slot == slot_id);
	}

	if (1 == sscanf(ifname, "eth%d-%*d.%*d.%*d", &slot_id)) {
		board_slot = dcli_dhcp_get_board_slot_id();
		return (board_slot == slot_id);
	}

	/* only master can create vlan interface, so take interface vlan for local interface. */
	if (0 == strncmp(ifname, "vlan", 4)) {
		return 1;
	}
	
	memset(&tmp, 0, sizeof(tmp));
	strcpy(tmp.ifr_name, ifname);

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		return -1;
	}

	if (ioctl(sock, SIOCGIFPRIVFLAGS , &tmp) < 0) {
		close(sock);
		return -1;
	}
	close(sock);
	/*
	printf("%s\n", (IFF_RPA == tmp.ifr_flags) ? "rpa interface" : " "); */
	
	return ((tmp.ifr_flags & IFF_RPA) != IFF_RPA);
	
#if 0
	int slot_id = -1;
	int board_slot_id = -1;
	
	slot_id = get_slot_id_by_ifname(ifname);
	board_slot_id = dcli_dhcp_get_board_slot_id();

	return (slot_id == board_slot_id);
#endif	
}

/**********************************************************************************
 *  dcli_dhcp_is_distributed
 *
 *	DESCRIPTION:
 * 		whether distributed
 *
 *	INPUT:
 *		NULL
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		0 -> not distributed
 *		1 -> distributed
 *		
 **********************************************************************************/ 
unsigned int dcli_dhcp_is_distributed(void)
{
	int fd = -1;
	int length = -1;
	char buf[8];
	int ret = NOT_DISTRIBUTED;

	memset(buf, 0, sizeof(buf));

	if ((fd = open(DISTRIBUTED_FILE, O_RDONLY)) < 0) {
		return ret;
	}

	if ((length = read(fd, buf, sizeof(buf))) < 0) {
		close(fd);
		return ret;
	}

	if (buf[length-1] == '\n') {
		buf[length-1] = '\0'; 
	}

	/* 1 : distributed , 0 : not */
	if('1' == buf[0]) {
		ret = IS_DISTRIBUTED;
	} else {
		ret = NOT_DISTRIBUTED;
	}
	close(fd);
	return ret;
}


/**********************************************************************************
 *  dcli_dhcp_is_master_board
 *
 *	DESCRIPTION:
 * 		whether active master board
 *
 *	INPUT:
 *		NULL
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		0 -> not master
 *		1 -> master board
 *		
 **********************************************************************************/ 
unsigned int dcli_dhcp_is_master_board(void)
{
	int fd = -1;
	int length = -1;
	char buf[8];
	int ret = -1;

	memset(buf, 0, sizeof(buf));

	if ((fd = open(DISTRIBUTED_MASTER_FILE, O_RDONLY)) < 0) {
		return ret;
	}

	if ((length = read(fd, buf, sizeof(buf))) < 0) {
		close(fd);
		return ret;
	}

	if (buf[length-1] == '\n') {
		buf[length-1] = '\0'; 
	}

	/* 1 : master , 0 : not */
	if('1' == buf[0]) {
		ret = IS_MASTER_BOARD;
	} else {
		ret = NOT_MASTER_BOARD;		/* not master, return */
	}

	close(fd);
	return ret;
}


/**********************************************************************************
 *  dcli_dhcp_is_active_master_board
 *
 *	DESCRIPTION:
 * 		whether active master board
 *
 *	INPUT:
 *		NULL
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		0 -> not active master
 *		1 -> active master board
 *		
 **********************************************************************************/ 
unsigned int dcli_dhcp_is_active_master_board(void)
{
	int fd = -1;
	int length = -1;
	char buf[8];
	int ret = -1;

	memset(buf, 0, sizeof(buf));

	if ((fd = open(DISTRIBUTED_ACTIVE_MASTER_FILE, O_RDONLY)) < 0) {
		return ret;
	}

	if ((length = read(fd, buf, sizeof(buf))) < 0) {
		close(fd);
		return ret;
	}

	if (buf[length-1] == '\n') {
		buf[length-1] = '\0'; 
	}

	/* 1 : master , 0 : not */
	if('1' == buf[0]) {
		ret = 1;
	} else {
		ret = 0;
	}

	close(fd);
	return ret;
}


/**********************************************************************************
 *  dcli_dhcp_distributed_whether_allow_dhcp
 *
 *	DESCRIPTION:
 * 		distributed whether allow config dhcp
 *
 *	INPUT:
 *		NULL
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		-1 -> failed 
 *		0 -> not allow
 *		1 -> allow
 *		
 **********************************************************************************/ 
int dcli_dhcp_distributed_whether_allow_dhcp(void)
{
	int ret = -1;

	if (IS_DISTRIBUTED != dcli_dhcp_is_distributed()) {
		return 0;
	}
	
	ret = dcli_dhcp_is_master_board();
	if (1 == ret) {	/* master board */
		/* is active master board : 1(active), 0(not), -1(failed) */
		ret = dcli_dhcp_is_active_master_board();
	}
	
	return ret;
}

/**********************************************************************************
 *  dcli_dhcp_distributed_process
 *
 *	DESCRIPTION:
 * 		distributed process
 *
 *	INPUT:
 *		vty -> vty 
 *		bind -> interface bind/unbind pool
 *		ifname -> interface name
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		0 -> success
 *		1 -> failed
 *		
 **********************************************************************************/ 
int dcli_dhcp_distributed_process(struct vty *vty, int bind, char *ifname)
{
	int ret = 1;
	int active_master_slot = 1;
	int send_to_slot = 0;
	int value1 = 0, value2 = 0, value3 = 0;
	unsigned short protocol	= 17;	/* udp */
	unsigned int src_port = 0;
	unsigned int dest_port = 0;
	char *src_ipaddr = "all";
	char *dest_ipaddr = "all";	
	

	if (NOT_DISTRIBUTED == dcli_dhcp_is_distributed()) {
		return 0;
	}

	if (NULL == ifname) {
		return 1;
	}
	/*
	vty_out(vty, "%s\n", ifname);  	*/
	
	/* whether local broad interface, if local, return */
	if (0 != is_local_board_interface(ifname)){
	//	vty_out(vty, "is local board interface\n");
		
		return 1;
	}
	
	/* get slot */
	send_to_slot = get_slot_id_by_ifname(ifname);
	#ifdef DCLI_DHCP_DEBUG
	vty_out(vty,"%s: send to slot %d\n", ifname, send_to_slot);	
	#endif

	active_master_slot = dcli_dhcp_get_board_slot_id();
	/*
	vty_out(vty,"send to slot %d, master slot %d\n", send_to_slot, active_master_slot); */
	

#define ADD_PFM_FORWORD_TABLE		(0)
#define DEL_PFM_FORWORD_TABLE		(1)

#define DHCP_CLIENT_PORT			(68)
#define DHCP_SERVER_PORT			(67)

	if (INTERFACE_BIND_POOL == bind) {
		/*
		ret = dcli_communicate_pfm_by_dbus(opt,     			// pfm operate
											0,    				//
											protocol,   		// packet protocol (tcp : 6  udp : 17)
											(char*)"eth2-1",    // ifname
											port,     			// packet port number (dhcp : 67 68)
											slot,   			// active master slot 
											forward_opt,    	// -2 : forward all packet
											forward_opt_para,   // forward option parameter
											ipaddr,    			// packet destination ip adddresss 
											send_to_slot);     	// destination slot (exp: eth2-1, slot is 2)
		*/


		src_port = 0;		/* relay to dhcp server */
		dest_port = DHCP_SERVER_PORT;
		ret = dcli_communicate_pfm_by_dbus(ADD_PFM_FORWORD_TABLE, 0, protocol, 
											ifname, 
											src_port, dest_port, 
											active_master_slot,
											src_ipaddr, dest_ipaddr,
											send_to_slot);
		if (0 != ret) {
			return 1;
		}
		#if 0
		src_port = DHCP_CLIENT_PORT;		/* client to server */
		dest_port = DHCP_SERVER_PORT;
		ret = dcli_communicate_pfm_by_dbus(ADD_PFM_FORWORD_TABLE, 0, protocol, 
											ifname, 
											src_port, dest_port, 
											active_master_slot,
											src_ipaddr, dest_ipaddr,
											send_to_slot);
		if (0 != ret) {
			/* delete  (ADD_PFM_FORWORD_TABLE DHCPv6_CLIENT_PORT) */
			return 1;
		}
		#endif
	}
	else { /* unbind */	
		src_port = 0;
		dest_port = DHCP_SERVER_PORT;
		ret = dcli_communicate_pfm_by_dbus(DEL_PFM_FORWORD_TABLE, 0, protocol, 
											ifname, 
											src_port, dest_port, 
											active_master_slot,
											src_ipaddr, dest_ipaddr,
											send_to_slot);
		if (0 != ret) {
			return 1;
		}
		#if 0
		src_port = DHCP_CLIENT_PORT;
		dest_port = DHCP_SERVER_PORT;
		ret = dcli_communicate_pfm_by_dbus(DEL_PFM_FORWORD_TABLE, 0, protocol, 
											ifname, 
											src_port, dest_port, 
											active_master_slot,
											src_ipaddr, dest_ipaddr,
											send_to_slot);
		if (0 != ret) {
			/* delete  (ADD_PFM_FORWORD_TABLE DHCPv6_CLIENT_PORT) */
			return 1;
		}		
		#endif
	}
	return 0;
}



unsigned int
dcli_get_server_state
(
	struct vty *vty,
	unsigned int *state
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	unsigned int isenable = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_GET_SERVER_STATE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &state, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &isenable,		
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			*state = isenable;
			return CMD_SUCCESS;
		} 
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}
}


/**********************************************************************************
 *  check_interfaces_ip_address
 *
 *	DESCRIPTION:
 * 		Check The interface whether Settings IP address
 *
 *	INPUT:
 *		name -> the interface name 
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		
 *		1 -> the interface have set ip address
 *		0 -> the interface without setting the IP address  
 **********************************************************************************/
static int check_interfaces_ip_address(char *name) 
{
	struct ifreq tmp;
	int sock = -1;
	struct sockaddr_in *addr = NULL;

	if (NULL == name) {
		return 0;
	}
	
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		return 0;
	}

	strcpy(tmp.ifr_name, name);
	if (ioctl(sock, SIOCGIFADDR, &tmp) < 0) {
		close(sock);
		sock = -1;
		
		return 0;
	}

	addr = (struct sockaddr_in*)&tmp.ifr_addr;
	if (0 != addr->sin_addr.s_addr) {
		close(sock);
		sock = -1;

		return 1;
	}
	close(sock);
	return 0;
}


/*0 del 1 add*/
unsigned int
dcli_set_interface_ip_pool
(
	struct vty *vty,
	char* poolName,
	char* ifname,
	unsigned int add_info,
	unsigned int unbindflag
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0, ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
	
	ret = dcli_dhcp_check_relay_interface_iSbusy(vty, ifname);
	if (ret) {
		return ret;
	}
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_INTERFACE_POOL);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &poolName,							
							DBUS_TYPE_STRING, &ifname,
							DBUS_TYPE_UINT32, &add_info,
							DBUS_TYPE_UINT32, &unbindflag,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		dbus_message_unref(reply);
		
		return op_ret;
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}
}
int
	check_slot_wheather_empty
(
int slot_id
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	int i;
	int board_code;
	int is_master, is_active_master;
	unsigned int function_type;
	char *name;
	int slot_count;
	unsigned int board_on_mask;
	int board_state;
	//int slot_sum = 0 ;
	
	query = dbus_sem_msg_new_method_call(SEM_DBUS_BUSNAME, SEM_DBUS_OBJPATH,
										 SEM_DBUS_INTERFACE, SEM_DBUS_SHOW_SLOT_INFO);
	if (!query)
	{
		printf("show slot id query failed\n");
		return CMD_FAILURE;
	}

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);

	if (!reply)
	{
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	
	dbus_message_iter_get_basic(&iter, &slot_count);	
	dbus_message_iter_next(&iter);

	dbus_message_iter_get_basic(&iter, &board_on_mask); 
	dbus_message_iter_next(&iter);
	
	for (i=0; i<slot_count; i++)
	{	
		dbus_message_iter_get_basic(&iter, &board_state);	
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter, &board_code);
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter, &function_type);
		dbus_message_iter_next(&iter);
		
		dbus_message_iter_get_basic(&iter, &is_master);
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter, &is_active_master);
		dbus_message_iter_next(&iter);
		
		dbus_message_iter_get_basic(&iter, &name);
		if (i < slot_count-1)
		{
			dbus_message_iter_next(&iter);
		}
		
		if (board_on_mask & (0x1<<i))
		{
			
			if (board_state <= 1)
			{
				//vty_out(vty, "slot %d:not work normal\n", i+1);
				continue;
			}
			if((i+1) == slot_id){
				return 2;
			}
			/*
			vty_out(vty, "slot %d:\n", i+1);
			vty_out(vty, "\tBOARD_CODE:\t\t0x%x\n", board_code);
			vty_out(vty, "\tFUNCTION_TYPE:\t\t0x%x\n", function_type);
			vty_out(vty, "\tIS_MASTER:\t\t%s\n", is_master ? "YES" : "NO");
			vty_out(vty, "\tIS_ACTIVE_MASTER:\t%s\n", is_active_master ? "YES" : "NO");
			vty_out(vty, "\tBOARD_NAME:\t\t%s\n", name);
			*/
		}
		else
		{
			//vty_out(vty, "slot %d is empty\n", i+1);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int
	dcli_dhcp_check_ve_interface

(	
	char *ifname
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int ret = 0;
	unsigned int slot_id = HostSlotId;
	slot_id = get_slot_id_by_ifname(ifname);
	ret = check_slot_wheather_empty(slot_id);
	if(slot_id < 1 || slot_id > 17){
		return CMD_FAILURE;
	}
	if(2 != ret){
		printf("slot id is empty\n");
		return CMD_FAILURE;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	char *name=NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_CHECK_INTERFACE_VE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &ifname,					 
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_STRING, &name,
		DBUS_TYPE_INVALID)) {
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	memset(ifname, 0, ALIAS_NAME_SIZE);
	memcpy(ifname, name, strlen(name));
	return 0;
}


DEFUN(set_interface_ip_pool_cmd_func,
	set_interface_ip_pool_cmd,
	"ip pool POOLNAME",
	"Ipv4 address of portal\n"	
	"Config ip pool entity\n"
	"Specify pool name begins with char, and name length no more than 20 characters\n"
)
{

	char* poolName = NULL, *ifname = NULL;;
	unsigned int nameSize = 0, nodeSave = 0;
	int ret = 0, index = 0;
	unsigned int op_ret = 0;
#if 0
	if (IS_DISTRIBUTED == dcli_dhcp_is_distributed()) {
		ret = dcli_dhcp_is_master_board();
		if (1 == ret) {	/* master board */

			/* is active master board : 1(active), 0(not), -1(failed) */
			if (1 != dcli_dhcp_is_active_master_board()) {
				vty_out(vty, "not active master board\n");
				return CMD_WARNING;			
			}
			
		} else {
			vty_out(vty, "not master board\n");
			return CMD_WARNING;			
		}
	}
#endif
	nameSize = strlen(argv[0]);
	if (nameSize >= ALIAS_NAME_SIZE) {
		vty_out(vty, "pool name is too long\n");
		return CMD_WARNING;
	}		

	poolName = (char*)malloc(ALIAS_NAME_SIZE);
	if (NULL == poolName) {
		vty_out(vty, "not enough memory\n");
		return CMD_WARNING;
	}
	ifname = (char*)malloc(ALIAS_NAME_SIZE);
	if (NULL == ifname) {
		vty_out(vty, "not enough memory\n");
		free(poolName);
		return CMD_WARNING;
	}
	
	memset(poolName, 0, ALIAS_NAME_SIZE);
	memset(ifname, 0, ALIAS_NAME_SIZE);
	
	memcpy(poolName, argv[0], nameSize);
	
	nameSize = strlen(vlan_eth_port_ifname);
	memcpy(ifname, vlan_eth_port_ifname, nameSize);
	if(strncmp(ifname, "ve", 2) == 0)
	dcli_dhcp_check_ve_interface(ifname); 

	/*
	ret = check_interfaces_ip_address(ifname);
	if (!ret) {
		vty_out(vty, "Interface without setting the IP address, please first set IP address\n");
		free(poolName);
		free(ifname);
		poolName = NULL;
		return CMD_WARNING;
	}
	*/

	int localid = 1, slot_id = HostSlotId, indextmp = 0, dest_slotid = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);
	
	dest_slotid = get_slot_id_by_ifname(ifname);

	/* interface node: vty->slotindex = 0
	   hansi node -> interface node ->slotindex != 0 */
	if ((0 != vty->slotindex)
		&& (dest_slotid != vty->slotindex)) {
		vty_out(vty, "bind ip pool failed, Because not support.\n");
		if(poolName){
			free(poolName);
			poolName=NULL;
		}
		if(ifname){
			free(ifname);
			ifname=NULL;
		}
		return CMD_WARNING;			
	}

	ret = dcli_set_interface_ip_pool(vty, poolName, ifname, SET_INTERFACE_IP_POOL, INTERFACE_BIND_FLAG);
	if (!ret) {

		if (dest_slotid != slot_id) {
		dcli_dhcp_distributed_process(vty, INTERFACE_BIND_POOL, ifname);
		}
		
		free(poolName);
		free(ifname);
		poolName = NULL;
		vty_out(vty, "Successfully binding!\n");	
		return CMD_SUCCESS;
	}
	else {	
		if(DHCP_HAVE_BIND_INTERFACE == ret){
			vty_out(vty, "Failed because pool has already binded to interface\n");		
		}else if(DHCP_POOL_SUBNET_NULL == ret){
			vty_out(vty, "Failed because pool has no subnet\n");
		}else if (DHCP_NOT_FOUND_POOL == ret){
			vty_out(vty, "Failed because not found pool\n");
		}else{
			vty_out(vty, "Interface bind the pool failed\n");
		}
		free(poolName);
		free(ifname);
		poolName = NULL;
		return CMD_WARNING;
	}
}

DEFUN(del_interface_ip_pool_cmd_func,
	del_interface_ip_pool_cmd,
	"no ip pool POOLNAME",
	"Delete old Configuration\n"
	"Ipv4 address of portal\n"	
	"Delete ip pool entity\n"
	"Specify pool name begins with char, and name length no more than 20 characters\n"
)
{

	char* poolName = NULL, *ifname = NULL;;
	unsigned int nameSize = 0, nodeSave = 0;
	unsigned int state = 0;
	int ret = 0, index = 0;
	unsigned int op_ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);


	/* check server state */
	dcli_get_server_state(vty, &state);
	if (state) {
		vty_out(vty, "Please first disable dhcp server\n");
		return CMD_WARNING;		
	}

	nameSize = strlen(argv[0]);
	if (nameSize >= ALIAS_NAME_SIZE) {
		vty_out(vty, "pool name is too long\n");
		return CMD_WARNING;
	}

	poolName = (char*)malloc(ALIAS_NAME_SIZE);
	ifname = (char*)malloc(ALIAS_NAME_SIZE);
	memset(poolName, 0, ALIAS_NAME_SIZE);
	memset(ifname, 0, ALIAS_NAME_SIZE);
	
	memcpy(poolName, argv[0], nameSize);
	
	nameSize = strlen(vlan_eth_port_ifname);
	memcpy(ifname, vlan_eth_port_ifname, nameSize);
	if(strncmp(ifname, "ve", 2) == 0)
	dcli_dhcp_check_ve_interface(ifname); 

	ret = dcli_set_interface_ip_pool(vty, poolName, ifname, CLR_INTERFACE_IP_POOL, INTERFACE_UNBIND_FLAG);
	
	if (!ret) {

		int localid = 1, slot_id = HostSlotId, indextmp = 0, dest_slotid = 0;
		get_slotid_index(vty, &indextmp, &slot_id, &localid);
		dest_slotid = get_slot_id_by_ifname(ifname);
		if (dest_slotid != slot_id) {
		dcli_dhcp_distributed_process(vty, INTERFACE_UNBIND_POOL, ifname);
		}
		
		free(poolName);
		free(ifname);
		poolName = NULL;
		return CMD_SUCCESS;
	}
	else {	
		if(DHCP_POOL_SUBNET_NULL == ret){
			vty_out(vty, "Failed because pool has no subnet\n");
		}else if (DHCP_NOT_FOUND_POOL == ret){
			vty_out(vty, "Failed because not found pool\n");
		}else{
			vty_out(vty, "Interface unbind the pool failed\n");
		}
		free(poolName);
		free(ifname);
		poolName = NULL;
		return CMD_WARNING;
	}
}

int 
dcli_add_dhcp_pool_ip_range
(
	struct vty *vty,
	unsigned int add,
	unsigned int ipaddrl, 
	unsigned int ipaddrh,
	unsigned int ipmask,
	unsigned int index
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError 		err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned int	op_ret = 0, ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_ADD_IP_POOL_RANGE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &add,
							DBUS_TYPE_UINT32, &ipaddrl,	
							DBUS_TYPE_UINT32, &ipaddrh,
							DBUS_TYPE_UINT32, &ipmask,							
							DBUS_TYPE_UINT32, &index, 
							DBUS_TYPE_INVALID);		   
   reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
			   
   dbus_message_unref(query);
   
   if (NULL == reply) {
	   if (dbus_error_is_set(&err)) {
		   dbus_error_free_for_dcli(&err);
	   }
	   return CMD_SUCCESS;
   }
   
   if (dbus_message_get_args ( reply, &err,
				   DBUS_TYPE_UINT32, &op_ret,
				   DBUS_TYPE_INVALID)) {
	   dbus_message_unref(reply);
	   if(!op_ret) {
		   return CMD_SUCCESS;
	   }
	   else {
			return op_ret;
	   }
   } 
   else {
	   if (dbus_error_is_set(&err)) {
		   dbus_error_free_for_dcli(&err);
	   }
	   dbus_message_unref(reply);
   
	   return CMD_WARNING;
   }
}

DEFUN(add_dhcp_pool_ip_range_cmd_func,
	add_dhcp_pool_ip_range_cmd,
	"(add|delete) range A.B.C.D A.B.C.D mask A.B.C.D",
	"Add dhcp ip range\n"
	"Delete dhcp ip range\n"
	"Range ip\n"
	"Low ip A.B.C.D\n"
	"High ip A.B.C.D\n"
	"Mask ip\n"
	"Mask ip A.B.C.D\n"
)
{
	unsigned int ipAddrl = 0, ipAddrh = 0, ipmask = 0, ip_Nums = 0;
	unsigned int ret = 0, add = 0, index = 0;
	unsigned int ipaddr = 0, i = 0;
/*	
	char *poolName = NULL;

	poolName = malloc((strlen(vty->index) + 1));
	memset(poolName, 0, ((strlen(vty->index) + 1)));
	memcpy(poolName, (char *)vty->index, (strlen(vty->index) + 1));
	*/
	index = (unsigned int)(vty->index_sub);

	/*add 0 means add, add 1 means delete*/
	if(strncmp("add", argv[0], strlen(argv[0]))==0) {
		add = 0;
	}
	else if (strncmp("delete", argv[0], strlen(argv[0]))==0) {
		add = 1;
	}
	else {
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}

	/* check ip address 0.X.X.X */
	for (i = 1; i < 3; i++) {
		ipaddr = 0;
		if(1 == inet_pton(AF_INET, (char *)argv[i], &ipaddr)) {
			if (0 == (ipaddr & 0xff000000)) {
				vty_out(vty, "invalid ip address : %s(0.X.X.X)\n", argv[i]);
				return CMD_WARNING;
			}
		} else {
			vty_out(vty, "invalid ip address %s\n", (char *)argv[i]);
			return CMD_WARNING;
		}
		if (dcli_dhcp_check_ip_address((char *)argv[i], NULL)) {
			vty_out(vty, "invalid ip address %s\n", (char *)argv[i]);
			return CMD_WARNING;
		}
	}
	if (dcli_dhcp_check_mask((char *)argv[3], NULL)) {
		vty_out(vty, "invalid mask: %s\n", (char *)argv[3]);
		return CMD_WARNING;
	}
	

	ipAddrl = dcli_ip2ulong((char*)argv[1]);
	ipAddrh = dcli_ip2ulong((char*)argv[2]);
	ipmask = dcli_ip2ulong((char*)argv[3]);

	if(0 == ((~ipmask)&ipAddrh) || ipmask == (~((~ipmask)&ipAddrh)) || 0 == ((~ipmask)&ipAddrl) || ipmask == (~((~ipmask)&ipAddrl))){
		vty_out(vty, "ip range fail ,because net and broadcast ip is not used!\n");
		return CMD_WARNING;
	}
	if ((ipAddrl > ipAddrh) 
		|| ((ipAddrl & ipmask) != (ipAddrh & ipmask))) {	
		vty_out(vty, "%s ip range fail \n", add ? "delete" : "add");
		return CMD_WARNING;
	}		

	ip_Nums = ipAddrh - ipAddrl;
	if ((ip_Nums > 0xffff) || (ipmask < 0xffff)){
		vty_out(vty, "%s ip range fail \n", add ? "delete" : "add");
		return CMD_WARNING;
	}	

	ret = dcli_add_dhcp_pool_ip_range(vty, add, ipAddrl, ipAddrh, ipmask, index);
	if (ret) {
		if(DHCP_INCORRET_IP_ADDRESS == ret){
			vty_out(vty, "fail because incorrect ip address\n");
		}else if (DHCP_TOO_MANY_HOST_NUM == ret){
			vty_out(vty, "fail because total host num too many\n");
		}else if(DHCP_NOT_THE_SUBNET == ret) {
			vty_out(vty, "fail because %s\n", (add ? "poolNode has no subnet" : "not the same subnet"));
		}else if(DHCP_UNBIND_BY_INTERFACE == ret){
			vty_out(vty, "fail because bound to interface cann't %s\n", add ? "delete" : "add");
		}else if(DHCP_NOT_FOUND_POOL == ret){
			vty_out(vty, "fail because not found pool\n");
		}else if(DHCP_SUBNET_EXIST == ret){
			vty_out(vty, "failed because same subnet exist!");			
		}else {
			vty_out(vty, "%s ip range fail \n", add ? "delete" : "add");
		}
	}
	
	return CMD_SUCCESS;
}

int 
dcli_add_dhcp_static_host
(
	struct vty *vty,
	unsigned int ipaddr, 
	unsigned char *mac,
	unsigned char *ifname,
	unsigned int add
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError 		err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned int	op_ret = 0, ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_ADD_STATIC_HOST);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &ipaddr,
							DBUS_TYPE_UINT32, &add,
							DBUS_TYPE_BYTE, &mac[0],	
							DBUS_TYPE_BYTE, &mac[1],
							DBUS_TYPE_BYTE, &mac[2],
							DBUS_TYPE_BYTE, &mac[3], 
							DBUS_TYPE_BYTE, &mac[4],
							DBUS_TYPE_BYTE, &mac[5],		
							DBUS_TYPE_STRING, &ifname, 
							DBUS_TYPE_INVALID);		   
   reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
			   
   dbus_message_unref(query);
   
   if (NULL == reply) {
	   if (dbus_error_is_set(&err)) {
		   dbus_error_free_for_dcli(&err);
	   }
	   return CMD_SUCCESS;
   }
   
   if (dbus_message_get_args ( reply, &err,
				   DBUS_TYPE_UINT32, &op_ret,
				   DBUS_TYPE_INVALID)) {
	   if(!op_ret) {
		   dbus_message_unref(reply);
		   return CMD_SUCCESS;
	   }
	   else {
			return op_ret;
	   }
   } 
   else {
	   if (dbus_error_is_set(&err)) {
		   dbus_error_free_for_dcli(&err);
	   }
	   dbus_message_unref(reply);
   
	   return CMD_WARNING;
   }
}

DEFUN(add_dhcp_static_host_cmd_func,
	add_dhcp_static_host_cmd,
	"ip dhcp static A.B.C.D HH:HH:HH:HH:HH:HH IFNAME",
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"ADD dhcp static host\n"
	"Static host ip address\n"
	"Static host mac address\n"
	"Static host interface name\n"
)
{
	unsigned int ipAddr = 0, nameSize = 0;
	unsigned int ret = 0;
	unsigned char macAddr[MAC_ADDRESS_LEN];
	unsigned int ipaddr = 0;
	unsigned int ebrid = 0, wlanid = 0;
	char ifname[ALIAS_NAME_SIZE] = {0};
	

	if(1 == inet_pton(AF_INET, (char *)argv[0], &ipaddr)) {
		if (0 == (ipaddr & 0xff000000)) {
			vty_out(vty, "invalid ip address : %s(0.X.X.X)\n", argv[0]);
			return CMD_WARNING;
		}
	} else {
		vty_out(vty, "invalid ip address %s\n", (char *)argv[0]);
		return CMD_WARNING;
	}

	if (dcli_dhcp_check_ip_address((char *)argv[0], NULL)) {
		vty_out(vty, "invalid ip address: %s\n", (char *)argv[0]);
		return CMD_WARNING;
	}

	nameSize = strlen(argv[2]);	
	if(nameSize > ALIAS_NAME_SIZE){
		vty_out (vty, "interface name is too long\n");
		return CMD_WARNING;
	}

	ipAddr = dcli_ip2ulong((char*)argv[0]);
	
	memset(&macAddr, 0, MAC_ADDRESS_LEN);	
	ret = parse_mac_addr((char *)argv[1], &macAddr);
	if (ret) {
		vty_out(vty, "bad mac address \n");
		return CMD_WARNING;
	}	
	memset(ifname, 0, ALIAS_NAME_SIZE);
	memcpy(ifname, argv[2], nameSize);
    
    /* zhangshu modify */
	if(NULL == strchr(ifname,'-'))
	    dcli_dhcp_convert_ifname(vty, ifname);
	
	if(strncmp(ifname, "ve", 2) == 0)
	    ret = dcli_dhcp_check_ve_interface(ifname);
	if(ret){
		vty_out(vty, "slot id wrong!\n");
		return CMD_WARNING;
	}
	
	ret = dcli_add_dhcp_static_host(vty, ipAddr, macAddr, ifname, 1);
	if(ret){
		if(DHCP_HAVE_CONTAIN_STATIC_USER == ret){
			vty_out(vty, "fail because have contained the static user\n");		
		}else{
			vty_out(vty, "fail operation\n");
		}
	}else{
	    vty_out(vty, "command successful.\n");
	}
	
	return CMD_SUCCESS;
}

DEFUN(delete_dhcp_static_host_cmd_func,
	delete_dhcp_static_host_cmd,
	"no ip dhcp static A.B.C.D HH:HH:HH:HH:HH:HH IFNAME",
	"Delete dhcp static host"
	"Config dhcp server"
	"Static host ip address"
	"Static host mac address"
	"Static host interface name"
)
{
	unsigned int ipAddr = 0, nameSize = 0;
	unsigned int ret = 0, index = 0;
	unsigned char macAddr[MAC_ADDRESS_LEN], *ifname = NULL;

	nameSize = strlen(argv[2]);	
	if(nameSize > ALIAS_NAME_SIZE){
		vty_out (vty, "interface name is too long\n");
		return CMD_WARNING;
	}

	ifname = (char*)malloc(ALIAS_NAME_SIZE);
	memset(ifname, 0, ALIAS_NAME_SIZE);

	ipAddr = dcli_ip2ulong((char*)argv[0]);
	
	memset(&macAddr, 0, MAC_ADDRESS_LEN);	
	ret = parse_mac_addr((char *)argv[1], &macAddr);
	if (ret) {
		vty_out(vty, "bad mac address \n");
		if(ifname){
			free(ifname);
			ifname=NULL;
		}
		return CMD_WARNING;
	}	

	memcpy(ifname, argv[2], nameSize);
	dcli_dhcp_convert_ifname(vty, ifname);
	if(strncmp(ifname, "ve", 2) == 0)
	    dcli_dhcp_check_ve_interface(ifname); 

	ret = dcli_add_dhcp_static_host(vty, ipAddr, macAddr, ifname, 0);
	if(ret){
		if(DHCP_NOT_FOUND_STATIC_USER == ret){
			vty_out(vty, "fail because don't find the static user\n");
		}else{
			vty_out(vty, "fail operation\n");
		}
	}else{
	    vty_out(vty, "command successful.\n");
		
	}
		if(ifname){
			free(ifname);
			ifname=NULL;
	}
	return CMD_SUCCESS;
}

unsigned int
dcli_set_server_domain_name
(
	struct vty *vty,
	char *domainName,	
	unsigned int mode,
	unsigned int index,
	unsigned int del
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	if (!domainName) {
		return 1;
	}

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_SERVER_DOMAIN_NAME);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_STRING, &domainName, 
							 DBUS_TYPE_UINT32, &mode,							 
							 DBUS_TYPE_UINT32, &index, 							 
							 DBUS_TYPE_UINT32, &del, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
		else if(DHCP_SET_ORDER_WANNING == op_ret){
			vty_out (vty, "first set ip range , Have not configured address pool range!\n");
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}else if(DHCP_NOT_FOUND_POOL  == op_ret) {
			vty_out (vty, "Not found the pool\n");
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}else if(DHCP_SERVER_RETURN_CODE_FAIL == op_ret){
			vty_out (vty, "domain name wrong!\n");
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}
}

DEFUN(ip_dhcp_server_domain_name_cmd_func,
	ip_dhcp_server_domain_name_cmd,
	"ip dhcp server domain NAME",
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"Ip dhcp server domain entity\n"
	"Specify domain name begins with char, and name length no more than 20 characters\n"
)
{
	char* domainName = NULL;
	unsigned int nameSize = 0, nodeSave = 0, index = 0;
	unsigned int ret = 0, op_ret = 0, mode = 0;
	char *ptr = NULL;

	nameSize = strlen(argv[0]);
	if(nameSize > DOMAIN_NAME){
		vty_out (vty, "domain name is too long\n");
		return CMD_WARNING;
	}

	/*printf("before parse_vlan_name %s length %d\n",argv[1],strlen(argv[1]));*/
	domainName = (char*)malloc(DOMAIN_NAME);
	memset(domainName, 0, DOMAIN_NAME);	
	
	
	memcpy(domainName, argv[0], nameSize);
	ptr = domainName;
	while (*ptr) {

		/* allow ab..z, AB...Z, 01...9, _, -, @	 */
		if ((*ptr >= 'a' && *ptr <= 'z') || (*ptr >= 'A' && *ptr <= 'Z')
			|| (*ptr >= '0' && *ptr <= '9')
			|| (*ptr == '_') || (*ptr == '-') || (*ptr == '.') || (*ptr == '@')) 
		{
			ptr++;
		} else {
			vty_out(vty, "domain name contains invalid character : '%c'\n", *ptr);
			if(domainName){
			free(domainName);
			domainName = NULL;
			}
			return CMD_WARNING;
		}
	}
	//domainName[nameSize+1] = '\0';
	/*mode 0 means global mode, 1 means pool mode */
	
	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;
	}
	else if((POOL_NODE == vty->node) || (HANSI_POOL_NODE == vty->node) || (LOCAL_HANSI_POOL_NODE  == vty->node)) {
		mode = 1;		
		index = (unsigned int)(vty->index_sub);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		if(domainName){
		free(domainName);
		domainName = NULL;
		}
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_domain_name(vty,domainName, mode, index, 0);
	if (!ret) {
		if(domainName){
		free(domainName);
		domainName = NULL;
		}
		return CMD_SUCCESS;
	}
	else {
		vty_out(vty,"% Bad parameter,pool %s NOT exists.\n", domainName);		
		if(domainName){
		free(domainName);
		domainName = NULL;
		}
		return CMD_WARNING;
	}
}

DEFUN(no_ip_dhcp_server_domain_name_cmd_func,
	no_ip_dhcp_server_domain_name_cmd,
	"no ip dhcp server domain NAME",
	"Delete old Configuration\n"
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"NO ip dhcp server domain entity\n"
)
{
	char* domainName = NULL;
	unsigned int nameSize = 0, nodeSave = 0, index = 0;
	unsigned int ret = 0, op_ret = 0, mode = 0;

	/*printf("before parse_vlan_name %s length %d\n",argv[1],strlen(argv[1]));*/
	domainName = (char*)malloc(ALIAS_NAME_SIZE);
	memset(domainName, 0, ALIAS_NAME_SIZE);
	nameSize = strlen(argv[0]);	
	
	if (nameSize >= ALIAS_NAME_SIZE) {
		vty_out(vty, "domain name is too long\n");
		free(domainName);
		domainName = NULL;	
		return CMD_WARNING;
	}
	memcpy(domainName, argv[0], nameSize);
	/*mode 0 means global mode, 1 means pool mode */

	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;
	}
	else if((POOL_NODE == vty->node) || (HANSI_POOL_NODE == vty->node) || (LOCAL_HANSI_POOL_NODE  == vty->node)) {	
		mode = 1;		
		index = (unsigned int)(vty->index_sub);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		if(domainName){
			free(domainName);
			domainName=NULL;
		}
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_domain_name(vty,domainName, mode, index, 1);
	if (!ret) {
		if(domainName){
			free(domainName);
			domainName=NULL;
		}
		return CMD_SUCCESS;
	}
	else {
		if(domainName){
			free(domainName);
			domainName=NULL;
		}
		vty_out(vty,"% Bad parameter,pool %s NOT exists.\n", domainName);		
		return CMD_WARNING;
	}
}

unsigned int
dcli_set_server_option43_v1
(
	struct vty *vty,
	char *veo,	
	unsigned int mode,
	unsigned int index,
	unsigned int del
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	if (!veo) {
		return 1;
	}

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
		
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_SERVER_VEO_OLD);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_STRING, &veo, 
							 DBUS_TYPE_UINT32, &mode,							 
							 DBUS_TYPE_UINT32, &index, 							 
							 DBUS_TYPE_UINT32, &del, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}else if(DHCP_SET_ORDER_WANNING == op_ret){
			vty_out (vty, "first set ip range , Have not configured address pool range!\n");
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}else if(DHCP_NOT_FOUND_POOL  == op_ret) {
			vty_out (vty, "Not found the pool\n");
			dbus_message_unref(reply);
			return CMD_SUCCESS;

		}else if(DHCP_ALREADY_ADD_OPTION  == op_ret) {
			vty_out (vty, "The option43 already add\n");
			dbus_message_unref(reply);
			return CMD_SUCCESS;

		}else{
			vty_out (vty, "fail operation\n");
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}
}

DEFUN(ip_dhcp_server_option43_v1_cmd_func,
	ip_dhcp_server_option43_v1_cmd,
	"ip dhcp server option43 HEX",
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"Ip dhcp server option43 entity\n"
	"option43 information\n"
)
{
	char* veo = NULL;
	unsigned int size = 0, index = 0, len = 0;
	unsigned int ret = 0, op_ret = 0, mode = 0;

	size = strlen(argv[0]);
	veo = (char*)malloc(size + 1);
	memset(veo, 0, (size + 1));
	
	memcpy(veo, argv[0], size);

	/*mode 0 means global mode, 1 means pool mode */

	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;
	}
	else if((POOL_NODE == vty->node) || (HANSI_POOL_NODE == vty->node) || (LOCAL_HANSI_POOL_NODE  == vty->node)) {	
		mode = 1;		
		index = (unsigned int)(vty->index_sub);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_option43_v1(vty,veo, mode, index, 0);
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		vty_out(vty, "set dhcp server option43 fail\n", veo);		
		return CMD_WARNING;
	}
}


DEFUN(no_ip_dhcp_server_option43_v1_cmd_func,
	no_ip_dhcp_server_option43_v1_cmd,
	"no ip dhcp server option43 HEX",
	"Delete old Configuration\n"
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"Ip dhcp server option43 entity\n"	
	"hexadecimal string, include of  type, string Length and ipv4 address list hexadecimal form\n"
)
{
	char* veo = NULL;
	unsigned int ret = 0, op_ret = 0, mode = 0, index = 0, size = 0;

	size = strlen(argv[0]);
	veo = (char*)malloc(size + 1);
	memset(veo, 0, (size + 1));
	
	memcpy(veo, argv[0], size);

	/*mode 0 means global mode, 1 means pool mode */
	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;
	}
	else if((POOL_NODE == vty->node) || (HANSI_POOL_NODE == vty->node) || (LOCAL_HANSI_POOL_NODE  == vty->node)) {
		mode = 1;		
		index = (unsigned int)(vty->index_sub);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		if(veo){
			free(veo);
			veo=NULL;
		}
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_option43_v1(vty,veo, mode, index, 1);
	if (!ret) {
		if(veo){
			free(veo);
			veo=NULL;
		}
		return CMD_SUCCESS;
	}
	else {
		vty_out(vty, "delete dhcp server option43 fail\n", veo);
		if(veo){
			free(veo);
			veo=NULL;
		}
		return CMD_WARNING;
	}
}


unsigned int
dcli_set_server_option43
(
	struct vty *vty,
	char*ap_via_address,	
	unsigned int mode,
	unsigned int index,
	unsigned int del
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	int i,j;

	if (NULL == ap_via_address || NULL == vty) {
		return CMD_WARNING;
	}

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_SERVER_VEO);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_STRING, &ap_via_address , 							  
							 DBUS_TYPE_UINT32, &mode,							 
							 DBUS_TYPE_UINT32, &index, 							 
							 DBUS_TYPE_UINT32, &del, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		goto free_exit;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			goto free_exit;
		}else if(DHCP_SET_ORDER_WANNING == op_ret){
			vty_out (vty, "first set ip range , Have not configured address pool range!\n");
			dbus_message_unref(reply);
			goto free_exit;
		}else if(DHCP_NOT_FOUND_POOL  == op_ret) {
			vty_out (vty, "Not found the pool\n");
			dbus_message_unref(reply);
			goto free_exit;

		}else if(DHCP_MAX_OPTION_LIST_WANNING  == op_ret) {
			vty_out (vty, "Max set sixteen AP address\n");
			dbus_message_unref(reply);
			goto free_exit;

		}else if(DHCP_ALREADY_ADD_OPTION  == op_ret) {
			vty_out (vty, "The option43 already add\n");
			dbus_message_unref(reply);
			goto free_exit;

		}else{
			vty_out (vty, "fail operation\n");
			dbus_message_unref(reply);
			goto free_exit;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		 
		if(ap_via_address != NULL){
			free(ap_via_address);
		}	
		return CMD_WARNING;
	}
free_exit:
	
	if(ap_via_address != NULL){
		free(ap_via_address);
	}

	return CMD_SUCCESS;
}

DEFUN(ip_dhcp_server_option43_cmd_func,
	ip_dhcp_server_option43_cmd,
	"ip dhcp server option43 A.B.C.D",
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"Ip dhcp server option43 entity\n"
	"Ipv4 address\n"
)
{
	char *ap_via_address = NULL;
	unsigned int  index = 0;
	unsigned int ret = 0, mode = 0;
	unsigned int ipaddr = 0;
	struct in_addr ipv4_adr;
	char ipv4list[MAX_IP_STRING_LEN] = {0};
	
	if((ret = inet_pton (AF_INET, (char*)argv[0], &ipv4_adr)) != 1){			
		vty_out(vty, "input IPV4 address %s  is Illegal\n", argv[0]);
		return CMD_WARNING;
	}
	/* check ip address 0.X.X.X */
	ipaddr = ipv4_adr.s_addr;
	if (0 == (ipaddr & 0xff000000)) {
		vty_out(vty, "invalid ip address %s(0.X.X.X)\n", argv[0]);
		return CMD_WARNING;
	}

	if (dcli_dhcp_check_ip_address((char *)argv[0], NULL)) {
		vty_out(vty, "invalid ip address: %s\n", (char *)argv[0]);
		return CMD_WARNING;
	}

	ap_via_address = malloc(MAX_IP_STRING_LEN);
	if(NULL == ap_via_address){
		vty_out(vty, "Failure to apply memory resources\n");
		return CMD_WARNING;
	}
	memset(ap_via_address, 0, MAX_IP_STRING_LEN);	

	memcpy(ap_via_address, argv[0],  strlen(argv[0]) + 1);


	/*mode 0 means global mode, 1 means pool mode */
	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;
	}
	else if((POOL_NODE == vty->node) || (HANSI_POOL_NODE == vty->node) || (LOCAL_HANSI_POOL_NODE  == vty->node)) {
		mode = 1;		
		index = (unsigned int)(vty->index_sub);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		if(ap_via_address){
			free(ap_via_address);
			ap_via_address=NULL;
		}
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_option43(vty, ap_via_address,  mode, index, ADD_OPTION);	
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		vty_out(vty, "set dhcp server option43 fail\n");		
		return CMD_WARNING;
	}
}

DEFUN(no_ip_dhcp_server_option43_cmd_func,
	no_ip_dhcp_server_option43_cmd,
	"no ip dhcp server option43 A.B.C.D",
	"Delete old Configuration\n"
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"Ip dhcp server option43 entity\n"
	"Ipv4 address\n"
)
{
	char*ap_via_address = NULL;
	unsigned int ret = 0,  mode = 0, index = 0;	
	struct in_addr ipv4_adr;
	
	if((ret = inet_pton (AF_INET, (char*)argv[0], &ipv4_adr)) != 1){			
		vty_out(vty, "input IPV4 address %s  is Illegal\n", argv[0]);
		return CMD_WARNING;
	}
	
	ap_via_address = malloc(MAX_IP_STRING_LEN);
	if(NULL == ap_via_address){
		vty_out(vty, "Failure to apply memory resources\n");
		return CMD_WARNING;
	}
	memset(ap_via_address, 0, MAX_IP_STRING_LEN);	

	memcpy(ap_via_address, argv[0],  strlen(argv[0]) + 1);

	/*mode 0 means global mode, 1 means pool mode */	
	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;
	}
	else if((POOL_NODE == vty->node) || (HANSI_POOL_NODE == vty->node) || (LOCAL_HANSI_POOL_NODE  == vty->node)) {
		mode = 1;		
		index = (unsigned int)(vty->index_sub);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_option43(vty,ap_via_address, mode, index, DELETE_OPTION);
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		vty_out(vty, "delete dhcp server option43 fail\n");		
		return CMD_WARNING;
	}
}


unsigned int
dcli_set_server_no_option43
(	struct vty *vty,
	unsigned int index
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_SERVER_NO_VEO);

	dbus_error_init(&err);
	dbus_message_append_args(query,							 					 
							 DBUS_TYPE_UINT32, &index, 						
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}else{

			vty_out (vty, "fail operation\n");
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
	}else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
 
		return CMD_WARNING;
	}	
}


DEFUN(del_ip_dhcp_server_option43_cmd_func,
	del_ip_dhcp_server_option43_cmd,
	"no ip dhcp server option43",
	"Delete old Configuration\n"
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"No ip dhcp server option43 entity\n"
)
{	
	unsigned int ret = 0,   index = 0;	

	/*Get pool index*/	
	if((POOL_NODE == vty->node) || (HANSI_POOL_NODE == vty->node) || (LOCAL_HANSI_POOL_NODE == vty->node)){				
		index = (unsigned int)(vty->index_sub);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_no_option43(vty , index);
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		vty_out(vty, "delete dhcp server option43 fail\n");		
		return CMD_WARNING;
	}
}


unsigned int
dcli_set_server_option138
(
	struct vty *vty,
	char *ap_via_address,	
	unsigned int mode,
	unsigned int index,
	unsigned int del
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	int i,j;

	if (!ap_via_address || !vty) {
		return CMD_WARNING;
	}

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_SERVER_OPTION138);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_STRING, &ap_via_address, 							
							 DBUS_TYPE_UINT32, &mode,							 
							 DBUS_TYPE_UINT32, &index, 							 
							 DBUS_TYPE_UINT32, &del, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			return CMD_SUCCESS;

		}else if(DHCP_SET_ORDER_WANNING == op_ret){
			vty_out (vty, "first set ip range , Have not configured address pool range!\n");
			dbus_message_unref(reply);
			return CMD_SUCCESS;

		}else if(DHCP_NOT_FOUND_POOL  == op_ret) {
			vty_out (vty, "Not found the pool\n");
			dbus_message_unref(reply);
			return CMD_SUCCESS;

		}else if(DHCP_ALREADY_ADD_OPTION  == op_ret) {
			vty_out (vty, "The option138 already add\n");
			dbus_message_unref(reply);
			return CMD_SUCCESS;

		}else if(DHCP_MAX_OPTION_LIST_WANNING  == op_ret) {
			vty_out (vty, "Max set sixteen AP address\n");
			dbus_message_unref(reply);
			return CMD_SUCCESS;		
			
		}else{
			vty_out (vty, "fail operation\n");
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
 
		return CMD_WARNING;
	}
 
}


DEFUN(ip_dhcp_server_option138_cmd_func,
	ip_dhcp_server_option138_cmd,
	"ip dhcp server option138 A.B.C.D",
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"	
	"Ip dhcp server option138 entity\n"
	"Ipv4 address list\n"
)
{
	char *ap_via_address = NULL;
	unsigned int size = 0, index = 0, len = 0;
	unsigned int ret = 0, op_ret = 0, mode = 0;
	struct in_addr ipv4_adr;
	unsigned int ipaddr = 0;

	if((ret = inet_pton (AF_INET, (char*)argv[0], &ipv4_adr)) != 1){			
		vty_out(vty, "input IPV4 address %s  is Illegal\n", argv[0]);
		return CMD_WARNING;
	}
	/* check ip address 0.X.X.X */
	ipaddr = ipv4_adr.s_addr;
	if (0 == (ipaddr & 0xff000000)) {
		vty_out(vty, "invalid ip address %s(0.X.X.X)\n", argv[0]);
		return CMD_WARNING;
	}
	if (dcli_dhcp_check_ip_address((char *)argv[0], NULL)) {
		vty_out(vty, "invalid ip address: %s\n", (char *)argv[0]);
		return CMD_WARNING;
	}

	ap_via_address = argv[0];


	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;
	}
	else if((POOL_NODE == vty->node) || (HANSI_POOL_NODE == vty->node) || (LOCAL_HANSI_POOL_NODE  == vty->node)) {
		mode = 1;		
		index = (unsigned int)(vty->index_sub);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_option138(vty, ap_via_address,  mode, index, ADD_OPTION);
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		vty_out(vty, "set dhcp server option138 fail\n");		
		return CMD_WARNING;
	}
}

DEFUN(no_ip_dhcp_server_option138_cmd_func,
	no_ip_dhcp_server_option138_cmd,
	"no ip dhcp server option138 A.B.C.D",
	"Delete old Configuration\n"
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"No ipv4 address list\n"	
)
{
	char *ap_via_address = NULL;
	unsigned int ret = 0, op_ret = 0, mode = 0, index = 0, size = 0;
	struct in_addr ipv4_adr;

	if((ret = inet_pton (AF_INET, (char*)argv[0], &ipv4_adr)) != 1){			
		vty_out(vty, "input IPV4 address %s  is Illegal\n", argv[0]);
		return CMD_WARNING;
	}	

	ap_via_address = argv[0];
	
	/*mode 0 means global mode, 1 means pool mode */
	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;
	}
	else if((POOL_NODE == vty->node) || (HANSI_POOL_NODE == vty->node) || (LOCAL_HANSI_POOL_NODE  == vty->node)) {
		mode = 1;		
		index = (unsigned int)(vty->index_sub);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_option138(vty, ap_via_address, mode, index, DELETE_OPTION);
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		vty_out(vty, "delete dhcp server option138 fail\n");		
		return CMD_WARNING;
	}
}


unsigned int
dcli_set_server_no_option138
(	struct vty *vty,
	unsigned int index
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);	
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_SERVER_NO_OPTION138);

	dbus_error_init(&err);
	dbus_message_append_args(query,							 					 
							 DBUS_TYPE_UINT32, &index, 						
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}else if (DHCP_HAVE_BIND_INTERFACE == op_ret) {
		
			vty_out (vty, "pool have binding to interface, please first unbind!\n");
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		} else {

			vty_out (vty, "fail operation\n");
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
	}else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
 
		return CMD_WARNING;
	}	
}


DEFUN(del_ip_dhcp_server_option138_cmd_func,
	del_ip_dhcp_server_option138_cmd,
	"no ip dhcp server option138",
	"Delete old Configuration\n"
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"No Ip dhcp server option138 entity\n"	
)
{	
	unsigned int ret = 0,   index = 0;	

	/*Get pool index*/	
	if((POOL_NODE == vty->node) || (HANSI_POOL_NODE == vty->node) || (LOCAL_HANSI_POOL_NODE == vty->node)){				
		index = (unsigned int)(vty->index_sub);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_no_option138(vty , index);
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		vty_out(vty, "delete dhcp server option43 fail\n");		
		return CMD_WARNING;
	}
}

unsigned int
dcli_set_server_dns
(
	struct vty *vty,
	unsigned int *ipAddr,
	unsigned int ipNum,
	unsigned int mode,
	unsigned int index,
	unsigned int del
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_SERVER_DNS);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &ipNum,							 
							 DBUS_TYPE_UINT32, &ipAddr[0], 
							 DBUS_TYPE_UINT32, &ipAddr[1], 
							 DBUS_TYPE_UINT32, &ipAddr[2], 
							 DBUS_TYPE_UINT32, &mode,							 
							 DBUS_TYPE_UINT32, &index, 
							 DBUS_TYPE_UINT32, &del,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}else if(DHCP_SET_ORDER_WANNING == op_ret){
			vty_out (vty, "first set ip range , Have not configured address pool range!\n");
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}else if(DHCP_NOT_FOUND_POOL  == op_ret) {
			vty_out (vty, "Not found the pool\n");
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		} else {
			vty_out (vty, "fail operation\n");
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}
}

DEFUN(ip_dhcp_server_dns_cmd_func,
	ip_dhcp_server_dns_cmd,
	"ip dhcp server dns A.B.C.D [A.B.C.D] [A.B.C.D]",	
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"Ip dhcp server dns entity\n"
	"Ipv4 address list\n"
)
{
	unsigned int ipAddr[3] = {0, 0, 0}, ipnum = 0;
	unsigned int ret = 0, op_ret = 0, mode = 0, index = 0, i = 0;
	unsigned int ipaddr = 0;
		
	ipnum = argc;
	for(;i < ipnum; i++) {

		/* check ip address 0.X.X.X */
		if(1 == inet_pton(AF_INET, (char *)argv[i], &ipaddr)) {
			if (0 == (ipaddr & 0xff000000)) {
				vty_out(vty, "invalid ip address %s(0.X.X.X)\n", argv[i]);
				return CMD_WARNING;
			}
		} else {
			vty_out(vty, "invalid ip address %s\n", (char *)argv[i]);
			return CMD_WARNING;
		}
		if (dcli_dhcp_check_ip_address((char *)argv[i], NULL)) {
			vty_out(vty, "invalid ip address: %s\n", (char *)argv[i]);
			return CMD_WARNING;
		}
		
		ipAddr[i] = dcli_ip2ulong((char*)argv[i]);
	}
	/*vty_out(vty, "dns ip is %x, %x ,%x \n", ipAddr[0], ipAddr[1], ipAddr[2]);*/
	/*mode 0 means global mode, 1 means pool mode */
	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;
	}
	else if((POOL_NODE == vty->node) || (HANSI_POOL_NODE == vty->node) || (LOCAL_HANSI_POOL_NODE  == vty->node)) {	
		mode = 1;		
		index = (unsigned int)(vty->index_sub);
		/*vty_out(vty,"ip pool server domain name sub index is %d\n", index);*/
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_dns(vty,ipAddr, ipnum, mode, index, 0);
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

DEFUN(no_ip_dhcp_server_dns_cmd_func,
	no_ip_dhcp_server_dns_cmd,
	"no ip dhcp server dns",
	"Delete old Configuration\n"
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"No ip dhcp server dns entity\n"
)
{
	unsigned int ipAddr[3] = {0, 0, 0}, ipnum = 0;
	unsigned int ret = 0, op_ret = 0, mode = 0, index = 0, i = 0;

	/*vty_out(vty, "dns ip is %x, %x ,%x \n", ipAddr[0], ipAddr[1], ipAddr[2]);*/
	/*mode 0 means global mode, 1 means pool mode */
	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;
	}
	else if((POOL_NODE == vty->node) || (HANSI_POOL_NODE == vty->node) || (LOCAL_HANSI_POOL_NODE  == vty->node)) {	
		mode = 1;		
		index = (unsigned int)(vty->index_sub);
		/*vty_out(vty,"ip pool server domain name sub index is %d\n", index);*/
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_dns(vty,ipAddr, ipnum, mode, index, 1);
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

unsigned int
dcli_set_server_routers_ip
(
	struct vty *vty,
	unsigned int routers,	
	unsigned int mode,
	unsigned int index,
	unsigned int del
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_SERVER_ROUTERS_IP);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &routers, 
							 DBUS_TYPE_UINT32, &mode,							 
							 DBUS_TYPE_UINT32, &index,
							 DBUS_TYPE_UINT32, &del,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}else if(DHCP_SET_ORDER_WANNING == op_ret){
			vty_out (vty, "first set ip range , Have not configured address pool range!\n");
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}else if(DHCP_NOT_FOUND_POOL  == op_ret) {
			vty_out (vty, "Not found the pool\n");
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}
}

DEFUN(ip_dhcp_server_routers_ip_cmd_func,
	ip_dhcp_server_routers_ip_cmd,
	"ip dhcp server routers A.B.C.D",
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"Ip dhcp server routers entity\n"
	"Ipv4 address\n"	 
)
{
	unsigned int ipAddr = 0, index = 0;
	unsigned int ret = 0, op_ret = 0, mode = 0;

	if((ret = inet_pton (AF_INET, (char*)argv[0], &ipAddr)) != 1){			
		vty_out(vty, "input IPV4 address %s is Illegal\n", argv[0]);
		return CMD_WARNING;
	}
	if (0 == (ipAddr & 0xff000000)) {
		vty_out(vty, "invalid ip address %s(0.X.X.X)\n", argv[0]);
		return CMD_WARNING;
	}
	if (dcli_dhcp_check_ip_address((char *)argv[0], NULL)) {
		vty_out(vty, "invalid ip address: %s\n", (char *)argv[0]);
		return CMD_WARNING;
	}

	ipAddr = dcli_ip2ulong((char*)argv[0]);

	/*mode 0 means global mode, 1 means pool mode */
	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;
	}
	else if((POOL_NODE == vty->node) || (HANSI_POOL_NODE == vty->node) || (LOCAL_HANSI_POOL_NODE  == vty->node)) {
		mode = 1;		
		index = (unsigned int)(vty->index_sub);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_routers_ip(vty,ipAddr, mode, index, 0);
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

DEFUN(no_ip_dhcp_server_routers_ip_cmd_func,
	no_ip_dhcp_server_routers_ip_cmd,
	"no ip dhcp server routers",
	"Delete old Configuration\n"
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"No ip dhcp server routers entity\n"
)
{
	unsigned int ipAddr = 0, index = 0;
	unsigned int ret = 0, op_ret = 0, mode = 0;

	/*mode 0 means global mode, 1 means pool mode */

	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;
	}
	else if((POOL_NODE == vty->node) || (HANSI_POOL_NODE == vty->node) || (LOCAL_HANSI_POOL_NODE  == vty->node)) {
		mode = 1;		
		index = (unsigned int)(vty->index_sub);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_routers_ip(vty,ipAddr, mode, index, 1);
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

unsigned int
dcli_set_server_wins_ip
(
	struct vty *vty,
	unsigned int wins,	
	unsigned int mode,
	unsigned int index,
	unsigned int del
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_SERVER_WINS_IP);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &wins, 
							 DBUS_TYPE_UINT32, &mode,							 
							 DBUS_TYPE_UINT32, &index,
							 DBUS_TYPE_UINT32, &del,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}else if(DHCP_SET_ORDER_WANNING == op_ret){
			vty_out (vty, "first set ip range , Have not configured address pool range!\n");
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}else if(DHCP_NOT_FOUND_POOL  == op_ret) {
			vty_out (vty, "Not found the pool\n");
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}
}

DEFUN(ip_dhcp_server_wins_ip_cmd_func,
	ip_dhcp_server_wins_ip_cmd,
	"ip dhcp server wins A.B.C.D",
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"Ip dhcp server wins entity\n"
	"Ipv4 address\n"
)
{
	unsigned int ipAddr = 0, index = 0;
	unsigned int ret = 0, op_ret = 0, mode = 0;
	unsigned int ipaddr = 0;

	/* check ip address 0.X.X.X */
	if(1 == inet_pton(AF_INET, (char *)argv[0], &ipaddr)) {
		if (0 == (ipaddr & 0xff000000)) {
			vty_out(vty, "invalid ip address %s(0.X.X.X)\n", argv[0]);
			return CMD_WARNING;
		}
	} else {
		vty_out(vty, "invalid ip address %s\n", (char *)argv[0]);
		return CMD_WARNING;
	}
	if (dcli_dhcp_check_ip_address((char *)argv[0], NULL)) {
		vty_out(vty, "invalid ip address: %s\n", (char *)argv[0]);
		return CMD_WARNING;
	}
	ipAddr = dcli_ip2ulong((char*)argv[0]);

	/*mode 0 means global mode, 1 means pool mode */
	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;
	}
	else if((POOL_NODE == vty->node) || (HANSI_POOL_NODE == vty->node) || (LOCAL_HANSI_POOL_NODE  == vty->node)) {	
		mode = 1;		
		index = (unsigned int)(vty->index_sub);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_wins_ip(vty,ipAddr, mode, index, 0);
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

DEFUN(no_ip_dhcp_server_wins_ip_cmd_func,
	no_ip_dhcp_server_wins_ip_cmd,
	"no ip dhcp server wins",
	"Delete old Configuration\n"
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"No ip dhcp server wins\n"	
)
{
	unsigned int ipAddr = 0, index = 0;
	unsigned int ret = 0, op_ret = 0, mode = 0;

	/*mode 0 means global mode, 1 means pool mode */
	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;
	}
	else if((POOL_NODE == vty->node) || (HANSI_POOL_NODE == vty->node) || (LOCAL_HANSI_POOL_NODE  == vty->node)) {
		mode = 1;		
		index = (unsigned int)(vty->index_sub);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_wins_ip(vty,ipAddr, mode, index, 1);
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

unsigned int
dcli_set_server_lease_default
(
	struct vty *vty,
	unsigned int lease_default,	
	unsigned int mode,
	unsigned int index,
	unsigned int del
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_SERVER_LEASE_DEFAULT);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &lease_default, 
							 DBUS_TYPE_UINT32, &mode,							 
							 DBUS_TYPE_UINT32, &index,
							 DBUS_TYPE_UINT32, &del,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}else if(DHCP_SET_ORDER_WANNING == op_ret){
			vty_out (vty, "first set ip range , Have not configured address pool range!\n");
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}else if(DHCP_NOT_FOUND_POOL  == op_ret) {
			vty_out (vty, "Not found the pool\n");
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}
}

DEFUN(ip_dhcp_server_lease_default_cmd_func,
	ip_dhcp_server_lease_default_cmd,
	"ip dhcp server lease-time TIME",
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"	
	"Ip dhcp server lease-time\n"
	"Ip dhcp server lease-time 60-31536000 seconds\n"	
)
{
	unsigned int lease_default = 0, index = 0;
	unsigned int ret = 0, op_ret = 0, mode = 0;

	lease_default = atoi((char*)argv[0]);
	if((lease_default<60) || (lease_default>31536000)){
		vty_out (vty, "Please input lease-time <60-31536000> !!!\n");

		return CMD_WARNING;
	}

	/*mode 0 means global mode, 1 means pool mode */
	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;
	}
	else if((POOL_NODE == vty->node) || (HANSI_POOL_NODE == vty->node) || (LOCAL_HANSI_POOL_NODE  == vty->node)) {
		mode = 1;		
		index = (unsigned int)(vty->index_sub);
		/*vty_out(vty,"ip pool server domain name sub index is %d\n", index);*/
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_lease_default(vty,lease_default, mode, index, 0);
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

DEFUN(no_ip_dhcp_server_lease_default_cmd_func,
	no_ip_dhcp_server_lease_default_cmd,
	"ip dhcp server lease-default",
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"	
	"Set ip dhcp server lease-time default entity\n"
)
{
	unsigned int lease_default = 0, index = 0;
	unsigned int ret = 0, op_ret = 0, mode = 0;

	/*mode 0 means global mode, 1 means pool mode */
	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;
	}
	else if((POOL_NODE == vty->node) || (HANSI_POOL_NODE == vty->node) || (LOCAL_HANSI_POOL_NODE  == vty->node)) {
		mode = 1;		
		index = (unsigned int)(vty->index_sub);
		/*vty_out(vty,"ip pool server domain name sub index is %d\n", index);*/
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_lease_default(vty,lease_default, mode, index, 1);
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

unsigned int
dcli_set_server_lease_max
(
	struct vty *vty,
	unsigned int lease_max,	
	unsigned int mode,
	unsigned int index,
	unsigned int del
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_SERVER_LEASE_MAX);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &lease_max, 
							 DBUS_TYPE_UINT32, &mode,							 
							 DBUS_TYPE_UINT32, &index, 
						     DBUS_TYPE_UINT32, &del, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}else if(DHCP_SET_ORDER_WANNING == op_ret){
			vty_out (vty, "first set ip range , Have not configured address pool range!\n");
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}else if(DHCP_NOT_FOUND_POOL  == op_ret) {
			vty_out (vty, "Not found the pool\n");
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}
}

DEFUN(ip_dhcp_server_lease_max_cmd_func,
	ip_dhcp_server_lease_max_cmd,
	"ip dhcp server lease-max TIME",
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"Ip dhcp server lease-max entity\n"	
)
{
	unsigned int lease_max = 0, index = 0;
	unsigned int ret = 0, op_ret = 0, mode = 0;

	lease_max = atoi((char*)argv[0]);
	
	/*mode 0 means global mode, 1 means pool mode */
	if(CONFIG_NODE == vty->node) {
		mode = 0;
	}
	else if(POOL_NODE == vty->node){
		mode = 1;		
		index = (unsigned int)(vty->index_sub);
		/*vty_out(vty,"ip pool server domain name sub index is %d\n", index);*/
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_lease_max(vty,lease_max, mode, index, 0);
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

DEFUN(no_ip_dhcp_server_lease_max_cmd_func,
	no_ip_dhcp_server_lease_max_cmd,
	"no ip dhcp server lease-max",
	"Delete old Configuration\n"
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"Delete server lease max time\n"	
)
{
	unsigned int lease_max = 0, index = 0;
	unsigned int ret = 0, op_ret = 0, mode = 0;

	/*mode 0 means global mode, 1 means pool mode */
	if(CONFIG_NODE == vty->node) {
		mode = 0;
	}
	else if(POOL_NODE == vty->node){
		mode = 1;		
		index = (unsigned int)(vty->index_sub);
		/*vty_out(vty,"ip pool server domain name sub index is %d\n", index);*/
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_lease_max(vty,lease_max, mode, index, 1);
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

unsigned int
dcli_set_server_ASN_enable
(
	struct vty *vty,
	unsigned int enable
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_ASN_ENABLE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &enable, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}
}

unsigned int
dcli_set_server_nak_rsp_enable
(
	struct vty *vty,
	unsigned int enable
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_NAK_RSP_ENABLE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &enable, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}
}

unsigned int
dcli_set_server_enable
(
	struct vty *vty,
	unsigned int enable
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_SERVER_ENABLE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &enable, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}
}

DEFUN(ip_dhcp_server_enable_cmd_func,
	ip_dhcp_server_enable_cmd,
	"ip dhcp server (enable|disable)",
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"Ip dhcp server enable\n"
	"Ip dhcp server disable\n"
)
{
	unsigned int ret = 0, op_ret = 0, isEnable = 0;

	if(strncmp("enable",argv[0],strlen(argv[0]))==0) {
		isEnable = 1;
	}
	else if (strncmp("disable",argv[0],strlen(argv[0]))==0) {
		isEnable = 0;
	}
	else {
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}

	ret = dcli_set_server_enable(vty, isEnable);
	if (!ret) {
		
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}


unsigned int
dcli_set_dynamic_arp_switch
(
	struct vty *vty,
	unsigned int enable
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_DYNAMIC_ARP_ENABLE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &enable, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}
}



DEFUN(ip_dhcp_dynamic_arp_cmd_func,
	ip_dhcp_dynamic_arp_enable_cmd,
	"ip dhcp dynamic arp (enable|disable)",
	"ip dhcp dynamic arp\n"
	"set ip dhcp dynamic arp switch\n"
	"set ip dhcp dynamic arp switch\n"
	"ip dhcp dynamic arp enable\n"
	"ip dhcp dynamic arp disable\n"
)
{
	unsigned int ret = 0, op_ret = 0, isEnable = 0;

	if(strncmp("enable",argv[0],strlen(argv[0]))==0) {
		isEnable = 1;
	}
	else if (strncmp("disable",argv[0],strlen(argv[0]))==0) {
		isEnable = 0;
	}
	else {
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}

	ret = dcli_set_dynamic_arp_switch(vty, isEnable);
	if (!ret) {
		
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}



DEFUN(ip_dhcp_server_ASN_cmd_func,
	ip_dhcp_server_asn_cmd,
	"ip dhcp server asn (enable|disable)",
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"Ip dhcp ASN enable\n"
	"Ip dhcp ASN disable\n"
)
{
	unsigned int ret = 0, op_ret = 0, isEnable = 0;

	if(strncmp("enable",argv[0],strlen(argv[0]))==0) {
		isEnable = 1;
	}
	else if (strncmp("disable",argv[0],strlen(argv[0]))==0) {
		isEnable = 0;
	}
	else {
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}

	ret = dcli_set_server_ASN_enable(vty, isEnable);
	if (!ret) {
		
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

DEFUN(ip_dhcp_server_nak_rsp_cmd_func,
	ip_dhcp_server_nak_rsp_cmd,
	"ip dhcp server nak_rsp (enable|disable)",
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"Ip dhcp nak_rsp enable\n"
	"Ip dhcp nak_rsp disable\n"
)
{
	unsigned int ret = 0, op_ret = 0, isEnable = 0;

	if(strncmp("enable",argv[0],strlen(argv[0]))==0) {
		isEnable = 1;
	}
	else if (strncmp("disable",argv[0],strlen(argv[0]))==0) {
		isEnable = 0;
	}
	else {
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}

	ret = dcli_set_server_nak_rsp_enable(vty, isEnable);
	if (!ret) {
		
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

unsigned int
dcli_set_server_static_arp_enable
(
	struct vty *vty,
	unsigned int enable
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_SERVER_STATIC_ENABLE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &enable, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}
}

DEFUN(ip_dhcp_server_static_arp_enable_cmd_func,
	ip_dhcp_server_static_arp_enable_cmd,
	"ip dhcp server static-arp (enable|disable)",
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"Ip dhcp server static-arp enable entity\n"	
	"Ip dhcp server static-arp disable entity\n"	
)
{
	unsigned int ret = 0, op_ret = 0, isEnable = 0;

	if(strncmp("enable",argv[0],strlen(argv[0]))==0) {
		isEnable = 1;
	}
	else if (strncmp("disable",argv[0],strlen(argv[0]))==0) {
		isEnable = 0;
	}
	else {
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}

	ret = dcli_set_server_static_arp_enable(vty, isEnable);
	
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

unsigned int
dcli_set_failover
(
	struct vty *vty,
	unsigned int mode,
	unsigned int index,
	struct dhcp_failover_show* failover_conf
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError 		err;
	unsigned int op_ret = 0;
	char *failname = NULL;

	failname = malloc(strlen(failover_conf->name) + 1);
	memset(failname, 0, (strlen(failover_conf->name) + 1));
	memcpy(failname, failover_conf->name, strlen(failover_conf->name));

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_ADD_DHCP_FAILOVER_PEER);
	dbus_error_init(&err);
	dbus_message_append_args(query,		
							DBUS_TYPE_UINT32, &mode,
							DBUS_TYPE_UINT32, &index,
							DBUS_TYPE_STRING, &(failname),
							DBUS_TYPE_UINT32, &(failover_conf->primary),
							DBUS_TYPE_UINT32, &(failover_conf->split),
							DBUS_TYPE_UINT32, &(failover_conf->mclt),
							DBUS_TYPE_UINT32, &(failover_conf->dstip),
							DBUS_TYPE_UINT32, &(failover_conf->dstport),
							DBUS_TYPE_UINT32, &(failover_conf->srcip),
							DBUS_TYPE_UINT32, &(failover_conf->srcport),
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		if(failname){
			free(failname);
			failname=NULL;
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID)) {
		if (!op_ret ) {
		}else if(DHCP_FAILOVER_HAVE_EXIST == op_ret){
			vty_out (vty, "failover have exist !\n");			
		}else if(DHCP_NOT_FOUND_POOL  == op_ret) {
			vty_out (vty, "Not found the pool\n");			
		}			
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}

DEFUN(add_dhcp_failover_pool_cmd_func,
	  add_dhcp_failover_pool_cmd,
	  "failover FAILOVERNAME (primary|secondary) sqlite <0-255> mclt MCLT dip A.B.C.D dst-port <0-65535> sip A.B.C.D src-port <0-65535>",
	  "Configure pool failover"
	  "Failover name\n"
	  "standard rule index range in 1-1000\n"
	  "acl rule action redirect\n"
	  "acl rule deal with TCP packet\n"
	  "acl rule deal with UDP packet\n"
      "Destination IP address for TCP OR UDP packet\n"
	  "Specify IP address in <A.B.C.D/M> format\n"
	  "Any legal destination IP address\n"
	  "Destination port with TCP OR UDP packet\n"
	  "Specify Port number in range 0-65535\n"
	  "Any legal destination port number\n"
      "Source IP address for TCP OR UDP packet\n"
	  "Specify IP address in <A.B.C.D/M> format\n"
	  "Any legal source IP address\n"
	  "Source port with TCP OR UDP packet\n"
	  "Specify Port number in range 0-65535\n"
	  "Any legal source port number\n"	
)
{
	unsigned int nameSize = 0, mode = 0, index = 0, op_ret = 0, ret = 0;
	struct dhcp_failover_show conf;

	if ((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {		
		mode = 0;		
		vty_out(vty, "configure failover must under pool mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	else if((POOL_NODE == vty->node) || (HANSI_POOL_NODE == vty->node) || (LOCAL_HANSI_POOL_NODE  == vty->node)) {
		mode = 1;		
		index = (unsigned int)(vty->index_sub);
	}
	
	memset(&conf, 0, sizeof(struct dhcp_failover_show));
	
	nameSize = strlen(argv[0]);
	if (nameSize >= ALIAS_NAME_SIZE) {
		vty_out(vty, "failover name is too long!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	memcpy(conf.name, argv[0], nameSize);

	if (strncmp("primary", argv[1], strlen(argv[1]))) {
		conf.primary = 1;
	}
	
	conf.split = atoi((char*)argv[2]);
				
	conf.mclt = atoi((char*)argv[3]);
	
	conf.dstip = dcli_ip2ulong((char*)argv[4]);

	conf.dstport = atoi((char*)argv[5]);
	
	conf.srcip = dcli_ip2ulong((char*)argv[6]);

	conf.srcport = atoi((char*)argv[7]);
	
	ret = dcli_set_failover(vty,mode, index, &conf);	
	
	return CMD_SUCCESS;
	
}

unsigned int
dcli_config_failover_pool
(
	struct vty *vty,
	unsigned int mode,
	unsigned int index,
	struct dhcp_failover_show* failover_conf
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError 		err;
	unsigned int op_ret = 0;
	char *failname = NULL;

	failname = malloc(strlen(failover_conf->name) + 1);
	memset(failname, 0, (strlen(failover_conf->name) + 1));
	memcpy(failname, failover_conf->name, strlen(failover_conf->name));
	
	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_CFG_DHCP_FAILOVER_PEER);
	dbus_error_init(&err);
	dbus_message_append_args(query,		
							DBUS_TYPE_UINT32, &mode,
							DBUS_TYPE_UINT32, &index,
							DBUS_TYPE_STRING, &(failname),
							DBUS_TYPE_UINT32, &(failover_conf->primary),
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		if(failname){
			free(failname);
			failname=NULL;
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID)) {
		if (!op_ret ) {
		}else if(DHCP_SAME_PRIMARY == op_ret){
			vty_out (vty, "failover state change error same primary\n");			
		}else if(DHCP_NOT_FOUND_POOL  == op_ret) {
			vty_out (vty, "Not found the pool\n");			
		}else if(DHCP_FAILOVER_NOT_ENABLE== op_ret){
			vty_out (vty, "failover state change error not enable\n");
		}			
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}

DEFUN(cfg_dhcp_failover_pool_cmd_func,
	  cfg_dhcp_failover_pool_cmd,
	  "config failover FAILOVERNAME (primary|secondary)",
	  "Configure pool failover"
	  "Failover name\n"
	  "Standard rule index range in 1-1000\n"
	  "Acl rule action redirect\n"
)
{
	unsigned int nameSize = 0, mode = 0, index = 0, op_ret = 0, ret = 0;
	struct dhcp_failover_show conf;

	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;		
		vty_out(vty, "configure failover must under pool mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	else if((POOL_NODE == vty->node) || (HANSI_POOL_NODE == vty->node) || (LOCAL_HANSI_POOL_NODE  == vty->node)) {		
		mode = 1;		
		index = (unsigned int)(vty->index_sub);
	}
	
	memset(&conf, 0, sizeof(struct dhcp_failover_show));
	
	nameSize = strlen(argv[0]);
	if (nameSize >= ALIAS_NAME_SIZE) {
		vty_out(vty, "failover name is too long!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	memcpy(conf.name, argv[0], nameSize);

	if (strncmp("primary", argv[1], strlen(argv[1]))) {
		conf.primary = 1;
	}
	
	ret = dcli_config_failover_pool(vty,mode, index, &conf);	

	return CMD_SUCCESS;
	
}

unsigned int
dcli_delete_failover_pool
(
	struct vty *vty,
	unsigned int mode,
	unsigned int index,
	struct dhcp_failover_show* failover_conf
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError 		err;
	unsigned int op_ret = 0, ret = 0;
	char *failname = NULL;
	int profile = 0,profile_sub = 0;
	char *endptr = NULL,*tmp = NULL;

	failname = malloc(strlen(failover_conf->name) + 1);
	memset(failname, 0, (strlen(failover_conf->name) + 1));
	memcpy(failname, failover_conf->name, strlen(failover_conf->name));
	if(strncmp(failname,"peer",4)){
		vty_out (vty, "FAILOVERNAME wrong!example: peer5\n");
		return CMD_FAILURE;
	}
	profile = strtoul(failname+4,&endptr,10);
	//printf("profile %d\n",profile);
	if(profile < 1 || profile > 16){
		vty_out (vty, "profile id wrong!\n");
		return CMD_FAILURE;
	}
	struct dhcp_failover_state failover_state;
	memset(&failover_state, 0, sizeof(failover_state));
	
	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);
	ret = dcli_show_dhcp_failover(vty, profile, &failover_state, slot_id);
	if (DCLI_VRRP_RETURN_CODE_OK != ret){
		vty_out(vty, "cannot get failobver state\n");
	}
	if(!strncmp(endptr,".",1)){
	tmp = endptr+1;
	profile_sub = strtoul(tmp,&endptr,10);
	//printf("profile_sub %d,failname(1) %s\n",profile_sub,failname);
	memset(failname, 0, (strlen(failover_conf->name) + 1));
	sprintf(failname,"peer%d",profile);
	//printf("failname(2) %s\n",failname);
	}
	if(((failover_state.peer != 0) || (failover_state.local != 6)) && (DCLI_VRRP_RETURN_CODE_OK == ret)){
		vty_out(vty, "Only local [recover] peer [unknown] for this CMD");
		if(profile_sub != 120)
		return CMD_FAILURE;
	}
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_DEL_DHCP_FAILOVER_PEER);
	dbus_error_init(&err);
	dbus_message_append_args(query,		
							DBUS_TYPE_UINT32, &mode,
							DBUS_TYPE_UINT32, &index,
							DBUS_TYPE_STRING, &(failname),
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		if(failname){
			free(failname);
			failname=NULL;
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_INVALID)) {
		if (!op_ret ) {
			
		}else if(DHCP_FAILOVER_NOT_ENABLE == op_ret){
			vty_out (vty, "failover state change error not enable\n");
		}else if(DHCP_NOT_FOUND_POOL  == op_ret) {
			vty_out (vty, "Not found the pool\n");			
		}else if(DHCP_FAILOVER_NAME_WRONG == op_ret){
			vty_out (vty, "FAILOVERNAME wrong!");
		}
		else {
			vty_out (vty, "delete operation fail\n");
		}			
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}

DEFUN(del_dhcp_failover_pool_cmd_func,
	  del_dhcp_failover_pool_cmd,
	  "no failover FAILOVERNAME",
	  "Delete pool failover"
	  "Failover name\n"
	  "Standard rule index range in 1-1000\n"
	  "Acl rule action redirect\n"
)
{
	unsigned int nameSize = 0, mode = 0, index = 0, op_ret = 0, ret = 0;
	struct dhcp_failover_show conf;


	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;		
		vty_out(vty, "configure failover must under pool mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	else if((POOL_NODE == vty->node) || (HANSI_POOL_NODE == vty->node) || (LOCAL_HANSI_POOL_NODE  == vty->node)) {
		mode = 1;		
		index = (unsigned int)(vty->index_sub);
	}
	
	memset(&conf, 0, sizeof(struct dhcp_failover_show));
	
	nameSize = strlen(argv[0]);
	if (nameSize >= ALIAS_NAME_SIZE) {
		vty_out(vty, "failover name is too long!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	memcpy(conf.name, argv[0], nameSize);
	
	ret = dcli_delete_failover_pool(vty,mode, index, &conf);	
	return CMD_SUCCESS;
	
}

unsigned int
dcli_show_failover_configure
(
	struct vty *vty,
	unsigned int mode,
	unsigned int index,
	struct dhcp_failover_show *failover_cfg
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned int op_ret = 0,ret = 0,i = 0,j = 0;
	char* failname = NULL;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SHOW_FAILOVER_CFG);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &mode,
							 DBUS_TYPE_UINT32, &index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_UINT32, &(failover_cfg->primary),
		DBUS_TYPE_UINT32, &(failover_cfg->split),
		DBUS_TYPE_UINT32, &(failover_cfg->mclt),
		DBUS_TYPE_UINT32, &(failover_cfg->dstip),
		DBUS_TYPE_UINT32, &(failover_cfg->dstport),
		DBUS_TYPE_UINT32, &(failover_cfg->srcip),
		DBUS_TYPE_UINT32, &(failover_cfg->srcport),
		DBUS_TYPE_STRING, &(failname),
		DBUS_TYPE_INVALID)) {
		if (!op_ret ) {
			strcpy(failover_cfg->name, failname);
		}else if(DHCP_POOL_HAVE_NOT_FAILOVER == op_ret){
			vty_out (vty, "this pool has no failover here\n");
		}else if(DHCP_NOT_THE_SUBNET  == op_ret) {
			vty_out (vty, "failover state change error pool has no subnet\n");			
		}else {
			vty_out (vty, "operation fail\n");
		}			
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);


	return CMD_SUCCESS;

}

DEFUN(show_failover_configure_cmd_func,
	show_failover_configure_cmd,
	"show ip pool failover",
	"Show ip pool failover\n"
	"Ipv4 address of portal\n"
	"Dhcp ip pool\n"
	"Show ip dhcp static information\n"
)
{
	struct dhcp_failover_show failover_cfg;
	unsigned int i = 0, count = 0, mode = 0, index = 0;
	
	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {		
		mode = 0;		
		vty_out(vty, "configure failover must under pool mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	else if((POOL_NODE == vty->node) || (HANSI_POOL_NODE == vty->node) || (LOCAL_HANSI_POOL_NODE  == vty->node)) {	
		mode = 1;		
		index = (unsigned int)(vty->index_sub);
	}
	memset(&failover_cfg, 0, sizeof(struct dhcp_failover_show));
	dcli_show_failover_configure(vty,mode, index, &failover_cfg);
	
	vty_out(vty, "========================================\n");
	vty_out(vty, "          failover configure\n");
	vty_out(vty, "failover peer name is : %s \n", failover_cfg.name);
	vty_out(vty, "state : %s \n", failover_cfg.primary ? "secondary" : "primary");	
	vty_out(vty, "split is : %u \n", failover_cfg.split);
	vty_out(vty, "mclt is : %u \n", failover_cfg.mclt);
	vty_out(vty, "dstip is : %u.%u.%u.%u \n",(((failover_cfg.dstip) & 0xff000000) >> 24),(((failover_cfg.dstip) & 0xff0000) >> 16),	\
			(((failover_cfg.dstip) & 0xff00) >> 8),((failover_cfg.dstip) & 0xff));
	vty_out(vty, "dstport is : %u \n", failover_cfg.dstport);
	vty_out(vty, "srcip is : %u.%u.%u.%u \n",(((failover_cfg.srcip) & 0xff000000) >> 24),(((failover_cfg.srcip) & 0xff0000) >> 16),	\
			(((failover_cfg.srcip) & 0xff00) >> 8),((failover_cfg.srcip) & 0xff));
	vty_out(vty, "srcport is : %u \n", failover_cfg.srcport);
	vty_out(vty, "========================================\n");
	return CMD_SUCCESS;
}


unsigned int
dcli_set_relay_enable
(
	struct vty *vty,
    unsigned int enable
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCRELAY_DBUS_BUSNAME, 
									DHCRELAY_DBUS_OBJPATH, 
									DHCRELAY_DBUS_INTERFACE, 
									DHCRELAY_DBUS_METHOD_SET_RELAY_ENABLE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &enable, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}
}

DEFUN(ip_dhcp_relay_enable_cmd_func,
	ip_dhcp_relay_enable_cmd,
	"ip dhcp relay (enable|disable)",
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp relay Function\n"
	"Ip dhcp relay enable\n"
	"Ip dhcp relay disable\n"
	
)
{
	unsigned int ret = 0, op_ret = 0, isEnable = 0;

	if(strncmp("enable",argv[0],strlen(argv[0]))==0) {
		isEnable = 1;
	}
	else if (strncmp("disable",argv[0],strlen(argv[0]))==0) {
		isEnable = 0;
	}
	else {
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}

	ret = dcli_set_relay_enable(vty, isEnable);
	
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

/*0 del 1 add*/
unsigned int
dcli_set_interface_ip_relay
(
	struct vty *vty,
	char *upifname,
	char *downifname,
	unsigned int ipaddr,
	unsigned int add_info
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0, ret = 0;

	ret = dcli_dhcp_check_server_interface_iSbusy(vty, upifname, downifname);
	if (!ret) {
		vty_out(vty, "interface is busy\n");
		return 1;
	}

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
	
	query = dbus_message_new_method_call(DHCRELAY_DBUS_BUSNAME, 
									DHCRELAY_DBUS_OBJPATH, 
									DHCRELAY_DBUS_INTERFACE, 
									DHCRELAY_DBUS_METHOD_SET_DHCP_RELAY);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &downifname,
							DBUS_TYPE_STRING, &upifname,
							DBUS_TYPE_UINT32, &ipaddr,
							DBUS_TYPE_UINT32, &add_info,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		dbus_message_unref(reply);
		
		return op_ret;
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}
}
int
dcli_dhcp_check_relay_interface
(
	char *upifname
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	char *name=NULL;
	unsigned int op_ret = 0;
	unsigned int localid = 1, slot_id = HostSlotId, indextmp = 0;
	//get_slotid_index(vty, &indextmp, &slot_id, &localid);
	slot_id = get_slot_id_by_ifname(upifname);
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
	query = dbus_message_new_method_call(DHCRELAY_DBUS_BUSNAME, 
									DHCRELAY_DBUS_OBJPATH, 
									DHCRELAY_DBUS_INTERFACE, 
									DHCRELAY_DBUS_METHOD_CHECK_RELAY_INTERFACE_VE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &upifname,				 
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
		DBUS_TYPE_STRING, &name,
		DBUS_TYPE_INVALID)) {
		if(op_ret){
		}
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	memset(upifname, 0, ALIAS_NAME_SIZE);
	memcpy(upifname, name, strlen(name));
	return 0;
}

DEFUN(add_interface_ip_relay_cmd_func,
	add_interface_ip_relay_cmd,
	"ip relay IFNAME A.B.C.D",
	"Ipv4 address of portal\n"
	"Config ip relay entity\n"
	"Bind interface for sending packet to interface\n"
	"Dhcp server ip\n"
)
{

	char* upifname = NULL, *downifname = NULL;;
	unsigned int nameSize = 0, nodeSave = 0, ipaddr = 0;
	unsigned int ret = 0, index = 0, op_ret = 0;

	ret = inet_pton (AF_INET, (char*)argv[1], &ipaddr);
	if (1 == ret) {
		if (0 == (ipaddr & 0xff000000)) {
			vty_out (vty, "invalid ip address %s(0.X.X.X)\n", argv[1]);
			return CMD_WARNING;
		}
	} else {
		vty_out (vty, "invalid ip address %s\n", argv[1]);
		return CMD_WARNING;
	}
	if (dcli_dhcp_check_ip_address((char *)argv[1], NULL)) {
		vty_out(vty, "invalid ip address: %s\n", (char *)argv[1]);
		return CMD_WARNING;
	}
	
	upifname = (char*)malloc(ALIAS_NAME_SIZE);
	downifname = (char*)malloc(ALIAS_NAME_SIZE);
	memset(upifname, 0, ALIAS_NAME_SIZE);
	memset(downifname, 0, ALIAS_NAME_SIZE);
	
	nameSize = strlen(argv[0]);
	if (nameSize >= ALIAS_NAME_SIZE) {
		vty_out(vty, "interface name is too long\n");
		free(upifname);
		free(downifname);
		upifname = NULL;	
		return CMD_WARNING;
	}		
	memcpy(upifname, argv[0], nameSize);
	
	nameSize = strlen(vlan_eth_port_ifname);
	memcpy(downifname, vlan_eth_port_ifname, nameSize);
	if(0 == strncmp(upifname,"ve",2))
    dcli_dhcp_check_relay_interface(upifname);	
	if (!strcmp(upifname, downifname)) {
		vty_out(vty, "interface add relay fail \n");	
		if(upifname){
			free(upifname);
			upifname=NULL;
		}
		if(downifname){
			free(downifname);
			downifname=NULL;
		}
		return CMD_WARNING;
	}
	
	ret = dcli_set_interface_ip_relay(vty, upifname, downifname, ipaddr, 1);
	
	if (!ret) {
		if (vty->slotindex != get_slot_id_by_ifname(downifname)) {
			dcli_dhcp_distributed_process(vty, INTERFACE_BIND_POOL, downifname);
		}
		if (vty->slotindex != get_slot_id_by_ifname(upifname)) {
			dcli_dhcp_distributed_process(vty, INTERFACE_BIND_POOL, upifname);
		}
		free(upifname);
		free(downifname);
		upifname = NULL;
		return CMD_SUCCESS;
	}
	else {
		if(DHCP_INTERFACE_IN_USR == ret){
			vty_out(vty, "the relay down interface is in used \n");	
		}else{
			vty_out(vty, "interface add relay fail\n");
		}
		free(upifname);
		free(downifname);
		upifname = NULL;
		return CMD_WARNING;
	}
}


DEFUN(del_interface_ip_relay_cmd_func,
	del_interface_ip_relay_cmd,
	"no ip relay IFNAME A.B.C.D",	
	"Delete ip relay entity\n"
	"Ipv4 address of portal\n"
	"Dhcp relay\n"
	"Bind interface for sending packet to interface\n"
	"Dhcp server ip\n"
)
{

	char* upifname = NULL, *downifname = NULL;;
	unsigned int nameSize = 0, nodeSave = 0, ipaddr = 0;
	unsigned int ret = 0, index = 0, op_ret = 0;

	upifname = (char*)malloc(ALIAS_NAME_SIZE);
	downifname = (char*)malloc(ALIAS_NAME_SIZE);
	memset(upifname, 0, ALIAS_NAME_SIZE);
	memset(downifname, 0, ALIAS_NAME_SIZE);
	
	nameSize = strlen(argv[0]);
	if (nameSize >= ALIAS_NAME_SIZE) {
		vty_out(vty, "interface name is too long\n");
		free(upifname);
		free(downifname);
		upifname = NULL;	
		return CMD_WARNING;
	}
	memcpy(upifname, argv[0], nameSize);
	if(0 == strncmp(upifname,"ve",2))
	dcli_dhcp_check_relay_interface(upifname);	
	
	ret = inet_pton (AF_INET, (char*)argv[1], &ipaddr);
	if (ret != 1) {
		vty_out (vty, "malformed ip address : %s\n", argv[1]);
		if(upifname){
			free(upifname);
			upifname = NULL;	
		}
		if(downifname){
			free(downifname);
			downifname = NULL;	
		}
		return CMD_SUCCESS;
	}
	
	nameSize = strlen(vlan_eth_port_ifname);
	memcpy(downifname, vlan_eth_port_ifname, nameSize);

	ret = dcli_set_interface_ip_relay(vty, upifname, downifname, ipaddr, 0);
	
	if (!ret) {

		if (vty->slotindex != get_slot_id_by_ifname(downifname)) {
			dcli_dhcp_distributed_process(vty, INTERFACE_UNBIND_POOL, downifname);
		}
		if (vty->slotindex != get_slot_id_by_ifname(upifname)) {
			dcli_dhcp_distributed_process(vty, INTERFACE_UNBIND_POOL, upifname);		
		}
		free(upifname);
		free(downifname);
		upifname = NULL;
		return CMD_SUCCESS;
	}
	else {	
		vty_out(vty, "interface delete relay fail\n");		
		free(upifname);
		free(downifname);
		upifname = NULL;
		return CMD_WARNING;
	}
}

unsigned int
dcli_show_dhcp_relay
(
	struct vty *vty,
	struct dhcp_relay_show **relay_node,
	unsigned int *node_num
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned int op_ret = 0,ret = 0,i = 0,j = 0;
	unsigned int ipaddr = 0, count = 0, value = 0, isenable= 0;
	struct dhcp_relay_show *node = NULL;
	char *upifname = NULL, *downifname = NULL;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);	
	
	query = dbus_message_new_method_call(DHCRELAY_DBUS_BUSNAME, 
									DHCRELAY_DBUS_OBJPATH, 
									DHCRELAY_DBUS_INTERFACE, 
									DHCRELAY_DBUS_METHOD_SHOW_DHCP_RELAY);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&value,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&isenable);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&count);
	dbus_message_iter_next(&iter);	
	dhcp_relay_enable = isenable;
	
	if (!op_ret) {
		if(count > 0) {	
			node = malloc(sizeof(struct dhcp_relay_show) * count);
			memset(node, 0, (sizeof(struct dhcp_relay_show)*count));
			
			dbus_message_iter_recurse(&iter,&iter_array);
			
			for(i = 0; i < count; i++) {
				dbus_message_iter_recurse(&iter_array,&iter_struct);
			
				dbus_message_iter_get_basic(&iter_struct, &ipaddr);
				dbus_message_iter_next(&iter_struct);
				
	 			dbus_message_iter_get_basic(&iter_struct, &upifname);
				dbus_message_iter_next(&iter_struct);

			 	dbus_message_iter_get_basic(&iter_struct, &downifname);
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_next(&iter_array);

				node[i].ipaddr = ipaddr;
				strcpy(node[i].upifname, upifname);		
				strcpy(node[i].downifname, downifname);
			}
		}
	}
	
	*node_num = count;
	*relay_node = node;
	dbus_message_unref(reply);

	return CMD_SUCCESS;

}

DEFUN(show_ip_dhcp_relay_cmd_func,
	show_ip_dhcp_relay_cmd,
	"show ip dhcp relay",
	"Show ip dhcp relay\n"
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Show ip dhcp relay information\n"
)
{
	struct dhcp_relay_show *node = NULL;
	unsigned int i = 0, count = 0;

	dcli_show_dhcp_relay(vty, &node, &count);
	
	vty_out(vty, "========================================\n");
	vty_out(vty, "       ip dhcp relay is %s\n", dhcp_relay_enable ? "enable" : "disable");
	for (i = 0; i < count; i++) {
		vty_out(vty,"local interface name is : %s \n", node[i].downifname);
		vty_out(vty,"remote server interface name is : %s \n", node[i].upifname);
		vty_out(vty,"remote server ip : %-3d.%-3d.%-3d.%-3d \n",(((node[i].ipaddr) & 0xff000000) >> 24),(((node[i].ipaddr) & 0xff0000) >> 16),	\
			(((node[i].ipaddr) & 0xff00) >> 8),((node[i].ipaddr) & 0xff));
		
	}
	vty_out(vty, "========================================\n");
	return CMD_SUCCESS;
}


unsigned int
dcli_set_debug_relay_state
(  
	struct vty *vty,
    unsigned int debug_type,
    unsigned int debug_enable
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCRELAY_DBUS_BUSNAME, 
									DHCRELAY_DBUS_OBJPATH, 
									DHCRELAY_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_DEBUG_RELAY_STATE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &debug_type,
							 DBUS_TYPE_UINT32, &debug_enable,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}
}
DEFUN(relay_debug_enable_cmd_func,
	relay_debug_enable_cmd,
	"debug dhcrelay (all|info|error|debug)",
	"Ddd debug dhcp relay Information\n"
	"Dhcp relay function\n"	
	"Open dhcp debug level all\n"	
	"Open dhcp debug level info\n"
	"Open dhcp debug level error\n"
	"Open dhcp debug level debug\n"
)
{
	unsigned int ret = 0, debug_type = 0, debug_enable = 1;

	if(strncmp("all",argv[0],strlen(argv[0]))==0) {
		debug_type = DEBUG_TYPE_ALL;
	}
	else if (strncmp("info",argv[0],strlen(argv[0]))==0) {
		debug_type = DEBUG_TYPE_INFO;
	}
	else if (strncmp("error",argv[0],strlen(argv[0]))==0) {
		debug_type = DEBUG_TYPE_ERROR;
	}
	else if (strncmp("debug",argv[0],strlen(argv[0]))==0) {
		debug_type = DEBUG_TYPE_DEBUG;
	}
	else {
		vty_out(vty,"bad command parameter %s\n", argv[0]);
		return CMD_WARNING;
	}

	ret = dcli_set_debug_relay_state(vty, debug_type, debug_enable);
	
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {		
		return CMD_WARNING;
	}
}

DEFUN(relay_debug_disable_cmd_func,
	relay_debug_disable_cmd,
	"no debug dhcrelay (all|info|error|debug)",	
	"Config dhcp debugging close\n"
	"debug dhcp relay Information\n"
	"Dhcp relay function\n"	
	"Close dhcp debug level all\n"	
	"Close dhcp debug level info\n"
	"Close dhcp debug level error\n"
	"Close dhcp debug level debug\n"

)
{
	unsigned int ret = 0, debug_type = 0, isEnable = 0;

    if(strncmp("all",argv[0],strlen(argv[0]))==0) {
		debug_type = DEBUG_TYPE_ALL;
	}
	else if (strncmp("info",argv[0],strlen(argv[0]))==0) {
		debug_type = DEBUG_TYPE_INFO;
	}
	else if (strncmp("error",argv[0],strlen(argv[0]))==0) {
		debug_type = DEBUG_TYPE_ERROR;
	}
	else if (strncmp("debug",argv[0],strlen(argv[0]))==0) {
		debug_type = DEBUG_TYPE_DEBUG;
	}
	else {
		vty_out(vty,"bad command parameter %s\n", argv[0]);
		return CMD_WARNING;
	}

	ret = dcli_set_debug_relay_state(vty, debug_type, isEnable);
	
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}


int dcli_show_lease_state
(	
	struct vty *vty,
	struct lease_state  *total_state, 
	struct sub_lease_state *sub_state,
	unsigned int *subnet_num
)
{
	char *cursor = NULL;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
      int ret = 1;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned int sub_num = 0;
	int i = 0;
	
	if(!total_state || !sub_state || !subnet_num){
		return CMD_WARNING;
	}

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);	
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SHOW_LEASE_STATE);
	
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("DHCP show lease state failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return ret;
	}	

	dbus_message_iter_init(reply,&iter);
	
	dbus_message_iter_get_basic(&iter,&(total_state->total_lease_num));
	dbus_message_iter_next(&iter);	
	
	dbus_message_iter_get_basic(&iter,&(total_state->active_lease_num));
	dbus_message_iter_next(&iter);	

	dbus_message_iter_get_basic(&iter,&(total_state->free_lease_num));
	dbus_message_iter_next(&iter);	

	dbus_message_iter_get_basic(&iter,&(total_state->backup_lease_num));
	dbus_message_iter_next(&iter);		
	
	dbus_message_iter_get_basic(&iter,&sub_num);
	dbus_message_iter_next(&iter);		

	*subnet_num = sub_num;
	

	dbus_message_iter_recurse(&iter,&iter_array);

	/*get subnet lease state*/
	for (i = 0; i < *subnet_num; ++i) {

		dbus_message_iter_recurse(&iter_array,&iter_struct);
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet));
		dbus_message_iter_next(&iter_struct); 
	
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].mask));
		dbus_message_iter_next(&iter_struct); 
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet_lease_state.total_lease_num));
		dbus_message_iter_next(&iter_struct); 
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet_lease_state.active_lease_num));
		dbus_message_iter_next(&iter_struct); 
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet_lease_state.free_lease_num));
		dbus_message_iter_next(&iter_struct); 
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet_lease_state.backup_lease_num));
		dbus_message_iter_next(&iter_struct); 

		dbus_message_iter_next(&iter_array);
	} 

	dbus_message_unref(reply);

	return ret;	
}


DEFUN(show_dhcp_lease_state_cmd,
	show_dhcp_lease_state,
	"show dhcp-lease brief",	
	"Show config information\n"
	"DHCP lease\n"
	"Use lease state\n"	
)
{	
	int ret = 1, i = 0;
	unsigned int lease_count = 0;
	unsigned int lease_free = 0;
	struct lease_state  total_state;
	struct sub_lease_state sub_state[MAX_SUB_NET];
	unsigned int subnet_num = 0;
	int offset = 0;
	char buf[128];
	char *space8 = "        "; /* 8 space */

	memset(&total_state, 0, sizeof(struct lease_state));

	for(i = 0; i < MAX_SUB_NET; ++i){
		memset(sub_state, 0, sizeof(struct sub_lease_state));
	}

	ret = dcli_show_lease_state(vty, &total_state, sub_state, &subnet_num);	

	vty_out(vty, "============================================================================\n");
	vty_out(vty, "dhcp lease state brief\n");
	vty_out(vty, "total:%d active:%d free:%d backup:%d",
				total_state.total_lease_num, total_state.active_lease_num,
				total_state.free_lease_num, total_state.backup_lease_num);
	if (0 == total_state.total_lease_num) {
		vty_out(vty, " use percent :0 %%\n");
	} else {
		vty_out(vty, " use percent :%.2f %%\n",
			(float)total_state.active_lease_num / (float)total_state.total_lease_num * 100.0);
	}

	memset(buf, 0, sizeof(buf));

	sprintf(buf, "subnet%s%s%s", space8, space8, space8);
	sprintf(buf + 18, "mask%s%s%s", space8, space8, space8);
	sprintf(buf + 36, "total%s", space8);
	sprintf(buf + 44, "active%s", space8);
	sprintf(buf + 52, "free%s", space8);
	sprintf(buf + 60, "backup%s", space8);
	offset = sprintf(buf + 68, "use percent");
	
//	buf[offset + 68] = '\0';
	vty_out(vty, "\n%s\n", buf); 
	for(i = 0; i < subnet_num; ++i) {

		sprintf(buf, "%s%s%s%s", sub_state[i].subnet, space8, space8, space8);
		sprintf(buf + 18, "%s%s%s%s", sub_state[i].mask, space8, space8, space8);
		sprintf(buf + 36, "%d%s", sub_state[i].subnet_lease_state.total_lease_num, space8);
		sprintf(buf + 44, "%d%s", sub_state[i].subnet_lease_state.active_lease_num, space8);
		sprintf(buf + 52, "%d%s", sub_state[i].subnet_lease_state.free_lease_num, space8);
		sprintf(buf + 60, "%d%s", sub_state[i].subnet_lease_state.backup_lease_num, space8);

		if (0 == sub_state[i].subnet_lease_state.total_lease_num) {
			offset = sprintf(buf + 68, "0 %%");
		} else {
			offset = sprintf(buf + 68, "%.2f %%", (float)sub_state[i].subnet_lease_state.active_lease_num 
					/ (float)sub_state[i].subnet_lease_state.total_lease_num * 100.0);
		}
//		buf[buf + 68 + offset] = '\0';
		vty_out(vty, "%s\n", buf);
	}
	vty_out(vty, "===========================================================================\n");
	
	return ret;	
}


int dcli_show_lease_statistics
(	
	struct vty *vty,
	struct lease_state  *total_state, 
	struct sub_lease_state *sub_state,
	unsigned int *subnet_num
)
{
	char *cursor = NULL;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
      int ret = 1;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned int sub_num = 0;
	int i = 0;
	
	if(!total_state || !sub_state || !subnet_num){
		return CMD_WARNING;
	}

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);	
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SHOW_LEASE_STATISTICS);
	
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("DHCP show lease state failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return ret;
	}	

	dbus_message_iter_init(reply,&iter);
	
	dbus_message_iter_get_basic(&iter,&(total_state->total_lease_num));
	dbus_message_iter_next(&iter);	
	
	dbus_message_iter_get_basic(&iter,&(total_state->active_lease_num));
	dbus_message_iter_next(&iter);	

	dbus_message_iter_get_basic(&iter,&(total_state->free_lease_num));
	dbus_message_iter_next(&iter);	

	dbus_message_iter_get_basic(&iter,&(total_state->backup_lease_num));
	dbus_message_iter_next(&iter);		
	
	dbus_message_iter_get_basic(&iter,&sub_num);
	dbus_message_iter_next(&iter);		

	*subnet_num = sub_num;
	

	dbus_message_iter_recurse(&iter,&iter_array);

	/*get subnet lease state*/
	for (i = 0; i < *subnet_num; ++i) {

		dbus_message_iter_recurse(&iter_array,&iter_struct);
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet));
		dbus_message_iter_next(&iter_struct); 
	
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].mask));
		dbus_message_iter_next(&iter_struct); 

		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].poolname));
		dbus_message_iter_next(&iter_struct); 
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet_lease_state.total_lease_num));
		dbus_message_iter_next(&iter_struct); 
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet_lease_state.active_lease_num));
		dbus_message_iter_next(&iter_struct); 
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet_lease_state.free_lease_num));
		dbus_message_iter_next(&iter_struct); 
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet_lease_state.backup_lease_num));
		dbus_message_iter_next(&iter_struct); 

		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].info.discover_times));
		dbus_message_iter_next(&iter_struct); 
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].info.offer_times));
		dbus_message_iter_next(&iter_struct); 
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].info.requested_times));
		dbus_message_iter_next(&iter_struct); 
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].info.ack_times));
		dbus_message_iter_next(&iter_struct); 

		dbus_message_iter_next(&iter_array);
	} 

	dbus_message_unref(reply);

	return ret;	
}


DEFUN(show_dhcp_pool_statistics_cmd,
	show_dhcp_pool_statistics,
	"show dhcp-lease statistics",	
	"Show config information\n"
	"DHCP lease\n"
	"statistics information\n"	
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;

	int ret = 1, i = 0;
	unsigned int subnet_num = 0;
	float tmp = 0;
	int len = 0;
	char *cursor = NULL;
	char buf[256];
	struct lease_state  total_state;
	struct sub_lease_state sub_state[MAX_SUB_NET];

	memset(&total_state, 0, sizeof(struct lease_state));

	memset(sub_state, 0, sizeof(sub_state));

	

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);	
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SHOW_LEASE_STATISTICS);
	
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("DHCP show lease state failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return ret;
	}	

	dbus_message_iter_init(reply, &iter);
	
	dbus_message_iter_get_basic(&iter,&(total_state.total_lease_num));
	dbus_message_iter_next(&iter);	
	
	dbus_message_iter_get_basic(&iter,&(total_state.active_lease_num));
	dbus_message_iter_next(&iter);	

	dbus_message_iter_get_basic(&iter,&(total_state.free_lease_num));
	dbus_message_iter_next(&iter);	

	dbus_message_iter_get_basic(&iter,&(total_state.backup_lease_num));
	dbus_message_iter_next(&iter);		
	
	dbus_message_iter_get_basic(&iter,&subnet_num);
	dbus_message_iter_next(&iter);		

	dbus_message_iter_recurse(&iter,&iter_array);

	for (i = 0; i < subnet_num; ++i) {

		dbus_message_iter_recurse(&iter_array,&iter_struct);
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet));
		dbus_message_iter_next(&iter_struct); 
	
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].mask));
		dbus_message_iter_next(&iter_struct); 

		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].poolname));
		dbus_message_iter_next(&iter_struct); 

		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet_lease_state.total_lease_num));
		dbus_message_iter_next(&iter_struct); 
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet_lease_state.active_lease_num));
		dbus_message_iter_next(&iter_struct); 
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet_lease_state.free_lease_num));
		dbus_message_iter_next(&iter_struct); 
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].subnet_lease_state.backup_lease_num));
		dbus_message_iter_next(&iter_struct); 

		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].info.discover_times));
		dbus_message_iter_next(&iter_struct); 
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].info.offer_times));
		dbus_message_iter_next(&iter_struct); 
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].info.requested_times));
		dbus_message_iter_next(&iter_struct); 
		
		dbus_message_iter_get_basic(&iter_struct, &(sub_state[i].info.ack_times));
		dbus_message_iter_next(&iter_struct); 

		dbus_message_iter_next(&iter_array);
	} 

	dbus_message_unref(reply);
	vty_out(vty, "\n\nDISCOVER : total discover packet\n");
	vty_out(vty, "OFFER    : total offer packet\n");
	vty_out(vty, "REQUEST  : total request packet\n");
	vty_out(vty, "ACK      : total ack packet\n");

	vty_out(vty, "ACTIVE   : active lease\n");
	vty_out(vty, "FREE     : free lease\n");
	vty_out(vty, "BACKUP   : backup lease\n\n");
	
	vty_out(vty, "==============================================================================================\n");
	vty_out(vty, "TOTAL LEASE:%d ACTIVE LEASE:%d FREE LEASE:%d BACKUP LEASE:%d\n",
				total_state.total_lease_num, total_state.active_lease_num,
				total_state.free_lease_num, total_state.backup_lease_num);
#if 0
	if (0 == total_state.total_lease_num) {
		vty_out(vty, " Utilization-Ratio: 0 %%\n");
	} else { 
		vty_out(vty, " Utilization-Ratio: %.2f %%\n",
			(float)total_state.active_lease_num / (float)total_state.total_lease_num * 100.0);
	}
#endif
	cursor = buf;
	len = 0;
	memset(buf, 0, sizeof(buf));

    len += sprintf(cursor, "%-*s", strlen("255.255.255.255")+1, "SUBNET");
    cursor = buf + len;

    len += sprintf(cursor, "%-*s", strlen("255.255.255.255")+1, "NETMASK");
    cursor = buf + len;

    len += sprintf(cursor, "%-*s", strlen("DISCOVER")+1, "DISCOVER");
    cursor = buf + len;

    len += sprintf(cursor, "%-*s", strlen("OFFER")+1, "OFFER");
    cursor = buf + len;

    len += sprintf(cursor, "%-*s", strlen("REQUEST")+1, "REQUEST");
    cursor = buf + len;

    len += sprintf(cursor, "%-*s", strlen("ACK")+1, "ACK");
    cursor = buf + len;

	len += sprintf(cursor, "%-*s", strlen("TOTAL-LEASE")+1, "TOTAL-LEASE");
    cursor = buf + len;

    len += sprintf(cursor, "%-*s", strlen("ACTIVE")+1, "ACTIVE");
    cursor = buf + len;

    len += sprintf(cursor, "%-*s", strlen("FREE")+1, "FREE");
    cursor = buf + len;

    len += sprintf(cursor, "%-*s", strlen("BACKUP")+1, "BACKUP");
    cursor = buf + len;
#if 0
    len += sprintf(cursor, "%-*s", strlen("Utilization-Ratio")+1, "Utilization-Ratio");
    cursor = buf + len;
#endif
	vty_out(vty, "\n%s\n", buf); 
	for(i = 0; i < subnet_num; ++i) {
		cursor = buf;
		len = 0;
		memset(buf, 0, sizeof(buf));

	    len += sprintf(cursor, "%-*s", strlen("255.255.255.255")+1, sub_state[i].subnet);
	    cursor = buf + len;

	    len += sprintf(cursor, "%-*s", strlen("255.255.255.255")+1, sub_state[i].mask);
	    cursor = buf + len;

	    len += sprintf(cursor, "%-*d", strlen("DISCOVER")+1, sub_state[i].info.discover_times);
	    cursor = buf + len;

	    len += sprintf(cursor, "%-*d", strlen("OFFER")+1, sub_state[i].info.offer_times);
	    cursor = buf + len;

	    len += sprintf(cursor, "%-*d", strlen("REQUEST")+1, sub_state[i].info.requested_times);
	    cursor = buf + len;

	    len += sprintf(cursor, "%-*d", strlen("ACK")+1, sub_state[i].info.ack_times);
	    cursor = buf + len;


		len += sprintf(cursor, "%-*d", strlen("TOTAL-LEASE")+1, sub_state[i].subnet_lease_state.total_lease_num);
	    cursor = buf + len;

	    len += sprintf(cursor, "%-*d", strlen("ACTIVE")+1, sub_state[i].subnet_lease_state.active_lease_num);
	    cursor = buf + len;

	    len += sprintf(cursor, "%-*d", strlen("FREE")+1, sub_state[i].subnet_lease_state.free_lease_num);
	    cursor = buf + len;

	    len += sprintf(cursor, "%-*d", strlen("BACKUP")+1, sub_state[i].subnet_lease_state.backup_lease_num);
	    cursor = buf + len;
#if 0
		if (0 == sub_state[i].subnet_lease_state.total_lease_num) {
		    len += sprintf(cursor, "%-*s", strlen("Utilization-Ratio")+1, "0 %%");
		    cursor = buf + len;			
		} else {
		    len += sprintf(cursor, "%-*.2f", 
						strlen("Utilization-Ratio")+1, 
						(float)sub_state[i].subnet_lease_state.active_lease_num 
						/ (float)sub_state[i].subnet_lease_state.total_lease_num * 100.0);
		    cursor = buf + len;			
		}
#endif		
		vty_out(vty, "%s\n", buf);
	}
	vty_out(vty, "=============================================================================================\n");

	return ret;	
}



DEFUN(show_dhcp_running_config_cmd,
	show_dhcp_running_config,
	"show dhcp running config",	
	"Config dhcp debugging close\n"
	"Dynamic Host Configuration Protocol\n"
	"running config\n"	
	"dhcp Configuration\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	char *showStr = NULL;
	char *tmp = NULL;
	unsigned int len = 0;
	int ret = 1;

	int localid = 1, slot_id = HostSlotId, index = 0;
	get_slotid_index(vty, &index, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SHOW_RUNNING_CFG);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("DHCP show running failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return ret;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) {
		len = strlen(showStr);
		if (!(tmp = malloc(len + 1))) {
			return ret;

		}
		memset(tmp, 0, len + 1);
		memcpy(tmp, showStr, len);
		ret = 0;
	} else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	vty_out(vty, "===============================================================\n");
	if (tmp){
		vty_out(vty, "%s\n", tmp);
	}
	vty_out(vty, "===============================================================\n");


	if (tmp) {
		free(tmp);
	}
	
	dbus_message_unref(reply);
	
	return ret;	
}



int 
dcli_dhcp_show_running_cfg
(
	struct vty *vty
)
{	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
    int ret = 1;
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SHOW_RUNNING_CFG);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("DHCP show running failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return ret;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) {
	
		char _tmpstr[64];
		memset(_tmpstr, 0, 64);
		sprintf(_tmpstr, BUILDING_MOUDLE, "DHCP SERVER");
		vtysh_add_show_string(_tmpstr);
		vtysh_add_show_string(showStr);
        ret = 0;
	} 
	else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return ret;	
}


char * 
dcli_dhcp_show_running_cfg2
(
	int slot_id
)
{	
	char *showStr = NULL;
	DBusMessage *query, *reply;
	DBusError err;
	char * tmp = NULL;
    int ret = 1;

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);	

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SHOW_RUNNING_CFG);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("DHCP show running failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) {
	
		tmp = (char *)malloc(strlen(showStr)+1);
		memset(tmp, 0, strlen(showStr)+1);
		memcpy(tmp,showStr,strlen(showStr));
		dbus_message_unref(reply);		
		return tmp; 
	} 
	else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return NULL;	
}

/*zhanglei add for distribute hansi show running*/
char * 
dcli_dhcp_show_running_hansi_cfg
(
	unsigned int slot_id,unsigned int InstID, unsigned int islocaled 
)
{	
	char *showStr = NULL;
	char * tmp = NULL;
	DBusMessageIter	 iter;
	DBusMessage *query, *reply;
	DBusError err;
    int ret = 1;
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);	
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SHOW_RUNNING_HANSI_CFG);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&slot_id,
							 DBUS_TYPE_UINT32,&InstID,
							 DBUS_TYPE_UINT32,&islocaled,
							 DBUS_TYPE_INVALID);


	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("DHCP show running failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) {
	
		tmp = (char *)malloc(strlen(showStr)+1);
		memset(tmp, 0, strlen(showStr)+1);
		memcpy(tmp,showStr,strlen(showStr));			
//		dcli_config_write(showStr,islocaled,slot_id,InstID,1,0);
		dbus_message_unref(reply);
		return tmp; 
	} 
	else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return NULL;	
}


DEFUN(dhcp_set_reply_mode_cmd_func,
	 dhcp_set_reply_mode_cmd,
	  "ip dhcp reply-mode (unicast|default)",
	  "Config ip command\n"
	  "Config dhcp command\n"
	  "Config dhcp reply mode\n"
	  "Config dhcp reply unicast mode\n"
	  "Config dhcp reply broadcast mode\n"		 
)
{	
	unsigned int  unicast = 0;//unicast flag
	DBusMessage *query , *reply = NULL;
	DBusError err;
    int ret = 1;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_UNICAST_REPLY_MODE);
	dbus_error_init(&err);

	if(0 == strncmp("unicast", argv[0], strlen(argv[0])))
	{
		unicast = 1;
	}

	/*send unicast flag to dhcp_dbus*/
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&unicast,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return ret;
	}

	
	dbus_message_unref(reply);

	return ret;	
}

unsigned int
dcli_set_debug_state
(   
	struct vty *vty,
    unsigned int debug_type,
    unsigned int debug_enable
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_DEBUG_STATE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,	
							DBUS_TYPE_UINT32, &debug_type,
							 DBUS_TYPE_UINT32, &debug_enable,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}
}

DEFUN(dhcp_debug_enable_cmd_func,
	dhcp_debug_enable_cmd,
	"debug dhcp (all|info|error|debug|debug_failover_connect|debug_failover_msg_deal|debug_failover_all)",
	"Add debug dhcp Information\n"
	"Dhcp server\n"	
	"Open dhcp debug level all\n"	
	"Open dhcp debug level info\n"
	"Open dhcp debug level error\n"
	"Open dhcp debug level debug\n"
	"Open dhcp debug level dhcp_failover_connect"
	"Open dhcp debug level dhcp_failover_msg_deal"
	"Open dhcp debug level dhcp_failover_all"	
)
{
	unsigned int ret = 0, debug_type = 0, debug_enable = 1;

	 if(strncmp("all",argv[0],strlen(argv[0]))==0) {
		debug_type = DEBUG_TYPE_ALL;
	}
	else if (strncmp("info",argv[0],strlen(argv[0]))==0) {
		debug_type = DEBUG_TYPE_INFO;
	}
	else if (strncmp("error",argv[0],strlen(argv[0]))==0) {
		debug_type = DEBUG_TYPE_ERROR;
	}
	else if (strncmp("debug",argv[0],strlen(argv[0]))==0) {
		debug_type = DEBUG_TYPE_DEBUG;
	}
	else if(strncmp("debug_failover_connect",argv[0],strlen(argv[0])) == 0){
		debug_type = DEBUG_TYPE_DEBUG_FAILOVER_CONNECT;
	}
	else if(strncmp("debug_failover_msg_deal",argv[0],strlen(argv[0])) == 0){
		debug_type = DEBUG_TYPE_DEBUG_FAILOVER_MSG_DEAL;
	}
	else if(strncmp("debug_failover_all",argv[0],strlen(argv[0])) == 0){
		debug_type = DEBUG_TYPE_DEBUG_FAILOVER_ALL;
	}
	else {
		vty_out(vty,"bad command parameter %s\n", argv[0]);
		return CMD_WARNING;
	}
	ret = dcli_set_debug_state(vty, debug_type, debug_enable);
	
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

DEFUN(dhcp_debug_disable_cmd_func,
	dhcp_debug_disable_cmd,
	"no debug dhcp (all|info|error|debug|debug_failover_connect|debug_failover_msg_deal|debug_failover_all)",
	"Delete old Configuration\n"
	"Config dhcp debugging close\n"
	"Dhcp server\n"
	"Close dhcp debug level all\n"	
	"Close dhcp debug level info\n"
	"Close dhcp debug level error\n"
	"Close dhcp debug level debug\n"
	"Close dhcp debug level dhcp_failover_connect"
	"Close dhcp debug level dhcp_failover_msg_deal"
	"Close dhcp debug level dhcp_failover_all"

)
{
	unsigned int ret = 0, debug_type = 0, isEnable = 0;

    if(strncmp("all",argv[0],strlen(argv[0]))==0) {
		debug_type = DEBUG_TYPE_ALL;
	}
	else if (strncmp("info",argv[0],strlen(argv[0]))==0) {
		debug_type = DEBUG_TYPE_INFO;
	}
	else if (strncmp("error",argv[0],strlen(argv[0]))==0) {
		debug_type = DEBUG_TYPE_ERROR;
	}
	else if (strncmp("debug",argv[0],strlen(argv[0]))==0) {
		debug_type = DEBUG_TYPE_DEBUG;
	}
	else if(strncmp("debug_failover_connect",argv[0],strlen(argv[0])) == 0){
		debug_type = DEBUG_TYPE_DEBUG_FAILOVER_CONNECT;
	}
	else if(strncmp("debug_failover_msg_deal",argv[0],strlen(argv[0])) == 0){
		debug_type = DEBUG_TYPE_DEBUG_FAILOVER_MSG_DEAL;
	}
	else if(strncmp("debug_failover_all",argv[0],strlen(argv[0])) == 0){
		debug_type = DEBUG_TYPE_DEBUG_FAILOVER_ALL;
	}
	else {
		vty_out(vty,"bad command parameter %s\n", argv[0]);
		return CMD_WARNING;
	}

	ret = dcli_set_debug_state(vty, debug_type, isEnable);
	
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}



char *dcli_dhcp_restart_load_hansi_cfg(struct vty *vty)
{
	char *showStr = NULL, *cmdstr = NULL;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int ret=0;
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
										DHCP_DBUS_OBJPATH, 
										DHCP_DBUS_INTERFACE, 
										DHCP_DBUS_METHOD_SHOW_HASNIS_RUNNING_CFG);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "restarting dhcpd failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) {

		if (NULL == (cmdstr = malloc(strlen(showStr) + 50))) {
			vty_out(vty, "not enough memory\n");
			return NULL;
		}
		memset(cmdstr, 0, strlen(showStr) + 50);

		sprintf(cmdstr, "sudo /opt/bin/vtysh -c \"configure terminal\n%s\"", showStr);
		//system(cmdstr);
	} 
	else {
		vty_out(vty, "restarting dhcpd failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}
	
	dbus_message_unref(reply);
	return cmdstr;	
}


DEFUN(dhcp_restart_cmd_func,
	dhcp_restart_cmd,
	"ip dhcp server restart",
	"restart dhcp service\n"
)
{
	char *showStr = NULL, *cmdstr = NULL, *hansi_cfg = NULL;
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int ret=0;
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
										DHCP_DBUS_OBJPATH, 
										DHCP_DBUS_INTERFACE, 
										DHCP_DBUS_METHOD_SHOW_RUNNING_CFG);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "restarting dhcpd failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) {

		cmdstr = malloc(strlen(showStr) + 50);

		memset(cmdstr, 0, strlen(showStr) + 50);
		hansi_cfg = dcli_dhcp_restart_load_hansi_cfg(vty);				

		sprintf(cmdstr, "sudo /opt/bin/vtysh -c \"configure terminal\n%s\"", showStr);

		ret=system("sudo /usr/bin/dhcp_restart.sh");
		if(WEXITSTATUS(ret) == 2) {
			vty_out(vty,"restart dhcpd failed");
		}
		else{
			sleep(3);	
			system(cmdstr);
			system(hansi_cfg);
		}
	} 
	else {
		vty_out(vty, "restarting dhcpd failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	
	dbus_message_unref(reply);
	free(cmdstr);
	if (hansi_cfg) {
		free(hansi_cfg);
	}
	return CMD_SUCCESS;	
}


int 
dcli_dhcrelay_show_running_cfg
(
	struct vty *vty
)
{	
	char *showStr = NULL;
	DBusMessage *query, *reply;
	DBusError err;
    int ret = 1;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
	
	query = dbus_message_new_method_call(DHCRELAY_DBUS_BUSNAME, 
									DHCRELAY_DBUS_OBJPATH, 
									DHCRELAY_DBUS_INTERFACE, 
									DHCRELAY_DBUS_METHOD_SHOW_RUNNING_CFG);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return ret;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) {
	/*
		char _tmpstr[64];
		memset(_tmpstr, 0, 64);
		sprintf(_tmpstr, BUILDING_MOUDLE, "DHCP RELAY");
		vtysh_add_show_string_parse(_tmpstr);	
		*/
		vtysh_add_show_string(showStr);
        ret = 0;
	} 
	else {
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return ret;	
}

char * 
dcli_dhcrelay_show_running_cfg2
(
	struct vty *vty,
	int slot_id
)
{	
	char *showStr = NULL;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	char *tmp = NULL;

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);	
	query = dbus_message_new_method_call(DHCRELAY_DBUS_BUSNAME, 
									DHCRELAY_DBUS_OBJPATH, 
									DHCRELAY_DBUS_INTERFACE, 
									DHCRELAY_DBUS_METHOD_SHOW_RUNNING_CFG);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) {
	
		tmp = (char *)malloc(strlen(showStr)+1);
		memset(tmp, 0, strlen(showStr)+1);
		memcpy(tmp,showStr,strlen(showStr));
		dbus_message_unref(reply);		
		return tmp; 
	} 
	else {
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return NULL;	
}


char * 
dcli_dhcrelay_show_running_hansi_cfg
(
	unsigned int slot_id,unsigned int InstID, unsigned int islocaled 
)
{	
	char *showStr = NULL;
	char * tmp = NULL;
	DBusMessageIter	 iter;
	DBusMessage *query, *reply;
	DBusError err;
    int ret = 1;

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);	
	
	query = dbus_message_new_method_call(DHCRELAY_DBUS_BUSNAME, 
									DHCRELAY_DBUS_OBJPATH, 
									DHCRELAY_DBUS_INTERFACE, 
									DHCRELAY_DBUS_METHOD_SHOW_RUNNING_HANSI_CFG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&slot_id,
							 DBUS_TYPE_UINT32,&InstID,
							 DBUS_TYPE_UINT32,&islocaled,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("DHCP relay show running failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) {
	
		tmp = (char *)malloc(strlen(showStr)+1);
		memset(tmp, 0, strlen(showStr)+1);
		memcpy(tmp,showStr,strlen(showStr));			
		dbus_message_unref(reply);
		return tmp; 
	} 
	else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return NULL;	
}
DEFUN (show_dhcp_running_cfg_cmd_func,
       show_dhcp_running_cfg_cmd,
       "show running-config dhcp",
       SHOW_STR
	   "DHCP module\n")
{
	char *showStr = NULL;
	char _tmpstr[64];	
	DBusMessage *query, *reply;
	DBusError err;

	vty_out(vty, "Building configuration...\n");
	
	memset(_tmpstr, 0, 64);
	sprintf(_tmpstr, BUILDING_MOUDLE, "DHCP SERVER");
	vty_out(vty, "%s\n", _tmpstr);
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SHOW_RUNNING_CFG);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("DHCP show running failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) {

		vty_out(vty, "%s\n", showStr);
	} 
	else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
}



unsigned int
dcli_set_server_pingcheck_enable
(
	struct vty *vty,
	unsigned int enable
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_PING_CHECK_ENABLE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &enable, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}
}


DEFUN(ip_dhcp_server_pingcheck_enable_cmd_func,
	ip_dhcp_server_pingcheck_enable_cmd,
	"ip dhcp server ping-check (enable|disable)",
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"Ping check"
	"Ip dhcp server ping check enable\n"
	"Ip dhcp server ping check disable\n"
)
{
	unsigned int ret = 0, op_ret = 0, enable = 0;

	if(strncmp("enable",argv[0],strlen(argv[0]))==0) {
		enable = 1;
	}
	else if (strncmp("disable",argv[0],strlen(argv[0]))==0) {
		enable = 0;
	}
	else {
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}

	ret = dcli_set_server_pingcheck_enable(vty, enable);
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}


unsigned int
dcli_set_server_lease_expiry_time
(
	struct vty *vty,
	unsigned int time
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_LEASE_EXPIRY_TIME);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &time, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}
}
/*

DEFUN(ip_dhcp_server_lease_default_cmd_func,
	ip_dhcp_server_lease_default_cmd,
	"ip dhcp server lease-time <60-31536000>",
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"	
	"Ip dhcp server lease-time\n"
	"Ip dhcp server lease-time 60-31536000 seconds\n"	
)
{
	unsigned int lease_default = 0, index = 0;
	unsigned int ret = 0, op_ret = 0, mode = 0;

	lease_default = atoi((char*)argv[0]);
*/

DEFUN(ip_dhcp_server_lease_expiry_cmd_func,
	ip_dhcp_server_lease_expiry_cmd,
	"ip dhcp server lease-expiry <10-300>",
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"	
	"Ip dhcp server lease timeout\n"
	"Ip dhcp server lease-time 10-300 seconds\n"	
)
{
	unsigned int ret = 0, expiry_time = 0;

	expiry_time = atoi((char*)argv[0]);

	ret = dcli_set_server_lease_expiry_time(vty, expiry_time);
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

DEFUN(show_dhcp_statistic_info_cmd_func,
	show_dhcp_statistic_info_cmd,
	"show dhcp statistic-information\n",	
	"Dynamic Host Configuration Protocol\n"
	"statistic information\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int ret = 1;
	struct statistics_info info;

	int localid = 1, slot_id = HostSlotId, index = 0;
	get_slotid_index(vty, &index, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_GET_STATISTICS_INFO);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "DHCP show statistics info failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return ret;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &(info.host_num),
					DBUS_TYPE_UINT32, &(info.segment_times),
					DBUS_TYPE_UINT32, &(info.discover_times),
					DBUS_TYPE_UINT32, &(info.offer_times),
					DBUS_TYPE_UINT32, &(info.requested_times),
					DBUS_TYPE_UINT32, &(info.ack_times),
					DBUS_TYPE_INVALID)) {		
	    ret = 0;
	} 
	else {
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	vty_out(vty, "===================================================\n");
	vty_out(vty, "total lease number:   %d\n", info.host_num);
	vty_out(vty, "total assigned lease: %d\n", info.segment_times);
	vty_out(vty, "total DISCOVER packet:%d\n", info.discover_times);	
	vty_out(vty, "total OFFER packet:   %d\n", info.offer_times);
	vty_out(vty, "total REQUEST packet: %d\n", info.requested_times);
	vty_out(vty, "total ACK packet:     %d\n", info.ack_times);
	vty_out(vty, "===================================================\n");

	dbus_message_unref(reply);
	return ret;	
}


unsigned int
dcli_dhcp_server_optimize_enable
(
	struct vty *vty,
	unsigned int enable
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_OPTIMIZE_ENABLE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &enable, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}
}


DEFUN(ip_dhcp_server_optimize_enable_cmd_func,
	ip_dhcp_server_optimize_enable_cmd,
	"ip dhcp server optimize (enable|disable)",
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"performance optimization\n"
	"optimize enable\n"
	"optimize disable\n"
)
{
	unsigned int ret = 0, op_ret = 0, enable = 0;

	if(strncmp("enable",argv[0],strlen(argv[0]))==0) {
		enable = 1;
	}
	else if (strncmp("disable",argv[0],strlen(argv[0]))==0) {
		enable = 0;
	}
	else {
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}

	ret = dcli_dhcp_server_optimize_enable(vty, enable);
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

unsigned int dcli_set_server_option60_enable(struct vty *vty, unsigned int index, unsigned int enable_flag)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);	
	
	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_SERVER_OPTION60_ENABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &index, 
							 DBUS_TYPE_UINT32, &enable_flag, 							 
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args (reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		} else {
			return op_ret;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
}
DEFUN(ip_dhcp_server_option60_enable_cmd_func,
	ip_dhcp_server_option60_enable_cmd,
	"ip dhcp server option60 (enable|disable)",
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"Vendor Class Identifier\n"
	"Parse client option60\n"
	"Donot parse client option60\n"
)
{
	unsigned int ret = 0, op_ret = 0, enable_flag = 0;
	unsigned int index = 0;
	if(0 == strncmp("enable", argv[0], strlen(argv[0]))) {
		enable_flag = 1;
	}
	else if (0 == strncmp("disable", argv[0], strlen(argv[0]))) {
		enable_flag = 0;
	}
	else {
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}

	index = (unsigned int *)(vty->index_sub);	/* pool index */
	
	ret = dcli_set_server_option60_enable(vty, index, enable_flag);	/* 1 : enable, 0 : disable */
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		if (DHCP_NOT_FOUND_POOL == ret) {
			vty_out(vty, "not found pool node\n");
		} else {
			vty_out(vty, "%s option60 failed\n", enable_flag ? "enable" : "disable");
		}	
		return CMD_WARNING;
	}
}
unsigned int
dcli_set_server_option60_set_id(struct vty *vty, unsigned int add_flag, unsigned int index, char *id)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_SERVER_OPTION60_SET_ID);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &add_flag,
							 DBUS_TYPE_UINT32, &index,							 
							 DBUS_TYPE_STRING, &id, 
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args (reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		} else {
			dbus_message_unref(reply);
			return op_ret;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
}
DEFUN(ip_dhcp_server_option60_set_id_cmd_func,
	ip_dhcp_server_option60_set_id_cmd,
	"ip dhcp server option60 ID",	
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"Vendor Class Identifier\n"
	"Specify ID, and length no more than 255 characters\n"
)
{
	unsigned int index = 0;
	unsigned int ret = 0;
	unsigned int state = 0;
	if (strlen(argv[0]) > DHCP_OPTION60_ID_DEFAULT_LENGTH - 1) {
		vty_out(vty, "Vendor Class Identifier is too long\n");
		return CMD_WARNING;
	}
	dcli_get_server_state(vty, &state);
	if (state) {
		vty_out(vty, "Please first disable dhcp server\n");
		return CMD_WARNING;		
	}
	index = (unsigned int *)(vty->index_sub);	
	ret = dcli_set_server_option60_set_id(vty, DHCP_OPTION60_ADD_ID, index, (char *)argv[0]);
	if (!ret) {
		vty_out(vty, "add Vendor Class Identifier \"%s\"\n", argv[0]);		
		return CMD_SUCCESS;
	} else {	
		if (DHCP_NOT_FOUND_POOL == ret) {
			vty_out(vty, "not found pool, add failed\n");
		} else if (DHCP_ALREADY_ADD_OPTION == ret) {
			vty_out(vty, "Already add this ID\n");
		} else if (DHCP_MAX_OPTION_LIST_WANNING == ret) {
			vty_out(vty, "Max set four ID, add failed\n");		
		} else {
			vty_out(vty, "Add Vendor Class Identifier failed\n");
		}
		return CMD_WARNING;
	}
}
DEFUN(no_ip_dhcp_server_option60_set_id_cmd_func,
	no_ip_dhcp_server_option60_id_cmd,
	"no ip dhcp server option60 ID",	
	"Delete dhcp option60\n"
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"Vendor Class Identifier\n"
	"Specify ID, and name length no more than 31 characters\n"
)
{
	unsigned int index = 0;
	unsigned int ret = 0;
	unsigned int state = 0;
	if (strlen(argv[0]) > DHCP_OPTION60_ID_DEFAULT_LENGTH - 1) {
		vty_out(vty, "Vendor Class Identifier is too long\n");
		return CMD_WARNING;
	}
	dcli_get_server_state(vty, &state);
	if (state) {
		vty_out(vty, "Please first disable dhcp server\n");
		return CMD_WARNING;		
	}
	index = (unsigned int *)(vty->index_sub);	
	ret = dcli_set_server_option60_set_id(vty, DHCP_OPTION60_DELETE_ID, index, (char *)argv[0]);
	if (!ret) {
		return CMD_SUCCESS;
	} else {	
		if (DHCP_NOT_FOUND_POOL == ret){
			vty_out(vty, "not found pool\n");
		}else{
			vty_out(vty, "delete dhcp option60 failed\n");
		}
		return CMD_WARNING;
	}
}

unsigned int
dcli_set_bootp_enable
(
	struct vty *vty,
	unsigned int enable
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCP_DBUS_BUSNAME, 
									DHCP_DBUS_OBJPATH, 
									DHCP_DBUS_INTERFACE, 
									DHCP_DBUS_METHOD_SET_BOOTP_ENABLE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &enable, 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
				
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args (reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		if(!op_ret) {
	
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}
}


DEFUN(ip_dhcp_server_bootp_enable_cmd_func,
	ip_dhcp_server_bootp_enable_cmd,
	"ip dhcp server bootp (enable|disable)",
	"Ipv4 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcp server\n"
	"BOOTP Protocol\n"	
	"BOOTP Protocol support enable\n"
	"BOOTP Protocol support enable\n"
)
{
	unsigned int ret = 0, op_ret = 0, isEnable = 0;

	if(strncmp("enable",argv[0],strlen(argv[0]))==0) {
		isEnable = 1;
		vty_out(vty, "===========================================================\n");
		vty_out(vty, "DHCP server assign an infinite lease duration\n"
			" for automatic addresses assigned to BOOTP clients.\n"
			"If there is a great amount of BOOTP clients,\n" 
			" may run out of lease." " So, please be careful!\n");
		vty_out(vty, "===========================================================\n");		
	}
	else if (strncmp("disable",argv[0],strlen(argv[0]))==0) {
		isEnable = 0;
	}
	else {
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}

	ret = dcli_set_bootp_enable(vty, isEnable);
	
	if (!ret) {
		
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}


void 
dcli_dhcp_init
(
	void
)  
{
	install_node (&pool_node, dcli_dhcp_show_running_cfg, "POOL_NODE");
//	install_node (&pool_node, NULL, "POOL_NODE");
	
	install_node (&dhcrelay_node, dcli_dhcrelay_show_running_cfg, "DHCRELAY_NODE");
	install_default(POOL_NODE);

	/*show dhcp lease state*/
	install_element(CONFIG_NODE, &show_dhcp_lease_state);
	install_element(CONFIG_NODE, &show_dhcp_pool_statistics);	
	install_element(CONFIG_NODE, &create_ip_pool_name_cmd);	
	install_element(CONFIG_NODE, &delete_ip_pool_name_cmd);
	install_element(CONFIG_NODE, &config_ip_pool_name_cmd);	
	install_element(CONFIG_NODE, &add_dhcp_static_host_cmd);	
	install_element(CONFIG_NODE, &delete_dhcp_static_host_cmd);	
	install_element(CONFIG_NODE, &ip_dhcp_server_enable_cmd);
	install_element(CONFIG_NODE, &ip_dhcp_dynamic_arp_enable_cmd);
	install_element(CONFIG_NODE, &ip_dhcp_server_nak_rsp_cmd);
	install_element(CONFIG_NODE, &ip_dhcp_server_asn_cmd);
	install_element(CONFIG_NODE, &ip_dhcp_server_static_arp_enable_cmd);
	install_element(POOL_NODE, &add_dhcp_pool_ip_range_cmd);
	install_element(CONFIG_NODE, &ip_dhcp_server_domain_name_cmd);
	install_element(POOL_NODE, &ip_dhcp_server_domain_name_cmd);

	install_element(POOL_NODE, &ip_dhcp_server_option43_v1_cmd);
	install_element(POOL_NODE, &no_ip_dhcp_server_option43_v1_cmd);
	install_element(POOL_NODE, &del_ip_dhcp_server_option43_cmd);
	
	install_element(POOL_NODE, &ip_dhcp_server_option43_cmd);
/*	install_element(CONFIG_NODE, &no_ip_dhcp_server_option43_cmd);*/
	install_element(POOL_NODE, &no_ip_dhcp_server_option43_cmd);
	
	install_element(POOL_NODE, &ip_dhcp_server_option138_cmd);
	install_element(POOL_NODE, &no_ip_dhcp_server_option138_cmd);
	install_element(POOL_NODE, &del_ip_dhcp_server_option138_cmd);	
	
	install_element(CONFIG_NODE, &no_ip_dhcp_server_domain_name_cmd);
	install_element(POOL_NODE, &no_ip_dhcp_server_domain_name_cmd);
	install_element(CONFIG_NODE, &ip_dhcp_server_dns_cmd);
	install_element(POOL_NODE, &ip_dhcp_server_dns_cmd);
	install_element(CONFIG_NODE, &no_ip_dhcp_server_dns_cmd);
	install_element(POOL_NODE, &no_ip_dhcp_server_dns_cmd);
	install_element(CONFIG_NODE, &ip_dhcp_server_routers_ip_cmd);
	install_element(POOL_NODE, &ip_dhcp_server_routers_ip_cmd);
	install_element(CONFIG_NODE, &no_ip_dhcp_server_routers_ip_cmd);
	install_element(POOL_NODE, &no_ip_dhcp_server_routers_ip_cmd);
	install_element(CONFIG_NODE, &ip_dhcp_server_wins_ip_cmd);
	install_element(POOL_NODE, &ip_dhcp_server_wins_ip_cmd);
	install_element(CONFIG_NODE, &no_ip_dhcp_server_wins_ip_cmd);
	install_element(POOL_NODE, &no_ip_dhcp_server_wins_ip_cmd);
	install_element(CONFIG_NODE, &ip_dhcp_server_lease_default_cmd);
	install_element(POOL_NODE, &ip_dhcp_server_lease_default_cmd);
	install_element(CONFIG_NODE, &no_ip_dhcp_server_lease_default_cmd);
	install_element(POOL_NODE, &no_ip_dhcp_server_lease_default_cmd);

	install_element(POOL_NODE, &ip_dhcp_server_option60_enable_cmd);	
	install_element(POOL_NODE, &ip_dhcp_server_option60_set_id_cmd);
	install_element(POOL_NODE, &no_ip_dhcp_server_option60_id_cmd);		

	/*
	install_element(CONFIG_NODE, &ip_dhcp_server_lease_max_cmd);
	install_element(POOL_NODE, &ip_dhcp_server_lease_max_cmd);
	install_element(CONFIG_NODE, &no_ip_dhcp_server_lease_max_cmd);
	install_element(POOL_NODE, &no_ip_dhcp_server_lease_max_cmd);
	*/
	install_element(CONFIG_NODE, &save_dhcp_lease_cmd);
	install_element(CONFIG_NODE, &show_dhcp_lease_cmd);
	install_element(CONFIG_NODE, &dhcp_client_lease_time);
	install_element(CONFIG_NODE, &show_dhcp_lease_by_ip_cmd);
	install_element(CONFIG_NODE, &show_dhcp_lease_by_ip_diff_cmd);
	install_element(CONFIG_NODE, &show_dhcp_lease_by_ip_mask_cmd);
	install_element(CONFIG_NODE, &show_dhcp_lease_by_mac_cmd);
	install_element(CONFIG_NODE, &show_ip_dhcp_server_cmd);
	install_element(POOL_NODE, &add_dhcp_failover_pool_cmd);
	install_element(POOL_NODE, &cfg_dhcp_failover_pool_cmd);
	install_element(POOL_NODE, &del_dhcp_failover_pool_cmd);
	install_element(POOL_NODE, &show_failover_configure_cmd);
	install_element(POOL_NODE, &show_ip_pool_cmd);
	install_element(CONFIG_NODE, &show_ip_pool_cmd);
	install_element(CONFIG_NODE, &show_ip_dhcp_static_cmd);
	install_element(INTERFACE_NODE, &set_interface_ip_pool_cmd);
	install_element(INTERFACE_NODE, &del_interface_ip_pool_cmd);
	install_element(CONFIG_NODE, &dhcp_set_reply_mode_cmd);
	install_element(INTERFACE_NODE, &add_interface_ip_relay_cmd);	
	install_element(INTERFACE_NODE, &del_interface_ip_relay_cmd);
	install_element(CONFIG_NODE, &show_ip_dhcp_relay_cmd);
	install_element(CONFIG_NODE, &ip_dhcp_relay_enable_cmd);
	install_element(CONFIG_NODE, &relay_debug_enable_cmd);
	install_element(CONFIG_NODE, &relay_debug_disable_cmd);	
	install_element(CONFIG_NODE, &dhcp_debug_enable_cmd);	
	install_element(CONFIG_NODE, &dhcp_debug_disable_cmd);	
	install_element(CONFIG_NODE, &show_dhcp_running_config);
	install_element(CONFIG_NODE, &ip_dhcp_server_pingcheck_enable_cmd);	
	install_element(CONFIG_NODE, &ip_dhcp_server_lease_expiry_cmd);
	install_element(CONFIG_NODE, &show_dhcp_statistic_info_cmd);
	install_element(CONFIG_NODE, &ip_dhcp_server_optimize_enable_cmd);		
	install_element(CONFIG_NODE, &ip_dhcp_server_bootp_enable_cmd);

	install_element(HIDDENDEBUG_NODE, &dhcp_restart_cmd);


	/* hansi node */
	install_element(HANSI_NODE, &create_ip_pool_name_cmd);				/* pool node */	
	install_element(HANSI_NODE, &delete_ip_pool_name_cmd);
	install_element(HANSI_NODE, &config_ip_pool_name_cmd);
	install_element(HANSI_NODE, &show_ip_pool_cmd);						
	
	install_element(HANSI_NODE, &show_dhcp_running_config);

	install_element(HANSI_NODE, &ip_dhcp_server_enable_cmd);
	install_element(HANSI_NODE, &ip_dhcp_dynamic_arp_enable_cmd);
	install_element(HANSI_NODE, &ip_dhcp_server_nak_rsp_cmd);
	install_element(HANSI_NODE, &ip_dhcp_server_asn_cmd);

	install_element(HANSI_NODE, &ip_dhcp_server_routers_ip_cmd);		/* global router */
	install_element(HANSI_NODE, &no_ip_dhcp_server_routers_ip_cmd);		
	
	install_element(HANSI_NODE, &ip_dhcp_server_lease_default_cmd);		/* lease time */
	install_element(HANSI_NODE, &no_ip_dhcp_server_lease_default_cmd);
	install_element(HANSI_NODE, &dhcp_client_lease_time);


	install_element(HANSI_NODE, &dhcp_debug_enable_cmd);				/* debug */
	install_element(HANSI_NODE, &dhcp_debug_disable_cmd);	

	install_element(HANSI_NODE, &ip_dhcp_server_domain_name_cmd);
	install_element(HANSI_NODE, &no_ip_dhcp_server_domain_name_cmd);	/* domain */

	install_element(HANSI_NODE, &show_ip_dhcp_server_cmd);				/* show dhcp server */

	install_element(HANSI_NODE, &ip_dhcp_server_dns_cmd);					/* dns */
	install_element(HANSI_NODE, &no_ip_dhcp_server_dns_cmd);

	install_element(HANSI_NODE, &ip_dhcp_server_wins_ip_cmd);				/* win */
	install_element(HANSI_NODE, &no_ip_dhcp_server_wins_ip_cmd);

	install_element(HANSI_NODE, &show_dhcp_lease_cmd);					/* show dhcp-lease */
	install_element(HANSI_NODE, &show_dhcp_lease_by_ip_cmd);
	install_element(HANSI_NODE, &show_dhcp_lease_by_ip_diff_cmd);
	install_element(HANSI_NODE, &show_dhcp_lease_by_ip_mask_cmd);
	install_element(HANSI_NODE, &show_dhcp_lease_by_mac_cmd);
	install_element(HANSI_NODE, &show_dhcp_lease_state);
	install_element(HANSI_NODE, &save_dhcp_lease_cmd);					/* save dhcp lease */
	install_element(HANSI_NODE, &show_dhcp_pool_statistics);	
	
	install_element(HANSI_NODE, &show_ip_dhcp_static_cmd);				/* static ip (ip:mac) */
	install_element(HANSI_NODE, &add_dhcp_static_host_cmd);			
	install_element(HANSI_NODE, &delete_dhcp_static_host_cmd);	
	
	install_element(HANSI_NODE, &dhcp_set_reply_mode_cmd);				/*  server reply mode */

	install_element(HANSI_NODE, &ip_dhcp_relay_enable_cmd);				/* dhcp relay */
	install_element(HANSI_NODE, &show_ip_dhcp_relay_cmd);				
	install_element(HANSI_NODE, &relay_debug_enable_cmd);
	install_element(HANSI_NODE, &relay_debug_disable_cmd);	

	install_element(HANSI_NODE, &ip_dhcp_server_static_arp_enable_cmd);	/* static arp */

	install_element(HANSI_NODE, &ip_dhcp_server_pingcheck_enable_cmd);
	install_element(HANSI_NODE, &ip_dhcp_server_lease_expiry_cmd);		
	install_element(HANSI_NODE, &show_dhcp_statistic_info_cmd);		
	install_element(HANSI_NODE, &ip_dhcp_server_optimize_enable_cmd);

	install_element(HANSI_NODE, &ip_dhcp_server_bootp_enable_cmd);

	
	/* hansi pool node */
	install_node(&hansi_pool_node,NULL,"HANSI_POOL_NODE");
	install_default(HANSI_POOL_NODE);
	install_element(HANSI_POOL_NODE, &add_dhcp_pool_ip_range_cmd);

	install_element(HANSI_POOL_NODE, &ip_dhcp_server_routers_ip_cmd);		/* pool node router */
	install_element(HANSI_POOL_NODE, &no_ip_dhcp_server_routers_ip_cmd);

	install_element(HANSI_POOL_NODE, &ip_dhcp_server_lease_default_cmd);	/* lease time */
	install_element(HANSI_POOL_NODE, &no_ip_dhcp_server_lease_default_cmd);

	install_element(HANSI_POOL_NODE, &show_ip_pool_cmd);					/* show ip pool */

	install_element(HANSI_POOL_NODE, &ip_dhcp_server_option43_cmd);			/* option43	*/
	install_element(HANSI_POOL_NODE, &no_ip_dhcp_server_option43_cmd);
	install_element(HANSI_POOL_NODE, &ip_dhcp_server_option43_v1_cmd);
	install_element(HANSI_POOL_NODE, &no_ip_dhcp_server_option43_v1_cmd);
	install_element(HANSI_POOL_NODE, &del_ip_dhcp_server_option43_cmd);

	install_element(HANSI_POOL_NODE, &ip_dhcp_server_option138_cmd);		/* option138 */
	install_element(HANSI_POOL_NODE, &no_ip_dhcp_server_option138_cmd);
	install_element(HANSI_POOL_NODE, &del_ip_dhcp_server_option138_cmd);	

	install_element(HANSI_POOL_NODE, &ip_dhcp_server_domain_name_cmd);
	install_element(HANSI_POOL_NODE, &no_ip_dhcp_server_domain_name_cmd);	/* domain */

	install_element(HANSI_POOL_NODE, &ip_dhcp_server_dns_cmd);				/* dns */
	install_element(HANSI_POOL_NODE, &no_ip_dhcp_server_dns_cmd);

	install_element(HANSI_POOL_NODE, &ip_dhcp_server_wins_ip_cmd);			/* win */
	install_element(HANSI_POOL_NODE, &no_ip_dhcp_server_wins_ip_cmd);

	install_element(HANSI_POOL_NODE, &add_dhcp_failover_pool_cmd);			/* failover ? */
	install_element(HANSI_POOL_NODE, &cfg_dhcp_failover_pool_cmd);
	install_element(HANSI_POOL_NODE, &del_dhcp_failover_pool_cmd);
	install_element(HANSI_POOL_NODE, &show_failover_configure_cmd);

	install_element(HANSI_POOL_NODE, &ip_dhcp_server_option60_enable_cmd);	
	install_element(HANSI_POOL_NODE, &ip_dhcp_server_option60_set_id_cmd);
	install_element(HANSI_POOL_NODE, &no_ip_dhcp_server_option60_id_cmd);		


	/*		TODO
	install_element(HIDDENDEBUG_NODE, &dhcp_restart_cmd);	
	*/
	/* local hansi node */
	install_element(LOCAL_HANSI_NODE, &create_ip_pool_name_cmd);				/* pool node */	
	install_element(LOCAL_HANSI_NODE, &delete_ip_pool_name_cmd);
	install_element(LOCAL_HANSI_NODE, &config_ip_pool_name_cmd);
	install_element(LOCAL_HANSI_NODE, &show_ip_pool_cmd);						
	
	install_element(LOCAL_HANSI_NODE, &show_dhcp_running_config);

	install_element(LOCAL_HANSI_NODE, &ip_dhcp_server_enable_cmd);
	install_element(LOCAL_HANSI_NODE, &ip_dhcp_dynamic_arp_enable_cmd);
	install_element(LOCAL_HANSI_NODE, &ip_dhcp_server_nak_rsp_cmd);
	install_element(LOCAL_HANSI_NODE, &ip_dhcp_server_asn_cmd);

	install_element(LOCAL_HANSI_NODE, &ip_dhcp_server_routers_ip_cmd);		/* global router */
	install_element(LOCAL_HANSI_NODE, &no_ip_dhcp_server_routers_ip_cmd);		
	
	install_element(LOCAL_HANSI_NODE, &ip_dhcp_server_lease_default_cmd);		/* lease time */
	install_element(LOCAL_HANSI_NODE, &no_ip_dhcp_server_lease_default_cmd);
	install_element(LOCAL_HANSI_NODE, &dhcp_client_lease_time);


	install_element(LOCAL_HANSI_NODE, &dhcp_debug_enable_cmd);					/* debug */
	install_element(LOCAL_HANSI_NODE, &dhcp_debug_disable_cmd);	

	install_element(LOCAL_HANSI_NODE, &ip_dhcp_server_domain_name_cmd);
	install_element(LOCAL_HANSI_NODE, &no_ip_dhcp_server_domain_name_cmd);		/* domain */

	install_element(LOCAL_HANSI_NODE, &show_ip_dhcp_server_cmd);				/* show dhcp server */

	install_element(LOCAL_HANSI_NODE, &ip_dhcp_server_dns_cmd);					/* dns */
	install_element(LOCAL_HANSI_NODE, &no_ip_dhcp_server_dns_cmd);

	install_element(LOCAL_HANSI_NODE, &ip_dhcp_server_wins_ip_cmd);				/* win */
	install_element(LOCAL_HANSI_NODE, &no_ip_dhcp_server_wins_ip_cmd);

	install_element(LOCAL_HANSI_NODE, &show_dhcp_lease_cmd);					/* show dhcp-lease */
	install_element(LOCAL_HANSI_NODE, &show_dhcp_lease_by_ip_cmd);
	install_element(LOCAL_HANSI_NODE, &show_dhcp_lease_by_ip_diff_cmd);
	install_element(LOCAL_HANSI_NODE, &show_dhcp_lease_by_ip_mask_cmd);
	install_element(LOCAL_HANSI_NODE, &show_dhcp_lease_by_mac_cmd);
	install_element(LOCAL_HANSI_NODE, &show_dhcp_lease_state);
	install_element(LOCAL_HANSI_NODE, &save_dhcp_lease_cmd);					/* save dhcp lease */
	install_element(LOCAL_HANSI_NODE, &show_dhcp_pool_statistics);	
	
	install_element(LOCAL_HANSI_NODE, &show_ip_dhcp_static_cmd);				/* static ip (ip:mac) */
	install_element(LOCAL_HANSI_NODE, &add_dhcp_static_host_cmd);			
	install_element(LOCAL_HANSI_NODE, &delete_dhcp_static_host_cmd);	
	
	install_element(LOCAL_HANSI_NODE, &dhcp_set_reply_mode_cmd);				/*  server reply mode */

	install_element(LOCAL_HANSI_NODE, &ip_dhcp_relay_enable_cmd);				/* dhcp relay */
	install_element(LOCAL_HANSI_NODE, &show_ip_dhcp_relay_cmd);				
	install_element(LOCAL_HANSI_NODE, &relay_debug_enable_cmd);
	install_element(LOCAL_HANSI_NODE, &relay_debug_disable_cmd);	

	install_element(LOCAL_HANSI_NODE, &ip_dhcp_server_static_arp_enable_cmd);	/* static arp */

	install_element(LOCAL_HANSI_NODE, &ip_dhcp_server_pingcheck_enable_cmd);
	install_element(LOCAL_HANSI_NODE, &ip_dhcp_server_lease_expiry_cmd);		
	install_element(LOCAL_HANSI_NODE, &show_dhcp_statistic_info_cmd);		
	install_element(LOCAL_HANSI_NODE, &ip_dhcp_server_optimize_enable_cmd);

	install_element(LOCAL_HANSI_NODE, &ip_dhcp_server_bootp_enable_cmd);

	
	/* hansi pool node */
	install_node(&local_hansi_pool_node,NULL,"LOCAL_HANSI_POOL_NODE");
	install_default(LOCAL_HANSI_POOL_NODE);
	install_element(LOCAL_HANSI_POOL_NODE, &add_dhcp_pool_ip_range_cmd);

	install_element(LOCAL_HANSI_POOL_NODE, &ip_dhcp_server_routers_ip_cmd);		/* pool node router */
	install_element(LOCAL_HANSI_POOL_NODE, &no_ip_dhcp_server_routers_ip_cmd);

	install_element(LOCAL_HANSI_POOL_NODE, &ip_dhcp_server_lease_default_cmd);	/* lease time */
	install_element(LOCAL_HANSI_POOL_NODE, &no_ip_dhcp_server_lease_default_cmd);

	install_element(LOCAL_HANSI_POOL_NODE, &show_ip_pool_cmd);					/* show ip pool */

	install_element(LOCAL_HANSI_POOL_NODE, &ip_dhcp_server_option43_cmd);			/* option43	*/
	install_element(LOCAL_HANSI_POOL_NODE, &no_ip_dhcp_server_option43_cmd);
	install_element(LOCAL_HANSI_POOL_NODE, &ip_dhcp_server_option43_v1_cmd);
	install_element(LOCAL_HANSI_POOL_NODE, &no_ip_dhcp_server_option43_v1_cmd);
	install_element(LOCAL_HANSI_POOL_NODE, &del_ip_dhcp_server_option43_cmd);

	install_element(LOCAL_HANSI_POOL_NODE, &ip_dhcp_server_option138_cmd);		/* option138 */
	install_element(LOCAL_HANSI_POOL_NODE, &no_ip_dhcp_server_option138_cmd);
	install_element(LOCAL_HANSI_POOL_NODE, &del_ip_dhcp_server_option138_cmd);	

	install_element(LOCAL_HANSI_POOL_NODE, &ip_dhcp_server_domain_name_cmd);
	install_element(LOCAL_HANSI_POOL_NODE, &no_ip_dhcp_server_domain_name_cmd);	/* domain */

	install_element(LOCAL_HANSI_POOL_NODE, &ip_dhcp_server_dns_cmd);				/* dns */
	install_element(LOCAL_HANSI_POOL_NODE, &no_ip_dhcp_server_dns_cmd);

	install_element(LOCAL_HANSI_POOL_NODE, &ip_dhcp_server_wins_ip_cmd);			/* win */
	install_element(LOCAL_HANSI_POOL_NODE, &no_ip_dhcp_server_wins_ip_cmd);

	install_element(LOCAL_HANSI_POOL_NODE, &add_dhcp_failover_pool_cmd);			/* failover ? */
	install_element(LOCAL_HANSI_POOL_NODE, &cfg_dhcp_failover_pool_cmd);
	install_element(LOCAL_HANSI_POOL_NODE, &del_dhcp_failover_pool_cmd);
	install_element(LOCAL_HANSI_POOL_NODE, &show_failover_configure_cmd);

	install_element(LOCAL_HANSI_POOL_NODE, &ip_dhcp_server_option60_enable_cmd);	
	install_element(LOCAL_HANSI_POOL_NODE, &ip_dhcp_server_option60_set_id_cmd);
	install_element(LOCAL_HANSI_POOL_NODE, &no_ip_dhcp_server_option60_id_cmd);		

	/*		TODO
	/* 
	install_element(INTERFACE_NODE, &set_interface_ip_pool_cmd);
	install_element(INTERFACE_NODE, &del_interface_ip_pool_cmd);
	install_element(INTERFACE_NODE, &add_interface_ip_relay_cmd);	
	install_element(INTERFACE_NODE, &del_interface_ip_relay_cmd);
	 */
}
#ifdef __cplusplus
}
#endif

