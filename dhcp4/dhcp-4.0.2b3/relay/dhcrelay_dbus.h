#ifndef __DHCRELAY_DBUS_H__
#define __DHCRELAY_DBUS_H__

void dhcrelay_dbus_start(void);

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

#endif

