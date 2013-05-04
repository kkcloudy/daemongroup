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
* ws_pvlan.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* tangsq@autelan.com
*
* DESCRIPTION:
* function for web
*
*
***************************************************************************/
/********************************************************************************/
/*dcli_pvlan.h V1.1*/
/*dcli_pvlan.c V1.4*/
/*author qiandawei*/
/*update time 08-8-27*/

#ifdef __cplusplus
extern "C"
{
#endif
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <dbus/dbus.h>

#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>


#include "ws_pvlan.h"
#include "ws_usrinfo.h"
#include "util/npd_list.h"
#include "npd/nam/npd_amapi.h"
#include "npd/nbm/npd_bmapi.h"
#include "ws_returncode.h"

/*#include "cgic.h"*/

extern FILE *cgiOut;
extern DBusConnection *ccgi_dbus_connection;

int is_enable_or_disable(char * str)
{

  if(!(strcmp(str,"enable")))
  		return ENABLE;
  else if(!(strcmp(str,"disable")))
  		return DISABLE;
  else 
  		return PVE_ERR;
  
}

int parse_char_param(char* str,unsigned char* shot){
	char *endptr = NULL;

	if (NULL == str) return NPD_FAIL;
	*shot= strtoul(str,&endptr,10);
	return NPD_SUCCESS;	
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
		if (NPD_FAIL == ret) {
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
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS != ret ) {
				vty_out(vty,"creat PVE err !\n");
			}
			
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
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
	if (NPD_FAIL == ret) {
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
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS != ret ) {
				vty_out(vty,"add PVE port err !\n");
			}
			
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
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
	if (NPD_FAIL == ret) {
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
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS != ret ) {
				vty_out(vty,"delete PVE port err !\n");
			}
			
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
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

    cmp =  is_enable_or_disable((char *) argv[0]);
	if(cmp == PVE_ERR){
		vty_out(vty,"confirm cpuport add or delete erro\n");
		return 	CMD_SUCCESS;
		}
	
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
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS != ret ) {
				vty_out(vty,"config PVE cpuport err !\n");
			}
			
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
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
*DEFUN(config_pvlan_port_to_port_cmd_func,
	config_pvlan_port_to_port_cmd,
	"config pvlan port PORTNO uplink PORTNO ",
	"Config port attribute \n"
	"config pvlan port \n"
	"config port number \n"
	"config port No.eg: 4/6 means slot 4# port 6#\n"
	"config uplink port \n"
	"config port No.eg: 4/6 means slot 4# port 6#\n"
)
****************************************/

int add_config_pvlan(char *port, char *up_port, char *create_mode)
{
	DBusMessage 		*query, *reply;
	DBusError 			err;
	unsigned int 		ret;
	unsigned char 		src_slot = 0,src_port = 0,dest_slot = 0,dest_port = 0;
	unsigned int         mode = PVE_MODE_SINGLE_E;

	
	ret = parse_slotport_no((char *)port, &src_slot, &src_port);
	if (NPD_FAIL == ret) {
    	//vty_out(vty,"%% input pvlan port error.\n");    	
		return -2;
	}

    
	ret = parse_slotport_no((char *)up_port, &dest_slot, &dest_port);
	if (NPD_FAIL == ret) {
    	//vty_out(vty,"%% input uplink port error.\n");    	
		return -3;
	}

	if((src_slot == dest_slot) && (src_port == dest_port)) {
		//vty_out(vty,"%% pvlan port and uplink port must be different.\n");
		return -4;
	}
	
	if(0 == strncmp("single",create_mode,strlen(create_mode))) {
		mode = PVE_MODE_SINGLE_E;
	}
	else if(0 == strncmp("multiple",create_mode,strlen(create_mode))) {
		mode = PVE_MODE_MULTI_E;
	}
	else {
		//vty_out(vty,"%% input bad mode\n");
		return -1;
	}
	
	query = dbus_message_new_method_call(		\
									NPD_DBUS_BUSNAME,		\
									NPD_DBUS_PVE_OBJPATH,		\
									NPD_DBUS_PVE_INTERFACE,		\
									NPD_DBUS_PVE_METHOD_CONFIG_PVE_PORT);
	
	dbus_error_init(&err);
	//printf("src_slot/src_port %d/%d  dest_slot/dest_port %d/%d\n",src_slot,src_port,dest_slot,dest_port);
	dbus_message_append_args(	query,
							 	DBUS_TYPE_BYTE,&src_slot,
							 	DBUS_TYPE_BYTE,&src_port,
							 	DBUS_TYPE_BYTE,&dest_slot,
							 	DBUS_TYPE_BYTE,&dest_port,
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
		return -1;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS == ret ) {
				return 0;
			}
			else if (NPD_DBUS_ERROR_NO_SUCH_PORT== ret ) {
				//vty_out(vty,"%% no such ports.\n");
				return -10;
			}
			else if(NPD_DBUS_ERROR == ret){ 
				//vty_out(vty,"%% config PVE port err !\n");
				return -5;
			}
			else if(PVE_ERROR_PVE_UPLINK_PORT_NOT_IN_SAME_VLAN == ret){
				//vty_out(vty,"%% all the vlans that ports took part in must be same.\n");
				return -6;
			}
			else if(PVE_ERROR_PVE_PORT_NOT_IN_ONLY_ONE_VLAN == ret) {
				//vty_out(vty,"%% the pvlan port must only be in one vlan and pvlan port and uplink port must be in same vlan.\n");
				return -7;
			}
			else if(PVE_AVOID_CYCLE_UPLINK == ret) {
				//vty_out(vty,"%% PVE port %d/%d have been uplink port.\n",src_slot,src_port);
				return -8;
			}
			else if(PVE_ERROR_THIS_PORT_HAVE_PVE == ret) {
				//vty_out(vty,"%% port %d/%d have been PVE port.\n",src_slot,src_port);
				return -9;
			}
	} else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	dbus_message_unref(reply);
	return 0;
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
/*
DEFUN(config_pvlan_port_to_spi_cmd_func,
	config_pvlan_port_to_spi_cmd,
	"config pvlan port PORTNO spi uplink DEVSLOT ",
	"Config port attribute \n"
	"config pvlan port \n"
	"config port number \n"
	"config port No.eg: 4/6 means slot 4# port 6#\n"
	"config uplink spi \n"
	"config uplink devslot \n"
	"config devslot No.eg: 2/1 means dev 4# port 6#\n"
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
	if (NPD_FAIL == ret) {
		vty_out(vty,"%% input PORTNO1 form erro!\n");
		return CMD_SUCCESS;
	}

	
	ret = parse_slotport_no((char *)argv[1], &slot_no2, &port_no2);
	if (NPD_FAIL == ret) {
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
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS == ret ) {
				;
			}
			else if (NPD_DBUS_ERROR_NO_SUCH_PORT== ret ) {
				vty_out(vty,"%% no such ports.\n");
			}
			else if(NPD_DBUS_ERROR == ret) {
				vty_out(vty,"%% config PVE port err !\n");
			}
			else {
				;
			}
			
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

*/

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
*DEFUN(delete_pvlan_port_to_port_cmd_func,
	delete_pvlan_port_to_port_cmd,
	"delete pvlan port PORTNO ",
	"delete port attribute \n"
	"delete pvlan port \n"
	"config port number \n"
	"config port No.eg: 4/6 means slot 4# port 6#\n"
)
****************************************/

int pvlan_delete(char *port)
{
	DBusMessage 		*query, *reply;
	DBusError 			err;
	unsigned int 		ret;
	unsigned char 		slot_no1,port_no1;
	
	ret = parse_slotport_no((char *)port, &slot_no1, &port_no1);
	if (NPD_FAIL == ret) {
    	//vty_out(vty,"input PORTNO1 form erro!\n");
		return -2;
	}
	

	//fprintf(stderr,"11111111111");
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_PVE_OBJPATH,NPD_DBUS_PVE_INTERFACE,NPD_DBUS_PVE_METHOD_DELETE_PVE_PORT);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_BYTE,&slot_no1,
							 	DBUS_TYPE_BYTE,&port_no1,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_ERROR_NO_SUCH_PORT== ret ) {
				//vty_out(vty,"the portno is not exist!\n");
				return -4;
			}
			else if(NPD_DBUS_SUCCESS==ret)
			{
				//fprintf(stderr,"22222222222");
			}
			else{
				//vty_out(vty,"delete portno happened err!\n");
				//vty_out(vty,"ret value is %d\n",ret);
				return -1;
				}
			
	} else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	dbus_message_unref(reply);
	return 0;
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
/*
DEFUN(config_pvlan_port_to_uplink_trunk_cmd_func,
	config_pvlan_port_to_trunk_cmd,
	"config pvlan port PORTNO uplink <1-127> ",
	"config system information \n"
	"config pvlan port attribute\n"
	"config port number \n"
	"config port No.eg: 4/6 means slot 4# port 6#\n"
	"config uplink port \n"
	"config trunkid range <1-127> \n"
)
{
	DBusMessage 		*query, *reply;
	DBusError 			err;
	unsigned int 		ret;
	unsigned char 		slot_no1,port_no1;
	unsigned char 		trunkid;
	unsigned int 		i=0;
	
	ret = parse_slotport_no((char *)argv[0], &slot_no1, &port_no1);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"input PORTNO1 form erro!\n");
		return CMD_SUCCESS;
	}
	if ((slot_no1<MIN_SLOT || slot_no1>MAX_SLOT)||(port_no1<MIN_PORT ||port_no1>MAX_PORT)){
		vty_out(vty,"PORTNO1 outrange!\n");
		return CMD_SUCCESS;
		}
    
	ret = parse_char_param((char *)argv[1], &trunkid);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"input TRUNK form erro!\n");
		return CMD_SUCCESS;
	}
	if ((trunkid<MIN_TRUNK)|| (trunkid>MAX_TRUNK)){
		vty_out(vty,"TRUNK outrange!\n");
		return CMD_SUCCESS;
		}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_PVE_OBJPATH,NPD_DBUS_PVE_INTERFACE,NPD_DBUS_PVE_METHOD_CONFIG_PVE_TRUNK);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_BYTE,&slot_no1,
							 	DBUS_TYPE_BYTE,&port_no1,
							 	DBUS_TYPE_BYTE,&trunkid,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS == ret ) {
				;
			}
			else if (NPD_DBUS_ERROR_NO_SUCH_PORT== ret ) {
				vty_out(vty,"config PVE porte exist !\n");
			}
			else {vty_out(vty,"config PVE porte err !\n");
				vty_out(vty,"ret value is %d\n",ret);
				}
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
*/

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
/*
DEFUN(config_pvlan_port_control_deal_cmd_func,
	config_pvlan_port_control_deal_cmd,
	"config pvlan port PORTNO tocpu (enable|disable)",
	"Config system information\n"
	"config pvlan port \n"
	"config port number \n"
	"config port No.eg: 4/6 means slot 4# port 6#\n"
	"config port traffic to cpu \n"
	"choose enble \n"
	"choose disable \n"
)
{
	DBusMessage 		*query, *reply;
	DBusError 			err;
	unsigned int 		ret;
	unsigned char 		slot_no1,port_no1;
	int 				cmp;

	ret = parse_slotport_no((char *)argv[0], &slot_no1, &port_no1);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"input PORTNO1 form erro!\n");
		return CMD_SUCCESS;
	}
	if ((slot_no1<MIN_SLOT || slot_no1>MAX_SLOT)||(port_no1<MIN_PORT ||port_no1>MAX_PORT)){
		vty_out(vty,"PORTNO1 outrange!\n");
		return CMD_SUCCESS;
		}
    cmp =  is_enable_or_disable((char *) argv[1]);
	if(cmp == PVE_ERR){
		vty_out(vty,"confirm port enable or disable erro\n");
		return 	CMD_SUCCESS;
		}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_PVE_OBJPATH,NPD_DBUS_PVE_INTERFACE,NPD_DBUS_PVE_METHOD_CONFIG_PVE_CONTROL);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT32,&cmp,
							 	DBUS_TYPE_BYTE,&slot_no1,
							 	DBUS_TYPE_BYTE,&port_no1,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS != ret ) {
				vty_out(vty,"config PVE port err !\n");
				vty_out(vty,"ret value is %d\n",ret);
			}
			
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

*/
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
*DEFUN(show_pvlan_port_to_uplink_port_cmd_func,
	show_pvlan_port_to_port_cmd,
	"show pvlan",
	"show system infomation \n"
	"show pvlan port \n"
)
****************************************/

