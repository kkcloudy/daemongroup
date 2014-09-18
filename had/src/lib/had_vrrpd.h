#ifndef __HAD_VRRPD_H__
#define __HAD_VRRPD_H__

/* system include */
#include <stdint.h>
#include <sys/time.h>
#define VRRPD_VERSION	"0.4"
/* Scott added 9-4-02 */
#include <syslog.h>
#include <netinet/ip.h>
#include <net/ethernet.h>

#define vrrpd_log syslog

//niehy add for ipv6 address,address is null return 1 
static inline int ipv6_addr_eq_null(const struct in6_addr *a1)
{
	return ((a1->s6_addr32[0]==0) && (a1->s6_addr32[1]==0)&&
		(a1->s6_addr32[2]==0) && (a1->s6_addr32[3]==0));
}
#define INET6_ADDRSTRLEN	(48)
#define NIP6QUAD(addr) \
	((__u8 *)&addr)[0], \
	((__u8 *)&addr)[1], \
	((__u8 *)&addr)[2], \
	((__u8 *)&addr)[3], \
	((__u8 *)&addr)[4], \
	((__u8 *)&addr)[5], \
	((__u8 *)&addr)[6], \
	((__u8 *)&addr)[7], \
	((__u8 *)&addr)[8], \
	((__u8 *)&addr)[9], \
	((__u8 *)&addr)[10], \
	((__u8 *)&addr)[11], \
	((__u8 *)&addr)[12], \
	((__u8 *)&addr)[13], \
	((__u8 *)&addr)[14], \
	((__u8 *)&addr)[15]

#define NIP6QUAD_FMT "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x"
extern int vrrp_ndisc_send_fd;
/*niehy add end*/


#ifndef _1K
#define _1K 1024
#endif

#define VRRP_MAC_ADDRESS_LEN		(6)
#define MAX_IFNAME_LEN 20
#define VRRP_ALL_THREAD_PID_PATH	"/var/run/vrrp_all.pid"
/*for hansi global mac*/
#define VRRP_DEVINFO_MAC_PATH  "/devinfo"
#define VRRP_DEVINFO_MAC_FILENAME "local_mac"
#define VRRP_DEVINFO_MAC_LEN  17 /*strlen("FF:FF:FF:FF:FF:FF")*/

#define VRRP_IFNAME_FILES_PATH  "/var/run/had/had"
#define VRRP_HEARTBEAT_IFNAME_FILENAME "heartbeat"
#define VRRP_HANSI_STATE_FILENAME "state"

extern unsigned char vrrp_global_mac[ETH_ALEN];
extern int vrrp_arp_send_fd;

#ifndef u_short
#define u_short unsigned short
#endif
#ifndef u_char
#define u_char unsigned char
#endif

#define VRRP_SERVICE_ENABLE		(1)		/* VRRP service enable */
#define VRRP_SERVICE_DISABLE	(0)		/* VRRP service disable */

#define SIOCSIFVMAC     0x893B		/* by guowei@autelan.com: Get global virtual mac       */
#define SIOCGIFVMAC     0x893C		/* by guowei@autelan.com: Set global virtual mac       */

/* VRRP notify flag */
#define VRRP_NOTIFY_BIT_WID		0x1
#define VRRP_NOTIFY_MASK_WID	0xFE
#define VRRP_NOTIFY_BIT_PORTAL	0x2
#define VRRP_NOTIFY_MASK_PORTAL 0xFD
#define VRRP_NOTIFY_BIT_DHCP	0x4
#define VRRP_NOTIFY_MASK_DHCP	0xFB
#define VRRP_NOTIFY_BIT_HMD	    0x8
#define VRRP_NOTIFY_MASK_HMD	0xFC/* faulse*/
#define VRRP_NOTIFY_BIT_PPPOE 0x10 //for pppoe
#define VRRP_NOTIFY_MASK_PPPOE 0xEF

/* now only support WID and PORTAL and DHCP service
 *  	notify obj:
 * 		(0 is off, 1 is on), default is open
 * 		bit[0] : WID(wireless-control)
 * 		bit[1] : PORTAL(easy-access-gateway)
 * 		bit[2] : DHCP(Dynamic Host Configuration Protocol)
 * 		...
 */
#define VRRP_NOTIFY_OBJ_DFL		(VRRP_NOTIFY_BIT_WID | VRRP_NOTIFY_BIT_PORTAL | VRRP_NOTIFY_BIT_DHCP|VRRP_NOTIFY_BIT_HMD|VRRP_NOTIFY_BIT_PPPOE)

#define VRRP_NOTIFY_ON			(0)		/* notify wid/portal	*/
#define VRRP_NOTIFY_OFF			(1)		/* no notify wid/portal */
#define VRRP_ON					(1)
#define VRRP_OFF				(0)
typedef enum continous_discrete_type{
	HAD_GRATUITOUS_ARP_CONTINOUS,
	HAD_GRATUITOUS_ARP_DISCRETE
}had_arp_send_mode;

#define MIN_CONTINOUS_VALUE 1
#define MAX_CONTINOUS_VALUE 300
#define MIN_DISCRETE_VALUE 1
#define MAX_DISCRETE_VALUE 50
#define DEFAULT_CONTINOUS_VALUE 10
#define DEFAULT_DISCRETE_VALUE 0


typedef enum
{
	VRRP_NOTIFY_OBJ_TPYE_WID = 0,		/* WID: wireless-control */
	VRRP_NOTIFY_OBJ_TPYE_PORTAL,		/* PORTAL: easy-access-gateway */
	VRRP_NOTIFY_OBJ_TYPE_DHCP,			/* DHCP: Dynamic Host Configuration Protocol */
	VRRP_NOTIFY_OBJ_TYPE_PPPOE,	/*PPPOE*/
	VRRP_NOTIFY_OBJ_TPYE_INVALID,
}VRRP_NOTIFY_OBJ_TPYE;


typedef enum notify_obj{
	VRRP_NOTIFY_NONE,
	VRRP_NOTIFY_TRAP,
	VRRP_NOTIFY_WID,
	VRRP_NOTIFY_PORTAL,
	VRRP_NOTIFY_DHCP,
	VRRP_NOTIFY_HMD,
	VRRP_NOTIFY_PPPOE,//for pppoe
	VRRP_NOTIFY_MAX
} vrrp_notify_obj;

