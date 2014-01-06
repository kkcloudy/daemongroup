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
* dhcp_snp_pkt.c
*
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		dhcp snooping packet for NPD module.
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
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/tipc.h>

#include "sysdef/npd_sysdef.h"
#include "sysdef/returncode.h"

#include "dhcp_snp_log.h"
#include "dhcp_snp_com.h"
#include "dhcp_snp_options.h"
#include "dhcp_snp_tbl.h"
#include "dhcp_snp_netlink.h"
#include "dhcp_snp_pkt.h"
#include "dhcp_snp_listener.h"

extern pthread_mutex_t mutexsnpunsolve;
extern unsigned int dhcp_snp_arp_proxy_enable;

/* ARP packet */
struct arp_packet {
	unsigned char		dmac[ETH_ALEN];		/* destination eth addr	*/
	unsigned char		smac[ETH_ALEN];		/* source ether addr	*/
	unsigned short		etherType;

	unsigned short 	hwType;				/* hardware type: 0x0001-ethernet */
	unsigned short 	protType;			/* protocol type:0x0800-IP */
	unsigned char 	hwSize;				/* hardware size */
	unsigned char 	protSize;			/* protocol size */
	unsigned short 	opCode;				/* 0x0001-request 0x0002-reply */
	unsigned char  	arpsmac[ETH_ALEN];/* sender's MAC address */
	unsigned char 	arpsip[IP_ADDR_LEN];	/* sender's ip address */
	unsigned char 	arpdmac[ETH_ALEN];/* target's MAC address */
	unsigned char 	arpdip[IP_ADDR_LEN];	/* target's ip address */
};

/*********************************************************
*	global variable define											*
**********************************************************/
unsigned int dhcp_snp_notify_portal_enable = 1;	/* 1:enable 0:disable */


/*********************************************************
*	extern variable												*
**********************************************************/


/*********************************************************
*	functions define												*
**********************************************************/

/**********************************************************************************
 *dhcpv6_snp_get_item_from_pkt()
 *
 *	DESCRIPTION:
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex
 *		NPD_DHCP_MESSAGE_T *packet
 *
 *	OUTPUTS:
 *		NPD_DHCP_SNP_USER_ITEM_T *user
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK
 *		DHCP_SNP_RETURN_CODE_ERROR
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL
 *
 ***********************************************************************************/
