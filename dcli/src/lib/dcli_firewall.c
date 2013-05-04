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
* dcli_firewall.c
*
* MODIFY:
*		by <chensheng@autelan.com> on 2010-3-24
*
* CREATOR:
*		by <chensheng@autelan.com> on 2010-3-24
*
* DESCRIPTION:
*		CLI definition for firewall module.
*
* DATE:
*		2010-3-24 11:15:30
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.18 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include <zebra.h>
#include <dbus/dbus.h>
#include <stdlib.h>
#include <sysdef/npd_sysdef.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "vty.h"
#include "command.h"

#include "ac_manage_def.h"

#include "ws_public.h"
#include "ws_firewall.h"
#include "ac_manage_firewall_interface.h"

#include "dcli_main.h"

#include "dcli_firewall.h"


#define FIREWALL_MAX_RULE_NUM			256
#define FW_MAX_PKG_STATE_LEN			64
#define FW_MAX_STR_FILTER_LEN			32


#define FIREWALL_PACKAGE_STATE		"(any|new|established|related|invalid"	\
										"|new,established|new,related|new,invalid|established,related|established,invalid|related,invalid"	\
										"|new,established,related|new,established,invalid|new,related,invalid|established,related,invalid"	\
										"|new,established,related,invalid)"

#define FIREWALL_PACKAGE_STATE_DESC	"any state\n"	\
										"new state\n"		\
										"established state\n"	\
										"related state\n"	\
										"invalid state\n"	\
										"new established state\n"	\
										"new related state\n"	\
										"new invalid state\n"	\
										"established related state\n"	\
										"established invalid state\n"	\
										"related invalid state\n"		\
										"new established related state\n"	\
										"new established invalid state\n"	\
										"new related invalid state\n"		\
										"established related invalid state\n"	\
										"new established related invalid state\n"


struct cmd_node firewall_node = 
{
	FIREWALL_NODE,
	"%s(config-firewall)# "
};

extern int is_distributed;

static int
firewall_index_is_legal_input(const char *str_index, u_long *slot_id, u_long *index) {
	char *endp;

	if(!str_index || !slot_id || !index )
		return -1;

	if (strchr(str_index, '-')) {
		if(2 != sscanf(str_index, "%d-%d", slot_id, index)) {
			return -1;
		}
	} else {
		*index = strtol(str_index, &endp, 10);
		if (endp && *endp) {
			return -1;
		}
		*slot_id = HostSlotId;
	}

	if (HostSlotId != *slot_id && 
		(!*slot_id || *slot_id > MAX_SLOT)) {
		return -1;
	}

	if (!*index || *index > FIREWALL_MAX_RULE_NUM) {
		return -1;
	}
	
	return 0;
}

static int 
firewall_ipaddr_is_legal_input(const char *str, u_long *addr_type) {
	const char *tempstr = NULL;
	char *endptr = NULL;
	u_char addr[8] = { 0 };
	int i = 0;

	if (NULL == str || '\0' == str[0] || '0' == str[0]) {
		return -1;
	}
	
	if(0 == strcmp(str, "any")) {
		*addr_type = FW_MASQUERADE;
		return 0;
	} 
	
	for(i = 0, tempstr = str; i < 8; i++) {
		u_long ip = strtoul(tempstr, &endptr, 10);
		if(((0 == i || 4 == i) && 0 == ip) || ip > 255) {
			return -1;
		}else if(endptr && ((ip < 10 && (endptr - tempstr) > 1)
				|| (ip < 100 && (endptr - tempstr) > 2) || (ip < 256 && (endptr - tempstr) > 3))) {
			return -1;
		}

		addr[i] = (u_char)ip;

		if(3 == i) {
			if(NULL == endptr || '\0' == endptr[0]) {
				if(0 == addr[i]) {
					return -1;
				} else {
					*addr_type = FW_IPSINGLE;
					return 0;
				}
			} else if('/' == endptr[0]) {
				*addr_type = FW_IPMASK;
			} else if('-' == endptr[0]) {
				if(0 == addr[i]) {
					return -1;
				}
				*addr_type = FW_IPRANG;
			} else {
				return -1;
			}
		}

		if(7 == i && FW_IPRANG == *addr_type) {
			if(0 == addr[i]) {
				return -1;
			}
		}
		
		tempstr = endptr + 1;
	}

	if(FW_IPMASK== *addr_type) {
		u_long *mask, flag = 0;
		mask = (u_long *)(addr + 4);

		/* Masklen must shorter than 30 */
		if ((*mask) & 0x3) {
			return -1;
		}
		
		for(i = 0; i < 32; i++, (*mask) /= 2) {
			if((*mask) % 2) {
				flag = 1;
			} else if(flag) {
				return -1;
			}
		}
		
		return 0;
	} else if(FW_IPRANG == *addr_type) {
		if(memcmp(addr + 4, addr, 4) > 0) {
			return 0;
		} else {
			return -1;
		}
	}
	
	return -1;
}

static int 
firewall_port_is_legal_input(const char *str, u_long *port_type) {
	const char *tempstr = NULL;
	char *startptr = NULL, *endptr = NULL;
	u_long port[2];
	int i;

	if (NULL == str || '\0' == str[0] || '0' == str[0]) {
		return -1;
	}
	
	if(0 == strcmp(str, "any")) {
		*port_type = 0;	// any port
		return 0;
	} 

	for(tempstr = str, i = 0; i < 2; i++) {
		port[i] = strtoul(tempstr, &endptr, 10);
		if(port[i] > 65535) {
			return -1;
		} else if(endptr && ((port[i] < 65536 && (endptr - tempstr) > 5)
				|| (port[i] < 10000 && (endptr - tempstr) > 4) || (port[i] < 1000 && (endptr - tempstr) > 3)
				|| (port[i] < 100 && (endptr - tempstr) > 2) || (port[i] < 10 && (endptr - tempstr) > 1))){
			return -1;
		}
		
		if(NULL == endptr || '\0' == endptr[0]) {
			if(0 == i) {
				*port_type = FW_PTSINGLE;
				return 0;
			} else if(1 == i) {
				if(port[i] > port[0]) {
					*port_type = FW_PTRANG;
					return 0;
				}
			}
		} else if(':' != endptr[0]) {
			return -1;
		}

		tempstr = endptr + 1;
	}

	return -1;
}


static int 
firewall_natport_is_legal_input(const char *str, u_long *port_type) {
	const char *tempstr = NULL;
	char *startptr = NULL, *endptr = NULL;
	u_long port[2];
	int i;

	if (NULL == str || '\0' == str[0] || '0' == str[0]) {
		return -1;
	}
	
	if(0 == strcmp(str, "any")) {
		*port_type = 0;	// any port
		return 0;
	} 

	for(tempstr = str, i = 0; i < 2; i++) {
		port[i] = strtoul(tempstr, &endptr, 10);
		if(port[i] > 65535) {
			return -1;
		} else if(endptr && ((port[i] < 65536 && (endptr - tempstr) > 5)
				|| (port[i] < 10000 && (endptr - tempstr) > 4) || (port[i] < 1000 && (endptr - tempstr) > 3)
				|| (port[i] < 100 && (endptr - tempstr) > 2) || (port[i] < 10 && (endptr - tempstr) > 1))){
			return -1;
		}
		
		if(NULL == endptr || '\0' == endptr[0]) {
			if(0 == i) {
				*port_type = FW_PTSINGLE;
				return 0;
			} else if(1 == i) {
				if(port[i] > port[0]) {
					*port_type = FW_PTRANG;
					return 0;
				}
			}
		} else if('-' != endptr[0]) {
			return -1;
		}

		tempstr = endptr + 1;
	}

	return -1;
}


static int 
firewall_state_is_legal_input(const char *str) {
	if (NULL == str || strlen(str) > (FW_MAX_PKG_STATE_LEN - 1)) {
		return -1;
	}	
	
	if(strcmp(str, "any")
		&&strcmp(str, "new")
		&& strcmp(str, "established")
		&& strcmp(str, "related")
		&& strcmp(str, "invalid")
		&& strcmp(str, "new,established")
		&& strcmp(str, "new,related")
		&& strcmp(str, "new,invalid")
		&& strcmp(str, "established,related")
		&& strcmp(str, "established,invalid")
		&& strcmp(str, "related,invalid")
		&& strcmp(str, "new,established,related")
		&& strcmp(str, "new,established,invalid")
		&& strcmp(str, "new,related,invalid")
		&& strcmp(str, "established,related,invalid")
		&& strcmp(str, "new,established,related,invalid")) {
		return -1;
	}
		
	return 0;
}

static int 
firewall_filterstring_is_legal_input(const char *str) {
	if (NULL == str || strlen(str) > (FW_MAX_STR_FILTER_LEN - 1)) {
		return -1;
	}	
	return 0;
}


