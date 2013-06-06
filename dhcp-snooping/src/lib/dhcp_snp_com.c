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
* dhcp_snp_com.c
*
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		dhcp snooping common for NPD module.
*
* DATE:
*		04/16/2010	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.3 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************
*	head files														*
**********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <dbus/dbus.h>
#include <net/if.h>
	
#include "util/npd_list.h"
#include "sysdef/npd_sysdef.h"
#include "sysdef/returncode.h"
#include "dbus/npd/npd_dbus_def.h"
#include "npd/nam/npd_amapi.h"
#include "npd/nbm/npd_bmapi.h"

#include "dhcp_snp_options.h"
#include "dhcp_snp_sqlite.h"
#include "dhcp_snp_tbl.h"
#include "dhcp_snp_if.h"
#include "dhcp_snp_pkt.h"
#include "dhcp_snp_log.h"
#include "dhcp_snp_listener.h"
#include "dhcp_snp_com.h"
#include "dhcp_snp_netlink.h"
#include "sys/wait.h"
#include "sys/types.h"

/*********************************************************
*	global variable define											*
**********************************************************/
/* DHCP-Snooping global status, default is disable	*/
unsigned char dhcp_snp_status = NPD_DHCP_SNP_DISABLE;

/* DHCP-Snooping option82, default is disable	*/
unsigned char g_dhcp_snp_opt82_enable = NPD_DHCP_SNP_OPT82_DISABLE;

/* storage format type of DHCP-Snooping option82, default is HEX */
unsigned char dhcp_snp_opt82_format_type = NPD_DHCP_SNP_OPT82_FORMAT_TYPE_HEX;

/* packet fill format type of DHCP-Snooping option82, default is extended */
unsigned char dhcp_snp_opt82_fill_format_type = NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_EXT;

/* remote-id of DHCP-Snooping option82, default is system MAC */
unsigned char dhcp_snp_opt82_remoteid_type = NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC;
unsigned char dhcp_snp_opt82_remoteid_str[NPD_DHCP_SNP_REMOTEID_STR_LEN] = {0};

unsigned int dhcp_snp_arp_proxy_enable = DHCP_SNP_ARP_PROXY_DISABLE;
extern unsigned int dhcp_snp_ageing_time;
extern pthread_mutex_t mutexDhcpsnptbl;

#define DHCPSNP_RD_DEBUG 1

#define EBTABLES_FORWARD_DROP 	"ebtables -P FORWARD DROP"
#define EBTABLES_FORWARD_ACCEPT "ebtables -P FORWARD ACCEPT"
#define EBTABLES_INPUT_ACCEPT	"ebtables -P INPUT ACCEPT"
#define EBTABLES_OUTPUT_ACCEPT 	"ebtables -P OUTPUT ACCEPT"

/*********************************************************
*	extern variable												*
**********************************************************/
extern sqlite3 *dhcp_snp_db;


/*********************************************************
*	functions define												*
**********************************************************/

