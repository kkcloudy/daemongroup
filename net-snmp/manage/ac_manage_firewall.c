
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "ws_firewall.h"
#include "ws_dbus_list_interface.h"

#include "ac_manage_def.h"
#include "ac_manage_public.h"

#include "ac_manage_firewall.h"

#define CMD_LEN 1024

#define IPTABLES_PATH	"/opt/bin/"

#define FW_FILTER_CHAIN "FW_FILTER"
#define FW_SNAT_CHAIN "FW_SNAT"
#define FW_DNAT_CHAIN "FW_DNAT"
#define FW_INPUT_CHAIN "FW_INPUT"
#define TC_MANGLE_CHAIN "TRAFFIC_CONTROL"
#define TC_FILTER_CHAIN "TRAFFIC_CONTROL"


struct firewall_rule_list {
	char *name;
	fwRule *list;
	u_long num;
};


static unsigned int firewall_service_status = 0;
static int strict_access_level = 0;
static unsigned int nat_udp_timeout = 0;

static struct firewall_rule_list rule_list[] = {
			{"firewall", 	NULL, 	0},
			{"dnat",		NULL, 	0},
			{"snat",		NULL, 	0},
			{"input",		NULL, 	0}
};
static const u_long rule_type_num = sizeof(rule_list)/sizeof(rule_list[0]);

/* use manage_ifname_is_legal_input instead it */
#if 0
int 
_ifname_is_legal_input(const char *str) {
	if(!str || !str[0]) {
		return AC_MANAGE_INPUT_TYPE_ERROR;
	}

	FILE *fp = NULL;
	char buff[256];
	char *p = NULL;
	const char *cmd = "ip link | sed \"/.*link\\/.*brd.*/d\" | sed \"s/^[0-9]*:.\\(.*\\):.*$/\\1/g\" | sed \"/lo/d\"";
	
	if(0 == strcmp(str, "any")) {
		return AC_MANAGE_SUCCESS;
	}
	
	if(NULL == (fp = popen(cmd, "r"))) {
		return AC_MANAGE_FILE_OPEN_FAIL;
	}
	
	while(fgets(buff, 256, fp)){
		for (p = buff; *p; p++) {
			if ('\r' == *p || '\n' == *p){
				*p = '\0';
				break;
			}
		}
		
		if(0 == strcmp(buff, str)) {
			pclose(fp);
			return AC_MANAGE_SUCCESS;
		}
	}

	pclose(fp);
	return AC_MANAGE_INPUT_TYPE_ERROR;
}
#endif

static void
_rule_copy(fwRule *new_rule, fwRule *rule) {
	memset(new_rule, 0, sizeof(fwRule));
	
	new_rule->type 		= rule->type;
	new_rule->id			= rule->id;
	new_rule->ordernum 	= rule->ordernum;	
	new_rule->enable 		= rule->enable;
	new_rule->status 		= rule->status;
	MANAGE_STRDUP(new_rule->name, rule->name);
	MANAGE_STRDUP(new_rule->comment, rule->comment);

	new_rule->srctype 	= rule->srctype;
	new_rule->dsttype 	= rule->dsttype;
	MANAGE_STRDUP(new_rule->ineth, rule->ineth);
	MANAGE_STRDUP(new_rule->outeth, rule->outeth);
	MANAGE_STRDUP(new_rule->srcadd, rule->srcadd);
	MANAGE_STRDUP(new_rule->dstadd, rule->dstadd);
	
	new_rule->protocl 		= rule->protocl;
	new_rule->sptype 		= rule->sptype;
	new_rule->dptype 		= rule->dptype;
	MANAGE_STRDUP(new_rule->sport, rule->sport);
	MANAGE_STRDUP(new_rule->dport, rule->dport);

	MANAGE_STRDUP(new_rule->connlimit, rule->connlimit);
	
	new_rule->act 		= rule->act;
	MANAGE_STRDUP(new_rule->tcpmss_var, rule->tcpmss_var);
	
	MANAGE_STRDUP(new_rule->pkg_state, rule->pkg_state);
	MANAGE_STRDUP(new_rule->string_filter, rule->string_filter);
	
	new_rule->natiptype 	= rule->natiptype;
	new_rule->natpttype 	= rule->natpttype;
	MANAGE_STRDUP(new_rule->natipadd, rule->natipadd);
	MANAGE_STRDUP(new_rule->natport, rule->natport);

	return ;
}

