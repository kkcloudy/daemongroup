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
* ws_dcli_mirror.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* tangsq@autelan.com
*
* DESCRIPTION:
* function for web
* dcli_mirror.c v1.34 update 2010-06-18 by zhouym
*
*
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dbus/dbus.h>
#include "ws_dcli_mirror.h"
#include <unistd.h>
#include "ws_returncode.h"

#if 0//these functions are defined in ws_dcli_acl.c
inline int dcli_checkPoint(char *ptr)
{
	int ret = 0;
	while(*ptr != '\0')
		{
			if(((*ptr) < '0')||((*ptr) > '9'))
				{
					ret = 1;
					break;
				}
			*ptr++;
		}
	return ret;
}
inline int mac_format_check
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

inline int dcli_str2ulong(char *str,unsigned int *Value)
{
	char *endptr = NULL;
	char c;
	int ret = 0;
	if (NULL == str) return NPD_FAIL;

	ret = dcli_checkPoint(str);
	if(ret == 1){
		return NPD_FAIL;
	}

	c = str[0];
	if((strlen(str) > 1)&&('0' == c)){
		// string(not single "0") should not start with '0'
		return NPD_FAIL;
	}		
	*Value= strtoul(str,&endptr,10);
	if('\0' != endptr[0]){
		return NPD_FAIL;
	}
	return NPD_SUCCESS;	
}


  
#endif
#if 0




// error message corresponding to mirror error code
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
/* 11 */	"%% Mirror profile not exists",
/* 12 */	"%% Mirror destination port has already exist",
/* 13 */	"%% Create mirror profile error",
/* 14 */	"%% The fdb-source does not exist",
/* 15 */	"%% The fdb-source exists",
/* 16 */	"%% Mirror does not support this source",
/* 17 */	"%% Acl global not exist",
/* 18 */	"%% The acl-source exist",
/* 19 */	"%% The acl-source does not exist",
/* 20 */	"%% The destination port has source members,so can not delete",
/* 21 */	"%% Mac address given cannot be system mac address",
/* 22 */	"%% The port-source conflict",
/* 23 */	"%% There is no mirror source",
/* 24 */	"%% Profile has already been created",
/* 25 */	"%% Profile has not been created"
};
#endif

// error message corresponding to mirror error code
char *dcli_mirror_err_msg[] = {	\
/*   0 */	"Error_none",
/*   1 */	"General_failure",
/*   2 */	"illegal_input",
/*   3 */	"Memory_allocation_failure",
/*   4 */	"Policy_must_permit",
/*   5 */	"Mirror_port_configured",
/*   6 */	"port_not_configured",
/*   7 */	"Profile_out_range",
/*   8 */	"Mirror_vlan_configured",
/*	9 */	"Mirror_vlan_not_configured",
/* 10 */	"VLAN_NOT_EXITSTS",
/* 11 */	"Mirror_profile_not_exists",
/* 12 */	"destination_port_already_exist",
/* 13 */	"Create_profile_error",
/* 14 */	"fdb_source_not_exist",
/* 15 */	"fdb_source_exists",
/* 16 */	"Mirror_not_support",
/* 17 */	"Acl_global_not_exist",
/* 18 */	"acl_source_exist",
/* 19 */	"acl_source_not_exist",
/* 20 */	"destination_port_not_delete",
/* 21 */	"Mac_address_cannot_system_mac_address",
/* 22 */	"port_source_conflict",
/* 23 */	"no_mirror_source",
/* 24 */	"Profile_already_created",
/* 25 */	"Profile_not_created",
/* 26 */	"not_use_extended_rule"//"Mirror failed, please use the extended rule"  added at 2009-2-12

};

