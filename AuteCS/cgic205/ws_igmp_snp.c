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
* ws_igmp_snp.c
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
/*dcli_igmp_snp.h v1.7*/
/*dcli_igmp_snp.c v1.47 */
/*dcli_common_igmp.h v1.7*/
/*dcli_common_igmp.c v1.27*/
/*author qiandawei*/
/*modify zhouyanmei*/
/*update time 09-11-23 with the new return code*/
     
#ifdef __cplusplus
extern "C"
{
#endif   

#include "ws_igmp_snp.h"
#include "cgic.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dbus/dbus.h>
#include <unistd.h>
#include "ws_ec.h"
#include "ws_returncode.h"


/*version snp 1.33*/
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

int param_first_char_check_new(char* str,unsigned int cmdtip)
{
	//int i;
	int ret = NPD_FAIL;
	char c = 0;
	if(NULL == str){
		return ret;
	}
	c = str[0];
	switch (cmdtip){
		case 0://add/delete
			if('a' == c){ret = 1;}
			else if('d' == c){ret = 0;}
			else {ret = NPD_FAIL;}
			break;
		case 1://untag/tag
			if(c =='t'){ret = 1;}
			else if('u' == c){ret = 0;}
			else {ret = NPD_FAIL;}
			break;
		default:
			break;
	}
	return ret;
	
}


////  dcli_vlan.c  version 1.81  "(add|delete) port SLOT_PORT (tag|untag)"
int add_delete(char * addordel,char * slot_port_no,char * Tagornot,unsigned short vID)
{
	
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	
	unsigned char slot_no = 0,local_port_no = 0;
	unsigned int t_slotno = 0,t_portno = 0;
	
	int 	ret 		= 0;
	unsigned short	vlanId=0;	
	unsigned int 	op_ret = 0;
	
	char  message[100];
	memset(message,0,100);
	
    unsigned char isAdd     = 0;
	unsigned char isTagged	=0;
	

	//unsigned int nodesave = 0;

	if(0 == strncmp(addordel,"add",strlen(addordel))) {
		isAdd = 1;

	}
	else if (0 == strncmp(addordel,"delete",strlen(addordel))) {
		isAdd= 0;

	}
	else {
		
		return CMD_FAILURE;
	}
	

	/*fetch the 2nd param : slotNo/portNo*/

	op_ret = parse_slotno_localport(slot_port_no,&t_slotno,&t_portno);

	if (NPD_FAIL == op_ret)
		{    	
		return IGMP_UNKNOW_PORT;  
		}
	else if (1 == op_ret){
		return IGMP_ERR_PORT; 
	}
	slot_no = (unsigned char)t_slotno;  //
	local_port_no = (unsigned char)t_portno;

	/*not support config port on slot0*/
	if(0  == slot_no)
		{		
		return IGMP_ERR_PORT; 
		}

	ret  = param_first_char_check_new((char *)Tagornot,1);
	if(NPD_FAIL == ret) 
	{		
		return IGMP_ERR_PORT_TAG; 
	}
	else if(1 == ret)
		{		isTagged = 1;	}	
	else if(0 == ret)
		{		isTagged = 0;	}
	
	vlanId = (unsigned short)vID;
	
	query = dbus_message_new_method_call(NPD_DBUS_BUSNAME,	
		NPD_DBUS_VLAN_OBJPATH,					
		NPD_DBUS_VLAN_INTERFACE,						
		NPD_DBUS_VLAN_METHOD_CONFIG_PORT_MEMBER_ADD_DEL);
	
	dbus_error_init(&err);	
	
	dbus_message_append_args(	query,			
		DBUS_TYPE_BYTE,&isAdd,					
		DBUS_TYPE_BYTE,&slot_no,	
		DBUS_TYPE_BYTE,&local_port_no,			
		DBUS_TYPE_BYTE,&isTagged,						
		DBUS_TYPE_UINT16,&vlanId,							 
		DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply)
		{	
	    return CMD_FAILURE;
		if (dbus_error_is_set(&err)) 
			{			
			dbus_error_free(&err);	
			}

		return CMD_SUCCESS;
		
		}

	if (dbus_message_get_args ( reply,
		&err,	
		DBUS_TYPE_UINT32,&op_ret,		
		DBUS_TYPE_INVALID)) 
		{			

            if (ETHPORT_RETURN_CODE_NO_SUCH_PORT == op_ret) 
			{
				//vty_out(vty,"%% Bad parameter: Bad slot/port number.\n");
				return IGMP_ERR_PORT;
			}/*either slot or local port No err,return NPD_DBUS_ERROR_NO_SUCH_PORT */
			else if (VLAN_RETURN_CODE_ERR_NONE == op_ret )
			{
				/*vty_out(vty,"Ethernet Port %d-%d %s success.\n",slot_no,local_port_no,isAdd?"Add":"Delete");*/
				return COMMON_FAIL;
			} 
			else if (VLAN_RETURN_CODE_BADPARAM == op_ret) 
			{
				//vty_out(vty,"%% Error occurs in Parse eth-port Index or devNO.& logicPortNo.\n");
				return IGMP_BADPARAM;
			}
			else if (VLAN_RETURN_CODE_VLAN_NOT_EXISTS == op_ret)
			{
				//vty_out(vty,"%% Error: Checkout-vlan to be configured NOT exists on SW.\n");
				return IGMP_NOTEXISTS;
			}
			else if (VLAN_RETURN_CODE_PORT_EXISTS == op_ret)
			{
				//vty_out(vty,"%% Bad parameter: port Already member of the vlan.\n");
				 return IGMP_PORT_EXISTS;
			}
			else if (VLAN_RETURN_CODE_PORT_NOTEXISTS == op_ret)
			{
				//vty_out(vty,"%% Bad parameter: port NOT member of the vlan.\n");
				return IGMP_PORT_NOTEXISTS;
			}
			else if (VLAN_RETURN_CODE_PORT_MBRSHIP_CONFLICT == op_ret) 
			{
				//vty_out(vty,"%% Bad parameter: port Already untagged member of other active vlan.\n");
				return IGMP_PORT_NOTMEMBER;
				
			}			
			else if (VLAN_RETURN_CODE_PORT_PROMIS_PORT_CANNOT_ADD2_VLAN == op_ret) 
			{
				//vty_out(vty,"%% Promiscous port %d/%d can't be added to any vlan!\n",slotno,portno);
				return COMMON_FAIL;
			}
			else if (VLAN_RETURN_CODE_PORT_PROMISCUOUS_MODE_ADD2_L3INTF == op_ret)
			{
				//vty_out(vty,"%% Bad parameter: promiscuous mode port can't add to l3 interface!\n");
				return CMD_FAILURE;
			}
			else if (VLAN_RETURN_CODE_PORT_TAG_CONFLICT == op_ret) 
			{
				//vty_out(vty,"%% Bad parameter: port Tag-Mode can NOT match!\n");
				return IGMP_TAG_CONFLICT;
			}
			else if (VLAN_RETURN_CODE_PORT_SUBINTF_EXISTS == op_ret)
			{
				//vty_out(vty,"%% Can't delete tag port with sub interface!\n");
				return COMMON_FAIL;
			}
			else if (VLAN_RETURN_CODE_PORT_DEL_PROMIS_PORT_TO_DFLT_VLAN_INTF == op_ret)
			{
				//vty_out(vty,"%% Promiscuous mode port can't delete!\n");
				 return IGMP_INTERFACE;
			}
			else if (VLAN_RETURN_CODE_PORT_TRUNK_MBR == op_ret) 
			{
				//vty_out(vty,"%% Bad parameter: port is member of a active trunk!\n");
				return IGMP_NOT_TRUNK;
				
			}
			else if (VLAN_RETURN_CODE_ARP_STATIC_CONFLICT == op_ret) 
			{
				//vty_out(vty,"%% Bad parameter: port has attribute of static arp!\n");
				return IGMP_HAS_ARP;
			}
			else if(PVLAN_RETURN_CODE_THIS_PORT_HAVE_PVE == op_ret)
			{
				//vty_out(vty,"%% Bad parameter: port is pvlan port!please delete pvlan first!\n")
				return IGMP_PVLAN;
			}
			else if (VLAN_RETURN_CODE_L3_INTF == op_ret) 
			{
				if(isAdd) 
				{
					//vty_out(vty,"%% Port adds to L3 interface vlan %d.\n",vlanid);
					return IGMP_L3;
				}
				else 
				{
					//vty_out(vty,"%% L3 interface vlan member can NOT delete here.\n");
					return IGMP_NOT_DELETE;
				}
			}
			else if (VLAN_RETURN_CODE_PORT_L3_INTF == op_ret) 
			{
				if(isAdd) 
				{
					//vty_out(vty,"%% Port can NOT add to vlan as port is L3 interface.\n");
					return IGMP_NOT_ADD;
				}
				else 
				{
					//vty_out(vty,"%% Port can NOT delete from vlan as port is L3 interface.\n");
					return IGMP_NOT_DEL_L3;
				}
			}
			else if (VLAN_RETURN_CODE_ERR_HW == op_ret) 
			{
				//vty_out(vty,"%% Error occurs in Config on HW.\n");	
				return IGMP_HW;
			}
			else if (ETHPORT_RETURN_CODE_UNSUPPORT == op_ret)
			{
				//vty_out(vty,"%% This operation is unsupported!\n");
				return COMMON_FAIL;
			}
			else if (VLAN_RETURN_CODE_ERR_NONE != op_ret)
			{
				//vty_out(vty,"%% Unknown Error! return %d \n",op_ret);
				return COMMON_FAIL;
			}		
			/////////
	}
	else
		{
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}	
    dbus_message_unref(reply);
	
	if(VLAN_RETURN_CODE_ERR_NONE != op_ret)
		return COMMON_ERROR;
	else 
		return COMMON_SUCCESS;

}


/**************************************
*show_igmpsnp_vlan_member_info
version snp 1.38
**************************************/
int show_igmpsnp_vlan_member_info
(
	unsigned int product_id,
	PORT_MEMBER_BMP untagBmp,
	PORT_MEMBER_BMP tagBmp

)
{
	unsigned int i,count = 0;
	unsigned int slot = 0,port = 0;
	
	unsigned int tmpVal[2];
	memset(tmpVal,0,sizeof(tmpVal));


	for (i=0;i<64;i++){
		if(PRODUCT_ID_AX7K == product_id) {
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
				//vty_out(vty,"\n%-9s"," ");
			}
			//vty_out(vty,"%s%d/%d(u)",(count%4) ? ",":"",slot,port);
			count++;
		}
	}
	slot = 0;
	port = 0;
	memset(tmpVal,0,sizeof(tmpVal));
	for (i=0;i<64;i++){
		if(PRODUCT_ID_AX7K == product_id) {
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
		if((tagBmp.portMbr[i/32]) & tmpVal[i/32]) {				
			if(count && (0 == count % 4)) {
				//vty_out(vty,"\n%-9s"," ");
			}
			//vty_out(vty,"%s%d/%d(t)",(count%4) ? ",":"",slot,port);
			count++;
		}
	}

	return 0;
}

int show_igmp_snp_timer
(
	unsigned int vlanlife,
	unsigned int grouplife,
	unsigned int robust,
	unsigned int queryinterval,
	unsigned int respinterval,	
	unsigned int hostime
)
{
	//vty_out(vty,"\n");
	//vty_out(vty,"========  =========  ==============  =============  ======  ========\r\n");
	//vty_out(vty,"VLANLIFE  GROUPLIFE  QUERY-INTERVAL  RESP-INTERVAL  ROBUST  HOSTLIFE\n");
	//vty_out(vty,"%-8d  %-9d  %-14d  %-13d  %-6d  %-8d",vlanlife,grouplife, queryinterval,respinterval,robust,rmxtinterval,hostime);
	//vty_out(vty,"\n========  =========  ==============  =============  ======  ========\r\n");
					//vty_out(vty,"Designated Root\t\t\t:  ");

	fprintf(cgiOut,"<tr>"\
					"<td id=td1>VLAN lifetime:</td>"\
					"<td id=td2>%8d</td>"\
				"</tr>",vlanlife);
	fprintf(cgiOut,"<tr>"\
				    "<td id=td1>Group lifetime:</td>"\
				    "<td id=td2>%d</td>"\
			    "</tr>",grouplife);
	fprintf(cgiOut,"<tr>"\
				    "<td id=td1>Query interval:</td>"\
				    "<td id=td2>%d</td>"\
			    "</tr>",queryinterval);	
	fprintf(cgiOut,"<tr>"\
				    "<td id=td1>RESP interval:</td>"\
				    "<td id=td2>%d</td>"\
			    "</tr>",respinterval);	
	fprintf(cgiOut,"<tr>"\
				    "<td id=td1>Robust:</td>"\
				    "<td id=td2>%d</td>"\
			    "</tr>",robust);
	fprintf(cgiOut,"<tr>"\
				    "<td id=td1>Host lifetime:</td>"\
				    "<td id=td2>%d</td>"\
			    "</tr>",hostime);	
	return 0;
}



/**********************************
 *	igmp snooping enable/disable 
 *	complete ONLY by igmp_snp Protocol
 *
 *  CMD Node : config Node
 snp version 1.33 
**********************************/
//igmp_snooping enable or disable
//return:-1 operate fail,0 operate succ,1 already enable,2 not enable
int igmp_snooping_able(char *able)	
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned char status=0,isEnable = 0;
	int ret=0;
	
	
	if(strncmp(able,"enable",1)==0){
		isEnable = 1;
	}
	else if (strncmp(able,"disable",1)==0){
		isEnable = 0;
	}
	else{
		//vty_out(vty,"bad command parameter!\n");
		return CMD_FAILURE;
	}
	ret = dcli_igmp_snp_check_status(&status);
	if(IGMPSNP_RETURN_CODE_OK == ret)
	{
	 
		if(1 == isEnable){
			if(1 == status){
				//vty_out(vty,"% Bad parameter:IGMP Snoop already Enabled!\n");
				return 1;
			}
		}
		else if(0 == isEnable){
			if(0 == status){
				//vty_out(vty,"% Bad parameter:IGMP Snoop has NOT Enabled!\n");
				return 2;
			}
		}
		}
		/*
	if(isEnable == status) {
			vty_out(vty, "%% Error igmp snooping already %s!\n", isEnable ? "enabled":"disabled");
			return CMD_WARNING;

	}
	*/
	/*
	vty_out(vty,"%s IGMP Snooping Protocol!\r\n",isEnable?"Enable":"Disable");
	vty_out(vty,"igmp dbus interface: %s\n",IGMP_DBUS_INTERFACE);
	*/
	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,   \
										IGMP_DBUS_OBJPATH,    \
										IGMP_DBUS_INTERFACE,   \
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_ENABLE);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_BYTE,&isEnable,
							DBUS_TYPE_INVALID);


	reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	if (dbus_message_get_args ( reply, &err,
									DBUS_TYPE_UINT32,&ret,
									DBUS_TYPE_INVALID)) {
	if(IGMPSNP_RETURN_CODE_OK != ret)
			return CMD_FAILURE;
	} 
	else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
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
version snp 1.33
*************************************************/
//配置igmp snooping时间参数
//输入:type 1代表配置VLAN_LIFETIME，2代表配置GROUP_LIFETIME，3代表配置ROBUST_VARIABLE，4代表配置QUERY_INTERVAL
//return:0 succ,-1 fail, 1 not start gloabl igmp
int config_igmp_snooping(int type, char *time)
{

	DBusMessage *query, *reply;
	DBusError err;

	unsigned int timer_type;
	unsigned int timeout = 0;
	int ret;


	timer_type = type;

	ret = parse_timeout((char *)time,&timeout);
	if (IGMPSNP_RETURN_CODE_ERROR == ret) {
    	//vty_out(vty,"Unknow portno format.\n");
		return CMD_FAILURE;
	}
	/*
	vty_out(vty,"timer type:%d\n",timer_type);
	vty_out(vty,"config timeout value :%d\n",timeout);
	*/
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


	reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}

	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_INVALID)) {
		
		if (IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL == ret){
		  //	vty_out(vty,"%% Error:igmp snooping not enabled!\n");
			return 1;
			
		}
		else if (IGMPSNP_RETURN_CODE_OUT_RANGE == ret) {
			//vty_out(vty,"%% Error:parameter value %d invalid!\n",timeout);
			return -1;
			
		}
		else if (IGMPSNP_RETURN_CODE_SAME_VALUE == ret) {
			//vty_out(vty,"%% Error:value %d is same as the original, or has configured!\n", timeout);
			return -1;
			
		}
		

	} 
	else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
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
//显示igmp snooping的时间
//return: -2 not start global igmp
int show_igmp_snp_time_interval()
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int vlanlife =0,grouplife =0,robust =0,query_interval =0,response_interval =0,hosttime =0;
	unsigned int ret=0;

	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,    \
										IGMP_DBUS_OBJPATH,    \
										IGMP_DBUS_INTERFACE,    \
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_SHOW_TIMER);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
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
			show_igmp_snp_timer(vlanlife,\
								grouplife,\
								robust,\
								query_interval,\
								response_interval,							
								hosttime);
		}/*error code ...*/
		else if (IGMPSNP_RETURN_CODE_ENABLE_GBL == ret){
			//vty_out(vty,"% Bad Param:IGMP Snooping NOT enabled global.\n");
			return -2;
		}
		else if (IGMPSNP_RETURN_CODE_GET_VLANLIFE_E == ret){
			//vty_out(vty,"% Error occures on vlanlife time.\n");
			return 1;
		}
		else if (IGMPSNP_RETURN_CODE_GET_GROUPLIFE_E == ret){
			//vty_out(vty,"% Error occures on grouplife time.\n");
			return 2;
		}
		else if (IGMPSNP_RETURN_CODE_GET_HOSTLIFE_E == ret){
			//vty_out(vty,"% Error occures on hostlife time.\n");
			return 3;
		}
		//else if (IGMPSNP_RETURN_CODE_GET_ROBUST_E == ret){
			//vty_out(vty,"% Error occures on robust variable.\n");
		//	return 4;
		//}
		else if (IGMPSNP_RETURN_CODE_GET_QUREY_INTERVAL_E == ret){
			//vty_out(vty,"% Error occures on query interval.\n");
			return 5;
		}
		else if (IGMPSNP_RETURN_CODE_GET_RESP_INTERVAL_E == ret){
			//vty_out(vty,"% Error occures on response interval.\n");
			return 6;
		}
	} 
	else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

