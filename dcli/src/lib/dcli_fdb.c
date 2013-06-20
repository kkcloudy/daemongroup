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
* dcli_fdb.c
*
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		CLI definition for FDB module.
*
* DATE:
*		02/21/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.67 $	
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

#include "dcli_fdb.h"
#include "dcli_main.h"
#include "npd/nbm/npd_bmapi.h"
#include "sysdef/returncode.h"
#include "dcli_sem.h"

extern int is_distributed;

#define SFD_ON		1
#define SFD_OFF		0
int sfd_debug_flag=SFD_OFF;
int sfd_flag=SFD_OFF;


extern DBusConnection *dcli_dbus_connection;
extern int toupper(int c);
/* delete this func*/
void dcli_fdb_output(unsigned int type) {
	switch (type){
	case NPD_FDB_ERR_NONE :
		break;
	case FDB_RETURN_CODE_GENERAL            :
		printf("Error occurs in config on SW.\n");
		break;
	case FDB_RETURN_CODE_NODE_EXIST :
		printf("fdb item exist!\n");
		
		break;
	case FDB_RETURN_CODE_NODE_NOT_EXIST:
		printf("fdb item not exist !\n");
		
		break;
	case FDB_RETURN_CODE_NODE_PORT_NOTIN_VLAN:
		printf("port not in vlan !\n");
		
		break;
	case FDB_RETURN_CODE_NODE_VLAN_NONEXIST:
		printf("vlan non-exist !\n");
		
		break;
	case FDB_RETURN_CODE_SYSTEM_MAC:
		printf("MAC address conflict with system MAC address!\n");
		
		break;
	default :
		printf("operate error !\n");
		
	}
}

int is_muti_brc_mac(ETHERADDR *mac)
{
  if(mac->arEther[0] & 0x1)
  	return 1;
  else{ return 0;}
}
int mac_format_check
(
	char* str,
	int len
) 
{
	int i = 0;
	unsigned int result = NPD_SUCCESS;
	char c = 0;
	
	if( 17 != len){
	   return NPD_FAIL;
	}
	for(;i<len;i++) {
		c = str[i];
		if((2 == i)||(5 == i)||(8 == i)||(11 == i)||(14 == i)){
			if((':'!=c)&&('-'!=c))
				return NPD_FAIL;
		}
		else if((c>='0'&&c<='9')||
			(c>='A'&&c<='F')||
			(c>='a'&&c<='f'))
			continue;
		else {
			result = NPD_FAIL;
			return result;
		}
    }
	if((str[2] != str[5])||(str[2] != str[8])||(str[2] != str[11])||(str[2] != str[14])||
		(str[5] != str[8])||(str[5] != str[11])||(str[5] != str[14])||
		(str[8] != str[11])||(str[8] != str[14])){
		
        result = NPD_FAIL;
		return result;
	}
	return result;
}
int parse_vlan_string(char* input){
	int i;
	 char c;
	 if(NULL == input) {
		 return NPD_FAIL;
	 }
	 c=input[0];
	 
	if ((c>='A'&&c<='Z')||(c>='a'&&c<='z')||('_'==c)){
	 	return 1;
		}
	return NPD_FAIL;
	
	 }

 int parse_mac_addr(char* input,ETHERADDR* macAddr) {
 	
	int i = 0;
	char cur = 0,value = 0;
	
	if((NULL == input)||(NULL == macAddr)) {
		return NPD_FAIL;
	}
	if(NPD_FAIL == mac_format_check(input,strlen(input))) {
		return NPD_FAIL;
	}
	
	for(i = 0; i <6;i++) {
		cur = *(input++);
		if((cur == ':') ||(cur == '-')){
			i--;
			continue;
		}
		if((cur >= '0') &&(cur <='9')) {
			value = cur - '0';
		}
		else if((cur >= 'A') &&(cur <='F')) {
			value = cur - 'A';
			value += 0xa;
		}
		else if((cur >= 'a') &&(cur <='f')) {
			value = cur - 'a';
			value += 0xa;
		}
		macAddr->arEther[i] = value;
		cur = *(input++);	
		if((cur >= '0') &&(cur <='9')) {
			value = cur - '0';
		}
		else if((cur >= 'A') &&(cur <='F')) {
			value = cur - 'A';
			value += 0xa;
		}
		else if((cur >= 'a') &&(cur <='f')) {
			value = cur - 'a';
			value += 0xa;
		}
		macAddr->arEther[i] = (macAddr->arEther[i]<< 4)|value;
	}

	/* cheak if mac == 00:00:00:00:00:00, is illegal .zhangdi@autelan.com 2011-07-11 */
    if( (macAddr->arEther[0]==0x00)&&(macAddr->arEther[1]==0x00)&&(macAddr->arEther[2]==0x00)&&   \
		(macAddr->arEther[3]==0x00)&&(macAddr->arEther[4]==0x00)&&(macAddr->arEther[5]==0x00) )
    {
		return NPD_FAIL;		
    }
	
	return NPD_SUCCESS;
} 

/**************************************
* update global static fdb
*
*input Params: 
*	 vty --       
*       mac:MAC ADDRESS
*       vid:vlan ID
*	 slot:SLOT ID
*       port:PORT NUMBER
*       trunkID:trunk NUMBER
*       flag:just use for backlist
*       flag_type: use for global update  
*			   0 represent  add static mac address by slot port and vlan
*              1 represent add static mac address by  vlan and trunk
*			   2 represent add static blacklist 
*			   3 represent  del static mac address by port
*			   4 represent del static mac address by vlan and mac 
*			   5 represent del static mac address by vlan
*			   6 represebt del static blacklist	
*output params: none
*
****************************************/


int dcli_update_gstaticfdb_for_master(struct vty* vty,
											ETHERADDR mac,
											unsigned short vid,
											unsigned char slot,
											unsigned char port,
											unsigned char trunkId,
											unsigned char flag,
											unsigned char flag_type)
{
	DBusError err;
	unsigned int 	op_ret = 0;
	DBusMessage *query = NULL, *reply = NULL;

	int i = 0,slot_id =0,ret=0;	
   	int master_slot_id[2] = {-1, -1};	
	char *master_slot_cnt_file = "/dbm/product/master_slot_count";		
    int master_slot_count = get_product_info(master_slot_cnt_file);
	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);

   	ret = dcli_master_slot_id_get(master_slot_id);
	if(ret !=0 )
	{
		vty_out(vty,"get master_slot_id error !\n");
		return CMD_WARNING;		
   	}
	if((local_slot_id<0)||(master_slot_count<0))
	{
		vty_out(vty,"get get_product_info return -1 !\n");
		return CMD_WARNING;		
   	}
	
    for(i=0;i<master_slot_count;i++)
    {

		slot_id = master_slot_id[i];
    	query = NULL;
    	reply = NULL;

		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_FDB_OBJPATH,	\
											NPD_DBUS_FDB_INTERFACE,	\
											NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_SAVE);
	
		dbus_error_init(&err);

		dbus_message_append_args(	query,
							 		DBUS_TYPE_UINT16,&vid,
								 	DBUS_TYPE_BYTE,&mac.arEther[0],
								 	DBUS_TYPE_BYTE,&mac.arEther[1],
								 	DBUS_TYPE_BYTE,&mac.arEther[2],
								 	DBUS_TYPE_BYTE,&mac.arEther[3],
								 	DBUS_TYPE_BYTE,&mac.arEther[4],
								 	DBUS_TYPE_BYTE,&mac.arEther[5],
								 	DBUS_TYPE_BYTE,&slot,
								 	DBUS_TYPE_BYTE,&port,
								 	DBUS_TYPE_BYTE,&trunkId,
								 	DBUS_TYPE_BYTE,&flag,
								 	DBUS_TYPE_BYTE,&flag_type,
							 		DBUS_TYPE_INVALID);
    	
        if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) 				
    	{
			if(slot_id == local_slot_id)
			{
                reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
			}
			else 
			{	
			    /* here do not print "Can not connect to MCB slot:5 " */	
				continue;   /* for next MCB */
			}
        }
    	else
    	{
            reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);				
    	}
    	
    	dbus_message_unref(query);
    	if (NULL == reply) {
    		vty_out(vty,"Please check npd on MCB slot %d\n",slot_id);
    		return CMD_SUCCESS;
    	}

    	if (dbus_message_get_args ( reply, &err,
    		DBUS_TYPE_UINT32,&op_ret,
    		DBUS_TYPE_INVALID)) {
    		if (CMD_SUCCESS != op_ret){
    			vty_out(vty,"%% create static fdb  Error! return %d \n",op_ret);
    		}		
    	} 
    	else {
    		vty_out(vty,"Failed get args,Please check npd on MCB slot %d\n",slot_id);
    	}
    	dbus_message_unref(reply);
    	if(CMD_SUCCESS != op_ret)
    	{
    		return CMD_WARNING;
    	}
    }
	return CMD_SUCCESS;	
}


int dcli_check_static_fdb_exists(struct vty* vty,
											ETHERADDR mac,
											unsigned short vid,
											unsigned char *slot,
											unsigned char *port,
											unsigned char *flag)
{
	DBusError err;
	unsigned int 	op_ret = 0;
	DBusMessage *query = NULL, *reply = NULL;

	int i = 0,slot_id =0,ret=0;	
   	int master_slot_id[2] = {-1, -1};	
	char *master_slot_cnt_file = "/dbm/product/master_slot_count";		
    int master_slot_count = get_product_info(master_slot_cnt_file);
	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);

   	ret = dcli_master_slot_id_get(master_slot_id);
	if(ret !=0 )
	{
		vty_out(vty,"get master_slot_id error !\n");
		return CMD_WARNING;		
   	}
	if((local_slot_id<0)||(master_slot_count<0))
	{
		vty_out(vty,"get get_product_info return -1 !\n");
		return CMD_WARNING;		
   	}
	
    for(i=0;i<1;i++)
    {

		slot_id = master_slot_id[i];
    	query = NULL;
    	reply = NULL;

		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_FDB_OBJPATH,	\
											NPD_DBUS_FDB_INTERFACE,	\
											NPD_DBUS_FDB_METHOD_CHECK_FDB_STATIC_EXISTS);
	
		dbus_error_init(&err);

		dbus_message_append_args(	query,
							 		DBUS_TYPE_UINT16,&vid,
								 	DBUS_TYPE_BYTE,&mac.arEther[0],
								 	DBUS_TYPE_BYTE,&mac.arEther[1],
								 	DBUS_TYPE_BYTE,&mac.arEther[2],
								 	DBUS_TYPE_BYTE,&mac.arEther[3],
								 	DBUS_TYPE_BYTE,&mac.arEther[4],
								 	DBUS_TYPE_BYTE,&mac.arEther[5],
							 		DBUS_TYPE_INVALID);
    	
        if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) 				
    	{
			if(slot_id == local_slot_id)
			{
                reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
			}
			else 
			{	
			    /* here do not print "Can not connect to MCB slot:5 " */	
				continue;   /* for next MCB */
			}
        }
    	else
    	{
            reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);				
    	}
    	
    	dbus_message_unref(query);
    	if (NULL == reply) {
    		vty_out(vty,"Please check npd on MCB slot %d\n",slot_id);
    		return CMD_SUCCESS;
    	}

    	if (dbus_message_get_args ( reply, &err,
    		DBUS_TYPE_UINT32,&op_ret,
    		DBUS_TYPE_BYTE,slot,
    		DBUS_TYPE_BYTE,port,
    		DBUS_TYPE_INVALID)) {
    		if (CMD_SUCCESS == op_ret){
					vty_out(vty,"The static fdb does not exists !\n");
    		}		
    	} 
    	else {
    		vty_out(vty,"Failed get args,Please check npd on MCB slot %d\n",slot_id);
    	}
    	dbus_message_unref(reply);
    	if(FDB_RETURN_CODE_NODE_EXIST != op_ret)
    	{
    		return CMD_WARNING;
    	}
		else
		{
			*flag = 1;
		}
    }
	return CMD_SUCCESS;	
}


/**************************************
* config fdb agingtime on Both Sw & Hw. 
*
* Usage: config fdb agingtime (<10-630>|0) 
*
*input Params: 
*       agingtime value:	<10-630|0>agingtime skip value is 10s.
*       param 0 means fdb table is not aging.
*
*output params: none
*
****************************************/

DEFUN(config_fdb_agingtime_cmd_func,
	config_fdb_agingtime_cmd,
	"config fdb agingtime (<10-630>|0)",
	"Config system information\n"
	"FDB table \n"
	"FDB table aging time Range <10-630s>|0\n"
	"Config fdb aging value <10-630>\n"
	"Config fdb aging out 0\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int testAgingtime=0;
	unsigned int 	op_ret = 0;
	int i;
	int local_slot_id = 0;
    int slotNum = 0;

	
	op_ret = parse_int_parse((char *)argv[0],&testAgingtime);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"FDB agingtime form erro!\n");
		return CMD_WARNING;
	}
	if   (testAgingtime ==0){
		;
		}
    else if ((testAgingtime < 10) ||(testAgingtime > 630)){
		vty_out(vty,"FDB agingtime is outrange!\n");
		vty_out(vty,"FDB agingtime %d. \n",testAgingtime);
		return CMD_WARNING;
		}
	/*vty_out(vty,"FDB agingtime %d. \n",agingtime);*/

	if(is_distributed == DISTRIBUTED_SYSTEM)	
	{
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		for(i=1; i <= slotNum; i++)
		{
			query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_AGINGTIME);
			
			dbus_error_init(&err);

			dbus_message_append_args(	query,
									 	DBUS_TYPE_UINT32,&testAgingtime,
									 	DBUS_TYPE_INVALID);


			if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
			{
				if(i == local_slot_id)
				{
               		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
			   		vty_out(vty,"Can not connect to slot:%d \n",i);	
					continue;
				}
        	}
			else
			{
            	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);				
			}
			
			dbus_message_unref(query);
			if (NULL == reply) {
				vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",i);
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_WARNING;
			}

			if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&op_ret,
				DBUS_TYPE_UINT32,&testAgingtime,
				DBUS_TYPE_INVALID)) {
					/*dcli_fdb_output(op_ret);*/
					if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
						vty_out(vty,"% Bad Parameters, the aging time input is illegal.\n");
					}
					else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
						vty_out(vty,"%% Error,failed when config hw.\n ");
					}
					else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
						vty_out(vty,"%% Product not support this function!\n");
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
		}
	}
	else
	{
		
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_AGINGTIME);
		
		dbus_error_init(&err);

		dbus_message_append_args(	query,
								 	DBUS_TYPE_UINT32,&testAgingtime,
								 	DBUS_TYPE_INVALID);
		
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
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
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_UINT32,&testAgingtime,
			DBUS_TYPE_INVALID)) {
				/*dcli_fdb_output(op_ret);*/
				if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
					vty_out(vty,"% Bad Parameters, the aging time input is illegal.\n");
				}
				else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
					vty_out(vty,"%% Error,failed when config hw.\n ");
				}
				else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
					vty_out(vty,"%% Product not support this function!\n");
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
	}
	return CMD_SUCCESS;
}



/**************************************
*show fdb agingtime on Both Sw & Hw. 
*
* Usage: show fdb agingtime 
*
*input Params: none
*
*output params:
*       agingtime value:	<10-630|0>agingtime .
*       param 0 means fdb table is not aging.
*
****************************************/

DEFUN(show_fdb_agingtime_cmd_func,
	show_fdb_agingtime_cmd,
	"show fdb agingtime",
	"Show system information\n"
	"FDB table\n"
	"FDB table aging time range in <10-630>s"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int testAgingtime = 0;
	unsigned int 	op_ret = 0;
	int i;
	int local_slot_id = 0;
    int slotNum = 0;

	
	if(is_distributed == DISTRIBUTED_SYSTEM)	
	{
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		for(i=1; i <= slotNum; i++)
		{
			query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_AGINGTIME);

			dbus_error_init(&err);

			if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
			{
				if(i == local_slot_id)
				{
               		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
			   		vty_out(vty,"Can not connect to slot:%d \n",i);	
					continue;
				}
        	}
			else
			{
            	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);				
			}
			dbus_message_unref(query);
			if (NULL == reply) {
				vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",i);
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_WARNING;
			}

			if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&op_ret,
			 	DBUS_TYPE_UINT32,&testAgingtime,
				DBUS_TYPE_INVALID)) {
				if (FDB_RETURN_CODE_OCCUR_HW == op_ret ) {
					vty_out(vty,"%% Error,failed when config hw.\n ");
				}
				else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
					vty_out(vty,"%% Product not support this function!\n");
				}
				else{
			        vty_out(vty,"The FDB AGINGTIME is %d\n",testAgingtime);
				}
			}
			dbus_message_unref(reply);
		}
	}
	else
	{
		
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_AGINGTIME);

		dbus_error_init(&err);

		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

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
			DBUS_TYPE_UINT32,&op_ret,
		 	DBUS_TYPE_UINT32,&testAgingtime,
			DBUS_TYPE_INVALID)) {
			if (FDB_RETURN_CODE_OCCUR_HW == op_ret ) {
				vty_out(vty,"%% Error,failed when config hw.\n ");
			}
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
				vty_out(vty,"%% Product not support this function!\n");
			}
			else{
		        vty_out(vty,"The FDB AGINGTIME is %d\n",testAgingtime);
			}
		}
		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;
}


/**************************************
*config fdb agingtime on Both Sw & Hw. 
*
* Usage:config fdb agingtime default 
*
*input Params: none
*
*output params:none
*     
****************************************/

DEFUN(config_fdb_no_config_agingtime_cmd_func,
	config_fdb_no_agingtime_cmd,
	"config fdb agingtime default ",
	"Config system information!\n"
	"Config FDB table information!\n"
	"Config FDB aging time default value!\n"
	"FDB aging time default value is 300s!\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int testAgingtime = 300;
	unsigned int 	ret = 0;
	/*agingtime = testagingtime;*/
	int i;
	int local_slot_id = 0;
    int slotNum = 0;

	if(is_distributed == DISTRIBUTED_SYSTEM)	
	{
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		for(i=1; i <= slotNum; i++)
		{
	
			query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_DEFAULT_AGINGTIME);
			
			dbus_error_init(&err);

			dbus_message_append_args(	query,
									 	DBUS_TYPE_UINT32,&testAgingtime,
									 	DBUS_TYPE_INVALID);
			
			if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
			{
				if(i == local_slot_id)
				{
               		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
			   		vty_out(vty,"Can not connect to slot:%d \n",i);	
					continue;
				}
        	}
			else
			{
            	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);				
			}
			
			dbus_message_unref(query);
			if (NULL == reply) {
				vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",i);
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_WARNING;
			}
			
			if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&ret,
				DBUS_TYPE_UINT32,&testAgingtime,
				DBUS_TYPE_INVALID)) {
				    if(NPD_FDB_ERR_NONE == ret){
						vty_out(vty,"The FDB AGINGTIME is %d\n",testAgingtime);
				    }
					else if( FDB_RETURN_CODE_BADPARA == ret){
						vty_out(vty,"% Bad Parameters, the aging time input is illegal.\n");
					}
					else if(FDB_RETURN_CODE_OCCUR_HW == ret){
						vty_out(vty,"%% Error,failed when config hw.\n ");
					}
					else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
						vty_out(vty,"%% Product not support this function!\n");
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
		}
	}
	else
	{
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_DEFAULT_AGINGTIME);
	
		dbus_error_init(&err);

		dbus_message_append_args(	query,
								 	DBUS_TYPE_UINT32,&testAgingtime,
								 	DBUS_TYPE_INVALID);
		
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
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
			DBUS_TYPE_UINT32,&ret,
			DBUS_TYPE_UINT32,&testAgingtime,
			DBUS_TYPE_INVALID)) {
			    if(NPD_FDB_ERR_NONE == ret){
					vty_out(vty,"The FDB AGINGTIME is %d\n",testAgingtime);
			    }
				else if( FDB_RETURN_CODE_BADPARA == ret){
					vty_out(vty,"% Bad Parameters, the aging time input is illegal.\n");
				}
				else if(FDB_RETURN_CODE_OCCUR_HW == ret){
					vty_out(vty,"%% Error,failed when config hw.\n ");
				}
				else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
					vty_out(vty,"%% Product not support this function!\n");
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
	}
	return CMD_SUCCESS;
}


/**************************************
*config fdb static fdb table item on Both Sw & Hw. 
*
* Usage:create fdb static mac MAC vlan <1-4094> port PORTNO 
*
*input Params: 
*	MAC: destination mac form 00:00:11:11:aa:aa
*	<1-4094>: valid vlan index  range,which has been configed befor doing.
*   	PORTNO:  destination port ,form slot/port
*
*output params:none
*     
****************************************/

DEFUN(config_fdb_mac_vlan_port_static_cmd_func,
	config_fdb_mac_vlan_port_cmd,
	"create fdb static mac MAC vlan <1-4094> port PORTNO",
	"Config system information\n"
	"Config FDB table\n"
	"Find static fdb item \n"
	"Config MAC field of FDB table.\n"
	"Config MAC address in Hex format\n"
	"Config FDB table vlan Id\n"
	"Config VLAN Id valid range <1-4094>\n"
	"Config port \n"
	CONFIG_ETHPORT_STR
)
{
	DBusMessage *query, *reply;
	ETHERADDR		macAddr;
	unsigned short	vlanId = 0;
	unsigned char	slot_no = 0,port_no = 0; 
	DBusError err;
	unsigned int 	op_ret = 0;	
	int local_slot_id = 0;
    int slotNum = 0;
	
	memset(&macAddr,0,sizeof(ETHERADDR));
	
	op_ret = parse_mac_addr((char *)argv[0],&macAddr);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad Parameter,Unknow mac addr format!\n");
		return CMD_WARNING;
	}
   
	op_ret=is_muti_brc_mac(&macAddr);
	if(op_ret==1){
		vty_out(vty,"%% Erro:input should not be broadcast or multicast mac!\n");
		return CMD_WARNING;
	}	
	op_ret = parse_short_parse((char*)argv[1], &vlanId);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad Parameters,Unknow vlan id format.\n");
		return CMD_WARNING;
	}
    if ((vlanId<MIN_VLANID)||(vlanId>MAX_VLANID)){
		vty_out(vty,"% Bad Parameters,FDB vlan outrange!\n");
		return CMD_WARNING;
		}
	/*vty_out(vty,"fdb vlan id %d.\n",vlanId);*/

	
	op_ret = parse_slotport_no((char *)argv[2], &slot_no, &port_no);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad Parameters,Unknow portno format!\n");
		return CMD_WARNING;
	}
  /*  
          if ( (slot_no<MIN_SLOT || slot_no>MAX_SLOT)||(port_no<MIN_PORT ||port_no>MAX_PORT)){
		vty_out(vty,"Unknow portno format!\n");
		return NPD_FDB_ERR_NONE;
		}*/
		
	/* just according productid in npd*/
  if(is_distributed == DISTRIBUTED_SYSTEM)
	{
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}

		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
											NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC);
		
		dbus_error_init(&err);
		dbus_message_append_args(	query,
									DBUS_TYPE_UINT16,&vlanId,
								 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
								 	DBUS_TYPE_BYTE,&slot_no,
								 	DBUS_TYPE_BYTE,&port_no,
								 	DBUS_TYPE_INVALID);
		
		if(local_slot_id == slot_no)
		{
			reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		}
		else
		{
			if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection)
			{
			   	vty_out(vty,"Can not connect to slot:%d, check the slot please !\n",slot_no);	
        		return CMD_WARNING;
			}
			else
			{
				reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,query,-1, &err);
			}
		}
		
		dbus_message_unref(query);
		if (NULL == reply) {
				vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",slot_no);
				if (dbus_error_is_set(&err)) {
						vty_out(vty,"%s raised: %s",err.name,err.message);
						dbus_error_free_for_dcli(&err);
				}
				return NPD_FDB_ERR_NONE;
		}

		if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&op_ret,
				DBUS_TYPE_INVALID)) {
				if(FDB_RETURN_CODE_NODE_EXIST == op_ret)
				{
					vty_out(vty,"%% This Entry  already exists\n");
				}
				else if( FDB_RETURN_CODE_GENERAL == op_ret){
	                vty_out(vty,"%% Error,operations error when configuration!\n");
			    }
				else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == op_ret){
					vty_out(vty,"% Bad parameters,the vlan input not exist!\n");
				}
				else if(FDB_RETURN_CODE_NODE_PORT_NOTIN_VLAN == op_ret){
					vty_out(vty,"%% Error,the port is not in vlan!\n");
				}
				else if(FDB_RETURN_CODE_BADPARA == op_ret){
					vty_out(vty,"%% Error,the parameter input contains some error!\n");
				}
				else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
					vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
				}
				else if(FDB_RETURN_CODE_SYSTEM_MAC == op_ret){
					vty_out(vty,"%% Error,the mac input conflict with system mac,please chang!\n");
				}
				else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
					vty_out(vty,"%% Product not support this function!\n");
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
		if(CMD_SUCCESS == op_ret)
        {
           	/* update g_vlanlist[] on Active-MCB & Standby-MCB. zhangdi 20110804*/
        	if(is_distributed == DISTRIBUTED_SYSTEM)	
        	{
    			vty_out(vty,"update g_static_fdb on SMU !\n");			
            	/* Wherever slot the CMD we input, then update the gvlanlist[] of 2 MCBs */
        		op_ret = dcli_update_gstaticfdb_for_master(vty,macAddr,vlanId,slot_no,port_no,0,0,0);
        	    if( CMD_SUCCESS != op_ret)
        		{
        			vty_out(vty,"update g_static_fdb on SMU error !\n");					
        		    return CMD_WARNING;
        	    }
        	}					
        }
  }
  else
  {
  	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH, \
										NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC);
	
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT16,&vlanId,
								DBUS_TYPE_BYTE,&macAddr.arEther[0],
								DBUS_TYPE_BYTE,&macAddr.arEther[1],
								DBUS_TYPE_BYTE,&macAddr.arEther[2],
								DBUS_TYPE_BYTE,&macAddr.arEther[3],
								DBUS_TYPE_BYTE,&macAddr.arEther[4],
								DBUS_TYPE_BYTE,&macAddr.arEther[5],
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
			return NPD_FDB_ERR_NONE;
	}
	
	if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID)) {
			if( FDB_RETURN_CODE_GENERAL == op_ret){
				vty_out(vty,"%% Error,operations error when configuration!\n");
			}
			else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == op_ret){
				vty_out(vty,"% Bad parameters,the vlan input not exist!\n");
			}
			else if(FDB_RETURN_CODE_NODE_PORT_NOTIN_VLAN == op_ret){
				vty_out(vty,"%% Error,the port is not in vlan!\n");
			}
			else if(FDB_RETURN_CODE_BADPARA == op_ret){
				vty_out(vty,"%% Error,the parameter input contains some error!\n");
			}
			else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
				vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
			}
			else if(FDB_RETURN_CODE_SYSTEM_MAC == op_ret){
				vty_out(vty,"%% Error,the mac input conflict with system mac,please chang!\n");
			}
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
				vty_out(vty,"%% Product not support this function!\n");
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
  }
  
	return CMD_SUCCESS;
}


