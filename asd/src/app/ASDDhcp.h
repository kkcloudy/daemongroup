#ifndef _ASD_DHCP_H
#define _ASD_DHCP_H
struct ip_info* dhcp_add_ip(struct vir_pool *pool,int ip);
struct ip_info* dhcp_assign_ip(struct vir_dhcp * vdhcp);
struct ip_info* dhcp_release_ip(struct vir_dhcp * vdhcp, int ip);

int dhcp_free_pool
(
	struct vir_pool *pool
);
struct ip_info *b_dhcp_assign_ip
(
	struct vir_dhcp *vdhcp,
	unsigned int viripaddr
);


#endif
