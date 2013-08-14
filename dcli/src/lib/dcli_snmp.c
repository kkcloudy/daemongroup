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
* dcli_dhcp.c
*
* MODIFY:
*		by <tangsq@autelan.com> on 2010-1-28 15:09:03 revision <0.1>
*
* CREATOR:
*		tangsq@autelan.com
*
* DESCRIPTION:
*		CLI definition for dhcp module.
*
* DATE:
*		2010-1-29 12:00:11
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.33 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include <zebra.h>
#include <dbus/dbus.h>
#include <stdlib.h>
#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpathInternals.h>
#include "vty.h"
#include "command.h"
#include "if.h"
#include "sysdef/returncode.h"
#include "dcli_snmp.h"
#include "ws_sysinfo.h"
#include <sys/wait.h>
#include "ws_snmpd_dbus_interface.h"
#include "manage_log.h"
#include "ac_manage_def.h"
#include "ws_snmpd_engine.h"
#include "ws_snmpd_manual.h"
#include "ac_manage_interface.h"
#include "bsd/bsdpub.h"
#include "dcli_main.h"
#include "ws_public.h"
#include "ac_manage_extend_interface.h"
#include "board/board_define.h"

DBusConnection *ccgi_dbus_connection = NULL;

struct cmd_node snmp_node = 
{
	SNMP_NODE,
	"%s(config-snmp)# "
};

struct cmd_node snmp_view_node = 
{
	SNMP_VIEW_NODE,
	"%s(config-snmp-view)# "
};

struct cmd_node snmp_slot_node = 
{
    SNMP_SLOT_NODE,
    "%s(config-snmp-slot %d)# "
};



static int CHECK_IP_DOT(int a)
{
	if(a<0 || a>255)
		return -1;
	else return 0;
}
static int check_ip_part(char * src)
{
	int ipPart[4];
	char * endptr;
	char * token = NULL;
	char * ipSplit = ".";
	int i = 1;
	
	if( src == NULL )
	{
		return -1;
	}
	/*alloc temp memery to strtok to avoid  error when occurs in strtok*/
	char src_back[32];
	memset(src_back, 0, 32);
	strncpy(src_back, src, 32);

	token = strtok( src_back, ipSplit );
	ipPart[0] = strtoul(token, &endptr, 10);
	if( 0 != CHECK_IP_DOT(ipPart[0]) )
	{
		return -2;
	}

	
	while( token != NULL && i < 4 )
	{
		token = strtok(NULL, ipSplit);
		ipPart[i] = strtoul(token, &endptr, 10);

		if( 0 != CHECK_IP_DOT(ipPart[i]) )
		{
			return -2;
		}
		i++;
	}

	return 0;
}

#if 0
static int checkIpFormatValid(char *ipAddress)
{
	if( ipAddress == NULL )
	{
		return -1;
	}

	int i;
	/*int mask;
	char * endptr;
	char * strPart1, *strPart2;
	char * strSplit = "/";*/
	char * str = NULL;
	str = ipAddress ;
	int length = strlen(str);
	if( length > WEB_IPMASK_STRING_MAXLEN || length < WEB_IPMASK_STRING_MINLEN )
	{
		return -1;
	}
	
	for( i=0; i<length; i++ )
	{
		if( str[i] != '.'  && \
			str[i] != '/'  && \
			str[i] != '\0' && \
			( str[i] > '9' || str[i] < '0' )
		  )
		  return -1;
	}
		
	#if 0       //为检测MASK合法性，目前只检测IP地址
	strPart1 = NULL;
	strPart1 = strtok(str, strSplit);
	strPart2 = strtok(NULL, strSplit);
	fprintf(stderr,"strPart1=%s--strPart2=%s\n",strPart1,strPart2);
	mask = strtoul(strPart2, &endptr, 0);
	if( mask < 0 || mask > 32 )
	{
		return INPUT_IP_MASK_ERROR;
	}
	
	
	if( INPUT_OK != check_ip_part(strPart1) )
	{
		return INPUT_IP_ERROR;
	}
	#endif
	
	if( 0 != check_ip_part(str) )
	{
		return -1;
	}
	return 0;
}
#endif

static int
dcli_check_snmp_name(char *name, char flag, struct vty *vty) {
    if(NULL == name)
        return -1;
        
    int ret_c = 0;
    ret_c = check_snmp_name(name, flag);
    if((ret_c == -2)||(ret_c == -1)) {
        vty_out(vty, "The lenth of name should be 1 to 20");
        return -1;
    }
    else if(ret_c == -3) {
        vty_out(vty, "User name can't begin with # \" \' , \\ only appeared in end");
        return -1;
    }
    else if(ret_c == -4) {
        vty_out(vty, "User name should be ASCII value in the Non-control characters (Spaces except)");
        return -1;
    }
    else if(ret_c == -5){
        vty_out(vty, "User name cann't contain special character \\");
        return -1;
    }

    return 0;
}

int 
trap_index_is_legal_input(const char *str){

	const char *p = NULL;

	if (NULL == str || '\0' == str[0] || strlen(str) > 10)
		return 0;
	if (strlen(str) > 1 && '0' == str[0])
		return 0;
	
	for (p = str; *p; p++)
		if (*p < '0' || *p > '9')
			return 0;

	if (atoi(str) > INT_MAX)
		return 0;
	
	return 1;
}

int 
trap_name_is_legal_input(const char *str) {

	if (NULL == str || '\0' == str[0] || strlen(str) > (MAX_SNMP_NAME_LEN - 1))
		return 0;

	const char *p = NULL;
	for (p = str; *p; p++)
		if (!isgraph(*p))
			return 0;
		
	return 1;
}

int 
snmp_ipaddr_is_legal_input(const char *str) {

	char *endptr = NULL;
	char* endptr1 = NULL;
	char c;
	int IP, i;
	
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

int 
snmp_community_is_legal_input(const char *str) {

	if (NULL == str || '\0' == str[0] || strlen(str) > MAX_SNMP_NAME_LEN)
		return 0;

	const char *p=NULL;
	for (p = str; *p; p++)
		if (!isalpha(*p))
			return 0;
		
	return 1;
}

int 
snmp_port_is_legal_input(const char *str) {

	const char *p = NULL;
	int port;
	
	if (NULL == str || '\0' == str[0] || '0' == str[0])
		return 0;

	for (p = str; *p; p++)
		if (*p < '0' || *p > '9')
			return 0;
		
	port = atoi(str);
	if (port < 0 || port > 65535)
		return 0;
	
	return 1;
}

DEFUN(snmp_manual_set_hansi_func,
	snmp_manual_set_hansi_cmd,
	"snmp manual set (local_hansi|remote_hansi) A-B master",
	"snmp manual set local_hansi master\n"
	"snmp manual set remote_hansi master\n" 
)
{
    
    unsigned int local_id = 0;
    unsigned int slot_id = 0;
    unsigned int instance_id = 0;
    
    if(0 == strcmp(argv[0], "local_hansi")) {
        local_id = 1;
    }
    else if(0 == strcmp(argv[0], "remote_hansi")) {
        local_id = 0;
    }
    else {
        vty_out(vty, "Input hansi type is error!\n");
        return CMD_WARNING;
    }
    
    if(0 == sscanf(argv[1], "%d-%d", &slot_id, &instance_id)) {
		vty_out(vty, "Input instance (%s) type is error\n", argv[1]);
        return CMD_WARNING;
    }
    
    if(0 == slot_id || slot_id > 16) {
		vty_out(vty, "Error slot id input : %s\n", argv[1]);
        return CMD_WARNING;
    }
    else if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		vty_out(vty, "The slot %d is not connect\n", slot_id);
        return CMD_WARNING;
    }
    
    int ret = AC_MANAGE_SUCCESS;
    ret = ac_manage_config_snmp_manual_instance(dbus_connection_dcli[slot_id]->dcli_dbus_connection,
                                                local_id, instance_id, 1);
    if(AC_MANAGE_SUCCESS == ret) {
        //vty_out(vty, "Snmp manual %s %d-%d master success\n", local_id ? "local_hansi" : "remote_hansi", slot_id, instance_id);
    }
    else if(AC_MANAGE_DBUS_ERROR == ret) {
        vty_out(vty, "<error> failed get reply.\n");
    }
    else {
        vty_out(vty, "Snmp manual %s %d-%d master failed!\n", local_id ? "local_hansi" : "remote_hansi", slot_id, instance_id);
    }
    
    return CMD_SUCCESS;
}

DEFUN(clear_snmp_manual_set_hansi_func,
	clear_snmp_manual_set_hansi_cmd,
	"clear snmp manual set (local_hansi|remote_hansi) A-B",
	"clear snmp manual set local_hansi master\n"
	"clear snmp manual set remote_hansi master\n" 
)
{    
    unsigned int local_id = 0;
    unsigned int slot_id = 0;
    unsigned int instance_id = 0;

    if(0 == strcmp(argv[0], "local_hansi")) {
        local_id = 1;
    }
    else if(0 == strcmp(argv[0], "remote_hansi")) {
        local_id = 0;
    }
    else {
        vty_out(vty, "Input hansi type is error!\n");
        return CMD_WARNING;
    }
    
    if(0 == sscanf(argv[1], "%d-%d", &slot_id, &instance_id)) {
		vty_out(vty, "Input instance (%s) type is error\n", argv[1]);
        return CMD_WARNING;
    }

    if(0 == slot_id || slot_id > 16) {
		vty_out(vty, "Error slot id input : %s\n", argv[1]);
        return CMD_WARNING;
    }
    else if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		vty_out(vty, "The slot %d is not connect\n", slot_id);
        return CMD_WARNING;
    }

    int ret = AC_MANAGE_SUCCESS;
    ret = ac_manage_config_snmp_manual_instance(dbus_connection_dcli[slot_id]->dcli_dbus_connection,
                                                local_id, instance_id, 0);

    if(AC_MANAGE_SUCCESS == ret) {
        //vty_out(vty, "Clear Snmp manual set %s %d-%d success\n", local_id ? "local_hansi" : "remote_hansi", slot_id, instance_id);
    }
    else if(AC_MANAGE_DBUS_ERROR == ret) {
        vty_out(vty, "<error> failed get reply.\n");
    }
    else {
        vty_out(vty, "Clear Snmp manual set %s %d-%d failed\n", local_id ? "local_hansi" : "remote_hansi", slot_id, instance_id);
    }
    
    return CMD_SUCCESS;
}


DEFUN(config_snmp_decentralized_collect_func,
	config_snmp_decentralized_collect_cmd,
	"(open|close) snmp decentralized collect",
	CONFIG_STR
	"open snmp decentralized collect\n"
	"close snmp decentralized collect\n"
)
{
	if (SNMP_NODE == vty->node) {
	    if(0 == strcmp(argv[0], "open")) {
            int i = 1;
            for(i = 1; i < MAX_SLOT; i++) {
                if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                
                    int ret = AC_MANAGE_SUCCESS;
                    ret = ac_manage_config_snmp_collection_mode(dbus_connection_dcli[i]->dcli_dbus_connection, 1); 
                    if(AC_MANAGE_DBUS_ERROR == ret) {
                        vty_out(vty, "<error> slot %d failed get reply.\n", i);
                    }
                }
            }
		}  
		else if(0 == strcmp(argv[0], "close")) {
		
            int i = 1;
            for(i = 1; i < MAX_SLOT; i++) {
                if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                    
                    int ret = AC_MANAGE_SUCCESS;
                    ret = ac_manage_config_snmp_collection_mode(dbus_connection_dcli[i]->dcli_dbus_connection, 0); 
                    if(AC_MANAGE_DBUS_ERROR == ret) {
                        vty_out(vty, "<error> slot %d failed get reply.\n", i);
                    }
                }
            }
		}
		else {
            vty_out(vty, "Input snmp decentralized collect status error!\n");
            return CMD_WARNING;
		}
	}
	else
	{
		vty_out (vty, "Terminal mode change must under snmp collection mode!\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}

DEFUN(conf_snmp_decentral_slot_func,
	conf_snmp_decentral_slot_cmd,
	"config snmp slot <1-16>",
	CONFIG_STR
	"config snmp slot\n"
)
{    
	if (SNMP_NODE == vty->node) {
	    if(snmp_cllection_mode(dcli_dbus_connection)) {
    	    unsigned int slot_id = atoi(argv[0]);
    	    if(slot_id >= MAX_SLOT || NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
                vty_out(vty, "This slot %d is not exist!\n", slot_id);
                return CMD_WARNING;
    	    } 
    	    else {
    	        vty->node = SNMP_SLOT_NODE;
    	        vty->slotindex = slot_id;
    	    }
        }
        else {
            vty_out (vty, "config snmp slot mode, need open snmp decentralized collect!\n");
            return CMD_WARNING;
        }
	}
	else {
		vty_out (vty, "Terminal mode change must under snmp collection mode!\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}


DEFUN(conf_snmp_func,
	conf_snmp_cmd,
	"config snmp",
	CONFIG_STR
	"config snmp!\n"
)
{

	if (CONFIG_NODE == vty->node) {
		vty->node = SNMP_NODE;
	}
	else
	{
		vty_out (vty, "Terminal mode change must under configure mode!\n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}



DEFUN(switch_service_func,
	switch_service_cmd,
	"snmp service (enable|disable)",
	"snmp \n"
	"snmp service \n" 
	"Enable snmp service \n"
	"Disable snmp service \n"
)
{	
    unsigned int state = 0;
	if (!strcmp(argv[0], "enable")){
		state = 1;
	}
	else if (!strcmp(argv[0], "disable")) {
		state = 0;
	}
	else {
        vty_out(vty, "Input snmp service status error!\n");
        return CMD_WARNING;
	}
	
    int ret = AC_MANAGE_SUCCESS;

	if(SNMP_NODE == vty->node) {
	    if(!snmp_cllection_mode(dcli_dbus_connection)) {
            ret = ac_manage_config_snmp_service(dcli_dbus_connection, state);
        }
        else {
            vty_out (vty, "Snmp service change must under snmp slot configure mode!\n");
            return CMD_WARNING;
        }
    }
    else if(SNMP_SLOT_NODE == vty->node) {
        unsigned int slot_id = vty->slotindex;
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
            ret = ac_manage_config_snmp_service(dbus_connection_dcli[slot_id]->dcli_dbus_connection, state);

        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!!\n", slot_id);
            return CMD_WARNING;
        }
    }
    else {
		vty_out (vty, "Terminal mode change must under configure snmp mode!\n");
        return CMD_WARNING;
    }
    
    if(AC_MANAGE_SUCCESS == ret) {
        //vty_out(vty, "Snmp service %s success\n", state ? "enable" : "disable");
    }
    else if(AC_MANAGE_DBUS_ERROR == ret) {
        vty_out(vty, "<error> failed get reply.\n");
    }
    else {
        vty_out(vty, "Snmp service %s failed\n", state ? "enable" : "disable");
    }

	return CMD_SUCCESS;
}

#if 0
static int ifname2ifindex_by_ioctl(const char *dev)
{
	struct ifreq ifr;
	int fd;
	int err;

	memset(&ifr,0,sizeof(ifr));
	strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	err = ioctl(fd, SIOCGIFINDEX, &ifr);
	if (err) 
	{
		close(fd);
		return 0;
	}
	close(fd);
	return ifr.ifr_ifindex;
}
#endif

DEFUN(snmp_config_collect_interface_port_func,
	snmp_config_collect_interface_port_cmd,
	"snmp (apply|delete) interface NAME udp port <1-65535>",
	SETT_STR
	"snmp apply interface\n" 
	"snmp delete interface\n"
	"snmp interface\n"
	"snmp interface name\n"
	"snmp bind udp port\n"
)
{
    
    unsigned int snmp_port = atoi(argv[2]);
	char inif[64] = { 0 };
    
    unsigned int state = 0;
    if(0 == strcmp(argv[0], "apply")) {
        state = 1;
    }
    else if(0 == strcmp(argv[0], "delete")) {
        state = 0;
    }
    else {
        vty_out(vty, "Input snmp interface mode error!\n");
        return CMD_WARNING;
    }

	memset(inif, 0, sizeof(inif));
	if(1 == state)
	{
		if (!strncmp(argv[1], "ve", 2)) 
		{
			if (ve_interface_parse(argv[1], inif, sizeof(inif))) 
			{
				vty_out(vty, "input interface %s error!\n", argv[1]);
				return CMD_WARNING;
			}
		} 
		else
		{
			strncpy(inif, argv[1], sizeof(inif)-1);
		}
		
		int ifindex = ifname2ifindex_by_ioctl(inif);
		if(0 == ifindex){
			vty_out(vty,"interface %s is not exist!\n",argv[1]);
			return CMD_WARNING;
		}	
	}
	else if(0 == state)
	{
		strncpy(inif, argv[1], sizeof(inif)-1);
	}
    
    char *ifName = (char *)malloc(MAX_INTERFACE_NAME_LEN);
    if(NULL == ifName) {
        vty_out(vty, "malloc error !\n");                
        return CMD_WARNING;
    }
    memset(ifName, 0, MAX_INTERFACE_NAME_LEN);
    strncpy(ifName, inif, MAX_INTERFACE_NAME_LEN - 1);
    
    int ret = AC_MANAGE_SUCCESS;    

    if(SNMP_SLOT_NODE == vty->node) {	    
        unsigned int slot_id = vty->slotindex;
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {     
            ret = ac_manage_config_snmp_pfm_requestpkts(dbus_connection_dcli[slot_id]->dcli_dbus_connection, ifName, snmp_port, state);
    	}
    	else {
            vty_out(vty, "The slot %d tipc dbus is not connect!!\n", slot_id);
            goto CMD_END;
    	}
	}
	else if(SNMP_NODE == vty->node) {
	    if(!snmp_cllection_mode(dcli_dbus_connection)) {
            ret = ac_manage_config_snmp_pfm_requestpkts(dcli_dbus_connection, ifName, snmp_port, state);
        }   
        else {
            vty_out (vty, "This mode must close snmp decentralized collect!\n");
            goto CMD_END;
        }
	}
	else {
		vty_out (vty, "Terminal mode change must under configure snmp mode or configure snmp slot mode!\n");
        goto CMD_END;
	}
    
    if(AC_MANAGE_SUCCESS == ret) {
        //vty_out(vty, "snmp %s apply interface(%s) udp port(%d) success\n", argv[0], ifName, snmp_port);
    }
    else if(AC_MANAGE_DBUS_ERROR == ret) {
        vty_out(vty, "<error> failed get reply.\n");
    }
    else if(AC_MANAGE_SERVICE_ENABLE == ret) {
        vty_out(vty, "Snmp service is enable, please disable it first!\n");
    }
    else if(AC_MANAGE_CONFIG_EXIST == ret) {
        vty_out(vty, "This interface(%s) is exist!\n", ifName);
    }
    else if(AC_MANAGE_CONFIG_NONEXIST == ret) {
        vty_out(vty, "This interface(%s) is not exist!\n", ifName);
    }    
    else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) {
        vty_out(vty, "The input para is error!\n");
    }
    else if(AC_MANAGE_CONFIG_FAIL == ret) {
        vty_out(vty, "There is exsit udp port config, please delete it frist!\n");
    }
    else {
        vty_out(vty, "snmp %s interface(%s) udp port(%d) failed!\n", argv[0],ifName, snmp_port);
    }

CMD_END:    
    MANAGE_FREE(ifName);
    return CMD_SUCCESS;
}


DEFUN(set_snmp_mode_func,
	set_snmp_mode_cmd,
	"set snmp mode (v1|v2|v3) (enable|disable)",
	SETT_STR
	"config snmp\n" 
	"snmp version mode\n"
	"snmp version mode v1  \n"
	"snmp version mode v2c \n"
	"snmp version mode v3user \n"
	"Enable snmp mode v1 or v2c or v3user \n"
	"Disable snmp mode v1 or v2c or v3user \n"
)
{
    unsigned int snmp_mode = 2; //v2c
    if(0 == strcmp(argv[0], "v1")) {
        snmp_mode = 1;
    }
    else if(0 == strcmp(argv[0], "v2")) {
        snmp_mode = 2;
    }
    else if(0 == strcmp(argv[0], "v3")) {
        snmp_mode = 3;
    }
    else {
        vty_out(vty, "Input snmp interface mode error!\n");
        return CMD_WARNING;
    }

    unsigned int state = 1; //enable
    if(0 == strcmp(argv[1], "enable")) {
        state = RULE_ENABLE;
    }
    else if(0 ==  strcmp(argv[1], "disable")) {
        state = RULE_DISABLE;
    }

    int ret = AC_MANAGE_SUCCESS;
        
	if(SNMP_NODE == vty->node) {	    
        if(snmp_cllection_mode(dcli_dbus_connection)) {
            int i = 1;
            for(i = 1; i < MAX_SLOT; i++) {
                if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                    int temp_ret = AC_MANAGE_SUCCESS;
                    temp_ret = ac_manage_config_snmp_version_mode(dbus_connection_dcli[i]->dcli_dbus_connection, snmp_mode, state);
                    if(AC_MANAGE_SUCCESS == temp_ret) {
                        //vty_out(vty, "Slot(%d) :set snmp mode success\n", i);
                    }
                    else if(AC_MANAGE_DBUS_ERROR == temp_ret) {
                        vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
                    }
                    else if(AC_MANAGE_SERVICE_ENABLE == temp_ret) {
                        vty_out(vty, "Slot(%d) :Snmp service is enable, please disable it first!\n", i);
                    }
                    else{
                        vty_out(vty, "Slot(%d) :set snmp mode failed\n", i);
                    }
                }
            }  
            
            return CMD_SUCCESS;
        }
        else {
            ret = ac_manage_config_snmp_version_mode(dcli_dbus_connection, snmp_mode, state);
        }
    }
    else if(SNMP_SLOT_NODE == vty->node) {
        unsigned int slot_id = vty->slotindex;
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
            
            ret = ac_manage_config_snmp_version_mode(dbus_connection_dcli[slot_id]->dcli_dbus_connection, snmp_mode, state);
        }
        else {  
            vty_out(vty, "The slot %d tipc dbus is not connect!!\n", slot_id);
            return CMD_WARNING;
        }
    }
    else {
		vty_out (vty, "Terminal mode change must under configure snmp mode!\n");
		return CMD_WARNING;
    }

	if(AC_MANAGE_SUCCESS == ret) {
        //vty_out(vty, "set snmp mode success\n");
    }
    else if(AC_MANAGE_DBUS_ERROR == ret) {
        vty_out(vty, "<error> failed get reply.\n");
    }
    else if(AC_MANAGE_SERVICE_ENABLE == ret) {
        vty_out(vty, "Snmp service is enable, please disable it first!\n");
    }
    else {
        vty_out(vty, "set snmp mode failed\n");
    }
    
    return CMD_SUCCESS;
}

DEFUN(update_snmp_sysinfo_func,
	update_snmp_sysinfo_cmd,
	"update snmp sysinfo",
	SHOW_STR
	"snmp sysinfo\n"
)
{
    if(SNMP_NODE == vty->node) {        
        if(snmp_cllection_mode(dcli_dbus_connection)) {
            int i = 1;
            for(i = 1; i < MAX_SLOT; i++) {
                if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                    int temp_ret = ac_manage_config_snmp_update_sysinfo(dbus_connection_dcli[i]->dcli_dbus_connection);
                    if(AC_MANAGE_SUCCESS == temp_ret) {
                        //vty_out(vty, "Slot(%d) :Update snmp sysinfo success\n", i);
                    }
                    else if(AC_MANAGE_SERVICE_ENABLE == temp_ret) {
                        vty_out(vty, "Slot(%d) :Snmp service is enable, please disable it first!\n", i);
                    }
                    else {
                        vty_out(vty, "Slot(%d) :Update snmp sysinfo failed!\n", i);
                    }
                }
            }
        }
        else {
            int temp_ret = ac_manage_config_snmp_update_sysinfo(dcli_dbus_connection);
            if(AC_MANAGE_SUCCESS == temp_ret) {
                //vty_out(vty, "Update snmp sysinfo success\n");
            }
            else if(AC_MANAGE_SERVICE_ENABLE == temp_ret) {
                vty_out(vty, "Snmp service is enable, please disable it first!\n");
            }
            else {
                vty_out(vty, "Update snmp sysinfo failed!\n");
            }
        }
    }  
    else if(SNMP_SLOT_NODE == vty->node) {
        unsigned int slot_id = vty->slotindex;
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
            int temp_ret = ac_manage_config_snmp_update_sysinfo(dbus_connection_dcli[slot_id]->dcli_dbus_connection);
            if(AC_MANAGE_SUCCESS == temp_ret) {
                //vty_out(vty, "Update snmp sysinfo success\n");
            }
            else if(AC_MANAGE_SERVICE_ENABLE == temp_ret) {
                vty_out(vty, "Snmp service is enable, please disable it first!\n");
            }
            else {
                vty_out(vty, "Update snmp sysinfo failed!\n");
            }
        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!\n", slot_id);
            return CMD_WARNING;
        }   
    }
    else {
        vty_out (vty, "Terminal mode change must under SNMP mode!\n");
    }
	return CMD_SUCCESS;
}


DEFUN(set_snmp_cache_time_func,
	set_snmp_cache_time_cmd,
	"set cachetime <1-1800>",
	"set\n"
	"snmp cachetime\n"
	"suggest 30 at least\n"
)
{	
    
    unsigned int cachetime = 0;
    if(NULL != argv[0])
        cachetime = atoi(argv[0]);
    else {
        vty_out(vty, "Input cacheTime type error!\n");
        return CMD_SUCCESS;
    }
    
    int ret = AC_MANAGE_SUCCESS;
    
    if(SNMP_NODE == vty->node) {
        if(snmp_cllection_mode(dcli_dbus_connection)) {
            int i = 1;
            for(i = 1; i < MAX_SLOT; i++) {
                if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                    int temp_ret = AC_MANAGE_SUCCESS;
                    temp_ret = ac_manage_config_snmp_cachetime(dbus_connection_dcli[i]->dcli_dbus_connection, cachetime);

                    if(AC_MANAGE_SUCCESS == temp_ret) {
                        //vty_out(vty, "Slot(%d) :set snmp cachetime(%d) success\n", i, cachetime);
                    }
                    else if(AC_MANAGE_DBUS_ERROR == temp_ret) {
                        vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
                    }
                    else if(AC_MANAGE_SERVICE_ENABLE == temp_ret) {
                        vty_out(vty, "Slot(%d) :Snmp service is enable, please disable it first!\n", i);
                    }
                    else{
                        vty_out(vty, "Slot(%d) :set snmp cacheTime failed!\n", i);
                    }
                }
            }  
            return CMD_SUCCESS;
        }
        else {
            ret = ac_manage_config_snmp_cachetime(dcli_dbus_connection, cachetime);
        }
	}
	else if(SNMP_SLOT_NODE == vty->node) {
	
        unsigned int slot_id = vty->slotindex;
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
            ret = ac_manage_config_snmp_cachetime(dbus_connection_dcli[slot_id]->dcli_dbus_connection, cachetime);
        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!\n", slot_id);
            return CMD_SUCCESS;
        }   
	}
	else {
		vty_out (vty, "Terminal mode change must under configure snmp mode!\n");
		return CMD_SUCCESS;
	}

    if(AC_MANAGE_SUCCESS == ret) {
        //vty_out(vty, "set snmp cachetime(%d) success\n", cachetime);
    }
    else if(AC_MANAGE_DBUS_ERROR == ret) {
        vty_out(vty, "<error> failed get reply.\n");
    }
    else if(AC_MANAGE_SERVICE_ENABLE == ret) {
        vty_out(vty, "Snmp service is enable, please disable it first!\n");
    }
    else{
        vty_out(vty, "set snmp cacheTime failed!\n");
    }
    
	return CMD_SUCCESS;
}

