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
* stp_npd_cmd.c
*
* CREATOR:
*       wangxiangfeng@autelan.com
*
* DESCRIPTION:
*       APIs for npd communication interface in stp module
*
* DATE:
*       07/012008
*
*  FILE REVISION NUMBER:
*       $Revision: 1.2 $
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/un.h>  
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/socket.h>

#include <linux/if.h>
#include <linux/if_bridge.h>
#include <linux/sockios.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/fcntl.h>

#include "stp_base.h"
#include "stp_port.h"
#include "stp_stpm.h"
#include "stp_bitmap.h"
#include "stp_in.h"
#include "stp_log.h"
#include "stp_npd_cmd.h"

#define SIZE_OF_ADDRESS sizeof(struct sockaddr_un)

extern Bool g_flag;
extern BITMAP_T    enabled_ports;
extern BITMAP_T  g_VlanPBMP[4096] ;
extern RUNNING_MODE current_mode;
extern unsigned int pkt_mstid;
struct sockaddr_un	npdser , npdclt;
unsigned int npdSock;	/*NPD cmd socket*/
unsigned char sysMac[6] = {0};

#ifndef WAN_MSTP_SUPORT
#define MAX_BRIDGES	(1024*4)
#define MAX_PORTS	(1024*4)
#define MAX_PAYLOAD 1024
#define GRP_ID 1
#define BR_RTMSG(r) ((unsigned char *)(r) + NLMSG_ALIGN(sizeof(struct br_netlink_msg)))
#define SLOT_PORT_SPLIT_DASH 	'-'
#define SLOT_PORT_SPLIT_SLASH	'/'
#define SLOT_PORT_SPLIT_COMMA 	','
#define BRCTL_SET_PORT_STATE 22
#define BRCTL_USER_MSTP_EN	23		/* user mtp enable*/

#define NETLINK_BRIDGE		18
#define LINKST_BRIDGE		0
#define ETH_HLEN	14		/* Total octets in ethernet header.	 */
#define MAX_IFNAME_LEN        20
#define BUFLENGTH (4096)
#define INTERFACE_UP     1



int br_socket_fd = -1;
int netlinkSock = -1;	/*netlink cmd socket*/
unsigned int linkstSock = -1;

struct br_netlink_msg
{
	int	br_ifindex;			/* bridge Link index	*/
	int	port_ifindex;		/* port Link index	*/
	unsigned int msg_len;			
};

/**********************************************************************************
 *  stp_wan_lnk_socket_init
 *
 *	DESCRIPTION:
 * 		init netlink socket
 *
 *	INPUT:		
 *		NULL
 *			
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		-1			- error
 *		linkstSock	- fd		success
 *
 *	NOTATION:
 *		
 **********************************************************************************/

