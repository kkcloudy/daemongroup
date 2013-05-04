 #ifndef _HAD_UID_H__
 #define _HAD_UID_H__

 #define MAX_IP_COUNT 8
 #define INVALID_HOST_IP 0xFFFFFFFF
 #define BUFLENGTH (4096)
 extern int vrrp_arp_send_fd;
 
typedef struct uid_msg_s {
	unsigned int ifindex;
	unsigned int bodyLen;
	unsigned char *vrrp;
} UID_MSG_T;

#define MAX_UID_MSG_SIZE    1200
#define INTERFACE_DOWN   0
#define INTERFACE_UP     1

#define VRRP_LINK_STATE_DESCANT(linkState)		\
({												\
	(linkState == INTERFACE_DOWN) ? "link-down" :	\
	(linkState == INTERFACE_UP) ? "link-up" :	\
	"error link state";							\
})

typedef struct pkt_msg_s{
    char set_flag;
	int  buflen;
	unsigned int time;
	char buf[MAX_UID_MSG_SIZE];
}BUF_INFO;
typedef struct pkt_buf_s{
   unsigned int vrid;
   BUF_INFO uplink;
   BUF_INFO downlink;
}PKT_BUF;

int had_SocketInit();
char had_read_socket
(
    vrrp_rt * vsrv,
    unsigned int ifindex,
    char * buf,
    int buflen
);

/********************************************************
*	function declare									*
*********************************************************/
int had_SocketInit
(
	void
);

int had_SocketInit2
(
	void
);

vrrp_rt* had_get_vrrp_by_vrid
(
    unsigned int vrid
);

/***************************
*Func:had_mac_check
*Param: 
*   vrrp_rt* vsrv,
*   char PktMac[],
*   unsigned int flag
*Use:
*   if two mac equal,return 0;
*   else return 1;
*
***************************/
int had_mac_check
(
   vrrp_rt* vsrv,
   unsigned char PktMac[],
   unsigned int flag /*up or down link*/
);

void had_updata
(
   int fd
);

int had_set_fd
(
	void
);

void had_read_sock_list
(
   fd_set readfds
);

int had_rx_packet
(
	vrrp_rt* vrrp,
	unsigned int ifindex,
	char* packet,
	int   len
);

void had_packet_detail
(
	unsigned char *buffer,
	int buffLen
);


void had_packet_thread2_main
(
	void
);

int had_check_br_muster
(
   char* brname,
   int*  radio_ifCount,
   int*  eth_ifCount
);

/*
  *******************************************************************************
  * had_state_sensitive_intf_check
  *
  *  DESCRIPTION:
  * 		check if the interface given is VRRP state sensitive.
  *
  *  INPUTS:
  *		ifname	- name of the interface
  *
  *  OUTPUTS:
  *		attr 		- 1 for VRRP state-sensitive interface,
  *				   0 for VRRP state non-sensitive inteface.
  *
  *  RETURN VALUE:
  *		0	- success
  *		1	- fail
  *
  *******************************************************************************
  */
int had_state_sensitive_intf_check
(
	char *ifname,
	unsigned int *attr
);
/*
 *******************************************************************************
 *had_check_ebr_state()
 *
 *  DESCRIPTION:
 *		get link-status of the special ebr
 *  INPUTS:
 *		char *ebrname		- name of the special ebr
 *
 *  OUTPUTS:
 *		char link_status			- INTERFACE_DOWN link-down,
 *							  INTERFACE_UP link-up
 *
 *  RETURN VALUE:
 *		0	- success
 *		1	- fail
 *
 *******************************************************************************
 */
int had_check_ebr_state
(
   char *ebrname,
   char *link_status
);

void had_set_link_state
(
    char* ifname,
    int   ifi_flags,
    vrrp_rt* vsrv
);

void had_read_link
(
	void* ptr,
	int totlemsglen
);

void had_update_uplink_downlink_state
(
	void
);

void had_get_netlink(int sockfd);

void had_link_thread_main
(
	void
);

void had_arp_thread_main
(
	void
);
#if 0
void had_timesyn_thread_main
(
	void
);
#endif
void vrrp_timer_thread_main
(
	void
);

/********************************************************
*	extern Functions									*
*********************************************************/
extern int vrrp_send_pkt(vrrp_rt* vsrv,char * ifname,char * buffer,int buflen);
extern int vrrp_dlt_len(vrrp_rt * rt);
extern int vrrp_in_chk_uplink(vrrp_rt * vsrv,struct iphdr * ip);
extern int vrrp_in_chk_downlink(vrrp_rt * vsrv,struct iphdr * ip);
unsigned int had_real_ipaddr_syn
(
	unsigned char * ifname, 
	unsigned int ifaddr, 
	unsigned int isAdd
);
int had_intf_netlink_get_ip_addrs
(
	unsigned char* ifName,
	unsigned int* ipAddrs,
	unsigned int* masks
) ;


#endif

