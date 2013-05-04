#ifndef __DHCP_DBUS_H__
#define __DHCP_DBUS_H__

void dhcp_dbus_start(void);

/* VRRP state machine -- rfc2338.6.4 */
#define VRRP_STATE_INIT	1	/* rfc2338.6.4.1 */
#define VRRP_STATE_BACK	2	/* rfc2338.6.4.2 */
#define VRRP_STATE_MAST	3	/* rfc2338.6.4.3 */
#define VRRP_STATE_LEARN 4  /*after init,to learning state,then to mast or back*/
#define VRRP_STATE_NONE  5  
#define VRRP_STATE_TRANSFER  6 /*state that waiting synchronization data*/  

#define VRRP_STATE_DISABLE	99	/* internal */
#define FAILOVER_LOCAL_PORT 6060 

#define DHCP_OPTION60_INVALID_ID			(0)
#define DHCP_OPTION60_ADD_ID				(1)
#define DHCP_OPTION60_DELETE_ID				(2)

#define SET_INTERFACE_IP_POOL			(1)		/* interface bind ip pool */
#define CLR_INTERFACE_IP_POOL			(0)		/* interface no ip pool */

#define INTERFACE_BIND_FLAG				(0)		/* interface bind flag */
#define INTERFACE_UNBIND_FLAG			(1)		/* interface unbind flag */


#if (defined _D_WCPSS_ || defined _D_CC_)

typedef enum{
	check_vrrid = 0,
	check_wtpid,
	check_sub,	
	check_sub1,
	check_radioid,
	check_wlanid,
	check_point,
	check_fail,
	check_end,
	check_success
}radio_ifname_state;

#define PARSE_RADIO_IFNAME_SUB '-'
#define PARSE_RADIO_IFNAME_POINT '.'

#endif

struct neigh_tbl_info
{
	unsigned char mac[6];
	unsigned int  ipAddr;
	unsigned int  ifIndex;
	unsigned short state;
};

struct distributed_iface_info {
	char ifname[IFNAMSIZ];	
	int slotid;
	int vrrpid;
	int ifnameid;
	int local_flag;
};


int
dhcp_dbus_set_host
(
	struct dcli_host* addhost,
	unsigned int add
);

unsigned int
dhcp_dbus_set_subnet
(
	struct dcli_subnet* addsubnet
);

int 
dhcp_dbus_set_option
(
	struct dcli_subnet* addsubnet,
	struct dcli_option* owned_option,
	unsigned int del
);

unsigned int
dhcp_dbus_find_subnet_by_ifname
(
	char* name,
	struct dcli_subnet** subnode
);

unsigned int
dhcp_dbus_four_char_to_int
(
	unsigned char* in_char,
	unsigned int *out_int
);

struct dcli_pool *dhcp_dbus_find_poolnode_by_subnet(struct subnet *subnet);

void 
dhcp_notify_to_trap_event
(
	unsigned int flags, /*0 not full 1 full*/
	unsigned int total_count,
	unsigned int count,
	struct subnet *set_subnet
);

void 
dhcp_tell_whoami
(
	char * myName,
	int isLast
);

int dhcp_dbus_do_failover(vrrp_param_t *params);

#endif