int	stp_wan_lnk_socket_init(	void)
{
	struct sockaddr_nl src_addr;
	
	/* Initialize data field */
	memset(&src_addr, 0, sizeof(struct sockaddr_nl));
	/* Create getting link state socket use NETLINK_BRIDGE(22) */
	if ((linkstSock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0) {
		stp_syslog_error("create getting link state bridge socket fail !\n");
		return -1;
	}

	/* Fill in src_addr */
	bzero(&src_addr, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid(); /* Thread method */
	src_addr.nl_groups = RTMGRP_IPV4_IFADDR|RTMGRP_IPV4_ROUTE |RTMGRP_LINK| RTMGRP_NOTIFY;
	
	if (bind(linkstSock, (struct sockaddr*)&src_addr, sizeof(src_addr)) < 0) {
		stp_syslog_error("bind getting link state bridge socket fail !\n");
		return -1;
	}

	return linkstSock;
}

/**********************************************************************************
 *  stp_instance_get_port_pnt
 *
 *	DESCRIPTION:
 * 		find port pointer in instance
 *
 *	INPUT:		
 *		this		instance pointer
 *		slot_no	slot number
 *		port_no	port number
 *			
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		port	-port pointer
 *		
 *	NOTATION:
 *		
 **********************************************************************************/

static PORT_T *
stp_instance_get_port_pnt(STPM_T* this, int slot_no, int port_no)
{
  register PORT_T* port;

  for (port = this->ports; port; port = port->next)
    if ((slot_no == port->slot_no) &&
		(port_no == port->port_no)){
      return port;
    }

  return NULL;
}

/**********************************************************************************
 *  stp_lnkst_read_socket
 *
 *	DESCRIPTION:
 * 		while port link state changed ,read link state and enable port 
 *
 *	INPUT:		
 *		NULL
 *			
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		-1	error
 *		0	success
 *		
 *	NOTATION:
 *		
 **********************************************************************************/

int stp_lnkst_read_socket(void)
{
	
	struct sockaddr_nl dest_addr;
	struct nlmsghdr *nlp = NULL;
	struct iovec iov;
	struct msghdr msg;
	struct ifinfomsg *rtEntry = NULL;
	struct rtattr *rtattrptr = NULL;
	unsigned char slot_no = 0, port_no = 0, subintf = 0;
	unsigned int mstid = 0;
	unsigned char* ifaddr = NULL;
	char* ifname = NULL;
	int msglen=0,payloadoff = 0;
	register STPM_T* this = NULL;
	register PORT_T* port = NULL;
	int portindex = 0,i = 0;

	unsigned int ifi_flags = 0, ifi_index = ~0UI;
	unsigned int status = 0,port_ifindex = 0;

#define SYSFS_PATH_MAX 256
	FILE *f = NULL;
	char path[SYSFS_PATH_MAX] = {0};
	unsigned int br_ifindex = 0;


	/* Initialize data field */
	memset(&dest_addr, 0, sizeof(dest_addr));
	memset(&iov, 0, sizeof(iov));
	memset(&msg, 0, sizeof(msg));

	/* Fill in dest_addr */
	dest_addr.nl_pid = 0; /* From kernel */
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_groups = GRP_ID;

	/* Initialize buffer */
	if((nlp = (struct nlmsghdr*)malloc(NLMSG_SPACE(MAX_PAYLOAD))) == NULL) {
		//perror("malloc");
		return -1;
	}

	memset(nlp, 0, NLMSG_SPACE(MAX_PAYLOAD));
	iov.iov_base = (void *)nlp;
	iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	/* Recv message from kernel */
	msglen = recvmsg(linkstSock, &msg, 0);

	for(;NLMSG_OK(nlp, msglen);nlp=NLMSG_NEXT(nlp, msglen)) {

	rtEntry = (struct ifinfomsg*)NLMSG_DATA(nlp);
	payloadoff = RTM_PAYLOAD(nlp);

	switch( nlp->nlmsg_type ) {
		  case RTM_NEWLINK:
			ifi_flags = rtEntry->ifi_flags;  /* get interface */
			ifi_index = rtEntry->ifi_index; /* ifindex for kernel*/
			rtattrptr = (struct rtattr *)IFLA_RTA(rtEntry);
			for(;RTA_OK(rtattrptr, payloadoff);rtattrptr=RTA_NEXT(rtattrptr,payloadoff)) {
				switch(rtattrptr->rta_type) {
					case IFLA_ADDRESS:
						ifaddr = (unsigned char*)RTA_DATA(rtattrptr);
						break;
					case IFLA_IFNAME:
						ifname = RTA_DATA(rtattrptr);
						stp_syslog_dbg("when port link state changed getting ifname %s\n",ifname);
						/*npd_syslog_dbg("ifindex %d,vid %d\n",ifindex,vid);*/
						break;
					default:
						/*npd_syslog_dbg("other value ignore %d\n",nlp->nlmsg_type);*/
						break;
				}
			}
		  default:
			 break;
		  } 	
}

	if(ifaddr && ifname && ifname[0] == 'e') {	/* ifname[0] == 'e' : interface lo is not in this branch */
		stp_netlink_parse_slot_port_no(ifname + 3, &slot_no, &port_no, &subintf);
		stp_syslog_dbg("slot %d port %d subintf %d\n",slot_no, port_no, subintf);
		if(ifi_flags & IFF_RUNNING){
				status = 0;
				stp_syslog_dbg("status %d\n",status);
		   }
		   else {
				status = 1;
				stp_syslog_dbg("status %d\n",status);
		   }
		   stp_syslog_dbg("pkt_mstid is %d\n",pkt_mstid);
	   if(1 == status){
			pkt_mstid = 0;
	   }
	mstid = (int)pkt_mstid;
	stp_syslog_dbg("while link state changed getting mstid is %d\n",mstid);
	this = stp_stpm_get_instance(mstid); /* find instance pointer by mstid */

	/*this = stp_in_stpm_find (mstid);*/
	if (! this) { /*  the stpm had not yet been created :( */
		stp_syslog_dbg("while link state changed find instance %d failed!\n",mstid);
	 	RSTP_CRITICAL_PATH_END;
	 	return 0;
	 }
	port = stp_instance_get_port_pnt(this, slot_no, port_no); /* find port pointer in instance */
	
	if(port){

		if(!subintf){
			stp_syslog_dbg("find port in instance %d successed!\n",mstid);
			portindex = port->port_index;
			stp_syslog_dbg("portindex %d \n",portindex);
			if(LINK_PORT_UP_E == status) {
				stp_bitmap_set_bit(&enabled_ports, portindex);	
			}
			else {
				stp_bitmap_clear_bit(&enabled_ports, portindex);
			}
			/*save port link info ,but when stp don't enable port don't enable*/
			if(!g_flag){
				return 0;
			}
			stp_in_enable_port_on_stpm(portindex, 1, status, 8192, 1, slot_no,port_no);

			if(mstid){
				for(i=1; i<4095; i++){
					snprintf(path, SYSFS_PATH_MAX, "/sys/class/net/eth%d-%d.%d/brport/bridge/ifindex", slot_no, port_no, i);
					f = fopen(path, "r");
					if (f) {
						fscanf(f, "%d", &br_ifindex);
						sprintf(ifname, "eth%d-%d.%d", slot_no, port_no, i);
						port->port_ifindex = if_nametoindex(ifname);
						port->br_ifindex = br_ifindex;
						fclose(f);
						stp_syslog_dbg("br_ifindex %d\n",port->br_ifindex);
						stp_syslog_dbg("port_ifindex %d\n",port->port_ifindex);
						/*when enable or disable port setting it's state discard first*/
						stp_npd_set_wan_state(port->br_ifindex, port->port_ifindex, 1);/*NAM_STP_PORT_STATE_DISCARD_E*/

						break;
					}
				}
			}
			stp_port_init( port, this, True );
		}
	}
	}
	return 0;
}

/**********************************************************************************
 *  stp_npd_ioctl_init
 *
 *	DESCRIPTION:
 * 		init stp ioctl
 *
 *	INPUT:		
 *		NULL
 *			
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		1 - error
 *		0 - success
 *		
 *	NOTATION:
 *		
 **********************************************************************************/
int 
stp_npd_ioctl_init
(
	void
)
{
	if ((br_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return 1;
	}
	return 0;
}

/**********************************************************************************
 *  stp_npd_get_portno
 *
 *	DESCRIPTION:
 * 		get port number from bridge and interface
 *
 *	INPUT:		
 *		char *bridge - bridge pointer
 *		char *ifname - ifname pointer
 *			
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		-1		error
 *		portno	success
 *		
 *	NOTATION:
 *		
 **********************************************************************************/
int 
stp_npd_get_portno
(
	char *brname, 
	char *ifname
)
{
	int i = 0;
	int ifindex = 0;
	int ifindices[MAX_PORTS] = {0};
	unsigned long long args[4] = { BRCTL_GET_PORT_LIST,
				  (unsigned long long)ifindices, MAX_PORTS, 0 };
	struct ifreq ifr;

	if (!brname || !ifname) {	
		stp_syslog_error("stp_npd_get_portno: brname is %s, ifname is %s \n", brname ? "exist" : "NULL", ifname ? "exist" : "NULL");
		return -1;
	}
	ifindex = if_nametoindex(ifname);
	if (ifindex <= 0) {
		stp_syslog_error("stp_npd_get_portno: get ifindex fail name is %s \n", ifname);
		return -1;
	}

	memset(ifindices, 0, sizeof(ifindices));
	memset(&ifr, 0, sizeof(struct ifreq));
	
	strncpy(ifr.ifr_name, brname, IFNAMSIZ);
	ifr.ifr_data = (char *) &args;

	if (ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr) < 0) {
		stp_syslog_error("stp_npd_get_portno: get ports of %s failed: %s\n", 
			brname, strerror(errno));
		return -1;
	}

	for (i = 0; i < MAX_PORTS; i++) {
		if (ifindices[i] == ifindex) {
			return i;
		}
	}

	stp_syslog_error("%s is not a in bridge %s\n", ifname, brname);
	return -1;
}

/**********************************************************************************
 *  stp_npd_port_set_state
 *
 *	DESCRIPTION:
 * 		set interface state use ioctl
 *
 *	INPUT:		
 *		char *bridge
 *		char *ifname
 *		unsigned long value
 *		unsigned long oldcode
 *			
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		errno	error
 *		0		success
 *		
 *	NOTATION:
 *		
 **********************************************************************************/
int 
stp_npd_port_set_state
(
	char *bridge, 
	char *ifname, 
	unsigned long value, 
	unsigned long oldcode
)
{
	int ret = 0;

	if (!bridge || !ifname) {
		stp_syslog_error("stp_npd_port_set_state: bridge is %s, ifname is %s \n", bridge ? "exist" : "NULL", ifname ? "exist" : "NULL");
		return -1;
	}
	
	int index = stp_npd_get_portno(bridge, ifname);

	if (index < 0) {
		ret = index;
		stp_syslog_error("set wan port state fail !!! ifbr %s, ifport %s \n", bridge, ifname);
	}
	else {
		struct ifreq ifr;
		memset(&ifr, 0, sizeof(struct ifreq));
		/*kernel 64bit user 32bit*/
		unsigned long long args[4] = { oldcode, index, value, 0 };
		
		strncpy(ifr.ifr_name, bridge, IFNAMSIZ);
		ifr.ifr_data = (char *) &args;
		ret = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);
	}
	stp_syslog_dbg("set wan port state br %s, port %s, state %d \n", bridge, ifname, value);

	return ret < 0 ? errno : 0;
}

/**********************************************************************************
 *  stp_npd_wan_mstp_enable
 *
 *	DESCRIPTION:
 * 		set interface state use ioctl
 *
 *	INPUT:		
 *		unsigned long enable
 *			
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		errno	error
 *		0		success
 *		
 *	NOTATION:
 *		
 **********************************************************************************/
int 
stp_npd_wan_mstp_enable
(
	unsigned long enable
)
{
	int ret = 0;
	unsigned long long args[3] = { BRCTL_USER_MSTP_EN, enable, 0 };

	if (enable > 1) {
		return 1;
	}

	ret = ioctl(br_socket_fd, SIOCSIFBR, args);
	if (ret < 0) {
		stp_syslog_error("set wan mstp enable fail %s \n",
			strerror(errno));
		return -errno;
	}
	stp_syslog_dbg("set wan mstp enable %s \n", enable ? "enable" : "disable");

	return ret;
}

/**********************************************************************************
 *  stp_npd_set_wan_state
 *
 *	DESCRIPTION:
 * 		set interface state
 *
 *	INPUT:		
 *		unsigned int br_ifindex
 *		unsigned int port_ifindex
 *		unsigned int state
 *			
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *		
 *	NOTATION:
 *		
 **********************************************************************************/
void 
stp_npd_set_wan_state
(
	unsigned int br_ifindex,
	unsigned int port_ifindex,
	unsigned int state
)
{
	int ret = 0;
	unsigned char br_ifname[MAX_IFNAME_LEN] = {0}, port_ifname[MAX_IFNAME_LEN] = {0};

	if (!if_indextoname(br_ifindex, br_ifname) ||
		!if_indextoname(port_ifindex, port_ifname)) {
		stp_syslog_error("wan port can not find ifname by index \n");
		return;
	}
	ret = stp_npd_port_set_state(br_ifname, port_ifname, state, BRCTL_SET_PORT_STATE);
	if (ret) {
		stp_syslog_error("set wan port state error br %s, port %s, state %d \n", br_ifname, port_ifname, state);
	}
}

/**********************************************************************************
 *  stp_netlink_socket_init
 *
 *	DESCRIPTION:
 * 		init netlink socket
 *
 *	INPUT:		
 *		NULL
 *			
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		-1				error
 *		netlink socket fd	success
 *		
 *	NOTATION:
 *		
 **********************************************************************************/
int 
stp_netlink_socket_init
(
	void
)
{
	struct sockaddr_nl src_addr;
	
	/* Initialize data field */
	memset(&src_addr, 0, sizeof(struct sockaddr_nl));
	/* Create netlink socket use NETLINK_BRIDGE(22) */
	if ((netlinkSock = socket(PF_NETLINK, SOCK_RAW, NETLINK_BRIDGE)) < 0) {
		stp_syslog_error("create netlink bridge socket fail !\n");
		return -1;
	}

	/* Fill in src_addr */
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid(); /* Thread method */
	src_addr.nl_groups = GRP_ID;
	
	if (bind(netlinkSock, (struct sockaddr*)&src_addr, sizeof(src_addr)) < 0) {
		stp_syslog_error("bind netlink bridge socket fail !\n");
		return -1;
	}

	return netlinkSock;
}

/**********************************************************************************
 *  stp_netlink_socket_sendto
 *
 *	DESCRIPTION:
 * 		send netlink message
 *
 *	INPUT:		
 *		char* ifname
 *		char* dpdu
 *		int buffer_size
 *			
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		NULL
 *		
 *	NOTATION:
 *		
 **********************************************************************************/
void 
stp_netlink_socket_sendto
(
	char *ifname, 
	char *bpdu,
	int buffer_size
)
{
	struct sockaddr_nl nladdr;
	struct {
		struct nlmsghdr 	n;
		struct br_netlink_msg 	ndm;
		char   			buf[255];
	} netlinkMsg;
	struct iovec iov = {
		.iov_base = (void*)&(netlinkMsg.n),
		.iov_len = netlinkMsg.n.nlmsg_len
	};
	struct msghdr msg = {
		.msg_name = &nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};
	
	if (!ifname || !bpdu) {	
		stp_syslog_error("stp_netlink_socket_sendto: ifname is %s, bpdu is %s \n", ifname ? "exist" : "NULL", bpdu ? "exist" : "NULL");
		return;
	}
	/* Initialize buffer */
	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;
	nladdr.nl_pid = 0;
	nladdr.nl_groups = 0;

	memset(&netlinkMsg, 0, sizeof(netlinkMsg));
	
	netlinkMsg.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct br_netlink_msg) + (buffer_size - ETH_HLEN));/*use skb data len*/
	/* ifindex */
	/*netlinkMsg.ndm.br_ifindex = 11; not need bridge interface index*/
	netlinkMsg.ndm.port_ifindex = if_nametoindex(ifname);
	netlinkMsg.ndm.msg_len= (buffer_size - ETH_HLEN);/*msg_len is small than 255, be careful of*/
	memcpy(netlinkMsg.buf, &bpdu[ETH_HLEN], (buffer_size - ETH_HLEN));

	/* update iov length */
	iov.iov_len = netlinkMsg.n.nlmsg_len;

	stp_syslog_dbg("send netlink msg to kernel fd is %d , nlmsg_len is %d\n", netlinkSock, netlinkMsg.n.nlmsg_len);

	sendmsg(netlinkSock, &msg, 0);

}