/*
  * 	global flag indicates whether boot stage check is needed or not ( 1- needed, as default; 0 - no need).
  */
extern unsigned char g_boot_check_flag ;

/* while notifying to trap set to 1 ,wid set to 2,portal set to 3,dhcp set to 4 */
extern int global_notifying_flag;
extern char* global_notify_obj [VRRP_NOTIFY_MAX];
extern int global_notify_count [VRRP_NOTIFY_MAX];
#define NOTIFY_DEBUG_OUT_INTERVAL 5

/* link set state */
#define VRRP_LINK_NO_SETTED		(0)		/* this link has not setted */
#define VRRP_LINK_SETTED		(1)		/* this link has setted		*/

/* max count of uplink|downlink interface */
#define VRRP_LINK_MAX_CNT		(8)

typedef struct had_br_uplink_info_s{
	unsigned char ifname[MAX_IFNAME_LEN];
	unsigned int state;
	int setflag;
} l2uplink_portinfo;



/* used to configed vgateway_tf_flag manual,
 * STATE MACHINE could transformed to other state,
 * when anomaly(such as physical interface linkdown)
 * was occured.
 * 0: not transform to other state
 * 1: transform to other state
 */
#define VRRP_VGATEWAY_TF_FLG_OFF	(0)
#define VRRP_VGATEWAY_TF_FLG_ON		(1)

#define VRRP_SYS_COMMAND_LEN		(128)	/* hjw change 64 to 128 */
#define VRRP_INST_EAG_CHECK_FAILED	(-1)
#define VRRP_INST_EAG_CREATED		(1)		/* VRRP instance check EAG process have created.	*/
#define VRRP_INST_EAG_NO_CREATED	(0)		/* VRRP instance check EAG process have not created.*/


#if 0
#define VRRP_CHILD_PROCESS_CNT	(15)	/* count of parent process allowed to fork child process.
										 * count of had process is
										 *		(VRRP_CHILD_PROCESS_CNT + 1)
										 * in system.
										 */
#endif
#define VRRP_INSTANCE_CNT		(16)	/* count of VRRP instance allowed to created.
										 * for support V1.2, add instance 0,
										 * so VRRP instance count is 17 now.
										 */

/*
 * prefix/suffix string,
 * use to splice string about wid
*/
#define WID_BAK_OBJPATH_PREFIX "/aw/wid"
#define WID_BAK_OBJPATH_SUFFIX "/bak"
#define WID_BAK_INTERFACE_PREFIX "aw.wid"
#define WID_BAK_INTERFACE_SUFFIX ".bak"

/* local include */

typedef struct {	/* rfc2338.5.1 */
	uint8_t		vers_type;	/* 0-3=type, 4-7=version */
	uint8_t		vrid;		/* virtual router id */
	uint8_t		priority;	/* router priority */
	uint8_t		naddr;		/* address counter */
	uint8_t		auth_type;	/* authentification type */
	uint8_t		adver_int;	/* advertissement interval(in sec) */
	uint16_t	chksum;
	/*when a back dev can not received advertisement packets within ms_down_timer,
	 send msg with the followed ip*/										   	
	uint32_t     old_master_ip; 
			/* checksum (ip-like one) */
	uint32_t     uplink_ip;
	uint32_t     downlink_ip;/*these two used to force to fail!*/
	/*uplink or downlink flag*/
    uint32_t    updown_flag;
	/*just now state*/
	uint32_t    state;
/* here <naddr> ip addresses */
/* here authentification infos */
	/*heartbeatlink ip*/
	uint32_t heartbeatlinkip;
    /*querry flag*/
	uint32_t que_f;
    struct timeval tv;
} vrrp_pkt;

/* protocol constants */
#define INADDR_VRRP_GROUP 0xe0000012	/* multicast addr - rfc2338.5.2.2 */
#define VRRP_IP_TTL	255	/* in and out pkt ttl -- rfc2338.5.2.3 */
#define IPPROTO_VRRP	112	/* IP protocol number -- rfc2338.5.2.4*/
#define VRRP_VERSION	2	/* current version -- rfc2338.5.3.1 */
#define VRRP_PKT_ADVERT	1	/* packet type -- rfc2338.5.3.2 */
#define VRRP_PRIO_OWNER	255	/* priority of the ip owner -- rfc2338.5.3.4 */
#define VRRP_PRIO_DFL	100	/* default priority -- rfc2338.5.3.4 */
#define VRRP_PRIO_STOP	0	/* priority to stop -- rfc2338.5.3.4 */
#define VRRP_AUTH_NONE	0	/* no authentification -- rfc2338.5.3.6 */
#define VRRP_AUTH_PASS	1	/* password authentification -- rfc2338.5.3.6 */
#define VRRP_AUTH_AH	2	/* AH(IPSec) authentification - rfc2338.5.3.6 */
#define VRRP_AUTH_FAIL  3   /*if master received this, leave master and set state to none*/
#define VRRP_ADVER_DFL	2	/* advert. interval (in sec) -- rfc2338.5.3.7 */
#define VRRP_PREEMPT_DFL 1	/* rfc2338.6.1.2.Preempt_Mode */
#define VRRP_DEFAULT_DOWN_PACKET_COUNT 8/*3*/ /* default value for master to config down packet count *//*wuwl change from 3 to 8*/
#define VRRP_SMART_VMAC_ENABLE (1) /* Default value to config global virtual mac */

#define VRRP_AUTH_TYPE_DESCANT(authType)		\
({												\
	(authType == VRRP_AUTH_NONE) ? "AUTH_NONE" :	\
	(authType == VRRP_AUTH_PASS) ? "AUTH_PASS" :	\
	(authType == VRRP_AUTH_AH) ?   "AUTH_AH"   :	\
	(authType == VRRP_AUTH_FAIL) ? "AUTH_FAIL" :	\
	"error auth type";								\
})


#define VRRP_STATE_DESCRIPTION(state) \
	(state == VRRP_STATE_MAST) ? "MASTER" :	\
	(state == VRRP_STATE_BACK) ? "BACK" :	\
	(state == VRRP_STATE_DISABLE) ? "DISABLE" :	\
	(state == VRRP_STATE_LEARN) ? "LEARNING" :	\
	(state == VRRP_STATE_TRANSFER) ? "TRANSFER" : "INIT" \


/* packet from uplink or downlink flag */
#define VRRP_PKT_VIA_UPLINK		1
#define VRRP_PKT_VIA_DOWNLINK	2