unsigned int dhcpv6_snp_get_item_from_pkt
(
	unsigned short vlanid,
	unsigned int ifindex,
	enum dhcpv6_packet_type type,
	NPD_DHCPv6_MESSAGE_T *packet,
	NPD_DHCPv6_SNP_USER_ITEM_T *user,
	unsigned char* mac_2
)
{
    unsigned char *temp = NULL;
	int status_g = 0;
	NPD_DHCPv6_SNP_TBL_ITEM_T *item = NULL;	
    if ((packet == NULL) || (user == NULL)) {
		syslog_ax_dhcp_snp_err("dhcp snp get item from pkt error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
    }

	//temp = (unsigned char *)dhcpv6_snp_get_option(packet, DCHPv6_CLIENT_ID);
    user->vlanId    = vlanid;
    user->haddr_len = 6;

    memcpy(user->chaddr, mac_2, NPD_DHCP_SNP_MAC_ADD_LEN);
	
    /* 为了支持从RELAY到本地SERVER申请IP地址，同时在本地支持SNOOPING的处理
     * 添加对没有通过用户MAC+VLAN定位到用户接口时从数据包取接口的处理，同时
     * 支持对响应数据包不带接口的处理
     */
    user->ifindex = ifindex;
    if ((NPD_DHCPv6_TYPE_ADVERTISE == type) || (NPD_DHCPv6_TYPE_REPLY == type))
    {
    	temp = (unsigned char *)dhcpv6_snp_get_option(packet, DHCPv6_IANA);
		if(!temp){
			temp = (unsigned char *)dhcpv6_snp_get_option(packet, DHCPv6_STATUS_CODE);
			if(temp)
				status_g = 1;
		}
		if(temp && status_g){
			if(0 == temp[1]){
				log_debug("DHCPv6 REPLY for :success\n");
				item = (NPD_DHCPv6_SNP_TBL_ITEM_T *)dhcpv6_snp_tbl_item_find(user);
				if(item){
					memset(user->ipv6_addr, 0 , 16);
					memcpy(user->ipv6_addr, item ->ipv6_addr, 16);
					user->lease_time = item ->lease_time;
				}
			}else{
				log_debug("DHCPv6 REPLY for status code : %d\n",temp[1]);
				return DHCP_SNP_RETURN_CODE_ERROR;
			}
			return DHCP_SNP_RETURN_CODE_OK;
		}else if(temp){
			memcpy(user->ipv6_addr, temp+16 ,16);
			temp = (unsigned char*)(temp+36);
			memcpy((char*)&(user->lease_time), temp, 4);
			user->lease_time = ntohl(user->lease_time);
		}else{
			syslog_ax_dhcp_snp_err("couldn't get lease-time option from packet\n");
			return DHCP_SNP_RETURN_CODE_ERROR;
		}
    }
	else if(NPD_DHCPv6_TYPE_CONFIRM == type || NPD_DHCPv6_TYPE_RELEASE == type) {
		temp = (unsigned char *)dhcpv6_snp_get_option(packet, DHCPv6_IANA);
		memcpy(user->ipv6_addr, temp+16 ,16);
		user->lease_time = NPD_DHCP_SNP_REQUEST_TIMEOUT; 
	}
	else {
		memset(user->ipv6_addr, 0, 16);
		user->lease_time = NPD_DHCP_SNP_REQUEST_TIMEOUT;
    }

    return DHCP_SNP_RETURN_CODE_OK;
}
/**********************************************************************************
 *dhcp_snp_get_item_from_pkt()
 *
 *	DESCRIPTION:
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex
 *		NPD_DHCP_MESSAGE_T *packet
 *
 *	OUTPUTS:
 *		NPD_DHCP_SNP_USER_ITEM_T *user
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK
 *		DHCP_SNP_RETURN_CODE_ERROR
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL
 *
 ***********************************************************************************/
unsigned int dhcp_snp_get_item_from_pkt
(
	unsigned short vlanid,
	unsigned int ifindex,
	enum dhcp_packet_type type,
	NPD_DHCP_MESSAGE_T *packet,
	NPD_DHCP_SNP_USER_ITEM_T *user
)
{
    unsigned char *temp = NULL;

    if ((packet == NULL) || (user == NULL)) {
		syslog_ax_dhcp_snp_err("dhcp snp get item from pkt error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
    }
    
    user->vlanId    = vlanid;
    user->haddr_len = packet->hlen;

    memcpy(user->chaddr, packet->chaddr, NPD_DHCP_SNP_MAC_ADD_LEN);

    /* 为了支持从RELAY到本地SERVER申请IP地址，同时在本地支持SNOOPING的处理
     * 添加对没有通过用户MAC+VLAN定位到用户接口时从数据包取接口的处理，同时
     * 支持对响应数据包不带接口的处理
     */
    user->ifindex = ifindex;
    if (NPD_DHCP_BOOTREPLY == packet->op)
    {
		user->ip_addr = ntohl(packet->yiaddr);

		temp = (unsigned char *)dhcp_snp_get_option(packet, DHCP_LEASE_TIME);
		if (!temp)
		{
			syslog_ax_dhcp_snp_err("couldn't get lease-time option from packet\n");
			return DHCP_SNP_RETURN_CODE_ERROR;
		}else {
			memcpy(&(user->lease_time), temp, 4);
			user->lease_time = ntohl(user->lease_time);
		}
    }
	else if((NPD_DHCP_BOOTREQUEST == packet->op) &&(NPD_DHCP_INFORM == type)) {
		user->ip_addr = ntohl(packet->ciaddr);
		user->lease_time = NPD_DHCP_SNP_REQUEST_TIMEOUT;
	}
	else {
		user->ip_addr    = 0;
		user->lease_time = NPD_DHCP_SNP_REQUEST_TIMEOUT;
    }

    return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *dhcp_snp_discovery_process()
 *
 *	DESCRIPTION:
 *		destroy DHCP Snooping packet receive
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex,
 *		NPD_DHCP_MESSAGE_T *dhcp
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK 		- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL - error, parameter is null
 ***********************************************************************************/
unsigned int dhcp_snp_solicit_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCPv6_MESSAGE_T *dhcp,
    unsigned char *mac_2
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	NPD_DHCPv6_SNP_USER_ITEM_T user;
	NPD_DHCPv6_SNP_TBL_ITEM_T *item = NULL;

	syslog_ax_dhcp_snp_dbg("receive DHCPv6 SOLICIT packet from vlan %d ifindex %d\n", vlanid, ifindex);
	if (dhcp == NULL) {
		syslog_ax_dhcp_snp_err("dhcp snp solicit process error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	memset(&user, 0, sizeof(NPD_DHCPv6_SNP_USER_ITEM_T));
	ret = dhcpv6_snp_get_item_from_pkt(vlanid, ifindex, NPD_DHCPv6_TYPE_SOLICIT, dhcp, &user, mac_2);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("get item value from packet error, ret %x\n", ret);		
		return ret;
	}

	item = (NPD_DHCPv6_SNP_TBL_ITEM_T *)dhcpv6_snp_tbl_item_find(&user);
	if (item == NULL) {
		syslog_ax_dhcp_snp_dbg("no found item from dhcp snooping hash table, then insert a new.\n");
		user.state = NPD_DHCP_SNP_BIND_STATE_REQUEST;
		if (!dhcpv6_snp_tbl_item_insert(&user))
		{
			syslog_ax_dhcp_snp_err("insert item to table error\n"); 		
			return DHCP_SNP_RETURN_CODE_ERROR;
		}
	}

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *dhcp_snp_advertise_process()
 *
 *	DESCRIPTION:
 *		destroy DHCP Snooping packet receive
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex,
 *		NPD_DHCP_MESSAGE_T *dhcp
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 ***********************************************************************************/
unsigned int dhcp_snp_advertise_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCPv6_MESSAGE_T *dhcp, 
	unsigned char * dhcp_buffr, 
	unsigned long buffr_len,
	int fd,
	unsigned char* mac_2
)
{
#if 0
/* dhcp broadcast to unicast */
	int ret = -1;
	unsigned char det_mac[6] ={0xff, 0xff, 0xff, 0xff, 0xff,0xff}; /*det mac is boardcast*/

	if(0 == memcmp(dhcp_buffr,det_mac, 6)) {
		memcpy(dhcp_buffr, &(dhcp->chaddr), 6);
		if(fd < 0 || !dhcp_buffr){
			if(fd < 0){
				syslog_ax_dhcp_snp_err("socket fd is err, receive offer packet from vlan %d ifindex %d\n", vlanid, ifindex);
			}else{
				syslog_ax_dhcp_snp_err("packet buffer is null, receive offer packet from vlan %d ifindex %d\n", vlanid, ifindex);
			}			
			return DHCP_SNP_RETURN_CODE_ERROR;			
		}
		ret = write(fd, dhcp_buffr, buffr_len);
		if(ret < 0) {
			syslog_ax_dhcp_snp_err("change to unist is error , receive offer packet from vlan %d ifindex %d \n", vlanid, ifindex);
			return DHCP_SNP_RETURN_CODE_ERROR;
		}
		syslog_ax_dhcp_snp_dbg("receive offer packet from socket %d vlan %d ifindex %d pscket len %d\n", fd, vlanid, ifindex,  buffr_len);
	} else {
		syslog_ax_dhcp_snp_dbg("receive offer packet from vlan %d ifindex %d\n", vlanid, ifindex);
	}
#endif	
	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *dhcp_snp_discovery_process()
 *
 *	DESCRIPTION:
 *		destroy DHCP Snooping packet receive
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex,
 *		NPD_DHCP_MESSAGE_T *dhcp
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, parameter is null
 ***********************************************************************************/
unsigned int dhcp_snp_discovery_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCP_MESSAGE_T *dhcp
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	NPD_DHCP_SNP_USER_ITEM_T user;
	NPD_DHCP_SNP_TBL_ITEM_T *item = NULL;

	syslog_ax_dhcp_snp_dbg("receive DHCP DISCOVER packet from vlan %d ifindex %d\n", vlanid, ifindex);
    if (dhcp == NULL) {
		syslog_ax_dhcp_snp_err("dhcp snp discovery process error, parameter is null\n");
        return DHCP_SNP_RETURN_CODE_PARAM_NULL;
    }

	memset(&user, 0, sizeof(NPD_DHCP_SNP_USER_ITEM_T));
	ret = dhcp_snp_get_item_from_pkt(vlanid, ifindex, NPD_DHCP_DISCOVER, dhcp, &user);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("get item value from packet error, ret %x\n", ret);		
		return ret;
	}

	item = (NPD_DHCP_SNP_TBL_ITEM_T *)dhcp_snp_tbl_item_find(&user);
	if (item == NULL) {
		syslog_ax_dhcp_snp_dbg("no found item from dhcp snooping hash table, then insert a new.\n");
		user.state = NPD_DHCP_SNP_BIND_STATE_REQUEST;
		if (!dhcp_snp_tbl_item_insert(&user))
		{
			syslog_ax_dhcp_snp_err("insert item to table error\n");			
			return DHCP_SNP_RETURN_CODE_ERROR;
		}
	}

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *dhcp_snp_offer_process()
 *
 *	DESCRIPTION:
 *		destroy DHCP Snooping packet receive
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex,
 *		NPD_DHCP_MESSAGE_T *dhcp
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 ***********************************************************************************/
unsigned int dhcp_snp_offer_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCP_MESSAGE_T *dhcp, 
	unsigned char * dhcp_buffr, 
	unsigned long buffr_len,
	int fd
)
{
#if 0
/* dhcp broadcast to unicast */
	int ret = -1;
	unsigned char det_mac[6] ={0xff, 0xff, 0xff, 0xff, 0xff,0xff}; /*det mac is boardcast*/

	if(0 == memcmp(dhcp_buffr,det_mac, 6)) {
		memcpy(dhcp_buffr, &(dhcp->chaddr), 6);
		if(fd < 0 || !dhcp_buffr){
			if(fd < 0){
				syslog_ax_dhcp_snp_err("socket fd is err, receive offer packet from vlan %d ifindex %d\n", vlanid, ifindex);
			}else{
				syslog_ax_dhcp_snp_err("packet buffer is null, receive offer packet from vlan %d ifindex %d\n", vlanid, ifindex);
			}			
			return DHCP_SNP_RETURN_CODE_ERROR;			
		}
		ret = write(fd, dhcp_buffr, buffr_len);
		if(ret < 0) {
			syslog_ax_dhcp_snp_err("change to unist is error , receive offer packet from vlan %d ifindex %d \n", vlanid, ifindex);
			return DHCP_SNP_RETURN_CODE_ERROR;
		}
		syslog_ax_dhcp_snp_dbg("receive offer packet from socket %d vlan %d ifindex %d pscket len %d\n", fd, vlanid, ifindex,  buffr_len);
	} else {
		syslog_ax_dhcp_snp_dbg("receive offer packet from vlan %d ifindex %d\n", vlanid, ifindex);
	}
#endif	
	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *dhcpv6_snp_confirm_process()
 *
 *	DESCRIPTION:
 *		destroy DHCP Snooping packet receive
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex,
 *		NPD_DHCP_MESSAGE_T *dhcp
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, parameter is null
 ***********************************************************************************/
unsigned int dhcpv6_snp_confirm_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCPv6_MESSAGE_T *dhcp,
	unsigned char* mac_2
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	NPD_DHCPv6_SNP_USER_ITEM_T user;
	NPD_DHCPv6_SNP_TBL_ITEM_T *item = NULL;

	log_debug("recv DHCPv6 CONFIRM packet vlan %d ifindex %d\n", vlanid, ifindex);
    if (dhcp == NULL) {
		log_error("%s: parameter null.\n", __func__);
        return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	memset(&user, 0, sizeof(NPD_DHCPv6_SNP_USER_ITEM_T));
	ret = dhcpv6_snp_get_item_from_pkt(vlanid, ifindex,NPD_DHCPv6_TYPE_CONFIRM, dhcp, &user,mac_2);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		log_error("get item value from packet error, ret %x\n", ret);
		return ret;
	}

	item = (NPD_DHCPv6_SNP_TBL_ITEM_T *)dhcpv6_snp_tbl_item_find(&user);
	if (!item) {
		
		log_debug("request: no item then insert %s %s\n", 
			mac2str(user.chaddr), u128ip2str(user.ipv6_addr));
		
		user.state = NPD_DHCP_SNP_BIND_STATE_REQUEST;
		if (!dhcpv6_snp_tbl_item_insert(&user))
		{
			log_error("insert item to table error\n");
			return DHCP_SNP_RETURN_CODE_ERROR;
		}
	}

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *dhcpv6_snp_request_process()
 *
 *	DESCRIPTION:
 *		destroy DHCP Snooping packet receive
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex,
 *		NPD_DHCP_MESSAGE_T *dhcp
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, parameter is null
 ***********************************************************************************/
unsigned int dhcpv6_snp_request_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCPv6_MESSAGE_T *dhcp,
	unsigned char *mac_2
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	NPD_DHCPv6_SNP_USER_ITEM_T user;
	NPD_DHCPv6_SNP_TBL_ITEM_T *item = NULL;

    if (dhcp == NULL) {
		log_error("%s: parameter null.\n", __func__);
        return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	memset(&user, 0, sizeof(NPD_DHCPv6_SNP_USER_ITEM_T));
	ret = dhcpv6_snp_get_item_from_pkt(vlanid, ifindex,NPD_DHCPv6_TYPE_REQUEST, dhcp, &user, mac_2);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		log_error("get item value from packet error, ret %x\n", ret);
		return ret;
	}

	item = (NPD_DHCPv6_SNP_TBL_ITEM_T *)dhcpv6_snp_tbl_item_find(&user);
	if (!item) {
		
		log_debug("request: no item then insert %s %s\n", 
			mac2str(user.chaddr), u128ip2str(user.ipv6_addr));
		
		user.state = NPD_DHCP_SNP_BIND_STATE_REQUEST;
		if (!dhcpv6_snp_tbl_item_insert(&user))
		{
			log_error("insert item to table error\n");
			return DHCP_SNP_RETURN_CODE_ERROR;
		}
	}

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *dhcp_snp_reply_process()
 *
 *	DESCRIPTION:
 *		destroy DHCP Snooping packet receive
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex,
 *		NPD_DHCP_MESSAGE_T *dhcp
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, parameter is null
 ***********************************************************************************/
unsigned int dhcp_snp_reply_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCPv6_MESSAGE_T *dhcp,
	struct dhcp_snp_listener *node,
	unsigned char* mac_2
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	NPD_DHCPv6_SNP_USER_ITEM_T user;
	NPD_DHCPv6_SNP_TBL_ITEM_T *item = NULL;
	NPD_DHCPv6_SNP_TBL_ITEM_T *del_item = NULL;

	if (!node || !dhcp) {
		log_error("%s: parameter null.\n", __func__);
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}
	
	log_debug("interface %s recv REPLY packet, vlan %d\n", node->ifname, vlanid);

	memset(&user, 0, sizeof(NPD_DHCPv6_SNP_USER_ITEM_T));
	ret = dhcpv6_snp_get_item_from_pkt(vlanid, ifindex,NPD_DHCPv6_TYPE_REPLY, dhcp, &user, mac_2);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		log_error("get item value from packet error, ret %x\n", ret);
		return ret;
	}

	item = (NPD_DHCPv6_SNP_TBL_ITEM_T *)dhcpv6_snp_tbl_item_find(&user);
	if (item == NULL) {
		return DHCP_SNP_RETURN_CODE_OK; /*from trust to trust interface*/
	}
	
    if (item->bind_type == NPD_DHCP_SNP_BIND_TYPE_STATIC) {
		/* maybe here need add some codes */
        return DHCP_SNP_RETURN_CODE_OK;
    }

	#if 0
	if (item->ip_addr && (del_item = dhcp_snp_tbl_item_find_by_ip(item->ip_addr))) {

		/* delete static arp */
		if (node->no_arp) {
			if((0xFFFF == del_item->vlanId)&&(NPD_DHCP_SNP_BIND_STATE_BOUND == del_item->state)) {
				dhcp_snp_netlink_do_ipneigh(DHCPSNP_RTNL_IPNEIGH_DEL_E,  \
													del_item->ifindex, del_item->ip_addr, del_item->chaddr);
				dhcp_snp_listener_handle_host_ebtables(del_item->chaddr, del_item->ip_addr, DHCPSNP_EBT_DEL_E);
			}
		}
		
	//	dhcp_snp_tbl_item_delete_iphash(del_item);
	}
	#endif
	
	user.state = NPD_DHCP_SNP_BIND_STATE_BOUND;
	ret = dhcpv6_snp_tbl_refresh_bind(&user, item, node);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		log_error("refresh bind table item value error, ret %x\n", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	pthread_mutex_lock(&mutexsnpunsolve);	
	if(strlen(item->ipv6_addr))
	dhcpv6_snp_initiative_notify_asd(item->chaddr, item->ipv6_addr);	
	pthread_mutex_unlock(&mutexsnpunsolve);	

	if (dhcp_snp_notify_portal_enable) {
		dhcpv6_snp_notify_to_protal(item->ipv6_addr, item->chaddr);
	}
	
	
	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *dhcp_snp_request_process()
 *
 *	DESCRIPTION:
 *		destroy DHCP Snooping packet receive
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex,
 *		NPD_DHCP_MESSAGE_T *dhcp
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, parameter is null
 ***********************************************************************************/
unsigned int dhcp_snp_request_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCP_MESSAGE_T *dhcp
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	NPD_DHCP_SNP_USER_ITEM_T user;
	NPD_DHCP_SNP_TBL_ITEM_T *item = NULL;

	log_debug("recv DHCP REQUEST packet vlan %d ifindex %d\n", vlanid, ifindex);
    if (dhcp == NULL) {
		log_error("%s: parameter null.\n", __func__);
        return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	memset(&user, 0, sizeof(NPD_DHCP_SNP_USER_ITEM_T));
	ret = dhcp_snp_get_item_from_pkt(vlanid, ifindex,NPD_DHCP_REQUEST, dhcp, &user);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		log_error("get item value from packet error, ret %x\n", ret);
		return ret;
	}

	item = (NPD_DHCP_SNP_TBL_ITEM_T *)dhcp_snp_tbl_item_find(&user);
	if (!item) {
		
		log_debug("request: no item then insert %s %s\n", 
			mac2str(user.chaddr), u32ip2str(user.ip_addr));
		
		user.state = NPD_DHCP_SNP_BIND_STATE_REQUEST;
		if (!dhcp_snp_tbl_item_insert(&user))
		{
			log_error("insert item to table error\n");
			return DHCP_SNP_RETURN_CODE_ERROR;
		}
	}

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *dhcp_snp_ack_process()
 *
 *	DESCRIPTION:
 *		destroy DHCP Snooping packet receive
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex,
 *		NPD_DHCP_MESSAGE_T *dhcp
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, parameter is null
 ***********************************************************************************/
unsigned int dhcp_snp_ack_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCP_MESSAGE_T *dhcp,
	struct dhcp_snp_listener *node	
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	NPD_DHCP_SNP_USER_ITEM_T user;
	NPD_DHCP_SNP_TBL_ITEM_T *item = NULL;
	NPD_DHCP_SNP_TBL_ITEM_T *del_item = NULL;

	if (!node || !dhcp) {
		log_error("%s: parameter null.\n", __func__);
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}
	
	log_debug("interface %s recv ACK packet, vlan %d\n", node->ifname, vlanid);

	memset(&user, 0, sizeof(NPD_DHCP_SNP_USER_ITEM_T));
	ret = dhcp_snp_get_item_from_pkt(vlanid, ifindex,NPD_DHCP_ACK, dhcp, &user);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		log_error("get item value from packet error, ret %x\n", ret);
		return ret;
	}

	item = (NPD_DHCP_SNP_TBL_ITEM_T *)dhcp_snp_tbl_item_find(&user);
	if (item == NULL) {
		return DHCP_SNP_RETURN_CODE_OK; /*from trust to trust interface*/
	}
	
    if (item->bind_type == NPD_DHCP_SNP_BIND_TYPE_STATIC) {
		/* maybe here need add some codes */
        return DHCP_SNP_RETURN_CODE_OK;
    }

	#if 0
	if (item->ip_addr && (del_item = dhcp_snp_tbl_item_find_by_ip(item->ip_addr))) {

		/* delete static arp */
		if (node->no_arp) {
			if((0xFFFF == del_item->vlanId)&&(NPD_DHCP_SNP_BIND_STATE_BOUND == del_item->state)) {
				dhcp_snp_netlink_do_ipneigh(DHCPSNP_RTNL_IPNEIGH_DEL_E,  \
													del_item->ifindex, del_item->ip_addr, del_item->chaddr);
				dhcp_snp_listener_handle_host_ebtables(del_item->chaddr, del_item->ip_addr, DHCPSNP_EBT_DEL_E);
			}
		}
		
	//	dhcp_snp_tbl_item_delete_iphash(del_item);
	}
	#endif
	
	user.state = NPD_DHCP_SNP_BIND_STATE_BOUND;
	ret = dhcp_snp_tbl_refresh_bind(&user, item, node);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		log_error("refresh bind table item value error, ret %x\n", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}

	pthread_mutex_lock(&mutexsnpunsolve);	
	dhcp_snp_initiative_notify_asd(item->chaddr, item->ip_addr);	
	pthread_mutex_unlock(&mutexsnpunsolve);	

	if (dhcp_snp_notify_portal_enable) {
		dhcp_snp_notify_to_protal(item->ip_addr, item->chaddr);
	}
	
	
	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *dhcp_snp_nack_process()
 *
 *	DESCRIPTION:
 *		destroy DHCP Snooping packet receive
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex,
 *		NPD_DHCP_MESSAGE_T *dhcp
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, parameter is null
 ***********************************************************************************/
unsigned int dhcp_snp_nack_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCP_MESSAGE_T *dhcp
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	NPD_DHCP_SNP_USER_ITEM_T user;
	NPD_DHCP_SNP_TBL_ITEM_T *item = NULL;

	syslog_ax_dhcp_snp_dbg("receive DHCP NACK packet from vlan %d ifindex %d\n", vlanid, ifindex);
	if (dhcp == NULL) {
		syslog_ax_dhcp_snp_err("dhcp snp nack process error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	ret = dhcp_snp_get_item_from_pkt(vlanid, ifindex,NPD_DHCP_NAK, dhcp, &user);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("get item value from packet error, ret %x\n", ret);
		return ret;
	}

	item = (NPD_DHCP_SNP_TBL_ITEM_T *)dhcp_snp_tbl_item_find(&user);
	if (item == NULL) {
		return DHCP_SNP_RETURN_CODE_OK; /*from trust to trust interface*/
	}	
	
	if (item->bind_type == NPD_DHCP_SNP_BIND_TYPE_STATIC) {
		return DHCP_SNP_RETURN_CODE_OK;
	}

	ret = dhcp_snp_tbl_identity_item(item, &user);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		return DHCP_SNP_RETURN_CODE_OK;
	}

	if (item->bind_type == NPD_DHCP_SNP_BIND_TYPE_DYNAMIC) {
		ret = dhcp_snp_tbl_item_delete(item);
		if (DHCP_SNP_RETURN_CODE_OK != ret) {
			syslog_ax_dhcp_snp_err("delete item from bind table error, ret %x\n", ret);
			return DHCP_SNP_RETURN_CODE_ERROR;
		}
	}

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *dhcpv6_snp_release_process()
 *
 *	DESCRIPTION:
 *		release DHCP Snooping packet receive
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex,
 *		NPD_DHCP_MESSAGE_T *dhcp
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, parameter is null
 ***********************************************************************************/
unsigned int dhcpv6_snp_release_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCPv6_MESSAGE_T *dhcp,
	struct dhcp_snp_listener *node,
	unsigned char* mac_2
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	NPD_DHCPv6_SNP_USER_ITEM_T user;
	NPD_DHCPv6_SNP_TBL_ITEM_T *item = NULL;
	char ifname[IF_NAMESIZE]={0};
	char command[128] = {0};

	syslog_ax_dhcp_snp_dbg("receive DHCPv6 RELEASE packet from vlan %d ifindex %d\n", vlanid, ifindex);
	if (dhcp == NULL) {
		syslog_ax_dhcp_snp_err("dhcp snp release process error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	ret = dhcpv6_snp_get_item_from_pkt(vlanid, ifindex,NPD_DHCPv6_TYPE_RELEASE, dhcp, &user, mac_2);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("get item value from packet error, ret %x\n", ret);
		return ret;
	}

	item = (NPD_DHCPv6_SNP_TBL_ITEM_T *)dhcpv6_snp_tbl_item_find(&user);
	if (item == NULL) {
		return DHCP_SNP_RETURN_CODE_OK; /*from trust to trust interface*/
	}	
	
	if (item->bind_type == NPD_DHCP_SNP_BIND_TYPE_STATIC) {
		return DHCP_SNP_RETURN_CODE_OK;
	}

	ret = dhcpv6_snp_tbl_identity_item(item, &user);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		return DHCP_SNP_RETURN_CODE_OK;
	}

	if (item->bind_type == NPD_DHCP_SNP_BIND_TYPE_DYNAMIC) {
		/* special branch for cpu interface or RGMII interface
		  * delete previous ip neigh item
		  */
	/*
		if (node->no_arp) {
			if((0xFFFF == item->vlanId)&&(NPD_DHCP_SNP_BIND_STATE_BOUND == item->state)) {
				ret = dhcp_snp_netlink_do_ipneigh(DHCPSNP_RTNL_IPNEIGH_DEL_E,  \
													item->ifindex, item->ip_addr, item->chaddr);
				if(DHCP_SNP_RETURN_CODE_OK != ret) {
					syslog_ax_dhcp_snp_err("dhcp snp release item del ip neigh error %x\n", ret);
				}
				dhcp_snp_listener_handle_host_ebtables(item->chaddr, item->ip_addr, DHCPSNP_EBT_DEL_E);
			}
		}
		if (node->add_router) {//delete router to host,next jump is the interface opening dhcp-snooping
			if((0xFFFF == item->vlanId)&&(NPD_DHCP_SNP_BIND_STATE_BOUND == item->state)) {
				
				if(!if_indextoname(item->ifindex, ifname)) {
					syslog_ax_dhcp_snp_err("no intf found as idx %d netlink error !\n", item->ifindex);
					return DHCP_SNP_RETURN_CODE_ERROR;
				}
				dhcp_snp_netlink_add_static_route(DHCPSNP_RTNL_STATIC_ROUTE_DEL_E,  \
													item->ifindex, item->ip_addr);
				//sprintf(command,"sudo route del -host %u.%u.%u.%u dev %s",(item->ip_addr>>24)&0xff,\
				//(item->ip_addr>>16)&0xff,(item->ip_addr>>8)&0xff,(item->ip_addr>>0)&0xff,ifname);
				//system(command);
				}
		}
	*/	
		ret = dhcpv6_snp_tbl_item_delete(item);
		if (DHCP_SNP_RETURN_CODE_OK != ret) {
			syslog_ax_dhcp_snp_err("delete item from bind table error, ret %x\n", ret);
			return DHCP_SNP_RETURN_CODE_ERROR;
		}
	}

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *dhcp_snp_release_process()
 *
 *	DESCRIPTION:
 *		release DHCP Snooping packet receive
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex,
 *		NPD_DHCP_MESSAGE_T *dhcp
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, parameter is null
 ***********************************************************************************/
unsigned int dhcp_snp_release_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCP_MESSAGE_T *dhcp,
	struct dhcp_snp_listener *node	
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	NPD_DHCP_SNP_USER_ITEM_T user;
	NPD_DHCP_SNP_TBL_ITEM_T *item = NULL;
	char ifname[IF_NAMESIZE]={0};
	char command[128] = {0};

	syslog_ax_dhcp_snp_dbg("receive DHCP RELEASE packet from vlan %d ifindex %d\n", vlanid, ifindex);
	if (dhcp == NULL) {
		syslog_ax_dhcp_snp_err("dhcp snp release process error, parameter is null\n");
		return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	ret = dhcp_snp_get_item_from_pkt(vlanid, ifindex,NPD_DHCP_RELEASE, dhcp, &user);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("get item value from packet error, ret %x\n", ret);
		return ret;
	}

	item = (NPD_DHCP_SNP_TBL_ITEM_T *)dhcp_snp_tbl_item_find(&user);
	if (item == NULL) {
		return DHCP_SNP_RETURN_CODE_OK; /*from trust to trust interface*/
	}	
	
	if (item->bind_type == NPD_DHCP_SNP_BIND_TYPE_STATIC) {
		return DHCP_SNP_RETURN_CODE_OK;
	}

	ret = dhcp_snp_tbl_identity_item(item, &user);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		return DHCP_SNP_RETURN_CODE_OK;
	}

	if (item->bind_type == NPD_DHCP_SNP_BIND_TYPE_DYNAMIC) {
		/* special branch for cpu interface or RGMII interface
		  * delete previous ip neigh item
		  */
		if (node->no_arp) {
			if((0xFFFF == item->vlanId)&&(NPD_DHCP_SNP_BIND_STATE_BOUND == item->state)) {
				ret = dhcp_snp_netlink_do_ipneigh(DHCPSNP_RTNL_IPNEIGH_DEL_E,  \
													item->ifindex, item->ip_addr, item->chaddr);
				if(DHCP_SNP_RETURN_CODE_OK != ret) {
					syslog_ax_dhcp_snp_err("dhcp snp release item del ip neigh error %x\n", ret);
				}
				dhcp_snp_listener_handle_host_ebtables(item->chaddr, item->ip_addr, DHCPSNP_EBT_DEL_E);
			}
		}
		if (node->add_router) {//delete router to host,next jump is the interface opening dhcp-snooping
			if((0xFFFF == item->vlanId)&&(NPD_DHCP_SNP_BIND_STATE_BOUND == item->state)) {
				
				if(!if_indextoname(item->ifindex, ifname)) {
					syslog_ax_dhcp_snp_err("no intf found as idx %d netlink error !\n", item->ifindex);
					return DHCP_SNP_RETURN_CODE_ERROR;
				}
				dhcp_snp_netlink_add_static_route(DHCPSNP_RTNL_STATIC_ROUTE_DEL_E,  \
													item->ifindex, item->ip_addr);
				//sprintf(command,"sudo route del -host %u.%u.%u.%u dev %s",(item->ip_addr>>24)&0xff,\
				//(item->ip_addr>>16)&0xff,(item->ip_addr>>8)&0xff,(item->ip_addr>>0)&0xff,ifname);
				//system(command);
				}
		}
		
		ret = dhcp_snp_tbl_item_delete(item);
		if (DHCP_SNP_RETURN_CODE_OK != ret) {
			syslog_ax_dhcp_snp_err("delete item from bind table error, ret %x\n", ret);
			return DHCP_SNP_RETURN_CODE_ERROR;
		}
	}

	return DHCP_SNP_RETURN_CODE_OK;
}

