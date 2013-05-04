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

#ifndef __FASTFWD_COMMON_DEFS_H__
#define __FASTFWD_COMMON_DEFS_H__

#include "fwd_debug.h"
#define RETURN_OK          0
#define RETURN_ERROR    -1

#define FUNC_ENABLE       1
#define FUNC_DISABLE     0

#ifdef INET6
#undef CVM_IP_DYNAMIC_SHORT_SUPPORT
#endif

#ifdef CVM_IP_DYNAMIC_SHORT_SUPPORT
#define CVM_COMMON_PD_ALIGN (4+16) /* Hardware aligns IP header in packet_data */
#define CVM_COMMON_IP6_PD_ALIGN 16 /* Hardware aligns IP header in packet_data */
#else
#define CVM_COMMON_PD_ALIGN 4 /* Hardware aligns IP header in packet_data */
#define CVM_COMMON_IP6_PD_ALIGN 0 /* Hardware aligns IP header in packet_data */
#endif

#define CVM_COMMON_BIG_ENDIAN 4321
#define CVM_COMMON_LITTLE_ENDIAN 1234
#define CVM_COMMON_BYTE_ORDER CVM_COMMON_BIG_ENDIAN

#define CVM_COMMON_WQE_INVALID 0xff
#define CVM_COMMON_WQE_TIMER_INTERRUPT 1
#define CVM_COMMON_WQE_LONG_TIMER_INTERRUPT 2

#define CVM_COMMON_MINUS_ONE_64 (0xffffffffffffffffULL)
#define CVM_COMMON_ALLOC_INIT   (0xBADDEED0BADDEEDDULL)

/* Common Arena Allocation Parameters */
#define CVM_COMMON_ARENA_SIZE    (8 * 1024 * 1024)
#define CVM_COMMON_ARENA_COUNT   (4)

#define CVM_COMMON_TICK_LEN_US (2000)      /* tick length in microsec */
#define CVM_COMMON_WHEEL_LEN_TICKS (2500)  /* wheel length in ticks */

#define CVM_COMMON_IDLE_PROCESSING_INTERVAL  5  /* secs */

#define CVM_ETH_P_8021Q	0x8100
#define CVM_ETH_P_IP	0x0800	
#define VLAN_TAG_LEN    4


#if defined(__KERNEL__) && defined(linux)
#define cvm_common_panic(...) \
    { \
       printk("PANIC:" __VA_ARGS__); \
       panic("%s:%d\n", __FILE__, __LINE__); \
    }
#else
extern void exit(int);

#define cvm_common_panic(...) \
    { \
       printf("PANIC:" __VA_ARGS__); \
       exit(-1); \
    }
#endif

#define CVM_COMMON_KASSERT(exp,msg) \
    { \
    if(!(exp)) cvm_common_panic msg; \
    }
	

#define cvm_common_ntohs(a) (a)
#define cvm_common_htons(a) (a)
#define cvm_common_ntohl(a) (a)
#define cvm_common_htonl(a) (a)

/* Macros for counting and rounding. */
#define cvm_common_howmany(x, y)   (((x)+((y)-1))/(y))
#define cvm_common_rounddown(x, y) (((x)/(y))*(y))
#define cvm_common_roundup(x, y)   ((((x)+((y)-1))/(y))*(y))  /* to any y */
#define cvm_common_roundup2(x, y)  (((x)+((y)-1))&(~((y)-1))) /* if y is powers of two */
#define cvm_common_powerof2(x)     ((((x)-1)&(x))==0)

#define CVM_COMMON_MALLOC_BACK_VAL 0xF

static inline int cvm_common_imax(int a, int b) { return (a > b ? a : b); }
static inline int cvm_common_imin(int a, int b) { return (a < b ? a : b); }

static inline int cvm_common_splnet(void) { return 1; }
static inline int cvm_common_splimp(void) { return 1; }
static inline void cvm_common_splx(int x) { return; }


