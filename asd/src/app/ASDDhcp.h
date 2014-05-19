#ifndef _ASD_DHCP_H
#define _ASD_DHCP_H
struct ip_info* dhcp_add_ip(struct vir_pool *pool, unsigned int ip);
struct ip_info* dhcp_assign_ip(struct vir_dhcp * vdhcp,unsigned char *mac);
struct ip_info *dhcp_release_ip
(
	struct vir_dhcp * vdhcp, 
	unsigned int ip,
	unsigned char *mac
);

int dhcp_free_pool
(
	struct vir_pool *pool
);

void dhcp_cancel_delete_timeout_virlease_cache
(
	struct vir_dhcp *vdhcp
);
struct ip_info *b_dhcp_assign_ip
(
	struct vir_dhcp *vdhcp,
	unsigned int viripaddr
);

void delete_timeout_virlease_cache(void *circle_ctx,void *timeout_ctx);
struct ip_info *dhcp_get_sta_by_random
(
	struct vir_dhcp *vdhcp
);
void dhcp_mac_hash_add
(
	struct vir_dhcp *vdhcp,
	struct ip_info *sta
);

#endif
