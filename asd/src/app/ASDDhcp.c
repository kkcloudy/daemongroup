#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <dbus/dbus.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "asd.h"
#include "ASDDhcp.h"
#include "circle.h"

#define IP_HASH(ip) (ip[0]+ip[1]+ip[2]+ip[3])
#define MAC_HASH(mac) (mac[2]+mac[3]+mac[4]+mac[5])
#define VIR_LEASE_CACHE_TIME_OUT  (60*15)  /*15 minutes*/

struct ip_info *dhcp_get_sta_by_ip
(
	struct vir_pool *pool,
	unsigned int ip
)
{
	struct ip_info *s = NULL;
	unsigned char * tip = (unsigned char*)&(ip);
	s = pool->ip_hash[IP_HASH(tip)];
	
	while (s != NULL && (ip != s->ip))
	{
		s = s->hnext;
	}
	
	return s;
}

struct ip_info *dhcp_get_sta_by_mac_cache
(
	struct vir_dhcp *vdhcp,
	unsigned char *mac
)
{	
	struct ip_info *s = NULL;
	
	if(NULL == vdhcp || NULL == mac)
		return NULL;

	s = vdhcp->cache_hash[MAC_HASH(mac)];
	
	while (s != NULL)
	{
		if(0 == os_strcmp((char*)s->mac, (char*)mac))
		{
			return s;
		}
		s = s->mhnext;
	}
	
	return s;
}

struct ip_info *dhcp_get_sta_by_random
(
	struct vir_dhcp *vdhcp
)
{	
	struct ip_info *s = NULL;
	unsigned int i = 0;
	
	if(NULL == vdhcp)
		return NULL;
	
	for(i=0; i<VIR_DHCP_HASH_SIZE; i++)
	{
		s = vdhcp->cache_hash[i];
		if(s != NULL)
		{
			return s;
		}
	}
	
	return s;
}


/* only remove next point, DON't free the node */
static void dhcp_ip_list_del
(
	struct vir_pool *pool,
	struct ip_info *ip
)
{
	struct ip_info *tmp = NULL;

	if (pool->ip_list == ip)
	{
		pool->ip_list = ip->next;
		return;
	}

	tmp = pool->ip_list;
	while (tmp != NULL && tmp->next != ip)
	{
		tmp = tmp->next;
	}
	
	if (tmp != NULL)
	{
		tmp->next = ip->next;
		return;
	}
	return;
}


void dhcp_ip_hash_add
(
	struct vir_pool *pool,
	struct ip_info *ip
)
{
	unsigned char *tip = NULL;

	if (NULL == pool 
		|| NULL == ip)
	{
		return;
	}

	tip = (unsigned char*)&(ip->ip);
	
	ip->hnext = pool->ip_hash[IP_HASH(tip)];
	pool->ip_hash[IP_HASH(tip)] = ip;

	return;
}


static void dhcp_ip_hash_del
(
	struct vir_pool *pool,
	struct ip_info *ip
)
{
	struct ip_info *s = NULL;
	unsigned char *tip = (unsigned char*)&(ip->ip);
	
	s = pool->ip_hash[IP_HASH(tip)];
	if (s == NULL)
	{
		return;
	}

	/* head node in hash list */
	if (s->ip == ip->ip)
	{
		pool->ip_hash[IP_HASH(tip)] = s->hnext;
		return;
	}

	/* other node in hash list */
#if 0
	while (s->hnext != NULL
			&& (s->ip != ip->ip))
	{
		s = s->hnext;
	}
	
	if (s->hnext != NULL)
	{
		s->hnext = s->hnext->hnext;
	}
#else
	while (s->hnext != NULL)
	{
		if (s->hnext->ip == ip->ip)
		{
			s->hnext = s->hnext->hnext;

			return;
		}

		s = s->hnext;
	}
#endif

	return;	
}

void dhcp_mac_hash_add
(
	struct vir_dhcp *vdhcp,
	struct ip_info *sta
)
{
	ASD_CHECK_POINTER(vdhcp);
	ASD_CHECK_POINTER(sta);

	sta->mhnext = vdhcp->cache_hash[MAC_HASH(sta->mac)];
	vdhcp->cache_hash[MAC_HASH(sta->mac)] = sta;

	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s: add sta "MACSTR" vir ip  %s \n",
							__func__, MAC2STR(sta->mac),u32ip2str(sta->ip));	

	return;
}

