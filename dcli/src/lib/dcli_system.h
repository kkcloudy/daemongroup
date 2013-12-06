#ifndef __DCLI_SYSTEM_H__
#define __DCLI_SYSTEM_H__

#define DCLI_DEBUG_FLAG_ALL       0xFF
#define DCLI_DEBUG_FLAG_DBG       0x1
#define DCLI_DEBUG_FLAG_WAR       0x2
#define DCLI_DEBUG_FLAG_ERR       0x4
#define DCLI_DEBUG_FLAG_EVT       0x8
#define DCLI_DEBUG_FLAG_PKT_REV   0x10
#define DCLI_DEBUG_FLAG_PKT_SED   0x20
#define DCLI_DEBUG_FLAG_PKT_ALL   0x30

#define DCLI_SYSTEM_CPU_ALL_QUEUE	8

#define MAX_VLAN_ID 4094
#define MIN_VLAN_ID 1
#define NPD_ERR_SYSTEM_MAC  3
#define MAX_IFNAME_LEN 20


typedef struct {
	unsigned char arEther[6];
}ETHERADDR;

#define DIAGNOSIS_STR "diagnosis hardware information\n"

/*#define DEBUG_STR "Config system debugging\n"*/
#define NODEBUG_STR	"Cancel system debugging\n"
#define MODULE_DEBUG_STR(module)	"Config "#module" debugging\n"
#define MODULE_DEBUG_LEVEL_STR(module,level) 	"Open "#module" debug level "#level"\n"
typedef struct
{
    unsigned int outUcFrames;
    unsigned int outMcFrames;
    unsigned int outBcFrames;
    unsigned int brgEgrFilterDisc;
    unsigned int txqFilterDisc;
    unsigned int outCtrlFrames;
} PORT_EGRESS_CNTR_STC;
typedef struct
{
    unsigned int gtHostInPkts;
    unsigned int gtHostOutPkts;
    unsigned int gtHostOutBroadcastPkts;
    unsigned int gtHostOutMulticastPkts;
} BRIDGE_HOST_CNTR_STC;

typedef enum arp_filter_type{
	ARP_FILTER_NONE,
	ARP_FILTER_IP,
	ARP_FILTER_IFINDEX,
	ARP_FILTER_MAC,
	ARP_FILTER_STATE
} arp_filter_type_e;

static unsigned char * arp_state_str(unsigned int state, unsigned char * outStr);

#define SYS_TECH_SUPPORT_SCRIPT		"/etc/tech.support"

#endif
