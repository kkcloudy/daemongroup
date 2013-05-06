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
* ws_dcli_interface.c
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
/*#include "cgic.h"*/
#include <unistd.h>
#include "ws_dcli_interface.h"
#include "ws_returncode.h"

///////////////////////////////////////////////////////////////////
/* dcli_intf.c  version :v1.55  wangpeng 2009-1-6*/     
//modify 2010-03-02 zhouymv1.125
///////////////////////////////////////////////////////////////////


int interface_eth_port(char port[12],char tag[12])/*返回0表示失败，返回1表示成功，返回-1表示Unknow portno format,返回-2表示The internal tag %d is the same as the external tag */
                           /*返回-3表示NO SUCH PORT，返回-4表示FAILED to add port %d/%d to vlan*/
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret;
	unsigned int op_ret = 0 ;
	unsigned char slot_no = 0, port_no = 0;
	unsigned int vid = 0;
	unsigned int vid2 = 0;
	int retu;

	ret = parse_slotport_no((char *)port,&slot_no,&port_no);	
	ret = parse_param_no((char *)tag,&vid);	
	
	query = dbus_message_new_method_call(			\
							NPD_DBUS_BUSNAME,					\
							NPD_DBUS_INTF_OBJPATH,		\
							NPD_DBUS_INTF_INTERFACE,		\
							NPD_DBUS_SUB_INTERFACE_CREATE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
							 DBUS_TYPE_UINT32,&vid,
                             DBUS_TYPE_UINT32,&vid2,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) 
	{
		if(dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
		retu = 0;
	}
	
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)){
            if (INTERFACE_RETURN_CODE_SUBIF_EXISTS == op_ret)
            {
                op_ret = INTERFACE_RETURN_CODE_SUCCESS;
            }
            if (INTERFACE_RETURN_CODE_SUCCESS == op_ret)
            {
                dbus_message_unref(reply);
                sleep(1);
                retu = 1;
            }
            else if (INTERFACE_RETURN_CODE_QINQ_TWO_SAME_TAG == op_ret)
            {
                retu = -2;
            }
            else if (INTERFACE_RETURN_CODE_NO_SUCH_PORT == op_ret)
            {
                retu = -3;
            }
            else if (INTERFACE_RETURN_CODE_ADD_PORT_FAILED == op_ret)
            {
                retu = -4;
            }
            else
            {
                retu = -5;
				//vty_out(vty,dcli_error_info_intf(op_ret));

            }
        }

	else 
	{
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return retu;
}

int no_interface_eth_port(char port[12],char tag[12])
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret;
	unsigned int op_ret = 0;
	//unsigned int ifIndex = 0;
	unsigned char slot_no = 0, port_no = 0;
	unsigned int vid = 0;
	//struct interface *ifp = NULL;

	//DCLI_DEBUG(("before parse_slotport_no %S\n",argv[0]));
	//if(argc >2) {
		//vty_out(vty,"input bad param\n");
	//}

	ret = parse_slotport_no((char *)port,&slot_no,&port_no);
	
	if (CMD_FAILURE== ret) 
	{
    		//vty_out(vty,"Unknow portno format.\n");
		return CMD_FAILURE;
	}
	
	ret = parse_param_no((char *)tag,&vid);
	
	if(CMD_FAILURE == ret) 
	{
    		//vty_out(vty,"Unknow portno format.\n");
		return CMD_FAILURE;
	}
	//DCLI_DEBUG(("after vid %d\n",vid));

	query = dbus_message_new_method_call(			\
							NPD_DBUS_BUSNAME,					\
							NPD_DBUS_INTF_OBJPATH,		\
							NPD_DBUS_INTF_INTERFACE,		\
							NPD_DBUS_SUB_INTERFACE_DELETE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
							 DBUS_TYPE_UINT32,&vid,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) 
	{
		//vty_out(vty,"failed get reply.\n");
		if(dbus_error_is_set(&err)) 
		{
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_SUCCESS;
	}
	
	//DCLI_DEBUG(("query reply not null\n"));
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if (CMD_SUCCESS == op_ret ) 
			{
				dbus_message_unref(reply);
				return CMD_SUCCESS;
			}
			else if(NPD_DBUS_ERROR_NO_SUCH_PORT == op_ret) 	
			{
				//vty_out(vty,"%% no such port\n");
				return NPD_DBUS_ERROR_NO_SUCH_PORT;
			}
			else if(NPD_DBUS_ERROR == op_ret)
			{
				//vty_out(vty,"%% create intf error\n");
				return NPD_DBUS_ERROR;
			}
			else if((0 != slot_no) && (DCLI_VLAN_NOTEXISTS == op_ret))
			{
				//vty_out(vty,"%% vlan not exist\n");
				return DCLI_VLAN_NOTEXISTS;
			}
			else if((0 != slot_no) && (NPD_VLAN_BADPARAM == op_ret))	
			{
				//vty_out(vty,"%% input tag error\n");
				return NPD_VLAN_BADPARAM;
			}
			else if(DCLI_NOT_CREATE_ROUTE_PORT_SUB_INTF == op_ret)
			{
				//vty_out(vty,"%% Don't del route port sub interface\n");
				return DCLI_NOT_CREATE_ROUTE_PORT_SUB_INTF;
			}
			else if(DCLI_NOT_CREATE_VLAN_INTF_SUB_INTF == op_ret)
			{
				//vty_out(vty,"%% Don't del vlan interface sub interface\n");
				return DCLI_NOT_CREATE_VLAN_INTF_SUB_INTF;
			}
			else if(NPD_VLAN_PORT_NOTEXISTS == op_ret)
			{
				//vty_out(vty,"%% The port isn't the tag\n");
				return NPD_VLAN_PORT_NOTEXISTS;
			}
			else if(DCLI_PARENT_INTF_NOT_EXSIT == op_ret)
			{
				//vty_out(vty,"%% Parent interface not exist\n");
				return DCLI_PARENT_INTF_NOT_EXSIT;
			}
			else if(DCLI_PROMI_SUBIF_NOTEXIST == op_ret)
			{
				//vty_out(vty,"%% sub interface not exist\n");
				return DCLI_PROMI_SUBIF_NOTEXIST;
			}

			dbus_message_unref(reply);
			return CMD_WARNING;
			
	} 
	else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/////view which advanced-routing enable