/**************************************
*config fdb static fdb table item on Both Sw & Hw. 
*
* Usage:create fdb static mac MAC vlan VLANNAME port PORTNO 
*
*input Params: 
*	MAC: destination mac form 00:00:11:11:aa:aa
*	VLANNAME:  before doing, the vlan name which has been configed ,
*                           VLANNAME must be to begin with letter or '_'
*   	PORTNO:  destination port, form slot/port
*
*output params:none
*     
****************************************/

DEFUN(config_fdb_mac_vlan_port_static_name_cmd_func,
	config_fdb_mac_vlan_port_name_cmd,
	"create fdb static mac MAC vlan VLANNAME port PORTNO",
	"Config system information\n"
	"Config FDB table\n"
	"Static fdb item \n"
	"Config MAC field of FDB table.\n"
	"Config MAC address in Hexa.Eg 00:00:11:11:aa:aa\n"
	"Config FDB table vlanname\n"
	"VLANNAME must be to begin with letter or '_'\n"
	"Config port \n"
	CONFIG_ETHPORT_STR
)
{
	DBusMessage *query, *reply;
	DBusError err;
	char* vlanname = NULL;
	unsigned short	vlanId = 0;
	unsigned char	slot_no = 0,port_no = 0; 
	int 	op_ret = 0;
	unsigned int len = 0;
	int local_slot_id = 0;
    int slotNum = 0;
	ETHERADDR		macAddr;
	memset(&macAddr,0,sizeof(ETHERADDR));	
	op_ret = parse_mac_addr((char *)argv[0],&macAddr);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad Parameter,Unknow mac addr format!\n");
		return CMD_SUCCESS;
	}
	
	op_ret=is_muti_brc_mac(&macAddr);
	if(op_ret==1){
		vty_out(vty,"%% Erro:input should not be broadcast or multicast mac!\n");
		return CMD_SUCCESS;
	}
			
   op_ret =parse_vlan_string((char*)argv[1]);
   if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad Parameters,vlanname form erro!\n");
		return NPD_FDB_ERR_NONE;
	}

    vlanname= (char *)argv[1];
	op_ret = parse_slotport_no((char *)argv[2], &slot_no, &port_no);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad Parameters,Unknow portno format!\n");
		return NPD_FDB_ERR_NONE;
	}
	/* SHOULD BE CHECKED IN NPD!!!!
    if ((slot_no<MIN_SLOT || slot_no>MAX_SLOT)||(port_no<MIN_PORT ||port_no>MAX_PORT)){
		vty_out(vty,"Unknow portno format!\n");
		return NPD_FDB_ERR_NONE;
		}
    */
	 if(is_distributed == DISTRIBUTED_SYSTEM)
	{
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_WITH_NAME);
		
		dbus_error_init(&err);
		dbus_message_append_args(	query,
									DBUS_TYPE_STRING,&vlanname,
								 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
								 	DBUS_TYPE_BYTE,&slot_no,
								 	DBUS_TYPE_BYTE,&port_no,
								 	DBUS_TYPE_INVALID);
		
		if(local_slot_id == slot_no)
		{
			reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		}
		else
		{
			if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection)
			{
			   	vty_out(vty,"Can not connect to slot:%d, check the slot please !\n",slot_no);	
        		return CMD_WARNING;
			}
			else
			{
				reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,query,-1, &err);
			}
		}
		
		dbus_message_unref(query);
		if (NULL == reply) {
			vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",slot_no);
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_WARNING;
		}

		if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			
			DBUS_TYPE_INVALID)) {
			    if( FDB_RETURN_CODE_GENERAL == op_ret){
	                vty_out(vty,"%% Error,operations error when configuration!\n");
			    }
				else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == op_ret){
					vty_out(vty,"%% Error,the vlan input not exist!\n");
				}
				else if(FDB_RETURN_CODE_NODE_PORT_NOTIN_VLAN == op_ret){
					vty_out(vty,"%% Error,the port is not in vlan!\n");
				}
				else if(FDB_RETURN_CODE_BADPARA == op_ret){
					vty_out(vty,"%% Error,the parameter input contains some error!\n");
				}
				else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
					vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
				}
				else if(FDB_RETURN_CODE_SYSTEM_MAC == op_ret){
					vty_out(vty,"%% Error,the mac input conflict with system mac,please chang!\n");

				}
				else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
					vty_out(vty,"%% Product not support this function!\n");
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
	 }
	 else
	 {
	 	
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_WITH_NAME);
		
		dbus_error_init(&err);
		dbus_message_append_args(	query,
									DBUS_TYPE_STRING,&vlanname,
								 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
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
			return CMD_WARNING;
		}

		if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			
			DBUS_TYPE_INVALID)) {
			    if( FDB_RETURN_CODE_GENERAL == op_ret){
	                vty_out(vty,"%% Error,operations error when configuration!\n");
			    }
				else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == op_ret){
					vty_out(vty,"%% Error,the vlan input not exist!\n");
				}
				else if(FDB_RETURN_CODE_NODE_PORT_NOTIN_VLAN == op_ret){
					vty_out(vty,"%% Error,the port is not in vlan!\n");
				}
				else if(FDB_RETURN_CODE_BADPARA == op_ret){
					vty_out(vty,"%% Error,the parameter input contains some error!\n");
				}
				else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
					vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
				}
				else if(FDB_RETURN_CODE_SYSTEM_MAC == op_ret){
					vty_out(vty,"%% Error,the mac input conflict with system mac,please chang!\n");

				}
				else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
					vty_out(vty,"%% Product not support this function!\n");
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
	 }
	return NPD_FDB_ERR_NONE;
}


/**************************************
*config fdb static fdb trunk table item on Both Sw & Hw. 
*
* Usage:create fdb static mac MAC vlan VLANNAME trunk <1-127>
*
*input Params: 
*	MAC: destination mac form 00:00:11:11:aa:aa
*	VLANNAME:  before doing, the vlan name which has been configed ,
*                           VLANNAME must be to begin with letter or '_'
*   	<1-127>:  destination trunk
*
*output params:none
*     
****************************************/

DEFUN(config_fdb_mac_vlan_trunk_static_name_cmd_func,
	config_fdb_mac_vlan_trunk_name_cmd,
	"create fdb static mac MAC vlan VLANNAME trunk <1-127>",
	"Config system information\n"
	"Config FDB table\n"
	"Static FDB item \n"
	"Config MAC field of FDB table.\n"
	"Config MAC address in Hex format\n"
	"Config FDB table vlanname\n"
	"VLANNAME must be to begin with letter or '_'\n"
	"Config trunk entry \n"
	"Trunk ID range <1-127>\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	char* vlanname = NULL;
	unsigned short	vlanId = 0;
	unsigned short  trunkId = 0;
	int 	op_ret = 0;
	unsigned int len = 0;
	int i;
	int local_slot_id = 0;
    int slotNum = 0;
	ETHERADDR		macAddr;
	memset(&macAddr,0,sizeof(ETHERADDR));	
	op_ret = parse_mac_addr((char *)argv[0],&macAddr);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad Parameters,Unknow mac addr format!\n");
		return CMD_WARNING;
	}
	
	op_ret=is_muti_brc_mac(&macAddr);
	if(op_ret==1){
		vty_out(vty,"%% Erro:input should not be broadcast or multicast mac!\n");
		return CMD_WARNING;
	}
		
	
	 op_ret =parse_vlan_string((char*)argv[1]);

   if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad Parameters,vlanname form erro!\n");
		return CMD_WARNING;
	}

    vlanname= (char *)argv[1];

    op_ret = parse_short_parse((char*)argv[2], &trunkId);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad Parameters,Unknow trunk id format.\n");
		return CMD_WARNING;
	}
    if ((trunkId<1)||(trunkId>127)){
		vty_out(vty,"%% Error,FDB vlan outrange!\n");
		return CMD_WARNING;
	}
	if(is_distributed == DISTRIBUTED_SYSTEM)	
	{
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		for(i=1; i <= slotNum; i++)
		{
			query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_TRUNK_WITH_NAME);
			
			dbus_error_init(&err);
			dbus_message_append_args(	query,
										DBUS_TYPE_STRING,&vlanname,
										DBUS_TYPE_UINT16,&trunkId,
									 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
									 	DBUS_TYPE_INVALID);
			
			if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
			{
				if(i == local_slot_id)
				{
               		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
			   		vty_out(vty,"Can not connect to slot:%d \n",i);	
					continue;
				}
        	}
			else
			{
            	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);				
			}
			dbus_message_unref(query);
			if (NULL == reply) {
				vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",i);
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_WARNING;
			}

			if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&op_ret,
				
				DBUS_TYPE_INVALID)) {
				   if( FDB_RETURN_CODE_GENERAL == op_ret){
		                vty_out(vty,"%% Error,operations error when configuration!\n");
				    }
					else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == op_ret){
						vty_out(vty,"%% Error,the vlan input not exist!\n");
					}
					else if(FDB_RETURN_CODE_NODE_PORT_NOTIN_VLAN == op_ret){
						vty_out(vty,"%% Error,the trunk is not in vlan!\n");
					}
					else if(FDB_RETURN_CODE_NODE_NOT_EXIST== op_ret){
						vty_out(vty,"%% Error,the trunk does not exist!\n");
					}
					else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
						vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
					}
					else if(FDB_RETURN_CODE_SYSTEM_MAC == op_ret){
						vty_out(vty,"%% Error,the mac input conflict with system mac,please chang!\n");
					}
					else if(FDB_RETURN_CODE_NODE_PORT_NOTIN_VLAN == op_ret){
						vty_out(vty,"%% Error,there is no port in trunk!\n");
					}
					else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
						vty_out(vty,"%% Product not support this function!\n");
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
		}
	}
	else
	{
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_TRUNK_WITH_NAME);
		
		dbus_error_init(&err);
		dbus_message_append_args(	query,
									DBUS_TYPE_STRING,&vlanname,
									DBUS_TYPE_UINT16,&trunkId,
								 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
								 	DBUS_TYPE_INVALID);
		
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
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
			DBUS_TYPE_UINT32,&op_ret,
			
			DBUS_TYPE_INVALID)) {
			   if( FDB_RETURN_CODE_GENERAL == op_ret){
	                vty_out(vty,"%% Error,operations error when configuration!\n");
			    }
				else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == op_ret){
					vty_out(vty,"%% Error,the vlan input not exist!\n");
				}
				else if(FDB_RETURN_CODE_NODE_PORT_NOTIN_VLAN == op_ret){
					vty_out(vty,"%% Error,the trunk is not in vlan!\n");
				}
				else if(FDB_RETURN_CODE_NODE_NOT_EXIST== op_ret){
					vty_out(vty,"%% Error,the trunk does not exist!\n");
				}
				else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
					vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
				}
				else if(FDB_RETURN_CODE_SYSTEM_MAC == op_ret){
					vty_out(vty,"%% Error,the mac input conflict with system mac,please chang!\n");
				}
				else if(FDB_RETURN_CODE_NODE_PORT_NOTIN_VLAN == op_ret){
					vty_out(vty,"%% Error,there is no port in trunk!\n");
				}
				else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
					vty_out(vty,"%% Product not support this function!\n");
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
	}
	return CMD_SUCCESS;
}



/**************************************
*config fdb static fdb table item on Both Sw & Hw. 
*
* Usage:create fdb static mac MAC vlan <1-4094> trunk <1-127> 
*
*input Params: 
*	MAC: destination mac form 00:00:11:11:aa:aa
*	<1-4094>: valid vlan index  range,which has been configed befor doing.
*   	<1-127>:  destination trunk number
*
*output params:none
*     
****************************************/

DEFUN(config_fdb_mac_vlan_trunk_static_cmd_func,
	config_fdb_mac_vlan_trunk_cmd,
	"create fdb static mac MAC vlan <1-4094> trunk <1-127>",
	"Config system information\n"
	"Config FDB table\n"
	"Static FDB item \n"
	"Config MAC field of FDB table.\n"
	"Config MAC address in Hex format\n"
	"Config FDB table vlan Id\n"
	"Config VLAN Id valid range <1-4094>\n"
	"Config trunk entry \n"
	"Trunk ID range <1-127>\n"
)
{
	DBusMessage *query, *reply;
	ETHERADDR		macAddr;
	unsigned short	vlanId = 0;
	unsigned short  trunkId = 0;
	DBusError err;
	unsigned int 	op_ret = 0;	
	int i;
	int local_slot_id = 0;
    int slotNum = 0;
	int ct_success = 0;
	
	memset(&macAddr,0,sizeof(ETHERADDR));
	op_ret = parse_mac_addr((char *)argv[0],&macAddr);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad Parameters,Unknow mac addr format!\n");
		return CMD_WARNING;
	}
   
	op_ret=is_muti_brc_mac(&macAddr);
	if(op_ret==1){
		vty_out(vty,"%% Error:input broadcast or multicast mac!\n");
		return CMD_WARNING;
		}
		
	
	op_ret = parse_short_parse((char*)argv[1], &vlanId);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad Parameters,Unknow vlan id format.\n");
		return CMD_WARNING;
	}
    if ((vlanId<MIN_VLANID)||(vlanId>MAX_VLANID)){
		vty_out(vty,"%% Error,FDB vlan outrange!\n");
		return CMD_WARNING;
		}
	/*vty_out(vty,"fdb vlan id %d.\n",vlanId);*/

	
	op_ret = parse_short_parse((char*)argv[2], &trunkId);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad Parameters,Unknow trunk id format.\n");
		return CMD_WARNING;
	}
    if ((trunkId<1)||(trunkId>127)){
		vty_out(vty,"%% Error,Trunk outrange!\n");
		return CMD_WARNING;
	}
	if(is_distributed == DISTRIBUTED_SYSTEM)	
	{
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		for(i=1; i <= slotNum; i++)
		{
			query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
												NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_TRUNK_STATIC);
			
			dbus_error_init(&err);
			dbus_message_append_args(	query,
										DBUS_TYPE_UINT16,&vlanId,
							            DBUS_TYPE_UINT16,&trunkId,
									 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
									 	DBUS_TYPE_INVALID);
			
			if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
			{
				if(i == local_slot_id)
				{
               		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
			   		vty_out(vty,"Can not connect to slot:%d \n",i);	
					continue;
				}
        	}
			else
			{
            	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);				
			}
			
			dbus_message_unref(query);
			if (NULL == reply) {
					vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",i);
					if (dbus_error_is_set(&err)) {
							vty_out(vty,"%s raised: %s",err.name,err.message);
							dbus_error_free_for_dcli(&err);
					}
					return CMD_WARNING;
			}

			if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)) {
					if(FDB_RETURN_CODE_NODE_EXIST == op_ret)
					{
						vty_out(vty,"%% This Entry already exists!\n");
					}
					else if( FDB_RETURN_CODE_GENERAL == op_ret){
		                vty_out(vty,"%% Error,operations error when configuration!\n");
				    }
					else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == op_ret){
						vty_out(vty,"%% Error,the vlan input not exist!\n");
					}
					else if(FDB_RETURN_CODE_NODE_PORT_NOTIN_VLAN == op_ret){
						vty_out(vty,"%% Error,the trunk is not in vlan!\n");
					}
					else if(FDB_RETURN_CODE_NODE_NOT_EXIST== op_ret){
						vty_out(vty,"%% Error,the trunk does not exist!\n");
					}
					else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
						vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
					}
					else if(FDB_RETURN_CODE_SYSTEM_MAC == op_ret){
						vty_out(vty,"%% Error,the mac input conflict with system mac,please chang!\n");
					}
					else if(FDB_RETURN_CODE_NODE_PORT_NOTIN_VLAN == op_ret){
						vty_out(vty,"%% Error,there is no port in trunk!\n");
					}
					else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
						vty_out(vty,"%% Product not support this function!\n");
					}
					else if(CMD_SUCCESS == op_ret)
					{
						ct_success = 1;
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
		}
		if(ct_success == 1)
        {
           	/* update g_vlanlist[] on Active-MCB & Standby-MCB. zhangdi 20110804*/
        	if(is_distributed == DISTRIBUTED_SYSTEM)	
        	{
    			vty_out(vty,"update g_static_fdb on SMU !\n");			
            	/* Wherever slot the CMD we input, then update the gvlanlist[] of 2 MCBs */
        		op_ret = dcli_update_gstaticfdb_for_master(vty,macAddr,vlanId,0,0,trunkId,0,1);
        	    if( CMD_SUCCESS != op_ret)
        		{
        			vty_out(vty,"update g_static_fdb on SMU error !\n");					
        		    return CMD_WARNING;
        	    }
        	}					
        }
	}
	else
	{
		
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
											NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_TRUNK_STATIC);
		
		dbus_error_init(&err);
		dbus_message_append_args(	query,
									DBUS_TYPE_UINT16,&vlanId,
						            DBUS_TYPE_UINT16,&trunkId,
								 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
								 	DBUS_TYPE_INVALID);
		
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
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
				DBUS_TYPE_UINT32,&op_ret,
				DBUS_TYPE_INVALID)) {
				if( FDB_RETURN_CODE_GENERAL == op_ret){
	                vty_out(vty,"%% Error,operations error when configuration!\n");
			    }
				else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == op_ret){
					vty_out(vty,"%% Error,the vlan input not exist!\n");
				}
				else if(FDB_RETURN_CODE_NODE_PORT_NOTIN_VLAN == op_ret){
					vty_out(vty,"%% Error,the trunk is not in vlan!\n");
				}
				else if(FDB_RETURN_CODE_NODE_NOT_EXIST== op_ret){
					vty_out(vty,"%% Error,the trunk does not exist!\n");
				}
				else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
					vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
				}
				else if(FDB_RETURN_CODE_SYSTEM_MAC == op_ret){
					vty_out(vty,"%% Error,the mac input conflict with system mac,please chang!\n");
				}
				else if(FDB_RETURN_CODE_NODE_PORT_NOTIN_VLAN == op_ret){
					vty_out(vty,"%% Error,there is no port in trunk!\n");
				}
				else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
					vty_out(vty,"%% Product not support this function!\n");
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
	}
	return CMD_SUCCESS;
}

/**************************************
*config specify fdb item deny to forward base on (dmac|smac) on Both Sw & Hw. 
*
* Usage:delete fdb  blacklist (dmac|smac) MAC vlan <1-4094> 
*
*input Params: 
*	(dmac|smac): two chose one param( destination mar or source mac) mac form 00:00:11:11:aa:aa
*	<1-4094>: valid vlan index  range, before doing the vlan index which has been configed 
*   
*
*output params:none
*     
****************************************/

DEFUN(config_fdb_mac_vlan_match_no_drop_cmd_func,
	config_fdb_mac_vlan_match_no_drop_cmd,
	"delete fdb  blacklist (dmac|smac) MAC vlan <1-4094>",
	"Delete FDB table\n\n"
	"FDB table\n"
	"Config FDB table make match forward command!\n"
	"Config FDB table dmac match out of blacklist \n"
	"Config FDB table smac match out of blacklist \n"
	"Config MAC address in Hex format\n"
	"Config FDB table vlan Id\n"
	"Config VLAN Id valid range <1-4094>\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	ETHERADDR		macAddr;
	unsigned short	vlanId = 0;
	int 	op_ret = 0,len = 0;
    unsigned int   flag = 0;
	int i;
	int local_slot_id = 0;
    int slotNum = 0;
	char*  string=  "dmac";
	memset(&macAddr,0,sizeof(ETHERADDR));

	len = strlen((char*)argv[0]);
	if((len>4)||(0 == len)){
		vty_out(vty,"% Bad Parameters,Error mac type.\n");
		return CMD_WARNING;
	}
	else{
		op_ret=strncmp(string,(char*)argv[0],len);
		if(op_ret==0){
			flag =1;
		}
		else if(op_ret<0){
			flag= 0;
		}
		else{
            vty_out(vty,"% Bad Parameters,Error mac type.\n");
		    return CMD_WARNING;
		}
	}
	op_ret = parse_mac_addr((char *)argv[1],&macAddr);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad Parameters,Unknow mac addr format.\n");
		return CMD_WARNING;
	}
	
	op_ret=is_muti_brc_mac(&macAddr);
	if(op_ret==1){
		vty_out(vty,"%% Error:input broadcast or multicast mac!\n");
		return CMD_WARNING;
	}
	op_ret = parse_short_parse((char*)argv[2], &vlanId);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"%% Error,Unknow vlan id format.\n");
		return CMD_WARNING;
	}
    if (vlanId<MIN_VLANID||vlanId>MAX_VLANID){
		vty_out(vty,"%% Error,vlan outrang!\n");
		return CMD_WARNING;
	}
	if(is_distributed == DISTRIBUTED_SYSTEM)	
	{
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		for(i=1; i <= slotNum; i++)
		{
			query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
								NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NO_DROP);
			
			dbus_error_init(&err);
			dbus_message_append_args(	query,
										DBUS_TYPE_UINT32,&flag,
									 	DBUS_TYPE_UINT16,&vlanId,
									 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
									 	DBUS_TYPE_INVALID);
			
				
			if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
			{
				if(i == local_slot_id)
				{
               		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
			   		vty_out(vty,"Can not connect to slot:%d \n",i);	
					continue;
				}
        	}
			else
			{
            	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);				
			}
			
			dbus_message_unref(query);
			if (NULL == reply) {
				vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",i);
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_WARNING;
			}
			

			if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&op_ret,
				DBUS_TYPE_INVALID)) {
				   if( FDB_RETURN_CODE_GENERAL == op_ret){
		                vty_out(vty,"%% Error,operations error when configuration!\n");
				    }
					else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == op_ret){
						vty_out(vty,"%% Error,the vlan input not exist!\n");
					}
					else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
						vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
					}
					else if(FDB_RETURN_CODE_SYSTEM_MAC == op_ret){
						vty_out(vty,"%% Error,the mac input conflict with system mac,please chang!\n");
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
		}
	}
	else
	{
		
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
							NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NO_DROP);
		
		dbus_error_init(&err);
		dbus_message_append_args(	query,
									DBUS_TYPE_UINT32,&flag,
								 	DBUS_TYPE_UINT16,&vlanId,
								 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
								 	DBUS_TYPE_INVALID);
		
			
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
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
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID)) {
			   if( FDB_RETURN_CODE_GENERAL == op_ret){
	                vty_out(vty,"%% Error,operations error when configuration!\n");
			    }
				else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == op_ret){
					vty_out(vty,"%% Error,the vlan input not exist!\n");
				}
				else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
					vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
				}
				else if(FDB_RETURN_CODE_SYSTEM_MAC == op_ret){
					vty_out(vty,"%% Error,the mac input conflict with system mac,please chang!\n");
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
	}
	return CMD_SUCCESS;
}

/**************************************
*config specify fdb item desterilize to forward base on (dmac|smac) on Both Sw & Hw. 
*
* Usage:delete fdb  blacklist (dmac|smac) MAC vlan <1-4094> 
*
*input Params: 
*	(dmac|smac): two chose one param( destination mar or source mac) mac form 00:00:11:11:aa:aa
*	VLANNAME:  before doing, the vlan name which has been configed ,
*                           VLANNAME must be to begin with letter or '_' 
*
*output params:none
*     
****************************************/

DEFUN(config_fdb_mac_vlan_match_no_drop_name_cmd_func,
	config_fdb_mac_vlan_match_no_drop_name_cmd,
	"delete fdb  blacklist (dmac|smac) MAC vlan VLANNAME",
	"Delete FDB table\n\n"
	"FDB table\n"
	"Config FDB table make match forward command!\n"
	"Config FDB table dmac match out of blacklist \n"
	"Config FDB table smac match out of blacklist \n"
	"Config MAC address in Hex format\n"
	"Config FDB table vlanname\n"
	"Vlanname must be to begin with letter or '_'\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	char *vlanname = NULL;
	ETHERADDR		macAddr;
	
	int 	op_ret = 0;
	unsigned int    flag = 0;
	unsigned char   len = 0;
	char* string="dmac";
	int i;
	int local_slot_id = 0;
    int slotNum = 0;
	memset(&macAddr,0,sizeof(ETHERADDR));
	
	len = strlen((char*)argv[0]);
	if((len>4)||(0 == len)){
		vty_out(vty,"% Bad Parameters,Error mac type.\n");
		return CMD_WARNING;
	}
	else{
		op_ret=strncmp(string,(char*)argv[0],len);
		if(op_ret==0){
			flag =1;
		}
		else if(op_ret<0){
			flag= 0;
		}
		else{
            vty_out(vty,"% Bad Parameters,Error mac type.\n");
		    return CMD_WARNING;
		}
	}

	op_ret = parse_mac_addr((char *)argv[1],&macAddr);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad Parameters,Unknow mac addr format.\n");
		return CMD_SUCCESS;
	}

	op_ret=is_muti_brc_mac(&macAddr);
	if(op_ret==1){
		vty_out(vty,"%% Erro:input should not be broadcast or multicast mac!\n");
		return CMD_SUCCESS;
	}

   op_ret =parse_vlan_string((char*)argv[2]);
   if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad Parameters,vlanname form erro!\n");
		return CMD_SUCCESS;
	}

    vlanname= (char *)argv[2];
	if(is_distributed == DISTRIBUTED_SYSTEM)	
	{
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		for(i=1; i <= slotNum; i++)
		{
    
			query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
								NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NO_DROP_WITH_NAME);
			
			dbus_error_init(&err);
			dbus_error_init(&err);
			dbus_message_append_args(	query,
										DBUS_TYPE_UINT32,&flag,
									 	DBUS_TYPE_STRING,&vlanname,
									 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
									 	DBUS_TYPE_INVALID);
			
				
			if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
			{
				if(i == local_slot_id)
				{
               		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
			   		vty_out(vty,"Can not connect to slot:%d \n",i);	
					continue;
				}
        	}
			else
			{
            	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);				
			}
			
			dbus_message_unref(query);
			if (NULL == reply) {
				vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",i);
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_SUCCESS;
			}
			

			if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&op_ret,
				DBUS_TYPE_INVALID)) {
					if( FDB_RETURN_CODE_GENERAL  == op_ret){
		                vty_out(vty,"%% Error,operations error when configuration!\n");
				    }
					else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == op_ret){
						vty_out(vty,"%% Error,the vlan input not exist!\n");
					}
		            else if(FDB_RETURN_CODE_BADPARA == op_ret){
						vty_out(vty,"%% Error,the parameter input is error!\n");
		            }
					else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
						vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
					}
					else if(FDB_RETURN_CODE_SYSTEM_MAC == op_ret){
						vty_out(vty,"%% Error,the mac input conflict with system mac,please chang!\n");
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
		}
	}
	else
	{
		
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
							NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NO_DROP_WITH_NAME);
		
		dbus_error_init(&err);
		dbus_error_init(&err);
		dbus_message_append_args(	query,
									DBUS_TYPE_UINT32,&flag,
								 	DBUS_TYPE_STRING,&vlanname,
								 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
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
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID)) {
				if( FDB_RETURN_CODE_GENERAL  == op_ret){
	                vty_out(vty,"%% Error,operations error when configuration!\n");
			    }
				else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == op_ret){
					vty_out(vty,"%% Error,the vlan input not exist!\n");
				}
	            else if(FDB_RETURN_CODE_BADPARA == op_ret){
					vty_out(vty,"%% Error,the parameter input is error!\n");
	            }
				else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
					vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
				}
				else if(FDB_RETURN_CODE_SYSTEM_MAC == op_ret){
					vty_out(vty,"%% Error,the mac input conflict with system mac,please chang!\n");
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
	}
	return CMD_SUCCESS;
}


