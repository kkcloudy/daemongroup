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
* dcli_tunnel.c
*
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		CLI definition for ASIC diagnosis configuration.
*
* DATE:
*		05/15/2009	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.10 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include <string.h>
#include <zebra.h>
#include <dbus/dbus.h>

#include "sysdef/npd_sysdef.h"
#include "dbus/npd/npd_dbus_def.h"

#include "command.h"
#include "if.h"

#include "util/npd_list.h"
#include "npd/nam/npd_amapi.h"

#include "dcli_tunnel.h"
#include "sysdef/returncode.h"

extern DBusConnection *dcli_dbus_connection;

DEFUN(config_add_ip_tunnel_cmd_func,
		config_add_ip_tunnel_cmd,
		"ip tunnel add remote A.B.C.D/M local A.B.C.D/M PORTNO",
		"Config ip command\n"
		"Config ip tunnel information\n"
		"Add remote ip\n"
		"Remote ip\n"
		"IP format: A.B.C.D/M "
		"Local ip\n"
		"IP format: A.B.C.D/M "
	    CONFIG_ETHPORT_STR
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	unsigned long	dipaddr = 0, sipaddr = 0;
	unsigned int	sipmaskLen = 0, dipmaskLen = 0, ret = 0, op_ret = 0,Val=0;
	unsigned char   mslot=0,mport=0;
	
	Val = ip_address_format2ulong((char**)&argv[0],&dipaddr,&dipmaskLen);			
	if(CMD_WARNING == Val) {
		vty_out(vty, "%% Bad parameter %s\n", argv[0]);
		return CMD_WARNING;
	}
	/*VALUE_IP_MASK_CHECK(dipmaskLen);*/
					
	Val = ip_address_format2ulong((char**)&argv[1],&sipaddr,&sipmaskLen); 	
	if(CMD_WARNING == Val) {
		vty_out(vty, "%% Bad parameter %s\n", argv[1]);
		return CMD_WARNING;
	}
	/*VALUE_IP_MASK_CHECK(sipmaskLen);*/
	
	ret = parse_slotport_no((char *)argv[2],&mslot,&mport);	
	 if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
		 vty_out(vty,"%% Unknow portno format.\n");
		 return CMD_SUCCESS;
	 }

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,		\
										   NPD_DBUS_OBJPATH,		\
										   NPD_DBUS_INTERFACE,	\ 
										   NPD_DBUS_IP_TUNNEL_ADD);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&sipmaskLen,
							DBUS_TYPE_UINT32,&dipmaskLen,
							DBUS_TYPE_UINT32,&dipaddr,
							DBUS_TYPE_UINT32,&sipaddr,
							DBUS_TYPE_BYTE,  &mslot,
							DBUS_TYPE_BYTE,  &mport,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if (TUNNEL_RETURN_CODE_SUCCESS == op_ret ) {
				/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));*/
			}		
			else if(2 == op_ret) {
				vty_out(vty, "%%   %d  !\n",(1+1));
			}
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
				vty_out(vty,"%% Product not support this function!\n");
			}
			else {
				vty_out(vty, "%% set fail!\n");
			}
	} 
	else {
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;	
}

