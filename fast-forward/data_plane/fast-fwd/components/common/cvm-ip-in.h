/***********************************************************************

  OCTEON TOOLKITS                                                         
  Copyright (c) 2007 Cavium Networks. All rights reserved.

  This file, which is part of the OCTEON TOOLKIT from Cavium Networks,
  contains proprietary and confidential information of Cavium Networks
  and in some cases its suppliers.

  Any licensed reproduction, distribution, modification, or other use of
  this file or confidential information embodied in this file is subject
  to your license agreement with Cavium Networks. The applicable license
  terms can be found by contacting Cavium Networks or the appropriate
  representative within your company.

  All other use and disclosure is prohibited.

  Contact Cavium Networks at info@caviumnetworks.com for more information.

 ************************************************************************/ 


#ifndef __CVM_IP_IN_H__
#define __CVM_IP_IN_H__

#if defined(__KERNEL__) && defined(linux)
/* Nothing*/
#else
#include <stdint.h>
#endif

//#include "cvmx.h"
//#include "cvmx-fpa.h"
//#include "cvm-enet.h"

/* Standard well-known ports.  */
enum
  {
    CMV_IP_IPPORT_ECHO = 7,            /* Echo service.  */
    CMV_IP_IPPORT_DISCARD = 9,         /* Discard transmissions service.  */
    CMV_IP_IPPORT_SYSTAT = 11,         /* System status service.  */
    CMV_IP_IPPORT_DAYTIME = 13,        /* Time of day service.  */
    CMV_IP_IPPORT_NETSTAT = 15,        /* Network status service.  */
    CMV_IP_IPPORT_FTP = 21,            /* File Transfer Protocol.  */
    CMV_IP_IPPORT_TELNET = 23,         /* Telnet protocol.  */
    CMV_IP_IPPORT_SMTP = 25,           /* Simple Mail Transfer Protocol.  */
    CMV_IP_IPPORT_TIMESERVER = 37,     /* Timeserver service.  */
    CMV_IP_IPPORT_NAMESERVER = 42,     /* Domain Name Service.  */
    CMV_IP_IPPORT_WHOIS = 43,          /* Internet Whois service.  */
    CMV_IP_IPPORT_MTP = 57,
                                                                                                              
    CMV_IP_IPPORT_TFTP = 69,           /* Trivial File Transfer Protocol.  */
    CMV_IP_IPPORT_RJE = 77,
    CMV_IP_IPPORT_FINGER = 79,         /* Finger service.  */
    CMV_IP_IPPORT_TTYLINK = 87,
    CMV_IP_IPPORT_SUPDUP = 95,         /* SUPDUP protocol.  */
                                                                                                              
                                                                                                              
    CMV_IP_IPPORT_EXECSERVER = 512,    /* execd service.  */
    CMV_IP_IPPORT_LOGINSERVER = 513,   /* rlogind service.  */
    CMV_IP_IPPORT_CMDSERVER = 514,
    CMV_IP_IPPORT_EFSSERVER = 520,
                                                                                                              
    /* UDP ports.  */
    CMV_IP_IPPORT_BIFFUDP = 512,
    CMV_IP_IPPORT_WHOSERVER = 513,
    CMV_IP_IPPORT_ROUTESERVER = 520,
                                                                                                              
    /* Ports less than this value are reserved for privileged processes.  */
    CMV_IP_IPPORT_RESERVED = 1024,
                                                                                                              
    /* Ports greater this value are reserved for (non-privileged) servers.  */
    CMV_IP_IPPORT_USERRESERVED = 5000
  };

/*
 * Ports < IPPORT_RESERVED are reserved for
 * privileged processes (e.g. root).         (IP_PORTRANGE_LOW)
 */
#define CVM_IP_IPPORT_RESERVED         1024
                                                                                                              
/*
 * Default local port range, used by both IP_PORTRANGE_DEFAULT
 * and IP_PORTRANGE_HIGH.
 */
#define CVM_IP_IPPORT_HIFIRSTAUTO      49152
#define CVM_IP_IPPORT_HILASTAUTO       65535
                                                                                                              
/*
 * Scanning for a free reserved port return a value below IPPORT_RESERVED,
 * but higher than IPPORT_RESERVEDSTART.  Traditionally the start value was
 * 512, but that conflicts with some well-known-services that firewalls may
 * have a fit if we use.
 */
#define CVM_IP_IPPORT_RESERVEDSTART    600
                                                                                                              
#define CVM_IP_IPPROTO_HOPOPTS          0       /* IP6 hop-by-hop options */
#define CVM_IP_IPPROTO_ROUTING          43      /* IP6 routing header */
#define CVM_IP_IPPROTO_FRAGMENT         44      /* IP6 fragmentation header */
#define CVM_IP_IPPROTO_ESP              50      /* IP6 Encap Sec. Payload */
#define CVM_IP_IPPROTO_AH               51      /* IP6 Auth Header */
#define CVM_IP_IPPROTO_ICMPV6           58      /* ICMP6 */
#define CVM_IP_IPPROTO_DSTOPTS          60      /* IP6 destination option */

/* last return value of *_input(), meaning "all job for this pkt is done".  */
#define CVM_IP_IPPROTO_DONE             257

#define CVM_IP_IPPORT_MAX              65535


typedef uint16_t cvm_ip_in_port_t;
typedef uint32_t cvm_ip_in_addr_t;

#define	CVM_IP_INADDR_ANY	(uint32_t)0x00000000
#define	CVM_IP_INADDR_BROADCAST	(uint32_t)0xffffffff	/* must be masked */

/*
 * Definitions of bits in internet address integers.
 * On subnets, the decomposition of addresses to host and net parts
 * is done according to subnet mask, not the masks here.
 */