int config_mirror_profile()//创建镜像文件
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	//unsigned int op_ret = 0;
	unsigned int profile = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_CONFIG_MIRROR);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}

	dbus_message_unref(reply);
	return 0;
}
int Create_destPort(char * PortNO,char * port_mode,unsigned int profilez)/*retu=0:error; retu= -1:fail ; retu = 1:succ,retu=-2:input error*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned int	temp = 0,op_ret=0,g_eth_index=0;
	unsigned char   slot_no,port_no;	
	MIRROR_DIRECTION_TYPE direct = MIRROR_INGRESS_E;
	unsigned int profile = 0;
	int retu = 0;
	
	temp = parse_slotport_no((char*)PortNO,&slot_no,&port_no);
	if (NPD_FAIL == temp) {
    	//vty_out(vty,"%% Illegal format with slot/port!\n");
		return CMD_FAILURE;
	}
	//fprintf(stderr,"slot_no=%d-port_no=%d",slot_no,port_no);
	if(0 == strncmp("ingress",port_mode,strlen(port_mode))) {
		direct = MIRROR_INGRESS_E;
	}
	else if(0 == strncmp("egress",port_mode,strlen(port_mode))) {
		direct = MIRROR_EGRESS_E;
	}
	else if(0 == strncmp("bidirection",port_mode,strlen(port_mode))) {
		direct = MIRROR_BIDIRECTION_E;
	}
	else {
		//vty_out(vty,"%% Input bad param\n");
		return -2;
	}

	//temp = 0;
	profile = (unsigned int)profilez;
	
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
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&g_eth_index,
		DBUS_TYPE_INVALID)){	
        if(MIRROR_RETURN_CODE_SUCCESS!=op_ret){
			//char temp[50];
			//sprintf(temp,"%s",dcli_mirror_err_msg[op_ret]);
		    //vty_out(vty,dcli_mirror_err_msg[op_ret]);	
		    //ShowAlert(search(lcontrol,temp));
		    retu = -1;
		}
		else{
			//ShowAlert(search(lcontrol,"Operation_Success"));
			retu = 1;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		retu  = 0;
	}
	dbus_message_unref(reply);
	return retu;
}


int del_destPort(char * PortNO,char * port_mode,unsigned int profilez)/*retu=0:fail; retu=-1:error; retu = 1 :succ*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned int	temp = 0,op_ret = 0,profile = 0;
	unsigned char   slot_no = 0,port_no = 0;	
	MIRROR_DIRECTION_TYPE direct = MIRROR_INGRESS_E;
	int retu = 0;

	temp = parse_slotport_no((char*)PortNO,&slot_no,&port_no);
	
	if (MIRROR_RETURN_CODE_SUCCESS != temp) {
    	//vty_out(vty,"%% Illegal format with slot/port!\n");
		return CMD_FAILURE;
	}
	if(0 == strncmp("ingress",port_mode,strlen(port_mode))) {
		direct = MIRROR_INGRESS_E;
	}
	else if(0 == strncmp("egress",port_mode,strlen(port_mode))) {
		direct = MIRROR_EGRESS_E;
	}
	else if(0 == strncmp("bidirection",port_mode,strlen(port_mode))){
		direct = MIRROR_BIDIRECTION_E;
	}

	profile = (unsigned int)profilez;
	
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
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){	
		if(MIRROR_RETURN_CODE_SUCCESS!=op_ret){
			//char temp[50];
			//sprintf(temp,"%s",dcli_mirror_err_msg[op_ret]);
		   //vty_out(vty,dcli_mirror_err_msg[op_ret]);	
		    //ShowAlert(search(lcontrol,temp));
		    retu = -1;
		}
		else{
			//ShowAlert(search(lcontrol,"Operation_Success"));
			retu = 1;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{

			dbus_error_free(&err);
		}
		retu = 0;
	}
	dbus_message_unref(reply);
	return retu;
}

int mirror_policy(char * policy_index,unsigned int profilez)/*retu=0:error, retu=-1:fail, retu = 1 :succ*/
{

	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned int ruleIndex = 0,profile = 0;
	unsigned int op_ret = 0;
	int ret = 0;
	int retu = 0;
	
	ret=dcli_str2ulong((char*)policy_index,&ruleIndex);	
	if(ret==MIRROR_RETURN_CODE_ERROR)
	{
		//vty_out(vty,"%% Illegal rule index!\n");
		return CMD_FAILURE;
	}
	ruleIndex = ruleIndex-1;
	

	profile = (unsigned int)profilez;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_APPEND_MIRROR_BASE_ACL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&ruleIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){			
		if(MIRROR_RETURN_CODE_SUCCESS!=op_ret){
			//char temp[50];
			//sprintf(temp,"%s",dcli_mirror_err_msg[op_ret]);
		   //vty_out(vty,dcli_mirror_err_msg[op_ret]);	
		    //ShowAlert(search(lcontrol,temp));		
		    retu = -1;
		}
		else{
			//ShowAlert(search(lcontrol,"Operation_Success"));
			retu = 1;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		retu = 0;
	}
	dbus_message_unref(reply);
	return retu;
}