/**********************************************************************************
 *dhcp_snp_inform_process()
 *
 *	DESCRIPTION:
 *		DHCP Inform packet receive
 *
 *	INPUTS:
 *		unsigned short vlanid,
 *		unsigned int ifindex,
 *		NPD_DHCP_MESSAGE_T *dhcp
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, parameter is null
 ***********************************************************************************/
unsigned int dhcp_snp_inform_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	NPD_DHCP_MESSAGE_T *dhcp,
	struct dhcp_snp_listener *node	
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	NPD_DHCP_SNP_USER_ITEM_T user;
	NPD_DHCP_SNP_TBL_ITEM_T *item = NULL;

	syslog_ax_dhcp_snp_dbg("receive DHCP INFORM packet from vlan %d ifindex %d\n", vlanid, ifindex);
    if (dhcp == NULL) {
		syslog_ax_dhcp_snp_err("dhcp snp inform process error, parameter is null\n");
        return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	memset(&user, 0, sizeof(NPD_DHCP_SNP_USER_ITEM_T));
	ret = dhcp_snp_get_item_from_pkt(vlanid, ifindex,NPD_DHCP_INFORM, dhcp, &user);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("DHCP inform get item value from packet error, ret %x\n", ret);
		return ret;
	}

	item = (NPD_DHCP_SNP_TBL_ITEM_T *)dhcp_snp_tbl_item_find(&user);
	if (item == NULL) {
		return DHCP_SNP_RETURN_CODE_OK; 
	}
	
    if (item->bind_type == NPD_DHCP_SNP_BIND_TYPE_STATIC) {
		/* maybe here need add some codes */
        return DHCP_SNP_RETURN_CODE_OK;
    }

	/* dynamic user already bound */
	if((NPD_DHCP_SNP_BIND_TYPE_DYNAMIC == item->bind_type) &&
		(NPD_DHCP_SNP_BIND_STATE_BOUND == item->state)) {
		if(item->ip_addr != user.ip_addr) {
			syslog_ax_dhcp_snp_warn("inform found conflict dynamic bound item %d.%d.%d.%d vs %d.%d.%d.%d(new)\n", \
						(item->ip_addr>>24)&0xFF,(item->ip_addr>>16)&0xFF,(item->ip_addr>>8)&0xFF,item->ip_addr&0xFF, \
						(user.ip_addr>>24)&0xFF,(user.ip_addr>>16)&0xFF,(user.ip_addr>>8)&0xFF,user.ip_addr&0xFF);
		}
		return DHCP_SNP_RETURN_CODE_OK;
	}
	
	user.state = NPD_DHCP_SNP_BIND_STATE_BOUND;
	ret = dhcp_snp_tbl_refresh_bind(&user, item, node);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("DHCP inform refresh binding table item value error, ret %x\n", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	
	return DHCP_SNP_RETURN_CODE_OK;
}