DEFUN(config_delete_ip_tunnel_cmd_func,
		config_delete_ip_tunnel_cmd,
		"ip tunnel delete remote A.B.C.D/M local A.B.C.D/M PORTNO",
	    "Config ip command\n"
	    "Config ip tunnel information\n"
	    "Delete remote ip\n"
	    "Remote ip\n"
	    "IP format: A.B.C.D/M "
	    "Local ip\n"
	    "IP format: A.B.C.D/M "
	    CONFIG_ETHPORT_STR

)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err = { 0 };
	unsigned long	dipaddr = 0, sipaddr = 0;
	unsigned int	sipmaskLen = 0, dipmaskLen = 0, ret = 0, op_ret = 0,Val=0;
	unsigned char   mslot = 0, mport = 0;
	
	Val = ip_address_format2ulong((char**)&argv[0],&dipaddr,&dipmaskLen);			
	if(CMD_WARNING == Val) {
		vty_out(vty, "%% Bad parameter %s\n", argv[0]);
		return CMD_WARNING;
	}
	/*VALUE_IP_MASK_CHECK(dipmaskLen);*/
					
	Val = ip_address_format2ulong((char**)&argv[1],&sipaddr,&sipmaskLen); 	
	if(CMD_WARNING == Val) {
		vty_out(vty, "%% Bad parameter %s\n", argv[1]);
		return CMD_WARNING;
	}
	/*VALUE_IP_MASK_CHECK(sipmaskLen);*/
	
	ret = parse_slotport_no((char *)argv[2],&mslot,&mport);	
	 if (TUNNEL_RETURN_CODE_SUCCESS != ret) {
		 vty_out(vty,"%% Unknow portno format.\n");
		 return CMD_SUCCESS;
	 }

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,		\
										   NPD_DBUS_OBJPATH,		\
										   NPD_DBUS_INTERFACE,	\ 
										   NPD_DBUS_IP_TUNNEL_DELETE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&sipmaskLen,
							DBUS_TYPE_UINT32,&dipmaskLen,
							DBUS_TYPE_UINT32,&dipaddr,
							DBUS_TYPE_UINT32,&sipaddr,
							DBUS_TYPE_BYTE,  &mslot,
							DBUS_TYPE_BYTE,  &mport,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if (TUNNEL_RETURN_CODE_SUCCESS == op_ret ) {
				/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));*/
			}		
			else if(2 == op_ret) {
				vty_out(vty, "%%   %d  !\n",(1+1));
			}
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
				vty_out(vty,"%% Product not support this function!\n");
			}
			else {
				vty_out(vty, "%% set fail!\n");
			}
	} 
	else {
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;	
}

DEFUN(config_ip_tunnel_host_add_cmd_func,
		config_ip_tunnel_host_add_cmd,
		"ip tunnel host add remote A.B.C.D/M local A.B.C.D/M dip A.B.C.D/M",
		"Config ip command\n"
		"Config ip tunnel information\n"
		"Host port\n"
		"Add remote ip\n"
		"Remote ip\n"
		"IP format: A.B.C.D/M "
		"Local ip\n"
		"IP format: A.B.C.D/M "
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	unsigned long	ripaddr = 0, lipaddr = 0, dipaddr = 0;
	unsigned int	ripmaskLen = 0, lipmaskLen = 0, dipmaskLen = 0, ret = 0, op_ret = 0,Val=0;
	
	Val = ip_address_format2ulong((char**)&argv[0],&ripaddr,&ripmaskLen);			
	if(CMD_WARNING == Val) {
		vty_out(vty, "%% Bad parameter %s\n", argv[0]);
		return CMD_WARNING;
	}
	/*VALUE_IP_MASK_CHECK(dipmaskLen);*/
					
	Val = ip_address_format2ulong((char**)&argv[1],&lipaddr,&lipmaskLen); 	
	if(CMD_WARNING == Val) {
		vty_out(vty, "%% Bad parameter %s\n", argv[1]);
		return CMD_WARNING;
	}
	/*VALUE_IP_MASK_CHECK(lipmaskLen);*/
	
	Val = ip_address_format2ulong((char**)&argv[2],&dipaddr,&dipmaskLen); 	
	if(CMD_WARNING == Val) {
		vty_out(vty, "%% Bad parameter %s\n", argv[2]);
		return CMD_WARNING;
	}
	/*VALUE_IP_MASK_CHECK(lipmaskLen);*/

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,		\
										   NPD_DBUS_OBJPATH,		\
										   NPD_DBUS_INTERFACE,	\ 
										   NPD_DBUS_IP_TUNNEL_HOST_ADD);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&ripmaskLen,
							DBUS_TYPE_UINT32,&lipmaskLen,
							DBUS_TYPE_UINT32,&dipmaskLen,
							DBUS_TYPE_UINT32,&ripaddr,
							DBUS_TYPE_UINT32,&lipaddr,							
							DBUS_TYPE_UINT32,&dipaddr,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if (TUNNEL_RETURN_CODE_SUCCESS == op_ret ) {
				/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));*/
			}		
			else if(2 == op_ret) {
				vty_out(vty, "%%  %d  !\n",(1+1));
			}
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
				vty_out(vty,"%% Product not support this function!\n");
			}
			else {
				vty_out(vty, "%% set fail!\n");
			}
	} 
	else {
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;	
}

