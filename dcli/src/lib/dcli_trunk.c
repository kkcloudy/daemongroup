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
* npd_trunk.c
*
*
* CREATOR:
*		wujh@autelan.com
*
* DESCRIPTION:
*		DCLI for TRUNK module.
*
* DATE:
*		05/04/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.66 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include <zebra.h>
#include <stdlib.h>
#include <dbus/dbus.h>
#include <unistd.h>  /*getpagesize(  ) */  
#include <sys/ipc.h>   
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/mman.h>  /* for mmap() */
#include <errno.h>



#include "sysdef/npd_sysdef.h"
#include "dbus/npd/npd_dbus_def.h"
#include "util/npd_list.h"
#include "npd/nam/npd_amapi.h"
#include "command.h"
#include "if.h"
#include "dcli_main.h"
#include "dcli_vlan.h"
#include "dcli_trunk.h"
#include "sysdef/returncode.h"
#include "npd/nbm/npd_bmapi.h"

#include "dcli_main.h"
#include "dcli_sem.h"

struct cmd_node trunk_node = 
{
	TRUNK_NODE,
	"%s(config-trunk)# "
};

extern DBusConnection *dcli_dbus_connection;
extern int is_distributed;



static char *trunkLoadBalanc[LOAD_BANLC_MAX] = {
	 "mac",
	 "ingress port",
	 "ip",
	 "tcp/udp",
	 "mac+ip",
	 "mac+l4",
	 "ip+l4",
	 "mac+ip+l4"
};

int parse_trunk_no(char* str,unsigned short* trunkId)
{
	char *endptr = NULL;
	char c = 0;
	if (NULL == str) return NPD_FAIL;
	c = str[0];
	if (c>'0'&&c<='9'){
		*trunkId= (unsigned short)strtoul(str,&endptr,10);
		if('\0' != endptr[0]){
			return NPD_FAIL;
		}
		return NPD_SUCCESS;	
	}
	else {
		return NPD_FAIL; /*not Vlan ID. for Example ,enter '.',and so on ,some special char.*/
	}
}

int trunk_name_legal_check(char* str,unsigned int len)
{
	int i = 0;
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
		ret =NPD_SUCCESS;
	}
	else {
		return ALIAS_NAME_HEAD_ERROR;
	}
	for (i=1;i<=len-1;i++){
		c = str[i];
		if( (c>='0' && c<='9')||
			(c<='z'&&c>='a')||
		    (c<='Z'&&c>='A')||
		    (c=='_')){
			continue;
		}
		else {
			ret =ALIAS_NAME_BODY_ERROR;
			break;
		}
	}
	return ret;
}
/****************************************************
 * parse vlanlist IDs from string 
 * CMD :allow  vlan 2-5,11,20,25-32
 ****************************************************/
int parse_vid_list(char* ptr,int* count,short vid[])
{
    char* endPtr = NULL;
    short tmpvid = 0;
    int   i = 0,continus = 0,cc = 0;

    endPtr = ptr;
    if(NULL == endPtr){
       printf("return -1.\r\n");
       return -1;
	}
	tmpvid = strtoul(endPtr,&endPtr,10);
	if(VID_LLEGAL(tmpvid)){
        vid[i++] = tmpvid;
        /*printf("iter i = %d,vid = %d.\r\n",i,tmpvid);*/
    }
	while (endPtr) {
		if (VID_SPLIT_SLASH == endPtr[0]) {
	        tmpvid = strtoul((char *)&(endPtr[1]),&endPtr,10);
        	
        	if(VID_LLEGAL(tmpvid)){
                vid[i++] = tmpvid;
                /*printf("iter i = %d,vid = %d.\r\n",i,tmpvid);*/
            }
        }
		if (VID_SPLIT_DASH == endPtr[0]){
            
            tmpvid = strtoul((char *)&(endPtr[1]),&endPtr,10);
            /*printf("iter i = %d,tmpvid = %d.\r\n",i,tmpvid);*/
        	if(VID_LLEGAL(tmpvid)){
                continus = tmpvid - vid[i-1];
                if(0>continus){return -1;}
               }
            for(cc=0;cc < continus;cc++){
                vid[i++] = vid[i-1]+1; 
                /*printf("continus vid = %d.\r\n",vid[i-1]);   */        
            }
            /* *count = i;*/
            /*printf("enter vlan list contains: %d vlans.\r\n",i);*/
            endPtr = (char*)endPtr + 1;
        }
       
		if ('\0' == endPtr[0]) {
            /* *count = i;*/
			break;
		}
		else{
 	        tmpvid = strtoul((char *)&(endPtr[0]),&endPtr,10);
        	
        	if(VID_LLEGAL(tmpvid)){
                vid[i++] = tmpvid;
                /*printf("***iter i = %d,vid = %d.\r\n",i,tmpvid);*/
            }
        }
    }
    *count = i;
	return 0;	
}
/**************************************
*show_trunk_member_slot_port
*Params:
*		portmbrBmp0 - uint bitmap
*		portmbrBmp1 - uint bitmap
*
*Usage: show vlan/trunk membership with
*		in slot&port number.
**************************************/
int show_trunk_member_slot_port
(
	struct vty *vty,
	unsigned int product_id,
	PORT_MEMBER_BMP mbrBmp_sp,
	PORT_MEMBER_BMP disMbrBmp_sp
)
{
	unsigned int i = 0,count = 0;
	unsigned int slot = 0,port = 0;
    unsigned int  tmpVal[2];
	memset(&tmpVal,0,sizeof(tmpVal));
	for (i=0;i<64;i++){
		if((PRODUCT_ID_AX7K_I == product_id)||
			(PRODUCT_ID_AX7K == product_id)) {
			slot = i/8 + 1;
			port = i%8;
		}
		else if((PRODUCT_ID_AX5K == product_id) ||
				(PRODUCT_ID_AX5K_I == product_id) ||
				(PRODUCT_ID_AU4K == product_id) ||
				(PRODUCT_ID_AU3K == product_id) ||
				(PRODUCT_ID_AU3K_BCM== product_id) ||
				(PRODUCT_ID_AU3K_BCAT == product_id) || 
				(PRODUCT_ID_AU2K_TCAT == product_id)) {
			slot = 1;
			port = i;
		}

		tmpVal[i/32] = (1<<(i%32));
		if((mbrBmp_sp.portMbr[i/32]) & tmpVal[i/32]) {				
			if(count && (0 == count % 3)) {
				vty_out(vty,"\n%-55s"," ");
			}
			if(PRODUCT_ID_AX7K_I == product_id) {
				vty_out(vty,"%scscd%d(e)",(count%3) ? ",":"",port-1);
			}
			else {
				vty_out(vty,"%s%d/%d(e)",(count%3) ? ",":"",slot,port);
			}
			count++;
		}
	}

	slot = 0;
	port = 0;
	memset(&tmpVal,0,sizeof(tmpVal));
	for (i=0;i<64;i++){
		if((PRODUCT_ID_AX7K_I == product_id) ||
			(PRODUCT_ID_AX7K == product_id)) {
			slot = i/8 + 1;
			port = i%8;
		}
		else if((PRODUCT_ID_AX5K == product_id) ||
				(PRODUCT_ID_AX5K_I == product_id) ||
				(PRODUCT_ID_AU4K == product_id) ||
				(PRODUCT_ID_AU3K == product_id) ||
				(PRODUCT_ID_AU3K_BCM == product_id) ||
				(PRODUCT_ID_AU3K_BCAT == product_id) || 
				(PRODUCT_ID_AU2K_TCAT == product_id)) {
			slot = 1;
			port = i;
		}

		tmpVal[i/32] = (1<<(i%32));
		if((disMbrBmp_sp.portMbr[i/32]) & tmpVal[i/32]) {				
			if(count && (0 == count % 3)) {
				vty_out(vty,"\n%-55s"," ");
			}
			if(PRODUCT_ID_AX7K_I == product_id) {
				vty_out(vty,"%scscd%d(d)",(count%3) ? ",":"",port-1);
			}
			else {
				vty_out(vty,"%s%d/%d(d)",(count%3) ? ",":"",slot,port);
			}
			count++;
		}
	}
	
	return 0;
}

int dcli_trunk_map_table_update(
	struct vty *vty,
	unsigned char isAdd,
	unsigned short trunkId,
	unsigned char actdevNum,
	unsigned char portNum,
	unsigned char endis,
	unsigned char slot_no
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	int ret = 0;
	unsigned int op_ret = 0;
	int i;
	int local_slot_id = 0;
    int slotNum = 0;
	


	local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	if((local_slot_id < 0) || (slotNum <0))
	{
		vty_out(vty,"read file error ! \n");
		return CMD_WARNING;
	}

	for(i=1; i <= slotNum; i++)
	{
		
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_TRUNK_OBJPATH ,	\
											NPD_DBUS_TRUNK_INTERFACE ,	\
											NPD_DBUS_TRUNK_METHOD_TRUNK_MAP_TABLE_UPDATE );

		dbus_error_init(&err);
		dbus_message_append_args(query,
								DBUS_TYPE_BYTE,&isAdd,
						 		DBUS_TYPE_UINT16,&trunkId,
						 		DBUS_TYPE_BYTE,&actdevNum,
						 		DBUS_TYPE_BYTE,&portNum,
						 		DBUS_TYPE_BYTE,&endis,
								DBUS_TYPE_INVALID);

		if(i == slot_no)
		{
			continue;
		}
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
						DBUS_TYPE_UINT32, &op_ret,
						DBUS_TYPE_INVALID)) 
		{
			if(TRUNK_RETURN_CODE_TRUNK_EXISTS == op_ret) {  
				vty_out(vty,"%% Trunk %d already exists.\n",trunkId);
			}
			else if(TRUNK_RETURN_CODE_ERR_HW == op_ret) {
				vty_out(vty,"%% Create trunk %d hardware fail.\n",trunkId);
			}
			else if(TRUNK_RETURN_CODE_ERR_GENERAL == op_ret) {
				vty_out(vty,"%% Create trunk %d general failure.\n",trunkId);
			}
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
				vty_out(vty,"%% Product not support this function!\n");
			}
		} 
		else 
		{
			vty_out(vty,"failed get args.\n");
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

/* update the g_trunks on 2 MCBs */
int dcli_g_trunks_add_del_port_set_master
(
	struct vty* vty,
	boolean addel,
	unsigned short trunkid,
	unsigned char  slotno, 
	unsigned char  portno,
	unsigned char  master,
	unsigned char  devnum,
	unsigned char  virportnum,
	unsigned char endis
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
		return CMD_WARNING;		
   	}
	if((local_slot_id<0)||(master_slot_count<0))
	{
		vty_out(vty,"get get_product_info return -1 !\n");
		return CMD_WARNING;		
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
											NPD_DBUS_TRUNK_OBJPATH,	\
											NPD_DBUS_TRUNK_INTERFACE,	\
											NPD_DBUS_TRUNK_METHOD_PORT_MEMBER_TRUNK_LIST_ADD_DEL);
	
		dbus_error_init(&err);

		dbus_message_append_args(	query,
							 		DBUS_TYPE_BYTE,&addel,
									DBUS_TYPE_BYTE,&slotno,
									DBUS_TYPE_BYTE,&portno,
									DBUS_TYPE_BYTE,&master,
									DBUS_TYPE_BYTE,&devnum,
									DBUS_TYPE_BYTE,&virportnum,
									DBUS_TYPE_BYTE,&endis,
							 		DBUS_TYPE_UINT16,&trunkid,
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
    		if (TRUNK_RETURN_CODE_ERR_NONE != op_ret){
    			vty_out(vty,"%% trunk add/delete port Error! return %d \n",op_ret);
    		}		
    	} 
    	else {
    		vty_out(vty,"Failed get args,Please check npd on MCB slot %d\n",slot_id);
    	}
    	dbus_message_unref(reply);
    	if(TRUNK_RETURN_CODE_ERR_NONE != op_ret)
    	{
    		return CMD_WARNING;
    	}
    }
	return CMD_SUCCESS;	
}