/**********************************************************************************
 *  stp_netlink_parse_slot_port_no
 *
 *	DESCRIPTION:
 * 		parse slot number port number
 *
 *	INPUT:		
 *		char *str
 *			
 *	OUTPUT:
 *		unsigned char *slotno
 *		unsigned char *portno
 *
 * 	RETURN:
 *		1	error
 *		0	success
 *		
 *	NOTATION:
 *		
 **********************************************************************************/
int  
stp_netlink_parse_slot_port_no
(
	char *str,
	unsigned char *slotno,
	unsigned char *portno,
	unsigned char *subintf
) 
{
	char *endptr = NULL;
	char *endptr2 = NULL;
	char *endptr3 = NULL;
#define SLOT_PORT_SPLIT_SPOT '.'
	if ((!str)||(!slotno)||(!portno)){ 
		stp_syslog_error("stp_netlink_parse_slot_port_no: null pointer %s %s %s \n", str ? "" : "str",\
			slotno ? "" : "slotno", portno ? "" : "portno");
		return 1;
	}
	
	*portno = strtoul(str, &endptr, 10);
	if (endptr) {
		if ((SLOT_PORT_SPLIT_DASH == endptr[0])||(SLOT_PORT_SPLIT_SLASH == endptr[0])) {
            *slotno = *portno;
			*subintf = strtoul((char *)&(endptr[1]),&endptr2,10);
			if('\0' == endptr2[0]) {
				*portno = *subintf;
				*subintf = 0;
				return 0;
			}
			else if(endptr2){
				if((SLOT_PORT_SPLIT_SPOT == endptr2[0])){
					*portno = *subintf;
					*subintf = strtoul((char *)&(endptr2[1]),&endptr3,10);
				}
			}
			if('\0' == endptr3[0]) {
					return 0;
				}
			else {
				stp_syslog_error(" unknow char %d %c \n", endptr2[0],endptr2[0]);
				return 1;
			}
		}
		if ('\0' == endptr[0]) {
			*slotno = 0;
			return 0;
		}		
		stp_syslog_error("endptr is not null str %s endptr %s\n", str, endptr);
	}
	stp_syslog_error("endptr is %s SLOT_PORT_SPLIT_DASH %c SLOT_PORT_SPLIT_SLASH %c  \n", \
		endptr ? "not" : "null", SLOT_PORT_SPLIT_DASH, SLOT_PORT_SPLIT_SLASH);
	
	return 1;	
}

