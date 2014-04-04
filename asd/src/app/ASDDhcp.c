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

#define IP_HASH(ip) (ip[0]+ip[1]+ip[2]+ip[3])
#define MAC_HASH(mac) (mac[2]+mac[3]+mac[4]+mac[5])

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

struct ip_info *dhcp_add_ip
(
	struct vir_pool *pool,
	int ip
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



struct ip_info *dhcp_assign_ip
(
	struct vir_dhcp *vdhcp
)
{
	struct ip_info *tmp = NULL;

	if (vdhcp->dhcpfree.ip_list != NULL)
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
		return NULL;
	}
}

struct ip_info *dhcp_release_ip(struct vir_dhcp * vdhcp, int ip)
{
	struct ip_info *tmp = NULL, *prev = NULL;
	
	tmp = dhcp_get_sta_by_ip(&(vdhcp->dhcplease), ip);
	if(tmp != NULL){
		/* Note: first delete node in hash list, second delete node in ip_list*/
		dhcp_ip_hash_del(&(vdhcp->dhcplease), tmp);
		tmp->hnext = NULL;
		dhcp_ip_list_del(&(vdhcp->dhcplease), tmp);
		tmp->next = NULL;
		vdhcp->dhcplease.ipnum--;

		if (vdhcp->dhcpfree.last && (vdhcp->dhcpfree.last != tmp))
		{
			vdhcp->dhcpfree.last->next = tmp;
			tmp->next = NULL;
		}
		else
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