int no_mirror_policy(char * policy_index,unsigned int profilez)/*retu=0:fail; retu=-1:error; retu=1:succ*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned int ruleIndex = 0,profile = 0;
	unsigned int op_ret = 0;
	int ret = 0;
	int retu = 0;
	
	ret=dcli_str2ulong((char*)policy_index,&ruleIndex);
	if(ret==MIRROR_RETURN_CODE_ERROR)
	{
		//vty_out(vty,"%% Illegal rule index!\n");
		return CMD_FAILURE;
	}
	ruleIndex = ruleIndex-1;
	
	profile = (unsigned int)profilez;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_CANCEL_MIRROR_BASE_ACL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT32,&ruleIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {	
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){			
		if(MIRROR_RETURN_CODE_SUCCESS!=op_ret){
				//char temp[50];
				//sprintf(temp,"%s",dcli_mirror_err_msg[op_ret]);
		   		//vty_out(vty,dcli_mirror_err_msg[op_ret]);	
		    	//ShowAlert(search(lcontrol,temp));	
		    	retu = -1;
			}
		else{
			//ShowAlert(search(lcontrol,"Operation_Success"));
			retu = 1;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			
			dbus_error_free(&err);
		}
		retu = 0;
	}
	dbus_message_unref(reply);
	return retu;
}


int mirror_port(char * PortNO,char * port_mode,unsigned int profilez)/*retu=0:fail, retu=-1:error, retu=1:succ*/
{

	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int profile = 0;
	unsigned int direct = 0;
	unsigned char slot = 0,port = 0;
	int ret = 0;
	int retu = 0;
	
	ret = parse_slotport_no((char*)PortNO,&slot,&port);
	if (MIRROR_RETURN_CODE_SUCCESS != ret) {
    	//vty_out(vty,"%% Illegal format with slot/port!\n");
		return CMD_FAILURE;
	}

	if(0 == strncmp("ingress",port_mode,strlen(port_mode))) {
		direct = MIRROR_INGRESS_E;
	}
	else if(0 == strncmp("egress",port_mode,strlen(port_mode))) {
		direct = MIRROR_EGRESS_E;
	}
	else if(0 == strncmp("bidirection",port_mode,strlen(port_mode))){
		direct = MIRROR_BIDIRECTION_E;
	}
	
	profile = (unsigned int)profilez;
	

	
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
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {	
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}
	
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID))
	{	
		if(MIRROR_RETURN_CODE_SUCCESS!=ret){
				//char temp[50];
				//sprintf(temp,"%s",dcli_mirror_err_msg[ret]);
		   		//vty_out(vty,dcli_mirror_err_msg[op_ret]);	
		    	//ShowAlert(search(lcontrol,temp));
		    	retu = -1;
		}
		else{
			//ShowAlert(search(lcontrol,"Operation_Success"));
			retu = 1;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
		
			dbus_error_free(&err);
		}
		retu  = 0;
	}
	dbus_message_unref(reply);
	return retu;
}

