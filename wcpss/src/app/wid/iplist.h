#ifndef __IPLIST_H__
#define __IPLIST_H__
int ipaddr_list( int ifindex, uint32_t *array, int max_elem );
int ipaddr_list_v2(struct ifi_info *ifi,uint32_t *array, int max_elem );

#endif
