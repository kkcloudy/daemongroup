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
* dcli_vlan.c
*
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		CLI definition for VLAN module.
*
* DATE:
*		02/21/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.112 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
#include <zebra.h>
#include <stdlib.h>
#include <dbus/dbus.h>

/* for mmap() */
#include <fcntl.h>
#include <sys/mman.h>  
#if 0
/* add for share mem */
#include <unistd.h>  /*getpagesize(  ) */  
#include <sys/ipc.h>   
#include <sys/shm.h>   
#define MY_SHM_VLANLIST_ID 54321
#endif

#include "sysdef/npd_sysdef.h"
#include "dbus/npd/npd_dbus_def.h"
#include "util/npd_list.h"
#include "npd/nam/npd_amapi.h"
#include "command.h"
#include "if.h"
#include "dcli_vlan.h"
#include "dcli_trunk.h"
#include "sysdef/returncode.h"
#include "npd/nbm/npd_bmapi.h"

#include "dcli_main.h"
#include "dcli_sem.h"
#include "board/board_define.h"  /* AC_BOARD */

extern int is_distributed;

extern DBusConnection *dcli_dbus_connection;
/* for igmp show run in vlan */
extern DBusConnection *dcli_dbus_connection_igmp;
struct cmd_node vlan_node = 
{
	VLAN_NODE,
	"%s(config-vlan)# ",
	1
};
struct cmd_node vlan_egress_node = 
{
	VLAN_EGRESS_NODE,
	" ",
	1
};

int parse_vlan_no(char* str,unsigned short* vlanId) {
	char *endptr = NULL;
	char c;
	if (NULL == str) return NPD_FAIL;
	c = str[0];
	if (c>'0'&&c<='9'){
		*vlanId= strtoul(str,&endptr,10);
		if('\0' != endptr[0]){
			return NPD_FAIL;
		}
		return NPD_SUCCESS;	
	}
	else {
		return NPD_FAIL; /*not Vlan ID. for Example ,enter '.',and so on ,some special char.*/
	}
}
int parse_single_param_no (char* str,unsigned short* sig_param) {
	char *endptr = NULL;

	if (NULL == str) return NPD_FAIL;
	*sig_param= strtoul(str,&endptr,10);
	return NPD_SUCCESS;	
}

int parse_subvlan_no(char* str,unsigned short* subvlanId) {
	char *endptr = NULL;

	if (NULL == str) return NPD_FAIL;
	*subvlanId= strtoul(str,&endptr,10);
	return NPD_SUCCESS;	
}
int parse_slotno_localport(char* str,unsigned int *slotno,unsigned int *portno) 
{
	char *endptr = NULL;
	char c = 0;
	if (NULL == str) return -1;
	/* add for AX7605i-alpha cscd port by qinhs@autelan.com 2009-11-18 */
	if(!strncmp(tolower(str), "cscd", 4)) {
		*slotno = AX7i_XG_CONNECT_SLOT_NUM;
		if(strlen(str) > strlen("cscd*")) {
			return NPD_FAIL ;
		}
		else if('0' == str[4]) {
			*portno = 1;
		}		
		else if('1' == str[4]) {
			*portno = 2;
		}
		else {
			return NPD_FAIL;
		}
		return NPD_SUCCESS;
	}
	c = str[0];
	if (c>='0' && c<='9'){
		*slotno= strtoul(str,&endptr,10);
		if(SLOT_PORT_SPLIT_SLASH != endptr[0] &&
            SLOT_PORT_SPLIT_DASH != endptr[0]){
		    return -1;
		}
		else {
             *portno = strtoul(&endptr[1],&endptr,10);
             if( 52 < *portno ||'\0' != endptr[0])/*for au5000 :24; for ax7000 : 6*/
			 	return 1;
        }
		return 0;	
	}
	else {
		return -1; /*not Vlan ID. for Example ,enter '.',and so on ,some special char.*/
	}
}
int parse_slotno_localport_include_slot0(char* str,unsigned int *slotno,unsigned int *portno) 
{
	char *endptr = NULL;
	char c = 0;
	if (NULL == str) return -1;
	
	/* add for AX7605i-alpha cscd port by qinhs@autelan.com 2009-11-18 */
	if(!strncmp(tolower(str), "cscd", 4)) {
		*slotno = AX7i_XG_CONNECT_SLOT_NUM;
		if(strlen(str) > strlen("cscd*")) {
			return NPD_FAIL ;
		}
		else if('0' == str[4]) {
			*portno = 1;
		}		
		else if('1' == str[4]) {
			*portno = 2;
		}
		else {
			return NPD_FAIL;
		}
		return NPD_SUCCESS;
	}

	c = str[0];
	if (c>='0' && c<='9'){
		*slotno= strtoul(str,&endptr,10);
		if(SLOT_PORT_SPLIT_SLASH != endptr[0] &&
            SLOT_PORT_SPLIT_DASH != endptr[0]){
		    return -1;
		}
		else {
             *portno = strtoul(&endptr[1],&endptr,10);
             if( 52 < *portno ||'\0' != endptr[0])/*for au5000 :24; for ax7000 : 6*/
			 	return 1;
        }
		return 0;	
	}
	else {
		return -1; /*not Vlan ID. for Example ,enter '.',and so on ,some special char.*/
	}
}

/****************************************************************
*FUN:vlan_name_legal_check
*Params :
*	IN	str	:vlan name string user entered from vtysh. 
*		len	:Length of vlan name string 
*
*	OUT ALIAS_NAME_LEN_ERROR--vlan name too long
*		ALIAS_NAME_HEAD_ERROR--illegal char on head of vlan name 
*		ALIAS_NAME_BODY_ERROR--unsupported char in vlan name
****************************************************************/
int vlan_name_legal_check(char* str,unsigned int len)
{
	int i;
	int ret = NPD_FAIL;
	char c = 0;
	if((NULL == str)||(len==0)){
		return ret;
	}
	if(len >= ALIAS_NAME_SIZE){
		ret = ALIAS_NAME_LEN_ERROR;
		return ret;
	}

	c = str[0];
	if(	(c=='_')||
		(c<='z'&&c>='a')||
		(c<='Z'&&c>='A')
	  ){
		ret = NPD_SUCCESS;
	}
	else {
		return ALIAS_NAME_HEAD_ERROR;
	}
	for (i=1;i<=len-1;i++){
		c = str[i];
		if( (c>='0' && c<='9')||
			(c<='z'&&c>='a')||
		    (c<='Z'&&c>='A')||
		    (c=='_')
		    ){
			continue;
		}
		else {
			ret = ALIAS_NAME_BODY_ERROR;
			break;
		}
	}
	return ret;
}
/****************************************************
*FUN:param_first_char_check 
*Params: 
*	IN	str		:String field user entered from vtysh. 
*		cmdtip	:Type of string field::0--add/delete
*									   1--tag/untag
*	OUT NPD_FAIL :Bad Command String Field.
*******************************************/
int param_first_char_check(char* str,unsigned int cmdtip)
{
	int i;
	int ret = NPD_FAIL;
	char c = 0;
	if(NULL == str){
		return ret;
	}
	c = str[0];
	switch (cmdtip){
		case 0:/*add/delete*/
			if('a' == c){ret = 1;}
			else if('d' == c){ret = 0;}
			else {ret = NPD_FAIL;}
			break;
		case 1:/*untag/tag*/
			if(c =='t'){ret = 1;}
			else if('u' == c){ret = 0;}
			else {ret = NPD_FAIL;}
			break;
		default:
			break;
	}
	return ret;
	
}
/**************************************
 *
 * === parse port list [Slot/Port]
 *
 *
 *************************************/
int parse_port_list(char* ptr,int* count,SLOT_PORT_S spL[])
{
    char* endPtr = NULL;
    short tmpslot = 0,tmpport = 0;
    int   i=0,cc=0;

    endPtr = ptr;
    if(NULL == endPtr)	return NPD_FAIL;
	while (endPtr) {
    	tmpslot = strtoul(endPtr,&endPtr,10);
    	if(SLOT_LLEGAL(tmpslot)){
            spL[i].slot = tmpslot;
            /*printf("iter i = %d,tmpslot = %d.\r\n",i,tmpslot);*/
        }
		if (SLOT_PORT_SPLIT_SLASH== endPtr[0]||
            SLOT_PORT_SPLIT_DASH == endPtr[0]) {
	        tmpport = strtoul((char *)&(endPtr[1]),&endPtr,10);
        	
        	if(PORT_LLEGAL(tmpport)){
                spL[i].port = tmpport;
                /*printf("iter i = %d,tmpport = %d.\r\n",i,tmpport);*/
            }
        }
		if(SLOT_PORT_SPLIT_COMMA!= endPtr[0]&&('\0' != endPtr[0])){
			return NPD_FAIL;
		}
		else if(SLOT_PORT_SPLIT_COMMA == endPtr[0]){
			
            endPtr = (char*)endPtr + 1;
        }
        i++;
        
       	if ('\0' == endPtr[0]) {
			
		   break;
        }
	}
    for(cc = 0;cc<i;cc++){
        if( 0 == spL[cc].slot ||
            0 == spL[cc].port){
            return NPD_FAIL;
        }       
    }
    *count = i;
	return 0;	
}

int dcli_vlan_qinq_endis
(
	struct vty* vty,
	boolean isEnable,
	unsigned short vlanid,
	unsigned char slotno,
	unsigned char portno
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int 	op_ret = 0;	
	if(is_distributed == DISTRIBUTED_SYSTEM)
	{
		int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
		if(local_slot_id<0)
		{
			vty_out(vty,"get get_product_info return -1 !\n");
			return CMD_SUCCESS;		
	   	}

		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_VLAN_OBJPATH,	\
											NPD_DBUS_VLAN_INTERFACE,	\
											NPD_DBUS_VLAN_METHOD_CONFIG_PORT_MEMBER_QINQ_ENDIS);
	
		dbus_error_init(&err);

		dbus_message_append_args(	query,
							 		DBUS_TYPE_BYTE,&isEnable,
									DBUS_TYPE_BYTE,&slotno,
									DBUS_TYPE_BYTE,&portno,
							 		DBUS_TYPE_UINT16,&vlanid,
							 		DBUS_TYPE_INVALID);
	
		
		if(NULL == dbus_connection_dcli[slotno]->dcli_dbus_connection)				
		{
			if(slotno == local_slot_id)
			{
				reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
			}
			else 
			{	
				vty_out(vty,"Can not connect to slot:%d, check the slot please !\n",slotno);	
				return CMD_WARNING;
			}
		}
		else
		{
			reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slotno]->dcli_dbus_connection,query,-1, &err);				
		}

		dbus_message_unref(query);
		if (NULL == reply) 
		{
			vty_out(vty,"failed get reply.\n");
			if (dbus_error_is_set(&err)) 
			{
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}

		if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID)) 
		{
			if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret) 
			{
        		vty_out(vty,"%% Bad parameter: Bad slot/port number.\n");
			}/*either slot or local port No err,return NPD_DBUS_ERROR_NO_SUCH_PORT */
			else if (VLAN_RETURN_CODE_PORT_TAG_CONFLICT == op_ret) 
			{
				vty_out(vty,"%% Bad parameter: port Tag-Mode can NOT match!\n");
			}
			else if (VLAN_RETURN_CODE_PORT_NOTEXISTS == op_ret)
			{
				vty_out(vty,"%% Bad parameter: port NOT member of the vlan.\n");
			}
			else if(VLAN_RETURN_CODE_PORT_EXISTS == op_ret)
			{
				vty_out(vty,"%% port %d/%d qinq has %s already !!!\n",slotno,portno,isEnable?"enabled":"disabled");
			}
			else if (VLAN_RETURN_CODE_ERR_NONE != op_ret)
			{
				vty_out(vty,"%% Unknown Error! return %d \n",op_ret);
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
		if(VLAN_RETURN_CODE_ERR_NONE != op_ret)
			return CMD_WARNING;
		else 
			return CMD_SUCCESS;
	}
	else
	{
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_VLAN_OBJPATH,	\
											NPD_DBUS_VLAN_INTERFACE,	\
											NPD_DBUS_VLAN_METHOD_CONFIG_PORT_MEMBER_QINQ_ENDIS);
	
		dbus_error_init(&err);

		dbus_message_append_args(	query,
							 		DBUS_TYPE_BYTE,&isEnable,
									DBUS_TYPE_BYTE,&slotno,
									DBUS_TYPE_BYTE,&portno,
							 		DBUS_TYPE_UINT16,&vlanid,
							 		DBUS_TYPE_INVALID);
	
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);



		dbus_message_unref(query);
		if (NULL == reply) {
			vty_out(vty,"failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}

		if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID)) {
			if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret) {
        		vty_out(vty,"%% Bad parameter: Bad slot/port number.\n");
			}/*either slot or local port No err,return NPD_DBUS_ERROR_NO_SUCH_PORT */
			else if (VLAN_RETURN_CODE_ERR_NONE == op_ret ) {
				/*vty_out(vty,"Ethernet Port %d-%d %s success.\n",slot_no,local_port_no,isAdd?"Add":"Delete");*/
			} 
			else if (VLAN_RETURN_CODE_BADPARAM == op_ret) {
				vty_out(vty,"%% Error occurs in Parse eth-port Index or devNO.& logicPortNo.\n");
			}
			else if (VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret) {
				vty_out(vty,"%% Error: Checkout-vlan to be configured NOT exists on SW.\n");
			}
			else if (VLAN_RETURN_CODE_PORT_EXISTS == op_ret) {
				vty_out(vty,"%% Bad parameter: port Already member of the vlan.\n");
			}
			else if (VLAN_RETURN_CODE_PORT_NOTEXISTS == op_ret){
				vty_out(vty,"%% Bad parameter: port NOT member of the vlan.\n");
			}
			else if (VLAN_RETURN_CODE_PORT_MBRSHIP_CONFLICT == op_ret) {
				vty_out(vty,"%% Bad parameter: port Already untagged member of other active vlan.\n");
			}			
			else if (VLAN_RETURN_CODE_PORT_PROMIS_PORT_CANNOT_ADD2_VLAN == op_ret) {
				vty_out(vty,"%% Promiscous port %d/%d can't be added to any vlan!\n",slotno,portno);
			}
			else if (VLAN_RETURN_CODE_PORT_PROMISCUOUS_MODE_ADD2_L3INTF == op_ret){
				vty_out(vty,"%% Bad parameter: promiscuous mode port can't add to l3 interface!\n");
			}
			else if (VLAN_RETURN_CODE_PORT_TAG_CONFLICT == op_ret) {
				vty_out(vty,"%% Bad parameter: port Tag-Mode can NOT match!\n");
			}
			else if (VLAN_RETURN_CODE_PORT_SUBINTF_EXISTS == op_ret){
            	vty_out(vty,"%% Can't delete tag port with sub interface!\n");
			}
			else if (VLAN_RETURN_CODE_PORT_DEL_PROMIS_PORT_TO_DFLT_VLAN_INTF == op_ret){
				vty_out(vty,"%% Promiscuous mode port can't delete!\n");
			}
			else if (VLAN_RETURN_CODE_PORT_TRUNK_MBR == op_ret) {
				vty_out(vty,"%% Bad parameter: port is member of a active trunk!\n");
			}
			else if (VLAN_RETURN_CODE_ARP_STATIC_CONFLICT == op_ret) {
				vty_out(vty,"%% Bad parameter: port has attribute of static arp!\n");
			}
			else if(PVLAN_RETURN_CODE_THIS_PORT_HAVE_PVE == op_ret) {
				vty_out(vty,"%% Bad parameter: port is pvlan port!please delete pvlan first!\n");
			}
        	else if(VLAN_RETURN_CODE_HAVE_PROT_VLAN_CONFIG == op_ret) {
				vty_out(vty,"%% There are protocol-vlan config on this vlan and port!\n");
			}
			else if (VLAN_RETURN_CODE_L3_INTF == op_ret) {
		
				vty_out(vty,"%% L3 interface vlan member can NOT delete here.\n");
			}
			else if (VLAN_RETURN_CODE_PORT_L3_INTF == op_ret) {
				vty_out(vty,"%% Port can NOT delete from vlan as port is L3 interface.\n");
			}
			else if (VLAN_RETURN_CODE_ERR_HW == op_ret) {
				vty_out(vty,"%% Error occurs in Config on HW.\n");	
			}
			else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret){
				vty_out(vty,"%% This operation is unsupported!\n");
			}
			else if(VLAN_RETURN_CODE_PROMIS_PORT_CANNOT_DEL == op_ret){
				vty_out(vty,"%% Can't del an advanced-routing interface port from this vlan\n");
			}
			else if (VLAN_RETURN_CODE_ERR_NONE != op_ret){
				vty_out(vty,"%% Unknown Error! return %d \n",op_ret);
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
		if(VLAN_RETURN_CODE_ERR_NONE != op_ret)
			return CMD_WARNING;
		else 
			return CMD_SUCCESS;
	}
}

int dcli_vlan_update_port_qinq_info
(
	struct vty* vty,
	boolean isEnable,
	unsigned short vlanid,
	unsigned char slotno,
	unsigned char portno
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int i = 0,slot_id =0,ret=0;	
   	int master_slot_id[2] = {-1, -1};	
	char *master_slot_cnt_file = "/dbm/product/master_slot_count";		
    int master_slot_count = get_product_info(master_slot_cnt_file);
	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);

   	ret = dcli_master_slot_id_get(master_slot_id);
	if(ret != 0 )
	{
		vty_out(vty,"get master_slot_id error !\n");
		return CMD_SUCCESS;		
   	}
	if((local_slot_id<0)||(master_slot_count<0))
	{
		vty_out(vty,"get get_product_info return -1 !\n");
		return CMD_SUCCESS;		
   	}
	for(i=0;i<master_slot_count;i++)
    {

		slot_id = master_slot_id[i];
    	query = NULL;
    	reply = NULL;
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_VLAN_OBJPATH,	\
											NPD_DBUS_VLAN_INTERFACE,	\
											NPD_DBUS_VLAN_METHOD_CONFIG_QINQ_UPDATE_FOR_MASTER);

		dbus_error_init(&err);

		dbus_message_append_args(	query,
							 		DBUS_TYPE_BYTE,&isEnable,
									DBUS_TYPE_BYTE,&slotno,
									DBUS_TYPE_BYTE,&portno,
							 		DBUS_TYPE_UINT16,&vlanid,
							 		DBUS_TYPE_INVALID);

		
		if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection)				
		{
			if(slot_id == local_slot_id)
			{
				reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
			}
			else 
			{	
				continue;
			}
		}
		else
		{
			reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);				
		}

		dbus_message_unref(query);
		if (NULL == reply) 
		{
			vty_out(vty,"Please check npd on MCB slot %d\n",slot_id);
    		return CMD_SUCCESS;
		}

		if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&ret,
			DBUS_TYPE_INVALID)) 
		{
			if(VLAN_RETURN_CODE_ERR_NONE != ret)
			{
				vty_out(vty,"Update qinq configuration failed ! \n");
			}
		}
		else 
		{
			vty_out(vty,"Failed get args,Please check npd on MCB slot %d\n",slot_id);
		}
		dbus_message_unref(reply);
		if(VLAN_RETURN_CODE_ERR_NONE != ret)
			return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

int dcli_vlan_qinq_allbackport_endis
(
	struct vty* vty,
	boolean isEnable,
	unsigned short vlanid
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int 	op_ret = 0;
	int slot_id =0;
	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);


	if((local_slot_id < 0) || (slot_count <0))		
	{			
		vty_out(vty,"read file error ! \n");			
		return CMD_WARNING;		
	}
	for(slot_id=1;slot_id<=slot_count;slot_id++)
	    {

        	

    		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
    											NPD_DBUS_VLAN_OBJPATH ,	\
    											NPD_DBUS_VLAN_INTERFACE ,	\
    											NPD_DBUS_VLAN_METHOD_CONFIG_ALLBACKPORT_QINQ_ENDIS );
    		
    		dbus_error_init(&err);
    		dbus_message_append_args(query,
									 DBUS_TYPE_BYTE,&isEnable,
    								 DBUS_TYPE_UINT16,&vlanid,
    								 DBUS_TYPE_INVALID);
			
            if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) 				
			{
    			if(slot_id == local_slot_id)
				{
                    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
				   	vty_out(vty,"Can not connect to slot:%d \n",slot_id);	
					continue;
				}
            }
			else
			{
                reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);				
			}
				
        	dbus_message_unref(query);
        	
        	if (NULL == reply) {
        		vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",slot_id);
        		return CMD_SUCCESS;
        	}

        	if (dbus_message_get_args ( reply, &err,
        					DBUS_TYPE_UINT32, &op_ret,
        					DBUS_TYPE_INVALID)) 
        	{
        		if(VLAN_RETURN_CODE_ERR_NONE == op_ret) {
        			vty_out(vty," set all cscd port qinq %s successfully !\n",isEnable?"enable":"disable");
        		}
				else if(VLAN_RETURN_CODE_VLAN_EXISTS == op_ret)
				{
					if(isEnable)
					{
						vty_out(vty,"all cscd port qinq are enabled already !\n");
					}
					else
					{
						vty_out(vty,"all cscd port qinq are disabled already !\n");
					}
				}
        		else 
        		{
        			vty_out(vty," set all cscd port qinq %s fail !\n",isEnable?"enable":"disable");
        		}
        	} 
        	else 
        	{
        		vty_out(vty,"% Failed get args.\n");
        		if (dbus_error_is_set(&err)) 
        		{
        			vty_out(vty,"% %s raised: %s",err.name,err.message);
        			dbus_error_free_for_dcli(&err);
        		}
        	}
        	dbus_message_unref(reply);
	    }
    	return CMD_SUCCESS;
}
int dcli_vlan_qinq_tocpuport_for_master
(
	struct vty* vty,
	boolean isEnable,
	unsigned short vlanid,
	unsigned short slot_id,
	unsigned short local_slot_id,
	unsigned short cpu_no,
	unsigned short cpu_port_no
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int 	op_ret = 0;
	int i = 0;
	unsigned short slot = 0;
	int master_slot_id[2] = {-1, -1};	
	char *master_slot_cnt_file = "/dbm/product/master_slot_count";		
    int master_slot_count = get_product_info(master_slot_cnt_file);

	
	op_ret = dcli_master_slot_id_get(master_slot_id);
	if(op_ret !=0 )
	{
		vty_out(vty,"get master_slot_id error !\n");
		return CMD_WARNING;		
   	}
	for(i=0;i<master_slot_count;i++)
    {

		slot = master_slot_id[i];
    	query = NULL;
    	reply = NULL;
    	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
    										NPD_DBUS_VLAN_OBJPATH,	\
    										NPD_DBUS_VLAN_INTERFACE,	\
    										NPD_DBUS_VLAN_METHOD_CONFIG_TOCPUPORT_QINQ_UPDATE_FOR_MASTER);
    	
    	dbus_error_init(&err);

 		dbus_message_append_args(query,
						 DBUS_TYPE_BYTE,&isEnable,
						 DBUS_TYPE_UINT16,&vlanid,
						 DBUS_TYPE_UINT16,&slot_id,
						 DBUS_TYPE_UINT16,&cpu_no,
						 DBUS_TYPE_UINT16,&cpu_port_no,
						 DBUS_TYPE_INVALID);
    	
        if(NULL == dbus_connection_dcli[slot]->dcli_dbus_connection) 				
    	{
			if(slot == local_slot_id)
			{
                reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
			}
			else 
			{	  
				continue;   /* for next MCB */
			}
        }
    	else
    	{
            reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot]->dcli_dbus_connection,query,-1, &err);				
    	}
    	
    	dbus_message_unref(query);
    	if (NULL == reply) {
    		vty_out(vty,"Please check npd on MCB slot %d\n",slot);
    		return CMD_SUCCESS;
    	}

    	if (dbus_message_get_args ( reply, &err,
    		DBUS_TYPE_UINT32,&op_ret,
    		DBUS_TYPE_INVALID)) {
    		if (VLAN_RETURN_CODE_ERR_NONE != op_ret){
    			vty_out(vty,"%% set vlan cpu port qinq fail ! return %d \n",op_ret);
    		}		
    	} 
    	else {
    		vty_out(vty,"Failed get args,Please check npd on MCB slot %d\n",slot);
    	}
    	dbus_message_unref(reply);
    	if(VLAN_RETURN_CODE_ERR_NONE != op_ret)
    	{
    		return CMD_WARNING;
    	}
    }
	return CMD_SUCCESS;
}

