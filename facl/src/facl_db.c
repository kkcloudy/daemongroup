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
* facl_rule.c
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

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>


#include "nm_list.h"
#include "nm_mem.h"
#include "nm_log.h"
#include "nm_blkmem.h"
#include "nm_dbus.h"
#include "facl_db.h"
#include "facl_shell.h"

#include "facl_errcode.h"

#define FACL_POLICY_BLKMEM_NAME			"facl_policy_blkmem"
#define FACL_POLICY_BLKMEM_ITEMNUM		128
#define FACL_POLICY_BLKMEM_MAXNUM		32
#define FACL_RULE_BLKMEM_NAME			"facl_rule_blkmem"
#define FACL_RULE_BLKMEM_ITEMNUM		1024
#define FACL_RULE_BLKMEM_MAXNUM			32


facl_db_t *
facl_db_create(void)
{
	facl_db_t *facldb = NULL;
	
	facldb = nm_malloc(sizeof(facl_db_t));
	if (NULL == facldb) {
		nm_log_err("facl_db_create malloc failed");
		goto failed_0;
	}
	
	memset(facldb, 0, sizeof(facl_db_t));
	if (NM_RETURN_OK != nm_blkmem_create(&(facldb->policy_blkmem),
							FACL_POLICY_BLKMEM_NAME,
							sizeof(facl_policy_t),
							FACL_POLICY_BLKMEM_ITEMNUM,
							FACL_POLICY_BLKMEM_MAXNUM)) {
		nm_log_err("facl_db_create blkmem_create failed");
		goto failed_1;
	}

	if (NM_RETURN_OK != nm_blkmem_create(&(facldb->rule_blkmem),
							FACL_RULE_BLKMEM_NAME,
							sizeof(facl_rule_t),
							FACL_RULE_BLKMEM_ITEMNUM,
							FACL_RULE_BLKMEM_MAXNUM)) {
		nm_log_err("facl_db_create blkmem_create failed");
		goto failed_2;
	}
	INIT_LIST_HEAD(&(facldb->policy_head));

	facldb->policy_num = 0;

	nm_log_info("facl_db_create create ok");
	return facldb;

failed_2:
	nm_blkmem_destroy(&(facldb->policy_blkmem));
failed_1:
	nm_free(facldb);
failed_0:
	return NULL;
}

int
facl_db_destroy(facl_db_t *facldb)
{
	if (NULL == facldb) {
		nm_log_err("facl_db_destroy input error");
		return -1;
	}	
	facl_policy_t *policy = NULL; 
	/* clean all rule */
	list_for_each_entry(policy, &(facldb->policy_head), policy_node) {
		facl_shell_del_policy(policy->facl_tag);
	}
	if (NULL != facldb->rule_blkmem) {
		nm_blkmem_destroy(&(facldb->rule_blkmem));
	}
	if (NULL != facldb->policy_blkmem) {
		nm_blkmem_destroy(&(facldb->policy_blkmem));
	}
	nm_free(facldb);

	nm_log_info("facl_db destroy ok");

	return FACL_RETURN_OK;
}

facl_policy_t *
facl_policy_find_by_name(facl_db_t *facldb, const char *facl_name)
{
	facl_policy_t *policy = NULL; 

	if (NULL == facldb || NULL == facl_name || 0 == strlen(facl_name)) {
		nm_log_err("facl_has_facl_name input error");
		return NULL;
	}

	list_for_each_entry(policy, &(facldb->policy_head), policy_node) {
		if (0 == strcmp(policy->facl_name, facl_name)) {
			return policy;
		}
	}

	return NULL;	
}

facl_policy_t *
facl_policy_find_by_tag(facl_db_t *facldb, const uint32_t facl_tag)
{
	facl_policy_t *policy = NULL; 

	if (NULL == facldb) {
		nm_log_err("facl_has_facl_name input error");
		return NULL;
	}

	list_for_each_entry(policy, &(facldb->policy_head), policy_node) {
		if (facl_tag == policy->facl_tag) {
			return policy;
		}
	}

	return NULL;	
}