static void
dcli_show_snmp_base_info(DBusConnection *connection, struct vty *vty) {
    if(NULL == connection || NULL == vty)
        return ;
    
    int ret1 = AC_MANAGE_SUCCESS, ret2 = AC_MANAGE_SUCCESS, ret3 = AC_MANAGE_SUCCESS;
    STSNMPSysInfo snmp_info;
    unsigned int snmp_state = 0;

    SNMPINTERFACE *interface_array = NULL;
    unsigned int interface_num = 0;
    
    ret1 = ac_manage_show_snmp_base_info(connection, &snmp_info);
    ret2 = ac_manage_show_snmp_state(connection, &snmp_state);
    if(AC_MANAGE_SUCCESS == ret1) {
        unsigned int port = 0;
        ret3 = ac_manage_show_snmp_pfm_interface(connection, &interface_array, &interface_num, &port);
    }
    
    if(AC_MANAGE_SUCCESS == ret1 && AC_MANAGE_SUCCESS == ret2) {
        
        vty_out(vty, "SNMP Service          : %s \n", snmp_state ? "enable" : "disable" );
        vty_out(vty, "system name           : %s \n", snmp_info.sys_name );
        vty_out(vty, "system describtion    : %s \n", snmp_info.sys_description);
        vty_out(vty, "system OID            : %s \n", snmp_info.sys_oid);
        vty_out(vty, "Collection mode       : %s \n", snmp_info.collection_mode ? "decentral" : "concentre");
        vty_out(vty, "cachetime             : %d \n", snmp_info.cache_time ? snmp_info.cache_time : 300);
        vty_out(vty, "SNMP V1 mode          : %s \n", (snmp_info.v1_status) ? "enable" : "disable");
        vty_out(vty, "SNMP V2C mode         : %s \n", (snmp_info.v2c_status)? "enable" : "disable");
        vty_out(vty, "SNMP V3 mode          : %s \n", (snmp_info.v3_status) ? "enable" : "disable");
        vty_out(vty, "-------------------------------------------------------------\n");            
        if(0 == interface_num && snmp_info.collection_mode) {
            vty_out(vty, "Bind Port Not Set!\n");   
        }
        else { 
            vty_out(vty, "Bind Port             : %d \n", snmp_info.agent_port ? snmp_info.agent_port : 161);   
            int i = 0;
            for(i = 0; i < interface_num; i++) {
                vty_out(vty, "Interface:            : %s\n", interface_array[i].ifName);   
            }
        }    
        vty_out(vty, "-------------------------------------------------------------\n");            
    }
    else if(AC_MANAGE_DBUS_ERROR == ret1 || AC_MANAGE_DBUS_ERROR == ret2) {
        vty_out(vty, "<error> failed get reply.\n");
    }
    else if(AC_MANAGE_MALLOC_ERROR == ret1) {
        vty_out(vty, "malloc error!\n");
    }
    else {
        vty_out(vty, "get snmp base info failed\n");
    }

    return ;
}


DEFUN(show_snmp_base_info_func,
	show_snmp_base_info_cmd,
	"show snmp base info",
	SHOW_STR
	"snmp information\n" 
	"snmp base inforamtion\n"
	"snmp base inforamtion\n"
)
{
    
    if(SNMP_SLOT_NODE == vty->node){
        unsigned int slot_id = vty->slotindex;
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {            
            dcli_show_snmp_base_info(dbus_connection_dcli[slot_id]->dcli_dbus_connection, vty);
        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!!\n", slot_id);
        }
    }
    else{
        if(snmp_cllection_mode(dcli_dbus_connection)) {
        
            vty_out(vty,"-------------------------------------------------------------------------------\n");

            int i = 1;
            for(i = 1; i < MAX_SLOT; i++) {
                if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                
                    vty_out(vty, "SNMP SLOT             : %d \n", i );
                    
                    dcli_show_snmp_base_info(dbus_connection_dcli[i]->dcli_dbus_connection, vty);

                    vty_out(vty,"-------------------------------------------------------------------------------\n");
                }
            } 
        }
        else {
        
            dcli_show_snmp_base_info(dcli_dbus_connection, vty);
        }	    
    }
	return CMD_SUCCESS;
}

DEFUN(show_snmp_dbus_connection_list_func,
	show_snmp_dbus_connection_list_cmd,
	"show snmp dbus connection list",
	SHOW_STR
	"snmp dbus\n" 
	"snmp dbus connection list\n"
	"snmp dbus connection list\n"
)
{
    if(SNMP_NODE == vty->node) {
        if(snmp_cllection_mode(dcli_dbus_connection)) {            
            vty_out(vty, "SNMP DBUS CONNECTION INFOMATION\n");       

            int i = 1;
            for(i = 1; i < MAX_SLOT; i++) {
                if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                    vty_out(vty,"===============================================================================\n");
                    int ret = 0;
                    int slot_count = 0;
                    struct snmpdInstanceInfo *snmpdHead = NULL, *snmpdNode = NULL;
                    
                    ret = show_snmpd_dbus_connection_list(dbus_connection_dcli[i]->dcli_dbus_connection, &snmpdHead, &slot_count);

                    if(SNMPD_DBUS_SUCCESS == ret && snmpdHead){
                        vty_out(vty, "SNMP SLOT             : %d \n", i);        
                        vty_out(vty,"-------------------------------------------------------------------------------\n");
                        for(snmpdNode = snmpdHead; NULL != snmpdNode; snmpdNode = snmpdNode->next) {
                            vty_out(vty, "Dbus Connection State : %s \n", snmpdNode->dbus_connection_state ? "CONNECT" : "BREAK");
                            vty_out(vty, "Master Instance Count : %d \n", snmpdNode->master_count);
                    
                            int i, j;
                            for(i = 0; i < VRRP_TYPE_NUM; i++){
                                for(j = 0; j<= INSTANCE_NUM; j++){
                                    if(SNMPD_VRRP_MASTER == snmpdNode->instance_state[i][j]) {
                                        vty_out(vty,"\n%s Instance %d : %s\n", i ? "Local  Hansi" : "Remote Hansi", j, "MASTER");
                                    }
                                    else if(SNMPD_VRRP_BACK== snmpdNode->instance_state[i][j]) {                        
                                        vty_out(vty,"\n%s Instance %d : %s\n", i ? "Local  Hansi" : "Remote Hansi", j, "BACK");
                                    }
                                }
                            }
                            vty_out(vty,"-------------------------------------------------------------------------------\n");
                        }
                    }
                    else if(SNMPD_DBUS_SUCCESS == ret && NULL == snmpdHead) {
                        vty_out(vty, "SLOT (%d):There is no instance!\n", i);
                    }
                    else {
                        vty_out(vty, "SLOT (%d):SNMP service is not enable!\n", i);
                    }

                    free_snmpd_dbus_connection_list(&snmpdHead);    
                    vty_out(vty,"===============================================================================\n");
                }

            }        
        }
        else {
            int ret = 0;
            int slot_count = 0;
        	struct snmpdInstanceInfo *snmpdHead = NULL, *snmpdNode = NULL;

            ret = show_snmpd_dbus_connection_list(dcli_dbus_connection, &snmpdHead, &slot_count);

            if(SNMPD_DBUS_SUCCESS == ret && snmpdHead){
                vty_out(vty, "SNMP DBUS CONNECTION INFOMATION\n");        
                vty_out(vty, "SNMP SLOT COUNT       : %d \n", slot_count);        
                vty_out(vty,"-------------------------------------------------------------------------------\n");
                for(snmpdNode = snmpdHead; NULL != snmpdNode; snmpdNode = snmpdNode->next) {
                    vty_out(vty, "Slot ID               : %d \n", snmpdNode->slot_id);
                    vty_out(vty, "Dbus Connection State : %s \n", snmpdNode->dbus_connection_state ? "CONNECT" : "BREAK");
                    vty_out(vty, "Master Instance Count : %d \n", snmpdNode->master_count);

                    int i, j;
                    for(i = 0; i < VRRP_TYPE_NUM; i++){
                        for(j = 0; j<= INSTANCE_NUM; j++){
                            if(SNMPD_VRRP_MASTER == snmpdNode->instance_state[i][j]) {
                                vty_out(vty,"\n%s Instance %d : %s\n", i ? "Local  Hansi" : "Remote Hansi", j, "MASTER");
                            }
                            else if(SNMPD_VRRP_BACK== snmpdNode->instance_state[i][j]) {                        
                                vty_out(vty,"\n%s Instance %d : %s\n", i ? "Local  Hansi" : "Remote Hansi", j, "BACK");
                            }
                        }
                    }
                    vty_out(vty,"-------------------------------------------------------------------------------\n");
                }
            }            
            else if(SNMPD_DBUS_SUCCESS == ret && NULL == snmpdHead) {
                vty_out(vty, "There is no instance!\n");
            }
            else {
                vty_out(vty, "SNMP service is not enable!\n");
            }
    	
            free_snmpd_dbus_connection_list(&snmpdHead);
        }        
    }
    else if(SNMP_SLOT_NODE == vty->node){
        unsigned int slot_id = vty->slotindex;
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {            
            int ret = 0;
            int slot_count = 0;
            struct snmpdInstanceInfo *snmpdHead = NULL, *snmpdNode = NULL;
            
            ret = show_snmpd_dbus_connection_list(dbus_connection_dcli[slot_id]->dcli_dbus_connection, &snmpdHead, &slot_count);

            if(SNMPD_DBUS_SUCCESS == ret && snmpdHead){
                vty_out(vty, "SNMP DBUS CONNECTION INFOMATION\n");       
                vty_out(vty, "SNMP SLOT             : %d \n", slot_id);        
                vty_out(vty,"-------------------------------------------------------------------------------\n");
                for(snmpdNode = snmpdHead; NULL != snmpdNode; snmpdNode = snmpdNode->next) {
                    vty_out(vty, "Dbus Connection State : %s \n", snmpdNode->dbus_connection_state ? "CONNECT" : "BREAK");
                    vty_out(vty, "Master Instance Count : %d \n", snmpdNode->master_count);
            
                    int i, j;
                    for(i = 0; i < VRRP_TYPE_NUM; i++){
                        for(j = 0; j<= INSTANCE_NUM; j++){
                            if(SNMPD_VRRP_MASTER == snmpdNode->instance_state[i][j]) {
                                vty_out(vty,"\n%s Instance %d : %s\n", i ? "Local  Hansi" : "Remote Hansi", j, "MASTER");
                            }
                            else if(SNMPD_VRRP_BACK== snmpdNode->instance_state[i][j]) {                        
                                vty_out(vty,"\n%s Instance %d : %s\n", i ? "Local  Hansi" : "Remote Hansi", j, "BACK");
                            }
                        }
                    }
                    vty_out(vty,"-------------------------------------------------------------------------------\n");
                }
            }            
            else if(SNMPD_DBUS_SUCCESS == ret && NULL == snmpdHead) {
                vty_out(vty, "There is no instance!\n");
            }
            else {
                vty_out(vty, "SNMP service is not enable!\n");
            }

            free_snmpd_dbus_connection_list(&snmpdHead);
        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!!\n", slot_id);
        }
    }

    
	return CMD_SUCCESS;
}


static void
dcli_snmp_show_community_info(DBusConnection *connection, struct vty *vty) {
    if(NULL == connection || NULL == vty)
        return;

    STCommunity *community_array = NULL;
    int ret = AC_MANAGE_SUCCESS;
    unsigned int community_num = 0;
    
    ret = ac_manage_show_snmp_community(connection, 
                                        &community_array,
                                        &community_num);
    
    if(AC_MANAGE_SUCCESS == ret){
        
        vty_out(vty,"-------------------------------------------------------------------------------\n");
        vty_out(vty, "  %13s    %10s      %10s      %10s    %7s \n", "CommunityName", "IPAddr", "MASK", "AccessMode", "Status");
    
        int i = 0;
        for(i = 0; i < community_num; i++) {
            vty_out(vty, "%12s   %14s    %14s    %10s    %7s \n", community_array[i].community,
                                                    community_array[i].ip_addr, community_array[i].ip_mask,
                                                    community_array[i].access_mode ? "ReadWrite" : "ReadOnly",
                                                    community_array[i].status ? "enabled" : "disabled");
        }
        vty_out(vty,"-------------------------------------------------------------------------------\n");
        free(community_array);
    }
    else if(AC_MANAGE_CONFIG_NONEXIST == ret) {
        vty_out(vty, "There is no community config!\n");
    }
    else if(AC_MANAGE_MALLOC_ERROR == ret){
        vty_out(vty, "<error> failed get reply.\n");
    }
    else if(AC_MANAGE_MALLOC_ERROR == ret) {
        vty_out(vty, "<error> malloc error!\n");
    }
    else {
        vty_out(vty, "Failed get snmp community!\n");
    }
    
    return;
}


DEFUN(show_community_info_func,
	show_community_info_cmd,
	"show community info",
	SHOW_STR
	"snmp community configure\n"
	"configure inforamtion\n"
)
{
    if(SNMP_SLOT_NODE != vty->node) {
        if(snmp_cllection_mode(dcli_dbus_connection)) {
                       
            vty_out(vty,"===============================================================================\n");

            int i = 1; 
            for(i = 1; i < MAX_SLOT; i++) {
                if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                    
                    vty_out(vty, "SNMP SLOT             : %d \n", i );  
                    
                    dcli_snmp_show_community_info(dbus_connection_dcli[i]->dcli_dbus_connection, vty);

                    vty_out(vty,"===============================================================================\n");
                }
            }
        }       
        else {
            dcli_snmp_show_community_info(dcli_dbus_connection, vty);
        }
    }
    else{        
        unsigned int slot_id = vty->slotindex;
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) { 
            dcli_snmp_show_community_info(dbus_connection_dcli[slot_id]->dcli_dbus_connection, vty);
        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!\n", slot_id);
        }
    }   
    
	return CMD_SUCCESS;
}


DEFUN(add_snmp_community_func,
	add_snmp_community_cmd,
	"add community NAME IP MASK (ro|rw) (enable|disable)",
	"add\n"
	"snmp community \n"
	"community name \n"
	"community ipaddr \n"
	"community mask \n"
	"community read-only \n"
	"community read-write\n"
	"community enable \n"
	"community disable\n"
)
{

    if(0 != dcli_check_snmp_name(argv[0], 1, vty))
        return CMD_WARNING;
        
    int ret1 = 0, ret2 = 0;
	unsigned long ip_int = 0;
	unsigned long ip_int1 = 0;
    ret1 = snmp_ipaddr_is_legal_input((char *)argv[1]);
    ret2 = snmp_ipaddr_is_legal_input((char *)argv[2]);
    if((ret1!=1) || (ret2!=1)) {
        vty_out(vty, "IP or MASK doesn't meet format!");
        return CMD_WARNING;
    }
	ip_int = inet_addr((char *)argv[1]);
	ip_int1 = inet_addr((char *)argv[2]);
	if(ip_int & ~ip_int1)
	{
        vty_out(vty, "(IP & ~MASK) must is 0!");
        return CMD_WARNING;
	}
    STCommunity communityNode = { 0 };
    strncpy(communityNode.community, argv[0], sizeof(communityNode.community) - 1);
    strncpy(communityNode.ip_addr, argv[1], sizeof(communityNode.ip_addr) - 1);
    strncpy(communityNode.ip_mask, argv[2], sizeof(communityNode.ip_mask) - 1);

    
    if(0 == strcmp(argv[3], "ro")) {
        communityNode.access_mode = 0;
    }
    else if(0 == strcmp(argv[3], "rw")) {
        communityNode.access_mode = 1;
    }
    else {
        vty_out(vty, "Input snmp community permission type error!\n");
        return CMD_WARNING;
    }

    if(0 == strcmp(argv[4], "disable")) {
        communityNode.status = 0;
    }
    else if(0 == strcmp(argv[4], "enable")) {
        communityNode.status = 1;
    }
    else {
        vty_out(vty, "Input snmp community status error!\n");
        return CMD_WARNING;
    }
    
    int ret = AC_MANAGE_SUCCESS;
    
    if(SNMP_NODE == vty->node) {
        if(snmp_cllection_mode(dcli_dbus_connection)) {
            int i = 1;
            for(i = 1; i < MAX_SLOT; i++) {
                if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                    int temp_ret = AC_MANAGE_SUCCESS;
                    temp_ret = ac_manage_config_snmp_add_community(dbus_connection_dcli[i]->dcli_dbus_connection, &communityNode);
                    if(AC_MANAGE_SUCCESS == temp_ret) {
                        //vty_out(vty, "Slot(%d) :add snmp community success\n", i);
                    }
                    else if(AC_MANAGE_DBUS_ERROR == temp_ret) {
                        vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
                    }
                    else if(AC_MANAGE_SERVICE_ENABLE == temp_ret) {
                        vty_out(vty, "Slot(%d) :Snmp service is enable, please disable it first!\n", i);
                    }
                        else if(AC_MANAGE_CONFIG_EXIST == temp_ret) {
                        vty_out(vty, "Slot(%d) :This community is exist!\n", i);
                    }
                    else{
                        vty_out(vty, "Slot(%d) :set snmp cacheTime failed!\n", i);
                    }
                                                                
                }
            }
            return CMD_SUCCESS; 
        }
        else {  
            ret = ac_manage_config_snmp_add_community(dcli_dbus_connection, &communityNode);
        }
    }
    else if(SNMP_SLOT_NODE == vty->node) {
        
        unsigned int slot_id = vty->slotindex;
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {

            ret = ac_manage_config_snmp_add_community(dbus_connection_dcli[slot_id]->dcli_dbus_connection, &communityNode);
        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!\n", slot_id);
            return CMD_WARNING; 
        }
    }
    else {
        vty_out (vty, "Terminal mode change must under config mode!\n");
        return CMD_WARNING; 
    }
    
    if(AC_MANAGE_SUCCESS == ret) {
        //vty_out(vty, "add snmp community success\n");
    }
    else if(AC_MANAGE_DBUS_ERROR == ret) {
        vty_out(vty, "<error> failed get reply.\n");
    }
    else if(AC_MANAGE_SERVICE_ENABLE == ret) {
        vty_out(vty, "Snmp service is enable, please disable it first!\n");
    }
    else if(AC_MANAGE_CONFIG_EXIST == ret) {
        vty_out(vty, "This community is exist!\n");
    }
    else{
        vty_out(vty, "add snmp community failed!\n");
    }
    
    return CMD_SUCCESS;	
}

DEFUN(set_snmp_community_func,
	set_snmp_community_cmd,
	"set community OLDNAME NEWNAME IP MASK (ro|rw) (enable|disable)",
	SETT_STR
	"snmp community \n"
	"community old name \n"
	"community new name \n"
	"community new ipaddr \n"
	"community new mask \n"
	"community read-only \n"
	"community read-write\n"
	"community enable \n"
	"community disable\n"
)
{
    if(0 != dcli_check_snmp_name(argv[0], 1, vty) || 0 != dcli_check_snmp_name(argv[1], 1, vty))
        return CMD_WARNING;

    int ret1 = 0, ret2 = 0;
	unsigned long ip_int = 0;
	unsigned long ip_int1 = 0;
    ret1 = snmp_ipaddr_is_legal_input((char *)argv[2]);
    ret2 = snmp_ipaddr_is_legal_input((char *)argv[3]);
    if((ret1!=1) || (ret2!=1)) {
        vty_out(vty, "IP or MASK doesn't meet format!");
        return CMD_WARNING;
    }
	ip_int = inet_addr((char *)argv[1]);
	ip_int1 = inet_addr((char *)argv[2]);
	if(ip_int & ~ip_int1)
	{
        vty_out(vty, "(IP & ~MASK) must is 0!");
        return CMD_WARNING;
	}
    
    STCommunity communityNode = { 0 };

    strncpy(communityNode.community, argv[1], sizeof(communityNode.community) - 1);
    strncpy(communityNode.ip_addr, argv[2], sizeof(communityNode.ip_addr) - 1);
    strncpy(communityNode.ip_mask, argv[3], sizeof(communityNode.ip_mask) - 1);
    
    unsigned int access_mode = 0;
    if(0 == strcmp(argv[4], "ro")) {
        communityNode.access_mode = 0;
    }
    else if(0 == strcmp(argv[4], "rw")) {
        communityNode.access_mode = 1;
    }
    else {
        vty_out(vty, "Input snmp community permission type error!\n");
        return CMD_WARNING;
    }
    
    unsigned int state = 1;
    if(0 == strcmp(argv[5], "disable")) {
        communityNode.status = 0;
    }
    else if(0 == strcmp(argv[5], "enable")) {
        communityNode.status = 1;
    }
    else {
        vty_out(vty, "Input snmp community status error!\n");
        return CMD_WARNING;
    }
    
    int ret = AC_MANAGE_SUCCESS;
    if(SNMP_NODE == vty->node) {
        if(snmp_cllection_mode(dcli_dbus_connection)) {
            int i = 1;
            for(i = 1; i < MAX_SLOT; i++) {
                if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                    int temp_ret = AC_MANAGE_SUCCESS;
                    temp_ret = ac_manage_config_snmp_set_community(dbus_connection_dcli[i]->dcli_dbus_connection, 
                                                                    argv[0],
                                                                    &communityNode);
                                                                
                    if(AC_MANAGE_SUCCESS == temp_ret) {
                        //vty_out(vty, "Slot(%d) :set snmp community success\n", i);
                    }
                    else if(AC_MANAGE_DBUS_ERROR == temp_ret) {
                        vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
						return CMD_SUCCESS;
                    }
//                    else if(AC_MANAGE_SERVICE_ENABLE == temp_ret) {
//                        vty_out(vty, "Slot(%d) :Snmp service is enable, please disable it first!\n", i);
//                    }
                    else if(AC_MANAGE_CONFIG_NONEXIST == temp_ret) {
                        vty_out(vty, "Slot(%d) :This community is not exist!\n", i);
						return CMD_SUCCESS;
                    }
                    else{
                        vty_out(vty, "Slot(%d) :set snmp community failed!\n", i);
						return CMD_SUCCESS;
                    }
                }
            } 
			vty_out(vty, "Please restart snmp service!\n");
            return CMD_SUCCESS;
        }
        else {
            ret = ac_manage_config_snmp_set_community(dcli_dbus_connection, 
                                                        argv[0],
                                                        &communityNode);
        }	
    }    
	else if(SNMP_SLOT_NODE == vty->node) {
	    
        unsigned int slot_id = vty->slotindex;
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
            
            ret = ac_manage_config_snmp_set_community(dbus_connection_dcli[slot_id]->dcli_dbus_connection, 
                                                        argv[0],
                                                        &communityNode);
        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!\n", slot_id);
            return CMD_WARNING;
        }
	}
	else {
        vty_out(vty, "Terminal mode change must under config mode!\n");
        return CMD_WARNING;
	}

        
    if(AC_MANAGE_SUCCESS == ret) {
        //vty_out(vty, "set snmp community success\n");
    }
    else if(AC_MANAGE_DBUS_ERROR == ret) {
        vty_out(vty, "<error> failed get reply.\n");
		return CMD_SUCCESS;
    }
