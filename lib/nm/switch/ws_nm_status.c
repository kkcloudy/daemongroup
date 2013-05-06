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

#ifdef __cplusplus
extern "C"
{
#endif

#include "ws_nm_status.h"

#include "ws_dbus_def.h"
#include "ws_dbus_list_interface.h"
#include "ws_returncode.h"

#define SLOT_PORT_SPLIT_DASH 		'-'
#define SLOT_PORT_SPLIT_SLASH		'/'

char *slot_status_str[MODULE_STAT_MAX] = {
	"NONE",
	"INITING",
	"RUNNING",
	"DISABLED"
};

char *eth_port_type_str[ETH_MAX] = {
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

char *link_status_str[2] = {
	"DOWN",
	"UP"
};


char *doneOrnot_status_str[2] = {
	"Incomplete",
	"Completed"
};


char *onoff_status_str[2] = {
	"-",
	"ON"
};


char *duplex_status_str[2] = {
	"FULL",
	"HALF"
};

char *eth_speed_str[ETH_ATTR_SPEED_MAX] = {
	"10M",
	"100M",
	"1000M",
	"10G",
	"12G",
	"2.5G",
	"5G"
};

char *eth_media_str[3] = {
	"NONE",
	"FIBER",
	"COPPER"
};

/*dcli_system.c*/
int nm_show_hw_config(int sno, struct slot *SlotV) {  
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned char slot_count = 0;
	int i;
	unsigned int product_id = PRODUCT_ID_NONE;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_OBJPATH,			\
								NPD_DBUS_INTERFACE,NPD_DBUS_INTERFACE_METHOD_HWCONF);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	fprintf(stderr,"wwwwwwwwwwwwwwwwwwww\n");
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CCGI_FAIL;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&slot_count);
	//dbus_message_iter_next(&iter);
	//dbus_message_iter_get_basic(&iter,&product_id);

	
	dbus_message_iter_next(&iter);
	
	dbus_message_iter_recurse(&iter,&iter_array);
	for (i = 0; i < slot_count; i++) {
		DBusMessageIter iter_struct;
		unsigned char slotno;
		unsigned int module_id;
		unsigned char module_status;
		unsigned char hw_ver;
		unsigned char ext_slot_num;
		char *sn;
		char *modname;		
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		
		dbus_message_iter_get_basic(&iter_struct,&slotno);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&module_id);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&module_status);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&hw_ver);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&ext_slot_num);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&sn);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&modname);
		
		dbus_message_iter_next(&iter_array); 
		if(i==sno) {
		  SlotV->module_status=module_status;
		  strcpy(SlotV->modname,modname);
		  strcpy(SlotV->sn,sn);
		  SlotV->hw_ver=hw_ver;
		  SlotV->ext_slot_num=ext_slot_num;
		  SlotV->module_id=module_id;
		  
		  break;
		}		
	}	

	dbus_message_unref(reply);	
	return CCGI_SUCCESS;
}

/*dcli_eth_port.c v1.116*/
int show_eth_port_atrr(unsigned int value,unsigned char type,struct global_ethport_s *PortV)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned char slot_no = 0,port_no = 0;
	struct eth_port_s ethPortInfo;
	unsigned int attr_map = 0;
	unsigned int op_ret = 0,link_keep_time = 0;;
	
	memset(PortV,0,sizeof(struct global_ethport_s));
	 
	if(0 == type){
		slot_no = (unsigned char)((value>>8)&0xff);
		port_no = (unsigned char)(value & 0xff);
		value = 0xffff; 
	}

	if ( 1 == distributed_flag)
	{
		struct global_ethport_s *global_ethport = NULL;
		global_ethport = show_global_ethports_info();
		if(NULL == global_ethport) {
		    return -1;
		}    
		
		unsigned int eth_g_index = value;
		slot_no = global_ethport[eth_g_index].slot_no;
		port_no = global_ethport[eth_g_index].local_port_no;
		
		PortV->port_type = global_ethport[eth_g_index].port_type;
		PortV->attr_bitmap = global_ethport[eth_g_index].attr_bitmap;
		PortV->mtu = global_ethport[eth_g_index].mtu;
		PortV->lastLinkChange = global_ethport[eth_g_index].lastLinkChange;
		PortV->linkchanged = global_ethport[eth_g_index].linkchanged;

		free(global_ethport);
		return CCGI_SUCCESS;

	}



	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,	\
								NPD_DBUS_ETHPORTS_INTERFACE,	\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_ETHPORT_ATTR);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&type,
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
		return CCGI_FAIL;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32, &op_ret,
		DBUS_TYPE_UINT32,&(ethPortInfo.port_type),
		DBUS_TYPE_UINT32,&(ethPortInfo.attr_bitmap),
		DBUS_TYPE_UINT32,&(ethPortInfo.mtu),
		DBUS_TYPE_UINT32,&(link_keep_time),	
		DBUS_TYPE_INVALID)) {
  
			if (ETHPORT_RETURN_CODE_ERR_NONE == op_ret ) {
				attr_map = ethPortInfo.attr_bitmap;

                PortV->port_type=ethPortInfo.port_type;
				PortV->attr_bitmap=attr_map;
				PortV->mtu=ethPortInfo.mtu;
			}
			else 
			  return CCGI_FAIL;
	}
	else
	  return CCGI_FAIL;

	dbus_message_unref(reply);

	return CCGI_SUCCESS;
}

/*dcli_system.c*/
int show_sys_envir(struct sys_envir *SysV)   /*显示风扇状态及CPU温度*/
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned char fan_power=0;
	unsigned short core_tmprt=0;
	unsigned short surface_tmprt=0;
	int ret = CCGI_SUCCESS;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_OBJPATH,NPD_DBUS_INTERFACE,NPD_DBUS_SYSTEM_SHOW_STATE);
	
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CCGI_FAIL;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_BYTE,&fan_power,
		DBUS_TYPE_UINT16,&core_tmprt,
		DBUS_TYPE_UINT16,&surface_tmprt,
		DBUS_TYPE_INVALID)) {
		if(ret != CCGI_SUCCESS){
			return CCGI_FAIL;
			}
		
		} else {
		if (dbus_error_is_set(&err)) {
				dbus_error_free(&err);				
				return CCGI_FAIL;
		}
	}

	SysV->fan_power=fan_power;
	SysV->core_tmprt=core_tmprt;
	SysV->surface_tmprt=surface_tmprt;
	
	dbus_message_unref(reply);
	return CCGI_SUCCESS;
}




#ifdef __cplusplus
}
#endif
