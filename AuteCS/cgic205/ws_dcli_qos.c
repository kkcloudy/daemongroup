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
* ws_dcli_qos.c
*
*
* CREATOR:
* autelan.software.Network Dep. team
* zhouym@autelan.com
*
* DESCRIPTION:
* function for web
*
*
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dbus/dbus.h>
#include "ws_dcli_qos.h"
/*#include "cgic.h"*/
#include <unistd.h>
#include "ws_returncode.h"
///////////////////////////////////////////////////////////////////
/* dcli_qos.c  version :v1.55  2009-12-25*/   
//////////////////////////////////////////////////////////////////

 int dcli_checkPoint_EX(char *ptr)
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



int dcli_str2ulong_EX(char *str,unsigned int *Value)
 {
	 char *endptr = NULL;
	 char c;
	 int ret = 0;
	 if (NULL == str) return NPD_FAIL;
 	
	 ret = dcli_checkPoint_EX(str);
	 if(ret == 1)
	 {
		 return NPD_FAIL;
	 }

	  c = str[0];
	  if((strlen(str) > 1)&&('0' == c)){
		return NPD_FAIL;
	}
	
	 *Value= strtoul(str,&endptr,10);
	
	 if('\0' != endptr[0]){
		 return NPD_FAIL;
	 }
	 return NPD_SUCCESS; 
 }


int INDEX_LENTH_CHECK(char * str,int num)	
{
	if(num<(strlen(str)))
		return 25;
	return 1;
}
unsigned long dcli_str2ulong_QOS(char *str)
{
	unsigned long val = 0;
	char * endChr;
	int base = 10;
	
	if(NULL == str)
		return 0;
	else
		val = strtoul(str, &endChr, base);

	return val;
}

int dcli_str2ulond(char *str,unsigned int *Value)
{
	char *endptr = NULL;
	char c;
	if (NULL == str) return NPD_FAIL;
	c = str[0];
	if (c>='0'&&c<='9'){
		*Value= strtoul(str,&endptr,10);
		if('\0' != endptr[0]){
				return NPD_FAIL;
		}
		return NPD_SUCCESS;	
	}
	else {
		return NPD_FAIL; 
	}
}

int get_one_port_index_QOS(char * slotport,unsigned int* port_index)
{
	DBusMessage *query, *reply;
	DBusError err;
	int op_ret = 0,ret=0;
	unsigned int eth_g_index = 0;
	unsigned char slot_no = 0,port_no=0;
	
	ret = parse_slotport_no((char *)slotport,&slot_no,&port_no);
	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
											NPD_DBUS_BUSNAME, 	\
											NPD_DBUS_ETHPORTS_OBJPATH,	\
											NPD_DBUS_ETHPORTS_INTERFACE, \
											STP_DBUS_METHOD_GET_PORT_INDEX
											);
	
	
		dbus_message_append_args(query,
										DBUS_TYPE_BYTE,&slot_no,										
										DBUS_TYPE_BYTE,&port_no,									
										DBUS_TYPE_INVALID);


	 reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}
	
	dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_UINT32,&eth_g_index,
								DBUS_TYPE_INVALID);
	*port_index=eth_g_index;
	return 0;
}

int  set_qos_profile(char * qosindex,struct list * lcontrol)  /*返回0表示成功，返回-1表示失败，返回-2表示Illegal qos profile index!返回-3表示Fail to config qos profile*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	int retu = 0;
	unsigned int profileIndex = 0;
	unsigned int op_ret = 0;
	int ret = 0;
	ret = dcli_str2ulong((char*)qosindex,&profileIndex);	
    if(ret==QOS_RETURN_CODE_ERROR)
	{
		//vty_out(vty,"%% Illegal qos profile index!\n");
		return -2;
	}
	//int k=INDEX_LENTH_CHECK((char*)qosindex,3);
	//if(25==k)
	//{
	//	ShowAlert(search(lcontrol,"illegal_input"));
	//}
   //profileIndex = dcli_str2ulong_QOS((char*)qosindex);	
   // profileIndex =profileIndex-1;
    	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_CONFIG_QOS_PROFILE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profileIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//return CMD_SUCCESS;
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
            if(QOS_RETURN_CODE_SUCCESS==op_ret){
                 /* if(CONFIG_NODE==vty->node){
                      vty->node = QOS_PROFILE_NODE;
                      vty->index = (void*)profileIndex;                           
                  }   */ 
                  retu = 0;

            }
             else {
                 //vty_out(vty,"%% Fail to config qos profile\n");     
				 retu = -3;
             }
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		retu = -1;
	}
	
	dbus_message_unref(reply);
	return retu;	
}

int set_qos_profile_atrribute(char * qosIndex,char * dpparam,char * upparam,char * tcparam,char * dscpparam,struct list * lcontrol) /*0-1,0-7,0-7,0-63*/
	                       /*返回0表示成功，返回-1表示失败，返回-2表示非法输入，返回-3表示Fail to config qos profile*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int profileIndex = 0, dp = 0, up = 0, tc = 0, dscp = 0;
	unsigned int op_ret = 0;
	unsigned int k=0;
	int retu = 0;

	k=INDEX_LENTH_CHECK((char*)dpparam,1);
	if(25==k)
	{
		//ShowAlert(search(lcontrol,"illegal_input"));
		retu = -2;
	}
	k=INDEX_LENTH_CHECK((char*)upparam,1);
	if(25==k)
	{
		//ShowAlert(search(lcontrol,"illegal_input"));
		retu = -2;
	}
	k=INDEX_LENTH_CHECK((char*)tcparam,1);
	if(25==k)
	{
		//ShowAlert(search(lcontrol,"illegal_input"));
		retu = -2;
	}
	k=INDEX_LENTH_CHECK((char*)dscpparam,2);
	if(25==k)
	{
		//ShowAlert(search(lcontrol,"illegal_input"));
		retu = -2;
	}

    dp = dcli_str2ulong_QOS((char*)dpparam);	
    up = dcli_str2ulong_QOS((char*)upparam);
    tc = dcli_str2ulong_QOS((char*)tcparam);
    dscp = dcli_str2ulong_QOS((char*)dscpparam);
    
    profileIndex = dcli_str2ulong_QOS((char*)qosIndex);	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_QOS_PROFILE_ATTRIBUTE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&profileIndex,     
							 DBUS_TYPE_UINT32,&dp,
						     DBUS_TYPE_UINT32,&up,
						     DBUS_TYPE_UINT32,&tc,
						     DBUS_TYPE_UINT32,&dscp,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//return CMD_SUCCESS;
		retu  =-1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
			 if(QOS_RETURN_CODE_PROFILE_NOT_EXISTED == op_ret){
                   //vty_out(vty,"%% Fail to config qos profile\n"); 
                   //ShowAlert(search(lcontrol,"set_qos_profile_attr_fail"));
				   retu = -3;
            }
			 else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
                   //ShowAlert(search(lcontrol,"set_qos_profile_suc"));
				   retu = 0;
            } 
                          
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		retu = -1;
	}
	dbus_message_unref(reply);
	return retu;
}
int delete_qos_profile(char * qosindex,struct list * lcontrol) /*返回0表示成功，返回-1表示失败，返回-2表示Illegal qos profile index!*/
	                            /*返回-3表示QoS profile not existed返回-4表示qos_profile_in_use返回-5表示Fail to delete*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;	

	unsigned int profileIndex = 0;
	unsigned int op_ret = 0;
	int ret = 0,retu = 0;

	ret = dcli_str2ulong((char*)qosindex,&profileIndex);	
	if(ret==QOS_RETURN_CODE_ERROR)
	{
		//vty_out(vty,"%% Illegal qos profile index!\n");
		return -2;
	}
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_DELETE_QOS_PROFILE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&profileIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//return CMD_SUCCESS;
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{			
			if(op_ret==QOS_RETURN_CODE_PROFILE_NOT_EXISTED){
				//vty_out(vty,"%% QoS profile %d not existed!\n",profileIndex);
				
				retu = -3;
			}
			else if(op_ret==QOS_RETURN_CODE_PROFILE_IN_USE) {
			
				 
                 retu = -4;
			 }
			else if(op_ret!=QOS_RETURN_CODE_SUCCESS){
				//vty_out(vty,"%% Fail to delete!\n");
				retu = -5;
			}			
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		retu = -1;
	}
	dbus_message_unref(reply);
	return retu;
}

int set_dscp_to_profile(char * dscpindex,char * profileindex,struct list * lcontrol)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int profileIndex = 0, dscp = 0;
	unsigned int op_ret = 0;
	int retu = 0;

    dcli_str2ulong((char*)dscpindex,&dscp);
    dcli_str2ulong((char*)profileindex,&profileIndex);

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_DSCP_PROFILE_TABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,                               						
						     DBUS_TYPE_UINT32,&dscp,
						     DBUS_TYPE_UINT32,&profileIndex, 
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//return CMD_SUCCESS;
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
			if(QOS_RETURN_CODE_PROFILE_NOT_EXISTED ==op_ret){
				
				retu = -2;
			}
			else if(ACL_RETURN_CODE_HYBRID_DSCP == op_ret) {
				//vty_out(vty,"%% Qos mode is hybrid, qos-profile index is 1~64\n");  
				retu = -3;
			}
			else if(QOS_RETURN_CODE_ERROR== op_ret){
				//vty_out(vty,"%% Fail to config dscp to qos profile map table \n");      
				retu = -4;
           	}       
	}
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

int del_dscp_to_profile(char * dscpindex,struct list * lcontrol)/*返回0表示成功，返回-1表示失败，返回-2表示Delete dscp to qos profile map not exis*/
	                                                            /*返回-3表示Fail to delete dscp to qos profile map table */
{	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int dscp = 0;
	unsigned int op_ret = 0;
	int retu = 0;
	
    dcli_str2ulong((char*)dscpindex,&dscp);
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_DELETE_DSCP_PROFILE_TABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,                               						
						     DBUS_TYPE_UINT32,&dscp,					  
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//return CMD_SUCCESS;
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{			
			 if(QOS_RETURN_CODE_NO_MAPPED == op_ret){
                // vty_out(vty,"%% Delete dscp to qos profile map not exist \n");  
                retu  = -2;
             } 	
			 else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
                 //vty_out(vty,"%% Fail to delete dscp to qos profile map table \n");   
				 ShowAlert(search(lcontrol,"del_map_dscp_qos_fail"));
				 retu = -3;
            }       
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu;
}