/**********************************************************************************
 * dhcp_snp_enable
 *		set DHCP_Snooping enable global status, and init 
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
unsigned int dhcp_snp_enable
(
	void
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;

	ret = dhcp_snp_set_global_status(NPD_DHCP_SNP_ENABLE);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("set DHCP-Snooping global status enable error, ret %x\n", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	ret = dhcp_snp_tbl_initialize();
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("init DHCP-Snooping bind table error, ret %x\n", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	#if 0
	/* set default value to global variable */
	memcpy(dhcp_snp_opt82_remoteid_str, (unsigned char*)PRODUCT_MAC_ADDRESS, 13);
	dhcp_snp_opt82_format_type = NPD_DHCP_SNP_OPT82_FORMAT_TYPE_HEX;
	dhcp_snp_opt82_fill_format_type = NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_EXT;
	dhcp_snp_opt82_remoteid_type = NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC;

	rc = npd_acl_add_udp_vlan_rule(NPD_DHCP_SNP_ENABLE);
	if (0 != rc) {
		syslog_ax_dhcp_snp_err("create acl rules for DHCP-Snooping error, rc %x\n", rc);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	#endif
	ret = dhcp_snp_socket_global_enable();
	if(DHCP_SNP_RETURN_CODE_OK != ret && DHCP_SNP_RETURN_CODE_NOT_FOUND != ret) {
		syslog_ax_dhcp_snp_err("global enable DHCP-Snooping sockets error, ret %x\n", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	/*exec ebtables filter*/
	#if 0
	system(EBTABLES_FORWARD_ACCEPT);
	system(EBTABLES_INPUT_ACCEPT);
	system(EBTABLES_OUTPUT_ACCEPT);
	#endif
	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * dhcp_snp_disable
 *		set DHCP_Snooping enable global status, and init 
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
unsigned int dhcp_snp_disable
(
	void
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;

	if (check_dhcp_snp_listener()) {
		syslog_ax_dhcp_snp_err("disable all interface dhcp snooping first.\n");
		return DHCP_SNP_RETURN_CODE_EN_INTF;
	}

	ret = dhcp_snp_set_global_status(NPD_DHCP_SNP_DISABLE);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("set DHCP-Snooping global status disable error, ret %x\n", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	ret = dhcp_snp_tbl_destroy();
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("destroy DHCP-Snooping bind table error, ret %x\n", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	#if 0
	/* set default value to global variable */
	dhcp_snp_opt82_format_type = NPD_DHCP_SNP_OPT82_FORMAT_TYPE_HEX;
	dhcp_snp_opt82_fill_format_type = NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_EXT;
	dhcp_snp_opt82_remoteid_type = NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC;
	memset(dhcp_snp_opt82_remoteid_str, 0, NPD_DHCP_SNP_REMOTEID_STR_LEN);

	/* for option82 */
	ret = dhcp_snp_reset_all_vlan_port();
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("clear DHCP-Snooping port info error, ret %x\n", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	if (DHCP_SNP_RETURN_CODE_EN_OPT82 == dhcp_snp_check_opt82_status()) {
		ret = dhcp_snp_opt82_disable();
		if (DHCP_SNP_RETURN_CODE_OK != ret) {
			syslog_ax_dhcp_snp_err("DHCP-Snooping disable option 82 error, ret %x.\n",
									ret);
			ret = DHCP_SNP_RETURN_CODE_ERROR;
		}
	}

	rc = npd_acl_add_udp_vlan_rule(NPD_DHCP_SNP_DISABLE);
	if (0 != rc) {
		syslog_ax_dhcp_snp_err("destroy acl rules for DHCP-Snooping error, rc %x\n", rc);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	#endif
	dhcp_snp_socket_global_disable();

	/*exec ebtables filter*/
	#if 0
	system(EBTABLES_FORWARD_ACCEPT);
	system(EBTABLES_INPUT_ACCEPT);
	system(EBTABLES_OUTPUT_ACCEPT);
	#endif
	
	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * dhcp_snp_check_global_status
 *		check DHCP_Snooping enable/disable global status
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_ENABLE_GBL		- global status is enable
 *	 	DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL	- global status is disable
 **********************************************************************************/
unsigned int dhcp_snp_check_global_status
(
	void
)
{
	
	if (dhcp_snp_status == NPD_DHCP_SNP_ENABLE) {
		return DHCP_SNP_RETURN_CODE_ENABLE_GBL;
	}else {
		return DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL;
	}
}

/**********************************************************************************
 * dhcp_snp_get_global_status
 *		get DHCP_Snooping enable/disable global status
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		unsigned char *status
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
unsigned int dhcp_snp_get_global_status
(
	unsigned char *status
)
{
	if (!status) {
		syslog_ax_dhcp_snp_err("get DHCP-Snooping global status error, parameters is null.\n");
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	
	*status = dhcp_snp_status;
	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * dhcp_snp_set_global_status
 *		set DHCP_Snooping enable/disable global status
 *
 *	INPUT:
 *		unsigned char isEnable
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
unsigned int dhcp_snp_set_global_status
(
	unsigned char isEnable
)
{
	dhcp_snp_status = isEnable ? NPD_DHCP_SNP_ENABLE : NPD_DHCP_SNP_DISABLE;

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * dhcp_snp_set_intf_arp
 *		set DHCP_Snooping enable/disable interface anti-arp-spoofing
 *
 *	INPUT:
 *		unsigned char isEnable
 *		char *ifname
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
unsigned int dhcp_snp_set_intf_arp
(
	unsigned char isEnable,
	char *ifname
)
{
	int fp = -1;
	struct ifreq tmp;

	if(!ifname) {
		syslog_ax_dhcp_snp_err("set interface anti-arp-spoof parameter null!\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}
	
	fp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	
	memset(&tmp, 0, sizeof(tmp));
	strcpy(tmp.ifr_name, ifname);
	if (ioctl(fp, SIOCGIFUDFFLAGS, &tmp) < 0) {
		syslog_ax_dhcp_snp_err("set interface %s anti-arp-spoof ioctl get flags error %s!\n",  \
							ifname, strerror(errno));
		if(fp){
			close(fp);
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	
	if (isEnable) {
		tmp.ifr_flags |= IFF_UDF_NOARP;		
		if (ioctl(fp, SIOCSIFUDFFLAGS, &tmp) < 0) {
			syslog_ax_dhcp_snp_err("set interface %s anti-arp-spoof ioctl set flags error %s!\n",  \
								ifname, strerror(errno));
			if(fp){
				close(fp);
			}
			return DHCP_SNP_RETURN_CODE_ERROR;
		}
		
		/* add ebtables rule */
		if(!strncmp(ifname, "ebr", 3) ||
			!strncmp(ifname, "wlan", 4)) {
			dhcp_snp_listener_handle_intf_ebtables(ifname, DHCPSNP_EBT_ADD_E);
		}
	}
	else {
		tmp.ifr_flags &= ~IFF_UDF_NOARP;
		if (ioctl(fp, SIOCSIFUDFFLAGS, &tmp) < 0) {
			syslog_ax_dhcp_snp_err("set interface %s anti-arp-spoof ioctl set flags error %s!\n",  \
								ifname, strerror(errno));
			if(fp){
				close(fp);
			}
			return DHCP_SNP_RETURN_CODE_ERROR;
		}
		
		/* delete ebtables rule */
		if(!strncmp(ifname, "ebr", 3) ||
			!strncmp(ifname, "wlan", 4)) {
			dhcp_snp_listener_handle_intf_ebtables(ifname, DHCPSNP_EBT_DEL_E);
		}
	}
	
	dhcp_snp_listener_handle_arp(isEnable, ifname);
	close(fp);
	return DHCP_SNP_RETURN_CODE_OK;
}

int dhcp_snp_set_udf_flag(char *ifname,  unsigned int set_flag)
{
	struct ifreq ifr;
	int fd = -1;
	if (!ifname) {
		syslog_ax_dhcp_snp_err("get interface link status with null name!\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}
	memset(&ifr, 0, sizeof(struct ifreq));
	fd	= socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		syslog_ax_dhcp_snp_err("get interface %s link status init socket error %s!\n", ifname, strerror(errno));
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	if (0 == ioctl(fd, SIOCGIFUDFFLAGS, (char *)&ifr)) {
		syslog_ax_dhcp_snp_dbg("get interface %s user define flag %#x\n", ifname, ifr.ifr_flags);
	}else {
		syslog_ax_dhcp_snp_err("get interface %s flags ioctl failed %s!\n", ifname, strerror(errno));
		close(fd);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	if(DHCP_SNP_SET_UDF_FLAG == set_flag) {
		ifr.ifr_flags |= (IFF_ARP_DORP);	
	} else if (DHCP_SNP_CLR_UDF_FLAG == set_flag) {		
		ifr.ifr_flags &= ~(IFF_ARP_DORP);
	} else {
		syslog_ax_dhcp_snp_dbg("set interface %s user define flag failed. unkown set flag %d\n",ifname, set_flag);
		close(fd);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	if (0 == ioctl(fd, SIOCSIFUDFFLAGS, (char *)&ifr)) {
		syslog_ax_dhcp_snp_dbg("set interface %s user define flag %#x\n",ifname, ifr.ifr_flags);
	}else {
		syslog_ax_dhcp_snp_err("set interface %s link status ioctl failed %s!\n", ifname, strerror(errno));
		close(fd);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	close(fd);
	return 0;
}

#if 0
/**********************************************************************************
 * dhcp_snp_check_vlanl_status
 *		check DHCP_Snooping enable/disable status on special vlan
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_EN_VLAN			- vlan status is enable
 *	 	DHCP_SNP_RETURN_CODE_NOT_EN_VLAN		- vlan status is disable
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST	- check fail, vlan not exist
 **********************************************************************************/
unsigned int dhcp_snp_check_vlan_status
(
	unsigned short vlanId
)
{
	struct vlan_s *vlanNode = NULL;

	vlanNode = npd_find_vlan_by_vid(vlanId);
	if ((NULL == vlanNode)||(vlanNode->isAutoCreated)) {
		return DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST;
	}

	if (NPD_DHCP_SNP_ENABLE == vlanNode->dhcpSnpEnDis) {
		return DHCP_SNP_RETURN_CODE_EN_VLAN;
	}else {
		return DHCP_SNP_RETURN_CODE_NOT_EN_VLAN;
	}
}

/**********************************************************************************
 * dhcp_snp_vlan_endis_config
 *		config DHCP_Snooping enable/disable status on special vlan
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *	 	DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST		- fail
 **********************************************************************************/
int dhcp_snp_vlan_endis_config
(
	unsigned short vlanId,
	unsigned char  isEnable
)
{
	unsigned ret = NPD_DHCP_SNP_INIT_0;
	struct vlan_s *vlanNode = NULL;

	struct vlan_port_list_node_s *node = NULL;
	struct list_head *list = NULL;
	struct list_head *pos  = NULL;
	vlan_port_list_s *portList = NULL;

	/* check vlan exist.*/
	ret = npd_check_vlan_exist(vlanId);
	if (VLAN_RETURN_CODE_VLAN_EXISTS != ret) {
		syslog_ax_dhcp_snp_err("vlan %d not exist.\n", vlanId);
		return DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST;
	}
	else {
		/*stamp igmp on vlan struct*/
		vlanNode = npd_find_vlan_by_vid(vlanId);
		vlanNode->dhcpSnpEnDis = isEnable;

		/*
		 ***********************************
		 * bind acl rule to vlan for dhcp snooping
		 * for select dhcp packets to cpu
		 ***********************************
		*/
		ret = npd_acl_add_udp_vlan_enable(vlanId, isEnable);
		if (0 != ret) {
			syslog_ax_dhcp_snp_err("%s acl rules %s vlan %d for DHCP-Snooping error, ret %x\n",
									isEnable ? "add" : "delete",
									isEnable ? "to" : "from",
									vlanId, ret);
			return DHCP_SNP_RETURN_CODE_ERROR;
		}
		syslog_ax_dhcp_snp_dbg("%s acl rules %s vlan %d for DHCP-Snooping success.\n",
								isEnable ? "add" : "delete",
								isEnable ? "to" : "from",
								vlanId);

		/*
		 ***********************************
		 * init dhcp snooping data on port of vlan
		 ***********************************
		 */
		syslog_ax_dhcp_snp_dbg("%s vlan %d dhcp snooping data.\n",
								isEnable ? "init" : "reset", vlanId);		
		/* tagged port list*/
		portList = vlanNode->tagPortList;
		
		if (NULL == portList) {
			syslog_ax_dhcp_snp_warning("Warning: vlan %d tag port list is null.\n",vlanId);	
		}
		else if(0 != portList->count)
		{
			syslog_ax_dhcp_snp_dbg("vlan %d tagged port count %d\n",
						vlanId, portList->count);

			list = &(portList->list);
			__list_for_each(pos, list) {
				node = list_entry(pos, struct vlan_port_list_node_s, list);
				if (NULL != node)
				{
					/* DHCP-Snooping trust mode of port, default is notrust 				*/
					node->dhcp_snp_info.trust_mode = NPD_DHCP_SNP_PORT_MODE_NOTRUST;
					/* DHCP-Snooping option82 strategy type of port, default is replace 	*/
					node->dhcp_snp_info.opt82_strategy = NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_REPLACE;
					/* DHCP-Snooping option82 circuit-id of port, default is vlan +portindex	*/
					node->dhcp_snp_info.opt82_circuitid = NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_DEFAULT;
					memset(node->dhcp_snp_info.opt82_circuitid_str, 0, NPD_DHCP_SNP_CIRCUITID_STR_LEN);
					/* DHCP-Snooping option82 remote-id of port, default is system mac	 	*/
					node->dhcp_snp_info.opt82_remoteid = NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC;
					memset(node->dhcp_snp_info.opt82_remoteid_str, 0, NPD_DHCP_SNP_REMOTEID_STR_LEN);
				}
			}
		}

		/* untagged port list */
		portList = vlanNode->untagPortList;
		
		if (NULL == portList) {
			syslog_ax_dhcp_snp_warning("Warning: vlan %d untag port list is null.\n", vlanId); 
		}
		else if(0 != portList->count)
		{
			syslog_ax_dhcp_snp_dbg("vlan %d untagged port count %d\n",
						vlanId, portList->count);
		
			list = &(portList->list);
			__list_for_each(pos, list) {
				node = list_entry(pos, struct vlan_port_list_node_s, list);
				if (NULL != node)
				{
					/* DHCP-Snooping trust mode of port, default is notrust 				*/
					node->dhcp_snp_info.trust_mode = NPD_DHCP_SNP_PORT_MODE_NOTRUST;
					/* DHCP-Snooping option82 strategy type of port, default is replace 	*/
					node->dhcp_snp_info.opt82_strategy = NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_REPLACE;
					/* DHCP-Snooping option82 circuit-id of port, default is vlan +portindex	*/
					node->dhcp_snp_info.opt82_circuitid = NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_DEFAULT;
					memset(node->dhcp_snp_info.opt82_circuitid_str, 0, NPD_DHCP_SNP_CIRCUITID_STR_LEN);
					/* DHCP-Snooping option82 remote-id of port, default is system mac		*/
					node->dhcp_snp_info.opt82_remoteid = NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC;
					memset(node->dhcp_snp_info.opt82_remoteid_str, 0, NPD_DHCP_SNP_REMOTEID_STR_LEN);
				}
			}
		}
	}

	syslog_ax_dhcp_snp_dbg("%s vlan %d dhcp snooping success.\n",
							isEnable ? "enable" : "disable", vlanId);	

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * dhcp_snp_reset_all_vlan_port
 *		reset DHCP_Snooping attribute on all vlan's ports
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK		- success
 *		DHCP_SNP_RETURN_CODE_ERROR		- fail
 **********************************************************************************/
unsigned int dhcp_snp_reset_all_vlan_port
(
	void
)
{
	unsigned int ret = NPD_DHCP_SNP_INIT_0;
	unsigned short vlanId = NPD_DHCP_SNP_INIT_0;
	struct vlan_s *vlanNode = NULL;

	syslog_ax_dhcp_snp_dbg("reset all vlan's ports for dhcp snooping.\n");

	for (vlanId = 1; vlanId <= CHASSIS_VLAN_RANGE_MAX; vlanId++)
	{
		ret = npd_check_vlan_exist(vlanId);
		if (VLAN_RETURN_CODE_VLAN_EXISTS != ret)
		{
			ret = DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST;
			continue;
		}else
		{
			vlanNode = npd_find_vlan_by_vid(vlanId);
			if ((NULL != vlanNode) &&
				(NPD_DHCP_SNP_ENABLE == vlanNode->dhcpSnpEnDis))
			{
				ret = dhcp_snp_vlan_endis_config(vlanId, NPD_DHCP_SNP_DISABLE);
				if (DHCP_SNP_RETURN_CODE_OK != ret) {
					syslog_ax_dhcp_snp_err("disable DHCP Snooping on vlan %d error, ret %x.\n",
						vlanId, ret);
					return DHCP_SNP_RETURN_CODE_ERROR;
				}
			}
			else {
				ret = DHCP_SNP_RETURN_CODE_NOT_EN_VLAN;
			}
		}
	}

	syslog_ax_dhcp_snp_dbg("reset all vlan ports for dhcp snooping success.\n");

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * dhcp_snp_set_port_trust_mode
 *		set port is DHCP_Snooping trust/no-bind/notrust
 *
 *	INPUT:
 *		unsigned short vlanId,			- vlan ID
 *		unsigned int g_eth_index,		- port index
 *		unsigned char tagMode,			- tag mode of port
 *		unsigned char trust_mode		- trust mode
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET			- have set trust or no-bind port
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST		- vlan not exist
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 *
 **********************************************************************************/
unsigned int dhcp_snp_set_port_trust_mode
(
	unsigned short vlanId,
	unsigned int g_eth_index,
	unsigned char tagMode,
	unsigned char trust_mode
)
{
	struct vlan_s *vlanNode = NULL;
	struct vlan_port_list_node_s *node = NULL;
	struct list_head *list = NULL;
	struct list_head *pos  = NULL;
	vlan_port_list_s *portList = NULL;

	syslog_ax_dhcp_snp_dbg("set vlan %d eth-port %d tag mode %s trust mode %s\n",
							vlanId, g_eth_index,
							tagMode ? "tagged" : "untagged",  
							(trust_mode == NPD_DHCP_SNP_PORT_MODE_TRUST) ? "trust" :
							((trust_mode == NPD_DHCP_SNP_PORT_MODE_NOBIND) ? "nobind" :
							((trust_mode == NPD_DHCP_SNP_PORT_MODE_NOTRUST) ? "notrust" :
							"unknow")));

	vlanNode = npd_find_vlan_by_vid(vlanId);
	if (NULL == vlanNode) {
		syslog_ax_dhcp_snp_err("vlan %d is not created.\n", vlanId);
		return DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST;
	}

	/* tagged port list*/
	if (NPD_TRUE == tagMode) {
		portList = vlanNode->tagPortList;
	}else if (NPD_FALSE == tagMode) {
		portList = vlanNode->untagPortList;
	}

	if (NULL == portList) {
		syslog_ax_dhcp_snp_err("eth-port %d is not member of the vlan %d.\n", g_eth_index, vlanId);
		return DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST;
	}
	else if(0 != portList->count)
	{
		syslog_ax_dhcp_snp_dbg("vlan %d %s port count %d\n",
					vlanId, tagMode ? "tagged" : "untagged", portList->count);
		
		list = &(portList->list);
		__list_for_each(pos,list) {
			node = list_entry(pos,struct vlan_port_list_node_s,list);
			if (NULL != node && g_eth_index == node->eth_global_index)
			{
				/* check port trust mode */
				if (node->dhcp_snp_info.trust_mode == trust_mode)
				{
					syslog_ax_dhcp_snp_err("eth-port %d have %s port\n",
											g_eth_index,
											(trust_mode == NPD_DHCP_SNP_PORT_MODE_TRUST) ? "trust" :
											((trust_mode == NPD_DHCP_SNP_PORT_MODE_NOBIND) ? "nobind" : "notrust"));
					return DHCP_SNP_RETURN_CODE_ALREADY_SET;
				}

				node->dhcp_snp_info.trust_mode = trust_mode;
				syslog_ax_dhcp_snp_dbg("set vlan %d eth-port %d tag mode %s trust mode %s success.\n",
										vlanId, g_eth_index,
										tagMode ? "tagged" : "untagged",  
										(node->dhcp_snp_info.trust_mode == NPD_DHCP_SNP_PORT_MODE_TRUST) ? "trust" :
										((node->dhcp_snp_info.trust_mode == NPD_DHCP_SNP_PORT_MODE_NOBIND) ? "nobind" :
										((node->dhcp_snp_info.trust_mode == NPD_DHCP_SNP_PORT_MODE_NOTRUST) ? "notrust" :
										"unknow")));

				return DHCP_SNP_RETURN_CODE_OK;
			}
		}
	}

	syslog_ax_dhcp_snp_err("eth-port %d is not member of the vlan %d.\n", g_eth_index, vlanId);
	return DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST;
}
#endif
/**********************************************************************************
 * dhcp_snp_get_option
 *		get an option with bounds checking (warning, not aligned).
 *
 *	INPUT:
 *		NPD_DHCP_MESSAGE_T *packet,
 *		unsigned int code
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NULL			- not get the option
 *		not NULL 		- string of the option
 **********************************************************************************/
void *dhcp_snp_get_option
(
	NPD_DHCP_MESSAGE_T *packet,
	unsigned int code
)
{
	unsigned int i = NPD_DHCP_SNP_INIT_0;
	unsigned int length = NPD_DHCP_SNP_INIT_0;
	unsigned int over = NPD_DHCP_SNP_INIT_0;
	unsigned int done = NPD_DHCP_SNP_INIT_0;
	unsigned int curr = NPD_DHCP_SNP_OPTION_FIELD;
	unsigned char *optionptr = NULL;

	if (!packet) {
		log_error("%s: parameter null\n", __func__);
		return NULL;
	}

	optionptr = (unsigned char *)packet->options;
	i = 0;
	length = NPD_DHCP_SNP_OPTION_LEN;

	while (!done) {
		if (i >= length) {
			log_error("bogus packet, option fields too long.");
			return NULL;
		}
		if (optionptr[i + NPD_DHCP_SNP_OPT_CODE] == code) {
			if (i + 1 + optionptr[i + NPD_DHCP_SNP_OPT_LEN] >= length) {
				log_error("bogus packet, option fields too long.");
				return NULL;
			}
			return optionptr + i + 2;
		}			
		switch (optionptr[i + NPD_DHCP_SNP_OPT_CODE]) {
			
			case DHCP_PADDING:
				i++;
				break;
			
			case DHCP_OPTION_OVER:
				if (i + 1 + optionptr[i + NPD_DHCP_SNP_OPT_LEN] >= length) {
					log_error("bogus packet, option fields too long.");
					return NULL;
				}
				over = optionptr[i + 3];
				i += optionptr[NPD_DHCP_SNP_OPT_LEN] + 2;
				break;
			
			case DHCP_END:
				if (curr == NPD_DHCP_SNP_OPTION_FIELD && over & NPD_DHCP_SNP_FILE_FIELD) {
					optionptr = (unsigned char *)packet->file;
					i = 0;
					length = 128;
					curr = NPD_DHCP_SNP_FILE_FIELD;
				} else if (curr == NPD_DHCP_SNP_FILE_FIELD && over & NPD_DHCP_SNP_SNAME_FIELD) {
					optionptr = (unsigned char *)packet->sname;
					i = 0;
					length = 64;
					curr = NPD_DHCP_SNP_SNAME_FIELD;
				} else {
					done = 1;
				}
				break;
			
			default:
				i += optionptr[NPD_DHCP_SNP_OPT_LEN + i] + 2;
		}
	}
	log_error("get dhcp option %d failed.\n", code);
	
	return NULL;
}

/**********************************************************************************
 * dhcp_snp_is_trusted
 *		check the port is trust/no-bind port, and get trust mode
 *
 *	INPUT:
 *		unsigned short vlanId,			- vlan ID
 *		unsigned int ifindex,			- port global index
 *		unsigned char isTagged,		- tag flag of port
 *
 *	OUTPUT:
 *		unsigned char *trust_mode		- trust mode of port
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK					- found the port which trust or no-bind
 *		DHCP_SNP_RETURN_CODE_ERROR					- not found the port which trust or no-bind
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL			- parameter is null
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST		- vlan is not exist
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 *
 **********************************************************************************/
unsigned int dhcp_snp_is_trusted
(
	unsigned short vlanId,
	unsigned int g_eth_index,
	unsigned char isTagged,
	unsigned char *trust_mode
)
{
	struct vlan_s *vlanNode = NULL;
	struct vlan_port_list_node_s *node = NULL;
	struct list_head *list = NULL;
	struct list_head *pos  = NULL;
	vlan_port_list_s *portList = NULL;

	*trust_mode = NPD_DHCP_SNP_PORT_MODE_TRUST;
	return DHCP_SNP_RETURN_CODE_OK;
	#if 0
	if (!trust_mode) {
		syslog_ax_dhcp_snp_err("check vlan trust ports error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	syslog_ax_dhcp_snp_dbg("check vlan %d eth-port %d tag mode %s trust mode.\n",
							vlanId, g_eth_index,
							isTagged ? "tagged" : "untagged");
	
	/* special branch for CPU interface or RGMII interface */
	if(0xFFFF == vlanId) {
		*trust_mode = NPD_DHCP_SNP_PORT_MODE_TRUST;
		return DHCP_SNP_RETURN_CODE_OK;
	}

	vlanNode = npd_find_vlan_by_vid(vlanId);
	if (NULL == vlanNode) {
		syslog_ax_dhcp_snp_err("vlan %d is not created.\n", vlanId);
		return DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST;
	}

	/* tagged port list*/
	if (NPD_TRUE == isTagged) {
		portList = vlanNode->tagPortList;
	}else if (NPD_FALSE == isTagged) {
		portList = vlanNode->untagPortList;
	}

	if (NULL == portList) {
		syslog_ax_dhcp_snp_err("eth-port %d is not member of the vlan %d.\n", g_eth_index, vlanId);
		return DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST;
	}
	else if(0 != portList->count)
	{
		syslog_ax_dhcp_snp_dbg("vlan %d %s port count %d\n",
					vlanId, isTagged ? "tagged" : "untagged", portList->count);
		
		list = &(portList->list);
		__list_for_each(pos,list) {
			node = list_entry(pos,struct vlan_port_list_node_s,list);
			if (NULL != node && g_eth_index == node->eth_global_index)
			{
				/* get port trust mode */
				*trust_mode = node->dhcp_snp_info.trust_mode;
				syslog_ax_dhcp_snp_dbg("get vlan %d eth-port %d tag mode %s trust mode %s success.\n",
										vlanId, g_eth_index,
										isTagged ? "tagged" : "untagged",  
										(node->dhcp_snp_info.trust_mode == NPD_DHCP_SNP_PORT_MODE_TRUST) ? "trust" :
										((node->dhcp_snp_info.trust_mode == NPD_DHCP_SNP_PORT_MODE_NOBIND) ? "nobind" :
										((node->dhcp_snp_info.trust_mode == NPD_DHCP_SNP_PORT_MODE_NOTRUST) ? "notrust" :
										"unknow")));

				return DHCP_SNP_RETURN_CODE_OK;
			}
		}
	}

	syslog_ax_dhcp_snp_err("eth-port %d is not member of the vlan %d.\n", g_eth_index, vlanId);
	return DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST;
	#endif
}

#if 0
/**********************************************************************************
 * dhcp_snp_get_vlan_trust_port
 *		get trust/no-bind ports of the vlan
 *
 *	INPUT:
 *		unsigned short vlanId,			- vlan ID
 *		unsigned char tagMode,			- tag mode
 *
 *	OUTPUT:
 *		unsigned int *cnt,
 *		NPD_DHCP_SNP_SHOW_TRUST_PORTS_T *ports_array
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK				- success
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL		- parameter is null
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST	- vlan is not exist
 *
 **********************************************************************************/
unsigned int dhcp_snp_get_vlan_trust_port
(
	unsigned short vlanId,
	unsigned char tagMode,
	unsigned int *cnt,
	NPD_DHCP_SNP_SHOW_TRUST_PORTS_T *ports_array
)
{
	struct vlan_s *vlanNode = NULL;
	struct vlan_port_list_node_s *node = NULL;
	struct list_head *list = NULL;
	struct list_head *pos  = NULL;
	vlan_port_list_s *portList = NULL;
	unsigned int count = NPD_DHCP_SNP_INIT_0;
	unsigned int slot_no = NPD_DHCP_SNP_INIT_0;
	unsigned int port_no = NPD_DHCP_SNP_INIT_0;

	if (!cnt || !ports_array) {
		syslog_ax_dhcp_snp_err("get vlan trust ports error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	count = *cnt;

	vlanNode = npd_find_vlan_by_vid(vlanId);
	if (NULL == vlanNode) {
		return DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST;
	}

	/* tagged port list*/
	if (NPD_TRUE == tagMode) {
		portList = vlanNode->tagPortList;
	}else if (NPD_FALSE == tagMode) {
		portList = vlanNode->untagPortList;
	}

	if (NULL == portList) {
		return DHCP_SNP_RETURN_CODE_OK;
	}
	else if(0 != portList->count)
	{
		syslog_ax_dhcp_snp_dbg("vlan %d %s port count %d\n",
					vlanId, tagMode ? "tagged":"untagged", portList->count);
		
		list = &(portList->list);
		__list_for_each(pos,list) {
			node = list_entry(pos,struct vlan_port_list_node_s,list);
			if (NULL != node)
			{
				/* check port trust mode */
				if ((node->dhcp_snp_info.trust_mode == NPD_DHCP_SNP_PORT_MODE_TRUST) ||
					(node->dhcp_snp_info.trust_mode == NPD_DHCP_SNP_PORT_MODE_NOBIND))
				{
					syslog_ax_dhcp_snp_err("eth-port %d have %s port.\n",
											node->eth_global_index,
											(node->dhcp_snp_info.trust_mode == NPD_DHCP_SNP_PORT_MODE_TRUST) ? "trust" :
											((node->dhcp_snp_info.trust_mode == NPD_DHCP_SNP_PORT_MODE_NOBIND) ? "nobind" : "notrust"));
					ports_array[count].vlanId = vlanId;
					slot_no = NPD_DHCP_SNP_INIT_0;
					port_no = NPD_DHCP_SNP_INIT_0;
					dhcp_snp_get_slot_port_from_eth_index(node->eth_global_index, &slot_no, &port_no);
					ports_array[count].slot_no = slot_no;
					ports_array[count].port_no = port_no;
					ports_array[count].tag_mode = tagMode;
					ports_array[count].trust_mode = node->dhcp_snp_info.trust_mode;

					count++;
				}
			}
		}
	}

	*cnt = count;
	syslog_ax_dhcp_snp_dbg("calc to vlan %d %s port list, all trust/nobind ports count %d\n",
							vlanId, tagMode ? "tagged" : "untagged", *cnt);

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * dhcp_snp_get_trust_ports
 *		get port is trust/no-bind port, and get trust mode
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		unsigned int *trust_ports_cnt,
 *		NPD_DHCP_SNP_SHOW_TRUST_PORTS_T *ports_array
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- parameter is null
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 **********************************************************************************/
unsigned int dhcp_snp_get_trust_ports
(
	unsigned int *trust_ports_cnt,
	NPD_DHCP_SNP_SHOW_TRUST_PORTS_T *ports_array
)
{
	unsigned int ret = NPD_DHCP_SNP_INIT_0;
	unsigned int cnt = NPD_DHCP_SNP_INIT_0;
	unsigned short vlanId = NPD_DHCP_SNP_INIT_0;
	struct vlan_s *vlanNode = NULL;

	syslog_ax_dhcp_snp_dbg("get all trust eth-ports\n");

	if (!trust_ports_cnt || !ports_array) {
		syslog_ax_dhcp_snp_err("get all trust eth-ports error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	for (vlanId = 1; vlanId <= CHASSIS_VLAN_RANGE_MAX; vlanId++)
	{
		ret = npd_check_vlan_exist(vlanId);
		if (VLAN_RETURN_CODE_VLAN_EXISTS != ret)
		{
			ret = DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST;
			continue;
		}else
		{
			vlanNode = npd_find_vlan_by_vid(vlanId);
			if ((NULL != vlanNode) &&
				(NPD_DHCP_SNP_ENABLE == vlanNode->dhcpSnpEnDis))
			{	/* get tagged trust port */
				if (0 != vlanNode->tagPortList)
				{
					ret = dhcp_snp_get_vlan_trust_port(vlanId, 1, &cnt, ports_array);
					if (DHCP_SNP_RETURN_CODE_OK != ret) {
						syslog_ax_dhcp_snp_err("get tag trust eth-ports in vlan %d error, ret %x\n", vlanId, ret);
						return DHCP_SNP_RETURN_CODE_ERROR;
					}
				}

				/* get untagged trust port */
				if (0 != vlanNode->untagPortList)
				{
					ret = dhcp_snp_get_vlan_trust_port(vlanId, 0, &cnt, ports_array);
					if (DHCP_SNP_RETURN_CODE_OK != ret) {
						syslog_ax_dhcp_snp_err("get untag trust eth-ports in vlan %d error, ret %x\n", vlanId, ret);
						return DHCP_SNP_RETURN_CODE_ERROR;
					}
				}
			}
			else {
				ret = DHCP_SNP_RETURN_CODE_NOT_EN_VLAN;
			}
		}
	}

	*trust_ports_cnt = cnt;
	syslog_ax_dhcp_snp_dbg("get all trust eth-ports count %d.\n", *trust_ports_cnt);

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * dhcp_snp_mac_ascii_to_hex
 * 
 * 	mac address in Hex format
 *
 *	INPUT:
 *		chaddr	- string of mac
 *		size		- macAddr buffer size
 *	
 *	OUTPUT:
 *		macAddr - mac address will be returned back
 *
 * 	RETURN:
 *		-1 - if mac address buffer size too small.
 *		-2 - illegal character found.
 *		-3 - parameter is null
 *		0 - if no error occur
 *
 *	NOTATION:
 *		mac address is a ASCII code formatted MAC address, such as 
 *		"001122334455" stands for mac address 00:11:22:33:44:55 or 00-11-22-33-44-55
 *		
 **********************************************************************************/
int dhcp_snp_mac_ascii_to_hex
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
	if (size < NPD_DHCP_SNP_MAC_ADD_LEN) {
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
 * dhcp_snp_remove_vlan_broadcast_ports
 *		remove port from vlan's broadcast ports
 *
 *	INPUT:
 *		unsigned short ifindex,
 *		unsigned char isTagged,
 *		struct vlan_ports *tagPorts,
 *		struct vlan_ports *untagPorts
 *	
 *	OUTPUT:
 *		struct vlan_ports *tagPorts,
 *		struct vlan_ports *untagPorts
 *
 * 	RETURN:
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL
 *		DHCP_SNP_RETURN_CODE_OK
 *		
 **********************************************************************************/
unsigned int dhcp_snp_remove_vlan_broadcast_ports
(
	unsigned short ifindex,
	unsigned char isTagged,
	struct vlan_ports *tagPorts,
	struct vlan_ports *untagPorts
)
{
	struct vlan_ports *tmpPorts = NULL;
	unsigned int i = NPD_DHCP_SNP_INIT_0;
	unsigned int j = NPD_DHCP_SNP_INIT_0;
	unsigned int cnt = NPD_DHCP_SNP_INIT_0;

	if (!tagPorts || !untagPorts) {
		syslog_ax_dhcp_snp_err("remove port from vlan's broadcast ports error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	if (isTagged) {
		tmpPorts = tagPorts;
	}else {
		tmpPorts = untagPorts;
	}

	cnt = tmpPorts->count;
	for (i = 0; i < cnt; i++)
	{
		if (tmpPorts->ports[i] == ifindex)
		{
			for (j = i; j < cnt; j++) {
				tmpPorts->ports[j] = tmpPorts->ports[j + 1];
			}
			tmpPorts->count = cnt - 1;
			break;
		}	
	}
	
	return DHCP_SNP_RETURN_CODE_OK;
}
#endif

/**********************************************************************************
 * dhcp_snp_packet_rx_process()
 *	DESCRIPTION:
 *		receive packets for DHCP Snooping rx 
 *
 *	INPUTS:
 *		unsigned long numOfBuff		- number of buffer
 *		unsigned char *packetBuffs[]	- array of buffer
 *		unsigned long buffLen[]			- array of buffer length
 *		unsigned int interfaceId			- port Or TrunkId has been transfer to eth_g_index
 *		unsigned char isTagged			- tag flag of port
 *		unsigned short vid				- vlanid
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		GT_FAIL			- build fail
 *		GT_NO_RESOURCE	- alloc memory error
 *		GT_BAD_SIZE		- packet length is invalid
 *		GT_OK			- build ok
 *		totalLen			- receive packet length
***********************************************************************************/
unsigned int dhcp_snp_packet_rx_process
(
	unsigned long numOfBuff,
	unsigned char *packetBuffs[],
	unsigned long buffLen[],
	unsigned int ifindex,
	unsigned char isTagged,
	unsigned short vid,
	struct dhcp_snp_listener *node
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	/*unsigned char *temp = NULL;*/
	unsigned char *message = NULL;
	unsigned char trust_mode = NPD_DHCP_SNP_PORT_MODE_INVALID;
	NPD_DHCP_MESSAGE_T *data = NULL;
	unsigned int endposition = NPD_DHCP_SNP_INIT_0;
	unsigned int del_len = NPD_DHCP_SNP_INIT_0;

	/* send packet */
	unsigned long isMc = NPD_DHCP_SNP_INIT_0;
	unsigned int pktType = NPD_DHCP_SNP_INIT_0;

	if (!node || !packetBuffs || !packetBuffs[0]) {
		log_error("%s: parameter null.\n", __func__);
		return DHCP_SNP_RETURN_CODE_ERROR;		
	}
	
	if (dhcp_snp_status != NPD_DHCP_SNP_ENABLE) {
		log_info("dhcp snooping not enabled global.\n");
		return DHCP_SNP_RETURN_CODE_OK;
	}

	log_debug("interface %s recv dhcp packet vlan %d\n", node->ifname, vid);

	data = (NPD_DHCP_MESSAGE_T *)(packetBuffs[0] + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr)); 

	message = (unsigned char *)dhcp_snp_get_option(data, NPD_DHCP_MESSAGE_TYPE);
	if (NULL == message) {
		log_error("can not get dhcp pakcet type options(opt 53), ignoring.\n");
		return DHCP_SNP_RETURN_CODE_PKT_DROP;
	}

	if (NPD_DHCP_BOOTREPLY == data->op)	{
		ret = dhcp_snp_is_trusted(vid, ifindex, isTagged, &trust_mode);
		if ((DHCP_SNP_RETURN_CODE_OK != ret) ||
			((DHCP_SNP_RETURN_CODE_OK == ret) && (NPD_DHCP_SNP_PORT_MODE_NOTRUST == trust_mode))) {
			log_error("receive dhcp reply packet from untrusted interface %d, discard\n", ifindex);
			#if DHCPSNP_RD_DEBUG
			log_error("ret %#x trust mode %d!\n", ret, trust_mode);
			#endif
			return DHCP_SNP_RETURN_CODE_PKT_DROP;
		}
	}

	/*
	 * trust mode port	: record ip-mac infor, and transmit dhcp packets
	 * no-bind port		: not record ip-mac infor, but transmit dhcp packets
	 * notrust mode port	: not record ip-mac infor, drop dhcp packets
	 *
	 * note: dhcp snooping only deal(transmit or drop) offer and ack packet of dhcp protocol,
	 *		other packets of dhcp protocol transmit as usual.
	*/
	trust_mode = NPD_DHCP_SNP_PORT_MODE_TRUST;
	switch (*message)
	{
		case NPD_DHCP_DISCOVER :
			ret = dhcp_snp_discovery_process(vid, ifindex, data);
			if (DHCP_SNP_RETURN_CODE_OK != ret) {
				log_error("receive dhcp discover packet error, ret %x\n", ret);
				return DHCP_SNP_RETURN_CODE_PKT_DROP;
			}
			break;
		case NPD_DHCP_OFFER :
			if (NPD_DHCP_SNP_PORT_MODE_TRUST == trust_mode) {
				ret = dhcp_snp_offer_process(vid, ifindex, data, packetBuffs[0], buffLen[0], node->fd);
				if (DHCP_SNP_RETURN_CODE_OK != ret) {
					log_error("receive dhcp offer packet error, ret %x\n", ret);
					return DHCP_SNP_RETURN_CODE_PKT_DROP;
				}
			}else if (NPD_DHCP_SNP_PORT_MODE_NOBIND == trust_mode) {
				log_error("receive dhcp offer packet from no-bind interface %d\n", ifindex);
			}else {
				log_error("receive dhcp offer packet from untrusted interface %d, discard\n", ifindex);
				return DHCP_SNP_RETURN_CODE_PKT_DROP;
			}
			break;
		case NPD_DHCP_REQUEST :
			ret = dhcp_snp_request_process(vid, ifindex, data);
			if (DHCP_SNP_RETURN_CODE_OK != ret) {
				log_error("receive dhcp request packet error, ret %x\n", ret);
				return DHCP_SNP_RETURN_CODE_PKT_DROP;
			}
			break;
		case NPD_DHCP_ACK :
			if (NPD_DHCP_SNP_PORT_MODE_TRUST == trust_mode) {
				ret = dhcp_snp_ack_process(vid, ifindex, data, node);
				if (DHCP_SNP_RETURN_CODE_OK != ret) {
					log_error("receive dhcp ack packet error, ret %x\n", ret);
					return DHCP_SNP_RETURN_CODE_PKT_DROP;
				}
			}else if (NPD_DHCP_SNP_PORT_MODE_NOBIND == trust_mode) {
				syslog_ax_dhcp_snp_dbg("receive dhcp ack packet from no-bind interface %d\n", ifindex);
			}else {
				syslog_ax_dhcp_snp_err("receive dhcp ack packet from untrusted interface %d, discard\n", ifindex);
				return DHCP_SNP_RETURN_CODE_PKT_DROP;
			}
			break;
		case NPD_DHCP_NAK :
			ret = dhcp_snp_nack_process(vid, ifindex, data);
			if (DHCP_SNP_RETURN_CODE_OK != ret) {
				syslog_ax_dhcp_snp_err("receive dhcp nak packet error, ret %x\n", ret);
				return DHCP_SNP_RETURN_CODE_PKT_DROP;
			}
			break;
		case NPD_DHCP_RELEASE :
			ret = dhcp_snp_release_process(vid, ifindex, data, node);
			if (DHCP_SNP_RETURN_CODE_OK != ret) {
				syslog_ax_dhcp_snp_err("receive dhcp release packet error, ret %x\n", ret);
				return DHCP_SNP_RETURN_CODE_PKT_DROP;
			}
			break;
		case NPD_DHCP_INFORM :
			ret = dhcp_snp_inform_process(vid, ifindex, data, node);
			if (DHCP_SNP_RETURN_CODE_OK != ret) {
				syslog_ax_dhcp_snp_err("receive dhcp inform packet error, ret %x\n", ret);
				return DHCP_SNP_RETURN_CODE_PKT_DROP;
			}			
			break;
		case NPD_DHCP_DECLINE :
			break;
		default :
			syslog_ax_dhcp_snp_err("receive dhcp packet unknown message\n");
			break;
	}	

	/*
	*********************************************
	*  option 82 process
	*********************************************
	*/
	if (g_dhcp_snp_opt82_enable == NPD_DHCP_SNP_OPT82_ENABLE) {
		syslog_ax_dhcp_snp_dbg("start option 82 process.\n");
		/* attach option 82 */
		if (((*message) == NPD_DHCP_DISCOVER) || ((*message) == NPD_DHCP_REQUEST))
		{
			ret = dhcp_snp_attach_opt82(packetBuffs[0], ifindex, isTagged, vid, &buffLen[0]);
			if (DHCP_SNP_RETURN_CODE_OK != ret) {
				syslog_ax_dhcp_snp_err("attach option 82 error, ret %x\n", ret);
				return DHCP_SNP_RETURN_CODE_PKT_DROP;
			}
		}

		/* remove option 82 */
		if (NPD_DHCP_BOOTREPLY == data->op)
		{
			endposition = dhcp_snp_end_option(data->options);
			ret = dhcp_snp_remove_opt82(data, DHCP_OPTION_82, endposition, &del_len);
			if (DHCP_SNP_RETURN_CODE_OK != ret) {
				syslog_ax_dhcp_snp_err("remove option 82 error, ret %x\n", ret);
				return DHCP_SNP_RETURN_CODE_PKT_DROP;
			}

			buffLen[0] = buffLen[0] - del_len;
		}

		syslog_ax_dhcp_snp_dbg("end option 82 process.\n");
	}

	#if 0
	/* for debug */
	syslog_ax_dhcp_snp_dbg("for debug packet content: please open nam debug\n");
	dhcp_snp_listener_dump_detail(packetBuffs[0], buffLen[0]);
	#endif

	return DHCP_SNP_RETURN_CODE_OK;	
}

/**********************************************************************************
 * dhcp_snp_dbus_enable_global_status
 *		set DHCP-Snooping enable/disable global status
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK
 * 	 	DHCP_SNP_RETURN_CODE_ERROR
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_enable_global_status
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char isEnable = NPD_DHCP_SNP_INIT_0;	
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_BYTE, &isEnable,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

	if (isEnable) {
		ret = dhcp_snp_enable();
	}else {
		ret = dhcp_snp_disable();
	}
	/*
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("DHCP-Snooping %s global error, ret %x.\n",
								isEnable ? "enable" : "disable", ret);
		ret = DHCP_SNP_RETURN_CODE_ERROR;
	}*/

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
} 

/**********************************************************************************
 * dhcp_snp_dbus_check_global_status
 *		check and get DHCP-Snooping enable/disable global status
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 * 	 	DHCP_SNP_RETURN_CODE_OK
 *		DHCP_SNP_RETURN_CODE_ERROR
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_check_global_status
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char status = NPD_DHCP_SNP_INIT_0;	
	
	dbus_error_init(&err);
	
	ret = dhcp_snp_get_global_status(&status);
	if (ret != DHCP_SNP_RETURN_CODE_OK) {
		syslog_ax_dhcp_snp_err("check DHCP-Snooping global status %s error, ret %x\n",
							status ? "enable" : "disable", ret);
	}
	syslog_ax_dhcp_snp_dbg("check DHCP-Snooping global status %s.\n",
						status ? "enable" : "disable");

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_BYTE, &status);

	return reply;
} 
#if 0
/**********************************************************************************
 * dhcp_snp_dbus_enable_vlan_status
 *		enable/disable DHCP-Snooping on special vlan
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 * 	 	DHCP_SNP_RETURN_CODE_OK
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_enable_vlan_status
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char isEnable = NPD_DHCP_SNP_DISABLE;
	unsigned short vlanId = NPD_DHCP_SNP_INIT_0;

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT16, &vlanId,
								DBUS_TYPE_BYTE, &isEnable,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args \n");
		if (dbus_error_is_set(&err)) {
			syslog_ax_dhcp_snp_err("%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

	ret = dhcp_snp_check_global_status();
	if (ret == DHCP_SNP_RETURN_CODE_ENABLE_GBL)
	{
		ret = dhcp_snp_check_vlan_status(vlanId);
		if (DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST == ret) {
			syslog_ax_dhcp_snp_err("vlan %d not created.\n", vlanId);
		}
		else if ((NPD_DHCP_SNP_DISABLE == isEnable) &&
				 (DHCP_SNP_RETURN_CODE_NOT_EN_VLAN == ret))
		{
			ret = DHCP_SNP_RETURN_CODE_ALREADY_SET;
			syslog_ax_dhcp_snp_err("vlan %d already disabled DHCP Snooping.\n", vlanId);
		}
		else if ((NPD_DHCP_SNP_ENABLE == isEnable) &&
				 (DHCP_SNP_RETURN_CODE_EN_VLAN == ret))
		{
			ret = DHCP_SNP_RETURN_CODE_ALREADY_SET;
			syslog_ax_dhcp_snp_err("vlan %d already enabled DHCP Snooping.\n", vlanId);
		}
		else {
			syslog_ax_dhcp_snp_dbg("%s DHCP Snooping on vlan %d\n",
				(isEnable == NPD_DHCP_SNP_ENABLE) ? "enable" : "disable", vlanId);
			ret = dhcp_snp_vlan_endis_config(vlanId, isEnable);
			if (DHCP_SNP_RETURN_CODE_OK != ret) {
				syslog_ax_dhcp_snp_err("%s DHCP Snooping on vlan %d error, ret %x.\n",
					(isEnable == NPD_DHCP_SNP_ENABLE) ? "enable" : "disable",
					vlanId, ret);
			}
		}
	}
	else {
		syslog_ax_dhcp_snp_dbg("check DHCP-Snooping global status not enabled global.\n");
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
} 


/**********************************************************************************
 * dhcp_snp_dbus_config_port
 *		config DHCP-Snooping trust mode of port
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL		- not enable global
 *		DHCP_SNP_RETURN_CODE_NOT_EN_VLAN			- vlan not enable  
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET			- have set trust or no-bind port
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST		- vlan not exist
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 *		DHCP_SNP_RETURN_CODE_NO_SUCH_PORT			- slotno or portno is not legal
 *		DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR		- port is trunk member
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_config_port
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char slot_no = NPD_DHCP_SNP_INIT_0;
	unsigned char local_port_no = NPD_DHCP_SNP_INIT_0;
	unsigned short vlanId = NPD_DHCP_SNP_INIT_0;
	unsigned char trust_mode = NPD_DHCP_SNP_PORT_MODE_INVALID;

	unsigned int eth_g_index = NPD_DHCP_SNP_INIT_0;
	unsigned char tagMode = NPD_DHCP_SNP_INIT_0;
	struct eth_port_s* ethPort = NULL;

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT16, &vlanId,
								DBUS_TYPE_BYTE, &slot_no,
								DBUS_TYPE_BYTE, &local_port_no,
								DBUS_TYPE_BYTE, &trust_mode,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

	ret = dhcp_snp_check_global_status();
	if (ret == DHCP_SNP_RETURN_CODE_ENABLE_GBL)
	{
		ret = dhcp_snp_check_vlan_status(vlanId);
		if (DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST == ret) {
			syslog_ax_dhcp_snp_err("vlan %d not created.", vlanId);
		}
		else if (DHCP_SNP_RETURN_CODE_NOT_EN_VLAN == ret)
		{
			syslog_ax_dhcp_snp_err("config DHCP-Snooping trust mode error, ret %x.\n", ret);
		}
		else if (DHCP_SNP_RETURN_CODE_EN_VLAN == ret)
		{
			syslog_ax_dhcp_snp_dbg("config DHCP-Snooping trust mode %s on panel port %d/%d in vlan %d\n",
									(trust_mode == NPD_DHCP_SNP_PORT_MODE_TRUST) ? "trust" :
									((trust_mode == NPD_DHCP_SNP_PORT_MODE_NOBIND) ? "nobind" :
									((trust_mode == NPD_DHCP_SNP_PORT_MODE_NOTRUST) ? "notrust" : "unknow")),
									slot_no, local_port_no, vlanId);

			if (CHASSIS_SLOTNO_ISLEGAL(slot_no))
			{
				if (ETH_LOCAL_PORTNO_ISLEGAL(slot_no, local_port_no))
				{
					eth_g_index = ETH_GLOBAL_INDEX_FROM_SLOT_PORT_LOCAL_NO(slot_no, local_port_no);
					ethPort = npd_get_port_by_index(eth_g_index);
					if (NULL == ethPort) {
						syslog_ax_dhcp_snp_err("no found port by eth global index.\n");
						ret = DHCP_SNP_RETURN_CODE_NO_SUCH_PORT;
					}
					else if (NULL != ethPort->funcs.func_data[ETH_PORT_FUNC_LINK_AGGREGATION]) {
						/*port is member of trunk*/
						ret = DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR;
					}
					else if (NPD_TRUE != npd_vlan_check_contain_port(vlanId, eth_g_index, &tagMode)) {
						/*check port membership*/
						ret = DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST;	
					}
					else {
						ret = dhcp_snp_set_port_trust_mode(vlanId, eth_g_index, tagMode, trust_mode);
						if (DHCP_SNP_RETURN_CODE_OK != ret) {
							syslog_ax_dhcp_snp_err("config DHCP-Snooping trust mode error, ret %x.\n", ret);
						}
					}
				}
				else {
					syslog_ax_dhcp_snp_err("slotno or portno is not legal\n");
					ret = DHCP_SNP_RETURN_CODE_NO_SUCH_PORT;
				}
			}
			else {
				syslog_ax_dhcp_snp_err("slotno or portno is not legal\n");
				ret = DHCP_SNP_RETURN_CODE_NO_SUCH_PORT;				
			}
		}
	}
	else {	
		syslog_ax_dhcp_snp_dbg("check DHCP-Snooping global status not enabled global.\n");
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
} 

/**********************************************************************************
 * dhcp_snp_dbus_show_bind_table
 *		show DHCP-Snooping bind table
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 * 	 	DHCP_SNP_RETURN_CODE_OK
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *		DHCP_SNP_RETURN_CODE_ERROR
 *
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_show_bind_table
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
) 
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusMessageIter iter_array;
	DBusError err;
	
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int i = NPD_DHCP_SNP_INIT_0;
	unsigned char tmp_caddr[NPD_DHCP_SNP_MAC_ADD_LEN] =  {0};
	unsigned int record_count = NPD_DHCP_SNP_INIT_0;
	enum product_id_e productId = PRODUCT_ID;

	unsigned int slot_no = NPD_DHCP_SNP_INIT_0;
	unsigned int port_no = NPD_DHCP_SNP_INIT_0;

	NPD_DHCP_SNP_SHOW_ITEM_T show_items[MAX_ETHPORT_PER_BOARD * CHASSIS_SLOT_COUNT];

	/*	"Type", "IP Address", "MAC Address", "Lease", "VLAN", "PORT" */
	static char *select_all_sql = "SELECT BINDTYPE, IP, MAC, LEASE, VLAN, PORT FROM dhcpSnpDb;";

	memset(show_items, 0, sizeof(NPD_DHCP_SNP_SHOW_ITEM_T) * MAX_ETHPORT_PER_BOARD * CHASSIS_SLOT_COUNT);

	dbus_error_init(&err);

	ret = dhcp_snp_check_global_status();
	if (ret == DHCP_SNP_RETURN_CODE_ENABLE_GBL)
	{
		ret = dhcp_snp_tbl_query(select_all_sql, &record_count, show_items);
		if (DHCP_SNP_RETURN_CODE_OK != ret) {
			syslog_ax_dhcp_snp_err("get DHCP-Snooping bind table error, ret %x.\n", ret);
			ret = DHCP_SNP_RETURN_CODE_ERROR;
		}
		
		syslog_ax_dhcp_snp_dbg("query %d records from dhcpSnpDb success.\n", record_count);

	}else {	
		syslog_ax_dhcp_snp_dbg("check DHCP-Snooping global status not enabled global.\n");
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &record_count);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &productId);

	/*open outer container*/
	dbus_message_iter_open_container(&iter,
								   DBUS_TYPE_ARRAY,
								      DBUS_STRUCT_BEGIN_CHAR_AS_STRING  /*begin*/
   											DBUS_TYPE_UINT32_AS_STRING /*bind_type*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[0]*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[1]*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[2]*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[3]*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[4]*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[5]*/
										   	DBUS_TYPE_UINT32_AS_STRING /*ip_addr*/
										   	DBUS_TYPE_UINT16_AS_STRING /*vlanId*/
										   	DBUS_TYPE_UINT32_AS_STRING /*slot_no*/
  										   	DBUS_TYPE_UINT32_AS_STRING /*port_no*/
										   	DBUS_TYPE_UINT32_AS_STRING /*lease_time	*/									   	
    								  DBUS_STRUCT_END_CHAR_AS_STRING, /*end*/
								   &iter_array);
	for (i = 0; i < record_count; i++) {
		slot_no = NPD_DHCP_SNP_INIT_0;
		port_no = NPD_DHCP_SNP_INIT_0;
		memset(tmp_caddr, 0, 6);

		DBusMessageIter iter_struct;
		dbus_message_iter_open_container(&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&(show_items[i].bind_type));

		dhcp_snp_mac_ascii_to_hex(show_items[i].chaddr, tmp_caddr, 6);

		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&(tmp_caddr[0]));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&(tmp_caddr[1]));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&(tmp_caddr[2]));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&(tmp_caddr[3]));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&(tmp_caddr[4]));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&(tmp_caddr[5]));
		
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&(show_items[i].ip_addr));

		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT16,
										&(show_items[i].vlanId));

		dhcp_snp_get_slot_port_from_eth_index(show_items[i].ifindex, &slot_no, &port_no);
		
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&(slot_no));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&(port_no));

		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&(show_items[i].lease_time));

		dbus_message_iter_close_container(&iter_array, &iter_struct);

	}
	dbus_message_iter_close_container(&iter, &iter_array);

	return reply;
}

/**********************************************************************************
 * dhcp_snp_dbus_show_static_bind_table
 *		show DHCP-Snooping static bind table
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 * 	 	DHCP_SNP_RETURN_CODE_OK
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *		DHCP_SNP_RETURN_CODE_ERROR
 *
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_show_static_bind_table
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
) 
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusMessageIter iter_array;
	DBusError err;
	
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int i = NPD_DHCP_SNP_INIT_0;
	unsigned char tmp_caddr[NPD_DHCP_SNP_MAC_ADD_LEN] =  {0};
	unsigned int record_count = NPD_DHCP_SNP_INIT_0;
	enum product_id_e productId = PRODUCT_ID;

	unsigned int slot_no = NPD_DHCP_SNP_INIT_0;
	unsigned int port_no = NPD_DHCP_SNP_INIT_0;

	NPD_DHCP_SNP_SHOW_ITEM_T show_items[MAX_ETHPORT_PER_BOARD * CHASSIS_SLOT_COUNT];

	/* "Type", "IP Address", "MAC Address", "Lease", "VLAN", "PORT" */
	/* item is static, when BINDTYPE = 1 */
	static char *select_all_sql = "SELECT BINDTYPE, IP, MAC, LEASE, VLAN, PORT FROM dhcpSnpDb WHERE BINDTYPE=1;";

	memset(show_items, 0, sizeof(NPD_DHCP_SNP_SHOW_ITEM_T) * MAX_ETHPORT_PER_BOARD * CHASSIS_SLOT_COUNT);

	dbus_error_init(&err);

	ret = dhcp_snp_check_global_status();
	if (ret == DHCP_SNP_RETURN_CODE_ENABLE_GBL)
	{
		ret = dhcp_snp_tbl_query(select_all_sql, &record_count, show_items);
		if (DHCP_SNP_RETURN_CODE_OK != ret) {
			syslog_ax_dhcp_snp_err("get DHCP-Snooping bind table error, ret %x.\n", ret);
			ret = DHCP_SNP_RETURN_CODE_ERROR;
		}
		
		syslog_ax_dhcp_snp_dbg("query %d records from dhcpSnpDb success.\n", record_count);

	}else {	
		syslog_ax_dhcp_snp_dbg("check DHCP-Snooping global status not enabled global.\n");
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &record_count);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &productId);

	/*open outer container*/
	dbus_message_iter_open_container(&iter,
								   DBUS_TYPE_ARRAY,
								      DBUS_STRUCT_BEGIN_CHAR_AS_STRING  /*begin*/
   											DBUS_TYPE_UINT32_AS_STRING /*bind_type*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[0]*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[1]*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[2]*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[3]*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[4]*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[5]*/
										   	DBUS_TYPE_UINT32_AS_STRING /*ip_addr*/
										   	DBUS_TYPE_UINT16_AS_STRING /*vlanId*/
										   	DBUS_TYPE_UINT32_AS_STRING /*slot_no*/
  										   	DBUS_TYPE_UINT32_AS_STRING /*port_no*/
										   	DBUS_TYPE_UINT32_AS_STRING /*lease_time	*/									   	
    								  DBUS_STRUCT_END_CHAR_AS_STRING, /*end*/
								   &iter_array);
	for (i = 0; i < record_count; i++) {
		slot_no = NPD_DHCP_SNP_INIT_0;
		port_no = NPD_DHCP_SNP_INIT_0;
		memset(tmp_caddr, 0, 6);

		DBusMessageIter iter_struct;
		dbus_message_iter_open_container(&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&(show_items[i].bind_type));

		dhcp_snp_mac_ascii_to_hex(show_items[i].chaddr, tmp_caddr, 6);

		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&(tmp_caddr[0]));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&(tmp_caddr[1]));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&(tmp_caddr[2]));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&(tmp_caddr[3]));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&(tmp_caddr[4]));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&(tmp_caddr[5]));
		
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&(show_items[i].ip_addr));

		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT16,
										&(show_items[i].vlanId));

		dhcp_snp_get_slot_port_from_eth_index(show_items[i].ifindex, &slot_no, &port_no);
		
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&(slot_no));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&(port_no));

		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&(show_items[i].lease_time));

		dbus_message_iter_close_container(&iter_array, &iter_struct);

	}
	dbus_message_iter_close_container(&iter, &iter_array);

	return reply;
}

