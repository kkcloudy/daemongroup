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
* capture.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
*
* DESCRIPTION:
*
*
*
*******************************************************************************/

/********************************************************************************
* ws_dcli_portconf.c
*
* MODIFY:
*		by qiaojie on 09/18/08
*
* CREATOR:
*		huangxiaohui
*
* DESCRIPTION:
*		web console function definition for ethernet port configuration.
*
* DATE:
*		06/10/2008	
*
*  FILE REVISION NUMBER:
*           $Revision: 1.2 $
*  CORRESPONDING: 
            dcli_eth_port.c  $version 1.116    2009-11-03  zhouyanmei
*******************************************************************************/


#ifdef __cplusplus
	extern "C"
	{
#endif
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <syslog.h>
#include <string.h>
#include <dbus/dbus.h>

#include "sysdef/npd_sysdef.h"
#include "dbus/npd/npd_dbus_def.h"

#include "util/npd_list.h"
#include "npd/nam/npd_amapi.h"

/*#include "../../dcli/src/lib/dcli_eth_port.h"*/
#include "nam/nam_eth_api.h"
#include "dbus/sem/sem_dbus_def.h"

#include "ws_init_dbus.h"
#include "ws_dcli_portconf.h"
#include "ws_dcli_vlan.h"
#include "ws_returncode.h"
#include "ws_dbus_list_interface.h"
#include "ws_dbus_def.h"

char *p_slot_status_str[MODULE_STAT_MAX] = {
	"NONE",
	"INITING",
	"RUNNING",
	"DISABLED"
};

char *p_eth_port_type_str[ETH_MAX] = {
	"ETH_INVALID",
	"ETH_FE_TX",
	"ETH_FE_FIBER",
	"ETH_GTX",
	"ETH_GE_FIBER",
	"ETH_GE_SFP",
	"ETH_XGE_XFP",
	"ETH_XGTX",
	"ETH_XGE_FIBER"
};

char *p_link_status_str[2] = {
	"DOWN",
	"UP"
};


char *p_doneOrnot_status_str[2] = {
	"NonDone",
	"DONE"
};


char *p_onoff_status_str[2] = {
	"OFF",
	"ON"
};

char *p_duplex_status_str[2] = {	
	"HALF",
	"FULL"
};

char *p_eth_speed_str[ETH_ATTR_SPEED_MAX] = {
	"10M",
	"100M",
	"1000M",
	"10G",
	"12G",
	"2.5G",
	"5G"
};

char *p_eth_media_str[3] = {
	"NONE",
	"FIBER",
	"COPPER"
};

int ccgi_get_port_index(char *str_port_name, unsigned int *pun_port_index)
{
	DBusMessage *query=NULL, *reply=NULL;
	DBusError err;

	int ret,op_ret;
	unsigned char slot_no = 0, port_no = 0;
	unsigned int local_slot_id = get_product_info(LOCAL_SLOTID_FILE);
	ret = parse_slotport_no((char*)str_port_name,&slot_no,&port_no);
	
	if (WS_FAIL == ret) {
		return WS_FAIL;
	}


    if (1 == distributed_flag)
	{
		query = dbus_message_new_method_call(
							SEM_DBUS_BUSNAME,\
							SEM_DBUS_OBJPATH,\
							SEM_DBUS_INTERFACE, 
							SEM_DBUS_CONFIG_ETHPORT
			                );
		
		dbus_error_init(&err);
	
		dbus_message_append_args(query,
								 DBUS_TYPE_BYTE,&slot_no,
								 DBUS_TYPE_BYTE,&port_no,
								 DBUS_TYPE_INVALID);
		
		DBusConnection *connection = NULL;
		get_slot_dbus_connection(slot_no, &connection, SNMPD_INSTANCE_MASTER_V3);
		if(connection) {
			
    		reply = dbus_connection_send_with_reply_and_block (connection,\
    				query, -1, &err);
    	}

		dbus_message_unref(query);

		if (NULL == reply) {
			if (dbus_error_is_set(&err)) {
				dbus_error_free(&err);
			}
			return WS_FAIL;
		}

		if (dbus_message_get_args ( reply, &err,
						DBUS_TYPE_UINT32, &op_ret,
						DBUS_TYPE_UINT32, pun_port_index,
						DBUS_TYPE_INVALID)) 
		{
			if (SEM_WRONG_PARAM == op_ret) 
			{
				return WS_FAIL;
			}
			else if (SEM_COMMAND_FAILED== op_ret) 
			{
				return WS_FAIL;
			}
			else  if(SEM_COMMAND_SUCCESS == op_ret)   
			{
				dbus_message_unref(reply);
				return WS_SUCCESS;
			}
			else if (SEM_COMMAND_NOT_SUPPORT == op_ret)
			{
				dbus_message_unref(reply);
			}
			else
			{
				return WS_FAIL;
			}				
		} 
		else 
		{
			if (dbus_error_is_set(&err)) 
			{
				dbus_error_free(&err);
			}
    		dbus_message_unref(reply);
    		return WS_FAIL;
		}

    	query = dbus_message_new_method_call(
    							NPD_DBUS_BUSNAME,		\
    							NPD_DBUS_ETHPORTS_OBJPATH ,	\
    							NPD_DBUS_ETHPORTS_INTERFACE ,		\
    							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT);
    	
    	dbus_error_init(&err);
		
    	dbus_message_append_args(query,
    							 DBUS_TYPE_BYTE,&slot_no,
    							 DBUS_TYPE_BYTE,&port_no,
    							 DBUS_TYPE_INVALID);
    	
		if(connection) {
			
    		reply = dbus_connection_send_with_reply_and_block (connection,\
    				query, -1, &err);
    	}
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		if (dbus_error_is_set(&err)) {
    			dbus_error_free(&err);
    		}
    		return WS_FAIL;
    	}

    	if (dbus_message_get_args ( reply, &err,
    					DBUS_TYPE_UINT32, &op_ret,
    					DBUS_TYPE_UINT32, pun_port_index,
    					DBUS_TYPE_INVALID)) 
    	{
			if (NPD_DBUS_ERROR_NO_SUCH_PORT == op_ret) 
			{
				ret = WS_NO_SUCH_PORT;
			}
			else if (NPD_DBUS_ERROR == op_ret ) 
			{
				ret = WS_EXEC_COMM_FAIL;
			}
			else  if((NPD_DBUS_SUCCESS) == op_ret)	 
			{
				ret = WS_SUCCESS;
			}
				else{
					ret = WS_FAIL;
				}
				dbus_message_unref(reply);
				return ret;
    	} 
    	else 
    	{
    		if (dbus_error_is_set(&err)) 
    		{
    			dbus_error_free(&err);
    		}
        	dbus_message_unref(reply);
        	return WS_FAIL;
    	}
    }




	if(8 == slot_no) {
		return WS_FAIL;
	}
	
	if (NPD_FAIL == ret) {
		return WS_FAIL;
	}

	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return WS_FAIL;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, pun_port_index,
					DBUS_TYPE_INVALID)) 
	{
		if (NPD_DBUS_ERROR_NO_SUCH_PORT == op_ret) 
		{
        	ret = WS_NO_SUCH_PORT;
		}
		else if (NPD_DBUS_ERROR == op_ret ) 
		{
			ret = WS_EXEC_COMM_FAIL;
		}
		else  if((NPD_DBUS_SUCCESS) == op_ret)   
		{
			ret = WS_SUCCESS;
		}
			else{
				ret = WS_FAIL;
			}
			dbus_message_unref(reply);
			return ret;
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	dbus_message_unref(reply);
	return WS_FAIL;
	}
}

int ccgi_port_admin_state(char *str_port_name, char *str_state)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	int ret = 0, op_ret = 0;
	unsigned int port_index = 0;
	unsigned int isEnable = 1;
	unsigned int type = ADMIN,  slot_no;
	int local_slot_id = get_product_info(LOCAL_SLOTID_FILE);
    
    if(NULL == str_port_name || NULL == str_state)
    {
    	return WS_FAIL;
    }
    
    if( (ret = ccgi_get_port_index(str_port_name, &port_index)) != WS_SUCCESS){
    	return ret;
    }
	if(strcmp(str_state , "ON") == 0){
		isEnable = 1;
	}
	else if (strcmp(str_state,"OFF")== 0){
		isEnable = 0;
	}
	else {
		return WS_FAIL;
	}


	SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);

    if (1 == distributed_flag)
	{
		query = dbus_message_new_method_call(
							SEM_DBUS_BUSNAME,\
							SEM_DBUS_OBJPATH,\
							SEM_DBUS_INTERFACE, \
							SEM_DBUS_CONFIG_ETHPORT_ATTR \
			                );
		dbus_error_init(&err);

		dbus_message_append_args(query,
		 		  		    DBUS_TYPE_UINT32,&type,
							DBUS_TYPE_UINT32,&port_index,
							DBUS_TYPE_UINT32,&isEnable,
							DBUS_TYPE_INVALID);
	
		DBusConnection *connection = NULL;
		get_slot_dbus_connection(slot_no, &connection, SNMPD_INSTANCE_MASTER_V3);
		if(connection) {
			
    		reply = dbus_connection_send_with_reply_and_block (connection,\
    				query, -1, &err);
    	}

		dbus_message_unref(query);

		
		if (NULL == reply) {
				if (dbus_error_is_set(&err)) {
					dbus_error_free(&err);
				}
				return WS_FAIL;
			}
		if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_UINT32, &port_index,
				DBUS_TYPE_INVALID)) 
		{
			if (SEM_COMMAND_FAILED == op_ret ) 
			{
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			else if(SEM_COMMAND_SUCCESS == op_ret)
			{
				dbus_message_unref(reply);
				return WS_SUCCESS;
			}
			else if (SEM_WRONG_PARAM == op_ret ) 
			{
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			else if (SEM_OPERATE_NOT_SUPPORT == op_ret) {
				
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			else if (SEM_COMMAND_NOT_SUPPORT == op_ret) {
				dbus_message_unref(reply);		
			}
			else
			{
				dbus_message_unref(reply);
				return WS_FAIL;
			}
		} 
		else 
		{
			if (dbus_error_is_set(&err)) 
			{
				dbus_error_free(&err);
			}
			dbus_message_unref(reply);
			return WS_FAIL;
		}
		
    	query = dbus_message_new_method_call(
    							NPD_DBUS_BUSNAME,		\
    							NPD_DBUS_ETHPORTS_OBJPATH ,	\
    							NPD_DBUS_ETHPORTS_INTERFACE ,		\
    							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
    							 DBUS_TYPE_UINT32,&type,
    							 DBUS_TYPE_UINT32,&port_index,
    							 DBUS_TYPE_UINT32,&isEnable,
    							 DBUS_TYPE_INVALID);
    	
		if(connection) {
			
    		reply = dbus_connection_send_with_reply_and_block (connection,\
    				query, -1, &err);
    	}
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		if (dbus_error_is_set(&err)) {
    			dbus_error_free(&err);
    		}
    		return WS_FAIL;
    	}

    	if (dbus_message_get_args ( reply, &err,
    					DBUS_TYPE_UINT32, &op_ret,
    					DBUS_TYPE_UINT32, &port_index,
    					DBUS_TYPE_INVALID)) 
    	{
    		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
			{
				ret = WS_FAIL; 
			}
    		else if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
			{
				ret = WS_SUCCESS;
			}
    		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret)
			{
				ret = WS_NOT_SUPPORT;
			}
			else
			{
				ret = WS_FAIL;
			}
    	} 
    	else 
    	{
			ret = WS_FAIL_GET_ARG;
    		if (dbus_error_is_set(&err)) 
    		{
    			dbus_error_free(&err);
    		}
    	}
    	
    	dbus_message_unref(reply);
    	return ret;
	
	}



	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&isEnable,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return WS_FAIL;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 
	{
		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
		{
			ret = WS_FAIL; 
		}
		else if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
		{
			ret = WS_SUCCESS;
		}
		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret)
        {
		    ret = WS_NOT_SUPPORT;
		}
		else
		{
		    ret = WS_FAIL;
		}
	} 
	else 
	{
		ret = WS_FAIL_GET_ARG;
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	
	dbus_message_unref(reply);
	return ret;
}
int ccgi_port_auto_state(char *str_port_name, char *str_state)
{

	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	int ret = 0, op_ret = 0;
	unsigned int port_index = 0;
	unsigned int isEnable = 1;
	unsigned int type = AUTONEGT;
    
    if(NULL == str_port_name || NULL == str_state)
    {

    	return WS_FAIL;
    }
    
    if( (ret = ccgi_get_port_index(str_port_name, &port_index)) != WS_SUCCESS){
    	return ret;
    }

   
	if(strcmp(str_state , "ON") == 0){
		isEnable = 1;
	}
	else if (strcmp(str_state,"OFF")== 0){
		isEnable = 0;
	}
	else {

		return WS_FAIL;
	}


    if (1 == distributed_flag)
	{
		unsigned int slot_no;
        unsigned int local_slot_id = get_product_info(LOCAL_SLOTID_FILE);
		
		SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);
		
		query = dbus_message_new_method_call(
							SEM_DBUS_BUSNAME,\
							SEM_DBUS_OBJPATH,\
							SEM_DBUS_INTERFACE, \
							SEM_DBUS_CONFIG_ETHPORT_ATTR \
			                );
		dbus_error_init(&err);

		dbus_message_append_args(query,
		 		  		    DBUS_TYPE_UINT32,&type,
							DBUS_TYPE_UINT32,&port_index,
							DBUS_TYPE_UINT32,&isEnable,
							DBUS_TYPE_INVALID);

		DBusConnection *connection = NULL;
		get_slot_dbus_connection(slot_no, &connection, SNMPD_INSTANCE_MASTER_V3);
		if(connection) {
			
    		reply = dbus_connection_send_with_reply_and_block (connection,\
    				query, -1, &err);
    	}
		
		dbus_message_unref(query);

		
		if (NULL == reply) {
			if (dbus_error_is_set(&err)) {
				dbus_error_free(&err);
			}
			return WS_FAIL;
		}
		if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_UINT32, &port_index,
				DBUS_TYPE_INVALID)) 
		{
			if (SEM_COMMAND_FAILED == op_ret ) 
			{
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			else if(SEM_COMMAND_SUCCESS == op_ret)
			{
				dbus_message_unref(reply);
				return WS_SUCCESS;
			}
			else if (SEM_WRONG_PARAM == op_ret ) 
			{
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			else if (SEM_OPERATE_NOT_SUPPORT == op_ret) {
				
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			else if (SEM_COMMAND_NOT_SUPPORT == op_ret) {
				dbus_message_unref(reply);		
			}
			else
			{
				dbus_message_unref(reply);
				return WS_FAIL;
			}
		} 
		else 
		{
			if (dbus_error_is_set(&err)) 
			{
				dbus_error_free(&err);
			}
			dbus_message_unref(reply);
			return WS_FAIL;
		}       

    	query = dbus_message_new_method_call(
    							NPD_DBUS_BUSNAME,		\
    							NPD_DBUS_ETHPORTS_OBJPATH ,	\
    							NPD_DBUS_ETHPORTS_INTERFACE ,		\
    							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
    							 DBUS_TYPE_UINT32,&type,
    							 DBUS_TYPE_UINT32,&port_index,
    							 DBUS_TYPE_UINT32,&isEnable,
    							 DBUS_TYPE_INVALID);

		if(connection) {
			
    		reply = dbus_connection_send_with_reply_and_block (connection,\
    				query, -1, &err);
    	}
    	
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		if (dbus_error_is_set(&err)) {
    			dbus_error_free(&err);
    		}
    		return WS_FAIL;
    	}

    	if (dbus_message_get_args ( reply, &err,
    					DBUS_TYPE_UINT32, &op_ret,
    					DBUS_TYPE_UINT32, &port_index,
    					DBUS_TYPE_INVALID)) 
    	{
			if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
			{
			
				ret = WS_FAIL; 
			}
			else if (ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
			{
			
				ret = WS_SUCCESS;
			}
			else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) 
			{
			
				ret = WS_NOT_SUPPORT;
			}
			else if (ETHPORT_RETURN_CODE_ETH_GE_SFP == op_ret) {
				
				ret = 9;
			}
			else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
			
				ret = 8; 
			}
			else if (ETHPORT_RETURN_CODE_ERR_OPERATE == op_ret) {
			
				ret = 7;
				}
			else
			{
			
				ret = WS_FAIL;
			}
    	} 
    	else 
    	{
			ret = WS_FAIL_GET_ARG;
    		if (dbus_error_is_set(&err)) 
    		{
    			dbus_error_free(&err);
    		}
    	}
    	
    	dbus_message_unref(reply);
		return ret;
    }



	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&isEnable,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}

		return WS_FAIL;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 
	{
		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
		{

			ret = WS_FAIL; 
		}
		else if (ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
		{
		
			ret = WS_SUCCESS;
		}
		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) 
        {
        
		    ret = WS_NOT_SUPPORT;
		}
		else if (ETHPORT_RETURN_CODE_ETH_GE_SFP == op_ret) {
			
			ret = 9;
		}
		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
		
			ret = 8; 
		}
        else if (ETHPORT_RETURN_CODE_ERR_OPERATE == op_ret) {
		
			ret = 7;
        	}
		else
		{
		
		    ret = WS_FAIL;
		}
	
	} 
	else 
	{
		ret = WS_FAIL_GET_ARG;
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	
	dbus_message_unref(reply);

	return ret;
}