unsigned short dhcp_snp_checksum
(
	void *addr,
	int count
)
{
	/* Compute Internet Checksum for "count" bytes
	*         beginning at location "addr".
	*/
	register int sum = 0;
	unsigned short *source = (unsigned short *)addr;

	while (count > 1)  
	{
		/*  This is the inner loop */
		sum += *source++;
		count -= 2;
	}

	/*  Add left-over byte, if any */
	if (count > 0) 
	{
		/* Make sure that the left-over byte is added correctly both
		* with little and big endian hosts */
		unsigned short tmp = 0;
		*(unsigned char *) (&tmp) = * (unsigned char *) source;
		sum += tmp;
	}
	/*  Fold 32-bit sum to 16 bits */
	while (sum >> 16)
	{
		sum = (sum & 0xffff) + (sum >> 16);
	}
	return ~sum;
}

/********************************************************************************************
 * 	dhcp_snp_packet_type_is_mc
 *
 *	DESCRIPTION:
 *             This function check out whether the packet is mcast or not.
 *
 *	INPUT:
 *             packetBuff - points to the packet's first buffer' head
 *	OUTPUT:
 *               NONE
 *	RETURNS:
 *              DHCP_SNP_TRUE - indicate the packet is mcast packet
 *              DHCP_SNP_FALSE - indicate the packet is not mcast packet
 *
 *	COMMENTS:
 *             NONE.
 *
 **********************************************************************************************/