int ccgi_intf_show_advanced_routing
(	
	unsigned int vlanAdv,
	unsigned int includeRgmii,
	char *infname
)
{

char *showStr = NULL;
DBusMessage *query, *reply;
DBusError err;
int ret = 0;
char *p;
#ifndef INTF_ADVANCED_ROUTING_SHOW_STR_LEN
#define INTF_ADVANCED_ROUTING_SHOW_STR_LEN 30
#endif
	
	query = dbus_message_new_method_call(		\
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_INTF_OBJPATH,		\
								NPD_DBUS_INTF_INTERFACE,			\
								NPD_DBUS_INTF_ADVANCED_ROUTING_SAVE_CFG);

	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&vlanAdv,
							 DBUS_TYPE_UINT32,&includeRgmii,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		//return ret;
	}


	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)){	
		if(NULL != showStr){
			if(strcmp(infname,"")!=0)
			{
				p=strstr(showStr,infname);

				if(p)
					ret=5;
				else
					ret=2;
			}
			else
				ret=2;
				

            #if 0
			tmpShowStr = strtok(showStr,"\n");
			while(NULL != tmpShowStr){
			if(0 == strncmp(tmpShowStr,"interface",strlen("interface"))){
			p=strstr(tmpShowStr,infname);
			if(p)
			ret=5;
			else
			ret=2;

			fprintf(stderr,"zym ret is : %d \n",ret);
			}
			tmpShowStr = strtok(NULL,"\n");
			}
         #endif
		}
		else
		{
			//vtysh_add_show_string_parse(showStr);
			ret=1;
		}
	} 
	else 
	{
		ret = 0;
		if (dbus_error_is_set(&err)) 
		{
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return ret;	
}

/////view which advanced-routing enable
int ccgi_intf_show_advanced_routing_list
(	
	unsigned int vlanAdv,
	unsigned int includeRgmii,
	struct eth_portlist *chead,
	int *cnum
)
{

	char *showStr = NULL;
	DBusMessage *query, *reply;
	DBusError err;
	int ret = 0,i=0;
	char *tmpShowStr=NULL;

		
	query = dbus_message_new_method_call(		\
									NPD_DBUS_BUSNAME,			\
									NPD_DBUS_INTF_OBJPATH,		\
									NPD_DBUS_INTF_INTERFACE,			\
									NPD_DBUS_INTF_ADVANCED_ROUTING_SAVE_CFG);

		dbus_error_init(&err);
		
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&vlanAdv,
								 DBUS_TYPE_UINT32,&includeRgmii,
								 DBUS_TYPE_INVALID);

		reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);

		dbus_message_unref(query);
		if (NULL == reply) {
			if (dbus_error_is_set(&err)) {
				dbus_error_free(&err);
			}
			//return ret;
		}

		struct eth_portlist *ctail=NULL;
		chead->next=NULL;
		ctail=chead;
		i=0;
		if (dbus_message_get_args ( reply, &err,
						DBUS_TYPE_STRING, &showStr,
						DBUS_TYPE_INVALID)){	
			if(NULL != showStr){			
				tmpShowStr = strtok(showStr,"\n");
				while(NULL != tmpShowStr){
				if(0 == strncmp(tmpShowStr,"interface",strlen("interface")))
				{
					struct eth_portlist  *cq=NULL;
					cq=(struct eth_portlist *)malloc(sizeof(struct eth_portlist)+1);
					memset(cq,0,sizeof(struct eth_portlist)+1);
					if(NULL == cq)
					{
						return  -1;
					}
					memset(cq->ethport,0,30);
					char *qpt = NULL;	
					qpt = strchr(tmpShowStr,' ');
					strcpy(cq->ethport, qpt+4 );
					
					i++;
					cq->next = NULL;
					ctail->next = cq;
					ctail = cq;
				}
				tmpShowStr = strtok(NULL,"\n");
				}
			}
			else
			{
				//vtysh_add_show_string_parse(showStr);
				ret=1;
			}
		} 
		else 
		{
			ret = 0;
			if (dbus_error_is_set(&err)) 
			{
				dbus_error_free(&err);
			}
		}
		*cnum = i;
		dbus_message_unref(reply);
		return ret;	
}

//free link list
void Free_ethp_info(struct eth_portlist *head)
{
    struct eth_portlist *f1,*f2;
	f1=head->next;		 
	f2=f1->next;
	while(f2!=NULL)
	{
	  free(f1);
	  f1=f2;
	  f2=f2->next;
	}
	free(f1);
}

int ccgi_interface_ifname_vlan(char *ptr)
{
	//int ret = 0;
	int i = 0;
	unsigned long vlanid = 0;
	//unsigned int advanced = 0;
	//unsigned int intfinfo = 0;
	if(0 != strncmp(ptr,"vlan",4)){
		return CMD_SUCCESS;
	}
	char *id = (char *)malloc(sizeof(char)*25);
	if(id == NULL)
	{
		return CMD_WARNING;
	}
	
	memset(id,0,25);
	memcpy(id,ptr+4,(strlen(ptr)-4));
	for (i = 0;i<strlen(ptr)-4;i++){
		if((id[0] == '0')||(id[0] == '\0')||((id[i]!='\0')&&((id[i]<'0')||(id[i]>'9')))){
			//vty_out(vty,"%% Bad parameter: %s !\n",ptr);
			free(id);
			id = NULL;
            return CMD_WARNING;
		}
	}
	vlanid = strtoul(id,NULL,10);
	free(id);
	id = NULL;
	if((vlanid < 1)||(vlanid >4094)){
        //vty_out(vty,"%% Bad parameter: %s !\n",ptr);
		return CMD_WARNING;
	}
	return ccgi_create_vlan_intf_by_vlan_ifname((unsigned short)vlanid);
}
int ccgi_no_interface_ifname_vlan(char *ptr)
{	
	int i = 0;
	unsigned long vlanid = 0;
	char *id = NULL;
	if(NULL == ptr) return CMD_WARNING;
	if(0 != strncmp(ptr,"vlan",4)){
		return CMD_SUCCESS;
	}
	id = (char *)malloc(sizeof(char)*25);
	if(id == NULL)
	{
		return CMD_WARNING;
	}
	memset(id,0,25);
	memcpy(id,ptr+4,(strlen(ptr)-4));
	for (i = 0;i<strlen(ptr)-4;i++){
		if((id[0] == '0')||((id[i]!='\0')&&((id[i]<'0')||(id[i]>'9')))){
			//vty_out(vty,"%% Bad parameter: %s !\n",ptr);
			free(id);
			id = NULL;
            return CMD_WARNING;
		}
	}
	vlanid = strtoul(id,NULL,10);
	free(id);
	id = NULL;
	if((vlanid < 1)||(vlanid >4094)){
        //vty_out(vty,"%% Bad parameter: %s !\n",ptr);
		return CMD_WARNING;
	}
	return ccgi_del_vlan_intf((unsigned short)vlanid);
}
int ccgi_interface_ifname_eth_port(char * ptr)
{
	int ret = 0;
	int i = 0;
	//unsigned long vlanid = 0;
	//unsigned int advanced = 0;
	unsigned char slot = 0;
	unsigned char port = 0;
	unsigned int tag1 = 0,tag2=0;
	//unsigned int ifIndex = ~0UI;
	//unsigned int intfinfo = 0;
	if(0 != strncmp(ptr,"eth",3)){
		return CMD_SUCCESS;
	}
	char *id = (char *)malloc(sizeof(char)*25);
	if(id == NULL)
	{
		return CMD_WARNING;
	}
	memset(id,0,25);
	memcpy(id,ptr+3,(strlen(ptr)-3));	
	for (i = 0;i<strlen(ptr)-3;i++){
		if((id[i] != '-')&&((id[i] != '.')&&((id[i]!='\0')&&((id[i]<'0')||(id[i]>'9'))))){
			//vty_out(vty,"%% Bad parameter: %s !\n",ptr);
			free(id);
			id = NULL;
            return CMD_WARNING;
		}
	}
	ret = parse_slotport_tag_no(id,&slot,&port,&tag1,&tag2);
	free(id);
	id = NULL;
	if((COMMON_SUCCESS != ret)||(tag1>4094)||(tag2>4094)){
		//vty_out(vty,"%% Bab parameter: %s ",ptr);
		return CMD_WARNING;
	}
	if(0 == tag1 )
	{
		return ccgi_eth_port_interface_mode_config(slot,port);
	}
	else 
	{
		//if(DEFAULT_VLAN_ID == tag)
		//{
			//y_out(vty,"%% The tag must be 2-4094!\n");
			//return CMD_WARNING;
		//}
		return ccgi_create_eth_port_sub_intf(slot,port,tag1,tag2);
	}

}