//    else if(AC_MANAGE_SERVICE_ENABLE == ret) {
//        vty_out(vty, "Snmp service is enable, please disable it first!\n");
//    }
    else if(AC_MANAGE_CONFIG_NONEXIST == ret) {
        vty_out(vty, "This community is not exist!\n");
		return CMD_SUCCESS;
    }
    else{
        vty_out(vty, "set snmp community failed!\n");
		return CMD_SUCCESS;
    }
    vty_out(vty, "Please restart snmp service!\n");
	return CMD_SUCCESS;
}


/*delete snmp  community  by name*/
DEFUN(delete_snmp_community_by_name_func,
	delete_snmp_community_by_name_cmd,
	"delete community name NAME",
	"delete\n"
	"snmp community \n"
	"snmp community key name \n"
	"community name \n"
)
{    
    int ret = AC_MANAGE_SUCCESS;

    if(SNMP_NODE == vty->node) {
        if(snmp_cllection_mode(dcli_dbus_connection)) {
            int i = 1;
            for(i = 1; i < MAX_SLOT; i++) {
                if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                    int temp_ret = AC_MANAGE_SUCCESS;
                    temp_ret = ac_manage_config_snmp_del_community(dbus_connection_dcli[i]->dcli_dbus_connection, argv[0]);
                    
                    if(AC_MANAGE_SUCCESS == temp_ret) {
                        //vty_out(vty, "Slot(%d) :delete snmp community success\n", i);
                    }
                    else if(AC_MANAGE_DBUS_ERROR == temp_ret) {
                        vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
                    }
                    else if(AC_MANAGE_SERVICE_ENABLE == temp_ret) {
                        vty_out(vty, "Slot(%d) :Snmp service is enable, please disable it first!\n", i);
                    }
                    else if(AC_MANAGE_CONFIG_NONEXIST == temp_ret){
                        vty_out(vty, "Slot(%d) :This community is not exist!\n", i);
                    }
                    else{
                        vty_out(vty, "Slot(%d) :delete snmp community failed!\n", i);
                    }
                }
            } 
            return CMD_SUCCESS;
        }
        else {
            ret = ac_manage_config_snmp_del_community(dcli_dbus_connection, argv[0]);
        }                                                    
    }
    else if(SNMP_SLOT_NODE == vty->node) {
        
        unsigned int slot_id = vty->slotindex;
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
            ret = ac_manage_config_snmp_del_community(dbus_connection_dcli[slot_id]->dcli_dbus_connection, argv[0]);
            
        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!\n", slot_id);            
            return CMD_WARNING;
        }   
    }
    else {
        vty_out(vty, "Terminal mode change must under config mode!\n");
        return CMD_WARNING;
    }
    
    if(AC_MANAGE_SUCCESS == ret) {
        //vty_out(vty, "delete snmp community success\n");
    }
    else if(AC_MANAGE_DBUS_ERROR == ret) {
        vty_out(vty, "<error> failed get reply.\n");
    }
    else if(AC_MANAGE_CONFIG_NONEXIST == ret){
        vty_out(vty, "This community is not exist!\n");
    }
    else if(AC_MANAGE_SERVICE_ENABLE == ret) {
        vty_out(vty, "Snmp service is enable, please disable it first!\n");
    }
    else{
        vty_out(vty, "delete snmp community failed!\n");
    }
    
	return CMD_SUCCESS;
}

//create view
DEFUN(create_snmp_view_func,
	create_snmp_view_cmd,
	"create view NAME",
	"create\n"
	"snmp view\n"
	"view name\n"
)
{
    if(0 != dcli_check_snmp_name(argv[0], 0, vty))
        return CMD_WARNING;
    
    int ret = AC_MANAGE_SUCCESS;
    
    if(SNMP_NODE == vty->node) {
        if(snmp_cllection_mode(dcli_dbus_connection)) {
            int i = 1;
            for(i = 1; i < MAX_SLOT; i++) {
                if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                    int temp_ret = AC_MANAGE_SUCCESS;
                    temp_ret = ac_manage_config_snmp_view(dbus_connection_dcli[i]->dcli_dbus_connection, argv[0], 1);
                    
                    if(AC_MANAGE_SUCCESS == temp_ret) {
                        //vty_out(vty, "Slot(%d) :create snmp view %s success\n", i, argv[0]);
                    }
                    else if(AC_MANAGE_DBUS_ERROR == temp_ret) {
                        vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
                    }
                    else if(AC_MANAGE_SERVICE_ENABLE == temp_ret) {
                        vty_out(vty, "Slot(%d) :Snmp service is enable, please disable it first!\n", i);
                    }
                    else if(AC_MANAGE_CONFIG_EXIST == temp_ret){
                        vty_out(vty, "Slot(%d) :The view is exist!\n", i);
                    }
                    else {
                        vty_out(vty, "Slot(%d) :create snmp view failed!\n", i);
                    }
                }
            }     
            return CMD_SUCCESS;
        }
        else {
            ret = ac_manage_config_snmp_view(dcli_dbus_connection, argv[0], 1);
        }    
	}
	else if(SNMP_SLOT_NODE == vty->node){
        unsigned int slot_id = vty->slotindex;
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
             ret = ac_manage_config_snmp_view(dbus_connection_dcli[slot_id]->dcli_dbus_connection, argv[0], 1);
        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!\n", slot_id);
            return CMD_WARNING;
        }   
	}
	else {
        vty_out (vty, "Terminal mode change must under config mode!\n");
        return CMD_WARNING;
	}

	
    if(AC_MANAGE_SUCCESS == ret) {
        //vty_out(vty, "create snmp view %s success\n", argv[0]);
        return CMD_SUCCESS;
    }
    else if(AC_MANAGE_DBUS_ERROR == ret) {
        vty_out(vty, "<error> failed get reply.\n");
    }
    else if(AC_MANAGE_SERVICE_ENABLE == ret) {
        vty_out(vty, "Snmp service is enable, please disable it first!\n");
    }
    else if(AC_MANAGE_CONFIG_EXIST == ret){
        vty_out(vty, "The view is exist!\n");
    }
    else {
        vty_out(vty, "create snmp view failed!\n");
    }

    return CMD_WARNING;
}

char dcli_view_name[MAX_VIEW_NAME_LEN] = { 0 };

DEFUN(conf_snmp_view_func,
	conf_snmp_view_cmd,
	"config view NAME",
	CONFIG_STR
	"config snmp view!\n"
	"snmp view name\n"
)
{
    int ret = AC_MANAGE_SUCCESS;
    if(SNMP_NODE == vty->node) {
        if(snmp_cllection_mode(dcli_dbus_connection)) {
            int i = 1;
            for(i = 1; i < MAX_SLOT; i++) {
                if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                    int ret_temp = AC_MANAGE_SUCCESS;
                    ret_temp = ac_manage_check_snmp_view(dbus_connection_dcli[i]->dcli_dbus_connection, argv[0]);

                    if(AC_MANAGE_SUCCESS != ret_temp) {
                        ret = AC_MANAGE_CONFIG_NONEXIST;
                        vty_out(vty, "Slot(%d) :The view is not exist!\n", i);
                    }
                }
            }
            if(AC_MANAGE_SUCCESS != ret)
                return CMD_WARNING;
        }
        else{
            ret = ac_manage_check_snmp_view(dcli_dbus_connection, argv[0]);
        }    
    }
    else if(SNMP_SLOT_NODE == vty->node) {
    
        unsigned int slot_id = vty->slotindex;
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
             ret = ac_manage_check_snmp_view(dbus_connection_dcli[slot_id]->dcli_dbus_connection, argv[0]);
        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!\n", slot_id);
            return CMD_WARNING;
        }   
    }
	else {
        vty_out(vty, "Terminal mode change must under config mode!\n");
        return CMD_WARNING;
	}

    if(AC_MANAGE_SUCCESS == ret) {
        memset(dcli_view_name, 0, sizeof(dcli_view_name));
        strncpy(dcli_view_name, argv[0], sizeof(dcli_view_name) - 1);

        if(SNMP_SLOT_NODE == vty->node) {
            vty->prenode = SNMP_SLOT_NODE;
        }
        else {
            vty->prenode = SNMP_NODE;
        }
		vty->node = SNMP_VIEW_NODE;
		vty->index = (void *)dcli_view_name;

		return CMD_SUCCESS;
    }
    else{
        vty_out(vty, "The view is not exist!\n");
    }

    return CMD_WARNING;    
}

//add view include/exclude
DEFUN(config_snmp_view_oid_func,
	config_snmp_view_oid_cmd,
	"(add|delete) (view-included|view-excluded) OID",
	"add\n"
	"delete\n"
	"snmp view included\n"
	"snmp view excluded\n"
	"snmp OID\n"
)
{

	if (SNMP_VIEW_NODE != vty->node) {
		vty_out(vty, "Terminal mode change must under snmp-view mode!\n");
		return CMD_WARNING;
	}

    char view_name[MAX_VIEW_NAME_LEN] = { 0 };
    if(NULL != vty->index) {
        strncpy(view_name, vty->index, sizeof(view_name) - 1);
    }

    char oid[MAX_oid] = { 0 };
    if(strlen(argv[2]) >= MAX_oid) {
		vty_out(vty, "The input oid lenth is over MAX_OID!\n");
		return CMD_WARNING;
    }
    strncpy(oid, argv[2], sizeof(oid) - 1);

    unsigned int oid_type = 0;
    if(0 == strcmp(argv[1], "view-included")) {
        oid_type = 1;
    }
    else if(0 == strcmp(argv[1], "view-excluded")){
        oid_type = 0;
    }

    unsigned int mode = 0;
    if(0 == strcmp(argv[0], "add")) {
        mode = 1;
    }
    else if(0 == strcmp(argv[0], "delete")){
        mode = 0;
    }
    else {
        vty_out(vty, "Input snmp view mode error!\n");
        return CMD_WARNING;
    }
    
    int ret = AC_MANAGE_SUCCESS;
    if(SNMP_NODE == vty->prenode) {
        if(snmp_cllection_mode(dcli_dbus_connection)) {
            int i = 1;
            for(i = 1; i < MAX_SLOT; i++) {
                if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                    int temp_ret = AC_MANAGE_SUCCESS;
                    temp_ret = ac_manage_config_snmp_view_oid(dbus_connection_dcli[i]->dcli_dbus_connection, view_name, oid, oid_type, mode);
                    
                    if(AC_MANAGE_SUCCESS == temp_ret) {
                        //vty_out(vty, "Slot(%d) :%s snmp %s oid success\n", i, argv[0], argv[1]);
                    }
                    else if(AC_MANAGE_CONFIG_EXIST == temp_ret) {
                        vty_out(vty, "Slot(%d) :This %s oid is exist!\n", i, argv[1]);
                    }
                    else if(AC_MANAGE_CONFIG_NONEXIST == temp_ret) {
                        vty_out(vty, "Slot(%d) :This %s oid is not exist!\n", i, argv[1]);
                    }
                    else if(AC_MANAGE_CONFIG_REACH_MAX_NUM == temp_ret) {
                        vty_out(vty, "Slot(%d) :snmp %s oid over max num!\n", i, argv[1]);
                    }
                    else if(AC_MANAGE_INPUT_TYPE_ERROR == temp_ret) {
                        vty_out(vty, "Slot(%d) :Input para type is error!\n", i);
                    }
                    else if(AC_MANAGE_SERVICE_ENABLE == temp_ret) {
                        vty_out(vty, "Slot(%d) :Snmp service is enable, please disable it first!\n", i);
                    }
                    else {
                        vty_out(vty, "Slot(%d) :%s snmp %s oid failed!\n", i, argv[0], argv[1]);
                    }
                }
            }
            return CMD_SUCCESS;
        }
        else {
            ret = ac_manage_config_snmp_view_oid(dcli_dbus_connection, view_name, oid, oid_type, mode);
        }    
    }
    else if(SNMP_SLOT_NODE == vty->prenode) {
        unsigned int slot_id = vty->slotindex;
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
            ret = ac_manage_config_snmp_view_oid(dbus_connection_dcli[slot_id]->dcli_dbus_connection, view_name, oid, oid_type, mode);
        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!\n", slot_id);
            return CMD_WARNING;
        }   
    }
    else {
		vty_out(vty, "Terminal prev mode mast snmp mode or snmp slot mode!\n");
		return CMD_WARNING;
    }

    if(AC_MANAGE_SUCCESS == ret) {
        //vty_out(vty, "%s snmp %s oid success\n", argv[0], argv[1]);
    }
    else if(AC_MANAGE_CONFIG_EXIST == ret) {
        vty_out(vty, "This %s oid is exist!\n", argv[1]);
    }
    else if(AC_MANAGE_CONFIG_NONEXIST == ret) {
        vty_out(vty, "This %s oid is not exist!\n", argv[1]);
    }
    else if(AC_MANAGE_CONFIG_REACH_MAX_NUM == ret) {
        vty_out(vty, "snmp %s oid over max num!\n", argv[1]);
    }
    else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) {
        vty_out(vty, "Input para type is error!\n");
    }
    else if(AC_MANAGE_SERVICE_ENABLE == ret) {
        vty_out(vty, "Snmp service is enable, please disable it first!\n");
    }
    else {
        vty_out(vty, "%s snmp %s oid failed!\n", argv[0], argv[1]);
    }
    
	return CMD_SUCCESS;
}


//del view
DEFUN(delete_snmp_view_func,
	delete_snmp_view_cmd,
	"delete view NAME",
	"delete\n"
	"delete view NAME\n"
)

{
    int ret = AC_MANAGE_SUCCESS;
    if(SNMP_NODE == vty->node) {
        if(snmp_cllection_mode(dcli_dbus_connection)) {
            int i = 1;
            for(i = 1; i < MAX_SLOT; i++) {
                if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                    int temp_ret = AC_MANAGE_SUCCESS;
                    temp_ret = ac_manage_config_snmp_view(dbus_connection_dcli[i]->dcli_dbus_connection, argv[0], 0);
                    
                    if(AC_MANAGE_SUCCESS == temp_ret) {
                        //vty_out(vty, "Slot(%d) :delete snmp view %s success\n", i, argv[0]);
                    }    
                    else if(AC_MANAGE_INPUT_TYPE_ERROR == temp_ret) {
                        vty_out(vty, "Slot(%d) :Input para type is error!\n", i);
                    }
                    else if(AC_MANAGE_CONFIG_RELEVANCE == temp_ret) {
                        vty_out(vty, "Slot(%d) :The view(%s) is used by group, please delete group first!\n", i, argv[0]);
                    }
                    else if(AC_MANAGE_DBUS_ERROR == temp_ret) {
                        vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
                    }
                    else if(AC_MANAGE_SERVICE_ENABLE == temp_ret) {
                        vty_out(vty, "Slot(%d) :Snmp service is enable, please disable it first!\n", i);
                    }
                    else if(AC_MANAGE_CONFIG_NONEXIST == temp_ret){
                        vty_out(vty, "Slot(%d) :The view is not exist!\n", i);
                    }
                    else {
                        vty_out(vty, "Slot(%d) :create snmp view failed!\n", i);
                    }
                }
            }   
            return CMD_SUCCESS;
        }
        else {
            ret = ac_manage_config_snmp_view(dcli_dbus_connection, argv[0], 0);
        }    
    }
    else if(SNMP_SLOT_NODE == vty->node) {
        unsigned int slot_id = vty->slotindex;
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
            ret = ac_manage_config_snmp_view(dbus_connection_dcli[slot_id]->dcli_dbus_connection, argv[0], 0);
        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!\n", slot_id);
            return CMD_WARNING;
        }   
    }
    else {
		vty_out (vty, "Terminal mode change must under SNMP mode!\n");
		return CMD_WARNING;
    }
    
    if(AC_MANAGE_SUCCESS == ret) {
        //vty_out(vty, "delete snmp view %s success\n", argv[0]);
    }    
    else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) {
        vty_out(vty, "Input para type is error!\n");
    }
    else if(AC_MANAGE_CONFIG_RELEVANCE == ret) {
        vty_out(vty, "The view(%s) is used by group, please delete group first!\n", argv[0]);
    }
    else if(AC_MANAGE_DBUS_ERROR == ret) {
        vty_out(vty, "<error> failed get reply.\n");
    }
    else if(AC_MANAGE_SERVICE_ENABLE == ret) {
        vty_out(vty, "Snmp service is enable, please disable it first!\n");
    }
    else if(AC_MANAGE_CONFIG_NONEXIST == ret){
        vty_out(vty, "The view is not exist!\n");
    }
    else {
        vty_out(vty, "delete snmp view failed!\n");
    }

	return CMD_SUCCESS;

}


static void
dcli_show_snmp_view_info(DBusConnection *connection, char *view_name, struct vty *vty) {
    int ret = AC_MANAGE_SUCCESS;

    STSNMPView *view_array = NULL;
    unsigned int view_num = 0;

    ret = ac_manage_show_snmp_view(connection, &view_array, &view_num, view_name);    

    if(AC_MANAGE_SUCCESS == ret) {
        vty_out(vty,"===============================================================================\n");
        int i = 0;
		for(i = 0; i < view_num; i++)
		{
			vty_out(vty, "View Name             : %s\n", view_array[i].name);
			
			struct oid_list *oid_node = NULL;
			int j = 0;

			vty_out(vty, "Included item num     : %d\n", view_array[i].view_included.oid_num);
			if(view_array[i].view_included.oid_num > 0)
                vty_out(vty,"-----------------------------------------------------------\n");
                
			for(j = 0, oid_node = view_array[i].view_included.oidHead;
			    j < view_array[i].view_included.oid_num && NULL != oid_node;
			    j++, oid_node = oid_node->next) {

                vty_out(vty, "Included item         : %s\n", oid_node->oid);
			}
            if(view_array[i].view_included.oid_num > 0)
                vty_out(vty,"-----------------------------------------------------------\n");

			vty_out(vty, "Excluded item num     : %d\n", view_array[i].view_excluded.oid_num);
			
			if(view_array[i].view_excluded.oid_num > 0)
                vty_out(vty,"-----------------------------------------------------------\n");
			for(j = 0, oid_node = view_array[i].view_excluded.oidHead;
			    j < view_array[i].view_excluded.oid_num && NULL != oid_node;
			    j++, oid_node = oid_node->next) {

                vty_out(vty, "Excluded item         : %s\n", oid_node->oid);
			}
            if(view_array[i].view_excluded.oid_num > 0)
                vty_out(vty,"-----------------------------------------------------------\n");
                
            vty_out(vty,"===============================================================================\n");
		}

		free_ac_manage_show_snmp_view(&view_array, view_num);
    }
    else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) {
        vty_out(vty, "Input para type is error!n");
    }
    else if(AC_MANAGE_CONFIG_NONEXIST == ret) {
        if(view_name) {
            vty_out(vty, "This view is not exist!\n");
        }
        else {
            vty_out(vty, "There is no view config!\n");
        }
    }
    else if(AC_MANAGE_MALLOC_ERROR == ret) {
        vty_out(vty, "malloc error !\n");
    }
    else if(AC_MANAGE_DBUS_ERROR == ret) {
        vty_out(vty, "<error> failed get reply.\n");
    }
    else {
        vty_out(vty, "show snmp view failed!\n");
    }

    return ;
}


DEFUN(show_view_info_func,
	show_view_info_cmd,
	"show view [NAME]",
	SHOW_STR
	"snmp view configure\n"
	"configure inforamtion\n"
)
{
    char *view_name = NULL;
    if(1 == argc) {
        view_name = (char *)malloc(MAX_VIEW_NAME_LEN);
        if(NULL == view_name) {
            vty_out(vty, "malloc error !\n");                
            return CMD_SUCCESS;
        }
        memset(view_name, 0, MAX_VIEW_NAME_LEN);
        strncpy(view_name, argv[0], MAX_VIEW_NAME_LEN - 1);
    }
    else if(argc > 1){
        vty_out(vty, "Input para is error!\n");
        return CMD_SUCCESS;
    }
    
    if(SNMP_SLOT_NODE == vty->node) {   
        unsigned int slot_id = vty->slotindex;
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
            dcli_show_snmp_view_info(dbus_connection_dcli[slot_id]->dcli_dbus_connection, view_name, vty);
        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!\n", slot_id);
            return CMD_WARNING;
        }   
    }
    else {
         if(snmp_cllection_mode(dcli_dbus_connection)) {
            
            vty_out(vty, "*******************************************************************************\n");
        
            int i = 1; 
            for(i = 1; i < MAX_SLOT; i++) {
                if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                    
                    vty_out(vty, "SNMP SLOT             : %d \n", i );  
                    
                    dcli_show_snmp_view_info(dbus_connection_dcli[i]->dcli_dbus_connection, view_name, vty);
        
                    vty_out(vty, "*******************************************************************************\n");
                }
            }
        }       
        else {
            dcli_show_snmp_view_info(dcli_dbus_connection, view_name, vty);
        }
    }
    
    MANAGE_FREE(view_name);
    return CMD_SUCCESS;
}