int dcli_vlan_add_del_port
(
	struct vty* vty,
	boolean addel,
	unsigned short vlanid,
	unsigned char  slotno, 
	unsigned char  portno,
	boolean tagged
)
{
	if(is_distributed == DISTRIBUTED_SYSTEM)	
	{
		DBusError err;
    	unsigned int 	op_ret = 0;
    	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
        if(local_slot_id<0)
        {
        	vty_out(vty,"get_product_info() return -1,Please check dbm file !\n");
    		return CMD_WARNING;			
        }
        /* 1.send to the dist slot */
    	DBusMessage *query = NULL, *reply = NULL;

    	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
    										NPD_DBUS_VLAN_OBJPATH,	\
    										NPD_DBUS_VLAN_INTERFACE,	\
    										NPD_DBUS_VLAN_METHOD_CONFIG_PORT_MEMBER_ADD_DEL);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(	query,
    							 	DBUS_TYPE_BYTE,&addel,
    								DBUS_TYPE_BYTE,&slotno,
    								DBUS_TYPE_BYTE,&portno,
    							 	DBUS_TYPE_BYTE,&tagged,
    							 	DBUS_TYPE_UINT16,&vlanid,
    							 	DBUS_TYPE_INVALID);
    	
        if(NULL == dbus_connection_dcli[slotno]->dcli_dbus_connection) 				
		{
			if(slotno == local_slot_id)
			{
                reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
			}
			else 
			{	
			   	vty_out(vty,"Can not connect to slot:%d, check the slot please !\n",slotno);	
        		return CMD_WARNING;
			}
        }
		else
		{
            reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slotno]->dcli_dbus_connection,query,-1, &err);				
		}
    	
    	dbus_message_unref(query);
    	if (NULL == reply) {
    		vty_out(vty,"failed get reply.\n");
    		if (dbus_error_is_set(&err)) {
    			printf("%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_SUCCESS;
    	}

    	if (dbus_message_get_args ( reply, &err,
    		DBUS_TYPE_UINT32,&op_ret,
    		DBUS_TYPE_INVALID)) {
    		if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret) {
            	vty_out(vty,"%% Bad parameter: Bad slot/port number.\n");
    		}/*either slot or local port No err,return NPD_DBUS_ERROR_NO_SUCH_PORT */
    		else if (VLAN_RETURN_CODE_ERR_NONE == op_ret ) {
    			/*vty_out(vty,"Ethernet Port %d-%d %s success.\n",slot_no,local_port_no,isAdd?"Add":"Delete");*/
    		} 
    		else if (VLAN_RETURN_CODE_BADPARAM == op_ret) {
    			vty_out(vty,"%% Error occurs in Parse eth-port Index or devNO.& logicPortNo.\n");
    		}
    		else if (VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret) {
    			vty_out(vty,"%% Error: Checkout-vlan to be configured NOT exists on SW.\n");
    		}
    		else if (VLAN_RETURN_CODE_PORT_EXISTS == op_ret) {
    			vty_out(vty,"%% Bad parameter: port Already member of the vlan.\n");
    		}
    		else if (VLAN_RETURN_CODE_PORT_NOTEXISTS == op_ret){
    			vty_out(vty,"%% Bad parameter: port NOT member of the vlan.\n");
    		}
    		else if (VLAN_RETURN_CODE_PORT_MBRSHIP_CONFLICT == op_ret) {
    			vty_out(vty,"%% Bad parameter: port Already untagged member of other active vlan.\n");
    		}			
    		else if (VLAN_RETURN_CODE_PORT_PROMIS_PORT_CANNOT_ADD2_VLAN == op_ret) {
    			vty_out(vty,"%% Promiscous port %d/%d can't be added to any vlan!\n",slotno,portno);
    		}
    		else if (VLAN_RETURN_CODE_PORT_PROMISCUOUS_MODE_ADD2_L3INTF == op_ret){
    			vty_out(vty,"%% Bad parameter: promiscuous mode port can't add to l3 interface!\n");
    		}
    		else if (VLAN_RETURN_CODE_PORT_TAG_CONFLICT == op_ret) {
    			vty_out(vty,"%% Bad parameter: port Tag-Mode can NOT match!\n");
    		}
    		else if (VLAN_RETURN_CODE_PORT_SUBINTF_EXISTS == op_ret){
                vty_out(vty,"%% Can't delete tag port with sub interface!\n");
    		}
    		else if (VLAN_RETURN_CODE_PORT_DEL_PROMIS_PORT_TO_DFLT_VLAN_INTF == op_ret){
    			vty_out(vty,"%% Promiscuous mode port can't delete!\n");
    		}
    		else if (VLAN_RETURN_CODE_PORT_TRUNK_MBR == op_ret) {
    			vty_out(vty,"%% Bad parameter: port is member of a active trunk!\n");
    		}
    		else if (VLAN_RETURN_CODE_ARP_STATIC_CONFLICT == op_ret) {
    			vty_out(vty,"%% Bad parameter: port has attribute of static arp!\n");
    		}
    		else if(PVLAN_RETURN_CODE_THIS_PORT_HAVE_PVE == op_ret) {
    			vty_out(vty,"%% Bad parameter: port is pvlan port!please delete pvlan first!\n");
    		}
            else if(VLAN_RETURN_CODE_HAVE_PROT_VLAN_CONFIG == op_ret) {
    			vty_out(vty,"%% There are protocol-vlan config on this vlan and port!\n");
    		}
    		else if (VLAN_RETURN_CODE_L3_INTF == op_ret) {
    			if(addel) {
    				vty_out(vty,"%% Port adds to L3 interface vlan %d.\n",vlanid);
    			}
    			else {
    				vty_out(vty,"%% L3 interface vlan member can NOT delete here.\n");
    			}
    		}
    		else if (VLAN_RETURN_CODE_PORT_L3_INTF == op_ret) {
    			if(addel) {
    				vty_out(vty,"%% Port can NOT add to vlan as port is L3 interface.\n");
    			}
    			else {
    				vty_out(vty,"%% Port can NOT delete from vlan as port is L3 interface.\n");
    			}
    		}
    		else if (VLAN_RETURN_CODE_ERR_HW == op_ret) {
    			vty_out(vty,"%% Error occurs in Config on HW.\n");	
    		}
    		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret){
    			vty_out(vty,"%% This operation is unsupported!\n");
    		}
    		else if(VLAN_RETURN_CODE_PROMIS_PORT_CANNOT_DEL == op_ret){
    			vty_out(vty,"%% Can't del an advanced-routing interface port from this vlan\n");
    		}
    		else if (VLAN_RETURN_CODE_ERR_NONE != op_ret){
    			vty_out(vty,"%% Unknown Error! return %d \n",op_ret);
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
    	if(VLAN_RETURN_CODE_ERR_NONE != op_ret)
    		return CMD_WARNING;
    	else 
    		return CMD_SUCCESS;

        #if 0    
        /* 2.send to the standby-MCB,move to out of this function */
    	query = NULL;
		reply = NULL;
		
		if(local_slot_id == master_slot_id[0])
		{
		    slot_id = master_slot_id[1];
		}
		else
		{
			slot_id = master_slot_id[0];
		}
    	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
    										NPD_DBUS_VLAN_OBJPATH,	\
    										NPD_DBUS_VLAN_INTERFACE,	\
    										NPD_DBUS_VLAN_METHOD_CONFIG_VLANLIST_PORT_MEMBER_ADD_DEL);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(	query,
    							 	DBUS_TYPE_BYTE,&addel,
    								DBUS_TYPE_BYTE,&slotno,
    								DBUS_TYPE_BYTE,&portno,
    							 	DBUS_TYPE_BYTE,&tagged,
    							 	DBUS_TYPE_UINT16,&vlanid,
    							 	DBUS_TYPE_INVALID);
    	
        if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) 				
    	{
		   	vty_out(vty,"Can not connect to MCB slot:%d !\n",slot_id);
    		return CMD_SUCCESS;				
        }
    	else
    	{
            reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);				
    	}
    	
    	dbus_message_unref(query);
    	if (NULL == reply) {
    		vty_out(vty,"Please check npd on slot %d\n",slot_id);
    		if (dbus_error_is_set(&err)) {
    			printf("%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_SUCCESS;
    	}

    	if (dbus_message_get_args ( reply, &err,
    		DBUS_TYPE_UINT32,&op_ret,
    		DBUS_TYPE_INVALID)) {
    		if (VLAN_RETURN_CODE_ERR_NONE != op_ret){
    			vty_out(vty,"%% vlan_list add/delete port Error! return %d \n",op_ret);
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
    	if(VLAN_RETURN_CODE_ERR_NONE != op_ret)
    		return CMD_WARNING;
    	else 
    		return CMD_SUCCESS;
		#endif
		
	}
	else
	{
		
    	DBusMessage *query = NULL, *reply = NULL;
    	DBusError err;
    	unsigned int 	op_ret = 0;

    	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
    										NPD_DBUS_VLAN_OBJPATH,	\
    										NPD_DBUS_VLAN_INTERFACE,	\
    										NPD_DBUS_VLAN_METHOD_CONFIG_PORT_MEMBER_ADD_DEL);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(	query,
    							 	DBUS_TYPE_BYTE,&addel,
    								DBUS_TYPE_BYTE,&slotno,
    								DBUS_TYPE_BYTE,&portno,
    							 	DBUS_TYPE_BYTE,&tagged,
    							 	DBUS_TYPE_UINT16,&vlanid,
    							 	DBUS_TYPE_INVALID);
    	
    	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
    	
    	dbus_message_unref(query);
    	if (NULL == reply) {
    		vty_out(vty,"failed get reply.\n");
    		if (dbus_error_is_set(&err)) {
    			printf("%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_SUCCESS;
    	}

    	if (dbus_message_get_args ( reply, &err,
    		DBUS_TYPE_UINT32,&op_ret,
    		DBUS_TYPE_INVALID)) {
    		if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret) {
            	vty_out(vty,"%% Bad parameter: Bad slot/port number.\n");
    		}/*either slot or local port No err,return NPD_DBUS_ERROR_NO_SUCH_PORT */
    		else if (VLAN_RETURN_CODE_ERR_NONE == op_ret ) {
    			/*vty_out(vty,"Ethernet Port %d-%d %s success.\n",slot_no,local_port_no,isAdd?"Add":"Delete");*/
    		} 
    		else if (VLAN_RETURN_CODE_BADPARAM == op_ret) {
    			vty_out(vty,"%% Error occurs in Parse eth-port Index or devNO.& logicPortNo.\n");
    		}
    		else if (VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret) {
    			vty_out(vty,"%% Error: Checkout-vlan to be configured NOT exists on SW.\n");
    		}
    		else if (VLAN_RETURN_CODE_PORT_EXISTS == op_ret) {
    			vty_out(vty,"%% Bad parameter: port Already member of the vlan.\n");
    		}
    		else if (VLAN_RETURN_CODE_PORT_NOTEXISTS == op_ret){
    			vty_out(vty,"%% Bad parameter: port NOT member of the vlan.\n");
    		}
    		else if (VLAN_RETURN_CODE_PORT_MBRSHIP_CONFLICT == op_ret) {
    			vty_out(vty,"%% Bad parameter: port Already untagged member of other active vlan.\n");
    		}			
    		else if (VLAN_RETURN_CODE_PORT_PROMIS_PORT_CANNOT_ADD2_VLAN == op_ret) {
    			vty_out(vty,"%% Promiscous port %d/%d can't be added to any vlan!\n",slotno,portno);
    		}
    		else if (VLAN_RETURN_CODE_PORT_PROMISCUOUS_MODE_ADD2_L3INTF == op_ret){
    			vty_out(vty,"%% Bad parameter: promiscuous mode port can't add to l3 interface!\n");
    		}
    		else if (VLAN_RETURN_CODE_PORT_TAG_CONFLICT == op_ret) {
    			vty_out(vty,"%% Bad parameter: port Tag-Mode can NOT match!\n");
    		}
    		else if (VLAN_RETURN_CODE_PORT_SUBINTF_EXISTS == op_ret){
                vty_out(vty,"%% Can't delete tag port with sub interface!\n");
    		}
    		else if (VLAN_RETURN_CODE_PORT_DEL_PROMIS_PORT_TO_DFLT_VLAN_INTF == op_ret){
    			vty_out(vty,"%% Promiscuous mode port can't delete!\n");
    		}
    		else if (VLAN_RETURN_CODE_PORT_TRUNK_MBR == op_ret) {
    			vty_out(vty,"%% Bad parameter: port is member of a active trunk!\n");
    		}
    		else if (VLAN_RETURN_CODE_ARP_STATIC_CONFLICT == op_ret) {
    			vty_out(vty,"%% Bad parameter: port has attribute of static arp!\n");
    		}
    		else if(PVLAN_RETURN_CODE_THIS_PORT_HAVE_PVE == op_ret) {
    			vty_out(vty,"%% Bad parameter: port is pvlan port!please delete pvlan first!\n");
    		}
            else if(VLAN_RETURN_CODE_HAVE_PROT_VLAN_CONFIG == op_ret) {
    			vty_out(vty,"%% There are protocol-vlan config on this vlan and port!\n");
    		}
    		else if (VLAN_RETURN_CODE_L3_INTF == op_ret) {
    			if(addel) {
    				vty_out(vty,"%% Port adds to L3 interface vlan %d.\n",vlanid);
    			}
    			else {
    				vty_out(vty,"%% L3 interface vlan member can NOT delete here.\n");
    			}
    		}
    		else if (VLAN_RETURN_CODE_PORT_L3_INTF == op_ret) {
    			if(addel) {
    				vty_out(vty,"%% Port can NOT add to vlan as port is L3 interface.\n");
    			}
    			else {
    				vty_out(vty,"%% Port can NOT delete from vlan as port is L3 interface.\n");
    			}
    		}
    		else if (VLAN_RETURN_CODE_ERR_HW == op_ret) {
    			vty_out(vty,"%% Error occurs in Config on HW.\n");	
    		}
    		else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret){
    			vty_out(vty,"%% This operation is unsupported!\n");
    		}
    		else if(VLAN_RETURN_CODE_PROMIS_PORT_CANNOT_DEL == op_ret){
    			vty_out(vty,"%% Can't del an advanced-routing interface port from this vlan\n");
    		}
    		else if (VLAN_RETURN_CODE_ERR_NONE != op_ret){
    			vty_out(vty,"%% Unknown Error! return %d \n",op_ret);
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
    	if(VLAN_RETURN_CODE_ERR_NONE != op_ret)
    		return CMD_WARNING;
    	else 
    		return CMD_SUCCESS;
	}
}

/**************************************
*show_vlan_member_slot_port
*Params:
*		vty - vty terminal
*		product_id - product id definition as enum product_id_e
*		portmbrBmp0 - uint bitmap
*		portmbrBmp1 - uint bitmap
*		mbrCount - the number of port
*
*Usage: show vlan membership with
*		in slot&port number.
**************************************/
int show_vlan_member_slot_port
(
	struct vty *vty,
	unsigned int product_id,
	PORT_MEMBER_BMP untagBmp,
	PORT_MEMBER_BMP tagBmp,
	unsigned int * promisPortBmp
)
{
if (is_distributed == DISTRIBUTED_SYSTEM)
{
    /* add for show vlan on AX71-2X12G12S,zhangdi 2011-05-30 */
	unsigned int i,count = 0;
	unsigned int port = 0;
    unsigned int tmpVal = 0;
	int slot = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    if(slot<0)
    {
    	vty_out(vty,"get_product_info() return -1,Please check dbm file !\n");
		return CMD_WARNING;			
    }
	
	for (i=0;i<64;i++)
	{
		port = i;
		tmpVal = (1<<((i>31) ? (i-32) : i));
		if(((i>31) ? untagBmp.portMbr[1] : untagBmp.portMbr[0]) & tmpVal)
		{	
			if(count && (0 == count % 4)) 
			{
				vty_out(vty,"\n%-32s"," ");
			}
			vty_out(vty,"%s%d/%d(u%s)",(count%4) ? ",":"",slot,port,((promisPortBmp[i/32] & tmpVal)?"*":""));
			count++;
		}
	}
	port = 0;
	tmpVal = 0;
	for (i=0;i<64;i++)
	{
		port = i;
		tmpVal = (1<<((i>31) ? (i-32) : i));
		if(((i>31) ? tagBmp.portMbr[1] : tagBmp.portMbr[0]) & tmpVal) \
		{				
			if(count && (0 == count % 4))
			{
				vty_out(vty,"\n%-32s"," ");
			}
			vty_out(vty,"%s%d/%d(t%s)",(count%4) ? ",":"",slot,port,((promisPortBmp[i/32] & tmpVal)?"*":""));
			count++;
		}
	}	
	return 0;
}
else
{
	unsigned int i,count = 0;
	unsigned int slot = 0,port = 0;
    unsigned int  tmpVal = 0;
	for (i=0;i<64;i++){
		if((PRODUCT_ID_AX7K_I == product_id) ||
			(PRODUCT_ID_AX7K == product_id)) {
			slot = i/8 + 1;
			port = i%8;
		}
		else if((PRODUCT_ID_AX5K == product_id) ||
				(PRODUCT_ID_AU4K == product_id) ||
				(PRODUCT_ID_AU3K == product_id) ||
				(PRODUCT_ID_AU3K_BCM == product_id) ||
				(PRODUCT_ID_AU3K_BCAT == product_id) || 
				(PRODUCT_ID_AU2K_TCAT == product_id)) {
			if(PRODUCT_ID_AU3K_BCAT == product_id){
				slot = 2;
			}
			else
			{
			    slot = 1;
			}
			port = i;
		}
		else if((PRODUCT_ID_AX5K_I == product_id)){
			slot = i/8;
			port = i%8 + 1;
		}

		tmpVal = (1<<((i>31) ? (i-32) : i));
		if(((i>31) ? untagBmp.portMbr[1] : untagBmp.portMbr[0]) & tmpVal) {	
			if(count && (0 == count % 4)) {
				vty_out(vty,"\n%-32s"," ");
			}
			if(PRODUCT_ID_AX7K_I == product_id) {
				vty_out(vty,"%scscd%d(u%s)",(count%4) ? ",":"", port-1 ,((promisPortBmp[i/32] & tmpVal)?"*":""));
			}
			else {
				vty_out(vty,"%s%d/%d(u%s)",(count%4) ? ",":"",slot,port,((promisPortBmp[i/32] & tmpVal)?"*":""));
			}
			count++;
		}
	}
	slot = 0;
	port = 0;
	tmpVal = 0;
	for (i=0;i<64;i++){
		if((PRODUCT_ID_AX7K_I == product_id) || 
			(PRODUCT_ID_AX7K == product_id)) {
			slot = i/8 + 1;
			port = i%8;
		}
		else if((PRODUCT_ID_AX5K == product_id) ||
				(PRODUCT_ID_AU4K == product_id) ||
				(PRODUCT_ID_AU3K == product_id) ||
				(PRODUCT_ID_AU3K_BCM == product_id) ||
				(PRODUCT_ID_AU3K_BCAT == product_id) || 
				(PRODUCT_ID_AU2K_TCAT == product_id)){
			if(PRODUCT_ID_AU3K_BCAT == product_id){
				slot = 2;
			}
			else
			{
			    slot = 1;
			}
			port = i;
		}
		else if((PRODUCT_ID_AX5K_I == product_id)){
			slot = i/8;
			port = i%8 + 1;
		}

		tmpVal = (1<<((i>31) ? (i-32) : i));
		if(((i>31) ? tagBmp.portMbr[1] : tagBmp.portMbr[0]) & tmpVal) {				
			if(count && (0 == count % 4)) {
				vty_out(vty,"\n%-32s"," ");
			}
			if(PRODUCT_ID_AX7K_I == product_id) {
				vty_out(vty,"%scscd%d(t%s)",(count%4) ? ",":"", port-1 ,((promisPortBmp[i/32] & tmpVal)?"*":""));
			}
			else {
				vty_out(vty,"%s%d/%d(t%s)",(count%4) ? ",":"",slot,port,((promisPortBmp[i/32] & tmpVal)?"*":""));
			}
			count++;
		}
	}	
	return 0;	
}
}

#ifndef AX_PLATFORM_DISTRIBUTED
/* update the gvlanlist[] on 2 MCBs */
int dcli_vlanlist_add_del_port
(
	struct vty* vty,
	boolean addel,
	unsigned short vlanid,
	unsigned char  slotno, 
	unsigned char  portno,
	boolean tagged
)
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
		return CMD_SUCCESS;		
   	}
	if((local_slot_id<0)||(master_slot_count<0))
	{
		vty_out(vty,"get get_product_info return -1 !\n");
		return CMD_SUCCESS;		
   	}
	#if 0
	/* Check is master board or not */
	if( (local_slot_id !=master_slot_id[0])&&(local_slot_id !=master_slot_id[1]) )
	{
		master_slot_count = 1;      /* loop one times */
		master_slot_id[0] = local_slot_id;  /* config for not master board */
	}
	#endif
    for(i=0;i<master_slot_count;i++)
    {

		slot_id = master_slot_id[i];
    	query = NULL;
    	reply = NULL;
    	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
    										NPD_DBUS_VLAN_OBJPATH,	\
    										NPD_DBUS_VLAN_INTERFACE,	\
    										NPD_DBUS_VLAN_METHOD_CONFIG_VLANLIST_PORT_MEMBER_ADD_DEL);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(	query,
    							 	DBUS_TYPE_BYTE,&addel,
    								DBUS_TYPE_BYTE,&slotno,
    								DBUS_TYPE_BYTE,&portno,
    							 	DBUS_TYPE_BYTE,&tagged,
    							 	DBUS_TYPE_UINT16,&vlanid,
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
				#if 0  
			   	vty_out(vty,"Can not connect to MCB slot:%d \n",slot_id);	
                #endif    
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
    		if (VLAN_RETURN_CODE_ERR_NONE != op_ret){
    			vty_out(vty,"%% vlan_list add/delete port Error! return %d \n",op_ret);
    		}		
    	} 
    	else {
    		vty_out(vty,"Failed get args,Please check npd on MCB slot %d\n",slot_id);
    	}
    	dbus_message_unref(reply);
    	if(VLAN_RETURN_CODE_ERR_NONE != op_ret)
    	{
    		return CMD_WARNING;
    	}
    }
	return CMD_SUCCESS;	
}

/***********************************
* Bond vlan to cpu port on special slot 
* Params:
*		vlan ID:	 <2-4093>
*		slot :	     <1-16>
* Usage: bond vlan <2-4093> to slot <1-16>
************************************/
DEFUN(bond_vlan_to_cpu_port_cmd_func,
	bond_vlan_to_cpu_port_cmd,
	"(bond|unbond) vlan <2-4093> slot <1-16>",
	"bond vlan to cpu port on special slot\n"
	"unbond vlan to cpu port on special slot\n"
	"Vlan entity\n"
	"Specify vlan id for vlan entity,valid range <2-4093>\n"
	"Slot id on system\n"
	"Specify slot id, range <1-16>\n"
)
{

	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int ret = 0;
	unsigned int op_ret = 0;
	
	boolean isbond = FALSE;
	boolean check_ve_exit = FALSE;
	unsigned short 	vlanId = 0;
	unsigned short 	cpu_no = 0, cpu_port_no = 0;
	unsigned short slot_id =0, slot = 0;
	int function_type = -1;
	char file_path[64] = {0};	
	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);

    if((local_slot_id<0)||(slot_count<0))
    {
    	vty_out(vty,"get_product_info() return -1,Please check dbm file !\n");
		return CMD_WARNING;			
    }
	
    /* bond or unbond */
	if(0 == strncmp(argv[0],"bond",strlen(argv[0]))) {
		isbond = TRUE;
	}
	else if (0 == strncmp(argv[0],"unbond",strlen(argv[0]))) {
		isbond= FALSE;
	}
	else {
		vty_out(vty,"% Bad parameter!\n");
		return CMD_WARNING;
	}

	/*get vlan ID*/
	ret = parse_vlan_no((char*)argv[1],&vlanId);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"% Bad parameter,vlan id illegal!\n");
		return CMD_WARNING;
	}
    /*get dist slot */
	ret = parse_single_param_no((char*)argv[2],&slot_id);
	if(NPD_SUCCESS != ret) {
		vty_out(vty,"%% parse param failed!\n");
		return CMD_WARNING;
	}
    if(slot_id > slot_count)
    {
    	vty_out(vty,"% Bad parameter,slot id illegal!\n");
		return CMD_WARNING;		
    }

	/* bond vlan 100 slot 1, is default to cpu index 0 port index 0 */
	cpu_no = 0;
    cpu_port_no = 0;
	
    /* Check if the dist slot is exist */
   	if(slot_id != local_slot_id)
   	{
        if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection)
        {
		    vty_out(vty,"Can not connect to slot %d, please check! \n",slot_id);	
		    return CMD_SUCCESS;			
        }		
   	}
    
    /* Check if the dist slot is AC board */
	sprintf(file_path,"/dbm/product/slot/slot%d/function_type", slot_id);
	function_type = get_product_info(file_path);	
	if((function_type&AC_BOARD)!=AC_BOARD)
	{
	    vty_out(vty,"% Bad parameter, the dist slot is not AC board !\n");
		return CMD_WARNING;
	}

	vty_out(vty,"%s vlan %d to slot %d:\n",(isbond==1)?"Bond":"Unbond",vlanId,slot_id);	

   	/* if unbond, send to dist slot before unbond, check interface ve exist or not. */	
    if(isbond == 0)
    {
        query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
        									NPD_DBUS_VLAN_OBJPATH ,	\
        									NPD_DBUS_VLAN_INTERFACE ,	\
        									NPD_DBUS_VLAN_EXIST_INTERFACE_UNDER_VLAN_TO_SLOT_CPU);
        	
        dbus_error_init(&err);

        dbus_message_append_args(query,
        						DBUS_TYPE_UINT16,&vlanId,
        						DBUS_TYPE_UINT16,&slot_id,
						        DBUS_TYPE_UINT16,&cpu_no,
        						DBUS_TYPE_UINT16,&cpu_port_no,
        						DBUS_TYPE_INVALID);

        if(NULL == dbus_connection_dcli[local_slot_id]->dcli_dbus_connection) 				
    	{
    		if(NULL == dcli_dbus_connection)
    		{
			   	vty_out(vty,"Can not connect to slot:%d \n",local_slot_id);
        		return CMD_WARNING;						
    		}
    		else 
			{	
                reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);		
			}	
        }
    	else
    	{
            reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[local_slot_id]->dcli_dbus_connection,query,-1, &err);				
    	}
        		
        dbus_message_unref(query);
        	
        if (NULL == reply) {
           	vty_out(vty,"Dbus reply==NULL, Please check slot: %d\n",local_slot_id);
        	return CMD_WARNING;
        }

        if (dbus_message_get_args ( reply, &err,DBUS_TYPE_UINT32, &op_ret,DBUS_TYPE_INVALID)) 
        {
			/* ve sub-intf is exist */
    		if(VLAN_RETURN_CODE_ERR_NONE == op_ret) 
            {
                vty_out(vty,"ve%02d%c%d.%d is exist,no interface first!\n",slot_id,(cpu_no == 0)?'f':'s',(cpu_port_no+1),vlanId);			
        		return CMD_WARNING;        	
			}
			else if(VLAN_RETURN_CODE_ERR_HW == op_ret)  /* ve sub-intf is not exist */
			{
                /*NULL;*/
			}			
			else if(VLAN_RETURN_CODE_VLAN_NOT_BONDED == op_ret)
			{
        		vty_out(vty,"vlan %d is no bond to slot %d, need not unbond.\n",vlanId,slot_id);
        		return CMD_WARNING;				
			}
			else
			{
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
    }

	/* send to dist slot first, bond or unbond */
	{
		slot = slot_id;    /* dist slot */
     	/*once bad param,it'll NOT sed message to NPD*/
    	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
    										NPD_DBUS_VLAN_OBJPATH ,	\
    										NPD_DBUS_VLAN_INTERFACE ,	\
    										NPD_DBUS_VLAN_METHOD_BOND_VLAN_TO_SLOT_CPU);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
     							 DBUS_TYPE_BYTE,&isbond,
    							 DBUS_TYPE_UINT16,&vlanId,
    							 DBUS_TYPE_UINT16,&slot_id,
        						 DBUS_TYPE_UINT16,&cpu_no,
        						 DBUS_TYPE_UINT16,&cpu_port_no,    							 
    							 DBUS_TYPE_INVALID);
        if(NULL == dbus_connection_dcli[slot]->dcli_dbus_connection) 				
    	{
    		if(slot == local_slot_id)
    		{
                reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
    		}
    		else 
			{	
			   	vty_out(vty,"Can not connect to slot:%d \n",slot);
        		return CMD_WARNING;				
			}	
        }
    	else
    	{
            reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot]->dcli_dbus_connection,query,-1, &err);				
    	}
    		
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
       		vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",slot);
    		return CMD_WARNING;
    	}

    	if (dbus_message_get_args ( reply, &err,DBUS_TYPE_UINT32, &op_ret,DBUS_TYPE_INVALID)) 
    	{
			if(VLAN_RETURN_CODE_ERR_NONE == op_ret) 
        	{
				if(isbond == 1)
				{
           		    vty_out(vty,"Bond vlan %d to slot %d OK\n",vlanId,slot);
				}
				else
				{
					vty_out(vty,"Unbond vlan %d to slot %d OK\n",vlanId,slot);

				}					
            	/*return CMD_SUCCESS;		*/
				/* can not return, for next slot update bondinfo */
            }
        	else 
        	{					
				if(VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret)
				{
            		vty_out(vty,"vlan %d do not exists on slot %d.\n",vlanId,slot);
					return CMD_WARNING;		
				}
				else if(VLAN_RETURN_CODE_VLAN_NOT_BONDED == op_ret)
				{
            		vty_out(vty,"vlan %d is no bond to slot %d, need not unbond.\n",vlanId,slot);
					return CMD_WARNING;								
				}
				else if(VLAN_RETURN_CODE_VLAN_ALREADY_BOND == op_ret)
				{
            		vty_out(vty,"vlan %d is already bond to slot %d, can not bond!\n",vlanId,slot);						
					return CMD_WARNING;								
				}
				else if(VLAN_RETURN_CODE_VLAN_BOND_ERR == op_ret)
				{
            		vty_out(vty,"bond vlan %d to slot %d failed, add port err.\n",vlanId,slot);	
					return CMD_WARNING;								
				}
				else if(VLAN_RETURN_CODE_VLAN_UNBOND_ERR == op_ret)
				{
            		vty_out(vty,"unbond vlan %d to slot %d failed, delete port err.\n",vlanId,slot);
					return CMD_WARNING;								
				}
				else if(VLAN_RETURN_CODE_VLAN_SYNC_ERR == op_ret)
				{
            		vty_out(vty,"slot %d sync vlan info err!\n",slot);
					return CMD_WARNING;								
				}
				else if(VLAN_RETURN_CODE_ERR_HW == op_ret)
				{
            		vty_out(vty,"slot %d have not such port!\n",slot);
					return CMD_WARNING;								
				}				
				else
				{
            		vty_out(vty,"operate vlan %d slot %d err, ret %d !\n",vlanId,slot,op_ret);
					return CMD_WARNING;														
				}
        	}
		}
    	else 
    	{
    		vty_out(vty,"Failed get args from slot %d.\n",slot);
    		if (dbus_error_is_set(&err)) 
    		{
    			vty_out(vty,"%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_WARNING;					
    	}
    	dbus_message_unref(reply);
	}
	/* if dist slot is ok, send to every other slot, update the g_vlanlist[] */
    for(slot=1; slot<=slot_count; slot++)
    {
		if(slot == slot_id) /* jump the dist slot */
		{
			continue;
		}
     	/*once bad param,it'll NOT sed message to NPD*/
    	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
    										NPD_DBUS_VLAN_OBJPATH ,	\
    										NPD_DBUS_VLAN_INTERFACE ,	\
    										NPD_DBUS_VLAN_METHOD_BOND_VLAN_TO_SLOT_CPU);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
     							 DBUS_TYPE_BYTE,&isbond,
    							 DBUS_TYPE_UINT16,&vlanId,
    							 DBUS_TYPE_UINT16,&slot_id,
        						 DBUS_TYPE_UINT16,&cpu_no,
        						 DBUS_TYPE_UINT16,&cpu_port_no,    							 
    							 DBUS_TYPE_INVALID);
        if(NULL == dbus_connection_dcli[slot]->dcli_dbus_connection) 				
    	{
    		if(slot == local_slot_id)
    		{
                reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
    		}
    		else 
			{	
			   	vty_out(vty,"Can not connect to slot:%d \n",slot);	
				continue;
			}	
        }
    	else
    	{
            reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot]->dcli_dbus_connection,query,-1, &err);				
    	}
    		
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
       		vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",slot);
    		return CMD_SUCCESS;
    	}

    	if (dbus_message_get_args ( reply, &err,DBUS_TYPE_UINT32, &op_ret,DBUS_TYPE_INVALID)) 
    	{
	
    		/* (VLAN_RETURN_CODE_ERR_NONE == op_ret) */
    		if(VLAN_RETURN_CODE_ERR_NONE == op_ret) 
			{
    			/* update the bond_slot in g_vlanlist[v_index] */
				vty_out(vty,"update bondinfo of vlan %d on slot %d OK\n",vlanId,slot);
			}
    		else
    		{
           		vty_out(vty,"update bondinfo of vlan %d on slot %d error\n",vlanId,slot);
				if(VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret)
				{
            		vty_out(vty,"vlan %d do not exists on slot %d\n",vlanId,slot);								
				}
				else if(VLAN_RETURN_CODE_VLAN_SYNC_ERR == op_ret)
				{
            		vty_out(vty,"slot %d sync vlan info err!\n",vlanId,slot);
				}
				else
				{
            		vty_out(vty,"operate vlan %d slot %d err!\n",vlanId,slot);
				}
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
    }
    return CMD_SUCCESS;
}

/***********************************
* Bond vlan to master/slave cpu port on special slot 
* Params:
*		vlan ID:	 <2-4093>
*		slot :	     <1-16>
*       cpu type:        master-cpu or slave-cpu
*       cpu port:    <0-7>
* Usage: (bond|unbond) vlan <2-4093> slot <1-16> (mcpu|scpu) <0-7>
* zhangdi@autelan.com 2012-05-29
************************************/
DEFUN(bond_vlan_to_ms_cpu_port_cmd_func,
	bond_vlan_to_ms_cpu_port_cmd,
	"(bond|unbond) vlan <2-4093> slot <1-16> cpu <1-2> port <1-8>",
	"bond vlan to cpu port on special slot\n"
	"unbond vlan to cpu port on special slot\n"
	"Vlan entity\n"
	"Specify vlan id for vlan entity,valid range <2-4093>\n"
	"Slot id on system\n"
	"Specify slot id, range <1-16>\n"
	"cpu on board\n"
	"Specify cpu, range <1-2>\n"
	"cpu port of xaui or pcie\n"
	"cpu port number\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int ret = 0;
	unsigned int op_ret = 0;
	
	boolean isbond = FALSE;
	boolean check_ve_exit = FALSE;
	unsigned short 	vlanId = 0;
	unsigned short 	cpu_no = 0, cpu_port_no = 0;
	unsigned short slot_id =0, slot = 0;
	int function_type = -1;
	char file_path[64] = {0};	
	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);

    if((local_slot_id<0)||(slot_count<0))
    {
    	vty_out(vty,"get_product_info() return -1,Please check dbm file !\n");
		return CMD_WARNING;			
    }
	
    /* bond or unbond */
	if(0 == strncmp(argv[0],"bond",strlen(argv[0]))) {
		isbond = TRUE;
	}
	else if (0 == strncmp(argv[0],"unbond",strlen(argv[0]))) {
		isbond= FALSE;
	}
	else {
		vty_out(vty,"% Bad parameter!\n");
		return CMD_WARNING;
	}

	/*get vlan ID*/
	ret = parse_vlan_no((char*)argv[1],&vlanId);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"% Bad parameter,vlan id illegal!\n");
		return CMD_WARNING;
	}
    /*get dist slot */
	ret = parse_single_param_no((char*)argv[2],&slot_id);
	if(NPD_SUCCESS != ret) {
		vty_out(vty,"%% parse param failed!\n");
		return CMD_WARNING;
	}
    if(slot_id > slot_count)
    {
    	vty_out(vty,"% Bad parameter,slot id illegal!\n");
		return CMD_WARNING;		
    }

	/* get cpu no */
	ret = parse_single_param_no((char*)argv[3],&cpu_no);
	if(NPD_SUCCESS != ret) {
		vty_out(vty,"%% parse cpu_port_no param failed!\n");
		return CMD_WARNING;
	}		

	/* get cpu prot number */
	ret = parse_single_param_no((char*)argv[4],&cpu_port_no);
	if(NPD_SUCCESS != ret) {
		vty_out(vty,"%% parse cpu_port_no param failed!\n");
		return CMD_WARNING;
	}
	/* change no to index, for hardware use */
    cpu_no = cpu_no -1;
    cpu_port_no = cpu_port_no -1;
	
    /* Check if the dist slot is exist */
   	if(slot_id != local_slot_id)
   	{
        if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection)
        {
		    vty_out(vty,"Can not connect to slot %d, please check! \n",slot_id);	
		    return CMD_SUCCESS;			
        }		
   	}
    
    /* Check if the dist slot is AC board */
	sprintf(file_path,"/dbm/product/slot/slot%d/function_type", slot_id);
	function_type = get_product_info(file_path);	
	if((function_type&AC_BOARD)!=AC_BOARD)
	{
	    vty_out(vty,"% Bad parameter, the dist slot is not AC board !\n");
		return CMD_WARNING;		
	}

	vty_out(vty,"%s vlan %d to slot %d:\n",(isbond==1)?"Bond":"Unbond",vlanId,slot_id);	

   	/* if unbond, send to dist slot before unbond, check interface ve exist or not. */	
    if(isbond == 0)
    {
        query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
        									NPD_DBUS_VLAN_OBJPATH ,	\
        									NPD_DBUS_VLAN_INTERFACE ,	\
        									NPD_DBUS_VLAN_EXIST_INTERFACE_UNDER_VLAN_TO_SLOT_CPU);
        	
        dbus_error_init(&err);

        dbus_message_append_args(query,
        						DBUS_TYPE_UINT16,&vlanId,
        						DBUS_TYPE_UINT16,&slot_id,
						        DBUS_TYPE_UINT16,&cpu_no,
        						DBUS_TYPE_UINT16,&cpu_port_no,
        						DBUS_TYPE_INVALID);

        if(NULL == dbus_connection_dcli[local_slot_id]->dcli_dbus_connection) 				
    	{
    		if(NULL == dcli_dbus_connection)
    		{
			   	vty_out(vty,"Can not connect to slot:%d \n",local_slot_id);
        		return CMD_WARNING;						
    		}
    		else 
			{	
                reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);		
			}	
        }
    	else
    	{
            reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[local_slot_id]->dcli_dbus_connection,query,-1, &err);				
    	}
        		
        dbus_message_unref(query);
        	
        if (NULL == reply) {
           	vty_out(vty,"Dbus reply==NULL, Please check slot: %d\n",local_slot_id);
        	return CMD_WARNING;
        }

        if (dbus_message_get_args ( reply, &err,DBUS_TYPE_UINT32, &op_ret,DBUS_TYPE_INVALID)) 
        {
			/* ve sub-intf is exist */
    		if(VLAN_RETURN_CODE_ERR_NONE == op_ret) 
            {
                vty_out(vty,"ve%02d%c%d.%d is exist,no interface first!\n",slot_id,(cpu_no == 0)?'f':'s',(cpu_port_no+1),vlanId);			
        		return CMD_WARNING;        	
			}
			else if(VLAN_RETURN_CODE_ERR_HW == op_ret)  /* ve sub-intf is not exist */
			{
                /*NULL;*/
			}			
			else if(VLAN_RETURN_CODE_VLAN_NOT_BONDED == op_ret)
			{
        		vty_out(vty,"vlan %d is no bond to slot %d, need not unbond.\n",vlanId,slot_id);
        		return CMD_WARNING;				
			}
			else
			{
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
    }

	/* send to dist slot first, bond or unbond */
	{
		slot = slot_id;    /* dist slot */
     	/*once bad param,it'll NOT sed message to NPD*/
    	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
    										NPD_DBUS_VLAN_OBJPATH ,	\
    										NPD_DBUS_VLAN_INTERFACE ,	\
    										NPD_DBUS_VLAN_METHOD_BOND_VLAN_TO_SLOT_CPU);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
     							 DBUS_TYPE_BYTE,&isbond,
    							 DBUS_TYPE_UINT16,&vlanId,
    							 DBUS_TYPE_UINT16,&slot_id,
        						 DBUS_TYPE_UINT16,&cpu_no,
        						 DBUS_TYPE_UINT16,&cpu_port_no,    							 
    							 DBUS_TYPE_INVALID);
        if(NULL == dbus_connection_dcli[slot]->dcli_dbus_connection) 				
    	{
    		if(slot == local_slot_id)
    		{
                reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
    		}
    		else 
			{	
			   	vty_out(vty,"Can not connect to slot:%d \n",slot);
        		return CMD_WARNING;				
			}	
        }
    	else
    	{
            reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot]->dcli_dbus_connection,query,-1, &err);				
    	}
    		
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
       		vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",slot);
    		return CMD_WARNING;
    	}

    	if (dbus_message_get_args ( reply, &err,DBUS_TYPE_UINT32, &op_ret,DBUS_TYPE_INVALID)) 
    	{
			if(VLAN_RETURN_CODE_ERR_NONE == op_ret) 
        	{
				if(isbond == 1)
				{
           		    vty_out(vty,"Bond vlan %d to slot %d OK\n",vlanId,slot);
				}
				else
				{
					vty_out(vty,"Unbond vlan %d to slot %d OK\n",vlanId,slot);

				}					
            	/*return CMD_SUCCESS;		*/
				/* can not return, for next slot update bondinfo */
            }
        	else 
        	{					
				if(VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret)
				{
            		vty_out(vty,"vlan %d do not exists on slot %d.\n",vlanId,slot);
					return CMD_WARNING;		
				}
				else if(VLAN_RETURN_CODE_VLAN_NOT_BONDED == op_ret)
				{
            		vty_out(vty,"vlan %d is no bond to slot %d, need not unbond.\n",vlanId,slot);
					return CMD_WARNING;								
				}
				else if(VLAN_RETURN_CODE_VLAN_ALREADY_BOND == op_ret)
				{
            		vty_out(vty,"vlan %d is already bond to slot %d, can not bond!\n",vlanId,slot);						
					return CMD_WARNING;								
				}
				else if(VLAN_RETURN_CODE_VLAN_BOND_ERR == op_ret)
				{
            		vty_out(vty,"bond vlan %d to slot %d failed, add port err.\n",vlanId,slot);	
					return CMD_WARNING;								
				}
				else if(VLAN_RETURN_CODE_VLAN_UNBOND_ERR == op_ret)
				{
            		vty_out(vty,"unbond vlan %d to slot %d failed, delete port err.\n",vlanId,slot);
					return CMD_WARNING;								
				}
				else if(VLAN_RETURN_CODE_VLAN_SYNC_ERR == op_ret)
				{
            		vty_out(vty,"slot %d sync vlan info err!\n",vlanId,slot);
					return CMD_WARNING;								
				}
				else if(VLAN_RETURN_CODE_ERR_HW == op_ret)
				{
            		vty_out(vty,"slot %d have not such cpu or port!\n",slot);
					return CMD_WARNING;								
				}				
				else
				{
            		vty_out(vty,"operate vlan %d slot %d err, ret %d !\n",vlanId,slot,op_ret);
					return CMD_WARNING;														
				}
        	}
		}
    	else 
    	{
    		vty_out(vty,"Failed get args from slot %d.\n",slot);
    		if (dbus_error_is_set(&err)) 
    		{
    			vty_out(vty,"%s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    		return CMD_WARNING;					
    	}
    	dbus_message_unref(reply);
	}
	/* if dist slot is ok, send to every other slot, update the g_vlanlist[] */
    for(slot=1; slot<=slot_count; slot++)
    {
		if(slot == slot_id)     /* jump the dist slot */
		{
			continue;
		}
     	/*once bad param,it'll NOT sed message to NPD*/
    	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
    										NPD_DBUS_VLAN_OBJPATH ,	\
    										NPD_DBUS_VLAN_INTERFACE ,	\
    										NPD_DBUS_VLAN_METHOD_BOND_VLAN_TO_SLOT_CPU);
    	
    	dbus_error_init(&err);

    	dbus_message_append_args(query,
     							 DBUS_TYPE_BYTE,&isbond,
    							 DBUS_TYPE_UINT16,&vlanId,
    							 DBUS_TYPE_UINT16,&slot_id,
        						 DBUS_TYPE_UINT16,&cpu_no,
        						 DBUS_TYPE_UINT16,&cpu_port_no,    							 
    							 DBUS_TYPE_INVALID);
        if(NULL == dbus_connection_dcli[slot]->dcli_dbus_connection) 				
    	{
    		if(slot == local_slot_id)
    		{
                reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
    		}
    		else 
			{	
			   	vty_out(vty,"Can not connect to slot:%d \n",slot);	
				continue;
			}	
        }
    	else
    	{
            reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot]->dcli_dbus_connection,query,-1, &err);				
    	}
    		
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
       		vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",slot);
    		return CMD_SUCCESS;
    	}

    	if (dbus_message_get_args ( reply, &err,DBUS_TYPE_UINT32, &op_ret,DBUS_TYPE_INVALID)) 
    	{
	
    		/* (VLAN_RETURN_CODE_ERR_NONE == op_ret) */
    		if(VLAN_RETURN_CODE_ERR_NONE == op_ret) 
			{
    			/* update the bond_slot in g_vlanlist[v_index] */
				vty_out(vty,"update bondinfo of vlan %d on slot %d OK\n",vlanId,slot);
			}
    		else
    		{
           		vty_out(vty,"update bondinfo of vlan %d on slot %d error\n",vlanId,slot);
				if(VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret)
				{
            		vty_out(vty,"vlan %d do not exists on slot %d\n",vlanId,slot);								
				}
				else if(VLAN_RETURN_CODE_VLAN_SYNC_ERR == op_ret)
				{
            		vty_out(vty,"slot %d sync vlan info err!\n",vlanId,slot);
				}
				else
				{
            		vty_out(vty,"operate vlan %d slot %d err!\n",vlanId,slot);
				}
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
    }
    return CMD_SUCCESS;
}