/* modify version 1.116 */
int ccgi_port_speed_conf(char *str_port_name, char *str_speed)
{
	DBusMessage *query=NULL, *reply=NULL;
	DBusError err;

	int ret,op_ret;
	unsigned int port_index = 0;
	unsigned short param = 1000;
	unsigned int speed,type = SPEED, slot_no;
    
    if(NULL == str_port_name || NULL == str_speed)
    {
    	return WS_FAIL;
    }
    	
	ret = parse_short_parse(str_speed,&param);
	if(NPD_SUCCESS != ret){
		return WS_FAIL;
	}
	speed = param;

    if( (ret = ccgi_get_port_index(str_port_name, &port_index)) != WS_SUCCESS){
    	return ret;
    }

    if (1 == distributed_flag)
	{
		query = dbus_message_new_method_call(
							SEM_DBUS_BUSNAME,\
							SEM_DBUS_OBJPATH,\
							SEM_DBUS_INTERFACE, \
							SEM_DBUS_CONFIG_ETHPORT_ATTR \
			                );
		dbus_error_init(&err);

		dbus_message_append_args(query,
		 		  		    DBUS_TYPE_UINT32,&type,
							DBUS_TYPE_UINT32,&port_index,
							DBUS_TYPE_UINT32,&speed,
							DBUS_TYPE_INVALID);
		
		SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);

		DBusConnection *connection = NULL;
		get_slot_dbus_connection(slot_no, &connection, SNMPD_INSTANCE_MASTER_V3);
		if(connection) {
			
    		reply = dbus_connection_send_with_reply_and_block (connection,\
    				query, -1, &err);
    	}

		dbus_message_unref(query);
		
		if (NULL == reply) {
				if (dbus_error_is_set(&err)) {
					dbus_error_free(&err);
				}
				return WS_FAIL;
			}
		if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_UINT32, &port_index,
				DBUS_TYPE_INVALID)) 
		{
			if (SEM_COMMAND_FAILED == op_ret) 
			{
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			else if(SEM_COMMAND_SUCCESS == op_ret)
			{
				dbus_message_unref(reply);
				return WS_SUCCESS;
			}
			else if (SEM_WRONG_PARAM == op_ret) 
			{
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			else if (SEM_OPERATE_NOT_SUPPORT == op_ret) {
				
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			else if (SEM_COMMAND_NOT_SUPPORT == op_ret) {
				dbus_message_unref(reply);		
			}
			else
			{
				dbus_message_unref(reply);
				return WS_FAIL;
			}
		} 
		else 
		{
			if (dbus_error_is_set(&err)) 
			{
				dbus_error_free(&err);
			}
			dbus_message_unref(reply);
			return WS_FAIL;
		}
		
    	query = dbus_message_new_method_call(
    							NPD_DBUS_BUSNAME,		\
    							NPD_DBUS_ETHPORTS_OBJPATH ,	\
    							NPD_DBUS_ETHPORTS_INTERFACE ,		\
    							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
    						   DBUS_TYPE_UINT32,&type,
    						   DBUS_TYPE_UINT32,&port_index,
    						   DBUS_TYPE_UINT32,&speed,
    						   DBUS_TYPE_INVALID);
    	
		if(connection) {
			
    		reply = dbus_connection_send_with_reply_and_block (connection,\
    				query, -1, &err);
    	}

    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		if (dbus_error_is_set(&err)) {
    			dbus_error_free(&err);
    		}
    		return WS_FAIL;
    	}

    	if (dbus_message_get_args ( reply, &err,
    					DBUS_TYPE_UINT32, &op_ret,
    					DBUS_TYPE_UINT32, &port_index,
    					DBUS_TYPE_INVALID)) 
    	{
    		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
    		{
    			ret = ETHPORT_RETURN_CODE_ERR_GENERAL; 
    		}
    		else if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret){
    			ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT; 
    		}		
    		else if (ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
    		{
    			ret = ETHPORT_RETURN_CODE_ERR_NONE; 
    		}
    		else if (ETHPORT_RETURN_CODE_SPEED_NODE == op_ret) {
    			ret = ETHPORT_RETURN_CODE_SPEED_NODE; 
    		}
    		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
    			ret = ETHPORT_RETURN_CODE_UNSUPPORT; 
    		}
    		else if (ETHPORT_RETURN_CODE_DUPLEX_MODE == op_ret) {
    			ret = ETHPORT_RETURN_CODE_DUPLEX_MODE; 
    		}		
    		else if (VLAN_RETURN_CODE_PORT_TRUNK_MBR == op_ret) {
    			ret = VLAN_RETURN_CODE_PORT_TRUNK_MBR; 
    		}		
    		else if (ETHPORT_RETURN_CODE_ETH_GE_SFP == op_ret) {
    			ret = ETHPORT_RETURN_CODE_ETH_GE_SFP; 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
    			ret = ETHPORT_RETURN_CODE_ERR_HW; 
    		}		
    		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
    			ret = ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE; 
    		}
    	} 
    	else 
    	{
    		if (dbus_error_is_set(&err)) 
    		{
    			dbus_error_free(&err);
    		}
			ret = WS_FAIL;
    	}
    	
    	dbus_message_unref(reply);
    	return ret;
            
	}
    
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&speed,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return WS_FAIL;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 
	{
		if (NPD_DBUS_ERROR == op_ret ) 
		{
			ret = WS_EXEC_COMM_FAIL;
		}
		else  if(NPD_DBUS_SUCCESS == op_ret)
		{
			ret = WS_SUCCESS;
		}
		/*
		else if(NPD_DBUS_ERROR_UNSUPPORT == op_ret)
		{
		    ret = WS_NOT_SUPPORT;
		}
		else
		{
		    ret = WS_FAIL;
		}
		*/
		else if(ETHPORT_RETURN_CODE_SPEED_NODE == op_ret) {
			//vty_out(vty,"%% Speed is configed only auto negotiation speed is disable.\n");
			ret = WS_NOT_SUPPORT;
		}
		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
			//vty_out(vty,"%% The eth-port wasn't supported media set\n");
			ret = WS_NOT_SUPPORT;
		}
		else if (ETHPORT_RETURN_CODE_DUPLEX_MODE == op_ret) {
			//vty_out(vty,"%% The eth-port wasn't supported config 1000 when duplex is half mode \n");
			ret = WS_FAIL;
		}
		else if (VLAN_RETURN_CODE_PORT_TRUNK_MBR == op_ret) {
			//vty_out(vty,"%% The eth-port wasn't supported config when the port belonged to a active trunk \n");
            ret = WS_FAIL;
		}
		else if (ETHPORT_RETURN_CODE_ETH_GE_SFP == op_ret) {
			//vty_out(vty,"%% Fiber port not support this operation\n"); 
			ret = WS_NOT_SUPPORT;
		}

	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		ret = WS_FAIL;
	}
	
	dbus_message_unref(reply);
	return ret;
}

/*2009-04-22 version 1.88*/
int ccgi_port_mode_conf( char *str_port_name, char *str_mode)  	/*返回0表示失败，返回1表示成功，返回-1表示no such port，返回-2表示it is already this mode*/
                                                                   	/*返回-3表示unsupport this command，返回-4表示execute command failed 返回-5表示与vlan接口冲突*/
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int 	op_ret,mode,retu=0;
	unsigned char   slot_no,port_no;
	op_ret = parse_slotport_no((char *)str_port_name, &slot_no, &port_no);

	if (NPD_FAIL == op_ret) {
		//vty_out(vty,"input bad slot/port!\n");
		//return CMD_SUCCESS;
		return 0;
	}

	
	if(0 == strncmp(str_mode,"switch",strlen(str_mode)))
		mode = ETH_PORT_FUNC_BRIDGE;
	else if(0 == strncmp(str_mode,"route",strlen(str_mode)))
		mode =ETH_PORT_FUNC_IPV4;
	else if(0 == strncmp(str_mode,"promiscuous",strlen(str_mode)))
		mode = ETH_PORT_FUNC_MODE_PROMISCUOUS;
	else {
		//vty_out(vty,"input bad mode!\n");
		//return CMD_WARNING;
		return 0;
	  }
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_MODE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
								DBUS_TYPE_BYTE,&slot_no,
								DBUS_TYPE_BYTE,&port_no,
							 	DBUS_TYPE_UINT32,&mode,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,40000, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if(INTERFACE_RETURN_CODE_SUCCESS == op_ret)
			{
				dbus_message_unref(reply);
				//sleep(1);
				retu=1;
			}
			else if(INTERFACE_RETURN_CODE_NO_SUCH_PORT == op_ret)
			{
			  //vty_out(vty,"%% NO SUCH PORT!\n");
				retu=-1;
			}

///////////////////////////需要重写做处理  v1.85
			else if(INTERFACE_RETURN_CODE_ALREADY_THIS_MODE == op_ret){
                if(ETH_PORT_FUNC_BRIDGE == mode){
                    //vty_out(vty,"%% Port interface not exists!\n");
                    retu=-6;
				}
				else if(ETH_PORT_FUNC_IPV4 == mode){
					//vty_out(vty,"%% Advanced-routing already disabled!\n");
					retu=-7;
				}
				else if(ETH_PORT_FUNC_MODE_PROMISCUOUS == mode){
					//vty_out(vty,"%% Advanced-routing already enabled!\n");
					retu=-8;
				}
			}
			else
			{
				//vty_out(vty,"%% Execute command failed\n");
				retu = 0;
			}
			
	} else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu=0;
	}
	dbus_message_unref(reply);
	return retu;
}

/*modify version 1.86*/
int ccgi_port_auto_neg(char *str_port_name, char *str_neg_state)
{
	DBusMessage *query=NULL, *reply=NULL;
	DBusError err;

	int ret=0,op_ret=0;
	unsigned int port_index = 0;
	unsigned int isEnable = 1;
	unsigned int type = AUTONEGT;

    if(NULL == str_port_name || NULL == str_neg_state)
    {
    	return WS_FAIL;
    }
	
	if(strcmp(str_neg_state , "enable") == 0){
		isEnable = 1;
	}
	else if (strcmp(str_neg_state,"disable")== 0){
		isEnable = 0;
	}
	else {
		return WS_FAIL;
	}
	
    if( (ret = ccgi_get_port_index(str_port_name, &port_index)) != WS_SUCCESS){
    	return ret;
    }
	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&isEnable,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return WS_FAIL;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 
	{
		if (NPD_DBUS_ERROR == op_ret ) 
		{
			ret = WS_EXEC_COMM_FAIL; 
		}
		else  if((NPD_DBUS_SUCCESS) == op_ret)
		{
			ret = WS_SUCCESS;
		}
		/*
		else
		{
		    ret = WS_FAIL;
		}
        */
		else if (NPD_DBUS_ERROR_UNSUPPORT == op_ret) {
			//vty_out(vty,"%% The eth-port wasn't supported media set\n");
			ret= WS_NOT_SUPPORT;
		}
		else if(NPD_DBUS_ERROR_AUNE_NONE == op_ret) {
			//vty_out(vty,"%% auto negotiation speed is configed only speed isn't 1000M.\n");
			ret = WS_FAIL;
		}
		else if (NPD_DBUS_ETH_GE_SFP == op_ret) {
			//vty_out(vty,"%% Fiber port not support this operation\n"); 
			ret= WS_NOT_SUPPORT;
		}

		
		else if (NPD_DBUS_ERROR_OPERATE == op_ret) {
			//vty_out(vty,"%% Admin disable not support this operation\n"); 
			ret=WS_ADMIN_DIS;
		}

		else {
			//vty_out(vty,"%% please config auto negotiation by configure guide.\n");
			ret= WS_FAIL;
		}

	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		ret = WS_FAIL;
	}
	
	dbus_message_unref(reply);
	return ret;
}

int ccgi_port_auto_speed(char *str_port_name, char *str_autospe_state)
	{
		DBusMessage *query = NULL, *reply = NULL;
		DBusError err;
	
		unsigned int port_index = 0;
		int ret = 0, op_ret = 0;
		unsigned int isEnable = 1;
		unsigned int type = AUTONEGTS;
	
		 if(NULL == str_port_name || NULL == str_autospe_state)
   		 {
    		return WS_FAIL;
   		 }
		
			if(strcmp(str_autospe_state , "ON") == 0){
			isEnable = 1;
		}
		else if (strcmp(str_autospe_state,"OFF")== 0){
			isEnable = 0;
		}
		else {
			return WS_FAIL;
		}
		
	    if( (ret = ccgi_get_port_index(str_port_name, &port_index)) != WS_SUCCESS){
	    	return ret;
	    }


		if (1 == distributed_flag)
		{
	    	unsigned int slot_no;
	        unsigned int local_slot_id = get_product_info(LOCAL_SLOTID_FILE);	

			SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);
			
			query = dbus_message_new_method_call(
								SEM_DBUS_BUSNAME,\
								SEM_DBUS_OBJPATH,\
								SEM_DBUS_INTERFACE, \
								SEM_DBUS_CONFIG_ETHPORT_ATTR \
				                );
			dbus_error_init(&err);

			dbus_message_append_args(query,
			 		  		    DBUS_TYPE_UINT32,&type,
								DBUS_TYPE_UINT32,&port_index,
								DBUS_TYPE_UINT32,&isEnable,
								DBUS_TYPE_INVALID);

			DBusConnection *connection = NULL;
			get_slot_dbus_connection(slot_no, &connection, SNMPD_INSTANCE_MASTER_V3);
			if(connection) {
				
				reply = dbus_connection_send_with_reply_and_block (connection,\
						query, -1, &err);
			}

			dbus_message_unref(query);

			
			if (NULL == reply) {
					if (dbus_error_is_set(&err)) {
						dbus_error_free(&err);
					}
					return WS_FAIL;
				}
			if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 
			{
				if (SEM_COMMAND_FAILED == op_ret ) 
				{
					dbus_message_unref(reply);
					return WS_FAIL;
				}
				else if(SEM_COMMAND_SUCCESS == op_ret)
				{
					dbus_message_unref(reply);
					return WS_SUCCESS;
				}
				else if (SEM_WRONG_PARAM == op_ret ) 
				{
					dbus_message_unref(reply);
					return WS_FAIL;
				}
				else if (SEM_OPERATE_NOT_SUPPORT == op_ret) {
					
					dbus_message_unref(reply);
					return WS_FAIL;
				}
				else if (SEM_COMMAND_NOT_SUPPORT == op_ret) {
					dbus_message_unref(reply);		
				}
				else
				{
					dbus_message_unref(reply);
					return WS_FAIL;
				}
			} 
			else 
			{
				if (dbus_error_is_set(&err)) 
				{
					dbus_error_free(&err);
				}
				dbus_message_unref(reply);
				return WS_FAIL;
			}
	    	query = dbus_message_new_method_call(
	    							NPD_DBUS_BUSNAME,		\
	    							NPD_DBUS_ETHPORTS_OBJPATH ,	\
	    							NPD_DBUS_ETHPORTS_INTERFACE ,		\
	    							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
	    	
	    	dbus_error_init(&err);

	    	dbus_message_append_args(query,
	    							DBUS_TYPE_UINT32,&type,
	    							 DBUS_TYPE_UINT32,&port_index,
	    							 DBUS_TYPE_UINT32,&isEnable,
	    							 DBUS_TYPE_INVALID);
	    	
			if(connection) {
				
				reply = dbus_connection_send_with_reply_and_block (connection,\
						query, -1, &err);
			}
	    	
	    	dbus_message_unref(query);
	    	
	    	if (NULL == reply) {
	    		if (dbus_error_is_set(&err)) {
	    			dbus_error_free(&err);
	    		}
	    		return WS_FAIL;
	    	}

	    	if (dbus_message_get_args ( reply, &err,
	    					DBUS_TYPE_UINT32, &op_ret,
	    					DBUS_TYPE_UINT32, &port_index,
	    					DBUS_TYPE_INVALID)) 
	    	{
	    		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
	    		{
					ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
	    		}
	    		else  if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
	    		{
					ret = ETHPORT_RETURN_CODE_ERR_NONE;
	    		}
	    		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
					ret = ETHPORT_RETURN_CODE_UNSUPPORT;
	    		}
	    		else if (ETHPORT_RETURN_CODE_NOT_SUPPORT == op_ret) {
					ret = ETHPORT_RETURN_CODE_NOT_SUPPORT;
	    		}		
	    		else if (ETHPORT_RETURN_CODE_ETH_GE_SFP == op_ret) {
					ret = ETHPORT_RETURN_CODE_ETH_GE_SFP;
	    		}
	    		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
					ret = ETHPORT_RETURN_CODE_ERR_HW;
	    		}
	            else if (ETHPORT_RETURN_CODE_ERR_OPERATE == op_ret) {
					ret = ETHPORT_RETURN_CODE_ERR_OPERATE;
	    		}
	    		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
					ret = ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE;
	    		}
	    	} 
	    	else 
	    	{
	    		if (dbus_error_is_set(&err)) 
	    		{
	    			dbus_error_free(&err);
					
					ret = WS_FAIL;
	    		}
	    	}
	    	
	    	dbus_message_unref(reply);
	    	return ret;
    	
		}



		
		
		query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH , \
								NPD_DBUS_ETHPORTS_INTERFACE ,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
		
		dbus_error_init(&err);
	
		dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&type,
								 DBUS_TYPE_UINT32,&port_index,
								 DBUS_TYPE_UINT32,&isEnable,
								 DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		
		if (NULL == reply) {

			if (dbus_error_is_set(&err)) {

				dbus_error_free(&err);
			}
			return WS_FAIL;
		}
	
		if (dbus_message_get_args ( reply, &err,
						DBUS_TYPE_UINT32, &op_ret,
						DBUS_TYPE_UINT32, &port_index,
						DBUS_TYPE_INVALID)) 
		{
			if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
			{
				ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
			}
			else  if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
			{
				ret = ETHPORT_RETURN_CODE_ERR_NONE;
			}
			else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
				ret = ETHPORT_RETURN_CODE_UNSUPPORT;
			}
			else if (ETHPORT_RETURN_CODE_NOT_SUPPORT == op_ret) {
				ret = ETHPORT_RETURN_CODE_NOT_SUPPORT;
			}		
			else if (ETHPORT_RETURN_CODE_ETH_GE_SFP == op_ret) {
				ret = ETHPORT_RETURN_CODE_ETH_GE_SFP;
			}
			else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
				ret = ETHPORT_RETURN_CODE_ERR_HW ;
			}
			else if (ETHPORT_RETURN_CODE_ERR_OPERATE == op_ret) {
				ret  = ETHPORT_RETURN_CODE_ERR_OPERATE;
			}
		} 
		else 
		{
			
			if (dbus_error_is_set(&err)) 
			{
				dbus_error_free(&err);
			}
			ret = WS_FAIL;
		}
		
		dbus_message_unref(reply);
		return ret;
	}