static void 
firewall_show_rule_entry(fwRule *rule, struct vty *vty) {

	const char *strRuleType = NULL, *strIpType = NULL, *strIpAddr = NULL, 
				*strProtocol = NULL, *strPortType = NULL, *strAct = NULL;

	/* type & index */
	switch(rule->type) {
		case FW_WALL:
			strRuleType = "Firewall";
			break;
		case FW_DNAT:
			strRuleType = "Dnat";
			break;
		case FW_SNAT:
			strRuleType = "Snat";
			break;
		case FW_INPUT:
			strRuleType = "Input";
			break;

		default:
			return ;
	}
	vty_out(vty, "%12s   Information\n", strRuleType);
	
	vty_out(vty, "%-18s : %d\n", "index", rule->id);
		
	/* valid */
	vty_out(vty, "%-18s : %s\n", "valid", rule->enable ? "enable" : "disable");
	
	/* in-if */
	if(FW_WALL == rule->type || FW_DNAT == rule->type || FW_INPUT == rule->type) {
		vty_out(vty, "%-18s : %s\n", "in-if", rule->ineth);
	}
	
	/* out-if */
	if(FW_WALL == rule->type || FW_SNAT == rule->type) {
		vty_out(vty, "%-18s : %s\n", "out-if", rule->outeth);
	}	
	
	/* src-ip */
	switch(rule->srctype) {
		case FW_MASQUERADE:
			strIpType = "any";
			break;
		case FW_IPSINGLE:
			strIpType = "single";
			break;
		case FW_IPMASK:
			strIpType = "mask";
			break;
		case FW_IPRANG:
			strIpType = "range";
			break;

		default:
			strIpType = NULL;
			break;
	}
	
	if(FW_MASQUERADE== rule->srctype) {
		vty_out(vty, "%-18s : %s\n", "src-ip", "any");
	} else if(FW_IPSINGLE == rule->srctype || FW_IPMASK== rule->srctype || FW_IPRANG== rule->srctype) {
		vty_out(vty, "%-18s : %s %s\n", "src-ip", strIpType, rule->srcadd);
	}	
	
	/* dst-ip */
	switch(rule->dsttype) {
		case FW_MASQUERADE:
			strIpType = "any";
			break;
		case FW_IPSINGLE:
			strIpType = "single";
			break;
		case FW_IPMASK:
			strIpType = "mask";
			break;
		case FW_IPRANG:
			strIpType = "range";
			break;

		default:
			strIpType = NULL;
			break;
	}

	if(FW_MASQUERADE== rule->dsttype) {
		vty_out(vty, "%-18s : %s\n", "dst-ip", "any");
	} else if(FW_IPSINGLE == rule->dsttype || FW_IPMASK== rule->dsttype || FW_IPRANG== rule->dsttype) {
		vty_out(vty, "%-18s : %s %s\n", "dst-ip", strIpType, rule->dstadd);
	}	

	/* protocol */
	switch(rule->protocl) {
		case FW_PTCP:
			strProtocol = "tcp";
			break;
		case FW_PUDP:
			strProtocol = "udp";
			break;
		case FW_PTCPANDUDP:
			strProtocol = "tcp-and-udp";
			break;
		case FW_PICMP:
			strProtocol = "icmp";
			break;

		default:
			strProtocol = "any";
			break;
	}
	vty_out(vty, "%-18s : %s\n", "protocol", strProtocol);

	/** src-port and dst-port **/
	if(FW_PTCP == rule->protocl || FW_PUDP == rule->protocl || FW_PTCPANDUDP== rule->protocl){
		/* src-port */
		switch(rule->sptype) {
			case 0:
				strPortType = "any";
				break;
			case FW_PTSINGLE:
				strPortType = "single";
				break;
			case FW_PTRANG:
				strPortType = "range";
				break;

			default:
				strPortType = NULL;
				break;
		}

		if(0 == rule->sptype) {
			vty_out(vty, "%-18s : %s\n", "src-port", "any");
		} else if(FW_PTSINGLE == rule->sptype || FW_PTRANG == rule->sptype) {
			vty_out(vty, "%-18s : %s %s\n", "src-port", strPortType, rule->sport);
		}	


		/* dst-port */
		switch(rule->dptype) {
			case 0:
				strPortType = "any";
				break;
			case FW_PTSINGLE:
				strPortType = "single";
				break;
			case FW_PTRANG:
				strPortType = "range";
				break;

			default:
				strPortType = NULL;
				break;
		}

		if(0 == rule->dptype) {
			vty_out(vty, "%-18s : %s\n", "dst-port", "any");
		} else if(FW_PTSINGLE == rule->dptype || FW_PTRANG == rule->dptype) {
			vty_out(vty, "%-18s : %s %s\n", "dst-port", strPortType, rule->dport);
		}	
	}

	/* state */
	if(FW_WALL == rule->type || FW_INPUT == rule->type) {
		vty_out(vty, "%-18s : %s\n", "state", rule->pkg_state ? rule->pkg_state : "any");
	}	

	/* filter string */
	if(FW_WALL == rule->type || FW_INPUT == rule->type) {
		vty_out(vty, "%-18s : %s\n", "filter-string", rule->string_filter ? rule->string_filter : "null");
	}	

	/* act */
	if(FW_WALL == rule->type || FW_INPUT == rule->type){
		if (rule->connlimit && rule->connlimit[0]) {
			vty_out(vty, "%-18s : %s\n", "connect limit", rule->connlimit);
		} else {
			switch(rule->act) {
				case FW_ACCEPT:
					strAct = "accept";
					break;
				case FW_DROP:
					strAct = "drop";
					break;
				case FW_REJECT:
					strAct = "reject";
					break;
				case FW_TCPMSS:
					strAct = "tcpmss";
					break;

				default:
					strAct = NULL;
					break;
			}

			if(FW_ACCEPT == rule->act || FW_DROP == rule->act || FW_REJECT == rule->act) {
				vty_out(vty, "%-18s : %s\n", "act", strAct);
			} else if(FW_TCPMSS == rule->act && FW_WALL == rule->type) {
				vty_out(vty, "%-18s : %s %s\n", "act", strAct, rule->tcpmss_var);
			}	
		}
	}

	/** nat-ip and nat-port **/
	if(FW_SNAT == rule->type || FW_DNAT == rule->type){
		/* nat-ip */
		switch(rule->natiptype) {
			case FW_MASQUERADE:
				strIpType = "any";
				break;
			case FW_IPSINGLE:
				strIpType = "single";
				break;
			case FW_IPMASK:
				strIpType = "mask";
				break;
			case FW_IPRANG:
				strIpType = "range";
				break;

			default:
				strIpType = NULL;
				break;
		}

		if(FW_MASQUERADE== rule->natiptype) {
			vty_out(vty, "%-18s : %s\n", "nat-ip", "any");
		} else if(FW_IPSINGLE == rule->natiptype || FW_IPMASK== rule->natiptype || FW_IPRANG== rule->natiptype) {
			vty_out(vty, "%-18s : %s %s\n", "nat-ip", strIpType, rule->natipadd);
		}

		
		/* nat-port */
		if(FW_PTCP == rule->protocl || FW_PUDP == rule->protocl || FW_PTCPANDUDP== rule->protocl) {
			switch(rule->natpttype) {
				case 0:
					strPortType = "any";
					break;
				case FW_PTSINGLE:
					strPortType = "single";
					break;
				case FW_PTRANG:
					strPortType = "range";
					break;

				default:
					strPortType = NULL;
					break;
			}

			if(0 == rule->natpttype) {
				vty_out(vty, "%-18s : %s\n", "nat-port", "any");
			} else if(FW_PTSINGLE == rule->natpttype || FW_PTRANG == rule->natpttype) {
				vty_out(vty, "%-18s : %s %s\n", "nat-port", strPortType, rule->natport);
			}	
		}	
	}

	return ;
}


static void
dcli_show_firewall_rule_info(DBusConnection *connection, u_int slot_id, struct vty *vty) {
	if(NULL == connection || NULL == vty) {
		return;
	}
	
	int ret;
	fwRule *rule_array = NULL;
	u_long rule_num = 0;
	ret = ac_manage_show_firewall_rule(connection, NULL, NULL, &rule_array, &rule_num);
	if(AC_MANAGE_SUCCESS == ret && rule_array && rule_num) {
		vty_out(vty, "----------------------------------------------------------------------\n");

		int i = 0;
		for(i = 0; i < rule_num; i++) {
			firewall_show_rule_entry(&rule_array[i], vty);
			vty_out(vty, "----------------------------------------------------------------------\n");
		}

		firewall_free_array(&rule_array, rule_num);
	}

    return;
}

