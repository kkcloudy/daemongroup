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
* dcli_qos.c
*
*
* CREATOR:
*		sulong@autelan.com
*
* DESCRIPTION:
*		CLI definition for QOS module.
*
* DATE:
*		05/30/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.61 $	
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
#include "vty.h"
#include "dcli_acl.h"
#include "dcli_qos.h"
#include "command.h"
#include "if.h"
#include "sysdef/returncode.h"


/* for distributed qos */
#include "dcli_main.h"  /* it must be here */
#include "dcli_sem.h"
#include "board/board_define.h"

extern DBusConnection *dcli_dbus_connection;
extern int dcli_str2ulong(char *str,unsigned int *value);

extern int is_distributed;
static DBusConnection *dcli_dbus_connection_qos;
static DBusConnection *dcli_dbus_connection_qos_port;

/*wangchong for distributed qos */
struct cmd_node qos_node_distributed = 
{
	QOS_NODE_DISTRIBUTED,
	"%s(config-qos-sw-board)# ",
};

struct cmd_node qos_node = 
{
	QOS_NODE,
	" ",
	1
};
struct cmd_node qos_profile_node = 
{
	QOS_PROFILE_NODE,
	"%s(config-qos-profile)# "
};
struct cmd_node policy_map_node = 
{
	POLICY_NODE,
	"%s(config-policy-map)# "
};
struct cmd_node policer_node = 
{
	POLICER_NODE,
	"%s(config-policer)# "
};
struct cmd_node out_profile_node = 
{
	OUT_PROFILE_NODE,
	"%s(config-policer-outprofile)# "
};

struct cmd_node sch_queue_node = 
{
	SHQUEUE_NODE,
	"%s(config-queue-scheduler)# "
};

#define INDEX_LENTH_CHECK(str,num) \
if(num<(strlen(str))){	\
	vty_out(vty,"%% illegal number value!\n"); \
    return CMD_WARNING; \
	}

int dcli_qos_profile_show_running_config()
{	 
	 char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	 int ret = 1;
	 DBusMessage *query, *reply;
	 DBusError err;

	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
    int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);	
	int function_type = -1;
	int i = 0;

	char file_path[64] = {0};

    if(is_distributed == DISTRIBUTED_SYSTEM)
    {
		for(i = 1; i <= slotNum; i++)
		{
			sprintf(file_path,"/dbm/product/slot/slot%d/function_type", i);
			function_type = get_product_info(file_path);
			
			if (function_type == SWITCH_BOARD)
			{
            	 query = dbus_message_new_method_call(  NPD_DBUS_BUSNAME,\
            										 	NPD_DBUS_QOS_OBJPATH, \
            										 	NPD_DBUS_QOS_INTERFACE, \
            										 	NPD_DBUS_METHOD_SHOW_QOS_PROFILE_RUNNIG_CONFIG);
     
            	 dbus_error_init(&err);
             
                if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
                {
                	if(i == local_slot_id) 
                	{
                	    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
                	}
                	else 
                	{	
                		printf("Can not connect to slot %d .\n", i);
                		continue;
                	}
                }
				else 
                {
                	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection, query, -1, &err);
                }
     
            	 dbus_message_unref(query);
            	 if (NULL == reply) {
            		 printf("show qos_profile running config failed get reply.\n");
            		 if (dbus_error_is_set(&err)) {
            			 dbus_error_free_for_dcli(&err);
            		 }
            		 return 1;
            	 }
             
            	 if (dbus_message_get_args ( reply, &err,
            					 DBUS_TYPE_STRING, &showStr,
            					 DBUS_TYPE_INVALID)) 
            	 {
	 
            		char _tmpstr[64];
            		memset(_tmpstr,0,64);
            		sprintf(_tmpstr,BUILDING_MOUDLE,"QOS PROFILE");
            		vtysh_add_show_string(_tmpstr);
            		vtysh_add_show_string(showStr);
            		ret = 0;
            	 } 
            	 else 
            	 {
            		 if (dbus_error_is_set(&err)) 
            		 {
            			 dbus_error_free_for_dcli(&err);
            		 }
            	 }
     
            	 dbus_message_unref(reply);
			}
		}
    }
	 return ret;
}

int dcli_qos_counter_show_running_config()
{	 
	 char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	 int ret = 0;	 
	 DBusMessage *query, *reply;
	 DBusError err;

	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
    int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);	
	int function_type = -1;
	int i = 0;

	char file_path[64] = {0};

    if(is_distributed == DISTRIBUTED_SYSTEM)
    {
		for(i = 1; i <= slotNum; i++)
		{
			sprintf(file_path,"/dbm/product/slot/slot%d/function_type", i);
			function_type = get_product_info(file_path);
			
			if (function_type == SWITCH_BOARD)
			{
            	 query = dbus_message_new_method_call(  NPD_DBUS_BUSNAME,\
            										 	NPD_DBUS_QOS_OBJPATH, \
            										 	NPD_DBUS_QOS_INTERFACE, \
            										 	NPD_DBUS_METHOD_SHOW_QOS_COUNTER_RUNNIG_CONFIG);
             
            	 dbus_error_init(&err);
             
                if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
                {
                	if(i == local_slot_id) 
                	{
                	    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
                	}
                	else 
                	{	
                		printf("Can not connect to slot %d .\n", i);
                		continue;
                	}
                }
				else 
                {
                	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection, query, -1, &err);
                }
             
            	 dbus_message_unref(query);
            	 if (NULL == reply) {
            		 printf("show qos_counter running config failed get reply.\n");
            		 if (dbus_error_is_set(&err)) {
            			 dbus_error_free_for_dcli(&err);
            		 }
            		 return 0;
            	 }
             
            	 if (dbus_message_get_args ( reply, &err,
            					 DBUS_TYPE_STRING, &showStr,
            					 DBUS_TYPE_INVALID)) 
            	 {
            	 
            		char _tmpstr[64];
            		memset(_tmpstr,0,64);
            		sprintf(_tmpstr,BUILDING_MOUDLE,"QOS COUNTER");
            		vtysh_add_show_string(_tmpstr);
            		vtysh_add_show_string(showStr);
            		ret = 1;
            	 } 
            	 else 
            	 {
            		 if (dbus_error_is_set(&err)) 
            		 {
            			 dbus_error_free_for_dcli(&err);
            		 }
            	 }
             
            	 dbus_message_unref(reply);
			}
		}
    }
    return ret;
 }
int dcli_qos_policy_map_show_running_config()
{	 
	 char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	 int ret = 0;
	 DBusMessage *query, *reply;
	 DBusError err;

	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
    int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);	
	int function_type = -1;
	int i = 0;

	char file_path[64] = {0};

    if(is_distributed == DISTRIBUTED_SYSTEM)
    {
		for(i = 1; i <= slotNum; i++)
		{
			sprintf(file_path,"/dbm/product/slot/slot%d/function_type", i);
			function_type = get_product_info(file_path);
			
			if (function_type == SWITCH_BOARD)
            {
            	 query = dbus_message_new_method_call(  NPD_DBUS_BUSNAME,\
            										 	NPD_DBUS_QOS_OBJPATH, \
            										 	NPD_DBUS_QOS_INTERFACE, \
            										 	NPD_DBUS_METHOD_SHOW_POLICY_MAP_RUNNIG_CONFIG);
             
            	 dbus_error_init(&err);
             
                if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
                {
                	if(i == local_slot_id) 
                	{
                	    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
                	}
                	else 
                	{	
                		printf("Can not connect to slot %d .\n", i);
                		continue;
                	}
                }
				else 
                {
                	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection, query, -1, &err);
                }
             
            	 dbus_message_unref(query);
            	 if (NULL == reply) {
            		 printf("show qos_policy_map running config failed get reply.\n");
            		 if (dbus_error_is_set(&err)) {
            			 dbus_error_free_for_dcli(&err);
            		 }
            		 return 0;
            	 }
             
            	 if (dbus_message_get_args ( reply, &err,
            					 DBUS_TYPE_STRING, &showStr,
            					 DBUS_TYPE_INVALID)) 
            	 {
            	 
            			char _tmpstr[64];
            			memset(_tmpstr,0,64);
            			sprintf(_tmpstr,BUILDING_MOUDLE,"QOS POLICY");
            			vtysh_add_show_string(_tmpstr);
            			vtysh_add_show_string(showStr);
            			ret = 1;
            	 } 
            	 else 
            	 {
            		 if (dbus_error_is_set(&err)) 
            		 {
            			 dbus_error_free_for_dcli(&err);
            		 }
            	 }
             
            	 dbus_message_unref(reply);
			}
		}
    }
    return ret; 
}
int dcli_qos_remap_table_show_running_config()
{	 
	 char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	 int ret = 0;	 
     DBusMessage *query, *reply;
	 DBusError err;

    int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
    int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);	
	int function_type = -1;
	int i = 0;

	char file_path[64] = {0};

    if(is_distributed == DISTRIBUTED_SYSTEM)
    {
		for(i = 1; i <= slotNum; i++)
		{
			sprintf(file_path,"/dbm/product/slot/slot%d/function_type", i);
			function_type = get_product_info(file_path);
			
			if (function_type == SWITCH_BOARD)
            {
            	 query = dbus_message_new_method_call(  NPD_DBUS_BUSNAME,\
            										 	NPD_DBUS_QOS_OBJPATH, \
            										 	NPD_DBUS_QOS_INTERFACE, \
            										 	NPD_DBUS_METHOD_SHOW_REMAP_TABLE_RUNNIG_CONFIG);
             
            	 dbus_error_init(&err);
             
                if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
                {
                	if(i == local_slot_id) 
                	{
                	    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
                	}
                	else 
                	{	
                		printf("Can not connect to slot %d .\n", i);
                		continue;
                	}
                }
				else 
                {
                	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection, query, -1, &err);
                }
             
            	 dbus_message_unref(query);
            	 if (NULL == reply) {
            		 printf("show qos_remap_table running config failed get reply.\n");
            		 if (dbus_error_is_set(&err)) {
            			 dbus_error_free_for_dcli(&err);
            		 }
            		 return 0;
            	 }
             
            	 if (dbus_message_get_args ( reply, &err,
            					 DBUS_TYPE_STRING, &showStr,
            					 DBUS_TYPE_INVALID)) 
            	 {
            	 
            		char _tmpstr[64];
            		memset(_tmpstr,0,64);
            		sprintf(_tmpstr,BUILDING_MOUDLE,"QOS REMAP");
            		vtysh_add_show_string(_tmpstr);
            		vtysh_add_show_string(showStr);
            		ret = 1;
            	 } 
            	 else 
            	 {
            		 if (dbus_error_is_set(&err)) 
            		 {
            			 dbus_error_free_for_dcli(&err);
            		 }
            	 }
             
            	 dbus_message_unref(reply);
			}
		}
    }
    return ret; 
}
int dcli_qos_policer_show_running_config()
{	 
	 char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	 int ret = 1;
	 DBusMessage		*query, *reply;
	 DBusError err;

	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
    int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);	
	int function_type = -1;
	int i = 0;

	char file_path[64] = {0};

    if(is_distributed == DISTRIBUTED_SYSTEM)
    {
		for(i = 1; i <= slotNum; i++)
		{
			sprintf(file_path,"/dbm/product/slot/slot%d/function_type", i);
			function_type = get_product_info(file_path);
			
			if (function_type == SWITCH_BOARD)
            {	 
            	 query = dbus_message_new_method_call(  NPD_DBUS_BUSNAME,\
            										 	NPD_DBUS_QOS_OBJPATH, \
            										 	NPD_DBUS_QOS_INTERFACE, \
            										 	NPD_DBUS_METHOD_SHOW_POLICER_RUNNIG_CONFIG);
             
            	 dbus_error_init(&err);
             
                if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
                {
                	if(i == local_slot_id) 
                	{
                	    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
                	}
                	else 
                	{	
                		printf("Can not connect to slot %d .\n", i);
                		continue;
                	}
                }
				else 
                {
                	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection, query, -1, &err);
                }
             
            	 dbus_message_unref(query);
            	 if (NULL == reply) {
            		 printf("show qos_policer running config failed get reply.\n");
            		 if (dbus_error_is_set(&err)) {
            			 dbus_error_free_for_dcli(&err);
            		 }
            		 return 1;
            	 }
             
            	 if (dbus_message_get_args ( reply, &err,
            					 DBUS_TYPE_STRING, &showStr,
            					 DBUS_TYPE_INVALID)) 
            	 {
            	 
            			char _tmpstr[64];
            			memset(_tmpstr,0,64);
            			sprintf(_tmpstr,BUILDING_MOUDLE,"QOS POLICER");
            			vtysh_add_show_string(_tmpstr);
            			vtysh_add_show_string(showStr);
            			ret = 0;
            	 } 
            	 else 
            	 {
            		 if (dbus_error_is_set(&err)) 
            		 {
            			 dbus_error_free_for_dcli(&err);
            		 }
            	 }
             
            	 dbus_message_unref(reply);
			}
		}
    }
    return ret; 
}
int dcli_qos_queue_scheduler_show_running_config()
{	 
	 char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	 int ret = 0;	 
	 DBusMessage *query, *reply;
	 DBusError err;
	 
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
    int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);	
	int function_type = -1;
	int i = 0;

	char file_path[64] = {0};

    if(is_distributed == DISTRIBUTED_SYSTEM)
    {
		for(i = 1; i <= slotNum; i++)
		{
			sprintf(file_path,"/dbm/product/slot/slot%d/function_type", i);
			function_type = get_product_info(file_path);
			
			if (function_type == SWITCH_BOARD)
			{	 
				query = dbus_message_new_method_call(  NPD_DBUS_BUSNAME,\
										 	NPD_DBUS_QOS_OBJPATH, \
										 	NPD_DBUS_QOS_INTERFACE, \
										 	NPD_DBUS_METHOD_SHOW_QUEUE_SCH_RUNNIG_CONFIG);
 
            	 dbus_error_init(&err);
             
                if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
                {
                	if(i == local_slot_id) 
                	{
                	    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
                	}
                	else 
                	{	
                		printf("Can not connect to slot %d .\n", i);
                		continue;
                	}
                }
				else 
                {
                	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection, query, -1, &err);
                }
             
            	 dbus_message_unref(query);
            	 if (NULL == reply) {
            		 printf("show qos_queue_scheduler running config failed get reply.\n");
            		 if (dbus_error_is_set(&err)) {
            			 dbus_error_free_for_dcli(&err);
            		 }
            		 return 0;
            	 }
             
            	 if (dbus_message_get_args ( reply, &err,
            					 DBUS_TYPE_STRING, &showStr,
            					 DBUS_TYPE_INVALID)) 
            	 {
            	 
            			char _tmpstr[64];
            			memset(_tmpstr,0,64);
            			sprintf(_tmpstr,BUILDING_MOUDLE,"QOS QUEUE");
            			vtysh_add_show_string(_tmpstr);
            			vtysh_add_show_string(showStr);
            			ret = 1;
            	 } 
            	 else 
            	 {
            		 if (dbus_error_is_set(&err)) 
            		 {
            			 dbus_error_free_for_dcli(&err);
            		 }
            	 }
             
            	 dbus_message_unref(reply);
			}
		}
    }
    return ret; 
 }

