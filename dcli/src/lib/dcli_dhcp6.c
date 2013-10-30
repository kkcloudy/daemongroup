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
*  		$Revision: 1.3 $	
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
#include "dbus/dhcp/dhcp6_dbus_def.h"
#include "dcli_main.h"
#include "vty.h"
#include "command.h"
#include "if.h"
#include "sysdef/returncode.h"
#include "dcli_dhcp6.h"

struct cmd_node poolv6_node = 
{
	POOLV6_NODE,
	"%s(config-poolv6)# ",
	1
};
struct cmd_node hansi_poolv6_node =
{
	HANSI_POOLV6_NODE,
	"%s(hansi%d-%d poolv6)# ",
	1
};
struct cmd_node local_hansi_poolv6_node =
{
	LOCAL_HANSI_POOLV6_NODE,
	"%s(local hansi%d-%d poolv6)# ",
	1
};

#define ALIAS_NAME_SIZE 		0x15
#define MAC_ADDRESS_LEN			6
#define MAX_IP_STRING_LEN		16

#define DHCP6_RETURN_SUCCESS	0
#define DHCP6_RETURN_ERROR	1
#define IPV6_MAX_LEN    (sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255") + 1)

#define MAX_OPTION52_ADDRESS_LIST 	8

//#define DHCPv6_DEBUG

extern char vlan_eth_port_ifname [INTERFACE_NAMSIZ];

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
	struct dhcp_sub_show* sub_show;
	unsigned int sub_count;
	struct dhcp_option_show option_show;
};

struct dhcp_global_show
{
	unsigned int enable;
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
};

static int
str2_ipv6_addr
( 
	char *str,
	struct iaddr *p
)
{
	int ret;

	ret = inet_pton(AF_INET6, str, p->iabuf);
	if (ret != 1) {
		return 0;
	}
	p->len= 16;

	return ret;
}

char*
ipv6_long2string(unsigned long ipAddress, unsigned char *buff)
{
	unsigned long	cnt = 0;
	unsigned char *tmpPtr = buff;
	
	cnt = sprintf((char*)tmpPtr,"%ld.%ld.%ld.%ld\n",(ipAddress>>24) & 0xFF, \
			(ipAddress>>16) & 0xFF,(ipAddress>>8) & 0xFF,ipAddress & 0xFF);
	
	return tmpPtr;
}

DEFUN(save_dhcp_lease_ipv6_cmd_func,
	save_dhcp_lease_ipv6_cmd,
	"save dhcp-lease-ipv6",
	"Save dhcp-lease-ipv6\n"
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
	
	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SAVE_DHCP_LEASE);
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

DEFUN(show_dhcp6_lease_cmd_func,
	show_dhcp6_lease_cmd,
	"show dhcp-lease-ipv6",
	"show dhcp-lease-ipv6\n"
	"Show dhcp-lease-ipv6 information\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array,  iter_ipv6_array;;
	unsigned int op_ret = 0,ret = 0,i = 0,j = 0;
	unsigned int count = 0, value = 0;
	char *ipaddr ;
	unsigned char mac[MAC_ADDRESS_LEN] = {0, 0, 0, 0, 0,0};
	int ipv6_num;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
	
	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SHOW_DHCP_LEASE);
	

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

				for(j = 0; j< MAC_ADDRESS_LEN; j++) {
		 			dbus_message_iter_get_basic(&iter_struct,&mac[j]);					
					dbus_message_iter_next(&iter_struct);
				}				

				dbus_message_iter_get_basic(&iter_struct, &(ipv6_num));			
				dbus_message_iter_next(&iter_struct); 
				
				dbus_message_iter_recurse(&iter_struct,&iter_ipv6_array);				
				
				for (j = 0; j < ipv6_num; j++){					
					DBusMessageIter iter_ipv6_struct;
					dbus_message_iter_recurse(&iter_ipv6_array,&iter_ipv6_struct);
					
					dbus_message_iter_get_basic(&iter_ipv6_struct,&ipaddr);		
					dbus_message_iter_next(&iter_ipv6_struct);					

					dbus_message_iter_next(&iter_ipv6_array);
					vty_out(vty,"%s ", ipaddr);
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


/********************************************************
 * check_ipv6_address
 *
 * Check the legality of the ipv6 address
 *
 *	INPUT:
 *		ipv6_address
 *		
 *	OUTPUT:
 *		void
 *
 *	RETURN:
 *		DHCP_IP6_RET_ERROR	      - Legal address
 *		DHCP_IP6_RET_SUCCESS	- Illegal address
 *
 *********************************************************/
static int check_ipv6_address(char *ipv6_address)
{
	char addrptr[128] = {0};
	if(NULL == ipv6_address)
	{
		return DHCP6_RETURN_ERROR;
	}

	if(inet_pton(AF_INET6, ipv6_address, addrptr) != 1)
	{
		return DHCP6_RETURN_ERROR;
	}

	return DHCP6_RETURN_SUCCESS;
}


/********************************************************
 * dcli_show_lease_by_ipv6
 *
 * show lease by ipv6 address
 *
 *	INPUT:
 *		ipAddrl
 *		
 *		vty
 *	OUTPUT:
 *		void
 *
 *	RETURN:
 *		CMD_WARNING	    
 *		CMD_SUCCESS		
 *
 *********************************************************/

int 
dcli_show_lease_by_ipv6
(
	struct iaddr *ipaddrl, 
	struct vty *vty
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError 		err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;	
	unsigned int	op_ret = 0, ret = 0, i = 0, j = 0;
	unsigned int 	count = 0, value = 0, ip_addr = 0;
	unsigned char 	mac[MAC_ADDRESS_LEN] = {0, 0, 0, 0, 0, 0};	

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
	
	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SHOW_DHCP_LEASE_BY_IP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE, &(ipaddrl->iabuf[0]),
    							 DBUS_TYPE_BYTE, &(ipaddrl->iabuf[1]),
    							 DBUS_TYPE_BYTE, &(ipaddrl->iabuf[2]),
    							 DBUS_TYPE_BYTE, &(ipaddrl->iabuf[3]),
    							 DBUS_TYPE_BYTE, &(ipaddrl->iabuf[4]),
    							 DBUS_TYPE_BYTE, &(ipaddrl->iabuf[5]),
    							 DBUS_TYPE_BYTE, &(ipaddrl->iabuf[6]),
    							 DBUS_TYPE_BYTE, &(ipaddrl->iabuf[7]),
    							 DBUS_TYPE_BYTE, &(ipaddrl->iabuf[8]),
    							 DBUS_TYPE_BYTE, &(ipaddrl->iabuf[9]),
    							 DBUS_TYPE_BYTE, &(ipaddrl->iabuf[10]),
    							 DBUS_TYPE_BYTE, &(ipaddrl->iabuf[11]),
    							 DBUS_TYPE_BYTE, &(ipaddrl->iabuf[12]),
    							 DBUS_TYPE_BYTE, &(ipaddrl->iabuf[13]),
    							 DBUS_TYPE_BYTE, &(ipaddrl->iabuf[14]),							
    							 DBUS_TYPE_BYTE, &(ipaddrl->iabuf[15]),
							 DBUS_TYPE_INVALID);
		   
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
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

DEFUN(show_dhcp_lease_by_ipv6_cmd_func,
	show_dhcp_lease_by_ipv6_cmd,
	"show dhcp-lease-ipv6 X:X::X:X",
	SHOW_STR
	"show dhcp-lease-ipv6 by ip\n"
	"Show dhcp-lease information\n"
)
{
	struct iaddr ipAddrl;
	unsigned int ret;
	
	if(check_ipv6_address(argv[0])){
		 vty_out(vty, "%% address %s  is Illegal\n", argv[0]);
		 return CMD_WARNING;
	}

	ret = str2_ipv6_addr((char*)argv[0], &ipAddrl);
	if (!ret) {
		return CMD_WARNING;
	}
	
	/*show dhcp ipv6 lease*/
	dcli_show_lease_by_ipv6(&ipAddrl, vty);
	
	return CMD_SUCCESS;
}

DEFUN(show_dhcp_lease_by_ipv6_diff_cmd_func,
	show_dhcp_lease_by_ipv6_diff_cmd,
	"show dhcp-lease-ipv6 A.B.C.D A.B.C.D",
	"show dhcp-lease-ipv6 by ip diff\n"
	"Show dhcp-lease information\n"
)
{
	unsigned int ipAddrl = 0, ipAddrb = 0, ip_Nums = 0;

	ipAddrl = dcli_ip2ulong((char*)argv[0]);
	ipAddrb = dcli_ip2ulong((char*)argv[1]);
	if (ipAddrl > ipAddrb) {
		return CMD_WARNING;
	}	

	ip_Nums = ipAddrb - ipAddrl;

	dcli_show_lease_by_ipv6(ipAddrl,  vty);
	
	return CMD_SUCCESS;
}

DEFUN(show_dhcp_lease_by_ipv6_mask_cmd_func,
	show_dhcp_lease_by_ipv6_mask_cmd,
	"show dhcp-lease-ipv6 A.B.C.D/M",
	"show dhcp-lease by ip\n"
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
	dcli_show_lease_by_ipv6(ipAddr, vty);
	
	return CMD_SUCCESS;
}

DEFUN(show_dhcp_lease_by_mac_ipv6_cmd_func,
	show_dhcp_lease_by_mac_ipv6_cmd,
	"show dhcp-lease-ipv6 MAC",
	"show dhcp-lease-ipv6 by mac\n"
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
		
	/*query*/

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
	
	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SHOW_DHCP_LEASE_BY_MAC);
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
dcli_show_static_host_ipv6
(
	struct dhcp_static_show **static_host,
	unsigned int *host_num,
	struct vty *vty
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

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
		
	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SHOW_STATIC_HOST);
	

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
				
				dbus_message_iter_next(&iter_array);
			}
		}

	}
	
	*host_num = count;
	*static_host = host;
	dbus_message_unref(reply);

	return CMD_SUCCESS;

}

