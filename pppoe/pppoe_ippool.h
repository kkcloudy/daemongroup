#ifndef _PPPOE_IPPOOL_H
#define _PPPOE_IPPOOL_H

typedef struct ippool_struct ippool_struct_t;

struct ipaddr_struct {
	uint32 ip;
	uint32 dns1;
	uint32 dns2;
};

int ipaddr_apply(ippool_struct_t *ippool, struct ipaddr_struct *ipaddr);
int ipaddr_recover(ippool_struct_t *ippool, struct ipaddr_struct *ipaddr);

ippool_struct_t *ippool_create(uint32 minIP, uint32 maxIP, uint32 dns1, uint32 dns2);
void ippool_destroy(ippool_struct_t **ippool);

#endif