int set_dscp_to_dscp(char * dscpsrc,char * dscpdst,struct list * lcontrol)/*返回0表示成功，返回-1表示失败，返回-2表示device not support dscp to dscp remap table返回-3表示Fail to config dscp to dscp remap table*/
{

	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int oldDscp = 0, newDscp = 0;
	unsigned int op_ret = 0;
    int retu = 0;

	dcli_str2ulong((char*)dscpsrc,&oldDscp);
    dcli_str2ulong((char*)dscpdst,&newDscp);
       
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_DSCP_DSCP_TABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&oldDscp,     						
						     DBUS_TYPE_UINT32,&newDscp,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//return CMD_SUCCESS;
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	

		if (ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT == op_ret) {
			 //vty_out(vty,"%% device not support dscp to dscp remap table \n"); 
			 retu = -2;
		}
		if(QOS_RETURN_CODE_SUCCESS!=op_ret){
		       //vty_out(vty,"%% Fail to config dscp to dscp remap table \n");      
		       retu = -3;
		}            
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

int del_dscp_to_dscp(char * dscpsrc,struct list * lcontrol)/*返回0表示成功，返回-1表示失败，返回-2表示delete dscp to dscp remap table not exist返回-3表示Fail to delete dscp to dscp remap table*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int oldDscp = 0;
	unsigned int op_ret = 0;
    int retu = 0;
	
	dcli_str2ulong((char*)dscpsrc,&oldDscp);        
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_DELETE_DSCP_DSCP_TABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&oldDscp,     						
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//return CMD_SUCCESS;
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	

		if(QOS_RETURN_CODE_NO_MAPPED == op_ret) {
			//vty_out(vty,"%% delete dscp to dscp remap table not exist\n");
			retu = -2;
		}
        else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
            //vty_out(vty,"%% Fail to delete dscp to dscp remap table \n"); 
            retu = -3;
        }       
                   
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

int set_up_to_profile(char * upindex,char * profileindex,struct list * lcontrol)/*返回0表示成功，返回-1表示失败，返回-2表示qos_profile_not_exist，返回-3表示Qos mode is hybrid, qos-profile index is 65~72*/
	                         /*返回-4表示device not support up to profile for change dscp, so dscp will not change，返回-5表示Fail to config up to profile table*/
{

	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int up = 0, profileIndex = 0;
	unsigned int op_ret = 0;
    int retu = 0;

    dcli_str2ulong((char*)upindex,&up);
    dcli_str2ulong((char*)profileindex,&profileIndex);

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_UP_PROFILE_TABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&up,     						
						     DBUS_TYPE_UINT32,&profileIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//return CMD_SUCCESS;
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
			if(QOS_RETURN_CODE_PROFILE_NOT_EXISTED ==op_ret){				
				retu = -2;
			}
			else if(ACL_RETURN_CODE_HYBRID_UP== op_ret) {
				//vty_out(vty,"%% Qos mode is hybrid, qos-profile index is 65~72\n");  
				retu = -3;
			}
			else if(ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT == op_ret) {
				//vty_out(vty,"%% device not support up to profile for change dscp, so dscp will not change\n");
				retu = -4;
			}
			else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
              // vty_out(vty,"%% Fail to config up to profile table \n");      
              retu = -5;
           }      
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

int del_up_to_profile(char * upindex,struct list * lcontrol)/*返回0表示成功，返回-1表示失败返回-2表示delete up to profile table not exist返回-3表示Fail to delete up to profile table*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int up = 0;
	unsigned int op_ret = 0;
    int retu = 0;
	
    dcli_str2ulong((char*)upindex,&up);
  
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_DELETE_UP_PROFILE_TABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&up,     						
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//return CMD_SUCCESS;
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{			
		if(QOS_RETURN_CODE_NO_MAPPED == op_ret) {
			//vty_out(vty,"%%delete up to profile table not exist\n");
			return -2;
		}
		else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
            //vty_out(vty,"%% Fail to delete up to profile table \n");      
            return -3;
        }    
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

int create_policy_map(char * policyID,struct list * lcontrol)/*返回0表示成功，返回-1表示失败，返回-2表示policy_map_exist，返回-3表示create_policy_map_fail*/
{

	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int policyIndex = 0;
	unsigned int op_ret = 0;
    int retu = 0;
	
	dcli_str2ulong((char*)policyID,&policyIndex);
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_CREATE_POLICY_MAP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policyIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//return CMD_SUCCESS;
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_POLICY_EXISTED == op_ret){
				retu = -2;
			 }
			else if (QOS_RETURN_CODE_SUCCESS!=op_ret){			 
				 retu = -3;
			 }
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

int del_policy_map(char * policyID,struct list * lcontrol)/*返回0表示成功，返回-1表示失败，返回-2表示Policy map %d not existed，返回-3表示Since policy map has been binded to some port,cannot delete*/
	                                                /*返回-4表示 Error! Fail to delete policy map*/
{

	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int policyIndex = 0;
	unsigned int op_ret = 0;
    int retu = 0;
	
	dcli_str2ulong((char*)policyID,&policyIndex);	

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_DELETE_POLICY_MAP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policyIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//return CMD_SUCCESS;
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_POLICY_NOT_EXISTED == op_ret){
				//vty_out(vty,"%%  Error! Policy map %d not existed!\n",policyIndex);
				
				retu = -2;
			 }
			else if(QOS_RETURN_CODE_POLICY_MAP_BIND==op_ret){
				//vty_out(vty,"%% Error! Since policy map has been binded to some port,cannot delete!\n");
				
				retu = -3;
			}
			else if (QOS_RETURN_CODE_SUCCESS!=op_ret){			 
				 //vty_out(vty,"%%  Error! Fail to delete policy map\n"); 
				 
				 retu = -4;
			 }
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

int config_policy_map(char * policyID,struct list * lcontrol)/*返回0表示成功，返回-1表示失败，返回-2表示policy_map_not_exist*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int policyIndex=0;
	unsigned int op_ret = 0;
    int retu = 0;

	dcli_str2ulong((char*)policyID,&policyIndex);
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
											 NPD_DBUS_QOS_OBJPATH,
											 NPD_DBUS_QOS_INTERFACE,
											 NPD_DBUS_METHOD_CONFIG_POLICY_MAP);
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&policyIndex,
								 DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
		dbus_message_unref(query);
		if (NULL == reply) {
			if (dbus_error_is_set(&err)) {
				dbus_error_free(&err);
			}
			//return CMD_SUCCESS;
			retu = -1;
		}
		if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID))
		{	
				if(QOS_RETURN_CODE_POLICY_EXISTED==op_ret){
					  /*if(CONFIG_NODE==vty->node){
						  vty->node = POLICY_NODE;
						  vty->index = (void*)policyIndex;						  
					  } 	*/				 
				}
				else if(QOS_RETURN_CODE_POLICY_NOT_EXISTED == op_ret){
					ShowAlert(search(lcontrol,"policy_map_not_exist"));  
					retu = -2;
				 }
				else if(QOS_RETURN_CODE_POLICY_MAP_BIND == op_ret){
					retu = -1;
				 }
				
		} 
		else 
		{		
			if (dbus_error_is_set(&err)) 
			{
				dbus_error_free(&err);
			}
		}
		dbus_message_unref(reply);
		return retu ;
}

int allow_qos_marker(char * enable,char * policyID,struct list * lcontrol)/*返回0表示成功，返回-1表示失败*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int IsEnable = 0,g_policy_index=0;
	unsigned int op_ret = 0;
    int retu = 0;	
    if(strncmp("enable",enable,strlen(enable))==0)
        IsEnable = 0;
    else if(strncmp("disable",enable,strlen(enable))==0)
        IsEnable =1 ;

	dcli_str2ulong((char*)policyID,&g_policy_index);


	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_MODIFY_MARK_QOS);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&IsEnable,  
                             DBUS_TYPE_UINT32,&g_policy_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//return CMD_FAILURE;
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
        if(QOS_RETURN_CODE_SUCCESS!=op_ret){
             retu =-1;
		}       
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

int set_default_up(char * upindex,char * policyID,struct list * lcontrol)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	
	unsigned int up,g_policy_index=0;
	unsigned int op_ret;

	int k=INDEX_LENTH_CHECK((char*)upindex,1);
	if(25==k)
	{
		ShowAlert(search(lcontrol,"illegal_input"));
	}
    up = dcli_str2ulong_QOS((char*)upindex);

	    g_policy_index = dcli_str2ulong_QOS((char*)policyID);
   	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_DEFAULT_UP);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&up,  
                             DBUS_TYPE_UINT32,&g_policy_index,
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
            if(QOS_SUCCESS!=op_ret){
                   //vty_out(vty,"%% Fail to set default up!\n");      
                   
            }       
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int set_default_qos(char * qosprofile,char * policyindex,struct list * lcontrol)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	
	unsigned int ProfileIndex,g_policy_index=0;
	unsigned int op_ret;

	int k=INDEX_LENTH_CHECK((char*)qosprofile,3);
	if(25==k)
	{
		ShowAlert(search(lcontrol,"illegal_input"));
	}
    ProfileIndex = dcli_str2ulong_QOS((char*)qosprofile);
	//ProfileIndex =ProfileIndex-1;

	 g_policy_index = dcli_str2ulong_QOS((char*)policyindex);
   	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_DEFAULT_QOS_PROFILE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&ProfileIndex,  
                             DBUS_TYPE_UINT32,&g_policy_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
            if(QOS_PROFILE_NOT_EXISTED==op_ret){
                   ShowAlert(search(lcontrol,"qos_profile_not_exist"));
            }  
			else if(QOS_SUCCESS!=op_ret){
                 //vty_out(vty,"%% Fail to set default qos profile!\n");
			}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int trust_l2_up(char *enable,char * policyindex,struct list * lcontrol)/*返回0表示成功，返回-1表示失败返回-2表示trust mode layer 2 device not suport change dscp，返回-3表示Fail to config port trust mode layer 2*/
{

	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int EnableUp = 0,g_policy_index=0;
	unsigned int op_ret = 0;
    int retu  =0;

	dcli_str2ulong((char*)policyindex,&g_policy_index);
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_PORT_TRUST_L2_MODE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&EnableUp,  
                             DBUS_TYPE_UINT32,&g_policy_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//return CMD_FAILURE;
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
		if (ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT == op_ret) {
			//vty_out(vty,"%% trust mode layer 2 device not suport change dscp\n");
			retu = -2;
		}
		else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
			//vty_out(vty,"%% Fail to config port trust mode layer 2!\n");      
			retu = -3;
		}       
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

int trust_l3_dscp(char * enabledscp,char * enableremap,char * policyindex,struct list * lcontrol)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int EnableDscp = 0,RemapFlag = 0,g_policy_index=0;
	unsigned int op_ret = 0;
	
	if(strncmp("disable",enabledscp,strlen(enabledscp))==0)
		EnableDscp  = 1;
	if(strncmp("enable",enabledscp,strlen(enabledscp))==0)
		EnableDscp  = 2;
    
	
	if(strncmp("enable",enableremap,strlen(enableremap))==0)
		RemapFlag  = 1;
	else if(strncmp("disable",enableremap,strlen(enableremap))==0)
		RemapFlag  = 0;	
	
	dcli_str2ulong((char*)policyindex,&g_policy_index);
    
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_PORT_TRUST_L3_MODE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&EnableDscp,  
                             DBUS_TYPE_UINT32,&RemapFlag,
                             DBUS_TYPE_UINT32,&g_policy_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
            if(QOS_SUCCESS!=op_ret){
                   //vty_out(vty,"%% Fail to config port trust mode layer 3!\n");  
                   ShowAlert(search(lcontrol,"trust_l3_fail"));
            }       
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int trust_l2andl3_mode(char * upable,char * dscpenable,char *remapenbale,char * policyindex,struct list * lcontrol)
	      /*返回0表示成功，返回-1表示失败，返回-2表示device not support trust mode layzer2+layer3，返回-3表示Fail to config port trust mode layzer2+layer3!*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int EnableUp = 0,EnableDscp = 0,RemapFlag = 0,g_policy_index=0;
	unsigned int op_ret = 0;
    int retu = 0;	
	if(strncmp("enable",upable,strlen(upable))==0)
		EnableUp  = 2;
	else if(strncmp("disable",upable,strlen(upable))==0)
		EnableUp  = 1;	

	if(strncmp("enable",dscpenable,strlen(dscpenable))==0)
		EnableDscp  = 2;
	else if(strncmp("disable",dscpenable,strlen(dscpenable))==0)
		EnableDscp  = 1;	
    
	if(strncmp("enable",remapenbale,strlen(remapenbale))==0)
		RemapFlag  = 1;
	else if(strncmp("disable",remapenbale,strlen(remapenbale))==0)
		RemapFlag  = 0;	
    
	
	 dcli_str2ulong((char*)policyindex,&g_policy_index);
    
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_PORT_TRUST_L2_L3_MODE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&EnableUp,
                             DBUS_TYPE_UINT32,&EnableDscp,  
                             DBUS_TYPE_UINT32,&RemapFlag,
                             DBUS_TYPE_UINT32,&g_policy_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//return CMD_FAILURE;
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
		if (ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT == op_ret) {
			//vty_out(vty,"%% device not support trust mode layzer2+layer3!\n"); 
			retu = -2;
		}
		else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
		      // vty_out(vty,"%% Fail to config port trust mode layzer2+layer3!\n");      
		      retu = -3;
		}       
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

int untrust_mode(char * policyindex,struct list * lcontrol)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	
	unsigned int g_policy_index=0;
	unsigned int op_ret;
   		
	g_policy_index = dcli_str2ulong_QOS((char*)policyindex);
  
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_PORT_TRUST_UNTRUST_MODE);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,                   
                             DBUS_TYPE_UINT32,&g_policy_index,
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
            if(QOS_SUCCESS!=op_ret){
                  // vty_out(vty,"%% Fail to config port untrust mode!\n");  
                   ShowAlert(search(lcontrol,"untrust_mode_fail"));
            }       
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int bind_policy_port(char * policyindex,unsigned int index)/*返回0表示成功，返回-2表示Error! no such port，返回-3表示Qos mode is flow, please change mode，返回-4表示Port not support bind policy map，返回-5表示There is no qos mode, please config qos mode，返回-6表示device not support acl group and policy-map binded in the same port*/
	                       /*返回-7表示Port has binded policy-map yet，返回-8表示Policy-map not existed!，返回-9表示device not support l2+l3 mode, please change trust mode，返回-10表示Bind policy-map fail，返回-11表示 Error happend as write to hw*/
{
	DBusMessage *query, *reply;
	DBusError err;

	unsigned int   eth_g_index=0,policyIndex=0;
	unsigned int   op_ret = 0,hw_ret = 0;
    int retu = 0;
	eth_g_index = index;
	dcli_str2ulong((char *)policyindex,&policyIndex);
	//fprintf(stderr, "policyIndex=%d\n",policyIndex);
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_ETHPORTS_OBJPATH,
                                         NPD_DBUS_ETHPORTS_INTERFACE,
                                         NPD_DBUS_ETHPORTS_METHOD_BIND_POLICY_MAP);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,                   
                             DBUS_TYPE_UINT32,&eth_g_index,
                             DBUS_TYPE_UINT32,&policyIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//return CMD_FAILURE;
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&hw_ret,
		DBUS_TYPE_INVALID))
		{	
			//fprintf(stderr, "op_ret=%d\n",op_ret);
			if (op_ret==NPD_DBUS_ERROR_NO_SUCH_PORT)  
			{
				//vty_out(vty,"%% Error! no such port!\n");
				retu = -2;
			}
			else if (op_ret == ACL_RETURN_CODE_ALREADY_FLOW)
			{
				//vty_out(vty, "%% Qos mode is flow, please change mode \n");
				retu = -3;
			}
			else if (ACL_PORT_NOT_SUPPORT_BINDED == op_ret)
			{
				//	vty_out(vty, "%% Port not support bind policy map.\n");
				retu = -4;
			}
			else if (op_ret == ACL_RETURN_CODE_NO_QOS_MODE)
			{
				//vty_out(vty, "%% There is no qos mode, please config qos mode \n");
				retu = -5;
			}
			else if (op_ret == ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT)
			{
				//vty_out(vty, "%% device not support acl group and policy-map binded in the same port \n");
				retu = -6;
			}
			else if (op_ret==QOS_RETURN_CODE_POLICY_MAP_BIND)
			{
				//vty_out(vty,"%% Port has binded policy-map yet\n");
				retu = -7;
			}
			else if (op_ret==QOS_RETURN_CODE_POLICY_NOT_EXISTED)
			{
				//vty_out(vty,"%% Policy-map not existed!\n");
				retu = -8;
			}
			else if (op_ret==ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT)
			{
				//vty_out(vty,"%% device not support l2+l3 mode, please change trust mode !\n");
				retu = -9;
			}
			else if (op_ret!=QOS_RETURN_CODE_SUCCESS)
			{
				//vty_out(vty,"%% Bind policy-map fail!\n");
				retu = -10;
			}
			else if (hw_ret!=QOS_RETURN_CODE_SUCCESS)
			{
				//vty_out(vty,"%% Error happend as write to hw!\n");
				retu = -11;
			}
		} 	
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