unsigned long	dhcp_snp_packet_type_is_mc
(       
	unsigned char  *packetBuff
)
{
	ether_header_t  *layer2 = NULL;

	layer2 = (ether_header_t*)(packetBuff);

	if((layer2->dmac[0] & layer2->dmac[1] & layer2->dmac[2] & layer2->dmac[3] & layer2->dmac[4] & layer2->dmac[5]) 	\
		== 0xff) {
		return DHCP_SNP_TRUE;
	}
	else if(0x01 & layer2->dmac[0]) {
		return DHCP_SNP_TRUE;
	}
	else {
		return DHCP_SNP_FALSE;
	}
}

/**********************************************************************************
 *dhcp_snp_gratuitous_arp_process()
 *
 *	DESCRIPTION:
 *		Gratuitous ARP packet receive
 *
 *	INPUTS:
 *		vlanid - vlan id
 *		ifindex - interface ifindex
 *		packet  - packet buffer
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, parameter is null
 ***********************************************************************************/
unsigned int dhcp_snp_gratuitous_arp_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	char *packet,
	struct dhcp_snp_listener *node	
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	ether_header_t *l2 = NULL;
	arp_packet_t *arp = NULL;
	NPD_DHCP_SNP_USER_ITEM_T user;
	NPD_DHCP_SNP_TBL_ITEM_T *item = NULL;

	syslog_ax_dhcp_snp_dbg("receive gratuitous packet from vlan %d ifindex %d\n", vlanid, ifindex);
    if (NULL == packet) {
		syslog_ax_dhcp_snp_err("gratuitous arp process error, parameter is null\n");
        return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}
	l2 = (ether_header_t *)packet;
	arp = (arp_packet_t *)(l2 + 1);
	
	memset(&user, 0, sizeof(NPD_DHCP_SNP_USER_ITEM_T));
	user.ifindex = ifindex;
	user.vlanId = vlanid;
	user.haddr_len = arp->hwSize;
	memcpy(user.chaddr, arp->smac, NPD_DHCP_SNP_MAC_ADD_LEN);
	memcpy(&(user.ip_addr),arp->sip, 4);
	user.lease_time = NPD_DHCP_SNP_REQUEST_TIMEOUT;	
	if (dhcp_snp_u32ip_check(user.ip_addr)) {
		return DHCP_SNP_RETURN_CODE_OK;
	}
	
	item = (NPD_DHCP_SNP_TBL_ITEM_T *)dhcp_snp_tbl_item_find(&user);
	if (item == NULL) {
		return DHCP_SNP_RETURN_CODE_OK; 
	}
	
    if (item->bind_type == NPD_DHCP_SNP_BIND_TYPE_STATIC) {
		/* maybe here need add some codes */
        return DHCP_SNP_RETURN_CODE_OK;
    }

	/* dynamic user already bound */
	if((NPD_DHCP_SNP_BIND_TYPE_DYNAMIC == item->bind_type) &&
		(NPD_DHCP_SNP_BIND_STATE_BOUND == item->state)) {
		if(item->ip_addr != user.ip_addr) {
			syslog_ax_dhcp_snp_warn("gratuitous arp found conflict dynamic bound item %d.%d.%d.%d vs %d.%d.%d.%d(new)\n", \
						(item->ip_addr>>24)&0xFF,(item->ip_addr>>16)&0xFF,(item->ip_addr>>8)&0xFF,item->ip_addr&0xFF, \
						(user.ip_addr>>24)&0xFF,(user.ip_addr>>16)&0xFF,(user.ip_addr>>8)&0xFF,user.ip_addr&0xFF);
		}
		return DHCP_SNP_RETURN_CODE_OK;
	}

	if (item->ip_addr != user.ip_addr) {
		return DHCP_SNP_RETURN_CODE_OK;
	}
	
	user.state = NPD_DHCP_SNP_BIND_STATE_BOUND;
	ret = dhcp_snp_tbl_refresh_bind(&user, item, node);
	if (DHCP_SNP_RETURN_CODE_OK != ret) {
		syslog_ax_dhcp_snp_err("gratuitous arp refresh binding table item value error, ret %x\n", ret);
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	
	return DHCP_SNP_RETURN_CODE_OK;
}

