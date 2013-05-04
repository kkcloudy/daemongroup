#ifndef __SFD_H__
#define __SFD_H__

#include <linux/if_ether.h>

#define SFD_NETLINK_ID 31
#define SFD_MSGDATA_SIZE 256
#define SFD_MEMBERDATA_SIZE 128

typedef enum {
  SFD_LOG_INFO,
  SFD_LOG_DEBUG,
  SFD_LOG_ERR
} SfdLogSeverity;

typedef enum {
	sfdcmd_daemon,
	sfdcmd_logswitch,
	sfdcmd_debugswitch,
	sfdcmd_switch,
	sfdcmd_arpswitch,
	sfdcmd_timespan,
	sfdcmd_limitpacket,
	sfdcmd_arplimitpacket,
	sfdcmd_newmember,
	sfdcmd_delmember,
	sfdcmd_arpnewmember,
	sfdcmd_arpdelmember,
	sfdcmd_arpwarning,
	sfdcmd_variables,
} sfdcmd;

typedef enum {
	sfdiptype_ipv4,
	sfdiptype_ipv6
} iptype;

typedef struct {
	iptype type;
	unsigned char mac[ETH_ALEN];
	unsigned char ip[SFD_MEMBERDATA_SIZE];
} sfdMember;

typedef struct {
	sfdcmd cmd;
	int datalen;
	unsigned char data[SFD_MEMBERDATA_SIZE];
} sfdMsg;

#endif /* __SFD_H__ */
