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
* dcli_dhcp_snp.c
*
*
* CREATOR:
*		jinpc@autelan.com
*
* DESCRIPTION:
*		dhcp snooping common for DCLI module.
*
* DATE:
*		06/04/2009	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.17 $	
*******************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************
*	head files														*
**********************************************************/
#include <net/if.h>
#include <zebra.h>
#include <dbus/dbus.h>
#include "command.h"
				
#include <sysdef/npd_sysdef.h>
#include <sysdef/returncode.h>
#include <dbus/npd/npd_dbus_def.h>
#include <dbus/dhcp/dhcpsnp_dbus_def.h>
#include <util/npd_list.h>

#include "if.h"
#include "dcli_system.h"
#include "dcli_main.h"

#include "dcli_dhcp_snp.h"

/*********************************************************
*	global variable define											*
**********************************************************/


/*********************************************************
*	extern variable												*
**********************************************************/
extern DBusConnection *dcli_dbus_connection;
extern char vlan_eth_port_ifname [INTERFACE_NAMSIZ];

/*********************************************************
*	functions define												*
**********************************************************/

/*
#define DCLI_DHCPSNP_DEBUG
*/
int dhcp_snp_get_slotid_index(struct vty *vty, int *index, int *slotid, int *localid)
{
	if(vty->node == CONFIG_NODE){
		*index = 0;

	}else if((HANSI_NODE == vty->node) || (INTERFACE_NODE == vty->node)){
		*index = (int)vty->index;
		if (localid) {
			*localid = vty->local;
		}
		if (slotid) {
			*slotid = vty->slotindex;
			#ifdef DCLI_DHCPSNP_DEBUG
			vty_out(vty, "hansi node: get_slotid_index: slot id %d\n", *slotid);
			#endif
		}		
	}
	else if((LOCAL_HANSI_NODE == vty->node) || (INTERFACE_NODE == vty->node)) {
		*index = (int)vty->index;
		if (localid) {
			*localid = vty->local;
		}
		if (slotid) {
			*slotid = vty->slotindex;
			#ifdef DCLI_DHCPSNP_DEBUG			
			vty_out(vty, "local hansi node: get_slotid_index: slot id %d\n", *slotid);	
			#endif			
		}		
	}
	return (0);
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
/*
DBusConnection *dcli_dbus_connection = NULL;
dhcp_snp_reinitDbusConnection(&dcli_dbus_connection, vty);
*/
void dhcp_snp_reinitDbusConnection(DBusConnection **dcli_dbus_connection, struct vty *vty)
{
	int localid = 1, slot_id = HostSlotId, indextmp = 0;
	dhcp_snp_get_slotid_index(vty, &indextmp, &slot_id, &localid);

	ReInitDbusConnection(dcli_dbus_connection, slot_id, distributFag);
}


/**********************************************************************************
 *  dcli_dhcp_snp_convert_ifname_check
 * 
 * 	check interfacename
 *
 *	INPUT:
 *		ifname	- input interface name
 *		
 *	
 *	OUTPUT:
 *		name - ouput interface name
 *
 * 	RETURN:
 *		0  - 
 *		
 **********************************************************************************/
int dcli_dhcp_snp_convert_ifname_check(struct vty *vty, char *ifname)
{
	int slotid = 0, vrrpid = 0, local_flag = 0 ;
	unsigned int slodid_info = 0,vrrpid_info = 0, num = 0;
	char *endptr = NULL;
	char *tmp = NULL;
	if ((!ifname) || (!vty)) {
		return -1;
	}

	if (!distributFag) {
		return 0;
	}
	
	if (CONFIG_NODE == vty->node) {
		if ((slotid = dcli_dhcp_get_board_slot_id()) < 0) {
			return -1;
		}
		vrrpid = 0;
		local_flag = 1;

	} else if ((HANSI_NODE == vty->node) || (LOCAL_HANSI_NODE == vty->node)) {
		slotid = (int)vty->slotindex;
		vrrpid = (int)vty->index;
		local_flag = (LOCAL_HANSI_NODE == vty->node);
	}

	if((0 == strncmp(ifname,"wlan",4)) || (0 == strncmp(ifname,"ebr",3))){
		if(0 == strncmp(ifname,"ebr",3))
			slodid_info = strtoul(ifname+3,&endptr,10);
		if(0 == strncmp(ifname,"wlan",4))
			slodid_info = strtoul(ifname+4,&endptr,10);
		if('-' == endptr[0]){
			tmp = endptr + 1;
			vrrpid_info = strtoul(tmp,&endptr,10);
			if('-' == endptr[0]){
				tmp = endptr + 1;
				num = strtoul(tmp,&endptr,10);
				if('\0' == endptr[0] || '\n' == endptr[0]){
					if(slodid_info == slotid && vrrpid_info == vrrpid)
					{
						if(local_flag){
							if((0 == strncmp(ifname,"wlan",4))){
								memset(ifname,0,IF_NAMESIZE);
								sprintf(ifname, "wlanl%d-%d-%d", slodid_info, vrrpid_info, num);
							}
							if((0 == strncmp(ifname,"ebr",3))){
								memset(ifname,0,IF_NAMESIZE);
								sprintf(ifname, "ebrl%d-%d-%d", slodid_info, vrrpid_info, num);
							}
						}
					
						return 1;
					}
					else
						return -1;
				}else{
					return -1;
				}
			}else{
				return -1;
			}
			
		}else if('\0' == endptr[0] || '\n' == endptr[0]){
				return 2;
			}else{
				return -1;
			}
	}	

	return 1;
}


/**********************************************************************************
 *  dcli_dhcp_snp_convert_ifname
 * 
 * 	convert name
 *
 *	INPUT:
 *		ifname	- input interface name
 *		
 *	
 *	OUTPUT:
 *		name - ouput interface name
 *
 * 	RETURN:
 *		0  - 
 *		
 **********************************************************************************/
int dcli_dhcp_snp_convert_ifname(struct vty *vty, char *ifname, char *name)
{
	int slotid = 0, vrrpid = 0, num = 0, local_flag = 0;
	char tmp[IF_NAMESIZE];
	if ((!ifname) || (!name) || (!vty)) {
		return -1;
	}

	if (!distributFag) {
		return 0;
	}
	
	if (CONFIG_NODE == vty->node) {
		if ((slotid = dcli_dhcp_get_board_slot_id()) < 0) {
			return -1;
		}
		vrrpid = 0;
		local_flag = 1;

	} else if ((HANSI_NODE == vty->node) || (LOCAL_HANSI_NODE == vty->node)) {
		slotid = (int)vty->slotindex;
		vrrpid = (int)vty->index;
		local_flag = (LOCAL_HANSI_NODE == vty->node);
	}

	/* hansi node or config node */
	if (!local_flag) {
		if (0 == strncmp(ifname, "wlan", 4)) {
			sscanf(ifname, "wlan%d", &num);	
			sprintf(name, "wlan%d-%d-%d", slotid, vrrpid, num);
		} else if (0 == strncmp(ifname, "ebr", 3)) {
			sscanf(ifname, "ebr%d", &num);			
			sprintf(name, "ebr%d-%d-%d", slotid, vrrpid, num);
		} else {
			sprintf(name, "%s", ifname);
		}

	/* local-hansi node */
	} else {
		if (0 == strncmp(ifname, "wlan", 4)) {
			sscanf(ifname, "wlan%d", &num);				
			sprintf(name, "wlanl%d-%d-%d", slotid, vrrpid, num);
		} else if (0 == strncmp(ifname, "ebr", 3)) {
			sscanf(ifname, "ebr%d", &num);			
			sprintf(name, "ebrl%d-%d-%d", slotid, vrrpid, num);
		} else {
			sprintf(name, "%s", ifname);
		}
	}
	return 0;
}

/**********************************************************************************
 * dcli_dhcp_snp_mac_ascii_to_hex
 * 
 * 	mac address in Hex format
 *
 *	INPUT:
 *		chaddr	- string of mac
 *		size	- macAddr buffer size
 *	
 *	OUTPUT:
 *		macAddr - mac address will be returned back
 *
 * 	RETURN:
 *		-1 - if mac address buffer size too small.
 *		-2 - illegal character found.
 *		-3 - parameter is null
 *		0  - if no error occur
 *
 *	NOTATION:
 *		mac address is a ASCII code formatted MAC address, such as 
 *		"001122334455" stands for mac address 00:11:22:33:44:55 or 00-11-22-33-44-55
 *		
 **********************************************************************************/
int dcli_dhcp_snp_mac_ascii_to_hex
(
    unsigned char *chaddr,
    unsigned char *macAddr,
    unsigned int  size
)
{
    unsigned char *sysMac = NULL;
	unsigned char cur = 0;
	unsigned char value = 0;
	unsigned int i = 0;

	if (!chaddr || !macAddr) {
		return -3;
	}
	if (size < DCLI_DHCP_SNP_MAC_ADD_LEN) {
		return -1;
	}
	
	sysMac = chaddr;
    for (i = 0; i < 12; i++ ) {
        cur = sysMac[i];
		if((cur >= '0') &&(cur <='9')) {
			value = cur - '0';
		}
		else if((cur >= 'A') &&(cur <='F')) {
			value = cur - 'A';
			value += 0xa;
		}
		else if((cur >= 'a') &&(cur <='f')) {
			value = cur - 'a';
			value += 0xa;
		}
		else { /* illegal character found*/
			return -2;
		}

		if(0 == i % 2) {
			macAddr[i / 2] = value;
		}
		else {
			macAddr[i/2] <<= 4;
			macAddr[i/2] |= value;
		}
	}

	return 0;
}


/**********************************************************************************
 * dcli_dhcp_snp_remoteid_string_legal_check
 * 
 *	INPUT:
 *		str	- remote-id string user entered from vtysh. 
 *		len	- Length of remote-id string 
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		1	--string too long
 *		2	--illegal char on head of string
 *		3	--unsupported char in string
 *		DHCP_SNP_RETURN_CODE_OK 		-- success
 *		DHCP_SNP_RETURN_CODE_ERROR		-- fail
 **********************************************************************************/
unsigned int dcli_dhcp_snp_remoteid_string_legal_check
(
	char *str,
	unsigned int len
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int i = DCLI_DHCP_SNP_INIT_0;
	char c = DCLI_DHCP_SNP_INIT_0;

	if ((NULL == str) || (len == 0)) {
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	if(len >= DCLI_DHCP_SNP_REMOTEID_STR_LEN){
		return 1;
	}

	c = str[0];
	if(	(c == '_')||
		(c <= 'z' && c >= 'a') ||
		(c <= 'Z' && c >= 'A')
	  ){
		ret = DHCP_SNP_RETURN_CODE_OK;
	}else {
		return 2;
	}

	for (i = 1; i <= len - 1; i++) {
		c = str[i];
		if ((c >= '0' && c <= '9')||
			(c <= 'z' && c >= 'a')||
		    (c <= 'Z' && c >= 'A')||
		    (c == '_')
		    ){
			continue;
		}else {
			ret = 3;
			break;
		}
	}

	return ret;
}

/**********************************************************************************
 * dcli_dhcp_snp_circuitid_string_legal_check
 * 
 *	INPUT:
 *		str	- circuit-id string user entered from vtysh. 
 *		len	- Length of circuit-id string 
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		1	--string too long or too short
 *		2	--illegal char on head of string
 *		3	--unsupported char in string
 *		DHCP_SNP_RETURN_CODE_OK 		-- success
 *		DHCP_SNP_RETURN_CODE_ERROR		-- fail
 **********************************************************************************/
unsigned int dcli_dhcp_snp_circuitid_string_legal_check
(
	char *str,
	unsigned int len
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int i = DCLI_DHCP_SNP_INIT_0;
	char c = DCLI_DHCP_SNP_INIT_0;

	if ((NULL == str) || (len == 0)) {
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	if(len >= DCLI_DHCP_SNP_REMOTEID_STR_LEN || len < 3) {
		return 1;
	}

	c = str[0];
	if(	(c == '_')||
		(c <= 'z' && c >= 'a') ||
		(c <= 'Z' && c >= 'A')
	  ){
		ret = DHCP_SNP_RETURN_CODE_OK;
	}else {
		return 2;
	}

	for (i = 1; i <= len - 1; i++) {
		c = str[i];
		if ((c >= '0' && c <= '9')||
			(c <= 'z' && c >= 'a')||
		    (c <= 'Z' && c >= 'A')||
		    (c == '_')
		    ){
			continue;
		}else {
			ret = 3;
			break;
		}
	}

	return ret;
}

/**********************************************************************************
 * dcli_dhcp_snp_check_global_status()
 *	DESCRIPTION:
 *		check DHCP-Snooping enable/disable global status 
 *
 *	INPUTS:
 *		NULL
 *
 *	OUTPUTS:
 *		unsigned char* stats		- check result
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *		DHCP_SNP_RETURN_CODE_ERROR		- fail
***********************************************************************************/
unsigned int dcli_dhcp_snp_check_global_status
(
	unsigned char* stats
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned char status = DCLI_DHCP_SNP_INIT_0;
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_CHECK_GLOBAL_STATUS);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	if (dbus_message_get_args(reply, &err,
								DBUS_TYPE_UINT32, &ret,
								DBUS_TYPE_BYTE, &status,
								DBUS_TYPE_INVALID)) 
	{
		if (DHCP_SNP_RETURN_CODE_OK == ret) {
			*stats = status;
		}else{
			ret = DHCP_SNP_RETURN_CODE_ERROR;
		}
	} else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);

	return ret;
}

/**********************************************************************************
 * dcli_dhcp_snp_wan_check_global_status()
 *	DESCRIPTION:
 *		check DHCP-Snooping enable/disable global status 
 *
 *	INPUTS:
 *		NULL
 *
 *	OUTPUTS:
 *		unsigned char* stats		- check result
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *		DHCP_SNP_RETURN_CODE_ERROR		- fail
***********************************************************************************/
unsigned int dcli_dhcp_snp_wan_check_global_status
(
	struct vty *vty,
	unsigned char* stats
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned char status = DCLI_DHCP_SNP_INIT_0;
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;

	DBusConnection *dcli_dbus_connection = NULL;
	dhcp_snp_reinitDbusConnection(&dcli_dbus_connection, vty);

	query = dbus_message_new_method_call(
										DHCPSNP_DBUS_BUSNAME,
										DHCPSNP_DBUS_OBJPATH,
										DHCPSNP_DBUS_INTERFACE,
										DHCPSNP_DBUS_METHOD_CHECK_GLOBAL_STATUS);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	if (dbus_message_get_args(reply, &err,
								DBUS_TYPE_UINT32, &ret,
								DBUS_TYPE_BYTE, &status,
								DBUS_TYPE_INVALID)) 
	{
		if (DHCP_SNP_RETURN_CODE_OK == ret) {
			*stats = status;
		}else{
			ret = DHCP_SNP_RETURN_CODE_ERROR;
		}
	} else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);

	return ret;
}

/**********************************************************************************
 * dcli_dhcp_snp_config_port()
 *	DESCRIPTION:
 *		config DHCP-Snooping port's trust mode 
 *
 *	INPUTS:
 *	 	unsigned short vlanId,			- vlan id
 *	 	unsigned char slot_no,			- slot no of port
 *	 	unsigned char local_port_no,	- port no of port
 *		unsigned char trust_mode		- trust mode
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *		DHCP_SNP_RETURN_CODE_ERROR		- fail
***********************************************************************************/
unsigned int dcli_dhcp_snp_config_port
(
	unsigned short vlanId,
	unsigned char slot_no,
	unsigned char local_port_no,
	unsigned char trust_mode
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,
								NPD_DBUS_DHCP_SNP_OBJPATH,
								NPD_DBUS_DHCP_SNP_INTERFACE,
								NPD_DBUS_DHCP_SNP_METHOD_SET_ETHPORT_TRUST_MODE);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16, &vlanId,
							 DBUS_TYPE_BYTE, &slot_no,
							 DBUS_TYPE_BYTE, &local_port_no,
							 DBUS_TYPE_BYTE, &trust_mode,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		dbus_message_unref(reply);
		return ret;
	}
	else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
}

/**********************************************************************************
 * dcli_dhcp_snp_show_bind_table()
 *	DESCRIPTION:
 *		show DHCP-Snooping bind table
 *
 *	INPUTS:
 *		struct vty* vty
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		CMD_SUCCESS		- success
 *		CMD_WARNING		- fail
***********************************************************************************/
unsigned int dcli_dhcp_snp_show_bind_table
(
	struct vty* vty
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	DBusMessageIter iter;
	DBusMessageIter	iter_array;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int product_id = PRODUCT_ID_NONE;
	unsigned int record_count = DCLI_DHCP_SNP_INIT_0;
	unsigned char macaddr[6] = {0};
	int rc = DCLI_DHCP_SNP_INIT_0;
	int j = DCLI_DHCP_SNP_INIT_0;

	unsigned int  bind_type = DCLI_DHCP_SNP_INIT_0;
	unsigned int   ip_addr = DCLI_DHCP_SNP_INIT_0;
	unsigned short vlanId = DCLI_DHCP_SNP_INIT_0;
	unsigned int   slot_no = DCLI_DHCP_SNP_INIT_0;
	unsigned int   port_no = DCLI_DHCP_SNP_INIT_0;
	unsigned int   lease_time = DCLI_DHCP_SNP_INIT_0;

	dbus_error_init(&err);
	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SHOW_BIND_TABLE);
	/* No params */ 
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "Failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	dbus_message_iter_init(reply, &iter);

	dbus_message_iter_get_basic(&iter, &ret);
	if (DHCP_SNP_RETURN_CODE_OK == ret) {
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &record_count);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &product_id);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter, &iter_array);

		vty_out(vty, "DHCP-Snooping is enabled.\n");
		vty_out(vty, "The client binding table for all trusted ports.\n");
		vty_out(vty, "Type : D--Dynamic , S--Static\n");
		vty_out(vty, "==========================================================\n");
		vty_out(vty, "%-4s %-15s %-17s %-8s %-4s %-5s\n",
					 "Type", "IP Address", "MAC Address", "Lease", "Vlan", "Port");
		vty_out(vty, "==== =============== ================= ======== ==== =====\n");

		for (j = 0; j < record_count; j++) {
			memset(macaddr, 0, 6);
			bind_type  = DCLI_DHCP_SNP_INIT_0;
			ip_addr    = DCLI_DHCP_SNP_INIT_0;
			vlanId     = DCLI_DHCP_SNP_INIT_0;
			slot_no    = DCLI_DHCP_SNP_INIT_0;
			port_no    = DCLI_DHCP_SNP_INIT_0;
			lease_time = DCLI_DHCP_SNP_INIT_0;

			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array, &iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &bind_type);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[5]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &ip_addr);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &vlanId);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &slot_no);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &port_no);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &lease_time);

			dbus_message_iter_next(&iter_array);
			
			vty_out(vty, "%-4s ", bind_type ? " S" : " D");
			vty_out(vty, "%3d.%3d.%3d.%3d ", (ip_addr>>24) & 0xff, (ip_addr>>16) & 0xff, (ip_addr>>8) & 0xff, ip_addr & 0xff);
			vty_out(vty, "%02x:%02x:%02x:%02x:%02x:%02x ",
						macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
			if (0 == bind_type) {	/* dynamic item */
				vty_out(vty, "%8d ", lease_time);
			}else {					/* static item */
				vty_out(vty, "%-8s ", "infinite");
			}
			vty_out(vty, "%-4d ", vlanId);
			vty_out(vty, "%2d/%-3d", slot_no, port_no);
			vty_out(vty, "\n");

		}
		vty_out(vty, "==========================================================\n");
	}
	else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret){
		vty_out(vty, "%% DHCP-Snooping not enabled global.\n");
	}
	else if (DHCP_SNP_RETURN_CODE_ERROR == ret) {
		vty_out(vty, "%% Show DHCP-Snooping bind table error!\n");	
	}

	dbus_message_unref(reply);

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * dcli_dhcp_snp_show_bind_table()
 *	DESCRIPTION:
 *		show DHCP-Snooping bind table
 *
 *	INPUTS:
 *		struct vty* vty
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		CMD_SUCCESS		- success
 *		CMD_WARNING		- fail
