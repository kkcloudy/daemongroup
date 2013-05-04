#ifndef _PPPOE_UTIL_H
#define _PPPOE_UTIL_H

static inline long 
time_compare(long lt, long rt) {
	return (lt - rt);
}

long time_sysup(void);
uint32 get_product_info(char *filename);
uint32 ifname_get_slot_id(const char *ifname);
int netmask_check(uint32 mask);
int set_nonblocking(int fd);
void rand_seed_init(uint8 *seed, uint32 size);
void rand_bytes(uint32 *seed, uint8 *buf, uint32 size);

#endif