// snp version 1.33 09-03-05
int show_igmp_snp_time_interval_new(igmp_timer *test)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int vlanlife =0,grouplife =0,robust =0,query_interval =0,response_interval =0,hosttime =0;
	unsigned int ret=0;

	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,    \
										IGMP_DBUS_OBJPATH,    \
										IGMP_DBUS_INTERFACE,    \
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_SHOW_TIMER);
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
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
			
            test->vlanlife=vlanlife;
			test->grouplife=grouplife;
			test->robust=robust;
			test->queryinterval=query_interval;
			test->respinterval=response_interval;
			test->hostime=hosttime;
			
		}/*error code ...*/
		else if (IGMPSNP_RETURN_CODE_ENABLE_GBL == ret){
			//vty_out(vty,"% Bad Param:IGMP Snooping NOT enabled global.\n");
			return -2;
		}
		else if (IGMPSNP_RETURN_CODE_GET_VLANLIFE_E == ret){
			//vty_out(vty,"% Error occures on vlanlife time.\n");
			return 1;
		}
		else if (IGMPSNP_RETURN_CODE_GET_GROUPLIFE_E == ret){
			//vty_out(vty,"% Error occures on grouplife time.\n");
			return 2;
		}
		else if (IGMPSNP_RETURN_CODE_GET_HOSTLIFE_E == ret){
			//vty_out(vty,"% Error occures on hostlife time.\n");
			return 3;
		}
		//else if (IGMPSNP_RETURN_CODE_GET_ROBUST_E == ret){
			//vty_out(vty,"% Error occures on robust variable.\n");
		//	return 4;
		//}
		else if (IGMPSNP_RETURN_CODE_GET_QUREY_INTERVAL_E == ret){
			//vty_out(vty,"% Error occures on query interval.\n");
			return 5;
		}
		else if (IGMPSNP_RETURN_CODE_GET_RESP_INTERVAL_E == ret){
			//vty_out(vty,"% Error occures on response interval.\n");
			return 6;
		}
	} 
	else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/******************************************
 *	show mcgroup count
 *
 *	CMD: vlan Node
 *
 ****************************************/