/**********************************************************************************
 * dhcp_snp_dbus_show_static_bind_table_by_vlan
 *		show DHCP-Snooping static bind item which belong to the vlan in the DB.
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 * 	 	DHCP_SNP_RETURN_CODE_OK
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *		DHCP_SNP_RETURN_CODE_ERROR
 *
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_show_static_bind_table_by_vlan
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
) 
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusMessageIter iter_array;
	DBusError err;
	
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned short vlanId = NPD_DHCP_SNP_INIT_0;
	
	unsigned int i = NPD_DHCP_SNP_INIT_0;
	unsigned char tmp_caddr[NPD_DHCP_SNP_MAC_ADD_LEN] =  {0};
	unsigned int record_count = NPD_DHCP_SNP_INIT_0;
	enum product_id_e productId = PRODUCT_ID;

	unsigned int slot_no = NPD_DHCP_SNP_INIT_0;
	unsigned int port_no = NPD_DHCP_SNP_INIT_0;

	NPD_DHCP_SNP_SHOW_ITEM_T show_items[MAX_ETHPORT_PER_BOARD * CHASSIS_SLOT_COUNT];

	char sql_str[1024] = {0};
	char *bufPtr = NULL;
	int sqlstr_len = NPD_DHCP_SNP_INIT_0;

	memset(show_items, 0, sizeof(NPD_DHCP_SNP_SHOW_ITEM_T) * MAX_ETHPORT_PER_BOARD * CHASSIS_SLOT_COUNT);

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT16, &vlanId,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args \n");
		if (dbus_error_is_set(&err)) {
			syslog_ax_dhcp_snp_err("%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

	/*
	 ************************************************************************
	 * cat select sql_string
	 ************************************************************************
	 */
	/* "Type", "IP Address", "MAC Address", "Lease", "VLAN", "PORT" */
	bufPtr = sql_str;

	/* item is static, when BINDTYPE = 1 */
	sqlstr_len += sprintf(bufPtr, "SELECT BINDTYPE, IP, MAC, LEASE, VLAN, PORT FROM dhcpSnpDb WHERE BINDTYPE=1 ");
	bufPtr = sql_str + sqlstr_len;

	sqlstr_len += sprintf(bufPtr, "and VLAN=");
	bufPtr = sql_str + sqlstr_len;

	sqlstr_len += sprintf(bufPtr, "%d", vlanId);
	bufPtr = sql_str + sqlstr_len;

	sqlstr_len += sprintf(bufPtr, ";");
	bufPtr = sql_str + sqlstr_len;

	syslog_ax_dhcp_snp_dbg("show by vlan sql string:%s\n", sql_str);

	/*
	 ************************************************************************
	 * exec sql_string
	 ************************************************************************
	 */
	ret = dhcp_snp_check_global_status();
	if (ret == DHCP_SNP_RETURN_CODE_ENABLE_GBL)
	{
		ret = dhcp_snp_tbl_query(sql_str, &record_count, show_items);
		if (DHCP_SNP_RETURN_CODE_OK != ret) {
			syslog_ax_dhcp_snp_err("get DHCP-Snooping bind table by vlan error, ret %x.\n", ret);
			ret = DHCP_SNP_RETURN_CODE_ERROR;
		}
		
		syslog_ax_dhcp_snp_dbg("query %d records from dhcpSnpDb success.\n", record_count);
	}else {	
		syslog_ax_dhcp_snp_dbg("check DHCP-Snooping global status not enabled global.\n");
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &record_count);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &productId);

	/*open outer container*/
	dbus_message_iter_open_container(&iter,
								   DBUS_TYPE_ARRAY,
								      DBUS_STRUCT_BEGIN_CHAR_AS_STRING  /*begin*/
   											DBUS_TYPE_UINT32_AS_STRING /*bind_type*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[0]*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[1]*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[2]*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[3]*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[4]*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[5]*/
										   	DBUS_TYPE_UINT32_AS_STRING /*ip_addr*/
										   	DBUS_TYPE_UINT16_AS_STRING /*vlanId*/
										   	DBUS_TYPE_UINT32_AS_STRING /*slot_no*/
  										   	DBUS_TYPE_UINT32_AS_STRING /*port_no*/
										   	DBUS_TYPE_UINT32_AS_STRING /*lease_time	*/									   	
    								  DBUS_STRUCT_END_CHAR_AS_STRING, /*end*/
								   &iter_array);
	for (i = 0; i < record_count; i++) {
		slot_no = NPD_DHCP_SNP_INIT_0;
		port_no = NPD_DHCP_SNP_INIT_0;
		memset(tmp_caddr, 0, 6);

		DBusMessageIter iter_struct;
		dbus_message_iter_open_container(&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&(show_items[i].bind_type));

		dhcp_snp_mac_ascii_to_hex(show_items[i].chaddr, tmp_caddr, 6);

		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&(tmp_caddr[0]));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&(tmp_caddr[1]));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&(tmp_caddr[2]));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&(tmp_caddr[3]));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&(tmp_caddr[4]));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&(tmp_caddr[5]));
		
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&(show_items[i].ip_addr));

		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT16,
										&(show_items[i].vlanId));

		dhcp_snp_get_slot_port_from_eth_index(show_items[i].ifindex, &slot_no, &port_no);
		
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&(slot_no));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&(port_no));

		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&(show_items[i].lease_time));

		dbus_message_iter_close_container(&iter_array, &iter_struct);

	}
	dbus_message_iter_close_container(&iter, &iter_array);

	return reply;
}

