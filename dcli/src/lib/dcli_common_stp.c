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
* dcli_common_stp.c
*
*
*CREATOR:
*	qinhs@autelan.com
*
*DESCRIPTION:
*	APIs used in DCLI for stp process.
*
*DATE:
*	04/03/2008
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.53 $		
*******************************************************************************/

#include <stdio.h>
#include <string.h>

#include <zebra.h>
#include <dbus/dbus.h>

#include <sysdef/npd_sysdef.h>
#include <sysdef/returncode.h>
#include <dbus/npd/npd_dbus_def.h>
#include "command.h"
#include "dcli_vlan.h"
#include "dcli_common_stp.h"
#include "dcli_stp.h"
#include "dcli_eth_port.h"

extern DBusConnection *dcli_dbus_connection;
extern DBusConnection *dcli_dbus_connection_stp;
//int dcli_debug_out = 0;

char* stp_port_role[7] = {
	
			  	"Dis",
				  "Alt",
				  "Bkp",
				  "Root",
				  "Desi",
				  "Mst",
				  "NStp"
	};

char* stp_port_state[5] = {
		"DISABLED",
		"DISCARDING",
		"LEARNING",
		"FORWARDING",
		"NON-STP"
	};

unsigned int  GetSlotPortFromPortIndex
(
    unsigned int eth_g_index,
    unsigned char *slot_no,
    unsigned char *port_no
)
{

	DBusMessage *query, *reply;
	DBusError err; 
	int op_ret = 0;
	unsigned char slotno = 0,portno = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								STP_DBUS_METHOD_GET_SLOTS_PORTS_FROM_INDEX);


    dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&eth_g_index,
									DBUS_TYPE_INVALID);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}


	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_BYTE,&slotno,
		DBUS_TYPE_BYTE,&portno,
		DBUS_TYPE_INVALID)) {
		*slot_no = slotno;
		*port_no = portno;
		/*DCLI_DEBUG(("return op_ret %d and product_id %d\n",op_ret,*id));*/
		dbus_message_unref(reply);
		return CMD_SUCCESS;

	} else {
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
	    return CMD_WARNING;
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;


#if 0
	unsigned char slot_index = 0,port_index = 0;
	
	slot_index = ((eth_g_index & 0x000007C0) >> 6);
	port_index = (eth_g_index & 0x0000003F);
    //printf("Welcome to the GetSlotPortFromEthIndex func!\n");
	//printf("port_index: %d,slot_index %d,port_index %d,productId %d\n",eth_g_index,slot_index,port_index,productId);
	if(PRODUCT_ID_AX7K == product_id){
      *slot_no = slot_index;//AU7K,AU3K,slot_start_no is 0
      *port_no = port_index + 1;//AU7K port start no is 1
      if((*slot_no > 5)||(*port_no > 6)){
         return DCLI_STP_OK;
	  }
     // printf("slot_no %d,port_no %d\n",*slot_no,*port_no);
	  
	}
	else if(PRODUCT_ID_AX5K == product_id){
       *slot_no = slot_index + 1;//AX5K,4K.slot start no is 1
       *port_no = port_index;//AX5K port start no is 0
       if((*slot_no > 1)||(*port_no > 28)){
         return DCLI_STP_OK;
	   }
     // printf("slot_no %d,port_no %d\n", *slot_no,*port_no);
	}
	else if(PRODUCT_ID_AU4K == product_id){//AX5K,4K.slot start no is 1
       *slot_no= slot_index + 1;
	   *port_no = port_index + 1;//AU4K port start no is 1
	   if((*slot_no > 1)||(*port_no > 28)){
         return DCLI_STP_OK;
	   }
	  //printf("slot_no %d,port_no %d\n", *slot_no,*port_no);
	}
	else if(PRODUCT_ID_AU3K == product_id){
       *slot_no = slot_index + 1;//AU3K,slot_start_no is 0
	   *port_no = port_index + 1;//AU3K port start no is 0
	   //::TODO¡¡restrict the slot,port !!!
	   
	 // printf("slot_no %d,port_no %d\n", *slot_no,*port_no);
	}
	else if(PRODUCT_ID_AU3K_BCM== product_id){
	   *slot_no = slot_index + 1;//AU3K,slot_start_no is 0
	   *port_no = port_index + 1;//AU3K port start no is 0
	   //::TODO¡¡restrict the slot,port !!!
	   
	 // printf("slot_no %d,port_no %d\n", *slot_no,*port_no);
	}
	return DCLI_STP_OK;
   /**********************
	*enum product_id_e {
	*	PRODUCT_ID_NONE,
	*	PRODUCT_ID_AX7K,
	**	PRODUCT_ID_AX5K,
	*	PRODUCT_ID_AU4K,
	*	PRODUCT_ID_AU3K,
	*	PRODUCT_ID_MAX
	*};
       **********************/
	
	/*************************************  
	*chassis_slot_count, chassis_slot_start_no
	*{0,0},  // PRODUCT_ID_NONE
	*{5,0},	// PRODUCT_ID_AX7K
	*{1,1},	// PRODUCT_ID_AX5K
	*{1,1},	// PRODUCT_ID_AU4K
	*{0,0}	// PRODUCT_ID_AU3K
	*************************************/
	/* ***********************************
	*ext_slot_count,  eth_port_1no, eth_port_count	
	*{0,0,0},   	//  MODULE_ID_NONE
       * {1,1,4},   	//  MODULE_ID_AX7_CRSMU
	*{0,1,6},	// MODULE_ID_AX7_6GTX
	*{0,1,6},    // MODULE_ID_AX7_6GE_SFP
	*{0,1,1},	// MODULE_ID_AX7_XFP
	*{0,1,6},	// MODULE_ID_AX7_6GTX_POE
	*{0,0,0},	// MODULE_ID_AX5
       ****************************************/
 #endif
}

/**********************************************************************************
 *  dcli_stp_function_support
 *
 *	DESCRIPTION:
 * 		get support stp or not
 *
 *	INPUT:
 *		IsSuport		stp support flg
*	OUTPUT:
 *		IsSuport		stp support flg
 *	
 * 	RETURN:
 *		0   -   dbus success
 *		11   -   dbus error
 * 	NOTATION:
 *		IsSupport input and in this function value is changed if dbus success
 *
 **********************************************************************************/
int dcli_stp_function_support(struct vty* vty, unsigned char *IsSuport)
{
	DBusMessage *query, *reply;
	DBusError err; 
	int op_ret = -1;
	unsigned char isSupport = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								STP_DBUS_METHOD_FUNCTION_SUPPORT);

	
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_BYTE,&isSupport,
		DBUS_TYPE_INVALID)) {
		if(CMD_SUCCESS == op_ret){
			*IsSuport = isSupport;
		}
		else{
			dbus_message_unref(reply);
			return CMD_WARNING;
		}
	}
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/**********************************************************************************
 *  dcli_stp_get_bord_product_id
 *
 *	DESCRIPTION:
 * 		get broad type
 *
 *	INPUT:
 *		product		product id
*	OUTPUT:
 *		product		product id
 *	
 * 	RETURN:
 *		0   -   dbus success
 *		11   -   dbus error
 * 	NOTATION:
 *		IsSupport input and in this function value is changed if dbus success
 *
 **********************************************************************************/
int dcli_stp_get_bord_product_id(struct vty* vty, int *product)
{
	DBusMessage *query, *reply;
	DBusError err; 
	int op_ret = -1;
	unsigned int product_id = 0;
	unsigned char isSupport = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								STP_DBUS_METHOD_GET_BROAD_TYPE);

	
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&product_id,
		DBUS_TYPE_INVALID)) {
		if(CMD_SUCCESS == op_ret){
			*product = product_id;
		}
		else{
			dbus_message_unref(reply);
			return CMD_WARNING;
		}
	}
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


/**********************************************************************************
 *  dcli_get_brg_g_state
 *
 *	DESCRIPTION:
 * 		get stp current state
 *
 *	INPUT:
 *		NONE		
 *	
 * RETURN:
 *		0   -   disable
 *		1   -   enable
 *
 **********************************************************************************/
/*Get the global state and stp mode of the bridge*/
int dcli_get_brg_g_state(struct vty* vty, int *stpmode)
{
	DBusMessage *query, *reply;
	DBusError err; 
	int op_ret = 0;
	int mode = 0;

	query = dbus_message_new_method_call(
								RSTP_DBUS_NAME,    \
								RSTP_DBUS_OBJPATH,    \
								RSTP_DBUS_INTERFACE,    \
								RSTP_DBUS_METHOD_GET_PROTOCOL_STATE);
	
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);
	
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&mode,
		DBUS_TYPE_INVALID)) {
		*stpmode = mode;
		/*vty_out(vty,"stp dcli stpmode value is %d",*stpmode);*/
	} 
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	/*vty_out(vty," stp return op_ret value %d and stpmode value %d\n",op_ret,*stpmode);*/
	dbus_message_unref(reply);
	return op_ret;
}

/**********************************************************************************
 *  dcli_get_brg_g_state
 *
 *	DESCRIPTION:
 * 		get stp current state
 *
 *	INPUT:
 *		NONE		
 *	
 * RETURN:
 *		0   -   disable
 *		1   -   enable
 *
 **********************************************************************************/
int dcli_set_stp_running_mode
(
	DCLI_STP_RUNNING_MODE mode
)
{
	
}

/**********************************************************************************
 *  dcli_enable_g_stp_to_protocol
 *
 *	DESCRIPTION:
 * 		enable or disable stp protocol
 *
 *	INPUT:
 *		type 			- stop/mstp
 *		isEnable	- en/disable
 *	
 * RETURN:
 *		CMD_WARNING   
 *		CMD_SUCCESS   
 *
 **********************************************************************************/