int no_mirror_port(char * PortNO,char * port_mode,unsigned int profilez)/*retu=1:fail; retu=-1:error; retu=1:succ*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	unsigned int profile = 0;
	unsigned char slot = 0,port = 0;
	unsigned int direct = 0;
	int ret = 0;
	int retu = 0;
	
	ret = parse_slotport_no((char*)PortNO,&slot,&port);
	if (MIRROR_RETURN_CODE_SUCCESS != ret) {
    	//vty_out(vty,"%% Illegal format with slot/port!\n");
		return CMD_FAILURE;
	}
	if(0 == strncmp("ingress",port_mode,strlen(port_mode))) {
			direct = MIRROR_INGRESS_E;
	}
	else if(0 == strncmp("egress",port_mode,strlen(port_mode))) {
		direct = MIRROR_EGRESS_E;
	}
	else if(0 == strncmp("bidirection",port_mode,strlen(port_mode))){
		direct = MIRROR_BIDIRECTION_E;
	}
	

	profile = (unsigned int)profilez;

	
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
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}

	
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&ret,
		DBUS_TYPE_INVALID))
	{	
		if(MIRROR_RETURN_CODE_SUCCESS!=ret){
			    //char temp[50];
				//sprintf(temp,"%s",dcli_mirror_err_msg[ret]);
		   		//vty_out(vty,dcli_mirror_err_msg[op_ret]);	
		    	//ShowAlert(search(lcontrol,temp));
		    	retu = -1;
		}
		else{
			//ShowAlert(search(lcontrol,"Operation_Success"));
			retu = 1;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		retu = 0;
	}
	dbus_message_unref(reply);
	return retu;
}


int mirror_vlan(char * VlanID,unsigned int profilez)/*retu=0:error,retu=-1:fail;retu=1:succ*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned short vid = 0;
	unsigned int op_ret = 0,profile = 0;
	int ret = 0;
	int retu = 0;
	
	ret=parse_vlan_no((char*)VlanID,&vid);	
	if(MIRROR_RETURN_CODE_SUCCESS != ret)
	{
		//vty_out(vty,"%% Illegal rule index!\n");
		return CMD_FAILURE;
	}

	profile = (unsigned int)profilez;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_APPEND_MIRROR_BASE_VLAN_CREATE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT16,&vid,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
		if(MIRROR_RETURN_CODE_SUCCESS!=op_ret){
			//char temp[50];
				//sprintf(temp,"%s",dcli_mirror_err_msg[op_ret]);
		   		//vty_out(vty,dcli_mirror_err_msg[op_ret]);	
		    	//ShowAlert(search(lcontrol,temp));			
		    	retu = -1;
		}
		else{
			//ShowAlert(search(lcontrol,"Operation_Success"));
			retu = 1;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		retu = 0;
	}
	dbus_message_unref(reply);
	return retu;
}


int no_mirror_vlan(char * VlanID,unsigned int profilez)/*retu=0:fail; retu=-1:error; retu=1:succ*/
{

	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned short vid = 0;
	unsigned int op_ret = 0,profile = 0;
	int ret = 0;
	int retu = 0;
	
	ret=parse_vlan_no((char*)VlanID,&vid);	
	if(MIRROR_RETURN_CODE_SUCCESS != ret)
	{
		//vty_out(vty,"%% Illegal rule index!\n");
		return CMD_FAILURE;
	}



	profile = (unsigned int)profilez;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_APPEND_MIRROR_BASE_VLAN_DEL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_UINT16,&vid,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
		if(MIRROR_RETURN_CODE_SUCCESS!=ret){
			//char temp[50];
			//sprintf(temp,"%s",dcli_mirror_err_msg[ret]);
		   	//vty_out(vty,dcli_mirror_err_msg[op_ret]);	
		    //ShowAlert(search(lcontrol,temp));	
		    retu = -1;
		}
		else{
			//ShowAlert(search(lcontrol,"Operation_Success"));
			retu = 1;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		retu = 0;
	}
	dbus_message_unref(reply);
	return retu;
}