/**********************************************************************************
 *  stp_netlink_read_socket
 *
 *	DESCRIPTION:
 * 		read netlink socket
 *
 *	INPUT:		
 *		NULL
 *			
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		-1	error
 *		0	success
 *		
 *	NOTATION:
 *		
 **********************************************************************************/
int 
stp_netlink_read_socket
(
	void
)
{
	struct sockaddr_nl dest_addr;
	struct nlmsghdr *nlh = NULL;
	struct iovec iov;
	struct msghdr msg;
	struct br_netlink_msg *nl_msg = NULL;
	unsigned char ifname[MAX_IFNAME_LEN] = {0};
	unsigned char slot = 0, port = 0, subintf = 0;
	unsigned int mstid = 0;
	
	/* Initialize data field */
	memset(&dest_addr, 0, sizeof(dest_addr));
	memset(&iov, 0, sizeof(iov));
	memset(&msg, 0, sizeof(msg));

	/* Fill in dest_addr */
	dest_addr.nl_pid = 0; /* From kernel */
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_groups = GRP_ID;

	/* Initialize buffer */
	if((nlh = (struct nlmsghdr*)malloc(NLMSG_SPACE(MAX_PAYLOAD))) == NULL) {
		//perror("malloc");
		return -1;
	}

	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
	iov.iov_base = (void *)nlh;
	iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	/* Recv message from kernel */
	recvmsg(netlinkSock, &msg, 0);

	nl_msg = (struct br_netlink_msg*)NLMSG_DATA(nlh);
	if_indextoname(nl_msg->port_ifindex, ifname);
	if(!strncmp("eth", ifname, 3)) {
		/*mstid = pkt_mstid;*/
		if (!stp_netlink_parse_slot_port_no(ifname + 3, &slot, &port, &subintf)) {
			stp_in_rx_bpdu (0, 0, slot, port, BR_RTMSG(nl_msg), nl_msg->msg_len);
			stp_syslog_dbg("port ifname %s,  skb_data len %d\n",ifname, nl_msg->msg_len);
		}
	}
	
	/*
	printf("br_ifname %s, port_ifname %s,  skb_data len %d\n",
			if_indextoname(nl_msg->br_ifindex, nameb), if_indextoname( nl_msg->port_ifindex, names), nl_msg->msg_len);	
	dump_packet_detail(BR_RTMSG(nl_msg), nl_msg->msg_len);
	*/

	return 0;
}