int dcli_enable_g_stp_to_protocol
(
	struct vty* vty,
	DCLI_STP_RUNNING_MODE mode,
	unsigned int isEnable
)
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret = 0,op_ret = 0,state = 0,stp_mode = 0;
	VLAN_PORTS_BMP ports_bmp;
	VLAN_PORTS_BMP *bmpPtr = NULL,*tmp = NULL;
	int count = 0,i;

	memset(&ports_bmp,0,sizeof(VLAN_PORTS_BMP));
	if(1 == isEnable) 
	{
	    state = dcli_get_brg_g_state(vty,&stp_mode);
		if(1 == state){/*has already enabled ,return*/
           vty_out(vty,"STP has already enabled!\n");
		   return CMD_SUCCESS;
		}
		ret = dcli_change_all_ports_to_bmp(vty,&ports_bmp,&count);
		/*vty_out(vty,"dcli stp %s%d>>ports_bmp %02x\n",__FILE__,__LINE__,ports_bmp.untagbmp);*/
		if(CMD_SUCCESS == ret) {
			/*vty_out(vty,"dcli stp %s%d>>ports_bmp vid[%d]\n",__FILE__,__LINE__,ports_bmp.vid);*/
			ret = dcli_send_vlanbmp_to_mstp(vty,&ports_bmp);
			if(CMD_WARNING == ret) {
				vty_out(vty," init mstp error\n");
				return CMD_WARNING;
			}
		}
		else
			return CMD_WARNING;	

		if(DCLI_MST_M == mode) {
			ret = dcli_get_vlan_portmap(vty,&bmpPtr,&count);

			tmp = bmpPtr;
			if(CMD_SUCCESS == ret) {
				for(i = 0; i<count; i++) {
					if(tmp) {
						ret = dcli_send_vlanbmp_to_mstp(vty,tmp);
						if(CMD_WARNING == ret) {
							vty_out(vty,"init mstp error\n");
							return CMD_WARNING;
						}
						tmp++;
					}
				}
			}
			if(bmpPtr)
				free(bmpPtr);
		}
	}


	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									RSTP_DBUS_METHOD_STPM_ENABLE);

    dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&isEnable,
									DBUS_TYPE_UINT32,&mode,
									DBUS_TYPE_INVALID);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);

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
		if(0 == ret) {
			dcli_enable_g_stp_to_npd(vty,isEnable);
		}
		else if(STP_DISABLE == ret)
			vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
		else if(STP_HAVE_ENABLED == ret)
			vty_out(vty,PRINTF_RSTP_HAVE_ENABLED);

		/*vty_out(vty,"dcli  stp ret value %02x\n",ret);*/

	} 
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


/**********************************************************************************
 *  dcli_enable_g_stp_to_npd
 *
 *	DESCRIPTION:
 * 		enable or disable stp to npd
 *
 *	INPUT:
 *		NONE
 *	
 * RETURN:
 *		CMD_WARNING   
 *		CMD_SUCCESS   
 *
 **********************************************************************************/
int dcli_enable_g_stp_to_npd(struct vty* vty,unsigned int enable)
{
	DBusMessage *query, *reply;
	DBusError err; 
	int ret,op_ret;



	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_METHOD_CONFIG_G_ALL_STP);

	
    dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&enable,
							DBUS_TYPE_INVALID);
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS == op_ret ) {
				DCLI_DEBUG(("success\n"));
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else{
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
}

int dcli_get_port_index_link_state
(
	struct vty*   vty,
	unsigned int port_index,
	int* lkState,
	unsigned int *isWAN
)
{
	DBusMessage *query, *reply;
	DBusError err; 
	int ret = 0, op_ret = 0;
	unsigned int value = 0, vlmaster = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_METHOD_STP_GET_PORT_LINK_STATE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&value,	
		DBUS_TYPE_UINT32,&vlmaster,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS == op_ret ) {
				/*vty_out(vty,"success,value [%d]\n",value);*/
				*lkState = value;
				*isWAN = vlmaster;
				/*DCLI_DEBUG(("success,lkstate [%d]\n",*lkState);*/

				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else{
				
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}

}



int dcli_get_port_index_speed
(
	struct vty*   vty,
	unsigned int port_index,
	unsigned int* speed
)
{
    DBusMessage *query, *reply;
	DBusError err; 
	int ret = 0, op_ret = 0;
	int value = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_METHOD_STP_GET_PORT_SPEED);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INT32,&value,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS == op_ret ) {
				/*DCLI_DEBUG(("success\n"));
				//DCLI_DEBUG(("success,value [%d]\n",value));*/
				*speed = value;
				/*DCLI_DEBUG(("success,lkstate [%d]\n",*lkState);*/

				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else{
				
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
}


int dcli_get_port_duplex_mode
(
	struct vty*   vty,
	unsigned int port_index,
	unsigned int* mode
)
{
    DBusMessage *query, *reply;
	DBusError err; 
	int ret = 0, op_ret = 0;
	int value = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_METHOD_STP_GET_PORT_DUPLEX_MODE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INT32,&value,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS == op_ret ) {
				*mode = value;
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else{
				
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
}

int dcli_set_port_duplex_mode_to_stp
(
	struct vty*   vty,
	unsigned int port_index,
	unsigned int mode
)
{
    DBusMessage *query, *reply;
	DBusError err; 
	int ret = 0, op_ret = 0;
	int value = 0;

	query = dbus_message_new_method_call(
								RSTP_DBUS_NAME,    \
								RSTP_DBUS_OBJPATH,	  \
								RSTP_DBUS_INTERFACE,	\
								MSTP_DBUS_METHOD_SET_STP_DUPLEX_MODE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&mode,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS == op_ret ) {
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else{
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
}

int dcli_enable_stp_on_one_port_to_protocol
(
	struct vty*	vty,
	unsigned int port_index,
	unsigned int enable,
	int lkState,
	unsigned int speed,
	unsigned int isWAN,
	unsigned int slot_no,
	unsigned int port_no
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int isEnable = enable;
	int ret = 0, op_ret = 0;
	int port_link = lkState;
	int port_speed = speed;
	unsigned int port_isWAN = isWAN;
	unsigned int port_slot_no = slot_no;
	unsigned int port_port_no = port_no;

	query = dbus_message_new_method_call(
								RSTP_DBUS_NAME,    \
								RSTP_DBUS_OBJPATH,    \
								RSTP_DBUS_INTERFACE,    \
								RSTP_DBUS_METHOD_PORT_STP_ENABLE);
	
	dbus_error_init(&err);

	/*vty_out(vty,"dcli_enable_stp_on_one_port_to_protocol: enable %d,  port_index %d, lkstate %d\n",enable,port_index,lkState);*/
	dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&port_index,
								DBUS_TYPE_UINT32,&isEnable,
								DBUS_TYPE_UINT32,&port_link,
								DBUS_TYPE_UINT32,&port_speed,
								DBUS_TYPE_UINT32,&port_isWAN,								
								DBUS_TYPE_UINT32,&port_slot_no,
								DBUS_TYPE_UINT32,&port_port_no,
								DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		/*dbus_message_unref(reply);*/
		return CMD_WARNING;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_INT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if (STP_RETURN_CODE_SUCCESS == op_ret ) {
				DCLI_DEBUG(("success\n"));
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else{
				DCLI_DEBUG(("enable port %d failed ,ret %02x.\n",port_index,op_ret));
				if(STP_DISABLE== op_ret){
					vty_out(vty,"%% RSTP not enable\n");
				}
				else if(STP_PORT_HAVE_ENABLED == op_ret) {
					vty_out(vty,"%% this port already enabled\n");
				}
				else if(STP_PORT_NOT_ENABLED == op_ret) {
					vty_out(vty,"%% this port not enable\n");
				}
				else if(STP_PORT_NOT_LINK == op_ret){
					vty_out(vty, "%% this port not linked!\n");	
				}
				else{
					vty_out(vty,"%% en/disable error\n");
				}
				
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
	} 
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}

}


int dcli_enable_stp_on_one_port_to_npd
(
	struct vty*	vty,	
	unsigned int mode,
	unsigned int port_index,
	unsigned int enable
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int isEnable = enable;
	int ret = 0,op_ret = 0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_METHOD_CONFIG_STP);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&port_index,
							 DBUS_TYPE_UINT32,&mode,
							 DBUS_TYPE_UINT32,&isEnable,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_INT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS == op_ret ) {
				DCLI_DEBUG(("success\n"));
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else{
				dbus_message_unref(reply);
				return CMD_WARNING;
			}
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}

}

 int dcli_get_one_port_index
(
	struct vty*		vty,
	unsigned char slot,
	unsigned char local_port,
	unsigned int* port_index
) 
{
	DBusMessage *query, *reply;
	DBusError err;
	int op_ret = 0;
	unsigned int eth_g_index = 0;
	unsigned char slot_no = slot,port_no = local_port;

	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
											NPD_DBUS_BUSNAME, 	\
											NPD_DBUS_ETHPORTS_OBJPATH,	\
											NPD_DBUS_ETHPORTS_INTERFACE, \
											STP_DBUS_METHOD_GET_PORT_INDEX
											);
	
		
		/*DCLI_DEBUG(("the slot_no: %d	port_no: %d\n",slot_no,port_no);
		
		//tran_value=ETH_GLOBAL_INDEX_FROM_SLOT_PORT_LOCAL_NO(slot_no,port_no);
		//DCLI_DEBUG(("changed value : slot %d,port %d\n",slot_no,port_no));*/
		dbus_message_append_args(query,
										DBUS_TYPE_BYTE,&slot_no,										
										DBUS_TYPE_BYTE,&port_no,									
										DBUS_TYPE_INVALID);


	 reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get re_slot_port reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	
	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_UINT32,&eth_g_index,
								DBUS_TYPE_INVALID)) {
		if(NPD_DBUS_SUCCESS == op_ret){
			*port_index = eth_g_index;
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
		else if(NPD_DBUS_ERROR == op_ret){
			/*DCLI_DEBUG(("execute command failed\n");*/
			dbus_message_unref(reply);
			return CMD_WARNING;
		}
		else if(NPD_DBUS_ERROR_NO_SUCH_PORT == op_ret){
			vty_out(vty,"NO SUCH PORT\n");
			dbus_message_unref(reply);
			return CMD_WARNING;
		}		
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
		
}

int dcli_get_all_ports_index
(
	struct vty*	 vty, 		
	PORT_MEMBER_BMP* portBmp
)
{
	DBusMessage *query, *reply;
	DBusError err;
	int op_ret = 0;
	unsigned int portbmp[2] = {0};

	dbus_error_init(&err);
	query = dbus_message_new_method_call(  \
												NPD_DBUS_BUSNAME, 	\
												NPD_DBUS_ETHPORTS_OBJPATH,	\
												NPD_DBUS_ETHPORTS_INTERFACE, \
												STP_DBUS_METHOD_GET_ALL_PORTS_INDEX_V1
												);

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get slot count npdReply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}

	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_UINT32,&portbmp[0],
								DBUS_TYPE_UINT32,&portbmp[1],
								DBUS_TYPE_INVALID)) {
		portBmp->portMbr[0]= portbmp[0];
		portBmp->portMbr[1]= portbmp[1];
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}

}


