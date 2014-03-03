#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <zebra.h>
#include <dbus/dbus.h>

#include "dcli_main.h"
#include "dcli_facl.h"
#include "nm_errcode.h"
#include "dcli_system.h"
#include "command.h"
#include "if.h"
#include "facl_db.h"
#include "facl_errcode.h"

#include "dcli_domain.h"
#include "drp_def.h"
#include "drp_interface.h"


extern DBusConnection *dcli_dbus_connection;
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

struct cmd_node facl_node = 
{
	FACL_NODE,
	"%s(config-facl)# ",
	1
};


static char *
ip2str(uint32_t ip, char *str, size_t size)
{
	if (NULL == str) {
		return NULL;
	}

	memset(str, 0, size);
	snprintf(str, size-1, "%u.%u.%u.%u",
		(ip>>24)&0xff, (ip>>16)&0xff, (ip>>8)&0xff, ip&0xff);

	return str;
}


DEFUN(create_facl_policy_func,
	create_facl_policy_cmd,
	"create facl-policy <1-2048> FACL-NAME",
	"create\n"
	"create facl-policy\n"
	"facl's tag value <1-2048>\n"
	"facl's name\n"
)
{
	char *facl_name = NULL;
	uint32_t facl_tag = 0;
	int ret = 0;
	
	facl_name = (char *)argv[1];
	facl_tag = strtoul(argv[0], NULL, 10);

	ret = facl_interface_create_policy(dcli_dbus_connection, facl_name, facl_tag);
#if 0
	if (0 != ret) {
		vty_out(vty, "create failed\n");
		return CMD_WARNING;
	}
#else
	if (FACL_RETURN_OK == ret) {
        return CMD_SUCCESS;
	} else if (FACL_TAG_VALUE_ERR == ret){
		vty_out(vty, "facl's tag value limit:%d\n", FACL_TAG_MAX_NUM);
	} else if (FACL_NAME_LEN_ERR == ret){
		vty_out(vty, "facl's name len out size:%d\n", FACL_NAME_MAX_LENGTH - 1);
	} else if (FACL_POLICY_TAG_ALREADY_EXIST == ret) {
		vty_out(vty, "facl's tag already exit\n");
	} else if (FACL_POLICY_NAME_ALREADY_EXIST == ret) {
		vty_out(vty, "facl's name already exit\n");
	} else if (FACL_TOTAL_RULE_NUM_OVER == ret) {
		vty_out(vty, "total facl rule num outsize:%u\n", FACL_TOTAL_RULE_NUM);
	} else {
		vty_out(vty, "unknown error\n");
	}
#endif
	return CMD_WARNING;
}