int show_pvlan(struct Pvlan_Info * Pinfo, int * pNum)
{
	DBusMessage 		*query, *reply;
	DBusError 			err;
	DBusMessageIter		iter_array,iter;
	struct Pvlan_Info *q,*tail;
	unsigned int		i=0;
	unsigned int 		count=0;
	unsigned short      tkflag=0;
	unsigned char 		pvslot=0,pvport=0,lkslot=0,lkport=0;
	unsigned int mode = PVE_MODE_SINGLE_E;
	
	query = dbus_message_new_method_call(   \
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_PVE_OBJPATH,		\
								NPD_DBUS_PVE_INTERFACE,		\
								NPD_DBUS_PVE_METHOD_SHOW_PVE);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&count);
	*pNum=count;
	if (count == 0){
		//vty_out(vty,"PVE TABLE IS NONE!\n");
		return -2;
		}
	//vty_out(vty," =======  =======  =======  =======  =======\n");
	//vty_out(vty," %-7s  %-7s  %-7s  %-7s  %-7s\n","PSLOT","PPORT","USLOT","UPORT","UTRUNK");
	//vty_out(vty," =======  =======  =======  =======  =======\n");

	Pinfo->next = NULL;
	tail=Pinfo;
	
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
		q=(struct Pvlan_Info*)malloc(sizeof(struct Pvlan_Info));

		if(tkflag==PVE_FAIL){
				//vty_out(vty,"  %-7d  %-7d  %-7d  %-7d  %-7s\n", pvslot,pvport,lkslot,lkport,"-");
				/*fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
				fprintf(cgiOut,"<td align=left>%d</td>",pvslot);
				fprintf(cgiOut,"<td align=left>%d</td>",pvport);
				fprintf(cgiOut,"<td align=left>%d</td>",lkslot);
				fprintf(cgiOut,"<td align=left>%d</td>",lkport);
				fprintf(cgiOut,"<td align=left>%s</td>",(PVE_MODE_SINGLE_E == mode) ? "single  " : "multiple");
				fprintf(cgiOut,"<td align=left>%s</td>","-");
				fprintf(cgiOut,"</tr>");*/
				q->pslot=pvslot;
          		q->pport=pvport;
          		q->lsort=lkslot;
          		q->lport=lkport;
          		if(PVE_MODE_SINGLE_E == mode)
          			q->mode=1;
          		else q->mode=2;
          		q->trunk=50;

			}
		else {
				//vty_out(vty,"  %-7d  %-7d  %-7s  %-7s  %-7d\n", pvslot,pvport,"-","-",lkslot);
				/*fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));		
				fprintf(cgiOut,"<td align=left>%d</td>",pvslot);
				fprintf(cgiOut,"<td align=left>%d</td>",pvport);
				fprintf(cgiOut,"<td align=left>%s</td>","-");
				fprintf(cgiOut,"<td align=left>%s</td>","-");
				fprintf(cgiOut,"<td align=left>%s</td>","-");
				fprintf(cgiOut,"<td align=left>%d</td>",lkslot);
				fprintf(cgiOut,"</tr>");*/

				q->pslot=pvslot;
          		q->pport=pvport;
          		q->lsort=50;
          		q->lport=50;
          		q->mode=3;
          		q->trunk=lkslot;

			}
		q->next=NULL;
		tail->next=q;
		tail=q;
		dbus_message_iter_next(&iter_array);
		}
	
	dbus_message_unref(reply);

	return 0;	
}