/* debug macros & associated enums */

/* local typedef for long long */
/* used with SIMPRINTF macro   */
typedef long long    ll64_t;


#ifdef OCTEON_DEBUG_LEVEL

/* 1 : lowest level with minimum debugging information  */
/* n : highest level with maximim debugging information */
typedef enum { 
        FASTFWD_COMMON_DBG_LVL_MUST_PRINT = 1,
        FASTFWD_COMMON_DBG_LVL_WARNING = 2,
        FASTFWD_COMMON_DBG_LVL_ERROR = 3,
        FASTFWD_COMMON_DBG_LVL_DEBUG = 4,
        FASTFWD_COMMON_DBG_LVL_INFO = 5,
        FASTFWD_COMMON_MAX_DBG_LVL 
} fastfwd_common_debug_level_t;

typedef enum { 	
        FASTFWD_COMMON_MOUDLE_MAIN = 1,
        FASTFWD_COMMON_MOUDLE_SHELL = 2,
        FASTFWD_COMMON_MOUDLE_FLOWTABLE = 3,
        FASTFWD_COMMON_MAX_TYPE 
} fastfwd_module_type_t;

extern CVMX_SHARED  int fastfwd_common_debug_level;
extern CVMX_SHARED  uint8_t module_print[FASTFWD_COMMON_MAX_TYPE];
extern CVMX_SHARED uint64_t core_mask;
extern void cvm_common_get_debug_level(void);
extern void cvm_common_set_debug_level(int64_t dbg_lvl);

/** Common Debug Message Print macro
 *
 *  @param lvl      - debug level for the message
 *  @param format   - message print format
 *  @param args     - print arguments
 *
 * This macro is used for debug message printng. The
 * lvl parameter specifies the debug level for this 
 * message. The OCTEON_DEBUG_LEVEL specifies system
 * wide debug level. The message is only printed if
 * the lvl paramter is less than or equal to the
 * OCTEON_DEBUG_LEVEL.
 *
 * Note that the debug message printing is enabled
 * only when the OCTEON_DEBUG_LEVEL is defined and
 * assigned a number from 0-9. If OCTEON_DEBUG_LEVEL
 * is not defined, all the calls to this macro are
 * ignored. 
 *
 */