int unbind_policy_port(char * policyindex,unsigned int index,struct list * lcontrol)/*返回0表示成功，返回-1表示失败，返回-2表示Error! no such port*/
	                      /*返回-3表示Policy-map index wrong，返回-4表示No policy map information on port，返回-5表示Unbind policy-map fail*/
{
	DBusMessage *query, *reply;
	DBusError err;

	unsigned int   eth_g_index=0,policyIndex=0;
	unsigned int   op_ret = 0,hw_ret = 0;
    int retu = 0;

	eth_g_index = index;
	dcli_str2ulong((char *)policyindex,&policyIndex);

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_ETHPORTS_OBJPATH,
										 NPD_DBUS_ETHPORTS_INTERFACE,
										 NPD_DBUS_ETHPORTS_METHOD_UNBIND_POLICY_MAP);
	
	dbus_error_init(&err);
	dbus_message_append_args(query, 				  
							 DBUS_TYPE_UINT32,&eth_g_index,
							 DBUS_TYPE_UINT32,&policyIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//return CMD_FAILURE;
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&hw_ret,
		DBUS_TYPE_INVALID))
	{	
		if(op_ret==NPD_DBUS_ERROR_NO_SUCH_PORT)
		{
			//return NPD_DBUS_ERROR_NO_SUCH_PORT;
			//vty_out(vty,"%% Error! no such port!\n");
			retu = -2;
		}
		 else if(op_ret==QOS_RETURN_CODE_POLICY_MAP_PORT_WRONG)
		{
			return QOS_POLICY_MAP_PORT_WRONG;
			//vty_out(vty,"%% Policy-map index wrong!\n");
			retu = -3;
		}
		 else if(op_ret==QOS_RETURN_CODE_POLICY_NOT_EXISTED)
		{
			return QOS_POLICY_NOT_EXISTED;
		  	//vty_out(vty,"%% No policy map information on port!\n");
		  	retu = -4;
		}
		 else if((op_ret!=QOS_RETURN_CODE_SUCCESS)||(hw_ret!=QOS_RETURN_CODE_SUCCESS))
		{
			//vty_out(vty,"%% Unbind policy-map fail!\n");
			retu = -5;
		}
	} 
	
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

int config_acl_ingress_policy(char * ruleindex,char * QoS_profile,char * enable,char * source_up,char * source_dscp,char * policerID,struct list * lcontrol)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	unsigned int	ruleIndex=0;
	unsigned int    profileIndex=0,up=10,dscp=100;
	unsigned int    op_ret = 0;
	unsigned int    policer=0,policerId=0;
	unsigned int    remark=0;
	unsigned int    ruleType=0;
	int ret=0,retu = 0;
	dcli_str2ulong((char *)ruleindex,&ruleIndex);
	ruleType = EXTENDED_ACL_RULE; 
	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		//vty_out(vty,"extended rule must less than 512!\n");
		//return CMD_WARNING;
		return -2;
	}
	ruleIndex -=1;
	dcli_str2ulong((char*)QoS_profile,&profileIndex);

	if(strncmp("enable",enable,strlen(enable))==0)
		remark = 0;
	else if(strncmp("disable",enable,strlen(enable))==0)
		remark =1 ;

	if((strncmp("none",source_up,strlen(source_up))==0)&&(strncmp("none",source_dscp,strlen(source_dscp))==0))
	{
		//vty_out(vty,"%% sourceUp and sourceDscp cannot be both none,acl set invalid!\n");
		//return CMD_WARNING;
		return  -3;
	}

	if(strncmp("none",source_up,strlen(source_up))!=0){		
		dcli_str2ulong((char*)source_up,&up);	
	}
	if(strncmp("none",source_dscp,strlen(source_dscp))!=0){		
		dcli_str2ulong((char*)source_dscp,&dscp);	
	}
	//policer
	if(strncmp("policer",policerID,strlen(policerID)) == 0){
			policer = 1;
			ret=dcli_str2ulong((char *)policerID,&policerId);
			if(ret==QOS_RETURN_CODE_ERROR)
			{
				//vty_out(vty,"%% Illegal policer ID!\n");
				//return CMD_WARNING;
				return -4;
			}
		}
		else{
			//vty_out(vty,"%% unknown command\n");
			//return CMD_WARNING;
			return -5;
		}						

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                     	 NPD_DBUS_METHOD_SET_QOS_INGRESS_POLICY_BASE_ON_ACL);		
	dbus_error_init(&err);
	dbus_message_append_args(query, 				  
							 DBUS_TYPE_UINT32,&ruleIndex,
							 DBUS_TYPE_UINT32,&ruleType,
							 DBUS_TYPE_UINT32,&profileIndex,
							 DBUS_TYPE_UINT32,&up,
							 DBUS_TYPE_UINT32,&dscp,
							 DBUS_TYPE_UINT32,&policer,
							 DBUS_TYPE_UINT32,&policerId,
							 DBUS_TYPE_UINT32,&remark,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//return CMD_SUCCESS;
		retu  = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,	
		DBUS_TYPE_INVALID)){	

		 if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
		 	{
				//vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
				retu = -6;
		 	}
			else if (ACL_RETURN_CODE_ALREADY_PORT == op_ret)
			{
				//vty_out(vty, "%% Qos mode is not flow, please change mode \n");
				retu = -7;
			}
			else if (op_ret == ACL_RETURN_CODE_NO_QOS_MODE)
			{
				//vty_out(vty, "%% There is no qos mode, please config qos mode \n");
				retu = -8;
			}
			else if (ACL_RETURN_CODE_HYBRID_FLOW== op_ret)
			{
				//vty_out(vty, "%% Qos mode is not hybrid, qos-profile is 72~127 \n");
				retu = -9;
			}
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
			{
		         	//vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
		         	retu = -10;
			}
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
			{
				//vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl,  but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));
				retu = -11;
			}
			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
			{
				//vty_out(vty,"%% identical fields of packet can not set again\n");	
				retu = -12;
			}
			else if(QOS_RETURN_CODE_PROFILE_NOT_EXISTED ==op_ret)
			{
				//vty_out(vty,"%% QoS profile %d not existed!\n",profileIndex);
				retu = -13;
			}
			else if(QOS_RETURN_CODE_SUCCESS!=op_ret)
			{
				//vty_out(vty,"%% Set QOS ingress-policy based on ACL fail!\n");	
				retu = -14;
			}
	} 		
	else {		
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

int config_acl_egress_policy(char * ruleindex,char * egress_up,char * egress_dscp,char * source_up,char * source_dscp,char * policerID,struct list * lcontrol)
{
	DBusMessage 	*query = NULL, *reply = NULL;
	DBusError		err;
	unsigned int	ruleIndex=0,ruleType=0;
	unsigned int    up=10,dscp=100,egrUp=0,egrDscp=0;
	unsigned int    op_ret;
	//unsigned int    policer=0,policerId=0;
	int retu = 0;

	dcli_str2ulong((char *)ruleindex,&ruleIndex);
	ruleType = EXTENDED_ACL_RULE; 
	if ((ruleType == EXTENDED_ACL_RULE)&&(ruleIndex > MAX_EXT_RULE_NUM)) {
		//vty_out(vty,"extended rule must less than 512!\n");
		//return CMD_WARNING;
		return -2;
	}
	ruleIndex -=1;
	dcli_str2ulong((char*)egress_up,&egrUp);
	dcli_str2ulong((char*)egress_dscp,&egrDscp);

	if((strncmp("none",source_up,strlen(source_up))==0)&&(strncmp("none",source_dscp,strlen(source_dscp))==0))
	{
		//vty_out(vty,"%% sourceUp and sourceDscp cannot be both none,acl set invalid!\n");
		//return CMD_WARNING;
		return -3;
	}
	if(strcmp(source_up,"none")!=0){		
		dcli_str2ulong((char*)source_up,&up);
	}
	if(strcmp(source_dscp,"none")!=0){		
		dcli_str2ulong((char*)source_dscp,&dscp);
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_SET_QOS_EGRESS_POLICY_BASE_ON_ACL);		
	dbus_error_init(&err);
	dbus_message_append_args(query, 				  
							 DBUS_TYPE_UINT32,&ruleIndex,
							 DBUS_TYPE_UINT32,&ruleType,
							 DBUS_TYPE_UINT32,&egrUp,
							 DBUS_TYPE_UINT32,&egrDscp,
							 DBUS_TYPE_UINT32,&up,
							 DBUS_TYPE_UINT32,&dscp,					
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//return CMD_SUCCESS;
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,	
		DBUS_TYPE_INVALID)){			  
			
			if(ACL_RETURN_CODE_GLOBAL_EXISTED == op_ret)
			{
				//vty_out(vty,"%% Access-list %d existed!\n",(ruleIndex+1));
				retu = -4;
			}
			else if (ACL_RETURN_CODE_ALREADY_PORT == op_ret)
			{
				//vty_out(vty, "%% Qos mode is not flow, please change mode \n");
				retu = -5;
			}
			else if (op_ret == ACL_RETURN_CODE_NO_QOS_MODE)
			{
				//vty_out(vty, "%% There is no qos mode, please config qos mode \n");
				retu = -6;
			}
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == STANDARD_ACL_RULE))
			{
	         	//vty_out(vty,"%% because the extended should take up twice spaces than stdard acl,but %d has been set extended!\n",(ruleIndex-511));
	         	retu = -7;
			}
			else if((ACL_RETURN_CODE_EXT_NO_SPACE==op_ret)&&(ruleType == EXTENDED_ACL_RULE))
			{
				//vty_out(vty,"%% because the extended rule should take up twice spaces than standard acl, but %d has been set standard rule! ~~set fail\n",(ruleIndex+513));
				retu = -8;
			}
			else if(ACL_RETURN_CODE_SAME_FIELD == op_ret)
			{
				//vty_out(vty,"%% identical fields of packet can not set again\n");
				retu  =-9;
			}
			else if(QOS_RETURN_CODE_SUCCESS!=op_ret)
			{
				//vty_out(vty,"%% Set QOS egress-policy based on ACL fail!\n");	
				retu = -10;
			}
	} 		
	else {		
		if (dbus_error_is_set(&err)){
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

int show_qos_profile(struct qos_info qos_all[],int * qospronum,struct list * lcontrol)
{
	 DBusMessage *query = NULL, *reply = NULL;
	 DBusError err;
	 DBusMessageIter  iter;
	 DBusMessageIter  iter_array;

	 unsigned int profileIndex=0,TC=0,DP=0,DSCP=0,UP=0;
	 unsigned int ret,count=0,j=0;
	 
	 query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_QOS_OBJPATH,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_SHOW_QOS_PROFILE);
	 dbus_error_init(&err);
	 reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	 dbus_message_unref(query);
	 if (NULL == reply) 
	 {
		 //vty_out(vty,"failed get reply.\n");
		 if (dbus_error_is_set(&err)) 
		 {
			 dbus_error_free(&err);
		 }
		 return CMD_SUCCESS;
	 }
	 dbus_message_iter_init(reply,&iter);

	 dbus_message_iter_get_basic(&iter,&ret);
	
	if(QOS_RETURN_CODE_SUCCESS == ret){
		 dbus_message_iter_next(&iter);  
		 dbus_message_iter_get_basic(&iter,&count);
		 dbus_message_iter_next(&iter); 	 
		 dbus_message_iter_recurse(&iter,&iter_array);
		*qospronum = count;
		
		 for (j = 0; j < count; j++) {
			 DBusMessageIter iter_struct;
			 dbus_message_iter_recurse(&iter_array,&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&profileIndex);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&TC);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&DP);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&UP);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&DSCP);
			 dbus_message_iter_next(&iter_array); 
		 	 qos_all[j].profileindex=profileIndex;
		 	 qos_all[j].dp=DP;
		 	 qos_all[j].up=UP;
		 	 qos_all[j].tc=TC;
		 	 qos_all[j].dscp=DSCP;
		    //vty_out(vty,"===============================================\n");
		    //vty_out(vty,"%-40s: %d\n","QOS Profile",profileIndex);
			//vty_out(vty,"%-40s: %d\n","Traffic Class",TC);
			//vty_out(vty,"%-40s: %s\n","Drop Precedence",drop);
			//vty_out(vty,"%-40s: %d\n","User Priority",UP);
			//vty_out(vty,"%-40s: %d\n","DSCP",DSCP);
			//vty_out(vty,"===============================================\n");
		 }
	}
	else if(QOS_RETURN_CODE_PROFILE_NOT_EXISTED== ret) 
	{
		//ShowAlert(search(lcontrol,"qos_profile_exist"));
		return -1;
	}
	
	dbus_message_unref(reply);
	 return CMD_SUCCESS;
}