int ccgi_port_auto_dup(char *str_port_name, char *str_autodup_state)
{
	DBusMessage *query=NULL, *reply=NULL;
	DBusError err;

	int ret,op_ret;
	unsigned int port_index = 0;
	unsigned int isEnable = 1;
	unsigned int type = AUTONEGTD;

    if(NULL == str_port_name || NULL == str_autodup_state)
    {
    	return WS_FAIL;
    }
	
	if(strcmp(str_autodup_state , "ON") == 0){
		isEnable = 1;
	}
	else if (strcmp(str_autodup_state,"OFF")== 0){
		isEnable = 0;
	}
	else {
		return WS_FAIL;
	}
	
    if( (ret = ccgi_get_port_index(str_port_name, &port_index)) != WS_SUCCESS){
    	return ret;
    }


    if (1 == distributed_flag)
	{
    	unsigned int slot_no;
        unsigned int local_slot_id = get_product_info(LOCAL_SLOTID_FILE);
		SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);
		 
		query = dbus_message_new_method_call(
							SEM_DBUS_BUSNAME,\
							SEM_DBUS_OBJPATH,\
							SEM_DBUS_INTERFACE, \
							SEM_DBUS_CONFIG_ETHPORT_ATTR \
			                );
		dbus_error_init(&err);

		dbus_message_append_args(query,
		 		  		    DBUS_TYPE_UINT32,&type,
							DBUS_TYPE_UINT32,&port_index,
							DBUS_TYPE_UINT32,&isEnable,
							DBUS_TYPE_INVALID);

		DBusConnection *connection = NULL;
		get_slot_dbus_connection(slot_no, &connection, SNMPD_INSTANCE_MASTER_V3);
		if(connection) {
			
			reply = dbus_connection_send_with_reply_and_block (connection,\
					query, -1, &err);
		}
		
		dbus_message_unref(query);

		
		if (NULL == reply) {
				if (dbus_error_is_set(&err)) {
					dbus_error_free(&err);
				}
				return WS_FAIL;
			}
		if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_UINT32, &port_index,
				DBUS_TYPE_INVALID)) 
		{
			if (SEM_COMMAND_FAILED == op_ret ) 
			{
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			else if(SEM_COMMAND_SUCCESS == op_ret)
			{
				dbus_message_unref(reply);
				return WS_SUCCESS;
			}
			else if (SEM_WRONG_PARAM == op_ret ) 
			{
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			else if (SEM_OPERATE_NOT_SUPPORT == op_ret) {
				
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			else if (SEM_COMMAND_NOT_SUPPORT == op_ret) {
				dbus_message_unref(reply);		
			}
			else
			{
				dbus_message_unref(reply);
				return WS_FAIL;
			}
		} 
		else 
		{
			if (dbus_error_is_set(&err)) 
			{
				dbus_error_free(&err);
			}
			dbus_message_unref(reply);
			return WS_FAIL;
		}

    	query = dbus_message_new_method_call(
    							NPD_DBUS_BUSNAME,		\
    							NPD_DBUS_ETHPORTS_OBJPATH ,	\
    							NPD_DBUS_ETHPORTS_INTERFACE ,	\
    							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
    							DBUS_TYPE_UINT32,&type,
    							 DBUS_TYPE_UINT32,&port_index,
    							 DBUS_TYPE_UINT32,&isEnable,
    							 DBUS_TYPE_INVALID);

		if(connection) {
			
			reply = dbus_connection_send_with_reply_and_block (connection,\
					query, -1, &err);
		}
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		if (dbus_error_is_set(&err)) {
    			dbus_error_free(&err);
    		}
    		return WS_FAIL;
    	}

    	if (dbus_message_get_args ( reply, &err,
    					DBUS_TYPE_UINT32, &op_ret,
    					DBUS_TYPE_UINT32, &port_index,
    					DBUS_TYPE_INVALID)) 
    	{
    		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
    		{
				ret = ETHPORT_RETURN_CODE_ERR_GENERAL; 
    		}
    		else  if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
    		{
				ret = ETHPORT_RETURN_CODE_ERR_NONE; 
    		}
    		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
				ret = ETHPORT_RETURN_CODE_UNSUPPORT; 
    		}
    		else if (ETHPORT_RETURN_CODE_NOT_SUPPORT == op_ret) {
				ret = ETHPORT_RETURN_CODE_NOT_SUPPORT; 
    		}		
    		else if (ETHPORT_RETURN_CODE_ETH_GE_SFP == op_ret) {
				ret = ETHPORT_RETURN_CODE_ETH_GE_SFP; 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
				ret = ETHPORT_RETURN_CODE_ERR_HW; 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_OPERATE == op_ret) {
				ret = ETHPORT_RETURN_CODE_ERR_OPERATE; 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
				ret = ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE; 
    		}

    	} 
    	else 
    	{
    		if (dbus_error_is_set(&err)) 
    		{
    			dbus_error_free(&err);
    		}
			ret = WS_FAIL;
    	}
    	
    	dbus_message_unref(reply);
    	return ret;
    	
	}


	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&isEnable,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return WS_FAIL;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 
	 
	///////////////////////
          	{
		if (NPD_DBUS_ERROR == op_ret ) 
		{
			//vty_out(vty,"execute command failed.\n"); 
			ret = WS_EXEC_COMM_FAIL; 
		}
		else  if((NPD_DBUS_SUCCESS) == op_ret)
		{
			//DCLI_DEBUG(("port %d config auto-negotiation duplex succeed\n",port_index));
			ret = WS_SUCCESS;
		}
		else if(NPD_DBUS_ERROR_AUNE_NONE == op_ret) {
			//vty_out(vty,"%% auto negotiation duplex is configed only speed isn't 1000M.\n");
			 ret = WS_FAIL;
		}
		else if (NPD_DBUS_ERROR_UNSUPPORT == op_ret) {
			//vty_out(vty,"%% The eth-port wasn't supported media set\n");
			ret = WS_NOT_SUPPORT;
		}
		else if (NPD_DBUS_ERROR_NOT_SUPPORT == op_ret) {
			//vty_out(vty,"%% The eth-port wasn't supported auto-duplex config\n");
			 ret = WS_FAIL;
		}		
		else if (NPD_DBUS_ETH_GE_SFP == op_ret) {
			//vty_out(vty,"%% Fiber port not support this operation\n"); 
			 ret = WS_FAIL;
		}
		else if (NPD_DBUS_ERROR_OPERATE == op_ret) {
			//vty_out(vty,"%% Admin disable not support this operation\n"); 
			return WS_ADMIN_DIS;
		}


	} 

	//////////////////////
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		ret = WS_FAIL;
	}
	
	dbus_message_unref(reply);
	return ret;
}

//modify v1.116
int ccgi_port_auto_flowctl(char *str_port_name, char *str_autoflow_state)
{
	DBusMessage *query=NULL, *reply=NULL;
	DBusError err;

	int ret,op_ret;
	unsigned int port_index = 0;
	unsigned int isEnable = 1;
	unsigned int type = AUTONEGTF;

    if(NULL == str_port_name || NULL == str_autoflow_state)
    {
    	return WS_FAIL;
    }
	
	if(strcmp(str_autoflow_state, "ON") == 0){
		isEnable = 1;
	}
	else if (strcmp(str_autoflow_state,"OFF")== 0){
		isEnable = 0;
	}
	else {
		return WS_FAIL;
	}
	
    if( (ret = ccgi_get_port_index(str_port_name, &port_index)) != WS_SUCCESS){
    	return ret;
    }



    if (1 == distributed_flag)
	{
    	unsigned int slot_no;
        unsigned int local_slot_id = get_product_info(LOCAL_SLOTID_FILE);
		SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);
		
		query = dbus_message_new_method_call(
							SEM_DBUS_BUSNAME,\
							SEM_DBUS_OBJPATH,\
							SEM_DBUS_INTERFACE, \
							SEM_DBUS_CONFIG_ETHPORT_ATTR \
			                );
		dbus_error_init(&err);

		dbus_message_append_args(query,
		 		  		    DBUS_TYPE_UINT32,&type,
							DBUS_TYPE_UINT32,&port_index,
							DBUS_TYPE_UINT32,&isEnable,
							DBUS_TYPE_INVALID);

		DBusConnection *connection = NULL;
		get_slot_dbus_connection(slot_no, &connection, SNMPD_INSTANCE_MASTER_V3);
		if(connection) {
			
			reply = dbus_connection_send_with_reply_and_block (connection,\
					query, -1, &err);
		}

		dbus_message_unref(query);

		
		if (NULL == reply) {
				if (dbus_error_is_set(&err)) {
					dbus_error_free(&err);
				}
				return WS_FAIL;
			}
		if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_UINT32, &port_index,
				DBUS_TYPE_INVALID)) 
		{
			if (SEM_COMMAND_FAILED == op_ret ) 
			{
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			else if(SEM_COMMAND_SUCCESS == op_ret)
			{
				dbus_message_unref(reply);
				return WS_SUCCESS;
			}
			else if (SEM_WRONG_PARAM == op_ret ) 
			{
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			else if (SEM_OPERATE_NOT_SUPPORT == op_ret) {
				
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			else if (SEM_COMMAND_NOT_SUPPORT == op_ret) {
				dbus_message_unref(reply);		
			}
			else
			{
				dbus_message_unref(reply);
				return WS_FAIL;
			}
		} 
		else 
		{
			if (dbus_error_is_set(&err)) 
			{
				dbus_error_free(&err);
			}
			dbus_message_unref(reply);
			return WS_FAIL;
		}
    	query = dbus_message_new_method_call(
    							NPD_DBUS_BUSNAME,		\
    							NPD_DBUS_ETHPORTS_OBJPATH ,	\
    							NPD_DBUS_ETHPORTS_INTERFACE ,		\
    							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
    							DBUS_TYPE_UINT32,&type,
    							 DBUS_TYPE_UINT32,&port_index,
    							 DBUS_TYPE_UINT32,&isEnable,
    							 DBUS_TYPE_INVALID);

		if(connection) {
			
			reply = dbus_connection_send_with_reply_and_block (connection,\
					query, -1, &err);
		}
    		
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		if (dbus_error_is_set(&err)) {
    			dbus_error_free(&err);
    		}
    		return WS_FAIL;
    	}

    	if (dbus_message_get_args ( reply, &err,
    					DBUS_TYPE_UINT32, &op_ret,
    					DBUS_TYPE_UINT32, &port_index,
    					DBUS_TYPE_INVALID)) 
    	{
    		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
    		{
    			ret = ETHPORT_RETURN_CODE_ERR_GENERAL; 
    		}
    		else  if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
    		{
    			ret = ETHPORT_RETURN_CODE_ERR_NONE; 
    		}
    		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
    			ret = ETHPORT_RETURN_CODE_UNSUPPORT; 
    		}
    		else if (ETHPORT_RETURN_CODE_NOT_SUPPORT == op_ret) {
    			ret = ETHPORT_RETURN_CODE_NOT_SUPPORT; 
    		}		
    		else if (ETHPORT_RETURN_CODE_ETH_GE_SFP == op_ret) {
    			ret = ETHPORT_RETURN_CODE_ETH_GE_SFP; 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
    			ret = ETHPORT_RETURN_CODE_ERR_HW; 
    		}		
    		else if (ETHPORT_RETURN_CODE_ERR_OPERATE == op_ret) {
    			ret = ETHPORT_RETURN_CODE_ERR_OPERATE; 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
    			ret = ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE; 
    		}
    	} 
    	else 
    	{
    		if (dbus_error_is_set(&err)) 
    		{
    			dbus_error_free(&err);
    		}
			return WS_FAIL;
    	}
    	
    	dbus_message_unref(reply);
    	return ret;
	}

	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&isEnable,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return WS_FAIL;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 	 
	///////////////////

	{
		if (NPD_DBUS_ERROR == op_ret ) 
		{
			ret = WS_EXEC_COMM_FAIL; 
		}
		else  if((NPD_DBUS_SUCCESS) == op_ret)
		{
			ret = WS_SUCCESS;
		}
		else if(NPD_DBUS_ERROR_AUNE_NONE == op_ret) {
			//vty_out(vty,"%% auto negotiation flow control is configed only speed isn't 1000M.\n");
			ret = WS_FAIL;
		}
		else if (NPD_DBUS_ERROR_UNSUPPORT == op_ret) {
			 ret = WS_NOT_SUPPORT;
		}
		else if (NPD_DBUS_ERROR_NOT_SUPPORT == op_ret) {
			//vty_out(vty,"%% The eth-port wasn't supported auto-flowControl config\n");
			ret = WS_FAIL;
		}		
		else if (NPD_DBUS_ETH_GE_SFP == op_ret) {
			//vty_out(vty,"%% Fiber port not support this operation\n"); 
			ret = WS_FAIL;
		}
		else if (NPD_VLAN_PORT_TRUNK_MBR == op_ret) {
			//vty_out(vty,"%% The eth-port wasn't supported config when the port belonged to a active trunk \n");
            ret = WS_FAIL;
		}		
		else if (NPD_DBUS_ERROR_OPERATE == op_ret) {
			//vty_out(vty,"%% Admin disable not support this operation\n"); 
			return WS_ADMIN_DIS;
		}


	} 

	//////////////////
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		ret = WS_FAIL;
	}
	
	dbus_message_unref(reply);
	return ret;

}