unsigned int dhcp_snp_arp_request_process
(
	unsigned short vlanid,
	unsigned int ifindex,
	char *packet,
	int fd
)
{
	unsigned int ret = DHCP_SNP_RETURN_CODE_OK;
	ether_header_t *l2 = NULL;
	arp_packet_t *arp = NULL;
	NPD_DHCP_SNP_USER_ITEM_T user;
	NPD_DHCP_SNP_TBL_ITEM_T *item = NULL;
	struct arp_packet *reply = NULL;
	int i;

	if ((NULL == packet) || (fd < 0)) {
		syslog_ax_dhcp_snp_err("arp request process error, parameter is null\n");
        return DHCP_SNP_RETURN_CODE_PARAM_NULL;
	}

	if (!dhcp_snp_arp_proxy_enable) {
		syslog_ax_dhcp_snp_dbg("arp proxy is disabled.\n");
		return 0;
	}
	
	l2 = (ether_header_t *)packet;
	arp = (arp_packet_t *)(l2 + 1);
	memset(&user, 0, sizeof(NPD_DHCP_SNP_USER_ITEM_T));
	user.ifindex = ifindex;
	user.vlanId = vlanid;
	user.haddr_len = arp->hwSize;
	memcpy(user.chaddr, arp->dmac, NPD_DHCP_SNP_MAC_ADD_LEN);
	user.ip_addr = ((arp->dip[0] & 0xff) << 24) | ((arp->dip[1] & 0xff) << 16)
					| ((arp->dip[2] & 0xff) << 8) | ((arp->dip[3] & 0xff) << 0);
	syslog_ax_dhcp_snp_dbg("receive arp request: find mac by ip %u.%u.%u.%u(%#x)\n", 
					arp->dip[0], arp->dip[1], arp->dip[2], arp->dip[3], user.ip_addr);
	item = (NPD_DHCP_SNP_TBL_ITEM_T *)dhcp_snp_tbl_item_find_by_ip(user.ip_addr);
	if (NULL == item) {
		syslog_ax_dhcp_snp_warn("not find user mac by ip %u.%u.%u.%u\n", arp->dip[0], arp->dip[1], arp->dip[2], arp->dip[3]);
		return DHCP_SNP_RETURN_CODE_OK; 
	}
	reply = malloc(sizeof(struct arp_packet));
	if (!reply) {
		syslog_ax_dhcp_snp_err("arp request process error, malloc failed!\n");
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	memset(reply, 0, sizeof(struct arp_packet));
	syslog_ax_dhcp_snp_dbg("ARP Proxy reply arp request\n");
	memcpy(reply->dmac, arp->smac, ETH_ALEN);	/* dest MAC */
	memcpy(reply->smac, item->chaddr, ETH_ALEN);	/* src MAC */
	reply->etherType = htons(0x0806);			/* ARP protocol */
	reply->hwType	= htons(0x1); 				/* ethernet hardware */
	reply->protType = htons(0x0800); 			/* IP */
	reply->hwSize 	= 0x6;						/* hardware size */
	reply->protSize	= 0x4;						/* protocol size */
	reply->opCode 	= htons(0x2); 				/* arp reply */
	memcpy(reply->arpsmac, item->chaddr, ETH_ALEN);	/* sender MAC */
	for(i = 0; i < 4; i++) {						/* sender IP */
		reply->arpsip[i] = (user.ip_addr >> ((3-i)*8)) & 0xFF;
	}
	memcpy(reply->arpdmac, arp->smac, ETH_ALEN);		/* target MAC */
	memcpy(reply->arpdip, arp->sip, IP_ADDR_LEN);		/* target IP */
	ret = write(fd, reply, sizeof(struct arp_packet));
	if(ret < 0){
		syslog_ax_dhcp_snp_err("arp reply error, receive arp packet from vlan %d ifindex %d \n", vlanid, ifindex);
		if(reply){
			free(reply);
		}
		return DHCP_SNP_RETURN_CODE_ERROR;
	}
	if(reply) {
		free(reply);
	}
	return DHCP_SNP_RETURN_CODE_OK;
}
/**********************************************************************************
 *dhcp_snp_get_ifi()
 *
 *	DESCRIPTION:
 *		Gratuitous ARP packet receive
 *
 *	INPUTS:
 *		ifindex - interface ifindex
 *		mac  	- mac addr
 *		maclen 	- mac addr len
 *		lc_addr - ip addr
 *		ipln	- ip addr len
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		0			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, parameter is null
 ***********************************************************************************/
int dhcp_snp_get_ifi
(
	unsigned int ifindex, 
	char * mac,
	int macln, 
	struct in_addr *lc_addr, 
	int ipln
)
{
	int reqfd, n;
	struct ifreq req;
	char ifname[IF_NAMESIZE];

	reqfd = socket(AF_INET, SOCK_DGRAM, 0);
	memset(&req, 0, sizeof(struct ifreq));
	memset(ifname, 0, IF_NAMESIZE);
	
	if(!if_indextoname(ifindex, ifname)) {
		syslog_ax_dhcp_snp_err("if index to name fail\n");
		if(reqfd){
			close(reqfd);
		}
		return 1;
	}

	memset(&req, 0, sizeof(req));
	strcpy(req.ifr_name, ifname);
	/* 获取本地接口MAC地址*/
	if(ioctl(reqfd, SIOCGIFHWADDR, &req) != 0) {
		syslog_ax_dhcp_snp_err("ioctl get mac address fail\n");
		if(reqfd){
			close(reqfd);
		}
		return 1;
	}
	memcpy(mac, req.ifr_hwaddr.sa_data, macln);

	memset(&req, 0, sizeof(req));
	strcpy(req.ifr_name, ifname);
	/* 获取本地接口IP地址*/
	if(ioctl(reqfd, SIOCGIFADDR, &req) != 0) {		
		syslog_ax_dhcp_snp_err("ioctl get ip address fail\n");
		if(reqfd){
			close(reqfd);
		}
		return 1;
	}
	memcpy(lc_addr, &((struct sockaddr_in *)(&req.ifr_addr))->sin_addr, ipln);
	if(reqfd>=0)
	close(reqfd);
	return 0;
}      

/**********************************************************************************
 *dhcp_snp_get_ifi()
 *
 *	DESCRIPTION:
 *		Gratuitous ARP packet receive
 *
 *	INPUTS:
 *		ifindex - interface ifindex
 *		dmac  	- dmac addr
 *		dip 	- dip addr
 *
 *	OUTPUTS:
 *		NULL
 *
 *	RETURN VALUE:
 *		DHCP_SNP_RETURN_CODE_OK			- success
 *		DHCP_SNP_RETURN_CODE_ERROR			- fail
 *		DHCP_SNP_RETURN_CODE_PARAM_NULL	- error, parameter is null
 ***********************************************************************************/
int dhcp_snp_send_arp_solicit
(
	unsigned int ifindex,
	char *dmac,
	unsigned int dip
)
{
	int reqfd = -1, n = 0, i = 0;  
	unsigned char *smac = NULL;
	struct in_addr lc_addr, req_addr;
	struct sockaddr_ll reqsa, repsa;
	struct arp_packet *req = NULL;

	bzero(&reqsa, sizeof(reqsa));
	reqsa.sll_family = PF_PACKET;
	reqsa.sll_ifindex = ifindex;

	if((reqfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_RARP))) < 0) {
		syslog_ax_dhcp_snp_err("Error: create socket for arp fail\n");
	}

	smac = (char *)malloc(ETH_ALEN);
	memset(smac, 0, ETH_ALEN);
	
	if(dhcp_snp_get_ifi(ifindex, smac, ETH_ALEN, &lc_addr, IP_ADDR_LEN)) {
		syslog_ax_dhcp_snp_err("Error: Get host's information failed\n");
	}
	/*
	* Build up ARP solicit packet
	*/
	req = malloc(sizeof(struct arp_packet));
	memset(req, 0, sizeof(struct arp_packet));

	/* layer 2 */
	memcpy(req->dmac,dmac,ETH_ALEN);
	memcpy(req->smac,smac,ETH_ALEN);
	req->etherType = htons(0x0806);

	/* layer 3 */
	req->hwType 		= htons(0x1); /* ethernet hardware */
	req->protType	= htons(0x0800); /* IP */
	req->hwSize 		= 0x6;
	req->protSize 	= 0x4;
	req->opCode 		= htons(0x1); /* request */
	memcpy(req->arpsmac,smac,ETH_ALEN);
	memset(req->arpdmac,0,ETH_ALEN);
	for(i = 0; i < 4; i++) {
		req->arpsip[i] = (lc_addr.s_addr >>((3-i)*8)) & 0xFF;
		req->arpdip[i] = (dip >>((3-i)*8)) & 0xFF;
	}

	if((n = sendto(reqfd, req, sizeof(*req), 0, (struct sockaddr *)&reqsa, sizeof(reqsa))) <= 0) {
		syslog_ax_dhcp_snp_err("send to user arp solicit fail\n");
	}

	close(reqfd);
	if(req){
		free(req);
		req=NULL;
	}
	if(smac){
		free(smac);
		smac=NULL;
	}
	return DHCP_SNP_RETURN_CODE_OK;
}


