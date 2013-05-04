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
* dcli_stp.c
*
* MODIFY:
*		by <qinhs@autelan.com> on 03/07/2008 revision <0.1>
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		CLI definition for STP module.
*
* DATE:
*		02/21/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.73 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include <zebra.h>
#include <dbus/dbus.h>

#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>
#include <util/npd_list.h>

#include "dcli_stp.h"
#include "dcli_sem.h"
#include "dcli_vlan.h"
#include "command.h"
#include "dcli_common_stp.h"
#include "dcli_main.h"
#include "board/board_define.h"
/*#define STP_FULL_DEBUG*/	

extern DBusConnection *dcli_dbus_connection;
DBusConnection *dcli_dbus_connection_stp;
/* For distributed system 2011-09-09 */
extern int is_distributed;
struct cmd_node stp_node_distributed = 
{
	STP_NODE_DISTRIBUTED,
	"%s(config-stp-sw-board)# "
};
struct cmd_node mst_node = 
{
	MST_NODE,
	"%s(config-mst)# "
};

struct cmd_node stp_node = 
{
	STP_NODE,
	"%s(config-stp)# "
};

static char g_regionName[32] = {'\0'};
DEFUN(config_stp_on_board_cmd_func,
	  config_stp_on_board_cmd,
	  "config stp switch-board <1-16>",
	  CONFIG_STR
	  "Configure stp of Switch-board\n"
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
	if (STP_ERROR == ret) {
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
    	sprintf(file_path,"/dbm/product/slot/slot%d/function_type", dist_slot);
    	function_type = get_product_info(file_path);
    	if (function_type != SWITCH_BOARD)
    	{
    		vty_out(vty, "Slot %d is not Switch-board, Please select another !\n", dist_slot);	
    		return CMD_WARNING;
    	}		
    	if(NULL == dbus_connection_dcli[dist_slot]->dcli_dbus_connection) 				
    	{
			vty_out(vty, "Can not connect to slot %d.\n", dist_slot);
			return CMD_WARNING;
    	}
		else 
    	{
			vty_out(vty, "<<========== Config stp switch-board slot: %d =========>>\n",dist_slot);
            #if 0
			if(dist_slot == local_slot_id)
    		{
                dcli_dbus_connection_stp = dcli_dbus_connection;
    		}
			else
			{
			    dcli_dbus_connection_stp = dbus_connection_dcli[dist_slot]->dcli_dbus_connection;			
			}
			#else
			dcli_dbus_connection_stp = dbus_connection_dcli[dist_slot]->dcli_dbus_connection;
            #endif
			if(CONFIG_NODE == vty->node)
			{
				vty->node = STP_NODE_DISTRIBUTED;
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

DEFUN(config_spannibg_tree_mode_cmd_func,
	config_spannibg_tree_mode_cmd,
	"config spanning-tree (stp|mst)",
	CONFIG_STR
	STP_STR
	"STP/RSTP mode\n"
	"MSTP mode\n"
	)
{
	int state = 1;
	int stp_mode = 0; 
	int node = DCLI_STP_M; 
	unsigned char IsSuport = 0;
	unsigned int ret = 0;
		
	if(argc > 1)
	{
		vty_out(vty,"command parameters error\n");
		return CMD_WARNING;
	}

	ret = dcli_stp_function_support(vty, &IsSuport);
	if(CMD_SUCCESS == ret){
		if(1 == IsSuport){
			vty_out(vty,"this product do not support stp!\n");
			return CMD_WARNING;
		}
	}
	else{ 
		return CMD_WARNING;
	}
	state = dcli_get_brg_g_state(vty,&stp_mode);
	/*Testing the state of the brdige , that geted.*/
	/*vty_out(vty,"Now the stp state is %d and stp mode is %d.\n",state, stp_mode);*/
	if(0 == state){
		if(2 == stp_mode) {
			if(strncmp("mst",argv[0],strlen(argv[0]))==0)
			{
				vty->node = MST_NODE;
			}
			else if (strncmp("stp",argv[0],strlen(argv[0]))==0)
			{
				vty->node = STP_NODE;
			}
			else
			{
				vty_out(vty,"command parameters error!\n");
				return CMD_WARNING;
			}

		}
		else{
			vty_out(vty,"Critical error in stp!\n");
			return CMD_WARNING;
		}
	}
	else if(1 == state)	
		{
			if(0 == stp_mode) /*Bridge is running stp*/
				{	
					if((strcmp(argv[0], "stp") == 0) && (STP_NODE_DISTRIBUTED == vty->node))
						vty->node = STP_NODE;
					else if((strcmp(argv[0], "mst") == 0) && (STP_NODE_DISTRIBUTED == vty->node))
						{
							vty_out(vty, "Please disable stp first!\n");
						}
					else
						{
							vty_out(vty, "Bad parameters error!\n");
							return CMD_WARNING;
						}
				}
			else if(1 == stp_mode) /*Bridge is running mstp*/
				{
					if((strcmp(argv[0], "stp") == 0) && (STP_NODE_DISTRIBUTED == vty->node))
						{
							vty_out(vty, "Please disable mstp first!\n");
						}
					else if((strcmp(argv[0], "mst") == 0) && (STP_NODE_DISTRIBUTED == vty->node))
						vty->node = MST_NODE;
					else
						{
							vty_out(vty, "Bad parameters error!\n");
							return CMD_WARNING;
						}
				}		
			else
				{
					vty_out(vty, "Critical error in stp!\n");
					return CMD_WARNING;
				}
		}
	else
		{
			vty_out(vty, "Critical error in stp!\n");
			return CMD_WARNING;
		}
	/*else {
		if(strcmp(argv[0],"mst")==0 && vty->node == CONFIG_NODE) {
			vty_out(vty,"Please disable stp first\n");
			vty->node = STP_NODE;
		}
		else if(strcmp(argv[0],"stp")==0 && vty->node == CONFIG_NODE) {
			vty_out(vty,"Please disable mtp first\n");
			vty->node = MST_NODE;
		}
	}
	*/
	return CMD_SUCCESS;

}


DEFUN(config_spannibg_tree_stp_cmd_func,
	config_spannibg_tree_stp_cmd,
	"config spanning-tree (enable|disable)",
	CONFIG_STR
	STP_STR
	"Enable spanning-tree \n"
	"Disable spanning-tree\n"
	)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int isEnable = 0;
	int ret,op_ret;
	VLAN_PORTS_BMP ports_bmp = {0};
	int count = 0;

	if(argc > 1)
	{
		vty_out(vty,"command parameters error\n");
		return CMD_WARNING;
	}
	if(strncmp("enable",argv[0],strlen(argv[0]))==0)
	{
		isEnable = 1;
	}
	else if (strncmp("disable",argv[0],strlen(argv[0]))==0)
	{
		isEnable = 0;
	}
	else
	{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}

	if(STP_NODE == vty->node) {
		/*init stp protocol*/
		dcli_enable_g_stp_to_protocol(vty,DCLI_STP_M,isEnable);
		/*set stp version*/
		/*vty_out(vty,"stp dcli to protocol %s\n",isEnable ? "enable" : "disable");*/
		if(isEnable){
			dcli_set_bridge_force_version(vty,STP_FORCE_VERS);
			/*vty_out(vty,"stp dcli force isenable  finish \n");*/
		}
	}
	else if(MST_NODE == vty->node) {
		dcli_enable_g_stp_to_protocol(vty,DCLI_MST_M,isEnable);
		if(isEnable)
			dcli_set_bridge_force_version(vty,MST_FORCE_VERS);
	}
	
	return CMD_SUCCESS;
}

DEFUN(config_spannibg_tree_stp_ethport__cmd_func,
	config_spannibg_tree_stp_ethport_cmd,
	"config spanning-tree eth-port PORTNO (enable|disable)",
	CONFIG_STR
	STP_STR
	"Ethernet port infomation\n"
	CONFIG_ETHPORT_STR
	"Enable spanning-tere on the port\n"
	"Disable spanning-tere on the port\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	struct interface * ethIntf = NULL;
	unsigned char slot_no = 0,port_no = 0;
	unsigned int isEnable = 0;
	int port_index = 0;
	int lk = 0;
	unsigned int speed = 0, isWAN = 0;
	unsigned int duplex_mode = 0;
	int ret = 0,op_ret = 0;
	unsigned int mode = DCLI_STP_M;
	
	if(argc > 2)
	{
		vty_out(vty,"command parameters error\n");
		return  CMD_WARNING ;
	}
	/* get the value of the enable */
	if(strncmp("enable",argv[1],strlen(argv[1]))==0)
	{
		isEnable = 1;
	}
	else if (strncmp("disable",argv[1],strlen(argv[1])) ==0)
	{
		isEnable = 0;
	}
	else
	{
		vty_out(vty,"bad command parameter!\n");
		return  CMD_WARNING ;
	}		

	ret = parse_slotport_no((unsigned char *)argv[0],&slot_no,&port_no);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"Unknow port number format.\n");
		return  CMD_WARNING ;
	}
	
	if(CMD_SUCCESS != dcli_get_one_port_index(vty,slot_no,port_no,&port_index))
	{
		DCLI_DEBUG(("execute command failed\n"));
		return  CMD_WARNING ;
	}
	
	/*get the link state of the port*/
	if (CMD_SUCCESS != dcli_get_port_index_link_state(vty,port_index,&lk,&isWAN))
	{
		vty_out(vty, "Can't get the port link state!\n");
	}
	/*
	else 
	{
		vty_out(vty,"port[%d],link state %s\n",port_index,lk ? "up" : "down");
	}
	*/
	/*get the speed of the port*/
	if(CMD_SUCCESS != dcli_get_port_index_speed(vty,port_index,&speed)){
	    vty_out(vty, "Can't get the port speed!\n");
	}
	/*
	else{
        vty_out(vty,"port[%d],speed is  %d\n",port_index,speed);
	}
	*/
	/*get the port duplex mode*/
	if(isEnable){
		if(CMD_SUCCESS != dcli_get_port_duplex_mode(vty,port_index,&duplex_mode)){
	        vty_out(vty,"Can't get the port duplex mode!\n");
		}
		else{/*set duplex_mode value to stp*/
	       if(CMD_SUCCESS != dcli_set_port_duplex_mode_to_stp(vty,port_index,duplex_mode)){
		   	  DCLI_DEBUG(("set duplex mode to stp error!\n"));
	       	}
		}
	}
	/*vty_out(vty,"parse slot %d,port %d,port_index %d\n",slot_no,port_no,port_index);*/

	if(STP_NODE == vty->node ) 
		mode = DCLI_STP_M;
	else if(MST_NODE == vty->node)
		mode = DCLI_MST_M;

	
	DCLI_DEBUG(("It has been here: !!!! and mode %d\n",mode));
	if((CMD_SUCCESS == dcli_enable_stp_on_one_port_to_protocol(vty,port_index,isEnable,lk,speed,isWAN,slot_no,port_no))) {
        /*vty_out(vty,"SUCCESS to set stp port enable to protocol port_index %d,lk %d,isEnable %d,speed %d\n",port_index,lk,isEnable,speed);*/
		ret = dcli_enable_stp_on_one_port_to_npd(vty,mode,port_index,isEnable);
		if(ret != 0 && !isWAN){
			vty_out(vty,"%% execute command failed\n");
			return  CMD_WARNING ;
        }
	}

	return CMD_SUCCESS;

}

DEFUN(stp_dbus_bridge_priority_func,
		stp_bridge_priority_cmd,
		"config spanning-tree priority <0-61440>",
		CONFIG_STR
		STP_STR
		"Config spanning-tree priority\n"
		"Specify priority value must be multiple of 4096\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int re_value,ret;
	unsigned int value,mstid = 0;
	
	value = strtoul (argv[0],0,10);
	/*Display the value that get from the command
	//printf("The parameter's value value = %ud\n", value);*/
	if(value < MIN_BR_PRIO || value > MAX_BR_PRIO ){
		vty_out(vty,"input priority value out range <0-61440>\n");
		return  CMD_WARNING;
	}
	else{
		if(0 != (value%4096)){
			vty_out (vty,"spanning-tree priority value must be times of 4096!\n");
			return CMD_WARNING;
		}
	}
	
	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									RSTP_DBUS_METHOD_CONFIG_PRIO);

    dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
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

		if(STP_DISABLE == ret){
			if(STP_NODE == vty->node)
				vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
			else if(MST_NODE == vty->node)
				vty_out(vty,PRINTF_MSTP_NOT_ENABLED);
		}
		else if(STP_Small_Bridge_Priority == ret){
            vty_out(vty,PRINTF_STP_Small_Bridge_Priority);
		}
		else if(STP_Large_Bridge_Priority == ret){
            vty_out(vty,PRINTF_STP_Large_Bridge_Priority);
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

DEFUN(stp_dbus_bridge_maxage_func,
		stp_bridge_maxage_cmd,
		"config spanning-tree max-age <6-40>",
		CONFIG_STR
		STP_STR
		"Config spanning-tree maximum aging time\n"
		"Specify priority value must be 2* (Hello-time + 1) <= Max-Age <= 2* (Forward-Delay - 1)\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	unsigned int value;
	
	value = strtoul (argv[0],0,10);
	if(value < MIN_BR_MAXAGE || value > MAX_BR_MAXAGE ){
		vty_out(vty,"input max-age value out range <6-40>\n");
		return  CMD_WARNING;
	}

	query = dbus_message_new_method_call( 				 \
										RSTP_DBUS_NAME,		 \
										RSTP_DBUS_OBJPATH,	 \
										RSTP_DBUS_INTERFACE, \
										RSTP_DBUS_METHOD_CONFIG_MAXAGE);

    dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&value,
								DBUS_TYPE_INVALID);

	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);


	dbus_message_unref(query);

	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",&err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
    
	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_INVALID)) {

		if(STP_DISABLE == ret){
			if(STP_NODE == vty->node)
				vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
			else if(MST_NODE == vty->node)
				vty_out(vty,PRINTF_MSTP_NOT_ENABLED);
		}
		else if(STP_Small_Max_Age == ret ){
			vty_out(vty,PRINTF_STP_Small_Max_Age);
		}
		else if(STP_Large_Max_Age == ret){
			vty_out(vty,PRINTF_STP_Large_Max_Age);
		}
		else if(STP_Forward_Delay_And_Max_Age_Are_Inconsistent == ret){
			vty_out(vty,PRINTF_STP_Forward_Delay_And_Max_Age_Are_Inconsistent);
		}
	    else if(STP_Hello_Time_And_Max_Age_Are_Inconsistent == ret){
            vty_out(vty,PRINTF_STP_Hello_Time_And_Max_Age_Are_Inconsistent);
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

DEFUN(stp_dbus_bridge_hetime,
		stp_bridge_hetime_cmd,
		"config spanning-tree hello-time <1-10>",
		CONFIG_STR
		STP_STR
		"Config spanning-tree hello time\n"
		"Specify hello time value\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int  ret;
	unsigned int value=0;
	
	value = strtoul (argv[0],0,10);
	if(value < MIN_BR_HELLOT || value > MAX_BR_HELLOT )
	{
		vty_out(vty,"input hello-time value out range <1-10>\n");
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(		\
									RSTP_DBUS_NAME,	\
									RSTP_DBUS_OBJPATH,	\
									RSTP_DBUS_INTERFACE,	\
									RSTP_DBUS_METHOD_CONFIG_HELTIME);

	dbus_error_init(&err);
	dbus_message_append_args(query,
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
		if(STP_DISABLE == ret){
			if(STP_NODE == vty->node)
				vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
			else if(MST_NODE == vty->node)
				vty_out(vty,PRINTF_MSTP_NOT_ENABLED);
		}
		else if(STP_Small_Hello_Time== ret ){
			vty_out(vty,PRINTF_STP_Small_Hello_Time);
		}
		else if(STP_Large_Hello_Time== ret){
			vty_out(vty,PRINTF_STP_Large_Hello_Time);
		}
	    else if(STP_Hello_Time_And_Max_Age_Are_Inconsistent == ret){
            vty_out(vty,PRINTF_STP_Hello_Time_And_Max_Age_Are_Inconsistent);
		}
		else if(STP_Hello_Time_And_Forward_Delay_Are_Inconsistent == ret){
			vty_out(vty,PRINTF_STP_Hello_Time_And_Forward_Delay_Are_Inconsistent);
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

DEFUN(stp_dbus_bridge_fordelay_fun,
		stp_bridge_fordelay_cmd,
		"config spanning-tree forward-delay <4-30>",
		CONFIG_STR
		STP_STR
		"Config spanning-tree forward delay time\n"
		"Specify forward delay value \n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int  ret;
	unsigned int value;
	
	value = strtoul (argv[0],0,10);
	if(value < MIN_BR_FWDELAY || value > MAX_BR_FWDELAY )
	{
		vty_out(vty,"input forward delay value out range <4-30>\n");
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(					\
										RSTP_DBUS_NAME,			\
										RSTP_DBUS_OBJPATH,		\
										RSTP_DBUS_INTERFACE,	\
										RSTP_DBUS_METHOD_CONFIG_FORDELAY);

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

		if(STP_DISABLE == ret){
			if(STP_NODE == vty->node)
				vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
			else if(MST_NODE == vty->node)
				vty_out(vty,PRINTF_MSTP_NOT_ENABLED);
		}
		else if(STP_Small_Forward_Delay== ret ){
			vty_out(vty,PRINTF_STP_Small_Forward_Delay);
		}
		else if(STP_Large_Forward_Delay== ret){
			vty_out(vty,PRINTF_STP_Large_Forward_Delay);
		}
	    else if(STP_Forward_Delay_And_Max_Age_Are_Inconsistent == ret){
            vty_out(vty,PRINTF_STP_Forward_Delay_And_Max_Age_Are_Inconsistent);
		}
		else if(STP_Hello_Time_And_Forward_Delay_Are_Inconsistent == ret){
			vty_out(vty,PRINTF_STP_Hello_Time_And_Forward_Delay_Are_Inconsistent);
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

DEFUN(stp_dbus_bridge_forversion_fun,
		stp_bridge_forversion_cmd,
		"config spanning-tree force-version FORVERSION",
		CONFIG_STR
		STP_STR
		"Config spanning-tree force-version(0 or 2)\n"
		"Specify force-version value,0 means that compatible stp,2 means that only support RSTP \n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	unsigned int value;
	
	value = strtoul (argv[0],0,10);
	if (!((value == 0)||(value == 2)) ) {
		vty_out (vty,"%%spanning tree version must be 0 or 2 !\n");
		return CMD_WARNING;
	}
	if(STP_NODE == vty->node) {
		ret = dcli_set_bridge_force_version(vty,value);
	}
	
	return ret;
}

DEFUN(stp_dbus_bridge_noconfig_fun,
		stp_bridge_noconfig_cmd,
		"config spanning-tree default ",
		CONFIG_STR
		STP_STR
		"Config spanning-tree default value\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	unsigned int mstid = 0;
	
	query = dbus_message_new_method_call(				\
									RSTP_DBUS_NAME,			\
									RSTP_DBUS_OBJPATH,		\
									RSTP_DBUS_INTERFACE,	\
									RSTP_DBUS_METHOD_CONFIG_NOCONFIG);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32&mstid,						
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

		if(STP_DISABLE == ret){
			if(STP_NODE == vty->node)
				vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
			else if(MST_NODE == vty->node)
				vty_out(vty,PRINTF_MSTP_NOT_ENABLED);
		}
	}	
	else {
		
		vty_out(vty," Failed get args.\n");
		if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
}

/**************************************************************************************
*RSTP command
*
**************************************************************************************/
DEFUN(stp_dbus_bridge_port_pathcost_fun,
		stp_bridge_port_pathcost_cmd,
		"config spanning-tree eth-port PORTNO path-cost (auto|<1-200000000>)",
		CONFIG_STR
		STP_STR
		"Config spanning-tree eth-port\n"
		CONFIG_ETHPORT_STR
		"config  spanning-tree path-cost \n"
		"Path cost value auto\n"
		"Specify path cost value (VALUE)\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int re_slot_port = 0;
	unsigned int re_value;
	unsigned int value,mstid = 0;
	unsigned char slot_no=0;
	unsigned char port_no=0;
	unsigned int ret;
	unsigned int port_index;
	
	if(argc > 2)
	{
		vty_out(vty,"command parameters error!\n");
		return CMD_WARNING;
	}
	
	ret = parse_slotport_no((unsigned char *)argv[0],&slot_no,&port_no);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"Unknow port number format.\n");
		return CMD_SUCCESS;
	}
	
	if (strcmp(argv[1] , "auto")== 0){
			value = ADMIN_PORT_PATH_COST_AUTO;
	}
	else {
		value = strtoul (argv[1],0,10);
		if (!((value > 0)&&(value % 20 ==0))){
			vty_out (vty,"path cost must be times of 20!\n");
			return CMD_WARNING;
		}
		else if(ADMIN_PORT_PATH_COST_AUTO == value){
            vty_out(vty,"The path-cost 200000000 is auto value!");
		}
	}
	if(CMD_SUCCESS != dcli_get_one_port_index(vty,slot_no,port_no,&port_index))	{
		DCLI_DEBUG(("execute command failed\n"));
		return CMD_WARNING;
	}

	
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
										RSTP_DBUS_NAME,		\
										RSTP_DBUS_OBJPATH,	\
										RSTP_DBUS_INTERFACE, \
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

		if(STP_DISABLE == ret){
			vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
		}
		else if(STP_PORT_NOT_ENABLED == ret) {
			vty_out(vty,PRINTF_PORT_NOT_ENABLED);
		}
		#if 0
		else if(0 == ret)
			dcli_stp_set_port_pathcost_to_npd(vty,mstid,port_index,value);
			DCLI_DEBUG(("port_index %d, priority %d\n", port_index, value));
		#endif
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

DEFUN(stp_dbus_bridge_port_prio_fun,
		stp_bridge_port_prio_cmd,
		"config spanning-tree eth-port PORTNO priority <0-240>",
		CONFIG_STR
		STP_STR
		"Config spanning-tree eth-port \n"
		CONFIG_ETHPORT_STR
		"Config spanning-tree eth-port priority value \n"
		"Specify port priority value must be multiple of 16\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int port_index;
	unsigned int re_slot_port,re_value;
	unsigned int value,mstid = 0;
	unsigned char slot_no,port_no;
	unsigned int ret;

	if(argc > 2)
	{
		vty_out(vty,"command parameters error\n");
		return CMD_WARNING;
	}
	
	ret = parse_slotport_no((unsigned char *)argv[0],&slot_no,&port_no);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"Unknow port number format.\n");
		return CMD_WARNING;
	}
	
	value = strtoul (argv[1],0,10);
	if(value < MIN_PORT_PRIO || value > MAX_PORT_PRIO ){
		vty_out(vty,"input port-priority value out range <0-240>\n");
		return CMD_WARNING;
	}
	else{
		if(0 != (value%16)){
			vty_out (vty,"spanning-tree port-priority value must be times of 16!\n");
			return CMD_WARNING;
		}
	}

	if(CMD_SUCCESS != dcli_get_one_port_index(vty,slot_no,port_no,&port_index))	{
		DCLI_DEBUG(("execute command failed\n"));
		return CMD_WARNING;
	}
	DCLI_DEBUG(("slot %d, port %d, prot_index %d\n", slot_no, port_no, port_index));
	
	dbus_error_init(&err);	
	query = dbus_message_new_method_call(				\
										RSTP_DBUS_NAME,		\
										RSTP_DBUS_OBJPATH,	\
										RSTP_DBUS_INTERFACE, \
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
		if(STP_DISABLE == ret){
			vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
		}
		else if(STP_PORT_NOT_ENABLED == ret) {
			vty_out(vty,PRINTF_PORT_NOT_ENABLED);
		}
		#if 0
		else if(0 == ret)
			dcli_stp_set_port_prio_to_npd(vty,mstid,port_index,value);
		#endif
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

DEFUN(stp_dbus_bridge_port_none_stp_fun,
		stp_bridge_port_none_stp_cmd,
		"config spanning-tree eth-port PORTNO none-stp (yes|no)",
		CONFIG_STR
		STP_STR
		"Config spanning-tree eth-port  \n"
		CONFIG_ETHPORT_STR
		"Config spanning-tree none-stp (yes| no) \n"
		"Specify spanning-tree  value (yes)\n"
		"Specify spanning-tree  value (no)\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int re_slot_port,re_value;
	unsigned int value,mstid = 0;
	unsigned char slot_no,port_no;
	unsigned int port_index;
	unsigned int ret;
	int i = 0;
	
	if(argc > 2)
	{
		vty_out(vty,"command parameter error\n");
		return CMD_WARNING;
	}
	ret = parse_slotport_no((unsigned char *)argv[0],&slot_no,&port_no);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"Unknow port number format.\n");
		return CMD_WARNING ;
	}

	if(CMD_SUCCESS != dcli_get_one_port_index(vty,slot_no,port_no,&port_index))	{
		DCLI_DEBUG(("execute command failed\n"));
		return CMD_WARNING;
	}

		
	if(strncmp("yes",argv[1],strlen(argv[1]))==0)
	{
		value = 1;
	}
	else if (strncmp("no",argv[1],strlen(argv[1]))==0)
	{
		value = 0;
	}
	else
	{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}		
	
	dbus_error_init(&err);
	

	query = dbus_message_new_method_call(				\
									RSTP_DBUS_NAME,			\
									RSTP_DBUS_OBJPATH,		\
									RSTP_DBUS_INTERFACE,	\
									RSTP_DBUS_METHOD_CONFIG_NONSTP);
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
		if(STP_DISABLE == ret){
			vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
		}
		else if(STP_PORT_NOT_ENABLED == ret) {
			vty_out(vty,PRINTF_PORT_NOT_ENABLED);
		}
		#if 0
		else if(0 == ret)
			dcli_stp_set_port_nonstp_to_npd(vty,mstid,port_index,value);
		#endif
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

DEFUN(stp_dbus_bridge_port_p2p_fun,
		stp_bridge_port_p2p_cmd,
		"config spanning-tree eth-port PORTNO p2p (yes | no |auto)",
		CONFIG_STR
		STP_STR
		"Config  spanning-tree eth-port  \n"
		CONFIG_ETHPORT_STR
		"Config  spanning-tree  p2p (yes | no |auto) value \n"
		"Specify spanning-tree p2p value ( yes )\n"
		"Specify spanning-tree p2p value ( no )\n"
		"Specify spanning-tree p2p value (auto)\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned char slot_no,port_no;
	unsigned int input,mstid = 0;
	unsigned int port_index;
	unsigned int ret;
	
	if(argc > 2)
	{
		vty_out(vty,"command parameters error\n");
		return CMD_WARNING;
	}

	ret = parse_slotport_no((unsigned char *)argv[0],&slot_no,&port_no);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"Unknow port number format.\n");
		return CMD_WARNING;
	}
	
	if (strncmp("no",argv[1] , strlen(argv[1]))== 0){
		input = 0;
	}
	else if(strcmp(argv[1],"yes")== 0){
		input = 1;
	}
	else if(strncmp("auto",argv[1],strlen(argv[1]))== 0){
		input = 2;
	}
	else {
		vty_out(vty,"p2p config value must be yes or no or auto!\n");
		return CMD_WARNING;
	}

	if(CMD_SUCCESS != dcli_get_one_port_index(vty,slot_no,port_no,&port_index))	{
		DCLI_DEBUG(("execute command failed\n"));
		return CMD_WARNING;
	}
	
	dbus_error_init(&err);
	

	query = dbus_message_new_method_call(						\
									RSTP_DBUS_NAME,					\
									RSTP_DBUS_OBJPATH,				\
									RSTP_DBUS_INTERFACE,			\
									RSTP_DBUS_METHOD_CONFIG_P2P);

	dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&mstid,
								DBUS_TYPE_UINT32,&port_index,
								DBUS_TYPE_UINT32,&input,
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
							DBUS_TYPE_UINT32,&ret,
							DBUS_TYPE_INVALID)) {
		if(STP_DISABLE == ret){
			vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
		}
		else if(STP_PORT_NOT_ENABLED == ret) {
			vty_out(vty,PRINTF_PORT_NOT_ENABLED);
		}
		#if 0
		else if(0 == ret) {
			dcli_stp_set_port_p2p_to_npd(vty,mstid,port_index,input);
		}
		#endif
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

DEFUN(stp_dbus_bridge_port_edge_fun,
		stp_bridge_port_edge_cmd,
		"config spanning-tree eth-port PORTNO edge (yes|no)",
		CONFIG_STR
		STP_STR
		"Config  spanning-tree eth-port  \n"
		CONFIG_ETHPORT_STR
		"Config  spanning-tree  edge value\n"
		"Specify spanning-tree edge value (yes)  \n"
		"Specify spanning-tree edge value (no)  \n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int re_slot_port,re_value;
	unsigned int port_index;
	
	unsigned char slot_no,port_no;
	unsigned int input,mstid = 0;
	
	unsigned int ret;
	
	if(argc > 2)
	{
		vty_out(vty,"command parameter error\n");
		return CMD_WARNING;
	}
	
	ret = parse_slotport_no((unsigned char *)argv[0],&slot_no,&port_no);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"Unknow port number format.\n");
		return CMD_SUCCESS;
	}	
	
	if(strncmp("no",argv[1] , strlen(argv[1])) == 0){
		input =0;
	}
	else if (strncmp("yes",argv[1],strlen(argv[1]))== 0){
		input = 1;
	}
	else {
		vty_out(vty,"edge config value must be yes or no\n");
		return CMD_WARNING;
	}

	if(CMD_SUCCESS != dcli_get_one_port_index(vty,slot_no,port_no,&port_index))	{
		DCLI_DEBUG(("execute command failed\n"));
		return CMD_WARNING;
	}


	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
									RSTP_DBUS_NAME,			\
									RSTP_DBUS_OBJPATH,		\
									RSTP_DBUS_INTERFACE,	\
									RSTP_DBUS_METHOD_CONFIG_EDGE);
	dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&mstid,
								DBUS_TYPE_UINT32,&port_index,
								DBUS_TYPE_UINT32,&input,
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
		if(STP_DISABLE == ret){
			vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
		}
		else if(STP_PORT_NOT_ENABLED == ret) {
			vty_out(vty,PRINTF_PORT_NOT_ENABLED);
		}
		#if 0
		else if(0 == ret)
			dcli_stp_set_port_edge_to_npd(vty,mstid,port_index,input);
		#endif
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

DEFUN(stp_dbus_bridge_port_mcheck_fun,
		stp_bridge_port_mcheck_cmd,
		"config spanning-tree eth-port PORTNO mcheck yes",
		CONFIG_STR
		STP_STR
		"Config  spanning-tree eth-port  \n"
		CONFIG_ETHPORT_STR
		"Config  spanning-tree mcheck value \n"
		"Specify spanning-tree  value yes\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int re_slot_port,re_value,ret;
	unsigned int port_index,mstid = 0;
	unsigned char slot_no,port_no;
	
	int test;
	int i = 0;
	
	if(argc > 2) {
		vty_out(vty,"command parameters error!\n");
		return CMD_WARNING;
	}

	test = parse_slotport_no((unsigned char *)argv[0],&slot_no,&port_no);
	if (NPD_FAIL == test) {
    	vty_out(vty,"Unknow port number format.\n");
		return CMD_WARNING;
	}
	
	if(CMD_SUCCESS != dcli_get_one_port_index(vty,slot_no,port_no,&port_index))	{
		DCLI_DEBUG(("execute command failed\n"));
		return CMD_WARNING;
	}
	
	#ifdef STP_FULL_DEBUG
	vty_out(vty,"SLOT_NO %d, LOCAL_PORT_NO %d, port_index %d \n",slot_no,port_no,port_index);
	#endif
	dbus_error_init(&err);
	query = dbus_message_new_method_call(					\
									RSTP_DBUS_NAME,				\
									RSTP_DBUS_OBJPATH,			\
									RSTP_DBUS_INTERFACE,		\
									RSTP_DBUS_METHOD_CONFIG_MCHECK);

	dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&mstid,
								DBUS_TYPE_UINT32,&port_index,
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
		if(STP_DISABLE == ret){
			vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
		}
		else if(STP_PORT_NOT_ENABLED == ret) {
			vty_out(vty,PRINTF_PORT_NOT_ENABLED);
		}
	}
	else {		
		vty_out(vty," Failed get args.\n");		
		if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(stp_dbus_bridge_port_noconfig_fun,
		stp_bridge_port_noconfig_cmd,
		"config spanning-tree eth-port PORTNO default ",
		CONFIG_STR
		STP_STR
		"Config spanning-tree eth-port \n"
		CONFIG_ETHPORT_STR
		"Config spanning-tree default value\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int re_slot_port,ret;
	unsigned char slot_no,port_no;
	unsigned int port_index,mstid = 0;

	
	if(argc > 2)
	{
		vty_out(vty,"command parameters error!\n");
		return CMD_WARNING;
	}
	
	ret = parse_slotport_no((unsigned char *)argv[0],&slot_no,&port_no);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"Unknown port number format!\n");
		return CMD_SUCCESS;
	}
	
	dbus_error_init(&err);

    if(CMD_SUCCESS != dcli_get_one_port_index(vty,slot_no,port_no,&port_index))	{
		DCLI_DEBUG(("execute command failed\n"));
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(			\
									RSTP_DBUS_NAME,		\
									RSTP_DBUS_OBJPATH,	\
									RSTP_DBUS_INTERFACE, \
									RSTP_DBUS_METHOD_CONFIG_PORT_NOCONFIG);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
									DBUS_TYPE_UINT32,&port_index,							 							 		
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
		if(STP_DISABLE == ret){
			vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
		}
		else if(STP_PORT_NOT_ENABLED == ret) {
			vty_out(vty,PRINTF_PORT_NOT_ENABLED);
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

DEFUN(stp_show_spanning_tree,
		stp_show_spanning_tree_cmd,
		"show spanning-tree ",
		SHOW_STR
		"Display spanning-tree information\n"
		
)
{

	SLOT_INFO slot_info[6];
	unsigned int port_index = 0;
	int i,j,ret = 0;
	int product_id = 0;
	unsigned char slot = 0,port = 0;
	unsigned char product_flag = 0;
	unsigned int tmpval[2] = {0};
    PORT_MEMBER_BMP portbmp;
  
    memset(&portbmp,0,sizeof(PORT_MEMBER_BMP));

	/*  get product_id */
	if(CMD_SUCCESS != dcli_stp_get_bord_product_id(vty,&product_id)){
		return CMD_WARNING;
	}
	
	/*get slot count*/
	if(CMD_SUCCESS !=  dcli_get_all_ports_index(vty,&portbmp))
	{
		vty_out(vty,"execute command failed \n");
		return CMD_WARNING;
	}
	vty_out(vty,"portbmp[0]=%x  portbmp[1]=%x\n",portbmp.portMbr[0],portbmp.portMbr[1]);
	/*
	 * Get bridge info  
	 */	
	if((ret = dcli_get_br_info(vty)))
	{
		if(STP_DISABLE == ret)
		{
				vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
				return CMD_WARNING;
		}
		else 
		{
			vty_out(vty,"execute command failed \n");
			return CMD_WARNING;
		}
	
	}

	
	vty_out(vty,"\n----------------------All ports information of STP domain 0-----------------------------------\n");	
	
	vty_out(vty,"%-16s%-4s%-9s%-5s%-12s%-3s%-4s%-5s%-20s%-10s%-10s\n","Name","pri","cost","role","span-state","lk","p2p","edge","Desi-bridge-id","Dcost","D-port");

	for (i = 0; i < 64; i++) 
	{	
		if(is_distributed == DISTRIBUTED_SYSTEM)
		{
			slot = 0xEE;            /*SLOT = 0x EE just used for get slot num from NPD*/
			port = i+1;
		}
		else if((PRODUCT_ID_AX7K_I == product_id) || 
			(PRODUCT_ID_AX7K == product_id)) {
			slot = i/8 + 1;
			port = i%8;
			
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
				(PRODUCT_ID_AU4K == product_id) ||
				(PRODUCT_ID_AU3K == product_id) ||
				(PRODUCT_ID_AU3K_BCM == product_id)||
				(PRODUCT_ID_AU3K_BCAT == product_id)||
				(PRODUCT_ID_AU2K_TCAT == product_id)) {
			slot = 1;
			port = i;
		}
		
		tmpval[i/32] = (1<<(i%32));
		if(portbmp.portMbr[i/32]& tmpval[i/32]) {
            if(is_distributed == DISTRIBUTED_SYSTEM)
            {
    			if(CMD_SUCCESS != dcli_get_one_port_index(vty,slot,port,&port_index))
				{
    				DCLI_DEBUG(("execute command failed\n"));
    				continue;
    			}
			    slot =(((port_index) & 0x000007C0)>>6) + 1;
  				vty_out(vty,"%-d/%-14d",slot,port);

    			/*printf("port -index %d",port_index);*/
    			if(dcli_get_one_port_info(vty,port_index,product_id))/*return value not is CMD_SUCCESS*/
    			{
    				vty_out(vty,"execute command failed \n");
    				return CMD_WARNING;
    			}				
            }
			else
			{			
    			if(PRODUCT_ID_AX7K_I == product_id) {
    				vty_out(vty,"cscd%d", port-1);
    			}
    			else {
    				vty_out(vty,"%-d/%-14d",slot,port);
    			}
    			if(CMD_SUCCESS != dcli_get_one_port_index(vty,slot,port,&port_index))	{
    				DCLI_DEBUG(("execute command failed\n"));
    				continue;
    			}
    			/*printf("port -index %d",port_index);*/
    			if(dcli_get_one_port_info(vty,port_index,product_id))/*return value not is CMD_SUCCESS*/
    			{
    				vty_out(vty,"execute command failed \n");
    				return CMD_WARNING;
    			}
			}
		}	
	}			
	return CMD_SUCCESS;
}

DEFUN(stp_show_spanning_tree_port,
		stp_show_spanning_tree_port_cmd,
		"show spanning-tree eth-port PORTNO",
		SHOW_STR
		"Display spanning-tree port information\n"
		CONFIG_ETHPORT_STR
		"Specify spanning-tree port slot/port) value\n"
		
)
{	
	unsigned int ret;
	unsigned char slot_no,port_no;
	unsigned int port_index;
	int product_id = 0;

	SLOT_INFO slot_info[6];
	
	if(argc > 1)
	{
		vty_out(vty,"command parameters error!\n");
		return CMD_WARNING;
	}
	ret = parse_slotport_no((unsigned char *)argv[0],&slot_no,&port_no);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"Unknown port number format!\n");
		return CMD_WARNING;
	}

	if(CMD_SUCCESS != dcli_get_one_port_index(vty,slot_no,port_no,&port_index))	{
		DCLI_DEBUG(("execute command failed\n"));
		return CMD_WARNING;
	}

	/*  get product_id */
	if(CMD_SUCCESS != dcli_stp_get_bord_product_id(vty,&product_id)){
		return CMD_WARNING;
	}
	#ifdef STP_FULL_DEBUG
	vty_out(vty,"SLOT_NO %d, LOCAL_PORT_NO %d, port_index %d \n",slot_no,port_no,port_index);
	#endif

	vty_out(vty,"%-16s%-4s%-9s%-5s%-12s%-3s%-4s%-5s%-20s%-10s%-10s\n","Name","pri","cost","role","span-state","lk","p2p","edge","Desi-bridge-id","Dcost","D-port");
	vty_out(vty,"%-d/%-10d    ",slot_no,port_no);
	if((ret = dcli_get_one_port_info(vty,port_index,product_id)))
	{
		if(STP_DISABLE == ret)
		{
				vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
				return CMD_WARNING;
		}
		else 
		{
			vty_out(vty,"execute command failed \n");
			return CMD_WARNING;
		}
	
	}

	return CMD_SUCCESS;
}


#if 0
DEFUN(stp_show_spanning_tree,
		stp_show_spanning_tree_cmd,
		"show spanning-tree ",
		CONFIG_STR
		SHOW_STR
		"Display  spanning-tree information\n"
		
)
{
	unsigned char slot_count;
	SLOT_INFO slot_info[6];
	int i,j;

	/*get slot count*/
	if((slot_count = dcli_get_all_ports_index(slot_info)) <0)
	{
		vty_out(vty,"execute command failed \n");
		return CMD_WARNING;
	}
	
	/*
	 * Get bridge info  
	 */	
	if(dcli_get_br_info())
	{
		vty_out(vty,"execute command failed \n");
		return CMD_WARNING;
	}
	
	vty_out(vty,"\n----------------------All ports information int STP domain 0-----------------------------------\n");	
	
	vty_out(vty,"%-16s%-4s%-9s%-5s%-12s%-3s%-4s%-5s%-20s%-10s%-10s\n","Name","pri","cost","role","span-state","lk","p2p","edge","Desi-bridge-id","Dcost","D-port");

	for (i = 0; i < slot_count; i++) 
	{	
		for( j = 0; j < slot_info[i].local_port_count; j++)
		{
			vty_out(vty,"%-d/%-10d    ",slot_info[i].slot_no,slot_info[i].port_no[j].local_port_no);
			if(dcli_get_one_port_info(slot_info[i].port_no[j].port_index))
			{
				vty_out(vty,"execute command failed \n");
				return CMD_WARNING;
			}
		}	
	}			
	return CMD_SUCCESS;
}

DEFUN(stp_show_spanning_tree_port,
		stp_show_spanning_tree_port_cmd,
		"show spanning-tree eth-port PORTNO",
		CONFIG_STR
		SHOW_STR
		"Display spanning-tree port (slot/port) information\n"
		"Specify spanning-tree port slot/port) value\n"
		
)
{	
	unsigned int ret;
	unsigned char slot_no,port_no;
	unsigned int port_index;

	SLOT_INFO slot_info[6];
	
	if(argc > 1)
	{
		vty_out(vty,"command parameters error!\n");
		return CMD_WARNING;
	}
	ret = parse_slotport_no((unsigned char *)argv[0],&slot_no,&port_no);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"Unknown port number format!\n");
		return CMD_WARNING;
	}

	if(dcli_get_one_port_index(slot_no,port_no,&port_index) < 0)	{
		DCLI_DEBUG(("execute command failed\n"));
		return CMD_WARNING;
	}

	#ifdef STP_FULL_DEBUG
	vty_out(vty,"SLOT_NO %d, LOCAL_PORT_NO %d, port_index %d \n",slot_no,port_no,port_index);
	#endif

	vty_out(vty,"%-16s%-4s%-9s%-5s%-12s%-3s%-4s%-5s%-20s%-10s%-10s\n","Name","pri","cost","role","span-state","lk","p2p","edge","Desi-bridge-id","Dcost","D-port");
	vty_out(vty,"%-d/%-10d    ",slot_no,port_no);
	if(dcli_get_one_port_info(port_index))
	{
		vty_out(vty,"execute command failed \n");
		return CMD_WARNING;
	}

	return CMD_SUCCESS;
}
#endif

/******************************************************************************************
*MSTP command
*
******************************************************************************************/
#if 0
DEFUN(config_spannibg_tree_mst_cmd_func,
	config_spannibg_tree_mst_cmd,
	"config spanning-tree (enable|disable)",
	CONFIG_STR
	STP_STR
	"Enable spanning-tere \n"
	"Disable spanning-tere \n"
	)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int isEnable = 0;
	int ret,op_ret,i;
	VLAN_PORTS_BMP ports_bmp = {0	};
	VLAN_PORTS_BMP *bmpPtr = NULL,*tmp = NULL;
	
	unsigned int count = 0;

	if(argc > 1)
	{
		vty_out(vty,"command parameters error\n");
		return CMD_WARNING;
	}
	
	if(strcmp(argv[0],"enable")==0)
	{
		isEnable = 1;
	}
	else if (strcmp(argv[0],"disable")==0)
	{
		isEnable = 0;
	}
	else
	{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}

	dcli_enable_g_stp_to_protocol(vty,DCLI_MST_M,isEnable);

	if(STP_NODE == vty->node) {
		dcli_set_bridge_force_version(vty,DCLI_STP_M,STP_FORCE_VERS);
	}
	else if(MST_NODE == vty->node) {
		dcli_set_bridge_force_version(vty,DCLI_MST_M,MST_FORCE_VERS);
	}

	return CMD_SUCCESS;

}


