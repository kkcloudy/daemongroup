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
* dcli_vrrp.c
*
*
* CREATOR:
*		zhengcs@autelan.com
*
* DESCRIPTION:
*		CLI definition for VRRP module.
*
* DATE:
*		06/16/2009	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.45 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#if 0
#include <zebra.h>
#include <lib/vty.h>
#include "vtysh/vtysh.h"
#include <dbus/dbus.h>
#include <stdlib.h>
#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>
#include <util/npd_list.h>

#include "sysdef/returncode.h"
#include "dcli_cavium_ratelimit.h"
#include "command.h"
#include "memory.h"
#endif

#include <string.h>
#include <sys/socket.h>
#include <zebra.h>
#include <dbus/dbus.h>
#include <dbus/sem/sem_dbus_def.h>
#include <sys/wait.h>

#include "command.h"
#include "dcli_main.h"
#include "dcli_sem.h"
#include "dcli_system.h"
#include "board/board_define.h"
#include "bsd/bsdpub.h"
#include "dcli_cavium_ratelimit.h"
#include "sysdef/returncode.h"


struct cmd_node cvm_rate_limit_node =
{
	TRAFFIC_POLICER_NODE,
	"	",
	1,
};
/* check parameter for decimal and hexadecimal digits string */
#define PARAMETER_NUM_CHECK(argv) 	if(num_check((argv),strlen((argv))))\
									{\
										vty_out(vty, "%% Bad parameter input: %s !\n", (argv));\
										return CMD_WARNING;\
									}

#define PARAMETER_DECIMAL_NUM_CHECK(argv) 	if(decimal_num_check((argv),strlen((argv))))\
									{\
										vty_out(vty, "%% Bad parameter input: %s !\n", (argv));\
										return CMD_WARNING;\
									}


static int get_product_info(char *filename)
{
	int fd;
	char buff[16] = {0};
	unsigned int data;

	if((filename == NULL) || (buff == NULL))
	{
		return -1;
	}

	fd = open(filename, O_RDONLY, 0);
	if (fd >= 0) 
	{
		if(read(fd, buff, 16) < 0) 
			printf("Read error : no value\n");
		/* for bug: AXSSZFI-292, "popen failed for pager: Too many open files" */
		close(fd);
	}
	else
	{        
		printf("Open file:%s error!\n",filename);
		return -1;
	}

	data = strtoul(buff, NULL, 10);

	return data;
}


DEFUN(cvm_rate_log_level_set_func,
	  cvm_rate_log_level_set_cmd,
	  "debug traffic-policer slot SLOT_ID (none|error|warning|debug)",
	  "Debug switch\n"
	  "Debug for traffic-policer\n"
	  "the slot id \n"
	  "the slot id num \n"
	  "Debug set Off\n"
	  "Set log level to error\n"
	  "Set log level to warning(including error)\n"
	  "set log level to debug(including warning and error)\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	unsigned int level = 0;
	unsigned int op_ret = 0;
	
	enum log_level{
		NONE = 0,
		ERROR = 1,
		WARNING = 2,
		DEBUG = 3
	};
	unsigned int slot_id = 0;
	char *endptr = NULL;
    int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);

    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}


	if(argc != 2){return CMD_WARNING;}
	
	if(!strncmp(argv[1], "none", strlen(argv[1])))
	{
		level = NONE;
	}
	else if(!strncmp(argv[1], "error", strlen(argv[1])))
	{
		level = ERROR;
	}
	else if(!strncmp(argv[1], "warning", strlen(argv[1])))
	{
		level = WARNING;
	}
	else if(!strncmp(argv[1], "debug", strlen(argv[1])))
	{
		level = DEBUG;
	}
	else
	{
		
		vty_out(vty, "%% Bad parameter input: %s !\n",argv[1]);
		
		return CMD_WARNING;
	}
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
    {

    	query = dbus_message_new_method_call(CVM_RATE_LIMIT_DBUS_BUSNAME,
    										 CVM_RATE_LIMIT_DBUS_OBJPATH,
    										 CVM_RATE_LIMIT_DBUS_INTERFACE,
    										 CVM_RATE_LIMIT_DBUS_METHOD_LOG_LEVEL_SET);
    	
    	dbus_error_init(&err);
    	
    	dbus_message_append_args(query,
    							 DBUS_TYPE_UINT32,&level, 
    							 DBUS_TYPE_INVALID);
    	
    	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection, query, -1, &err);
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		
    		printf("Traffic-policer enable set failed get reply!\n");
    		
    		if (dbus_error_is_set(&err)) {
    			
    			printf("%s raised: %s",err.name,err.message);
    			
    			dbus_error_free_for_dcli(&err);
    		}
    		
    		return CMD_WARNING;
    	}
    	else if (dbus_message_get_args ( reply, &err,
    				DBUS_TYPE_UINT32,&op_ret,
    				DBUS_TYPE_INVALID))
    	{
    		if(CVM_RATELIMIT_RETURN_CODE_SUCCESS != op_ret)
    		{
    			
    			vty_out(vty, "%% Traffic-policer enable set failed, ret %#x!\n", op_ret);
    			
    			dbus_message_unref(reply);
    			
    			return CMD_WARNING;
    		}
    	} 
    	else 
    	{		
    		if (dbus_error_is_set(&err)) 
    		{
    		
    			printf("%s raised: %s",err.name,err.message);
    			
    			dbus_error_free_for_dcli(&err);
    		}
    	}
    	
    	dbus_message_unref(reply);
    	
    	return CMD_SUCCESS;
    }
	else 
	{
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

/* modified by zhengbo for hide ethernet rate trigger options */
DEFUN(cvm_rate_enable_set_4fastfwd,
	  cvm_rate_enable_set_4fastfwd_cmd,
	  "config traffic-policer slot SLOT_ID (enable|disable) phase1",
	  CONFIG_STR
	  "Config traffic-policer\n"
	  "the slot id \n"
	  "the slot id num \n"
	  "Config traffic-policer enable\n"
	  "Config traffic-policer disable\n"
	  "Config phase1 traffic-policer enable/disable\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	unsigned int enable = 0;
	unsigned int op_ret = 0;
	unsigned int flag = 0;
	unsigned int slot_id = 0;
	char *endptr = NULL;
    int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);

    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	
	if(2 != argc)
	{
		vty_out(vty, "%% Bad parameter number!\n");
		return CMD_WARNING;
	}

#if 0
	if(!strncmp(argv[2], "phase1", strlen(argv[2])))
	{
		flag = 2;
	}
	else
	{
		vty_out(vty, "%% Bad parameter %s!\n", argv[2]);
		return CMD_WARNING;
	}
#else
	flag = 2;
#endif

	if(!strncmp(argv[1], "enable", strlen(argv[1])))
	{
		enable = 1;
	}
	else if(!strncmp(argv[1], "disable", strlen(argv[1])))
	{
		enable = 0;
	}
	else{
		
		vty_out(vty, "%% Bad parameter input: %s !\n",argv[1]);
		return CMD_WARNING;
	}

	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
    {
    	query = dbus_message_new_method_call(CVM_RATE_LIMIT_DBUS_BUSNAME,
    										 CVM_RATE_LIMIT_DBUS_OBJPATH,
    										 CVM_RATE_LIMIT_DBUS_INTERFACE,
    										 CVM_RATE_LIMIT_DBUS_METHOD_ENABLE_SET);
    	
    	dbus_error_init(&err);
    	
    	dbus_message_append_args(query,
    							 DBUS_TYPE_UINT32,&enable, 
    							 DBUS_TYPE_UINT32,&flag, 
    							 DBUS_TYPE_INVALID);
    	
    	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection, query, -1, &err);
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		
			printf("Traffic policer enable set failed get reply!\n");
    		
    		if (dbus_error_is_set(&err)) {
    			
    			printf("%s raised: %s",err.name,err.message);
    			
    			dbus_error_free_for_dcli(&err);			
    		}
    		
    		return CMD_WARNING;
    	}
    	else if (dbus_message_get_args ( reply, &err,
    				DBUS_TYPE_UINT32,&op_ret,
    				DBUS_TYPE_INVALID))
    	{	
    	
    		if(CVM_RATELIMIT_RETURN_CODE_SUCCESS != op_ret) 
    		{
    			
				if(CVM_RATELIMIT_RETURN_CODE_MODULE_NOTRUNNING == op_ret)
				{
					vty_out(vty, "%% Traffic-policer module is not started, start it first!\n");
				}
				else
				{
					vty_out(vty, "%% Traffic-policer enable set failed, ret %#x!\n", op_ret);
				}
    			dbus_message_unref(reply);
    			
    			return CMD_WARNING;
    		}
    	} 
    	else 
    	{		
    		if (dbus_error_is_set(&err)) 
    		{
    		
    			printf("%s raised: %s",err.name,err.message);
    			
    			dbus_error_free_for_dcli(&err);
    		}
    	}
    	
    	dbus_message_unref(reply);
    	
    	return CMD_SUCCESS;
    }
	else 
	{
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(cvm_rate_enable_set_4ethernet,
	  cvm_rate_enable_set_4ethernet_cmd,
	  "config traffic-policer slot SLOT_ID (enable|disable) phase2",
	  CONFIG_STR
	  "Config traffic-policer\n"
	  "the slot id \n"
	  "the slot id num \n"
	  "Config traffic-policer enable\n"
	  "Config traffic-policer disable\n"
	  "Config phase2 traffic-policer enable/disable\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	unsigned int enable = 0;
	unsigned int op_ret = 0;
	unsigned int flag = 0;
	unsigned int slot_id = 0;
	char *endptr = NULL;
    int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);

    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}

	if(argc != 2)
	{
		vty_out(vty, "%% Bad parameter number!\n");
		return CMD_WARNING;
	}

#if 0
	if(!strncmp(argv[2], "phase2", strlen(argv[2])))
	{
		flag = 1;
	}
	else
	{
		vty_out(vty, "%% Bad parameter %s!\n", argv[2]);
		return CMD_WARNING;
	}
#else
	flag = 1;