DEFUN(show_ipv6_dhcp_static_cmd_func,
	show_ipv6_dhcp_static_cmd,
	"show ipv6 dhcp static",
	"Show ipv6 dhcp static\n"
	"Show ip dhcp static information\n"
)
{
	struct dhcp_static_show *host = NULL;
	unsigned int i = 0, count = 0;

	dcli_show_static_host_ipv6(&host, &count,vty);
	
	vty_out(vty, "========================================\n");
	vty_out(vty, "          dhcp static host\n");
	for (i = 0; i < count; i++) {
		vty_out(vty,"%-3d.%-3d.%-3d.%-3d ",(((host[i].ipaddr) & 0xff000000) >> 24),(((host[i].ipaddr) & 0xff0000) >> 16),	\
			(((host[i].ipaddr) & 0xff00) >> 8),((host[i].ipaddr) & 0xff));
		
		vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x \n",host[i].mac[0], host[i].mac[1], host[i].mac[2], host[i].mac[3], host[i].mac[4], host[i].mac[5]);
	}
	vty_out(vty, "========================================\n");
	return CMD_SUCCESS;
}

unsigned int 
dcli_show_ipv6_dhcp_server
(
	struct vty *vty,
	struct dhcp6_show *owned_option	
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0, value = 0, enable;
	struct dhcp6_show option;
	struct iaddr ipAddr[3];

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
	
	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SHOW_DHCP_GLOBAL_CONF);	
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
					DBUS_TYPE_UINT32, &enable,
					DBUS_TYPE_STRING, &(option.domainsearch),					
					DBUS_TYPE_UINT32, &(option.dnsnum),
					
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[0]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[1]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[2]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[3]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[4]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[5]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[6]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[7]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[8]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[9]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[10]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[11]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[12]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[13]),
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[14]),							
					DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[15]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[0]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[1]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[2]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[3]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[4]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[5]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[6]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[7]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[8]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[9]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[10]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[11]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[12]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[13]),
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[14]),							
					DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[15]),	
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[0]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[1]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[2]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[3]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[4]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[5]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[6]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[7]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[8]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[9]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[10]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[11]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[12]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[13]),
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[14]),							
					DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[15]),
						
					DBUS_TYPE_UINT32, &(option.defaulttime),					
					DBUS_TYPE_INVALID)) {
		 owned_option->enable = enable;		
		 owned_option->defaulttime = option.defaulttime;		 
		 owned_option->dnsnum = option.dnsnum;		
		 owned_option->domainsearch = option.domainsearch;		
		 
		 memcpy(owned_option->dnsip[0].iabuf,  ipAddr[0].iabuf, 16);
		 memcpy(owned_option->dnsip[1].iabuf,  ipAddr[1].iabuf, 16);
		 memcpy(owned_option->dnsip[2].iabuf,  ipAddr[2].iabuf, 16);		

		 dbus_message_unref(reply);
		 return CMD_SUCCESS;
	} 
	else {
		
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}

}

DEFUN(show_ipv6_dhcp_server_cmd_func,
	show_ipv6_dhcp_server_cmd,
	"show ipv6 dhcp",
	SHOW_STR
	"Show ip dhcp server entity\n"
	"Specify pool name begins with char, and name length no more than 20 characters\n"
)
{
	unsigned int ret = 0, op_ret = 0, j;	
	struct dhcp6_show owned_option;	
	char *ipv6_dns[3]= {NULL, NULL, NULL};		
	memset(&owned_option, 0, sizeof(struct dhcp6_show));		
	ret = dcli_show_ipv6_dhcp_server(vty, &owned_option);
	 
	
	vty_out(vty, "===============================================\n");
	vty_out(vty, "          dhcp server global configure\n");
	vty_out(vty, "dhcp server is %s\n", (owned_option.enable) ? "enable" : "disable");
	
	if(owned_option.domainsearch){
		vty_out(vty, "dhcp server global domain name is : %s\n", owned_option.domainsearch);		
	}
	vty_out(vty, "ipv6 dhcp server dns : ");
	for(j = 0; j < owned_option.dnsnum;++j){          
		 ipv6_dns[j] =malloc(IPV6_MAX_LEN) ;
		 if(NULL == ipv6_dns[j]){
		 	vty_out(vty,"access memory fail\n");
		 }
	 	inet_ntop(AF_INET6, owned_option.dnsip[j].iabuf, ipv6_dns[j] , INET6_ADDRSTRLEN);
          	vty_out(vty, "%s", ipv6_dns[j]);          
      }
      vty_out(vty, "\n");	
	vty_out(vty, "dhcp server global lease time is : %u seconds\n", owned_option.defaulttime? owned_option.defaulttime : 86400);
	
	vty_out(vty,"===============================================\n");

	for(j = 0; j < owned_option.dnsnum;++j){
		if(NULL != ipv6_dns[j]){
			free(ipv6_dns[j]);
		}
	}	

	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

unsigned int
dcli_show_ipv6_pool
(
	struct vty *vty,
	unsigned int mode,
	unsigned int index,
	struct dhcp6_pool_show** poolshow,
	unsigned int *num
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	struct dhcp6_pool_show* pool = NULL; 
	unsigned int     i = 0, j = 0,ret = 0, count=0, pool_count = 0;
	char *tmpstr = NULL;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SHOW_IP_POOL_CONF);	
	dbus_error_init(&err);
	dbus_message_append_args(query, 
						     DBUS_TYPE_UINT32, &mode,
						     DBUS_TYPE_UINT32, &index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"--------------> %s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}		
		return CMD_SUCCESS;
	 }
	 dbus_message_iter_init(reply,&iter); 	
	 dbus_message_iter_get_basic(&iter,&ret);
 	
	 if (!ret) {
		dbus_message_iter_next(&iter);  
		dbus_message_iter_get_basic(&iter, &pool_count); 
		pool = malloc(sizeof(struct dhcp6_pool_show)*pool_count);
		memset(pool, 0, sizeof(struct dhcp6_pool_show)*pool_count);
		dbus_message_iter_next(&iter);  
		dbus_message_iter_recurse(&iter,&iter_array);

		for (i = 0; i < pool_count; i++) {
			DBusMessageIter iter_struct;
			DBusMessageIter iter_sub_array;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);			  

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].poolname));
			dbus_message_iter_next(&iter_struct); 

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].domain_name));
			dbus_message_iter_next(&iter_struct); 

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option52[0]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option52[1]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option52[2]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option52[3]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option52[4]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option52[5]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option52[6]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option52[7]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].option_adr_num));
			dbus_message_iter_next(&iter_struct);			
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].dnsip[0]));
			dbus_message_iter_next(&iter_struct);			
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].dnsip[1]));
			dbus_message_iter_next(&iter_struct); 
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].dnsip[2]));
			dbus_message_iter_next(&iter_struct); 
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].dnsnum));
			dbus_message_iter_next(&iter_struct); 
			
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].defaulttime));
			dbus_message_iter_next(&iter_struct); 	

			dbus_message_iter_get_basic(&iter_struct, &(pool[i].sub_count));

			count = pool[i].sub_count;
			dbus_message_iter_next(&iter_struct); 
		
			dbus_message_iter_get_basic(&iter_struct, &(pool[i].interfacename));
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);	

			pool[i].ipv6_subnet = malloc(sizeof(struct dhcp6_sub)*count);
			memset(pool[i].ipv6_subnet, 0, sizeof(struct dhcp6_sub)*count);
			
			for (j = 0; j < count; j++){
				
				DBusMessageIter iter_sub_struct;
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				
				dbus_message_iter_get_basic(&iter_sub_struct,&(pool[i].ipv6_subnet[j].range_low_ip));		
				dbus_message_iter_next(&iter_sub_struct);
				
				dbus_message_iter_get_basic(&iter_sub_struct,&(pool[i].ipv6_subnet[j].range_high_ip));			
				dbus_message_iter_next(&iter_sub_struct);
				
				
				dbus_message_iter_get_basic(&iter_sub_struct,&(pool[i].ipv6_subnet[j].prefix_length));
				dbus_message_iter_next(&iter_sub_struct);

				dbus_message_iter_next(&iter_sub_array);
			}			  	  
			dbus_message_iter_next(&iter_array);

		}/*for*/
	}  /*if*/
	//*num = pool_count;
	//*poolshow = pool;
	for (i = 0; i < pool_count; i ++) {		
			vty_out(vty, "===============================================================\n");
			vty_out(vty, "               dhcp server ip pool configure\n");
			vty_out(vty, "ipv6 pool name %s\n", pool[i].poolname);
			if (pool[i].domain_name) {
				vty_out(vty, "dhcp server domain name is : %s\n", pool[i].domain_name); 	
			}
	
			vty_out(vty, "dhcp server option52 is : \n");
	
			for(j = 0; j < pool[i].option_adr_num; j++){
				vty_out(vty, "%s\n", pool[i].option52[j]);
			}		
	
			vty_out(vty, "dhcp server dns ip is :");
			for (j = 0; j < pool[i].dnsnum; j++) {				
					vty_out(vty, " %s ",  pool[i].dnsip[j]);
			}
			vty_out(vty, "\n");
			vty_out(vty, "dhcp server global lease time is : %u seconds\n", pool[i].defaulttime ? pool[i].defaulttime : 86400);
	
			if(pool[i].interfacename != NULL && pool[i].interfacename[0] != '\0'){
				vty_out(vty, "binded by interface name : %s\n", pool[i].interfacename);
			}
			for (j = 0; j < pool[i].sub_count; j++) {
			
				vty_out(vty, "ipv6 range low %s\n", pool[i].ipv6_subnet[j].range_low_ip);
					
				vty_out(vty, "ipv6 range high %s\n", pool[i].ipv6_subnet[j].range_high_ip);
				
				vty_out(vty, "ipv6 range prefix length %d\n", pool[i].ipv6_subnet->prefix_length);
			}
			vty_out(vty, "===============================================================\n");
		}

	dbus_message_unref(reply);
	 return CMD_SUCCESS;		 
}