/**************************************
* create trunk entity on Both Sw & Hw. 
* Params: 
*       trunk ID:	<1-110>
*		trunk name:  such as "autelan0","autelan1","_office"
*
* Usage: create trunk <1-110> TRUNKNAME 
****************************************/
DEFUN(create_trunk_cmd_fun, 
	create_trunk_cmd, 
	"create trunk <1-64> TRUNKNAME",
	"Create trunk on system\n"
	"Trunk entity\n"
	"Trunk ID range <1-64>\n"
	"Trunk name begins with alphabet or '_',no more than 20 alphabets\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned short 	trunkId = 0;
	char*  trunkName = NULL;
	int ret = 0;
	unsigned int op_ret = 0;
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
		ret = parse_trunk_no((char*)argv[0],&trunkId);
		if (NPD_FAIL == ret) {
    		vty_out(vty,"% Bad parameter,trunk id illegal!\n");
			return CMD_WARNING; 
		}
		trunkName = (char*)malloc(ALIAS_NAME_SIZE);
		if(NULL == trunkName){
    		vty_out(vty,"% Memory resource error!\n");
			return CMD_WARNING;
		}
		memset(trunkName,0,ALIAS_NAME_SIZE);

		if(strcmp((char*)argv[1],"list")==0){
			vty_out(vty,"% Can not assign this name to trunk!\n");
			free(trunkName);
			return CMD_WARNING;
		}
		for(i=1; i <= slotNum; i++)
		{
			ret = trunk_name_legal_check((char*)argv[1],strlen(argv[1]));
			if(ALIAS_NAME_LEN_ERROR == ret) {
				vty_out(vty,"% Bad parameter,TRUNKNAME length must no more than 20!\n");
				free(trunkName);
				return CMD_WARNING;
			}
			else if(ALIAS_NAME_HEAD_ERROR == ret) {
				vty_out(vty,"% Bad parameter,TRUNKNAME must begin with alphabet or '_'\n");
				free(trunkName);
				return CMD_WARNING;
			}
			else if(ALIAS_NAME_BODY_ERROR == ret) {
				vty_out(vty,"% Bad parameter,TRUNKNAME contains illegal character!\n");
				free(trunkName);
				return CMD_WARNING;
			}
			else {
				memcpy(trunkName,argv[1],strlen(argv[1]));
				query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
													NPD_DBUS_TRUNK_OBJPATH ,	\
													NPD_DBUS_TRUNK_INTERFACE ,	\
													NPD_DBUS_TRUNK_METHOD_CREATE_TRUNK_ONE );
		
				dbus_error_init(&err);
				dbus_message_append_args(query,
								 		DBUS_TYPE_UINT16,&trunkId,
								 		DBUS_TYPE_STRING,&trunkName,
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
			}
			dbus_message_unref(query);
	
			if (NULL == reply) {
				vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",i);
				if (dbus_error_is_set(&err)) {
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				free(trunkName);
				return CMD_SUCCESS;
			}

			if (dbus_message_get_args ( reply, &err,
							DBUS_TYPE_UINT32, &op_ret,
							DBUS_TYPE_INVALID)) 
			{
				if(TRUNK_RETURN_CODE_TRUNK_EXISTS == op_ret) {  
					vty_out(vty,"%% Trunk %d already exists.\n",trunkId);
				}
				else if(TRUNK_RETURN_CODE_NAME_CONFLICT == op_ret) {
					vty_out(vty,"%% Trunk name %s conflict.\n",trunkName);
				}
				else if(TRUNK_RETURN_CODE_ERR_HW == op_ret) {
					vty_out(vty,"%% Create trunk %d hardware fail.\n",trunkId);
				}
				else if(TRUNK_RETURN_CODE_ERR_GENERAL == op_ret) {
					vty_out(vty,"%% Create trunk %d general failure.\n",trunkId);
				}
				else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
					vty_out(vty,"%% Product not support this function!\n");
				}
			} 
			else 
			{
				vty_out(vty,"failed get args.\n");
				if (dbus_error_is_set(&err)) 
				{
					vty_out(vty,"%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
			}
			dbus_message_unref(reply);
		}
		free(trunkName);
	}
	else
	{
		/*get trunk ID*/
		ret = parse_trunk_no((char*)argv[0],&trunkId);
		if (NPD_FAIL == ret) {
    		vty_out(vty,"% Bad parameter,trunk id illegal!\n");
			return CMD_WARNING; 
		}
		trunkName = (char*)malloc(ALIAS_NAME_SIZE);
		if(NULL == trunkName){
    		vty_out(vty,"% Memory resource error!\n");
			return CMD_WARNING;
		}
		memset(trunkName,0,ALIAS_NAME_SIZE);

		if(strcmp((char*)argv[1],"list")==0){
			vty_out(vty,"% Can not assign this name to trunk!\n");
			free(trunkName);
			return CMD_WARNING;
		}
		ret = trunk_name_legal_check((char*)argv[1],strlen(argv[1]));
		if(ALIAS_NAME_LEN_ERROR == ret) {
			vty_out(vty,"% Bad parameter,TRUNKNAME length must no more than 20!\n");
			free(trunkName);
			return CMD_WARNING;
		}
		else if(ALIAS_NAME_HEAD_ERROR == ret) {
			vty_out(vty,"% Bad parameter,TRUNKNAME must begin with alphabet or '_'\n");
			free(trunkName);
			return CMD_WARNING;
		}
		else if(ALIAS_NAME_BODY_ERROR == ret) {
			vty_out(vty,"% Bad parameter,TRUNKNAME contains illegal character!\n");
			free(trunkName);
			return CMD_WARNING;
		}
		else {
			memcpy(trunkName,argv[1],strlen(argv[1]));
			query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
												NPD_DBUS_TRUNK_OBJPATH ,	\
												NPD_DBUS_TRUNK_INTERFACE ,	\
												NPD_DBUS_TRUNK_METHOD_CREATE_TRUNK_ONE );
		
			dbus_error_init(&err);
			dbus_message_append_args(query,
								 	DBUS_TYPE_UINT16,&trunkId,
								 	DBUS_TYPE_STRING,&trunkName,
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
			free(trunkName);
			return CMD_SUCCESS;
		}

		if (dbus_message_get_args ( reply, &err,
						DBUS_TYPE_UINT32, &op_ret,
						DBUS_TYPE_INVALID)) 
		{
			if(TRUNK_RETURN_CODE_TRUNK_EXISTS == op_ret) {  
				vty_out(vty,"%% Trunk %d already exists.\n",trunkId);
			}
			else if(TRUNK_RETURN_CODE_NAME_CONFLICT == op_ret) {
				vty_out(vty,"%% Trunk name %s conflict.\n",trunkName);
			}
			else if(TRUNK_RETURN_CODE_ERR_HW == op_ret) {
				vty_out(vty,"%% Create trunk %d hardware fail.\n",trunkId);
			}
			else if(TRUNK_RETURN_CODE_ERR_GENERAL == op_ret) {
				vty_out(vty,"%% Create trunk %d general failure.\n",trunkId);
			}
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
				vty_out(vty,"%% Product not support this function!\n");
			}
		} 
		else 
		{
			vty_out(vty,"failed get args.\n");
			if (dbus_error_is_set(&err)) 
			{
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
		}
		dbus_message_unref(reply);
		free(trunkName);
	}
	return CMD_SUCCESS;
}

/***********************************
* Enter trunk config mode to configure 
* special trunk.
* Params:
*		trunk ID:	 <1-118>
* Usage: config trunk <1-118>
************************************/

DEFUN(config_trunk_func,
	config_trunk_cmd,
	"config trunk <1-64>",
	CONFIG_STR
	"Config layer 2 trunk entity\n"
	"Specify trunk ID for trunk entity\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned short 	trunkId = 0;
	int ret = 0;
	unsigned int op_ret = 0, nodesave = 0;
	int i;
	int local_slot_id = 0;
    int slotNum = 0;
	
	ret = parse_trunk_no((char*)argv[0],&trunkId);

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
			if (NPD_FAIL == ret) {
    			vty_out(vty,"% Bad parameter,trunk ID illegal!\n");
				return CMD_WARNING;
			}
			else {
			/*once bad param,it'll NOT sed message to NPD*/
				query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
													NPD_DBUS_TRUNK_OBJPATH ,	\
													NPD_DBUS_TRUNK_INTERFACE ,	\
													NPD_DBUS_TRUNK_METHOD_CONFIG_ONE);
		
				dbus_error_init(&err);

				dbus_message_append_args(query,
								 		DBUS_TYPE_UINT16,&trunkId,
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

			if (dbus_message_get_args( reply, &err,
							DBUS_TYPE_UINT32, &op_ret,
							DBUS_TYPE_INVALID)) 
			{	
				if (ETHPORT_RETURN_CODE_NO_SUCH_TRUNK == op_ret) 
				{
        			vty_out(vty,"% Bad parameter,trunk %d not exist.\n",trunkId);/*Stay here,not enter trunk_config_node CMD.*/
				}
				else if (TRUNK_RETURN_CODE_ERR_GENERAL == op_ret ) 
				{
					vty_out(vty,"% Config trunk %d out of range.\n",trunkId); /*Stay here,not enter trunk_config_node CMD.*/
				}
				else if(TRUNK_RETURN_CODE_ERR_NONE == op_ret)   /*0+3,trunk exist,then enter trunk_config_node CMD.*/
				{
					dbus_message_unref(reply);
					continue;
				}
				else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
					vty_out(vty,"%% Product not support this function!\n");
				}
                /* one op_ret is error, then return */
				dbus_message_unref(reply);
            	return CMD_WARNING;				
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
			}
		}

		if(CONFIG_NODE == vty->node) 
		{
			vty->node = TRUNK_NODE;
			nodesave = trunkId;
			vty->index = (void*)nodesave;/*when not add & before trunkId, the Vty enter <config-line> CMD Node.*/
		}
		else
		{
			vty_out (vty, "% CLI mode must be CONFIG mode!%s",VTY_NEWLINE);
			return CMD_WARNING;
		}
		return CMD_SUCCESS;
	}
	else
	{
		if (NPD_FAIL == ret) {
    		vty_out(vty,"% Bad parameter,trunk ID illegal!\n");
			return CMD_WARNING;
		}
		else {
			/*once bad param,it'll NOT sed message to NPD*/
			query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
												NPD_DBUS_TRUNK_OBJPATH ,	\
												NPD_DBUS_TRUNK_INTERFACE ,	\
												NPD_DBUS_TRUNK_METHOD_CONFIG_ONE);
		
			dbus_error_init(&err);

			dbus_message_append_args(query,
								 	DBUS_TYPE_UINT16,&trunkId,
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

		if (dbus_message_get_args( reply, &err,
						DBUS_TYPE_UINT32, &op_ret,
						DBUS_TYPE_INVALID)) 
		{	
			if (ETHPORT_RETURN_CODE_NO_SUCH_TRUNK == op_ret) 
			{
        		vty_out(vty,"% Bad parameter,trunk not exist.\n",trunkId);/*Stay here,not enter trunk_config_node CMD.*/
			}
			else if (TRUNK_RETURN_CODE_ERR_GENERAL == op_ret ) 
			{
				vty_out(vty,"% Config trunk general failure.\n",trunkId); /*Stay here,not enter trunk_config_node CMD.*/
			}
			else if(TRUNK_RETURN_CODE_ERR_NONE == op_ret)   /*0+3,trunk exist,then enter trunk_config_node CMD.*/
			{
				if(CONFIG_NODE == vty->node) {
					vty->node = TRUNK_NODE;
					nodesave = trunkId;
					vty->index = (void*)nodesave;/*when not add & before trunkId, the Vty enter <config-line> CMD Node.*/
				}
				else{
					vty_out (vty, "% CLI mode must be CONFIG mode!%s",VTY_NEWLINE);
					return CMD_WARNING;
				}
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
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
			dbus_message_unref(reply);
		}
	}
	return CMD_SUCCESS;
}
/***********************************
* Enter trunk config mode to configure 
* special trunk.
* Params:
*		trunk name:	 
* Usage: config trunk TRUNKNAME
************************************/
DEFUN(config_trunk_vname_func,
	config_trunk_name_cmd,
	"config trunk TRUNKNAME",
	CONFIG_STR
	"Config layer 2 trunk entity\n"
	"Trunk name begins with char or'_',no more than 20 chars\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	char* 	trunkName = NULL;
	unsigned short trunkId = 0;
	unsigned int nameSize = 0,nodesave = 0;
	int ret = 0;
	unsigned int op_ret = 0;
	int i;
	int local_slot_id = 0;
    int slotNum = 0;
	
	trunkName = (char*)malloc(ALIAS_NAME_SIZE);
	if(NULL == trunkName){
    	vty_out(vty,"% Memory resource error!\n");
		return CMD_WARNING;
	}	
	memset(trunkName,0,ALIAS_NAME_SIZE);

	ret = trunk_name_legal_check((char*)argv[0],strlen(argv[0]));
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
			if(ALIAS_NAME_LEN_ERROR == ret) {
				vty_out(vty,"% Bad parameter,trunk name too long!\n");
				return CMD_WARNING;
			}
			else if(ALIAS_NAME_HEAD_ERROR == ret) {
				vty_out(vty,"% Bad parameter,trunk name begins with an illegal char!\n");
				return CMD_WARNING;
			}
			else if(ALIAS_NAME_BODY_ERROR == ret) {
				vty_out(vty,"% Bad parameter,trunk name contains illegal char!\n");
				return CMD_WARNING;
			}
			else{
				nameSize = strlen(argv[0]);
				memcpy(trunkName,argv[0],nameSize);
				query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
													NPD_DBUS_TRUNK_OBJPATH ,	\
													NPD_DBUS_TRUNK_INTERFACE ,	\
													NPD_DBUS_TRUNK_METHOD_CONFIG_VIA_TRUNKNAME );
		
				dbus_error_init(&err);
				dbus_message_append_args(query,
									 	DBUS_TYPE_STRING,&trunkName, 
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
							DBUS_TYPE_UINT32, &op_ret,
							DBUS_TYPE_UINT16, &trunkId,
							DBUS_TYPE_INVALID)) 
			{
				if (TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret ) 
				{
					vty_out(vty,"% Bad parameter,trunk %s not exist.\n",trunkName); /*Stay here,not enter trunk_config_node CMD.*/
				}
				else  if(TRUNK_RETURN_CODE_ERR_NONE == op_ret)   /*0+3,Trunk exist,then enter trunk_config_node CMD.*/
				{
					
					dbus_message_unref(reply);
					continue;
				}
				else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
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
				dbus_message_unref(reply);
				free(trunkName);
				return CMD_SUCCESS;
			}
		}
		if(CONFIG_NODE == vty->node) 
		{
			vty->node = TRUNK_NODE;
			nodesave = trunkId;
			vty->index = (void*)nodesave;/*when not add & before trunkId, the Vty enter <config-line> CMD Node.*/
		}
		else
		{
			vty_out (vty, "% Terminal mode change must under configure mode.\n", VTY_NEWLINE);
			return CMD_WARNING;
		}
		return CMD_SUCCESS;
	}
	else
	{
		if(ALIAS_NAME_LEN_ERROR == ret) {
			vty_out(vty,"% Bad parameter,trunk name too long!\n");
			return CMD_WARNING;
		}
		else if(ALIAS_NAME_HEAD_ERROR == ret) {
			vty_out(vty,"% Bad parameter,trunk name begins with an illegal char!\n");
			return CMD_WARNING;
		}
		else if(ALIAS_NAME_BODY_ERROR == ret) {
			vty_out(vty,"% Bad parameter,trunk name contains illegal char!\n");
			return CMD_WARNING;
		}
		else{
			nameSize = strlen(argv[0]);
			memcpy(trunkName,argv[0],nameSize);
			query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
												NPD_DBUS_TRUNK_OBJPATH ,	\
												NPD_DBUS_TRUNK_INTERFACE ,	\
												NPD_DBUS_TRUNK_METHOD_CONFIG_VIA_TRUNKNAME );
		
			dbus_error_init(&err);
			dbus_message_append_args(query,
									 DBUS_TYPE_STRING,&trunkName, 
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
						DBUS_TYPE_UINT16, &trunkId,
						DBUS_TYPE_INVALID)) 
		{
			if (TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret ) 
			{
				vty_out(vty,"% Bad parameter,trunk %s not exist.\n",trunkName); /*Stay here,not enter trunk_config_node CMD.*/
			}
			else  if(TRUNK_RETURN_CODE_ERR_NONE == op_ret)   /*0+3,Trunk exist,then enter trunk_config_node CMD.*/
			{
				if(CONFIG_NODE == vty->node) {
					vty->node = TRUNK_NODE;
					nodesave = trunkId;
					vty->index = (void*)nodesave;/*when not add & before trunkId, the Vty enter <config-line> CMD Node.*/
				}
				else{
					vty_out (vty, "% Terminal mode change must under configure mode.\n", VTY_NEWLINE);
					return CMD_WARNING;
				}
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
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
		dbus_message_unref(reply);
		free(trunkName);
		return CMD_SUCCESS;
		}
	}
}
/**************************************
* add | delete  trunk port  on Both Sw & Hw. 
*Execute at trunk-config Node
* Params: 
*              PORTNO port number
*
* Usage: add port  slot/port 
****************************************/