//return: 1 not start global igmp
int show_igmp_snp_group_count()
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short vlanId;
	unsigned int groupcount = 0,ret;
	query = dbus_message_new_method_call(
										IGMP_DBUS_BUSNAME,   \
										IGMP_DBUS_OBJPATH,    \
										IGMP_DBUS_INTERFACE,   \
										IGMP_SNP_DBUS_METHOD_IGMP_SNP_TOTAL_GROUP_COUNT_SHOW);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_UINT16,&vlanId,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}

	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_UINT32,&groupcount,
								DBUS_TYPE_INVALID)) {
		if(IGMPSNP_RETURN_CODE_OK == ret) {
			if(0 == groupcount){
				//vty_out(vty,"No group exist in this vlan.");
				fprintf(cgiOut,"<tr>"\
								"<td id=td1>MCGroup count:</td>"\
								"<td id=td2>No group exist in this vlan.</td>");

			}
			//vty_out(vty,"\n=======	=============\r\n");
			//vty_out(vty,"VLAN_ID	MCGROUP_COUNT\r\n");
			//vty_out(vty,"%-7d	%d\r\n",vlanId,groupcount);
			//vty_out(vty,"=======	===============\r\n");
			else
			fprintf(cgiOut,"<tr>"\
							"<td id=td1>MCGroup count:</td>"\
							"<td id=td2>%d</td>"\
						"</tr>",groupcount);
		}
		else if (IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL == ret){
			//vty_out(vty,"% Bad Param:IGMP Snooping NOT enabled global.\n");
			return 1;
		}
	} 
	else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
	
}


/***************************************
*	show igmp vlan count
*
*
*	CMD :config Node
snp version 1.33
**************************************/

//return: 1 not start global igmp

int iShow_igmp_vlan_count(int *vlan_count)
{

	DBusMessage *query, *reply;
	DBusError err;
	//unsigned short vlanId;
	unsigned int vlancount = 0,ret;


	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,			\
										NPD_DBUS_VLAN_OBJPATH,		\
										NPD_DBUS_VLAN_INTERFACE,		\
										NPD_DBUS_VLAN_METHOD_IGMP_SNP_VLAN_COUNT);

	dbus_error_init(&err);
	/*No params*/
	
	reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}

	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_UINT32,&vlancount,
								DBUS_TYPE_INVALID)) {
		if(IGMPSNP_RETURN_CODE_OK == ret) {
			//vty_out(vty,"\n====================\r\n");
			//vty_out(vty,"IGMP_VLAN COUNT:	%d.\n",vlancount);
			//vty_out(vty,"====================\r\n");
			*vlan_count = vlancount;
		}
		else if (IGMPSNP_RETURN_CODE_ENABLE_GBL == ret){
			//vty_out(vty,"% Bad Param:IGMP Snooping NOT enabled global.\n");
			return 1;
		}

	} 
	else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
	
}
/* 	dcli_vlan_igmp_snp_status_get
 *  by slot and local_port get global port index
 */
int dcli_vlan_igmp_snp_status_get(unsigned short vid,unsigned char* stats)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned char status = 0;
	unsigned int  op_ret = IGMPSNP_RETURN_CODE_OK;
	/*DCLI_DEBUG(("Enter :dcli_igmp_snp_check_status.\n"));*/
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_VLAN_OBJPATH,		\
								NPD_DBUS_VLAN_INTERFACE,	\
								NPD_DBUS_VLAN_METHOD_CHECK_VLAN_IGMP_SNP_STATUS);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							DBUS_TYPE_UINT16,&vid,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}

	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_BYTE,&status,
								DBUS_TYPE_INVALID)) 
	{
		if (IGMPSNP_RETURN_CODE_OK == op_ret ) {
			//DCLI_DEBUG(("success\n"));
			*stats = status;
		}
		else 
			op_ret = IGMPSNP_RETURN_CODE_ERROR;
	} 
	else {
		//printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	dbus_message_unref(reply);
	return op_ret;
}

/******************************************
 * show mcgroup vlan router port
 * CMD :show mcgroup router_port
 *
 * CMD node: VLAN node
 *snp version 1.33  未完(show running 没有用到，故不添加)
 *****************************************/