static fwRule *
_firewall_rule_copy(fwRule *rule) {
	fwRule *new_rule = NULL;

	new_rule = (fwRule *)malloc(sizeof(fwRule));
	if(NULL == new_rule) {
		syslog(LOG_WARNING, "_firewall_rule_copy: MALLOC fail\n");
		return NULL;
	}
	
	_rule_copy(new_rule, rule);

	return new_rule;
}


static void 
manage_get_iptableCmd(const fwRule *rule, char *cmd) {	
	if(!rule->enable) {
		return;
	}
	syslog(LOG_DEBUG, "in manage_get_iptableCmd!");
	if(FW_WALL == rule->type) {
		strcat(cmd, IPTABLES_PATH"iptables ");
		strcat(cmd, "-A "FW_FILTER_CHAIN);
	} else if(FW_INPUT == rule->type) {
		strcat(cmd, IPTABLES_PATH"iptables ");
		strcat(cmd, "-A "FW_INPUT_CHAIN);
	} else {
		strcat(cmd, IPTABLES_PATH"iptables -t nat ");
		if(FW_DNAT == rule->type && FW_MASQUERADE != rule->natiptype) {//因为MASQUERADE 只能在 postrouting 链上
			strcat(cmd, "-A "FW_DNAT_CHAIN);
		} else {
			strcat(cmd, "-A "FW_SNAT_CHAIN);
		}
	}
	
	if(FW_SNAT != rule->type){
		if(rule->ineth && strcmp(rule->ineth, "any")){
			syslog(LOG_DEBUG, "in manage_get_iptableCmd! rule->ineth=%s",rule->ineth);
			if(0 == strncmp(rule->ineth, "r", 1)) {
				strcat(cmd, " -m physdev --physdev-in ");
			} else {
				strcat(cmd, " -i ");
			}
			strcat(cmd, rule->ineth);
		}
	}
	syslog(LOG_DEBUG, "in manage_get_iptableCmd! rule->type=%d",rule->type);
	if(FW_DNAT != rule->type && FW_INPUT != rule->type){
		if(rule->outeth && strcmp(rule->outeth, "any")){
			if(0 == strncmp(rule->outeth, "r", 1)) {
				strcat(cmd, " -m physdev --physdev-out ");
			} else {
				strcat(cmd, " -o ");
			}
			strcat(cmd, rule->outeth);
		}
	}
	syslog(LOG_DEBUG, "in manage_get_iptableCmd! rule->srctype=%d",rule->srctype);
	if(rule->srctype) {
		switch((int)rule->srctype)	{
			case FW_IPSINGLE:
			case FW_IPMASK:
				strcat(cmd, " -s ");
				strcat(cmd, rule->srcadd);
				break;		
			case FW_IPRANG:
				strcat(cmd, " -m iprange --src-range ");
				strcat(cmd, rule->srcadd);	//格式 "IP1-IP2"
				break;

			default:
				break;
		}
	}

	syslog(LOG_DEBUG, "in manage_get_iptableCmd! rule->dsttype=%d",rule->dsttype);
	if(rule->dsttype) {
		switch((int)rule->dsttype) {
			case FW_IPSINGLE:
			case FW_IPMASK:
				strcat(cmd, " -d ");
				strcat(cmd, rule->dstadd);
				break;		
			case FW_IPRANG:
				strcat(cmd, " -m iprange --dst-range ");
				strcat(cmd, rule->dstadd);	//格式 "IP1-IP2"
				break;

			default:
				break;
		}
	}
	syslog(LOG_DEBUG, "in manage_get_iptableCmd! rule->protocl=%d",rule->protocl);
// protocol	
	if(rule->protocl) {
		switch((int)rule->protocl) {
			case FW_PTCP:
			case FW_PTCPANDUDP:
				strcat(cmd, " -p tcp ");
				break;
			case FW_PUDP:
				strcat(cmd, " -p udp ");
				break;
			case FW_PICMP:
				strcat(cmd, " -p icmp ");
				break;

			default:
				break;
		}
		
		if(rule->sptype) {
			switch((int)rule->sptype) {
				case FW_PTSINGLE:
				case FW_PTRANG:
					strcat(cmd, " --sport ");
					strcat(cmd, rule->sport);	//port[:port]
					break;
				case FW_PTCOLLECT:
					/*
					strcat(cmd," -m multiport --sports ");
					strcat(cmd, rule->sport);	//port[,port]
					*/
					break;

				default:
					break;
			}
		}	
		
		if(rule->dptype) {
			switch((int)rule->dptype) {
				case FW_PTSINGLE:
				case FW_PTRANG:
					strcat(cmd, " --dport ");
					strcat(cmd, rule->dport);	//port[:port]
					/*
					if(FW_PTCOLLECT == rule->sptype) {
						strcat(cmd," -m multiport --sports ");
						strcat(cmd, rule->sport);	//port[,port]
					}
					*/
					break;
				case FW_PTCOLLECT:
					strcat(cmd," -m multiport --dports ");
					strcat(cmd, rule->dport);	//port[,port]
					break;

				default:
					break;
			}
		}
	}
	
// state
	
	syslog(LOG_DEBUG, "in manage_get_iptableCmd! rule->pkg_state=%p",rule->pkg_state);
	if(rule->pkg_state && strlen(rule->pkg_state)) {
		syslog(LOG_DEBUG, "in manage_get_iptableCmd! rule->pkg_state=%s",rule->pkg_state);
		strcat( cmd, " -m state --state " );
		strcat( cmd, rule->pkg_state );
		strcat( cmd, " " );
	}
// string filter
	
	syslog(LOG_DEBUG, "in manage_get_iptableCmd! rule->string_filter=%p",rule->string_filter);

	if(rule->string_filter && strlen(rule->string_filter)) {
		syslog(LOG_DEBUG, "in manage_get_iptableCmd! rule->string_filter=%s",rule->string_filter);
		strcat( cmd, " -m string --string \"" );
		strcat( cmd, rule->string_filter );
		strcat( cmd, "\" --algo bm " );
	}

//comment
	syslog(LOG_DEBUG, "in manage_get_iptableCmd! rule->comment=%p",rule->comment);

	if(rule->comment && strlen(rule->comment)) {
		syslog(LOG_DEBUG, "in manage_get_iptableCmd! rule->comment=%s",rule->comment);
		strcat(cmd," -m comment --comment ");
		strcat(cmd, rule->comment);
	}

	syslog(LOG_DEBUG, "in manage_get_iptableCmd! rule->connlimit=%p",rule->connlimit);
	if (rule->connlimit && strlen(rule->connlimit)) {
		syslog(LOG_DEBUG, "in manage_get_iptableCmd! rule->comment=%s",rule->comment);
		strcat(cmd," -m connlimit --connlimit-above ");
		strcat(cmd, rule->connlimit);
	}

// action
//tcpmss  打的补丁～～～
	syslog(LOG_DEBUG, "in manage_get_iptableCmd! rule->act=%d",rule->act);

	if(FW_WALL == rule->type && FW_TCPMSS == rule->act && strlen(rule->tcpmss_var)) {
		strcat( cmd, " -m tcp --tcp-flags FIN,SYN,RST SYN -j TCPMSS --set-mss " );
		strcat( cmd, rule->tcpmss_var );
		#if 1
		/* Add by houyongtao for set ipv6 tcpmss in ip6tables.
		* Due to time emergenc, now is a temporary deal, 
		* TODO: need a complete deal in the future. */
		char temp_buf[CMD_LEN] = "";
		memset(temp_buf, 0, sizeof(temp_buf));
		snprintf(temp_buf, sizeof(temp_buf) - 1, "%s%s", 
				cmd + strlen(IPTABLES_PATH"iptables -A "), " >/dev/null 2>&1;");
		strcat(cmd, ";"IPTABLES_PATH"ip6tables -D ");
		strcat(cmd, temp_buf);
		strcat(cmd, IPTABLES_PATH"ip6tables -D "FW_FILTER_CHAIN" -j ACCEPT >/dev/null 2>&1;");
		strcat(cmd, IPTABLES_PATH"ip6tables -A ");
		strcat(cmd, temp_buf);
		strcat(cmd, IPTABLES_PATH"ip6tables -A "FW_FILTER_CHAIN" -j ACCEPT >/dev/null 2>&1;");
		#endif
	}
	else { //tcpmss
		strcat(cmd, " -j ");
	}
	
	if(FW_WALL == rule->type || FW_INPUT == rule->type) {		
		switch((int)rule->act) {
			case FW_ACCEPT:
				strcat(cmd, "ACCEPT");
				break;
				
			case FW_DROP:
				strcat(cmd, "DROP");
				break;
				
			case FW_REJECT:
				strcat(cmd, "REJECT");
				break;
				
			default:
				break;
		}
	} else if(FW_SNAT == rule->type
			|| (FW_DNAT == rule->type && FW_MASQUERADE == (int)rule->natiptype)) 
	{	
		syslog(LOG_DEBUG, "in manage_get_iptableCmd! rule->type=FW_SNAT | FW_DNAT | FW_MASQUERADE");

		char natip[64] = { 0 };//主要是针对mask用的，其他类型，直接copy就可以了
		if(FW_IPMASK == (int)rule->natiptype
			&& NULL != rule->natipadd) {
			ipmask2iprange(rule->natipadd, natip);
		}
		else if(NULL!=rule->natipadd){
			strncpy(natip, rule->natipadd, sizeof(natip) - 1);
		}
		syslog(LOG_DEBUG, "in manage_get_iptableCmd! rule->natiptype=FW_SNAT | FW_DNAT | FW_MASQUERADE");
		switch((int)rule->natiptype) {
			case FW_MASQUERADE:
				strcat(cmd, "MASQUERADE");
				if(rule->protocl && FW_PICMP != rule->protocl && rule->natpttype) {
					strcat(cmd, " --to-ports ");
					strcat(cmd, rule->natport);
				}
				break;
				
			case FW_IPSINGLE:
			case FW_IPMASK:	
			case FW_IPRANG:
				strcat(cmd, " SNAT --to-source ");
				strcat(cmd, natip);
				if(rule->protocl && FW_PICMP != rule->protocl && rule->natpttype) {
					strcat(cmd, ":");
					strcat(cmd, rule->natport);
				}
				break;

			default:
				break;
		}
		
	} else {
		char natip[64] = { 0 };//主要是针对mask用的，其他类型，直接copy就可以了
		if( FW_IPMASK == (int)rule->natiptype 
			&& NULL != rule->natipadd)
		{
			ipmask2iprange( rule->natipadd, natip );
		}
		else if( NULL != rule->natipadd ){
			strncpy(natip, rule->natipadd, sizeof(natip) - 1);
		}
		
		switch((int)rule->natiptype) {
			case FW_IPSINGLE:
			case FW_IPMASK:	
			case FW_IPRANG:
				strcat(cmd, " DNAT --to-destination ");
				strcat(cmd, natip );
				if(rule->protocl && FW_PICMP != rule->protocl && rule->natpttype) {
					strcat(cmd, ":");
					strcat(cmd, rule->natport);
				}
				break;

			default:
				break;
		}
	}

	//双协议需再生成一条命令
	if(FW_PTCPANDUDP == rule->protocl) {
		char *ptr = NULL;
		char *tmp = (char *)malloc(CMD_LEN);
		if(NULL == tmp){
			return ;
		}
		memset(tmp, 0, sizeof(CMD_LEN));
		
		strncpy(tmp, cmd, CMD_LEN - 1);

		ptr = strstr(tmp, "-p tcp");
		if(NULL != ptr){
			*(ptr + 3) = 'u';
			*(ptr + 4) = 'd';
			
			strcat(cmd," && ");
			strcat(cmd, tmp);
			
			MANAGE_FREE(tmp);
		}

		syslog(LOG_DEBUG, "cmd = %s\n", cmd);
	}
	return ;
}