/**********************************************************************************
 * dhcp_snp_dbus_show_static_bind_table_by_ethport
 *		show DHCP-Snooping static bind item which belong to the etyport in the DB.
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 * 	 	DHCP_SNP_RETURN_CODE_OK
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *		DHCP_SNP_RETURN_CODE_NO_SUCH_PORT
 *		DHCP_SNP_RETURN_CODE_ERROR
 *
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_show_static_bind_table_by_ethport
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
) 
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusMessageIter iter_array;
	DBusError err;
	
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int i = NPD_DHCP_SNP_INIT_0;
	unsigned char tmp_caddr[NPD_DHCP_SNP_MAC_ADD_LEN] =  {0};
	unsigned int record_count = NPD_DHCP_SNP_INIT_0;
	enum product_id_e productId = PRODUCT_ID;
	/* get parameters from dcli */
	unsigned char local_slot_no = NPD_DHCP_SNP_INIT_0;
	unsigned char local_port_no = NPD_DHCP_SNP_INIT_0;
	unsigned int eth_g_index = NPD_DHCP_SNP_INIT_0;
	/* temp */
	unsigned int slot_no = NPD_DHCP_SNP_INIT_0;
	unsigned int port_no = NPD_DHCP_SNP_INIT_0;

	NPD_DHCP_SNP_SHOW_ITEM_T show_items[MAX_ETHPORT_PER_BOARD * CHASSIS_SLOT_COUNT];

	char sql_str[1024] = {0};
	char *bufPtr = NULL;
	int sqlstr_len = NPD_DHCP_SNP_INIT_0;

	memset(show_items, 0, sizeof(NPD_DHCP_SNP_SHOW_ITEM_T) * MAX_ETHPORT_PER_BOARD * CHASSIS_SLOT_COUNT);

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_BYTE, &local_slot_no,
								DBUS_TYPE_BYTE, &local_port_no,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args \n");
		if (dbus_error_is_set(&err)) {
			syslog_ax_dhcp_snp_err("%s raised: %s\n", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

	/*
	 ************************************************************************
	 * cat select sql_string
	 ************************************************************************
	 */
	if (CHASSIS_SLOTNO_ISLEGAL(local_slot_no))
	{
		if (ETH_LOCAL_PORTNO_ISLEGAL(local_slot_no, local_port_no))
		{
			eth_g_index = ETH_GLOBAL_INDEX_FROM_SLOT_PORT_LOCAL_NO(local_slot_no, local_port_no);
			
			/* "Type", "IP Address", "MAC Address", "Lease", "VLAN", "PORT" */
			bufPtr = sql_str;
			
			/* item is static, when BINDTYPE = 1 */
			sqlstr_len += sprintf(bufPtr, "SELECT BINDTYPE, IP, MAC, LEASE, VLAN, PORT FROM dhcpSnpDb WHERE BINDTYPE=1 ");
			bufPtr = sql_str + sqlstr_len;
			
			sqlstr_len += sprintf(bufPtr, "and PORT=");
			bufPtr = sql_str + sqlstr_len;
			
			sqlstr_len += sprintf(bufPtr, "%d", eth_g_index);
			bufPtr = sql_str + sqlstr_len;
			
			sqlstr_len += sprintf(bufPtr, ";");
			bufPtr = sql_str + sqlstr_len;
			
			syslog_ax_dhcp_snp_dbg("show by ethport sql string:%s\n", sql_str);
			
			/*
			 ************************************************************************
			 * exec sql_string
			 ************************************************************************
			 */
			ret = dhcp_snp_check_global_status();
			if (ret == DHCP_SNP_RETURN_CODE_ENABLE_GBL)
			{
				ret = dhcp_snp_tbl_query(sql_str, &record_count, show_items);
				if (DHCP_SNP_RETURN_CODE_OK != ret) {
					syslog_ax_dhcp_snp_err("get DHCP-Snooping static bind table error, ret %x.\n", ret);
					ret = DHCP_SNP_RETURN_CODE_ERROR;
				}
				
				syslog_ax_dhcp_snp_dbg("query %d records from dhcpSnpDb success.\n", record_count);
			}else { 
				syslog_ax_dhcp_snp_dbg("check DHCP-Snooping global status not enabled global.\n");
			}
			
		}else {
			syslog_ax_dhcp_snp_err("slotno or portno is not legal\n");
			ret = DHCP_SNP_RETURN_CODE_NO_SUCH_PORT;				
		}
	}
	else {
		syslog_ax_dhcp_snp_err("slotno or portno is not legal\n");
		ret = DHCP_SNP_RETURN_CODE_NO_SUCH_PORT;				
	}
	
	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &record_count);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &productId);

	/*open outer container*/
	dbus_message_iter_open_container(&iter,
								   DBUS_TYPE_ARRAY,
								      DBUS_STRUCT_BEGIN_CHAR_AS_STRING  /*begin*/
   											DBUS_TYPE_UINT32_AS_STRING /*bind_type*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[0]*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[1]*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[2]*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[3]*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[4]*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[5]*/
										   	DBUS_TYPE_UINT32_AS_STRING /*ip_addr*/
										   	DBUS_TYPE_UINT16_AS_STRING /*vlanId*/
										   	DBUS_TYPE_UINT32_AS_STRING /*slot_no*/
  										   	DBUS_TYPE_UINT32_AS_STRING /*port_no*/
										   	DBUS_TYPE_UINT32_AS_STRING /*lease_time	*/									   	
    								  DBUS_STRUCT_END_CHAR_AS_STRING, /*end*/
								   &iter_array);
	for (i = 0; i < record_count; i++) {
		slot_no = NPD_DHCP_SNP_INIT_0;
		port_no = NPD_DHCP_SNP_INIT_0;
		memset(tmp_caddr, 0, 6);

		DBusMessageIter iter_struct;
		dbus_message_iter_open_container(&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&(show_items[i].bind_type));

		dhcp_snp_mac_ascii_to_hex(show_items[i].chaddr, tmp_caddr, 6);

		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&(tmp_caddr[0]));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&(tmp_caddr[1]));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&(tmp_caddr[2]));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&(tmp_caddr[3]));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&(tmp_caddr[4]));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE,
										&(tmp_caddr[5]));
		
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&(show_items[i].ip_addr));

		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT16,
										&(show_items[i].vlanId));

		dhcp_snp_get_slot_port_from_eth_index(show_items[i].ifindex, &slot_no, &port_no);
		
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&(slot_no));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&(port_no));

		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&(show_items[i].lease_time));

		dbus_message_iter_close_container(&iter_array, &iter_struct);

	}
	dbus_message_iter_close_container(&iter, &iter_array);

	return reply;
}


/**********************************************************************************
 * dhcp_snp_dbus_show_trust_ports
 *		show DHCP-Snooping trust and no-bind ports
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 * 	 	DHCP_SNP_RETURN_CODE_OK
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *		DHCP_SNP_RETURN_CODE_ERROR
 *
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_show_trust_ports
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusMessageIter iter_array;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int i = NPD_DHCP_SNP_INIT_0;
	unsigned int trust_ports_cnt = NPD_DHCP_SNP_INIT_0;
	enum product_id_e productId = PRODUCT_ID;
	NPD_DHCP_SNP_SHOW_TRUST_PORTS_T show_trust_ports[MAX_ETHPORT_PER_BOARD * CHASSIS_SLOT_COUNT];
	memset(show_trust_ports, 0, sizeof(NPD_DHCP_SNP_SHOW_TRUST_PORTS_T) * MAX_ETHPORT_PER_BOARD * CHASSIS_SLOT_COUNT);

	dbus_error_init(&err);
	
	ret = dhcp_snp_check_global_status();
	if (ret == DHCP_SNP_RETURN_CODE_ENABLE_GBL)
	{
		ret = dhcp_snp_get_trust_ports(&trust_ports_cnt, show_trust_ports);
		if (DHCP_SNP_RETURN_CODE_OK != ret) {
			syslog_ax_dhcp_snp_err("get DHCP-Snooping trust and no-bind ports error, ret %x.\n", ret);
			ret = DHCP_SNP_RETURN_CODE_ERROR;
		}
	}else { 
		syslog_ax_dhcp_snp_dbg("check DHCP-Snooping global status not enabled global.\n");
	}

	syslog_ax_dhcp_snp_dbg("get trust or un-bind ports count %d.\n", trust_ports_cnt);

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &ret);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &trust_ports_cnt);

	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32,
									 &productId);

	/*open outer container*/
	dbus_message_iter_open_container(&iter,
								   DBUS_TYPE_ARRAY,
								      DBUS_STRUCT_BEGIN_CHAR_AS_STRING  /*begin*/
								      		DBUS_TYPE_UINT16_AS_STRING /*vlan id*/
										   	DBUS_TYPE_UINT32_AS_STRING /*slot_no*/
  										   	DBUS_TYPE_UINT32_AS_STRING /*port_no*/
  										   	DBUS_TYPE_UINT32_AS_STRING /*tag_mode*/
										   	DBUS_TYPE_UINT32_AS_STRING /*trust_mode*/									   	
    								  DBUS_STRUCT_END_CHAR_AS_STRING, /*end*/
								   &iter_array);
	for (i = 0; i < trust_ports_cnt; i++) {
		DBusMessageIter iter_struct;
		dbus_message_iter_open_container(&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT16,
										&(show_trust_ports[i].vlanId));

		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&(show_trust_ports[i].slot_no));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&(show_trust_ports[i].port_no));

		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&(show_trust_ports[i].tag_mode));

		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32,
										&(show_trust_ports[i].trust_mode));

		dbus_message_iter_close_container(&iter_array, &iter_struct);

	}
	dbus_message_iter_close_container(&iter, &iter_array);

	return reply;
} 

/**********************************************************************************
 * dhcp_snp_dbus_enable_opt82
 *		enable/disable DHCP-Snooping option82
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK
 * 	 	DHCP_SNP_RETURN_CODE_ERROR
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_enable_opt82
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char isEnable = NPD_DHCP_SNP_INIT_0;	
	unsigned char opt82_status = NPD_DHCP_SNP_OPT82_DISABLE;	

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_BYTE, &isEnable,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

	ret = dhcp_snp_check_global_status();
	if (ret == DHCP_SNP_RETURN_CODE_ENABLE_GBL)
	{
		ret = dhcp_snp_get_opt82_status(&opt82_status);
		if (ret != DHCP_SNP_RETURN_CODE_OK) {
			syslog_ax_dhcp_snp_err("check DHCP-Snooping option82 status %s error, ret %x\n",
								opt82_status ? "enable" : "disable", ret);
			ret = DHCP_SNP_RETURN_CODE_ERROR;
		}else {
			syslog_ax_dhcp_snp_dbg("check DHCP-Snooping option82 status %s.\n",
								opt82_status ? "enable" : "disable");
			
			if (opt82_status == isEnable) {
				syslog_ax_dhcp_snp_err("DHCP-Snooping option82 status already set %s\n",
									opt82_status ? "enable" : "disable");
				ret = DHCP_SNP_RETURN_CODE_ALREADY_SET;
			}else {
				if (isEnable) {
					ret = dhcp_snp_opt82_enable();
				}else {
					ret = dhcp_snp_opt82_disable();
				}
				if (DHCP_SNP_RETURN_CODE_OK != ret) {
					syslog_ax_dhcp_snp_err("DHCP-Snooping %s option 82 error, ret %x.\n",
											isEnable ? "enable" : "disable", ret);
					ret = DHCP_SNP_RETURN_CODE_ERROR;
				}
			}
		}
	}
	else { 
		syslog_ax_dhcp_snp_dbg("check DHCP-Snooping global status not enabled global.\n");
		ret = DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL;
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
} 

/**********************************************************************************
 * dhcp_snp_dbus_set_opt82_format_type
 *		set storage format type of  DHCP-Snooping option82
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK
 * 	 	DHCP_SNP_RETURN_CODE_ERROR
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_set_opt82_format_type
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char format_type = NPD_DHCP_SNP_OPT82_FORMAT_TYPE_INVALID;	

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_BYTE, &format_type,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

	ret = dhcp_snp_check_global_status();
	if (ret == DHCP_SNP_RETURN_CODE_ENABLE_GBL)
	{
		if (dhcp_snp_opt82_format_type == format_type) {
			syslog_ax_dhcp_snp_err("DHCP-Snooping option82 storage format type already set %s\n",
								dhcp_snp_opt82_format_type ? "ascii" : "hex");
			ret = DHCP_SNP_RETURN_CODE_ALREADY_SET;
		}else {
			syslog_ax_dhcp_snp_dbg("set DHCP-Snooping option82 storage format type %s.\n",
								format_type ? "ascii" : "hex");
			dhcp_snp_opt82_format_type = format_type;
			ret = DHCP_SNP_RETURN_CODE_OK;
		}
	}
	else { 
		syslog_ax_dhcp_snp_dbg("check DHCP-Snooping global status not enabled global.\n");
		ret = DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL;
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
} 




/**********************************************************************************
 * dhcp_snp_dbus_set_opt82_format_type
 *		set fill format type of  DHCP-Snooping option82
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK
 * 	 	DHCP_SNP_RETURN_CODE_ERROR
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_set_opt82_fill_format_type
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char fill_type = NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_INVALID;	

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_BYTE, &fill_type,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

	ret = dhcp_snp_check_global_status();
	if (ret == DHCP_SNP_RETURN_CODE_ENABLE_GBL)
	{
		if (dhcp_snp_opt82_fill_format_type == fill_type) {
			syslog_ax_dhcp_snp_err("DHCP-Snooping option82 packet fill-format type already set %s\n",
				(dhcp_snp_opt82_fill_format_type == NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_EXT) ? "extended" :
				((dhcp_snp_opt82_fill_format_type == NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_STD) ? "standard" : "unknown"));
			ret = DHCP_SNP_RETURN_CODE_ALREADY_SET;
		}else {
			syslog_ax_dhcp_snp_dbg("set DHCP-Snooping option82 packet fill-format type %s.\n",
									(fill_type == NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_EXT) ? "extended" :
									((fill_type == NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_STD) ? "standard" : "unknown"));
			dhcp_snp_opt82_fill_format_type = fill_type;
			ret = DHCP_SNP_RETURN_CODE_OK;
		}
	}
	else {
		syslog_ax_dhcp_snp_dbg("check DHCP-Snooping global status not enabled global.\n");
		ret = DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL;
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
} 

/**********************************************************************************
 * dhcp_snp_dbus_set_opt82_remoteid_content
 *		set remote-id type and content of  DHCP-Snooping option82
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK
 * 	 	DHCP_SNP_RETURN_CODE_ERROR
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_set_opt82_remoteid_content
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char remoteid_type = NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_INVALID;	
	char *remoteid_str = NULL;

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_BYTE, &remoteid_type,
								DBUS_TYPE_STRING, &remoteid_str,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

	ret = dhcp_snp_check_global_status();
	if (ret == DHCP_SNP_RETURN_CODE_ENABLE_GBL)
	{
		ret = DHCP_SNP_RETURN_CODE_OK;
		if (remoteid_type == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC) {
			/* set default value for remote-id content */
			dhcp_snp_opt82_remoteid_type = remoteid_type;
			memset(dhcp_snp_opt82_remoteid_str, 0, NPD_DHCP_SNP_REMOTEID_STR_LEN);
			memcpy(dhcp_snp_opt82_remoteid_str, (unsigned char*)PRODUCT_MAC_ADDRESS, 13);
		}else if (remoteid_type == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSNAME) {
			/* set system name for remote-id content */
			dhcp_snp_opt82_remoteid_type = remoteid_type;
			memset(dhcp_snp_opt82_remoteid_str, 0, NPD_DHCP_SNP_REMOTEID_STR_LEN);
			memcpy(dhcp_snp_opt82_remoteid_str, (unsigned char*)PRODUCT_SYSTEM_NAME, strlen(PRODUCT_SYSTEM_NAME));/* 24 is SYSINFO_PRODUCT_NAME */
		}else if (remoteid_type == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_USERSTR) {
			/* set user-defined string for remote-id content */
			dhcp_snp_opt82_remoteid_type = remoteid_type;
			memset(dhcp_snp_opt82_remoteid_str, 0, NPD_DHCP_SNP_REMOTEID_STR_LEN);
			/*memcpy(dhcp_snp_opt82_remoteid_str, remoteid_str, NPD_DHCP_SNP_REMOTEID_STR_LEN);*/
			memcpy(dhcp_snp_opt82_remoteid_str, remoteid_str, strlen(remoteid_str));
		}else {
			syslog_ax_dhcp_snp_err("set ERROR remote-id type %d of DHCP-Snooping option82.\n", remoteid_type);
			ret = DHCP_SNP_RETURN_CODE_ERROR;
		}

		syslog_ax_dhcp_snp_dbg("remote-id type is %d, content [%s]\n",
								dhcp_snp_opt82_remoteid_type, dhcp_snp_opt82_remoteid_str);
	}
	else {
		syslog_ax_dhcp_snp_dbg("check DHCP-Snooping global status not enabled global.\n");
		ret = DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL;
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
} 

