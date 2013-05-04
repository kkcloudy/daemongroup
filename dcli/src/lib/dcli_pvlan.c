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
* dcli_pvlan.c
*
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		CLI definition for PVLAN module.
*
* DATE:
*		02/21/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.19 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include <ctype.h>
#include <string.h>
#include <zebra.h>
#include <dbus/dbus.h>

#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>

#include "command.h"
#include "if.h"
#include "npd/nbm/npd_bmapi.h"
#include "util/npd_list.h"
#include "npd/nam/npd_amapi.h"
#include "dcli_pvlan.h"
#include "dcli_main.h"
#include "sysdef/returncode.h"


extern DBusConnection *dcli_dbus_connection;

int parse_char_param(char* str,unsigned char* shot){
	char *endptr = NULL;

	if (NULL == str) return PVLAN_RETURN_CODE_ERROR;
	*shot= strtoul(str,&endptr,10);
	return PVLAN_RETURN_CODE_SUCCESS;	
}

/**************************************
* config PVE enable or disable on Both Sw & Hw. 
*
* Usage: config pvlan port PORTNO (enable|disable) uplink PORTNO
*
*input Params: 
*      PORTNO:config the pvlan port (form slot/pot for example 1/4) enable or disable pvlan.
*      (enable|disable):enable config the port enble pvlan attrbute,reverse disable.
*      PORTNO:config the uplink port (form slot/pot for example 1/4) enable or disable pvlan. 
*
*output params: none
*
****************************************/