int
manage_config_firewall_service(u_long status) {
	syslog(LOG_DEBUG, "firewall_service_status = %d, status = %d\n", firewall_service_status, status);
	if((status && firewall_service_status)
		|| (!status && !firewall_service_status)) {
		return AC_MANAGE_SUCCESS;
	}

	int ret =  AC_MANAGE_SUCCESS;
	
	firewall_chmod_conf_file();

	if(status) {
		int i = 0;
		int sys_return, sys_ret;
		char *command;
		fwRule *rule;
		
		system( "[ -e /opt/services/status/iptables_status.status ] || /opt/services/init/iptables_init start >/dev/null 2>&1" );
		system( "echo \"start\" > /opt/services/status/firewall_status.status" );
		
		command = (char *)malloc(CMD_LEN);
		if(NULL == command) {
			return AC_MANAGE_MALLOC_ERROR;
		}

		DORULE_ETHERNET_OFF;

		//Flush firewall rules and delete chains.
		sys_ret = fwServiceFlush();
		if(0 != sys_ret) {
			ret = AC_MANAGE_CONFIG_FLUSH_FAIL;
			goto error;
		}

		//Make current rules effective
		for(i = 0, rule = rule_list[FW_SNAT].list; i <  rule_list[FW_SNAT].num && rule; i++, rule = rule->next) {
			memset(command, 0, CMD_LEN);
			manage_get_iptableCmd(rule, command);
			sys_return = system(command); 
			sys_ret = WEXITSTATUS(sys_return);
			if(0 != sys_ret) {
				ret = AC_MANAGE_CONFIG_EXEC_FAIL;
				goto error;
			}
		}

		for(i = 0, rule = rule_list[FW_DNAT].list; i <  rule_list[FW_DNAT].num && rule; i++, rule = rule->next) {
			memset(command, 0, CMD_LEN);
			manage_get_iptableCmd(rule, command);
			sys_return = system(command);	  
			sys_ret = WEXITSTATUS(sys_return);
			if(0 != sys_ret) {
				ret = AC_MANAGE_CONFIG_EXEC_FAIL;
				goto error;
			}			
		}

		for(i = 0, rule = rule_list[FW_WALL].list; i <  rule_list[FW_WALL].num && rule; i++, rule = rule->next) {
			memset(command,0,CMD_LEN);
			manage_get_iptableCmd(rule, command);
			sys_return = system(command);
			sys_ret = WEXITSTATUS(sys_return);
			if(0 != sys_ret) {
				ret = AC_MANAGE_CONFIG_EXEC_FAIL;
				goto error;
			}		
		}

		/* for input chain */
		for(i = 0, rule = rule_list[FW_INPUT].list; i <  rule_list[FW_INPUT].num && rule; i++, rule = rule->next) {
			memset(command,0,CMD_LEN);
			manage_get_iptableCmd(rule, command);
			sys_return = system(command);
			sys_ret = WEXITSTATUS(sys_return);
			if(0 != sys_ret) {
				ret = AC_MANAGE_CONFIG_EXEC_FAIL;
				goto error;
			}		
		}

	//TRAFFIC 相关的控制//这里并不会去修改Tc相关链中的规则,有可能会修改filter mangle中对tc相关链的引用.
		system( IPTABLES_PATH"iptables -t mangle -N "TC_MANGLE_CHAIN" >/dev/null 2>&1;"\
				IPTABLES_PATH"iptables -t mangle -D PREROUTING -j "TC_MANGLE_CHAIN" >/dev/null 2>&1;"\
				IPTABLES_PATH"iptables -t mangle -A PREROUTING -j "TC_MANGLE_CHAIN" >/dev/null 2>&1;"\
				IPTABLES_PATH"iptables -t filter -N "TC_FILTER_CHAIN" >/dev/null 2>&1;"\
				IPTABLES_PATH"iptables -t filter -D "TC_FILTER_CHAIN" -j RETURN>/dev/null 2>&1;"\
				IPTABLES_PATH"iptables -t filter -A "TC_FILTER_CHAIN" -j RETURN>/dev/null 2>&1;" );
		//Set default rules
	//先将default设置为ACCEPT,防火墙，snat，dnat都需要设置
		system( IPTABLES_PATH"iptables -I "FW_FILTER_CHAIN" -j "TC_FILTER_CHAIN" >/dev/null 2>&1" );
		system( IPTABLES_PATH"iptables -A "FW_FILTER_CHAIN" -j ACCEPT" );
		system( IPTABLES_PATH"iptables -t nat -A "FW_SNAT_CHAIN" -j ACCEPT" );
		system( IPTABLES_PATH"iptables -t nat -A "FW_DNAT_CHAIN" -j ACCEPT" );
		system( IPTABLES_PATH"iptables -A "FW_INPUT_CHAIN" -j RETURN" );

		//Save rule config for next boot	   
		sys_ret = fwServiceSaveConf();
		if(0 != sys_ret) {
			ret = AC_MANAGE_CONFIG_SAVE_FAIL;
			goto error;
		}		

	error:
		DORULE_ETHERNET_ON;
		MANAGE_FREE(command);
	} else {
		if(fwServiceStop()) {
			ret = AC_MANAGE_SERVICE_STOP_FAIL;
		} 
	}

	if(AC_MANAGE_SUCCESS == ret) {
		firewall_service_status = status;
	}
	
	return ret;	
}