int dhcp_snp_init_tipc_sock(void)
{
	int sockfd = -1;
	
	sockfd = socket(AF_TIPC, SOCK_RDM, 0);
	if (sockfd < 0) {
		syslog_ax_dhcp_snp_err("init TIPC sock faild: %m\n");
		return -1;
	}
/*
	if ((flags = fcntl(sockfd, F_GETFL)) < 0) {
		fprintf(stderr, "fcntl(F_GETFL) failed for fd %d: %s",
			sockfd, strerror(errno));
		return -1;	
	}	
	if (fcntl(sockfd, F_SETFL, (flags | O_NONBLOCK)) < 0) {
		fprintf(stderr, "fcntl failed setting fd %d non-blocking: %s",
			sockfd, strerror(errno));
		return -1;
	}
*/
	return sockfd;
}
/**********************************************************************************
 *  dhcpsnp_notify_to_protal
 *
 *	DESCRIPTION:
 * 		when receive ACK, notify protal client IP and MAC
 *
 *	INPUT:
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		0   ->  success
 *		-1  ->  failed
 *		
 **********************************************************************************/
int dhcpv6_snp_notify_to_protal(char* userip, uint8_t *usermac)
{
#define MACAUTH_SERVER_TYPE			0x4000
#define MACAUTH_SERVER_INSTANCE		0x10000

	struct dhcpv6_sta_msg {
			uint8_t family;
			uint8_t usermac[ETH_ALEN];
			union {
			uint32_t userip;
			struct in6_addr user_ipv6;
			} addr;
	};


	static int sock_fd = -1;

	size_t size = 0;
	struct sockaddr_tipc servaddr;
	struct dhcpv6_sta_msg msg;

	if (!usermac) {
		return -1;
	}
	
	if (sock_fd < 0) {
		sock_fd = dhcp_snp_init_tipc_sock();
	}
	
	if (sock_fd >= 0) {
		/* server address */
		memset(&servaddr, 0, sizeof(struct sockaddr_tipc));
		servaddr.family = AF_TIPC;
		servaddr.addrtype = TIPC_ADDR_NAMESEQ;
		servaddr.addr.nameseq.type = MACAUTH_SERVER_TYPE;
		servaddr.addr.nameseq.lower = MACAUTH_SERVER_INSTANCE;
		servaddr.addr.nameseq.upper = MACAUTH_SERVER_INSTANCE;
		servaddr.scope = TIPC_CLUSTER_SCOPE;

		/* msg */
		memset(&msg, 0, sizeof(msg));
		memcpy(msg.addr.user_ipv6.s6_addr, userip, 16);
		memcpy(msg.usermac, usermac, ETH_ALEN);
		msg.family = 6;

		syslog_ax_dhcp_snp_dbg("msg notify to protal(ipv6) %s %02x:%02x:%02x:%02x:%02x:%02x\n",
			u128ip2str(msg.addr.user_ipv6.s6_addr), 
			usermac[0], usermac[1], usermac[2], usermac[3], usermac[4], usermac[5]);

		size = sendto(sock_fd, &msg, sizeof(msg), MSG_DONTWAIT, 
			(struct sockaddr *)(&servaddr), sizeof(servaddr));
		if (size != sizeof(msg)) {
			syslog_ax_dhcp_snp_dbg("%s %s %02x:%02x:%02x:%02x:%02x:%02x :%m\n",
				"send msg to portal failed",u128ip2str(userip), 
				usermac[0], usermac[1], usermac[2], usermac[3], usermac[4], usermac[5]);
		}
	}
	
	return 0;
}