#if 0

int show_policy_map(struct policy_map_info policy_map_all[],int * policyNum,struct list * lcontrol)
{
	DBusMessage *query = NULL, *reply = NULL;
	 DBusError err;
	 DBusMessageIter  iter;
	 DBusMessageIter  iter_array;

	 unsigned int ModUpEn=0,ModDscpEn=0,Dp=0,TrustFlag=0,remapDscp=0;
	 unsigned int policyMapIndex=0;
	 unsigned int ret,count=0,j=0;
	 char * trustMem=(char *)malloc(30);
	 memset(trustMem,0,30);
	 char * modiUp=(char *)malloc(20);
	 memset(modiUp,0,20);
	 char * modiDscp=(char *)malloc(20);
	 memset(modiDscp,0,20);
	 char * droppre=(char *)malloc(20);
	 memset(droppre,0,20);
	 char * remaps=(char *)malloc(20);
	 memset(remaps,0,20);

	
	 query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_QOS_OBJPATH,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_SHOW_POLICY_MAP);
	 dbus_error_init(&err);
	 reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	 dbus_message_unref(query);
	 if (NULL == reply) 
	 {
		 if (dbus_error_is_set(&err)) 
		 {
			 dbus_error_free(&err);
		 }
		 return CMD_SUCCESS;
	 }
	 dbus_message_iter_init(reply,&iter);

	 dbus_message_iter_get_basic(&iter,&ret);
	 if(QOS_RETURN_CODE_SUCCESS == ret){
		 dbus_message_iter_next(&iter);  
		 dbus_message_iter_get_basic(&iter,&count);
		 dbus_message_iter_next(&iter); 	 
		 dbus_message_iter_recurse(&iter,&iter_array);
		*policyNum=count;		   
		 for (j = 0; j < count; j++) {
			 DBusMessageIter iter_struct;
			 dbus_message_iter_recurse(&iter_array,&iter_struct);			 
			 dbus_message_iter_get_basic(&iter_struct,&policyMapIndex);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&Dp);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&TrustFlag);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&ModUpEn);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&ModDscpEn);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&remapDscp);
			 dbus_message_iter_next(&iter_array); 
			 
			 switch(TrustFlag){
			 	case 0:strcpy(trustMem,"Untrust");			 	 break;
			 	case 1:strcpy(trustMem,"layer2");		 break;
			 	case 2:strcpy(trustMem,"layer3"); 		 break;
			 	case 3:strcpy(trustMem,"layer2+layer3"); break;			
				default: break; 	 
		 	 }
		 	switch(ModUpEn){
			 	case 0:strcpy(modiUp,"Keep");	     break;
			 	case 1:strcpy(modiUp,"Disable");		 break;
			 	case 2:strcpy(modiUp,"Enable"); 		 break;			 	
				default: break; 	 
		 	 }
			switch(ModDscpEn){
			 	case 0:strcpy(modiDscp,"Keep");	 break;
			 	case 1:strcpy(modiDscp,"Disable");   break;
			 	case 2:strcpy(modiDscp,"Enable"); 	 break;			 	
				default: break; 	 
		 	 }
			switch(Dp){
			 	case 0:strcpy(droppre,"enable");	 break;
			 	case 1:strcpy(droppre,"disable");   break;		 	
				default: break; 	 
		 	 }
			switch(remapDscp){
			 	case 0:strcpy(remaps,"Disable");	 break;
			 	case 1:strcpy(remaps,"Enable");   break;		 	
				default: break; 	 
		 	 }
		 	 strcpy(policy_map_all[j].trustMem,trustMem);
		 	 strcpy(policy_map_all[j].modiUp,modiUp);
		 	 strcpy(policy_map_all[j].modiDscp,modiDscp);
		 	 strcpy(policy_map_all[j].remaps,remaps);
		 	 strcpy(policy_map_all[j].droppre,droppre);
			
		 	 policy_map_all[j].policy_map_index=policyMapIndex;

			 ///////////////////////////////////////////////////
             policy_map_all[j]
			 ///////////////////////////////////////////////////
		 }
	}
	else if(QOS_RETURN_CODE_POLICY_NOT_EXISTED== ret) 
	{}
	
	dbus_message_unref(reply);
	free(trustMem);
	free(modiUp);
	free(modiDscp);
	free(droppre);
	free(remaps);
	return CMD_SUCCESS;
}

#endif

#if 1

int show_policy_map(struct policy_map_info policy_map_all[],int * policyNum,struct list * lcontrol)
{
	 DBusMessage *query = NULL, *reply = NULL;
	 DBusError err;
	 DBusMessageIter  iter;
	 DBusMessageIter  iter_array;

	 unsigned int ModUpEn=0,ModDscpEn=0,Dp=0,TrustFlag=0,remapDscp=0;
	 unsigned int policyMapIndex=0;

	 
	 /////////////add  by kehao ,02/22/2011  11:45//////////////////////
	 unsigned int slot_no = 0,port_no = 0,pmcount = 0;
	 ///////////////////////////////////////////////////////////////
	 unsigned int ret,count=0,j=0;
	 /////////add  by kehao ,02/22/2011  11:45/////////////
     unsigned int k  = 0;
	 ///////////////////////////////////////////

	 
	 
	 char * trustMem=(char *)malloc(30);
	 memset(trustMem,0,30);
	 char * modiUp=(char *)malloc(20);
	 memset(modiUp,0,20);
	 char * modiDscp=(char *)malloc(20);
	 memset(modiDscp,0,20);
	 char * droppre=(char *)malloc(20);
	 memset(droppre,0,20);
	 char * remaps=(char *)malloc(20);
	 memset(remaps,0,20);

	
	 query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_QOS_OBJPATH,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_METHOD_SHOW_POLICY_MAP);
	 dbus_error_init(&err);
	 reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	 dbus_message_unref(query);
	 if (NULL == reply) 
	 {
		 if (dbus_error_is_set(&err)) 
		 {
			 dbus_error_free(&err);
		 }
		 return CMD_SUCCESS;
	 }
	 dbus_message_iter_init(reply,&iter);

	 dbus_message_iter_get_basic(&iter,&ret);
	 if(QOS_RETURN_CODE_SUCCESS == ret){
		 dbus_message_iter_next(&iter);  
		 dbus_message_iter_get_basic(&iter,&count);
		 dbus_message_iter_next(&iter); 	 
		 dbus_message_iter_recurse(&iter,&iter_array);
		*policyNum=count;		   
		 for (j = 0; j < count; j++) {
			 DBusMessageIter iter_struct;
			 
			 
			 ///////// add by kehao  02/22/2011  11:45///////////////////////////////
             DBusMessageIter iter_sub_array;
			 /////////////////////////////////////////////////////////////////////

			 
			 dbus_message_iter_recurse(&iter_array,&iter_struct);			 
			 dbus_message_iter_get_basic(&iter_struct,&policyMapIndex);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&Dp);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&TrustFlag);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&ModUpEn);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&ModDscpEn);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&remapDscp);

			 
			 //dbus_message_iter_next(&iter_array);    // comment by kehao 02/22/2011  11:45

			 
			 /////////////// add by kehao  02/22/2011  11:45    //////////////////////////
			 
             dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&pmcount);
			 dbus_message_iter_next(&iter_struct); 
			 
			 ///////////////////////////////////////////////////////////////////////
			 
			 
			 switch(TrustFlag){
			 	case 0:strcpy(trustMem,"Untrust");			 	 break;
			 	case 1:strcpy(trustMem,"layer2");		 break;
			 	case 2:strcpy(trustMem,"layer3"); 		 break;
			 	case 3:strcpy(trustMem,"layer2+layer3"); break;			
				default: break; 	 
		 	 }
		 	switch(ModUpEn){
			 	case 0:strcpy(modiUp,"Keep");	     break;
			 	case 1:strcpy(modiUp,"Disable");		 break;
			 	case 2:strcpy(modiUp,"Enable"); 		 break;			 	
				default: break; 	 
		 	 }
			switch(ModDscpEn){
			 	case 0:strcpy(modiDscp,"Keep");	 break;
			 	case 1:strcpy(modiDscp,"Disable");   break;
			 	case 2:strcpy(modiDscp,"Enable"); 	 break;			 	
				default: break; 	 
		 	 }
			switch(Dp){
			 	case 0:strcpy(droppre,"enable");	 break;
			 	case 1:strcpy(droppre,"disable");   break;		 	
				default: break; 	 
		 	 }
			switch(remapDscp){
			 	case 0:strcpy(remaps,"Disable");	 break;
			 	case 1:strcpy(remaps,"Enable");   break;		 	
				default: break; 	 
		 	 }

             ///////////////////add by kehao  02/22/2011  11:45 ///////////////////
             
			 dbus_message_iter_recurse(&iter_struct,&iter_sub_array);
			 for (k = 0; k < pmcount; k++){
				  DBusMessageIter iter_sub_struct;
				  dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
							  
				  dbus_message_iter_get_basic(&iter_sub_struct,&slot_no);
				  dbus_message_iter_next(&iter_sub_struct);

				  dbus_message_iter_get_basic(&iter_sub_struct,&port_no);
			         dbus_message_iter_next(&iter_sub_struct);
			         //vty_out(vty,"%-40s: %d/%d\n","binded by port",slot_no, port_no);

			         dbus_message_iter_next(&iter_sub_array);
			  }

			dbus_message_iter_next(&iter_struct); 
			dbus_message_iter_next(&iter_array); 
			
			//vty_out(vty,"===============================================\n");

			/////////////////////////////////////////////////////////////////////////////////
		 	 strcpy(policy_map_all[j].trustMem,trustMem);
		 	 strcpy(policy_map_all[j].modiUp,modiUp);
		 	 strcpy(policy_map_all[j].modiDscp,modiDscp);
		 	 strcpy(policy_map_all[j].remaps,remaps);
		 	 strcpy(policy_map_all[j].droppre,droppre);
			
		 	 policy_map_all[j].policy_map_index=policyMapIndex;

			 /////////// add by kehao  02/22/2011  11:45///////////////
             policy_map_all[j].slot_no = slot_no;
			 policy_map_all[j].port_no = port_no;
			 ///////////////////////////////////////////////////
			 
		 }
	}
	else if(QOS_RETURN_CODE_POLICY_NOT_EXISTED== ret) 
	{}
	
	dbus_message_unref(reply);
	free(trustMem);
	free(modiUp);
	free(modiDscp);
	free(droppre);
	free(remaps);
	return CMD_SUCCESS;
}

#endif


