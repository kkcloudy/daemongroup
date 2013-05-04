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
* npd_dynamic_trunk.c
*
*
* CREATOR:
*		hanhui@autelan.com
*
* DESCRIPTION:
*		DCLI for DYNAMIC TRUNK module.
*
* DATE:
*		06/28/2010	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.3 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include <zebra.h>
#include <stdlib.h>
#include <dbus/dbus.h>

#include "sysdef/npd_sysdef.h"
#include "dbus/npd/npd_dbus_def.h"
#include "util/npd_list.h"
#include "npd/nam/npd_amapi.h"
#include "command.h"
#include "if.h"
#include "dcli_vlan.h"
#include "dcli_trunk.h"
#include "dcli_dynamic_trunk.h"
#include "sysdef/returncode.h"
#include "dcli_dynamic_trunk.h"
#include "dcli_main.h"
#include "dcli_sem.h"
#include "board/board_define.h"
#include "dcli_intf.h"
struct cmd_node dynamic_trunk_node = 
{
	DYNAMIC_TRUNK_NODE,
	"%s(config-dynamic-trunk)# "
};
struct cmd_node distributed_dynamic_trunk_node = 
{
	DISTRIBUTED_DYNAMIC_TRUNK_NODE,
	"%s(config-dyn-trunk-sw-board)# "
};
extern char dcli_vty_ifname[INTERFACE_NAMSIZ+1];
extern char vlan_eth_port_ifname [INTERFACE_NAMSIZ];

extern DBusConnection *dcli_dbus_connection;
extern int is_distributed;
static DBusConnection *dcli_dbus_connection_lacp;