DEFUN(show_ipv6_pool_cmd_func,
	show_ipv6_pool_cmd,
	"show ipv6 pool",
	CONFIG_STR
	"Show ip pool entity\n"
	"Specify pool name begins with char, and name length no more than 20 characters\n"
)
{
	unsigned int i = 0, j = 0, ret = 0, op_ret = 0, mode = 0, index = 0, count = 0;	
	struct dhcp6_pool_show* poolshow;	
	
	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;
	}
	else if((POOLV6_NODE == vty->node) || (HANSI_POOLV6_NODE == vty->node) || (LOCAL_HANSI_POOLV6_NODE  == vty->node)){
		mode = 1;		
		index = (unsigned int *)(vty->index_sub);
	}
	
	ret = dcli_show_ipv6_pool(vty, mode, index, &poolshow, &count);
#if 0
	for (i = 0; i < count; i ++) {		
		vty_out(vty, "===============================================================\n");
		vty_out(vty, "              dhcp server ip pool configure\n");
		vty_out(vty, "ipv6 pool name %s\n", poolshow[i].poolname);
		if (poolshow[i].domain_name) {
			vty_out(vty, "dhcp server domain name is : %s\n", poolshow[i].domain_name); 	
		}

		vty_out(vty, "dhcp server option52 is : \n");

		for(j = 0; j < poolshow[i].option_adr_num; j++){
			vty_out(vty, "%s\n", poolshow[i].option52[j]);
		}		

		vty_out(vty, "dhcp server dns ip is :");
		for (j = 0; j < poolshow[i].dnsnum; j++) {				
				vty_out(vty, " %s ",  poolshow[i].dnsip[j]);
		}
		vty_out(vty, "\n");
		vty_out(vty, "dhcp server global lease time is : %u seconds\n", poolshow[i].defaulttime ? poolshow[i].defaulttime : 86400);

		if(poolshow[i].interfacename != NULL && poolshow[i].interfacename[0] != '\0'){
			vty_out(vty, "binded by interface name  : %s\n", poolshow[i].interfacename);
		}
		for (j = 0; j < poolshow[i].sub_count; j++) {
		
			vty_out(vty, "ipv6 range low %s\n", poolshow[i].ipv6_subnet[j].range_low_ip);
				
			vty_out(vty, "ipv6 range high %s\n", poolshow[i].ipv6_subnet[j].range_high_ip);
			
			vty_out(vty, "ipv6 range prefix length %d\n", poolshow[i].ipv6_subnet->prefix_length);
		}
		vty_out(vty, "===============================================================\n");
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
dcli_create_ipv6_pool_name
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
	
	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_CREATE_POOL_NODE);
	
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
			if (del) {
			}
			else {
				if(CONFIG_NODE == vty->node) {
					vty->node = POOLV6_NODE;
					vty->index_sub= (void *)index;
					/*vty_out(vty, "create pool index is %d \n", vty->index);*/
				}else if (HANSI_NODE == vty->node){
					vty->node = HANSI_POOLV6_NODE;
					vty->index_sub= (void *)index;
				} else if (LOCAL_HANSI_NODE == vty->node){
					vty->node = LOCAL_HANSI_POOLV6_NODE;
					vty->index_sub= (void *)index;					
				} else {
					vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
					return CMD_WARNING;
				}
			}
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}else {
				if (del) {			
					vty_out (vty, "delete ip pool fail \n");
				}else if(2 == op_ret){
					op_ret = dcli_config_ipv6_pool_name(poolName, vty);
					if(op_ret)
				    vty_out (vty, "enter ip pool fail \n");
				}
				else {
					vty_out (vty, "create ip pool fail \n");
				}

				dbus_message_unref(reply);
				return CMD_SUCCESS;
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

DEFUN(create_ipv6_pool_name_cmd_func,
	create_ipv6_pool_name_cmd,
	"ipv6 pool POOLNAME",
	CONFIG_STR
	"Create ipv6 pool entity\n"
	"Specify pool name begins with char, and name length no more than 20 characters\n"
)
{
	char* poolName = NULL;
	char *ptr = NULL;
	unsigned int nameSize = 0;
	int ret = 0;
	unsigned int op_ret = 0;

	/*printf("before parse_vlan_name %s length %d\n",argv[1],strlen(argv[1]));*/
	if(NULL == (poolName = (char*)malloc(ALIAS_NAME_SIZE))){
		vty_out(vty, "create ip pool failed\n");
		return CMD_WARNING;
	}
	memset(poolName, 0, ALIAS_NAME_SIZE);

	nameSize = strlen(argv[0]);
	if (nameSize >= ALIAS_NAME_SIZE) {
		vty_out(vty, "pool name is too long\n");
		free(poolName);
		poolName = NULL;	
		return CMD_WARNING;
	}
	memcpy(poolName, argv[0], nameSize);

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
	//poolName[nameSize+1] = '\0';
	ret = dcli_create_ipv6_pool_name(0, poolName, vty);
	if (!ret) {
		free(poolName);
		poolName = NULL;
		return CMD_SUCCESS;
	}
	else {
		free(poolName);
		poolName = NULL;	
		return CMD_WARNING;
	}
}

DEFUN(delete_ipv6_pool_name_cmd_func,
	delete_ipv6_pool_name_cmd,
	"no ipv6 pool POOLNAME",
	CONFIG_STR
	"Create ipv6 pool entity\n"
	"Specify pool name begins with char, and name length no more than 20 characters\n"
)
{
	char* poolName = NULL;
	char *ptr = NULL;
	unsigned int nameSize = 0;
	int ret = 0;
	unsigned int op_ret = 0;

	/*printf("before parse_vlan_name %s length %d\n",argv[1],strlen(argv[1]));*/
	poolName = (char*)malloc(ALIAS_NAME_SIZE);
	if(NULL == poolName){
		vty_out(vty, "create ip pool failed\n");
		return CMD_WARNING;
	}
	memset(poolName, 0, ALIAS_NAME_SIZE);

	nameSize = strlen(argv[0]);
	if (nameSize >= ALIAS_NAME_SIZE) {
		vty_out(vty, "pool name is too long\n");
		free(poolName);
		poolName = NULL;	
		return CMD_WARNING;
	}
	memcpy(poolName, argv[0], nameSize);

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
	
	//poolName[nameSize+1] = '\0';
	ret = dcli_create_ipv6_pool_name(1, poolName, vty);
	if (!ret) {
		free(poolName);
		poolName = NULL;
		return CMD_SUCCESS;
	}
	else {
		free(poolName);
		poolName = NULL;	
		return CMD_WARNING;
	}
}

unsigned int
dcli_config_ipv6_pool_name
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
	
	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_ENTRY_POOL_NODE);
	
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
				vty->node = POOLV6_NODE;
				vty->index_sub= (void*)index;
			}else if (HANSI_NODE == vty->node){
					vty->node = HANSI_POOLV6_NODE;
					vty->index_sub= (void *)index;
				} else if (LOCAL_HANSI_NODE == vty->node){
					vty->node = LOCAL_HANSI_POOLV6_NODE;
					vty->index_sub= (void *)index;					
				}else {
				vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
				return CMD_WARNING;
			}
			dbus_message_unref(reply);
			return CMD_SUCCESS;
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
DEFUN(config_ipv6_pool_name_cmd_func,
	config_ipv6_pool_name_cmd,
	"config ipv6 pool POOLNAME",
	CONFIG_STR
	"Config ipv6 pool entity\n"
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
	ret = dcli_config_ipv6_pool_name(poolName, vty);
	
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

/*supf add for distribute hansi show running*/
char * 
dcli_dhcp6_show_running_hansi_cfg
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
	
	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SHOW_RUNNING_HANSI_CFG);

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

