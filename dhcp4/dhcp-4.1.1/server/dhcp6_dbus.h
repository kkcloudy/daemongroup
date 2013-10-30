#ifndef __DHCP6_DBUS_H__
#define __DHCP6_DBUS_H__

void dhcp6_dbus_start(void);
unsigned int dhcp_log_level;

/* VRRP state machine -- rfc2338.6.4 */
#define VRRP_STATE_INIT	1	/* rfc2338.6.4.1 */
#define VRRP_STATE_BACK	2	/* rfc2338.6.4.2 */
#define VRRP_STATE_MAST	3	/* rfc2338.6.4.3 */
#define VRRP_STATE_LEARN 4  /*after init,to learning state,then to mast or back*/
#define VRRP_STATE_NONE  5  
#define VRRP_STATE_TRANSFER  6 /*state that waiting synchronization data*/  

#define VRRP_STATE_DISABLE	99	/* internal */
#define FAILOVER_LOCAL_PORT 6060 
#ifdef _D_WCPSS_

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
int dhcp6_dbus_set_host
(
	struct dcli_host* addhost,
	unsigned int add
);

unsigned int
dhcp6_dbus_set_subnet
(
	struct dcli_subnet* addsubnet
);

int
dhcp6_dbus_set_option
(
	struct dcli_subnet* addsubnet, /*null is global option*/
	struct dcli_option* addoption,
	unsigned int del
);

unsigned int
dhcp6_dbus_find_subnet_by_ifname
(
	char* name,
	struct dcli_subnet** subnode
);

#endif