***********************************************************************************/
unsigned int dcli_dhcp_snp_show_wan_bind_table
(
	struct vty* vty
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	DBusMessageIter iter;
	DBusMessageIter	iter_array;
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int record_count = DCLI_DHCP_SNP_INIT_0;
	unsigned char macaddr[6] = {0};
	int j = DCLI_DHCP_SNP_INIT_0;
	unsigned char bind_state = DCLI_DHCP_SNP_INIT_0;
	unsigned int  bind_type = DCLI_DHCP_SNP_INIT_0;
	unsigned int   ip_addr = DCLI_DHCP_SNP_INIT_0;
	unsigned short vlanId = DCLI_DHCP_SNP_INIT_0;
	unsigned int   lease_time = DCLI_DHCP_SNP_INIT_0;
	unsigned int   ifindex = DCLI_DHCP_SNP_INIT_0;
	//char ifname[IF_NAMESIZE];
	char *ifname_ep=NULL;
	DBusConnection *dcli_dbus_connection = NULL;
	dhcp_snp_reinitDbusConnection(&dcli_dbus_connection, vty);
	query = dbus_message_new_method_call(
								DHCPSNP_DBUS_BUSNAME,
								DHCPSNP_DBUS_OBJPATH,
								DHCPSNP_DBUS_INTERFACE,
								DHCPSNP_DBUS_METHOD_SHOW_WAN_BIND_TABLE);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);
	if (DHCP_SNP_RETURN_CODE_ENABLE_GBL == ret) {
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &record_count);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter, &iter_array);
		vty_out(vty, "DHCP-Snooping is enabled.\n");
		vty_out(vty, "Type : D--Dynamic , S--Static\n");
		vty_out(vty, "===============================================================\n");
		vty_out(vty, "%-4s %-15s %-17s %-8s %-5s %-9s\n",
					 "Type", "IP Address", "MAC Address", "Lease", "Vlan", "Interface");
		vty_out(vty, "==== =============== ================= ======== ===== =========\n");
		for (j = 0; j < record_count; j++) {
			memset(macaddr, 0, 6);
			bind_state = DCLI_DHCP_SNP_INIT_0;
			bind_type  = DCLI_DHCP_SNP_INIT_0;
			ip_addr    = DCLI_DHCP_SNP_INIT_0;
			vlanId     = DCLI_DHCP_SNP_INIT_0;
			lease_time = DCLI_DHCP_SNP_INIT_0;
			ifindex    = DCLI_DHCP_SNP_INIT_0;
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array, &iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &bind_type);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &bind_state);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[5]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &vlanId);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &ip_addr);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &lease_time);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &ifname_ep);
			dbus_message_iter_next(&iter_array);
			vty_out(vty, "%-4s ", bind_type ? " S" : " D");
			vty_out(vty, "%3d.%3d.%3d.%3d ", (ip_addr>>24) & 0xff, (ip_addr>>16) & 0xff, (ip_addr>>8) & 0xff, ip_addr & 0xff);
			vty_out(vty, "%02x:%02x:%02x:%02x:%02x:%02x ",
						macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
			if (0 == bind_type) {	/* dynamic item */
				vty_out(vty, "%-8d ", lease_time);
			}else {					/* static item */
				vty_out(vty, "%-8s ", "infinite");
			}
			vty_out(vty, "%-5d ", vlanId);
			
			vty_out(vty, "%s",  ifname_ep);
			vty_out(vty, "\n");
		}
		vty_out(vty, "===============================================================\n");
	}
	else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret){
		vty_out(vty, "%% DHCP-Snooping not enabled global.\n");
	}
	else if (DHCP_SNP_RETURN_CODE_ERROR == ret) {
		vty_out(vty, "%% Show DHCP-Snooping bind table error!\n");	
	}
	dbus_message_unref(reply);
	return DHCP_SNP_RETURN_CODE_OK;
}
/**********************************************************************************
 * dcli_dhcp_snp_show_static_bind_table()
 *	DESCRIPTION:
 *		show dhcp-snooping static binding items in db
 *
 *	INPUTS:
 *		struct vty* vty
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		CMD_SUCCESS		- success
 *		CMD_WARNING		- fail
***********************************************************************************/
unsigned int dcli_dhcp_snp_show_static_bind_table
(
	struct vty* vty
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	DBusMessageIter iter;
	DBusMessageIter	iter_array;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int product_id = PRODUCT_ID_NONE;
	unsigned int record_count = DCLI_DHCP_SNP_INIT_0;
	unsigned char macaddr[6] = {0};
	int rc = DCLI_DHCP_SNP_INIT_0;
	int j = DCLI_DHCP_SNP_INIT_0;

	unsigned int  bind_type = DCLI_DHCP_SNP_INIT_0;
	unsigned int   ip_addr = DCLI_DHCP_SNP_INIT_0;
	unsigned short vlanId = DCLI_DHCP_SNP_INIT_0;
	unsigned int   slot_no = DCLI_DHCP_SNP_INIT_0;
	unsigned int   port_no = DCLI_DHCP_SNP_INIT_0;
	unsigned int   lease_time = DCLI_DHCP_SNP_INIT_0;

	dbus_error_init(&err);
	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SHOW_STATIC_BIND_TABLE);
	/* No params */ 
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "Failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	dbus_message_iter_init(reply, &iter);

	dbus_message_iter_get_basic(&iter, &ret);
	if (DHCP_SNP_RETURN_CODE_OK == ret) {
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &record_count);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &product_id);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter, &iter_array);

		vty_out(vty, "DHCP-Snooping is enabled.\n");
		vty_out(vty, "The client binding table for all trusted ports.\n");
		vty_out(vty, "Type : D--Dynamic , S--Static\n");
		vty_out(vty, "==========================================================\n");
		vty_out(vty, "%-4s %-15s %-17s %-8s %-4s %-5s\n",
					 "Type", "IP Address", "MAC Address", "Lease", "Vlan", "Port");
		vty_out(vty, "==== =============== ================= ======== ==== =====\n");

		for (j = 0; j < record_count; j++) {
			memset(macaddr, 0, 6);
			bind_type  = DCLI_DHCP_SNP_INIT_0;
			ip_addr    = DCLI_DHCP_SNP_INIT_0;
			vlanId     = DCLI_DHCP_SNP_INIT_0;
			slot_no    = DCLI_DHCP_SNP_INIT_0;
			port_no    = DCLI_DHCP_SNP_INIT_0;
			lease_time = DCLI_DHCP_SNP_INIT_0;

			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array, &iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &bind_type);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[5]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &ip_addr);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &vlanId);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &slot_no);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &port_no);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &lease_time);

			dbus_message_iter_next(&iter_array);
			
			vty_out(vty, "%-4s ", bind_type ? " S" : " D");
			vty_out(vty, "%3d.%3d.%3d.%3d ", (ip_addr>>24) & 0xff, (ip_addr>>16) & 0xff, (ip_addr>>8) & 0xff, ip_addr & 0xff);
			vty_out(vty, "%02x:%02x:%02x:%02x:%02x:%02x ",
						macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
			vty_out(vty, "%-8s ", "infinite");
			vty_out(vty, "%-4d ", vlanId);
			vty_out(vty, "%2d/%-3d", slot_no, port_no);
			vty_out(vty, "\n");
		}
		vty_out(vty, "==========================================================\n");
	}
	else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret){
		vty_out(vty, "%% DHCP-Snooping not enabled global.\n");
	}
	else if (DHCP_SNP_RETURN_CODE_ERROR == ret) {
		vty_out(vty, "%% Show DHCP-Snooping static bind table error!\n");	
	}

	dbus_message_unref(reply);

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * dcli_dhcp_snp_show_static_bind_table_by_vlan()
 *	DESCRIPTION:
 *		show dhcp-snooping static binding items by vlan in db
 *
 *	INPUTS:
 *		struct vty* vty
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		CMD_SUCCESS		- success
 *		CMD_WARNING		- fail
***********************************************************************************/
unsigned int dcli_dhcp_snp_show_static_bind_table_by_vlan
(
	struct vty* vty,
	unsigned short vlanId
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	DBusMessageIter iter;
	DBusMessageIter	iter_array;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int product_id = PRODUCT_ID_NONE;
	unsigned int record_count = DCLI_DHCP_SNP_INIT_0;
	unsigned char macaddr[6] = {0};
	int rc = DCLI_DHCP_SNP_INIT_0;
	int j = DCLI_DHCP_SNP_INIT_0;

	unsigned int  bind_type = DCLI_DHCP_SNP_INIT_0;
	unsigned int   ip_addr = DCLI_DHCP_SNP_INIT_0;
	unsigned short tmp_vlanId = DCLI_DHCP_SNP_INIT_0;
	unsigned int   slot_no = DCLI_DHCP_SNP_INIT_0;
	unsigned int   port_no = DCLI_DHCP_SNP_INIT_0;
	unsigned int   lease_time = DCLI_DHCP_SNP_INIT_0;

	dbus_error_init(&err);
	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SHOW_STATIC_BIND_TABLE_BY_VLAN);

	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT16, &vlanId,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "Failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	dbus_message_iter_init(reply, &iter);

	dbus_message_iter_get_basic(&iter, &ret);
	if (DHCP_SNP_RETURN_CODE_OK == ret) {
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &record_count);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &product_id);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter, &iter_array);

		vty_out(vty, "DHCP-Snooping is enabled.\n");
		vty_out(vty, "The client binding table for all trusted ports.\n");
		vty_out(vty, "Type : D--Dynamic , S--Static\n");
		vty_out(vty, "==========================================================\n");
		vty_out(vty, "%-4s %-15s %-17s %-8s %-4s %-5s\n",
					 "Type", "IP Address", "MAC Address", "Lease", "Vlan", "Port");
		vty_out(vty, "==== =============== ================= ======== ==== =====\n");

		for (j = 0; j < record_count; j++) {
			memset(macaddr, 0, 6);
			bind_type  = DCLI_DHCP_SNP_INIT_0;
			ip_addr    = DCLI_DHCP_SNP_INIT_0;
			tmp_vlanId = DCLI_DHCP_SNP_INIT_0;
			slot_no    = DCLI_DHCP_SNP_INIT_0;
			port_no    = DCLI_DHCP_SNP_INIT_0;
			lease_time = DCLI_DHCP_SNP_INIT_0;

			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array, &iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &bind_type);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[5]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &ip_addr);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &tmp_vlanId);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &slot_no);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &port_no);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &lease_time);

			dbus_message_iter_next(&iter_array);
			
			vty_out(vty, "%-4s ", bind_type ? " S" : " D");
			vty_out(vty, "%3d.%3d.%3d.%3d ", (ip_addr>>24) & 0xff, (ip_addr>>16) & 0xff, (ip_addr>>8) & 0xff, ip_addr & 0xff);
			vty_out(vty, "%02x:%02x:%02x:%02x:%02x:%02x ",
						macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
			vty_out(vty, "%-8s ", "infinite");
			vty_out(vty, "%-4d ", tmp_vlanId);
			vty_out(vty, "%2d/%-3d", slot_no, port_no);
			vty_out(vty, "\n");
		}
		vty_out(vty, "==========================================================\n");
	}
	else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret){
		vty_out(vty, "%% DHCP-Snooping not enabled global.\n");
	}
	else if (DHCP_SNP_RETURN_CODE_ERROR == ret) {
		vty_out(vty, "%% Show DHCP-Snooping static bind table error!\n");	
	}

	dbus_message_unref(reply);

	return DHCP_SNP_RETURN_CODE_OK;
}


/**********************************************************************************
 * dcli_dhcp_snp_show_static_bind_table_by_ethport()
 *	DESCRIPTION:
 *		show dhcp-snooping static binding items by ethport in db
 *
 *	INPUTS:
 *		struct vty* vty
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		CMD_SUCCESS		- success
 *		CMD_WARNING		- fail
***********************************************************************************/
unsigned int dcli_dhcp_snp_show_static_bind_table_by_ethport
(
	struct vty* vty,
	unsigned char local_slot_no,
	unsigned char local_port_no
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	DBusMessageIter iter;
	DBusMessageIter	iter_array;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int product_id = PRODUCT_ID_NONE;
	unsigned int record_count = DCLI_DHCP_SNP_INIT_0;
	unsigned char macaddr[6] = {0};
	int rc = DCLI_DHCP_SNP_INIT_0;
	int j = DCLI_DHCP_SNP_INIT_0;

	unsigned int  bind_type = DCLI_DHCP_SNP_INIT_0;
	unsigned int   ip_addr = DCLI_DHCP_SNP_INIT_0;
	unsigned short tmp_vlanId = DCLI_DHCP_SNP_INIT_0;
	unsigned int   slot_no = DCLI_DHCP_SNP_INIT_0;
	unsigned int   port_no = DCLI_DHCP_SNP_INIT_0;
	unsigned int   lease_time = DCLI_DHCP_SNP_INIT_0;

	dbus_error_init(&err);
	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SHOW_STATIC_BIND_TABLE_BY_ETHPORT);

	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_BYTE, &local_slot_no,
							DBUS_TYPE_BYTE, &local_port_no,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "Failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	dbus_message_iter_init(reply, &iter);

	dbus_message_iter_get_basic(&iter, &ret);
	if (DHCP_SNP_RETURN_CODE_OK == ret) {
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &record_count);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &product_id);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter, &iter_array);

		vty_out(vty, "DHCP-Snooping is enabled.\n");
		vty_out(vty, "The client binding table for all trusted ports.\n");
		vty_out(vty, "Type : D--Dynamic , S--Static\n");
		vty_out(vty, "==========================================================\n");
		vty_out(vty, "%-4s %-15s %-17s %-8s %-4s %-5s\n",
					 "Type", "IP Address", "MAC Address", "Lease", "Vlan", "Port");
		vty_out(vty, "==== =============== ================= ======== ==== =====\n");

		for (j = 0; j < record_count; j++) {
			memset(macaddr, 0, 6);
			bind_type  = DCLI_DHCP_SNP_INIT_0;
			ip_addr    = DCLI_DHCP_SNP_INIT_0;
			tmp_vlanId = DCLI_DHCP_SNP_INIT_0;
			slot_no    = DCLI_DHCP_SNP_INIT_0;
			port_no    = DCLI_DHCP_SNP_INIT_0;
			lease_time = DCLI_DHCP_SNP_INIT_0;

			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array, &iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &bind_type);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &macaddr[5]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &ip_addr);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &tmp_vlanId);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &slot_no);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &port_no);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &lease_time);

			dbus_message_iter_next(&iter_array);
			
			vty_out(vty, "%-4s ", bind_type ? " S" : " D");
			vty_out(vty, "%3d.%3d.%3d.%3d ", (ip_addr>>24) & 0xff, (ip_addr>>16) & 0xff, (ip_addr>>8) & 0xff, ip_addr & 0xff);
			vty_out(vty, "%02x:%02x:%02x:%02x:%02x:%02x ",
						macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
			vty_out(vty, "%-8s ", "infinite");
			vty_out(vty, "%-4d ", tmp_vlanId);
			vty_out(vty, "%2d/%-3d", slot_no, port_no);
			vty_out(vty, "\n");
		}
		vty_out(vty, "==========================================================\n");
	}
	else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret){
		vty_out(vty, "%% DHCP-Snooping not enabled global.\n");
	}
	else if (DHCP_SNP_RETURN_CODE_ERROR == ret) {
		vty_out(vty, "%% Show DHCP-Snooping static bind table error!\n");	
	}

	dbus_message_unref(reply);

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * dcli_dhcp_snp_show_trust_ports()
 *	DESCRIPTION:
 *		show DHCP-Snooping trust and no-bind ports
 *
 *	INPUTS:
 *		struct vty* vty
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		CMD_SUCCESS		- success
 *		CMD_WARNING		- fail
***********************************************************************************/
unsigned int dcli_dhcp_snp_show_trust_ports
(
	struct vty* vty
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	DBusMessageIter iter;
	DBusMessageIter	iter_array;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int j = DCLI_DHCP_SNP_INIT_0;
	unsigned int port_cnt = DCLI_DHCP_SNP_INIT_0;
	unsigned int product_id = PRODUCT_ID_NONE;
	unsigned int slot_no = DCLI_DHCP_SNP_INIT_0;
	unsigned int port_no = DCLI_DHCP_SNP_INIT_0;
	unsigned int trust_mode = DCLI_DHCP_SNP_INIT_0;
	unsigned short vlanId = DCLI_DHCP_SNP_INIT_0;
	unsigned int tagmode = DCLI_DHCP_SNP_INIT_0;

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SHOW_TRUST_PORTS);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	dbus_message_iter_init(reply, &iter);

	dbus_message_iter_get_basic(&iter, &ret);
	if (DHCP_SNP_RETURN_CODE_OK == ret) {
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &port_cnt);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &product_id);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter, &iter_array);
		/* for debug 
		vty_out(vty, "trust port count:%-4d \n", port_cnt);
		*/

		vty_out(vty, "DHCP-Snooping is enabled.\n");
		vty_out(vty, "DHCP-Snooping all trust and no-bind ports.\n");
		vty_out(vty, "Tagged  : T -- tagged\n");
		vty_out(vty, "          U -- untagged\n");
		vty_out(vty, "Trusted : T  -- trust\n");
		vty_out(vty, "          NB -- no-bind\n");
		vty_out(vty, "==============================\n");
		vty_out(vty, "%-4s %-6s %-8s %-9s\n", " Vlan", " Port", " Tagged", " Trusted");
		vty_out(vty, "==== ====== ======== =========\n");

		for (j = 0; j < port_cnt; j++) {
			slot_no = DCLI_DHCP_SNP_INIT_0;
			port_no = DCLI_DHCP_SNP_INIT_0;
			trust_mode = DCLI_DHCP_SNP_INIT_0;
			vlanId = DCLI_DHCP_SNP_INIT_0;
			tagmode = DCLI_DHCP_SNP_INIT_0;

			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array, &iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &vlanId);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct, &slot_no);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &port_no);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &tagmode);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct, &trust_mode);

			dbus_message_iter_next(&iter_array);
			vty_out(vty, "%-4d ", vlanId);
			vty_out(vty, "%2d/%-3d", slot_no, port_no);
			vty_out(vty, "%-8s ", tagmode ? "  T" : "  U");
			vty_out(vty, "%-9s ", (trust_mode == DCLI_DHCP_SNP_PORT_MODE_NOBIND) ? "  NB" :
									((trust_mode == DCLI_DHCP_SNP_PORT_MODE_TRUST) ? "  T" : "NT"));
			vty_out(vty, "\n");
		}
		vty_out(vty, "==============================\n");
	}
	else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret){
		vty_out(vty, "%% DHCP-Snooping not enabled global.\n");
	}
	else if (DHCP_SNP_RETURN_CODE_ERROR == ret) {
		vty_out(vty, "%% Show DHCP-Snooping trust and no-bind ports error!\n");	
	}

	dbus_message_unref(reply);

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *dcli_dhcp_snp_vlan_show_running_config()
 *
 *	DESCRIPTION:
 *		show DHCP Snooping vlan running config
 *
 *	INPUTS:
 *		NULL
 *	
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		NULL
 *
 ***********************************************************************************/
