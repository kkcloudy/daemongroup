/*******************************************************************************
Copyright (C) Autelan Technology


This software file is owned and distributed by Autelan Technology 
********************************************************************************


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************
* facl_rule.h
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
* facl rule
*
*
*******************************************************************************/

#ifndef _FACL_DB_H
#define _FACL_DB_H

#include <stdint.h>
#include "nm_list.h"
#include "nm_blkmem.h"

#define FACL_TAG_MAX_NUM			2048
#define FACL_INDEX_MAX_NUM			128
#define FACL_TOTAL_RULE_NUM			4096

#define FACL_NAME_MAX_LENGTH		256
#define FACL_INTF_NAME_MAX_LENGTH	16
#define FACL_DOMAIN_NAME_MAX_LENGTH	64
#define FACL_CHAIN_NAME_MAX_SIZE	32
#define FACL_CHAIN_NAME_MAX_LENGTH	30
#define FACL_IPSTR_MAX_LENGTH		64
#define FACL_PORTSTR_MAX_LENGTH		128
#define FACL_RULE_INFO_BUF_SIZE		256

#define FACL_RULE_TYPE_PERMIT		0
#define FACL_RULE_TYPE_DENY			1

#define FACL_FW_FILTER_CHAIN		"FW_FILTER"
#define FACL_FW_NAT_CHAIN			"FW_DNAT"
#define FACL_DENY_TARGET		"DROP"

typedef struct facl_db facl_db_t;

typedef struct facl_rule facl_rule_t;

typedef struct facl_policy facl_policy_t;

typedef struct policy_rule_buf policy_rule_buf_t;

typedef enum{
	FACL_IPANY,
	FACL_IPSINGLE,
	FACL_IPHOST,						/* defined but not used */
	FACL_IPMASK,
	FACL_IPNET,
	FACL_IPRANG,
}FACL_IP_TYPE;

typedef enum{
	FACL_PTANY,
	FACL_PTSINGLE,
	FACL_PTRANG,
	FACL_PTMULTI,
}FACL_PORT_TYPE;

typedef enum{
	FACL_PRANY	= 0,
	FACL_ICMP	= 1,
	FACL_TCP	= 6,
	FACL_UDP	= 17,
}FACL_PROTO_TYPE;

struct rule_info {	
	uint32_t id;
	int type;						/* permit or deny */
	char domain[FACL_DOMAIN_NAME_MAX_LENGTH];

	char inif[FACL_INTF_NAME_MAX_LENGTH];
	char outif[FACL_INTF_NAME_MAX_LENGTH];

	FACL_IP_TYPE srcip_type;
	char srcip[FACL_IPSTR_MAX_LENGTH];
	FACL_IP_TYPE dstip_type;
	char dstip[FACL_IPSTR_MAX_LENGTH];

	int proto;					/* icmp,tcp,udp */

	FACL_PORT_TYPE srcport_type;
	char srcport[FACL_PORTSTR_MAX_LENGTH];
	FACL_PORT_TYPE dstport_type;
	char dstport[FACL_PORTSTR_MAX_LENGTH];	
};

struct facl_rule {
	struct list_head node;

	uint32_t index;	

	char policy_filter_chain[FACL_CHAIN_NAME_MAX_SIZE];
	char policy_nat_chain[FACL_CHAIN_NAME_MAX_SIZE];
	struct rule_info data;

	facl_policy_t *policy;
	facl_db_t *facldb;
};

struct facl_policy {
	struct list_head policy_node;
	struct list_head rule_head;
	char facl_name[FACL_NAME_MAX_LENGTH];
	uint32_t facl_tag;
	int rule_num;
	facl_db_t *facldb;
};

struct facl_db {
	int policy_num;
	int total_rule_num;
	struct list_head policy_head;
	nm_blk_mem_t *policy_blkmem;
	nm_blk_mem_t *rule_blkmem;
};

struct policy_rule_buf {
	struct list_head node;
	uint32_t facl_tag;
	char facl_name[FACL_NAME_MAX_LENGTH];
	char buf[FACL_RULE_INFO_BUF_SIZE*FACL_INDEX_MAX_NUM];
};/*for all rule in each policy*/

facl_policy_t *
facl_policy_find_by_tag(facl_db_t *facldb, const uint32_t facl_tag);

int
facl_add_policy(facl_db_t *facldb, const char *facl_name, uint32_t facl_tag);

int
facl_del_policy_by_name(facl_db_t *facldb, const char *facl_name);

int
facl_del_policy_by_tag(facl_db_t *facldb, const uint32_t facl_tag);

int
facl_add_rule(facl_policy_t *policy, struct rule_info *input);

int
facl_del_rule(facl_policy_t *policy, uint32_t index);

int
facl_db_show_running(facl_db_t *facldb, char *showStr, int size);

int
facl_policy_show_running(facl_policy_t *policy, char *showStr, int size);

#endif		/* _FACL_RULE_H */