#endif

	if(!strncmp(argv[1], "enable", strlen(argv[1])))
	{
		enable = 1;
	}
	else if(!strncmp(argv[1], "disable", strlen(argv[1])))
	{
		enable = 0;
	}
	else{
		
		vty_out(vty, "%% Bad parameter input: %s !\n",argv[1]);
		return CMD_WARNING;
	}

	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
    {
    	query = dbus_message_new_method_call(CVM_RATE_LIMIT_DBUS_BUSNAME,
    										 CVM_RATE_LIMIT_DBUS_OBJPATH,
    										 CVM_RATE_LIMIT_DBUS_INTERFACE,
    										 CVM_RATE_LIMIT_DBUS_METHOD_ENABLE_SET);
    	
    	dbus_error_init(&err);
    	
    	dbus_message_append_args(query,
    							 DBUS_TYPE_UINT32,&enable, 
    							 DBUS_TYPE_UINT32,&flag, 
    							 DBUS_TYPE_INVALID);
    	
    	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection, query, -1, &err);
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		
			printf("Traffic policer enable set failed get reply!\n");
    		
    		if (dbus_error_is_set(&err)) {
    			
    			printf("%s raised: %s",err.name,err.message);
    			
    			dbus_error_free_for_dcli(&err);			
    		}
    		
    		return CMD_WARNING;
    	}
    	else if (dbus_message_get_args ( reply, &err,
    				DBUS_TYPE_UINT32,&op_ret,
    				DBUS_TYPE_INVALID))
    	{	
    	
    		if(CVM_RATELIMIT_RETURN_CODE_SUCCESS != op_ret) 
    		{
    			
				if(CVM_RATELIMIT_RETURN_CODE_MODULE_NOTRUNNING == op_ret)
				{
					vty_out(vty, "%% Traffic-policer module is not started, start it first!\n");
				}
				else
				{
					vty_out(vty, "%% Traffic-policer enable set failed, ret %#x!\n", op_ret);
				}
    			dbus_message_unref(reply);
    			
    			return CMD_WARNING;
    		}
    	} 
    	else 
    	{		
    		if (dbus_error_is_set(&err)) 
    		{
    		
    			printf("%s raised: %s",err.name,err.message);
    			
    			dbus_error_free_for_dcli(&err);
    		}
    	}
    	
    	dbus_message_unref(reply);
    	
    	return CMD_SUCCESS;
    }
	else 
	{
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

#if 0
ALIAS(cvm_rate_enable_set_func,
	  cvm_rate_enable_set_cmd,
	  "config traffic-policer slot SLOT_ID (enable|disable)",
	  CONFIG_STR
	  "Config traffic-policer\n"
	  "the slot id \n"
	  "the slot id num \n"
	  "Config traffic-policer enable\n"
	  "Config traffic-policer disable\n"
);
#endif

static int protocol_name_check(unsigned char * name, int name_len)
{/*valid characters: decimal digits, letters, underline */
	int i = 0;
	
	if(!name) return -1;
	
	for(i = 0; i < name_len; i++)
	{
		if(!((name[i] >= '0' && name[i] <= '9')||\
			(name[i] >= 'a' && name[i] <= 'z')||
			(name[i] >= 'A' && name[i] <='Z')||\
			(name[i] == '_')))
		{
			return -1;
		}
	}
	
	return 0;
}

static int decimal_num_check(unsigned char * name, int name_len)
{   /*not including minus */
	int i = 0;
	
	if(!name || !name_len) return -1;

	if((name[0] == '0') && (name_len > 1))
	{
		return -1;
	}

	for(i = 0; i < name_len; i++)
	{
		if(!(name[i] >= '0' && name[i] <= '9'))
		{
			return -1;
		}
	}
	
	return 0;
}

static int hex_num_check(unsigned char * name, int name_len)
{
	int i = 0;
	
	if(!name || !name_len) return -1;	
	
	if((name[0] == '0') && (name_len > 1))
	{
		if(name_len < 3) return -1;
		if((name[1] != 'x') && (name[1] != 'X')) return -1;
		for(i = 2; i < name_len; i++)
		{
			if(!((name[i] >= '0' && name[i] <= '9')||\
				 (name[i] >= 'A' && name[i] <= 'F')||\
				 (name[i] >= 'a' && name[i] <= 'f')))
			{
				return -1;
			}
		}
		return 0;
	}
	
	return -1;
}
static int num_check(unsigned char * name, int name_len)
{/*valid characters: decimal digits, hexadecimal digits, decimal not including minus */
	int i = 0;
	
	if(!name || !name_len) return -1;
	
	if(hex_num_check(name, name_len))
	{
		return decimal_num_check(name, name_len);
	}
	
	return 0;
}

static int rules_array_parse(unsigned char * inputStr, unsigned int len, MATCH_TYPE *rules)
{
#define PARSE_TYPE unsigned int

	unsigned char * tmp  =  NULL;
	int i = 0;
	int slen = 0;
	PARSE_TYPE *rules_ptr = rules;
	unsigned int int_len = (len*MATCH_TYPE_LEN)/sizeof(PARSE_TYPE);
	
	if(!inputStr || !rules)
	{
		return -1;
	}
	
	slen = strlen(inputStr);
	
	for(i = 0; i < slen; i++)
	{
	if(!(((inputStr[i] >= '0') && (inputStr[i] <= '9'))||\
		((inputStr[i] >= 'A') && (inputStr[i] <= 'F'))||\
		((inputStr[i] >= 'a') && (inputStr[i] <= 'f'))||\
		inputStr[i] == ','))
		{
			return -1;
		}
	}
	
	memset(rules, 0, len*sizeof(MATCH_TYPE));
	
	if(len == 0)
	{
		return 0;
	}
	
	tmp = strtok(inputStr, ",");
	
	i = 0;
	
	while(tmp && i < int_len)
	{
		rules_ptr[i++] = strtoul(tmp,NULL,16);
		
		tmp = strtok(NULL, ",");
	}
	
	return 0;
}

DEFUN(cvm_rate_rules_add_simp_func,
	  cvm_rate_rules_add_simp_cmd,
	  "add traffic-policer-rules slot SLOT_ID NAME (ip-tcp|ip-udp) sport (any|PORTNO) dport (any|PORTNO) (LIMITER|nolimit)",
	  "Add system function\n"
	  "Add traffic-policer-rules\n"
	  "the slot id \n"
	  "the slot id num \n"
	  "Traffic-policer-rule's name\n"
	  "Traffic-policer-rule's ip-tcp\n"
	  "Traffic-policer-rule's ip-udp\n"
	  "Traffic-policer-rule's sport\n"
	  "Traffic-policer-rule's sport any\n"
	  "Traffic-policer-rule's sport num\n"
	  "Traffic-policer-rule's dport\n"
	  "Traffic-policer-rule's dport any\n"
	  "Traffic-policer-rule's dport num\n"
	  "Traffic-policer-rule's limiter <0-5000000>\n"
	  "Traffic-policer-rule's limiter nolimit\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	unsigned char name[PROTOCOL_NAME_LEN + 1] = {0};
	unsigned char * namePtr = name;
	int name_len = 0;
	unsigned int limiter = 0;
	int ret = 0;
	unsigned int iptype = 0;
	unsigned int sport = 0;
	unsigned int dport = 0;
	unsigned int slot_id = 0;
	char *endptr = NULL;
    int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);

    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	
	if(6 != argc)
	{
		vty_out(vty, "%% Bad parameters !");
		return CMD_WARNING;
	}

	name_len = strlen(argv[1]);
	
	name_len = (name_len > PROTOCOL_NAME_LEN) ? PROTOCOL_NAME_LEN : name_len;
	
	strncpy(name, argv[1], name_len);
	
	if(protocol_name_check(name, name_len))
	{
		vty_out(vty, "%% Bad parameter input: %s !\n",argv[1]);
		
		return CMD_WARNING;
	}

	if(!strncmp(argv[2], "ip-tcp", strlen(argv[1])))
	{
		iptype = CVM_RATE_IPTCP;
	}
	else if(!strncmp(argv[2], "ip-udp", strlen(argv[1])))
	{
		iptype = CVM_RATE_IPUDP;
	}
	else
	{
		vty_out(vty, "%% Bad parameter input: %s !\n",argv[1]);
		
		return CMD_WARNING;
	}

	if(!strncmp(argv[3], "any", strlen(argv[3])))
	{
		sport = CVM_RATE_PORT_ANY;
	}
	else
	{
		PARAMETER_NUM_CHECK(argv[3]);
		
		sport = (unsigned int) strtoul(argv[3], NULL, 0);
		
		if(sport > 65535)
		{
			vty_out(vty, "%% Bad parameter input: %s !\n", argv[2]);
			
			return CMD_WARNING;
		}
	}

	if(!strncmp(argv[4], "any", strlen(argv[4])))
	{
		dport = CVM_RATE_PORT_ANY;
	}
	else
	{
		PARAMETER_NUM_CHECK(argv[4]);
		
		dport = (unsigned int) strtoul(argv[4], NULL, 0);
		
		if(dport > 65535)
		{
			vty_out(vty, "%% Bad parameter input: %s !\n", argv[3]);
			
			return CMD_WARNING;
		}
	}

	if(!strncmp(argv[5], "nolimit", strlen(argv[5])))
	{
		limiter = CVM_RATE_NO_LIMIT;
	}
	else
	{
		PARAMETER_NUM_CHECK(argv[5]);
		
		limiter = strtoul(argv[5], NULL, 0);
		
		if(limiter >= CVM_RATE_NO_LIMIT || limiter < 0)
		{
			vty_out(vty, "%% Bad parameter input: %s !\n", argv[5]);
			
			return CMD_WARNING;
		}
	}
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
    {
    	query = dbus_message_new_method_call(CVM_RATE_LIMIT_DBUS_BUSNAME,
    										 CVM_RATE_LIMIT_DBUS_OBJPATH,
    										 CVM_RATE_LIMIT_DBUS_INTERFACE,
    										 CVM_RATE_LIMIT_DBUS_METHOD_ADD_RULES_SIMP);
    	
    	dbus_error_init(&err);
    	
    	if(dbus_message_append_args(query,
    							DBUS_TYPE_STRING,&namePtr, 
    							DBUS_TYPE_UINT32,&limiter,
    							DBUS_TYPE_UINT32,&iptype,
    							DBUS_TYPE_UINT32,&sport,
    							DBUS_TYPE_UINT32,&dport,
    							DBUS_TYPE_INVALID))
    							
    	if(query)
    	{
    	
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection, query, -1, &err);
    		
    		dbus_message_unref(query);
    	}
    	if (NULL == reply)
    	{
    		printf("Traffic-policer add rules failed get reply.\n");
    		
    		if (dbus_error_is_set(&err))
    		{
    			printf("%s raised: %s",err.name,err.message);
    			
    			dbus_error_free_for_dcli(&err);
    		}
    		
    		return CMD_WARNING;
    	}
    	else if (dbus_message_get_args ( reply, &err,
    				DBUS_TYPE_UINT32,&op_ret,
    				DBUS_TYPE_INVALID))
    	{	
    		if(CVM_RATELIMIT_RETURN_CODE_SUCCESS != op_ret) 
    		{
    			if(CVM_RATELIMIT_RETURN_CODE_RULE_EXIST == op_ret)
    			{
    				vty_out(vty, "%% The Rule \"%s\" already exists!\n", argv[1]);
    			}
    			else if(CVM_RATELIMIT_RETURN_CODE_RULE_FULL == op_ret)
    			{
    				vty_out(vty, "%% No more position for new rule!\n");
    			}
    			else if(CVM_RATELIMIT_RETURN_CODE_INVALID_RULE == op_ret)
    			{
    				vty_out(vty, "%% Mask should not all zero!");
    			}
    			else
    			{
    				vty_out(vty, "%% Traffic policer add rules failed, ret %#x!\n", op_ret);
    			}
    			
    			dbus_message_unref(reply);
    			
    			return CMD_WARNING;
    		}
    	} 
    	else 
    	{		
    		if (dbus_error_is_set(&err)) 
    		{
    			printf("%s raised: %s",err.name,err.message);
    			
    			dbus_error_free_for_dcli(&err);
    		}
    	}
    	
    	dbus_message_unref(reply);
    	
    	return CMD_SUCCESS;
    }
	else 
	{
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}