DEFUN(config_spannibg_tree_mst_ethport_cmd_func,
	config_spannibg_tree_mst_ethport_cmd,
	"config spanning-tree eth-port PORTNO (enable|disable)",
	CONFIG_STR
	STP_STR
	"Ethernet port infomation\n"
	"Specify spanning-tree eth-port value (slot/port) \n"
	"Enable spanning-tere on the port\n"
	"Disable spanning-tere on the port\n"
	)
{
	DBusMessage *query, *reply;
	DBusError err;
	struct interface * ethIntf = NULL;
	unsigned char slot_no = 0,port_no = 0;
	unsigned int isEnable = 0;
	unsigned int port_index;
	int ret,op_ret;

	if(argc > 2)
	{
		vty_out(vty,"command parameters error\n");
		return CMD_WARNING;
	}

	ret = parse_slotport_no((unsigned char *)argv[0],&slot_no,&port_no);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"Unknow port number format.\n");
		return CMD_WARNING;
	}

	if(dcli_get_one_port_index(slot_no,port_no,&port_index) < 0)
	{
		//DCLI_DEBUG(("execute command failed\n");
		return CMD_WARNING;
	}
	

	if(strcmp(argv[1],"enable")==0)
	{
		isEnable = 1;
	}
	else if (strcmp(argv[1],"disable")==0)
	{
		isEnable = 0;
	}
	else
	{
		vty_out(vty,"bad command parameter!\n");
		return CMD_WARNING;
	}		