int
facl_policy_check_conflict(facl_db_t *facldb, const char *facl_name, const uint32_t facl_tag)
{
	facl_policy_t *policy = NULL; 

	if (NULL == facldb || NULL == facl_name || 0 == strlen(facl_name)) {
		nm_log_err("facl_policy_check_conflict input error");
		return -1;
	}

	list_for_each_entry(policy, &(facldb->policy_head), policy_node) {
		if (facl_tag == policy->facl_tag) {
			nm_log_warning("facl_policy_check_conflict tag(%u) conflict", facl_tag);
			return FACL_POLICY_TAG_ALREADY_EXIST;
		}
		if (0 == strcmp(policy->facl_name, facl_name)) {
			nm_log_warning("facl_policy_check_conflict name(%s) conflict", facl_name);
			return FACL_POLICY_NAME_ALREADY_EXIST;
		}
	}

	return FACL_RETURN_OK;	
}

facl_policy_t *
facl_policy_new(facl_db_t *facldb, const char *facl_name, const uint32_t facl_tag)
{
	facl_policy_t *policy = NULL; 

	if (NULL == facldb || NULL == facl_name || 0 == strlen(facl_name)) {
		nm_log_err("facl_policy_new input error");
		return NULL;
	}
	policy = nm_blkmem_malloc_item(facldb->policy_blkmem);
	if (NULL == policy) {
		nm_log_err("facl_policy_new blkmem_malloc_item failed");
		return NULL;
	}

	memset(policy, 0, sizeof(facl_policy_t));
	strncpy(policy->facl_name, facl_name, sizeof(policy->facl_name));
	policy->facl_tag = facl_tag;
	policy->facldb = facldb;
	policy->rule_num = 0;
	INIT_LIST_HEAD(&(policy->rule_head));

	nm_log_debug("facl_db", "facl_policy_new OK, facl_name=%s, facl_tag=%u",
				policy->facl_name, policy->facl_tag);

	return policy;

}

int
facl_policy_free(facl_policy_t *policy)
{
	facl_db_t *facldb = NULL;

	if (NULL == policy) {
		nm_log_err("facl_policy_free input err");
		return -1;
	}
	facldb = policy->facldb;

	nm_log_debug("facl_db", "facl_policy_free, facl_name=%s, facl_tag=%u",
					policy->facl_name, policy->facl_tag);

	nm_blkmem_free_item(facldb->policy_blkmem, policy);
	
	return FACL_RETURN_OK;
}


int
facl_add_policy(facl_db_t *facldb, const char *facl_name, uint32_t facl_tag)
{
	facl_policy_t *policy = NULL;
	int ret = 0;
	
	if (NULL == facldb || NULL == facl_name) {
		nm_log_err("facl_add_policy input error");
		return -1;
	}

	if (strlen(facl_name) > 255 || 0 == strlen(facl_name)) {
		nm_log_err("facl_add_policy facl_name length error");
		return NM_ERR_INPUT_PARAM_ERR;
	}

	if (facl_tag > FACL_TAG_MAX_NUM) {
		nm_log_err("facl_add_policy facl tag is too big");
		return NM_ERR_INPUT_PARAM_ERR;
	}

	ret = facl_policy_check_conflict(facldb, facl_name, facl_tag);
	if (FACL_RETURN_OK != ret) {
		nm_log_err("facl_add_policy facl_policy_check_conflict failed, ret=%d", ret);
		return ret;
	}

	policy = facl_policy_new(facldb, facl_name, facl_tag);
	if (NULL == policy) {
		nm_log_err("facl_add_policy facl_policy_new failed");
		return NM_ERR_MALLOC_FAILED;
	}

	ret = facl_shell_add_policy(facl_tag);
	if (0 != ret) {
		nm_log_err("facl_add_policy facl_shell_add_policy tag(%u) failed", facl_tag);
		facl_policy_free(policy);
		return ret;
	}
	
	list_add_tail(&(policy->policy_node), &(facldb->policy_head));

	facldb->policy_num++;

	return NM_RETURN_OK;
}

