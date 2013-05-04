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
* dcli_tc.c
*
* MODIFY:
*		by <chensheng@autelan.com> on 2010-4-26
*
* CREATOR:
*		by <chensheng@autelan.com> on 2010-4-26
*
* DESCRIPTION:
*		CLI definition for traffic control module.
*
* DATE:
*		2010-3-24 11:15:30
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.2 $	
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
#include "sysdef/returncode.h"
#include "dcli_tc.h"

#include "dcli_main.h"

#include "ws_public.h"
#include "ws_dbus_list.h"

#include "ac_manage_def.h"
#include "ac_manage_tcrule_interface.h"

struct cmd_node tc_node = 
{
	TC_NODE,
	"%s(config-tc)# "
};

#undef  TC_DEBUG
#define TC_DEBUG  1
#if TC_DEBUG
#define debug_printf(a...) fprintf(a)
#else
#define debug_printf(a...)
#endif

DEFUN(conf_tc_func,
	conf_tc_cmd,
	"config tc",
	CONFIG_STR
	"config traffic control\n" 
)
{
	if(CONFIG_NODE == vty->node)
	{
		vty->node = TC_NODE;
		vty->index = NULL;	
	}
	else
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}
	
	return CMD_SUCCESS;
}

int tc_ifname_is_legal_input(const char *str)
{
	FILE *fp;
	const char *cmd = "ip link | sed \"/.*link\\/.*brd.*/d\" | sed \"s/^[0-9]*:.\\(.*\\):.*$/\\1/g\" | sed \"/lo/d\"";
	char buff[256];
	char *p = NULL;

	if (strcmp(str, "any") == 0)
		return 1;
	
	if ( (fp = popen(cmd, "r")) == NULL)
		return 0;

	while (fgets(buff, 256, fp) != NULL){
		for (p = buff; *p; p++)
			if ('\r' == *p || '\n' == *p){
				*p = '\0';
				break;
			}

		if (strcmp(buff, str) == 0)
			return 1;
	}

	pclose(fp);
	return 0;
}

int tc_ipaddr_is_legal_input(const char *str)
{
	char *endptr = NULL;
	char* endptr1 = NULL;
	char c;
	int IP,i;
	
	c = str[0];
	if (c>='0'&&c<='9'){
		IP= strtoul(str,&endptr,10);
		if(IP < 0||IP > 255)
			return 0;
		else if(((IP < 10)&&((endptr - str) > 1))||((IP < 100)&&((endptr - str) > 2))||((IP < 256)&&((endptr - str) > 3)))
			return 0;
		for(i = 0; i < 3; i++){
			if(endptr[0] == '\0'||endptr[0] != '.')
				return 0;
			else{
				endptr1 = &endptr[1];
				IP= strtoul(&endptr[1],&endptr,10); 			
				if(IP < 0||IP > 255)
					return 0;				
				else if(((IP < 10)&&((endptr - endptr1) > 1))||((IP < 100)&&((endptr - endptr1) > 2))||((IP < 256)&&((endptr - endptr1) > 3)))
					return 0;
			}
		}
		if(endptr[0] == '\0' && IP >= 0)
			return 1;
		else
			return 0;
	}
	else
		return 0;		
}

int tc_ipmask_is_legal_input(const char *str)
{
	unsigned int ip;
	int i, flag;
	char *token, *strtmp;

	if (!tc_ipaddr_is_legal_input(str))
		return 0;

	strtmp = strdup(str);
	for (ip = 0, token = strtok(strtmp, "."); token; token = strtok(NULL, "."))
		ip = ip*256 + atoi(token);
	free(strtmp);

	for (flag = 0, i = 0; i < 32; i++, ip/=2)
		if (ip%2 == 1) 
			flag = 1;
		else if (flag) 
			return 0;

	return 1;
}

int tc_ipaddrmask_is_legal_input(const char *str)
{
	char addr[16];
	char *mask = NULL;
	memset(addr, 0, sizeof(addr));

	if ( (mask = strchr(str, '/')) == NULL)
		return 0;

	if (mask - str > 15)
		return 0;

	strncpy(addr, str, mask-str);
	if (!tc_ipaddr_is_legal_input(addr))
		return 0;

	mask++;
	if (!tc_ipmask_is_legal_input(mask))
		return 0;
		
	return 1;
}