DEFUN(cvm_rate_rules_add_func,
	  cvm_rate_rules_add_cmd,
	  "add traffic-policer-rules slot SLOT_ID NAME RULES MASK (LIMITER|nolimit)",
	  "Add system function\n"
	  "Add traffic-policer-rules\n"
	  "the slot id \n"
	  "the slot id num \n"
	  "Traffic-policer-rules' name\n"
	  "Traffic-policer-rules' rules\n"
	  "Traffic-policer-rules' mask\n"
	  "Traffic-policer-rules' limiter <0-5000000>\n"
	  "Traffic-policer-rules' limiter nolimit\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	unsigned char name[PROTOCOL_NAME_LEN] = {0};
	unsigned char * namePtr = name;
	MATCH_TYPE rules[PACKET_MATCH_BYTE_NUM] = {0};
	MATCH_TYPE mask[PACKET_MATCH_BYTE_NUM] = {0};
	int name_len = 0;
	unsigned int limiter = 0;
	int ret = 0;
	unsigned int slot_id = 0;
	char *endptr = NULL;
    int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);

    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}


	if(argc != 5){return CMD_WARNING;}
	
	name_len = strlen(argv[1]);
	name_len = (name_len > PROTOCOL_NAME_LEN) ? PROTOCOL_NAME_LEN : name_len;
	
	strncpy(name, argv[1], name_len);
	
	if(protocol_name_check(name, name_len)){
		vty_out(vty, "%% Bad parameter input: %s !\n",argv[1]);
		return CMD_WARNING;
	}
	
	ret = rules_array_parse(argv[2], PACKET_MATCH_BYTE_NUM, rules);
	
	if(ret)
	{
		vty_out(vty,"%% Bad parameter input: %s ret %#x!\n", argv[2],ret);
		return CMD_WARNING;
	}
	
	ret = rules_array_parse(argv[3], PACKET_MATCH_BYTE_NUM, mask);
	
	if(ret)
	{
		vty_out(vty,"%% Bad parameter input: %s !\n", argv[3]);
		return CMD_WARNING;
	}
	
	if(!strncmp(argv[4], "nolimit", strlen(argv[4])))
	{
		limiter = CVM_RATE_NO_LIMIT;
	}
	else
	{
		PARAMETER_NUM_CHECK(argv[4]);
		
		limiter = strtoul(argv[4], NULL, 0);
		
		if(limiter >= CVM_RATE_NO_LIMIT)
		{
			vty_out(vty, "%% Bad parameter input: %s !\n", argv[4]);
			return CMD_WARNING;
		}
	}
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
    {
    	query = dbus_message_new_method_call(CVM_RATE_LIMIT_DBUS_BUSNAME,
    										 CVM_RATE_LIMIT_DBUS_OBJPATH,
    										 CVM_RATE_LIMIT_DBUS_INTERFACE,
    										 CVM_RATE_LIMIT_DBUS_METHOD_ADD_RULES);
    	
    	dbus_error_init(&err);
    	
    	dbus_message_append_args(query,
    							DBUS_TYPE_STRING,&namePtr, 
    							DBUS_MATCH_TYPE,&rules[0],
    							DBUS_MATCH_TYPE,&rules[1],
    							DBUS_MATCH_TYPE,&rules[2],
    							DBUS_MATCH_TYPE,&rules[3],
    							DBUS_MATCH_TYPE,&rules[4],
    							DBUS_MATCH_TYPE,&rules[5],
    							DBUS_MATCH_TYPE,&rules[6],
    							DBUS_MATCH_TYPE,&rules[7],
    							DBUS_MATCH_TYPE,&rules[8],
    							DBUS_MATCH_TYPE,&rules[9],
    							DBUS_MATCH_TYPE,&rules[10],
    							DBUS_MATCH_TYPE,&rules[11],
    							DBUS_MATCH_TYPE,&rules[12],
    							DBUS_MATCH_TYPE,&rules[13],
    							DBUS_MATCH_TYPE,&rules[14],
    							DBUS_MATCH_TYPE,&rules[15],
#if 0
    							DBUS_TYPE_UINT32,&rules[16],
    							DBUS_TYPE_UINT32,&rules[17],
    							DBUS_TYPE_UINT32,&rules[18],
    							DBUS_TYPE_UINT32,&rules[19],
    							DBUS_TYPE_UINT32,&rules[20],
    							DBUS_TYPE_UINT32,&rules[21],
    							DBUS_TYPE_UINT32,&rules[22],
    							DBUS_TYPE_UINT32,&rules[23],
    							DBUS_TYPE_UINT32,&rules[24],
    							DBUS_TYPE_UINT32,&rules[25],
    							DBUS_TYPE_UINT32,&rules[26],
    							DBUS_TYPE_UINT32,&rules[27],
    							DBUS_TYPE_UINT32,&rules[28],
    							DBUS_TYPE_UINT32,&rules[29],
    							DBUS_TYPE_UINT32,&rules[30],
    							DBUS_TYPE_UINT32,&rules[31],
#endif
    							DBUS_MATCH_TYPE,&mask[0],
    							DBUS_MATCH_TYPE,&mask[1],
    							DBUS_MATCH_TYPE,&mask[2],
    							DBUS_MATCH_TYPE,&mask[3],
    							DBUS_MATCH_TYPE,&mask[4],
    							DBUS_MATCH_TYPE,&mask[5],
    							DBUS_MATCH_TYPE,&mask[6],
    							DBUS_MATCH_TYPE,&mask[7],
    							DBUS_MATCH_TYPE,&mask[8],
    							DBUS_MATCH_TYPE,&mask[9],
    							DBUS_MATCH_TYPE,&mask[10],
    							DBUS_MATCH_TYPE,&mask[11],
    							DBUS_MATCH_TYPE,&mask[12],
    							DBUS_MATCH_TYPE,&mask[13],
    							DBUS_MATCH_TYPE,&mask[14],
    							DBUS_MATCH_TYPE,&mask[15],
#if 0
    							DBUS_TYPE_UINT32,&mask[16],
    							DBUS_TYPE_UINT32,&mask[17],
    							DBUS_TYPE_UINT32,&mask[18],
    							DBUS_TYPE_UINT32,&mask[19],
    							DBUS_TYPE_UINT32,&mask[20],
    							DBUS_TYPE_UINT32,&mask[21],
    							DBUS_TYPE_UINT32,&mask[22],
    							DBUS_TYPE_UINT32,&mask[23],
    							DBUS_TYPE_UINT32,&mask[24],
    							DBUS_TYPE_UINT32,&mask[25],
    							DBUS_TYPE_UINT32,&mask[26],
    							DBUS_TYPE_UINT32,&mask[27],
    							DBUS_TYPE_UINT32,&mask[28],
    							DBUS_TYPE_UINT32,&mask[29],
    							DBUS_TYPE_UINT32,&mask[30],
    							DBUS_TYPE_UINT32,&mask[31],
#endif
    							DBUS_TYPE_UINT32,&limiter,
    							DBUS_TYPE_INVALID);

    	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection, query, -1, &err);
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		printf("Traffic-policer add rules failed get reply.\n");
    		if (dbus_error_is_set(&err)) {
    			printf("%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_WARNING;
    	}
    	else if (dbus_message_get_args ( reply, &err,
    				DBUS_TYPE_UINT32,&op_ret,
    				DBUS_TYPE_INVALID))
    	{	
    		if(CVM_RATELIMIT_RETURN_CODE_SUCCESS != op_ret) 
    		{
    			if(CVM_RATELIMIT_RETURN_CODE_RULE_EXIST == op_ret)
    			{
    				vty_out(vty, "%% The Rule \"%s\" already exists!\n", argv[1]);
    			}
    			else if(CVM_RATELIMIT_RETURN_CODE_RULE_FULL == op_ret)
    			{
    				vty_out(vty, "%% No more position for new rule!\n");
    			}
    			else if(CVM_RATELIMIT_RETURN_CODE_INVALID_RULE == op_ret)
    			{
    				vty_out(vty, "%% Mask should not all zero!");
    			}
    			else
    			{
    				vty_out(vty, "%% Traffic policer add rules failed, ret %#x!\n", op_ret);
    			}
    			
    			dbus_message_unref(reply);
    			
    			return CMD_WARNING;
    		}
    	} 
    	else 
    	{		
    		if (dbus_error_is_set(&err)) 
    		{
    			printf("%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    	}
    	
    	dbus_message_unref(reply);
    	
    	return CMD_SUCCESS;
    }
	else 
	{
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(cvm_rate_config_rules_source_func,
	  cvm_rate_config_rules_source_cmd,
	  "config traffic-policer-rules load-from slot SLOT_ID (file|console)",
	  "Config system information\n"
	  "Config traffic-policer rules\n"
	  "Config traffic-policer rules source\n"
	  "the slot id \n"
	  "the slot id num \n"
	  "Config traffic-policer rules from file\n"
	  "Config traffic-policer rules from console\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	unsigned int source = 0;
	unsigned int slot_id = 0;
	char *endptr = NULL;
    int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);

    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}

	if(2 != argc)
	{
		vty_out(vty, "%% Bad parameter number!\n");
		return CMD_WARNING;
	}
	if(!strncmp(argv[1], "console", strlen(argv[1])))
	{
		source = 0;
	}
	else if(!strncmp(argv[1], "file", strlen(argv[1])))
	{
		source = 1;
	}
	else 
	{
		vty_out(vty, "%% Bad parameter input %s!\n", argv[1]);
		return CMD_WARNING;
	}
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
    {
    	query = dbus_message_new_method_call(CVM_RATE_LIMIT_DBUS_BUSNAME,
    										 CVM_RATE_LIMIT_DBUS_OBJPATH,
    										 CVM_RATE_LIMIT_DBUS_INTERFACE,
    										 CVM_RATE_LIMIT_DBUS_METHOD_LOAD_RULES_SOURCE_SET);
    	dbus_error_init(&err);
    	
    	dbus_message_append_args(query,
    							DBUS_TYPE_UINT32,&source,
    							DBUS_TYPE_INVALID);
    	
    	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection, query, -1, &err);
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		
    		printf("Traffic-policer load rules failed get reply.\n");
    		
    		if (dbus_error_is_set(&err)) {
    			
    			printf("%s raised: %s",err.name,err.message);
    			
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_WARNING;
    	}
    	else if (dbus_message_get_args ( reply, &err,
    				DBUS_TYPE_UINT32,&op_ret,
    				DBUS_TYPE_INVALID))
    	{	
    		if(CVM_RATELIMIT_RETURN_CODE_SUCCESS != op_ret) 
    		{
    			vty_out(vty, "%% Traffic-policer load rules failed, ret %#x!\n", op_ret);
    			
    			dbus_message_unref(reply);
    			
    			return CMD_WARNING;
    		}
    	} 
    	else 
    	{		
    		if (dbus_error_is_set(&err)) 
    		{
    			printf("%s raised: %s",err.name,err.message);
    			
    			dbus_error_free_for_dcli(&err);
    		}
    	}
    	
    	dbus_message_unref(reply);
    	
    	return CMD_SUCCESS;
    }
	else 
	{
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}


DEFUN(cvm_rate_load_rules_func,
	  cvm_rate_load_rules_cmd,
	  "load traffic-policer-rules slot SLOT_ID ",
	  "Load system config file\n"
	  "Load traffic-policer rules config file\n"
	  "the slot id \n"
	  "the slot id num \n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	unsigned int slot_id = 0;
	char *endptr = NULL;
    int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);

    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}
	
    if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
    {
    	query = dbus_message_new_method_call(CVM_RATE_LIMIT_DBUS_BUSNAME,
    										 CVM_RATE_LIMIT_DBUS_OBJPATH,
    										 CVM_RATE_LIMIT_DBUS_INTERFACE,
    										 CVM_RATE_LIMIT_DBUS_METHOD_LOAD_RULES_FROM_FILE);
    	dbus_error_init(&err);
    	
    	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection, query, -1, &err);
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		
    		printf("Traffic-policer load rules failed get reply.\n");
    		
    		if (dbus_error_is_set(&err)) 
    		{
    			printf("%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_WARNING;
    	}
    	else if (dbus_message_get_args ( reply, &err,
    				DBUS_TYPE_UINT32,&op_ret,
    				DBUS_TYPE_INVALID))
    	{	
    		if(CVM_RATELIMIT_RETURN_CODE_SUCCESS != op_ret) 
    		{
    			
    			vty_out(vty, "%% Traffic policer load rules failed, ret %#x!\n", op_ret);
    			
    			dbus_message_unref(reply);
    			
    			return CMD_WARNING;
    		}
    	} 
    	else 
    	{		
    		if (dbus_error_is_set(&err)) 
    		{
    			printf("%s raised: %s",err.name,err.message);
    			
    			dbus_error_free_for_dcli(&err);
    		}
    	}
    	
    	dbus_message_unref(reply);
    	
    	return CMD_SUCCESS;
    }
	else 
	{
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(cvm_rate_restore_rules_func,
	  cvm_rate_restore_rules_cmd,
	  "restore traffic-policer-rules slot SLOT_ID ",
	  "Restore rules to system config file\n"
	  "Restore rules to special config file\n"
	  "the slot id \n"
	  "the slot id num \n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	unsigned int slot_id = 0;
	char *endptr = NULL;
    int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);

    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}

    if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
    {
    	query = dbus_message_new_method_call(CVM_RATE_LIMIT_DBUS_BUSNAME,
    										 CVM_RATE_LIMIT_DBUS_OBJPATH,
    										 CVM_RATE_LIMIT_DBUS_INTERFACE,
    										 CVM_RATE_LIMIT_DBUS_METHOD_RESTORE_RULES_TO_FILE);
    	dbus_error_init(&err);
    	
    	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection, query, -1, &err);
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		
    		printf("Traffic-policer restore rules failed get reply.\n");
    		
    		if (dbus_error_is_set(&err)) {
    			
    			printf("%s raised: %s",err.name,err.message);
    			
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_WARNING;
    	}
    	else if (dbus_message_get_args ( reply, &err,
    				DBUS_TYPE_UINT32,&op_ret,
    				DBUS_TYPE_INVALID)){	
    				
    		if(CVM_RATELIMIT_RETURN_CODE_SUCCESS != op_ret) {
    			
    			vty_out(vty, "%% Traffic policer restore rules failed, ret %#x!\n", op_ret);
    			
    			dbus_message_unref(reply);
    			
    			return CMD_WARNING;
    		}
    	} 
    	else 
    	{		
    		if (dbus_error_is_set(&err)) 
    		{
    			printf("%s raised: %s",err.name,err.message);
    			
    			dbus_error_free_for_dcli(&err);
    		}
    	}
    	
    	dbus_message_unref(reply);
    	
    	return CMD_SUCCESS;
    }
	else 
	{
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}


