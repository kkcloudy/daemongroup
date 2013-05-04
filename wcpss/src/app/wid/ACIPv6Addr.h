
#ifndef AC_IPV6_ADDR_H
#define AC_IPV6_ADDR_H


#include "wcpss/waw.h"
#include "wcpss/wid/WID.h" 
#include "CWAC.h"
#include "ACDbus.h"
#include "ACDbus_handler.h"

#define PROC_IFINET6_PATH   "/proc/net/if_inet6"
#define INET6_ADDRLEN 128

struct tag_ipv6_addr{
		unsigned char ipv6addr[40];
		struct tag_ipv6_addr *next;
};

struct tag_ipv6_addr_list{
	struct tag_ipv6_addr *ipv6list;
	unsigned short ipv6num;
	unsigned short ifindex;
};

#define WIFI_IOC_GET_V6ADDR     _IOWR(243, 5, dev_ipv6_addr_t)

#define LINK_DOWN 0
#define LINK_UP 1
#define MAX_IPV6_ADDR_PER_DEV 128

typedef struct {
        unsigned char addr[16];
} ipv6_addr_t;

typedef struct {
        unsigned char ifname[16];
        int ifindex;
        unsigned short stat;
        unsigned short addr_cnt;
        ipv6_addr_t addr[MAX_IPV6_ADDR_PER_DEV];
} dev_ipv6_addr_t;

int get_if_addr_ipv6(const char *ifname, struct sockaddr_in6 *ipaddr,int *sysindex);
int get_if_addr_ipv6_list(const char *ifname,struct tag_ipv6_addr_list *ipv6list);
int ipv6_bind_interface_for_wid(struct ifi_info *ifi, int port);
void free_ipv6_addr_list(struct tag_ipv6_addr_list *ipv6list);
void display_ginterface_list();
void display_gmlltisock_list(CWMultiHomedSocket *sockPtr);
void display_ipv6_addr_list(struct tag_ipv6_addr_list *ipv6list);

struct tag_ipv6_addr_list * get_ipv6_addr_list(char * ifname);



#endif

