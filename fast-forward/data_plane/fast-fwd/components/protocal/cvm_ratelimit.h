#ifndef __CVM_RATELIMIT_H__
#define __CVM_RATELIMIT_H__

#include "cvmx-config.h"
#include "cvmx.h"
#include "cvmx-spinlock.h"
#include "cvmx-fpa.h"
#include "cvmx-pip.h"
#include "cvmx-ipd.h"
#include "cvmx-pko.h"
#include "cvmx-dfa.h"
#include "cvmx-pow.h"
#include "cvmx-gmx.h"
#include "cvmx-sysinfo.h"
#include "cvmx-coremask.h"
#include "cvmx-bootmem.h"
#include "cvmx-helper.h"
#include "cvmx-malloc.h"
#include "cvmx-atomic.h"


#define CVM_RCV_PROTOCOL_UNKNOWN 0

#define MAX_MATCH_RULES_NUM 32
#define CVM_RATE_LIMIT_DEFAULT_RULE_NUM 24
#define PACKET_MATCH_BYTE_NUM 16
#define PROTOCOL_NAME_LEN 32
#define ETH_TYPE_8021Q 0x8100

/*MATCH_TYPE only allowed to be uint or ulonglong*/
#define MATCH_TYPE uint64_t
#define MATCH_TYPE_LEN sizeof(MATCH_TYPE)
#define STATISTIC_TYPE uint64_t

#define MATCH_SUCCESS 0
#define MATCH_FAILED -1
#define ETH_VLAN_LEN 4
#define SCALE_FACTOR_BIT_SHIFT  16
#define SCALE_FACTOR            (1 << SCALE_FACTOR_BIT_SHIFT)
#define PASS_PKT 1
#define DROP_PKT (! PASS_PKT)
#define RATELIMIT_KBPS 0
#define RATELIMIT_PPS 1
#define RATELIMIT_TBL_NAME "ratelimit_tbl"
#define BURST_PASS_COUNT 10

typedef struct ratelimit_s
{
    uint32_t   rate;        /* CIR in kbps*/
    uint32_t   depth;       /*CBS in bytes*/

    uint64_t   rate_in_cycles_per_byte;
    uint64_t   depth_in_cycles;
    uint64_t   cycles_prev;

} ratelimit_t; 

typedef struct ratelimit_pps_s
{
    uint32_t   rate_pps;        /* pps */
    uint64_t   rate_in_cycles_per_pkt;
    uint64_t   cycles_prev;

} ratelimit_pps_t; 

/* sigal match rule item struct */
typedef struct protocol_match_item_s{
	uint8_t name[PROTOCOL_NAME_LEN];
	uint64_t protocol_match_rule[PACKET_MATCH_BYTE_NUM];
	uint64_t protocol_match_mask [PACKET_MATCH_BYTE_NUM];
	uint32_t rate_bps;
	uint32_t rate_pps;
	uint32_t rules_length;/* the number of 64bit  */
	uint32_t unused;
	uint64_t drop_counter;/*for statistic */
	uint64_t pass_counter;/*for statistic */
	ratelimit_t ratelimit_bps;/*for calculate*/
	ratelimit_pps_t ratelimit_pps;	/*for calculate*/
} protocol_match_item_t;

typedef struct global_various_s{
	uint64_t car_rate_scaled;
	uint64_t car_rate_pps_scaled;	
	uint64_t cvm_rate_limit_drop_counter;
	uint64_t cvm_rate_limit_pass_counter;
	int 	 cvm_rate_limit_enabled;
	uint32_t last_valid_index;
	uint64_t unused[11];
} global_various_t;

int32_t cvm_rate_limit_init(void);
int32_t cvm_rate_limit(cvmx_wqe_t *work);



#endif