/**********************************************************************************
 *  dcli_stp_set_stpid_to_npd
 *
 *	DESCRIPTION:
 * 		bind vlan id  and specifed stpid
 *
 *	INPUT:
 *		NONE		
 *	
 * RETURN:
 *		CMD_SUCCESS
 *		CMD_WARNING
 *
 **********************************************************************************/
int dcli_stp_set_stpid_to_npd
(
	struct vty* vty,
	unsigned short vid,
	unsigned int mstid
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								MSTP_DBUS_METHOD_CFG_VLAN_ON_MST);

	
	dbus_message_append_args(query,
									DBUS_TYPE_UINT16,&vid,							 								 		
									DBUS_TYPE_UINT32,&mstid,									
									DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);
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

		if(0 != ret) {
			vty_out(vty,"npd:vlan not exit, please check! \n");
		}
		
	} else {
		vty_out(vty,"Failed get args from rstp!.\n");	
		if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int dcli_stp_set_port_prio_to_npd
(
	struct vty* vty,
	unsigned int mstid,
	unsigned int port_index,
	unsigned int value
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								RSTP_DBUS_METHOD_CONFIG_PORTPRIO);

	
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
									DBUS_TYPE_UINT32,&port_index,							 								 		
									DBUS_TYPE_UINT32,&value,									
									DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);
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

		if(0 != ret) {
			vty_out(vty,"execute command error\n");
		}
		
	} else {
		vty_out(vty,"Failed get args from rstp!.\n");	
		if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int dcli_stp_set_port_pathcost_to_npd
(
	struct vty* vty,
	unsigned int mstid,
	unsigned int port_index,
	unsigned int value
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								RSTP_DBUS_METHOD_CONFIG_PORT_PATHCOST);

	
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
									DBUS_TYPE_UINT32,&port_index,							 								 		
									DBUS_TYPE_UINT32,&value,									
									DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);
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

		if(0 != ret) {
			vty_out(vty,"execute command error\n");
		}
		
	} else {
		vty_out(vty,"Failed get args from rstp!.\n");	
		if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int dcli_stp_set_port_edge_to_npd
(
	struct vty* vty,
	unsigned int mstid,
	unsigned int port_index,
	unsigned int value
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								RSTP_DBUS_METHOD_CONFIG_EDGE);

	
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&port_index,							 								 		
									DBUS_TYPE_UINT32,&value,									
									DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);
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

		if(0 != ret) {
			vty_out(vty,"execute command error\n");
		}
		
	} else {
		vty_out(vty,"Failed get args from rstp!.\n");	
		if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int dcli_stp_set_port_p2p_to_npd
(
	struct vty* vty,
	unsigned int mstid,
	unsigned int port_index,
	unsigned int value
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								RSTP_DBUS_METHOD_CONFIG_P2P);

	
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&port_index,							 								 		
									DBUS_TYPE_UINT32,&value,									
									DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);
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

		if(0 != ret) {
			vty_out(vty,"execute command error\n");
		}
		
	} else {
		vty_out(vty,"Failed get args from rstp!.\n");	
		if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int dcli_stp_set_port_nonstp_to_npd
(
	struct vty* vty,
	unsigned int mstid,
	unsigned int port_index,
	unsigned int value
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								RSTP_DBUS_METHOD_CONFIG_NONSTP);

	
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&port_index,							 								 		
									DBUS_TYPE_UINT32,&value,									
									DBUS_TYPE_INVALID);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);
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

		if(0 != ret) {
			vty_out(vty,"execute command error\n");
		}
		
	} else {
		vty_out(vty,"Failed get args from rstp!.\n");	
		if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int dcli_change_all_ports_to_bmp
(
	struct vty*				vty,
	VLAN_PORTS_BMP* ports_bmp,
	unsigned int *num
)
{
	PORT_MEMBER_BMP portBmp;
	int ret = 0;
	/*SLOT_INFO slot_info[6] = {{0}};*/
    memset(&portBmp,0,sizeof(PORT_MEMBER_BMP));
	*num = 0;
	ports_bmp->vid = 0;  /*init stp ports,create port struct,vid 0 is flag*/
	ret = dcli_get_all_ports_index(vty,&portBmp);
	if(0 != ret)
		return CMD_WARNING;
	else {  /*init cist ports*/
		/*ports_bmp->untagbmp = portBmp;*/
		memcpy(&(ports_bmp->untagbmp),&portBmp,sizeof(PORT_MEMBER_BMP));
		memset(&(ports_bmp->tagbmp),0,sizeof(PORT_MEMBER_BMP));
		ports_bmp->vid = 0;
	}
	return 0;
}

int show_slot_port_by_productid
(
	struct vty *vty,
	unsigned int product_id,
	/*unsigned int portBmp,*/
	PORT_MEMBER_BMP* portBmp,
	unsigned int vid,
	unsigned int mstid
)
{

	unsigned int i,port_index = 0;
	unsigned int slot = 0,port = 0;
    unsigned int tmpVal[2];

	memset(&tmpVal,0,sizeof(tmpVal));
	for (i=0;i<64;i++) {
		if(PRODUCT_ID_AX7K == product_id) {
			slot = i/8 + 1;
			port = i%8;			
		}
		else if(PRODUCT_ID_AX7K_I == product_id){
			/*slot = i/8 + 1;
			port = i%8;*/
			slot = i/8 ;
			port = i%8+1;
		}
		else if(PRODUCT_ID_AX5K_I == product_id) {
			if (i < 8) {
				slot = 0;
				port = i + 1;
			}
			else {
				slot = 1;
				port = i + 1 - 8;
			}
		}
		else if((PRODUCT_ID_AX5K == product_id) ||
				(PRODUCT_ID_AX5K_E == product_id) ||
				(PRODUCT_ID_AX5608== product_id) ||
				(PRODUCT_ID_AU4K == product_id) ||
				(PRODUCT_ID_AU3K == product_id) ||
				(PRODUCT_ID_AU3K_BCM == product_id)||	
				(PRODUCT_ID_AU3K_BCAT == product_id)||
				(PRODUCT_ID_AU2K_TCAT == product_id)) {
			slot = 1;
			port = i;
		}
		
		tmpVal[i/32] = (1<<(i%32));
		if(portBmp->portMbr[i/32] & tmpVal[i/32]) {				
			if(PRODUCT_ID_AX7K_I == product_id) {
				//vty_out(vty,"cscd%d", port-1);
				vty_out(vty,"%-d/%-10d",slot,port);
			}
			else {
				vty_out(vty,"%-d/%-10d",slot,port);
			}
			vty_out(vty,"%-4d",vid);
			if(dcli_get_one_port_index(vty,slot,port,&port_index) < 0)	{
				DCLI_DEBUG(("execute command failed\n"));
				continue;
			}
			dcli_get_mstp_one_port_info(vty,mstid,port_index);
		}
	}
		
	return 0;
}
#if 0
int dcli_change_all_ports_to_bmp
(
	struct vty*				vty,
	VLAN_PORTS_BMP* ports_bmp,
	unsigned int *num
)
{
	int i,j,count = 0;
	SLOT_INFO slot_info[6] = {{0}};

	*num = 0;
	ports_bmp->vid = 0;  //init stp ports,create port struct,vid 0 is flag
	count = dcli_get_all_ports_index(vty,&slot_info);
	for(i = 0; i<count; i++) {
		for(j = 0; j< slot_info[i].local_port_count; j++) {
			if(0 != slot_info[i].port_no[j].port_index)
				ports_bmp->untagbmp |= (1 << ((slot_info[i].slot_no-1)*8) + slot_info[i].port_no[j].local_port_no);
			else
				continue;
		}
	}

	return 0;
}
#endif
unsigned int dcli_get_one_port_admin_state
(
	struct vty*	vty,
	unsigned int port_index
)
{
	DBusMessage *stpQuery,*stpReply;
	DBusError err;
	DBusMessageIter	 stpIter;

	unsigned int ret = 0;

    unsigned int enable = 0;
	/*printf("port index %d\n",port_index);*/
	stpQuery = dbus_message_new_method_call(			\
								RSTP_DBUS_NAME,		\
								RSTP_DBUS_OBJPATH,	\
								RSTP_DBUS_INTERFACE, \
								RSTP_DBUS_METHOD_SHOW_SPANTREE_ADMIN_STATE);

	dbus_message_append_args(stpQuery,
					 DBUS_TYPE_UINT32,&port_index,
					 DBUS_TYPE_INVALID);
	dbus_error_init(&err);
	stpReply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,stpQuery,-1, &err);


	dbus_message_unref(stpQuery);
	if (NULL == stpReply) {
		vty_out(vty,"failed get stp stpReply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(stpReply);
		return CMD_WARNING;
	}
	dbus_message_iter_init(stpReply,&stpIter);			
	dbus_message_iter_get_basic(&stpIter, &ret);
	if(DCLI_STP_OK == ret )
	{
		dbus_message_iter_next(&stpIter);
		dbus_message_iter_get_basic(&stpIter,&enable);
        vty_out(vty,"%-11s",enable ? "Y":"N");
		dbus_message_unref(stpReply);
		return CMD_SUCCESS;
		
	}
	else if(STP_DISABLE == ret)
	{
		dbus_message_unref(stpReply);
		return ret;
	}
	else
	{
		dbus_message_unref(stpReply);
		return ret;
	}
}


unsigned int dcli_get_one_port_info
(
	struct vty*	vty,
	unsigned int port_index,
	unsigned int portductid
)
{
	DBusMessage *stpQuery,*stpReply;
	DBusError err;
	DBusMessageIter	 stpIter;
	DBusMessageIter  stpIter_array;
	DBusMessageIter	 stpIter_struct;
	DBusMessageIter	 stpIter_sub_struct;


	char buf[10] = {0};
	int n,ret;

	unsigned char  port_prio;
	unsigned int 	port_cost;
	int 					port_role;
	int 					port_state;
	int 					port_lk;
	int 					port_p2p;
	int 					port_edge;
	unsigned short br_prio;
	unsigned char   mac[6] ={'\0','\0','\0','\0','\0','\0'};
	unsigned int     br_cost;
	unsigned short br_dPort;

	/*printf("port index %d\n",port_index);*/
	stpQuery = dbus_message_new_method_call(			\
								RSTP_DBUS_NAME,		\
								RSTP_DBUS_OBJPATH,	\
								RSTP_DBUS_INTERFACE, \
								RSTP_DBUS_METHOD_SHOW_SPANTREE);

			dbus_message_append_args(stpQuery,
							 DBUS_TYPE_UINT32,&(port_index),
							 DBUS_TYPE_INVALID);
			dbus_error_init(&err);
			stpReply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,stpQuery,-1, &err);


			dbus_message_unref(stpQuery);
			if (NULL == stpReply) {
				vty_out(vty,"failed get stp stpReply.\n");
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				dbus_message_unref(stpReply);
				return CMD_WARNING;
			}

	dbus_message_iter_init(stpReply,&stpIter);				
	dbus_message_iter_get_basic(&stpIter, &ret);
	dbus_message_iter_next(&stpIter);
	if(DCLI_STP_OK == ret )
	{
		/*		
			Array of Port Infos.
			port no
			port prio
			port role
			port State
			port link
			port p2p
			port edge
			port Desi bridge
			port Dcost
			port D-port
		*/
		dbus_message_iter_recurse(&stpIter,&stpIter_array);

		dbus_message_iter_recurse(&stpIter_array,&stpIter_struct);
	
		dbus_message_iter_get_basic(&stpIter_struct,&port_prio);
		
		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&port_cost);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&port_role);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&port_state);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&port_lk);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&port_p2p);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&port_edge);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&br_cost);			

		dbus_message_iter_next(&stpIter_struct);			
		dbus_message_iter_get_basic(&stpIter_struct,&br_dPort);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_recurse(&stpIter_struct,&stpIter_sub_struct);


		dbus_message_iter_get_basic(&stpIter_sub_struct,&br_prio);
		dbus_message_iter_next(&stpIter_sub_struct);

		for(n = 0; n < 6; n++)
		{
			dbus_message_iter_get_basic(&stpIter_sub_struct,&mac[n]);
			dbus_message_iter_next(&stpIter_sub_struct);
		}
		if(strcmp("NStp",stp_port_role[port_role]) == 0) {
		
					vty_out(vty,"%-4s","-");
					vty_out(vty,"%-9s","-");
				    vty_out(vty,"%-5s",stp_port_role[port_role]);
					vty_out(vty,"%-13s","-");
					vty_out(vty,"%-3s", "-");
		
					if(0 == port_p2p)
						vty_out(vty,"%-4s","-");
					else if(1 == port_p2p)
						vty_out(vty,"%-4s","-");
					else if(2 == port_p2p)
						vty_out(vty,"%-4s","-");
					
					vty_out(vty,"%-5s",port_edge ? "-" : "-");
		
					memset(buf,0,sizeof(buf));
					sprintf(buf,"%s","-");
		
				   memset(buf,0,sizeof(buf));
				   sprintf(buf,"%s","-");
				   vty_out(vty,"%s","");
						
				   vty_out(vty,"\n");
		
				}
		
	else {
		vty_out(vty,"%-4d",port_prio);
		vty_out(vty,"%-9d",port_cost);
		vty_out(vty,"%-5s",stp_port_role[port_role]);
		vty_out(vty,"%-13s",stp_port_state[port_state]);
		vty_out(vty,"%-3s", port_lk ? "Y" : "N");

		if(0 == port_p2p)
			vty_out(vty,"%-4s","N");
		else if(1 == port_p2p)
			vty_out(vty,"%-4s","Y");
		else if(2 == port_p2p)
			vty_out(vty,"%-4s","A");
		
		vty_out(vty,"%-5s",port_edge ? "Y" : "N");

		memset(buf,0,sizeof(buf));
		sprintf(buf,"%d",br_prio);
		if(port_state)
		{				
			vty_out(vty,"%-5s:",port_state ? buf : "");
			for(n = 0; n < 6; n++)
			{
				vty_out(vty,"%02x",mac[n]);
			}
			vty_out(vty,"  ");
		}
		
		memset(buf,0,sizeof(buf));
		sprintf(buf,"%d",br_cost);
		/*if((strcmp("NStp",stp_port_role[port_role]))){
		   vty_out(vty,"%-10s",port_state ? buf : "");
		}
		else {
			vty_out(vty,"%-10s","     -");
		}*/
		memset(buf,0,sizeof(buf));
		sprintf(buf,"%#0x",br_dPort);
		vty_out(vty,"%s",port_state ? buf : "");
				
		vty_out(vty,"\n");

	}
		dbus_message_unref(stpReply);
		return CMD_SUCCESS;
		
	}
	else if(STP_DISABLE == ret)
	{
		dbus_message_unref(stpReply);
		return ret;
	}
	else
	{
		dbus_message_unref(stpReply);
		return ret;
	}
}