DEFUN(config_facl_policy_func,
	config_facl_policy_cmd,
	"config facl-policy <1-2048>",
	"config\n"
	"config facl-policy\n"
	"facl's tag value <1-2048>\n"
)
{
	uint32_t facl_tag = 0;
	int ret = 0;
	
	facl_tag = strtoul(argv[0], NULL, 10);

	ret = facl_interface_get_policy(dcli_dbus_connection, facl_tag);
	if (ret <= 0) {
		vty_out(vty, "policy %d not exist\n", facl_tag);
		return CMD_WARNING;
	} else {
		facl_tag = ret;
	}

	if (CONFIG_NODE == vty->node) {
		vty->node = FACL_NODE;
		vty->index = (void*)facl_tag;
	} else {
		vty_out(vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}


DEFUN(delete_facl_policy_func,
	delete_facl_policy_cmd,
	"delete facl-policy <1-2048>",
	"delete\n"
	"delete facl-policy\n"
	"facl's tag value <1-2048>\n"
)
{
	uint32_t facl_tag = 0;
	int ret = 0;
	
	facl_tag = strtoul(argv[0], NULL, 10);

	ret = facl_interface_delete_policy(dcli_dbus_connection, facl_tag);
#if 0
        if (0 != ret) {
            vty_out(vty, "delete failed\n");
            return CMD_WARNING;
        }
#else
        if (FACL_RETURN_OK == ret) {
            return CMD_SUCCESS;
        } else if (FACL_TAG_VALUE_ERR == ret){
            vty_out(vty, "facl's tag value limit:%d\n", FACL_TAG_MAX_NUM);
        } else if (FACL_POLICY_TAG_ALREADY_EXIST == ret) {
            vty_out(vty, "facl's tag already exit\n");
        } else if (FACL_POLICY_TAG_NOT_EXIST == ret) {
            vty_out(vty, "facl's tag not find\n");
        } else {
            vty_out(vty, "unknown error\n");
        }
#endif
	return CMD_WARNING;
}

DEFUN(add_facl_rule_func,
	add_facl_rule_cmd,
	"add rule INDEX (deny|permit) (any|INIF) (any|OUTIF) (any|SOURIP) (any|DESTIP) (any|tcp|udp|icmp) (any|SOURPORT) (any|DESTPORT)",
	"add\n"
	"add rule\n"
	"rule's id num for rule's priority\n"
	"rule's action type deny :means reject all matched packet\n"
	"rule's action type permit :means access all matched packet\n"
	"rule's parameter: input interface all\n"
	"rule's parameter: input interface's name\n"
	"rule's parameter: output interface all\n"
	"rule's parameter: output interface's name\n"
	"rule's parameter: source ip(any)\n"
	"rule's parameter: source ip[format like(A.B.C.D) or (A.B.C.D-A.B.C.D) or (A.B.C.D/X.X.X.X)]\n"
	"rule's parameter: destination ip(any)\n"
	"rule's parameter: destination ip[format like(A.B.C.D) or (A.B.C.D-A.B.C.D) or (A.B.C.D/X.X.X.X)]\n"
	"rule's parameter: protocal any\n"
	"rule's parameter: protocal tcp\n"
	"rule's parameter: protocal udp\n"
	"rule's parameter: protocal icmp\n"
	"rule's parameter: source port(any)\n"
	"rule's parameter: source port(format like 80 or 22:30)\n"
	"rule's parameter: destination port(any)\n"
	"rule's parameter: destination port(format like 80 or 22:30)\n"
)
{
	uint32_t facl_tag = 0;
	uint32_t id = 0;
	uint32_t type = 0;
	int proto = 0;
	char *inif = NULL, *outif = NULL;
	char *srcip = NULL, *dstip = NULL;
	char *srcport = NULL, *dstport = NULL;
	int ret = 0;

	facl_tag = (uint32_t)(vty->index);
	
	id = strtoul(argv[0], NULL, 10);
	if (0 == strncmp("deny", argv[1], strlen(argv[1]))) {
		type = 1;
	}
	inif = (char *)argv[2];
	outif = (char *)argv[3];
	srcip = (char *)argv[4];
	dstip = (char *)argv[5];
	if (0 == strncmp("icmp", argv[6], strlen(argv[6]))) {
		proto = 1;
	} else if (0 == strncmp("tcp", argv[6], strlen(argv[6]))) {
		proto = 6;
	} else if (0 == strncmp("udp", argv[6], strlen(argv[6]))) {
		proto = 17;
	}
	srcport = (char *)argv[7];
	dstport = (char *)argv[8];
	ret = facl_interface_add_rule(dcli_dbus_connection, facl_tag, id, type, 
					inif, outif, srcip, dstip, proto, srcport, dstport, "");
	if (FACL_RETURN_OK == ret) {
        return CMD_SUCCESS;
	} else if (FACL_INDEX_ALREADY_EXIST == ret) {
		vty_out(vty, "facl's rule index already exit\n");
	} else if (FACL_TOTAL_RULE_NUM_OVER == ret) {
		vty_out(vty, "total facl rule num outsize:%u\n", FACL_TOTAL_RULE_NUM);
	} else {
		vty_out(vty, "create failed\n");
	}
	
	return CMD_WARNING;
}

DEFUN(add_facl_domain_rule_func,
	add_facl_damain_rule_cmd,
	"add rule INDEX (deny|permit) (any|INIF) (any|OUTIF) DOMAIN",
	"add\n"
	"add rule\n"
	"rule's id num for rule's priority\n"
	"rule's action type deny :means reject all matched packet\n"
	"rule's action type permit :means access all matched packet\n"
	"rule's parameter: input interface all\n"
	"rule's parameter: input interface's name\n"
	"rule's parameter: output interface all\n"
	"rule's parameter: output interface's name\n"
	"rule's parameter: domain(any)\n"
	"rule's parameter: domain url\n"
)
{
	uint32_t facl_tag = 0;
	uint32_t id = 0;
	uint32_t type = 0;
	char *inif = NULL, *outif = NULL;
	char *domain = NULL;
	char *srcip = NULL, *dstip = NULL;
	char ipstr[32] = {0};
	int slotid = HostSlotId;
	domain_pt domain_conf;
	domain_ct domain_ctr;
	int ret = 0;
	int i = 0;

	facl_tag = (uint32_t)(vty->index);
	
	id = strtoul(argv[0], NULL, 10);
	if (0 == strncmp("deny", argv[1], strlen(argv[1]))) {
		type = 1;
	}
	inif = (char *)argv[2];
	outif = (char *)argv[3];
	domain = (char *)argv[4];

	ret = conf_drp_get_dbus_connect_params(&slotid);
	if (ret < 0){
		vty_out(vty, "facl get drp connection config error:%d\n",ret);
		return CMD_FAILURE;
	}
	memset(&domain_conf,0,sizeof(domain_conf));
	strncpy((domain_conf.domain_name),domain,sizeof(domain_conf.domain_name)-1);
	memset(&domain_ctr,0,sizeof(domain_ctr));
	ReInitDbusConnection(&dcli_dbus_connection,slotid,distributFag);
	ret = conf_drp_get_domain_ip(dcli_dbus_connection,	&domain_conf, &domain_ctr);
	_drp_return_if_fail(0 == ret,ret,CMD_WARNING);
	if (0 == domain_ctr.num) {
		vty_out(vty, "%% this domain can not parse, please check it\n");
		return CMD_SUCCESS;
	}

	if (0 != ret) {
		vty_out(vty, "%% this domain can not parse, please check it\n");
		return CMD_SUCCESS;
	}
	for (i = 0; i<domain_ctr.num; i++){
		ip2str(domain_ctr.domain_ip[i].ipaddr, ipstr, sizeof(ipstr));
		ret = facl_interface_add_rule(dcli_dbus_connection, facl_tag, id, type, 
				inif, outif, ipstr, "any", 0, "any", "any", domain);
		if (FACL_RETURN_OK != ret) {
			break;
		}
		ret = facl_interface_add_rule(dcli_dbus_connection, facl_tag, id, type, 
				inif, outif, "any", ipstr, 0, "any", "any", domain);
		if (FACL_RETURN_OK != ret) {
			break;
		}
	}
	
	if (FACL_RETURN_OK == ret) {
        return CMD_SUCCESS;
	} else if (FACL_INDEX_ALREADY_EXIST == ret) {
		vty_out(vty, "facl's rule index already exit\n");
	} else if (FACL_TOTAL_RULE_NUM_OVER == ret) {
		vty_out(vty, "total facl rule num outsize:%u\n", FACL_TOTAL_RULE_NUM);
	} else {
		vty_out(vty, "create failed\n");
	}
	
	return CMD_WARNING;
}

DEFUN(del_facl_rule_func,
	del_facl_rule_cmd,
	"del rule INDEX",
	"del\n"
	"del rule\n"
	"rule's id for rule's priority\n"
)
{
	uint32_t facl_tag = 0;
	uint32_t id = 0;
	int ret = 0;
	
	facl_tag = (uint32_t)(vty->index);
	
	id = strtoul(argv[0], NULL, 10);

	ret = facl_interface_del_rule(dcli_dbus_connection, facl_tag, id);
	if (0 != ret) {
		vty_out(vty, "del rule failed\n");
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}

DEFUN(show_facl_rule_func,
	show_facl_rule_cmd,
	"show rule running config",
	"show\n"
	"show rule\n"
	"show rule running\n"
	"show rule running config"
)
{	
    struct list_head rule_head = {0};
	facl_rule_t *rule = NULL;
	uint32_t facl_tag = 0;
	int ret = 0;
	int i = 0;
	
	memset(&rule_head, 0, sizeof(rule_head));
	INIT_LIST_HEAD(&rule_head);
	
	facl_tag = (uint32_t)(vty->index);
	
	ret = facl_interface_show_rule(dcli_dbus_connection, facl_tag, &rule_head);

	if (FACL_RETURN_OK == ret) {
        vty_out(vty, " facl-policy : %d\n", facl_tag);
        vty_out(vty, " -----Index ActionType InputIntf OutputIntf SourceIp DestIp Protocal SourcePort DestPort Domain\n");
        list_for_each_entry(rule, &rule_head, node) {
        	i++;
            vty_out(vty, " add rule %u %s %s %s %s %s %s %s %s %s\n",
                rule->index, FACL_RULE_TYPE_PERMIT==rule->data.type?"permit":"deny",
                rule->data.inif, rule->data.outif, rule->data.srcip, rule->data.dstip,
                facl_proto_to_str(rule->data.proto), rule->data.srcport, rule->data.dstport, rule->data.domain);
        }
        vty_out(vty, " total rule num : %d\n", i);
    } else {
		vty_out(vty, "show rule failed\n");
		return CMD_WARNING;
    }
    facl_interface_free_rule(&rule_head);
    
	return FACL_RETURN_OK;
}

DEFUN(show_facl_running_fun,
	show_facl_running_cmd,
	"show facl running config",
	"show\n"
	"show facl\n"
	"show facl running\n"
	"show facl running config\n"
)
{
	struct list_head policy_buf_head = {0};
	policy_rule_buf_t *policy_buf = NULL;
	int ret = 0;
	
	memset(&policy_buf_head, 0, sizeof(policy_buf_head));
	INIT_LIST_HEAD(&policy_buf_head);

	ret = facl_interface_show_running(dcli_dbus_connection, &policy_buf_head);
	if (0 != ret) {
		vty_out(vty, "facl show running failed, ret=%d\n", ret);
		return CMD_WARNING;
	}
	
	list_for_each_entry(policy_buf, &policy_buf_head, node) {
		vty_out(vty, "%s\n", policy_buf->buf);
	}
	
	facl_interface_free_policy_buf(&policy_buf_head);
	
	return CMD_SUCCESS;
}

int
dcli_facl_show_running_return(struct vty* vty)
{
	struct list_head policy_buf_head = {0};
	policy_rule_buf_t *policy_buf = NULL;
	int ret = 0;
	
	memset(&policy_buf_head, 0, sizeof(policy_buf_head));
	INIT_LIST_HEAD(&policy_buf_head);

	ret = facl_interface_show_running(dcli_dbus_connection, &policy_buf_head);

	if (0 != ret) {
		return CMD_WARNING;
	}
	
	vtysh_add_show_string("\n!FACL section.\n\n");
	list_for_each_entry(policy_buf, &policy_buf_head, node) {
        vtysh_add_show_string(policy_buf->buf);
	}
	
	facl_interface_free_policy_buf(&policy_buf_head);
	
	return CMD_SUCCESS;
}

void 
dcli_facl_init(void)
{
	install_node(&facl_node, dcli_facl_show_running_return, "FACL_NODE");
	install_default(FACL_NODE);
	
	install_element(CONFIG_NODE, &create_facl_policy_cmd);
	install_element(CONFIG_NODE, &config_facl_policy_cmd);
	install_element(CONFIG_NODE, &delete_facl_policy_cmd);
	install_element(CONFIG_NODE, &show_facl_running_cmd);

	install_element(FACL_NODE, &add_facl_rule_cmd);
	install_element(FACL_NODE, &add_facl_damain_rule_cmd);
	install_element(FACL_NODE, &del_facl_rule_cmd);	
	install_element(FACL_NODE, &show_facl_rule_cmd);	
}