/**********************************************************************************
 * dhcp_snp_dbus_set_opt82_port_strategy
 *		set DHCP-Snooping option 82 strategy mode of port
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL		- not enable global
 *		DHCP_SNP_RETURN_CODE_NOT_EN_VLAN			- vlan not enable  
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET			- option 82 strategy already setted same value
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST		- vlan not exist
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 *		DHCP_SNP_RETURN_CODE_NO_SUCH_PORT			- slotno or portno is not legal
 *		DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR		- port is trunk member
 * 
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_set_opt82_port_strategy
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char slot_no = NPD_DHCP_SNP_INIT_0;
	unsigned char local_port_no = NPD_DHCP_SNP_INIT_0;
	unsigned short vlanId = NPD_DHCP_SNP_INIT_0;
	unsigned char strategy_mode = NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_INVALID;

	unsigned int eth_g_index = NPD_DHCP_SNP_INIT_0;
	unsigned char tagMode = NPD_DHCP_SNP_INIT_0;
	struct eth_port_s* ethPort = NULL;

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT16, &vlanId,
								DBUS_TYPE_BYTE, &slot_no,
								DBUS_TYPE_BYTE, &local_port_no,
								DBUS_TYPE_BYTE, &strategy_mode,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

	syslog_ax_dhcp_snp_dbg("strategy_mode %d \n", strategy_mode);

	ret = dhcp_snp_check_global_status();
	if (ret == DHCP_SNP_RETURN_CODE_ENABLE_GBL)
	{
		ret = dhcp_snp_check_vlan_status(vlanId);
		if (DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST == ret) {
			syslog_ax_dhcp_snp_err("vlan %d not created.", vlanId);
		}
		else if (DHCP_SNP_RETURN_CODE_NOT_EN_VLAN == ret)
		{
			syslog_ax_dhcp_snp_err("config DHCP-Snooping option82 strategy mode error, ret %x.\n", ret);
		}
		else if (DHCP_SNP_RETURN_CODE_EN_VLAN == ret)
		{
			syslog_ax_dhcp_snp_dbg("config DHCP-Snooping option82 strategy mode %s on panel port %d/%d in vlan %d\n",
									(strategy_mode == NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_DROP) ? "drop" :
									((strategy_mode == NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_KEEP) ? "keep" :
									((strategy_mode == NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_REPLACE) ? "replace" : "unknow")),
									slot_no, local_port_no, vlanId);

			if (CHASSIS_SLOTNO_ISLEGAL(slot_no))
			{
				if (ETH_LOCAL_PORTNO_ISLEGAL(slot_no, local_port_no))
				{
					eth_g_index = ETH_GLOBAL_INDEX_FROM_SLOT_PORT_LOCAL_NO(slot_no, local_port_no);
					ethPort = npd_get_port_by_index(eth_g_index);
					if (NULL == ethPort) {
						syslog_ax_dhcp_snp_err("no found port by eth global index.\n");
						ret = DHCP_SNP_RETURN_CODE_NO_SUCH_PORT;
					}
					else if (NULL != ethPort->funcs.func_data[ETH_PORT_FUNC_LINK_AGGREGATION]) {
						/*port is member of trunk*/
						ret = DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR;
					}
					else if (NPD_TRUE != npd_vlan_check_contain_port(vlanId, eth_g_index, &tagMode)) {
						/*check port membership*/
						ret = DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST;	
					}
					else {
						ret = dhcp_snp_set_opt82_port_strategy(vlanId, eth_g_index, tagMode, strategy_mode);
						if (DHCP_SNP_RETURN_CODE_OK != ret) {
							syslog_ax_dhcp_snp_err("config DHCP-Snooping option82 strategy mode error, ret %x.\n", ret);
						}
					}
				}
				else {
					syslog_ax_dhcp_snp_err("slotno or portno is not legal\n");
					ret = DHCP_SNP_RETURN_CODE_NO_SUCH_PORT;
				}
			}
			else {
				syslog_ax_dhcp_snp_err("slotno or portno is not legal\n");
				ret = DHCP_SNP_RETURN_CODE_NO_SUCH_PORT;				
			}
		}
	}
	else {	
		syslog_ax_dhcp_snp_dbg("check DHCP-Snooping global status not enabled global.\n");
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
}

/**********************************************************************************
 * dhcp_snp_dbus_set_opt82_port_circuitid_content
 *		set circuit-id type and content of  DHCP-Snooping option82 on special port
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL		- not enable global
 *		DHCP_SNP_RETURN_CODE_NOT_EN_VLAN			- vlan not enable  
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET			- option 82  circuit-id mode already setted same value
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST		- vlan not exist
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 *		DHCP_SNP_RETURN_CODE_NO_SUCH_PORT			- slotno or portno is not legal
 *		DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR		- port is trunk member
 *		DHCP_SNP_RETURN_CODE_ERROR					- fail
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_set_opt82_port_circuitid_content
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char slot_no = NPD_DHCP_SNP_INIT_0;
	unsigned char local_port_no = NPD_DHCP_SNP_INIT_0;
	unsigned short vlanId = NPD_DHCP_SNP_INIT_0;
	unsigned char circuitid_type = NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_INVALID;	
	char *circuitid_str = NULL;

	unsigned int eth_g_index = NPD_DHCP_SNP_INIT_0;
	unsigned char tagMode = NPD_DHCP_SNP_INIT_0;
	struct eth_port_s* ethPort = NULL;

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT16, &vlanId,
								DBUS_TYPE_BYTE, &slot_no,
								DBUS_TYPE_BYTE, &local_port_no,
								DBUS_TYPE_BYTE, &circuitid_type,
								DBUS_TYPE_STRING, &circuitid_str,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

	ret = dhcp_snp_check_global_status();
	if (ret == DHCP_SNP_RETURN_CODE_ENABLE_GBL)
	{
		ret = dhcp_snp_check_vlan_status(vlanId);
		if (DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST == ret) {
			syslog_ax_dhcp_snp_err("vlan %d not created.", vlanId);
		}
		else if (DHCP_SNP_RETURN_CODE_NOT_EN_VLAN == ret)
		{
			syslog_ax_dhcp_snp_err("config DHCP-Snooping option82 Circuit-ID mode error, ret %x.\n", ret);
		}
		else if (DHCP_SNP_RETURN_CODE_EN_VLAN == ret)
		{
			syslog_ax_dhcp_snp_dbg("config DHCP-Snooping option82 Circuit-ID mode %s on panel port %d/%d in vlan %d.\n",
									(circuitid_type == NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_DEFAULT) ? "default" :
									((circuitid_type == NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_USERSTR) ? "user-defined" : "unknow"),
									slot_no, local_port_no, vlanId);

			if (CHASSIS_SLOTNO_ISLEGAL(slot_no))
			{
				if (ETH_LOCAL_PORTNO_ISLEGAL(slot_no, local_port_no))
				{
					eth_g_index = ETH_GLOBAL_INDEX_FROM_SLOT_PORT_LOCAL_NO(slot_no, local_port_no);
					ethPort = npd_get_port_by_index(eth_g_index);
					if (NULL == ethPort) {
						syslog_ax_dhcp_snp_err("no found port by eth global index.\n");
						ret = DHCP_SNP_RETURN_CODE_NO_SUCH_PORT;
					}
					else if (NULL != ethPort->funcs.func_data[ETH_PORT_FUNC_LINK_AGGREGATION]) {
						/*port is member of trunk*/
						ret = DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR;
					}
					else if (NPD_TRUE != npd_vlan_check_contain_port(vlanId, eth_g_index, &tagMode)) {
						/*check port membership*/
						ret = DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST;	
					}
					else {
						ret = dhcp_snp_set_opt82_port_circuitid(vlanId, eth_g_index, tagMode,
																	circuitid_type, circuitid_str);
						if (DHCP_SNP_RETURN_CODE_OK != ret) {
							syslog_ax_dhcp_snp_err("config DHCP-Snooping option82 Circuit-ID mode error, ret %x.\n", ret);
						}
					}
				}
				else {
					syslog_ax_dhcp_snp_err("slotno or portno is not legal\n");
					ret = DHCP_SNP_RETURN_CODE_NO_SUCH_PORT;
				}
			}
			else {
				syslog_ax_dhcp_snp_err("slotno or portno is not legal\n");
				ret = DHCP_SNP_RETURN_CODE_NO_SUCH_PORT;				
			}
		}
	}
	else {	
		syslog_ax_dhcp_snp_dbg("check DHCP-Snooping global status not enabled global.\n");
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
}

/**********************************************************************************
 * dhcp_snp_dbus_set_opt82_port_remoteid_content
 *		set remote-id type and content of  DHCP-Snooping option82 on special port
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK
 * 	 	DHCP_SNP_RETURN_CODE_ERROR
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET
 *
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_set_opt82_port_remoteid_content
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char slot_no = NPD_DHCP_SNP_INIT_0;
	unsigned char local_port_no = NPD_DHCP_SNP_INIT_0;
	unsigned short vlanId = NPD_DHCP_SNP_INIT_0;
	unsigned char remoteid_type = NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_INVALID;	
	char *remoteid_str = NULL;

	unsigned int eth_g_index = NPD_DHCP_SNP_INIT_0;
	unsigned char tagMode = NPD_DHCP_SNP_INIT_0;
	struct eth_port_s* ethPort = NULL;

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT16, &vlanId,
								DBUS_TYPE_BYTE, &slot_no,
								DBUS_TYPE_BYTE, &local_port_no,
								DBUS_TYPE_BYTE, &remoteid_type,
								DBUS_TYPE_STRING, &remoteid_str,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

	ret = dhcp_snp_check_global_status();
	if (ret == DHCP_SNP_RETURN_CODE_ENABLE_GBL)
	{
		ret = dhcp_snp_check_vlan_status(vlanId);
		if (DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST == ret) {
			syslog_ax_dhcp_snp_err("vlan %d not created.", vlanId);
		}
		else if (DHCP_SNP_RETURN_CODE_NOT_EN_VLAN == ret)
		{
			syslog_ax_dhcp_snp_err("config DHCP-Snooping option82 Remote-ID mode error, ret %x.\n", ret);
		}
		else if (DHCP_SNP_RETURN_CODE_EN_VLAN == ret)
		{
			syslog_ax_dhcp_snp_dbg("config DHCP-Snooping option82 Remote-ID mode %s on panel port %d/%d in vlan %d.\n",
									(remoteid_type == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC) ? "default" :
									((remoteid_type == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_USERSTR) ? "user-defined" : "unknow"),
									slot_no, local_port_no, vlanId);

			if (CHASSIS_SLOTNO_ISLEGAL(slot_no))
			{
				if (ETH_LOCAL_PORTNO_ISLEGAL(slot_no, local_port_no))
				{
					eth_g_index = ETH_GLOBAL_INDEX_FROM_SLOT_PORT_LOCAL_NO(slot_no, local_port_no);
					ethPort = npd_get_port_by_index(eth_g_index);
					if (NULL == ethPort) {
						syslog_ax_dhcp_snp_err("no found port by eth global index.\n");
						ret = DHCP_SNP_RETURN_CODE_NO_SUCH_PORT;
					}
					else if (NULL != ethPort->funcs.func_data[ETH_PORT_FUNC_LINK_AGGREGATION]) {
						/*port is member of trunk*/
						ret = DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR;
					}
					else if (NPD_TRUE != npd_vlan_check_contain_port(vlanId, eth_g_index, &tagMode)) {
						/*check port membership*/
						ret = DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST;	
					}
					else {
						ret = dhcp_snp_set_opt82_port_remoteid(vlanId, eth_g_index, tagMode,
																	remoteid_type, remoteid_str);
						if (DHCP_SNP_RETURN_CODE_OK != ret) {
							syslog_ax_dhcp_snp_err("config DHCP-Snooping option82 Remote-ID mode error, ret %x.\n", ret);
						}
					}
				}
				else {
					syslog_ax_dhcp_snp_err("slotno or portno is not legal\n");
					ret = DHCP_SNP_RETURN_CODE_NO_SUCH_PORT;
				}
			}
			else {
				syslog_ax_dhcp_snp_err("slotno or portno is not legal\n");
				ret = DHCP_SNP_RETURN_CODE_NO_SUCH_PORT;				
			}
		}
	}
	else {	
		syslog_ax_dhcp_snp_dbg("check DHCP-Snooping global status not enabled global.\n");
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
}

/**********************************************************************************
 * dhcp_snp_dbus_add_del_static_binding
 *		add/delete dhcp-snooping static-binding item to/from bind table
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK
 * 	 	DHCP_SNP_RETURN_CODE_ERROR
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST
 *		DHCP_SNP_RETURN_CODE_NO_SUCH_PORT
 *		DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST
 *
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_add_del_static_binding
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int opt_type = NPD_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE_INVALID;	
	unsigned long ip      = NPD_DHCP_SNP_INIT_0;
	unsigned char macAddr[NPD_DHCP_SNP_MAC_ADD_LEN] = {0};
	unsigned short vlanId = NPD_DHCP_SNP_INIT_0;
	unsigned char slot_no = NPD_DHCP_SNP_INIT_0;
	unsigned char local_port_no = NPD_DHCP_SNP_INIT_0;

	unsigned int eth_g_index = NPD_DHCP_SNP_INIT_0;
	unsigned char tagMode = NPD_DHCP_SNP_INIT_0;
	struct eth_port_s* ethPort = NULL;
	NPD_DHCP_SNP_TBL_ITEM_T item;
	
	unsigned int record_count = NPD_DHCP_SNP_INIT_0;
	NPD_DHCP_SNP_SHOW_ITEM_T show_items[MAX_ETHPORT_PER_BOARD * CHASSIS_SLOT_COUNT];
	char select_sql[1024] = {0};
	char *bufPtr = NULL;
	int loglength = NPD_DHCP_SNP_INIT_0;

	memset(&item, 0, sizeof(NPD_DHCP_SNP_TBL_ITEM_T));
	memset(show_items, 0, sizeof(NPD_DHCP_SNP_SHOW_ITEM_T) * MAX_ETHPORT_PER_BOARD * CHASSIS_SLOT_COUNT);

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT32, &opt_type,
								DBUS_TYPE_UINT32, &ip,
								DBUS_TYPE_BYTE, &macAddr[0],
								DBUS_TYPE_BYTE, &macAddr[1],
								DBUS_TYPE_BYTE, &macAddr[2],
								DBUS_TYPE_BYTE, &macAddr[3],
								DBUS_TYPE_BYTE, &macAddr[4],
								DBUS_TYPE_BYTE, &macAddr[5],
								DBUS_TYPE_UINT16, &vlanId,
								DBUS_TYPE_BYTE, &slot_no,
								DBUS_TYPE_BYTE, &local_port_no,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

	ret = dhcp_snp_check_global_status();
	if (ret == DHCP_SNP_RETURN_CODE_ENABLE_GBL)
	{
		/*********************************************************************
		 *	the vlan must exist,
		 *	not concerned about enabled/disabled DHCP Snooping on the vlan
		 *********************************************************************/
		ret = dhcp_snp_check_vlan_status(vlanId);
		if (DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST == ret) {
			syslog_ax_dhcp_snp_err("vlan %d not created.", vlanId);
		}
		else
		{	/* add/delete DHCP Snooping static binding item to/from binding table */
			syslog_ax_dhcp_snp_dbg("%s DHCP-Snooping static binding item %s binding table.",
									(opt_type == NPD_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE_ADD) ? "add" :
									((opt_type == NPD_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE_DEL) ? "delete" :
									"error operate type"),
									(opt_type == NPD_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE_ADD) ? "to" :
									((opt_type == NPD_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE_DEL) ? "from" :
									"error"));

			syslog_ax_dhcp_snp_dbg("IP %d.%d.%d.%d MAC %02x:%02x:%02x:%02x:%02x:%02x vlan %d panel port %d/%d.\n",
									(ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF,	ip & 0xFF,
									macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5],
									vlanId, slot_no, local_port_no);

			if (CHASSIS_SLOTNO_ISLEGAL(slot_no))
			{
				if (ETH_LOCAL_PORTNO_ISLEGAL(slot_no, local_port_no))
				{
					eth_g_index = ETH_GLOBAL_INDEX_FROM_SLOT_PORT_LOCAL_NO(slot_no, local_port_no);
					ethPort = npd_get_port_by_index(eth_g_index);
					if (NULL == ethPort) {
						syslog_ax_dhcp_snp_err("no found port by eth global index.\n");
						ret = DHCP_SNP_RETURN_CODE_NO_SUCH_PORT;
					}
					else if (NULL != ethPort->funcs.func_data[ETH_PORT_FUNC_LINK_AGGREGATION]) {
						/*port is member of trunk*/
						ret = DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR;
					}
					else if (NPD_TRUE != npd_vlan_check_contain_port(vlanId, eth_g_index, &tagMode)) {
						/*check port membership*/
						ret = DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST;	
					}
					else {
						/* step 1: fill DHCP Snooping table item */
						item.bind_type = NPD_DHCP_SNP_BIND_TYPE_STATIC;
						item.ip_addr = ip;
						memcpy(item.chaddr, macAddr, NPD_DHCP_SNP_MAC_ADD_LEN);
						item.vlanId = vlanId;
						item.ifindex = eth_g_index;
						item.sys_escape = 0;
						item.lease_time = 0;

						/* step 2: query db and check the mac had existed in db. */
						/*
						 ****************************
						 *cat query string 
						 ****************************
						 */
						bufPtr = select_sql;
					
						/*	"Type", "IP Address", "MAC Address", "Lease", "VLAN", "PORT" */
						loglength += sprintf(bufPtr, "SELECT BINDTYPE, IP, MAC, LEASE, VLAN, PORT FROM dhcpSnpDb WHERE MAC='");
						bufPtr = select_sql + loglength;
						loglength += sprintf(bufPtr, "%02x", item.chaddr[0]);
						bufPtr = select_sql + loglength;
						loglength += sprintf(bufPtr, "%02x", item.chaddr[1]);
						bufPtr = select_sql + loglength;
						loglength += sprintf(bufPtr, "%02x", item.chaddr[2]);
						bufPtr = select_sql + loglength;
						loglength += sprintf(bufPtr, "%02x", item.chaddr[3]);
						bufPtr = select_sql + loglength;
						loglength += sprintf(bufPtr, "%02x", item.chaddr[4]);
						bufPtr = select_sql + loglength;
						loglength += sprintf(bufPtr, "%02x", item.chaddr[5]);
						bufPtr = select_sql + loglength;
						loglength += sprintf(bufPtr, "';");
						bufPtr = select_sql + loglength;

						ret = dhcp_snp_tbl_query(select_sql, &record_count, show_items);
						if (DHCP_SNP_RETURN_CODE_OK != ret) {
							syslog_ax_dhcp_snp_err("get DHCP-Snooping bind table error, ret %x.\n", ret);
							ret = DHCP_SNP_RETURN_CODE_ERROR;
						}
						syslog_ax_dhcp_snp_dbg("query %d records from dhcpSnpDb success.\n", record_count);

						/* step 3: add/delete */
						if (NPD_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE_ADD == opt_type)
						{
							if (NPD_DHCP_SNP_INIT_0 != record_count) {
								/* the mac had existed in db, add fail */
								ret = DHCP_SNP_RETURN_CODE_ALREADY_SET;
							}else {
								ret = dhcp_snp_tbl_static_binding_insert(item);
								if (DHCP_SNP_RETURN_CODE_OK != ret) {
									syslog_ax_dhcp_snp_err("add static binding item to DHCP-Snooping DB error, ret %x.\n", ret);
									ret = DHCP_SNP_RETURN_CODE_ERROR;
								}
							}
						}
						else if (NPD_DHCP_SNP_STATIC_BINDING_OPERATE_TYPE_DEL == opt_type)
						{
							if (NPD_DHCP_SNP_INIT_0 == record_count) {
								/* the mac had not existed in db, delete fail */
								ret = DHCP_SNP_RETURN_CODE_ALREADY_SET;
							}else {
								ret = dhcp_snp_tbl_static_binding_delete(item);
								if (DHCP_SNP_RETURN_CODE_OK != ret) {
									syslog_ax_dhcp_snp_err("delete static binding item from DHCP-Snooping DB error, ret %x.\n", ret);
									ret = DHCP_SNP_RETURN_CODE_ERROR;
								}
							}
						}
						else {
							syslog_ax_dhcp_snp_err("add/delete dhcp snooping static binding item, error operate type %d.\n",
													opt_type);
							ret = DHCP_SNP_RETURN_CODE_ERROR;
						}
					}
				}
				else {
					syslog_ax_dhcp_snp_err("slotno or portno is not legal\n");
					ret = DHCP_SNP_RETURN_CODE_NO_SUCH_PORT;
				}
			}
			else {
				syslog_ax_dhcp_snp_err("slotno or portno is not legal\n");
				ret = DHCP_SNP_RETURN_CODE_NO_SUCH_PORT;				
			}
		}
	}
	else {	
		syslog_ax_dhcp_snp_dbg("check DHCP-Snooping global status not enabled global.\n");
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
}