//add group
DEFUN(add_snmp_group_func,
	add_snmp_group_cmd,
	"add group NAME (all|VIEWNAME) (ro|rw) (noauth|auth|priv)",
	"add\n"
	"snmp group \n"
	"group name \n"
	"include all oids \n"
	"view name(You need to create a view first)\n"
	"read-only \n"
	"read-write\n"
	":noauth security level\n"
	":auth security level\n"
	":security level\n"

)
{
    
    if(0 != dcli_check_snmp_name(argv[0], 0, vty))
        return CMD_WARNING;

    STSNMPGroup group_node = { 0 };
    strncpy(group_node.group_name, argv[0], sizeof(group_node.group_name) - 1);
    strncpy(group_node.group_view, argv[1], sizeof(group_node.group_view) - 1);

    if(0 == strcmp(argv[2], "ro")) {
        group_node.access_mode = ACCESS_MODE_RO;
    }
    else if(0 == strcmp(argv[2], "rw")) {
        group_node.access_mode = ACCESS_MODE_RW;
    }
    else {
        vty_out(vty, "Input snmp group permission type error!\n");
        return CMD_WARNING;
    }

    if(0 == strcmp(argv[3], "noauth")) {
        group_node.sec_level = SEC_NOAUTH;
    }
    else if(0 == strcmp(argv[3], "auth")) {
        group_node.sec_level = SEC_AUTH;
    }
    else if(0 == strcmp(argv[3], "priv")) {
        group_node.sec_level = SEC_PRIV;
    }
    else {
        vty_out(vty, "Input snmp group security level error!\n");
        return CMD_WARNING;
    }

    int ret = AC_MANAGE_SUCCESS;
    
    if(SNMP_NODE == vty->node) {
        if(snmp_cllection_mode(dcli_dbus_connection)) {
            int i = 1;
            for(i = 1; i < MAX_SLOT; i++) {
                if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                    int temp_ret = AC_MANAGE_SUCCESS;
                    temp_ret = ac_manage_config_snmp_add_group(dbus_connection_dcli[i]->dcli_dbus_connection, &group_node);
                    
                    if(AC_MANAGE_SUCCESS == temp_ret) {
                        //vty_out(vty, "Slot(%d) :add snmp group success\n", i);
                    }
                    else if(AC_MANAGE_SERVICE_ENABLE == temp_ret) {
                        vty_out(vty, "Slot(%d) :Snmp service is enable, please disable it first!\n", i);
                    }
                    else if(AC_MANAGE_CONFIG_EXIST == temp_ret) {
                        vty_out(vty, "Slot(%d) :The group(%s) is exist!\n", i, group_node.group_name);
                    }
                    else if(AC_MANAGE_CONFIG_NONEXIST == temp_ret) {
                        vty_out(vty, "Slot(%d) :The view(%s) is not exist!\n", i, group_node.group_view);
                    }
                    else if(AC_MANAGE_DBUS_ERROR == temp_ret) {
                        vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
                    }
                    else {
                        vty_out(vty, "Slot(%d) :add snmp group failed!\n", i);
                    }
                }
            }  
            return CMD_SUCCESS;
        }
        else {
            ret = ac_manage_config_snmp_add_group(dcli_dbus_connection, &group_node);
        }    
    }
    else if(SNMP_SLOT_NODE == vty->node) {
        unsigned int slot_id = vty->slotindex;
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
            ret = ac_manage_config_snmp_add_group(dbus_connection_dcli[slot_id]->dcli_dbus_connection, &group_node);
        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!\n", slot_id);
            return CMD_WARNING;
        }   
    }
    else {
		vty_out (vty, "Terminal mode change must under SNMP mode!\n");
        return CMD_WARNING;
    }
    
    if(AC_MANAGE_SUCCESS == ret) {
        //vty_out(vty, "add snmp group success\n");
    }
    else if(AC_MANAGE_SERVICE_ENABLE == ret) {
        vty_out(vty, "Snmp service is enable, please disable it first!\n");
    }
    else if(AC_MANAGE_CONFIG_EXIST == ret) {
        vty_out(vty, "The group(%s) is exist!\n", group_node.group_name);
    }
    else if(AC_MANAGE_CONFIG_NONEXIST == ret) {
        vty_out(vty, "The view(%s) is not exist!\n", group_node.group_view);
    }
    else if(AC_MANAGE_DBUS_ERROR == ret) {
        vty_out(vty, "<error> failed get reply.\n");
    }
    else {
        vty_out(vty, "add snmp group failed!\n");
    }
    
    return CMD_SUCCESS;
}

DEFUN(delete_snmp_group_func,
	delete_snmp_group_cmd,
	"delete group NAME",
	"delete\n"
	"delete group\n"
	"delete group NAME\n"
)
{    
    if(0 != dcli_check_snmp_name(argv[0], 0, vty))
        return CMD_WARNING;

    char *group_name = (char *)malloc(MAX_GROUP_NAME_LEN);
    if(NULL == group_name) {
        vty_out(vty, "MALLOC error!\n");
        return CMD_WARNING;
    }
    memset(group_name, 0, MAX_GROUP_NAME_LEN);
    strncpy(group_name, argv[0], MAX_GROUP_NAME_LEN - 1);

    int ret = AC_MANAGE_SUCCESS;
    
    if(SNMP_NODE == vty->node) {
        if(snmp_cllection_mode(dcli_dbus_connection)) {
            int i = 1;
            for(i = 1; i < MAX_SLOT; i++) {
                if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                    int temp_ret = AC_MANAGE_SUCCESS;
                    temp_ret = ac_manage_config_snmp_del_group(dbus_connection_dcli[i]->dcli_dbus_connection, group_name);
                    
                    if(AC_MANAGE_SUCCESS == temp_ret) {
                        //vty_out(vty, "Slot(%d) :delete snmp group success\n", i);
                    }
                    else if(AC_MANAGE_SERVICE_ENABLE == temp_ret) {
                        vty_out(vty, "Slot(%d) :Snmp service is enable, please disable it first!\n", i);
                    }
                    else if(AC_MANAGE_CONFIG_NONEXIST == temp_ret) {
                        vty_out(vty, "Slot(%d) :The group(%s) is not exist!\n", i, group_name);
                    }
                    else if(AC_MANAGE_CONFIG_RELEVANCE == temp_ret) {
                        vty_out(vty, "Slot(%d) :The group(%s) is used by v3user, please delete v3user first!\n", i, group_name);
                    }
                    else if(AC_MANAGE_DBUS_ERROR == temp_ret) {
                        vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
                    }
                    else {
                        vty_out(vty, "Slot(%d) :delete snmp group failed!\n", i);
                    }
                }
            } 
            goto CMD_END;
        }
        else {
            ret = ac_manage_config_snmp_del_group(dcli_dbus_connection, group_name);
        }    
    }
    else if(SNMP_SLOT_NODE == vty->node) {
        unsigned int slot_id = vty->slotindex;
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
            ret = ac_manage_config_snmp_del_group(dbus_connection_dcli[slot_id]->dcli_dbus_connection, group_name);
        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!\n", slot_id);
            goto CMD_END;
        }   
    }
    else {
		vty_out (vty, "Terminal mode change must under SNMP mode!\n");
		goto CMD_END;
    }
    
    if(AC_MANAGE_SUCCESS == ret) {
        //vty_out(vty, "delete snmp group success\n");
    }
    else if(AC_MANAGE_SERVICE_ENABLE == ret) {
        vty_out(vty, "Snmp service is enable, please disable it first!\n");
    }
    else if(AC_MANAGE_CONFIG_NONEXIST == ret) {
        vty_out(vty, "The group(%s) is not exist!\n", group_name);
    }
    else if(AC_MANAGE_CONFIG_RELEVANCE == ret) {
        vty_out(vty, "The group(%s) is used by v3user, please delete v3user first!\n", group_name);
    }
    else if(AC_MANAGE_DBUS_ERROR == ret) {
        vty_out(vty, "<error> failed get reply.\n");
    }
    else {
        vty_out(vty, "delete snmp group failed!\n");
    }

CMD_END:
    MANAGE_FREE(group_name);

    return CMD_SUCCESS;
}


static void
dcli_show_group_info(DBusConnection *connection, char *group_name, struct vty *vty) {
    
    int ret = AC_MANAGE_SUCCESS;

    STSNMPGroup *group_array = NULL;
    unsigned int group_num = 0;

    ret = ac_manage_show_snmp_group(connection, &group_array, &group_num, group_name);  

    if(AC_MANAGE_SUCCESS == ret) {
		vty_out(vty,"------------------------------------------------------------------------\n");
        vty_out(vty, "  %10s    %8s      %9s    %10s\n", "GroupName", "ViewName", "RO|RW", "Sec_Level");

        int i = 0;
        for (i = 0; i < group_num; i++) {

            char level_str[10] = { 0 };
            if(SEC_NOAUTH== group_array[i].sec_level) {
                strncpy(level_str, "noauth", sizeof(level_str) - 1);
            }
            else if(SEC_AUTH == group_array[i].sec_level) {
                strncpy(level_str, "auth", sizeof(level_str) - 1);
            }
            else if(SEC_PRIV == group_array[i].sec_level){
                strncpy(level_str, "priv", sizeof(level_str) - 1);
            }
			  
            vty_out(vty, "%10s    %8s        %6s    %10s\n", group_array[i].group_name, group_array[i].group_view, 
                            ACCESS_MODE_RO == group_array[i].access_mode ? "ro" : "rw", level_str);
		
		}
		
        MANAGE_FREE(group_array);
    }
    else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) {
        vty_out(vty, "Input para type is error!n");
    }
    else if(AC_MANAGE_CONFIG_NONEXIST == ret) {
        if(group_name) {
            vty_out(vty, "This group (%s) is not exist!\n", group_name);
        }
        else {
            vty_out(vty, "There is no group config!\n");
        }
    }
    else if(AC_MANAGE_MALLOC_ERROR == ret) {
        vty_out(vty, "malloc error !\n");
    }
    else if(AC_MANAGE_DBUS_ERROR == ret) {
        vty_out(vty, "<error> failed get reply.\n");
    }
    else {
        vty_out(vty, "show snmp group failed!\n");
    }

    return ;
}


DEFUN(show_group_info_func,
	show_group_info_cmd,
	"show group [NAME]",
	SHOW_STR
	"snmp group configure\n"
	"configure inforamtion\n"
)
{
    char *group_name = NULL;
    if(1 == argc) {
        group_name = (char *)malloc(MAX_GROUP_NAME_LEN);
        if(NULL == group_name) {
            vty_out(vty, "malloc error !\n");                
            return CMD_WARNING;
        }
        memset(group_name, 0, MAX_GROUP_NAME_LEN);
        strncpy(group_name, argv[0], MAX_GROUP_NAME_LEN - 1);
    }
    else if(argc > 1){
        vty_out(vty, "Input para is error!\n");
        return CMD_WARNING;
    }

    if(SNMP_SLOT_NODE == vty->node) {   
        unsigned int slot_id = vty->slotindex;
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
            dcli_show_group_info(dbus_connection_dcli[slot_id]->dcli_dbus_connection, group_name, vty);
        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!\n", slot_id);
            goto CMD_END;
        }   
    }
    else {
         if(snmp_cllection_mode(dcli_dbus_connection)) {
            
            vty_out(vty,"===============================================================================\n");
        
            int i = 1; 
            for(i = 1; i < MAX_SLOT; i++) {
                if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                    
                    vty_out(vty, "SNMP SLOT             : %d \n", i );  
                    
                    dcli_show_group_info(dbus_connection_dcli[i]->dcli_dbus_connection, group_name, vty);
        
                    vty_out(vty,"===============================================================================\n");
                }
            }
        }       
        else {
            dcli_show_group_info(dcli_dbus_connection, group_name, vty);
        }
    }

CMD_END:    
    MANAGE_FREE(group_name);
    return CMD_SUCCESS;
}


DEFUN(add_snmp_v3user_func,
	add_snmp_v3user_cmd,
	"add v3user NAME (md5|sha) AUTHKEY des PRIVATEKEY GROUPNAME (enable|disable)",
	"add\n"
	"snmp v3user \n"
	"v3user name \n"
	"v3user MD5 auth portocal \n"
	"v3user SHA auth portocal \n"
	"v3user auth key \n"
	"v3user DES  private portocal \n"
	"v3user private key \n"
	"v3user Group name(you need to creat a group first)\n"
	"v3user enable \n"
	"v3user disbale \n"
)
{
    if(3 == argc) {        
        if(0 != dcli_check_snmp_name(argv[0], 0, vty) || 0 != dcli_check_snmp_name(argv[1], 0, vty)) {
            return CMD_WARNING;
        }    
    }    
    else if(5 == argc) {
        if(0 != dcli_check_snmp_name(argv[0], 0, vty) || 0 != dcli_check_snmp_name(argv[3], 0, vty)) {
            return CMD_WARNING;
        }    
        if(strlen(argv[2]) < 8 || strlen(argv[2]) > 20) {
            vty_out(vty, "The v3user auth key lenth is wrong!\n");
            return CMD_WARNING;
        }
    }
    else if(6 == argc) {
        if(0 != dcli_check_snmp_name(argv[0], 0, vty) || 0 != dcli_check_snmp_name(argv[4], 0, vty)) {
            return CMD_WARNING;
        }    
        if(strlen(argv[2]) < 8 || strlen(argv[2]) > 20) {
            vty_out(vty, "The v3user auth key lenth is wrong!\n");
            return CMD_WARNING;
        }
        if(strlen(argv[3]) < 8 || strlen(argv[3]) > 20) {
            vty_out(vty, "The v3user priv key lenth is wrong!\n");
            return CMD_WARNING;
        }
    }
    else {
        vty_out(vty, "Input para is error!\n");
        return CMD_WARNING;
    }

    STSNMPV3User v3user_node = { 0 };
    
    if(3 == argc) {
        strncpy(v3user_node.name, argv[0], sizeof(v3user_node.name) - 1);
        strncpy(v3user_node.group_name, argv[1], sizeof(v3user_node.group_name) - 1);

        v3user_node.authentication.protocal = AUTH_PRO_NONE;
        if(0 == strcmp(argv[2], "enable")) {
            v3user_node.status = RULE_ENABLE;
        }
        else if(0 == strcmp(argv[2], "disable")){
            v3user_node.status = RULE_DISABLE;
        }
        else {
            vty_out(vty, "Input snmp v3user status error!\n");
            return CMD_WARNING;
        }
    }
    else if(5 == argc) {       
        strncpy(v3user_node.name, argv[0], sizeof(v3user_node.name) - 1);
        strncpy(v3user_node.authentication.passwd, argv[2], sizeof(v3user_node.authentication.passwd) - 1);
        strncpy(v3user_node.group_name, argv[3], sizeof(v3user_node.group_name) - 1);

        if(0 == strcmp(argv[1], "md5")) {
            v3user_node.authentication.protocal = AUTH_PRO_MD5;
        }
        else if(0 == strcmp(argv[1], "sha")) {
            v3user_node.authentication.protocal = AUTH_PRO_SHA;
        }
        else {
            vty_out(vty, "Input snmp v3user auth portocal error!\n");
            return CMD_WARNING;
        }
        
        v3user_node.privacy.protocal = PRIV_PRO_NONE;

        if(0 == strcmp(argv[4], "enable")) {
            v3user_node.status = RULE_ENABLE;
        }
        else if(0 == strcmp(argv[4], "disable")){
            v3user_node.status = RULE_DISABLE;
        }        
        else {
            vty_out(vty, "Input snmp v3user status error!\n");
            return CMD_WARNING;
        }
    }
    else if(6 == argc) {
        strncpy(v3user_node.name, argv[0], sizeof(v3user_node.name) - 1);
        strncpy(v3user_node.authentication.passwd, argv[2], sizeof(v3user_node.authentication.passwd) - 1);
        strncpy(v3user_node.privacy.passwd, argv[3], sizeof(v3user_node.privacy.passwd) - 1);
        strncpy(v3user_node.group_name, argv[4], sizeof(v3user_node.group_name) - 1);

        if(0 == strcmp(argv[1], "md5")) {
            v3user_node.authentication.protocal = AUTH_PRO_MD5;
        }
        else if(0 == strcmp(argv[1], "sha")) {
            v3user_node.authentication.protocal = AUTH_PRO_SHA;
        }
        else {
            vty_out(vty, "Input snmp v3user auth portocal error!\n");
            return CMD_WARNING;
        }
        
        v3user_node.privacy.protocal = PRIV_PRO_DES;

        if(0 == strcmp(argv[5], "enable")) {
             v3user_node.status = RULE_ENABLE;
        }
        else if(0 == strcmp(argv[5], "disable")){
             v3user_node.status = RULE_DISABLE;
        }
        else {
            vty_out(vty, "Input snmp v3user status error!\n");
            return CMD_WARNING;
        }
    }

    int ret = AC_MANAGE_SUCCESS;

    if(SNMP_NODE == vty->node) {
        if(snmp_cllection_mode(dcli_dbus_connection)) {
            int i = 1;
            for(i = 1; i < MAX_SLOT; i++) {
                if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                    int temp_ret = AC_MANAGE_SUCCESS;
                    temp_ret = ac_manage_config_snmp_add_v3user(dbus_connection_dcli[i]->dcli_dbus_connection, &v3user_node);
                    if(AC_MANAGE_SUCCESS == temp_ret) {
                        //vty_out(vty, "Slot(%d) :add snmp v3user success\n", i);
                    }
                    else if(AC_MANAGE_SERVICE_ENABLE == temp_ret) {
                        vty_out(vty, "Slot(%d) :Snmp service is enable, please disable it first!\n", i);
                    }
                    else if(AC_MANAGE_CONFIG_EXIST == temp_ret) {
                        vty_out(vty, "Slot(%d) :The v3user(%s) is exist!\n", i, v3user_node.name);
                    }
                    else if(AC_MANAGE_CONFIG_NONEXIST == temp_ret) {
                        vty_out(vty, "Slot(%d) :The group(%s) is not exist!\n", i, v3user_node.group_name);
                    }
                    else if(AC_MANAGE_DBUS_ERROR == temp_ret) {
                        vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
                    }
                    else {
                        vty_out(vty, "Slot(%d) :add snmp v3user failed!\n", i);
                    }
                }
            }  
            return CMD_SUCCESS;
        }
        else {
            ret = ac_manage_config_snmp_add_v3user(dcli_dbus_connection, &v3user_node);
        }    
    }
    else if(SNMP_SLOT_NODE == vty->node) {
        unsigned int slot_id = vty->slotindex;
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
            ret = ac_manage_config_snmp_add_v3user(dbus_connection_dcli[slot_id]->dcli_dbus_connection, &v3user_node);
        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!\n", slot_id);
            return CMD_WARNING;
        }   
    }
    else {
		vty_out (vty, "Terminal mode change must under SNMP mode!\n");
        return CMD_WARNING;
    }

    if(AC_MANAGE_SUCCESS == ret) {
        //vty_out(vty, "add snmp v3user success\n");
    }
    else if(AC_MANAGE_SERVICE_ENABLE == ret) {
        vty_out(vty, "Snmp service is enable, please disable it first!\n");
    }
    else if(AC_MANAGE_CONFIG_EXIST == ret) {
        vty_out(vty, "The v3user(%s) is exist!\n", v3user_node.name);
    }
    else if(AC_MANAGE_CONFIG_NONEXIST == ret) {
        vty_out(vty, "The group(%s) is not exist!\n", v3user_node.group_name);
    }
    else if(AC_MANAGE_DBUS_ERROR == ret) {
        vty_out(vty, "<error> failed get reply.\n");
    }
    else {
        vty_out(vty, "add snmp v3user failed!\n");
    }

    return CMD_SUCCESS;
}


ALIAS(add_snmp_v3user_func,
	add_snmp_v3user_none_auth_cmd,
	"add v3user NAME none GROUPNAME (enable|disable)",
	"add\n"
	"snmp v3user \n"
	"v3user name \n"
	"v3user none auth portocal \n"
	"v3user Group name(you need to creat a group first)\n"
	"v3user enable \n"
	"v3user disbale \n"
)

ALIAS(add_snmp_v3user_func,
	add_snmp_v3user_none_private_cmd,
	"add v3user NAME (md5|sha) AUTHKEY none GROUPNAME (enable|disable)",
	"add\n"
	"snmp v3user \n"
	"v3user name \n"
	"v3user MD5 auth portocal \n"
	"v3user SHA auth portocal \n"
	"v3user auth key \n"
	"v3user none private portocal \n"
	"v3user Group name(you need to creat a group first)\n"
	"v3user enable \n"
	"v3user disbale \n"
)

/*delete snmp  v3user  by name*/
DEFUN(delete_snmp_v3user_func,
	delete_snmp_v3user_cmd,
	"delete v3user NAME",
	"delete\n"
	"snmp v3user \n"
	"snmp v3user NAME \n"
)
{
    if(0 != dcli_check_snmp_name(argv[0], 0, vty)) {
        return CMD_WARNING;
    } 

    char *v3user_name = (char *)malloc(MAX_SNMP_NAME_LEN);
    if(NULL == v3user_name) {
        vty_out(vty, "MALLOC error!\n");
        return CMD_WARNING;
    }
    memset(v3user_name, 0, MAX_SNMP_NAME_LEN);
    strncpy(v3user_name, argv[0], MAX_SNMP_NAME_LEN - 1);

    int ret = AC_MANAGE_SUCCESS;

    if(SNMP_NODE == vty->node) {
        if(snmp_cllection_mode(dcli_dbus_connection)) {
            int i = 1;
            for(i = 1; i < MAX_SLOT; i++) {
                if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                    int temp_ret = AC_MANAGE_SUCCESS;
                    temp_ret = ac_manage_config_snmp_del_v3user(dbus_connection_dcli[i]->dcli_dbus_connection, v3user_name);
                    
                    if(AC_MANAGE_SUCCESS == temp_ret) {
                        //vty_out(vty, "Slot(%d) :delete snmp v3user success\n", i);
                    }
                    else if(AC_MANAGE_SERVICE_ENABLE == temp_ret) {
                        vty_out(vty, "Slot(%d) :Snmp service is enable, please disable it first!\n", i);
                    }
                    else if(AC_MANAGE_CONFIG_NONEXIST == temp_ret) {
                        vty_out(vty, "Slot(%d) :The v3user(%s) is not exist!\n", i, v3user_name);
                    }
                    else if(AC_MANAGE_DBUS_ERROR == temp_ret) {
                        vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
                    }
                    else {
                        vty_out(vty, "Slot(%d) :delete snmp v3user failed!\n", i);
                    }
                }
            }  
            goto CMD_END;
        }
        else {
            ret = ac_manage_config_snmp_del_v3user(dcli_dbus_connection, v3user_name);
        }    
    }
    else if(SNMP_SLOT_NODE == vty->node) {
        unsigned int slot_id = vty->slotindex;
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
            ret = ac_manage_config_snmp_del_v3user(dbus_connection_dcli[slot_id]->dcli_dbus_connection, v3user_name);
        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!\n", slot_id);
            goto CMD_END;
        }   
    }
    else {
		vty_out (vty, "Terminal mode change must under SNMP mode!\n");
        goto CMD_END;
    }

    if(AC_MANAGE_SUCCESS == ret) {
        //vty_out(vty, "delete snmp v3user success\n");
    }
    else if(AC_MANAGE_SERVICE_ENABLE == ret) {
        vty_out(vty, "Snmp service is enable, please disable it first!\n");
    }
    else if(AC_MANAGE_CONFIG_NONEXIST == ret) {
        vty_out(vty, "The v3user(%s) is not exist!\n", v3user_name);
    }
    else if(AC_MANAGE_DBUS_ERROR == ret) {
        vty_out(vty, "<error> failed get reply.\n");
    }
    else {
        vty_out(vty, "delete snmp v3user failed!\n");
    }

CMD_END:
    MANAGE_FREE(v3user_name);

    return CMD_SUCCESS;
}

static void
dcli_show_v3user_info(DBusConnection *connection, char *v3user_name, struct vty *vty) {
    
    int ret = AC_MANAGE_SUCCESS;

    STSNMPV3User *v3user_array = NULL;
    unsigned int v3user_num = 0;

    ret = ac_manage_show_snmp_v3user(connection, &v3user_array, &v3user_num, v3user_name);  

    if(AC_MANAGE_SUCCESS == ret) {
		vty_out(vty,"------------------------------------------------------------------------\n");
		vty_out(vty, "  %10s    %9s    %12s       %9s     %6s \n", "V3UserName", "AuthProto", "PrivateProto", "GroupName", "Status");

        int i = 0;
        for (i = 0; i < v3user_num; i++) {
            char auth_pro[10] = { 0 };
            char pri_pro[10] = { 0 };
			
			/*auth protocal*/
			 if(AUTH_PRO_NONE == v3user_array[i].authentication.protocal)
			 {
			 	strncpy(auth_pro, "None", sizeof(auth_pro) - 1);
			 }
			 else if(AUTH_PRO_MD5 == v3user_array[i].authentication.protocal)
			 {
			 	strncpy(auth_pro, "MD5", sizeof(auth_pro) - 1);
			 }
			 else if(AUTH_PRO_SHA == v3user_array[i].authentication.protocal)
			 {
			 	strncpy(auth_pro, "SHA", sizeof(auth_pro) - 1);
			 }

			 /*pricate protocal*/	 
			 if(PRIV_PRO_NONE == v3user_array[i].privacy.protocal)
			 {
				 strncpy(pri_pro, "None", sizeof(pri_pro) - 1);
			 }
			 else if(PRIV_PRO_DES == v3user_array[i].privacy.protocal)
			 {
				 strncpy(pri_pro, "DES", sizeof(pri_pro) - 1);
			 }
			 else if(PRIV_PRO_AES == v3user_array[i].privacy.protocal)
			 {
				 strncpy(pri_pro, "AES", sizeof(pri_pro) - 1);
			 }

			  
			vty_out(vty, "%10s    %8s      %8s	    %10s      %8s\n", 
			                v3user_array[i].name, auth_pro, pri_pro,  v3user_array[i].group_name,
							v3user_array[i].status ? "enable":"disable");
		
		}
		
        MANAGE_FREE(v3user_array);
    }
    else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) {
        vty_out(vty, "Input para type is error!n");
    }
    else if(AC_MANAGE_CONFIG_NONEXIST == ret) {
        if(v3user_name) {
            vty_out(vty, "This v3user(%s) is not exist!\n", v3user_name);
        }
        else {
            vty_out(vty, "There is no v3user config!\n");
        }
    }
    else if(AC_MANAGE_MALLOC_ERROR == ret) {
        vty_out(vty, "malloc error !\n");
    }
    else if(AC_MANAGE_DBUS_ERROR == ret) {
        vty_out(vty, "<error> failed get reply.\n");
    }
    else {
        vty_out(vty, "show snmp v3user failed!\n");
    }

    return ;
}

