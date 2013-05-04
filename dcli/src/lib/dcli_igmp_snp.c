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
* dcli_igmp_snp.c
*
*
* CREATOR:
* 		wujh@autelan.com
*
* DESCRIPTION:
* 		dcli to handle igmp orders comunicate with igmp module and npd.
*
* DATE:
*		6/19/2008
*
* FILE REVISION NUMBER:
*  		$Revision: 1.57 $
*
*******************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif
#include <zebra.h>
#include <dbus/dbus.h>
#include "command.h"

#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>
#include <util/npd_list.h>

#include "dcli_igmp_snp.h"
#include "dcli_vlan.h"
#include "dcli_common_igmp_snp.h"
#include "dcli_main.h"
#include "dcli_system.h"
#include "sysdef/returncode.h"

/* for distributed igmp houxx@autelan.com 2012-11-27 */
#include "dcli_main.h"
#include "dcli_sem.h"
#include "board/board_define.h"

/* add for distributed igmp */
extern DBusConnection *dcli_dbus_connection_igmp = NULL;
extern int is_distributed;
extern int igmp_dist_slot = 1; 

extern unsigned long dcli_ip2ulong(char *str);
extern DBusConnection *dcli_dbus_connection;

/*******************************************************************************
 * parse_timeout
 *
 * DESCRIPTION:
 *   		parse the igmp time value from argv[1]
 *
 * INPUTS:
 * 		str - point to argv[1]
 *
 * OUTPUTS:
 *    	timeout - output the igmp time value get from str
 *
 * RETURNS:
 *		IGMPSNP_RETURN_CODE_OK - parse the time value success
 *		IGMPSNP_RETURN_CODE_ERROR - str is null or timeout value be end by invalid char or not timeout
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
int parse_timeout(char* str,unsigned int* timeout) {
	char *endptr = NULL;
	char c;

	
	if (NULL == str) 
		return IGMPSNP_RETURN_CODE_ERROR;
	
	c = str[0];
	if (c>='0'&&c<='9') {
		*timeout= strtoul(str,&endptr,10);
		if('\0' != endptr[0]) {
			return IGMPSNP_RETURN_CODE_ERROR; /*timeout value be end by invalid char.*/
		}
		return IGMPSNP_RETURN_CODE_OK;	
	}
	else {
		return IGMPSNP_RETURN_CODE_ERROR; /*not timeout; for Example ,enter '.',and so on ,some special char.*/
	}
}
/*******************************************************************************
 * show_igmp_snp_timer
 *
 * DESCRIPTION:
 *   		show all igmp timer value
 *
 * INPUTS:
 *		vty - vty terminal 		
 *		vlanlife -current vlan lifetime
 *		grouplife - current group lifetime
 *		robust - current robust value
 *		queryinterval - current query time
 *		respinterval - current response time
 *		hosttime - current host time
 *
 * OUTPUTS:
 *    	
 *
 * RETURNS:
 *
 *
 * COMMENTS:
 *     
 **
 ********************************************************************************/
int show_igmp_snp_timer
(
	struct vty  *vty,
	unsigned int vlanlife,
	unsigned int grouplife,
	unsigned int robust,
	unsigned int queryinterval,
	unsigned int respinterval,
	unsigned int hosttime
)
{
	vty_out(vty,"\n");
	vty_out(vty,"========  =========  ==============  =============  ======  ========\r\n");
	vty_out(vty,"VLANLIFE  GROUPLIFE  QUERY-INTERVAL  RESP-INTERVAL  ROBUST  HOSTLIFE\n");
	vty_out(vty,"%-8d  %-9d  %-14d  %-13d  %-6d  %-8d",vlanlife,\
															grouplife, \
															queryinterval,\
															respinterval,\
															robust,\
															hosttime);
	vty_out(vty,"\n========  =========  ==============  =============  ======  ========\r\n");
}

/*******************************************************************************
 * show_igmpsnp_vlan_member_info
 *
 * DESCRIPTION:
 *   		show igmpsnooping vlan membership with	in slot&port number.
 *
 * INPUTS:
 *		vty - vty terminal
 *		product_id - product id definition as enum product_id_e
 *		portmbrBmp0 - uint bitmap
 *		portmbrBmp1 - uint bitmap 	
 *
 * OUTPUTS:
 *    	
 *
 * RETURNS:
 *
 *
 * COMMENTS:
 *      
 **
 ********************************************************************************/
int show_igmpsnp_vlan_member_info
(
	struct vty *vty,
	unsigned int product_id,
	PORT_MEMBER_BMP untagBmp,
	PORT_MEMBER_BMP tagBmp
)
{
	unsigned int i,count = 0;
	unsigned int slot = 0,port = 0;
    unsigned int tmpVal[2];

	memset(tmpVal,0,sizeof(tmpVal));
	if(is_distributed == DISTRIBUTED_SYSTEM)
	{
		slot = igmp_dist_slot;
		/*
		vty_out(vty,"\nuntagBmp.portMbr[0] 0x%x \n",untagBmp.portMbr[0]);
		vty_out(vty,"\nuntagBmp.portMbr[1] 0x%x \n",untagBmp.portMbr[1]);
		vty_out(vty,"\ntagBmp.portMbr[0] 0x%x \n",tagBmp.portMbr[0]);
		vty_out(vty,"\ntagBmp.portMbr[1] 0x%x \n",tagBmp.portMbr[1]);
		*/
		/* untag port */
    	for (i=0;i<64;i++)
		{
			port = i+1;
    		tmpVal[i/32] = (1<<(i%32));
    		if((untagBmp.portMbr[i/32]) & tmpVal[i/32]) 
			{				
    			if(count && (0 == count % 4)) 
				{
					vty_out(vty,"\n%-32s"," ");

    				/*vty_out(vty,"\n%-9s"," ");*/
    			}
    			vty_out(vty,"%s%d/%d(u)",(count%4) ? ",":"",slot,port);
    			count++;
    		}
    	}	

		/* tag port */
    	memset(tmpVal,0,sizeof(tmpVal));		
    	for (i=0;i<64;i++)
		{
			port = i+1;
    		tmpVal[i/32] = (1<<(i%32));
    		if((tagBmp.portMbr[i/32]) & tmpVal[i/32]) 
			{				
    			if(count && (0 == count % 4)) 
				{
					vty_out(vty,"\n%-32s"," ");

    				/*vty_out(vty,"\n%-9s"," ");*/
    			}
    			vty_out(vty,"%s%d/%d(t)",(count%4) ? ",":"",slot,port);
    			count++;
    		}
    	}	
    	return 0;	
	}
	else
	{
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
    				(PRODUCT_ID_AU2K_TCAT == product_id)){
    			slot = 1;
    			port = i;
    		}
    		
    		tmpVal[i/32] = (1<<(i%32));
    		if((untagBmp.portMbr[i/32]) & tmpVal[i/32]) {				
    			if(count && (0 == count % 4)) {
    				vty_out(vty,"\n%-9s"," ");
    			}
    			if(PRODUCT_ID_AX7K_I == product_id) {
    				vty_out(vty,"%scscd%d(u)",(count%4) ? ",":"",port-1);
    			}
    			else {
    				vty_out(vty,"%s%d/%d(u)",(count%4) ? ",":"",slot,port);
    			}
    			count++;
    		}
    	}
    	slot = 0;
    	port = 0;
    	memset(tmpVal,0,sizeof(tmpVal));
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
    				(PRODUCT_ID_AU3K_BCM== product_id) ||
    				(PRODUCT_ID_AU3K_BCAT == product_id) ||
    				(PRODUCT_ID_AU2K_TCAT == product_id)) {
    			slot = 1;
    			port = i;
    		}
    		
    		tmpVal[i/32] = (1<<(i%32));
    		if((tagBmp.portMbr[i/32]) & tmpVal[i/32]) {			
    			if(count && (0 == count % 4)) {
    				vty_out(vty,"\n%-9s"," ");
    			}
    			if(PRODUCT_ID_AX7K_I == product_id) {
    				vty_out(vty,"%scscd%d(t)",(count%4) ? ",":"",port-1);
    			}
    			else {
    				vty_out(vty,"%s%d/%d(t)",(count%4) ? ",":"",slot,port);
    			}
    			count++;
    		}
    	}
	}
	return 0;
}

struct cmd_node igmp_node_distributed = 
{
	IGMP_NODE_DISTRIBUTED,
	"%s(config-igmp-sw-board)# "
};

struct cmd_node igmp_vlan_node = 
{
	IGMP_VLAN_NODE,
	"%s(config-igmp-sw-board-vlan)# "
};

/***igmp snooping config CMD executed on VLAN node**/
/***************************************************/
struct cmd_node igmp_snp_node = 
{
	IGMP_SNP_NODE,
	" "
};


/* add for distributed igmp */
DEFUN(config_igmp_sw_board_cmd_func,
	  config_igmp_sw_board_cmd,
	  "config igmp-snooping sw-board <1-16>",
	  CONFIG_STR
	  "Configure igmp snooping of switch-board\n"
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
	if (ACL_RETURN_CODE_ERROR== ret) {
		vty_out(vty,"%% Illegal slot number!\n");
		return CMD_WARNING;
	}

	if((dist_slot < 1)||(dist_slot > 16))
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
			vty_out(vty, "<<========== Config igmp switch-board slot: %d =========>>\n",dist_slot);
            dcli_dbus_connection_igmp = dbus_connection_dcli[dist_slot]->dcli_dbus_connection;			

			/*vty_out(vty,"Enter inner CMD node...\n");*/
			if(CONFIG_NODE == vty->node)
			{
				vty->node = IGMP_NODE_DISTRIBUTED;
				nodeSave = dist_slot;				
				vty->index = (void*)nodeSave;/*when not add & before vlanId, the Vty enter <config-line> CMD Node.*/
                /* add dist slot for vlan display */
				igmp_dist_slot = dist_slot;
			}
			else{
				vty_out (vty, "Terminal mode change must under configure mode!\n", VTY_NEWLINE);
				return CMD_WARNING;
			}
		}
    }
	return CMD_SUCCESS;
}