int dcli_dhcp_snp_vlan_show_running_config
(
	void
)
{
	unsigned char *showStr = NULL;
	int ret = 0;
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	
	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SHOW_RUNNING_VLAN_CONFIG);

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return ret;
	}

	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_STRING, &showStr,
							DBUS_TYPE_INVALID)) 
	{
	
		char _tmpstr[64];
		memset(_tmpstr,0,64);
		sprintf(_tmpstr,BUILDING_MOUDLE,"DHCP SNOOPING VLAN");
		vtysh_add_show_string(_tmpstr);
		vtysh_add_show_string(showStr);
		ret = 1;
	} 
	else 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);

	return ret;
}

/**********************************************************************************
 *dcli_dhcp_snp_global_show_running_config()
 *
 *	DESCRIPTION:
 *		show DHCP Snooping global running config
 *
 *	INPUTS:
 *		NULL
 *	
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		NULL
 *
 ***********************************************************************************/
int dcli_dhcp_snp_global_show_running_config
(
	void
)
{
	unsigned char *showStr = NULL;
	int ret = 1;
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	
	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SHOW_RUNNING_GLOBAL_CONFIG);

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return ret;
	}

	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_STRING, &showStr,
							DBUS_TYPE_INVALID)) 
	{
	
		char _tmpstr[64];
		memset(_tmpstr,0,64);
		sprintf(_tmpstr,BUILDING_MOUDLE,"DHCP SNOOPING GLOBAL");
		vtysh_add_show_string(_tmpstr);
		vtysh_add_show_string(showStr);
		ret = 0;
	} 
	else 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);

	return ret;
}

/**********************************************************************************
 *dcli_dhcp_snp_show_running_save_bind_tbl()
 *
 *	DESCRIPTION:
 *		save bind table
 *
 *	INPUTS:
 *		NULL
 *	
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		NULL
 *
 ***********************************************************************************/
int dcli_dhcp_snp_show_running_save_bind_tbl
(
	void
)
{
	unsigned char *showStr = NULL;
	int ret = 0;
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	
	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SHOW_RUNNING_SAVE_BIND_TABLE);

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return ret;
	}

	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_STRING, &showStr,
							DBUS_TYPE_INVALID)) 
	{
	
		char _tmpstr[64];
		memset(_tmpstr,0,64);
		sprintf(_tmpstr,BUILDING_MOUDLE,"DHCP SNOOPING SAVE BING TABLE");
		vtysh_add_show_string(_tmpstr);
		vtysh_add_show_string(showStr);
		ret = 1;
	} 
	else 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);

	return ret;
}

/**********************************************************************************
 *dcli_dhcp_snp_wan_global_show_running_config()
 *
 *	DESCRIPTION:
 *		show DHCP Snooping global running config
 *
 *	INPUTS:
 *		NULL
 *	
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		NULL
 *
 ***********************************************************************************/
void dcli_dhcp_snp_wan_global_show_running_config
(
	void
)
{
	unsigned char *showStr = NULL;
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	
	query = dbus_message_new_method_call(
										DHCPSNP_DBUS_BUSNAME,
										DHCPSNP_DBUS_OBJPATH,
										DHCPSNP_DBUS_INTERFACE,
										DHCPSNP_DBUS_METHOD_SHOW_RUNNING_GLOBAL_CONFIG);

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show dhcp_snp_wan_global running config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return ;
	}

	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_STRING, &showStr,
							DBUS_TYPE_INVALID)) 
	{
		vtysh_add_show_string(showStr);
	} 
	else 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);

	return ;
}


/**********************************************************************************
 *dcli_dhcp_snp_show_running_config()
 *
 *	DESCRIPTION:
 *		show DHCP Snooping running config
 *
 *	INPUTS:
 *		NULL
 *	
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		NULL
 *
 ***********************************************************************************/
int dcli_dhcp_snp_show_running_config
(
	struct vty *vty
)
{
	char _tmpstr[64];
	memset(_tmpstr,0,64);
	sprintf(_tmpstr,BUILDING_MOUDLE,"DHCP SNOOPING");
	vtysh_add_show_string(_tmpstr);
	dcli_dhcp_snp_wan_global_show_running_config();
	dcli_dhcp_snp_global_show_running_config();
	dcli_dhcp_snp_vlan_show_running_config();
	dcli_dhcp_snp_show_running_save_bind_tbl();
	return 0;
}

/**********************************************************************************
 *dcli_dhcp_snp_show_running_config2()
 *
 *	DESCRIPTION:
 *		show DHCP Snooping running config2
 *
 *	INPUTS:
 *		NULL
 *	
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		NULL
 *
 ***********************************************************************************/
char *dcli_dhcp_snp_show_running_config2
(
	struct vty *vty,
	int slot_id
)
{
	unsigned char *showStr = NULL;
	char *tmp = NULL;
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;

	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);	
	
	query = dbus_message_new_method_call(
										DHCPSNP_DBUS_BUSNAME,
										DHCPSNP_DBUS_OBJPATH,
										DHCPSNP_DBUS_INTERFACE,
										DHCPSNP_DBUS_METHOD_SHOW_RUNNING_GLOBAL_CONFIG);

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "show dhcp_snp_wan_global running config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}

	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_STRING, &showStr,
							DBUS_TYPE_INVALID)) 
	{
		tmp = (char *)malloc(strlen(showStr)+1);
		memset(tmp, 0, strlen(showStr)+1);
		memcpy(tmp,showStr,strlen(showStr));

		dbus_message_unref(reply);
		
		return tmp; 
	} 
	else 
	{
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);

	return NULL;

	#if 0
	/* TODO npd dhcp snooping */
	dcli_dhcp_snp_global_show_running_config();
	dcli_dhcp_snp_vlan_show_running_config();
	dcli_dhcp_snp_show_running_save_bind_tbl();
	#endif
}

char * 
dcli_dhcp_snp_show_running_hansi_cfg
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
	
	query = dbus_message_new_method_call(
										DHCPSNP_DBUS_BUSNAME,
										DHCPSNP_DBUS_OBJPATH,
										DHCPSNP_DBUS_INTERFACE,
										DHCPSNP_DBUS_METHOD_SHOW_RUNNING_GLOBAL_HANSI_CONFIG);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&slot_id,
							 DBUS_TYPE_UINT32,&InstID,
							 DBUS_TYPE_UINT32,&islocaled,
							 DBUS_TYPE_INVALID);


	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("DHCP SNOOPING show running failed get reply.\n");
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



/**********************************************************************************
 * dcli_dhcp_snp_config_intf
 *
 *	DESCRIPTION:
 *		config DHCP-Snooping enable status on interface
 *
 *	INPUTS:
 *	 	ifindex - interface ifindex
 *	 	unsigned char slot_no,			- slot no of port
 *	 	unsigned char local_port_no,		- port no of port
 *		unsigned char trust_mode		- trust mode
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *		DHCP_SNP_RETURN_CODE_ERROR		- fail
***********************************************************************************/
unsigned int dcli_dhcp_snp_config_intf
(
	struct vty *vty,
	char *ifname,
	int flag
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;

	DBusConnection *dcli_dbus_connection = NULL;
	dhcp_snp_reinitDbusConnection(&dcli_dbus_connection, vty);

	query = dbus_message_new_method_call(
								DHCPSNP_DBUS_BUSNAME,
								DHCPSNP_DBUS_OBJPATH,
								DHCPSNP_DBUS_INTERFACE,
								DHCPSNP_DBUS_METHOD_INTERFACE_ENABLE);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_STRING, &ifname,
							 DBUS_TYPE_INT32, &flag,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		dbus_message_unref(reply);
		return ret;
	}
	else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
}

/**********************************************************************************
 * dcli_dhcp_snp_add_router_config_intf
 *
 *	DESCRIPTION:
 *		config DHCP-Snooping add router enable status on interface
 *
 *	INPUTS:
 *	 	ifindex - interface ifindex
 *	 	unsigned char slot_no,			- slot no of port
 *	 	unsigned char local_port_no,		- port no of port
 *		unsigned char trust_mode		- trust mode
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *		DHCP_SNP_RETURN_CODE_ERROR		- fail
***********************************************************************************/
unsigned int dcli_dhcp_snp_add_router_config_intf
(
	struct vty *vty,
	char *ifname,
	int flag,
	int add_router_type
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;

	DBusConnection *dcli_dbus_connection = NULL;
	dhcp_snp_reinitDbusConnection(&dcli_dbus_connection, vty);

	query = dbus_message_new_method_call(
								DHCPSNP_DBUS_BUSNAME,
								DHCPSNP_DBUS_OBJPATH,
								DHCPSNP_DBUS_INTERFACE,
								DHCPSNP_DBUS_METHOD_INTERFACE_ADD_ROUTER_ENABLE);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_STRING, &ifname,
							 DBUS_TYPE_INT32, &flag,
							 DBUS_TYPE_INT32, &add_router_type,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		dbus_message_unref(reply);
		return ret;
	}
	else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
}

/**********************************************************************************
 * dcli_dhcp_snp_arp_config_intf
 *
 *	DESCRIPTION:
 *		config DHCP-Snooping enable status on interface
 *
 *	INPUTS:
 *	 	ifindex - interface ifindex
 *	 	unsigned char slot_no,			- slot no of port
 *	 	unsigned char local_port_no,		- port no of port
 *		unsigned char trust_mode		- trust mode
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *		DHCP_SNP_RETURN_CODE_ERROR		- fail
***********************************************************************************/
unsigned int dcli_dhcp_snp_arp_config_intf
(
	struct vty *vty,
	char *ifname,
	int flag,
	int anti_arp_type
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;

	DBusConnection *dcli_dbus_connection = NULL;
	dhcp_snp_reinitDbusConnection(&dcli_dbus_connection, vty);

	query = dbus_message_new_method_call(
								DHCPSNP_DBUS_BUSNAME,
								DHCPSNP_DBUS_OBJPATH,
								DHCPSNP_DBUS_INTERFACE,
								DHCPSNP_DBUS_METHOD_INTERFACE_ARP_ENABLE);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_STRING, &ifname,
							 DBUS_TYPE_INT32, &flag,
							 DBUS_TYPE_INT32, &anti_arp_type,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		dbus_message_unref(reply);
		return ret;
	}
	else {
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
}

/**************************************************
 *	command: config dhcp-snooping (enable|disable)
 *
 *	note      : complete ONLY by dhcp snooping Protocol
 *
 *	cmd Node: config Node
 *
***************************************************/
DEFUN(config_dhcp_snp_enable_cmd_func,
	config_dhcp_snp_enable_cmd,
	"config dhcp-snooping lan (enable|disable)",
	CONFIG_STR
	DCLI_DHCP_SNP_STR
	"Config DHCP-Snooping on lan interface\n"
	"Enable DHCP-Snooping functionality\n"
	"Disable DHCP-Snooping functionality\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned char status = DCLI_DHCP_SNP_INIT_0;
	unsigned char isEnable = DCLI_DHCP_SNP_INIT_0;
	unsigned int ret = DCLI_DHCP_SNP_INIT_0;
	
	if (argc > 1) {
		vty_out(vty,"%% Command without so many parameters!\n");
		return CMD_WARNING;
	}
	
	if (strncmp(argv[0], "enable", strlen(argv[0])) == 0) {
		isEnable = DCLI_DHCP_SNP_ENABLE;
	}
	else if (strncmp(argv[0], "disable", strlen(argv[0])) == 0) {
		isEnable = DCLI_DHCP_SNP_DISABLE;
	}else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	ret = dcli_dhcp_snp_check_global_status(&status);
	if (DHCP_SNP_RETURN_CODE_OK == ret) {
		if(isEnable == status) {
			vty_out(vty, "%% DHCP-Snooping already %s!\n",
					isEnable ? "enabled" : "disabled");
			return CMD_WARNING;
		}
	}else {
		vty_out(vty, "%% Check DHCP-Snooping state error %x!\n", ret);
		return CMD_SUCCESS;
	}

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_GLOBAL_ENABLE);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_BYTE, &isEnable,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)) {
		if (DHCP_SNP_RETURN_CODE_OK != ret) {
			vty_out(vty, "%% %s DHCP-Snooping error %x!", isEnable ? "Enable":"Disable", ret);
			return CMD_WARNING;
		}
	} else {
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
}

/**************************************************
 *	command: config dhcp-snooping (enable|disable)
 *
 *	note      : complete ONLY by dhcp snooping Protocol
 *
 *	cmd Node: config Node
 *
***************************************************/
DEFUN(config_dhcp_snp_wan_enable_cmd_func,
	config_dhcp_snp_wan_enable_cmd,
	"config dhcp-snooping wan (enable|disable)",
	CONFIG_STR
	DCLI_DHCP_SNP_STR
	"Config DHCP-Snooping on wan interface\n"
	"Enable DHCP-Snooping functionality\n"
	"Disable DHCP-Snooping functionality\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned char status = DCLI_DHCP_SNP_INIT_0;
	unsigned char isEnable = DCLI_DHCP_SNP_INIT_0;
	unsigned int ret = DCLI_DHCP_SNP_INIT_0;
	
	if (argc > 1) {
		vty_out(vty,"%% Command without so many parameters!\n");
		return CMD_WARNING;
	}
	
	if (strncmp(argv[0], "enable", strlen(argv[0])) == 0) {
		isEnable = DCLI_DHCP_SNP_ENABLE;
	}
	else if (strncmp(argv[0], "disable", strlen(argv[0])) == 0) {
		isEnable = DCLI_DHCP_SNP_DISABLE;
	}else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	ret = dcli_dhcp_snp_wan_check_global_status(vty, &status);
	if (DHCP_SNP_RETURN_CODE_OK == ret) {
		if(isEnable == status) {
			vty_out(vty, "%% DHCP-Snooping already %s!\n",
					isEnable ? "enabled" : "disabled");
			return CMD_WARNING;
		}
	}else {
		vty_out(vty, "%% Check DHCP-Snooping state error %x!\n", ret);
		return CMD_SUCCESS;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	dhcp_snp_reinitDbusConnection(&dcli_dbus_connection, vty);

	query = dbus_message_new_method_call(
										DHCPSNP_DBUS_BUSNAME,
										DHCPSNP_DBUS_OBJPATH,
										DHCPSNP_DBUS_INTERFACE,
										DHCPSNP_DBUS_METHOD_GLOBAL_ENABLE);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_BYTE, &isEnable,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)) {
		if (DHCP_SNP_RETURN_CODE_EN_INTF == ret) {
			vty_out(vty, "%% Disable interface dhcp snooping first.");
			return CMD_WARNING;
		} else if (DHCP_SNP_RETURN_CODE_OK != ret){
			vty_out(vty, "%% %s DHCP-Snooping error %x!", isEnable ? "Enable":"Disable", ret);
			return CMD_WARNING;
		}
	} else {
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
}

/**************************************************
 *	command: config dhcp-snooping (enable|disable)
 *
 *	note      : complete only by dhcp snooping Protocol
 *
 *	cmd Node: vlan Node
 *
***************************************************/
DEFUN(config_dhcp_snp_vlan_enable_cmd_func,
	config_dhcp_snp_vlan_enable_cmd,
	"config dhcp-snooping (enable|disable)",
	CONFIG_STR
	DCLI_DHCP_SNP_STR
	"Enable DHCP-Snooping functionality\n"
	"Disable DHCP-Snooping functionality\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned char status = DCLI_DHCP_SNP_INIT_0;
	unsigned char isEnable = DCLI_DHCP_SNP_INIT_0;
	unsigned short vlanId = DCLI_DHCP_SNP_INIT_0;
	unsigned int nodesave = DCLI_DHCP_SNP_INIT_0;
	unsigned int ret = DCLI_DHCP_SNP_INIT_0;
	
	if (argc > 1) {
		vty_out(vty,"%% Command without so many parameters!\n");
		return CMD_WARNING;
	}

	if (strncmp(argv[0], "enable", strlen(argv[0])) == 0) {
		isEnable = DCLI_DHCP_SNP_ENABLE;
	}
	else if (strncmp(argv[0], "disable", strlen(argv[0])) == 0) {
		isEnable = DCLI_DHCP_SNP_DISABLE;
	}else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	/*get vlanid */
	nodesave = (unsigned int)(vty->index);
	vlanId = (unsigned short)nodesave;

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_VLAN_ENABLE);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_UINT16, &vlanId,
							DBUS_TYPE_BYTE, &isEnable,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		if (DHCP_SNP_RETURN_CODE_OK == ret) {
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
			vty_out(vty, "%% DHCP-Snooping not enabled global.\n");
		}else if (DHCP_SNP_RETURN_CODE_ALREADY_SET == ret) {
			vty_out(vty, "%% DHCP-Snooping already %s on vlan %d.\n",
					(isEnable== DCLI_DHCP_SNP_ENABLE) ? "enable" : "disable",
					vlanId);
		}else if (DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST == ret) {
			vty_out(vty, "%% Error: vlan %d not exists!\n", vlanId);
		}else {
			vty_out(vty, "%% Error: %s DHCP-Snooping on vlan %d error %x!",
				isEnable ? "enable" : "disable",
				ret);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	else {
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
}


/*********************************************************
 *	command: config dhcp-snooping PORTNO (trust|trust-nobind|notrust)
 *
 *	note      : config dhcp-snooping trust mode of port in special vlan
 *
 *	cmd Node: vlan Node
 *
**********************************************************/
DEFUN(config_dhcp_snp_set_vlan_port_mode_cmd_func,
	config_dhcp_snp_set_vlan_port_mode_cmd,
	"config dhcp-snooping PORTNO (trust|trust-nobind|notrust)",
	CONFIG_STR
	DCLI_DHCP_SNP_STR
	CONFIG_ETHPORT_STR
	"Config DHCP-Snooping trust mode on port\n"
	"Config DHCP-Snooping trust-nobind mode on port\n"
	"Config DHCP-Snooping notrust mode on port\n"
)
{
	unsigned int  ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char trust_mode = DCLI_DHCP_SNP_PORT_MODE_INVALID;
	unsigned char slot_no = DCLI_DHCP_SNP_INIT_0;
	unsigned char local_port_no = DCLI_DHCP_SNP_INIT_0;
	unsigned int t_slotno = DCLI_DHCP_SNP_INIT_0;
	unsigned int t_portno = DCLI_DHCP_SNP_INIT_0;
	unsigned short vlanId = DCLI_DHCP_SNP_INIT_0;
	unsigned int   op_ret = DCLI_DHCP_SNP_INIT_0;
	unsigned int nodesave = DCLI_DHCP_SNP_INIT_0;

	if (argc > 2) {
		vty_out(vty,"%% Command without so many parameters!\n");;
		return CMD_WARNING;
	}

	/* fetch the 1nd param : slotNo/portNo */
	op_ret = parse_slotno_localport((char *)argv[0], &t_slotno, &t_portno);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty, "% Bad parameter:Unknow portno format.\n");
		return CMD_WARNING;
	}
	else if (1 == op_ret){
		vty_out(vty, "% Bad parameter: Bad slot/port number.\n");
		return CMD_WARNING;
	}
	slot_no = (unsigned char)t_slotno;
	local_port_no = (unsigned char)t_portno;

	/* not support config port on slot0 */
	if(0  == slot_no) {
		vty_out(vty, "% Bad parameter: Bad slot/port number.\n");
		return CMD_WARNING;
	}


	/* get 2nd param : trust mode */
	if (!strncmp(argv[1], "trust", strlen(argv[1]))) {
		trust_mode = DCLI_DHCP_SNP_PORT_MODE_TRUST;
	}
	else if(!strncmp(argv[1], "trust-nobind", strlen(argv[1]))) {
		trust_mode = DCLI_DHCP_SNP_PORT_MODE_NOBIND;
	}
	else if(!strncmp(argv[1], "notrust", strlen(argv[1]))) {
		trust_mode = DCLI_DHCP_SNP_PORT_MODE_NOTRUST;
	}else {
		vty_out(vty, "%% Command parameters error!\n");
		return CMD_WARNING;
	}

	nodesave = (unsigned int)(vty->index);
	vlanId = (unsigned short)nodesave;

	ret = dcli_dhcp_snp_config_port(vlanId, slot_no, local_port_no, trust_mode);

	if (DHCP_SNP_RETURN_CODE_OK == ret) {
		
	}else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
		vty_out(vty, "%% DHCP-Snooping not enabled global.\n");
	}else if (DHCP_SNP_RETURN_CODE_NOT_EN_VLAN == ret) {
		vty_out(vty, "%% DHCP-Snooping not enabled on vlan %d.\n", vlanId);
	}else if (DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST == ret) {
		vty_out(vty, "%% Vlan %d is not exist.\n", vlanId);
	}else if (DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST == ret) {
		vty_out(vty, "%% Port %d/%d is not member of vlan %d.\n", slot_no, local_port_no, vlanId);
	}else if (DHCP_SNP_RETURN_CODE_NO_SUCH_PORT == ret) {
		vty_out(vty, "%% slot no or port no is not legal.\n");
	}else if (DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR == ret) {
		vty_out(vty, "%% Port %d/%d is member of trunk.\n", slot_no, local_port_no);
	}else if (DHCP_SNP_RETURN_CODE_ALREADY_SET == ret) {
		vty_out(vty, "%% Port already setted %s mode.\n",
				(DCLI_DHCP_SNP_PORT_MODE_TRUST == trust_mode) ? "trust" :
					((DCLI_DHCP_SNP_PORT_MODE_NOBIND == trust_mode) ? "trust-nobind" : 
						((DCLI_DHCP_SNP_PORT_MODE_NOTRUST == trust_mode) ? "notrust" : "unknown")));
	}else {
		vty_out(vty, "%% Unknown Error! return %x \n", ret);
	}

	return CMD_SUCCESS;
}