DEFUN(show_v3user_info_func,
	show_v3user_info_cmd,
	"show v3user [NAME]",
	SHOW_STR
	"snmp v3user configure\n"
	"configure inforamtion\n"
)
{
    char *v3user_name = NULL;
    if(1 == argc) {
        v3user_name = (char *)malloc(MAX_SNMP_NAME_LEN);
        if(NULL == v3user_name) {
            vty_out(vty, "malloc error !\n");                
            return CMD_WARNING;
        }
        memset(v3user_name, 0, MAX_SNMP_NAME_LEN);
        strncpy(v3user_name, argv[0], MAX_SNMP_NAME_LEN - 1);
    }
    else if(argc > 1){
        vty_out(vty, "Input para is error!\n");
        return CMD_WARNING;
    }

    if(SNMP_SLOT_NODE == vty->node) {   
        unsigned int slot_id = vty->slotindex;
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
            dcli_show_v3user_info(dbus_connection_dcli[slot_id]->dcli_dbus_connection, v3user_name, vty);
        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!\n", slot_id);
            return CMD_WARNING;
        }   
    }
    else {
         if(snmp_cllection_mode(dcli_dbus_connection)) {
            
            vty_out(vty,"===============================================================================\n");
        
            int i = 1; 
            for(i = 1; i < MAX_SLOT; i++) {
                if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                    
                    vty_out(vty, "SNMP SLOT             : %d \n", i );  
                    
                    dcli_show_v3user_info(dbus_connection_dcli[i]->dcli_dbus_connection, v3user_name, vty);
        
                    vty_out(vty,"===============================================================================\n");
                }
            }
        }       
        else {
            dcli_show_v3user_info(dcli_dbus_connection, v3user_name, vty);
        }
    }
    
    MANAGE_FREE(v3user_name);
    return CMD_SUCCESS;
}

DEFUN(conf_acmanage_log_debug_func,
	conf_acmanage_log_debug_cmd,
	"(open|close) acmanage log debug",
	"open acmanage log debug\n"
	"close acmanage log debug\n"
	"acmanage log debug\n"
	"log debug\n"
)
{    
    unsigned int log_state = 0;
    if(0 == strcmp(argv[0], "open")) {
        log_state = 1;
    }

    unsigned int ret = AC_MANAGE_SUCCESS;

    ret = ac_manage_config_log_debug(dcli_dbus_connection, log_state);

    if(AC_MANAGE_SUCCESS == ret) {
        vty_out(vty, "%s acmanage log debug success\n", argv[0]);
    }
    else if(AC_MANAGE_DBUS_ERROR == ret) {
        vty_out(vty, "<error> failed get reply.\n");
    }
    else {
        vty_out(vty, "%s acmanage log debug failed!\n", argv[0]);
    }

	return CMD_SUCCESS;
}

DEFUN(conf_acmanage_token_debug_func,
	conf_acmanage_token_debug_cmd,
	"(open|close) acmanage debug (init|session|transport|task|tipc)",
	"open acmanage token debug\n"
	"close acmanage token debug\n"
	"acmanage token debug\n"
	"token debug\n"
	"init debug\n"
	"session debug\n"
	"transport debug\n"
	"task debug\n"
	"tipc debug\n"
)
{    
	unsigned int token, state = 0;

	if(0 == strcmp(argv[0], "open")) {
		state = 1;
	}

	if(0 == strcmp(argv[1], "init")) {
		token = LOG_TOKEN_INIT;
	} else if(0 == strcmp(argv[1], "session")) {
		token = LOG_TOKEN_SESSION;
	} else if(0 == strcmp(argv[1], "transport")) {
		token = LOG_TOKEN_TRANSPORT;
	} else if(0 == strcmp(argv[1], "task")) {
		token = LOG_TOKEN_TASK;
	} else if(0 == strcmp(argv[1], "tipc")) {
		token = LOG_TOKEN_TIPC;
	} else {
		vty_out(vty, "input para %s error\n", argv[1]);
		return CMD_WARNING;
	}
	
	unsigned int ret = AC_MANAGE_SUCCESS;

	ret = ac_manage_config_token_debug(dcli_dbus_connection, token, state);
	if(AC_MANAGE_SUCCESS == ret) {
		vty_out(vty, "%s acmanage debug %s success\n", argv[0], argv[1]);
	}
	else if(AC_MANAGE_DBUS_ERROR == ret) {
		vty_out(vty, "<error> failed get reply.\n");
	}
	else {
		vty_out(vty, "%s acmanage debug %s failed!\n", argv[0], argv[1]);
	}

	return CMD_SUCCESS;
}


DEFUN(conf_snmp_log_debug_func,
	conf_snmp_log_debug_cmd,
	"(open|close) snmp log (debug|source-debug)",
	CONFIG_STR
	"open snmp log \n"
	"close snmp log \n"
	"snmp dbus connection list debug\n"
	"snmp dbus connection list and source code debug\n"
)
{    
    unsigned int debugLevel = 0;
    if(0 == strcmp(argv[0], "open")) {
        if(0 == strcmp(argv[1], "debug")) {
            debugLevel = 1;
        }
        else if(0 == strcmp(argv[1], "source-debug")) {
            debugLevel = 2;
        }
    }

    unsigned int ret = AC_MANAGE_SUCCESS;
    ret = ac_manage_config_snmp_log_debug(dcli_dbus_connection, debugLevel);
    if(AC_MANAGE_SUCCESS == ret) {
        dbus_config_snmp_log_debug(dcli_dbus_connection, debugLevel);
        vty_out(vty, "%s snmp log %s success\n", argv[0], argv[1]);
    }
    else if(AC_MANAGE_DBUS_ERROR == ret) {
        vty_out(vty, "<error> failed get reply.\n");
    }
    else {
        vty_out(vty, "%s snmp log %s failed!\n", argv[0], argv[1]);
    }

	return CMD_SUCCESS;
}


DEFUN(conf_trap_log_debug_func,
	conf_trap_log_debug_cmd,
	"(open|close) trap-helper log (debug|source-debug)",
	CONFIG_STR
	"open trap-helper log \n"
	"close trap-helper log \n"
	"trap-helper debug\n"
	"trap-helper and net-snmp source code debug\n"
)
{    
    unsigned int debugLevel = 0;
    if(0 == strcmp(argv[0], "open")) {
        if(0 == strcmp(argv[1], "debug")) {
            debugLevel = 1;
        }
        else if(0 == strcmp(argv[1], "source-debug")) {
            debugLevel = 2;
        }
    }

    unsigned int ret = AC_MANAGE_SUCCESS;
    
    ret = ac_manage_config_trap_log_debug(dcli_dbus_connection, debugLevel);

    if(AC_MANAGE_SUCCESS == ret) {
        vty_out(vty, "%s acmanage log %s success\n", argv[0], argv[1]);
    }
    else if(AC_MANAGE_DBUS_ERROR == ret) {
        vty_out(vty, "<error> failed get reply.\n");
    }
    else {
        vty_out(vty, "%s acmanage log %s failed!\n", argv[0], argv[1]);
    }

	return CMD_SUCCESS;
}


int
ac_manage_show_snmp_running_config(DBusConnection *connection, unsigned int mode, struct vty *vty) {
    
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	int ret = AC_MANAGE_SUCCESS;
	unsigned int moreConfig = 0;
	char *showStr = NULL;

	query = dbus_message_new_method_call(AC_MANAGE_DBUS_DBUSNAME,
										AC_MANAGE_SNMP_DBUS_OBJPATH,
										AC_MANAGE_SNMP_DBUS_INTERFACE,
										AC_MANAGE_DBUS_SHOW_SNMP_RUNNING_CONFIG);

    dbus_error_init(&err);
    
    dbus_message_append_args(query,
                            DBUS_TYPE_UINT32, &mode,
                            DBUS_TYPE_INVALID);

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


DEFUN(set_trap_service_func,
	set_trap_service_cmd,
	"trap service (enable|disable)",
	"snmp trap\n"
	"start or stop trap service\n"
	"enable, start\n"
	"disable, stop\n"
)
{
    unsigned int state = 0;
    if(0 == strcmp(argv[0], "enable")) {
        state = 1;
    }
    else if(0 == strcmp(argv[0], "disable")) {
        state = 0;
    }
    else {
        vty_out(vty, "Input trap service status is error!\n");
        return CMD_WARNING;
    }

    if(SNMP_NODE == vty->node) {
        int i = 1;
        for(i = 1; i < MAX_SLOT; i++) {
            if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                int ret = AC_MANAGE_SUCCESS;
                ret = ac_manage_config_trap_service(dbus_connection_dcli[i]->dcli_dbus_connection, state); 

                if(AC_MANAGE_SUCCESS == ret) {
                    //vty_out(vty, "Slot(%d) :trap service %s success\n", i, state ? "enable" : "disable");
                }
                else if(AC_MANAGE_DBUS_ERROR == ret) {
                    vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
                }
                else{
                    vty_out(vty, "Slot(%d) :trap service %s failed!\n", i, state ? "enable" : "disable");
                }
            }
        }       
    }
    else if(SNMP_SLOT_NODE == vty->node){
        unsigned int slot_id = vty->slotindex;
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
            int ret = AC_MANAGE_SUCCESS;
            ret = ac_manage_config_trap_service(dbus_connection_dcli[slot_id]->dcli_dbus_connection, state);

            if(AC_MANAGE_SUCCESS == ret) {
                //vty_out(vty, "trap service %s success\n", state ? "enable" : "disable");
            }
            else if(AC_MANAGE_DBUS_ERROR == ret) {
                vty_out(vty, "<error> failed get reply.\n");
            }
            else{
                vty_out(vty, "trap service %s failed!\n", state ? "enable" : "disable");
            }
        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!\n", slot_id);
        }   
    }
    else {
        vty_out (vty, "Terminal mode change must under SNMP or SNMP_SLOT mode!\n");
    }
   
    return CMD_SUCCESS;
}


DEFUN(config_snmp_trap_bind_local_receiver_func,
	config_trap_bind_local_receiver_v1v2c_cmd,
	"(add|set) trap NAME (local-hansi|remote-hansi) A-B (v1|v2c) souraddr A.B.C.D destaddr A.B.C.D COMMUNITY (enable|disable) [<1-65535>]",
	"add\n"
	"set\n"
	"snmp trap\n"
	"trap name\n"
	"trap Local hansi\n"
	"trap Remote hansi\n"
	"trap instance id\n"
	"snmp version v1\n"
	"snmp version v2c\n"
	"trap source address\n"
	"source ip address\n"
	"trap destination address\n"
	"destination ip address\n"
	"trap community\n"
	"trap enable\n"
	"trap disable\n"
	"trap port number, default 162\n"
)
{
    unsigned int slot_id = 0;
    
    STSNMPTrapReceiver receiver;
    memset(&receiver, 0, sizeof(STSNMPTrapReceiver));

    unsigned int type = 0;
    if(0 == strcmp("add", argv[0])) {
        type = 1;
    }
    else if(0 == strcmp("set", argv[0])) {
        type = 0;
    }
    else {
        vty_out(vty, "Input trap receiver mode is error!\n");
        return CMD_WARNING;
    }
    
    if(1 != trap_name_is_legal_input(argv[1])) {
		vty_out(vty, "error name input : %s\n", argv[1]);
		return CMD_SUCCESS;
    }
    strncpy(receiver.name, argv[1], sizeof(receiver.name) - 1);
    
    if(0 == strcmp(argv[2], "local-hansi")) {
        receiver.local_id = 1;
    }
    else if(0 == strcmp(argv[2], "remote-hansi")) {
        receiver.local_id = 0;
    }
    else {
        vty_out(vty, "Input trap receiver instance type is error!\n");
        return CMD_WARNING;
    }

    if(0 == sscanf(argv[3], "%d-%d", &slot_id, &(receiver.instance_id))) {
		vty_out(vty, "input instance (%s) type is error\n", argv[3]);
        return CMD_WARNING;
    }

    if(0 == slot_id || slot_id > 16) {
		vty_out(vty, "error slot id input : %s\n", argv[3]);
        return CMD_WARNING;
    }
    else if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		vty_out(vty, "The slot %d is not connect\n", slot_id);
        return CMD_WARNING;
    }
    
    if(0 == receiver.instance_id || receiver.instance_id > 16) {
		vty_out(vty, "error instnace id input : %s\n", argv[3]);
        return CMD_WARNING;
    }
    
    if(9 == argc || 10 == argc) {
        if(0 == strcmp(argv[4], "v1")) {
            receiver.version = 1;
        }
        else if(0 == strcmp(argv[4], "v2c")) {
            receiver.version = 2;
        }
        else {
            vty_out(vty, "Input trap receiver version mode error!\n");
            return CMD_WARNING;
        }

        if(1 != snmp_ipaddr_is_legal_input(argv[5])) {
            vty_out(vty, "error source ipaddr input : %s\n", argv[5]);
            return CMD_WARNING;
        }
        strncpy(receiver.sour_ipAddr, argv[5], sizeof(receiver.sour_ipAddr) - 1);
        
        if(1 != snmp_ipaddr_is_legal_input(argv[6])) {
            vty_out(vty, "error dest ipaddr input : %s\n", argv[6]);
            return CMD_WARNING;
        }
        strncpy(receiver.dest_ipAddr, argv[6], sizeof(receiver.dest_ipAddr) - 1);
        
        if(dcli_check_snmp_name(argv[7], 1, vty)) {
            return CMD_WARNING;
        }
        strncpy(receiver.trapcom, argv[7], sizeof(receiver.trapcom) - 1);
        
        if(0 == strcmp(argv[8], "enable")) {
            receiver.status = 1;
        }
        else if( 0 == strcmp(argv[8], "disable")) {
            receiver.status = 0;
        }
        else {
            vty_out(vty, "Input trap receiver status error!\n");
            return CMD_WARNING;
        }

        if(10 == argc) {
            receiver.dest_port = atoi(argv[9]);
        }
    }
    else if(7 == argc || 8 == argc) {
        receiver.version = 3;

        if(1 != snmp_ipaddr_is_legal_input(argv[4])) {
            vty_out(vty, "error source ipaddr input : %s\n", argv[4]);
            return CMD_SUCCESS;
        }
        strncpy(receiver.sour_ipAddr, argv[4], sizeof(receiver.sour_ipAddr) - 1);
        
        if(1 != snmp_ipaddr_is_legal_input(argv[5])) {
            vty_out(vty, "error dest ipaddr input : %s\n", argv[5]);
            return CMD_SUCCESS;
        }
        strncpy(receiver.dest_ipAddr, argv[5], sizeof(receiver.dest_ipAddr) - 1);
        
        if(0 == strcmp(argv[6], "enable")) {
            receiver.status = 1;
        }
        else if( 0 == strcmp(argv[6], "disable")) {
            receiver.status = 0;
        }        
        else {
            vty_out(vty, "Input trap receiver status error!\n");
            return CMD_WARNING;
        }

        if(8 == argc) {
            receiver.dest_port = atoi(argv[7]);
        }
        
    }
    else {
        vty_out(vty, "Input prara is error!\n");
        return CMD_SUCCESS;
    }

    if(SNMP_NODE == vty->node){
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
            int ret = AC_MANAGE_SUCCESS;
            ret = ac_manage_config_trap_config_receiver(dbus_connection_dcli[slot_id]->dcli_dbus_connection, &receiver,type);
            if(AC_MANAGE_SUCCESS == ret) {
                //vty_out(vty, "%s trap receiver success\n", type ? "add" : "set");
            }
            else if(AC_MANAGE_DBUS_ERROR == ret) {
                vty_out(vty, "<error> failed get reply.\n");
            }
            else if(AC_MANAGE_SERVICE_ENABLE == ret) {
                vty_out(vty, "Trap service is enable, please disable it first!\n");
            }
            else if(AC_MANAGE_CONFIG_EXIST == ret) {
                vty_out(vty, "This trap receiver is exist!\n");
            }
            else if(AC_MANAGE_CONFIG_NONEXIST == ret) {
                vty_out(vty, "This trap receiver is not exist!\n");
            }
            else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) {
                vty_out(vty, "input type is error!\n");
            }
            else{
                vty_out(vty, "%s trap receiver failed!\n", type ? "add" : "set");
           }                                                  
        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!\n", slot_id);
        }   
    }
    else {
		vty_out (vty, "Terminal mode change must under SNMP mode!\n");
    }

    return CMD_SUCCESS;
}




ALIAS(config_snmp_trap_bind_local_receiver_func,
	config_trap_bind_local_receiver_v3_cmd,
	"(add|set) trap NAME (local-hansi|remote-hansi) A-B v3 souraddr A.B.C.D destaddr A.B.C.D (enable|disable) [<1-65535>]",
	"add\n"
	"set\n"
	"snmp trap\n"
	"trap name\n"
	"trap Local hansi\n"
	"trap Remote hansi\n"
	"trap instance id\n"
	"snmp version v3\n"
	"trap source address"
	"source ip address\n"
	"trap destination address\n"
	"destination ip address\n"
	"trap enable\n"
	"trap disable\n"
	"trap destination port, default 162\n"
)

DEFUN(config_snmp_trap_receiver_func,
	config_trap_receiver_v1v2c_cmd,
	"(add|set) trap NAME (local-hansi|remote-hansi) A-B (v1|v2c) destaddr A.B.C.D COMMUNITY (enable|disable) [<1-65535>]",
	"add\n"
	"set\n"
	"snmp trap\n"
	"trap name\n"
	"trap Local hansi\n"
	"trap Remote hansi\n"
	"trap instance id\n"
	"snmp version v1\n"
	"snmp version v2c\n"
	"trap destination address\n"
	"destination ip address\n"
	"trap community\n"
	"trap enable\n"
	"trap disable\n"
	"trap destination port, default 162\n"
)
{
    unsigned int slot_id = 0;
    
    STSNMPTrapReceiver receiver;
    memset(&receiver, 0, sizeof(STSNMPTrapReceiver));

    unsigned int type = 0;
    if(0 == strcmp("add", argv[0])) {
        type = 1;
    }
    else if(0 == strcmp("set", argv[0])) {
        type = 0;
    }    
    else {
        vty_out(vty, "Input trap receiver mode is error!\n");
        return CMD_WARNING;
    }
    
    if(1 != trap_name_is_legal_input(argv[1])) {
		vty_out(vty, "error name input : %s\n", argv[1]);
		return CMD_WARNING;
    }
    strncpy(receiver.name, argv[1], sizeof(receiver.name) - 1);
    
    if(0 == strcmp(argv[2], "local-hansi")) {
        receiver.local_id= 1;
    }
    else if(0 == strcmp(argv[2], "remote-hansi")) {
        receiver.local_id = 0;
    }
    else {
        vty_out(vty, "Input trap receiver instance type is error!\n");
        return CMD_WARNING;
    }
    
    if(0 == sscanf(argv[3], "%d-%d", &slot_id, &(receiver.instance_id))) {
		vty_out(vty, "input instance (%s) type is error\n", argv[3]);
        return CMD_WARNING;
    }
    
    if(0 == slot_id || slot_id > 16) {
		vty_out(vty, "error slot id input : %s\n", argv[3]);
        return CMD_WARNING;
    }
    else if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		vty_out(vty, "The slot %d is not connect\n", slot_id);
        return CMD_WARNING;
    }
    
    if(0 == receiver.instance_id || receiver.instance_id > 16) {
		vty_out(vty, "error instnace id input : %s\n", argv[3]);
        return CMD_WARNING;
    }
    
    if(8 == argc || 9 == argc) {
        if(0 == strcmp(argv[4], "v1")) {
            receiver.version = 1;
        }
        else if(0 == strcmp(argv[4], "v2c")) {
            receiver.version = 2;
        }
        else {
            vty_out(vty, "Input trap receiver version mode error!\n");
            return CMD_WARNING;
        }
        
        if(1 != snmp_ipaddr_is_legal_input(argv[5])) {
            vty_out(vty, "error dest ipaddr input : %s\n", argv[5]);
            return CMD_WARNING;
        }
        strncpy(receiver.dest_ipAddr, argv[5], sizeof(receiver.dest_ipAddr) - 1);
        
        if(dcli_check_snmp_name(argv[6], 1, vty)) {
            return CMD_WARNING;
        }
        strncpy(receiver.trapcom, argv[6], sizeof(receiver.trapcom) - 1);
        
        if(0 == strcmp(argv[7], "enable")) {
            receiver.status = 1;
        }
        else if( 0 == strcmp(argv[7], "disable")) {
            receiver.status = 0;
        }
        else {
            vty_out(vty, "Input trap receiver status error!\n");
            return CMD_WARNING;
        }
        
        if(9 == argc) {
            receiver.dest_port = atoi(argv[8]);
        }
    }
    else if(6 == argc || 7 == argc) {
        receiver.version = 3;

        if(1 != snmp_ipaddr_is_legal_input(argv[4])) {
            vty_out(vty, "error dest ipaddr input : %s\n", argv[4]);
            return CMD_WARNING;
        }
        strncpy(receiver.dest_ipAddr, argv[4], sizeof(receiver.dest_ipAddr) - 1);
        
        if(0 == strcmp(argv[5], "enable")) {
            receiver.status = 1;
        }
        else if( 0 == strcmp(argv[5], "disable")) {
            receiver.status = 0;
        }
        else {
            vty_out(vty, "Input trap receiver status error!\n");
            return CMD_WARNING;
        }
        
        if(7 == argc) {
            receiver.dest_port = atoi(argv[6]);
        }
        
    }
    else {
        vty_out(vty, "Input prara is error!\n");
        return CMD_WARNING;
    }

    if(SNMP_NODE == vty->node){
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
            int ret = AC_MANAGE_SUCCESS;
            ret = ac_manage_config_trap_config_receiver(dbus_connection_dcli[slot_id]->dcli_dbus_connection, &receiver, type);
            if(AC_MANAGE_SUCCESS == ret) {
                //vty_out(vty, "%s trap receiver success\n", type ? "add" : "set");
            }
            else if(AC_MANAGE_DBUS_ERROR == ret) {
                vty_out(vty, "<error> failed get reply.\n");
            }
            else if(AC_MANAGE_SERVICE_ENABLE == ret) {
                vty_out(vty, "Trap service is enable, please disable it first!\n");
            }
            else if(AC_MANAGE_CONFIG_EXIST == ret) {
                vty_out(vty, "This trap receiver is exist!\n");
            }
            else if(AC_MANAGE_CONFIG_NONEXIST == ret) {
                vty_out(vty, "This trap receiver is not exist!\n");
            }
            else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) {
                vty_out(vty, "input type is error!\n");
            }
            else{
                vty_out(vty, "%s trap receiver failed\n", type ? "add" : "set");
           }                                                  
        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!\n", slot_id);
        }   
    }
    else {
		vty_out (vty, "Terminal mode change must under SNMP mode!\n");
    }

    return CMD_SUCCESS;
}

ALIAS(config_snmp_trap_receiver_func,
    config_trap_receiver_v3_cmd,
    "(add|set) trap NAME (local-hansi|remote-hansi) A-B v3 destaddr A.B.C.D (enable|disable) [<1-65535>]",
    "add\n"
	"set\n"
	"snmp trap\n"
	"trap receiver name\n"
	"trap Local hansi\n"
	"trap Remote hansi\n"
	"trap instance id\n"
	"snmp version v3\n"
	"trap destination address\n"
	"destination ip address\n"
	"trap enable\n"
	"trap disable\n"
	"trap destination port, default 162\n"
)