/**********************************************************************************
 * dhcp_snp_dbus_add_binding
 *		add dhcp-snooping binding item to bind table
 *		inner interface in dhcp snooping, for save content of bindint table
 *		by command "write" before reboot
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK
 * 	 	DHCP_SNP_RETURN_CODE_ERROR
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST
 *		DHCP_SNP_RETURN_CODE_NO_SUCH_PORT
 *		DHCP_SNP_RETURN_CODE_PORT_TRUNK_MBR
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST
 *
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_add_binding
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char tagMode = NPD_DHCP_SNP_INIT_0;
	unsigned char macAddr[NPD_DHCP_SNP_MAC_ADD_LEN] = {0};
	unsigned long ip      = NPD_DHCP_SNP_INIT_0;
	unsigned short vlanId = NPD_DHCP_SNP_INIT_0;
	unsigned int eth_g_index = NPD_DHCP_SNP_INIT_0;
	unsigned int system_time = NPD_DHCP_SNP_INIT_0;
	unsigned int lease_time = NPD_DHCP_SNP_INIT_0;
	unsigned int bind_type = NPD_DHCP_SNP_BIND_TYPE_DYNAMIC;
	NPD_DHCP_SNP_TBL_ITEM_T item;
	
	memset(&item, 0, sizeof(NPD_DHCP_SNP_TBL_ITEM_T));

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_BYTE, &macAddr[0],
								DBUS_TYPE_BYTE, &macAddr[1],
								DBUS_TYPE_BYTE, &macAddr[2],
								DBUS_TYPE_BYTE, &macAddr[3],
								DBUS_TYPE_BYTE, &macAddr[4],
								DBUS_TYPE_BYTE, &macAddr[5],
								DBUS_TYPE_UINT32, &ip,
								DBUS_TYPE_UINT16, &vlanId,
								DBUS_TYPE_UINT32, &eth_g_index,
								DBUS_TYPE_UINT32, &system_time,
								DBUS_TYPE_UINT32, &lease_time,
								DBUS_TYPE_UINT32, &bind_type,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

	ret = dhcp_snp_check_global_status();
	if (ret == DHCP_SNP_RETURN_CODE_ENABLE_GBL)
	{
		/*********************************************************************
		 *	the vlan must exist,
		 *	not concerned about enabled/disabled DHCP Snooping on the vlan
		 *********************************************************************/
		ret = dhcp_snp_check_vlan_status(vlanId);
		if (DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST == ret) {
			syslog_ax_dhcp_snp_err("vlan %d not created.", vlanId);
		}
		else
		{	/* add DHCP Snooping dynamic binding item to binding table */
			syslog_ax_dhcp_snp_dbg("add DHCP-Snooping binding item to binding table.");

			syslog_ax_dhcp_snp_dbg("MAC %02x:%02x:%02x:%02x:%02x:%02x IP %d.%d.%d.%d",
									macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5],
									(ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);
			syslog_ax_dhcp_snp_dbg("vlan %d eth_g_index %d systime %d lease-time %d bind-type %d(0:dynamic 1:static)\n",
									vlanId, eth_g_index, system_time, lease_time, bind_type);

			if (NPD_TRUE != npd_vlan_check_contain_port(vlanId, eth_g_index, &tagMode)) {
				/*check port membership*/
				ret = DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST;	
			}
			else {
				/* step 1: fill DHCP Snooping table item */
				item.bind_type = bind_type;
				item.ip_addr = ip;
				memcpy(item.chaddr, macAddr, NPD_DHCP_SNP_MAC_ADD_LEN);
				item.vlanId = vlanId;
				item.ifindex = eth_g_index;
				item.sys_escape = system_time;
				item.lease_time = lease_time;

				/* step 2: add */
				ret = dhcp_snp_tbl_binding_insert(item);
				if (DHCP_SNP_RETURN_CODE_OK != ret) {
					syslog_ax_dhcp_snp_err("add dynamic binding item to DHCP-Snooping DB error, ret %x.\n", ret);
					ret = DHCP_SNP_RETURN_CODE_ERROR;
				}
			}
		}
	}
	else {
		syslog_ax_dhcp_snp_dbg("check DHCP-Snooping global status not enabled global.\n");
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
}
#endif

/**********************************************************************************
 * dhcp_snp_dbus_show_running_global_config
 *		DHCP Snooping show running global config
 *
 *	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		reply
 *	 	ret:
 *			DHCP_SNP_RETURN_CODE_OK				- success
 *			DHCP_SNP_RETURN_CODE_ERROR				- fail
 *
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_show_running_global_config
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter	 iter;
	unsigned char *showStr = NULL;
	unsigned char en_dis = 0;
	unsigned int length = 0;

	showStr = (unsigned char*)malloc(NPD_DHCP_SNP_RUNNING_CFG_MEM);
	if (NULL == showStr) {
		syslog_ax_dhcp_snp_err("DHCP Snooping show running global config, memory malloc error\n");
		return NULL;
	}
	memset(showStr, 0, NPD_DHCP_SNP_RUNNING_CFG_MEM);

	/**************************************** 
	  * save DHCP Snooping global config information
	  ***************************************/
	length = dhcp_snp_save_global_cfg(showStr, NPD_DHCP_SNP_RUNNING_CFG_MEM, &en_dis);
	syslog_ax_dhcp_snp_dbg("DHCP Snooping service %s\n", en_dis ? "enable" : "disable");
	if(NPD_DHCP_SNP_ENABLE == en_dis) {
		dhcp_snp_save_intf_cfg((showStr+length),(NPD_DHCP_SNP_RUNNING_CFG_MEM-length));
	}

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_STRING, &showStr);	

	free(showStr);
	showStr = NULL;

	return reply;
}


/**********************************************************************************
 * dhcp_snp_dbus_show_running_global_hansi_config
 *		DHCP Snooping show running global config
 *
 *	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		reply
 *	 	ret:
 *			DHCP_SNP_RETURN_CODE_OK				- success
 *			DHCP_SNP_RETURN_CODE_ERROR				- fail
 *
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_show_running_global_hansi_config
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;	  
	DBusMessageIter iter;
	DBusError err;   		
	unsigned char *showStr = NULL;
	unsigned int slot_id = 0;
	unsigned int vrrp_id = 0;
	unsigned int local_flag = 0;
	unsigned char status = NPD_DHCP_SNP_INIT_0;

	dbus_error_init(&err);

	if( !(dbus_message_get_args( msg ,&err,
					DBUS_TYPE_UINT32, &slot_id,
					DBUS_TYPE_UINT32, &vrrp_id,	
					DBUS_TYPE_UINT32, &local_flag,					
					DBUS_TYPE_INVALID))) {
		if(dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}

	showStr = (unsigned char*)malloc(NPD_DHCP_SNP_RUNNING_CFG_MEM);
	if (NULL == showStr) {
		syslog_ax_dhcp_snp_err("DHCP Snooping show running global config, memory malloc error\n");
		return NULL;
	}
	memset(showStr, 0, NPD_DHCP_SNP_RUNNING_CFG_MEM);



	/**************************************** 
	  * save DHCP Snooping global config information
	  ***************************************/
	dhcp_snp_get_global_status(&status);
	if (NPD_DHCP_SNP_ENABLE == status)
	{
		dhcp_snp_save_intf_hansi_cfg(showStr, NPD_DHCP_SNP_RUNNING_CFG_MEM, slot_id, vrrp_id, local_flag);
	}

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_STRING, &showStr);	


	free(showStr);
	showStr = NULL;

	return reply;
}


/**********************************************************************************
 * dhcp_snp_save_global_cfg
 *		get string of DHCP Snooping show running global config
 *
 *	INPUT:
 *		unsigned int bufLen
 *
 *	OUTPUT:
 *		unsigned char *buf
 *		unsigned char *enDis
 *
 *	RETURN:
 *		show string length
 *
 **********************************************************************************/
unsigned int dhcp_snp_save_global_cfg
(
	unsigned char *buf,
	unsigned int bufLen,
	unsigned char *enDis
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int length = NPD_DHCP_SNP_INIT_0;
	unsigned char status = NPD_DHCP_SNP_INIT_0;
	char *showStr = NULL;
	char *current = NULL;

	if (NULL == buf || NULL == enDis) {
		syslog_ax_dhcp_snp_err("DHCP Snooping show running global config, parameter is null error\n");
		return 0;
	}

	showStr = (char *)buf;
	current = showStr;

	if (DHCP_SNP_ARP_PROXY_ENABLE == dhcp_snp_arp_proxy_enable) {
		length += sprintf(current, "config dhcp-snooping arp-proxy enable\n");
		syslog_ax_dhcp_snp_dbg("%s\n", current);
		current = showStr + length;
	}
	ret = dhcp_snp_get_global_status(&status);
	if (NPD_DHCP_SNP_ENABLE == status)
	{
		*enDis = NPD_DHCP_SNP_ENABLE;
		if ((length + 30) < bufLen) {
			length += sprintf(current, "config dhcp-snooping wan enable\n");
			syslog_ax_dhcp_snp_dbg("%s\n", current);
			current = showStr + length;
		}

		if (NPD_DHCP_SNP_OPT82_ENABLE == g_dhcp_snp_opt82_enable) {
			if ((length + 42) < bufLen) {
				length += sprintf(current, "config dhcp-snooping information enable\n");
				syslog_ax_dhcp_snp_dbg("%s\n", current);
				current = showStr + length;
			}
		}

		if (NPD_DHCP_SNP_OPT82_FORMAT_TYPE_HEX != dhcp_snp_opt82_format_type) {
			if ((length + 48) < bufLen) {
				length += sprintf(current, "config dhcp-snooping information format ascii\n");
				syslog_ax_dhcp_snp_dbg("%s\n", current);
				current = showStr + length;
			}
		}

		if (NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_EXT != dhcp_snp_opt82_fill_format_type) {
			if ((length + 58) < bufLen) {
				length += sprintf(current, "config dhcp-snooping information packet-format standard\n");
				syslog_ax_dhcp_snp_dbg("%s\n", current);
				current = showStr + length;
			}
		}

		if (NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC != dhcp_snp_opt82_remoteid_type)
		{
			if (NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSNAME == dhcp_snp_opt82_remoteid_type)
			{
				if ((length + 53) < bufLen) {
					length += sprintf(current, "config dhcp-snooping information remote-id sysname\n");
					syslog_ax_dhcp_snp_dbg("%s\n", current);
					current = showStr + length;
				}
			}
			else if (NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_USERSTR == dhcp_snp_opt82_remoteid_type)
			{
				if ((length + 116) < bufLen) {
					length += sprintf(current, "config dhcp-snooping information remote-id string %s\n", dhcp_snp_opt82_remoteid_str);
					syslog_ax_dhcp_snp_dbg("%s\n", current);
					current = showStr + length;
				}
			}
		}

		if(DHCPSNP_NL_FALSE == dhcpsnp_ipneigh_on) {
			if ((length + 39) < bufLen) {
				length += sprintf(current, "set dhcp-snooping-wan static arp op disable\n");
				syslog_ax_dhcp_snp_dbg("%s\n", current);
				current = showStr + length;
			}
		}
	}
	else {
		*enDis = NPD_DHCP_SNP_DISABLE;
	}

	return length;
} 

#if 0
/**********************************************************************************
 * dhcp_snp_dbus_show_running_vlan_config
 *		DHCP Snooping show running vlan config
 *
 *	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		reply
 *	 	ret:
 *			DHCP_SNP_RETURN_CODE_OK				- success
 *			DHCP_SNP_RETURN_CODE_ERROR				- fail
 *
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_show_running_vlan_config
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter  iter;
	unsigned char *showStr = NULL;
	unsigned char en_dis = 0;

	showStr = (unsigned char*)malloc(NPD_DHCP_SNP_RUNNING_CFG_MEM);
	if (NULL == showStr) {
		syslog_ax_dhcp_snp_err("DHCP Snooping show running vlan config, memory malloc error\n");
		return NULL;
	}
	memset(showStr, 0, NPD_DHCP_SNP_RUNNING_CFG_MEM);

	/**************************************** 
	  * save DHCP Snooping vlan config information
	  ***************************************/
	dhcp_snp_save_vlan_cfg(showStr, NPD_DHCP_SNP_RUNNING_CFG_MEM, &en_dis);
	syslog_ax_dhcp_snp_dbg("DHCP Snooping service %s\n", en_dis ? "enable" : "disable");

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_STRING, &showStr);	

	free(showStr);
	showStr = NULL;

	return reply;
}

/**********************************************************************************
 * dhcp_snp_save_vlan_cfg
 *		get string of DHCP Snooping show running vlan config
 *
 *	INPUT:
 *		unsigned int bufLen
 *
 *	OUTPUT:
 *		unsigned char *buf
 *		unsigned char *enDis
 *
 *	RETURN:
 *		NULL
 *
 **********************************************************************************/
void dhcp_snp_save_vlan_cfg
(
	unsigned char *buf,
	unsigned int bufLen,
	unsigned char *enDis
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int i = NPD_DHCP_SNP_INIT_0;
	unsigned int length = NPD_DHCP_SNP_INIT_0;
	unsigned char status = NPD_DHCP_SNP_INIT_0;
	char *showStr = NULL;
	char *current = NULL;

	struct vlan_s* node = NULL;
	struct vlan_port_list_node_s *portNode = NULL;
	vlan_port_list_s* portList = NULL;
	struct list_head *pos = NULL;
	struct list_head *list = NULL;

	unsigned int  eth_g_index   = NPD_DHCP_SNP_INIT_0;
	unsigned char slot_index    = NPD_DHCP_SNP_INIT_0;
	unsigned int  slot_no       = NPD_DHCP_SNP_INIT_0;
	unsigned int  local_port_no = NPD_DHCP_SNP_INIT_0;

	if (NULL == buf || NULL == enDis) {
		syslog_ax_dhcp_snp_err("DHCP Snooping show running vlan config, parameter is null error\n");
		return ;
	}

	showStr = (char *)buf;
	current = showStr;

	ret = dhcp_snp_get_global_status(&status);
	if (NPD_DHCP_SNP_ENABLE == status)
	{
		*enDis = NPD_DHCP_SNP_ENABLE;
		for (i = 1; i < NPD_MAX_VLAN_ID; i++)
		{
			if (VLAN_RETURN_CODE_VLAN_EXISTS == npd_check_vlan_exist(i))
			{
				node = npd_find_vlan_by_vid(i);
				if (NULL != node)
				{
					if (NPD_DHCP_SNP_ENABLE == node->dhcpSnpEnDis)
					{
						if ((length + 19) < bufLen) {
							length += sprintf(current, "config vlan %d\n", i);
							syslog_ax_vlan_dbg("%s\n\n", current);
							current = showStr+ length;
						}
					
						if ((length + 30) < bufLen) {
							length += sprintf(current, " config dhcp-snooping enable\n");
							syslog_ax_dhcp_snp_dbg("%s\n", current);
							current = showStr + length;
						}

						/* untagged ports */
						portList = node->untagPortList;
						if (NULL != portList)
						{
							if (0 != portList->count)
							{
								list = &(portList->list);
								__list_for_each(pos, list)
								{
									portNode = list_entry(pos, struct vlan_port_list_node_s, list);
									if (portNode)
									{	/* init tmp variable */
										eth_g_index = NPD_DHCP_SNP_INIT_0;
										slot_index = NPD_DHCP_SNP_INIT_0;
										slot_no = NPD_DHCP_SNP_INIT_0;
										local_port_no = NPD_DHCP_SNP_INIT_0;

										eth_g_index = portNode->eth_global_index;
										slot_index = SLOT_INDEX_FROM_ETH_GLOBAL_INDEX(eth_g_index);
										slot_no = CHASSIS_SLOT_INDEX2NO(slot_index);
										local_port_no = ETH_LOCAL_INDEX_FROM_ETH_GLOBAL_INDEX(eth_g_index);
										local_port_no = ETH_LOCAL_INDEX2NO(slot_index,local_port_no);
										
										/* trust mode */
										if (NPD_DHCP_SNP_PORT_MODE_NOTRUST != portNode->dhcp_snp_info.trust_mode)
										{
											if (NPD_DHCP_SNP_PORT_MODE_TRUST == portNode->dhcp_snp_info.trust_mode)
											{
												if ((length + 35) < bufLen) {
													length += sprintf(current, " config dhcp-snooping %d/%d trust\n",
																	slot_no, local_port_no);
													syslog_ax_dhcp_snp_dbg("%s\n", current);
													current = showStr + length;
												}
											}else if (NPD_DHCP_SNP_PORT_MODE_NOBIND == portNode->dhcp_snp_info.trust_mode)
											{
												if ((length + 42) < bufLen) {
													length += sprintf(current, " config dhcp-snooping %d/%d trust-nobind\n",
																	slot_no, local_port_no);
													syslog_ax_dhcp_snp_dbg("%s\n", current);
													current = showStr + length;
												}
											}
										}

										/* option 82 strategy */
										if (NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_REPLACE != portNode->dhcp_snp_info.opt82_strategy)
										{
											if (NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_DROP == portNode->dhcp_snp_info.opt82_strategy)
											{
												if ((length + 55) < bufLen) {
													length += sprintf(current, " config dhcp-snooping information strategy %d/%d drop\n",
																	slot_no, local_port_no);
													syslog_ax_dhcp_snp_dbg("%s\n", current);
													current = showStr + length;
												}
											}else if (NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_KEEP == portNode->dhcp_snp_info.opt82_strategy)
											{
												if ((length + 55) < bufLen) {
													length += sprintf(current, " config dhcp-snooping information strategy %d/%d keep\n",
																	slot_no, local_port_no);
													syslog_ax_dhcp_snp_dbg("%s\n", current);
													current = showStr + length;
												}
											}
										}

										/* option 82 Circuit-ID */
										if (NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_DEFAULT != portNode->dhcp_snp_info.opt82_circuitid)
										{
											if ((length + 124) < bufLen) {
												length += sprintf(current, " config dhcp-snooping information circuit-id %d/%d string %s\n",
																slot_no, local_port_no, portNode->dhcp_snp_info.opt82_circuitid_str);
												syslog_ax_dhcp_snp_dbg("%s\n", current);
												current = showStr + length;
											}
										}

										/* option 82 Remote-ID */
										if (NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC != portNode->dhcp_snp_info.opt82_remoteid)
										{
											if ((length + 124) < bufLen) {
												length += sprintf(current, " config dhcp-snooping information remote-id %d/%d string %s\n",
																slot_no, local_port_no, portNode->dhcp_snp_info.opt82_remoteid_str);
												syslog_ax_dhcp_snp_dbg("%s\n", current);
												current = showStr + length;
											}
										}										
									}
								}
							}
						}
						
						/* tagged ports */
						portList = node->tagPortList;
						if (NULL != portList)
						{
							if (0 != portList->count)
							{
								list = &(portList->list);
								__list_for_each(pos, list)
								{
									portNode = list_entry(pos, struct vlan_port_list_node_s, list);
									if (portNode)
									{	/* init tmp variable */
										eth_g_index = NPD_DHCP_SNP_INIT_0;
										slot_index = NPD_DHCP_SNP_INIT_0;
										slot_no = NPD_DHCP_SNP_INIT_0;
										local_port_no = NPD_DHCP_SNP_INIT_0;

										eth_g_index = portNode->eth_global_index;
										slot_index = SLOT_INDEX_FROM_ETH_GLOBAL_INDEX(eth_g_index);
										slot_no = CHASSIS_SLOT_INDEX2NO(slot_index);
										local_port_no = ETH_LOCAL_INDEX_FROM_ETH_GLOBAL_INDEX(eth_g_index);
										local_port_no = ETH_LOCAL_INDEX2NO(slot_index,local_port_no);
										
										/* trust mode */
										if (NPD_DHCP_SNP_PORT_MODE_NOTRUST != portNode->dhcp_snp_info.trust_mode)
										{
											if (NPD_DHCP_SNP_PORT_MODE_TRUST == portNode->dhcp_snp_info.trust_mode)
											{
												if ((length + 35) < bufLen) {
													length += sprintf(current, " config dhcp-snooping %d/%d trust\n",
																	slot_no, local_port_no);
													syslog_ax_dhcp_snp_dbg("%s\n", current);
													current = showStr + length;
												}
											}else if (NPD_DHCP_SNP_PORT_MODE_NOBIND == portNode->dhcp_snp_info.trust_mode)
											{
												if ((length + 42) < bufLen) {
													length += sprintf(current, " config dhcp-snooping %d/%d trust-nobind\n",
																	slot_no, local_port_no);
													syslog_ax_dhcp_snp_dbg("%s\n", current);
													current = showStr + length;
												}
											}
										}

										/* option 82 strategy */
										if (NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_REPLACE != portNode->dhcp_snp_info.opt82_strategy)
										{
											if (NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_DROP == portNode->dhcp_snp_info.opt82_strategy)
											{
												if ((length + 55) < bufLen) {
													length += sprintf(current, " config dhcp-snooping information strategy %d/%d drop\n",
																	slot_no, local_port_no);
													syslog_ax_dhcp_snp_dbg("%s\n", current);
													current = showStr + length;
												}
											}else if (NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_KEEP == portNode->dhcp_snp_info.opt82_strategy)
											{
												if ((length + 55) < bufLen) {
													length += sprintf(current, " config dhcp-snooping information strategy %d/%d keep\n",
																	slot_no, local_port_no);
													syslog_ax_dhcp_snp_dbg("%s\n", current);
													current = showStr + length;
												}
											}
										}

										/* option 82 Circuit-ID */
										if (NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_DEFAULT != portNode->dhcp_snp_info.opt82_circuitid)
										{
											if ((length + 124) < bufLen) {
												length += sprintf(current, " config dhcp-snooping information circuit-id %d/%d string %s\n",
																slot_no, local_port_no, portNode->dhcp_snp_info.opt82_circuitid_str);
												syslog_ax_dhcp_snp_dbg("%s\n", current);
												current = showStr + length;
											}
										}

										/* option 82 Remote-ID */
										if (NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC != portNode->dhcp_snp_info.opt82_remoteid)
										{
											if ((length + 124) < bufLen) {
												length += sprintf(current, " config dhcp-snooping information remote-id %d/%d string %s\n",
																slot_no, local_port_no, portNode->dhcp_snp_info.opt82_remoteid_str);
												syslog_ax_dhcp_snp_dbg("%s\n", current);
												current = showStr + length;
											}
										}										
									}
								}
							}
						}

						if ((length + 6) < bufLen) {
							length += sprintf(current, " exit\n");
							syslog_ax_dhcp_snp_dbg("%s", current);
							current = showStr + length;
						}
					}
				}
			}
		}
	}
	else {
		*enDis = NPD_DHCP_SNP_DISABLE;
	}

	return ;
} 