#ifdef STP_FULL_DEBUG
	DCLI_DEBUG(("parse slot %d,port %d,port_index %d\n",slot_no,port_no,port_index));
#endif

	if(dcli_enable_stp_on_one_port(port_index,isEnable)<0)
	{
		DCLI_DEBUG((" execute command failed\n"));
		return CMD_WARNING;
	}

	return CMD_SUCCESS;

}
#endif

DEFUN(mst_dbus_bridge_name_func,
		mst_bridge_name_cmd,
		"config spanning-tree region-name NAME",
		CONFIG_STR
		STP_STR
		"Config spanning-tree region name\n"
		"Specify name must be limited to 31 characters \n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int re_value,ret;
	unsigned int value;
	char* name = g_regionName;
	
	value = strlen (argv[0]);
	if(value > 31){
		vty_out(vty,"input name too many characters\n");
		return  CMD_WARNING;
	}
	memset(name,'\0',32);
	strcpy(name,argv[0]);

	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									MSTP_DBUS_METHOD_CONFIG_REG_NAME);

    dbus_message_append_args(query,
									DBUS_TYPE_STRING,&name,
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

	#ifdef STP_FULL_DEBUG
	vty_out(vty,"test recv message\n");
	#endif
	if (dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {
		#ifdef STP_FULL_DEBUG
		vty_out(vty," region name: %s	return-value %d\n",name,ret);
		#endif
		if(STP_DISABLE == ret){
			vty_out(vty,"MSTP not enable\n");
			return CMD_SUCCESS;
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

DEFUN(mst_dbus_bridge_revision_func,
		mst_bridge_revision_cmd,
		"config spanning-tree revision <0-61440>",
		CONFIG_STR
		STP_STR
		"Config spanning-tree revision\n"
		"Specify revision \n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int re_value,ret;
	unsigned short value = 0;
	
	
	if((strtoul (argv[0],0,10) < MIN_BR_REVISION) || (strtoul (argv[0],0,10) > MAX_BR_REVISION )){
		vty_out(vty,"input revision value out range <0-65535>\n");
		return  CMD_WARNING;
	}
	else {
		value = strtoul (argv[0],0,10);
	}
	DCLI_DEBUG(("input revision %d\n",value));
	
	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									MSTP_DBUS_METHOD_CONFIG_BR_REVISION);

    dbus_message_append_args(query,
									DBUS_TYPE_UINT16,&value,
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
		DCLI_DEBUG(("input value %d, return value %d\n",value,ret));
		if(STP_DISABLE == ret){
			vty_out(vty,"MSTP not enable\n");
			return CMD_SUCCESS;
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


DEFUN(mst_dbus_bridge_priority_func,
		mst_bridge_priority_cmd,
		"config spanning-tree MSTID priority <0-61440>",
		CONFIG_STR
		STP_STR
		MST_STR
		"Config spanning-tree priority\n"
		"Specify priority value must be 4096 times \n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int re_value,ret;
	unsigned int mstid = 0,value = 0;

	mstid = strtoul(argv[0],0,10);
	value = strtoul(argv[1],0,10);
	if(value < MIN_BR_PRIO || value > MAX_BR_PRIO  || 
		mstid < MIN_MST_ID || mstid > MAX_MST_ID){
		vty_out(vty,"input priority value out range \n");
		return  CMD_WARNING;
	}
	else{
		if(0 != (value%4096)){
			vty_out (vty,"spanning-tree priority value must be times of 4096!\n");
			return CMD_WARNING;
		}
	}
	
	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,    \
									RSTP_DBUS_OBJPATH,    \
									RSTP_DBUS_INTERFACE,    \
									RSTP_DBUS_METHOD_CONFIG_PRIO);

    dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
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
		if(STP_DISABLE == ret){
			vty_out(vty,"MSTP hasn't enabled\n");
			dbus_message_unref(reply);
			return CMD_WARNING;
		}
		else if(STP_Vlan_Had_Not_Yet_Been_Created == ret){
			vty_out(vty,"STP does not exist yet!\n");
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


DEFUN(mst_dbus_bridge_maxhops,
		mst_bridge_maxhops_cmd,
		"config spanning-tree max-hops <6-40>",
		CONFIG_STR
		STP_STR
		"Config spanning-tree max hops\n"
		"Specify max-hops value\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int re_value , ret;
	unsigned int value=0;
	
	value = strtoul (argv[0],0,10);
	if(value < MIN_REMAINING_HOPS || value > MAX_REMAINING_HOPS )
	{
		vty_out(vty,"input max-hops value out range <6-40>\n");
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(		\
									RSTP_DBUS_NAME,	\
									RSTP_DBUS_OBJPATH,	\
									RSTP_DBUS_INTERFACE,	\
									MSTP_DBUS_METHOD_CONFIG_MAXHOPS);

	dbus_error_init(&err);
	dbus_message_append_args(query,
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
		if(STP_DISABLE == ret){
			vty_out(vty,"MSTP hasn't enabled\n");
			return CMD_SUCCESS;
		}
		else if(STP_Small_Max_Hops == ret){
            vty_out(vty,PRINTF_STP_Small_Max_Hops);
		}
		else if(STP_Large_Max_Hops == ret){
            vty_out(vty,PRINTF_STP_Large_Max_Hops);
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

DEFUN(mst_dbus_bridge_port_pathcost_fun,
		mst_bridge_port_pathcost_cmd,
		"config spanning-tree MSTID eth-port PORTNO path-cost (auto|<1-200000000>)",
		CONFIG_STR
		STP_STR
		MST_STR
		"Config spanning-tree eth-port\n"
		CONFIG_ETHPORT_STR
		"config  spanning-tree path-cost \n"
		"Path cost value auto\n"
		"Specify path cost value (VALUE)\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int value,mstid = 0;
	unsigned char slot_no=0;
	unsigned char port_no=0;
	unsigned int ret;
	unsigned int port_index;
	
	if(argc > 3)
	{
		vty_out(vty,"command parameters error!\n");
		return CMD_WARNING;
	}

	mstid =  strtoul (argv[0],0,10);
	if(mstid < MIN_MST_ID || mstid > MAX_MST_ID) {
		vty_out(vty,"input para out range.\n");
		return CMD_WARNING;
	}
	ret = parse_slotport_no((unsigned char *)argv[1],&slot_no,&port_no);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"Unknow port number format.\n");
		return CMD_SUCCESS;
	}
	
	if (strncmp("auto",argv[2] , strlen(argv[2]))== 0){
			value = 200000000;
	}
	else {
		value = strtoul (argv[2],0,10);
		if (!((value > 0)&&(value % 20 ==0))){
			vty_out (vty,"path cost must be times of 20!\n");
			return CMD_WARNING;
		}
		else if(ADMIN_PORT_PATH_COST_AUTO == value){
            vty_out(vty,"The path-cost 200000000 is auto value!");
		}		
	}
	if(CMD_SUCCESS != dcli_get_one_port_index(vty,slot_no,port_no,&port_index))	{
		DCLI_DEBUG(("execute command failed\n"));
		return CMD_WARNING;
	}

	DCLI_DEBUG(("mstid %d, slot/port %d/%d,value %d\n",mstid,slot_no,port_no,value));
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
										RSTP_DBUS_NAME,		\
										RSTP_DBUS_OBJPATH,	\
										RSTP_DBUS_INTERFACE, \
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
		if(STP_DISABLE == ret){
			vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
		}
		else if(STP_PORT_NOT_ENABLED == ret) {
			vty_out(vty,PRINTF_PORT_NOT_ENABLED);
		}
		else if(STP_Cannot_Find_Vlan == ret){
            vty_out(vty,"Port not in the mst!\n");
		}
		#if 0
		else if(0 == ret)
			dcli_stp_set_port_pathcost_to_npd(vty,mstid,port_index,value);
		#endif
		
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

DEFUN(mst_dbus_bridge_port_prio_fun,
		mst_bridge_port_prio_cmd,
		"config spanning-tree MSTID eth-port PORTNO priority <0-240>",
		CONFIG_STR
		STP_STR
		MST_STR
		"Config spanning-tree eth-port  \n"
		CONFIG_ETHPORT_STR
		"Config spanning-tree eth-port priority value \n"
		"Specify port priority value must be 16 times \n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int port_index;
	unsigned int value,mstid = 0;
	unsigned char slot_no,port_no;
	unsigned int ret;

	if(argc > 3)
	{
		vty_out(vty,"command parameters error\n");
		return CMD_WARNING;
	}

	mstid =  strtoul (argv[0],0,10);
	if(mstid < MIN_MST_ID || mstid > MAX_MST_ID) {
		vty_out(vty,"input para out range.\n");
		return CMD_WARNING;
	}
	ret = parse_slotport_no((unsigned char *)argv[1],&slot_no,&port_no);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"Unknow port number format.\n");
		return CMD_WARNING;
	}
	
	value = strtoul (argv[2],0,10);
	if(value < MIN_PORT_PRIO || value > MAX_PORT_PRIO ){
		vty_out(vty,"input port-priority value out range <0-240>\n");
		return CMD_WARNING;
	}
	else{
		if(0 != (value%16)){
			vty_out (vty,"spanning-tree port-priority value must be times of 16!\n");
			return CMD_WARNING;
		}
	}

	if(CMD_SUCCESS != dcli_get_one_port_index(vty,slot_no,port_no,&port_index))	{
		DCLI_DEBUG(("execute command failed\n"));
		return CMD_WARNING;
	}

	DCLI_DEBUG(("mstid %d, slot/port %d/%d,value %d\n",mstid,slot_no,port_no,value));
	dbus_error_init(&err);	
	query = dbus_message_new_method_call(				\
										RSTP_DBUS_NAME,		\
										RSTP_DBUS_OBJPATH,	\
										RSTP_DBUS_INTERFACE, \
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
		if(STP_DISABLE == ret){
			vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
		}
		else if(STP_PORT_NOT_ENABLED == ret) {
			vty_out(vty,PRINTF_PORT_NOT_ENABLED);
		}
		else if(STP_Cannot_Find_Vlan == ret){
            vty_out(vty,"Port not in the mst!\n");
		}
		#if 0
		else if(0 == ret)
			dcli_stp_set_port_prio_to_npd(vty,mstid,port_index,value);
		#endif
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

DEFUN(mst_dbus_bridge_noconfig_fun,
		mst_bridge_noconfig_cmd,
		"config spanning-tree MSTID default ",
		CONFIG_STR
		STP_STR
		MST_STR
		"Config spanning-tree default value\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int value = 0 ,ret;

	value = strtoul (argv[0],0,10);
	if (value < MIN_MST_ID || value > MAX_MST_ID ) {
		vty_out (vty,"input param out range!\n");
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(				\
									RSTP_DBUS_NAME,			\
									RSTP_DBUS_OBJPATH,		\
									RSTP_DBUS_INTERFACE,	\
									RSTP_DBUS_METHOD_CONFIG_NOCONFIG);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&value,
							DBUS_TYPE_INVALID);

	DCLI_DEBUG(("mstid %d\n",value));
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
		if(STP_DISABLE == ret){
			vty_out(vty,PRINTF_RSTP_NOT_ENABLED);
		}
		else if(STP_PORT_NOT_ENABLED == ret) {
			vty_out(vty,PRINTF_PORT_NOT_ENABLED);
		}
	}	
	else {
		
		vty_out(vty," Failed get args.\n");
		if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
		}
	}
		
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
}


DEFUN(mst_dbus_bridge_port_noconfig_fun,
		mst_bridge_port_noconfig_cmd,
		"config spanning-tree MSTID eth-port PORTNO default ",
		CONFIG_STR
		STP_STR
		MST_STR
		"Config spanning-tree eth-port \n"
		CONFIG_ETHPORT_STR
		"Config spanning-tree default value\n"
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	unsigned char slot_no,port_no;
	unsigned int port_index,mstid = 0;

	
	if(argc > 2)
	{
		vty_out(vty,"command parameters error!\n");
		return CMD_WARNING;
	}

	mstid =  strtoul (argv[0],0,10);
	if(mstid < MIN_MST_ID || mstid > MAX_MST_ID) {
		vty_out(vty,"input para out range.\n");
		return CMD_WARNING;
	}
	
	ret = parse_slotport_no((unsigned char *)argv[1],&slot_no,&port_no);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"Unknown port number format!\n");
		return CMD_SUCCESS;
	}
	
	dbus_error_init(&err);

if(CMD_SUCCESS != dcli_get_one_port_index(vty,slot_no,port_no,&port_index))	{
		DCLI_DEBUG(("execute command failed\n"));
		return CMD_WARNING;
	}

	DCLI_DEBUG(("mstid %d, slot/port %d/%d\n",mstid,slot_no,port_no));
	query = dbus_message_new_method_call(			\
									RSTP_DBUS_NAME,		\
									RSTP_DBUS_OBJPATH,	\
									RSTP_DBUS_INTERFACE, \
									RSTP_DBUS_METHOD_CONFIG_PORT_NOCONFIG);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&mstid,
									DBUS_TYPE_UINT32,&port_index,				 		
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
		if(STP_DISABLE == ret){
			vty_out(vty,PRINTF_MSTP_NOT_ENABLED);
		}
		else if(STP_PORT_NOT_ENABLED == ret) {
			vty_out(vty,"This instance not exist\n");
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

DEFUN(mst_set_vlan_to_mst_func,
		mst_set_vlan_to_mst_cmd,
		"config spanning-tree map <2-4093> instance <0-63>" ,
		CONFIG_STR
		STP_STR
		"Vlan map to mst instance\n"
		"Specify vlanid range\n"
		"Mstp instance\n"
		MST_STR
		
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	unsigned short vid = 0;
	unsigned int mstid = 0;
	VLAN_PORTS_BMP ports_bmp = {0}; 

	if(argc > 2)
	{
		vty_out(vty,"command parameters error!\n");
		return CMD_WARNING;
	}

	vid =  strtoul(argv[0],0,10);
	if(vid < 2 || vid > 4093) {
		vty_out(vty,"input vid out  of range.\n");
		return CMD_WARNING;
	}

	ret = parse_int_parse((char *)argv[1] ,&mstid);
    if(NPD_FAIL == ret){
      vty_out(vty,"instance form error !\n");
	}
	if(mstid < MIN_MST_ID || mstid > MAX_MST_ID) {
		vty_out(vty,"input mstid out of range.\n");
		return CMD_WARNING;
	}

	/*ret = dcli_get_one_vlan_portmap(vty,vid,&ports_bmp);
	if(CMD_SUCCESS != ret){
		vty_out(vty,"The vlan not exist or can not get the vlan\n");
		return CMD_WARNING;
	}*/

	dbus_error_init(&err);
	DCLI_DEBUG(("set vid %d on mstid %d\n",vid,mstid));
	query = dbus_message_new_method_call(			\
									RSTP_DBUS_NAME,		\
									RSTP_DBUS_OBJPATH,	\
									RSTP_DBUS_INTERFACE, \
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

		if(STP_DISABLE == ret){
			vty_out(vty,PRINTF_MSTP_NOT_ENABLED);
		}
		else if(STP_Cannot_Find_Vlan == ret){
			vty_out(vty,"The vlan %d not exist!\n ",vid);
		}
		else if(0 == ret) {
			dcli_stp_set_stpid_to_npd(vty,vid,mstid);
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

DEFUN(mst_show_spanning_all_instance_func,
		mst_show_spanning_tree_all_instance_cmd,
		"show spanning-tree instance",
		SHOW_STR
		STP_STR
		"Display spanning-tree MST instance\n"		
)
{	
	unsigned int ret;
	unsigned short vid = 0;
	unsigned int mstid = 0;

	


	/*get portbmp*/
	/*get bridge info*/
	/*get port info*/
	return CMD_SUCCESS;
}


DEFUN(mst_show_spanning_one_instance_func,
		mst_show_spanning_tree_one_instance_cmd,
		"show spanning-tree instance MSTID",
		SHOW_STR
		STP_STR
		"Display spanning-tree MST instance\n"	
		MST_STR
)
{	
	int ret,i,j;
	unsigned short *pvid = NULL,*tmp =NULL;
	unsigned int mstid = 0,num = 0;
	VLAN_PORTS_BMP ports_bmp ; 
	unsigned int slot = 0,port = 0,count = 0,port_index = 0;
	unsigned char product_flag = 0;
	unsigned int tmpVal[2] = {0};
    PORT_MEMBER_BMP portsmap;
	unsigned int product_id = 0;
	
	memset(&ports_bmp,0,sizeof(VLAN_PORTS_BMP));
    memset(&portsmap,0,sizeof(PORT_MEMBER_BMP));
	if(argc > 1)
	{
		vty_out(vty,"command parameters error!\n");
		return CMD_WARNING;
	}	
	mstid =  strtoul (argv[0],0,10);
	if(mstid < MIN_MST_ID || mstid > MAX_MST_ID) {
		vty_out(vty,"input param out range.\n");
		return CMD_WARNING;
	}

	/*  get product_id */
	if(CMD_SUCCESS != dcli_stp_get_bord_product_id(vty,&product_id)){
		return CMD_WARNING;
	}
	/*get bridge info*/
	
	ret = dcli_get_br_self_info(vty,mstid, &pvid, &num);
	/*DCLI_DEBUG(("dcli_stp 1774:: show instance mstid %d,num %d\n",mstid,num));*/
	if(CMD_SUCCESS == ret) {
		if(0 == mstid) {
			ret |= dcli_get_cist_info(vty,mstid);
		}
		/*DCLI_DEBUG(("dcli_stp 1853:: dcli_get_msti_info\n"));*/
		dcli_get_msti_info(vty,mstid);
			
		/*get port info*/
		/*DCLI_DEBUG(("dcli_stp 1857:: port info \n"));*/
		vty_out(vty,"\n----------------------All ports information of MSTP domain %d-----------------------------------\n",mstid);		
		vty_out(vty,"%-12s%-4s%-4s%-9s%-5s%-12s%-3s%-4s%-5s%-20s%-10s%-10s\n","Name","vid","pri","cost","role","span-state","lk","p2p","edge","Desi-bridge-id","Dcost","D-port");
		if(0 != mstid) {
			tmp = pvid;
			for(j = 0; j < num; j++) {
			 	ret |= dcli_get_one_vlan_portmap(vty,*tmp,&ports_bmp);
				/*DCLI_DEBUG(("dcli_stp 1804:: show instance vid %d\n",*tmp));*/
				if(CMD_SUCCESS == ret) {
					portsmap.portMbr[0]= (ports_bmp.tagbmp.portMbr[0]| ports_bmp.untagbmp.portMbr[0]);
                    portsmap.portMbr[1]= (ports_bmp.tagbmp.portMbr[1]| ports_bmp.untagbmp.portMbr[1]);
					/*DCLI_DEBUG(("dcli_stp 1804:: show instance vid %d,portsmap %02x,\n",*tmp,portsmap));*/
					if(is_distributed == DISTRIBUTED_SYSTEM)
					{
						for (i=0;i<64;i++)
						{
							slot = 0xEE;            /*SLOT = 0x EE just used for get slot num for NPD*/
							port = i+1;

							tmpVal[i/32] = (1<<(i%32));
							if(portsmap.portMbr[i/32]& tmpVal[i/32]) 
							{	
								if(CMD_SUCCESS != dcli_get_one_port_index(vty,slot,port,&port_index))
									{
    									DCLI_DEBUG(("execute command failed\n"));
    									continue;
    								}
			    					slot =(((port_index) & 0x000007C0)>>6) + 1;
  									vty_out(vty,"%-d/%-14d",slot,port);

    								/*printf("port -index %d",port_index);*/
    								ret |= dcli_get_mstp_one_port_info(vty,mstid,port_index);
							}
						}
					}
					else
					{
						show_slot_port_by_productid(vty,product_id,&portsmap,*tmp,mstid);
					}
#if 0
					for (i=0;i<32;i++) {
						slot = i/8 + 1;
						port = i%8;
						tmpVal = (1<<i);
						if(portsmap & tmpVal) {				
							vty_out(vty,"%-d/%-10d",slot,port);
							vty_out(vty,"%-4d",*tmp);
							if(dcli_get_one_port_index(vty,slot,port,&port_index) < 0)	{
								DCLI_DEBUG(("execute command failed\n"));
								continue;
							}
							dcli_get_mstp_one_port_info(vty,mstid,port_index);
						}
					}
#endif
					tmp++;
					memset(&ports_bmp,0,sizeof(VLAN_PORTS_BMP));
				}
			}
		}
		else {
			if((ret |= dcli_change_all_ports_to_bmp(vty,&ports_bmp,&count)) < 0){
				
				DCLI_DEBUG(("execute command failed\n"));
				return CMD_WARNING;
			}
			else{
				portsmap.portMbr[0] = (ports_bmp.tagbmp.portMbr[0]| ports_bmp.untagbmp.portMbr[0]);
				portsmap.portMbr[1] = (ports_bmp.tagbmp.portMbr[1]| ports_bmp.untagbmp.portMbr[1]);
				/*DCLI_DEBUG(("dcli_stp 1804:: show instance vid %d,portsmap %02x,\n",*tmp,portsmap));*/
				for (i=0;i<64;i++) {
					if(is_distributed == DISTRIBUTED_SYSTEM){
						slot = 0xEE;            /*SLOT = 0x EE just used for get slot num for NPD*/
						port = i+1;
					}
					else if(PRODUCT_ID_AX7K == product_id){
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
							(PRODUCT_ID_AX5608 == product_id) ||
							(PRODUCT_ID_AU4K == product_id) ||
							(PRODUCT_ID_AU3K == product_id) ||
							(PRODUCT_ID_AU3K_BCM == product_id)||
							(PRODUCT_ID_AU3K_BCAT == product_id)||
							(PRODUCT_ID_AU2K_TCAT == product_id)) {
						slot = 1;
						port = i;
					}

				
					tmpVal[i/32] = (1<<(i%32));
					if(portsmap.portMbr[i/32]& tmpVal[i/32]) {	
						if(is_distributed == DISTRIBUTED_SYSTEM)
           				{
    						if(CMD_SUCCESS != dcli_get_one_port_index(vty,slot,port,&port_index))
							{
    							DCLI_DEBUG(("execute command failed\n"));
    							continue;
    						}
			    			slot =(((port_index) & 0x000007C0)>>6) + 1;
  							vty_out(vty,"%-d/%-14d",slot,port);

    						/*printf("port -index %d",port_index);*/
    						ret |= dcli_get_mstp_one_port_info(vty,mstid,port_index);
						}
						else
						{
							if(PRODUCT_ID_AX7K_I == product_id) {
								/*vty_out(vty,"cscd%d", port-1);*/
								vty_out(vty,"%-d/%-10d",slot,port);
							}
							else {
								vty_out(vty,"%-d/%-10d",slot,port);
							}
							vty_out(vty,"%-4s"," ");
							if((ret |= dcli_get_one_port_index(vty,slot,port,&port_index)) < 0)	{
							DCLI_DEBUG(("execute command failed\n"));
							continue;
							}
						ret |= dcli_get_mstp_one_port_info(vty,mstid,port_index);
						}
					}
				}
			}
		}
	}
	if(0 != num)
		free(pvid);
				
	else if(STP_DISABLE == ret)
		vty_out(vty,"MSTP hasn't enabled\n");
	else if(DCLI_STP_NO_SUCH_MSTID == ret)
		vty_out(vty," hasn't the instance\n");
	
	return CMD_SUCCESS;
}


/******************************************************************************************
*	common command
******************************************************************************************/
DEFUN(debug_spanning_tree ,
		debug_spanning_tree_cmd,
		"debug spanning-tree (all|error|warning|debug|event|protocol|packet_send|packet_receive|packet_all)",
		DEBUG_STR
		"debus spanning-tree parameter \n"
		"debug spanning-tree all \n"
		"debug spanning-tree error \n"
		"debug spanning-tree warning \n"
		"debug spanning-tree debug \n"
		"debug spanning-tree event \n"
		"debug spanning-tree protocol \n"
		"debug spanning-tree packet send \n"
		"debug spanning-tree packet received \n"
		"debug spanning-tree packet send and receive \n"       
)
{	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	unsigned int flag;
	
	if(argc > 1) {
		vty_out(vty,"command parameters error!\n");
		return CMD_WARNING;
	}
	
	
	if(0 == strcmp(argv[0],"all")) {
		flag = STP_DEBUG_FLAG_ALL;
	}
	else if(0 == strcmp(argv[0],"error")) {
		flag = STP_DEBUG_FLAG_ERR;
	}
	else if(0 == strcmp(argv[0],"warning")) {
		flag = STP_DEBUG_FLAG_WAR;
	}
	else if(0 == strcmp(argv[0],"debug")) {
		flag = STP_DEBUG_FLAG_DBG;
	}
	else if(0 == strcmp(argv[0],"event")) {
		flag = STP_DEBUG_FLAG_EVT;
	}
	else if(0 == strcmp(argv[0],"packet_send")) {
		flag = STP_DEBUG_FLAG_PKT_SED;
	}
	else if(0 == strcmp(argv[0],"packet_receive")) {
		flag = STP_DEBUG_FLAG_PKT_REV;
	}
	else if(0 == strcmp(argv[0],"packet_all")) {
		flag = STP_DEBUG_FLAG_PKT_ALL;
	}
	else if(0 == strcmp(argv[0],"protocol")) {
		flag = STP_DEBUG_FLAG_PROTOCOL;
	}
	else {
		vty_out(vty,"command parameter error!\n");
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(RSTP_DBUS_NAME,RSTP_DBUS_OBJPATH,RSTP_DBUS_INTERFACE,RSTP_DBUS_METHOD_DEBUG_SPANNTREE);
    dbus_error_init(&err);
	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&flag,
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
		#ifdef STP_FULL_DEBUG		
		vty_out(vty," return-value:%d\n",ret);
		#endif
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

DEFUN(no_debug_spanning_tree ,
		 no_debug_spanning_tree_cmd,
         "no debug spanning-tree (all|error|warning|debug|event|protocol|packet_send|packet_receive|packet_all)",
         "cancel the debug"
         DEBUG_STR
		 "no debus spanning-tree parameter \n"
		 "no debug spanning-tree all \n"
		 "no debug spanning-tree error \n"
		 "no debug spanning-tree warning \n"
		 "no debug spanning-tree debug \n"
		 "no debug spanning-tree event \n"
		 "no debug spanning-tree protocol \n"
		 "no debug spanning-tree packet send \n"
		 "no debug spanning-tree packet received \n"
	   	 "no debug spanning-tree packet send and receive \n"	

)
{	DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int flag;
	
	unsigned int ret;

	if(0 == strcmp(argv[0],"all")) {
		flag = STP_DEBUG_FLAG_ALL;
	}
	else if(0 == strcmp(argv[0],"error")) {
		flag = STP_DEBUG_FLAG_ERR;
	}
	else if(0 == strcmp(argv[0],"warning")) {
		flag = STP_DEBUG_FLAG_WAR;
	}
	else if(0 == strcmp(argv[0],"debug")) {
		flag = STP_DEBUG_FLAG_DBG;
	}
	else if(0 == strcmp(argv[0],"event")) {
		flag = STP_DEBUG_FLAG_EVT;
	}
	else if(0 == strcmp(argv[0],"packet_send")) {
		flag = STP_DEBUG_FLAG_PKT_SED;
	}
	else if(0 == strcmp(argv[0],"packet_receive")) {
		flag = STP_DEBUG_FLAG_PKT_REV;
	}
	else if(0 == strcmp(argv[0],"packet_all")) {
		flag = STP_DEBUG_FLAG_PKT_ALL;
	}
	else if(0 == strcmp(argv[0],"protocol")) {
		flag = STP_DEBUG_FLAG_PROTOCOL;
	}

	else {
		vty_out(vty,"Unknown command parameter!\n");
		return CMD_WARNING;
	}
	
	if(argc > 1)
	{
		vty_out(vty,"command parameters error!\n");
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(					\
										RSTP_DBUS_NAME,			\
										RSTP_DBUS_OBJPATH,		\
										RSTP_DBUS_INTERFACE,	\
										RSTP_DBUS_METHOD_NO_DEBUG_SPANNTREE);
    dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&flag,
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
		#ifdef STP_FULL_DEBUG			
		vty_out(vty," return-value: %d\n",ret);
		#endif
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

DEFUN(stp_ethport_endis_config_digest_snp_cmd_func,
	stp_ethport_endis_config_digest_snp_cmd,
	"config spanning-tree eth-port PORTNO digest-snooping (enable|disable)",
	CONFIG_STR
	STP_STR
	"Ethernet port infomation\n"
	CONFIG_ETHPORT_STR
	"Configuration digest snooping\n"
	"Enable spanning-tere on the port\n"
	"Disable spanning-tere on the port\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;
	
	unsigned char slot_no = 0,port_no = 0;
	unsigned int isEnable = 0;
	int port_index = 0;
	int ret = 0;
	
	if (argc > 2)
	{
		vty_out(vty, "command parameters error\n");
		return CMD_WARNING;
	}
	/* get the value of the enable */
	if (strncmp("enable", argv[1], strlen(argv[1]))==0)
	{
		isEnable = 1;
	}
	else if (strncmp("disable", argv[1], strlen(argv[1])) ==0)
	{
		isEnable = 0;
	}
	else
	{
		vty_out(vty, "bad command parameter!\n");
		return CMD_WARNING;
	}		

	ret = parse_slotport_no((unsigned char *)argv[0], &slot_no, &port_no);
	if (NPD_FAIL == ret) {
    	vty_out(vty,"Unknow port number format.\n");
		return  CMD_WARNING ;
	}
	
	if(CMD_SUCCESS != dcli_get_one_port_index(vty, slot_no, port_no, &port_index))
	{
		DCLI_DEBUG(("execute command failed\n"));
		return CMD_WARNING;
	}

	dbus_error_init(&err);	
	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,
									RSTP_DBUS_OBJPATH,
									RSTP_DBUS_INTERFACE,
									MSTP_DBUS_METHOD_PORT_ENDIS_CFG_DIGEST_SNP);

    dbus_message_append_args(query,
								DBUS_TYPE_UINT32, &port_index,
								DBUS_TYPE_UINT32, &isEnable,
								DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args(reply, &err,
								DBUS_TYPE_UINT32, &ret,
								DBUS_TYPE_INVALID))
	{
		DCLI_DEBUG(("return value %d\n", ret));
		if (STP_DISABLE == ret) {
			vty_out(vty, "MSTP not enable\n");
			return CMD_SUCCESS;
		}else if (STP_PORT_NOTFOUND == ret) {
			vty_out(vty, "MSTP not enable\n");
		}
	}
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(mstp_bridge_config_digest_cmd1_func,
	mstp_bridge_config_digest_cmd1,
	"config spanning-tree digest STRING",
	CONFIG_STR
	STP_STR
	"Configuration digest\n"
	"User-defined string of the digest, format for the HEX string that range from 1 to 32 characters\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;

	unsigned char *digest_str = NULL;
	unsigned int str_size = 0;
	int ret = 0;
	
	if (argc > 2)
	{
		vty_out(vty, "command parameters error\n");
		return CMD_WARNING;
	}

	digest_str = (char *)malloc(32 + 1);
	memset(digest_str, 0, 32 + 1);

	str_size = strlen(argv[0]);
	if (str_size >= 32 + 1) {
		free(digest_str);
		vty_out(vty, "%% Bad parameter, maximum valid length of string is 32.\n");
		return CMD_WARNING;
	}
	memcpy(digest_str, argv[0], str_size);

	dbus_error_init(&err);	
	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,
									RSTP_DBUS_OBJPATH,
									RSTP_DBUS_INTERFACE,
									MSTP_DBUS_METHOD_CONFIG_BRIDGE_DIGEST_CONTENT);

    dbus_message_append_args(query,
								DBUS_TYPE_STRING, &digest_str,
								DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		free(digest_str);
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args(reply, &err,
								DBUS_TYPE_UINT32, &ret,
								DBUS_TYPE_INVALID))
	{
		DCLI_DEBUG(("return value %d\n", ret));
		if (STP_DISABLE == ret) {
			vty_out(vty, "MSTP not enable\n");
			free(digest_str);
			return CMD_SUCCESS;
		}else if (STP_PORT_NOTFOUND == ret) {
			vty_out(vty, "MSTP not enable\n");
		}
	}
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);

	free(digest_str);
	return CMD_SUCCESS;
}

DEFUN(mstp_bridge_config_digest_cmd2_func,
	mstp_bridge_config_digest_cmd2,
	"config spanning-tree digest default",
	CONFIG_STR
	STP_STR
	MST_STR
	"Configuration digest snooping\n"
	"Reset value of the digest, default value is empty string\n"
)
{
	DBusMessage *query = NULL;
	DBusMessage *reply = NULL;
	DBusError err;

	unsigned char *digest_str = NULL;
	unsigned int str_size = 0;
	int ret = 0;
	
	if (argc > 2)
	{
		vty_out(vty, "command parameters error\n");
		return CMD_WARNING;
	}

	digest_str = (char *)malloc(32 + 1);
	memset(digest_str, 0, 32 + 1);

	dbus_error_init(&err);	
	query = dbus_message_new_method_call(
									RSTP_DBUS_NAME,
									RSTP_DBUS_OBJPATH,
									RSTP_DBUS_INTERFACE,
									MSTP_DBUS_METHOD_CONFIG_BRIDGE_DIGEST_CONTENT);

    dbus_message_append_args(query,
								DBUS_TYPE_STRING, &digest_str,
								DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp, query, -1, &err);

	dbus_message_unref(query);
	if (NULL == reply)
	{
		vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty, "%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		free(digest_str);
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args(reply, &err,
								DBUS_TYPE_UINT32, &ret,
								DBUS_TYPE_INVALID))
	{
		DCLI_DEBUG(("return value %d\n", ret));
		if (STP_DISABLE == ret) {
			vty_out(vty, "MSTP not enable\n");
			free(digest_str);
			return CMD_SUCCESS;
		}else if (STP_PORT_NOTFOUND == ret) {
			vty_out(vty, "MSTP not enable\n");
		}
	}
	else {
		vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);

	free(digest_str);
	return CMD_SUCCESS;
}
#if 0
void dcli_stp_show_running_cfg(struct vty* vty)
{
	int state = 0;

	char _tmpstr[64];
	memset(_tmpstr,0,64);
	sprintf(_tmpstr,BUILDING_MOUDLE,"STP");
	vtysh_add_show_string(_tmpstr);

	DCLI_DEBUG(("npd Starting save stp configure\n"));
	
	dcli_stp_show_protocol_running_config(&state);

	DCLI_DEBUG(("###State %d\n", state));

	#if 0
	if(0 == state) {
		dcli_stp_show_npd_running_config();
	}
    #endif
	
	return state;
}

void dcli_stp_show_protocol_running_config
(
	int *state
) 
{	
	DCLI_DEBUG(("### Starting save config stp!\n"));
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	int ret = 0;
	
	query = dbus_message_new_method_call(   \
										RSTP_DBUS_NAME,			\
										RSTP_DBUS_OBJPATH,		\
										RSTP_DBUS_INTERFACE,	\
										RSTP_DBUS_METHOD_SHOW_STP_RUNNING_CFG);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show stp_protoco running config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) 
	{
		
		if(0 == ret) {
			
			vtysh_add_show_string(showStr);
		}
		
		*state = ret;
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
}
#else
int dcli_stp_show_running_cfg(struct vty* vty)
{
	DBusMessage *query, *reply;
	DBusError err;
	int i = 0;
	int ret = 0;
	int function_type = -1;
	char _tmpstr[64];
	char tmpstr[64];
	char file_path[64] = {0};
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	int slotNum = get_product_info(SEM_SLOT_COUNT_PATH);
    int local_slot_id = get_product_info(SEM_LOCAL_SLOT_ID_PATH);

	memset(_tmpstr,0,64);
	sprintf(_tmpstr,BUILDING_MOUDLE,"STP");
	vtysh_add_show_string(_tmpstr);
    DCLI_DEBUG(("### Starting save config stp!\n"));
	if(is_distributed == DISTRIBUTED_SYSTEM)
    {
		for(i = 1; i <= slotNum; i++)
		{
			sprintf(file_path,"/dbm/product/slot/slot%d/function_type", i);
			function_type = get_product_info(file_path);
			if (function_type == SWITCH_BOARD)
			{
                query = dbus_message_new_method_call(   \
										RSTP_DBUS_NAME,			\
										RSTP_DBUS_OBJPATH,		\
										RSTP_DBUS_INTERFACE,	\
										RSTP_DBUS_METHOD_SHOW_STP_RUNNING_CFG);
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
		            printf("show stp_protocol running config failed get reply.\n");
		            if (dbus_error_is_set(&err)) {
			            printf("%s raised: %s",err.name,err.message);
			            dbus_error_free_for_dcli(&err);
		            }
		            return CMD_WARNING;
	            }
	            if (dbus_message_get_args ( reply, &err,
					                        DBUS_TYPE_UINT32, &ret,
					                        DBUS_TYPE_STRING, &showStr,
					                        DBUS_TYPE_INVALID)) 
	            {
					if(ret == 0)
					{
						memset(tmpstr,0,64);
						sprintf(tmpstr,"config stp switch-board %d",i);
						vtysh_add_show_string(tmpstr);
    			        vtysh_add_show_string(showStr);
					}
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
			}
		}
	}
	return CMD_SUCCESS;
}
#endif
int dcli_stp_show_npd_running_config() 
{	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	
	query = dbus_message_new_method_call(   \
										NPD_DBUS_BUSNAME,			\
										NPD_DBUS_ETHPORTS_OBJPATH,		\
										NPD_DBUS_ETHPORTS_INTERFACE,		\
										RSTP_DBUS_METHOD_SHOW_STP_RUNNING_CFG);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection_stp,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
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
			dbus_error_free_for_dcli(&err);
		}
	}

	dbus_message_unref(reply);
}

void dcli_stp_element_init(void)  
{
	install_node (&stp_node_distributed, NULL,"STP_NODE_DISTRIBUTED");
    install_default(STP_NODE_DISTRIBUTED);
	install_node (&stp_node, dcli_stp_show_running_cfg, "STP_NODE");
	install_default(STP_NODE);

	install_node (&mst_node, NULL, "MST_NODE");
	install_default(MST_NODE);

    install_element(CONFIG_NODE,&config_stp_on_board_cmd);
	install_element(STP_NODE_DISTRIBUTED,&config_spannibg_tree_mode_cmd);
	

	install_element(MST_NODE,&config_spannibg_tree_stp_cmd);
	install_element(MST_NODE,&config_spannibg_tree_stp_ethport_cmd);
	install_element(MST_NODE,&mst_bridge_name_cmd);
	install_element(MST_NODE,&mst_bridge_revision_cmd);	
	install_element(MST_NODE,&mst_bridge_priority_cmd);
	install_element(MST_NODE,&mst_bridge_maxhops_cmd);	
	install_element(MST_NODE,&stp_bridge_maxage_cmd);
	install_element(MST_NODE,&stp_bridge_hetime_cmd);
	install_element(MST_NODE,&stp_bridge_fordelay_cmd);
	/*install_element(MST_NODE,&stp_bridge_forversion_cmd);*/
	install_element(MST_NODE,&mst_bridge_noconfig_cmd);
	install_element(MST_NODE,&mst_bridge_port_pathcost_cmd);
	install_element(MST_NODE,&mst_bridge_port_prio_cmd);
	install_element(MST_NODE,&stp_bridge_port_none_stp_cmd);
	install_element(MST_NODE,&stp_bridge_port_p2p_cmd);
	install_element(MST_NODE,&stp_bridge_port_edge_cmd);
	/*install_element(MST_NODE,&mst_bridge_port_none_stp_cmd);
	//install_element(MST_NODE,&mst_bridge_port_p2p_cmd);
	//install_element(MST_NODE,&mst_bridge_port_edge_cmd);
	//install_element(MST_NODE,&mst_bridge_port_mcheck_cmd);*/
	install_element(MST_NODE,&mst_bridge_port_noconfig_cmd);
	install_element(MST_NODE,&mst_set_vlan_to_mst_cmd);
	install_element(MST_NODE,&mst_show_spanning_tree_one_instance_cmd);
	install_element(MST_NODE,&stp_ethport_endis_config_digest_snp_cmd);
	install_element(MST_NODE,&mstp_bridge_config_digest_cmd1);
	install_element(MST_NODE,&mstp_bridge_config_digest_cmd2);
/*	install_element(VIEW_NODE,&mst_show_spanning_tree_one_instance_cmd);
//	install_element(ENABLE_NODE,&mst_show_spanning_tree_one_instance_cmd);
//	install_element(CONFIG_NODE,&mst_show_spanning_tree_one_instance_cmd);*/


	install_element(STP_NODE,&config_spannibg_tree_stp_cmd);
	install_element(STP_NODE,&config_spannibg_tree_stp_ethport_cmd);
	install_element(STP_NODE,&stp_bridge_priority_cmd);
	install_element(STP_NODE,&stp_bridge_maxage_cmd);
	install_element(STP_NODE,&stp_bridge_hetime_cmd);
	install_element(STP_NODE,&stp_bridge_fordelay_cmd);
	install_element(STP_NODE,&stp_bridge_forversion_cmd);
	install_element(STP_NODE,&stp_bridge_noconfig_cmd);
	install_element(STP_NODE,&stp_bridge_port_pathcost_cmd);
	install_element(STP_NODE,&stp_bridge_port_prio_cmd);
	install_element(STP_NODE,&stp_bridge_port_none_stp_cmd);
	install_element(STP_NODE,&stp_bridge_port_p2p_cmd);
	install_element(STP_NODE,&stp_bridge_port_edge_cmd);
	install_element(STP_NODE,&stp_bridge_port_mcheck_cmd);
	install_element(STP_NODE,&stp_bridge_port_noconfig_cmd);
	install_element(STP_NODE,&stp_show_spanning_tree_cmd);
	/*install_element(VIEW_NODE,&stp_show_spanning_tree_cmd);
	//install_element(ENABLE_NODE,&stp_show_spanning_tree_cmd);
	//install_element(CONFIG_NODE,&stp_show_spanning_tree_cmd);*/
	install_element(STP_NODE,&stp_show_spanning_tree_port_cmd);
	/*install_element(VIEW_NODE,&stp_show_spanning_tree_port_cmd);
	//install_element(ENABLE_NODE,&stp_show_spanning_tree_port_cmd);
	//install_element(CONFIG_NODE,&stp_show_spanning_tree_port_cmd);*/
	install_element(STP_NODE,&debug_spanning_tree_cmd);
	/*install_element(MST_NODE,&debug_spanning_tree_cmd);*/
	install_element(STP_NODE,&no_debug_spanning_tree_cmd);
	/*install_element(MST_NODE,&no_debug_spanning_tree_cmd);*/
    install_element(MST_NODE,&debug_spanning_tree_cmd);
	/*install_element(MST_NODE,&debug_spanning_tree_cmd);*/
	install_element(MST_NODE,&no_debug_spanning_tree_cmd);
	/*install_element(MST_NODE,&no_debug_spanning_tree_cmd);*/
}
#ifdef __cplusplus
}
#endif