int ccgi_create_eth_port_sub_intf
(
    unsigned char slot_no,
    unsigned char port_no,
    unsigned int vid,
    unsigned int vid2

)
{
	DBusMessage *query, *reply;
	DBusError err;

	unsigned int op_ret = 0;
	//struct interface *ifp = NULL;

	query = dbus_message_new_method_call(			\
							NPD_DBUS_BUSNAME,					\
							NPD_DBUS_INTF_OBJPATH,		\
							NPD_DBUS_INTF_INTERFACE,		\
							NPD_DBUS_SUB_INTERFACE_CREATE);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
                             DBUS_TYPE_UINT32,&vid,
                             DBUS_TYPE_UINT32,&vid2,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_WARNING;
	}

	//DCLI_DEBUG(("query reply not null\n"));
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			/*vty_out(vty,"return op_ret %d\n",op_ret);*/
			if(INTERFACE_RETURN_CODE_SUBIF_EXISTS == op_ret){
                op_ret = INTERFACE_RETURN_CODE_SUCCESS;
			}
			if (INTERFACE_RETURN_CODE_SUCCESS == op_ret ) {
				dbus_message_unref(reply);
				//sleep(1);
				return CMD_SUCCESS;
			}
			else if(INTERFACE_RETURN_CODE_VLAN_NOTEXIST== op_ret){
				if(0 != slot_no){
					//vty_out(vty,"%% Vlan not exists!\n");
				}
			}
			else if(COMMON_RETURN_CODE_BADPARAM== op_ret){
				if(0 != slot_no){
					//vty_out(vty,"%% Input tag error!\n");
				}
			}
			else if(INTERFACE_RETURN_CODE_NO_SUCH_PORT == op_ret){
				//vty_out(vty,"%% NO SUCH PORT %d/%d, tag %d!\n",slot_no,port_no,vid);
			}
			else if(INTERFACE_RETURN_CODE_ADD_PORT_FAILED == op_ret){
                //vty_out(vty,"%% FAILED to add port %d/%d to vlan %d!\n",slot_no,port_no,vid);
			}
			else {
				//vty_out(vty,dcli_error_info_intf(op_ret));
			}

	}
	else 
	{
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_WARNING;

}

int ccgiconfig_no_interface_ifname_eth_port(char * ptr)/*0:succ;other:fail*/
{
	int ret = 0;
	int i = 0;
	unsigned long vlanid = 0;
	unsigned int advanced = 0;
	unsigned char slot = 0;
	unsigned char port = 0;
	unsigned int mode = 0;
	unsigned int tag = 0;
	unsigned int tag2 = 0;
	int retu = 0;
	if (0 != strncmp(ptr,"eth",3))
	{
		return 0;
	}
	char *id = (char *)malloc(sizeof(char)*25);
	if (id == NULL)
	{
		return -1;
	}
	memset(id,0,25);
	memcpy(id,ptr+3,(strlen(ptr)-3));
	mode = ETH_PORT_FUNC_BRIDGE;
	ret = parse_slotport_tag_no(id,&slot,&port,&tag,&tag2);
	free(id);
	id = NULL;
	if ((0 != ret)||(tag>4094)||(tag2>4094))
	{
		//vty_out(vty,"%% Bab parameter: %s ",ptr);
		return -2;
	}
	if (0 == tag)
	{
		retu =  ccgi_eth_port_mode_config(slot,port,mode);
	}
	else
	{
		retu =  ccgi_del_eth_port_sub_intf(slot,port,tag,tag2);
	}
	return retu;
}
int ccgi_del_eth_port_sub_intf
(
    unsigned char slot_no,
    unsigned char port_no,
    unsigned int vid,
    unsigned int vid2
)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int op_ret = 0 ;
	//struct interface *ifp = NULL;

	query = dbus_message_new_method_call(			\
							NPD_DBUS_BUSNAME,					\
							NPD_DBUS_INTF_OBJPATH,		\
							NPD_DBUS_INTF_INTERFACE,		\
							NPD_DBUS_SUB_INTERFACE_DELETE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&slot_no,
							 DBUS_TYPE_BYTE,&port_no,
                             DBUS_TYPE_UINT32,&vid,
                             DBUS_TYPE_UINT32,&vid2,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_WARNING;
	}
	
	//DCLI_DEBUG(("query reply not null\n"));
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_INVALID)) {
			if (INTERFACE_RETURN_CODE_SUCCESS == op_ret ) {
				dbus_message_unref(reply);
				//sleep(1);
				return CMD_SUCCESS;
			}
			else {
				//vty_out(vty,dcli_error_info_intf(op_ret));
				return -1;
			}

			dbus_message_unref(reply);
			return CMD_WARNING;
			
	} 
	else {
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_WARNING;

}
int ccgi_no_interface_ifname_eth_port(char * ptr)
{
	int ret = 0;
	//int i = 0;
	//unsigned long vlanid = 0;
	//unsigned int advanced = 0;
	unsigned char slot = 0;
	unsigned char port = 0;
	unsigned int mode = 0;
	unsigned int tag1 = 0,tag2=0;
	if(0 != strncmp(ptr,"eth",3)){
		return CMD_SUCCESS;
	}
	char *id = (char *)malloc(sizeof(char)*25);
	if(id == NULL)
	{
		return CMD_WARNING;
	}
	memset(id,0,25);
	memcpy(id,ptr+3,(strlen(ptr)-3));	
	mode = ETH_PORT_FUNC_BRIDGE;
	ret = parse_slotport_tag_no(id,&slot,&port,&tag1,&tag2);
	free(id);
	id = NULL;
	if((NPD_SUCCESS != ret)||(tag1>4094)||(tag2>4094)){
		//vty_out(vty,"%% Bab parameter: %s ",ptr);
		return CMD_WARNING;
	}
	if(0 == tag1)
	{
	    return ccgi_eth_port_mode_config(slot,port,mode);
	}
	else
	{
        return ccgi_del_eth_port_sub_intf(slot,port,tag1,tag2);
	}

}
int ccgi_create_vlan_intf
(
	unsigned short vid,
	unsigned int advanced
)
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret = 0;
	unsigned int op_ret = INTERFACE_RETURN_CODE_ERROR,ifIndex = 0;
	//unsigned int vId = 0;

	char *pname = NULL;

	if(vid > 4094) {
		//vty_out(vty,"%% input bad param.\n");
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_INTF_OBJPATH,			\
								NPD_DBUS_INTF_INTERFACE,					\
								NPD_DBUS_INTF_METHOD_CREATE_VID_INTF);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vid,
							 DBUS_TYPE_UINT32,&advanced,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		//vty_out(vty,"query reply null\n");
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&ifIndex,
		DBUS_TYPE_STRING,&pname,
		DBUS_TYPE_INVALID)) {
		if (INTERFACE_RETURN_CODE_SUCCESS != op_ret ) {
				//vty_out(vty,dcli_error_info_intf(op_ret));
			    ret = CMD_WARNING;
		}
		dbus_message_unref(reply);
		return ret;
	}
	else{
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_WARNING;
}