int dcli_qos_mode_show_running_config()
{	 
	 char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	 int ret = 0;	 
	 DBusMessage *query, *reply;
	 DBusError err;

	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
    int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);	
	int function_type = -1;
	int i = 0;

	char file_path[64] = {0};

    if(is_distributed == DISTRIBUTED_SYSTEM)
    {
		for(i = 1; i <= slotNum; i++)
		{
			sprintf(file_path,"/dbm/product/slot/slot%d/function_type", i);
			function_type = get_product_info(file_path);
			
			if (function_type == SWITCH_BOARD)
			{	 
            	 query = dbus_message_new_method_call(  NPD_DBUS_BUSNAME,\
            										 	NPD_DBUS_QOS_OBJPATH, \
            										 	NPD_DBUS_QOS_INTERFACE, \
            										 	NPD_DBUS_METHOD_SHOW_QOS_MODE_RUNNIG_CONFIG);
             
            	 dbus_error_init(&err);
             
                if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
                {
                	if(i == local_slot_id) 
                	{
                	    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
                	}
                	else 
                	{	
                		printf("Can not connect to slot %d .\n", i);
                		continue;
                	}
                }
				else 
                {
                	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection, query, -1, &err);
                }
             
            	 dbus_message_unref(query);
            	 if (NULL == reply) {
            		 printf("show qos_mode running config failed get reply.\n");
            		 if (dbus_error_is_set(&err)) {
            			 dbus_error_free_for_dcli(&err);
            		 }
            		 return 0;
            	 }
             
            	 if (dbus_message_get_args ( reply, &err,
            					 DBUS_TYPE_STRING, &showStr,
            					 DBUS_TYPE_INVALID)) 
            	 {
            			char _tmpstr[64];
            			memset(_tmpstr,0,64);
            			sprintf(_tmpstr,BUILDING_MOUDLE,"QOS");
            			vtysh_add_show_string(_tmpstr);
            			vtysh_add_show_string(showStr);
            			ret = 1;
            	 } 
            	 else 
            	 {
            		 if (dbus_error_is_set(&err)) 
            		 {
            			 dbus_error_free_for_dcli(&err);
            		 }
            	 }
             
            	 dbus_message_unref(reply);
			}
		}
    }
	 return ret; 
 }

/*wangchong for distributed qos */
DEFUN(config_qos_on_board_cmd_func,
	  config_qos_on_board_cmd,
	  "config qos switch-board <1-10>",
	  CONFIG_STR
	  "Configure qos of Switch-board\n"
	  "Configure swtich-board on slot N\n"
	  "Slot id of swtich-board \n"
)
{
	unsigned int dist_slot = 0; 
	int ret = 0;
   	unsigned int nodeSave = 0;
   	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
	int function_type = -1;
	char file_path[64] = {0};

	
	ret = dcli_str2ulong((char*)argv[0],&dist_slot);
	if (QOS_RETURN_CODE_ERROR == ret) {
		vty_out(vty,"%% Illegal slot number!\n");
		return CMD_WARNING;
	}

	if((dist_slot < 1)||(dist_slot > 10))
	{
		vty_out(vty, "%% Slot number out range!\n");
		return CMD_WARNING;
	}

	if(is_distributed == DISTRIBUTED_SYSTEM)
    {
        
		/* check if the right board */
    	sprintf(file_path,"/dbm/product/slot/slot%d/function_type", dist_slot);
    	function_type = get_product_info(file_path);
    	
    	if (function_type != SWITCH_BOARD)
    	{
    		vty_out(vty, "Slot %d is not Switch-board, Please select another !\n", dist_slot);	
    		return CMD_WARNING;
    	}		

		/* send CMD */
    	if(NULL == dbus_connection_dcli[dist_slot]->dcli_dbus_connection) 				
    	{
			vty_out(vty, "Can not connect to slot %d.\n", dist_slot);
			return CMD_WARNING;
    	}
		else 
    	{
			vty_out(vty, "<<========== Config qos switch-board slot: %d =========>>\n",dist_slot);
            dcli_dbus_connection_qos= dbus_connection_dcli[dist_slot]->dcli_dbus_connection;			

			/*vty_out(vty,"Enter inner CMD node...\n");*/
			if(CONFIG_NODE == vty->node)
			{
				vty->node = QOS_NODE_DISTRIBUTED;
				nodeSave = dist_slot;
				vty->index = (void*)nodeSave;/*when not add & before vlanId, the Vty enter <config-line> CMD Node.*/
			}
			else{
				vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
				return CMD_WARNING;
			}
		}
    }
	return CMD_SUCCESS;
}

DEFUN(config_qos_base_acl_traffic_func,
	  config_qos_base_acl_traffic,
	  "append acl INDEX ingress-qos <1-127>",
	  "Append acl configuration\n"
	  ACL_STR
	  "Specify acl index, extended rule range in 1-512\n"
	  "Ingress QoS mark\n"
	  "QoS profile Index range in 1-127\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned int ruleIndex = 0,profileIndex = 0;
	unsigned int op_ret = 0;
	int ret = 0;
	
    ret=dcli_str2ulong((char*)argv[0],&ruleIndex);	
	if(ret==QOS_RETURN_CODE_ERROR)
	{
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_WARNING;
	}
	if(ruleIndex > MAX_EXT_RULE_NUM) {
		vty_out(vty,"%% extended rule must less than 512\n");
		return CMD_WARNING;
	}
	ruleIndex = ruleIndex-1;

	ret= dcli_str2ulong((char*)argv[1],&profileIndex);	
	if(ret==QOS_RETURN_CODE_ERROR)
	{
		vty_out(vty,"%% Illegal qos profile index!\n");
		return CMD_WARNING;
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_APPEND_QOS_MARK_BASE_ACL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
						     DBUS_TYPE_UINT32,&ruleIndex,
							 DBUS_TYPE_UINT32,&profileIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	

			if(ACL_RETURN_CODE_GLOBAL_NOT_EXISTED==op_ret)
			{
				vty_out(vty,"%% ACL rule %d not existed!\n",(ruleIndex+1));
			}
			else if (ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT== op_ret)
			{
				vty_out(vty,"%% device not support append qos-profile for change up \n");	
				vty_out(vty,"so up will not change\n");	
			}
			else if (ACL_RETURN_CODE_ALREADY_PORT==op_ret)
			{
				vty_out(vty,"%% QoS mode is not flow, please change mode \n");	
			}
			else if (op_ret == ACL_RETURN_CODE_NO_QOS_MODE) {
				vty_out(vty, "%% There is no qos mode, please config qos mode \n");
			}	
			else if (ACL_RETURN_CODE_HYBRID_FLOW==op_ret)
			{
				vty_out(vty,"%% QoS mode is hybrid, qos-profile index is 72~127 \n");	
			}
			else if (QOS_RETURN_CODE_PROFILE_NOT_EXISTED==op_ret)
			{
				vty_out(vty,"%% QoS profile %d not existed!\n",profileIndex);
			}
			else if(ACL_RETURN_CODE_RULE_EXT_ONLY == op_ret){
				vty_out(vty,"%% Fail to append qos profile,  please use the extended rule \n");
			}
			else if(op_ret == ACL_RETURN_CODE_GROUP_RULE_EXISTED){
				vty_out(vty,"%% Can't append this acl since it is bound to ingress group \n");
			}	
			else if(op_ret ==ACL_RETURN_CODE_EGRESS_GROUP_RULE_EXISTED){
				vty_out(vty,"%% Can't append this acl since it is bound to egress group \n");
			}
			else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
				vty_out(vty,"%% Fail to append qos mark to acl traffic!\n");	
			}

	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;


}

DEFUN(show_qos_base_acl_traffic_func,
	show_qos_base_acl_traffic,
	"show append",
	SHOW_STR
	"Show append\n"
	"Show append information\n"
)
{
	
	 DBusMessage *query = NULL, *reply = NULL;
	 DBusError err;
	 DBusMessageIter  iter;
	 DBusMessageIter  iter_array;

	 unsigned int aclindex=0,profileindex=0;
	 unsigned int ret,count=0,j=0;
	 unsigned char drop[10]={0};
	 
	 query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_QOS_OBJPATH,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_SHOW_APPEND_QOS_MARK_BASE_ACL);
	 dbus_error_init(&err);
	 reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	 dbus_message_unref(query);
	 if (NULL == reply) 
	 {
		 /*vty_out(vty,"failed get reply.\n");*/
		 if (dbus_error_is_set(&err)) 
		 {
			 dbus_error_free_for_dcli(&err);
		 }
		 return CMD_SUCCESS;
	 }
	 dbus_message_iter_init(reply,&iter);

	 dbus_message_iter_get_basic(&iter,&ret);

	if(QOS_RETURN_CODE_SUCCESS == ret) {
		 dbus_message_iter_next(&iter);  
		 dbus_message_iter_get_basic(&iter,&count);
		 dbus_message_iter_next(&iter); 	 
		 dbus_message_iter_recurse(&iter,&iter_array);		   
		 for (j = 0; j < count; j++) {
			 DBusMessageIter iter_struct;
			 dbus_message_iter_recurse(&iter_array,&iter_struct);			 
			 dbus_message_iter_get_basic(&iter_struct,&aclindex);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&profileindex);
			 dbus_message_iter_next(&iter_array);
		    vty_out(vty,"===============================================\n");
		    vty_out(vty,"%-40s: %d\n","append acl ",aclindex);
			vty_out(vty,"%-40s: %d\n","Qos profile ",profileindex);
			vty_out(vty,"===============================================\n");
		 }
	}
	else if(QOS_RETURN_CODE_PROFILE_NOT_EXISTED== ret) 
		vty_out(vty,"%% Error: No QoS profile exists.\n");
	
	dbus_message_unref(reply);
	 return CMD_SUCCESS;

}