static void dhcp_mac_hash_del
(
	struct vir_dhcp *vdhcp,
	struct ip_info *sta
)
{
	struct ip_info *s = NULL;

	ASD_CHECK_POINTER(vdhcp);
	ASD_CHECK_POINTER(sta);
	
	s = vdhcp->cache_hash[MAC_HASH(sta->mac)];
	if (s == NULL)
	{
		return;
	}

	/* head node in hash list */
	if (0 == os_strcmp((char*)s->mac, (char*)sta->mac))
	{
		vdhcp->cache_hash[MAC_HASH(sta->mac)] = s->mhnext;
		return;
	}

	/* other node in hash list */
	while (s->mhnext != NULL)
	{
		if (0 == os_strcmp((char*)s->mhnext->mac, (char*)sta->mac))
		{
			s->mhnext = s->mhnext->mhnext;

			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s: del sta "MACSTR" vir ip  %s \n",
							__func__, MAC2STR(sta->mac),u32ip2str(sta->ip));	

			return;
		}

		s = s->mhnext;
	}

	asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s: no found sta "MACSTR" vir ip  %s \n",
				__func__, MAC2STR(sta->mac),u32ip2str(sta->ip));	
	return;	
}

struct ip_info *dhcp_add_ip
(
	struct vir_pool *pool,
	unsigned int ip
)
{
	struct ip_info *tmp = NULL;

	/* check the ip is exist in vir-dhcp pool */
	tmp = dhcp_get_sta_by_ip(pool, ip);
	if (tmp != NULL)
	{
		return tmp;
	}

	/* add ip to vir-dhcp pool */
	tmp = (struct ip_info*)malloc(sizeof(struct ip_info));
	if (NULL == tmp)
	{
		return NULL;
	}
	memset(tmp, 0, sizeof(struct ip_info));
	tmp->next = NULL;
	tmp->hnext = NULL;
	tmp->mhnext = NULL;
	tmp->ip = ip;

	tmp->next = pool->ip_list;
	pool->ip_list = tmp;
	pool->ipnum++;

	/* add ip to hash-table */
	dhcp_ip_hash_add(pool, tmp);
	
	return tmp;
}

int dhcp_free_pool
(
	struct vir_pool *pool
)
{
	struct ip_info *tmp = NULL;

	if (NULL == pool)
	{
		return -1;
	}

	tmp = pool->ip_list;
	while (NULL != tmp)
	{
		/* delete ip from hash-table */
		dhcp_ip_hash_del(pool, tmp);
		tmp->hnext = NULL;

		/* remove ip from ip_list */	
		pool->ip_list = tmp->next;
		if (NULL != tmp)
		{
			free(tmp);
			pool->ipnum--;
			tmp = pool->ip_list;
		}
	}
	
	return 0;
}

void dhcp_cancel_delete_timeout_virlease_cache
(
	struct vir_dhcp *vdhcp
)
{
	ASD_CHECK_POINTER(vdhcp);

	struct ip_info *s = NULL;
	int i=0 ;
	int removed = 0;
	
	for(i=0; i<VIR_DHCP_HASH_SIZE;i++)
	{
		s = vdhcp->cache_hash[i];
		
		while(s != NULL)
		{
			removed = circle_cancel_timeout(delete_timeout_virlease_cache, vdhcp, s);

			asd_printf(ASD_DEFAULT,MSG_DEBUG, "%s: cancel timeout vir ip %s for sta "MACSTR" num %d\n",
						__func__, u32ip2str(s->ip), MAC2STR(s->mac), removed);	
								
			s = s->mhnext;
		}
	}
		
	
}