#endif

int stp_npd_socket_init(void)

{
	memset(&npdclt,0,sizeof(npdclt));
	memset(&npdser,0,sizeof(npdser));
	//fromlen = sizeof(server);
	
	if((npdSock = socket(AF_LOCAL,SOCK_DGRAM,0))!= -1)
	{       
		stp_syslog_dbg("RSTP npdsock id is %d \n", npdSock);
		npdclt.sun_family = AF_LOCAL;
		npdser.sun_family = AF_LOCAL;

		strcpy(npdclt.sun_path, "/tmp/NpdCmd_CLIENT");
	    strcpy(npdser.sun_path, "/tmp/NpdCmd_SERVER");
		//printf("create socket\n");

		unlink(npdclt.sun_path);

		
		//printf("link addr\n");
		if(bind(npdSock,(struct sockaddr *)&npdclt,sizeof(npdclt))!= -1)
		{	
			chmod(npdclt.sun_path, 0777);
		}
		else
		{
			stp_syslog_error("RSTP npdsock bind error;");
			close (npdSock);
			return 0;
		}
	}
	else 
	{
		stp_syslog_warning("RSTP npdsock init failed\n");
		//close (sock);
	}
     	
	return npdSock;
}

static int stp_npd_socket_rcvfrom (void* msg, int buffer_size)
{
	int	byteRecv;
	unsigned int len = SIZE_OF_ADDRESS;
	
	while(1) 
	{
	    byteRecv = recvfrom(npdSock, msg, buffer_size, 0,(struct sockaddr*) &npdser ,&len);
	    if(byteRecv < 0 && errno == EINTR) 
	  {
	  	continue;
	    }
	    break;
	}

	return byteRecv;
}