char * 
dcli_dhcp_ipv6_show_running_cfg2
(
	int slot_id
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	char *showStr = NULL;
	char *tmp = NULL;
	unsigned int len = 0;
	int ret = 1;

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SHOW_RUNNING_CFG);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) {
		len = strlen(showStr);
		if (!(tmp = malloc(len + 1))) {
			dbus_message_unref(reply);
			return NULL;

		}
		memset(tmp, 0, len + 1);
		memcpy(tmp, showStr, len);
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

DEFUN(show_dhcpv6_running_config_cmd,
	show_dhcpv6_running_config,
	"show dhcpv6 running config",	
	"Config dhcpv6 debugging close\n"
	"Dynamic Host Configuration Protocol\n"
	"running config\n"	
	"dhcpv6 Configuration\n"
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

	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SHOW_RUNNING_CFG);

	dbus_error_init(&err);

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
	} 
	else {
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

/**********************************************************************************
 *  get_slot_id_by_ifname_v6
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

int get_slot_id_by_ifname_v6(const char *ifname)
{
	int slotnum = -1;
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

	/* radio */
	else if (0 == strncmp(ifname, "r", 1)) {
		for (i = 0; i < strlen(ifname); i++) {
			/*use '-' to make sure this radio is local board or remote board */
			if (tmp[i] == '-') {
				count++;
			}			
		}
		
		if (2 == count) {	/*local board*/
			slotnum = dcli_dhcp_get_board_slot_id_v6();
		} else if(3 == count) {	/*remote board*/
			sscanf(ifname, "r%d-%*d-%*d-%d.%*d", &slotnum);
		}
	}

	#if 1
	/* wlanl */
	else if(0 == strncmp(ifname, "wlanl", 5)) {
		sscanf(ifname, "wlanl%d-%*d-%*d", &slotnum);
	}

	else if ((0 == strncmp(ifname, "wlan", 4)) && (strncmp(ifname, "wlanl", 5))) {
		sscanf(ifname, "wlan%d-%*d-%*d", &slotnum);
	}
	#else
	/* wlan */
	else if (0 == strncmp(ifname, "wlan", 4)) {
		for (i = 0; i < strlen(ifname); i++) {
			if(tmp[i] == '-') {
				count++;
			}
		}
		
		if (1 == count) {	/*local board*/
			slotnum = dcli_dhcp_get_board_slot_id_v6();
		} else if (2 == count) {	/*remote board*/
			sscanf(ifname, "wlan%d-%*d-%*d", &slotnum);
		}
	}
	#endif

	#if 1
	/* ebrl */
	else if (0 == strncmp(ifname, "ebrl", 4)) {
		sscanf(ifname, "ebrl%d-%*d-%*d", &slotnum);
	}

	/* ebr */
	else if ((0 == strncmp(ifname, "ebr", 3)) && (strncmp(ifname, "ebrl", 4))) {
		sscanf(ifname, "ebr%d-%*d-%*d", &slotnum);
	}
	
	#else
	/* ebr */
	else if (0 == strncmp(ifname, "ebr", 3)) {
		for (i = 0; i < strlen(ifname); i++) {
			if (tmp[i] == '-') {
				count++;
			}
		}
		if (1 == count) {	/*local board*/
			slotnum = dcli_dhcp_get_board_slot_id_v6();
		} else if (2 == count) {	/*remote board*/
			sscanf(ifname, "ebr%d-%*d-%*d", &slotnum);
		}
	}
	#endif
	
	return slotnum;
}


/**********************************************************************************
 *  dcli_dhcp_get_board_slot_id_v6
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
int dcli_dhcp_get_board_slot_id_v6(void)
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
 *  is_local_board_interface_v6
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
unsigned int is_local_board_interface_v6(const char *ifname)
{
	/*********************hanhui upgrade***************
#define SIOCGIFPRIVFLAGS 0x89b0
	***************************************************/
#define SIOCGIFPRIVFLAGS 0x89b1
	/**************************end*********************/

#define IFF_RPA 0x20

	int sock = -1;
	int board_slot = -1;
	int slot_id = -1;
	struct ifreq tmp;

	if (NULL == ifname) {
		return -1;
	}

	/* ve */
	if (0 == strncmp(ifname, "ve", 2)) {
		board_slot = dcli_dhcp_get_board_slot_id_v6();
		slot_id = get_slot_id_by_ifname_v6(ifname);

		return (board_slot == slot_id);
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

	//printf("%s\n", (IFF_RPA == tmp.ifr_flags) ? "rpa interface" : " ");
	
	return ((tmp.ifr_flags & IFF_RPA) != IFF_RPA);
	
#if 0
	int slot_id = -1;
	int board_slot_id = -1;
	
	slot_id = get_slot_id_by_ifname_v6(ifname);
	board_slot_id = dcli_dhcp_get_board_slot_id_v6();

	return (slot_id == board_slot_id);
#endif	
}

/**********************************************************************************
 *  dcli_dhcpv6_is_distributed
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
unsigned int dcli_dhcpv6_is_distributed(void)
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
 *  dcli_dhcpv6_is_master_board
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
unsigned int dcli_dhcpv6_is_master_board(void)
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
 *  dcli_dhcpv6_is_active_master_board
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
unsigned int dcli_dhcpv6_is_active_master_board(void)
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
 *  dcli_dhcpv6_distributed_whether_allow_dhcp
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
int dcli_dhcpv6_distributed_whether_allow_dhcp(void)
{
	int ret = -1;

	if (IS_DISTRIBUTED != dcli_dhcpv6_is_distributed()) {
		return 0;
	}
	
	ret = dcli_dhcpv6_is_master_board();
	if (1 == ret) {	/* master board */
		/* is active master board : 1(active), 0(not), -1(failed) */
		ret = dcli_dhcpv6_is_active_master_board();
	}
	
	return ret;
}

/**********************************************************************************
 *  dcli_dhcpv6_distributed_process
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
int dcli_dhcpv6_distributed_process(struct vty *vty, int bind, char *ifname)
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

	if (NOT_DISTRIBUTED == dcli_dhcpv6_is_distributed()) {
		return 0;
	}

	if (NULL == ifname) {
		return 1;
	}
	#ifdef	DHCPv6_DEBUG
	vty_out(vty, "%s\n", ifname);
	#endif
	/* whether local broad interface, if local, return */
	if (0 != is_local_board_interface_v6(ifname)){
	//	vty_out(vty, "is local board interface\n");
		
		return 1;
	}
	
	/* get slot */
			send_to_slot = get_slot_id_by_ifname_v6(ifname);
	active_master_slot = dcli_dhcp_get_board_slot_id_v6();

	#ifdef	DHCPv6_DEBUG
	vty_out(vty,"send to slot %d, master slot %d\n", send_to_slot, active_master_slot);
	#endif
#define DHCPv6_SERVER_PORT			(547)
#define DHCPv6_CLIENT_PORT			(546)
#define ADD_PFM_FORWORD_TABLEv6		(10)
#define DEL_PFM_FORWORD_TABLEv6		(11)

#if 0
int 
dcli_communicate_pfm_by_dbus(int opt,  				/*11 add,  12 delete*/
							int opt_para , 			/**/
							unsigned short protocol, 
							char* ifname, 			/* interface name */
							unsigned int src_port,	/* src port */
							unsigned int dest_port, /* dest port */
							int slot,				/* mater slot */
							char* src_ipaddr, 		/* src ip address "all" */
							char* dest_ipaddr,		/* dest ip address "all" */
							unsigned int send_to)	/* send to slot, -1* send to all slot */

#endif

	if (INTERFACE_BIND_POOL == bind) {

		src_port = 0;
		dest_port = DHCPv6_SERVER_PORT;
		ret = dcli_communicate_pfm_by_dbus(ADD_PFM_FORWORD_TABLEv6, 0, protocol, 
											ifname, 
											src_port, dest_port, 
											active_master_slot,
											src_ipaddr, dest_ipaddr,
											send_to_slot);
		if (0 != ret) {
			return 1;
		}
		#if 0
		src_port = 0;
		dest_port = DHCPv6_SERVER_PORT;
		ret = dcli_communicate_pfm_by_dbus(ADD_PFM_FORWORD_TABLEv6, 0, protocol, 
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
		dest_port = DHCPv6_SERVER_PORT;
		ret = dcli_communicate_pfm_by_dbus(DEL_PFM_FORWORD_TABLEv6, 0, protocol, 
											ifname, 
											src_port, dest_port, 
											active_master_slot,
											src_ipaddr, dest_ipaddr,
											send_to_slot);
		if (0 != ret) {
			return 1;
		}
		#if 0
		src_port = DHCPv6_SERVER_PORT;
		dest_port = DHCPv6_SERVER_PORT;
		ret = dcli_communicate_pfm_by_dbus(DEL_PFM_FORWORD_TABLEv6, 0, protocol, 
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


/*0 del 1 add*/
unsigned int
dcli_set_interface_ipv6_pool
(
	char* poolName,
	char* ifname,
	unsigned int add_info,
	struct vty *vty
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
	
	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SET_INTERFACE_POOL);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_STRING, &poolName,							
							DBUS_TYPE_STRING, &ifname,
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

DEFUN(set_interface_ipv6_pool_cmd_func,
	set_interface_ipv6_pool_cmd,
	"ipv6 pool POOLNAME",
	CONFIG_STR
	"Config ipv6 pool entity\n"
	"Specify pool name begins with char, and name length no more than 20 characters\n"
)
{

	char* poolName = NULL, *ifname = NULL;;
	unsigned int nameSize = 0, nodeSave = 0;
	int ret = 0, index = 0;
	unsigned int op_ret = 0;
	int dest_slotid = 0;

	poolName = (char*)malloc(ALIAS_NAME_SIZE);
	ifname = (char*)malloc(ALIAS_NAME_SIZE);
	memset(poolName, 0, ALIAS_NAME_SIZE);
	memset(ifname, 0, ALIAS_NAME_SIZE);
	
	nameSize = strlen(argv[0]);
	if (nameSize >= ALIAS_NAME_SIZE) {
		vty_out(vty, "pool name is too long\n");
		free(poolName);
		free(ifname);
		poolName = NULL;	
		return CMD_WARNING;
	}		
	memcpy(poolName, argv[0], nameSize);
	nameSize = strlen(vlan_eth_port_ifname);
	memcpy(ifname, vlan_eth_port_ifname, nameSize);
	if(strncmp(ifname, "ve", 2) == 0)
	dcli_dhcp_check_ve_interface(ifname);
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

	ret = dcli_set_interface_ipv6_pool(poolName, ifname, 1,vty);
	
	if (!ret) {
		if (dest_slotid != vty->slotindex) {
		dcli_dhcpv6_distributed_process(vty, INTERFACE_BIND_POOL, ifname);
		}
		free(poolName);
		free(ifname);
		poolName = NULL;
		vty_out(vty,"Successfully binding!\n");
		return CMD_SUCCESS;
	}
	else {	
		vty_out(vty,"interface bind pool fail\n");		
		free(poolName);
		free(ifname);
		poolName = NULL;
		return CMD_WARNING;
	}
}

unsigned int
dcli_get_server_v6_state
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

	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE,
									DHCP6_DBUS_METHOD_GET_SERVER_STATE);
	
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

DEFUN(del_interface_ipv6_pool_cmd_func,
	del_interface_ipv6_pool_cmd,
	"no ipv6 pool POOLNAME",
	CONFIG_STR
	"Delete ipvt pool entity\n"
	"Specify pool name begins with char, and name length no more than 20 characters\n"
)
{

	char* poolName = NULL, *ifname = NULL;;
	unsigned int nameSize = 0, nodeSave = 0 ,dest_slotid = 0;
	int ret = 0, index = 0;
	unsigned int op_ret = 0;
	unsigned int state = 0;

	/* check server state */
	dcli_get_server_v6_state(vty, &state);
	if (state) {
		vty_out(vty, "Please first disable dhcp server\n");
		return CMD_WARNING;		
	}

	poolName = (char*)malloc(ALIAS_NAME_SIZE);
	ifname = (char*)malloc(ALIAS_NAME_SIZE);
	memset(poolName, 0, ALIAS_NAME_SIZE);
	memset(ifname, 0, ALIAS_NAME_SIZE);
	
	nameSize = strlen(argv[0]);
	if (nameSize >= ALIAS_NAME_SIZE) {
		vty_out(vty, "pool name is too long\n");
		free(poolName);
		free(ifname);
		poolName = NULL;	
		return CMD_WARNING;
	}
	memcpy(poolName, argv[0], nameSize);
	
	nameSize = strlen(vlan_eth_port_ifname);
	memcpy(ifname, vlan_eth_port_ifname, nameSize);
	if(strncmp(ifname, "ve", 2) == 0)
	dcli_dhcp_check_ve_interface(ifname);
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
	ret = dcli_set_interface_ipv6_pool(poolName, ifname, 0,vty);
	
	if (!ret) {
		if (dest_slotid != vty->slotindex)
		dcli_dhcpv6_distributed_process(vty, INTERFACE_UNBIND_POOL, ifname);
		
		free(poolName);
		free(ifname);
		poolName = NULL;
		return CMD_SUCCESS;
	}
	else {	
		vty_out(vty,"interface unbind pool fail\n");		
		free(poolName);
		free(ifname);
		poolName = NULL;
		return CMD_WARNING;
	}
}

int 
dcli_add_dhcp_pool_ipv6_range
(
	unsigned int add,
	struct iaddr *ipaddrl, 
	struct iaddr *ipaddrh,
	unsigned int prefix_length,
	unsigned int index,
	struct vty *vty
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

	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_ADD_IP_POOL_RANGE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &add,
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[0]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[1]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[2]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[3]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[4]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[5]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[6]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[7]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[8]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[9]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[10]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[11]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[12]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[13]),
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[14]),							
							DBUS_TYPE_BYTE, &(ipaddrl->iabuf[15]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[0]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[1]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[2]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[3]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[4]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[5]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[6]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[7]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[8]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[9]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[10]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[11]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[12]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[13]),
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[14]),							
							DBUS_TYPE_BYTE, &(ipaddrh->iabuf[15]),	
							DBUS_TYPE_UINT32, &prefix_length,
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