/**************************************
* create more vlan entity on Both Sw & Hw. 
* Params: 
*       vlan id start no,valid range <2-4093>
*		vlan id end no(biger than start no),valid range <2-4093>
*
* Usage: create vlan <2-4093> to <2-4093>
*		default vlan can NOT be created
****************************************/
DEFUN(create_layer2_more_vlan_cmd_func,
	create_layer2_more_vlan_cmd,
	"create vlanlist <2-4093> to <2-4093>",
	"create system manageable entity\n"
	"create more layer 2 vlan entity\n"
	"vlan id start no,valid range <2-4093>\n"
	"vlan id end no(biger than start no),valid range <2-4093>\n"
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned short 	start_vid = 0;
	unsigned short 	end_vid = 0;
	unsigned short 	vlanId = 0;	
   	char name[21] = {0};
   	char *vlanName = NULL;
	unsigned int nameSize = 0;
	int ret = 0;
	unsigned int op_ret = 0;

	int slot_id =0;
	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);

    if((local_slot_id<0)||(slot_count<0))
    {
    	vty_out(vty,"get_product_info() return -1,Please check dbm file !\n");
		return CMD_WARNING;			
    }
	
	/*get start vlan ID*/
	ret = parse_vlan_no((char*)argv[0],&start_vid);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"% Bad parameter,start vlan id illegal!\n");
		return CMD_WARNING;
	}

	/*get end vlan ID*/
	ret = parse_vlan_no((char*)argv[1],&end_vid);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"% Bad parameter,end vlan id illegal!\n");
		return CMD_WARNING;
	}

	if(start_vid>end_vid)
	{		
    	vty_out(vty,"% Bad parameter,start_no can not big than end_no\n");
		return CMD_WARNING;		
	}

	/*malloc for vlan name*/
	vlanName = (char*)malloc(ALIAS_NAME_SIZE);
	memset(vlanName,0,ALIAS_NAME_SIZE);
		
    for(start_vid;start_vid<=end_vid;start_vid++)
    {
    	sprintf(name,"vlan%d", start_vid);				
		nameSize = strlen(name);
		memcpy(vlanName,name,nameSize);
        vlanId = start_vid;
	    for(slot_id=1;slot_id<=slot_count;slot_id++)
	    {
        	
    		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
    											NPD_DBUS_VLAN_OBJPATH ,	\
    											NPD_DBUS_VLAN_INTERFACE ,	\
    											NPD_DBUS_VLAN_METHOD_CREATE_VLAN_ONE );
    		
    		dbus_error_init(&err);
    		dbus_message_append_args(query,
    								 DBUS_TYPE_UINT16,&vlanId,
    								 DBUS_TYPE_STRING,&vlanName, 
    								 DBUS_TYPE_INVALID);
			
            if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) 				
			{
    			if(slot_id == local_slot_id)
				{
                    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
				   	vty_out(vty,"Can not connect to slot:%d \n",slot_id);
					continue;
				}
            }
			else
			{
                reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);				
			}
        	dbus_message_unref(query);
        	
        	if (NULL == reply) {
        		vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",slot_id);
        		return CMD_SUCCESS;
        	}

        	if (dbus_message_get_args ( reply, &err,
        					DBUS_TYPE_UINT32, &op_ret,
        					DBUS_TYPE_INVALID)) 
        	{
        		if(VLAN_RETURN_CODE_ERR_NONE == op_ret) {
        			/*vty_out(vty,"% Create vlan success,query reply op_ret %d .\n",op_ret);*/
        		}
        		else if (ETHPORT_RETURN_CODE_NO_SUCH_VLAN == op_ret) {
                	vty_out(vty,"%% ERROR:Illegal Vlan ID %d.\n",vlanId);
        		}
        		else if(VLAN_RETURN_CODE_VLAN_EXISTS == op_ret) {
        			vty_out(vty,"% Vlan %d Already Exists on slot %d.\n",vlanId,slot_id);
        		}
        		else if(VLAN_RETURN_CODE_NAME_CONFLICT == op_ret) {
        			vty_out(vty,"% Vlan name %s conflict.\n",vlanName);
        		}
        		else if(VLAN_RETURN_CODE_ERR_HW == op_ret) {
        			vty_out(vty,"% Error occurs in Config vlan %d on HW.\n",vlanId);
        		}
        		else if(VLAN_RETURN_CODE_ERR_GENERAL == op_ret) {
        			vty_out(vty,"& Error occurs in config vlan %d on SW.\n",vlanId);
        		}
        		else if(VLAN_RETURN_CODE_VLAN_CREATE_NOT_ALLOWED == op_ret) {
        			vty_out(vty,"% Vlan %d could not be created.\n",vlanId);
        		}
        		else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
        			vty_out(vty,"%% Product not support this function!\n");
        		}
        	} 
        	else 
        	{
        		vty_out(vty,"% Failed get args.\n");
        		if (dbus_error_is_set(&err)) 
        		{
        			vty_out(vty,"% %s raised: %s",err.name,err.message);
        			dbus_error_free_for_dcli(&err);
        		}
        	}
	    }
     	vty_out(vty,"Creat vlan%d OK.\n",start_vid);
    }
	dbus_message_unref(reply);
  	free(vlanName);	
    return CMD_SUCCESS;
}	

/**************************************
* delete more vlan entity on Both Sw & Hw. 
* Params: 
*       vlan id start no,valid range <2-4093>
*		vlan id end no(biger than start no),valid range <2-4093>
*
* Usage: delete vlan <2-4093> to <2-4093>
*		default vlan can NOT be delete
****************************************/
DEFUN(delete_layer2_more_vlan_cmd_func,
	delete_layer2_more_vlan_cmd,
	"delete vlanlist <2-4093> to <2-4093>",
	"delete system manageable entity\n"
	"delete more layer 2 vlan entity\n"
	"vlan id start no,valid range <2-4093>\n"
	"vlan id end no(biger than start no),valid range <2-4093>\n"
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned short 	start_vid = 0;
	unsigned short 	end_vid = 0;
	unsigned short 	vlanId = 0;	
	int ret = 0;
	unsigned int op_ret = 0;

	int slot_id =0;
	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);

    if((local_slot_id<0)||(slot_count<0))
    {
    	vty_out(vty,"get_product_info() return -1,Please check dbm file !\n");
		return CMD_WARNING;			
    }
	
	/*get start vlan ID*/
	ret = parse_vlan_no((char*)argv[0],&start_vid);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"% Bad parameter,start vlan id illegal!\n");
		return CMD_WARNING;
	}

	/*get end vlan ID*/
	ret = parse_vlan_no((char*)argv[1],&end_vid);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"% Bad parameter,end vlan id illegal!\n");
		return CMD_WARNING;
	}

	if(start_vid>end_vid)
	{
		
    	vty_out(vty,"% Bad parameter,start_no can not big than end_no\n");
		return CMD_WARNING;
		
	}
		
    for(start_vid;start_vid<=end_vid;start_vid++)
    {
		vlanId = start_vid;
	    for(slot_id=1;slot_id<=slot_count;slot_id++)
	    {
        	
    		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
    											NPD_DBUS_VLAN_OBJPATH ,	\
    											NPD_DBUS_VLAN_INTERFACE ,	\
    											NPD_DBUS_VLAN_METHOD_CONFIG_DELETE_VLAN_ENTRY );
    		
    		dbus_error_init(&err);

    		dbus_message_append_args(query,
    								DBUS_TYPE_UINT16,&vlanId,
    								DBUS_TYPE_INVALID);
			/* send the dbus msg to any other board */
            if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) 				
			{
    			if(slot_id == local_slot_id)
				{
                    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
				   	vty_out(vty,"Can not connect to slot:%d \n",slot_id);	
					continue;
				}
            }
			else
			{
                reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);				
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
        					DBUS_TYPE_UINT32, &op_ret,
        					DBUS_TYPE_INVALID)) 
        	{
        		if (ETHPORT_RETURN_CODE_NO_SUCH_VLAN == op_ret) {
        			vty_out(vty,"%% Bad Parameter:vlan id illegal.\n");
        		}
        		else if (VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret) {
                	vty_out(vty,"%% Bad Parameter:vlan %d NOT exists on slot %d.\n",vlanId,slot_id);
        		}
        		else if (VLAN_RETURN_CODE_BADPARAM == op_ret) {
        			vty_out(vty,"%% Bad Parameter:Can NOT delete Default vlan.\n");
        		}
        		else if (VLAN_RETURN_CODE_ERR_HW == op_ret) {
        			vty_out(vty,"%% Error occurs in config on hardware.\n");
        		}
        		else if (VLAN_RETURN_CODE_L3_INTF == op_ret) {
        			vty_out(vty,"%% Bad Parameter:Can NOT delete Layer3 Interface vlan.\n");
        		}
        		else if (VLAN_RETURN_CODE_SUBINTF_EXISTS == op_ret){
					#if 0  /* Advanced-routing not use in distributed system */
                    vty_out(vty,"%% Bad Parameter:Can NOT delete Advanced-routing Interface vlan.\n");
					#endif
					/* vlan is bonded, can not delete */
                    vty_out(vty,"slot%d: vlan%d has been bonded, Please unbond it first !\n",slot_id,vlanId);				
        		}
        		else if (VLAN_RETURN_CODE_PORT_SUBINTF_EXISTS == op_ret){
                    vty_out(vty,"%% Can NOT delete vlan with port sub interface.\n");
        		}
                else if(VLAN_RETURN_CODE_HAVE_PROT_VLAN_CONFIG== op_ret){
                    vty_out(vty,"%% There is protocol-vlan config on this vlan,please delete it first!\n");
                }
        		else if(VLAN_RETURN_CODE_ERR_NONE == op_ret) {
        			/*vty_out(vty,"Delete vlan OK.\n");*/
        		}
        		else{
                    vty_out(vty,"%% Unknown error occured when delete vlan.\n");
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
	    }
     	vty_out(vty,"Delete vlan%d OK.\n",start_vid);
    }
	dbus_message_unref(reply);
    return CMD_SUCCESS;
}	

/***********************************
*show corrent vlan.
*Execute at Vlan-config Node
*CMD : show vlan port
*show port members of corrent vlan 
***********************************/
DEFUN(show_vlan_list_count_cmd_func,
	show_vlan_list_count_cmd,
	"show vlan list count",
	"Show system info\n"
	"Layer2 vlan entity\n"
	"list of vlan\n"
	"amount of all vlan\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	
	DBusMessageIter	 iter_array;
	unsigned short	vlanId = 0;
	unsigned int	i = 0, count = 0, ret = 0;
		
	int fd = -1;
	struct stat sb;

	vlan_list_t * mem_vlan_list =NULL;
	char* file_path = "/dbm/shm/vlan/shm_vlan";
	/* only read file */
    fd = open(file_path, O_RDONLY);
	if(fd < 0)
    {
        vty_out(vty,"Failed to open! %s\n", strerror(errno));
        return NULL;
    }
	fstat(fd,&sb);
	/* map not share */	
    mem_vlan_list = (vlan_list_t *)mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0 );
    if(MAP_FAILED == mem_vlan_list)
    {
        vty_out(vty,"Failed to mmap for g_vlanlist[]! %s\n", strerror(errno));
		close(fd);
        return NULL;
    }	

    for(i=1;i<=4093;i++)
    {
        if(mem_vlan_list[i-1].vlanStat == 1)
        {
			count++;
        }
	    else
		{
            /* do nothing */
            continue;
		}
    }

   	vty_out(vty,"========================================================================\n",vlanId);
   	vty_out(vty,"Count of all vlan: %d \n",count);
   	vty_out(vty,"========================================================================\n",vlanId);
	
	/* munmap and close fd */
    ret = munmap(mem_vlan_list,sb.st_size);
    if( ret != 0 )
    {
        vty_out(vty,"Failed to munmap for g_vlanlist[]! %s\n", strerror(errno));			
    }	
	ret = close(fd);
	if( ret != 0 )
    {
        vty_out(vty,"close shm_vlan failed \n" );   
    }
    /*free(vlanName);*/
    return CMD_SUCCESS;				
}