/* implementation specific */
#define VRRP_PIDDIR_DFL	"/var/run"		/* dir to store the pid file */
#define VRRP_PID_FORMAT	"vrrpd_%s_%d.pid"	/* pid file format */

#define HAD_BOOT_CHECK_MSG_HZ	30


typedef struct {	/* parameters per interface -- rfc2338.6.1.1 */
	int 	set_flg;	/* flag of if the content be setted */
	int		auth_type;	/* authentification type. VRRP_AUTH_* */
	uint8_t		auth_data[8];	/* authentification data */

	uint32_t	ipaddr;		/* the address of the interface */
	struct in6_addr ipv6_addr;
	char		hwaddr[6];	/* WORK: lame hardcoded for ethernet !!!! */
	int         ifindex;
	int         mask;
	char		ifname[MAX_IFNAME_LEN];
							/* the device name for this ipaddr */
	l2uplink_portinfo l2uplinkinfo[4];
	char        linkstate;
} vrrp_if;

typedef struct {
	uint32_t	addr;		/* the ip address */
	uint32_t    mask;
	int		deletable;	/* TRUE if one of my primary addr */
} vip_addr;

/*niehy add for ipv6 address */
typedef struct {
	struct in6_addr sin6_addr;
	uint32_t    mask;
	int		deletable;	/* TRUE if one of my primary addr */
} vipv6_addr;
/*niehy add end*/

typedef struct dhcp_failover_s {
	uint32_t 	peerip; 	/* failover peer interface ip address */
	uint32_t 	localip; 	/* failover local interface ip address */
};

typedef struct vrrp_t{	/* parameters per virtual router -- rfc2338.6.1.2 */
    struct vrrp_t* next;
	int	vrid;		/* virtual id. from 1(!) to 255 */
	int profile;
	int	priority;	/* priority value */
	/*int naddr*/     /* number of ip addresses */
	int	uplink_naddr;		/* number of uplink ip addresses */
	int downlink_naddr;     /* number of downlink ip addresses */
	int vgateway_naddr;		/* number of vgateway ip addresses */
	int l2_uplink_naddr;    /* number of l2-uplink interface configured s*/
	/*vip_addr *vaddr*/        /* point on the ip address array */
	vip_addr uplink_vaddr[VRRP_LINK_MAX_CNT];
								/* point on the uplink ip address array */
	vip_addr downlink_vaddr[VRRP_LINK_MAX_CNT];
								/* point on the downlink ip address array */
	vip_addr vgateway_vaddr[VRRP_LINK_MAX_CNT];
	/*ipv6 address*/
	int	uplink_ipv6_naddr;		/* number of uplink ip addresses */
	int downlink_ipv6_naddr;     /* number of downlink ip addresses */
	int vgateway_ipv6_naddr;		/* number of vgateway ip addresses */
	int l2_uplink_ipv6_naddr;    /* number of l2-uplink interface configured s*/
	/*vipv6_addr *vaddr*/        /* point on the ip address array */
	vipv6_addr uplink_local_ipv6_vaddr[VRRP_LINK_MAX_CNT];
	vipv6_addr uplink_ipv6_vaddr[VRRP_LINK_MAX_CNT];
								/* point on the uplink ip address array */
	vipv6_addr downlink_local_ipv6_vaddr[VRRP_LINK_MAX_CNT];
	vipv6_addr downlink_ipv6_vaddr[VRRP_LINK_MAX_CNT];
								/* point on the downlink ip address array */
	vipv6_addr vgateway_local_ipv6_vaddr[VRRP_LINK_MAX_CNT];
	vipv6_addr vgateway_ipv6_vaddr[VRRP_LINK_MAX_CNT];
	int	adver_int;	/* delay between advertisements(in sec) */	

#if 0	/* dynamically calculated */
	double	skew_time;	/* skew Master_Down_Interval. (256-Prio)/256 */	
	int	mast_down_int;	/* interval for backup to declare master down*/
#endif
	int	smart_vmac_enable ;	/* true if a global virtual mac enable */
	int	preempt;	/* true if a higher prio preempt a lower one */
	unsigned char notify_flg;	/* notify obj: (0 is off, 1 is on)
								 * bit[0] : WID(wireless-control)
								 * bit[1] : PORTAL(easy-access-gateway)
								 * ...
								 */
	int	state;		/* internal state (init/backup/master) */
	int	wantstate;	/* user explicitly wants a state (back/mast) */

    int sockfd;      /* the socket descriptor */
    int uplink_fd[VRRP_LINK_MAX_CNT];
    int downlink_fd[VRRP_LINK_MAX_CNT];
	
	/*int	uplink_sockfd;	*/	/* the uplink socket descriptor */

	/*int	downlink_sockfd;	*/	/* the downlink socket descriptor */
	
	int	initF;		/* true if the struct is init */
	
	int	no_vmac;	/* dont handle the virtual MAC --rfc2338.7.3 */

	/* rfc2336.6.2 */
	
	/*uint32_t	ms_down_timer;*/
	/*uint32_t	adver_timer;*/
	int    uplink_ms_down_timer;
	int    downlink_ms_down_timer;
	int    uplink_adver_timer;
	int    downlink_adver_timer;
	
    /*modified for actual using*/
	/*vrrp_if vif */        
	vrrp_if	uplink_vif[VRRP_LINK_MAX_CNT]; //uplink
	vrrp_if downlink_vif[VRRP_LINK_MAX_CNT]; //downlink
	vrrp_if vgateway_vif[VRRP_LINK_MAX_CNT]; //vgateway
	vrrp_if l2_uplink_vif[VRRP_LINK_MAX_CNT]; //l2_uplink
	/*uplink or down link set*/
	uint32_t uplink_flag;
	uint32_t downlink_flag;
	uint32_t vgateway_flag;
	uint32_t l2_uplink_flag;

	uint32_t uplink_back_down_flag;      /* if on shutdown uplink sensitive interfaces when to back*/
	uint32_t downlink_back_down_flag;
	uint32_t vgateway_back_down_flag;
	uint32_t l2_uplink_back_down_flag;
	
	/* flag for vgateway configed manual,
	 * STATE MACHINE could transformed to other state,
	 * when anomaly(such as physical interface linkdown)
	 * was occured.
	 * 0: not transform to other state
	 * 1: transform to other state
	 */
	uint32_t vgateway_tf_flag;

	/*system time*/
	uint32_t que_f;/*querry flag for system time*/

	/* dhcp failover settings */
	struct dhcp_failover_s	failover;
	/* enter master or leave master times */
	unsigned int backup_switch_times;
	int vrrp_trap_sw;/*0--disable,1--enable*/
} vrrp_rt;