/**************************************
*config specify fdb item deny to forward base on (dmac|smac) on Both Sw & Hw. 
*
* Usage:create fdb blacklist (dmac|smac) MAC vlan VLANNAME" 
*
*input Params: 
*	(dmac|smac): two chose one param( destination mar or source mac) mac form 00:00:11:11:aa:aa
*	VLANNAME:  before done the vlan name which has been configed ,
*                           VLANNAME must be to begin with letter or '_'
*  
*
*output params:none
*     
****************************************/

DEFUN(config_fdb_mac_vlanname_match_drop_name_cmd_func,
	config_fdb_mac_vlan_name_match_cmd,
	"create fdb blacklist (dmac|smac) MAC vlan VLANNAME",
	"Config system information\n"
	"Config FDB table\n"
	"Config FDB table make match drop command!\n"
	"Config FDB table dmac match into blacklist \n"
	"Config FDB table smac match into blacklist \n"
	"Config MAC address in Hex format\n"
	"Config FDB table vlanname\n"
	"Config VLANNAME must be to begin with letter or '_'\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret = 0,op_ret = 0;
	unsigned char   len = 0;
	unsigned int    flag = 0;
	char* vlanname= NULL;
	ETHERADDR		macAddr;
	char* string="dmac";
	int i;
	int local_slot_id = 0;
    int slotNum = 0;
	
	memset(&macAddr,0,sizeof(ETHERADDR));

	len = strlen((char*)argv[0]);
	if((len>4)||(0 == len)){
		vty_out(vty,"% Bad Parameters,Error mac type.\n");
		return CMD_WARNING;
	}
	else{
		op_ret=strncmp(string,(char*)argv[0],len);
		if(op_ret==0){
			flag =1;
		}
		else if(op_ret<0){
			flag= 0;
		}
		else{
            vty_out(vty,"% Bad Parameters,Error mac type.\n");
		    return CMD_WARNING;
		}
	}
	
	ret = parse_mac_addr((char *)argv[1],&macAddr);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"%Bad Parameters,Unknow mac addr format!\n");
		return CMD_SUCCESS;
	}

	ret=is_muti_brc_mac(&macAddr);
	if(ret==1){
		vty_out(vty,"%% Error:input broadcast or multicast mac!\n");
		return CMD_SUCCESS;
		}
		

   ret =parse_vlan_string((char*)argv[2]);

   if (NPD_FAIL == ret) {
    	vty_out(vty,"% Bad Parameters,vlanname form erro!\n");
		return CMD_SUCCESS;
	}

   vlanname= (char *)argv[2];
	
 	if(is_distributed == DISTRIBUTED_SYSTEM)	
	{
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		for(i=1; i <= slotNum; i++)
		{
			query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_DROP_WITH_NAME);
			
			dbus_error_init(&err);
			
			dbus_message_append_args(	query,
										DBUS_TYPE_UINT32,&flag,
									 	DBUS_TYPE_STRING,&vlanname,
									 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
									 	DBUS_TYPE_INVALID);
			
				
			if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
			{
				if(i == local_slot_id)
				{
               		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
			   		vty_out(vty,"Can not connect to slot:%d \n",i);	
					continue;
				}
        	}
			else
			{
            	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);				
			}
			
			dbus_message_unref(query);
			if (NULL == reply) {
					vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",i);
					if (dbus_error_is_set(&err)) {
						vty_out(vty,"%s raised: %s",err.name,err.message);
						dbus_error_free_for_dcli(&err);
					}
			}
			
			else{
					if (!(dbus_message_get_args ( reply, &err,
													DBUS_TYPE_UINT32,&ret,
													DBUS_TYPE_INVALID))) {
						
							vty_out(vty,"Failed get args.\n");
							if (dbus_error_is_set(&err)) {
									vty_out(vty,"%s raised: %s",err.name,err.message);
									dbus_error_free_for_dcli(&err);
							}
					}
					else{
						if( FDB_RETURN_CODE_GENERAL == ret){
		                vty_out(vty,"%% Error,operations error when configuration!\n");
					    }
						else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == ret){
							vty_out(vty,"%% Error,the vlan input not exist!\n");
						}
						else if(FDB_RETURN_CODE_OCCUR_HW == ret){
							vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
						}
						else if(FDB_RETURN_CODE_SYSTEM_MAC == ret){
							vty_out(vty,"%% Error,the mac input conflict with system mac,please chang!\n");
						}
						else if(COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
							vty_out(vty,"%% Product not support this function!\n");
						}
					}
			}
			dbus_message_unref(reply);
		}
 	}
	else
	{
		
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_DROP_WITH_NAME);
		
		dbus_error_init(&err);
		
		dbus_message_append_args(	query,
									DBUS_TYPE_UINT32,&flag,
								 	DBUS_TYPE_STRING,&vlanname,
								 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
								 	DBUS_TYPE_INVALID);
		
			
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		if (NULL == reply) {
				vty_out(vty,"failed get reply.\n");
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
		}
		
		else{
				if (!(dbus_message_get_args ( reply, &err,
												DBUS_TYPE_UINT32,&ret,
												DBUS_TYPE_INVALID))) {
					
						vty_out(vty,"Failed get args.\n");
						if (dbus_error_is_set(&err)) {
								vty_out(vty,"%s raised: %s",err.name,err.message);
								dbus_error_free_for_dcli(&err);
						}
				}
				else{
					if( FDB_RETURN_CODE_GENERAL == ret){
	                vty_out(vty,"%% Error,operations error when configuration!\n");
				    }
					else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == ret){
						vty_out(vty,"%% Error,the vlan input not exist!\n");
					}
					else if(FDB_RETURN_CODE_OCCUR_HW == ret){
						vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
					}
					else if(FDB_RETURN_CODE_SYSTEM_MAC == ret){
						vty_out(vty,"%% Error,the mac input conflict with system mac,please chang!\n");
					}
					else if(COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
						vty_out(vty,"%% Product not support this function!\n");
					}
				}
		}
		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;
}

/**************************************
*config specify fdb item deny to forward base on (dmac|smac) on Both Sw & Hw. 
*
* Usage:create fdb blacklist (dmac|smac) MAC vlan <1-4094> 
*
*input Params: 
*	(dmac|smac): two chose one param( destination mar or source mac) mac form 00:00:11:11:aa:aa
*	<1-4094>: valid vlan index  range, before done the vlan index which has been configed 
*   
*
*output params:none
*     
****************************************/

DEFUN(config_fdb_mac_vlan_match_drop_cmd_func,
	config_fdb_mac_vlan_match_cmd,
	"create fdb blacklist (dmac|smac) MAC vlan <1-4094>",
	"Config system information\n"
    "Config FDB table\n"
	"Config FDB table make match  drop command!\n"
	"Config FDB table match dMAC into blacklist \n"
	"Config FDB table match sMAC into blacklist \n"
	"Config MAC address in Hex format\n"
	"Config FDB table vlan Id\n"
	"Config VLAN Id valid range <1-4094>\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	ETHERADDR		macAddr;
	unsigned short	vlanId = 0;
	int 	op_ret = 0;
	unsigned char   len = 0;
	unsigned int    flag = 0;
	char*  string ="dmac";
	int i;
	int local_slot_id = 0;
    int slotNum = 0;
	
	memset(&macAddr,0,sizeof(ETHERADDR));
	len = strlen((char*)argv[0]);
	if((len>4)||(0 == len)){
		vty_out(vty,"% Bad Parameters,Error mac type,strlen = %d.\n",len);
		return CMD_WARNING;
	}
	else{
		op_ret=strncmp(string,(char*)argv[0],len);
		if(op_ret==0){
			flag =1;
		}
		else if(op_ret<0){
			flag= 0;
		}
		else{
            vty_out(vty,"% Bad Parameters,Error mac type,return value is %d\n",op_ret);
		    return CMD_WARNING;
		}
	}

	op_ret = parse_mac_addr((char *)argv[1],&macAddr);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad Parameters,Unknow mac addr format!\n");
		return CMD_SUCCESS;
	}
	
	op_ret=is_muti_brc_mac(&macAddr);
	if(op_ret==1){
		vty_out(vty,"%% Error:input should not be broadcast or multicast mac!\n");
		return CMD_SUCCESS;
		}
		
	
	op_ret = parse_short_parse((char*)argv[2], &vlanId);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"%% Error,Unknow vlan id format!\n");
		return CMD_SUCCESS;
	}
    if (vlanId<MIN_VLANID || vlanId>MAX_VLANID){
	vty_out(vty,"%% Error, vlan outrang!\n");
		return CMD_SUCCESS;
	}
	if(is_distributed == DISTRIBUTED_SYSTEM)	
	{
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		for(i=1; i <= slotNum; i++)
		{
			query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_DROP);
			
			
			dbus_error_init(&err);
			dbus_message_append_args(	query,
										DBUS_TYPE_UINT32,&flag,
									 	DBUS_TYPE_UINT16,&vlanId,
									 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
									 	DBUS_TYPE_INVALID);
			
				
			if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
			{
				if(i == local_slot_id)
				{
               		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
			   		vty_out(vty,"Can not connect to slot:%d \n",i);	
					continue;
				}
        	}
			else
			{
            	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);				
			}
			
			dbus_message_unref(query);
			if (NULL == reply) {
				vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",i);
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_SUCCESS;
			}
			

			if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&op_ret,
				DBUS_TYPE_INVALID)) {
					if (FDB_RETURN_CODE_NODE_EXIST == op_ret){
						vty_out(vty,"%%this item already exists !\n");
					}
				    else if( FDB_RETURN_CODE_GENERAL == op_ret){
		                vty_out(vty,"%% Error,operations error when configuration!\n");
				    }
					else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == op_ret){
						vty_out(vty,"%% Error,the vlan input not exist!\n");
					}
					else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
						vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
					}
					else if(FDB_RETURN_CODE_SYSTEM_MAC == op_ret){
						vty_out(vty,"%% Error,the mac input conflict with system mac,please chang!\n");
					}
					else if(COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
						vty_out(vty,"%% Product not support this function!\n");
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
			
		}
	}
	else
	{
		
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_DROP);
		
		
		dbus_error_init(&err);
		dbus_message_append_args(	query,
									DBUS_TYPE_UINT32,&flag,
								 	DBUS_TYPE_UINT16,&vlanId,
								 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
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
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID)) {
			    if( FDB_RETURN_CODE_GENERAL == op_ret){
	                vty_out(vty,"%% Error,operations error when configuration!\n");
			    }
				else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == op_ret){
					vty_out(vty,"%% Error,the vlan input not exist!\n");
				}
				else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
					vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
				}
				else if(FDB_RETURN_CODE_SYSTEM_MAC == op_ret){
					vty_out(vty,"%% Error,the mac input conflict with system mac,please chang!\n");
				}
				else if(COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
					vty_out(vty,"%% Product not support this function!\n");
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
	}
	return CMD_SUCCESS;
}

/**************************************
*delete specify static fdb item on Both Sw & Hw. 
*
* Usage:delete fdb static mac MAC vlan VLANNAME 
*
*input Params: 
*	(dmac|smac): two chose one param( destination mar or source mac) mac form 00:00:11:11:aa:aa
*	VLANNAME:  before done the vlan name which has been configed ,
*                           VLANNAME must be to begin with letter or '_'
*   
*
*output params:none
*     
****************************************/

DEFUN(config_fdb_mac_vlan_no_static_port_name_cmd_func,
	config_fdb_mac_vlan_no_static_port_name_cmd,
	"delete fdb static mac MAC vlan VLANNAME ",
	"Delete FDB table\n"
	"Config FDB table\n"
	"Static FDB item\n"
	"Config MAC field of  FDB table.\n"
	"Config MAC address in Hex format\n"
	"Config FDB table vlanname\n"
	"Config VLANNAME must be to begin with letter or '_'\n"
)
{
	DBusMessage *query, *reply;
	ETHERADDR		macAddr;
	char*	vlanname=NULL; 
	DBusError err;
	unsigned int 	ret = 0;
	int i;
	int local_slot_id = 0;
    int slotNum = 0;
	memset(&macAddr,0,sizeof(ETHERADDR));
	
	ret = parse_mac_addr((char *)argv[0],&macAddr);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"% Bad Parameters,Unknow mac addr format!\n");
		return CMD_SUCCESS;
	}
	
	ret=is_muti_brc_mac(&macAddr);
	if(ret==1){
		vty_out(vty,"%% Error:input broadcast or multicast mac!\n");
		return CMD_SUCCESS;
		}
		
	
	ret =parse_vlan_string((char*)argv[1]);
	
	   if (NPD_FAIL == ret) {
			vty_out(vty,"%% Error,vlanname form erro!\n");
			return CMD_SUCCESS;
		}
	
	   vlanname= (char *)argv[1];

	if(is_distributed == DISTRIBUTED_SYSTEM)	
	{
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		for(i=1; i <= slotNum; i++)
		{
			query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NO_STATIC_WITH_NAME);
			
			dbus_error_init(&err);

			dbus_message_append_args(	query,
										/*DBUS_TYPE_STRING,&vlanname,*/
									 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
									 	DBUS_TYPE_STRING,&vlanname,
									 	DBUS_TYPE_INVALID);
			
			if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
			{
				if(i == local_slot_id)
				{
               		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
			   		vty_out(vty,"Can not connect to slot:%d \n",i);	
					continue;
				}
        	}
			else
			{
            	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);				
			}
			dbus_message_unref(query);
			if (NULL == reply) {
				vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",i);
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
			}
			else{
					if (dbus_message_get_args ( reply, &err,
						DBUS_TYPE_UINT32,&ret,
						
						DBUS_TYPE_INVALID)) {
						if( FDB_RETURN_CODE_GENERAL == ret){
		                vty_out(vty,"%% Error,operations error when configuration!\n");
					    }
						else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == ret){
							vty_out(vty,"%% Error,the vlan input not exist!\n");
						}
						else if(FDB_RETURN_CODE_NODE_NOT_EXIST== ret){
							vty_out(vty,"%% Error,the node not exist!\n");
						}
						else if(FDB_RETURN_CODE_ITEM_ISMIRROR == ret){
							vty_out(vty,"%% Error,the item want to delete is mirroring and can not delete directly!\n");
						}
						else if(FDB_RETURN_CODE_OCCUR_HW == ret){
							vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
						}
						else if(FDB_RETURN_CODE_SYSTEM_MAC == ret){
							vty_out(vty,"%% Error,the mac input conflict with system mac,please chang!\n");
						}
					} 
					else {
						vty_out(vty,"Failed get args.\n");
						if (dbus_error_is_set(&err)) {
							vty_out(vty,"%s raised: %s",err.name,err.message);
							dbus_error_free_for_dcli(&err);
						}
					}
			}
			dbus_message_unref(reply);
		}
	}
	else
	{
		
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NO_STATIC_WITH_NAME);
		
		dbus_error_init(&err);

		dbus_message_append_args(	query,
									/*DBUS_TYPE_STRING,&vlanname,*/
								 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
								 	DBUS_TYPE_STRING,&vlanname,
								 	DBUS_TYPE_INVALID);
		
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		if (NULL == reply) {
			vty_out(vty,"failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
		}
		else{
				if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					
					DBUS_TYPE_INVALID)) {
					if( FDB_RETURN_CODE_GENERAL == ret){
	                vty_out(vty,"%% Error,operations error when configuration!\n");
				    }
					else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == ret){
						vty_out(vty,"%% Error,the vlan input not exist!\n");
					}
					else if(FDB_RETURN_CODE_NODE_NOT_EXIST== ret){
						vty_out(vty,"%% Error,the node not exist!\n");
					}
					else if(FDB_RETURN_CODE_ITEM_ISMIRROR == ret){
						vty_out(vty,"%% Error,the item want to delete is mirroring and can not delete directly!\n");
					}
					else if(FDB_RETURN_CODE_OCCUR_HW == ret){
						vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
					}
					else if(FDB_RETURN_CODE_SYSTEM_MAC == ret){
						vty_out(vty,"%% Error,the mac input conflict with system mac,please chang!\n");
					}
				} 
				else {
					vty_out(vty,"Failed get args.\n");
					if (dbus_error_is_set(&err)) {
						vty_out(vty,"%s raised: %s",err.name,err.message);
						dbus_error_free_for_dcli(&err);
					}
				}
		}
		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;
}

/**************************************
*delete specify static fdb item on Both Sw & Hw. 
*
* Usage:delete fdb static mac MAC vlan <1-4094> 
*
*input Params: 
*	(dmac|smac): two chose one param( destination mar or source mac) mac form 00:00:11:11:aa:aa
*	<1-4094>:  valid vlan index  range, before done the vlan index which has been configed
*   
*
*output params:none
*     
****************************************/

DEFUN(config_fdb_mac_vlan_no_static_port_cmd_func,
	config_fdb_mac_vlan_no_static_port_cmd,
	"delete fdb static mac MAC vlan <1-4094> ",
	"Delete FDB table\n"
	"Config FDB table\n"
	"Static FDB item\n"
	"Config MAC field of FDB table.\n"
	"Config MAC address in Hex format\n"
	"Config FDB table vlan Id\n"
	"Config VLAN Id valid range <1-4094>\n"
)
{
	DBusMessage *query, *reply;
	ETHERADDR		macAddr;
	unsigned short	vlanId = 0;
	int local_slot_id = 0;
	DBusError err;
	unsigned int 	op_ret = 0;
	unsigned char slot_no=0,port_no=0,flag=0;	/*judge static does exists*/

	memset(&macAddr,0,sizeof(ETHERADDR));
	
	op_ret = parse_mac_addr((char *)argv[0],&macAddr);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad Parameters,Unknow mac addr format!\n");
		return CMD_SUCCESS;

	}
	op_ret=is_muti_brc_mac(&macAddr);
	if(op_ret==1){
		vty_out(vty,"%% Error:input broadcast or multicast mac!\n");
		return CMD_SUCCESS;
	}
	op_ret = parse_short_parse((char*)argv[1], &vlanId);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad Parameters,Unknow vlan id format!\n");
		return CMD_SUCCESS;
	}
    if (vlanId <MIN_VLANID||vlanId>MAX_VLANID){
		vty_out(vty,"%% Error,vlan outrang!\n");
		return CMD_SUCCESS;
	}
	if(is_distributed == DISTRIBUTED_SYSTEM)	
	{
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
		if(local_slot_id < 0)
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		op_ret = dcli_check_static_fdb_exists(vty,macAddr,vlanId,&slot_no,&port_no,&flag);
		if( CMD_SUCCESS != op_ret)
		{
			vty_out(vty,"static fdb not exists !\n");					
			return CMD_WARNING;
	    }
		else if(flag == 1)
		{
			query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NO_STATIC);
			
			dbus_error_init(&err);

			dbus_message_append_args(	query,
										DBUS_TYPE_UINT16,&vlanId,
									 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
									 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
									 	DBUS_TYPE_INVALID);
			
			if(local_slot_id == slot_no)
			{
				reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
			}
			else
			{
				if(NULL == dbus_connection_dcli[slot_no]->dcli_dbus_connection)
				{
				   	vty_out(vty,"Can not connect to slot:%d, check the slot please !\n",slot_no);	
	        		return CMD_WARNING;
				}
				else
				{
					reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_no]->dcli_dbus_connection,query,-1, &err);
				}
			}
			
			dbus_message_unref(query);
			if (NULL == reply) {
				vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",slot_no);
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
			}
			else{
					if (dbus_message_get_args ( reply, &err,
						DBUS_TYPE_UINT32,&op_ret,
						DBUS_TYPE_INVALID)) {
						if( FDB_RETURN_CODE_GENERAL == op_ret){
		                    vty_out(vty,"%% Error,operations error when configuration!\n");
					    }
		                else if(FDB_RETURN_CODE_NODE_NOT_EXIST== op_ret){
							vty_out(vty,"%% Error,the node not exist!\n");
						}
						else if(FDB_RETURN_CODE_ITEM_ISMIRROR == op_ret){
							vty_out(vty,"%% Error,the item want to delete is mirroring and can not delete directly!\n");
						}
						else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
							vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
						}
						else if(FDB_RETURN_CODE_SYSTEM_MAC == op_ret){
							vty_out(vty,"%% Error,the mac input conflict with system mac,please chang!\n");
						}
					} 
					else {
						vty_out(vty,"Failed get args.\n");
						if (dbus_error_is_set(&err)) {
							vty_out(vty,"%s raised: %s",err.name,err.message);
							dbus_error_free_for_dcli(&err);
						}
					}
			}
			dbus_message_unref(reply);

			if(CMD_SUCCESS == op_ret)
	        {
	           	/* update g_vlanlist[] on Active-MCB & Standby-MCB. zhangdi 20110804*/
	        	if(is_distributed == DISTRIBUTED_SYSTEM)	
	        	{
	    			vty_out(vty,"update g_static_fdb on SMU !\n");			
	            	/* Wherever slot the CMD we input, then update the gvlanlist[] of 2 MCBs */
	        		op_ret = dcli_update_gstaticfdb_for_master(vty,macAddr,vlanId,slot_no,port_no,0,0,4);
	        	    if( CMD_SUCCESS != op_ret)
	        		{
	        			vty_out(vty,"update g_static_fdb on SMU error !\n");					
	        		    return CMD_WARNING;
	        	    }
	        	}					
	        }
		}
		
	}
	else
	{
		
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NO_STATIC);
		
		dbus_error_init(&err);

		dbus_message_append_args(	query,
									DBUS_TYPE_UINT16,&vlanId,
								 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
								 	DBUS_TYPE_INVALID);
		
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		if (NULL == reply) {
			vty_out(vty,"failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
		}
		else{
				if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&op_ret,
					DBUS_TYPE_INVALID)) {
					if( FDB_RETURN_CODE_GENERAL == op_ret){
	                    vty_out(vty,"%% Error,operations error when configuration!\n");
				    }
	                else if(FDB_RETURN_CODE_NODE_NOT_EXIST== op_ret){
						vty_out(vty,"%% Error,the node not exist!\n");
					}
					else if(FDB_RETURN_CODE_ITEM_ISMIRROR == op_ret){
						vty_out(vty,"%% Error,the item want to delete is mirroring and can not delete directly!\n");
					}
					else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
						vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
					}
					else if(FDB_RETURN_CODE_SYSTEM_MAC == op_ret){
						vty_out(vty,"%% Error,the mac input conflict with system mac,please chang!\n");
					}
				} 
				else {
					vty_out(vty,"Failed get args.\n");
					if (dbus_error_is_set(&err)) {
						vty_out(vty,"%s raised: %s",err.name,err.message);
						dbus_error_free_for_dcli(&err);
					}
				}
		}
		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;
}

/**************************************
*delete fdb item which belong to the vlan on Both Sw & Hw. 
*
* Usage:delete fdb vlan <1-4094> 
*
*input Params: 
*	
*	<1-4094>:  valid vlan index  range, before done the vlan index which has been configed
*   
*
*output params:none
*     
****************************************/

DEFUN(config_fdb_delete_vlan_cmd_func,
	config_fdb_delete_vlan_cmd,
	"delete fdb vlan <1-4094>",
	"Delete FDB table\n"
	"Config FDB table \n"
	"FDB table vlan Range <1-4094s>\n"
	"Config Fdb vlan value\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short vlanid = 0;
	unsigned int 	op_ret = 0;
	int i;
	int local_slot_id = 0;
    int slotNum = 0;

	op_ret = parse_short_parse((char *)argv[0],&vlanid);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad Parameters,FDB agingtime form erro!\n");
		return CMD_SUCCESS;
	}
	 if (vlanid <MIN_VLANID||vlanid>MAX_VLANID){
		vty_out(vty,"%% Error, vlan outrang!\n");
		return CMD_SUCCESS;
		}
	if(is_distributed == DISTRIBUTED_SYSTEM)	
	{
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		for(i=1; i <= slotNum; i++)
		{
			query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_DELETE_WITH_VLAN );
			
			dbus_error_init(&err);

			dbus_message_append_args(	query,
									 	DBUS_TYPE_UINT16,&vlanid,
									 	DBUS_TYPE_INVALID);
			
			if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
			{
				if(i == local_slot_id)
				{
               		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
			   		vty_out(vty,"Can not connect to slot:%d \n",i);	
					continue;
				}
        	}
			else
			{
            	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);				
			}
			dbus_message_unref(query);
			if (NULL == reply) {
				vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",i);
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_SUCCESS;
			}

			if (!(dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&op_ret,
				DBUS_TYPE_INVALID))) {
				vty_out(vty,"Failed get args.\n");
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}		
			} 
			else {
				   if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == op_ret){
						vty_out(vty,"%% Error,the vlan input not exist!\n");
					}
					else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
						vty_out(vty,"%% Error,there is something wrong when deleting.\n");
					}
				}
			dbus_message_unref(reply);
		}
	}
	else
	{
		
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_DELETE_WITH_VLAN );
		
		dbus_error_init(&err);
		
		dbus_message_append_args(	query,
									DBUS_TYPE_UINT16,&vlanid,
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
		
		if (!(dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID))) {
			vty_out(vty,"Failed get args.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}		
		} 
		else {
			   if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == op_ret){
					vty_out(vty,"%% Error,the vlan input not exist!\n");
				}
				else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
					vty_out(vty,"%% Error,there is something wrong when deleting.\n");
				}
			}
		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;
}

/**************************************
*delete fdb item which belong to the port on Both Sw & Hw. 
*
* Usage:delete fdb port PORTNO 
*
*input Params: 
*	
*		PORTNO:  destination port, form slot/port
*   
*
*output params:none
*     
****************************************/