#endif
/**************************************
* create vlan entity on Both Sw & Hw. 
* Params: 
*       vlan ID:	<2-4094>
*		vlan name:  such as "autelan0","autelan1","_office"
*
* Usage: create vlan <2-4094> VLANNAME 
*		default vlan can NOT be created
****************************************/
DEFUN(create_layer2_vlan_func,
	create_layer2_vlan_cmd,
	"create vlan <2-4093> VLANNMAE",/*"create vlan <2-4094> [vlanname]"*/
	"create system manageable entity\n"
	"create layer 2 vlan entity\n"
	"Specify vlan id for vlan entity,valid range <2-4093>\n"
	"Specify vlan name,begins with char or'_', and no more than 20 characters\n"
)
{
	if(is_distributed == DISTRIBUTED_SYSTEM)	
    {   	
    	DBusMessage *query = NULL, *reply = NULL;
    	DBusError err;

    	unsigned short 	vlanId = 0;
    	char *vlanName = NULL;
    	unsigned int nameSize = 0;
    	int ret = 0;
    	unsigned int op_ret = 0;

    	int slot_id =0;
    	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
        int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);
        int product_serial = get_product_info("/dbm/product/product_serial");
        if((local_slot_id<0)||(slot_count<0))
        {
        	vty_out(vty,"get_product_info() return -1,Please check dbm file !\n");
    		return CMD_WARNING;			
        }
		
    	/*get vlan ID*/
    	ret = parse_vlan_no((char*)argv[0],&vlanId);
    	if (NPD_FAIL == ret) {
        	vty_out(vty,"% Bad parameter,vlan id illegal!\n");
    		return CMD_WARNING;
    	}
    	if (4095 == vlanId) {
    		vty_out(vty,"% Bad parameter,Reserved vlan for Layer3 interface of EthPort!\n");
    		return CMD_WARNING;
    	}
		/* vlan 4094 do not support config */
    	if (4094 == vlanId){
    		vty_out(vty,"% Bad parameter,Reserved vlan for internal use!\n");
    		return CMD_WARNING;
    	}

        /* Not support vlan 4093 on AX7605i, it's for RPA zhangdi@autelan.com 2012-10-09 */
		if ((product_serial == 7)&&(vlanId == 4093))
		{
    		vty_out(vty,"% Vlan %d is reserved on this Product.\n",vlanId);
    		return CMD_WARNING;
    	}
		
	    for(slot_id=1;slot_id<=slot_count;slot_id++)
	    {

        	/*get vlan name*/
        	vlanName = (char*)malloc(ALIAS_NAME_SIZE);
        	memset(vlanName,0,ALIAS_NAME_SIZE);

        	/*"list" will conflict with the CLI of show vlan list*/
        	if(strcmp((char*)argv[1],"list")==0){
        		vty_out(vty,"Please enter another name for this vlan.\n");
        		free(vlanName);
        		return CMD_WARNING;
        	}
        	
        	ret = vlan_name_legal_check((char*)argv[1],strlen(argv[1]));
        	if(ALIAS_NAME_LEN_ERROR == ret) {
        		vty_out(vty,"% Bad parameter,vlan name too long!\n");
        		free(vlanName);
        		return CMD_WARNING;
        	}
        	else if(ALIAS_NAME_HEAD_ERROR == ret) {
        		vty_out(vty,"% Bad parameter,vlan name begins with an illegal char!\n");
        		free(vlanName);
        		return CMD_WARNING;
        	}
        	else if(ALIAS_NAME_BODY_ERROR == ret) {
        		vty_out(vty,"% Bad parameter,vlan name contains illegal char!\n");
        		free(vlanName);
        		return CMD_WARNING;
        	}
        	else {
        		/*once bad param appears,dbus message'll NOT send from DCLI*/
        		nameSize = strlen(argv[1]);
        		memcpy(vlanName,argv[1],nameSize);

        		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
        											NPD_DBUS_VLAN_OBJPATH ,	\
        											NPD_DBUS_VLAN_INTERFACE ,	\
        											NPD_DBUS_VLAN_METHOD_CREATE_VLAN_ONE );
        		
        		dbus_error_init(&err);
        		dbus_message_append_args(query,
        								 DBUS_TYPE_UINT16,&vlanId,
        								 DBUS_TYPE_STRING,&vlanName, 
        								 DBUS_TYPE_INVALID);
    			
                if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) 				
    			{
        			if(slot_id == local_slot_id)
    				{
                        reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
    				}
    				else 
    				{	
    				   	vty_out(vty,"Can not connect to slot:%d \n",slot_id);	
    					continue;
    				}
                }
    			else
    			{
                    reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);				
    			}
        	}
        	dbus_message_unref(query);
        	
        	if (NULL == reply) {
        		vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",slot_id);
        		return CMD_SUCCESS;
        	}

        	if (dbus_message_get_args ( reply, &err,
        					DBUS_TYPE_UINT32, &op_ret,
        					DBUS_TYPE_INVALID)) 
        	{
        		if(VLAN_RETURN_CODE_ERR_NONE == op_ret) {
        			/*vty_out(vty,"% Create vlan success,query reply op_ret %d .\n",op_ret);*/
        		}
        		else if (ETHPORT_RETURN_CODE_NO_SUCH_VLAN == op_ret) {
                	vty_out(vty,"%% ERROR:Illegal Vlan ID %d.\n",vlanId);
        		}
        		else if(VLAN_RETURN_CODE_VLAN_EXISTS == op_ret) {
        			vty_out(vty,"% Vlan %d Already Exists on slot %d.\n",vlanId,slot_id);
        		}
        		else if(VLAN_RETURN_CODE_NAME_CONFLICT == op_ret) {
        			vty_out(vty,"% Vlan name %s conflict.\n",vlanName);
        		}
        		else if(VLAN_RETURN_CODE_ERR_HW == op_ret) {
        			vty_out(vty,"% Error occurs in Config vlan %d on HW.\n",vlanId);
        		}
        		else if(VLAN_RETURN_CODE_ERR_GENERAL == op_ret) {
        			vty_out(vty,"& Error occurs in config vlan %d on SW.\n",vlanId);
        		}
        		else if(VLAN_RETURN_CODE_VLAN_CREATE_NOT_ALLOWED == op_ret) {
        			vty_out(vty,"% Vlan %d could not be created.\n",vlanId);
        		}
        		else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
        			vty_out(vty,"%% Product not support this function!\n");
        		}
        	} 
        	else 
        	{
        		vty_out(vty,"% Failed get args.\n");
        		if (dbus_error_is_set(&err)) 
        		{
        			vty_out(vty,"% %s raised: %s",err.name,err.message);
        			dbus_error_free_for_dcli(&err);
        		}
        	}
        	dbus_message_unref(reply);
        	free(vlanName);
	    }
    	return CMD_SUCCESS;
    }	
	else
    {
    	
    	DBusMessage *query = NULL, *reply = NULL;
    	DBusError err;

    	unsigned short 	vlanId = 0;
    	char *vlanName = NULL;
    	unsigned int nameSize = 0;
    	int ret = 0;
    	unsigned int op_ret = 0;
    	
    	/*get vlan ID*/
    	ret = parse_vlan_no((char*)argv[0],&vlanId);
    	if (NPD_FAIL == ret) {
        	vty_out(vty,"% Bad parameter,vlan id illegal!\n");
    		return CMD_WARNING;
    	}
    	if (4095 == vlanId) {
    		vty_out(vty,"% Bad parameter,Reserved vlan for Layer3 interface of EthPort!\n");
    		return CMD_WARNING;
    	}

    	/*get vlan name*/
    	vlanName = (char*)malloc(ALIAS_NAME_SIZE);
    	memset(vlanName,0,ALIAS_NAME_SIZE);

    	/*"list" will conflict with the CLI of show vlan list*/
    	if(strcmp((char*)argv[1],"list")==0){
    		vty_out(vty,"Please enter another name for this vlan.\n");
    		free(vlanName);
    		return CMD_WARNING;
    	}
    	
    	ret = vlan_name_legal_check((char*)argv[1],strlen(argv[1]));
    	if(ALIAS_NAME_LEN_ERROR == ret) {
    		vty_out(vty,"% Bad parameter,vlan name too long!\n");
    		free(vlanName);
    		return CMD_WARNING;
    	}
    	else if(ALIAS_NAME_HEAD_ERROR == ret) {
    		vty_out(vty,"% Bad parameter,vlan name begins with an illegal char!\n");
    		free(vlanName);
    		return CMD_WARNING;
    	}
    	else if(ALIAS_NAME_BODY_ERROR == ret) {
    		vty_out(vty,"% Bad parameter,vlan name contains illegal char!\n");
    		free(vlanName);
    		return CMD_WARNING;
    	}
    	else {
    		/*once bad param appears,dbus message'll NOT send from DCLI*/
    		nameSize = strlen(argv[1]);
    		memcpy(vlanName,argv[1],nameSize);

    		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
    											NPD_DBUS_VLAN_OBJPATH ,	\
    											NPD_DBUS_VLAN_INTERFACE ,	\
    											NPD_DBUS_VLAN_METHOD_CREATE_VLAN_ONE );
    		
    		dbus_error_init(&err);
    		dbus_message_append_args(query,
    								 DBUS_TYPE_UINT16,&vlanId,
    								 DBUS_TYPE_STRING,&vlanName, 
    								 DBUS_TYPE_INVALID);

    		/*printf("build query for vlanId %d vlan name %s\n",vlanId,vlanName);*/
    		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
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
    					DBUS_TYPE_UINT32, &op_ret,
    					DBUS_TYPE_INVALID)) 
    	{
    		if(VLAN_RETURN_CODE_ERR_NONE == op_ret) {
    			/*vty_out(vty,"% Create vlan success,query reply op_ret %d .\n",op_ret);*/
    		}
    		else if (ETHPORT_RETURN_CODE_NO_SUCH_VLAN == op_ret) {
            	vty_out(vty,"%% ERROR:Illegal Vlan ID %d.\n",vlanId);
    		}
    		else if(VLAN_RETURN_CODE_VLAN_EXISTS == op_ret) {
    			vty_out(vty,"% Vlan %d Already Exists.\n",vlanId);
    		}
    		else if(VLAN_RETURN_CODE_NAME_CONFLICT == op_ret) {
    			vty_out(vty,"% Vlan name %s conflict.\n",vlanName);
    		}
    		else if(VLAN_RETURN_CODE_ERR_HW == op_ret) {
    			vty_out(vty,"% Error occurs in Config vlan %d on HW.\n",vlanId);
    		}
    		else if(VLAN_RETURN_CODE_ERR_GENERAL == op_ret) {
    			vty_out(vty,"& Error occurs in config vlan %d on SW.\n",vlanId);
    		}
    		else if(VLAN_RETURN_CODE_VLAN_CREATE_NOT_ALLOWED == op_ret) {
    			vty_out(vty,"% Vlan %d could not be created.\n",vlanId);
    		}
    		else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
    			vty_out(vty,"%% Product not support this function!\n");
    		}
    	} 
    	else 
    	{
    		vty_out(vty,"% Failed get args.\n");
    		if (dbus_error_is_set(&err)) 
    		{
    			vty_out(vty,"% %s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    	}
    	dbus_message_unref(reply);
    	free(vlanName);
    	return CMD_SUCCESS;
    }
}
/***********************************
* Enter vlan config mode to configure 
* special vlan.
* Params:
*		vlan ID:	 <2-4095>
* Usage: config vlan <2-4095>
************************************/

DEFUN(config_layer2_vlan_func,
	config_layer2_vlan_cmd,
	"config vlan <2-4093>",
	CONFIG_STR
	"Config layer 2 vlan entity\n"
	"Specify vlan id for vlan entity\n"
)
{
	if(is_distributed == DISTRIBUTED_SYSTEM)	
	{
    	DBusMessage *query = NULL, *reply = NULL;
    	DBusError err;

    	unsigned short 	vlanId = 0;
    	int ret = 0;
    	unsigned int op_ret = 0, nodeSave = 0;

    	int slot_id =0;
    	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
        int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);
        if((local_slot_id<0)||(slot_count<0))
        {
        	vty_out(vty,"get_product_info() return -1,Please check dbm file !\n");
    		return CMD_WARNING;			
        }

	    for(slot_id=1;slot_id<=slot_count;slot_id++)
	    {
    
        	ret = parse_vlan_no((char*)argv[0],&vlanId);
        	
        	if (NPD_FAIL == ret) {
            	vty_out(vty,"% Bad parameter,vlan id illegal!\n");
        		return CMD_WARNING;
        	}
        	if (4095 == vlanId){
        		vty_out(vty,"% Bad parameter,Reserved vlan for Layer3 interface of EthPort!\n");
        		return CMD_WARNING;
        	}
        	/* vlan 4094 do not support config */
        	if (4094 == vlanId){
        		vty_out(vty,"% Bad parameter,Reserved vlan for internal use!\n");
        		return CMD_WARNING;
        	}			
        	else {
        		/*once bad param,it'll NOT sed message to NPD*/
        		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
        											NPD_DBUS_VLAN_OBJPATH ,	\
        											NPD_DBUS_VLAN_INTERFACE ,	\
        											NPD_DBUS_VLAN_METHOD_CONFIG_LAYER2_ONE);
        		
        		dbus_error_init(&err);

        		dbus_message_append_args(query,
        								 DBUS_TYPE_UINT16,&vlanId,
        								 DBUS_TYPE_INVALID);
                if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) 				
    			{
        			if(slot_id == local_slot_id)
    				{
                        reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
    				}
    				else 
    				{	
    				   	vty_out(vty,"Can not connect to slot:%d \n",slot_id);	
    					continue;
    				}
                }
    			else
    			{
                    reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);				
    			}
				
        	}
        	dbus_message_unref(query);
        	
        	if (NULL == reply) {
        		vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",slot_id);
        		return CMD_SUCCESS;
        	}

        	if (dbus_message_get_args ( reply, &err,
        					DBUS_TYPE_UINT32, &op_ret,
        					/*DBUS_TYPE_UINT16, &vlanId,*/
        					DBUS_TYPE_INVALID)) 
        	{
        		if(ETHPORT_RETURN_CODE_NO_SUCH_VLAN == op_ret) 
        		{
                	vty_out(vty,"% Bad parameter:vlan ID illegal.\n",vlanId);/*Stay here,not enter vlan_config_node CMD.*/
        		}
        		else if(VLAN_RETURN_CODE_ERR_GENERAL == op_ret ) 
        		{
        			vty_out(vty,"% Bad parameter:vlan %d NOT exists on slot %d.\n",vlanId,slot_id); /*Stay here,not enter vlan_config_node CMD.*/
        		}
        		else if(COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
        			vty_out(vty,"%% Product not support this function!\n");
        		}
        		else if(VLAN_RETURN_CODE_ERR_NONE == op_ret)   /*0+3,Vlan exist,then enter vlan_config_node CMD.*/
        		{
					dbus_message_unref(reply);
					continue;  /* goto next slot */
        		}
				else{}
                /* op_ret is error, return. not into config---bug:AXSSZFI-278 */
				dbus_message_unref(reply);
            	return CMD_SUCCESS;
        	} 
        	else 
        	{
        		vty_out(vty,"Failed get args.Please check npd on slot: %d\n",slot_id);
            	dbus_message_unref(reply);
            	return CMD_SUCCESS;
        	}
	    }

		/* only every board is OK, then into config node */
		if(CONFIG_NODE == vty->node) {
			vty->node = VLAN_NODE;   /* change this only one time,zhangdi */
			nodeSave = vlanId;
			vty->index = (void*)nodeSave;/*when not add & before vlanId, the Vty enter <config-line> CMD Node.*/
		}
		else{
			vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
			return CMD_WARNING;
		}
		
		return CMD_SUCCESS;	
	}
	else
	{
    	DBusMessage *query = NULL, *reply = NULL;
    	DBusError err;

    	unsigned short 	vlanId = 0;
    	int ret = 0;
    	unsigned int op_ret = 0, nodeSave = 0;

    	ret = parse_vlan_no((char*)argv[0],&vlanId);
    	
    	if (NPD_FAIL == ret) {
        	vty_out(vty,"% Bad parameter,vlan id illegal!\n");
    		return CMD_WARNING;
    	}
    	if (4095 == vlanId){
    		vty_out(vty,"% Bad parameter,Reserved vlan for Layer3 interface of EthPort!\n");
    		return CMD_WARNING;
    	}
    	else {
    		/*once bad param,it'll NOT sed message to NPD*/
    		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
    											NPD_DBUS_VLAN_OBJPATH ,	\
    											NPD_DBUS_VLAN_INTERFACE ,	\
    											NPD_DBUS_VLAN_METHOD_CONFIG_LAYER2_ONE);
    		
    		dbus_error_init(&err);

    		dbus_message_append_args(query,
    								 DBUS_TYPE_UINT16,&vlanId,
    								 DBUS_TYPE_INVALID);
    		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
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
    					DBUS_TYPE_UINT32, &op_ret,
    					/*DBUS_TYPE_UINT16, &vlanId,*/
    					DBUS_TYPE_INVALID)) 
    	{
    		if(ETHPORT_RETURN_CODE_NO_SUCH_VLAN == op_ret) 
    		{
            	vty_out(vty,"% Bad parameter:vlan ID illegal.\n",vlanId);/*Stay here,not enter vlan_config_node CMD.*/
    		}
    		else if(VLAN_RETURN_CODE_ERR_GENERAL == op_ret ) 
    		{
    			vty_out(vty,"% Bad parameter:vlan %d NOT exists.\n",vlanId); /*Stay here,not enter vlan_config_node CMD.*/
    		}
    		else if(COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
    			vty_out(vty,"%% Product not support this function!\n");
    		}
    		else if(VLAN_RETURN_CODE_ERR_NONE == op_ret)   /*0+3,Vlan exist,then enter vlan_config_node CMD.*/
    		{
    			/*vty_out(vty,"Enter inner CMD node...\n");*/
    			if(CONFIG_NODE == vty->node) {
    				vty->node = VLAN_NODE;
    				nodeSave = vlanId;
    				vty->index = (void*)nodeSave;/*when not add & before vlanId, the Vty enter <config-line> CMD Node.*/
    			}
    			else{
    				vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
    				return CMD_WARNING;
    			}
    			dbus_message_unref(reply);
    			return CMD_SUCCESS;
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
    	dbus_message_unref(reply);
    	return CMD_SUCCESS;
    	}
    }
}
/***********************************
* Enter vlan config mode to configure 
* special vlan.
* Params:
*		vlan name:	 
* Usage: config vlan VLANNAME
************************************/
DEFUN(config_layer2_vlan_vname_func,
	config_layer2_vlan_vname_cmd,
	"config vlan VLANNAME",
	CONFIG_STR
	"Config layer 2 vlan entity\n"
	"Specify vlan name begins with char or'_', and name length no more than 20 characters\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	vty_out(vty,"Do not support CMD on distributed system. \n");
	return CMD_WARNING;

	char* vlanName = NULL;
	unsigned short vlanId = 0;
	unsigned int nameSize = 0, nodeSave = 0;
	int ret = 0;
	unsigned int op_ret = 0;

	/*printf("before parse_vlan_name %s length %d\n",argv[1],strlen(argv[1]));*/
	vlanName = (char*)malloc(ALIAS_NAME_SIZE);
	memset(vlanName,0,ALIAS_NAME_SIZE);

	ret = vlan_name_legal_check((char*)argv[0],strlen(argv[0]));
	if(ALIAS_NAME_LEN_ERROR == ret) {
		vty_out(vty,"% Bad parameter,vlan name too long!\n");
		return CMD_WARNING;
	}
	else if(ALIAS_NAME_HEAD_ERROR == ret) {
		vty_out(vty,"% Bad parameter,vlan name begins with an illegal char!\n");
		return CMD_WARNING;
	}
	else if(ALIAS_NAME_BODY_ERROR == ret) {
		vty_out(vty,"% Bad parameter,vlan name contains illegal char!\n");
		return CMD_WARNING;
	}
	else{
		nameSize = strlen(argv[0]);
		memcpy(vlanName,argv[0],nameSize);
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_VLAN_OBJPATH ,	\
											NPD_DBUS_VLAN_INTERFACE ,	\
											NPD_DBUS_VLAN_METHOD_CONFIG_LAYER2_VIA_VLANNAME );
		
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_STRING,&vlanName, 
								 DBUS_TYPE_INVALID);

		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
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
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT16, &vlanId,
					DBUS_TYPE_INVALID)) 
	{
		if (VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret ) 
		{
			vty_out(vty,"% Bad parameter,Vlan %s NOT exists.\n",vlanName); /*Stay here,not enter vlan_config_node CMD.*/
		}
		else if (VLAN_RETURN_CODE_BADPARAM == op_ret) {
			vty_out(vty,"% Bad Parameter:Can NOT config Default vlan.\n");
		}
		else if (VLAN_RETURN_CODE_PORT_L3_INTF == op_ret) {
			vty_out(vty,"% Bad parameter,Reserved vlan for Layer3 interface of EthPort!\n");
		}
		else if(COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
			vty_out(vty,"%% Product not support this function!\n");
		}
		else  if(VLAN_RETURN_CODE_ERR_NONE == op_ret)   /*0+3,Vlan exist,then enter vlan_config_node CMD.*/
		{
			/*vty_out(vty,"Enter inner CMD node...\n");*/
			if(CONFIG_NODE == vty->node) {
				vty->node = VLAN_NODE;
				nodeSave = vlanId;
				vty->index = (void*)nodeSave;/*when not add & before vlanId, the Vty enter <config-line> CMD Node.*/
			}
			else{
				vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
				return CMD_WARNING;
			}
			dbus_message_unref(reply);
			return CMD_SUCCESS;
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
	dbus_message_unref(reply);
	free(vlanName);
	return CMD_SUCCESS;
	}
}
/************************************************
* Add or delete specific port to(from) vlan entry
* Params:
*		slot/port:		1/1,1/2,... 4/6,
*		tag/untag:		
* Usage:(add|delete) port (tag|untag)
*********************************************/
DEFUN(config_vlan_add_del_port_cmd_func,
	config_vlan_add_del_port_cmd,
	"(add|delete) port SLOT_PORT (tag|untag)",
	"Add port as vlan member\n"
	"Delete port from vlan\n"
	"Port that in System\n"
	CONFIG_ETHPORT_STR
	"Port member of vlan support 802.1q\n"
	"Port member of vlan not support 802.1q\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned char slot_no = 0,local_port_no = 0;
	unsigned int t_slotno = 0,t_portno = 0;
	boolean isAdd 		= FALSE;
	boolean isTagged 	= FALSE;
	int 	ret 		= 0;
	unsigned short	vlanId = 0;
	unsigned int 	op_ret = 0,nodesave = 0;
    int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);

	/*fetch the 1st param : add ? delete*/
	if(0 == strncmp(argv[0],"add",strlen(argv[0]))) {
		isAdd = TRUE;
	}
	else if (0 == strncmp(argv[0],"delete",strlen(argv[0]))) {
		isAdd= FALSE;
	}
	else {
		vty_out(vty,"% Bad parameter!\n");
		return CMD_WARNING;
	}
	/*fetch the 2nd param : slotNo/portNo*/
	op_ret = parse_slotno_localport((char *)argv[1],&t_slotno,&t_portno);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad parameter:Unknow portno format.\n");
		return CMD_WARNING;
	}
	else if (1 == op_ret){
		vty_out(vty,"% Bad parameter: Bad slot/port number.\n");
		return CMD_WARNING;
	}

	/* check the slot, budfix:AXSSZFI-1011 */
    if(t_slotno > slot_count)
    {
    	vty_out(vty,"% Bad parameter,slot id illegal!\n");
		return CMD_WARNING;		
    }
	
	slot_no = (unsigned char)t_slotno;
	local_port_no = (unsigned char)t_portno;
	/*not support config port on slot0*/
	#if 0
	if(0  == slot_no) {
		vty_out(vty,"% Bad parameter: Bad slot/port number.\n");
		return CMD_WARNING;
	}
	#endif
	
	/*slot_no =0, it'll FAIL during parseing devNum/portNum in nam.*/
	/*fetch the 3nd param : tagged OR untagged*/
	ret = param_first_char_check((char*)argv[2],1);
	if (NPD_FAIL == ret) {
		vty_out(vty,"% Bad parameter!\n");
		return CMD_WARNING;
	}
	else if(1 == ret){
		isTagged = TRUE;
	}
	else if(0 == ret){
		isTagged = FALSE;
	}
	nodesave = (unsigned int)(vty->index);
	vlanId = (unsigned short)nodesave;

	ret = dcli_vlan_add_del_port(vty,isAdd,vlanId,slot_no,local_port_no,isTagged);
	if( CMD_SUCCESS != ret){
		return CMD_WARNING;
	}

   	/* update g_vlanlist[] on Active-MCB & Standby-MCB. zhangdi 20110804*/
	if(is_distributed == DISTRIBUTED_SYSTEM)	
	{
		#if 0
		/* get my local slot */
       	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	if(local_slot_id<0)
    	{
    		vty_out(vty,"get local_slot_id return -1 !\n");
    		return CMD_SUCCESS;		
       	}
		#endif
    	/* Wherever slot the CMD we input, then update the gvlanlist[] of 2 MCBs */
		ret = dcli_vlanlist_add_del_port(vty,isAdd,vlanId,slot_no,local_port_no,isTagged);
	    if( CMD_SUCCESS != ret)
		{
		    return CMD_WARNING;
	    }

	}		
	return CMD_SUCCESS;

	#if 0
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_PORT_MEMBER_ADD_DEL);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_BYTE,&isAdd,
								DBUS_TYPE_BYTE,&slot_no,
								DBUS_TYPE_BYTE,&local_port_no,
							 	DBUS_TYPE_BYTE,&isTagged,
							 	DBUS_TYPE_UINT16,&vlanId,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_ERROR_NO_SUCH_PORT == op_ret) {
            	vty_out(vty,"%% Bad parameter: Bad slot/port number.\n");
			}/*either slot or local port No err,return NPD_DBUS_ERROR_NO_SUCH_PORT */
			else if (VLAN_RETURN_CODE_ERR_NONE == op_ret ) {
				/*vty_out(vty,"Ethernet Port %d-%d %s success.\n",slot_no,local_port_no,isAdd?"Add":"Delete");*/
			} 
			else if (VLAN_RETURN_CODE_BADPARAM == op_ret) {
				vty_out(vty,"%% Error occurs in Parse eth-port Index or devNO.& logicPortNo.\n");
			}
			else if (VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret) {
				vty_out(vty,"%% Error: Checkout-vlan to be configured NOT exists on SW.\n");
			}
			else if (VLAN_RETURN_CODE_PORT_EXISTS == op_ret) {
				vty_out(vty,"%% Bad parameter: port Already member of the vlan.\n");
			}
			else if (VLAN_RETURN_CODE_PORT_NOTEXISTS == op_ret){
				vty_out(vty,"%% Bad parameter: port NOT member of the vlan.\n");
			}
			else if (VLAN_RETURN_CODE_PORT_MBRSHIP_CONFLICT == op_ret) {
				vty_out(vty,"%% Bad parameter: port Already untagged member of other active vlan.\n");
			}			
			else if (VLAN_RETURN_CODE_PORT_PROMIS_PORT_CANNOT_ADD2_VLAN == op_ret) {
				vty_out(vty,"%% Promiscous port %d/%d can't be added to any vlan!\n",slot_no,local_port_no);
			}
			else if (VLAN_RETURN_CODE_PORT_PROMISCUOUS_MODE_ADD2_L3INTF == op_ret){
				vty_out(vty,"%% Bad parameter: promiscuous mode port can't add to l3 interface!\n");
			}
			else if (VLAN_RETURN_CODE_PORT_TAG_CONFLICT == op_ret) {
				vty_out(vty,"%% Bad parameter: port Tag-Mode can NOT match!\n");
			}
			else if (VLAN_RETURN_CODE_PORT_SUBINTF_EXISTS == op_ret){
                vty_out(vty,"%% Can't delete tag port with sub interface!\n");
			}
			else if (VLAN_RETURN_CODE_PORT_DEL_PROMIS_PORT_TO_DFLT_VLAN_INTF == op_ret){
				vty_out(vty,"%% Promiscuous mode port can't delete!\n");
			}
			else if (VLAN_RETURN_CODE_PORT_TRUNK_MBR == op_ret) {
				vty_out(vty,"%% Bad parameter: port is member of a active trunk!\n");
			}
			else if (VLAN_RETURN_CODE_ARP_STATIC_CONFLICT == op_ret) {
				vty_out(vty,"%% Bad parameter: port has attribute of static arp!\n");
			}
			else if(PVLAN_RETURN_CODE_THIS_PORT_HAVE_PVE == op_ret) {
				vty_out(vty,"%% Bad parameter: port is pvlan port!please delete pvlan first!\n");
			}
			else if (VLAN_RETURN_CODE_L3_INTF == op_ret) {
				if(isAdd) {
					vty_out(vty,"%% Port adds to L3 interface vlan %d.\n",vlanId);
				}
				else {
					vty_out(vty,"%% L3 interface vlan member can NOT delete here.\n");
				}
			}
			else if (VLAN_RETURN_CODE_PORT_L3_INTF == op_ret) {
				if(isAdd) {
					vty_out(vty,"%% Port can NOT add to vlan as port is L3 interface.\n");
				}
				else {
					vty_out(vty,"%% Port can NOT delete from vlan as port is L3 interface.\n");
				}
			}
			else if (VLAN_RETURN_CODE_ERR_HW == op_ret) {
				vty_out(vty,"%% Error occurs in Config on HW.\n");	
			}
			else if (VLAN_RETURN_CODE_ERR_NONE != op_ret){
				vty_out(vty,"%% Unknown Error! return %d \n",op_ret);
			}
			else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret){
				vty_out(vty,"This operation is unsupported!\n");
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
	#endif
}

/************************************************
* Add or delete portlist to(from) vlan entry
* Params:
*		slot/port:		1/1,1/2,... 4/6,
*		tag/untag:		
* Usage:(add|delete) port (tag|untag)
*********************************************/
DEFUN(config_vlan_add_del_portlist_cmd_func,
	config_vlan_add_del_portlist_cmd,
	"(add|delete) portlist PORT_LIST (tag|untag)",
	"Add port as vlan member\n"
	"Delete port from vlan\n"
	"Ports that in System\n"
	CONFIG_ETHPORT_STR
	"Ports member of vlan support 802.1q\n"
	"Ports member of vlan not support 802.1q\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	SLOT_PORT_S slotport[24] ;
	boolean isAdd 		= FALSE;
	boolean isTagged 	= FALSE;
	int 	i,p_count = 0,ret = 0;
	unsigned short	vlanId = 0;
	unsigned int nodesave = 0;
    int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);

	memset(slotport,0,sizeof(SLOT_PORT_S)*24);
	/*fetch the 1st param : add ? delete*/
	if(0 == strncmp(argv[0],"add",strlen(argv[0]))) {
		isAdd = TRUE;
	}
	else if (0 == strncmp(argv[0],"delete",strlen(argv[0]))) {
		isAdd= FALSE;
	}
	else {
		vty_out(vty,"% Bad parameter!\n");
		return CMD_WARNING;
	}
	/*fetch the 2nd param : slotNo/portNo*/
	ret = parse_port_list((char *)argv[1],&p_count,slotport);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"%% Bad parameter:Unknow slot-port format.\n");
		return CMD_WARNING;
	}
	/*slot_no =0, it'll FAIL during parseing devNum/portNum in nam.*/
	/*fetch the 3nd param : tagged OR untagged*/
	if(strncmp(argv[2],"tag",strlen(argv[2]))==0) {
		isTagged = TRUE;
	}
	else if (strncmp(argv[2],"untag",strlen(argv[2]))==0) {
		isTagged = FALSE;
	}
	else {
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}
	nodesave = (unsigned int)(vty->index);
	vlanId = (unsigned short)nodesave;
	
	/*vty_out(vty,"portlist including %d ports.\n",p_count);*/
	for(i = 0;i<p_count;i++){
		if(slotport[i].slot>slot_count)
		{
        	vty_out(vty,"%% Bad parameter SLOT: %d-%d.\n",slotport[i].slot,slotport[i].port);
			continue;
		}
		ret = dcli_vlan_add_del_port(vty,isAdd,vlanId,slotport[i].slot,slotport[i].port,isTagged);
		if( CMD_SUCCESS != ret){
			return CMD_WARNING;
		}
       	/* update g_vlanlist[] on Active-MCB & Standby-MCB. zhangdi 20110804*/
    	if(is_distributed == DISTRIBUTED_SYSTEM)	
    	{
    		ret = dcli_vlanlist_add_del_port(vty,isAdd,vlanId,slotport[i].slot,slotport[i].port,isTagged);
    	    if( CMD_SUCCESS != ret)
    		{
    		    return CMD_WARNING;
    	    }
    	}		
	}
	return CMD_SUCCESS;
}
DEFUN(config_vlan_qinq_allbackport_cmd_func,
	config_vlan_qinq_allbackport_cmd,
	"config qinq (enable|disable)",
	"Enable qinq function\n"
	"Disable qinq function\n"
	"Port that in System\n"
	CONFIG_ETHPORT_STR
	"Port member of vlan support 802.1q\n"
	"Port member of vlan not support 802.1q\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	boolean Endis 	= FALSE;
	int 	ret 		= 0;
	unsigned short	vlanId = 0;
	unsigned int 	op_ret = 0,nodesave = 0;

	/*fetch the 1st param : add ? delete*/
	if(strncmp(argv[0],"enable",strlen(argv[0]))==0) {
		Endis = TRUE;
	}
	else if (strncmp(argv[0],"disable",strlen(argv[0]))==0) {
		Endis = FALSE;
	}
	else
	{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}
	
	nodesave = (unsigned int)(vty->index);
	vlanId = (unsigned short)nodesave;

	if(is_distributed == DISTRIBUTED_SYSTEM)	
	{
		ret = dcli_vlan_qinq_allbackport_endis(vty,Endis,vlanId);
	    if( CMD_SUCCESS != ret)
		{
		    return CMD_WARNING;
	    }

	}		
	return CMD_SUCCESS;

}

DEFUN(config_vlan_qinq_tocpuport_cmd_func,
	config_vlan_qinq_tocpuport_cmd,
	"set vlan <2-4093> to-cpu-port qinq (enable|disable)",
	"Enable to cpu port qinq function\n"
	"Disable to cpu portqinq function\n"
	"Vlan id\n"
	CONFIG_ETHPORT_STR
	"Port member of vlan support 802.1q\n"
	"Port member of vlan not support 802.1q\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	boolean Endis 	= FALSE;
	int 	ret 		= 0;
	unsigned short	vlanId = 0;
	unsigned int 	op_ret = 0;
	int slot_id =0;
	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
	int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);
	
	/*get vlan ID*/
	ret = parse_vlan_no((char*)argv[0],&vlanId);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"% Bad parameter,vlan id illegal!\n");
		return CMD_WARNING;
	}
	if(strncmp(argv[1],"enable",strlen(argv[1]))==0) {
		Endis = TRUE;
	}
	else if (strncmp(argv[1],"disable",strlen(argv[1]))==0) {
		Endis = FALSE;
	}
	else
	{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}
	/* change no to index, for hardware use */
   if((local_slot_id < 0) || (slot_count <0))		
	{			
		vty_out(vty,"read file error ! \n");			
		return CMD_WARNING;		
	}
	for(slot_id=1;slot_id<=slot_count;slot_id++)
    {

    	

		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_VLAN_OBJPATH ,	\
											NPD_DBUS_VLAN_INTERFACE ,	\
											NPD_DBUS_VLAN_METHOD_CONFIG_TOCPUPORT_QINQ_ENDIS_FOR_OLD );
		
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_BYTE,&Endis,
								 DBUS_TYPE_UINT16,&vlanId,
								 DBUS_TYPE_INVALID);
		
        if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) 				
		{
			if(slot_id == local_slot_id)
			{
                reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
			}
			else 
			{	
			   	vty_out(vty,"Can not connect to slot:%d \n",slot_id);	
				continue;
			}
        }
		else
		{
            reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);				
		}
			
    	dbus_message_unref(query);
    	
    	if (NULL == reply) {
    		vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",slot_id);
    		return CMD_SUCCESS;
    	}

    	if (dbus_message_get_args ( reply, &err,
    					DBUS_TYPE_UINT32, &op_ret,
    					DBUS_TYPE_INVALID)) 
    	{
    		if(VLAN_RETURN_CODE_ERR_NONE == op_ret) {
    			vty_out(vty," set cpu 1 port 1 qinq  %s successfully !\n",Endis?"enabled":"disabled");
    		}
			else if(VLAN_RETURN_CODE_VLAN_EXISTS == op_ret)
			{
				vty_out(vty,"cpu 1 port 1 qinq are %s already !\n",Endis?"enabled":"disabled");
			}
			else if(VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret)
			{
				vty_out(vty,"vlan does not exist!\n");
			}
			else if(VLAN_RETURN_CODE_ERR_GENERAL == op_ret)
			{
				vty_out(vty,"please bond first !\n");
			}
    		else 
    		{
    			vty_out(vty," cpu 1 port 1 qinq %s fail !\n",Endis?"enabled":"disabled");
    		}
    	} 
    	else 
    	{
    		vty_out(vty,"% Failed get args.\n");
    		if (dbus_error_is_set(&err)) 
    		{
    			vty_out(vty,"% %s raised: %s",err.name,err.message);
    			dbus_error_free_for_dcli(&err);
    		}
    	}
    	dbus_message_unref(reply);
    }
    return CMD_SUCCESS;


}
DEFUN(config_vlan_to_ms_cpu_port_qinq_cmd_func,
	config_vlan_to_ms_cpu_port_qinq_cmd,
	"set vlan <2-4093> slot <1-16> cpu <1-2> port <1-8> qinq (enable|disable)",
	"Enable cpu port qinq function\n"
	"Disable cpu portqinq function\n"
	"Vlan id\n"
	CONFIG_ETHPORT_STR
	"Port member of vlan support 802.1q\n"
	"Port member of vlan not support 802.1q\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	boolean Endis 	= FALSE;
	int 	ret 		= 0;
	unsigned short	vlanId = 0;
	unsigned int 	op_ret = 0;
	unsigned short 	cpu_no = 0, cpu_port_no = 0;
	unsigned short slot_id =0, slot = 0;
	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);

    if((local_slot_id<0)||(slot_count<0))
    {
    	vty_out(vty,"get_product_info() return -1,Please check dbm file !\n");
		return CMD_WARNING;			
    }
	
	/*get vlan ID*/
	ret = parse_vlan_no((char*)argv[0],&vlanId);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"% Bad parameter,vlan id illegal!\n");
		return CMD_WARNING;
	}
    /*get dist slot */
	ret = parse_single_param_no((char*)argv[1],&slot_id);
	if(NPD_SUCCESS != ret) {
		vty_out(vty,"%% parse param failed!\n");
		return CMD_WARNING;
	}
    if(slot_id > slot_count)
    {
    	vty_out(vty,"% Bad parameter,slot id illegal!\n");
		return CMD_WARNING;		
    }

	/* get cpu no */
	ret = parse_single_param_no((char*)argv[2],&cpu_no);
	if(NPD_SUCCESS != ret) {
		vty_out(vty,"%% parse cpu_port_no param failed!\n");
		return CMD_WARNING;
	}		

	/* get cpu prot number */
	ret = parse_single_param_no((char*)argv[3],&cpu_port_no);
	if(NPD_SUCCESS != ret) {
		vty_out(vty,"%% parse cpu_port_no param failed!\n");
		return CMD_WARNING;
	}
	/* change no to index, for hardware use */
    cpu_no = cpu_no -1;
    cpu_port_no = cpu_port_no -1;
	
	if(strncmp(argv[4],"enable",strlen(argv[4]))==0) {
		Endis = TRUE;
	}
	else if (strncmp(argv[4],"disable",strlen(argv[4]))==0) {
		Endis = FALSE;
	}
	else
	{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}


	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_TOCPUPORT_QINQ_ENDIS );
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&Endis,
							 DBUS_TYPE_UINT16,&vlanId,
							 DBUS_TYPE_UINT16,&slot_id,
							 DBUS_TYPE_UINT16,&cpu_no,
							 DBUS_TYPE_UINT16,&cpu_port_no,
							 DBUS_TYPE_INVALID);
	
    if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) 				
    {
    	if(slot_id == local_slot_id)
    	{
            reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
    	}
    	else 
		{	
		    vty_out(vty,"Can not connect to slot:%d \n",slot_id);	
		    return CMD_WARNING;
		}	
    }
    else
    {
        reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);				
    }
		
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",slot_id);
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) 
	{
		if(VLAN_RETURN_CODE_ERR_NONE == op_ret) {
			vty_out(vty," set vlan %d slot %d cpu %d port %d qinq %s success !\n",vlanId,slot_id,cpu_no+1,cpu_port_no+1,Endis?"enable":"disable");
		}
		else if(VLAN_RETURN_CODE_VLAN_EXISTS == op_ret)
		{
			if(Endis)
			{
				vty_out(vty," vlan %d slot %d cpu %d port %d qinq is enabled already !\n",vlanId,slot_id,cpu_no+1,cpu_port_no+1);
			}
			else
			{
				vty_out(vty," vlan %d slot %d cpu %d port %d qinq is disabled already !\n",vlanId,slot_id,cpu_no+1,cpu_port_no+1);
			}
		}
		else if(VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret)
		{
			vty_out(vty,"vlan does not exist!\n");
		}
		else if(VLAN_RETURN_CODE_ERR_GENERAL == op_ret)
		{
			vty_out(vty,"please bond first !\n");
		}
		else 
		{
			vty_out(vty," enable vlan %d slot %d cpu %d port %d qinq  fail  !\n",vlanId,slot_id,cpu_no+1,cpu_port_no+1);
		}
	} 
	else 
	{
		vty_out(vty,"% Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"% %s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	/*update for master slot*/
	if(ret == VLAN_RETURN_CODE_ERR_NONE)
	{
		ret = dcli_vlan_qinq_tocpuport_for_master(vty,Endis,vlanId,slot_id,local_slot_id,cpu_no,cpu_port_no);
		if(ret != CMD_SUCCESS)
		{
			vty_out(vty,"update vlan %d slot %d cpu %d port %d  for master fail !\n",vlanId,slot_id,cpu_no+1,cpu_port_no+1);
			return CMD_FAILURE;
		}
	}
	return CMD_SUCCESS;

}


DEFUN(config_vlan_qinq_cmd_func,
	config_vlan_qinq_cmd,
	"config port SLOT_PORT qinq (enable|disable)",
	"enable port SLOT_PORT qinq function \n"
	"disable port SLOT_PORT qinq function\n"
	"Ports that in System\n"
	CONFIG_ETHPORT_STR
	"Ports member of vlan support 802.1q\n"
	"Ports member of vlan not support 802.1q\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	boolean isEnable 	= FALSE;
	int ret = 0;
	unsigned short	vlanId = 0;
	unsigned int nodesave = 0;
	unsigned char slot_no = 0,local_port_no = 0;
	unsigned int t_slotno = 0,t_portno = 0;
	int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);

	ret = parse_slotno_localport((char *)argv[0],&t_slotno,&t_portno);
	if (NPD_FAIL == ret) 
	{
    	vty_out(vty,"% Bad parameter:Unknow portno format.\n");
		return CMD_WARNING;
	}
	else if (1 == ret)
	{
		vty_out(vty,"% Bad parameter: Bad slot/port number.\n");
		return CMD_WARNING;
	}

	/* check the slot, budfix:AXSSZFI-1011 */
    if(t_slotno > slot_count)
    {
    	vty_out(vty,"% Bad parameter,slot id illegal!\n");
		return CMD_WARNING;		
    }
	
	slot_no = (unsigned char)t_slotno;
	local_port_no = (unsigned char)t_portno;
	/*fetch the 3nd param : tagged OR untagged*/
	if(strncmp(argv[1],"enable",strlen(argv[1]))==0) 
	{
		isEnable = TRUE;
	}
	else if (strncmp(argv[1],"disable",strlen(argv[1]))==0) 
	{
		isEnable = FALSE;
	}
	else 
	{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}
	nodesave = (unsigned int)(vty->index);
	vlanId = (unsigned short)nodesave;
	
	/*vty_out(vty,"portlist including %d ports.\n",p_count);*/
	ret = dcli_vlan_qinq_endis(vty,isEnable,vlanId,slot_no,local_port_no);
	if( CMD_SUCCESS != ret)
	{
		return CMD_WARNING;
	}
	else
	{
		ret = dcli_vlan_update_port_qinq_info(vty,isEnable,vlanId,slot_no,local_port_no);
		if(CMD_SUCCESS != ret)
		{
			return CMD_WARNING;
		}
	}
	return CMD_SUCCESS;
}

DEFUN(config_vlan_vid_update_cmd_func, 
	config_vlan_vid_update_cmd, 
	"set vid <2-4094>", 
	"Assigned a new vlan ID\n"
	"Vlan ID \n"
	"Vlan ID range <2-4094>\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned short	vid_corrent = 0,vid_new = 0;
	unsigned int 	ret = 0,op_ret = 0,nodesave = 0;

	/*for bug: AXSSZFI-297 */
	vty_out(vty,"Do not support CMD on distributed system. \n");
	return CMD_WARNING;	

	nodesave = (unsigned int)(vty->index);
	vid_corrent = (unsigned short)nodesave;	
	/*vty_out(vty,"config vlanId %d\n",vid_corrent);*/

	/*printf("before parse_vlan_no %s\n",argv[0]);*/
	ret = parse_vlan_no((char*)argv[0],&vid_new);
	
	if (NPD_FAIL == ret) {
    	vty_out(vty,"% Bad parameter,vlan id illegal!\n");
		return CMD_WARNING;
	}
	if (4095 == vid_new){
		vty_out(vty,"% Bad parameter,Reserved vlan for Layer3 interface of EthPort!\n");
		return CMD_WARNING;
	}
	/* vlan 4094 do not support config */
	else if ((4094 == vid_new)&&(is_distributed == DISTRIBUTED_SYSTEM)){
		vty_out(vty,"% Bad parameter,Reserved vlan for internal use!\n");
		return CMD_WARNING;
	}	
	else if(vid_corrent == vid_new){
		vty_out(vty,"% Bad parameter,vlan id same to original vid!\n");
		return CMD_WARNING; /*no return lead to segmentation fault.*/
	}
	else {
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_VLAN_OBJPATH,	\
											NPD_DBUS_VLAN_INTERFACE,	\
											NPD_DBUS_VLAN_METHOD_UPDATE_VID);
	
		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_UINT16,&vid_corrent,	
								DBUS_TYPE_UINT16,&vid_new,
								DBUS_TYPE_INVALID);
		
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	}
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
		if (VLAN_RETURN_CODE_VLAN_EXISTS == op_ret ) {
			vty_out(vty,"% Bad param:new vid points to a vlan has Already exists!\n");
		} 
		else if (VLAN_RETURN_CODE_TRUNK_EXISTS == op_ret) {
			vty_out(vty,"% Bad param:vlan has trunk member.\n");
		}
		else if (VLAN_RETURN_CODE_ERR_HW == op_ret) {
			vty_out(vty,"%% Error occurs on HW.\n");
		}
        else if (VLAN_RETURN_CODE_PORT_SUBINTF_EXISTS== op_ret) {
			vty_out(vty,"%% Can't set vid, There are some subif with this vlan!\n");
		}
		else if(VLAN_RETURN_CODE_CONFIG_NOT_ALLOWED == op_ret)
		{
			vty_out(vty,"%% Advanced-routing default vlan is not allowed to set vid!\n");
		}
		else if (VLAN_RETURN_CODE_ERR_NONE == op_ret) {
			/*vty_out(vty,"vlan %d Changed vid to %d.\n",vid_corrent,vid_new);*/
		}
		else if (VLAN_RETURN_CODE_L3_INTF == op_ret) {
			vty_out(vty,"vlan L3 interface NOT support modify vid.\n");
		}
        else if(VLAN_RETURN_CODE_HAVE_PROT_VLAN_CONFIG== op_ret){
            vty_out(vty,"%% There is protocol-vlan config on this vlan,please delete it first!\n");
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
	if(NPD_SUCCESS == op_ret) {
		nodesave = vid_new;
		vty->index = (void*)nodesave;
	}/*in order to config vlan with new ID.For example:add/delete port,show vlan.*/
	return CMD_SUCCESS;
}