static int stp_npd_socket_sendto(void* msg,int buffer_size)
{
	int rc;

	rc = sendto (npdSock, msg, buffer_size, 0,(struct sockaddr *) &npdser ,SIZE_OF_ADDRESS);
	stp_syslog_dbg("stp send message to npd rc %d \n",rc);
	if(rc < 0) {
	    if(errno == EINTR) {
	      return 0;
	    } 
	    else {
	      return -1; 
	    }
	}

	return rc;
}

#if 0
static int npd_cmd_enable_port(unsigned int port_index,Bool enable)
{
	int rc = 0;

	STP_LOG(STP_LOG_DEBUG,"RSTP>>npd_cmd108:: STP_enable \n");
	if(!g_flag)
		return 0xff;

	rc = stp_in_enable_port_on_stpm (port_index, enable);
	if(stp_bitmap_get_bit(&enabled_ports, port_index)) {
		
		//stp_port_trace_state_machine (newPort, "all", 1, 1);
		//stp_port_bridge_trace_state_machine (first, "all", 1, 2);
		STP_LOG(STP_LOG_DEBUG,"RSTP>>npd_cmd115:: STP_enable end ,rc %d\n",rc);
	}
	
	return 0;
}
#endif 

static int stp_npd_cmd_add_port(unsigned short vid,unsigned int port_index,unsigned int isWan)
{
	int rc,i;
	
	stp_syslog_dbg("RSTP add port to vid:: vid %d, port %d \n", 	\
		vid,port_index);
	
	if((!g_flag) || (MST_MODE != stp_param_get_running_mode())) 
		return 0xff;


	if(vid < 1 || vid > STP_MAX_VID) {
		stp_syslog_error("vid %d is error",vid);
		return -1;
	}
	stp_syslog_dbg("port_index %d, iswan %d\n",port_index,isWan);
	rc = stp_in_add_port_to_mstp(vid,port_index,isWan);
	
	stp_bitmap_set_bit(&g_VlanPBMP[vid],port_index);
	stp_syslog_dbg("RSTP add port to vid end,rc %d\n",rc);
	return rc;
}

static int stp_npd_cmd_del_port(unsigned short vid,unsigned int port_index)
{
	int rc,i;
	
	stp_syslog_dbg("RSTP delete port from vid::vid %d, port %d \n",  \
		vid,port_index);
	if((!g_flag) || (MST_MODE != stp_param_get_running_mode()))
		return 0xff;
	
	if(vid < 1 || vid > STP_MAX_VID) {
		stp_syslog_dbg("vid %d is error",vid);
		return -1;
	}
	rc = stp_in_del_port_from_mstp(vid,port_index);
	stp_bitmap_clear_bit(&g_VlanPBMP[vid],port_index);

	stp_syslog_dbg("RSTP delete port from vid end,rc %d\n",rc);
	return rc;
}