int ccgi_port_dupmode_conf(char *str_port_name, char *str_mode)
{
	DBusMessage *query=NULL, *reply=NULL;
	DBusError err;

	int ret,op_ret;
	unsigned int port_index = 0;
	unsigned int isEnable = 1;
	unsigned int type = DUMOD;

    if(NULL == str_port_name || NULL == str_mode)
    {
    	return WS_FAIL;
    }
	
	if(strcmp(str_mode, "FULL") == 0){
		isEnable = 1;
	}
	else if (strcmp(str_mode,"HALF")== 0){
		isEnable = 0;
	}
	else {
		return WS_FAIL;
	}
	
    if( (ret = ccgi_get_port_index(str_port_name, &port_index)) != WS_SUCCESS){
    	return ret;
    }



    if (1 == distributed_flag)
	{
		unsigned int slot_no;
	    unsigned int local_slot_id = get_product_info(LOCAL_SLOTID_FILE);
		SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);
		
		query = dbus_message_new_method_call(
							SEM_DBUS_BUSNAME,\
							SEM_DBUS_OBJPATH,\
							SEM_DBUS_INTERFACE, \
							SEM_DBUS_CONFIG_ETHPORT_ATTR \
			                );
		dbus_error_init(&err);

		dbus_message_append_args(query,
		 		  		    DBUS_TYPE_UINT32,&type,
							DBUS_TYPE_UINT32,&port_index,
							DBUS_TYPE_UINT32,&isEnable,
							DBUS_TYPE_INVALID);

		DBusConnection *connection = NULL;
		get_slot_dbus_connection(slot_no, &connection, SNMPD_INSTANCE_MASTER_V3);
		if(connection) {
			
			reply = dbus_connection_send_with_reply_and_block (connection,\
					query, -1, &err);
		}

		dbus_message_unref(query);

		
		if (NULL == reply) {
				if (dbus_error_is_set(&err)) {
					dbus_error_free(&err);
				}
				return WS_FAIL;
			}
		if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_UINT32, &port_index,
				DBUS_TYPE_INVALID)) 
		{
			if (SEM_COMMAND_FAILED == op_ret ) 
			{
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			else if(SEM_COMMAND_SUCCESS == op_ret)
			{
				dbus_message_unref(reply);
				return WS_SUCCESS;
			}
			else if (SEM_WRONG_PARAM == op_ret ) 
			{
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			else if (SEM_OPERATE_NOT_SUPPORT == op_ret) {
				
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			else if (SEM_COMMAND_NOT_SUPPORT == op_ret) {
				dbus_message_unref(reply);		
			}
			else
			{
				dbus_message_unref(reply);
				return WS_FAIL;
			}
		} 
		else 
		{
			if (dbus_error_is_set(&err)) 
			{
				dbus_error_free(&err);
			}
			dbus_message_unref(reply);
			return WS_FAIL;
		} 
    	query = dbus_message_new_method_call(
    							NPD_DBUS_BUSNAME,		\
    							NPD_DBUS_ETHPORTS_OBJPATH ,	\
    							NPD_DBUS_ETHPORTS_INTERFACE ,		\
    							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
    							DBUS_TYPE_UINT32,&type,
    							 DBUS_TYPE_UINT32,&port_index,
    							 DBUS_TYPE_UINT32,&isEnable,
    							 DBUS_TYPE_INVALID);
    	
		if(connection) {
			
			reply = dbus_connection_send_with_reply_and_block (connection,\
					query, -1, &err);
		}
    	
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		if (dbus_error_is_set(&err)) {
    			dbus_error_free(&err);
    		}
    		return WS_FAIL;
    	}

    	if (dbus_message_get_args ( reply, &err,
    					DBUS_TYPE_UINT32, &op_ret,
    					DBUS_TYPE_UINT32, &port_index,
    					DBUS_TYPE_INVALID)) 
    	{
    		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
    		{
    			ret = ETHPORT_RETURN_CODE_ERR_GENERAL; 
    		}
    		else if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
    		{
    			ret = ETHPORT_RETURN_CODE_ERR_NONE; 
    		}
    		else if(ETHPORT_RETURN_CODE_DUPLEX_NODE  == op_ret) {
    			ret = ETHPORT_RETURN_CODE_DUPLEX_NODE; 
    		}
    		else if(ETHPORT_RETURN_CODE_ERROR_DUPLEX_HALF == op_ret) {
    			ret = ETHPORT_RETURN_CODE_ERROR_DUPLEX_HALF; 
    		}
    		else if(ETHPORT_RETURN_CODE_ERROR_DUPLEX_FULL == op_ret) {
    			ret = ETHPORT_RETURN_CODE_ERROR_DUPLEX_FULL; 
    		}
    		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
    			ret = ETHPORT_RETURN_CODE_UNSUPPORT; 
    		}
    		else if (ETHPORT_RETURN_CODE_ETH_GE_SFP == op_ret) {
    			ret = ETHPORT_RETURN_CODE_ETH_GE_SFP; 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
    			ret = ETHPORT_RETURN_CODE_ERR_HW; 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
    			ret = ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE; 
    		}
    	} 
    	else 
    	{
    		if (dbus_error_is_set(&err)) 
    		{
    			dbus_error_free(&err);
    		}
			ret = WS_FAIL;
    	}
    	
    	dbus_message_unref(reply);
    	return ret;		
	}
    
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&isEnable,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return WS_FAIL;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 
	{
		if (NPD_DBUS_ERROR == op_ret ) 
		{
			ret = WS_EXEC_COMM_FAIL; 
		}
		else  if(NPD_DBUS_SUCCESS == op_ret)
		{
			ret = WS_SUCCESS;
		}
		else if(NPD_DBUS_ERROR_UNSUPPORT == op_ret)
		{
		    ret = WS_NOT_SUPPORT;
		}
		else
		{
		    ret = WS_FAIL;
		}
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		ret = WS_FAIL;
	}
	
	dbus_message_unref(reply);
	return ret;

}

int ccgi_port_flowctl_conf(char *str_port_name, char *str_flowctl_mode)
{
	DBusMessage *query=NULL, *reply=NULL;
	DBusError err;

	int ret,op_ret;
	unsigned int port_index = 0;
	unsigned int isEnable = 0;
	unsigned int type = FLOWCTRL;

    if(NULL == str_port_name || NULL == str_flowctl_mode)
    {
    	return WS_FAIL;
    }
	
	if(strcmp(str_flowctl_mode , "ON") == 0){
		isEnable = 1;
	}
	else if (strcmp(str_flowctl_mode,"OFF")== 0){
		isEnable = 0;
	}
	else {
		return WS_FAIL;
	}
	
	if( (ret = ccgi_get_port_index(str_port_name, &port_index)) != WS_SUCCESS){
		return ret;
	}

	
    if (1 == distributed_flag)
	{
		unsigned int slot_no;
        unsigned int local_slot_id = get_product_info(LOCAL_SLOTID_FILE);
	    SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);
		
		query = dbus_message_new_method_call(
							SEM_DBUS_BUSNAME,\
							SEM_DBUS_OBJPATH,\
							SEM_DBUS_INTERFACE, \
							SEM_DBUS_CONFIG_ETHPORT_ATTR \
			                );
		dbus_error_init(&err);

		dbus_message_append_args(query,
		 		  		    DBUS_TYPE_UINT32,&type,
							DBUS_TYPE_UINT32,&port_index,
							DBUS_TYPE_UINT32,&isEnable,
							DBUS_TYPE_INVALID);

		DBusConnection *connection = NULL;
		get_slot_dbus_connection(slot_no, &connection, SNMPD_INSTANCE_MASTER_V3);
		if(connection) {
			
			reply = dbus_connection_send_with_reply_and_block (connection,\
					query, -1, &err);
		}

		dbus_message_unref(query);

		
		if (NULL == reply) {
				if (dbus_error_is_set(&err)) {
					dbus_error_free(&err);
				}
				return WS_FAIL;
			}
		if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_UINT32, &port_index,
				DBUS_TYPE_INVALID)) 
		{
			if (SEM_COMMAND_FAILED == op_ret ) 
			{
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			else if(SEM_COMMAND_SUCCESS == op_ret)
			{
				dbus_message_unref(reply);
				return WS_SUCCESS;
			}
			else if (SEM_WRONG_PARAM == op_ret ) 
			{
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			else if (SEM_OPERATE_NOT_SUPPORT == op_ret) {
				
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			else if (SEM_COMMAND_NOT_SUPPORT == op_ret) {
				dbus_message_unref(reply);		
			}
			else
			{
				dbus_message_unref(reply);
				return WS_FAIL;
			}
		} 
		else 
		{
			if (dbus_error_is_set(&err)) 
			{
				dbus_error_free(&err);
			}
			dbus_message_unref(reply);
			return WS_FAIL;
		}	
    	query = dbus_message_new_method_call(
    							NPD_DBUS_BUSNAME,		\
    							NPD_DBUS_ETHPORTS_OBJPATH ,	\
    							NPD_DBUS_ETHPORTS_INTERFACE ,		\
    							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
    							DBUS_TYPE_UINT32,&type,
    							 DBUS_TYPE_UINT32,&port_index,
    							 DBUS_TYPE_UINT32,&isEnable,
    							 DBUS_TYPE_INVALID);
    	
		if(connection) {
			
			reply = dbus_connection_send_with_reply_and_block (connection,\
					query, -1, &err);
		}
        	 
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		if (dbus_error_is_set(&err)) {
    			dbus_error_free(&err);
    		}
    		return WS_FAIL;
    	}

    	if (dbus_message_get_args ( reply, &err,
    					DBUS_TYPE_UINT32, &op_ret,
    					DBUS_TYPE_UINT32, &port_index,
    					DBUS_TYPE_INVALID)) 
    	{
    		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
    		{
    			ret = ETHPORT_RETURN_CODE_ERR_GENERAL; 
    		}
    		else if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
    		{
    			ret = ETHPORT_RETURN_CODE_ERR_NONE; 
    		}
    		else if(ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret)
    		{
    			ret = ETHPORT_RETURN_CODE_NO_SUCH_PORT; 
    		}
    		else if(ETHPORT_RETURN_CODE_FLOWCTL_NODE == op_ret) {
    			ret = ETHPORT_RETURN_CODE_FLOWCTL_NODE; 
    		}
    		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
    			ret = ETHPORT_RETURN_CODE_UNSUPPORT; 
    		}
    		else if (VLAN_RETURN_CODE_PORT_TRUNK_MBR == op_ret) {
    			ret = VLAN_RETURN_CODE_PORT_TRUNK_MBR; 
    		}		
    		else if (ETHPORT_RETURN_CODE_NOT_SUPPORT == op_ret) {
    			ret = ETHPORT_RETURN_CODE_NOT_SUPPORT; 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
    			ret = ETHPORT_RETURN_CODE_ERR_HW; 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
    			ret = ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE; 
    		}
    	} 
    	else 
    	{
    		if (dbus_error_is_set(&err)) 
    		{
    			dbus_error_free(&err);
    		}
			ret = WS_FAIL;
    	}
    	
    	dbus_message_unref(reply);
    	return ret;
	}
	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH , \
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&isEnable,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return WS_FAIL;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 
	
	////////////////////////


	{
		
		if (NPD_DBUS_ERROR == op_ret ) 
		{
			ret = WS_EXEC_COMM_FAIL;
		}
		else  if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
		{
			ret = WS_SUCCESS;
		}
		else if(ETHPORT_RETURN_CODE_FLOWCTL_NODE == op_ret	) {
			//vty_out(vty,"%% Flow control is configed only duplex is full and auto-negotiation flow-control is disable.\n");
			ret = WS_FAIL;
		}
		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
			//vty_out(vty,"%% The eth-port wasn't supported media set\n");
			ret = WS_NOT_SUPPORT;
		}
		else if (ETHPORT_RETURN_CODE_NOT_SUPPORT == op_ret) {
			//vty_out(vty,"%% The eth-port wasn't supported the flow-control config\n");
			  ret = WS_FAIL;
		}
		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
			ret = WS_FAIL;
		}
		else
			ret = WS_FAIL;
} 
else 
{
	if (dbus_error_is_set(&err)) 
	{
		dbus_error_free(&err);
	}
	ret = WS_FAIL;
}

dbus_message_unref(reply);
return ret;

}


int ccgi_port_backpre_conf(char *str_port_name, char *str_bp_mode)
	{
	#if 0
		DBusMessage *query, *reply;
		DBusError err;
		fprintf(stderr,"ccgi_port_backpre_conf***\n");
	
		int ret,op_ret;
		unsigned int port_index = 0;
		unsigned int isEnable = 0;
		unsigned int type = BACKPRE;
	#endif
		DBusMessage *query = NULL, *reply = NULL;
		DBusError err;
	
		int ret = 0, op_ret = 0;
		unsigned int port_index = 0;
		unsigned int isEnable = 0;
		unsigned int type = BACKPRE;
		unsigned int slot_no;
		int local_slot_id = get_product_info(LOCAL_SLOTID_FILE);
		
		if(NULL == str_port_name || NULL == str_bp_mode)
		{
			return WS_FAIL;
		}
		
		if(strcmp(str_bp_mode , "ON") == 0){
			isEnable = 1;
		}
		else if (strcmp(str_bp_mode,"OFF")== 0){
			isEnable = 0;
		}
		else {
			return WS_FAIL;
		}
		fprintf(stderr,"********port_index********============%u\n",port_index);
		if( (ret = ccgi_get_port_index(str_port_name, &port_index)) != WS_SUCCESS){
			return ret;
		}
	#if 0
		query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH , \
								NPD_DBUS_ETHPORTS_INTERFACE ,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
		
		dbus_error_init(&err);
	
		dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&type,
								 DBUS_TYPE_UINT32,&port_index,
								 DBUS_TYPE_UINT32,&isEnable,
								 DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		
		if (NULL == reply) {
			if (dbus_error_is_set(&err)) {
				dbus_error_free(&err);
			}
			return WS_FAIL;
		}
	
		if (dbus_message_get_args ( reply, &err,
						DBUS_TYPE_UINT32, &op_ret,
						DBUS_TYPE_UINT32, &port_index,
						DBUS_TYPE_INVALID)) 
		{
			if (NPD_DBUS_ERROR == op_ret ) 
			{
				ret = WS_EXEC_COMM_FAIL; 
			}
			else  if((NPD_DBUS_SUCCESS) == op_ret)
			{
				ret = WS_SUCCESS;
			}
			else if(NPD_DBUS_ERROR_UNSUPPORT == op_ret)
			{
				ret = WS_NOT_SUPPORT;
			}
			else
			{
				ret = WS_FAIL;
			}
		} 
		else 
		{
			if (dbus_error_is_set(&err)) 
			{
				dbus_error_free(&err);
			}
			ret = WS_FAIL;
		}
		
		dbus_message_unref(reply);
		return ret;
	#endif
		if (distributed_flag == 1)
		{
	#if 1
			unsigned int slot_no;
			int local_slot_id = get_product_info(LOCAL_SLOTID_FILE);
			SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);
		# if 1
			query = dbus_message_new_method_call(
								SEM_DBUS_BUSNAME,\
								SEM_DBUS_OBJPATH,\
								SEM_DBUS_INTERFACE, \
								SEM_DBUS_CONFIG_ETHPORT_ATTR \
								);
			dbus_error_init(&err);
	
			dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&type,
								DBUS_TYPE_UINT32,&port_index,
								DBUS_TYPE_UINT32,&isEnable,
								DBUS_TYPE_INVALID);
			DBusConnection *connection = NULL;
			get_slot_dbus_connection(slot_no, &connection, SNMPD_INSTANCE_MASTER_V3);
			if(connection) {
				
				reply = dbus_connection_send_with_reply_and_block (connection,\
						query, -1, &err);
			}
			dbus_message_unref(query);
			
			if (NULL == reply) {
					
					if (dbus_error_is_set(&err)) {
						
						dbus_error_free(&err);
					}
					return WS_FAIL;
				}
			if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 
			{
				if (SEM_COMMAND_FAILED == op_ret ) 
				{
					
					dbus_message_unref(reply);
					return WS_FAIL;
				}
				else if(SEM_COMMAND_SUCCESS == op_ret)
				{
					
					dbus_message_unref(reply);
					return WS_SUCCESS;
				}
				else if (SEM_WRONG_PARAM == op_ret ) 
				{
					
					dbus_message_unref(reply);
					return WS_FAIL;
				}
				else if (SEM_OPERATE_NOT_SUPPORT == op_ret) {
					
					
					dbus_message_unref(reply);
					return WS_FAIL;
				}
				else if (SEM_COMMAND_NOT_SUPPORT == op_ret) {
					
					dbus_message_unref(reply);		
				}
				else
				{
					
					dbus_message_unref(reply);
					return WS_FAIL;
				}
			} 
			else 
			{
				
				if (dbus_error_is_set(&err)) 
				{
					
					dbus_error_free(&err);
				}
				dbus_message_unref(reply);
				return WS_FAIL;
			}
		# endif
	#endif
			query = dbus_message_new_method_call(
									NPD_DBUS_BUSNAME,		\
									NPD_DBUS_ETHPORTS_OBJPATH , \
									NPD_DBUS_ETHPORTS_INTERFACE ,		\
									NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
			
			dbus_error_init(&err);
	
			dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									 DBUS_TYPE_UINT32,&port_index,
									 DBUS_TYPE_UINT32,&isEnable,
									 DBUS_TYPE_INVALID);
			SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);
			if(connection) {
				
				reply = dbus_connection_send_with_reply_and_block (connection,\
						query, -1, &err);
			}
			
			
			dbus_message_unref(query);
			
			if (NULL == reply) {
				
				if (dbus_error_is_set(&err)) {
					
					dbus_error_free(&err);
				}
				return WS_FAIL;
			}
	
			if (dbus_message_get_args ( reply, &err,
							DBUS_TYPE_UINT32, &op_ret,
							DBUS_TYPE_UINT32, &port_index,
							DBUS_TYPE_INVALID)) 
			{
				if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
				{
					ret = ETHPORT_RETURN_CODE_ERR_GENERAL; 
				}
				else if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
				{
					ret = ETHPORT_RETURN_CODE_ERR_NONE; 
				}
				else if(ETHPORT_RETURN_CODE_BACKPRE_NODE == op_ret) {
					ret = ETHPORT_RETURN_CODE_BACKPRE_NODE; 
				}
				else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
					ret = ETHPORT_RETURN_CODE_UNSUPPORT; 
				}
				else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
					ret = ETHPORT_RETURN_CODE_ERR_HW; 
				}		
				else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
					ret = ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE; 
				}
			} 
			else 
			{
				
				if (dbus_error_is_set(&err)) 
				{
					
					dbus_error_free(&err);
				}
				ret = WS_FAIL;
			}
			
			dbus_message_unref(reply);
			return ret;
		}
		/* is not distributed  */
		query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH , \
								NPD_DBUS_ETHPORTS_INTERFACE ,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
		
		dbus_error_init(&err);
	
		dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&type,
								 DBUS_TYPE_UINT32,&port_index,
								 DBUS_TYPE_UINT32,&isEnable,
								 DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		
		if (NULL == reply) {
			
			if (dbus_error_is_set(&err)) {
			
				dbus_error_free(&err);
			}
			return WS_FAIL;
		}
	
		if (dbus_message_get_args ( reply, &err,
						DBUS_TYPE_UINT32, &op_ret,
						DBUS_TYPE_UINT32, &port_index,
						DBUS_TYPE_INVALID)) 
		{
			if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
			{
				 ret = ETHPORT_RETURN_CODE_ERR_GENERAL; 
			}
			else if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
			{
				 ret = ETHPORT_RETURN_CODE_ERR_NONE; 
			}
			else if(ETHPORT_RETURN_CODE_BACKPRE_NODE == op_ret) {
				ret = ETHPORT_RETURN_CODE_BACKPRE_NODE; 
			}
			else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
				ret = ETHPORT_RETURN_CODE_UNSUPPORT; 
			}
			else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
				ret = ETHPORT_RETURN_CODE_ERR_HW;
			}		
			else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
				ret = ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE;
			}
		} 
		else 
		{
		
			if (dbus_error_is_set(&err)) 
			{
				
				dbus_error_free(&err);
			}
			ret = WS_FAIL;
		}
		
		dbus_message_unref(reply);
		fprintf(stderr,"*********************(ret)********************=%d\n",ret);
		return ret;
	}