/**********************************************************************************
 * dhcp_snp_dbus_show_running_save_bind_table
 *		DHCP Snooping show running save bind table
 *
 *	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		reply
 *	 	ret:
 *			DHCP_SNP_RETURN_CODE_OK				- success
 *			DHCP_SNP_RETURN_CODE_ERROR			- fail
 *
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_show_running_save_bind_table
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter  iter;
	unsigned char *showStr = NULL;
	unsigned char en_dis = 0;

	showStr = (unsigned char*)malloc(NPD_DHCP_SNP_RUNNING_CFG_MEM);
	if (NULL == showStr) {
		syslog_ax_dhcp_snp_err("DHCP Snooping show running vlan config, memory malloc error\n");
		return NULL;
	}
	memset(showStr, 0, NPD_DHCP_SNP_RUNNING_CFG_MEM);

	/******************************************** 
	  * save DHCP Snooping bind table information
	  *******************************************/
	dhcp_snp_save_bind_table(showStr, NPD_DHCP_SNP_RUNNING_CFG_MEM, &en_dis);
	syslog_ax_dhcp_snp_dbg("DHCP Snooping service %s\n", en_dis ? "enable" : "disable");

	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_STRING, &showStr);	

	free(showStr);
	showStr = NULL;

	return reply;
}

/**********************************************************************************
 * dhcp_snp_save_bind_table
 *		get string of DHCP Snooping show running save bind table
 *
 *	INPUT:
 *		unsigned int bufLen
 *
 *	OUTPUT:
 *		unsigned char *buf
 *		unsigned char *enDis
 *
 *	RETURN:
 *		NULL
 *
 **********************************************************************************/
void dhcp_snp_save_bind_table
(
	unsigned char *buf,
	unsigned int bufLen,
	unsigned char *enDis
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int i = NPD_DHCP_SNP_INIT_0;
	unsigned int length = NPD_DHCP_SNP_INIT_0;
	unsigned char status = NPD_DHCP_SNP_INIT_0;
	char *showStr = NULL;
	char *current = NULL;

	unsigned char macAddr_str[NPD_DHCP_SNP_MAC_ADD_LEN] = {0};
	unsigned int ip      = NPD_DHCP_SNP_INIT_0;
	unsigned short vlanId = NPD_DHCP_SNP_INIT_0;
	unsigned int eth_g_index = NPD_DHCP_SNP_INIT_0;
	unsigned int system_time = NPD_DHCP_SNP_INIT_0;
	unsigned int lease_time = NPD_DHCP_SNP_INIT_0;
	unsigned int bind_type = NPD_DHCP_SNP_BIND_TYPE_DYNAMIC;

	unsigned int record_count = NPD_DHCP_SNP_INIT_0;
	NPD_DHCP_SNP_SHOW_ITEM_T show_items[MAX_ETHPORT_PER_BOARD * CHASSIS_SLOT_COUNT];
	
	/*	"Type", "IP Address", "MAC Address", "Lease", "VLAN", "PORT" */
	static char *select_all_sql = "SELECT BINDTYPE, IP, MAC, LEASE, VLAN, PORT FROM dhcpSnpDb;";

	if (NULL == buf || NULL == enDis) {
		syslog_ax_dhcp_snp_err("DHCP Snooping show running vlan config, parameter is null error\n");
		return ;
	}

	memset(show_items, 0, sizeof(NPD_DHCP_SNP_SHOW_ITEM_T) * MAX_ETHPORT_PER_BOARD * CHASSIS_SLOT_COUNT);

	showStr = (char *)buf;
	current = showStr;

	ret = dhcp_snp_get_global_status(&status);
	if (NPD_DHCP_SNP_ENABLE == status)
	{
		*enDis = NPD_DHCP_SNP_ENABLE;

		/* step 1: query all items in DB. */
		ret = dhcp_snp_tbl_query(select_all_sql, &record_count, show_items);
		if (DHCP_SNP_RETURN_CODE_OK != ret) {
			syslog_ax_dhcp_snp_err("get DHCP-Snooping bind table error, ret %x.\n", ret);
			ret = DHCP_SNP_RETURN_CODE_ERROR;
		}
		
		syslog_ax_dhcp_snp_dbg("query %d records from dhcpSnpDb success.\n", record_count);

		/* step 2: cat config string */
		for (i = 0; i < record_count; i++)
		{
			memset(macAddr_str, 0, NPD_DHCP_SNP_MAC_ADD_LEN);
			ip = NPD_DHCP_SNP_INIT_0;
			vlanId = NPD_DHCP_SNP_INIT_0;
			eth_g_index = NPD_DHCP_SNP_INIT_0;
			system_time = NPD_DHCP_SNP_INIT_0;
			lease_time = NPD_DHCP_SNP_INIT_0;
			bind_type = NPD_DHCP_SNP_BIND_TYPE_DYNAMIC;

			dhcp_snp_mac_ascii_to_hex(show_items[i].chaddr, macAddr_str, NPD_DHCP_SNP_MAC_ADD_LEN);
			ip = show_items[i].ip_addr;
			vlanId = show_items[i].vlanId;
			eth_g_index = show_items[i].ifindex;
			system_time = time(0);
			lease_time = show_items[i].lease_time;
			bind_type = show_items[i].bind_type;

			/* mac */
			if ((length + 55) < bufLen) {
				length += sprintf(current, "dhcp-snooping binding add %02x:%02x:%02x:%02x:%02x:%02x",
							macAddr_str[0], macAddr_str[1],
							macAddr_str[2], macAddr_str[3],
							macAddr_str[4], macAddr_str[5]);
				syslog_ax_vlan_dbg("%s\n", current);
				current = showStr+ length;
			}

			/* ip */
			if ((length + 16) < bufLen) {
				length += sprintf(current, " %d.%d.%d.%d",
						(ip>>24) & 0xff, (ip>>16) & 0xff, (ip>>8) & 0xff, ip & 0xff);
				syslog_ax_vlan_dbg("%s\n", current);
				current = showStr + length;
			}

			/* vlanid and eth_g_index */
			if ((length + 10) < bufLen) {
				length += sprintf(current, " %d %d ", vlanId, eth_g_index);
				syslog_ax_vlan_dbg("%s\n", current);
				current = showStr + length;
			}

			/* system_time and lease_time */
			if ((length + 10) < bufLen) {
				length += sprintf(current, " %d %d ", system_time, lease_time);
				syslog_ax_vlan_dbg("%s\n", current);
				current = showStr + length;
			}
			
			/* bind_type */
			if ((length + 2) < bufLen) {
				length += sprintf(current, " %d\n", bind_type);
				syslog_ax_vlan_dbg("%s\n", current);
				current = showStr + length;
			}
		}
	}
	else {
		*enDis = NPD_DHCP_SNP_DISABLE;
	}

	return ;
} 
#endif
/**********************************************************************************
 * dhcp_snp_dbus_config_intf
 *		config DHCP-Snooping enable status on interface
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL		- not enable global
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET			- have set trust or no-bind port
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_config_intf
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK, ifindex = ~0UI;
	int status = 0, sockFd = -1;
	char ifname[IF_NAMESIZE] = {0};
	char *name = NULL;

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_STRING, &name,
								DBUS_TYPE_INT32, &status,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

	strncpy(ifname, name, sizeof(ifname)-1);
	ret = dhcp_snp_check_global_status();
	if (ret == DHCP_SNP_RETURN_CODE_ENABLE_GBL)
	{
		if(if_nametoindex(ifname)) {
			ret = dhcp_snp_listener_query((unsigned char*)ifname);
			if(DHCP_SNP_ENABLE == status ) {/* enable */
				if (DHCP_SNP_RETURN_CODE_OK == ret) {
					syslog_ax_dhcp_snp_err("interface %s already enabled dhcp snooping!\n", ifname);
					ret = DHCP_SNP_RETURN_CODE_EN_INTF;
				}
				else if (DHCP_SNP_RETURN_CODE_NOT_FOUND == ret)
				{
					ret = dhcp_snp_listener_open(ifname);
					syslog_ax_dhcp_snp_dbg("enable dhcp snooping on interface %s %s!\n",  \
										ifname, (DHCP_SNP_RETURN_CODE_OK == ret) ? "ok":"fail");
					if ((DHCP_SNP_ARP_PROXY_ENABLE == dhcp_snp_arp_proxy_enable)) {
						dhcp_snp_set_udf_flag(ifname, DHCP_SNP_SET_UDF_FLAG);
					}
				}
			}
			else /* disable */
			{
				if(DHCP_SNP_RETURN_CODE_NOT_FOUND == ret) {
					syslog_ax_dhcp_snp_err("interface %s not enabled dhcp snooping!\n", ifname);
					ret = DHCP_SNP_RETURN_CODE_NOT_EN_INTF;
				}
				else if(DHCP_SNP_RETURN_CODE_OK == ret) {
					if ((DHCP_SNP_ARP_PROXY_ENABLE == dhcp_snp_arp_proxy_enable)) {
						dhcp_snp_set_udf_flag(ifname, DHCP_SNP_CLR_UDF_FLAG);
					}
					ret = dhcp_snp_listener_close(ifname);
					syslog_ax_dhcp_snp_dbg("disable dhcp snooping on interface %s %s!\n",  \
										ifname, (DHCP_SNP_RETURN_CODE_OK == ret) ? "ok":"fail");
					
				}
			}
		}
		else {
			syslog_ax_dhcp_snp_err("no interface found with index %d!\n", ifindex);
			ret = DHCP_SNP_RETURN_CODE_NO_SUCH_INTF;
		}
	}
	else {	
		syslog_ax_dhcp_snp_warn("dhcp snooping global status not enabled.\n");
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
} 

/**********************************************************************************
 * dhcp_snp_dbus_config_intf_add_router
 *		config DHCP-Snooping enable status on interface
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL		- not enable global
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET			- have set trust or no-bind port
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_config_intf_add_router
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK, ifindex = ~0UI;
	int status = 0, sockFd = -1;
	char ifname[IF_NAMESIZE] = {0};
	char *name = NULL;
	int add_router_type  = -1;

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_STRING, &name,
								DBUS_TYPE_INT32, &status,
								DBUS_TYPE_INT32, &add_router_type,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

	strncpy(ifname, name, sizeof(ifname)-1);
	
	if(0 == add_router_type) { /* anti ARP spoofing working on dhcp snooping interface */
		ret = dhcp_snp_check_global_status();
		if (ret == DHCP_SNP_RETURN_CODE_ENABLE_GBL)
		{
			if(if_nametoindex(ifname)) {
				ret = dhcp_snp_listener_query((unsigned char*)ifname);
				if(DHCP_SNP_ENABLE == status ) {/* enable */
					if (DHCP_SNP_RETURN_CODE_OK == ret) {
						//ret = dhcp_snp_set_intf_add_router(NPD_DHCP_SNP_ENABLE, ifname);
						ret = dhcp_snp_listener_handle_add_router(NPD_DHCP_SNP_ENABLE, ifname);
					}
					else if (DHCP_SNP_RETURN_CODE_NOT_FOUND == ret)
					{
						syslog_ax_dhcp_snp_err("enable dhcp snooping on interface %s %s!\n",  \
											ifname, (DHCP_SNP_RETURN_CODE_OK == ret) ? "ok":"fail");
						ret = DHCP_SNP_RETURN_CODE_NOT_EN_INTF;
					}
				}
				else /* disable */
				{
					if(DHCP_SNP_RETURN_CODE_NOT_FOUND == ret) {
						syslog_ax_dhcp_snp_err("enable dhcp snooping on interface %s %s!\n",  \
											ifname, (DHCP_SNP_RETURN_CODE_OK == ret) ? "ok":"fail");
						ret = DHCP_SNP_RETURN_CODE_NOT_EN_INTF;
					}
					else if(DHCP_SNP_RETURN_CODE_OK == ret) {
						//ret = dhcp_snp_set_intf_add_router(NPD_DHCP_SNP_DISABLE, ifname);	
						ret = dhcp_snp_listener_handle_add_router(NPD_DHCP_SNP_DISABLE, ifname);
					}
				}
			}
			else {
				syslog_ax_dhcp_snp_err("no interface found with index %d!\n", ifindex);
				ret = DHCP_SNP_RETURN_CODE_NO_SUCH_INTF;
			}
		}
		else {	
			syslog_ax_dhcp_snp_warn("dhcp snooping global status not enabled.\n");
		}
	}
	else if(1 == add_router_type) { /* anti ARP spoofing working on normal interface */
		if(if_indextoname(ifindex, ifname)) {
			if(DHCP_SNP_ENABLE == status ) {/* enable */
				syslog_ax_dhcp_snp_dbg("enable add-route if %d name %s\n", ifindex, ifname);
				ret = dhcp_snp_handle_intf_ebtables(ifname, DHCP_SNP_ENABLE);
			}
			else /* disable */
			{
				syslog_ax_dhcp_snp_dbg("disable add-route if %d name %s\n", ifindex, ifname);
				ret = dhcp_snp_handle_intf_ebtables(ifname, DHCP_SNP_DISABLE);	
			}
		}
		else {
			syslog_ax_dhcp_snp_err("no interface found with index %d!\n", ifindex);
			ret = DHCP_SNP_RETURN_CODE_NO_SUCH_INTF;
		}
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
} 

/**********************************************************************************
 * dhcp_snp_dbus_config_intf
 *		config DHCP-Snooping enable status on interface
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_NOT_ENABLE_GBL		- not enable global
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET			- have set trust or no-bind port
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_config_intf_arp
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK, ifindex = ~0UI;
	int status = 0, sockFd = -1;
	char ifname[IF_NAMESIZE] = {0};
	char *name = NULL;
	int anti_arp_type  = -1;

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_STRING, &name,
								DBUS_TYPE_INT32, &status,
								DBUS_TYPE_INT32, &anti_arp_type,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

	strncpy(ifname, name, sizeof(ifname)-1);
	
	if(0 == anti_arp_type) { /* anti ARP spoofing working on dhcp snooping interface */
		ret = dhcp_snp_check_global_status();
		if (ret == DHCP_SNP_RETURN_CODE_ENABLE_GBL)
		{
			if(if_nametoindex(ifname)) {
				ret = dhcp_snp_listener_query((unsigned char*)ifname);
				if(DHCP_SNP_ENABLE == status ) {/* enable */
					if (DHCP_SNP_RETURN_CODE_OK == ret) {
						ret = dhcp_snp_set_intf_arp(NPD_DHCP_SNP_ENABLE, ifname);
					}
					else if (DHCP_SNP_RETURN_CODE_NOT_FOUND == ret)
					{
						syslog_ax_dhcp_snp_err("enable dhcp snooping on interface %s %s!\n",  \
											ifname, (DHCP_SNP_RETURN_CODE_OK == ret) ? "ok":"fail");
						ret = DHCP_SNP_RETURN_CODE_NOT_EN_INTF;
					}
				}
				else /* disable */
				{
					if(DHCP_SNP_RETURN_CODE_NOT_FOUND == ret) {
						syslog_ax_dhcp_snp_err("enable dhcp snooping on interface %s %s!\n",  \
											ifname, (DHCP_SNP_RETURN_CODE_OK == ret) ? "ok":"fail");
						ret = DHCP_SNP_RETURN_CODE_NOT_EN_INTF;
					}
					else if(DHCP_SNP_RETURN_CODE_OK == ret) {
						ret = dhcp_snp_set_intf_arp(NPD_DHCP_SNP_DISABLE, ifname);	
					}
				}
			}
			else {
				syslog_ax_dhcp_snp_err("no interface found with index %d!\n", ifindex);
				ret = DHCP_SNP_RETURN_CODE_NO_SUCH_INTF;
			}
		}
		else {	
			syslog_ax_dhcp_snp_warn("dhcp snooping global status not enabled.\n");
		}
	}
	else if(1 == anti_arp_type) { /* anti ARP spoofing working on normal interface */
		if(if_indextoname(ifindex, ifname)) {
			if(DHCP_SNP_ENABLE == status ) {/* enable */
				syslog_ax_dhcp_snp_dbg("enable anti arp spoof if %d name %s\n", ifindex, ifname);
				ret = dhcp_snp_handle_intf_ebtables(ifname, DHCP_SNP_ENABLE);
			}
			else /* disable */
			{
				syslog_ax_dhcp_snp_dbg("disable anti arp spoof if %d name %s\n", ifindex, ifname);
				ret = dhcp_snp_handle_intf_ebtables(ifname, DHCP_SNP_DISABLE);	
			}
		}
		else {
			syslog_ax_dhcp_snp_err("no interface found with index %d!\n", ifindex);
			ret = DHCP_SNP_RETURN_CODE_NO_SUCH_INTF;
		}
	}

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
} 
int parse_int_ve(char* str, unsigned int* slotid, unsigned int *vlanid, char *cid, unsigned int *port){
	char c;
	char *tmp = NULL;
	char *endptr = NULL;
	c = str[0];
	if (c>='0'&&c<='9'){
		*slotid= strtoul(str,&endptr,10);
		
		if(endptr[0] == '.'){
			tmp = endptr+1;
			*vlanid= strtoul(tmp,&endptr,10);//regard the ve interface name as right
			if((endptr[0] != '\0')&&(endptr[0] != '\n'))
				return -1;
			return 1;
		}
		else if((endptr[0] == 'f')||(endptr[0] == 's')){
			*cid = endptr[0];
			tmp = endptr+1;
			*port = strtoul(tmp,&endptr,10);
			if(endptr[0] == '.'){
				tmp = endptr+1;
				*vlanid= strtoul(tmp,&endptr,10);
				if((endptr[0] != '\0')&&(endptr[0] != '\n'))
					return -1;
				return 2;
			}
			else if((endptr[0] == '\0')||(endptr[0] == '\n'))
				return 2;
			
			return -1;
			}
	}
	
		return -1;
}
	

