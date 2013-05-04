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
* dhcp_snp_options.c
*
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		dhcp options process for DHCP snooping module.
*
* DATE:
*		04/16/2010	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.1.1.1 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************
*	head files														*
**********************************************************/
#include <sys/types.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <linux/if_ether.h>
#include <linux/filter.h>

#include <ctype.h>
#include <dbus/dbus.h>
		
#include "util/npd_list.h"
#include "sysdef/npd_sysdef.h"
#include "dbus/npd/npd_dbus_def.h"
#include "npd/nam/npd_amapi.h"
#include "sysdef/returncode.h"
#include "npd/nbm/npd_bmapi.h"
#include "dhcp_snp_log.h"

#include "dhcp_snp_com.h"
#include "dhcp_snp_tbl.h"
#include "dhcp_snp_pkt.h"

#include "dhcp_snp_options.h"

/*********************************************************
*	global variable define											*
**********************************************************/


/*********************************************************
*	extern variable												*
**********************************************************/
extern unsigned char g_dhcp_snp_opt82_enable;
extern unsigned char dhcp_snp_opt82_format_type;
extern unsigned char dhcp_snp_opt82_fill_format_type;
extern unsigned char dhcp_snp_opt82_remoteid_type;
extern unsigned char dhcp_snp_opt82_remoteid_str[NPD_DHCP_SNP_REMOTEID_STR_LEN];

/*********************************************************
*	functions define												*
**********************************************************/

/**********************************************************************************
 * dhcp_snp_opt82_enable
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
unsigned int dhcp_snp_opt82_enable
(
	void
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;

	ret = dhcp_snp_set_opt82_status(NPD_DHCP_SNP_OPT82_ENABLE);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("set DHCP-Snooping option82 status enable error, ret %x", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * dhcp_snp_opt82_disable
 *		set DHCP_Snooping enable option82 status, and init 
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
unsigned int dhcp_snp_opt82_disable
(
	void
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;

	ret = dhcp_snp_set_opt82_status(NPD_DHCP_SNP_OPT82_DISABLE);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("set DHCP-Snooping option82 status disable error, ret %x", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * dhcp_snp_check_opt82_status
 *		check DHCP_Snooping enable/disable option82 status
 *
 *	INPUT:
 *		NULL
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_EN_OPT82			- option82 status is enable
 *	 	DHCP_SNP_RETURN_CODE_NOT_EN_OPT82		- option82 status is disable
 **********************************************************************************/
unsigned int dhcp_snp_check_opt82_status
(
	void
)
{
	
	if (g_dhcp_snp_opt82_enable == NPD_DHCP_SNP_OPT82_ENABLE) {
		return DHCP_SNP_RETURN_CODE_EN_OPT82;
	}else {
		return DHCP_SNP_RETURN_CODE_NOT_EN_OPT82;
	}
}

/**********************************************************************************
 * dhcp_snp_get_opt82_status
 *		get DHCP_Snooping enable/disable option82 status
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
unsigned int dhcp_snp_get_opt82_status
(
	unsigned char *status
)
{
	if (!status) {
		syslog_ax_dhcp_snp_err("get DHCP-Snooping option82 status error, parameters is null.\n");
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	
	*status = g_dhcp_snp_opt82_enable;
	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * dhcp_snp_set_opt82_status
 *		set DHCP_Snooping enable/disable option82 status
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
unsigned int dhcp_snp_set_opt82_status
(
	unsigned char isEnable
)
{
	g_dhcp_snp_opt82_enable = isEnable ?
								NPD_DHCP_SNP_OPT82_ENABLE : NPD_DHCP_SNP_OPT82_DISABLE;

	return DHCP_SNP_RETURN_CODE_OK;
}

void dhcp_snp_print_port_func_data
(
	unsigned int g_eth_index,
	NPD_DHCP_SNP_PORT_T *dhcp_snp_info
)
{
	syslog_ax_dhcp_snp_dbg("==============dhcp snp func data===============\n");
	syslog_ax_dhcp_snp_dbg("eth-port %d\n", g_eth_index);
	syslog_ax_dhcp_snp_dbg("trust mode %s\n",
							(dhcp_snp_info->trust_mode == NPD_DHCP_SNP_PORT_MODE_TRUST) ? "trust" :
							((dhcp_snp_info->trust_mode == NPD_DHCP_SNP_PORT_MODE_NOBIND) ? "nobind" :
							((dhcp_snp_info->trust_mode == NPD_DHCP_SNP_PORT_MODE_NOTRUST) ? "notrust" :
							"unknow")));

	syslog_ax_dhcp_snp_dbg("strategy mode %s\n",
							(dhcp_snp_info->opt82_strategy == NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_DROP) ? "drop" :
							((dhcp_snp_info->opt82_strategy  == NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_KEEP) ? "keep" :
							((dhcp_snp_info->opt82_strategy  == NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_REPLACE) ? "replace" :
							"unknow")));

	syslog_ax_dhcp_snp_dbg("circuit-id mode %s\n",
							(dhcp_snp_info->opt82_circuitid == NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_DEFAULT) ? "default" :
							((dhcp_snp_info->opt82_circuitid == NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_USERSTR) ? "user-defined" :
							"unknow"));
	syslog_ax_dhcp_snp_dbg("           content [%s]\n",
							dhcp_snp_info->opt82_circuitid_str);

	syslog_ax_dhcp_snp_dbg("remote-id mode %s\n",
							(dhcp_snp_info->opt82_remoteid == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC) ? "default" :
							((dhcp_snp_info->opt82_remoteid == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_USERSTR) ? "user-defined" :
							((dhcp_snp_info->opt82_remoteid == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSNAME) ? "sysname" :
							"unknow")));
	syslog_ax_dhcp_snp_dbg("           content [%s]\n",
							dhcp_snp_info->opt82_remoteid_str);
	
	syslog_ax_dhcp_snp_dbg("===============================================\n");

	return ;
}

/**********************************************************************************
 * dhcp_snp_set_opt82_port_strategy
 *		set strategy of DHCP_Snooping option82 on special vlan's port
 *
 *	INPUT:
 *		unsigned short vlanId,
 *		unsigned int g_eth_index,
 *		unsigned char tagMode,
 *		unsigned char strategy_mode
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET			- have set strategy mode
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST		- vlan not exist
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 *
 **********************************************************************************/
unsigned int dhcp_snp_set_opt82_port_strategy
(
	unsigned short vlanId,
	unsigned int g_eth_index,
	unsigned char tagMode,
	unsigned char strategy_mode
)
{
	struct vlan_s *vlanNode = NULL;
	struct vlan_port_list_node_s *node = NULL;
	struct list_head *list = NULL;
	struct list_head *pos  = NULL;
	vlan_port_list_s *portList = NULL;

	syslog_ax_dhcp_snp_dbg("set vlan %d eth-port %d tag mode %s option82 strategy %d mode %s\n",
							vlanId, g_eth_index,
							tagMode ? "tagged" : "untagged",  
							strategy_mode,
							(strategy_mode == NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_DROP) ? "drop" :
							((strategy_mode == NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_KEEP) ? "keep" :
							((strategy_mode == NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_REPLACE) ? "replace" :
							"unknow")));
	#ifdef NO_DHCPSNP_STANDALONE
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
				/* check port startegy mode */
				if (node->dhcp_snp_info.opt82_strategy == strategy_mode)
				{
					syslog_ax_dhcp_snp_err("vlanId %d eth-port %d option82 strategy mode setted %s.\n",
											vlanId, g_eth_index,
											(strategy_mode == NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_DROP) ? "drop" :
											((strategy_mode == NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_KEEP) ? "keep" :
											((strategy_mode == NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_REPLACE) ? "replace" :
											"unknow")));

					return DHCP_SNP_RETURN_CODE_ALREADY_SET;
				}

				node->dhcp_snp_info.opt82_strategy = strategy_mode;
				syslog_ax_dhcp_snp_dbg("set vlan %d eth-port %d tag mode %s option82 strategy mode %s success.\n",
										vlanId, g_eth_index,
										tagMode ? "tagged" : "untagged",  
										(strategy_mode == NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_DROP) ? "drop" :
										((strategy_mode == NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_KEEP) ? "keep" :
										((strategy_mode == NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_REPLACE) ? "replace" :
										"unknow")));

				return DHCP_SNP_RETURN_CODE_OK;
			}
		}
	}
	#endif
	return DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST;
}