unsigned int dcli_get_br_info
(
	struct vty* vty
)
{
	DBusMessage *brQuery,*brReply;
	DBusError err;
	DBusMessageIter	 brIter,brIter_array,brIter_struct;

	char buf[10] = {0};

	unsigned char   root_br_mac[6] ={'\0','\0','\0','\0','\0','\0'};
	unsigned char   design_br_mac[6] ={'\0','\0','\0','\0','\0','\0'};
	unsigned int    root_path_cost,design_br_version;
	unsigned short  root_br_prio,design_br_prio;
	unsigned short  root_br_portId;
	unsigned short  root_br_maxAge,design_br_maxAge;
	unsigned short  root_br_hTime,design_br_hTime;
	unsigned short  root_br_fdelay,design_br_fdelay;
	unsigned int    eth_g_index = 0;
	unsigned char   slot = 0,port = 0;
	int i = 0,ret = 1;

	brQuery = dbus_message_new_method_call(			\
						RSTP_DBUS_NAME,		\
						RSTP_DBUS_OBJPATH,	\
						RSTP_DBUS_INTERFACE, \
						RSTP_DBUS_METHOD_SHOW_BRIDGE_INFO);

	dbus_error_init(&err);
	brReply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,brQuery,-1, &err);
	dbus_message_unref(brQuery);
	if (NULL == brReply) {
		vty_out(vty,"failed get slot count npdReply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		//dbus_message_unref(brReply);
		return CMD_WARNING;
	}
	dbus_message_iter_init(brReply,&brIter);
	dbus_message_iter_get_basic(&brIter, &ret);
	dbus_message_iter_next(&brIter);
	if(DCLI_STP_OK == ret )
	{

	dbus_message_iter_recurse(&brIter,&brIter_array);


	dbus_message_iter_recurse(&brIter_array,&brIter_struct);

	/*display root bridge info*/
		vty_out(vty,"-----------------------SPANNING TREE information of STP domain 0------------------------------\n");

		vty_out(vty,"Designated Root\t\t\t:  ");

		for(i = 0; i< 5; i++)
		{
			dbus_message_iter_get_basic(&brIter_struct,&root_br_mac[i]);
			vty_out(vty,"%02x:",root_br_mac[i]);
			dbus_message_iter_next(&brIter_struct);
		}

		dbus_message_iter_get_basic(&brIter_struct,&design_br_mac[i]);
		vty_out(vty,"%02x",design_br_mac[5]);
		dbus_message_iter_next(&brIter_struct);

		vty_out(vty,"\nDesignated Root Priority\t:  ");
		dbus_message_iter_get_basic(&brIter_struct,&root_br_prio);
		vty_out(vty,"%d\n",root_br_prio);
		dbus_message_iter_next(&brIter_struct);

		vty_out(vty,"Designated Root Path Cost\t:  ");
		dbus_message_iter_get_basic(&brIter_struct,&root_path_cost);
		vty_out(vty,"%d\n",root_path_cost);
		dbus_message_iter_next(&brIter_struct);


		vty_out(vty,"Root Port\t\t\t:  ");
		dbus_message_iter_get_basic(&brIter_struct,&root_br_portId);
		if(root_br_portId){
			eth_g_index = root_br_portId&0xfff;
			GetSlotPortFromPortIndex(eth_g_index,&slot,&port);
			vty_out(vty,"%d/%d\n",slot,port);
		}
		else{
			vty_out(vty,"%s\n","none");
		}
		dbus_message_iter_next(&brIter_struct);


		vty_out(vty,"Root Max Age");
		dbus_message_iter_get_basic(&brIter_struct,&root_br_maxAge);
		vty_out(vty,"%6d\t",root_br_maxAge);
		dbus_message_iter_next(&brIter_struct);

		vty_out(vty,"Hello Time");
		dbus_message_iter_get_basic(&brIter_struct,&root_br_hTime);
		vty_out(vty,"%5d\t\t",root_br_hTime);
		dbus_message_iter_next(&brIter_struct);

		vty_out(vty,"Forward Delay");
		dbus_message_iter_get_basic(&brIter_struct,&root_br_fdelay);
		vty_out(vty,"%5d\n",root_br_fdelay);
		dbus_message_iter_next(&brIter_struct);

		/*display self-bridge info*/

		vty_out(vty,"\nBridge ID Mac Address\t\t:  ");

		for(i = 0; i< 5; i++)
		{
			dbus_message_iter_get_basic(&brIter_struct,&design_br_mac[i]);
			vty_out(vty,"%02x:",design_br_mac[i]);
			dbus_message_iter_next(&brIter_struct);
		}

		dbus_message_iter_get_basic(&brIter_struct,&design_br_mac[i]);
		vty_out(vty,"%02x",design_br_mac[5]);
		dbus_message_iter_next(&brIter_struct);

		vty_out(vty,"\nBridge ID Priority\t\t:  ");
		dbus_message_iter_get_basic(&brIter_struct,&design_br_prio);
		vty_out(vty,"%d\n",design_br_prio);
		dbus_message_iter_next(&brIter_struct);

		vty_out(vty,"Bridge ID ForceVersion\t\t:  ");
		dbus_message_iter_get_basic(&brIter_struct,&design_br_version);
		vty_out(vty,"%d\n",design_br_version);
		dbus_message_iter_next(&brIter_struct);

		vty_out(vty,"Bridge Max Age");
		dbus_message_iter_get_basic(&brIter_struct,&design_br_maxAge);
		vty_out(vty,"%5d\t",design_br_maxAge);
		dbus_message_iter_next(&brIter_struct);

		vty_out(vty,"Hello Time");
		dbus_message_iter_get_basic(&brIter_struct,&design_br_hTime);
		vty_out(vty,"%5d\t\t",design_br_hTime);
		dbus_message_iter_next(&brIter_struct);

		vty_out(vty,"Forward Delay");
		dbus_message_iter_get_basic(&brIter_struct,&design_br_fdelay);
		vty_out(vty,"%5d\n",design_br_fdelay);
		dbus_message_iter_next(&brIter_struct);
		dbus_message_unref(brReply);
		return CMD_SUCCESS;
	}
	else if(STP_DISABLE == ret)
	{
		/*vty_out(vty,"stp is disable return reval is %d\n",ret);*/
		dbus_message_unref(brReply);
		return ret;
	}
	else
	{
		dbus_message_unref(brReply);
		return ret;
	}
	
}