int ccgi_port_link_state(char *str_port_name, char *str_link_mode)
{
	DBusMessage *query, *reply;
	DBusError err;

	int ret,op_ret;
	unsigned int port_index = 0;
	unsigned int isEnable = 0;
	//unsigned int type = LINKS;

	unsigned char slot_no,port_no;
	op_ret = parse_slotport_no((char *)str_port_name, &slot_no, &port_no);


    if(NULL == str_port_name || NULL == str_link_mode)
    {
    	return WS_FAIL;
    }
	
	if(strcmp(str_link_mode , "UP") == 0){
		isEnable = 1;
	}
	else if (strcmp(str_link_mode,"DOWN")== 0){
		isEnable = 0;
	}	
	else if (strcmp(str_link_mode,"AUTO")== 0){
		isEnable = 2;
	}
	else {
		return WS_FAIL;
	}
	
    if( (ret = ccgi_get_port_index(str_port_name, &port_index)) != WS_SUCCESS){
    	return ret;
    }
    
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_SYSTEM_CONFIG_PORT_LINK);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
							 DBUS_TYPE_UINT32,&isEnable,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return WS_FAIL;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 	 

	/////////////////////////


   	{
		if (NPD_DBUS_ERROR == op_ret ) 
		{
			//vty_out(vty,"execute command failed.\n"); 
			ret = WS_EXEC_COMM_FAIL; 
		}
		else if(NPD_DBUS_ERROR_NO_SUCH_PORT == op_ret){
				//vty_out(vty,"%% Error NO SUCH PORT!\n");
				 ret = WS_FAIL;
		}
		else  if((NPD_DBUS_SUCCESS) == op_ret){
			//vty_out(vty," config link state succeed\n",port_index);
			ret = WS_SUCCESS;
		}
		else if (NPD_DBUS_ERROR_UNSUPPORT == op_ret) {
			//vty_out(vty,"%% The eth-port wasn't supported media set\n");
			ret = WS_NOT_SUPPORT;
		}
	} 


	/////////////////////////
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		ret = WS_FAIL;
	}
	
	dbus_message_unref(reply);
	return ret;
}

/* modify 2009-02-24 version 1.73*/
int ccgi_port_mtu_conf(char *str_port_name, char *str_mtu)
{
	DBusMessage *query=NULL, *reply=NULL;
	DBusError err;

	int ret=0,op_ret=0;
	unsigned int port_index = 0;
	unsigned short param = 1522;
	unsigned int mtu,type = CFGMTU;

    if(NULL == str_port_name || NULL == str_mtu)
    {
    	return WS_FAIL;
    }
	
	ret = parse_short_parse((char *)str_mtu,&param);
	
	if(NPD_SUCCESS != ret){
		return WS_FAIL;
	}

	if((param < 64) || (param > 8192)) {
		//vty_out(vty,"%% bad param.\n");
		return WS_OUT_RANGE;  //超出范围 8
	    
	}

	
	mtu = param;
	
	if( (ret = ccgi_get_port_index(str_port_name, &port_index)) != WS_SUCCESS){
    	return ret;
    }

    if (1 == distributed_flag)
	{
		unsigned int slot_no;
        unsigned int local_slot_id = get_product_info(LOCAL_SLOTID_FILE);
		
		SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);
		
		query = dbus_message_new_method_call(
							SEM_DBUS_BUSNAME,\
							SEM_DBUS_OBJPATH,\
							SEM_DBUS_INTERFACE, \
							SEM_DBUS_CONFIG_ETHPORT_ATTR \
			                );
		dbus_error_init(&err);

		dbus_message_append_args(query,
		 		  		    DBUS_TYPE_UINT32,&type,
							DBUS_TYPE_UINT32,&port_index,
							DBUS_TYPE_UINT32,&mtu,
							DBUS_TYPE_INVALID);
		
		SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);

		DBusConnection *connection = NULL;
		get_slot_dbus_connection(slot_no, &connection, SNMPD_INSTANCE_MASTER_V3);
		if(connection) {
			
    		reply = dbus_connection_send_with_reply_and_block (connection,\
    				query, -1, &err);
    	}
		
		dbus_message_unref(query);

		
		if (NULL == reply) {
			if (dbus_error_is_set(&err)) {
				dbus_error_free(&err);
			}
			return WS_FAIL;
		}
		if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32, &op_ret,
				DBUS_TYPE_UINT32, &port_index,
				DBUS_TYPE_INVALID)) 
		{
			if (SEM_COMMAND_FAILED == op_ret ) 
			{
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			else if(SEM_COMMAND_SUCCESS == op_ret)
			{
				dbus_message_unref(reply);
				return WS_SUCCESS;
			}
			else if (SEM_WRONG_PARAM == op_ret ) 
			{
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			else if (SEM_OPERATE_NOT_SUPPORT == op_ret) {
				
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			else if (SEM_COMMAND_NOT_SUPPORT == op_ret) {
				dbus_message_unref(reply);		
			}
			else
			{
				dbus_message_unref(reply);
				return WS_FAIL;
			}
		} 
		else 
		{
			if (dbus_error_is_set(&err)) 
			{
				dbus_error_free(&err);
			}
			dbus_message_unref(reply);
			return WS_FAIL;
		} 
		
    	query = dbus_message_new_method_call(
    							NPD_DBUS_BUSNAME,		\
    							NPD_DBUS_ETHPORTS_OBJPATH ,	\
    							NPD_DBUS_ETHPORTS_INTERFACE ,		\
    							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
    							 DBUS_TYPE_UINT32,&type,
    							 DBUS_TYPE_UINT32,&port_index,
    							 DBUS_TYPE_UINT32,&mtu,
    							 DBUS_TYPE_INVALID);

		if(connection) {
			
    		reply = dbus_connection_send_with_reply_and_block (connection,\
    				query, -1, &err);
    	}
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		if (dbus_error_is_set(&err)) {
    			dbus_error_free(&err);
    		}
    		return WS_FAIL;
    	}

    	if (dbus_message_get_args ( reply, &err,
    					DBUS_TYPE_UINT32, &op_ret,
    					DBUS_TYPE_UINT32, &port_index,
    					DBUS_TYPE_INVALID)) 
    	{
    		if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret ) 
    		{
    			ret = ETHPORT_RETURN_CODE_ERR_GENERAL; 
    		}
    		else if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
    		{
    			ret = ETHPORT_RETURN_CODE_ERR_NONE; 
    		}
    		else if (ETHPORT_RETURN_CODE_BAD_VALUE == op_ret)
    		{
    			ret = ETHPORT_RETURN_CODE_BAD_VALUE; 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_HW == op_ret) {
    			ret = ETHPORT_RETURN_CODE_ERR_HW; 
    		}
    		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
    			ret = ETHPORT_RETURN_CODE_UNSUPPORT; 
    		}
    		else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
    			ret = ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE; 
    		}
    	} 
    	else 
    	{
    		if (dbus_error_is_set(&err)) 
    		{
    			dbus_error_free(&err);
    		}
			ret = WS_FAIL;
    	}
    	
    	dbus_message_unref(reply);
    	return ret;
    }
    
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&mtu,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return WS_FAIL;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 
	{
		if (NPD_DBUS_ERROR == op_ret ) 
		{
			ret = WS_EXEC_COMM_FAIL; 
			//return 4;
		}
		else  if((NPD_DBUS_SUCCESS) == op_ret)
		{ 
		    ret = WS_SUCCESS;
			//return 0;
		}
		
		else if (NPD_DBUS_BAD_VALUE==op_ret)

		{
           //vty_out(vty,"%% input only even value.\n"); 
           //偶数
           ret=WS_BAD_VALUE;
		}

	    /*
		else if(NPD_DBUS_ERROR_UNSUPPORT == op_ret)
		{
		    ret = WS_NOT_SUPPORT;
		    //return 6;
		}
		else
		{
		    ret = WS_FAIL;
		    //return 1;
		}
		*/
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		ret = WS_FAIL;
	}
	
	dbus_message_unref(reply);
	return ret;	
	
}

int ccgi_port_default(char *str_port_name)
{
	DBusMessage *query, *reply;
	DBusError err;

	int ret,op_ret;
	unsigned int port_index = 0;
	unsigned int defval = 0;
	unsigned int type = DEFAULT;
	unsigned int slot_no;
	instance_parameter *para = NULL;
	
    if( (ret = ccgi_get_port_index(str_port_name, &port_index)) != WS_SUCCESS){
    	return ret;
    }
    
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&defval,
							 DBUS_TYPE_INVALID);
	
	SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);
	fprintf(stderr,"slot_no=%d\n",slot_no);
	if(SNMPD_DBUS_ERROR == get_slot_dbus_connection(slot_no, &para, SNMPD_SLOT_CONNECT))
	{
		return -1;
	}
	
	reply = dbus_connection_send_with_reply_and_block (para->connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return WS_FAIL;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT32, &port_index,
					DBUS_TYPE_INVALID)) 
	{
		if (NPD_DBUS_ERROR == op_ret ) 
		{
			ret = WS_EXEC_COMM_FAIL; 
		}
		else  if((NPD_DBUS_SUCCESS) == op_ret)
		{
			ret = WS_SUCCESS;
		}
		else if(NPD_DBUS_ERROR_UNSUPPORT == op_ret)
		{
		    ret = WS_NOT_SUPPORT;
		}
		else
		{
		    ret = WS_FAIL;
		}
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		ret = WS_FAIL;
	}
	
	dbus_message_unref(reply);
	return ret;
}

int ccgi_pinterface_state_return_err()
{
	char *p_err = NULL;
	char err_tmp[256] ={0};
	FILE *error = NULL;
	if (access(VTY_SHUTDOWN_ERR_TMP, F_OK|R_OK) == 0)
	{						
		error = fopen(VTY_SHUTDOWN_ERR_TMP,"r");
		//fprintf(stderr,"error = %p\n",error);
		if(NULL != error)
		{
			fgets(err_tmp,sizeof(err_tmp),error);
			//fprintf(stderr,"err_tmp_test = %s\n",err_tmp);
			p_err = err_tmp;
			if(strstr(p_err, "delete the port from vlan first")!=NULL){
				fclose(error);
				return WS_DEL_FROM_VLAN_FIRST;
			}
			else if((strstr(p_err, "NO SUCH PORT")!=NULL)||(strstr(p_err, "Port interface not exists")!=NULL)){
				fclose(error);
				return WS_NO_SUCH_PORT;
			}
			else if(strstr(p_err, "Unsupport this command")!=NULL){
				fclose(error);
				return WS_NOT_SUPPORT;
			}
			else{
				fclose(error);
				return WS_ERR_UNKNOW;	
			}
		}
		else
			return WS_ERR_UNKNOW;	
		fclose(error);
	}
}
int ccgi_port_interface_state_conf(char *str_port_name, char *state)
{
	char cmd[256]={0};	
	int sys_ret = -1;
	int ret_error = -1;
	if(strcmp(state,"DOWN") == 0)
	{
		if (access(VTY_SHUTDOWN, F_OK|X_OK) != 0)
		{
			return WS_EXEC_COMM_FAIL;		
		}
		else
		{
			snprintf(cmd,sizeof(cmd)-1,"%s eth%s shutdown",VTY_SHUTDOWN,str_port_name);			
			sys_ret = system(cmd);			
			if(sys_ret == 0)
				return WS_SUCCESS;
			else
			{
				//fprintf(stderr,"sys_ret = %d\n",sys_ret);
				ret_error = ccgi_pinterface_state_return_err();
				return ret_error;				
			}				
		}
	}
	else if(strcmp(state,"UP") == 0)
	{
		if (access(VTY_SHUTDOWN, F_OK|X_OK) != 0)
		{
			return WS_EXEC_COMM_FAIL;			
		}
		else
		{
			snprintf(cmd,sizeof(cmd)-1,"%s eth%s \"no shutdown\"",VTY_SHUTDOWN, str_port_name);
			sys_ret = system(cmd);
			//fprintf(stderr,"cmd = %s,sys_ret=%d\n",cmd,sys_ret);
			if(sys_ret == 0)
				return WS_SUCCESS;
			{
				//fprintf(stderr,"sys_ret = %d\n",sys_ret);
				ret_error = ccgi_pinterface_state_return_err();
				return ret_error;				
			}
		}
	}
	else if(strcmp(state,"NO_INTERFACE") == 0)
	{
		if (access(VTY_SHUTDOWN, F_OK|X_OK) != 0)
		{
			return WS_EXEC_COMM_FAIL;			
		}
		else
		{
			snprintf(cmd,sizeof(cmd)-1,"%s eth%s",VTY_SHUTDOWN,str_port_name);
			sys_ret = system(cmd);
			//fprintf(stderr,"cmd = %s,sys_ret=%d\n",cmd,sys_ret);
			if(sys_ret == 0)
				return WS_SUCCESS;
			{
				//fprintf(stderr,"sys_ret = %d\n",sys_ret);
				ret_error = ccgi_pinterface_state_return_err();
				return ret_error;				
			}
		}
	}
		
	return 0;
}


int ccgi_port_medai_conf(char *str_port_name, char *str_media_mode)
{
	DBusMessage *query=NULL, *reply=NULL;
	DBusError err;
	unsigned int media = 0;
	unsigned int ret,op_ret = 0;
	unsigned int port_index = 0;

    if(NULL == str_port_name || NULL == str_media_mode)
    {
    	return WS_FAIL;
    }
	
	if(strcmp(str_media_mode , "COPPER") == 0){
		media = COMBO_PHY_MEDIA_PREFER_COPPER;
	}
	else if(strcmp(str_media_mode,"FIBER")== 0){
		media = COMBO_PHY_MEDIA_PREFER_FIBER;
	}
	else if(strcmp(str_media_mode,"NONE")== 0){
		media = COMBO_PHY_MEDIA_PREFER_NONE;
	}
	else {
		return WS_FAIL;
	}

    if( (ret = ccgi_get_port_index(str_port_name, &port_index)) != WS_SUCCESS){
    	return ret;
    }

    if (1 == distributed_flag)
	{
    	int slot_no;
    	unsigned int local_slot_id = get_product_info(LOCAL_SLOTID_FILE);
		SLOT_PORT_ANALYSIS_SLOT(port_index, slot_no);
		
		query = dbus_message_new_method_call(
							SEM_DBUS_BUSNAME,\
							SEM_DBUS_OBJPATH,\
							SEM_DBUS_INTERFACE, 
							SEM_DBUS_CONFIG_MEDIA_PREFERR
			);

		dbus_error_init(&err);

		dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&media,
							 DBUS_TYPE_INVALID); 	
		
		DBusConnection *connection = NULL;
		get_slot_dbus_connection(slot_no, &connection, SNMPD_INSTANCE_MASTER_V3);
		if(connection) {
			
    		reply = dbus_connection_send_with_reply_and_block (connection,\
    				query, -1, &err);
    	}

		dbus_message_unref(query);
	
		if (NULL == reply) {
			if (dbus_error_is_set(&err)) {
				dbus_error_free(&err);
			}
			return WS_FAIL;
		}
	
		if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID)) 
		{
			if (SEM_COMMAND_SUCCESS == op_ret ) {
				dbus_message_unref(reply);
				return WS_SUCCESS;
			}
			else if (SEM_OPERATE_NOT_SUPPORT == op_ret) {
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			#if 0
			else if (ETHPORT_RETURN_CODE_NOT_SUPPORT == op_ret) {
				vty_out(vty,"%% The eth-port don't support media set 'none'.\n");
			}
			else if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret) {
				vty_out(vty,"%% general error such as device or port out of range and etc.");
			}
			#endif
			else if (SEM_WRONG_PARAM == op_ret)
			{
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			else if (SEM_COMMAND_NOT_SUPPORT == op_ret)
			{
				dbus_message_unref(reply);
			}
			else if (SEM_COMMAND_FAILED == op_ret) {
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			else 
			{
				dbus_message_unref(reply);
				return WS_FAIL;
			}
			
		} else {
			if (dbus_error_is_set(&err)) {
				dbus_error_free(&err);
			}
		    dbus_message_unref(reply);
			return WS_FAIL;
		}

    	query = dbus_message_new_method_call(
    					NPD_DBUS_BUSNAME,				\
    					NPD_DBUS_ETHPORTS_OBJPATH,		\
    					NPD_DBUS_ETHPORTS_INTERFACE,		\
    					NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_MEDIA);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
    							 DBUS_TYPE_UINT32,&port_index,
    							 DBUS_TYPE_UINT32,&media,
    							 DBUS_TYPE_INVALID);

		if(connection) {
			
    		reply = dbus_connection_send_with_reply_and_block (connection,\
    				query, -1, &err);
    	}
    	
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		if (dbus_error_is_set(&err)) {
    			dbus_error_free(&err);
    		}
    		return WS_FAIL;
    	}
    	
    	if (dbus_message_get_args ( reply, &err,
    		DBUS_TYPE_UINT32,&op_ret,
    		DBUS_TYPE_INVALID)) {
    			if (ETHPORT_RETURN_CODE_ERR_NONE == op_ret ) {
    				ret = ETHPORT_RETURN_CODE_ERR_NONE;
    			}
    			else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret) {
    				ret = ETHPORT_RETURN_CODE_UNSUPPORT;
    			}
    			else if (ETHPORT_RETURN_CODE_NOT_SUPPORT == op_ret) {
    				ret = ETHPORT_RETURN_CODE_NOT_SUPPORT;
    			}
    			else if (ETHPORT_RETURN_CODE_ERR_GENERAL == op_ret) {
    				ret = ETHPORT_RETURN_CODE_ERR_GENERAL;
    			}
    			else if (ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE == op_ret) {
    				ret = ETHPORT_RETURN_CODE_ERR_PRODUCT_TYPE;
    			}
    			else 
    				ret = WS_FAIL;
    		
    	} else {
    		if (dbus_error_is_set(&err)) {
    			dbus_error_free(&err);
    		}
			ret = WS_FAIL;
    	}
    	
    	dbus_message_unref(reply);
    	return ret;
	}

	query = dbus_message_new_method_call(
					NPD_DBUS_BUSNAME,				\
					NPD_DBUS_ETHPORTS_OBJPATH,		\
					NPD_DBUS_ETHPORTS_INTERFACE,		\
					NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_MEDIA);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&media,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return WS_FAIL;
	}
	
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) 
	{
	  
		///////////////////

            if (NPD_DBUS_SUCCESS == op_ret ) 
			{
				ret = WS_SUCCESS;
			}
			else if (NPD_DBUS_ERROR_UNSUPPORT == op_ret) {
				//vty_out(vty,"%% The eth-port isn't supported media set\n");
				 ret = WS_NOT_SUPPORT;
			}
			else if (NPD_DBUS_ERROR_NOT_SUPPORT == op_ret) {
				//vty_out(vty,"%% The eth-port isn't supported media set 'none'.\n");
				ret = WS_FAIL;
			}
			else 
				//vty_out(vty,"%% Config media error\n");
				ret = WS_FAIL;

		/////////////////
	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		ret = WS_FAIL;
	}
	
	dbus_message_unref(reply);
	return ret;

}