DEFUN(add_dhcp_pool_ipv6_range_cmd_func,
	add_dhcp_pool_ipv6_range_cmd,
	"(add|delete) rangev6 IPV6 IPV6 prefix-length <1-16>",
	"Add|Delete dhcp ipv6 range\n"
	"Add dhcp ipv6 range\n"
	"Delete dhcp ipv6 range\n"
	"Range ipv6"
	"Low ip IPV6\n"
	"High ip IPV6\n"
)
{
	struct iaddr ipAddrl, ipAddrh;
	unsigned int ret = 0, add = 0, index = 0, prefix_length = 0;

	memset(&ipAddrh, 0 ,sizeof(struct iaddr));
	memset(&ipAddrl, 0 ,sizeof(struct iaddr));
	index = (unsigned int *)(vty->index_sub);

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
	ret = str2_ipv6_addr((char*)argv[1], &ipAddrl);
	if (!ret) {
		vty_out(vty,"bad command patameter!\n");
		return CMD_WARNING;
	}
	
	ret = str2_ipv6_addr((char*)argv[2], &ipAddrh);
	if (!ret) {
		vty_out(vty,"bad command patameter!\n");
		return CMD_WARNING;
	}
	/* need do it
	if (ipAddrl > ipAddrh) {
		return CMD_WARNING;
	}*/		
	prefix_length = atoi((char *)argv[3]);

	ret = dcli_add_dhcp_pool_ipv6_range(add, &ipAddrl, &ipAddrh, prefix_length,index,vty);
	if (ret) {
		vty_out(vty, "%s ip range fail \n", add ? "delete" : "add");
	}
	
	return CMD_SUCCESS;
}

unsigned int
dcli_set_server_lease_default_ipv6
(
	unsigned int lease_default,	
	unsigned int mode,
	unsigned int index,
	unsigned int del,
	struct vty *vty
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SET_SERVER_LEASE_DEFAULT);
	
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

DEFUN(ipv6_dhcp_server_lease_default_cmd_func,
	ipv6_dhcp_server_lease_default_cmd,
	"ipv6 dhcp server lease-time <60-31536000>",
	CONFIG_STR
	"Ipv6 dhcp server lease-time entity\n"
	"Ipv6 dhcp server lease-time 60-31536000 seconds\n"
	"Specify pool name begins with char, and name length no more than 20 characters\n"
)
{
	unsigned int lease_default = 0, index = 0;
	unsigned int ret = 0, op_ret = 0, mode = 0;

	lease_default = atoi((char*)argv[0]);

	/*mode 0 means global mode, 1 means pool mode */
	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;
	}
	else if((POOLV6_NODE == vty->node) || (HANSI_POOLV6_NODE == vty->node) || (LOCAL_HANSI_POOLV6_NODE  == vty->node)) {
		mode = 1;		
		index = (unsigned int *)(vty->index_sub);
		/*vty_out(vty,"ip pool server domain name sub index is %d\n", index);*/
	}else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_lease_default_ipv6(lease_default, mode, index, 0,vty);
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

DEFUN(no_ipv6_dhcp_server_lease_default_cmd_func,
	no_ipv6_dhcp_server_lease_default_cmd,
	"ipv6 dhcp server lease-default",
	CONFIG_STR
	"Set ipv6 dhcp server lease-time default entity\n"
	"Specify pool name begins with char, and name length no more than 20 characters\n"
)
{
	unsigned int lease_default = 0, index = 0;
	unsigned int ret = 0, op_ret = 0, mode = 0;

	/*mode 0 means global mode, 1 means pool mode */
	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;
	}
	else if((POOLV6_NODE == vty->node) || (HANSI_POOLV6_NODE == vty->node) || (LOCAL_HANSI_POOLV6_NODE  == vty->node)) {
		mode = 1;		
		index = (unsigned int *)(vty->index_sub);
		/*vty_out(vty,"ip pool server domain name sub index is %d\n", index);*/
	}else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_lease_default_ipv6(lease_default, mode, index, 1,vty);
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

unsigned int
dcli_set_server_option52_ipv6
(
	char **ap_via_address,
	unsigned int ipv6num,
	unsigned int mode,
	unsigned int index,
	unsigned int del,
	struct vty *vty
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	int i;

	if (NULL == ap_via_address) {
		return 1;
	}

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
	
	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SET_SERVER_OPTION52);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_STRING, &ap_via_address[0],
							 DBUS_TYPE_STRING, &ap_via_address[1],
							 DBUS_TYPE_STRING, &ap_via_address[2],
							 DBUS_TYPE_STRING, &ap_via_address[3],
							 DBUS_TYPE_STRING, &ap_via_address[4],
							 DBUS_TYPE_STRING, &ap_via_address[5],
							 DBUS_TYPE_STRING, &ap_via_address[6],
							 DBUS_TYPE_STRING, &ap_via_address[7],
							 DBUS_TYPE_UINT32, &ipv6num,
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
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);

		for(i = 0; i < MAX_OPTION52_ADDRESS_LIST; ++i){
			if(ap_via_address[i] != NULL){
				free(ap_via_address[i]);
			}
		}

		if(ap_via_address != NULL){
			free(ap_via_address);
		}	
		return CMD_WARNING;
	}

free_exit:

	for(i = 0; i < MAX_OPTION52_ADDRESS_LIST; ++i){
			if(ap_via_address[i] != NULL){
				free(ap_via_address[i]);
			}
		}

	if(ap_via_address != NULL){
		free(ap_via_address);
	}
	return CMD_SUCCESS;	
}