unsigned int dcli_get_brage_info
(
)
{
	DBusMessage *brQuery,*brReply;
	DBusError err;
	DBusMessageIter	 brIter,brIter_struct;

	char buf[10] = {0};

	unsigned char   root_br_mac[6] ={'\0','\0','\0','\0','\0','\0'};
	unsigned char   design_br_mac[6] ={'\0','\0','\0','\0','\0','\0'};
	unsigned int      root_path_cost,design_br_version;
	unsigned short  root_br_prio,design_br_prio;
	unsigned short  root_br_portId;
	unsigned short  root_br_maxAge,design_br_maxAge;
	unsigned short  root_br_hTime,design_br_hTime;
	unsigned short  root_br_fdelay,design_br_fdelay;
	int i,ret;

	brQuery = dbus_message_new_method_call(			\
						RSTP_DBUS_NAME,		\
						RSTP_DBUS_OBJPATH,	\
						RSTP_DBUS_INTERFACE, \
						RSTP_DBUS_METHOD_SHOW_BRIDGE_INFO);

	dbus_error_init(&err);
	brReply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,brQuery,-1, &err);
	dbus_message_unref(brQuery);
	if (NULL == brReply) {
		/*vty_out(vty,"failed get slot count npdReply.\n");*/
		printf("failed get slot count npdReply.\n");
		if (dbus_error_is_set(&err)) {
			/*vty_out(vty,"%s raised: %s",err.name,err.message);*/
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(brReply);
		return CMD_WARNING;
	}

	dbus_message_iter_init(brReply,&brIter);
	dbus_message_iter_get_basic(&brIter, &ret);
	dbus_message_iter_next(&brIter);
	if(DCLI_STP_OK == ret )
	{

	dbus_message_iter_recurse(&brIter,&brIter_struct);

	/*display root bridge info*/
		/*vty_out(vty,"-----------------------SPANNING TREE information of STP domain 0------------------------------\n");

		//vty_out(vty,"Designated Root\t\t\t:  ");*/

		for(i = 0; i< 5; i++)
		{
			dbus_message_iter_get_basic(&brIter_struct,&root_br_mac[i]);
			/*vty_out(vty,"%02x:",root_br_mac[i]);*/
			dbus_message_iter_next(&brIter_struct);
		}

		dbus_message_iter_get_basic(&brIter_struct,&design_br_mac[i]);
		/*vty_out(vty,"%02x",design_br_mac[5]);*/
		dbus_message_iter_next(&brIter_struct);

		/*vty_out(vty,"\nDesignated Root Priority\t:  ");*/
		dbus_message_iter_get_basic(&brIter_struct,&root_br_prio);
		/*vty_out(vty,"%d\n",root_br_prio);*/
		dbus_message_iter_next(&brIter_struct);

		/*vty_out(vty,"Designated Root Path Cost\t:  ");*/
		dbus_message_iter_get_basic(&brIter_struct,&root_path_cost);
		/*vty_out(vty,"%d\n",root_path_cost);*/
		dbus_message_iter_next(&brIter_struct);


		
		/*vty_out(vty,"Root Port\t\t\t:  ");*/
		dbus_message_iter_get_basic(&brIter_struct,&root_br_portId);
		if(root_br_portId){
			;/*vty_out(vty,"%d\n",root_br_portId);*/
		}
		else{
			;/*vty_out(vty,"%s\n","none");*/
		}
		dbus_message_iter_next(&brIter_struct);


		/*vty_out(vty,"Root Max Age");*/
		dbus_message_iter_get_basic(&brIter_struct,&root_br_maxAge);
		/*vty_out(vty,"%6d\t",root_br_maxAge);*/
		dbus_message_iter_next(&brIter_struct);

		/*vty_out(vty,"Hello Time");*/
		dbus_message_iter_get_basic(&brIter_struct,&root_br_hTime);
		/*vty_out(vty,"%5d\t\t",root_br_hTime);*/
		dbus_message_iter_next(&brIter_struct);

		/*vty_out(vty,"Forward Delay");*/
		dbus_message_iter_get_basic(&brIter_struct,&root_br_fdelay);
		/*vty_out(vty,"%5d\n",root_br_fdelay);*/
		dbus_message_iter_next(&brIter_struct);

		/*display self-bridge info*/

		/*vty_out(vty,"\nBridge ID Mac Address\t\t:  ");*/

		for(i = 0; i< 5; i++)
		{
			dbus_message_iter_get_basic(&brIter_struct,&design_br_mac[i]);
			/*vty_out(vty,"%02x:",design_br_mac[i]);*/
			dbus_message_iter_next(&brIter_struct);
		}

		dbus_message_iter_get_basic(&brIter_struct,&design_br_mac[i]);
		/*vty_out(vty,"%02x",design_br_mac[5]);*/
		dbus_message_iter_next(&brIter_struct);

		/*vty_out(vty,"\nBridge ID Priority\t\t:  ");*/
		dbus_message_iter_get_basic(&brIter_struct,&design_br_prio);
		/*vty_out(vty,"%d\n",design_br_prio);*/
		dbus_message_iter_next(&brIter_struct);

		/*vty_out(vty,"Bridge ID ForceVersion\t\t:  ");*/
		dbus_message_iter_get_basic(&brIter_struct,&design_br_version);
		/*vty_out(vty,"%d\n",design_br_version);*/
		dbus_message_iter_next(&brIter_struct);

		/*vty_out(vty,"Bridge Max Age");*/
		dbus_message_iter_get_basic(&brIter_struct,&design_br_maxAge);
		/*vty_out(vty,"%5d\t",design_br_maxAge);*/
		dbus_message_iter_next(&brIter_struct);

		/*vty_out(vty,"Hello Time");*/
		dbus_message_iter_get_basic(&brIter_struct,&design_br_hTime);
		/*vty_out(vty,"%5d\t\t",design_br_hTime);*/
		dbus_message_iter_next(&brIter_struct);

		/*vty_out(vty,"Forward Delay");*/
		dbus_message_iter_get_basic(&brIter_struct,&design_br_fdelay);
		/*vty_out(vty,"%5d\n",design_br_fdelay);*/
		dbus_message_iter_next(&brIter_struct);
		dbus_message_unref(brReply);
		return CMD_SUCCESS;
	}
	else if(STP_DISABLE == ret)
	{
		dbus_message_unref(brReply);
		return ret;
	}
	else
	{
		dbus_message_unref(brReply);
		return ret;
	}
	
}