/* add for distributed dynamic trunk*/
DEFUN(config_lacp_on_board_cmd_func,
	  config_lacp_on_board_cmd,
	  "config dynamic trunk switch-board <1-10>",
	  CONFIG_STR
	  "Configure lacp of Switch-board\n"
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
	if (ACL_RETURN_CODE_ERROR == ret) {
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
			vty_out(vty, "<<========== Config dynamic trunk switch-board slot: %d =========>>\n",dist_slot);
            dcli_dbus_connection_lacp= dbus_connection_dcli[dist_slot]->dcli_dbus_connection;			

			/*vty_out(vty,"Enter inner CMD node...\n");*/
			if(CONFIG_NODE == vty->node)
			{
				vty->node = DISTRIBUTED_DYNAMIC_TRUNK_NODE;
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

 


/**************************************
* create dynamic trunk entity on Both Sw & Hw 
*		and kernel(bonding interface). 
* Params: 
*       trunk ID:	<1-8>
*	  trunk name:  such as "autelan0","autelan1","_office"
*
* Usage: create dynamic trunk <1-8> TRUNKNAME 
****************************************/
DEFUN(create_dynamic_trunk_cmd_fun, 
	create_dynamic_trunk_cmd, 
	"create dynamic-trunk <1-8>",
	"Create configuration\n"
	"Create dynamic-trunk on system\n"
	"Dynamic trunk ID range <1-8>\n"
	"Dynamic trunk name begins with alphabet or '_',no more than 20 alphabets\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned short 	trunkId = 0;
	int ret = 0;
	unsigned int op_ret = 0;
	
	/*get trunk ID*/
	ret = parse_trunk_no((char*)argv[0],&trunkId);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"% Bad parameter,dynamic trunk id illegal!\n");
		return CMD_WARNING; 
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_TRUNK_OBJPATH ,	\
										NPD_DBUS_TRUNK_INTERFACE ,	\
										NPD_DBUS_TRUNK_METHOD_CREATE_DYNAMIC_TRUNK);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&trunkId,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_lacp,query,-1, &err);

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
					DBUS_TYPE_UINT32, &op_ret,
					DBUS_TYPE_INVALID)) 
	{
		if(TRUNK_RETURN_CODE_ERR_NONE == op_ret){
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
		if(TRUNK_RETURN_CODE_ERR_NONE != op_ret) {			
			vty_out(vty, dcli_dynamic_trunk_error_info(op_ret));
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
	return CMD_WARNING;
}


/**/ 
DEFUN(delete_dynamic_trunk_cmd_fun, 
	delete_dynamic_trunk_cmd, 
	"delete dynamic-trunk <1-8>",
	"Delete operation\n"
	"Delete dynamic trunk on system\n"
	"Dynamic trunk ID range <1-8>\n")
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned short 	trunkId =0;
	int ret = 0;
	unsigned int op_ret = 0;

	ret = parse_trunk_no((char*)argv[0], &trunkId);
	if(NPD_FAIL == ret){
		vty_out(vty,"% Bad parameter,illegal dynamic trunk ID %s !\n", (char *)argv[0]);
		return CMD_WARNING;
	}
	else {
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_TRUNK_OBJPATH ,	\
											NPD_DBUS_TRUNK_INTERFACE ,	\
											NPD_DBUS_TRUNK_METHOD_DELETE_DYNAMIC_TRUNK );
		
		dbus_error_init(&err);

		dbus_message_append_args(query,
								DBUS_TYPE_UINT16,&trunkId,
								DBUS_TYPE_INVALID);
		
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_lacp,query,-1, &err);
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
		if(TRUNK_RETURN_CODE_ERR_NONE != op_ret) {			
			vty_out(vty, dcli_dynamic_trunk_error_info(op_ret));
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


DEFUN(add_delete_dynamic_trunk_member_cmd_fun, 
	add_delete_dynamic_trunk_member_cmd, 
	"(add|delete) port PORTNO",
	"Add port into dynamic trunk\n"
	"Delete port from dynamic trunk\n"
	"Port on system\n"
	CONFIG_ETHPORT_STR)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned char slot_no = 0, local_port_no = 0;
	unsigned int  t_slotno = 0, t_portno = 0;
	unsigned int 	  isAdd = FALSE;
	unsigned short	trunkId = 0;
	unsigned int 	op_ret = 0, nodesave = 0;
	int slotNum = 0;

	if(2 != argc){
		vty_out(vty, "%% Bad parameter number: %d\n", argc);
	}
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
	
	nodesave = (unsigned int)(vty->index);
	trunkId = (unsigned short)nodesave;

	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	if(slot_no < 0)
	{
		vty_out(vty,"read file error ! \n");
		return CMD_WARNING;
	}
	if(slot_no > slotNum)
	{
		vty_out(vty,"% Bad parameter,bad slot/port number.\n");
		return CMD_WARNING;
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_TRUNK_OBJPATH,	\
										NPD_DBUS_TRUNK_INTERFACE,	\
										NPD_DBUS_TRUNK_METHOD_DYNAMIC_TRUNK_PORT_MEMBER_ADD_DEL);
	
	dbus_error_init(&err);

	dbus_message_append_args(	query,
							 	DBUS_TYPE_UINT32,&isAdd,
								DBUS_TYPE_BYTE,&slot_no,
								DBUS_TYPE_BYTE,&local_port_no,
							 	DBUS_TYPE_UINT16,&trunkId,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_lacp,query,-1, &err);
	
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
			if(TRUNK_RETURN_CODE_ERR_NONE != op_ret){
				vty_out(vty, dcli_dynamic_trunk_error_info(op_ret));
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

/***********************************
* Enter dynamic trunk config mode to configure 
* special trunk.
* Params:
*		dynamic trunk ID:	 <1-8>
* Usage: config trunk <1-8>
************************************/

DEFUN(config_dynamic_trunk_func,
	config_dynamic_trunk_cmd,
	"config dynamic-trunk <1-8>",
	CONFIG_STR
	"Config dynamic trunk entity\n"
	"Specify trunk ID for trunk entity\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned short 	trunkId = 0;
	int ret = 0;
	unsigned int op_ret = 0, nodesave = 0;

	ret = parse_trunk_no((char*)argv[0],&trunkId);
	
	if (NPD_FAIL == ret) {
    	vty_out(vty,"% Bad parameter,trunk ID illegal!\n");
		return CMD_WARNING;
	}
	else {
		/*once bad param,it'll NOT sed message to NPD*/
		query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
											NPD_DBUS_TRUNK_OBJPATH ,	\
											NPD_DBUS_TRUNK_INTERFACE ,	\
											NPD_DBUS_DYNAMIC_TRUNK_METHOD_CONFIG);
		
		dbus_error_init(&err);

		dbus_message_append_args(query,
								 DBUS_TYPE_UINT16,&trunkId,
								 DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_lacp,query,-1, &err);
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
		if(TRUNK_RETURN_CODE_ERR_NONE != op_ret) {			
			vty_out(vty, dcli_dynamic_trunk_error_info(op_ret));
		}
		else if(TRUNK_RETURN_CODE_ERR_NONE == op_ret)   /*0+3,trunk exist,then enter trunk_config_node CMD.*/
		{
			if(DISTRIBUTED_DYNAMIC_TRUNK_NODE == vty->node) {
				vty->node = DYNAMIC_TRUNK_NODE;
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

int dcli_dynamic_trunk_show_running_config
(
	struct vty * vty
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	char *showStr = NULL;	
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
    int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);	
	int product_type = get_product_info(SEM_PRODUCT_TYPE_PATH);
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
				query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,	\
										NPD_DBUS_TRUNK_OBJPATH ,	\
										NPD_DBUS_TRUNK_INTERFACE ,	\
										NPD_DBUS_DYNAMIC_TRUNK_METHOD_SHOW_RUNNING_CONFIG);

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
					printf("show dynamic-trunk running config failed get reply.\n");
					if (dbus_error_is_set(&err)) {
						printf("%s raised: %s",err.name,err.message);
						dbus_error_free_for_dcli(&err);
					}
					return CMD_WARNING;
				}

				if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_STRING, &showStr,
								DBUS_TYPE_INVALID)) 
				{

					char _tmpstr[64];
					memset(_tmpstr,0,64);
					sprintf(_tmpstr,BUILDING_MOUDLE,"DYNAMIC TRUNK");
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
					return CMD_WARNING;
				}

				dbus_message_unref(reply);
			}
		}
	 }
	return CMD_SUCCESS;	
}

