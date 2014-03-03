#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>


#include "nm_list.h"
#include "nm_mem.h"
#include "nm_log.h"
#include "nm_blkmem.h"
#include "nm_process.h"
#include "facl_db.h"
#include "facl_errcode.h"

extern nmp_mutex_t iptables_lock;

int
facl_rule_to_cmd(struct rule_info *rule, char *cmd, int size)
{
	if (NULL == rule || NULL == cmd) {
		nm_log_err("facl_rule_to_cmd input error");
		return NM_ERR_INPUT_PARAM_ERR;
	}
	memset(cmd, 0, size);
	if (0 != strcmp("any", rule->inif)) {
		if(0 == strncmp(rule->inif, "r", 1)) {
			strncat(cmd, " -m physdev --physdev-in ", size-strlen(cmd));
		} else {
			strncat(cmd, " -i ", size-strlen(cmd));
		}
		strncat(cmd, rule->inif, size-strlen(cmd));
	}
	if (0 != strcmp("any", rule->outif)) {
		if(0 == strncmp(rule->outif, "r", 1)) {
			strncat(cmd, " -m physdev --physdev-out ", size-strlen(cmd));
		} else {
			strncat(cmd, " -o ", size-strlen(cmd));
		}
		strncat(cmd, rule->outif, size-strlen(cmd));
	}

	switch (rule->srcip_type) {
	case FACL_IPSINGLE:
	case FACL_IPMASK:
		strncat(cmd, " -s ", size-strlen(cmd));
		strncat(cmd, rule->srcip, size-strlen(cmd));
		break;
	case FACL_IPRANG:
		strncat(cmd, " -m iprange --src-range ", size-strlen(cmd));
		strncat(cmd, rule->srcip, size-strlen(cmd));
		break;
	default:
		break;
	}

	switch (rule->dstip_type) {
	case FACL_IPSINGLE:
	case FACL_IPMASK:
		strncat(cmd, " -d ", size-strlen(cmd));
		strncat(cmd, rule->dstip, size-strlen(cmd));
		break;
	case FACL_IPRANG:
		strncat(cmd, " -m iprange --dst-range ", size-strlen(cmd));
		strncat(cmd, rule->dstip, size-strlen(cmd));
		break;
	default:
		break;
	}

	if (0 != rule->proto) {
		strncat(cmd, " -p ", size-strlen(cmd));
		sprintf(cmd+strlen(cmd), "%d", rule->proto);
	}

	switch (rule->srcport_type) {
	case FACL_PTSINGLE:
	case FACL_PTRANG:
		strncat(cmd, " --sport ", size-strlen(cmd));
		strncat(cmd, rule->srcport, size-strlen(cmd));
		break;
	default:
		break;
	}

	switch (rule->dstport_type) {
	case FACL_PTSINGLE:
	case FACL_PTRANG:
		strncat(cmd, " --dport ", size-strlen(cmd));
		strncat(cmd, rule->dstport, size-strlen(cmd));
		break;
	default:
		break;
	}
	if (NULL != rule->domain && 0 != strcmp(rule->domain, "")) {
		strncat(cmd, " -m comment --comment ", size-strlen(cmd));
		strncat(cmd, rule->domain, size-strlen(cmd));
	}
	return FACL_RETURN_OK;
}

int
facl_shell_add_policy(uint32_t facl_tag)
{
	int ret = 0;
	char cmd[256] = {0};

	snprintf(cmd, sizeof(cmd), "sudo /usr/bin/facl_shell_add_policy.sh %u > /dev/null 2>&1", facl_tag);
	
	nmp_mutex_lock(&iptables_lock);	
	ret = system(cmd);
	nmp_mutex_unlock(&iptables_lock);
	
	ret = WEXITSTATUS(ret);

	nm_log_debug("facl_shell", "facl_shell_add_policy cmd=%s, ret=%d\n", cmd, ret);
	return ret;
}

int
facl_shell_del_policy(uint32_t facl_tag)
{
	int ret = 0;
	char cmd[256] = {0};

	snprintf(cmd, sizeof(cmd), "sudo /usr/bin/facl_shell_del_policy.sh %u > /dev/null 2>&1", facl_tag);
	
	nmp_mutex_lock(&iptables_lock);	
	ret = system(cmd);
	nmp_mutex_unlock(&iptables_lock);
	
	ret = WEXITSTATUS(ret);

	nm_log_debug("facl_shell", "facl_shell_del_policy cmd=%s, ret=%d\n", cmd, ret);
	return ret;
}

int
facl_shell_add_rule(facl_rule_t *rule)
{
	int ret = 0;
	char cmd[1024] = {0};
	char tmpstr[256] = {0};
	char *filter_target = NULL;
	char *nat_target = NULL;

	if (NULL == rule) {
		nm_log_err("facl_shell_add_rule input error");
		return NM_ERR_INPUT_PARAM_ERR;
	}

	if (FACL_RULE_TYPE_PERMIT == rule->data.type) {
		filter_target = FACL_FW_FILTER_CHAIN;
		nat_target = FACL_FW_NAT_CHAIN;
	} else {
		filter_target = FACL_DENY_TARGET;
		nat_target = FACL_FW_NAT_CHAIN;
	}

	facl_rule_to_cmd(&(rule->data), tmpstr, sizeof(tmpstr));
	
	snprintf(cmd, sizeof(cmd), "sudo /opt/bin/iptables -I %s %u %s -j %s;"
				" sudo /opt/bin/iptables -t nat -I %s %u %s -j %s",
				rule->policy_filter_chain, rule->data.id, tmpstr, filter_target,
				rule->policy_nat_chain, rule->data.id, tmpstr, nat_target);
	
	nmp_mutex_lock(&iptables_lock);	
	ret = system(cmd);
	nmp_mutex_unlock(&iptables_lock);
	
	ret = WEXITSTATUS(ret);

	nm_log_debug("facl_shell", "facl_shell_add_rule cmd=%s, ret=%d\n", cmd, ret);
	return ret;
}

int
facl_shell_del_rule(facl_rule_t *rule)
{
	int ret = 0;
	char cmd[1024] = {0};

	snprintf(cmd, sizeof(cmd), "sudo /opt/bin/iptables -D %s %u; sudo /opt/bin/iptables -t nat -D %s %u",
				rule->policy_filter_chain, rule->data.id,
				rule->policy_nat_chain, rule->data.id);
	
	nmp_mutex_lock(&iptables_lock);	
	ret = system(cmd);
	nmp_mutex_unlock(&iptables_lock);
	
	ret = WEXITSTATUS(ret);

	nm_log_debug("facl_shell", "facl_shell_del_rule cmd=%s, ret=%d\n", cmd, ret);
	return ret;
}