void Free_arp_nexthop_head(struct arp_nexthop_profile *head)
{
    struct arp_nexthop_profile *f1,*f2;
	f1=head->next;		 
	f2=f1->next;
	while(f2!=NULL)
	{
	  free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
}

   

int show_ethport_arp_info(unsigned int value,unsigned int type,struct arp_nexthop_profile *head,int *num)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned char slot_no = 0 ,port_no = 0;
	unsigned int op_ret = 0,i = 0,j = 0,retu=1;
	unsigned int arpCount = 0;
	unsigned int ipaddr = 0;
	unsigned char mac[6] = {0};
	unsigned char isTrunk = 0;
	unsigned char trunkId = 0;
	unsigned short vid = 0,vidx = 0;
	unsigned char isTagged = 0,isStatic = 0, isValid = 0;
	struct arp_nexthop_profile *q,*tail;	
	


	if(0 == type){
		slot_no = (unsigned char)((value>>8)&0xff);
		port_no = (unsigned char)(value & 0xff);
		value = 0xffff; 
	}

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_ARP);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
							 DBUS_TYPE_UINT32,&value,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}



	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&arpCount);

	dbus_message_iter_next(&iter);	
	

	if(0 == op_ret){

		 if(1 == type) {
			ccgi_get_slot_port_by_portindex(value, &slot_no,&port_no);
		}

        head->next = NULL;
	    tail=head;
		*num=arpCount;      

		if(arpCount > 0) {
			dbus_message_iter_recurse(&iter,&iter_array);
			for(i = 0; i<arpCount; i++) {
                q=(struct arp_nexthop_profile *)malloc(sizeof(struct arp_nexthop_profile));			
                if ( NULL ==q )
     			{
     			return 0;
     			}
				
				dbus_message_iter_recurse(&iter_array,&iter_struct);

				dbus_message_iter_get_basic(&iter_struct,&isValid);
				dbus_message_iter_next(&iter_struct);
				//vty_out(vty,"%-2s",isValid ? " " : "*");
				q->isValidz=isValid;
                
				dbus_message_iter_get_basic(&iter_struct,&ipaddr);
				dbus_message_iter_next(&iter_struct);

                q->ipaddr=ipaddr;

				for(j = 0; j< 6; j++) {
		 			dbus_message_iter_get_basic(&iter_struct,&mac[j]);
					dbus_message_iter_next(&iter_struct);
				}

				q->mac[0]=mac[0];
				q->mac[1]=mac[1];
				q->mac[2]=mac[2];
				q->mac[3]=mac[3];
				q->mac[4]=mac[4];
				q->mac[5]=mac[5];

				q->slot_no=slot_no;
				q->port_no=port_no;

				dbus_message_iter_get_basic(&iter_struct,&isTrunk);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&trunkId);
				dbus_message_iter_next(&iter_struct);
				q->isTrunk=isTrunk;
				if(isTrunk) 
				  q->trunkId=trunkId;

				dbus_message_iter_get_basic(&iter_struct,&vid);
				dbus_message_iter_next(&iter_struct);
				q->vid=vid;

				dbus_message_iter_get_basic(&iter_struct,&vidx);
				dbus_message_iter_next(&iter_struct);

				dbus_message_iter_get_basic(&iter_struct,&isTagged);
				dbus_message_iter_next(&iter_struct);
				q->isTagged=isTagged;

				dbus_message_iter_get_basic(&iter_struct,&isStatic);
				dbus_message_iter_next(&iter_struct);
				q->isStatic = isStatic;

				dbus_message_iter_next(&iter_array);

				q->next=NULL;
			    tail->next=q;
			    tail=q;
			}
		}
	}
	else if (NPD_DBUS_ERROR_NO_SUCH_PORT == op_ret) 
	{
		retu=-1;
	}
	else if (NPD_DBUS_ERROR == op_ret ) 
	{
		retu=-2;
	}
	dbus_message_unref(reply);

	return retu;
}

/*当retu==1且arp_num>0时，Free_arp_nexthop_head()*/
int ccgi_show_ethport_arp(char *str_port_name,struct arp_nexthop_profile *arp_head,int *arp_num)  	/*返回0表示失败，返回1表示成功，返回-1表示no such port，返回-2表示execute command failed*/
{
	unsigned char slotno = 0,portno = 0;
	unsigned int type;
	unsigned int value = 0;
	int ret=0;
	
	parse_slotport_no(str_port_name, &slotno, &portno);
	
	type = 0;
	value |= (slotno<<8);
	value |= portno;

	ret = show_ethport_arp_info(value,type,arp_head,arp_num);
	return ret;
}

/*modify version 1.73*/
int clear_eth_port_arp(unsigned int value,unsigned int type,unsigned int isStatic)
{
	DBusMessage *query, *reply;
	DBusError err;

	unsigned char slot_no = 0 ,port_no = 0;
	unsigned int op_ret = 0,retu=0;


	if(0 == type){
		slot_no = (unsigned char)((value>>8)&0xff);
		port_no = (unsigned char)(value & 0xff);
		value = 0xffff; 
	}

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CLEAR_ETHPORT_ARP);
	

	dbus_error_init(&err);
		dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
							 DBUS_TYPE_UINT32,&value,
							 DBUS_TYPE_UINT32,&isStatic,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) 
	{
		if (NPD_DBUS_ERROR == op_ret ) 
		{
			retu=-1;
		}
		else  if((NPD_DBUS_SUCCESS) == op_ret)
		{
			retu=1;
		}
		else if (NPD_DBUS_ERROR_NO_SUCH_PORT == op_ret) 
		{
			//vty_out(vty,"Error:Illegal %d/%d,No such port.\n",slot_no,port_no);//Stay here,not enter eth_config_node CMD.
            retu=WS_NO_SUCH_PORT;
			//retu=2;
		}

	} 
	else 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		retu=0;
	}
	
	dbus_message_unref(reply);
	return retu;
}

int ccgi_clear_ethport_arp(char *str_port_name)  	/*返回0表示失败，返回1表示成功，返回-1表示execute command failed*/
{

	unsigned char slot_no,port_no;
	unsigned int type;
	unsigned int value = 0;
	int ret;
	unsigned int isStatic = 0;

	ret=parse_slotport_no((char *)str_port_name,&slot_no,&port_no);

	if (NPD_FAIL == ret) {
    	//vty_out(vty,"Unknow portno format.\n");
		return WS_FAIL;
	}

	
	type = 0;
	value = slot_no;
	value =  (value << 8) |port_no;

	/*
	if(!strncmp(argv[1],"static",strlen(argv[1]))){
            isStatic = 1;
		}
		else {
            //vty_out(vty,"%% Unknown Command!\n");
			return WS_FAIL;
		}

   */
	
	ret = clear_eth_port_arp(value,type,isStatic);	
	if(ret != WS_SUCCESS)
		//vty_out(vty,"%% Clear arp error\n");
		return WS_FAIL;

	return ret;
}


int show_ethport_nexthop_info(	unsigned int value,unsigned int type,struct arp_nexthop_profile *head,int *num)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned char slot_no = 0 ,port_no = 0;
	unsigned int op_ret = 0,i = 0,j = 0,retu=1;
	unsigned int arpCount = 0;
	unsigned int ipaddr = 0;
	unsigned char mac[6] = {0};
	unsigned char isTrunk = 0;
	unsigned char trunkId = 0;
	unsigned short vid = 0,vidx = 0;
	unsigned char isTagged = 0,isStatic = 0;
	unsigned int refCnt = 0;
	struct arp_nexthop_profile *q,*tail;	
	


	if(0 == type){
		slot_no = (unsigned char)((value>>8)&0xff);
		port_no = (unsigned char)(value & 0xff);
		value = 0xffff; 
	}

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_NEXTHOP);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
							 DBUS_TYPE_UINT32,&value,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}



	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&arpCount);

	dbus_message_iter_next(&iter);	
	

	if(0 == op_ret){

		head->next = NULL;
	    tail=head;
		*num=arpCount;
		
		if(arpCount > 0) {
			dbus_message_iter_recurse(&iter,&iter_array);
			for(i = 0; i<arpCount; i++) {
				q=(struct arp_nexthop_profile *)malloc(sizeof(struct arp_nexthop_profile));		
				
				dbus_message_iter_recurse(&iter_array,&iter_struct);

				dbus_message_iter_get_basic(&iter_struct,&ipaddr);
				dbus_message_iter_next(&iter_struct);
				q->ipaddr=ipaddr;

				for(j = 0; j< 6; j++) {
		 			dbus_message_iter_get_basic(&iter_struct,&mac[j]);
					dbus_message_iter_next(&iter_struct);
				}

				q->mac[0]=mac[0];
				q->mac[1]=mac[1];
				q->mac[2]=mac[2];
				q->mac[3]=mac[3];
				q->mac[4]=mac[4];
				q->mac[5]=mac[5];

				q->slot_no=slot_no;
				q->port_no=port_no;

				dbus_message_iter_get_basic(&iter_struct,&isTrunk);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&trunkId);
				dbus_message_iter_next(&iter_struct);
				q->isTrunk=isTrunk;
				if(isTrunk) 
				  q->trunkId=trunkId;

				dbus_message_iter_get_basic(&iter_struct,&vid);
				dbus_message_iter_next(&iter_struct);
				q->vid=vid;

				dbus_message_iter_get_basic(&iter_struct,&vidx);
				dbus_message_iter_next(&iter_struct);

				dbus_message_iter_get_basic(&iter_struct,&isTagged);
				dbus_message_iter_next(&iter_struct);
				q->isTagged=isTagged;

				dbus_message_iter_get_basic(&iter_struct,&isStatic);
				dbus_message_iter_next(&iter_struct);
				q->isStatic = isStatic;//add by sjw  20081203

				dbus_message_iter_get_basic(&iter_struct,&refCnt);
				dbus_message_iter_next(&iter_struct);
				q->refCnt=refCnt;

				dbus_message_iter_next(&iter_array);

				q->next=NULL;
			    tail->next=q;
			    tail=q;
			}
		}
	}
	else if (NPD_DBUS_ERROR_NO_SUCH_PORT == op_ret) 
	{
		retu=-1;
	}
	else if (NPD_DBUS_ERROR == op_ret ) 
	{
	    retu=-2;
	}
	dbus_message_unref(reply);

	return retu;
}


/*当retu==1且nexthop_num>0时，Free_arp_nexthop_head()*/
int ccgi_show_ethport_nexthop(char *str_port_name,struct arp_nexthop_profile *nexthop_head,int *nexthop_num)  	/*返回0表示失败，返回1表示成功，返回-1表示no such port，返回-2表示execute command failed*/
{
	unsigned char slotno = 0,portno = 0;
	unsigned int type;
	unsigned int value = 0;
	int ret;
	
	parse_slotport_no(str_port_name, &slotno, &portno);
	
	type = 0;
	value |= (slotno<<8);
	value |= portno;

	ret = show_ethport_nexthop_info(value,type,nexthop_head,nexthop_num);
	return ret;
}

//************************************
int ccgi_get_port_flow
(
unsigned int value,
unsigned char type,
port_flow *test,
unsigned int slot,
unsigned int plot
)
{
//函数体部分

	DBusMessage *query=NULL, *reply=NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned char slot_no = 0 ,port_no = 0;
	unsigned int op_ret=0,i=0;
	unsigned long long tmp = 0;  	
	unsigned int linkupcount = 0,linkdowncount = 0;
	struct eth_port_counter_s *ptr,stat = {{0}};
	ptr = &stat;
    unsigned int port_index = 0;
	
    if(0 == type){		
		slot_no=(unsigned char)slot;
		port_no=(unsigned char)plot;
		value = 0xffff; 
				
	}
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_STAT);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&type,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
							 DBUS_TYPE_UINT32,&value,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);  //dcli-----ccgi
	
	dbus_message_unref(query);
	if (NULL == reply) {	
		if (dbus_error_is_set(&err)) {		 
			dbus_error_free(&err);
			}		
		return WS_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	if(0 == op_ret){
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for(i = 0; i < 32 ; i++)
		{
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&tmp);
			dbus_message_iter_next(&iter_struct);
			*((unsigned long long*)ptr + i) = tmp;
			tmp = 0ULL;
			
			dbus_message_iter_get_basic(&iter_struct,&linkupcount);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&linkdowncount);
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_next(&iter_array);
		}
		//vty_out(vty,"here end\n");
		
         //可到页面调用的地方去判断条件
         
		if(0 == type){
		////	vty_out(vty,"Detail Information of Ethernet Port %d-%d\n",slot_no,port_no);
		test->slot=slot_no;
		test->plot=port_no;
			}
		else {
			
		//	dcli_get_slot_port_by_portindex(vty,value, &slot_no,&port_no);  
		ccgi_get_slot_port_by_portindex(port_index,&slot_no,&port_no);		
		test->slot=slot_no;
		test->plot=port_no;
		}	
	
	//// RX
		tmp = stat.rx.goodbytesh;
		tmp = (tmp<<32) | stat.rx.goodbytesl;
		/*// vty_out(vty,"goodbytes:%lld  ( %lld  MiB ) badbytes:%lld  ( %lld  MiB )\n",	\
								tmp, (tmp!=0) ? (tmp>>20) : 0,stat.rx.badbytes,(stat.rx.badbytes!=0) ? (stat.rx.badbytes>>20) : 0);		*/
					test->rx_goodbytes=tmp;
		
		            if(tmp!=0)
						test->rx_goodbytes_mib=tmp>>20;
					else
						test->rx_goodbytes_mib=0;
					
					test->rx_badbytes=stat.rx.badbytes;
					
					if(stat.rx.badbytes!=0)
						test->rx_badbytes_mib=stat.rx.badbytes>>20;
					else
						test->rx_badbytes_mib=0;
						
		         
		/*/// vty_out(vty,"\tuncastpkts:%lld  bcastpkts:%lld  mcastpkts:%lld\n",  \
							//	stat.rx.uncastframes,stat.rx.bcastframes,stat.rx.mcastframes);*/
		
		     test->rx_uncastpkts=stat.rx.uncastframes;
		     test->rx_bcastpkts=stat.rx.bcastframes;
			 test->rx_mcastpkts=stat.rx.mcastframes;
			 
		/*/// vty_out(vty,"\tfcframe:%lld  fifooverruns:%lld\n",  \
							//	stat.rx.fcframe,stat.rx.fifooverruns);*/
		
		     test->rx_fcframe=stat.rx.fcframe;
		     test->rx_fifooverruns=stat.rx.fifooverruns;
			 
		/*/// vty_out(vty,"\tunderSizeframe:%lld  fragments:%lld  overSizeframe:%lld \n",  \
							//	stat.rx.underSizepkt,stat.rx.fragments,stat.rx.overSizepkt);*/
		
		  test->rx_underSizeframe=stat.rx.underSizepkt;
		  test->rx_fragments=stat.rx.fragments;
		  test->rx_overSizeframe=stat.rx.overSizepkt;
		
		/*/// vty_out(vty,"\tjabber:%lld 	errorframe:%lld\n",  \
							//	stat.rx.jabber,stat.rx.errorframe);*/

		test->rx_jabber=stat.rx.jabber;
		test->rx_errorframe=stat.rx.errorframe;
		
		
		/*/// vty_out(vty,"\tBadCrc: %lld  collision:%lld  late_collision:%lld\n",\
							//	stat.otc.badCRC,stat.otc.collision,stat.otc.late_collision);*/	

		test->rx_BadCrc=stat.otc.badCRC;
		test->rx_collision=stat.otc.collision;
		test->rx_late_collision=stat.otc.late_collision;
		
		//// vty_out(vty,"\nTX");
		/*
		vty_out(vty,"packets:%lld  errors:%lld  dropped:%lld  overruns:%lld  frame:%lld\n",  \
								stat.tx.packets,stat.tx.errors,stat.tx.dropped,stat.tx.overruns,stat.tx.frame);
		*/
		tmp = stat.tx.goodbytesh;
		tmp = (tmp<<32) | stat.tx.goodbytesl;
		//// vty_out(vty,"\tsent_deferred:%lld  goodbytes:%lld  ( %lld MiB )\n",stat.tx.sent_deferred, tmp, (tmp!=0) ? (tmp>>20) : 0);
		
        test->tx_goodbytes=tmp;
		test->tx_sent_deferred=stat.tx.sent_deferred;
		
	/*/// 	vty_out(vty,"\tuncastframe:%lld  excessiveCollision:%lld\n",  \
							//	stat.tx.uncastframe,stat.tx.excessiveCollision);*/

		test->tx_uncastframe=stat.tx.uncastframe;
		test->tx_excessiveCollision=stat.tx.excessiveCollision;
		
		/*/// vty_out(vty,"\tmcastframe:%lld  bcastframe:%lld  sentMutiple:%lld\n",  \
							//	stat.tx.uncastframe,stat.tx.mcastframe,stat.tx.bcastframe,stat.tx.sentMutiple);*/


   //奇怪的值 
   
		test->tx_uncastframe=stat.tx.uncastframe;
		//test->tx_mcastframe=stat.tx.mcastframe;
		test->tx_mcastframe=stat.tx.uncastframe;
		test->tx_bcastframe=stat.tx.bcastframe;
		test->tx_sentMutiple=stat.tx.sentMutiple;
		
		//// vty_out(vty,"\tfcframe:%lld  crcerror_fifooverrun:%lld\n",stat.tx.fcframe,stat.tx.crcerror_fifooverrun);

		test->tx_fcframe=stat.tx.fcframe;		
		test->tx_crcerror_fifooverrun=stat.tx.crcerror_fifooverrun;
		
		//// vty_out(vty,"\n");
		/*/// vty_out(vty,"\t1024tomax_frame:%lld  512to1023oct_frame:%lld  256to511oct_frame:%lld\n",  \
								//stat.otc.b1024oct2max,stat.otc.b512oct21023,stat.otc.b256oct511);*/

		test->k1024tomax=stat.otc.b1024oct2max;
		test->k512to1023=stat.otc.b512oct21023;
		test->k256to511=stat.otc.b256oct511;
		
		/*/// vty_out(vty,"\t128to255oct_frame:%lld  65to127oct_frame:%lld  64oct_frame:%lld\n",  \
							//	stat.otc.b128oct255,stat.otc.b64oct127,stat.otc.b64oct);*/
		
		//vty_out(vty,"\tlinkupcount:%d linkdowncount:%d",linkupcount,linkdowncount);		

		test->k128to255=stat.otc.b128oct255;
		test->k65to127=stat.otc.b64oct127;
		test->k64oct=stat.otc.b64oct;
		
		test->linkupcount=linkupcount;
		test->linkdowncount=linkdowncount;
		}	
		
	else if (NPD_DBUS_ERROR_NO_SUCH_PORT == op_ret) 
	{
		return WS_NO_SUCH_PORT;
	}
	else if (NPD_DBUS_ERROR == op_ret ) 
	{
		return WS_EXEC_COMM_FAIL;
	}
	dbus_message_unref(reply);

		return WS_SUCCESS;
}