int set_delete_pvlan()
{
	DBusMessage 		*query, *reply;
	DBusError 			err;
	DBusMessageIter		iter_array,iter;

	unsigned int		i=0;
	unsigned int 		count=0;
	unsigned short      tkflag=0;
	unsigned char 		pvslot=0,pvport=0,lkslot=0,lkport=0;
	unsigned int mode = PVE_MODE_SINGLE_E;
	
	
	query = dbus_message_new_method_call(   \
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_PVE_OBJPATH,		\
								NPD_DBUS_PVE_INTERFACE,		\
								NPD_DBUS_PVE_METHOD_SHOW_PVE);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&count);
	if (count == 0){
		//vty_out(vty,"PVE TABLE IS NONE!\n");
		return -2;
		}
	//vty_out(vty," =======  =======  =======  =======  =======\n");
	//vty_out(vty," %-7s  %-7s  %-7s  %-7s  %-7s\n","PSLOT","PPORT","USLOT","UPORT","UTRUNK");
	//vty_out(vty," =======  =======  =======  =======  =======\n");
	fprintf(cgiOut,"<tr height=25 bgcolor=#eaeff9 style=font-size:14px align=left>");
	fprintf(cgiOut,"<th width=30>&nbsp;</th>");
	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","PSLOT");
	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","PPORT");
	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","USLOT");
	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","UPORT");
	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","MODE");
	fprintf(cgiOut,"<th  style=font-size:12px>%s</th>","UTRUNK");
	fprintf(cgiOut,"</tr>");
	int cl = 1;

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
				//vty_out(vty,"  %-7d  %-7d  %-7d  %-7d  %-7s\n", pvslot,pvport,lkslot,lkport,"-");
				fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));
				fprintf(cgiOut,"<td><input type=checkbox name=del_pvlan value=%d/%d></td>",(unsigned int)pvslot,(unsigned int)pvport);
				fprintf(cgiOut,"<td align=left>%d</td>",pvslot);
				fprintf(cgiOut,"<td align=left>%d</td>",pvport);
				fprintf(cgiOut,"<td align=left>%d</td>",lkslot);
				fprintf(cgiOut,"<td align=left>%d</td>",lkport);
				fprintf(cgiOut,"<td align=left>%s</td>",(PVE_MODE_SINGLE_E == mode) ? "single  " : "multiple");
				fprintf(cgiOut,"<td align=left>%s</td>","-");
				fprintf(cgiOut,"</tr>");
			}
		else {
				//vty_out(vty,"  %-7d  %-7d  %-7s  %-7s  %-7d\n", pvslot,pvport,"-","-",lkslot);
				fprintf(cgiOut,"<tr height=25 bgcolor=%s>",setclour(cl));
				fprintf(cgiOut,"<td><input type=checkbox name=del_pvlan value=%d/%d></td>",(unsigned int)pvslot,(unsigned int)pvport);
				fprintf(cgiOut,"<td align=left>%d</td>",pvslot);
				fprintf(cgiOut,"<td align=left>%d</td>",pvport);
				fprintf(cgiOut,"<td align=left>%s</td>","-");
				fprintf(cgiOut,"<td align=left>%s</td>","-");
				fprintf(cgiOut,"<td align=left>%s</td>","-");
				fprintf(cgiOut,"<td align=left>%d</td>",lkslot);
				fprintf(cgiOut,"</tr>");
			}
		dbus_message_iter_next(&iter_array);
		cl = !cl;
			}
	
	dbus_message_unref(reply);

	return 0;	
}