int ccgi_del_vlan_intf( unsigned short vId)
{
	DBusMessage *query, *reply;
	DBusError err;
	unsigned int op_ret = 0,ifIndex = 0;
	char name[16] = {'\0'};

	if(vId > 4094) {
		//vty_out(vty,"%% input bad param.\n");
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_INTF_OBJPATH,		\
								NPD_DBUS_INTF_INTERFACE,			\
								NPD_DBUS_INTF_METHOD_DEL_VID_INTF);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vId,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		//vty_out(vty,"query reply null\n");
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32, &ifIndex,
		DBUS_TYPE_INVALID)) {
			if (INTERFACE_RETURN_CODE_SUCCESS == op_ret ) {
				
				sprintf(name,"vlan%d",vId);
				dbus_message_unref(reply);
				sleep(1);
	            return CMD_SUCCESS;
			}
			else{	
			 	//vty_out(vty,dcli_error_info_intf(op_ret));
				
				dbus_message_unref(reply);
				return CMD_WARNING;
		    }
	}else{
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_WARNING;

}


int ccgi_create_vlan_intf_by_vlan_ifname
(
	unsigned short vid
)
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret = CMD_SUCCESS;
	unsigned int op_ret = INTERFACE_RETURN_CODE_ERROR,ifIndex = 0;
	
	char *pname = NULL;

	if(vid > 4094) {
		//vty_out(vty,"%% input bad param.\n");
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_INTF_OBJPATH,			\
								NPD_DBUS_INTF_INTERFACE,					\
								NPD_DBUS_INTF_METHOD_CREATE_VID_INTF_BY_VLAN_IFNAME);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vid,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		//vty_out(vty,"query reply null\n");
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&ifIndex,
		DBUS_TYPE_STRING,&pname,
		DBUS_TYPE_INVALID)) {
		if (INTERFACE_RETURN_CODE_SUCCESS != op_ret ) {
				//vty_out(vty,dcli_error_info_intf(op_ret));
			    ret = CMD_WARNING;
		}
		dbus_message_unref(reply);
		/*sleep(1);*/
		return ret;
	}
	else{
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_WARNING;
}
int ccgi_vlan_interface_advanced_routing_enable
(
	unsigned short vid,
	unsigned int enable
)
{
	DBusMessage *query, *reply;
	DBusError err;
	int ret = CMD_SUCCESS;
	unsigned int op_ret = INTERFACE_RETURN_CODE_ERROR,ifIndex = 0;
	
	char *pname = NULL;

	if(vid > 4094) {
		//vty_out(vty,"%% input bad param.\n");
		return CMD_WARNING;
	}

	query = dbus_message_new_method_call(
								NPD_DBUS_BUSNAME,			\
								NPD_DBUS_INTF_OBJPATH,			\
								NPD_DBUS_INTF_INTERFACE,					\
								NPD_DBUS_INTF_METHOD_VLAN_INTERFACE_ADVANCED_ROUTING_ENABLE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT16,&vid,
							 DBUS_TYPE_UINT32,&enable,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (ccgi_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		//vty_out(vty,"query reply null\n");
		//vty_out(vty,"failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		return CMD_WARNING;
	}
	if (dbus_message_get_args ( reply, &err,
		DBUS_TYPE_UINT32,&op_ret,
		DBUS_TYPE_UINT32,&ifIndex,
		DBUS_TYPE_STRING,&pname,
		DBUS_TYPE_INVALID)) {
        if (INTERFACE_RETURN_CODE_SUCCESS != op_ret ) {
			if(INTERFACE_RETURN_CODE_INTERFACE_NOTEXIST== op_ret){
			   // vty_out(vty,"%% Advanced-routing already disabled!\n");
		    }
		    else if(INTERFACE_RETURN_CODE_ALREADY_ADVANCED== op_ret){
			   // vty_out(vty,"%% Advanced-routing already enabled!\n");
		    }
		    else {
				//vty_out(vty,dcli_error_info_intf(op_ret));
		    }
			ret = CMD_WARNING;
		}
		dbus_message_unref(reply);
		return ret;
	}
	else{
		//vty_out(vty,"Failed get args.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
	}
	dbus_message_unref(reply);
	return CMD_WARNING;
}
#if 0
xxx
#endif
int set_intf_qinq_type(char *infnamez,char *infvalue)
{
        unsigned char intfName[INTERFACE_NAMSIZ+1] = {0};
        unsigned char type [16] = {0};
        unsigned char * ptr = NULL;

        /*if(argc < 1){
            vty_out(vty,"Command incompleted!\n");
            return CMD_WARNING;
        }*/
        //if(argc == 1){
            memcpy(type,(char *)infvalue,strlen(infvalue)<16 ? strlen(infvalue) : 15);
            memcpy(intfName, infnamez,INTERFACE_NAMSIZ);
       // }
        ptr = type;
        while(*ptr != '\0'){
            if((*ptr != 'x')&&(*ptr != 'X')&&\
                ((*ptr < '0')||(*ptr > '9'))&&\
                ((*ptr < 'a')||(*ptr > 'f'))&&\
                ((*ptr < 'A')||(*ptr > 'F'))){
                //vty_out(vty,"Bad Parameter %s! %s\n",(char *)infvalue,vlan_eth_port_ifname);
                return -2;
            }
            ptr++;
        }
        return ccgi_intf_subif_set_qinq_type(intfName,type);
        
    }

int ccgi_intf_subif_set_qinq_type
(
    unsigned char *intfName,
    unsigned char *type
)/*返回0表示成功，返回-1表示失败，返回-2表示Bad parameter ，返回-3表示Unsupport this command*/
{

    DBusMessage *query, *reply;
    DBusError err;
    unsigned char slot = 0;
    unsigned char port = 0;
    unsigned int  vid1 = 0;
    unsigned int  vid2 = 0;
    int ret = 0;

    query = dbus_message_new_method_call(\
                                         NPD_DBUS_BUSNAME,           \
                                         NPD_DBUS_INTF_OBJPATH,      \
                                         NPD_DBUS_INTF_INTERFACE,            \
                                         NPD_DBUS_INTF_SUBIF_SET_QINQ_TYPE);

    dbus_error_init(&err);
    if(!strncmp(intfName,"eth",3)){
        ret = parse_slotport_tag_no((char *)(intfName + 3),&slot,&port,&vid1,&vid2);
        if(NPD_SUCCESS != ret){
            //vty_out(vty,"%% Bad parameter %s!\n",intfName);
            dbus_message_unref(query);
            return -2;
        }
    }
    else{
        //vty_out(vty,"%% Unsupport this command!\n");
        dbus_message_unref(query);
        return -3;
    }
    
    dbus_message_append_args(query,
                             DBUS_TYPE_UINT16,&slot,
                             DBUS_TYPE_UINT16,&port,
                             DBUS_TYPE_UINT32,&vid1,
                             DBUS_TYPE_UINT32,&vid2,                                     
                             DBUS_TYPE_STRING,&type,
                             DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection,query,-1, &err);

    dbus_message_unref(query);
    if (NULL == reply)
    {
        if (dbus_error_is_set(&err))
        {
            dbus_error_free(&err);
        }
        return -1;
    }
    if (dbus_message_get_args(reply, &err,
                              DBUS_TYPE_UINT32, &ret,
                              DBUS_TYPE_INVALID))
    {
        if(INTERFACE_RETURN_CODE_SUCCESS != ret){
            //vty_out(vty,dcli_error_info_intf(ret));
            return -1;
        }
        dbus_message_unref(reply);
		//sucessful
        return 0;
    }
    else
    {
        if (dbus_error_is_set(&err))
        {
            dbus_error_free(&err);
        }
    }
    dbus_message_unref(reply);
    return ret;
}

/////////////////add new no .h file
int ccgi_config_interface_vanId_advanced(char *plot)
{

	int ret;
	unsigned int vid = 0;
	unsigned int advanced = 1;

	/*printf("before interface vlan parse %s\n",argv[0]);*/
	ret = parse_param_no((char *)plot,&vid);

	if (COMMON_ERROR == ret) {
    	//vty_out(vty,"Unknow portno format.\n");
		return CMD_WARNING;
	}
	
	/*vty->index = ethIntf;
	//printf("interface vId %d\n",vId);*/
	return ccgi_create_vlan_intf(vid,advanced);

}

int ccgi_interface_vlan(char *vlanid)
{

	int ret;
	unsigned int vid = 0;
	//char *pname = NULL;
	unsigned int advanced = 0;

	/*printf("before interface vlan parse %s\n",argv[0]);*/
	ret = parse_param_no((char *)vlanid,&vid);

	if (COMMON_ERROR == ret) {
    	//vty_out(vty,"Unknow vlan id %s\n",argv[0]);
		return CMD_WARNING;
	}

	return ccgi_create_vlan_intf(vid,advanced);

}

int ccgi_no_interface_vlan(char *port)
{
	int ret;
	unsigned int input = 0;
	unsigned short vId = 0;
	
	ret = parse_param_no((char *)port,&input);

	if (COMMON_ERROR == ret) {
    	//vty_out(vty,"Unknow vlan id %s\n",argv[0]);
		return CMD_WARNING;
	}
	vId = (unsigned short)input;
	
	return ccgi_del_vlan_intf(vId);
}

int ccgi_interface_tag(char *arg1,char *arg2)
{
	
	int ret;
    unsigned char slot_no = 0;
	unsigned char port_no = 0;
    unsigned int vid = 0;

	//DCLI_DEBUG(("before parse_slotport_no %s\n",argv[0]));
	//if(argc >2) {
	//	vty_out(vty,"input bad param\n");
	//}
	ret = parse_slotport_no((char *)arg1,&slot_no,&port_no);;
	if (COMMON_ERROR == ret) {
		//vty_out(vty,"Unknow portno format.\n");
		return CMD_SUCCESS;
	}

	ret = parse_param_no((char *)arg2,&vid);
	if (COMMON_ERROR == ret) {
		//vty_out(vty,"Unknow portno format.\n");
		return CMD_SUCCESS;
	}
	//DCLI_DEBUG(("after parse_slotport_no vid %d\n",vid));

	return ccgi_create_eth_port_sub_intf(slot_no,port_no,vid,1);

}

int ccgi_no_interface_tag(char *arg1,char *arg2)
{
	//DBusMessage *query, *reply;
	//DBusError err;
	int ret;
	//unsigned int op_ret = 0 ;
	unsigned char slot_no = 0, port_no = 0;
	unsigned int vid = 0;
	//struct interface *ifp = NULL;

	//DCLI_DEBUG(("before parse_slotport_no %S\n",argv[0]));
	//if(argc >2) {
	//	vty_out(vty,"input bad param\n");
	//}

	ret = parse_slotport_no((char *)arg1,&slot_no,&port_no);;
	if (COMMON_ERROR == ret) {
    	//vty_out(vty,"Unknow portno format.\n");
		return CMD_SUCCESS;
	}
	
	ret = parse_param_no((char *)arg2,&vid);
	if (COMMON_ERROR == ret) {
    	//vty_out(vty,"Unknow portno format.\n");
		return CMD_SUCCESS;
	}
	//DCLI_DEBUG(("after vid %d\n",vid));

	return ccgi_del_eth_port_sub_intf(slot_no,port_no,vid,1);
}

int ccgi_advanced_routing_config(char * ifName,unsigned int isEnable)
{
	unsigned int mode = 0;
	unsigned int flag = 2;
	unsigned char slot_no = 0,port_no =0;
	unsigned int vid = 0;
	
	if(1 == isEnable){
	    mode = ETH_PORT_FUNC_MODE_PROMISCUOUS;
    }
    else if (0 == isEnable){
	    mode = ETH_PORT_FUNC_IPV4;
    }
	else {
		return CMD_WARNING;
	}
	flag = parse_param_ifName(ifName,&slot_no,&port_no,&vid);
	
	if(0 == flag){
        if(CMD_SUCCESS != ccgi_vlan_interface_advanced_routing_enable((unsigned short)vid,isEnable)){
			return CMD_WARNING;
        }
	}
	else if(1 == flag){
		if(0 == vid){
	        if(CMD_SUCCESS != ccgi_eth_port_mode_config(slot_no,port_no,mode)){
				return CMD_WARNING;
			}
		}
		else {
           // vty_out(vty,"%% Sub interface not support advanced-routing!\n");
			return CMD_WARNING;
		}
	}
	else{
           /* support not not ?*/
	}
	return CMD_SUCCESS;
}

/*返回0表示成功，返回-1表示失败*/
int ccgi_interface_ifname_bond(char * ptr)
{
	int i   = 0;
	int ret = -1;
	unsigned int bondid = 0;
	char cmd[MAXLEN_BOND_CMD];
	char bond_file[MAXLEN_BOND_CMD] = {0};
	unsigned short tag1 = 0;
	unsigned short tag2 = 0;
	if(0 != strncmp(ptr, "bond", 4)){
		//return CMD_SUCCESS;
		return -1;
	}
	char *id = (char *)malloc(sizeof(char)*25);
	
	if(id == NULL)
	{
		//return CMD_WARNING;
		return -1;
	}
	
	memset(id, 0, 25);
	memcpy(id, ptr+4, (strlen(ptr)-4));
	
	if(id[0] == '\0'){
		free(id);
		id = NULL;
        return -1;
	}
	
	for (i=0; i<strlen(ptr)-4; i++){
		if((id[i]!='\0') && (id[i]!='.') && ((id[i]<'0')||(id[i]>'9'))){
			free(id);
			id = NULL;
            return -1;
		}
	}
	bondid = strtoul(id, NULL, 10);
	
	for (i=1; i<strlen(ptr)-4; i++){
		
		if((id[i]!='\0') && (id[i]!='.') )
			continue;
		
		if(id[i] == '\0')
			break;
		
		if(id[i] == '.'){
			if(tag1 && tag2){
        		free(id);
        		id = NULL;
				tag1 = tag2 = 0;
                return -1;
			}
			if(tag1)
				tag2 = strtoul(&id[i+1], NULL, 10);
			else
				tag1 = strtoul(&id[i+1], NULL, 10);
		}
	}
	
	
	free(id);
	id = NULL;
	if( (bondid < MIN_BONDID) || (bondid > MAX_BONDID) ){
		return -1;
	}

	//memset(bond_name, 0, MAXLEN_BOND_NAME);
	//sprintf(bond_name, "%s", ptr);
	//vty->index = (void*)bond_name;

	/*If bondx is already exist, just turn to config mode.*/
	sprintf(bond_file,"test -f /proc/net/bonding/%s\n",ptr);
	/*If bondx is already exist, just turn to config mode.*/
	ret = system(bond_file);
	if(!ret){
		return CMD_SUCCESS;
	}

	if( (!tag1) && (!tag2) ){
    	memset(cmd, 0, MAXLEN_BOND_CMD);
    	sprintf(cmd, "sudo modprobe bonding -o %s bondname=%s\n", ptr, ptr);
    	system(cmd);
    	memset(cmd, 0, MAXLEN_BOND_CMD);
    	sprintf(cmd, "sudo ifconfig %s up\n", ptr);
    	system(cmd);
	}else{
		if(tag1){
			memset(cmd, 0, MAXLEN_BOND_CMD);
        	sprintf(cmd, "sudo ifconfig bond%d up\n", bondid);
        	system(cmd);
			memset(cmd, 0, MAXLEN_BOND_CMD);
        	sprintf(cmd, "sudo vconfig add bond%d %d\n", bondid, tag1);
        	system(cmd);
			memset(cmd, 0, MAXLEN_BOND_CMD);
        	sprintf(cmd, "sudo ifconfig bond%d.%d up\n", bondid, tag1);
        	system(cmd);
		}
		if(tag2){
			memset(cmd, 0, MAXLEN_BOND_CMD);
        	sprintf(cmd, "sudo ifconfig bond%d.%d up\n", bondid, tag1);
        	system(cmd);
			memset(cmd, 0, MAXLEN_BOND_CMD);
        	sprintf(cmd, "sudo vconfig add bond%d.%d %d\n", bondid, tag1, tag2);
        	system(cmd);
			memset(cmd, 0, MAXLEN_BOND_CMD);
        	sprintf(cmd, "sudo ifconfig bond%d.%d.%d up\n", bondid, tag1, tag2);
        	system(cmd);
		}
	}
	
	return 0;
}
/**
 *	dcli_no_interface_ifname_bond - delete a bonding interface
 *	@vty
 *	@ptr: a point pointed to a bondname
 *
 */
int ccgi_no_interface_ifname_bond(char * ptr)
{
	int i	= 0;
	int ret = -1;
	unsigned long bondid = 0;
	char cmd[MAXLEN_BOND_CMD];
	unsigned short tag1 = 0;
	unsigned short tag2 = 0;
	
	char bond_file[MAXLEN_BOND_CMD] = {0};
	if(0 != strncmp(ptr, "bond", 4)){
		return -2;
	}
	char *id = (char *)malloc(sizeof(char)*25);
	
	if(id == NULL)
		return -1;
	
	memset(id, 0, 25);
	memcpy(id, ptr+4, (strlen(ptr)-4));
	
	if(id[0] == '\0'){
		free(id);
		id = NULL;
		return -1;
	}
	
	for (i=0; i<strlen(ptr)-4; i++){
		if((id[i]!='\0') && (id[i]!='.') && ((id[i]<'0')||(id[i]>'9'))){
			free(id);
			id = NULL;
			return -1;
		}
	}
	bondid = strtoul(id, NULL, 10);
	
	for (i=1; i<strlen(ptr)-4; i++){
		
		if((id[i]!='\0') && (id[i]!='.') )
			continue;
		
		if(id[i] == '\0')
			break;
		
		if(id[i] == '.'){
			if(tag1 && tag2){
        		free(id);
        		id = NULL;
				tag1 = tag2 = 0;
                return -1;
			}
			if(tag1)
				tag2 = strtoul(&id[i+1], NULL, 10);
			else
				tag1 = strtoul(&id[i+1], NULL, 10);
		}
	}
	
	
	free(id);
	id = NULL;
	if( (bondid < MIN_BONDID) || (bondid > MAX_BONDID) ){
		return -1;
	}

	sprintf(bond_file,"test -f /proc/net/bonding/bond%d\n", bondid);
	/*If bondx is already exist, just turn to config mode.*/
	ret = system(bond_file);
	if(ret){
		return -1;
	}
	memset(cmd, 0, MAXLEN_BOND_CMD);

	/* no interface bondx */
	if( (!tag1) && (!tag2) ){
    	sprintf(cmd, "sudo rmmod %s\n", ptr);
	}
	/* no interface bondx.y or bondx.y.z*/
	else{
		sprintf(cmd, "sudo vconfig rem %s\n", ptr);
	}

	ret = system(cmd);
	
    return 0;
}

/*返回0表示成功，返回-1表示失败，返回-2表示Bad parameter，返回-3表示Only bonding interface support add/delete operation*/
int interface_bond_add_del_port(char *addordel,char *inftype)
{
	int ret = -1;
	int isAdd = 0;
	unsigned int bondid = 0;
	char cmd[MAXLEN_BOND_CMD];
	char *ptr = NULL;
	
	/*fetch the 1st param : add ? delete*/
	if(0 == strncmp(addordel, "add", strlen(addordel))) {
		isAdd = TRUE;
	}
	else if (0 == strncmp(addordel, "delete", strlen(addordel))) {
		isAdd= FALSE;
	}
	else {
		return -1;
	}

	/* fetch the 2nd param : ethm-n/vlann. */
	if( (strncmp(inftype, "eth", 3) != 0) && (strncmp(inftype, "vlan", 4) != 0) ) {
		//vty_out(vty,"% Bad parameter: %s\n", argv[1]);
		return -2;
	}
	
	//ptr = (char *)(vty->index);
	if( (NULL == ptr) || (0 != strncmp(ptr, "bond", 4)) ){
		//vty_out(vty,"% Warning: Only bonding interface support add/delete operation.\n");
		return -3;
	}
	
	memset(cmd, 0, MAXLEN_BOND_CMD);
	if(isAdd){
		sprintf(cmd, "sudo ifenslave -f %s %s\n", ptr, inftype);
	}else{
		sprintf(cmd, "sudo ifenslave -d %s %s\nsudo ifconfig %s up\n", ptr, inftype, inftype);
	}
	
	system(cmd);

	return 0;
}

int show_bond_slave(char *paramz)
{
	unsigned int bondid = 0;
	unsigned int nodesave = 0;
	char cmd[MAXLEN_BOND_CMD];
	char *ptr = NULL;
	int i;
	char bond_file[MAXLEN_BOND_CMD];
	int ret;

	/*match bondx*/
	if(0 != strncmp(paramz, "bond", 4)) {
		return -1;
	}

	ptr = paramz;
	char *id = (char *)malloc(sizeof(char)*25);
	if(id == NULL)
		return -1;
	
	memset(id, 0, 25);
	memcpy(id, ptr+4, (strlen(ptr)-4));
	for (i=0; i<strlen(ptr)-4; i++){
		if((id[0] == '\0')||((id[i]!='\0')&&((id[i]<'0')||(id[i]>'9')))){
			//vty_out(vty, "%% Bad parameter: %s !\n", ptr);
			free(id);
			id = NULL;
            return -2;
		}
	}
	
	bondid = strtoul(id, NULL, 10);
	free(id);
	id = NULL;
	if( (bondid < MIN_BONDID) || (bondid > MAX_BONDID) ){
        //vty_out(vty, "%% Bad parameter: %s !\n", ptr);
		return -2;
	}
	sprintf(bond_file,"test -f /proc/net/bonding/%s\n",ptr);
	/*If bondx is already exist, just turn to config mode.*/
	ret = system(bond_file);
	if(!ret){
		memset(cmd, 0, MAXLEN_BOND_CMD);
    	sprintf(cmd, "sudo cat /proc/net/bonding/bond%d\n", bondid);
    	system(cmd);
		//return CMD_SUCCESS;
		return 0;
	}
	else{
		//vty_out(vty, "%% %s not exist!\n", ptr);
		return -3;
	}

}


unsigned int ccgi_intf_vlan_eth_port_interface_show_advanced_routing
(
        unsigned int flag,
        unsigned int slot_no,
        unsigned int port_no,
        unsigned int vid
)/*返回0表示成功，返回-1表示失败，*/
{
    DBusMessage *query, *reply;
    DBusError err;
    int ret = CMD_SUCCESS; 
    unsigned int op_ret = INTERFACE_RETURN_CODE_ERROR;
    unsigned int isEnable = 0;
	int retu;
    if (0 == flag)
    {
        if (vid > 4094)
        {
           // vty_out(vty,"%% Vlan id %d is out of range!\n",vid);
            return -2;
        }
    }
    if (flag > 1)
    {
       // vty_out("%% Interface %s not support advaced-routing\n",vlan_eth_port_ifname);
        return -3;
    }

    query = dbus_message_new_method_call(
                NPD_DBUS_BUSNAME,           \
                NPD_DBUS_INTF_OBJPATH,          \
                NPD_DBUS_INTF_INTERFACE,                    \
                NPD_DBUS_INTF_METHOD_VLAN_ETH_PORT_INTERFACE_ADVANCED_ROUTING_SHOW);

    dbus_error_init(&err);

    dbus_message_append_args(query,
                             DBUS_TYPE_UINT32,&flag,
                             DBUS_TYPE_UINT32,&slot_no,
                             DBUS_TYPE_UINT32,&port_no,
                             DBUS_TYPE_UINT32,&vid,
                             DBUS_TYPE_INVALID);

    reply = dbus_connection_send_with_reply_and_block(ccgi_dbus_connection,query,-1, &err);

    dbus_message_unref(query);

    if (NULL == reply)
    {
        if (dbus_error_is_set(&err))
        {
            dbus_error_free(&err);
        }
        return -1;
    }
    if (dbus_message_get_args(reply, &err,
                              DBUS_TYPE_UINT32,&op_ret,
                              DBUS_TYPE_UINT32,&isEnable,
                              DBUS_TYPE_INVALID))
    {
        if (INTERFACE_RETURN_CODE_SUCCESS != op_ret)
        {
            if (COMMON_RETURN_CODE_BADPARAM == op_ret)
            {
                //vty_out(vty,"%% Input bad parameter!\n");
                retu = -4;
            }
            else
            {
                //vty_out(vty,dcli_error_info_intf(op_ret));
            }
            ret = -1;
        }
        else
        {
            //vty_out(vty," advanced-routing %s\n",isEnable ? "enabled":"disabled");
            retu = 0;
        }
        dbus_message_unref(reply);
        return ret;
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







#if 0 
int interface_ifnamez(char *ifnamez)
{
        size_t sl;
        int ret = 0;
        char cmd[INTERFACE_NAMSIZ+40];
        int i;
        int cmd_stat;
        char line[80];
        FILE *fp = NULL;
        fp = stdout;
		int retu;

        if ((sl = strlen(ifnamez)) > INTERFACE_NAMSIZ)
        {
            return CMD_WARNING;
        }
        /*get interface name , will be used for vlan interface or eth-port interface
        // when config "advanced-routing enable/disable" in interface node*/
        memcpy((void *)vlan_eth_port_ifname,(void *)ifnamez,INTERFACE_NAMSIZ);
        /*fprintf(stdout,"IFNAME:%s\n",argv[0]);*/
#ifdef _D_WCPSS_/*zhanglei add*/
        /*if ifname is wlan*/
        if (!strncasecmp(ifnamez,"wlan",4))
        {
            ret = wid_interface_ifname_wlan((char*)ifnamez,vty,line);
            if (ret == CMD_SUCCESS)
            {
                
                for (i = 0; i < 7; i++)
                {
                    cmd_stat = vtysh_client_execute(&vtysh_client[i], line, fp);
                    if (cmd_stat == CMD_WARNING||cmd_stat == CMD_FAILURE)
                        break;
                }
                return CMD_SUCCESS;
            }
            else
            {
                return CMD_SUCCESS;
            }
        }


        /*if the ifname is radio*/
        else if (!strncasecmp(ifnamez,"radio",5))
        {
            ret = wid_interface_ifname_radio((char *)ifnamez,vty,line);
            if (ret == CMD_SUCCESS)
            {
                if ((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE))
                {
                    vty->node = INTERFACE_NODE;
                    vty->prenode = 0;
                }
                else if (vty->node == HANSI_NODE)
                {
                    vty->node = INTERFACE_NODE;
                    vty->prenode = HANSI_NODE;
                    vty->index_sub = vty->index;
                }

                for (i = 0; i < 7; i++)
                {
                    cmd_stat = vtysh_client_execute(&vtysh_client[i], line, fp);
                    if (cmd_stat == CMD_WARNING||cmd_stat == CMD_FAILURE)
                        break;
                }
                return CMD_SUCCESS;
            }
            else
            {
                return CMD_WARNING;
            }
        }
        /*if the ifname is ebr*/
        else if (!strncasecmp(ifnamez,"ebr",3))
        {
            ret = wid_interface_ifname_ebr((char *)ifnamez,vty,line);
            if (ret == CMD_SUCCESS)
            {
                if ((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE))
                {
                    vty->node = INTERFACE_NODE;
                }
                else if (vty->node == HANSI_NODE)
                {
                    vty->node = INTERFACE_NODE;
                    vty->prenode = HANSI_NODE;
                    vty->index_sub = vty->index;
                }

                for (i = 0; i < 7; i++)
                {
                    cmd_stat = vtysh_client_execute(&vtysh_client[i], line, fp);
                    if (cmd_stat == CMD_WARNING||cmd_stat == CMD_FAILURE)
                        break;
                }
                return CMD_SUCCESS;
            }
            else
            {
                return CMD_WARNING;
            }
        }
        else
            /*the other ifnames,add here*/
#endif
        {
            if (!strncmp(ifnamez,"vlan",4))
            {
                if (CMD_SUCCESS != dcli_interface_ifname_vlan(vty,(char *)ifnamez))
                {
                    return CMD_WARNING;
                }
            }
            else if (!strncmp(ifnamez,"eth",3))
            {
                if (CMD_SUCCESS != dcli_interface_ifname_eth_port(vty,(char *)argv[0]))
                {
                    return CMD_WARNING;
                }
            }
            else if (!strncmp(ifnamez,"bond",4))
            {
                if (CMD_SUCCESS != dcli_interface_ifname_bond(vty,(char *)argv[0]))
                {
                    return CMD_WARNING;
                }
            }
            else
            {
                if (strcmp(ifnamez,"lo")&&strcmp(ifnamez,"sit0"))
                {
                    vty_out(vty,"%% Error IFNAME\n");
                    return CMD_WARNING;
                }
            }

        }

        if (!strncmp(ifnamez,"eth0",4))
        {
            sprintf(cmd,"sudo ifconfig %s up",ifnamez);
            system(cmd);
        }
        else if (!strncmp(ifnamez,"eth1",4))
        {
            int portno=0;
            char* __tmp = strchr(ifnamez,'-');
            if (__tmp)
                portno=atoi(__tmp+1);
            if (portno>=9&&portno<=12)
            {
                sprintf(cmd,"sudo ifconfig %s up",ifnamez);
                vty_out(vty,"interface %s",ifnamez);
                system(cmd);
            }
        }
        if ((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE))
        {
            vty->node = INTERFACE_NODE;
        }
        else if (vty->node == HANSI_NODE)
        {
            vty->node = INTERFACE_NODE;
            vty->prenode = HANSI_NODE;
            vty->index_sub = vty->index;
        }

        sprintf(line,"interface %s",ifnamez);
        for (i = 0; i < 7; i++)
        {
            cmd_stat = vtysh_client_execute(&vtysh_client[i], line, fp);
            if (cmd_stat == CMD_WARNING||cmd_stat == CMD_FAILURE)
                break;
        }
        return CMD_SUCCESS;
    }
#endif