int mirror_fdb(char * mac,char * vlanid,char * PortNo,unsigned int profilez)/*retu=0:fail; retu=-1:error; retu=1:succ*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned int profile = 0;
	unsigned int op_ret = 0;
	int retu = 0;
	ETHERADDR macAddr = {0};
	unsigned short vlanId = 0;
	unsigned char   slot_no = 0,port_no = 0;	
	
	memset(&macAddr,0,sizeof(ETHERADDR));

    op_ret = parse_mac_addr((char *)mac,&macAddr);
	if (MIRROR_RETURN_CODE_SUCCESS != op_ret) {
		//vty_out(vty,"Unknow mac addr format!\n");
		//return NPD_FDB_ERR_NONE;
		return 2;
	}
   
	op_ret=is_muti_brc_mac(&macAddr);
	if(op_ret==1){
		//vty_out(vty,"erro:input broadcast or multicast mac!\n");
		return NPD_FDB_ERR_NONE;
	}
	
	op_ret = parse_short_parse((char*)vlanid, &vlanId);
	if (MIRROR_RETURN_CODE_SUCCESS != op_ret) {
		//vty_out(vty,"Unknow vlan id format.\n");
		return NPD_FDB_ERR_NONE;
	}
	if ((vlanId<MIN_VLANID)||(vlanId>MAX_VLANID)){
		//vty_out(vty,"FDB vlan outrange!\n");
		return NPD_FDB_ERR_NONE;
	}

    /*check port*/
	op_ret = parse_slotport_no((char*)PortNo,&slot_no,&port_no);
	if (MIRROR_RETURN_CODE_SUCCESS != op_ret) {
    	//vty_out(vty,"%% Illegal format with slot/port!\n");
		return NPD_FDB_ERR_NONE;
	}
		//macAddr.arEther[1],macAddr.arEther[2],macAddr.arEther[3],macAddr.arEther[4],macAddr.arEther[5]);

	profile = (unsigned int)profilez;

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
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
       if(MIRROR_RETURN_CODE_SUCCESS!=op_ret){
			//char temp[50];
			//sprintf(temp,"%s",dcli_mirror_err_msg[op_ret]);
		   	//vty_out(vty,dcli_mirror_err_msg[op_ret]);	
		    //ShowAlert(search(lcontrol,temp));
		    retu = -1;
		}
	   else{
			//ShowAlert(search(lcontrol,"Operation_Success"));
			retu = 1;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		retu = 0;
	}
	dbus_message_unref(reply);
	return retu;
}