int show_port_qos(char * portNo,struct list * lcontrol)/*返回0表示成功，返回-1表示失败，返回-2表示QOS_RETURN_CODE_POLICY_NOT_EXISTED，返回-3表示NPD_DBUS_ERROR_NO_SUCH_PORT*/
{
	 DBusMessage *query = NULL, *reply = NULL;
	 DBusError err;
	 //DBusMessageIter  iter;
	 //DBusMessageIter  iter_array;
	 unsigned int 	policyIndex=0,op_ret,g_eth_index=0;
	 unsigned int 	slot_no=0,port_no=0;
	 int retu = 0;
	 
     dcli_str2ulong((char *)portNo,&g_eth_index);
	

	 query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,\
	 									 NPD_DBUS_ETHPORTS_OBJPATH,\
                                         NPD_DBUS_ETHPORTS_INTERFACE,\
                                         NPD_DBUS_ETHPORT_METHOD_SHOW_POLICY_MAP);
	 dbus_error_init(&err);
	 dbus_message_append_args(query,
						     DBUS_TYPE_UINT32,&g_eth_index,
							 DBUS_TYPE_INVALID);
	 reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	 dbus_message_unref(query);
	 if (NULL == reply) 
	 {
		 if (dbus_error_is_set(&err)) 
		 {
			 dbus_error_free(&err);
		 }
		 retu = -1;
	 }
	 if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,  &slot_no,
		DBUS_TYPE_UINT32,  &port_no,
		DBUS_TYPE_UINT32,  &policyIndex,
		DBUS_TYPE_INVALID)){	
			if(op_ret ==QOS_RETURN_CODE_POLICY_MAP_BIND){
				retu = 0;
			}
			else if(op_ret==QOS_RETURN_CODE_POLICY_NOT_EXISTED){
				retu = -2;
			}
			else if(op_ret==NPD_DBUS_ERROR_NO_SUCH_PORT){
				retu = -3;
			}
	 }
	else {		
		if (dbus_error_is_set(&err)){	
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

struct mapping_info show_remap_table_byindex(int * mappingNum,int * upnum,int * dscpnum,int * dscpselfnum,struct list * lcontrol)
{
	 DBusMessage *query = NULL, *reply = NULL;
	 DBusError err = { 0 };
	 DBusMessageIter  iter;
	 DBusMessageIter  iter_array;
	 unsigned int	  upCount=0,dscpCount=0,dscpReCount=0,countVal=0;
	 unsigned int	  i=0;
	 unsigned int    l=0,m=0,n=0,g=0;
	 //unsigned int    flag=0;
	 DCLI_QOS_REMAP_SHOW_STC *getIndex[150];
	struct mapping_info map_all;
	map_all.mapping_des=(char *)malloc(50);
	memset(map_all.mapping_des,0,50);
	for(i=0;i<Mapping_num;i++)
    	{
    		map_all.flag[i]=0;
    		map_all.profileindex[i]=0;				
    	}

	 query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,\
										  NPD_DBUS_QOS_OBJPATH,
										  NPD_DBUS_QOS_INTERFACE,
										  NPD_DBUS_ETHPORT_METHOD_SHOW_REMAP_TABLE);
	 dbus_error_init(&err);
	 reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	 dbus_message_unref(query);
	 if (NULL == reply) 
	 {
		 if (dbus_error_is_set(&err)) 
		 {
			 dbus_error_free(&err);
		 }
		 return map_all;
	 }
	 dbus_message_iter_init(reply,&iter);
	 dbus_message_iter_get_basic(&iter,&upCount);
	 dbus_message_iter_next(&iter);  
	 dbus_message_iter_get_basic(&iter,&dscpCount);
	 dbus_message_iter_next(&iter);  
	 dbus_message_iter_get_basic(&iter,&dscpReCount);
	 dbus_message_iter_next(&iter);  
	 dbus_message_iter_get_basic(&iter,&countVal);
	 dbus_message_iter_next(&iter);
		 
	 dbus_message_iter_recurse(&iter,&iter_array);	
	 
	 for (i = 0; i< countVal; i++){
		 DBusMessageIter iter_struct;

		 getIndex[i]=(DCLI_QOS_REMAP_SHOW_STC *)malloc(sizeof(DCLI_QOS_REMAP_SHOW_STC));
		 if(NULL==getIndex[i])
		 {
		 	//vty_out(vty,"malloc fail\n");
		 }		
		 else memset(getIndex[i],0,sizeof(DCLI_QOS_REMAP_SHOW_STC));

		 dbus_message_iter_recurse(&iter_array,&iter_struct);	
		 dbus_message_iter_get_basic(&iter_struct,&(getIndex[i]->flag));
		 dbus_message_iter_next(&iter_struct);
		 dbus_message_iter_get_basic(&iter_struct,&(getIndex[i]->profileIndex));		 
		 dbus_message_iter_next(&iter_array); 
	}
	for(g=0;g<upCount+dscpCount+dscpReCount;g++)
	{
		//if(getIndex[m]->profileIndex==qosindex || g>=upCount+dscpCount)
		{
     		*mappingNum=upCount+dscpCount+dscpReCount;
     		*upnum=upCount;
     		*dscpnum=dscpCount;
     		*dscpselfnum=dscpReCount;
         	 if(upCount>0){
         	 	 //vty_out(vty,"=============================\n");
         		//vty_out(vty,"%s\n","UP->QoSProfile Mapping");
         		strcpy(map_all.mapping_des,"UP->QoSProfile Mapping");
         	 	
         
         		for(l=0;l<upCount;l++){
         			 //vty_out(vty,"%d-->%d\n",getIndex[l]->flag,getIndex[l]->profileIndex);
         			map_all.flag[l]=getIndex[l]->flag;
         			map_all.profileindex[l]=getIndex[l]->profileIndex;
         		}
         	 }	
         	
         	 if(dscpCount>0){
         	 	//vty_out(vty,"=============================\n");
         	 	//vty_out(vty,"%s\n","DSCP->QoSProfile Mapping");
         	 	strcpy(map_all.mapping_des,"DSCP->QoSProfile Mapping");
         	 
         		for(m=0;m<dscpCount;m++){
         			 //vty_out(vty,"%d-->%d\n",getIndex[upCount+m]->flag,getIndex[upCount+m]->profileIndex);
         			map_all.flag[upCount+m]=getIndex[upCount+m]->flag;
         			map_all.profileindex[upCount+m]=getIndex[upCount+m]->profileIndex;
         		}
         	 }
         	
         	 if(dscpReCount>0){
         	 	//vty_out(vty,"=============================\n");
         	 	//vty_out(vty,"%s\n","DSCP->DSCP Remapping");
         	 	strcpy(map_all.mapping_des,"DSCP->DSCP Remapping");
         		
         		for(n=0;n<dscpReCount;n++){
         			 //vty_out(vty,"%d-->%d\n",getIndex[upCount+dscpCount+n]->flag,getIndex[upCount+dscpCount+n]->profileIndex);
         			map_all.flag[upCount+dscpCount+n]=getIndex[upCount+dscpCount+n]->flag;
         			map_all.profileindex[upCount+dscpCount+n]=getIndex[upCount+dscpCount+n]->profileIndex;
         		}
         	 }
         	if((upCount==0)&&(dscpCount==0)&&(dscpReCount==0)){
         		//vty_out(vty,"%% No any remap info\n");
         	}
		}	
		for(l=0; l<upCount; l++){
		free(getIndex[l]);
	}

	}
	dbus_message_unref(reply);
	return map_all;
}

int set_policer(char * policerId)/*返回0表示成功，返回-1表示失败，返回-2表示you should disable the policer firstly，返回-3表示config policer fail*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int policerIndex = 0;
	unsigned int op_ret = 0;

	int retu;

	dcli_str2ulong((char*)policerId,&policerIndex);	
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_CONFIG_POLICER);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policerIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
		   if((QOS_RETURN_CODE_SUCCESS==op_ret)||(QOS_POLICER_DISABLE==op_ret)){
				  
			}
			else if(QOS_POLICER_ENABLE==op_ret)
			{
				//vty_out(vty,"%% you should disable the policer firstly!\n");
                retu = -2;
			}
			else 
			{
				// vty_out(vty,"%% config policer fail \n");	  
               retu = -3;
			}
			 
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}


int set_cir_cbs(char * cirparam,char * cbsparam,char * policyindex,struct list * lcontrol)/*返回0表示成功，返回-1表示失败，*/
	         /*返回-2表示config cir cbs fail，返回-3表示policer cbs is too big，返回-4表示policer cbs is too little*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int policerIndex = 0;
	unsigned int op_ret = 0;
	unsigned int cir = 0,cbs = 0;
	int retu = 0;
	
	dcli_str2ulong((char*)cirparam,&cir);	
	dcli_str2ulong((char*)cbsparam,&cbs); 
	dcli_str2ulong((char*)policyindex,&policerIndex);	

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_CONFIG_CIR_CBS);
	dbus_error_init(&err);
	dbus_message_append_args(query,
		 					 DBUS_TYPE_UINT32,&policerIndex,
							 DBUS_TYPE_UINT32,&cir,
							 DBUS_TYPE_UINT32,&cbs,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
       retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_ERROR == op_ret){
				//vty_out(vty,"%% config cir cbs fail\n");		
				retu  =-2;
			}
			else if(QOS_RETURN_CODE_POLICER_CBS_BIG == op_ret){
				//vty_out(vty,"%% policer cbs is too big !\n");					  				   
				retu = -3;
			}
			else if(QOS_RETURN_CODE_POLICER_CBS_LITTLE == op_ret){
				//vty_out(vty,"%% policer cbs is too little !\n");					  				   
				retu = -4;
			}
					ShowAlert(search(lcontrol,"add_policer_suc"));		  				   
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

int set_out_profile(char * policyindex)/*0:succ,-1:fail,-2:Qos mode is not flow, please change mode,-3:There is no qos mode, please config qos mode*/
	                    /*-4:Qos mode is not hybrid, qos-profile is 72~127,-5:config out profile fail*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int policerIndex = 0;
	unsigned int op_ret = 0;
	int retu= 0;

	policerIndex = dcli_str2ulong_QOS((char*)policyindex);	
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_CONFIG_OUT_PROFILE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policerIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_SUCCESS==op_ret){
			}
			else if (ACL_RETURN_CODE_ALREADY_PORT == op_ret)
				//vty_out(vty, "%% Qos mode is not flow, please change mode \n");
				retu = -2;
			else if (op_ret == ACL_RETURN_CODE_NO_QOS_MODE)
				//vty_out(vty, "%% There is no qos mode, please config qos mode \n");
				retu = -3;
			else if (ACL_RETURN_CODE_HYBRID_FLOW== op_ret)
				//vty_out(vty, "%% Qos mode is not hybrid, qos-profile is 72~127 \n");
				retu = -4;
			else 			 
				 //vty_out(vty,"%% config out profile fail \n");	  
				 retu = -5;
			 
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

int set_out_profile_action(char * actionparam,char * policyindex,struct list * lcontrol)/*0:succ,-1:fail,-2:config out profile action fail*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int policerIndex = 0,action = 0;
	unsigned int op_ret = 0;
	int retu = 0;
	
	dcli_str2ulong((char*)policyindex,&policerIndex);	
	
	if(strcmp(actionparam,"keep")==0)
		action = 0;
	else if(strcmp(actionparam,"drop")==0)
		action=1;
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_OUT_PROFILE_DROP_KEEP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policerIndex,
							 DBUS_TYPE_UINT32,&action,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_SUCCESS!=op_ret){
				//vty_out(vty,"%% config out profile action fail \n");	  
				retu = -2;
			}		 
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

int set_out_profile_remap(char * profile,char * policyindex,struct list * lcontrol)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int policerIndex = 0,profileIndex = 0;
	unsigned int op_ret = 0;
	int retu = 0;
	dcli_str2ulong((char*)policyindex,&profileIndex);
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_OUT_PROFILE_REMAP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policerIndex,
							 DBUS_TYPE_UINT32,&profileIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_PROFILE_NOT_EXISTED == op_ret){
				//vty_out(vty,"%% QoS profile %d not existed!\n",profileIndex);
				retu = -2;
			}
			else if (ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT == op_ret) {
				//vty_out(vty,"%% device not support remap qos profile \n");
				retu = -3;
			}
			else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
				 //vty_out(vty,"%% config out profile action fail \n");
				 retu = -4;
			}			
			
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

int set_color_mode(char * colormode,char * policyindex)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int color = 0,policerIndex=0;
	unsigned int op_ret = 0;
	int retu = 0;

	dcli_str2ulong((char*)policyindex,&policerIndex);	
		
	if(strcmp(colormode,"blind")==0)
		color = 0;
	else if(strcmp(colormode,"aware")==0)
		color = 1;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_POLICER_COLOR);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policerIndex,
							 DBUS_TYPE_UINT32,&color,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_SUCCESS!=op_ret){
				// vty_out(vty,"%% config policer color mode fail \n");	  
				retu = -2;
			}			 
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu;
}

int counter_policer(char * counterindex,char * enable,char * policyindex)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int	 policerIndex=0,coutIndex=0,IsEnable = 0;
	unsigned int	 op_ret = 0;
    int retu = 0;
	
	dcli_str2ulong((char *)counterindex,&coutIndex);		
	dcli_str2ulong((char*)policyindex,&policerIndex);	
	
	if(strncmp("enable",enable,strlen(enable))==0)
		IsEnable = 1;
	else if(strncmp("disable",enable,strlen(enable))==0)
		IsEnable = 0;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_POLICER_COUNTER);
	dbus_error_init(&err);
	dbus_message_append_args(query, 					
							 DBUS_TYPE_UINT32,&policerIndex,
							 DBUS_TYPE_UINT32,&coutIndex,
							 DBUS_TYPE_UINT32,&IsEnable,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_COUNTER_NOT_EXISTED==op_ret){
				//vty_out(vty," %% counter %d not existed!\n",coutIndex); 
				retu = -2;
			}
			else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
				 //vty_out(vty,"%% set counter for policer fail \n");	  
				 retu = -3;
			}			 
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

int set_policer_enable(char * policerId,char * enable)
{
	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int policerIndex=0,IsEnable = 0;
	unsigned int op_ret = 0;
	unsigned long	cir=0,cbs=0;
    int retu = 0;
	
	dcli_str2ulong((char *)policerId,&policerIndex);
	
	if(strncmp("enable",enable,strlen(enable))==0)
		IsEnable = 1;
	else if(strncmp("disable",enable,strlen(enable))==0)
		IsEnable =0;

	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_POLICER_ENABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query,						
							 DBUS_TYPE_UINT32,&policerIndex,
							 DBUS_TYPE_UINT32,&IsEnable,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu  =-1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&cir,
		DBUS_TYPE_UINT32,&cbs,
		DBUS_TYPE_INVALID))
	{				
		    if(QOS_RETURN_CODE_BAD_PTR==op_ret){
				 //vty_out(vty,"%% policer %d not existed!\n",policerIndex);	  
				 retu = -2;
			}
			else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
				//vty_out(vty,"%% fail operate policer \n");
				retu = -3;
			}

	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

int set_strict_mode(char * mode)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int packetsize=0;
	unsigned int op_ret = 0;
    int retu = 0;
	
	if(strcmp(mode,"l1")==0)
		packetsize = 3;
	else if(strcmp(mode,"l2")==0)
		packetsize = 2;
	else if(strcmp(mode,"l3")==0)
		packetsize = 1;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_GLOBAL_PACKET_SIZE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&packetsize,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_SUCCESS!=op_ret){
				 //vty_out(vty,"%% config global policer strict mode and metering packet size fail \n");   
				 retu = -2;
			}			 
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

int set_meter_loose_mode(char * loose_mru)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	//unsigned int mode=0;
	unsigned int mru=0;
	unsigned int op_ret = 0;
	int retu = 0;
	
    dcli_str2ulong((char *)loose_mru,&mru);	

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_GLOBAL_METER_MODE);
	dbus_error_init(&err);
	dbus_message_append_args(query, 					
							 DBUS_TYPE_UINT32,&mru,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_SUCCESS!=op_ret){
				//vty_out(vty,"%% config global policer loose mode and MRU fail \n");   
				retu = -2;
			}			 
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

int set_counter_attr(char * counterindex,char * Inprofile,char * Outprofile)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int     coutIndex=0;
	unsigned int	 inRange=0,outRange=0;
	unsigned int     op_ret = 0;
	int retu = 0;
	
	dcli_str2ulong((char *)counterindex,&coutIndex);
	dcli_str2ulong((char *)Inprofile,&inRange);
	dcli_str2ulong((char *)Outprofile,&outRange);

	if((1>inRange)||(0xFFFFFFFF < inRange)) {
		//vty_out(vty,"%% Bad parameter!\n");
		//return CMD_WARNING;
		return -2;
	}
	if((1>outRange)||(0xFFFFFFFF < outRange)) {
		//vty_out(vty,"%% Bad parameter!\n");
		//return CMD_WARNING;
		return -2;
	}
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_SET_COUNTER);
	dbus_error_init(&err);
	dbus_message_append_args(query,						
							 DBUS_TYPE_UINT32,&coutIndex,
							 DBUS_TYPE_UINT32,&inRange,
							 DBUS_TYPE_UINT32,&outRange,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_SUCCESS!=op_ret){
				 //vty_out(vty,"%% set counter fail \n");	  
				 retu = -3;
			}			 
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu;
}

int show_counter(char * counterindex,struct counter_info * all_info)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int	coutIndex=0;
	unsigned int	 op_ret = 0;
	unsigned long    inprofile=0,outprofile=0;
    int retu = 0;
	
	dcli_str2ulong((char *)counterindex,&coutIndex);
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_GET_COUNTER);
	dbus_error_init(&err);
	dbus_message_append_args(query, 					
							 DBUS_TYPE_UINT32,&coutIndex,	
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&inprofile,
		DBUS_TYPE_UINT32,&outprofile,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_BAD_PTR==op_ret)
			{
				retu = -2;
			}
			else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
				 retu = -3;
			}	
			else{
				all_info->index = coutIndex;
				all_info->inprofile = inprofile;
				all_info->outprofile = outprofile;
			}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

int show_policer(struct policer_info policer_all[],int * num)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;	
	unsigned long    cir=0,cbs=0;
	unsigned int     cmd=0,counterEnable=0,counterSetIndex=0,meterColorMode=0;
	//unsigned int      modifyDscp=0,modifyUp=0;
	unsigned int     policerIndex=0,policerEnable=QOS_POLICER_DISABLE,qosProfile=0;
	unsigned int     ret=QOS_RETURN_CODE_ERROR,j=0,count=0;
	//unsigned char    cmdstr[20]={0},coutE[10]={0},meterC[20]={0},modeC[10]={0},packsizeC[10]={0},mruC[10]={0};
	//unsigned char    policerE[10]={0};
	unsigned int     packsize=3,mru=0,mode=0;
    int retu = 0;
  
	char * cmdstr=(char *)malloc(20);
	memset(cmdstr,0,20);
	char * 	coutE=(char *)malloc(20);
	memset(	coutE,0,20);
	char * modeC=(char *)malloc(20);
	memset(modeC,0,20);
	char * packsizeC=(char *)malloc(20);
	memset(packsizeC,0,20);
	char * mruC=(char *)malloc(20);
	memset(mruC,0,20);
	char * policerE=(char *)malloc(20);
	memset(policerE,0,20);
	
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_SHOW_POLICER);
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu  =-1;
	}
	 dbus_message_iter_init(reply,&iter);

	 dbus_message_iter_get_basic(&iter,&ret);
	
	if(QOS_SUCCESS == ret){
		 dbus_message_iter_next(&iter);  
		 dbus_message_iter_get_basic(&iter,&count);
		 dbus_message_iter_next(&iter); 	 
		 dbus_message_iter_recurse(&iter,&iter_array);
		*num=count;	   
		 for (j = 0; j < count; j++) {
			 DBusMessageIter iter_struct;
			 dbus_message_iter_recurse(&iter_array,&iter_struct);			 
			 dbus_message_iter_get_basic(&iter_struct,&policerIndex);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&cir);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&cbs);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&cmd);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&counterEnable);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&counterSetIndex);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&meterColorMode);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&policerEnable);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&qosProfile);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&mode);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&mru);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&packsize);
			 dbus_message_iter_next(&iter_array); 
			 		
			 switch(cmd){
			 	case 0:strcpy(cmdstr,"Keep");		 break;
			 	case 1:strcpy(cmdstr,"Drop");		 break;
			 	case 2:strcpy(cmdstr,"Remap"); 		 break;
			 		
				default: break; 	 
		 	 }
		 	switch(counterEnable){
			 	case 0:strcpy(coutE,"Disable");	     break;			 	
			 	case 1:strcpy(coutE,"Enable"); 		 break;			 	
				default: break; 	 
		 	 }
			switch(policerEnable){
			 	case 0:strcpy(policerE,"Disable");	     break;			 	
			 	case 1:strcpy(policerE,"Enable"); 		 break;			 	
				default: break; 	 
		 	 }
			switch(packsize){
			 	case 0:			 	
			 	case 1:	strcpy(packsizeC,"layer3");	     break;	
				case 2: strcpy(packsizeC,"layer2");	     break;	
				case 3: strcpy(packsizeC,"layer1");	     break;	
				default: break; 	 
		 	 }
			switch(mode){
			 	case 0:	strcpy(modeC,"Strict");	     break;			 	
			 	case 1:	strcpy(modeC,"Loose");	     break;	
				default: break; 	 
		 	 }
			switch(mru){
			 	case 0:	strcpy(mruC,"MRU 1.5K");	 break;			 	
			 	case 1:	strcpy(mruC,"MRU 2K");	     break;		
				case 2: strcpy(mruC,"MRU 10K");	     break;		
				default: break; 	 
		 	 }
		 	 policer_all[j].policer_index=policerIndex;
		 	 strcpy(policer_all[j].policer_state,policerE);
		 	 policer_all[j].cir=cir;
		 	 policer_all[j].cbs=cbs;
		 	 strcpy(policer_all[j].CounterState,coutE);

			if(counterEnable==1)
			{
				policer_all[j].CounterIndex = counterSetIndex;
			}
			else
			{
				policer_all[j].CounterIndex = 0;
			}
			strcpy(policer_all[j].Out_Profile_Action,cmdstr);
			policer_all[j].Remap_QoSProfile=qosProfile;
			strcpy(policer_all[j].Policer_Mode,modeC);
			if(mode==0)
			{
				strcpy(policer_all[j].Policing_Packet_Size,packsizeC);
			}
			else if(mode==1)
			{
				strcpy(policer_all[j].Policing_Packet_Size,mruC);
			}
		 }
	}
	else if(QOS_RETURN_CODE_POLICER_NOT_EXISTED== ret) 
	{
      retu = -2;
	}
	
	dbus_message_unref(reply);
	free(cmdstr);
	free(coutE);
	free(modeC);
	free(packsizeC);
	free(mruC);
	free(policerE);
	return retu ;
}

int delete_policer(char * policerId)/*0:succ,-1:fail,-2:policer %d not existed,-3:Since policer is in use,can not delete,-4 Delete policer fail*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int policerIndex = 0;
	unsigned int op_ret = 0;
	int retu = 0;
	
	dcli_str2ulong((char*)policerId,&policerIndex);	
	
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_DELETE_POLICER);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policerIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_POLICER_NOT_EXISTED==op_ret)
			{
				retu = -2;		
			}
			else if(QOS_RETURN_CODE_POLICER_USE_IN_ACL==op_ret)
			{
				retu = -3;
			}
			else if(QOS_RETURN_CODE_SUCCESS!=op_ret)
			{
				retu = -4;
			}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

int set_queue_scheduler(char * queuemode)/*0:succ,-1:fail,-2:config queue scheduler fail, device not support,-3:config queue scheduler fail*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int algFlag=0;
	unsigned int op_ret = 0;
    int retu = 0;
	
	if(strncmp("wrr",queuemode,strlen(queuemode))==0)
		algFlag = 1;
	else if(strncmp("sp",queuemode,strlen(queuemode))==0)
		algFlag =0;
	else if(strncmp("hybrid",queuemode,strlen(queuemode))==0)
		algFlag =2;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_QUEQUE_SCH);
	dbus_error_init(&err);
	dbus_message_append_args(query, 					
							 DBUS_TYPE_UINT32,&algFlag,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(op_ret==QOS_RETURN_CODE_SUCCESS){
				if((algFlag==1)||(algFlag==2)){
					/*if(vty->node==CONFIG_NODE){
						vty->node=SHQUEUE_NODE;
						vty->index=(void*)algFlag;
					}*/
				}
			}
			else if (ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT == op_ret) {
				//vty_out(vty,"%% config queue scheduler fail, device not support \n");
				retu  =-2;
			}
			else{
				//vty_out(vty,"%% config queue scheduler fail\n");
				retu  =-3;
			}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

