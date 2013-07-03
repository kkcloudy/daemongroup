#ifndef __DCLI_DHCP_H__
#define __DCLI_DHCP_H__

#define DHCP_SERVER_SUBNET 1
#define DHCP_SERVER_HOST 2	
#define DHCP_SERVER_DOMAIN_NAME 3
#define DHCP_SERVER_DNS 4
#define DHCP_SERVER_WINS 5
#define DHCP_SERVER_MAX_LEASE_TIME 6
#define DHCP_SERVER_DEFAULT_LEASE_TIME 7

#define DHCP_OPTION60_ID_DEFAULT_LENGTH		(256)	/* RFC1533 (255)*/
#define DHCP_OPTION60_DEFAULT_NUMBER		(4)
#define DHCP_OPTION60_INVALID_ID			(0)
#define DHCP_OPTION60_ADD_ID				(1)
#define DHCP_OPTION60_DELETE_ID				(2)
#define DHCP_CRIT_LOGFILE	"/etc/motd"

/* distrubuted */
#define INTERFACE_BIND_POOL			(1)
#define INTERFACE_UNBIND_POOL		(0)

#define DISTRIBUTED_FILE 			"/dbm/product/is_distributed"
#define DISTRIBUTED_MASTER_FILE		"/dbm/local_board/is_master"
#define DISTRIBUTED_ACTIVE_MASTER_FILE		"/dbm/local_board/is_active_master"
#define DISTRIBUTED_BOARD_SLOT_ID_FILE	"/dbm/local_board/slot_id"
#define IS_DISTRIBUTED				(1)
#define NOT_DISTRIBUTED				(0)
#define IS_MASTER_BOARD				(1)
#define NOT_MASTER_BOARD			(0)


enum dhcp_debug_type {	
	DEBUG_TYPE_INFO  = 1,
	DEBUG_TYPE_ERROR = 2,
	DEBUG_TYPE_DEBUG = 4,
	DEBUG_TYPE_DEBUG_FAILOVER_CONNECT = 8,
	DEBUG_TYPE_DEBUG_FAILOVER_MSG_DEAL = 16,
	DEBUG_TYPE_DEBUG_FAILOVER_ALL = 24,
	DEBUG_TYPE_ALL   = 31
	
};

struct iaddr {
	unsigned len;
	unsigned char iabuf [16];
};


struct statistics_info {
	unsigned int host_num;		/* total number of ip address */
	unsigned int segment_times;	/* ip address assigned */
	unsigned int discover_times;
	unsigned int offer_times;
	unsigned int requested_times;
	unsigned int ack_times;
};


/*lease state struct */
struct lease_state{
	unsigned int total_lease_num;
	unsigned int active_lease_num;
	unsigned int free_lease_num;
	unsigned int backup_lease_num;
};


/*subnet lease state*/
struct sub_lease_state{
	char *subnet;
	char *mask;
	char *poolname;
	struct statistics_info info;
	struct lease_state subnet_lease_state;
};

enum failover_state {
	unknown_state			=  0, /* XXX: Not a standard state. */
	startup				=  1,
	normal				=  2,
	communications_interrupted	=  3,
	partner_down			=  4,
	potential_conflict		=  5,
	recover				=  6,
	paused				=  7,
	shut_down			=  8,
	recover_done			=  9,
	resolution_interrupted		= 10,
	conflict_done			= 11,

	/* Draft revision 12 of the failover protocol documents a RECOVER-WAIT
	 * state, but does not enumerate its value in the section 12.24
	 * table.  ISC DHCP 3.0.x used value 254 even though the state was
	 * not documented at all.  For the time being, we will continue to use
	 * this value.
	 */
	recover_wait			= 254
};

struct dhcp_failover_state {
	enum failover_state local;
	enum failover_state peer;
};

/*when ipv4 len is 4 iabuf used 0 1 2 3*/
unsigned int dhcp_dcli_set_server_pool
(
	struct iaddr* lowip,
	struct iaddr* highip,
	struct iaddr* mask,
	char* interface,
	char* poolname
);

unsigned int dhcp_dcli_set_server_routers
(
	struct iaddr* routersip,
	char* poolname
);

unsigned int dhcp_dcli_set_server_host
(
	char* hostname,
	struct iaddr* hostip,
	char* hostmac,
	char* poolname
);

unsigned int dhcp_dcli_set_server_domain_name
(
	char* domainname,
	unsigned int namenum,
	char* poolname
);

unsigned int dhcp_dcli_set_server_dns
(
	struct iaddr** dnsip,
	unsigned int dnsnum,
	char* poolname
);

unsigned int dhcp_dcli_set_server_wins
(
	struct iaddr** winsip,
	unsigned int winsnum,
	char* poolname
);

unsigned int dhcp_dcli_set_server_max_lease_time
(
	unsigned int maxtime,
	char* poolname
);

unsigned int dhcp_dcli_set_server_default_lease_time
(
	unsigned int defaulttime,
	char* poolname
);
/*zhanglei add for distribute hansi show running*/
char * 
dcli_dhcp_show_running_hansi_cfg
(
	unsigned int slot_id,unsigned int InstID, unsigned int islocaled 
);
/*zhanglei add for wid distribute interface checking*/
unsigned int is_local_board_interface(const char *ifname);

char * 
dcli_dhcp_show_running_cfg2
(
	int slot_id
);

char * 
dcli_dhcrelay_show_running_cfg2
(
	struct vty *vty,
	int slot_id
);

char * 
dcli_dhcrelay_show_running_hansi_cfg
(
	unsigned int slot_id,unsigned int InstID, unsigned int islocaled 
);


#endif