DEFUN(cvm_rate_rules_del_func,
	  cvm_rate_rules_del_cmd,
	  "delete traffic-policer-rules slot SLOT_ID NAME",
	  "Delete system function\n"
	  "Delete traffic-policer-rules\n"
	  "the slot id \n"
	  "the slot id num \n"
	  "Traffic-policer-rules' name\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	unsigned char name[PROTOCOL_NAME_LEN] = {0};
	unsigned char * namePtr = name;
	int name_len = 0;
	unsigned int slot_id = 0;
	char *endptr = NULL;
    int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);

    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}


	if(argc != 2){return CMD_WARNING;}
	
	name_len = strlen(argv[1]);
	
	name_len = (name_len > PROTOCOL_NAME_LEN) ? PROTOCOL_NAME_LEN:name_len;
	
	strncpy(name, argv[1], name_len);
	
	if(protocol_name_check(name, name_len)){
		
		vty_out(vty, "%% Bad parameter input: %s !\n",argv[1]);
		
		return CMD_WARNING;
	}
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
    {
    	query = dbus_message_new_method_call(CVM_RATE_LIMIT_DBUS_BUSNAME,
    										 CVM_RATE_LIMIT_DBUS_OBJPATH,
    										 CVM_RATE_LIMIT_DBUS_INTERFACE,
    										 CVM_RATE_LIMIT_DBUS_METHOD_DEL_RULES);
    	
    	dbus_error_init(&err);
    	
    	dbus_message_append_args(query,
    							 DBUS_TYPE_STRING,&namePtr, 
    							 DBUS_TYPE_INVALID);
    	
    	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection, query, -1, &err);
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		
    		printf("Traffic-policer delete rules failed get reply.\n");
    		
    		if (dbus_error_is_set(&err)) {
    			
    			printf("%s raised: %s",err.name,err.message);
    			
    			dbus_error_free_for_dcli(&err);
    		}
    		
    		return CMD_WARNING;		
    	}
    	else if (dbus_message_get_args ( reply, &err,
    				DBUS_TYPE_UINT32,&op_ret,
    				DBUS_TYPE_INVALID))
    	{	
    	
    		if(CVM_RATELIMIT_RETURN_CODE_SUCCESS != op_ret) 
    		{
    			if(CVM_RATELIMIT_RETURN_CODE_RULE_NOTEXIST == op_ret)
    			{
    				vty_out(vty, "%% The Rule \"%s\" not exists!\n", argv[1]);
    			}
    			else if(CVM_RATELIMIT_RETURN_CODE_UNSUPPORT == op_ret)
    			{
    				vty_out(vty, "%% This Rule is not allowed to delete!\n", argv[1]);
    			}
    			else
    			{
    				vty_out(vty, "%% Traffic policer delete rules failed, ret %#x!\n", op_ret);
    			}
    			
    			dbus_message_unref(reply);
    			
    			return CMD_WARNING;
    		}
    	} 
    	else 
    	{		
    		if (dbus_error_is_set(&err)) 
    		{
    			printf("%s raised: %s",err.name,err.message);
    			
    			dbus_error_free_for_dcli(&err);
    		}
    	}
    	
    	dbus_message_unref(reply);
    	
    	return CMD_SUCCESS;
    }
	else 
	{
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(cvm_rate_rules_clear_func,
	  cvm_rate_rules_clear_cmd,
	  "clear traffic-policer-rules slot SLOT_ID ",
	  "Clear system configure\n"
	  "Clear traffic-policer-rules\n"
	  "the slot id \n"
	  "the slot id num \n"
)
{
#define CMD_LINE_LEN 256
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	unsigned char cmd[CMD_LINE_LEN] = {0};
	unsigned int slot_id = 0;
	char *endptr = NULL;
    int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);

    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}

	if(argc != 1){return CMD_WARNING;}
	
	
	printf( "Are you sure you want to clear all user added traffic policer rules,\n"\
			" and reset default rules limiter (yes/no)?");
		
	memset(cmd, 0, CMD_LINE_LEN);
	
	fgets(cmd, CMD_LINE_LEN-1, stdin);
	
	fflush(stdin);
	
	if ('\n' == cmd[strlen(cmd)-1])
	{
		cmd[strlen(cmd)-1] = '\0';
	}
	if(strncmp(cmd, "yes", strlen(cmd)))
	{
		vty_out(vty, "%% Command cancelled!\n");
		
		return CMD_SUCCESS;
	}
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
	{
	query = dbus_message_new_method_call(CVM_RATE_LIMIT_DBUS_BUSNAME,
										 CVM_RATE_LIMIT_DBUS_OBJPATH,
										 CVM_RATE_LIMIT_DBUS_INTERFACE,
										 CVM_RATE_LIMIT_DBUS_METHOD_CLEAR_RULES);
	
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		
		printf("Traffic-policer clear rules failed get reply.\n");
		
		if (dbus_error_is_set(&err)) {
			
			printf("%s raised: %s",err.name,err.message);
			
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_WARNING;
	}
	else if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&op_ret,
				DBUS_TYPE_INVALID))
	{	
		if(CVM_RATELIMIT_RETURN_CODE_SUCCESS != op_ret) 
		{			
		
			vty_out(vty, "%% Traffic policer clear rules failed, ret %#x!\n", op_ret);
			
			dbus_message_unref(reply);
			
			return CMD_WARNING;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{	
			printf("%s raised: %s",err.name,err.message);
			
			dbus_error_free_for_dcli(&err);
		}
	}
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
	}
	else 
	{
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(cvm_rate_rules_modify_func,
	  cvm_rate_rules_modify_cmd,
	  "config traffic-policer-rules slot SLOT_ID NAME (rule|mask|limiter) VALUE",
	  CONFIG_STR
	  "Config traffic-policer-rules\n"
	  "the slot id \n"
	  "the slot id num \n"
	  "Traffic-policer-rules' name\n"
	  "Traffic-policer-rules' rules\n"
	  "Traffic-policer-rules' mask\n"
	  "Traffic-policer-rules' limiter\n"
	  "Traffic-policer-rules' value <0-5000000>\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	unsigned char name[PROTOCOL_NAME_LEN] = {0};
	unsigned char * namePtr = name;
	MATCH_TYPE rules[PACKET_MATCH_BYTE_NUM] = {0};
	int name_len = 0;
	unsigned int limiter = 0;
	int type = 0;
	int ret = 0;
	unsigned int slot_id = 0;
	char *endptr = NULL;
    int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);

    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}

	if((4 != argc) && (2 != argc))
	{
		return CMD_WARNING;
	}
	
	name_len = strlen(argv[1]);
	
	name_len = (name_len > PROTOCOL_NAME_LEN) ? PROTOCOL_NAME_LEN:name_len;
	
	strncpy(name, argv[1], name_len);
	
	if(protocol_name_check(name, name_len)){
		
		vty_out(vty, "%% Bad parameter input: %s !\n",argv[1]);
		
		return CMD_WARNING;
	}
	if(4 == argc)
	{
		if(!strncmp(argv[2], "rule", strlen(argv[2])))
		{
			type = 1;
			
			ret = rules_array_parse(argv[3], PACKET_MATCH_BYTE_NUM, rules);
		}
		else if(!strncmp(argv[2], "mask", strlen(argv[2])))
		{
			type = 2;
			
			ret = rules_array_parse(argv[3], PACKET_MATCH_BYTE_NUM, rules);
		}
		else if(!strncmp(argv[2], "limiter", strlen(argv[2])))
		{
			type = 3;
			
			if(!strcmp(argv[3], "nolimit"))
			{
				limiter = CVM_RATE_NO_LIMIT;//means nolimit 
			}
			else
			{
				PARAMETER_NUM_CHECK(argv[3]);
		
				limiter = strtoul(argv[3], NULL, 0);
				
				if(limiter >= CVM_RATE_NO_LIMIT)
				{
					vty_out(vty, "%% Limiter value range is (<0-5000000>|nolimit) !\n");
				}
			}
		}
		else
		{
			vty_out(vty, "%% Bad parameter input: %s !\n", argv[2]);
			
			return CMD_WARNING;
		}
	}
	else
	{
		type = 3;
		
		limiter = CVM_RATE_NO_LIMIT;//means nolimit 
	}
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
	{
	query = dbus_message_new_method_call(CVM_RATE_LIMIT_DBUS_BUSNAME,
										 CVM_RATE_LIMIT_DBUS_OBJPATH,
										 CVM_RATE_LIMIT_DBUS_INTERFACE,
										 CVM_RATE_LIMIT_DBUS_METHOD_MODIFY_RULES);
	
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							DBUS_TYPE_STRING,&namePtr, 
							DBUS_TYPE_UINT32,&type,
							DBUS_MATCH_TYPE,&rules[0],
							DBUS_MATCH_TYPE,&rules[1],
							DBUS_MATCH_TYPE,&rules[2],
							DBUS_MATCH_TYPE,&rules[3],
							DBUS_MATCH_TYPE,&rules[4],
							DBUS_MATCH_TYPE,&rules[5],
							DBUS_MATCH_TYPE,&rules[6],
							DBUS_MATCH_TYPE,&rules[7],
							DBUS_MATCH_TYPE,&rules[8],
							DBUS_MATCH_TYPE,&rules[9],
							DBUS_MATCH_TYPE,&rules[10],
							DBUS_MATCH_TYPE,&rules[11],
							DBUS_MATCH_TYPE,&rules[12],
							DBUS_MATCH_TYPE,&rules[13],
							DBUS_MATCH_TYPE,&rules[14],
							DBUS_MATCH_TYPE,&rules[15],
#if 0
							DBUS_TYPE_UINT32,&rules[16],
							DBUS_TYPE_UINT32,&rules[17],
							DBUS_TYPE_UINT32,&rules[18],
							DBUS_TYPE_UINT32,&rules[19],
							DBUS_TYPE_UINT32,&rules[20],
							DBUS_TYPE_UINT32,&rules[21],
							DBUS_TYPE_UINT32,&rules[22],
							DBUS_TYPE_UINT32,&rules[23],
							DBUS_TYPE_UINT32,&rules[24],
							DBUS_TYPE_UINT32,&rules[25],
							DBUS_TYPE_UINT32,&rules[26],
							DBUS_TYPE_UINT32,&rules[27],
							DBUS_TYPE_UINT32,&rules[28],
							DBUS_TYPE_UINT32,&rules[29],
							DBUS_TYPE_UINT32,&rules[30],
							DBUS_TYPE_UINT32,&rules[31],
#endif
							DBUS_TYPE_UINT32,&limiter,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) 
	{
		printf("Traffic-policer add rules failed get reply.\n");
		
		if (dbus_error_is_set(&err)) {
			
			printf("%s raised: %s",err.name,err.message);
			
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_WARNING;
	}
	else if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&op_ret,
				DBUS_TYPE_INVALID))
	{	
		if(CVM_RATELIMIT_RETURN_CODE_SUCCESS != op_ret) 
		{
			if(CVM_RATELIMIT_RETURN_CODE_RULE_NOTEXIST == op_ret)
			{
				vty_out(vty, "%% The Rule \"%s\" not exists!\n", argv[1]);
			}
			else if(CVM_RATELIMIT_RETURN_CODE_UNSUPPORT == op_ret)
			{
				vty_out(vty, "%% This Rule is not allowed to change!\n");
			}
			else if(CVM_RATELIMIT_RETURN_CODE_INVALID_RULE == op_ret)
			{
				vty_out(vty, "%% Mask should not all zero!\n");
			}
			else
			{
				vty_out(vty, "%% Traffic policer add rules failed, ret %#x!\n", op_ret);
			}
			
			dbus_message_unref(reply);
			
			return CMD_WARNING;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			
			dbus_error_free_for_dcli(&err);
		}
	}
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
	}
	else 
	{
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

ALIAS(cvm_rate_rules_modify_func,
	  cvm_rate_rules_modify_limiter_cmd,
	  "config traffic-policer-rules slot SLOT_ID NAME limiter nolimit",
	  CONFIG_STR
	  "Config traffic-policer-rules\n"
	  "the slot id \n"
	  "the slot id num \n"
	  "Traffic-policer-rules' name\n"
	  "Traffic-policer-rules' limiter\n"
	  "Traffic-policer-rules' value nolimit\n"
);

int dcli_cvm_rate_show_rules
(
	struct vty * vty,
	unsigned char *name, 
	unsigned int rule_length,
	MATCH_TYPE *rules,
	MATCH_TYPE * mask, 
	unsigned int ratelimiter,
	STATISTIC_TYPE drop_counter,
	STATISTIC_TYPE pass_counter
)
{
#define INT_NUM_PER_LINE 4
#define SHOW_RULE_TYPE unsigned int

	int i = 0;
	SHOW_RULE_TYPE *rules_ptr = NULL;
	static const int show_type_len = sizeof(SHOW_RULE_TYPE);
	int type_rule_len = (rule_length + (show_type_len - 1))/show_type_len;
	
	if(name)
	{
		vty_out(vty, "%3sName:\t%s\n", "", name);
	}
	
	if(rules)
	{
		vty_out(vty, "%3sRule:\t", "");
		
		if(!rule_length)
		{
			vty_out(vty, "%04x %04x\n", 0, 0);
		}
		
		rules_ptr = rules;
		
		for(i = 0; i < type_rule_len; i++)
		{
			vty_out(vty, "%04x %04x%s", \
				(rules_ptr[i]>>16)&0xffff, \
				rules_ptr[i]&0xffff, \
				((i+1)%INT_NUM_PER_LINE)? " " : \
				((i < type_rule_len - 1) ? "\n\t\t" : ""));
		}
		
		vty_out(vty, "\n");
		
	}
	
	if(mask)
	{
		vty_out(vty, "%3sMask:\t", "");
		
		if(!rule_length)
		{
			vty_out(vty, "%04x %04x\n", 0, 0);
		}
		
		rules_ptr = mask;
		
		for(i = 0; i < type_rule_len; i++)
		{
			vty_out(vty, "%04x %04x%s",\
				(rules_ptr[i]>>16)&0xffff, \
				rules_ptr[i]&0xffff, \
				((i+1)%INT_NUM_PER_LINE)? " " : \
				((i < type_rule_len - 1) ? "\n\t\t" : ""));
		}
		
		vty_out(vty, "\n");
	}
	if(ratelimiter < CVM_RATE_NO_LIMIT)
	{
		vty_out(vty, "%3sRatelimiter:\t%d pps\n", "", ratelimiter);
	}
	else
	{
		vty_out(vty, "%3sRatelimiter:\t%s\n", "", "nolimit");
	}
	
	vty_out(vty, "%3sDrop counter:\t%llu packages\n", "", drop_counter);
	
	vty_out(vty, "%3sPass counter:\t%llu packages\n", "", pass_counter);
	
	return 0;
}

DEFUN(cvm_rate_clear_statistic_func,
	  cvm_rate_clear_statistic_4fastfwd_cmd,
	  "clear traffic-policer-statistic slot SLOT_ID (phase1|phase2)",
	  "Clear system function\n"
	  "Clear traffic-policer-statistic, drop counter and pass counter\n"
	  "the slot id \n"
	  "the slot id num \n"
	  "Clear phase1 traffic-policer-statistic, drop counter and pass counter\n"
	  "Clear phase2 traffic-policer-statistic, drop counter and pass counter\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	DBusMessageIter		iter;
	DBusMessageIter	 iter_array;
	int op_ret = 0;
	STATISTIC_TYPE drop_count = 0;
	STATISTIC_TYPE pass_count = 0;
	unsigned int flag = 0;
	unsigned int slot_id = 0;
	char *endptr = NULL;
    int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);

    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}

	if(!(2 == argc || 1 == argc))
	{
		vty_out(vty, "%% Bad parameter number!\n");
		
		return CMD_WARNING;
	}
	
	if(2 == argc)
	{
		if(!strncmp(argv[1], "phase2", strlen(argv[1])))
		{
			flag = 1;
		}
		else if(!strncmp(argv[1], "phase1", strlen(argv[1])))
		{
			flag = 2;
		}
		else
		{
			vty_out(vty, "%% Bad parameter %s!\n", argv[1]);
			
			return CMD_WARNING;
		}
	}
	else
	{
		flag = 3;
	}
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
    {
    	query = dbus_message_new_method_call(CVM_RATE_LIMIT_DBUS_BUSNAME,
    										 CVM_RATE_LIMIT_DBUS_OBJPATH,
    										 CVM_RATE_LIMIT_DBUS_INTERFACE,
    										 CVM_RATE_LIMIT_DBUS_METHOD_CLEAR_STATISTIC);
    	dbus_error_init(&err);
    	
    	dbus_message_append_args(query,
    							 DBUS_TYPE_UINT32,&flag, 
    							 DBUS_TYPE_INVALID);
    	
    	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection, query, -1, &err);
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		
    		printf("Traffic-policer show statistic failed get reply.\n");
    		
    		if (dbus_error_is_set(&err)) {
    			
    			printf("%s raised: %s",err.name,err.message);
    			
    			dbus_error_free_for_dcli(&err);
    		}
    		
    		return CMD_WARNING;
    	}
    	if (dbus_message_get_args(reply, &err,
    							 DBUS_TYPE_UINT32, &op_ret,
    							 DBUS_STATISTIC_TYPE, &drop_count,
    							 DBUS_STATISTIC_TYPE, &pass_count,
    							 DBUS_TYPE_INVALID))
    	{
    		if(CVM_RATELIMIT_RETURN_CODE_SUCCESS == op_ret)
    		{
    			vty_out(vty, "%s Statistic before %s clear:\n", (2 != flag) ? "Phase2" : "Phase1", (3 == flag) ? "both" : "");
    			vty_out(vty, "==========================================\n");
    			vty_out(vty, " drop counter: %llu\n", drop_count);
    			vty_out(vty, " pass counter: %llu\n", pass_count);
    			vty_out(vty, "==========================================\n");
    		}
    		else
    		{
    			vty_out(vty, "%% Clear statistic data failed, ret %#x!\n", op_ret);
    		}
    	   
    	}
    	
    	dbus_message_unref(reply);
    	
    	return CMD_SUCCESS;
    }
	else 
	{
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}
ALIAS(cvm_rate_clear_statistic_func,
	  cvm_rate_clear_statistic_cmd,
	  "clear traffic-policer-statistic slot SLOT_ID",
	  "Clear system function\n"
	  "Clear traffic-policer-statistic, drop counter and pass counter\n"
	  "the slot id \n"
	  "the slot id num \n"
);