int
facl_del_policy_by_name(facl_db_t *facldb, const char *facl_name)
{
	facl_policy_t *policy = NULL;
	int ret = 0;

	if (NULL == facldb || NULL == facl_name) {
		nm_log_err("facl_del_policy_by_name input error");
		return -1;
	}
		
	if (strlen(facl_name) > 255 || 0 == strlen(facl_name)) {
		nm_log_err("facl_del_policy_by_name facl_name length error");
		return NM_ERR_INPUT_PARAM_ERR;
	}

	policy = facl_policy_find_by_name(facldb, facl_name);
	if (NULL == policy) {
		nm_log_err("facl_del_policy_by_name not find %s policy", facl_name);
		return NM_ERR_NULL_POINTER;
	}

	ret = facl_shell_del_policy(policy->facl_tag);
	if (0 != ret) {
		nm_log_err("facl_del_policy_by_name facl_shell_del_policy tag(%u) failed", policy->facl_tag);
		return ret;
	}

    facldb->total_rule_num -= policy->rule_num;

	list_del(&(policy->policy_node));
	facldb->policy_num--;

	facl_policy_free(policy);

	return FACL_RETURN_OK;
}

int
facl_del_policy_by_tag(facl_db_t *facldb, const uint32_t facl_tag)
{
	facl_policy_t *policy = NULL;
	int ret = 0;
#if 0
	if (NULL == facldb || NULL == facl_name) {
		nm_log_err("facl_del_policy_by_name input error");
		return -1;
	}
		
	if (strlen(facl_name) > 255 || 0 == strlen(facl_name)) {
		nm_log_err("facl_del_policy_by_name facl_name length error");
		return NM_ERR_INPUT_PARAM_ERR;
	}
#endif
	policy = facl_policy_find_by_tag(facldb, facl_tag);
	if (NULL == policy) {
		nm_log_err("facl_del_policy_by_tag not find %u policy", facl_tag);
		return FACL_POLICY_TAG_NOT_EXIST;
	}

	ret = facl_shell_del_policy(policy->facl_tag);
	if (0 != ret) {
		nm_log_err("facl_del_policy_by_tag facl_shell_del_policy tag(%u) failed", policy->facl_tag);
		return ret;
	}
	
    facldb->total_rule_num -= policy->rule_num;

	list_del(&(policy->policy_node));
	facldb->policy_num--;

	facl_policy_free(policy);

	return FACL_RETURN_OK;
}

char *
facl_proto_to_str(int proto)
{
	switch (proto) {
	case FACL_ICMP:
		return "icmp";
	case FACL_TCP:
		return "tcp";
	case FACL_UDP:
		return "udp";
	case FACL_PRANY:
	default:
		return "any";
	}
}

int
facl_rule_check_index_legal(struct list_head *rule_head, uint32_t index)
{
	facl_rule_t *rule = NULL;
//	int i = 0;
	if (NULL == rule_head) {
		nm_log_err("facl_rule_check_index_legal input is null");
		return NM_ERR_INPUT_PARAM_ERR;
	}
	if (0 == index || index > FACL_INDEX_MAX_NUM) {
		nm_log_err("facl_rule_check_index_legal index should be 1~128");
		return FACL_INDEX_INPUT_ERR;
	}
#if 0
	list_for_each_entry(rule, rule_head, node) {
		if (index == rule->index) {
			return FACL_INDEX_ALREADY_EXIST;
		}
		nm_log_debug("facl_db", "facl_rule_check_index_legal rule_id=%u, rule_index=%u",
						rule->data.id, rule->index);		
	}
#endif
	return FACL_RETURN_OK;
}

int
facl_rule_check_proto_legal(int proto)
{
	switch (proto) {
	case FACL_PRANY:
	case FACL_ICMP:
	case FACL_TCP:
	case FACL_UDP:
		return FACL_RETURN_OK;
	default:
		return FACL_PROTO_UNKNOWN;
	}
}

int
facl_rule_check_intf_legal(const char *intf)
{
	struct ifreq ifr;
	int sk = 0;
	
	if (NULL == intf) {
		nm_log_err("facl_rule_check_intf_legal input is null");
		return NM_ERR_INPUT_PARAM_ERR;
	}

	if (0 == strcmp(intf, "any")) {
		return FACL_RETURN_OK;
	}
	
	sk = socket(AF_INET, SOCK_DGRAM, 0);
	if (sk <= 0) {
		nm_log_err("facl_rule_check_intf_legal socket failed");
		return FACL_INTF_FAILED;
	}
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, intf, sizeof(ifr.ifr_name) - 1); 		

	if (ioctl(sk, SIOCGIFINDEX, &ifr)) {
		nm_log_err("facl_rule_check_intf_legal get intf failed");
		close(sk);
		return FACL_INTF_FAILED;
	}

	close(sk);
	return FACL_RETURN_OK;
}