/*********************************************
*	show igmp snp config params
*	including:	vlan lifetime
*				group lifetime
*				robust variable
*				query interval
*				response interval
*
*	CMD node: config Node
*************************************************/
DEFUN(show_igmp_snp_get_state_cmd_func,
	show_igmp_snp_get_state_cmd,
	"show igmp-snooping state",
	SHOW_STR
	IGMP_SNOOPING_STR
	"Show igmp snooping global config state\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int state = 0;
	unsigned int ismld = 0;
    	
	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,    \
										IGMP_DBUS_OBJPATH,    \
										IGMP_DBUS_INTERFACE,    \
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_SHOW_STATE);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ismld,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_igmp,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		vty_out(vty,"Failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&state,
								DBUS_TYPE_INVALID)) {
		vty_out(vty,"\n===============================\n");
		vty_out(vty,"IGMP snooping config: %s \n",state ? "Enable":"Disable");			
		vty_out(vty,"===============================\n");			

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
* Enter vlan config mode to configure 
* special vlan.
* Params:
*		vlan ID:	 <2-4093>
* Usage: config vlan <2-4093>
************************************/

DEFUN(config_igmp_layer2_vlan_func,
	config_igmp_layer2_vlan_cmd,
	"config igmp-vlan <2-4093>",
	CONFIG_STR
	"Config layer 2 vlan entity\n"
	"Specify vlan id for vlan entity\n"
)
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
	if (4095 == vlanId) {
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
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_igmp,query,-1, &err);
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
			if(IGMP_NODE_DISTRIBUTED == vty->node) {
				vty->node = IGMP_VLAN_NODE;
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


/**************************************************
* no igmp snooping Debug (all|error|warning|debug|event)
*	
*	cmd Node : config Node
*
***************************************************/
DEFUN(config_igmp_snp_no_debug_cmd_func,
	config_igmp_snp_no_debug_cmd,
	"no debug igmp-snooping (all|error|warning|debug|event)",
	"Disable specific function \n"
	DEBUG_STR
	IGMP_SNOOPING_STR
	"Debug igmp-snooping all\n"
	"Debug igmp-snooping error\n"
	"Debug igmp-snooping warning\n"
	"Debug igmp-snooping debug\n"
	"Debug igmp-snooping event\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret;
	unsigned int flag = 0;
	unsigned char IsSupport = 0;
	
	if(argc > 1) {
		vty_out(vty,"%% Command parameters number error!\n");
		return CMD_WARNING;
	}

	dcli_get_product_igmp_function(vty, &IsSupport);
	if(1 == IsSupport){
		vty_out(vty,"%% Product not support this function!\n");
		return CMD_WARNING;
	}
	
	if(0 == strncmp(argv[0], "all", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ALL;
	}		
	else if(0 == strncmp(argv[0],"error", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ERR;
	}
	else if (0 == strncmp(argv[0],"warning", strlen(argv[0]))) {
	    flag = DCLI_DEBUG_FLAG_WAR;
	}
	else if (0 == strncmp(argv[0],"debug", strlen(argv[0]))) {
	    flag = DCLI_DEBUG_FLAG_DBG;
	}
	else if (0 == strncmp(argv[0],"event", strlen(argv[0]))) {
	    flag = DCLI_DEBUG_FLAG_EVT;
	}
	else {
		vty_out(vty,"%% Command parameter %s error!\n", argv[0]);
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(
								IGMP_DBUS_BUSNAME,    \
								IGMP_DBUS_OBJPATH,    \
								IGMP_DBUS_INTERFACE,  \
								IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEBUG_OFF);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&flag,
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
		if (IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL == ret){
			vty_out(vty,"%% Error:igmp snooping not enabled!\n");
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

/**************************************************
* igmp snooping Debug (all|error|packet|warning|debug|event)
*	
*	cmd Node : config Node
*
***************************************************/
DEFUN(config_igmp_snp_debug_cmd_func,
	config_igmp_snp_debug_cmd,
	"debug igmp-snooping (all|error|warning|debug|event)",
	DEBUG_STR
	IGMP_SNOOPING_STR
	"Debug igmp-snooping all\n"
	"Debug igmp-snooping error\n"
	"Debug igmp-snooping warning\n"
	"Debug igmp-snooping debug\n"
	"Debug igmp-snooping event\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret;
	unsigned int flag = 0;
	unsigned char IsSupport = 0;
	
	if(argc > 1) {
		vty_out(vty,"%% Command parameters number error!\n");
		return CMD_WARNING;
	}

	dcli_get_product_igmp_function(vty, &IsSupport);
	if(1 == IsSupport){
		vty_out(vty,"%% Product not support this function!\n");
		return CMD_WARNING;
	}
	
	if(0 == strncmp(argv[0], "all", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ALL;
	}		
	else if(0 == strncmp(argv[0],"error", strlen(argv[0]))) {
		flag = DCLI_DEBUG_FLAG_ERR;
	}
	else if (0 == strncmp(argv[0],"warning", strlen(argv[0]))) {
	    flag = DCLI_DEBUG_FLAG_WAR;
	}
	else if (0 == strncmp(argv[0],"debug", strlen(argv[0]))) {
	    flag = DCLI_DEBUG_FLAG_DBG;
	}
	else if (0 == strncmp(argv[0],"event", strlen(argv[0]))) {
	    flag = DCLI_DEBUG_FLAG_EVT;
	}
	else {
		vty_out(vty,"%% Command parameter %s error!\n", argv[0]);
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(
								IGMP_DBUS_BUSNAME,    \
								IGMP_DBUS_OBJPATH,    \
								IGMP_DBUS_INTERFACE,  \
								IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEBUG_ON);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&flag,
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
		if (IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL == ret){
			vty_out(vty,"%% Error:igmp snooping not enabled!\n");
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

/**************************************************
*  debug igmp-snooping packet (all|receive|send)
*	
*	cmd Node : config Node
*
***************************************************/
DEFUN(debug_igmp_snp_pkt_info,
		debug_igmp_snp_pkt_info_cmd,
		"debug igmp-snooping packet (all|receive|send)",
		DEBUG_STR
		MODULE_DEBUG_STR(igmp-snooping)
		MODULE_DEBUG_STR(packet)
		MODULE_DEBUG_LEVEL_STR(packet, all)
		MODULE_DEBUG_LEVEL_STR(packet, receive)
		MODULE_DEBUG_LEVEL_STR(packet, send)
		)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	unsigned int ret = 0;
	unsigned int flag = 0;
	unsigned char IsSupport = 0;
	
	if(argc > 1)
	{
		vty_out(vty, "%% Command parameters number error!\n");
		return CMD_WARNING;
	}

	dcli_get_product_igmp_function(vty, &IsSupport);
	if(1 == IsSupport){
		vty_out(vty,"%% Product not support this function!\n");
		return CMD_WARNING;
	}
	
	if(0 == strncmp(argv[0], "all", strlen(argv[0])))
	{
		flag = DCLI_DEBUG_FLAG_PKT_ALL;
	}		
	else if(0 == strncmp(argv[0], "receive", strlen(argv[0])))
	{
		flag = DCLI_DEBUG_FLAG_PKT_REV;
	}
	else if(0 == strncmp(argv[0], "send", strlen(argv[0])))
	{
		flag = DCLI_DEBUG_FLAG_PKT_SED;
	}
	else {
		vty_out(vty,"%% Command parameter %s error!\n", argv[0]);
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,    \
										IGMP_DBUS_OBJPATH,    \
										IGMP_DBUS_INTERFACE,  \
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEBUG_ON);	
    dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &flag,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection, query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
							  DBUS_TYPE_UINT32, &ret,
							  DBUS_TYPE_INVALID))
	{
		/*vty_out(vty," re_value:%d",re_value);				
		//vty_out(vty," return-value:%d\n",ret);*/
	}
	else
	{
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err))
		{
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/*The no debug npd function*/

DEFUN(no_debug_igmp_snp_pkt_info,
		no_debug_igmp_snp_pkt_info_cmd,
		"no debug igmp-snooping packet (all|receive|send)",
		NO_STR
		NODEBUG_STR
		MODULE_DEBUG_STR(igmp-snooping)
		MODULE_DEBUG_STR(packet)
		MODULE_DEBUG_LEVEL_STR(packet, all)
		MODULE_DEBUG_LEVEL_STR(packet, receive)
		MODULE_DEBUG_LEVEL_STR(packet, send)
		)
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int ret = 0;
	unsigned int flag = 0;
	unsigned char IsSupport = 0;
	
	if(argc > 1)
	{
		vty_out(vty,"%% Command parameters number error!\n");
		return CMD_WARNING;
	}

	dcli_get_product_igmp_function(vty, &IsSupport);
	if(1 == IsSupport){
		vty_out(vty,"%% Product not support this function!\n");
		return CMD_WARNING;
	}
	
	if(0 == strncmp(argv[0], "all", strlen(argv[0])))
	{
		flag = DCLI_DEBUG_FLAG_PKT_ALL;
	}
	else if(0 == strncmp(argv[0], "receive", strlen(argv[0])))
	{
		flag = DCLI_DEBUG_FLAG_PKT_REV;
	}
	else if(0 == strncmp(argv[0], "send", strlen(argv[0])))
	{
		flag = DCLI_DEBUG_FLAG_PKT_SED;
	}
	else {
		vty_out(vty,"%% Command parameter %s error!\n",argv[0]);
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,    \
										IGMP_DBUS_OBJPATH,    \
										IGMP_DBUS_INTERFACE,  \
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEBUG_OFF);	
    dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &flag,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args(reply, &err,
							  DBUS_TYPE_UINT32, &ret,
							  DBUS_TYPE_INVALID))
	{
		/*vty_out(vty," re_value:%d",re_value);				
		//vty_out(vty," return-value:%d\n",ret);*/
	}
	else
	{		
		vty_out(vty,"Failed get args.\n");		
		if (dbus_error_is_set(&err))
		{
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/**********************************
 *	igmp snooping enable/disable 
 *	complete ONLY by igmp_snp Protocol
 *
 *  CMD Node : config Node
**********************************/
DEFUN(config_igmp_snp_enable_cmd_func,
	config_igmp_snp_enable_cmd,
	"config igmp-snooping (enable|disable)",
	CONFIG_STR
	IGMP_SNOOPING_STR
	"Enable igmp snooping functionality\n"
	"Disable igmp snooping functionality\n"
	)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned char status=0,isEnable = 0,ismldt = 0,ismldq = 0,devchk = 0,IsSupport = 0;
	int ret = 0,op_ret = 0;
	
	if(argc > 1) {
		vty_out(vty,"%% Command without so many parameters!\n");
		return CMD_WARNING;
	}

	dcli_get_product_igmp_function(vty, &IsSupport);
	if(1 == IsSupport){
		vty_out(vty,"%% Product not support this function!\n");
		return CMD_WARNING;
	}
	
	if(strncmp(argv[0],"enable",strlen(argv[0]))==0){
		isEnable = 1;
	}
	else if (strncmp(argv[0],"disable",strlen(argv[0]))==0){
		isEnable = 0;
	}
	else{
		vty_out(vty,"%% Bad parameter %s!\n",argv[0]);
		return CMD_WARNING;
	}
	ret = dcli_igmp_snp_check_status(&status, ismldq, &devchk);
	if(IGMPSNP_RETURN_CODE_OK == ret){
		if(TURE_5612E == devchk){
			vty_out(vty, "%% Error dev 5612e not support igmp snooping!\n");
			return CMD_WARNING;
		}
		
		if(isEnable == status) {
			vty_out(vty, "%% Error igmp snooping already %s!\n", isEnable ? "enabled":"disabled");
			return CMD_WARNING;
		}
	}
	else {
		vty_out(vty, "%% Check igmp snooping state error %d!\n",ret);
		return CMD_SUCCESS;
	}

	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,   \
										IGMP_DBUS_OBJPATH,    \
										IGMP_DBUS_INTERFACE,   \
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_ENABLE);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&isEnable,
							DBUS_TYPE_BYTE,&ismldt,
							DBUS_TYPE_INVALID);


	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_igmp,query,-1, &err);

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
		if(IGMPSNP_RETURN_CODE_OK == ret)
        {
            /* set ok */
        }
		else if(IGMPSNP_RETURN_CODE_ERROR_SW == ret)
		{
			vty_out(vty,"%% MLD snooping is enable, can not enable igmp !\n");
			return CMD_WARNING;			
		}
		else if(IGMPSNP_RETURN_CODE_ALREADY_SET == ret)
		{
			vty_out(vty,"%% %s IGMP snooping is already set!\n", isEnable ? "Enable":"Disable");
			return CMD_WARNING;				
		}
		else
		{
			vty_out(vty,"%% %s igmp snooping error %d!", isEnable ? "Enable":"Disable", ret);
			return CMD_WARNING;
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


/*********************************************
*	set igmp snp time params
*	including:	vlan lifetime
*				group lifetime
*				robust variable
*				query interval
*				response interval
*
*	CMD node: config Node
*************************************************/
DEFUN(config_igmp_snp_timer_cmd_func,
	config_igmp_snp_timer_cmd,
	"config igmp-snooping (vlan-lifetime|group-lifetime|robust|query-interval|response-interval) TIMEOUT",
	CONFIG_STR
	IGMP_SNOOPING_STR
	"Config vlan's multicast-group lifetime: valid range [10000-100000]\n"
	"Config multicast-group lifetime:valid range [1000-50000]\n"
	"Config multicast-group member host robust variables:valid range [1-100]\n"
	"Config multicast-group query interval:valid range [1000-10000]\n"
	"Config multicast-group response interval:valid range [100-1000]\n"
	"Timeout value in unit of 1/10 second,except for robust need integral number value\n"
)
{

	DBusMessage *query, *reply;
	DBusError err;

	unsigned int timer_type;
	unsigned int timeout = 0;
	int ret;
	unsigned char IsSupport = 0;

	dcli_get_product_igmp_function(vty, &IsSupport);
	if(1 == IsSupport){
		vty_out(vty,"%% Product not support this function!\n");
		return CMD_WARNING;
	}
	
	if(!strncmp(argv[0],"vlan-lifetime",strlen(argv[0]))){
		timer_type = VLAN_LIFETIME;
	}
	else if(!strncmp(argv[0],"group-lifetime",strlen(argv[0]))){
		timer_type = GROUP_LIFETIME;
	}
	else if(!strncmp(argv[0],"robust",strlen(argv[0]))){
		timer_type = ROBUST_VARIABLE;
	}
	else if(!strncmp(argv[0],"query-interval",strlen(argv[0]))){
		timer_type = QUERY_INTERVAL;
	}
	else if(!strncmp(argv[0],"response-interval",strlen(argv[0]))){
		timer_type = RESP_INTERVAL;
	}
	else {
		vty_out(vty,"%% Unknown command format!\n");
		return CMD_WARNING;
	}

	ret = parse_timeout((char *)argv[1],&timeout);
	if (IGMPSNP_RETURN_CODE_ERROR == ret) {
    	vty_out(vty,"%% Error:malformed TIMEOUT value!\n");
		return CMD_WARNING;
	}
	
	switch(timer_type){
		default:
			break;
		case VLAN_LIFETIME:
			if(timeout > 100000 || timeout <10000){
	    		vty_out(vty,"%% Valid vlan-lifetime value must be in range [10000-100000]\n");
				return CMD_WARNING;
			}
			break;
		case GROUP_LIFETIME:
			if(timeout > 50000 || timeout <1000){
	    		vty_out(vty,"%% Valid group-lifetime value must be in range [1000-50000]\n");
				return CMD_WARNING;
			}
			break;
		case ROBUST_VARIABLE:
			if(timeout > 100 || timeout <1){
	    		vty_out(vty,"%% Valid robust value must be in range [1-100]\n");
				return CMD_WARNING;
			}
			break;
		case QUERY_INTERVAL:
			if(timeout > 10000 || timeout <1000){
	    		vty_out(vty,"%% Valid query-interval value must be in range [1000-10000]\n");
				return CMD_WARNING;
			}
			break;
		case RESP_INTERVAL:
			if(timeout > 1000 || timeout <100){
	    		vty_out(vty,"%% Valid response-interval value must be in range [100-1000]\n");
				return CMD_WARNING;
			}
			break;
	}

	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,    \
										IGMP_DBUS_OBJPATH,    \
										IGMP_DBUS_INTERFACE,    \
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_SET_TIMER);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&timer_type,
							DBUS_TYPE_UINT32,&timeout,
							DBUS_TYPE_INVALID);


	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_igmp,query,-1, &err);

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
		if (IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL == ret){
			vty_out(vty,"%% Error:igmp snooping not enabled!\n");
		}
		else if (IGMPSNP_RETURN_CODE_OUT_RANGE == ret) {
			vty_out(vty,"%% Error:parameter value %d invalid!\n",timeout);
		}
		else if (IGMPSNP_RETURN_CODE_SAME_VALUE == ret) {
			vty_out(vty,"%% Error:value %d is same as the original, or has configured!\n", timeout);
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


/*********************************************
*	show igmp snp timer params
*	including:	vlan lifetime
*				group lifetime
*				robust variable
*				query interval
*				response interval
*
*	CMD node: config Node
*************************************************/
DEFUN(show_igmp_snp_timers_cmd_func,
	show_igmp_snp_timers_cmd,
	"show igmp-snooping time-interval",
	SHOW_STR
	IGMP_SNOOPING_STR
	"Show igmp snooping time interval configuration\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int vlanlife =0,grouplife =0,robust =0,query_interval =0,response_interval =0,rmxt_interval =0,hosttime =0;
	unsigned int ret = 0;
	unsigned char IsSupport = 0;

	dcli_get_product_igmp_function(vty, &IsSupport);
	if(1 == IsSupport){
		vty_out(vty,"%% Product not support this function!\n");
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,    \
										IGMP_DBUS_OBJPATH,    \
										IGMP_DBUS_INTERFACE,    \
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_SHOW_TIMER);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_igmp,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		vty_out(vty,"Failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_UINT32,&vlanlife,
								DBUS_TYPE_UINT32,&grouplife,
								DBUS_TYPE_UINT32,&robust,
								DBUS_TYPE_UINT32,&query_interval,
								DBUS_TYPE_UINT32,&response_interval,
								DBUS_TYPE_UINT32,&hosttime,
								DBUS_TYPE_INVALID)) {
		if(IGMPSNP_RETURN_CODE_OK == ret) {
			show_igmp_snp_timer(vty,\
								vlanlife,\
								grouplife,\
								robust,\
								query_interval,\
								response_interval,\
								hosttime);
		}/*error code ...*/
		else if (IGMPSNP_RETURN_CODE_ENABLE_GBL == ret){
			vty_out(vty,"%% Error:igmp snooping not enabled!\n");
		}
		else if (IGMPSNP_RETURN_CODE_GET_VLANLIFE_E == ret){
			vty_out(vty,"%% Get vlan lifetime error!\n");
		}
		else if (IGMPSNP_RETURN_CODE_GET_GROUPLIFE_E == ret){
			vty_out(vty,"%% Get group lifetime error!\n");
		}
		else if (IGMPSNP_RETURN_CODE_GET_HOSTLIFE_E == ret){
			vty_out(vty,"%% Get host lifetime error!\n");
		}
		else if (IGMPSNP_RETURN_CODE_GET_ROBUST_E == ret){
			vty_out(vty,"%% Get robust variable error!\n");
		}
		else if (IGMPSNP_RETURN_CODE_GET_QUREY_INTERVAL_E == ret){
			vty_out(vty,"%% Get query interval error!\n");
		}
		else if (IGMPSNP_RETURN_CODE_GET_RESP_INTERVAL_E == ret){
			vty_out(vty,"%% Get response interval error!\n");
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

/******************************************
 *	show mcgroup count
 *
 *	CMD: CONFIG Node
 *
 ****************************************/
DEFUN(igmp_snp_show_group_count_cmd_func,
		igmp_snp_show_group_count_cmd,
		"show igmp-snooping group-count",
		SHOW_STR
		IGMP_SNOOPING_STR
		"Show igmp snooping active group count on layer 2\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int groupcount = 0,ret;
	unsigned char IsSupport = 0;

	dcli_get_product_igmp_function(vty, &IsSupport);
	if(1 == IsSupport){
		vty_out(vty,"%% Product not support this function!\n");
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,   \
										IGMP_DBUS_OBJPATH,    \
										IGMP_DBUS_INTERFACE,   \
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_TOTAL_GROUP_COUNT_SHOW);
	dbus_error_init(&err);


	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_igmp,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		vty_out(vty,"Failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_UINT32,&groupcount,
								DBUS_TYPE_INVALID)) {
		if(IGMPSNP_RETURN_CODE_OK == ret) {
			if(0 == groupcount){
				vty_out(vty,"%% No group exist on layer 2!\n");
			}
			else{
				vty_out(vty,"\n====================\r\n");
				vty_out(vty,"MCGROUP_COUNT: %-7d\r\n",groupcount);
				vty_out(vty,"======================\r\n");
			}
		}
		else if (IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL == ret){
			vty_out(vty,"%% Error:igmp snooping not enabled!\n");
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


/******************************************
 *	show igmp-snooping group-list vlan <1-4094>
 *
 *	CMD: CONFIG Node
 *
 ****************************************/
DEFUN(igmp_snp_show_vlan_group_list_cmd_func,
		igmp_snp_show_vlan_group_list_cmd,
		"show igmp-snooping group-list vlan <1-4094>",
		SHOW_STR
		IGMP_SNOOPING_STR
		"Show igmp-snooping active group list in the vlan on layer 2\n"
		"Vlan entity\n"
		"Specify vlan id for vlan entity, valid range <1-4094>\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	int				ret = 0;
	unsigned short	vlanId = 0;

	ret = parse_vlan_no((char*)argv[0], &vlanId);
	if (NPD_FAIL == ret)
	{
    	vty_out(vty, "%% Error: vlan id illegal!\n");
		return CMD_WARNING;
	}
	if (4095 == vlanId)
	{
		vty_out(vty, "%% Error: Reserved vlan for Layer3 interface of EthPort!\n");
		return CMD_WARNING;
	}
	if (0 == vlanId || vlanId > 4095)
	{
		vty_out(vty, "%% Error: vlan id is out of valid range <1-4094>!\n");
		return CMD_WARNING;
	}

	dcli_show_igmp_snp_mcgroup_list(vty, vlanId);

	return CMD_SUCCESS;
}

#if 0

/******************************************
 *	show mcgroup count
 *
 *	CMD: VLAN Node
 *
 ****************************************/
DEFUN(igmp_snp_show_group_count_cmd_func,
		igmp_snp_show_group_count_cmd,
		"show igmp_snooping group_count ",
		"Display IGMP Snooping protocol information\n"
		"IGMP snooping protocol\n"
		"Active group count\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short vlanId;
	unsigned int groupcount = 0,ret;
	vlanId = (unsigned short)vty->index;
	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,   \
										IGMP_DBUS_OBJPATH,    \
										IGMP_DBUS_INTERFACE,   \
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_GROUP_COUNT_SHOW);
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
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_UINT32,&groupcount,
								DBUS_TYPE_INVALID)) {
		if(0 == ret) {
			if(0 == groupcount){
				vty_out(vty,"No group exist in this vlan.\n");
			}
			else{
				vty_out(vty,"\n=======	=============\r\n");
				vty_out(vty,"VLAN_ID	MCGROUP_COUNT\r\n");
				vty_out(vty,"%-7d	%d\r\n",vlanId,groupcount);
				vty_out(vty,"=======	===============\r\n");
			}
		}
		else if (IGMP_SNOOP_ERR_NOT_ENABLE_GLB == ret){
			vty_out(vty,"%% Bad Param:IGMP Snooping NOT enabled global.\n");
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

/*********************************************
*	delete mcgroup : 
*				specific by D-Ip, 
*				all mcgroup
*	
*	CMD Node : vlan Node
*
**********************************************/

DEFUN(igmp_snp_delete_mcgroup_cmd_func,
	igmp_snp_delete_mcgroup_cmd,
	"delete mcgroup ip ( A.B.C.D|any )",
	"Delete a spscific mcgroup interface\n"
	"Multicast group interface \n"
	"Ip address \n"
	"Multicast IP\n"
	"Any mcgroup IP\n"
	)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short vid = 0;
	unsigned long  ipAddr = 0;
	int ret;

	if(argc > 1) {
		vty_out(vty,"command parameters error\n");
		return CMD_FAILURE;
	}
	
    if(strncmp(argv[0],"any",1)==0){
	  	ipAddr=0;
	}
	else {	
		ipAddr = dcli_ip2ulong((char*)argv[0]);			
	}

	vid = (unsigned short)vty->index;

	/*TODO ON IGMP */
	/**************/
	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,    \
										IGMP_DBUS_OBJPATH,    \
										IGMP_DBUS_INTERFACE,    \
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCGROUP);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_UINT16,&vid,
							DBUS_TYPE_UINT32,&ipAddr,
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

/*********************************************
*	delete mcvlan : 
*		result is same to delete all mcgroup.
*	*!* igmp_snp_delvlan
*	CMD Node : VLAN Node
*
**********************************************/

DEFUN(igmp_snp_delete_mcgroup_vlan_cmd_func,
	igmp_snp_delete_mcgroup_vlan_cmd,
	"delete mcgroup-vlan",				//"delete mcgroup-vlan (<1-4094>|any)",
	"Delete a spscific mcgroup-vlan \n"
	"Multicast group-vlan \n"
										//"Vlan Id,valid range [1-4094] \n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short vid = 0;	
	int ret;

	if(argc > 1) {
		vty_out(vty,"command parameters error\n");
		return CMD_FAILURE;
	}
	/*
    if(strcmp(argv[0],"any")==0){
		vid = 0;
	}
	else {
		ret = parse_vlan_no((unsigned char *)argv[0],&vid);
		if (IGMP_FAIL == ret) {
	    	vty_out(vty,"Unknow port number format.\n");
			return CMD_FAILURE;
		}
	}
	*/ 
	vid = (unsigned short)vty->index;
	/*TODO ON NPD*/
	/* ... */
	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,    \
										IGMP_DBUS_OBJPATH,    \
										IGMP_DBUS_INTERFACE,    \
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCGROUP_VLAN_ONE);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_UINT16,&vid,
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
/*********************************************
*	delete all mcvlan : 
*		each group in all mcgroup vlans'll be delete.
*	*!*igmp_del_all_vlan
*	CMD Node : config Node
*
**********************************************/

DEFUN(igmp_snp_delete_mcgroup_vlan_all_cmd_func,
	igmp_snp_delete_mcgroup_vlan_all_cmd,
	"delete mcgroup-vlan list",				//"delete mcgroup-vlan (<1-4094>|any)",
	"Delete a specific mcgroup-vlan \n"
	"Multicast group-vlan \n"
	"All Mcgroup Vlans \n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short vid = 0;	
	int ret;

	if(argc > 0) {
		vty_out(vty,"command parameters error\n");
		return CMD_FAILURE;
	}
	/*TODO ON NPD*/
	/* ... */
	
	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,    \
										IGMP_DBUS_OBJPATH,    \
										IGMP_DBUS_INTERFACE,    \
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_DEL_MCGROUP_VLAN_ALL);
	dbus_error_init(&err);

	/*no params*/
	/*
    dbus_message_append_args(query,
							DBUS_TYPE_UINT16,&vid,
							DBUS_TYPE_INVALID);
	*/

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
		/*error code*/
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

#endif
/***************************************
*	show igmp vlan list
*
*
*	CMD :config Node
**************************************/

DEFUN(igmp_snp_show_vlan_list_cmd_func,
		igmp_snp_show_vlan_list_cmd,
		"show igmp-snooping vlan-list",
		SHOW_STR
		IGMP_SNOOPING_STR
		"Show igmp snooping enabled vlan list\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int vlan_Cont = 0;
	unsigned short  vlanId = 0;
	PORT_MEMBER_BMP	untagBmp ,tagBmp;
	unsigned int 	product_id = PRODUCT_ID_NONE,j,ret;

    memset(&untagBmp,0,sizeof(PORT_MEMBER_BMP));
	memset(&tagBmp,0,sizeof(PORT_MEMBER_BMP));
	dbus_error_init(&err);

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,			\
										NPD_DBUS_VLAN_OBJPATH,		\
										NPD_DBUS_VLAN_INTERFACE,		\
										NPD_DBUS_VLAN_METHOD_IGMP_SNP_VLAN_LIST_SHOW_V1);
	/*No params*/
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_igmp,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		vty_out(vty,"Failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);

	dbus_message_iter_get_basic(&iter,&ret);
	if(IGMPSNP_RETURN_CODE_OK == ret){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&vlan_Cont);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&product_id);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		vty_out(vty,"========================================================================\n");
		vty_out(vty,"%-7s  %-40s\n","VLAN ID","                      PORT MEMBER LIST");
		vty_out(vty,"=======  ===============================================================\n");

		for (j = 0; j < vlan_Cont; j++) {
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&vlanId);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&untagBmp.portMbr[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&untagBmp.portMbr[1]);

			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&tagBmp.portMbr[0]);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&tagBmp.portMbr[1]);

			dbus_message_iter_next(&iter_array); 


			
			if(NPD_PORT_L3INTF_VLAN_ID != vlanId) {
				vty_out(vty,"%-30d  ",vlanId);				
				if ((0 != untagBmp.portMbr[0]) ||
					(0 != untagBmp.portMbr[1]) ||
					(0 != tagBmp.portMbr[0]) ||
					(0 != tagBmp.portMbr[1]))
					show_igmpsnp_vlan_member_info(vty,product_id,untagBmp,tagBmp);
				else 
					vty_out(vty,"%-45s","no port enabled igmp snooping!\n");
				vty_out(vty,"\n");
			}
		}
		vty_out(vty,"========================================================================\n");
	}
	else if (IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL == ret){
			vty_out(vty,"%% Error:igmp snooping service not enabled!\n");
	}
	else if(IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST == ret) {
		vty_out(vty,"%% Error:no vaild vlan exist.\n");
	}
	else if (IGMPSNP_RETURN_CODE_ERROR_HW == ret) {
		vty_out(vty,"%% Read vlan entry from hardware error!\n");	
	}
	else if (IGMPSNP_RETURN_CODE_NOTENABLE_VLAN == ret){
		vty_out(vty,"%% igmp snooping service not enabled in vlan!\n");
	}
	else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret)
		vty_out(vty,"%% Product not support this function!\n");
	dbus_message_unref(reply);
	return CMD_SUCCESS;
	
}

/***************************************
*	show igmp vlan count
*
*
*	CMD :config Node
**************************************/

DEFUN(igmp_snp_show_vlan_count_cmd_func,
		igmp_snp_show_vlan_count_cmd,
		"show igmp-snooping vlan-count",
		SHOW_STR
		IGMP_SNOOPING_STR
		"Show igmp snooping configured vlan count\n "
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int vlancount = 0,ret;
	
	query = dbus_message_new_method_call(
						NPD_DBUS_BUSNAME,			\
						NPD_DBUS_VLAN_OBJPATH,		\
						NPD_DBUS_VLAN_INTERFACE,		\
						NPD_DBUS_VLAN_METHOD_IGMP_SNP_VLAN_COUNT);
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_igmp,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		vty_out(vty,"Failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_UINT32,&vlancount,
								DBUS_TYPE_INVALID)) {
		if(IGMPSNP_RETURN_CODE_OK == ret) {
			vty_out(vty,"\n====================\r\n");
			vty_out(vty,"IGMP_VLAN COUNT: %d\n",vlancount);
			vty_out(vty,"====================\r\n");
		}
		else if (IGMPSNP_RETURN_CODE_ENABLE_GBL == ret){
			vty_out(vty,"%% Error:igmp snooping service not enabled!\n");
		}
		else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret)
			vty_out(vty,"%% Product not support this function!\n");
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


/******************************************
 * show multicast-group route-port
 * CMD :show multicast-group route-port
 *
 * CMD node: VLAN node
 *
 *****************************************/
 DEFUN(show_igmp_snp_multicast_route_port_cmd_func,
	show_igmp_snp_multicast_route_port_cmd,
	"show multicast-group route-port",
	SHOW_STR
	"Multicast group on layer 2\n"
	"Show multicast route-port in this vlan\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;

	char *vlanName;
	char *vlanName2;
	unsigned char status = 0;	
	unsigned int ret = 0;
	unsigned int mbrCount = 0;
	unsigned int product_id = 0; 
	unsigned int vlanStat = 0;
	unsigned short vlanId = 0;
	unsigned int tmp_index = 0;
	PORT_MEMBER_BMP routeBmp;
    PORT_MEMBER_BMP untagBmp,tagBmp;
	unsigned int promisPortBmp[2] = {0};

	memset(&routeBmp,0,sizeof(PORT_MEMBER_BMP));
	memset(&tagBmp,0,sizeof(PORT_MEMBER_BMP));
	memset(&untagBmp,0,sizeof(PORT_MEMBER_BMP));

	if(argc > 0)
	{
		vty_out(vty, "%% Command with so many parameters!\n");
		return CMD_WARNING;
	}

	tmp_index = (unsigned int)vty->index;
	vlanId = (unsigned short)tmp_index;
	dcli_vlan_igmp_snp_status_get(vlanId, &status);
	if(0 == status){
		vty_out(vty,"%% Error:vlan %d not enable igmp snooping!\n", vlanId);
		return CMD_WARNING;
	}

	/* get the user profile route-port */
	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_IGMP_SNP_SHOW_ROUTE_PORT_V1);

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vlanId,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_igmp,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"Failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	if (dbus_message_get_args(  reply, &err,
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
								DBUS_TYPE_INVALID))
	{
		if (IGMPSNP_RETURN_CODE_OK == ret)
		{
			vty_out(vty, "Codes:  U - vlan link status is up,    D - vlan link status is down, \n");
			vty_out(vty, "        u - untagged port member,       t - tagged port member, \n");
			vty_out(vty, "        * - promiscuous mode port member\n");
			vty_out(vty, "\n");
			vty_out(vty, "USER PROFILE ROUTE-PORT:\n");
			vty_out(vty, "========================================================================\n");
			vty_out(vty, "%-7s  %-20s  %-40s\n","VLAN ID","VLAN NAME","ROUTERPORT MEMBER LIST");
			vty_out(vty, "=======  ====================  =========================================\n");

			vty_out(vty, "%-4d(%s)  ", vlanId, vlanStat ? "U":"D");
			vty_out(vty, "%-20s	 ", vlanName);
			if (0 != untagBmp.portMbr[0]||0 != untagBmp.portMbr[1]|| 0 != tagBmp.portMbr[0]|| 0 != tagBmp.portMbr[1])
			{
				/*show_vlan_member_slot_port(vty, product_id, untagBmp, tagBmp,promisPortBmp);*/
				show_igmpsnp_vlan_member_info(vty,product_id,untagBmp,tagBmp);				
			}
			else
			{
				vty_out(vty, "  %-40s", "No RoutePort member.");
			}
			vty_out(vty, "\n");
			vty_out(vty, "========================================================================\n");
		}
		else if ((IGMPSNP_RETURN_CODE_NO_SUCH_VLAN) == ret)
		{
			vty_out(vty, "%% Error: vlan id illegal.\n");
		}
		else if(IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST == ret)
		{
			vty_out(vty, "%% Error: vlan %d not exist.\n", vlanId);
		}
		else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret)
			vty_out(vty,"%% Product not support this function!\n");
	}
	else
	{
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	vlanName2 = (char*)malloc(ALIAS_NAME_SIZE + 1);
	memset(vlanName2, '\0', ALIAS_NAME_SIZE + 1);
	strncpy(vlanName2, vlanName, ALIAS_NAME_SIZE + 1);

	dbus_message_unref(reply);

	if (ret == IGMPSNP_RETURN_CODE_OK)
	{	/* get the running route-port, include the route-port from PIM's helle packet */
		ret = dcli_igmp_snp_show_running_routeport(vty, vlanId, &routeBmp);
		vty_out(vty, "\n");
		vty_out(vty, "RUNNING ROUTE-PORT:\n");
		if (ret == CMD_SUCCESS)
		{
			/* output the route-port informations */
			vty_out(vty, "========================================================================\n");
			vty_out(vty, "%-7s  %-20s  %-40s\n","VLAN ID","VLAN NAME","ROUTERPORT MEMBER LIST");
			vty_out(vty, "=======  ====================  =========================================\n");
		
			vty_out(vty, "%-4d(%s)  ", vlanId, vlanStat ? "U":"D");
			vty_out(vty, "%-20s  ", vlanName2);

			if ((0 != routeBmp.portMbr[0]) || (0 != routeBmp.portMbr[1]))
			{
				/* here show the route-port */
				ret = show_group_member_slot_port(vty, product_id, routeBmp);
			}
			else
			{
				vty_out(vty, "	%-40s", "No running RoutePort member.");
			}
			vty_out(vty, "\n");
			vty_out(vty, "========================================================================\n");
		}
		else if(IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST == ret)
		{
			vty_out(vty, "%% Error: vlan %d not exist.\n", vlanId);
		}
		else if(IGMPSNP_RETURN_CODE_NO_SUCH_VLAN == ret)
		{
			vty_out(vty, "%% Error: vlan id illegal.\n", vlanId);
		}
	}

	free(vlanName2);
	return  CMD_SUCCESS;
}


/******************************************
 * show igmp-snooping route-port vlan <1-4094>
 * CMD :show igmp-snooping route-port vlan <1-4094>
 *
 *	CMD: CONFIG Node
  *
 *****************************************/
 DEFUN(igmp_snp_show_vlan_router_port_cmd_func,
	igmp_snp_show_vlan_router_port_cmd,
	"show igmp-snooping route-port vlan <1-4094>",
	SHOW_STR
	IGMP_SNOOPING_STR
	"Show multicast route-port list in the vlan on layer 2\n"
	"Vlan entity\n"
	"Specify vlan id for vlan entity, valid range <1-4094>\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;

	char *vlanName;
	char *vlanName2;
	unsigned char status = 0;	
	unsigned int ret = 0;
	unsigned int mbrCount = 0;
	unsigned int product_id = 0; 
	unsigned int vlanStat = 0;
	PORT_MEMBER_BMP routeBmp;
	unsigned short vlanId = 0;
    PORT_MEMBER_BMP untagBmp,tagBmp;
	unsigned int promisPortBmp[2] = {0};
	memset(&routeBmp,0,sizeof(PORT_MEMBER_BMP));
	memset(&tagBmp,0,sizeof(PORT_MEMBER_BMP));
	memset(&untagBmp,0,sizeof(PORT_MEMBER_BMP));

	ret = parse_vlan_no((char*)argv[0], &vlanId);
	if (NPD_FAIL == ret)
	{
    	vty_out(vty, "%% Error: vlan id illegal!\n");
		return CMD_WARNING;
	}
	if (4095 == vlanId)
	{
		vty_out(vty, "%% Error: Reserved vlan for Layer3 interface of EthPort!\n");
		return CMD_WARNING;
	}
	if (0 == vlanId || vlanId > 4095)
	{
		vty_out(vty, "%% Error: vlan id is out of valid range <1-4094>!\n");
		return CMD_WARNING;
	}

	ret = dcli_vlan_igmp_snp_status_get(vlanId, &status);
	if (0 == status)
	{
		vty_out(vty, "%% Error:vlan %d not enable igmp snooping!\n", vlanId);
		return CMD_WARNING;
	}

	/* get the user profile route-port */
	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_IGMP_SNP_SHOW_ROUTE_PORT_V1);

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16, &vlanId,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_igmp, query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		vty_out(vty, "Failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	if (dbus_message_get_args(  reply, &err,
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
								DBUS_TYPE_INVALID))
	{
		if (IGMPSNP_RETURN_CODE_OK == ret)
		{
			vty_out(vty, "Codes:  U - vlan link status is up,   D - vlan link status is down, \n");
			vty_out(vty, "        u - untagged port member,      t - tagged port member, \n");
			vty_out(vty, "        * - promiscuous mode port member\n");
			vty_out(vty, "\n");
			vty_out(vty, "USER PROFILE ROUTE-PORT:\n");
			vty_out(vty, "========================================================================\n");
			vty_out(vty, "%-7s  %-20s  %-40s\n","VLAN ID","VLAN NAME","ROUTERPORT MEMBER LIST");
			vty_out(vty, "=======  ====================  =========================================\n");

			vty_out(vty, "%-4d(%s)  ", vlanId, vlanStat ? "U":"D");
			vty_out(vty, "%-20s	 ", vlanName);
			if (0 != untagBmp.portMbr[0]|| 0 != untagBmp.portMbr[1]|| 0 != tagBmp.portMbr[0]||0 != tagBmp.portMbr[1])
			{
				/*show_vlan_member_slot_port(vty, product_id, untagBmp, tagBmp,promisPortBmp);*/
				show_igmpsnp_vlan_member_info(vty,product_id,untagBmp,tagBmp);
			}
			else
			{
				vty_out(vty, "  %-40s", "No RoutePort member.");
			}
			vty_out(vty, "\n");
			vty_out(vty, "========================================================================\n");
		}
		else if (IGMPSNP_RETURN_CODE_NO_SUCH_VLAN == ret)
		{
			vty_out(vty, "%% Error:vlan id illegal.\n");
		}
		else if(IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST == ret)
		{
			vty_out(vty, "%% Error:vlan %d not exist.\n", vlanId);
		}
		else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret)
			vty_out(vty,"%% Product not support this function!\n");
	}
	else
	{
		vty_out(vty, "Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	vlanName2 = (char*)malloc(ALIAS_NAME_SIZE + 1);
	memset(vlanName2, '\0', ALIAS_NAME_SIZE + 1);
	strncpy(vlanName2, vlanName, ALIAS_NAME_SIZE + 1);

	dbus_message_unref(reply);

	if (ret == IGMPSNP_RETURN_CODE_OK)
	{	/* get the running route-port, include the route-port from PIM's helle packet */
		ret = dcli_igmp_snp_show_running_routeport(vty, vlanId, &(routeBmp.portMbr[0]));
		vty_out(vty, "\n");
		vty_out(vty, "RUNNING ROUTE-PORT:\n");
		
		
			/* output the route-port informations */
			vty_out(vty, "========================================================================\n");
			vty_out(vty, "%-7s  %-20s  %-40s\n","VLAN ID","VLAN NAME","ROUTERPORT MEMBER LIST");
			vty_out(vty, "=======  ====================  =========================================\n");
		
			vty_out(vty, "%-4d(%s)  ", vlanId, vlanStat ? "U":"D");
			vty_out(vty, "%-20s  ", vlanName2);

			if ((0 != routeBmp.portMbr[0]) || (0 != routeBmp.portMbr[1]))
			{
				/* here show the route-port */
				ret = show_group_member_slot_port(vty, product_id, routeBmp);
			}
			else
			{
				vty_out(vty, "	%-40s", "No running RoutePort member.");
			}
			vty_out(vty, "\n");
			vty_out(vty, "========================================================================\n");
		
		 if(IGMPSNP_RETURN_CODE_MC_VLAN_NOT_EXIST == ret)
		{
			vty_out(vty, "%% Error: multicast vlan %d not exist.\n", vlanId);
		}
		else if(IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL == ret)
		{
			vty_out(vty, "%% Error:igmp snoop is disable.\n", vlanId);
		}
	}

	free(vlanName2);
	return	CMD_SUCCESS;
}

/******************************************
 *	add/delete vlan to join igmp protocol
 *	dcli-->npd--->igmp  (by mng socket;DEV_EVNET)
 *			  |-->
 *	CMD:igmp-snooping (enable|disable)
 *
 *	config hw by npd first,and then 
 *	pass param to igmp snooping Protocol
 *
 *	CMD :VLAN node
*******************************************/
DEFUN(config_igmp_snp_npd_vlan_cmd_func,
	config_igmp_snp_npd_vlan_cmd,
	"config vlan igmp-snooping (enable|disable)",
	CONFIG_STR
	IGMP_SNOOPING_STR
	"Enable igmp snooping functionality on vlan\n"
	"Disable igmp snooping functionality on vlan\n"

												
)
{
	/*
	DBusMessage *query, *reply;
	DBusError err;
	*/
	unsigned int tmp_index = 0;
	unsigned short	vlanId = 0;
	unsigned char	enDis = FALSE;
	int ret;

	if(argc > 1)
	{
		vty_out(vty,"%% Command without so many parameters!\n");
		return CMD_WARNING;
	}

	/*get first param*/
	if(!strncmp(argv[0],"enable",strlen(argv[0]))) {
		enDis= TRUE;
	}
	else if(!strncmp(argv[0],"disable",strlen(argv[0]))) {
		enDis= FALSE;
	}
	else {
		vty_out(vty,"%% Error:command parameters error!\n");
		return CMD_WARNING;
	}

	tmp_index = (unsigned int)vty->index;
	vlanId = (unsigned short)tmp_index;
	/*vty_out(vty,"enDis = %d, vlan ID = %d.\r\n",enDis,vlanId);*/
	#if 0
	ret = dcli_igmp_snp_vlan_status_check(&status);
	if(1 == enDis){
		if(1 == status){
			vty_out(vty,"%% Bad parameter:IGMP Snoop already Enabled on Vlan %d !\n",vlanId);
			return CMD_FAILURE;
		}
	}
	else if(0 == enDis){
		if(0 == status){
			vty_out(vty,"%% Bad parameter:IGMP Snoop has NOT Enabled on VLan %d!\n",vlanId);
			return CMD_FAILURE;
		}
	}
	#endif
	ret = dcli_enable_disable_igmp_one_vlan(vty,vlanId,enDis);
	if (IGMPSNP_RETURN_CODE_OK == ret ) {
		/*vty_out(vty,"success\n"));*/
	}
	else if (IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL == ret){
		vty_out(vty,"%% Error:igmp snooping service not enabled!\n");
	}
	else if (IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST == ret){
		vty_out(vty,"%% Error:vlan %d not exists!\n",vlanId);
	}
	else if (IGMPSNP_RETURN_CODE_NOTENABLE_VLAN == ret){
		vty_out(vty,"%% Error:vlan %d not enabled!\n",vlanId);
	}
	else if (IGMPSNP_RETURN_CODE_HASENABLE_VLAN == ret){
		vty_out(vty,"%% Error:vlan %d already enabled!\n",vlanId);
	}
	else if (IGMPSNP_RETURN_CODE_ERROR_HW == ret){
		vty_out(vty,"%% Error:set asic Hw for vlan error!\n");
	}
	else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret)
		vty_out(vty,"%% Product not support this function!\n");
	
	return CMD_SUCCESS;

}

