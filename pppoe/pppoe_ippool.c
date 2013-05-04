#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pppoe_def.h"
#include "pppoe_priv_def.h"

#include "pppoe_log.h"
#include "pppoe_ippool.h"


struct ippool_struct {
	uint32 startIP, totalNUM;
	uint32 dns1, dns2;
	struct ipaddr_struct **pool;
};

static inline uint32
ipaddr_to_index(uint32 startIP, uint32 ip) {
	return ip - startIP - ((ip >> 8) - (startIP >> 8)) * 2;
}

static inline uint32
index_to_ipaddr(uint32 startIP, uint32 index) {
	uint32 num = index / 0xfe;
	uint32 remain = index - (num * 0xfe);

	if ((remain + (startIP & 0xff)) >= 0xff) {
		return startIP + (num << 8) + remain + 2;
	} 
	return startIP + (num << 8) + remain;	
}

static inline uint32
ippool_get_free_index(ippool_struct_t *ippool) {
	uint32 i;
	for (i = 0; i < ippool->totalNUM; i++) {
		if (NULL == ippool->pool[i])
			return i;
	}
	
	return ippool->totalNUM; /* can not find free index */
}

int
ipaddr_apply(ippool_struct_t *ippool, struct ipaddr_struct *ipaddr) {
	uint32 index;

	if (unlikely(!ipaddr)) {
		return PPPOEERR_EINVAL;
	}

	if (ipaddr->ip) {
		index = ipaddr_to_index(ippool->startIP, ipaddr->ip);
	} else {
		index = ippool_get_free_index(ippool);
	}

	if (unlikely(index >= ippool->totalNUM)) {
		return PPPOEERR_ENOMEM;
	}

	ipaddr->ip = ipaddr->ip ? : 
				index_to_ipaddr(ippool->startIP, index);
	ipaddr->dns1 = ippool->dns1;
	ipaddr->dns2 = ippool->dns2;
	ippool->pool[index] = ipaddr;
	return PPPOEERR_SUCCESS;
}

int
ipaddr_recover(ippool_struct_t *ippool, struct ipaddr_struct *ipaddr) {
	uint32 index;

	if (unlikely(!ipaddr || !ipaddr->ip))
		return PPPOEERR_EINVAL;

	index = ipaddr_to_index(ippool->startIP, ipaddr->ip);
	if (unlikely(index >= ippool->totalNUM
		|| ipaddr != ippool->pool[index]))
		return PPPOEERR_ENOEXIST;

	memset(ipaddr, 0, sizeof(*ipaddr));
	ippool->pool[index] = NULL;
	return PPPOEERR_SUCCESS;
}

ippool_struct_t *
ippool_create(uint32 minIP, uint32 maxIP, uint32 dns1, uint32 dns2) {
	ippool_struct_t *ippool;
	
	minIP = (minIP & 0xff) ? (0xff == (minIP & 0xff) ? (minIP + 2) : minIP) : (minIP + 1);
	maxIP = (maxIP & 0xff) ? (0xff == (maxIP & 0xff) ? (maxIP - 1) : maxIP) : (maxIP - 2);

	if (minIP > maxIP) {
		pppoe_log(LOG_ERR, "ip pool input ip error\n");
		goto error;
	}
	
	ippool = (ippool_struct_t *)malloc(sizeof(ippool_struct_t));
	if (unlikely(!ippool)) {
		pppoe_log(LOG_ERR, "malloc ip pool failed\n");
		goto error;
	}

	memset(ippool, 0, sizeof(*ippool));
	ippool->startIP = minIP;
	ippool->totalNUM = maxIP - minIP + 1 - ((maxIP >> 8) - (minIP >> 8)) * 2;
	ippool->dns1 = dns1;
	ippool->dns2 = dns2;
	ippool->pool = (struct ipaddr_struct **)calloc(ippool->totalNUM, sizeof(struct ipaddr_struct *));
	if (unlikely(!ippool->pool)) {
		pppoe_log(LOG_ERR, "ip pool calloc %u failed\n", ippool->totalNUM);
		goto error1;
	}

	pppoe_log(LOG_INFO, "ippool create start ip %u.%u.%u.%u and total %u\n",
						HIPQUAD(ippool->startIP), ippool->totalNUM);
	return ippool;

error1:	
	PPPOE_FREE(ippool);	
error:
	return NULL;
}


void
ippool_destroy(ippool_struct_t **ippool) {
	if (unlikely(!ippool || !(*ippool)))
		return ;

	PPPOE_FREE((*ippool)->pool);
	PPPOE_FREE(*ippool);
}


#if 0

#define IPPOOL_MINIP ((120 << 24) + (1 << 16) + (1 << 8) + 255) 
#define IPPOOL_MAXIP ((120 << 24) + (1 << 16) + (10 << 8) + 0)

int
main(void) {
	ippool_struct_t *ippool;
	struct ipaddr_struct array[1024];
	int i, ret;
	
	ippool = ippool_create(IPPOOL_MINIP, IPPOOL_MAXIP, 0, 0);
	if (!ippool) {
		printf("ippool create failed\n");
		return -1;
	}

	memset(array, 0, sizeof(array));
	for (i = 0; i < ARRAY_SIZE(array); i++) {
		ret = ipaddr_apply(ippool, &array[i]);
		if (ret) {
			printf("%d apply ipaddr failed, ret %d\n", i, ret);
		} else {
			printf("%d apply ipaddr %u.%u.%u.%u\n", i, HIPQUAD(array[i].ip));
		}
	}

	for (i = 0; i < ARRAY_SIZE(array); i++) {
 		printf("%d recover ipaddr %u.%u.%u.%u\n", i, HIPQUAD(array[i].ip));
		ret = ipaddr_recover(ippool, &array[i]);
		if (ret) {
			printf("%d recover ipaddr failed, ret %d\n", i, ret);
		} 
	}

	ippool_destroy(&ippool);
	return 0;
}

#endif