//return 3 :No Route port Config. return 2:Bad port index get form IGMP SNP. return 1:not start global igmp .return 4:vlan %d NOT enable IGMP Snooping
int show_igmp_snp_mcgroup_router_port
(
    int vid,
    struct igmp_port *head,
	int *num
)
{
	DBusMessage *query, *reply;
	DBusError err;
	
	char *vlanName;
	unsigned char status = 0;	
	unsigned int ret = 0;
	//unsigned int untagBmp = 0, tagBmp = 0;
	unsigned int product_id = 0; 
	unsigned int vlanStat = 0;
	unsigned short vlanId = 0;
	PORT_MEMBER_BMP routeBmp;
    PORT_MEMBER_BMP untagBmp,tagBmp;
	//unsigned int promisPortBmp = 0;
	unsigned int promisPortBmp[2] = {0};


    //int i;
	int ret_value;
	/*unsigned int	eth_g_index =0;
	unsigned char	slot_no =0,local_port_no =0;*/
	
	memset(&routeBmp,0,sizeof(PORT_MEMBER_BMP));
	memset(&tagBmp,0,sizeof(PORT_MEMBER_BMP));
	memset(&untagBmp,0,sizeof(PORT_MEMBER_BMP));

	
	vlanId = (unsigned short)vid;
	dcli_vlan_igmp_snp_status_get(vlanId,&status);
	if(0 == status){
		//vty_out(vty,"% Bad param: vlan %d NOT enable IGMP Snooping.\n");
		return 4;
	}

		query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH,	\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_IGMP_SNP_SHOW_ROUTE_PORT_V1);

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vlanId,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
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

////////////////////////////////////////// 新函数
		if (IGMPSNP_RETURN_CODE_OK == ret)
		{
		  // if (0 != untagBmp || 0 != tagBmp)
			if (0 != untagBmp.portMbr[0]||0 != untagBmp.portMbr[1]|| 0 != tagBmp.portMbr[0]|| 0 != tagBmp.portMbr[1])
			{
				
				ret_value=show_vlan_slot_port(product_id,untagBmp,tagBmp,head,num);
				//ret_value=show_vlan_slot_port(product_id,untagBmp,tagBmp,head,num);


			}
			else
			{
				//vty_out(vty, "  %-40s", "No RoutePort member.");
				return 3;
			}
		}
		//else if ((IGMPSNP_RETURN_CODE_NO_SUCH_VLAN) == ret)
		//{ 
			//vty_out(vty, "%% Error: vlan id illegal.\n");  //vlan 非法
		//	return  5;
		//}
		else if(IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST == ret)
		{
			//vty_out(vty, "%% Error: vlan %d not exist.\n", vlanId);  // vlan不存在
			return 6;
		}
////////////////////////////////////////////

	else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	
	dbus_message_unref(reply);
	
//if((ret_value==0)&&(igmp_num>0))
//  Free_igmp_head(&igmp_head);

	return CMD_SUCCESS;
}