DEFUN(delete_snmp_trap_func,
	delete_snmp_trap_cmd,
	"delete trap (local-hansi|remote-hansi) A-B NAME",
	"delete\n"
	"snmp trap\n"
	"trap Local hansi\n"
	"trap Remote hansi\n"
	"trap instance id\n"
	"trap receiver name\n"
)
{

    unsigned int slot_id = 0;
    unsigned int instance_id = 0;
    
    if(0 == sscanf(argv[1], "%d-%d", &slot_id, &instance_id)) {
		vty_out(vty, "input instance (%s) type is error\n", argv[1]);
        return CMD_WARNING;
    }
    
    if(0 == slot_id || slot_id > 16) {
		vty_out(vty, "error slot id input : %s\n", argv[1]);
        return CMD_WARNING;
    }
    else if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		vty_out(vty, "The slot %d is not connect\n", slot_id);
        return CMD_WARNING;
    }
    
    if(0 == instance_id || instance_id > 16) {
		vty_out(vty, "error instnace id input : %s\n", argv[1]);
        return CMD_WARNING;
    }
    
    if(1 != trap_name_is_legal_input(argv[2])) {
		vty_out(vty, "error name input : %s\n", argv[2]);
		return CMD_WARNING;
    }
    char *name = strdup(argv[2]);
    
    if(SNMP_NODE == vty->node) {
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
            int ret = AC_MANAGE_SUCCESS;
            ret = ac_manage_config_trap_del_receiver(dbus_connection_dcli[slot_id]->dcli_dbus_connection, name);
            if(AC_MANAGE_SUCCESS == ret) {
                //vty_out(vty, "delete trap receiver success\n");
            }
            else if(AC_MANAGE_DBUS_ERROR == ret) {
                vty_out(vty, "<error> failed get reply.\n");
            }
            else if(AC_MANAGE_SERVICE_ENABLE == ret) {
                vty_out(vty, "Trap service is enable, please disable it first!\n");
            }
            else if(AC_MANAGE_CONFIG_NONEXIST == ret) {
                vty_out(vty, "This trap receiver is not exist!\n");
            }
            else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) {
                vty_out(vty, "input type is error!\n");
            }
            else{
                vty_out(vty, "delete trap receiver failed!\n");
           }                                                  
        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!\n", slot_id);
        }   
    }
    else {
        vty_out (vty, "Terminal mode change must under SNMP mode!\n");
    }

    MANAGE_FREE(name);
    
    return CMD_SUCCESS;
}

DEFUN(set_trap_instance_heartbeat_func,
	set_trap_instance_heartbeat_cmd,
	"set trap (local-hansi|remote-hansi) A-B heartbeat IPADDR",
	"set\n"
	"snmp trap\n"
	"trap Local hansi\n"
	"trap Remote hansi\n"
	"trap instance id\n"
	"trap instance heartbeat ipAddr\n"
)
{

    TRAPHeartbeatIP heartbeatNode = { 0 };
    unsigned int slot_id = 0;

    if(0 == strcmp("local-hansi", argv[0])) {
        heartbeatNode.local_id = 1;
    }
    else if(0 == strcmp("remote-hansi", argv[0])) {
        heartbeatNode.local_id = 0;
    }
    else {
        vty_out(vty, "Input trap heartbeat instance type is error!\n");
        return CMD_WARNING;
    }
    
    if(0 == sscanf(argv[1], "%d-%d", &slot_id, &heartbeatNode.instance_id)) {
		vty_out(vty, "input instance (%s) type is error\n", argv[1]);
        return CMD_WARNING;
    }
    
    if(0 == slot_id || slot_id > 16) {
		vty_out(vty, "error slot id input : %s\n", argv[1]);
        return CMD_WARNING;
    }
    else if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		vty_out(vty, "The slot %d is not connect\n", slot_id);
        return CMD_WARNING;
    }
    
    if(0 == heartbeatNode.instance_id || heartbeatNode.instance_id > 16) {
		vty_out(vty, "error instnace id input : %s\n", argv[1]);
        return CMD_WARNING;
    }
    
    if(1 != snmp_ipaddr_is_legal_input(argv[2])) {
        vty_out(vty, "error source ipaddr input : %s\n", argv[2]);
        return CMD_WARNING;
    }
    strncpy(heartbeatNode.ipAddr, argv[2], sizeof(heartbeatNode.ipAddr) - 1);
    
    if(SNMP_NODE == vty->node) {
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
            int ret = AC_MANAGE_SUCCESS;
            ret = ac_manage_config_trap_instance_heartbeat(dbus_connection_dcli[slot_id]->dcli_dbus_connection, &heartbeatNode);
            if(AC_MANAGE_SUCCESS == ret) {
                //vty_out(vty, "set trap heartbeat ipAddr success\n");
            }
            else if(AC_MANAGE_DBUS_ERROR == ret) {
                vty_out(vty, "<error> failed get reply.\n");
            }
            else if(AC_MANAGE_SERVICE_ENABLE == ret) {
                vty_out(vty, "Trap service is enable, please disable it first!\n");
            }
            else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) {
                vty_out(vty, "input type is error!\n");
            }
            else{
                vty_out(vty, "set trap heartbeat ipAddr failed!\n");
           }                                                  
        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!\n", slot_id);
        }   
    }
    else {
        vty_out (vty, "Terminal mode change must under SNMP mode!\n");
    }
    
    return CMD_SUCCESS;
}

DEFUN(clear_trap_instance_heartbeat_func,
	clear_trap_instance_heartbeat_cmd,
	"clear trap (local-hansi|remote-hansi) A-B heartbeat",
	"clear\n"
	"snmp trap\n"
	"trap Local hansi\n"
	"trap Remote hansi\n"
	"trap instance id\n"
)
{

    unsigned int slot_id = 0;
    unsigned int local_id = 0;
    unsigned int instance_id = 0;

    if(0 == strcmp("local-hansi", argv[0])) {
        local_id = 1;
    }
    else if(0 == strcmp("remote-hansi", argv[0])) {
        local_id = 0;
    }
    else {
        vty_out(vty, "Input trap heartbeat instance type is error!\n");
        return CMD_WARNING;
    }
    
    if(0 == sscanf(argv[1], "%d-%d", &slot_id, &instance_id)) {
		vty_out(vty, "input instance (%s) type is error\n", argv[1]);
        return CMD_WARNING;
    }
    
    if(0 == slot_id || slot_id > 16) {
		vty_out(vty, "error slot id input : %s\n", argv[1]);
        return CMD_WARNING;
    }
    else if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
		vty_out(vty, "The slot %d is not connect\n", slot_id);
        return CMD_WARNING;
    }
    
    if(0 == instance_id || instance_id > 16) {
		vty_out(vty, "error instnace id input : %s\n", argv[1]);
        return CMD_WARNING;
    }
        
    if(SNMP_NODE == vty->node) {
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
            int ret = AC_MANAGE_SUCCESS;
            ret = ac_manage_clear_trap_instance_heartbeat(dbus_connection_dcli[slot_id]->dcli_dbus_connection, local_id, instance_id);
            if(AC_MANAGE_SUCCESS == ret) {
                //vty_out(vty, "clear trap heartbeat ipAddr success\n");
            }
            else if(AC_MANAGE_DBUS_ERROR == ret) {
                vty_out(vty, "<error> failed get reply.\n");
            }
            else if(AC_MANAGE_SERVICE_ENABLE == ret) {
                vty_out(vty, "Trap service is enable, please disable it first!\n");
            }
            else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) {
                vty_out(vty, "input type is error!\n");
            }
            else{
                vty_out(vty, "clear trap heartbeat ipAddr failed!\n");
           }                                                  
        }
        else {
            vty_out(vty, "The slot %d tipc dbus is not connect!\n", slot_id);
        }   
    }
    else {
        vty_out (vty, "Terminal mode change must under SNMP mode!\n");
    }
    
    return CMD_SUCCESS;
}


DEFUN(set_trap_statistics_interval_func,
	set_trap_statistics_interval_cmd,
	"set statistics-interval <0-3600>",
	SETT_STR
	"trap statistics interval\n"
	"trap statistics interval\n"
)
{
    char *temp_str = (char *)malloc(MAX_TRAP_PARAMETER_LEN);
    if(NULL == temp_str) {
        vty_out(vty, "malloc temp_str error!\n");
        return CMD_WARNING;
    }
    
    memset(temp_str, 0, MAX_TRAP_PARAMETER_LEN);
    strncpy(temp_str, STATISTICS_INTERVAL, MAX_TRAP_PARAMETER_LEN - 1);

    unsigned int data = atoi(argv[0]);
    if(SNMP_NODE == vty->node) {
        int i = 1;
        for(i = 1; i < MAX_SLOT; i++) {
            if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                int ret = AC_MANAGE_SUCCESS;
                ret = ac_manage_config_trap_parameter(dbus_connection_dcli[i]->dcli_dbus_connection, temp_str, data);

                if(AC_MANAGE_SUCCESS == ret) {
                    //vty_out(vty, "Slot(%d) :set trap statistics-interval success\n", i);
                }
                else if(AC_MANAGE_DBUS_ERROR == ret) {
                    vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
                }
                else if(AC_MANAGE_SERVICE_ENABLE == ret) {
                    vty_out(vty, "Slot(%d) :Trap service is enable, please disable it first!\n", i);
                }
                else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) {
                    vty_out(vty, "Slot(%d) :input type is error!\n", i);
                }
                else{
                    vty_out(vty, "Slot(%d) :set trap statistics-interval failed!\n", i);
                }
            }
        }
    }
    else {
        vty_out (vty, "Terminal mode change must under SNMP mode!\n");
    }
    
    MANAGE_FREE(temp_str);
    
	return CMD_SUCCESS;
}

DEFUN(set_trap_sampling_interval_func,
	set_trap_sampling_interval_cmd,
	"set sampling-interval <0-3600>",
	SETT_STR
	"trap sampling interval\n"
	"trap sampling interval\n"
)
{
    char *temp_str = (char *)malloc(MAX_TRAP_PARAMETER_LEN);
    if(NULL == temp_str) {
        vty_out(vty, "malloc temp_str error!\n");
        return CMD_WARNING;
    }
    
    memset(temp_str, 0, MAX_TRAP_PARAMETER_LEN);
    strncpy(temp_str, SAMPLING_INTERVAL, MAX_TRAP_PARAMETER_LEN - 1);

    unsigned int data = atoi(argv[0]);
    if(SNMP_NODE == vty->node) {
        int i = 1;
        for(i = 1; i < MAX_SLOT; i++) {
            if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                int ret = AC_MANAGE_SUCCESS;
                ret = ac_manage_config_trap_parameter(dbus_connection_dcli[i]->dcli_dbus_connection, temp_str, data);

                if(AC_MANAGE_SUCCESS == ret) {
                    //vty_out(vty, "Slot(%d) :set trap sampling-interval success\n", i);
                }
                else if(AC_MANAGE_DBUS_ERROR == ret) {
                    vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
                }
                else if(AC_MANAGE_SERVICE_ENABLE == ret) {
                    vty_out(vty, "Slot(%d) :Trap service is enable, please disable it first!\n", i);
                }
                else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) {
                    vty_out(vty, "Slot(%d) :input type is error!\n", i);
                }
                else{
                    vty_out(vty, "Slot(%d) :set trap sampling-interval failed!\n", i);
                }
            }
        }
    }
    else {
        vty_out (vty, "Terminal mode change must under SNMP mode!\n");
    }
    
    MANAGE_FREE(temp_str);
    
    return CMD_SUCCESS;
}

DEFUN(set_trap_heartbeat_interval_func,
	set_trap_heartbeat_interval_cmd,
	"set heartbeat-interval <0-3600>",
	SETT_STR
	"trap heartbeat interval\n"
	"trap heartbeat interval\n"
)
{
    char *temp_str = (char *)malloc(MAX_TRAP_PARAMETER_LEN);
    if(NULL == temp_str) {
        vty_out(vty, "malloc temp_str error!\n");
        return CMD_WARNING;
    }
    
    memset(temp_str, 0, MAX_TRAP_PARAMETER_LEN);
    strncpy(temp_str, HEARTBEAT_INTERVAL, MAX_TRAP_PARAMETER_LEN - 1);

    unsigned int data = atoi(argv[0]);
    if(SNMP_NODE == vty->node) {
        int i = 1;
        for(i = 1; i < MAX_SLOT; i++) {
            if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                int ret = AC_MANAGE_SUCCESS;
                ret = ac_manage_config_trap_parameter(dbus_connection_dcli[i]->dcli_dbus_connection, temp_str, data);

                if(AC_MANAGE_SUCCESS == ret) {
                    //vty_out(vty, "Slot(%d) :set trap heartbeat-interval success\n", i);
                }
                else if(AC_MANAGE_DBUS_ERROR == ret) {
                    vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
                }
                else if(AC_MANAGE_SERVICE_ENABLE == ret) {
                    vty_out(vty, "Slot(%d) :Trap service is enable, please disable it first!\n", i);
                }
                else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) {
                    vty_out(vty, "Slot(%d) :input type is error!\n", i);
                }
                else{
                    vty_out(vty, "Slot(%d) :set trap heartbeat-interval failed!\n", i);
                }
            }
        }
    }
    else {
        vty_out (vty, "Terminal mode change must under SNMP mode!\n");
    }
    
    MANAGE_FREE(temp_str);
    
    return CMD_SUCCESS;
}

DEFUN(set_trap_heartbeat_mode_func,
	set_trap_heartbeat_mode_cmd,
	"set trap heartbeat mode <0-1>",
	SETT_STR
	"snmp trap\n" 
	"set trap heartbeat mode 0 or 1\n"
)
{
    char *temp_str = (char *)malloc(MAX_TRAP_PARAMETER_LEN);
    if(NULL == temp_str) {
        vty_out(vty, "malloc temp_str error!\n");
        return CMD_WARNING;
    }
    
    memset(temp_str, 0, MAX_TRAP_PARAMETER_LEN);
    strncpy(temp_str, HEARTBEAT_MODE, MAX_TRAP_PARAMETER_LEN - 1);

    unsigned int data = atoi(argv[0]);
    if(SNMP_NODE == vty->node) {
        int i = 1;
        for(i = 1; i < MAX_SLOT; i++) {
            if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                int ret = AC_MANAGE_SUCCESS;
                ret = ac_manage_config_trap_parameter(dbus_connection_dcli[i]->dcli_dbus_connection, temp_str, data);

                if(AC_MANAGE_SUCCESS == ret) {
                    //vty_out(vty, "Slot(%d) :set trap heartbeat mode success\n", i);
                }
                else if(AC_MANAGE_DBUS_ERROR == ret) {
                    vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
                }
                else if(AC_MANAGE_SERVICE_ENABLE == ret) {
                    vty_out(vty, "Slot(%d) :Trap service is enable, please disable it first!\n", i);
                }
                else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) {
                    vty_out(vty, "Slot(%d) :input type is error!\n", i);
                }
                else{
                    vty_out(vty, "Slot(%d) :set trap heartbeat mode failed!\n", i);
                }
            }
        }
    }
    else {
        vty_out (vty, "Terminal mode change must under SNMP mode!\n");
    }
    
    MANAGE_FREE(temp_str);
    
    return CMD_SUCCESS;
}


DEFUN(set_trap_cpu_threshold_func,
	set_trap_cpu_threshold_cmd,
	"set cpu-threshold <0-100>",
	SETT_STR
	"threshold for CPU utilization\n"
	"threshold for CPU utilization\n"
)
{
    char *temp_str = (char *)malloc(MAX_TRAP_PARAMETER_LEN);
    if(NULL == temp_str) {
        vty_out(vty, "malloc temp_str error!\n");
        return CMD_WARNING;
    }
    
    memset(temp_str, 0, MAX_TRAP_PARAMETER_LEN);
    strncpy(temp_str, CPU_THRESHOLD, MAX_TRAP_PARAMETER_LEN - 1);

    unsigned int data = atoi(argv[0]);
    if(SNMP_NODE == vty->node) {
        int i = 1;
        for(i = 1; i < MAX_SLOT; i++) {
            if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                int ret = AC_MANAGE_SUCCESS;
                ret = ac_manage_config_trap_parameter(dbus_connection_dcli[i]->dcli_dbus_connection, temp_str, data);

                if(AC_MANAGE_SUCCESS == ret) {
                    //vty_out(vty, "Slot(%d) :set trap cpu-threshold success\n", i);
                }
                else if(AC_MANAGE_DBUS_ERROR == ret) {
                    vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
                }
                else if(AC_MANAGE_SERVICE_ENABLE == ret) {
                    vty_out(vty, "Slot(%d) :Trap service is enable, please disable it first!\n", i);
                }
                else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) {
                    vty_out(vty, "Slot(%d) :input type is error!\n", i);
                }
                else{
                    vty_out(vty, "Slot(%d) :set trap cpu-threshold failed!\n", i);
                }
            }
        }
    }
    else {
        vty_out (vty, "Terminal mode change must under SNMP mode!\n");
    }
    
    MANAGE_FREE(temp_str);
    
    return CMD_SUCCESS;
}

DEFUN(set_trap_memory_threshold_func,
	set_trap_memory_threshold_cmd,
	"set memory-threshold <0-100>",
	SETT_STR
	"threshold for memory utilization\n"
	"threshold for memory utilization\n"
)
{
    char *temp_str = (char *)malloc(MAX_TRAP_PARAMETER_LEN);
    if(NULL == temp_str) {
        vty_out(vty, "malloc temp_str error!\n");
        return CMD_WARNING;
    }
    
    memset(temp_str, 0, MAX_TRAP_PARAMETER_LEN);
    strncpy(temp_str, MEMORY_THRESHOLD, MAX_TRAP_PARAMETER_LEN - 1);

    unsigned int data = atoi(argv[0]);
    if(SNMP_NODE == vty->node) {
        int i = 1;
        for(i = 1; i < MAX_SLOT; i++) {
            if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                int ret = AC_MANAGE_SUCCESS;
                ret = ac_manage_config_trap_parameter(dbus_connection_dcli[i]->dcli_dbus_connection, temp_str, data);

                if(AC_MANAGE_SUCCESS == ret) {
                    //vty_out(vty, "Slot(%d) :set trap memory-threshold success\n", i);
                }
                else if(AC_MANAGE_DBUS_ERROR == ret) {
                    vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
                }
                else if(AC_MANAGE_SERVICE_ENABLE == ret) {
                    vty_out(vty, "Slot(%d) :Trap service is enable, please disable it first!\n", i);
                }
                else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) {
                    vty_out(vty, "Slot(%d) :input type is error!\n", i);
                }
                else{
                    vty_out(vty, "Slot(%d) :set trap memory-threshold failed!\n", i);
                }
            }
        }
    }
    else {
        vty_out (vty, "Terminal mode change must under SNMP mode!\n");
    }
    
    MANAGE_FREE(temp_str);
    
    return CMD_SUCCESS;
}

DEFUN(set_trap_temperature_threshold_func,
	set_trap_temperature_threshold_cmd,
	"set temperature-threshold <0-100>",
	SETT_STR
	"temperature threshold\n"
	"temperature threshold\n"
)
{
    char *temp_str = (char *)malloc(MAX_TRAP_PARAMETER_LEN);
    if(NULL == temp_str) {
        vty_out(vty, "malloc temp_str error!\n");
        return CMD_WARNING;
    }
    
    memset(temp_str, 0, MAX_TRAP_PARAMETER_LEN);
    strncpy(temp_str, TEMPERATURE_THRESHOLD, MAX_TRAP_PARAMETER_LEN - 1);

    unsigned int data = atoi(argv[0]);
    if(SNMP_NODE == vty->node) {
        int i = 1;
        for(i = 1; i < MAX_SLOT; i++) {
            if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                int ret = AC_MANAGE_SUCCESS;
                ret = ac_manage_config_trap_parameter(dbus_connection_dcli[i]->dcli_dbus_connection, temp_str, data);

                if(AC_MANAGE_SUCCESS == ret) {
                    //vty_out(vty, "Slot(%d) :set trap temperature-threshold success\n", i);
                }
                else if(AC_MANAGE_DBUS_ERROR == ret) {
                    vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
                }
                else if(AC_MANAGE_SERVICE_ENABLE == ret) {
                    vty_out(vty, "Slot(%d) :Trap service is enable, please disable it first!\n", i);
                }
                else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) {
                    vty_out(vty, "Slot(%d) :input type is error!\n", i);
                }
                else{
                    vty_out(vty, "Slot(%d) :set trap temperature-threshold failed!\n", i);
                }
            }
        }
    }
    else {
        vty_out (vty, "Terminal mode change must under SNMP mode!\n");
    }
    
    MANAGE_FREE(temp_str);
    
    return CMD_SUCCESS;
}


DEFUN(set_trap_resend_info_func,
	set_trap_resend_info_cmd,
	"set trap resend interval <0-65535> times <0-50>",
	SETT_STR
	"config snmp\n" 
	"trap resned interval\n"
	"trap resend times\n"
)
{
    char *inter_str = (char *)malloc(MAX_TRAP_PARAMETER_LEN);
    char *times_str = (char *)malloc(MAX_TRAP_PARAMETER_LEN);
    if(NULL == inter_str || NULL == times_str) {
        vty_out(vty, "malloc temp_str error!\n");
        goto CMD_END;
    }
    
    memset(inter_str, 0, MAX_TRAP_PARAMETER_LEN);
    strncpy(inter_str, RESEND_INTERVAL, MAX_TRAP_PARAMETER_LEN - 1);

    memset(times_str, 0, MAX_TRAP_PARAMETER_LEN);
    strncpy(times_str, RESEND_TIMES, MAX_TRAP_PARAMETER_LEN - 1);

    unsigned int inter_data = atoi(argv[0]);
    unsigned int times_data = atoi(argv[1]);
    
    if(SNMP_NODE == vty->node) {
        int i = 1;
        for(i = 1; i < MAX_SLOT; i++) {
            if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                int ret = ac_manage_config_trap_parameter(dbus_connection_dcli[i]->dcli_dbus_connection, inter_str, inter_data);
                if(AC_MANAGE_SUCCESS == ret) {
                    int temp_ret = ac_manage_config_trap_parameter(dbus_connection_dcli[i]->dcli_dbus_connection, times_str, times_data);
                    if(AC_MANAGE_SUCCESS == temp_ret) {
                        //vty_out(vty, "Slot(%d) :set trap resend parameter success\n", i);
                    }
                    else if(AC_MANAGE_SERVICE_ENABLE == temp_ret) {
                        vty_out(vty, "Slot(%d) :Trap service is enable, please disable it first!\n", i);
                    }
                    else if(AC_MANAGE_DBUS_ERROR == temp_ret) {
                        vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
                    }
                    else if(AC_MANAGE_INPUT_TYPE_ERROR == temp_ret) {
                        vty_out(vty, "Slot(%d) :trap resend times input type is error!\n", i);
                    }
                    else{
                        vty_out(vty, "Slot(%d) :set trap resend times failed!\n", i);
                    }
                }                
                else if(AC_MANAGE_SERVICE_ENABLE == ret) {
                    vty_out(vty, "Slot(%d) :Trap service is enable, please disable it first!\n", i);
                }
                else if(AC_MANAGE_DBUS_ERROR == ret) {
                    vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
                }
                else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) {
                    vty_out(vty, "Slot(%d) :trap resend interval input type is error!\n", i);
                }
                else{
                    vty_out(vty, "Slot(%d) :set trap resend interval failed!\n", i);
                }
            }
        }
    }
    else {
        vty_out (vty, "Terminal mode change must under SNMP mode!\n");
    }

CMD_END:    
    MANAGE_FREE(inter_str);
    MANAGE_FREE(times_str);
    
    return CMD_SUCCESS;
}


