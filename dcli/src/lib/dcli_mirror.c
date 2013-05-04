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
* dcli_mirror.c
*
*
* CREATOR:
*		qinhs@autelan.com
*
* DESCRIPTION:
*		CLI definition for MIRROR module.
*
* DATE:
*		02/21/2008	
*
*  FILE REVISION NUMBER:
*  		$Revision: 1.37 $	
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
#include "util/npd_list.h"
#include "npd/nam/npd_amapi.h"
#include "dcli_main.h"
#include "command.h"
#include "dcli_mirror.h"
#include "if.h"
#include "dcli_fdb.h"
#include "dcli_vlan.h"
#include "sysdef/returncode.h"

struct cmd_node mirror_node =
{
	MIRROR_NODE,
	"%s(config-mirror)# "
};

/* error message corresponding to mirror error code*/
unsigned char *dcli_mirror_err_msg[] = {	\
/*   0 */	"%% Error none",
/*   1 */	"%% General failure",
/*   2 */	"%% Bad parameter specified",
/*   3 */	"%% Memory allocation failure",
/*   4 */	"%% Policy based mirror must has permit action",
/*   5 */	"%% Mirror port source has been configured",
/*   6 */	"%% Mirror port source have not been configured",
/*   7 */	"%% Profile id is out of range",
/*   8 */	"%% Mirror vlan source has been configured",
/*	9 */	"%% Mirror vlan source have not been configured",
/* 10 */	"%% Vlan is not exists",
/* 11 */	"%% destination node not exist,the destination node should be added first.",
/* 12 */	"%% Mirror destination port has already exist",
/* 13 */	"%% creat the destination port fail.",
/* 14 */	"%% The fdb-source does not exist",
/* 15 */	"%% The fdb-source exists",
/* 16 */	"%% Mirror does not support this source",
/* 17 */	"%% Acl global not exist",
/* 18 */	"%% The acl-source exist",
/* 19 */	"%% The acl-source does not exist",
/* 20 */	"%% The destination port has source members,so can not delete",
/* 21 */	"%% Mac address given cannot be system mac address",
/* 22 */	"%% The port-source conflict",
/* 23 */	"%% destination node member not exist.",
/* 24 */	"%% Profile has already been created",
/* 25 */	"%% Profile has not been created",
/* 26 */	"%% Mirror failed, please use the extended rule",
/* 27 */ "%% Not support the profile",
/* 28 */ "%% Not supported command"
};


extern DBusConnection *dcli_dbus_connection;


DEFUN(config_mirror_cmd_func,
	  config_mirror_cmd,
	  "config mirror-profile <0-6>",
	  CONFIG_STR
	  MIRROR_STR
	  "Profile ID for configuration"
)