static void
firewall_show_rule_entry_running_config(u_long slot_id, fwRule *rule, struct vty *vty) {
	if(NULL == rule) {
		return;
	}

	char command[512];
	char index[16];

	char *str_protocl = NULL;
	char *str_sport = NULL;
	char *str_dport = NULL;

	memset(index, 0, sizeof(index));
	if (slot_id) {
		snprintf(index, sizeof(index), "%u-%u", slot_id, rule->id);
	} else {
		snprintf(index, sizeof(index), "%u", rule->id);
	}
	
	if (rule->connlimit && rule->connlimit[0]) {
		if ((FW_WALL != rule->type && FW_INPUT != rule->type) || !rule->srcadd) {
			return;
		}

		memset(command, 0, sizeof(command));
		snprintf(command, sizeof(command), " add %s %s %s connlimit %s %s",
				rule->type == FW_WALL ? "firewall" : "input", index,
				rule->srcadd, rule->connlimit, rule->enable ? "enable" : "disable");
		goto out;
	}

	
	switch(rule->protocl) {
		case FW_PTCP:
			str_protocl = "tcp";
			str_sport = rule->sport ? rule->sport : "any";
			str_dport = rule->dport ? rule->dport : "any";
			break;
		case FW_PUDP:
			str_protocl = "udp";
			str_sport = rule->sport ? rule->sport : "any";
			str_dport = rule->dport ? rule->dport : "any";
			break;
		case FW_PTCPANDUDP:
			str_protocl = "tcp-and-udp";
			str_sport = rule->sport ? rule->sport : "any";
			str_dport = rule->dport ? rule->dport : "any";
			break;
		case FW_PICMP:
			str_protocl = "icmp";
			break;
			
		default:
			str_protocl = "any";
			break;
	}

	char *str_act = NULL;
	char *str_tcpmss = NULL;

	char *str_natip = NULL;
	char *str_natport = NULL;
	
	if(FW_WALL == rule->type || FW_INPUT == rule->type) {
		switch(rule->act) {
			case FW_ACCEPT:
				str_act = "accept";
				break;
			case FW_DROP:
				str_act = "drop";
				break;
			case FW_REJECT:
				str_act = "reject";
				break;
			case FW_TCPMSS:
				if(FW_INPUT == rule->type|| FW_PTCP != rule->protocl || NULL == rule->tcpmss_var) {
					return; 
				}
				str_act = "tcpmss";	
				str_tcpmss = rule->tcpmss_var;
				break;

			default:
				return ;
		}
	} else if(FW_DNAT == rule->type || FW_SNAT == rule->type) {
		if(FW_DNAT == rule->type && NULL == rule->natipadd) {
			return ;
		}
		
		str_natip = rule->natipadd ? rule->natipadd : "any";
		
		switch(rule->protocl) {
			case FW_PTCP:
			case FW_PUDP:
			case FW_PTCPANDUDP:
				str_natport = rule->natport ? rule->natport : "any";
				break;

			default:
				break;
		}
	}
	
	switch(rule->type) {
		case FW_WALL:
			memset(command, 0, sizeof(command));
			snprintf(command, sizeof(command) - 1, " add firewall %s %s %s %s %s %s",
					index, rule->ineth, rule->outeth, 
					(FW_MASQUERADE == rule->srctype || NULL == rule->srcadd) ? "any" : rule->srcadd,
					(FW_MASQUERADE == rule->dsttype || NULL == rule->dstadd) ? "any" : rule->dstadd,
					str_protocl);	
			
			if(str_sport) {
				strncat(command, " ", sizeof(command) - strlen(command) - 1);
				strncat(command, str_sport, sizeof(command) - strlen(command) - 1);
			}
			if(str_dport) {
				strncat(command, " ", sizeof(command) - strlen(command) - 1);
				strncat(command, str_dport, sizeof(command) - strlen(command) - 1);
			}

			strncat(command, " ", sizeof(command) - strlen(command) - 1);
			strncat(command, rule->pkg_state ? rule->pkg_state : "any", sizeof(command) - strlen(command) - 1);
			
			strncat(command, " ", sizeof(command) - strlen(command) - 1);
			strncat(command, rule->string_filter ? rule->string_filter : "null", sizeof(command) - strlen(command) - 1);

			strncat(command, " ", sizeof(command) - strlen(command) - 1);
			strncat(command, str_act, sizeof(command) - strlen(command) - 1);

			if(str_tcpmss) {
				strncat(command, " ", sizeof(command) - strlen(command) - 1);
				strncat(command, str_tcpmss, sizeof(command) - strlen(command) - 1);
			}

			strncat(command, " ", sizeof(command) - strlen(command) - 1);
			strncat(command, rule->enable ? "enable" : "disable", sizeof(command) - strlen(command) - 1);
			
			break;
		case FW_DNAT:
			memset(command, 0, sizeof(command));
			snprintf(command, sizeof(command) - 1, " add dnat %s %s %s %s %s",
					index, rule->ineth, 
					(FW_MASQUERADE == rule->srctype || NULL == rule->srcadd) ? "any" : rule->srcadd,
					(FW_MASQUERADE == rule->dsttype || NULL == rule->dstadd) ? "any" : rule->dstadd,
					str_protocl);

			if(str_sport) {
				strncat(command, " ", sizeof(command) - strlen(command) - 1);
				strncat(command, str_sport, sizeof(command) - strlen(command) - 1);
			}
			if(str_dport) {
				strncat(command, " ", sizeof(command) - strlen(command) - 1);
				strncat(command, str_dport, sizeof(command) - strlen(command) - 1);
			}
			
			strncat(command, " ", sizeof(command) - strlen(command) - 1);
			strncat(command, str_natip, sizeof(command) - strlen(command) - 1);

			if(str_natport) {
				strncat(command, " ", sizeof(command) - strlen(command) - 1);
				strncat(command, str_natport, sizeof(command) - strlen(command) - 1);
			}

			strncat(command, " ", sizeof(command) - strlen(command) - 1);
			strncat(command, rule->enable ? "enable" : "disable", sizeof(command) - strlen(command) - 1);
			
			break;
		case FW_SNAT:
			memset(command, 0, sizeof(command));
			snprintf(command, sizeof(command) - 1, " add snat %s %s %s %s %s",
					index, rule->outeth, 
					(FW_MASQUERADE == rule->srctype || NULL == rule->srcadd) ? "any" : rule->srcadd,
					(FW_MASQUERADE == rule->dsttype || NULL == rule->dstadd) ? "any" : rule->dstadd,
					str_protocl);

			if(str_sport) {
				strncat(command, " ", sizeof(command) - strlen(command) - 1);
				strncat(command, str_sport, sizeof(command) - strlen(command) - 1);
			}
			if(str_dport) {
				strncat(command, " ", sizeof(command) - strlen(command) - 1);
				strncat(command, str_dport, sizeof(command) - strlen(command) - 1);
			}
			
			strncat(command, " ", sizeof(command) - strlen(command) - 1);
			strncat(command, str_natip, sizeof(command) - strlen(command) - 1);

			if(str_natport) {
				strncat(command, " ", sizeof(command) - strlen(command) - 1);
				strncat(command, str_natport, sizeof(command) - strlen(command) - 1);
			}

			strncat(command, " ", sizeof(command) - strlen(command) - 1);
			strncat(command, rule->enable ? "enable" : "disable", sizeof(command) - strlen(command) - 1);

			break;
		case FW_INPUT:
			memset(command, 0, sizeof(command));
			snprintf(command, sizeof(command) - 1, " add input %s %s %s %s %s",
					index, rule->ineth, 
					(FW_MASQUERADE == rule->srctype || NULL == rule->srcadd) ? "any" : rule->srcadd,
					(FW_MASQUERADE == rule->dsttype || NULL == rule->dstadd) ? "any" : rule->dstadd,
					str_protocl);

			if(str_sport) {
				strncat(command, " ", sizeof(command) - strlen(command) - 1);
				strncat(command, str_sport, sizeof(command) - strlen(command) - 1);
			}
			if(str_dport) {
				strncat(command, " ", sizeof(command) - strlen(command) - 1);
				strncat(command, str_dport, sizeof(command) - strlen(command) - 1);
			}

			strncat(command, " ", sizeof(command) - strlen(command) - 1);
			strncat(command, rule->pkg_state ? rule->pkg_state : "any", sizeof(command) - strlen(command) - 1);
			
			strncat(command, " ", sizeof(command) - strlen(command) - 1);
			strncat(command, rule->string_filter ? rule->string_filter : "null", sizeof(command) - strlen(command) - 1);

			strncat(command, " ", sizeof(command) - strlen(command) - 1);
			strncat(command, str_act, sizeof(command) - strlen(command) - 1);

			strncat(command, " ", sizeof(command) - strlen(command) - 1);
			strncat(command, rule->enable ? "enable" : "disable", sizeof(command) - strlen(command) - 1);

			break;

		default:
			return ;
	}


out:	
	if(vty) {
		vty_out(vty, "%s\n", command);
	} else {
		vtysh_add_show_string(command);
	}

	return ;
}

static void
firewall_show_rule_running_config(struct vty *vty) {
	u_long service_status, timeout, rule_num;
	fwRule *rule_array;
	char command[256];
	int ret;

	if (is_distributed) {
		int i = 1;
		for (; i < MAX_SLOT; i++) {
			if(i == HostSlotId) {
				ret = ac_manage_show_firewall_rule(dcli_dbus_connection, &service_status, &timeout, &rule_array, &rule_num);	
			} else if(dbus_connection_dcli[i]->dcli_dbus_connection) {
				ret = ac_manage_show_firewall_rule(dbus_connection_dcli[i]->dcli_dbus_connection, NULL, NULL, &rule_array, &rule_num);	
			}

			if(AC_MANAGE_SUCCESS == ret && rule_array && rule_num) {
				int j = 0;
				for (; j < rule_num; j++) {
					firewall_show_rule_entry_running_config(i, &rule_array[j], vty);	
				}
				firewall_free_array(&rule_array, rule_num);
			}	
		}
	} else {
		ret = ac_manage_show_firewall_rule(dcli_dbus_connection, &service_status, &timeout, &rule_array, &rule_num);
		if(AC_MANAGE_SUCCESS == ret && rule_array && rule_num) {
			int j ;
			for(j = 0; j < rule_num; j++) {
				firewall_show_rule_entry_running_config(HostSlotId, &rule_array[j], vty);	
			}
			firewall_free_array(&rule_array, rule_num);
		}	
	}

	if (timeout) {
		memset(command, 0, sizeof(command));
		snprintf(command, sizeof(command), " modify nat udp timeout %u", timeout);
		
		if(vty) {
			vty_out(vty, "%s\n", command);
		} else {
			vtysh_add_show_string(command);
		}	
	}

	if(service_status) {
		if(vty) {
			vty_out(vty, " service enable\n");
		} else {
			vtysh_add_show_string(" service enable");
		}	
	}
}