typedef struct vrrp_if_real_ip_s
{
	int set_flg;					/* flag for if the value setted(1: setted, 0: not setted) */
	char ifname[MAX_IFNAME_LEN];	/* interface name */
	int real_ip;					/* appoint real ip of the interface */
	struct in6_addr real_ipv6;
}vrrp_if_real_ip;

typedef struct hansi_struct_s {
    /*now just set vrid one*/
	int vrid;
	/*vrrp list in the hansi profile*/
	vrrp_rt* vlist;

	/*real ip configurations*/
	vrrp_if_real_ip uplink_real_ip[VRRP_LINK_MAX_CNT];
	vrrp_if_real_ip downlink_real_ip[VRRP_LINK_MAX_CNT];
	
    /*oeth operation*/
	void* func;
}hansi_s;

struct sock_s{
    struct sock_s* next;
	int sockfd;
	int uplink_fd[VRRP_LINK_MAX_CNT];
	int downlink_fd[VRRP_LINK_MAX_CNT];
	int vrid;
};
struct sock_list_t{
    struct sock_s*  sock_fd;
	int size;
	int count;
};
struct state_trace{
	int   profile;
	int   state;
    char* action;/*loop: stability state; goMaster: ->master;...*/
	char* step; /*which action is done*/
};
/* VRRP state machine -- rfc2338.6.4 */
#define VRRP_STATE_INIT	1	/* rfc2338.6.4.1 */
#define VRRP_STATE_BACK	2	/* rfc2338.6.4.2 */
#define VRRP_STATE_MAST	3	/* rfc2338.6.4.3 */
#define VRRP_STATE_LEARN 4  /*after init,to learning state,then to mast or back*/
#define VRRP_STATE_NONE  5  
#define VRRP_STATE_TRANSFER  6 /*state that waiting synchronization data*/  


#define VRRP_STATE_DISABLE	99	/* internal */

#define VRRP_AUTH_LEN	8

#define VRRP_IS_BAD_VID(id) ((id)<0 || (id)>255)	/* rfc2338.6.1.vrid */
#define VRRP_IS_BAD_PRIORITY(p) ((p)<1 || (p)>255)	/* rfc2338.6.1.prio */
#define VRRP_IS_BAD_ADVERT_INT(d) ((d)<1)



/* use the 'tcp sequence number arithmetic' to handle the wraparound.
** VRRP_TIMER_SUB: <0 if t1 precedes t2, =0 if t1 equals t2, >0 if t1 follows t2
*/
#if 0
#define VRRP_TIMER_SET( val, delta )	(val) = had_TIMER_CLK() + (delta)
#define VRRP_TIMER_SUB( t1, t2 ) ((int32_t)(((uint32_t)t1)-((uint32_t)t2)))
#define VRRP_TIMER_DELTA( val )		VRRP_TIMER_SUB( val, had_TIMER_CLK() )
#define VRRP_TIMER_EXPIRED( val )	((val) && VRRP_TIMER_DELTA(val)<=0)
#define VRRP_TIMER_CLR( val ) 		(val) = 0
#define VRRP_TIMER_IS_RUNNING( val )	(val)
#define VRRP_TIMER_HZ			1000000
#else
#define VRRP_TIMER_SET( val, delta )	(val) =(delta)
#define VRRP_TIMER_SUB( t1, t2 ) ((int32_t)(((uint32_t)t1)-((uint32_t)t2)))
#define VRRP_TIMER_DELTA( val )		(val)
#define VRRP_TIMER_EXPIRED( val )	(val <= 0)
#define VRRP_TIMER_CLR( val ) 		(val) = 0
#define VRRP_TIMER_IS_RUNNING( val )	(val)
#define VRRP_TIMER_HZ			1000000
#define VRRP_TIMER_DECREASE(val)	(val -= VRRP_TIMER_HZ)
#endif
         
#define VRRP_TIMER_SKEW( srv ) ((256-(srv)->priority)*VRRP_TIMER_HZ/256) 

#define VRRP_MIN( a , b )	( (a) < (b) ? (a) : (b) )
#define VRRP_MAX( a , b )	( (a) > (b) ? (a) : (b) )


void vrrp_update
(
   vrrp_rt* vsrv,
   char* uplink_buf,
   char* downlink_buf,
   unsigned int uplink_len,
   unsigned int downlink_len
);

/********************************************************
*	function declare									*
*********************************************************/

uint32_t had_TIMER_CLK
(
	void
);

struct sock_list_t* had_LIST_CREATE
(
   void
);

/*
 *******************************************************************************
 *had_LIST_ADD()
 *
 *  DESCRIPTION:
 *		add socket fd about heartbeat|uplink|downlink to socklist.
 *  INPUTS:
 *		vrrp_rt* vsrv
 *
 *  OUTPUTS:
 *		struct sock_s* sock_fd		- new struct of socket fd
 *
 *  RETURN VALUE:
 *		NULL						- add faild
 *		sock_fd						- add success
 *
 *******************************************************************************
 */
struct sock_s* had_LIST_ADD
(
    vrrp_rt* vsrv
);

/*
 *******************************************************************************
 *had_LIST_UPDATE()
 *
 *  DESCRIPTION:
 *		update socket fd about heartbeat|uplink|downlink to socklist.
 *		find the socket fd which vrid is special vrid.
 *		found it, update. not found, add new one.
 *
 *  INPUTS:
 *		vrrp_rt* vsrv
 *
 *  OUTPUTS:
 *		struct sock_s* sock_fd		- new struct of socket fd
 *
 *  RETURN VALUE:
 *		NULL						- update faild
 *		sock_fd						- update success
 *
 *******************************************************************************
 */
struct sock_s* had_LIST_UPDATE
(
    vrrp_rt* vsrv,
    unsigned int vrid
);

/*
 *******************************************************************************
 *had_LIST_DEL()
 *
 *  DESCRIPTION:
 *		delete socket fd about heartbeat|uplink|downlink from socklist.
 *
 *  INPUTS:
 *		vrrp_rt* vsrv
 *
 *  OUTPUTS:
 * 	 	NULL
 *
 *  RETURN VALUE:
 * 	 	NULL
 *
 *******************************************************************************
 */