void
manage_free_firewall_rule(fwRule **rule) {
	if( NULL == rule ){
		return ;
	}else if( NULL == *rule){
		return;
	}
	
	fwRule *temp = *rule;

	MANAGE_FREE(temp->name);
	MANAGE_FREE(temp->comment);
	MANAGE_FREE(temp->ineth);
	MANAGE_FREE(temp->outeth);
	MANAGE_FREE(temp->srcadd);
	MANAGE_FREE(temp->dstadd);
	MANAGE_FREE(temp->sport);
	MANAGE_FREE(temp->dport);
	MANAGE_FREE(temp->connlimit);
	MANAGE_FREE(temp->tcpmss_var);
	MANAGE_FREE(temp->pkg_state);
	MANAGE_FREE(temp->string_filter);
	MANAGE_FREE(temp->natipadd);
	MANAGE_FREE(temp->natport);

	MANAGE_FREE(temp);
	*rule = NULL;
	
	return ;
}

int
manage_add_firewall_rule(fwRule *rule) {
	int i;
	fwRule *new_rule = NULL, *node, *prior;
	
	if(NULL == rule || !((u_long)rule->type < rule_type_num)) {
		//syslog(LOG_WARNING, "rule->type = %d, rule_type_num = %d\n", rule->type, rule_type_num);
		syslog(LOG_WARNING, "rule is NULL or rule->type < rule_type_num \n");
		return AC_MANAGE_INPUT_TYPE_ERROR;
	}	

	switch(rule->type) {
		case FW_WALL:
			if(manage_ifname_is_legal_input(rule->outeth)) {
				return AC_MANAGE_INPUT_TYPE_ERROR;
			}
			break;
		case FW_DNAT:
		case FW_INPUT:
			if(manage_ifname_is_legal_input(rule->ineth)) {
				return AC_MANAGE_INPUT_TYPE_ERROR;
			}
			break;
			
		case FW_SNAT:
			if(manage_ifname_is_legal_input(rule->outeth)) {
				return AC_MANAGE_INPUT_TYPE_ERROR;
			}
			break;
			
		default:
			return AC_MANAGE_INPUT_TYPE_ERROR;
	}

	for(i = 0, node = rule_list[rule->type].list, prior = node;
		i < rule_list[rule->type].num && node;
		i++, prior = node, node = node->next) {
		if(node->id == rule->id) {
			return AC_MANAGE_CONFIG_EXIST;
		} else if(node->id > rule->id){
			break;
		}
	}
	
	new_rule = _firewall_rule_copy(rule);
	if(NULL == new_rule) {
		return AC_MANAGE_MALLOC_ERROR;
	}

	if(prior == node) {
		rule_list[rule->type].list = new_rule;
	} else {
		prior->next = new_rule;
	}
	new_rule->next = node;
	
	rule_list[rule->type].num++;
	
	return AC_MANAGE_SUCCESS;
}

