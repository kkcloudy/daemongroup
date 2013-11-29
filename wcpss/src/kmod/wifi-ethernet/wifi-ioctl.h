
#ifndef __WIFI_IOCTL_H__
#define __WIFI_IOCTL_H__

#define MAX_IPV6_ADDR_PER_DEV		128
#define LINK_DOWN					0
#define LINK_UP						1

typedef struct interface_INFO
{
	unsigned char if_name[15];
	unsigned char	wlanID;
	unsigned int BSSIndex;
	unsigned int vrid;	
	unsigned int isIPv6;
	unsigned int acip;
	unsigned int apip;
	unsigned char acipv6[IPv6_LEN];
	unsigned char apipv6[IPv6_LEN];
	unsigned int protect_type;
	unsigned short acport;
	unsigned short apport;
	unsigned char bssid[MAC_LEN];	
	unsigned char apmac[MAC_LEN];
	unsigned char acmac[MAC_LEN];
	unsigned char ifname[ETH_LEN];	
	unsigned char WLANID;
	unsigned char wsmswitch;
	unsigned char if_policy;		/*0-local,1-tunnel,ht add 110314*/
	unsigned char Eap1XServerMac[MAC_LEN];	
	unsigned char Eap1XServerSwitch;
	unsigned char f802_3;//if 1---capwap+802.3,else 0---802.11
	unsigned short	vlanid;
	unsigned char vlanSwitch;	
	unsigned char apname[DEFAULT_LEN];
	unsigned char essid[ESSID_LEN+1];
}IF_info;

struct interface_basic_INFO{
	char if_name[15];
	unsigned char wlanID;
    int    BSSIndex;	
	unsigned int vrid;
};

#define PATCH_OP_RADIO_MAX 80
struct interface_batch_INFO{
	int count;
	struct interface_basic_INFO ifinfo[PATCH_OP_RADIO_MAX];
};


typedef struct vrid_shared_mem{
	unsigned int vrid;
	unsigned long long mm_addr;
}sh_mem_t;

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

extern unsigned long long vm[VRID_MAX];
extern struct net_device *alloc_etherdev(int sizeof_priv);
extern struct net_device *alloc_etherdev_mq(int sizeof_priv, unsigned int queue_count);

int get_ipv6_addr(dev_ipv6_addr_t *ipv6_addr);
int dynamic_update_if(struct interface_INFO *if_info);
struct wifi_bss_tbl * wifi_bssid_bssidx_tbl_get(unsigned char *bssid);
struct wifi_sta_tbl *wifi_sta_tbl_add(const  char*s);
struct wifi_sta_tbl *wifi_sta_tbl_get(const char *s);
int  wifi_sta_tbl_del(char *s);

#endif