void had_LIST_DEL
(
    vrrp_rt* vsrv
);

/*
 *******************************************************************************
 *vrrp_clear_link_fd()
 *
 *  DESCRIPTION:
 *		clear socket fd on vrrp_rt (heartbeat & uplink & downlink).
 *		Support multi-uplink and multi-downlink.
 *
 *  INPUTS:
 *		vrrp_rt *vsrv,
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		NULL
 *
 *******************************************************************************
 */
void vrrp_clear_link_fd
(
	vrrp_rt *vsrv
);


int vrrp_add_to_multicast
(
    vrrp_rt* vrrp,
    int uplink_ip,
    int downlink_ip
);

/****************************************************************
 NAME	: get_status_from_ifname			00/02/08 06:51:32
 AIM	:
 REMARK	:
****************************************************************/
uint32_t had_ifname_to_status
(
	vrrp_rt* vrrp,
	char* ifname
);

/*
 *******************************************************************************
 *had_ifname_to_status_byIoctl()
 *
 *  DESCRIPTION:
 *		get link-status of the special link type.
 *		Support multi-uplink and multi-downlink.
 *
 *  INPUTS:
 *		int	linkType		- type of the special link
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		unsigned int link_status
 *			 				- INTERFACE_DOWN link-down,
 *							  INTERFACE_UP link-up
 *
 *******************************************************************************
 */
unsigned int vrrp_get_link_state
(
	vrrp_rt *vsrv,
	int linkType
);

/*
 *******************************************************************************
 *had_ifname_to_status_byIoctl()
 *
 *  DESCRIPTION:
 *		get link-status of the special interface
 *  INPUTS:
 *		char *ifname		- name of the special interface
 *
 *  OUTPUTS:
 *		unsigned int *link_status
 *			 				- INTERFACE_DOWN link-down,
 *							  INTERFACE_UP link-up
 *
 *  RETURN VALUE:
 *		0	- success
 *		1	- fail
 *
 *******************************************************************************
 */
uint32_t had_ifname_to_status_byIoctl
(
	char *ifname,
	unsigned int *link_status
);

uint32_t vrrp_get_ifname_linkstate
(
	char* ifname
);

/*
 *******************************************************************************
 *had_ifname_to_ip()
 *
 *  DESCRIPTION:
 *		get interface ip address by ifname.
 *
 *  INPUTS:
 *		char *ifname,			- interface name
 *
 *  OUTPUTS:
 *		unsigned int *addr		- ip address
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_ERR	- faild
 *		VRRP_RETURN_CODE_OK		- success
 *
 *******************************************************************************
 */
unsigned int had_ifname_to_ip
(
	char *ifname,
	unsigned int *addr
);

/****************************************************************
 NAME	: get_dev_from_ip			00/02/08 06:51:32
 AIM	:
 REMARK	:
****************************************************************/
int had_ifname_to_idx
(
	char *ifname
);

/*
 *******************************************************************************
 *had_hwaddr_get()
 *
 *  DESCRIPTION:
 *		get interface mac address by ifname.
 *
 *  INPUTS:
 *		char *ifname,			- interface name
 *		int addrlen				- length of mac address
 *
 *  OUTPUTS:
 *		char *addr				- mac address
 *
 *  RETURN VALUE:
 *		-1				- faild
 *		0				- success
 *
 *******************************************************************************
 */
int had_hwaddr_get
(
	char *ifname,
	char *addr,
	int addrlen
);

/****************************************************************
 NAME	: vrrp_dlthd_len			00/02/02 15:16:23
 AIM	: return the vrrp header size in byte
 REMARK	:
****************************************************************/
 int vrrp_dlt_len
 (
	vrrp_rt *rt
 );

/****************************************************************
 NAME	: vrrp_in_chk				00/02/02 12:54:54
 AIM	: check a incoming packet. return 0 if the pkt is valid, != 0 else
 REMARK	: rfc2338.7.1
****************************************************************/
 int vrrp_in_chk_uplink
(
	vrrp_rt *vsrv,
	struct iphdr *ip
);

/****************************************************************
 NAME	: vrrp_in_chk				00/02/02 12:54:54
 AIM	: check a incoming packet. return 0 if the pkt is valid, != 0 else
 REMARK	: rfc2338.7.1
****************************************************************/
int vrrp_in_chk_downlink
(
	vrrp_rt *vsrv,
	struct iphdr *ip
);

/****************************************************************
 NAME	: vrrp_send_pkt				00/02/06 16:37:10
 AIM	:
 REMARK	:
****************************************************************/
int vrrp_send_pkt
(
	vrrp_rt* vsrv,
	char* ifname,
	char *buffer,
	int buflen
);


/****************************************************************
 NAME	: vrrp_send_pkt				00/02/06 16:37:10
 AIM	:
 REMARK	:
****************************************************************/
int vrrp_send_pkt_directly
(
	vrrp_rt* vsrv,
	char* ifname,
	char *buffer,
	int buflen 
);

/****************************************************************
 NAME	: vrrp_send_pkt				00/02/06 16:37:10
 AIM	:
 REMARK	:
****************************************************************/
int vrrp_send_pkt_arp
(
	char* ifname,
	char *buffer,
	int buflen 
);


/****************************************************************
 NAME	: vrrp_send_adv				00/02/06 16:31:24
 AIM	:
 REMARK	:
****************************************************************/
int vrrp_send_adv
(
	vrrp_rt *vsrv,
	int prio
);

void vrrp_state_trace
(
  int profile,
  int state,
  char* action,
  char* step
);

/*
 *******************************************************************************
 *had_cfg_delete_uplink_ipaddr()
 *
 *  DESCRIPTION:
 *		delete virtual ip of uplink.
 *
 *  INPUTS:
 *		vrrp_rt *vsrv,					- vrrps
 *		unsigned int ipaddr,			- virtual ip
 *		unsigned int mask,				- mask
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		NULL
 *
 *******************************************************************************
 */
void had_cfg_delete_uplink_ipaddr
(
	vrrp_rt *vsrv,
	unsigned int ipaddr,
	unsigned int mask,
	int index
);