/*************************************************
 *	enable|disable port of specific vlan to(from) 
 *	IGMP snooping;
 *	config hw by npd first,and then 
 *	pass param to igmp snooping Protocol
 *
 *	CMD : VLAN node
*************************************************/
DEFUN(config_igmp_snp_npd_port_cmd_func,
	config_igmp_snp_npd_port_cmd,
	"config igmp-snooping PORTNO (enable|disable)",
	CONFIG_STR
	IGMP_SNOOPING_STR
	"Config igmp snooping functionality on vlan port member\n"
	CONFIG_ETHPORT_STR
	"Enable igmp snooping functionality on vlan port member\n"
	"Disable igmp snooping functionality on vlan port member\n"
)
{
	/*
	DBusMessage *query, *reply;
	DBusError err;
	*/
	unsigned char	enDis = FALSE;
	unsigned short	vlanId;
	unsigned char	slot_no = 0,port_no = 0;
	unsigned int	eth_g_index = 0;	
	unsigned int	ret;
	unsigned int	tmp_index = 0;

	/*vty_out(vty,"enter params count: %d\n",argc);*/
	if(argc > 2) {
		vty_out(vty,"%% Command without so many parameters!\n");;
		return CMD_WARNING;
	}
	
	/*get first param*/
	ret = parse_slotport_no((char*)argv[0],&slot_no,&port_no);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"%% Error:malformed port number format!\n");
		return CMD_SUCCESS;
	}	

	/*get 2nd param : slot port number*/
	if(!strncmp(argv[1],"enable",IGMP_STR_CMP_LEN-3)) {
		enDis= TRUE;
	}
	else if(!strncmp(argv[1],"disable",IGMP_STR_CMP_LEN-3)) {
		enDis= FALSE;
	}
	else {
		vty_out(vty,"%% Command parameters error!\n");
		return CMD_WARNING;
	}

	tmp_index = (unsigned int)vty->index;
	vlanId = (unsigned short)tmp_index;
	/*
	vty_out(vty,"%s IGMP Snooping on vlan %d,slot %d,port %d.\n",\
					(enDis==1)?"enable":"disable",vlanId,slot_no,port_no);
	*/
	ret = dcli_enable_disable_igmp_one_port(vty,vlanId,slot_no,port_no,enDis);
	if (IGMPSNP_RETURN_CODE_OK == ret ) {
		/*vty_out(vty,"success\n");*/
		return CMD_SUCCESS;
	}
	else{
		if(IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL == ret){
			vty_out(vty,"%% Error:igmp snooping service not enabled!\n");
		}
		else if(IGMPSNP_RETURN_CODE_NO_SUCH_PORT == ret){
			vty_out(vty,"%% Error:bad slot/port number!\n");
		}
		else if(IGMPSNP_RETURN_CODE_NOTENABLE_PORT == ret){
			vty_out(vty,"%% Error:port %d/%d not enable igmp snooping!\n",slot_no,port_no);
		}
		else if(IGMPSNP_RETURN_CODE_HASENABLE_PORT == ret){
			vty_out(vty,"%% Error:port %d/%d already enable igmp snooping!\n",slot_no,port_no);
		}
		else if(IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST == ret){
			vty_out(vty,"%% Error:vlan %d not exist!\n",vlanId);
		}
		else if(IGMPSNP_RETURN_CODE_NOT_SUPPORT_IGMP_SNP == ret){
			vty_out(vty,"%% Error:vlan %d not support igmp snooping!\n",vlanId);
		}
		else if (IGMPSNP_RETURN_CODE_PORT_NOT_EXIST == ret){
			vty_out(vty,"%% Error:port %d/%d not vlan %d's member!\n",slot_no,port_no,vlanId);
		}
		else if (IGMPSNP_RETURN_CODE_ERROR_HW == ret){
			vty_out(vty,"%% Error:igmp configured error occurs on hardware!\n");
		}
		else if (IGMPSNP_RETURN_CODE_PORT_TRUNK_MBR == ret){
			vty_out(vty,"%% Error:port is trunk member!\n");
		}
		else if(IGMPSNP_RETURN_CODE_ERROR == ret){
			vty_out(vty,"%% Error:get devport error!\n");
		}
		else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
			vty_out(vty,"%% Product not support this function!\n");
		}
	}
	return CMD_SUCCESS;
}
/*********************************************
*	add|delete mcroute port slot/port
*
*
*	CMD : VLAN mode
**********************************************/