static int stp_npd_cmd_add_vlan_on_mst(unsigned short vid,unsigned int untagbmp[],unsigned int tagbmp[])
{
	int rc = 0,i;
	unsigned int tmp[2] = {0};
	BITMAP_T port_bmp = {{0}};

	stp_syslog_dbg("RSTP add vid to cist::vid %d, untagbmp[0] %d , untagbmp[1] %d,tagbmp[0] %d ,tagbmp[1] %d\n",  \
		vid,untagbmp[0],untagbmp[1],tagbmp[0],tagbmp[1]);
		
	if((!g_flag) || (MST_MODE != stp_param_get_running_mode()))
		return 0xff;
	
	/*when initing ,vlan has maped cist*/

	if(vid < 1 || vid > STP_MAX_VID) {
		stp_syslog_error("vid %d is error",vid);
		return -1;
	}

	tmp[0] = (untagbmp[0] | tagbmp[0]);	
	tmp[1] = (untagbmp[1] | tagbmp[1]);	
	stp_syslog_dbg("RSTP add vlan to cist port_bmp  ");
	for(i = 1 ; i < 9; i++) {
		if(i<5){
		   port_bmp.part[i] = ((tmp[0]>> ((i-1)*8)) & 0xff);
		}
		else{
		   port_bmp.part[i] = ((tmp[1]>> ((i-5)*8)) & 0xff);

		}
		stp_syslog_dbg("%02x ",port_bmp.part[i]);
	}
	stp_syslog_dbg("\n");
	rc = stp_in_add_vlan_to_cist(vid,port_bmp);
	
	stp_syslog_dbg("RSTP add vlan to cist end,rc %d\n",rc);
		
	return rc;
}

static int stp_npd_cmd_del_vlan_on_mst(unsigned short vid)
{
	int rc;   	
	if((!g_flag) || (MST_MODE != stp_param_get_running_mode()))
		return 0xff;
	
	if(vid < 1 || vid > STP_MAX_VID) {
		stp_syslog_error("vid %d is error",vid);
		return -1;
	}

	rc = stp_in_del_vlan_from_cist(vid);
	stp_bitmap_portbmp_add(&g_VlanPBMP[1], &g_VlanPBMP[vid]);
	stp_bitmap_clear(&g_VlanPBMP[vid]);
	stp_syslog_dbg("RSTP delete vlan from cist end,rc %d\n",rc);
	return rc;
}

static int stp_npd_cmd_set_param_by_link_state(NPD_CMD_STC* cmdptr)
{
	Bool enable = False;
	unsigned int duplex_mode = cmdptr->cmdData.cmdLink.duplex_mode;

	if(LINK_PORT_UP_E == cmdptr->cmdData.cmdLink.portState) {
		enable = True;
		stp_bitmap_set_bit(&enabled_ports, cmdptr->cmdData.cmdLink.portIdx);	
	}
	else {
		enable = False;
		stp_bitmap_clear_bit(&enabled_ports, cmdptr->cmdData.cmdLink.portIdx);
	}

	stp_syslog_dbg("RSTP get port %d link state %s \n", cmdptr->cmdData.cmdLink.portIdx, \
		enable ? "enable" : "disable");

	/*save port link info ,but when stp don't enable port don't enable*/
	if(!g_flag)
		return 0xff;
	//enable port in stp protocol
	//printf("STP¡¡link state get ::port state %s \n",enable ? "enable" : "disable");
	stp_in_enable_port_on_stpm (cmdptr->cmdData.cmdLink.portIdx, 1,cmdptr->cmdData.cmdLink.portState,cmdptr->cmdData.cmdLink.speed, cmdptr->cmdData.cmdLink.isWAN, cmdptr->cmdData.cmdLink.slot_no, cmdptr->cmdData.cmdLink.port_no);
   //set the port duplex mode
    stp_param_set_port_duplex_mode(cmdptr->cmdData.cmdLink.portIdx,duplex_mode);
    stp_syslog_dbg("port %d,enable %d,portstate %d,port speed %d,port duplex mode %d",cmdptr->cmdData.cmdLink.portIdx, enable,cmdptr->cmdData.cmdLink.portState,cmdptr->cmdData.cmdLink.speed,duplex_mode);
	
	return 0;
	
}