int dcli_get_one_vlan_portmap
(
	struct vty*		vty,
	unsigned short vid,
	VLAN_PORTS_BMP* ports_bmp
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	char*			vlanName;
	/*unsigned int untagBmp = 0,tagBmp =0;*/
	unsigned int ret = 0;
	unsigned int product_id = 0;
	unsigned int vlanStat = 0;
	unsigned int promisPortBmp[2] = {0};
	PORT_MEMBER_BMP untagBmp,tagBmp;
	memset(&untagBmp,0,sizeof(PORT_MEMBER_BMP));
	memset(&tagBmp,0,sizeof(PORT_MEMBER_BMP));
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,\
										NPD_DBUS_VLAN_METHOD_SHOW_VLAN_PORT_MEMBERS_V1 );
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vid,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &ret,
					DBUS_TYPE_UINT32, &product_id,
					DBUS_TYPE_UINT32, &promisPortBmp[0],					
					DBUS_TYPE_UINT32, &promisPortBmp[1],
					DBUS_TYPE_STRING, &vlanName,
					DBUS_TYPE_UINT32, &untagBmp.portMbr[0],
					DBUS_TYPE_UINT32, &untagBmp.portMbr[1],
					DBUS_TYPE_UINT32, &tagBmp.portMbr[0],
					DBUS_TYPE_UINT32, &tagBmp.portMbr[1],
					DBUS_TYPE_UINT32, &vlanStat,
					DBUS_TYPE_INVALID)) {
		/*vty_out(vty,"vlanName %s , untagBmp %#0X , tagBmp %#0X\n",vlanName,untagBmp,tagBmp);*/
		if(0==ret|| 64 == ret) {
			ports_bmp->tagbmp.portMbr[0]= tagBmp.portMbr[0];
			ports_bmp->tagbmp.portMbr[1]= tagBmp.portMbr[1];
			ports_bmp->untagbmp.portMbr[0]= untagBmp.portMbr[0];
			ports_bmp->untagbmp.portMbr[1]= untagBmp.portMbr[1];
			return CMD_SUCCESS;
		}
		else if(NPD_DBUS_ERROR_NO_SUCH_VLAN +1== ret) {
			vty_out(vty,"% Bad parameter: vlan id illegal.\n");
			dbus_message_unref(reply);
			return ret;
		}
		/*
		else if(NPD_VLAN_NOTEXISTS == ret) {
			vty_out(vty,"% Bad parameter: vlan %d NOT Exists.\n",vid);
			dbus_message_unref(reply);
			return ret;
		}
		*/
		else if(0xff  == ret) {
			vty_out(vty,"% Error: Op on Hw Fail.\n");
			dbus_message_unref(reply);
			return ret;
		}
		vty_out(vty,"% Bad parameter: vlan id illegal.\n");
	}
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	
}

int dcli_get_vlan_portmap
(
	struct vty*				   vty,	
	VLAN_PORTS_BMP** ports_bmp,
	unsigned int* count
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int vlan_Cont = 0;
	unsigned short  vlanId = 0,trunkId;
	char*			vlanName = NULL;
	PORT_MEMBER_BMP	untagBmp,tagBmp;
	unsigned int j,ret;
	unsigned int vlanStat = 0;
	VLAN_PORTS_BMP* tmp = NULL;
	unsigned int 	product_id = PRODUCT_ID_NONE;
	unsigned int    promisPortBmp[2] = {0};
	unsigned char trkTagMode;

    memset(&untagBmp,0,sizeof(PORT_MEMBER_BMP));
	memset(&tagBmp,0,sizeof(PORT_MEMBER_BMP));
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,	\
										NPD_DBUS_VLAN_METHOD_SHOW_VLANLIST_PORT_MEMBERS_V1 );
	

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	dbus_message_iter_init(reply,&iter);

	dbus_message_iter_get_basic(&iter,&ret);
	if(0 == ret){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&vlan_Cont);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&product_id);
				
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&promisPortBmp[0]);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&promisPortBmp[1]);

		
		/*DCLI_DEBUG(("get basic return value actVlanCont =%d\n",vlan_Cont);*/
		tmp = (VLAN_PORTS_BMP*)malloc(vlan_Cont * sizeof(VLAN_PORTS_BMP));
		if(!tmp)
			return CMD_WARNING;
		
		memset(tmp,0,vlan_Cont * sizeof(VLAN_PORTS_BMP));
		*ports_bmp = tmp ;
		*count = vlan_Cont;
		DCLI_DEBUG(("tmp %p,count %d\n",tmp,*count));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for (j = 0; j < vlan_Cont; j++) {

			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&vlanId);
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&vlanName);
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&untagBmp.portMbr[0]);
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&untagBmp.portMbr[1]);
			dbus_message_iter_next(&iter_struct);	
			
			dbus_message_iter_get_basic(&iter_struct,&tagBmp.portMbr[0]);
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&tagBmp.portMbr[1]);
			dbus_message_iter_next(&iter_struct);	
			
			dbus_message_iter_get_basic(&iter_struct,&trkTagMode);
			dbus_message_iter_next(&iter_array);
			DCLI_DEBUG(("untagbmp[0-1] %02x-%02x,tagbmp[0-1] %02x-%02x\n",untagBmp.portMbr[0],untagBmp.portMbr[1],tagBmp.portMbr[0],tagBmp.portMbr[1]));
			tmp->vid = vlanId;
			tmp->untagbmp.portMbr[0]= untagBmp.portMbr[0];
			tmp->untagbmp.portMbr[1]= untagBmp.portMbr[1];
			tmp->tagbmp.portMbr[0]= tagBmp.portMbr[0];
			tmp->tagbmp.portMbr[1]= tagBmp.portMbr[1];
			DCLI_DEBUG(("1318::tmp ++ [0]%02x [1]%02x,tagbmp [0]%02x [1]%02x\n",untagBmp.portMbr[0],untagBmp.portMbr[1],tagBmp.portMbr[0],tagBmp.portMbr[1]));
			tmp++;
			
		}
	}
	else if (NPD_VLAN_ERR_HW == ret) {
		DCLI_DEBUG(("Error occurs in initing mstp\n"));				
	}
	dbus_message_unref(reply);
	DCLI_DEBUG(("715 :: dcli_get_vlan_portsbmp end\n"));
	return CMD_SUCCESS;
	
}

int dcli_send_vlanbmp_to_mstp
(
	struct vty*				vty,
	VLAN_PORTS_BMP* ports_bmp
)
{
	DBusMessage *query, *reply;
	DBusError err;
	int op_ret = -1;
	unsigned int product_id = 0;

	if(CMD_SUCCESS != dcli_stp_get_bord_product_id(vty,&product_id)){
		return CMD_WARNING;
	}
	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									STP_DBUS_METHOD_INIT_MSTP_V1);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&(ports_bmp->vid),
							 DBUS_TYPE_UINT32,&(ports_bmp->untagbmp.portMbr[0]),
							 DBUS_TYPE_UINT32,&(ports_bmp->untagbmp.portMbr[1]),
							 DBUS_TYPE_UINT32,&(ports_bmp->tagbmp.portMbr[0]),
							 DBUS_TYPE_UINT32,&(ports_bmp->tagbmp.portMbr[1]),
							 DBUS_TYPE_UINT32,&product_id,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS == op_ret ) {
				DCLI_DEBUG(("757 :: success\n"));
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else{
				if(0xff == op_ret){
					vty_out(vty,"MSTP hasn't enable\n");
				}
				else{
					vty_out(vty,"MSTP en/disable error\n");
				}
				dbus_message_unref(reply);
				return CMD_WARNING;
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


int dcli_get_cist_info
(
	struct vty*	vty
)
{
	DBusMessage *query,*reply;
	DBusError err;
	DBusMessageIter	iter,iter_struct;

	char buf[10] = {0};

	char* pname = NULL;
	unsigned short revision = 0;
	unsigned char   mac[6] ={'\0','\0','\0','\0','\0','\0'};
	unsigned int		path_cost;
	unsigned short root_portId;
	unsigned short vid = 0;
	unsigned short  br_prio;
	unsigned short  br_maxAge;
	unsigned short  br_hTime;
	unsigned short  br_fdelay;
	unsigned char 	 br_hops;
	unsigned int    eth_g_index = 0;
	unsigned char   slot = 0,port = 0;
	int i,j,ret;

	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									MSTP_DBUS_METHOD_SHOW_CIST_INFO);
	
	dbus_error_init(&err);	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_next(&iter);

	/*vty_out(vty,"STP>> slot_count = %d\n",slot_count);*/
	if(DCLI_STP_OK == ret) {

	dbus_message_iter_recurse(&iter,&iter_struct);

	vty_out(vty,"\nRoot Bridge\t\t:  ");
	for(i = 0; i< 5; i++)
	{
		dbus_message_iter_get_basic(&iter_struct,&mac[i]);
		vty_out(vty,"%02x:",mac[i]);
		dbus_message_iter_next(&iter_struct);
	}

		dbus_message_iter_get_basic(&iter_struct,&mac[i]);
		vty_out(vty,"%02x",mac[5]);
		dbus_message_iter_next(&iter_struct);

		vty_out(vty,"\nRoot Priority\t\t:  ");
		dbus_message_iter_get_basic(&iter_struct,&br_prio);
		vty_out(vty,"%d\n",br_prio);
		dbus_message_iter_next(&iter_struct);

		vty_out(vty,"Root Path Cost\t\t:  ");
		dbus_message_iter_get_basic(&iter_struct,&path_cost);
		vty_out(vty,"%d\n",path_cost);
		dbus_message_iter_next(&iter_struct);

		vty_out(vty,"Root Port\t\t:  ");
		dbus_message_iter_get_basic(&iter_struct,&root_portId);
		if(root_portId){
			eth_g_index = root_portId& 0xfff;
			GetSlotPortFromPortIndex(eth_g_index,&slot,&port);
			vty_out(vty,"%d/%d\n",slot,port);
		}
		else{
			vty_out(vty,"%s\n","none");
		}
		dbus_message_iter_next(&iter_struct);

		/*DCLI_DEBUG(("dcli_common872 :: END suc dcli_get_cist_info\n"));*/
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}
	else if(DCLI_STP_NO_SUCH_MSTID == ret)
	{
		DCLI_DEBUG(("dcli_common879 :: END no ins dcli_get_cist_info\n"));
		dbus_message_unref(reply);
		return DCLI_STP_NO_SUCH_MSTID;
	}
	else if(STP_DISABLE == ret)
	{
		dbus_message_unref(reply);
		return ret;
	}
	else
	{
		dbus_message_unref(reply);
		return ret;
	}
	
}


int dcli_get_msti_info
(
	struct vty*	vty,
	int 				mstid
)
{
	DBusMessage *query,*reply;
	DBusError err;
	DBusMessageIter	iter,iter_struct,iter_sub_array,iter_sub_struct;

	char buf[10] = {0};

	char* pname = NULL;
	unsigned short revision = 0;
	unsigned char   mac[6] ={'\0','\0','\0','\0','\0','\0'};
	unsigned int		path_cost;
	unsigned short root_portId;
	unsigned short vid = 0;
	unsigned short  br_prio;
	unsigned short  br_maxAge;
	unsigned short  br_hTime;
	unsigned short  br_fdelay;
	unsigned char 	 br_hops;
	unsigned int    eth_g_index = 0;
	unsigned char   slot = 0,port = 0;
	int i,j,ret;

	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									MSTP_DBUS_METHOD_SHOW_MSTI_INFO);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&mstid,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_next(&iter);
	/*vty_out(vty,"STP>> slot_count = %d\n",slot_count);*/
	if(DCLI_STP_OK == ret) {

		dbus_message_iter_recurse(&iter,&iter_struct);

		vty_out(vty,"\nRegion Root\t\t:  ");
		for(i = 0; i< 5; i++)
		{
			dbus_message_iter_get_basic(&iter_struct,&mac[i]);
			vty_out(vty,"%02x:",mac[i]);
			dbus_message_iter_next(&iter_struct);
		}

		dbus_message_iter_get_basic(&iter_struct,&mac[i]);
		vty_out(vty,"%02x",mac[5]);
		dbus_message_iter_next(&iter_struct);

		vty_out(vty,"\nRegion Root Priority\t:  ");
		dbus_message_iter_get_basic(&iter_struct,&br_prio);
		vty_out(vty,"%d\n",br_prio);
		dbus_message_iter_next(&iter_struct);

		vty_out(vty,"Region Root Path Cost\t:  ");
		dbus_message_iter_get_basic(&iter_struct,&path_cost);
		vty_out(vty,"%d\n",path_cost);
		dbus_message_iter_next(&iter_struct);

		vty_out(vty,"Region Root Port\t:  ");
		dbus_message_iter_get_basic(&iter_struct,&root_portId);
		if(root_portId){
			eth_g_index = root_portId&0xfff;
			GetSlotPortFromPortIndex(eth_g_index,&slot,&port);
			vty_out(vty,"%d/%d\n",slot,port);
		}
		else{
			vty_out(vty,"%s\n","none");
		}
		dbus_message_iter_next(&iter_struct);

		vty_out(vty,"Root Max Age");
		dbus_message_iter_get_basic(&iter_struct,&br_maxAge);
		vty_out(vty,"%6d\t",br_maxAge);
		dbus_message_iter_next(&iter_struct);

		vty_out(vty,"Hello Time");
		dbus_message_iter_get_basic(&iter_struct,&br_hTime);
		vty_out(vty,"%5d\t\t",br_hTime);
		dbus_message_iter_next(&iter_struct);

		vty_out(vty,"Forward Delay");
		dbus_message_iter_get_basic(&iter_struct,&br_fdelay);
		vty_out(vty,"%5d\t",br_fdelay);
		dbus_message_iter_next(&iter_struct);

		vty_out(vty,"MaxHops");
		dbus_message_iter_get_basic(&iter_struct,&br_hops);
		vty_out(vty,"%5d",br_hops);
		dbus_message_iter_next(&iter_struct);
		
		/*DCLI_DEBUG(("dcli_common995:: END suc dcli_get_msti_info\n"));*/
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}
	else if(DCLI_STP_NO_SUCH_MSTID == ret)
	{
		DCLI_DEBUG(("dcli_common879 :: END no ins dcli_get_cist_info\n"));
		dbus_message_unref(reply);
		return DCLI_STP_NO_SUCH_MSTID;
	}
	else if(STP_DISABLE == ret)
	{
		dbus_message_unref(reply);
		return ret;
	}
	else
	{
		dbus_message_unref(reply);
		return ret;
	}
}