DEFUN(add_delete_trunk_member_cmd_fun, 
	add_delete_trunk_member_cmd, 
	"(add|delete) port PORTNO",
	"Add port into trunk\n"
	"Delete port from trunk\n"
	"Port on system\n"
	CONFIG_ETHPORT_STR)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned char slot_no = 0, local_port_no = 0;
	unsigned int  t_slotno = 0, t_portno = 0;
	boolean 	  isAdd = FALSE;
	boolean       isTagged = FALSE;
	unsigned short	trunkId = 0;
	unsigned int 	op_ret = 0, nodesave = 0;
	int local_slot_id = 0;
    int slotNum = 0;
	unsigned char  master= 0;
	unsigned char actdevNum = 0,virportNum = 0,endis = 0;
	unsigned char is_same_board_port = 0,cscd_type = 0; /*only the cacd system can add ports from different boards*/
	int fd = -1;
	struct stat sb;

	dst_trunk_s *trunk_list =NULL;
	char* file_path = "/dbm/shm/trunk/shm_trunk";	
	

	/*fetch the 1st param : add ? delete*/
	if(strncmp(argv[0],"add",strlen(argv[0]))==0) {
		isAdd = TRUE;
	}
	else if (strncmp(argv[0],"delete",strlen(argv[0]))==0) {
		isAdd = FALSE;
	}
	else {
		vty_out(vty,"% Bad parameter.\n");
		return CMD_WARNING;
	}
	/*fetch the 2nd param : slotNo/portNo*/
	op_ret = parse_slotno_localport((char *)argv[1],&t_slotno,&t_portno);
   	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad parameter,unknow portno format.\n");
		return CMD_WARNING;
	}
	else if (1 == op_ret){
		vty_out(vty,"% Bad parameter,bad slot/port number.\n");
		return CMD_WARNING;
	}
	slot_no = (unsigned char)t_slotno;
	local_port_no = (unsigned char)t_portno;
	
	/*if (NPD_FAIL == op_ret) {
    	vty_out(vty,"Unknown PORTNO format.\n");
		return CMD_SUCCESS;
	}*/

	nodesave = (unsigned int)(vty->index);
	trunkId = (unsigned short)nodesave;

	if(is_distributed == DISTRIBUTED_SYSTEM)
	{
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		/* only read file */
		if(isAdd)
		{
		    fd = open(file_path, O_RDONLY);
	    	if(fd < 0)
	        {
	            vty_out(vty,"Failed to open! %s\n", strerror(errno));
	            return NULL;
	        }
			fstat(fd,&sb);
			/* map not share */	
	        trunk_list = (dst_trunk_s *)mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0 );
	        if(MAP_FAILED == trunk_list)
	        {
	            vty_out(vty,"Failed to mmap for g_dst_trunk[]! %s\n", strerror(errno));
				close(fd);
	            return NULL;
	        }
			if(trunk_list[trunkId-1].portLog != 0)
			{
				if(slot_no == trunk_list[trunkId-1].portmap[0].slot)
				{
					is_same_board_port = 1;/*0 means different boards port,1 means same board*/
				}
				else
				{
					is_same_board_port = 0;
				}
			}
			else
			{
				is_same_board_port = 1;
			}
			
			/* munmap and close fd */
		    op_ret = munmap(trunk_list,sb.st_size);
		    if( op_ret != 0 )
		    {
		        vty_out(vty,"Failed to munmap for g_trunklist[]! %s\n", strerror(errno));			
		    }	
			op_ret = close(fd);
			if( op_ret != 0 )
		    {
		        vty_out(vty,"close shm_vlan failed \n" );   
		    }
		}
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_TRUNK_OBJPATH,	\
											NPD_DBUS_TRUNK_INTERFACE,	\
											NPD_DBUS_TRUNK_METHOD_PORT_MEMBER_ADD_DEL);
	
		dbus_error_init(&err);

		dbus_message_append_args(	query,
							 		DBUS_TYPE_BYTE,&isAdd,
									DBUS_TYPE_BYTE,&slot_no,
									DBUS_TYPE_BYTE,&local_port_no,
									DBUS_TYPE_BYTE,&is_same_board_port,
							 		DBUS_TYPE_UINT16,&trunkId,
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
			vty_out(vty,"failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				printf("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}

		if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_BYTE, &actdevNum,
			DBUS_TYPE_BYTE,&virportNum,
			DBUS_TYPE_BYTE,&endis,
			DBUS_TYPE_BYTE,&cscd_type,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID)) {
				if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret) {
					vty_out(vty,"%% Bad parameter,port %d/%d not exist.\n",slot_no,local_port_no);
				}
				else if (TRUNK_RETURN_CODE_BADPARAM == op_ret) {
					vty_out(vty,"%% Error occurs in Parse portNo or deviceNo.\n");
				}
				else if (TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret) {
					vty_out(vty,"%% Trunk to be configured not exists on SW.\n");
				}
				else if (TRUNK_RETURN_CODE_PORT_EXISTS == op_ret) {
					vty_out(vty,"%% Port was already the member of this trunk.\n ");
				}
				else if (TRUNK_RETURN_CODE_PORT_NOTEXISTS == op_ret){
					vty_out(vty,"%% Port was not member of this trunk ever.\n");
				}
				else if (TRUNK_RETURN_CODE_PORT_MBRS_FULL == op_ret) {
					vty_out(vty,"%% Trunk port member full on hardware.\n"); /*such as add port to trunkNode struct.*/
				}
				else if (TRUNK_RETURN_CODE_ERR_HW == op_ret) {
					vty_out(vty,"%% Config trunk on hardware general failure.\n");	
				}
				else if (TRUNK_RETURN_CODE_ERR_GENERAL == op_ret) {
					vty_out(vty,"%% Error occurs in config on software.\n"); /*such as add port to trunkNode struct.*/
				}
				else if (TRUNK_RETURN_CODE_MEMBERSHIP_CONFICT == op_ret) {
					vty_out(vty,"%% Error,this port is a member of other trunk.\n"); /*such as add port to trunkNode struct.*/
				}
				else if (TRUNK_RETURN_CODE_PORT_CONFIG_DIFFER == op_ret) {
					vty_out(vty,"%% Error,this port has not the same configuration as master port.\n");
				}			
				else if (TRUNK_RETURN_CODE_PORT_L3_INTFG == op_ret) {
					vty_out(vty,"%% Error,this port is L3 interface.\n");
				}
				else if (TRUNK_RETURN_CODE_MEMBER_ADD_ERR == op_ret) {
					vty_out(vty,"%% Error on adding this port to default or allowed vlans.\n");
				}
				else if (TRUNK_RETURN_CODE_MEMBER_DEL_ERR == op_ret) {
					vty_out(vty,"%% Error on deleting this port from allowed vlan.\n");
				}
				else if (TRUNK_RETURN_CODE_SET_TRUNKID_ERR == op_ret) {
					vty_out(vty,"%% Error on setting port trunkId.\n");
				}
				else if (TRUNK_RETURN_CODE_DEL_MASTER_PORT == op_ret) {
					vty_out(vty,"%% Can not delete master port.\n");
				}	
				else if (TRUNK_RETURN_CODE_UNSUPPORT == op_ret) {
					vty_out(vty,"%% This operation is unsupported!\n");
				}
				else if (TRUNK_RETURN_CODE_BASE == op_ret)
				{
					vty_out(vty,"%% Not CSCD system,do not allow to add ports from differets boards!\n");
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


        if(TRUNK_RETURN_CODE_ERR_NONE == op_ret)
        {
           	/* update g_vlanlist[] on Active-MCB & Standby-MCB. zhangdi 20110804*/
        	if(is_distributed == DISTRIBUTED_SYSTEM)	
        	{
				if(cscd_type == 1)/*only CSCD SYSTEM update trunk map table*/
				{
					op_ret = dcli_trunk_map_table_update(vty,isAdd,trunkId,actdevNum,virportNum,endis,slot_no);
					if(CMD_SUCCESS != op_ret)
					{
						vty_out(vty,"update trunk map table FAIL !\n");
						return CMD_WARNING;
					}
				}
    			vty_out(vty,"update g_dst_trunk on SMU !\n");			
            	/* Wherever slot the CMD we input, then update the gvlanlist[] of 2 MCBs */
        		op_ret = dcli_g_trunks_add_del_port_set_master(vty,isAdd,trunkId,slot_no,local_port_no,master,actdevNum,virportNum,endis);
        	    if( CMD_SUCCESS != op_ret)
        		{
        			vty_out(vty,"update g_dst_trunk on SMU error !\n");					
        		    return CMD_WARNING;
        	    }
        	}					
        }
		return CMD_SUCCESS;
	}
	else
	{
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_TRUNK_OBJPATH,	\
											NPD_DBUS_TRUNK_INTERFACE,	\
											NPD_DBUS_TRUNK_METHOD_PORT_MEMBER_ADD_DEL);
	
		dbus_error_init(&err);

		dbus_message_append_args(	query,
							 		DBUS_TYPE_BYTE,&isAdd,
									DBUS_TYPE_BYTE,&slot_no,
									DBUS_TYPE_BYTE,&local_port_no,
							 		DBUS_TYPE_UINT16,&trunkId,
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
					vty_out(vty,"%% Bad parameter,port %d/%d not exist.\n",slot_no,local_port_no);
				}
				else if (TRUNK_RETURN_CODE_BADPARAM == op_ret) {
					vty_out(vty,"%% Error occurs in Parse portNo or deviceNo.\n");
				}
				else if (TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret) {
					vty_out(vty,"%% Trunk to be configured not exists on SW.\n");
				}
				else if (TRUNK_RETURN_CODE_PORT_EXISTS == op_ret) {
					vty_out(vty,"%% Port was already the member of this trunk.\n ");
				}
				else if (TRUNK_RETURN_CODE_PORT_NOTEXISTS == op_ret){
					vty_out(vty,"%% Port was not member of this trunk ever.\n");
				}
				else if (TRUNK_RETURN_CODE_PORT_MBRS_FULL == op_ret) {
					vty_out(vty,"%% Trunk port member full on hardware.\n"); /*such as add port to trunkNode struct.*/
				}
				else if (TRUNK_RETURN_CODE_ERR_HW == op_ret) {
					vty_out(vty,"%% Config trunk on hardware general failure.\n");	
				}
				else if (TRUNK_RETURN_CODE_ERR_GENERAL == op_ret) {
					vty_out(vty,"%% Error occurs in config on software.\n"); /*such as add port to trunkNode struct.*/
				}
				else if (TRUNK_RETURN_CODE_MEMBERSHIP_CONFICT == op_ret) {
					vty_out(vty,"%% Error,this port is a member of other trunk.\n"); /*such as add port to trunkNode struct.*/
				}
				else if (TRUNK_RETURN_CODE_PORT_CONFIG_DIFFER == op_ret) {
					vty_out(vty,"%% Error,this port has not the same configuration as master port.\n");
				}			
				else if (TRUNK_RETURN_CODE_PORT_L3_INTFG == op_ret) {
					vty_out(vty,"%% Error,this port is L3 interface.\n");
				}
				else if (TRUNK_RETURN_CODE_MEMBER_ADD_ERR == op_ret) {
					vty_out(vty,"%% Error on adding this port to default or allowed vlans.\n");
				}
				else if (TRUNK_RETURN_CODE_MEMBER_DEL_ERR == op_ret) {
					vty_out(vty,"%% Error on deleting this port from allowed vlan.\n");
				}
				else if (TRUNK_RETURN_CODE_SET_TRUNKID_ERR == op_ret) {
					vty_out(vty,"%% Error on setting port trunkId.\n");
				}
				else if (TRUNK_RETURN_CODE_DEL_MASTER_PORT == op_ret) {
					vty_out(vty,"%% Can not delete master port.\n");
				}	
				else if (TRUNK_RETURN_CODE_UNSUPPORT == op_ret) {
					vty_out(vty,"%% This operation is unsupported!\n");
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
}

/***********************************************************
 *
 *		==Trunk (allow|refuse) (vlan-list|all)
 *
 *
 *
 ***********************************************************/