#if 0
DEFUN(create_pvlan_cmd_func,
	create_pvlan_cmd,
	"config pvlan pvlanid <0-31>",
	"Config system info \n"
	"config pvlan vlan \n"
	"config pvlan vlanid \n"
	"vlanid value range <0-31>,default 0\n"
	
)
{
	DBusMessage 		*query, *reply;
	DBusError 			err;
	unsigned int 		ret;
	unsigned short      pvlanid;
	unsigned int 		change=0;
	
	ret = parse_short_parse((char *)argv[0], &pvlanid);
		if (PVLAN_RETURN_CODE_SUCCESS != ret) {
			vty_out(vty,"input pvlanid form erro!\n");
			return CMD_SUCCESS;
		}
		if ((pvlanid<MIN_PVLAN)|| (pvlanid>MAX_PVLAN)){
			vty_out(vty,"pvlanid outrange!\n");
			return CMD_SUCCESS;
			}

	change = (unsigned int)pvlanid;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_PVE_OBJPATH,NPD_DBUS_PVE_INTERFACE,NPD_DBUS_PVE_METHOD_CREATE_PVE);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_UINT32,&change,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {
			if (PVLAN_RETURN_CODE_SUCCESS != ret ) {
				vty_out(vty,"creat PVE err !\n");
			}
			
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/**************************************
* config PVE enable or disable on Both Sw & Hw. 
*
* Usage: config pvlan port PORTNO (enable|disable) uplink PORTNO
*
*input Params: 
*      PORTNO:config the pvlan port (form slot/pot for example 1/4) enable or disable pvlan.
*      (enable|disable):enable config the port enble pvlan attrbute,reverse disable.
*      PORTNO:config the uplink port (form slot/pot for example 1/4) enable or disable pvlan. 
*
*output params: none
*
****************************************/

DEFUN(config_pvlan_add_port_cmd_func,
	config_pvlan_add_port_cmd,
	"add pvlan port PORTNO>",
	"Config system info \n"
	"config pvlan vlan \n"
	"config add port\n"
	"config port No.eg: 4/6 means slot 4# port 6#\n"
)
{
	DBusMessage 		*query, *reply;
	DBusError 			err;
	unsigned int 		ret;
	unsigned char       slot_no1,port_no1;
	unsigned int 		change=0;
	
	ret = parse_slotport_no((char *)argv[0], &slot_no1, &port_no1);
	if (PVLAN_RETURN_CODE_SUCCESS != ret) {
    	vty_out(vty,"input PORTNO1 form erro!\n");
		return CMD_SUCCESS;
	}
	if ((slot_no1<MIN_SLOT || slot_no1>MAX_SLOT)||(port_no1<MIN_PORT ||port_no1>MAX_PORT)){
		vty_out(vty,"PORTNO1 outrange!\n");
		return CMD_SUCCESS;
		}

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_PVE_OBJPATH,NPD_DBUS_PVE_INTERFACE,NPD_DBUS_PVE_METHOD_ADD_PORT);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_BYTE,&slot_no1,
							 	DBUS_TYPE_BYTE,&port_no1,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {
			if (PVLAN_RETURN_CODE_SUCCESS != ret ) {
				vty_out(vty,"add PVE port err !\n");
			}
			
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/**************************************
* config PVE enable or disable on Both Sw & Hw. 
*
* Usage: config pvlan port PORTNO (enable|disable) uplink PORTNO
*
*input Params: 
*      PORTNO:config the pvlan port (form slot/pot for example 1/4) enable or disable pvlan.
*      (enable|disable):enable config the port enble pvlan attrbute,reverse disable.
*      PORTNO:config the uplink port (form slot/pot for example 1/4) enable or disable pvlan. 
*
*output params: none
*
****************************************/

DEFUN(config_pvlan_delete_port_cmd_func,
	config_pvlan_delete_port_cmd,
	"delete pvlan port protected PORTNO",
	"Config system info \n"
	"config pvlan vlan \n"
	"config delete port\n"
	"config pvlan base srcid filter\n"
	"config port No.eg: 4/6 means slot 4# port 6#\n"
)
{
	DBusMessage 		*query, *reply;
	DBusError 			err;
	unsigned int 		ret;
	unsigned char       slot_no1,port_no1;
	unsigned int 		change=0;
	
	ret = parse_slotport_no((char *)argv[0], &slot_no1, &port_no1);
	if (PVLAN_RETURN_CODE_SUCCESS != ret) {
    	vty_out(vty,"input PORTNO1 form erro!\n");
		return CMD_SUCCESS;
	}
	if ((slot_no1<MIN_SLOT || slot_no1>MAX_SLOT)||(port_no1<MIN_PORT ||port_no1>MAX_PORT)){
		vty_out(vty,"PORTNO1 outrange!\n");
		return CMD_SUCCESS;
		}

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_PVE_OBJPATH,NPD_DBUS_PVE_INTERFACE,NPD_DBUS_PVE_METHOD_DELETE_PORT);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_BYTE,&slot_no1,
							 	DBUS_TYPE_BYTE,&port_no1,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {
			if (PVLAN_RETURN_CODE_SUCCESS != ret ) {
				vty_out(vty,"delete PVE port err !\n");
			}
			
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/**************************************
* config PVE enable or disable on Both Sw & Hw. 
*
* Usage: config pvlan port PORTNO (enable|disable) 
*
*input Params: 
*      PORTNO:config the pvlan port (form slot/pot for example 1/4) enable or disable pvlan.
*      (enable|disable):enable config the port enble pvlan attrbute,reverse disable.
*      
*
*output params: none
*
****************************************/

DEFUN(config_pvlan_cpu_port_add_cmd_func,
	config_pvlan_cpu_port_add_cmd,
	"config pvlan cpuport (enable|disable)",
	"Config port attribute \n"
	"pvlan port \n"
	"config port number \n"
	"config port No.eg: 4/6 means slot 4# port 6#\n"
	"choose enble or disable port pvlan\n"
	"config destrunk \n"
	"config trunk id \n"
)
{
	DBusMessage 		*query, *reply;
	DBusError 			err;
	unsigned int 		ret;
	unsigned char 		slot_no1,port_no1;
	int 				cmp;

    if(strncmp("enable",argv[0],strlen(argv[0]))==0)
        IsEnable = 0;
    else if(strncmp("disable",argv[0],strlen(argv[0]))==0)
        IsEnable =1 ;

	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_PVE_OBJPATH,NPD_DBUS_PVE_INTERFACE,NPD_DBUS_PVE_METHOD_CONFIG_PVE_CPUPORT);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32,&cmp,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {
			if (PVLAN_RETURN_CODE_SUCCESS != ret ) {
				vty_out(vty,"config PVE cpuport err !\n");
			}
			
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
#endif


/**************************************
* config PVE enable or disable on Both Sw & Hw. 
*
* Usage: config pvlan port PORTNO  uplink PORTNO
*
*input Params: 
*      PORTNO:config the pvlan port (form slot/pot for example 1/4) 
*     
*      PORTNO:config the uplink port (form slot/pot for example 1/4) pvlan. 
*
*output params: none
*
****************************************/

DEFUN(config_pvlan_port_to_port_cmd_func,
	config_pvlan_port_to_port_cmd,
	"config pvlan port PORTNO uplink PORTNO mode (single|multiple)",
	"Config port attribute \n"
	"Config pvlan port \n"
	"Config port number \n"
	CONFIG_ETHPORT_STR
	"Config uplink port \n"
	CONFIG_ETHPORT_STR
	"Pvlan work mode\n"
	"Pvlan work in single vlan\n"
	"Pvlan work in multiple vlans\n"
)
{
	DBusMessage 		*query, *reply;
	DBusError 			err;
	unsigned int 		ret;
	unsigned char 		src_slot = 0,src_port = 0,dest_slot = 0,dest_port = 0;
	unsigned int 		i=0;
	unsigned int         mode = PVE_MODE_SINGLE_E;
	ret = parse_slotport_no((char *)argv[0], &src_slot, &src_port);
	if (PVLAN_RETURN_CODE_SUCCESS != ret) {
    	vty_out(vty,"%% input pvlan port error.\n");
		return CMD_SUCCESS;
	}

    
	ret = parse_slotport_no((char *)argv[1], &dest_slot, &dest_port);
	if (PVLAN_RETURN_CODE_SUCCESS != ret) {
    	vty_out(vty,"%% input uplink port error.\n");
		return CMD_SUCCESS;
	}

	if((src_slot == dest_slot) && (src_port == dest_port)) {
		vty_out(vty,"%% pvlan port and uplink port must be different.\n");
		return CMD_WARNING;
	}
	
	if(0 == strncmp("single",argv[2],strlen(argv[2]))) {
		mode = PVE_MODE_SINGLE_E;
	}
	else if(0 == strncmp("multiple",argv[2],strlen(argv[2]))) {
		mode = PVE_MODE_MULTI_E;
	}
	else {
		vty_out(vty,"%% input bad mode\n");
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(		\
									NPD_DBUS_BUSNAME,		\
									NPD_DBUS_PVE_OBJPATH,		\
									NPD_DBUS_PVE_INTERFACE,		\
									NPD_DBUS_PVE_METHOD_CONFIG_PVE_PORT);
	
	dbus_error_init(&err);
	/*printf("src_slot/src_port %d/%d  dest_slot/dest_port %d/%d\n",src_slot,src_port,dest_slot,dest_port);*/
	dbus_message_append_args(	query,
							 	DBUS_TYPE_BYTE,&src_slot,
							 	DBUS_TYPE_BYTE,&src_port,
							 	DBUS_TYPE_BYTE,&dest_slot,
							 	DBUS_TYPE_BYTE,&dest_port,
							 	DBUS_TYPE_UINT32,&mode,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {
			if (PVLAN_RETURN_CODE_SUCCESS == ret ) {
				;
			}
			else if (NPD_DBUS_ERROR_NO_SUCH_PORT== ret ) {
				vty_out(vty,"%% no such ports.\n");
			}
			else if(PVLAN_RETURN_CODE_ERROR == ret){ 
				vty_out(vty,"%% config PVE port error.\n");
			}
			else if(PVLAN_RETURN_CODE_UPLINK_PORT_NOT_IN_SAME_VLAN == ret){
				vty_out(vty,"%% all the vlans that ports took part in must be same.\n");
			}
			else if(PVLAN_RETURN_CODE_PORT_NOT_IN_ONLY_ONE_VLAN == ret) {
				vty_out(vty,"%% the pvlan port must only be in one vlan and pvlan port and uplink port must be in same vlan.\n");
			}
			else if(PVLAN_RETURN_CODE_AVOID_CYCLE_UPLINK == ret) {
				vty_out(vty,"%% PVE port %d/%d have been uplink port.\n",src_slot,src_port);
			}
			else if(PVLAN_RETURN_CODE_THIS_PORT_HAVE_PVE == ret) {
				vty_out(vty,"%% port %d/%d have been PVE port.\n",src_slot,src_port);
			}
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
				vty_out(vty,"%% Product not support this function!\n");
			}
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/**************************************
* config PVE enable or disable on Both Sw & Hw. 
*
* Usage: config pvlan port PORTNO spi uplink PORTNO
*
*input Params: 
*      PORTNO:config the pvlan port (form slot/pot for example 1/4) 
*      spi  config other cheet asic.
*      PORTNO:config the uplink port (form dev/pot for example 1/4) pvlan. 
*
*output params: none
*
****************************************/

DEFUN(config_pvlan_port_to_spi_cmd_func,
	config_pvlan_port_to_spi_cmd,
	"config pvlan port PORTNO spi uplink DEVSLOT ",
	"Config port attribute \n"
	"Config pvlan port \n"
	"Config port number \n"
	CONFIG_ETHPORT_STR
	"Config uplink spi \n"
	"Config uplink devslot \n"
	CONFIG_ETHPORT_STR
)
	{
		DBusMessage 		*query, *reply;
		DBusError			err;
		unsigned int		ret;
		unsigned char		slot_no1=0;
		unsigned char 		port_no1=0;
		unsigned char		slot_no2=0;
		unsigned char 		port_no2=0;
		unsigned int		i=0;
		
		ret = parse_slotport_no((char *)argv[0], &slot_no1, &port_no1);
		if (PVLAN_RETURN_CODE_SUCCESS != ret) {
			vty_out(vty,"%% input PORTNO1 form erro!\n");
			return CMD_SUCCESS;
		}

		
		ret = parse_slotport_no((char *)argv[1], &slot_no2, &port_no2);
		if (PVLAN_RETURN_CODE_SUCCESS != ret) {
			vty_out(vty,"%% input PORTNO1 form erro!\n");
			return CMD_SUCCESS;
		}
		
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_PVE_OBJPATH,NPD_DBUS_PVE_INTERFACE,NPD_DBUS_PVE_METHOD_CONFIG_PVE_SPI_PORT);
		
		dbus_error_init(&err);
	
		dbus_message_append_args(	query,
									DBUS_TYPE_BYTE,&slot_no1,
									DBUS_TYPE_BYTE,&port_no1,
									DBUS_TYPE_BYTE,&slot_no2,
									DBUS_TYPE_BYTE,&port_no2,
									DBUS_TYPE_INVALID);
		
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		if (NULL == reply) {
			vty_out(vty,"failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}
	
		if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&ret,
			DBUS_TYPE_INVALID)) {
				if (PVLAN_RETURN_CODE_SUCCESS == ret ) {
					;
				}
				else if (NPD_DBUS_ERROR_NO_SUCH_PORT== ret ) {
					vty_out(vty,"%% no such ports.\n");
				}
				else if(PVLAN_RETURN_CODE_ERROR == ret) {
					vty_out(vty,"%% config PVE port err !\n");
				}
				else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
					vty_out(vty,"%% Product not support this function!\n");
				}
				else {
					;
				}
				
		} else {
			vty_out(vty,"Failed get args.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
		}
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}



/**************************************
* config PVE enable or disable on Both Sw & Hw. 
*
* Usage: delete pvlan port PORTNO
*
*input Params: 
*      PORTNO:config the pvlan port (form slot/pot for example 1/4) disable pvlan on the port.
*     
*
*output params: none
*
****************************************/

DEFUN(delete_pvlan_port_to_port_cmd_func,
	delete_pvlan_port_to_port_cmd,
	"delete pvlan port PORTNO ",
	"Delete port attribute \n"
	"Delete pvlan port \n"
	"Config port number \n"
	CONFIG_ETHPORT_STR
)
{
	DBusMessage 		*query, *reply;
	DBusError 			err;
	unsigned int 		ret;
	unsigned char 		slot_no = 0,port_no = 0;
	unsigned int 		i=0;
	
	ret = parse_slotport_no((char *)argv[0], &slot_no, &port_no);
	if (PVLAN_RETURN_CODE_SUCCESS != ret) {
    	vty_out(vty,"%% input param error!\n");
		return CMD_SUCCESS;
	}
	
	query = dbus_message_new_method_call(	    \
						NPD_DBUS_BUSNAME,		\
						NPD_DBUS_PVE_OBJPATH,		\
						NPD_DBUS_PVE_INTERFACE,		\
						NPD_DBUS_PVE_METHOD_DELETE_PVE_PORT);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_BYTE,&slot_no,
							 	DBUS_TYPE_BYTE,&port_no,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_ERROR_NO_SUCH_PORT== ret ) {
				vty_out(vty,"%% no such port!\n");
			}
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
				vty_out(vty,"%% Product not support this function!\n");
			}
			else if(PVLAN_RETURN_CODE_SUCCESS==ret){;}
			else{
				vty_out(vty,"%% execute command error!\n");
			}
			
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}



/**************************************
* config PVE enable or disable on Both Sw & Hw. 
*
* Usage: config pvlan port PORTNO (enable|disable) uplink TRUNK
*
*input Params: 
*      PORTNO:config the pvlan port (form slot/pot for example 1/4) enable pvlan on the port.
*      
*      <0-127>:config the uplink trunkid ,range from 0 to 127. 
*
*output params: none
*
****************************************/

DEFUN(config_pvlan_port_to_uplink_trunk_cmd_func,
	config_pvlan_port_to_trunk_cmd,
	"config pvlan port PORTNO uplink <1-127> ",
	"Config system information \n"
	"Config pvlan port attribute\n"
	"Config port number \n"
	CONFIG_ETHPORT_STR
	"Config uplink port \n"
	"Trunk ID range in <1-127> \n"
)
{
	DBusMessage 		*query, *reply;
	DBusError 			err;
	unsigned int 		ret;
	unsigned char 		slot_no,port_no;
	unsigned char 		trunkid;
	unsigned int 		i=0;
	
	ret = parse_slotport_no((char *)argv[0], &slot_no, &port_no);
	if (PVLAN_RETURN_CODE_SUCCESS != ret) {
    	vty_out(vty,"input PORTNO1 form erro!\n");
		return CMD_SUCCESS;
	}
	if ((slot_no<MIN_SLOT || slot_no>MAX_SLOT)||(port_no<MIN_PORT ||port_no>MAX_PORT)){
		vty_out(vty,"PORTNO1 outrange!\n");
		return CMD_SUCCESS;
		}
    
	ret = parse_char_param((char *)argv[1], &trunkid);
	if (PVLAN_RETURN_CODE_ERROR == ret) {
    	vty_out(vty,"input TRUNK form erro!\n");
		return CMD_SUCCESS;
	}
	if ((trunkid<MIN_TRUNK)|| (trunkid>MAX_TRUNK)){
		vty_out(vty,"TRUNK outrange!\n");
		return CMD_SUCCESS;
		}
	
	query = dbus_message_new_method_call(		\
									NPD_DBUS_BUSNAME,		\
									NPD_DBUS_PVE_OBJPATH,		\
									NPD_DBUS_PVE_INTERFACE,		\
									NPD_DBUS_PVE_METHOD_CONFIG_PVE_TRUNK);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_BYTE,&slot_no,
							 	DBUS_TYPE_BYTE,&port_no,
							 	DBUS_TYPE_BYTE,&trunkid,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_ERROR_NO_SUCH_PORT== ret ) {
				vty_out(vty,"config PVE porte exist !\n");
			}
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
				vty_out(vty,"%% Product not support this function!\n");
			}
			else {vty_out(vty,"config PVE porte err !\n");
				vty_out(vty,"ret value is %d\n",ret);
			}
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


/**************************************
* config PVE enable or disable on Both Sw & Hw. 
*
* Usage: config pvlan port PORTNO (enable|disable) 
*
*input Params: 
*      PORTNO:config the pvlan port (form slot/pot for example 1/4) enable or disable pvlan.
*      (enable|disable):enable config the port enable pvlan attrbute,reverse disable.
*      
*
*output params: none
*
****************************************/

DEFUN(config_pvlan_port_control_deal_cmd_func,
	config_pvlan_port_control_deal_cmd,
	"config pvlan port PORTNO tocpu (enable|disable)",
	"Config system information\n"
	"Config pvlan port \n"
	"Config port number \n"
	CONFIG_ETHPORT_STR
	"Config port traffic to CPU \n"
	"Choose enble \n"
	"Choose disable \n"
)
{
	DBusMessage 		*query, *reply;
	DBusError 			err;
	unsigned int 		ret;
	unsigned char 		slot_no,port_no;
	int 				cmp;

	ret = parse_slotport_no((char *)argv[0], &slot_no, &port_no);
	if (PVLAN_RETURN_CODE_SUCCESS != ret) {
    	vty_out(vty,"input PORTNO1 form erro!\n");
		return CMD_SUCCESS;
	}
	/*if ((slot_no1<MIN_SLOT || slot_no1>MAX_SLOT)||(port_no1<MIN_PORT ||port_no1>MAX_PORT)){
		vty_out(vty,"PORTNO1 outrange!\n");
		return CMD_SUCCESS;
		}*/
	if(strncmp("enable",argv[1],strlen(argv[1]))==0) {
		cmp = 1;
	}
	else if(strncmp("disable",argv[1],strlen(argv[1]))==0) {
		cmp = 0 ;
	}

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
		                                 NPD_DBUS_PVE_OBJPATH,
		                                 NPD_DBUS_PVE_INTERFACE,
		                                 NPD_DBUS_PVE_METHOD_CONFIG_PVE_CONTROL);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32,&cmp,
							 	DBUS_TYPE_BYTE,&slot_no,
							 	DBUS_TYPE_BYTE,&port_no,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {

		   if  (NPD_DBUS_ERROR_NO_SUCH_PORT == ret){
		        vty_out(vty,"Error:Illegal %d/%d,No such port.\n",slot_no,port_no);
		   	}
		   if  (NPD_DBUS_NOT_SUPPORT ==ret){
                vty_out(vty,"Error:Illegal Not  support this operate.\n");
		   	}
		   if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
				vty_out(vty,"%% Product not support this function!\n");
			}
		   else if ((PVLAN_RETURN_CODE_SUCCESS != ret)&&(NPD_DBUS_ERROR_NO_SUCH_PORT != ret)&&(NPD_DBUS_NOT_SUPPORT !=ret) ) {
				vty_out(vty,"config PVE port err !\n");
				vty_out(vty,"ret value is %d\n",ret);
			}
			
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


/**************************************
* config PVE enable or disable on Both Sw & Hw. 
*
* Usage: show pvlan 
*
*input Params: 
*      none
*      
*
*output params:
*	PSLOT     - pvlan slot.
*	PPORT    - pvlan port.
*	USLOT     - uplink slot to which accepting from pvlan slot/port. 
*    UPORT    - uplink port to which accepting from pvlan slot/port. 
*    UTRUNK  - uplink trunk to which accepting from pvlan slot/port. 
*
****************************************/

DEFUN(show_pvlan_port_to_uplink_port_cmd_func,
	show_pvlan_port_to_port_cmd,
	"show pvlan",
	"show system infomation \n"
	"show pvlan port \n"
)
{
	DBusMessage 		*query, *reply;
	DBusError 			err;
	DBusMessageIter		iter_array,iter;
	unsigned int 		ret = 0;
	unsigned int		i=0;
	unsigned int 		count = 0;
	unsigned short      tkflag=0;
	unsigned char 		pvslot = 0,pvport = 0,lkslot = 0,lkport = 0;
	unsigned int mode = PVE_MODE_SINGLE_E;
	
	
	query = dbus_message_new_method_call(   \
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_PVE_OBJPATH,		\
								NPD_DBUS_PVE_INTERFACE,		\
								NPD_DBUS_PVE_METHOD_SHOW_PVE);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&count);
	if (count == 0){
		vty_out(vty,"%% pve table is none.\n");
		return CMD_SUCCESS;
		}
	vty_out(vty," =======  =======  =======  =======  =======  =======\n");
	vty_out(vty," %-7s  %-7s  %-7s  %-7s  %-7s  %-7s\n","PSLOT","PPORT","USLOT","UPORT","MODE","UTRUNK");
	vty_out(vty," =======  =======  =======  =======  =======  =======\n");
	dbus_message_iter_next(&iter);
	
	dbus_message_iter_recurse(&iter,&iter_array);
	for ( ; count > i; i++){
		DBusMessageIter iter_struct;
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&tkflag);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&pvslot);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&pvport);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&lkslot);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&lkport);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&mode);
		dbus_message_iter_next(&iter_struct);
		if(tkflag==PVE_FAIL){
				vty_out(vty,"  %-7d  %-7d  %-7d  %-7d  %-7s  %-7s\n", 	\
					pvslot,pvport,lkslot,lkport,
					(PVE_MODE_SINGLE_E == mode) ? "single  " : "multiple",
					"-");
			}
		else {
				vty_out(vty,"  %-7d  %-7d  %-7s  %-7s  %-7s  %-7d\n", pvslot,pvport,"-","-","-",lkslot);
			}
		dbus_message_iter_next(&iter_array);
		
			}
	
	dbus_message_unref(reply);

	return CMD_SUCCESS;	
}