DEFUN(config_qos_mode_cmd_func,
      config_qos_mode_cmd,
      "config qos-mode (port|flow|hybrid|default)",
      CONFIG_STR
      "Config QoS mode\n"
      "Config QoS port mode\n"
      "Config QoS flow mode\n"
      "Config QoS hybrid mode\n"
      "Config QoS default mode\n"
      )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int qosmode=0;
	unsigned int op_ret=0;
	int ret=0;
	if(strncmp("port",argv[0],strlen(argv[0]))==0)
	{
		qosmode = 0;
	}
	else if (strncmp("flow",argv[0],strlen(argv[0]))==0)
	{
		qosmode = 1;
	}
	else if (strncmp("hybrid",argv[0],strlen(argv[0]))==0)
	{
		qosmode = 2;
	}
	else if (strncmp("default",argv[0],strlen(argv[0]))==0)
	{
		qosmode = 3;
	}
	else
	{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}
   	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_CONFIG_QOS_MODE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&qosmode,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
            if(ACL_RETURN_CODE_ALREADY_PORT==op_ret){
                 vty_out(vty,"%% qos mode is already port mode\n");                    
            }
            else if(ACL_RETURN_CODE_ALREADY_FLOW==op_ret){
                 vty_out(vty,"%% qos mode is already flow mode\n");                    
            }
            else if(ACL_RETURN_CODE_ALREADY_HYBRID==op_ret){
                 vty_out(vty,"%% qos mode is already bybrid mode\n");                    
            }
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(show_qos_mode_cmd_func,
	show_qos_mode_cmd,
	"show qos-mode",
	SHOW_STR
	"Show QoS mode\n"
)
{
	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int qosmode = 0;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                     NPD_DBUS_QOS_OBJPATH,
                                     NPD_DBUS_QOS_INTERFACE,
                                     NPD_DBUS_METHOD_SHOW_QOS_MODE);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&qosmode,
		DBUS_TYPE_INVALID)) {						
		if(1 == qosmode) {
			vty_out(vty,"Qos-mode is flow\n");
			}
		else if(2 == qosmode) {
			vty_out(vty,"Qos-mode is hybrid\n");
			}
		else if(0 == qosmode) {
			vty_out(vty,"Qos-mode is port\n");
			}
		else {
			vty_out(vty,"Qos-mode is default\n");
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/*wangchong no change*/
DEFUN(config_qos_profile_cmd_func,
      config_qos_profile_cmd,
      "set qos-profile <1-127>",
      "Set system configuration\n"
      "Set QoS profile\n"
      "Specify QoS profile range in 1-127\n"
      )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int profileIndex = 0;
	unsigned int op_ret = 0;
	int ret = 0;
	
	ret = dcli_str2ulong((char*)argv[0],&profileIndex);	
      if(ret==QOS_RETURN_CODE_ERROR)
	{
		vty_out(vty,"%% Illegal qos profile index!\n");
		return CMD_WARNING;
	}
    	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_CONFIG_QOS_PROFILE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profileIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
            if(QOS_RETURN_CODE_SUCCESS==op_ret){
                  if(QOS_NODE_DISTRIBUTED==vty->node){
                      vty->node = QOS_PROFILE_NODE;
                      vty->index = (void*)profileIndex;                           
                  }                      
            }
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
            else {
                 vty_out(vty,"%% Fail to config qos profile\n");     
             }
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_qos_profile_atrribute_cmd_func,
      config_qos_profile_atrribute_cmd,
      "dp <0-1> up <0-7> tc <0-7> dscp <0-63>",
      "QoS dropPrecedence\n"
      "Drop Precedence 0 green ,1 red\n"
      "QoS userPriority\n"
      "User Priority range in 0-7\n"
      "QoS trafficClass\n"
      "Traffic Class range in 0-7\n"
      "QoS dscp\n"
      "DSCP range in 0-63\n"
      )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int profileIndex = 0, dp = 0, up = 0, tc = 0, dscp = 0;
	unsigned int op_ret = 0;

     dcli_str2ulong((char*)argv[0],&dp);	
     dcli_str2ulong((char*)argv[1],&up);
     dcli_str2ulong((char*)argv[2],&tc);
     dcli_str2ulong((char*)argv[3],&dscp);
    
    profileIndex = (unsigned int )vty->index;	
    
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_QOS_PROFILE_ATTRIBUTE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&profileIndex,     
							 DBUS_TYPE_UINT32,&dp,
						     DBUS_TYPE_UINT32,&up,
						     DBUS_TYPE_UINT32,&tc,
						     DBUS_TYPE_UINT32,&dscp,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
			 if(QOS_RETURN_CODE_PROFILE_NOT_EXISTED == op_ret){
                vty_out(vty,"%% Qos-Profile %d not existed!\n",profileIndex);     
             }
			 else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			 else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
                vty_out(vty,"%% Fail to config qos profile\n");      
             }       
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(delete_qos_profile_cmd_func,
	  delete_qos_profile_cmd,
	  "delete qos-profile <1-127>",
	  "Delete configuration \n"
	  "Delete QoS profile\n"
	  "QoS profile range in 1-127"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int profileIndex = 0;
	unsigned int op_ret = 0;
	int ret = 0;

	ret = dcli_str2ulong((char*)argv[0],&profileIndex);	
	if(ret==QOS_RETURN_CODE_ERROR)
	{
		vty_out(vty,"%% Illegal qos profile index!\n");
		return CMD_WARNING;
	}
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_DELETE_QOS_PROFILE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profileIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{			
			if(op_ret==QOS_RETURN_CODE_PROFILE_NOT_EXISTED)
				vty_out(vty,"%% QoS profile %d not existed!\n",profileIndex);
			else if(op_ret==QOS_RETURN_CODE_PROFILE_IN_USE) {
				 vty_out(vty,"%% Since qos profile be in use ,cannot delete it\n");	 
			 }
			else if(op_ret!=QOS_RETURN_CODE_SUCCESS){
				vty_out(vty,"%% Fail to delete!\n");
			}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(delete_dscp2profile_cmd_func,
      delete_dscp2profile_cmd,
      "delete dscp-to-profile <0-63>",
      "Delete system configuration\n"
      "Delete DSCP to profile map table\n"
      "Specify DSCP range in 0-63\n"
      )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int dscp = 0;
	unsigned int op_ret = 0;
		
    dcli_str2ulong((char*)argv[0],&dscp);

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_DELETE_DSCP_PROFILE_TABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,                               						
						     DBUS_TYPE_UINT32,&dscp,					  
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
			 if(QOS_RETURN_CODE_NO_MAPPED == op_ret){
                 vty_out(vty,"%% Delete dscp to qos profile map not exist \n");      
             } 
			 else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
                 vty_out(vty,"%% Fail to delete dscp to qos profile map table \n");      
             }
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(delete_append_cmd_func,
	  delete_append_cmd,
	  "delete append <1-512>",
	  "Delete system configuration\n"
	  "Delete acl to profile map table\n"
	  "Specify acl range in 1-512\n"
	  )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int aclIndex = 0;
	unsigned int op_ret = 0;
		
	dcli_str2ulong((char*)argv[0],&aclIndex);

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_DELETE_ACL_PROFILE_TABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query, 													
							 DBUS_TYPE_UINT32,&aclIndex,					  
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
			 if(QOS_RETURN_CODE_NO_MAPPED == op_ret){
				 vty_out(vty,"%% Delete acl to qos profile map not exist \n"); 
			 } 
			 else if(op_ret == ACL_RETURN_CODE_GROUP_RULE_EXISTED){
				vty_out(vty,"%% Can't delete this acl since it is bound to group\n");
			 }	
			else if(op_ret == ACL_RETURN_CODE_EGRESS_GROUP_RULE_EXISTED){
				vty_out(vty,"%% Can't delete this acl since it is bound togroup \n");
			 }
			 else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
				 vty_out(vty,"%% Fail to delete acl to qos profile map table \n"); 	 
			 }
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(config_dscp2profile_cmd_func,
      config_dscp2profile_cmd,
      "set dscp-to-profile <0-63> <1-127>",
      "Set system configuration\n"
      "Set DSCP to profile map table\n"
      "Specify DSCP range in 0-63\n"
      "Specify profile index range in 1-127\n"
      )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int profileIndex = 0, dscp = 0;
	unsigned int op_ret = 0;
	
    dcli_str2ulong((char*)argv[0],&dscp);
    dcli_str2ulong((char*)argv[1],&profileIndex);
     
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_DSCP_PROFILE_TABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,                               						
						     DBUS_TYPE_UINT32,&dscp,
						     DBUS_TYPE_UINT32,&profileIndex, 
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
			if(QOS_RETURN_CODE_PROFILE_NOT_EXISTED ==op_ret){
				vty_out(vty,"%% QoS profile not existed!\n");
			}
			else if(ACL_RETURN_CODE_HYBRID_DSCP == op_ret) {
				vty_out(vty,"%% Qos mode is hybrid, qos-profile index is 1~64\n");  
			}
			else if(QOS_RETURN_CODE_ERROR== op_ret){
				vty_out(vty,"%% Fail to config dscp to qos profile map table \n");      
           	}       
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(delete_dcsp2dscp_cmd_func,
      delete_dcsp2dscp_cmd,
      "delete dscp-to-dscp <0-63>",
      "Delete system configuration\n"
      "Delete DSCP to DSCP remap table\n"
      "Specify old DSCP range in 0-63\n"
      )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int oldDscp = 0;
	unsigned int op_ret = 0;
		
   dcli_str2ulong((char*)argv[0],&oldDscp);
        
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_DELETE_DSCP_DSCP_TABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&oldDscp,     						
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
		if(QOS_RETURN_CODE_NO_MAPPED == op_ret) {
			vty_out(vty,"%% delete dscp to dscp remap table not exist\n");
		}
        else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
            vty_out(vty,"%% Fail to delete dscp to dscp remap table \n");      
        }       
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_dcsp2dscp_cmd_func,
      config_dcsp2dscp_cmd,
      "set dscp-to-dscp <0-63> <0-63>",
      "Set system configuration\n"
      "Set DSCP to DSCP remap table\n"
      "Specify old DSCP range in 0-63\n"
      "Specify new DSCP range in 0-63\n"
      )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int oldDscp = 0, newDscp = 0;
	unsigned int op_ret = 0;
		
      dcli_str2ulong((char*)argv[0],&oldDscp);
      dcli_str2ulong((char*)argv[1],&newDscp);
       
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_DSCP_DSCP_TABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&oldDscp,     						
						     DBUS_TYPE_UINT32,&newDscp,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
		if (ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT == op_ret) {
			 vty_out(vty,"%% device not support dscp to dscp remap table \n"); 
		}
		if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
			vty_out(vty,"%% Product not support this function!\n");
		}
		else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
		       vty_out(vty,"%% Fail to config dscp to dscp remap table \n");      
		}       
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(delete_up2profile_cmd_func,
      delete_up2profile_cmd,
      "delete up-to-profile <0-7>",
      "Delete system configuration\n"
      "Delete UP to profile table entry\n"
      "Specify UP range in 0-7\n"
      )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int up = 0;
	unsigned int op_ret = 0;

	
     dcli_str2ulong((char*)argv[0],&up);
  
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_DELETE_UP_PROFILE_TABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&up,     						
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
		if(QOS_RETURN_CODE_NO_MAPPED == op_ret) {
			vty_out(vty,"%%delete up to profile table not exist\n");
		}
		else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
            vty_out(vty,"%% Fail to delete up to profile table \n");      
        }       
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_up2profile_cmd_func,
      config_up2profile_cmd,
      "set up-to-profile <0-7> <1-127>",
      "Set system configuration\n"
      "Set UP to profile table\n"
      "Specify UP range in 0-7\n"
      "Specify QoS profile range in 1-127\n"
      )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int up = 0, profileIndex = 0;
	unsigned int op_ret = 0;

     dcli_str2ulong((char*)argv[0],&up);
     dcli_str2ulong((char*)argv[1],&profileIndex);
  
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_UP_PROFILE_TABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&up,     						
						     DBUS_TYPE_UINT32,&profileIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
			if(QOS_RETURN_CODE_PROFILE_NOT_EXISTED ==op_ret){
				vty_out(vty,"%% QoS profile not existed!\n");
			}
			else if(ACL_RETURN_CODE_HYBRID_UP== op_ret) {
				vty_out(vty,"%% Qos mode is hybrid, qos-profile index is 65~72\n");  
			}
			else if(ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT == op_ret) {
				vty_out(vty,"%% device not support up to profile for change dscp, so dscp will not change\n");
			}
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
                  	       vty_out(vty,"%% Fail to config up to profile table \n");      
           	      }       
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(delete_policy_map_cmd_func,
	  delete_policy_map_cmd,
	  "delete policy-map <1-1000>",
	  "Delete configuration \n"
	  "Delete policy map\n"
	  "Policy map range in 1-1000\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int policyIndex = 0;
	unsigned int op_ret = 0;
	
	 dcli_str2ulong((char*)argv[0],&policyIndex);	
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_DELETE_POLICY_MAP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policyIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_POLICY_NOT_EXISTED == op_ret){
				vty_out(vty,"%%  Error! Policy map %d not existed!\n",policyIndex);	   
			 }
			else if(QOS_RETURN_CODE_POLICY_MAP_BIND==op_ret){
				vty_out(vty,"%% Error! Since policy map has been binded to some port,cannot delete!\n");	  
			}
			else if (QOS_RETURN_CODE_SUCCESS!=op_ret){			 
				 vty_out(vty,"%%  Error! Fail to delete policy map\n");   
			 }
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(creat_policy_map_cmd_func,
	  creat_policy_map_cmd,
	  "create policy-map <1-1000>",
	  "Create configuration\n"
	  "Create policy-map\n"
	  "Specify policy-map range in 1-1000\n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int policyIndex = 0;
	unsigned int op_ret = 0;


	 dcli_str2ulong((char*)argv[0],&policyIndex);	
	
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_CREATE_POLICY_MAP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policyIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_POLICY_EXISTED == op_ret){
				vty_out(vty,"%% Policy map %d existed!\n",policyIndex);	   
			 }
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			else if (QOS_RETURN_CODE_SUCCESS!=op_ret){			 
				vty_out(vty,"%% Fail to create policy map\n");	  
			 }
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(config_policy_map_cmd_func,
	  config_policy_map_cmd,
	  "config policy-map <1-1000>",
	  CONFIG_STR
	  "Config policy map\n"
	  "Specify policy map range in 1-1000\n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int policyIndex=0;
	unsigned int op_ret = 0;

	dcli_str2ulong((char*)argv[0],&policyIndex);

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
											 NPD_DBUS_QOS_OBJPATH,
											 NPD_DBUS_QOS_INTERFACE,
											 NPD_DBUS_METHOD_CONFIG_POLICY_MAP);
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&policyIndex,
								 DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
		dbus_message_unref(query);
		if (NULL == reply) {
			vty_out(vty,"failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}
		if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID))
		{	
				if(QOS_RETURN_CODE_POLICY_EXISTED==op_ret){
					  if(QOS_NODE_DISTRIBUTED==vty->node){
						  vty->node = POLICY_NODE;
						  vty->index = (void*)policyIndex;						  
					  } 					 
				}
				else if(QOS_RETURN_CODE_POLICY_NOT_EXISTED == op_ret){
					vty_out(vty,"%% Policy-map %d not existed!\n",policyIndex);	   
				 }
				else if(QOS_RETURN_CODE_POLICY_MAP_BIND == op_ret){
					 vty_out(vty,"%% Since the policy-map binded to some port,cannot config the policy-map\n");	  
				 }
				else
					vty_out(vty,"%% Fail to config policy-map\n");
		} 
		else 
		{		
			if (dbus_error_is_set(&err)) 
			{
				dbus_error_free_for_dcli(&err);
			}
		}
		dbus_message_unref(reply);
		return CMD_SUCCESS;


}
DEFUN(allow_qos_mark_cmd_func,
      allow_qos_mark_cmd,
      "config qos-markers (enable|disable)",
      CONFIG_STR
      "Config QoS markers\n"
      "Allow other markers assign QoS attributes\n"
      "Not allow other markers assign QoS attributes\n"
      )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int IsEnable = 0,g_policy_index=0;
	unsigned int op_ret = 0;
	
    if(strncmp("enable",argv[0],strlen(argv[0]))==0)
        IsEnable = 0;
    else if(strncmp("disable",argv[0],strlen(argv[0]))==0)
        IsEnable =1 ;

    if(POLICY_NODE==vty->node)
	    g_policy_index = (unsigned int)vty->index; 
   
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_MODIFY_MARK_QOS);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&IsEnable,  
                             DBUS_TYPE_UINT32,&g_policy_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
            if(QOS_RETURN_CODE_SUCCESS!=op_ret){
                   vty_out(vty,"%% Fail to operate qos marker!\n");      
            }       
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
#if 0
DEFUN(set_default_up_cmd_func,
      set_default_up_cmd,
      "set default-up <0-7>",
      "Set system configuration\n"
      "Set default up\n"
      "Specify up range in 0-7\n"
      )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	
	unsigned int up,g_policy_index=0;
	unsigned int op_ret;

    up = dcli_str2ulong((char*)argv[0]);

    if(POLICY_NODE==vty->node)
	    g_policy_index = (unsigned int)vty->index;
   	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_DEFAULT_UP);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&up,  
                             DBUS_TYPE_UINT32,&g_policy_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
            if(QOS_RETURN_CODE_SUCCESS!=op_ret){
                   vty_out(vty,"%% Fail to set default up!\n");      
            }       
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(config_port_default_profile_cmd_func,
      config_port_default_profile_cmd,
      "set default-qos-profile <1-127>",
      "Set system configuration\n"
      "Set default qos profile\n"
      "Specify up range in 1-127\n"
      )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	
	unsigned int ProfileIndex,g_policy_index=0;
	unsigned int op_ret;


    ProfileIndex = dcli_str2ulong((char*)argv[0]);
	//ProfileIndex =ProfileIndex-1;

    if(POLICY_NODE==vty->node)
	    g_policy_index = (unsigned int)vty->index;
   	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_DEFAULT_QOS_PROFILE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&ProfileIndex,  
                             DBUS_TYPE_UINT32,&g_policy_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
            if(QOS_RETURN_CODE_PROFILE_NOT_EXISTED==op_ret){
                   vty_out(vty,"%% QoS profile not existed!\n");      
            }  
			else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
                 vty_out(vty,"%% Fail to set default qos profile!\n");
			}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
#endif
DEFUN(config_port_trust_mode_lay2_cmd_func,
      config_port_trust_mode_lay2_cmd,
      "trust-mode l2 up (enable|disable)",
      "Configure trust mode\n"
      "Trust mode based on Layer 2\n"
      "Modify up\n"
      "Enable modify up based on layer 2\n"
      "Disable modify up based on layer 2\n"
      )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int EnableUp = 0,g_policy_index=0;
	unsigned int op_ret = 0;

	
	if(strncmp("disable",argv[0],strlen(argv[0]))==0)
		EnableUp  = 1;	
	else if(strncmp("enable",argv[0],strlen(argv[0]))==0)
		EnableUp  = 2;	
   		
    if(POLICY_NODE==vty->node)
	    g_policy_index = (unsigned int)vty->index;
  
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_PORT_TRUST_L2_MODE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&EnableUp,  
                             DBUS_TYPE_UINT32,&g_policy_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
		if (ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT == op_ret) {
			vty_out(vty,"%% trust mode layer 2 device not suport change dscp\n");
		}
		else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
			vty_out(vty,"%% Fail to config port trust mode layer 2!\n");      
		}       
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(config_port_trust_mode_lay3_cmd_func,
      config_port_trust_mode_lay3_cmd,
      "trust-mode l3 dscp (enable|disable) remap (enable|disable)",
      "Configure trust mode\n"
      "Trust mode based on Layer 3\n"
      "Modify DSCP\n"
      "Enable modify DSCP based on layer 3\n"
      "Disable modify DSCP based on layer 3\n"
      "Remap DSCP\n"
      "Enable remap DSCP to new value\n"
      "Disable remap DSCP to a new value\n"
      )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int EnableDscp = 0,RemapFlag = 0,g_policy_index=0;
	unsigned int op_ret = 0;
	
	if(strncmp("disable",argv[0],strlen(argv[0]))==0)
		EnableDscp  = 1;
	if(strncmp("enable",argv[0],strlen(argv[0]))==0)
		EnableDscp  = 2;
    
	
	if(strncmp("enable",argv[1],strlen(argv[1]))==0)
		RemapFlag  = 1;
	else if(strncmp("disable",argv[1],strlen(argv[1]))==0)
		RemapFlag  = 0;	
    
	
    if(POLICY_NODE==vty->node)
	    g_policy_index = (unsigned int)vty->index;
    
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_PORT_TRUST_L3_MODE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&EnableDscp,  
                             DBUS_TYPE_UINT32,&RemapFlag,
                             DBUS_TYPE_UINT32,&g_policy_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
		if (ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT == op_ret) {
			vty_out(vty,"%%device not support layer 3 remap dscp\n");
		}
            	else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
                   vty_out(vty,"%% Fail to config port trust mode layer 3!\n");      
            }       
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(config_port_trust_mode_l2_l3_cmd_func,
      config_port_trust_mode_l2_l3_cmd,
      "trust-mode l2+l3 up (enable|disable) dscp (enable|disable) remap (enable|disable)",
      "Configure trust mode\n"
      "Trust mode based on Layer2+Layer3\n"
      "Modify up\n"
      "Enable modify up based on layer2\n"
      "Disable modify up based on layer2\n"
      "Modify dscp\n"
      "Enable modify dscp based on layer3\n"
      "Disable modify dscp based on layer3\n"
      "Remap dscp\n"
      "Enable remap dscp to new value\n"
      "Disable remap dscp to a new value\n"
      )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int EnableUp = 0,EnableDscp = 0,RemapFlag = 0,g_policy_index=0;
	unsigned int op_ret = 0;
	
	if(strncmp("enable",argv[0],strlen(argv[0]))==0)
		EnableUp  = 2;
	else if(strncmp("disable",argv[0],strlen(argv[0]))==0)
		EnableUp  = 1;	

	if(strncmp("enable",argv[1],strlen(argv[1]))==0)
		EnableDscp  = 2;
	else if(strncmp("disable",argv[1],strlen(argv[1]))==0)
		EnableDscp  = 1;	
    
	if(strncmp("enable",argv[2],strlen(argv[2]))==0)
		RemapFlag  = 1;
	else if(strncmp("disable",argv[2],strlen(argv[2]))==0)
		RemapFlag  = 0;	
    
	
    if(POLICY_NODE==vty->node)
	    g_policy_index = (unsigned int)vty->index;
    
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_PORT_TRUST_L2_L3_MODE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&EnableUp,
                             DBUS_TYPE_UINT32,&EnableDscp,  
                             DBUS_TYPE_UINT32,&RemapFlag,
                             DBUS_TYPE_UINT32,&g_policy_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
		if (ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT == op_ret) {
			vty_out(vty,"%% device not support trust mode layer2+layer3!\n"); 
		}
		else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
		       vty_out(vty,"%% Fail to config port trust mode layer2+layer3!\n");      
		}       
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
#if 0
DEFUN(config_port_trust_mode_untrust_cmd_func,
      config_port_trust_mode_untrust_cmd,
      "trust-mode untrust",
      "Configure trust mode\n"
      "Trust mode is untrust\n"
      )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	
	unsigned int g_policy_index=0;
	unsigned int op_ret;
   		
    if(POLICY_NODE==vty->node)
	    g_policy_index = (unsigned int)vty->index;
  
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_PORT_TRUST_UNTRUST_MODE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,                   
                             DBUS_TYPE_UINT32,&g_policy_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
            if(QOS_RETURN_CODE_SUCCESS!=op_ret){
                   vty_out(vty,"%% Fail to config port untrust mode!\n");      
            }       
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
#endif
DEFUN(bind_policy_map_cmd_func,
	  bind_policy_map_cmd,
	  "bind policy-map <1-1000>",
	  "Bind configuration\n"
	  "Policy-map\n"
	  "Specify policy-map range in 1-1000\n"
	)
{
	DBusMessage *query, *reply;
	DBusError err;

	unsigned int   eth_g_index=0,policyIndex=0;
	unsigned int   op_ret = 0,hw_ret = 0;
	unsigned int slot_no = 0;

	if(vty->node==ETH_PORT_NODE)
		eth_g_index = (unsigned int)vty->index;

	
	 dcli_str2ulong((char *)argv[0],&policyIndex);

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_ETHPORTS_OBJPATH,
                                         NPD_DBUS_ETHPORTS_INTERFACE,
                                         NPD_DBUS_ETHPORTS_METHOD_BIND_POLICY_MAP);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,                   
                             DBUS_TYPE_UINT32,&eth_g_index,
                             DBUS_TYPE_UINT32,&policyIndex,
							 DBUS_TYPE_INVALID);
	if(is_distributed == DISTRIBUTED_SYSTEM)
    {
		SLOT_PORT_ANALYSIS_SLOT(eth_g_index, slot_no);
    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
			return CMD_WARNING;
    	}
		else 
    	{
			dcli_dbus_connection_qos_port = dbus_connection_dcli[slot_no]->dcli_dbus_connection;
		}
    }
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos_port,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&hw_ret,
		DBUS_TYPE_INVALID)) {
		 if (op_ret==NPD_DBUS_ERROR_NO_SUCH_PORT)  
		 	vty_out(vty,"%% Error! no such port!\n");
		else if (op_ret == ACL_RETURN_CODE_ALREADY_FLOW)
			vty_out(vty, "%% Qos mode is flow, please change mode \n");
		else if (ACL_PORT_NOT_SUPPORT_BINDED == op_ret)
			vty_out(vty, "%% Port not support bind policy map.\n");
		else if (op_ret == ACL_RETURN_CODE_NO_QOS_MODE)
			vty_out(vty, "%% There is no qos mode, please config qos mode \n");
		else if (op_ret == ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT)
			vty_out(vty, "%% device not support acl group and policy-map binded in the same port \n");
		else if (op_ret==QOS_RETURN_CODE_POLICY_MAP_BIND)
			vty_out(vty,"%% Port has binded policy-map yet\n");
		else if (op_ret==QOS_RETURN_CODE_POLICY_NOT_EXISTED)
			vty_out(vty,"%% Policy-map not existed!\n");
		else if (op_ret==ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT)
			vty_out(vty,"%% device not support l2+l3 mode, please change trust mode !\n");
		else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
			vty_out(vty,"%% Product not support this function!\n");
		else if (op_ret!=QOS_RETURN_CODE_SUCCESS)
			vty_out(vty,"%% Bind policy-map fail!\n");
		else if (hw_ret!=QOS_RETURN_CODE_SUCCESS)
			vty_out(vty,"%% Error happend as write to hw!\n");
	} 
	else {		
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}