//* * * * * * * * *
int ccgi_get_slot_port_by_portindex
(	
	unsigned int port_index,
	unsigned char* slot,
	unsigned char* port
)
{
	DBusMessage *query, *reply;
	DBusError err;
	int op_ret = 0;
//	unsigned int eth_g_index = 0;
	unsigned char slot_no = 0,port_no = 0;

	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
											NPD_DBUS_BUSNAME, 	\
											NPD_DBUS_ETHPORTS_OBJPATH,	\
											NPD_DBUS_ETHPORTS_INTERFACE, \
											NPD_DBUS_ETHPORTS_INTERFACE_METHOD_GET_SLOT_PORT
											);
	
		
		//DCLI_DEBUG(("the slot_no: %d	port_no: %d\n",slot_no,port_no);
		
		//tran_value=ETH_GLOBAL_INDEX_FROM_SLOT_PORT_LOCAL_NO(slot_no,port_no);
		//DCLI_DEBUG(("changed value : slot %d,port %d\n",slot_no,port_no));
		dbus_message_append_args(query,
										DBUS_TYPE_UINT32,&port_index,																			
										DBUS_TYPE_INVALID);


	 reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get re_slot_port reply.\n");
		if (dbus_error_is_set(&err)) {
		//	vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return WS_FAIL;
	}
	
	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_BYTE,&slot_no,
								DBUS_TYPE_BYTE,&port_no,
								DBUS_TYPE_INVALID)) {
		if(NPD_DBUS_SUCCESS == op_ret){
			*slot = slot_no;
			*port = port_no;
			dbus_message_unref(reply);
			return WS_SUCCESS;
		}
		else if(NPD_DBUS_ERROR == op_ret){
			//DCLI_DEBUG(("execute command failed\n");
			dbus_message_unref(reply);
			return WS_FAIL;
		}
	} else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//	vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return WS_FAIL;
	}
		return WS_SUCCESS;
}




// add by shaojunwu   2008-11-7
/*
DEFUN(config_ethport_ipg_func,
   config_ethport_ipg_cmd,
   "config minimum-ipg <0-15>",
   CONFIG_STR
   "Config ethernet port minimum inter-packet-gap\n"
   "Config inter-packet-gap value\n"
   )
*/

int set_ethport_ipg_value( char *str_port_name, char *value )
{
	DBusMessage *query, *reply;
	DBusError err;

	unsigned int port_index = 0;
	unsigned short shport_ipg = 0;
	unsigned int ret;
	unsigned char port_ipg = 0;
	int p_ret = 0;
	int iret = WS_SUCCESS;

/*	
	if(argc > 1){
//		vty_out(vty,"input too many params\n ");
		return WS_FAIL;
	}
	*/

//	vty_out(vty,"before parse value %s\n",argv[0]);
	p_ret = parse_short_parse((char*)value,&shport_ipg);
	if(NPD_SUCCESS != p_ret){
//		vty_out(vty,"parse param error\n");
		return WS_FAIL;
	}
 	port_ipg = (unsigned char )shport_ipg;
	if(/*(port_ipg < 0) ||*/ (port_ipg > 15)) {
//		vty_out(vty,"% Bad parameter.\n");
		return WS_FAIL;
	}

//	vty_out(vty,"%% get param: ret %d,port_ipg %d.\n",ret,port_ipg);
//	port_index =(unsigned int ) vty->index;
	if( (iret = ccgi_get_port_index(str_port_name, &port_index)) != WS_SUCCESS){
    	return iret;
    }

	query = dbus_message_new_method_call(	NPD_DBUS_BUSNAME,		\
										NPD_DBUS_ETHPORTS_OBJPATH ,	\
										NPD_DBUS_ETHPORTS_INTERFACE ,		\
										NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_PORT_IPG);

	dbus_error_init(&err);

	dbus_message_append_args(query,
								DBUS_TYPE_UINT32, &port_index,
								DBUS_TYPE_BYTE, &port_ipg,
								DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
//		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
//			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return WS_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32, &ret,
								DBUS_TYPE_INVALID)) {
		if (NPD_DBUS_ERROR == ret) 
		{
//			vty_out(vty,"%% execute command failed.\n"); 
			iret = WS_FAIL;
		}
		else if(NAM_ERR_HW == ret){
//			vty_out(vty,"%% port IPG configure failure on hardware.\n");
			iret = WS_FAIL;
		}
		else if (NPD_DBUS_ERROR_BAD_IPG==ret){
		{
//			DCLI_DEBUG(("port %d config ipg %d succeed\n",port_index,port_ipg));
		}
			}
	} 
	else 
	{
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	return iret;
}

/*
DEFUN(config_buffer_mode_cmd_func,
	  config_buffer_mode_cmd,
	  "config buffer mode (shared |divided)",
	  CONFIG_STR
	  "Configure buffer mode\n"
	  "Configure buffer mode enable or disable\n"
	  "Enable buffer mode \n"
	  "Disable buffer mode \n"	  
)*/
int set_port_buffer_mode( char *str_mode )
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	boolean isEnable = 0;
	unsigned int op_ret;
	if(strncmp("shared",str_mode,strlen(str_mode))==0)
	{
		isEnable = 0;
	}
	else if (strncmp("divided",str_mode,strlen(str_mode))==0)
	{
		isEnable = 1;
	}
	else
	{
		//vty_out(vty,"bad command parameter!\n");
		return WS_FAIL;
	}
		query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_BUFFER_MODE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&isEnable,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return WS_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{
			if (NPD_DBUS_SUCCESS == op_ret ) 
			{
				//vty_out(vty,"Access-list Service is %d\n",isEnable);
			}		
		
	} 
	else 
	{
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return WS_SUCCESS;
}


/*
DEFUN(show_buffer_mode_cmd_func,
	show_buffer_mode_cmd,
	"show buffer mode",
	SHOW_STR
	"Show  buffer mode\n"
	"Show  buffer mode information\n"
)*/
int get_port_buffer_mode( char *mode )
{
	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = { 0 };
	unsigned int Isable = 0;
	int iret = WS_SUCCESS;
	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH ,	\
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_BUFFER_MODE);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		return WS_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&Isable,
		DBUS_TYPE_INVALID))
	{						
		//vty_out(vty,"----------------------------\n");
		if(BUFFER_MODE_SHARED == Isable) {
		//	vty_out(vty,"Global buffer mode is shared!\n");
			strcpy( mode, "shared" );
		}
		else if(BUFFER_MODE_DIVIDED == Isable) {
		//	vty_out(vty,"Global buffer mode is divided!\n");		
			strcpy( mode, "divided" );
		}
		else {
			//vty_out(vty,"Global buffer mode is uncertain!\n");

			iret = -1;
		}
//		vty_out(vty,"----------------------------\n");
	} 
	else 
	{
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return iret;
}

//show physical port list add by wangpeng 2009-3-6 14:48:56 version 1.85

struct global_ethport_s *
show_global_ethports_info(void)
{
    
   	 int fd = -1;
	char file_path[64];	
	unsigned int slotNum = get_product_info(SLOT_COUNT_FILE);
	struct global_ethport_s *start_fp[MAX_SLOT_COUNT];
	struct global_ethport_s *temp_ethport = NULL;
	unsigned int flength = 0;
	int i, j;
	int error;

	temp_ethport = (struct global_ethport_s *)calloc(GLOBAL_ETHPORTS_MAXNUM, sizeof(struct global_ethport_s));
	if(NULL == temp_ethport) {
		return NULL;
	}

	for(i = 0; i < slotNum; i++) {
		
    		sprintf(file_path, "/dbm/shm/ethport/shm%d", i + 1);

	 	fd = open(file_path, O_RDONLY);
		if(fd < 0) {
			goto RETURN_ERR;
		}

		start_fp[i] = (struct global_ethport_s *)mmap(NULL, sizeof(struct global_ethport_s)*BOARD_GLOBAL_ETHPORTS_MAXNUM,
												PROT_READ, MAP_SHARED, fd, 0);

		if(start_fp[i] == MAP_FAILED) {
			close(fd);
			goto RETURN_ERR;
		}
		
		/*lixiang modify at 2012-03-27*/
		memcpy(&temp_ethport[i * BOARD_GLOBAL_ETHPORTS_MAXNUM], start_fp[i], sizeof(struct global_ethport_s) * BOARD_GLOBAL_ETHPORTS_MAXNUM);
		/*
		for(j = 0; j < BOARD_GLOBAL_ETHPORTS_MAXNUM; j++) {
			temp_ethport[i*BOARD_GLOBAL_ETHPORTS_MAXNUM + j] = &start_fp[i][j];
		}
		*/

		munmap(start_fp[i], sizeof(struct global_ethport_s)*BOARD_GLOBAL_ETHPORTS_MAXNUM);
		/*lixiang modify at 2012-03-27*/
		close(fd);
    }	
    return temp_ethport;

RETURN_ERR:
	free(temp_ethport);	
	return NULL;
}