/*********************************************************
 *	command: show dhcp-snooping
 *
 *	note      : show dhcp-snooping bind table
 *
 *	cmd Node: config Node
 *
**********************************************************/
DEFUN(show_dhcp_snp_bindtable_cmd_func,
	show_dhcp_snp_bindtable_cmd,
	"show dhcp-snooping",
	SHOW_STR
	"Show DHCP-Snooping binding table for all trusted ports\n"
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;

	if (argc > 0) {
		vty_out(vty,"%% Command without so many parameters!\n");;
		return CMD_WARNING;
	}

	ret = dcli_dhcp_snp_show_bind_table(vty);
	if (DHCP_SNP_RETURN_CODE_OK == ret) {
		return CMD_SUCCESS;
	}else {
		vty_out(vty, "%% Show dhcp-snooping error!\n", ret);
		return CMD_WARNING;
	}
}

/*********************************************************
 *	command: show dhcp-snooping static-binding
 *
 *	note   : show dhcp-snooping static binding items
 *			 in binding table
 *
 *	cmd Node: config Node
 *
**********************************************************/
DEFUN(show_dhcp_snp_static_bindtable_cmd_func,
	show_dhcp_snp_static_bindtable_cmd,
	"show dhcp-snooping static-binding",
	SHOW_STR
	"Show DHCP-Snooping static-binding items in binding table\n"
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;

	if (argc > 0) {
		vty_out(vty,"%% Command without so many parameters!\n");;
		return CMD_WARNING;
	}

	ret = dcli_dhcp_snp_show_static_bind_table(vty);
	if (DHCP_SNP_RETURN_CODE_OK == ret) {
		return CMD_SUCCESS;
	}else {
		vty_out(vty, "%% Show dhcp-snooping error!\n", ret);
		return CMD_WARNING;
	}
}

/***************************************************************
 *	command: show dhcp-snooping static-binding vlan <1-4094>
 *
 *	note   : show dhcp-snooping static binding items
 *			 which belong to the special vlan in binding table 
 *
 *	cmd Node: config Node
 *
*****************************************************************/
DEFUN(show_dhcp_snp_static_bindtable_by_vlan_cmd_func,
	show_dhcp_snp_static_bindtable_by_vlan_cmd,
	"show dhcp-snooping static-binding vlan <1-4094>",
	SHOW_STR
	"Show DHCP-Snooping static-binding items in binding table\n"
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned short  vlanId = DCLI_DHCP_SNP_INIT_0;

	if (argc > 1) {
		vty_out(vty, "%% Command without so many parameters!\n");;
		return CMD_WARNING;
	}

	/*get first param, vlan id	*/
	ret = parse_vlan_no((char*)argv[0], &vlanId);
	if (NPD_FAIL == ret) {
    	vty_out(vty, "% Bad parameter,vlan id illegal!\n");
		return CMD_WARNING;
	}

	ret = dcli_dhcp_snp_show_static_bind_table_by_vlan(vty, vlanId);
	if (DHCP_SNP_RETURN_CODE_OK == ret) {
		return CMD_SUCCESS;
	}else {
		vty_out(vty, "%% Show dhcp-snooping error!\n", ret);
		return CMD_WARNING;
	}
}

/******************************************************************
 *	command: show dhcp-snooping static-binding eth-port PORTNO
 *
 *	note   : show dhcp-snooping static binding items
 *			 which belong to the special eth-port in binding table
 *
 *	cmd Node: config Node
 *
*******************************************************************/
DEFUN(show_dhcp_snp_static_bindtable_by_ethport_cmd_func,
	show_dhcp_snp_static_bindtable_by_ethport_cmd,
	"show dhcp-snooping static-binding eth-port PORTNO",
	SHOW_STR
	"Show DHCP-Snooping static-binding items in binding table\n"
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char local_slot_no = DCLI_DHCP_SNP_INIT_0;
	unsigned char local_port_no = DCLI_DHCP_SNP_INIT_0;
	unsigned int t_slotno = DCLI_DHCP_SNP_INIT_0;
	unsigned int t_portno = DCLI_DHCP_SNP_INIT_0;
	unsigned int   op_ret = DCLI_DHCP_SNP_INIT_0;

	if (argc > 1) {
		vty_out(vty,"%% Command without so many parameters!\n");;
		return CMD_WARNING;
	}

	/* fetch the 1nd param : slotNo/portNo */
	op_ret = parse_slotno_localport((char *)argv[0], &t_slotno, &t_portno);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty, "%% Bad parameter:Unknow portno format.\n");
		return CMD_WARNING;
	}
	else if (1 == op_ret){
		vty_out(vty, "%% Bad parameter: Bad slot/port number.\n");
		return CMD_WARNING;
	}
	local_slot_no = (unsigned char)t_slotno;
	local_port_no = (unsigned char)t_portno;

	/* not support config port on slot0 */
	if (0 == local_slot_no) {
		vty_out(vty, "%% Bad parameter: Bad slot/port number.\n");
		return CMD_WARNING;
	}

	ret = dcli_dhcp_snp_show_static_bind_table_by_ethport(vty, local_slot_no, local_port_no);
	if (DHCP_SNP_RETURN_CODE_OK == ret) {
		return CMD_SUCCESS;
	}else {
		vty_out(vty, "%% Show dhcp-snooping error!\n", ret);
		return CMD_WARNING;
	}
}

/*********************************************************
 *	command: show dhcp-snooping
 *
 *	note      : show dhcp-snooping trust and no-bind ports
 *
 *	cmd Node: config Node
 *
**********************************************************/
DEFUN(show_dhcp_snp_trust_ports_cmd_func,
	show_dhcp_snp_trust_ports_cmd,
	"show dhcp-snooping trust",
	SHOW_STR
	DCLI_DHCP_SNP_STR
	"Show DHCP-Snooping all trusted no-bind ports\n"
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;

	if (argc > 0) {
		vty_out(vty,"%% Command without so many parameters!\n");;
		return CMD_WARNING;
	}

	ret = dcli_dhcp_snp_show_trust_ports(vty);
	if (DHCP_SNP_RETURN_CODE_OK == ret) {
		return CMD_SUCCESS;
	}else {
		vty_out(vty, "%% Show dhcp-snooping error!\n", ret);
		return CMD_WARNING;
	}
}


