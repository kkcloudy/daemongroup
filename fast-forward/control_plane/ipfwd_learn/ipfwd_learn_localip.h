#ifndef _IPFWD_LEARN_LOCALIP_H_
#define _IPFWD_LEARN_LOCALIP_H_

/* we want filter local ip, so we make a tiny hash table to keep local ip. */
/* when system add/del/update ip configure, we will be  notified, and we will update hash table. */
/* use fwd_ifa_table_lookup(ip) to filter local ip */
/* 2012-12-04. add by zhaohan@autelan.com */
struct fwd_ifaddr_entry
{
    struct hlist_node hlist;
	struct in_device *ifa_dev;    
	u32 ifa_address;
	u32	ifa_mask;    
};

#define FWD_IFA_HASH_SIZE 64

struct fwd_ifaddr_entry *fwd_ifa_table_lookup(const uint32_t ip);
void fwd_ifa_hash_rebuild(void);
void fwd_ifa_hash_init(void);
void fwd_ifa_hash_release(void);

#endif