int set_wrr_queue(char * group,char * TCrange,char * Weight,unsigned int  queueindex)/*0:succ,-1:fail,-2:configqueue scheduler wrr group fail, device not support ,-3:config queue scheduler wrr group fail*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int groupFlag=0,tc=0,weight=0,wrrF = 0;
	unsigned int op_ret = 0;
    int retu = 0;
	
	if(strncmp("group1",group,strlen(group))==0)
	{
		groupFlag = 0;
	}
	else if(strncmp("group2",group,strlen(group))==0)
	{
		groupFlag =1;
	}
	
	wrrF = queueindex;
	
	dcli_str2ulong((char *)TCrange,&tc);
    if(strncmp("sp",Weight,strlen(Weight)) == 0) {
		weight = QOS_SCH_GROUP_IS_SP;
	}
	 else {
	 	dcli_str2ulong((char *)Weight,&weight);
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_QUEQUE_WRR_GROUP);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wrrF,
							 DBUS_TYPE_UINT32,&groupFlag,
							 DBUS_TYPE_UINT32,&tc,
							 DBUS_TYPE_UINT32,&weight,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{							
			if (ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT == op_ret) {
				//vty_out(vty,"%% configqueue scheduler wrr group fail, device not support \n");
				retu = -2;
			}
			else if(op_ret != QOS_RETURN_CODE_SUCCESS){
				//vty_out(vty,"%% config queue scheduler wrr group fail\n");
				retu = -3;
			}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu;
}

int show_queue(struct query_info query_all[])
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	unsigned int i = 0,j = 0,k = 0,m = 0,flag = 0;
	unsigned int groupflag=0,weight=0;
	int retu = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_SHOW_QUEUE);
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = -1;
	}
	 dbus_message_iter_init(reply,&iter);
	 dbus_message_iter_get_basic(&iter,&flag);
	
	 if(flag==0){
		for(j=0;j<8;j++){
			query_all[j].QID=j;
			strcpy(query_all[j].Scheduling_group,"sp");
			query_all[j].weight=0;
		}
	 }
	 else if(flag==1){
		for(k=0;k<8;k++){
			query_all[k].QID=k;
			strcpy(query_all[k].Scheduling_group,"group1");
			query_all[k].weight=k+1;
		}
	 }
	 else if(flag==2){	 	
		 dbus_message_iter_next(&iter); 	 
		 dbus_message_iter_recurse(&iter,&iter_array);	
		 for (i = 0; i < 8; i++) {
			 DBusMessageIter iter_struct;
			 dbus_message_iter_recurse(&iter_array,&iter_struct);			 
			 dbus_message_iter_get_basic(&iter_struct,&groupflag);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&weight);
			 dbus_message_iter_next(&iter_array);

			 
			query_all[i].QID=i;
			 if(groupflag==0)
			 {
			 	strcpy(query_all[i].Scheduling_group,"group1");
			 }
			 else if(groupflag==1)
			 {
			 	strcpy(query_all[i].Scheduling_group,"group2");
			 }
			query_all[i].weight=weight;
		 
	     }	 
	 }
	else if(flag==3){
		 dbus_message_iter_next(&iter); 	 
		 dbus_message_iter_recurse(&iter,&iter_array);	
		 for (m = 0; m < 8; m++) {
			 DBusMessageIter iter_struct;
			 dbus_message_iter_recurse(&iter_array,&iter_struct);			 
			 dbus_message_iter_get_basic(&iter_struct,&groupflag);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&weight);
			 dbus_message_iter_next(&iter_array);
				
			 //vty_out(vty,"%-7d",m);			
			 query_all[m].QID=m;
			 if(groupflag==0)
			 {
			 	strcpy(query_all[m].Scheduling_group,"group1");
			 }
				//vty_out(vty,"  %-20s","group1");
			 else if(groupflag==1)
			 {
			 	strcpy(query_all[m].Scheduling_group,"group2");
			 }
				//vty_out(vty,"  %-20s","group2");
			 if(weight == 0){
			 	strcpy(query_all[m].Scheduling_group,"sp");
			 	query_all[m].weight=0;
				  //vty_out(vty,"  %-20s","sp");
				 // vty_out(vty,"  %-12s\n","0");
			  }
			 else
			 {
			 	query_all[m].weight=weight;
			 }
	     }	 
	}
	dbus_message_unref(reply);
	return retu ;
}

int set_traffic_shape(char * maxrateparam,char * burstsize,unsigned int portindex,char * kORm,struct list * lcontrol)  /*portNO要转化*/
	     /*0:succ,-1:fail,-2:error input ,-3:if k mode 1~4096 you can choice ,-4:if m mode 1~1000 you can choice ,-5:config traffic shape for queue %d on port fail*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err ;
	unsigned int algFlag = 0,queueId = 0,burst = 0,kmstate=0;
	unsigned int op_ret = 0,eth_index = 0;
	unsigned int maxrate = 0;
	int ret = 0;
	int retu = 0;
	
	eth_index = portindex;	

	ret = dcli_str2ulong((char *)maxrateparam,&maxrate);
	if(ret == QOS_RETURN_CODE_ERROR)
	{
		return -2;
	}
	
	if(strncmp("k",kORm,strlen(kORm)) == 0) {
		kmstate = 0;
	}	
	else if(strncmp("m",kORm,strlen(kORm)) == 0) {
		kmstate = 1;
	}
	ret=dcli_str2ulong((char *)burstsize,&burst);
	if(ret==QOS_RETURN_CODE_ERROR)
	{
		return -2;
	}

	if((maxrate > 1000000)||( maxrate < 1))
	{
		return -2;

	}
	if((burst>4095)||(burst<1))
	{
		return -2;
	}

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,\
										 NPD_DBUS_ETHPORTS_OBJPATH,\
                                         NPD_DBUS_ETHPORTS_INTERFACE,\
										 NPD_DBUS_METHOD_TRAFFIC_SHAPE);
	dbus_error_init(&err);
	dbus_message_append_args(query, 
						     DBUS_TYPE_UINT32,&eth_index,
							 DBUS_TYPE_UINT32,&algFlag,
							 DBUS_TYPE_UINT32,&queueId,
							 DBUS_TYPE_UINT32,&maxrate,
							 DBUS_TYPE_UINT32,&kmstate,
							 DBUS_TYPE_UINT32,&burst,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
		if(op_ret == ACL_RETURN_CODE_BAD_k){ 	
			//vty_out(vty,"%% if k mode 1~4096 you can choice \n");
			retu = -3;
		}
		else if(op_ret == ACL_RETURN_CODE_BAD_M){		
			//vty_out(vty,"%% if m mode 1~1000 you can choice \n");
			retu = -4;
		}
		else if(op_ret != QOS_RETURN_CODE_SUCCESS){ 	
			//vty_out(vty,"%% config traffic shape for queue %d on port fail\n",queueId);
			retu = -5;
		}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}


int set_traffic_queue_attr(char * queueindex,char * maxrateparam,char * burstsize,unsigned int portindex,char * kORm,struct list * lcontrol)    /*portNO要转化*/
	        /*0:succ,-1:fail,-2:Bad parameter,-3:if k mode 1~4096 you can choice,-4:if m mode 1~1000 you can choice,-5:config traffic shape for queue %d on port fail*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err ;
	unsigned int algFlag = 1,queueId = 0,burst = 0;
	unsigned int op_ret = 0,eth_index = 0;
	unsigned int maxrate = 0, kmstate = 0;
	int ret = 0,retu = 0;
	
	eth_index = portindex;
	
	 dcli_str2ulong((char *)queueindex,&queueId);	
	 
	 ret=dcli_str2ulong((char *)maxrateparam,&maxrate);
	 
	 if(ret==QOS_RETURN_CODE_ERROR)
	 {
		 return -2;
	 }
	 if(strncmp("k",kORm,strlen(kORm)) == 0) {
		kmstate = 0;
	 }	
	else if(strncmp("m",kORm,strlen(kORm)) == 0) {
		kmstate = 1;
	}	
	 ret=dcli_str2ulong((char *)burstsize,&burst);
	 if(ret==QOS_RETURN_CODE_ERROR)
	 {
		 return -2;
	 }
	 if((maxrate>1000000)||(maxrate<1))
	 {
		 return -2;
	 }
	 if((burst>4095)||(burst<1))
	 {
		 return -2;
	 }
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,\
										 NPD_DBUS_ETHPORTS_OBJPATH,\
                                         NPD_DBUS_ETHPORTS_INTERFACE,\
										 NPD_DBUS_METHOD_TRAFFIC_SHAPE);
	dbus_error_init(&err);
	dbus_message_append_args(query, 
						     DBUS_TYPE_UINT32,&eth_index,
							 DBUS_TYPE_UINT32,&algFlag,
							 DBUS_TYPE_UINT32,&queueId,
							 DBUS_TYPE_UINT32,&maxrate,
							 DBUS_TYPE_UINT32,&kmstate,
							 DBUS_TYPE_UINT32,&burst,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(op_ret == ACL_RETURN_CODE_BAD_k){		
				//vty_out(vty,"%% if k mode 1~4096 you can choice \n");
				retu = -3;
			}
			else if(op_ret == ACL_RETURN_CODE_BAD_M){		
				//vty_out(vty,"%% if m mode 1~1000 you can choice \n");
				retu = -4;
			}
			else if(op_ret != QOS_RETURN_CODE_SUCCESS){		
				//vty_out(vty,"%% config traffic shape for queue %d on port fail\n",queueId);
				retu = -5;
			}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}

int show_traffic_shape(struct Shapping_info shapping_all[],unsigned int portindex)    /*portNO要转化*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	unsigned int  j = 0,ret = 0,eth_index = 0;
	unsigned int  burstsize=0,queueburst=0;
	unsigned long maxrate=0,queuerate=0;
	unsigned int  portEnable=0,queueEnable=0;
	//unsigned char strcpp[10];
	char strcpp[10];
	int retu = 0;
	
	eth_index = portindex;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,\
										 NPD_DBUS_ETHPORTS_OBJPATH,\
                                         NPD_DBUS_ETHPORTS_INTERFACE,\
										 NPD_DBUS_METHOD_SHOW_TRAFFIC);
	dbus_error_init(&err);
	dbus_message_append_args(query, 
						     DBUS_TYPE_UINT32,&eth_index,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = -1;
	}
     dbus_message_iter_init(reply,&iter);
	 dbus_message_iter_get_basic(&iter,&ret);
	 
	if(QOS_RETURN_CODE_SUCCESS == ret){
		 dbus_message_iter_next(&iter);  
		 dbus_message_iter_get_basic(&iter,&portEnable);
		 dbus_message_iter_next(&iter); 
		 dbus_message_iter_get_basic(&iter,&burstsize);
		 dbus_message_iter_next(&iter); 
		 dbus_message_iter_get_basic(&iter,&maxrate);
		 dbus_message_iter_next(&iter); 
		 if(1==portEnable)
		 {
		 	strcpy(shapping_all[0].Port_Shaping_status,"Enable");
		 }
		 	//vty_out(vty,"Port Shaping: Enable\n");
		 else if(0==portEnable)
		 {
		 	strcpy(shapping_all[0].Port_Shaping_status,"Disable");
		 	shapping_all[0].port_maxrate=maxrate;
		 	shapping_all[0].port_burstsize=burstsize;
		 }
		 shapping_all[0].port_maxrate=maxrate;
		 shapping_all[0].port_burstsize=burstsize;
		 	//vty_out(vty,"Port Shaping: Disable\n");
		 //vty_out(vty,"\n");
		 //vty_out(vty,"%ld kbps   ",maxrate);
		 //vty_out(vty,"%d*4K burst\n",burstsize);
		 // vty_out(vty,"\n");
		// vty_out(vty,"%-7s  %-20s  %-12s  %-12s\n","QID","status","max-rate(kbps)","burst-size(*4K byte)");
		 //vty_out(vty,"=======  ====================	============ ============ \n");	
			
		 dbus_message_iter_recurse(&iter,&iter_array);
		 
		 for (j = 0; j < 8; j++) {
			 DBusMessageIter iter_struct;
			 dbus_message_iter_recurse(&iter_array,&iter_struct);	
			 dbus_message_iter_get_basic(&iter_struct,&queueEnable);		 
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&queueburst);		 
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&queuerate);		 
			 dbus_message_iter_next(&iter_array); 
			 switch(queueEnable){
				case 0: strcpy(strcpp,"disable"); break;
				case 1: strcpy(strcpp,"enable");  break;
				default : break;
			 }
			 shapping_all[j].QOS_ID=j;
			 strcpy(shapping_all[j].Shaping_status,strcpp);
			 shapping_all[j].maxrate=queuerate;
			 shapping_all[j].burstsize=queueburst;
			// vty_out(vty,"%-7d  ",j);
			// vty_out(vty,"%-20s  ",strcpp);
			// vty_out(vty,"%-12ld  ",queuerate);
			// vty_out(vty,"%-12d\n",queueburst);
		 	}
		}
	else if(QOS_RETURN_CODE_ERROR==ret){
		strcpy(shapping_all[0].Port_Shaping_status,"Disable");
		shapping_all[0].port_maxrate=0;
		shapping_all[0].port_burstsize=0;
		// vty_out(vty,"Port Shaping: Disable\n");
		 //vty_out(vty,"\n");
		 //vty_out(vty,"0 kbps   ");
		 //vty_out(vty,"0 burst\n");
		  //vty_out(vty,"\n");
		// vty_out(vty,"%-7s  %-20s  %-12s  %-12s\n","QID","status","max-rate(kbps)","burst-size(*4K byte)");
		// vty_out(vty,"=======  ====================	============ ============ \n");	
		for (j = 0; j < 8; j++) {
			shapping_all[j].QOS_ID=j;
			strcpy(shapping_all[j].Shaping_status,"Disable");
			shapping_all[j].maxrate=0;
			shapping_all[j].burstsize=0;
			// vty_out(vty,"%-7d  ",j);
			// vty_out(vty,"%-20s  ","disable");
			 //vty_out(vty,"%-12s  ","0");
			 //vty_out(vty,"%-12s\n","0");
		 	}	
	}
	else if(NPD_DBUS_ERROR_NO_SUCH_PORT==ret){
		//vty_out(vty,"%% Port information error!\n");
		retu = -2;
	}		
	dbus_message_unref(reply);
	return retu ;
}

int del_traffic_shape(unsigned int portindex)/*0:succ,-1:fail,-2:Port information error,-3:port has no traffic shape information,-4:Delete traffic shape information on port fail*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int  op_ret = 0,eth_index = 0,algFlag=0,queueId=0;
    int retu = 0;	

	eth_index = portindex;
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,\
										 NPD_DBUS_ETHPORTS_OBJPATH,\
										 NPD_DBUS_ETHPORTS_INTERFACE,\
										 NPD_DBUS_METHOD_DELETE_TRAFFIC);
	dbus_error_init(&err);
	dbus_message_append_args(query, 
							 DBUS_TYPE_UINT32,&eth_index,
							 DBUS_TYPE_UINT32,&algFlag,
							 DBUS_TYPE_UINT32,&queueId,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) 
	{
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID))
		{				
		if(NPD_DBUS_ERROR_NO_SUCH_PORT==op_ret){
				//vty_out(vty,"%% Port information error!\n");
				retu = -2;
		}
		else if(op_ret==QOS_RETURN_CODE_TRAFFIC_NO_INFO){		
				retu = -3;
		}						 
		else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
			retu = -4;
			}
		} 
		else 
		{		
			if (dbus_error_is_set(&err)) 
			{
				dbus_error_free(&err);
			}
		}
		
	dbus_message_unref(reply);
	return retu ;
}


int del_queue_traffic(char * queueIdparam,unsigned int portindex)/*0:succ,-1:fail,-2:Port information error,-3:Queue has no traffic shape information,-4:Delete traffic shape information for queue %d on port fail*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	unsigned int  op_ret = 0,eth_index = 0,algFlag=1,queueId=0;
	int retu  = 0;
	
	eth_index = portindex;
	dcli_str2ulong((char *)queueIdparam,&queueId);

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,\
										 NPD_DBUS_ETHPORTS_OBJPATH,\
										 NPD_DBUS_ETHPORTS_INTERFACE,\
										 NPD_DBUS_METHOD_DELETE_TRAFFIC);
	dbus_error_init(&err);
	dbus_message_append_args(query, 
							 DBUS_TYPE_UINT32,&eth_index,
							 DBUS_TYPE_UINT32,&algFlag,
							 DBUS_TYPE_UINT32,&queueId,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err))
		{
			dbus_error_free(&err);
		}
		retu  =-1;
	}
	if (dbus_message_get_args ( reply, &err,
			DBUS_TYPE_UINT32,&op_ret,
			DBUS_TYPE_INVALID))
		{				
				if(NPD_DBUS_ERROR_NO_SUCH_PORT==op_ret){
					//vty_out(vty,"%% Port information error!\n");
					retu  =-2;
				}
				else if(op_ret==QOS_RETURN_CODE_TRAFFIC_NO_INFO){		
					/*vty_out(vty,"%% Queue has no traffic shape information\n");*/
					retu  =-3;
				}						 
				else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
					//vty_out(vty,"%% Delete traffic shape information for queue %d on port fail!\n",queueId);
					retu = -4;
				}
		} 
		else 
		{		
			if (dbus_error_is_set(&err)) 
			{
				dbus_error_free(&err);
			}
		}
		
	dbus_message_unref(reply);
	return retu ;
}