int dcli_get_br_self_info(
	struct vty*							vty,
	int mstid,
	unsigned short ** pvid,
	unsigned int* 						num
)
{
	DBusMessage *query,*reply;
	DBusError err;
	DBusMessageIter	iter,iter_struct,iter_sub_array,iter_sub_struct;

	char buf[10] = {0};

	char* pname = NULL;
	unsigned short revision = 0;
	unsigned char   mac[6] ={'\0','\0','\0','\0','\0','\0'};
	unsigned int      br_version;
	unsigned int		count = 0;
	unsigned short vid = 0,*tmp = NULL,*bufs = NULL,oldvid = 0;
	unsigned short  br_prio;
	unsigned short  br_maxAge;
	unsigned short  br_hTime;
	unsigned short  br_fdelay;
	unsigned char 	 br_hops;
	int i = 0,j=0,ret=0;

	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									MSTP_DBUS_METHOD_SHOW_SELF_INFO);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&mstid,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_next(&iter);

	/*DCLI_DEBUG(("dcli_common_stp:: 1053 ret %d\n",ret));*/
	if(DCLI_STP_OK == ret) {

		dbus_message_iter_recurse(&iter,&iter_struct);

		/*display root bridge info*/
		vty_out(vty,"-----------------------SPANNING TREE information of MSTP domain %d------------------------------\n",mstid);

		dbus_message_iter_get_basic(&iter_struct,&pname);
		vty_out(vty,"Region name\t\t:  %s",pname);
		dbus_message_iter_next(&iter_struct);

		dbus_message_iter_get_basic(&iter_struct,&revision);
		vty_out(vty,"\nBridge revision\t\t:  %d",revision);
		dbus_message_iter_next(&iter_struct);

		dbus_message_iter_get_basic(&iter_struct,&count);
		dbus_message_iter_next(&iter_struct);
		/*DCLI_DEBUG(("dcli_common_stp:: 1131 count %d\n",count);*/
		*num = count;
		if(0 != count) {
			tmp = (unsigned short*)malloc(sizeof(unsigned short)*count);
			if(NULL == tmp)
				return CMD_WARNING;
			else{
				memset(tmp,0,sizeof(unsigned short)*count);
			}
			bufs = (unsigned short*)malloc(sizeof(unsigned short)*count);
			if(NULL == bufs){
				free(tmp);
				tmp = NULL;
				return CMD_WARNING;
			}
			else{
				memset(bufs,0,sizeof(unsigned short)*count);
				*pvid = bufs;
			}
		}
		
		dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
		vty_out(vty,"\nVlan map\t\t:  ");
		if(0 == count)
			vty_out(vty,"0");
		else {
			for(i=0; i < count; i++)
			{
				dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
				dbus_message_iter_get_basic(&iter_sub_struct,&vid);
                 bufs[i] = vid; 
				 if(0 == i){
				 	 j = 0;
                     oldvid=tmp[j]=vid;
					 if(1 == count){
                         vty_out(vty,"%d",oldvid);
					 }
                  }
		          else{
		              if(1 != (vid-oldvid)){
		                   if(0 == j){
		                       vty_out(vty,"%d,",oldvid);
		                       oldvid=tmp[j]=vid;
							   if( i == (count - 1)){
		                           vty_out(vty,"%d",oldvid);
		                           break;
		                       }
		                   }
		                   else{
		                       vty_out(vty,"%d-%d,",tmp[0],tmp[j]);
		                       j=0;
		                       oldvid=tmp[j]=vid;
		                       if( i == (count - 1)){
		                           vty_out(vty,"%d",oldvid);
		                            break;
		                       }
		                   }
		              }
		              else{
		                   j++;
		                   oldvid=tmp[j]=vid;
		                    if( i == (count - 1)){
		                         vty_out(vty,"%d-%d",tmp[0],tmp[j]);
		                    }
		              }
		              
		          }
				dbus_message_iter_next(&iter_sub_struct);
				dbus_message_iter_next(&iter_sub_array);
			}
			free(tmp);
			tmp = NULL;
		}	
		dbus_message_iter_next(&iter_struct);
		vty_out(vty,"\nBridge ID Mac Address\t:  ");
		for(i = 0; i< 5; i++)
		{
			dbus_message_iter_get_basic(&iter_struct,&mac[i]);
			vty_out(vty,"%02x:",mac[i]);
			dbus_message_iter_next(&iter_struct);
		}

		dbus_message_iter_get_basic(&iter_struct,&mac[i]);
		vty_out(vty,"%02x",mac[5]);
		dbus_message_iter_next(&iter_struct);

		vty_out(vty,"\nBridge Priority\t\t:  ");
		dbus_message_iter_get_basic(&iter_struct,&br_prio);
		vty_out(vty,"%d\n",br_prio);
		dbus_message_iter_next(&iter_struct);

		vty_out(vty,"Bridge Force Version\t:  ");
		dbus_message_iter_get_basic(&iter_struct,&br_version);
		vty_out(vty,"%d\n",br_version);
		dbus_message_iter_next(&iter_struct);

		vty_out(vty,"Root Max Age");
		dbus_message_iter_get_basic(&iter_struct,&br_maxAge);
		vty_out(vty,"%6d\t",br_maxAge);
		dbus_message_iter_next(&iter_struct);

		vty_out(vty,"Hello Time");
		dbus_message_iter_get_basic(&iter_struct,&br_hTime);
		vty_out(vty,"%5d\t\t",br_hTime);
		dbus_message_iter_next(&iter_struct);

		vty_out(vty,"Forward Delay");
		dbus_message_iter_get_basic(&iter_struct,&br_fdelay);
		vty_out(vty,"%5d\t",br_fdelay);
		dbus_message_iter_next(&iter_struct);

		vty_out(vty,"MaxHops");
		dbus_message_iter_get_basic(&iter_struct,&br_hops);
		vty_out(vty,"%5d\n",br_hops);
		dbus_message_iter_next(&iter_struct);

		/*DCLI_DEBUG(("dcli_common1134:: END success dcli_get_br_self_info\n"));*/
		dbus_message_unref(reply);
		return CMD_SUCCESS;

	}
	else if(DCLI_STP_NO_SUCH_MSTID == ret)
	{
		DCLI_DEBUG(("dcli_common879 :: END no ins dcli_get_cist_info\n"));
		dbus_message_unref(reply);
		return DCLI_STP_NO_SUCH_MSTID;
	}
	else if(STP_DISABLE == ret)
	{
		dbus_message_unref(reply);
		return ret;
	}
	else
	{
		dbus_message_unref(reply);
		return ret;
	}	
	
}