DEFUN(trunk_allow_refuse_vlan_cmd_fun, 
	trunk_allow_refuse_vlan_cmd, 
	"(allow|refuse) vlan <2-4094> (tag|untag)",/*(VLAN_LIST|all)",*/
	"Allow trunk to transmit packets for vlan\n"
	"Refuse trunk form transmit packets for vlan\n"
	"Vlan on system\n"
	"Vlan list will be allowed or refused\n"
	"Trunk allow/refuse vlan tagged\n"
	"Trunk allow/refuse vlan untagged\n")
	/*"All vlans in system allowed or refused\n")*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned char slot_no = 0, local_port_no = 0;
	boolean isAllow = FALSE;
	boolean isTag	= FALSE;	
	unsigned short	trunkId = 0, vid = 0;/*vid[VID_MAX_NUM] = {0};*/
	unsigned int 	count = 1, ret = 0, op_ret = 0, nodesave = 0;
	int i;
	
	int local_slot_id = 0;
	int slotNum = 0;
	

	/*fetch the 1st param : add ? delete*/
	if(strncmp(argv[0],"allow",strlen((char*)argv[0]))==0) {
		isAllow = TRUE;
	}
	else if (strncmp(argv[0],"refuse",strlen((char*)argv[0]))==0) {
		isAllow = FALSE;
	}
	else {
		vty_out(vty,"% Bad command parameter0!\n");
		return CMD_WARNING;
	}		
	/*
	if(0 != parse_vid_list((char*) argv[1], &count, vid)){
		vty_out(vty,"Unkown Format Params.");
		return CMD_WARNING;
	}
	*/
	ret = parse_vlan_no((char*)argv[1],&vid);
	if(0 != ret ){
		vty_out(vty,"% Bad vlan parameter.\n");
		return CMD_WARNING;
	}

	/*fetch the 3rd param : tag ? untag*/
	if(strncmp(argv[2],"tag",strlen((char*)argv[2]))==0) {
		isTag = TRUE;
	}
	else if (strncmp(argv[2],"untag",strlen((char*)argv[2]))==0) {
		isTag = FALSE;
	}
	else {
		vty_out(vty,"% Bad command parameter.\n");
		return CMD_WARNING;
	}		

	nodesave = (unsigned int)(vty->index);
	trunkId = nodesave;
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
			query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
												NPD_DBUS_TRUNK_OBJPATH,	\
												NPD_DBUS_TRUNK_INTERFACE,	\
												NPD_DBUS_TRUNK_METHOD_ALLOW_REFUSE_VLAN_LIST);

			dbus_error_init(&err);

			dbus_message_append_args(	query,
							 			DBUS_TYPE_BYTE,&isAllow,
							 			DBUS_TYPE_BYTE,&isTag,
							 			DBUS_TYPE_UINT32,&count,
										DBUS_TYPE_UINT16,&vid,
							 			DBUS_TYPE_UINT16,&trunkId,
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
					printf("%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_SUCCESS;
			}

			if (dbus_message_get_args ( reply, &err,
				DBUS_TYPE_UINT32,&op_ret,
				DBUS_TYPE_INVALID)) {
					if (TRUNK_RETURN_CODE_ERR_NONE == op_ret ) {
						/*vty_out(vty,"trunk %d %s vlan %d success.\n",trunkId,isAllow?"Allow":"Refuse",vid);*/
					}
					else if (VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret ) {
						vty_out(vty,"%% Bad Parameter,vlan not exist.\n");
					}
					else if (VLAN_RETURN_CODE_L3_INTF == op_ret ) {
						vty_out(vty,"%% Bad Parameter,vlan is L3 interface .\n");
					}
					else if (TRUNK_RETURN_CODE_BADPARAM == op_ret) {
						vty_out(vty,"%% Error occurs in parse portNo or deviceNO.\n");
					}
					else if (VLAN_RETURN_CODE_TRUNK_EXISTS== op_ret) {
						vty_out(vty,"%% Bad Parameter,vlan Already allow in trunk %d.\n",trunkId);
					}
					else if (VLAN_RETURN_CODE_TRUNK_CONFLICT == op_ret) {
						vty_out(vty,"%% Bad Parameter,trunk %d already untagged member of other active vlan.\n",trunkId);
					}
					else if (TRUNK_RETURN_CODE_ALLOW_ERR == op_ret) {
						vty_out(vty,"%% Error occurs in trunk port add to allowed vlans.\n");
					}
					else if (TRUNK_RETURN_CODE_REFUSE_ERR == op_ret) {
						vty_out(vty,"%% Error occurs in trunk port delete from refused vlans.\n");
					}			
					else if (VLAN_RETURN_CODE_PORT_L3_INTF == op_ret) {
						/*never happen*/
						vty_out(vty,"%% Bad Parameter,there exists L3 interface port.\n");
					}
					else if (TRUNK_RETURN_CODE_ERR_HW == op_ret) {
						vty_out(vty,"%% Error occurs in config on HW.\n");	
					}
					else if (TRUNK_RETURN_CODE_ERR_GENERAL == op_ret) {
						vty_out(vty,"%% Error occurs in config on SW.\n"); /*such as add port to trunkNode struct.*/
					}
					else if (TRUNK_RETURN_CODE_MEMBERSHIP_CONFICT == op_ret) {
						vty_out(vty,"%% Error,this port is a member of other trunk.\n"); /*such as add port to trunkNode struct.*/
					}
					else if (TRUNK_RETURN_CODE_NO_MEMBER == op_ret) {
						vty_out(vty,"%% Bad parameter,there exists no member in trunk %d.\n",trunkId); /*such as add port to trunkNode struct.*/
					}
					else if (TRUNK_RETURN_CODE_ALLOW_VLAN == op_ret) {
						vty_out(vty,"%% Bad parameter,vlan already allow in trunk %d.\n",trunkId); /*such as add port to trunkNode struct.*/
					}
					else if (TRUNK_RETURN_CODE_NOTALLOW_VLAN == op_ret) {
						vty_out(vty,"%% Bad parameter,vlan not allow in trunk %d.\n",trunkId); /*such as add port to trunkNode struct.*/
					}
					else if (TRUNK_RETURN_CODE_VLAN_TAGMODE_ERR == op_ret) {
						vty_out(vty,"%% Bad parameter,trunk %d tagMode error in vlan.\n",trunkId); 
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
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_TRUNK_OBJPATH,	\
											NPD_DBUS_TRUNK_INTERFACE,	\
											NPD_DBUS_TRUNK_METHOD_ALLOW_REFUSE_VLAN_LIST);

		dbus_error_init(&err);

		dbus_message_append_args(	query,
							 		DBUS_TYPE_BYTE,&isAllow,
							 		DBUS_TYPE_BYTE,&isTag,
							 		DBUS_TYPE_UINT32,&count,
									DBUS_TYPE_UINT16,&vid,
							 		DBUS_TYPE_UINT16,&trunkId,
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
				if (TRUNK_RETURN_CODE_ERR_NONE == op_ret ) {
					/*vty_out(vty,"trunk %d %s vlan %d success.\n",trunkId,isAllow?"Allow":"Refuse",vid);*/
				}
				else if (VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret ) {
					vty_out(vty,"%% Bad Parameter,vlan not exist.\n");
				}
				else if (VLAN_RETURN_CODE_L3_INTF == op_ret ) {
					vty_out(vty,"%% Bad Parameter,vlan is L3 interface .\n");
				}
				else if (TRUNK_RETURN_CODE_BADPARAM == op_ret) {
					vty_out(vty,"%% Error occurs in parse portNo or deviceNO.\n");
				}
				else if (VLAN_RETURN_CODE_TRUNK_EXISTS== op_ret) {
					vty_out(vty,"%% Bad Parameter,vlan Already allow in trunk %d.\n",trunkId);
				}
				else if (VLAN_RETURN_CODE_TRUNK_CONFLICT == op_ret) {
					vty_out(vty,"%% Bad Parameter,trunk %d already untagged member of other active vlan.\n",trunkId);
				}
				else if (TRUNK_RETURN_CODE_ALLOW_ERR == op_ret) {
					vty_out(vty,"%% Error occurs in trunk port add to allowed vlans.\n");
				}
				else if (TRUNK_RETURN_CODE_REFUSE_ERR == op_ret) {
					vty_out(vty,"%% Error occurs in trunk port delete from refused vlans.\n");
				}			
				else if (VLAN_RETURN_CODE_PORT_L3_INTF == op_ret) {
					/*never happen*/
					vty_out(vty,"%% Bad Parameter,there exists L3 interface port.\n");
				}
				else if (TRUNK_RETURN_CODE_ERR_HW == op_ret) {
					vty_out(vty,"%% Error occurs in config on HW.\n");	
				}
				else if (TRUNK_RETURN_CODE_ERR_GENERAL == op_ret) {
					vty_out(vty,"%% Error occurs in config on SW.\n"); /*such as add port to trunkNode struct.*/
				}
				else if (TRUNK_RETURN_CODE_MEMBERSHIP_CONFICT == op_ret) {
					vty_out(vty,"%% Error,this port is a member of other trunk.\n"); /*such as add port to trunkNode struct.*/
				}
				else if (TRUNK_RETURN_CODE_NO_MEMBER == op_ret) {
					vty_out(vty,"%% Bad parameter,there exists no member in trunk %d.\n",trunkId); /*such as add port to trunkNode struct.*/
				}
				else if (TRUNK_RETURN_CODE_ALLOW_VLAN == op_ret) {
					vty_out(vty,"%% Bad parameter,vlan already allow in trunk %d.\n",trunkId); /*such as add port to trunkNode struct.*/
				}
				else if (TRUNK_RETURN_CODE_NOTALLOW_VLAN == op_ret) {
					vty_out(vty,"%% Bad parameter,vlan not allow in trunk %d.\n",trunkId); /*such as add port to trunkNode struct.*/
				}
				else if (TRUNK_RETURN_CODE_VLAN_TAGMODE_ERR == op_ret) {
					vty_out(vty,"%% Bad parameter,trunk %d tagMode error in vlan.\n",trunkId); 
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
* delete  trunk  on Both Sw & Hw. 
*Execute at trunk-config Node
* Params: 
*              trunk ID:	 <1-118>
*
* Usage: delete trunk <1-118>
****************************************/

DEFUN(delete_trunk_cmd_fun, 
	delete_trunk_cmd, 
	"delete trunk <1-64>",
	"Delete trunk on system\n"
	"Trunk entity\n"
	"Trunk ID range <1-110>\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned short 	trunkId =0;
	int ret = 0;
	unsigned int op_ret = 0;
	int i;
	unsigned int slot_id;
	int local_slot_id = 0;
    int slotNum = 0;
	

	ret = parse_trunk_no((char*)argv[0], &trunkId);

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
			if(NPD_FAIL == ret){
				vty_out(vty,"% Bad parameter,illegal trunk ID!\n");
				return CMD_WARNING;
			}
			else {
				query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
													NPD_DBUS_TRUNK_OBJPATH ,	\
													NPD_DBUS_TRUNK_INTERFACE ,	\
													NPD_DBUS_TRUNK_METHOD_DELETE_TRUNK_ENTRY );
			
				dbus_error_init(&err);

				dbus_message_append_args(query,
										DBUS_TYPE_UINT16,&trunkId,
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
							DBUS_TYPE_UINT32, &op_ret,
							DBUS_TYPE_INVALID)) 
			{
				if (ETHPORT_RETURN_CODE_NO_SUCH_TRUNK == op_ret) {
					vty_out(vty,"% Bad Parameter,trunk %d Invalid.\n",trunkId);
				}
				else if (TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret) {
        			vty_out(vty,"% Bad Parameter,trunk %d NOT exists.\n",trunkId);
				}
				else if (TRUNK_RETURN_CODE_BADPARAM == op_ret) {
					vty_out(vty,"%% Error occurs in Parse portNo or deviceNo.\n");
				}		
				else if (TRUNK_RETURN_CODE_ERR_HW == op_ret) {
					vty_out(vty,"%% Error occurs in Config on HW.\n");
				}
				else if(TRUNK_RETURN_CODE_ERR_NONE == op_ret) {
					/*vty_out(vty,"query reply op %d ,Delete trunk %d OK.\n",op_ret,trunkId);*/
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
	}
	else
	{
		if(NPD_FAIL == ret){
			vty_out(vty,"% Bad parameter,illegal trunk ID!\n");
			return CMD_WARNING;
		}
		else {
			query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
												NPD_DBUS_TRUNK_OBJPATH ,	\
												NPD_DBUS_TRUNK_INTERFACE ,	\
												NPD_DBUS_TRUNK_METHOD_DELETE_TRUNK_ENTRY );
			
			dbus_error_init(&err);

			dbus_message_append_args(query,
									DBUS_TYPE_UINT16,&trunkId,
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
			if (ETHPORT_RETURN_CODE_NO_SUCH_TRUNK == op_ret) {
				vty_out(vty,"% Bad Parameter,trunk %d Invalid.\n",trunkId);
			}
			else if (TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret) {
        		vty_out(vty,"% Bad Parameter,trunk %d NOT exists.\n",trunkId);
			}
			else if (TRUNK_RETURN_CODE_BADPARAM == op_ret) {
				vty_out(vty,"%% Error occurs in Parse portNo or deviceNo.\n");
			}		
			else if (TRUNK_RETURN_CODE_ERR_HW == op_ret) {
				vty_out(vty,"%% Error occurs in Config on HW.\n");
			}
			else if(TRUNK_RETURN_CODE_ERR_NONE == op_ret) {
				/*vty_out(vty,"query reply op %d ,Delete trunk %d OK.\n",op_ret,trunkId);*/
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
* Delete trunk entity
* Params:
*		trunk name:
* Usage:delete trunk <1-118>
**************************************/
DEFUN(delete_trunk_cmd_name_fun,
	delete_trunk_name_cmd,
	"delete trunk TRUNKNAME",
	"Delete System Interface\n"
	"Actives trunk via Specify trunkId\n"
	"Specify trunk name,begins with char or'_', and no more than 20 characters\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned short 	trunkId = 0;
	char*			trunkName = NULL;
	unsigned int	nameSize = 0;
	int ret = 0;
	unsigned int op_ret = 0;
	int i;
	int local_slot_id = 0;
    int slotNum = 0;
	
	/*get trunk name*/
	trunkName = (char*)malloc(ALIAS_NAME_SIZE);
	if(NULL == trunkName){
    	vty_out(vty,"% Memory resource error!\n");
		return CMD_WARNING;
	}	
	memset(trunkName,0,ALIAS_NAME_SIZE);

	ret = trunk_name_legal_check((char*)argv[0],strlen(argv[0]));

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
			if(ALIAS_NAME_LEN_ERROR == ret) {
				vty_out(vty,"% Bad parameter,trunk name too long!\n");
				return CMD_WARNING;
			}
			else if(ALIAS_NAME_HEAD_ERROR == ret) {
				vty_out(vty,"% Bad parameter,trunk name begins with a illegal char!\n");
				return CMD_WARNING;
			}
			else if(ALIAS_NAME_BODY_ERROR == ret) {
				vty_out(vty,"% Bad parameter,trunk name contains illegal char!\n");
				return CMD_WARNING;
			}
			else {
				nameSize = strlen(argv[0]);
				memcpy(trunkName, argv[0], nameSize);
			}
			if (NPD_SUCCESS != ret) {
				return CMD_WARNING;
			}
			else {

				query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
													NPD_DBUS_TRUNK_OBJPATH ,	\
													NPD_DBUS_TRUNK_INTERFACE ,	\
													NPD_DBUS_TRUNK_METHOD_DELETE_TRUNK_ENTRY_VIA_NAME );
		
				dbus_error_init(&err);

				dbus_message_append_args(query,
										DBUS_TYPE_STRING,&trunkName,
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
							DBUS_TYPE_UINT32, &op_ret,
							DBUS_TYPE_INVALID)) 
			{
				if (TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret) {
        			vty_out(vty,"%% Bad Parameter,trunk %s not exist.\n",trunkName);
				}
				else if (TRUNK_RETURN_CODE_ERR_GENERAL == op_ret) {
					vty_out(vty,"%% Error,operation on software fail.\n");
				}
				else if (TRUNK_RETURN_CODE_ERR_HW == op_ret) {
					vty_out(vty,"%% Error,operation on hardware fail.\n");
				}
				else if(TRUNK_RETURN_CODE_NAME_CONFLICT == op_ret){
					vty_out(vty, "%% Trunk name is conflict with dynamic trunk\n");
				}
				else if(TRUNK_RETURN_CODE_ERR_NONE == op_ret) {
					/*vty_out(vty,"Delete trunk OK.\n");*/
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
			free(trunkName);
		}
	}
	else
	{
		if(ALIAS_NAME_LEN_ERROR == ret) {
			vty_out(vty,"% Bad parameter,trunk name too long!\n");
			return CMD_WARNING;
		}
		else if(ALIAS_NAME_HEAD_ERROR == ret) {
			vty_out(vty,"% Bad parameter,trunk name begins with a illegal char!\n");
			return CMD_WARNING;
		}
		else if(ALIAS_NAME_BODY_ERROR == ret) {
			vty_out(vty,"% Bad parameter,trunk name contains illegal char!\n");
			return CMD_WARNING;
		}
		else {
			nameSize = strlen(argv[0]);
			memcpy(trunkName, argv[0], nameSize);
		}
		if (NPD_SUCCESS != ret) {
			return CMD_WARNING;
		}
		else {

			query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
												NPD_DBUS_TRUNK_OBJPATH ,	\
												NPD_DBUS_TRUNK_INTERFACE ,	\
												NPD_DBUS_TRUNK_METHOD_DELETE_TRUNK_ENTRY_VIA_NAME );
		
			dbus_error_init(&err);

			dbus_message_append_args(query,
									DBUS_TYPE_STRING,&trunkName,
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
			if (TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret) {
        		vty_out(vty,"%% Bad Parameter,trunk %s not exist.\n",trunkName);
			}
			else if (TRUNK_RETURN_CODE_ERR_GENERAL == op_ret) {
				vty_out(vty,"%% Error,operation on software fail.\n");
			}
			else if (TRUNK_RETURN_CODE_ERR_HW == op_ret) {
				vty_out(vty,"%% Error,operation on hardware fail.\n");
			}
			else if(TRUNK_RETURN_CODE_NAME_CONFLICT == op_ret){
				vty_out(vty, "%% Trunk name is conflict with dynamic trunk\n");
			}
			else if(TRUNK_RETURN_CODE_ERR_NONE == op_ret) {
				/*vty_out(vty,"Delete trunk OK.\n");*/
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
		free(trunkName);
	}
	return CMD_SUCCESS;
}

/***********************************
*show corrent trunk.
*Execute at trunk-config Node
*CMD : show trunk
*show members slot/port of corrent trunk 
***********************************/
DEFUN(show_current_trunk_cmd_fun,
	show_current_trunk_cmd,
	"show trunk ",
	"Show system information\n"
	"Trunk entity\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	
	DBusMessageIter	 iter_array;
	unsigned short	trunkId = 0;
	char*			trunkName = NULL;
	unsigned int	count = 0, ret = 0;
	unsigned int 	i = 0, row = 0, nodesave = 0;
	unsigned char  	masterFlag = 0;
	unsigned int   	mSlotNo = 0, mPortNo = 0;
	PORT_MEMBER_BMP 	mbrBmp_sp, disMbrBmp_sp;
	unsigned int 	product_id = 0, loadBalanc = 0; 
	unsigned int    trunkStat = 0;
	unsigned int    slot_id;
	
	unsigned int count_temp=0;
	unsigned int slot = 0,port = 0;
	int fd = -1;
	struct stat sb;

	dst_trunk_s *trunk_list =NULL;
	char* file_path = "/dbm/shm/trunk/shm_trunk";	

	memset(&mbrBmp_sp,0,sizeof(PORT_MEMBER_BMP));
	memset(&disMbrBmp_sp,0,sizeof(PORT_MEMBER_BMP));
	nodesave = (unsigned int)(vty->index);
	trunkId = nodesave;

	if(is_distributed == DISTRIBUTED_SYSTEM)
	{
		/* only read file */
	    fd = open(file_path, O_RDONLY);
    	if(fd < 0)
        {
            vty_out(vty,"Failed to open! %s\n", strerror(errno));
            return NULL;
        }
		fstat(fd,&sb);
		/* map not share */	
        trunk_list = (dst_trunk_s *)mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0 );
        if(MAP_FAILED == trunk_list)
        {
            vty_out(vty,"Failed to mmap for g_dst_trunk[]! %s\n", strerror(errno));
			close(fd);
            return NULL;
        }	

		vty_out(vty,"====================================================================\n");
		vty_out(vty,"%-8s %-15s %-12s %-35s\n","TRUNK ID","TRUNK NAME","LOAD BALANCE","PORT MEMBER LIST");
		vty_out(vty,"======== =============== ============ ==============================\n");	
		vty_out(vty,"%-5d(%s) ",trunkId,trunk_list[trunkId-1].tLinkState? "U":"D");			
		vty_out(vty,"%-15s ",trunk_list[trunkId-1].trunkName);
		vty_out(vty,"%-12s ",trunkLoadBalanc[trunk_list[trunkId-1].loadBanlcMode]);
		#if 0
		for(i=0;i<8;i++)
		{
			if(trunk_list[trunkId-1].portmap[i].master == 1)
			{
				masterFlag	= 1;
				mSlotNo = trunk_list[trunkId-1].portmap[i].slot;
				mPortNo = trunk_list[trunkId-1].portmap[i].port;
				break;
			}
			else
			{
				masterFlag = 0;
			}
		}
		if(masterFlag)
			vty_out(vty,"%d/%-9d ",mSlotNo,mPortNo);
		else
			vty_out(vty,"%-13s ","No masterPort");
		#endif
		if(0 != trunk_list[trunkId-1].portLog)
		{
			for(i=0;i<trunk_list[trunkId-1].portLog;i++) /*first show enable port*/
			{
				if(trunk_list[trunkId-1].portmap[i].enable)
				{
					slot = trunk_list[trunkId-1].portmap[i].slot;
					port = trunk_list[trunkId-1].portmap[i].port;
					if(count_temp && (0 == count_temp % 3))
					{
						vty_out(vty,"\n%-38s"," ");
					}
					vty_out(vty,"%s%d/%d(e)",(count_temp%3)?",":"",slot,port);
					count_temp++;
				}
			}
			for(i=0;i<trunk_list[trunkId-1].portLog;i++) /*then show disable port*/
			{
				if(!trunk_list[trunkId-1].portmap[i].enable)
				{
					slot = trunk_list[trunkId-1].portmap[i].slot;
					port = trunk_list[trunkId-1].portmap[i].port;
					if(count_temp && (0 == count_temp % 3))
					{
						vty_out(vty,"\n%-38s"," ");
					}
					vty_out(vty,"%s%d/%d(d)",(count_temp%3)?",":"",slot,port);
					count_temp++;
				}
			}
		}
		vty_out(vty,"\n");
		vty_out(vty,"====================================================================\n");
		/* munmap and close fd */
	    ret = munmap(trunk_list,sb.st_size);
	    if( ret != 0 )
	    {
	        vty_out(vty,"Failed to munmap for g_trunklist[]! %s\n", strerror(errno));			
	    }	
		ret = close(fd);
		if( ret != 0 )
	    {
	        vty_out(vty,"close shm_vlan failed \n" );   
	    }
	}
	else
	{
		
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_TRUNK_OBJPATH ,	\
											NPD_DBUS_TRUNK_INTERFACE ,	\
											NPD_DBUS_TRUNK_METHOD_SHOW_TRUNK_PORT_MEMBERS_V1 );
		

		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT16,&trunkId,
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
	     				DBUS_TYPE_STRING, &trunkName,
						DBUS_TYPE_UINT32, &product_id,
						DBUS_TYPE_UINT32, &slot_id,
						DBUS_TYPE_BYTE,	  &masterFlag,
	     				DBUS_TYPE_UINT32, &mSlotNo,
	     				DBUS_TYPE_UINT32, &mPortNo,
						DBUS_TYPE_UINT32, &mbrBmp_sp.portMbr[0],
						DBUS_TYPE_UINT32, &mbrBmp_sp.portMbr[1],
						DBUS_TYPE_UINT32, &disMbrBmp_sp.portMbr[0],
						DBUS_TYPE_UINT32, &disMbrBmp_sp.portMbr[1],
						DBUS_TYPE_UINT32, &loadBalanc,
						DBUS_TYPE_UINT32, &trunkStat,
						DBUS_TYPE_INVALID)) 
		{
			/*vty_out(vty,"trunkName %s, portmbrBmp0 %#0X,loadBalanc %d .ret %d.\n",trunkName,mbrBmp_sp,loadBalanc,ret);*/
			if(TRUNK_RETURN_CODE_ERR_NONE == ret) {
				vty_out(vty,"================================================================================\n");
				vty_out(vty,"%-8s %-20s %-12s %-11s %-30s\n","TRUNK ID","TRUNK NAME","LOAD BALANCE","MASTER PORT","PORT MEMBER LIST");
				vty_out(vty,"======== ==================== ============ =========== =========================\n");
				
				vty_out(vty,"%-5d(%s) ",trunkId,trunkStat ? "U":"D");
				vty_out(vty,"%-20s ",trunkName);
				vty_out(vty,"%-12s ",trunkLoadBalanc[loadBalanc]);
				if(0 != masterFlag)
					vty_out(vty,"%d/%-9d ",mSlotNo,mPortNo);
				else 
					vty_out(vty,"%-13s ","No masterPort");
				show_trunk_member_slot_port(vty,product_id,mbrBmp_sp,disMbrBmp_sp);

				vty_out(vty,"\n");
				vty_out(vty,"================================================================================\n");

			}
			else if(ETHPORT_RETURN_CODE_NO_SUCH_TRUNK == ret) {
				vty_out(vty,"% Bad parameter,illegal trunk Id.\n");
			}
			else if(TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == ret) {
				vty_out(vty,"% Bad parameter,trunk %d not exist.\n",trunkId);
			}
			else if(TRUNK_RETURN_CODE_ERR_GENERAL == ret) {
				vty_out(vty,"%% Error,operation on trunk portlist fail.\n");
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
	}
	return CMD_SUCCESS;
}
/**************************************
* display special trunk  
* Params: 
*              trunk ID:	 <1-118>
*
* Usage: show trunk <1-118>
****************************************/

DEFUN(show_trunk_one_cmd_fun, 
	show_trunk_one_cmd, 
	"show trunk <1-64>",
	"Show trunk on system\n"
	"Trunk entity\n"
	"Trunk ID range <1-64>\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	
	unsigned short	trunkId = 0;
	char*			trunkName = NULL;
	unsigned int	ret = 0;
	unsigned char   masterFlag = 0;
	unsigned int    mSlotNo = 0, mPortNo = 0;
	unsigned int 	product_id = 0,loadBalanc = 0;
	unsigned int 	trunkStat = 0;
	unsigned int    slot_id;
	int i;
	unsigned int count_temp=0;
	unsigned int slot = 0,port = 0;
	int fd = -1;
	struct stat sb;
	dst_trunk_s *trunk_list =NULL;
	char* file_path = "/dbm/shm/trunk/shm_trunk";	
	PORT_MEMBER_BMP mbrBmp_sp, disMbrBmp_sp;
	memset(&mbrBmp_sp,0,sizeof(PORT_MEMBER_BMP));
	memset(&disMbrBmp_sp,0,sizeof(PORT_MEMBER_BMP));
	ret = parse_trunk_no((char*)argv[0],&trunkId);
	
	if (NPD_FAIL == ret) {
		vty_out(vty,"Illegal trunkId .\n");
		return CMD_SUCCESS;
	}

	if(is_distributed == DISTRIBUTED_SYSTEM)
	{
		/* only read file */
	    fd = open(file_path, O_RDONLY);
    	if(fd < 0)
        {
            vty_out(vty,"Failed to open! %s\n", strerror(errno));
            return NULL;
        }
		fstat(fd,&sb);
		/* map not share */	
        trunk_list = (dst_trunk_s *)mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0 );
        if(MAP_FAILED == trunk_list)
        {
            vty_out(vty,"Failed to mmap for g_dst_trunk[]! %s\n", strerror(errno));
			close(fd);
            return NULL;
        }	
		if(trunk_list[trunkId-1].trunkId != trunkId)
		{
			vty_out(vty,"%% TRUNK does not exists,please check !\n");
			return CMD_SUCCESS;
		}
		
		vty_out(vty,"====================================================================\n");
		vty_out(vty,"%-8s %-15s %-12s %-35s\n","TRUNK ID","TRUNK NAME","LOAD BALANCE","PORT MEMBER LIST");
		vty_out(vty,"======== =============== ============ ==============================\n");	
		vty_out(vty,"%-5d(%s) ",trunkId,trunk_list[trunkId-1].tLinkState? "U":"D");			
		vty_out(vty,"%-15s ",trunk_list[trunkId-1].trunkName);
		vty_out(vty,"%-12s ",trunkLoadBalanc[trunk_list[trunkId-1].loadBanlcMode]);
		#if 0
		for(i=0;i<8;i++)
		{
			if(trunk_list[trunkId-1].portmap[i].master == 1)
			{
				masterFlag	= 1;
				mSlotNo = trunk_list[trunkId-1].portmap[i].slot;
				mPortNo = trunk_list[trunkId-1].portmap[i].port;
				break;
			}
			else
			{
				masterFlag = 0;
			}
		}
		if(masterFlag)
			vty_out(vty,"%d/%-9d ",mSlotNo,mPortNo);
		else
			vty_out(vty,"%-13s ","No masterPort");
		#endif

		if(0 != trunk_list[trunkId-1].portLog)
		{
			for(i=0;i<trunk_list[trunkId-1].portLog;i++) /*first show enable port*/
			{
				if(trunk_list[trunkId-1].portmap[i].enable)
				{
					slot = trunk_list[trunkId-1].portmap[i].slot;
					port = trunk_list[trunkId-1].portmap[i].port;
					if(count_temp && (0 == count_temp % 3))
					{
						vty_out(vty,"\n%-38s"," ");
					}
					vty_out(vty,"%s%d/%d(e)",(count_temp%3)?",":"",slot,port);
					count_temp++;
				}
			}
			for(i=0;i<trunk_list[trunkId-1].portLog;i++) /*then show disable port*/
			{
				if(!trunk_list[trunkId-1].portmap[i].enable)
				{
					slot = trunk_list[trunkId-1].portmap[i].slot;
					port = trunk_list[trunkId-1].portmap[i].port;
					if(count_temp && (0 == count_temp % 3))
					{
						vty_out(vty,"\n%-38s"," ");
					}
					vty_out(vty,"%s%d/%d(d)",(count_temp%3)?",":"",slot,port);
					count_temp++;
				}
			}
		}
		vty_out(vty,"\n");
		vty_out(vty,"====================================================================\n");
		/* munmap and close fd */
	    ret = munmap(trunk_list,sb.st_size);
	    if( ret != 0 )
	    {
	        vty_out(vty,"Failed to munmap for g_trunklist[]! %s\n", strerror(errno));			
	    }	
		ret = close(fd);
		if( ret != 0 )
	    {
	        vty_out(vty,"close shm_vlan failed \n" );   
	    }
	}
	else
	{
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_TRUNK_OBJPATH ,	\
											NPD_DBUS_TRUNK_INTERFACE ,	\
											NPD_DBUS_TRUNK_METHOD_SHOW_TRUNK_PORT_MEMBERS_V1 );
	

		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT16,&trunkId,
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
     					DBUS_TYPE_STRING, &trunkName,
						DBUS_TYPE_UINT32, &product_id,
						DBUS_TYPE_UINT32, &slot_id,
						DBUS_TYPE_BYTE,	  &masterFlag,
     					DBUS_TYPE_UINT32, &mSlotNo,
     					DBUS_TYPE_UINT32, &mPortNo,
						DBUS_TYPE_UINT32, &mbrBmp_sp.portMbr[0],
						DBUS_TYPE_UINT32, &mbrBmp_sp.portMbr[1],
						DBUS_TYPE_UINT32, &disMbrBmp_sp.portMbr[0],
						DBUS_TYPE_UINT32, &disMbrBmp_sp.portMbr[1],
						DBUS_TYPE_UINT32, &loadBalanc,
						DBUS_TYPE_UINT32, &trunkStat,
						DBUS_TYPE_INVALID)) {
			if(TRUNK_RETURN_CODE_ERR_NONE == ret) {
				vty_out(vty,"================================================================================\n");
				vty_out(vty,"%-8s %-20s %-12s %-11s %-30s\n","TRUNK ID","TRUNK NAME","LOAD BALANCE","MASTER PORT","PORT MEMBER LIST");
				vty_out(vty,"======== ==================== ============ =========== =========================\n");
			
				vty_out(vty,"%-5d(%s) ",trunkId,trunkStat ? "U":"D");			
				vty_out(vty,"%-20s ",trunkName);
				vty_out(vty,"%-12s ",trunkLoadBalanc[loadBalanc]);
				if(0 != masterFlag)
					vty_out(vty,"%d/%-9d ",mSlotNo,mPortNo);
				else 
					vty_out(vty,"%-13s ","No masterPort");
				show_trunk_member_slot_port(vty,product_id,mbrBmp_sp,disMbrBmp_sp);
				vty_out(vty,"\n");
				vty_out(vty,"================================================================================\n");

			}
			else if(ETHPORT_RETURN_CODE_NO_SUCH_TRUNK == ret) {
				vty_out(vty,"% Bad parameter,illegal trunk Id.\n");
			}
			else if(TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == ret) {
				vty_out(vty,"% Bad parameter,trunk %d not exist.\n",trunkId);
			}
			else if(TRUNK_RETURN_CODE_ERR_GENERAL == ret) {
				vty_out(vty,"%% Error,operation on trunk portlist fail.\n");
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
	}
	return CMD_SUCCESS;
}


DEFUN(show_trunk_by_name_cmd_fun, 
	show_trunk_by_name_cmd, 
	"show trunk TRUNKNAME",
	"Show trunk on system\n"
	"Trunk entity\n"
	"Trunk name,begins with char or'_',no more than 20 chars\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	
	unsigned short	trunkId = 0;
	char*			trunkName = NULL;
	unsigned int	ret = 0;
	unsigned int   mSlotNo = 0, mPortNo = 0;
	unsigned char masterFlag = 0;
	unsigned int product_id = 0, loadBalanc = 0;
	unsigned int trunkStat = 0;
    PORT_MEMBER_BMP mbrBmp_sp, disMbrBmp_sp;
	unsigned int    slot_id;
	unsigned int count_temp=0;
	unsigned int slot = 0,port = 0;
   	unsigned int  tmpVal[2];
	int local_slot_id = 0;
	int slotNum = 0,i=0,k=0;
	
	memset(&mbrBmp_sp,0,sizeof(PORT_MEMBER_BMP));
	memset(&disMbrBmp_sp,0,sizeof(PORT_MEMBER_BMP));
	trunkName = (char*)malloc(ALIAS_NAME_SIZE);
	if(NULL == trunkName){
    	vty_out(vty,"% Memory resource error!\n");
		return CMD_WARNING;
	}	
	memset(trunkName,0,ALIAS_NAME_SIZE);
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
			if(i==1)
			{
				vty_out(vty,"================================================================================\n");
				vty_out(vty,"%-8s %-15s %-12s %-35s\n","TRUNK ID","TRUNK NAME","LOAD BALANCE","MASTER PORT","PORT MEMBER LIST");
				vty_out(vty,"======== =============== ============ =========== ==============================\n");			
			}			
			ret = trunk_name_legal_check((char*)argv[0],strlen(argv[0]));
			if(ALIAS_NAME_LEN_ERROR == ret) {
				vty_out(vty,"% Bad parameter,trunk name too long!\n");
				return CMD_WARNING;
			}
			else if(ALIAS_NAME_HEAD_ERROR == ret) {
				vty_out(vty,"% Bad parameter,trunk name begins of a illegal char!\n");
				return CMD_WARNING;
			}
			else if(ALIAS_NAME_BODY_ERROR == ret) {
				vty_out(vty,"% Bad parameter,trunk name contains illegal char!\n");
				return CMD_WARNING;
			}
			else {
				memcpy(trunkName, argv[0], strlen(argv[0]));
				query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
													NPD_DBUS_TRUNK_OBJPATH ,	\
													NPD_DBUS_TRUNK_INTERFACE ,	\
													NPD_DBUS_TRUNK_METHOD_SHOW_TRUNK_BY_NAME_V1 );
				
			}
			dbus_error_init(&err);
			dbus_message_append_args(query,
									 DBUS_TYPE_STRING,&trunkName,
									 DBUS_TYPE_INVALID);

			if(NULL == dbus_connection_dcli[i]->dcli_dbus_connection) 				
			{
    			if(i == local_slot_id)
				{
                    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
				#if 0
				   	vty_out(vty,"Can not connect to slot:%d \n",i);	
				#endif/*for print*/
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
					printf("%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_SUCCESS;
			}
			if (dbus_message_get_args ( reply, &err,
							DBUS_TYPE_UINT32, &ret,
		     				DBUS_TYPE_UINT16, &trunkId,
							DBUS_TYPE_UINT32, &product_id,
							DBUS_TYPE_UINT32, &slot_id,
							DBUS_TYPE_BYTE,	  &masterFlag,
		     				DBUS_TYPE_UINT32, &mSlotNo,
		     				DBUS_TYPE_UINT32, &mPortNo,
							DBUS_TYPE_UINT32, &mbrBmp_sp.portMbr[0],
							DBUS_TYPE_UINT32, &mbrBmp_sp.portMbr[1],
							DBUS_TYPE_UINT32, &disMbrBmp_sp.portMbr[0],
							DBUS_TYPE_UINT32, &disMbrBmp_sp.portMbr[1],
							DBUS_TYPE_UINT32, &loadBalanc,
							DBUS_TYPE_UINT32, &trunkStat,
							DBUS_TYPE_INVALID)) {
				if(TRUNK_RETURN_CODE_ERR_NONE == ret) {
					vty_out(vty,"Trunk on slot %d \n",i);	
			
					vty_out(vty,"%-5d(%s) ",trunkId,trunkStat ? "U":"D");			
					vty_out(vty,"%-15s ",trunkName);
					vty_out(vty,"%-12s ",trunkLoadBalanc[loadBalanc]);
					if(0 != masterFlag)
						vty_out(vty,"%d/%-9d ",mSlotNo,mPortNo);
					else 
						vty_out(vty,"%-13s ","No masterPort");
					#if 0
					show_trunk_member_slot_port(vty,product_id,mbrBmp_sp,disMbrBmp_sp);
					#endif
					if(0 != mbrBmp_sp.portMbr[0] || 0 !=mbrBmp_sp.portMbr[1] || 0!=disMbrBmp_sp.portMbr[0] || 0!=disMbrBmp_sp.portMbr[1]){
						memset(&tmpVal,0,sizeof(tmpVal));
						slot = slot_id;
						for (k=0;k<64;k++){
							port = k;

							tmpVal[k/32] = (1<<(k%32));
							if((mbrBmp_sp.portMbr[k/32]) & tmpVal[k/32]) {				
								if(count_temp && (0 == count_temp % 3)) {
								vty_out(vty,"\n%-50s"," ");
								}
								vty_out(vty,"%s%d/%d(e)",(count_temp%3) ? ",":"",slot,port);
								count_temp++;
							}
						}

						port = 0;
						memset(&tmpVal,0,sizeof(tmpVal));
						for (k=0;k<64;k++){
							port = k;

							tmpVal[k/32] = (1<<(k%32));
							if((disMbrBmp_sp.portMbr[k/32]) & tmpVal[k/32]) {				
								if(count_temp && (0 == count_temp % 3)) {
									vty_out(vty,"\n%-50s"," ");
								}
								vty_out(vty,"%s%d/%d(d)",(count_temp%3) ? ",":"",slot,port);
								count_temp++;
							}
						}
				}
					vty_out(vty,"\n");
					vty_out(vty,"================================================================================\n");
				}
				else if(ETHPORT_RETURN_CODE_NO_SUCH_TRUNK == ret) {
					vty_out(vty,"% Bad parameter,illegal trunk Id.\n");
				}
				else if(TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == ret) {
					vty_out(vty,"% Bad parameter,trunk %s not exist.\n",trunkName);
				}
				else if(TRUNK_RETURN_CODE_ERR_GENERAL == ret) {
					vty_out(vty,"%% Error,operation on trunk portlist fail.\n");
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
		}
		free(trunkName);
	}
	else
	{
			ret = trunk_name_legal_check((char*)argv[0],strlen(argv[0]));
		if(ALIAS_NAME_LEN_ERROR == ret) {
			vty_out(vty,"% Bad parameter,trunk name too long!\n");
			return CMD_WARNING;
		}
		else if(ALIAS_NAME_HEAD_ERROR == ret) {
			vty_out(vty,"% Bad parameter,trunk name begins of a illegal char!\n");
			return CMD_WARNING;
		}
		else if(ALIAS_NAME_BODY_ERROR == ret) {
			vty_out(vty,"% Bad parameter,trunk name contains illegal char!\n");
			return CMD_WARNING;
		}
		else {
			memcpy(trunkName, argv[0], strlen(argv[0]));
			query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
												NPD_DBUS_TRUNK_OBJPATH ,	\
												NPD_DBUS_TRUNK_INTERFACE ,	\
												NPD_DBUS_TRUNK_METHOD_SHOW_TRUNK_BY_NAME_V1 );
			
		}
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_STRING,&trunkName,
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
	     				DBUS_TYPE_UINT16, &trunkId,
						DBUS_TYPE_UINT32, &product_id,
						DBUS_TYPE_UINT32, &slot_id,
						DBUS_TYPE_BYTE,	  &masterFlag,
	     				DBUS_TYPE_UINT32, &mSlotNo,
	     				DBUS_TYPE_UINT32, &mPortNo,
						DBUS_TYPE_UINT32, &mbrBmp_sp.portMbr[0],
						DBUS_TYPE_UINT32, &mbrBmp_sp.portMbr[1],
						DBUS_TYPE_UINT32, &disMbrBmp_sp.portMbr[0],
						DBUS_TYPE_UINT32, &disMbrBmp_sp.portMbr[1],
						DBUS_TYPE_UINT32, &loadBalanc,
						DBUS_TYPE_UINT32, &trunkStat,
						DBUS_TYPE_INVALID)) {
			if(TRUNK_RETURN_CODE_ERR_NONE == ret) {
				vty_out(vty,"================================================================================\n");
				vty_out(vty,"%-8s %-20s %-12s %-11s %-30s\n","TRUNK ID","TRUNK NAME","LOAD BALANCE","MASTER PORT","PORT MEMBER LIST");
				vty_out(vty,"======== ==================== ============ =========== =========================\n");

				vty_out(vty,"%-5d(%s) ",trunkId,trunkStat ? "U":"D");			
				vty_out(vty,"%-20s ",trunkName);
				vty_out(vty,"%-12s ",trunkLoadBalanc[loadBalanc]);
				if(0 != masterFlag)
					vty_out(vty,"%d/%-9d ",mSlotNo,mPortNo);
				else 
					vty_out(vty,"%-13s ","No masterPort");
				show_trunk_member_slot_port(vty,product_id,mbrBmp_sp,disMbrBmp_sp);
				vty_out(vty,"\n");
				vty_out(vty,"================================================================================\n");

			}
			else if(ETHPORT_RETURN_CODE_NO_SUCH_TRUNK == ret) {
				vty_out(vty,"% Bad parameter,illegal trunk Id.\n");
			}
			else if(TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == ret) {
				vty_out(vty,"% Bad parameter,trunk %s not exist.\n",trunkName);
			}
			else if(TRUNK_RETURN_CODE_ERR_GENERAL == ret) {
				vty_out(vty,"%% Error,operation on trunk portlist fail.\n");
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
		free(trunkName);
	}
	return CMD_SUCCESS;
}
/**************************************
* display all the trunk  
* Params: 
*              NULL
*
* Usage: show trunk list
****************************************/

DEFUN(show_trunk_list_cmd_fun, 
	show_trunk_list_cmd, 
	"show trunk list",
	"Show trunk list on system\n"
	"Trunk entity\n"
	"Trunk list\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int     trunk_Cont = 0;
	unsigned short   trunkId = 0;
	char*			 trunkName = NULL;
	unsigned char 	 masterFlag = 0;
	unsigned int     mSlotNo = 0, mPortNo = 0, loadBalanc = 0,slot_id;
	unsigned int	 product_id = 0; 
	unsigned int 	 j = 0, ret = 0,i=0;
	unsigned int     trunkStat = 0;
	unsigned int count_temp=0;
	unsigned int slot = 0,port = 0;
	int trunk_exists = 0;
	int fd = -1;
	struct stat sb;
	dst_trunk_s *trunk_list =NULL;
	char* file_path = "/dbm/shm/trunk/shm_trunk";	
    PORT_MEMBER_BMP mbrBmp_sp, disMbrBmp_sp;
	memset(&mbrBmp_sp,0,sizeof(PORT_MEMBER_BMP));
	memset(&disMbrBmp_sp,0,sizeof(PORT_MEMBER_BMP));
	trunkName = (char*)malloc(ALIAS_NAME_SIZE);
	memset(trunkName,0,ALIAS_NAME_SIZE);
	
	if(is_distributed == DISTRIBUTED_SYSTEM)
	{

		fd = open(file_path, O_RDONLY);
    	if(fd < 0)
        {
            vty_out(vty,"Failed to open! %s\n", strerror(errno));
            return NULL;
        }
		fstat(fd,&sb);
		/* map not share */	
        trunk_list = (dst_trunk_s *)mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0 );
        if(MAP_FAILED == trunk_list)
        {
            vty_out(vty,"Failed to mmap for g_dst_trunk[]! %s\n", strerror(errno));
			close(fd);
            return NULL;
        }	
		vty_out(vty,"====================================================================\n");
		vty_out(vty,"%-8s %-15s %-12s %-35s\n","TRUNK ID","TRUNK NAME","LOAD BALANCE","PORT MEMBER LIST");
		vty_out(vty,"======== =============== ============ ==============================\n");	
		for(i=0;i<110;i++)
		{
			if(!trunk_list[i].trunkId)
			{
				continue;
			}
			trunk_exists++;
			vty_out(vty,"%-5d(%s) ",trunk_list[i].trunkId,trunk_list[i].tLinkState? "U":"D");			
			vty_out(vty,"%-15s ",trunk_list[i].trunkName);
			vty_out(vty,"%-12s ",trunkLoadBalanc[trunk_list[i].loadBanlcMode]);
			#if 0
			for(j=0;j<8;j++)
			{
				if(trunk_list[i].portmap[j].master == 1)
				{
					masterFlag	= 1;
					mSlotNo = trunk_list[i].portmap[j].slot;
					mPortNo = trunk_list[i].portmap[j].port;
					break;
				}
				else
				{
					masterFlag = 0;
				}
			}
			if(masterFlag)
				vty_out(vty,"%d/%-9d ",mSlotNo,mPortNo);
			else
				vty_out(vty,"%-13s ","No masterPort");
			#endif

			if(0 != trunk_list[i].portLog)
			{
				for(j=0;j<trunk_list[i].portLog;j++) /*first show enable port*/
				{
					if(trunk_list[i].portmap[j].enable)
					{
						slot = trunk_list[i].portmap[j].slot;
						port = trunk_list[i].portmap[j].port;
						if(count_temp && (0 == count_temp % 3))
						{
							vty_out(vty,"\n%-38s"," ");
						}
						vty_out(vty,"%s%d/%d(e)",(count_temp%3)?",":"",slot,port);
						count_temp++;
					}
				}
				for(j=0;j<trunk_list[i].portLog;j++) /*then show disable port*/
				{
					if(!trunk_list[i].portmap[j].enable)
					{
						slot = trunk_list[i].portmap[j].slot;
						port = trunk_list[i].portmap[j].port;
						if(count_temp && (0 == count_temp % 3))
						{
							vty_out(vty,"\n%-38s"," ");
						}
						vty_out(vty,"%s%d/%d(d)",(count_temp%3)?",":"",slot,port);
						count_temp++;
					}
				}
			}
			count_temp = 0;
			vty_out(vty,"\n");
		}
		if(!trunk_exists)
			vty_out(vty,"%% There is no vaild trunk exist !\n");
		vty_out(vty,"====================================================================\n");
		/* munmap and close fd */
	    ret = munmap(trunk_list,sb.st_size);
	    if( ret != 0 )
	    {
	        vty_out(vty,"Failed to munmap for g_trunklist[]! %s\n", strerror(errno));			
	    }	
		ret = close(fd);
		if( ret != 0 )
	    {
	        vty_out(vty,"close shm_vlan failed \n" );   
	    }
	}
	else
	{

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_TRUNK_OBJPATH ,	\
										NPD_DBUS_TRUNK_INTERFACE ,	\
										NPD_DBUS_TRUNK_METHOD_SHOW_TRUNKLIST_PORT_MEMBERS_V1 );
	

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
	if(TRUNK_RETURN_CODE_ERR_NONE == ret){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&trunk_Cont);

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&product_id);


		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&slot_id);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
			vty_out(vty,"================================================================================\n");
			vty_out(vty,"%-8s %-20s %-12s %-11s %-30s\n","TRUNK ID","TRUNK NAME","LOAD BALANCE","MASTER PORT","PORT MEMBER LIST");
			vty_out(vty,"======== ==================== ============ =========== =========================\n");
			
		for (j = 0; j < trunk_Cont; j++) {
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&trunkId);
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&trunkName);
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&masterFlag);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&mSlotNo);
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&mPortNo);
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&mbrBmp_sp.portMbr[0]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&mbrBmp_sp.portMbr[1]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&disMbrBmp_sp.portMbr[0]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&disMbrBmp_sp.portMbr[1]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&loadBalanc);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&trunkStat);

			dbus_message_iter_next(&iter_array);  

			vty_out(vty,"%-5d(%s) ",trunkId,trunkStat ? "U":"D");		
			vty_out(vty,"%-20s ",trunkName);
			vty_out(vty,"%-12s ",trunkLoadBalanc[loadBalanc]);
			if(0 != masterFlag)
				vty_out(vty,"%d/%-9d ",mSlotNo,mPortNo);
			else 
				vty_out(vty,"%-13s ","No masterPort");
			show_trunk_member_slot_port(vty,product_id,mbrBmp_sp,disMbrBmp_sp);
			vty_out(vty,"\n");
		}
		vty_out(vty,"================================================================================\n");
	}
	else if(TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == ret) {
		vty_out(vty,"%% There is no vaild trunk exist.\n");
	}
	else if(TRUNK_RETURN_CODE_ERR_HW == ret) {
		vty_out(vty,"%% Error occurs in read trunk entry in hardware.\n");				
	}
	else if(TRUNK_RETURN_CODE_ERR_GENERAL == ret) {
		vty_out(vty,"%% Error,operation on trunk portlist fail.\n");
	}
	else if(COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
		vty_out(vty,"%% Product not support this function!\n");
	}
	dbus_message_unref(reply);
	}
	return CMD_SUCCESS;
	

}
/***********************************
*show trunk allows vlan list.
*Trunk-config Node
*CMD : show trunk vlan_list
*show vlan allowed in trunk 
***********************************/
DEFUN(show_trunk_vlanlist_cmd_fun,
	show_trunk_vlanlist_cmd,
	"show trunk vlan_list",
	"Show system information\n"
	"Trunk entity in system\n"
	"Vlan aggregation in trunk\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	
	DBusMessageIter	 iter_array;
	unsigned short	trunkId = 0,vlanId = 0;/*vlanId[NPD_MAX_VLAN_ID] = {0};*/
	char			*trunkName = NULL,*vlanName = NULL;
	unsigned int	i = 0,vlan_Cont = 0,ret = 0,nodesave = 0,j = 0;
	unsigned char	tagMode = 0;
	nodesave = (unsigned int)(vty->index);
	trunkId = nodesave;

	int local_slot_id = 0;
    int slotNum = 0;
	
	if(is_distributed == DISTRIBUTED_SYSTEM){

		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		for(j=1; j <= slotNum; j++)
		{
           	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_TRUNK_OBJPATH ,\
										NPD_DBUS_TRUNK_INTERFACE ,	\
										NPD_DBUS_TRUNK_METHOD_SHOW_TRUNK_VLAN_AGGREGATION );
			

			dbus_error_init(&err);
			dbus_message_append_args(query,
									 DBUS_TYPE_UINT16,&trunkId,
									 DBUS_TYPE_INVALID);
			
            if(NULL == dbus_connection_dcli[j]->dcli_dbus_connection) 				
			{
    			if(j == local_slot_id)
				{
                    reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				}
				else 
				{	
				   	vty_out(vty,"Can not connect to slot:%d \n",j);	
					continue;
				}
            }
			else
			{
                reply = dbus_connection_send_with_reply_and_block (dbus_connection_dcli[j]->dcli_dbus_connection,query,-1, &err);				
			}
			
			dbus_message_unref(query);
			if (NULL == reply) {
				vty_out(vty,"Dbus reply==NULL, Please check npd on slot: %d\n",j);
				if (dbus_error_is_set(&err)) {
					printf("%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_SUCCESS;
			}
	
			dbus_message_iter_init(reply,&iter);
			dbus_message_iter_get_basic(&iter,&ret);

			if(TRUNK_RETURN_CODE_ERR_NONE == ret){
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&trunkName);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&vlan_Cont);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);
			/*vty_out(vty,"trunk %d allows vlan count %d.\n",trunkId,vlan_Cont);*/
			vty_out(vty,"=======================================================================\n");
			vty_out(vty,"%-8s  %-20s  %-10s  %-7s  %-20s\n","TRUNK ID","TRUNK NAME","TAG-MODE","VLAN ID","VLAN NAME");
			vty_out(vty,"========  ====================  ==========  =======  ==================\n");
			
			vty_out(vty,"%-8d  ",trunkId);
			vty_out(vty,"%-20s  ",trunkName);
			for(i=0;i<vlan_Cont;i++){
				DBusMessageIter iter_struct;
				dbus_message_iter_recurse(&iter_array,&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&vlanId);

				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&vlanName);
				
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&tagMode);

				dbus_message_iter_next(&iter_array);
				if(0 == i){
					vty_out(vty,"%-10s  ",(1==tagMode)?"tag":"untag");
					vty_out(vty,"%-7d  ",vlanId);
					vty_out(vty,"%-20s  %s",vlanName,(i<vlan_Cont-1)?"\n":"");
				}
				else {
					vty_out(vty,"%-32s%-10s  ","",(1==tagMode)?"tag":"untag");
					vty_out(vty,"%-7d  ",vlanId);
					vty_out(vty,"%-20s  %s",vlanName,(i<vlan_Cont-1)?"\n":"");
				}
						
		}
			vty_out(vty,"\n");
			vty_out(vty,"=======================================================================\n");
		}
		else if(ETHPORT_RETURN_CODE_NO_SUCH_TRUNK == ret) {
			vty_out(vty,"%% Bad parameter,illegal trunk Id.\n");
		}
		else if(TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == ret) {
			vty_out(vty,"%% Bad parameter,trunk %d not exist.\n",trunkId);
		}
		else if(TRUNK_RETURN_CODE_ERR_HW == ret) {
			vty_out(vty,"%% Error,operation on hardware fail.\n");
		}
		else if(TRUNK_RETURN_CODE_GET_ALLOWVLAN_ERR == ret) {
			vty_out(vty,"%% Error,operation on getting trunk allow vlanlist fail.\n");
		}
		dbus_message_unref(reply);
		}
        /*free(vlanName);*/
        return CMD_SUCCESS;		
		
	}
	else
	{


	
		vty_out(vty,"trunkId = %d\n",trunkId);
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_TRUNK_OBJPATH ,\
											NPD_DBUS_TRUNK_INTERFACE ,	\
											NPD_DBUS_TRUNK_METHOD_SHOW_TRUNK_VLAN_AGGREGATION );
	

		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT16,&trunkId,
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
	
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);

		if(TRUNK_RETURN_CODE_ERR_NONE == ret){
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&trunkName);
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&vlan_Cont);
			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);
			/*vty_out(vty,"trunk %d allows vlan count %d.\n",trunkId,vlan_Cont);*/
			vty_out(vty,"=======================================================================\n");
			vty_out(vty,"%-8s  %-20s  %-10s  %-7s  %-20s\n","TRUNK ID","TRUNK NAME","TAG-MODE","VLAN ID","VLAN NAME");
			vty_out(vty,"========  ====================  ==========  =======  ==================\n");
			
			vty_out(vty,"%-8d  ",trunkId);
			vty_out(vty,"%-20s  ",trunkName);
			for(i=0;i<vlan_Cont;i++){
				DBusMessageIter iter_struct;
				dbus_message_iter_recurse(&iter_array,&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&vlanId);

				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&vlanName);
				
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&tagMode);

				dbus_message_iter_next(&iter_array);
				if(0 == i){
					vty_out(vty,"%-10s  ",(1==tagMode)?"tag":"untag");
					vty_out(vty,"%-7d  ",vlanId);
					vty_out(vty,"%-20s  %s",vlanName,(i<vlan_Cont-1)?"\n":"");
				}
				else {
					vty_out(vty,"%-32s%-10s  ","",(1==tagMode)?"tag":"untag");
					vty_out(vty,"%-7d  ",vlanId);
					vty_out(vty,"%-20s  %s",vlanName,(i<vlan_Cont-1)?"\n":"");
				}
						
			}
			vty_out(vty,"\n");
			vty_out(vty,"=======================================================================\n");
		}
		else if(ETHPORT_RETURN_CODE_NO_SUCH_TRUNK == ret) {
			vty_out(vty,"%% Bad parameter,illegal trunk Id.\n");
		}
		else if(TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == ret) {
			vty_out(vty,"%% Bad parameter,trunk %d not exist.\n",trunkId);
		}
		else if(TRUNK_RETURN_CODE_ERR_HW == ret) {
			vty_out(vty,"%% Error,operation on hardware fail.\n");
		}
		else if(TRUNK_RETURN_CODE_GET_ALLOWVLAN_ERR == ret) {
			vty_out(vty,"%% Error,operation on getting trunk allow vlanlist fail.\n");
		}
		dbus_message_unref(reply);
	}
	return CMD_SUCCESS;
}