/*add or delete subvlan to(from)vlan entry*/

/*CMD :for add or delete port to (from) vlan*/
DEFUN(config_vlan_add_del_subvlan_cmd_func,
	config_vlan_add_del_subvlan_cmd,
	"(add|delete) subvlan <1-4095>",
	"Add subvlan to super vlan\n"
	"Delete subvlan from super vlan\n"
	"Sub-vlan in System\n"
	"VlanId range <1-4095> valid\n"
	)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short subvlanId;
	boolean isAdd 		= FALSE;
	boolean isTagged 	= FALSE;
	unsigned short	vlanId;
	unsigned int 	op_ret;

	/*fetch the 1st param : add ? delete*/
	if(strcmp(argv[0],"add")==0)
	{
		isAdd = TRUE;
	}
	else if (strcmp(argv[0],"delete")==0)
	{
		isAdd= FALSE;
	}
	else
	{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}		

	/*fetch the 2nd param : slotNo/portNo*/
	op_ret = parse_subvlan_no((char *)argv[1],&subvlanId);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"Unknow portno format.\n");
		return CMD_SUCCESS;
	}
	if (4095 == subvlanId){
		vty_out(vty,"% Bad parameter,Reserved vlan for Layer3 interface of EthPort!\n");
		return CMD_WARNING;
	}
	else
	{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}
	vty_out(vty,"%s ",isAdd ? "Add" : "Delete");
	vty_out(vty,"sub-vlan %d ",subvlanId);

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_SUBVLAN_ADD_DEL);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_BYTE,&isAdd,
								DBUS_TYPE_BYTE,&subvlanId,
							 	DBUS_TYPE_BYTE,&isTagged,
							 	DBUS_TYPE_UINT16,&vlanId,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if (VLAN_RETURN_CODE_ERR_NONE == op_ret ) {
				vty_out(vty,"Ethernet sub-vlan %d-%d index %#0x\n",subvlanId);
			}
			if (ETHPORT_RETURN_CODE_NO_SUCH_VLAN == op_ret) {
            	vty_out(vty,"ERROR: No such port.\n");
			}
		
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
#if 0
DEFUN(vlan_trunk_add_del_tag_cmd_func,
	vlan_add_del_trunk_tag_cmd,
	"(add|delete) trunk <1-127> (tag|untag)",
	"Add trunk as vlan member\n"
	"Delete trunk from vlan\n"
	"Trunk that in system\n"
	"Trunk ID range <1-127>\n"
	"Trunk member of vlan support 802.1q\n"
	"Trunk member of vlan not support 802.1q\n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned short vlanId = 0, trunk_no = 0;
	boolean isAdd 		= FALSE;
	boolean isTagged 	= FALSE;
	unsigned int 	ret = 0, op_ret = 0, nodesave = 0;
	ret = param_first_char_check((char*)argv[0],0);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Unknow parameter format.\n");
		return CMD_SUCCESS;
	}
	else if (1 ==ret){
		isAdd = TRUE;
	}
	else if(0 == ret){
		isAdd = FALSE;
	}
	/*fetch the 2nd param : slotNo/portNo*/
	op_ret = parse_trunk_no((char *)argv[1],&trunk_no);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Unknow trunkNo format.\n");
		return CMD_SUCCESS;
	}
	#if 1
	/*fetch the 3nd param : tagged OR untagged*/
	if(strncmp(argv[2],"tag",strlen(argv[2]))==0)
	{
		isTagged = TRUE;
	}
	else if (strncmp(argv[2],"untag",strlen(argv[2]))==0)
	{
		isTagged = FALSE;
	}
	else
	{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}
	#endif 
	nodesave = (unsigned int)(vty->index);
	vlanId = nodesave;

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_TRUNK_MEMBER_UNTAG_TAG_ADD_DEL);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_BYTE,&isAdd,
							 	DBUS_TYPE_UINT16,&trunk_no,
							 	DBUS_TYPE_BYTE,&isTagged,
							 	DBUS_TYPE_UINT16,&vlanId,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
		if (VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret) {
			vty_out(vty,"% Bad parameter,vlan %d not exist.\n",vlanId);
		}
		else if (TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret){
			vty_out(vty,"% Bad parameter,trunk %d not exsit.\n",trunk_no);
		}
		else if (VLAN_RETURN_CODE_TRUNK_EXISTS == op_ret){
			vty_out(vty,"% Bad parameter,trunk %d already vlan %d member.\n",trunk_no,vlanId);
		}
		else if (VLAN_RETURN_CODE_TRUNK_NOTEXISTS == op_ret){
			vty_out(vty,"% Bad parameter,trunk %d was NOT member vlan %d.\n",trunk_no,vlanId);
		}
		else if (VLAN_RETURN_CODE_TRUNK_CONFLICT == op_ret){
			vty_out(vty,"% Bad parameter,trunk %d already untagged member of other active vlan.\n",trunk_no);
		}
		else if (VLAN_RETURN_CODE_TRUNK_MEMBER_NONE == op_ret){
			vty_out(vty,"% Bad parameter,there exists no member in trunk %d.\n",trunk_no);
		}		
		/*else if (NPD_VLAN_TRUNK_MEMBERSHIP_CONFLICT == op_ret) {
		//	vty_out(vty,"% Bad parameter: port Already untagged member of other active vlan.\n");
		//}*/
		else if (VLAN_RETURN_CODE_ERR_HW == op_ret){
			vty_out(vty,"%% Error occurs in config on hardware.\n");
		}	
		else if (VLAN_RETURN_CODE_ERR_NONE == op_ret ) {
			/*vty_out(vty,"%s trunk %d vlan %d ok.\n", isAdd ? "add":"delete",trunk_no,vlanId);*/
		}
		else if (VLAN_RETURN_CODE_TRUNK_MBRSHIP_CONFLICT == op_ret ) {
			vty_out(vty,"% Bad parameter,trunk %d membership conflict.\n", trunk_no);
		} 
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
#else
DEFUN(vlan_trunk_add_del_tag_cmd_func,
	vlan_add_del_trunk_tag_cmd,
	"(add|delete) trunk <1-64> (tag|untag)",
	"Add trunk as vlan member\n"
	"Delete trunk from vlan\n"
	"Trunk that in system\n"
	"Trunk ID range <1-64>\n"
	"Trunk member of vlan support 802.1q\n"
	"Trunk member of vlan not support 802.1q\n"
	)
{

	if(is_distributed == DISTRIBUTED_SYSTEM)	
    {  

        DBusMessage *query = NULL, *reply = NULL;
        DBusError err;
        unsigned short vlanId = 0, trunk_no = 0;
        boolean isAdd 		= FALSE;
        boolean isTagged 	= FALSE;
        unsigned int 	ret = 0, op_ret = 0, nodesave = 0;
        ret = param_first_char_check((char*)argv[0],0);
        if (NPD_FAIL == op_ret) {
        	vty_out(vty,"% Unknow parameter format.\n");
        	return CMD_SUCCESS;
        }
        else if (1 ==ret){
        	isAdd = TRUE;
        }
        else if(0 == ret){
        	isAdd = FALSE;
        }
        /*fetch the 2nd param : slotNo/portNo*/
        op_ret = parse_trunk_no((char *)argv[1],&trunk_no);
        if (NPD_FAIL == op_ret) {
        	vty_out(vty,"% Unknow trunkNo format.\n");
        	return CMD_SUCCESS;
        }
		if((trunk_no<=0) || (trunk_no >64))
		{
			vty_out(vty,"%Please input a correct trunkId between <1-64>\n");
			return CMD_WARNING;
		}
        #if 1
        /*fetch the 3nd param : tagged OR untagged*/
        if(strncmp(argv[2],"tag",strlen(argv[2]))==0)
        {
        	isTagged = TRUE;
        }
        else if (strncmp(argv[2],"untag",strlen(argv[2]))==0)
        {
        	isTagged = FALSE;
        }
        else
        {
        	vty_out(vty,"bad command parameter!\n");
        	return CMD_WARNING;
        }
        #endif 
        nodesave = (unsigned int)(vty->index);
        vlanId = nodesave;

    	int slot_id =0;
    	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
        int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);

        if((local_slot_id<0)||(slot_count<0))
        {
        	vty_out(vty,"get_product_info() return -1,Please check dbm file !\n");
    		return CMD_WARNING;			
        }
		
	    for(slot_id=1;slot_id<=slot_count;slot_id++)
	    {

            query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
            									NPD_DBUS_VLAN_OBJPATH,	\
            									NPD_DBUS_VLAN_INTERFACE,	\
            									NPD_DBUS_VLAN_METHOD_CONFIG_TRUNK_MEMBER_UNTAG_TAG_ADD_DEL);

            dbus_error_init(&err);

            dbus_message_append_args(	query,
            						 	DBUS_TYPE_BYTE,&isAdd,
            						 	DBUS_TYPE_UINT16,&trunk_no,
            						 	DBUS_TYPE_BYTE,&isTagged,
            						 	DBUS_TYPE_UINT16,&vlanId,
            						 	DBUS_TYPE_INVALID);

            if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) 				
			{
    			if(slot_id == local_slot_id)
				{
                    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
				   	vty_out(vty,"Can not connect to slot:%d \n",slot_id);	
					continue;
				}
            }
			else
			{
                reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);				
			}

            dbus_message_unref(query);

        	if (NULL == reply) {
        		vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",slot_id);
        		return CMD_SUCCESS;
        	}
			
            if (dbus_message_get_args ( reply, &err,
            	DBUS_TYPE_UINT32,&op_ret,
            	DBUS_TYPE_INVALID)) {
            	if (VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret) {
            		vty_out(vty,"% Bad parameter,vlan %d not exist.\n",vlanId);
            	}
            	else if (TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret){
            		vty_out(vty,"% Bad parameter,trunk %d not exsit.\n",trunk_no);
            	}
            	else if (VLAN_RETURN_CODE_TRUNK_EXISTS == op_ret){
            		vty_out(vty,"% Bad parameter,trunk %d already vlan %d member.\n",trunk_no,vlanId);
            	}
            	else if (VLAN_RETURN_CODE_TRUNK_NOTEXISTS == op_ret){
            		vty_out(vty,"% Bad parameter,trunk %d was NOT member vlan %d.\n",trunk_no,vlanId);
            	}
            	else if (VLAN_RETURN_CODE_TRUNK_CONFLICT == op_ret){
            		vty_out(vty,"% Bad parameter,trunk %d already untagged member of other active vlan.\n",trunk_no);
            	}
            	else if (VLAN_RETURN_CODE_TRUNK_MEMBER_NONE == op_ret){
            		vty_out(vty,"% Bad parameter,there exists no member in trunk %d.\n",trunk_no);
            	}		
            	/*else if (NPD_VLAN_TRUNK_MEMBERSHIP_CONFLICT == op_ret) {
            	//	vty_out(vty,"% Bad parameter: port Already untagged member of other active vlan.\n");
            	//}*/
            	else if (VLAN_RETURN_CODE_ERR_HW == op_ret){
            		vty_out(vty,"%% Error occurs in config on hardware.\n");
            	}	
            	else if (VLAN_RETURN_CODE_ERR_NONE == op_ret ) {
            		/*vty_out(vty,"%s trunk %d vlan %d ok.\n", isAdd ? "add":"delete",trunk_no,vlanId);*/
            	}
            	else if (VLAN_RETURN_CODE_TRUNK_MBRSHIP_CONFLICT == op_ret ) {
            		vty_out(vty,"% Bad parameter,trunk %d membership conflict.\n", trunk_no);
            	} 
            }
        	else 
        	{
        		vty_out(vty,"Failed get args.Please check npd on slot: %d\n",slot_id);
            	dbus_message_unref(reply);
            	return CMD_SUCCESS;
        	}
            dbus_message_unref(reply);
	    }
		return CMD_SUCCESS;
	}
	else
    {
        DBusMessage *query = NULL, *reply = NULL;
        DBusError err;
        unsigned short vlanId = 0, trunk_no = 0;
        boolean isAdd 		= FALSE;
        boolean isTagged 	= FALSE;
        unsigned int 	ret = 0, op_ret = 0, nodesave = 0;
        ret = param_first_char_check((char*)argv[0],0);
        if (NPD_FAIL == op_ret) {
        	vty_out(vty,"% Unknow parameter format.\n");
        	return CMD_SUCCESS;
        }
        else if (1 ==ret){
        	isAdd = TRUE;
        }
        else if(0 == ret){
        	isAdd = FALSE;
        }
        /*fetch the 2nd param : slotNo/portNo*/
        op_ret = parse_trunk_no((char *)argv[1],&trunk_no);
        if (NPD_FAIL == op_ret) {
        	vty_out(vty,"% Unknow trunkNo format.\n");
        	return CMD_SUCCESS;
        }
        #if 1
        /*fetch the 3nd param : tagged OR untagged*/
        if(strncmp(argv[2],"tag",strlen(argv[2]))==0)
        {
        	isTagged = TRUE;
        }
        else if (strncmp(argv[2],"untag",strlen(argv[2]))==0)
        {
        	isTagged = FALSE;
        }
        else
        {
        	vty_out(vty,"bad command parameter!\n");
        	return CMD_WARNING;
        }
        #endif 
        nodesave = (unsigned int)(vty->index);
        vlanId = nodesave;



        query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
        									NPD_DBUS_VLAN_OBJPATH,	\
        									NPD_DBUS_VLAN_INTERFACE,	\
        									NPD_DBUS_VLAN_METHOD_CONFIG_TRUNK_MEMBER_UNTAG_TAG_ADD_DEL);

        dbus_error_init(&err);

        dbus_message_append_args(	query,
        						 	DBUS_TYPE_BYTE,&isAdd,
        						 	DBUS_TYPE_UINT16,&trunk_no,
        						 	DBUS_TYPE_BYTE,&isTagged,
        						 	DBUS_TYPE_UINT16,&vlanId,
        						 	DBUS_TYPE_INVALID);

        reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

        dbus_message_unref(query);
        if (NULL == reply) {
        	vty_out(vty,"failed get reply.\n");
        	if (dbus_error_is_set(&err)) {
        		printf("%s raised: %s",err.name,err.message);
        		dbus_error_free_for_dcli(&err);
        	}
        	return CMD_SUCCESS;
        }

        if (dbus_message_get_args ( reply, &err,
        	DBUS_TYPE_UINT32,&op_ret,
        	DBUS_TYPE_INVALID)) {
        	if (VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret) {
        		vty_out(vty,"% Bad parameter,vlan %d not exist.\n",vlanId);
        	}
        	else if (TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret){
        		vty_out(vty,"% Bad parameter,trunk %d not exsit.\n",trunk_no);
        	}
        	else if (VLAN_RETURN_CODE_TRUNK_EXISTS == op_ret){
        		vty_out(vty,"% Bad parameter,trunk %d already vlan %d member.\n",trunk_no,vlanId);
        	}
        	else if (VLAN_RETURN_CODE_TRUNK_NOTEXISTS == op_ret){
        		vty_out(vty,"% Bad parameter,trunk %d was NOT member vlan %d.\n",trunk_no,vlanId);
        	}
        	else if (VLAN_RETURN_CODE_TRUNK_CONFLICT == op_ret){
        		vty_out(vty,"% Bad parameter,trunk %d already untagged member of other active vlan.\n",trunk_no);
        	}
        	else if (VLAN_RETURN_CODE_TRUNK_MEMBER_NONE == op_ret){
        		vty_out(vty,"% Bad parameter,there exists no member in trunk %d.\n",trunk_no);
        	}		
        	/*else if (NPD_VLAN_TRUNK_MEMBERSHIP_CONFLICT == op_ret) {
        	//	vty_out(vty,"% Bad parameter: port Already untagged member of other active vlan.\n");
        	//}*/
        	else if (VLAN_RETURN_CODE_ERR_HW == op_ret){
        		vty_out(vty,"%% Error occurs in config on hardware.\n");
        	}	
        	else if (VLAN_RETURN_CODE_ERR_NONE == op_ret ) {
        		/*vty_out(vty,"%s trunk %d vlan %d ok.\n", isAdd ? "add":"delete",trunk_no,vlanId);*/
        	}
        	else if (VLAN_RETURN_CODE_TRUNK_MBRSHIP_CONFLICT == op_ret ) {
        		vty_out(vty,"% Bad parameter,trunk %d membership conflict.\n", trunk_no);
        	} 
        } else {
        	vty_out(vty,"Failed get args.\n");
        	if (dbus_error_is_set(&err)) {
        		printf("%s raised: %s",err.name,err.message);
        		dbus_error_free_for_dcli(&err);
        	}
        }
        dbus_message_unref(reply);
        return CMD_SUCCESS;
    }
}
#endif
/*active protocol vlan/

/* CMD: config one vlan entry by vlanId*/
/* Default-Building VLANs on dev 0(98DX275)*/
DEFUN(config_vlan_active_protvlan_cmd_fun,
	config_vlan_active_protvlan_cmd,
	"config interface protvlan <1-4095>",
	"Configure system interfaces\n"
	"System interfaces\n"
	"Protocol vlan interface\n"
	"VlanId range <1-4095> valid\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;

	unsigned short 	protvlanId;
	int ret;
	unsigned int op_ret;

	/*printf("before parse_vlan_no %s\n",argv[0]);*/
	ret = parse_vlan_no((char*)argv[0],&protvlanId);
	
	if (NPD_FAIL == ret) {
    	vty_out(vty,"Illegal vlanId .\n");
		return CMD_WARNING;
	}
	if (4095 == protvlanId){
		vty_out(vty,"% Bad parameter,Reserved vlan for Layer3 interface of EthPort!\n");
		return CMD_WARNING;
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_PROT_VLAN_ONE );
		
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&protvlanId,
							 DBUS_TYPE_INVALID);
	/*printf("build query for vlanId %d.\n",protvlanId);*/
	
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
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT16, &protvlanId,
					DBUS_TYPE_INVALID)) 
	{
		if (NPD_DBUS_ERROR == op_ret ) 
		{
			vty_out(vty,"Protocal VlanId %d \n",protvlanId);
		}
		else if (NPD_DBUS_ERROR_NO_SUCH_VLAN == op_ret) 
		{
        	vty_out(vty,"ERROR: No such vlan.\n");
		}
		else if(NPD_DBUS_SUCCESS == op_ret)
		{
			/*vty_out(vty,"query reply op %d vlanId %d.\n",op_ret,protvlanId);*/
		}
		else
		{
			/* other error code */
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

/*active super vlan entry*/

/* CMD: config one vlan entry by vlanId*/
/* Default-Building VLANs on dev 0(98DX275)*/
DEFUN(config_vlan_active_supervlan_cmd_fun,
	config_vlan_active_supervlan_cmd,
	"config interface supervlan <1-4095>",
	"Configure system interfaces\n"
	"System interfaces\n"
	"Super vlan interface\n"
	"VlanId range <1-4095> valid\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;

	unsigned short 	supervid;
	int ret;
	unsigned int op_ret;

	/*printf("defore parse_vlan_no %s\n",argv[0]);*/
	ret = parse_vlan_no((char*)argv[0],&supervid);
	
	if (NPD_FAIL == ret) {
    	vty_out(vty,"Illegal vlanId .\n");
		return CMD_SUCCESS;
	}
	if (4095 == supervid){
		vty_out(vty,"% Bad parameter,Reserved vlan for Layer3 interface of EthPort!\n");
		return CMD_WARNING;
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_SUPER_VLAN_ONE );
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&supervid,
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
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT16, &supervid,
					DBUS_TYPE_INVALID)) 
	{
		if (NPD_DBUS_ERROR == op_ret ) {
			vty_out(vty,"Super VlanId %d \n",supervid);
		}
		else if (NPD_DBUS_ERROR_NO_SUCH_VLAN == op_ret)	{
        	vty_out(vty,"ERROR: No such vlan.\n");
		}
		else if(NPD_DBUS_SUCCESS == op_ret)	{
			vty_out(vty,"query reply op %d vlanId %d.\n",op_ret,supervid);
		}
		else {
			/* other error code */
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


/*add or delete protocol Encapsulation Type Value*/

/*CMD :for add or delete port to (from) vlan*/
DEFUN(config_vlan_add_del_prot_ethertype_cmd_func,
	config_vlan_add_del_prot_ethertype_cmd,
	"(add|delete) protocol (dix|snap|llc) <ether_type>",
	"Add subvlan to super vlan\n"
	"Delete subvlan from super vlan\n"
	"DIX-EthernetII Protocol encapsulatio\n"
	"SNAP-802.3 LLC/SNAP Protocol encapsulation\n"
	"LLC--802.3 non-SNAP LLC Protocol encapsulation\n"
	"EtherType-Protocol Value compared with Golbal Prot Table\n"
	)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short etherType;
	boolean isAdd 		= FALSE;
	boolean isTagged 	= FALSE;
	unsigned short	vlanId;
	unsigned int 	op_ret;

	/*fetch the 1st param : add ? delete*/
	if(strcmp(argv[0],"add")==0)
	{
		isAdd = TRUE;
	}
	else if (strcmp(argv[0],"delete")==0)
	{
		isAdd= FALSE;
	}
	else
	{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}		

	/*fetch the 2nd param : slotNo/portNo*/
	op_ret = parse_single_param_no((char *)argv[1],&etherType);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"Unknow portno format.\n");
		return CMD_SUCCESS;
	}
	else
	{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}
	vty_out(vty,"%s ",isAdd ? "Add" : "Delete");
	vty_out(vty,"ether Type %d ",etherType);



	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_PROT_ETHER_ADD_DEL);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_BYTE,&isAdd,
								DBUS_TYPE_BYTE,&etherType,
							 	DBUS_TYPE_BYTE,&isTagged,
							 	DBUS_TYPE_UINT16,&vlanId,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if (NPD_DBUS_SUCCESS == op_ret ) {
				vty_out(vty,"Ethernet sub-vlan %d-%d index %#0x\n",etherType);
			}
			if (NPD_DBUS_ERROR_NO_SUCH_VLAN== op_ret) {
            	vty_out(vty,"ERROR: No such port.\n");
			}
		
	} else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/* show super vlan*/
/* super vlan : static | dynamic, super vlan IP, Port_no members, Mac Addrs*/
DEFUN(show_supervlan_cmd_fun,
	show_supervlan_cmd,
	"show interface supervlan <1-4095>",
	"Show system information\n"
	"System special interface,sunch as: eth_port;vlan;L3_intf\n"
	"Super Vlan interface\n"
	"VlanId range in <1-4095> valid\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;

	unsigned short 	vlanId;
	int ret;
	unsigned int op_ret;

	/*printf("before parse_vlan_no %s\n",argv[0]);*/
	ret = parse_vlan_no((char*)argv[0],&vlanId);
	
	if (NPD_FAIL == ret) {
    	vty_out(vty,"Illegal vlanId .\n");
		return CMD_SUCCESS;
	}
	if (4095 == vlanId){
		vty_out(vty,"% Bad parameter,Reserved vlan for Layer3 interface of EthPort!\n");
		return CMD_WARNING;
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_SHOW_SUPERVLAN_ENTRY );
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vlanId,
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
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_UINT16, &vlanId,
					DBUS_TYPE_INVALID)) 
	{
		if (NPD_DBUS_ERROR == op_ret ) {
			vty_out(vty,"VlanId %d \n",vlanId);
		}
		else if (NPD_DBUS_ERROR_NO_SUCH_VLAN == op_ret) {
        	vty_out(vty,"ERROR: No such vlan.\n");
		}
		else if(NPD_DBUS_SUCCESS == op_ret) {
			vty_out(vty,"query reply op %d vlanId %d.\n",op_ret,vlanId);
		}
		else {
			/* other error code */
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

/*show members slot/port of a specific vlan */
/*CMD :for show the port members of the specific vlan*/
DEFUN(show_vlan_port_member_cmd_fun,
	show_vlan_port_member_cmd,
	"show vlan <1-4093> port-member",
	"Show system information\n"
	"Vlan entity\n"
	"VLan ID range <1-4093> valid\n"
	"Port members in this vlan\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	
	DBusMessageIter	 iter_array;
	unsigned short	vlanId = 0;
	char*			vlanName = NULL;
	unsigned int	count = 0, ret = 0;
	unsigned int 	i = 0, row = 0;
	unsigned int slot = 0, port = 0, tmpVal = 0;
	unsigned int product_id = 0; 
	unsigned int vlanStat = 0;
	unsigned int bitOffset = 0, mbrCount = 0;
	unsigned int promisPortBmp[2] = {0};
	
	PORT_MEMBER_BMP untagPortBmp,tagPortBmp;
	memset(&untagPortBmp,0,sizeof(PORT_MEMBER_BMP));
	memset(&tagPortBmp,0,sizeof(PORT_MEMBER_BMP));
	ret = parse_vlan_no((char*)argv[0],&vlanId);

	if (NPD_FAIL == ret) {
		vty_out(vty,"% Bad parameter :vlan ID illegal!\n");
		return CMD_SUCCESS;
	}

	if(is_distributed == DISTRIBUTED_SYSTEM)	
	{
        /* read vlan info from file: shm_vlan ,zhangdi@autelan.com */		
		int	count_tag =0;
		int	count_untag =0;		
		int count_bond =0;
        int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);
		
		int fd = -1, j=0, m=0, n=0;
		struct stat sb;

		vlan_list_t * mem_vlan_list =NULL;
		char* file_path = "/dbm/shm/vlan/shm_vlan";
		/* only read file */
	    fd = open(file_path, O_RDONLY);
    	if(fd < 0)
        {
            vty_out(vty,"Failed to open! %s\n", strerror(errno));
            return NULL;
        }
		fstat(fd,&sb);
		/* map not share */	
        mem_vlan_list = (vlan_list_t *)mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0 );
        if(MAP_FAILED == mem_vlan_list)
        {
            vty_out(vty,"Failed to mmap for g_vlanlist[]! %s\n", strerror(errno));
			close(fd);
            return NULL;
        }	

        if(mem_vlan_list[vlanId-1].vlanStat == 1)
		{
        	vty_out(vty,"Codes:  U - vlan link status is up,     D - vlan link status is down, \n");
        	vty_out(vty,"        u - untagged port member,       t - tagged port member, \n");
        	vty_out(vty,"        f - first cpu on board,         s - second cpu on board, \n");
        	vty_out(vty,"        * - promiscuous mode port member\n");
        	vty_out(vty,"========================================================================\n");
        	vty_out(vty,"%-7s  %-20s  %-40s\n","VLAN ID","VLAN NAME","PORT MEMBER LIST");
        	vty_out(vty,"=======  ====================  =========================================\n");				
			
			vty_out(vty,"%-4d(%s)  ",vlanId,mem_vlan_list[vlanId-1].updown ? "U":"D");			
			vty_out(vty,"%-20s	",mem_vlan_list[vlanId-1].vlanName);

			count_tag =0;
			for(i = 0; i < slot_count; i++ )
			{
    			for(j = 0; j < 64; j++ )
    			{
					if(j<32)
					{
                        if((mem_vlan_list[vlanId-1].untagPortBmp[i].low_bmp)&(1<<j))
                    	{
    						slot = i+1;
    						port = j+1;
                			if((count_tag!=0) && (0 == count_tag % 4)) 
                			{
                				vty_out(vty,"\n%-32s"," ");
                			}
                			vty_out(vty,"%s%d/%d(u)",(count_tag%4) ? ",":"",slot,port);
                			count_tag++;						
                    	}
    					else
    					{
    						continue;
    					}
					}
					else
					{
                        if((mem_vlan_list[vlanId-1].untagPortBmp[i].high_bmp)&(1<<(j-32)))
                    	{
    						slot = i+1;
    						port = j+1;
                			if((count_tag!=0) && (0 == count_tag % 4)) 
                			{
                				vty_out(vty,"\n%-32s"," ");
                			}
                			vty_out(vty,"%s%d/%d(u)",(count_tag%4) ? ",":"",slot,port);
                			count_tag++;						
                    	}
    					else
    					{
    						continue;
    					}								
					}
    			}				
			}

			count_untag = 0;
			for(i = 0; i < slot_count; i++ )
			{
    			for(j = 0; j < 64; j++ )
    			{
					if(j<32)
					{
                        if((mem_vlan_list[vlanId-1].tagPortBmp[i].low_bmp)&(1<<j))
                    	{
    						slot = i+1;
    						port = j+1;
                            /* add '\n' between tag and untag list  */
            				if(count_tag != 0)
            				{
            					vty_out(vty,"\n%-32s"," ");
            					count_tag = 0;     /* change count_temp back to 0 for next use */
            				}								
                			if((count_untag != 0)&&(0 == (count_untag % 4)))    /* tag port start at new line */
                			{
                				vty_out(vty,"\n%-32s"," ");
                			}
                			vty_out(vty,"%s%d/%d(t)",(count_untag%4) ? ",":"",slot,port);
                			count_untag++;													
                    	}
    					else
    					{
    						continue;
    					}													
					}
					else
					{
                        if((mem_vlan_list[vlanId-1].tagPortBmp[i].high_bmp)&(1<<(j-32)))
                    	{
    						slot = i+1;
    						port = j+1;
                            /* add '\n' between tag and untag list  */
            				if(count_tag != 0)
            				{
            					vty_out(vty,"\n%-32s"," ");
            					count_tag = 0;     /* change count_temp back to 0 */
            				}								
                			if((count_untag != 0)&&(0 == (count_untag % 4)))    /* tag port start at new line */
                			{
                				vty_out(vty,"\n%-32s"," ");
                			}
                			vty_out(vty,"%s%d/%d(t)",(count_untag%4) ? ",":"",slot,port);
                			count_untag++;													
                    	}
    					else
    					{
    						continue;
    					}									
					}
    			}				
			}
			vty_out(vty,"\n");

            /* print which slot this vlan bonded to */
			count_bond = 0;
			for(m = 0; m < slot_count; m++ )
			{   
				if(mem_vlan_list[vlanId-1].bond_slot[m] != 0)
				{
					if(count_bond == 0)   /* print one times */
					{
        				vty_out(vty,"Bonded to slot : ");	
					}
					count_bond++;
					/* first cpu 8 ports + second cpu 8 ports */
					for(n=0;n<16;n++)
					{
						if((mem_vlan_list[vlanId-1].bond_slot[m])&(0x1<<n))   /* if !=0, the bit is 1 */
						{	
                    		vty_out(vty,"%d(%c%d) ",m+1,(n<8)?'f':'s',(n%8)+1);
						}   						
					}
				}
    		}
			if(count_bond != 0)
			{
			    vty_out(vty,"\n");	
			}			
       		vty_out(vty,"========================================================================\n");		
		}
	    else
		{
            /* do nothing */
    		vty_out(vty,"% Bad parameter: vlan is NOT exists!\n");
		}

		/* munmap and close fd */
        ret = munmap(mem_vlan_list,sb.st_size);
        if( ret != 0 )
        {
            vty_out(vty,"Failed to munmap for g_vlanlist[]! %s\n", strerror(errno));			
        }	
		ret = close(fd);
		if( ret != 0 )
        {
            vty_out(vty,"close shm_vlan failed \n" );   
        }
        /*free(vlanName);*/
        return CMD_SUCCESS;				
	}	

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,\
										NPD_DBUS_VLAN_METHOD_SHOW_VLAN_PORT_MEMBERS_V1 );
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vlanId,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &ret,
					DBUS_TYPE_UINT32, &product_id,
					DBUS_TYPE_UINT32, &promisPortBmp[0],
					DBUS_TYPE_UINT32, &promisPortBmp[1],
					DBUS_TYPE_STRING, &vlanName,
					DBUS_TYPE_UINT32, &untagPortBmp.portMbr[0],
					DBUS_TYPE_UINT32, &untagPortBmp.portMbr[1],
					DBUS_TYPE_UINT32, &tagPortBmp.portMbr[0],
					DBUS_TYPE_UINT32, &tagPortBmp.portMbr[1],
					DBUS_TYPE_UINT32, &vlanStat,
					DBUS_TYPE_INVALID)) {
		/*vty_out(vty,"ret %d,vlanName %s,untagBmp %#0X, tagBmp %#0X,trunkId %d.\n",ret,vlanName,untagBmp,tagBmp,trunkId);*/
		if(VLAN_RETURN_CODE_ERR_NONE == ret) {
			vty_out(vty,"Codes:  U - vlan link status is up,   D - vlan link status is down, \n");
			vty_out(vty,"        u - untagged port member,      t - tagged port member, \n");
			vty_out(vty,"        * - promiscuous mode port member\n");
			vty_out(vty,"========================================================================\n");
			vty_out(vty,"%-7s  %-20s  %-40s\n","VLAN ID","VLAN NAME","PORT MEMBER LIST");
			vty_out(vty,"=======  ====================  =========================================\n");
			
			vty_out(vty,"%-4d(%s)  ",vlanId,vlanStat ? "U":"D");
			vty_out(vty,"%-20s	",vlanName);

			if(0 != untagPortBmp.portMbr[0] || 0 != tagPortBmp.portMbr[0] || 0 != untagPortBmp.portMbr[1] || 0 != tagPortBmp.portMbr[1]){
				show_vlan_member_slot_port(vty,product_id,untagPortBmp,tagPortBmp,promisPortBmp);
			}
			else 
				vty_out(vty,"  %-40s","No Port member.");
			vty_out(vty,"\n");
			vty_out(vty,"========================================================================\n");
		}
		else if(ETHPORT_RETURN_CODE_NO_SUCH_VLAN == ret) {
			vty_out(vty,"% Bad parameter,vlan id illegal.\n");
		}
		else if(VLAN_RETURN_CODE_VLAN_NOT_EXISTS == ret) {
			vty_out(vty,"% Bad parameter,vlan %d not exist.\n",vlanId);
		}
		else if(VLAN_RETURN_CODE_ERR_GENERAL == ret) {
			vty_out(vty,"%% Error,operation on software fail.\n");
		}
	}
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	
	dbus_message_unref(reply);
	/*free(vlanName);*/
	return CMD_SUCCESS;
}