DEFUN(config_fdb_delete_port_cmd_func,
	config_fdb_delete_port_cmd,
	"delete fdb port PORTNO",
	"Delete FDB table\n"
	"Config FDB table \n"
	"FDB table port\n"
	CONFIG_ETHPORT_STR
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned char slotNum = 0,portNum = 0;
	unsigned int 	op_ret = 0;
	int local_slot_id = 0;
    int slotnum = 0;
	op_ret =  parse_slotport_no((char*)argv[0],&slotNum,&portNum);
	if (NPD_FAIL == op_ret) {
		vty_out(vty,"% Bad Parameters,Unknow portno format!\n");
		return CMD_SUCCESS;
	}
	/*
	if((slotNum <MIN_SLOT ||slotNum>MAX_SLOT)||(portNum <MIN_PORT||portNum>MAX_PORT)){
		vty_out(vty,"% Err port outrange!\n");
		return CMD_SUCCESS;
	}
	*/
	if(is_distributed == DISTRIBUTED_SYSTEM)
	{
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotnum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotnum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_DELETE_WITH_PORT);
		
		dbus_error_init(&err);

		dbus_message_append_args(	query,
								 	DBUS_TYPE_BYTE,&slotNum,
								 	DBUS_TYPE_BYTE,&portNum,
								 	DBUS_TYPE_INVALID);
		
		if(local_slot_id == slotNum)
		{
			reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		}
		else
		{
			if(NULL == dbus_connection_dcli[slotNum]->dcli_dbus_connection)
			{
			   	vty_out(vty,"Can not connect to slot:%d, check the slot please !\n",slotNum);	
        		return CMD_WARNING;
			}
			else
			{
				reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slotNum]->dcli_dbus_connection,query,-1, &err);
			}
		}
		
		dbus_message_unref(query);
		if (NULL == reply) {
			vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",slotNum);
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}

		if (!(dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID))) {
			vty_out(vty,"Failed get args.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
				
				
		} 
		else {
	           if(FDB_RETURN_CODE_BADPARA == op_ret){
					vty_out(vty,"%% Error,the parameter input is error!\n");
	            }
				else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
					vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
				}
			}
		dbus_message_unref(reply);
	}
	else
	{
		
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_DELETE_WITH_PORT);
		
		dbus_error_init(&err);

		dbus_message_append_args(	query,
								 	DBUS_TYPE_BYTE,&slotNum,
								 	DBUS_TYPE_BYTE,&portNum,
								 	DBUS_TYPE_INVALID);
		
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		if (NULL == reply) {
			vty_out(vty,"% Bad Parameters,failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}

		if (!(dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID))) {
			vty_out(vty,"Failed get args.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
				
				
		} 
		else {
	           if(FDB_RETURN_CODE_BADPARA == op_ret){
					vty_out(vty,"%% Error,the parameter input is error!\n");
	            }
				else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
					vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
				}
			}
		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;
}


/**************************************
*delete fdb item which belong to the vlan on Both Sw & Hw. 
*
* Usage:delete fdb static vlan <1-4094> 
*
*input Params: 
*	
*	<1-4094>:  valid vlan index  range, before done the vlan index which has been configed
*   
*
*output params:none
*     
****************************************/

DEFUN(config_fdb_static_delete_vlan_cmd_func,
	config_fdb_static_delete_vlan_cmd,
	"delete fdb static vlan <1-4094>",
	"Delete FDB table\n"
	"Config FDB table \n"
	"Static FDB table\n"
	"FDB table vlan Range <1-4094>\n"
	"Config FDB vlan value\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short vlanid = 0;
	unsigned int 	op_ret = 0;
	
	int i;
	int local_slot_id = 0;
	int slotNum = 0;
	int ct_success = 0;
	ETHERADDR		macAddr;
	memset(&macAddr,0,sizeof(ETHERADDR));
	op_ret = parse_short_parse((char *)argv[0],&vlanid);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad Parameters,FDB agingtime form erro!\n");
		return CMD_SUCCESS;
	}
	 if (vlanid <MIN_VLANID||vlanid>MAX_VLANID){
		vty_out(vty,"% Bad Parameters,FDB vlan outrang!\n");
		return CMD_SUCCESS;
		}
	if(is_distributed == DISTRIBUTED_SYSTEM)	
	{
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		for(i=1; i <= slotNum; i++)
		{
			query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_DELETE_WITH_VLAN );
			
			dbus_error_init(&err);

			dbus_message_append_args(	query,
									 	DBUS_TYPE_UINT16,&vlanid,
									 	DBUS_TYPE_INVALID);
			
			if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
			{
				if(i == local_slot_id)
				{
               		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
			   		vty_out(vty,"Can not connect to slot:%d \n",i);	
					continue;
				}
        	}
			else
			{
            	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);				
			}
			
			dbus_message_unref(query);
			if (NULL == reply) {
				vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",i);
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_SUCCESS;
			}

			if (!(dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&op_ret,
				DBUS_TYPE_INVALID))) {
				vty_out(vty,"Failed get args.\n");
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
					
					
			} 
			else {
				   if( FDB_RETURN_CODE_GENERAL == op_ret){
		                vty_out(vty,"%% Error,operations error when configuration!\n");
				    }
					else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == op_ret){
						vty_out(vty,"%% Error,the vlan input not exist!\n");
					}
					else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
						vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
					}
					else if(CMD_SUCCESS == op_ret)
					{
						ct_success = 1;
					}
				}
			dbus_message_unref(reply);
		}
		if(ct_success == 1)
        {
           	/* update g_vlanlist[] on Active-MCB & Standby-MCB. zhangdi 20110804*/
        	if(is_distributed == DISTRIBUTED_SYSTEM)	
        	{
    			vty_out(vty,"update g_static_fdb on SMU !\n");			
            	/* Wherever slot the CMD we input, then update the gvlanlist[] of 2 MCBs */
        		op_ret = dcli_update_gstaticfdb_for_master(vty,macAddr,vlanid,0,0,0,0,5);
        	    if( CMD_SUCCESS != op_ret)
        		{
        			vty_out(vty,"update g_static_fdb on SMU error !\n");					
        		    return CMD_WARNING;
        	    }
        	}					
        }
	}
	else
	{
		
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_DELETE_WITH_VLAN );
		
		dbus_error_init(&err);
		
		dbus_message_append_args(	query,
									DBUS_TYPE_UINT16,&vlanid,
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
		
		if (!(dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID))) {
			vty_out(vty,"Failed get args.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
				
				
		} 
		else {
			   if( FDB_RETURN_CODE_GENERAL == op_ret){
					vty_out(vty,"%% Error,operations error when configuration!\n");
				}
				else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == op_ret){
					vty_out(vty,"%% Error,the vlan input not exist!\n");
				}
				else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
					vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
				}
			}
		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;
}

/**************************************
*delete fdb item which belong to the port on Both Sw & Hw. 
*
* Usage:delete fdb static port PORTNO 
*
*input Params: 
*	
*		PORTNO:  destination port, form slot/port
*   
*
*output params:none
*     
****************************************/

DEFUN(config_fdb_static_delete_port_cmd_func,
	config_fdb_static_delete_port_cmd,
	"delete fdb static port PORTNO",
	"Delete FDB table\n"
	"Config FDB table \n"
	"Static FDB table \n"
	"FDB table port\n"
	CONFIG_ETHPORT_STR
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned char slotNum = 0,portNum = 0;
	unsigned int 	op_ret = 0;
	int local_slot_id = 0;
    int slotnum = 0;
	ETHERADDR		macAddr;
	memset(&macAddr,0,sizeof(ETHERADDR));
	op_ret =  parse_slotport_no((char*)argv[0],&slotNum,&portNum);
	if (NPD_FAIL == op_ret) {
		vty_out(vty,"% Bad Parameters,Unknow portno format!\n");
		return CMD_SUCCESS;
	}
	/*
	if((slotNum <MIN_SLOT ||slotNum>MAX_SLOT)||(portNum <MIN_PORT||portNum>MAX_PORT)){
		vty_out(vty,"%% Error, port outrange!\n");
		return CMD_SUCCESS;
	}
	*/
	if(is_distributed == DISTRIBUTED_SYSTEM)
	{
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotnum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotnum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_DELETE_WITH_PORT);
		
		dbus_error_init(&err);

		dbus_message_append_args(	query,
								 	DBUS_TYPE_BYTE,&slotNum,
								 	DBUS_TYPE_BYTE,&portNum,
								 	DBUS_TYPE_INVALID);
		
		if(local_slot_id == slotNum)
		{
			reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		}
		else
		{
			if(NULL == dbus_connection_dcli[slotNum]->dcli_dbus_connection)
			{
			   	vty_out(vty,"Can not connect to slot:%d, check the slot please !\n",slotNum);	
        		return CMD_WARNING;
			}
			else
			{
				reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slotNum]->dcli_dbus_connection,query,-1, &err);
			}
		}
		
		dbus_message_unref(query);
		if (NULL == reply) {
			vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",slotNum);
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}

		if (!(dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID))) {
			vty_out(vty,"Failed get args.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
				
				
		} 
		else {
			   if( FDB_RETURN_CODE_GENERAL == op_ret){
	                vty_out(vty,"%% Error,operations error when configuration!\n");
			    }

	            else if(FDB_RETURN_CODE_BADPARA == op_ret){
					vty_out(vty,"%% Error,the parameter input is error!\n");
	            }
				else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
					vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
				}
			}
		dbus_message_unref(reply);

		if(CMD_SUCCESS == op_ret)
        {
           	/* update g_vlanlist[] on Active-MCB & Standby-MCB. zhangdi 20110804*/
        	if(is_distributed == DISTRIBUTED_SYSTEM)	
        	{
    			vty_out(vty,"update g_static_fdb on SMU !\n");			
            	/* Wherever slot the CMD we input, then update the gvlanlist[] of 2 MCBs */
        		op_ret = dcli_update_gstaticfdb_for_master(vty,macAddr,0,slotNum,portNum,0,0,3);
        	    if( CMD_SUCCESS != op_ret)
        		{
        			vty_out(vty,"update g_static_fdb on SMU error !\n");					
        		    return CMD_WARNING;
        	    }
        	}					
        }
	}
	else
	{
		
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_STATIC_DELETE_WITH_PORT);
		
		dbus_error_init(&err);

		dbus_message_append_args(	query,
								 	DBUS_TYPE_BYTE,&slotNum,
								 	DBUS_TYPE_BYTE,&portNum,
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

		if (!(dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID))) {
			vty_out(vty,"Failed get args.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
				
				
		} 
		else {
			   if( FDB_RETURN_CODE_GENERAL == op_ret){
	                vty_out(vty,"%% Error,operations error when configuration!\n");
			    }

	            else if(FDB_RETURN_CODE_BADPARA == op_ret){
					vty_out(vty,"%% Error,the parameter input is error!\n");
	            }
				else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
					vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
				}
			}
		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;
}



/**************************************
*delete fdb item which belong to the port on Both Sw & Hw. 
*
* Usage:delete fdb TRUNK trunkno
*
*input Params: 
*	
*		TRUNKNO:  destination trunk, form trunkno
*   
*
*output params:none
*     
****************************************/

DEFUN(config_fdb_delete_trunk_cmd_func,
	config_fdb_delete_trunk_cmd,
	"delete fdb trunk <1-127>",
	"Delete FDB table\n"
	"Config FDB table \n"
	"FDB table trunk\n"
	"Config FDB trunk value, range in <1-127> \n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short trunk_no;
	unsigned int 	op_ret;
	int i;
	int local_slot_id = 0;
	int slotNum = 0;

	op_ret = parse_trunk_no((char*)argv[0],&trunk_no); 
	if (NPD_FAIL == op_ret) {
		vty_out(vty,"% Bad Parameters,Unknow trunk number\n");
		return CMD_SUCCESS;
	}
	if((trunk_no<0) || (trunk_no>127))
	{
		vty_out(vty,"trunk number must be in range of <1-127>\n");
		return CMD_SUCCESS;
	}

	if(is_distributed == DISTRIBUTED_SYSTEM)	
	{
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		for(i=1; i <= slotNum; i++)
		{
			query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_DELETE_WITH_TRUNK);
			
			dbus_error_init(&err);

			dbus_message_append_args(	query,
									 	DBUS_TYPE_UINT16,&trunk_no,
									 	DBUS_TYPE_INVALID);
			
			if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
			{
				if(i == local_slot_id)
				{
               		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
			   		vty_out(vty,"Can not connect to slot:%d \n",i);	
					continue;
				}
        	}
			else
			{
            	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);				
			}
			
			dbus_message_unref(query);
			if (NULL == reply) {
				vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",i);
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_SUCCESS;
			}

			if (!(dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&op_ret,
				DBUS_TYPE_INVALID))) {
				vty_out(vty,"Failed get args.\n");
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
					
					
			} 
			else {
				   if( FDB_RETURN_CODE_GENERAL == op_ret){
		                vty_out(vty,"%% Error,operations error when configuration!\n");
				    }

					else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
						vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
					}
			}
			dbus_message_unref(reply);
		}
	}
	else
	{
		
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_DELETE_WITH_TRUNK);
		
		dbus_error_init(&err);
		
		dbus_message_append_args(	query,
									DBUS_TYPE_UINT16,&trunk_no,
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
		
		if (!(dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID))) {
			vty_out(vty,"Failed get args.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
				
				
		} 
		else {
			   if( FDB_RETURN_CODE_GENERAL == op_ret){
					vty_out(vty,"%% Error,operations error when configuration!\n");
				}
		
				else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
					vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
				}
		}
		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;
}


DEFUN(show_fdb_one_cmd_func,
	show_fdb_one_cmd,
	"show fdb mac MAC vlan <1-4094>" ,
	"Show system information\n"
	"FDB table content!\n"
	"Config mac address!\n"
	"Mac address in Hex format\n"
	"Config vlan \n"
	"Config VLAN Id valid range <1-4094>\n"
	
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int dnumber = 0;
	unsigned int dcli_flag = 0;
	unsigned char  show_mac[6] ={0};
	unsigned short vlanid = 0;
	unsigned int devNum = 0;
	unsigned int portNum= 0;
	unsigned int trans_value1 = 0;
	unsigned int trans_value2 = 0;
	ETHERADDR		macAddr;
	unsigned short	vlanId = 0;
	unsigned int	op_ret = 0;
	int i;
	int local_slot_id = 0;
    int slotNum = 0;

	

    memset(&macAddr,0,sizeof(ETHERADDR));
	
	/*fetch the 1st param : MAC addr*/
	op_ret = parse_mac_addr((char *)argv[0],&macAddr);
	if (NPD_FAIL == op_ret) {
		vty_out(vty,"% Bad Parameters,Unknow mac addr format.\n");
		return CMD_SUCCESS;
	}
	
	op_ret = parse_short_parse((char*)argv[1], &vlanId);
	if (NPD_FAIL == op_ret) {
		vty_out(vty,"%% Error,Unknow vlan id format.\n");
		return CMD_SUCCESS;
	}
	if(vlanId <MIN_VLANID ||vlanId>MAX_VLANID){
   		vty_out(vty,"%% Error, vlan outrang!\n");
		return CMD_SUCCESS;
	}
	if(is_distributed == DISTRIBUTED_SYSTEM)	
    {
	    #if 1
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		for(i=1; i <= slotNum; i++)
		{
            
			query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
								NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_ONE);
	
			dbus_error_init(&err);

			dbus_message_append_args(	query,
									DBUS_TYPE_UINT16,&vlanId,
						 			DBUS_TYPE_BYTE,&macAddr.arEther[0],
						 			DBUS_TYPE_BYTE,&macAddr.arEther[1],
						 			DBUS_TYPE_BYTE,&macAddr.arEther[2],
						 			DBUS_TYPE_BYTE,&macAddr.arEther[3],
						 			DBUS_TYPE_BYTE,&macAddr.arEther[4],
								 	DBUS_TYPE_BYTE,&macAddr.arEther[5],						 	
								 	DBUS_TYPE_INVALID);
            
            if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
			{
    			if(i == local_slot_id)
				{
                    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
				   	vty_out(vty,"Can not connect to slot:%d \n",i);	
					continue;
				}
            }
			else
			{
                reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);				
			}
			
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
				dbus_message_iter_get_basic(&iter,&op_ret);
				 if(NPD_FDB_ERR_NONE != op_ret){
					if( FDB_RETURN_CODE_GENERAL  == op_ret){
						vty_out(vty,"%% Error,operations error when configuration!\n");
					}
					else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == op_ret){
						vty_out(vty,"%% Error,the vlan input not exist!\n");
					}
					else if(FDB_RETURN_CODE_BADPARA == op_ret){
						vty_out(vty,"%% Error,the parameter input is error!\n");
					}
					else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
						vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
					}
					else if(FDB_RETURN_CODE_SYSTEM_MAC == op_ret){
						vty_out(vty,"%% Error,the mac input conflict with system mac,please chang!\n");
					}
					dbus_message_unref(reply);
					return CMD_SUCCESS;
				}
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&dnumber);
				if (dnumber == 0){
					vty_out(vty,"THE ITEM IS NONE IN FDB TABLE !\n");
					if(i == slotNum)
					{
						return CMD_SUCCESS;
					}
					else
					{
						continue;
					}
				}
			
				vty_out(vty,"Codes:  CPU - target port which connect to cpu via pci interface\n");
				vty_out(vty,"		 HSC - high speed channel direct to cpu rgmii interface\n\n");
			
				vty_out(vty,"%-17s	%-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
				vty_out(vty,"%-17s	%-8s%-11s%-8s%-8s%-6s\n","MAC","VLAN","SLOT/PORT","TRUNK","VID","VIDX");
				vty_out(vty,"%-17s	%-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
				
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&dcli_flag);
				
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&vlanid);
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&trans_value1);
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&trans_value2);
				
			
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&show_mac[0]);
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&show_mac[1]);
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&show_mac[2]);
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&show_mac[3]);
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&show_mac[4]);
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&show_mac[5]);
				
				if (dcli_flag == port_type){
					if(CPU_PORT_VIRTUAL_SLOT == trans_value1) { /*static FDB to CPU or SPI (has the same virtual slot number)*/
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%-11s%-8s%-8s%-6s\n",	\
								show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
								vlanid,(CPU_PORT_VIRTUAL_PORT == trans_value2) ? "CPU": \
								(SPI_PORT_VIRTUAL_PORT == trans_value2) ? "HSC":"ERR"," - "," - "," - ");
					}
					else {
						vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%5d/%-6d%-8s%-8s%-6s\n",   \
									show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
									vlanid,trans_value1,trans_value2," - "," - "," - ");
					}
				}
				else if (dcli_flag == trunk_type){				
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%-11s%-8d%-8s%-6s\n",	 \
								show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
								vlanid," - ",trans_value1," - "," - ");
				}
				else if (dcli_flag == vidx_type){				
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%-11s%-8s%-8d%-6s\n",	  \
							   show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5], 	\
							   vlanid," - "," - ",trans_value1," - ");	
				}			
				else if (dcli_flag == vid_type){				
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%-11s%-8s%-8s%-8d\n",	  \
							   show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5], 	\
							   vlanid," - "," - "," - ",trans_value1);				
				}
				else {
					vty_out (vty,"sorry interface type wrong !\n");
					return -1;;
				}
			
			dbus_message_unref(reply);

		}
        /*free(vlanName);*/
        return CMD_SUCCESS;		
		#endif
    }
	else
	{
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
								NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_ONE);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							DBUS_TYPE_UINT16,&vlanId,
						 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
						 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
						 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
						 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
						 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
						 	DBUS_TYPE_BYTE,&macAddr.arEther[5],						 	
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
	
	dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&op_ret);
		 if(NPD_FDB_ERR_NONE != op_ret){
			if( FDB_RETURN_CODE_GENERAL  == op_ret){
				vty_out(vty,"%% Error,operations error when configuration!\n");
			}
			else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == op_ret){
				vty_out(vty,"%% Error,the vlan input not exist!\n");
			}
			else if(FDB_RETURN_CODE_BADPARA == op_ret){
				vty_out(vty,"%% Error,the parameter input is error!\n");
			}
			else if(FDB_RETURN_CODE_OCCUR_HW == op_ret){
				vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
			}
			else if(FDB_RETURN_CODE_SYSTEM_MAC == op_ret){
				vty_out(vty,"%% Error,the mac input conflict with system mac,please chang!\n");
			}
		    dbus_message_unref(reply);
	        return CMD_SUCCESS;
		}
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&dnumber);
		if (dnumber == 0){
			vty_out(vty,"THE ITEM IS NONE IN FDB TABLE !\n");
			return CMD_SUCCESS;
		}

		vty_out(vty,"Codes:  CPU - target port which connect to cpu via pci interface\n");
		vty_out(vty,"        HSC - high speed channel direct to cpu rgmii interface\n\n");

		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","MAC","VLAN","SLOT/PORT","TRUNK","VID","VIDX");
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&dcli_flag);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&vlanid);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&trans_value1);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&trans_value2);
		

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&show_mac[0]);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&show_mac[1]);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&show_mac[2]);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&show_mac[3]);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&show_mac[4]);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&show_mac[5]);
		
		if (dcli_flag == port_type){
			if(CPU_PORT_VIRTUAL_SLOT == trans_value1) { /*static FDB to CPU or SPI (has the same virtual slot number)*/
			vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8s%-6s\n",	\
						show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
						vlanid,(CPU_PORT_VIRTUAL_PORT == trans_value2) ? "CPU":	\
						(SPI_PORT_VIRTUAL_PORT == trans_value2) ? "HSC":"ERR"," - "," - "," - ");
			}
			else {
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%5d/%-6d%-8s%-8s%-6s\n",   \
							show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
							vlanid,trans_value1,trans_value2," - "," - "," - ");
			}
		}
		else if (dcli_flag == trunk_type){ 				
			vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8d%-8s%-6s\n",    \
				        show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
				        vlanid," - ",trans_value1," - "," - ");
		}
		else if (dcli_flag == vidx_type){				
			vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8d%-6s\n",     \
				       show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
				       vlanid," - "," - ",trans_value1," - ");	
		}			
		else if (dcli_flag == vid_type){				
			vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8s%-8d\n",     \
				       show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
				       vlanid," - "," - "," - ",trans_value1);				
		}
		else {
			vty_out (vty,"sorry interface type wrong !\n");
			return -1;;
		}
	
	dbus_message_unref(reply);
	}
	return CMD_SUCCESS;
}

/**************************************
*display valid fdb item count on hw . 
*
* Usage:show fdb count 
*
*input Params: none
*
*output params:
*	fdb valid count.
*     
****************************************/

DEFUN(show_fdb_count_cmd_func,
	show_fdb_count_cmd,
	"show fdb count" ,
	"Show system information\n"
	"FDB table content!\n"
	"Show FDB valid count!\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int dnumber = 0;
	int i;

	int local_slot_id = 0;
    int slotNum = 0;
	
	if(is_distributed == DISTRIBUTED_SYSTEM)	
    {
	    #if 1
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		for(i=1; i <= slotNum; i++)
		{
            
			query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH, \
												NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_COUNT);
				
			dbus_error_init(&err);
            
            if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
			{
    			if(i == local_slot_id)
				{
                    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
				   	vty_out(vty,"Can not connect to slot:%d \n",i);	
					continue;
				}
            }
			else
			{
                reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);				
			}
			
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
				DBUS_TYPE_UINT32,&dnumber,
				DBUS_TYPE_INVALID)) {
					   if(dnumber==0){
							vty_out(vty,"%% FDB TABLE IS NONE!\n");
							if(i == slotNum)
							{
								return CMD_SUCCESS;
							}
							else
							{
								continue;
							}
							}
						vty_out(vty,"FDB Table count %d.\n",dnumber);	
			} 
			else {
					
				vty_out(vty,"Failed get args.\n");
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
			}
			
			dbus_message_unref(reply);

		}
        /*free(vlanName);*/
        return CMD_SUCCESS;		
		#endif
    }
	else
	{
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
										NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_COUNT);
	
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
		if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&dnumber,
			DBUS_TYPE_INVALID)) {
		     	  if(dnumber==0){
					vty_out(vty,"%% FDB TABLE IS NONE!\n");
					return CMD_SUCCESS;
					}
					vty_out(vty,"FDB Table count %d.\n",dnumber);	
		} 
		else {
			
			vty_out(vty,"Failed get args.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
		}
	
		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;
}

/**************************************
*display valid fdb items  on hw . 
*
* Usage:show fdb 
*
*input Params: none
*
*output params:
*	fdb information ,include mac address, vlan index, destination interface.
*     
****************************************/

DEFUN(show_fdb_cmd_func,
	show_fdb_cmd,
	"show fdb" ,
	"Show system information\n"
	"FDB table content!\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int dnumber = 0;
	unsigned int dcli_flag =0;
	unsigned int type_flag = 0;
	unsigned char* type = NULL;
	
	unsigned char  show_mac[6] = {0};
	unsigned short vlanid=0;
	unsigned int i=0;
	unsigned int devNum = 0;
	unsigned int portNum = 0;
	unsigned int trans_value1 = 0;
	unsigned int trans_value2 = 0;
	unsigned int k=0;

	int local_slot_id = 0;
    int slotNum = 0;
	
	if(is_distributed == DISTRIBUTED_SYSTEM)	
    {
	    #if 1
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		for(k=1; k <= slotNum; k++)
		{
            query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
								NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE);

            dbus_error_init(&err);
            if(NULL == dbus_connection_dcli[k]->dcli_dbus_connection) 				
			{
    			if(k == local_slot_id)
				{
                    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
				   	vty_out(vty,"Can not connect to slot:%d \n",k);	
					continue;
				}
            }
			else
			{
                reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[k]->dcli_dbus_connection,query,-1, &err);				
			}
			
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
		dbus_message_iter_get_basic(&iter,&dnumber);
		if (dnumber == 0){
			vty_out(vty,"%% FDB TABLE IS NONE!\n");
			#if 0
			return CMD_SUCCESS;
			#endif
			if(k == slotNum)
			{
				return CMD_SUCCESS;
			}
			else
			{
				continue;
			}
		}

		vty_out(vty,"Codes:  CPU - target port which connect to cpu via pci interface\n");
		vty_out(vty,"        HSC - high speed channel direct to cpu rgmii interface\n\n");
		/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info*/
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s%-8s\n","=================",	"======", "=========","======","======","======"," ======");
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s%-8s\n","MAC","VLAN","SLOT/PORT","TRUNK","VID","VIDX","TYPE");
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s%-8s\n","=================",	"======", "=========","======","======","======"," ======");
		dbus_message_iter_next(&iter);
	
		dbus_message_iter_recurse(&iter,&iter_array);
		for ( i=0; dnumber > i; i++){
			DBusMessageIter iter_struct;
		
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dcli_flag);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&vlanid);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trans_value1);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trans_value2);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&type_flag);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[5]);
		
			if( 0 ==type_flag){
       	    	type = "DYNAMIC";
			}
			else if(1 ==type_flag){
				type = "STATIC";
			}
			else {
				vty_out (vty,"sorry fdb item type wrong !\n");
				return -1;
			}
			if (dcli_flag == port_type){
				if(CPU_PORT_VIRTUAL_SLOT == trans_value1) { /*static FDB to CPU or SPI (has the same virtual slot number)*/
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%8s%7s%8s%8s%9s\n",	\
							show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
							vlanid,(CPU_PORT_VIRTUAL_PORT == trans_value2) ? "CPU":	\
							(SPI_PORT_VIRTUAL_PORT == trans_value2) ? "HSC":"ERR"," - "," - "," - "," - ");
				}
				else {
					if(trans_value1 <= 10){
						vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%5d/%-6d%-8s%-8s%-6s%-6s\n",   \
									show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
									vlanid,trans_value1,trans_value2," - "," - "," - ",type);
					}
							
				}
			}
			else if (dcli_flag == trunk_type){ 				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%8s%6d%9s%8s%10s\n",    \
				       	 show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
				       	 vlanid," - ",trans_value1," - "," - ",type);
			}
		
			else if (dcli_flag == vid_type){				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8d%-6s%-6s\n",     \
				      	 show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
				      	 vlanid," - "," - ",trans_value1," - ",type);	
			}			
			else if (dcli_flag == vidx_type){				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8s%-8d%-6s\n",     \
				     	  show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
				      	 vlanid," - "," - "," - ",trans_value1,type);				
			}
			else {
				vty_out (vty,"sorry interface type wrong !\n");
				return -1;
			}
		
			dbus_message_iter_next(&iter_array);
		
			}
	
		dbus_message_unref(reply);
		}
        /*free(vlanName);*/
        return CMD_SUCCESS;		
		#endif
    }
	else
	{
	
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
								NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE);
	
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
		dbus_message_iter_get_basic(&iter,&dnumber);
		if (dnumber == 0){
			vty_out(vty,"%% FDB TABLE IS NONE!\n");
			return CMD_SUCCESS;
			}

		vty_out(vty,"Codes:  CPU - target port which connect to cpu via pci interface\n");
		vty_out(vty,"        HSC - high speed channel direct to cpu rgmii interface\n\n");
		/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info*/
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s%-8s\n","=================",	"======", "=========","======","======","======"," ======");
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s%-8s\n","MAC","VLAN","SLOT/PORT","TRUNK","VID","VIDX","TYPE");
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s%-8s\n","=================",	"======", "=========","======","======","======"," ======");
		dbus_message_iter_next(&iter);
	
		dbus_message_iter_recurse(&iter,&iter_array);
		for ( ; dnumber > i; i++){
			DBusMessageIter iter_struct;
		
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dcli_flag);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&vlanid);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trans_value1);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trans_value2);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&type_flag);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[5]);
		
			if( 0 ==type_flag){
       	    	type = "DYNAMIC";
			}
			else if(1 ==type_flag){
				type = "STATIC";
			}
			else {
				vty_out (vty,"sorry fdb item type wrong !\n");
				return -1;
			}
			if (dcli_flag == port_type){
				if(CPU_PORT_VIRTUAL_SLOT == trans_value1) { /*static FDB to CPU or SPI (has the same virtual slot number)*/
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%8s%7s%8s%8s%9s\n",	\
							show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
							vlanid,(CPU_PORT_VIRTUAL_PORT == trans_value2) ? "CPU":	\
							(SPI_PORT_VIRTUAL_PORT == trans_value2) ? "HSC":"ERR"," - "," - "," - "," - ");
				}
				else {
					if(trans_value1 <= 3){
						vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%5d/%-6d%-8s%-8s%-6s%-6s\n",   \
									show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
									vlanid,trans_value1,trans_value2," - "," - "," - ",type);
					}
							
				}
			}
			else if (dcli_flag == trunk_type){ 				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%8s%6d%9s%8s%10s\n",    \
				       	 show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
				       	 vlanid," - ",trans_value1," - "," - ",type);
			}
		
			else if (dcli_flag == vid_type){				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8d%-6s%-6s\n",     \
				      	 show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
				      	 vlanid," - "," - ",trans_value1," - ",type);	
			}			
			else if (dcli_flag == vidx_type){				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8s%-8d%-6s\n",     \
				     	  show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
				      	 vlanid," - "," - "," - ",trans_value1,type);				
			}
			else {
				vty_out (vty,"sorry interface type wrong !\n");
				return -1;
			}
		
			dbus_message_iter_next(&iter_array);
		
			}
	
		dbus_message_unref(reply);
	}

	return CMD_SUCCESS;	
}