static int 
dcli_firewall_show_running_config(struct vty* vty) {
	do {
		char _tmpstr[64];
		memset(_tmpstr, 0, 64);
		sprintf(_tmpstr, BUILDING_MOUDLE, "firewall");
		vtysh_add_show_string(_tmpstr);
	} while(0);

	vtysh_add_show_string("config firewall");	

	firewall_show_rule_running_config(NULL);
		
	vtysh_add_show_string(" exit");	
	
	return CMD_SUCCESS;
}

					
DEFUN(conf_firewall_func,
	conf_firewall_cmd,
	"config firewall",
	CONFIG_STR
	"config firewall\n" 
)
{
	if(CONFIG_NODE == vty->node) {
		vty->node = FIREWALL_NODE;
		vty->index = NULL;	
	}
	else {
		vty_out(vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}


DEFUN(config_firewall_service_func,
	config_firewall_service_cmd,
	"service (enable|disable)",
	"start or stop firewall\n"
	"enable, start\n"
	"disable, stop\n"
)
{
	const char *str_status = argv[0];
	u_long status = 0;		/* disable*/
	int ret;

	if(0 == strncmp(str_status, "e", 1)) {
		status = 1;	
	} else if(strncmp(str_status, "d", 1)) {
		vty_out(vty, "input parameter error!\n");
		return CMD_WARNING;
	}

	if (is_distributed) {
		int i;
		for(i = 1; i < MAX_SLOT; i++) {
			if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 
				continue;
			
			ret = ac_manage_config_firewall_service(dbus_connection_dcli[i]->dcli_dbus_connection, status);
			switch(ret) {
				case AC_MANAGE_SUCCESS:            
					break;
				case AC_MANAGE_INPUT_TYPE_ERROR:
					vty_out(vty, "Slot(%d): input parameter error!\n", i);
					break;
				case AC_MANAGE_DBUS_ERROR:
					vty_out(vty, "Slot(%d): <error> failed get reply.\n", i);
					break;
				case AC_MANAGE_SERVICE_START_FAIL:
					vty_out(vty, "Slot(%d): firewall serivce start fail\n", i);
					break;
				case AC_MANAGE_SERVICE_STOP_FAIL:
					vty_out(vty, "Slot(%d): firewall serivce stop fail\n", i);
					break;
				case AC_MANAGE_CONFIG_FLUSH_FAIL:
					vty_out(vty, "Slot(%d): firewall rule flush fail\n", i);
					break;
				case AC_MANAGE_CONFIG_SAVE_FAIL:
					vty_out(vty, "Slot(%d): firewall rule save fail\n", i);
					break;
				case AC_MANAGE_CONFIG_EXEC_FAIL:
					vty_out(vty, "Slot(%d): firewall rule exec fail\n", i);
					break;

				default:
					vty_out(vty, "Slot(%d): unknow fail reason!\n", i);
					break;
			}
		}	
	}else {
		ret = ac_manage_config_firewall_service(dcli_dbus_connection, status);
		switch(ret) {
			case AC_MANAGE_SUCCESS:            
				return CMD_SUCCESS;
				
			case AC_MANAGE_INPUT_TYPE_ERROR:
				vty_out(vty, "input parameter error!\n");
				break;
			case AC_MANAGE_DBUS_ERROR:
				vty_out(vty, "<error> failed get reply.\n");
				break;
			case AC_MANAGE_SERVICE_START_FAIL:
				vty_out(vty, "firewall serivce start fail\n");
				break;
			case AC_MANAGE_SERVICE_STOP_FAIL:
				vty_out(vty, "firewall serivce stop fail\n");
				break;
			case AC_MANAGE_CONFIG_FLUSH_FAIL:
				vty_out(vty, "firewall rule flush fail\n");
				break;
			case AC_MANAGE_CONFIG_SAVE_FAIL:
				vty_out(vty, "firewall rule save fail\n");
				break;
			case AC_MANAGE_CONFIG_EXEC_FAIL:
				vty_out(vty, "firewall rule exec fail\n");
				break;

			default:
				vty_out(vty, "unknow fail reason!\n");
				break;
		}

		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}

DEFUN(config_firewall_rule_func,
	config_firewall_rule_cmd,
	"(add|modify) firewall INDEX (any|INIF) (any|OUTIF) (any|SOURIP) (any|DESTIP) (any|icmp) "
	FIREWALL_PACKAGE_STATE" (null|FILTERSTR) (accept|drop|reject) (enable|disable)",
	"add rule\n"
	"modify rule\n"
	"firewall, for firewall chain rule\n"
	"rule index. index format: A-B (A is slot id, B is rule index which begin with 1 and end with 256)\n"
	"input interface is any\n"
	"input interface\n"
	"output interface is any\n"
	"output interface\n"
	"source ip is any\n"
	"source ip, format: A.B.C.D or A.B.C.D/X.X.X.X or A.B.C.D-A.B.C.D\n"
	"destination ip is any\n"
	"destination ip, format: A.B.C.D or A.B.C.D/X.X.X.X or A.B.C.D-A.B.C.D\n"
	"any protocol\n"
	"icmp protocol\n"
	FIREWALL_PACKAGE_STATE_DESC	
	"filter string is null\n"
	"filter string\n"
	"accept act\n"
	"drop act\n"
	"reject act\n"
	"enable\n"
	"disable\n"
)
{	
	fwRule rule;
	DBusConnection *connection;
	
	u_long config_type = 0, slot_id, index;
	char *str_config	= (char *)argv[0];
	char *str_index		= (char *)argv[1];

	char *in_interface	= (char *)argv[2];
	char *out_interface	= (char *)argv[3];
	char inif[32], outif[32];

	u_long sip_type = FW_MASQUERADE, dip_type = FW_MASQUERADE;
	char *sour_ipaddr	= (char *)argv[4];
	char *dest_ipaddr	= (char *)argv[5];

	u_long protocl, sptype = 0, dptype = 0;
	char *str_protocl = NULL;
	char *sour_port = NULL;
	char *dest_port = NULL;

	char *pkg_state = NULL;
	char *string_filter = NULL;

	u_long act;
	char *str_act = NULL;
	char *tcpmss_var = NULL;

	u_long status = 0;
	char *str_status = NULL;

	if(0 == strncmp(str_config, "m", 1)) {
		config_type = 1;
	} else if(strncmp(str_config, "a", 1)) {
		vty_out(vty, "input config type error!\n");
		return CMD_WARNING;
	}

	if(firewall_index_is_legal_input(str_index, &slot_id, &index)) {
		vty_out(vty, "input index(%s) type error!\n", str_index);
		return CMD_WARNING;
	}

	if (slot_id == HostSlotId) {
		connection = dcli_dbus_connection;
	} else {
		connection = dbus_connection_dcli[slot_id]->dcli_dbus_connection;
	}
	
	if (NULL == connection) {
		vty_out(vty, "The slot(%d) is not connect!\n", slot_id);
		return CMD_WARNING;
	} 

    if (!strncmp(in_interface, "ve", 2)) {
		if (ve_interface_parse(in_interface, inif, sizeof(inif))) {
			vty_out(vty, "input INIF(%s) error!\n", in_interface);
			return CMD_WARNING;
		}
	} else {
		strncpy(inif, in_interface, sizeof(inif) - 1);
	}

    if (!strncmp(out_interface, "ve", 2)) {
		if (ve_interface_parse(out_interface, outif, sizeof(outif))) {
			vty_out(vty, "input OUTIF(%s) error!\n", out_interface);
			return CMD_WARNING;
		}
	} else {
		strncpy(outif, out_interface, sizeof(outif) - 1);
	}

	if(firewall_ipaddr_is_legal_input(sour_ipaddr, &sip_type)) {
		vty_out(vty, "sour ip addr(%s) input error\n", sour_ipaddr);
		return CMD_WARNING;
	}

	if(firewall_ipaddr_is_legal_input(dest_ipaddr, &dip_type)) {
		vty_out(vty, "dest ip addr(%s) input error\n", dest_ipaddr);
		return CMD_WARNING;
	}

	if(11 == argc) {
		str_protocl = (char *)argv[6];

		pkg_state = (char *)argv[7];
		string_filter = (char *)argv[8];

		str_act = (char *)argv[9];

		str_status = (char *)argv[10];
			
		if(0 == strcmp(str_protocl, "any")) {
			protocl = 0;	/** any protocol **/
		} else if(0 == strcmp(str_protocl, "icmp")) {
			protocl = FW_PICMP;
		} else {
			vty_out(vty, "protocl(%s) input error\n", str_protocl);
			return CMD_WARNING;
		}
	} else if(12 == argc) {		/** tcpmss **/	
		protocl = FW_PTCP;
		sour_port = (char *)argv[6];
		dest_port = (char *)argv[7];

		pkg_state = (char *)argv[8];
		string_filter = (char *)argv[9];

		act = FW_TCPMSS;
		tcpmss_var = (char *)argv[10];	

		str_status = (char *)argv[11];
			
		if(firewall_port_is_legal_input(sour_port, &sptype)) {
			vty_out(vty, "sour port(%s) input error\n", sour_port);
			return CMD_WARNING;
		}

		if(firewall_port_is_legal_input(dest_port, &dptype)) {
			vty_out(vty, "dest port(%s) input error\n", dest_port);
			return CMD_WARNING;
		}
	} else if(13 == argc) {
		str_protocl = (char *)argv[6];
		sour_port = (char *)argv[7];
		dest_port = (char *)argv[8];

		pkg_state = (char *)argv[9];
		string_filter = (char *)argv[10];

		str_act = (char *)argv[11];

		str_status = (char *)argv[12];
	
		if(0 == strcmp(str_protocl, "tcp")) {
			protocl = FW_PTCP;
		} else if(0 == strcmp(str_protocl, "udp")) {
			protocl = FW_PUDP;
		} else if(0 == strcmp(str_protocl, "tcp-and-udp")) {
			protocl = FW_PTCPANDUDP;
		} else {
			vty_out(vty, "protocl(%s) input error\n", str_protocl);
			return CMD_WARNING;
		}

		if(firewall_port_is_legal_input(sour_port, &sptype)) {
			vty_out(vty, "sour port(%s) input error\n", sour_port);
			return CMD_WARNING;
		}

		if(firewall_port_is_legal_input(dest_port, &dptype)) {
			vty_out(vty, "dest port(%s) input error\n", dest_port);
			return CMD_WARNING;
		}		
	} else {
		vty_out(vty, "input argc (%d) error\n", argc);
		return CMD_WARNING;
	}

	if(firewall_state_is_legal_input(pkg_state)) {
		vty_out(vty, "state(%s) input error\n", pkg_state);
		return CMD_WARNING;
	}

	if(firewall_filterstring_is_legal_input(string_filter)) {
		vty_out(vty, "filter string(%s) input error\n", string_filter);
		return CMD_WARNING;
	}

	if(str_act) {
	 	if(0 == strcmp(str_act, "accept")) {
			act = FW_ACCEPT;
		} else if(0 == strcmp(str_act, "drop")) {
			act = FW_DROP;
		} else if(0 == strcmp(str_act, "reject")) {
			act = FW_REJECT;
		} else {
			vty_out(vty, "act(%s) input error\n", str_act);
			return CMD_WARNING;
		}
	}

 	if(0 == strncmp(str_status, "e", 1)) {
		status = 1;
	} else if(strncmp(str_status, "d", 1)) {
		vty_out(vty, "status(%s) input error\n", str_status);
		return CMD_WARNING;
	} 
	
	memset(&rule, 0, sizeof(fwRule));

	rule.type = FW_WALL;
	rule.id = index;

	rule.ineth = inif;
	rule.outeth = outif;
	rule.srctype = sip_type;
	rule.srcadd = (FW_MASQUERADE == sip_type) ? NULL: sour_ipaddr;
	rule.dsttype = dip_type;
	rule.dstadd = (FW_MASQUERADE == dip_type) ? NULL: dest_ipaddr;
	
	rule.protocl = protocl;
	rule.sptype = sptype;
	rule.sport = (0 == sptype) ? NULL: sour_port;
	rule.dptype = dptype;
	rule.dport = (0 == dptype) ? NULL: dest_port;

	rule.pkg_state = strcmp(pkg_state, "any") ? pkg_state : NULL;
	rule.string_filter = strcmp(string_filter, "null") ? string_filter : NULL;

	rule.act = act;
	rule.tcpmss_var = (FW_TCPMSS == act) ? tcpmss_var : NULL;

	rule.enable = status;

	int ret;
	ret = ac_manage_config_firewall_rule(connection, &rule, config_type);
	switch(ret) {
		case AC_MANAGE_SUCCESS:            
			return CMD_SUCCESS;
		case AC_MANAGE_CONFIG_NONEXIST:
			vty_out(vty, "Slot(%d): index %d rule is not exist!\n", slot_id, index);
			break;
		case AC_MANAGE_CONFIG_EXIST:
			vty_out(vty, "Slot(%d): index %d rule is exist!\n", slot_id, index);
			break;
		case AC_MANAGE_INPUT_TYPE_ERROR:
			vty_out(vty, "Slot(%d): input parameter error!\n", slot_id);
			break;
		case AC_MANAGE_DBUS_ERROR:
			vty_out(vty, "Slot(%d): <error> failed get reply.\n", slot_id);
			break;

		default:
			vty_out(vty, "Slot(%d): unknow fail reason!\n", slot_id);
			break;
	}
	
	return CMD_WARNING;
}

ALIAS(config_firewall_rule_func,
	config_firewall_port_rule_cmd,
	"(add|modify) firewall INDEX (any|INIF) (any|OUTIF) (any|SOURIP) (any|DESTIP) (tcp|udp|tcp-and-udp) (any|SOURPORT) (any|DESTPORT) "
	FIREWALL_PACKAGE_STATE" (null|FILTERSTR) (accept|drop|reject) (enable|disable)",
	"add rule\n"
	"modify rule\n"
	"firewall, for firewall chain rule\n"
	"rule index. index format: A-B (A is slot id, B is rule index which begin with 1)\n"
	"input interface is any\n"
	"input interface\n"
	"output interface is any\n"
	"output interface\n"
	"source ip is any\n"
	"source ip, format: A.B.C.D or A.B.C.D/X.X.X.X or A.B.C.D-A.B.C.D\n"
	"destination ip is any\n"
	"destination ip, format: A.B.C.D or A.B.C.D/X.X.X.X or A.B.C.D-A.B.C.D\n"
	"tcp protocol\n"
	"udp protocol\n"
	"both tcp and udp protocol\n"
	"source port is any\n"
	"source port, format: 80 or 80:162\n"
	"destination port is any\n"
	"destination port, format: 80 or 80:162\n"
	FIREWALL_PACKAGE_STATE_DESC	
	"filter string is null\n"
	"filter string\n"
	"accept act\n"
	"drop act\n"
	"reject act\n"
	"enable\n"
	"disable\n"
)

ALIAS(config_firewall_rule_func,
	config_firewall_tcpmss_rule_cmd,
	"(add|modify) firewall INDEX (any|INIF) (any|OUTIF) (any|SOURIP) (any|DESTIP) tcp (any|SOURPORT) (any|DESTPORT) "
	FIREWALL_PACKAGE_STATE" (null|FILTERSTR) tcpmss <0-9999> (enable|disable)",
	"add rule\n"
	"modify rule\n"
	"firewall, for firewall chain rule\n"
	"rule index. index format: A-B (A is slot id, B is rule index which begin with 1)\n"
	"input interface is any\n"
	"input interface\n"
	"output interface is any\n"
	"output interface\n"
	"source ip is any\n"
	"source ip, format: A.B.C.D or A.B.C.D/X.X.X.X or A.B.C.D-A.B.C.D\n"
	"destination ip is any\n"
	"destination ip, format: A.B.C.D or A.B.C.D/X.X.X.X or A.B.C.D-A.B.C.D\n"
	"tcp protocol\n"
	"source port is any\n"
	"source port, format: 80 or 80:162\n"
	"destination port is any\n"
	"destination port, format: 80 or 80:162\n"
	FIREWALL_PACKAGE_STATE_DESC	
	"filter string is null\n"
	"filter string\n"
	"tcpmss act\n"
	"tcp mss vlaue\n"
	"enable\n"
	"disable\n"
)

DEFUN(config_input_rule_func,
	config_input_rule_cmd,
	"(add|modify) input INDEX (any|INIF) (any|SOURIP) (any|DESTIP) (any|icmp) "
	FIREWALL_PACKAGE_STATE" (null|FILTERSTR) (accept|drop|reject) (enable|disable)",
	"add rule\n"
	"modify rule\n"
	"input, for input chain rule\n"
	"rule index. index format: A-B (A is slot id, B is rule index which begin with 1 and end with 256)\n"
	"input interface is any\n"
	"input interface\n"
	"source ip is any\n"
	"source ip, format: A.B.C.D or A.B.C.D/X.X.X.X or A.B.C.D-A.B.C.D\n"
	"destination ip is any\n"
	"destination ip, format: A.B.C.D or A.B.C.D/X.X.X.X or A.B.C.D-A.B.C.D\n"
	"any protocol\n"
	"icmp protocol\n"
	FIREWALL_PACKAGE_STATE_DESC	
	"filter string is null\n"
	"filter string\n"
	"accept act\n"
	"drop act\n"
	"reject act\n"
	"enable\n"
	"disable\n"
)
{	
	fwRule rule;
	DBusConnection *connection;
	
	u_long config_type = 0, slot_id, index;
	char *str_config	= (char *)argv[0];
	char *str_index		= (char *)argv[1];

	char *in_interface	= (char *)argv[2];
	char inif[32];
	
	u_long sip_type = FW_MASQUERADE, dip_type = FW_MASQUERADE;
	char *sour_ipaddr	= (char *)argv[3];
	char *dest_ipaddr	= (char *)argv[4];

	u_long protocl, sptype = 0, dptype= 0;
	char *str_protocl = NULL;
	char *sour_port = NULL;
	char *dest_port = NULL;

	char *pkg_state = NULL;
	char *string_filter = NULL;

	u_long act;
	char *str_act = NULL;

	u_long status = 0;
	char *str_status = NULL;

	if(0 == strncmp(str_config, "m", 1)) {
		config_type = 1;
	} else if(strncmp(str_config, "a", 1)) {
		vty_out(vty, "input config type error!\n");
		return CMD_WARNING;
	}

	if(firewall_index_is_legal_input(str_index, &slot_id, &index)) {
		vty_out(vty, "input index(%s) type error!\n", str_index);
		return CMD_WARNING;
	}

	if (slot_id == HostSlotId) {
		connection = dcli_dbus_connection;
	} else {
		connection = dbus_connection_dcli[slot_id]->dcli_dbus_connection;
	}
	
	if (NULL == connection) {
		vty_out(vty, "The slot(%d) is not connect!\n", slot_id);
		return CMD_WARNING;
	} 

    if (!strncmp(in_interface, "ve", 2)) {
		if (ve_interface_parse(in_interface, inif, sizeof(inif))) {
			vty_out(vty, "input INIF(%s) error!\n", in_interface);
			return CMD_WARNING;
		}
	} else {
		strncpy(inif, in_interface, sizeof(inif) - 1);
	}

	if(firewall_ipaddr_is_legal_input(sour_ipaddr, &sip_type)) {
		vty_out(vty, "sour ip addr(%s) input error\n", sour_ipaddr);
		return CMD_WARNING;
	}

	if(firewall_ipaddr_is_legal_input(dest_ipaddr, &dip_type)) {
		vty_out(vty, "dest ip addr(%s) input error\n", dest_ipaddr);
		return CMD_WARNING;
	}

	if(10 == argc) {
		str_protocl = (char *)argv[5];

		pkg_state = (char *)argv[6];
		string_filter = (char *)argv[7];

		str_act = (char *)argv[8];

		str_status = (char *)argv[9];
			
		if(0 == strcmp(str_protocl, "any")) {
			protocl = 0;	/** any protocol **/
		} else if(0 == strcmp(str_protocl, "icmp")) {
			protocl = FW_PICMP;
		} else {
			vty_out(vty, "protocl(%s) input error\n", str_protocl);
			return CMD_WARNING;
		}
	} else if(12 == argc) {
		str_protocl = (char *)argv[5];
		sour_port = (char *)argv[6];
		dest_port = (char *)argv[7];

		pkg_state = (char *)argv[8];
		string_filter = (char *)argv[9];

		str_act = (char *)argv[10];

		str_status = (char *)argv[11];
	
		if(0 == strcmp(str_protocl, "tcp")) {
			protocl = FW_PTCP;
		} else if(0 == strcmp(str_protocl, "udp")) {
			protocl = FW_PUDP;
		} else if(0 == strcmp(str_protocl, "tcp-and-udp")) {
			protocl = FW_PTCPANDUDP;
		} else {
			vty_out(vty, "protocl(%s) input error\n", str_protocl);
			return CMD_WARNING;
		}

		if(firewall_port_is_legal_input(sour_port, &sptype)) {
			vty_out(vty, "sour port(%s) input error\n", sour_port);
			return CMD_WARNING;
		}

		if(firewall_port_is_legal_input(dest_port, &dptype)) {
			vty_out(vty, "dest port(%s) input error\n", dest_port);
			return CMD_WARNING;
		}		
	} else {
		vty_out(vty, "input argc (%d) error\n", argc);
		return CMD_WARNING;
	}

	if(firewall_state_is_legal_input(pkg_state)) {
		vty_out(vty, "state(%s) input error\n", pkg_state);
		return CMD_WARNING;
	}

	if(firewall_filterstring_is_legal_input(string_filter)) {
		vty_out(vty, "filter string(%s) input error\n", string_filter);
		return CMD_WARNING;
	}

 	if(0 == strcmp(str_act, "accept")) {
		act = FW_ACCEPT;
	} else if(0 == strcmp(str_act, "drop")) {
		act = FW_DROP;
	} else if(0 == strcmp(str_act, "reject")) {
		act = FW_REJECT;
	} else {
		vty_out(vty, "act(%s) input error\n", str_act);
		return CMD_WARNING;
	}

 	if(0 == strncmp(str_status, "e", 1)) {
		status = 1;
	} else if(strncmp(str_status, "d", 1)) {
		vty_out(vty, "status(%s) input error\n", str_status);
		return CMD_WARNING;
	} 
	
	memset(&rule, 0, sizeof(fwRule));

	rule.type = FW_INPUT;
	rule.id = index;

	rule.ineth = inif;
	rule.srctype = sip_type;
	rule.srcadd = (FW_MASQUERADE == sip_type) ? NULL: sour_ipaddr;
	rule.dsttype = dip_type;
	rule.dstadd = (FW_MASQUERADE == dip_type) ? NULL: dest_ipaddr;
	
	rule.protocl = protocl;
	rule.sptype = sptype;
	rule.sport = (0 == sptype) ? NULL: sour_port;
	rule.dptype = dptype;
	rule.dport = (0 == dptype) ? NULL: dest_port;

	rule.pkg_state = strcmp(pkg_state, "any") ? pkg_state : NULL;
	rule.string_filter = strcmp(string_filter, "null") ? string_filter : NULL;

	rule.act = act;

	rule.enable = status;

	int ret;
	ret = ac_manage_config_firewall_rule(connection, &rule, config_type);
	switch(ret) {
		case AC_MANAGE_SUCCESS:            
			return CMD_SUCCESS;
		case AC_MANAGE_CONFIG_NONEXIST:
			vty_out(vty, "Slot(%d): index %d rule is not exist!\n", slot_id, index);
			break;
		case AC_MANAGE_CONFIG_EXIST:
			vty_out(vty, "Slot(%d): index %d rule is exist!\n", slot_id, index);
			break;
		case AC_MANAGE_INPUT_TYPE_ERROR:
			vty_out(vty, "Slot(%d): input parameter error!\n", slot_id);
			break;
		case AC_MANAGE_DBUS_ERROR:
			vty_out(vty, "Slot(%d): <error> failed get reply.\n", slot_id);
			break;

		default:
			vty_out(vty, "Slot(%d): unknow fail reason!\n", slot_id);
			break;
	}
	
	return CMD_WARNING;
}

ALIAS(config_input_rule_func,
	config_input_port_rule_cmd,
	"(add|modify) input INDEX (any|INIF) (any|SOURIP) (any|DESTIP) (tcp|udp|tcp-and-udp) (any|SOURPORT) (any|DESTPORT) "
	FIREWALL_PACKAGE_STATE" (null|FILTERSTR) (accept|drop|reject) (enable|disable)",
	"add rule\n"
	"modify rule\n"
	"input, for input chain rule\n"
	"rule index. index format: A-B (A is slot id, B is rule index which begin with 1)\n"
	"input interface is any\n"
	"input interface\n"
	"source ip is any\n"
	"source ip, format: A.B.C.D or A.B.C.D/X.X.X.X or A.B.C.D-A.B.C.D\n"
	"destination ip is any\n"
	"destination ip, format: A.B.C.D or A.B.C.D/X.X.X.X or A.B.C.D-A.B.C.D\n"
	"tcp protocol\n"
	"udp protocol\n"
	"both tcp and udp protocol\n"
	"source port is any\n"
	"source port, format: 80 or 80:162\n"
	"destination port is any\n"
	"destination port, format: 80 or 80:162\n"
	FIREWALL_PACKAGE_STATE_DESC	
	"filter string is null\n"
	"filter string\n"
	"accept act\n"
	"drop act\n"
	"reject act\n"
	"enable\n"
	"disable\n"
)

DEFUN(config_snat_rule_func,
	config_snat_rule_cmd,
	"(add|modify) snat INDEX (any|OUTIF) (any|SOURIP) (any|DESTIP) (any|icmp) (any|NATIP) (enable|disable)",
	"add rule\n"
	"modify rule\n"
	"snat, for snat chain rule\n"
	"rule index. index format: A-B (A is slot id, B is rule index which begin with 1 and end with 256)\n"
	"output interface is any\n"
	"output interface\n"
	"source ip is any\n"
	"source ip, format: A.B.C.D or A.B.C.D/X.X.X.X or A.B.C.D-A.B.C.D\n"
	"destination ip is any\n"
	"destination ip, format: A.B.C.D or A.B.C.D/X.X.X.X or A.B.C.D-A.B.C.D\n"
	"any protocol\n"
	"icmp protocol\n"
	"any nat ip address\n"
	"nat ip, format: A.B.C.D or A.B.C.D/X.X.X.X or A.B.C.D-A.B.C.D\n"
	"enable\n"
	"disable\n"
)
{
	fwRule rule;
	DBusConnection *connection;
	
	u_long config_type = 0, slot_id, index;
	char *str_config	= (char *)argv[0];
	char *str_index		= (char *)argv[1];

	char *out_interface	= (char *)argv[2];
	char outif[32];
	
	u_long sip_type = FW_MASQUERADE, dip_type = FW_MASQUERADE;
	char *sour_ipaddr	= (char *)argv[3];
	char *dest_ipaddr	= (char *)argv[4];

	u_long protocl, sptype = 0, dptype = 0;
	char *str_protocl = NULL;
	char *sour_port = NULL;
	char *dest_port = NULL;

	u_long natip_type = FW_MASQUERADE, natport_type = 0;
	char *str_natip = NULL;
	char *str_natport = NULL;

	u_long status = 0;
	char *str_status = NULL;

	if(0 == strncmp(str_config, "m", 1)) {
		config_type = 1;
	} else if(strncmp(str_config, "a", 1)) {
		vty_out(vty, "input config type error!\n");
		return CMD_WARNING;
	}

	if(firewall_index_is_legal_input(str_index, &slot_id, &index)) {
		vty_out(vty, "input index(%s) type error!\n", str_index);
		return CMD_WARNING;
	}
	
	if (slot_id == HostSlotId) {
		connection = dcli_dbus_connection;
	} else {
		connection = dbus_connection_dcli[slot_id]->dcli_dbus_connection;
	}
	
	if (NULL == connection) {
		vty_out(vty, "The slot(%d) is not connect!\n", slot_id);
		return CMD_WARNING;
	} 

    if (!strncmp(out_interface, "ve", 2)) {
		if (ve_interface_parse(out_interface, outif, sizeof(outif))) {
			vty_out(vty, "input OUTIF(%s) error!\n", out_interface);
			return CMD_WARNING;
		}
	} else {
		strncpy(outif, out_interface, sizeof(outif) - 1);
	}

	if(firewall_ipaddr_is_legal_input(sour_ipaddr, &sip_type)) {
		vty_out(vty, "sour ip addr(%s) input error\n", sour_ipaddr);
		return CMD_WARNING;
	}

	if(firewall_ipaddr_is_legal_input(dest_ipaddr, &dip_type)) {
		vty_out(vty, "dest ip addr(%s) input error\n", dest_ipaddr);
		return CMD_WARNING;
	}

	if(8 == argc) {
		str_protocl = (char *)argv[5];
		str_natip	= (char *)argv[6];
		str_status = (char *)argv[7];
			
		if(0 == strcmp(str_protocl, "any")) {
			protocl = 0;	/** any protocol **/
		} else if(0 == strcmp(str_protocl, "icmp")) {
			protocl = FW_PICMP;
		} else {
			vty_out(vty, "protocl(%s) input error\n", str_protocl);
			return CMD_WARNING;
		}
	} else if(11 == argc) {
		str_protocl = (char *)argv[5];
		sour_port = (char *)argv[6];
		dest_port = (char *)argv[7];
		str_natip	= (char *)argv[8];
		str_natport = (char *)argv[9];
		str_status = (char *)argv[10];
	
		if(0 == strcmp(str_protocl, "tcp")) {
			protocl = FW_PTCP;
		} else if(0 == strcmp(str_protocl, "udp")) {
			protocl = FW_PUDP;
		} else if(0 == strcmp(str_protocl, "tcp-and-udp")) {
			protocl = FW_PTCPANDUDP;
		} else {
			vty_out(vty, "protocl(%s) input error\n", str_protocl);
			return CMD_WARNING;
		}

		if(firewall_port_is_legal_input(sour_port, &sptype)) {
			vty_out(vty, "sour port(%s) input error\n", sour_port);
			return CMD_WARNING;
		}

		if(firewall_port_is_legal_input(dest_port, &dptype)) {
			vty_out(vty, "dest port(%s) input error\n", dest_port);
			return CMD_WARNING;
		}		

		if(firewall_natport_is_legal_input(str_natport, &natport_type)) {
			vty_out(vty, "nat port(%s) input error\n", str_natport);
			return CMD_WARNING;
		}
	} else {
		vty_out(vty, "input argc (%d) error\n", argc);
		return CMD_WARNING;
	}

	if(firewall_ipaddr_is_legal_input(str_natip, &natip_type)) {
		vty_out(vty, "nat ip addr(%s) input error\n", str_natip);
		return CMD_WARNING;
	}

	
 	if(0 == strncmp(str_status, "e", 1)) {
		status = 1;
	} else if(strncmp(str_status, "d", 1)) {
		vty_out(vty, "status(%s) input error\n", str_status);
		return CMD_WARNING;
	} 
	
	memset(&rule, 0, sizeof(fwRule));

	rule.type = FW_SNAT;
	rule.id = index;

	rule.outeth = outif;
	rule.srctype = sip_type;
	rule.srcadd = (FW_MASQUERADE == sip_type) ? NULL: sour_ipaddr;
	rule.dsttype = dip_type;
	rule.dstadd = (FW_MASQUERADE == dip_type) ? NULL: dest_ipaddr;

	rule.protocl = protocl;
	rule.sptype = sptype;
	rule.sport = (0 == sptype) ? NULL: sour_port;
	rule.dptype = dptype;
	rule.dport = (0 == dptype) ? NULL: dest_port;
	
	rule.natiptype = natip_type;
	rule.natipadd = (FW_MASQUERADE == natip_type) ? NULL : str_natip;
	rule.natpttype = natport_type;
	rule.natport = (0 == natport_type) ? NULL : str_natport;

	rule.enable = status;

	int ret;
	ret = ac_manage_config_firewall_rule(connection, &rule, config_type);
	switch(ret) {
		case AC_MANAGE_SUCCESS:            
			return CMD_SUCCESS;
		case AC_MANAGE_CONFIG_NONEXIST:
			vty_out(vty, "Slot(%d): index %d rule is not exist!\n", slot_id, index);
			break;
		case AC_MANAGE_CONFIG_EXIST:
			vty_out(vty, "Slot(%d): index %d rule is exist!\n", slot_id, index);
			break;
		case AC_MANAGE_INPUT_TYPE_ERROR:
			vty_out(vty, "Slot(%d): input parameter error!\n", slot_id);
			break;
		case AC_MANAGE_DBUS_ERROR:
			vty_out(vty, "Slot(%d): <error> failed get reply.\n", slot_id);
			break;

		default:
			vty_out(vty, "Slot(%d): unknow fail reason!\n", slot_id);
			break;
	}
	
	return CMD_WARNING;
}

ALIAS(config_snat_rule_func,
	config_snat_port_rule_cmd,
	"(add|modify) snat INDEX (any|OUTIF) (any|SOURIP) (any|DESTIP) (tcp|udp|tcp-and-udp) (any|SOURPORT) (any|DESTPORT) (any|NATIP) (any|NATPORT) (enable|disable)",
	"add rule\n"
	"modify rule\n"
	"snat, for snat chain rule\n"
	"rule index. index format: A-B (A is slot id, B is rule index which begin with 1)\n"
	"output interface is any\n"
	"output interface\n"
	"source ip is any\n"
	"source ip, format: A.B.C.D or A.B.C.D/X.X.X.X or A.B.C.D-A.B.C.D\n"
	"destination ip is any\n"
	"destination ip, format: A.B.C.D or A.B.C.D/X.X.X.X or A.B.C.D-A.B.C.D\n"
	"tcp protocol\n"
	"udp protocol\n"
	"both tcp and udp protocol\n"
	"source port is any\n"
	"source port, format: 80 or 80:162\n"
	"destination port is any\n"
	"destination port, format: 80 or 80:162\n"
	"any nat ip address\n"
	"nat ip, format: A.B.C.D or A.B.C.D/X.X.X.X or A.B.C.D-A.B.C.D\n"
	"default nat port\n"
	"nat port, format:80 or 80-162\n "
	"enable\n"
	"disable\n"
)



DEFUN(config_dnat_rule_func,
	config_dnat_rule_cmd,
	"(add|modify) dnat INDEX (any|INIF) (any|SOURIP) (any|DESTIP) (any|icmp) NATIP (enable|disable)",
	"add rule\n"
	"modify rule\n"
	"dnat, for dnat chain rule\n"
	"rule index. index format: A-B (A is slot id, B is rule index which begin with 1 and end with 256)\n"
	"output interface is any\n"
	"output interface\n"
	"source ip is any\n"
	"source ip, format: A.B.C.D or A.B.C.D/X.X.X.X or A.B.C.D-A.B.C.D\n"
	"destination ip is any\n"
	"destination ip, format: A.B.C.D or A.B.C.D/X.X.X.X or A.B.C.D-A.B.C.D\n"
	"any protocol\n"
	"icmp protocol\n"
	"nat ip, format: A.B.C.D or A.B.C.D/X.X.X.X or A.B.C.D-A.B.C.D\n"
	"enable\n"
	"disable\n"
)
{
	fwRule rule;	
	DBusConnection *connection;
	
	u_long config_type = 0, slot_id, index;
	char *str_config	= (char *)argv[0];
	char *str_index		= (char *)argv[1];

	char *in_interface	= (char *)argv[2];
	char inif[32];
	
	u_long sip_type = FW_MASQUERADE, dip_type = FW_MASQUERADE;
	char *sour_ipaddr	= (char *)argv[3];
	char *dest_ipaddr	= (char *)argv[4];

	u_long protocl, sptype = 0, dptype = 0;
	char *str_protocl = NULL;
	char *sour_port = NULL;
	char *dest_port = NULL;

	u_long natip_type = FW_MASQUERADE, natport_type = 0;
	char *str_natip = NULL;
	char *str_natport = NULL;

	u_long status = 0;
	char *str_status = NULL;

	if(0 == strncmp(str_config, "m", 1)) {
		config_type = 1;
	} else if(strncmp(str_config, "a", 1)) {
		vty_out(vty, "input config type error!\n");
		return CMD_WARNING;
	}

	if(firewall_index_is_legal_input(str_index, &slot_id, &index)) {
		vty_out(vty, "input index(%s) type error!\n", str_index);
		return CMD_WARNING;
	}
	
	if (slot_id == HostSlotId) {
		connection = dcli_dbus_connection;
	} else {
		connection = dbus_connection_dcli[slot_id]->dcli_dbus_connection;
	}
	
	if (NULL == connection) {
		vty_out(vty, "The slot(%d) is not connect!\n", slot_id);
		return CMD_WARNING;
	} 

    if (!strncmp(in_interface, "ve", 2)) {
		if (ve_interface_parse(in_interface, inif, sizeof(inif))) {
			vty_out(vty, "input INIF(%s) error!\n", in_interface);
			return CMD_WARNING;
		}
	} else {
		strncpy(inif, in_interface, sizeof(inif) - 1);
	}
	
	if(firewall_ipaddr_is_legal_input(sour_ipaddr, &sip_type)) {
		vty_out(vty, "sour ip addr(%s) input error\n", sour_ipaddr);
		return CMD_WARNING;
	}

	if(firewall_ipaddr_is_legal_input(dest_ipaddr, &dip_type)) {
		vty_out(vty, "dest ip addr(%s) input error\n", dest_ipaddr);
		return CMD_WARNING;
	}

	if(8 == argc) {
		str_protocl = (char *)argv[5];
		str_natip	= (char *)argv[6];
		str_status = (char *)argv[7];
			
		if(0 == strcmp(str_protocl, "any")) {
			protocl = 0;	/** any protocol **/
		} else if(0 == strcmp(str_protocl, "icmp")) {
			protocl = FW_PICMP;
		} else {
			vty_out(vty, "protocl(%s) input error\n", str_protocl);
			return CMD_WARNING;
		}
	} else if(11 == argc) {
		str_protocl = (char *)argv[5];
		sour_port = (char *)argv[6];
		dest_port = (char *)argv[7];
		str_natip	= (char *)argv[8];
		str_natport = (char *)argv[9];
		str_status = (char *)argv[10];
	
		if(0 == strcmp(str_protocl, "tcp")) {
			protocl = FW_PTCP;
		} else if(0 == strcmp(str_protocl, "udp")) {
			protocl = FW_PUDP;
		} else if(0 == strcmp(str_protocl, "tcp-and-udp")) {
			protocl = FW_PTCPANDUDP;
		} else {
			vty_out(vty, "protocl(%s) input error\n", str_protocl);
			return CMD_WARNING;
		}

		if(firewall_port_is_legal_input(sour_port, &sptype)) {
			vty_out(vty, "sour port(%s) input error\n", sour_port);
			return CMD_WARNING;
		}

		if(firewall_port_is_legal_input(dest_port, &dptype)) {
			vty_out(vty, "dest port(%s) input error\n", dest_port);
			return CMD_WARNING;
		}		

		if(firewall_natport_is_legal_input(str_natport, &natport_type)) {
			vty_out(vty, "nat port(%s) input error\n", str_natport);
			return CMD_WARNING;
		}
	} else {
		vty_out(vty, "input argc (%d) error\n", argc);
		return CMD_WARNING;
	}

	if(0 == strcmp(str_natip, "any")) {
		vty_out(vty, "nat ip addr(%s) input error\n", str_natip);
		return CMD_WARNING;
	}else if(firewall_ipaddr_is_legal_input(str_natip, &natip_type)) {
		vty_out(vty, "nat ip addr(%s) input error\n", str_natip);
		return CMD_WARNING;
	}

 	if(0 == strncmp(str_status, "e", 1)) {
		status = 1;
	} else if(strncmp(str_status, "d", 1)) {
		vty_out(vty, "status(%s) input error\n", str_status);
		return CMD_WARNING;
	} 
	
	memset(&rule, 0, sizeof(fwRule));

	rule.type = FW_DNAT;
	rule.id = index;

	rule.ineth = inif;
	rule.srctype = sip_type;
	rule.srcadd = (FW_MASQUERADE == sip_type) ? NULL: sour_ipaddr;
	rule.dsttype = dip_type;
	rule.dstadd = (FW_MASQUERADE == dip_type) ? NULL: dest_ipaddr;
	
	rule.protocl = protocl;
	rule.sptype = sptype;
	rule.sport = (0 == sptype) ? NULL: sour_port;
	rule.dptype = dptype;
	rule.dport = (0 == dptype) ? NULL: dest_port;
	
	rule.natiptype = natip_type;
	rule.natipadd = (FW_MASQUERADE == natip_type) ? NULL : str_natip;
	rule.natpttype = natport_type;
	rule.natport = (0 == natport_type) ? NULL : str_natport;

	rule.enable = status;

	int ret;
	ret = ac_manage_config_firewall_rule(connection, &rule, config_type);
	switch(ret) {
		case AC_MANAGE_SUCCESS:            
			return CMD_SUCCESS;
		case AC_MANAGE_CONFIG_NONEXIST:
			vty_out(vty, "Slot(%d): index %d rule is not exist!\n", slot_id, index);
			break;
		case AC_MANAGE_CONFIG_EXIST:
			vty_out(vty, "Slot(%d): index %d rule is exist!\n", slot_id, index);
			break;
		case AC_MANAGE_INPUT_TYPE_ERROR:
			vty_out(vty, "Slot(%d): input parameter error!\n", slot_id);
			break;
		case AC_MANAGE_DBUS_ERROR:
			vty_out(vty, "Slot(%d): <error> failed get reply.\n", slot_id);
			break;

		default:
			vty_out(vty, "Slot(%d): unknow fail reason!\n", slot_id);
			break;
	}
	
	return CMD_WARNING;
}

ALIAS(config_dnat_rule_func,
	config_dnat_port_rule_cmd,
	"(add|modify) dnat INDEX (any|INIF) (any|SOURIP) (any|DESTIP) (tcp|udp|tcp-and-udp) (any|SOURPORT) (any|DESTPORT) NATIP (any|NATPORT) (enable|disable)",
	"add rule\n"
	"modify rule\n"
	"dnat, for dnat chain rule\n"
	"rule index. index format: A-B (A is slot id, B is rule index which begin with 1)\n"
	"output interface is any\n"
	"output interface\n"
	"source ip is any\n"
	"source ip, format: A.B.C.D or A.B.C.D/X.X.X.X or A.B.C.D-A.B.C.D\n"
	"destination ip is any\n"
	"destination ip, format: A.B.C.D or A.B.C.D/X.X.X.X or A.B.C.D-A.B.C.D\n"
	"tcp protocol\n"
	"udp protocol\n"
	"both tcp and udp protocol\n"
	"source port is any\n"
	"source port, format: 80 or 80:162\n"
	"destination port is any\n"
	"destination port, format: 80 or 80:162\n"
	"nat ip, format: A.B.C.D or A.B.C.D/X.X.X.X or A.B.C.D-A.B.C.D\n"
	"default nat port\n"
	"nat port, format:80 or 80-162\n "
	"enable\n"
	"disable\n"
)

DEFUN(change_firewall_index_func,
	change_firewall_index_cmd,
	"change (firewall|input|snat|dnat) INDEX NEWINDEX",
	"change rule index\n"
	"firewall, for filter rule\n"
	"input, for input rule\n"
	"snat, for snat rule\n"
	"dnat, for dnat rule\n"
	"rule index. index format: A-B (A is slot id, B is rule index which begin with 1 and end with 256)\n"
	"new rule index. index format: A-B (A is slot id, B is rule index which begin with 1 and end with 256)\n"
)
{
	DBusConnection *connection;

	char *str_type = (char *)argv[0];
	char *str_index = (char *)argv[1];
	char *str_newindex = (char *)argv[2];
	u_long rule_type, slot_id, index;
	u_long new_slot_id, new_index;

	if(0 == strncmp(str_type, "f", 1)) {
		rule_type = FW_WALL;
	} else if(0 == strncmp(str_type, "i", 1)) {
		rule_type = FW_INPUT;
	} else if(0 == strncmp(str_type, "s", 1)) {
		rule_type = FW_SNAT;
	} else if(0 == strncmp(str_type, "d", 1)) {
		rule_type = FW_DNAT;
	} else {
		vty_out(vty, "input rule type error!\n");
		return CMD_WARNING;
	}

	if(firewall_index_is_legal_input(str_index, &slot_id, &index)) {
		vty_out(vty, "input index(%s) type error!\n", str_index);
		return CMD_WARNING;
	}

	if(firewall_index_is_legal_input(str_newindex, &new_slot_id, &new_index)) {
		vty_out(vty, "input new index(%s) type error!\n", str_index);
		return CMD_WARNING;
	}

	if(new_slot_id != slot_id) {
		vty_out(vty, "error: not support rule move form one slot to another!\n");
		return CMD_WARNING;
	}

	if (slot_id == HostSlotId) {
		connection = dcli_dbus_connection;
	} else {
		connection = dbus_connection_dcli[slot_id]->dcli_dbus_connection;
	}
	
	if (NULL == connection) {
		vty_out(vty, "The slot(%d) is not connect!\n", slot_id);
		return CMD_WARNING;
	} 
	
	int ret;
	ret = ac_manage_change_firewall_index(connection, new_index, rule_type, index);
	switch(ret) {
		case AC_MANAGE_SUCCESS:            
			return CMD_SUCCESS;
		case AC_MANAGE_CONFIG_NONEXIST:
			vty_out(vty, "Slot(%d): index %d rule is not exist!\n", slot_id, index);
			break;
		case AC_MANAGE_INPUT_TYPE_ERROR:
			vty_out(vty, "Slot(%d): input parameter error!\n", slot_id);
			break;
		case AC_MANAGE_DBUS_ERROR:
			vty_out(vty, "Slot(%d): <error> failed get reply.\n", slot_id);
			break;

		default:
			vty_out(vty, "Slot(%d): unknow fail reason!\n", slot_id);
			break;
	}
	
	return CMD_WARNING;
}


DEFUN(del_firewall_func,
	del_firewall_cmd,
	"del (firewall|input|snat|dnat) INDEX",
	"delete rule\n"
	"firewall, for filter rule\n"
	"input, for input rule\n"
	"snat, for snat rule\n"
	"dnat, for dnat rule\n"
	"rule index. index format: A-B (A is slot id, B is rule index which begin with 1 and end with 256)\n"
)
{
	DBusConnection *connection;

	char *str_type = (char *)argv[0];
	char *str_index = (char *)argv[1];
	u_long rule_type, slot_id, index;
	
	if(0 == strncmp(str_type, "f", 1)) {
		rule_type = FW_WALL;
	} else if(0 == strncmp(str_type, "i", 1)) {
		rule_type = FW_INPUT;
	} else if(0 == strncmp(str_type, "s", 1)) {
		rule_type = FW_SNAT;
	} else if(0 == strncmp(str_type, "d", 1)) {
		rule_type = FW_DNAT;
	} else {
		vty_out(vty, "input rule type error!\n");
		return CMD_WARNING;
	}

	if(firewall_index_is_legal_input(str_index, &slot_id, &index)) {
		vty_out(vty, "input index(%s) type error!\n", str_index);
		return CMD_WARNING;
	}

	if (slot_id == HostSlotId) {
		connection = dcli_dbus_connection;
	} else {
		connection = dbus_connection_dcli[slot_id]->dcli_dbus_connection;
	}
	
	if (NULL == connection) {
		vty_out(vty, "The slot(%d) is not connect!\n", slot_id);
		return CMD_WARNING;
	} 

	int ret;
	ret = ac_manage_del_firewall_rule(connection, rule_type, index);
	switch(ret) {
		case AC_MANAGE_SUCCESS:            
			return CMD_SUCCESS;
		case AC_MANAGE_CONFIG_NONEXIST:
			vty_out(vty, "Slot(%d): index %d rule is not exist!\n", slot_id, index);
			break;
		case AC_MANAGE_INPUT_TYPE_ERROR:
			vty_out(vty, "Slot(%d): input parameter error!\n", slot_id);
			break;
		case AC_MANAGE_DBUS_ERROR:
			vty_out(vty, "Slot(%d): <error> failed get reply.\n", slot_id);
			break;

		default:
			vty_out(vty, "Slot(%d): unknow fail reason!\n", slot_id);
			break;
	}
		
	return CMD_WARNING;
}

DEFUN(config_connlimit_rule_func,
	config_connlimit_rule_cmd,
	"(add|modify) (firewall|input) INDEX IPADDR connlimit <0-4096> (enable|disable)",
	"add rule\n"
	"modify rule\n"
	"firewall, for firewall chain rule\n"
	"input, for input chain rule\n"
	"rule index. index format: A-B (A is slot id, B is rule index which begin with 1 and end with 256)\n"
	"ipaddr, format: A.B.C.D or A.B.C.D/X.X.X.X or A.B.C.D-A.B.C.D\n"
	"connect limit\n"
	"limit value\n"
	"enable\n"
	"disable\n"
)
{
	DBusConnection *connection;

	char *str_config = (char *)argv[0];
	char *str_chain = (char *)argv[1];
	char *str_index	= (char *)argv[2];
	char *str_ipaddr = (char *)argv[3];
	char *str_limit	= (char *)argv[4];
	char *str_status = (char *)argv[5];
	fwRule rule;	
	u_long config_type, slot_id;	
	int ret;	

	memset(&rule, 0, sizeof(fwRule));

	if(!strncmp(str_config, "m", 1)) {
		config_type = 1;
	} else if(!strncmp(str_config, "a", 1)) {
		config_type = 0;
	} else {
		vty_out(vty, "input config type error!\n");
		return CMD_WARNING;
	}

	if (firewall_index_is_legal_input(str_index, &slot_id, &rule.id)) {
		vty_out(vty, "input index(%s) type error!\n", str_index);
		return CMD_WARNING;
	}
	
	if (slot_id == HostSlotId) {
		connection = dcli_dbus_connection;
	} else {
		connection = dbus_connection_dcli[slot_id]->dcli_dbus_connection;
	}
	
	if (NULL == connection) {
		vty_out(vty, "The slot(%d) is not connect!\n", slot_id);
		return CMD_WARNING;
	} 

	if (!strncmp(str_chain, "f", 1)) {
		rule.type = FW_WALL;
		rule.ineth = "any";
		rule.outeth = "any";
	} else if (!strncmp(str_chain, "i", 1)) {
		rule.type = FW_INPUT;
		rule.ineth = "any";		
	} else {
		vty_out(vty, "input chain rule failed!\n");
		return CMD_WARNING;
	}

	if (firewall_ipaddr_is_legal_input(str_ipaddr, &rule.srctype)) {
		vty_out(vty, "ipaddr(%s) input error\n", str_ipaddr);
		return CMD_WARNING;
	}

	if (rule.srctype == FW_MASQUERADE) {
		vty_out(vty, "ipaddr(%s) input error\n", str_ipaddr);
		return CMD_WARNING;
	}

	rule.srcadd = str_ipaddr;
	rule.connlimit = str_limit;
	rule.act = FW_DROP;

 	if (!strncmp(str_status, "e", 1)) {
		rule.enable = 1;
	} else if (!strncmp(str_status, "d", 1)) {
		rule.enable = 0;
	} else {
		vty_out(vty, "status(%s) input error\n", str_status);
		return CMD_WARNING;
	} 

	ret = ac_manage_config_firewall_rule(connection, &rule, config_type);
	switch(ret) {
		case AC_MANAGE_SUCCESS:            
			return CMD_SUCCESS;
			
		case AC_MANAGE_CONFIG_NONEXIST:
			vty_out(vty, "Slot(%d): %s index %d rule is not exist!\n", slot_id, str_chain, rule.id);
			break;
		case AC_MANAGE_CONFIG_EXIST:
			vty_out(vty, "Slot(%d): %s index %d rule is exist!\n", slot_id, str_chain, rule.id);
			break;
		case AC_MANAGE_INPUT_TYPE_ERROR:
			vty_out(vty, "Slot(%d): input parameter error!\n", slot_id);
			break;
		case AC_MANAGE_DBUS_ERROR:
			vty_out(vty, "Slot(%d): <error> failed get reply.\n", slot_id);
			break;

		default:
			vty_out(vty, "Slot(%d): unknow fail reason!\n", slot_id);
			break;
	}
	
	return CMD_WARNING;
}


DEFUN(modify_nat_udp_timeout_func,
	modify_nat_udp_timeout_cmd,
	"modify nat udp timeout <30-1800>",
	"modify nat\n"
	"modify nat udp timeout\n"
	"nat udp timeout\n"
	"udp timeout\n"
)
{
	u_long timeout = atoi((char *)argv[0]);
	int ret;

	if (is_distributed) {
		int i;
		for(i = 1; i < MAX_SLOT; i++) {
		
			if (!dbus_connection_dcli[i]->dcli_dbus_connection) 
				continue;

			ret = ac_manage_config_nat_udp_timeout(dbus_connection_dcli[i]->dcli_dbus_connection, timeout);
			switch(ret) {
				case AC_MANAGE_SUCCESS:            
					break;
					
				case AC_MANAGE_INPUT_TYPE_ERROR:
					vty_out(vty, "Slot(%d): input parameter error!\n", i);
					break;
				case AC_MANAGE_DBUS_ERROR:
					vty_out(vty, "Slot(%d): <error> failed get reply.\n", i);
					break;

				default:
					vty_out(vty, "Slot(%d): unknow fail reason!\n", i);
					break;
			}
		}	
	} else {
		ret = ac_manage_config_nat_udp_timeout(dcli_dbus_connection, timeout);
		switch(ret) {
			case AC_MANAGE_SUCCESS:            
				return CMD_SUCCESS;
				
			case AC_MANAGE_INPUT_TYPE_ERROR:
				vty_out(vty, "input parameter error!\n");
				break;
			case AC_MANAGE_DBUS_ERROR:
				vty_out(vty, "<error> failed get reply.\n");
				break;

			default:
				vty_out(vty, "unknow fail reason!\n");
				break;
		}	

		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}


DEFUN(show_firewall_rule_info_func,
	show_firewall_rule_info_cmd,
	"show firewall rule info",
	SHOW_STR
	"firewall configure\n"
	"configure inforamtion\n"
)
{
	vty_out(vty, "========================================================================\n");

	if (is_distributed) {
		int i = 1; 
		for(i = 1; i < MAX_SLOT; i++) {
			if(dbus_connection_dcli[i]->dcli_dbus_connection) {
				vty_out(vty, "Slot %d Firewall Rule Info\n", i);
				dcli_show_firewall_rule_info(dbus_connection_dcli[i]->dcli_dbus_connection, i, vty);
				vty_out(vty, "========================================================================\n");
			}
		}
	} else {
		vty_out(vty, "Firewall Rule Info\n");
		dcli_show_firewall_rule_info(dcli_dbus_connection, HostSlotId, vty);
		vty_out(vty, "========================================================================\n");
	}

	return CMD_SUCCESS;
}

DEFUN(show_firewall_running_config_func,
	show_firewall_running_config_cmd,
	"show firewall running config",
	SHOW_STR
	"show firewall running config\n" 
)
{   
	vty_out(vty, "============================================================================\n");
	vty_out(vty, "config firewall\n");

	firewall_show_rule_running_config(vty);
	    
	vty_out(vty, " exit\n");
	vty_out(vty, "============================================================================\n");

	return CMD_SUCCESS;
}


void dcli_firewall_init
(
	void
)  
{
	install_node(&firewall_node, dcli_firewall_show_running_config, "FIREWALL_NODE");
	install_default(FIREWALL_NODE);
	
	install_element(CONFIG_NODE, &conf_firewall_cmd);
	
	install_element(FIREWALL_NODE, &config_firewall_service_cmd);
	
	install_element(FIREWALL_NODE, &config_firewall_rule_cmd);
	install_element(FIREWALL_NODE, &config_firewall_port_rule_cmd);
	install_element(FIREWALL_NODE, &config_firewall_tcpmss_rule_cmd);
	
	install_element(FIREWALL_NODE, &config_input_rule_cmd);
	install_element(FIREWALL_NODE, &config_input_port_rule_cmd);

	install_element(FIREWALL_NODE, &config_snat_rule_cmd);
	install_element(FIREWALL_NODE, &config_snat_port_rule_cmd);
	
	install_element(FIREWALL_NODE, &config_dnat_rule_cmd);
	install_element(FIREWALL_NODE, &config_dnat_port_rule_cmd);

	install_element(FIREWALL_NODE, &config_connlimit_rule_cmd);

	install_element(FIREWALL_NODE, &change_firewall_index_cmd);
	install_element(FIREWALL_NODE, &del_firewall_cmd);

	install_element(FIREWALL_NODE, &modify_nat_udp_timeout_cmd);

	install_element(FIREWALL_NODE, &show_firewall_rule_info_cmd);
	install_element(FIREWALL_NODE, &show_firewall_running_config_cmd);
}


#ifdef __cplusplus
}
#endif