int dcli_show_cvm_ratelimit_statistic(struct vty * vty, unsigned int flag,unsigned int slot_id)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	DBusMessageIter		iter;
	DBusMessageIter	 iter_array;
	int op_ret = 0;
	STATISTIC_TYPE drop_count = 0;
	STATISTIC_TYPE pass_count = 0;
	
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
	{
	query = dbus_message_new_method_call(CVM_RATE_LIMIT_DBUS_BUSNAME,
										 CVM_RATE_LIMIT_DBUS_OBJPATH,
										 CVM_RATE_LIMIT_DBUS_INTERFACE,
										 CVM_RATE_LIMIT_DBUS_METHOD_SHOW_STATISTIC);
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&flag, 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) 
	{
		printf("Traffic-policer show statistic failed get reply.\n");
		
		if (dbus_error_is_set(&err)) {
			
			printf("%s raised: %s",err.name,err.message);
			
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_WARNING;
	}
	if (dbus_message_get_args(reply, &err,
							 DBUS_TYPE_UINT32, &op_ret,
							 DBUS_STATISTIC_TYPE, &drop_count,
							 DBUS_STATISTIC_TYPE, &pass_count,
							 DBUS_TYPE_INVALID))
	{
		if(CVM_RATELIMIT_RETURN_CODE_SUCCESS == op_ret)
		{
			vty_out(vty, "Traffic-policer Statistic for %s:\n", (1 == flag) ? "phase2" : "phase1");
			vty_out(vty, "==========================================\n");
			vty_out(vty, " drop counter: %llu\n", drop_count);
			vty_out(vty, " pass counter: %llu\n", pass_count);
			vty_out(vty, "==========================================\n");
		}
		else
		{
			vty_out(vty, "%% Get statistic data failed, ret %#x!\n", op_ret);
		}
	   
	}
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
	}
	else 
	{
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
	}
}