int show_trunk_arp_info
(
	struct vty* vty, 
	unsigned short trunkId
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned char slot_no = 0, port_no = 0;
	unsigned int op_ret = 0, ret = 0, i = 0, j = 0;
	unsigned int arpCount = 0;
	unsigned int ipaddr = 0;
	unsigned char mac[6] = {0};
	unsigned char isTrunk = 0;
	unsigned short vid = 0, vidx = 0;
	unsigned char isTagged = 0, isStatic = 0;
    unsigned char isValid = 0;

	/*vty_out(vty,"show trunk %d arp information before dbus\n",trunkId);*/
	
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_TRUNK_ARP);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&trunkId,
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

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&arpCount);
	/*vty_out(vty,"arp count %d\n",arpCount);*/
	dbus_message_iter_next(&iter);	
	

	if(TRUNK_RETURN_CODE_ERR_NONE == op_ret){
		vty_out(vty,"Detail Information of Trunk %d (%d Items)    * - Invalid item\n",trunkId, arpCount);
		vty_out(vty,"================================================================================\n");
		vty_out(vty,"%-2s%-17s%-19s%-10s%-8s%-6s%-6s%-6s%-8s\n"," ","IP","MAC","SLOT/PORT","TRUNKID","VID","VIDX","ISTAG","STATIC");
		vty_out(vty,"================================================================================\n");

		if(arpCount > 0) {
			dbus_message_iter_recurse(&iter,&iter_array);
			for(i = 0; i<arpCount; i++) {
				dbus_message_iter_recurse(&iter_array,&iter_struct);

				dbus_message_iter_get_basic(&iter_struct,&isValid);
				dbus_message_iter_next(&iter_struct);                
				vty_out(vty,"%-2s",isValid ? " " : "*");
                
				dbus_message_iter_get_basic(&iter_struct,&ipaddr);
				dbus_message_iter_next(&iter_struct);
				/*vty_out(vty,"ipaddr %d\n",ipaddr);*/
				vty_out(vty,"%-3d.%-3d.%-3d.%-3d  ",((ipaddr & 0xff000000) >> 24),((ipaddr & 0xff0000) >> 16),	\
					((ipaddr & 0xff00) >> 8),(ipaddr & 0xff));

				for(j = 0; j< 6; j++) {
		 			dbus_message_iter_get_basic(&iter_struct,&mac[j]);
					/*vty_out(vty,"mac[j] %d\n",mac[j]);*/
					dbus_message_iter_next(&iter_struct);
				}
				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x  ",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
				vty_out(vty,"%2s%-3s","","-");

				dbus_message_iter_get_basic(&iter_struct,&isTrunk);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&trunkId);
				dbus_message_iter_next(&iter_struct);
				vty_out(vty,"%-5s","");
				if(isTrunk) 
					vty_out(vty,"%2s%-6d","",trunkId);
				else
					vty_out(vty,"%-8s"," - ");

				dbus_message_iter_get_basic(&iter_struct,&vid);
				dbus_message_iter_next(&iter_struct);
				vty_out(vty,"%1s%-5d","",vid);

				dbus_message_iter_get_basic(&iter_struct,&vidx);
				dbus_message_iter_next(&iter_struct);
				vty_out(vty,"%-6s"," - ");

				dbus_message_iter_get_basic(&iter_struct,&isTagged);
				dbus_message_iter_next(&iter_struct);
				vty_out(vty,"%-6s",isTagged ? "TRUE":"FALSE");

				dbus_message_iter_get_basic(&iter_struct,&isStatic);
				dbus_message_iter_next(&iter_struct);
				vty_out(vty,"%-8s",isStatic ? "TRUE" : "FALSE");

				vty_out(vty,"\n");
				dbus_message_iter_next(&iter_array);
			}
		}

		/*vty_out(vty,"here end\n");*/
		vty_out(vty,"================================ ARP COUNT %-4d=================================\n",arpCount);
	}
	else if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret) {
    		vty_out(vty,"%% Error:Illegal %d/%d,No such port.\n",slot_no,port_no);/*Stay here,not enter eth_config_node CMD.*/
	}
	else if (TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret) {
        vty_out(vty,"%% The trunk not exists\n");
	}
	else {
		vty_out(vty,"%% Execute command failed.\n"); 
	}
	dbus_message_unref(reply);

	return CMD_SUCCESS;
}



