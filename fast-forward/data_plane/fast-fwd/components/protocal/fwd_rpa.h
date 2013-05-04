#ifndef _FWD_RPA_H_
#define _FWD_RPA_H_


#include "cvmx.h"
#include "cvmx-wqe.h"
#ifdef SDK_VERSION_2_2
#include "fastfwd-common-rnd.h"
#include "fastfwd-common-misc.h"
#include "fastfwd-common-defs.h"
#include "fastfwd-common-fpa.h"
#else
#include "cvm-common-rnd.h"
#include "cvm-common-misc.h"
#include "cvm-common-defs.h"
#include "cvm-common-fpa.h"
#endif
#include "acl.h"


#define RPA_COOKIE            0x8fff
#define TIPC_COOKIE           0x88ca

/*for tag*/

#define RPA_ENET_ETHER_ADDR_LEN   6
#define RPA_HEAD_LEN              18

#define RETURN_TO_LINUX  1
#define RETURN_CONTINUE  2
#define IS_RPA                0x1234
#define NOT_RPA               0x4567

typedef struct {
	unsigned char  ether_dhost[RPA_ENET_ETHER_ADDR_LEN];
	unsigned char  ether_shost[RPA_ENET_ETHER_ADDR_LEN];
    unsigned short cookie;
	unsigned char  type; 
	unsigned char  dnetdevNum;
	unsigned char  snetdevNum;
	unsigned char  d_s_slotNum; /* dst port and src slot number */
}rpa_eth_head_t;

typedef struct {
	unsigned char  ether_dhost[RPA_ENET_ETHER_ADDR_LEN];
	unsigned char  ether_shost[RPA_ENET_ETHER_ADDR_LEN];
	unsigned short vlan_type;
	unsigned short vlan_tag;
    unsigned short cookie;
	unsigned char  type; 
	unsigned char  dnetdevNum;
	unsigned char  snetdevNum;
	unsigned char  d_s_slotNum; /* dst port and src slot number */
}rpa_vlan_eth_head_t;


/* mac is 0 */
static inline int is_zero_ether_addr(const uint8_t *addr)
{
	return !(addr[0] | addr[1] | addr[2] | addr[3] | addr[4] | addr[5]);
}

extern inline int rpa_packet_handle(cvmx_wqe_t* work,cvm_common_ip_hdr_t **ip,uint32_t *action_type);

extern inline void add_rpa_head(cvmx_wqe_t *work, rule_item_t *rule);


#endif