/**************************************
*display dynamic fdb items  on hw . 
*
* Usage:show fdb dynamic
*
*input Params: none
*
*output params:
*	fdb information ,include mac address, vlan index, destination interface.
*     
****************************************/



DEFUN(show_fdb_dynamic_cmd_func,
	show_fdb_dynamic_cmd,
	"show fdb dynamic" ,
	"Show system information\n"
	"FDB table content!\n"
	"Dynamic FDB items\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int dnumber = 0;
	unsigned int dcli_flag= 0;
	
	unsigned char  show_mac[6];
	unsigned short vlanid = 0;
	unsigned int i=0;
	unsigned int devNum = 0;
	unsigned int portNum = 0;
	unsigned int trans_value1 = 0;
	unsigned int trans_value2 = 0;
	unsigned int k=0;


	int local_slot_id = 0;
    int slotNum = 0;
	
	if(is_distributed == DISTRIBUTED_SYSTEM)	
    {
	    #if 1
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		for(k=1; k <= slotNum; k++)
		{
            query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
									NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_DYNAMIC_TABLE);

            dbus_error_init(&err);
            if(NULL == dbus_connection_dcli[k]->dcli_dbus_connection) 				
			{
    			if(k == local_slot_id)
				{
                    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
				   	vty_out(vty,"Can not connect to slot:%d \n",k);	
					continue;
				}
            }
			else
			{
                reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[k]->dcli_dbus_connection,query,-1, &err);				
			}
			
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
		dbus_message_iter_get_basic(&iter,&dnumber);
		if (dnumber == 0){
			vty_out(vty,"%% FDB TABLE IS NONE!\n");
			if(k == slotNum)
				{
					return CMD_SUCCESS;
				}
				else
				{
					continue;
				}
		}

		vty_out(vty,"Codes:  CPU - target port which connect to cpu via pci interface\n");
		vty_out(vty,"        HSC - high speed channel direct to cpu rgmii interface\n\n");
	/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info*/
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","MAC","VLAN","SLOT/PORT","TRUNK","VID","VIDX");
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
		dbus_message_iter_next(&iter);
	
		dbus_message_iter_recurse(&iter,&iter_array);
		for ( i=0; dnumber > i; i++){
			DBusMessageIter iter_struct;
		
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dcli_flag);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&vlanid);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trans_value1);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trans_value2);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[5]);
			if (dcli_flag == port_type){
				if(CPU_PORT_VIRTUAL_SLOT == trans_value1) { /*static FDB to CPU or SPI (has the same virtual slot number)*/
			   	 vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8s%-6s\n",	\
							show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
							vlanid,(CPU_PORT_VIRTUAL_PORT == trans_value2) ? "CPU":	\
							(SPI_PORT_VIRTUAL_PORT == trans_value2) ? "HSC":"ERR"," - "," - "," - ");
				}
				else {
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%5d/%-6d%-8s%-8s%-6s\n",   \
								show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
								vlanid,trans_value1,trans_value2," - "," - "," - ");
				}
			}
			else if (dcli_flag == trunk_type){ 				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8d%-8s%-6s\n",    \
				       	 show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
				       	 vlanid," - ",trans_value1," - "," - ");
			}
		/*
		else if (dcli_flag == vid_type){				
			vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8d%-6s\n",     \
				       show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
				       vlanid," - "," - ",trans_value1," - ");	
		}			
		else if (dcli_flag == vidx_type){				
			vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8s%-8d\n",     \
				       show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
				       vlanid," - "," - "," - ",trans_value1);				
		}
		*/
			else {
				vty_out (vty,"sorry interface type wrong !\n");
				return -1;;
			}
		
			dbus_message_iter_next(&iter_array);
		
			}
	
			dbus_message_unref(reply);
		}
        /*free(vlanName);*/
        return CMD_SUCCESS;		
		#endif
    }
	else
	{
	
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
									NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_DYNAMIC_TABLE);
	
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
		dbus_message_iter_get_basic(&iter,&dnumber);
		if (dnumber == 0){
			vty_out(vty,"%% FDB TABLE IS NONE!\n");
			return CMD_SUCCESS;
		}

		vty_out(vty,"Codes:  CPU - target port which connect to cpu via pci interface\n");
		vty_out(vty,"        HSC - high speed channel direct to cpu rgmii interface\n\n");
	/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info*/
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","MAC","VLAN","SLOT/PORT","TRUNK","VID","VIDX");
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
		dbus_message_iter_next(&iter);
	
		dbus_message_iter_recurse(&iter,&iter_array);
		for ( ; dnumber > i; i++){
			DBusMessageIter iter_struct;
		
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dcli_flag);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&vlanid);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trans_value1);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trans_value2);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[5]);
			if (dcli_flag == port_type){
				if(CPU_PORT_VIRTUAL_SLOT == trans_value1) { /*static FDB to CPU or SPI (has the same virtual slot number)*/
			   	 vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8s%-6s\n",	\
							show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
							vlanid,(CPU_PORT_VIRTUAL_PORT == trans_value2) ? "CPU":	\
							(SPI_PORT_VIRTUAL_PORT == trans_value2) ? "HSC":"ERR"," - "," - "," - ");
				}
				else {
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%5d/%-6d%-8s%-8s%-6s\n",   \
								show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
								vlanid,trans_value1,trans_value2," - "," - "," - ");
				}
			}
			else if (dcli_flag == trunk_type){ 				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8d%-8s%-6s\n",    \
				       	 show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
				       	 vlanid," - ",trans_value1," - "," - ");
			}
		/*
		else if (dcli_flag == vid_type){				
			vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8d%-6s\n",     \
				       show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
				       vlanid," - "," - ",trans_value1," - ");	
		}			
		else if (dcli_flag == vidx_type){				
			vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8s%-8d\n",     \
				       show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
				       vlanid," - "," - "," - ",trans_value1);				
		}
		*/
			else {
				vty_out (vty,"sorry interface type wrong !\n");
				return -1;;
			}
		
			dbus_message_iter_next(&iter_array);
		
			}
	
		dbus_message_unref(reply);
	}

	return CMD_SUCCESS;	
}



/**************************************
*display valid static fdb items  on hw . 
*
* Usage:show fdb static
*
*input Params: none
*
*output params:
*	fdb information ,include mac address, vlan index, destination interface.
*     
****************************************/

DEFUN(show_fdb_static_cmd_func,
	show_fdb_static_cmd,
	"show fdb static" ,
	"Show system information\n"
	"FDB table content!\n"
	"Show FDB table static item!\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int dnumber = 0 ;
	unsigned int dcli_flag = 0;
	
	unsigned char  show_mac[6]={0};
	unsigned short vlanid =0;

	unsigned int i=0;
	unsigned int devNum = 0;
	unsigned int portNum = 0;
	unsigned int trans_value1 = 0;
	unsigned int trans_value2 = 0;
	unsigned int k=0;

	int local_slot_id = 0;
    int slotNum = 0;
	
	if(is_distributed == DISTRIBUTED_SYSTEM)	
    {
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_STATIC_TABLE);
	
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
		dbus_message_iter_get_basic(&iter,&dnumber); 
		if (dnumber == 0){
			vty_out(vty,"FDB ITEM IS NONE!\n");
			return CMD_SUCCESS;

		}

		vty_out(vty,"Codes:  CPU - target port which connect to cpu via pci interface\n");
		vty_out(vty,"        HSC - high speed channel direct to cpu rgmii interface\n\n");
		/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info */
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","MAC","VLAN","SLOT/PORT","TRUNK","VID","VIDX");
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
		dbus_message_iter_next(&iter);
	
		dbus_message_iter_recurse(&iter,&iter_array);
		for (i =0; i< dnumber; i++){
			DBusMessageIter iter_struct;
		
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dcli_flag);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&vlanid);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trans_value1);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trans_value2);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[5]);
			if (dcli_flag == port_type){
				if(CPU_PORT_VIRTUAL_SLOT == trans_value1) { /*static FDB to CPU or SPI (has the same virtual slot number)*/
			 	   vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8s%-6s\n",	\
								show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
								vlanid,(CPU_PORT_VIRTUAL_PORT == trans_value2) ? "CPU":	\
								(SPI_PORT_VIRTUAL_PORT == trans_value2) ? "HSC":"ERR"," - "," - "," - ");
				}
				else {
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%d/%-9d%-8s%-8s%-6s\n",   \
								show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
								vlanid,trans_value1,trans_value2,  " - "," - "," - ");
				}
			}
			else if (dcli_flag == trunk_type){ 				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8d%-8s%-6s\n",    \
				       	 show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
				       	 vlanid," - ",trans_value1," - "," - ");
			}
			else if (dcli_flag == vid_type){				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8d%-6s\n",     \
				      	 show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
				 			vlanid," - "," - ",trans_value1," - ");	
			}			
			else if (dcli_flag == vidx_type){				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8s%-8d\n",     \
				       show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
				       vlanid," - "," - "," - ",trans_value1);				
			}
			else {
				vty_out (vty,"sorry interface type wrong !\n");
				return -1;;
			}
		
			dbus_message_iter_next(&iter_array);
		
		}
	
		dbus_message_unref(reply);
    }
	else
	{

	
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_STATIC_TABLE);
	
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
		dbus_message_iter_get_basic(&iter,&dnumber); 
		if (dnumber == 0){
			vty_out(vty,"FDB ITEM IS NONE!\n");
			return CMD_SUCCESS;

		}

		vty_out(vty,"Codes:  CPU - target port which connect to cpu via pci interface\n");
		vty_out(vty,"        HSC - high speed channel direct to cpu rgmii interface\n\n");
		/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info */
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","MAC","VLAN","SLOT/PORT","TRUNK","VID","VIDX");
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
		dbus_message_iter_next(&iter);
	
		dbus_message_iter_recurse(&iter,&iter_array);
		for (i =0; i< dnumber; i++){
			DBusMessageIter iter_struct;
		
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dcli_flag);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&vlanid);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trans_value1);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trans_value2);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[5]);
			if (dcli_flag == port_type){
				if(CPU_PORT_VIRTUAL_SLOT == trans_value1) { /*static FDB to CPU or SPI (has the same virtual slot number)*/
			 	   vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8s%-6s\n",	\
								show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
								vlanid,(CPU_PORT_VIRTUAL_PORT == trans_value2) ? "CPU":	\
								(SPI_PORT_VIRTUAL_PORT == trans_value2) ? "HSC":"ERR"," - "," - "," - ");
				}
				else {
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%d/%-9d%-8s%-8s%-6s\n",   \
								show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
								vlanid,trans_value1,trans_value2,  " - "," - "," - ");
				}
			}
			else if (dcli_flag == trunk_type){ 				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8d%-8s%-6s\n",    \
				       	 show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
				       	 vlanid," - ",trans_value1," - "," - ");
			}
			else if (dcli_flag == vid_type){				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8d%-6s\n",     \
				      	 show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
				 			vlanid," - "," - ",trans_value1," - ");	
			}			
			else if (dcli_flag == vidx_type){				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8s%-8d\n",     \
				       show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
				       vlanid," - "," - "," - ",trans_value1);				
			}
			else {
				vty_out (vty,"sorry interface type wrong !\n");
				return -1;;
			}
		
			dbus_message_iter_next(&iter_array);
		
		}
	
		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;
}	
/**************************************
*display valid blacklist fdb items  on hw . 
*
* Usage:show fdb blacklist
*
*input Params: none
*
*output params:
*	fdb information ,include mac address, vlan index, destination interface.
*     
****************************************/

DEFUN(show_fdb_blacklist_cmd_func,
	show_fdb_blacklist_cmd,
	"show fdb blacklist" ,
	"Show system information\n"
	"FDB table content!\n"
	"FDB table blacklist item!\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int dnumber = 0;
    unsigned char dmac = 0;
	unsigned char smac = 0;
	
	unsigned char  show_mac[6];
	unsigned short vlanid = 0;
	unsigned int i=0;
	unsigned int k=0;
	int local_slot_id = 0;
    int slotNum = 0;
	
	if(is_distributed == DISTRIBUTED_SYSTEM)	
    {
	    #if 1
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		for(k=1; k <= slotNum; k++)
		{
           query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_BLACKLIST_TABLE);

            dbus_error_init(&err);
            if(NULL == dbus_connection_dcli[k]->dcli_dbus_connection) 				
			{
    			if(k == local_slot_id)
				{
                    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
				   	vty_out(vty,"Can not connect to slot:%d \n",k);	
					continue;
				}
            }
			else
			{
                reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[k]->dcli_dbus_connection,query,-1, &err);				
			}
			
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
			dbus_message_iter_get_basic(&iter,&dnumber);
			if (dnumber == 0){
				vty_out(vty,"%% FDB ITEM IS NONE!\n");
				if(k == slotNum)
				{
					return CMD_SUCCESS;
				}
				else
				{
					continue;
				}
			}
			
			/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info*/
			vty_out(vty,"%-17s	%-8s%-8s%-8s\n","=================",	"======", "=========", "=========");
			vty_out(vty,"%-17s	%-8s%-8s%-8s\n","MAC","VLAN","DMAC","SMAC");
			vty_out(vty,"%-17s	%-8s%-8s%-8s\n","=================",	"======", "=========", "=========");
			dbus_message_iter_next(&iter);
			
			dbus_message_iter_recurse(&iter,&iter_array);
			for ( i=0; dnumber > i; i++){
				DBusMessageIter iter_struct;
				
				dbus_message_iter_recurse(&iter_array,&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&dmac);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&smac);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&vlanid);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&show_mac[0]);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&show_mac[1]);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&show_mac[2]);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&show_mac[3]);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&show_mac[4]);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&show_mac[5]);
						
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d	 %d 	%d\n",\
								show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],\
								vlanid,dmac,smac);
			
				
				dbus_message_iter_next(&iter_array);
				
			}
			
			dbus_message_unref(reply);

		}
        /*free(vlanName);*/
        return CMD_SUCCESS;		
		#endif
    }
	else
	{
	
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_BLACKLIST_TABLE);
	
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
		dbus_message_iter_get_basic(&iter,&dnumber);
		if (dnumber == 0){
			vty_out(vty,"%% FDB ITEM IS NONE!\n");
			return CMD_SUCCESS;
		}
	
	/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info*/
		vty_out(vty,"%-17s  %-8s%-8s%-8s\n","=================",	"======", "=========", "=========");
		vty_out(vty,"%-17s  %-8s%-8s%-8s\n","MAC","VLAN","DMAC","SMAC");
		vty_out(vty,"%-17s  %-8s%-8s%-8s\n","=================",	"======", "=========", "=========");
		dbus_message_iter_next(&iter);
	
		dbus_message_iter_recurse(&iter,&iter_array);
		for ( ; dnumber > i; i++){
			DBusMessageIter iter_struct;
		
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dmac);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&smac);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&vlanid);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[5]);
				
			vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d     %d     %d\n",\
				        show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],\
				        vlanid,dmac,smac);

		
			dbus_message_iter_next(&iter_array);
		
		}
	
		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;
}	

/**************************************
*display valid fdb items which destination port is the port on hw . 
*
* Usage:show fdb port PORTNO
*
*input Params:
* 	PORTNO:  destination port, form slot/port
*
*output params:
*	fdb information ,include mac address, vlan index, destination interface.
*     
****************************************/

DEFUN(show_fdb_port_cmd_func,
	show_fdb_port_cmd,
	"show fdb port PORTNO" ,
	"Show system information\n"
	"FDB table content!\n"
	"Show match port FDB!\n"
	CONFIG_ETHPORT_STR
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int dnumber = 0;
	unsigned int dcli_flag = 0;
	
	unsigned char  show_mac[6] ={0};
	unsigned short vlanid = 0;
	unsigned int i=0;
	unsigned char slotNum = 0;
	unsigned char portNum = 0;
	unsigned int trans_value1 = 0;
	unsigned int trans_value2 = 0;
	int op_ret = 0;
	unsigned int k=0;
	char path_start_no[64];
	char path_cnt[64];
	int asic_port_start_no = 0,asic_port_cnt = 0,asic_port_end_no;
	
	
	op_ret =  parse_slotport_no((char*)argv[0],&slotNum,&portNum);
	if (NPD_FAIL == op_ret) 
	{
		vty_out(vty,"%% Error,Unknow portno format!\n");
		return CMD_SUCCESS;
	}
	/*
	if((slotNum <MIN_SLOT ||slotNum>MAX_SLOT)||(portNum <MIN_PORT||portNum>MAX_PORT)){
		vty_out(vty,"err port outrange!\n");
		return CMD_SUCCESS;
	}
	*/

	int local_slot_id = 0;
    int slotnum = 0;
	
	if(is_distributed == DISTRIBUTED_SYSTEM)	
    {
		#if 1
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotnum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotnum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		if(slotNum > slotnum)
		{
			vty_out(vty,"Param err,slotnum should be in <1-%d>\n",slotnum);
			return CMD_WARNING;
		}
		sprintf(path_start_no, "/dbm/product/slot/slot%d/asic_start_no", slotNum);
		sprintf(path_cnt, "/dbm/product/slot/slot%d/asic_port_num", slotNum);
		asic_port_start_no = get_product_info(path_start_no);
		asic_port_cnt = get_product_info(path_cnt);
	    if((asic_port_start_no<0)||(asic_port_start_no>64))	
	    {
			vty_out(vty,"There is no board on slot %d!\n",slotNum);
			return CMD_WARNING;
	    }
		asic_port_end_no = asic_port_start_no+asic_port_cnt-1;
		if((portNum < asic_port_start_no) || (portNum > asic_port_end_no))
		{
			if((asic_port_start_no == 0) || (asic_port_cnt == 0))
			{
				vty_out(vty,"Param err,this board has no asic ports\n");
				return CMD_WARNING;
			}
			vty_out(vty,"Param err,portnum should be in <%d-%d>\n",asic_port_start_no,asic_port_end_no);
			return CMD_WARNING;
		}
		
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
								NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_PORT);

		dbus_error_init(&err);
		dbus_message_append_args(	query,
						DBUS_TYPE_BYTE,&slotNum,
					 	DBUS_TYPE_BYTE,&portNum,
					 	DBUS_TYPE_INVALID);
		if(local_slot_id == slotNum)
		{
			reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		}
		else
		{
			if(NULL == dbus_connection_dcli[slotNum]->dcli_dbus_connection)
			{
			   	vty_out(vty,"Can not connect to slot:%d, check the slot please !\n",slotNum);	
				return CMD_WARNING;
			}
			else
			{
				reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slotNum]->dcli_dbus_connection,query,-1, &err);
			}
		}

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
		dbus_message_iter_get_basic(&iter,&dnumber);

		if (dnumber == 0){
			vty_out(vty,"% THE ITEM IS NONE !\n");
			return CMD_SUCCESS;
		}

		vty_out(vty,"Codes:  CPU - target port which connect to cpu via pci interface\n");
		vty_out(vty,"		 HSC - high speed channel direct to cpu rgmii interface\n\n");

		/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info*/
		vty_out(vty,"%-17s	%-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
		vty_out(vty,"%-17s	%-8s%-11s%-8s%-8s%-6s\n","MAC","VLAN","SLOT/PORT","TRUNK","VID","VIDX");
		vty_out(vty,"%-17s	%-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
		dbus_message_iter_next(&iter);

		dbus_message_iter_recurse(&iter,&iter_array);
		for ( i=0; dnumber > i; i++)
		{
			DBusMessageIter iter_struct;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dcli_flag);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&vlanid);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trans_value1);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trans_value2);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[5]);
			if (dcli_flag == port_type){
				if(CPU_PORT_VIRTUAL_SLOT == trans_value1) { /*static FDB to CPU or SPI (has the same virtual slot number)*/
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%-11s%-8s%-8s%-6s\n",	\
								show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
								vlanid,(CPU_PORT_VIRTUAL_PORT == trans_value2) ? "CPU": \
								(SPI_PORT_VIRTUAL_PORT == trans_value2) ? "HSC":"ERR"," - "," - "," - ");
				}
				else {
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%5d/%-6d%-8s%-8s%-6s\n",   \
								show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
								vlanid,trans_value1,trans_value2," - "," - "," - ");
				}
			}
			else if (dcli_flag == trunk_type){				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%-11s%-8d%-8s%-6s\n",	 \
							show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
							vlanid," - ",trans_value1," - "," - ");
			}
			else if (dcli_flag == vid_type){				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%-11s%-8s%-8d%-6s\n",	  \
						   show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5], 	\
						   vlanid," - "," - ",trans_value1," - ");	
			}			
			else if (dcli_flag == vidx_type){				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%-11s%-8s%-8s%-8d\n",	  \
						   show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5], 	\
						   vlanid," - "," - "," - ",trans_value1);				
			}
			else {
				vty_out (vty,"sorry interface type wrong !\n");
				return -1;;
			}
			
			
			dbus_message_iter_next(&iter_array);
		}
		dbus_message_unref(reply);
		/*free(vlanName);*/
#endif
    }
	else
	{
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
									NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_PORT);
	
		dbus_error_init(&err);

		dbus_message_append_args(	query,
								DBUS_TYPE_BYTE,&slotNum,
						 		DBUS_TYPE_BYTE,&portNum,
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
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&dnumber);
	
		if (dnumber == 0){
			vty_out(vty,"% THE ITEM IS NONE !\n");
			return CMD_SUCCESS;
		}

		vty_out(vty,"Codes:  CPU - target port which connect to cpu via pci interface\n");
		vty_out(vty,"        HSC - high speed channel direct to cpu rgmii interface\n\n");

	/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info*/
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","MAC","VLAN","SLOT/PORT","TRUNK","VID","VIDX");
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
		dbus_message_iter_next(&iter);
	
		dbus_message_iter_recurse(&iter,&iter_array);
		for ( ; dnumber > i; i++){
			DBusMessageIter iter_struct;
		
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dcli_flag);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&vlanid);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trans_value1);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trans_value2);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[5]);
			if (dcli_flag == port_type){
				if(CPU_PORT_VIRTUAL_SLOT == trans_value1) { /*static FDB to CPU or SPI (has the same virtual slot number)*/
			   	 vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8s%-6s\n",	\
								show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
								vlanid,(CPU_PORT_VIRTUAL_PORT == trans_value2) ? "CPU":	\
								(SPI_PORT_VIRTUAL_PORT == trans_value2) ? "HSC":"ERR"," - "," - "," - ");
				}
				else {
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%5d/%-6d%-8s%-8s%-6s\n",   \
								show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
								vlanid,trans_value1,trans_value2," - "," - "," - ");
				}
			}
			else if (dcli_flag == trunk_type){ 				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8d%-8s%-6s\n",    \
				        	show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
				        	vlanid," - ",trans_value1," - "," - ");
			}
			else if (dcli_flag == vid_type){				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8d%-6s\n",     \
				       	show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
				       	vlanid," - "," - ",trans_value1," - ");	
			}			
			else if (dcli_flag == vidx_type){				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8s%-8d\n",     \
				      	 show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
				      	 vlanid," - "," - "," - ",trans_value1);				
			}
			else {
				vty_out (vty,"sorry interface type wrong !\n");
				return -1;;
			}
		
		
			dbus_message_iter_next(&iter_array);
		
		}
	
		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;
}	

/**************************************
*display valid fdb items which belong to the vlan on hw . 
*
* Usage:show fdb port <1-4094>
*
*input Params:
* 	 <1-4094>:  valid vlan index  range, before done the vlan index which has been configed
*
*output params:
*	fdb information ,include mac address, vlan index, destination interface.
*     
****************************************/