int show_trunk_nexthop_info
(
	struct vty* vty, 
	unsigned short trunkId
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_struct,iter_array;
	unsigned char slot_no = 0, port_no = 0;
	unsigned int op_ret = 0, ret = 0, i = 0, j = 0;
	unsigned int arpCount = 0;
	unsigned int ipaddr = 0;
	unsigned char mac[6] = {0};
	unsigned char isTrunk = 0;
	unsigned short vid = 0, vidx = 0;
	unsigned char isTagged = 0, isStatic = 0;
	unsigned int refCnt = 0; 

	/*vty_out(vty,"show trunk %d arp information before dbus\n",trunkId);*/
	
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_METHOD_SHOW_TRUNK_NEXTHOP);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&trunkId,
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

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&arpCount);
	/*vty_out(vty,"arp count %d\n",arpCount);*/
	dbus_message_iter_next(&iter);	
	

	if(TRUNK_RETURN_CODE_ERR_NONE == op_ret){
		vty_out(vty,"Detail Information of Trunk %d (%d Items)\n",trunkId, arpCount);
		vty_out(vty,"================================================================================\n");
		vty_out(vty,"%-17s%-18s%-10s%-8s%-5s%-5s%-6s%-7s%-4s\n",	\
					 "IP","MAC","SLOT/PORT","TRUNKID","VID","VIDX","ISTAG","STATIC","REF");
		vty_out(vty,"================================================================================\n");

		if(arpCount > 0) {
			dbus_message_iter_recurse(&iter,&iter_array);
			for(i = 0; i<arpCount; i++) {
				dbus_message_iter_recurse(&iter_array,&iter_struct);

				dbus_message_iter_get_basic(&iter_struct,&ipaddr);
				dbus_message_iter_next(&iter_struct);
				/*vty_out(vty,"ipaddr %d\n",ipaddr);*/
				vty_out(vty,"%-3d.%-3d.%-3d.%-3d%2s",((ipaddr & 0xff000000) >> 24),((ipaddr & 0xff0000) >> 16),	\
					((ipaddr & 0xff00) >> 8),(ipaddr & 0xff),"");

				for(j = 0; j< 6; j++) {
		 			dbus_message_iter_get_basic(&iter_struct,&mac[j]);
					/*vty_out(vty,"mac[j] %d\n",mac[j]);*/
					dbus_message_iter_next(&iter_struct);
				}
				
				vty_out(vty,"%02x:%02x:%02x:%02x:%02x:%02x%1s",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],"");
				vty_out(vty,"%4s%s","","-");

				dbus_message_iter_get_basic(&iter_struct,&isTrunk);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&trunkId);
				dbus_message_iter_next(&iter_struct);
				vty_out(vty,"%-5s","");
				if(isTrunk) 
					vty_out(vty,"%2s%-6d","",trunkId);
				else
					vty_out(vty,"%-8s"," - ");

				dbus_message_iter_get_basic(&iter_struct,&vid);
				dbus_message_iter_next(&iter_struct);
				vty_out(vty,"%1s%-4d","",vid);

				dbus_message_iter_get_basic(&iter_struct,&vidx);
				dbus_message_iter_next(&iter_struct);
				vty_out(vty,"%-5s"," - ");

				dbus_message_iter_get_basic(&iter_struct,&isTagged);
				dbus_message_iter_next(&iter_struct);
				vty_out(vty,"%-6s",isTagged ? "TRUE":"FALSE");

				dbus_message_iter_get_basic(&iter_struct,&isStatic);
				dbus_message_iter_next(&iter_struct);
				vty_out(vty,"%-7s",isStatic ? "TRUE" : "FALSE");
				
				dbus_message_iter_get_basic(&iter_struct,&refCnt);
				dbus_message_iter_next(&iter_struct);
				vty_out(vty,"%-4d",refCnt);

				vty_out(vty,"\n");
				dbus_message_iter_next(&iter_array);
			}
		}

		/*vty_out(vty,"here end\n");*/
		vty_out(vty,"===============================NEXT-HOP COUNT %-4d==============================\n",arpCount);
	}
	else if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret) {
    		vty_out(vty,"%% Error:Illegal %d/%d,No such port.\n",slot_no,port_no);/*Stay here,not enter eth_config_node CMD.*/
	}
	else if (TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret) {
        vty_out(vty,"%% The trunk not exists\n");
	}
	else {
		vty_out(vty,"%% Execute command failed.\n"); 
	}
	dbus_message_unref(reply);

	return CMD_SUCCESS;
}