/*delete snmp  trap  by index*/
DEFUN(set_trap_type_switch_func,
	set_trap_type_switch_cmd,
	"set trap type (1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16|17|18|19|20|"\
					"21|22|23|24|25|26|27|28|29|30|31|32|33|34|35|36|37|38|39|40|"\
					"41|42|43|44|45|46|47|48|49|50|51|52|53|54|55|56|57|58|59|60|"\
					"61|62|63|64|65|66|67|68|69|70|71|72|73|74|75|76|77|78|79|80|"\
					"81|82|83|84|85|86|87|88|89|90|91|92|93|94) (on|off)",
	SETT_STR
	"snmp trap \n"
	"snmp trap type \n"
	/*1*/"AP is down \n"
	/*2*/"AP reboot \n"
	/*3*/"channel is changed \n"
	/*4*/"AP ip address changed \n"
	/*5*/"AP flash write failed \n"
	/*6*/"AP cold boot \n"
	/*7*/"AP auth mode changed \n"
	/*8*/"Pre-share-key is changed \n"
	/*9*/"Electricity-reg-circle is on \n"
	/*10*/"ap update \n"
	/*11*/"AP cover hole \n"
	/*12*/"AP CPU usage is over threshold \n"
	/*13*/"AP CPU usage recovery \n"
	/*14*/"AP MEM usage is over threshold \n"
	/*15*/"AP MEM usage recovery \n"
	/*16*/"AP temperature is too high \n"
	/*17*/"AP temperature is recover \n"
	/*18*/"AP MTwork mode is changed \n"
	/*19*/"SSID key conflict \n"
	/*20*/"AP status is running \n"
	/*21*/"AP status is quit \n"
	/*22*/"AP cover hole clear \n"
	/*23*/"AP software update succeed \n"
	/*24*/"AP software update failed \n"
	/*25*/"AP channel obstruction \n"
	/*26*/"AP interfere has detected \n"
	/*27*/"Station interfere has detected \n"
	/*28*/"Device interfere has detected \n"
	/*29*/"AP can not add station \n"
	/*30*/"Channel count minor \n"
	/*31*/"File is transfered \n"
	/*32*/"wtp station offline \n"
	/*33*/"Link varified fail clear \n"
	/*34*/"Link varified fail \n"
	/*35*/"station has been on line \n"
	/*36*/"AP interfere has been clear \n"
	/*37*/"Station interfere has been clear\n"
	/*38*/"device interfere has been clear \n"
	/*39*/"Wireless moudle show errors \n"
	/*40*/"Wireless moudle show errors clear \n"
	/*41*/"Wireless link show errors \n"
	/*42*/"Wireless link show errors clear \n"
	/*43*/"Station auth fail \n"
	/*44*/"Station assoc fail \n"
	/*45*/"Invalid certification user attack \n"
	/*46*/"Client Re-attack \n"
	/*47*/"tamper attack \n"
	/*48*/"low safe level attack \n"
	/*49*/"address redirect attack \n"
	/*50*/"AP can not add station clear \n"
	/*51*/"Channel count minor clear \n"
	/*52*/"AP find unsafe SSID \n"
	/*53*/"AP find syn attack \n"
	/*54*/"AP neighbor channel interfere \n"
	/*55*/"AP neighbor channel interfere clear \n"
	/*56*/"AC reboot \n"
	/*57*/"AC synoc time fail with ap \n"
	/*58*/"AC ip address has changed \n"
	/*59*/"AC turn to backup state \n"
	/*60*/"AC configuration error \n"
	/*61*/"AC system cold start \n"
	/*62*/"AC synoc time success with ap \n"
	/*63*/"AC discover danger ap \n"
	/*64*/"Radius auth server can not reach \n"
	/*65*/"Radius accounting server can not reach \n"
	/*66*/"Portal server can not reach \n"
	/*67*/"AP has been lost network \n"
	/*68*/"AC CPU usage is over threshold \n"
	/*69*/"AC MEM usage is over threshold \n"
	/*70*/"AC bandwith is too high \n"
	/*71*/"AC drop pkg rate is too high \n"
	/*72*/"AC Max Online user is too high \n"
	/*73*/"AC radius auth suc rate is too low \n"
	/*74*/"AC IP pool average rate is too high \n"
	/*75*/"AC IP pool max rate is too high \n"
	/*76*/"AC power is turned \n"
	/*77*/"AC power is recovery \n"
	/*78*/"AC CPU usage is over threshold clear \n"
	/*79*/"AC MEM usage is over threshold clear \n"
	/*80*/"AC temperature is too high \n"
	/*81*/"AC temperature high recovered \n"
	/*82*/"AC DHCP pool exhaust \n"
	/*83*/"AC DHCP pool exhaust clear \n"
	/*84*/"radius auth server can reached \n"
	/*85*/"radius account server can reached \n"
	/*86*/"Portal server can reached \n"
	/*87*/"AC find attack \n"
	/*88*/"AC heart time break \n"
	/*89*/"AC Max Online user is too high clear \n"
	/*90*/"AP station leave abnormal \n" 
	/*91*/"user logoff abnormal\n"
	/*92*/"AP Configuration Error\n"
	/*93*/"User Traffic Overload \n"
	/*94*/"Unauthorized Station Mac\n"
	"trap switch on \n"
	"trap switch off \n"
)
{

    unsigned int index = atoi(argv[0]) - 1;
    TRAP_DETAIL_CONFIG trapConfig = { 0 };
    if(0 == strcmp(argv[1], "on")) {
         trapConfig.trapSwitch = 1;   
    }
    
    if(SNMP_NODE == vty->node) {
        int i = 1;
        for(i = 1; i < MAX_SLOT; i++) {
            if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                int ret = AC_MANAGE_SUCCESS;
                ret = ac_manage_config_trap_switch(dbus_connection_dcli[i]->dcli_dbus_connection, index, &trapConfig);

                if(AC_MANAGE_SUCCESS == ret) {
                    //vty_out(vty, "Slot(%d) :set trap %d switch %s success\n", i, index + 1, trapConfig.trapSwitch ? "on" : "off");
                }
                else if(AC_MANAGE_DBUS_ERROR == ret) {
                    vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
                }
                else if(AC_MANAGE_SERVICE_ENABLE == ret) {
                    vty_out(vty, "Slot(%d) :Trap service is enable, please disable it first!\n", i);
                }
                else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) {
                    vty_out(vty, "Slot(%d) :input type is error!\n", i);
                }
                else{
                    vty_out(vty, "Slot(%d) :set trap %d switch %s failed!\n", i, index + 1, trapConfig.trapSwitch ? "on" : "off");
                }
            }
        }
    }
    else {
        vty_out (vty, "Terminal mode change must under SNMP mode!\n");
    }
    
	return CMD_SUCCESS;
}

DEFUN(set_trap_group_switch_func,
	set_trap_group_switch_cmd,
	"set trap group switch GROUP",
	SETT_STR
	"set trap \n"
	"set trap group switch\n"
)
{
    struct trap_group_switch group_switch = { 0 };
    if(AC_MANAGE_SUCCESS != string_to_trap_group_switch(argv[0], &group_switch)) {
        vty_out(vty, "Input group type is error!\n");
        return CMD_WARNING;
    }

    if(SNMP_NODE == vty->node) {
        int i = 1;
        for(i = 1; i < MAX_SLOT; i++) {
            if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                int ret = AC_MANAGE_SUCCESS;
                ret = ac_manage_config_trap_group_switch(dbus_connection_dcli[i]->dcli_dbus_connection, &group_switch);

                if(AC_MANAGE_SUCCESS == ret) {
                    //vty_out(vty, "Slot(%d) :set trap %d switch %s success\n", i, index + 1, trapConfig.trapSwitch ? "on" : "off");
                }
                else if(AC_MANAGE_DBUS_ERROR == ret) {
                    vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
                }
                else if(AC_MANAGE_SERVICE_ENABLE == ret) {
                    vty_out(vty, "Slot(%d) :Trap service is enable, please disable it first!\n", i);
                }
                else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) {
                    vty_out(vty, "Slot(%d) :input type is error!\n", i);
                }
                else{
                    vty_out(vty, "Slot(%d) :set trap group switch failed!\n");
                }
            }
        }
    }
    else {
        vty_out (vty, "Terminal mode change must under SNMP mode!\n");
    }

    return CMD_SUCCESS;
}


DEFUN(show_trap_info_func,
	show_trap_info_cmd,
	"show trap info",
	SHOW_STR
	"snmp trap configure\n"
	"configure information\n"
)
{
    if((SNMP_NODE == vty->node)||(VIEW_NODE == vty->node)) {        
        vty_out(vty,"======================================================================\n");
        int slot_index = 1;
        for(slot_index = 1; slot_index < MAX_SLOT; slot_index++) {
            if(dbus_connection_dcli[slot_index]->dcli_dbus_connection) {
                vty_out(vty, "TRAP SLOT             : %d \n", slot_index );  

                int ret = AC_MANAGE_SUCCESS;
                unsigned int trap_state = 0;
                ret = ac_manage_show_trap_state(dbus_connection_dcli[slot_index]->dcli_dbus_connection, &trap_state);
                if(AC_MANAGE_SUCCESS == ret) {
                    vty_out(vty, "TRAP Service          : %s \n", trap_state ? "enable" : "disable" );
                }
                else {
                    vty_out(vty, "Fail to get trap state!\n");
                    vty_out(vty,"======================================================================\n");
                    break;
                }
                
                STSNMPTrapReceiver *receiver_array = NULL;
                unsigned int receiver_num = 0;
                
                unsigned int receiver_ret = AC_MANAGE_SUCCESS;
                receiver_ret = ac_manage_show_trap_receiver(dbus_connection_dcli[slot_index]->dcli_dbus_connection, &receiver_array, &receiver_num);
                if(AC_MANAGE_SUCCESS == receiver_ret && receiver_array) {
                    
                    if(receiver_num) {
                        
                        int i = 0;
                        for(i = 0; i < receiver_num; i++) {
                            char *strVersion = NULL;    
                            if (V2 == receiver_array[i].version)
                                strVersion = "v2c";
                            else if (V3 == receiver_array[i].version)
                                strVersion = "v3";
                            else
                                strVersion = "v1";
                                
                            vty_out(vty,"----------------------------------------------------------------------\n");
                            vty_out(vty, "%s   %d-%d\n", receiver_array[i].local_id ? "Local-Hansi" : "Remote-Hansi", slot_index, receiver_array[i].instance_id);
                            vty_out(vty, "Trap Receiver:               %s\n", receiver_array[i].name);
                            vty_out(vty, "Sour-IPAddr:                 %s\n", receiver_array[i].sour_ipAddr[0] ? receiver_array[i].sour_ipAddr : "0.0.0.0");
                            vty_out(vty, "Dest-IPAddr:                 %s\n", receiver_array[i].dest_ipAddr);
                            vty_out(vty, "Dest-Port:                   %d\n", receiver_array[i].dest_port ? receiver_array[i].dest_port : 162);
                            vty_out(vty, "Trap Receiver Community:     %s\n", receiver_array[i].trapcom);
                            vty_out(vty, "Trap Receiver Version:       %s\n", strVersion);
                            vty_out(vty, "Trap Receiver Status:        %s\n", receiver_array[i].status ? "enable" : "disable");
                            vty_out(vty,"----------------------------------------------------------------------\n");
                        }
                    }
                    MANAGE_FREE(receiver_array);
                }
                else if(AC_MANAGE_DBUS_ERROR == receiver_ret) {
                    vty_out(vty, "<error> failed get reply.\n");
                }
                else if(AC_MANAGE_CONFIG_NONEXIST == receiver_ret){
                    vty_out(vty, "There is no trap receiver configure!\n");
                }
                else {
                    vty_out(vty, "Fail to show trap info.\n");
                }
                
                vty_out(vty,"======================================================================\n");
            }
        }
    }    
    else {
        vty_out (vty, "Terminal mode change must under SNMP mode or VIEW mode!\n");
    }

	return CMD_SUCCESS;
}


static void
dcli_show_trap_switch(DBusConnection *connection, struct vty *vty) {

	TRAP_DETAIL_CONFIG *trapDetail_array = NULL;
	unsigned int trapDetail_num = 0;

	unsigned int ret = AC_MANAGE_SUCCESS;
	ret = ac_manage_show_trap_switch(connection, &trapDetail_array, &trapDetail_num);
	if(AC_MANAGE_SUCCESS == ret) {
        
        if(trapDetail_num) {
            vty_out(vty,"----------------------------------------------------------------------\n");
            vty_out(vty, "%7s   %5s %20s    %20s  \n", "INDEX","SWITCH", "TRAP NAME","TRAP DES");
            
            int i = 0;
            for(i = 0; i < trapDetail_num; i++) {
                vty_out(vty, "%7d   %5s %20s    %20s  \n", i + 1, trapDetail_array[i].trapSwitch ? "on" : "off",
                                                           trapDetail_array[i].trapName, trapDetail_array[i].trapEDes );
            }
            vty_out(vty,"----------------------------------------------------------------------\n");
        }
        else {
            vty_out(vty, "Fail to show trap switch.\n");
        }
        MANAGE_FREE(trapDetail_array);
	}
    else if(AC_MANAGE_DBUS_ERROR == ret) {
        vty_out(vty, "<error> failed get reply.\n");
    }
    else {
        vty_out(vty, "Fail to show trap switch.\n");
    }

    return ;
}


DEFUN(show_trap_swtch_info_func,
	show_trap_swtch_info_cmd,
	"show trap switch info",
	SHOW_STR
	"snmp trap\n"
	"snmp trap swtch\n"
	"configure inforamtion\n"
)
{
    if(SNMP_NODE == vty->node) {        
        vty_out(vty,"======================================================================\n");
        int i = 1;
        for(i = 1; i < MAX_SLOT; i++) {
            if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                vty_out(vty, "TRAP SLOT             : %d \n", i );  
                dcli_show_trap_switch(dbus_connection_dcli[i]->dcli_dbus_connection, vty);
                vty_out(vty,"======================================================================\n");
            }
        }
    }    
    else {
        vty_out (vty, "Terminal mode change must under SNMP mode!\n");
    }
	return CMD_SUCCESS;
}

static void
dcli_show_trap_parameter(DBusConnection *connection, struct vty *vty) {
    
	TRAPParameter *parameter_array = NULL;
	unsigned int parameter_num = 0;

	unsigned int ret = AC_MANAGE_SUCCESS;
	ret = ac_manage_show_trap_parameter(connection, &parameter_array, &parameter_num);
	if(AC_MANAGE_SUCCESS == ret) {
        
        if(parameter_num) {
            vty_out(vty,"----------------------------------------------------------------------\n");            
            int i = 0;
            for(i = 0; i < parameter_num; i++) {
                vty_out(vty, "%-25s : %d\n", parameter_array[i].paraStr, parameter_array[i].data);
            }
            vty_out(vty,"----------------------------------------------------------------------\n");
        }
        else {
            vty_out(vty, "Fail to show trap parameter.\n");
        }
        
        MANAGE_FREE(parameter_array);
	}
    else if(AC_MANAGE_DBUS_ERROR == ret) {
        vty_out(vty, "<error> failed get reply.\n");
    }
    else {
        vty_out(vty, "Fail to show trap parameter.\n");
    }

    return ;
}

DEFUN(show_trap_params_func,
	show_trap_params_cmd,
	"show trap params",
	SHOW_STR
	"snmp trap\n"
	"trap params\n"
)
{
    if(SNMP_NODE == vty->node) {        
        vty_out(vty,"======================================================================\n");
        int i = 1;
        for(i = 1; i < MAX_SLOT; i++) {
            if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                vty_out(vty, "TRAP SLOT             : %d \n", i );  
                vty_out(vty, "Trap Parameter Config\n");
                dcli_show_trap_parameter(dbus_connection_dcli[i]->dcli_dbus_connection, vty);
                vty_out(vty,"======================================================================\n");
            }
        }
    }    
    else {
        vty_out (vty, "Terminal mode change must under SNMP mode!\n");
    }
	return CMD_SUCCESS;
}

DEFUN(snmp_string_to_ascii_func,
	snmp_string_to_ascii_cmd,
	"snmp string_to_ascii STRING",
	SETT_STR
	"snmp string to ascii\n"
	"snmp string to ascii\n"
)
{
    
    if(0 == argc){        
        vty_out(vty, "Input error!\n");
        return ;
    }
    
    char *temp_string = argv[0];
    
    char string[128] = { 0 };
    unsigned int len = strlen(temp_string);
    
    char temp[10] = { 0 };
    snprintf(temp, sizeof(temp) - 1, "%d", len);
    strncat(string, temp, sizeof(string) - 1);
    
    int i = 0;
    for(i = 0; i < len; i++) {
            memset(temp, 0, sizeof(temp));

            if(temp_string[i] >= 'a' && temp_string[i] <= 'z') {
                temp_string[i] = temp_string[i] - 'a' + 'A';
            }

            snprintf(temp, sizeof(temp) - 1, ".%d", temp_string[i]);
            strncat(string, temp, sizeof(string) - strlen(string) - 1);
    }
    vty_out(vty, "%s\n", string);
    
	return CMD_SUCCESS;
}

DEFUN(manual_set_mib_acif_stats_func,
	manual_set_mib_acif_stats_cmd,
	"set acif stats IFNAME (acIfInNUcastPkts|acIfInDiscardPkts|acIfInErrors|acIfInMulticastPkts" \
	                        "|acIfOutDiscardPkts|acIfOutErrors|acIfOutNUcastPkts|acIfOutMulticastPkts) VAULE",
	SETT_STR
	"set ac interface stats\n"
	"ac interface name\n"
)
{
        
    struct mib_acif_stats acif_node = { 0 };
    strncpy(acif_node.ifname, argv[0], sizeof(acif_node.ifname) - 1);

    unsigned int value = atoi(argv[2]);
    
    if(0 == strcmp("acIfInNUcastPkts", argv[1])) {
        acif_node.acIfInNUcastPkts = value;
    }
    else if(0 == strcmp("acIfInDiscardPkts", argv[1])) {
        acif_node.acIfInDiscardPkts = value;
    }
    else if(0 == strcmp("acIfInErrors", argv[1])) {
        acif_node.acIfInErrors = value;
    }
    else if(0 == strcmp("acIfInMulticastPkts", argv[1])) {
        acif_node.acIfInMulticastPkts = value;
    }
    else if(0 == strcmp("acIfOutDiscardPkts", argv[1])) {
        acif_node.acIfOutDiscardPkts = value;
    }
    else if(0 == strcmp("acIfOutErrors", argv[1])) {
        acif_node.acIfOutErrors = value;
    }
    else if(0 == strcmp("acIfOutNUcastPkts", argv[1])) {
        acif_node.acIfOutNUcastPkts = value;
    }
    else if(0 == strcmp("acIfOutMulticastPkts", argv[1])) {
        acif_node.acIfOutMulticastPkts = value;
    }
    
    int ret = ac_manage_manual_set_mib_acif_stats(dcli_dbus_connection, &acif_node);
    if(AC_MANAGE_SUCCESS == ret) {
        vty_out(vty, "set acif stats %s %s success\n", argv[0], argv[1]);
    }
    else {
        vty_out(vty, "set acif stats %s %s failed!\n", argv[0], argv[1]);
    }
    
	return CMD_SUCCESS;
}

DEFUN(snmp_test_extend_command_func,
	snmp_test_extend_command_cmd,
	"exec slot ID (dcli|system) .COMMAND",
	"exec extend command\n"
    "exec slot ID command\n"
    "slot ID\n"
    "exec dcli extend command\n"
    "exec system extend command\n"
)
{
    unsigned int slot_id = 0;
    unsigned int command_type;

    slot_id = atoi(argv[0]);
    if(0 == slot_id || slot_id >= MAX_SLOT) {
        vty_out(vty, "The slot id (%d) input error\n", slot_id);
        return CMD_WARNING;
    }
    
    if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
        vty_out(vty, "The slot(%d) is not connect\n", slot_id);
        return CMD_WARNING;
    }
    
    if(0 == strcmp("dcli", argv[1])) {
        command_type = AC_MANAGE_EXTEND_COMMAND_DCLI;
    }
    else if(0 == strcmp("system", argv[1])){
        command_type = AC_MANAGE_EXTEND_COMMAND_SYSTEM;
    }
    else {
        vty_out(vty, "%s input error\n", argv[1]);
        return CMD_WARNING;
    }

    unsigned int count = argc;
    char command[1024] = { 0 };
    
    int i;
    for(i = 2; ; i++) {
    
        if((sizeof(command) - 1) < (strlen(command) + strlen(argv[i]))) {
            vty_out(vty, "input command exit max len\n");
            return CMD_WARNING;
        }
        
        strcat(command, argv[i]);
        
        if((i + 2) > count) {
            break;
        }

        strcat(command, " ");
    }
    
    struct command_return_s *command_return;
    
    int ret = AC_MANAGE_SUCCESS;
    ret = ac_manage_exec_extend_command(dbus_connection_dcli[slot_id]->dcli_dbus_connection,
                                        command_type,
                                        command,
                                        &command_return);

    if(AC_MANAGE_SUCCESS == ret) {
        struct command_return_s *node_return = NULL;
        for(node_return = command_return; NULL != node_return; node_return = node_return->next) {
            vty_out(vty, "%s", node_return->returnString);
        }
    }
    else if(AC_MANAGE_DBUS_ERROR == ret) {
        vty_out(vty, "exec command timeout!\n");
    }
    else if(AC_MANAGE_INPUT_TYPE_ERROR == ret) {
        vty_out(vty, "input command format error!\n");
    }
    else if(AC_MANAGE_FILE_OPEN_FAIL == ret) {
        vty_out(vty, "exec command failed!\n");
    }
    else if(AC_MANAGE_MALLOC_ERROR == ret) {
        vty_out(vty, "memory malloc failed!\n");
    }
    else {
        vty_out(vty, "unknown fail reason\n");
    }

    free_ac_manage_command_return(&command_return);
    
	return CMD_SUCCESS;
}

DEFUN(debug_em_switch_func,
	debug_em_switch_cmd,
	"config debug-em <1-2> (enable|disable)",
	"config \n"
	"config debug-em\n"
	"config debug-em <1-2>\n"
	"enable debug-em <1-2>\n"
	"disable debug-em <1-2>\n"
)
{
	int debug_em_type = 0;
	int debug_em_switch = 0;
	debug_em_type = atoi(argv[0]);
	if(debug_em_type<1 || debug_em_type>2)
	{
		vty_out(vty,"<error> input value should be 1 to 2.\n");
		return CMD_WARNING;
	}
	
	if (!strcmp(argv[1], "enable")){
		debug_em_switch = 1;
	}
	else if (!strcmp(argv[1], "disable")) {
		debug_em_switch = 0;
	}
	else {
        vty_out(vty, "input value should be enable or disable!\n");
        return CMD_WARNING;
	}	

	if(CONFIG_NODE == vty->node) {
        int i = 1;
        for(i = 1; i < MAX_SLOT; i++) {
            if(dbus_connection_dcli[i]->dcli_dbus_connection) {
				if(1 == debug_em_type)/*ac start trap*/
				{
					ac_trap_set_flag("/var/run/ac_restart_trap_flag",debug_em_switch);
					dcli_bsd_copy_file_to_board_v2(dcli_dbus_connection,i,"/var/run/ac_restart_trap_flag","/var/run/ac_restart_trap_flag",1,BSD_TYPE_NORMAL);
				}
				else if(2 == debug_em_type)/*standby switch trap*/
				{
					ac_trap_set_flag("/var/run/standby_switch_trap_flag",debug_em_switch);
					dcli_bsd_copy_file_to_board_v2(dcli_dbus_connection,i,"/var/run/standby_switch_trap_flag","/var/run/standby_switch_trap_flag",1,BSD_TYPE_NORMAL);
				}
            }
        }
    }

	return CMD_SUCCESS;
}


DEFUN(debug_vrrp_preempt_switch_func,
	debug_vrrp_preempt_switch_cmd,
	"config vrrp preempt (enable|disable) [INTERVAL]",
	"config \n"
	"config vrrp preempt\n"
	"config vrrp preempt\n"
	"enable vrrp preempt\n"
	"disable vrrp preempt interval\n"
)
{
	int interval = 20;
	int preempt_switch = 0;
	
	if (!strcmp(argv[0], "enable")){
		preempt_switch = 1;
	}
	else if (!strcmp(argv[0], "disable")) {
		preempt_switch = 0;
	}
	else {
        vty_out(vty, "input value should be enable or disable!\n");
        return CMD_WARNING;
	}	

	if(argc == 2){
		interval = atoi(argv[1]);
		if(interval<10 || interval>90)
		{
			vty_out(vty,"<error> input value should be 10 to 90.\n");
			return CMD_WARNING;
		}
	}
	if(CONFIG_NODE == vty->node) {
        int i = 1;
        for(i = 1; i < MAX_SLOT; i++) {
            if(dbus_connection_dcli[i]->dcli_dbus_connection) {
				if(1 == preempt_switch)
				{
					ac_set_vrrp_preempt(1,interval);
					dcli_bsd_copy_file_to_board_v2(dcli_dbus_connection,i,"/var/run/preempt_state","/var/run/preempt_state",1,BSD_TYPE_NORMAL);
					dcli_bsd_copy_file_to_board_v2(dcli_dbus_connection,i,"/var/run/preempt_interval","/var/run/preempt_interval",1,BSD_TYPE_NORMAL);
				}else{
					ac_set_vrrp_preempt(0,20);
					dcli_bsd_copy_file_to_board_v2(dcli_dbus_connection,i,"/var/run/preempt_state","/var/run/preempt_state",1,BSD_TYPE_NORMAL);
					dcli_bsd_copy_file_to_board_v2(dcli_dbus_connection,i,"/var/run/preempt_interval","/var/run/preempt_interval",1,BSD_TYPE_NORMAL);
				}
            }
        }
    }

	return CMD_SUCCESS;
}