DEFUN(show_fdb_vlan_cmd_func,
	show_fdb_vlan_cmd,
	"show fdb vlan <1-4094>" ,
	"Show system information\n"
	"FDB table content!\n"
	"Show match vlan content!\n"
	"Vlan id, range in <1-4094>!\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int dnumber = 0;
	unsigned int dcli_flag = 0;
	unsigned int ret=0;
	unsigned char  show_mac[6];
	unsigned short vlanid=0;
	unsigned int i=0;
	unsigned int trans_value1 = 0;
	unsigned int trans_value2 = 0;
	int op_ret = 0;
	unsigned int k=0;
	op_ret =  parse_short_parse((char*)argv[0],&vlanid);
	if (NPD_FAIL == op_ret) {
		vty_out(vty,"%% Error,FDB vlan outrang!\n");
		return CMD_SUCCESS;
	}
	 if (vlanid<MIN_VLANID||vlanid>MAX_VLANID){
		vty_out(vty,"%% Error,FDB vlan outrang!\n");
		return CMD_SUCCESS;
		}

	int local_slot_id = 0;
    int slotNum = 0;
	
	if(is_distributed == DISTRIBUTED_SYSTEM)	
    {
	    #if 1
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		for(k=1; k <= slotNum; k++)
		{
          query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
									NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_VLAN);

            dbus_error_init(&err);
			dbus_message_append_args(	query,
							 	DBUS_TYPE_UINT16,&vlanid,
							 	DBUS_TYPE_INVALID);
            if(NULL == dbus_connection_dcli[k]->dcli_dbus_connection) 				
			{
    			if(k == local_slot_id)
				{
                    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
				   	vty_out(vty,"Can not connect to slot:%d \n",k);	
					continue;
				}
            }
			else
			{
                reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[k]->dcli_dbus_connection,query,-1, &err);				
			}
			
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
			dbus_message_iter_get_basic(&iter,&ret);
			if(ret==FDB_RETURN_CODE_NODE_VLAN_NONEXIST){
					vty_out(vty,"%% ERROR,the vlanid is not register !\n");
					return CMD_SUCCESS;
			}
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&dnumber);
			if (dnumber == 0){
				vty_out(vty,"% THE ITEM IS NONE !\n");
				if(k == slotNum)
				{
					return CMD_SUCCESS;
				}
				else
				{
					continue;
				}
			}
			
			vty_out(vty,"Codes:  CPU - target port which connect to cpu via pci interface\n");
			vty_out(vty,"		 HSC - high speed channel direct to cpu rgmii interface\n\n");
			/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info*/
			vty_out(vty,"%-17s	%-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
			vty_out(vty,"%-17s	%-8s%-11s%-8s%-8s%-6s\n","MAC","VLAN","SLOT/PORT","TRUNK","VID","VIDX");
			vty_out(vty,"%-17s	%-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
			dbus_message_iter_next(&iter);
			
			dbus_message_iter_recurse(&iter,&iter_array);
			for ( i=0; dnumber > i; i++){
				DBusMessageIter iter_struct;
				
				dbus_message_iter_recurse(&iter_array,&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&dcli_flag);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&vlanid);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&trans_value1);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&trans_value2);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&show_mac[0]);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&show_mac[1]);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&show_mac[2]);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&show_mac[3]);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&show_mac[4]);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&show_mac[5]);
				if (dcli_flag == port_type){
					if(CPU_PORT_VIRTUAL_SLOT == trans_value1) { /*static FDB to CPU or SPI (has the same virtual slot number)*/
						vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%-11s%-8s%-8s%-6s\n",	\
									show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
									vlanid,(CPU_PORT_VIRTUAL_PORT == trans_value2) ? "CPU": \
									(SPI_PORT_VIRTUAL_PORT == trans_value2) ? "HSC":"ERR"," - "," - "," - ");
					}
					else {
						vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%5d/%-6d%-8s%-8s%-6s\n",   \
									show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
									vlanid,trans_value1,trans_value2," - "," - "," - ");
					}
				}
				else if (dcli_flag == trunk_type){				
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%-11s%-8d%-8s%-6s\n",	 \
								show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
								vlanid," - ",trans_value1," - "," - ");
				}
				else if (dcli_flag == vid_type){				
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%-11s%-8s%-8d%-6s\n",	  \
							   show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5], 	\
							   vlanid," - "," - ",trans_value1," - ");	
				}			
				else if (dcli_flag == vidx_type){				
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%-11s%-8s%-8s%-8d\n",	  \
							   show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5], 	\
							   vlanid," - "," - "," - ",trans_value1);				
				}
				else {
					vty_out (vty,"sorry interface type wrong !\n");
					return -1;;
				}
				
				
				dbus_message_iter_next(&iter_array);
				
					}
			
			
			dbus_message_unref(reply);



		}
        /*free(vlanName);*/
        return CMD_SUCCESS;		
		#endif
    }
	else
	{
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
										NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_VLAN);
	
		dbus_error_init(&err);

		dbus_message_append_args(	query,
							 		DBUS_TYPE_UINT16,&vlanid,
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
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		if(ret==FDB_RETURN_CODE_NODE_VLAN_NONEXIST){
				vty_out(vty,"%% ERROR,the vlanid is not register !\n");
				return CMD_SUCCESS;
		}
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&dnumber);
		if (dnumber == 0){
			vty_out(vty,"% THE ITEM IS NONE !\n");
			return CMD_SUCCESS;
		}

		vty_out(vty,"Codes:  CPU - target port which connect to cpu via pci interface\n");
		vty_out(vty,"        HSC - high speed channel direct to cpu rgmii interface\n\n");
		/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info*/
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","MAC","VLAN","SLOT/PORT","TRUNK","VID","VIDX");
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
		dbus_message_iter_next(&iter);
	
		dbus_message_iter_recurse(&iter,&iter_array);
		for ( ; dnumber > i; i++){
			DBusMessageIter iter_struct;
		
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dcli_flag);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&vlanid);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trans_value1);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trans_value2);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[5]);
			if (dcli_flag == port_type){
				if(CPU_PORT_VIRTUAL_SLOT == trans_value1) { /*static FDB to CPU or SPI (has the same virtual slot number)*/
			   	 vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8s%-6s\n",	\
							show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
							vlanid,(CPU_PORT_VIRTUAL_PORT == trans_value2) ? "CPU":	\
							(SPI_PORT_VIRTUAL_PORT == trans_value2) ? "HSC":"ERR"," - "," - "," - ");
				}
				else {
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%5d/%-6d%-8s%-8s%-6s\n",   \
							show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
							vlanid,trans_value1,trans_value2," - "," - "," - ");
				}
			}
			else if (dcli_flag == trunk_type){ 				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8d%-8s%-6s\n",    \
				        show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
				        vlanid," - ",trans_value1," - "," - ");
			}
			else if (dcli_flag == vid_type){				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8d%-6s\n",     \
				       show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
				       vlanid," - "," - ",trans_value1," - ");	
			}			
			else if (dcli_flag == vidx_type){				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8s%-8d\n",     \
				       show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
				       vlanid," - "," - "," - ",trans_value1);				
			}
			else {
				vty_out (vty,"sorry interface type wrong !\n");
				return -1;;
			}
		
		
			dbus_message_iter_next(&iter_array);
		
		}
	
	
		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;
}	

/**************************************
*display valid fdb items which belong to the vlan on hw . 
*
* Usage:show fdb port VLANNAME
*
*input Params:
* 	 VLANNAME: before done the vlan name which has been configed ,
*                           VLANNAME must be to begin with letter or '_'
*
*output params:
*	fdb information ,include mac address, vlan index, destination interface.
*     
****************************************/

DEFUN(show_fdb_vlan_name_cmd_func,
	show_fdb_vlan_name_cmd,
	"show fdb vlan VLANNAME" ,
	"Show system information\n"
	"FDB table content!\n"
	"Show match vlan content!\n"
	"Vlan name!\n"
)

{
	DBusMessage *query, *reply;
	DBusError err;
	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int dnumber = 0;
	unsigned int dcli_flag = 0;
	unsigned short vlanid=0;
	unsigned char  show_mac[6];
	char* vlanname = NULL;
	unsigned int i=0;
	unsigned int trans_value1 = 0;
	unsigned int trans_value2 = 0;
	int op_ret=0;
	unsigned int k=0;
	op_ret =parse_vlan_string((char*)argv[0]);
	
	if (NPD_FAIL == op_ret) {
	   vty_out(vty,"%% Error,vlanname form erro!\n");
	   return CMD_SUCCESS;
	}

	vlanname= (char *)argv[0];

	int local_slot_id = 0;
    int slotNum = 0;
	
	if(is_distributed == DISTRIBUTED_SYSTEM)	
    {
	    #if 1
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		for(k=1; k <= slotNum; k++)
		{
         query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
									NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_VLAN_WITH_NAME);
	
		dbus_error_init(&err);

		dbus_message_append_args(	query,
							 		DBUS_TYPE_STRING,&vlanname,
							 		DBUS_TYPE_INVALID);
            if(NULL == dbus_connection_dcli[k]->dcli_dbus_connection) 				
			{
    			if(k == local_slot_id)
				{
                    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
				   	vty_out(vty,"Can not connect to slot:%d \n",k);	
					continue;
				}
            }
			else
			{
                reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[k]->dcli_dbus_connection,query,-1, &err);				
			}
			
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
			dbus_message_iter_get_basic(&iter,&op_ret);
			if (FDB_RETURN_CODE_NODE_VLAN_NONEXIST == op_ret ) {
					vty_out(vty,"%% Error,the vlanname is not register!\n");
					return CMD_SUCCESS;
				}
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&dnumber);
			if (0 == dnumber){
				vty_out(vty,"% THE ITEM IS NONE !\n");
				if(k == slotNum)
				{
					return CMD_SUCCESS;
				}
				else
				{
					continue;
				}
				}
			vty_out(vty,"THE ITEM IS %d\n",dnumber);
			
			vty_out(vty,"Codes:  CPU - target port which connect to cpu via pci interface\n");
			vty_out(vty,"		 HSC - high speed channel direct to cpu rgmii interface\n\n");
			/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info*/
			vty_out(vty,"%-17s	%-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
			vty_out(vty,"%-17s	%-8s%-11s%-8s%-8s%-6s\n","MAC","VLAN","SLOT/PORT","TRUNK","VID","VIDX");
			vty_out(vty,"%-17s	%-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
			dbus_message_iter_next(&iter);
			
			dbus_message_iter_recurse(&iter,&iter_array);
			for ( i=0; dnumber > i; i++){
				DBusMessageIter iter_struct;
				
				dbus_message_iter_recurse(&iter_array,&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&dcli_flag);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&vlanid);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&trans_value1);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&trans_value2);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&show_mac[0]);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&show_mac[1]);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&show_mac[2]);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&show_mac[3]);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&show_mac[4]);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&show_mac[5]);
				if (dcli_flag == port_type){
					if(CPU_PORT_VIRTUAL_SLOT == trans_value1) { /*static FDB to CPU or SPI (has the same virtual slot number)*/
						vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%-11s%-8s%-8s%-6s\n",	\
									show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
									vlanid,(CPU_PORT_VIRTUAL_PORT == trans_value2) ? "CPU": \
									(SPI_PORT_VIRTUAL_PORT == trans_value2) ? "HSC":"ERR"," - "," - "," - ");
					}
					else {
						vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%5d/%-6d%-8s%-8s%-6s\n",   \
									show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
									vlanid,trans_value1,trans_value2," - "," - "," - ");
					}
				}
				else if (dcli_flag == trunk_type){				
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%-11s%-8d%-8s%-6s\n",	 \
								show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
								vlanid," - ",trans_value1," - "," - ");
				}
				else if (dcli_flag == vid_type){				
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%-11s%-8s%-8d%-6s\n",	  \
							   show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5], 	\
							   vlanid," - "," - ",trans_value1," - ");	
				}			
				else if (dcli_flag == vidx_type){				
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%-11s%-8s%-8s%-8d\n",	  \
							   show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5], 	\
							   vlanid," - "," - "," - ",trans_value1);				
				}
				else {
					vty_out (vty,"sorry interface type wrong !\n");
					return -1;;
				}
				
				
				dbus_message_iter_next(&iter_array);
				
					}
			
			dbus_message_unref(reply);




		}
        /*free(vlanName);*/
        return CMD_SUCCESS;		
		#endif
    }
	else
	{
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
										NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_VLAN_WITH_NAME);
	
		dbus_error_init(&err);

		dbus_message_append_args(	query,
							 		DBUS_TYPE_STRING,&vlanname,
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
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&op_ret);
		if (FDB_RETURN_CODE_NODE_VLAN_NONEXIST == op_ret ) {
				vty_out(vty,"%% Error,the vlanname is not register!\n");
				return CMD_SUCCESS;
		}
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&dnumber);
		if (0 == dnumber){
			vty_out(vty,"% THE ITEM IS NONE !\n");
			return CMD_SUCCESS;
		}
		vty_out(vty,"THE ITEM IS %d\n",dnumber);

		vty_out(vty,"Codes:  CPU - target port which connect to cpu via pci interface\n");
		vty_out(vty,"        HSC - high speed channel direct to cpu rgmii interface\n\n");
	/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info*/
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","MAC","VLAN","SLOT/PORT","TRUNK","VID","VIDX");
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
		dbus_message_iter_next(&iter);
	
		dbus_message_iter_recurse(&iter,&iter_array);
		for ( ; dnumber > i; i++){
			DBusMessageIter iter_struct;
		
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dcli_flag);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&vlanid);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trans_value1);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trans_value2);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[5]);
			if (dcli_flag == port_type){
				if(CPU_PORT_VIRTUAL_SLOT == trans_value1) { /*static FDB to CPU or SPI (has the same virtual slot number)*/
			    	vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8s%-6s\n",	\
								show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
								vlanid,(CPU_PORT_VIRTUAL_PORT == trans_value2) ? "CPU":	\
								(SPI_PORT_VIRTUAL_PORT == trans_value2) ? "HSC":"ERR"," - "," - "," - ");
				}
				else {
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%5d/%-6d%-8s%-8s%-6s\n",   \
								show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
								vlanid,trans_value1,trans_value2," - "," - "," - ");
				}
			}
			else if (dcli_flag == trunk_type){ 				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8d%-8s%-6s\n",    \
				        	show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
				       	 vlanid," - ",trans_value1," - "," - ");
			}
			else if (dcli_flag == vid_type){				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8d%-6s\n",     \
				       	show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
				      	 vlanid," - "," - ",trans_value1," - ");	
			}			
			else if (dcli_flag == vidx_type){				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8s%-8d\n",     \
				      	 show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
				       	vlanid," - "," - "," - ",trans_value1);				
			}
			else {
				vty_out (vty,"sorry interface type wrong !\n");
				return -1;;
			}
		
		
			dbus_message_iter_next(&iter_array);
		
		}
	
		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;
}

/**************************************
*display valid fdb items which belong to the mac on hw . 
*
* Usage:show fdb port MAC
*
*input Params:
* 	 MAC:  valid mac address , form 00:00:11:11:aa:aa
*
*output params:
*	fdb information ,include mac address, vlan index, destination interface.
*     
****************************************/

DEFUN(show_fdb_mac_cmd_func,
	show_fdb_mac_cmd,
	"show fdb mac MAC " ,
	"Show system information\n"
	"FDB table content!\n"
	"Config mac address!\n"
	"Mac address in Hex format\n"	
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int dnumber = 0;
	unsigned int dcli_flag = 0;
	ETHERADDR		macAddr;
	unsigned char  show_mac[6];
	unsigned short vlanid = 0;
	unsigned int i=0;
	unsigned int devNum = 0;
	unsigned int portNum = 0;
	unsigned int trans_value1 = 0;
	unsigned int trans_value2 = 0;
	int op_ret = 0;
	unsigned int k=0;
	memset(&macAddr,0,sizeof(ETHERADDR));
	
	op_ret = parse_mac_addr((char *)argv[0],&macAddr);
	if (NPD_FAIL == op_ret) {
		vty_out(vty,"%% Error,Unknow mac addr format!\n");
		return CMD_SUCCESS;
	}

	int local_slot_id = 0;
    int slotNum = 0;
	
	if(is_distributed == DISTRIBUTED_SYSTEM)	
    {
	    #if 1
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		for(k=1; k <= slotNum; k++)
		{
         	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
										NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_MAC);
		

			dbus_message_append_args(	query,
							 			DBUS_TYPE_BYTE,&macAddr.arEther[0],
							 			DBUS_TYPE_BYTE,&macAddr.arEther[1],
							 			DBUS_TYPE_BYTE,&macAddr.arEther[2],
							 			DBUS_TYPE_BYTE,&macAddr.arEther[3],
							 			DBUS_TYPE_BYTE,&macAddr.arEther[4],
							 			DBUS_TYPE_BYTE,&macAddr.arEther[5],
							 			DBUS_TYPE_INVALID);
	
			dbus_error_init(&err);
            if(NULL == dbus_connection_dcli[k]->dcli_dbus_connection) 				
			{
    			if(k == local_slot_id)
				{
                    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
				   	vty_out(vty,"Can not connect to slot:%d \n",k);	
					continue;
				}
            }
			else
			{
                reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[k]->dcli_dbus_connection,query,-1, &err);				
			}
			
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
			dbus_message_iter_get_basic(&iter,&dnumber);
			if (dnumber == 0){
				vty_out(vty,"% THE ITEM IS NONE !\n");
				if(k == slotNum)
				{
					return CMD_SUCCESS;
				}
				else
				{
					continue;
				}
				}
			
			vty_out(vty,"Codes:  CPU - target port which connect to cpu via pci interface\n");
			vty_out(vty,"		 HSC - high speed channel direct to cpu rgmii interface\n\n");
			vty_out(vty,"%-17s	%-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
			vty_out(vty,"%-17s	%-8s%-11s%-8s%-8s%-6s\n","MAC","VLAN","SLOT/PORT","TRUNK","VID","VIDX");
			vty_out(vty,"%-17s	%-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
			dbus_message_iter_next(&iter);
			
			dbus_message_iter_recurse(&iter,&iter_array);
			for ( i=0; dnumber > i; i++){
				DBusMessageIter iter_struct;
				
				dbus_message_iter_recurse(&iter_array,&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&dcli_flag);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&vlanid);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&trans_value1);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&trans_value2);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&show_mac[0]);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&show_mac[1]);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&show_mac[2]);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&show_mac[3]);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&show_mac[4]);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&show_mac[5]);
				if (dcli_flag == port_type){
					if(CPU_PORT_VIRTUAL_SLOT == trans_value1) { /*static FDB to CPU or SPI (has the same virtual slot number)*/
						vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%8s%7s%8s%8s\n", \
									show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
									vlanid,(CPU_PORT_VIRTUAL_PORT == trans_value2) ? "CPU": \
									(SPI_PORT_VIRTUAL_PORT == trans_value2) ? "HSC":"ERR"," - "," - "," - ");
					}
					else {
						vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%5d/%-6d%-8s%-8s%-6s%-6s\n",   \
									show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
									vlanid,trans_value1,trans_value2," - "," - "," - ");
					}
				}
				else if (dcli_flag == trunk_type){				
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%8s%6d%9s%8s\n",	\
								show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
								vlanid," - ",trans_value1," - "," - ");
				}
				else if (dcli_flag == vid_type){				
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%-11s%-8s%-8d%-6s\n",	  \
							   show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5], 	\
							   vlanid," - "," - ",trans_value1," - ");	
				}			
				else if (dcli_flag == vidx_type){				
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%-11s%-8s%-8s%-8d\n",	  \
							   show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5], 	\
							   vlanid," - "," - "," - ",trans_value1);				
				}
				else {
					vty_out (vty,"sorry interface type wrong !\n");
					return -1;;
				}
			
				
				dbus_message_iter_next(&iter_array);
				
					}
			
			
			dbus_message_unref(reply);





		}
        /*free(vlanName);*/
        return CMD_SUCCESS;		
		#endif
    }
	else
	{

		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
										NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_MAC);
		

		dbus_message_append_args(	query,
							 		DBUS_TYPE_BYTE,&macAddr.arEther[0],
							 		DBUS_TYPE_BYTE,&macAddr.arEther[1],
							 		DBUS_TYPE_BYTE,&macAddr.arEther[2],
							 		DBUS_TYPE_BYTE,&macAddr.arEther[3],
							 		DBUS_TYPE_BYTE,&macAddr.arEther[4],
							 		DBUS_TYPE_BYTE,&macAddr.arEther[5],
							 		DBUS_TYPE_INVALID);
	
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
		dbus_message_iter_get_basic(&iter,&dnumber);
		if (dnumber == 0){
			vty_out(vty,"% THE ITEM IS NONE !\n");
			return CMD_SUCCESS;
		}

		vty_out(vty,"Codes:  CPU - target port which connect to cpu via pci interface\n");
		vty_out(vty,"        HSC - high speed channel direct to cpu rgmii interface\n\n");
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","MAC","VLAN","SLOT/PORT","TRUNK","VID","VIDX");
		vty_out(vty,"%-17s  %-8s%-11s%-8s%-8s%-6s\n","=================",	"======", "=========","======","======","======");
		dbus_message_iter_next(&iter);
	
		dbus_message_iter_recurse(&iter,&iter_array);
		for ( ; dnumber > i; i++){
			DBusMessageIter iter_struct;
		
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dcli_flag);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&vlanid);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trans_value1);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trans_value2);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[5]);
			if (dcli_flag == port_type){
				if(CPU_PORT_VIRTUAL_SLOT == trans_value1) { /*static FDB to CPU or SPI (has the same virtual slot number)*/
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%8s%7s%8s%8s\n", \
								show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
								vlanid,(CPU_PORT_VIRTUAL_PORT == trans_value2) ? "CPU":	\
								(SPI_PORT_VIRTUAL_PORT == trans_value2) ? "HSC":"ERR"," - "," - "," - ");
				}
				else {
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%5d/%-6d%-8s%-8s%-6s%-6s\n",   \
								show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
								vlanid,trans_value1,trans_value2," - "," - "," - ");
				}
			}
			else if (dcli_flag == trunk_type){				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x	%-8d%8s%6d%9s%8s\n",	\
							show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
							vlanid," - ",trans_value1," - "," - ");
			}
			else if (dcli_flag == vid_type){				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8d%-6s\n",     \
				     	  show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
				     	  vlanid," - "," - ",trans_value1," - ");	
			}			
			else if (dcli_flag == vidx_type){				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8s%-8d\n",     \
				      	 show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
				      	 vlanid," - "," - "," - ",trans_value1);				
			}
			else {
				vty_out (vty,"sorry interface type wrong !\n");
				return -1;;
			}

		
			dbus_message_iter_next(&iter_array);
		
		}

	
		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;
}


/**************************************
*config static fdb item belong to system during initialization on hw . 
*
* Usage:config system mac vlan <1-4094>
*
*input Params:
* 	<1-4094>:  valid vlan index  range, before done the vlan index which has been configed
*
*output params:
*	fdb information ,include mac address, vlan index, destination interface.
*     
****************************************/

DEFUN(config_fdb_system_mac_cmd_func,
	config_fdb_system_mac_cmd,
	"config system mac vlan <1-4094>",
	"Config system information!\n"
	"Config FDB table!\n"
	"Config system dut mac!\n"
	"Config vlan content!\n"
	"FDB table vlan value <1-4094>\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short vlanid = 0;
	unsigned int 	op_ret = 0;
	int i;
	int local_slot_id = 0;
    int slotNum = 0;
	op_ret =  parse_short_parse((char*)argv[0],&vlanid);
	if (NPD_FAIL == op_ret) {
		vty_out(vty,"%% Error,FDB vlan outrang!\n");
		return CMD_SUCCESS;
	}
	if (vlanid<MIN_VLANID||vlanid>MAX_VLANID){
		vty_out(vty,"%% Error,FDB vlan outrang!\n");
		return CMD_SUCCESS;
	}
	if(is_distributed == DISTRIBUTED_SYSTEM)	
	{
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		for(i=1; i <= slotNum; i++)
		{
			query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
											NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_SYSTEM_FDB);
			
			dbus_error_init(&err);
		    dbus_message_append_args(	query,
									 	DBUS_TYPE_UINT16,&vlanid,
									 	DBUS_TYPE_INVALID);
			if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
			{
				if(i == local_slot_id)
				{
               		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
			   		vty_out(vty,"Can not connect to slot:%d \n",i);	
					continue;
				}
        	}
			else
			{
            	reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[i]->dcli_dbus_connection,query,-1, &err);				
			}
			
			dbus_message_unref(query);
			if (NULL == reply) {
				vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",i);
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_SUCCESS;
			}

			if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&op_ret,
				DBUS_TYPE_INVALID)) {
				/*dcli_fdb_output(op_ret);*/
				if( FDB_RETURN_CODE_NODE_VLAN_NONEXIST ==op_ret){
					vty_out(vty,"%% Error,the vlan does not exists!\n");
				}
				else if( FDB_RETURN_CODE_BADPARA== op_ret){
					vty_out(vty,"%% Error,the vlan ID input is not right.\n");
				}
				else if( FDB_RETURN_CODE_GENERAL == op_ret){
					vty_out(vty,"%% Error,Opration error when configuration.\n");
				}
				else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
					vty_out(vty,"%% Product not support this function!\n");
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
		}
	}
	else
	{
		
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH, \
										NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_SYSTEM_FDB);
		
		dbus_error_init(&err);
		dbus_message_append_args(	query,
									DBUS_TYPE_UINT16,&vlanid,
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
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID)) {
			/*dcli_fdb_output(op_ret);*/
			if( FDB_RETURN_CODE_NODE_VLAN_NONEXIST ==op_ret){
				vty_out(vty,"%% Error,the vlan does not exists!\n");
			}
			else if( FDB_RETURN_CODE_BADPARA== op_ret){
				vty_out(vty,"%% Error,the vlan ID input is not right.\n");
			}
			else if( FDB_RETURN_CODE_GENERAL == op_ret){
				vty_out(vty,"%% Error,Opration error when configuration.\n");
			}
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
				vty_out(vty,"%% Product not support this function!\n");
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
	}
	return CMD_SUCCESS;
}


/**************************************
config fdb item with porton Both Sw & Hw. 
*
* Usage:config fdb number Number on the port Port
*
*input Params: 
*	Port:       The port wanted to be stricted for fdb count
*	Number:  The fdb count wanted to be learned
*   
*
*output params:none
*     
****************************************/

DEFUN(config_fdb_port_number_cmd_func,
	config_fdb_port_number_cmd,
	"config fdb port PORTNO (<1-16384>|0)",
	"Config system information\n"
	"Config FDB table\n"
	"Config port-based \n"
	CONFIG_ETHPORT_STR
	"Config the number of the fdb entry 1-16384\n"
	"Cancel the configuration\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int 	ret = 0;
	unsigned char slotNum = 0;
	unsigned char portNum = 0;
	unsigned int number = 0;
	unsigned int fdblimit = 0;


	/* parse the slot/port number*/
	ret =  parse_slotport_no((char*)argv[0],&slotNum,&portNum);
	if (NPD_FAIL == ret) {
		vty_out(vty,"%% Error,Unknow portno format!\n");
		return CMD_SUCCESS;
	}
	/*
	if((slotNum <MIN_SLOT ||slotNum>MAX_SLOT)||(portNum <MIN_PORT||portNum>MAX_PORT)){
		vty_out(vty,"err port outrange!\n");
		return CMD_SUCCESS;
		}
	*/
	
    /*parse the number*/

	ret = parse_int_parse((char *)argv[1],&number);
	if(NPD_FAIL == ret){
		vty_out(vty,"%% Error,Unknow number format!\n");
		return CMD_SUCCESS;
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NUMBER_WITH_PORT);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_BYTE,&slotNum,
							 	DBUS_TYPE_BYTE,&portNum,
							 	DBUS_TYPE_UINT32,&number,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	else{
			if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&ret,
				DBUS_TYPE_UINT32,&fdblimit,
				DBUS_TYPE_INVALID)) {
				/*dcli_fdb_output(ret);*/
				if(FDB_RETURN_CODE_BADPARA == ret){
					vty_out(vty,"%% Error,bad parameters input!\n");
				}
				else if(FDB_RETURN_CODE_GENERAL  == ret){
                    vty_out(vty,"%% Error occured in configuration\n");
				}
				else if(FDB_RETURN_CODE_NODE_PORT_NOTIN_VLAN == ret){
					vty_out(vty,"%% Error occured for port not in vlan or is one member of trunk\n");
				}					
				else if(FDB_RETURN_CODE_OCCUR_HW == ret){
					vty_out(vty,"%% Error occured in hw when configuration\n");
				}
				else if(NPD_FDB_ERR_HW_NOT_SUPPORT == ret){
					vty_out(vty,"%% Error occured for hardware not support\n");
				}				
				else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
					vty_out(vty,"%% Product not support this function!\n");
				}
				else{
				    vty_out(vty,"The number has been set is: %d\n",fdblimit);
				}
			} 
			else {
				vty_out(vty,"Failed get args.\n");
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
			}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}