DEFUN(ipv6_dhcp_server_option52_cmd_func,
	ipv6_dhcp_server_option52_cmd,
	"ipv6 dhcp server option52 IPV6 [IPV6] [IPV6] [IPV6] [IPV6] [IPV6] [IPV6] [IPV6]",
	CONFIG_STR
	"dhcp\n"
	"dhcp server\n"
	"option52"
	"ipv6 address list"
)
{
	char **ap_via_address = NULL;
	unsigned int size = 0, index = 0, len = 0;
	unsigned int ret = 0, op_ret = 0, mode = 0;
	int i, j;

	for(i = 0 ; i < argc ; ++i){
		if(check_ipv6_address(argv[i])){
		 	vty_out(vty, "%% address %s  is Illegal\n", argv[i]);
			 return CMD_WARNING;
		}		
	}

	ap_via_address = (char **)malloc(sizeof(char *) * MAX_OPTION52_ADDRESS_LIST);
	if(NULL == ap_via_address){
		vty_out(vty, "Failure to apply memory resources\n");
		return CMD_WARNING;
	}
	memset(ap_via_address, 0, sizeof(char *) * MAX_OPTION52_ADDRESS_LIST);

	for(i = 0; i < MAX_OPTION52_ADDRESS_LIST; ++i){
		ap_via_address[i] = malloc(IPV6_MAX_LEN);
		if(NULL == ap_via_address[i]){
			break;
		}
		memset(ap_via_address[i], 0 , IPV6_MAX_LEN);
	}

	if(i  <  MAX_OPTION52_ADDRESS_LIST ){
		for(j = 0; j < i; ++j){
			if(ap_via_address[j] != NULL){
				free(ap_via_address[j]);
			}
		}

		if(ap_via_address != NULL){
			free(ap_via_address);
		}
		vty_out(vty, "Failure to apply memory resources\n");
		return CMD_WARNING;
	}
	
	for(i = 0; i < argc ; ++i){
		memcpy(ap_via_address[i], argv[i],  strlen(argv[i]) + 1);
	}
	
	/*mode 0 means global mode, 1 means pool mode */
	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;
	}
	else if((POOLV6_NODE == vty->node) || (HANSI_POOLV6_NODE == vty->node) || (LOCAL_HANSI_POOLV6_NODE  == vty->node)) {
		mode = 1;		
		index = (unsigned int *)(vty->index_sub);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		for(j = 0; j < MAX_OPTION52_ADDRESS_LIST; ++j){
			if(ap_via_address[j] != NULL){
				free(ap_via_address[j]);
			}
		}

		if(ap_via_address != NULL){
			free(ap_via_address);
		}
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_option52_ipv6(ap_via_address, argc , mode, index, 0,vty);
	if (!ret) {
		
		return CMD_SUCCESS;
	}
	else {		
		vty_out(vty, "set dhcp server option52 fail\n");		
		return CMD_WARNING;
	}
}

DEFUN(no_ipv6_dhcp_server_option52_cmd_func,
	no_ipv6_dhcp_server_option52_cmd,
	"no ipv6 dhcp server option52",
	CONFIG_STR
	"Ipv6 dhcp server option52 entity\n"
	"Specify pool name begins with char, and name length no more than 20 characters\n"
)
{
	char **ap_via_address = NULL;
	unsigned int ret = 0, op_ret = 0, mode = 0, index = 0, size = 0;
	int i, j;
	
	ap_via_address = (char **)malloc(sizeof(char *) * MAX_OPTION52_ADDRESS_LIST);
	if(NULL == ap_via_address){
		vty_out(vty, "Failure to apply memory resources\n");
		return CMD_WARNING;
	}
	memset(ap_via_address, 0, sizeof(char *) * MAX_OPTION52_ADDRESS_LIST);

	for(i = 0; i < MAX_OPTION52_ADDRESS_LIST; ++i){
		ap_via_address[i] = malloc(IPV6_MAX_LEN);
		if(NULL == ap_via_address[i]){
			break;
		}
		memset(ap_via_address[i], 0 , IPV6_MAX_LEN);
	}

	if(i < MAX_OPTION52_ADDRESS_LIST){
		for(j = 0; j < i; ++j){
			if(ap_via_address[j] != NULL){
				free(ap_via_address[j]);
			}
		}

		if(ap_via_address != NULL){
			free(ap_via_address);
		}
		vty_out(vty, "Failure to apply memory resources\n");
		return CMD_WARNING;
	}
	
	/*mode 0 means global mode, 1 means pool mode */
	
	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;
	}
	else if((POOLV6_NODE == vty->node) || (HANSI_POOLV6_NODE == vty->node) || (LOCAL_HANSI_POOLV6_NODE  == vty->node)){
		mode = 1;		
		index = (unsigned int *)(vty->index_sub);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);	
		for(j = 0; j < MAX_OPTION52_ADDRESS_LIST; ++j){
			if(ap_via_address[j] != NULL){
				free(ap_via_address[j]);
			}
		}

		if(ap_via_address != NULL){
			free(ap_via_address);
		}
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_option52_ipv6(ap_via_address, 0 , mode, index, 1,vty);
	if (!ret) {		
		
		return CMD_SUCCESS;
	}
	else {
		vty_out(vty, "delete dhcp server option52 fail\n");
		
		return CMD_WARNING;
	}
}

unsigned int
dcli_set_server_domain_search_ipv6
(
	char *domainName,	
	unsigned int mode,
	unsigned int index,
	unsigned int del,
	struct vty *vty
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
	
	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SET_SERVER_DOMAIN_SEARCH);
	
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
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}
}

DEFUN(ipv6_dhcp_server_domain_search_cmd_func,
	ipv6_dhcp_server_domain_search_cmd,
	"ipv6 dhcp server domain-search NAME",
	CONFIG_STR
	"Ipv6 dhcp server domain-search entity\n"
	"Specify pool name begins with char, and name length no more than 20 characters\n"
)
{
	char* domainName = NULL;
	unsigned int nameSize = 0, nodeSave = 0, index = 0;
	unsigned int ret = 0, op_ret = 0, mode = 0;

	//printf("before parse_vlan_name %s length %d\n",argv[1],strlen(argv[1]));
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
	//domainName[nameSize+1] = '\0';
	//mode 0 means global mode, 1 means pool mode 
	
	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;
	}
	else if((POOLV6_NODE == vty->node) || (HANSI_POOLV6_NODE == vty->node) || (LOCAL_HANSI_POOLV6_NODE  == vty->node)){
		mode = 1;		
		index = (unsigned int *)(vty->index_sub);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		if(domainName){
			free(domainName);
			domainName=NULL;
		}
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_domain_search_ipv6(domainName, mode, index, 0,vty);
	if (!ret) {
		if(domainName){
			free(domainName);
			domainName=NULL;
		}
		return CMD_SUCCESS;
	}
	else {
		vty_out(vty,"% Bad parameter,pool %s NOT exists.\n", domainName);		
		return CMD_WARNING;
	}
}

DEFUN(no_ipv6_dhcp_server_domain_search_cmd_func,
	no_ipv6_dhcp_server_domain_search_cmd,
	"no ipv6 dhcp server domain-search",
	CONFIG_STR
	"Ipv6 dhcp server domain entity\n"
	"Specify pool name begins with char, and name length no more than 20 characters\n"
)
{
	char* domainName = NULL;
	unsigned int nameSize = 0, nodeSave = 0, index = 0;
	unsigned int ret = 0, op_ret = 0, mode = 0;

	domainName = (char*)malloc(ALIAS_NAME_SIZE);
	memset(domainName, 0, ALIAS_NAME_SIZE);
	if (nameSize >= ALIAS_NAME_SIZE) {
		vty_out(vty, "domain name is too long\n");
		free(domainName);
		domainName = NULL;	
		return CMD_WARNING;
	}

	
	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;
	}
	else if((POOLV6_NODE == vty->node) || (HANSI_POOLV6_NODE == vty->node) || (LOCAL_HANSI_POOLV6_NODE  == vty->node)){
		mode = 1;		
		index = (unsigned int *)(vty->index_sub);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		if(domainName){
			free(domainName);
			domainName=NULL;
		}
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_domain_search_ipv6(domainName, mode, index, 1,vty);
	if (!ret) {
		if(domainName){
			free(domainName);
			domainName=NULL;
		}
		return CMD_SUCCESS;
	}
	else {
		vty_out(vty,"% Bad parameter,pool %s NOT exists.\n", domainName);		
		return CMD_WARNING;
	}
}

unsigned int
dcli_set_server_name_servers_ipv6
(
	struct iaddr *ipAddr,
	unsigned int ipNum,
	unsigned int mode,
	unsigned int index,
	unsigned int del,
	struct vty *vty
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SET_SERVER_DNS);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &ipNum,							 
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[0]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[1]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[2]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[3]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[4]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[5]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[6]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[7]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[8]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[9]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[10]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[11]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[12]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[13]),
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[14]),	
							DBUS_TYPE_BYTE, &(ipAddr[0].iabuf[15]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[0]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[1]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[2]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[3]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[4]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[5]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[6]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[7]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[8]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[9]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[10]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[11]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[12]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[13]),
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[14]),							
							DBUS_TYPE_BYTE, &(ipAddr[1].iabuf[15]),	
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[0]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[1]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[2]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[3]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[4]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[5]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[6]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[7]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[8]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[9]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[10]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[11]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[12]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[13]),
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[14]),							
							DBUS_TYPE_BYTE, &(ipAddr[2].iabuf[15]),	
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
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	
		return CMD_WARNING;
	}
}