/**************************************************
 *	command: config dhcp-snooping information (enable|disable)
 *
 *	note      : enable/disable  DHCP-Snooping support option 82 functionality
 *
 *	cmd Node: config Node
 *
***************************************************/
DEFUN(config_dhcp_snp_opt82_enable_cmd_func,
	config_dhcp_snp_opt82_enable_cmd,
	"config dhcp-snooping information (enable|disable)",
	CONFIG_STR
	DCLI_DHCP_SNP_STR
	DCLI_DHCP_SNP_OPT82_STR
	"Enable DHCP-Snooping support option 82 functionality\n"
	"Disable DHCP-Snooping support option 82 functionality\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char isEnable = DCLI_DHCP_SNP_INIT_0;
	
	if (argc > 1) {
		vty_out(vty,"%% Command without so many parameters!\n");
		return CMD_WARNING;
	}
	
	if (strncmp(argv[0], "enable", strlen(argv[0])) == 0) {
		isEnable = 1;
	}
	else if (strncmp(argv[0], "disable", strlen(argv[0])) == 0) {
		isEnable = 0;
	}else {
		vty_out(vty, "%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_OPT82_ENABLE);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_BYTE, &isEnable,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
 		if (DHCP_SNP_RETURN_CODE_OK == ret) {
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
			vty_out(vty, "%% DHCP-Snooping not enabled global.\n");
		}else if (DHCP_SNP_RETURN_CODE_ALREADY_SET == ret) {
			vty_out(vty, "%% DHCP-Snooping already %s information.\n",
							isEnable ? "enabled" : "disabled");
		}else {
			vty_out(vty, "%% Unknown Error! return %x \n", ret);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}else {
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
}

/**************************************************
 *	command: config dhcp-snooping information format (hex|ascii)
 *
 *	note      : Config DHCP-Snooping storage format of option 82
 *
 *	cmd Node: config Node
 *
***************************************************/
DEFUN(config_dhcp_snp_opt82_format_cmd_func,
	config_dhcp_snp_opt82_format_cmd,
	"config dhcp-snooping information format (hex|ascii)",
	CONFIG_STR
	DCLI_DHCP_SNP_STR
	DCLI_DHCP_SNP_OPT82_STR
	DCLI_DHCP_SNP_OPT82_FORMAT_STR
	"Config DHCP-Snooping storage format of option 82 is HEX\n"
	"Config DHCP-Snooping storage format of option 82 is ASCII\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char format_type = DCLI_DHCP_SNP_OPT82_FORMAT_TYPE_INVALID;
	
	if (argc > 1) {
		vty_out(vty,"%% Command without so many parameters!\n");
		return CMD_WARNING;
	}
	
	if (strncmp(argv[0], "hex", strlen(argv[0])) == 0) {
		format_type = DCLI_DHCP_SNP_OPT82_FORMAT_TYPE_HEX;
	}
	else if (strncmp(argv[0], "ascii", strlen(argv[0])) == 0) {
		format_type = DCLI_DHCP_SNP_OPT82_FORMAT_TYPE_ASCII;
	}else {
		vty_out(vty, "%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_FORMAT_TYPE);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_BYTE, &format_type,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
 		if (DHCP_SNP_RETURN_CODE_OK == ret) {
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
			vty_out(vty, "%% DHCP-Snooping not enabled global.\n");
		}else if (DHCP_SNP_RETURN_CODE_ALREADY_SET == ret) {
			vty_out(vty, "%% DHCP-Snooping already set format %s for information.\n",
							format_type ? "ascii" : "hex");
		}else {
			vty_out(vty, "%% Unknown Error! return %x \n", ret);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}else {
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);

	return CMD_SUCCESS;
}


/*****************************************************************
 *	command: config dhcp-snooping information packet-format (extended|standard)
 *
 *	note      : Config DHCP-Snooping fill packet format of option 82
 *
 *	cmd Node: config Node
 *
******************************************************************/
DEFUN(config_dhcp_snp_opt82_fill_format_cmd_func,
	config_dhcp_snp_opt82_fill_format_cmd,
	"config dhcp-snooping information packet-format (extended|standard)",
	CONFIG_STR
	DCLI_DHCP_SNP_STR
	DCLI_DHCP_SNP_OPT82_STR
	DCLI_DHCP_SNP_OPT82_PKT_FORMAT_STR
	"Config DHCP-Snooping fill format of option 82 is extended\n"
	"Config DHCP-Snooping fill format of option 82 is standard\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char fill_type = DCLI_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_INVALID;
	
	if (argc > 1) {
		vty_out(vty,"%% Command without so many parameters!\n");
		return CMD_WARNING;
	}
	
	if (strncmp(argv[0], "extended", strlen(argv[0])) == 0) {
		fill_type = DCLI_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_EXT;
	}
	else if (strncmp(argv[0], "standard", strlen(argv[0])) == 0) {
		fill_type = DCLI_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_STD;
	}else {
		vty_out(vty, "%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_FILL_FORMAT_TYPE);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_BYTE, &fill_type,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
 		if (DHCP_SNP_RETURN_CODE_OK == ret) {
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
			vty_out(vty, "%% DHCP-Snooping not enabled global.\n");
		}else if (DHCP_SNP_RETURN_CODE_ALREADY_SET == ret) {
			vty_out(vty, "%% DHCP-Snooping already set packet fill format %s for information.\n",
							fill_type ? "standard" : "extended");
		}else {
			vty_out(vty, "%% Unknown Error! return %x \n", ret);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}else {
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);

	return CMD_SUCCESS;
}


/*****************************************************************
 *	command: config dhcp-snooping information remote-id (sysmac|sysname)
 *
 *	note      : Config DHCP-Snooping Remote-ID content of option 82
 *
 *	cmd Node: config Node
 *
******************************************************************/
DEFUN(config_dhcp_snp_opt82_set_remoteid_cmd_func1,
	config_dhcp_snp_opt82_set_remoteid_cmd1,
	"config dhcp-snooping information remote-id (sysmac|sysname)",
	CONFIG_STR
	DCLI_DHCP_SNP_STR
	DCLI_DHCP_SNP_OPT82_STR
	DCLI_DHCP_SNP_OPT82_REMOTEID_STR
	"Config DHCP-Snooping Remote-ID content of option 82 is system mac\n"
	"Config DHCP-Snooping Remote-ID content of option 82 is system name\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int str_size = DCLI_DHCP_SNP_INIT_0;
	unsigned char remoteid_type = DCLI_DHCP_SNP_OPT82_REMOTEID_TYPE_INVALID;
	unsigned char *remoteid_str = NULL;

	if (argc > 1) {
		vty_out(vty,"%% Command without so many parameters!\n");
		return CMD_WARNING;
	}

	remoteid_str = (char*)malloc(DCLI_DHCP_SNP_REMOTEID_STR_LEN);
	memset(remoteid_str, 0, DCLI_DHCP_SNP_REMOTEID_STR_LEN);

	if (strncmp(argv[0], "sysmac", strlen(argv[0])) == 0) {
		remoteid_type = DCLI_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC;
	}else if (strncmp(argv[0], "sysname", strlen(argv[0])) == 0) {
		remoteid_type = DCLI_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSNAME;
	}
	else {
		vty_out(vty, "%% Bad parameter %s!\n", argv[0]);
		free(remoteid_str);
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_REMOTEID_CONTENT);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_BYTE, &remoteid_type,
							DBUS_TYPE_STRING, &remoteid_str,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		free(remoteid_str);
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
 		if (DHCP_SNP_RETURN_CODE_OK == ret) {
			dbus_message_unref(reply);
			free(remoteid_str);
			return CMD_SUCCESS;
		}else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
			vty_out(vty, "%% DHCP-Snooping not enabled global.\n");
		}else {
			vty_out(vty, "%% Unknown Error! return %x \n", ret);
		}
		dbus_message_unref(reply);
		free(remoteid_str);
		return CMD_WARNING;
	}else {
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	free(remoteid_str);
	return CMD_SUCCESS;
}


/*****************************************************************
 *	command: config dhcp-snooping information remote-id string STRING
 *
 *	note      : Config DHCP-Snooping Remote-ID content of option 82
 *
 *	cmd Node: config Node
 *
******************************************************************/
DEFUN(config_dhcp_snp_opt82_set_remoteid_cmd_func2,
	config_dhcp_snp_opt82_set_remoteid_cmd2,
	"config dhcp-snooping information remote-id string STRING",
	CONFIG_STR
	DCLI_DHCP_SNP_STR
	DCLI_DHCP_SNP_OPT82_STR
	DCLI_DHCP_SNP_OPT82_REMOTEID_STR
	"Config DHCP-Snooping Remote-ID content of option 82 is user-defined string\n"
	"User-defined string of the Remote-ID content, format for the ASCII string that range from 1 to 63 characters"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int str_size = DCLI_DHCP_SNP_INIT_0;
	unsigned char remoteid_type = DCLI_DHCP_SNP_OPT82_REMOTEID_TYPE_INVALID;
	unsigned char *remoteid_str = NULL;

	if (argc > 1) {
		vty_out(vty,"%% Command without so many parameters!\n");
		return CMD_WARNING;
	}

	remoteid_str = (char*)malloc(DCLI_DHCP_SNP_REMOTEID_STR_LEN);
	memset(remoteid_str, 0, DCLI_DHCP_SNP_REMOTEID_STR_LEN);

	remoteid_type = DCLI_DHCP_SNP_OPT82_REMOTEID_TYPE_USERSTR;

	ret = dcli_dhcp_snp_remoteid_string_legal_check((char*)argv[0], strlen(argv[0]));
	if (1 == ret) {
		vty_out(vty, "%% Bad parameter, valid length of string must be in range [1-63]!\n");
		free(remoteid_str);
		return CMD_WARNING;
	}
	else if(2 == ret) {
		vty_out(vty, "%% Bad parameter, string begins with an illegal char!\n");
		free(remoteid_str);
		return CMD_WARNING;
	}
	else if(3 == ret) {
		vty_out(vty, "%% Bad parameter, string contains illegal char!\n");
		free(remoteid_str);
		return CMD_WARNING;
	}

	str_size = strlen(argv[0]);
	memcpy(remoteid_str, argv[0], str_size);

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_REMOTEID_CONTENT);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_BYTE, &remoteid_type,
							DBUS_TYPE_STRING, &remoteid_str,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		free(remoteid_str);
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
 		if (DHCP_SNP_RETURN_CODE_OK == ret) {
			dbus_message_unref(reply);
			free(remoteid_str);
			return CMD_SUCCESS;
		}else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
			vty_out(vty, "%% DHCP-Snooping not enabled global.\n");
		}else {
			vty_out(vty, "%% Unknown Error! return %x \n", ret);
		}
		dbus_message_unref(reply);
		free(remoteid_str);
		return CMD_WARNING;
	}else {
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	free(remoteid_str);
	return CMD_SUCCESS;
}

/*********************************************************
 *	command: config dhcp-snooping information strategy (drop|keep|replace)
 *
 *	note      : Config the configuration strategy for option 82
 *			on special vlan's port
 *
 *	cmd Node: vlan Node
 *
**********************************************************/
DEFUN(config_dhcp_snp_set_port_opt82_strategy_cmd_func,
	config_dhcp_snp_set_port_opt82_strategy_cmd,
	"config dhcp-snooping information strategy PORTNO (drop|keep|replace)",
	CONFIG_STR
	DCLI_DHCP_SNP_STR
	DCLI_DHCP_SNP_OPT82_STR
	DCLI_DHCP_SNP_OPT82_STRATEGY_STR
	CONFIG_ETHPORT_STR
	"Config the configuration strategy is drop for option 82\n"
	"Config the configuration strategy is keep for option 82\n"
	"Config the configuration strategy is replace for option 82\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char strategy_mode = DCLI_DHCP_SNP_OPT82_STRATEGY_TYPE_INVALID;
	unsigned char slot_no = DCLI_DHCP_SNP_INIT_0;
	unsigned char local_port_no = DCLI_DHCP_SNP_INIT_0;
	unsigned int t_slotno = DCLI_DHCP_SNP_INIT_0;
	unsigned int t_portno = DCLI_DHCP_SNP_INIT_0;
	unsigned short vlanId = DCLI_DHCP_SNP_INIT_0;
	unsigned int   op_ret = DCLI_DHCP_SNP_INIT_0;
	unsigned int nodesave = DCLI_DHCP_SNP_INIT_0;

	if (argc > 2) {
		vty_out(vty,"%% Command without so many parameters!\n");;
		return CMD_WARNING;
	}

	/* fetch the 1nd param : slotNo/portNo */
	op_ret = parse_slotno_localport((char *)argv[0], &t_slotno, &t_portno);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty, "% Bad parameter:Unknow portno format.\n");
		return CMD_WARNING;
	}
	else if (1 == op_ret){
		vty_out(vty, "% Bad parameter: Bad slot/port number.\n");
		return CMD_WARNING;
	}
	slot_no = (unsigned char)t_slotno;
	local_port_no = (unsigned char)t_portno;

	/* not support config port on slot0 */
	if(0  == slot_no) {
		vty_out(vty, "% Bad parameter: Bad slot/port number.\n");
		return CMD_WARNING;
	}

	/*get param : trust mode*/
	if (!strncmp(argv[1], "drop", strlen(argv[1]))) {
		strategy_mode = DCLI_DHCP_SNP_OPT82_STRATEGY_TYPE_DROP;
	}
	else if(!strncmp(argv[1], "keep", strlen(argv[1]))) {
		strategy_mode = DCLI_DHCP_SNP_OPT82_STRATEGY_TYPE_KEEP;
	}
	else if(!strncmp(argv[1], "replace", strlen(argv[1]))) {
		strategy_mode = DCLI_DHCP_SNP_OPT82_STRATEGY_TYPE_REPLACE;
	}else {
		vty_out(vty, "%% Command parameters error!\n");
		return CMD_WARNING;
	}

	nodesave = (unsigned int)(vty->index);
	vlanId = (unsigned short)nodesave;

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_PORT_STRATEGY);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT16, &vlanId,
							DBUS_TYPE_BYTE, &slot_no,
							DBUS_TYPE_BYTE, &local_port_no,
							DBUS_TYPE_BYTE, &strategy_mode,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		if (DHCP_SNP_RETURN_CODE_OK == ret) {
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
			vty_out(vty, "%% DHCP-Snooping not enabled global.\n");
		}else if (DHCP_SNP_RETURN_CODE_NOT_EN_VLAN == ret) {
			vty_out(vty, "%% DHCP-Snooping not enabled on vlan %d.\n", vlanId);
		}else if (DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST == ret) {
			vty_out(vty, "%% Vlan %d is not exist.\n", vlanId);
		}else if (DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST == ret) {
			vty_out(vty, "%% Port %d/%d is not member of vlan %d.\n", slot_no, local_port_no, vlanId);
		}else if (DHCP_SNP_RETURN_CODE_NO_SUCH_PORT == ret) {
			vty_out(vty, "%% slot no or port no is not legal.\n");
		}else if (DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR == ret) {
			vty_out(vty, "%% Port %d/%d is member of trunk.\n", slot_no, local_port_no);
		}else if (DHCP_SNP_RETURN_CODE_ALREADY_SET == ret) {
			vty_out(vty, "%% Port already setted option strategy %s mode.\n",
					(DCLI_DHCP_SNP_OPT82_STRATEGY_TYPE_DROP == strategy_mode) ? "drop" :
						((DCLI_DHCP_SNP_OPT82_STRATEGY_TYPE_KEEP == strategy_mode) ? "keep" : 
							((DCLI_DHCP_SNP_OPT82_STRATEGY_TYPE_REPLACE == strategy_mode) ? "replace" : "unknown")));
		}else {
			vty_out(vty, "%% Unknown Error! return %x \n", ret);
		}

		dbus_message_unref(reply);
		return CMD_WARNING;
	}else {
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/*****************************************************************
 *	command: config dhcp-snooping information circuit-id default
 *
 *	note      : Config DHCP-Snooping circuit-ID content of option 82 on port
 *
 *	cmd Node: vlan Node
 *
******************************************************************/
DEFUN(config_dhcp_snp_opt82_set_port_circuitid_cmd_func1,
	config_dhcp_snp_opt82_set_port_circuitid_cmd1,
	"config dhcp-snooping information circuit-id PORTNO default",
	CONFIG_STR
	DCLI_DHCP_SNP_STR
	DCLI_DHCP_SNP_OPT82_STR
	DCLI_DHCP_SNP_OPT82_CIRCUITID_STR
	CONFIG_ETHPORT_STR
	"Config DHCP-Snooping Circuit-ID content of option 82 is default\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int str_size = DCLI_DHCP_SNP_INIT_0;
	unsigned char circuitid_type = DCLI_DHCP_SNP_OPT82_CIRCUITID_TYPE_INVALID;
	unsigned char *circuitid_str = NULL;
	unsigned char slot_no = DCLI_DHCP_SNP_INIT_0;
	unsigned char local_port_no = DCLI_DHCP_SNP_INIT_0;
	unsigned int t_slotno = DCLI_DHCP_SNP_INIT_0;
	unsigned int t_portno = DCLI_DHCP_SNP_INIT_0;
	unsigned short vlanId = DCLI_DHCP_SNP_INIT_0;
	unsigned int   op_ret = DCLI_DHCP_SNP_INIT_0;
	unsigned int nodesave = DCLI_DHCP_SNP_INIT_0;

	if (argc > 1) {
		vty_out(vty,"%% Command without so many parameters!\n");
		return CMD_WARNING;
	}

	/* fetch the 1nd param : slotNo/portNo */
	op_ret = parse_slotno_localport((char *)argv[0], &t_slotno, &t_portno);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty, "% Bad parameter:Unknow portno format.\n");
		return CMD_WARNING;
	}
	else if (1 == op_ret){
		vty_out(vty, "% Bad parameter: Bad slot/port number.\n");
		return CMD_WARNING;
	}
	slot_no = (unsigned char)t_slotno;
	local_port_no = (unsigned char)t_portno;

	/* not support config port on slot0 */
	if(0  == slot_no) {
		vty_out(vty, "% Bad parameter: Bad slot/port number.\n");
		return CMD_WARNING;
	}

	nodesave = (unsigned int)(vty->index);
	vlanId = (unsigned short)nodesave;

	circuitid_str = (char*)malloc(DCLI_DHCP_SNP_CIRCUITID_STR_LEN);
	memset(circuitid_str, 0, DCLI_DHCP_SNP_CIRCUITID_STR_LEN);

	circuitid_type = DCLI_DHCP_SNP_OPT82_CIRCUITID_TYPE_DEFAULT;

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_PORT_CIRCUITID_CONTENT);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_UINT16, &vlanId,
							DBUS_TYPE_BYTE, &slot_no,
							DBUS_TYPE_BYTE, &local_port_no,
							DBUS_TYPE_BYTE, &circuitid_type,
							DBUS_TYPE_STRING, &circuitid_str,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		free(circuitid_str);
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{

		if (DHCP_SNP_RETURN_CODE_OK == ret) {
			dbus_message_unref(reply);
			free(circuitid_str);
			return CMD_SUCCESS;
		}else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
			vty_out(vty, "%% DHCP-Snooping not enabled global.\n");
		}else if (DHCP_SNP_RETURN_CODE_NOT_EN_VLAN == ret) {
			vty_out(vty, "%% DHCP-Snooping not enabled on vlan %d.\n", vlanId);
		}else if (DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST == ret) {
			vty_out(vty, "%% Vlan %d is not exist.\n", vlanId);
		}else if (DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST == ret) {
			vty_out(vty, "%% Port %d/%d is not member of vlan %d.\n", slot_no, local_port_no, vlanId);
		}else if (DHCP_SNP_RETURN_CODE_NO_SUCH_PORT == ret) {
			vty_out(vty, "%% slot no or port no is not legal.\n");
		}else if (DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR == ret) {
			vty_out(vty, "%% Port %d/%d is member of trunk.\n", slot_no, local_port_no);
		}else if (DHCP_SNP_RETURN_CODE_ALREADY_SET == ret) {
			vty_out(vty, "%% DHCP-Snooping already setted circuit-id default mode for information.\n");
		}else {
			vty_out(vty, "%% Unknown Error! return %x \n", ret);
		}
		dbus_message_unref(reply);
		free(circuitid_str);
		return CMD_WARNING;
	}else {
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	free(circuitid_str);
	return CMD_SUCCESS;
}


/*****************************************************************
 *	command: config dhcp-snooping information circuit-id string STRING
 *
 *	note      : Config DHCP-Snooping circuit-ID content of option 82 on port
 *
 *	cmd Node: vlan Node
 *
******************************************************************/
DEFUN(config_dhcp_snp_opt82_set_port_circuitid_cmd_func2,
	config_dhcp_snp_opt82_set_port_circuitid_cmd2,
	"config dhcp-snooping information circuit-id PORTNO string STRING",
	CONFIG_STR
	DCLI_DHCP_SNP_STR
	DCLI_DHCP_SNP_OPT82_STR
	DCLI_DHCP_SNP_OPT82_CIRCUITID_STR
	CONFIG_ETHPORT_STR
	"Config DHCP-Snooping Circuit-ID content of option 82 is user-defined string\n"
	"User-defined string of the Circuit-ID content, format for the ASCII string that range from 3 to 63 characters"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int str_size = DCLI_DHCP_SNP_INIT_0;
	unsigned char circuitid_type = DCLI_DHCP_SNP_OPT82_CIRCUITID_TYPE_INVALID;
	unsigned char *circuitid_str = NULL;
	unsigned char slot_no = DCLI_DHCP_SNP_INIT_0;
	unsigned char local_port_no = DCLI_DHCP_SNP_INIT_0;
	unsigned int t_slotno = DCLI_DHCP_SNP_INIT_0;
	unsigned int t_portno = DCLI_DHCP_SNP_INIT_0;
	unsigned short vlanId = DCLI_DHCP_SNP_INIT_0;
	unsigned int   op_ret = DCLI_DHCP_SNP_INIT_0;
	unsigned int nodesave = DCLI_DHCP_SNP_INIT_0;

	if (argc > 2) {
		vty_out(vty, "%% Command without so many parameters!\n");
		return CMD_WARNING;
	}

	/* fetch the 1nd param : slotNo/portNo */
	op_ret = parse_slotno_localport((char *)argv[0], &t_slotno, &t_portno);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty, "% Bad parameter:Unknow portno format.\n");
		return CMD_WARNING;
	}
	else if (1 == op_ret){
		vty_out(vty, "% Bad parameter: Bad slot/port number.\n");
		return CMD_WARNING;
	}
	slot_no = (unsigned char)t_slotno;
	local_port_no = (unsigned char)t_portno;

	/* not support config port on slot0 */
	if(0  == slot_no) {
		vty_out(vty, "% Bad parameter: Bad slot/port number.\n");
		return CMD_WARNING;
	}

	nodesave = (unsigned int)(vty->index);
	vlanId = (unsigned short)nodesave;

	circuitid_str = (char*)malloc(DCLI_DHCP_SNP_CIRCUITID_STR_LEN);
	memset(circuitid_str, 0, DCLI_DHCP_SNP_CIRCUITID_STR_LEN);

	circuitid_type = DCLI_DHCP_SNP_OPT82_CIRCUITID_TYPE_USERSTR;

	ret = dcli_dhcp_snp_circuitid_string_legal_check((char*)argv[1], strlen(argv[1]));
	if (1 == ret) {
		vty_out(vty, "%% Bad parameter, valid length of string must be in range [3-63]!\n");
		free(circuitid_str);
		return CMD_WARNING;
	}
	else if(2 == ret) {
		vty_out(vty, "%% Bad parameter, string begins with an illegal char!\n");
		free(circuitid_str);
		return CMD_WARNING;
	}
	else if(3 == ret) {
		vty_out(vty, "%% Bad parameter, string contains illegal char!\n");
		free(circuitid_str);
		return CMD_WARNING;
	}

	str_size = strlen(argv[1]);
	memcpy(circuitid_str, argv[1], str_size);

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_PORT_CIRCUITID_CONTENT);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_UINT16, &vlanId,
							DBUS_TYPE_BYTE, &slot_no,
							DBUS_TYPE_BYTE, &local_port_no,
							DBUS_TYPE_BYTE, &circuitid_type,
							DBUS_TYPE_STRING, &circuitid_str,
							DBUS_TYPE_INVALID);


	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		free(circuitid_str);
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{

		if (DHCP_SNP_RETURN_CODE_OK == ret) {
			dbus_message_unref(reply);
			free(circuitid_str);
			return CMD_SUCCESS;
		}else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
			vty_out(vty, "%% DHCP-Snooping not enabled global.\n");
		}else if (DHCP_SNP_RETURN_CODE_NOT_EN_VLAN == ret) {
			vty_out(vty, "%% DHCP-Snooping not enabled on vlan %d.\n", vlanId);
		}else if (DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST == ret) {
			vty_out(vty, "%% Vlan %d is not exist.\n", vlanId);
		}else if (DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST == ret) {
			vty_out(vty, "%% Port %d/%d is not member of vlan %d.\n", slot_no, local_port_no, vlanId);
		}else if (DHCP_SNP_RETURN_CODE_NO_SUCH_PORT == ret) {
			vty_out(vty, "%% slot no or port no is not legal.\n");
		}else if (DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR == ret) {
			vty_out(vty, "%% Port %d/%d is member of trunk.\n", slot_no, local_port_no);
		}else if (DHCP_SNP_RETURN_CODE_ALREADY_SET == ret) {
			vty_out(vty, "%% DHCP-Snooping already setted circuit-id default mode for information.\n");
		}else {
			vty_out(vty, "%% Unknown Error! return %x \n", ret);
		}
		dbus_message_unref(reply);
		free(circuitid_str);
		return CMD_WARNING;
	}else {
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	free(circuitid_str);
	return CMD_SUCCESS;
}

/*****************************************************************
 *	command: config dhcp-snooping information remote-id default
 *
 *	note      : Config DHCP-Snooping Remote-ID content of option 82 on port
 *
 *	cmd Node: vlan Node
 *
******************************************************************/
DEFUN(config_dhcp_snp_opt82_set_port_remoteid_cmd_func1,
	config_dhcp_snp_opt82_set_port_remoteid_cmd1,
	"config dhcp-snooping information remote-id PORTNO default",
	CONFIG_STR
	DCLI_DHCP_SNP_STR
	DCLI_DHCP_SNP_OPT82_STR
	DCLI_DHCP_SNP_OPT82_REMOTEID_STR
	CONFIG_ETHPORT_STR
	"Config DHCP-Snooping Remote-ID content of option 82 is default\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int str_size = DCLI_DHCP_SNP_INIT_0;
	unsigned char remoteid_type = DCLI_DHCP_SNP_OPT82_REMOTEID_TYPE_INVALID;
	unsigned char *remoteid_str = NULL;
	unsigned char slot_no = DCLI_DHCP_SNP_INIT_0;
	unsigned char local_port_no = DCLI_DHCP_SNP_INIT_0;
	unsigned int t_slotno = DCLI_DHCP_SNP_INIT_0;
	unsigned int t_portno = DCLI_DHCP_SNP_INIT_0;
	unsigned short vlanId = DCLI_DHCP_SNP_INIT_0;
	unsigned int   op_ret = DCLI_DHCP_SNP_INIT_0;
	unsigned int nodesave = DCLI_DHCP_SNP_INIT_0;

	if (argc > 1) {
		vty_out(vty, "%% Command without so many parameters!\n");
		return CMD_WARNING;
	}

	/* fetch the 1nd param : slotNo/portNo */
	op_ret = parse_slotno_localport((char *)argv[0], &t_slotno, &t_portno);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty, "% Bad parameter:Unknow portno format.\n");
		return CMD_WARNING;
	}
	else if (1 == op_ret){
		vty_out(vty, "% Bad parameter: Bad slot/port number.\n");
		return CMD_WARNING;
	}
	slot_no = (unsigned char)t_slotno;
	local_port_no = (unsigned char)t_portno;

	/* not support config port on slot0 */
	if(0  == slot_no) {
		vty_out(vty, "% Bad parameter: Bad slot/port number.\n");
		return CMD_WARNING;
	}

	nodesave = (unsigned int)(vty->index);
	vlanId = (unsigned short)nodesave;

	remoteid_str = (char*)malloc(DCLI_DHCP_SNP_REMOTEID_STR_LEN);
	memset(remoteid_str, 0, DCLI_DHCP_SNP_REMOTEID_STR_LEN);

	remoteid_type = DCLI_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC;


	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_PORT_REMOTEID_CONTENT);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_UINT16, &vlanId,
							DBUS_TYPE_BYTE, &slot_no,
							DBUS_TYPE_BYTE, &local_port_no,
							DBUS_TYPE_BYTE, &remoteid_type,
							DBUS_TYPE_STRING, &remoteid_str,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		free(remoteid_str);
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{

		if (DHCP_SNP_RETURN_CODE_OK == ret) {
			dbus_message_unref(reply);
			free(remoteid_str);
			return CMD_SUCCESS;
		}else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
			vty_out(vty, "%% DHCP-Snooping not enabled global.\n");
		}else if (DHCP_SNP_RETURN_CODE_NOT_EN_VLAN == ret) {
			vty_out(vty, "%% DHCP-Snooping not enabled on vlan %d.\n", vlanId);
		}else if (DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST == ret) {
			vty_out(vty, "%% Vlan %d is not exist.\n", vlanId);
		}else if (DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST == ret) {
			vty_out(vty, "%% Port %d/%d is not member of vlan %d.\n", slot_no, local_port_no, vlanId);
		}else if (DHCP_SNP_RETURN_CODE_NO_SUCH_PORT == ret) {
			vty_out(vty, "%% slot no or port no is not legal.\n");
		}else if (DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR == ret) {
			vty_out(vty, "%% Port %d/%d is member of trunk.\n", slot_no, local_port_no);
		}else if (DHCP_SNP_RETURN_CODE_ALREADY_SET == ret) {
			vty_out(vty, "%% DHCP-Snooping already setted remote-id default mode for information.\n");
		}else {
			vty_out(vty, "%% Unknown Error! return %x \n", ret);
		}
		dbus_message_unref(reply);
		free(remoteid_str);
		return CMD_WARNING;
	}
	else {
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	free(remoteid_str);
	return CMD_SUCCESS;
}