/**************************************
*show_vlan_member_slot_port
*Usage: show vlan membership with
*		in slot&port number.
**************************************/
int show_vlan_slot_port
(
	unsigned int product_id,
	PORT_MEMBER_BMP untagBmp,
	PORT_MEMBER_BMP tagBmp,
	struct igmp_port *head,
	int *num
)
{
	unsigned int i,tmpVal = 0,count = 0;
	unsigned int slot = 0,port = 0;

	struct igmp_port *q,*tail;	
	
	head->next = NULL;
	    tail=head;
		*num=64;

	for (i=0;i<64;i++){

		      //分配空间
        q=(struct igmp_port *)malloc(sizeof(struct igmp_port));	
		if( NULL == q )
		{
			return 0;
		}
		memset( q, 0, sizeof(struct igmp_port) );
		if(PRODUCT_ID_AX7K == product_id) {
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

		tmpVal = (1<<((i>31) ? (i-32) : i));
		if(((i>31) ? untagBmp.portMbr[1] : untagBmp.portMbr[0]) & tmpVal) {	
		
			if(count && (0 == count % 4)) {
				//vty_out(vty,"\n%-32s"," ");
			}

//vty_out(vty,"%s%d/%d(u)",(count%4) ? ",":"",slot,port);

//vty_out(vty,"%s%d/%d(u%s)",(count%4) ? ",":"",slot,port,((promisPortBmp & tmpVal)?"*":""));

            q->un_slot=slot;
			q->un_port=port;

			
			
				
			count++;

			q->next=NULL;
			tail->next=q;
			tail=q;
		}
	}
	slot = 0;
	port = 0;
	tmpVal = 0;
	for (i=0;i<64;i++){
		
			      //分配空间
        q=(struct igmp_port *)malloc(sizeof(struct igmp_port));
		if( NULL == q )
		{
			return 0;
		}
		memset( q, 0, sizeof(struct igmp_port) );
		if(PRODUCT_ID_AX7K == product_id) {
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

		tmpVal = (1<<((i>31) ? (i-32) : i));
		if(((i>31) ? tagBmp.portMbr[1] : tagBmp.portMbr[0]) & tmpVal) {				
		
			if(count && (0 == count % 4)) {
				//vty_out(vty,"\n%-32s"," ");
			}
			//vty_out(vty,"%s%d/%d(t)",(count%4) ? ",":"",slot,port);

            q->slot=slot;
			q->port=port;
						
			count++;

			q->next=NULL;
			tail->next=q;
			tail=q;
		}
	}

	return 0;
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
 DEFUN(config_igmp_snp_npd_vlan_cmd_func,
	config_igmp_snp_npd_vlan_cmd,
	"igmp_snooping vlan (enable|disable)",			
	"IGMP Snooping protocol\n"
	"IGMP Snooping Protocal configuration on vlan\n"
	"Enable vlan to Support IGMP Snooping\n"
	"Disable vlan from Support IGMP Snooping\n"
	"vlan entity in system\n"
												
)
*******************************************/

int config_igmp_snp_npd_vlan_ccgi(int vid,char *able)
{
	/*
	DBusMessage *query, *reply;
	DBusError err;
	*/
	unsigned short	vlanId = 0;
	unsigned char	enDis = FALSE;
	int ret;


	/*get first param*/
	if(!strncmp(able,"enable",IGMP_STR_CMP_LEN-3)) {
		enDis= TRUE;
	}
	else if(!strncmp(able,"disable",IGMP_STR_CMP_LEN-3)) {
		enDis= FALSE;
	}
	else {
		//vty_out(vty,"command parameters error\n");
		return CMD_FAILURE;
	}

	vlanId = (unsigned short)vid;
	//vty_out(vty,"enDis = %d\r\n",enDis);
	//vty_out(vty,"vlan ID = %d.\r\n",vlanId);

	ret = dcli_enable_disable_igmp_one_vlan(vlanId,enDis);
	return ret;

}

/*************************************************
 *	enable|disable port of specific vlan to(from) 
 *	IGMP snooping;
 *	config hw by npd first,and then 
 *	pass param to igmp snooping Protocol
 *
 *	CMD : VLAN node
 DEFUN(config_igmp_snp_npd_port_cmd_func,
	config_igmp_snp_npd_port_cmd,
	"igmp_snooping port (enable|disable) SLOT_PORT",
	"IGMP snooping Protcol \n"
	"IGMP Snooping Protocal configuration on vlan port member\n"
	"Enable port of vlan to support IGMP snooping\n"
	"Disable port of vlan from support IGMP snooping\n"
	"Vlan port member,slot/port number:such as 1/1,4/6\n"
)
snp version 1.33  页面中处理返回值
*************************************************/
int config_igmp_snp_npd_port(int vid,char *able, char *port)
{
	/*
	DBusMessage *query, *reply;
	DBusError err;
	*/
	unsigned char	enDis = FALSE;
	unsigned short	vlanId;
	unsigned char	slot_no = 0,port_no = 0;
	//unsigned int	eth_g_index = 0;	
	unsigned int	ret;

	/*get first param*/
	if(!strncmp(able,"enable",IGMP_STR_CMP_LEN-3)) {
		enDis= TRUE;
	}
	else if(!strncmp(able,"disable",IGMP_STR_CMP_LEN-3)) {
		enDis= FALSE;
	}
	else {
		//vty_out(vty,"command parameters error\n");
		return CMD_FAILURE;
	}
	//vty_out(vty,"enDis = %d\n",enDis);
	/*get 2nd param : slot port number*/
	ret = parse_slotport_no((unsigned char*)port,&slot_no,&port_no);
	if (NPD_FAIL == ret) {
    	//vty_out(vty,"Unknow port number format.\n");
		return CMD_FAILURE;
	}	

	vlanId = (unsigned short)vid;
	/*
	vty_out(vty,"%s IGMP Snooping on vlan %d,slot %d,port %d.\n",\
					(enDis==1)?"enable":"disable",vlanId,slot_no,port_no);
	*/
	ret = dcli_enable_disable_igmp_one_port(vlanId,slot_no,port_no,enDis);
	
	if(CMD_SUCCESS != ret){
		//vty_out(vty,"execute command failed\n");
		return ret;
	}
	
	return CMD_SUCCESS;
}
/*********************************************
*	add|delete mcroute port slot/port
*
*
*	CMD : VLAN mode
DEFUN(igmp_snp_mcroute_port_add_del_cmd_func,
	igmp_snp_mcroute_port_add_del_cmd,
	"igmp_snooping mcroute_port (add|delete) SLOT_PORT",
	"IGMP snooping Protcol \n"
	"Multicast route port for IGMP Snooping \n"
	"Add mcroute-port to IGMP Snooping Protocol \n"
	"Delete mcroute-port from IGMP Snooping Protocol \n"
	"Slot and port number: 1/1,4/3 .. \n"
)
snp version 1.33 
**********************************************/
//输入参数:cmd 1 add,0 delete
//return :3 port not start igmp,4 vlan already route port, 5 vlan not has route port ,6 is not route port
int igmp_snp_mcroute_port_add_del(int vid,int cmd,char *port)
{
	/*
	DBusMessage *query, *reply;
	DBusError err;
	*/
	unsigned char slot_no = 0,local_port_no = 0;
	unsigned char	enDis = FALSE;
	unsigned short vlanId = 0;
	int ret;

	/*fetch the 1st param : add ? delete*/
	if(1 == cmd){
		enDis = TRUE;
	}
	else if(0 == cmd){
		enDis = FALSE;
	}
	
	/*get 2nd param : slot port number*/
	ret = parse_slotport_no((unsigned char*)port,&slot_no,&local_port_no);
	if (NPD_FAIL == ret) {
    	//vty_out(vty,"Unknow port number format.\n");
		return CMD_FAILURE;
	}	

	vlanId = (unsigned short)vid;
	/*
	vty_out(vty,"%s mcrouter port :",(enDis==1)?"enable":"disable");
	vty_out(vty," vlan %d,slot %d,port %d",vlanId,slot_no,local_port_no);
	vty_out(vty," to(from) IGMP snooping.\n");
	*/
	ret = dcli_enable_disable_igmp_mcrouter_port(vlanId,slot_no,local_port_no,enDis);
	if(IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST == ret){
		//vty_out(vty,"% Bad param: port is NOT member of the vlan.\n");
	}
	else if(IGMPSNP_RETURN_CODE_PORT_NOT_EXIST == ret){
		//vty_out(vty,"%% Error: port is not member of the vlan!\n");
	}

	else if (IGMPSNP_RETURN_CODE_NOTENABLE_PORT == ret){
		//vty_out(vty,"% Bad param: port not enabled IGMP Snooping.\n");
		return 3;
	}
	else if(IGMPSNP_RETURN_CODE_ROUTE_PORT_NOTEXIST == ret){
		//vty_out(vty,"% Bad param: vlan has exist router port.\n");
		return 4;
	}
	else if(IGMPSNP_RETURN_CODE_NOTROUTE_PORT == ret){
		//vty_out(vty,"% Bad param: vlan has NOT exist router port.\n");
		return 5;
	}	
	else if(IGMPSNP_RETURN_CODE_NOTROUTE_PORT == ret){
		//vty_out(vty,"%% Error: port not configured as router port!\n");
		return CMD_FAILURE;
	}	
	else if (IGMPSNP_RETURN_CODE_PORT_TRUNK_MBR == ret){
		//vty_out(vty,"%% Error: port is trunk member!\n");
		return CMD_FAILURE;
	}
	else if (IGMPSNP_RETURN_CODE_PORT_NOT_EXIST == ret){
		//vty_out(vty,"%% Error: no such port!\n");
		return CMD_FAILURE;
	}
	else if (IGMPSNP_RETURN_CODE_NO_SUCH_PORT == ret){
		//vty_out(vty,"%% Error: illegal slot or port!\n");
		return CMD_FAILURE;
	}
	else if (1 == ret){
		//vty_out(vty,"%% Error: dbus error!\n");
		return CMD_FAILURE;
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
 DEFUN(show_vlan_igmp_snp_mcgroup_list_cmd_func,
	show_vlan_igmp_snp_mcgroup_list_cmd,
	"show mcgroup list",			
	SHOW_STR
	"Multicast group on Layer 2\n"
	"All L2 mcgroup in this vlan\n"
)
*******************************************/
//return: -1 fail ,-2 has not multicast group
int show_vlan_igmp_snp_mcgroup_list(int vid,struct igmp_vlan *igmp_head,int *igmp_num)
{
	//DBusMessage *query, *reply;
	//DBusError err;
	unsigned short	vlanId = 0;
	int ret;

	vlanId = (unsigned short)vid;
	
   //失败，未对其进行返回错误类别处理
	if (4095 == vlanId)
	{
		//vty_out(vty, "%% Error: Reserved vlan for Layer3 interface of EthPort!\n");
		
		return CMD_FAILURE;
	}
	if (0 == vlanId || vlanId > 4095)
	{
		//vty_out(vty, "%% Error: vlan id is out of valid range <1-4094>!\n");
		
		return CMD_FAILURE;
	}

	//ret = parse_vlan_no((char*)argv[0], &vlanId);
	//if (NPD_FAIL == ret)
	//{
    ///	vty_out(vty, "%% Error: vlan id illegal!\n");
	//	return CMD_WARNING;
	//}

	
	//ret = dcli_show_igmp_snp_mcgroup_list(vlanId);
	ret = dcli_show_igmp_snp_mcgroup_list(vlanId,igmp_head,igmp_num);

	if(NPD_GROUP_NOTEXIST == ret) {
		//vty_out(vty,"Error: l2mc entry NOT exist.\n");
		return -2;
	}
	else if (NPD_VLAN_ERR_HW == ret) {
		//vty_out(vty,"Error occurs in Read vidx Entry in HW.\n");
		return CMD_FAILURE;
	}

	return CMD_SUCCESS;

}
/******************************************
 *	show  one special mcgroup's member in this vlan
 *
 *	CMD:show mcgroup <1-4095> //mcgroup entry 4095 uses specially
 *	mcgroup<4095> has all port in system
 *
 *	1).read mcgroup member bitmap from hw  
 *	2).get group MacAddr
 *	CMD :CONFIG node
 DEFUN(show_igmp_snp_mcgroup_one_cmd_func,
	show_igmp_snp_mcgroup_one_cmd,
	"show mcgroup <1-4095>",			
	SHOW_STR
	"Multicast group on Layer 2\n"
	"Layer2 mcgroup entry index:valid range[1-4095]\n"
)
*******************************************/
/*
int show_igmp_snp_mcgroup_one(char *vlanid)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short	vidx = 0,vlanId = 0;
	int ret;

	vlanId = (unsigned short )vty->index;
	//get vIdx
	//printf("before parse_vlan_no %s\n",argv[0]);
	ret = parse_vlan_no((char*)vlanid,&vidx);
	//vty_out(vty,"To show l2mcgroup entry :vidx = %d.\r\n",vidx);
	dcli_show_igmp_snp_one_mcgroup(vlanId,vidx);
	return CMD_SUCCESS;

}
*/


/**************************************
*show_group_member_slot_port
common version 1.23  09-03-05
**************************************/
int show_group_member_slot_port
(
    unsigned int product_id,
	PORT_MEMBER_BMP mbrBmp

)
{
	unsigned int i,count = 0;
	unsigned int slot = 0,port = 0;
    unsigned int tmpVal[2];
	
	memset(tmpVal,0,sizeof(tmpVal));

	for (i=0;i<64;i++){
				if(PRODUCT_ID_AX7K == product_id) {
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
		if((mbrBmp.portMbr[i/32]) & tmpVal[i/32]) {				
	
		if(count && (0 == count % 4)) {
				//vty_out(vty,"\n%-32s"," ");
			}
			//vty_out(vty,"%s%d/%d",(count%4) ? ",":"",slot,port);
			count++;

			//vty_out(vty,"%d/%d,",slot,port);
			fprintf(cgiOut, "<td style=font-size:12px align=center>%d/%d </td>",slot,port);
		}
		memset(tmpVal,0,sizeof(tmpVal));

	}
	//return NPD_SUCCESS;
	return 0;
}
/*****************************************
 *
 *
 *
 *
 ****************************************/
unsigned int ltochararry
(
	unsigned int Num,
	unsigned char *c0,
	unsigned char *c1,
	unsigned char *c2,
	unsigned char *c3
)
{
	unsigned char cstr0 = 0,cstr1= 0,cstr2 = 0,cstr3 = 0;
	cstr0= ((unsigned char *)&Num)[0]; 
	cstr1= ((unsigned char *)&Num)[1]; 
	cstr2= ((unsigned char *)&Num)[2];
	cstr3= ((unsigned char *)&Num)[3];

	*c0 = cstr0;
	*c1 = cstr1;
	*c2 = cstr2;
	*c3 = cstr3;
	return 0;
}
/*
 *  by slot and local_port get global port index
 */
int dcli_igmp_snp_check_status(unsigned char* stats)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned char status = 0;
	unsigned int  op_ret = 0;
	/*DCLI_DEBUG(("Enter :dcli_igmp_snp_check_status.\n"));*/
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_VLAN_OBJPATH,		\
								NPD_DBUS_VLAN_INTERFACE,		\
								NPD_DBUS_VLAN_METHOD_CHECK_IGMP_SNP_STATUS);
	
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return IGMPSNP_RETURN_CODE_OK;
	}

	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_BYTE,&status,
								DBUS_TYPE_INVALID)) 
	{
		if (IGMPSNP_RETURN_CODE_OK == op_ret ) {
			//DCLI_DEBUG(("success\n"));
			
			*stats = status;
		}
		else
		{
			op_ret = IGMPSNP_RETURN_CODE_ERROR;
		}
	} 
	else {
		//printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	dbus_message_unref(reply);
	return op_ret;
}
 

//common version 1.23  09-03-05
int dcli_igmp_snp_vlan_portmbr_status_check
(  
    unsigned short vlanId,
	unsigned int slotno,
	unsigned int portno,
	unsigned int* stats
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int status = 0,op_ret = 0;
	/*DCLI_DEBUG(("Enter :dcli_igmp_snp_check_status.\n"));*/
	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_VLAN_OBJPATH,		\
								NPD_DBUS_VLAN_INTERFACE,		\
								NPD_DBUS_VLAN_METHOD_CHECK_IGMP_SNP_VLANMBR_STATUS);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT16,&vlanId,
							DBUS_TYPE_UINT32,&slotno,
							DBUS_TYPE_UINT32,&portno,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return IGMPSNP_RETURN_CODE_OK;

	}

	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_UINT32,&status,
								DBUS_TYPE_INVALID)) 
	{
		if (IGMPSNP_RETURN_CODE_OK == op_ret ) {
			//DCLI_DEBUG(("success\n"));
			*stats = status;
		}
		else
			op_ret = IGMPSNP_RETURN_CODE_ERROR;

	} 
	else {
		//printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return op_ret;
}