void Free_list(struct Pvlan_Info * head,int pvlannum)
{
	struct Pvlan_Info * f1,* f2;
	if(pvlannum>0)
	{
    	f1=head->next;		 
      	f2=f1->next;
      	while(f2!= NULL)
      	{
      		free(f1);
      		f1=f2;
      		f2=f2->next;
      	}
  	}
}

/*
void dcli_pvlan_show_running_cfg()
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
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) 
	{
		vtysh_add_show_string(showStr);
	} 
	else 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}

	dbus_message_unref(reply);
	return;	
}

void dcli_pvlan_init() {
	install_element(CONFIG_NODE, &config_pvlan_port_to_port_cmd);
	install_element(CONFIG_NODE, &delete_pvlan_port_to_port_cmd);
	install_element(CONFIG_NODE, &config_pvlan_port_to_trunk_cmd);
	install_element(CONFIG_NODE, &config_pvlan_port_control_deal_cmd);
	install_element(CONFIG_NODE, &show_pvlan_port_to_port_cmd);
	install_element(CONFIG_NODE, &config_pvlan_port_to_spi_cmd);

#if 0
	install_element(CONFIG_NODE, &create_pvlan_cmd);
	install_element(CONFIG_NODE, &config_pvlan_add_port_cmd);
	install_element(CONFIG_NODE, &config_pvlan_delete_port_cmd);
#endif

}
*/
#ifdef __cplusplus
}
#endif