int
manage_modify_firewall_rule(fwRule *rule) {
	int i;
	fwRule *new_rule = NULL, *node, *prior;

	if(NULL == rule || !((u_long)rule->type < rule_type_num)) {
		//syslog(LOG_WARNING, "rule->type = %d, rule_type_num = %d\n", rule->type, rule_type_num);
		syslog(LOG_WARNING, "rule->type = %d or rule->type < rule_type_num\n");
		return AC_MANAGE_INPUT_TYPE_ERROR;
	}

	switch(rule->type) {
		case FW_WALL:
			if(manage_ifname_is_legal_input(rule->outeth)) {
				return AC_MANAGE_INPUT_TYPE_ERROR;
			}
			break;
		case FW_DNAT:
		case FW_INPUT:
			if(manage_ifname_is_legal_input(rule->ineth)) {
				return AC_MANAGE_INPUT_TYPE_ERROR;
			}
			break;
			
		case FW_SNAT:
			if(manage_ifname_is_legal_input(rule->outeth)) {
				return AC_MANAGE_INPUT_TYPE_ERROR;
			}
			break;
			
		default:
			return AC_MANAGE_INPUT_TYPE_ERROR;
	}
	
	for(i = 0, node = rule_list[rule->type].list, prior = node;
		i < rule_list[rule->type].num && node;
		i++, prior = node, node = node->next) {
		if(node->id == rule->id) {
			new_rule = _firewall_rule_copy(rule);
			if(NULL == new_rule) {
				return AC_MANAGE_MALLOC_ERROR;
			}

			 if(prior == node) {
				 rule_list[rule->type].list = new_rule;
			} else {
				prior->next = new_rule;
			}
			new_rule->next = node->next;

			manage_free_firewall_rule(&node);			
			return AC_MANAGE_SUCCESS;
		} else if(node->id > rule->id){
			return AC_MANAGE_CONFIG_NONEXIST;
		}
	}
		
	return AC_MANAGE_CONFIG_NONEXIST;
}