#define	CVM_IP_IN_CLASSA(i)		(((uint32_t)(i) & 0x80000000) == 0)
#define	CVM_IP_IN_CLASSA_NET		0xff000000
#define	CVM_IP_IN_CLASSA_NSHIFT	24
#define	CVM_IP_IN_CLASSA_HOST		0x00ffffff
#define	CVM_IP_IN_CLASSA_MAX		128

#define	CVM_IP_IN_CLASSB(i)		(((uint32_t)(i) & 0xc0000000) == 0x80000000)
#define	CVM_IP_IN_CLASSB_NET		0xffff0000
#define	CVM_IP_IN_CLASSB_NSHIFT	16
#define	CVM_IP_IN_CLASSB_HOST		0x0000ffff
#define	CVM_IP_IN_CLASSB_MAX		65536

#define	CVM_IP_IN_CLASSC(i)		(((uint32_t)(i) & 0xe0000000) == 0xc0000000)
#define	CVM_IP_IN_CLASSC_NET		0xffffff00
#define	CVM_IP_IN_CLASSC_NSHIFT	8
#define	CVM_IP_IN_CLASSC_HOST		0x000000ff

#define	CVM_IP_IN_CLASSD(i)		(((uint32_t)(i) & 0xf0000000) == 0xe0000000)
#define	CVM_IP_IN_CLASSD_NET		0xf0000000	/* These ones aren't really */
#define	CVM_IP_IN_CLASSD_NSHIFT	28		/* net and host fields, but */
#define	CVM_IP_IN_CLASSD_HOST		0x0fffffff	/* routing needn't know.    */
#define	CVM_IP_IN_MULTICAST(i)		CVM_IP_IN_CLASSD(i)

#define	CVM_IP_IN_EXPERIMENTAL(i)	(((uint32_t)(i) & 0xf0000000) == 0xf0000000)
#define	CVM_IP_IN_BADCLASS(i)		(((uint32_t)(i) & 0xf0000000) == 0xf0000000)
#define	CVM_IP_IN_LOOPBACK(i)		(((uint32_t)(i) & 0xff000000) == 0x7f000000)
#define	CVM_IP_IN_ZERONET(i)		(((uint32_t)(i) & 0xff000000) == 0x00000000)
#define	CVM_IP_IN_LOCAL_MCAST(i)		(((uint32_t)(i) & 0xffffff00) == 0xe0000000)

#define	CVM_IP_INADDR_LOOPBACK		(uint32_t)0x7f000001

#define	CVM_IP_INADDR_UNSPEC_GROUP	(u_int32_t)0xe0000000	/* 224.0.0.0 */
#define	CVM_IP_INADDR_ALLHOSTS_GROUP	(u_int32_t)0xe0000001	/* 224.0.0.1 */
#define	CVM_IP_INADDR_ALLRTRS_GROUP	(u_int32_t)0xe0000002	/* 224.0.0.2 */
#define	CVM_IP_INADDR_ALLMDNS_GROUP	(u_int32_t)0xe00000fb	/* 224.0.0.251 */
#define	CVM_IP_INADDR_MAX_LOCAL_GROUP	(u_int32_t)0xe00000ff	/* 224.0.0.255 */

#define	CVM_IP_IN_LOOPBACKNET		127			/* official! */

#define CVM_IP_IPPROTO_MAX 256

/* Internet address (a structure for historical reasons). */
#ifndef _CVM_IP_STRUCT_IN_ADDR_DECLARED
struct cvm_ip_in_addr {
    cvm_ip_in_addr_t s_addr;
};
#define _CVM_IP_STRUCT_IN_ADDR_DECLARED
#endif

/* Socket address, internet style. */
struct cvm_ip_sockaddr_in {
    uint8_t			sin_len;
    uint8_t			sin_family;
    cvm_ip_in_port_t		sin_port;
    struct cvm_ip_in_addr	sin_addr;
    char			sin_zero[8];
};

#define cvm_ip_in_nullhost(x)	((x).s_addr == CVM_IP_INADDR_ANY)
#define cvm_ip_satosin(sa)	((struct cvm_ip_sockaddr_in *)(sa))
#define cvm_ip_ifatoia(ifa)	((struct cvm_ip_in_ifaddr *)(ifa))

/**
 * Function to calculate IPv4 header checksum.
 * 
 * @param *ip pointer to the beginning of IP header
 * 
 * @return 16-bit one's complement IPv4 checksum.
 * No alignment requirements.
 */

static inline uint16_t cvm_ip_calculate_ip_header_checksum(cvm_common_ip_hdr_t *ip)
{
   uint64_t sum;
   uint16_all_alias *ptr = (uint16_all_alias*) ip;
   uint8_all_alias *bptr = (uint8_all_alias*) ip;

   sum  = ptr[0];		
   sum += ptr[1];
   sum += ptr[2];
   sum += ptr[3];
   sum += ptr[4];
   // Skip checksum field
   sum += ptr[6];
   sum += ptr[7];
   sum += ptr[8];
   sum += ptr[9];

   // Check for options
   if (cvmx_unlikely(bptr[0] != 0x45)) goto slow_cksum_calc;

return_from_slow_cksum_calc:

   sum = (uint16_t) sum + (sum >> 16);
   sum = (uint16_t) sum + (sum >> 16);
   return ((uint16_t) (sum ^ 0xffff));

slow_cksum_calc:
   // Adds IPv4 options into the checksum (if present)
   {
      uint64_t len = (bptr[0] & 0xf) - 5;
      ptr = &ptr[10];

      while (len-- > 0) {
	    sum += *ptr++;
	    sum += *ptr++;
      }
   }

   goto return_from_slow_cksum_calc;
}

#endif /* __CVM_IP_IN_H__ */
