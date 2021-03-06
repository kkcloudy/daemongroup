#ifndef __DCLI_DHCP6_H__
#define __DCLI_DHCP6_H__

#define DHCP_SERVER_SUBNET 1
#define DHCP_SERVER_HOST 2	
#define DHCP_SERVER_DOMAIN_NAME 3
#define DHCP_SERVER_DNS 4
#define DHCP_SERVER_WINS 5
#define DHCP_SERVER_MAX_LEASE_TIME 6
#define DHCP_SERVER_DEFAULT_LEASE_TIME 7

enum dhcpv6_debug_type {	
	DEBUG_TYPE_INFO  = 1,
	DEBUG_TYPE_ERROR = 2,
	DEBUG_TYPE_DEBUG = 4,
	DEBUG_TYPE_ALL   = 7
	
};
struct dhcpv6_statistics_info {
	//unsigned int host_num;		/* total number of ip address */
	//unsigned int segment_times;	/* ip address assigned */
	unsigned int dhcpv6_solicit_times;
	unsigned int dhcpv6_advertise_times;
	unsigned int dhcpv6_request_times;
	unsigned int dhcpv6_renew_times;
	unsigned int dhcpv6_reply_times;
};

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

struct iaddr {
	unsigned len;
	unsigned char iabuf [16];
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
unsigned int dcli_config_ipv6_pool_name
(
	char *poolName,	
	struct vty *vty
);

unsigned int dhcp_dcli_set_server_default_lease_time
(
	unsigned int defaulttime,
	char* poolname
);

/*use show dhcp6 */
struct dhcp6_show {
	unsigned int enable;	
	char* domainsearch;	
	struct iaddr dnsip[3];
	unsigned int dnsnum;	
	unsigned int defaulttime;
};

struct dhcp6_sub{
	char * range_low_ip;
	char * range_high_ip;
	unsigned int prefix_length;	
};

/*use show dhcp6 pool*/
struct dhcp6_pool_show
{
	char *poolname;
	char *domain_name;
	char *option52[8];
	unsigned int option_adr_num;
	char *dnsip[3];
	unsigned int dnsnum;	
	unsigned int defaulttime;
	unsigned int sub_count;
	struct dhcp6_sub* ipv6_subnet;
	char *interfacename;
};

#endif