#define FASTFWD_COMMON_DBG_MSG(module,lvl, format, args...) \
{ \
    if ((module_print[module] > 0) && ((1 << cvmx_get_core_num()) & (core_mask))) \
    { \
    	if (lvl <= fastfwd_common_debug_level) \
    	{ \
	        if(fwd_debug_log_enable == FUNC_ENABLE) \
	        { \
	            fwd_debug_printf("Core %d:  [DBG %d] [TIM %lu]", cvmx_get_core_num(),lvl, cvmx_clock_get_count(CVMX_CLOCK_SCLK)); \
	            fwd_debug_printf(format, ##args); \
	        } \
	        else \
	        { \
	            printf("Core %d:  [DBG %d] ", cvmx_get_core_num(),lvl); \
	            printf(format, ##args); \
	        } \
    	} \
        if (lvl <= FASTFWD_COMMON_DBG_LVL_WARNING) \
        { \
			fwd_debug_agent_printf(format, ##args); \
		} \
    } \
}

/* fast-fwd group add for logsave */


/** Common Debug Memory Dump macro
 *
 *  @param lvl      - debug level
 *  @param buf      - char pointer to the buffer
 *  @param len      - number of bytes to dump
 *
 * This macro is used to dump memory. The
 * lvl parameter specifies the debug level for this 
 * dump. The OCTEON_DEBUG_LEVEL specifies system
 * wide debug level. The message is only printed if
 * the lvl paramter is less than or equal to the
 * OCTEON_DEBUG_LEVEL.
 *
 * Note that the debug memory dump macro is enabled
 * only when the OCTEON_DEBUG_LEVEL is defined and
 * assigned a number from 0-9. If OCTEON_DEBUG_LEVEL
 * is not defined, all the calls to this macro are
 * ignored. 
 *
 */
#define FASTFWD_COMMON_DBG_DUMP(module,lvl, buf, len) \
{ \
	if ((lvl <= fastfwd_common_debug_level) && (lvl < FASTFWD_COMMON_MAX_DBG_LVL) && (module_print[module] > 0)&& ((1 << cvmx_get_core_num()) & core_mask)) \
	{ \
		int a=0; \
		unsigned char tmp=0; \
		printf("[DBG %d]\n", lvl); \
		printf("0x%04X : ", a); \
		for (a=0;a<(len);a++) \
		{ \
		if(a && ((a%8) == 0)) \
			{ \
		printf("%s", "\n"); \
				printf("0x%04X : ", a); \
			} \
		tmp = (buf)[a];\
		tmp &= 0xff;\
		printf("%02x ",tmp);\
		} \
	printf("\n\n"); \
	} \
}





extern void fastfwd_common_set_debug_level(uint64_t dbg_lvl);

#else
#define FASTFWD_COMMON_DBG_MSG(module,lvl,format, args...);
#define FASTFWD_COMMON_DBG_DUMP(module,lvl, buf, len);
#endif

#if defined(__KERNEL__) && defined(linux)
#define CVM_COMMON_MALLOC_TSAFE(arena_list,size) \
({ \
    void *ptr= NULL; \
    ptr = kmalloc(size, GFP_DMA); \
    ptr; \
})
  
  
#define CVM_COMMON_FREE_TSAFE(ptr) \
{ \
    kfree(ptr); \
}

#elif defined(linux)

#define CVM_COMMON_MALLOC_TSAFE(arena_list,size) \
({ \
    void *ptr= NULL; \
    ptr = malloc(size); \
    ptr; \
})
  
#define CVM_COMMON_FREE_TSAFE(ptr) \
{ \
    free(ptr); \
}

#else
/*
 * threadsafe functionality of cvmx_malloc
 */
extern CVMX_SHARED cvmx_spinlock_t cvm_common_malloc_lock;

#define CVM_COMMON_MALLOC_TSAFE(arena_list, size) \
({ \
    void *ptr = NULL; \
    cvmx_spinlock_lock(&cvm_common_malloc_lock); \
    ptr = cvmx_malloc(arena_list, size); \
    cvmx_spinlock_unlock(&cvm_common_malloc_lock); \
    ptr; \
})

#define CVM_COMMON_FREE_TSAFE(ptr) \
{ \
    cvmx_spinlock_lock(&cvm_common_malloc_lock); \
    cvmx_free(ptr); \
    cvmx_spinlock_unlock(&cvm_common_malloc_lock); \
}

#define CVM_COMMON_REALLOC_TSAFE(arena_list, oldptr, size)	\
({ \
    void *ptr = NULL; \
    cvmx_spinlock_lock(&cvm_common_malloc_lock); \
    ptr = cvmx_realloc(arena_list, oldptr, size);	\
    cvmx_spinlock_unlock(&cvm_common_malloc_lock); \
    ptr; \
})
#endif

/* common function declarations */
int cvm_common_allocate_arenas(uint64_t arena_size, uint64_t arena_count);
int cvm_common_global_init(void);
int cvm_common_local_init(void);
inline const char *cvm_common_get_version(void);

extern void cvm_common_analyze_fpa(uint32_t pool);
extern void cvm_common_dump_fpa(uint32_t pool);
extern void cvm_common_display_fpa_buffer_count(void);

/* packet dump related misc routines */
void cvm_common_dump_special(cvmx_wqe_t *work);
void cvm_common_dump_packet(cvmx_wqe_t *work, int dir);

#if !defined(__KERNEL__) && !defined(linux)
/* common externs */
extern CVMX_SHARED cvmx_arena_list_t  cvm_common_arenas;
#endif

#if defined(__KERNEL__) && defined(linux)
#define bcopy(s,d,n) memcpy(d,s,n)
#define bzero(s,n) memset(s,0,n)
#define bcmp(s1,s2,n) memcmp(s1,s2,n)
static inline uint64_t ulmin(uint64_t a, uint64_t b)
{
 return (((a)<(b))?(a):(b));
}
#endif

#define cvm_common_bootmem_alloc(size, flags) cvmx_bootmem_alloc(size,flags)

/*
 * Various network packet headers
 */
#define MAC_LEN				6
#define ETH_H_LEN			14
#define ETH_T_LEN			2
#define CW_H_LEN			16
#define IP_H_LEN				20
#define UDP_H_LEN			8
#define VLAN_PROTO_LEN		2
#define VLAN_HLEN	4  

/* ethernet headers*/
typedef struct eth_hdr_s {
    uint8_t  h_dest[6];    /*destination eth addr */
    uint8_t h_source[6];       /* source ether addr */
    uint16_t h_vlan_proto;              /* Should always be 0x8100 */
}eth_hdr;
    
typedef struct vlan_eth_hdr_s {
    uint8_t h_dest[6];
    uint8_t h_source[6];
    uint16_t        h_vlan_proto;
    uint16_t        h_vlan_TCI;
    uint16_t        h_eth_type;
}vlan_eth_hdr_t;

/*
 * Definitions for internet protocol version 4.
 * Per RFC 791, September 1981.
 */
#define	CVM_IP_IPVERSION	4

/*
 * Definitions for  fragment flag.
 */
#define	CVM_IP_IP_RF 0x8000		/**< reserved fragment flag */
#define	CVM_IP_IP_DF 0x4000		/**< dont fragment flag */
#define	CVM_IP_IP_MF 0x2000		/**< more fragments flag */
#define	CVM_IP_IP_OFFMASK 0x1fff	/**< mask for fragmenting bits */

#define DEFAULT_TTL							128

/*
 * Definitions for options.
 */
#define CVM_IP_IPOPT_COPIED(o)         ((o)&0x80)
#define CVM_IP_IPOPT_CLASS(o)          ((o)&0x60)
#define CVM_IP_IPOPT_NUMBER(o)         ((o)&0x1f) 

#define CVM_IP_IPOPT_CONTROL           0x00
#define CVM_IP_IPOPT_RESERVED1         0x20
#define CVM_IP_IPOPT_DEBMEAS           0x40
#define CVM_IP_IPOPT_RESERVED2         0x60

#define CVM_IP_IPOPT_EOL               0               /* end of option list */
#define CVM_IP_IPOPT_NOP               1               /* no operation */

#define CVM_IP_IPOPT_RR                7               /* record packet route */
#define CVM_IP_IPOPT_TS                68              /* timestamp */
#define CVM_IP_IPOPT_SECURITY          130             /* provide s,c,h,tcc */
#define CVM_IP_IPOPT_LSRR              131             /* loose source route */
#define CVM_IP_IPOPT_SATID             136             /* satnet id */
#define CVM_IP_IPOPT_SSRR              137             /* strict source route */

/*
 * Offsets to fields in options other than EOL and NOP.
 */
#define CVM_IP_IPOPT_OPTVAL            0               /* option ID */
#define CVM_IP_IPOPT_OLEN              1               /* option length */
#define CVM_IP_IPOPT_OFFSET            2               /* offset within option */
#define CVM_IP_IPOPT_MINOFF            4               /* min value of above */

/** Definition for the return code to application from IP stack
 *
 */

typedef enum cvm_ip_ret_val{
        CVM_IP_PKT_RCVD,
        CVM_IP_FRAG_RCVD,
        CVM_IP_REASS_PKT_RCVD,
        CVM_IP_PKT_BAD,
} cvm_ip_ret_val_t;

/*
 * Structure of IP header (IPV4)
 */
typedef struct cvm_common_ip_hdr 
{

    uint32_t ip_v:4;		/* version */
    uint32_t ip_hl:4;		/* header length */
    uint32_t ip_tos:8;		/* type of service */
    uint32_t ip_len:16;		/* total length */
    uint32_t ip_id:16;		/* identification */
    uint32_t ip_off:16;		/* fragment offset field */
    uint32_t ip_ttl:8;		/* time to live */
    uint32_t ip_p:8;		/* protocol */
    uint32_t ip_sum:16;		/* checksum */
    uint32_t ip_src;	        /* source address */
    uint32_t ip_dst;	        /* dest address */

} cvm_common_ip_hdr_t;


/*
 * Structure of TCP header.
 */
typedef struct cvm_common_tcp_hdr 
{
    uint16_t  th_sport;		/* source port */
    uint16_t  th_dport;		/* destination port */
    uint32_t  th_seq;		/* sequence number */
    uint32_t  th_ack;		/* acknowledgement number */
    uint32_t  th_off:4,		/* data offset */
              th_x2:4;		/* (unused) */
    uint8_t   th_flags;
    uint16_t  th_win;	        /* window */
    uint16_t  th_sum;	        /* checksum */
    uint16_t  th_urp;		/* urgent pointer */

} cvm_common_tcp_hdr_t;

/*
 * Udp protocol header.
 * Per RFC 768, September, 1981.
 */
typedef struct cvm_common_udp_hdr {
	uint16_t	uh_sport;		/* source port */
	uint16_t	uh_dport;		/* destination port */
	uint16_t	uh_ulen;		/* udp length */
	uint16_t	uh_sum;			/* udp checksum */
}cvm_common_udp_hdr_t;
	
/* various TCP flags */
#define	CVM_COMMON_TCP_TH_FIN	 0x01
#define	CVM_COMMON_TCP_TH_SYN	 0x02
#define	CVM_COMMON_TCP_TH_RST	 0x04
#define	CVM_COMMON_TCP_TH_PUSH	 0x08
#define	CVM_COMMON_TCP_TH_ACK	 0x10
#define	CVM_COMMON_TCP_TH_URG	 0x20
#define	CVM_COMMON_TCP_TH_ECE	 0x40
#define	CVM_COMMON_TCP_TH_CWR	 0x80

/* various protocols */
#define CVM_COMMON_IPPROTO_IP	    0
#define CVM_COMMON_IPPROTO_ICMP     1
#define CVM_COMMON_IPPROTO_TCP	    6
#define CVM_COMMON_IPPROTO_UDP	   17
#define CVM_COMMON_IPPROTO_IPV6 41
#define CVM_COMMON_IPPROTO_RAW	  255

/*
 * The number of bytes in an ethernet (MAC) address.
 */
#define	ETHER_DEFAULT_VLAN_PROTO		0x8100
#define	ETHER_ADDR_LEN		6

typedef struct _cvm_enet_vlan_tag
{
    uint16_t          priority :  3;   /* user priority */
    uint16_t          cfi      :  1;   /* canonical format indicator */
    uint16_t          vlan_id  : 12;   /* vlan id (0-4095) */
} cvm_enet_vlan_tag_t;

/*
 * VLAN-tagged ethernet headers
 */
struct vlan_ethhdr {
	uint8_t  ether_dhost[ETHER_ADDR_LEN];
	uint8_t  ether_shost[ETHER_ADDR_LEN];
	uint16_t out_ether_type;
	cvm_enet_vlan_tag_t out_tag;
	uint16_t in_ether_type;
	cvm_enet_vlan_tag_t in_tag;
	uint16_t ether_type;
};

/** zero copy 64 bit buffer pointer (BP) related macros 
 *                                                     
 * A note about these macros:                         
 *                                                   
 * The input 'p' to all SET_BP* macros could either be
 * a uint64_t value or a pointer. e.g.               
 *                                                  
 *    uint64_t addr = 0;                           
 *    void *ptr = 0;                              
 *                                               
 *    SET_BP_BACK(addr, 0xd);                   
 *    SET_BP_BACK(ptr,  0x3);                  
 *                                            
 * but you shouldn't do a typecast a pointer to a
 * uint64_t within the parameters of the macro i.e.
 *                                                
 *   SET_BP_BACK( (uint64_t)ptr, 0x3);           
 *                                              
 * this will confuse the compiler as it will not be    
 * able to get the address of the ptr                 
 *                                                   
 * This restriction does not apply to GET_BP* macros
 */

#define CVM_COMMON_GET_BP_I_BIT(p) ( ((cvmx_buf_ptr_t)((uint64_t)(p))).s.i )
#define CVM_COMMON_GET_BP_BACK(p)  ( ((cvmx_buf_ptr_t)((uint64_t)(p))).s.back )
#define CVM_COMMON_GET_BP_POOL(p)  ( ((cvmx_buf_ptr_t)((uint64_t)(p))).s.pool )
#define CVM_COMMON_GET_BP_SIZE(p)  ( ((cvmx_buf_ptr_t)((uint64_t)(p))).s.size )
#define CVM_COMMON_GET_BP_ADDR(p)  ( ((cvmx_buf_ptr_t)((uint64_t)(p))).s.addr )

#define CVM_COMMON_SET_BP_I_BIT(p,v) ( (((cvmx_buf_ptr_t*)(&(p))))->s.i=v )
#define CVM_COMMON_SET_BP_BACK(p,v)  ( (((cvmx_buf_ptr_t*)(&(p))))->s.back=v )
#define CVM_COMMON_SET_BP_POOL(p,v)  ( (((cvmx_buf_ptr_t*)(&(p))))->s.pool=v )
#define CVM_COMMON_SET_BP_SIZE(p,v)  ( (((cvmx_buf_ptr_t*)(&(p))))->s.size=v  )
#define CVM_COMMON_SET_BP_ADDR(p,v)  ( (((cvmx_buf_ptr_t*)(&(p))))->s.addr=v )


#define CVM_COMMON_SET_BP(bp, addr, size, pool, back, i) \
{ \
    CVM_COMMON_SET_BP_ADDR(bp, addr); \
    CVM_COMMON_SET_BP_SIZE(bp, size); \
    CVM_COMMON_SET_BP_POOL(bp, pool); \
    CVM_COMMON_SET_BP_BACK(bp, back); \
    CVM_COMMON_SET_BP_I_BIT(bp, i);  \
}

#define CVM_COMMON_MARK_BP_AS_SW_BUFFER(bp) \
{ \
    CVM_COMMON_SET_BP_POOL((bp), CVMX_FPA_WQE_POOL); \
    CVM_COMMON_SET_BP_BACK((bp), CVM_COMMON_MALLOC_BACK_VAL); \
}


#define CVM_COMMON_IS_BP_SW_BUFFER(bp) ( ( ((CVM_COMMON_GET_BP_POOL(bp)) == CVMX_FPA_WQE_POOL) && ( (CVM_COMMON_GET_BP_BACK(bp)) == CVM_COMMON_MALLOC_BACK_VAL)) ? 1 : 0 )



/*
 * Various ptr types
 */

#define CVM_COMMON_DIRECT_DATA  0     /**< Local Pointer points directly to data */
#define CVM_COMMON_GATHER_DATA  1     /**< Local Pointer points to a gather list of local pointers */
#define CVM_COMMON_LINKED_DATA  2     /**< Local pointer points to data which has links to more buffers */
#define CVM_COMMON_NULL_DATA    3     /**< If no data is sent and a flag is required, use this. */

typedef struct cvm_common_pci_sanity_hdr
{
    cvmx_buf_ptr_t      lptr;
    uint32_t            pko_ptr_type;
    uint32_t            segs;
    uint32_t            total_bytes;
    uint32_t            port;

} cvm_common_pci_sanity_hdr_t;


/*
 * CAST macro
 */
#define CVM_COMMON_UCAST64(v) ((long long)(unsigned long)(v))


/*
 * macro to calculate the back field correctly using
 * the current address of an FPA buffer
 */
#define CVM_COMMON_CALC_BACK(p) ( (((p.s.addr - (p.s.addr & ~((cvmx_fpa_get_block_size(p.s.pool))-1))) >> 7) & 0xf) )


/*
 * Pointer <-> Pointer_Offset conversion macros
 */
#define CVM_OFFSET_TO_PTR(ptr_offset, ptr_type, bit_shift) \
        (CASTPTR(ptr_type, cvmx_phys_to_ptr(CAST64(ptr_offset) << bit_shift)))

#define CVM_PTR_TO_OFFSET(ptr, bit_shift) \
        ((uint32_t)(CAST64(cvmx_ptr_to_phys(ptr)) >> bit_shift))


/* stirct aliasing macros */
typedef uint8_t  uint8_all_alias   __attribute__ ((may_alias));
typedef uint16_t uint16_all_alias  __attribute__ ((may_alias));
typedef uint32_t uint32_all_alias  __attribute__ ((may_alias));
typedef uint64_t uint64_all_alias  __attribute__ ((may_alias));


/* wqe defines for sdk2.2. */
#ifdef SDK_VERSION_2_2
#define CVM_WQE_GET_LEN(work)   cvmx_wqe_get_len((work))
#define CVM_WQE_SET_LEN(work, v)   cvmx_wqe_set_len((work), (v))
#define CVM_WQE_GET_QOS(work)   cvmx_wqe_get_qos((work))
#define CVM_WQE_SET_QOS(work, v)   cvmx_wqe_set_qos((work),(v))
#define CVM_WQE_GET_PORT(work)   cvmx_wqe_get_port((work))
#define CVM_WQE_SET_PORT(work, v)   cvmx_wqe_set_port((work), (v))
#define CVM_WQE_GET_GRP(work)   cvmx_wqe_get_grp((work))
#define CVM_WQE_SET_GRP(work, v)   cvmx_wqe_set_grp((work), (v))
#define CVM_WQE_GET_TAG(work)   ((work)->word1.tag)
#define CVM_WQE_SET_TAG(work, v)   ((work)->word1.tag = (v))
#define CVM_WQE_GET_TAG_TYPE(work)   ((work)->word1.tag_type)
#define CVM_WQE_SET_TAG_TYPE(work, v)   ((work)->word1.tag_type = (v))
#define CVM_WQE_GET_UNUSED(work)   cvmx_wqe_get_unused8((work))
#define CVM_WQE_SET_UNUSED(work, v)   cvmx_wqe_set_unused8((work), (v))
#else
#define CVM_WQE_GET_LEN(work)   ((work)->len)
#define CVM_WQE_SET_LEN(work, v)   (work)->len = (v)
#define CVM_WQE_GET_QOS(work)   ((work)->qos)
#define CVM_WQE_SET_QOS(work, v)   ((work)->qos = (v))
#define CVM_WQE_GET_PORT(work)   ((work)->ipprt)
#define CVM_WQE_SET_PORT(work, v)   ((work)->ipprt = (v))
#define CVM_WQE_GET_GRP(work)   ((work)->grp)
#define CVM_WQE_SET_GRP(work, v)   ((work)->grp = (v))
#define CVM_WQE_GET_TAG(work)   ((work)->tag)
#define CVM_WQE_SET_TAG(work, v)   ((work)->tag = (v))
#define CVM_WQE_GET_TAG_TYPE(work)   ((work)->tag_type)
#define CVM_WQE_SET_TAG_TYPE(work, v)   ((work)->tag_type = (v))
#define CVM_WQE_GET_UNUSED(work)   ((work)->unused)
#define CVM_WQE_SET_UNUSED(work, v)   ((work)->unused = (v))
#endif



#endif /* __CVM_COMMON_DEFS_H__ */