/* not support IPNET IPHOST yet*/
int
facl_rule_check_ip_legal(const char *ipstr, FACL_IP_TYPE *ip_type)
{
	const char *tempstr = NULL;
	char *endptr = NULL;
	uint8_t addr[8] = {0};
	int i = 0;

	if (NULL == ipstr || NULL == ip_type) {
		nm_log_err("facl_rule_check_ip_legal input is null");
		return NM_ERR_INPUT_PARAM_ERR;
	}
	
	if (0 == strcmp(ipstr, "any")) {
		*ip_type = FACL_IPANY;
		return FACL_RETURN_OK;
	} 
	
	for (i = 0, tempstr = ipstr; i < 8; i++) {
		uint32_t ip = strtoul(tempstr, &endptr, 10);
		if (((0 == i || 4 == i) && 0 == ip) || ip > 255) {
			return FACL_IP_FORMAT_ERR;
		} else if (endptr && ((ip < 10 && (endptr - tempstr) > 1)
				|| (ip < 100 && (endptr - tempstr) > 2) || (ip < 256 && (endptr - tempstr) > 3))) {
			return FACL_IP_FORMAT_ERR;
		}

		addr[i] = (uint8_t) ip;

		if (3 == i) {
			if (NULL == endptr || '\0' == endptr[0]) {
				if (0 == addr[i]) {
					return FACL_IP_FORMAT_ERR;
				} else {
					*ip_type = FACL_IPSINGLE;
					return FACL_RETURN_OK;
				}
			} else if ('/' == endptr[0]) {
				*ip_type = FACL_IPMASK;
			} else if ('-' == endptr[0]) {
				if(0 == addr[i]) {
					return FACL_IP_FORMAT_ERR;
				}
				*ip_type = FACL_IPRANG;
			} else {
				return FACL_IP_FORMAT_ERR;
			}
		}

		if (7 == i && FACL_IPRANG == *ip_type) {
			if (0 == addr[i]) {
				return FACL_IP_FORMAT_ERR;
			}
		}		
		tempstr = endptr + 1;
	}

	if (FACL_IPMASK == *ip_type) {
		uint32_t *mask, flag = 0;
		mask = (uint32_t *)(addr + 4);
		for(i = 0; i < 32; i++, (*mask) /= 2) {
			if ((*mask) % 2) {
				flag = 1;
			} else if (flag) {
				return FACL_IP_FORMAT_ERR;
			}
		}
		return FACL_RETURN_OK;
	} else if (FACL_IPRANG == *ip_type) {
		if (memcmp(addr + 4, addr, 4) > 0) {
			return FACL_RETURN_OK;
		} else {
			return FACL_IP_FORMAT_ERR;
		}
	}
	
	return FACL_IP_FORMAT_ERR;
}

/* not support multi port yet */
int 
facl_rule_check_port_legal(const char *portstr, FACL_PORT_TYPE *port_type, int proto)
{
	const char *tempstr = NULL;
	char *endptr = NULL;
	uint32_t port[2];
	int i;

	if (NULL == portstr || NULL == port_type) {
		nm_log_err("facl_rule_check_port_legal input is null");
		return NM_ERR_INPUT_PARAM_ERR;
	}
	
	if (0 == strcmp(portstr, "any")) {
		*port_type = FACL_PTANY;
		return FACL_RETURN_OK;
	}

	if (0 == proto || 1 == proto) {
		nm_log_err("facl_rule_check_port_legal proto not match");
		return FACL_PORT_FORMAT_ERR;
	}

	for (tempstr = portstr, i = 0; i < 2; i++) {
		port[i] = strtoul(tempstr, &endptr, 10);
		if (port[i] > 65535) {
			return FACL_PORT_FORMAT_ERR;
		} else if (endptr && ((port[i] < 65536 && (endptr - tempstr) > 5)
				|| (port[i] < 10000 && (endptr - tempstr) > 4) || (port[i] < 1000 && (endptr - tempstr) > 3)
				|| (port[i] < 100 && (endptr - tempstr) > 2) || (port[i] < 10 && (endptr - tempstr) > 1))){
			return FACL_PORT_FORMAT_ERR;
		}
		
		if (NULL == endptr || '\0' == endptr[0]) {
			if(0 == i) {
				*port_type = FACL_PTSINGLE;
				return 0;
			} else if (1 == i) {
				if(port[i] > port[0]) {
					*port_type = FACL_PTRANG;
					return FACL_RETURN_OK;
				}
			}
		} else if (':' != endptr[0]) {
			return FACL_PORT_FORMAT_ERR;
		}

		tempstr = endptr + 1;
	}

	return FACL_PORT_FORMAT_ERR;
}