DEFUN(
	  cvm_rate_show_statistic_4fastfwd_func,
	  cvm_rate_show_statistic_4fastfwd_cmd,	  
	  "show traffic-policer-statistic phase1 slot SLOT_ID ",
	  "Show system function\n"
	  "Show traffic-policer-statistic, drop counter and pass counter\n"
	  "Show phase1 traffic-policer-statistic, drop counter and pass counter\n"
	  "the slot id \n"
	  "the slot id num \n"
)
{	
	unsigned int flag = 2;
	unsigned int slot_id = 0;
	char *endptr = NULL;
    int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);

    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}

	if(1 != argc)
	{
		vty_out(vty, "%% Bad parameter number!\n");
		
		return CMD_WARNING;
	}
	
	if(dcli_show_cvm_ratelimit_statistic(vty, flag,slot_id) == CMD_WARNING)
	{
        vty_out(vty, "fail show traffic policer statistic\n");
		return CMD_WARNING;
	}
	else
	{
        //vty_out(vty, "success show traffic policer statistic\n");
        return CMD_SUCCESS;
	}
}
DEFUN(
	  cvm_rate_show_statistic_func,
	  cvm_rate_show_statistic_cmd,
	  "show traffic-policer-statistic slot SLOT_ID",
	  "Show system function\n"
	  "Show traffic-policer-statistic, drop counter and pass counter\n"
	  "the slot id \n"
	  "the slot id num \n"
)
{	
	unsigned int flag = 1;
	unsigned int slot_id = 0;
	char *endptr = NULL;
    int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);

    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}

	if(1 != argc)
	{
		vty_out(vty, "%% Bad parameter number!\n");
		
		return CMD_WARNING;
	}
	
	if(dcli_show_cvm_ratelimit_statistic(vty, flag,slot_id) == CMD_WARNING)
	{
        vty_out(vty, "fail show traffic policer statistic\n");
		return CMD_WARNING;
	}
	else
	{
        //vty_out(vty, "success show traffic policer statistic\n");
        return CMD_SUCCESS;
	}
}