DEFUN(igmp_snp_mcroute_port_add_del_cmd_func,
	igmp_snp_mcroute_port_add_del_cmd,
	"(add|delete) igmp-snooping route-port PORTNO",
	"Add multicast route port with igmp snooping functionality\n"
	"Delete multicast route port with igmp snooping functionality\n"
	IGMP_SNOOPING_STR
	"Multicast route port for igmp snooping\n"
	CONFIG_ETHPORT_STR
)
{
	/*
	DBusMessage *query, *reply;
	DBusError err;
	*/
	unsigned char slot_no = 0,local_port_no = 0;
	unsigned char	enDis = FALSE;
	unsigned short vlanId = 0;
	unsigned int tmp_index = 0;
	int ret;

	if(argc > 2) {
		vty_out(vty,"%% Command with so many parameters!\n");
		return CMD_WARNING;
	}
	/*fetch the 1st param : add ? delete*/
	ret  = param_first_char_check((char*)argv[0],0);
	if (NPD_FAIL == ret) {
		vty_out(vty,"%% Bad parameter!\n");
		return CMD_WARNING;
	}
	else if(1 == ret){
		enDis = TRUE;
	}
	else if(0 == ret){
		enDis = FALSE;
	}
	
	/*get 2nd param : slot port number*/
	ret = parse_slotport_no((unsigned char *)argv[1],&slot_no,&local_port_no);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"%% Error:malformed port number format!\n");
		return CMD_SUCCESS;
	}	

	tmp_index = (unsigned int)vty->index;
	vlanId = (unsigned short)tmp_index;
	/*
	vty_out(vty,"%s mcrouter port :",(enDis==1)?"enable":"disable");
	vty_out(vty," vlan %d,slot %d,port %d",vlanId,slot_no,local_port_no);
	vty_out(vty," to(from) IGMP snooping.\n");
	*/
	ret = dcli_enable_disable_igmp_mcrouter_port(vlanId,slot_no,local_port_no,enDis);
	if(IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST == ret){
		vty_out(vty,"%% Error: port is not member of the vlan!\n");
		}
	else if(IGMPSNP_RETURN_CODE_PORT_NOT_EXIST == ret){
		vty_out(vty,"%% Error: port is not member of the vlan!\n");
	}
	else if (IGMPSNP_RETURN_CODE_NOTENABLE_PORT == ret){
		vty_out(vty,"%% Error: port not enabled igmp snooping!\n");
	}
	else if (IGMPSNP_RETURN_CODE_ROUTE_PORT_EXIST == ret){
		vty_out(vty,"%% Error: vlan has exist router port!\n");
	}
	else if(IGMPSNP_RETURN_CODE_ROUTE_PORT_NOTEXIST == ret){
		vty_out(vty,"%% Error: vlan has not exist router port!\n");
	}	
	else if(IGMPSNP_RETURN_CODE_NOTROUTE_PORT == ret){
		vty_out(vty,"%% Error: port not configured as router port!\n");
	}	
	else if (IGMPSNP_RETURN_CODE_PORT_TRUNK_MBR == ret){
		vty_out(vty,"%% Error: port is trunk member!\n");
	}
	else if (IGMPSNP_RETURN_CODE_PORT_NOT_EXIST == ret){
		vty_out(vty,"%% Error: no such port!\n");
	}
	else if (IGMPSNP_RETURN_CODE_NO_SUCH_PORT == ret){
		vty_out(vty,"%% Error: illegal slot or port!\n");
	}
	else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret)
		vty_out(vty,"%% Product not support this function!\n");
	else if (1 == ret){
		vty_out(vty,"%% Error: dbus error!\n");
	}
	
	return CMD_SUCCESS;
}
/******************************************
 *	show mcgroup member list in this vlan
 *	CMD:show mcgroup list 
 *
 *	1).read mcgroup member bitmap from hw  
 *	2).get group MacAddr
 *	CMD :VLAN node
*******************************************/
DEFUN(show_vlan_igmp_snp_mcgroup_list_cmd_func,
	show_vlan_igmp_snp_mcgroup_list_cmd,
	"show multicast-group list",			
	SHOW_STR
	"Show multicast group on layer 2\n"
	"All layer 2 multicast group in this vlan\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short	vlanId = 0;
	unsigned int tmp_index = 0;

	if(argc > 0)
	{
		vty_out(vty,"%% Command with so many parameters!\n");
		return CMD_WARNING;
	}

	tmp_index = (unsigned int)vty->index;
	vlanId = (unsigned short)tmp_index;

	dcli_show_igmp_snp_mcgroup_list(vty,vlanId);

	return CMD_SUCCESS;

}
/******************************************
 *	show multicast group's count in this vlan
 *	CMD:show multicast-group count
 *
 *	CMD :VLAN node
*******************************************/
DEFUN(show_vlan_igmp_snp_mcgroup_count_cmd_func,
	show_vlan_igmp_snp_mcgroup_count_cmd,
	"show multicast-group count",			
	SHOW_STR
	"Show multicast group on layer 2\n"
	"All layer 2 multicast group's count in this vlan\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short	vlanId = 0;
	unsigned int tmp_index = 0;

	if(argc > 0)
	{
		vty_out(vty,"%% Command with so many parameters!\n");
		return CMD_WARNING;
	}

	tmp_index = (unsigned int)vty->index;
	vlanId = (unsigned short)tmp_index;

	dcli_show_igmp_snp_mcgroup_count(vty,vlanId);

	return CMD_SUCCESS;
}