int tc_bandwidth_is_legal_input(const char *str)
{
	const char *p = NULL;
	int bandwidth;
	
	if (NULL == str || '\0' == str[0])
		return 0;

	for (p = str; *p; p++)
		if (*p < '0' || *p > '9')
			return 0;

	if (strlen(str) > 1 && '0' == str[0])
		return 0;
	
	bandwidth = atoi(str);
	if (bandwidth < 0 || bandwidth > 9999)
		return 0;
	
	return 1;
}

int tc_index_is_legal_input(const char *str)
{
	const char *p;

	if (NULL == str || '\0' == str[0] || '0' == str[0] || strlen(str) > 10)
		return 0;
	
	for (p = str; *p; p++)
		if (*p < '0' || *p > '9')
			return 0;

	return 1;
}

int tc_offset_is_legal_input(const char *str)
{
	const char *p = NULL;

	if (NULL == str || '\0' == str[0])
		return 0;
    
    if('-' == str[0]) {
        if(strlen(str) > 5 || 1 == strlen(str)) {
            return 0;
        }
        p = ++str;
    }
    else if(str[0] < '0' || str[0] > '9') {
        return 0;
    }
    else {
        if(strlen(str) > 4) {
            return 0;
        }
        p = str;        
    }
    
	for (; *p; p++) {
		if (*p < '0' || *p > '9') {
			return 0;
        }			
    }
    
	return 1;
}


static int 
tc_get_slot_id_by_ifname(const char *ifname) {

	int slotnum = -1;
	int i = 0;
	int count = 0;
	char tmp[32];

	memset(tmp, 0, sizeof(tmp));
	memcpy(tmp, ifname, strlen(ifname));

	/* eth : cpu */
	if (0 == strncmp(ifname, "eth", 3)) {
		sscanf(ifname, "eth%d-%*d", &slotnum);
	}

	/* ve */
	else if (0 == strncmp(ifname, "ve", 2)) {
		sscanf(ifname, "ve%d.%*d", &slotnum);
	} 

	/* radio */
	else if (0 == strncmp(ifname, "r", 1)) {
		for (i = 0; i < strlen(ifname); i++) {
			/*use '-' to make sure this radio is local board or remote board */
			if (tmp[i] == '-') {
				count++;
			}			
		}
		
		if (2 == count) {	/*local board*/
			slotnum = get_product_info(PRODUCT_LOCAL_SLOTID);
		} else if(3 == count) {	/*remote board*/
			sscanf(ifname, "r%d-%*d-%*d-%d.%*d", &slotnum);
		}
	}

	/* wlan */
	else if (0 == strncmp(ifname, "wlan", 4)) {
		for (i = 0; i < strlen(ifname); i++) {
			if(tmp[i] == '-') {
				count++;
			}
		}
		
		if (1 == count) {	/*local board*/
			slotnum = get_product_info(PRODUCT_LOCAL_SLOTID);
		} else if (2 == count) {	/*remote board*/
			sscanf(ifname, "wlan%d-%*d-%*d", &slotnum);
		}
	}

	/* ebr */
	else if (0 == strncmp(ifname, "ebr", 3)) {
		for (i = 0; i < strlen(ifname); i++) {
			if (tmp[i] == '-') {
				count++;
			}
		}
		if (1 == count) {	/*local board*/
			slotnum = get_product_info(PRODUCT_LOCAL_SLOTID);
		} else if (2 == count) {	/*remote board*/
			sscanf(ifname, "ebr%d-%*d-%*d", &slotnum);
		}
	}
	else {
        slotnum  = 0;
	}
	
	return slotnum;
}


inline TCRule *tcParseDoc_for_cli(void)
{
	if (access(TCRULES_XML_FILE, 0) != 0)
		return NULL;

	return tcParseDoc(TCRULES_XML_FILE);
}

inline int tc_service_is_running(DBusConnection *connection)
{
    unsigned int status = 0;
	return (ac_manage_show_flow_control_service(connection, &status) ? 0 : status);
}

#define check_tc_service_running(connection) \
do { \
	if(tc_service_is_running(connection)){ \
		vty_out(vty, "traffic control service running, please stop it first\n"); \
		return CMD_FAILURE; \
	} \
} while(0)

int	tc_get_num(TCRule *root)
{
	TCRule *rule = NULL;
	int num = 0;

	for (rule = root; rule; rule = rule->next) num++;

	return num;
}