int stp_npd_read_socket(void)
{
	char recv_buf[sizeof(NPD_CMD_STC)] = {0};
	NPD_CMD_STC*	msg;
	
	unsigned int			msgsize;
	int i;
	int rc = 0;

	stp_syslog_dbg("RSTP begin receive msg from npd:\n");
	msgsize = stp_npd_socket_rcvfrom(recv_buf, sizeof(NPD_CMD_STC));
	if(msgsize <= 0)
	{
		stp_syslog_error("Something wrong in UIF\n ");
		return 0;
	}
	msg = (NPD_CMD_STC*) recv_buf;

	stp_syslog_dbg("msg cmdType %d\n",msg->cmdType);
	switch(msg->cmdType)
	{
		case INTERFACE_ADD_E:
			stp_syslog_dbg("%s :: add vlan id %d add port_index %d isWan %d\n",__FILE__,msg->cmdData.cmdVlan.vid,msg->cmdData.cmdIntf.portIdx,msg->cmdData.cmdIntf.isWan);
			rc = stp_npd_cmd_add_port(msg->cmdData.cmdIntf.vid,msg->cmdData.cmdIntf.portIdx,msg->cmdData.cmdIntf.isWan);
			if(rc < 0)
				stp_syslog_warning("RSTP>>create port return info failed\n");
			return 0;
			
		case INTERFACE_DEL_E:
			stp_syslog_dbg("%s ::  vlan id %d delete port_index %d\n",__FILE__,msg->cmdData.cmdVlan.vid,msg->cmdData.cmdIntf.portIdx);
			rc = stp_npd_cmd_del_port(msg->cmdData.cmdIntf.vid,msg->cmdData.cmdIntf.portIdx);
			if(rc != 0)
				stp_syslog_warning("RSTP>>del port return info failed\n");
			return 0;
			
		case VLAN_ADD_ON_MST_E:
			stp_syslog_dbg("%s :: add vlan id %d\n",__FILE__,msg->cmdData.cmdVlan.vid);
			rc = stp_npd_cmd_add_vlan_on_mst(msg->cmdData.cmdVlan.vid,msg->cmdData.cmdVlan.untagbmp,msg->cmdData.cmdVlan.tagbmp);
			if(rc != 0)
				stp_syslog_warning("RSTP>>add vlan return info failed\n");
			return 0;
			
		case VLAN_DEL_ON_MST_E:
			stp_syslog_dbg("%s :: del vlan id %d\n",__FILE__,msg->cmdData.cmdVlan.vid);
			rc = stp_npd_cmd_del_vlan_on_mst(msg->cmdData.cmdVlan.vid);
			if(rc != 0)
				stp_syslog_warning("RSTP>>del port return info failed\n");
			return 0;
	
		case LINK_CHANGE_EVENT_E:
			stp_syslog_dbg("%s :: link change port index %d\n",__FILE__,msg->cmdData.cmdLink.portIdx);	
			rc =  stp_npd_cmd_set_param_by_link_state(msg);
			break;
		case STP_GET_SYS_MAC_E:
			if(NULL != msg){
	           for(i = 0;i<6;i++){
	               sysMac[i] = msg->cmdData.cmdFdb.MacDa[i];
			   }			     
			}
			else{
	           stp_syslog_warning("Get msg error\n");
			}
			break;
		default:
			break;
	}

   	return 0;
}


int stp_npd_cmd_send_stp_info
(
	unsigned int mstid,
	unsigned short vid,
	unsigned int port_index,
	NAM_RSTP_PORT_STATE_E state
)
{
	int rc = 0;
	NPD_CMD_STC stpInfo;
	memset(&stpInfo,0,sizeof(NPD_CMD_STC));

	stpInfo.cmdType = STP_STATE_UPDATE_E;
	stpInfo.cmdLen = sizeof(CMD_STP_STC);
	stpInfo.cmdData.cmdStp.mstid = mstid;
	stpInfo.cmdData.cmdStp.vid = vid;
	stpInfo.cmdData.cmdStp.port_index = port_index;
	stpInfo.cmdData.cmdStp.portState = state;

	rc = stp_npd_socket_sendto(&stpInfo,sizeof(stpInfo));
	if(rc >= 0) {
		stp_syslog_dbg("%s%d:: stp_npd_socket_sendto rc %d \n",__FILE__,__LINE__,rc);
	}

	return 0;
}

int stp_npd_cmd_notify_tcn
(
	unsigned int mstid,
	unsigned short vid,
	unsigned int port_index
)
{
	int rc = 0;
	NPD_CMD_STC stpInfo;
	memset(&stpInfo,0,sizeof(NPD_CMD_STC));

	stpInfo.cmdType = STP_RECEIVE_TCN_E;
	stpInfo.cmdLen = sizeof(CMD_STP_STC);
	stpInfo.cmdData.cmdStp.mstid = mstid;
	stpInfo.cmdData.cmdStp.vid = vid;
	stpInfo.cmdData.cmdStp.port_index = port_index;

	rc = stp_npd_socket_sendto(&stpInfo,sizeof(stpInfo));
	if(rc >= 0) {
		stp_syslog_dbg("%s%d:: stp_npd_socket_sendto rc %d \n",__FILE__,__LINE__,rc);
	}

	return 0;
}


int stp_npd_cmd_send_stp_mac()

{
    int rc = 0;
	NPD_CMD_STC stpInfo;
	memset(&stpInfo,0,sizeof(NPD_CMD_STC));

	stpInfo.cmdType = STP_GET_SYS_MAC_E;
	stpInfo.cmdLen = sizeof(sizeof(CMD_VALUE_UNION));
	rc = stp_npd_socket_sendto(&stpInfo,sizeof(stpInfo));
	if(rc >= 0) {
		stp_syslog_dbg("The send value count is %d\n",rc);
	}
	else{
        stp_syslog_error("Send mac request failed\n");
	} 
	return 0;
	
}
#ifdef __cplusplus
}
#endif