DEFUN(config_ip_tunnel_host_delete_cmd_func,
		config_ip_tunnel_host_delete_cmd,
		"ip tunnel host delete remote A.B.C.D/M local A.B.C.D/M dip A.B.C.D/M",
		"Config ip command\n"
		"Config ip tunnel information\n"
		"Host port\n"
		"Delete remote ip\n"
		"Remote ip\n"
		"IP format: A.B.C.D/M "
		"Local ip\n"
		"IP format: A.B.C.D/M "
)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	unsigned long	ripaddr = 0, lipaddr = 0, dipaddr = 0;
	unsigned int	ripmaskLen = 0, lipmaskLen = 0, dipmaskLen = 0, ret = 0, op_ret = 0,Val=0;
	
	Val = ip_address_format2ulong((char**)&argv[0],&ripaddr,&ripmaskLen);			
	if(CMD_WARNING == Val) {
		vty_out(vty, "%% Bad parameter %s\n", argv[0]);
		return CMD_WARNING;
	}
	/*VALUE_IP_MASK_CHECK(dipmaskLen);*/
					
	Val = ip_address_format2ulong((char**)&argv[1],&lipaddr,&lipmaskLen); 	
	if(CMD_WARNING == Val) {
		vty_out(vty, "%% Bad parameter %s\n", argv[1]);
		return CMD_WARNING;
	}
	/*VALUE_IP_MASK_CHECK(lipmaskLen);*/
	
	Val = ip_address_format2ulong((char**)&argv[2],&dipaddr,&dipmaskLen); 	
	if(CMD_WARNING == Val) {
		vty_out(vty, "%% Bad parameter %s\n", argv[2]);
		return CMD_WARNING;
	}
	/*VALUE_IP_MASK_CHECK(lipmaskLen);*/

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,		\
										   NPD_DBUS_OBJPATH,		\
										   NPD_DBUS_INTERFACE,	\ 
										   NPD_DBUS_IP_TUNNEL_HOST_DELETE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&ripmaskLen,
							DBUS_TYPE_UINT32,&lipmaskLen,
							DBUS_TYPE_UINT32,&dipmaskLen,
							DBUS_TYPE_UINT32,&ripaddr,
							DBUS_TYPE_UINT32,&lipaddr,							
							DBUS_TYPE_UINT32,&dipaddr,
							DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if (TUNNEL_RETURN_CODE_SUCCESS == op_ret ) {
				/*vty_out(vty,"Access-list %d is successfully set!\n",(ruleIndex+1));*/
			}		
			else if(2 == op_ret) {
				vty_out(vty, "%%  %d  !\n",(1+1));
			}
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
				vty_out(vty,"%% Product not support this function!\n");
			}
			else {
				vty_out(vty, "%% set fail!\n");
			}
	} 
	else {
		/*vty_out(vty,"Failed get args.\n");*/
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;	
}

DEFUN(Diagnosis_tunnel_show_cmd_func,
		Diagnosis_tunnel_show_cmd,
		"show ip tunnel",
		SHOW_STR
		"Ip command\n"
		"Show ip tunnel\n"
) 
{
	unsigned int opDevice = 0, portnum = 0;
	char *showStr = NULL,*cursor = NULL,ch = 0;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	query = dbus_message_new_method_call(  
										NPD_DBUS_BUSNAME,		\
										NPD_DBUS_OBJPATH,			\
										NPD_DBUS_INTERFACE, 		\
										NPD_DBUS_IP_TUNNEL_SHOW_TAB);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
						DBUS_TYPE_UINT32, &opDevice,
						DBUS_TYPE_UINT32, &portnum,
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return;
	}
	
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) {
		vty_out(vty, "%s ", showStr);
	} 
	else {
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	
	dbus_message_unref(reply);
	return; 
}


void dcli_tunnel_init(void) {
	install_element(HIDDENDEBUG_NODE, &config_add_ip_tunnel_cmd);
	install_element(HIDDENDEBUG_NODE, &config_delete_ip_tunnel_cmd);
	install_element(HIDDENDEBUG_NODE, &config_ip_tunnel_host_add_cmd);
	install_element(HIDDENDEBUG_NODE, &config_ip_tunnel_host_delete_cmd);
	install_element(HIDDENDEBUG_NODE, &Diagnosis_tunnel_show_cmd);
}

#ifdef __cplusplus
}
#endif