DEFUN(unbind_policy_map_cmd_func,
	  unbind_policy_map_cmd,
	  "unbind policy-map <1-1000>",
	  "Unbind configuration \n"
	  "Unbind policy map\n"
	  "Policy map range in 1-1000\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;

	unsigned int   eth_g_index=0,policyIndex=0;
	unsigned int   op_ret = 0,hw_ret = 0;
	unsigned int slot_no = 0;

	if(vty->node==ETH_PORT_NODE)
		eth_g_index = (unsigned int)vty->index;

	dcli_str2ulong((char *)argv[0],&policyIndex);

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_ETHPORTS_OBJPATH,
										 NPD_DBUS_ETHPORTS_INTERFACE,
										 NPD_DBUS_ETHPORTS_METHOD_UNBIND_POLICY_MAP);
	
	dbus_error_init(&err);
	dbus_message_append_args(query, 				  
							 DBUS_TYPE_UINT32,&eth_g_index,
							 DBUS_TYPE_UINT32,&policyIndex,
							 DBUS_TYPE_INVALID);
	if(is_distributed == DISTRIBUTED_SYSTEM)
    {
		SLOT_PORT_ANALYSIS_SLOT(eth_g_index, slot_no);
    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
			return CMD_WARNING;
    	}
		else 
    	{
			dcli_dbus_connection_qos_port = dbus_connection_dcli[slot_no]->dcli_dbus_connection;
			vty_out(vty, "Unbind qos Policy-map of port on slot %d.\n", slot_no);
		}
    }
		
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos_port,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&hw_ret,
		DBUS_TYPE_INVALID))
	{	
		  if(op_ret==NPD_DBUS_ERROR_NO_SUCH_PORT)  
			vty_out(vty,"%% Error! no such port!\n");
		  else if(op_ret==QOS_RETURN_CODE_POLICY_MAP_PORT_WRONG)
			vty_out(vty,"%% Policy-map index wrong!\n");
		  else if(op_ret==QOS_RETURN_CODE_POLICY_NOT_EXISTED)
		  	vty_out(vty,"%% No policy map information on port!\n");
		  else if((op_ret!=QOS_RETURN_CODE_SUCCESS)||(hw_ret!=QOS_RETURN_CODE_SUCCESS))
			vty_out(vty,"%% Unbind policy-map fail!\n");
	} 
	
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_acl_ingress_policy_qos_cmd_func,
	config_acl_ingress_policy_qos_cmd,
	"acl extended <1-1000> ingress-qos <1-127> sub-qos-markers (enable|disable) source-up (<0-7>|none) source-dscp (<0-63>|none) [policer] [<1-255>]",
	ACL_STR
	"Extended acl rule\n"
	"Extended rule index range in 1-500\n"
	"Action as ingress qos policy\n"
	"Match QoS profile 1-127\n"
	"Config QoS-markers\n"
	"Allow other markers assign QoS attributes\n"
	"Not allow other markers assign QoS attributes\n"
	"Source up obtained from packets\n"
	"Modify source up which ranges in 0-7 to a new value in QoS profile\n"
	"Keep source up\n"
	"Source DSCP obtained from packets\n"
	"Modify source DSCP which ranges in 0-63 to a new value in QoS profile\n"
	"Keep source DSCP\n"
	"Allow policing on ingress port\n"
	"Policer Id range in 1-255\n"
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	unsigned int	ruleIndex=0;
	unsigned int    profileIndex=0,up=10,dscp=100;
	unsigned int    op_ret = 0;
	unsigned int    policer=0,policerId=0;
	unsigned int    remark=0;
	unsigned int    ruleType=0;
	int ret=0;
	
	dcli_str2ulong((char *)argv[0],&ruleIndex);
	ruleType = EXTENDED_ACL_RULE; 
	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 512!\n");
		return CMD_WARNING;
	}
	ruleIndex -=1;

	 dcli_str2ulong((char*)argv[1],&profileIndex);
	
	if(strncmp("enable",argv[2],strlen(argv[2]))==0)
		remark = 0;
	else if(strncmp("disable",argv[2],strlen(argv[2]))==0)
		remark =1 ;
		
	if((strncmp("none",argv[3],strlen(argv[3]))==0)&&(strncmp("none",argv[4],strlen(argv[4]))==0))
	{
		vty_out(vty,"%% sourceUp and sourceDscp cannot be both none,acl set invalid!\n");
		return CMD_WARNING;
	}

	if(strncmp("none",argv[3],strlen(argv[3]))!=0){
		
		dcli_str2ulong((char*)argv[3],&up);	
	}
	if(strncmp("none",argv[4],strlen(argv[4]))!=0){
		
		dcli_str2ulong((char*)argv[4],&dscp);	
	}
	
	/*policer*/
	if((argc>5)&&(argc<8)){		
		if(strncmp("policer",argv[5],strlen(argv[5])) == 0){
			policer = 1;
			ret=dcli_str2ulong((char *)argv[6],&policerId);
			if(ret==QOS_RETURN_CODE_ERROR)
			{
				vty_out(vty,"%% Illegal policer ID!\n");
				return CMD_WARNING;
			}
		}
		else{
			vty_out(vty,"%% unknown command\n");
			return CMD_WARNING;
		}						
	}
	else if(argc>7){
		vty_out(vty,"%% too many param\n");
		return CMD_WARNING;
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                     	 NPD_DBUS_METHOD_SET_QOS_INGRESS_POLICY_BASE_ON_ACL);		
	dbus_error_init(&err);
	dbus_message_append_args(query, 				  
							 DBUS_TYPE_UINT32,&ruleIndex,
							 DBUS_TYPE_UINT32,&ruleType,
							 DBUS_TYPE_UINT32,&profileIndex,
							 DBUS_TYPE_UINT32,&up,
							 DBUS_TYPE_UINT32,&dscp,
							 DBUS_TYPE_UINT32,&policer,
							 DBUS_TYPE_UINT32,&policerId,
							 DBUS_TYPE_UINT32,&remark,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,	
		DBUS_TYPE_INVALID)){	
		 if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
			else if (ACL_RETURN_CODE_ALREADY_PORT == op_ret)
				vty_out(vty, "%% Qos mode is not flow, please change mode \n");
			else if (op_ret == ACL_RETURN_CODE_NO_QOS_MODE)
				vty_out(vty, "%% There is no qos mode, please config qos mode \n");
			else if (ACL_RETURN_CODE_HYBRID_FLOW== op_ret)
				vty_out(vty, "%% Qos mode is not hybrid, qos-profile is 72~127 \n");
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
		         	vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
				vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));
			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%% identical fields of packet can not set again\n");	
			else if(QOS_RETURN_CODE_PROFILE_NOT_EXISTED ==op_ret)
				vty_out(vty,"%% QoS profile %d not existed!\n",profileIndex);
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			else if(QOS_RETURN_CODE_SUCCESS!=op_ret)
				vty_out(vty,"%% Set QOS ingress-policy based on ACL fail!\n");	
	} 		
	else {		
		if (dbus_error_is_set(&err)){
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(config_acl_egress_policy_qos_cmd_func,
	  config_acl_egress_policy_qos_cmd,
	  "acl extended <1-1000> egress-qos egress-up <0-7> egress-dscp <0-63> source-up (<0-7>|none) source-dscp (<0-63>|none)",
	  ACL_STR
	  "Extended acl rule\n"
	  "Extended rule index range in 1-500\n"
	  "Action as egress qos policy\n"
	  "Set egress up value if allow modify up\n"
	  "Egress up value range in 0-7\n"
	  "Set egress DSCP value if allow modify dscp\n"
	  "Egress egress DSCP value range in 0-63\n"
	  "Source up obtained from packets\n"
	  "Modify source UP which ranges in 0-7 to a new value in QoS profile\n"
	  "Keep source UP\n"
	  "Source DSCP obtained from packets\n"
	  "Modify source DSCP which ranges in 0-63 to a new value in QoS profile\n"
	  "Keep source DSCP\n"
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	unsigned int	ruleIndex=0,ruleType=0;
	unsigned int    up=10,dscp=100,egrUp=0,egrDscp=0;
	unsigned int    op_ret;
	unsigned int    policer=0,policerId=0;

	if(argc>5){
		vty_out(vty,"%% Bad command paramter!\n");
		return CMD_WARNING;
	}
	
	dcli_str2ulong((char *)argv[0],&ruleIndex);
	ruleType = EXTENDED_ACL_RULE; 
	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		vty_out(vty,"extended rule must less than 512!\n");
		return CMD_WARNING;
	}
	ruleIndex -=1;

	dcli_str2ulong((char*)argv[1],&egrUp);
	dcli_str2ulong((char*)argv[2],&egrDscp);

	if((strncmp("none",argv[3],strlen(argv[3]))==0)&&(strncmp("none",argv[4],strlen(argv[4]))==0))
	{
		vty_out(vty,"%% sourceUp and sourceDscp cannot be both none,acl set invalid!\n");
		return CMD_WARNING;
	}
	if(strncmp("none",argv[3],strlen(argv[3]))!=0){
	
		dcli_str2ulong((char*)argv[3],&up);
	}
	if(strncmp("none",argv[4],strlen(argv[4]))!=0){
		
		dcli_str2ulong((char*)argv[4],&dscp);
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_QOS_EGRESS_POLICY_BASE_ON_ACL);		
	dbus_error_init(&err);
	dbus_message_append_args(query, 				  
							 DBUS_TYPE_UINT32,&ruleIndex,
							 DBUS_TYPE_UINT32,&ruleType,
							 DBUS_TYPE_UINT32,&egrUp,
							 DBUS_TYPE_UINT32,&egrDscp,
							 DBUS_TYPE_UINT32,&up,
							 DBUS_TYPE_UINT32,&dscp,					
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,	
		DBUS_TYPE_INVALID)){			  
			if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
				vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
			else if (ACL_RETURN_CODE_ALREADY_PORT == op_ret)
				vty_out(vty, "%% Qos mode is not flow, please change mode \n");
			else if (op_ret == ACL_RETURN_CODE_NO_QOS_MODE)
				vty_out(vty, "%% There is no qos mode, please config qos mode \n");
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
		         	vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
				vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl, but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));
			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
				vty_out(vty,"%% identical fields of packet can not set again\n");	
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			else if(QOS_RETURN_CODE_SUCCESS!=op_ret)
				vty_out(vty,"%% Set QOS egress-policy based on ACL fail!\n");	
	} 		
	else {		
		if (dbus_error_is_set(&err)){
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(show_qos_profile_cmd_func,
	  show_qos_profile_cmd,
	  "show qos-profile",
	  SHOW_STR
	  "Show qos profile\n"
)
{
	 DBusMessage *query = NULL, *reply = NULL;
	 DBusError err;
	 DBusMessageIter  iter;
	 DBusMessageIter  iter_array;

	 unsigned int profileIndex=0,TC=0,DP=0,DSCP=0,UP=0;
	 unsigned int ret,count=0,j=0;
	 unsigned char drop[10]={0};
	 
	 query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_QOS_OBJPATH,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_SHOW_QOS_PROFILE);
	 dbus_error_init(&err);
	 reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	 dbus_message_unref(query);
	 if (NULL == reply) 
	 {
		 /*vty_out(vty,"failed get reply.\n");*/
		 if (dbus_error_is_set(&err)) 
		 {
			 dbus_error_free_for_dcli(&err);
		 }
		 return CMD_SUCCESS;
	 }
	 dbus_message_iter_init(reply,&iter);

	 dbus_message_iter_get_basic(&iter,&ret);

	if(QOS_RETURN_CODE_SUCCESS == ret){
		 dbus_message_iter_next(&iter);  
		 dbus_message_iter_get_basic(&iter,&count);
		 dbus_message_iter_next(&iter); 	 
		 dbus_message_iter_recurse(&iter,&iter_array);
				   
		 for (j = 0; j < count; j++) {
			 DBusMessageIter iter_struct;
			 dbus_message_iter_recurse(&iter_array,&iter_struct);			 
			 dbus_message_iter_get_basic(&iter_struct,&profileIndex);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&TC);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&DP);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&UP);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&DSCP);
			 dbus_message_iter_next(&iter_array); 
			 switch(DP){
			 	case 0:strcpy(drop,"Green");		 break;
			 	case 1:
			 	case 2:strcpy(drop,"Red"); 		     break;	
				default: break; 	 
		 	 }
		    vty_out(vty,"===============================================\n");
		    vty_out(vty,"%-40s: %d\n","QOS Profile",profileIndex);
			vty_out(vty,"%-40s: %d\n","Traffic Class",TC);
			vty_out(vty,"%-40s: %s\n","Drop Precedence",drop);
			vty_out(vty,"%-40s: %d\n","User Priority",UP);
			vty_out(vty,"%-40s: %d\n","DSCP",DSCP);
			vty_out(vty,"===============================================\n");
		 }
	}
	else if(QOS_RETURN_CODE_PROFILE_NOT_EXISTED== ret) 
		vty_out(vty,"%% Error: No QoS profile exists.\n");
	
	dbus_message_unref(reply);
	 return CMD_SUCCESS;
		 
}
DEFUN(show_policy_map_cmd_func,
	  show_policy_map_cmd,
	  "show policy-map",
	  SHOW_STR
	  "Show policy map\n"
)
{
	 DBusMessage *query = NULL, *reply = NULL;
	 DBusError err;
	 DBusMessageIter  iter;
	 DBusMessageIter  iter_array;

	/* unsigned int defaultUp=0,defaultQproIndex=0;*/
	 unsigned int ModUpEn=0,ModDscpEn=0,Dp=0,TrustFlag=0,remapDscp=0;
	 unsigned int policyMapIndex=0,slot_no=0,port_no=0,pmcount=0;
	 unsigned int ret,count=0,j=0,k=0;
	 unsigned char trustMem[25],modiUp[10],modiDscp[10],droppre[10],remaps[10];
	 
	 query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_QOS_OBJPATH,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_SHOW_POLICY_MAP);
	 dbus_error_init(&err);
	 reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	 dbus_message_unref(query);
	 if (NULL == reply) 
	 {
		 vty_out(vty,"show policy-map failed get reply.\n");/*remove*/
		 if (dbus_error_is_set(&err)) 
		 {
			 dbus_error_free_for_dcli(&err);
		 }
		 return CMD_SUCCESS;
	 }
	 dbus_message_iter_init(reply,&iter);

	 dbus_message_iter_get_basic(&iter,&ret);

	 if(QOS_RETURN_CODE_SUCCESS == ret){
		 dbus_message_iter_next(&iter);  
		 dbus_message_iter_get_basic(&iter,&count);
		 dbus_message_iter_next(&iter); 	 
		 dbus_message_iter_recurse(&iter,&iter_array);
				   
		 for (j = 0; j < count; j++) {
			 DBusMessageIter iter_struct;
			 DBusMessageIter iter_sub_array;
			 dbus_message_iter_recurse(&iter_array,&iter_struct);			 
			 dbus_message_iter_get_basic(&iter_struct,&policyMapIndex);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&Dp);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&TrustFlag);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&ModUpEn);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&ModDscpEn);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&remapDscp);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&pmcount);
			 dbus_message_iter_next(&iter_struct); 

			 switch(TrustFlag){
			 	case 0:strcpy(trustMem,"Untrust");			 	 break;
			 	case 1:strcpy(trustMem,"Trust layer2");		 break;
			 	case 2:strcpy(trustMem,"Trust layer3"); 		 break;
			 	case 3:strcpy(trustMem,"Trust layer2+layer3"); break;			
				default: break; 	 
		 	 }
		 	switch(ModUpEn){
			 	case 0:strcpy(modiUp,"Keep");	     break;
			 	case 1:strcpy(modiUp,"Disable");		 break;
			 	case 2:strcpy(modiUp,"Enable"); 		 break;			 	
				default: break; 	 
		 	 }
			switch(ModDscpEn){
			 	case 0:strcpy(modiDscp,"Keep");	 break;
			 	case 1:strcpy(modiDscp,"Disable");   break;
			 	case 2:strcpy(modiDscp,"Enable"); 	 break;			 	
				default: break; 	 
		 	 }
			switch(Dp){
			 	case 0:strcpy(droppre,"enable");	 break;
			 	case 1:strcpy(droppre,"disable");   break;		 	
				default: break; 	 
		 	 }
			switch(remapDscp){
			 	case 0:strcpy(remaps,"Disable");	 break;
			 	case 1:strcpy(remaps,"Enable");   break;		 	
				default: break; 	 
		 	 }
			vty_out(vty,"===============================================\n");
			vty_out(vty,"%-40s: %d\n","Policy Map",policyMapIndex);			
			vty_out(vty,"%-40s: %s\n","QoS sub-markers",droppre);
			vty_out(vty,"%-40s: %s\n","TrustMode",trustMem);
			vty_out(vty,"%-40s: %s\n","Modify UP",modiUp);
			vty_out(vty,"%-40s: %s\n","Modify DSCP",modiDscp);
			vty_out(vty,"%-40s: %s\n","Remap DSCP",remaps);

			dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			for (k = 0; k < pmcount; k++){
				  DBusMessageIter iter_sub_struct;
				  dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
							  
				  dbus_message_iter_get_basic(&iter_sub_struct,&slot_no);
				  dbus_message_iter_next(&iter_sub_struct);

				  dbus_message_iter_get_basic(&iter_sub_struct,&port_no);
			         dbus_message_iter_next(&iter_sub_struct);
			         vty_out(vty,"%-40s: %d/%d\n","binded by port",slot_no, port_no);

			         dbus_message_iter_next(&iter_sub_array);
			  }

			dbus_message_iter_next(&iter_struct); 
			dbus_message_iter_next(&iter_array); 
			
			vty_out(vty,"===============================================\n");
		 }
	}
	else if(QOS_RETURN_CODE_POLICY_NOT_EXISTED== ret) 
		vty_out(vty,"%% Error: No Policy map exists.\n");
	
	dbus_message_unref(reply);
	 return CMD_SUCCESS;
		 
}