/**************************************
config fdb item with vlan Both Sw & Hw. 
*
* Usage:config fdb number Number on the port Port
*
*input Params: 
*	vlan:       The port wanted to be stricted for fdb count
*	Number:  The fdb count wanted to be learned
*   
*
*output params:none
*     
****************************************/

DEFUN(config_fdb_vlan_number_cmd_func,
	config_fdb_vlan_number_cmd,
	"config fdb vlan <1-4094> (<1-16384>|0)",
	"Config system information\n"
	"Config FDB table\n"
	"Config vlan-based \n"
	"Config the vlan number \n"
	"Config the number of the FDB entry 1-16384\n"
	"Cancel the configuration\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int 	ret = 0;

	unsigned short vlanid = 0;
	unsigned int number = 0;


	/* parse the vlan number*/
	ret = parse_vlan_no((char*)argv[0],&vlanid);
	
	if (NPD_FAIL == ret) {
    	vty_out(vty,"%% Error,Illegal vlanId .\n");
		return CMD_SUCCESS;
	}
	if (4095 == vlanid){
		vty_out(vty,"% Bad parameter,Reserved vlan for Layer3 interface of EthPort!\n");
		return CMD_WARNING;
	}
	
    /*parse the number*/

	ret = parse_int_parse((char *)argv[1],&number);
	if(NPD_FAIL == ret){
		vty_out(vty,"%% Error,Unknow number format!\n");
		return CMD_SUCCESS;
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NUMBER_WITH_VLAN);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT16,&vlanid,
							 	DBUS_TYPE_UINT32,&number,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	else{
			if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&ret,
				DBUS_TYPE_UINT32,&number,
				DBUS_TYPE_INVALID)) {
				/*dcli_fdb_output(ret);*/
				if( -1 == ret){
				     vty_out(vty,"%% Error,Error in the process\n");
				}
				else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == ret){
					 vty_out(vty,"%% Error,The vlan %d does not exist\n",vlanid);
				}
				else if(NPD_FDB_ERR_VLAN_NO_PORT == ret){
					vty_out(vty,"%% Error,There is no port number in vlan %d\n",vlanid);
				}
				else if(FDB_RETURN_CODE_GENERAL == ret){
                    vty_out(vty,"%% Error occured in configuration\n");
				}
				else if(FDB_RETURN_CODE_OCCUR_HW == ret){
					vty_out(vty,"%% Error occured in hw when configuration\n");
				}
				else if(NPD_FDB_ERR_HW_NOT_SUPPORT == ret){
					vty_out(vty,"%% Error occured for hardware not support\n");
				}					
				else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
					vty_out(vty,"%% Product not support this function!\n");
				}
			    else{
					 vty_out(vty,"The number set is %d\n",number);
				}
			}
			 else {
				vty_out(vty,"Failed get args.\n");
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
			}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(config_fdb_vlan_port_number_cmd_func,
	config_fdb_vlan_port_number_cmd,
	"config fdb vlan <1-4094> port PORTNO (<1-16384>|0)",
	"Config system information\n"
	"Config FDB table\n"
	"Config vlan-based \n"
	"Config the vlan id\n"
	"Config port-based \n"
	CONFIG_ETHPORT_STR
	"Config the number of the fdb entry 1-16384 \n"
	"Cancel the configuration\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int 	ret = 0;

	unsigned short vlanid = 0;
	unsigned int number = 0;
	unsigned char slotNum = 0;
	unsigned char portNum = 0;


	/* parse the vlan number*/
	ret = parse_vlan_no((char*)argv[0],&vlanid);
	
	if (NPD_FAIL == ret) {
    	vty_out(vty,"Illegal vlanId .\n");
		return CMD_SUCCESS;
	}
	if (4095 == vlanid){
		vty_out(vty,"% Bad parameter,Reserved vlan for Layer3 interface of EthPort!\n");
		return CMD_WARNING;
	}

	/*parse the slot/port number*/
	ret =  parse_slotport_no((char*)argv[1],&slotNum,&portNum);
	if (NPD_FAIL == ret) {
		vty_out(vty,"Unknow portno format!\n");
		return CMD_SUCCESS;
	}
	/*
	if((slotNum <MIN_SLOT ||slotNum>MAX_SLOT)||(portNum <MIN_PORT||portNum>MAX_PORT)){
		vty_out(vty,"err port outrange!\n");
		return CMD_SUCCESS;
		}
	*/
    /*parse the number*/

	ret = parse_int_parse((char *)argv[2],&number);
	if(NPD_FAIL == ret){
		vty_out(vty,"Unknow number format!\n");
		return CMD_SUCCESS;
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_FDB_NUMBER_WITH_VLAN_PORT);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
								DBUS_TYPE_UINT16,&vlanid,
								DBUS_TYPE_BYTE,&slotNum,
							 	DBUS_TYPE_BYTE,&portNum,
							 	DBUS_TYPE_UINT32,&number,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	else{
			if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&ret,
				DBUS_TYPE_INVALID)) {
				/*dcli_fdb_output(ret);*/
				if( -1 == ret){
				     vty_out(vty,"%% Error in the process\n");
				}
				else if(FDB_RETURN_CODE_BADPARA == ret){
					vty_out(vty,"%% Error,Bad parameters input!\n");
				}
				else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == ret){
					 vty_out(vty,"%% Error,The vlan %d does not exist\n",vlanid);
				}
				else if(FDB_RETURN_CODE_NODE_PORT_NOTIN_VLAN == ret){
					vty_out(vty,"%% Error,The port number is not in vlan %d\n",vlanid);
				}
				else if(FDB_RETURN_CODE_GENERAL == ret){
                    vty_out(vty,"%% Error occured in configuration\n");
				}
				else if(FDB_RETURN_CODE_OCCUR_HW == ret){
					vty_out(vty,"%% Error occured in hw when configuration\n");
				}
				else if(NPD_FDB_ERR_HW_NOT_SUPPORT == ret){
					vty_out(vty,"%% Error occured for hardware not support\n");
				}					
				else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
					vty_out(vty,"%% Product not support this function!\n");
				}
			    else{
					 vty_out(vty,"The number set is %d\n",number);
				}
			} 
			else {
				vty_out(vty,"Failed get args.\n");
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
			}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


/**************************************
*show fdb item with port on Both Sw & Hw. 
*
* Usage:show fdb number Number on the port Port
*
*input Params: 
*	Port:       The port wanted to be stricted for fdb count
*	Number:  The fdb count wanted to be learned
*   
*
*output params:none
*     
****************************************/




DEFUN(show_fdb_number_limit_cmd_func,
	show_fdb_number_limit_cmd,
	"show fdb limit",
	"Show system information\n"
	"FDB table\n"
	"The number of the FDB entry \n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	
	unsigned int 	ret = 0;
	unsigned char slotNum = 0;
	unsigned char portNum = 0;
	unsigned short vlanId = 0;
	unsigned int number = 0;
	unsigned int fdbnumber = 0;
	unsigned int i = 0;
	unsigned int k=0;

	int local_slot_id = 0;
    int slotnum = 0;
	
	if(is_distributed == DISTRIBUTED_SYSTEM)	
    {
	    #if 1
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotnum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotnum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		for(k=1; k <= slotnum; k++)
		{
       		
			query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_NUMBER_LIMIT_ITEM);
				
				dbus_error_init(&err);
       		
            if(NULL == dbus_connection_dcli[k]->dcli_dbus_connection) 				
			{
    			if(k == local_slot_id)
				{
                    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
				   	vty_out(vty,"Can not connect to slot:%d \n",k);	
					continue;
				}
            }
			else
			{
                reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[k]->dcli_dbus_connection,query,-1, &err);				
			}
			
			dbus_message_unref(query);
			if (NULL == reply) {
				vty_out(vty,"failed get reply.\n");
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
			}
			dbus_message_iter_init(reply,&iter);
			dbus_message_iter_get_basic(&iter,&number);
			if (number == 0){
				vty_out(vty,"There is no fdb limit!\n");
				if(k == slotNum)
				{
					return CMD_SUCCESS;
				}
				else
				{
					continue;
				}
			}
			/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info*/
			vty_out(vty,"%-8s%-11s%-8s%-8s\n","======", "=========","======","======");
			vty_out(vty,"%-8s%-11s%-8s%-8s\n","VLAN","SLOT/PORT","TRUNK","number");
			vty_out(vty,"%-8s%-11s%-8s%-8s\n","======", "=========","======","======");
			
			dbus_message_iter_next(&iter);
			
			dbus_message_iter_recurse(&iter,&iter_array);
			for ( i=0; i < number; i++){
				DBusMessageIter iter_struct;
				
				dbus_message_iter_recurse(&iter_array,&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&vlanId);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&slotNum);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&portNum);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&fdbnumber);
			
				if(0 == vlanId){
				   vty_out(vty,"%-8s%5d%s%-8d%-8s%-8d\n",	
								"-",slotNum,"/",portNum," - ",fdbnumber);
				}
				else if((0 == slotNum)&&( 0 == portNum)){
					vty_out(vty,"%-8d%5s%s%-8s%-8s%-8d\n",	
								vlanId,"-","/","-"," - ",fdbnumber);
				}
				else{
					vty_out(vty,"%-8d%5d%s%-8d%-8s%-8d\n",	
								vlanId,slotNum,"/",portNum," - ",fdbnumber);
				}
				
				dbus_message_iter_next(&iter_array);
				
				}
			
			dbus_message_unref(reply);






		}
        /*free(vlanName);*/
        return CMD_SUCCESS;		
		#endif
    }
	else
	{
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_NUMBER_LIMIT_ITEM);
	
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&number);
	if (number == 0){
		vty_out(vty,"There is no fdb limit!\n");
		return CMD_SUCCESS;
	}
	/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info*/
	vty_out(vty,"%-8s%-11s%-8s%-8s\n","======", "=========","======","======");
	vty_out(vty,"%-8s%-11s%-8s%-8s\n","VLAN","SLOT/PORT","TRUNK","number");
	vty_out(vty,"%-8s%-11s%-8s%-8s\n","======", "=========","======","======");
	
	dbus_message_iter_next(&iter);
	
	dbus_message_iter_recurse(&iter,&iter_array);
	for ( ; i < number; i++){
		DBusMessageIter iter_struct;
		
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&vlanId);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&slotNum);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&portNum);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&fdbnumber);

        if(0 == vlanId){
		   vty_out(vty,"%-8s%5d%s%-8d%-8s%-8d\n",	
						"-",slotNum,"/",portNum," - ",fdbnumber);
        }
		else if((0 == slotNum)&&( 0 == portNum)){
            vty_out(vty,"%-8d%5s%s%-8s%-8s%-8d\n",	
						vlanId,"-","/","-"," - ",fdbnumber);
		}
		else{
			vty_out(vty,"%-8d%5d%s%-8d%-8s%-8d\n",	
						vlanId,slotNum,"/",portNum," - ",fdbnumber);
		}
		
		dbus_message_iter_next(&iter_array);
		
		}
	
	dbus_message_unref(reply);
	}

	return CMD_SUCCESS;	
}

#define SEM_SHOWRUN_CFG_SIZE	(3*1024) /* for all 24GE ports configuration */
int sfd_show_running_config(struct vty* vty) 
{	
	char *showStr = NULL,*cursor = NULL;
	int totalLen = 0;
	int i = 0;
	int length = 0;

	showStr = (char*)malloc(SEM_SHOWRUN_CFG_SIZE);

	if(NULL == showStr) {
		printf("memory malloc error\n");
		return CMD_FAILURE;
	}
	memset(showStr, 0, SEM_SHOWRUN_CFG_SIZE);
	cursor = showStr;
	
    if(sfd_flag){
	    length += sprintf(cursor,"service sfd on\n");
	    cursor = showStr+length;
    }
	
    if(sfd_debug_flag){
	    length += sprintf(cursor,"service sfd debug on\n");
	    cursor = showStr+length;
    }

	char _tmpstr[64];
	memset(_tmpstr,0,64);
	sprintf(_tmpstr,BUILDING_MOUDLE,"SFD");
	vtysh_add_show_string(_tmpstr);
	vtysh_add_show_string(showStr);

	return CMD_SUCCESS;
}



int dcli_fdb_show_running_config(struct vty* vty) {	
	char *showStr = NULL;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,		\
							NPD_DBUS_FDB_OBJPATH , \
							NPD_DBUS_FDB_INTERFACE ,		\
							NPD_DBUS_STATIC_FDB_METHOD_SHOW_RUNNING_CONFIG);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show fdb running config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) 
	{
	
		char _tmpstr[64];
		memset(_tmpstr,0,64);
		sprintf(_tmpstr,BUILDING_MOUDLE,"FDB");
		vtysh_add_show_string(_tmpstr);
		vtysh_add_show_string(showStr);
		sfd_show_running_config(vty);
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
	return 0;
	/*printf("%s",showStr);*/
}

/**************************************
*display valid fdb items  on hw . 
*
* Usage:show fdb 
*
*input Params: none
*
*output params:
*	fdb information ,include mac address, vlan index, destination interface.
*     
****************************************/

DEFUN(show_fdb_cmd_func_test,
	show_fdb_cmd_test,
	"show debug_fdb" ,
	"Show system information\n"
	"FDB table content!\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int dnumber = 0;
	unsigned int dcli_flag =0;
	unsigned int type_flag = 0;
	unsigned int dev = 0;
	unsigned char* type = NULL;
	
	unsigned char  show_mac[6] = {0};
	unsigned short vlanid=0;
	unsigned int i=0;
	unsigned int devNum = 0;
	unsigned int portNum = 0;
	unsigned int trans_value1 = 0;
	unsigned int trans_value2 = 0;
	unsigned int k=0;
	unsigned int asic_num = 0;

	int local_slot_id = 0;
    int slotNum = 0;
	
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
							NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_DEBUG);

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
	dbus_message_iter_get_basic(&iter,&dnumber);
	if (dnumber == 0){
		vty_out(vty,"%% FDB TABLE IS NONE!\n");
		return CMD_SUCCESS;
		}

	vty_out(vty,"Codes:  CPU - target port which connect to cpu via pci interface\n");
	vty_out(vty,"        HSC - high speed channel direct to cpu rgmii interface\n\n");
	/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info*/
	vty_out(vty,"%-17s  %-4s  %-4s  %-9s  %-5s  %-4s  %-4s  %-8s\n","=================",	"====","====", "=========","=====","====","====","========");
	vty_out(vty,"%-17s  %-4s  %-4s  %-9s  %-5s  %-4s  %-4s  %-8s\n","MAC","VLAN","ASIC","DEV/PORT","TRUNK","VID","VIDX","TYPE");
	vty_out(vty,"%-17s  %-4s  %-4s  %-9s  %-5s  %-4s  %-4s  %-8s\n","=================",	"====","====", "=========","=====","====","====","========");
	dbus_message_iter_next(&iter);

	dbus_message_iter_recurse(&iter,&iter_array);
	for ( ; dnumber > i; i++){
		DBusMessageIter iter_struct;
	
		dbus_message_iter_recurse(&iter_array,&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&dcli_flag);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&dev);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&asic_num);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&vlanid);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&trans_value1);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&trans_value2);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&type_flag);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&show_mac[0]);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&show_mac[1]);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&show_mac[2]);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&show_mac[3]);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&show_mac[4]);
		dbus_message_iter_next(&iter_struct);
		dbus_message_iter_get_basic(&iter_struct,&show_mac[5]);
	
		if( 0 ==type_flag){
   	    	type = "DYNAMIC";
		}
		else if(1 ==type_flag){
			type = "STATIC";
		}
		else {
			vty_out (vty,"sorry fdb item type wrong !\n");
			return -1;
		}
		if (dcli_flag == port_type){
			if(CPU_PORT_VIRTUAL_SLOT == trans_value1) { /*static FDB to CPU or SPI (has the same virtual slot number)*/
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-4d  %-4d  %-9s  %-5s  %4s  %4s  %-8s\n",	\
						show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
						vlanid,asic_num,(CPU_PORT_VIRTUAL_PORT == trans_value2) ? "   CPU   ":	\
						(SPI_PORT_VIRTUAL_PORT == trans_value2) ? "   HSC   ":"   ERR   ","  -  "," - "," - "," - ");
			}
			else {
				if(trans_value1 <= 10){
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-4d  %-4d  %4d/%-4d  %-5s  %-4s  %-4s  %-8s\n",   \
								show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
								vlanid,asic_num,dev,trans_value2," - "," - "," - ",type);
				}
						
			}
		}
		else if (dcli_flag == trunk_type){ 				
			vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-4d  %-4d  %9s  %-5d  %-4s  %-4s  %-8s\n",    \
			       	 show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
			       	 vlanid,asic_num,"    -    ",trans_value1," - "," - ",type);
		}
	
		else if (dcli_flag == vid_type){				
			vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8d%-6s%-6s\n",     \
			      	 show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
			      	 vlanid," - "," - ",trans_value1," - ",type);	
		}			
		else if (dcli_flag == vidx_type){				
			vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8s%-8d%-6s\n",     \
			     	  show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
			      	 vlanid," - "," - "," - ",trans_value1,type);				
		}
		else {
			vty_out (vty,"sorry interface type wrong !\n");
			return -1;
		}
	
		dbus_message_iter_next(&iter_array);
	
		}

	dbus_message_unref(reply);

	return CMD_SUCCESS;	
}

DEFUN(config_debug_fdb_mac_vlan_trunk_static_cmd_func,
	config_debug_fdb_mac_vlan_trunk_cmd,
	"create fdb static mac MAC vlan <1-4094> trunk <1-127>",
	"Config system information\n"
	"Config FDB table\n"
	"Static FDB item \n"
	"Config MAC field of FDB table.\n"
	"Config MAC address in Hex format\n"
	"Config FDB table vlan Id\n"
	"Config VLAN Id valid range <1-4094>\n"
	"Config trunk entry \n"
	"Trunk ID range <1-127>\n"
)
{
	DBusMessage *query, *reply;
	ETHERADDR		macAddr;
	unsigned short	vlanId = 0;
	unsigned short  trunkId = 0;
	DBusError err;
	unsigned int 	op_ret = 0;	
	
	memset(&macAddr,0,sizeof(ETHERADDR));
	op_ret = parse_mac_addr((char *)argv[0],&macAddr);
	if (NPD_FAIL == op_ret) 
	{
    	vty_out(vty,"% Bad Parameters,Unknow mac addr format!\n");
		return CMD_WARNING;
	}
   
	op_ret=is_muti_brc_mac(&macAddr);
	if(op_ret==1)
	{
		vty_out(vty,"%% Error:input broadcast or multicast mac!\n");
		return CMD_WARNING;
	}
		
	
	op_ret = parse_short_parse((char*)argv[1], &vlanId);
	if (NPD_FAIL == op_ret) 
	{
    	vty_out(vty,"% Bad Parameters,Unknow vlan id format.\n");
		return CMD_WARNING;
	}
    if ((vlanId<MIN_VLANID)||(vlanId>MAX_VLANID))
	{
		vty_out(vty,"%% Error,FDB vlan outrange!\n");
		return CMD_WARNING;
	}
	/*vty_out(vty,"fdb vlan id %d.\n",vlanId);*/

	
	op_ret = parse_short_parse((char*)argv[2], &trunkId);
	if (NPD_FAIL == op_ret) 
	{
    	vty_out(vty,"% Bad Parameters,Unknow trunk id format.\n");
		return CMD_WARNING;
	}
    if ((trunkId<1)||(trunkId>127))
	{
		vty_out(vty,"%% Error,Trunk outrange!\n");
		return CMD_WARNING;
	}
	
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
										NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_DEBUG_FDB_TRUNK_STATIC);
	
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT16,&vlanId,
					            DBUS_TYPE_UINT16,&trunkId,
							 	DBUS_TYPE_BYTE,&macAddr.arEther[0],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[1],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[2],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[3],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[4],
							 	DBUS_TYPE_BYTE,&macAddr.arEther[5],
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_INVALID)) 
	{
		if( FDB_RETURN_CODE_GENERAL == op_ret)
		{
	        vty_out(vty,"%% Error,operations error when configuration!\n");
	    }
		else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == op_ret)
		{
			vty_out(vty,"%% Error,the vlan input not exist!\n");
		}
		else if(FDB_RETURN_CODE_NODE_PORT_NOTIN_VLAN == op_ret)
		{
			vty_out(vty,"%% Error,the trunk is not in vlan!\n");
		}
		else if(FDB_RETURN_CODE_NODE_NOT_EXIST== op_ret)
		{
			vty_out(vty,"%% Error,the trunk does not exist!\n");
		}
		else if(FDB_RETURN_CODE_OCCUR_HW == op_ret)
		{
			vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
		}
		else if(FDB_RETURN_CODE_SYSTEM_MAC == op_ret)
		{
			vty_out(vty,"%% Error,the mac input conflict with system mac,please chang!\n");
		}
		else if(FDB_RETURN_CODE_NODE_PORT_NOTIN_VLAN == op_ret)
		{
			vty_out(vty,"%% Error,there is no port in trunk!\n");
		}
		else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
		{
			vty_out(vty,"%% Product not support this function!\n");
		}
	}
	else 
	{
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(config_debug_fdb_static_delete_vlan_cmd_func,
	config_debug_fdb_static_delete_vlan_cmd,
	"delete fdb static vlan <1-4094>",
	"Delete FDB table\n"
	"Config FDB table \n"
	"Static FDB table\n"
	"FDB table vlan Range <1-4094>\n"
	"Config FDB vlan value\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short vlanid = 0;
	unsigned int 	op_ret = 0;
	
	op_ret = parse_short_parse((char *)argv[0],&vlanid);
	if (NPD_FAIL == op_ret) 
	{
    	vty_out(vty,"% Bad Parameters,FDB agingtime form erro!\n");
		return CMD_SUCCESS;
	}
	if (vlanid <MIN_VLANID||vlanid>MAX_VLANID)
	{
		vty_out(vty,"% Bad Parameters,FDB vlan outrang!\n");
		return CMD_SUCCESS;
	}
		
	query = dbus_message_new_method_call(	NPD_DBUS_BUSNAME,
											NPD_DBUS_FDB_OBJPATH,
											NPD_DBUS_FDB_INTERFACE,
											NPD_DBUS_FDB_METHOD_CONFIG_DEBUG_FDB_STATIC_DELETE_WITH_VLAN );
	
	dbus_error_init(&err);
	
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT16,&vlanid,
								DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	if (!(dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))) 
	{
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
			
			
	} 
	else 
	{
		if( FDB_RETURN_CODE_GENERAL == op_ret)
		{
			vty_out(vty,"%% Error,operations error when configuration!\n");
		}
		else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == op_ret)
		{
			vty_out(vty,"%% Error,the vlan input not exist!\n");
		}
		else if(FDB_RETURN_CODE_OCCUR_HW == op_ret)
		{
			vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}



DEFUN(config_debug_fdb_mac_vlan_port_static_cmd_func,
	config_debug_fdb_mac_vlan_port_cmd,
	"create fdb static mac MAC vlan <1-4094> dev <0-1> port PORTNO",
	"Config system information\n"
	"Config FDB table\n"
	"Find static fdb item \n"
	"Config MAC field of FDB table.\n"
	"Config MAC address in Hex format\n"
	"Config FDB table vlan Id\n"
	"Config VLAN Id valid range <1-4094>\n"
	"Config dev \n"
	"config dev id valid range <0-1>\n"
	"Config port \n"
	"Config port id valid range <0-100>\n"
)
{
	DBusMessage *query, *reply;
	ETHERADDR		macAddr;
	unsigned short	vlanId = 0;
	unsigned char	dev = 0,port = 0; 
	DBusError err;
	unsigned int 	op_ret = 0;	
	int local_slot_id = 0;
    int slotNum = 0;
	
	memset(&macAddr,0,sizeof(ETHERADDR));
	
	op_ret = parse_mac_addr((char *)argv[0],&macAddr);
	if (NPD_FAIL == op_ret) 
	{
    	vty_out(vty,"% Bad Parameter,Unknow mac addr format!\n");
		return CMD_WARNING;
	}
   
	op_ret=is_muti_brc_mac(&macAddr);
	if(op_ret==1)
	{
		vty_out(vty,"%% Erro:input should not be broadcast or multicast mac!\n");
		return CMD_WARNING;
	}	
	op_ret = parse_short_parse((char*)argv[1], &vlanId);
	if (NPD_FAIL == op_ret) 
	{
    	vty_out(vty,"% Bad Parameters,Unknow vlan id format.\n");
		return CMD_WARNING;
	}
    if ((vlanId<MIN_VLANID)||(vlanId>MAX_VLANID))
	{
		vty_out(vty,"% Bad Parameters,FDB vlan outrange!\n");
		return CMD_WARNING;
	}
	/*vty_out(vty,"fdb vlan id %d.\n",vlanId);*/

	if(0 == strncmp("0", argv[2], strlen(argv[2]))){
	   dev = 0;
	}
	else if(0 == strncmp("1", argv[2], strlen(argv[2]))){
	   dev = 1;
	}
	else{
		vty_out(vty,"%% Bad parameter %s!\n", argv[2]);
		return CMD_WARNING;
	}
	
	port = strtoul((char*)argv[3], NULL, 0);
	if ((port > 100) || (port < 0)) {
	   vty_out(vty,"%% Bad parameter %s!\n", argv[3]);
	   return CMD_WARNING;
	}
  	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH, \
										NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_CONFIG_DEBUG_FDB_STATIC);
	
	dbus_error_init(&err);
	dbus_message_append_args(	query,
								DBUS_TYPE_UINT16,&vlanId,
								DBUS_TYPE_BYTE,&macAddr.arEther[0],
								DBUS_TYPE_BYTE,&macAddr.arEther[1],
								DBUS_TYPE_BYTE,&macAddr.arEther[2],
								DBUS_TYPE_BYTE,&macAddr.arEther[3],
								DBUS_TYPE_BYTE,&macAddr.arEther[4],
								DBUS_TYPE_BYTE,&macAddr.arEther[5],
								DBUS_TYPE_BYTE,&dev,
								DBUS_TYPE_BYTE,&port,
								DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return NPD_FDB_ERR_NONE;
	}
	
	if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID)) 
	{
		if( FDB_RETURN_CODE_GENERAL == op_ret)
		{
			vty_out(vty,"%% Error,operations error when configuration!\n");
		}
		else if(FDB_RETURN_CODE_NODE_VLAN_NONEXIST == op_ret)
		{
			vty_out(vty,"% Bad parameters,the vlan input not exist!\n");
		}
		else if(FDB_RETURN_CODE_NODE_PORT_NOTIN_VLAN == op_ret)
		{
			vty_out(vty,"%% Error,the port is not in vlan!\n");
		}
		else if(FDB_RETURN_CODE_BADPARA == op_ret)
		{
			vty_out(vty,"%% Error,the parameter input contains some error!\n");
		}
		else if(FDB_RETURN_CODE_OCCUR_HW == op_ret)
		{
			vty_out(vty,"%% Error,there is something wrong when configuration hw.\n");
		}
		else if(FDB_RETURN_CODE_SYSTEM_MAC == op_ret)
		{
			vty_out(vty,"%% Error,the mac input conflict with system mac,please chang!\n");
		}
		else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret)
		{
			vty_out(vty,"%% Product not support this function!\n");
		}
			
	}
	
	else 
	{
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
  
	return CMD_SUCCESS;
}