DEFUN(show_trunk_arp_cn_cmd_func,
	show_trunk_arp_cn_cmd,
	"show trunk <1-110> arp",
	SHOW_STR
	"Show trunk information\n"
	"Please input trunk ID.\n"
	"ARP information\n"
)
{
	unsigned int type;
	unsigned short trunkId = 0;
	unsigned int value = 0;
	int ret = 0;
	
	/*get trunk ID*/
	ret = parse_trunk_no((char*)argv[0],&trunkId);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"% Bad parameter,trunk id illegal!\n");
		return CMD_WARNING; 
	}

	/*vty_out(vty,"show trunk %d arp information.\n",trunkId);*/
	
	ret = show_trunk_arp_info(vty,trunkId);

	return CMD_SUCCESS;
}

DEFUN(show_trunk_nexthop_cn_cmd_func,
	show_trunk_nexthop_cn_cmd,
	"show trunk <1-110> nexthop",
	SHOW_STR
	"Show trunk information\n"
	"Please input trunk ID.\n"
	"Nexthop information\n"
)
{
	unsigned int type = 0;
	unsigned short trunkId = 0;
	unsigned int value = 0;
	int ret = 0;
	
	/*get trunk ID*/
	ret = parse_trunk_no((char*)argv[0],&trunkId);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"% Bad parameter,trunk id illegal!\n");
		return CMD_WARNING; 
	}

	/*vty_out(vty,"show trunk %d arp information.\n",trunkId);*/
	
	ret = show_trunk_nexthop_info(vty,trunkId);

	return CMD_SUCCESS;
}


/*********************************************************
 *config one of the trunk members tobe Master port.
 *
 *delete the original master port of trunk
 *set the port pointed by CMD tobe master port.
 *
 *********************************************************/