int dcli_pvlan_show_running_cfg(struct vty *vty)
{	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_PVE_OBJPATH,		\
								NPD_DBUS_PVE_INTERFACE,		\
								NPD_DBUS_PVE_METHOD_SHOW_RUNNING_CFG);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show pvlan running config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
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
			sprintf(_tmpstr,BUILDING_MOUDLE,"PVLAN");
			vtysh_add_show_string(_tmpstr);
			vtysh_add_show_string(showStr);
	} 
	else 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return 1;
	}

	dbus_message_unref(reply);
	return 0;	
}
	struct cmd_node pvlan_node = 
	{
		PVLAN_NODE,
		" ",
		1
	};

void dcli_pvlan_init() {
	install_node(&pvlan_node,dcli_pvlan_show_running_cfg,"PVLAN_NODE");
	install_element(CONFIG_NODE, &config_pvlan_port_to_port_cmd);
	install_element(CONFIG_NODE, &delete_pvlan_port_to_port_cmd);
	/*install_element(CONFIG_NODE, &config_pvlan_port_to_trunk_cmd);*/
	install_element(CONFIG_NODE, &config_pvlan_port_control_deal_cmd);
	install_element(CONFIG_NODE, &show_pvlan_port_to_port_cmd);
	/*install_element(CONFIG_NODE, &config_pvlan_port_to_spi_cmd);*/

#if 0
	install_element(CONFIG_NODE, &create_pvlan_cmd);
	install_element(CONFIG_NODE, &config_pvlan_add_port_cmd);
	install_element(CONFIG_NODE, &config_pvlan_delete_port_cmd);
#endif

}

#ifdef __cplusplus
}
#endif