/*****************************************************************
 *	command: config dhcp-snooping information remote-id string STRING
 *
 *	note      : Config DHCP-Snooping Remote-ID content of option 82 on port
 *
 *	cmd Node: vlan Node
 *
******************************************************************/
DEFUN(config_dhcp_snp_opt82_set_port_remoteid_cmd_func2,
	config_dhcp_snp_opt82_set_port_remoteid_cmd2,
	"config dhcp-snooping information remote-id PORTNO string STRING",
	CONFIG_STR
	DCLI_DHCP_SNP_STR
	DCLI_DHCP_SNP_OPT82_STR
	DCLI_DHCP_SNP_OPT82_REMOTEID_STR
	CONFIG_ETHPORT_STR
	"Config DHCP-Snooping Remote-ID content of option 82 is user-defined string\n"
	"User-defined string of the Remote-ID content, format for the ASCII string that range from 3 to 63 characters"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int str_size = DCLI_DHCP_SNP_INIT_0;
	unsigned char remoteid_type = DCLI_DHCP_SNP_OPT82_REMOTEID_TYPE_INVALID;
	unsigned char *remoteid_str = NULL;
	unsigned char slot_no = DCLI_DHCP_SNP_INIT_0;
	unsigned char local_port_no = DCLI_DHCP_SNP_INIT_0;
	unsigned int t_slotno = DCLI_DHCP_SNP_INIT_0;
	unsigned int t_portno = DCLI_DHCP_SNP_INIT_0;
	unsigned short vlanId = DCLI_DHCP_SNP_INIT_0;
	unsigned int   op_ret = DCLI_DHCP_SNP_INIT_0;
	unsigned int nodesave = DCLI_DHCP_SNP_INIT_0;

	if (argc > 2) {
		vty_out(vty, "%% Command without so many parameters!\n");
		return CMD_WARNING;
	}

	/* fetch the 1nd param : slotNo/portNo */
	op_ret = parse_slotno_localport((char *)argv[0], &t_slotno, &t_portno);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty, "% Bad parameter:Unknow portno format.\n");
		return CMD_WARNING;
	}
	else if (1 == op_ret){
		vty_out(vty, "% Bad parameter: Bad slot/port number.\n");
		return CMD_WARNING;
	}
	slot_no = (unsigned char)t_slotno;
	local_port_no = (unsigned char)t_portno;

	/* not support config port on slot0 */
	if(0  == slot_no) {
		vty_out(vty, "% Bad parameter: Bad slot/port number.\n");
		return CMD_WARNING;
	}

	nodesave = (unsigned int)(vty->index);
	vlanId = (unsigned short)nodesave;

	remoteid_str = (char*)malloc(DCLI_DHCP_SNP_REMOTEID_STR_LEN);
	memset(remoteid_str, 0, DCLI_DHCP_SNP_REMOTEID_STR_LEN);

	remoteid_type = DCLI_DHCP_SNP_OPT82_REMOTEID_TYPE_USERSTR;

	/* check remote-id string */
	ret = dcli_dhcp_snp_circuitid_string_legal_check((char*)argv[1], strlen(argv[1]));
	if (1 == ret) {
		vty_out(vty, "%% Bad parameter, valid length of string must be in range [3-63]!\n");
		free(remoteid_str);
		return CMD_WARNING;
	}
	else if(2 == ret) {
		vty_out(vty, "%% Bad parameter, string begins with an illegal char!\n");
		free(remoteid_str);
		return CMD_WARNING;
	}
	else if(3 == ret) {
		vty_out(vty, "%% Bad parameter, string contains illegal char!\n");
		free(remoteid_str);
		return CMD_WARNING;
	}

	str_size = strlen(argv[1]);
	memcpy(remoteid_str, argv[1], str_size);

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_SET_OPT82_PORT_REMOTEID_CONTENT);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_UINT16, &vlanId,
							DBUS_TYPE_BYTE, &slot_no,
							DBUS_TYPE_BYTE, &local_port_no,
							DBUS_TYPE_BYTE, &remoteid_type,
							DBUS_TYPE_STRING, &remoteid_str,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		free(remoteid_str);
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{

		if (DHCP_SNP_RETURN_CODE_OK == ret) {
			dbus_message_unref(reply);
			free(remoteid_str);
			return CMD_SUCCESS;
		}else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
			vty_out(vty, "%% DHCP-Snooping not enabled global.\n");
		}else if (DHCP_SNP_RETURN_CODE_NOT_EN_VLAN == ret) {
			vty_out(vty, "%% DHCP-Snooping not enabled on vlan %d.\n", vlanId);
		}else if (DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST == ret) {
			vty_out(vty, "%% Vlan %d is not exist.\n", vlanId);
		}else if (DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST == ret) {
			vty_out(vty, "%% Port %d/%d is not member of vlan %d.\n", slot_no, local_port_no, vlanId);
		}else if (DHCP_SNP_RETURN_CODE_NO_SUCH_PORT == ret) {
			vty_out(vty, "%% slot no or port no is not legal.\n");
		}else if (DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR == ret) {
			vty_out(vty, "%% Port %d/%d is member of trunk.\n", slot_no, local_port_no);
		}else if (DHCP_SNP_RETURN_CODE_ALREADY_SET == ret) {
			vty_out(vty, "%% DHCP-Snooping already setted remote-id default mode for information.\n");
		}else {
			vty_out(vty, "%% Unknown Error! return %x \n", ret);
		}
		dbus_message_unref(reply);
		free(remoteid_str);
		return CMD_WARNING;
	}
	else{
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	free(remoteid_str);
	return CMD_SUCCESS;
}

/***************************************************************************************************************
 *	command : dhcp-snooping (add|delete) static-binding ip-address A.B.C.D mac MAC vlan <1-4094> eth-port PORTNO
 *
 *	note    : add/delete dhcp-snooping static-binding item to/from bind table
 *
 *	cmd Node: config Node
 *
***************************************************************************************************************/
DEFUN(config_dhcp_snp_add_delete_static_binding_cmd_func,
	config_dhcp_snp_add_delete_static_binding_cmd,
	"dhcp-snooping static-binding (add|delete) ip-address A.B.C.D mac MAC vlan <1-4094> eth-port PORTNO",
	DCLI_DHCP_SNP_STR
	DCLI_DHCP_SNP_ADD_BINDING_STR
	DCLI_DHCP_SNP_DEL_BINDING_STR
	DCLI_DHCP_SNP_STATIC_BINDING_STR
	DCLI_DHCP_SNP_IP_STR
	DCLI_DHCP_SNP_IP_ADDRESS_STR
	DCLI_DHCP_SNP_MAC_STR
	DCLI_DHCP_SNP_MAC_ADDRESS_STR
	"Config layer 2 vlan entity\n"
	"Specify vlan id for vlan entity\n"
	"Port that in System\n"
	CONFIG_ETHPORT_STR
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int opt_type = DCLI_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE_INVALID;

	unsigned short vlanId = DCLI_DHCP_SNP_INIT_0;
	unsigned char slot_no = DCLI_DHCP_SNP_INIT_0;
	unsigned char local_port_no = DCLI_DHCP_SNP_INIT_0;
	unsigned int t_slotno = DCLI_DHCP_SNP_INIT_0;
	unsigned int t_portno = DCLI_DHCP_SNP_INIT_0;

	unsigned long ip      = DCLI_DHCP_SNP_INIT_0;
	ETHERADDR macAddr;
	memset(&macAddr, 0, sizeof(ETHERADDR));

	if (argc > 5) {
		vty_out(vty, "%% Command without so many parameters!\n");;
		return CMD_WARNING;
	}

	/* get operate type */
	if (strncmp(argv[0], "add", strlen(argv[0])) == 0) {
		opt_type = DCLI_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE_ADD;
	}
	else if (strncmp(argv[0], "delete", strlen(argv[0])) == 0) {
		opt_type = DCLI_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE_DEL;
	}else {
		vty_out(vty, "%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}

	/* get IP address */
	if(1 != inet_atoi(argv[1], &ip)) {
		vty_out(vty, "%% Can't get IP address\n");
		return CMD_WARNING;
	}

	/* get MAC address */
	ret = parse_mac_addr((char *)argv[2], &macAddr);	
	if (NPD_FAIL == ret) {
    	vty_out(vty, "%% Unknow MAC address format!\n");
		return CMD_WARNING;
	}

	/* get vlanID */
	ret = parse_vlan_no((char*)argv[3], &vlanId);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"% Bad parameter: vlan id illegal!\n");
		return CMD_WARNING;
	}

	/* fetch slotNo/portNo */
	ret = parse_slotno_localport((char *)argv[4], &t_slotno, &t_portno);
	if (NPD_FAIL == ret) {
    	vty_out(vty, "% Bad parameter: Unknow slot/port format.\n");
		return CMD_WARNING;
	}
	else if (1 == ret){
		vty_out(vty, "% Bad parameter: Bad slot/port number.\n");
		return CMD_WARNING;
	}
	slot_no = (unsigned char)t_slotno;
	local_port_no = (unsigned char)t_portno;

	/* not support config port on slot0 */
	if(0 == slot_no) {
		vty_out(vty, "% Bad parameter: Bad slot/port number.\n");
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_ADD_DEL_STATIC_BINDING);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &opt_type,
							DBUS_TYPE_UINT32, &ip,
							DBUS_TYPE_BYTE, &macAddr.arEther[0],
							DBUS_TYPE_BYTE, &macAddr.arEther[1],
							DBUS_TYPE_BYTE, &macAddr.arEther[2],
							DBUS_TYPE_BYTE, &macAddr.arEther[3],
							DBUS_TYPE_BYTE, &macAddr.arEther[4],
							DBUS_TYPE_BYTE, &macAddr.arEther[5],
							DBUS_TYPE_UINT16, &vlanId,
							DBUS_TYPE_BYTE, &slot_no,
							DBUS_TYPE_BYTE, &local_port_no,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		if (DHCP_SNP_RETURN_CODE_OK == ret) {
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
			vty_out(vty, "%% DHCP-Snooping not enabled global.\n");
		}else if (DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST == ret) {
			vty_out(vty, "%% Vlan %d is not exist.\n", vlanId);
		}else if (DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST == ret) {
			vty_out(vty, "%% Port %d/%d is not member of vlan %d.\n", slot_no, local_port_no, vlanId);
		}else if (DHCP_SNP_RETURN_CODE_NO_SUCH_PORT == ret) {
			vty_out(vty, "%% slot no or port no is not legal.\n");
		}else if (DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR == ret) {
			vty_out(vty, "%% Port %d/%d is member of trunk.\n", slot_no, local_port_no);
		}else if (DHCP_SNP_RETURN_CODE_ALREADY_SET == ret) {
			if (DCLI_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE_ADD == opt_type) {
				vty_out(vty, "%% MAC has already in binding table.\n");
			}
			else if (DCLI_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE_DEL == opt_type) {
				vty_out(vty, "%% MAC has not in binding table.\n");
			}
		}else {
			vty_out(vty, "%% Unknown Error! return %x \n", ret);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	else{
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;	
}

/***************************************************************************************************************
 *	command : dhcp-snooping binding add MAC A.B.C.D <1-4094> PORTINDEX SYSTIEM LESAE BINDTYPE
 *
 *	note    : add dhcp-snooping binding item to bind table
 *				inner interface in dhcp snooping, for save content of binding table
 *				by command "write" before reboot
 *
 *	cmd Node: config Node
 *
***************************************************************************************************************/
DEFUN(config_dhcp_snp_add_binding_cmd_func,
	config_dhcp_snp_add_binding_cmd,
	"dhcp-snooping binding add MAC A.B.C.D <1-4094> PORTINDEX SYSTIEM LEASE BINDTYPE",
	DCLI_DHCP_SNP_STR
	DCLI_DHCP_SNP_BINDING_STR
	DCLI_DHCP_SNP_ADD_BINDING_STR
	DCLI_DHCP_SNP_MAC_ADDRESS_STR
	DCLI_DHCP_SNP_IP_ADDRESS_STR
	"Specify vlan id for vlan entity\n"
	"Portindex that in System\n"
	"Add a binding table entry when the system starts ever since the time of"
	"Lease time"
	"Type of bind item"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	ETHERADDR macAddr;
	memset(&macAddr, 0, sizeof(ETHERADDR));
	unsigned long ip      = DCLI_DHCP_SNP_INIT_0;
	unsigned short vlanId = DCLI_DHCP_SNP_INIT_0;
	unsigned int eth_g_index = DCLI_DHCP_SNP_INIT_0;
	unsigned int system_time = DCLI_DHCP_SNP_INIT_0;
	unsigned int lease_time = DCLI_DHCP_SNP_INIT_0;
	unsigned int bind_type = DCLI_DHCP_SNP_INIT_0;

	if (argc > 7) {
		vty_out(vty, "%% Command without so many parameters!\n");;
		return CMD_WARNING;
	}

	/* get MAC address */
	ret = parse_mac_addr((char *)argv[0], &macAddr);	
	if (NPD_FAIL == ret) {
    	vty_out(vty, "%% Unknow MAC address format!\n");
		return CMD_WARNING;
	}

	/* get IP address */
	if(1 != inet_atoi(argv[1], &ip)) {
		vty_out(vty, "%% Can't get IP address\n");
		return CMD_WARNING;
	}

	/* get vlanID */
	ret = parse_vlan_no((char*)argv[2], &vlanId);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"% Bad parameter: vlan id illegal!\n");
		return CMD_WARNING;
	}

	/* get eth_g_index */
	ret = parse_param_no((char *)argv[3], &eth_g_index);
	if (COMMON_ERROR == ret) {
    	vty_out(vty, "%% eth-port global index %s illegal.\n", argv[3]);
		return CMD_WARNING;
	}

	/* get system_time */
	ret = parse_param_no((char *)argv[4], &system_time);
	if (COMMON_ERROR == ret) {
    	vty_out(vty, "%% system time %s illegal.\n", argv[4]);
		return CMD_WARNING;
	}

	/* get lease time */
	ret = parse_param_no((char *)argv[5], &lease_time);
	if (COMMON_ERROR == ret) {
    	vty_out(vty, "%% lease time %s illegal.\n", argv[5]);
		return CMD_WARNING;
	}

	/* get bind_type */
	ret = parse_param_no((char *)argv[6], &bind_type);
	if (COMMON_ERROR == ret) {
    	vty_out(vty, "%% Type of bind %s illegal.\n", argv[6]);
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										NPD_DBUS_DHCP_SNP_OBJPATH,
										NPD_DBUS_DHCP_SNP_INTERFACE,
										NPD_DBUS_DHCP_SNP_METHOD_ADD_BINDING);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_BYTE, &macAddr.arEther[0],
							DBUS_TYPE_BYTE, &macAddr.arEther[1],
							DBUS_TYPE_BYTE, &macAddr.arEther[2],
							DBUS_TYPE_BYTE, &macAddr.arEther[3],
							DBUS_TYPE_BYTE, &macAddr.arEther[4],
							DBUS_TYPE_BYTE, &macAddr.arEther[5],
							DBUS_TYPE_UINT32, &ip,
							DBUS_TYPE_UINT16, &vlanId,
							DBUS_TYPE_UINT32, &eth_g_index,
							DBUS_TYPE_UINT32, &system_time,
							DBUS_TYPE_UINT32, &lease_time,
							DBUS_TYPE_UINT32, &bind_type,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		if (DHCP_SNP_RETURN_CODE_OK == ret) {
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
			vty_out(vty, "%% DHCP-Snooping not enabled global.\n");
		}else if (DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST == ret) {
			vty_out(vty, "%% Vlan %d is not exist.\n", vlanId);
		}else if (DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST == ret) {
			vty_out(vty, "%% Port %d is not member of vlan %d.\n", eth_g_index, vlanId);
		}else {
			vty_out(vty, "%% Unknown Error! return %x \n", ret);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	else{
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;	
}
int
dcli_dhcp_check_snp_interface
(
	char *ifname
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	char *name=NULL;
	unsigned int op_ret = 0;
	unsigned int localid = 1, slot_id = HostSlotId, indextmp = 0;
	//get_slotid_index(vty, &indextmp, &slot_id, &localid);
	slot_id = get_slot_id_by_ifname(ifname);
	if(0 == slot_id){
		printf("slot_id is error!\n");
		return 0;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	if(slot_id >= 0){
	ReInitDbusConnection(&dcli_dbus_connection, slot_id, distributFag);
	}
	query = dbus_message_new_method_call(DHCPSNP_DBUS_BUSNAME,	
										DHCPSNP_DBUS_OBJPATH,
										DHCPSNP_DBUS_INTERFACE, 
										DHCPSNP_DBUS_METHOD_CHECK_SNP_INTERFACE_VE);								
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
		if(op_ret){
		}
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	memset(ifname, 0, IF_NAMESIZE);
	memcpy(ifname, name, strlen(name));
	
	return 0;
}


/*********************************************************
 *	command: config dhcp-snooping IFNAME (enable|disable)
 *
 *	note : config dhcp-snooping on special interface
 *
 *	cmd Node: config node
 *
**********************************************************/
DEFUN(config_dhcp_snp_wan_set_intf_cmd_func,
	config_dhcp_snp_wan_set_intf_cmd,
	"config dhcp-snooping IFNAME (enable|disable)",
	CONFIG_STR
	DCLI_DHCP_SNP_STR
	"Config dhcp snooping interface name\n"
	"Config dhcp snooping enable\n"
	"Config dhcp snooping disable\n"
)
{
	unsigned int  ret = DHCP_SNP_RETURN_CODE_OK, ifindex = ~0UI;
	char ifname[IF_NAMESIZE] = {0}, name[IF_NAMESIZE] = {0};
	int flag = DCLI_DHCP_SNP_DISABLE, argi = 0 ,Ret = 0;

	if((CONFIG_NODE == vty->node) || (HANSI_NODE == vty->node) || (LOCAL_HANSI_NODE == vty->node)) {
		/* get 1st argument */
		if(2 == argc) {
			memcpy(ifname, argv[0], strlen((char*)argv[0]));
			argi = 1;
		}
		else {
			vty_out(vty, "%% Command argument error!\n");
			return CMD_WARNING;
		}
	}
	else if(INTERFACE_NODE == vty->node) {
		//ifname = vlan_eth_port_ifname;
		memcpy(ifname, vlan_eth_port_ifname, strlen((char*)vlan_eth_port_ifname));
		argi = 0;
	}
	else {
		vty_out(vty, "%% Wrong command node for dhcp snooping config!\n");
		return CMD_WARNING;
	}
	/*
	ifindex = if_nametoindex(ifname);
	if(!ifindex) {
		vty_out(vty, "%% Interface %s not correct!\n", ifname);
		return CMD_WARNING;
	}
	*/
	Ret = dcli_dhcp_snp_convert_ifname_check(vty, ifname);
	if(2 == Ret)
	dcli_dhcp_snp_convert_ifname(vty, ifname, name);
	else if(1 == Ret){
		sprintf(name, "%s", ifname);
	}else{
		vty_out(vty, "%% Wrong interfacename for dhcp snooping config!\n");
		return CMD_WARNING;
	}
	if(0 == strncmp(name, "ve", 2))
	dcli_dhcp_check_snp_interface(name);	
	
	/* get 2nd argument : enable flag */
	if (!strncmp(argv[argi], "enable", strlen(argv[argi]))) {
		flag = DCLI_DHCP_SNP_ENABLE;
	}
	if(!strncmp(argv[argi], "disable", strlen(argv[argi]))) {
		flag = DCLI_DHCP_SNP_DISABLE;
	}

	ret = dcli_dhcp_snp_config_intf(vty, name, flag);

	if (DHCP_SNP_RETURN_CODE_OK == ret) {
		
	}else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
		vty_out(vty, "%% DHCP snooping not enabled global.\n");
	}else if (DHCP_SNP_RETURN_CODE_NOT_EN_INTF == ret) {
		vty_out(vty, "%% DHCP snooping not enabled on interface %s.\n", name);
	}else if (DHCP_SNP_RETURN_CODE_EN_INTF == ret) {
		vty_out(vty, "%% DHCP snooping already enabled on interface %s.\n", name);
	}else if (DHCP_SNP_RETURN_CODE_NO_SUCH_INTF == ret) {
		vty_out(vty, "%% Interface %s not correct or not exist.\n", name ? name : "nil");
	}else if (DHCP_SNP_RETURN_CODE_EN_ANTI_ARP == ret) {
		vty_out(vty, "%% Interface %s enable anti-arp-spoof, disable anti-arp-spoof first.\n", name ? name : "nil");
	}else if (DHCP_SNP_RETURN_CODE_EN_ADD_ROUTE== ret) {
		vty_out(vty, "%% Interface %s enable add-route, disable add-route first.\n", name ? name : "nil");
	}
	else{
		vty_out(vty, "%% Config error %#x\n", ret);
	}

	return CMD_SUCCESS;
}

ALIAS(config_dhcp_snp_wan_set_intf_cmd_func,
	config_dhcp_snp_wan_enable_intf_cmd,
	"config dhcp-snooping (enable|disable)",
	CONFIG_STR
	DCLI_DHCP_SNP_STR
	"Config dhcp snooping enable\n"
	"Config dhcp snooping disable\n"
);

/*********************************************************
 *	command: config dhcp-snooping IFNAME add router to host(enable|disable)
 *
 *	note : config dhcp-snooping on special interface
 *
 *	cmd Node: config node
 *
**********************************************************/
DEFUN(config_dhcp_snp_add_router_intf_cmd_func,
	config_dhcp_snp_add_router_intf_cmd,
	"config dhcp-snooping add-router IFNAME (enable|disable)",
	CONFIG_STR
	DCLI_DHCP_SNP_STR
	"Config dhcp snooping add-router\n"
	"Config dhcp snooping add-router interface name\n"
	"Config dhcp snooping add-router enable\n"
	"Config dhcp snooping add-router disable\n"
)
{
	unsigned int  ret = DHCP_SNP_RETURN_CODE_OK, ifindex = ~0UI;
	char *ifname = NULL, name[IF_NAMESIZE] = {0};
	int flag = DCLI_DHCP_SNP_DISABLE, argi = 0,Ret = 0;
	int add_router_type = -1;

	if((CONFIG_NODE == vty->node) || (HANSI_NODE == vty->node) || (LOCAL_HANSI_NODE == vty->node)) {
		/* get 1st argument */
		if(2 == argc) {
			ifname = (char*)argv[0];
			argi = 1;
		}
		else {
			vty_out(vty, "%% Command argument error!\n");
			return CMD_WARNING;
		}
	}
	else if(INTERFACE_NODE == vty->node) {
		ifname = vlan_eth_port_ifname;
		argi = 0;
	}
	else {
		vty_out(vty, "%% Wrong command node for dhcp snooping config!\n");
		return CMD_WARNING;
	}
	/*
	ifindex = if_nametoindex(ifname);
	if(!ifindex) {
		vty_out(vty, "%% Interface %s not correct or not exists!\n", ifname);
		return CMD_WARNING;
	}
	*/
	Ret = dcli_dhcp_snp_convert_ifname_check(vty, ifname);
	if(2 == Ret)
	dcli_dhcp_snp_convert_ifname(vty, ifname, name);
	else if(1 == Ret){
		sprintf(name, "%s", ifname);
	}else{
		vty_out(vty, "%% Wrong interfacename for dhcp snooping config!\n");
		return CMD_WARNING;
	}
	if(0 == strncmp(name, "ve", 2))
	dcli_dhcp_check_snp_interface(name);
	
	/* get 2nd argument : enable flag */
	if (!strncmp(argv[argi], "enable", strlen(argv[argi]))) {
		flag = DCLI_DHCP_SNP_ENABLE;
	}
	if(!strncmp(argv[argi], "disable", strlen(argv[argi]))) {
		flag = DCLI_DHCP_SNP_DISABLE;
	}

	/* anti-arp-spoof type : 0 - via dhcp snooping interface, 1 - via normal interface */
	add_router_type = 0;

	ret = dcli_dhcp_snp_add_router_config_intf(vty, name, flag, add_router_type);

	if (DHCP_SNP_RETURN_CODE_OK == ret) {
		
	}else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
		vty_out(vty, "%% DHCP snooping not enabled global.\n");
	}else if (DHCP_SNP_RETURN_CODE_NOT_EN_INTF == ret) {
		vty_out(vty, "%% DHCP snooping not enabled on interface %s.\n", ifname);
	}else if (DHCP_SNP_RETURN_CODE_EN_INTF == ret) {
		vty_out(vty, "%% DHCP snooping already enabled on interface %s.\n", ifname);
	}else if (DHCP_SNP_RETURN_CODE_NO_SUCH_INTF == ret) {
		vty_out(vty, "%% Interface %s not correct or not exist.\n", ifname ? ifname : "nil");
	}else if (DHCP_SNP_RETURN_CODE_EN_ADD_ROUTE== ret) {
		vty_out(vty, "%% Interface %s enable add-route, disable add-route first.\n", name ? name : "nil");
	}else{
		vty_out(vty, "%% Config error %#x\n", ret);
	}

	return CMD_SUCCESS;
}

/*********************************************************
 *	command: config dhcp-snooping IFNAME anti-arp-spoof (enable|disable)
 *
 *	note : config dhcp-snooping on special interface
 *
 *	cmd Node: config node
 *
**********************************************************/
DEFUN(config_dhcp_snp_wan_set_arp_intf_cmd_func,
	config_dhcp_snp_wan_set_arp_intf_cmd,
	"config dhcp-snooping anti-arp-spoof IFNAME (enable|disable)",
	CONFIG_STR
	DCLI_DHCP_SNP_STR
	"Config dhcp snooping anti arp spoof\n"
	"Config dhcp snooping anti arp spoof interface name\n"
	"Config dhcp snooping anti arp spoof enable\n"
	"Config dhcp snooping anti arp spoof disable\n"
)
{
	unsigned int  ret = DHCP_SNP_RETURN_CODE_OK, ifindex = ~0UI;
	char *ifname = NULL, name[IF_NAMESIZE] = {0};
	int flag = DCLI_DHCP_SNP_DISABLE, argi = 0,Ret = 0;
	int anti_arps_type = -1;

	if((CONFIG_NODE == vty->node) || (HANSI_NODE == vty->node) || (LOCAL_HANSI_NODE == vty->node)) {
		/* get 1st argument */
		if(2 == argc) {
			ifname = (char*)argv[0];
			argi = 1;
		}
		else {
			vty_out(vty, "%% Command argument error!\n");
			return CMD_WARNING;
		}
	}
	else if(INTERFACE_NODE == vty->node) {
		ifname = vlan_eth_port_ifname;
		argi = 0;
	}
	else {
		vty_out(vty, "%% Wrong command node for dhcp snooping config!\n");
		return CMD_WARNING;
	}
	/*
	ifindex = if_nametoindex(ifname);
	if(!ifindex) {
		vty_out(vty, "%% Interface %s not correct or not exists!\n", ifname);
		return CMD_WARNING;
	}
	*/

	Ret = dcli_dhcp_snp_convert_ifname_check(vty, ifname);
	if(2 == Ret)
	dcli_dhcp_snp_convert_ifname(vty, ifname, name);
	else if(1 == Ret){
		sprintf(name, "%s", ifname);
	}else{
		vty_out(vty, "%% Wrong interfacename for dhcp snooping config!\n");
		return CMD_WARNING;
	}
	if(0 == strncmp(name, "ve", 2))
	dcli_dhcp_check_snp_interface(name);
	
	/* get 2nd argument : enable flag */
	if (!strncmp(argv[argi], "enable", strlen(argv[argi]))) {
		flag = DCLI_DHCP_SNP_ENABLE;
	}
	if(!strncmp(argv[argi], "disable", strlen(argv[argi]))) {
		flag = DCLI_DHCP_SNP_DISABLE;
	}

	/* anti-arp-spoof type : 0 - via dhcp snooping interface, 1 - via normal interface */
	anti_arps_type = 0;

	ret = dcli_dhcp_snp_arp_config_intf(vty, name, flag, anti_arps_type);

	if (DHCP_SNP_RETURN_CODE_OK == ret) {
		
	}else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
		vty_out(vty, "%% DHCP snooping not enabled global.\n");
	}else if (DHCP_SNP_RETURN_CODE_NOT_EN_INTF == ret) {
		vty_out(vty, "%% DHCP snooping not enabled on interface %s.\n", ifname);
	}else if (DHCP_SNP_RETURN_CODE_EN_INTF == ret) {
		vty_out(vty, "%% DHCP snooping already enabled on interface %s.\n", ifname);
	}else if (DHCP_SNP_RETURN_CODE_NO_SUCH_INTF == ret) {
		vty_out(vty, "%% Interface %s not correct or not exist.\n", ifname ? ifname : "nil");
	}
	else{
		vty_out(vty, "%% Config error %#x\n", ret);
	}

	return CMD_SUCCESS;
}