DEFUN(ipv6_dhcp_server_name_servers_cmd_func,
	ipv6_dhcp_server_name_servers_cmd,
	"ipv6 dhcp server name-servers IPV6 [IPV6] [IPV6]",
	CONFIG_STR
	"Ipv6 dhcp server dns entity\n"
	"Specify pool name begins with char, and name length no more than 20 characters\n"
)
{
	struct iaddr ipAddr[3];
	unsigned int ret = 0, op_ret = 0, ipnum = 0, mode = 0, index = 0, i = 0;

	memset(ipAddr, 0, 3*sizeof(struct iaddr));
	ipnum = argc;
	for(;i < ipnum; i++) {
		ret = str2_ipv6_addr((char*)argv[i], &ipAddr[i]);
		if (!ret) {
		vty_out(vty,"bad command patameter!\n");
		return CMD_WARNING;
		}
	}
	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;
	}
	else if((POOLV6_NODE == vty->node) || (HANSI_POOLV6_NODE == vty->node) || (LOCAL_HANSI_POOLV6_NODE  == vty->node)) {
		mode = 1;		
		index = (unsigned int *)(vty->index_sub);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_name_servers_ipv6(ipAddr, ipnum, mode, index, 0,vty);
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

DEFUN(no_ipv6_dhcp_server_name_servers_cmd_func,
	no_ipv6_dhcp_server_name_servers_cmd,
	"no ipv6 dhcp server name-servers",
	CONFIG_STR
	"No ipv6 dhcp server dns entity\n"
	"Specify pool name begins with char, and name length no more than 20 characters\n"
)
{
	struct iaddr ipAddr[3];
	unsigned int ret = 0, op_ret = 0, ipnum = 0, mode = 0, index = 0, i = 0;

	if((CONFIG_NODE == vty->node) || (HANSI_NODE  == vty->node) || (LOCAL_HANSI_NODE  == vty->node)) {
		mode = 0;
	}
	else if((POOLV6_NODE == vty->node) || (HANSI_POOLV6_NODE == vty->node) || (LOCAL_HANSI_POOLV6_NODE  == vty->node)) {
		mode = 1;		
		index = (unsigned int *)(vty->index_sub);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_name_servers_ipv6(ipAddr, ipnum, mode, index, 1,vty);
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

/*
int 
dcli_add_dhcp_static_host
(
	unsigned int ipaddr, 
	unsigned char* mac,
	unsigned int add
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError 		err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned int	op_ret = 0, ret = 0;

	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_ADD_STATIC_HOST);
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

DEFUN(add_dhcp_ipv6_static_host_cmd_func,
	add_dhcp_ipv6_static_host_cmd,
	"ipv6 dhcp static A.B.C.D HH:HH:HH:HH:HH:HH",
	CONFIG_STR	
	"ADD dhcp static host"
	"Config dhcp server"
	"Static host ip"
	"Static host mac"
)
{
	unsigned int ipAddr = 0;
	unsigned int ret = 0, index = 0;
	unsigned char macAddr[MAC_ADDRESS_LEN];
	
	ipAddr = dcli_ip2ulong((char*)argv[0]);
	
	memset(&macAddr, 0, MAC_ADDRESS_LEN);	
	ret = parse_mac_addr((char *)argv[1], &macAddr);

	ret = dcli_add_dhcp_static_host(ipAddr, macAddr, 1);
	if (ret) {
		vty_out(vty, "add ip dhcp static host fail\n");
	}
	return CMD_SUCCESS;
}

DEFUN(delete_dhcp_ipv6_static_host_cmd_func,
	delete_dhcp_ipv6_static_host_cmd,
	"no ipv6 dhcp static A.B.C.D HH:HH:HH:HH:HH:HH",
	CONFIG_STR	
	"DELETE dhcp static host"
	"Config dhcp server"
	"Static host ip"
	"Static host mac"

)
{
	unsigned int ipAddr = 0;
	unsigned int ret = 0, index = 0;
	unsigned char macAddr[MAC_ADDRESS_LEN];
	
	ipAddr = dcli_ip2ulong((char*)argv[0]);
	
	memset(&macAddr, 0, MAC_ADDRESS_LEN);	
	ret = parse_mac_addr((char *)argv[1], &macAddr);

	ret = dcli_add_dhcp_static_host(ipAddr, macAddr, 0);
	if (ret) {
		vty_out(vty, "delete ip dhcp static host fail\n");
	}
	return CMD_SUCCESS;
}

unsigned int
dcli_set_server_option43
(
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
	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SET_SERVER_VEO);
	
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

DEFUN(ipv6_dhcp_server_option43_cmd_func,
	ipv6_dhcp_server_option43_cmd,
	"ipv6 dhcp server option43 HEX",
	CONFIG_STR
	"Ip dhcp server option43 entity\n"
	"Specify pool name begins with char, and name length no more than 20 characters\n"
)
{
	char* veo = NULL;
	unsigned int size = 0, index = 0, len = 0;
	unsigned int ret = 0, op_ret = 0, mode = 0;

	size = strlen(argv[0]);
	veo = (char*)malloc(size + 1);
	memset(veo, 0, (size + 1));
	
	memcpy(veo, argv[0], size);

	if(CONFIG_NODE == vty->node) {
		mode = 0;
	}
	else if(POOLV6_NODE == vty->node) {
		mode = 1;		
		index = (unsigned int *)(vty->index);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_option43(veo, mode, index, 0);
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		vty_out(vty, "set dhcp server option43 fail\n", veo);		
		return CMD_WARNING;
	}
}

DEFUN(no_ipv6_dhcp_server_option43_cmd_func,
	no_ipv6_dhcp_server_option43_cmd,
	"no ipv6 dhcp server option43",
	CONFIG_STR
	"Ip dhcp server option43 entity\n"
	"Specify pool name begins with char, and name length no more than 20 characters\n"
)
{
	char* veo = NULL;
	unsigned int ret = 0, op_ret = 0, mode = 0, index = 0, size = 0;

	size = strlen(argv[0]);
	veo = (char*)malloc(size + 1);
	memset(veo, 0, (size + 1));
	
	memcpy(veo, argv[0], size);

	
	if(CONFIG_NODE == vty->node) {
		mode = 0;
	}
	else if(POOLV6_NODE == vty->node){
		mode = 1;		
		index = (unsigned int *)(vty->index);
	}
	else {		
		vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
		return CMD_WARNING;
	}
	
	ret = dcli_set_server_option43(veo, mode, index, 1);
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		vty_out(vty, "delete dhcp server option43 fail\n", veo);		
		return CMD_WARNING;
	}
}
*/
unsigned int
dcli_set_dhcpv6_debug_state
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
	
	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SET_DEBUG_STATE);
	
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
	dhcpv6_debug_enable_cmd,
	"debug dhcpv6 (all|info|error|debug)",
	"Add debug dhcp Information\n"
	"Dhcp server\n" 
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
	ret = dcli_set_dhcpv6_debug_state(vty, debug_type, debug_enable);
	
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

DEFUN(dhcp_debug_disable_cmd_func,
	dhcpv6_debug_disable_cmd,
	"no debug dhcpv6 (all|info|error|debug)",
	"Delete old Configuration\n"
	"Config dhcp debugging close\n"
	"Dhcp server\n"
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

	ret = dcli_set_dhcpv6_debug_state(vty, debug_type, isEnable);
	
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

unsigned int
dcli_set_dhcpv6_server_enable
(
	unsigned int enable,struct vty *vty
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;

	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	get_slotid_index(vty, &indextmp, &slot_id, &localid);

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);

	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SET_SERVER_ENABLE);
	
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

DEFUN(ipv6_dhcp_server_enable_cmd_func,
	ipv6_dhcp_server_enable_cmd,
	"ipv6 dhcp server (enable|disable)",
	"Ipv6 address of portal\n"
	"Dynamic Host Configuration Protocol\n"
	"Dhcpv6 server\n"
	"Ipv6 dhcp server enable\n"
	"Ipv6 dhcp server disable\n"
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

	ret = dcli_set_dhcpv6_server_enable(isEnable,vty);
	
	if (!ret) {
		return CMD_SUCCESS;
	}
	else {
		return CMD_WARNING;
	}
}

int 
dcli_dhcp_ipv6_show_running_cfg
(
	struct vty *vty
)
{	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
      int ret = 1;
	char _tmpstr[64];
	
	query = dbus_message_new_method_call(DHCP6_DBUS_BUSNAME, 
									DHCP6_DBUS_OBJPATH, 
									DHCP6_DBUS_INTERFACE, 
									DHCP6_DBUS_METHOD_SHOW_RUNNING_CFG);

	dbus_error_init(&err);

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

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) {
	
		
		memset(_tmpstr, 0, 64);
		sprintf(_tmpstr, BUILDING_MOUDLE, "DHCP6 SERVER");
		
		vtysh_add_show_string(_tmpstr);
		vtysh_add_show_string(showStr);
        ret = 0;
	} 
	else {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return ret;	
}

void 
dcli_dhcp_ipv6_init
(
	void
)  
{
	install_node (&poolv6_node, dcli_dhcp_ipv6_show_running_cfg,"POOLV6_NODE");
	install_default(POOLV6_NODE);	
	install_element(CONFIG_NODE, &create_ipv6_pool_name_cmd);	
	install_element(CONFIG_NODE, &dhcpv6_debug_enable_cmd);
	install_element(CONFIG_NODE, &dhcpv6_debug_disable_cmd);
	install_element(CONFIG_NODE, &delete_ipv6_pool_name_cmd);
	install_element(CONFIG_NODE, &config_ipv6_pool_name_cmd);
	install_element(CONFIG_NODE, &show_dhcpv6_running_config);
//	install_element(CONFIG_NODE, &add_dhcp_static_host_cmd);	
//	install_element(CONFIG_NODE, &delete_dhcp_static_host_cmd);	
	install_element(CONFIG_NODE, &ipv6_dhcp_server_enable_cmd);	
	install_element(POOLV6_NODE, &add_dhcp_pool_ipv6_range_cmd);
	/*
	install_element(POOLV6_NODE, &ipv6_dhcp_server_option43_cmd);
	install_element(POOLV6_NODE, &no_ipv6_dhcp_server_option43_cmd);
*/
	install_element(CONFIG_NODE, &ipv6_dhcp_server_lease_default_cmd);
	install_element(POOLV6_NODE, &ipv6_dhcp_server_lease_default_cmd);
	install_element(CONFIG_NODE, &no_ipv6_dhcp_server_lease_default_cmd);
	install_element(POOLV6_NODE, &no_ipv6_dhcp_server_lease_default_cmd);
	install_element(CONFIG_NODE, &ipv6_dhcp_server_option52_cmd);
	install_element(POOLV6_NODE, &ipv6_dhcp_server_option52_cmd);
	install_element(CONFIG_NODE, &no_ipv6_dhcp_server_option52_cmd);
	install_element(POOLV6_NODE, &no_ipv6_dhcp_server_option52_cmd);
	install_element(CONFIG_NODE, &ipv6_dhcp_server_domain_search_cmd);
	install_element(POOLV6_NODE, &ipv6_dhcp_server_domain_search_cmd);
	install_element(CONFIG_NODE, &no_ipv6_dhcp_server_domain_search_cmd);
	install_element(POOLV6_NODE, &no_ipv6_dhcp_server_domain_search_cmd);
	install_element(CONFIG_NODE, &ipv6_dhcp_server_name_servers_cmd);
	install_element(POOLV6_NODE, &ipv6_dhcp_server_name_servers_cmd);
	install_element(CONFIG_NODE, &no_ipv6_dhcp_server_name_servers_cmd);
	install_element(POOLV6_NODE, &no_ipv6_dhcp_server_name_servers_cmd);
	/*
	install_element(CONFIG_NODE, &show_dhcp_lease_by_ip_cmd);
	install_element(CONFIG_NODE, &show_dhcp_lease_by_ip_diff_cmd);
	install_element(CONFIG_NODE, &show_dhcp_lease_by_ip_mask_cmd);
	install_element(CONFIG_NODE, &show_dhcp_lease_by_mac_cmd);
*/
	
	install_element(CONFIG_NODE, &save_dhcp_lease_ipv6_cmd);
	install_element(CONFIG_NODE, &show_dhcp6_lease_cmd);
	install_element(CONFIG_NODE, &show_ipv6_dhcp_server_cmd);
	install_element(POOLV6_NODE, &show_ipv6_pool_cmd);
	install_element(CONFIG_NODE, &show_ipv6_pool_cmd);
//	install_element(CONFIG_NODE, &show_ip_dhcp_static_cmd);
	install_element(INTERFACE_NODE, &set_interface_ipv6_pool_cmd);
	install_element(INTERFACE_NODE, &del_interface_ipv6_pool_cmd);

	install_element(HANSI_NODE, &create_ipv6_pool_name_cmd);				/* pool node */		
	install_element(HANSI_NODE, &delete_ipv6_pool_name_cmd);
	install_element(HANSI_NODE, &config_ipv6_pool_name_cmd);
	install_element(HANSI_NODE, &show_dhcpv6_running_config);
	install_element(HANSI_NODE, &dhcpv6_debug_enable_cmd);
	install_element(HANSI_NODE, &dhcpv6_debug_disable_cmd);
//	install_element(CONFIG_NODE, &add_dhcp_static_host_cmd);	
//	install_element(CONFIG_NODE, &delete_dhcp_static_host_cmd);	
	install_element(HANSI_NODE, &ipv6_dhcp_server_enable_cmd);

	install_node(&hansi_poolv6_node,NULL,"HANSI_POOLV6_NODE");	
	install_default(HANSI_POOLV6_NODE);
	install_element(HANSI_POOLV6_NODE, &add_dhcp_pool_ipv6_range_cmd);
	/*
	install_element(POOLV6_NODE, &ipv6_dhcp_server_option43_cmd);
	install_element(POOLV6_NODE, &no_ipv6_dhcp_server_option43_cmd);
*/
	install_element(HANSI_NODE, &ipv6_dhcp_server_lease_default_cmd);
	install_element(HANSI_POOLV6_NODE, &ipv6_dhcp_server_lease_default_cmd);
	install_element(HANSI_NODE, &no_ipv6_dhcp_server_lease_default_cmd);
	install_element(HANSI_POOLV6_NODE, &no_ipv6_dhcp_server_lease_default_cmd);
	install_element(HANSI_NODE, &ipv6_dhcp_server_option52_cmd);
	install_element(HANSI_POOLV6_NODE, &ipv6_dhcp_server_option52_cmd);
	install_element(HANSI_NODE, &no_ipv6_dhcp_server_option52_cmd);
	install_element(HANSI_POOLV6_NODE, &no_ipv6_dhcp_server_option52_cmd);
	install_element(HANSI_NODE, &ipv6_dhcp_server_domain_search_cmd);
	install_element(HANSI_POOLV6_NODE, &ipv6_dhcp_server_domain_search_cmd);
	install_element(HANSI_NODE, &no_ipv6_dhcp_server_domain_search_cmd);
	install_element(HANSI_POOLV6_NODE, &no_ipv6_dhcp_server_domain_search_cmd);
	install_element(HANSI_NODE, &ipv6_dhcp_server_name_servers_cmd);
	install_element(HANSI_POOLV6_NODE, &ipv6_dhcp_server_name_servers_cmd);
	install_element(HANSI_NODE, &no_ipv6_dhcp_server_name_servers_cmd);
	install_element(HANSI_POOLV6_NODE, &no_ipv6_dhcp_server_name_servers_cmd);
	/*
	install_element(CONFIG_NODE, &show_dhcp_lease_by_ip_cmd);
	install_element(CONFIG_NODE, &show_dhcp_lease_by_ip_diff_cmd);
	install_element(CONFIG_NODE, &show_dhcp_lease_by_ip_mask_cmd);
	install_element(CONFIG_NODE, &show_dhcp_lease_by_mac_cmd);
*/
	
	install_element(HANSI_NODE, &save_dhcp_lease_ipv6_cmd);
	install_element(HANSI_NODE, &show_dhcp6_lease_cmd);
	install_element(HANSI_NODE, &show_ipv6_dhcp_server_cmd);
	install_element(HANSI_POOLV6_NODE, &show_ipv6_pool_cmd);
	install_element(HANSI_NODE, &show_ipv6_pool_cmd);

	install_node(&local_hansi_poolv6_node,NULL,"LOCAL_HANSI_POOLV6_NODE");
	install_default(LOCAL_HANSI_POOLV6_NODE);
	install_element(LOCAL_HANSI_NODE, &create_ipv6_pool_name_cmd);				/* pool node */	
	install_element(LOCAL_HANSI_NODE, &delete_ipv6_pool_name_cmd);
	install_element(LOCAL_HANSI_NODE, &config_ipv6_pool_name_cmd);
	install_element(LOCAL_HANSI_NODE, &show_dhcpv6_running_config);
//	install_element(CONFIG_NODE, &add_dhcp_static_host_cmd);	
//	install_element(CONFIG_NODE, &delete_dhcp_static_host_cmd);	
	install_element(LOCAL_HANSI_NODE, &ipv6_dhcp_server_enable_cmd);	
	install_element(LOCAL_HANSI_POOLV6_NODE, &add_dhcp_pool_ipv6_range_cmd);
	/*
	install_element(POOLV6_NODE, &ipv6_dhcp_server_option43_cmd);
	install_element(POOLV6_NODE, &no_ipv6_dhcp_server_option43_cmd);
*/
	install_element(LOCAL_HANSI_NODE, &ipv6_dhcp_server_lease_default_cmd);
	install_element(LOCAL_HANSI_POOLV6_NODE, &ipv6_dhcp_server_lease_default_cmd);
	install_element(LOCAL_HANSI_NODE, &no_ipv6_dhcp_server_lease_default_cmd);
	install_element(LOCAL_HANSI_POOLV6_NODE, &no_ipv6_dhcp_server_lease_default_cmd);
	install_element(LOCAL_HANSI_NODE, &ipv6_dhcp_server_option52_cmd);
	install_element(LOCAL_HANSI_POOLV6_NODE, &ipv6_dhcp_server_option52_cmd);
	install_element(LOCAL_HANSI_NODE, &no_ipv6_dhcp_server_option52_cmd);
	install_element(LOCAL_HANSI_POOLV6_NODE, &no_ipv6_dhcp_server_option52_cmd);
	install_element(LOCAL_HANSI_NODE, &ipv6_dhcp_server_domain_search_cmd);
	install_element(LOCAL_HANSI_POOLV6_NODE, &ipv6_dhcp_server_domain_search_cmd);
	install_element(LOCAL_HANSI_NODE, &no_ipv6_dhcp_server_domain_search_cmd);
	install_element(LOCAL_HANSI_POOLV6_NODE, &no_ipv6_dhcp_server_domain_search_cmd);
	install_element(LOCAL_HANSI_NODE, &ipv6_dhcp_server_name_servers_cmd);
	install_element(LOCAL_HANSI_POOLV6_NODE, &ipv6_dhcp_server_name_servers_cmd);
	install_element(LOCAL_HANSI_NODE, &no_ipv6_dhcp_server_name_servers_cmd);
	install_element(LOCAL_HANSI_POOLV6_NODE, &no_ipv6_dhcp_server_name_servers_cmd);
	/*
	install_element(CONFIG_NODE, &show_dhcp_lease_by_ip_cmd);
	install_element(CONFIG_NODE, &show_dhcp_lease_by_ip_diff_cmd);
	install_element(CONFIG_NODE, &show_dhcp_lease_by_ip_mask_cmd);
	install_element(CONFIG_NODE, &show_dhcp_lease_by_mac_cmd);
*/
	
	install_element(LOCAL_HANSI_NODE, &save_dhcp_lease_ipv6_cmd);
	install_element(LOCAL_HANSI_NODE, &show_dhcp6_lease_cmd);
	install_element(LOCAL_HANSI_NODE, &show_ipv6_dhcp_server_cmd);
	install_element(LOCAL_HANSI_POOLV6_NODE, &show_ipv6_pool_cmd);
	install_element(LOCAL_HANSI_NODE, &show_ipv6_pool_cmd);
}
#ifdef __cplusplus
}
#endif