int
manage_chanage_firewall_rule_index(u_long new_index, u_long rule_type, u_long index) {
	int i;
	fwRule *node, *prior;
	fwRule *new_node, *new_prior;

	if(!index || !new_index || index == new_index|| !(rule_type < rule_type_num)) {
		syslog(LOG_INFO, "index = %d, new_index = %d, rule_type = %d\n", index, new_index, rule_type);
		return AC_MANAGE_INPUT_TYPE_ERROR;
	}	

	/* find the new index place */
	for(i = 0, new_node = rule_list[rule_type].list, new_prior = new_node;
		i < rule_list[rule_type].num && new_node;
		i++, new_prior = new_node, new_node = new_node->next) {
		if(new_node->id >= new_index) {
			break;	
		}
	}

	syslog(LOG_DEBUG, "new_node->id = %d, new_index = %d\n", new_node ? new_node->id : 0, new_index);

	for(i = 0, node = rule_list[rule_type].list, prior = node;
		i < rule_list[rule_type].num && node;
		i++, prior = node, node = node->next) {
		if(node->id == index) {
			
			fwRule *temp_rule = NULL;

			if(new_node && new_node->id == new_index) {
				if(new_node->next == node) {		/** the next point of new_node is node **/
					syslog(LOG_DEBUG, "the next point of new_node(%d) is node(%d)\n",
										new_node->id, node->id);

					new_node->next = node->next;
					node->next = new_node;

					if(new_prior == new_node) {
						rule_list[rule_type].list= node;
					} else {
						new_prior->next = node;
					}
				} else if(node->next == new_node) {		/** the next point of node is new_node **/
					syslog(LOG_DEBUG, "the next point of node(%d) is new_node(%d)\n",
										node->id, new_node->id);

					node->next = new_node->next;
					new_node->next = node;
					
					if(prior == node) {
						rule_list[rule_type].list = new_node;
					} else {
						prior->next = new_node;
					}
				} else {
					syslog(LOG_DEBUG, "replace node(%d) and new_node(%d)\n",
										node->id, new_node->id);
					
					temp_rule = node->next;
					node->next = new_node->next;
					new_node->next = temp_rule;

					/** change the next point of prior point of node **/
					if(prior == node) {
						rule_list[rule_type].list = new_node;
					} else {
						prior->next = new_node;
					}

					/** change the next point of prior point of new_node **/
					if(new_prior == new_node) {
						rule_list[rule_type].list= node;
					} else {
						new_prior->next = node;
					}
				}
				/** replace node and new node index **/
				new_node->id = index;
				node->id = new_index;
			}else {
				if(node->next == new_node) {
					syslog(LOG_DEBUG, "the next point of node(%d) is new_node(%d), so no need move node\n",
										node->id, new_node ? new_node->id : 0);
				} else if(new_node && new_node->next == node) {
					syslog(LOG_DEBUG, "the next point of new_node(%d) is node(%d), need move node before new_node, new_index = %d\n",
										new_node->id, node->id,  new_index);

					new_node->next = node->next;
					node->next = new_node;

					if(new_prior == new_node) {
						rule_list[rule_type].list= node;
					} else {
						new_prior->next = node;
					}
				} else {
					syslog(LOG_DEBUG, "need move node(%d) before new_node(%d), new_index = %d\n",
										node->id, new_node ? new_node->id : 0, new_index);
					
					if(prior == node) {
						rule_list[rule_type].list = node->next;
					} else {
						prior->next = node->next;
					}	
					
					node->next = new_node;
					
					if(new_prior == new_node) {
						rule_list[rule_type].list= node;
					} else {
						new_prior->next = node;
					}
				}

				/** chanage node index to new_index **/
				node->id = new_index;
			}
			
			return AC_MANAGE_SUCCESS;
		} else if(node->id > index){
			return AC_MANAGE_CONFIG_NONEXIST;
		}
	}
	
	return AC_MANAGE_CONFIG_NONEXIST;
}