int
facl_rule_check_input_legal(facl_policy_t *policy, struct rule_info *input)
{
	int ret = 0;
	if (NULL == policy || NULL == input) {
		nm_log_err("facl_rule_check_legal input is null");
		return NM_ERR_INPUT_PARAM_ERR;
	}

	ret = facl_rule_check_index_legal(&(policy->rule_head), input->id);
	if (FACL_RETURN_OK != ret) {
		nm_log_err("facl_rule_check_index_legal failed, ret=%d", ret);
		return ret;
	}
	ret = facl_rule_check_proto_legal(input->proto);
	if (FACL_RETURN_OK != ret) {
		nm_log_err("facl_rule_check_proto_legal failed, proto=%d ret=%d", input->proto, ret);
		return ret;
	}
	ret = facl_rule_check_intf_legal(input->inif);
	if (FACL_RETURN_OK != ret) {
		nm_log_err("facl_rule_check_intf_legal inif failed, ret=%d", ret);
		return ret;
	}
	ret = facl_rule_check_intf_legal(input->outif);
	if (FACL_RETURN_OK != ret) {
		nm_log_err("facl_rule_check_intf_legal outif failed, ret=%d", ret);
		return ret;
	}
	ret = facl_rule_check_ip_legal(input->srcip, &(input->srcip_type));
	if (FACL_RETURN_OK != ret) {
		nm_log_err("facl_rule_check_ip_legal srcip failed, ret=%d", ret);
		return ret;
	}
	ret = facl_rule_check_ip_legal(input->dstip, &(input->dstip_type));
	if (FACL_RETURN_OK != ret) {
		nm_log_err("facl_rule_check_ip_legal dstip failed, ret=%d", ret);
		return ret;
	}
	ret = facl_rule_check_port_legal(input->srcport, &(input->srcport_type), input->proto);
	if (FACL_RETURN_OK != ret) {
		nm_log_err("facl_rule_check_ip_legal srcport failed, ret=%d", ret);
		return ret;
	}
	ret = facl_rule_check_port_legal(input->dstport, &(input->dstport_type), input->proto);
	if (FACL_RETURN_OK != ret) {
		nm_log_err("facl_rule_check_ip_legal dstport failed, ret=%d", ret);
		return ret;
	}

	return FACL_RETURN_OK;
}

facl_rule_t *
facl_find_rule_by_index(struct list_head *rule_head, uint32_t index)
{
	facl_rule_t *rule = NULL;
	if (NULL == rule_head) {
		nm_log_err("facl_find_rule_by_index input is null");
		return NULL;
	}
	if (0 == index || index > FACL_INDEX_MAX_NUM) {
		return NULL;
	}

	list_for_each_entry(rule, rule_head, node) {
		if (index == rule->index) {
			return rule;
		}
	}

	return NULL;
}

int
facl_rule_in_order(facl_policy_t *policy)
{
	facl_rule_t *rule = NULL;
	int i = 0;
	list_for_each_entry(rule, &(policy->rule_head), node) {
		rule->data.id = ++i;
		nm_log_debug("facl_db", "facl_add_rule i=%d rule_id=%u, rule_index=%u",
						i, rule->data.id, rule->index);		
	}
	return FACL_RETURN_OK;
}