int check_ve_interface(char *ifname ,char *name){
	int sockfd;
	unsigned int slotid = 0;
	unsigned int vlanid = 0;
	unsigned int port = 0;
	char cpu = 'f';
	//char *cpu_id = &cpu;
	struct ifreq ifr;
	int ret = 0;
	
	if (0 != strncmp(ifname, "ve", 2)) {
		log_debug("It's not ve interface\n");
		sprintf(name,"%s",ifname);
		return 0;
	}
	else{
		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if(sockfd<0){
			log_debug("sockfd failed!\n");
			return -1;
		}
		strncpy(ifr.ifr_name,ifname, sizeof(ifr.ifr_name)); 	
		ret = parse_int_ve(ifname+2,&slotid,&vlanid,&cpu,&port);
		
		if(1 == ret)
		{
			log_debug("slotid = %d\n",slotid);
			log_debug("vlanid = %d\n",vlanid);

			if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
				log_debug("SIOCGIFINDEX error\n");

				//convert to new ve name
				if(slotid < 10)
					sprintf(name,"ve0%df1.%d",slotid,vlanid);
				else if(slotid >= 10)
					sprintf(name,"ve%df1.%d",slotid,vlanid);
				log_debug("ve name is %s\n",name);

				memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
				strncpy(ifr.ifr_name,name, sizeof(ifr.ifr_name));		
				if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
					log_debug("SIOCGIFINDEX error\n");		
					close(sockfd);
					return -1;	//the new ve interface doesn't exist
				}	
				else{
					close(sockfd);
				    return 0;	//the new ve interface exists
				}
			}
			else{
				sprintf(name,"%s",ifname);
				close(sockfd);
				return 0;//the old ve interface exists
			}
		}
		else if(2 == ret)
		{
			log_debug("slotid = %d\n",slotid);
			log_debug("vlanid=%d\n",vlanid);
			log_debug("ifname=%s\n",ifname);
			if(vlanid == 0){
				if(slotid < 10)
					sprintf(name,"ve0%d%c%d",slotid,cpu,port);
				else if(slotid >= 10)
					sprintf(name,"ve%d%c%d",slotid,cpu,port);
			}
			else if(vlanid > 0){
				if(slotid < 10)
					sprintf(name,"ve0%d%c%d.%d",slotid,cpu,port,vlanid);
				else if(slotid >= 10)
					sprintf(name,"ve%d%c%d.%d",slotid,cpu,port,vlanid);
			}
			
			log_debug("name=%s\n",name);
			memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
			strncpy(ifr.ifr_name,name, sizeof(ifr.ifr_name));
			if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1){//bind to a interface 
				log_debug("SIOCGIFINDEX error\n");
				close(sockfd);
				return -1;	//the new ve interface doesn't exist
			}		
			else{
				//sprintf(name,"%s",ifname);
			    close(sockfd);
			    return 0;	//the new ve interface exists
		 	}
		}
		else{
			log_debug("the ve name is wrong\n");
			sprintf(name,"%s",ifname);
			close(sockfd);
			return -1;
		}
	}
}



/**********************************************************************************
 * dhcp_snp_dbus_config_debug_level
 *		config DHCP-Snooping debug level
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_ERROR				- fail
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_config_debug_level
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK, enable = 0, flag = 0;

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT32, &enable,
								DBUS_TYPE_UINT32, &flag,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

	ret = dhcp_snp_log_config_debug_level(flag, enable);

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
} 
/**********************************************************************************
 * dhcp_snp_dbus_check_snp_interface_ve
 *		
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_ERROR				- fail
 **********************************************************************************/
DBusMessage 
*dhcp_snp_dbus_check_snp_interface_ve
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter;
	DBusError		err;
	unsigned int	ret = 1;
	char *ifname = NULL;
	char *name=NULL;

	dbus_error_init( &err );
	if( !(dbus_message_get_args( msg ,&err,
					DBUS_TYPE_STRING, &ifname,
					DBUS_TYPE_INVALID))) {
		if(dbus_error_is_set( &err )) {
			dbus_error_free( &err );
		}
		return NULL;
	}
	
	
	name=malloc(IF_NAMESIZE);
	if(!name){
		syslog_ax_dhcp_snp_err("application of name fails ");
		return NULL;	
	}
	memset(name,0,IF_NAMESIZE);
	ret = check_ve_interface(ifname,name);

	reply = dbus_message_new_method_return(msg);
	if (NULL == reply) {
		if(name){
			free(name);
			name=NULL;
		}
		return reply;
	}
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_STRING, &name);
	if(name){
		free(name);
		name=NULL;
	}
	return reply;
} 

/**********************************************************************************
 * dhcp_snp_dbus_config_ipneigh
 *		config DHCP-Snooping ip neigh operation flag
 *
 * 	INPUT:
 *		DBusConnection *conn, 
 *		DBusMessage *msg, 
 *		void *user_data
 *
 *	OUTPUT:
 * 		NULL
 *
 *	RETURN:
 * 	 	reply
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_ERROR				- fail
 **********************************************************************************/
DBusMessage *dhcp_snp_dbus_config_ipneigh
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char enable = 0;

	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_BYTE, &enable,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}

	ret = dhcp_snp_netlink_ipneigh_ctrl(enable);

	reply = dbus_message_new_method_return(msg);
	
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);

	return reply;
} 

/**********************************************************************************
 * dhcp_snp_save_intf_cfg
 *		get string of interface DHCP Snooping show running config
 *
 *	INPUT:
 *		buf - show running string buffer
 *		bufLen - buffer length
 *
 *	OUTPUT:
 *		unsigned char *buf
 *		unsigned char *enDis
 *
 *	RETURN:
 *		NULL
 *
 **********************************************************************************/
void dhcp_snp_save_intf_cfg
(
	unsigned char *buf,
	unsigned int bufLen
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int length = NPD_DHCP_SNP_INIT_0;
	unsigned char status = NPD_DHCP_SNP_INIT_0;
	char *showStr = NULL;
	char *current = NULL;

	if (NULL == buf) {
		syslog_ax_dhcp_snp_err("DHCP Snooping show running global config, parameter is null error\n");
		return ;
	}

	dhcp_snp_listener_save_cfg(buf, bufLen);
	
	return ;
} 


/**********************************************************************************
 * dhcp_snp_save_intf_hansi_cfg
 *		get string of interface DHCP Snooping show running config
 *
 *	INPUT:
 *		buf - show running string buffer
 *		bufLen - buffer length
 *
 *	OUTPUT:
 *		unsigned char *buf
 *		unsigned char *enDis
 *
 *	RETURN:
 *		NULL
 *
 **********************************************************************************/
void dhcp_snp_save_intf_hansi_cfg
(
	unsigned char *buf,
	unsigned int bufLen,
	int slotid,
	int vrrpid,
	int local_flag	
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int length = NPD_DHCP_SNP_INIT_0;
	unsigned char status = NPD_DHCP_SNP_INIT_0;
	char *showStr = NULL;
	char *current = NULL;

	if (NULL == buf) {
		syslog_ax_dhcp_snp_err("DHCP Snooping show running global config, parameter is null error\n");
		return ;
	}

	dhcp_snp_listener_save_hansi_cfg(buf, bufLen, slotid, vrrpid, local_flag);
	
	return ;
} 

DBusMessage *dhcp_snp_dbus_show_wan_bind_table
(
	DBusConnection *conn,
	DBusMessage *msg,
	void *user_data
) 
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusMessageIter iter_array;
	DBusError err;
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int i = NPD_DHCP_SNP_INIT_0;
	unsigned char tmp_caddr[NPD_DHCP_SNP_MAC_ADD_LEN] =  {0};
	unsigned int record_count = NPD_DHCP_SNP_INIT_0;
	NPD_DHCP_SNP_TBL_ITEM_T *show_items = NULL;
	char ifname[IF_NAMESIZE]={0};
	char *ifname_ep=ifname;
	dbus_error_init(&err);
	ret = dhcp_snp_check_global_status();
	if (ret != DHCP_SNP_RETURN_CODE_ENABLE_GBL) {
		syslog_ax_dhcp_snp_dbg("check DHCP-Snooping global status not enabled global.\n");
	}

	pthread_mutex_lock(&mutexDhcpsnptbl);
	record_count = dhcp_snp_tbl_mac_hash_foreach();
	pthread_mutex_unlock(&mutexDhcpsnptbl);		

	show_items = (NPD_DHCP_SNP_TBL_ITEM_T *)malloc(sizeof(NPD_DHCP_SNP_TBL_ITEM_T) * record_count);
	if (!show_items) {
		syslog_ax_dhcp_snp_err("show dhcp-snooping bind-table: malloc show items failed\n");
		return NULL;
	}
	
	pthread_mutex_lock(&mutexDhcpsnptbl);
	dhcp_snp_copy_bind_table(show_items, record_count);	
	pthread_mutex_unlock(&mutexDhcpsnptbl);
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append (reply, &iter);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, &ret);
	dbus_message_iter_append_basic (&iter,
									 DBUS_TYPE_UINT32, &record_count);
	dbus_message_iter_open_container(&iter,
								   DBUS_TYPE_ARRAY,
								      DBUS_STRUCT_BEGIN_CHAR_AS_STRING  /*begin*/
   											DBUS_TYPE_UINT32_AS_STRING /*bind_type*/
											DBUS_TYPE_BYTE_AS_STRING /*bind state*/ 
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[0]*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[1]*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[2]*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[3]*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[4]*/
											DBUS_TYPE_BYTE_AS_STRING /*chaddr[5]*/
										   	DBUS_TYPE_UINT16_AS_STRING /*vlanId*/
										   	DBUS_TYPE_UINT32_AS_STRING /*ip_addr*/
										   	DBUS_TYPE_UINT32_AS_STRING /*lease_time	*/									   	
										   	DBUS_TYPE_STRING_AS_STRING /*ifname*/
    								  DBUS_STRUCT_END_CHAR_AS_STRING, /*end*/
								   &iter_array);
	for (i = 0; i < record_count; i++) {
		if(!if_indextoname(show_items[i].ifindex, ifname)) {
			syslog_ax_dhcp_snp_err("no intf found as idx %d netlink error !\n", \
									show_items[i].ifindex);
			free(show_items);
			show_items=NULL;
			return NULL;
		}
		//memset(tmp_caddr, 0, 6);
		DBusMessageIter iter_struct;
		dbus_message_iter_open_container(&iter_array,
										DBUS_TYPE_STRUCT,
										NULL,
										&iter_struct);
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32, &(show_items[i].bind_type));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE, &(show_items[i].state));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE, &(show_items[i].chaddr[0]));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE, &(show_items[i].chaddr[1]));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE, &(show_items[i].chaddr[2]));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE, &(show_items[i].chaddr[3]));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE, &(show_items[i].chaddr[4]));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_BYTE, &(show_items[i].chaddr[5]));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT16, &(show_items[i].vlanId));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32, &(show_items[i].ip_addr));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_UINT32, &(show_items[i].lease_time));
		dbus_message_iter_append_basic(&iter_struct,
										DBUS_TYPE_STRING, &ifname_ep);
		dbus_message_iter_close_container(&iter_array, &iter_struct);
		memset(ifname,0,IF_NAMESIZE);	
	}
	dbus_message_iter_close_container(&iter, &iter_array);
	free(show_items);
	show_items=NULL;
	return reply;
}
DBusMessage *dhcp_snp_dbus_del_host_router
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	NPD_DHCP_SNP_USER_ITEM_T user;
	NPD_DHCP_SNP_TBL_ITEM_T *item = NULL;
	unsigned int record_count = NPD_DHCP_SNP_INIT_0;
    NPD_DHCP_SNP_TBL_ITEM_T *show_items = NULL;
	unsigned int i = NPD_DHCP_SNP_INIT_0;
	unsigned int ip  = NPD_DHCP_SNP_INIT_0;
	char ifname[IF_NAMESIZE]={0};
	char *name = NULL;
//	char command[128] = {0};
	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
							DBUS_TYPE_UINT32, &ip,
							DBUS_TYPE_STRING, &name,
							DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}
	ret = dhcp_snp_check_global_status();
	if (DHCP_SNP_RETURN_CODE_ENABLE_GBL == ret) {
		pthread_mutex_lock(&mutexDhcpsnptbl);
		record_count = dhcp_snp_tbl_mac_hash_foreach();
		pthread_mutex_unlock(&mutexDhcpsnptbl);		

		show_items = (NPD_DHCP_SNP_TBL_ITEM_T *)malloc(sizeof(NPD_DHCP_SNP_TBL_ITEM_T) * record_count);
		if (!show_items) {
			syslog_ax_dhcp_snp_err("show dhcp-snooping bind-table: malloc show_items failed\n");
			return NULL;
		}
		
		pthread_mutex_lock(&mutexDhcpsnptbl);
		dhcp_snp_copy_bind_table(show_items, record_count);	
		pthread_mutex_unlock(&mutexDhcpsnptbl);
		for (i = 0; i < record_count; i++){
			if(!if_indextoname(show_items[i].ifindex, ifname)) {
				syslog_ax_dhcp_snp_err("no intf found as idx %d netlink error !\n", \
										show_items[i].ifindex);
				free(show_items);
				show_items=NULL;
				return NULL;
			}
			if(0 != strlen(name) && ip && (show_items[i].ip_addr == ip) && (!strncmp(name,ifname,IF_NAMESIZE))){
				dhcp_snp_netlink_add_static_route(DHCPSNP_RTNL_STATIC_ROUTE_DEL_E,  \
													show_items[i].ifindex, show_items[i].ip_addr);
				ret = DHCP_SNP_RETURN_CODE_FOUND;
				break;
			}else if(!ip && ((0 == strlen(name)) || (!strncmp(name,ifname,IF_NAMESIZE)))){
			//sprintf(command,"sudo route del -host %u.%u.%u.%u dev %s",((show_items[i].ip_addr)>>24)&0xff,\
			//	((show_items[i].ip_addr)>>16)&0xff,((show_items[i].ip_addr)>>8)&0xff,((show_items[i].ip_addr)>>0)&0xff,ifname);
			//system(command);
				dhcp_snp_netlink_add_static_route(DHCPSNP_RTNL_STATIC_ROUTE_DEL_E,  \
													show_items[i].ifindex, show_items[i].ip_addr);
				
				ret = DHCP_SNP_RETURN_CODE_FOUND;
			}
			memset(ifname,0,IF_NAMESIZE);
			//memset(command,0,128);
		}	
	    if(DHCP_SNP_RETURN_CODE_ENABLE_GBL == ret){
			ret = DHCP_SNP_RETURN_CODE_NOT_FOUND;
	} 
	}
	
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);
	free(show_items);
	show_items=NULL;
	return reply;
}

DBusMessage *dhcp_snp_dbus_del_wan_bind_table
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	NPD_DHCP_SNP_USER_ITEM_T user;
	NPD_DHCP_SNP_TBL_ITEM_T *item = NULL;
	user.ifindex=0;
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_BYTE, &user.chaddr[0],
								DBUS_TYPE_BYTE, &user.chaddr[1],
								DBUS_TYPE_BYTE, &user.chaddr[2],
								DBUS_TYPE_BYTE, &user.chaddr[3],
								DBUS_TYPE_BYTE, &user.chaddr[4],
								DBUS_TYPE_BYTE, &user.chaddr[5],
								DBUS_TYPE_INVALID))) {
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}
	ret = dhcp_snp_check_global_status();
	if (DHCP_SNP_RETURN_CODE_ENABLE_GBL == ret) {
		if (item = dhcp_snp_tbl_item_find(&user)) {
			dhcp_snp_del_static_bindtable(item->ip_addr, item->chaddr, item->ifindex);
			if (item->ip_addr) {
				dhcp_snp_tbl_item_delete_iphash(item);
			}
			ret = dhcp_snp_tbl_item_delete(item);
		} else {
			syslog_ax_dhcp_snp_err("delete failed, not find item by mac %02x:%02x:%02x:%02x:%02x:%02x\n", 
										user.chaddr[0], user.chaddr[1], user.chaddr[2], 
										user.chaddr[3], user.chaddr[4], user.chaddr[5]);
			ret = DHCP_SNP_RETURN_CODE_NOT_FOUND;
		}	
	} 
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);
	return reply;
}
DBusMessage *dhcp_snp_dbus_add_wan_bind_table
(
	DBusConnection *conn,
	DBusMessage *msg, 
	void *user_data
)
{
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	NPD_DHCP_SNP_USER_ITEM_T user;
	NPD_DHCP_SNP_TBL_ITEM_T *item = NULL;
	char *name = NULL;
	memset(&user, 0, sizeof(user));
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT32, &user.ip_addr,
								DBUS_TYPE_BYTE, &user.chaddr[0],
								DBUS_TYPE_BYTE, &user.chaddr[1],
								DBUS_TYPE_BYTE, &user.chaddr[2],
								DBUS_TYPE_BYTE, &user.chaddr[3],
								DBUS_TYPE_BYTE, &user.chaddr[4],
								DBUS_TYPE_BYTE, &user.chaddr[5],
								DBUS_TYPE_STRING, &name,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}
	user.ifindex = if_nametoindex(name);
	user.bind_type = NPD_DHCP_SNP_BIND_TYPE_STATIC;
	user.state = NPD_DHCP_SNP_BIND_STATE_BOUND;
	ret = dhcp_snp_check_global_status();
	if (ret == DHCP_SNP_RETURN_CODE_ENABLE_GBL) {
		if ((dhcp_snp_tbl_item_find(&user)) 
			|| (dhcp_snp_tbl_item_find_by_ip(user.ip_addr))) {
			ret = DHCP_SNP_RETURN_CODE_ALREADY_SET;
		} else {
			if (item = dhcp_snp_tbl_item_insert(&user)) {
				dhcp_snp_tbl_item_insert_ip_hash(item);
				dhcp_snp_add_static_bindtable(user.ip_addr, user.chaddr, user.ifindex);
				ret = DHCP_SNP_RETURN_CODE_OK;
			} else {
				ret= DHCP_SNP_RETURN_CODE_ERROR;
			}		
		}
	} else {	
		syslog_ax_dhcp_snp_dbg("check DHCP-Snooping global status not enabled global.\n");
	}
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);
	return reply;
}
#if 0
DBusMessage *dhcp_snp_dbus_config_ageing_time
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int ageing_time = NPD_DHCP_SNP_INIT_0;	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT32, &ageing_time,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}
	dhcp_snp_ageing_time = ageing_time;
	syslog_ax_dhcp_snp_dbg("set dhcp snooping ageing time %d\n", dhcp_snp_ageing_time);
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);
	return reply;
} 
#endif
DBusMessage *dhcp_snp_dbus_enable_arp_proxy
(
	DBusConnection *conn, 
	DBusMessage *msg, 
	void *user_data
)
{	
	DBusMessage* reply = NULL;
	DBusMessageIter iter;
	DBusError err;
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int isEnable = NPD_DHCP_SNP_INIT_0;	
	dbus_error_init(&err);
	if (!(dbus_message_get_args(msg, &err,
								DBUS_TYPE_UINT32, &isEnable,
								DBUS_TYPE_INVALID)))
	{
		syslog_ax_dhcp_snp_err("Unable to get input args ");
		if (dbus_error_is_set(&err)) {
			 syslog_ax_dhcp_snp_err("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		syslog_ax_dhcp_snp_err("return error caused dbus.\n");
		return NULL;
	}
	dhcp_snp_arp_proxy_enable = isEnable;
	syslog_ax_dhcp_snp_dbg("DHCP-Snooping ARP Proxy is %s.\n", 
					(DHCP_SNP_ARP_PROXY_ENABLE == isEnable) ? "enable" : "disable");
	reply = dbus_message_new_method_return(msg);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_append_basic(&iter,
									DBUS_TYPE_UINT32, &ret);
	return reply;
} 
#ifdef __cplusplus
}
#endif



