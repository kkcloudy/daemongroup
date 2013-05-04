/* Copyright (C) 2000-2002 Joakim Axelsson <gozem@linux.nu>
 *                         Patrick Schaaf <bof@bof.de>
 * Copyright (C) 2003-2004 Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/* Kernel module implementing an IP set type: the single bitmap type */

#include <linux/module.h>
#include <linux/ip.h>
#include <linux/skbuff.h>
#include <linux/version.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ip_set.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <asm/bitops.h>
#include <linux/spinlock.h>

#include <linux/netfilter_ipv4/ip_set_ipmap.h>

static inline ip_set_ip_t
ip_to_id(const struct ip_set_ipmap *map, ip_set_ip_t ip)
{
	return (ip - map->first_ip)/map->hosts;
}

static inline int
__testip(struct ip_set *set, ip_set_ip_t ip, ip_set_ip_t *hash_ip)
{
	struct ip_set_ipmap *map = set->data;
	
	if (ip < map->first_ip || ip > map->last_ip)
		return -ERANGE;

	*hash_ip = ip & map->netmask;
	DP("set: %s, ip:%u.%u.%u.%u, %u.%u.%u.%u",
	   set->name, HIPQUAD(ip), HIPQUAD(*hash_ip));
	return !!test_bit(ip_to_id(map, *hash_ip), map->members);
}

static int
testip(struct ip_set *set, const void *data, size_t size,
       ip_set_ip_t *hash_ip)
{
	const struct ip_set_req_ipmap *req = data;

	if (size != sizeof(struct ip_set_req_ipmap)) {
		ip_set_printk("data length wrong (want %zu, have %zu)",
			      sizeof(struct ip_set_req_ipmap),
			      size);
		return -EINVAL;
	}
	return __testip(set, req->ip, hash_ip);
}

static int
testip_kernel(struct ip_set *set,
	      const struct sk_buff *skb,
	      ip_set_ip_t *hash_ip,
	      const u_int32_t *flags,
	      unsigned char index)
{
	int res =  __testip(set,
			ntohl(flags[index] & IPSET_SRC
				? ip_hdr(skb)->saddr
				: ip_hdr(skb)->daddr),
			hash_ip);
	return (res < 0 ? 0 : res);
}

/*
// WE USE 64-bits kernel
// ALTHOUGH WE USE IP_SET_TEST_AND_SET_BIT here, use test_and_set_bit in kernel SHOULD be correct too.
#if (_MIPS_SZLONG == 32)
#define SZLONG_LOG 5
#define SZLONG_MASK 31UL
#define __LL		"ll	"
#define __SC		"sc	"
#define cpu_to_lelongp(x) cpu_to_le32p((__u32 *) (x))
#elif (_MIPS_SZLONG == 64)
#define SZLONG_LOG 6
#define SZLONG_MASK 63UL
#define __LL		"lld	"
#define __SC		"scd	"
#define cpu_to_lelongp(x) cpu_to_le64p((__u64 *) (x))
#endif
*/
/*
 * test_and_set_bit - Set a bit and return its old value
 * @nr: Bit to set
 * @addr: Address to count from
 *
 * This operation is atomic and cannot be reordered.
 * It also implies a memory barrier.
 */
static inline int ip_set_test_and_set_bit(unsigned long nr,
	volatile unsigned long *addr)
{
	if (cpu_has_llsc && R10000_LLSC_WAR) {
		unsigned long *m = ((unsigned long *) addr) + (nr >> SZLONG_LOG);
		unsigned long temp, res;
		
		DP("test_and_set_bit:1:m %u\n",m);
			
		__asm__ __volatile__(
		"	.set	mips3					\n"
		"1:	" __LL "%0, %1		# test_and_set_bit	\n"
		"	or	%2, %0, %3				\n"
		"	" __SC	"%2, %1					\n"
		"	beqzl	%2, 1b					\n"
		"	and	%2, %0, %3				\n"
#ifdef CONFIG_SMP
		"	sync						\n"
#endif
		"	.set	mips0					\n"
		: "=&r" (temp), "=m" (*m), "=&r" (res)
		: "r" (1UL << (nr & SZLONG_MASK)), "m" (*m)
		: "memory");

		return res != 0;
	} else if (cpu_has_llsc) {
		unsigned long *m = ((unsigned long *) addr) + (nr >> SZLONG_LOG);
		unsigned long temp, res;
		DP("test_and_set_bit:2:addr %u nr %u SZLONG_LOG %u SZLONG_MASK %u\n",addr,nr,SZLONG_LOG,SZLONG_MASK);

		__asm__ __volatile__(
		"	.set	push					\n"
		"	.set	noreorder				\n"
#ifdef CONFIG_CPU_CAVIUM_OCTEON
		OCTEON_SYNCW_STR
#endif
		"	.set	mips3					\n"
		"1:	" __LL "%0, %1		# test_and_set_bit	\n"
		"	or	%2, %0, %3				\n"
		"	" __SC	"%2, %1					\n"
		"	beqz	%2, 1b					\n"
		"	 and	%2, %0, %3				\n"
#if defined(CONFIG_SMP) && !defined(CONFIG_CPU_CAVIUM_OCTEON)
		"	sync						\n"
#endif
		"	.set	pop					\n"
		: "=&r" (temp), "=m" (*m), "=&r" (res)
		: "r" (1UL << (nr & SZLONG_MASK)), "m" (*m)
		: "memory");

		return res != 0;
	} else {
		volatile unsigned long *a = addr;
		unsigned long mask;
		int retval;
	//	__bi_flags;


		a += nr >> SZLONG_LOG;
		DP("test_and_set_bit:3:a %u\n",a);

		mask = 1UL << (nr & SZLONG_MASK);
	//	__bi_local_irq_save(flags);
		retval = (mask & *a) != 0;
		*a |= mask;
//		__bi_local_irq_restore(flags);

		return retval;
	}
}