int dcli_enable_disable_igmp_one_vlan
(
	unsigned short vlanId,
	unsigned int enable
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned short vid = vlanId;
	unsigned int isEnable = enable;
	int op_ret =0;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_VLAN_OBJPATH,		\
								NPD_DBUS_VLAN_INTERFACE,		\
								NPD_DBUS_VLAN_METHOD_CONFIG_IGMP_SNP_VLAN);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vid,
							 DBUS_TYPE_UINT32,&isEnable,
							 DBUS_TYPE_INVALID);
	//vty_out(vty,"get params: vid = %d\r\n",vid);
	//vty_out(vty,"get params: isEnable = %d.\r\n",isEnable);
	reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}

	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_INVALID)) {
	if (IGMPSNP_RETURN_CODE_OK == op_ret ) {
			//DCLI_DEBUG(("success\n"));
			return CMD_SUCCESS;
		}
	else if (IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL == op_ret){
			//vty_out(vty,"IGMP Snooping NOT Enabled Global.\n");
			return 4;
		}
	else if (IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST == op_ret){
			//vty_out(vty,"% Bad perameter : vlan %d NOT exists.\n",vid);
			return 3;
		}
	else if (IGMPSNP_RETURN_CODE_NOTENABLE_VLAN == op_ret){
			//vty_out(vty,"% Bad perameter : vlan %d NOT Enabled.\n",vid);
			return 5;
		}
	else if (IGMPSNP_RETURN_CODE_HASENABLE_VLAN == op_ret){
			//vty_out(vty,"% Bad perameter : vlan %d Already Enabled.\n",vid);
			return 6;
		}

		dbus_message_unref(reply);
		//return op_ret ;
	} 
	else {
		//printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
	}
	return CMD_FAILURE;
}
/*
 *  by slot and local_port get global port index
 */