ALIAS(config_dhcp_snp_wan_set_arp_intf_cmd_func,
	config_dhcp_snp_wan_arp_enable_intf_cmd,
	"config dhcp-snooping anti-arp-spoof (enable|disable)",
	CONFIG_STR
	DCLI_DHCP_SNP_STR
	"Config dhcp snooping anti arp spoof\n"
	"Config dhcp snooping anti arp spoof enable\n"
	"Config dhcp snooping anti arp spoof disable\n"
);

/*********************************************************
 *	command: config IFNAME anti-arp-spoof (enable|disable)
 *
 *	note : config anti-arp-spoof on special interface
 *
 *	cmd Node: config node
 *
**********************************************************/
DEFUN(config_anti_arp_spoof_func,
	config_anti_arp_spoof_cmd,
	"config anti-arp-spoof IFNAME (enable|disable)",
	CONFIG_STR
	"Config anti arp spoofing\n"
	"Config anti arp spoofing interface name\n"
	"Config interface anti arp spoofing enable\n"
	"Config interface anti arp spoofing disable\n"
)
{
	unsigned int  ret = DHCP_SNP_RETURN_CODE_OK, ifindex = ~0UI;
	char *ifname = NULL, name[IF_NAMESIZE] = {0};
	int flag = DCLI_DHCP_SNP_DISABLE, argi = 0;
	int anti_arps_type = -1;

	if(CONFIG_NODE == vty->node) {
		/* get 1st argument */
		if(2 == argc) {
			ifname = (char*)argv[0];
			argi = 1;
		}
		else {
			vty_out(vty, "%% Command argument error!\n");
			return CMD_WARNING;
		}
	}
	else if(INTERFACE_NODE == vty->node) {
		ifname = vlan_eth_port_ifname;
		argi = 0;
	}
	else {
		vty_out(vty, "%% Wrong command node for anti arp spoofing config!\n");
		return CMD_WARNING;
	}

	ifindex = if_nametoindex(ifname);
	if(!ifindex) {
		vty_out(vty, "%% Interface %s not correct or not exists!\n", ifname);
		return CMD_WARNING;
	}
	
	/* get 2nd argument : enable flag */
	if (!strncmp(argv[argi], "enable", strlen(argv[argi]))) {
		flag = DCLI_DHCP_SNP_ENABLE;
	}
	if(!strncmp(argv[argi], "disable", strlen(argv[argi]))) {
		flag = DCLI_DHCP_SNP_DISABLE;
	}

	/* anti-arp-spoof type : 0 - via dhcp snooping interface, 1 - via normal interface */
	anti_arps_type = 1;
	
	ret = dcli_dhcp_snp_arp_config_intf(vty, ifname, flag, anti_arps_type);

	if (DHCP_SNP_RETURN_CODE_OK == ret) {
		
	}else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
		vty_out(vty, "%% Anti arp spoofing not enabled on interface %s.\n", ifname);
	}else if (DHCP_SNP_RETURN_CODE_EN_INTF == ret) {
		vty_out(vty, "%% DHCP snooping already enabled on interface %s.\n", ifname);
	}else if (DHCP_SNP_RETURN_CODE_NO_SUCH_INTF == ret) {
		vty_out(vty, "%% Interface %s not correct or not exist.\n", ifname ? ifname : "nil");
	}
	else{
		vty_out(vty, "%% Config error %#x\n", ret);
	}

	return CMD_SUCCESS;
}

ALIAS(config_anti_arp_spoof_func,
		config_intf_anti_arp_spoof_cmd,
		"config anti-arp-spoof (enable|disable)",
		CONFIG_STR
		"Config anti arp spoofing\n"
		"Config interface anti arp spoofing enable\n"
		"Config interface anti arp spoofing disable\n"
);