#if 0
/******************************************
 *	show  one special mcgroup's member in this vlan
 *
 *	CMD:show mcgroup <1-4095> //mcgroup entry 4095 uses specially
 *	mcgroup<4095> has all port in system
 *
 *	1).read mcgroup member bitmap from hw  
 *	2).get group MacAddr
 *	CMD :CONFIG node
*******************************************/
DEFUN(show_igmp_snp_mcgroup_one_cmd_func,
	show_igmp_snp_mcgroup_one_cmd,
	"show mcgroup <1-4095>",			
	SHOW_STR
	"Multicast group on Layer 2\n"
	"Layer2 mcgroup entry index:valid range[1-4095]\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short	vidx = 0,vlanId = 0;
	int ret;

	if(argc > 1)
	{
		vty_out(vty,"command parameters error\n");
		return CMD_FAILURE;
	}

	vlanId = (unsigned short )vty->index;
	/*get vIdx*/
	/*printf("before parse_vlan_no %s\n",argv[0]);*/
	ret = parse_vlan_no((char*)argv[0],&vidx);
	vty_out(vty,"To show l2mcgroup entry :vidx = %d.\r\n",vidx);
	dcli_show_igmp_snp_one_mcgroup(vty,vlanId,vidx);
	return CMD_SUCCESS;

}
#endif
/*******************************************************************************
 * dcli_igmp_snp_show_running_config
 *
 * DESCRIPTION:
 *   		Get igmp-snooping running config by Dbus.
 *
 * INPUTS:
 * 		null
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		null
 *
 * COMMENTS:
 *     
 **
 ********************************************************************************/