int dcli_enable_disable_igmp_one_port
(
	unsigned short vlanId,
	unsigned char slot_no,
	unsigned char port_no,
	unsigned char enable
)
{
	DBusMessage *query, *reply;
	DBusError err;
	//DCLI_DEBUG(("Enter::: dcli_enable_disable_igmp_one_port...\n"));
	//DCLI_DEBUG(("vid %d,slot_no %d,port_no %d,enable %d.\n",vlanId,slot_no,port_no,enable));
	unsigned char isEnable = enable;
	int op_ret;

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,	\
								NPD_DBUS_VLAN_OBJPATH,	\
								NPD_DBUS_VLAN_INTERFACE,	\
								NPD_DBUS_VLAN_METHOD_CONFIG_IGMP_SNP_ETHPORT);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vlanId,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
							 DBUS_TYPE_BYTE,&isEnable,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}

	if (dbus_message_get_args( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_INVALID)) {
	if (IGMPSNP_RETURN_CODE_OK == op_ret ) {
				//DCLI_DEBUG(("success\n"));
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else{
		if(IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL == op_ret){
					//vty_out(vty,"% Error: IGMP Snooping NOT Enabled Global!\n");
					return 4;
				}
		else if(IGMPSNP_RETURN_CODE_NO_SUCH_PORT == op_ret){
					//vty_out(vty,"% Bad Parameter :Bad Slot/Port Number!\n");
					return 9;
				}
		else if(IGMPSNP_RETURN_CODE_NOTENABLE_PORT == op_ret){
					//vty_out(vty,"% Bad parameter :Port %d/%d NOT Enable IGMP Snooping.\n",slot_no,port_no);
					return 7;
				}
		else if(IGMPSNP_RETURN_CODE_HASENABLE_PORT == op_ret){
					//vty_out(vty,"% Bad parameter :Port %d/%d Already Enable IGMP Snooping.\n",slot_no,port_no);
					return 8;
				}
		else if(IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST == op_ret){
					//vty_out(vty,"% Bad Parameter :vlan %d Not exist.\n",vlanId);
					return 15;
				}
		//else if(IGMPSNP_RETURN_CODE_NOT_SUPPORT_IGMP_SNP == op_ret){
		//			//vty_out(vty,"% Bad Parameter :vlan %d Not support IGMP snooping.\n",vlanId);
		//			return 70;
		//		}
		else if (IGMPSNP_RETURN_CODE_PORT_NOT_EXIST == op_ret){
					//vty_out(vty,"% Bad Parameter :Illegal Slot/Port Number!\n");
					return CMD_FAILURE;
				}
		else if (IGMPSNP_RETURN_CODE_ERROR_HW == op_ret){
					//vty_out(vty,"% Bad parameter :Port %d/%d NOT vlan %d's member.\n",slot_no,port_no,vlanId);
					return 18;
				}
		else if (IGMPSNP_RETURN_CODE_PORT_TRUNK_MBR == op_ret){
					//vty_out(vty,"% Error :IGMP config error occurs on HW.\n");
					return 16;
				}
	    else if(IGMPSNP_RETURN_CODE_ERROR == op_ret){
			//vty_out(vty,"%% Error:get devport error!\n");
			return 20;
		}

				dbus_message_unref(reply);
				return op_ret;
			}
	} else {
		//printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}

///snp version 1.33 
int dcli_enable_disable_igmp_mcrouter_port
(
	unsigned short vlanId,
	unsigned char slot_no,
	unsigned char port_no,
	unsigned char enDis
)
{
	DBusMessage *query, *reply;
	DBusError err;
	//printf("Enter::: dcli_enable_disable_igmp_mcrouter_port...\n");
	//printf("vid %d,slot_no %d,port_no %d,enable %d.\n",vlanId,slot_no,port_no,enDis);
	unsigned short vid = vlanId;
	unsigned char isEnable = enDis,slotNo =slot_no,localPortNo = port_no;

	//unsigned int port_index;
	int ret;
	
	
	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,			\
										NPD_DBUS_VLAN_OBJPATH,		\
										NPD_DBUS_VLAN_INTERFACE,	\
										NPD_DBUS_VLAN_METHOD_IGMP_SNP_ADD_DEL_MCROUTE_PORT);
	dbus_error_init(&err);

    dbus_message_append_args(query,
							DBUS_TYPE_UINT16,&vid,
							DBUS_TYPE_BYTE,	&slotNo,
							DBUS_TYPE_BYTE,	&localPortNo,
							DBUS_TYPE_BYTE,	&isEnable,
							DBUS_TYPE_INVALID);


	reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		//printf("failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}

	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&ret,
								DBUS_TYPE_INVALID)) {
	    if (NPD_DBUS_SUCCESS == ret ) {
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
		else{
			dbus_message_unref(reply);
			return ret;
		}

	} 
	else {
		//printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}
	dbus_message_unref(reply);
	return ret;
}

void Free_igmp_head(struct igmp_vlan *head)
{
	struct igmp_vlan *f1,*f2;
	if (NULL != head)
	{
		f1=head->next;	
		if (NULL != f1)
		{
			f2=f1->next;
			while(f2!=NULL)
			{
				free(f1);
				f1=f2;
				f2=f2->next;
			}
			free(f1);
		}
	}
}



//////释放链表


void Free_igmp_port(struct igmp_port *head)
{
    struct igmp_port *f1,*f2;
	if (NULL != head)
	{
		f1=head->next;	
		if (NULL != f1)
		{
			f2=f1->next;
			while(f2!=NULL)
			{
			  free(f1);
			  f1=f2;
			  f2=f2->next;
			}
			free(f1);
		}
	}
}





//////释放链表


void Free_igmp_sum(struct igmp_sum *head)
{
	struct igmp_sum *f1,*f2;
	if (NULL != head)
	{
		f1=head->next;	
		if (NULL != f1)
		{
			f2=f1->next;
			while(f2!=NULL)
			{
				free(f1);
				f1=f2;
				f2=f2->next;
			}
			free(f1);
		}
	}
}

////////////新函数存储
int dcli_show_igmp_snp_mcgroup_list
(
	unsigned short vlanId,
	struct igmp_vlan *head,
	int *num
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int 	group_Cont = 0,groupIp=0;
	unsigned short  vidx= 0;
	unsigned int	j,product_id = 0;
	unsigned int 	ret = 0;
	unsigned char ip0=0,ip1=0,ip2=0,ip3=0;
	
    PORT_MEMBER_BMP mbrBmp;
	memset(&mbrBmp,0,sizeof(PORT_MEMBER_BMP));

	struct igmp_vlan *q,*tail;	

	
	query = dbus_message_new_method_call(\
										NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,	\
										NPD_DBUS_VLAN_METHOD_SHOW_VLAN_MCGROUP_LIST_PORT_MEMBERS_V1 );

	

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vlanId,
							 DBUS_TYPE_INVALID);
	//DCLI_DEBUG(("get params: vid = %d\r\n",vlanId));
	
	reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}

	dbus_message_iter_init(reply,&iter);

	dbus_message_iter_get_basic(&iter,&ret);
	if(IGMPSNP_RETURN_CODE_OK == ret){
		
		////////////////  新增加
		
		head->next = NULL;
	    tail=head;
		*num=group_Cont;
		
		////////////////
		if(group_Cont > 0) {
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&group_Cont);
		//printf("get basic return value actgroupCont =%d\n",group_Cont);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&product_id);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		//vty_out(vty,"========================================================================\n");
		//vty_out(vty,"%-7s  %-4s  %-15s  %-40s\n","VLAN ID","VIDX","GROUP_IP","PORT MEMBER LIST");
		//vty_out(vty,"=======  ====  ===============  ========================================\n");

		for (j = 0; j < group_Cont; j++) {

            //分配空间
           q=(struct igmp_vlan *)malloc(sizeof(struct igmp_vlan));			
		
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&vidx);
			dbus_message_iter_next(&iter_struct);

			q->vidx=vidx;   //short type

			dbus_message_iter_get_basic(&iter_struct,&groupIp);
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(mbrBmp.portMbr[0]));
			dbus_message_iter_next(&iter_struct);


			dbus_message_iter_get_basic(&iter_struct,&(mbrBmp.portMbr[1]));
			dbus_message_iter_next(&iter_array);


			ltochararry(groupIp,&ip0,&ip1,&ip2,&ip3);

			q->ip0=ip0;
			q->ip1=ip1;
			q->ip2=ip2;
			q->ip3=ip3;
			
			//vty_out(vty,"%-7d  ",vlanId);
			//vty_out(vty,"%-4d  ",vidx);
			//vty_out(vty,"%-3u.%-3u.%-3u.%-3u  ",ip0,ip1,ip2,ip3);
			
			//show_group_member_slot_port(vlanId,mbrBmp);
			show_group_member_slot_port(product_id,mbrBmp);

			q->next=NULL;
			tail->next=q;
			tail=q;

			//vty_out(vty,"\n");
		      }
		//vty_out(vty,"========================================================================\n");
	      }
		}
	/*
	else
	{
		if(NPD_GROUP_NOTEXIST == ret) {
			//vty_out(vty,"Error: l2mc entry NOT exist.\n");
			return NPD_GROUP_NOTEXIST;
		}
		else if (NPD_VLAN_ERR_HW == ret) {
			//vty_out(vty,"Error occurs in Read vidx Entry in HW.\n");
			return NPD_VLAN_ERR_HW;  
		}
		return ret;
	}
  */
	else if(IGMPSNP_RETURN_CODE_GROUP_NOTEXIST == ret) {
		//vty_out(vty,"%% Error:layer 2 multicast entry not exist!\n");
		return NPD_GROUP_NOTEXIST;
	}
	else if (IGMPSNP_RETURN_CODE_ERROR_HW == ret) {
		//vty_out(vty,"%% Read layer 2 multicast entry from hardware error!\n");	
		return NPD_VLAN_ERR_HW;  
	}
	else if(IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL == ret){
		//vty_out(vty,"%% Error:igmp snooping not enabled global!\n");
		return 1;
	}
	else if(IGMPSNP_RETURN_CODE_NOTENABLE_VLAN == ret){
		//vty_out(vty,"%% Error:vlan %d not enabled igmp snooping!\n",vlanId);
		return 4;
		
	}
	//else if(IGMPSNP_RETURN_CODE_MC_VLAN_NOT_EXIST == ret){
	//	vty_out(vty,"%% Error:layer 2 multicast vlan %d not exist!\n",vlanId);	
	//}
	else {
		//vty_out(vty,"%% Error %d\n",ret);
		return CMD_FAILURE;
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;

}

//common add version 1.23 
int show_igmp_mcgroup_count
(
	unsigned short vlanId
)
{
	DBusMessage     *query, *reply;
	DBusError        err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int 	 group_Cont = 0, groupIp = 0;
	unsigned short   vidx = 0;
	unsigned int	 j,product_id = 0;
	unsigned int 	 ret = 0;
	PORT_MEMBER_BMP  mbrBmp;

//	unsigned char    ip0 = 0, ip1 = 0, ip2 = 0, ip3 = 0;

    memset(&mbrBmp,0,sizeof(PORT_MEMBER_BMP));
	query = dbus_message_new_method_call(\
						NPD_DBUS_BUSNAME,	\
						NPD_DBUS_VLAN_OBJPATH ,	\
						NPD_DBUS_VLAN_INTERFACE ,	\
						NPD_DBUS_VLAN_METHOD_SHOW_VLAN_MCGROUP_LIST_PORT_MEMBERS_V1 );

	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16, &vlanId,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection, query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		//vty_out(vty, "failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s", err.name, err.message);
			dbus_error_free(&err);
		}
		return CMD_FAILURE;
	}

	dbus_message_iter_init(reply, &iter);

	dbus_message_iter_get_basic(&iter, &ret);
	if(IGMPSNP_RETURN_CODE_OK == ret){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &group_Cont);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter, &product_id);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter, &iter_array);

		//vty_out(vty,"====================\n");
		//vty_out(vty,"%-7s  %-11s\n", "VLAN ID", "GROUP_COUNT");
		//vty_out(vty,"=======  ===========\n");
		//vty_out(vty,"%-7d  ", vlanId);
		//vty_out(vty,"%-4d  ", group_Cont);
		//vty_out(vty,"\n====================\n");

		/* this "for" loop isn't use in fact, but existing for the dbus function  */
		for (j = 0; j < group_Cont; j++) {
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array, &iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct, &vidx);
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct, &groupIp);
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct, &(mbrBmp.portMbr[0]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct, &(mbrBmp.portMbr[1]));
			dbus_message_iter_next(&iter_array);			

		}
	}
	else if(IGMPSNP_RETURN_CODE_GROUP_NOTEXIST == ret) {
		//vty_out(vty,"%% Error:layer 2 multicast group not exist!\n");
		return CMD_FAILURE;
	}
	else if (IGMPSNP_RETURN_CODE_ERROR_HW == ret) {
		//vty_out(vty,"%% Read layer 2 multicast entry from hardware error!\n");
				return CMD_FAILURE;

	}
	else if(IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL == ret){
		//vty_out(vty,"%% Error:igmp snooping not enabled global!\n");
				return CMD_FAILURE;

	}
	else if(IGMPSNP_RETURN_CODE_NOTENABLE_VLAN == ret){
		//vty_out(vty,"%% Error:vlan %d not enabled igmp snooping!\n", vlanId);
				return CMD_FAILURE;

	}
	else {
		//vty_out(vty,"%% Error %d\n", ret);
				return ret;

	}
		
	dbus_message_unref(reply);
	return 0;
}