DEFUN(show_port_qos_cmd_func,
	  show_port_qos_cmd,
	  "show port-qos",
	  SHOW_STR
	  "Show QoS information on port\n"
)
{
	 DBusMessage *query = NULL, *reply = NULL;
	 DBusError err;
	 DBusMessageIter  iter;
	 DBusMessageIter  iter_array;
	 unsigned int 			 policyIndex=0,op_ret,g_eth_index=0;
	 unsigned int 			 slot_no=0,port_no=0;
	 
	 if(vty->node==ETH_PORT_NODE)
		g_eth_index = (unsigned int)vty->index;
	
	 query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,\
	 									 NPD_DBUS_ETHPORTS_OBJPATH,\
                                         NPD_DBUS_ETHPORTS_INTERFACE,\
                                         NPD_DBUS_ETHPORT_METHOD_SHOW_POLICY_MAP);
	 dbus_error_init(&err);
	 dbus_message_append_args(query,
						     DBUS_TYPE_UINT32,&g_eth_index,
							 DBUS_TYPE_INVALID);
	 if(is_distributed == DISTRIBUTED_SYSTEM)
    {
		SLOT_PORT_ANALYSIS_SLOT(g_eth_index, slot_no);
    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
			return CMD_WARNING;
    	}
		else 
    	{
			dcli_dbus_connection_qos_port = dbus_connection_dcli[slot_no]->dcli_dbus_connection;
		}
    }
	
	 reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos_port,query,-1, &err);
	 dbus_message_unref(query);
	 if (NULL == reply) 
	 {
		 /*vty_out(vty,"failed get reply.\n");*/
		 if (dbus_error_is_set(&err)) 
		 {
			 dbus_error_free_for_dcli(&err);
		 }
		 return CMD_SUCCESS;
	 }
	 if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,  &slot_no,
		DBUS_TYPE_UINT32,  &port_no,
		DBUS_TYPE_UINT32,  &policyIndex,
		DBUS_TYPE_INVALID)){	
			if(op_ret ==QOS_RETURN_CODE_POLICY_MAP_BIND){
				vty_out(vty,"================================================\n");
				vty_out(vty,"%-40s: %d/%d\n","Interface port",slot_no,port_no);
				vty_out(vty,"%-40s: %d\n","Policy map index",policyIndex);		
				vty_out(vty,"================================================\n");
			}
			else if(op_ret==QOS_RETURN_CODE_POLICY_NOT_EXISTED){
				vty_out(vty,"================================================\n");
				vty_out(vty,"%-40s: %d/%d\n","Interface port",slot_no,port_no);
				vty_out(vty,"%-40s: %s\n","Policy map index","null");				
				vty_out(vty,"================================================\n");
			}
			else if(op_ret==NPD_DBUS_ERROR_NO_SUCH_PORT){
				vty_out(vty,"%% Error! illegal port index!\n");
			}
	 }
	else {		
		if (dbus_error_is_set(&err)){
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}
DEFUN(show_global_remap_cmd_func,
	  show_global_remap_cmd,
	  "show remap-table",
	  SHOW_STR
	  "Show remap table with UP and DSCP\n"
)
{
	 DBusMessage *query = NULL, *reply = NULL;
	 DBusError err;
	 DBusMessageIter  iter;
	 DBusMessageIter  iter_array;
	 unsigned int	  upCount=0,dscpCount=0,dscpReCount=0,countVal=0;
	 unsigned int	  i=0,j=0;
	 unsigned int    profileIndex=0;
	 unsigned int    l=0,m=0,n=0;
	 unsigned int    flag=0;
	 DCLI_QOS_REMAP_SHOW_STC *getIndex[150];

	 query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,\
										  NPD_DBUS_QOS_OBJPATH,
										  NPD_DBUS_QOS_INTERFACE,
										  NPD_DBUS_ETHPORT_METHOD_SHOW_REMAP_TABLE);
	 dbus_error_init(&err);
	
	 reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	 dbus_message_unref(query);
	 if (NULL == reply) 
	 {
		 /*vty_out(vty,"failed get reply.\n");*/
		 if (dbus_error_is_set(&err)) 
		 {
			 dbus_error_free_for_dcli(&err);
		 }
		 return CMD_SUCCESS;
	 }

	 dbus_message_iter_init(reply,&iter);
	 dbus_message_iter_get_basic(&iter,&upCount);
	 dbus_message_iter_next(&iter);  
	 dbus_message_iter_get_basic(&iter,&dscpCount);
	 dbus_message_iter_next(&iter);  
	 dbus_message_iter_get_basic(&iter,&dscpReCount);
	 dbus_message_iter_next(&iter);  
	 dbus_message_iter_get_basic(&iter,&countVal);
	 dbus_message_iter_next(&iter); 	 
		 
	 dbus_message_iter_recurse(&iter,&iter_array);					   
	 for (i = 0; i< countVal; i++){
		 DBusMessageIter iter_struct;

		 getIndex[i]=(DCLI_QOS_REMAP_SHOW_STC *)malloc(sizeof(DCLI_QOS_REMAP_SHOW_STC));
		 if(NULL==getIndex[i]) { vty_out(vty,"malloc fail\n"); }		
		 else { memset(getIndex[i],0,sizeof(DCLI_QOS_REMAP_SHOW_STC)); }

		 dbus_message_iter_recurse(&iter_array,&iter_struct);	
		 dbus_message_iter_get_basic(&iter_struct,&(getIndex[i]->flag));
		 dbus_message_iter_next(&iter_struct);
		 dbus_message_iter_get_basic(&iter_struct,&(getIndex[i]->profileIndex));		 
		 dbus_message_iter_next(&iter_array); 
	}

	
	 if(upCount>0){
	 	 vty_out(vty,"=============================\n");
		 vty_out(vty,"%s\n","UP->QoSProfile Mapping");
	 	

		for(l=0;l<upCount;l++){
			 vty_out(vty,"%d-->%d\n",getIndex[l]->flag,getIndex[l]->profileIndex);
		}
	 }	
	
	 
	 if(dscpCount>0){
	 	vty_out(vty,"=============================\n");
	 	vty_out(vty,"%s\n","DSCP->QoSProfile Mapping");
	 	
	 
		for(m=0;m<dscpCount;m++){
			 vty_out(vty,"%d-->%d\n",getIndex[upCount+m]->flag,getIndex[upCount+m]->profileIndex);
		}
	 }
	
	 
	 if(dscpReCount>0){
	 	vty_out(vty,"=============================\n");
	 	vty_out(vty,"%s\n","DSCP->DSCP Remapping");
	 	
		
		for(n=0;n<dscpReCount;n++){
			 vty_out(vty,"%d-->%d\n",getIndex[upCount+dscpCount+n]->flag,getIndex[upCount+dscpCount+n]->profileIndex);
		}
	 }
	if((upCount==0)&&(dscpCount==0)&&(dscpReCount==0)){
		vty_out(vty,"%% No any remap info\n");
	}
	
	for(l=0; l<upCount; l++){
		free(getIndex[l]);
	}
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_policer_cmd_func,
	  config_policer_cmd,
	  "set policer <1-255>",
	  SETT_STR
	  "Set policer\n"
	  "Specify policer range in 1-255\n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int policerIndex = 0;
	unsigned int op_ret = 0;
	
	 dcli_str2ulong((char*)argv[0],&policerIndex);	
	
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_CONFIG_POLICER);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policerIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if((QOS_RETURN_CODE_SUCCESS==op_ret)||(QOS_POLICER_DISABLE==op_ret)){
				 if(QOS_NODE_DISTRIBUTED==vty->node){
					  vty->node = POLICER_NODE;
					  vty->index = (void*)policerIndex;						  
				  } 
			}
			else if(QOS_POLICER_ENABLE==op_ret)
				vty_out(vty,"%% you should disable the policer firstly!\n");
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			else 			 
				 vty_out(vty,"%% config policer fail \n");	  
			 
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(set_cir_cbs_cmd_func,
	  set_cir_cbs_cmd,
	  "policer cir <1-100000000> cbs <1-2000000000>",
	  "Configure policer\n" 
	  "Policer cir,commited rate in kbps\n"
	  "Specify cir range in 1-100000000 kbps\n"
	  "Policer cbs,commited burst size in bytes\n"
	  "Specify cbs range in 1-2000000000 B\n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int policerIndex = 0;
	unsigned int op_ret = 0;
	unsigned int cir = 0,cbs = 0;
	
	dcli_str2ulong((char*)argv[0],&cir);	
	dcli_str2ulong((char*)argv[1],&cbs); 

	if(POLICER_NODE==vty->node)
		policerIndex = (unsigned int)vty->index;
	else
		return CMD_WARNING;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_CONFIG_CIR_CBS);
	dbus_error_init(&err);
	dbus_message_append_args(query,
		 					 DBUS_TYPE_UINT32,&policerIndex,
							 DBUS_TYPE_UINT32,&cir,
							 DBUS_TYPE_UINT32,&cbs,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_ERROR == op_ret){
				vty_out(vty,"%% config cir cbs fail !\n");					  				   
			}
			else if(QOS_RETURN_CODE_POLICER_CBS_BIG == op_ret){
				vty_out(vty,"%% policer cbs is too big ! cbs < cir*40\n");					  				   
			}
			else if(QOS_RETURN_CODE_POLICER_CBS_LITTLE == op_ret){
				vty_out(vty,"%% policer cbs is too little ! cbs>2000 and cbs>cir\n");					  				   
			}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(set_out_profile_cmd_func,
	  set_out_profile_cmd,
	  "config out-profile",
	  CONFIG_STR 
	  "Config out profile action\n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int policerIndex = 0;
	unsigned int op_ret = 0;
	
	if(POLICER_NODE==vty->node)
		policerIndex = (unsigned int)vty->index;
	else
		return CMD_WARNING;
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_CONFIG_OUT_PROFILE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policerIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_SUCCESS==op_ret){
				 if(POLICER_NODE==vty->node){
					  vty->node = OUT_PROFILE_NODE;
					  vty->index = (void*)policerIndex;						  
				  } 
			}
			else if (ACL_RETURN_CODE_ALREADY_PORT == op_ret)
				vty_out(vty, "%% Qos mode is not flow, please change mode \n");
			else if (op_ret == ACL_RETURN_CODE_NO_QOS_MODE)
				vty_out(vty, "%% There is no qos mode, please config qos mode \n");
			else if (ACL_RETURN_CODE_HYBRID_FLOW== op_ret)
				vty_out(vty, "%% Qos mode is not hybrid, qos-profile is 72~127 \n");
			else 			 
				 vty_out(vty,"%% config out profile fail \n");	  
			 
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(set_out_profile_action_drop_keep_cmd_func,
	  set_out_profile_action_drop_keep_cmd,
	  "(keep|drop)",
	  "Keep after metering as out of profile\n"
	  "Drop after metering as out of profile\n"  
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int policerIndex = 0,action = 0;
	unsigned int op_ret = 0;
	
	if(OUT_PROFILE_NODE==vty->node)
		policerIndex = (unsigned int)vty->index;

	if(strcmp(argv[0],"keep")==0)
		action = 0;
	else if(strcmp(argv[0],"drop")==0)
		action=1;
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_OUT_PROFILE_DROP_KEEP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policerIndex,
							 DBUS_TYPE_UINT32,&action,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"%% failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_SUCCESS!=op_ret){
				vty_out(vty,"%% config out profile action fail \n");	  
			}		 
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(set_out_profile_action_remap_cmd_func,
	  set_out_profile_action_remap_cmd,
	  "remap <1-127>",
	  "Remap to QoS profile after metering as out of profile\n"
	  "QoS profile index 1-127\n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int policerIndex = 0,profileIndex = 0;
	unsigned int op_ret = 0;
	
	if(OUT_PROFILE_NODE==vty->node)
		policerIndex = (unsigned int)vty->index;

	dcli_str2ulong((char*)argv[0],&profileIndex);
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_OUT_PROFILE_REMAP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policerIndex,
							 DBUS_TYPE_UINT32,&profileIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_PROFILE_NOT_EXISTED == op_ret){
				vty_out(vty,"%% QoS profile %d not existed!\n",profileIndex);
			}
			else if (ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT == op_ret) {
				vty_out(vty,"%% device not support remap qos profile \n");
			}
			else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
				 vty_out(vty,"%% config out profile action fail \n");
			}			
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(set_color_mode_cmd_func,
	  set_color_mode_cmd,
	  "color-mode (blind|aware)",
	  "Set policer color mode\n"
	  "Packet's conformance level is determined by the Token Bucket meter result only\n"
	  "Packet's conformance level is determined by the Token Bucket meter result and the incoming QoSProfile DP\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int color = 0,policerIndex=0;
	unsigned int op_ret = 0;

	if(POLICER_NODE==vty->node)
		policerIndex = (unsigned int)vty->index;
		
	if(strcmp(argv[0],"blind")==0)
		color = 0;
	else if(strcmp(argv[0],"aware")==0)
		color = 1;	
	else
		vty_out(vty,"%% Bad parameter!\n");
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_POLICER_COLOR);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policerIndex,
							 DBUS_TYPE_UINT32,&color,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_SUCCESS!=op_ret){
				 vty_out(vty,"%% config policer color mode fail \n");	  
			}			 
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(enable_counter_cmd_func,
	  enable_counter_cmd,
	  "counter <1-15> (enable|disable)",	 
	  "Set a counter for policer\n"
	  "Counter index 1-15\n"
	  "Enable counter for the policer\n"
	  "Disable counter for the polcier\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int	 policerIndex=0,coutIndex=0,IsEnable = 0;
	unsigned int	 op_ret = 0;

	dcli_str2ulong((char *)argv[0],&coutIndex);
	
	if(POLICER_NODE==vty->node)
		policerIndex = (unsigned int)vty->index;

	if(strncmp("enable",argv[1],strlen(argv[1]))==0)
		IsEnable = 1;
	else if(strncmp("disable",argv[1],strlen(argv[1]))==0)
		IsEnable = 0;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_POLICER_COUNTER);
	dbus_error_init(&err);
	dbus_message_append_args(query, 					
							 DBUS_TYPE_UINT32,&policerIndex,
							 DBUS_TYPE_UINT32,&coutIndex,
							 DBUS_TYPE_UINT32,&IsEnable,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_COUNTER_NOT_EXISTED==op_ret){
				vty_out(vty," %% counter %d not existed!\n",coutIndex); 
			}
			else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
				 vty_out(vty,"%% set counter for policer fail \n");	  
			}			 
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(set_policer_share_cmd_func,
	  set_policer_share_cmd,
	  "set mode (share|noshare)",	 
	  "Set a mode for policer\n"
	  "mode share or noshare\n"
	  "set mode share for the policer\n"
	  "set mode noshare for the polcier\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int	 policerIndex=0,coutIndex=0,IsShare = 0;
	unsigned int	 op_ret = 0;
	
	if(POLICER_NODE==vty->node) {
		policerIndex = (unsigned int)vty->index;
	}
	/*0 for share, 1 for noshare*/
	if(strncmp("share",argv[0],strlen(argv[0]))==0) {
		IsShare = 0;
	}
	else if(strncmp("noshare",argv[0],strlen(argv[0]))==0) {
		IsShare = 1;
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_POLICER_SHARE);
	dbus_error_init(&err);
	dbus_message_append_args(query, 					
							 DBUS_TYPE_UINT32,&policerIndex,
							 DBUS_TYPE_UINT32,&coutIndex,
							 DBUS_TYPE_UINT32,&IsShare,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_COUNTER_NOT_EXISTED==op_ret){
				vty_out(vty," %% counter %d not existed!\n",coutIndex); 
			}
			else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
				 vty_out(vty,"%% set counter for policer fail \n");	  
			}			 
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_policer_enable_cmd_func,
	  config_policer_enable_cmd,
	  "policer <1-255> (enable|disable)",
	  "Configure policer\n" 
	  "Policer index 1-255\n"
	  "Enable policer\n"
	  "Disable policer\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int policerIndex=0,IsEnable = 0;
	unsigned int op_ret = 0;
	unsigned long	cir=0,cbs=0;

	 dcli_str2ulong((char *)argv[0],&policerIndex);
		
	if(strncmp("enable",argv[1],strlen(argv[1]))==0)
		IsEnable = 1;
	else if(strncmp("disable",argv[1],strlen(argv[1]))==0)
		IsEnable =0;
	else
		vty_out(vty,"%% Bad parameter!\n");
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_POLICER_ENABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,						
							 DBUS_TYPE_UINT32,&policerIndex,
							 DBUS_TYPE_UINT32,&IsEnable,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&cir,
		DBUS_TYPE_UINT32,&cbs,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_BAD_PTR==op_ret){
				 vty_out(vty,"%% policer %d not existed!\n",policerIndex);	  
			}
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
				vty_out(vty,"%% Product not support this function!\n");
			}
			else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
				vty_out(vty,"%% fail operate policer \n");
			}
			/* used for hardware only */
			/*
			else{
				vty_out(vty,"%% actual CIR %ld CBS %ld in hardware!\n",cir,cbs);	
			}
			*/
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(set_g_strict_meter_mode_cmd_func,
	  set_g_strict_meter_mode_cmd,
	  "policer strict packetsize (l1|l2|l3)",
	  "Configure policer\n" 
	  "Meter mode strict\n"
	  "Strict meter mode with packet sizes\n"
	  "Packet size preamble+IPG+CRC\n"
	  "Packet size L2+L3+header+CRC\n,"
	  "Packet size L3+packet without CRC\n"	  
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int packetsize=0;
	unsigned int op_ret = 0;

	if(strcmp(argv[0],"l1")==0)
		packetsize = 3;
	else if(strcmp(argv[0],"l2")==0)
		packetsize = 2;
	else if(strcmp(argv[0],"l3")==0)
		packetsize = 1;
	else
		vty_out(vty,"%% Bad parameter!\n");
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_GLOBAL_PACKET_SIZE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&packetsize,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{		
			if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
				vty_out(vty,"%% Product not support this function!\n");
			}
			else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
				 vty_out(vty,"%% config global policer strict mode and metering packet size fail \n");   
			}			 
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(set_g_loose_meter_mode_cmd_func,
	  set_g_loose_meter_mode_cmd,
	  "policer loose mru <0-2>",
	  "Configure policer\n" 
	  "Meter mode loose\n"
	  "Loose meter mode with MRU sizes\n"
	  "MRU sizes(0=1.5KB ,1=2KB,2=10KB)\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int mode=0,mru=0;
	unsigned int op_ret = 0;
		
     dcli_str2ulong((char *)argv[0],&mru);	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_GLOBAL_METER_MODE);
	dbus_error_init(&err);
	dbus_message_append_args(query, 					
							 DBUS_TYPE_UINT32,&mru,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
				vty_out(vty,"%% Product not support this function!\n");
			}
			else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
				vty_out(vty,"%% config global policer loose mode and MRU fail \n");   
			}			 
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(set_counter_cmd_func,
	  set_counter_cmd,
	  "set counter <1-15> inprofile RANGE outprofile RANGE",
	  "Set system configuration\n"
	  "Set counter for policer\n"
	  "Counter index 1-15\n"
	  "Set Inprofile init value\n"
	  "Inprofile init value range in 0-4G Byte\n"
	  "Set Outprofile init value\n"
	  "Outprofile init value range in 0-4G Byte\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int     coutIndex=0;
	unsigned int	 inRange=0,outRange=0;
	unsigned int     op_ret = 0;

	dcli_str2ulong((char *)argv[0],&coutIndex);

	dcli_str2ulong((char *)argv[1],&inRange);
	dcli_str2ulong((char *)argv[2],&outRange);

	if((1>inRange)||(0xFFFFFFFF < inRange)) {
		vty_out(vty,"%% Bad parameter!\n");
		return CMD_WARNING;
	}
	if((1>outRange)||(0xFFFFFFFF < outRange)) {
		vty_out(vty,"%% Bad parameter!\n");
		return CMD_WARNING;
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_SET_COUNTER);
	dbus_error_init(&err);
	dbus_message_append_args(query,						
							 DBUS_TYPE_UINT32,&coutIndex,
							 DBUS_TYPE_UINT32,&inRange,
							 DBUS_TYPE_UINT32,&outRange,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{		
			if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
				vty_out(vty,"%% Product not support this function!\n");
			}
			else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
				 vty_out(vty,"%% set counter fail \n");	  
			}			 
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(read_counter_cmd_func,
	  read_counter_cmd,
	  "read counter <1-15>",
	  "Read configuration\n"
	  "Read counter information\n"
	  "Counter index 1-15\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int	coutIndex=0;
	unsigned int	 op_ret = 0;
	unsigned long    inprofile=0,outprofile=0;

	dcli_str2ulong((char *)argv[0],&coutIndex);
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_GET_COUNTER);
	dbus_error_init(&err);
	dbus_message_append_args(query, 					
							 DBUS_TYPE_UINT32,&coutIndex,	
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&inprofile,
		DBUS_TYPE_UINT32,&outprofile,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_BAD_PTR==op_ret)
				vty_out(vty,"%% the counter not existed!\n");
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
				vty_out(vty,"%% Product not support this function!\n");
			else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
				 vty_out(vty,"%% read counter information fail \n");	  
			}	
			else{
				vty_out(vty,"=========================================\n");
				vty_out(vty,"%-30s: %d\n","counter",coutIndex);
				vty_out(vty,"%-30s: %lu %s\n","in profile",inprofile,"bytes");
				vty_out(vty,"%-30s: %lu %s\n","out of profile",outprofile,"bytes");
				vty_out(vty,"=========================================\n");
			}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(show_policer_cmd_func,
	show_policer_cmd,
	"show policer",
	SHOW_STR
	"Show policer\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;	
	unsigned long    cir=0,cbs=0, sharemode = 0;
	unsigned int     cmd=0,counterEnable=0,counterSetIndex=0,meterColorMode=0,modifyDscp=0,modifyUp=0;
	unsigned int     policerIndex=0,policerEnable=QOS_POLICER_DISABLE,qosProfile=0;
	unsigned int     ret=QOS_RETURN_CODE_ERROR,j=0,count=0;
	unsigned char    cmdstr[20]={0},coutE[10]={0},meterC[20]={0},modeC[10]={0},packsizeC[10]={0},mruC[10]={0};
	unsigned char    policerE[10]={0};
	unsigned int     packsize=3,mru=0,mode=0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_SHOW_POLICER);
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	 dbus_message_iter_init(reply,&iter);

	 dbus_message_iter_get_basic(&iter,&ret);
	
	if(QOS_RETURN_CODE_SUCCESS == ret){
		 dbus_message_iter_next(&iter);  
		 dbus_message_iter_get_basic(&iter,&count);
		 dbus_message_iter_next(&iter); 	 
		 dbus_message_iter_recurse(&iter,&iter_array);
				   
		 for (j = 0; j < count; j++) {
			 DBusMessageIter iter_struct;
			 dbus_message_iter_recurse(&iter_array,&iter_struct);			 
			 dbus_message_iter_get_basic(&iter_struct,&policerIndex);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&sharemode);
			 dbus_message_iter_next(&iter_struct);
			 dbus_message_iter_get_basic(&iter_struct,&cir);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&cbs);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&cmd);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&counterEnable);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&counterSetIndex);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&meterColorMode);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&policerEnable);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&qosProfile);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&mode);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&mru);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&packsize);
			 dbus_message_iter_next(&iter_array); 
			 		
			 switch(cmd){
			 	case 0:strcpy(cmdstr,"Keep");		 break;
			 	case 1:strcpy(cmdstr,"Drop");		 break;
			 	case 2:strcpy(cmdstr,"Remap"); 		 break;
			 		
				default: break; 	 
		 	 }
		 	switch(counterEnable){
			 	case 0:strcpy(coutE,"Disable");	     break;			 	
			 	case 1:strcpy(coutE,"Enable"); 		 break;			 	
				default: break; 	 
		 	 }
			switch(meterColorMode){
			 	case 0:strcpy(meterC,"Color Blind");  break;
			 	case 1:strcpy(meterC,"Color Aware");  break;	 	
				default: break; 	 
		 	 }			
			switch(policerEnable){
			 	case 0:strcpy(policerE,"Disable");	     break;			 	
			 	case 1:strcpy(policerE,"Enable"); 		 break;			 	
				default: break; 	 
		 	 }
			switch(packsize){
			 	case 0:			 	
			 	case 1:	strcpy(packsizeC,"layer3");	     break;	
				case 2: strcpy(packsizeC,"layer2");	     break;	
				case 3: strcpy(packsizeC,"layer1");	     break;	
				default: break; 	 
		 	 }
			switch(mode){
			 	case 0:	strcpy(modeC,"Strict");	     break;			 	
			 	case 1:	strcpy(modeC,"Loose");	     break;	
				default: break; 	 
		 	 }
			switch(mru){
			 	case 0:	strcpy(mruC,"1.5K");	 break;			 	
			 	case 1:	strcpy(mruC,"2K");	     break;		
				case 2: strcpy(mruC,"10K");	     break;		
				default: break; 	 
		 	 }
			vty_out(vty,"===============================================\n");
			vty_out(vty,"%-40s: %d\n","Policer ",policerIndex);
			vty_out(vty,"%-40s: %s\n","Policer State",policerE);
			vty_out(vty,"%-40s: %s\n","Policer Share-mode", sharemode ? "noshare" : "share");
			vty_out(vty,"%-40s: %ld\n","CIR",cir);
			vty_out(vty,"%-40s: %ld\n","CBS",cbs);
			/*vty_out(vty,"%-40s: %s\n","Meter mode",meterC);*/
			vty_out(vty,"%-40s: %s\n","Counter",coutE);
			if(counterEnable==1)
			   vty_out(vty,"%-40s: %d\n","Counter Index",counterSetIndex);
			vty_out(vty,"%-40s: %s\n","Out Profile Action",cmdstr);					
			vty_out(vty,"%-40s: %d\n","Remap QoSProfile",qosProfile);
			vty_out(vty,"%-40s: %s\n","Policer Mode",modeC);
			if(mode==0)
				vty_out(vty,"%-40s: %s\n","Policing Packet Size",packsizeC);
			else if(mode==1)
				vty_out(vty,"%-40s: %s\n","Policing MRU",mruC);
			vty_out(vty,"===============================================\n");
		 }
	}
	else if(QOS_RETURN_CODE_POLICER_NOT_EXISTED== ret) 
		vty_out(vty,"%% Error: No Policer exists.\n");
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(delete_policer_cmd_func,
	  delete_policer_cmd,
	  "delete policer <1-255>",
	  "Delete configuration\n"
	  "Delete policer\n"
	  "Policer range in 1-255\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int policerIndex = 0;
	unsigned int op_ret = 0;
	
	dcli_str2ulong((char*)argv[0],&policerIndex);	
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_DELETE_POLICER);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policerIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_POLICER_NOT_EXISTED==op_ret)
				vty_out(vty,"%% policer %d not existed!\n",policerIndex);
			else if(QOS_RETURN_CODE_POLICER_USE_IN_ACL==op_ret)
				vty_out(vty,"%% Since policer is in use,can not delete!\n");
			else if(QOS_RETURN_CODE_SUCCESS!=op_ret)
				vty_out(vty,"%% Delete policer fail!\n");		 
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(set_queue_scheduler_cmd_func,
	  set_queue_scheduler_cmd,
	  "queue-scheduler (wrr|sp|hybrid)",
	  "Queue scheduler configuration\n"
	  "Queue scheduler WRR algorithm\n"	 
	  "Queue scheduler SP algorithm\n"
	  "Queue scheduler SP+WRR algorithm\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int algFlag=0;
	unsigned int op_ret = 0;
	
	if(strncmp("wrr",argv[0],strlen(argv[0]))==0)
		algFlag = 1;
	else if(strncmp("sp",argv[0],strlen(argv[0]))==0)
		algFlag =0;
	else if(strncmp("hybrid",argv[0],strlen(argv[0]))==0)
		algFlag =2;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_QUEQUE_SCH);
	dbus_error_init(&err);
	dbus_message_append_args(query, 					
							 DBUS_TYPE_UINT32,&algFlag,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(op_ret==QOS_RETURN_CODE_SUCCESS){
				if((algFlag==1)||(algFlag==2)){
					if(vty->node==QOS_NODE_DISTRIBUTED){
						vty->node=SHQUEUE_NODE;
						vty->index=(void*)algFlag;
					}
				}
			}
			else if (ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT == op_ret) {
				vty_out(vty,"%% config queue scheduler fail, device not support \n");
			}
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
				vty_out(vty,"%% Product not support this function!\n");
			}
			else{
				vty_out(vty,"%% config queue scheduler fail\n");
			}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(wrr_queue_weight_cmd_func,
	  wrr_queue_weight_cmd,
	  "wrr (group1|group2) <0-7> (<1-255>|sp)",
	  "Wrr algorithm configuration\n"
	  "Wrr group1 configuration\n"
	  "Wrr group2 configuration\n"
	  "Traffic class range in 0-7\n"
	  "Assign weight for each traffic class\n"
	  "SP group1 is for hybrid\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int groupFlag=0,tc=0,weight=0,wrrF = 0;
	unsigned int op_ret = 0;

	
	if(strncmp("group1",argv[0],strlen(argv[0]))==0)
		groupFlag = 0;
	else if(strncmp("group2",argv[0],strlen(argv[0]))==0)
		groupFlag =1;
	else
		vty_out(vty,"%% unknown command!\n");

	if(SHQUEUE_NODE==vty->node)
		wrrF = (unsigned int)vty->index;
	
	 dcli_str2ulong((char *)argv[1],&tc);
	 if(strncmp("sp",argv[2],strlen(argv[2])) == 0) {
		weight = QOS_SCH_GROUP_IS_SP;
	}
	 else {
	 	dcli_str2ulong((char *)argv[2],&weight);
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_QUEQUE_WRR_GROUP);
	dbus_error_init(&err);
	dbus_message_append_args(query, 	
							 DBUS_TYPE_UINT32,&wrrF,
							 DBUS_TYPE_UINT32,&groupFlag,
							 DBUS_TYPE_UINT32,&tc,
							 DBUS_TYPE_UINT32,&weight,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
			if (ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT == op_ret) {
				vty_out(vty,"%% configqueue scheduler wrr group fail, device not support \n");
			}
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
				vty_out(vty,"%% Product not support this function!\n");
			}
			else if(op_ret != QOS_RETURN_CODE_SUCCESS){
				vty_out(vty,"%% config queue scheduler wrr group fail\n");
			}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(show_queue_scheduler_cmd_func,
	  show_queue_scheduler_cmd,
	  "show queue-scheduler",
	  SHOW_STR
	  "Queue scheduler\n"	 
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	 DBusMessageIter  iter;
	 DBusMessageIter  iter_array;
	unsigned int i = 0,j = 0,k = 0,m = 0,flag = 0;
	unsigned int groupflag=0,weight=0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_SHOW_QUEUE);
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	 dbus_message_iter_init(reply,&iter);
	 dbus_message_iter_get_basic(&iter,&flag);

	/*vty_out("flag %d\n",flag);*/
	vty_out(vty,"===========================================\n");
	vty_out(vty,"%-7s  %-20s  %-12s\n","QID","Scheduling-group","Weight");
	vty_out(vty,"=======  ====================	============  \n");	
	
	 if(flag==0){
		for(j=0;j<8;j++){
			vty_out(vty,"%-7d",j);
			vty_out(vty,"  %-20s","sp");
			vty_out(vty,"  %-12s\n","0");
		}
	 }
	 else if(flag==1){
		for(k=0;k<8;k++){
			vty_out(vty,"%-7d",k);
			vty_out(vty,"  %-20s","group1");
			vty_out(vty,"  %-12d\n",k+1);
		}
	 }
	 else if(flag==2){	 	
		 dbus_message_iter_next(&iter); 	 
		 dbus_message_iter_recurse(&iter,&iter_array);	
		 for (i = 0; i < 8; i++) {
			 DBusMessageIter iter_struct;
			 dbus_message_iter_recurse(&iter_array,&iter_struct);			 
			 dbus_message_iter_get_basic(&iter_struct,&groupflag);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&weight);
			 dbus_message_iter_next(&iter_array);

			 vty_out(vty,"%-7d",i);			

			 if(groupflag==0)
				vty_out(vty,"  %-20s","group1");
			 else if(groupflag==1)
				vty_out(vty,"  %-20s","group2");

			 vty_out(vty,"  %-12d\n",weight);
		 
	     }	 
	 }
	else if(flag==3){
		 dbus_message_iter_next(&iter); 	 
		 dbus_message_iter_recurse(&iter,&iter_array);	
		 for (m = 0; m < 8; m++) {
			 DBusMessageIter iter_struct;
			 dbus_message_iter_recurse(&iter_array,&iter_struct);			 
			 dbus_message_iter_get_basic(&iter_struct,&groupflag);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&weight);
			 dbus_message_iter_next(&iter_array);

			 vty_out(vty,"%-7d",m);			
			 
			 if(groupflag==0)
				vty_out(vty,"  %-20s","group1");
			 else if(groupflag==1)
				vty_out(vty,"  %-20s","group2");
			 if(weight == 0){
				  vty_out(vty,"  %-20s","sp");
				  vty_out(vty,"  %-12s\n","0");
			  }
			 else
				 vty_out(vty,"  %-12d\n",weight);		 
	     }	 
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(traffic_shape_port_cmd_func,
	  traffic_shape_port_cmd,
	  "traffic-shape MAXRATE (k|m) BURSTSIZE",
	  "Configure traffic shape on port\n"
	  "Max rate of traffic shape, range in 64k-1000M(k means 64kbps e.g.:2 k means 128kbps, m means 1mbps)\n"
	  "K means 64kbps\n"
	  "M means mbs\n"
	  "BurstSize, range in 1-4095, which unit is 4k bytes.(Eg:2 means burstsize 8K bytes)\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err ;
	unsigned int algFlag = 0,queueId = 0,burst = 0,kmstate=0;
	unsigned int op_ret = 0,eth_index = 0;
	unsigned int maxrate = 0;
	int ret = 0;
	unsigned int slot_no = 0;
	
	if(ETH_PORT_NODE == vty->node){
		eth_index = (unsigned int)vty->index;
	}
	
	ret = dcli_str2ulong((char *)argv[0],&maxrate);
	if(ret == QOS_RETURN_CODE_ERROR)
	{
		vty_out(vty,"%% Bad parameter MAXRATE %s!\n",argv[0]);
		return CMD_WARNING;
	}
	
	if(strncmp("k",argv[1],strlen(argv[1])) == 0) {
		kmstate = 0;
	}	
	else if(strncmp("m",argv[1],strlen(argv[1])) == 0) {
		kmstate = 1;
	}
	ret=dcli_str2ulong((char *)argv[2],&burst);
	if(ret==QOS_RETURN_CODE_ERROR)
	{
		vty_out(vty,"%% Bad parameter BURSTSIZE %s!\n",argv[2]);
		return CMD_WARNING;
	}

	if((maxrate > 1000000)||( maxrate < 1))
	{
		vty_out(vty,"%% Bad parameter!\n");
		return CMD_WARNING;

	}
	if((burst>4095)||(burst<1))
	{
		vty_out(vty,"%% Bad parameter!\n");
		return CMD_WARNING;
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,\
										 NPD_DBUS_ETHPORTS_OBJPATH,\
                                         NPD_DBUS_ETHPORTS_INTERFACE,\
										 NPD_DBUS_METHOD_TRAFFIC_SHAPE);
	dbus_error_init(&err);
	dbus_message_append_args(query, 
						     DBUS_TYPE_UINT32,&eth_index,
							 DBUS_TYPE_UINT32,&algFlag,
							 DBUS_TYPE_UINT32,&queueId,
							 DBUS_TYPE_UINT32,&maxrate,
							 DBUS_TYPE_UINT32,&kmstate,
							 DBUS_TYPE_UINT32,&burst,
							 DBUS_TYPE_INVALID);

	if(is_distributed == DISTRIBUTED_SYSTEM)
    {
		SLOT_PORT_ANALYSIS_SLOT(eth_index, slot_no);
    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
			return CMD_WARNING;
    	}
		else 
    	{
			dcli_dbus_connection_qos_port = dbus_connection_dcli[slot_no]->dcli_dbus_connection;
		}
    }
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos_port,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
		if(op_ret == ACL_RETURN_CODE_BAD_k){ 	
			vty_out(vty,"%% if k mode 1~4096 you can choice \n");
		}
		else if(op_ret == ACL_RETURN_CODE_BAD_M){		
			vty_out(vty,"%% if m mode 1~1000 you can choice \n");
		}
		else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
				vty_out(vty,"%% Product not support this function!\n");
		}
		else if(op_ret != QOS_RETURN_CODE_SUCCESS){ 	
			vty_out(vty,"%% config traffic shape for queue %d on port fail\n",queueId);
		}

	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(traffic_shape_queue_cmd_func,
	  traffic_shape_queue_cmd,
	  "traffic-shape queue <0-7> MAXRATE (k|m) BURSTSIZE",
	  "Configure traffic shape for queue on port\n"
	  "Specify queue <0-7> for traffic shape\n"
	  "Traffic queue 0 - 7 \n"
	  "Max rate of traffic shape, range in 64k-1000M(k means 64kbps Eg:2 k means 128kbps m means mbs)\n"
	  "K means 64kbps\n"
	  "M means mbs\n"
	  "BurstSize, range in 1-4095, which unit is 4k bytes.(Eg:2 means burstsize 8K bytes)\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err ;
	unsigned int algFlag = 1,queueId = 0,burst = 0;
	unsigned int op_ret = 0,eth_index = 0;
	unsigned int maxrate = 0, kmstate = 0;
	int ret = 0;
	unsigned int slot_no = 0;
	
	if(ETH_PORT_NODE == vty->node){
		eth_index = (unsigned int)vty->index;
	}
	
	 dcli_str2ulong((char *)argv[0],&queueId);	
	 
	 ret=dcli_str2ulong((char *)argv[1],&maxrate);
	 
	 if(ret==QOS_RETURN_CODE_ERROR)
	 {
		 vty_out(vty,"%% Bad parameter!\n");
		 return CMD_WARNING;
	 }
	 if(strncmp("k",argv[2],strlen(argv[2])) == 0) {
		kmstate = 0;
	 }	
	else if(strncmp("m",argv[2],strlen(argv[2])) == 0) {
		kmstate = 1;
	}	
	 ret=dcli_str2ulong((char *)argv[3],&burst);
	 if(ret==QOS_RETURN_CODE_ERROR)
	 {
		 vty_out(vty,"%% Bad parameter!\n");
		 return CMD_WARNING;
	 }
	 
	 if((maxrate>1000000)||(maxrate<1))
	 {
		vty_out(vty,"%% Bad parameter!\n");
		return CMD_WARNING;

	 }
	 if((burst>4095)||(burst<1))
	 {
		vty_out(vty,"%% Bad parameter!\n");
		return CMD_WARNING;
	 }
	 query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,\
										 NPD_DBUS_ETHPORTS_OBJPATH,\
                                         NPD_DBUS_ETHPORTS_INTERFACE,\
										 NPD_DBUS_METHOD_TRAFFIC_SHAPE);
	dbus_error_init(&err);
	dbus_message_append_args(query, 
						     DBUS_TYPE_UINT32,&eth_index,
							 DBUS_TYPE_UINT32,&algFlag,
							 DBUS_TYPE_UINT32,&queueId,
							 DBUS_TYPE_UINT32,&maxrate,
							 DBUS_TYPE_UINT32,&kmstate,
							 DBUS_TYPE_UINT32,&burst,
							 DBUS_TYPE_INVALID);
	if(is_distributed == DISTRIBUTED_SYSTEM)
    {
		SLOT_PORT_ANALYSIS_SLOT(eth_index, slot_no);
    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
			return CMD_WARNING;
    	}
 		else 
    	{
			dcli_dbus_connection_qos_port = dbus_connection_dcli[slot_no]->dcli_dbus_connection;
		}   
	}
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos_port,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(op_ret == ACL_RETURN_CODE_BAD_k){		
				vty_out(vty,"%% if k mode 1~4096 you can choice \n");
			}
			else if(op_ret == ACL_RETURN_CODE_BAD_M){		
				vty_out(vty,"%% if m mode 1~1000 you can choice \n");
			}
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
				vty_out(vty,"%% Product not support this function!\n");
			}
			else if(op_ret != QOS_RETURN_CODE_SUCCESS){		
				vty_out(vty,"%% config traffic shape for queue %d on port fail\n",queueId);
			}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(show_traffic_shape_cmd_func,
	  show_traffic_shape_cmd,
	  "show traffic-shape",
	  SHOW_STR
	  "Traffic shape\n"	 
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	unsigned int  i = 0,j = 0,ret = 0,eth_index = 0;
	unsigned int  burstsize=0,queueburst=0;
	unsigned long maxrate=0,queuerate=0;
	unsigned int  portEnable=0,queueEnable=0;
	unsigned char strcpp[10];
	unsigned int slot_no = 0;
	
	if(ETH_PORT_NODE==vty->node){
		eth_index = (unsigned int)vty->index;
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,\
										 NPD_DBUS_ETHPORTS_OBJPATH,\
                                         NPD_DBUS_ETHPORTS_INTERFACE,\
										 NPD_DBUS_METHOD_SHOW_TRAFFIC);
	dbus_error_init(&err);
	dbus_message_append_args(query, 
						     DBUS_TYPE_UINT32,&eth_index,
							 DBUS_TYPE_INVALID);
	if(is_distributed == DISTRIBUTED_SYSTEM)
    {
		SLOT_PORT_ANALYSIS_SLOT(eth_index, slot_no);
    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
			return CMD_WARNING;
    	}
		else 
    	{
			dcli_dbus_connection_qos_port = dbus_connection_dcli[slot_no]->dcli_dbus_connection;
		}
    }
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos_port,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
     dbus_message_iter_init(reply,&iter);

	 dbus_message_iter_get_basic(&iter,&ret);
	vty_out(vty,"=======================================================\n");
	if(QOS_RETURN_CODE_SUCCESS == ret){
		 dbus_message_iter_next(&iter);  
		 dbus_message_iter_get_basic(&iter,&portEnable);
		 dbus_message_iter_next(&iter); 
		 dbus_message_iter_get_basic(&iter,&burstsize);
		 dbus_message_iter_next(&iter); 
		 dbus_message_iter_get_basic(&iter,&maxrate);
		 dbus_message_iter_next(&iter); 
		 if(1==portEnable)
		 	vty_out(vty,"Port Shaping: Enable\n");
		 else if(0==portEnable)
		 	vty_out(vty,"Port Shaping: Disable\n");
		 vty_out(vty,"\n");
		 vty_out(vty,"%ld kbps   ",maxrate);
		 vty_out(vty,"%d*4K burst\n",burstsize);
		  vty_out(vty,"\n");
		/* vty_out(vty,"=======================================================\n"); */
		 vty_out(vty,"%-7s  %-20s  %-12s  %-12s\n","QID","status","max-rate(kbps)","burst-size(*4K byte)");
		 vty_out(vty,"=======  ====================	============ ============ \n");	
			
		 dbus_message_iter_recurse(&iter,&iter_array);
		 
		 for (j = 0; j < 8; j++) {
			 DBusMessageIter iter_struct;
			 dbus_message_iter_recurse(&iter_array,&iter_struct);	
			 dbus_message_iter_get_basic(&iter_struct,&queueEnable);		 
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&queueburst);		 
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&queuerate);		 
			 dbus_message_iter_next(&iter_array); 
			 switch(queueEnable){
				case 0: strcpy(strcpp,"disable"); break;
				case 1: strcpy(strcpp,"enable");  break;
				default : break;
			 }		
			 vty_out(vty,"%-7d  ",j);
			 vty_out(vty,"%-20s  ",strcpp);
			 vty_out(vty,"%-12ld  ",queuerate);
			 vty_out(vty,"%-12d\n",queueburst);
		 	}
		}
	else if(QOS_RETURN_CODE_ERROR==ret){
		/*vty_out(vty,"%% No traffic shape information on port!\n");*/
		 vty_out(vty,"Port Shaping: Disable\n");
		 vty_out(vty,"\n");
		 vty_out(vty,"0 kbps   ");
		 vty_out(vty,"0 burst\n");
		  vty_out(vty,"\n");
		/* vty_out(vty,"=======================================================\n"); */
		 vty_out(vty,"%-7s  %-20s  %-12s  %-12s\n","QID","status","max-rate(kbps)","burst-size(*4K byte)");
		 vty_out(vty,"=======  ====================	============ ============ \n");	
		for (j = 0; j < 8; j++) {			 
			 vty_out(vty,"%-7d  ",j);
			 vty_out(vty,"%-20s  ","disable");
			 vty_out(vty,"%-12s  ","0");
			 vty_out(vty,"%-12s\n","0");
		 	}	
	}
	else if(NPD_DBUS_ERROR_NO_SUCH_PORT==ret){
		vty_out(vty,"%% Port information error!\n");
	}		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(delete_traffic_shape_cmd_func,
	  delete_traffic_shape_cmd,
	   "delete traffic-shape port",
	   "Delete configuration\n"
	   "Delete traffic shape information \n"
	   "Traffic shape information on port\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int  op_ret = 0,eth_index = 0,algFlag=0,queueId=0;
	unsigned int slot_no = 0;
	
	if(ETH_PORT_NODE==vty->node){
		eth_index = (unsigned int)vty->index;
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,\
										 NPD_DBUS_ETHPORTS_OBJPATH,\
										 NPD_DBUS_ETHPORTS_INTERFACE,\
										 NPD_DBUS_METHOD_DELETE_TRAFFIC);
	dbus_error_init(&err);
	dbus_message_append_args(query, 
							 DBUS_TYPE_UINT32,&eth_index,
							 DBUS_TYPE_UINT32,&algFlag,
							 DBUS_TYPE_UINT32,&queueId,
							 DBUS_TYPE_INVALID);
	if(is_distributed == DISTRIBUTED_SYSTEM)
    {
		SLOT_PORT_ANALYSIS_SLOT(eth_index, slot_no);
    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
			return CMD_WARNING;
    	}
		else 
    	{
			dcli_dbus_connection_qos_port = dbus_connection_dcli[slot_no]->dcli_dbus_connection;
		}
    }
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos_port,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID))
		{				
				if(NPD_DBUS_ERROR_NO_SUCH_PORT==op_ret){
					vty_out(vty,"%% Port information error!\n");
				}
				else if(op_ret==QOS_RETURN_CODE_TRAFFIC_NO_INFO){		
					/*vty_out(vty,"%% port has no traffic shape information\n");*/
				}						 
				else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
					vty_out(vty,"%% Delete traffic shape information on port fail!\n");
				}
		} 
		else 
		{		
			if (dbus_error_is_set(&err)) 
			{
				dbus_error_free_for_dcli(&err);
			}
		}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(delete_traffic_shape_queue_cmd_func,
	  delete_traffic_shape_queue_cmd,
	   "delete traffic shape queue <0-7>",
	   "Delete configuration\n"
	   "Delete traffic shape information on port\n"
	   "Traffic shape on queue\n"
	   "Queue Id, range in 0-7\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int  op_ret = 0,eth_index = 0,algFlag=1,queueId=0;
	unsigned int slot_no = 0;
	
	if(ETH_PORT_NODE==vty->node){
		eth_index = (unsigned int)vty->index;
	}

	dcli_str2ulong((char *)argv[0],&queueId);
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,\
										 NPD_DBUS_ETHPORTS_OBJPATH,\
										 NPD_DBUS_ETHPORTS_INTERFACE,\
										 NPD_DBUS_METHOD_DELETE_TRAFFIC);
	dbus_error_init(&err);
	dbus_message_append_args(query, 
							 DBUS_TYPE_UINT32,&eth_index,
							 DBUS_TYPE_UINT32,&algFlag,
							 DBUS_TYPE_UINT32,&queueId,
							 DBUS_TYPE_INVALID);
	if(is_distributed == DISTRIBUTED_SYSTEM)
    {
		SLOT_PORT_ANALYSIS_SLOT(eth_index, slot_no);

    	if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection) 				
    	{
			vty_out(vty, "Connection to slot%d is not exist.\n", slot_no);
			return CMD_WARNING;
    	}
		else 
    	{
			dcli_dbus_connection_qos_port = dbus_connection_dcli[slot_no]->dcli_dbus_connection;
		}
    }
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos_port,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID))
		{				
				if(NPD_DBUS_ERROR_NO_SUCH_PORT==op_ret){
					vty_out(vty,"%% Port information error!\n");
				}
				else if(op_ret==QOS_RETURN_CODE_TRAFFIC_NO_INFO){		
					/*vty_out(vty,"%% Queue has no traffic shape information\n");*/
				}						 
				else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
					vty_out(vty,"%% Delete traffic shape information for queue %d on port fail!\n",queueId);
				}
		} 
		else 
		{		
			if (dbus_error_is_set(&err)) 
			{
				dbus_error_free_for_dcli(&err);
			}
		}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_policer_range_cmd_func,
	  config_policer_range_cmd,
	  "policer-range <1-255> <1-255> alias <1-255>",
	  "policer-range\n"
	  "Specify policer range in 1-255\n"
	  "Specify policer range in 1-255\n"
	  "Alias of policer\n"
	  "Specify policer range in 1-255\n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int policerIndex= 0, startPid = 0, endPid = 0;
	unsigned int op_ret = 0;
	
	dcli_str2ulong((char*)argv[0], &startPid);
	dcli_str2ulong((char*)argv[1], &endPid);
	dcli_str2ulong((char*)argv[2], &policerIndex);
			
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_CONFIG_POLICER_RANGE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &policerIndex,
							 DBUS_TYPE_UINT32, &startPid,
							 DBUS_TYPE_UINT32, &endPid,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
		if((QOS_RETURN_CODE_SUCCESS==op_ret)||(QOS_POLICER_DISABLE==op_ret)){
			 if(QOS_NODE_DISTRIBUTED==vty->node){
				  vty->index = (void*)policerIndex;						  
			  } 
		}
		else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
				vty_out(vty,"%% Product not support this function!\n");
		}
		else {		 
			 vty_out(vty,"%% config policer fail \n");	 
		}
		 
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(delete_policer_range_cmd_func,
	  delete_policer_range_cmd,
	  "delete policer-range <1-255> <1-255>",
	  "Delete configuration\n"
	  "Delete policer-range\n"
	  "Policer range in 1-255\n"
	  "Policer range in 1-255\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int policerIndex = 0, startPid = 0, endPid = 0;
	unsigned int op_ret = 0;
	
	dcli_str2ulong((char*)argv[0], &startPid);
	dcli_str2ulong((char*)argv[1], &endPid);	
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_DELETE_POLICER_RANGE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&startPid,
							 DBUS_TYPE_UINT32,&endPid,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_qos,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_POLICER_NOT_EXISTED==op_ret)
				vty_out(vty,"%% policer %d not existed!\n",policerIndex);
			else if(QOS_RETURN_CODE_POLICER_USE_IN_ACL==op_ret)
				vty_out(vty,"%% Since policer is in use,can not delete!\n");
			else if(QOS_RETURN_CODE_SUCCESS!=op_ret)
				vty_out(vty,"%% Delete policer fail!\n");		 
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
int dcli_qos_running_config(struct vty *vty)
{
	/* show qos mode*/
	dcli_qos_mode_show_running_config();
	
	/* show qos profile */
	dcli_qos_profile_show_running_config();
	
	/* show qos counter*/
	dcli_qos_counter_show_running_config();
	/*show remap table*/
	dcli_qos_remap_table_show_running_config();
	/*show policy map*/
	dcli_qos_policy_map_show_running_config();
	/*show queue-scheduler*/
	dcli_qos_queue_scheduler_show_running_config();
	#if 0
	/*show polier*/
	dcli_qos_policer_show_running_config();   /* this should be load before ACL */
    #endif
	return 0;

}