int dcli_igmp_snp_show_running_config(struct vty *vty){
	unsigned char igmpEnDis = 0;
	
	unsigned char Ismld = ISIGMP;
	char _tmpstr[64];

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
                dcli_dbus_connection_igmp = dbus_connection_dcli[i]->dcli_dbus_connection;			
            	memset(_tmpstr,0,64);
            	sprintf(_tmpstr,BUILDING_MOUDLE,"IGMP_SNOOPING");
            	vtysh_add_show_string(_tmpstr);
            	dcli_igmp_snp_vlan_show_running_config(Ismld);
            	if(igmpEnDis){
            		/*dcli_igmp_snp_protl_show_running_config()*/;
            	}

            	dcli_igmp_snp_time_show_running_config(Ismld);
            	if(igmpEnDis){
            		/*dcli_igmp_snp_protl_show_running_config()*/;
            	}
			}
		}
    }
	return 0;
}
/*******************************************************************************
 * dcli_igmp_snp_time_show_running_config
 *
 * DESCRIPTION:
 *   		Get igmp-snooping times value by Dbus.
 *
 * INPUTS:
 * 		null
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		null
 *
 * COMMENTS:
 *     
 **
 ********************************************************************************/
int dcli_igmp_snp_time_show_running_config(unsigned char Ismld) { 
	char *showStr = NULL;
	int ret = 0;
	DBusMessage *query, *reply;
	DBusError err;

	/*  dbus is used to get the time parameters */
	dbus_error_init(&err);
	query = dbus_message_new_method_call(
									IGMP_DBUS_BUSNAME,   \
									IGMP_DBUS_OBJPATH,    \
									IGMP_DBUS_INTERFACE,   \
									IGMP_SNP_DBUS_METHOD_IGMP_SNP_TIME_PARAMETER_GET
									);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&Ismld,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_igmp, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		printf("show igmp_snp_time running config failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return ret;
	}

	if (dbus_message_get_args(  reply, &err,
								DBUS_TYPE_STRING, &showStr,
								DBUS_TYPE_INVALID)) 
	{
	
		char _tmpstr[64];
		memset(_tmpstr,0,64);
		sprintf(_tmpstr,BUILDING_MOUDLE,"IGMP SNOOPING TIME");
		vtysh_add_show_string(_tmpstr);
		vtysh_add_show_string(showStr);
		ret = 1;
	}
	else
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
 
	return ret;
}