/*
 *******************************************************************************
 *had_cfg_delete_downlink_ipaddr()
 *
 *  DESCRIPTION:
 *		delete virtual ip of downlink.
 *
 *  INPUTS:
 *		vrrp_rt *vsrv,					- vrrps
 *		unsigned int ipaddr,			- virtual ip
 *		unsigned int mask,				- mask
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		NULL
 *
 *******************************************************************************
 */
void had_cfg_delete_downlink_ipaddr
(
	vrrp_rt *vsrv,
	unsigned int ipaddr,
	unsigned int mask,
	int index
);

/*
 *******************************************************************************
 *had_get_link_index_by_ifname()
 *
 *  DESCRIPTION:
 *		check if the interface in link,
 *		if exist, output its index.
 *		if not exist, output empty first position index.
 *
 *  INPUTS:
 *		vrrp_rt *vsrv,					- vrrps
 *		char *ifname,					- interface name
 *		unsigned int up_down_flg,		- link type
 *
 *  OUTPUTS:
 *		int *index						- link index		
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_IF_EXIST		- the interface is exist.
 *		VRRP_RETURN_CODE_IF_NOT_EXIST	- the interface is not exist.
 *
 *******************************************************************************
 */
unsigned int had_get_link_index_by_ifname
(
	vrrp_rt *vsrv,
	char *ifname,
	unsigned int up_down_flg,
	int *index
);

/*
 *******************************************************************************
 *had_get_link_realip_index_by_ifname()
 *
 *  DESCRIPTION:
 *		check if the interface in real ip array,
 *		if exist, output its index.
 *		if not exist, output empty first position index.
 *
 *  INPUTS:
 *		vrrp_rt *vsrv,					- vrrps
 *		char *ifname,					- interface name
 *		unsigned int up_down_flg,		- link type
 *
 *  OUTPUTS:
 *		int *index						- link index		
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_IF_EXIST		- the interface is exist.
 *		VRRP_RETURN_CODE_IF_NOT_EXIST	- the interface is not exist.
 *
 *******************************************************************************
 */
unsigned int had_get_link_realip_index_by_ifname
(
	hansi_s *hansi,
	char *ifname,
	unsigned int up_down_flg,
	int *index
);

/*
 *******************************************************************************
 *had_check_link_vip_exist()
 *
 *  DESCRIPTION:
 *		check if the virtual ip exist.
 *
 *  INPUTS:
 *		vrrp_rt *vsrv,					- vrrps
 *		unsigned int uplink_ip,			- virtual ip
 *		unsigned int uplink_mask,		- mask
 *		unsigned int up_down_flg		- link type
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_VIP_NOT_EXIST		- the virtual ip is not exist.
 *		VRRP_RETURN_CODE_VIP_EXIST			- the virtual ip is exist.
 *
 *******************************************************************************
 */
unsigned int had_check_link_vip_exist
(
	vrrp_rt *vsrv,
	unsigned int vip,
	unsigned int mask,
	unsigned int up_down_flg
);

/*
 *******************************************************************************
 *had_add_vip_to_linkvif()
 *
 *  DESCRIPTION:
 *		add interface to uplink|downlink.
 *
 *  INPUTS:
 *		int profile,					- hansi profile
 *		vrrp_rt *vsrv,					- vrrps
 *		unsigned int up_down_flg		- link type
 *		char * ifname,					- interface name
 *		int index						- add position
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_ERR		- add faild
 *		VRRP_RETURN_CODE_OK			- add success
 *
 *******************************************************************************
 */
unsigned int had_add_vip_to_linkvif
(
	int profile,
	vrrp_rt *vsrv,
	unsigned int up_down_flg,
	char *ifname,
	int index
);

/*
 *******************************************************************************
 *had_del_vip_from_linkvif()
 *
 *  DESCRIPTION:
 *		delete interface from uplink_vif|downlink_vif array.
 *
 *  INPUTS:
 *		vrrp_rt *vsrv,					- vrrps
 *		unsigned int up_down_flg		- link type
 *		char * ifname,					- interface name
 *		int index						- add position
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_ERR		- delete faild
 *		VRRP_RETURN_CODE_OK			- delete success
 *
 *******************************************************************************
 */
unsigned int had_del_vip_from_linkvif
(
	vrrp_rt *vsrv,
	unsigned int up_down_flg,
	char * ifname,
	int index
);

/*
 *******************************************************************************
 *had_add_vip()
 *
 *  DESCRIPTION:
 *		add link virtual ip,
 *		before add must setted linkmode(linktype + priority)
 *
 *  INPUTS:
 *		unsigned int profile,			- vrid
 *		char *uplink_ifname,			- uplink interface name
 *		unsigned long uplink_ip,		- virtual ip
 *		unsigned int uplink_mask,		- mask
 *		unsigned int up_down_flg		- link type
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_ERR						- null parameter or error linktype
 *		VRRP_RETURN_CODE_PROFILE_NOTEXIST			- profile not exist
 *		VRRP_RETURN_CODE_SERVICE_NOT_PREPARE		- service enabled
 *		VRRP_RETURN_CODE_LINKMODE_NO_SETTED			- not setted linkmode
 *		VRRP_RETURN_CODE_VIP_EXIST					- the virtual ip has exist
 *
 *******************************************************************************
 */
unsigned int had_add_vip
(
	unsigned int profile,
	char *ifname,
	unsigned long vip,
	unsigned int mask,
	unsigned int up_down_flg
);

/*
 *******************************************************************************
 *had_delete_vip()
 *
 *  DESCRIPTION:
 *		delete link virtual ip,
 *		before delete must be sure setted linkmode(linktype + priority),
 *		and virtual ip which configured by "config uplink | downlink priority"
 *		is not allow to delete.
 *
 *  INPUTS:
 *		unsigned int profile,			- vrid
 *		char *uplink_ifname,			- uplink interface name
 *		unsigned long uplink_ip,		- virtual ip
 *		unsigned int uplink_mask,		- mask
 *		unsigned int up_down_flg		- link type
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_ERR						- null parameter or error linktype
 *		VRRP_RETURN_CODE_PROFILE_NOTEXIST			- profile not exist
 *		VRRP_RETURN_CODE_SERVICE_NOT_PREPARE		- service enabled
 *		VRRP_RETURN_CODE_LINKMODE_NO_SETTED			- not setted linkmode
 *		VRRP_RETURN_CODE_VIP_NOT_EXIST				- the virtual ip has not exist
 *
 *******************************************************************************
 */