/**********************************************************************************
 * dhcp_snp_get_opt82_port_strategy
 *		get strategy of DHCP_Snooping option82 on special vlan's port
 *
 *	INPUT:
 *		unsigned short vlanId,
 *		unsigned int g_eth_index,
 *		unsigned char isTagged,
 *
 *	OUTPUT:
 *		unsigned int *strategy_mode
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST		- vlan is not exist
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL			- parameter is null
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 *
 **********************************************************************************/
unsigned int dhcp_snp_get_opt82_port_strategy
(
	unsigned short vlanId,
	unsigned int g_eth_index,
	unsigned char isTagged,
	unsigned char *strategy_mode
)
{
	struct vlan_s *vlanNode = NULL;
	struct vlan_port_list_node_s *node = NULL;
	struct list_head *list = NULL;
	struct list_head *pos	= NULL;
	vlan_port_list_s *portList = NULL;

	if (!strategy_mode) {
		syslog_ax_dhcp_snp_err("get port strategy mode error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	syslog_ax_dhcp_snp_dbg("get vlan %d eth-port %d tag mode %s option82 strategy mode.\n",
							vlanId, g_eth_index,
							isTagged ? "tagged" : "untagged");

	#ifdef NO_DHCPSNP_STANDALONE
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
			node = list_entry(pos, struct vlan_port_list_node_s, list);
			if (NULL != node && g_eth_index == node->eth_global_index)
			{
				/* get port strategy mode */
				*strategy_mode = node->dhcp_snp_info.opt82_strategy;
				syslog_ax_dhcp_snp_dbg("get vlan %d eth-port %d tag mode %s option82 strategy mode %s success.\n",
										vlanId, g_eth_index,
										isTagged ? "tagged" : "untagged",  
										(*strategy_mode == NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_DROP) ? "drop" :
										((*strategy_mode == NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_KEEP) ? "keep" :
										((*strategy_mode == NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_REPLACE) ? "replace" :
										"unknow")));

				return DHCP_SNP_RETURN_CODE_OK;
			}
		}
	}
	#endif
	
	return DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST;
}

/**********************************************************************************
 * dhcp_snp_set_opt82_port_circuitid
 *		set circuitid of DHCP_Snooping option82  on special vlan's port
 *
 *	INPUT:
 *		unsigned short vlanId,
 *		unsigned int g_eth_index,
 *		unsigned char tagMode,
 *		unsigned char circuitid_mode,
 *		char *circuitid_content
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_ALREADY_SET			- have set circuit-id mode
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST		- vlan not exist
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL			- parameter is null
 *
 **********************************************************************************/
unsigned int dhcp_snp_set_opt82_port_circuitid
(
	unsigned short vlanId,
	unsigned int g_eth_index,
	unsigned char tagMode,
	unsigned char circuitid_mode,
	char *circuitid_content
)
{
	struct vlan_s *vlanNode = NULL;
	struct vlan_port_list_node_s *node = NULL;
	struct list_head *list = NULL;
	struct list_head *pos  = NULL;
	vlan_port_list_s *portList = NULL;

	if (!circuitid_content) {
		syslog_ax_dhcp_snp_err("set option 82 circuit-id error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	syslog_ax_dhcp_snp_dbg("set vlan %d eth-port %d tag mode %s option82 Circuit-ID mode %s\n",
							vlanId, g_eth_index,
							tagMode ? "tagged" : "untagged",  
							(circuitid_mode == NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_DEFAULT) ? "default" :
							((circuitid_mode == NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_USERSTR) ? "user-defined" :
							"unknow"));

	#ifdef NO_DHCPSNP_STANDALONE
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
				/* check port circuit-id mode */
				if ((node->dhcp_snp_info.opt82_circuitid == circuitid_mode) &&
					(node->dhcp_snp_info.opt82_circuitid == NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_DEFAULT))
				{
					syslog_ax_dhcp_snp_err("vlanId %d eth-port %d option82 Circuit-ID mode have setted default.\n",
											vlanId, g_eth_index);
					return DHCP_SNP_RETURN_CODE_ALREADY_SET;
				}

				node->dhcp_snp_info.opt82_circuitid = circuitid_mode;

				if (circuitid_mode == NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_DEFAULT) {
					/* clear the circuit-id str */
					memset(node->dhcp_snp_info.opt82_circuitid_str, 0, NPD_DHCP_SNP_CIRCUITID_STR_LEN);
				}else {
					memset(node->dhcp_snp_info.opt82_circuitid_str, 0, NPD_DHCP_SNP_CIRCUITID_STR_LEN);
					memcpy(node->dhcp_snp_info.opt82_circuitid_str, circuitid_content, strlen(circuitid_content));
				}

				syslog_ax_dhcp_snp_dbg("set vlan %d eth-port %d tag mode %s option82 Circuit-ID mode %s content [%s] success.\n",
					vlanId, g_eth_index,
					tagMode ? "tagged" : "untagged",  
					(node->dhcp_snp_info.opt82_circuitid == NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_DEFAULT) ? "default" :
					((node->dhcp_snp_info.opt82_circuitid == NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_USERSTR) ? "user-defined" :
					"unknow"),
					circuitid_content);

				return DHCP_SNP_RETURN_CODE_OK;
			}
		}
	}
	#endif
	return DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST;
}

/**********************************************************************************
 * dhcp_snp_get_opt82_port_circuitid
 *		set circuitid of DHCP_Snooping option82 on special port
 *
 *	INPUT:
 *		unsigned int g_eth_index,
 *		unsigned char tagMode,
 *		unsigned short vlanid,
 *
 *	OUTPUT:
 *		unsigned int *circuitid_mode,
 *		char *circuitid_content
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK					- success
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL			- parameter is null
 *		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST		- vlan is not exist
 *		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 *
 **********************************************************************************/