int config_qos_base_acl_traffic(char * index,char * rule)
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};

	unsigned int ruleIndex = 0,profileIndex = 0;
	unsigned int op_ret = 0;
	int ret = 0,retu = 0;
	char * warning = (char *)malloc(512);
	memset(warning,0,512);

	//configQosMode(aclType);
	
        ret=dcli_str2ulong_EX((char*)index,&ruleIndex);	

	
	if(ret==QOS_RETURN_CODE_ERROR)
	{
		//vty_out(vty,"%% Illegal rule index!\n");
		//ShowAlert(search(lcontrol,"illegalrule"));
		return -1;
	}
	if(ruleIndex > MAX_EXT_RULE_NUM) {
		//vty_out(vty,"%% extended rule must less than 512\n");
		return -1;
	}
	ruleIndex = ruleIndex-1;

	ret= dcli_str2ulong_EX((char*)rule,&profileIndex);	
	if(ret==QOS_RETURN_CODE_ERROR)
	{
		//vty_out(vty,"%% Illegal qos profile index!\n");
		//ShowAlert(search(lcontrol,"illegalqos"));
		return -1;
	}
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_APPEND_QOS_MARK_BASE_ACL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
						     DBUS_TYPE_UINT32,&ruleIndex,
							 DBUS_TYPE_UINT32,&profileIndex,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		//ShowAlert(search(lcontrol,"failgetreplay"));
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//return CMD_FAILURE;
		retu  = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	

			//////////////////////////////////////////
			if(ACL_RETURN_CODE_GLOBAL_NOT_EXISTED==op_ret)
			{
				//vty_out(vty,"%% ACL rule %d not existed!\n",(ruleIndex+1));
				retu = -1;
			}
			else if (ACL_RETURN_CODE_BCM_DEVICE_NOT_SUPPORT== op_ret)
			{
				//vty_out(vty,"%% device not support append qos-profile for change up \n");	
				//vty_out(vty,"so up will not change\n");	
				retu = -1;

			}
			else if (ACL_RETURN_CODE_ALREADY_PORT==op_ret)
			{
				//vty_out(vty,"%% QoS mode is not flow, please change mode \n");
				//retu = -6;
				retu = NPD_DBUS_ERROR_ALREADY_PORT;
			}
			else if (op_ret == ACL_RETURN_CODE_NO_QOS_MODE) {
				//vty_out(vty, "%% There is no qos mode, please config qos mode \n");
				retu = NPD_DBUS_ERROR_NO_QOS_MODE ;
			}	
			else if (ACL_RETURN_CODE_HYBRID_FLOW==op_ret)
			{
				//vty_out(vty,"%% QoS mode is hybrid, qos-profile index is 72~127 \n");	
				retu  = NPD_DBUS_ERROR_HYBRID_FLOW;
			}
			else if (QOS_RETURN_CODE_PROFILE_NOT_EXISTED==op_ret)
			{
				//vty_out(vty,"%% QoS profile %d not existed!\n",profileIndex);
				retu = QOS_PROFILE_NOT_EXISTED;
			}
			else if(ACL_RETURN_CODE_RULE_EXT_ONLY == op_ret){
				//vty_out(vty,"%% Fail to append qos profile,  please use the extended rule \n");
				retu  = ACL_RULE_EXT_ONLY;
			}
			else if(op_ret == ACL_RETURN_CODE_GROUP_RULE_EXISTED){
				//vty_out(vty,"%% Can't append this acl since it is bound to ingress group \n");
				retu = -1;
			}	
			else if(op_ret ==ACL_RETURN_CODE_EGRESS_GROUP_RULE_EXISTED){
				//vty_out(vty,"%% Can't append this acl since it is bound to egress group \n");
				retu = -1;
			}
			else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
				//vty_out(vty,"%% Fail to append qos mark to acl traffic!\n");	
				retu = QOS_FAIL_TRAFFIC;
			}
			//////////////////////////////////////////

	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu;
}