unsigned int had_delete_vip
(
	unsigned int profile,
	char *ifname,
	unsigned long vip,
	unsigned int mask,
	unsigned int up_down_flg
);

/*
 *******************************************************************************
 * had_cfg_dhcp_failover()
 *
 *  DESCRIPTION:
 *		config dhcp failover settings such as peer and local interface's ip address.
 *
 *  INPUTS:
 *		profile	- hansi profile
 *		peerip 	- peer side ip address
 *		localip	- loca ip address
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_PROFILE_NOTEXIST		- profile not exist
 *		VRRP_RETURN_CODE_SERVICE_NOT_PREPARE	- instance is not prepare ok
 *		VRRP_RETURN_CODE_OK						- set success
 *
 *******************************************************************************/
int had_cfg_dhcp_failover
(
	int profile,
	uint32_t peerip,
	uint32_t localip
);

/****************************************************************
 NAME	: had_send_gratuitous_arp			00/05/11 11:56:30
 AIM	:
 REMARK	: rfc0826
	: WORK: ugly because heavily hardcoded for ethernet
****************************************************************/
int had_send_gratuitous_arp
(
	char* ifname,
	char* mac,
	int addr
);


/****************************************************************
 NAME	: had_send_vgateway_gratuitous_arp			00/05/11 11:56:30
 AIM	:
 REMARK	: rfc0826
	: WORK: ugly because heavily hardcoded for ethernet
****************************************************************/
int had_send_vgateway_gratuitous_arp
(
	vrrp_rt *vsrv,
	int addr,
	int index,
	int vAddrF
);

/****************************************************************
 NAME	: had_send_uplink_gratuitous_arp			00/05/11 11:56:30
 AIM	:
 REMARK	: rfc0826
	: WORK: ugly because heavily hardcoded for ethernet
****************************************************************/
int had_send_uplink_gratuitous_arp
(
	vrrp_rt *vsrv,
	int addr,
	int index,
	int vAddrF
);

/****************************************************************
 NAME	: had_send_downlink_gratuitous_arp			00/05/11 11:56:30
 AIM	:
 REMARK	: rfc0826
	: WORK: ugly because heavily hardcoded for ethernet
****************************************************************/
int had_send_downlink_gratuitous_arp
(
	vrrp_rt *vsrv,
	int addr,
	int index,
	int vAddrF
);

/****************************************************************
 NAME	: state_gotomaster			00/02/07 00:15:26
 AIM	:
 REMARK	: called when the state is now MASTER
****************************************************************/
char *had_ipaddr_to_str
(
	uint32_t ipaddr
);

vrrp_rt* had_check_if_exist
(
   unsigned int vrid
);

int had_get_profile_by_vrid
(
   int vrid
);

int had_notify_to_npd
(
    char* ifname1,
    char* ifname2
);

#if 0
/*******************************************
* func:had_notify_to_wid
* param: u_int32 vrid
*           u_int32 state
*           u_int32 ip--if state == back,ip is the master real ip
*                            if state == master of disable,is null
*attention:
*             no
*
********************************************/
int had_notify_to_protal
(
    int vrid,
    int state,
    int uplinkip,
    int downlinkip,
    int virtual_uplink_ip,
    int virtual_downlink_ip
);

/*******************************************
* func:had_notify_to_protal
* param: u_int32 vrid
*           u_int32 state
*           u_int32 ip--if state == back,ip is the master real ip
*                            if state == master of disable,is null
*attention:
*             no
*
********************************************/
int had_notify_to_wid
(
    int vrid,
    int state,
    int uplinkip,
    int downlinkip,
    int virtual_uplink_ip,
    int virtual_downlink_ip
);
#endif

#if 1
/*
 *******************************************************************************
 *had_get_link_first_setted_index()
 *
 *  DESCRIPTION:
 *		check if link(uplink or downlink) is setted,
 *		if setted, output its first setted index.
 *		if not setted, output -1.
 *
 *  INPUTS:
 *		vrrp_rt *vsrv,					- vrrps
 *		unsigned int up_down_flg,		- link type
 *
 *  OUTPUTS:
 *		int *index						- link index		
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_IF_EXIST		- the interface is exist.
 *		VRRP_RETURN_CODE_IF_NOT_EXIST	- the interface is not exist.
 *
 *******************************************************************************
 */
unsigned int had_get_link_first_setted_index
(
	vrrp_rt *vsrv,
	unsigned int up_down_flg,
	int *index
);

int had_notify_to_protal
(
	vrrp_rt *vrrp,
	int state
);

int had_notify_to_wid
(
	vrrp_rt *vrrp,
    int state
);

#endif
void had_notify_to_dhcp
(
	vrrp_rt* vsrv,
	int addF
);

void had_notify_to_trap
(
	vrrp_rt* vsrv
);

void had_notify_to_snmp_mib
(
	vrrp_rt* vsrv
);


/*
 *******************************************************************************
 *had_set_link_auth_type()
 *
 *  DESCRIPTION:
 *		set link authentification type of the special link type(uplink or downlink).
 *		Support multi-uplink and multi-downlink.
 *
 *  INPUTS:
 *		vrrp_rt *vsrv,				- vrrps
 *		unsigned int linkType,		- link type
 *		unsigned int authType		- authentification type
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_ERR		- failed
 *		VRRP_RETURN_CODE_OK			- success
 *
 *******************************************************************************
 */
unsigned int had_set_link_auth_type
(
	vrrp_rt *vsrv,
	unsigned int linkType,
	unsigned int authType
);

/*
 *******************************************************************************
 *had_link_auth_type_cmp()
 *
 *  DESCRIPTION:
 *		compare link authentification type of the special link type(uplink or downlink)
 *		with parameter authentification type.
 *		Support multi-uplink and multi-downlink.
 *
 *  INPUTS:
 *		vrrp_rt *vsrv,				- vrrps
 *		unsigned int linkType,		- link type
 *		unsigned int authType		- authentification type
 *
 *  OUTPUTS:
 *		NULL
 *
 *  RETURN VALUE:
 *		0		- equal
 *		1		- not equal
 *
 *******************************************************************************
 */
unsigned int had_link_auth_type_cmp
(
	vrrp_rt *vsrv,
	unsigned int linkType,
	unsigned int authType
);

/****************************************************************
 NAME	: had_state_leave_master			00/02/07 00:15:26
 AIM	:
 REMARK	: called when the state is no more MASTER
****************************************************************/
void had_state_leave_master
(
	vrrp_rt *vsrv,
	int advF,
	struct iphdr* uplink_iph,
	struct iphdr* downlink_iph
);