struct ip_info *dhcp_assign_ip
(
	struct vir_dhcp *vdhcp,
	unsigned char *mac
)
{
	struct ip_info *tmp = NULL;
	int removed = 0;

	if(NULL == vdhcp || NULL == mac)
		return NULL;

	/*check whether sta is in cache*/
	tmp = dhcp_get_sta_by_mac_cache(vdhcp,mac);
	if(tmp != NULL)
	{
		removed = circle_cancel_timeout(delete_timeout_virlease_cache, vdhcp, tmp);

		asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s: cancel timeout vir ip %s for sta "MACSTR" num %d\n",
				__func__, u32ip2str(tmp->ip),MAC2STR(tmp->mac), removed);
		
		dhcp_mac_hash_del(vdhcp,tmp);
		tmp->mhnext = NULL;
		
		tmp->next = vdhcp->dhcplease.ip_list;
		
		vdhcp->dhcplease.ip_list = tmp;
		
		dhcp_ip_hash_add(&(vdhcp->dhcplease),tmp);
		
		vdhcp->dhcplease.ipnum++;	
		
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s: assign vip %s from cache\n",
				__func__, u32ip2str(tmp->ip));	

		return tmp;
	}
	else if (vdhcp->dhcpfree.ip_list != NULL)
	{
		tmp = vdhcp->dhcpfree.ip_list;
		vdhcp->dhcpfree.ip_list = tmp->next;
		if (NULL == vdhcp->dhcpfree.ip_list)
		{
			vdhcp->dhcpfree.last = NULL;
		}
		tmp->next = NULL;
		dhcp_ip_hash_del(&(vdhcp->dhcpfree), tmp);
		tmp->hnext = NULL;
		vdhcp->dhcpfree.ipnum--;
		
		tmp->next = vdhcp->dhcplease.ip_list;
		vdhcp->dhcplease.ip_list = tmp;
		dhcp_ip_hash_add(&(vdhcp->dhcplease),tmp);
		vdhcp->dhcplease.ipnum++;

		asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s: assign vip  %s from free \n",
		__func__, u32ip2str(tmp->ip));	
		
		return tmp;
	}
	else
	{
		tmp = dhcp_get_sta_by_random(vdhcp);
		if(tmp == NULL)
		{
			return NULL;
		}
		else
		{
			removed = circle_cancel_timeout(delete_timeout_virlease_cache, vdhcp, tmp);

			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s: cancel timeout vir ip %s for sta "MACSTR" num %d\n",
				__func__, u32ip2str(tmp->ip),MAC2STR(tmp->mac), removed);
			
			dhcp_mac_hash_del(vdhcp,tmp);
			
			tmp->mhnext = NULL;
			
			tmp->next = vdhcp->dhcplease.ip_list;
			
			vdhcp->dhcplease.ip_list = tmp;
			
			dhcp_ip_hash_add(&(vdhcp->dhcplease),tmp);
			
			vdhcp->dhcplease.ipnum++;	

			asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s: assign vip  %s from cache when free NULL\n",
					__func__, u32ip2str(tmp->ip));	

			return tmp;
			
		}
	}
}

struct ip_info *dhcp_release_ip
(
	struct vir_dhcp * vdhcp, 
	unsigned int ip,
	unsigned char *mac
)
{
	struct ip_info *tmp = NULL;
	
	tmp = dhcp_get_sta_by_ip(&(vdhcp->dhcplease), ip);
	if(tmp != NULL)
	{
		/* Note: first delete node in hash list, second delete node in ip_list*/
		dhcp_ip_hash_del(&(vdhcp->dhcplease), tmp);
		tmp->hnext = NULL;
		
		dhcp_ip_list_del(&(vdhcp->dhcplease), tmp);
		tmp->next = NULL;
		
		vdhcp->dhcplease.ipnum--;

		asd_printf(ASD_DEFAULT,MSG_INFO,"%s: release %s\n", __func__, u32ip2str(ip));

		memcpy(tmp->mac, mac, MAC_LEN);
		
		dhcp_mac_hash_add(vdhcp, tmp);

		circle_register_timeout(VIR_LEASE_CACHE_TIME_OUT, 0, 
							delete_timeout_virlease_cache, vdhcp, tmp);
		
		/*
		#if 0
		if (vdhcp->dhcpfree.last && (vdhcp->dhcpfree.last != tmp))
		{
			vdhcp->dhcpfree.last->next = tmp;
			tmp->next = NULL;
		}
		else
		#endif
		{
			prev = vdhcp->dhcpfree.ip_list;
			while (prev && prev->next)
			{
				prev = prev->next;
			}
			if (prev)
			{
				prev->next = tmp;
				tmp->next = NULL;
			}
			else
			{
				vdhcp->dhcpfree.ip_list = tmp;
				tmp->next = NULL;
			}			
		}
		
		vdhcp->dhcpfree.last = tmp;

		dhcp_ip_hash_add(&(vdhcp->dhcpfree),tmp);
		vdhcp->dhcpfree.ipnum++;
		memset(tmp->mac, 0, MAC_LEN);
		*/
	}
	return tmp;
}