static inline int
__addip(struct ip_set *set, ip_set_ip_t ip, ip_set_ip_t *hash_ip)
{
	struct ip_set_ipmap *map = set->data;

	int bytes = bitmap_bytes(0, map->sizeid - 1);

#if defined(IP_SET_DEBUG)

	char tmpstr[256];
	int i;
	memset(tmpstr,0,256);
	for (i=0; i < ((bytes < 256)?bytes:256); i++)
		{
			sprintf((char *)&tmpstr[i*2],"%02x", ((char *)map->members)[i]);
		}
	DP("Before Set Members Data: [%s]",tmpstr);
#endif

	if (ip < map->first_ip || ip > map->last_ip)
		return -ERANGE;

	*hash_ip = ip & map->netmask;
	DP("%u.%u.%u.%u, %u.%u.%u.%u, id %u", HIPQUAD(ip), HIPQUAD(*hash_ip),ip_to_id(map, *hash_ip));
	if (ip_set_test_and_set_bit(ip_to_id(map, *hash_ip), map->members))
		return -EEXIST;

#if defined(IP_SET_DEBUG)

	memset(tmpstr,0,256);
	for (i=0; i < ((bytes < 256)?bytes:256); i++)
		{
			sprintf((char *)&tmpstr[i*2],"%02x", ((char *)map->members)[i]);
		}
	DP("After Set Members Data: [%s]",tmpstr);
#endif

	return 0;
}

static int
addip(struct ip_set *set, const void *data, size_t size,
      ip_set_ip_t *hash_ip)
{
	const struct ip_set_req_ipmap *req = data;

	if (size != sizeof(struct ip_set_req_ipmap)) {
		ip_set_printk("data length wrong (want %zu, have %zu)",
			      sizeof(struct ip_set_req_ipmap),
			      size);
		return -EINVAL;
	}
	DP("%u.%u.%u.%u", HIPQUAD(req->ip));
	return __addip(set, req->ip, hash_ip);
}

static int
addip_kernel(struct ip_set *set,
	     const struct sk_buff *skb,
	     ip_set_ip_t *hash_ip,
	     const u_int32_t *flags,
	     unsigned char index)
{
	return __addip(set,
		       ntohl(flags[index] & IPSET_SRC
				? ip_hdr(skb)->saddr
				: ip_hdr(skb)->daddr),
		       hash_ip);
}

static inline int
__delip(struct ip_set *set, ip_set_ip_t ip, ip_set_ip_t *hash_ip)
{
	struct ip_set_ipmap *map = set->data;

	if (ip < map->first_ip || ip > map->last_ip)
		return -ERANGE;

	*hash_ip = ip & map->netmask;
	DP("%u.%u.%u.%u, %u.%u.%u.%u", HIPQUAD(ip), HIPQUAD(*hash_ip));
	if (!test_and_clear_bit(ip_to_id(map, *hash_ip), map->members))
		return -EEXIST;
	
	return 0;
}

static int
delip(struct ip_set *set, const void *data, size_t size,
      ip_set_ip_t *hash_ip)
{
	const struct ip_set_req_ipmap *req = data;

	if (size != sizeof(struct ip_set_req_ipmap)) {
		ip_set_printk("data length wrong (want %zu, have %zu)",
			      sizeof(struct ip_set_req_ipmap),
			      size);
		return -EINVAL;
	}
	return __delip(set, req->ip, hash_ip);
}

static int
delip_kernel(struct ip_set *set,
	     const struct sk_buff *skb,
	     ip_set_ip_t *hash_ip,
	     const u_int32_t *flags,
	     unsigned char index)
{
	return __delip(set,
		       ntohl(flags[index] & IPSET_SRC
				? ip_hdr(skb)->saddr
				: ip_hdr(skb)->daddr),
		       hash_ip);
}