/****************************************************************
 NAME	: had_state_goto_disable			00/02/07 00:15:26
 AIM	:
 REMARK	: called when the state is no more MASTER
****************************************************************/
void had_state_goto_disable
(
	vrrp_rt *vsrv,
	int advF,
	struct iphdr* uplink_iph,
	struct iphdr* downlink_iph
);

int had_state_from_disable
(
	vrrp_rt* vsrv,
	vrrp_pkt* hd
);

int had_state_from_learn
(
   vrrp_rt* vsrv,
   vrrp_pkt* hd
);

int had_state_back
(
   vrrp_rt* vsrv,
   vrrp_pkt* hd
);

void had_set_heartbeatlink_sock
(
  void
);

/****************************************************************
 NAME	: had_open_sock				00/02/07 12:40:00
 AIM	: open the socket and join the multicast group.
 REMARK	:
****************************************************************/
int had_open_sock
(
	vrrp_rt *vsrv
);

int had_init_trace
(
	int profile
);

void hansi_init
(
	void
);

int had_get_runing_hansi
(
   void
);

int had_start
(
   unsigned int profile,
   unsigned int priority,
   char*  uplink_if,
   unsigned long  uplink_ip,
   unsigned int   uplink_mask,
   char*  downlink_if,
   unsigned long downlink_ip,
   unsigned int  downlink_mask
);

int had_v_gateway
(
   unsigned int profile,
   char*  vgateway_if,
   unsigned long  vgateway_ip ,
   unsigned int mask
);;

int had_no_v_gateway
(
   unsigned int profile,
   char*  vgateway_if,
   unsigned long  vgateway_ip ,
   unsigned int mask
);

int had_end
(
   unsigned int profile
);

int had_priority_cfg
(
   int profile,
   int priority
);

int had_global_vmac_cfg
(
   int profile,
   int g_vmac 
);

int had_preempt_cfg
(
   int profile,
   int preempt
);

/*
 *******************************************************************************
 *had_set_notify_obj_on_off()
 *
 *  DESCRIPTION:
 *		set notify obj and on/off flg
 *
 *  INPUTS:
 * 		int profile,
 *		unsigned char notify_obj,
 *		unsigned char notify_flg
 *
 *  OUTPUTS:
 * 	 	NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_PROFILE_NOTEXIST		- profile not exist
 *		VRRP_RETURN_CODE_SERVICE_NOT_PREPARE	- instance is not prepare ok
 *		VRRP_RETURN_CODE_OK						- set success
 *
 *******************************************************************************
 */
unsigned int had_set_notify_obj_on_off
(
	int profile,
	unsigned char notify_obj,
	unsigned char notify_flg	
);

/*
 *******************************************************************************
 *had_set_vgateway_tf_flag_on_off()
 *
 *  DESCRIPTION:
 *		set vgateway transform flag on/off
 *
 *  INPUTS:
 * 		int profile,
 *		unsigned int vgateway_tf_flg
 *
 *  OUTPUTS:
 * 	 	NULL
 *
 *  RETURN VALUE:
 *		VRRP_RETURN_CODE_PROFILE_NOTEXIST		- profile not exist
 *		VRRP_RETURN_CODE_SERVICE_NOT_PREPARE	- instance is not prepare ok
 *		VRRP_RETURN_CODE_VGATEWAY_NO_SETTED		- vgataway not setted
 *		VRRP_RETURN_CODE_OK						- set success
 *
 *******************************************************************************
 */
unsigned int had_set_vgateway_tf_flag_on_off
(
	int profile,
	unsigned int vgateway_tf_flg	
);

int had_advert_cfg
(
   int profile,
   int time
);

int had_mac_cfg
(
   int profile,
   int mac
);

vrrp_rt* had_show_cfg
(
   int profile
);

int had_get_min_advtime
(
	void
);

void had_sleep_time
(
	void
);

void had_sleep_seconds
(
	int second
);


void had_update_uplink
(
   vrrp_rt* vsrv,
   char* uplink_buf,
   unsigned int uplink_len  
);

void had_update_downlink
(
   vrrp_rt* vsrv,
   char* downlink_buf,
   unsigned int downlink_len  
);

#if 0
/* delete by jinpc@autelan.com
 * for not in use
 */
vrrp_rt* vrrp_get_instance_by_ifname
(
	char* ifname
);
#endif

/**********************************************************************************
 * had_check_boot_status
 *	This method check up boot flag to verify that system is in boot stage or not.
 *
 *	INPUT:
 *		profile - hansi prifle id
 *	
 *	OUTPUT:
 *		NULL
 *
 * 	RETURN:
 *		-1 - if check boot status files error
 *		0 - system is not in boot stage(in running stage)
 *		1 - system is in boot stage
 *		
 *	NOTE:
 *
 **********************************************************************************/
int had_check_boot_status
(
	int profile
);

void had_state_thread_main
(
	void
);

int had_init
(
	void
);


/*
 *******************************************************************************
 *had_wid_init()
 *
 *  DESCRIPTION:
 *		use vrrp instance no splice string
 *			global_wid_dbusname
 *			global_wid_bak_objpath
 *			global_wid_bak_interfce,
 *		for send dbus message to wid
 *
 *  INPUTS:
 * 	 	NULL
 *
 *  OUTPUTS:
 * 	 	NULL
 *
 *  RETURN VALUE:
 *
 *******************************************************************************
 */
void had_wid_init
(
	void
);

/********************************************************
*	extern Functions									*
*********************************************************/
unsigned int had_br_uplink_rcv_pck(vrrp_rt* vsrv);
unsigned int had_br_uplink_info_get(vrrp_rt* vsrv);
unsigned int had_br_uplink_info_clear(vrrp_rt* vsrv);
unsigned int had_br_uplink_drop_pck(vrrp_rt* vsrv);
unsigned int had_vrrp_vif_drop_pck
(
	vrrp_rt* vsrv
);
unsigned int had_vrrp_vif_rcv_pck_ON
(
	vrrp_rt* vsrv
);

unsigned int had_vrrp_vif_rcv_pck_OFF
(
	vrrp_rt* vsrv
);

uint32_t had_set_intf_state_byIoctl
(
	char *ifname,
	unsigned int rx_flag
);

#endif	/* __VRRP_H__ */