unsigned int dhcp_snp_get_opt82_port_circuitid
(
	unsigned int g_eth_index,
	unsigned char tagMode,
	unsigned short vlanid,
	unsigned char *circuitid_mode,
	char *circuitid_content
)
{
	unsigned int get_flag = NPD_DHCP_SNP_INIT_0;
	unsigned short tmp_port_index = g_eth_index;
	unsigned int tmp_circuitid_mode = NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_INVALID;
	char tmp_circuitid_content[NPD_DHCP_SNP_CIRCUITID_STR_LEN + 4] = {0};
	unsigned int content_len = NPD_DHCP_SNP_INIT_0;
	
	NPD_DHCP_OPT82_SUBOPT_T subopt;
	NPD_DHCP_OPT82_CIRCUITID_T circuitid;
	NPD_DHCP_OPT82_CIRCUITID_DATA_T circuitid_data;
	NPD_DHCP_OPT82_COM_DATA_T com_data;
	NPD_DHCP_SNP_PORT_T *dhcp_snp_info = NULL;

	struct vlan_s *vlanNode = NULL;
	struct vlan_port_list_node_s *node = NULL;
	struct list_head *list = NULL;
	struct list_head *pos  = NULL;
	vlan_port_list_s *portList = NULL;

	if (!circuitid_content || !circuitid_mode) {
		syslog_ax_dhcp_snp_err("get option 82 circuit-id error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	syslog_ax_dhcp_snp_dbg("get vlan %d eth-port %d tag mode %s option82 Circuit-ID mode\n",
							vlanid, g_eth_index,
							tagMode ? "tagged" : "untagged");

	memset(&subopt, 0, sizeof(NPD_DHCP_OPT82_SUBOPT_T));
	memset(&circuitid, 0, sizeof(NPD_DHCP_OPT82_CIRCUITID_T));
	memset(&circuitid_data, 0, sizeof(NPD_DHCP_OPT82_CIRCUITID_DATA_T));
	memset(&com_data, 0, sizeof(NPD_DHCP_OPT82_COM_DATA_T));

	#ifdef NO_DHCPSNP_STANDALONE
	vlanNode = npd_find_vlan_by_vid(vlanid);
	if (NULL == vlanNode) {
		syslog_ax_dhcp_snp_err("vlan %d is not created.\n", vlanid);
		return DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST;
	}

	/* tagged port list*/
	if (NPD_TRUE == tagMode) {
		portList = vlanNode->tagPortList;
	}else if (NPD_FALSE == tagMode) {
		portList = vlanNode->untagPortList;
	}

	if (NULL == portList) {
		syslog_ax_dhcp_snp_err("eth-port %d is not member of the vlan %d.\n", g_eth_index, vlanid);
		return DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST;
	}
	else if(0 != portList->count)
	{
		syslog_ax_dhcp_snp_dbg("vlan %d %s port count %d\n",
					vlanid, tagMode ? "tagged" : "untagged", portList->count);
		
		list = &(portList->list);
		__list_for_each(pos,list) {
			node = list_entry(pos, struct vlan_port_list_node_s, list);
			if (NULL != node && g_eth_index == node->eth_global_index)
			{
				/* get port circuit-id mode */
				dhcp_snp_info = &(node->dhcp_snp_info);
				syslog_ax_dhcp_snp_dbg("get vlan %d eth-port %d tag mode %s option82 Circuit-ID mode %s content [%s] success.\n",
					vlanid, g_eth_index,
					tagMode ? "tagged" : "untagged",  
					(dhcp_snp_info->opt82_circuitid == NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_DEFAULT) ? "default" :
					((dhcp_snp_info->opt82_circuitid == NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_USERSTR) ? "user-defined" :
					"unknow"),
					dhcp_snp_info->opt82_circuitid_str);

				get_flag = 1;/* found */
				break;
			}
		}
	}

	if (1 != get_flag) {
		syslog_ax_dhcp_snp_err("eth-port %d is not member of the vlan %d.\n", g_eth_index, vlanid);
		return DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST;
	}

	dhcp_snp_print_port_func_data(g_eth_index, dhcp_snp_info);

	/*
	           0                      7                     15                   23                  31
	            --------------------------------------------------------
	           | suboption_type | suboption_len  | circuit-id_type| circuit-id_len |
	            --------------------------------------------------------
	    data |                 vlanid                     |           port-index                |
	            --------------------------------------------------------
	    note:
	        1. if fill format is standard, field of  circuit-id_type and circuit-id_len are miss
	        2. if data is default, length is 2 + 2
	            if data is user-defined, length is 64
	*/
	/* get port Circuit-ID mode and Circuit-ID content */
	tmp_circuitid_mode = dhcp_snp_info->opt82_circuitid;
	if (NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_USERSTR == tmp_circuitid_mode)
	{
		/* use-defined use ascii format */
		circuitid.circuit_type = 0x1;
		circuitid.circuit_len = NPD_DHCP_SNP_CIRCUITID_STR_LEN;
		memcpy(com_data.data, dhcp_snp_info->opt82_circuitid_str, NPD_DHCP_SNP_CIRCUITID_STR_LEN);

		if (NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_EXT == dhcp_snp_opt82_fill_format_type)
		{	/* add field of circuit-id_type and circuit-id_len */
			memcpy(tmp_circuitid_content + 2, &circuitid, sizeof(NPD_DHCP_OPT82_CIRCUITID_T));
			content_len += sizeof(NPD_DHCP_OPT82_CIRCUITID_T);

		}else if (NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_STD == dhcp_snp_opt82_fill_format_type)
		{	/* not need add field of circuit-id_type and circuit-id_len */
			syslog_ax_dhcp_snp_dbg("option82 fill format is standard.\n");		
		}
		
		/* add user-defined string */
		memcpy(tmp_circuitid_content + 2 + content_len, com_data.data, NPD_DHCP_SNP_CIRCUITID_STR_LEN);
		content_len += sizeof(NPD_DHCP_OPT82_COM_DATA_T);

		subopt.subopt_type = DHCP_CIRCUIT_ID;
		subopt.subopt_len = content_len;
		memcpy(tmp_circuitid_content, &subopt, sizeof(NPD_DHCP_OPT82_SUBOPT_T));
	}
	else if (NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_DEFAULT == tmp_circuitid_mode)
	{
		/* default use hex format */
		circuitid.circuit_type = 0x0;
		circuitid.circuit_len = 0x4;

		if (NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_EXT == dhcp_snp_opt82_fill_format_type)
		{	/* add field of circuit-id_type and circuit-id_len */
			memcpy(tmp_circuitid_content + 2, &circuitid, sizeof(NPD_DHCP_OPT82_CIRCUITID_T));
			content_len += sizeof(NPD_DHCP_OPT82_CIRCUITID_T);

		}
		else if (NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_STD == dhcp_snp_opt82_fill_format_type)
		{	/* not need add field of circuit-id_type and circuit-id_len */
			syslog_ax_dhcp_snp_dbg("option82 fill format is standard.\n");
		}

		if (NPD_DHCP_SNP_OPT82_FORMAT_TYPE_HEX == dhcp_snp_opt82_format_type)
		{
			/* add default data use in hex */
			circuitid_data.vlanid = htons(vlanid);
			circuitid_data.portindex = htons(tmp_port_index);
			memcpy(tmp_circuitid_content + 2 + content_len, &circuitid_data, sizeof(NPD_DHCP_OPT82_CIRCUITID_DATA_T));
			content_len += sizeof(NPD_DHCP_OPT82_CIRCUITID_DATA_T);
		}
		else if (NPD_DHCP_SNP_OPT82_FORMAT_TYPE_ASCII == dhcp_snp_opt82_format_type)
		{
			char vlan_str[10] = {0};
			char port_str[10] = {0};
			sprintf(vlan_str, "%02d", vlanid);
			sprintf(port_str, "%02d", tmp_port_index);
			#if 0
			itoa(vlanid, vlan_str, 10);
			itoa(tmp_port_index, port_str, 10);
			#endif
			memcpy(com_data.data, vlan_str, strlen(vlan_str));
			memcpy(com_data.data + strlen(vlan_str), port_str, strlen(port_str));

			memcpy(tmp_circuitid_content + 2 + content_len, com_data.data, strlen((char *)com_data.data));
			content_len += strlen((char *)com_data.data);

			/* modify circuit-id len, becase the len is not 4 byte at this time */
			if (NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_EXT == dhcp_snp_opt82_fill_format_type)
			{
				NPD_DHCP_OPT82_CIRCUITID_T *tmp_circuitid = NULL;
				tmp_circuitid = (NPD_DHCP_OPT82_CIRCUITID_T *)(tmp_circuitid_content + 2);
				tmp_circuitid->circuit_type = 0x1; 
				tmp_circuitid->circuit_len = strlen((char *)com_data.data);
			}
		}

		subopt.subopt_type = DHCP_CIRCUIT_ID;
		subopt.subopt_len = content_len;
		memcpy(tmp_circuitid_content, &subopt, sizeof(NPD_DHCP_OPT82_SUBOPT_T));
	}

	*circuitid_mode = tmp_circuitid_mode;
	memcpy(circuitid_content, tmp_circuitid_content, content_len + 2);

	syslog_ax_dhcp_snp_dbg("get DHCP-Snooping option82 circuit-id mode %s on eth-port(%d) success\n",
							(*circuitid_mode == NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_DEFAULT) ? "default" :
							((*circuitid_mode == NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_USERSTR) ? "user-defined" : "unknow"),
							g_eth_index);
	#endif
	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * dhcp_snp_set_opt82_port_remoteid
 *		set remoteid of DHCP_Snooping option82 on special vlan's port
 *
 *	INPUT:
 *	 	unsigned short vlanId,
 *		unsigned int g_eth_index,
 *		unsigned char tagMode,
 *		unsigned char remoteid_mode,
 *		char *remoteid_content
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 * 	 	DHCP_SNP_RETURN_CODE_OK					- success
 *	 	DHCP_SNP_RETURN_CODE_ALREADY_SET			- have set remote-id mode
 * 		DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST 		- vlan not exist
 * 		DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST	- port is not member of the vlan
 * 		DHCP_SNP_RETURN_CODE_PARAM_NULL		 	- parameter is null
 *
 **********************************************************************************/
unsigned int dhcp_snp_set_opt82_port_remoteid
(
	unsigned short vlanId,
	unsigned int g_eth_index,
	unsigned char tagMode,
	unsigned char remoteid_mode,
	char *remoteid_content
)
{
	struct vlan_s *vlanNode = NULL;
	struct vlan_port_list_node_s *node = NULL;
	struct list_head *list = NULL;
	struct list_head *pos  = NULL;
	vlan_port_list_s *portList = NULL;

	if (!remoteid_content) {
		syslog_ax_dhcp_snp_err("set option 82 remote-id error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	syslog_ax_dhcp_snp_dbg("set vlan %d eth-port %d tag mode %s option82 remote-ID mode %s\n",
							vlanId, g_eth_index,
							tagMode ? "tagged" : "untagged",  
							(remoteid_mode == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC) ? "default" :
							((remoteid_mode == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_USERSTR) ? "user-defined" :
							((remoteid_mode == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSNAME) ? "sysname" :
							"unknow")));

	#ifdef NO_DHCPSNP_STANDALONE
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
				/* check port circuit-id mode */
				if ((node->dhcp_snp_info.opt82_remoteid== remoteid_mode) &&
					(node->dhcp_snp_info.opt82_remoteid == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC))
				{
					syslog_ax_dhcp_snp_err("vlanId %d eth-port %d option82 Remote-ID mode have setted default.\n",
											vlanId, g_eth_index);
					return DHCP_SNP_RETURN_CODE_ALREADY_SET;
				}

				node->dhcp_snp_info.opt82_remoteid = remoteid_mode;

				if (remoteid_mode == NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_DEFAULT) {
					/* clear the circuit-id str */
					memset(node->dhcp_snp_info.opt82_remoteid_str, 0, NPD_DHCP_SNP_REMOTEID_STR_LEN);
				}else {
					memset(node->dhcp_snp_info.opt82_remoteid_str, 0, NPD_DHCP_SNP_REMOTEID_STR_LEN);
					memcpy(node->dhcp_snp_info.opt82_remoteid_str, remoteid_content, strlen(remoteid_content));
				}

				syslog_ax_dhcp_snp_dbg("set vlan %d eth-port %d tag mode %s option82 Remote-ID mode %s content [%s] success.\n",
					vlanId, g_eth_index,
					tagMode ? "tagged" : "untagged",  
					(remoteid_mode == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC) ? "default" :
					((remoteid_mode == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_USERSTR) ? "user-defined" :
					((remoteid_mode == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSNAME) ? "sysname" :
					"unknow")),
					remoteid_content);

				return DHCP_SNP_RETURN_CODE_OK;
			}
		}
	}
	#endif
	
	return DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST;
}