int no_mirror_fdb(char * mac,char * vlanid,char * PortNo,unsigned int profilez)/*retu=0:fail; retu=-1:error; retu=1:succ*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned int profile = 0;
	unsigned int op_ret = 0;
	int ret = 0;
	ETHERADDR macAddr = {0};
	unsigned short vlanId = 0;
	unsigned char slot_no = 0,port_no = 0;
	int retu = 0;
	
	memset(&macAddr,0,sizeof(ETHERADDR));

	op_ret = parse_mac_addr((char *)mac,&macAddr);
	if (MIRROR_RETURN_CODE_SUCCESS != op_ret) {
		//vty_out(vty,"Unknow mac addr format!\n");
		return NPD_FDB_ERR_NONE;
	}
   
	op_ret=is_muti_brc_mac(&macAddr);
	if(op_ret==1){
		//vty_out(vty,"erro:input broadcast or multicast mac!\n");
		return NPD_FDB_ERR_NONE;
	}
	
	op_ret = parse_short_parse((char*)vlanid, &vlanId);
	if (MIRROR_RETURN_CODE_SUCCESS != op_ret) {
		//vty_out(vty,"Unknow vlan id format.\n");
		return NPD_FDB_ERR_NONE;
	}
	if ((vlanId<MIN_VLANID)||(vlanId>MAX_VLANID)){
		//vty_out(vty,"FDB vlan outrange!\n");
		return NPD_FDB_ERR_NONE;
	}

	op_ret = parse_slotport_no((char*)PortNo,&slot_no,&port_no);
	if (MIRROR_RETURN_CODE_SUCCESS != op_ret) {
    	//vty_out(vty,"%% Illegal format with slot/port!\n");
		return NPD_FDB_ERR_NONE;
	}
	profile = (unsigned int)profilez;
	
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
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return 0;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
      if(MIRROR_RETURN_CODE_SUCCESS!=ret){
			//char temp[50];
			//sprintf(temp,"%s",dcli_mirror_err_msg[ret]);
		   	//vty_out(vty,dcli_mirror_err_msg[op_ret]);	
		    //ShowAlert(search(lcontrol,temp));
		    retu = -1;
		}
	  else{
			//ShowAlert(search(lcontrol,"Operation_Success"));
			retu = 1;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		retu = 0;
	}
	dbus_message_unref(reply);
	return retu;
}

int show_mirror_configure(struct mirror_info * rev_mirror_info)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	

	//unsigned int	temp = 0,op_ret =0,g_eth_index =0;
	unsigned int ruleIndex = 0;
	unsigned char   slot_no = 0,port_no = 0,bi_slot_no = 0,bi_port_no = 0,in_slot_no = 0,in_port_no = 0, eg_slot_no = 0,eg_port_no = 0;	
	unsigned int ret = 0,i = 0;
	//unsigned int rc = 0,fdb_loop = 0;
	unsigned int port_count = 0,vlan_count = 0,policy_count = 0,fdb_count = 0;
	unsigned int bi_flag = 0,in_flag = 0,eg_flag = 0;
	unsigned short vlanid = 0;
	unsigned char mac[6] = {0};
	unsigned int profile=0;
	//MIRROR_DIRECTION_TYPE direct = MIRROR_INGRESS_E;
	MIRROR_DIRECTION_TYPE port_direct = MIRROR_INGRESS_E;
	//mirror_info *q,*tail;
	//rev_mirror_info=(mirror_info * )malloc(sizeof(mirror_info));
	rev_mirror_info->destPort_bid=(char *)malloc(10);
	memset(rev_mirror_info->destPort_bid,0,10);
	rev_mirror_info->destPort_in=(char *)malloc(10);
	memset(rev_mirror_info->destPort_in,0,10);
	rev_mirror_info->destPort_eg=(char *)malloc(10);
	memset(rev_mirror_info->destPort_eg,0,10);
	rev_mirror_info->bid_flag = 0;
	rev_mirror_info->in_flag = 0;
	rev_mirror_info->eg_flag = 0;
	rev_mirror_info->PortNum = 0;
	rev_mirror_info->fdbNum = 0;
	rev_mirror_info->VlanNum = 0;
	rev_mirror_info->policyindexNum = 0;

	for(i=0;i<mirror_number;i++)
		{
			rev_mirror_info->VlanID[i]=0;
			rev_mirror_info->policyindex[i]=0;
			rev_mirror_info->fdb_vlan[i]=0;
			rev_mirror_info->PortNo[i]=(char *)malloc(10);
			memset(rev_mirror_info->PortNo[i],0,10);
			rev_mirror_info->Port_mode[i]=(char *)malloc(20);
			memset(rev_mirror_info->Port_mode[i],0,20);
			rev_mirror_info->Mac[i]=(char *)malloc(30);
			memset(rev_mirror_info->Mac[i],0,30);
			rev_mirror_info->fdb_port[i]=(char *)malloc(10);
			memset(rev_mirror_info->fdb_port[i],0,10);
		}
	//fprintf(stderr,"33333");

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_MIRROR_OBJPATH,
										 NPD_DBUS_MIRROR_INTERFACE,
										 NPD_DBUS_METHOD_MIRROR_SHOW);

	
	profile = 0;
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profile,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if (MIRROR_DESTINATION_NODE_NOTEXIST == ret){
	   //vty_out(vty,"Warning:no destination port,configuration may not take effect!\n");
	   dbus_message_unref(reply);
       return CMD_SUCCESS;
	}
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
	//vty_out(vty,"Detailed mirror profile:\n");
	//vty_out(vty,"============================================================\n");
	if(bi_flag){
		//vty_out(vty,"%-20s:%d/%d\n","Bidirection Destination",bi_slot_no,bi_port_no);
		//strcpy(rev_mirror_info->destPort_mode,"Bidirection");
		rev_mirror_info->bid_flag = bi_flag ;
		sprintf(rev_mirror_info->destPort_bid,"%d/%d",bi_slot_no,bi_port_no);
	}
	if(in_flag){
		//vty_out(vty,"%-20s:%d/%d\n","Ingress Destination",in_slot_no,in_port_no);
		//strcpy(rev_mirror_info->destPort_mode,"Ingress");
		rev_mirror_info->in_flag = in_flag ;
		sprintf(rev_mirror_info->destPort_in,"%d/%d",in_slot_no,in_port_no);
	}
	if(eg_flag){
		//vty_out(vty,"%-20s:%d/%d\n","Egress Destination",eg_slot_no,eg_port_no);
		//strcpy(rev_mirror_info->destPort_mode,"Egress");
		rev_mirror_info->eg_flag = eg_flag ;
		sprintf(rev_mirror_info->destPort_eg,"%d/%d",eg_slot_no,eg_port_no);
	}
	if((!bi_flag)&&(!in_flag)&&(!eg_flag)){
        //vty_out(vty,"warning:no destination port!");
        //strcpy(rev_mirror_info->destPort_mode,"no-destination-port");
		dbus_message_unref(reply);
        return CMD_SUCCESS;
	}
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&port_count);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&vlan_count);
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&policy_count);
    dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&fdb_count);
	rev_mirror_info->PortNum=port_count;
	rev_mirror_info->VlanNum=vlan_count;
	rev_mirror_info->fdbNum=fdb_count;
	rev_mirror_info->policyindexNum=policy_count;
	if(port_count > 0){
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter,&iter_array);
		for(i = 0;i<port_count;i++){
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&slot_no);
			dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct,&port_no);
			dbus_message_iter_next(&iter_struct);
            dbus_message_iter_get_basic(&iter_struct,&port_direct);
			/*if(i<(port_count-1)) {
			  if((0!=i) && (0 == i%6)) { // show at most 8-port per line
					vty_out(vty,"\n%-20s","");
			   }
			   vty_out(vty,"%d/%d(%s),",slot_no,port_no,(MIRROR_INGRESS_E == port_direct)? "I":(MIRROR_EGRESS_E == port_direct)?"E" :"B" );
			}
			else{
               vty_out(vty,"%d/%d(%s)\n",slot_no,port_no,(MIRROR_INGRESS_E == port_direct)? "I":(MIRROR_EGRESS_E == port_direct)?"E" :"B");
			}*/
			sprintf(rev_mirror_info->PortNo[i],"%d/%d",slot_no,port_no);
			strcpy(rev_mirror_info->Port_mode[i],(MIRROR_INGRESS_E == port_direct)? "Ingress":(MIRROR_EGRESS_E == port_direct)?"Egress" :"Bidirection" );
			dbus_message_iter_next(&iter_array);
		}

	}
	if(vlan_count > 0){
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter,&iter_array);
		//vty_out(vty,"%-19s:","vlan-source");
		for(i = 0;i<vlan_count;i++){		
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&vlanid);
			/*if(i<(vlan_count-1)) {
 			   if((0!=i) && (0==(i%8))) { // show at most 8-vlan per line
 				  vty_out(vty,"\n%-20s","");
 			   }
			   vty_out(vty,"%d,",vlanid);
			}
			else{
               vty_out(vty,"%d\n",vlanid);
			}*/
			rev_mirror_info->VlanID[i]=vlanid;
			dbus_message_iter_next(&iter_array);
		}
		
	}
	if(policy_count > 0){

		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter,&iter_array);
		//vty_out(vty,"%-19s:","policy-source");
		for(i = 0;i<policy_count;i++){		
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&ruleIndex);
			/*if(i<(policy_count-1)) {
			   if((0!=i) && (0==(i%8))) { // show at most 8-policy per line
					vty_out(vty,"\n%-20s","");
			   }
			   vty_out(vty,"%d,",ruleIndex+1);//to consistant the sw and hw
			}
			else{
               vty_out(vty,"%d\n",ruleIndex+1);
			}*/
			rev_mirror_info->policyindex[i]=ruleIndex+1;
			dbus_message_iter_next(&iter_array);
		}
		
	}

  if(fdb_count > 0){
	  dbus_message_iter_next(&iter);
	  dbus_message_iter_recurse(&iter,&iter_array);
	// vty_out(vty,"%-19s:","fdb-source");
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
			/*if(!fdb_loop){
				vty_out(vty," %02x:%02x:%02x:%02x:%02x:%02x %d %d/%d \n", \
									mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],vlanid,slot_no,port_no);
			}
			else {
				vty_out(vty,"%-20s %02x:%02x:%02x:%02x:%02x:%02x %d %d/%d \n", \
					"",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],vlanid,slot_no,port_no);
			}
			fdb_loop++;*/
			sprintf(rev_mirror_info->Mac[i],"%02x:%02x:%02x:%02x:%02x:%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
			rev_mirror_info->fdb_vlan[i]=vlanid;
			sprintf(rev_mirror_info->fdb_port[i],"%d/%d",slot_no,port_no);
			dbus_message_iter_next(&iter_array);
	 }	
   }
  
 	dbus_message_unref(reply);
	return 0;
}