/*show trunk members of a specific vlan */
/*CMD :for show the trunk members of the specific vlan*/

DEFUN(show_vlan_trunk_member_cmd_fun,
	show_vlan_trunk_member_cmd,
	"show vlan <1-4093> trunk-member",
	"Show system information\n"
	"Vlan entity\n"
	"VLan ID range <1-4093> valid\n"
	"Trunk members in this vlan\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	
	DBusMessageIter	 iter_array;
	unsigned short	vlanId = 0;
	char*			vlanName = NULL;
	unsigned int vlan_Cont = 0;
	unsigned int count = 0, ret = 0;
	unsigned int i = 0, j = 0;
	unsigned int slot = 0, port = 0, tmpVal = 0;
	unsigned int product_id = 0;
	unsigned int bitOffset = 0, mbrCount = 0;
	unsigned int untagTrkBmp[4] = {0}, tagTrkBmp[4] = {0};
	unsigned int vlanStat = 0;
	
	ret = parse_vlan_no((char*)argv[0],&vlanId);

	if (NPD_FAIL == ret) {
		vty_out(vty,"% Bad parameter,vlan ID illegal.\n");
		return CMD_SUCCESS;
	}

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,\
										NPD_DBUS_VLAN_METHOD_SHOW_VLAN_TRUNK_MEMBERS );
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vlanId,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &ret,
					DBUS_TYPE_UINT32, &product_id,
					DBUS_TYPE_STRING, &vlanName,
					DBUS_TYPE_UINT32, &untagTrkBmp[0],
					DBUS_TYPE_UINT32, &untagTrkBmp[1],
					DBUS_TYPE_UINT32, &untagTrkBmp[2],
					DBUS_TYPE_UINT32, &untagTrkBmp[3],
					DBUS_TYPE_UINT32, &tagTrkBmp[0],
					DBUS_TYPE_UINT32, &tagTrkBmp[1],
					DBUS_TYPE_UINT32, &tagTrkBmp[2],
					DBUS_TYPE_UINT32, &tagTrkBmp[3],					
					DBUS_TYPE_UINT32, &vlanStat,
					DBUS_TYPE_INVALID)){	

		if(VLAN_RETURN_CODE_ERR_NONE == ret){
			vty_out(vty,"========================================================================\n");
			vty_out(vty,"%-7s  %-20s  %-40s\n","VLAN ID","VLAN NAME","TRUNK MEMBER LIST");
			vty_out(vty,"=======  ====================  =========================================\n");	
			
			vty_out(vty,"%-4d(%s)  ",vlanId,vlanStat ? "U" : "D");
			vty_out(vty,"%-20s	",vlanName);
			for(i =0; i<4; i++) {
				if(0 != untagTrkBmp[i] || 0 != tagTrkBmp[i]) {
					for(bitOffset = 0;bitOffset < 32;bitOffset++) {
						if(untagTrkBmp[i] & (1<<bitOffset)) {
							if(0 == mbrCount % 4 && 0 != mbrCount){
								vty_out(vty,"\n%-32s"," ");
							}	
							mbrCount++;
							vty_out(vty,"%s%s%d(u)",((mbrCount-1)%4)?",":"","trunk",(i*32 + bitOffset+1));
						}
						else 
							continue;
					}
					for(bitOffset = 0;bitOffset < 32;bitOffset++) {
						if(tagTrkBmp[i] & (1<<bitOffset)) {
							if(0 == mbrCount % 4 && 0 != mbrCount){
								vty_out(vty,"\n%-32s"," ");
							}	
							mbrCount++;
							vty_out(vty,"%s%s%d(t)",((mbrCount-1)%4)?",":"","trunk",(i*32 + bitOffset+1));
						}
						else
							continue;
					}
				}
				else 
					continue;
			}
			if(0 == mbrCount){ 
				vty_out(vty,"  %-40s","No Trunk member.");
			}	
			vty_out(vty,"\n");	
			vty_out(vty,"========================================================================\n");
		}
		else if(ETHPORT_RETURN_CODE_NO_SUCH_VLAN == ret) {
			vty_out(vty,"% Bad parameter,vlan id illegal.\n");
		}		
		else if(VLAN_RETURN_CODE_VLAN_NOT_EXISTS == ret) {
			vty_out(vty,"% Bad parameter: vlan %d not exist.\n",vlanId);
		}
		else if(COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
			vty_out(vty,"%% Product not support this function!\n");
		}
	}
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	
	dbus_message_unref(reply);
	/*free(vlanName);*/
	return CMD_SUCCESS;
}

/*show port members of a specific vlan */
/*CMD :for show the port members of the specific vlan*/
/*vlan 4095 for L3 interface of Eth_port CAN only be show be name.Do you its name?*/
DEFUN(show_vlan_port_member_vname_cmd_fun,
	show_vlan_port_member_vname_cmd,
	"show vlan VLANNAME port-member",
	"Show system information\n"
	"Vlan entity\n"
	"Specify vlan name,beginning of char or'_', and no more than 32 characters\n"
	"Port members in this vlan\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	
	DBusMessageIter	 iter_array;
	unsigned short	vlanId = 0;
	char*			vlanName =NULL;
	unsigned int	nameSize = 0;
	unsigned int	count = 0, ret = 0;
	unsigned int 	i = 0;
	/*unsigned int untagPortBmp[2] = {0},tagPortBmp[2] = {0};*/
	unsigned int slot = 0,port = 0,tmpVal = 0;
	unsigned int product_id = 0; 
	unsigned int vlanStat = 0;
	unsigned int promisPortBmp[2] = {0};
	unsigned int bitOffset = 0,mbrCount =0;
	PORT_MEMBER_BMP untagPortBmp,tagPortBmp;
	memset(&untagPortBmp,0,sizeof(PORT_MEMBER_BMP));
	memset(&tagPortBmp,0,sizeof(PORT_MEMBER_BMP));
	/*printf("get %d param(s) from dcli Input.\n",argc);*/

	/*get vlan name*/
	/*printf("before parse_vlan_name %s length %d\n",argv[1],strlen(argv[1]));*/
	vlanName = (char*)malloc(ALIAS_NAME_SIZE);
	memset(vlanName,0,ALIAS_NAME_SIZE);

	ret = vlan_name_legal_check((char*)argv[0],strlen(argv[0]));
	if(ALIAS_NAME_LEN_ERROR == ret) {
		vty_out(vty,"% Bad parameter,vlan name too long!\n");
		return CMD_WARNING;
	}
	else if(ALIAS_NAME_HEAD_ERROR == ret) {
		vty_out(vty,"% Bad parameter,vlan name begins with an illegal char!\n");
		return CMD_WARNING;
	}
	else if(ALIAS_NAME_BODY_ERROR == ret) {
		vty_out(vty,"% Bad parameter,vlan name contains illegal char!\n");
		return CMD_WARNING;
	}
	else {
		/*vlan name illegal ,it'll not send message to npd*/
		nameSize = strlen(argv[0]);
		memcpy(vlanName,argv[0],nameSize);
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_VLAN_OBJPATH ,\
											NPD_DBUS_VLAN_INTERFACE ,	\
											NPD_DBUS_VLAN_METHOD_SHOW_VLAN_PORT_MEMBERS_VNAME_V1 );
		

		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_STRING,&vlanName,
								 DBUS_TYPE_INVALID);
		
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	}
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &ret,
					DBUS_TYPE_UINT32, &product_id,
					DBUS_TYPE_UINT32, &promisPortBmp[0],
					DBUS_TYPE_UINT32, &promisPortBmp[1],
					DBUS_TYPE_UINT16, &vlanId,
					DBUS_TYPE_UINT32, &untagPortBmp.portMbr[0],
					DBUS_TYPE_UINT32, &untagPortBmp.portMbr[1],
					DBUS_TYPE_UINT32, &tagPortBmp.portMbr[0],
					DBUS_TYPE_UINT32, &tagPortBmp.portMbr[1],
					DBUS_TYPE_UINT32, &vlanStat,
					DBUS_TYPE_INVALID)) { 
		if(VLAN_RETURN_CODE_ERR_NONE == ret) {
			vty_out(vty,"Codes:  U - vlan link status is up,   D - vlan link status is down, \n");
			vty_out(vty,"		 u - untagged port member,	   t - tagged port member, \n");
        	vty_out(vty,"        f - first cpu on board,         s - second cpu on board, \n");			
			vty_out(vty,"		 * - promiscuous mode port member\n");
			vty_out(vty,"========================================================================\n");
			vty_out(vty,"%-7s  %-20s  %-40s\n","VLAN ID","VLAN NAME","PORT MEMBER LIST");
			vty_out(vty,"=======  ====================  =========================================\n");
			
			vty_out(vty,"%-4d(%s)  ",vlanId,vlanStat ? "U" : "D");
			vty_out(vty,"%-20s	",vlanName);

			if(0 != untagPortBmp.portMbr[0] || 0 != tagPortBmp.portMbr[0] || 0 != untagPortBmp.portMbr[1] || 0 != tagPortBmp.portMbr[1]){
				show_vlan_member_slot_port(vty,product_id,untagPortBmp,tagPortBmp,promisPortBmp);
			}
			else 
				vty_out(vty,"  %-40s","No Port member.");
			vty_out(vty,"\n");
			vty_out(vty,"========================================================================\n");
		}
		else if(VLAN_RETURN_CODE_VLAN_NOT_EXISTS == ret) {
			vty_out(vty,"%% Bad parameter: vlan %s NOT Exists.\n",vlanName);
		}
		else if(VLAN_RETURN_CODE_ERR_GENERAL == ret) {
			vty_out(vty,"%% Error: operation on software Fail.\n");
		}
		else if(COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
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
	/*free(vlanName);*/
	dbus_message_unref(reply);
	free(vlanName);
	return CMD_SUCCESS;
}

/*show trunk members of a specific vlan */
/*CMD :for show the trunk members of the specific vlan*/
DEFUN(show_vlan_trunk_member_vname_cmd_fun,
	show_vlan_trunk_member_vname_cmd,
	"show vlan VLANNAME trunk-member",
	"Show system information\n"
	"Vlan entity\n"
	"Specify vlan name,beginning of char or'_', and no more than 32 characters\n"
	"Trunk members in this vlan\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	
	DBusMessageIter	 iter_array;
	unsigned short	vlanId = 0;
	char*			vlanName =NULL;
	unsigned int	nameSize = 0;
	unsigned int	count = 0, ret = 0;
	unsigned int 	i = 0;
	unsigned int slot = 0,port = 0,tmpVal = 0;
	unsigned int product_id = 0; 
	unsigned int vlanStat = 0;
	unsigned int bitOffset = 0, mbrCount = 0;
	unsigned int untagTrkBmp[4] = {0}, tagTrkBmp[4] = {0};
	/*printf("get %d param(s) from dcli Input.\n",argc);*/

	/*get vlan name*/
	/*printf("before parse_vlan_name %s length %d\n",argv[1],strlen(argv[1]));*/
	vlanName = (char*)malloc(ALIAS_NAME_SIZE);
	memset(vlanName,0,ALIAS_NAME_SIZE);

	ret = vlan_name_legal_check((char*)argv[0],strlen(argv[0]));
	if(ALIAS_NAME_LEN_ERROR == ret) {
		vty_out(vty,"% Bad parameter,vlan name too long!\n");
		return CMD_WARNING;
	}
	else if(ALIAS_NAME_HEAD_ERROR == ret) {
		vty_out(vty,"% Bad parameter,vlan name begins with an illegal char!\n");
		return CMD_WARNING;
	}
	else if(ALIAS_NAME_BODY_ERROR == ret) {
		vty_out(vty,"% Bad parameter,vlan name contains illegal char!\n");
		return CMD_WARNING;
	}
	else {
		/*vlan name illegal ,it'll not send message to npd*/
		nameSize = strlen(argv[0]);
		memcpy(vlanName,argv[0],nameSize);
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_VLAN_OBJPATH ,\
											NPD_DBUS_VLAN_INTERFACE ,	\
											NPD_DBUS_VLAN_METHOD_SHOW_VLAN_TRUNK_MEMBERS_VNAME );
		

		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_STRING,&vlanName,
								 DBUS_TYPE_INVALID);
		
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	}
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &ret,
					DBUS_TYPE_UINT32, &product_id,
					DBUS_TYPE_UINT16, &vlanId,
					DBUS_TYPE_UINT32, &untagTrkBmp[0],
					DBUS_TYPE_UINT32, &untagTrkBmp[1],
					DBUS_TYPE_UINT32, &untagTrkBmp[2],
					DBUS_TYPE_UINT32, &untagTrkBmp[3],
					DBUS_TYPE_UINT32, &tagTrkBmp[0],
					DBUS_TYPE_UINT32, &tagTrkBmp[1],
					DBUS_TYPE_UINT32, &tagTrkBmp[2],
					DBUS_TYPE_UINT32, &tagTrkBmp[3],
					DBUS_TYPE_UINT32, &vlanStat,
					DBUS_TYPE_INVALID)) {	
		if(VLAN_RETURN_CODE_ERR_NONE == ret){
			vty_out(vty,"========================================================================\n");
			vty_out(vty,"%-7s  %-20s  %-40s\n","VLAN ID","VLAN NAME","TRUNK MEMBER LIST");
			vty_out(vty,"=======  ====================  =========================================\n");	
			
			vty_out(vty,"%-4d(%s)  ",vlanId,vlanStat ? "U" : "D");
			vty_out(vty,"%-20s	",vlanName);
			for(i =0; i<4; i++) {
				if(0 != untagTrkBmp[i] || 0 != tagTrkBmp[i]) {
					for(bitOffset = 0;bitOffset < 32;bitOffset++) {
						if(untagTrkBmp[i] & (1<<bitOffset)) {
							if(0 == mbrCount % 4 && 0 != mbrCount){
								vty_out(vty,"\n%-32s"," ");
							}	
							mbrCount++;
							vty_out(vty,"%s%s%d(u)",((mbrCount-1)%4)?",":"","trunk",(i*32 + bitOffset+1));
						}
						else 
							continue;
					}
					for(bitOffset = 0;bitOffset < 32;bitOffset++) {
						if(tagTrkBmp[i] & (1<<bitOffset)) {
							if(0 == mbrCount % 4 && 0 != mbrCount){
								vty_out(vty,"\n%-32s"," ");
							}	
							mbrCount++;
							vty_out(vty,"%s%s%d(t)",((mbrCount-1)%4)?",":"","trunk",(i*32 + bitOffset+1));
						}
						else
							continue;
					}
				}
				else 
					continue;
			}
			if(0 == mbrCount){ 
				vty_out(vty,"  %-40s","No trunk member.");
			}	
			vty_out(vty,"\n");	
			vty_out(vty,"========================================================================\n");
		}
		else if(VLAN_RETURN_CODE_VLAN_NOT_EXISTS == ret) {
			vty_out(vty,"% Bad parameter,vlan %s not exist.\n",vlanName);
		}
		else if(COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
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
	/*free(vlanName);*/
	dbus_message_unref(reply);
	free(vlanName);
	return CMD_SUCCESS;
}

/***********************************
*show corrent vlan.
*Execute at Vlan-config Node
*CMD : show vlan port
*show port members of corrent vlan 
***********************************/
DEFUN(show_corrent_vlan_port_member_cmd_fun,
	show_corrent_vlan_port_member_cmd,
	"show vlan port-member",
	"Show system information\n"
	"Vlan entity\n"
	"Port members in this vlan\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	
	DBusMessageIter	 iter_array;
	unsigned short	vlanId = 0;
	char*			vlanName = NULL;
	unsigned int	count = 0, ret = 0;
	unsigned int 	i = 0, row = 0;
	/*unsigned int untagPortBmp[2] = {0},tagPortBmp[2] = {0};*/
	unsigned int slot = 0, port = 0, tmpVal = 0;
	unsigned int product_id = 0, nodesave = 0; 
	unsigned short trunkId = 0;
	unsigned int vlanStat = 0;
	unsigned char  trkTagMode = 0;
	unsigned int bitOffset = 0, mbrCount = 0;
	unsigned int promisPortBmp[2] = {0};
	
	PORT_MEMBER_BMP untagPortBmp,tagPortBmp;
	memset(&untagPortBmp,0,sizeof(PORT_MEMBER_BMP));
	memset(&tagPortBmp,0,sizeof(PORT_MEMBER_BMP));

	nodesave = (unsigned int)(vty->index);
	vlanId = (unsigned short)nodesave;
	if(is_distributed == DISTRIBUTED_SYSTEM)	
	{
        /* read vlan info from file: shm_vlan ,zhangdi@autelan.com */		
		int	count_tag =0;
		int	count_untag =0;		
		int count_bond =0;
        int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);
		
		int fd = -1, j=0, m=0, n=0;
		struct stat sb;

		vlan_list_t * mem_vlan_list =NULL;
		char* file_path = "/dbm/shm/vlan/shm_vlan";
		/* only read file */
	    fd = open(file_path, O_RDONLY);
    	if(fd < 0)
        {
            vty_out(vty,"Failed to open! %s\n", strerror(errno));
            return NULL;
        }
		fstat(fd,&sb);
		/* map not share */	
        mem_vlan_list = (vlan_list_t *)mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0 );
        if(MAP_FAILED == mem_vlan_list)
        {
            vty_out(vty,"Failed to mmap for g_vlanlist[]! %s\n", strerror(errno));
			close(fd);
            return NULL;
        }	
    	vty_out(vty,"Codes:  U - vlan link status is up,     D - vlan link status is down, \n");
    	vty_out(vty,"        u - untagged port member,       t - tagged port member, \n");
       	vty_out(vty,"        f - first cpu on board,         s - second cpu on board, \n");		
    	vty_out(vty,"        * - promiscuous mode port member\n");
    	vty_out(vty,"========================================================================\n");
    	vty_out(vty,"%-7s  %-20s  %-40s\n","VLAN ID","VLAN NAME","PORT MEMBER LIST");
    	vty_out(vty,"=======  ====================  =========================================\n");				

        if(mem_vlan_list[vlanId-1].vlanStat == 1)
		{
			vty_out(vty,"%-4d(%s)  ",vlanId,mem_vlan_list[vlanId-1].updown ? "U":"D");			
			vty_out(vty,"%-20s	",mem_vlan_list[vlanId-1].vlanName);

			count_tag =0;
			for(i = 0; i < slot_count; i++ )
			{
    			for(j = 0; j < 64; j++ )
    			{
					if(j<32)
					{
                        if((mem_vlan_list[vlanId-1].untagPortBmp[i].low_bmp)&(1<<j))
                    	{
    						slot = i+1;
    						port = j+1;
                			if((count_tag!=0) && (0 == count_tag % 4)) 
                			{
                				vty_out(vty,"\n%-32s"," ");
                			}
                			vty_out(vty,"%s%d/%d(u)",(count_tag%4) ? ",":"",slot,port);
                			count_tag++;						
                    	}
    					else
    					{
    						continue;
    					}
					}
					else
					{
                        if((mem_vlan_list[vlanId-1].untagPortBmp[i].high_bmp)&(1<<(j-32)))
                    	{
    						slot = i+1;
    						port = j+1;
                			if((count_tag!=0) && (0 == count_tag % 4)) 
                			{
                				vty_out(vty,"\n%-32s"," ");
                			}
                			vty_out(vty,"%s%d/%d(u)",(count_tag%4) ? ",":"",slot,port);
                			count_tag++;						
                    	}
    					else
    					{
    						continue;
    					}								
					}
    			}				
			}

			count_untag = 0;
			for(i = 0; i < slot_count; i++ )
			{
    			for(j = 0; j < 64; j++ )
    			{
					if(j<32)
					{
                        if((mem_vlan_list[vlanId-1].tagPortBmp[i].low_bmp)&(1<<j))
                    	{
    						slot = i+1;
    						port = j+1;
                            /* add '\n' between tag and untag list  */
            				if(count_tag != 0)
            				{
            					vty_out(vty,"\n%-32s"," ");
            					count_tag = 0;     /* change count_temp back to 0 for next use */
            				}								
                			if((count_untag != 0)&&(0 == (count_untag % 4)))    /* tag port start at new line */
                			{
                				vty_out(vty,"\n%-32s"," ");
                			}
                			vty_out(vty,"%s%d/%d(t)",(count_untag%4) ? ",":"",slot,port);
                			count_untag++;													
                    	}
    					else
    					{
    						continue;
    					}													
					}
					else
					{
                        if((mem_vlan_list[vlanId-1].tagPortBmp[i].high_bmp)&(1<<(j-32)))
                    	{
    						slot = i+1;
    						port = j+1;
                            /* add '\n' between tag and untag list  */
            				if(count_tag != 0)
            				{
            					vty_out(vty,"\n%-32s"," ");
            					count_tag = 0;     /* change count_temp back to 0 */
            				}								
                			if((count_untag != 0)&&(0 == (count_untag % 4)))    /* tag port start at new line */
                			{
                				vty_out(vty,"\n%-32s"," ");
                			}
                			vty_out(vty,"%s%d/%d(t)",(count_untag%4) ? ",":"",slot,port);
                			count_untag++;													
                    	}
    					else
    					{
    						continue;
    					}									
					}
    			}				
			}
			vty_out(vty,"\n");
            /* print which slot this vlan bonded to */
			count_bond = 0;
			for(m = 0; m < slot_count; m++ )
			{   
				if(mem_vlan_list[vlanId-1].bond_slot[m] != 0)
				{
					if(count_bond == 0)   /* print one times */
					{
        				vty_out(vty,"Bonded to slot : ");	
					}
					count_bond++;
					/* first cpu 8 ports + second cpu 8 ports */
					for(n=0;n<16;n++)
					{
						if((mem_vlan_list[vlanId-1].bond_slot[m])&(0x1<<n))   /* if !=0, the bit is 1 */
						{	
                    		vty_out(vty,"%d(%c%d) ",m+1,(n<8)?'f':'s',(n%8)+1);
						}   						
					}
				}
    		}
			if(count_bond != 0)
			{
			    vty_out(vty,"\n");	
			}
		}
	    else
		{
            /* do nothing */
    		vty_out(vty,"Vlan state is not right ! \n");
		}
   		vty_out(vty,"========================================================================\n");			

		/* munmap and close fd */
        ret = munmap(mem_vlan_list,sb.st_size);
        if( ret != 0 )
        {
            vty_out(vty,"Failed to munmap for g_vlanlist[]! %s\n", strerror(errno));			
        }	
		ret = close(fd);
		if( ret != 0 )
        {
            vty_out(vty,"close shm_vlan failed \n" );   
        }
        /*free(vlanName);*/
        return CMD_SUCCESS;				
	}	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,\
										NPD_DBUS_VLAN_METHOD_SHOW_VLAN_PORT_MEMBERS_V1 );
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vlanId,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &ret,
					DBUS_TYPE_UINT32, &product_id,
					DBUS_TYPE_UINT32, &promisPortBmp[0],
					DBUS_TYPE_UINT32, &promisPortBmp[1],
					DBUS_TYPE_STRING, &vlanName,
					DBUS_TYPE_UINT32, &untagPortBmp.portMbr[0],
					DBUS_TYPE_UINT32, &untagPortBmp.portMbr[1],
					DBUS_TYPE_UINT32, &tagPortBmp.portMbr[0],		
					DBUS_TYPE_UINT32, &tagPortBmp.portMbr[1],
					DBUS_TYPE_UINT32, &vlanStat,
					DBUS_TYPE_INVALID)) {
		if(VLAN_RETURN_CODE_ERR_NONE == ret) {
			vty_out(vty,"Codes:  U - vlan link status is up,   D - vlan link status is down, \n");
			vty_out(vty,"        u - untagged port member,      t - tagged port member, \n");
			vty_out(vty,"        * - promiscuous mode port member\n");
			vty_out(vty,"========================================================================\n");
			vty_out(vty,"%-7s  %-20s  %-40s\n","VLAN ID","VLAN NAME","PORT MEMBER LIST");
			vty_out(vty,"=======  ====================  =========================================\n");

			
			vty_out(vty,"%-4d(%s)  ",vlanId,vlanStat ? "U":"D");
			vty_out(vty,"%-20s	",vlanName);
			
			if(0 != untagPortBmp.portMbr[0] || 0 != tagPortBmp.portMbr[0] || 0 != untagPortBmp.portMbr[1] || 0 != tagPortBmp.portMbr[1]){
				show_vlan_member_slot_port(vty,product_id,untagPortBmp,tagPortBmp,promisPortBmp);
			}
			else 
				vty_out(vty,"  %-40s","No Port member.");
			vty_out(vty,"\n");
			vty_out(vty,"========================================================================\n");
		}
		else if(ETHPORT_RETURN_CODE_NO_SUCH_VLAN == ret) {
			vty_out(vty,"%% Bad parameter: vlan id illegal.\n");
		}
		else if(VLAN_RETURN_CODE_VLAN_NOT_EXISTS == ret) {
			vty_out(vty,"%% Bad parameter: vlan %d NOT Exists.\n",vlanId);
		}
		else if(VLAN_RETURN_CODE_ERR_GENERAL == ret) {
			vty_out(vty,"%% Error: operation on software fail.\n");
		}
	}
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	
	dbus_message_unref(reply);
	/*free(vlanName);*/
	return CMD_SUCCESS;
}

/***********************************
*show corrent vlan.
*Execute at Vlan-config Node
*CMD : show vlan trunk
*show trunk members of corrent vlan 
***********************************/
DEFUN(show_corrent_vlan_trunk_member_cmd_fun,
	show_corrent_vlan_trunk_member_cmd,
	"show vlan trunk-member",
	"Show system information\n"
	"Vlan entity\n"
	"Trunk members in this vlan\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	
	DBusMessageIter	 iter_array;
	unsigned short	vlanId = 0;
	char*			vlanName = NULL;
	unsigned int vlan_Cont = 0;
	unsigned int count = 0, ret = 0;
	unsigned int i = 0, j = 0, nodesave = 0;
	unsigned int slot = 0, port = 0, tmpVal = 0;
	unsigned int product_id = 0;
	unsigned int bitOffset = 0, mbrCount = 0;
	unsigned int untagTrkBmp[4] = {0}, tagTrkBmp[4] = {0};
	unsigned int vlanStat = 0;
	
	nodesave = (unsigned int)(vty->index);
	vlanId = (unsigned short)nodesave;

	if(is_distributed == DISTRIBUTED_SYSTEM)	
	{
        /* read vlan info from file: shm_vlan ,zhangdi@autelan.com */		
		int	count_tag =0;
		int	count_untag =0;		
		int count_trunk =0;
		
		int fd = -1, k=0;
		struct stat sb;

		vlan_list_t * mem_vlan_list =NULL;
		char* file_path = "/dbm/shm/vlan/shm_vlan";
		/* only read file */
	    fd = open(file_path, O_RDONLY);
    	if(fd < 0)
        {
            vty_out(vty,"Failed to open! %s\n", strerror(errno));
            return NULL;
        }
		fstat(fd,&sb);
		/* map not share */	
        mem_vlan_list = (vlan_list_t *)mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0 );
        if(MAP_FAILED == mem_vlan_list)
        {
            vty_out(vty,"Failed to mmap for g_vlanlist[]! %s\n", strerror(errno));
			close(fd);
            return NULL;
        }	

		vty_out(vty,"========================================================================\n");
		vty_out(vty,"%-7s  %-20s  %-40s\n","VLAN ID","VLAN NAME","TRUNK ID LIST");
		vty_out(vty,"=======  ====================  =========================================\n");

        if(mem_vlan_list[vlanId-1].vlanStat == 1)
		{
			vty_out(vty,"%-4d(%s)  ",vlanId,mem_vlan_list[vlanId-1].updown ? "U":"D");			
			vty_out(vty,"%-20s	",mem_vlan_list[vlanId-1].vlanName);

			count_trunk = 0;
            for(i=0; i<127; i++)
            {
                if(1 == mem_vlan_list[vlanId-1].untagTrkBmp[i])
                {
             		if((count_trunk!=0) && (0 == count_trunk % 4)) 
					{
						vty_out(vty,"\n%-32s"," ");
					}
					vty_out(vty,"%strunk%d(u)",((count_trunk)%4)?",":"",(i+1));
					
					count_trunk ++;
					continue;
					
                }
				else if(1==mem_vlan_list[vlanId-1].tagTrkBmp[i])
				{
             		if((count_trunk!=0) && (0 == count_trunk % 4)) 
					{
						vty_out(vty,"\n%-32s"," ");
					}
					vty_out(vty,"%strunk%d(t)",((count_trunk)%4)?",":"",(i+1));
					count_trunk ++;
					continue;					
				}
            }
			
			if(0 == count_trunk){ 
				vty_out(vty,"  %-40s","No Trunk member.");
			}	
			vty_out(vty,"\n");	
			vty_out(vty,"========================================================================\n");
		}
	    else
		{
            /* do nothing */
    		vty_out(vty,"Vlan state is not right ! \n");
		}

		/* munmap and close fd */
        ret = munmap(mem_vlan_list,sb.st_size);
        if( ret != 0 )
        {
            vty_out(vty,"Failed to munmap for g_vlanlist[]! %s\n", strerror(errno));			
        }	
		ret = close(fd);
		if( ret != 0 )
        {
            vty_out(vty,"close shm_vlan failed \n" );   
        }
        /*free(vlanName);*/
        return CMD_SUCCESS;				
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,\
										NPD_DBUS_VLAN_METHOD_SHOW_VLAN_TRUNK_MEMBERS );
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vlanId,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &ret,
					DBUS_TYPE_UINT32, &product_id,
					DBUS_TYPE_STRING, &vlanName,
					DBUS_TYPE_UINT32, &untagTrkBmp[0],
					DBUS_TYPE_UINT32, &untagTrkBmp[1],
					DBUS_TYPE_UINT32, &untagTrkBmp[2],
					DBUS_TYPE_UINT32, &untagTrkBmp[3],
					DBUS_TYPE_UINT32, &tagTrkBmp[0],
					DBUS_TYPE_UINT32, &tagTrkBmp[1],
					DBUS_TYPE_UINT32, &tagTrkBmp[2],
					DBUS_TYPE_UINT32, &tagTrkBmp[3],					
					DBUS_TYPE_UINT32, &vlanStat,
					DBUS_TYPE_INVALID)){	

		if(VLAN_RETURN_CODE_ERR_NONE == ret){
			vty_out(vty,"========================================================================\n");
			vty_out(vty,"%-7s  %-20s  %-40s\n","VLAN ID","VLAN NAME","TRUNK MEMBER LIST");
			vty_out(vty,"=======  ====================  =========================================\n");	
			
			vty_out(vty,"%-4d(%s)  ",vlanId,vlanStat ? "U" : "D");
			vty_out(vty,"%-20s	",vlanName);
			for(i =0; i<4; i++) {
				if(0 != untagTrkBmp[i] || 0 != tagTrkBmp[i]) {
					for(bitOffset = 0;bitOffset < 32;bitOffset++) {
						if(untagTrkBmp[i] & (1<<bitOffset)) {
							if(0 == mbrCount % 4 && 0 != mbrCount){
								vty_out(vty,"\n%-32s"," ");
							}	
							mbrCount++;
							vty_out(vty,"%s%s%d(u)",((mbrCount-1)%4)?",":"","trunk",(i*32 + bitOffset+1));
						}
						else 
							continue;
					}
					for(bitOffset = 0;bitOffset < 32;bitOffset++) {
						if(tagTrkBmp[i] & (1<<bitOffset)) {
							if(0 == mbrCount % 4 && 0 != mbrCount){
								vty_out(vty,"\n%-32s"," ");
							}	
							mbrCount++;
							vty_out(vty,"%s%s%d(t)",((mbrCount-1)%4)?",":"","trunk",(i*32 + bitOffset+1));
						}
						else
							continue;
					}
				}
				else 
					continue;
			}
			if(0 == mbrCount){ 
				vty_out(vty,"  %-40s","No Trunk member.");
			}	
			vty_out(vty,"\n");	
			vty_out(vty,"========================================================================\n");
		}
		else if(ETHPORT_RETURN_CODE_NO_SUCH_VLAN == ret) {
			vty_out(vty,"% Bad parameter: vlan id illegal.\n");
		}		
		else if(VLAN_RETURN_CODE_VLAN_NOT_EXISTS == ret) {
			vty_out(vty,"% Bad parameter,vlan %d not exists.\n",vlanId);
		}
	}	
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	
	dbus_message_unref(reply);
	/*free(vlanName);*/
	return CMD_SUCCESS;
}