int dcli_dhcp_snp_config_debug
(
	struct vty *vty,
	char *level,
	unsigned int enable
)
{
	unsigned int flag = 0, ret = 0;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	
	if(!vty || !level) {
		
		return CMD_WARNING;
	}

	if(0 == strncmp(level,"all",strlen(level))) {
		flag = DCLI_DEBUG_FLAG_ALL;
	}		
	else if(0 == strncmp(level,"error",strlen(level))) {
		flag = DCLI_DEBUG_FLAG_ERR;
	}
	else if (0 == strncmp(level,"warning",strlen(level))){
		flag = DCLI_DEBUG_FLAG_WAR;
	}
	else if (0 == strncmp(level,"info",strlen(level))){
		flag = DCLI_DEBUG_FLAG_EVT;		/* information */
	}	
	else if (0 == strncmp(level,"debug",strlen(level))){
		flag = DCLI_DEBUG_FLAG_DBG;
	}
	else if (0 == strncmp(level,"packet",strlen(level))){
		flag = DCLI_DEBUG_FLAG_PKT_ALL;
	}
	else {
		vty_out(vty,"%% Command parameter %s error!\n",level);
		return CMD_WARNING;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	dhcp_snp_reinitDbusConnection(&dcli_dbus_connection, vty);

	query = dbus_message_new_method_call(DHCPSNP_DBUS_BUSNAME,	\
										DHCPSNP_DBUS_OBJPATH,
										DHCPSNP_DBUS_INTERFACE, \
										DHCPSNP_DBUS_METHOD_DEBUG_STATE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &enable,
							DBUS_TYPE_UINT32, &flag,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {		
		vty_out(vty,"set dhcpsnp debug failed get reply.\n");	
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {

	}
	else {		
		vty_out(vty,"set dhcpsnp debug Failed get args.\n");		
		if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);
	
}
DEFUN(debug_dhcp_snp_info ,
		debug_dhcp_snp_info_cmd,
		"debug dhcp-snooping (all|error|warning|info|debug|packet)",
		DEBUG_STR
		MODULE_DEBUG_STR(dhcp-snooping)
		MODULE_DEBUG_LEVEL_STR(dhcp-snooping,all)
		MODULE_DEBUG_LEVEL_STR(dhcp-snooping,error)
		MODULE_DEBUG_LEVEL_STR(dhcp-snooping,warning)
		MODULE_DEBUG_LEVEL_STR(dhcp-snooping,info)		
		MODULE_DEBUG_LEVEL_STR(dhcp-snooping,debug)
		MODULE_DEBUG_LEVEL_STR(dhcp-snooping,packet)
)
{	
	int ret = 0;
	
	if(argc > 1) {
		vty_out(vty,"%% Command parameters number error!\n");
		return CMD_WARNING;
	}

	ret = dcli_dhcp_snp_config_debug(vty, (char*)argv[0], 1);
	
	return CMD_SUCCESS;
}

DEFUN(no_debug_dhcp_snp_info ,
		no_debug_dhcp_snp_info_cmd,
		"no debug dhcp-snooping (all|error|warning|info|debug|packet)",
		NO_STR
		DEBUG_STR
		MODULE_DEBUG_STR(dhcp-snooping)
		MODULE_DEBUG_LEVEL_STR(dhcp-snooping,all)
		MODULE_DEBUG_LEVEL_STR(dhcp-snooping,error)
		MODULE_DEBUG_LEVEL_STR(dhcp-snooping,warning)
		MODULE_DEBUG_LEVEL_STR(dhcp-snooping,info)		
		MODULE_DEBUG_LEVEL_STR(dhcp-snooping,debug)
		MODULE_DEBUG_LEVEL_STR(dhcp-snooping,packet)
)
{	
	int ret = 0;
	
	if(argc > 1) {
		vty_out(vty,"%% Command parameters number error!\n");
		return CMD_WARNING;
	}

	ret = dcli_dhcp_snp_config_debug(vty, (char*)argv[0], 0);
	
	return CMD_SUCCESS;
}

DEFUN(show_dhcp_snp_wan_bindtable_cmd_func,
	show_dhcp_snp_wan_bindtable_cmd,
	"show dhcp-snooping bind-table",
	SHOW_STR
	DCLI_DHCP_SNP_STR
	"Show DHCP-Snooping binding table\n"
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	ret = dcli_dhcp_snp_show_wan_bind_table(vty);
	if (DHCP_SNP_RETURN_CODE_OK == ret) {
		return CMD_SUCCESS;
	}else {
		vty_out(vty, "%% Show dhcp-snooping error!\n", ret);
		return CMD_WARNING;
	}
}
unsigned int 
dcli_dhcp_snp_del_route
(
	struct vty *vty ,
	char *name
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;

	DBusConnection *dcli_dbus_connection = NULL;
	dhcp_snp_reinitDbusConnection(&dcli_dbus_connection, vty);

	query = dbus_message_new_method_call(
								DHCPSNP_DBUS_BUSNAME,
								DHCPSNP_DBUS_OBJPATH,
								DHCPSNP_DBUS_INTERFACE,
								DHCPSNP_DBUS_METHOD_DELETE_HOST_ROUTER);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_STRING, &name,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		dbus_message_unref(reply);
		return ret;
	}
	else{
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;	

}
/*
*delete all the host routers added as for "config dhcp-snooping add router IFNAME "
*/
DEFUN(delete_dhcp_snp_router_cmd_func_all,
	delete_dhcp_snp_router_cmd_all,
	"delete dhcp-snooping host-router all",
	DCLI_DHCP_SNP_DEL_BINDING_STR
	DCLI_DHCP_SNP_STR
	"host-router\n"
	"delete dhcp-snooping host-router all\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	char ifname[IF_NAMESIZE] = {0};
	//ETHERADDR macAddr;
	//memset(&macAddr, 0, sizeof(ETHERADDR));
	ret = dcli_dhcp_snp_del_route(vty,ifname);
	if (DHCP_SNP_RETURN_CODE_ENABLE_GBL == ret) {
		return CMD_SUCCESS;
	}else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
		vty_out(vty, "%% DHCP-Snooping not enabled global.\n");
	}else if (DHCP_SNP_RETURN_CODE_NOT_FOUND == ret) {
		vty_out(vty, "%% Delete item not found.\n");
	}else {
		vty_out(vty, "%% Delete host-route failed\n");
	}

	return CMD_SUCCESS;
}

/*
*delete all the host routers added as for "config dhcp-snooping add router IFNAME "
*/
DEFUN(delete_dhcp_snp_router_cmd_func,
	delete_dhcp_snp_router_cmd,
	"delete dhcp-snooping host-router by IFNAME",
	DCLI_DHCP_SNP_DEL_BINDING_STR
	DCLI_DHCP_SNP_STR
	"host-router\n"
	"delete dhcp-snooping host-router by\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	char ifname[IF_NAMESIZE] = {0}, name[IF_NAMESIZE] = {0};
	int Ret = 0;
	//ETHERADDR macAddr;
	//memset(&macAddr, 0, sizeof(ETHERADDR));
	if((CONFIG_NODE == vty->node) || (HANSI_NODE == vty->node) || (LOCAL_HANSI_NODE == vty->node)) {
		/* get 1st argument */
		if(1 == argc){
				memcpy(ifname, argv[0], strlen((char*)argv[0]));
		}
		else {
			vty_out(vty, "%% Command without so many parameters!\n");
			return CMD_WARNING;
		}
	}
	else if(INTERFACE_NODE == vty->node) {
		//ifname = vlan_eth_port_ifname;
		memcpy(ifname, vlan_eth_port_ifname, strlen((char*)vlan_eth_port_ifname));
	}
	else {
		vty_out(vty, "%% Wrong command node for dhcp snooping config!\n");
		return CMD_WARNING;
	}
	Ret = dcli_dhcp_snp_convert_ifname_check(vty, ifname);
	if(2 == Ret)
	dcli_dhcp_snp_convert_ifname(vty, ifname, name);
	else if(1 == Ret){
		sprintf(name, "%s", ifname);
	}else{
		vty_out(vty, "%% Wrong interfacename for dhcp snooping config!\n");
		return CMD_WARNING;
	}
	if(0 == strncmp(name, "ve", 2))
	dcli_dhcp_check_snp_interface(name);
	
	ret = dcli_dhcp_snp_del_route(vty,name);
	if (DHCP_SNP_RETURN_CODE_ENABLE_GBL == ret) {
		return CMD_SUCCESS;
	}else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
		vty_out(vty, "%% DHCP-Snooping not enabled global.\n");
	}else if (DHCP_SNP_RETURN_CODE_NOT_FOUND == ret) {
		vty_out(vty, "%% Delete item not found.\n");
	}else {
		vty_out(vty, "%% Delete host-route failed\n");
	}

	return CMD_SUCCESS;	
}

DEFUN(delete_dhcp_snp_wan_bindtable_by_mac_cmd_func,
	delete_dhcp_snp_wan_bindtable_by_mac_cmd,
	"delete dhcp-snooping bind-table mac MAC",
	DCLI_DHCP_SNP_DEL_BINDING_STR
	DCLI_DHCP_SNP_STR
	"bind table\n"
	DCLI_DHCP_SNP_MAC_STR
	DCLI_DHCP_SNP_MAC_ADDRESS_STR
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	ETHERADDR macAddr;
	memset(&macAddr, 0, sizeof(ETHERADDR));
	if (argc > 1) {
		vty_out(vty, "%% Command without so many parameters!\n");;
		return CMD_WARNING;
	}
	ret = parse_mac_addr((char *)argv[0], &macAddr);	
	if (NPD_FAIL == ret) {
    	vty_out(vty, "%% Unknow MAC address format!\n");
		return CMD_WARNING;
	}

	DBusConnection *dcli_dbus_connection = NULL;
	dhcp_snp_reinitDbusConnection(&dcli_dbus_connection, vty);

	query = dbus_message_new_method_call(
								DHCPSNP_DBUS_BUSNAME,
								DHCPSNP_DBUS_OBJPATH,
								DHCPSNP_DBUS_INTERFACE,
								DHCPSNP_DBUS_METHOD_DELETE_WAN_TABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_BYTE, &macAddr.arEther[0],
							DBUS_TYPE_BYTE, &macAddr.arEther[1],
							DBUS_TYPE_BYTE, &macAddr.arEther[2],
							DBUS_TYPE_BYTE, &macAddr.arEther[3],
							DBUS_TYPE_BYTE, &macAddr.arEther[4],
							DBUS_TYPE_BYTE, &macAddr.arEther[5],
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		if (DHCP_SNP_RETURN_CODE_OK == ret) {
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
			vty_out(vty, "%% DHCP-Snooping not enabled global.\n");
		}else if (DHCP_SNP_RETURN_CODE_NOT_FOUND == ret) {
			vty_out(vty, "%% Delete item not found.\n");
		}else {
			vty_out(vty, "%% Delete item by mac failed\n");
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	else{
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;	
}
DEFUN(add_dhcp_snp_wan_bindtable_cmd_func,
	add_dhcp_snp_wan_bindtable_cmd,
	"add dhcp-snooping bind-table ip A.B.C.D mac MAC iface IFNAME",
	DCLI_DHCP_SNP_ADD_BINDING_STR
	DCLI_DHCP_SNP_STR
	"bind table\n"
	DCLI_DHCP_SNP_IP_STR
	DCLI_DHCP_SNP_IP_ADDRESS_STR
	DCLI_DHCP_SNP_MAC_STR
	DCLI_DHCP_SNP_MAC_ADDRESS_STR
	"interface name\n"
	"interface name\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int ip      = DCLI_DHCP_SNP_INIT_0;
	unsigned int ifindex = DCLI_DHCP_SNP_INIT_0;
	ETHERADDR macAddr;
	char ifname[IF_NAMESIZE] = {0}, name[IF_NAMESIZE] = {0};
	char *ifnamep = NULL;
	int Ret = 0;

	vty_out(vty, "%s %s %s\n", (char *)argv[0], (char *)argv[1], (char *)argv[2]);
	if(1 != inet_pton(AF_INET, (char *)argv[0], &ip)) {
		vty_out(vty, "%% invalid ip address %s\n", (char *)argv[0]);
		return CMD_WARNING;
	}
	memset(&macAddr, 0, sizeof(ETHERADDR));	
	ret = parse_mac_addr((char *)argv[1], &macAddr);	
	if (NPD_FAIL == ret) {
    	vty_out(vty, "%% Unknow MAC address format!\n");
		return CMD_WARNING;
	}
	if (strlen((char *)argv[2]) > IF_NAMESIZE) {
		vty_out(vty, "%% Bad interface name, too long\n");
		return CMD_WARNING;
	}
	
	strncpy(ifname, (char *)argv[2], IF_NAMESIZE);
	Ret = dcli_dhcp_snp_convert_ifname_check(vty, ifname);
	if(2 == Ret)
	dcli_dhcp_snp_convert_ifname(vty, ifname, name);
	else if(1 == Ret){
		sprintf(name, "%s", ifname);
	}else{
		vty_out(vty, "%% Wrong interfacename for dhcp snooping config!\n");
		return CMD_WARNING;
	}
	dcli_dhcp_check_snp_interface(name);

	ifnamep = malloc(strlen(name)+1);
	memset(ifnamep, 0 ,strlen(name)+1);
	memcpy(ifnamep, name ,strlen(name));
	if(0 == (ifindex = if_nametoindex((char *)name))) {
		vty_out(vty, "%% Bad interface name\n");
		return CMD_WARNING;
	}
	
	vty_out(vty, "interface %s index %d\n", name, ifindex);

	DBusConnection *dcli_dbus_connection = NULL;
	dhcp_snp_reinitDbusConnection(&dcli_dbus_connection, vty);

	query = dbus_message_new_method_call(
										DHCPSNP_DBUS_BUSNAME,
										DHCPSNP_DBUS_OBJPATH,
										DHCPSNP_DBUS_INTERFACE,
										DHCPSNP_DBUS_METHOD_ADD_WAN_TABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &ip,
							DBUS_TYPE_BYTE, &macAddr.arEther[0],
							DBUS_TYPE_BYTE, &macAddr.arEther[1],
							DBUS_TYPE_BYTE, &macAddr.arEther[2],
							DBUS_TYPE_BYTE, &macAddr.arEther[3],
							DBUS_TYPE_BYTE, &macAddr.arEther[4],
							DBUS_TYPE_BYTE, &macAddr.arEther[5],
							DBUS_TYPE_STRING, &ifnamep,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		if(ifnamep){
			free(ifnamep);
			ifnamep=NULL;
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID))
	{
		if (DHCP_SNP_RETURN_CODE_OK == ret) {
			dbus_message_unref(reply);
			if(ifnamep){
			free(ifnamep);
			ifnamep=NULL;
		}
			return CMD_SUCCESS;
		}else if (DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL == ret) {
			vty_out(vty, "%% DHCP-Snooping not enabled global.\n");
		}else if (DHCP_SNP_RETURN_CODE_ALREADY_SET == ret) {
			vty_out(vty, "%% MAC or IP has already in binding table.\n");
		}else {
			vty_out(vty, "%% Add bind table failed\n");
		}
		dbus_message_unref(reply);
		if(ifnamep){
			free(ifnamep);
			ifnamep=NULL;
		}
		return CMD_WARNING;
	}
	else{
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	if(ifnamep){
			free(ifnamep);
			ifnamep=NULL;
		}
	return CMD_SUCCESS;	
}
#if 0
DEFUN(config_dhcp_snp_ageing_time_cmd_func,
	config_dhcp_snp_ageing_time_cmd,
	"config dhcp-snooping ageing-time <60-86400>",
	CONFIG_STR
	DCLI_DHCP_SNP_STR
	"Config DHCP-Snooping on wan interface\n"
	"Enable DHCP-Snooping functionality\n"
	"Disable DHCP-Snooping functionality\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int aging_time = DCLI_DHCP_SNP_INIT_0;
	unsigned int ret = DCLI_DHCP_SNP_INIT_0;
	aging_time = atoi((char*)argv[0]);
	query = dbus_message_new_method_call(
										DHCPSNP_DBUS_BUSNAME,
										DHCPSNP_DBUS_OBJPATH,
										DHCPSNP_DBUS_INTERFACE,
										DHCPSNP_DBUS_METHOD_CONFIG_AGING_TIME);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &aging_time, 
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)) {
		if (DHCP_SNP_RETURN_CODE_OK != ret) {
			vty_out(vty, "%% Config dhcp snooping ageing time failed!");
			return CMD_WARNING;
		}
	} else {
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
#endif
DEFUN(config_dhcp_snp_arp_proxy_enable_cmd_func,
	config_dhcp_snp_arp_proxy_enable_cmd,
	"config dhcp-snooping arp-proxy (enable|disable)",
	CONFIG_STR
	DCLI_DHCP_SNP_STR
	"ARP Proxy\n"
	"Enable ARP Proxy functionality\n"
	"Disable ARP Proxyg functionality\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned char status = DCLI_DHCP_SNP_INIT_0;
	unsigned int isEnable = DCLI_DHCP_SNP_INIT_0;
	unsigned int ret = DCLI_DHCP_SNP_INIT_0;
	if (strncmp(argv[0], "enable", strlen(argv[0])) == 0) {
		isEnable = DHCP_SNP_ARP_PROXY_ENABLE;
	}
	else if (strncmp(argv[0], "disable", strlen(argv[0])) == 0) {
		isEnable = DHCP_SNP_ARP_PROXY_DISABLE;
	}else {
		vty_out(vty,"%% Bad parameter %s!\n", argv[0]);
		return CMD_WARNING;
	}
	query = dbus_message_new_method_call(
										DHCPSNP_DBUS_BUSNAME,
										DHCPSNP_DBUS_OBJPATH,
										DHCPSNP_DBUS_INTERFACE,
										DHCPSNP_DBUS_METHOD_ENABLE_ARP_PROXY);
	dbus_error_init(&err);
    dbus_message_append_args(query,
							DBUS_TYPE_UINT32, &isEnable,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
							DBUS_TYPE_UINT32, &ret,
							DBUS_TYPE_INVALID)) {
		if (DHCP_SNP_RETURN_CODE_OK != ret) {
			vty_out(vty, "%% %s ARP Proxy failed!\m", 
					(DHCP_SNP_ARP_PROXY_ENABLE == isEnable) ? "Enable":"Disable");
			return CMD_WARNING;
		}
	} else {
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
struct cmd_node dhcp_snp_node =
{
  DHCP_SNP_NODE,
  "	",
  1,
};

void dcli_dhcp_snp_element_init
(
	void
)
{
	install_node(&dhcp_snp_node,dcli_dhcp_snp_show_running_config,"DHCP_SNP_NODE");
	install_element(CONFIG_NODE, &config_dhcp_snp_enable_cmd);
	install_element(VLAN_NODE, &config_dhcp_snp_vlan_enable_cmd);
	install_element(VLAN_NODE, &config_dhcp_snp_set_vlan_port_mode_cmd);

	install_element(CONFIG_NODE, &show_dhcp_snp_bindtable_cmd);
	install_element(CONFIG_NODE, &show_dhcp_snp_trust_ports_cmd);

	install_element(CONFIG_NODE, &config_dhcp_snp_opt82_enable_cmd);
	install_element(CONFIG_NODE, &config_dhcp_snp_opt82_format_cmd);
	install_element(CONFIG_NODE, &config_dhcp_snp_opt82_fill_format_cmd);
	install_element(CONFIG_NODE, &config_dhcp_snp_opt82_set_remoteid_cmd1);
	install_element(CONFIG_NODE, &config_dhcp_snp_opt82_set_remoteid_cmd2);

	install_element(VLAN_NODE, &config_dhcp_snp_set_port_opt82_strategy_cmd);
	install_element(VLAN_NODE, &config_dhcp_snp_opt82_set_port_circuitid_cmd1);
	install_element(VLAN_NODE, &config_dhcp_snp_opt82_set_port_circuitid_cmd2);
	install_element(VLAN_NODE, &config_dhcp_snp_opt82_set_port_remoteid_cmd1);
	install_element(VLAN_NODE, &config_dhcp_snp_opt82_set_port_remoteid_cmd2);

	install_element(CONFIG_NODE, &config_dhcp_snp_add_delete_static_binding_cmd);
	install_element(CONFIG_NODE, &show_dhcp_snp_static_bindtable_cmd);	
	install_element(CONFIG_NODE, &show_dhcp_snp_static_bindtable_by_vlan_cmd);
	install_element(CONFIG_NODE, &show_dhcp_snp_static_bindtable_by_ethport_cmd);
	install_element(CONFIG_NODE, &config_dhcp_snp_add_binding_cmd);

	install_element(CONFIG_NODE, &config_dhcp_snp_wan_enable_cmd);
	install_element(CONFIG_NODE, &config_dhcp_snp_wan_set_intf_cmd);
	install_element(INTERFACE_NODE, &config_dhcp_snp_wan_enable_intf_cmd);
	install_element(CONFIG_NODE, &config_dhcp_snp_wan_set_arp_intf_cmd);
	install_element(CONFIG_NODE, &config_dhcp_snp_add_router_intf_cmd);
	install_element(INTERFACE_NODE, &config_dhcp_snp_wan_arp_enable_intf_cmd);
	install_element(CONFIG_NODE, &config_anti_arp_spoof_cmd);
	install_element(INTERFACE_NODE, &config_intf_anti_arp_spoof_cmd);
	install_element(CONFIG_NODE,&debug_dhcp_snp_info_cmd);
	install_element(CONFIG_NODE,&no_debug_dhcp_snp_info_cmd);
	install_element(HIDDENDEBUG_NODE,&debug_dhcp_snp_info_cmd);
	install_element(HIDDENDEBUG_NODE,&no_debug_dhcp_snp_info_cmd);

	install_element(CONFIG_NODE, &show_dhcp_snp_wan_bindtable_cmd);
	install_element(CONFIG_NODE, &add_dhcp_snp_wan_bindtable_cmd);	
	install_element(CONFIG_NODE, &delete_dhcp_snp_wan_bindtable_by_mac_cmd);
	install_element(CONFIG_NODE, &delete_dhcp_snp_router_cmd);
	install_element(CONFIG_NODE, &delete_dhcp_snp_router_cmd_all);
	install_element(CONFIG_NODE, &config_dhcp_snp_arp_proxy_enable_cmd);


	/* hansi node */
	install_element(HANSI_NODE, &show_dhcp_snp_wan_bindtable_cmd);
	install_element(HANSI_NODE, &config_dhcp_snp_wan_enable_cmd);
	install_element(HANSI_NODE, &config_dhcp_snp_wan_set_intf_cmd);
	install_element(HANSI_NODE, &config_dhcp_snp_wan_set_arp_intf_cmd);
	install_element(HANSI_NODE, &config_dhcp_snp_add_router_intf_cmd);
	install_element(HANSI_NODE, &debug_dhcp_snp_info_cmd);
	install_element(HANSI_NODE, &no_debug_dhcp_snp_info_cmd);
	install_element(HANSI_NODE, &add_dhcp_snp_wan_bindtable_cmd);
	install_element(HANSI_NODE, &delete_dhcp_snp_wan_bindtable_by_mac_cmd);
	install_element(HANSI_NODE, &delete_dhcp_snp_router_cmd);
	install_element(HANSI_NODE, &delete_dhcp_snp_router_cmd_all);

	install_element(LOCAL_HANSI_NODE, &config_dhcp_snp_wan_enable_cmd);
	install_element(LOCAL_HANSI_NODE, &config_dhcp_snp_wan_set_intf_cmd);
	install_element(LOCAL_HANSI_NODE, &config_dhcp_snp_wan_set_arp_intf_cmd);
	install_element(LOCAL_HANSI_NODE, &config_dhcp_snp_add_router_intf_cmd);
	install_element(LOCAL_HANSI_NODE, &debug_dhcp_snp_info_cmd);
	install_element(LOCAL_HANSI_NODE, &no_debug_dhcp_snp_info_cmd);
	install_element(LOCAL_HANSI_NODE, &add_dhcp_snp_wan_bindtable_cmd);
	install_element(LOCAL_HANSI_NODE, &delete_dhcp_snp_wan_bindtable_by_mac_cmd);
	install_element(LOCAL_HANSI_NODE, &delete_dhcp_snp_router_cmd);
	install_element(LOCAL_HANSI_NODE, &delete_dhcp_snp_router_cmd_all);


	return ;
}


#ifdef __cplusplus
}
#endif