/**********************************************************************************
 * dhcp_snp_get_opt82_port_remoteid
 *		get remoteid of DHCP_Snooping option82 on special port
 *
 *	INPUT:
 *		unsigned int g_eth_index,
 *		unsigned char tagMode,
 *		unsigned short vlanid,
 *
 *	OUTPUT:
 *		unsigned int *remoteid_mode,
 *		char *remoteid_content
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_NOT_FOUND	- error
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- parameter is null
 **********************************************************************************/
unsigned int dhcp_snp_get_opt82_port_remoteid
(
	unsigned int g_eth_index,
	unsigned char tagMode,
	unsigned short vlanid,
	unsigned char *remoteid_mode,
	char *remoteid_content
)
{
	unsigned int get_flag = NPD_DHCP_SNP_INIT_0;
	unsigned int tmp_remoteid_mode = NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_INVALID;
	char tmp_remoteid_content[NPD_DHCP_SNP_REMOTEID_STR_LEN + 4] = {0};
	unsigned int content_len = NPD_DHCP_SNP_INIT_0;


	NPD_DHCP_OPT82_SUBOPT_T subopt;
	NPD_DHCP_OPT82_REMOTEID_T remoteid;
	NPD_DHCP_OPT82_REMOTEID_DATA_T remoteid_data;
	NPD_DHCP_OPT82_COM_DATA_T com_data;
	NPD_DHCP_SNP_PORT_T *dhcp_snp_info = NULL;

	struct vlan_s *vlanNode = NULL;
	struct vlan_port_list_node_s *node = NULL;
	struct list_head *list = NULL;
	struct list_head *pos  = NULL;
	vlan_port_list_s *portList = NULL;

	if (!remoteid_content || !remoteid_mode) {
		syslog_ax_dhcp_snp_err("get option 82 remote-id error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	syslog_ax_dhcp_snp_dbg("get vlan %d eth-port %d tag mode %s option82 Remote-ID mode\n",
							vlanid, g_eth_index,
							tagMode ? "tagged" : "untagged");

	memset(&subopt, 0, sizeof(NPD_DHCP_OPT82_SUBOPT_T));
	memset(&remoteid, 0, sizeof(NPD_DHCP_OPT82_REMOTEID_T));
	memset(&remoteid_data, 0, sizeof(NPD_DHCP_OPT82_REMOTEID_DATA_T));
	memset(&com_data, 0, sizeof(NPD_DHCP_OPT82_COM_DATA_T));

	#ifdef NO_DHCPSNP_STANDALONE
	vlanNode = npd_find_vlan_by_vid(vlanid);
	if (NULL == vlanNode) {
		syslog_ax_dhcp_snp_err("vlan %d is not created.\n", vlanid);
		return DHCP_SNP_RETURN_CODE_VLAN_NOTEXIST;
	}

	/* tagged port list*/
	if (NPD_TRUE == tagMode) {
		portList = vlanNode->tagPortList;
	}else if (NPD_FALSE == tagMode) {
		portList = vlanNode->untagPortList;
	}

	if (NULL == portList) {
		syslog_ax_dhcp_snp_err("eth-port %d is not member of the vlan %d.\n", g_eth_index, vlanid);
		return DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST;
	}
	else if(0 != portList->count)
	{
		syslog_ax_dhcp_snp_dbg("vlan %d %s port count %d\n",
					vlanid, tagMode ? "tagged" : "untagged", portList->count);
		
		list = &(portList->list);
		__list_for_each(pos,list) {
			node = list_entry(pos, struct vlan_port_list_node_s, list);
			if (NULL != node && g_eth_index == node->eth_global_index)
			{
				/* get port remote-id mode */
				dhcp_snp_info = &(node->dhcp_snp_info);
				syslog_ax_dhcp_snp_dbg("get vlan %d eth-port %d tag mode %s option82 Remote-ID mode %s content [%s] success.\n",
					vlanid, g_eth_index,
					tagMode ? "tagged" : "untagged",  
					(dhcp_snp_info->opt82_remoteid == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC) ? "default" :
					((dhcp_snp_info->opt82_remoteid == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_USERSTR) ? "user-defined" :
					((dhcp_snp_info->opt82_remoteid == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSNAME) ? "sysname" :
					"unknow")),
					dhcp_snp_info->opt82_remoteid_str);

				get_flag = 1;/* found */
				break;
			}
		}
	}

	if (1 != get_flag) {
		syslog_ax_dhcp_snp_err("eth-port %d is not member of the vlan %d.\n", g_eth_index, vlanid);
		return DHCP_SNP_RETURN_CODE_VLAN_PORT_NO_EXIST;
	}

	dhcp_snp_print_port_func_data(g_eth_index, dhcp_snp_info);

	/*
	           0                      7                     15                    23                  31
	            ---------------------------------------------------------
	           | suboption_type | suboption_len  | remote-id_type| remote-id_len |
	            ---------------------------------------------------------
	    data |                 Bridge Mac Address                                                |
	                                                          ----------------------------
	           |                                             |
	            -----------------------------
	    note:
	        1. if fill format is standard, field of  remote-id_type and remote-id_len are miss
	        2. if data is default, length is 2 + 2
	            if data is user-defined, length is 64
	*/
	/* check port remoteid mode */
	if (dhcp_snp_opt82_remoteid_type == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC)
	{
		/* config of port is priority */
		tmp_remoteid_mode = dhcp_snp_info->opt82_remoteid;
		if (tmp_remoteid_mode == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC)
		{	/* default use hex format */
			remoteid.remote_type = 0x0;
			remoteid.remote_len = 0x6;
			
			if (NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_EXT == dhcp_snp_opt82_fill_format_type)
			{	/* add field of remote-id_type and remote-id_len */
				memcpy(tmp_remoteid_content + 2, &remoteid, sizeof(NPD_DHCP_OPT82_REMOTEID_T));
				content_len += sizeof(NPD_DHCP_OPT82_REMOTEID_T);
			
			}else if (NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_STD == dhcp_snp_opt82_fill_format_type)
			{	/* not add field of remote-id_type and remote-id_len  */
				syslog_ax_dhcp_snp_dbg("option82 fill format is standard.\n");
			}

			if (NPD_DHCP_SNP_OPT82_FORMAT_TYPE_HEX == dhcp_snp_opt82_format_type)
			{
				/* add default data use in hex */
				if (0 != npd_system_get_basemac(remoteid_data.mac, 6)) {
					syslog_ax_dhcp_snp_err("get basemac error\n");
					return DHCP_SNP_RETURN_CODE_ERROR;
				}
				memcpy(tmp_remoteid_content + 2 + content_len, &remoteid_data, sizeof(NPD_DHCP_OPT82_REMOTEID_DATA_T));
				content_len += sizeof(NPD_DHCP_OPT82_REMOTEID_DATA_T);
	
			}
			else if (NPD_DHCP_SNP_OPT82_FORMAT_TYPE_ASCII == dhcp_snp_opt82_format_type)
			{
				memcpy(com_data.data, PRODUCT_MAC_ADDRESS, 12);
				memcpy(tmp_remoteid_content + 2 + content_len, com_data.data, 12);
				content_len += 12;
	
				/* modify remote-id len, becase the len is not 6 byte at this time */
				if (NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_EXT == dhcp_snp_opt82_fill_format_type)
				{
					NPD_DHCP_OPT82_REMOTEID_T *tmp_remoteid = NULL;
					tmp_remoteid = (NPD_DHCP_OPT82_REMOTEID_T *)(tmp_remoteid_content + 2);
					tmp_remoteid->remote_type = 0x1; /* ascii */
					tmp_remoteid->remote_len = 12;
				}
			}
			
			subopt.subopt_type = DHCP_REMOTE_ID;
			subopt.subopt_len = content_len;
			memcpy(tmp_remoteid_content, &subopt, sizeof(NPD_DHCP_OPT82_SUBOPT_T));
		}
		else {
			/* user-defined on port value, ascii */
			remoteid.remote_type = 0x1;
			remoteid.remote_len = 0x40; /* 64 byte */

			if (NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_EXT == dhcp_snp_opt82_fill_format_type)
			{	/* add field of remote-id_type and remote-id_len */
				memcpy(tmp_remoteid_content + 2, &remoteid, sizeof(NPD_DHCP_OPT82_REMOTEID_T));
				content_len += sizeof(NPD_DHCP_OPT82_REMOTEID_T);
			
			}else if (NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_STD == dhcp_snp_opt82_fill_format_type)
			{	/* not add field of remote-id_type and remote-id_len  */
				syslog_ax_dhcp_snp_dbg("option82 fill format is standard.\n");
			}

			memcpy(com_data.data, dhcp_snp_info->opt82_remoteid_str, NPD_DHCP_SNP_REMOTEID_STR_LEN);
			
			memcpy(tmp_remoteid_content + 2 + content_len, com_data.data, NPD_DHCP_SNP_REMOTEID_STR_LEN);
			content_len += NPD_DHCP_SNP_REMOTEID_STR_LEN;

			subopt.subopt_type = DHCP_REMOTE_ID;
			subopt.subopt_len = content_len;
			memcpy(tmp_remoteid_content, &subopt, sizeof(NPD_DHCP_OPT82_SUBOPT_T));
		}
	}
	else {
		/* user-defined */
		if (dhcp_snp_info->opt82_remoteid != NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC)
		{
			/* user-defined value on port, ascii */
			remoteid.remote_type = 0x1;
			remoteid.remote_len = 0x40; /* 64 byte */

			if (NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_EXT == dhcp_snp_opt82_fill_format_type)
			{	/* add field of remote-id_type and remote-id_len */
				memcpy(tmp_remoteid_content + 2, &remoteid, sizeof(NPD_DHCP_OPT82_REMOTEID_T));
				content_len += sizeof(NPD_DHCP_OPT82_REMOTEID_T);
			
			}else if (NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_STD == dhcp_snp_opt82_fill_format_type)
			{	/* not add field of remote-id_type and remote-id_len  */
				syslog_ax_dhcp_snp_dbg("option82 fill format is standard.\n");
			}

			memcpy(com_data.data, dhcp_snp_info->opt82_remoteid_str, NPD_DHCP_SNP_REMOTEID_STR_LEN);
			
			memcpy(tmp_remoteid_content + 2 + content_len, com_data.data, NPD_DHCP_SNP_REMOTEID_STR_LEN);
			content_len += NPD_DHCP_SNP_REMOTEID_STR_LEN;

			subopt.subopt_type = DHCP_REMOTE_ID;
			subopt.subopt_len = content_len;
			memcpy(tmp_remoteid_content, &subopt, sizeof(NPD_DHCP_OPT82_SUBOPT_T));

			tmp_remoteid_mode = dhcp_snp_info->opt82_remoteid;
		}
		else {
			/* global config */	
			/* user-defined value global, ascii */
			remoteid.remote_type = 0x1;
			remoteid.remote_len = 0x40; /* 64 byte */

			if (NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_EXT == dhcp_snp_opt82_fill_format_type)
			{	/* add field of remote-id_type and remote-id_len */
				memcpy(tmp_remoteid_content + 2, &remoteid, sizeof(NPD_DHCP_OPT82_REMOTEID_T));
				content_len += sizeof(NPD_DHCP_OPT82_REMOTEID_T);
			
			}else if (NPD_DHCP_SNP_OPT82_FILL_FORMAT_TYPE_STD == dhcp_snp_opt82_fill_format_type)
			{	/* not add field of remote-id_type and remote-id_len  */
				syslog_ax_dhcp_snp_dbg("option82 fill format is standard.\n");
			}
			
			if (dhcp_snp_opt82_remoteid_type == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSNAME)
			{							
				memcpy(com_data.data, PRODUCT_SYSTEM_NAME, strlen(PRODUCT_SYSTEM_NAME));
				tmp_remoteid_mode = NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSNAME;
			}
			else if (dhcp_snp_opt82_remoteid_type == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_USERSTR)
			{
				memcpy(com_data.data, dhcp_snp_opt82_remoteid_str, NPD_DHCP_SNP_REMOTEID_STR_LEN);	
				/* user-defined value */
				tmp_remoteid_mode = NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_USERSTR;
			}
			else {
				return DHCP_SNP_RETURN_CODE_ERROR;
			}

			memcpy(tmp_remoteid_content + 2 + content_len, com_data.data, NPD_DHCP_SNP_REMOTEID_STR_LEN);
			content_len += NPD_DHCP_SNP_REMOTEID_STR_LEN;
			
			subopt.subopt_type = DHCP_REMOTE_ID;
			subopt.subopt_len = content_len;
			memcpy(tmp_remoteid_content, &subopt, sizeof(NPD_DHCP_OPT82_SUBOPT_T));
		}
	}

	/* get port Circuit-ID mode and Circuit-ID content */
	*remoteid_mode = tmp_remoteid_mode;
	memcpy(remoteid_content, tmp_remoteid_content, content_len + 2);

	syslog_ax_dhcp_snp_dbg("get DHCP-Snooping option82 remote-id mode %s on eth-port(%d) success\n",
							(*remoteid_mode == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSMAC) ? "default" :
								((*remoteid_mode == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_USERSTR) ? "user-defined" :
									((*remoteid_mode == NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_SYSNAME) ? "sysname" : "unknow")),
							g_eth_index);
	#endif
	return DHCP_SNP_RETURN_CODE_OK;
}


/**********************************************************************************
 * dhcp_snp_get_opt82_suboption
 *		get option82's sub-option with bounds checking (warning, not aligned).
 *
 *	INPUT:
 *		NPD_DHCP_MESSAGE_T *packet,
 *		unsigned int code,
 *		unsinged int subid
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		NULL			- not get the option
 *		not NULL 		- sub option of the option 82
 **********************************************************************************/
NPD_DHCP_OPTION_T *dhcp_snp_get_opt82_suboption
(
	NPD_DHCP_MESSAGE_T *packet,
	unsigned int code,
	unsigned int subcode
)
{
	unsigned int i = NPD_DHCP_SNP_INIT_0;
	unsigned int m = NPD_DHCP_SNP_INIT_0;
	unsigned int length = NPD_DHCP_SNP_INIT_0;
	unsigned int over = NPD_DHCP_SNP_INIT_0;
	unsigned int done = NPD_DHCP_SNP_INIT_0;
	unsigned int curr = NPD_DHCP_SNP_OPTION_FIELD;
	unsigned char *optionptr = NULL;
	NPD_DHCP_OPTION_T *option = NULL;
	NPD_DHCP_OPTION_T *sub_option = NULL;

	if (!packet) {
		syslog_ax_dhcp_snp_err("get option %02x suboption %02x error, parameter is null.", code, subcode);
		return NULL;
	}

	optionptr = (unsigned char *)packet->options;
	i = 0;
	length = NPD_DHCP_SNP_OPTION_LEN;

	while (!done)
	{
		if (i >= length)
		{
			syslog_ax_dhcp_snp_err("bogus packet, option fields too long.");
			return NULL;
		}
	
		if (optionptr[i + NPD_DHCP_SNP_OPT_CODE] == code)
		{
			if (i + 1 + optionptr[i + NPD_DHCP_SNP_OPT_LEN] >= length)
			{
				syslog_ax_dhcp_snp_err("bogus packet, option fields too long.");
				return NULL;
			}

			option = (NPD_DHCP_OPTION_T *)(optionptr[i] + NPD_DHCP_SNP_OPT_CODE);
			for (m = 2; m < option->leng; m += sub_option->leng + 2)
			{
				sub_option = (NPD_DHCP_OPTION_T *)(optionptr[i] + m);
				if (sub_option->code == subcode) {
					return sub_option;
				}
			}
		}

		switch (optionptr[i + NPD_DHCP_SNP_OPT_CODE])
		{
			case DHCP_PADDING:
				i++;
				break;
			
			case DHCP_OPTION_OVER:
				if (i + 1 + optionptr[i + NPD_DHCP_SNP_OPT_LEN] >= length)
				{
					syslog_ax_dhcp_snp_err("bogus packet, option fields too long.");
					return NULL;
				}
				over = optionptr[i + 3];
				i += optionptr[NPD_DHCP_SNP_OPT_LEN] + 2;
				break;
			
			case DHCP_END:
				if (curr == NPD_DHCP_SNP_OPTION_FIELD && over & NPD_DHCP_SNP_FILE_FIELD)
				{
					optionptr = (unsigned char *)packet->file;
					i = 0;
					length = 128;
					curr = NPD_DHCP_SNP_FILE_FIELD;
				} else if (curr == NPD_DHCP_SNP_FILE_FIELD && over & NPD_DHCP_SNP_SNAME_FIELD)
				{
					optionptr = (unsigned char *)packet->sname;
					i = 0;
					length = 64;
					curr = NPD_DHCP_SNP_SNAME_FIELD;
				} else
				{
					done = 1;
				}
				break;
			
			default:
				i += optionptr[NPD_DHCP_SNP_OPT_LEN + i] + 2;
		}
	}

	return NULL;
}



/**********************************************************************************
 * dhcp_snp_check_opt82
 *		check the packet if have option 82 with bounds checking (warning, not aligned).
 *
 *	INPUT:
 *		NPD_DHCP_MESSAGE_T *packet,
 *		unsigned int code
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_FOUND		- have the option
 *		DHCP_SNP_RETURN_CODE_NOT_FOUND	- not have the option
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- parameter is null
 **********************************************************************************/
unsigned int dhcp_snp_check_opt82
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
		syslog_ax_dhcp_snp_err("remove option82 error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	optionptr = (unsigned char *)packet->options;
	i = 0;
	length = NPD_DHCP_SNP_OPTION_LEN;

	while (!done)
	{
		if (i >= length)
		{
			syslog_ax_dhcp_snp_err("bogus packet, option fields too long.");
			return DHCP_SNP_RETURN_CODE_NOT_FOUND;
		}
	
		if (optionptr[i + NPD_DHCP_SNP_OPT_CODE] == code)
		{
			if (i + 1 + optionptr[i + NPD_DHCP_SNP_OPT_LEN] >= length)
			{
				syslog_ax_dhcp_snp_err("bogus packet, option fields too long.");
				return DHCP_SNP_RETURN_CODE_NOT_FOUND;
			}
			
			syslog_ax_dhcp_snp_dbg("found option code %x.", code);
			return DHCP_SNP_RETURN_CODE_FOUND;
		}

		switch (optionptr[i + NPD_DHCP_SNP_OPT_CODE])
		{
			case DHCP_PADDING:
				i++;
				break;
			
			case DHCP_OPTION_OVER:
				if (i + 1 + optionptr[i + NPD_DHCP_SNP_OPT_LEN] >= length)
				{
					syslog_ax_dhcp_snp_err("bogus packet, option fields too long.");
					return DHCP_SNP_RETURN_CODE_NOT_FOUND;
				}
				over = optionptr[i + 3];
				i += optionptr[NPD_DHCP_SNP_OPT_LEN] + 2;
				break;
			
			case DHCP_END:
				if (curr == NPD_DHCP_SNP_OPTION_FIELD && over & NPD_DHCP_SNP_FILE_FIELD)
				{
					optionptr = (unsigned char *)packet->file;
					i = 0;
					length = 128;
					curr = NPD_DHCP_SNP_FILE_FIELD;
				} else if (curr == NPD_DHCP_SNP_FILE_FIELD && over & NPD_DHCP_SNP_SNAME_FIELD)
				{
					optionptr = (unsigned char *)packet->sname;
					i = 0;
					length = 64;
					curr = NPD_DHCP_SNP_SNAME_FIELD;
				} else
				{
					done = 1;
				}
				break;
			
			default:
				i += optionptr[NPD_DHCP_SNP_OPT_LEN + i] + 2;
		}
	}

	syslog_ax_dhcp_snp_dbg("not found option code %x.", code);
	return DHCP_SNP_RETURN_CODE_NOT_FOUND;
}

/**********************************************************************************
 * dhcp_snp_remove_opt82
 *		remove option 82 the packet if have option 82 with bounds checking (warning, not aligned).
 *
 *	INPUT:
 *		NPD_DHCP_MESSAGE_T *packet,
 *		unsigned int code,
 *		unsigned int opt_len
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- parameter is null
 *
 **********************************************************************************/
unsigned int dhcp_snp_remove_opt82
(
	NPD_DHCP_MESSAGE_T *packet,
	unsigned int code,
	unsigned int opt_len,
	unsigned int *del_len
)
{
	unsigned int i = NPD_DHCP_SNP_INIT_0;
	unsigned int length = NPD_DHCP_SNP_INIT_0;
	unsigned int over = NPD_DHCP_SNP_INIT_0;
	unsigned int done = NPD_DHCP_SNP_INIT_0;
	unsigned int curr = NPD_DHCP_SNP_OPTION_FIELD;
	unsigned char *optionptr = NULL;

	if (!packet) {
		syslog_ax_dhcp_snp_err("remove option82 error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	optionptr = (unsigned char *)packet->options;
	i = 0;
	length = NPD_DHCP_SNP_OPTION_LEN;

	while (!done)
	{
		if (i >= length)
		{
			syslog_ax_dhcp_snp_err("bogus packet, option fields too long.");
			return DHCP_SNP_RETURN_CODE_ERROR;
		}
	
		if (optionptr[i + NPD_DHCP_SNP_OPT_CODE] == code)
		{
			if (i + 1 + optionptr[i + NPD_DHCP_SNP_OPT_LEN] >= length)
			{
				syslog_ax_dhcp_snp_err("bogus packet, option fields too long.");
				return DHCP_SNP_RETURN_CODE_ERROR;
			}

			*del_len = optionptr[NPD_DHCP_SNP_OPT_LEN] + 2;
			memcpy(optionptr + i, optionptr + (i + optionptr[NPD_DHCP_SNP_OPT_LEN] + 2),
					opt_len - (i + optionptr[NPD_DHCP_SNP_OPT_LEN] + 2));
			return DHCP_SNP_RETURN_CODE_OK;
		}

		switch (optionptr[i + NPD_DHCP_SNP_OPT_CODE])
		{
			case DHCP_PADDING:
				i++;
				break;
			
			case DHCP_OPTION_OVER:
				if (i + 1 + optionptr[i + NPD_DHCP_SNP_OPT_LEN] >= length)
				{
					syslog_ax_dhcp_snp_err("bogus packet, option fields too long.");
					return DHCP_SNP_RETURN_CODE_ERROR;
				}
				over = optionptr[i + 3];
				i += optionptr[NPD_DHCP_SNP_OPT_LEN] + 2;
				break;
			
			case DHCP_END:
				if (curr == NPD_DHCP_SNP_OPTION_FIELD && over & NPD_DHCP_SNP_FILE_FIELD)
				{
					optionptr = (unsigned char *)packet->file;
					i = 0;
					length = 128;
					curr = NPD_DHCP_SNP_FILE_FIELD;
				} else if (curr == NPD_DHCP_SNP_FILE_FIELD && over & NPD_DHCP_SNP_SNAME_FIELD)
				{
					optionptr = (unsigned char *)packet->sname;
					i = 0;
					length = 64;
					curr = NPD_DHCP_SNP_SNAME_FIELD;
				} else
				{
					done = 1;
				}
				break;
			
			default:
				i += optionptr[NPD_DHCP_SNP_OPT_LEN + i] + 2;
		}
	}

	*del_len = NPD_DHCP_SNP_INIT_0;
	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 * dhcp_snp_cat_opt82_sting
 *		cat option 82 string with Circuit-ID sting and Remote-ID string.
 *
 *	INPUT:
 *		char *circuitid_str,
 *		char *remoteid_str,
 *
 *	OUTPUT:
 *		char *opt82_str,
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- parameter is null
 *
 **********************************************************************************/
unsigned int dhcp_snp_cat_opt82_sting
(
	char *circuitid_str,
	char *remoteid_str,
	char *opt82_str
)
{
	unsigned int opt82_len = NPD_DHCP_SNP_INIT_0;	
	
	if (!opt82_str || !circuitid_str || !remoteid_str) {
		syslog_ax_dhcp_snp_err("cat option82 string error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	opt82_str[NPD_DHCP_SNP_OPT_CODE] = DHCP_OPTION_82;

	memcpy(opt82_str + 2, circuitid_str, circuitid_str[NPD_DHCP_SNP_OPT_LEN] + 2);
	opt82_len += circuitid_str[NPD_DHCP_SNP_OPT_LEN] + 2;
	
	memcpy(opt82_str + 2 + opt82_len, remoteid_str, remoteid_str[NPD_DHCP_SNP_OPT_LEN] + 2);
	opt82_len += remoteid_str[NPD_DHCP_SNP_OPT_LEN] + 2;

	opt82_str[NPD_DHCP_SNP_OPT_LEN] = opt82_len;

	return DHCP_SNP_RETURN_CODE_OK;
}




/**********************************************************************************
 * dhcp_snp_find_opt82_add_position
 *		get option82's add position.
 *
 *	INPUT:
 *		NPD_DHCP_MESSAGE_T *packet,
 *		unsigned int code,
 *
 *	OUTPUT:
 *		unsigned int *position
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK			- success, get the position
 *		DHCP_SNP_RETURN_CODE_ERROR 		- fail, not get the position
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- parameter is null
 *
 **********************************************************************************/
unsigned int dhcp_snp_find_opt82_add_position
(
	NPD_DHCP_MESSAGE_T *packet,
	unsigned int code,
	unsigned int *position
)
{
	unsigned int i = NPD_DHCP_SNP_INIT_0;
	unsigned int length = NPD_DHCP_SNP_INIT_0;
	unsigned int over = NPD_DHCP_SNP_INIT_0;
	unsigned int done = NPD_DHCP_SNP_INIT_0;
	unsigned int curr = NPD_DHCP_SNP_OPTION_FIELD;
	unsigned char *optionptr = NULL;

	if (!packet || !position) {
		syslog_ax_dhcp_snp_err("find option %02x add position error, parameter is null\n", code);
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	optionptr = (unsigned char *)packet->options;
	i = 0;
	length = NPD_DHCP_SNP_OPTION_LEN;

	while (!done)
	{
		if (i >= length)
		{
			syslog_ax_dhcp_snp_err("bogus packet, option fields too long.");
			return DHCP_SNP_RETURN_CODE_ERROR;
		}

		if (optionptr[i + NPD_DHCP_SNP_OPT_CODE] > code) {
			*position = i;
			syslog_ax_dhcp_snp_dbg("find position %d to add option %02x.", *position, code);
			return DHCP_SNP_RETURN_CODE_OK;
		}

		switch (optionptr[i + NPD_DHCP_SNP_OPT_CODE])
		{
			case DHCP_PADDING:
				i++;
				break;
			
			case DHCP_OPTION_OVER:
				if (i + 1 + optionptr[i + NPD_DHCP_SNP_OPT_LEN] >= length)
				{
					syslog_ax_dhcp_snp_err("bogus packet, option fields too long.");
					return DHCP_SNP_RETURN_CODE_ERROR;
				}
				over = optionptr[i + 3];
				i += optionptr[NPD_DHCP_SNP_OPT_LEN] + 2;
				break;
			
			case DHCP_END:
				if (curr == NPD_DHCP_SNP_OPTION_FIELD && over & NPD_DHCP_SNP_FILE_FIELD)
				{
					optionptr = (unsigned char *)packet->file;
					i = 0;
					length = 128;
					curr = NPD_DHCP_SNP_FILE_FIELD;
				} else if (curr == NPD_DHCP_SNP_FILE_FIELD && over & NPD_DHCP_SNP_SNAME_FIELD)
				{
					optionptr = (unsigned char *)packet->sname;
					i = 0;
					length = 64;
					curr = NPD_DHCP_SNP_SNAME_FIELD;
				} else
				{
					done = 1;
				}
				break;
			
			default:
				i += optionptr[NPD_DHCP_SNP_OPT_LEN + i] + 2;
		}
	}

	syslog_ax_dhcp_snp_err("not found position to add option %02x.", code);
	return DHCP_SNP_RETURN_CODE_ERROR;
}


/**********************************************************************************
 * dhcp_snp_attach_opt82_string
 *		attach option 82 string to the packet with bounds checking (warning, not aligned).
 *		add an option string to the options (an option string contains an option code, * length, then data) 
 *
 *	INPUT:
 *		NPD_DHCP_MESSAGE_T *packet,
 *		unsigned int code,
 *		unsigned char *string
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- parameter is null
 *
 **********************************************************************************/

unsigned int dhcp_snp_attach_opt82_string
(
	NPD_DHCP_MESSAGE_T *packet,
	unsigned int code,
	char *string
)
{
	unsigned int end = NPD_DHCP_SNP_INIT_0;
	unsigned char *optionptr = NULL;

	if (!packet || !string) {
		syslog_ax_dhcp_snp_err("attach option82 string error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	optionptr = (unsigned char *)packet->options;
	end = dhcp_snp_end_option(optionptr);
	
	/* end position + string length + option code/length + end option */
	if (end + string[NPD_DHCP_SNP_OPT_LEN] + 2 + 1 >= NPD_DHCP_SNP_OPTION_LEN) {
		syslog_ax_dhcp_snp_err("Option 0x%02x did not fit into the packet!\n", string[NPD_DHCP_SNP_OPT_CODE]);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	syslog_ax_dhcp_snp_dbg("adding option 0x%02x\n", string[NPD_DHCP_SNP_OPT_CODE]);

	/* copy */
	memcpy(optionptr + end, string, string[NPD_DHCP_SNP_OPT_LEN] + 2);

	optionptr[end + string[NPD_DHCP_SNP_OPT_LEN] + 2] = DHCP_END;
	
	return DHCP_SNP_RETURN_CODE_OK;
}


#if 0
unsigned int dhcp_snp_attach_opt82_string
(
	NPD_DHCP_MESSAGE_T *packet,
	unsigned int code,
	char *string
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned int end = NPD_DHCP_SNP_INIT_0;
	unsigned char *optionptr = NULL;
	unsigned int add_position = NPD_DHCP_SNP_INIT_0;

	if (!packet || !string) {
		syslog_ax_dhcp_snp_err("attach option82 string error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	optionptr = (unsigned char *)packet->options;
	end = dhcp_snp_end_option(optionptr);
	
	/* end position + string length + option code/length + end option */
	if (end + string[NPD_DHCP_SNP_OPT_LEN] + 2 + 1 >= NPD_DHCP_SNP_OPTION_LEN) {
		syslog_ax_dhcp_snp_err("Option 0x%02x did not fit into the packet!\n", string[NPD_DHCP_SNP_OPT_CODE]);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	syslog_ax_dhcp_snp_dbg("adding option 0x%02x\n", string[NPD_DHCP_SNP_OPT_CODE]);

	/* find the poistion of the option82 */
	ret = dhcp_snp_find_opt82_add_position(packet, code, &add_position);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("find position to add option error, ret %x.\n", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	/* move copy */
	if (end >= add_position) {
		memcpy(optionptr + add_position + string[NPD_DHCP_SNP_OPT_LEN] + 2, optionptr + add_position, end - add_position);
		memcpy(optionptr + add_position, string, string[NPD_DHCP_SNP_OPT_LEN] + 2);
	}else {
		syslog_ax_dhcp_snp_err("end position %d, add position %d \n", end, add_position);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	/*optionptr[end + string[NPD_DHCP_SNP_OPT_LEN] + 2] = DHCP_END;*/
	
	return DHCP_SNP_RETURN_CODE_OK;
}
#endif

/**********************************************************************************
 * dhcp_snp_end_option
 *		return the position of the 'end' option (no bounds checking) 
 *
 *	INPUT:
 *		unsigned char *optionptr
 *
 *	OUTPUT:
 *		NULL
 *
 *	RETURN:
 *		unsigned int		- end position of the option
 *
 **********************************************************************************/
unsigned int dhcp_snp_end_option
(
	unsigned char *optionptr
) 
{
	int i = NPD_DHCP_SNP_INIT_0;
	
	while (optionptr[i] != DHCP_END) {
		if (optionptr[i] == DHCP_PADDING) {
			i++;
		}else {
			i += optionptr[i + NPD_DHCP_SNP_OPT_LEN] + 2;
		}
	}

	return i;
}

/**********************************************************************************
 * dhcp_snp_attach_opt82
 *		attach DHCP_Snooping option82 to dhcp packet
 *
 *	INPUT:
 *		unsigned char *packet
 *		unsigned int ifindex,
 *		unsigned char isTagged,
 *		unsigned short vlanid
 *
 *	OUTPUT:
 *		unsigned long *bufflen		- packet len
 *
 *	RETURN:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *	 	DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- parameter is null
 *
 **********************************************************************************/
unsigned int dhcp_snp_attach_opt82
(
	unsigned char *packet,
	unsigned int ifindex,
	unsigned char isTagged,
	unsigned short vlanid,
	unsigned long *bufflen
)
{

	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	unsigned char strategy_mode = NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_INVALID;
	unsigned char circuitid_type = NPD_DHCP_SNP_OPT82_CIRCUITID_TYPE_INVALID;
	char circuitid_str[NPD_DHCP_SNP_CIRCUITID_STR_LEN + 4] = {0};
	unsigned char remoteid_type = NPD_DHCP_SNP_OPT82_REMOTEID_TYPE_INVALID;
	char remoteid_str[NPD_DHCP_SNP_REMOTEID_STR_LEN + 4] = {0};
	char opt82_str[140] = {0};
	
	NPD_DHCP_MESSAGE_T *dhcp = NULL;
	struct iphdr* ip = NULL;
	struct udphdr* udp = NULL;
	struct iphdr tempip;
	unsigned int source = NPD_DHCP_SNP_INIT_0;
	unsigned int dest = NPD_DHCP_SNP_INIT_0;
	unsigned short check = NPD_DHCP_SNP_INIT_0;
	
	unsigned int headlen = NPD_DHCP_SNP_INIT_0;
	unsigned int originlen = NPD_DHCP_SNP_INIT_0;
	unsigned int endposition = NPD_DHCP_SNP_INIT_0;
	unsigned int del_len = NPD_DHCP_SNP_INIT_0;

	if (!packet || !bufflen) {
		syslog_ax_dhcp_snp_err("attach option 82 error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	memset(&tempip, 0, sizeof(struct iphdr));
	ip = (struct iphdr *)(packet + sizeof(struct ethhdr));
	udp = (struct udphdr *)(packet + sizeof(struct ethhdr) + sizeof(struct iphdr)); 
	dhcp = (NPD_DHCP_MESSAGE_T *)(packet + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr));

	ret = dhcp_snp_get_opt82_port_strategy(vlanid, ifindex, isTagged, &strategy_mode);
	/* get success */
	if (DHCP_SNP_RETURN_CODE_OK == ret)
	{
		if (NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_REPLACE == strategy_mode)
		{
			syslog_ax_dhcp_snp_dbg("get option 82 port strategy is replace.\n");

			/*
			**********************
			*  get Circuit-ID string
			**********************
			*/
			ret = dhcp_snp_get_opt82_port_circuitid(ifindex, isTagged, vlanid, &circuitid_type, circuitid_str);
			if (DHCP_SNP_RETURN_CODE_OK != ret) {
				syslog_ax_dhcp_snp_err("get option 82 port Circuit-ID error, ret %x\n", ret);
				return DHCP_SNP_RETURN_CODE_ERROR;
			}
			/*
			**********************
			* get Remote-ID string 
			**********************
			*/
			ret = dhcp_snp_get_opt82_port_remoteid(ifindex, isTagged, vlanid, &remoteid_type, remoteid_str);
			if (DHCP_SNP_RETURN_CODE_OK != ret) {
				syslog_ax_dhcp_snp_err("get option 82 port Remote-ID error, ret %x\n", ret);
				return DHCP_SNP_RETURN_CODE_ERROR;
			}

			/*
			*******************************
			* cat string of Circuit-ID and Remote-ID
			*******************************
			*/
			ret = dhcp_snp_cat_opt82_sting(circuitid_str, remoteid_str, opt82_str);
			if (DHCP_SNP_RETURN_CODE_OK != ret) {
				syslog_ax_dhcp_snp_err("cat option 82 string of Circuit-ID and Remote-ID error, ret %x\n", ret);
				return DHCP_SNP_RETURN_CODE_ERROR;
			}

			ret = dhcp_snp_check_opt82(dhcp, DHCP_OPTION_82);
			if (DHCP_SNP_RETURN_CODE_FOUND == ret)
			{
				/* 1: remove origin option 82*/
				endposition = dhcp_snp_end_option(dhcp->options);
				ret = dhcp_snp_remove_opt82(dhcp, DHCP_OPTION_82, endposition, &del_len);
				if (DHCP_SNP_RETURN_CODE_OK != ret) {
					syslog_ax_dhcp_snp_err("remove option 82 error, ret %x\n", ret);
					return DHCP_SNP_RETURN_CODE_ERROR;
				}

				/* 2: attach local option 82*/
				ret = dhcp_snp_attach_opt82_string(dhcp, DHCP_OPTION_82, opt82_str);
				if (DHCP_SNP_RETURN_CODE_OK != ret) {
					syslog_ax_dhcp_snp_err("attach option 82 string error, ret %x\n", ret);
					return DHCP_SNP_RETURN_CODE_ERROR;
				}
			}
			else if (DHCP_SNP_RETURN_CODE_NOT_FOUND == ret) {
				/* 2: attach local option 82*/
				ret = dhcp_snp_attach_opt82_string(dhcp, DHCP_OPTION_82, opt82_str);
				if (DHCP_SNP_RETURN_CODE_OK != ret) {
					syslog_ax_dhcp_snp_err("attach new option 82 string error, ret %x\n", ret);
					return DHCP_SNP_RETURN_CODE_ERROR;
				}
			}
			else {
				syslog_ax_dhcp_snp_err("check option 82 error, ret %x\n", ret);
				return DHCP_SNP_RETURN_CODE_ERROR;
			}

			/*
			***************************
			*modify packet ip upd head
			***************************
			*/
			headlen = sizeof(struct udphdr) + sizeof(NPD_DHCP_MESSAGE_T) - NPD_DHCP_SNP_OPTION_LEN; 
			originlen = ntohs(udp->len);
			endposition = dhcp_snp_end_option(dhcp->options);
			syslog_ax_dhcp_snp_dbg("endpositon is %d originlen is %d headlen is %d\n", endposition, originlen, headlen);

			if ((headlen + endposition + 1) > originlen)
			{									/* origin len - delete len + add len */
				udp->len = htons(ntohs(udp->len) - del_len + opt82_str[NPD_DHCP_SNP_OPT_LEN] + 2);
				ip->tot_len = htons(ntohs(udp->len) + sizeof(struct iphdr));
				*bufflen = *bufflen - del_len + opt82_str[NPD_DHCP_SNP_OPT_LEN] + 2;
			}
			memcpy(&tempip, ip, sizeof(struct iphdr));
	
			source = ip->saddr;
			dest = ip->daddr;
			check = udp->check;
			udp->check = 0;
			memset(ip, 0, sizeof(struct iphdr));
			
			ip->protocol = IPPROTO_UDP;
			ip->saddr = source;
			ip->daddr = dest;
			ip->tot_len = udp->len; /* cheat on the psuedo-header */
			if (check) {
				udp->check = dhcp_snp_checksum(ip, ntohs(udp->len) + sizeof(struct iphdr)); 
			}		
			memcpy(ip, &tempip, sizeof(struct iphdr));
			ip->check = 0;
			ip->check = dhcp_snp_checksum(ip, sizeof(struct iphdr));
		
		}
		else if (NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_KEEP == strategy_mode) {
			syslog_ax_dhcp_snp_dbg("get option 82 port strategy is keep\n");
			return DHCP_SNP_RETURN_CODE_OK;
		}
		else if (NPD_DHCP_SNP_OPT82_STRATEGY_TYPE_DROP == strategy_mode) {
			syslog_ax_dhcp_snp_dbg("get option 82 port strategy is drop\n");
			return DHCP_SNP_RETURN_CODE_PKT_DROP;
		}
	}
	else {/* get fail */
		syslog_ax_dhcp_snp_err("get option 82 port strategy error, ret %x\n", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	
	return DHCP_SNP_RETURN_CODE_OK;
}

#ifdef __cplusplus
}
#endif