void dcli_qos_init()
{
  /* wangchong  init the dbus connect to the local board, config qos */
  dcli_dbus_connection_qos= dcli_dbus_connection;
  /* bind qos policy-map to eth-port */
  dcli_dbus_connection_qos_port	= dcli_dbus_connection;

  install_node (&qos_node_distributed, NULL,"QOS_NODE_DISTRIBUTED");
  install_default(QOS_NODE_DISTRIBUTED);

  //install_node (&qos_profile_node, dcli_qos_profile_show_running_config, "QOS_PROFILE_NODE");
  install_node (&qos_profile_node, NULL, "QOS_PROFILE_NODE");
  install_default(QOS_PROFILE_NODE);

  install_node (&qos_node, dcli_qos_running_config, "QOS_NODE");
  install_default(QOS_NODE);
  
  install_node (&policy_map_node, NULL, "POLICY_NODE");
  install_default(POLICY_NODE);
  install_node (&policer_node, NULL, "POLICER_NODE");
  install_default(POLICER_NODE);
  install_node(&sch_queue_node,NULL, "SHQUEUE_NODE");
  install_default(SHQUEUE_NODE);
  install_node(&out_profile_node,NULL, "OUT_PROFILE_NODE");
  install_default(OUT_PROFILE_NODE);
  
  install_element(CONFIG_NODE,&config_qos_on_board_cmd);    /* config node */

  install_element(QOS_NODE_DISTRIBUTED,          &config_qos_mode_cmd);
  install_element(QOS_NODE_DISTRIBUTED,          &show_qos_mode_cmd);
  install_element(QOS_NODE_DISTRIBUTED,          &config_qos_profile_cmd);
  install_element(QOS_PROFILE_NODE, &config_qos_profile_atrribute_cmd);
  install_element(QOS_NODE_DISTRIBUTED,          &delete_qos_profile_cmd);
  install_element(QOS_NODE_DISTRIBUTED,  	 &config_dscp2profile_cmd);
  install_element(QOS_NODE_DISTRIBUTED,  	 &delete_dscp2profile_cmd);
  install_element(QOS_NODE_DISTRIBUTED,  	 &config_dcsp2dscp_cmd);
  install_element(QOS_NODE_DISTRIBUTED,  	 &delete_dcsp2dscp_cmd);
  install_element(QOS_NODE_DISTRIBUTED,  	 &config_up2profile_cmd);
  install_element(QOS_NODE_DISTRIBUTED,  	 &delete_up2profile_cmd);
  install_element(QOS_NODE_DISTRIBUTED,  	 &creat_policy_map_cmd); 
  install_element(QOS_NODE_DISTRIBUTED,		 &config_policy_map_cmd);
  install_element(QOS_NODE_DISTRIBUTED,		 &delete_policy_map_cmd);
  install_element(POLICY_NODE,		 &allow_qos_mark_cmd);
  /*install_element(POLICY_NODE,		&set_default_up_cmd);
  //install_element(POLICY_NODE,		&config_port_default_profile_cmd);*/
  install_element(POLICY_NODE,		 &config_port_trust_mode_lay2_cmd);
  install_element(POLICY_NODE,		 &config_port_trust_mode_lay3_cmd);
  install_element(POLICY_NODE,		 &config_port_trust_mode_l2_l3_cmd);
  /*install_element(POLICY_NODE,		&config_port_trust_mode_untrust_cmd);*/
  install_element(ETH_PORT_NODE,	 &bind_policy_map_cmd);
  install_element(ETH_PORT_NODE,	 &unbind_policy_map_cmd);
  install_element(QOS_NODE_DISTRIBUTED,		 &config_acl_ingress_policy_qos_cmd);
  install_element(QOS_NODE_DISTRIBUTED,		 &config_acl_egress_policy_qos_cmd);
  install_element(QOS_NODE_DISTRIBUTED,		 &show_qos_profile_cmd);
  install_element(QOS_NODE_DISTRIBUTED,		 &show_policy_map_cmd);
  install_element(ETH_PORT_NODE,	 &show_port_qos_cmd);
  install_element(QOS_NODE_DISTRIBUTED,		 &show_global_remap_cmd);
  /*flow-control*/
  install_element(QOS_NODE_DISTRIBUTED,		 &set_g_strict_meter_mode_cmd);
  install_element(QOS_NODE_DISTRIBUTED,		 &set_g_loose_meter_mode_cmd);
  install_element(QOS_NODE_DISTRIBUTED,		 &config_policer_enable_cmd);
  install_element(QOS_NODE_DISTRIBUTED,		 &config_policer_cmd);
  install_element(POLICER_NODE,		 &set_cir_cbs_cmd);
  install_element(POLICER_NODE,		 &set_out_profile_cmd);
  install_element(POLICER_NODE,		 &set_policer_share_cmd);
  /*install_element(POLICER_NODE,		&set_color_mode_cmd);*/
  install_element(OUT_PROFILE_NODE,	 &set_out_profile_action_drop_keep_cmd);
  install_element(OUT_PROFILE_NODE,	 &set_out_profile_action_remap_cmd);
  install_element(POLICER_NODE,		 &enable_counter_cmd);
  install_element(QOS_NODE_DISTRIBUTED,		 &set_counter_cmd);
  install_element(QOS_NODE_DISTRIBUTED,		 &read_counter_cmd);
  install_element(QOS_NODE_DISTRIBUTED,		 &show_policer_cmd);
  install_element(QOS_NODE_DISTRIBUTED,		 &delete_policer_cmd);
  install_element(QOS_NODE_DISTRIBUTED,		 &set_queue_scheduler_cmd);
  install_element(SHQUEUE_NODE,	 &wrr_queue_weight_cmd);
  install_element(QOS_NODE_DISTRIBUTED,		 &show_queue_scheduler_cmd);
  install_element(ETH_PORT_NODE,	 &traffic_shape_port_cmd);
  install_element(ETH_PORT_NODE,	 &traffic_shape_queue_cmd);
  install_element(ETH_PORT_NODE,	 &show_traffic_shape_cmd);
  install_element(ETH_PORT_NODE,	 &delete_traffic_shape_cmd);
  install_element(ETH_PORT_NODE,	 &delete_traffic_shape_queue_cmd);
  install_element(QOS_NODE_DISTRIBUTED,		 &config_qos_base_acl_traffic);
  install_element(QOS_NODE_DISTRIBUTED,		 &show_qos_base_acl_traffic);
  install_element(QOS_NODE_DISTRIBUTED,  	 &delete_append_cmd);

  install_element(ENABLE_NODE,		 &show_qos_base_acl_traffic); 
  install_element(VIEW_NODE,		 &show_qos_base_acl_traffic); 
  install_element(ENABLE_NODE,		 &show_policer_cmd);
  install_element(VIEW_NODE,		 &show_policer_cmd);  
  install_element(ENABLE_NODE,		 &show_qos_profile_cmd);
  install_element(VIEW_NODE,		 &show_policy_map_cmd);
  install_element(ENABLE_NODE,		 &show_global_remap_cmd);
  install_element(VIEW_NODE,		 &show_qos_profile_cmd);
  install_element(ENABLE_NODE,		 &show_policy_map_cmd);
  install_element(VIEW_NODE,		 &show_global_remap_cmd);  
  install_element(ENABLE_NODE,       &show_qos_mode_cmd);
  install_element(VIEW_NODE,         &show_qos_mode_cmd);  
  install_element(QOS_NODE_DISTRIBUTED,         &config_policer_range_cmd);  
  install_element(QOS_NODE_DISTRIBUTED,         &delete_policer_range_cmd); 
   
}
#ifdef __cplusplus
}
#endif