DEFUN(syn_fdb_cmd_func,
	syn_fdb_cmd,
	"syn fdb with slot <1-16>" ,
	"Syn fdb between to slots\n"
	"FDB table content!\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int dnumber = 0;
	unsigned int dcli_flag =0;
	unsigned int type_flag = 0;
	unsigned int dev = 0;
	unsigned char* type = NULL;
	
	unsigned char  show_mac[6] = {0};
	unsigned short vlanid=0;
	unsigned int i=0,j=0;
	unsigned int devNum = 0;
	unsigned int local_fdb_count = 0,special_fdb_count = 0;
	unsigned int portNum = 0;
	unsigned int trans_value1 = 0;
	unsigned int trans_value2 = 0;
	unsigned int k=0;
	unsigned int asic_num = 0;
	int ret = 0;
	static DCLI_FDB_DBG dcli_save_fdb[32768];
	int local_slot_id = 0;
    int slotNum = 0;
	unsigned short slot_id = 0;
	int slot_count = 0;
	int total_fdb_count = 0;

	memset(dcli_save_fdb,0,sizeof(DCLI_FDB_DBG)*32768);
	local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	ret = parse_single_param_no((char*)argv[0],&slot_id);
	if(CMD_SUCCESS != ret) 
	{
		vty_out(vty,"%% parse param failed!\n");
		return CMD_WARNING;
	}
    if((slot_id > slotNum) || (slot_id == local_slot_id))
    {
    	vty_out(vty,"% Bad parameter,slot id illegal!\n");
		return CMD_WARNING;		
    }

	for(k=1; k <= slotNum; k++)
	{
		if((k != local_slot_id) && (k != slot_id))
		{
			continue;
		}
        query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
							NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SHOW_FDB_TABLE_DEBUG);

        dbus_error_init(&err);
        if(NULL == dbus_connection_dcli[k]->dcli_dbus_connection) 				
		{
			if(k == local_slot_id)
			{
                reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
			}
			else 
			{	
			   	vty_out(vty,"Can not connect to slot:%d \n",k);	
				continue;
			}
        }
		else
		{
            reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[k]->dcli_dbus_connection,query,-1, &err);				
		}
		
        dbus_message_unref(query);
		if (NULL == reply) 
		{
			vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",k);
        	return CMD_SUCCESS;
		}
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&dnumber);
		if (dnumber == 0)
		{
			if(k == slotNum)
			{
				return CMD_SUCCESS;
			}
			else
			{
				continue;
			}
		}
		dbus_message_iter_next(&iter);

		dbus_message_iter_recurse(&iter,&iter_array);
		for ( i=0; dnumber > i; i++)
		{
			DBusMessageIter iter_struct;
		
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dcli_flag);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&dev);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&asic_num);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&vlanid);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trans_value1);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trans_value2);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&type_flag);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[1]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[2]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[3]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[4]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&show_mac[5]);
			j=(local_slot_id<slot_id)?(local_fdb_count+i):(special_fdb_count+i);
			dcli_save_fdb[j].asic_num = asic_num;
			dcli_save_fdb[j].dev = dev;
			dcli_save_fdb[j].inter_type = dcli_flag;
			dcli_save_fdb[j].vlanid = vlanid;
			dcli_save_fdb[j].type_flag = type_flag;
			dcli_save_fdb[j].ether_mac[0] = show_mac[0];
			dcli_save_fdb[j].ether_mac[1] = show_mac[1];
			dcli_save_fdb[j].ether_mac[2] = show_mac[2];
			dcli_save_fdb[j].ether_mac[3] = show_mac[3];
			dcli_save_fdb[j].ether_mac[4] = show_mac[4];
			dcli_save_fdb[j].ether_mac[5] = show_mac[5];
			dcli_save_fdb[j].slot_id = k;
			dcli_save_fdb[j].value = trans_value1;
			if (dcli_flag == port_type)
			{
				dcli_save_fdb[j].value = trans_value2;
			}
			dbus_message_iter_next(&iter_array);
	
		}

		if(k == local_slot_id)
		{
			local_fdb_count = dnumber;
		}
		else
		{
			special_fdb_count = dnumber;
		}
		dnumber = 0;
		j = 0;
		dbus_message_unref(reply);
	}
	#if 0
	vty_out(vty,"Codes:  CPU - target port which connect to cpu via pci interface\n");
	vty_out(vty,"        HSC - high speed channel direct to cpu rgmii interface\n\n");
	/* NOTICE We used 68 bytes here, we still got 12 bytes of summary info*/
	vty_out(vty,"%-17s  %-4s  %-4s  %-9s  %-5s  %-4s  %-4s  %-8s\n","=================",	"====","====", "=========","=====","====","====","========");
	vty_out(vty,"%-17s  %-4s  %-4s  %-9s  %-5s  %-4s  %-4s  %-8s\n","MAC","VLAN","ASIC","DEV/PORT","TRUNK","VID","SLOT","TYPE");
	vty_out(vty,"%-17s  %-4s  %-4s  %-9s  %-5s  %-4s  %-4s  %-8s\n","=================",	"====","====", "=========","=====","====","====","========");
	#if 1
	vty_out(vty,"=======================================================\n");
	vty_out(vty,"SLOT %d FDB count is %d\n",local_slot_id,local_fdb_count);
	vty_out(vty,"=======================================================\n");
	#endif
	for(i=((local_slot_id<slot_id)?0:special_fdb_count) ;i<((local_slot_id<slot_id)?(local_fdb_count):(local_fdb_count+special_fdb_count));i++)
	{
		asic_num = dcli_save_fdb[i].asic_num;
		dev = dcli_save_fdb[i].dev;
		dcli_flag = dcli_save_fdb[i].inter_type;
		vlanid = dcli_save_fdb[i].vlanid;
		type_flag = dcli_save_fdb[i].type_flag;
		show_mac[0] = dcli_save_fdb[i].ether_mac[0];
		show_mac[1] =dcli_save_fdb[i].ether_mac[1];
		show_mac[2] =dcli_save_fdb[i].ether_mac[2];
		show_mac[3] =dcli_save_fdb[i].ether_mac[3];
		show_mac[4] =dcli_save_fdb[i].ether_mac[4];
		show_mac[5] =dcli_save_fdb[i].ether_mac[5];
		if (dcli_flag == port_type)
		{
			trans_value1 = 0;
			trans_value2 = dcli_save_fdb[i].value;
			if(trans_value2 == CPU_PORT_VIRTUAL_PORT)
			{
				trans_value1 = CPU_PORT_VIRTUAL_SLOT;
			}
		}
		else
		{
			trans_value1 = dcli_save_fdb[i].value ;
		}
		if( 0 ==type_flag)
		{
	   	    type = "DYNAMIC";
		}
		else if(1 ==type_flag)
		{
			type = "STATIC";
		}
		else 
		{
			vty_out (vty,"sorry fdb item type wrong !\n");
			return -1;
		}
		if (dcli_flag == port_type)
		{
			if(CPU_PORT_VIRTUAL_SLOT == trans_value1) 
			{ /*static FDB to CPU or SPI (has the same virtual slot number)*/
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-4d  %-4d  %-9s  %-5s  %4s  %-4d  %-8s\n",	\
						show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
						vlanid,asic_num,(CPU_PORT_VIRTUAL_PORT == trans_value2) ? "   CPU   ":	\
						(SPI_PORT_VIRTUAL_PORT == trans_value2) ? "   HSC   ":"   ERR   ","  -  "," - ",dcli_save_fdb[i].slot_id," - ");
			}
			else 
			{
				if(trans_value1 <= 10)
				{
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-4d  %-4d  %4d/%-4d  %-5s  %-4s  %-4d  %-8s\n",   \
								show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
								vlanid,asic_num,dev,trans_value2," - "," - ",dcli_save_fdb[i].slot_id,type);
				}
						
			}
		}
		else if (dcli_flag == trunk_type)
		{ 				
			vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-4d  %-4d  %9s  %-5d  %-4s  %-4d  %-8s\n",    \
			       	 show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
			       	 vlanid,asic_num,"    -    ",trans_value1," - ",dcli_save_fdb[i].slot_id,type);
		}
	
		else if (dcli_flag == vid_type)
		{				
			vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8d%-6s%-6s\n",     \
			      	 show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
			      	 vlanid," - "," - ",trans_value1," - ",type);	
		}			
		else if (dcli_flag == vidx_type)
		{				
			vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8s%-8d%-6s\n",     \
			     	  show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
			      	 vlanid," - "," - "," - ",trans_value1,type);				
		}
		else 
		{
			vty_out (vty,"sorry interface type wrong !\n");
			return -1;
		}	
	}

	#if 1
	vty_out(vty,"=======================================================\n");
	vty_out(vty,"SLOT %d FDB count is %d\n",slot_id,special_fdb_count);
	vty_out(vty,"=======================================================\n");
	for(i=((local_slot_id>slot_id)?0:local_fdb_count) ;i<((local_slot_id>slot_id)?(special_fdb_count):(local_fdb_count+special_fdb_count));i++)
	{
		asic_num = dcli_save_fdb[i].asic_num;
		dev = dcli_save_fdb[i].dev;
		dcli_flag = dcli_save_fdb[i].inter_type;
		vlanid = dcli_save_fdb[i].vlanid;
		type_flag = dcli_save_fdb[i].type_flag;
		show_mac[0] = dcli_save_fdb[i].ether_mac[0];
		show_mac[1] =dcli_save_fdb[i].ether_mac[1];
		show_mac[2] =dcli_save_fdb[i].ether_mac[2];
		show_mac[3] =dcli_save_fdb[i].ether_mac[3];
		show_mac[4] =dcli_save_fdb[i].ether_mac[4];
		show_mac[5] =dcli_save_fdb[i].ether_mac[5];
		if (dcli_flag == port_type)
		{
			trans_value1 = 0;
			trans_value2 = dcli_save_fdb[i].value;
			if(trans_value2 == CPU_PORT_VIRTUAL_PORT)
			{
				trans_value1 = CPU_PORT_VIRTUAL_SLOT;
			}
		}
		else
		{
			trans_value1 = dcli_save_fdb[i].value ;
		}
		if( 0 ==type_flag)
		{
	   	    type = "DYNAMIC";
		}
		else if(1 ==type_flag)
		{
			type = "STATIC";
		}
		else 
		{
			vty_out (vty,"sorry fdb item type wrong !\n");
			return -1;
		}
		if (dcli_flag == port_type)
		{
			if(CPU_PORT_VIRTUAL_SLOT == trans_value1) 
			{ /*static FDB to CPU or SPI (has the same virtual slot number)*/
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-4d  %-4d  %-9s  %-5s  %4s  %-4d  %-8s\n",	\
						show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
						vlanid,asic_num,(CPU_PORT_VIRTUAL_PORT == trans_value2) ? "   CPU   ":	\
						(SPI_PORT_VIRTUAL_PORT == trans_value2) ? "   HSC   ":"   ERR   ","  -  "," - ",dcli_save_fdb[i].slot_id," - ");
			}
			else 
			{
				if(trans_value1 <= 10)
				{
					vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-4d  %-4d  %4d/%-4d  %-5s  %-4s  %-4d  %-8s\n",   \
								show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
								vlanid,asic_num,dev,trans_value2," - "," - ",dcli_save_fdb[i].slot_id,type);
				}
						
			}
		}
		else if (dcli_flag == trunk_type)
		{ 				
			vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-4d  %-4d  %9s  %-5d  %-4s  %-4d  %-8s\n",    \
			       	 show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],	\
			       	 vlanid,asic_num,"    -    ",trans_value1," - ",dcli_save_fdb[i].slot_id,type);
		}
	
		else if (dcli_flag == vid_type)
		{				
			vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8d%-6s%-6s\n",     \
			      	 show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
			      	 vlanid," - "," - ",trans_value1," - ",type);	
		}			
		else if (dcli_flag == vidx_type)
		{				
			vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  %-8d%-11s%-8s%-8s%-8d%-6s\n",     \
			     	  show_mac[0],show_mac[1],show_mac[2],show_mac[3],show_mac[4],show_mac[5],		\
			      	 vlanid," - "," - "," - ",trans_value1,type);				
		}
		else 
		{
			vty_out (vty,"sorry interface type wrong !\n");
			return -1;
		}	
	}
	#endif
	#endif
	total_fdb_count = local_fdb_count + special_fdb_count;
	
	for(k=1; k <= slotNum; k++)
	{
		if((k != local_slot_id) && (k != slot_id))
		{
			continue;
		}
        query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_FDB_OBJPATH,	\
							NPD_DBUS_FDB_INTERFACE,NPD_DBUS_FDB_METHOD_SYN_FDB_TABLE);

        dbus_error_init(&err);
      
		dbus_message_iter_init_append (query, &iter);
	
		dbus_message_iter_append_basic (&iter,
									 	DBUS_TYPE_UINT32,&total_fdb_count);
		dbus_message_iter_open_container (&iter,
									   	DBUS_TYPE_ARRAY,
									   	DBUS_STRUCT_BEGIN_CHAR_AS_STRING
										DBUS_TYPE_UINT32_AS_STRING
										DBUS_TYPE_UINT32_AS_STRING
										DBUS_TYPE_UINT32_AS_STRING
										DBUS_TYPE_UINT32_AS_STRING
										DBUS_TYPE_UINT16_AS_STRING
										DBUS_TYPE_UINT32_AS_STRING
									   	DBUS_TYPE_UINT32_AS_STRING
									   	DBUS_TYPE_UINT32_AS_STRING
									   	DBUS_TYPE_BYTE_AS_STRING 
									   	DBUS_TYPE_BYTE_AS_STRING 
									   	DBUS_TYPE_BYTE_AS_STRING 
									   	DBUS_TYPE_BYTE_AS_STRING 
									   	DBUS_TYPE_BYTE_AS_STRING 
									   	DBUS_TYPE_BYTE_AS_STRING 
									   	DBUS_STRUCT_END_CHAR_AS_STRING,
									   &iter_array);
		for ( i=0; total_fdb_count > i; i++)
		{
			if (dcli_save_fdb[i].inter_type == port_type)
			{
				trans_value1 = 0;
				trans_value2 = dcli_save_fdb[i].value;
				if(trans_value2 == CPU_PORT_VIRTUAL_PORT)
				{
					trans_value1 = CPU_PORT_VIRTUAL_SLOT;
				}
			}
			else
			{
				trans_value2 = 0;
				trans_value1 = dcli_save_fdb[i].value ;
			}
			DBusMessageIter iter_struct;
			dbus_message_iter_open_container (&iter_array,
											   DBUS_TYPE_STRUCT,
											   NULL,
											   &iter_struct);
			/* fdb destination interface type */
			dbus_message_iter_append_basic
					(&iter_struct,
					 DBUS_TYPE_UINT32,
					 &(dcli_save_fdb[i].inter_type));
				/*devNum*/
			dbus_message_iter_append_basic
					(&iter_struct,
					 DBUS_TYPE_UINT32,
					 &(dcli_save_fdb[i].dev));
				/*asic_num*/
			dbus_message_iter_append_basic
					(&iter_struct,
					 DBUS_TYPE_UINT32,
					 &(dcli_save_fdb[i].asic_num));
			dbus_message_iter_append_basic
					(&iter_struct,
					 DBUS_TYPE_UINT32,
					 &(dcli_save_fdb[i].slot_id));
			/* VLAN ID*/
			dbus_message_iter_append_basic
					(&iter_struct,
					 DBUS_TYPE_UINT16,
					 &(dcli_save_fdb[i].vlanid));

			/* SLOT number or VLAN ID or VIDX or Trunk ID*/
			dbus_message_iter_append_basic
					(&iter_struct,
					 DBUS_TYPE_UINT32,
					 &trans_value1);

			/* PORT number*/
			dbus_message_iter_append_basic
					(&iter_struct,
					 DBUS_TYPE_UINT32,
					 &trans_value2);
			/*ITEM type*/
			dbus_message_iter_append_basic
					(&iter_struct,
					 DBUS_TYPE_UINT32,
					 &(dcli_save_fdb[i].type_flag));

			/*MAC address*/
			dbus_message_iter_append_basic
					(&iter_struct,
					 DBUS_TYPE_BYTE,
					 &(dcli_save_fdb[i].ether_mac[0]));
			dbus_message_iter_append_basic
					(&iter_struct,
					 DBUS_TYPE_BYTE,
					 &(dcli_save_fdb[i].ether_mac[1]));
			dbus_message_iter_append_basic
					(&iter_struct,
					 DBUS_TYPE_BYTE,
					 &(dcli_save_fdb[i].ether_mac[2]));
			dbus_message_iter_append_basic
					(&iter_struct,
					 DBUS_TYPE_BYTE,
					&(dcli_save_fdb[i].ether_mac[3]));
			dbus_message_iter_append_basic
					(&iter_struct,
					 DBUS_TYPE_BYTE,
					 &(dcli_save_fdb[i].ether_mac[4]));
			dbus_message_iter_append_basic
					(&iter_struct,
					 DBUS_TYPE_BYTE,
					 &(dcli_save_fdb[i].ether_mac[5]));
			dbus_message_iter_close_container (&iter_array, &iter_struct);
		}
		dbus_message_iter_close_container (&iter, &iter_array);
		if(NULL == dbus_connection_dcli[k]->dcli_dbus_connection) 				
		{
			if(k == local_slot_id)
			{
                reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
			}
			else 
			{	
			   	vty_out(vty,"Can not connect to slot:%d \n",k);	
				continue;
			}
        }
		else
		{
            reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[k]->dcli_dbus_connection,query,-1, &err);				
		}
		
        dbus_message_unref(query);
		if (NULL == reply) 
		{
			vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",k);
        	return CMD_SUCCESS;
		}
		else
		{
			if (dbus_message_get_args ( reply, &err,
										DBUS_TYPE_UINT32,&ret,
										DBUS_TYPE_INVALID)) 
			{
				if(ret == CMD_SUCCESS)
				{
					vty_out(vty,"Slot %d syn fdb table successfully !\n",k);
				}
				else
				{
					vty_out(vty,"Slot %d syn fdb table failed !\n",k);
				}
			} 
			else 
			{
				if (dbus_error_is_set(&err)) 
				{
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
			}
			dbus_message_unref(reply);
		}
	}
		
	
	return CMD_SUCCESS;	
}

DEFUN(show_sfd_info_func,
	show_sfd_info_cmd,
	"show sfd info",
	"show sfd info"
	"config node\n"
	"show sfd info\n"
	)
{
	vty_out(vty, "====================================================\n");
    vty_out(vty, "Wireless SFD info\n");
	if(sfd_flag)
	    vty_out(vty, "SFD :ON\n");
	else
		vty_out(vty, "SFD :OFF\n");
	if(sfd_debug_flag)
	    vty_out(vty, "SFD DEBUG:ON\n");
	else
		vty_out(vty, "SFD DEBUG:OFF\n");
	vty_out(vty, "====================================================\n");
	return CMD_SUCCESS;	
}

DEFUN(service_sfd_debug_func,
	service_sfd_debug_cmd,
	"service sfd debug (on|off)",
	"service sfd debug (on|off)"
	"config node\n"
	"config sfd info\n"
	"debug sfd on\n"
	"debug sfd off\n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusMessageIter iter;
	DBusError err;
    int ret;
	int flag;
	
	if(0 == strncmp("on",argv[0],strlen((char*)argv[0])))
	{
		sfd_debug_flag = SFD_ON;
	}
	else if(0 == strncmp("off",argv[0],strlen((char*)argv[0])))
	{
		sfd_debug_flag = SFD_OFF;
	}
	else
	{
    	vty_out(vty, "service sfd debug set failed\n");
		return CMD_WARNING;
	}
		
    vty_out(vty, "service sfd debug set successfully\n");
	return CMD_SUCCESS;	
}

DEFUN(service_sfd_func,
	service_sfd_cmd,
	"service sfd (on|off)",
	"service sfd (on|off)"
	"config node\n"
	"debug sfd on\n"
	"debug sfd off\n"
	)
{	
	if(0 == strncmp("on",argv[0],strlen((char*)argv[0])))
	{
		sfd_flag = 1;
	}
	else if(0 == strncmp("off",argv[0],strlen((char*)argv[0])))
	{
		sfd_flag = 0;
	}
	else
	{
    	vty_out(vty, "service sfd set failed\n");
		return CMD_WARNING;
	}
	

    vty_out(vty, "service sfd set successfully\n");

		

	return CMD_SUCCESS;	
}

struct cmd_node fdb_node = 
{
	FDB_NODE,
	" ",
	1
};


void dcli_fdb_init() {
	install_node(&fdb_node,dcli_fdb_show_running_config,"FDB_NODE");
	install_element(CONFIG_NODE, &config_fdb_agingtime_cmd);
	install_element(CONFIG_NODE, &show_fdb_agingtime_cmd);
	install_element(VIEW_NODE,   &show_fdb_agingtime_cmd);
	install_element(ENABLE_NODE, &show_fdb_agingtime_cmd);
	install_element(CONFIG_NODE, &config_fdb_no_agingtime_cmd);
	install_element(CONFIG_NODE, &config_fdb_mac_vlan_port_cmd);
	install_element(CONFIG_NODE, &config_fdb_delete_vlan_cmd);
	install_element(CONFIG_NODE, &config_fdb_delete_port_cmd);
	install_element(CONFIG_NODE, &config_fdb_delete_trunk_cmd);
	install_element(CONFIG_NODE, &config_fdb_static_delete_vlan_cmd);
	install_element(CONFIG_NODE, &config_fdb_static_delete_port_cmd);
	install_element(CONFIG_NODE, &config_fdb_mac_vlan_trunk_cmd);
	install_element(CONFIG_NODE, &show_fdb_port_cmd);
	install_element(VIEW_NODE,   &show_fdb_port_cmd);
	install_element(ENABLE_NODE, &show_fdb_port_cmd);
	install_element(CONFIG_NODE, &show_fdb_vlan_cmd);
	install_element(VIEW_NODE,   &show_fdb_vlan_cmd);
	install_element(ENABLE_NODE, &show_fdb_vlan_cmd);
	install_element(CONFIG_NODE, &config_fdb_mac_vlan_match_no_drop_cmd);
	install_element(CONFIG_NODE, &config_fdb_mac_vlan_match_no_drop_name_cmd);
	install_element(CONFIG_NODE, &config_fdb_mac_vlan_match_cmd);
	install_element(CONFIG_NODE, &config_fdb_mac_vlan_name_match_cmd);
	install_element(CONFIG_NODE, &config_fdb_mac_vlan_no_static_port_cmd);
	install_element(CONFIG_NODE, &config_fdb_mac_vlan_no_static_port_name_cmd);
	install_element(CONFIG_NODE, &show_fdb_cmd);
	install_element(VIEW_NODE,   &show_fdb_cmd);
	install_element(ENABLE_NODE, &show_fdb_cmd);
	install_element(CONFIG_NODE, &show_fdb_dynamic_cmd);
	install_element(VIEW_NODE,   &show_fdb_dynamic_cmd);
	install_element(ENABLE_NODE, &show_fdb_dynamic_cmd);
	install_element(CONFIG_NODE, &show_fdb_count_cmd);
	install_element(VIEW_NODE,   &show_fdb_count_cmd);
	install_element(ENABLE_NODE, &show_fdb_count_cmd);
	install_element(CONFIG_NODE, &show_fdb_static_cmd);
	install_element(VIEW_NODE,   &show_fdb_static_cmd);
	install_element(ENABLE_NODE, &show_fdb_static_cmd);
	install_element(CONFIG_NODE, &show_fdb_blacklist_cmd);
	install_element(VIEW_NODE,   &show_fdb_blacklist_cmd);
	install_element(ENABLE_NODE, &show_fdb_blacklist_cmd);
	install_element(CONFIG_NODE, &config_fdb_system_mac_cmd);
	install_element(HIDDENDEBUG_NODE, &show_fdb_cmd_test);	
	install_element(HIDDENDEBUG_NODE,&config_debug_fdb_mac_vlan_trunk_cmd);
	install_element(HIDDENDEBUG_NODE,&config_debug_fdb_static_delete_vlan_cmd);
	install_element(HIDDENDEBUG_NODE,&config_debug_fdb_mac_vlan_port_cmd);
	install_element(HIDDENDEBUG_NODE,&syn_fdb_cmd);

    install_element(CONFIG_NODE, &show_sfd_info_cmd);//huangjing
    install_element(CONFIG_NODE, &service_sfd_debug_cmd);//huangjing
	install_element(CONFIG_NODE, &service_sfd_cmd);//huangjing
	#if 0
	install_element(CONFIG_NODE, &config_fdb_port_number_cmd);
	install_element(CONFIG_NODE, &config_fdb_vlan_number_cmd);
	install_element(CONFIG_NODE, &show_fdb_number_limit_cmd);
	install_element(VIEW_NODE,   &show_fdb_number_limit_cmd);
	install_element(ENABLE_NODE, &show_fdb_number_limit_cmd);
	install_element(CONFIG_NODE, &config_fdb_vlan_port_number_cmd);
	install_element(CONFIG_NODE, &show_fdb_one_cmd);
	install_element(VIEW_NODE,   &show_fdb_one_cmd);
	install_element(ENABLE_NODE, &show_fdb_one_cmd);
	install_element(CONFIG_NODE, &show_fdb_mac_cmd);
	install_element(VIEW_NODE,   &show_fdb_mac_cmd);
	install_element(ENABLE_NODE, &show_fdb_mac_cmd);
	install_element(CONFIG_NODE, &config_fdb_mac_vlan_port_name_cmd);
	install_element(CONFIG_NODE, &config_fdb_mac_vlan_trunk_name_cmd);
	install_element(CONFIG_NODE, &show_fdb_vlan_name_cmd);
	install_element(VIEW_NODE,   &show_fdb_vlan_name_cmd);
	install_element(ENABLE_NODE, &show_fdb_vlan_name_cmd);
	#endif
	
}
#ifdef __cplusplus
}
#endif