DEFUN(config_trunk_master_port_cmd_func, 
	config_trunk_master_port_cmd, 
	"set master-port SLOT_PORT", 
	"Set trunk entity configuration\n" 
	"Master port in trunk entity\n"
	CONFIG_ETHPORT_STR
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned char slot_no = 0, local_port_no = 0;
	unsigned int t_slotno = 0, t_portno = 0;
	unsigned short	trunkId = 0;
	unsigned int op_ret = 0, nodesave = 0;
	int local_slot_id = 0;
    int slotNum = 0;
	
	unsigned char  master= 1;    /* for update mster info on smu */
	boolean isAdd = FALSE;  /* no use param */
	
	/*fetch the 1st param : slotNo/portNo*/
	#if 0
	if(4 < strlen(argv[0])){
    	vty_out(vty,"% Bad parameter,unknow portno format.\n");
		return CMD_WARNING;
	}
	#endif/*for slot number biger than 9*/
	op_ret = parse_slotno_localport((char *)argv[0],&t_slotno,&t_portno);
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad parameter,unknow portno format.\n");
		return CMD_WARNING;
	}
	else if (1 == op_ret){
		vty_out(vty,"% Bad parameter,bad slot/port number.\n");
		return CMD_WARNING;
	}
	slot_no = (unsigned char)t_slotno;
	local_port_no = (unsigned char)t_portno;
	if (NPD_FAIL == op_ret) {
    	vty_out(vty,"% Bad parameter,unknow portno format.\n");
		return CMD_WARNING;
	}

	nodesave = (unsigned int)(vty->index);
	trunkId = nodesave;

	if(is_distributed == DISTRIBUTED_SYSTEM)
	{
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_TRUNK_OBJPATH,	\
											NPD_DBUS_TRUNK_INTERFACE,	\
											NPD_DBUS_TRUNK_METHOD_PORT_MEMBER_MASTERSHIP_CONFIG);
		
		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_BYTE,&slot_no,
								DBUS_TYPE_BYTE,&local_port_no,
							 	DBUS_TYPE_UINT16,&trunkId,
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
					vty_out(vty,"% Bad parameter,bad slot/port number.\n");
				}
				else if (TRUNK_RETURN_CODE_ERR_NONE == op_ret ) {
        			vty_out(vty,"update master port of g_dst_trunk on SMU.\n");			
                	/* Wherever slot the CMD we input, then update the gvlanlist[] of 2 MCBs */
            		op_ret = dcli_g_trunks_add_del_port_set_master(vty,isAdd,trunkId,slot_no,local_port_no,master,0,0,0);
            	    if( CMD_SUCCESS != op_ret)
            		{
            			vty_out(vty,"update g_dst_trunk on SMU error !\n");					
            		    return CMD_WARNING;
            	    }	
				/*vty_out(vty,"Ethernet Port %d-%d %s success.\n",slot_no,local_port_no,isAdd?"Add":"Delete");*/
				}/*either slot or local port No err,return NPD_DBUS_ERROR_NO_SUCH_PORT */
				else if (TRUNK_RETURN_CODE_BADPARAM == op_ret) {
					vty_out(vty,"%% Error occurs in Parse portNo or deviceNo.\n");
				}
				else if (TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret) {
					vty_out(vty,"%% Error,trunk to be configured not exist on software.\n");
				}
				else if (TRUNK_RETURN_CODE_PORT_NOTEXISTS == op_ret){
					vty_out(vty,"%% Error,port was not member of this trunk ever.\n");
				}
				else if (TRUNK_RETURN_CODE_ERR_HW == op_ret) {
					vty_out(vty,"%% Error occurs in config on hardware.\n");	
				}
				else if (TRUNK_RETURN_CODE_ERR_GENERAL == op_ret) {
					vty_out(vty,"%% Error occurs in config on software.\n"); /*such as add port to trunkNode struct.*/
				}
				else if (TRUNK_RETURN_CODE_PORT_L3_INTFG == op_ret) {
					vty_out(vty,"%% Error,this port is L3 interface.\n");
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
	else
	{
		
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_TRUNK_OBJPATH,	\
											NPD_DBUS_TRUNK_INTERFACE,	\
											NPD_DBUS_TRUNK_METHOD_PORT_MEMBER_MASTERSHIP_CONFIG);
		
		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_BYTE,&slot_no,
								DBUS_TYPE_BYTE,&local_port_no,
							 	DBUS_TYPE_UINT16,&trunkId,
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
					vty_out(vty,"% Bad parameter,bad slot/port number.\n");
				}
				else if (TRUNK_RETURN_CODE_ERR_NONE == op_ret ) {
				/*vty_out(vty,"Ethernet Port %d-%d %s success.\n",slot_no,local_port_no,isAdd?"Add":"Delete");*/
				}/*either slot or local port No err,return NPD_DBUS_ERROR_NO_SUCH_PORT */
				else if (TRUNK_RETURN_CODE_BADPARAM == op_ret) {
					vty_out(vty,"%% Error occurs in Parse portNo or deviceNo.\n");
				}
				else if (TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret) {
					vty_out(vty,"%% Error,trunk to be configured not exist on software.\n");
				}
				else if (TRUNK_RETURN_CODE_PORT_NOTEXISTS == op_ret){
					vty_out(vty,"%% Error,port was not member of this trunk ever.\n");
				}
				else if (TRUNK_RETURN_CODE_ERR_HW == op_ret) {
					vty_out(vty,"%% Error occurs in config on hardware.\n");	
				}
				else if (TRUNK_RETURN_CODE_ERR_GENERAL == op_ret) {
					vty_out(vty,"%% Error occurs in config on software.\n"); /*such as add port to trunkNode struct.*/
				}
				else if (TRUNK_RETURN_CODE_PORT_L3_INTFG == op_ret) {
					vty_out(vty,"%% Error,this port is L3 interface.\n");
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

DEFUN(config_trunk_port_enable_cmd_fun,
	config_trunk_port_enable_cmd, 
	"set port SLOT_PORT (enable|disable)",
	"Set trunk member state\n"
	"Trunk port member\n"
	CONFIG_ETHPORT_STR
	"Enable the member\n"
	"Disable the member\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned char en_dis = 0;
	unsigned char slot_no = 0, local_port_no = 0;
	unsigned int t_slotno = 0, t_portno = 0;
	unsigned short	trunkId = 0;
	unsigned int 	ret = 0, op_ret = 0, nodesave = 0;
	int local_slot_id = 0;
    int slotNum = 0;
	/*fetch the 1st param: slotNo/portNo*/
	ret = parse_slotno_localport((char *)argv[0],&t_slotno,&t_portno);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"% Bad parameter,unknow portno format.\n");
		return CMD_WARNING;
	}
	else if (1 == ret){
		vty_out(vty,"% Bad parameter,bad slot/port number.\n");
		return CMD_WARNING;
	}
	slot_no = (unsigned char)t_slotno;
	local_port_no = (unsigned char)t_portno;
	if (NPD_FAIL == ret) {
    	vty_out(vty,"% Bad parameter,unknow portno format.\n");
		return CMD_WARNING;
	}

	/*fetch the 2nd param: enable or disable trunk member*/
	if(strncmp(argv[1],"enable",strlen((char*)argv[1]))==0) {
		en_dis = TRUE;
	}
	else if (strncmp(argv[1],"disable",strlen((char*)argv[1]))==0) {
		en_dis = FALSE;
	}
	else {
		vty_out(vty,"% Bad command parameter.\n");
		return CMD_WARNING;
	}		
	nodesave = (unsigned int)(vty->index);
	trunkId = nodesave;
	if(is_distributed == DISTRIBUTED_SYSTEM)
	{
		local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);
    	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
		
		if((local_slot_id < 0) || (slotNum <0))
		{
			vty_out(vty,"read file error ! \n");
			return CMD_WARNING;
		}
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_TRUNK_OBJPATH,	\
											NPD_DBUS_TRUNK_INTERFACE,	\
											NPD_DBUS_TRUNK_METHOD_PORT_MEMBER_ENALBE_DISABLE);
	
		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_BYTE,&slot_no,
								DBUS_TYPE_BYTE,&local_port_no,
						 		DBUS_TYPE_UINT16,&trunkId,
								DBUS_TYPE_BYTE,&en_dis,
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
					vty_out(vty,"% Bad parameter,bad slot/port number.\n");
				}
				else if (TRUNK_RETURN_CODE_ERR_NONE == op_ret ) {
				/*vty_out(vty,"Ethernet Port %d-%d %s success.\n",slot_no,local_port_no,isAdd?"Add":"Delete");*/
				}
				else if (TRUNK_RETURN_CODE_BADPARAM == op_ret) {
					vty_out(vty,"%% Error occurs in Parse portNo or deviceNo.\n");
				}
				else if (TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret) {
					vty_out(vty,"%% Error,trunk not exist on software.\n");
				}
				else if (TRUNK_RETURN_CODE_PORT_NOTEXISTS == op_ret){
					vty_out(vty,"%% Error,port was not member of this trunk ever.\n");
				}
				else if (TRUNK_RETURN_CODE_PORT_ENABLE == op_ret) {
					vty_out(vty,"%% Error port already enable.\n");	
				}
				else if (TRUNK_RETURN_CODE_PORT_NOTENABLE == op_ret) {
					vty_out(vty,"%% Error port not enable.\n"); /*such as add port to trunkNode struct.*/
				}
				else if (TRUNK_RETURN_CODE_ERR_HW == op_ret) {
					vty_out(vty,"%% Error occurs on hardware.\n"); /*such as add port to trunkNode struct.*/
				}
				else if (TRUNK_RETURN_CODE_PORT_LINK_DOWN == op_ret) {
					vty_out(vty,"%% The port must be link first.\n");
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
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_TRUNK_OBJPATH,	\
											NPD_DBUS_TRUNK_INTERFACE,	\
											NPD_DBUS_TRUNK_METHOD_PORT_MEMBER_ENALBE_DISABLE);
	
		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_BYTE,&slot_no,
								DBUS_TYPE_BYTE,&local_port_no,
						 		DBUS_TYPE_UINT16,&trunkId,
								DBUS_TYPE_BYTE,&en_dis,
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
					vty_out(vty,"% Bad parameter,bad slot/port number.\n");
				}
				else if (TRUNK_RETURN_CODE_ERR_NONE == op_ret ) {
				/*vty_out(vty,"Ethernet Port %d-%d %s success.\n",slot_no,local_port_no,isAdd?"Add":"Delete");*/
				}
				else if (TRUNK_RETURN_CODE_BADPARAM == op_ret) {
					vty_out(vty,"%% Error occurs in Parse portNo or deviceNo.\n");
				}
				else if (TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret) {
					vty_out(vty,"%% Error,trunk not exist on software.\n");
				}
				else if (TRUNK_RETURN_CODE_PORT_NOTEXISTS == op_ret){
					vty_out(vty,"%% Error,port was not member of this trunk ever.\n");
				}
				else if (TRUNK_RETURN_CODE_PORT_ENABLE == op_ret) {
					vty_out(vty,"%% Error port already enable.\n");	
				}
				else if (TRUNK_RETURN_CODE_PORT_NOTENABLE == op_ret) {
					vty_out(vty,"%% Error port not enable.\n"); /*such as add port to trunkNode struct.*/
				}
				else if (TRUNK_RETURN_CODE_ERR_HW == op_ret) {
					vty_out(vty,"%% Error occurs on hardware.\n"); /*such as add port to trunkNode struct.*/
				}
				else if (TRUNK_RETURN_CODE_PORT_LINK_DOWN == op_ret) {
					vty_out(vty,"%% The port must be link first.\n");
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

#if 1

/***********************************
* trunk load balance mode select 
* special trunk.
* Params:
*		trunk ID:	 <1-118>
*
* Default load balance Mode: based smac & dmac
* Usage: config trunk <1-118>
************************************/

DEFUN(config_trunk_load_balanc_func,
	config_trunk_load_balanc_cmd,
	"config trunk load-balance (based-port|based-mac|based-ip|based-L4|mac+ip|mac+L4|ip+L4|mac+ip+L4)",
	CONFIG_STR
	"Config layer 2 trunk entity\n"
	"Trunk load-balancing Mode Select\n"
	"Based on sorce port\n"
	"Based on packet Soure&Desti-Mac Fields\n"
	"Based on packet Source&Desti-IP Fields\n"
	"Based on packet L4 Source&Desti-Port Fields\n"
	"Based on packet Macs and IPs Fields\n"
	"Based on packet Macs and L4 Fields\n"
	"Based on packet IPs and L4 Fields\n"
	"Based on packet Macs,IPs and L4 Fields\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	/*unsigned short 	trunkId = 0;*/
	unsigned int loadBalanMode = LOAD_BANLC_MAX;
	unsigned int op_ret = 0;
	int i;
	int local_slot_id = 0;
    int slotNum = 0;

	/*fetch 1st param*/
	if(strncmp(argv[0],"based-port",strlen(argv[0]))==0) {
		loadBalanMode = LOAD_BANLC_SOURCE_DEV_PORT;
	}
	else if (strncmp(argv[0],"based-mac",strlen(argv[0]))==0) {
		loadBalanMode= LOAD_BANLC_SRC_DEST_MAC;
	}
	else if (strncmp(argv[0],"based-ip",strlen(argv[0]))==0) {
		loadBalanMode= LOAD_BANLC_SRC_DEST_IP;
	}
	else if (strncmp(argv[0],"based-L4",strlen(argv[0]))==0) {
		loadBalanMode= LOAD_BANLC_TCP_UDP_RC_DEST_PORT;
	}
	else if (strncmp(argv[0],"mac+ip",strlen(argv[0]))==0) {
		loadBalanMode= LOAD_BANLC_MAC_IP;
	}
	else if (strncmp(argv[0],"mac+L4",strlen(argv[0]))==0) {
		loadBalanMode= LOAD_BANLC_MAC_L4;
	}
	else if (strncmp(argv[0],"ip+L4",strlen(argv[0]))==0) {
		loadBalanMode= LOAD_BANLC_IP_L4;
	}
	else if (strncmp(argv[0],"mac+ip+L4",strlen(argv[0]))==0) {
		loadBalanMode= LOAD_BANLC_MAC_IP_L4;
	}
	else {
		vty_out(vty,"% Bad command parameter.\n");
		return CMD_WARNING;
	}		
	/*trunkId = (unsigned short)(vty->index);*/

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
			query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
												NPD_DBUS_TRUNK_OBJPATH ,	\
												NPD_DBUS_TRUNK_INTERFACE ,	\
												NPD_DBUS_TRUNK_METHOD_CONFIG_LOAD_BANLC_MODE);
			
			dbus_error_init(&err);

			dbus_message_append_args(query,
									 /*DBUS_TYPE_UINT16,&trunkId,*/
									 DBUS_TYPE_UINT32,&loadBalanMode,
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

			if (dbus_message_get_args( reply, &err,
							DBUS_TYPE_UINT32, &op_ret,
							DBUS_TYPE_INVALID)) 
			{	
				if(ETHPORT_RETURN_CODE_NO_SUCH_TRUNK == op_ret ||
					TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret){
					vty_out(vty,"%% Bad parameter,there no trunk exist.");
				}
				else if(TRUNK_RETURN_CODE_ERR_NONE == op_ret)   /*0+3,log exist,then enter trunk_config_node CMD.*/
				{/*nothing to do*/ }
				else if(TRUNK_RETURN_CODE_LOAD_BANLC_CONFLIT == op_ret) {
					vty_out(vty,"%% Bad parameter,trunk load-balance mode same to corrent mode.");
				}
				else if(TRUNK_RETURN_CODE_UNSUPPORT == op_ret) {
					vty_out(vty,"%% The device isn't supported this mode set,it can only support base_mac and base_ip set.");
				}
				else if(COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
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
		}
	}
	else
	{
		
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_TRUNK_OBJPATH ,	\
											NPD_DBUS_TRUNK_INTERFACE ,	\
											NPD_DBUS_TRUNK_METHOD_CONFIG_LOAD_BANLC_MODE);
		
		dbus_error_init(&err);

		dbus_message_append_args(query,
								 /*DBUS_TYPE_UINT16,&trunkId,*/
								 DBUS_TYPE_UINT32,&loadBalanMode,
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

		if (dbus_message_get_args( reply, &err,
						DBUS_TYPE_UINT32, &op_ret,
						DBUS_TYPE_INVALID)) 
		{	
			if(ETHPORT_RETURN_CODE_NO_SUCH_TRUNK == op_ret ||
				TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret){
				vty_out(vty,"%% Bad parameter,there no trunk exist.");
			}
			else if(TRUNK_RETURN_CODE_ERR_NONE == op_ret)   /*0+3,log exist,then enter trunk_config_node CMD.*/
			{/*nothing to do*/ }
			else if(TRUNK_RETURN_CODE_LOAD_BANLC_CONFLIT == op_ret) {
				vty_out(vty,"%% Bad parameter,trunk load-balance mode same to corrent mode.");
			}
			else if(TRUNK_RETURN_CODE_UNSUPPORT == op_ret) {
				vty_out(vty,"%% The device isn't supported this mode set,it can only support base_mac and base_ip set.");
			}
			else if(COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
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
	}
	return CMD_SUCCESS;
}
#endif 

DEFUN(clear_trunk_arp_cn_cmd_func,
	clear_trunk_arp_cn_cmd,
	"clear trunk <1-110> arp [static]",
	CLEAR_STR
	"Clear trunk information\n"
	"Please input the trunk No. <1-118> .\n"
	"ARP information\n"
	"Clear static arp only\n"
	)
{
	unsigned char slot_no = 0, port_no = 0;
	unsigned int type = 0;
	unsigned int value = 0;
	unsigned int isStatic = 0;
	unsigned int trunkId = 0;
	int ret;
    if(argc >= 1){
	    trunkId = strtoul(argv[0],NULL,NULL);
    }
	else {
    	vty_out(vty,"%% Wrong parameter count.\n");
		return CMD_WARNING;
	}
	type = 0;
	if(argc > 1){
		if(!strncmp(argv[1],"static",strlen(argv[1]))){
            isStatic = 1;
		}
		else {
            vty_out(vty,"%% Unknown Command!\n");
			return CMD_WARNING;
		}
	}
	ret = dcli_trunk_clear_trunk_arp(vty,trunkId,isStatic);
	if(ret != CMD_SUCCESS)
		vty_out(vty,"%% Clear arp error\n");
	
	return ret;
}


int dcli_trunk_show_running_config(struct vty *vty) {	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	char *showStr = NULL;	
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,	\
							NPD_DBUS_TRUNK_OBJPATH ,	\
							NPD_DBUS_TRUNK_INTERFACE ,	\
							NPD_DBUS_TRUNK_METHOD_SHOW_RUNNING_CONFIG);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show trunk running config failed get reply.\n");
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
		sprintf(_tmpstr,BUILDING_MOUDLE,"TRUNK");
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

unsigned int dcli_trunk_clear_trunk_arp
(
    struct vty * vty,
    unsigned int trunkId,
    unsigned int isStatic
)
{
    DBusMessage *query, *reply;
	DBusError err;
	unsigned int op_ret = 0;


	
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,	\
							    NPD_DBUS_TRUNK_OBJPATH ,	\
							    NPD_DBUS_TRUNK_INTERFACE ,		\
								NPD_DBUS_TRUNK_METHOD_CLEAR_TRUNK_ARP);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&trunkId,
							 DBUS_TYPE_UINT32,&isStatic,
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
		if (TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret ) 
		{
		    vty_out(vty,"%%The trunk not exists!\n");
		}
		else if(COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
			vty_out(vty,"%% Product not support this function!\n");
		}
		else if(TRUNK_RETURN_CODE_ERR_NONE != op_ret){
			vty_out(vty,"%%Execute command failed.\n"); 
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

void dcli_trunk_init() {
	install_node (&trunk_node, dcli_trunk_show_running_config, "TRUNK_NODE");
	install_default(TRUNK_NODE);
	
	install_element(CONFIG_NODE,&create_trunk_cmd);
	install_element(CONFIG_NODE,&config_trunk_cmd);
	install_element(TRUNK_NODE,	&add_delete_trunk_member_cmd);
	install_element(TRUNK_NODE,	&config_trunk_master_port_cmd);
	install_element(TRUNK_NODE,	&show_current_trunk_cmd);
	install_element(CONFIG_NODE,&config_trunk_load_balanc_cmd);
	install_element(CONFIG_NODE,&delete_trunk_cmd);
	install_element(CONFIG_NODE,&show_trunk_one_cmd);
	install_element(CONFIG_NODE,&show_trunk_list_cmd);
	install_element(VIEW_NODE,&show_trunk_list_cmd);
	install_element(VIEW_NODE,&show_trunk_one_cmd);
	install_element(ENABLE_NODE,&show_trunk_list_cmd);
	install_element(ENABLE_NODE,&show_trunk_one_cmd);
	#if 0  /* do not need, zhangdi 2011-12-06 */
	install_element(TRUNK_NODE,	&trunk_allow_refuse_vlan_cmd);
	install_element(TRUNK_NODE,	&config_trunk_port_enable_cmd);
	install_element(TRUNK_NODE,	&show_trunk_vlanlist_cmd);
	install_element(CONFIG_NODE,&show_trunk_by_name_cmd);
	install_element(VIEW_NODE,&show_trunk_by_name_cmd);
	install_element(ENABLE_NODE,&show_trunk_by_name_cmd);	
	install_element(CONFIG_NODE,&config_trunk_name_cmd);
	install_element(CONFIG_NODE,&delete_trunk_name_cmd);
	install_element(CONFIG_NODE,&show_trunk_arp_cn_cmd);
	install_element(CONFIG_NODE,&show_trunk_nexthop_cn_cmd);
	install_element(CONFIG_NODE,&clear_trunk_arp_cn_cmd);
	install_element(ENABLE_NODE, &show_trunk_arp_cn_cmd);
	install_element(ENABLE_NODE, &show_trunk_nexthop_cn_cmd);
	#endif
}


#ifdef __cplusplus
}
#endif