{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int op_ret = 0;
	unsigned int profile = -1;

    if(argc == 1){
		profile = strtoul((char *)argv[0],NULL,0);
		if((profile < 0)||profile > 6){
	        vty_out(vty,"%% Bad parameter : %s !",argv[0]);
			return CMD_WARNING;
		}
    }
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_CONFIG_MIRROR);
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
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&profile,
		DBUS_TYPE_INVALID)){	

		if(MIRROR_RETURN_CODE_SUCCESS==op_ret){
			if(CONFIG_NODE==(vty->node)){
					vty->node=MIRROR_NODE;	
					vty->index = (void *)profile;
			}
		}
		else if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
			vty_out(vty,"%% Product not support this function!\n");
		}
		else {
            vty_out(vty,dcli_mirror_err_msg[op_ret - MIRROR_RETURN_CODE_BASE]);
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

DEFUN(debug_mirror_dest_port_cmd_func,
	  debug_mirror_dest_port_cmd,
	  "destination-port PORTNO profile [<1-6>]",
	  "Destination port for mirror\n"
	  CONFIG_ETHPORT_STR
	  "Mirror ingress traffic\n"
	  "Mirror egress traffic\n"
	  "Mirror bidirection traffic\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned int	temp = 0,op_ret,g_eth_index;
	unsigned char   slot_no,port_no;	
	unsigned int profile = 0;

	if(argc > 2) {
		vty_out(vty,"%% Input too many param\n");
		return CMD_FAILURE;
	}
	
	temp = parse_slotport_no((char*)argv[0],&slot_no,&port_no);
	if (MIRROR_RETURN_CODE_SUCCESS != temp) {
    	vty_out(vty,"%% Illegal format with slot/port!\n");
		return CMD_FAILURE;
	}

	profile = strtoul((char *)argv[1],NULL,0);
	if((profile < 0)||profile > 6){
        vty_out(vty,"%% Bad parameter : %s !",argv[1]);
		return CMD_WARNING;
	}


	/*vty_out(vty,"profile %d,slot/port %d/%d , direct %s, profile %d\n",profile,slot_no,port_no,direct ? "egress" : "ingress",profile);*/
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_DEBUG_MIRROR_DEST_PORT_CREATE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
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
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
		if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
			vty_out(vty,"%% Product not support this function!\n");
		}
        else if(MIRROR_RETURN_CODE_SUCCESS!=op_ret){
			#if 0
		   vty_out(vty,dcli_mirror_err_msg[op_ret - MIRROR_RETURN_CODE_BASE]);
			#endif
			vty_out(vty,"mirror destination port set failure !!! \n");
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


DEFUN(mirror_dest_port_cmd_func,
	  mirror_dest_port_cmd,
	  "destination-port PORTNO (ingress|egress|bidirection)",
	  "Destination port for mirror\n"
	  CONFIG_ETHPORT_STR
	  "Mirror ingress traffic\n"
	  "Mirror egress traffic\n"
	  "Mirror bidirection traffic\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned int	temp = 0,op_ret,g_eth_index;
	unsigned char   slot_no,port_no;	
	MIRROR_DIRECTION_TYPE direct = MIRROR_INGRESS_E;
	unsigned int profile = 0;

	if(argc > 2) {
		vty_out(vty,"%% Input too many param\n");
		return CMD_FAILURE;
	}
	
	temp = parse_slotport_no((char*)argv[0],&slot_no,&port_no);
	if (MIRROR_RETURN_CODE_SUCCESS != temp) {
    	vty_out(vty,"%% Illegal format with slot/port!\n");
		return CMD_FAILURE;
	}

	if(0 == strncmp("ingress",argv[1],strlen(argv[1]))) {
		direct = MIRROR_INGRESS_E;
	}
	else if(0 == strncmp("egress",argv[1],strlen(argv[1]))) {
		direct = MIRROR_EGRESS_E;
	}
	else if(0 == strncmp("bidirection",argv[1],strlen(argv[1]))) {
		direct = MIRROR_BIDIRECTION_E;
	}
	else {
		vty_out(vty,"%% Input bad param\n");
		return CMD_FAILURE;
	}

	if(MIRROR_NODE==vty->node)
	{
		profile = (unsigned int)(vty->index);
	}
	/*vty_out(vty,"profile %d,slot/port %d/%d , direct %s, profile %d\n",profile,slot_no,port_no,direct ? "egress" : "ingress",profile);*/
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_MIRROR_DEST_PORT_CREATE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
							 DBUS_TYPE_UINT32,&direct,
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
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&g_eth_index,
		DBUS_TYPE_INVALID)){	
		if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
			vty_out(vty,"%% Product not support this function!\n");
		}
        else if(MIRROR_RETURN_CODE_SUCCESS!=op_ret){
			#if 0
		   vty_out(vty,dcli_mirror_err_msg[op_ret - MIRROR_RETURN_CODE_BASE]);
			#endif
			vty_out(vty,"mirror destination port set failure !!! \n");
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

DEFUN(no_mirror_dest_port_cmd_func,
	  no_mirror_dest_port_cmd,
	  "no destination-port PORTNO (ingress|egress|bidirection)",
	  NO_CONFIG_STR
	  "Destination port for mirror\n"
	  CONFIG_ETHPORT_STR
	  "Mirror ingress traffic\n"
	  "Mirror egress traffic\n"
	  "Mirror ingress and egress traffic\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned int	temp = 0,op_ret = 0,profile = 0;
	unsigned char   slot_no = 0,port_no = 0;	
	MIRROR_DIRECTION_TYPE direct = MIRROR_INGRESS_E;

	temp = parse_slotport_no((char*)argv[0],&slot_no,&port_no);
	
	if (MIRROR_RETURN_CODE_SUCCESS != temp) {
    	vty_out(vty,"%% Illegal format with slot/port!\n");
		return CMD_FAILURE;
	}
	if(0 == strncmp("ingress",argv[1],strlen(argv[1]))) {
		direct = MIRROR_INGRESS_E;
	}
	else if(0 == strncmp("egress",argv[1],strlen(argv[1]))) {
		direct = MIRROR_EGRESS_E;
	}
	else if(0 == strncmp("bidirection",argv[1],strlen(argv[1]))){
		direct = MIRROR_BIDIRECTION_E;
	}

	
	/*printf("slot/port %d/%d \n",slot_no,port_no);*/
	if(MIRROR_NODE==vty->node)
	{
		profile = (unsigned int)(vty->index);
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_MIRROR_DEST_PORT_DEL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
							 DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&direct,
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
		DBUS_TYPE_INVALID)){	
		if(MIRROR_RETURN_CODE_SUCCESS!=op_ret){
			#if 0
			vty_out(vty,dcli_mirror_err_msg[op_ret - MIRROR_RETURN_CODE_BASE]);
			#endif
			vty_out(vty,"delete destination port error !\n");
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

DEFUN(mirror_acl_cmd_func,
	  mirror_acl_cmd,
	  "mirror policy-source INDEX",
	  MIRROR_STR
	  "Mirror based on policy\n"
	  "Acl rule Index range in 1-1024\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned int ruleIndex = 0,profile = 0;
	unsigned int op_ret = 0;
	int ret = 0;
	
	ret=dcli_str2ulong((char*)argv[0],&ruleIndex);	
	if(ret==MIRROR_RETURN_CODE_ERROR)
	{
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_FAILURE;
	}
	ruleIndex = ruleIndex-1;
	
	if(MIRROR_NODE==vty->node)
	{
		profile = (unsigned int)(vty->index);
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_APPEND_MIRROR_BASE_ACL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&ruleIndex,
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
		DBUS_TYPE_INVALID)){	
		if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
			vty_out(vty,"%% Product not support this function!\n");
		}
		else if(MIRROR_RETURN_CODE_SUCCESS!=op_ret){
			vty_out(vty,dcli_mirror_err_msg[op_ret - MIRROR_RETURN_CODE_BASE]);
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

DEFUN(no_mirror_acl_cmd_func,
	  no_mirror_acl_cmd,
	  "no mirror policy-source INDEX",
	  NO_CONFIG_STR
      MIRROR_STR
	  "Mirror based on policy\n"
	  "Acl rule Index range in 1-1024\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned int ruleIndex = 0,profile = 0;
	unsigned int op_ret = 0;
	int ret = 0;
	
	ret=dcli_str2ulong((char*)argv[0],&ruleIndex);	
	if(ret==MIRROR_RETURN_CODE_ERROR)
	{
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_FAILURE;
	}
	ruleIndex = ruleIndex-1;
	
	if(MIRROR_NODE==vty->node)
	{
		profile = (unsigned int)(vty->index);
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_CANCEL_MIRROR_BASE_ACL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&ruleIndex,
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
		DBUS_TYPE_INVALID)){			
		if(MIRROR_RETURN_CODE_SUCCESS!=op_ret){
				vty_out(vty,dcli_mirror_err_msg[op_ret - MIRROR_RETURN_CODE_BASE]);						 
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

DEFUN(mirror_port_cmd_func,
	  mirror_port_cmd,
	  "mirror port-source PORTNO (ingress|egress|bidirection)",
	  MIRROR_STR
	  "Mirror based on port\n"
	  CONFIG_ETHPORT_STR
	  "Mirror ingress traffic\n"
	  "Mirror egress traffic\n"
	  "Mirror ingress and egress traffic\n"

)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int profile = 0;
	unsigned int direct = 0;
	unsigned char slot = 0,port = 0;
	int ret = 0;

	
	ret = parse_slotport_no((char*)argv[0],&slot,&port);
	if (MIRROR_RETURN_CODE_SUCCESS != ret) {
    	vty_out(vty,"%% Illegal format with slot/port!\n");
		return CMD_FAILURE;
	}
    else if(0 == slot){
        vty_out(vty,"%% Illegal slot no!\n");
		return CMD_FAILURE;
	}
	if(0 == strncmp("ingress",argv[1],strlen(argv[1]))) {
		direct = MIRROR_INGRESS_E;
	}
	else if(0 == strncmp("egress",argv[1],strlen(argv[1]))) {
		direct = MIRROR_EGRESS_E;
	}
	else if(0 == strncmp("bidirection",argv[1],strlen(argv[1]))){
		direct = MIRROR_BIDIRECTION_E;
	}
	
	if(MIRROR_NODE==vty->node) {
		profile = (unsigned int)(vty->index);
	}

	/*printf("slot/port %d/%d \n",slot,port);*/
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_APPEND_MIRROR_BASE_PORT_CREATE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&direct,
							 DBUS_TYPE_BYTE,&slot,
							 DBUS_TYPE_BYTE,&port,
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
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID))
	{	
		if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
			vty_out(vty,"%% Product not support mirror!\n");
		}
		else if(MIRROR_RETURN_CODE_SUCCESS!=ret){
				vty_out(vty,dcli_mirror_err_msg[ret - MIRROR_RETURN_CODE_BASE]);						 
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


DEFUN(debug_mirror_port_cmd_func,
	  debug_mirror_port_cmd,
	  "mirror port-source PORTNO (ingress|egress|bidirection) profile [<1-6>]",
	  MIRROR_STR
	  "Mirror based on port\n"
	  CONFIG_ETHPORT_STR
	  "Mirror ingress traffic\n"
	  "Mirror egress traffic\n"
	  "Mirror ingress and egress traffic\n"

)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int profile = 0;
	unsigned int direct = 0;
	unsigned char slot = 0,port = 0;
	int ret = 0;

	
	ret = parse_slotport_no((char*)argv[0],&slot,&port);
	if (MIRROR_RETURN_CODE_SUCCESS != ret) {
    	vty_out(vty,"%% Illegal format with slot/port!\n");
		return CMD_FAILURE;
	}
	if(0 == strncmp("ingress",argv[1],strlen(argv[1]))) {
		direct = MIRROR_INGRESS_E;
	}
	else if(0 == strncmp("egress",argv[1],strlen(argv[1]))) {
		direct = MIRROR_EGRESS_E;
	}
	else if(0 == strncmp("bidirection",argv[1],strlen(argv[1]))){
		direct = MIRROR_BIDIRECTION_E;
	}
	
	
	profile = strtoul((char *)argv[2],NULL,0);
	if((profile < 0)||profile > 6){
		vty_out(vty,"%% Bad parameter : %s !",argv[2]);
		return CMD_WARNING;
	}

	/*printf("slot/port %d/%d \n",slot,port);*/
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_DEBUG_APPEND_MIRROR_BASE_PORT_CREATE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&direct,
							 DBUS_TYPE_BYTE,&slot,
							 DBUS_TYPE_BYTE,&port,
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
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID))
	{	
		if (MIRROR_RETURN_CODE_SUCCESS!=ret)
		{
			vty_out(vty,"%% config mirror fail !!\n");
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


DEFUN(debug_no_mirror_port_cmd_func,
	  debug_no_mirror_port_cmd,
	  "no mirror port-source PORTNO (ingress|egress|bidirection) profile [<1-6>]",
	  NO_CONFIG_STR
	  MIRROR_STR
	  "Mirror based on port\n"
	  CONFIG_ETHPORT_STR
	  "Mirror ingress traffic\n"
	  "Mirror egress traffic\n"
	  "Mirror ingress and egress traffic\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int profile = 0;
	unsigned char slot = 0,port = 0;
	unsigned int direct = 0;
	int ret = 0;

	
	ret = parse_slotport_no((char*)argv[0],&slot,&port);
	if (MIRROR_RETURN_CODE_SUCCESS != ret) {
    	vty_out(vty,"%% Illegal format with slot/port!\n");
		return CMD_FAILURE;
	}
	if(0 == strncmp("ingress",argv[1],strlen(argv[1]))) {
			direct = MIRROR_INGRESS_E;
	}
	else if(0 == strncmp("egress",argv[1],strlen(argv[1]))) {
		direct = MIRROR_EGRESS_E;
	}
	else if(0 == strncmp("bidirection",argv[1],strlen(argv[1]))){
		direct = MIRROR_BIDIRECTION_E;
	}
	
	
	profile = strtoul((char *)argv[2],NULL,0);
	if((profile < 0)||profile > 6){
		vty_out(vty,"%% Bad parameter : %s !",argv[2]);
		return CMD_WARNING;
	}
	

	/*printf("slot/port %d/%d \n",slot,port);*/
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_DEBUG_APPEND_MIRROR_BASE_PORT_DEL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&direct,
							 DBUS_TYPE_BYTE,&slot,
							 DBUS_TYPE_BYTE,&port,
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
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID))
	{	
		if(MIRROR_RETURN_CODE_SUCCESS!=ret){
			vty_out(vty,dcli_mirror_err_msg[ret - MIRROR_RETURN_CODE_BASE]);						 
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


DEFUN(no_mirror_port_cmd_func,
	  no_mirror_port_cmd,
	  "no mirror port-source PORTNO (ingress|egress|bidirection)",
	  NO_CONFIG_STR
	  MIRROR_STR
	  "Mirror based on port\n"
	  CONFIG_ETHPORT_STR
	  "Mirror ingress traffic\n"
	  "Mirror egress traffic\n"
	  "Mirror ingress and egress traffic\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int profile = 0;
	unsigned char slot = 0,port = 0;
	unsigned int direct = 0;
	int ret = 0;

	
	ret = parse_slotport_no((char*)argv[0],&slot,&port);
	if (MIRROR_RETURN_CODE_SUCCESS != ret) {
    	vty_out(vty,"%% Illegal format with slot/port!\n");
		return CMD_FAILURE;
	}
	if(0 == strncmp("ingress",argv[1],strlen(argv[1]))) {
			direct = MIRROR_INGRESS_E;
	}
	else if(0 == strncmp("egress",argv[1],strlen(argv[1]))) {
		direct = MIRROR_EGRESS_E;
	}
	else if(0 == strncmp("bidirection",argv[1],strlen(argv[1]))){
		direct = MIRROR_BIDIRECTION_E;
	}
	
	if(MIRROR_NODE==vty->node) {
		profile = (unsigned int)(vty->index);
	}

	/*printf("slot/port %d/%d \n",slot,port);*/
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_APPEND_MIRROR_BASE_PORT_DEL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&direct,
							 DBUS_TYPE_BYTE,&slot,
							 DBUS_TYPE_BYTE,&port,
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
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID))
	{	
		if(MIRROR_RETURN_CODE_SUCCESS!=ret){
			vty_out(vty,dcli_mirror_err_msg[ret - MIRROR_RETURN_CODE_BASE]);						 
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

DEFUN(mirror_vlan_cmd_func,
	  mirror_vlan_cmd,
	  "mirror vlan-source <1-4094>",
	  MIRROR_STR
	  "Mirror based on vlan\n"
	  "Vlan id\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned short vid = 0;
	unsigned int op_ret = 0,profile = 0;
	int ret = 0;
	
	ret=parse_vlan_no((char*)argv[0],&vid);	
	if(MIRROR_RETURN_CODE_SUCCESS != ret)
	{
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_FAILURE;
	}

	/*printf("vid %d\n",vid);*/
	
	if(MIRROR_NODE==vty->node)
	{
		profile = (unsigned int)(vty->index);
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_APPEND_MIRROR_BASE_VLAN_CREATE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT16,&vid,
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
		DBUS_TYPE_INVALID))
	{	
		if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
			vty_out(vty,"%% Product not support mirror!\n");
		}
		else if(MIRROR_RETURN_CODE_SUCCESS!=op_ret){
			vty_out(vty,dcli_mirror_err_msg[op_ret - MIRROR_RETURN_CODE_BASE]);						 
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

DEFUN(no_mirror_vlan_cmd_func,
	  no_mirror_vlan_cmd,
	  "no mirror vlan-source <1-4094>",
	  NO_CONFIG_STR
	  MIRROR_STR
	  "Mirror based on vlan\n"
	  "Vlan id\n"
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned short vid = 0;
	unsigned int op_ret = 0,profile = 0;
	int ret = 0;
	
	ret=parse_vlan_no((char*)argv[0],&vid);	
	if(MIRROR_RETURN_CODE_SUCCESS != ret)
	{
		vty_out(vty,"%% Illegal rule index!\n");
		return CMD_FAILURE;
	}


	if(MIRROR_NODE==vty->node)
	{
		profile = (unsigned int)(vty->index);
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_APPEND_MIRROR_BASE_VLAN_DEL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT16,&vid,
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
		DBUS_TYPE_INVALID))
	{	
		if(MIRROR_RETURN_CODE_SUCCESS!=ret){
			vty_out(vty,dcli_mirror_err_msg[ret - MIRROR_RETURN_CODE_BASE]);						 
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

DEFUN(mirror_fdb_cmd_func,
	  mirror_fdb_cmd,
	  "mirror fdb-source MAC <1-4094> PORTNO",
	  MIRROR_STR
	  "Mirror based on FDB\n"
	  "Config MAC address in Hex format\n"
	  "Config vlan number in 1-1024\n"
	  CONFIG_ETHPORT_STR
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned int profile = 0;
	unsigned int op_ret = 0;
	int ret = 0;
	ETHERADDR macAddr = {0};
	unsigned short vlanId = 0;
	unsigned char   slot_no = 0,port_no = 0;	
	
	memset(&macAddr,0,sizeof(ETHERADDR));

    op_ret = parse_mac_addr((char *)argv[0],&macAddr);
	if (MIRROR_RETURN_CODE_SUCCESS != op_ret) {
		vty_out(vty,"Unknow mac addr format!\n");
		return NPD_FDB_ERR_NONE;
	}
   
	op_ret=is_muti_brc_mac(&macAddr);
	if(op_ret==1){
		vty_out(vty,"erro:input broadcast or multicast mac!\n");
		return NPD_FDB_ERR_NONE;
	}
	
	op_ret = parse_short_parse((char*)argv[1], &vlanId);
	if (MIRROR_RETURN_CODE_SUCCESS != op_ret) {
		vty_out(vty,"Unknow vlan id format.\n");
		return NPD_FDB_ERR_NONE;
	}
	if ((vlanId<MIN_VLANID)||(vlanId>MAX_VLANID)){
		vty_out(vty,"FDB vlan outrange!\n");
		return NPD_FDB_ERR_NONE;
	}

    /*check port*/
	op_ret = parse_slotport_no((char*)argv[2],&slot_no,&port_no);
	if (MIRROR_RETURN_CODE_SUCCESS != op_ret) {
    	vty_out(vty,"%% Illegal format with slot/port!\n");
		return NPD_FDB_ERR_NONE;
	}
	/*printf("vid %d,mac :: %#02x:%#02x:%#02x:%#02x:%#02x:%#02x\n",vlanId,macAddr.arEther[0],
		//macAddr.arEther[1],macAddr.arEther[2],macAddr.arEther[3],macAddr.arEther[4],macAddr.arEther[5]);*/
	
	if(MIRROR_NODE==vty->node)
	{
		profile = (unsigned int)vty->index;
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_APPEND_MIRROR_BASE_FDB);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vlanId,
							 DBUS_TYPE_BYTE, &slot_no,
							 DBUS_TYPE_BYTE, &port_no,
							 DBUS_TYPE_BYTE,&(macAddr.arEther[0]),
							 DBUS_TYPE_BYTE,&(macAddr.arEther[1]),
							 DBUS_TYPE_BYTE,&(macAddr.arEther[2]),
							 DBUS_TYPE_BYTE,&(macAddr.arEther[3]),
							 DBUS_TYPE_BYTE,&(macAddr.arEther[4]),
							 DBUS_TYPE_BYTE,&(macAddr.arEther[5]),
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
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
		if (COMMON_PRODUCT_NOT_SUPPORT_FUCTION == op_ret){
			vty_out(vty,"%% Product not support mirror!\n");
		}
        else if(MIRROR_RETURN_CODE_SUCCESS!=op_ret){
			vty_out(vty,dcli_mirror_err_msg[op_ret - MIRROR_RETURN_CODE_BASE]);						 
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


DEFUN(no_mirror_fdb_cmd_func,
	  no_mirror_fdb_cmd,
	  "no mirror fdb-source MAC <1-4094> PORTNO",
	  NO_CONFIG_STR
	  MIRROR_STR
	  "Mirror based on fdb\n"
	  "Config MAC address in Hex format\n"
	  "Config vlan number in 1-1024\n"
	  CONFIG_ETHPORT_STR
)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned int profile = 0;
	unsigned int op_ret = 0;
	int ret = 0;
	ETHERADDR macAddr = {0};
	unsigned short vlanId = 0;
	unsigned char slot_no = 0,port_no = 0;
	
	memset(&macAddr,0,sizeof(ETHERADDR));

    	op_ret = parse_mac_addr((char *)argv[0],&macAddr);
	if (MIRROR_RETURN_CODE_SUCCESS != op_ret) {
		vty_out(vty,"Unknow mac addr format!\n");
		return NPD_FDB_ERR_NONE;
	}
   
	op_ret=is_muti_brc_mac(&macAddr);
	if(op_ret==1){
		vty_out(vty,"erro:input broadcast or multicast mac!\n");
		return NPD_FDB_ERR_NONE;
	}
	
	op_ret = parse_short_parse((char*)argv[1], &vlanId);
	if (MIRROR_RETURN_CODE_SUCCESS != op_ret) {
		vty_out(vty,"Unknow vlan id format.\n");
		return NPD_FDB_ERR_NONE;
	}
	if ((vlanId<MIN_VLANID)||(vlanId>MAX_VLANID)){
		vty_out(vty,"FDB vlan outrange!\n");
		return NPD_FDB_ERR_NONE;
	}

	op_ret = parse_slotport_no((char*)argv[2],&slot_no,&port_no);
	if (MIRROR_RETURN_CODE_SUCCESS != op_ret) {
    	vty_out(vty,"%% Illegal format with slot/port!\n");
		return NPD_FDB_ERR_NONE;
	}
	
				
	if(MIRROR_NODE==vty->node)
	{
		profile = (unsigned int)vty->index;
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_CANCEL_MIRROR_BASE_FDB);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vlanId,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
							 DBUS_TYPE_BYTE,&(macAddr.arEther[0]),
							 DBUS_TYPE_BYTE,&(macAddr.arEther[1]),
							 DBUS_TYPE_BYTE,&(macAddr.arEther[2]),
							 DBUS_TYPE_BYTE,&(macAddr.arEther[3]),
							 DBUS_TYPE_BYTE,&(macAddr.arEther[4]),
							 DBUS_TYPE_BYTE,&(macAddr.arEther[5]),
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
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
      if(MIRROR_RETURN_CODE_SUCCESS!=ret){
			vty_out(vty,dcli_mirror_err_msg[ret - MIRROR_RETURN_CODE_BASE]);						 
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



DEFUN(show_mirror_cmd_func,
	  show_mirror_cmd,
	  "show mirror-profile [<0-6>]",
	  "show system information\n"
	  MIRROR_STR
	  "Mirror profile ID\n"
)
{
    unsigned int profile = 0,count = 0;
    unsigned int rc = MIRROR_RETURN_CODE_SUCCESS;
    if(0 == argc){
       for(profile = 0;profile < 7;profile++){
	   	  count++;
		  rc += dcli_show_mirror(vty,profile);
	   }
    }
	else if(1 == argc){
        profile = strtoul((char *)argv[0],NULL,0);
		if((profile < 0)||profile > 6){
	        vty_out(vty,"%% Bad parameter : %s !",argv[0]);
			return CMD_WARNING;
		}
		rc = dcli_show_mirror(vty,profile);
		count = 1;
	}
    if(MIRROR_RETURN_CODE_DESTINATION_NODE_NOTEXIST*count == rc){
		vty_out(vty,"%% Warning:no destination port,configuration may not take effect!\n");
	}
}



DEFUN(delete_mirror_cmd_func,
	  delete_mirror_cmd,
	  "delete mirror",
	  "Delete system information\n"
	   MIRROR_STR
)
{
      DBusMessage *query = NULL, *reply = NULL;
	  DBusError err = {0};

	  DBusMessageIter	 iter;

	  unsigned int profile = 0;
	  unsigned int ret = 0;

	if(MIRROR_NODE==vty->node)
	{
		profile = (unsigned int)(vty->index);
	}

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_MIRROR_DELETE);
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
		return CMD_SUCCESS;
	}

	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID))
	{	
         if(MIRROR_RETURN_CODE_SUCCESS!=ret){
			vty_out(vty,"%% Fail to delete mirrored port!\n");						 
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

unsigned int  dcli_show_mirror
(
    struct vty* vty,
    unsigned int profile
)
{
    DBusMessage *query = NULL, *reply = NULL;
	DBusError err;

	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	
    unsigned int op_rc = MIRROR_RETURN_CODE_SUCCESS;
	unsigned int	temp = 0,op_ret =0,g_eth_index =0,ruleIndex = 0;
	unsigned char   slot_no = 0,port_no = 0,bi_slot_no = 0,bi_port_no = 0,in_slot_no = 0,in_port_no = 0, eg_slot_no = 0,eg_port_no = 0;	
	unsigned int ret = 0,i = 0,fdb_loop = 0;
	unsigned int port_count = 0,vlan_count = 0,policy_count = 0,fdb_count = 0;
	unsigned int bi_flag = 0,in_flag = 0,eg_flag = 0;
	unsigned short vlanid = 0;
	unsigned char mac[6] = {0};
	MIRROR_DIRECTION_TYPE direct = MIRROR_INGRESS_E,port_direct = MIRROR_INGRESS_E;


	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_MIRROR_SHOW);
    /*
	if(MIRROR_NODE==vty->node)
	{
		profile = (unsigned int)(vty->index);
	}
	*/
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		vty_out(vty,"get profile %d failed get reply.\n",profile);
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
	}
    
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(COMMON_PRODUCT_NOT_SUPPORT_FUCTION == ret){
		vty_out(vty,"%% Product not support this function!\n");
	}
	else if (MIRROR_RETURN_CODE_DESTINATION_NODE_NOTEXIST == ret){
	   /*vty_out(vty,"%% Warning:no destination port,configuration may not take effect!\n");*/
	   dbus_message_unref(reply);
	   op_rc = MIRROR_RETURN_CODE_DESTINATION_NODE_NOTEXIST;
	}
	else {
	    dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&bi_flag);
	    dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&in_flag);
	    dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&eg_flag);
	    if(bi_flag){
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&bi_slot_no);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&bi_port_no);
		}
		if(in_flag){
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&in_slot_no);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&in_port_no);
		}
		if(eg_flag){
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&eg_slot_no);
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&eg_port_no);
		}
	
		vty_out(vty,"Detailed mirror profile %d:\n",profile);
		vty_out(vty,"============================================================\n");
		if(bi_flag){
			vty_out(vty,"%-20s:%d/%d\n","Bidirection Destination",bi_slot_no,bi_port_no);
		}
		if(in_flag){
			vty_out(vty,"%-20s:%d/%d\n","Ingress Destination",in_slot_no,in_port_no);
		}
		if(eg_flag){
			vty_out(vty,"%-20s:%d/%d\n","Egress Destination",eg_slot_no,eg_port_no);
		}
		if((!bi_flag)&&(!in_flag)&&(!eg_flag)){
	        vty_out(vty,"warning:no destination port!");
			dbus_message_unref(reply);
		}
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&port_count);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&vlan_count);
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&policy_count);
	    dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&fdb_count);
		if(port_count > 0){
			vty_out(vty,"------------------------------------------------------------\n");
			dbus_message_iter_next(&iter);
			dbus_message_iter_recurse(&iter,&iter_array);
			vty_out(vty,"%-19s:","port-source");
			for(i = 0;i<port_count;i++){		
				DBusMessageIter iter_struct;
				dbus_message_iter_recurse(&iter_array,&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&slot_no);
				dbus_message_iter_next(&iter_struct);
	            dbus_message_iter_get_basic(&iter_struct,&port_no);
				dbus_message_iter_next(&iter_struct);
	            dbus_message_iter_get_basic(&iter_struct,&port_direct);
				if(i<(port_count-1)) {
				  if((0!=i) && (0 == i%6)) { /* show at most 8-port per line*/
						vty_out(vty,"\n%-20s","");
				   }
				   vty_out(vty,"%d/%d(%s),",slot_no,port_no,(MIRROR_INGRESS_E == port_direct)? "I":(MIRROR_EGRESS_E == port_direct)?"E" :"B" );
				}
				else{
	               vty_out(vty,"%d/%d(%s)\n",slot_no,port_no,(MIRROR_INGRESS_E == port_direct)? "I":(MIRROR_EGRESS_E == port_direct)?"E" :"B");
				}
				dbus_message_iter_next(&iter_array);
			}

		}
		if(vlan_count > 0){
			vty_out(vty,"------------------------------------------------------------\n");
			dbus_message_iter_next(&iter);
			dbus_message_iter_recurse(&iter,&iter_array);
			vty_out(vty,"%-19s:","vlan-source");
			for(i = 0;i<vlan_count;i++){		
				DBusMessageIter iter_struct;
				dbus_message_iter_recurse(&iter_array,&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&vlanid);
				if(i<(vlan_count-1)) {
	 			   if((0!=i) && (0==(i%8))) { /* show at most 8-vlan per line*/
	 				  vty_out(vty,"\n%-20s","");
	 			   }
				   vty_out(vty,"%d,",vlanid);
				}
				else{
	               vty_out(vty,"%d\n",vlanid);
				}
				dbus_message_iter_next(&iter_array);
			}
			
		}
		if(policy_count > 0){
			vty_out(vty,"------------------------------------------------------------\n");
			dbus_message_iter_next(&iter);
			dbus_message_iter_recurse(&iter,&iter_array);
			vty_out(vty,"%-19s:","policy-source");
			for(i = 0;i<policy_count;i++){		
				DBusMessageIter iter_struct;
				dbus_message_iter_recurse(&iter_array,&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&ruleIndex);
				if(i<(policy_count-1)) {
				   if((0!=i) && (0==(i%8))) { /* show at most 8-policy per line*/
						vty_out(vty,"\n%-20s","");
				   }
				   vty_out(vty,"%d,",ruleIndex+1);/*to consistant the sw and hw*/
				}
				else{
	               vty_out(vty,"%d\n",ruleIndex+1);
				}
				dbus_message_iter_next(&iter_array);
			}
			
		}

	  if(fdb_count > 0){
		  vty_out(vty,"------------------------------------------------------------\n");
		  dbus_message_iter_next(&iter);
		  dbus_message_iter_recurse(&iter,&iter_array);
		 vty_out(vty,"%-19s:","fdb-source");
		 for(i = 0;i<fdb_count;i++){		
		 	    DBusMessageIter iter_struct;
				dbus_message_iter_recurse(&iter_array,&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&vlanid);
				dbus_message_iter_next(&iter_struct);
	            dbus_message_iter_get_basic(&iter_struct,&mac[0]);
				dbus_message_iter_next(&iter_struct);
	            dbus_message_iter_get_basic(&iter_struct,&mac[1]);
				dbus_message_iter_next(&iter_struct);
	            dbus_message_iter_get_basic(&iter_struct,&mac[2]);
				dbus_message_iter_next(&iter_struct);
	            dbus_message_iter_get_basic(&iter_struct,&mac[3]);
				dbus_message_iter_next(&iter_struct);
	            dbus_message_iter_get_basic(&iter_struct,&mac[4]);
				dbus_message_iter_next(&iter_struct);
	            dbus_message_iter_get_basic(&iter_struct,&mac[5]);
				dbus_message_iter_next(&iter_struct);
	            dbus_message_iter_get_basic(&iter_struct,&slot_no);
				dbus_message_iter_next(&iter_struct);
	            dbus_message_iter_get_basic(&iter_struct,&port_no);
				if(!fdb_loop){
					vty_out(vty," %02x:%02x:%02x:%02x:%02x:%02x %d %d/%d \n", \
										mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],vlanid,slot_no,port_no);
				}
				else {
					vty_out(vty,"%-20s %02x:%02x:%02x:%02x:%02x:%02x %d %d/%d \n", \
						"",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],vlanid,slot_no,port_no);
				}
				fdb_loop++;
				dbus_message_iter_next(&iter_array);
		 }	
	   }
	  vty_out(vty,"============================================================\n");
	 	dbus_message_unref(reply);
	}
	return op_rc;
}