int dcli_get_slot_port_by_gindex
(
	unsigned int port_index,
	unsigned char *slot_no,
	unsigned char *local_port_no
) 
{
	DBusMessage *query, *reply;
	DBusError err;
	int op_ret;
	unsigned int eth_g_index = port_index;
	unsigned char slot = 0,port = 0;

	dbus_error_init(&err);
	query = dbus_message_new_method_call(				\
										NPD_DBUS_BUSNAME,	\
										NPD_DBUS_VLAN_OBJPATH ,	\
										NPD_DBUS_VLAN_INTERFACE ,	\
										NPD_DBUS_VLAN_METHOD_GET_SLOTPORT_BY_INDEX
										);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&eth_g_index,										
							DBUS_TYPE_INVALID);


	reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		//printf("failed get re_slot_port reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}
	
	if (dbus_message_get_args ( reply, &err,
								DBUS_TYPE_UINT32,&op_ret,
								DBUS_TYPE_BYTE,&slot,
								DBUS_TYPE_BYTE,&port,
								DBUS_TYPE_INVALID)) {
		if(NPD_DBUS_SUCCESS == op_ret){
			*slot_no = slot;
			*local_port_no = port;
			/*DCLI_DEBUG(("eth port index %d -->slot %d,port %d.",eth_g_index,slot,port));*/
			dbus_message_unref(reply);
			return CMD_SUCCESS;
		}
		else if(NPD_DBUS_ERROR_NO_SUCH_PORT == op_ret){
			//printf("Bad slot/port Number.\n");
			dbus_message_unref(reply);
			return CMD_FAILURE;
		}		
	} 
	else {
		//printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) {
				//printf("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_FAILURE;
	}
	return CMD_SUCCESS;
}

/*
//show igmp vlan list snp version 1.33 added 
* 显示所有的信息和相应的端口配置
*/
int show_igmp_vlan_list
(
    struct igmp_sum *head,
	int *num,
	int *port_num
)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned int vlan_Cont = 0;
	unsigned short  vlanId = 0;
	//unsigned int	untagBmp = 0,tagBmp = 0;
	PORT_MEMBER_BMP	untagBmp ,tagBmp;
	unsigned int 	product_id = PRODUCT_ID_NONE,j,ret;
	
    memset(&untagBmp,0,sizeof(PORT_MEMBER_BMP));
	memset(&tagBmp,0,sizeof(PORT_MEMBER_BMP));


	struct igmp_sum *q,*tail;	
    

	dbus_error_init(&err);

	query = dbus_message_new_method_call(
										NPD_DBUS_BUSNAME,			\
										NPD_DBUS_VLAN_OBJPATH,		\
										NPD_DBUS_VLAN_INTERFACE,		\
										NPD_DBUS_VLAN_METHOD_IGMP_SNP_VLAN_LIST_SHOW_V1);
	/*No params*/
	
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
	if(IGMPSNP_RETURN_CODE_OK == ret){

		head->next=NULL;
		tail=head;
		*num=vlan_Cont;

		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&vlan_Cont);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&product_id);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		//vty_out(vty,"=================================================\n");
		//vty_out(vty,"%-7s  %-40s\n","VLAN ID","PORT MEMBER LIST");
		//vty_out(vty,"=======  ========================================\n");

		for (j = 0; j < vlan_Cont; j++) {

			///分配空间
		 q=(struct igmp_sum *)malloc(sizeof(struct igmp_sum));
		 if ( NULL==q )
			{
			return 0;
			}
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&vlanId);

			q->vlanId=vlanId;  

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

				//vty_out(vty,"%-7d  ",vlanId);
				
				//if(0 != untagBmp || 0 != tagBmp)
				if ((0 != untagBmp.portMbr[0]) ||
					(0 != untagBmp.portMbr[1]) ||
					(0 != tagBmp.portMbr[0]) ||
					(0 != tagBmp.portMbr[1]))

					return 1;
				else 
					return CMD_FAILURE;
					//vty_out(vty,"%% %-45s","no port enabled igmp snooping!\n");
				//vty_out(vty,"\n");
			}

			q->next=NULL;
			tail->next=q;
			tail=q;
		}
	}
	else if (IGMPSNP_RETURN_CODE_NOT_ENABLE_GBL == ret){
			//vty_out(vty,"%% Error:igmp snooping service not enabled!\n");
	}
	else if(IGMPSNP_RETURN_CODE_VLAN_NOT_EXIST == ret) {
		//vty_out(vty,"%% Error:no vaild vlan exist.\n");
	}
	else if (IGMPSNP_RETURN_CODE_ERROR_HW == ret) {
		//(vty,"%% Read vlan entry from hardware error!\n");	
	}
	else if (IGMPSNP_RETURN_CODE_NOTENABLE_VLAN == ret){
		//vty_out(vty,"%% igmp snooping service not enabled in vlan!\n");
	}

	dbus_message_unref(reply);
	return CMD_SUCCESS;
	
}

int show_vlan_port_member
(
	char * VID,
	struct igmp_port *head,
	int *num
)
{
	DBusMessage *query, *reply;
	DBusError err;
	/*DBusMessageIter	 iter;
	
	DBusMessageIter	 iter_array;*/
	unsigned short	vlanId;
	char*			vlanName;
    //unsigned int count;
	unsigned int	ret;
	/*unsigned int 	i = 0,row = 0;
	unsigned int slot = 0,port = 0,tmpVal = 0;*/
	unsigned int product_id = 0; 
	unsigned int vlanStat = 0;
	//unsigned int bitOffset = 0,mbrCount = 0;
	unsigned int promisPortBmp[2] = {0};
	
	PORT_MEMBER_BMP untagPortBmp,tagPortBmp;
	memset(&untagPortBmp,0,sizeof(PORT_MEMBER_BMP));
	memset(&tagPortBmp,0,sizeof(PORT_MEMBER_BMP));
	ret = parse_vlan_no((char*)VID,&vlanId);

	if (NPD_FAIL == ret) {
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
	
	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
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
		if(0==ret) {
			//vty_out(vty,"Codes:  U - vlan link status is up,   D - vlan link status is down, \n");
			//vty_out(vty,"        u - untagged port member,      t - tagged port member, \n");
			//vty_out(vty,"        * - promiscuous mode port member\n");
			//vty_out(vty,"========================================================================\n");
			//vty_out(vty,"%-7s  %-20s  %-40s\n","VLAN ID","VLAN NAME","PORT MEMBER LIST");
			//vty_out(vty,"=======  ====================  =========================================\n");
			
			//vty_out(vty,"%-4d(%s)  ",vlanId,vlanStat ? "U":"D");
			//vty_out(vty,"%-20s	",vlanName);

			if(0 != untagPortBmp.portMbr[0] || 0 != tagPortBmp.portMbr[0] || 0 != untagPortBmp.portMbr[1] || 0 != tagPortBmp.portMbr[1]){

				//直接引用参数
				show_vlan_slot_port(product_id,untagPortBmp,tagPortBmp,head,num);
			}
			else 
				//vty_out(vty,"  %-40s","No Port member.");
				return CMD_FAILURE;
		}
		else if(NPD_DBUS_ERROR_NO_SUCH_VLAN +1== ret) {
			//vty_out(vty,"% Bad parameter,vlan id illegal.\n");
			return CMD_FAILURE;
		}
		else if(NPD_VLAN_NOTEXISTS == ret) {
			//vty_out(vty,"% Bad parameter,vlan %d not exist.\n",vlanId);
			return CMD_FAILURE;
		}
		else if(0xff == ret) {
			//vty_out(vty,"%% Error,operation on hardware fail.\n");
			return CMD_FAILURE;
		}
	}
	else {
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


#ifdef __cplusplus
}
#endif