int configQosMode(char * type)/*返回0表示成功，返回-1表示失败，返回-2表示qos mode is already port mode*/
	                          /*返回-3表示qos mode is already flow mode,返回-4表示qos mode is already bybird mode*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = {0};
	int retu = 0;
	unsigned int qosmode=0;
	unsigned int op_ret=0;
	//int ret=0;
	if(strncmp("port",type,strlen(type))==0)
	{
		qosmode = 0;
	}
	else if (strncmp("flow",type,strlen(type))==0)
	{
		qosmode = 1;
	}
	else if (strncmp("hybird",type,strlen(type))==0)
	{
		qosmode = 2;
	}
   	else if (strncmp("default",type,strlen(type))==0)
	{
		qosmode = 3;
	}

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                         NPD_DBUS_QOS_OBJPATH,
                                         NPD_DBUS_QOS_INTERFACE,
                                         NPD_DBUS_METHOD_CONFIG_QOS_MODE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&qosmode,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		//ShowAlert(search(lcontrol,"failgetreplay"));
		if (dbus_error_is_set(&err)) 
		{
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		//return CMD_SUCCESS;
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
            if(ACL_RETURN_CODE_ALREADY_PORT==op_ret)
		     {
	             //vty_out(vty,"%% qos mode is already port mode\n");   
			   //ShowAlert(search(lcontrol,"port"));
			   retu = -2;
	            }
            else if(ACL_RETURN_CODE_ALREADY_FLOW==op_ret){
	                 //vty_out(vty,"%% qos mode is already flow mode\n");   
	                 //ShowAlert(search(lcontrol,"flow"));
	                 retu = -3;
	            }
            else if(ACL_RETURN_CODE_ALREADY_HYBRID==op_ret){
	                 //vty_out(vty,"%% qos mode is already bybird mode\n"); 
			   //ShowAlert(search(lcontrol,"bybird"));	 
			     retu  = -4;
	            }
	} 
	else
	{		
			if (dbus_error_is_set(&err)) 
			{
				//printf("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
			}
			retu  = -1;
	}
	dbus_message_unref(reply);
	return retu;
}

int show_qos_mode(char * mode)
{
	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err = { 0 };
	unsigned int qosmode = 0;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
                                     NPD_DBUS_QOS_OBJPATH,
                                     NPD_DBUS_QOS_INTERFACE,
                                     NPD_DBUS_METHOD_SHOW_QOS_MODE);
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&qosmode,
		DBUS_TYPE_INVALID)) {
		if(1 == qosmode) 
		{
			//vty_out(vty,"Qos-mode is flow\n");
			strcpy(mode,"flow");
		}
		else if(2 == qosmode) 
		{
			//vty_out(vty,"Qos-mode is hybrid\n");
			strcpy(mode,"hybird");
		}
		else if(0 == qosmode) 
		{
			//vty_out(vty,"Qos-mode is port\n");
			strcpy(mode,"port");
		}
		else 
		{
			//vty_out(vty,"There is no qos-mode\n");
			strcpy(mode,"nomode");
		}
	} 
	else {
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
int show_qos_base_acl_traffic()/*返回0表示成功，返回-1表示失败，返回-2表示No QoS profile exists*/
{
	
	 DBusMessage *query = NULL, *reply = NULL;
	 DBusError err;
	 DBusMessageIter  iter;
	 DBusMessageIter  iter_array;
     int retu = 0;
	 unsigned int aclindex=0,profileindex=0;
	 unsigned int ret,count=0,j=0;
	 //unsigned char drop[10]={0};
	 
	 query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,NPD_DBUS_QOS_OBJPATH,NPD_DBUS_QOS_INTERFACE,NPD_DBUS_SHOW_APPEND_QOS_MARK_BASE_ACL);
	 dbus_error_init(&err);
	 reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	 dbus_message_unref(query);
	 if (NULL == reply) 
	 {
		 /*vty_out(vty,"failed get reply.\n");*/
		 if (dbus_error_is_set(&err)) 
		 {
			 dbus_error_free(&err);
		 }
		 //return CMD_SUCCESS;
		 retu = -1;
	 }
	 dbus_message_iter_init(reply,&iter);

	 dbus_message_iter_get_basic(&iter,&ret);

	if(QOS_RETURN_CODE_SUCCESS == ret) {
		 dbus_message_iter_next(&iter);  
		 dbus_message_iter_get_basic(&iter,&count);
		 dbus_message_iter_next(&iter); 	 
		 dbus_message_iter_recurse(&iter,&iter_array);		   
		 for (j = 0; j < count; j++) {
			 DBusMessageIter iter_struct;
			 dbus_message_iter_recurse(&iter_array,&iter_struct);			 
			 dbus_message_iter_get_basic(&iter_struct,&aclindex);
			 dbus_message_iter_next(&iter_struct); 
			 dbus_message_iter_get_basic(&iter_struct,&profileindex);
			 dbus_message_iter_next(&iter_array);
		    //vty_out(vty,"===============================================\n");
		    //vty_out(vty,"%-40s: %d\n","append acl ",aclindex);
			//vty_out(vty,"%-40s: %d\n","Qos profile ",profileindex);
			//vty_out(vty,"===============================================\n");
		 }
	}
	else if(QOS_RETURN_CODE_PROFILE_NOT_EXISTED== ret) 
		//vty_out(vty,"%% Error: No QoS profile exists.\n");
		retu  = -2;
	
	dbus_message_unref(reply);
	 return retu ;
}

int delete_append(char *aclvalue)/*返回0表示成功，返回-1表示失败，返回-2表示Delete acl to qos profile map not exist返回-3表示Can't delete this acl since it is bound to group*/
                                 /*返回-4表示Can't delete this acl since it is bound togroup 返回-5表示Fail to delete acl to qos profile map table*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int aclIndex = 0;
	unsigned int op_ret = 0;
	int retu = 0;
		
	dcli_str2ulong((char*)aclvalue,&aclIndex);

	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_DELETE_ACL_PROFILE_TABLE);
	dbus_error_init(&err);
	dbus_message_append_args(query, 													
							 DBUS_TYPE_UINT32,&aclIndex,					  
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//return CMD_SUCCESS;
		retu  = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{	
			 if(QOS_RETURN_CODE_NO_MAPPED == op_ret){
				 //vty_out(vty,"%% Delete acl to qos profile map not exist \n"); 
				 retu = -2;
			 } 
			 else if(op_ret == ACL_RETURN_CODE_GROUP_RULE_EXISTED){
				//vty_out(vty,"%% Can't delete this acl since it is bound to group\n");
				retu = -3;
			 }	
			else if(op_ret == ACL_RETURN_CODE_EGRESS_GROUP_RULE_EXISTED){
				//vty_out(vty,"%% Can't delete this acl since it is bound togroup \n");
				retu = -4;
			 }
			 else if(QOS_RETURN_CODE_SUCCESS!=op_ret){
				 //vty_out(vty,"%% Fail to delete acl to qos profile map table \n"); 	 
				 retu = -5;
			 }
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}
int config_policer_range(char *spid,char *epid,char *pindex)/*0:succ,-1:fail,-2:config policer fail */
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned int policerIndex= 0, startPid = 0, endPid = 0;
	unsigned int op_ret = 0;
	int retu = 0;
	
	dcli_str2ulong((char*)spid, &startPid);
	dcli_str2ulong((char*)epid, &endPid);
	dcli_str2ulong((char*)pindex, &policerIndex);
			
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_CONFIG_POLICER_RANGE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32, &policerIndex,
							 DBUS_TYPE_UINT32, &startPid,
							 DBUS_TYPE_UINT32, &endPid,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
		if((QOS_RETURN_CODE_SUCCESS==op_ret)||(QOS_POLICER_DISABLE==op_ret)){
		}
		else {		 
			 //vty_out(vty,"%% config policer fail \n");	 
			 retu = -2;
		}
		 
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}
int delete_policer_range(char *spid,char *epid)/*0:succ,-1:fail,-2:policer %d not existed,-3:Since policer is in use,can not delete!,-4:Delete policer fail*/
{
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	//unsigned int policerIndex = 0;
	unsigned int startPid = 0, endPid = 0;
	unsigned int op_ret = 0;
	int retu = 0;
	
	dcli_str2ulong((char*)spid, &startPid);
	dcli_str2ulong((char*)epid, &endPid);	
		
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,
										 NPD_DBUS_QOS_OBJPATH,
										 NPD_DBUS_QOS_INTERFACE,
										 NPD_DBUS_METHOD_DELETE_POLICER_RANGE);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&startPid,
							 DBUS_TYPE_UINT32,&endPid,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		retu = -1;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID))
	{				
			if(QOS_RETURN_CODE_POLICER_NOT_EXISTED==op_ret)
			{
				//vty_out(vty,"%% policer %d not existed!\n",policerIndex);
				retu = -2;
			}				
			else if(QOS_RETURN_CODE_POLICER_USE_IN_ACL==op_ret)
			{
				//vty_out(vty,"%% Since policer is in use,can not delete!\n");
				retu = -3;
			}
			else if(QOS_RETURN_CODE_SUCCESS!=op_ret)
			{
				//vty_out(vty,"%% Delete policer fail!\n");		 
				retu = -4;
			}
	} 
	else 
	{		
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu ;
}