int dcli_dynamic_trunk_show_trunk_member_list
(
	struct vty * vty,
	unsigned short trunkId
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;	
	DBusMessageIter	 iter;
	unsigned int haveMore = FALSE;
	unsigned char slot = 0, port = 0;
	unsigned short id = 0;
	unsigned int tmpPortCount = 0;
	unsigned int tableOut = FALSE;
	unsigned int ret = TRUNK_RETURN_CODE_ERR_NONE;
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,	\
							NPD_DBUS_TRUNK_OBJPATH ,	\
							NPD_DBUS_TRUNK_INTERFACE ,	\
							NPD_DBUS_DYNAMIC_TRUNK_METHOD_SHOW_TRUNK_MEMBER_LIST);

	dbus_error_init(&err);
	
	dbus_message_append_args(query,
									 DBUS_TYPE_UINT16,&trunkId,
									 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_lacp,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show trunk running config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}

	dbus_message_iter_init(reply,&iter);
	
	if(!trunkId){
		dbus_message_iter_get_basic(&iter,&id);	
		dbus_message_iter_next(&iter);
		if(id){
			vty_out(vty, "=====================================\n");
			
			vty_out(vty, "%-7s%-10s%-20s\n", " ID", "  NAME", "PORT MEMBER LIST");
			vty_out(vty, "-------------------------------------\n");
			tableOut = TRUE; /* flag for output table endline */
		}
		while(id){/*get and output all exist dynamic trunk info*/
			vty_out(vty, "%1s%-4d%2s", "", id, "");
			vty_out(vty, "dytrunk%-3d", id);
			dbus_message_iter_get_basic(&iter,&haveMore);				
			dbus_message_iter_next(&iter);
			tmpPortCount = 0;
			if(!haveMore){
				vty_out(vty, "No port member");
			}
			while(haveMore){/*get and output all ports for this dynamic trunk*/
				if(tmpPortCount%4 > 0){/* add ',' between 2 ports on the same line */
					vty_out(vty,",");
				}
				else if(tmpPortCount > 0){/* output 4 ports per line */
					vty_out(vty, "\n%17s", "");
				}
				dbus_message_iter_get_basic(&iter,&slot);				
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&port);				
				dbus_message_iter_next(&iter);				
				vty_out(vty, "%d/%d", slot, port);				
				tmpPortCount++;				
				dbus_message_iter_get_basic(&iter,&haveMore);				
				dbus_message_iter_next(&iter);
			}
			vty_out(vty, "\n");
			dbus_message_iter_get_basic(&iter,&id);	
			dbus_message_iter_next(&iter);
		}
		if(tableOut){
			vty_out(vty, "=====================================\n");
		}
	}
	else{
		id = trunkId;
		dbus_message_iter_get_basic(&iter,&ret);				
		dbus_message_iter_next(&iter);
		if(TRUNK_RETURN_CODE_ERR_NONE == ret){
			vty_out(vty, "=====================================\n");
			
			vty_out(vty, "%-7s%-10s%-20s\n", " ID", "  NAME", "PORT MEMBER LIST");
			vty_out(vty, "-------------------------------------\n");
			vty_out(vty, "%1s%-4d%2s", "", id, "");
			vty_out(vty, "dytrunk%-3d", id);
			dbus_message_iter_get_basic(&iter,&haveMore);				
			dbus_message_iter_next(&iter);
			tmpPortCount = 0;
			if(!haveMore){
				vty_out(vty, "No port member");
			}
			while(haveMore){/*get and output all ports for this dynamic trunk*/
				if(tmpPortCount%4 > 0){/* add ',' between 2 ports on the same line */
					vty_out(vty,",");
				}
				else if(tmpPortCount > 0){/* output 4 ports per line */
					vty_out(vty, "\n%17s", "");
				}
				dbus_message_iter_get_basic(&iter,&slot);				
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&port);				
				dbus_message_iter_next(&iter);				
				vty_out(vty, "%d/%d", slot, port);				
				tmpPortCount++;				
				dbus_message_iter_get_basic(&iter,&haveMore);				
				dbus_message_iter_next(&iter);
			}
			vty_out(vty, "\n");			
			vty_out(vty, "=====================================\n");
		}
		else if(TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == ret){
			vty_out(vty, "%% The dynamic-trunk %d does not exist!\n", id);
		}
		else {
			vty_out(vty, "%% Show dynamic-trunk member failed, ret %#x\n", ret);
		}
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;	
}
DEFUN(show_dynamic_trunk_member_list_func,
	show_dynamic_trunk_member_list_cmd,
	"show dynamic-trunk list",
	SHOW_STR
	"Show dynamic trunk entity\n"
	"Show dynamic trunk entity list\n"
)
{
	if(dcli_dynamic_trunk_show_trunk_member_list(vty, 0)){
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

DEFUN(show_dynamic_trunk_member_func,
	show_dynamic_trunk_member_cmd,
	"show dynamic-trunk <1-8>",
	SHOW_STR
	"Show dynamic trunk entity\n"
	"Specify dynamic trunk ID for trunk entity\n"
)
{
	unsigned short trunkId = 0;
	if(1 < argc){
		vty_out("%% Too many parameters! count %d, expected count 0 or 1\n", argc);
		return CMD_WARNING;
	}
	if(1 == argc){
		trunkId = strtoul((char *)argv[0], NULL, 0);
	}
	else{
		if(DYNAMIC_TRUNK_NODE == vty->node){
			trunkId = (unsigned short)vty->index;
		}
		else {
			trunkId = 0;
		}
	}
	if((trunkId)&&((trunkId < 1)||(trunkId > 8))){
		vty_out("%% Invalidated dynamic trunk ID: %d!\n", trunkId);
	}
	if(dcli_dynamic_trunk_show_trunk_member_list(vty, trunkId)){
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
}

ALIAS(show_dynamic_trunk_member_func,
	show_dynamic_trunk_member_current_cmd,
	"show dynamic-trunk",
	SHOW_STR
	"Show dynamic trunk entity\n"
);
int dcli_show_dynamic_trunk_hw_info_list
	(
	 struct vty * vty,
	 unsigned short trunkId
)
	{
    DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter  iter;

	unsigned short	id = 0;
	
	unsigned int	ret = TRUNK_RETURN_CODE_ERR_NONE;
	unsigned int	portnum = 0;
	unsigned int	numOfPortHW = 0;
	unsigned char slot_no = 0;
	unsigned char port_no = 0;
    unsigned char status = 0;	
	unsigned int  count = 0;
 
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	\
										NPD_DBUS_TRUNK_OBJPATH ,	\
										NPD_DBUS_TRUNK_INTERFACE ,	\
										NPD_DBUS_TRUNK_METHOD_SHOW_DYNAMIC_TRUNK_HW_INFO);
	

	dbus_error_init(&err);
	dbus_message_append_args(query,
									 DBUS_TYPE_UINT16,&trunkId,
									 DBUS_TYPE_INVALID);	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_lacp,query,-1, &err);
	
	dbus_message_unref(query);

	if (NULL == reply) {		
		printf("show dynamic-trunk running config failed get reply.\n");		
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
    dbus_message_iter_init(reply,&iter);
   if(!trunkId){
   	     dbus_message_iter_get_basic(&iter,&id);	
		 dbus_message_iter_next(&iter);
   		 if(id){
			vty_out(vty,"=======================================================\n");
			vty_out(vty,"%-8s %-20s\n","TrunkID","Port Member List");
			vty_out(vty,"-------------------------------------------------------\n");
			}
		 while(id){
		   	dbus_message_iter_get_basic(&iter,&ret);
		    dbus_message_iter_next(&iter);
		   	if(TRUNK_RETURN_CODE_ERR_NONE == ret){
			    dbus_message_iter_get_basic(&iter,&id);	
			    dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&numOfPortHW);
				dbus_message_iter_next(&iter);
				vty_out(vty, "%3s%-4d%2s", "", id, "");
				if(!numOfPortHW){
					vty_out(vty, "No port exists");
				   }
	            for(portnum = 0;portnum<numOfPortHW;portnum++){	
					if(count%4 > 0){/* add ',' between 2 ports on the same line */
					vty_out(vty,",");
				    }
				    else if(count > 0){/* output 4 ports per line */
					vty_out(vty, "\n%17s", "");
				    }
					dbus_message_iter_get_basic(&iter,&slot_no);				 

					dbus_message_iter_next(&iter); 				 
					dbus_message_iter_get_basic(&iter,&port_no);	
					 
					dbus_message_iter_next(&iter);
					dbus_message_iter_get_basic(&iter,&status);		 
					 
					dbus_message_iter_next(&iter);
					 
					vty_out(vty,"%d/%d(%c)",slot_no,port_no,status);
					 count++;
	                }			
				vty_out(vty,"\n");
			}
			else {
			    vty_out(vty, dcli_dynamic_trunk_error_info(ret));
	        }	
			dbus_message_iter_get_basic(&iter,&id);	
			dbus_message_iter_next(&iter);
		   	}	
	        vty_out(vty,"=======================================================\n");  	
}	
	else{
		id = trunkId;
		dbus_message_iter_get_basic(&iter,&ret);
	    dbus_message_iter_next(&iter);
		if(TRUNK_RETURN_CODE_ERR_NONE == ret) {
			vty_out(vty,"=======================================================\n");
			vty_out(vty,"%-8s %-20s\n","TrunkID","Port Member List");
			vty_out(vty,"=======================================================\n");
			dbus_message_iter_get_basic(&iter,&numOfPortHW);
			dbus_message_iter_next(&iter);
		    vty_out(vty, "%3s%-4d%2s", "", id, "");
			if(!numOfPortHW){
				vty_out(vty, "No port exists");
			    }
            for(portnum = 0;portnum<numOfPortHW;portnum++){	
					if(count%4 > 0){/* add ',' between 2 ports on the same line */
					vty_out(vty,",");
				    }
				    else if(count > 0){/* output 4 ports per line */
					vty_out(vty, "\n%17s", "");
				    }
					 dbus_message_iter_get_basic(&iter,&slot_no);				 

					 dbus_message_iter_next(&iter); 				 
					 dbus_message_iter_get_basic(&iter,&port_no);	
					 
					 dbus_message_iter_next(&iter);
					 dbus_message_iter_get_basic(&iter,&status);		 
					 
					 dbus_message_iter_next(&iter);
					 
					 vty_out(vty,"%d/%d(%c)",slot_no,port_no,status);
					 count++;
                	}
			vty_out(vty,"\n");
			vty_out(vty,"=======================================================\n");		
			}
		
		else{
			vty_out(vty, dcli_dynamic_trunk_error_info(ret));
			}	
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(show_dynamic_trunk_hardware_func,
	show_dynamic_trunk_hardware_cmd,
	"show dynamic-trunk hardware-information <1-8>",
	SHOW_STR
	"Show dynamic trunk entity\n"
	"Show hardware information"
	"Dynamic-trunk ID range in <1-8>"
	)
{
	unsigned short	trunkId = 0;

	if(1 < argc){
		vty_out("%% Too many parameters! count %d, expected count 0 or 1\n", argc);
		return CMD_WARNING;
	}
	if(1 == argc){
	      parse_trunk_no((char*)argv[0],&trunkId);
	}
	else{
		if(DYNAMIC_TRUNK_NODE == vty->node){
			trunkId = (unsigned short)vty->index;
		}
		else {
			trunkId = 0;
		}
	}
    if((trunkId)&&((trunkId < 1)||(trunkId > 8))){
		vty_out("%% Invalidated dynamic trunk ID: %d!\n", trunkId);
	}
	if(dcli_show_dynamic_trunk_hw_info_list(vty,trunkId)){
		return CMD_WARNING;
	}
	return CMD_SUCCESS;	
}
ALIAS(show_dynamic_trunk_hardware_func,
	show_dynamic_trunk_hardware_current_cmd,
	"show dynamic-trunk hardware-information",
	SHOW_STR
	"Show dynamic trunk entity\n"
	"Show hardware information"	    
);

DEFUN(show_dynamic_trunk_hardware_list_func,
	show_dynamic_trunk_hardware_list_cmd,
	"show dynamic-trunk hardware-information list",
	SHOW_STR
	"Show dynamic trunk entity\n"
	"Show hardware information"
	"All dynamic trunks"
	)
	{
	if(dcli_show_dynamic_trunk_hw_info_list(vty,0)){
		return CMD_WARNING;
	}
	return CMD_SUCCESS;
	}
char * dcli_dynamic_trunk_error_info(unsigned int op_ret){
	
	if(TRUNK_RETURN_CODE_UNSUPPORT == op_ret){
		return "%% This port is not support the command!\n";
	}
	else if(TRUNK_RETURN_CODE_TRUNKID_OUT_OF_RANGE == op_ret){
		return "%% The trunk ID is out of range!\n";
	}
	else if(TRUNK_RETURN_CODE_NO_SUCH_PORT == op_ret){
		return "%% Bad parameter,bad slot/port number.\n";
	}
	else if(TRUNK_RETURN_CODE_PORT_MBRS_FULL == op_ret){
		return "%% Dynamic trunk port member full!\n"; 
	}
	else if (TRUNK_RETURN_CODE_BADPARAM == op_ret) {
		return "%% Bad parameter input!\n";
	}
	else if(TRUNK_RETURN_CODE_TRUNK_EXISTS == op_ret){
		return "%% The dynamic trunk exists!\n";
	}
	else if (TRUNK_RETURN_CODE_TRUNK_NOTEXISTS == op_ret) {
		return "%% The dynamic trunk not exists!\n";
	}
	else if (TRUNK_RETURN_CODE_PORT_EXISTS == op_ret) {
		return "%% Port is already a member of this trunk!\n ";
	}
	else if (TRUNK_RETURN_CODE_PORT_NOTEXISTS == op_ret){
		return "%% Port is not member of this dynamic trunk!\n";
	}
	else if (TRUNK_RETURN_CODE_ERR_HW == op_ret) {
		return "%% Config dynamic trunk on hardware failure!\n";	
	}
	else if (TRUNK_RETURN_CODE_ERR_GENERAL == op_ret) {
		return "%% Config failed on software!\n"; 
	}
	else if(TRUNK_RETURN_CODE_INTERFACE_NOT_EXIST == op_ret){
		return "%% The port is not a interface!\n";	
	}
	else if(TRUNK_RETURN_CODE_INTERFACE_L3_ENABLE == op_ret){
			return "%% The port is not a l3 disable interface, please disable it first!\n";
	}
	else if (TRUNK_RETURN_CODE_MEMBERSHIP_CONFICT == op_ret) {
		return "%% This port is already a member of a dynamic trunk!\n"; 
	}	
	else if(TRUNK_RETURN_CODE_ERR_NONE != op_ret){
		return "%% Unknown error !\n";
	}
}
DEFUN(dynamic_trunk_allow_refuse_vlan_cmd_fun, 
	dynamic_trunk_allow_refuse_vlan_cmd, 
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
	ret = parse_vlan_no((char*)argv[1],&vid);
	if(0 != ret ){
		vty_out(vty,"% Bad vlan parameter.\n");
		return CMD_WARNING;
	}
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
	/*fetch the 3rd param : tag ? untag*/	

	nodesave = (unsigned int)(vty->index);
	trunkId = nodesave;
	
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

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_lacp,query,-1, &err);

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


/* config eth-port to route or promiscuous mode */
int dcli_eth_port_lacp_function_enable
(
    struct vty * vty,
    int ifnameType,
    unsigned char slot_no,
    unsigned char port_no,
    unsigned char endis
    /*unsigned int ifIndex*/
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int op_ret = NPD_DBUS_ERROR;
	unsigned int tmpIfIndex = ~0UI;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,		\
								NPD_DBUS_ETHPORTS_OBJPATH,		\
								NPD_DBUS_ETHPORTS_INTERFACE,		\
								NPD_DBUS_ETHPORTS_INTERFACE_LACP_FUNCTION_ENDIS);
	dbus_error_init(&err);
	dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&ifnameType,
								DBUS_TYPE_BYTE,&slot_no,
								DBUS_TYPE_BYTE,&port_no,
								DBUS_TYPE_BYTE,&endis,
							 	DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_lacp,query,-1, &err);
	
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
			if(INTERFACE_RETURN_CODE_SUCCESS == op_ret)
			{
	            dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else if(INTERFACE_RETURN_CODE_NO_SUCH_PORT == op_ret)
			{
				vty_out(vty,"%% Bad parameter,bad slot/port number!\n");
			}
			else if((endis) && (INTERFACE_RETURN_CODE_INTERFACE_EXIST == op_ret))
			{
				vty_out(vty,"%% Port %d/%d has  been enabled  lacp already !!!\n",slot_no,port_no);
			}
			else if((!endis) && (INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST == op_ret))
			{
				vty_out(vty,"%% Port %d/%d has been disabld lacp already !!!\n",slot_no,port_no);
			}
			else
			{
				vty_out(vty,dcli_error_info_intf(op_ret));
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
	return CMD_WARNING;

}

DEFUN(
    dynamic_trunk_port_function_enable,
    dynamic_trunk_port_function_enable_cmd,
    "(enable|disable) port PORTNO lacp function",
    "Select an interface to enable lacp function\n"
    "Interface's name\n"
    "Port on system"
    CONFIG_ETHPORT_STR)
{
    int ret = 0;
	unsigned int t_slotno = 0,t_portno = 0;
	unsigned char slot_no = 0,local_port_no = 0;
	unsigned char endis = 0;
	int slotNum = 0;

	slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
	if(slotNum <0)
	{
		vty_out(vty,"read file error ! \n");
		return CMD_WARNING;
	}
	if(2 != argc)
	{
		vty_out(vty, "%% Bad parameter number: %d\n", argc);
	}
	if(strncmp(argv[0],"enable",strlen(argv[0]))==0) 
	{
		endis = TRUE;
	}
	else if (strncmp(argv[0],"disable",strlen(argv[0]))==0) 
	{
		endis = FALSE;
	}
	else 
	{
		vty_out(vty,"% Bad parameter.\n");
		return CMD_WARNING;
	}
	ret = parse_slotno_localport((char *)argv[1],&t_slotno,&t_portno);
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
	if(slot_no > slotNum)
	{
		vty_out(vty,"% Bad parameter,bad slot/port number.\n");
		return CMD_WARNING;
	}
	ret = dcli_eth_port_lacp_function_enable(vty,0,slot_no,local_port_no,endis);
	if(ret != CMD_SUCCESS)
	{
		return CMD_WARNING;
	}
    return CMD_SUCCESS;
}



void dcli_dynamic_trunk_init() {
	dcli_dbus_connection_lacp = dcli_dbus_connection;
	install_node(&distributed_dynamic_trunk_node,NULL,"DISTRIBUTED_DYNAMIC_TRUNK_NODE");
	install_default(DISTRIBUTED_DYNAMIC_TRUNK_NODE);
	install_node (&dynamic_trunk_node, dcli_dynamic_trunk_show_running_config,"DYNAMIC_TRUNK_NODE");
	install_default(DYNAMIC_TRUNK_NODE);
	install_element(CONFIG_NODE,&config_lacp_on_board_cmd);
	install_element(DISTRIBUTED_DYNAMIC_TRUNK_NODE,&create_dynamic_trunk_cmd);
	install_element(DISTRIBUTED_DYNAMIC_TRUNK_NODE,&delete_dynamic_trunk_cmd);
	install_element(DISTRIBUTED_DYNAMIC_TRUNK_NODE,&config_dynamic_trunk_cmd);
	install_element(DYNAMIC_TRUNK_NODE,&dynamic_trunk_port_function_enable_cmd);
	install_element(DYNAMIC_TRUNK_NODE,&add_delete_dynamic_trunk_member_cmd);
	install_element(DYNAMIC_TRUNK_NODE,&dynamic_trunk_allow_refuse_vlan_cmd);
	install_element(DYNAMIC_TRUNK_NODE,&show_dynamic_trunk_member_current_cmd);
	install_element(DYNAMIC_TRUNK_NODE,&show_dynamic_trunk_member_cmd);
	install_element(DYNAMIC_TRUNK_NODE,&show_dynamic_trunk_hardware_current_cmd);
    install_element(DYNAMIC_TRUNK_NODE,&show_dynamic_trunk_hardware_cmd);
	
	#if 0
	install_node (&dynamic_trunk_node, dcli_dynamic_trunk_show_running_config);
	install_default(DYNAMIC_TRUNK_NODE);
	
	install_element(CONFIG_NODE,&create_dynamic_trunk_cmd);
	install_element(CONFIG_NODE,&delete_dynamic_trunk_cmd);
	
	install_element(CONFIG_NODE,&config_dynamic_trunk_cmd);
	install_element(DYNAMIC_TRUNK_NODE,&add_delete_dynamic_trunk_member_cmd);
	install_element(DYNAMIC_TRUNK_NODE,&show_dynamic_trunk_member_current_cmd);
	install_element(DYNAMIC_TRUNK_NODE,&show_dynamic_trunk_member_cmd);
	install_element(DYNAMIC_TRUNK_NODE,&show_dynamic_trunk_hardware_current_cmd);
    install_element(DYNAMIC_TRUNK_NODE,&show_dynamic_trunk_hardware_cmd);
	install_element(CONFIG_NODE,&show_dynamic_trunk_hardware_cmd);
	install_element(CONFIG_NODE,&show_dynamic_trunk_member_list_cmd);
	install_element(CONFIG_NODE,&show_dynamic_trunk_member_cmd);
	install_element(CONFIG_NODE,&show_dynamic_trunk_hardware_list_cmd);
	install_element(DYNAMIC_TRUNK_NODE,&dynamic_trunk_allow_refuse_vlan_cmd);
	#endif
	
}

#ifdef __cplusplus
}
#endif