/**********************************************************************************
 *  dhcpsnp_notify_to_protal
 *
 *	DESCRIPTION:
 * 		when receive ACK, notify protal client IP and MAC
 *
 *	INPUT:
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		0   ->  success
 *		-1  ->  failed
 *		
 **********************************************************************************/
int dhcp_snp_notify_to_protal(uint32_t userip, uint8_t *usermac)
{
#define MACAUTH_SERVER_TYPE			0x4000
#define MACAUTH_SERVER_INSTANCE		0x10000

	struct dhcp_sta_msg {
			uint8_t family;
			uint8_t usermac[ETH_ALEN];
			union {
			uint32_t userip;
			struct in6_addr user_ipv6;
			} addr;
	};

	static int sock_fd = -1;

	size_t size = 0;
	struct sockaddr_tipc servaddr;
	struct dhcp_sta_msg msg;

	if (!usermac) {
		return -1;
	}
	
	if (sock_fd < 0) {
		sock_fd = dhcp_snp_init_tipc_sock();
	}
	
	if (sock_fd >= 0) {
		/* server address */
		memset(&servaddr, 0, sizeof(struct sockaddr_tipc));
		servaddr.family = AF_TIPC;
		servaddr.addrtype = TIPC_ADDR_NAMESEQ;
		servaddr.addr.nameseq.type = MACAUTH_SERVER_TYPE;
		servaddr.addr.nameseq.lower = MACAUTH_SERVER_INSTANCE;
		servaddr.addr.nameseq.upper = MACAUTH_SERVER_INSTANCE;
		servaddr.scope = TIPC_CLUSTER_SCOPE;

		/* msg */
		memset(&msg, 0, sizeof(msg));
		msg.addr.userip = userip;
		memcpy(msg.usermac, usermac, ETH_ALEN);
		msg.family = 4;

		syslog_ax_dhcp_snp_dbg("msg to potal(ipv4) %d.%d.%d.%d %02x:%02x:%02x:%02x:%02x:%02x\n",
			(userip>>24)&0xff, (userip>>16)&0xff, (userip>>8)&0xff, (userip>>0)&0xff, 
			usermac[0], usermac[1], usermac[2], usermac[3], usermac[4], usermac[5]);

		size = sendto(sock_fd, &msg, sizeof(msg), MSG_DONTWAIT, 
			(struct sockaddr *)(&servaddr), sizeof(servaddr));
		if (size != sizeof(msg)) {
			syslog_ax_dhcp_snp_dbg("%s %d.%d.%d.%d %02x:%02x:%02x:%02x:%02x:%02x :%m\n",
				"send msg to portal failed",
				(userip>>24)&0xff, (userip>>16)&0xff, (userip>>8)&0xff, (userip>>0)&0xff, 
				usermac[0], usermac[1], usermac[2], usermac[3], usermac[4], usermac[5]);
		}
	}
	
	return 0;
}
/******************************************************************************
 * dhcp_snp_u32ip_check
 *  check u32 ip address
 *	INPUT:
 *		ipaddr		- u32 IPv4 address
 *	
 *	OUTPUT:
 *
 * 	RETURN:
 *		0	-		valid
 *		-1	-		invalid ipv4 address
 ******************************************************************************/
int dhcp_snp_u32ip_check
(
	unsigned int ipaddr
)
{
	if (DHCP_SNP_PRIVATE_NET(htonl(ipaddr))
		|| DHCP_SNP_LOOPBACK(htonl(ipaddr))
		|| DHCP_SNP_MULTICAST(htonl(ipaddr))
		|| DHCP_SNP_BADCLASS(htonl(ipaddr))
		|| DHCP_SNP_ZERONET(htonl(ipaddr))) {
		log_debug("%s: invalid IPv4 address %s\n", __func__, u32ip2str(ipaddr));

		return -1;
	}
	   
	return 0;
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
int check_ipv6_address(char *ipv6_address)
{
	//char addrptr[16] = {0};
	static char
		addrptr[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];
	if(NULL == ipv6_address)
	{
		return -1;
	}

	if(NULL == inet_ntop(AF_INET6, ipv6_address, addrptr, sizeof(addrptr)))
	{
		return -1;
	}

	return 0;
}

#ifdef __cplusplus
}
#endif