DEFUN(config_stype_func,
	config_stype_cmd,
	"config stype <0-1>",
	"config \n"
	"config stype\n"
	"config stype <0-1>\n"
)
{
	int stype = 0;
	stype = atoi(argv[0]);
	if(stype<0 || stype>1)
	{
		vty_out(vty,"<error> input value should be 0 to 1.\n");
		return CMD_WARNING;
	}
	
	if(CONFIG_NODE == vty->node) {
        int i = 1;
        for(i = 1; i < MAX_SLOT; i++) {
            if(dbus_connection_dcli[i]->dcli_dbus_connection) {
				ac_trap_set_flag("/var/run/ac_sn_type",stype);
				dcli_bsd_copy_file_to_board_v2(dcli_dbus_connection,i,"/var/run/ac_sn_type","/var/run/ac_sn_type",1,BSD_TYPE_NORMAL);
            }
        }
    }

	return CMD_SUCCESS;
}

DEFUN(config_sysoid_func,
	config_sysoid_cmd,
	"config sysoid <0-255>",
	"config \n"
	"config sysoid\n"
	"config sysoid <0-255>\n"
)
{
	unsigned int sysoid = 0;
	sysoid = atoi(argv[0]);
	if(sysoid<0 || sysoid>255)
	{
		vty_out(vty,"<error> input value should be 0 to 255.\n");
		return CMD_WARNING;
	}
	
	if(CONFIG_NODE == vty->node) {
        int i = 1;
        for(i = 1; i < MAX_SLOT; i++) {
            if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                int ret = AC_MANAGE_SUCCESS;
                ret = ac_manage_config_snmp_sysoid_boardtype(dbus_connection_dcli[i]->dcli_dbus_connection, sysoid);

                if(AC_MANAGE_SUCCESS == ret) {
                    //vty_out(vty, "Slot(%d) :config sysoid %s success\n", i, argv[0]);
                }
                else if(AC_MANAGE_DBUS_ERROR == ret) {
                    vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
                }
                else{
                    vty_out(vty, "Slot(%d) :config sysoid %s failed!\n", i, argv[0]);
                }
            }
        }
    }
	int ret = AC_MANAGE_SUCCESS;

	if(SNMP_NODE == vty->node) {	    
        if(snmp_cllection_mode(dcli_dbus_connection)) {
            int i = 1;
            for(i = 1; i < MAX_SLOT; i++) {
                if(dbus_connection_dcli[i]->dcli_dbus_connection) {
               		ret = ac_manage_config_snmp_sysoid_boardtype(dbus_connection_dcli[i]->dcli_dbus_connection, sysoid);
                    if(AC_MANAGE_SUCCESS == ret) {
                        //vty_out(vty, "Slot(%d) :config sysoid %s success\n", i, argv[0]);
                    }
                    else if(AC_MANAGE_DBUS_ERROR == ret) {
                        vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
                    }
                    else{
	                    vty_out(vty, "Slot(%d) :config sysoid %s failed!\n", i, argv[0]);
	                }
                }
            }  
            
            return CMD_SUCCESS;
        }
        else {
            ret = ac_manage_config_snmp_sysoid_boardtype(dcli_dbus_connection, sysoid);
        }
    }
    else if(SNMP_SLOT_NODE == vty->node) {
        unsigned int slot_id = vty->slotindex;
        if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
            
            ret = ac_manage_config_snmp_sysoid_boardtype(dbus_connection_dcli[slot_id]->dcli_dbus_connection, sysoid);
        }
        else {  
            vty_out(vty, "The slot %d tipc dbus is not connect!!\n", slot_id);
            return CMD_WARNING;
        }
    }
    else {
		vty_out (vty, "Terminal mode change must under configure snmp mode!\n");
		return CMD_WARNING;
    }

	if(AC_MANAGE_SUCCESS == ret) {
        //vty_out(vty, "config sysoid success\n");
    }
    else if(AC_MANAGE_DBUS_ERROR == ret) {
        vty_out(vty, "<error> failed get reply.\n");
    }
    else {
        vty_out(vty, "config sysoid failed\n");
    }

	return CMD_SUCCESS;
}

static void dcli_show_trap_record_info(char *file_path, struct vty *vty) {
    if(NULL == file_path || NULL == vty)
        return ;
    
	FILE *fp = NULL;
	fp = fopen(file_path, "r");
	if(fp)
	{		
		char buff[256] = {0};
		char temp_buff[256] = { 0 };
		char send_time[20] = { 0 };
		unsigned int temp = 0;
		char ins_id[10] = { 0 };
		unsigned int local_id = 0;
		unsigned int instance_id = 0;
		unsigned int trap_version = 0;
		char DesIP[20] = { 0 };
		unsigned int ip1 = 0, ip2 = 0, ip3 = 0, ip4 = 0;
		unsigned int dest_port = 0; 
		char trap_oid[64] = { 0 };

		vty_out(vty,"==========================================================================================================\n");
		vty_out(vty,"%-20s %-15s %-20s %-12s %-17s\n","Time","InstanceID","DesIP","DesPort","TrapOID");
		
		while(fgets(buff, sizeof(buff), fp) != NULL) {
			memset(temp_buff, 0, sizeof(temp_buff));
			memset(send_time, 0, sizeof(send_time));
			sscanf(buff, "%[^info]%[^\n]", send_time, temp_buff);

			temp = 0;
			local_id = 0;
			instance_id = 0;
			trap_version = 0;
			ip1 = 0;
			ip2 = 0;
			ip3 = 0;
			ip4 = 0;
			dest_port = 0; 
			memset(trap_oid, 0, sizeof(trap_oid));
			sscanf(temp_buff, "info trap-helper[%u]: send trap: instance id = %u-%u, trap version = %u, ip = %u.%u.%u.%u, port = %u, trap_oid = %s",
			&temp, &local_id, &instance_id, &trap_version, &ip1, &ip2, &ip3, &ip4, &dest_port, trap_oid);

			memset(ins_id, 0, sizeof(ins_id));
			snprintf(ins_id, sizeof(ins_id)-1, "%u-%u", local_id, instance_id);

			memset(DesIP, 0, sizeof(DesIP));
			snprintf(DesIP, sizeof(DesIP)-1, "%u.%u.%u.%u", ip1, ip2, ip3, ip4);
			
			vty_out(vty, "%-20s %-15s %-20s %-12u %s\n", send_time, ins_id, DesIP, dest_port, trap_oid);
			
			memset(buff, 0, sizeof(buff));
		}
		vty_out(vty,"==========================================================================================================\n");
	}		
	else
	{
		vty_out(vty, "open file error!\n");
	}	
}


DEFUN(show_trap_record_func,
	show_trap_record_cmd,
	"show slot SLOTID trap record",
	SHOW_STR
	"trap record\n"
	"record trap\n"
)
{
	int slot_id = 0;
	int local_slot_id = 0;
	char des_path[32] = { 0 };
	char command[32] = { 0 };

	slot_id = atoi(argv[0]);
	if(slot_id < 1 || slot_id >= MAX_SLOT)
	{
		vty_out(vty,"<error> slot id should be 1 to %d.\n",MAX_SLOT-1);
		return CMD_WARNING;
	}

	if(VALID_DBM_FLAG == get_dbm_effective_flag())
	{
		local_slot_id = get_product_info(PRODUCT_LOCAL_SLOTID);
	}
	if(local_slot_id <= 0)
	{
		vty_out(vty,"<error> get local slot id fail.\n");
		return CMD_WARNING;
	}

	memset(des_path, 0, sizeof(des_path));
	snprintf(des_path, sizeof(des_path)-1, "/var/log/trap%d.log",slot_id);

	if(SNMP_NODE == vty->node) {	    
        if(slot_id != local_slot_id) {
            if(dbus_connection_dcli[slot_id]->dcli_dbus_connection) {
				dcli_bsd_copy_file_to_board_v2(dbus_connection_dcli[slot_id]->dcli_dbus_connection,local_slot_id,"/var/log/trap.log",des_path,1,BSD_TYPE_NORMAL);

           		dcli_show_trap_record_info(des_path, vty);

				memset(command, 0, sizeof(command));
				snprintf(command, sizeof(command)-1, "rm -rf %s", des_path);
				system(command);
            }  
        }
        else {
            dcli_show_trap_record_info("/var/log/trap.log", vty);
        }
    }    
    else {
		vty_out (vty, "show trap record must under configure snmp mode!\n");
		return CMD_WARNING;
    }

	return CMD_SUCCESS;
}


DEFUN(config_snmp_para_func,
	config_snmp_para_cmd,
	"config snmp maxmem <150-500>",
	"config \n"
	"config snmp maxmem\n"
	"config snmp maxmem <150-500>M\n"
)
{
	unsigned int type = 1;//maxmem
	unsigned int maxmem = 0;
	maxmem = atoi(argv[0]);
	if(maxmem<150 || maxmem>500)
	{
		vty_out(vty,"<error> input value should be 150 to 500 M.\n");
		return CMD_WARNING;
	}
	
	int ret = AC_MANAGE_SUCCESS;
	if(SNMP_NODE == vty->node) {	    
        int i = 1;
        for(i = 1; i < MAX_SLOT; i++) {
            if(dbus_connection_dcli[i]->dcli_dbus_connection) {
           		ret = ac_manage_config_mem_status_dog(dbus_connection_dcli[i]->dcli_dbus_connection, type, maxmem);
                if(AC_MANAGE_SUCCESS == ret) {
                    //vty_out(vty, "Slot(%d) :config snmp maxmem %s success\n", i, argv[0]);
                }
                else if(AC_MANAGE_DBUS_ERROR == ret) {
                    vty_out(vty, "Slot(%d) :<error> failed get reply.\n", i);
                }
                else{
                    vty_out(vty, "Slot(%d) :config snmp maxmem %s failed!\n", i, argv[0]);
                }
            }
        } 	
		
		return CMD_SUCCESS;
    }
    else {
		vty_out (vty, "Terminal mode change must under configure snmp mode!\n");
		return CMD_WARNING;
    }
}

DEFUN(show_snmp_running_config_func,
	show_snmp_running_config_cmd,
	"show snmp running config",
	SHOW_STR
	"show snmp running config\n" 
)
{
    vty_out(vty, "============================================================================\n");
    vty_out(vty, "config snmp\n");
    
    int i = 1;
    for(i = 1; i < MAX_SLOT; i++) {
        if(dbus_connection_dcli[i]->dcli_dbus_connection) {
            ac_manage_show_snmp_running_config(dbus_connection_dcli[i]->dcli_dbus_connection, SHOW_RUNNING_SNMP_MANUAL, vty);
        }
    }
    
    if(snmp_cllection_mode(dcli_dbus_connection)) {  
        vty_out(vty, " open snmp decentralized collect\n");
        
        for(i = 1; i < MAX_SLOT; i++) {
            if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                ac_manage_show_snmp_running_config(dbus_connection_dcli[i]->dcli_dbus_connection, SHOW_RUNNING_SNMP_CONFIG, vty);
            }
        }
    }
    else {
        ac_manage_show_snmp_running_config(dcli_dbus_connection, SHOW_RUNNING_SNMP_CONFIG, vty);
    }
    
    for(i = 1; i < MAX_SLOT; i++) {
        if(dbus_connection_dcli[i]->dcli_dbus_connection) {
            ac_manage_show_snmp_running_config(dbus_connection_dcli[i]->dcli_dbus_connection, SHOW_RUNNING_TRAP_RECEIVER, vty);
        }
    }
    
    ac_manage_show_snmp_running_config(dcli_dbus_connection, SHOW_RUNNING_TRAP_CONFIG, vty);

    vty_out(vty, " exit\n");
    vty_out(vty, "============================================================================\n");
	return CMD_SUCCESS;
}


static int
dcli_snmp_show_running_config(struct vty *vty) {
    
	do {
		char _tmpstr[64];
		memset(_tmpstr, 0, 64);
		sprintf(_tmpstr, BUILDING_MOUDLE, "snmp");
		vtysh_add_show_string(_tmpstr);
	} while(0);

	if (1 == ac_trap_get_flag("/var/run/ac_restart_trap_flag"))
	{
		vtysh_add_show_string("config debug-em 1 enable");
	}
	if (1 == ac_trap_get_flag("/var/run/standby_switch_trap_flag"))
	{
		vtysh_add_show_string("config debug-em 2 enable");
	}
    
	if (1 == ac_trap_get_flag("/var/run/preempt_state"))
	{
		int interval = 20;
		char temp[128] = { 0 };
		
		interval = ac_preempt_interval_trap_has_sent();
		
		memset(temp, 0, sizeof(temp));		
		snprintf(temp, sizeof(temp)-1, "config vrrp preempt enable %d", interval);
		vtysh_add_show_string(temp);
	}

	if (1 == ac_trap_get_flag("/var/run/ac_sn_type"))
	{
		vtysh_add_show_string("config stype 1");
	}
	else
	{
		vtysh_add_show_string("config stype 0");
	}
	
    vtysh_add_show_string("config snmp");
    
    int i = 1;
    for(i = 1; i < MAX_SLOT; i++) {
        if(dbus_connection_dcli[i]->dcli_dbus_connection) {
            ac_manage_show_snmp_running_config(dbus_connection_dcli[i]->dcli_dbus_connection, SHOW_RUNNING_SNMP_MANUAL, NULL);
        }
    }
    
    if(snmp_cllection_mode(dcli_dbus_connection)) {  
        vtysh_add_show_string(" open snmp decentralized collect");
        
        for(i = 1; i < MAX_SLOT; i++) {
            if(dbus_connection_dcli[i]->dcli_dbus_connection) {
                ac_manage_show_snmp_running_config(dbus_connection_dcli[i]->dcli_dbus_connection, SHOW_RUNNING_SNMP_CONFIG, NULL);
            }
        }
    }
    else {
        ac_manage_show_snmp_running_config(dcli_dbus_connection, SHOW_RUNNING_SNMP_CONFIG, NULL);
    }
    
    for(i = 1; i < MAX_SLOT; i++) {
        if(dbus_connection_dcli[i]->dcli_dbus_connection) {
            ac_manage_show_snmp_running_config(dbus_connection_dcli[i]->dcli_dbus_connection, SHOW_RUNNING_TRAP_RECEIVER, NULL);
        }
    }
    
    ac_manage_show_snmp_running_config(dcli_dbus_connection, SHOW_RUNNING_TRAP_CONFIG, NULL);

	FILE *fp = NULL;
	fp = popen("sed -n '/SNMPD_MAXMEM=/p' /usr/bin/mem_status_dog.sh", "r");	
	if(fp) 
	{
		char buf[255] = { 0 };
		memset(buf, 0, sizeof(buf));
		fgets(buf, sizeof(buf), fp);
		pclose(fp);
		fp = NULL;

		unsigned int mem = 0;
		sscanf(buf, "SNMPD_MAXMEM=%d", &mem);
		if(200000 != mem)
		{
			memset(buf, 0, sizeof(buf));
			sprintf(buf, " config snmp maxmem %d",(mem/1000));
			vtysh_add_show_string(buf);
		}
	}	
    
    vtysh_add_show_string(" exit");

    return CMD_SUCCESS;
}



void 
dcli_snmp_init(void) {
    
	install_element(ENABLE_NODE, &show_snmp_base_info_cmd);

	install_element(VIEW_NODE, &show_snmp_base_info_cmd); 	
	install_element(CONFIG_NODE, &show_snmp_base_info_cmd);

	install_element(CONFIG_NODE, &show_snmp_running_config_cmd);
    
	install_element(HIDDENDEBUG_NODE, &conf_acmanage_log_debug_cmd);	
	install_element(HIDDENDEBUG_NODE, &conf_acmanage_token_debug_cmd);		
	install_element(HIDDENDEBUG_NODE, &conf_snmp_log_debug_cmd);
	install_element(HIDDENDEBUG_NODE, &conf_trap_log_debug_cmd);
    
	install_element(CONFIG_NODE, &conf_snmp_cmd);	
	install_node(&snmp_node, dcli_snmp_show_running_config, "SNMP_NODE");
	install_default(SNMP_NODE);

	install_element(SNMP_NODE, &snmp_string_to_ascii_cmd);

	install_element(SNMP_NODE, &show_snmp_running_config_cmd);
	install_element(SNMP_NODE, &show_snmp_base_info_cmd);
	install_element(SNMP_NODE, &show_snmp_dbus_connection_list_cmd);

	install_element(SNMP_NODE, &snmp_manual_set_hansi_cmd);
	install_element(SNMP_NODE, &clear_snmp_manual_set_hansi_cmd);

	install_element(SNMP_NODE, &update_snmp_sysinfo_cmd);
    
	install_element(SNMP_NODE, &switch_service_cmd);
	install_element(SNMP_NODE, &set_snmp_mode_cmd);
	install_element(SNMP_NODE, &set_snmp_cache_time_cmd);
	install_element(SNMP_NODE, &snmp_config_collect_interface_port_cmd);
    
	install_element(SNMP_NODE, &show_community_info_cmd);
	install_element(SNMP_NODE, &show_view_info_cmd);
	install_element(SNMP_NODE, &add_snmp_community_cmd);	
	install_element(SNMP_NODE, &set_snmp_community_cmd);
	install_element(SNMP_NODE, &delete_snmp_community_by_name_cmd);
	
	install_element(SNMP_NODE, &show_group_info_cmd);
	install_element(SNMP_NODE, &add_snmp_group_cmd);
	install_element(SNMP_NODE, &delete_snmp_group_cmd);
	
	install_element(SNMP_NODE, &show_v3user_info_cmd);
	install_element(SNMP_NODE, &add_snmp_v3user_cmd);
	install_element(SNMP_NODE, &add_snmp_v3user_none_auth_cmd);
	install_element(SNMP_NODE, &add_snmp_v3user_none_private_cmd);
	install_element(SNMP_NODE, &delete_snmp_v3user_cmd);

	install_element(SNMP_NODE, &create_snmp_view_cmd);
	install_element(SNMP_NODE, &delete_snmp_view_cmd);
	install_element(SNMP_NODE, &conf_snmp_view_cmd);	
	install_node(&snmp_view_node, NULL, "SNMP_VIEW_NODE");
	install_default(SNMP_VIEW_NODE);
	install_element(SNMP_VIEW_NODE, &config_snmp_view_oid_cmd);

	install_element(SNMP_NODE, &set_trap_service_cmd);	
	install_element(SNMP_NODE, &config_trap_bind_local_receiver_v1v2c_cmd);	
	install_element(SNMP_NODE, &config_trap_bind_local_receiver_v3_cmd);	
	install_element(SNMP_NODE, &config_trap_receiver_v1v2c_cmd);	
	install_element(SNMP_NODE, &config_trap_receiver_v3_cmd);
	install_element(SNMP_NODE, &delete_snmp_trap_cmd);	
	
#if 0	
	install_element(SNMP_NODE, &set_trap_statistics_interval_cmd);	
	install_element(SNMP_NODE, &set_trap_sampling_interval_cmd);	
	install_element(SNMP_NODE, &set_trap_cpu_threshold_cmd);	
	install_element(SNMP_NODE, &set_trap_memory_threshold_cmd);	
	install_element(SNMP_NODE, &set_trap_temperature_threshold_cmd);
#endif

    
	install_element(SNMP_NODE, &set_trap_instance_heartbeat_cmd);	
	install_element(SNMP_NODE, &clear_trap_instance_heartbeat_cmd);	
	install_element(SNMP_NODE, &set_trap_heartbeat_interval_cmd);	
	install_element(SNMP_NODE, &set_trap_heartbeat_mode_cmd);	
	install_element(SNMP_NODE, &set_trap_resend_info_cmd);
	install_element(SNMP_NODE, &set_trap_type_switch_cmd);
	install_element(SNMP_NODE, &set_trap_group_switch_cmd);

    
	install_element(VIEW_NODE, &show_trap_info_cmd);
	install_element(SNMP_NODE, &show_trap_info_cmd);
	install_element(SNMP_NODE, &show_trap_swtch_info_cmd);
	install_element(SNMP_NODE, &show_trap_params_cmd);

	install_element(SNMP_NODE, &config_snmp_decentralized_collect_cmd);

	install_element(SNMP_NODE, &conf_snmp_decentral_slot_cmd);
	install_element(SNMP_NODE, &config_sysoid_cmd);
	install_element(SNMP_NODE, &config_snmp_para_cmd);
	install_element(SNMP_NODE, &show_trap_record_cmd);
	install_node(&snmp_slot_node, NULL, "SNMP_SLOT_NODE");
	install_default(SNMP_SLOT_NODE);

	install_element(SNMP_SLOT_NODE, &show_snmp_base_info_cmd);
	install_element(SNMP_SLOT_NODE, &show_snmp_dbus_connection_list_cmd);

	install_element(SNMP_SLOT_NODE, &switch_service_cmd);
	install_element(SNMP_SLOT_NODE, &snmp_config_collect_interface_port_cmd);	
	install_element(SNMP_SLOT_NODE, &set_snmp_mode_cmd);
	install_element(SNMP_SLOT_NODE, &set_snmp_cache_time_cmd);

	install_element(SNMP_SLOT_NODE, &update_snmp_sysinfo_cmd);

	install_element(SNMP_SLOT_NODE, &show_community_info_cmd);
	install_element(SNMP_SLOT_NODE, &add_snmp_community_cmd);	
	install_element(SNMP_SLOT_NODE, &set_snmp_community_cmd);
	install_element(SNMP_SLOT_NODE, &delete_snmp_community_by_name_cmd);

	install_element(SNMP_SLOT_NODE, &create_snmp_view_cmd);
	install_element(SNMP_SLOT_NODE, &conf_snmp_view_cmd);
	install_element(SNMP_SLOT_NODE, &delete_snmp_view_cmd);
	install_element(SNMP_SLOT_NODE, &show_view_info_cmd);

	install_element(SNMP_SLOT_NODE, &show_group_info_cmd);
	install_element(SNMP_SLOT_NODE, &add_snmp_group_cmd);
	install_element(SNMP_SLOT_NODE, &delete_snmp_group_cmd);

	install_element(SNMP_SLOT_NODE, &show_group_info_cmd);
	install_element(SNMP_SLOT_NODE, &add_snmp_v3user_cmd);
	install_element(SNMP_SLOT_NODE, &add_snmp_v3user_none_auth_cmd);
	install_element(SNMP_SLOT_NODE, &add_snmp_v3user_none_private_cmd);
	install_element(SNMP_SLOT_NODE, &delete_snmp_v3user_cmd);
	install_element(SNMP_SLOT_NODE, &config_sysoid_cmd);

#if 0    
	install_element(SNMP_SLOT_NODE, &set_trap_service_cmd);	
	install_element(SNMP_SLOT_NODE, &add_snmp_trap_v1v2c_cmd);	
	install_element(SNMP_SLOT_NODE, &add_snmp_trap_v3_cmd);	
	install_element(SNMP_SLOT_NODE, &set_snmp_trap_v1v2c_cmd);	
	install_element(SNMP_SLOT_NODE, &set_snmp_trap_v3_cmd);	
	install_element(SNMP_SLOT_NODE, &delete_snmp_trap_cmd);	
	install_element(SNMP_SLOT_NODE, &set_trap_statistics_interval_cmd);	
	install_element(SNMP_SLOT_NODE, &set_trap_sampling_interval_cmd);	
	install_element(SNMP_SLOT_NODE, &set_trap_heartbeat_interval_cmd);	
	install_element(SNMP_SLOT_NODE, &set_trap_heartbeat_mode_cmd);	
	install_element(SNMP_SLOT_NODE, &set_trap_cpu_threshold_cmd);	
	install_element(SNMP_SLOT_NODE, &set_trap_memory_threshold_cmd);	
	install_element(SNMP_SLOT_NODE, &set_trap_temperature_threshold_cmd);
	install_element(SNMP_SLOT_NODE, &set_trap_type_switch_cmd);
	install_element(SNMP_SLOT_NODE, &set_trap_instance_info_cmd);
	install_element(SNMP_SLOT_NODE, &delete_trap_instance_info_cmd);
	
	install_element(SNMP_SLOT_NODE, &show_trap_info_cmd);
	install_element(SNMP_SLOT_NODE, &show_trap_swtch_info_cmd);
	install_element(SNMP_SLOT_NODE, &show_trap_params_cmd);
	install_element(SNMP_SLOT_NODE, &show_trap_instance_info_cmd);
#endif	
    
	install_element(HIDDENDEBUG_NODE, &manual_set_mib_acif_stats_cmd);

	install_element(CONFIG_NODE, &snmp_test_extend_command_cmd);
	install_element(CONFIG_NODE, &debug_em_switch_cmd);
	install_element(CONFIG_NODE, &debug_vrrp_preempt_switch_cmd);
	install_element(CONFIG_NODE, &config_stype_cmd);
}



#ifdef __cplusplus
}
#endif