/*返回0且slot_mun>0时，调用Free_ethslot_head()释放空间*/
int show_ethport_list(ETH_SLOT_LIST *head,int * slot_num)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	ETH_SLOT_LIST *q = NULL,*tail = NULL;
	ETH_PORT_LIST *p_q = NULL,*p_tail = NULL;
	unsigned char slot_count = 0;
	int i=0;
    
	if(1 == distributed_flag) {

		head->next = NULL;
		tail = head;
		
		/*lixiang modify at 2012-03-27*/
		struct global_ethport_s *global_ethport = NULL;
		global_ethport = show_global_ethports_info();
		if(NULL == global_ethport) {
			*slot_num = 0;
			return -1;
		}    

		unsigned int slotNum = get_product_info(SLOT_COUNT_FILE);
		*slot_num = slotNum;

		int i = 0;
		int slot_id = 0;
		for(i = 0; i < BOARD_GLOBAL_ETHPORTS_MAXNUM*slotNum; i++) {	
			if(global_ethport[i].isValid == VALID_ETHPORT) {
				if(slot_id != global_ethport[i].slot_no) {
					slot_id = global_ethport[i].slot_no;
					q = (ETH_SLOT_LIST *)malloc(sizeof(ETH_SLOT_LIST));
					if(NULL == q) {
						continue;
					}
					
					memset(q, 0, sizeof(ETH_SLOT_LIST));
					q->slot_no = global_ethport[i].slot_no;
					q->port_num = 0;
					q->port.next = NULL;
					p_tail = &(q->port);

					q->next = NULL;
					tail->next = q;
					tail = q;
				}

				p_q = (ETH_PORT_LIST *)malloc(sizeof(ETH_PORT_LIST));
				if(NULL == p_q) {
					continue;
				}
				
				memset(p_q, 0, sizeof(ETH_PORT_LIST));
				p_q->port_no = global_ethport[i].local_port_no;
				p_q->porttype = global_ethport[i].port_type;
				p_q->attr_map = global_ethport[i].attr_bitmap;

				time_t timer;
				timer = time(NULL);
				p_q->link_keep_time = (unsigned long)timer - global_ethport[i].lastLinkChange;

				p_q->mtu = global_ethport[i].mtu;
				p_q->next = NULL;
				p_tail->next = p_q;
				p_tail = p_q;
				q->port_num++;
			    	
			}
		}

		free(global_ethport);
		/*lixiang modify at 2012-03-27*/
	}
	else {
		
    	head->next = NULL;
    	tail = head;
    	query = dbus_message_new_method_call(
    								NPD_DBUS_BUSNAME,		\
    								NPD_DBUS_ETHPORTS_OBJPATH,		\
    								NPD_DBUS_ETHPORTS_INTERFACE,		\
    								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_LIST);
    	

    	dbus_error_init(&err);
    	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
    	
    	dbus_message_unref(query);
    	if (NULL == reply) 
    	{
    		//vty_out(vty,"failed get reply.\n");
    		if (dbus_error_is_set(&err)) 
    		{
    			//vty_out(vty,"%s raised: %s",err.name,err.message);
    			dbus_error_free(&err);
    		}
    		return -1;
    	}

    	dbus_message_iter_init(reply,&iter);
    	dbus_message_iter_get_basic(&iter,&slot_count);

    	// NOTICE We used 68 bytes here, we still got 12 bytes of summary info
    	//vty_out(vty,"%-9s%-16s%-5s%-5s%-7s%-9s%-6s%-11s\n","PORTNO","PORTTYPE","LINK","AUTO","DUPLEX","FLOWCTRL","SPEED","MTU(BYTES)");
    	
    	
    	dbus_message_iter_next(&iter);
    	
    	dbus_message_iter_recurse(&iter,&iter_array);
    	
    	*slot_num = slot_count;
    	
    	for (i = 0; i < slot_count; i++) 
    	{
    		DBusMessageIter iter_struct;
    		DBusMessageIter iter_sub_array;
    		unsigned char slotno=0;
    		unsigned char local_port_count=0;
    		int j=0;
    		

    		
    		dbus_message_iter_recurse(&iter_array,&iter_struct);
    		
    		dbus_message_iter_get_basic(&iter_struct,&slotno);
    		//vty_out(vty,"slotno %d\n",slotno);
    		
    		dbus_message_iter_next(&iter_struct);
    		dbus_message_iter_get_basic(&iter_struct,&local_port_count);
    		//vty_out(vty,"local port count %d\n",local_port_count);
    		
    		
    		dbus_message_iter_next(&iter_struct);
    		dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

    		

    		q=(ETH_SLOT_LIST *)malloc(sizeof(ETH_SLOT_LIST));
    		q->slot_no = slotno;
    		q->port_num = local_port_count;

    		q->port.next = NULL;
    		p_tail = &(q->port);
    		for (j = 0; j < local_port_count; j++) 
    		{
    			
    			DBusMessageIter iter_sub_struct;
    			unsigned char portno,porttype;
    			unsigned int attr_map,mtu;
    			unsigned int link_keep_time;
    			p_q = (ETH_PORT_LIST *)malloc(sizeof(ETH_PORT_LIST));
    	/*		
    			Array of Port Infos.
    			port no
    			port type
    			port attriute_bitmap
    			port MTU
    	*/
    			
    			dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
    			
    			dbus_message_iter_get_basic(&iter_sub_struct,&portno);
    			dbus_message_iter_next(&iter_sub_struct);
    			//vty_out(vty,"local port no %d\n",portno);
    			
    			dbus_message_iter_get_basic(&iter_sub_struct,&porttype);
    			dbus_message_iter_next(&iter_sub_struct);
    			
    			dbus_message_iter_get_basic(&iter_sub_struct,&attr_map);
    			dbus_message_iter_next(&iter_sub_struct);
    			
    			dbus_message_iter_get_basic(&iter_sub_struct,&mtu);
    			dbus_message_iter_next(&iter_sub_struct);

    			dbus_message_iter_get_basic(&iter_sub_struct,&link_keep_time);
    			dbus_message_iter_next(&iter_sub_struct);			
    			if(ETH_MAX == porttype) {				
    					dbus_message_iter_next(&iter_sub_array);
    					continue;
    				}

    			//q->port = (ETH_PORT_LIST *)malloc(sizeof(ETH_PORT_LIST));
    			p_q->port_no = portno;
    			p_q->porttype = porttype;
    			p_q->attr_map = attr_map;
    			p_q->link_keep_time = link_keep_time;
    			#if 0
    			if (1 == slot_count) 
    			{
    				vty_out(vty,"%-9d",portno);
    			}
    			else 
    			{
    				vty_out(vty,"%2d-%-2d    ",slotno,portno);
    			}
    			#endif
    			if(ETH_ATTR_LINKUP == ((attr_map & ETH_ATTR_LINK_STATUS)>>ETH_LINK_STATUS_BIT)) 
    			{
    				p_q->mtu = mtu;
    				#if 0
    				vty_out(vty,"%-16s",eth_port_type_str[porttype]);
    				vty_out(vty,"%-5s",link_status_str[(attr_map & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT]);
    				vty_out(vty,"%-5s",onoff_status_str[(attr_map & ETH_ATTR_AUTONEG) >> ETH_AUTONEG_BIT]);
    				vty_out(vty,"%-7s",duplex_status_str[(attr_map & ETH_ATTR_DUPLEX) >> ETH_DUPLEX_BIT]);
    				vty_out(vty,"%-9s",onoff_status_str[(attr_map & ETH_ATTR_FLOWCTRL) >> ETH_FLOWCTRL_BIT]);
    				vty_out(vty,"%-6s",eth_speed_str[(attr_map & ETH_ATTR_SPEED_MASK) >> ETH_SPEED_BIT]);
    				vty_out(vty,"%-11d",mtu);
    				vty_out(vty,"\n");
    				#endif
    			}
    			else 
    			{
    				p_q->mtu = 0;//if MTU equals with 0,print "-" on the web

    				#if 0
    				vty_out(vty,"%-16s",eth_port_type_str[porttype]);//port type
    				vty_out(vty,"%-5s",link_status_str[(attr_map & ETH_ATTR_LINK_STATUS) >> ETH_LINK_STATUS_BIT]);//link status
    				vty_out(vty,"%-5s","-");//auto-negotiation
    				vty_out(vty,"%-7s","-");//duplex status
    				vty_out(vty,"%-9s","-");//flow control state
    				vty_out(vty,"%-6s","-");//speed
    				vty_out(vty,"%-11s","-");//mtu
    				vty_out(vty,"\n");
    				#endif
    			}
    			dbus_message_iter_next(&iter_sub_array);
    			p_q->next = NULL;
    			p_tail->next = p_q;
    			p_tail = p_q;
    		}
    		
    		dbus_message_iter_next(&iter_array);
    		q->next = NULL;
    		tail->next = q;
    		tail = q;
    	}
    		
    	dbus_message_unref(reply);
    }
	return 0;
}
#if 0
int ccgi_show_eth_xg_port_stat
(	
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned int op_ret = 0;

	unsigned int j =0,count =0;
	unsigned int i = 0;
	
	unsigned int stat[MAX_HSP_COUNT][HSP_MIB_ITEM_COUNT_EACH] = {0};
	char *uMsgHeader = NULL, *uMsgTail = NULL;
	unsigned int product_id = 0, hsp_count =0;
	unsigned int value = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_XG_ETHPORT_STAT);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}

    dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&product_id);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&hsp_count);
	/*vty_out(vty,"dcli value op_ret %d product_id %d hsp_count %d \n",op_ret,product_id,hsp_count);*/
	
	if(CMD_SUCCESS == op_ret){	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);	
		for(i = 0; i < hsp_count; i++) {	
			for(j = 0; j < HSP_MIB_ITEM_COUNT_EACH ; j++){	
				dbus_message_iter_recurse(&iter_array,&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&value);
				dbus_message_iter_next(&iter_struct);
				stat[i][j] = value;
				/*vty_out(vty,"%d",value);*/
				dbus_message_iter_next(&iter_array);
			}
		}
	}


	switch(product_id) {
		default:
			uMsgHeader = HSP_UMSG_HEADER_ERROR;
			uMsgTail = HSP_UMSG_TAIL_ERROR;
			break;
		case PRODUCT_ID_AX7K:
			uMsgHeader = HSP_UMSG_HEADER_PRODUCT_AX7605;
			uMsgTail = HSP_UMSG_TAIL_PRODUCT_AX7605;
			break;
		case PRODUCT_ID_AX5K:
			uMsgHeader = HSP_UMSG_HEADER_PRODUCT_AX5612;
			uMsgTail = HSP_UMSG_TAIL_PRODUCT_AX5612;
			break;
		case PRODUCT_ID_AX5K_I:
			uMsgHeader = HSP_UMSG_HEADER_PRODUCT_AX5612i;
			uMsgTail = HSP_UMSG_TAIL_PRODUCT_AX5612i;
			break;
	}

	/* give out user message header */
	//vty_out(vty, "%s", uMsgHeader);
	if(hsp_count) {
		for(i = 0; i < (HSP_MIB_ITEM_COUNT_EACH/2); i++) {
			//vty_out(vty, "%-1s%-14s%-1s","|", portStatDesc[i],"|");

			/* Rx counter */
			for(j = 0; j < hsp_count; j++) {
				if(PRODUCT_ID_AX7K == product_id) {
					//vty_out(vty, "%-9d%-1s",stat[j][i],"|");
					
				}
				else {
					//vty_out(vty, "%-15d%-1s",stat[j][i],"|");
				}
			}
			
			/* Tx counter */
			for(j = 0; j < hsp_count; j++) {
				if(PRODUCT_ID_AX7K == product_id) {
				  //vty_out(vty, "%-9d%-1s",stat[j][i+(HSP_MIB_ITEM_COUNT_EACH/2)],"|");
				  if(2 == j) {
					//vty_out(vty,"    +--------------+---------+---------+---------+---------+---------+---------+");
					                
				  }
				  else{ ;}
				}
				  
			
				else {
					//vty_out(vty, "%-15d%-1s\n",stat[j][i+(HSP_MIB_ITEM_COUNT_EACH/2)],"|");
					//vty_out(vty,"+--------------+---------------+---------------+");
				}
			}
			
			//vty_out(vty, "\n");
		}
	}	
	/* give out user message tail */
	/*vty_out(vty, "%s", uMsgTail);*/
	dbus_message_unref(reply);

	return CMD_SUCCESS;
}

void Free_ethport_head(ETH_PORT_LIST * head)
{
	ETH_PORT_LIST *f1=NULL,*f2=NULL;
	if(head->next!=NULL)
	{
		f1=head->next;
		
		if(f1->next!=NULL)
		{
			f2=f1->next;
		}
		while(f2!=NULL)
		{
			free(f1);
			f1=f2;
			f2=f2->next;
		}
	 	if(f1!=NULL)
	 	{
			free(f1);
	 	}
	}
}

#endif

int show_eth_port_stat(struct eth_port_counter_s *EthStat, DBusConnection * connection,unsigned int value,unsigned char type)
{
	if(NULL == connection)
		return CCGI_FAIL;
	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned char slot_no = 0, port_no = 0;
	unsigned int op_ret = 0, i = 0;
	unsigned long long tmp = 0;
	unsigned int linkupcount = 0, linkdowncount = 0;
	struct eth_port_counter_s *ptr = NULL, stat;

	ptr = &stat;
	memset(ptr,0,sizeof(struct eth_port_counter_s));

	slot_no = (unsigned char)(((value>>6) & 0x1f) + 1);
	port_no = (unsigned char)((value & 0x3f) + 1);

	query = dbus_message_new_method_call(
								SEM_DBUS_BUSNAME,		\
								SEM_DBUS_ETHPORTS_OBJPATH,		\
								SEM_DBUS_ETHPORTS_INTERFACE,		\
								SEM_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_STAT);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&type,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
							 DBUS_TYPE_UINT32,&value,
							 DBUS_TYPE_INVALID);
	
    reply = dbus_connection_send_with_reply_and_block (connection, query, -1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CCGI_FAIL;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info
	//vty_out(vty,"here start\n");*/
	if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
	{
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for(i = 0; i < 32 ; i++)
		{
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&tmp);
			/*vty_out(vty,"tmp %d value %lld\n",i,tmp);*/
			dbus_message_iter_next(&iter_struct);
			*((unsigned long long*)ptr + i) = tmp;
			tmp = 0ULL;
			
			dbus_message_iter_get_basic(&iter_struct,&linkupcount);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&linkdowncount);
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_next(&iter_array);
		}
		memcpy(EthStat,&stat,sizeof(struct eth_port_counter_s));
    	dbus_message_unref(reply);
    	return CCGI_SUCCESS;
	}
	else if(INTERFACE_RETURN_CODE_UNSUPPORT_COMMAND == op_ret){
        dbus_message_unref(reply);
	}
	else
	{
        dbus_message_unref(reply);
	    return CCGI_FAIL;
	}
    
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_STAT);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&type,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
							 DBUS_TYPE_UINT32,&value,
							 DBUS_TYPE_INVALID);
	
    reply = dbus_connection_send_with_reply_and_block (connection, query, -1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CCGI_FAIL;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);

	/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info
	//vty_out(vty,"here start\n");*/
	if(ETHPORT_RETURN_CODE_ERR_NONE == op_ret)
	{
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for(i = 0; i < 32 ; i++)
		{
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&tmp);
			/*vty_out(vty,"tmp %d value %lld\n",i,tmp);*/
			dbus_message_iter_next(&iter_struct);
			*((unsigned long long*)ptr + i) = tmp;
			tmp = 0ULL;
			
			dbus_message_iter_get_basic(&iter_struct,&linkupcount);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&linkdowncount);
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_next(&iter_array);
		}
		/*vty_out(vty,"here end\n");*/
		

		/*if(0 == type)
			vty_out(vty,"Detail Information of Ethernet Port %d-%d\n",slot_no,port_no);
		else {
			dcli_get_slot_port_by_portindex(vty,value, &slot_no,&port_no);
			vty_out(vty,"Detail Information of Ethernet Port %d-%d\n",slot_no,port_no);
		}*/
		memcpy(EthStat,&stat,sizeof(struct eth_port_counter_s));	
	}
	else {
		dbus_message_unref(reply);
		return CCGI_FAIL;
	}
	dbus_message_unref(reply);

	return CCGI_SUCCESS;		
}


#if 0
/*add by tsq 2010-4-9 */
int show_eth_port_statistic(struct eth_port_counter_s *stat, unsigned char slot, unsigned char port)
{
	int ret = 0;
	unsigned char slot_no = 0,port_no = 0;
	unsigned char type = 0;
	unsigned int value = 0;

	slot_no = slot;
	port_no = port;
	
	value = slot_no;
	value = (value << 8) |port_no;
	
	ret = show_eth_port_stat(stat,value,type);

	return ret;
}
#endif


void Free_ethslot_head(ETH_SLOT_LIST * head)
{
	ETH_SLOT_LIST *f1,*f2;
	ETH_PORT_LIST *pf1,*pf2;
	int i;
	f1=head->next;
	f2=f1->next;
	while(f2!=NULL)
	{
		pf1=f1->port.next;
		if(pf1!=NULL)
		{
			pf2=pf1->next;
			for(i=0;i<f1->port_num;i++)
			{
				free(pf1);
				pf1=pf2;
				if(pf2!=NULL)
					pf2=pf2->next;
			}
		}		
		free(f1);
		f1=f2;
		f2=f2->next;
	}
	pf1=f1->port.next;
	if(pf1!=NULL)
	{
		pf2=pf1->next;
		for(i=0;i<f1->port_num;i++)
		{
			free(pf1);
			pf1=pf2;
			if(pf2!=NULL)
				pf2=pf2->next;
		}
	}
	free(f1);
}


int count_eth_port_num()
{
	ETH_SLOT_LIST  head,*p;
	int ret,total_num=0,num=0;

	ccgi_dbus_init();
	ret=show_ethport_list(&head,&num);
	p=head.next;
	while(p!=NULL)
	{
		total_num+=p->port_num;
		p=p->next;
	}

	if((ret==0)&&(num>0))
	{
		Free_ethslot_head(&head);
	}
	return total_num;
}


///dcli_eth_port_mode_config version add

int ccgi_eth_port_mode_config
(
    unsigned char slot_no,
    unsigned char port_no,
    unsigned int mode
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int op_ret = NPD_DBUS_ERROR;

	//vty_out(vty,"FDB agingtime %d. \n",agingtime);
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_ETHPORT_MODE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
								DBUS_TYPE_BYTE,&slot_no,
								DBUS_TYPE_BYTE,&port_no,
							 	DBUS_TYPE_UINT32,&mode,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return WS_FAIL;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if(NPD_DBUS_SUCCESS == op_ret){
	            dbus_message_unref(reply);
				//sleep(1);
				return WS_SUCCESS;
			}
			else if(NPD_DBUS_ERROR_NO_SUCH_PORT == op_ret){
				//vty_out(vty,"%% NO SUCH PORT %d/%d!\n",slot_no,port_no);
			}
			else if(DCLI_ETH_PORT_ALREADY_RUN_THIS_MODE == op_ret){
                if(ETH_PORT_FUNC_BRIDGE == mode){
                   // vty_out(vty,"%% Port interface not exists!\n");
                   return WS_FAIL;
				}
				else if(ETH_PORT_FUNC_IPV4 == mode){
					//vty_out(vty,"%% Advanced-routing already disabled!\n");
					return WS_FAIL;
				}
				else if(ETH_PORT_FUNC_MODE_PROMISCUOUS == mode){
					//vty_out(vty,"%% Advanced-routing already enabled!\n");
					return WS_FAIL;
				}
			}
			else if(DCLI_ETH_PORT_HAVE_SUBIF == op_ret){
                //vty_out(vty,"%% Please delete sub interface first!\n");
                return WS_FAIL;
			}
			else if((DCLI_ETH_PORT_ALREADY_IN_L3_VLAN == op_ret) || \
				(DCLI_DEFAULT_VLAN_IS_L3_VLAN == op_ret)){
				//vty_out(vty,"%% Port mode change fail,conflict with vlan interface\n");
				return WS_FAIL;
			}
			else if(NPD_DBUS_ERROR_UNSUPPORT == op_ret)
				//vty_out(vty,"%% Unsupport this command\n");
				return WS_FAIL;
			else
				//vty_out(vty,"%% Execute command failed\n");
				return WS_FAIL;
			
	}
	else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return op_ret;

}
#if 0
//add v1.116
unsigned int ccgi_eth_port_config_sc_common
(
	unsigned char modeType,
    unsigned int g_index,
    unsigned int scMode,
    unsigned int scType,
    unsigned int scValue,
    unsigned int* ret
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int op_ret = 0;
	unsigned char slotno = 0,portno = 0;

	if(0 == modeType){
		slotno = (unsigned char)((g_index>>8)&0xff);
		portno = (unsigned char)(g_index & 0xff);
		g_index = 0xffff; 
	}
	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_ETHPORTS_OBJPATH , \
							NPD_DBUS_ETHPORTS_INTERFACE ,		\
							NPD_DBUS_ETHPORTS_INTERFACE_METHOD_CONFIG_STORM_CONTROL);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&modeType,
							 DBUS_TYPE_BYTE,&slotno,
							 DBUS_TYPE_BYTE,&portno,
							 DBUS_TYPE_UINT32,&g_index,
							 DBUS_TYPE_UINT32,&scMode,
							 DBUS_TYPE_UINT32,&scType,
							 DBUS_TYPE_UINT32,&scValue,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {	
		return DCLI_ERROR_DBUS;
	}

	else if (!dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) {
		dbus_message_unref(reply);
	    return DCLI_ERROR_DBUS;
	}
	*ret = op_ret;
	dbus_message_unref(reply);
	return DCLI_ERROR_NONE;

}
//add v1.116
unsigned int ccgi_global_config_sc_common
(
    unsigned int scType,
    unsigned int* ret
)
{
    DBusMessage *query, *reply;
	DBusError err;

	unsigned int op_ret = 0;
	
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_STORM_CONTROL_GLOBAL_MODEL);


	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&scType,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
	   *ret = 0;
       return DCLI_ERROR_DBUS;
	}

	else if (! dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) 
	{
		*ret = 0;
		return DCLI_ERROR_DBUS;

	} 
    *ret = op_ret;
	dbus_message_unref(reply);
    return DCLI_ERROR_NONE;
}
#endif

#ifdef __cplusplus
}
#endif