DEFUN(cvm_rate_dmesg_enable_set_func,
	  cvm_rate_dmesg_enable_set_cmd,
	  "config traffic-policer-dmesg slot SLOT_ID (enable|disable|TIMES)",
	  CONFIG_STR
	  "Config traffic-policer-dmesg switch\n"
	  "the slot id \n"
	  "the slot id num \n"
	  "Config traffic-policer-dmesg enable\n"
	  "Config traffic-policer-dmesg disable\n"
	  "Config traffic-policer-dmesg print times before auto disable\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	int enable = 0;
	int op_ret = 0;
	unsigned int slot_id = 0;
	char *endptr = NULL;
    int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);

    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}

	if(argc != 2){return CMD_WARNING;}
	
	if(!strncmp(argv[1], "enable", strlen(argv[1])))
	{
		enable = 1;
	}
	else if(!strncmp(argv[1], "disable", strlen(argv[1])))
	{
		enable = 0;
	}
	else 
	{
		PARAMETER_DECIMAL_NUM_CHECK(argv[1]);
		
		if((enable = strtoul(argv[1], NULL, 10)) > 0)
		{
			enable = -enable;
		}
		else{
			
			vty_out(vty, "%% Bad parameter input: %s !\n",argv[1]);
			
			return CMD_WARNING;
		}
	}
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
	{
	query = dbus_message_new_method_call(CVM_RATE_LIMIT_DBUS_BUSNAME,
										 CVM_RATE_LIMIT_DBUS_OBJPATH,
										 CVM_RATE_LIMIT_DBUS_INTERFACE,
										 CVM_RATE_LIMIT_DBUS_METHOD_DMESG_ENABLE_SET);
	
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&enable, 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		
		printf("Traffic policer log level set failed get reply!\n");
		
		if (dbus_error_is_set(&err)) {
			
			printf("%s raised: %s",err.name,err.message);
			
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_WARNING;
	}
	else if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&op_ret,
				DBUS_TYPE_INVALID))
	{	
		if(CVM_RATELIMIT_RETURN_CODE_SUCCESS != op_ret) 
		{
			
			vty_out(vty, "%% Traffic policer log level set failed, ret %#x!\n", op_ret);
			
			dbus_message_unref(reply);
			
			return CMD_WARNING;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			
			dbus_error_free_for_dcli(&err);
		}
	}
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
	}
	else 
	{
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}


DEFUN(cvm_rate_show_rules_func,
	  cvm_rate_show_rules_cmd,
	  "show traffic-policer-rules slot SLOT_ID (phase1|phase2) [NAME]",
	  "Show system function\n"
	  "Show traffic-policer-rules\n"
	  "the slot id \n"
	  "the slot id num \n"
	  "Show phase1 traffic-policer-rules\n"
	  "Show phase2 traffic-policer-rules\n"
	  "Show traffic-policer-rules by name\n"
)
{
#define TRUE 1
#define FALSE 0

	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	DBusMessageIter		iter;
	DBusMessageIter	 iter_array;
	unsigned int enable = 0;
	unsigned int op_ret = 0;
	unsigned char name[PROTOCOL_NAME_LEN] = {0};
	unsigned char *namePtr = name;
	MATCH_TYPE rules[PACKET_MATCH_BYTE_NUM] = {0};
	MATCH_TYPE mask[PACKET_MATCH_BYTE_NUM] = {0};
	unsigned int ratelimiter = 0;
	unsigned int rule_length = 0;
	STATISTIC_TYPE drop_counter = 0;
	STATISTIC_TYPE pass_counter = 0;
	int i = 0;
	int j = 0;
	int flag = 0;
	int first = TRUE;	
	unsigned int slot_id = 0;
	char *endptr = NULL;
    int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);

    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}

	if(argc == 2)
	{
		strcpy(name, "");
	}
	else if(argc == 3)
	{
		strcpy(name, argv[2]);
	}
	else{
		
		vty_out(vty, "%% Bad parameter number!\n");
		
		return CMD_WARNING;
	}
	if(!strncmp(argv[1], "phase1", strlen(argv[1])))
	{/* for fast-fwd */
		flag = 1;
	}
	else if(!strncmp(argv[1], "phase2", strlen(argv[1])))
	{
		flag = 0;
	}
	else
	{
		vty_out(vty, "%% Bad parameter input: %s !\n",argv[1]);
		
		return CMD_WARNING;
	}
	if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
	{
	query = dbus_message_new_method_call(CVM_RATE_LIMIT_DBUS_BUSNAME,
										 CVM_RATE_LIMIT_DBUS_OBJPATH,
										 CVM_RATE_LIMIT_DBUS_INTERFACE,
										 CVM_RATE_LIMIT_DBUS_METHOD_SHOW_RULES);
	
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_STRING,&namePtr, 
							 DBUS_TYPE_UINT32,&flag,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		
		printf("Traffic-policer show rules failed get reply.\n");
		
		if (dbus_error_is_set(&err)) {
			
			printf("%s raised: %s",err.name,err.message);
			
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_WARNING;
	}
	else if (strcmp(name, ""))
	{
		dbus_message_iter_init(reply, &iter);
		
		dbus_message_iter_recurse(&iter, &iter_array);
		
	
		DBusMessageIter iter_struct;
		
		dbus_message_iter_recurse(&iter_array, &iter_struct);
		
		dbus_message_iter_get_basic(&iter_struct, &op_ret);
		dbus_message_iter_next(&iter_struct);
		
		dbus_message_iter_get_basic(&iter_struct, &namePtr);
		dbus_message_iter_next(&iter_struct);
		
		dbus_message_iter_get_basic(&iter_struct, &rule_length);
		dbus_message_iter_next(&iter_struct);
		
		for(j = 0; j < PACKET_MATCH_BYTE_NUM; j++)
		{
			dbus_message_iter_get_basic(&iter_struct, &rules[j]);
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct, &mask[j]);
			dbus_message_iter_next(&iter_struct);
			
		}
		
		dbus_message_iter_get_basic(&iter_struct, &ratelimiter);
		dbus_message_iter_next(&iter_struct);
		
		dbus_message_iter_get_basic(&iter_struct, &drop_counter);
		dbus_message_iter_next(&iter_struct);
		
		dbus_message_iter_get_basic(&iter_struct, &pass_counter);
		dbus_message_iter_next(&iter_array);
		
		if(CVM_RATELIMIT_RETURN_CODE_SUCCESS == op_ret)
		{
			vty_out(vty, "Traffic Policer Rules:\n");
			vty_out(vty, "=========================================================\n");
			
			dcli_cvm_rate_show_rules(vty, namePtr, rule_length, rules, mask, ratelimiter, drop_counter, pass_counter);
			
			vty_out(vty, "=========================================================\n");
		}				
		else if(CVM_RATELIMIT_RETURN_CODE_MODULE_NOTRUNNING == op_ret)
		{
			vty_out(vty, "%% Traffic-policer module is not started, start it first!\n");
		}
		
	}
	else
	{		
		dbus_message_iter_init(reply, &iter);
		
		dbus_message_iter_recurse(&iter, &iter_array);
		
		
		for(i = 0;i < MAX_MATCH_RULES_NUM; i++)
		{
			//if(CVM_RATELIMIT_RETURN_CODE_SUCCESS == op_ret)
			{
				DBusMessageIter iter_struct;
				
				dbus_message_iter_recurse(&iter_array, &iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct, &op_ret);
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct, &namePtr);
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct, &rule_length);
				dbus_message_iter_next(&iter_struct);
				
				for(j = 0; j < PACKET_MATCH_BYTE_NUM; j++)
				{
					dbus_message_iter_get_basic(&iter_struct, &rules[j]);
					dbus_message_iter_next(&iter_struct);
					
					dbus_message_iter_get_basic(&iter_struct, &mask[j]);
					dbus_message_iter_next(&iter_struct);
					
				}				
				
				dbus_message_iter_get_basic(&iter_struct, &ratelimiter);
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct, &drop_counter);
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct, &pass_counter);
				dbus_message_iter_next(&iter_array);
				
				if(!op_ret)
				{
					if(!first)
					{
						vty_out(vty, "_________________________________________________________\n");					
					}
					else
					{
						vty_out(vty, "Traffic Policer Rules:\n");
						vty_out(vty, "=========================================================\n");
						first = FALSE;
					}
					
					vty_out(vty, "%3d:\n",i);
					
					dcli_cvm_rate_show_rules(vty, namePtr, rule_length, rules, mask, ratelimiter, drop_counter, pass_counter);
				}				
				else if(CVM_RATELIMIT_RETURN_CODE_MODULE_NOTRUNNING == op_ret)
				{
					vty_out(vty, "%% Traffic-policer module is not started, start it first!\n");
					break;
				}
				
			}
		}
		if(!first)
		{		
			vty_out(vty, "=========================================================\n");
		}
		
	}
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
	}
	else 
	{
		vty_out(vty, "no connection to slot %d\n", slot_id);
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

int dcli_cvm_ratelimit_show_running(struct vty * vty)
{
	unsigned char *showStr = NULL;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int ret = 1;
	unsigned int slot_id = 0;
	char *endptr = NULL;
    int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);

    for(slot_id = 1;slot_id <= slotNum;slot_id++)
    {
		if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
	    {
        	query = dbus_message_new_method_call(CVM_RATE_LIMIT_DBUS_BUSNAME,
        										 CVM_RATE_LIMIT_DBUS_OBJPATH,
        										 CVM_RATE_LIMIT_DBUS_INTERFACE,
        			   							 CVM_RATE_LIMIT_DBUS_METHOD_SHOW_RUNNING);

        	dbus_message_append_args(query,
        							 DBUS_TYPE_UINT32,&slot_id,
        							 DBUS_TYPE_INVALID);
        	dbus_error_init(&err);

        	reply = dbus_connection_send_with_reply_and_block(dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);

        	dbus_message_unref(query);
        	
        	if (NULL == reply)
        	{
        	   printf("failed get reply.\n");
        	   
        	   if (dbus_error_is_set(&err))
        	   {
        		   printf("%s raised: %s",err.name,err.message);
        		   
        		   dbus_error_free_for_dcli(&err);
        	   }
        	   
        	   continue;
        	}

        	if (dbus_message_get_args(reply, &err,
        							 DBUS_TYPE_STRING, &showStr,
        							 DBUS_TYPE_INVALID))
        	{

        	   char _tmpstr[64];
        	   
        	   memset(_tmpstr,0,64);
        	   
        	   sprintf(_tmpstr,BUILDING_MOUDLE,"TRAFFIC POLICER");
        	   
        	   vtysh_add_show_string(_tmpstr);
        	   
        	   vtysh_add_show_string(showStr);
        	   
        	   ret = 0;
        	}
        	else
        	{
        	   printf("Failed get args.\n");
        	   
        	   if (dbus_error_is_set(&err))
        	   {
        		   printf("%s raised: %s",err.name,err.message);
        		   
        		   dbus_error_free_for_dcli(&err);
        	   }
        	}

        	dbus_message_unref(reply);
        	
        	continue;
		}
		else 
    	{
    		vty_out(vty, "no connection to slot %d\n", slot_id);
    		continue;
    	}
    }
    return CMD_SUCCESS;
}