facl_rule_t *
facl_rule_new(facl_policy_t *policy, struct rule_info *input)
{
	facl_rule_t *rule = NULL;
	facl_db_t *facldb = NULL;
	
	if (NULL == policy) {
		nm_log_err("facl_rule_new input is null");
		return NULL;
	}

	facldb = policy->facldb;
	
	rule = nm_blkmem_malloc_item(facldb->rule_blkmem);
	if (NULL == rule) {
		nm_log_err("facl_rule_new blkmem_malloc_item failed");
		return NULL;
	}

	memset(rule, 0, sizeof(facl_rule_t));
	rule->policy = policy;
	rule->facldb = facldb;
	rule->index = input->id;
	memcpy(&(rule->data), input, sizeof(rule->data));
	snprintf(rule->policy_filter_chain, FACL_CHAIN_NAME_MAX_LENGTH, 
			"FACL_POLICY_%u_F", policy->facl_tag);
	snprintf(rule->policy_nat_chain, FACL_CHAIN_NAME_MAX_LENGTH, 
			"FACL_POLICY_%u_N", policy->facl_tag);

	nm_log_debug("facl_db", "facl_rule_new index=%u ok", rule->index);
	
	return rule;
}

int
facl_rule_free(facl_rule_t *rule)
{
	facl_db_t *facldb = NULL;
	
	if (NULL == rule) {
		nm_log_err("facl_rule_free input err");
		return -1;
	}

	facldb = rule->facldb;

	nm_log_debug("facl_db", "facl_rule_free, rule index=%u",
					rule->index);

	nm_blkmem_free_item(facldb->rule_blkmem, rule);

	return FACL_RETURN_OK;
}

int
facl_add_rule(facl_policy_t *policy, struct rule_info *input)
{
	facl_rule_t *rule = NULL;
	facl_rule_t *tmp = NULL;	
	int ret = 0;
	int i = 1;
	
	if (NULL == policy || NULL == input) {
		nm_log_err("facl_add_rule input error");
		return NM_ERR_INPUT_PARAM_ERR;
	}
	
	if (policy->facldb->total_rule_num >= FACL_TOTAL_RULE_NUM) {
		nm_log_err("facl_add_rule total rule num outsize:%d", FACL_TOTAL_RULE_NUM);
		return FACL_TOTAL_RULE_NUM_OVER;
    }

	ret = facl_rule_check_input_legal(policy, input);
	if (FACL_RETURN_OK != ret) {
		nm_log_err("facl_add_rule rule input err, ret=%d", ret);
		return ret;
	}

	rule = facl_rule_new(policy, input);
	if (NULL == rule) {
		nm_log_err("facl_add_rule facl_rule_new failed");
		return NM_ERR_MALLOC_FAILED;
	}

	list_for_each_entry(tmp, &(policy->rule_head), node) {
		if (rule->index < tmp->index) {
			break;
		}		
		i++;
		nm_log_debug("facl_db", "facl_add_rule i=%d rule_id=%u, rule_index=%u",
						i, tmp->data.id, tmp->index);
	}
	rule->data.id = i;
	ret = facl_shell_add_rule(rule);
	if (0 != ret) {
		nm_log_err("facl_add_rule facl_shell_add_rule policy(%u) index(%u) failed", 
						policy->facl_tag, input->id);
		facl_rule_free(rule);
		return ret;
	}
	
	
	list_add_tail(&(rule->node), &(tmp->node));
	facl_rule_in_order(policy);
	#if 0
	list_for_each_entry(tmp, &(rule->node), node) {
		tmp->data.id = i++;
		nm_log_debug("facl_db", "facl_add_rule i=%d rule_id=%u, rule_index=%u",
						i, tmp->data.id, tmp->index);
	}	
	#endif
	policy->rule_num++;
	policy->facldb->total_rule_num++;

	return NM_RETURN_OK;
	
}

int
facl_del_rule(facl_policy_t *policy, uint32_t index)
{
	facl_rule_t *rule = NULL;
	int ret = 0;
	
	if (NULL == policy) {
		nm_log_err("facl_add_rule input error");
		return NM_ERR_INPUT_PARAM_ERR;
	}
	rule = facl_find_rule_by_index(&(policy->rule_head), index);
	if (NULL == rule) {
		nm_log_err("facl_del_rule not find rule(%u) in policy(%u), ret=%d",
					index, policy->facl_tag, ret);
		return NM_ERR_NULL_POINTER;
	}

	ret = facl_shell_del_rule(rule);
	if (0 != ret) {
		nm_log_err("facl_del_rule facl_shell_del_rule rule(%u) in policy(%u), ret=%d",
					index, policy->facl_tag, ret);
		return ret;
	}

	list_del(&(rule->node));
	facl_rule_in_order(policy);
	policy->rule_num--;
	policy->facldb->total_rule_num--;

	facl_rule_free(rule);
	
	return NM_RETURN_OK;
}

