#ifndef __NETLINK_H__
#define __NETLINK_H__

#include <linux/if_ether.h>



struct pppoe_debug {
	int set_flag;		/* 1 : open log, 0 : no log */
	int log_level;
};

typedef enum pppoe_msg_type {
	pppoe_snp_type,
	dba_msg_type
} msg_type_t;

/* pppoe snooping submsg type */
typedef enum pppoe_msg_subtype {
	pppoe_snp_subtype_enable,
	pppoe_snp_subtype_setmru,
	pppoe_snp_subtype_logswitch
}ppoesnp_subtype_t;
/*
#define PPPOE_SNP_SUBMSG_ENABLE			0x01
#define PPPOE_SNP_SUBMSG_SETMRU			0x02
#define PPPOE_SNP_SUBMSG_LOGSWITCH		0x03
*/


/* pppoe snooping submsg type */
typedef enum dba_msg_subtype {
	dba_subtype_enable,
}dba_subtype_t;

struct msg_type {
	msg_type_t type;
	unsigned int subtype;	
//	msg_subtype_t subtype;	
	int datalen;
	union {
		unsigned short mru;
		unsigned int enable_flag;
		struct pppoe_debug debug;
	}data;
};

typedef struct msg_type pppoe_msg_t;
typedef struct msg_type dba_msg_t;

int pppoe_snooping_netlink_init(void);
int pppoe_snooping_netlink_release(void);

#endif /* __PPPOE_SNP_NETLINK_H__ */