DEFUN(traffic_policer_service_load_func,
	  traffic_policer_service_load_cmd,
	  "traffic-policer service slot SLOT_ID (load|unload)",
	  "Config traffic-policer\n"
	  "Config traffic-policer service\n"
	  "the slot id \n"
	  "the slot id num \n"
	  "Config traffic-policer service load\n"
	  "Config traffic-policer service unload\n"
)
{
#define CMD_LINE_LEN 256
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	unsigned int op_ret = 0;
	unsigned char confirm[CMD_LINE_LEN] = {0};
	unsigned char cmd[CMD_LINE_LEN] = {0};
	unsigned int load = 0;
	unsigned int mloaded = 0;
	int ret;
	int i = 0;
	unsigned int slot_id = 0;
	char *endptr = NULL;
    int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);

    slot_id = strtoul(argv[0], &endptr, 10);
	if(slot_id > slotNum || slot_id <= 0)
	{
		vty_out(vty,"%% NO SUCH SLOT %d!\n", slot_id);
        return CMD_WARNING;
	}

	if(argc != 2)
	{
		vty_out(vty, "%% Bad parameter number!\n");
		return CMD_WARNING;
	}
	
	if(!strncmp(argv[1], "unload", strlen(argv[1])))
	{
		#if 1
	    if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
	    {
    		query = dbus_message_new_method_call(CVM_RATE_LIMIT_DBUS_BUSNAME,
    											 CVM_RATE_LIMIT_DBUS_OBJPATH,
    											 CVM_RATE_LIMIT_DBUS_INTERFACE,
    											 CVM_RATE_LIMIT_DBUS_METHOD_SERVICE_UNLOAD);
    		
    		dbus_error_init(&err);
    			
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection, query, -1, &err);
    		
    		dbus_message_unref(query);
    		if (NULL == reply) {
    			
    			printf("SLOT %d:Traffic-policer service %s failed get reply.\n",slot_id,load ? "load" : "unload");
    			
    			if (dbus_error_is_set(&err)) {
    				
    				printf("SLOT %d:%s raised: %s",slot_id,err.name,err.message);
    				
    				dbus_error_free_for_dcli(&err);
    			}
    			
    			return CMD_WARNING;
    		}
    		else if (dbus_message_get_args ( reply, &err,
    					DBUS_TYPE_INT32,&ret,
    					DBUS_TYPE_INVALID))
    		{	
    			if(CVM_RATELIMIT_RETURN_CODE_SUCCESS == ret) 
    			{			
    				vty_out(vty, "%% Traffic policer service unloaded success! ret= %x\n",ret);
    			}
				else if(CVM_RATELIMIT_RETURN_CODE_FAILED == ret) 
				{
                    vty_out(vty, "%% Traffic policer service unloaded fail! ret= %x\n",ret);
				}
				else if(CVM_RATELIMIT_RETURN_CODE_MODULE_NOTRUNNING == ret) 
				{
                    vty_out(vty, "%% Traffic policer service not running ! ret= %x\n",ret);
				}
				else
				{
                    vty_out(vty, "%% Traffic policer service unloaded bad return! ret= %x\n",ret);
				}
				dbus_message_unref(reply);
    		    return CMD_WARNING;
    		} 
    		else 
    		{		
    			if (dbus_error_is_set(&err)) 
    			{	
    				printf("SLOT %d:%s raised: %s",slot_id,err.name,err.message);
    				
    				dbus_error_free_for_dcli(&err);
    			}
    		}
    		
    		dbus_message_unref(reply);
    	}
    	else 
    	{
    		vty_out(vty, "no connection to slot %d\n", slot_id);
    		return CMD_WARNING;
    	}
        #endif
		
		#if 0
		memset(cmd, 0, CMD_LINE_LEN);
		sprintf(cmd, "sudo lsmod|grep cavium_ratelimit > /dev/null\n");
		op_ret = system(cmd);
		if(op_ret)
		{
			vty_out(vty, "%% Traffic-policer module is not started!\n");
		}
		else
		{
			printf( "Are you sure you want to unload traffic-policer service,\n"\
					" and erase all traffic-policer rules (yes/no)?");
			
			memset(confirm, 0, CMD_LINE_LEN);
			
			fgets(confirm, CMD_LINE_LEN-1, stdin);
			
			fflush(stdin);
			
			if ('\n' == confirm[strlen(confirm)-1])
			{
				confirm[strlen(confirm)-1] = '\0';
			}
			if(strncmp(confirm, "yes", strlen(confirm)))
			{
				vty_out(vty, "%% Command cancelled!\n");
				
				return CMD_SUCCESS;
			}
			printf("\n");
			mloaded = 1;
		}
		memset(cmd, 0, CMD_LINE_LEN);
		sprintf(cmd, "sudo /etc/init.d/cvm_rate stop > /dev/null\n");
		op_ret = system(cmd);
		if(op_ret)
		{
			vty_out(vty, "%% Stop service failed when service unload!\n");
			return CMD_WARNING;
		}
		if(mloaded)
		{
			for(i = 0; i < 10; i++)
			{
				sleep(1);
				memset(cmd, 0, CMD_LINE_LEN);
				sprintf(cmd, "sudo rmmod cavium_ratelimit 2> /dev/null\n");
				op_ret = system(cmd);
				if(!op_ret)
				{
					break;
					//return CMD_WARNING;
				}
			}
			if((10 == i) && (op_ret))
			{
				vty_out(vty, "%% Service unload failed, ret: %d!\n", WEXITSTATUS(op_ret));
			}
		}
		memset(cmd, 0, CMD_LINE_LEN);
		sprintf(cmd, "/etc/init.d/cvm_rate start > /dev/null\n");
		op_ret = system(cmd);
		if(op_ret)
		{
			vty_out(vty, "%% Restart service failed when service unload!\n");
			return CMD_WARNING;
		}
		#endif
	}
	else if(!strncmp(argv[1], "load", strlen(argv[1])))
	{
		load = 1;
	}
		
	if(1 == load)
	{
	    if (dbus_connection_dcli[slot_id]->dcli_dbus_connection)
	    {
    		query = dbus_message_new_method_call(CVM_RATE_LIMIT_DBUS_BUSNAME,
    											 CVM_RATE_LIMIT_DBUS_OBJPATH,
    											 CVM_RATE_LIMIT_DBUS_INTERFACE,
    											 CVM_RATE_LIMIT_DBUS_METHOD_SERVICE_LOAD);
    		
    		dbus_error_init(&err);
    			
    		reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection, query, -1, &err);
    		
    		dbus_message_unref(query);
    		
    		if (NULL == reply) {
    			
    			printf("Traffic-policer service %s failed get reply.\n", load ? "load" : "unload");
    			
    			if (dbus_error_is_set(&err)) {
    				
    				printf("%s raised: %s",err.name,err.message);
    				
    				dbus_error_free_for_dcli(&err);
    			}
    			
    			return CMD_WARNING;
    		}
    		else if (dbus_message_get_args ( reply, &err,
    					DBUS_TYPE_UINT32,&op_ret,
    					DBUS_TYPE_INVALID))
    		{	
    			if(CVM_RATELIMIT_RETURN_CODE_SUCCESS != op_ret) 
    			{			

    				if(CVM_RATELIMIT_RETURN_CODE_SERVICE_ALREADY_LOAD == op_ret)
    				{
    					vty_out(vty, "%% Traffic policer service already loaded!\n");;
    				}
    				else
    				{
    					vty_out(vty, "%% Traffic policer service %s failed, ret %#x!\n", load ? "load" : "unload", op_ret);
    				}
    				
    				dbus_message_unref(reply);
    				
    				return CMD_WARNING;
    			}
    		} 
    		else 
    		{		
    			if (dbus_error_is_set(&err)) 
    			{	
    				printf("slot %d:%s raised: %s",slot_id,err.name,err.message);
    				
    				dbus_error_free_for_dcli(&err);
    			}
    		}
    		
    		dbus_message_unref(reply);
    	}
    	else 
    	{
    		vty_out(vty, "no connection to slot %d\n", slot_id);
    		return CMD_WARNING;
    	}
    	return CMD_SUCCESS;
	}
}

void dcli_cvm_ratelimit_element_init(void)  
{	
	
    install_node(&cvm_rate_limit_node, dcli_cvm_ratelimit_show_running, "TRAFFIC_POLICER_NODE");

	/* modified by zhengbo for hide ethernet rate trigger options */
    install_element(CONFIG_NODE,&traffic_policer_service_load_cmd);
    install_element(CONFIG_NODE,&cvm_rate_enable_set_4fastfwd_cmd);
	install_element(HIDDENDEBUG_NODE,&cvm_rate_enable_set_4ethernet_cmd);

    install_element(CONFIG_NODE,&cvm_rate_dmesg_enable_set_cmd);
	
    install_element(CONFIG_NODE,&cvm_rate_log_level_set_cmd);
	
	
    install_element(CONFIG_NODE,&cvm_rate_rules_add_simp_cmd);
	install_element(CONFIG_NODE,&cvm_rate_rules_add_cmd);
	
    install_element(CONFIG_NODE,&cvm_rate_rules_del_cmd);
	
    install_element(CONFIG_NODE,&cvm_rate_rules_modify_cmd);
    install_element(CONFIG_NODE,&cvm_rate_rules_modify_limiter_cmd);
	
    install_element(CONFIG_NODE,&cvm_rate_rules_clear_cmd);
	
    install_element(CONFIG_NODE,&cvm_rate_clear_statistic_cmd);
    install_element(CONFIG_NODE,&cvm_rate_clear_statistic_4fastfwd_cmd);	
	
    install_element(CONFIG_NODE,&cvm_rate_load_rules_cmd);
    install_element(CONFIG_NODE,&cvm_rate_config_rules_source_cmd);	
    install_element(CONFIG_NODE,&cvm_rate_restore_rules_cmd);	
	
    install_element(CONFIG_NODE,&cvm_rate_show_rules_cmd);
	
    install_element(CONFIG_NODE,&cvm_rate_show_statistic_cmd);
    install_element(CONFIG_NODE,&cvm_rate_show_statistic_4fastfwd_cmd);	
		
}

#ifdef __cplusplus
}
#endif