int
facl_db_show_running(facl_db_t *facldb, char *showStr, int size)
{
	facl_policy_t *policy = NULL; 
	facl_rule_t *rule = NULL;
	char *cursor = showStr;
	int totalLen = 0;
	int tmp_id = 0;

	if (NULL == facldb || NULL == showStr || size <= 0) {
		nm_log_err("facl_db_show_running input error");
		return NM_ERR_INPUT_PARAM_ERR;
	}

	totalLen += snprintf(cursor+totalLen, size-totalLen-1, "\n!%s section.\n\n", "FACL");
	
	list_for_each_entry(policy, &(facldb->policy_head), policy_node) {
		totalLen += snprintf(cursor+totalLen, size-totalLen-1, 
				"create facl-policy %s %u\n", policy->facl_name, policy->facl_tag);
		totalLen += snprintf(cursor+totalLen, size-totalLen-1, 
				"config facl-policy %s\n", policy->facl_name);
		list_for_each_entry(rule, &(policy->rule_head), node) {
			if (NULL != rule->data.domain && 0 != strcmp(rule->data.domain, "")) {
				if (tmp_id == rule->index) {
					continue;
				}
				tmp_id = rule->index;
				totalLen += snprintf(cursor+totalLen, size-totalLen-1, 
				" add rule %u %s %s %s %s\n", 
				rule->index, FACL_RULE_TYPE_PERMIT==rule->data.type?"permit":"deny",
				rule->data.inif, rule->data.outif, rule->data.domain);
			} else {
				totalLen += snprintf(cursor+totalLen, size-totalLen-1, 
					" add rule %u %s %s %s %s %s %s %s %s\n", 
					rule->index, FACL_RULE_TYPE_PERMIT==rule->data.type?"permit":"deny",
					rule->data.inif, rule->data.outif, rule->data.srcip, rule->data.dstip,
					facl_proto_to_str(rule->data.proto), rule->data.srcport, rule->data.dstport);
			}
		}
		
		totalLen += snprintf(cursor+totalLen, size-totalLen-1, " exit\n\n");
	}
	return FACL_RETURN_OK;
}

int
facl_policy_show_running(facl_policy_t *policy, char *showStr, int size)
{
	facl_rule_t *rule = NULL;
	char *cursor = showStr;
	int totalLen = 0;
	int tmp_id = 0;

	if (NULL == policy || NULL == showStr || size <= 0) {
		nm_log_err("facl_policy_show_running input error");
		return NM_ERR_INPUT_PARAM_ERR;
	}

	totalLen += snprintf(cursor+totalLen, size-totalLen-1, 
			"create facl-policy %u %s\n", policy->facl_tag, policy->facl_name);

	if (policy->rule_num <= 0) {
        return FACL_RETURN_OK;
	}

	totalLen += snprintf(cursor+totalLen, size-totalLen-1, 
			"config facl-policy %u\n", policy->facl_tag);
	list_for_each_entry(rule, &(policy->rule_head), node) {
		if (NULL != rule->data.domain && 0 != strcmp(rule->data.domain, "")) {
			if (tmp_id == rule->index) {
				continue;
			}
			tmp_id = rule->index;
			totalLen += snprintf(cursor+totalLen, size-totalLen-1, 
			" add rule %u %s %s %s %s\n", 
			rule->index, FACL_RULE_TYPE_PERMIT==rule->data.type?"permit":"deny",
			rule->data.inif, rule->data.outif, rule->data.domain);
		} else {
			totalLen += snprintf(cursor+totalLen, size-totalLen-1, 
				" add rule %u %s %s %s %s %s %s %s %s\n", 
				rule->index, FACL_RULE_TYPE_PERMIT==rule->data.type?"permit":"deny",
				rule->data.inif, rule->data.outif, rule->data.srcip, rule->data.dstip,
				facl_proto_to_str(rule->data.proto), rule->data.srcport, rule->data.dstport);
		}
        
	        if (totalLen + 8 >= size) {
	            nm_log_err("facl_policy_show_running policy out size");
	            break;
	        }
	}
	
    	totalLen += snprintf(cursor+totalLen, size-totalLen-1, " exit\n");
	return FACL_RETURN_OK;
}