#if 0
void dcli_igmp_snp_protl_show_running_config() {	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	unsigned char enDis = 0;
	query = dbus_message_new_method_call(
							NPD_DBUS_BUSNAME,	\
							NPD_DBUS_VLAN_OBJPATH ,	\
							NPD_DBUS_VLAN_INTERFACE ,	\
							NPD_DBUS_VLAN_METHOD_IGMP_SNP_VLAN_SHOW_RUNNING_CONFIG);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_BYTE, &enDis,
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
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return;	
}
#endif
/*******************************************************************************
 * dcli_igmp_snp_element_init
 *
 * DESCRIPTION:
 *   		when init element add the config cmd to CONFIG NODE 
 *
 * INPUTS:
 * 		null
 *
 * OUTPUTS:
 *    	null
 *
 * RETURNS:
 *		null
 *
 * COMMENTS:
 *     
 **
 ********************************************************************************/
void dcli_igmp_snp_element_init(void)  
{

    /* init the dbus connect to the local board, config acl */
    dcli_dbus_connection_igmp = dcli_dbus_connection;

    install_node (&igmp_node_distributed, NULL,"IGMP_NODE_DISTRIBUTED");
    install_default(IGMP_NODE_DISTRIBUTED);
    install_node (&igmp_vlan_node, NULL,"IGMP_VLAN_NODE");
    install_default(IGMP_VLAN_NODE);
	
	install_node (&igmp_snp_node, dcli_igmp_snp_show_running_config, "IGMP_SNP_NODE");
    install_element(CONFIG_NODE,&config_igmp_sw_board_cmd);    /* config node */

	#if 1
	install_element(CONFIG_NODE,&config_igmp_snp_debug_cmd);
	install_element(CONFIG_NODE,&config_igmp_snp_no_debug_cmd);
	install_element(CONFIG_NODE,&debug_igmp_snp_pkt_info_cmd);
	install_element(CONFIG_NODE,&no_debug_igmp_snp_pkt_info_cmd);	
	install_element(IGMP_NODE_DISTRIBUTED,&config_igmp_snp_enable_cmd);
	install_element(IGMP_NODE_DISTRIBUTED,&config_igmp_snp_timer_cmd);
	install_element(IGMP_NODE_DISTRIBUTED,&show_igmp_snp_timers_cmd);
	install_element(IGMP_NODE_DISTRIBUTED,&config_igmp_layer2_vlan_cmd);
	install_element(IGMP_NODE_DISTRIBUTED,&show_igmp_snp_get_state_cmd);
	
	/*
	install_element(VLAN_NODE,&igmp_snp_delete_mcgroup_cmd);
	install_element(VLAN_NODE,&igmp_snp_delete_mcgroup_vlan_cmd);
	install_element(VLAN_NODE,&igmp_snp_delete_mcgroup_vlan_all_cmd);
	*/

	install_element(IGMP_VLAN_NODE,&igmp_snp_mcroute_port_add_del_cmd);
	install_element(IGMP_VLAN_NODE,&config_igmp_snp_npd_vlan_cmd);
	install_element(IGMP_VLAN_NODE,&config_igmp_snp_npd_port_cmd);
	install_element(IGMP_VLAN_NODE,&show_vlan_igmp_snp_mcgroup_list_cmd);
	install_element(IGMP_VLAN_NODE,&show_vlan_igmp_snp_mcgroup_count_cmd);	
	install_element(IGMP_VLAN_NODE,&show_igmp_snp_multicast_route_port_cmd);
	
	install_element(IGMP_NODE_DISTRIBUTED,&igmp_snp_show_vlan_count_cmd);
	/*install_element(VLAN_NODE,&show_igmp_snp_mcgroup_one_cmd);*/
	install_element(IGMP_NODE_DISTRIBUTED,&igmp_snp_show_vlan_list_cmd);
	install_element(IGMP_NODE_DISTRIBUTED,&igmp_snp_show_group_count_cmd);
	install_element(IGMP_NODE_DISTRIBUTED,&igmp_snp_show_vlan_group_list_cmd);
	install_element(IGMP_NODE_DISTRIBUTED,&igmp_snp_show_vlan_router_port_cmd);	
	#else
/*
	install_default(IGMP_SNP_NODE);
	*/
	install_element(CONFIG_NODE,&config_igmp_snp_debug_cmd);
	install_element(CONFIG_NODE,&config_igmp_snp_no_debug_cmd);
	install_element(CONFIG_NODE,&debug_igmp_snp_pkt_info_cmd);
	install_element(CONFIG_NODE,&no_debug_igmp_snp_pkt_info_cmd);	
	install_element(CONFIG_NODE,&config_igmp_snp_enable_cmd);
	install_element(CONFIG_NODE,&config_igmp_snp_timer_cmd);
	install_element(CONFIG_NODE,&show_igmp_snp_timers_cmd);
	install_element(ENABLE_NODE,&show_igmp_snp_timers_cmd);
	install_element(VIEW_NODE,&show_igmp_snp_timers_cmd);	
	/*
	install_element(VLAN_NODE,&igmp_snp_delete_mcgroup_cmd);
	install_element(VLAN_NODE,&igmp_snp_delete_mcgroup_vlan_cmd);
	install_element(VLAN_NODE,&igmp_snp_delete_mcgroup_vlan_all_cmd);
	*/

	install_element(VLAN_NODE,&igmp_snp_mcroute_port_add_del_cmd);
	install_element(VLAN_NODE,&config_igmp_snp_npd_vlan_cmd);
	install_element(VLAN_NODE,&config_igmp_snp_npd_port_cmd);
	install_element(VLAN_NODE,&show_vlan_igmp_snp_mcgroup_list_cmd);
	install_element(VLAN_NODE,&show_vlan_igmp_snp_mcgroup_count_cmd);	
	install_element(VLAN_NODE,&show_igmp_snp_multicast_route_port_cmd);
	install_element(CONFIG_NODE,&igmp_snp_show_vlan_count_cmd);
	/*install_element(VLAN_NODE,&show_igmp_snp_mcgroup_one_cmd);*/
	install_element(CONFIG_NODE,&igmp_snp_show_vlan_list_cmd);
	install_element(CONFIG_NODE,&igmp_snp_show_group_count_cmd);
	install_element(CONFIG_NODE,&igmp_snp_show_vlan_group_list_cmd);
	install_element(CONFIG_NODE,&igmp_snp_show_vlan_router_port_cmd);	
	#endif
}
#ifdef __cplusplus
}
#endif