int
manage_del_firewall_rule(u_long rule_type, u_long rule_id) {
	int i;
	fwRule *node, *prior;

	if(!rule_id || !(rule_type < rule_type_num)) {
		return AC_MANAGE_INPUT_TYPE_ERROR;
	}	

	for(i = 0, node = rule_list[rule_type].list, prior = node;
		i < rule_list[rule_type].num && node;
		i++, prior = node, node = node->next) {
		if(node->id == rule_id) {
			if(prior == node) {
				 rule_list[rule_type].list = node->next;
			} else {
				prior->next = node->next;
			}
			manage_free_firewall_rule(&node);
			rule_list[rule_type].num--;
			
			return AC_MANAGE_SUCCESS;
		} else if(node->id > rule_id){
			return AC_MANAGE_CONFIG_NONEXIST;
		}
	}
	
	return AC_MANAGE_CONFIG_NONEXIST;
}

int
manage_modify_nat_udp_timeout(unsigned int timeout) {
#define CMDFORMAT "echo %u > /proc/sys/net/netfilter/nf_conntrack_udp_timeout_nat"
	char command[256];
	int ret, sret;


	if (timeout > 1800 || timeout < 30) {
		return AC_MANAGE_INPUT_TYPE_ERROR;
	}

	memset(command, 0, sizeof(command));
	snprintf(command, sizeof(command), CMDFORMAT, timeout);
	
	sret = system(command);
	ret = WEXITSTATUS(sret);
	if (ret) {
		manage_log(LOG_WARNING, "system %s failed\n", command);
		return AC_MANAGE_CONFIG_EXEC_FAIL;
	}

	nat_udp_timeout = timeout;
	return AC_MANAGE_SUCCESS;
}

