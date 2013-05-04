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
* dcli_hbip.c
*
* MODIFY:
*		by <qinhs@autelan.com> on 03/07/2008 revision <0.1>
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		CLI definition for HBIP module.
*
* DATE:
*		02/21/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.3 $	
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#include <zebra.h>
#include <dbus/dbus.h>
#include <stdlib.h>
#include <sysdef/npd_sysdef.h>
#include <dbus/npd/npd_dbus_def.h>
#include <util/npd_list.h>

#include "dcli_hbip.h"
#include "command.h"

extern DBusConnection *dcli_dbus_connection;

unsigned char *dcli_hbip_err_msg[] = {	\
/*   0 */	"%% Error none",
/*   1 */	"%% General failure",
/*   2 */	"%% Profile out of range",
/*   3 */	"%% Profile has already exist",
/*   4 */	"%% Profile not exist",
/*   5 */	"%% Memory malloc failed",
/*   6 */	"%% Bad parameter input",
/*   7*/ "%% Heartbeatlink or uplink,downlink,vgateway not configured",
/*	8*/ "%% Service should be disabled first"
};

struct cmd_node hbip_node = 
{
	HBIP_NODE,
	" "
};


DEFUN(config_hbip_cmd_func,
	  config_hbip_cmd,
	  "config hbip <1-16> primary IFNAME secondary IFNAME [A.B.C.D]",
	  CONFIG_STR
	  "Hotbackup interface pair\n"
	  "Hbip profile id\n"
	  "Primary interface\n"
	  "Primary interface name\n"
	  "Secondary interface\n"
	  "Secondary interface name\n"
	  "IP address of secondary interface, format as [A.B.C.D]\n"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int profile = 0;
	char* primary_if = NULL,*secondary_if = NULL;
	unsigned int ipaddr = 0;
	primary_if = (char *)malloc(MAX_IFNAME_LEN);
    if(NULL == primary_if){
       return CMD_WARNING;
	}	
	memset(primary_if,0,MAX_IFNAME_LEN);
	memcpy(primary_if,argv[1],strlen(argv[1]));

	secondary_if = (char *)malloc(MAX_IFNAME_LEN);
    if(NULL == secondary_if){
       return CMD_WARNING;
	}	
	memset(secondary_if,0,MAX_IFNAME_LEN);
	memcpy(secondary_if,argv[2],strlen(argv[2]));	

	profile = strtoul((char *)argv[0],NULL,10);
	if((profile < 1)|| profile>16){
        vty_out(vty,"%% Bad parameter : %s !",argv[0]);
		return CMD_WARNING;
	}
	if(argc == 4){
       ipaddr = inet_addr((char*)argv[3]) ;
	}

	query = dbus_message_new_method_call(HBIP_DBUS_BUSNAME,
										 HBIP_DBUS_OBJPATH,
										 HBIP_DBUS_INTERFACE,
										 HBIP_DBUS_METHOD_START_HBIP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,
		                     DBUS_TYPE_STRING,&primary_if,
							 DBUS_TYPE_STRING,&secondary_if,
							 DBUS_TYPE_UINT32,&ipaddr,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_HBIP_RETURN_CODE_OK != op_ret){
            vty_out(vty,dcli_hbip_err_msg[op_ret - DCLI_HBIP_RETURN_CODE_OK]);
		}
		/*dcli_hbip_notify_to_npd(vty,uplink_ifname,downlink_ifname,add);*/
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	free(primary_if);
	free(secondary_if);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(no_hbip_cmd_func,
	  no_hbip_cmd,
	  "no hbip <1-16>",
	  "Cancel configuraton\n"
	  "Hotbackup interface pair\n"
	  "Hbip profile id\n"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int profile = 0;
	profile = strtoul((char *)argv[0],NULL,10);
	if((profile < 1)|| profile>16){
        vty_out(vty,"%% Bad parameter : %s !",argv[0]);
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(HBIP_DBUS_BUSNAME,
										 HBIP_DBUS_OBJPATH,
										 HBIP_DBUS_INTERFACE,
										 HBIP_DBUS_METHOD_STOP_HBIP);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,					 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	else if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
        if( DCLI_HBIP_RETURN_CODE_OK != op_ret){
            vty_out(vty,dcli_hbip_err_msg[op_ret - DCLI_HBIP_RETURN_CODE_OK]);
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(show_hbip_cmd_func,
	  show_hbip_cmd,
	  "show hbip <1-16>",
	  "Show configuraton\n"
	  "Hotbackup interface pair\n"
	  "Hbip profile id\n"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	DBusMessageIter	 iter;	
	unsigned int op_ret = 0;
	unsigned int profile = 0;
	unsigned char *primary = NULL,*secondary = NULL;
	unsigned char  primary_link = 0,secondary_link = 0;
	profile = strtoul((char *)argv[0],NULL,10);
	if((profile < 1)|| profile>16){
        vty_out(vty,"%% Bad parameter : %s !",argv[0]);
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(HBIP_DBUS_BUSNAME,
										 HBIP_DBUS_OBJPATH,
										 HBIP_DBUS_INTERFACE,
										 HBIP_DBUS_METHOD_SHOW);
	dbus_error_init(&err);

	dbus_message_append_args(query,
		                     DBUS_TYPE_UINT32,&profile,					 
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);		
		return DCLI_HBIP_RETURN_CODE_ERR;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&op_ret);
    if(DCLI_HBIP_RETURN_CODE_OK != op_ret){
        vty_out(vty,"no hbip instance!\n");		
	}
	else{
		vty_out(vty,"-------HBIP %d-------\n",profile);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&primary);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&primary_link);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&secondary);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&secondary_link);
        vty_out(vty,"PRIMARY:   %s(%s)\n",primary,primary_link ? "UP" : "DOWN");
		vty_out(vty,"SECONDARY: %s(%s)\n",secondary,secondary_link ? "UP" : "DOWN");
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(debug_hbip_cmd_func,
		debug_hbip_cmd,
		"debug hbip (all|error|warning|debug|event|protocol|packet_send|packet_receive|packet_all)",
		DEBUG_STR
		"Debus hbip parameter \n"
		"Debug hbip all \n"
		"Debug hbip error \n"
		"Debug hbip warning \n"
		"Debug hbip debug \n"
		"Debug hbip event \n"
		"Debug hbip protocol \n"
		"Debug hbip packet send \n"
		"Debug hbip packet received \n"
		"Debug hbip packet send and receive \n"       
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
		flag = HBIP_DEBUG_FLAG_ALL;
	}
	else if(0 == strcmp(argv[0],"error")) {
		flag = HBIP_DEBUG_FLAG_ERR;
	}
	else if(0 == strcmp(argv[0],"warning")) {
		flag = HBIP_DEBUG_FLAG_WAR;
	}
	else if(0 == strcmp(argv[0],"debug")) {
		flag = HBIP_DEBUG_FLAG_DBG;
	}
	else if(0 == strcmp(argv[0],"event")) {
		flag = HBIP_DEBUG_FLAG_EVT;
	}
	else if(0 == strcmp(argv[0],"packet_send")) {
		flag = HBIP_DEBUG_FLAG_PKT_SED;
	}
	else if(0 == strcmp(argv[0],"packet_receive")) {
		flag = HBIP_DEBUG_FLAG_PKT_REV;
	}
	else if(0 == strcmp(argv[0],"packet_all")) {
		flag = HBIP_DEBUG_FLAG_PKT_ALL;
	}
	else if(0 == strcmp(argv[0],"protocol")) {
		flag = HBIP_DEBUG_FLAG_PROTOCOL;
	}
	else {
		vty_out(vty,"command parameter error!\n");
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(HBIP_DBUS_BUSNAME,HBIP_DBUS_OBJPATH,HBIP_DBUS_INTERFACE,HBIP_DBUS_METHOD_BRG_DBUG_HBIP);
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
         ;
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

DEFUN(no_debug_hbip_cmd_func,
		no_debug_hbip_cmd,
		"no debug hbip (all|error|warning|debug|event|protocol|packet_send|packet_receive|packet_all)",
        "cancel the debug\n"
        DEBUG_STR
		"Debus hbip parameter \n"
		"Debug hbip all \n"
		"Debug hbip error \n"
		"Debug hbip warning \n"
		"Debug hbip debug \n"
		"Debug hbip event \n"
		"Debug hbip protocol \n"
		"Debug hbip packet send \n"
		"Debug hbip packet received \n"
		"Debug hbip packet send and receive \n"       
)
{	
    DBusMessage *query, *reply;
	DBusError err;
	
	unsigned int ret;
	unsigned int flag;
	
	if(argc > 1) {
		vty_out(vty,"command parameters error!\n");
		return CMD_WARNING;
	}
	
	
	if(0 == strcmp(argv[0],"all")) {
		flag = HBIP_DEBUG_FLAG_ALL;
	}
	else if(0 == strcmp(argv[0],"error")) {
		flag = HBIP_DEBUG_FLAG_ERR;
	}
	else if(0 == strcmp(argv[0],"warning")) {
		flag = HBIP_DEBUG_FLAG_WAR;
	}
	else if(0 == strcmp(argv[0],"debug")) {
		flag = HBIP_DEBUG_FLAG_DBG;
	}
	else if(0 == strcmp(argv[0],"event")) {
		flag = HBIP_DEBUG_FLAG_EVT;
	}
	else if(0 == strcmp(argv[0],"packet_send")) {
		flag = HBIP_DEBUG_FLAG_PKT_SED;
	}
	else if(0 == strcmp(argv[0],"packet_receive")) {
		flag = HBIP_DEBUG_FLAG_PKT_REV;
	}
	else if(0 == strcmp(argv[0],"packet_all")) {
		flag = HBIP_DEBUG_FLAG_PKT_ALL;
	}
	else if(0 == strcmp(argv[0],"protocol")) {
		flag = HBIP_DEBUG_FLAG_PROTOCOL;
	}
	else {
		vty_out(vty,"command parameter error!\n");
		return CMD_WARNING;
	}
	
	query = dbus_message_new_method_call(HBIP_DBUS_BUSNAME,HBIP_DBUS_OBJPATH,HBIP_DBUS_INTERFACE,HBIP_DBUS_METHOD_BRG_NO_DBUG_HBIP);
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
         ;
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

int dcli_hbip_show_running_cfg(struct vty * vty)
{	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	
	query = dbus_message_new_method_call(HBIP_DBUS_BUSNAME,
										 HBIP_DBUS_OBJPATH,
										 HBIP_DBUS_INTERFACE,
										 HBIP_DBUS_METHOD_SHOW_RUNNING);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("failed get reply.\n");
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
	return 0;	
}
void dcli_hbip_element_init(void)  
{
	install_node(&hbip_node, dcli_hbip_show_running_cfg,"HBIP_NODE");
    install_element(CONFIG_NODE,&config_hbip_cmd);
	install_element(CONFIG_NODE,&no_hbip_cmd);
	install_element(CONFIG_NODE,&show_hbip_cmd);
	install_element(CONFIG_NODE,&debug_hbip_cmd);
	install_element(CONFIG_NODE,&no_debug_hbip_cmd);
}

#ifdef __cplusplus
}
#endif