static int create(struct ip_set *set, const void *data, size_t size)
{
	int newbytes;
	const struct ip_set_req_ipmap_create *req = data;
	struct ip_set_ipmap *map;

	if (size != sizeof(struct ip_set_req_ipmap_create)) {
		ip_set_printk("data length wrong (want %zu, have %zu)",
			      sizeof(struct ip_set_req_ipmap_create),
			      size);
		return -EINVAL;
	}

	DP("from %u.%u.%u.%u to %u.%u.%u.%u",
	   HIPQUAD(req->from), HIPQUAD(req->to));

	if (req->from > req->to) {
		DP("bad ip range");
		return -ENOEXEC;
	}

	map = kmalloc(sizeof(struct ip_set_ipmap), GFP_KERNEL);
	if (!map) {
		DP("out of memory for %d bytes",
		   sizeof(struct ip_set_ipmap));
		return -ENOMEM;
	}
	map->first_ip = req->from;
	map->last_ip = req->to;
	map->netmask = req->netmask;

	if (req->netmask == 0xFFFFFFFF) {
		map->hosts = 1;
		map->sizeid = map->last_ip - map->first_ip + 1;
	} else {
		unsigned int mask_bits, netmask_bits;
		ip_set_ip_t mask;
		
		map->first_ip &= map->netmask;	/* Should we better bark? */
		
		mask = range_to_mask(map->first_ip, map->last_ip, &mask_bits);
		netmask_bits = mask_to_bits(map->netmask);
		
		if ((!mask && (map->first_ip || map->last_ip != 0xFFFFFFFF))
		    || netmask_bits <= mask_bits)
			return -ENOEXEC;

		DP("mask_bits %u, netmask_bits %u",
		   mask_bits, netmask_bits);
		map->hosts = 2 << (32 - netmask_bits - 1);
		map->sizeid = 2 << (netmask_bits - mask_bits - 1);
	}
	if (map->sizeid > MAX_RANGE + 1) {
		ip_set_printk("range too big (max %d addresses)",
			       MAX_RANGE+1);
		kfree(map);
		return -ENOEXEC;
	}
	DP("hosts %u, sizeid %u", map->hosts, map->sizeid);
	newbytes = bitmap_bytes(0, map->sizeid - 1);
	map->members = kmalloc(newbytes, GFP_KERNEL);
	if (!map->members) {
		DP("out of memory for %d bytes", newbytes);
		kfree(map);
		return -ENOMEM;
	}
	memset(map->members, 0, newbytes);
	
	set->data = map;
	return 0;
}

static void destroy(struct ip_set *set)
{
	struct ip_set_ipmap *map = set->data;
	
	kfree(map->members);
	kfree(map);
	
	set->data = NULL;
}

static void flush(struct ip_set *set)
{
	struct ip_set_ipmap *map = set->data;
	memset(map->members, 0, bitmap_bytes(0, map->sizeid - 1));
}

static void list_header(const struct ip_set *set, void *data)
{
	const struct ip_set_ipmap *map = set->data;
	struct ip_set_req_ipmap_create *header = data;

	header->from = map->first_ip;
	header->to = map->last_ip;
	header->netmask = map->netmask;
}

static int list_members_size(const struct ip_set *set)
{
	const struct ip_set_ipmap *map = set->data;

	return bitmap_bytes(0, map->sizeid - 1);
}
#define OPT_NUMERIC		0x0001U		/* -n */

static void list_members(const struct ip_set *set, void *data)
{
	const struct ip_set_ipmap *map = set->data;
	int bytes = bitmap_bytes(0, map->sizeid - 1);
#if defined(IP_SET_DEBUG)
	char tmpstr[256];
	int i;
#endif	
	memcpy(data, map->members, bytes);

#if defined(IP_SET_DEBUG)
	memset(tmpstr,0,256);
	for (i=0; i < ((bytes < 256)?bytes:256); i++)
		{
			sprintf((char *)&tmpstr[i*2],"%02x", ((char *)data)[i]);
		}
	DP("Members Data: [%s]",tmpstr);
#endif
}

static struct ip_set_type ip_set_ipmap = {
	.typename		= SETTYPE_NAME,
	.features		= IPSET_TYPE_IP | IPSET_DATA_SINGLE,
	.protocol_version	= IP_SET_PROTOCOL_VERSION,
	.create			= &create,
	.destroy		= &destroy,
	.flush			= &flush,
	.reqsize		= sizeof(struct ip_set_req_ipmap),
	.addip			= &addip,
	.addip_kernel		= &addip_kernel,
	.delip			= &delip,
	.delip_kernel		= &delip_kernel,
	.testip			= &testip,
	.testip_kernel		= &testip_kernel,
	.header_size		= sizeof(struct ip_set_req_ipmap_create),
	.list_header		= &list_header,
	.list_members_size	= &list_members_size,
	.list_members		= &list_members,
	.me			= THIS_MODULE,
};

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>");
MODULE_DESCRIPTION("ipmap type of IP sets");

static int __init ip_set_ipmap_init(void)
{
	return ip_set_register_set_type(&ip_set_ipmap);
}

static void __exit ip_set_ipmap_fini(void)
{
	/* FIXME: possible race with ip_set_create() */
	ip_set_unregister_set_type(&ip_set_ipmap);
}

module_init(ip_set_ipmap_init);
module_exit(ip_set_ipmap_fini);