int dcli_get_mstp_one_port_info
(
	struct vty*	vty,
	int 				mstid,
	unsigned int port_index
)
{
	DBusMessage *stpQuery,*stpReply;
	DBusError err;
	DBusMessageIter	 stpIter;
	DBusMessageIter	 stpIter_struct;
	DBusMessageIter	 stpIter_sub_struct;

	char buf[10] = {0};
	int n,ret;

	unsigned char  port_prio;
	unsigned int 	port_cost;
	int 					port_role;
	int 					port_state;
	int 					port_lk;
	int 					port_p2p;
	int 					port_edge;
	unsigned short br_prio;
	unsigned char   mac[6] ={'\0','\0','\0','\0','\0','\0'};
	unsigned int     br_cost;
	unsigned short br_dPort;

	stpQuery = dbus_message_new_method_call(			\
								RSTP_DBUS_NAME,		\
								RSTP_DBUS_OBJPATH,	\
								RSTP_DBUS_INTERFACE, \
								MSTP_DBUS_METHOD_GET_PORT_INFO);

	dbus_message_append_args(stpQuery,
					 DBUS_TYPE_UINT32,&mstid,
					 DBUS_TYPE_UINT32,&(port_index),
					 DBUS_TYPE_INVALID);
	dbus_error_init(&err);
	stpReply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,stpQuery,-1, &err);


	dbus_message_unref(stpQuery);
	if (NULL == stpReply) {
		vty_out(vty,"failed get stp stpReply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(stpReply);
		return CMD_WARNING;
	}

	dbus_message_iter_init(stpReply,&stpIter);				
	dbus_message_iter_get_basic(&stpIter,&ret);
	dbus_message_iter_next(&stpIter);
	if(DCLI_STP_OK == ret)
	{
	/*		
		Array of Port Infos.
		port no
		port prio
		port role
		port State
		port link
		port p2p
		port edge
		port Desi bridge
		port Dcost
		port D-port
	*/

		dbus_message_iter_recurse(&stpIter,&stpIter_struct);

		dbus_message_iter_get_basic(&stpIter_struct,&port_prio);
		
		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&port_cost);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&port_role);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&port_state);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&port_lk);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&port_p2p);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&port_edge);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_get_basic(&stpIter_struct,&br_cost);			

		dbus_message_iter_next(&stpIter_struct);			
		dbus_message_iter_get_basic(&stpIter_struct,&br_dPort);

		dbus_message_iter_next(&stpIter_struct);
		dbus_message_iter_recurse(&stpIter_struct,&stpIter_sub_struct);


		dbus_message_iter_get_basic(&stpIter_sub_struct,&br_prio);
		dbus_message_iter_next(&stpIter_sub_struct);

		for(n = 0; n < 6; n++)
		{
			dbus_message_iter_get_basic(&stpIter_sub_struct,&mac[n]);
			dbus_message_iter_next(&stpIter_sub_struct);
		}						
		vty_out(vty,"%-4d",port_prio);
		vty_out(vty,"%-9d",port_cost);
		vty_out(vty,"%-5s",stp_port_role[port_role]);
		vty_out(vty,"%-13s",stp_port_state[port_state]);
		vty_out(vty,"%-3s", port_lk ? "Y" : "N");

		if(0 == port_p2p)
			vty_out(vty,"%-4s","N");
		else if(1 == port_p2p)
			vty_out(vty,"%-4s","Y");
		else if(2 == port_p2p)
				vty_out(vty,"%-4s","A");
		
		vty_out(vty,"%-4s",port_edge ? "Y" : "N");

		memset(buf,0,sizeof(buf));
		sprintf(buf,"%d",br_prio);
		if(port_state)
		{				
			vty_out(vty,"%-5s:",port_state ? buf : "");
			for(n = 0; n < 6; n++)
			{
				vty_out(vty,"%02x",mac[n]);
			}
			vty_out(vty,"  ");
		}
		
		memset(buf,0,sizeof(buf));
		sprintf(buf,"%d",br_cost);
		if((strcmp("NStp",stp_port_role[port_role]))){
		   vty_out(vty,"%-10s",port_state ? buf : "");
		}
		else {
			vty_out(vty,"%-10s","     -");
		}
		memset(buf,0,sizeof(buf));
		sprintf(buf,"%#0x",br_dPort);
		vty_out(vty,"%s",port_state ? buf : "");
				
		vty_out(vty,"\n");

		dbus_message_unref(stpReply);

		return CMD_SUCCESS;
	}
	else if(STP_DISABLE == ret)
	{
		dbus_message_unref(stpReply);
		return ret;
	}
	else
	{
		dbus_message_unref(stpReply);
		return ret;
	}	
		
}

/**************************************************************************
*
*	command func
*
*
***************************************************************************/

int dcli_set_bridge_priority
(	
	DCLI_STP_RUNNING_MODE mode,
	unsigned int prio
)
{
}

int dcli_set_bridge_max_age
(
	DCLI_STP_RUNNING_MODE mode,
	unsigned int max_age
)
{
}

int dcli_set_bridge_hello_time
(
	DCLI_STP_RUNNING_MODE mode,
	unsigned int htime
)
{
}

int dcli_set_bridge_forward_delay
(
	DCLI_STP_RUNNING_MODE mode,
	unsigned int fdelay
)
{
}

int dcli_set_bridge_force_version
(
	struct vty* vty,
	unsigned int fversion
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret = 0;
	unsigned int value = fversion;
	
	query = dbus_message_new_method_call(
										RSTP_DBUS_NAME,				\
										RSTP_DBUS_OBJPATH,			\
										RSTP_DBUS_INTERFACE,		\
										RSTP_DBUS_METHOD_CONFIG_FORVERSION);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&value,
							DBUS_TYPE_INVALID);
	dbus_error_init(&err);
 
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);

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

		/*vty_out(vty,"%s, ret %02x\n",__FILE__,ret);*/
		if(STP_DISABLE == ret){
			if(STP_NODE == vty->node)
				vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
			else if(MST_NODE == vty->node)
				vty_out(vty,PRINTF_MSTP_NOT_ENABLED);
		}
	} 
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int dcli_set_bridge_max_hops
(
	DCLI_STP_RUNNING_MODE mode,
	unsigned int maxhops
)
{

}

int dcli_set_bridge_default_value
(
	DCLI_STP_RUNNING_MODE mode
)
{

}

int dcli_set_port_priority
(
	DCLI_STP_RUNNING_MODE mode,
	unsigned int mstid,
	unsigned int port_index,
	unsigned int value
)
{

}


#if 0
unsigned char dcli_get_all_ports_index
(
	struct vty*	 vty, 		
	SLOT_INFO* slot_info
)
{
	DBusMessage *npdQuery, *npdReply;
	DBusError err;
	DBusMessageIter	 npdIter;
	DBusMessageIter	 npdIter_array;

	unsigned char slot_count;

	int i,j;

	dbus_error_init(&err);
	npdQuery = dbus_message_new_method_call(  \
												NPD_DBUS_BUSNAME, 	\
												NPD_DBUS_ETHPORTS_OBJPATH,	\
												NPD_DBUS_ETHPORTS_INTERFACE, \
												STP_DBUS_METHOD_GET_ALL_PORTS_INDEX
												);

	
	npdReply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,npdQuery,-1, &err);
	dbus_message_unref(npdQuery);
	if (NULL == npdReply) {
		vty_out(vty,"failed get slot count npdReply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(npdReply);
		return CMD_WARNING;
	}

	
	dbus_message_iter_init(npdReply,&npdIter);
	dbus_message_iter_get_basic(&npdIter,&slot_count);

	//vty_out(vty,"STP>> slot_count = %d\n",slot_count);

	dbus_message_iter_next(&npdIter);
	
	dbus_message_iter_recurse(&npdIter,&npdIter_array);

	for (i = 0; i < slot_count; i++) {
		DBusMessageIter npdIter_struct;
		DBusMessageIter npdIter_sub_array;
		
		//vty_out(vty,"\n**************************************\n");
		dbus_message_iter_recurse(&npdIter_array,&npdIter_struct);
		
		dbus_message_iter_get_basic(&npdIter_struct,&slot_info[i].slot_no);
		//vty_out(vty,"slotno %d\n",slot_info[i].slot_no);
		
		dbus_message_iter_next(&npdIter_struct);
		dbus_message_iter_get_basic(&npdIter_struct,&slot_info[i].local_port_count);
		//vty_out(vty,"local port count %d\n",slot_info[i].local_port_count);
		
		
		dbus_message_iter_next(&npdIter_struct);
		dbus_message_iter_recurse(&npdIter_struct,&npdIter_sub_array);
		
		for (j = 0; j < slot_info[i].local_port_count; j++) {
			DBusMessageIter npdIter_sub_struct;
			
			dbus_message_iter_recurse(&npdIter_sub_array,&npdIter_sub_struct);
			
			dbus_message_iter_get_basic(&npdIter_sub_struct,&slot_info[i].port_no[j].local_port_no);
			dbus_message_iter_next(&npdIter_sub_struct);
			//vty_out(vty,"local port no %d\n",slot_info[i].port_no[j].local_port_no);
			
			dbus_message_iter_get_basic(&npdIter_sub_struct,&slot_info[i].port_no[j].port_index);
			dbus_message_iter_next(&npdIter_sub_struct);
			//vty_out(vty,"local port no %d\n",slot_info[i].port_no[j].port_index);
			
			dbus_message_iter_next(&npdIter_sub_array);
		
		}
		
		dbus_message_iter_next(&npdIter_array);
		
	}	
		
	dbus_message_unref(npdReply);

	return slot_count;
}
#endif