DEFUN(flow_control_config_service_func,
		flow_control_config_service_cmd,
		"service (enable|disable)",
		"config flow_control service\n"
		"service enable\n"
		"service disable\n" )
{
	int ret = AC_MANAGE_SUCCESS;
    unsigned int status = 0;
    
    if(0 == strcmp(argv[0], "enable")) {
        status = 1;
    }
    else if(strcmp(argv[0], "disable")) {
        vty_out(vty, "input parameter error!\n");
        return CMD_WARNING;
    }

    int i = 1;
    for(i = 1; i < MAX_SLOT; i++) {
    
        if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 
            continue;
            
	    ret = ac_manage_config_flow_control_service(dbus_connection_dcli[i]->dcli_dbus_connection, status);
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

	return CMD_SUCCESS;
}


static TCRule *
tc_get_rule_by_single_ip(const unsigned int index,
                                const char *strUpIf, 
                                const char *strDownIf, 
	                            const char *strIpAddr, 
	                            const char *strUpBandwidth, 
	                            const char *strDownBandwidth)
{
	TCRule *rule = NULL;

	if (NULL == strUpIf || NULL == strDownIf || NULL == strIpAddr || NULL == strUpBandwidth || NULL == strUpBandwidth)
		return NULL;
	if ( (rule = tcNewRule()) == NULL)
		return NULL;
	
	rule->enable = 1;
	rule->ruleIndex = index;
	/*up-if*/
	rule->up_interface = (char *)malloc(TC_IFNAME_LEN);
	memset(rule->up_interface, 0, TC_IFNAME_LEN);
	strncpy(rule->up_interface, strUpIf, TC_IFNAME_LEN);
	/*down-if*/
	rule->interface = (char *)malloc(TC_IFNAME_LEN);
	memset(rule->interface, 0, TC_IFNAME_LEN);
	strncpy(rule->interface, strDownIf, TC_IFNAME_LEN);
	/*single ip*/
	rule->addrtype = (char *)malloc(TC_ADDRTYPE_LEN);
	memset(rule->addrtype, 0, TC_ADDRTYPE_LEN);
	strncpy(rule->addrtype, "address", TC_ADDRTYPE_LEN);
	/* ip addr */
	rule->addr_begin = (char *)malloc(TC_IPADDR_LEN);
	memset(rule->addr_begin, 0, TC_IPADDR_LEN);
	strncpy(rule->addr_begin, strIpAddr, TC_IPADDR_LEN);
	/* up bandwidth */
	rule->uplink_speed = (char *)malloc(TC_BANDWIDTH_LEN);
	memset(rule->uplink_speed, 0, TC_BANDWIDTH_LEN);
	strncpy(rule->uplink_speed, strUpBandwidth, TC_BANDWIDTH_LEN);
	/*down bandwidth*/
	rule->downlink_speed = (char *)malloc(TC_BANDWIDTH_LEN);
	memset(rule->downlink_speed, 0, TC_BANDWIDTH_LEN);
	strncpy(rule->downlink_speed, strDownBandwidth, TC_BANDWIDTH_LEN);

	rule->useP2P = 0;
	rule->time_begin = 0;
	rule->time_end = 0;

	return rule;
}


DEFUN(add_tc_ip_single_func,
	add_tc_ip_single_cmd,
	"add tc INDEX UP-IF DOWN-IF ip single A.B.C.D <0-9999> <0-9999>",
	"add traffic control\n"
	"traffic control\n"
	"tc rule index,like 2-1\n"
	"up interface\n"
	"down interface\n"
	"ip address\n"
	"single ip address\n"
	"ip address\n"
	"up bandwidth\n"
	"down bandwidth\n"
)
{
    int ret = AC_MANAGE_SUCCESS;

	TCRule *ruleRoot = NULL;
	TCRule *rule = NULL;
	const char *strIndex = NULL, *strUpIf = NULL, *strDownIf = NULL, *strIpAddr = NULL, *strUpBandwidth = NULL, *strDownBandwidth = NULL;
	char upif[32], downif[32];
	unsigned int index = 0;
    unsigned int slot_id = 0;

    strIndex = argv[0];
    if(2 != sscanf(strIndex, "%d-%d", &slot_id, &index)) {
		vty_out(vty, "error index input : %s\n", strIndex);
		return CMD_WARNING;
    }
    
    if(0 == slot_id || slot_id >= MAX_SLOT || 0 == index) {
        vty_out(vty, "parse index(%s) fail!\n", strIndex);
        return CMD_WARNING;
    }
    
    if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
        vty_out(vty, "The slot(%d) is not connect!\n", slot_id);
        return CMD_WARNING;
    }    
    
	strUpIf = argv[1];
    /*acmanage check ifname is legal*/
#if 0	
	if (!tc_ifname_is_legal_input(strUpIf)){
		vty_out(vty, "error up interface input : %s\n", strUpIf);
		return CMD_WARNING;
	}
#endif
    if (!strncmp(strUpIf, "ve", 2)) {
		if (ve_interface_parse(strUpIf, upif, sizeof(upif))) {
			vty_out(vty, "error up interface input : %s\n", strUpIf);
			return CMD_WARNING;
		}
	} else {
		strncpy(upif, strUpIf, sizeof(upif) - 1);
	}

	strDownIf = argv[2];
    /*acmanage check ifname is legal*/
#if 0	
	if (!tc_ifname_is_legal_input(strDownIf)){
		vty_out(vty, "error down interface input : %s\n", strDownIf);
		return CMD_WARNING;
	}
#endif
    if (!strncmp(strDownIf, "ve", 2)) {
		if (ve_interface_parse(strDownIf, downif, sizeof(downif))) {
			vty_out(vty, "error down interface input : %s\n", strDownIf);
			return CMD_WARNING;
		}
	} else {
		strncpy(downif, strDownIf, sizeof(downif) - 1);
	}

	strIpAddr = argv[3];
	if (!tc_ipaddr_is_legal_input(strIpAddr)){
		vty_out(vty, "error ip address input : %s\n", strIpAddr);
		return CMD_WARNING;
	}

	strUpBandwidth = argv[4];
	if (!tc_bandwidth_is_legal_input(strUpBandwidth)){
		vty_out(vty, "error bandwidth input : %s\n", strUpBandwidth);
		return CMD_WARNING;
	}

	strDownBandwidth = argv[5];
	if (!tc_bandwidth_is_legal_input(strDownBandwidth)){
		vty_out(vty, "error bandwidth input : %s\n", strDownBandwidth);
		return CMD_WARNING;
	}
    
	check_tc_service_running(dbus_connection_dcli[slot_id]->dcli_dbus_connection);

   	rule = tc_get_rule_by_single_ip(index, upif, downif, strIpAddr, strUpBandwidth, strDownBandwidth);
    ret = ac_manage_add_tcrule(dbus_connection_dcli[slot_id]->dcli_dbus_connection, rule);

    tcFreeRule(rule);

    switch(ret) {
        case AC_MANAGE_SUCCESS:
            return CMD_SUCCESS;

        case AC_MANAGE_CONFIG_EXIST:
			vty_out(vty, "input rule index is exist!\n");
			break;
		case AC_MANAGE_INPUT_TYPE_ERROR:
			vty_out(vty, "input parameter error!\n");
			break;
		case AC_MANAGE_MALLOC_ERROR:
			vty_out(vty, "process malloc memory fail!\n");
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


static TCRule *
tc_get_rule_by_range_ip(const unsigned int index,
                                const char *strUpIf, 
                                const char *strDownIf, 
	                            const char *strIpAddrMask, 
	                            const char *strBWType,
	                            const char *strUpBandwidth, 
	                            const char *strDownBandwidth)
{
	TCRule *rule = NULL;

	if (NULL == strUpIf || NULL == strDownIf || NULL == strIpAddrMask || NULL == strUpBandwidth || NULL == strUpBandwidth)
		return NULL;
	if ( (rule = tcNewRule()) == NULL)
		return NULL;
	
	rule->enable = 1;
	rule->ruleIndex = index;
	/*up-if*/
	rule->up_interface = (char *)malloc(TC_IFNAME_LEN);
	memset(rule->up_interface, 0, TC_IFNAME_LEN);
	strncpy(rule->up_interface, strUpIf, TC_IFNAME_LEN);
	/*down-if*/
	rule->interface = (char *)malloc(TC_IFNAME_LEN);
	memset(rule->interface, 0, TC_IFNAME_LEN);
	strncpy(rule->interface, strDownIf, TC_IFNAME_LEN);
	/*single ip*/
	rule->addrtype = (char *)malloc(TC_ADDRTYPE_LEN);
	memset(rule->addrtype, 0, TC_ADDRTYPE_LEN);
	strncpy(rule->addrtype, "addrrange", TC_ADDRTYPE_LEN);
	/* ip addr */
	char addr_begin[TC_IPADDR_LEN] = { 0 };
	char addr_end[TC_IPADDR_LEN] = { 0 };
	
	sscanf(strIpAddrMask, "%[^/]/%s", addr_begin, addr_end);
	printf("addr_begin = %s\n", addr_begin);
	printf("addr_end = %s\n", addr_end);
	rule->addr_begin = (char *)malloc(TC_IPADDR_LEN);
	memset(rule->addr_begin, 0, TC_IPADDR_LEN);
	strncpy(rule->addr_begin, addr_begin, TC_IPADDR_LEN);
	
	rule->addr_end = (char *)malloc(TC_IPADDR_LEN);
	memset(rule->addr_end, 0, TC_IPADDR_LEN);
	strncpy(rule->addr_end, addr_end, TC_IPADDR_LEN);
	
	/* BWType */
	rule->mode = (char *)malloc(TC_IPADDR_LEN);
	memset(rule->mode, 0, TC_IPADDR_LEN);
	strncpy(rule->mode, strBWType, TC_IPADDR_LEN);
	
	/* up bandwidth */
	rule->uplink_speed = (char *)malloc(TC_BANDWIDTH_LEN);
	memset(rule->uplink_speed, 0, TC_BANDWIDTH_LEN);
	strncpy(rule->uplink_speed, strUpBandwidth, TC_BANDWIDTH_LEN);
	/*down bandwidth*/
	rule->downlink_speed = (char *)malloc(TC_BANDWIDTH_LEN);
	memset(rule->downlink_speed, 0, TC_BANDWIDTH_LEN);
	strncpy(rule->downlink_speed, strDownBandwidth, TC_BANDWIDTH_LEN);

	rule->useP2P = 0;
	rule->time_begin = 0;
	rule->time_end = 0;

	return rule;
}

DEFUN(delete_tc_rule_func,
	delete_tc_rule_cmd,
	"delete tc rule INDEX",
	"delete tc rule\n"
	"rule index"
)
{
    int ret = AC_MANAGE_SUCCESS;

    unsigned int slot_id = 0;
    unsigned int index = 0;

    if(2 != sscanf(argv[0], "%d-%d", &slot_id, &index)) {
        vty_out(vty, "Input INDEX format error!\n");
        return CMD_WARNING;
    }

    if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
        vty_out(vty, "Input INDEX is not exist!\n");
        return CMD_WARNING;
    }

    ret = ac_manage_delete_tcrule(dbus_connection_dcli[slot_id]->dcli_dbus_connection, index);
    switch(ret) {
        case AC_MANAGE_SUCCESS:
            return CMD_SUCCESS;
        
        case AC_MANAGE_CONFIG_EXIST:
			vty_out(vty, "input rule index is exist!\n");
			break;        
		case AC_MANAGE_INPUT_TYPE_ERROR:
			vty_out(vty, "input parameter error!\n");
			break;
		case AC_MANAGE_MALLOC_ERROR:
			vty_out(vty, "process malloc memory fail!\n");
			break;
	    case AC_MANAGE_DBUS_ERROR:
            vty_out(vty, "<error> failed get reply.\n");
			break;

	    default:
	        vty_out(vty, "unknow fail reason!\n");
	        break;
    }
	
	return CMD_SUCCESS;
}


DEFUN(add_tc_ip_subnet_func,
	add_tc_ip_subnet_cmd,
	"add tc INDEX UP-IF DOWN-IF ip subnet A.B.C.D/X.X.X.X (shared|non-shared) <0-9999> <0-9999>",
	"add traffic control\n"
	"traffic control\n"
	"tc rule index,like 2-1\n"
	"up interface\n"
	"down interface\n"
	"ip address\n"
	"subnet\n"
	"ip address and mask\n"
	"shared bandwidth\n"
	"non-shared bandwidth\n"
	"up bandwidth\n"
	"down bandwidth\n"
)
{
    int ret = AC_MANAGE_SUCCESS;
    
	TCRule *ruleRoot = NULL;
	TCRule *rule = NULL;
	const char *strIndex = NULL, *strUpIf = NULL, *strDownIf = NULL, *strIpAddrMask = NULL, *strBWType = NULL, *strUpBandwidth = NULL, *strDownBandwidth = NULL;
	char upif[32], downif[32];
    unsigned int slot_id = 0;
    unsigned int index = 0;
    
    strIndex = argv[0];
    if(2 != sscanf(strIndex, "%d-%d", &slot_id, &index)) {
		vty_out(vty, "error index input : %s\n", strIndex);
		return CMD_WARNING;
    }
    
    if(0 == slot_id || slot_id >= MAX_SLOT || 0 == index) {
        vty_out(vty, "parse index(%s) fail!\n", strIndex);
        return CMD_WARNING;
    }
    
    if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
        vty_out(vty, "The slot(%d) is not connect!\n", slot_id);
        return CMD_WARNING;
    }    

	strUpIf = argv[1];
	/*acmanage check ifname is legal*/
#if 0
	if (!tc_ifname_is_legal_input(strUpIf)){
		vty_out(vty, "error interface input : %s\n", strUpIf);
		return CMD_WARNING;
	}
#endif
    if (!strncmp(strUpIf, "ve", 2)) {
		if (ve_interface_parse(strUpIf, upif, sizeof(upif))) {
			vty_out(vty, "error up interface input : %s\n", strUpIf);
			return CMD_WARNING;
		}
	} else {
		strncpy(upif, strUpIf, sizeof(upif) - 1);
	}

	strDownIf = argv[2];    
	/*acmanage check ifname is legal*/
#if 0	
	if (!tc_ifname_is_legal_input(strDownIf)){
		vty_out(vty, "error interface input : %s\n", strDownIf);
		return CMD_WARNING;
	}
#endif
    if (!strncmp(strDownIf, "ve", 2)) {
		if (ve_interface_parse(strDownIf, downif, sizeof(downif))) {
			vty_out(vty, "error down interface input : %s\n", strDownIf);
			return CMD_WARNING;
		}
	} else {
		strncpy(downif, strDownIf, sizeof(downif) - 1);
	}

	strIpAddrMask = argv[3];
	if (!tc_ipaddrmask_is_legal_input(strIpAddrMask)){
		vty_out(vty, "error ip address mask input : %s\n", strIpAddrMask);
		return CMD_WARNING;
	}

	if (0 == strcmp(argv[4], "shared"))
		strBWType = "share";
	else if (0 == strcmp(argv[4], "non-shared"))
		strBWType = "notshare";
	else {
		vty_out(vty, "error parameter input : %s\n", strBWType);
		return CMD_WARNING;
	}

	strUpBandwidth = argv[5];
	if (!tc_bandwidth_is_legal_input(strUpBandwidth)){
		vty_out(vty, "error bandwidth input : %s\n", strUpBandwidth);
		return CMD_WARNING;
	}

	strDownBandwidth = argv[6];
	if (!tc_bandwidth_is_legal_input(strDownBandwidth)){
		vty_out(vty, "error bandwidth input : %s\n", strDownBandwidth);
		return CMD_WARNING;
	}	

	check_tc_service_running(dbus_connection_dcli[slot_id]->dcli_dbus_connection);

	rule = tc_get_rule_by_range_ip(index, 
	                                upif, 
	                                downif, 
	                                strIpAddrMask,
	                                strBWType, 
	                                strUpBandwidth,
	                                strDownBandwidth);

    ret = ac_manage_add_tcrule(dbus_connection_dcli[slot_id]->dcli_dbus_connection, rule);	
    switch(ret) {
        case AC_MANAGE_SUCCESS:
            return CMD_SUCCESS;
            
        case AC_MANAGE_CONFIG_NONEXIST:
            vty_out(vty, "input rule index is not exist!\n");
            break;        
		case AC_MANAGE_INPUT_TYPE_ERROR:
			vty_out(vty, "input parameter error!\n");
			break;
		case AC_MANAGE_MALLOC_ERROR:
			vty_out(vty, "process malloc memory fail!\n");
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


DEFUN(offset_tc_rule_func,
	offset_tc_rule_cmd,
	"offset tc rule INDEX uplink OFFSET downlink OFFSET",
	"offset tc rule\n"
	"rule index"
	"up link offset: -9999 ~ 9999"
	"down link offset: -9999 ~ 9999"
)
{
    int ret = AC_MANAGE_SUCCESS;
    
    unsigned int slot_id = 0;
    struct tcrule_offset_s offset = { 0 };

    if(2 != sscanf(argv[0], "%d-%d", &slot_id, &offset.ruleIndex)) {
        vty_out(vty, "Input INDEX format error!\n");
        return CMD_WARNING;
    }

    if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
        vty_out(vty, "Input INDEX is not exist!\n");
        return CMD_WARNING;
    }
    if(1 != tc_offset_is_legal_input(argv[1])) {
        vty_out(vty, "Input uplink offset error!\n");
        return CMD_WARNING;
    }
    offset.uplink_offset = atoi(argv[1]);
    
    if(1 != tc_offset_is_legal_input(argv[2])) {
        vty_out(vty, "Input downlink offset error!\n");
        return CMD_WARNING;
    }
    offset.downlink_offset = atoi(argv[2]);

    ret = ac_manage_offset_tcrule(dbus_connection_dcli[slot_id]->dcli_dbus_connection, &offset);	    
    switch(ret) {
        case AC_MANAGE_SUCCESS:
            return CMD_SUCCESS;
            
        case AC_MANAGE_CONFIG_NONEXIST:
            vty_out(vty, "input rule index is not exist!\n");
            break;        
		case AC_MANAGE_INPUT_TYPE_ERROR:
			vty_out(vty, "input parameter error!\n");
			break;
		case AC_MANAGE_MALLOC_ERROR:
			vty_out(vty, "process malloc memory fail!\n");
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


static void
dcli_show_tcrule_info(DBusConnection *connection, unsigned int slot_id, struct vty *vty) {
    if(NULL == connection || NULL == vty)
        return;

    int ret = AC_MANAGE_SUCCESS;
    TCRule *rule_array = NULL;
    unsigned int count = 0;
    ret = ac_manage_show_tcrule(connection, &rule_array, &count);
    if(AC_MANAGE_SUCCESS == ret) {

        int i = 0;
        for(i = 0; i < count; i++) {
            
            vty_out(vty, "%d-%d   %10s   %10s       %10s   %s%s%s  %8s  %5s  %5s  %7s\n", slot_id, rule_array[i].ruleIndex, rule_array[i].up_interface ? rule_array[i].up_interface : "", 
                        rule_array[i].interface ? rule_array[i].interface : "", rule_array[i].addrtype ? rule_array[i].addrtype : "",
                        rule_array[i].addr_begin ? rule_array[i].addr_begin : "", rule_array[i].addr_end ? "/" : "", rule_array[i].addr_end ? rule_array[i].addr_end : "",
                        rule_array[i].mode ? rule_array[i].mode : "", rule_array[i].uplink_speed ? rule_array[i].uplink_speed : "", 
                        rule_array[i].downlink_speed ? rule_array[i].downlink_speed : "", rule_array[i].enable ? "enable" : "disable");
                        
        }
        
        tcFreeArray(&rule_array, count);
    }
    
    return;
}


DEFUN(show_tcrule_info_func,
	show_tcrule_info_cmd,
	"show tcrule info",
	SHOW_STR
	"tc configure\n"
	"configure inforamtion\n"
)
{
                       
    
    vty_out(vty,"-------------------------------------------------------------------------------\n");
    vty_out(vty, "%5s   %11s   %13s   %10s    %6s           %6s     %6s  %8s  %6s\n", 
                    "Index", "UpInterface", "DownInterface", "IPAddrType", "IPAddr", "BWType", "UpLink", "DownLink", "Status");
    int i = 1; 
    for(i = 1; i < MAX_SLOT; i++) {
        if(dbus_connection_dcli[i]->dcli_dbus_connection) {
            
            dcli_show_tcrule_info(dbus_connection_dcli[i]->dcli_dbus_connection, i, vty);
        }
    }
    vty_out(vty,"-------------------------------------------------------------------------------\n");
	return CMD_SUCCESS;
}

static void
dcli_show_tcrule_offset(DBusConnection *connection, unsigned int slot_id, struct vty *vty) {
    if(NULL == connection || NULL == vty)
        return;

    int ret = AC_MANAGE_SUCCESS;
    struct tcrule_offset_s *offset_array = NULL;
    unsigned int count = 0;
    ret = ac_manage_show_tcrule_offset(connection, &offset_array, &count);
    if(AC_MANAGE_SUCCESS == ret) {

        int i = 0;
        for(i = 0; i < count; i++) {
            if(0 == offset_array[i].ruleIndex) {
                continue;   /*no offset config*/
            }
            
            vty_out(vty, "%d-%d   %12d    %14d\n", slot_id, offset_array[i].ruleIndex, offset_array[i].uplink_offset, offset_array[i].downlink_offset);
        }
        MANAGE_FREE(offset_array);
    }
    
    return;
}


DEFUN(show_tcrule_offset_func,
	show_tcrule_offset_cmd,
	"show tcrule offset",
	SHOW_STR
)
{
                       
    
    vty_out(vty,"-------------------------------------------------------------------------------\n");
    vty_out(vty, "%5s     %12s    %14s\n", "Index", "UpLinkOffset", "DownLinkOffset");
    int i = 1; 
    for(i = 1; i < MAX_SLOT; i++) {
        if(dbus_connection_dcli[i]->dcli_dbus_connection) {
            
            dcli_show_tcrule_offset(dbus_connection_dcli[i]->dcli_dbus_connection, i, vty);
        }
    }
    vty_out(vty,"-------------------------------------------------------------------------------\n");
	return CMD_SUCCESS;
}



int
ac_manage_show_tcrule_running_config(DBusConnection *connection, struct vty *vty) {
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

    int ret = AC_MANAGE_SUCCESS;
    unsigned int moreConfig = 0;
    char *showStr = NULL;
    
    query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME,
										AC_MANAGE_TCRULE_DBUS_OBJPATH,
										AC_MANAGE_TCRULE_DBUS_INTERFACE,
										AC_MANAGE_DBUS_SHOW_TCRULE_RUNNING_CONFIG);
	                                     
    dbus_error_init(&err);

    reply = dbus_connection_send_with_reply_and_block(connection, query, -1, &err);

    dbus_message_unref(query);

    if(NULL == reply) {
        if(dbus_error_is_set(&err)) {
            dbus_error_free_for_dcli(&err);
        }
        return AC_MANAGE_DBUS_ERROR;
    }
    dbus_message_iter_init(reply, &iter);
    dbus_message_iter_get_basic(&iter, &ret);

    dbus_message_iter_next(&iter);  
    dbus_message_iter_get_basic(&iter, &moreConfig);

    while(moreConfig) {
    
        dbus_message_iter_next(&iter);  
        dbus_message_iter_get_basic(&iter, &showStr);
        if(vty) {
            vty_out(vty, "%s\n", showStr);
        }    
        else {
            vtysh_add_show_string(showStr);
        }

        dbus_message_iter_next(&iter);  
        dbus_message_iter_get_basic(&iter, &moreConfig);
    }    

    dbus_message_unref(reply);

    return ret;
    	                                     
}


DEFUN(show_tc_running_config_func,
	show_tc_running_config_cmd,
	"show tc running config",
	SHOW_STR
	"show tc running config\n" 
)
{   
    vty_out(vty, "============================================================================\n");
    vty_out(vty, "config tc\n");

    int i = 1;
    for(i = 1; i < MAX_SLOT; i++) {
        if(dbus_connection_dcli[i]->dcli_dbus_connection) {            
            ac_manage_show_tcrule_running_config(dbus_connection_dcli[i]->dcli_dbus_connection, vty);            
        }
    }

    if(tc_service_is_running(dcli_dbus_connection)) {
        vty_out(vty, " service enable\n");
    }
    
    vty_out(vty, " exit\n");
    vty_out(vty, "============================================================================\n");
	return CMD_SUCCESS;
}


int dcli_tc_show_running_config(struct vty* vty)
{
	do {
		char _tmpstr[64];
		memset(_tmpstr, 0, 64);
		sprintf(_tmpstr, BUILDING_MOUDLE, "traffic control");
		vtysh_add_show_string(_tmpstr);
	} while(0);
    
    vtysh_add_show_string("config tc");
    
    int i = 1;
    for(i = 1; i < MAX_SLOT; i++) {
        if(dbus_connection_dcli[i]->dcli_dbus_connection) {
            ac_manage_show_tcrule_running_config(dbus_connection_dcli[i]->dcli_dbus_connection, NULL);
        }
    }
    
    if(tc_service_is_running(dcli_dbus_connection)) {
        vtysh_add_show_string(" service enable");
    }
    
    vtysh_add_show_string(" exit");

	return CMD_SUCCESS;;
}

void dcli_tc_init
(
	void
)  
{
	install_node(&tc_node, dcli_tc_show_running_config, "TC_NODE");
	install_default(TC_NODE);
	
	install_element(CONFIG_NODE, &conf_tc_cmd);

	
	install_element(TC_NODE, &flow_control_config_service_cmd);
	install_element(TC_NODE, &add_tc_ip_single_cmd);
	install_element(TC_NODE, &add_tc_ip_subnet_cmd);	
	install_element(HIDDENDEBUG_NODE, &offset_tc_rule_cmd);
	install_element(TC_NODE, &delete_tc_rule_cmd);

    install_element(HIDDENDEBUG_NODE, &show_tcrule_offset_cmd);    
	install_element(TC_NODE, &show_tcrule_info_cmd);	
	install_element(TC_NODE, &show_tc_running_config_cmd);	
	install_element(CONFIG_NODE, &show_tc_running_config_cmd);	
}

#ifdef __cplusplus
}
#endif