DEFUN(show_vlan_list_port_member_cmd_fun,
	show_vlan_list_port_member_cmd,
	"show vlan port-member list",
	"Show system information\n"
	"Vlan entity\n"
	"Port members in vlans\n"
	"Currnet all active vlans\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int vlan_Cont = 0;
	unsigned short  vlanId = 0;
	char*			vlanName = NULL;	
	unsigned int	slot =0, port =0, tmpVal =0;
	unsigned int 	product_id = PRODUCT_ID_NONE, count = 0, i = 0, j = 0,m = 0, n=0, ret = 0;
	unsigned int    promisPortBmp[2] = {0};
	unsigned int vlanStat = 0;
	PORT_MEMBER_BMP untagPortBmp, tagPortBmp;
	memset(&untagPortBmp,0,sizeof(PORT_MEMBER_BMP));
	memset(&tagPortBmp,0,sizeof(PORT_MEMBER_BMP));
	vlanName = (char*)malloc(ALIAS_NAME_SIZE);
	memset(vlanName,0,ALIAS_NAME_SIZE);
	
	if(is_distributed == DISTRIBUTED_SYSTEM)	
    {
	    #if 1 /* read vlan info from file: shm_vlan ,zhangdi@autelan.com */		
		int	count_tag =0;
		int	count_untag =0;		
		int count_bond =0;
        int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);
		
		int fd = -1;
		struct stat sb;
		vlan_list_t * mem_vlan_list =NULL;
		char* file_path = "/dbm/shm/vlan/shm_vlan";
		
    	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
        int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
        if((local_slot_id<0)||(slotNum<0))
        {
        	vty_out(vty,"get_product_info() return -1,Please check dbm file !\n");
    		return CMD_WARNING;			
        }
		
		/* only read file */
	    fd = open(file_path, O_RDONLY);
    	if(fd < 0)
        {
            vty_out(vty,"Failed to open! %s\n", strerror(errno));
            return NULL;
        }
		fstat(fd,&sb);
		/* map not share */	
        mem_vlan_list = (vlan_list_t *)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0 );
        if(MAP_FAILED == mem_vlan_list)
        {
            vty_out(vty,"Failed to mmap for g_vlanlist[]! %s\n", strerror(errno));
			close(fd);
            return NULL;
        }	
    	vty_out(vty,"Codes:  U - vlan link status is up,     D - vlan link status is down, \n");
    	vty_out(vty,"        u - untagged port member,       t - tagged port member, \n");
       	vty_out(vty,"        f - first cpu on board,         s - second cpu on board, \n");		
    	vty_out(vty,"        * - promiscuous mode port member\n");
    	vty_out(vty,"========================================================================\n");
    	vty_out(vty,"%-7s  %-20s  %-40s\n","VLAN ID","VLAN NAME","PORT MEMBER LIST");
    	vty_out(vty,"=======  ====================  =========================================\n");				
		for (vlanId=1;vlanId<=4093;vlanId++)
        {
            if(mem_vlan_list[vlanId-1].vlanStat == 1)
    		{
    			vty_out(vty,"%-4d(%s)  ",vlanId,mem_vlan_list[vlanId-1].updown ? "U":"D");			
    			vty_out(vty,"%-20s	",mem_vlan_list[vlanId-1].vlanName);

				count_tag =0;
    			for(i = 0; i < slot_count; i++ )
    			{
        			for(j = 0; j < 64; j++ )
        			{
						if(j<32)
						{
                            if((mem_vlan_list[vlanId-1].untagPortBmp[i].low_bmp)&(1<<j))
                        	{
        						slot = i+1;
        						port = j+1;
                    			if((count_tag!=0) && (0 == count_tag % 4)) 
                    			{
                    				vty_out(vty,"\n%-32s"," ");
                    			}
                    			vty_out(vty,"%s%d/%d(u)",(count_tag%4) ? ",":"",slot,port);
                    			count_tag++;						
                        	}
        					else
        					{
        						continue;
        					}
						}
						else
						{
                            if((mem_vlan_list[vlanId-1].untagPortBmp[i].high_bmp)&(1<<(j-32)))
                        	{
        						slot = i+1;
        						port = j+1;
                    			if((count_tag!=0) && (0 == count_tag % 4)) 
                    			{
                    				vty_out(vty,"\n%-32s"," ");
                    			}
                    			vty_out(vty,"%s%d/%d(u)",(count_tag%4) ? ",":"",slot,port);
                    			count_tag++;						
                        	}
        					else
        					{
        						continue;
        					}								
						}
        			}				
    			}

				count_untag = 0;
    			for(i = 0; i < slot_count; i++ )
    			{
        			for(j = 0; j < 64; j++ )
        			{
						if(j<32)
						{
                            if((mem_vlan_list[vlanId-1].tagPortBmp[i].low_bmp)&(1<<j))
                        	{
        						slot = i+1;
        						port = j+1;
                                /* add '\n' between tag and untag list  */
                				if(count_tag != 0)
                				{
                					vty_out(vty,"\n%-32s"," ");
                					count_tag = 0;     /* change count_temp back to 0 for next use */
                				}								
                    			if((count_untag != 0)&&(0 == (count_untag % 4)))    /* tag port start at new line */
                    			{
                    				vty_out(vty,"\n%-32s"," ");
                    			}
                    			vty_out(vty,"%s%d/%d(t)",(count_untag%4) ? ",":"",slot,port);
                    			count_untag++;													
                        	}
        					else
        					{
        						continue;
        					}													
						}
						else
						{
                            if((mem_vlan_list[vlanId-1].tagPortBmp[i].high_bmp)&(1<<(j-32)))
                        	{
        						slot = i+1;
        						port = j+1;
                                /* add '\n' between tag and untag list  */
                				if(count_tag != 0)
                				{
                					vty_out(vty,"\n%-32s"," ");
                					count_tag = 0;     /* change count_temp back to 0 */
                				}								
                    			if((count_untag != 0)&&(0 == (count_untag % 4)))    /* tag port start at new line */
                    			{
                    				vty_out(vty,"\n%-32s"," ");
                    			}
                    			vty_out(vty,"%s%d/%d(t)",(count_untag%4) ? ",":"",slot,port);
                    			count_untag++;													
                        	}
        					else
        					{
        						continue;
        					}									
						}
        			}				
    			}
				vty_out(vty,"\n");
                /* print slot this vlan bonded to */
				count_bond = 0;
    			for(m = 0; m < slot_count; m++ )
    			{   
    				if(mem_vlan_list[vlanId-1].bond_slot[m] != 0)
    				{
    					if(count_bond == 0)   /* print one times */
    					{
            				vty_out(vty,"Bonded to slot : ");	
    					}
    					count_bond++;
    					/* first cpu 8 ports + second cpu 8 ports */
    					for(n=0;n<16;n++)
    					{
    						if((mem_vlan_list[vlanId-1].bond_slot[m])&(0x1<<n))   /* if !=0, the bit is 1 */
    						{	
                        		vty_out(vty,"%d(%c%d) ",m+1,(n<8)?'f':'s',(n%8)+1);
    						}   						
    					}
    				}
    			}			
				if(count_bond != 0)
				{
				    vty_out(vty,"\n");	
				}
				
				vty_out(vty,"\n");				
				/* continue for next vlan list */
    		}
    	    else
    		{
                /* do nothing */
                continue;
    		}
 
    	}
		vty_out(vty,"========================================================================\n");
        /*
		vty_out(vty,"sb.st_size: %d\n",sb.st_size );   
		vty_out(vty,"mem_vlan_list: 0x%x\n",mem_vlan_list);   
		vty_out(vty,"mem_vlan_list[0]: 0x%x\n",&mem_vlan_list[0]);   
		vty_out(vty,"mem_vlan_list[1]: 0x%x\n",&mem_vlan_list[1]);   
        */
		/* munmap and close fd */
        ret = munmap(mem_vlan_list,sb.st_size);
        if( ret != 0 )
        {
            vty_out(vty,"Failed to munmap for g_vlanlist[]! %s\n", strerror(errno));			
        }	
		ret = close(fd);
		if( ret != 0 )
        {
            vty_out(vty,"close shm_vlan failed \n" );   
        }
        /*free(vlanName);*/
        return CMD_SUCCESS;
			
        #else
		int count =0;
		for(i=1; i <= slotNum; i++)
		{
            query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
            									NPD_DBUS_VLAN_OBJPATH ,	\
            									NPD_DBUS_VLAN_INTERFACE ,	\
            									NPD_DBUS_VLAN_METHOD_SHOW_VLANLIST_PORT_MEMBERS_V1 );

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
            		printf("%s raised: %s",err.name,err.message);
            		dbus_error_free_for_dcli(&err);
            	}
            	return CMD_SUCCESS;
            }

            dbus_message_iter_init(reply,&iter);

            dbus_message_iter_get_basic(&iter,&ret);
            if(VLAN_RETURN_CODE_ERR_NONE == ret){
            	dbus_message_iter_next(&iter);	
            	dbus_message_iter_get_basic(&iter,&vlan_Cont);

            	dbus_message_iter_next(&iter);	
            	dbus_message_iter_get_basic(&iter,&product_id);
            	
            	dbus_message_iter_next(&iter);	
            	dbus_message_iter_get_basic(&iter,&promisPortBmp[0]);
            	dbus_message_iter_next(&iter);	
            	dbus_message_iter_get_basic(&iter,&promisPortBmp[1]);

            	dbus_message_iter_next(&iter);	
            	dbus_message_iter_recurse(&iter,&iter_array);
				if(0 == count)      /* make the next just show one time */
				{
                	vty_out(vty,"Codes:  U - vlan link status is up,    D - vlan link status is down, \n");
                	vty_out(vty,"        u - untagged port member,       t - tagged port member, \n");
                	vty_out(vty,"        * - promiscuous mode port member\n");
                	vty_out(vty,"========================================================================\n");
                	vty_out(vty,"%-7s  %-20s  %-40s\n","VLAN ID","VLAN NAME","PORT MEMBER LIST");
                	vty_out(vty,"=======  ====================  =========================================\n");
                    count++;
				}
            	for (j = 0; j < vlan_Cont; j++) {
            		vlanStat = 0;
            		memset(&untagPortBmp,0,sizeof(untagPortBmp));
            		memset(&tagPortBmp,0,sizeof(tagPortBmp));
            		
            		DBusMessageIter iter_struct;
            		dbus_message_iter_recurse(&iter_array,&iter_struct);
            		dbus_message_iter_get_basic(&iter_struct,&vlanId);

            		dbus_message_iter_next(&iter_struct);
            		dbus_message_iter_get_basic(&iter_struct,&vlanName);

            		dbus_message_iter_next(&iter_struct);
            		dbus_message_iter_get_basic(&iter_struct,&untagPortBmp.portMbr[0]);

            		dbus_message_iter_next(&iter_struct);
            		dbus_message_iter_get_basic(&iter_struct,&untagPortBmp.portMbr[1]);

            		dbus_message_iter_next(&iter_struct);
            		dbus_message_iter_get_basic(&iter_struct,&tagPortBmp.portMbr[0]);
            		
            		dbus_message_iter_next(&iter_struct);
            		dbus_message_iter_get_basic(&iter_struct,&tagPortBmp.portMbr[1]);

            		dbus_message_iter_next(&iter_struct);
            		dbus_message_iter_get_basic(&iter_struct,&vlanStat);

            		dbus_message_iter_next(&iter_array); 
            		if(NPD_PORT_L3INTF_VLAN_ID != vlanId){
            			vty_out(vty,"%-4d(%s)  ",vlanId,vlanStat ? "U" : "D");
            			vty_out(vty,"%-20s	",vlanName);

                        #if 0
            			if(0 != untagPortBmp.portMbr[0] || 0 != tagPortBmp.portMbr[0] || 0 != untagPortBmp.portMbr[1] || 0 != tagPortBmp.portMbr[1]){
            				show_vlan_member_slot_port(vty,product_id,untagPortBmp,tagPortBmp,promisPortBmp);
            			}
						#endif
            			if(0 != untagPortBmp.portMbr[0] || 0 != tagPortBmp.portMbr[0] || 0 != untagPortBmp.portMbr[1] || 0 != tagPortBmp.portMbr[1])
                        {
                            /* add for show vlan on AX71-2X12G12S,zhangdk 2011-05-30 */
                        	unsigned int k,count_temp = 0;
                        	unsigned int port_temp = 0;
                            unsigned int value_tmep = 0;

                        	for (k=0;k<64;k++)
                        	{
                        		port_temp = k;
                        		value_tmep = (1<<((k>31) ? (k-32) : k));
                        		if(((k>31) ? untagPortBmp.portMbr[1] : untagPortBmp.portMbr[0]) & value_tmep)
                        		{	
                        			if(count_temp && (0 == count_temp % 4)) 
                        			{
                        				vty_out(vty,"\n%-32s"," ");
                        			}
                        			vty_out(vty,"%s%d/%d(u%s)",(count_temp%4) ? ",":"",i,port_temp,((promisPortBmp[k/32] & value_tmep)?"*":""));
                        			count_temp++;
                        		}
                        	}
                        	port_temp = 0;
                        	value_tmep = 0;
                        	for (k=0;k<64;k++)
                        	{
                        		port_temp = k;
                        		value_tmep = (1<<((k>31) ? (k-32) : k));
                        		if(((k>31) ? tagPortBmp.portMbr[1] : tagPortBmp.portMbr[0]) & value_tmep) \
                        		{				
                        			if(count_temp && (0 == count_temp % 4))
                        			{
                        				vty_out(vty,"\n%-32s"," ");
                        			}
                        			vty_out(vty,"%s%d/%d(t%s)",(count_temp%4) ? ",":"",i,port_temp,((promisPortBmp[k/32] & value_tmep)?"*":""));
                        			count_temp++;
                        		}
                        	}	
                        }                                                							
            			else 
            				vty_out(vty,"  %-40s","No Port member.");
            			vty_out(vty,"\n");
            		}
            	}
            	vty_out(vty,"========================================================================\n");
            }
            else if(VLAN_RETURN_CODE_VLAN_NOT_EXISTS == ret) {
            	vty_out(vty,"%% Error,no vaild vlan exist.\n");
            }
            else if (VLAN_RETURN_CODE_ERR_GENERAL == ret) {
            	vty_out(vty,"%% Error: operation on software fail.\n");				
            }
            else if(COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
            	vty_out(vty,"%% Product not support this function!\n");
            }
            else {
            	vty_out(vty,"%% Unknown error ret %#x\n",ret);
            }
            dbus_message_unref(reply);
		}
        /*free(vlanName);*/
        return CMD_SUCCESS;		
		#endif
    }
    else
    {
        query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
        									NPD_DBUS_VLAN_OBJPATH ,	\
        									NPD_DBUS_VLAN_INTERFACE ,	\
        									NPD_DBUS_VLAN_METHOD_SHOW_VLANLIST_PORT_MEMBERS_V1 );


        dbus_error_init(&err);
        reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

        dbus_message_unref(query);
        if (NULL == reply) {
        	vty_out(vty,"failed get reply.\n");
        	if (dbus_error_is_set(&err)) {
        		printf("%s raised: %s",err.name,err.message);
        		dbus_error_free_for_dcli(&err);
        	}
        	return CMD_SUCCESS;
        }

        dbus_message_iter_init(reply,&iter);

        dbus_message_iter_get_basic(&iter,&ret);
        if(VLAN_RETURN_CODE_ERR_NONE == ret){
        	dbus_message_iter_next(&iter);	
        	dbus_message_iter_get_basic(&iter,&vlan_Cont);

        	dbus_message_iter_next(&iter);	
        	dbus_message_iter_get_basic(&iter,&product_id);
        	
        	dbus_message_iter_next(&iter);	
        	dbus_message_iter_get_basic(&iter,&promisPortBmp[0]);
        	dbus_message_iter_next(&iter);	
        	dbus_message_iter_get_basic(&iter,&promisPortBmp[1]);

        	dbus_message_iter_next(&iter);	
        	dbus_message_iter_recurse(&iter,&iter_array);
        	vty_out(vty,"Codes:  U - vlan link status is up,    D - vlan link status is down, \n");
        	vty_out(vty,"        u - untagged port member,       t - tagged port member, \n");
        	vty_out(vty,"        * - promiscuous mode port member\n");
        	vty_out(vty,"========================================================================\n");
        	vty_out(vty,"%-7s  %-20s  %-40s\n","VLAN ID","VLAN NAME","PORT MEMBER LIST");
        	vty_out(vty,"=======  ====================  =========================================\n");

        	for (j = 0; j < vlan_Cont; j++) {
        		vlanStat = 0;
        		memset(&untagPortBmp,0,sizeof(untagPortBmp));
        		memset(&tagPortBmp,0,sizeof(tagPortBmp));
        		
        		DBusMessageIter iter_struct;
        		dbus_message_iter_recurse(&iter_array,&iter_struct);
        		dbus_message_iter_get_basic(&iter_struct,&vlanId);

        		dbus_message_iter_next(&iter_struct);
        		dbus_message_iter_get_basic(&iter_struct,&vlanName);

        		dbus_message_iter_next(&iter_struct);
        		dbus_message_iter_get_basic(&iter_struct,&untagPortBmp.portMbr[0]);

        		dbus_message_iter_next(&iter_struct);
        		dbus_message_iter_get_basic(&iter_struct,&untagPortBmp.portMbr[1]);

        		dbus_message_iter_next(&iter_struct);
        		dbus_message_iter_get_basic(&iter_struct,&tagPortBmp.portMbr[0]);
        		
        		dbus_message_iter_next(&iter_struct);
        		dbus_message_iter_get_basic(&iter_struct,&tagPortBmp.portMbr[1]);

        		dbus_message_iter_next(&iter_struct);
        		dbus_message_iter_get_basic(&iter_struct,&vlanStat);

        		dbus_message_iter_next(&iter_array); 
        		if(NPD_PORT_L3INTF_VLAN_ID != vlanId){
        			vty_out(vty,"%-4d(%s)  ",vlanId,vlanStat ? "U" : "D");
        			vty_out(vty,"%-20s	",vlanName);

        			if(0 != untagPortBmp.portMbr[0] || 0 != tagPortBmp.portMbr[0] || 0 != untagPortBmp.portMbr[1] || 0 != tagPortBmp.portMbr[1]){
        				show_vlan_member_slot_port(vty,product_id,untagPortBmp,tagPortBmp,promisPortBmp);
        			}
        			else 
        				vty_out(vty,"  %-40s","No Port member.");
        			vty_out(vty,"\n");
        		}
        	}
        	vty_out(vty,"========================================================================\n");
        }
        else if(VLAN_RETURN_CODE_VLAN_NOT_EXISTS == ret) {
        	vty_out(vty,"%% Error,no vaild vlan exist.\n");
        }
        else if (VLAN_RETURN_CODE_ERR_GENERAL == ret) {
        	vty_out(vty,"%% Error: operation on software fail.\n");				
        }
        else if(COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
        	vty_out(vty,"%% Product not support this function!\n");
        }
        else {
        	vty_out(vty,"%% Unknown error ret %#x\n",ret);
        }
        dbus_message_unref(reply);
        /*free(vlanName);*/
        return CMD_SUCCESS;
    }
}

DEFUN(show_vlan_list_trunk_member_cmd_fun,
	show_vlan_list_trunk_member_cmd,
	"show vlan trunk-member list",
	"Show system information\n"
	"Vlan entity\n"
	"Trunk members in vlans\n"
	"Currnet all active vlans\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int vlan_Cont = 0;
	unsigned short  vlanId = 0;
	char*			vlanName = NULL;
	unsigned int 	product_id = PRODUCT_ID_NONE, i = 0, j = 0, ret = 0;
	unsigned int	bitOffset = 0, mbrCount = 0;
	unsigned int	untagTrkBmp[4] = {0}, tagTrkBmp[4] = {0};
	unsigned int 	vlanStat = 0;
	int k;
	int local_slot_id = 0;
   	int slotNum = 0;
	
	vlanName = (char*)malloc(ALIAS_NAME_SIZE);
	memset(vlanName,0,ALIAS_NAME_SIZE);

	if(is_distributed == DISTRIBUTED_SYSTEM)	
	{	
        /* read vlan info from file: shm_vlan ,zhangdi@autelan.com */		
		int	count_tag =0;
		int	count_untag =0;		
		int count_trunk =0;
		
		int fd = -1, k=0;
		struct stat sb;

		vlan_list_t * mem_vlan_list =NULL;
		char* file_path = "/dbm/shm/vlan/shm_vlan";
		/* only read file */
	    fd = open(file_path, O_RDONLY);
    	if(fd < 0)
        {
            vty_out(vty,"Failed to open! %s\n", strerror(errno));
            return NULL;
        }
		fstat(fd,&sb);
		/* map not share */	
        mem_vlan_list = (vlan_list_t *)mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0 );
        if(MAP_FAILED == mem_vlan_list)
        {
            vty_out(vty,"Failed to mmap for g_vlanlist[]! %s\n", strerror(errno));
			close(fd);
            return NULL;
        }
		
		vty_out(vty,"========================================================================\n");
		vty_out(vty,"%-7s  %-20s  %-40s\n","VLAN ID","VLAN NAME","TRUNK ID LIST");
		vty_out(vty,"=======  ====================  =========================================\n");

		for (vlanId=0;vlanId<=4093;vlanId++)
        {
            if(mem_vlan_list[vlanId-1].vlanStat == 1)
    		{

    			vty_out(vty,"%-4d(%s)  ",vlanId,mem_vlan_list[vlanId-1].updown ? "U":"D");			
    			vty_out(vty,"%-20s	",mem_vlan_list[vlanId-1].vlanName);

    			count_trunk = 0;
                for(i=0; i<127; i++)
                {
                    if(1 == mem_vlan_list[vlanId-1].untagTrkBmp[i])
                    {
                 		if((count_trunk!=0) && (0 == count_trunk % 4)) 
    					{
    						vty_out(vty,"\n%-32s"," ");
    					}
    					vty_out(vty,"%strunk%d(u)",((count_trunk)%4)?",":"",(i+1));  					
    					count_trunk ++;    					
                    }
					
    				if(1==mem_vlan_list[vlanId-1].tagTrkBmp[i])
    				{
                 		if((count_trunk!=0) && (0 == count_trunk % 4)) 
    					{
    						vty_out(vty,"\n%-32s"," ");
    					}
    					vty_out(vty,"%strunk%d(t)",((count_trunk)%4)?",":"",(i+1));
    					count_trunk ++;
    				}
                }
    			
    			if(0 == count_trunk){ 
    				vty_out(vty,"  %-40s","No Trunk member.");
    			}
				vty_out(vty,"\n");
            }
		}
   		vty_out(vty,"========================================================================\n");

		/* munmap and close fd */
        ret = munmap(mem_vlan_list,sb.st_size);
        if( ret != 0 )
        {
            vty_out(vty,"Failed to munmap for g_vlanlist[]! %s\n", strerror(errno));			
        }	
		ret = close(fd);
		if( ret != 0 )
        {
            vty_out(vty,"close shm_vlan failed \n" );   
        }
        /*free(vlanName);*/
        return CMD_SUCCESS;				
	}
	else
	{
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_VLAN_OBJPATH ,	\
											NPD_DBUS_VLAN_INTERFACE ,	\
											NPD_DBUS_VLAN_METHOD_SHOW_VLANLIST_TRUNK_MEMBERS );
	

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

		dbus_message_iter_init(reply,&iter);

		dbus_message_iter_get_basic(&iter,&ret);
		if(VLAN_RETURN_CODE_ERR_NONE == ret){
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&vlan_Cont);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&product_id);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);
			vty_out(vty,"========================================================================\n");
			vty_out(vty,"%-7s  %-20s  %-40s\n","VLAN ID","VLAN NAME","TRUNK MEMBER LIST");
			vty_out(vty,"=======  ====================  =========================================\n");

			for (j = 0; j < vlan_Cont; j++) {
				mbrCount = 0;
				vlanStat = 0;
				memset(untagTrkBmp,0,sizeof(untagTrkBmp));
				memset(tagTrkBmp,0,sizeof(tagTrkBmp));
				DBusMessageIter iter_struct;
				dbus_message_iter_recurse(&iter_array,&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&vlanId);

				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&vlanName);

				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&untagTrkBmp[0]);
			
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&untagTrkBmp[1]);
			
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&untagTrkBmp[2]);
			
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&untagTrkBmp[3]);

				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&tagTrkBmp[0]);
			
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&tagTrkBmp[1]);
			
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&tagTrkBmp[2]);
			
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&tagTrkBmp[3]);

				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&vlanStat);


				dbus_message_iter_next(&iter_array); 
				if(NPD_PORT_L3INTF_VLAN_ID != vlanId /*&& DEFAULT_VLAN_ID != vlanId*/){
					vty_out(vty,"%-4d(%s)  ",vlanId,vlanStat ? "U" : "D");
					vty_out(vty,"%-20s	",vlanName);
					for(i =0; i<4; i++) {
						if(0 != untagTrkBmp[i] || 0 != tagTrkBmp[i]) {
							for(bitOffset = 0;bitOffset < 32;bitOffset++) {
								if(untagTrkBmp[i] & (1<<bitOffset)) {
									if(0 == mbrCount % 4 && 0 != mbrCount){
										vty_out(vty,"\n%-32s"," ");
									}	
									mbrCount++;
									vty_out(vty,"%s%s%d(u)",((mbrCount-1)%4)?",":"","trunk",(i*32 + bitOffset+1));
								}
								else 
									continue;
							}
							for(bitOffset = 0;bitOffset < 32;bitOffset++) {
								if(tagTrkBmp[i] & (1<<bitOffset)) {
									if(0 == mbrCount % 4 && 0 != mbrCount){
										vty_out(vty,"\n%-32s"," ");
									}	
									mbrCount++;
									vty_out(vty,"%s%s%d(t)",((mbrCount-1)%4)?",":"","trunk",(i*32 + bitOffset+1));
								}
								else
									continue;
							}
						}
						else 
							continue;
					}
					if(0 == mbrCount) { 
						vty_out(vty,"  %-40s","No Trunk member.");
					}	
					vty_out(vty,"\n");	
				}
			}		
			vty_out(vty,"========================================================================\n");
		}	
		else if(VLAN_RETURN_CODE_VLAN_NOT_EXISTS == ret) {
			vty_out(vty,"%% Error,no vaild vLan exist.\n");
		}
		else if(COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
			vty_out(vty,"%% Product not support this function!\n");
		}
		dbus_message_unref(reply);
    	/*free(vlanName);*/
    	return CMD_SUCCESS;		
	}
}


DEFUN(set_port_vlan_ingress_filter_cmd_fun,
	set_port_vlan_ingress_filter_cmd,
	"set port ingressfilter SLOT_PORT (enable|disable)",
	"Set system information\n"
	"An enable port in system\n"
	"Vlan entity ingress Fliter\n"
	CONFIG_ETHPORT_STR
	"Enable vlan ingress Fliter\n"
	"Disable vlan ingress Filter\n")
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned char slot_no = 0, local_port_no = 0;
	boolean enDis = FALSE;
	unsigned int	ret = 0;

	ret = parse_slotport_no((char *)argv[0],&slot_no,&local_port_no);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"% Bad parameter,unknow portno format.\n");
		return CMD_WARNING;
	}
	if( 0 == slot_no) {
		vty_out(vty,"% Bad parameter,bad slot/port number.\n");
		return CMD_WARNING;
	}
	if(strcmp(argv[1],"enable")==0) {
		enDis = TRUE;
	}
	else if (strcmp(argv[1],"disable")==0) {
		enDis = FALSE;
	}
	else {
		vty_out(vty,"% Bad command parameter!\n");
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,	\
										NPD_DBUS_VLAN_METHOD_SET_PORT_VLAN_INGRES_FLITER );
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&slot_no,
							DBUS_TYPE_BYTE,&local_port_no,
							DBUS_TYPE_BYTE,&enDis,
							DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &ret,
					DBUS_TYPE_INVALID)) {
		if(VLAN_RETURN_CODE_ERR_NONE == ret) {
			vty_out(vty,"port %d/%d ingressFliter %s\n",slot_no,local_port_no,(enDis)?"enable":"Disable");
		}
		else if (VLAN_RETURN_CODE_ERR_HW == ret) {
			vty_out(vty,"% Error occurs in write Pvid Reg in hardware.\n");				
		}
		else if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == ret) {
        	vty_out(vty,"% Bad parameter,slot/port No illegal\n");
		}
		else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
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
	