struct ip_info *b_dhcp_assign_ip
(
	struct vir_dhcp *vdhcp,
	unsigned int viripaddr
)
{
	struct ip_info *next = NULL, *tmp = NULL;


	if (NULL == vdhcp->dhcpfree.ip_list)
	{
		asd_printf(ASD_DEFAULT,MSG_ERROR,"%s: vir dhcp no free ip\n", __func__);			
	
		return NULL;
	}

	if (viripaddr == vdhcp->dhcpfree.ip_list->ip)
	{		
		tmp = vdhcp->dhcpfree.ip_list;
		vdhcp->dhcpfree.ip_list = tmp->next;
		if (NULL == vdhcp->dhcpfree.ip_list)
		{
			vdhcp->dhcpfree.last = NULL;
		}
		tmp->next = NULL;
		dhcp_ip_hash_del(&(vdhcp->dhcpfree), tmp);
		tmp->hnext = NULL;
		vdhcp->dhcpfree.ipnum--;
		
		tmp->next = vdhcp->dhcplease.ip_list;
		vdhcp->dhcplease.ip_list = tmp;
		dhcp_ip_hash_add(&(vdhcp->dhcplease),tmp);
		vdhcp->dhcplease.ipnum++;
		
		return tmp;
	}
	else
	{
		next = vdhcp->dhcpfree.ip_list;

		while ((NULL != next) && (NULL != next->next) && (viripaddr != next->next->ip))
		{
			next = next->next;
		}

		if ((NULL == next) || ((NULL == next->next)))
		{
			return NULL;
		}

		tmp = next->next;
		next->next = tmp->next;
		
		tmp->next = NULL;
		dhcp_ip_hash_del(&(vdhcp->dhcpfree), tmp);
		tmp->hnext = NULL;
		vdhcp->dhcpfree.ipnum--;		

		if (NULL == vdhcp->dhcpfree.ip_list)
		{
			vdhcp->dhcpfree.last = NULL;
		}
		
		tmp->next = vdhcp->dhcplease.ip_list;
		vdhcp->dhcplease.ip_list = tmp;
		dhcp_ip_hash_add(&(vdhcp->dhcplease),tmp);
		vdhcp->dhcplease.ipnum++;
		return tmp;
	}

	return NULL;
}

void delete_timeout_virlease_cache(void *circle_ctx,void *timeout_ctx)
{
	struct vir_dhcp *vdhcp = circle_ctx;
	struct ip_info *sta = timeout_ctx;
	struct ip_info *tmp = NULL, *prev = NULL;
	
	ASD_CHECK_POINTER(vdhcp);
	ASD_CHECK_POINTER(sta);	
	
	tmp = dhcp_get_sta_by_mac_cache(vdhcp, sta->mac);
	if(tmp != NULL)
	{
		dhcp_mac_hash_del(vdhcp,tmp);
		
		tmp->mhnext = NULL;

		prev = vdhcp->dhcpfree.ip_list;
		
		while (prev && prev->next)
		{
			prev = prev->next;
		}
		if (prev)
		{
			prev->next = tmp;
			tmp->next = NULL;
		}
		else
		{
			vdhcp->dhcpfree.ip_list = tmp;
			tmp->next = NULL;
		}			

		
		vdhcp->dhcpfree.last = tmp;

		dhcp_ip_hash_add(&(vdhcp->dhcpfree),tmp);
		
		vdhcp->dhcpfree.ipnum++;

		asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s: timeout release vir ip  %s for sta "MACSTR"\n",
			__func__, u32ip2str(tmp->ip),MAC2STR(sta->mac));	

		memset(tmp->mac, 0, MAC_LEN);
	}
	else
	{
		asd_printf(ASD_DEFAULT,MSG_DEBUG,"%s: sta "MACSTR" not in vir cache\n", 
			__func__, MAC2STR(sta->mac));	
	}
}
	