int dcli_mirror_show_running_cfg(struct vty *vty)
{	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
    int ret = 1;
	
	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_SHOW_RUNNING_CGF);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show mirror running config failed get reply.\n");
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
		sprintf(_tmpstr,BUILDING_MOUDLE,"MIRROR");
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


void dcli_mirror_init(void)
{
	install_node (&mirror_node, dcli_mirror_show_running_cfg, "MIRROR_NODE");
	install_default(MIRROR_NODE);

  /*mirror*/
	install_element(CONFIG_NODE,&config_mirror_cmd);
	install_element(MIRROR_NODE,&mirror_dest_port_cmd);
	install_element(MIRROR_NODE,&no_mirror_dest_port_cmd);
	install_element(MIRROR_NODE,&mirror_acl_cmd);
	install_element(MIRROR_NODE,&no_mirror_acl_cmd);
	install_element(MIRROR_NODE,&mirror_port_cmd);
	install_element(MIRROR_NODE,&no_mirror_port_cmd);
	install_element(MIRROR_NODE,&mirror_vlan_cmd);
	install_element(MIRROR_NODE,&no_mirror_vlan_cmd);
  	install_element(MIRROR_NODE,&mirror_fdb_cmd);
 	install_element(MIRROR_NODE,&no_mirror_fdb_cmd);
	install_element(MIRROR_NODE,&show_mirror_cmd);
	install_element(VIEW_NODE,&show_mirror_cmd);
	install_element(ENABLE_NODE,&show_mirror_cmd);
	install_element(CONFIG_NODE,&show_mirror_cmd);	
	install_element(MIRROR_NODE,&delete_mirror_cmd);
	install_element(HIDDENDEBUG_NODE, &debug_mirror_dest_port_cmd);		
	install_element(HIDDENDEBUG_NODE, &debug_mirror_port_cmd);
	install_element(HIDDENDEBUG_NODE, &debug_no_mirror_port_cmd);
}
#ifdef __cplusplus
}
#endif