DEFUN(set_port_pvid_cmd_fun,
	set_port_pvid_cmd, 
	"set port pvid SLOT_PORT <1-4095>",
	"Set system information\n"
	"An enable port in system\n" 
	"Assign(Match) to(with) packet vlanID field on port\n"
	CONFIG_ETHPORT_STR
	"Pvid value set to the port\n")
{
	DBusMessage *query, *reply;
	DBusError err;

	unsigned int	ret;
	unsigned char slot_no,local_port_no;
	unsigned short pvid= 1;
	/*
	  unsigned short vlanId;
	  vlanId = (unsigned short)(vty->index);
	*/ 
	/*set global,Not need to check port membership in any vlan.
	//when this Cmd execute at VLAN_CONFIG node, it does do it.*/
	
	ret = parse_slotport_no((char *)argv[0],&slot_no,&local_port_no);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"% Bad parameter,unknow portno format.\n");
		return CMD_WARNING;
	}
	if( 0 == slot_no) {
		vty_out(vty,"% Bad parameter,bad slot/port number.\n");
		return CMD_WARNING;
	}
	ret = parse_vlan_no((char*)argv[1],&pvid);
	
	if (NPD_FAIL == ret) {
    	vty_out(vty,"% Bad parameter,pvid illegal.\n");
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,	\
										NPD_DBUS_VLAN_METHOD_SET_ONE_PORT_PVID );
	
	dbus_error_init(&err);
	dbus_message_append_args(	query,
							 	DBUS_TYPE_BYTE,&slot_no,
							 	DBUS_TYPE_BYTE,&local_port_no,
							 	DBUS_TYPE_UINT16,&pvid,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &ret,
					DBUS_TYPE_INVALID)) {
		if(VLAN_RETURN_CODE_ERR_NONE == ret) {
			/*vty_out(vty,"port %d/%d pvid %d\n",slot_no,local_port_no,pvid);*/
		}
		else if (VLAN_RETURN_CODE_ERR_HW == ret) {
			vty_out(vty,"%% Error occurs in write Pvid Reg in hardware.\n");				
		}
		else if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == ret) {
        	vty_out(vty,"% Bad parameter,slot/port No illegal\n");
		}
		else if (VLAN_RETURN_CODE_BADPARAM == ret) {
			vty_out(vty,"%% Error occurs in Parse eth-port Index or devNO.& logicPortNo.\n");
		}
		else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
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
DEFUN(show_port_pvid_cmd_fun,
	show_port_pvid_cmd, 
	"show pvid port SLOT_PORT",
	"Show system information\n"
	"An enable port\n"
	"Port's Pvid\n"
	CONFIG_ETHPORT_STR)
{
	DBusMessage *query, *reply;
	DBusError err;

	unsigned int	ret = 0;
	unsigned char slot_no = 0, local_port_no = 0;
	unsigned short pvid = 1;
	ret = parse_slotport_no((char *)argv[0],&slot_no,&local_port_no);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"Unknow portno format.\n");
		return CMD_SUCCESS;
	}
	if( 0 == slot_no ) {
		vty_out(vty,"% Bad parameter,bad slot/port number.\n");
		return CMD_WARNING;
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,	\
										NPD_DBUS_VLAN_METHOD_SHOW_ONE_PORT_PVID );
	
	dbus_error_init(&err);
	dbus_message_append_args(	query,
							 	DBUS_TYPE_BYTE,&slot_no,
							 	DBUS_TYPE_BYTE,&local_port_no,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32, &ret,
					DBUS_TYPE_UINT16, &pvid,
					DBUS_TYPE_INVALID)) {
		if(NPD_DBUS_SUCCESS == ret) {
			vty_out(vty,"======== ========\n");
			vty_out(vty,"%-8s %-8s\n","PORT","PVID");
			vty_out(vty,"%d/%d      %-8d\n",slot_no,local_port_no,pvid);
			vty_out(vty,"======== ========\n");
		}
		else if (VLAN_RETURN_CODE_ERR_HW == ret) {
			vty_out(vty,"% Error occurs in Read Pvid Reg in HW.\n");				
		}
		else if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == ret) {
        	vty_out(vty,"% Bad parameter,slot/port No illegal\n");
			/*vty_out(vty,"Please make sure slotNo.in [1-4],portNo.in [1-6].\n");*/
		}
		else if (VLAN_RETURN_CODE_BADPARAM == ret) {
			vty_out(vty,"%% Error occurs in Parse eth-port Index or devNO.& logicPortNo.\n");
		}
		else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
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

/***********
*show ports-list pvid
*for 24 eth-ports 
*				 on 6GTX card 
************/
DEFUN(show_port_list_pvid_cmd_fun,
	show_port_list_pvid_cmd, 
	"show pvid ports-list",
	"Show system information\n"
	"Port's Pvid\n"	
	"All enable ports\n")
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;

	unsigned int	i = 0,count = 0,ret = 0;
	unsigned int 	slot_no = 0,local_port_no = 0;

	unsigned short pvid= 1;
	query = dbus_message_new_method_call(
						NPD_DBUS_BUSNAME,  \
						NPD_DBUS_VLAN_OBJPATH ,  \
						NPD_DBUS_VLAN_INTERFACE ,  \
						NPD_DBUS_VLAN_METHOD_SHOW_PORTS_LIST_PVID );
	

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);

	dbus_message_iter_get_basic(&iter,&ret);

	if(VLAN_RETURN_CODE_ERR_NONE == ret){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&count);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		vty_out(vty,"========== ===========\n");
		vty_out(vty,"%-10s %-30s  \n",	"SLOT_PORT","PVID");
		vty_out(vty,"========== =========\n");

		for(i = 0; i < count;i ++){
			DBusMessageIter iter_struct;

			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&slot_no);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&local_port_no);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&pvid);

			dbus_message_iter_next(&iter_array);
			vty_out(vty,"%d/%-2d%-6s",slot_no,local_port_no,"");
			vty_out(vty," %-30d\n",pvid);
		}
		vty_out(vty,"========== ===========\n");
	}
	else if (VLAN_RETURN_CODE_ERR_HW == ret) {
		vty_out(vty,"%% Error occurs in Read Pvid Reg in HW.\n");				
	}
	else if (VLAN_RETURN_CODE_BADPARAM == ret) {
		vty_out(vty,"%% Error occurs in Parse eth-port Index or devNO.& logicPortNo.\n");
	}
	else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
		vty_out(vty,"%% Product not support this function!\n");
	}
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;

}
/**************************************
* Delete vlan entity
* Params:
*		vlan ID <1-4095>
*		vlan name:
* Usage:delete vlan <1-4095>
**************************************/
DEFUN(config_vlan_delete_vlan_cmd_fun,
	config_vlan_delete_vlan_cmd,
	"delete vlan <2-4093>",		/*"delete vlan (<1-4095>|VLANNAME)",*/
	"Delete System Interface\n"
	"Vlan entity\n"
	"Specify vlan id for vlan entity,valid range <1-4093>\n"
)
{
	if(is_distributed == DISTRIBUTED_SYSTEM)	
	{
    	DBusMessage *query = NULL, *reply = NULL;
    	DBusError err;

    	unsigned short 	vlanId =0;
    	int ret = 0;
    	unsigned int op_ret = 0;

    	int slot_id =0;
    	int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
        int slot_count = get_product_info(SEM_SLOT_COUNT_PATH);
		
        if((local_slot_id<0)||(slot_count<0))
        {
        	vty_out(vty,"get_product_info() return -1,Please check dbm file !\n");
    		return CMD_WARNING;			
        }
		
    	ret = parse_vlan_no((char*)argv[0],&vlanId);
    	
    	if (NPD_FAIL == ret) {
        	vty_out(vty,"% Bad parameter,vlan id illegal!\n");
    		return CMD_WARNING;
    	}

	    for(slot_id=1;slot_id<=slot_count;slot_id++)
	    {
		
        	if (1 == vlanId){
        		vty_out(vty,"% Bad parameter,can NOT delete Default vlan!\n");
        		return CMD_WARNING;
        	}
        	if (4095 == vlanId){
        		vty_out(vty,"% Bad parameter,Reserved vlan for Layer3 interface of EthPort!\n");
        		return CMD_WARNING;
        	}
    		/* vlan 4094 do not support config */
        	if (4094 == vlanId){
        		vty_out(vty,"% Bad parameter,Reserved vlan for internal use!\n");
        		return CMD_WARNING;
        	}			
        	else {

        		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
        											NPD_DBUS_VLAN_OBJPATH ,	\
        											NPD_DBUS_VLAN_INTERFACE ,	\
        											NPD_DBUS_VLAN_METHOD_CONFIG_DELETE_VLAN_ENTRY );
        		
        		dbus_error_init(&err);

        		dbus_message_append_args(query,
        								DBUS_TYPE_UINT16,&vlanId,
        								DBUS_TYPE_INVALID);
				/* send the dbus msg to any other board */
                if(NULL == dbus_connection_dcli[slot_id]->dcli_dbus_connection) 				
    			{
        			if(slot_id == local_slot_id)
    				{
                        reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
    				}
    				else 
    				{	
    				   	vty_out(vty,"Can not connect to slot:%d \n",slot_id);	
    					continue;
    				}
                }
    			else
    			{
                    reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[slot_id]->dcli_dbus_connection,query,-1, &err);				
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

        	if (dbus_message_get_args ( reply, &err,
        					DBUS_TYPE_UINT32, &op_ret,
        					DBUS_TYPE_INVALID)) 
        	{
        		if (ETHPORT_RETURN_CODE_NO_SUCH_VLAN == op_ret) {
        			vty_out(vty,"%% Bad Parameter:vlan id illegal.\n");
        		}
        		else if (VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret) {
                	vty_out(vty,"%% Bad Parameter:vlan %d NOT exists on slot %d.\n",vlanId,slot_id);
        		}
        		else if (VLAN_RETURN_CODE_BADPARAM == op_ret) {
        			vty_out(vty,"%% Bad Parameter:Can NOT delete Default vlan.\n");
        		}
        		else if (VLAN_RETURN_CODE_ERR_HW == op_ret) {
        			vty_out(vty,"%% Error occurs in config on hardware.\n");
        		}
        		else if (VLAN_RETURN_CODE_L3_INTF == op_ret) {
        			vty_out(vty,"%% Bad Parameter:Can NOT delete Layer3 Interface vlan.\n");
        		}
        		else if (VLAN_RETURN_CODE_SUBINTF_EXISTS == op_ret){
					#if 0  /* Advanced-routing not use in distributed system */
                    vty_out(vty,"%% Bad Parameter:Can NOT delete Advanced-routing Interface vlan.\n");
					#endif
					/* vlan is bonded, can not delete */
                    vty_out(vty,"slot%d: vlan%d has been bonded, Please unbond it first !\n",slot_id,vlanId);				
        		}
        		else if (VLAN_RETURN_CODE_PORT_SUBINTF_EXISTS == op_ret){
                    vty_out(vty,"%% Can NOT delete vlan with port sub interface.\n");
        		}
                else if(VLAN_RETURN_CODE_HAVE_PROT_VLAN_CONFIG== op_ret){
                    vty_out(vty,"%% There is protocol-vlan config on this vlan,please delete it first!\n");
                }
        		else if(VLAN_RETURN_CODE_ERR_NONE == op_ret) {
        			/*vty_out(vty,"Delete vlan OK.\n");*/
        		}
        		else{
                    vty_out(vty,"%% Unknown error occured when delete vlan.\n");
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
	    }   	
    	return CMD_SUCCESS;	
	}
	else
	{
	
    	DBusMessage *query = NULL, *reply = NULL;
    	DBusError err;

    	unsigned short 	vlanId =0;
    	int ret = 0;
    	unsigned int op_ret = 0;
    	#if 0
    	vlanName = (char*)malloc(ALIAS_NAME_SIZE);
    	memset(vlanName,0,ALIAS_NAME_SIZE);

    	ret = vlan_name_legal_check((char*)argv[0],strlen(argv[0]));
    	if(ALIAS_NAME_LEN_ERROR == ret) {
    		vty_out(vty,"% Bad parameter,vlan name too long!\n");
    		return CMD_WARNING;
    	}
    	else if(ALIAS_NAME_HEAD_ERROR == ret) {
    		ret = parse_vlan_no((char*)argv[0], &vlanId);
    		if(NPD_FAIL == ret){
    			vty_out(vty,"% Bad parameter,vlan name begins with a illegal char!\n");
    			return CMD_WARNING;
    		}
    		else {
    			/*vty_out(vty,"Get vlan ID %d,ret %d.\n",vlanId,ret);*/
    		}
    	}
    	else if(ALIAS_NAME_BODY_ERROR == ret) {
    		vty_out(vty,"% Bad parameter,vlan name contains illegal char!\n");
    		return CMD_WARNING;
    	}
    	else {
    		nameSize = strlen(argv[0]);
    		memcpy(vlanName, argv[0], nameSize);
    	}
    	if (NPD_SUCCESS != ret) {
    		return CMD_WARNING;
    	}
    	#endif
    	ret = parse_vlan_no((char*)argv[0],&vlanId);
    	
    	if (NPD_FAIL == ret) {
        	vty_out(vty,"% Bad parameter,vlan id illegal!\n");
    		return CMD_WARNING;
    	}
    	if (1 == vlanId){
    		vty_out(vty,"% Bad parameter,can NOT delete Default vlan!\n");
    		return CMD_WARNING;
    	}
    	if (4095 == vlanId){
    		vty_out(vty,"% Bad parameter,Reserved vlan for Layer3 interface of EthPort!\n");
    		return CMD_WARNING;
    	}
    	else {

    		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
    											NPD_DBUS_VLAN_OBJPATH ,	\
    											NPD_DBUS_VLAN_INTERFACE ,	\
    											NPD_DBUS_VLAN_METHOD_CONFIG_DELETE_VLAN_ENTRY );
    		
    		dbus_error_init(&err);

    		dbus_message_append_args(query,
    								DBUS_TYPE_UINT16,&vlanId,
    								DBUS_TYPE_INVALID);
    		/*printf("build query for vlanId %d.\n",vlanId);*/
    		
    		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
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
    					DBUS_TYPE_UINT32, &op_ret,
    					DBUS_TYPE_INVALID)) 
    	{
    		if (ETHPORT_RETURN_CODE_NO_SUCH_VLAN == op_ret) {
    			vty_out(vty,"%% Bad Parameter:vlan id illegal.\n");
    		}
    		else if (VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret) {
            	vty_out(vty,"%% Bad Parameter:vlan %d NOT exists.\n",vlanId);
    		}
    		else if (VLAN_RETURN_CODE_BADPARAM == op_ret) {
    			vty_out(vty,"%% Bad Parameter:Can NOT delete Default vlan.\n");
    		}
    		else if (VLAN_RETURN_CODE_ERR_HW == op_ret) {
    			vty_out(vty,"%% Error occurs in config on hardware.\n");
    		}
    		else if (VLAN_RETURN_CODE_L3_INTF == op_ret) {
    			vty_out(vty,"%% Bad Parameter:Can NOT delete Layer3 Interface vlan.\n");
    		}
    		else if (VLAN_RETURN_CODE_SUBINTF_EXISTS == op_ret){
                vty_out(vty,"%% Bad Parameter:Can NOT delete Advanced-routing Interface vlan.\n");
    		}
    		else if (VLAN_RETURN_CODE_PORT_SUBINTF_EXISTS == op_ret){
                vty_out(vty,"%% Can NOT delete vlan with port sub interface.\n");
    		}
            else if(VLAN_RETURN_CODE_HAVE_PROT_VLAN_CONFIG== op_ret){
                vty_out(vty,"%% There is protocol-vlan config on this vlan,please delete it first!\n");
            }
    		else if(VLAN_RETURN_CODE_ERR_NONE == op_ret) {
    			/*vty_out(vty,"Delete vlan OK.\n");*/
    		}
    		else{
                vty_out(vty,"%% Unknown error occured when delete vlan.\n");
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
}
/**************************************
* Delete vlan entity
* Params:
*		vlan name:
* Usage:delete vlan <2-4095>
**************************************/
DEFUN(config_vlan_delete_vlan_cmd_vname_fun,
	config_vlan_delete_vlan_vname_cmd,
	"delete vlan VLANNAME",
	"Delete System Interface\n"
	"Actives vlan via Specify vlanId\n"
	"Specify vlan name,begins with char or'_', and no more than 20 characters\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	/*for bug: AXSSZFI-508 */
	vty_out(vty,"Do not support CMD on distributed system. \n");
	return CMD_WARNING;	
	
	unsigned short 	vlanId =0;
	char*			vlanName = NULL;
	unsigned int	nameSize = 0;
	int ret = 0;
	unsigned int op_ret = 0;
	/*get vlan name*/
	/*printf("before parse_vlan_name %s length %d\n",argv[0],strlen(argv[0]));*/
	vlanName = (char*)malloc(ALIAS_NAME_SIZE);
	memset(vlanName,0,ALIAS_NAME_SIZE);

	ret = vlan_name_legal_check((char*)argv[0],strlen(argv[0]));
	if(ALIAS_NAME_LEN_ERROR == ret) {
		vty_out(vty,"% Bad parameter,vlan name too long!\n");
		return CMD_WARNING;
	}
	else if(ALIAS_NAME_HEAD_ERROR == ret) {
		vty_out(vty,"% Bad parameter,vlan name begins with an illegal char!\n");
		return CMD_WARNING;
	}
	else if(ALIAS_NAME_BODY_ERROR == ret) {
		vty_out(vty,"% Bad parameter,vlan name contains illegal char!\n");
		return CMD_WARNING;
	}
	else {
		nameSize = strlen(argv[0]);
		memcpy(vlanName, argv[0], nameSize);
	}
	if (NPD_SUCCESS != ret) {
		return CMD_WARNING;
	}
	else {

		query = dbus_message_new_method_call(
											NPD_DBUS_BUSNAME,	\
											NPD_DBUS_VLAN_OBJPATH ,	\
											NPD_DBUS_VLAN_INTERFACE ,	\
											NPD_DBUS_VLAN_METHOD_DELETE_VLAN_ENTRY_VIA_NAME );
		
		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_STRING,&vlanName,
								DBUS_TYPE_INVALID);
		
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
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
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) 
	{
		if (VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret) {
        	vty_out(vty,"% Bad Parameter:vlan %s NOT exists.\n",vlanName);
		}
		if (ETHPORT_RETURN_CODE_NO_SUCH_VLAN == op_ret) {
			vty_out(vty,"%% Bad Parameter:vlan id illegal.\n");
		}		
		else if (VLAN_RETURN_CODE_BADPARAM == op_ret) {
			vty_out(vty,"% Bad Parameter:Can NOT delete Default vlan.\n");
		}
		else if (VLAN_RETURN_CODE_ERR_HW == op_ret) {
			vty_out(vty,"%% Error occurs in config on hardware.\n");
		}
		else if (VLAN_RETURN_CODE_PORT_L3_INTF == op_ret) {
			vty_out(vty,"% Bad parameter,Reserved vlan for Layer3 interface of EthPort!\n");
		}
		else if (VLAN_RETURN_CODE_L3_INTF == op_ret) {
			vty_out(vty,"% Bad Parameter:Can NOT delete Layer3 Interface vlan.\n");
		}
		else if (VLAN_RETURN_CODE_SUBINTF_EXISTS == op_ret){
            vty_out(vty,"%% Can NOT delete Advanced-routing interface vlan.\n");
		}
		else if (VLAN_RETURN_CODE_PORT_SUBINTF_EXISTS == op_ret){
            vty_out(vty,"%% Can NOT delete vlan with port sub interface.\n");
		}
		else if(COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
			vty_out(vty,"%% Product not support this function!\n");
		}
        else if(VLAN_RETURN_CODE_HAVE_PROT_VLAN_CONFIG== op_ret){
            vty_out(vty,"%% There is protocol-vlan config on this vlan,please delete it first!\n");
        }
		else if(VLAN_RETURN_CODE_ERR_NONE != op_ret) {
			vty_out(vty,"%%Delete vlan FAILED.\n");
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
	free(vlanName);
	return CMD_SUCCESS;
}

/**************************************
* config vlan mtu
* Params:
*		mtu value:
* Usage:delete vlan <64-8192>
**************************************/
DEFUN(config_vlan_mtu_cmd_fun,
	config_vlan_mtu_cmd,
	"config vlan mtu <64-8192>",
	CONFIG_STR
	"Config layer 2 vlan entity\n"
	"Config vlan mtu\n"
	"Specify eth-port mtu value \n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned int 	mtu =1522;
	unsigned short value = 0;
	int ret = 0;
	unsigned int op_ret = 0;
	
	ret = parse_single_param_no((char*)argv[0],&value);
	if(NPD_SUCCESS != ret) {
		vty_out(vty,"%% parse param failed!\n");
		return CMD_WARNING;
	}

	mtu = value;
	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_MTU );
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&mtu,
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
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) 
	{
		if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
			vty_out(vty,"%% Product not support this function!\n");
		}
		else if (VLAN_RETURN_CODE_ERR_NONE != op_ret) {
        	vty_out(vty,"% Config vlan mtu failed.\n");
		}
		else if(VLAN_RETURN_CODE_ERR_NONE == op_ret) {
			/*vty_out(vty,"Delete vlan OK.\n");*/
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

/**************************************
* config vlan egress filter
* Params:
*		filter value:
* Usage:config vlan egress-filter (enable|disable)
**************************************/
DEFUN(config_vlan_egress_filter_cmd_fun,
	config_vlan_egress_filter_cmd,
	"config vlan egress-filter (enable|disable)",
	CONFIG_STR
	"Config layer 2 vlan entity\n"
	"Config vlan egress filter\n"
	"Enable eth-port egress filter \n"
	"Disable eth-port egress filter \n"
)
{
	DBusMessage *query, *reply;
	DBusError err;

	unsigned int isable = 0;
	int ret = 0;
	unsigned int op_ret = 0;
	
	if(strncmp("enable",argv[0],strlen(argv[0]))==0)
	{
		isable = 1;
	}
	else if (strncmp("disable",argv[0],strlen(argv[0]))==0)
	{
		isable = 0;
	}
	else
	{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_EGRESS_FILTER );
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&isable,
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
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) 
	{
		if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
			vty_out(vty,"%% Product not support this function!\n");
		}
		else if (NPD_DBUS_ERROR == op_ret) {
        	vty_out(vty,"% Config vlan mtu failed.\n");
		}
		else if(NPD_DBUS_SUCCESS == op_ret) {
			/*vty_out(vty,"Delete vlan OK.\n");*/
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

/*********************************
* 		config vlan filter
*********************************/
/************************
* config vlan filter
* Params:
*		filter enable/disable:
* commond:
* 		vlan config 
**************************************/
DEFUN(config_vlan_filter_cmd_fun,
	config_vlan_filter_cmd,
	"(enable|disable) filter (unkown-unicast|unreg-nonIp-multicast|unreg-ipv4-multicast|unreg-ipv6-multicast|unreg-nonIpv4-broadcast|unreg-ipv4-broadcast)",
	"Enable vlan filter\n"
	"Disable vlan filter\n"
	"Config layer2 vlan filter\n"
	"Unkown Unicast filter\n"
	"Unregister Non-IP Multicast filter\n"
	"Unregister IPv4 Multicast filter\n"
	"Unregister IPv6 Multicast filter\n"
	"Unregister Non-IPv4 Broadcast filter\n"
	"Unregister IPv4 Broadcast filter\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;

	VLAN_FLITER_ENT vlanfilterType = VLAN_FILTER_TYPE_MAX;
	unsigned int en_dis = 0, op_ret = 0,nodesave = 0;
	unsigned short vlanId = 0;
	if(0 == strncmp(argv[0],"enable",strlen(argv[0]))) {
		en_dis = TRUE;
	}
	else if (0 == strncmp(argv[0],"disable",strlen(argv[0]))) {
		en_dis = FALSE;
	}
	else {
		vty_out(vty,"% Bad parameter!\n");
		return CMD_WARNING;
	}

	if(0 == strncmp(argv[1],"unkown-unicast",strlen((char*)argv[1]))){
		vlanfilterType = VLAN_FILTER_UNKOWN_UC;
	}
	else if(0 == strncmp(argv[1],"unreg-ipv4-multicast",strlen((char*)argv[1]))){
		vlanfilterType = VLAN_FILTER_UNREG_IPV4_MC;
	}
	else if(0 == strncmp(argv[1],"unreg-ipv6-multicast",strlen((char*)argv[1]))){
		vlanfilterType = VLAN_FILTER_UNREG_IPV6_MC;
	}
	else if(0 == strncmp(argv[1],"unreg-nonIp-multicast",strlen((char*)argv[1]))){
		vlanfilterType = VLAN_FILTER_UNREG_NONIP_MC;
	}
	else if(0 == strncmp(argv[1],"unreg-ipv4-broadcast",strlen((char*)argv[1]))){
		vlanfilterType = VLAN_FILTER_UNREG_IPV4_BC;
	}
	else if(0 == strncmp(argv[1],"unreg-nonIpv4-broadcast",strlen((char*)argv[1]))){
		vlanfilterType = VLAN_FILTER_UNREG_NONIPV4_BC;
	}
	else {
		vty_out(vty,"%% Bad filter type!\n");
		return CMD_WARNING;
	}

	nodesave = (unsigned int)(vty->index);
	vlanId = (unsigned short)nodesave;
	
	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,	\
										NPD_DBUS_VLAN_METHOD_CONFIG_FILTER );
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT16,&vlanId,
							DBUS_TYPE_UINT32,&vlanfilterType,
							DBUS_TYPE_UINT32,&en_dis,
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
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) 
	{
		if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
			vty_out(vty,"%% Product not support this function!\n");
		}
		else if (NPD_DBUS_ERROR == op_ret) {
        	vty_out(vty,"%%  config vlan filter failed.\n");
		}
		else if(NPD_DBUS_SUCCESS == op_ret) {
			/*vty_out(vty,"Delete vlan OK.\n");*/
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

/*add by yinlm@autelan.com for config vlan egress-filter show and show runn 12.26*/
DEFUN(show_vlan_egress_filter_cmd_func,
	show_vlan_egress_filter_cmd,
	"show vlan egress-filter",
	"Show vlan egress-filter\n"
	"Show vlan egress-filter\n"
	"Show vlan egress-filter information\n"
)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = { 0 };
	unsigned int Isable = NPD_TRUE, ret = 0;
	query = dbus_message_new_method_call(
					NPD_DBUS_BUSNAME,  \
					NPD_DBUS_VLAN_OBJPATH ,  \
					NPD_DBUS_VLAN_INTERFACE ,  \
					NPD_DBUS_VLAN_METHOD_SHOW_EGRESS_FILTER );
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&Isable,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID)) {	
		if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
			vty_out(vty,"%% Product not support this function!\n");
		}
		else{
			if(NPD_TRUE == Isable) {
				vty_out(vty,"Vlan egress filter is enabled!\n");
			}
			if(NPD_FALSE == Isable) {
				vty_out(vty,"Vlan egress filter is disabled!\n");		
			}
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int dcli_vlan_egress_filter_show_running_config(struct vty *vty) {	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	int ret = 1;
	DBusMessage *query, *reply;
	DBusError err;
	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,	\
							NPD_DBUS_VLAN_OBJPATH ,	\
							NPD_DBUS_VLAN_INTERFACE ,	\
							NPD_DBUS_VLAN_METHOD_EGRESS_FILTER_SHOW_RUNNING_CONFIG);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show vlan egress filter running config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return ret;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) 
	{
		char _tmpstr[64];
		memset(_tmpstr,0,64);
		sprintf(_tmpstr,BUILDING_MOUDLE,"VLAN EGRESS FILTER");
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
	return ret;	
}

int dcli_vlan_show_running_config(struct vty* vty) {	
	int ret = 1;
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,	\
							NPD_DBUS_VLAN_OBJPATH ,	\
							NPD_DBUS_VLAN_INTERFACE ,	\
							NPD_DBUS_VLAN_METHOD_SHOW_RUNNING_CONFIG);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show vlan running config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return ret;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) 
	{
	
		char _tmpstr[64];
		memset(_tmpstr,0,64);
		sprintf(_tmpstr,BUILDING_MOUDLE,"VLAN");
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
	return ret;	
}

/* called by igmp/mld snooping show run */
int dcli_igmp_snp_vlan_show_running_config(unsigned char Ismld) {	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	int ret = 1;
	DBusMessage *query, *reply;
	DBusError err;
	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,	\
							NPD_DBUS_VLAN_OBJPATH ,	\
							NPD_DBUS_VLAN_INTERFACE ,	\
							NPD_DBUS_VLAN_METHOD_IGMP_SNP_VLAN_SHOW_RUNNING_CONFIG);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&Ismld,
							 DBUS_TYPE_INVALID);


	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_igmp,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show igmp_snp_vlan running config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return ret;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) 
	{
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
	return ret;	
}

void dcli_vlan_init() {
	install_node (&vlan_node, dcli_vlan_show_running_config, "VLAN_NODE");
	install_node (&vlan_egress_node, dcli_vlan_egress_filter_show_running_config, "VLAN_EGRESS_NODE");
	install_default(VLAN_NODE);

	install_element(VLAN_NODE,&config_vlan_add_del_port_cmd);
	install_element(VLAN_NODE,&config_vlan_add_del_portlist_cmd);
	//install_element(VLAN_NODE,&config_vlan_vid_update_cmd);
	install_element(VLAN_NODE,&config_vlan_filter_cmd);
	install_element(VLAN_NODE,&config_vlan_qinq_cmd);
	install_element(VLAN_NODE,&config_vlan_qinq_allbackport_cmd);
	/*install_element(VLAN_NODE,&config_vlan_add_del_subvlan_cmd);
	//install_element(VLAN_NODE,&config_vlan_add_del_trunk_cmd);*/
	install_element(VLAN_NODE,&vlan_add_del_trunk_tag_cmd);
	/*install_element(VLAN_NODE,&config_vlan_active_supervlan_cmd);
	//install_element(VLAN_NODE,&config_vlan_active_protvlan_cmd);
	//install_element(VLAN_NODE,&config_vlan_add_del_prot_ethertype_cmd);*/
	install_element(VLAN_NODE,&show_corrent_vlan_port_member_cmd);
	install_element(VLAN_NODE,&show_corrent_vlan_trunk_member_cmd);
	install_element(CONFIG_NODE,&set_port_vlan_ingress_filter_cmd);
	install_element(CONFIG_NODE,&set_port_pvid_cmd);
	install_element(CONFIG_NODE,&show_port_pvid_cmd);
	install_element(CONFIG_NODE,&show_port_list_pvid_cmd);
	install_element(CONFIG_NODE,&show_vlan_port_member_cmd);
	install_element(CONFIG_NODE,&show_vlan_trunk_member_cmd);
	install_element(CONFIG_NODE,&show_vlan_list_trunk_member_cmd);
	//install_element(CONFIG_NODE,&show_vlan_port_member_vname_cmd);
	//install_element(CONFIG_NODE,&show_vlan_trunk_member_vname_cmd);
	install_element(CONFIG_NODE,&show_vlan_list_port_member_cmd);
	install_element(VIEW_NODE,&show_vlan_port_member_cmd);
	install_element(VIEW_NODE,&show_vlan_trunk_member_cmd);
	install_element(VIEW_NODE,&show_vlan_list_trunk_member_cmd);
	//install_element(VIEW_NODE,&show_vlan_port_member_vname_cmd);
	//install_element(VIEW_NODE,&show_vlan_trunk_member_vname_cmd);
	install_element(VIEW_NODE,&show_vlan_list_port_member_cmd);
	install_element(ENABLE_NODE,&show_vlan_port_member_cmd);
	install_element(ENABLE_NODE,&show_vlan_trunk_member_cmd);
	install_element(ENABLE_NODE,&show_vlan_list_trunk_member_cmd);
	//install_element(ENABLE_NODE,&show_vlan_port_member_vname_cmd);
	//install_element(ENABLE_NODE,&show_vlan_trunk_member_vname_cmd);
	install_element(ENABLE_NODE,&show_vlan_list_port_member_cmd);

	install_element(CONFIG_NODE,&create_layer2_vlan_cmd);
	install_element(CONFIG_NODE,&config_layer2_vlan_cmd);
	//install_element(CONFIG_NODE,&config_layer2_vlan_vname_cmd);	
	install_element(CONFIG_NODE,&config_vlan_delete_vlan_cmd);
	//install_element(CONFIG_NODE,&config_vlan_delete_vlan_vname_cmd);
	install_element(CONFIG_NODE,&config_vlan_mtu_cmd);
	install_element(CONFIG_NODE,&config_vlan_egress_filter_cmd);
	install_element(CONFIG_NODE,&show_vlan_egress_filter_cmd);
    #ifndef AX_PLATFORM_DISTRIBUTED
	install_element(HIDDENDEBUG_NODE,&create_layer2_more_vlan_cmd);
	install_element(HIDDENDEBUG_NODE,&delete_layer2_more_vlan_cmd);	
	install_element(CONFIG_NODE,&show_vlan_list_count_cmd);
	/* add for Bond vlan to cpu port on special slot */
	install_element(CONFIG_NODE,&bond_vlan_to_cpu_port_cmd);
	/* bond vlan to master cpu or slave cpu */
	install_element(CONFIG_NODE,&bond_vlan_to_ms_cpu_port_cmd);	
	install_element(CONFIG_NODE,&config_vlan_qinq_tocpuport_cmd);
	install_element(CONFIG_NODE,&config_vlan_to_ms_cpu_port_qinq_cmd);
    #endif	
	/*install_element(CONFIG_NODE,&show_supervlan_cmd);*/
}
#ifdef __cplusplus
}
#endif