unsigned int
manage_show_nat_udp_timeout(void) {
	return (nat_udp_timeout < NAT_UDP_TIMEOUT_MIN ||
			nat_udp_timeout == NAT_UDP_TIMEOUT_DEFAULT ||
			nat_udp_timeout > NAT_UDP_TIMEOUT_MAX) ? 0 : nat_udp_timeout;
}

int
manage_show_firewall_service(void) {
	return firewall_service_status;
}

int
manage_show_firewall_rule(fwRule **rule_array, u_long *rule_num) {
	int i, j;
	u_long total_num = 0, rule_index = 0;
	fwRule *temp_array = NULL;
	
	if(NULL == rule_array || NULL == rule_num) {
		return AC_MANAGE_INPUT_TYPE_ERROR;
	}

	*rule_array = NULL;
	*rule_num = 0;


	for(i = 0; i < rule_type_num; i++) {
		total_num += rule_list[i].num;
	}

	temp_array = (fwRule *)calloc(total_num, sizeof(fwRule));
	if(NULL == temp_array) {
		syslog(LOG_ERR, "manage_show_firewall_rule: malloc rule temp_array fail\n");
		return AC_MANAGE_MALLOC_ERROR;
	}

	for(i = 0; i < rule_type_num; i++) {
		fwRule *node;
		syslog(LOG_DEBUG, "manage_show_firewall_rule: %s have %d rule\n", rule_list[i].name, rule_list[i].num);
		for(j = 0, node = rule_list[i].list; j < rule_list[i].num && node; j++, node = node->next) {
			_rule_copy(&temp_array[rule_index++], node);
		}
	}

	*rule_array = temp_array;
	*rule_num = total_num;

	return AC_MANAGE_SUCCESS;
}

int
manage_config_strict_access_level(int level)
{
	syslog(LOG_DEBUG, "strict_access_level = %d, level = %d\n", strict_access_level, level);
	if((level && strict_access_level)
		|| (!level && !strict_access_level)) {
		return AC_MANAGE_SUCCESS;
	}
	
	int ret =  AC_MANAGE_SUCCESS;
	int sys_return, sys_ret;
	char command[256] = "";
	
	snprintf(command, sizeof(command)-1, 
		"sudo /usr/bin/strict_access.sh %s >/dev/null 2>&1", level?"start":"stop");
	sys_return = system(command); 
	sys_ret = WEXITSTATUS(sys_return);
	if(0 != sys_ret) {
		ret = AC_MANAGE_CONFIG_EXEC_FAIL;
	}
	
	if(AC_MANAGE_SUCCESS == ret) {
		strict_access_level = level;
	}
	
	return ret;	
}

int
manage_show_strict_access_level(void)
{
	return strict_access_level;
}

