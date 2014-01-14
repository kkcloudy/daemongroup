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
* dcli_ac.c
*
* MODIFY:
*		by <weiay@autelan.com> on 09/04/2008 revision <0.1>
*
* CREATOR:
*		weiay@autelan.com
*
* DESCRIPTION:
*		ac config information dynamic config
*
* DATE:
*		09/03/2007	
*
*  FILE REVISION NUMBER:
*  			
*******************************************************************************/

#ifndef HAVE_SOCKLEN_T
#define HAVE_SOCKLEN_T
#endif

#ifdef _D_WCPSS_
#include <string.h>
#include <zebra.h>
#include <dbus/dbus.h>
#include <sys/stat.h>

#include "command.h"
#include "vtysh/vtysh.h"
#include <sys/wait.h>

#include "dcli_main.h"
#include "dcli_dhcp.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "dcli_ac.h"
#include "wid_ac.h"
#include "dcli_wqos.h"
#include "wcpss/asd/asd.h"
#include "wid_wtp.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "dbus/wcpss/wsm_dbus_def.h"
#include "dbus/hmd/HmdDbusDef.h"  /*fengwenchao add 20120228 for AXSSZFI-680*/
#include "bsd/bsdpub.h"

extern struct vtysh_client vtysh_client[];
extern int boot_flag;

int wid_parse_OUI(char* input,OUI_S *ouielem) 
 {
 	
	int i = 0,j=0;
	char cur = 0,value = 0;
	char *p;
	
	if((NULL == input)||(NULL == ouielem)) 
		return -1;
	
	if(strlen(input)!=8)
		return -2;

	p=input;
	
	for(j=0;j<8;j++){
		if(j==2||j==5){
			if(*(p+j)!=':')
				return -3;
			else
				continue;

		}else{
			if(!((*(p+j)>='0'&&*(p+j)<='9')
				||(*(p+j)>='a'&&*(p+j)<='f')
				||(*(p+j)>='A'&&*(p+j)<='F'))){
				return -3;
			}
				
		}
	}

	
	for(i = 0; i <3;i++) {
		cur = *(input++);
		if(cur == ':'){
			i--;
			continue;
		}
		if((cur >= '0') &&(cur <='9')) {
			value = cur - '0';
		}
		else if((cur >= 'A') &&(cur <='F')) {
			value = cur - 'A';
			value += 0xa;
		}
		else if((cur >= 'a') &&(cur <='f')) {
			value = cur - 'a';
			value += 0xa;
		}
		
		ouielem->oui[i]= value;
		cur = *(input++);	
		if((cur >= '0') &&(cur <='9')) {
			value = cur - '0';
		}
		else if((cur >= 'A') &&(cur <='F')) {
			value = cur - 'A';
			value += 0xa;
		}
		else if((cur >= 'a') &&(cur <='f')) {
			value = cur - 'a';
			value += 0xa;
		}
		ouielem->oui[i] = (ouielem->oui[i]<< 4)|value;
	}
	
	return 0;
}

int parse_core_id_list(char* ptr,unsigned short *core)
{
	char* endPtr = NULL;
	int   id1 = 0;
	int   id2 = 0;
	int   min = 0;
	int	  max = 0;
	endPtr = ptr;
	wtp_list_state state = dcli_wtp_check_wtpid;
	struct tag_wtpid *wtp_id = NULL;
	
	while(1)
	{
		switch(state)
		{
			
			case dcli_wtp_check_wtpid: 
									id1 = strtoul(endPtr,&endPtr,10);
									if(id1>0 && id1 <= 16)
									{
					            		state=dcli_wtp_check_comma;
									}
									else
										state=dcli_wtp_check_fail;
									break;
		
			case dcli_wtp_check_comma: 
				
									if(WTP_LIST_SPLIT_COMMA == endPtr[0])
									{
										endPtr = (char*)endPtr + 1;
										state = dcli_wtp_check_wtpid;
										//save wtpid1
										*core |= 0x1 << (id1-1);
																				
									}
									else if(WTP_LIST_SPLIT_BAR == endPtr[0])
									{
										endPtr = (char*)endPtr + 1;
										id2 = strtoul(endPtr,&endPtr,10);
										if(id2>0&&id2 <= 16)
										{
						            		//save wtpid1
											min = (id2 > id1)?id1:id2;
											max = (id2 > id1)?id2:id1;
											while(min <= max)
											{
												*core |= 0x1 << (min-1);													
												min++;
											}
											if('\0' == endPtr[0])
											{
												return 0;
											}
											else
											{											
												endPtr = (char*)endPtr + 1;
												state=dcli_wtp_check_wtpid;
											}
										}
										else
										{
											state = dcli_wtp_check_fail;
										}
										
									}
									else
										state = dcli_wtp_check_end;
									break;
				
		
			case dcli_wtp_check_fail:
				
									return -1;
									break;

			case dcli_wtp_check_end: 
				
									if ('\0' == endPtr[0]) 
									{
										state = dcli_wtp_check_success;
									}
									else
										state = dcli_wtp_check_fail;
									break;
			
			case dcli_wtp_check_success: 				
										//save wtpid1
										*core |= 0x1 << (id1-1);
									return 0;
									break;
			
			default:

				break;
		}
		
	}
	
}



DEFUN(show_attack_mac_cmd_func,
		  show_attack_mac_list_cmd,
		  "show attack ap mac list",
		  "show attack ap mac list\n"
	 )
{
	int ret = -1;
	int num = 0;
	int i = 0;
	int length = 0;
	DCLI_AC_API_GROUP_ONE  *dcli_list = NULL;
	//CW_CREATE_OBJECT_ERR(dcli_list, DCLI_AC_API_GROUP_ONE, return NULL;);	
	//dcli_list->dcli_attack_mac_list = NULL;
	//dcli_list->dcli_essid_list = NULL;
	//dcli_list->dcli_oui_list = NULL;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#if 0
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter,iter_array;
	DBusError err;

    int state = 1;
	unsigned char mac[DCLIAC_MAC_LEN];	
	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}
	ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath(index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_ATTACK_MAC_SHOW);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE, WID_DBUS_CONF_METHOD_ATTACK_MAC_SHOW );*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret == 0)
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		vty_out(vty,"Attack mac list \n");
		for (i = 0; i < num; i++)
		{
			DBusMessageIter iter_struct;
			
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(mac[0]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(mac[1]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(mac[2]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(mac[3]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(mac[4]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(mac[5]));

			dbus_message_iter_next(&iter_array);


			vty_out(vty,"\t%d\t%02X:%02X:%02X:%02X:%02X:%02X\n",i+1,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

dcli_add_attack_ap_mac(mac,&dcli_attack_mac_list);
		}
	}
	else if(ret == WID_NO_WHITELIST)
	{
		vty_out(vty,"there is no attack mac list \n");
	}
	else
	{
		vty_out(vty,"error %d \n",ret);
	}
#endif
	dcli_list = dcli_ac_show_api_group_one(
		index,
		0,/*"show attack ap mac list"*/
		0,
		0,
		&ret,
		0,
		&localid,
		//&dcli_list,
		dcli_dbus_connection,
		WID_DBUS_CONF_METHOD_ATTACK_MAC_SHOW
		);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
			vty_out(vty,"Attack mac list \n");
		unsigned int len = 0;
		struct attack_mac_node*head;
			if(dcli_list->dcli_attack_mac_list == NULL){	
				//printf("dcli_attack_mac_list is NULL.\n");
			}else{
				len  = dcli_list->dcli_attack_mac_list->list_len;
				head = dcli_list->dcli_attack_mac_list->attack_mac_list;
				int ii=0;	
				for (ii = 0; ii < len; ii++){
					if(head != NULL){
						vty_out(vty,"\t%d\t%02X:%02X:%02X:%02X:%02X:%02X\n",ii+1,head->mac[0],head->mac[1],head->mac[2],head->mac[3],head->mac[4],head->mac[5]);
						head = head->next;
					}
				}	
			}
		//CW_FREE_OBJECT(dcli_list->dcli_attack_mac_list->attack_mac_list);
		dcli_ac_free_fun(WID_DBUS_CONF_METHOD_ATTACK_MAC_SHOW,dcli_list);
		dcli_list = NULL;
		}
		else if(ret == WID_NO_WHITELIST)
		{
			vty_out(vty,"there is no attack mac list \n");
		}
		else
		{
			vty_out(vty,"error %d \n",ret);
		}

	//CW_FREE_OBJECT(dcli_list->dcli_attack_mac_list);
	//CW_FREE_OBJECT(dcli_list);

	//dbus_message_unref(reply);	
	
	return CMD_SUCCESS;				
}

DEFUN(show_legal_essid_list_cmd_func,
		  show_legal_essid_list_cmd,
		  "show legal essid list ",
		  "show legal essid list\n"
	 )
{
	int ret = -1;
	int num = 0;
	int i = 0;
	int length = 0;

	DCLI_AC_API_GROUP_ONE  *dcli_list = NULL;
//	CW_CREATE_OBJECT_ERR(dcli_list, DCLI_AC_API_GROUP_ONE, return NULL;);	
//	dcli_list->dcli_attack_mac_list = NULL;
//	dcli_list->dcli_essid_list = NULL;
//	dcli_list->dcli_oui_list = NULL;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

#if	0
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter,iter_array;
	DBusError err;

    int state = 1;
	char *essid=NULL;
	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}
	ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath(index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_ESSID_SHOW);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_ESSID_SHOW );*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret == 0)
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		vty_out(vty,"Legal essid list\n");
		for (i = 0; i < num; i++)
		{
			DBusMessageIter iter_struct;
			
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(essid));
			dbus_message_iter_next(&iter_array);


			vty_out(vty,"\t%d\t%s\n",i+1,essid);
			dcli_add_legal_essid(essid,&dcli_essid_list);

		}
	}
#endif
	dcli_list = dcli_ac_show_api_group_one(
			index,
			0,/*"show legal essid list "*/
			0,
			0,
			&ret,
			0,
			&localid,
			//&dcli_list,			
			dcli_dbus_connection,
			WID_DBUS_CONF_METHOD_ESSID_SHOW
			);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
		vty_out(vty,"Legal essid list\n");
		if(dcli_list->dcli_essid_list != NULL){
			int len;
			int j= 0;
			struct essid_node *head = NULL;
			len = dcli_list->dcli_essid_list->list_len;
			head = dcli_list->dcli_essid_list->essid_list;
			for(j=0;j<len;j++){
				if(head != NULL){
					vty_out(vty,"\t%d\t%s\n",j+1,head->essid);
					head = head->next;
				}
			}
		}
		//CW_FREE_OBJECT(dcli_list->dcli_essid_list->essid_list);
		dcli_ac_free_fun(WID_DBUS_CONF_METHOD_ESSID_SHOW,dcli_list);
		dcli_list = NULL;
	}
	else if(ret == WID_NO_WHITELIST)
	{
		vty_out(vty,"there is no legal essid list \n");
	}
	else
	{
		vty_out(vty,"error %d \n",ret);
	}
		
	//CW_FREE_OBJECT(dcli_list->dcli_essid_list);
//	CW_FREE_OBJECT(dcli_list);

//	dbus_message_unref(reply);	
	
	return CMD_SUCCESS;				
}



DEFUN(show_manufacturer_oui_list_cmd_func,
		  show_manufacturer_oui_list_cmd,
		  "show legal manufacturer list ",
		  "show legal manufacturer oui list\n"
	 )
{
	int ret = -1;
	int num = 0;
	int i = 0;
	int length = 0;

	DCLI_AC_API_GROUP_ONE  *dcli_list = NULL;
	CW_CREATE_OBJECT_ERR(dcli_list, DCLI_AC_API_GROUP_ONE, return -1;);	
	dcli_list->dcli_attack_mac_list = NULL;
	dcli_list->dcli_essid_list = NULL;
	dcli_list->dcli_oui_list = NULL;

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

#if 0
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter,iter_array;
	DBusError err;

    int state = 1;
	unsigned char oui[DCLIAC_MAC_LEN];	
	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}
	ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath(index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_OUI_SHOW);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_OUI_SHOW);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret == 0)
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		vty_out(vty,"Legal manufacturer (OUI) list\n");
		for (i = 0; i < num; i++)
		{
			DBusMessageIter iter_struct;
			
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(oui[0]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(oui[1]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(oui[2]));
			dbus_message_iter_next(&iter_array);


			vty_out(vty,"#######\t%d\t%02X:%02X:%02X\n",i+1,oui[0],oui[1],oui[2]);

			dcli_add_manufacturer_oui(oui,&dcli_oui_list);
		}
	}
#endif
	dcli_list = dcli_ac_show_api_group_one(
			index,
			0,/*"show legal manufacturer list "*/
			0,
			0,
			&ret,
			0,
			&localid,
			//&dcli_list,
			dcli_dbus_connection,
			WID_DBUS_CONF_METHOD_OUI_SHOW
			);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
		vty_out(vty,"Legal manufacturer (OUI) list\n");
		if((dcli_list != NULL )&&(dcli_list->dcli_oui_list != NULL)){
			int len= 0;
			int j = 0;
				struct oui_node *head = NULL;
				len = dcli_list->dcli_oui_list->list_len;
				head = dcli_list->dcli_oui_list->oui_list;
			for(j=0;j<dcli_list->dcli_oui_list->list_len;j++){
				vty_out(vty,"\t%d\t%02X:%02X:%02X\n",j+1,head->oui[0],head->oui[1],head->oui[2]);
				head = head->next;
			}
		}	
		//CW_FREE_OBJECT(dcli_list->dcli_oui_list->oui_list);
		dcli_ac_free_fun(WID_DBUS_CONF_METHOD_OUI_SHOW,dcli_list);
		dcli_list = NULL;
	}
	else if(ret == WID_NO_WHITELIST)
	{
		vty_out(vty,"there is no legal manufacturer list \n");
	}
	else
	{
		vty_out(vty,"error %d \n",ret);
	}
	//CW_FREE_OBJECT(dcli_list->dcli_oui_list);
	//CW_FREE_OBJECT(dcli_list);
	//	dbus_message_unref(reply);	
	
	return CMD_SUCCESS;				
}



DEFUN(add_legal_manufacturer_func,
		  add_legal_manufacturer_cmd,
		  "add legal manufacturer  OUI",
		  "add legal manufacturer id\n"
		  "OUI xx:xx:xx\n"
	 )
{

	int ret;
	OUI_S ouiElem; 
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	ret = wid_parse_OUI((char*)argv[0],&ouiElem);
	if (ret<0) {
		vty_out(vty,"<error> Unknown OUI format.\n");
		return CMD_FAILURE;
	}

	/*//printf("oui:\t%02X:%02X:%02X\n",ouiElem.oui[0],ouiElem.oui[1],ouiElem.oui[2]);*/
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_ADD_OUI);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_ADD_OUI);*/
	dbus_error_init(&err);
	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,  &ouiElem.oui[0],
							 DBUS_TYPE_BYTE,  &ouiElem.oui[1],
							 DBUS_TYPE_BYTE,  &ouiElem.oui[2],
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," set manufacturer OUI %s successfully\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return CMD_SUCCESS;	
}

DEFUN(add_legal_essid_func,
		  add_legal_essid_cmd,
		  "add legal essid  ESSID",
		  "add legal essid\n"
	 )
{

	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	if (strlen(argv[0]) > 32)
	{
		vty_out(vty,"<error> essid is too long,out of the limit of 32\n");
		return CMD_SUCCESS;
	}

	char *essid=(char *)malloc(strlen(argv[0])+1);
	memset(essid,0,strlen(argv[0])+1);
	memcpy(essid,argv[0],strlen(argv[0]));

	/*//printf("essid:\t%s\n",essid);*/
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_ADD_ESSID);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_ADD_ESSID );*/
	dbus_error_init(&err);
	

	dbus_message_append_args(query,
							 DBUS_TYPE_STRING,&essid,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	free(essid);
	essid=NULL;

	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," set legal essid %s successfully\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return CMD_SUCCESS;	
}

DEFUN(add_attack_ap_mac_func,
		  add_attack_ap_mac_cmd,
		  "add attack ap mac MAC",
		  "wireless control\n"
		  "add attack ap MAC \n"
		  "MAC xx:xx:xx:xx:xx:xx\n"
	 )
{
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	WIDMACADDR  macaddr;
	ret = wid_parse_mac_addr((char *)argv[0],&macaddr);
	if (CMD_FAILURE == ret) {
		vty_out(vty,"<error> Unknown mac addr format.\n");
		return CMD_FAILURE;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_ADD_ATTACK_MAC);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_ADD_ATTACK_MAC);*/

	dbus_error_init(&err);
	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[0],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[1],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[2],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[3],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[4],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[5],
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	/*//printf("mac :: %02X:%02X:%02X:%02X:%02X:%02X\n",macaddr.macaddr[0],macaddr.macaddr[1],macaddr.macaddr[2],macaddr.macaddr[3],macaddr.macaddr[4],macaddr.macaddr[5]);*/
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," set attack ap mac successfully\n");
	}				
	else 
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return CMD_SUCCESS;			
}


DEFUN(del_legal_manufacturer_func,
		  del_legal_manufacturer_cmd,
		  "del legal manufacturer  OUI",
		  "del legal manufacturer id\n"
		  "OUI xx:xx:xx\n"
	 )
{

	int ret;
	OUI_S ouiElem; 
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	ret = wid_parse_OUI((char*)argv[0],&ouiElem);
	if (ret<0) {
		vty_out(vty,"<error> Unknown OUI format.\n");
		return CMD_FAILURE;
	}

	/*//printf("oui:\t%02X:%02X:%02X\n",ouiElem.oui[0],ouiElem.oui[1],ouiElem.oui[2]);*/
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_DEL_OUI);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_DEL_OUI);*/
	
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,  &ouiElem.oui[0],
							 DBUS_TYPE_BYTE,  &ouiElem.oui[1],
							 DBUS_TYPE_BYTE,  &ouiElem.oui[2],
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," del manufacturer OUI %s successfully\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return CMD_SUCCESS;	
}


DEFUN(del_legal_essid_func,
		  del_legal_essid_cmd,
		  "del legal essid  ESSID",
		  "del legal essid\n"
	 )
{

	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	if (strlen(argv[0]) > 32)
	{
		vty_out(vty,"<error> essid is too long,out of the limit of 32\n");
		return CMD_SUCCESS;
	}

	char *essid=(char *)malloc(strlen(argv[0])+1);
	memset(essid,0,strlen(argv[0])+1);
	memcpy(essid,argv[0],strlen(argv[0]));

	/*//printf("essid:\t%s\n",essid);*/
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_DEL_ESSID);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_DEL_ESSID );*/
	dbus_error_init(&err);
	

	dbus_message_append_args(query,
							 DBUS_TYPE_STRING,&essid,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	free(essid);
	essid=NULL;

	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," del legal essid %s successfully\n",argv[0]);
	}
	else if(ret == ESSID_NOT_EXIT)
	{
		vty_out(vty," essid not exist\n");
	}	
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return CMD_SUCCESS;	
}

DEFUN(del_attack_ap_mac_func,
		  del_attack_ap_mac_cmd,
		  "del attack ap mac MAC",
		  "wireless control\n"
		  "del attack ap MAC \n"
		  "MAC xx:xx:xx:xx:xx:xx\n"
	 )
{
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	WIDMACADDR  macaddr;
	ret = wid_parse_mac_addr((char *)argv[0],&macaddr);
	if (CMD_FAILURE == ret) {
		vty_out(vty,"<error> Unknown mac addr format.\n");
		return CMD_FAILURE;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_DEL_ATTACK_MAC);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_DEL_ATTACK_MAC);*/

	dbus_error_init(&err);
	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[0],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[1],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[2],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[3],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[4],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[5],
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	/*//printf("mac :: %02X:%02X:%02X:%02X:%02X:%02X\n",macaddr.macaddr[0],macaddr.macaddr[1],macaddr.macaddr[2],macaddr.macaddr[3],macaddr.macaddr[4],macaddr.macaddr[5]);*/
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," del attack ap mac successfully\n");
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return CMD_SUCCESS;			
}




DEFUN(show_wid_config_cmd_func,
	  show_wid_config_cmd,
	  "show wireless-control config [remote] [local] [PARAMETER]",
	  "show wireless-control configuration\n"
	  "wireless-control config\n"
	  "wireless-control config infomation\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;

	int ret = 0;
/*	int result = 0;
	int hw_version;
	int sw_version;
	char *hw_version_char;
	char *sw_version_char;
	char *ac_name;
	int sta_count;
	int max_wtp;
	int static_wtp;
	int force_mtu;
	int log_switch;
	int log__level =1;
	char loglevel[20];	
	char clog_swith[2][4] = {"OFF","ON"};
	int log_size;
	unsigned char uclev3_protocol = 1;
	char caauth_security[2][20] = {"CW_PRESHARED","CW_X509_CERTIFICATE"};
	unsigned char auth_security = 1;
	char calev3_protocol[2][8] = {"CW_IPv6","CW_IPv4"};
*/	
	DCLI_AC_API_GROUP_FIVE *wirelessconfig = NULL;
//	CW_CREATE_OBJECT_ERR(wirelessconfig, DCLI_AC_API_GROUP_FIVE, return NULL;); 
//	wirelessconfig->wireless_control = NULL;
	char clog_swith[2][4] = {"OFF","ON"};
	int log_size;
	unsigned char uclev3_protocol = 1;
	char caauth_security[2][20] = {"CW_PRESHARED","CW_X509_CERTIFICATE"};
	unsigned char auth_security = 1;
	char calev3_protocol[2][8] = {"CW_IPv6","CW_IPv4"};
	char en[] = "enable";
	char dis[] = "disable";
	char tranpwrctrlscope[2][4] = {"own","all"};	/*xiaodawei add, 20110115, for transmit power control scope*/
	char op[] = "open";
	char clo[] = "close";
	char ac_balance_method[3][8] = {"disable","number","flow"};
	char wtp_wids_policy[2][7] = {"no","forbid"};
	char radio_src_mgmt_countermeasures_mode[3][6] = {"ap","adhoc","all"};
//	char loglevel[20][][][][][][][] =  ;	
	
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	if((argc == 1)||(argc == 3)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 2){
		if (!strcmp(argv[0],"remote")){
			localid = 0;
		}else if(!strcmp(argv[0],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[0],"remote"))&&(!strcmp(argv[1],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[1],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[1], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}
#if 0
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WIDCONFIG);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_WIDCONFIG);*/
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);/*000*/
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(hw_version));/*001*/

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(sw_version));/*002*/
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(auth_security));/*003*/
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(uclev3_protocol));/*004*/

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(ac_name));/*005*/
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(sta_count));/*006*/
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(max_wtp));/*007*/
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(static_wtp));/*007	////////////*/
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(force_mtu));/*008*/
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(log_switch));/*009*/

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(log_size));/*010*/

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(log__level));/*011*/

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&(result));


	parse_daemonlog_level(log__level,loglevel);
	if(result == 2)
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(hw_version_char));

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(sw_version_char));

		
		vty_out(vty,"wireless-control config list summary\n");
		vty_out(vty,"==============================================================================\n");
		/*vty_out(vty,"hwversion:       %-20s\n",hw_version_char);
		//vty_out(vty,"swversion:       %-20s\n",sw_version_char);*/
		vty_out(vty,"auth_security:   %-20s\n",caauth_security[auth_security]);
		vty_out(vty,"protocol:        %-15s\n",calev3_protocol[uclev3_protocol]);
		/*vty_out(vty,"ac_name:         %-20s\n",ac_name);*/
		vty_out(vty,"max_sta:         %-8d\n",sta_count);
		vty_out(vty,"max_wtp:         %-8d\n",max_wtp);
		vty_out(vty,"static_wtp:      %-11d\n",static_wtp);
		/*vty_out(vty,"mtu:             %-10d\n",force_mtu);*/
		vty_out(vty,"log_switch:      %-12s\n",clog_swith[log_switch]);
		vty_out(vty,"log_level: 	 %-12s\n",loglevel);
		vty_out(vty,"log_size:        %-10d\n",log_size);
		vty_out(vty,"==============================================================================\n");

		return CMD_SUCCESS;	
	}
	
	vty_out(vty,"wireless-control config list summary\n");
	vty_out(vty,"==============================================================================\n");
	/*vty_out(vty,"hwversion:       %-10d\n",hw_version);
	//vty_out(vty,"swversion:       %-10d\n",sw_version);*/
	vty_out(vty,"auth_security:   %-20s\n",caauth_security[auth_security]);
	vty_out(vty,"protocol:        %-15s\n",calev3_protocol[uclev3_protocol]);
	/*vty_out(vty,"ac_name:         %-10s\n",ac_name);*/
	vty_out(vty,"max_sta:         %-8d\n",sta_count);
	vty_out(vty,"max_wtp:         %-8d\n",max_wtp);
	vty_out(vty,"static_wtp:      %-11d\n",static_wtp);
	/*vty_out(vty,"mtu:             %-10d\n",force_mtu);*/
	vty_out(vty,"log_switch:      %-12s\n",clog_swith[log_switch]);
	vty_out(vty,"log_level:      %-12s\n",loglevel);
	vty_out(vty,"log_size:        %-10d\n",log_size);
	vty_out(vty,"==============================================================================\n");

	/*vty_out(vty,"%-10s %-10s %-20s %-15s%-10s %-8s %-8s %-11s %-10s %-12s %-10s\n","HWversion","SWversion","auth_security","protocol","AC_name","max_sta","max_wtp", "static_wtp", "MTU","switch","log_size");
	
	//vty_out(vty,"%-10d %-10d %-20s %-15s%-10s %-8d %-8d %-11d %-10d %-12s %-10d\n",hw_version,sw_version,caauth_security[auth_security],calev3_protocol[uclev3_protocol],ac_name,sta_count,max_wtp,static_wtp,force_mtu,clog_swith[log_switch],log_size);*/
#endif
	if(vty->node != VIEW_NODE){
		wirelessconfig = dcli_ac_show_api_group_five(
			index,
			FIRST,/*"show wireless-control config"*/
			0,
			0,
			0,
			&ret,
			0,
			0,
			localid,
			//wirelessconfig,
			dcli_dbus_connection,
			WID_DBUS_CONF_METHOD_WIDCONFIG
			);
		if(ret == -1){
			cli_syslog_info("<error> failed get reply.\n");
		}
		else if(ret == 0){	
			if((wirelessconfig != NULL)&&(wirelessconfig->wireless_control != NULL)){
					wireless_config *head = NULL;
					head = wirelessconfig->wireless_control;
					parse_daemonlog_level(head->log__level,head->loglevel);
					vty_out(vty,"wireless-control config list summary\n");
					vty_out(vty,"==============================================================================\n");
					/*vty_out(vty,"hwversion:       %-20s\n",hw_version_char);
					//vty_out(vty,"swversion:       %-20s\n",sw_version_char);*/
					vty_out(vty,"auth_security:   %-20s\n",caauth_security[head->auth_security]);
					vty_out(vty,"protocol:        %-15s\n",calev3_protocol[head->uclev3_protocol]);
					/*vty_out(vty,"ac_name:         %-20s\n",ac_name);*/
					vty_out(vty,"max_sta:         %-8d\n",head->sta_count);
					vty_out(vty,"max_wtp:         %-8d\n",head->max_wtp);
					vty_out(vty, "gMaxWtps:        %-8d\n", head->gmaxwtps);
					vty_out(vty,"static_wtp:      %-11d\n",head->static_wtp);
					/*vty_out(vty,"mtu:             %-10d\n",force_mtu);*/
					vty_out(vty,"log_switch:      %-12s\n",clog_swith[head->log_switch]);
					vty_out(vty,"log_level: 	 %-12s\n",head->loglevel);
					vty_out(vty,"log_size:        %-10d\n",head->log_size);
					vty_out(vty,"ap statistics state: 	  %-10s\n",(head->apstaticstate == 1)?en:dis);
					vty_out(vty,"apstatic interval: 	  %-10u\n",head->apstatisticsinterval);
					vty_out(vty,"mac whitelist: 	  %-10s\n",(head->macfiltrflag == 1)?en:dis);
					vty_out(vty,"essid whitelist: 	%-12s\n",(head->essidfiltrflag == 1)?en:dis);
					vty_out(vty,"traplevel:        %-10d\n",head->trapflag);
					vty_out(vty,"ac extention infor switch:        %-10s\n",(head->g_ac_all_extention_information_switch == 1)?en:dis);
					/*xiaodawei add, 20110115*/
					vty_out(vty,"radio resource management:	%-10s\n",(head->radioresmgmt == 1)?en:dis);
					vty_out(vty,"transmit power control: 	 %-12s\n",(head->tranpowerctrl == 1)?op:clo);
					vty_out(vty,"transmit power control scope:        %-10s\n",tranpwrctrlscope[head->tranpwrctrlscope]);
					vty_out(vty,"auto_ap_login switch:	%-10s\n",(head->autoaploginswitch == 1)?en:dis);
					vty_out(vty,"auto_ap_login save_config_switch:	%-10s\n",(head->autoaplogin_saveconfigswitch == 1)?en:dis);			
					vty_out(vty,"wireless control monitor: %-10s\n",(head->wirelessctrlmonitor == 1)?en:dis);
					vty_out(vty,"wireless control sample: %-10s\n",(head->wirelessctrlsample == 1)?en:dis);
					vty_out(vty,"wid watch dog: 	 %-12s\n",(head->widwatchdog == 1)?op:clo);
					vty_out(vty,"ac balance method:        %-10s\n",ac_balance_method[head->ac_balance_method]);
					vty_out(vty,"ap hotreboot: %-10s\n",(head->ap_hotreboot == 1)?en:dis);
					vty_out(vty,"ap access through nat: %-10s\n",(head->ap_acc_through_nat == 1)?en:dis);
					vty_out(vty,"wtp wids policy:        %-10s\n",wtp_wids_policy[head->wtp_wids_policy]);
					vty_out(vty,"radio resource management countermeasures: %-10s\n",(head->radio_src_mgmt_countermeasures == 1)?en:dis);
					vty_out(vty,"radio resource management countermeasures mode:		 %-10s\n",radio_src_mgmt_countermeasures_mode[head->radio_src_mgmt_countermeasures_mode]);
					vty_out(vty,"wireless interface vmac: %-10s\n",(head->wireless_interface_vmac == 1)?en:dis);
					vty_out(vty,"wtp link detect: %-10s\n",(head->wtp_link_detect == 1)?en:dis);
					vty_out(vty,"wsm switch: %-10s\n",(head->wsm_switch == 1)?en:dis);
					vty_out(vty,"service tftp: %-10s\n",(head->service_tftp == 1)?en:dis);
					vty_out(vty,"service ftp: %-10s\n",(head->service_ftp == 1)?en:dis);
					/*xiaodawei add for trap switch, 20110115*/
					vty_out(vty,"ap_run_quit: %-10s\n",(head->ap_run_quit == 1)?en:dis);
					vty_out(vty,"ap_cpu_threshold: %-10s\n",(head->ap_cpu_threshold == 1)?en:dis);
					vty_out(vty,"ap_mem_threshold: %-10s\n",(head->ap_mem_threshold == 1)?en:dis);
					vty_out(vty,"ap_update_fail: %-10s\n",(head->ap_update_fail == 1)?en:dis);
					vty_out(vty,"rrm_change: %-10s\n",(head->rrm_change == 1)?en:dis);
					vty_out(vty,"rogue_ap_threshold: %-10s\n",(head->rogue_ap_threshold == 1)?en:dis);
					vty_out(vty,"rogue_terminal_threshold: %-10s\n",(head->rogue_terminal_threshold == 1)?en:dis);
					vty_out(vty,"rogue_device: %-10s\n",(head->rogue_device == 1)?en:dis);
					vty_out(vty,"wireless_interface_down: %-10s\n",(head->wireless_interface_down == 1)?en:dis);
					vty_out(vty,"channel_count_minor: %-10s\n",(head->channel_count_minor == 1)?en:dis);
					vty_out(vty,"channel_change: %-10s\n",(head->channel_change == 1)?en:dis);
					vty_out(vty,"rogue_ap: %-10s\n",(head->rogue_ap == 1)?en:dis);
					/*wcl add*/
					switch(head->country_code){
						case 0:
							vty_out(vty,"country-code : CN\n");
								break;
						case 1:
							vty_out(vty,"country-code : EU\n");
								break;
						case 2:
							vty_out(vty,"country-code : US\n");
								break;
						case 3:
							vty_out(vty,"country-code : JP\n");
								break;
						case 4:
							vty_out(vty,"country-code : FR\n");
								break;
						case 5:
							vty_out(vty,"country-code : ES\n");
								break;
						default:
							vty_out(vty,"country-code : %d\n",head->country_code);
								break;
					}
					/*end*/

					/*end of trap switch*/
					vty_out(vty,"==============================================================================\n");
			}
			//CW_FREE_OBJECT(wirelessconfig->wireless_control);		
			dcli_ac_free_fun_five(WID_DBUS_CONF_METHOD_WIDCONFIG,wirelessconfig);
		}else vty_out(vty,"ret:   %d\n",ret);
	//	dbus_message_unref(reply);	
	//	CW_FREE_OBJECT(wirelessconfig);	
	}

	if(vty->node == VIEW_NODE){
		//for remote hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}
	
		hansi_parameter:
				wirelessconfig = dcli_ac_show_api_group_five(
					profile,
					FIRST,/*"show wireless-control config"*/
					0,
					0,
					0,
					&ret,
					0,
					0,
					localid,
					dcli_dbus_connection,
					WID_DBUS_CONF_METHOD_WIDCONFIG
					);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0){	
					if((wirelessconfig != NULL)&&(wirelessconfig->wireless_control != NULL)){
							wireless_config *head = NULL;
							head = wirelessconfig->wireless_control;
							parse_daemonlog_level(head->log__level,head->loglevel);
							vty_out(vty,"wireless-control config list summary\n");
							vty_out(vty,"-----------------------------------------------------------------------------\n");
							vty_out(vty,"auth_security:   %-20s\n",caauth_security[head->auth_security]);
							vty_out(vty,"protocol:		  %-15s\n",calev3_protocol[head->uclev3_protocol]);
							vty_out(vty,"max_sta:		  %-8d\n",head->sta_count);
							vty_out(vty,"max_wtp:		  %-8d\n",head->max_wtp);
							vty_out(vty, "gMaxWtps:        %-8d\n", head->gmaxwtps);
							vty_out(vty,"static_wtp:	  %-11d\n",head->static_wtp);
							vty_out(vty,"log_switch:	  %-12s\n",clog_swith[head->log_switch]);
							vty_out(vty,"log_level: 	 %-12s\n",head->loglevel);
							vty_out(vty,"log_size:		  %-10d\n",head->log_size);
							vty_out(vty,"ap statistics state:	  %-10s\n",(head->apstaticstate == 1)?en:dis);
							vty_out(vty,"apstatic interval: 	  %-10u\n",head->apstatisticsinterval);
							vty_out(vty,"mac whitelist: 	  %-10s\n",(head->macfiltrflag == 1)?en:dis);
							vty_out(vty,"essid whitelist:	%-12s\n",(head->essidfiltrflag == 1)?en:dis);
							vty_out(vty,"traplevel: 	   %-10d\n",head->trapflag);
							vty_out(vty,"ac extention infor switch: 	   %-10s\n",(head->g_ac_all_extention_information_switch == 1)?en:dis);
							/*xiaodawei add, 20110115*/
							vty_out(vty,"radio resource management: %-10s\n",(head->radioresmgmt == 1)?en:dis);
							vty_out(vty,"transmit power control:	 %-12s\n",(head->tranpowerctrl == 1)?op:clo);
							vty_out(vty,"transmit power control scope:		  %-10s\n",tranpwrctrlscope[head->tranpwrctrlscope]);
							vty_out(vty,"auto_ap_login switch:	%-10s\n",(head->autoaploginswitch == 1)?en:dis);
							vty_out(vty,"auto_ap_login save_config_switch:	%-10s\n",(head->autoaplogin_saveconfigswitch == 1)?en:dis); 		
							vty_out(vty,"wireless control monitor: %-10s\n",(head->wirelessctrlmonitor == 1)?en:dis);
							vty_out(vty,"wireless control sample: %-10s\n",(head->wirelessctrlsample == 1)?en:dis);
							vty_out(vty,"wid watch dog: 	 %-12s\n",(head->widwatchdog == 1)?op:clo);
							vty_out(vty,"ac balance method: 	   %-10s\n",ac_balance_method[head->ac_balance_method]);
							vty_out(vty,"ap hotreboot: %-10s\n",(head->ap_hotreboot == 1)?en:dis);
							vty_out(vty,"ap access through nat: %-10s\n",(head->ap_acc_through_nat == 1)?en:dis);
							vty_out(vty,"wtp wids policy:		 %-10s\n",wtp_wids_policy[head->wtp_wids_policy]);
							vty_out(vty,"radio resource management countermeasures: %-10s\n",(head->radio_src_mgmt_countermeasures == 1)?en:dis);
							vty_out(vty,"radio resource management countermeasures mode:		 %-10s\n",radio_src_mgmt_countermeasures_mode[head->radio_src_mgmt_countermeasures_mode]);
							vty_out(vty,"wireless interface vmac: %-10s\n",(head->wireless_interface_vmac == 1)?en:dis);
							vty_out(vty,"wtp link detect: %-10s\n",(head->wtp_link_detect == 1)?en:dis);
							vty_out(vty,"wsm switch: %-10s\n",(head->wsm_switch == 1)?en:dis);
							vty_out(vty,"service tftp: %-10s\n",(head->service_tftp == 1)?en:dis);
							vty_out(vty,"service ftp: %-10s\n",(head->service_ftp == 1)?en:dis);
							/*xiaodawei add for trap switch, 20110115*/
							vty_out(vty,"ap_run_quit: %-10s\n",(head->ap_run_quit == 1)?en:dis);
							vty_out(vty,"ap_cpu_threshold: %-10s\n",(head->ap_cpu_threshold == 1)?en:dis);
							vty_out(vty,"ap_mem_threshold: %-10s\n",(head->ap_mem_threshold == 1)?en:dis);
							vty_out(vty,"ap_update_fail: %-10s\n",(head->ap_update_fail == 1)?en:dis);
							vty_out(vty,"rrm_change: %-10s\n",(head->rrm_change == 1)?en:dis);
							vty_out(vty,"rogue_ap_threshold: %-10s\n",(head->rogue_ap_threshold == 1)?en:dis);
							vty_out(vty,"rogue_terminal_threshold: %-10s\n",(head->rogue_terminal_threshold == 1)?en:dis);
							vty_out(vty,"rogue_device: %-10s\n",(head->rogue_device == 1)?en:dis);
							vty_out(vty,"wireless_interface_down: %-10s\n",(head->wireless_interface_down == 1)?en:dis);
							vty_out(vty,"channel_count_minor: %-10s\n",(head->channel_count_minor == 1)?en:dis);
							vty_out(vty,"channel_change: %-10s\n",(head->channel_change == 1)?en:dis);
							vty_out(vty,"rogue_ap: %-10s\n",(head->rogue_ap == 1)?en:dis);
							/*wcl add*/
							switch(head->country_code){
								case 0:
									vty_out(vty,"country-code : CN\n");
										break;
								case 1:
									vty_out(vty,"country-code : EU\n");
										break;
								case 2:
									vty_out(vty,"country-code : US\n");
										break;
								case 3:
									vty_out(vty,"country-code : JP\n");
										break;
								case 4:
									vty_out(vty,"country-code : FR\n");
										break;
								case 5:
									vty_out(vty,"country-code : ES\n");
										break;
								default:
									vty_out(vty,"country-code : %d\n",head->country_code);
										break;
							}
							/*end*/
			
							/*end of trap switch*/
					}
					dcli_ac_free_fun_five(WID_DBUS_CONF_METHOD_WIDCONFIG,wirelessconfig);
					wirelessconfig = NULL;
				}else vty_out(vty,"ret:   %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 2){
					return CMD_SUCCESS;
				}
			}
		}

		//for local hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 1;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

		local_hansi_parameter:
				wirelessconfig = dcli_ac_show_api_group_five(
				profile,
				FIRST,/*"show wireless-control config"*/
				0,
				0,
				0,
				&ret,
				0,
				0,
				localid,
				dcli_dbus_connection,
				WID_DBUS_CONF_METHOD_WIDCONFIG
				);
			    vty_out(vty,"==============================================================================\n");
			    vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
			    vty_out(vty,"-----------------------------------------------------------------------------\n");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0){	
					if((wirelessconfig != NULL)&&(wirelessconfig->wireless_control != NULL)){
						wireless_config *head = NULL;
						head = wirelessconfig->wireless_control;
						parse_daemonlog_level(head->log__level,head->loglevel);
						vty_out(vty,"wireless-control config list summary\n");
						vty_out(vty,"-----------------------------------------------------------------------------\n");
						vty_out(vty,"auth_security:   %-20s\n",caauth_security[head->auth_security]);
						vty_out(vty,"protocol:        %-15s\n",calev3_protocol[head->uclev3_protocol]);
						vty_out(vty,"max_sta:         %-8d\n",head->sta_count);
						vty_out(vty,"max_wtp:         %-8d\n",head->max_wtp);
						vty_out(vty, "gMaxWtps:        %-8d\n", head->gmaxwtps);
						vty_out(vty,"static_wtp:      %-11d\n",head->static_wtp);
						vty_out(vty,"log_switch:      %-12s\n",clog_swith[head->log_switch]);
						vty_out(vty,"log_level: 	 %-12s\n",head->loglevel);
						vty_out(vty,"log_size:        %-10d\n",head->log_size);
						vty_out(vty,"ap statistics state: 	  %-10s\n",(head->apstaticstate == 1)?en:dis);
						vty_out(vty,"apstatic interval: 	  %-10u\n",head->apstatisticsinterval);
						vty_out(vty,"mac whitelist: 	  %-10s\n",(head->macfiltrflag == 1)?en:dis);
						vty_out(vty,"essid whitelist: 	%-12s\n",(head->essidfiltrflag == 1)?en:dis);
						vty_out(vty,"traplevel:        %-10d\n",head->trapflag);
						vty_out(vty,"ac extention infor switch:        %-10s\n",(head->g_ac_all_extention_information_switch == 1)?en:dis);
						/*xiaodawei add, 20110115*/
						vty_out(vty,"radio resource management:	%-10s\n",(head->radioresmgmt == 1)?en:dis);
						vty_out(vty,"transmit power control: 	 %-12s\n",(head->tranpowerctrl == 1)?op:clo);
						vty_out(vty,"transmit power control scope:        %-10s\n",tranpwrctrlscope[head->tranpwrctrlscope]);
						vty_out(vty,"auto_ap_login switch:	%-10s\n",(head->autoaploginswitch == 1)?en:dis);
						vty_out(vty,"auto_ap_login save_config_switch:	%-10s\n",(head->autoaplogin_saveconfigswitch == 1)?en:dis);			
						vty_out(vty,"wireless control monitor: %-10s\n",(head->wirelessctrlmonitor == 1)?en:dis);
						vty_out(vty,"wireless control sample: %-10s\n",(head->wirelessctrlsample == 1)?en:dis);
						vty_out(vty,"wid watch dog: 	 %-12s\n",(head->widwatchdog == 1)?op:clo);
						vty_out(vty,"ac balance method:        %-10s\n",ac_balance_method[head->ac_balance_method]);
						vty_out(vty,"ap hotreboot: %-10s\n",(head->ap_hotreboot == 1)?en:dis);
						vty_out(vty,"ap access through nat: %-10s\n",(head->ap_acc_through_nat == 1)?en:dis);
						vty_out(vty,"wtp wids policy:        %-10s\n",wtp_wids_policy[head->wtp_wids_policy]);
						vty_out(vty,"radio resource management countermeasures: %-10s\n",(head->radio_src_mgmt_countermeasures == 1)?en:dis);
						vty_out(vty,"radio resource management countermeasures mode:		 %-10s\n",radio_src_mgmt_countermeasures_mode[head->radio_src_mgmt_countermeasures_mode]);
						vty_out(vty,"wireless interface vmac: %-10s\n",(head->wireless_interface_vmac == 1)?en:dis);
						vty_out(vty,"wtp link detect: %-10s\n",(head->wtp_link_detect == 1)?en:dis);
						vty_out(vty,"wsm switch: %-10s\n",(head->wsm_switch == 1)?en:dis);
						vty_out(vty,"service tftp: %-10s\n",(head->service_tftp == 1)?en:dis);
						vty_out(vty,"service ftp: %-10s\n",(head->service_ftp == 1)?en:dis);
						/*xiaodawei add for trap switch, 20110115*/
						vty_out(vty,"ap_run_quit: %-10s\n",(head->ap_run_quit == 1)?en:dis);
						vty_out(vty,"ap_cpu_threshold: %-10s\n",(head->ap_cpu_threshold == 1)?en:dis);
						vty_out(vty,"ap_mem_threshold: %-10s\n",(head->ap_mem_threshold == 1)?en:dis);
						vty_out(vty,"ap_update_fail: %-10s\n",(head->ap_update_fail == 1)?en:dis);
						vty_out(vty,"rrm_change: %-10s\n",(head->rrm_change == 1)?en:dis);
						vty_out(vty,"rogue_ap_threshold: %-10s\n",(head->rogue_ap_threshold == 1)?en:dis);
						vty_out(vty,"rogue_terminal_threshold: %-10s\n",(head->rogue_terminal_threshold == 1)?en:dis);
						vty_out(vty,"rogue_device: %-10s\n",(head->rogue_device == 1)?en:dis);
						vty_out(vty,"wireless_interface_down: %-10s\n",(head->wireless_interface_down == 1)?en:dis);
						vty_out(vty,"channel_count_minor: %-10s\n",(head->channel_count_minor == 1)?en:dis);
						vty_out(vty,"channel_change: %-10s\n",(head->channel_change == 1)?en:dis);
						vty_out(vty,"rogue_ap: %-10s\n",(head->rogue_ap == 1)?en:dis);
						/*wcl add*/
						switch(head->country_code){
							case 0:
								vty_out(vty,"country-code : CN\n");
									break;
							case 1:
								vty_out(vty,"country-code : EU\n");
									break;
							case 2:
								vty_out(vty,"country-code : US\n");
									break;
							case 3:
								vty_out(vty,"country-code : JP\n");
									break;
							case 4:
								vty_out(vty,"country-code : FR\n");
									break;
							case 5:
								vty_out(vty,"country-code : ES\n");
									break;
							default:
								vty_out(vty,"country-code : %d\n",head->country_code);
									break;
						}
						/*end*/

						/*end of trap switch*/
					}
					dcli_ac_free_fun_five(WID_DBUS_CONF_METHOD_WIDCONFIG,wirelessconfig);
					wirelessconfig = NULL;
				}else vty_out(vty,"ret:   %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 2){
					return CMD_SUCCESS;
				}
			}
		}
	}
	return CMD_SUCCESS;	
}

DEFUN(set_wid_hw_version_func,
	  set_wid_hw_version_cmd,
	  "set wireless-control hw version PARAMETER",
	  CONFIG_STR
	  "wireless-control config\n"
	  "wireless-control config\n"
	  "wireless-control hardware config\n"
	  "wireless-control hardware version config\n"
	  "wireless-control hardware version vaule\n"
	 )
{
	int ret;
	unsigned char * hwversion;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	
	hwversion = (char*)malloc(strlen(argv[0])+1);
	memset(hwversion, 0, strlen(argv[0])+1);
	memcpy(hwversion, argv[0], strlen(argv[0]));	

	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_HW_VERSION);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_HW_VERSION);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_STRING,&hwversion,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		free(hwversion);
		hwversion = NULL;

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"wireless-control set sw version %s successfully\n",hwversion);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);
	
	free(hwversion);
	hwversion = NULL;
	
	return CMD_SUCCESS;	

}

DEFUN(set_wid_sw_version_func,
	  set_wid_sw_version_cmd,
	  "set wireless-control sw version PARAMETER",
	  CONFIG_STR
	  "wireless-control config\n"
	  "wireless-control config\n"
	  "wireless-control software config\n"
	  "wireless-control software version config\n"
	  "wireless-control software version vaule\n"
	 )
{
	int ret;
	unsigned char * swversion;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	
	swversion = (char*)malloc(strlen(argv[0])+1);
	memset(swversion, 0, strlen(argv[0])+1);
	memcpy(swversion, argv[0], strlen(argv[0]));	

	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SW_VERSION);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SW_VERSION);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_STRING,&swversion,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		free(swversion);
		swversion = NULL;

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"wireless-control set sw version %s successfully\n",swversion);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);
	
	free(swversion);
	swversion = NULL;
	
	return CMD_SUCCESS;	
}

DEFUN(set_wid_lev3_protocol_func,
	  set_wid_lev3_protocol_cmd,
	  "set wireless-control lev3 protocol (IPv4|IPv6)",
	  CONFIG_STR
	  "wireless-control config\n"
	  "wireless-control config\n"
	  "wireless-control lev3 protocol config\n"
	  "wireless-control lev3 protocol config\n"
	  "wireless-control lev3 protocol vaule\n"
	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	unsigned char lev3protocol;
	int ret;

	if (!strcmp(argv[0],"IPv4"))
	{
		lev3protocol = 1;	
	}		
	else if (!strcmp(argv[0],"IPv6"))
	{
		/*lev3protocol = 0;*/
		vty_out(vty,"This version only surport IPv4,we will update later\n");
		return CMD_SUCCESS;
	}
	else
	{
		//printf("<error> input patameter should only be 'IPv4' or 'IPv6' *****\n");
		return CMD_SUCCESS;
	}

	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_LEV3_PROTOCOL);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_LEV3_PROTOCOL);*/


	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&lev3protocol,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		vty_out(vty,"wireless-control set layer3 protocol %s successfully\n",argv[0]);
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;		
	
}

DEFUN(set_wid_auth_security_func,
	  set_wid_auth_security_cmd,
	  "set wireless-control auth security (PRESHARED|X509_CERTIFCATE)",
	  CONFIG_STR
	  "wireless-control config\n"
	  "wireless-control config\n"
	  "wireless-control auth security config\n"
	  "wireless-control auth security config\n"
	  "wireless-control auth security vaule\n"
	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	unsigned char securitytype;
	int ret;

	if (!strcmp(argv[0],"X509_CERTIFCATE"))
	{
		securitytype = 1;	
	}		
	else if (!strcmp(argv[0],"PRESHARED"))
	{
		securitytype = 0;
	}
	else
	{
		vty_out(vty,"<error> input patameter should only be 'X509_CERTIFCATE' or 'PRESHARED' \n");
		return CMD_SUCCESS;
	}

	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SECURITY_TYPE);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SECURITY_TYPE);*/


	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&securitytype,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		vty_out(vty,"wireless-control set security type %s successfully\n",argv[0]);
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;		
	
}

DEFUN(set_wid_name_func,
	  set_wid_name_cmd,
	  "set wireless-control name PARAMETER",
	  CONFIG_STR
	  "wireless-control config\n"
	  "wireless-control config\n"
	  "wireless-control name config\n"
	  "wireless-control name vaule\n"
	 )
{
	int ret;
	unsigned char * acname;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	
	acname = (char*)malloc(strlen(argv[0])+1);
	memset(acname, 0, strlen(argv[0])+1);
	memcpy(acname, argv[0], strlen(argv[0]));	

	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AC_NAME);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AC_NAME);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_STRING,&acname,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		free(acname);
		acname = NULL;

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"wireless-control set ac name %s successfully\n",acname);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);
	
	free(acname);
	acname = NULL;
	
	return CMD_SUCCESS;		
}

DEFUN(set_wirelesscontrol_max_wtp_func,
	  set_wirelesscontrol_max_wtp_cmd,
	  "set wireless-control max wtp WTPNUM [SLOTID] [ISLOCALED] [INSTID]",
	  "wireless-control config\n"
	  "wireless-control config\n"
	  "wireless-control max wtp config\n"
	  "wireless-control max wtp config\n"
	  "wireless-control max wtp vaule\n"
	 )
{
	int ret;
	int wtpnums;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    wtpnums = atoi(argv[0]);
	//make robust code

	if((wtpnums < 15)||(wtpnums > WTP_NUM))
	{
		vty_out(vty,"<error> input parameter should be 15 to %d\n",WTP_NUM);
		return CMD_SUCCESS;
	}

	int localid = 0;
	int slot_id = HostSlotId;
	int index = 0;
	switch(vty->node)
	{
		case HANSI_NODE:
			if (argc > 1)
			{
				vty_out(vty, "<error> too many parameters\n");
				return CMD_SUCCESS;
			}
			index = (int)vty->index;
			slot_id = (int)vty->slotindex;
			localid = (int)vty->local;
			break;
		case LOCAL_HANSI_NODE:
			if (argc > 1)
			{
				vty_out(vty, "<error> too many parameters\n");
				return CMD_SUCCESS;
			}
			index = (int)vty->index;
			slot_id = (int)vty->slotindex;
			localid = (int)vty->local;
			break;
		case HIDDENDEBUG_NODE:
			if (argc != 4)
			{
				vty_out(vty, "<error> parameters' number must be 4\n");
				return CMD_SUCCESS;
			}
			localid = atoi(argv[2]);
			slot_id = atoi(argv[1]);
			index = atoi(argv[3]);
			break;
		default:
			break;
	}
	/*
	int localid = atoi(argv[2]);
	int slot_id = atoi(argv[1]);
	int index = atoi(argv[3]);
	*/
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_MAX_WTP);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_MAX_WTP);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpnums,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"wireless-control set max wtp %d successfully\n",wtpnums);
	}				
	else if(ret == WTP_LESS_THAN_STATIC_WTP)
	{
		vty_out(vty,"<error> wireless-control set max wtp must over current static wtp\n");
	}
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;		
}

DEFUN(set_wid_max_mtu_func,
	  set_wid_max_mtu_cmd,
	  "set wireless-control max mtu <500-1500>",
	  "wireless-control config\n"
	  "wireless-control config\n"
	  "wireless-control max mtu config\n"
	  "wireless-control max mtu config\n"
	  "wireless-control max mtu vaule\n"
	 )
{
	int ret;
	int maxmtu;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    maxmtu = atoi(argv[0]);
	/*make robust code*/

	if((maxmtu < 500)||(maxmtu > 1500))
	{
		vty_out(vty,"<error> input parameter should be 500 to 1500\n");
		return CMD_SUCCESS;
	}


	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_MAX_MTU);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_MAX_MTU);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&maxmtu,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"wireless-control set mtu %d successfully\n",maxmtu);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;		
}

DEFUN(set_wid_log_size_func,
	  set_wid_log_size_cmd,
	  "set wireless-control log size <1000000-500000000>",
	  "wireless-control config\n"
	  "wireless-control config\n"
	  "wireless-control log size config\n"
	  "wireless-control log size config\n"
	  "wireless-control log size vaule\n"
	 )
{
	int ret;
	int logsize;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    logsize = atoi(argv[0]);
	/*make robust code*/

	if((logsize < 1000000)||(logsize > 500000000))
	{
		vty_out(vty,"<error> input parameter should be 1000000 to 500000000\n");
		return CMD_SUCCESS;
	}


	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_LOG_SIZE);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_LOG_SIZE);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&logsize,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"wireless-control set log size %d successfully\n",logsize);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}

DEFUN(set_wid_log_switch_func,
	  set_wid_log_switch_cmd,
	  "set wireless-control log switch (ON|OFF)",
	  CONFIG_STR
	  "wireless-control config\n"
	  "wireless-control config\n"
	  "wireless-control log switch config\n"
	  "wireless-control log switch config\n"
	  "wireless-control log switch vaule\n"
	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	unsigned char logswitch;
	int ret;

	if (!strcmp(argv[0],"ON"))
	{
		logswitch = 1;	
	}		
	else if (!strcmp(argv[0],"OFF"))
	{
		logswitch = 0;
	}
	else
	{
		vty_out(vty,"<error> input patameter should only be 'ON' or 'OFF'\n");
		return CMD_SUCCESS;
	}

	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_LOG_SWITCH);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_LOG_SWITCH);*/


	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&logswitch,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		vty_out(vty,"wireless-control set log switch %s successfully\n",argv[0]);
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;			
}
/*sz20080917*/
DEFUN(set_wid_log_level_func,
	  set_wid_log_level_cmd,
	  "set wireless-control log level (info|debug|all)",
	  "wireless-control config\n"
	  "wireless-control config\n"
	  "wireless-control log level config\n"
	  "wireless-control log level config\n"
	  "wireless-control log level vaule\n"
	 )/*going to delete it*/
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    unsigned char widloglevel;

	/*if (!strcmp(argv[0],"none"))
	{
		widloglevel = 0;	
	}		
	else if (!strcmp(argv[0],"warning"))
	{
		widloglevel = 1;
	}
	else */if (!strcmp(argv[0],"info"))
	{
		widloglevel = 1;	
	}
	else if (!strcmp(argv[0],"debug"))
	{
		widloglevel = 8;	
	}
	else if (!strcmp(argv[0],"all"))
	{
		widloglevel = 15;	
	}
	else
	{
		/*//printf("******* input patameter error only with 'none' 'warning' 'info' 'debug' or 'all' *****\n");*/
		vty_out(vty,"<error> input patameter should only be 'info' 'debug' or 'all'\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_LOG_LEVEL);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_LOG_LEVEL);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&widloglevel,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"wireless-control set log level %d successfully\n",widloglevel);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}

DEFUN(set_wid_daemonlog_debug_open_func,
	  set_wid_daemonlog_debug_open_cmd,
	  "set wireless-control daemonlog (default|dbus|wtp|mb|all) debug (open|close)",
	  "wireless-control config\n"
	  "wireless-control daemonlog config\n"
	  "wireless-control daemonlog debug open|close\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    unsigned int daemonlogtype;
    unsigned int daemonloglevel;

	if (!strcmp(argv[0],"default"))
	{
		daemonlogtype = WID_DEFAULT;	
	}
	else if (!strcmp(argv[0],"dbus"))
	{
		daemonlogtype = WID_DBUS;	
	}
	else if (!strcmp(argv[0],"wtp"))
	{
		daemonlogtype = WID_WTPINFO;	
	}
	else if (!strcmp(argv[0],"mb"))
	{
		daemonlogtype = WID_MB;	
	}	
	else if (!strcmp(argv[0],"all"))
	{
		daemonlogtype = WID_ALL;	
	}
	else
	{
		vty_out(vty,"<error> input patameter should only be default|dbus|wtp|mb|all\n");
		return CMD_SUCCESS;
	}

	
	if (!strcmp(argv[1],"open"))
	{
		daemonloglevel = 7;	
	}
	else if (!strcmp(argv[1],"close"))
	{
		daemonloglevel = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter should only be 'open' or 'close'\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_DAEMONLOG_DEBUG_OPEN);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_DAEMONLOG_DEBUG_OPEN);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
					 		 DBUS_TYPE_UINT32,&daemonlogtype,
							 DBUS_TYPE_UINT32,&daemonloglevel,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"wireless-control set daemonlog %s debug %s successfully\n",argv[0],argv[1]);
	}				
	else
	{
		vty_out(vty,"error %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}

DEFUN(set_wid_trap_open_func,
	  set_wid_trap_open_cmd,
	  "set wireless-control trap level <0-25>",
	  "wireless-control config\n"
	  "wireless-control trap config\n"
	  "wireless-control trap level 1/2/3.../25   0/no trap.\n"
	 )
{
	int ret,ret2;

	DBusMessage *query, *reply, *query2,*reply2;	
	DBusMessageIter	 iter,iter2;
	DBusError err,err2;

    unsigned char trapflag = 0;

	 trapflag = (unsigned char)atoi(argv[0]);
	/*	
	if (!strcmp(argv[0],"open"))
	{
		trapflag = 1;	
	}
	else if (!strcmp(argv[0],"close"))
	{
		trapflag = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter should only be 'open' or 'close'\n");
		return CMD_SUCCESS;
	}*/

	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_TRAP_DEBUG_OPEN);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_TRAP_DEBUG_OPEN);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&trapflag,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"wireless-control set trap %s successfully\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"error %d\n",ret);
	}
		
	dbus_message_unref(reply);
	
	if(ret == 0)
	{
		
	/*	int index = 0;
		char BUSNAME[PATH_LEN];
		char OBJPATH[PATH_LEN];
		char INTERFACE[PATH_LEN];
		char METHOD[PATH_LEN];	
		if(vty->node == CONFIG_NODE){
			index = 0;
		}else if(vty->node == HANSI_NODE){
			index = vty->index;
		}*/
		memset(BUSNAME,0,PATH_LEN);
		memset(OBJPATH,0,PATH_LEN);
		memset(INTERFACE,0,PATH_LEN);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
		query2 = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_TRAP_OPEN);
		
	/*	query2 = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_TRAP_OPEN);*/
		dbus_error_init(&err2);
		dbus_message_append_args(query2,
						 DBUS_TYPE_BYTE,&trapflag,
						 DBUS_TYPE_INVALID);
		reply2 = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query2,-1, &err2);	
		dbus_message_unref(query2);
		if (NULL == reply2) {
			cli_syslog_info("<error> failed get reply.\n");
			if (dbus_error_is_set(&err2)) {
				cli_syslog_info("%s raised: %s",err2.name,err2.message);
				dbus_error_free_for_dcli(&err2);
			}
			return CMD_SUCCESS;
		}
		
		dbus_message_iter_init(reply2,&iter2);
		dbus_message_iter_get_basic(&iter2,&ret2);
		dbus_message_unref(reply2);
		if(ret2 != ASD_DBUS_SUCCESS){
			return CMD_SUCCESS;		
		}	
	}
	
	return CMD_SUCCESS;			
}

DEFUN(set_wid_trap_switch_able_func,
	  set_wid_trap_switch_able_cmd,
	  "set wireless-control trap (ap_run_quit|ap_cpu_threshold|ap_mem_threshold\
	  |ap_update_fail|rrm_change|rogue_ap_threshold|rogue_terminal_threshold|rogue_ap|rogue_device|wireless_interface_down\
	  |channel_count_minor|channel_change)\
	  switch (enable|disable)",
	  "wireless-control config\n"
	  "wireless-control trap config\n"
	  "wireless-control trap level 1/2/3.../25   0/no trap.\n"
	 )
{
	int ret,ret2;
	unsigned int trap_switch = 0;
	unsigned int trap_policy = 0;
	DBusMessage *query, *reply, *query2,*reply2;	
	DBusMessageIter	 iter,iter2;
	DBusError err,err2;

		

	/*parse first papatameter*/
	if (!strcmp(argv[0],"ap_run_quit"))
	{
		trap_switch = 1;	
	}
	else if (!strcmp(argv[0],"ap_cpu_threshold"))
	{
		trap_switch = 2;	
	}
	else if (!strcmp(argv[0],"ap_mem_threshold"))
	{
		trap_switch = 3;	
	}
	else if (!strcmp(argv[0],"ap_update_fail"))
	{
		trap_switch = 4;	
	}
	else if (!strcmp(argv[0],"rrm_change"))
	{
		trap_switch = 5;	
	}
	else if (!strcmp(argv[0],"rogue_ap_threshold"))
	{
		trap_switch = 6;	
	}
	else if (!strcmp(argv[0],"rogue_terminal_threshold"))
	{
		trap_switch = 7;	
	}
	else if (!strcmp(argv[0],"rogue_device"))
	{
		trap_switch = 8;	
	}
	else if (!strcmp(argv[0],"wireless_interface_down"))
	{
		trap_switch = 9;	
	}
	else if (!strcmp(argv[0],"channel_count_minor"))
	{
		trap_switch = 10;	
	}
	else if (!strcmp(argv[0],"channel_change"))
	{
		trap_switch = 11;	
	}
	else if (!strcmp(argv[0],"rogue_ap"))
	{
		trap_switch = 12;	
	}
	else
	{
		vty_out(vty,"<error> the first input patameter error.\n");
		return CMD_SUCCESS;
	}
	/*parse second papatameter*/
	if (!strcmp(argv[1],"enable"))
	{
		trap_policy = 1;	
	}
	else if (!strcmp(argv[1],"disable"))
	{
		trap_policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter should only be 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_TRAP_SWITCH_ABLE);
	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_TRAP_SWITCH_ABLE);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
 							DBUS_TYPE_UINT32,&trap_switch,
 						    DBUS_TYPE_UINT32,&trap_policy,
							DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"wireless-control set trap %s switch %s successfully\n",argv[0],argv[1]);
	}				
	else
	{
		vty_out(vty,"error %d\n",ret);
	}
		
	dbus_message_unref(reply);
	
	
	return CMD_SUCCESS;			
}

DEFUN(show_wid_trap_switch_info_func,
	show_wid_trap_switch_info_cmd,
	"show wireless-control trap switch infomation [remote] [local] [PARAMETER]",
	"wireless-control config\n"
	"wireless-control trap config\n"
	"wireless-control trap level 1/2/3.../25   0/no trap.\n"
	"trap infomation\n"
	"trap infomation\n"
	"'remote' or 'local' hansi\n"
	"'remote' or 'local' hansi\n"
	"slotid-instid\n"
   )
{
	WID_TRAP_SWITCH *INFO = NULL;	
	int ret = 0;
	char dis[] = "close";
	char en[] = "open";
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	if((argc == 1)||(argc == 3)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 2){
		if (!strcmp(argv[0],"remote")){
			localid = 0;
		}else if(!strcmp(argv[0],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[0],"remote"))&&(!strcmp(argv[1],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[1],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[1], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}

	if(vty->node != VIEW_NODE){
		INFO = (typeof(INFO))dcli_ac_show_wtp_trap_switch(localid,index,&ret,dcli_dbus_connection);
		
		vty_out(vty,"wireless-control config trap switch info\n");
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"ap_run_quit_trap_switch:		%-10s\n",(INFO->dcli_ap_run_quit_trap_switch == 0)?dis:en);
		vty_out(vty,"ap_cpu_trap_switch:			%-10s\n",(INFO->dcli_ap_cpu_trap_switch == 0)?dis:en);
		vty_out(vty,"ap_mem_trap_switch:   			%-10s\n",(INFO->dcli_ap_mem_trap_switch == 0)?dis:en);
		vty_out(vty,"update_fail_trap_switch:		%-10s\n",(INFO->dcli_flash_write_fail_trap_switch == 0)?dis:en);
		vty_out(vty,"rrm_change_trap_switch: 		%-10s\n",(INFO->dcli_rrm_change_trap_switch == 0)?dis:en);
		vty_out(vty,"rogue_ap_threshold_switch:	    %-10s\n",(INFO->dcli_rogue_ap_threshold_switch == 0)?dis:en);
		vty_out(vty,"rogue_terminal_switch:		    %-10s\n",(INFO->dcli_channel_terminal_interference_switch == 0)?dis:en);
		vty_out(vty,"rogue_device_switch:	        %-10s\n",(INFO->dcli_channel_device_interference_switch == 0)?dis:en);
		vty_out(vty,"wireless_interface_down_switch:%-10s\n",(INFO->dcli_wireless_interface_down_switch == 0)?dis:en);
		vty_out(vty,"channel_count_minor_switch: 	%-10s\n",(INFO->dcli_channel_count_minor_switch == 0)?dis:en);
		vty_out(vty,"channel_change_switch: 	    %-10s\n",(INFO->dcli_channel_change_switch == 0)?dis:en);
		vty_out(vty,"rogue ap switch                %-10s\n",(INFO->dcli_rogue_ap_switch== 0)?dis:en);
		vty_out(vty,"==============================================================================\n");
	}
	
	if(vty->node == VIEW_NODE){
		//for remote hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

		hansi_parameter:
				INFO = (typeof(INFO))dcli_ac_show_wtp_trap_switch(localid,profile,&ret,dcli_dbus_connection);
				
			    vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d     wireless-control config trap switch info\n",slot_id,profile);
			    vty_out(vty,"-----------------------------------------------------------------------------\n");
				vty_out(vty,"ap_run_quit_trap_switch:		%-10s\n",(INFO->dcli_ap_run_quit_trap_switch == 0)?dis:en);
				vty_out(vty,"ap_cpu_trap_switch:			%-10s\n",(INFO->dcli_ap_cpu_trap_switch == 0)?dis:en);
				vty_out(vty,"ap_mem_trap_switch:   			%-10s\n",(INFO->dcli_ap_mem_trap_switch == 0)?dis:en);
				vty_out(vty,"update_fail_trap_switch:		%-10s\n",(INFO->dcli_flash_write_fail_trap_switch == 0)?dis:en);
				vty_out(vty,"rrm_change_trap_switch: 		%-10s\n",(INFO->dcli_rrm_change_trap_switch == 0)?dis:en);
				vty_out(vty,"rogue_ap_threshold_switch:	    %-10s\n",(INFO->dcli_rogue_ap_threshold_switch == 0)?dis:en);
				vty_out(vty,"rogue_terminal_switch:		    %-10s\n",(INFO->dcli_channel_terminal_interference_switch == 0)?dis:en);
				vty_out(vty,"rogue_device_switch:	        %-10s\n",(INFO->dcli_channel_device_interference_switch == 0)?dis:en);
				vty_out(vty,"wireless_interface_down_switch:%-10s\n",(INFO->dcli_wireless_interface_down_switch == 0)?dis:en);
				vty_out(vty,"channel_count_minor_switch: 	%-10s\n",(INFO->dcli_channel_count_minor_switch == 0)?dis:en);
				vty_out(vty,"channel_change_switch: 	    %-10s\n",(INFO->dcli_channel_change_switch == 0)?dis:en);
				vty_out(vty,"rogue ap switch                %-10s\n",(INFO->dcli_rogue_ap_switch== 0)?dis:en);
				vty_out(vty,"==============================================================================\n");
				if(argc == 2){
					return CMD_SUCCESS;
				}
			}
		}

	//for local hansi info
	for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		localid = 1;
		
		for (profile = 1; profile < MAX_INSTANCE; profile++) 
		{
			instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
			if (INSTANCE_NO_CREATED == instRun) {
				continue;
			}
	
	local_hansi_parameter:
			INFO = (typeof(INFO))dcli_ac_show_wtp_trap_switch(localid,profile,&ret,dcli_dbus_connection);
			
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"local hansi %d-%d     wireless-control config trap switch info\n",slot_id,profile);
			vty_out(vty,"-----------------------------------------------------------------------------\n");
			vty_out(vty,"ap_run_quit_trap_switch:		%-10s\n",(INFO->dcli_ap_run_quit_trap_switch == 0)?dis:en);
			vty_out(vty,"ap_cpu_trap_switch:			%-10s\n",(INFO->dcli_ap_cpu_trap_switch == 0)?dis:en);
			vty_out(vty,"ap_mem_trap_switch:   			%-10s\n",(INFO->dcli_ap_mem_trap_switch == 0)?dis:en);
			vty_out(vty,"update_fail_trap_switch:		%-10s\n",(INFO->dcli_flash_write_fail_trap_switch == 0)?dis:en);
			vty_out(vty,"rrm_change_trap_switch: 		%-10s\n",(INFO->dcli_rrm_change_trap_switch == 0)?dis:en);
			vty_out(vty,"rogue_ap_threshold_switch:	    %-10s\n",(INFO->dcli_rogue_ap_threshold_switch == 0)?dis:en);
			vty_out(vty,"rogue_terminal_switch:		    %-10s\n",(INFO->dcli_channel_terminal_interference_switch == 0)?dis:en);
			vty_out(vty,"rogue_device_switch:	        %-10s\n",(INFO->dcli_channel_device_interference_switch == 0)?dis:en);
			vty_out(vty,"wireless_interface_down_switch:%-10s\n",(INFO->dcli_wireless_interface_down_switch == 0)?dis:en);
			vty_out(vty,"channel_count_minor_switch: 	%-10s\n",(INFO->dcli_channel_count_minor_switch == 0)?dis:en);
			vty_out(vty,"channel_change_switch: 	    %-10s\n",(INFO->dcli_channel_change_switch == 0)?dis:en);
			vty_out(vty,"rogue ap switch                %-10s\n",(INFO->dcli_rogue_ap_switch== 0)?dis:en);
			vty_out(vty,"==============================================================================\n");
			if(argc == 2){
				return CMD_SUCCESS;
			}
			}
		}
	}
	return CMD_SUCCESS; 
}
DEFUN(set_ap_scanning_func,
		  set_ap_scanning_cmd,
		  "set radio resource management (enable|disable)",
		  "ap rm config\n"
		  "ap scanning mode enable|disable\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    int policy = 0;

	
	if (!strcmp(argv[0],"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[0],"disable"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_SCANNING);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AP_SCANNING);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," set ap scanning %s successfully\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}

DEFUN(set_ap_scanning_report_interval_func,
		  set_ap_scanning_report_interval_cmd,
	 	  "set radio resource management report interval TIME",
		  "ap config\n"
		  "ap report interval config\n"
		  "ap scanning report interval value\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    int time = 0;

	time = atoi(argv[0]);
	
	if (time < 30 || time > 32767)  /*fengwenchao change 3600 to 32767*/
	{
	
		vty_out(vty,"<error> input patameter should be 30 to 32767\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_SCANNING_REPORT_INTERVAL);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AP_SCANNING_REPORT_INTERVAL);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&time,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," set ap scanning report interval %s successfully\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}

DEFUN(update_ap_scanning_info_func,
		  update_ap_scanning_info_cmd,
		  "update ap scanning info",
		  "ap config\n"
		  "ap update scanning report\n"
		  "ap update scanning report\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    int state = 1;

	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_UPDATE_AP_SCANNING_INFO);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_UPDATE_AP_SCANNING_INFO);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," update ap scanning info successfully\n");
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}

DEFUN(set_wirelesscontrol_whitelist_func,
		  set_wirelesscontrol_whitelist_cmd,
		  "set mac MAC whitelist",
		  "wireless control\n"
		  "add MAC into whitelist\n"
		  "MAC xx:xx:xx:xx:xx:xx\n"
	 )
{
	int ret;
	WIDMACADDR macaddr; 
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	ret = wid_parse_mac_addr((char *)argv[0],&macaddr);
	if (CMD_FAILURE == ret) {
		vty_out(vty,"<error> Unknown mac addr format.\n");
		return CMD_FAILURE;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WHITELIST);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_WHITELIST);*/
	dbus_error_init(&err);
	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[0],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[1],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[2],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[3],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[4],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[5],
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	/*//printf("mac :: %02X:%02X:%02X:%02X:%02X:%02X\n",macaddr.macaddr[0],macaddr.macaddr[1],macaddr.macaddr[2],macaddr.macaddr[3],macaddr.macaddr[4],macaddr.macaddr[5]);*/
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," set mac MAC whitelist successfully\n");
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return CMD_SUCCESS;			
}

DEFUN(set_wirelesscontrol_blacklist_func,
		  set_wirelesscontrol_blacklist_cmd,
		  "set mac MAC blacklist",
		  "wireless control\n"
		  "add MAC into blacklist\n"
		  "MAC xx:xx:xx:xx:xx:xx\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    WIDMACADDR macaddr;

	ret = wid_parse_mac_addr((char *)argv[0],&macaddr);
	if (CMD_FAILURE == ret) {
		vty_out(vty,"<error> Unknown mac addr format.\n");
		return CMD_FAILURE;
	}


	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_BLACKLIST);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_BLACKLIST);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[0],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[1],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[2],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[3],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[4],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[5],
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	/*//printf("mac :: %02X:%02X:%02X:%02X:%02X:%02X\n",macaddr.macaddr[0],macaddr.macaddr[1],macaddr.macaddr[2],macaddr.macaddr[3],macaddr.macaddr[4],macaddr.macaddr[5]);*/
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," set mac MAC blacklist successfully\n");
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}

DEFUN(delete_wirelesscontrol_whitelist_func,
		  delete_wirelesscontrol_whitelist_cmd,
		  "delete mac MAC whitelist",
		  "wireless control\n"
		  "delete MAC into whitelist\n"
		  "MAC xx:xx:xx:xx:xx:xx\n"
	 )
{
	int ret;
	WIDMACADDR macaddr; 
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	ret = wid_parse_mac_addr((char *)argv[0],&macaddr);
	if (CMD_FAILURE == ret) {
		vty_out(vty,"<error> Unknown mac addr format.\n");
		return CMD_FAILURE;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WHITELIST_DELETE);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_WHITELIST_DELETE);*/
	dbus_error_init(&err);
	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[0],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[1],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[2],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[3],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[4],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[5],
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	/*//printf("mac :: %02X:%02X:%02X:%02X:%02X:%02X\n",macaddr.macaddr[0],macaddr.macaddr[1],macaddr.macaddr[2],macaddr.macaddr[3],macaddr.macaddr[4],macaddr.macaddr[5]);*/
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," delete mac MAC whitelist successfully\n");
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return CMD_SUCCESS;			
}

DEFUN(delete_wirelesscontrol_blacklist_func,
		  delete_wirelesscontrol_blacklist_cmd,
		  "delete mac MAC blacklist",
		  "wireless control\n"
		  "delete MAC into blacklist\n"
		  "MAC xx:xx:xx:xx:xx:xx\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    WIDMACADDR macaddr;

	ret = wid_parse_mac_addr((char *)argv[0],&macaddr);
	if (CMD_FAILURE == ret) {
		vty_out(vty,"<error> Unknown mac addr format.\n");
		return CMD_FAILURE;
	}

	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_BLACKLIST_DELETE);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_BLACKLIST_DELETE);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[0],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[1],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[2],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[3],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[4],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[5],
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	/*//printf("mac :: %02X:%02X:%02X:%02X:%02X:%02X\n",macaddr.macaddr[0],macaddr.macaddr[1],macaddr.macaddr[2],macaddr.macaddr[3],macaddr.macaddr[4],macaddr.macaddr[5]);*/
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," delete mac MAC blacklist successfully\n");
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}

DEFUN(show_rogue_ap_list_func,
		  show_rogue_ap_list_cmd,
		  "show rogue ap list",
		  "show rogue ap list\n"
		  "show rogue ap list\n"
		  "show rogue ap list\n"
	 )
{
	int ret;
	int num = 0;
	int i = 0;
	int length = 0;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter,iter_array;
	DBusError err;

    int state = 1;
	DCLI_AC_API_GROUP_TWO *dcli_list = NULL;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#if 0	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_ROGUE_AP_LIST);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_ROGUE_AP_LIST);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret == 0 )
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		vty_out(vty,"detection rouge ap list \n");
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"%-17s %-4s %-7s %-4s %-5s %-7s %-5s %-12s %-10s\n","mac","rate","channel","rssi","noise","bea_int","capab","essid","ie_info");
		for (i = 0; i < num; i++)
		{
			DBusMessageIter iter_struct;
			
			rouge_ap = (struct Neighbor_AP_ELE *)malloc(sizeof(struct Neighbor_AP_ELE));
			if((rouge_ap == NULL))
			{
				//printf("##malloc memory fail #\n");
				break;
			}

			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[0]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[1]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[2]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[3]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[4]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[5]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->Rate));
			dbus_message_iter_next(&iter_struct);	

			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->Channel));
			dbus_message_iter_next(&iter_struct);				
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->RSSI));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->NOISE));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BEACON_INT));
			dbus_message_iter_next(&iter_struct);	
			
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->status));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->opstatus));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->capabilityinfo));
			dbus_message_iter_next(&iter_struct);

					
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->ESSID));
			dbus_message_iter_next(&iter_struct);


			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->IEs_INFO));
			dbus_message_iter_next(&iter_array);


			vty_out(vty,"%02X:%02X:%02X:%02X:%02X:%02X %-4d %-7d %-4d %-5d %-7d %-5d %-12s %-9s\n",\
				rouge_ap->BSSID[0],rouge_ap->BSSID[1],rouge_ap->BSSID[2],rouge_ap->BSSID[3],rouge_ap->BSSID[4],\
				rouge_ap->BSSID[5],rouge_ap->Rate,rouge_ap->Channel,rouge_ap->RSSI,rouge_ap->NOISE,rouge_ap->BEACON_INT,\
				rouge_ap->capabilityinfo,rouge_ap->ESSID,rouge_ap->IEs_INFO);

				if(dcli_add_rogue_ap_ele_fun(create_ap_info,rouge_ap) == -1)
				{
					CW_FREE_OBJECT(rouge_ap->ESSID);
					CW_FREE_OBJECT(rouge_ap->IEs_INFO);
				
					CW_FREE_OBJECT(rouge_ap);
				}


			//free(rouge_ap);		
			//rouge_ap = NULL;

		}
		vty_out(vty,"==============================================================================\n");
	}
#endif
	dcli_list = dcli_ac_show_api_group_two(
		index,
		FIRST,/*"show rogue ap list"*/
		0,
		0,
		0,
		&ret,
		0,
		&localid,
		//dcli_list,		
		dcli_dbus_connection,
		WID_DBUS_CONF_METHOD_ROGUE_AP_LIST
		);

	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
		vty_out(vty,"detection rouge ap list \n");
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"%-17s %-4s %-7s %-4s %-5s %-7s %-5s %-12s %-10s\n","mac","rate","channel","rssi","noise","bea_int","capab","essid","ie_info");
		if(dcli_list->rouge_ap_list->neighborapInfos != NULL){
			int len;
			int j= 0;
			struct Neighbor_AP_ELE *head;
			len = dcli_list->rouge_ap_list->neighborapInfosCount;
			head = dcli_list->rouge_ap_list->neighborapInfos;
			for(j=0;j<len;j++){
				if(head != NULL){
					vty_out(vty,"%02X:%02X:%02X:%02X:%02X:%02X %-4d %-7d %-4d %-5d %-7d %-5d %-12s %-9s\n",\
						head->BSSID[0],head->BSSID[1],head->BSSID[2],head->BSSID[3],head->BSSID[4],\
						head->BSSID[5],head->Rate,head->Channel,head->RSSI,head->NOISE,head->BEACON_INT,\
						head->capabilityinfo,head->ESSID,head->IEs_INFO);
					head = head->next;
				}
			}
		}
		vty_out(vty,"==============================================================================\n");

		//CW_FREE_OBJECT(dcli_list->rouge_ap_list->neighborapInfos);
		dcli_ac_free_fun_two(WID_DBUS_CONF_METHOD_ROGUE_AP_LIST,dcli_list);
		dcli_list = NULL;
	}		
	else if(ret == WID_AP_SCANNING_DISABLE)
	{
		vty_out(vty,"radio resource managment is disable please enable first\n");
	}
	else
	{	
		//vty_out(vty,"ret is %d .\n",ret);
		vty_out(vty,"good luck there is no rouge ap\n");
	}
	//CW_FREE_OBJECT(dcli_list->rouge_ap_list);
	//CW_FREE_OBJECT(dcli_list);
		
//	dbus_message_unref(reply);	
	
	return CMD_SUCCESS;			
}

DEFUN(show_rogue_ap_trap_threshold_func,
		  show_rogue_ap_trap_threshold_cmd,
		  "show rogue ap trap threshold [remote] [local] [PARAMETER]",
		  SHOW_STR
		  "show rogue ap trap threshold\n"
		  "show rogue ap trap threshold\n"
		  "show rogue ap trap threshold\n"
		  "trap threshold\n"
		  "'remote' or 'local' hansi\n"
		  "'remote' or 'local' hansi\n"
		  "slotid-instid\n"
	 )
{
	int ret;
	int num = 0;
	
//	DBusMessage *query, *reply;	
//	DBusMessageIter	 iter;
//	DBusError err;

	DCLI_AC_API_GROUP_FIVE *rogue_trap = NULL;

    int state = 1;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	if((argc == 1)||(argc == 3)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 2){
		if (!strcmp(argv[0],"remote")){
			localid = 0;
		}else if(!strcmp(argv[0],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[0],"remote"))&&(!strcmp(argv[1],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[1],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[1], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}
#if 0	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_ROGUE_AP_THRESHOLD);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_ROGUE_AP_THRESHOLD);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret == 0 )
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
	
		
		vty_out(vty,"==============================================================================\n");
		
		vty_out(vty,"current rogue ap trap threshold is %d\n",num);
		
		vty_out(vty,"==============================================================================\n");
	}
#endif	
	if(vty->node != VIEW_NODE){
		rogue_trap = dcli_ac_show_api_group_five(
			index,
			SECOND,/*"show rogue ap trap threshold"*/
			0,
			0,
			0,
			&ret,
			0,
			0,
			localid,
			//rogue_trap,
			dcli_dbus_connection,
			WID_DBUS_CONF_METHOD_ROGUE_AP_THRESHOLD
			);
		if(ret == -1){
			cli_syslog_info("<error> failed get reply.\n");
		}
		else if(ret == 0){	
			vty_out(vty,"==============================================================================\n");
			
			vty_out(vty,"current rogue ap trap threshold is %d\n",rogue_trap->num);
			
			vty_out(vty,"==============================================================================\n");
			dcli_ac_free_fun_five(WID_DBUS_CONF_METHOD_ROGUE_AP_THRESHOLD,rogue_trap);
			rogue_trap = NULL;
		}else 
			vty_out(vty,"ret is %d .\n",ret);
	//	dbus_message_unref(reply);	
	}

	if(vty->node == VIEW_NODE){
		//for remote hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}
	
		hansi_parameter:

				rogue_trap = dcli_ac_show_api_group_five(
					profile,
					SECOND,/*"show rogue ap trap threshold"*/
					0,
					0,
					0,
					&ret,
					0,
					0,
					localid,
					dcli_dbus_connection,
					WID_DBUS_CONF_METHOD_ROGUE_AP_THRESHOLD
					);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0){	
					vty_out(vty,"current rogue ap trap threshold is %d\n",rogue_trap->num);
					
					dcli_ac_free_fun_five(WID_DBUS_CONF_METHOD_ROGUE_AP_THRESHOLD,rogue_trap);
					rogue_trap = NULL;
				}else 
					vty_out(vty,"ret is %d .\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 2){
					return CMD_SUCCESS;
				}
			}
		}

		//for local hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 1;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}
		
			local_hansi_parameter:
				rogue_trap = dcli_ac_show_api_group_five(
					index,
					SECOND,/*"show rogue ap trap threshold"*/
					0,
					0,
					0,
					&ret,
					0,
					0,
					localid,
					dcli_dbus_connection,
					WID_DBUS_CONF_METHOD_ROGUE_AP_THRESHOLD
					);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0){	
					vty_out(vty,"current rogue ap trap threshold is %d\n",rogue_trap->num);
					
					dcli_ac_free_fun_five(WID_DBUS_CONF_METHOD_ROGUE_AP_THRESHOLD,rogue_trap);
					rogue_trap = NULL;
				}else 
					vty_out(vty,"ret is %d .\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 2){
					return CMD_SUCCESS;
				}
			}
		}
	}

	return CMD_SUCCESS;			
}

DEFUN(show_rogue_ap_list_v1_func,
		  show_rogue_ap_list_v1_cmd,
		  "show rogue ap list v1",
		  "show rogue ap list v1\n"
		  "show rogue ap list v1 \n"
		  "show rogue ap list v1\n"
	 )
{
	int ret;
	int num = 0;
	int i = 0;
	int length = 0;

	DCLI_AC_API_GROUP_TWO *dcli_list = NULL;

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#if 0	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter,iter_array;
	DBusError err;

    int state = 1;
	struct Neighbor_AP_ELE *rouge_ap = NULL;	

	
	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_ROGUE_AP_LIST_V1);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_ROGUE_AP_LIST_V1);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret == 0 )
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		vty_out(vty,"detection rouge ap list \n");
		vty_out(vty,"==============================================================================\n");
		/*vty_out(vty,"%-17s %-26s %-26s %-8s %-8s\n","mac","fst_dtct_tm","fst_dtct_tm","encrp_type","policy");*/
		for (i = 0; i < num; i++)
		{
			DBusMessageIter iter_struct;
			
			rouge_ap = (struct Neighbor_AP_ELE *)malloc(sizeof(struct Neighbor_AP_ELE));
			if((rouge_ap == NULL))
			{
				//printf("##malloc memory fail #\n");
				break;
			}

			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[0]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[1]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[2]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[3]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[4]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[5]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->Rate));
			dbus_message_iter_next(&iter_struct);	

			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->Channel));
			dbus_message_iter_next(&iter_struct);				
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->RSSI));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->NOISE));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BEACON_INT));
			dbus_message_iter_next(&iter_struct);	
			
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->status));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->opstatus));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->capabilityinfo));
			dbus_message_iter_next(&iter_struct);

					
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->ESSID));
			dbus_message_iter_next(&iter_struct);



			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->IEs_INFO));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->fst_dtc_tm));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->lst_dtc_tm));
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->encrp_type));
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->polcy));
			dbus_message_iter_next(&iter_array);

			/*time(&rouge_ap->lst_dtc_tm);*/

			vty_out(vty,"Mac: %02X:%02X:%02X:%02X:%02X:%02X\n",\
				rouge_ap->BSSID[0],rouge_ap->BSSID[1],rouge_ap->BSSID[2],rouge_ap->BSSID[3],rouge_ap->BSSID[4],\
				rouge_ap->BSSID[5]);
			vty_out(vty,"fst_dtct_tm: %s",ctime(&rouge_ap->fst_dtc_tm));
			vty_out(vty,"lst_dtct_tm: %s",ctime(&rouge_ap->lst_dtc_tm));
			vty_out(vty,"encrp type: %d\n",rouge_ap->encrp_type);
			vty_out(vty,"policy: %d\n",rouge_ap->polcy);
			vty_out(vty,"############################################\n");
			free(rouge_ap);		
			rouge_ap = NULL;

		}
		vty_out(vty,"==============================================================================\n");
	}
#endif
	dcli_list = dcli_ac_show_api_group_two(
		index,
		SECOND,/*"show rogue ap list v1"*/
		0,
		0,
		0,
		&ret,
		0,
		&localid,
		//dcli_list,		
		dcli_dbus_connection,
		WID_DBUS_CONF_METHOD_ROGUE_AP_LIST_V1
		);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
		vty_out(vty,"WTP_Online_num: %d\n",dcli_list->rouge_ap_list->wtp_online_num);  //fengwenchao add 20110402
		vty_out(vty,"detection rouge ap list \n");
		vty_out(vty,"==============================================================================\n");
		if(dcli_list->rouge_ap_list->neighborapInfos != NULL){
			int len;
			int j= 0;
			struct Neighbor_AP_ELE *head;
			len = dcli_list->rouge_ap_list->neighborapInfosCount;
			head = dcli_list->rouge_ap_list->neighborapInfos;
			for(j=0;j<len;j++){
				vty_out(vty,"Mac: %02X:%02X:%02X:%02X:%02X:%02X\n",\
					head->BSSID[0],head->BSSID[1],head->BSSID[2],head->BSSID[3],head->BSSID[4],\
					head->BSSID[5]);
				vty_out(vty,"fst_dtct_tm: %s",ctime(&head->fst_dtc_tm));
				vty_out(vty,"lst_dtct_tm: %s",ctime(&head->lst_dtc_tm));
				vty_out(vty,"encrp type: %d\n",head->encrp_type);
				vty_out(vty,"policy: %d\n",head->polcy);
				/*fengwenchao add 20110402*/
				vty_out(vty,"RogueAPRssi: %d\n",head->RSSI);
				vty_out(vty,"rogueStaChannel: %d\n",head->Channel);
				vty_out(vty,"rogueStaAttackStatus: %s\n",(head->RogueAPAttackedStatus == 1)?"true":"false");
				vty_out(vty,"rogueStaToIgnore: %s\n",(head->RogueAPToIgnore == 1)?"true":"false");
				vty_out(vty,"RogueAPSSID: %s\n",head->ESSID);
				/*fengwenchao add end*/
				vty_out(vty,"############################################\n");
				head = head->next;
			}
		}

		vty_out(vty,"==============================================================================\n");
		//CW_FREE_OBJECT(dcli_list->rouge_ap_list->neighborapInfos);
		dcli_ac_free_fun_two(WID_DBUS_CONF_METHOD_ROGUE_AP_LIST_V1,dcli_list);
		dcli_list = NULL;
	}
	else if(ret == WID_AP_SCANNING_DISABLE)
	{
		vty_out(vty,"radio resource managment is disable please enable first\n");
	}
	else
	{
		vty_out(vty,"good luck there is no rouge ap\n");
	}

	//CW_FREE_OBJECT(dcli_list->rouge_ap_list);
	//CW_FREE_OBJECT(dcli_list);
//	dbus_message_unref(reply);	
	
	return CMD_SUCCESS;			
}
		


DEFUN(show_rogue_ap_list_bywtpid_func,
		  show_rogue_ap_list_bywtpid_cmd,
		  "show rogue ap list bywtp ID",
		  "show rogue ap list\n"
		  "show rogue ap list bywtp ID\n"
		  "wtpid value\n"
	 )
{
	int ret = -1;
	int num = 0;
	int i = 0;
	int length = 0;
	
	DCLI_AC_API_GROUP_TWO *dcli_list = NULL;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	int wtpid = 0;

	wtpid = atoi(argv[0]);
	if ((wtpid <= 0) || (wtpid >= WTP_NUM)) {
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_FAILURE;
		}
	
#if 0
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter,iter_array;
	DBusError err;

    int state = 1;
	int wtpid = 0;
	struct Neighbor_AP_ELE *rouge_ap = NULL;

	wtpid = atoi(argv[0]);
	if ((wtpid <= 0) || (wtpid >= WTP_NUM)) {
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_FAILURE;
		}
	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_ROGUE_AP_LIST_BYWTPID);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_ROGUE_AP_LIST_BYWTPID);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);


	if(ret == 0 )
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		vty_out(vty,"wtp %d detection rouge ap list \n",wtpid);
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"%-17s %-4s %-7s %-4s %-5s %-7s %-5s %-12s %-10s\n","mac","rate","channel","rssi","noise","bea_int","capab","essid","ie_info");
		for (i = 0; i < num; i++)
		{
			DBusMessageIter iter_struct;
			
			rouge_ap = (struct Neighbor_AP_ELE *)malloc(sizeof(struct Neighbor_AP_ELE));
			if((rouge_ap == NULL))
			{
				//printf("##malloc memory fail #\n");
				break;
			}

			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[0]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[1]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[2]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[3]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[4]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[5]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->Rate));
			dbus_message_iter_next(&iter_struct);	

			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->Channel));
			dbus_message_iter_next(&iter_struct);				
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->RSSI));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->NOISE));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BEACON_INT));
			dbus_message_iter_next(&iter_struct);	
			
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->status));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->opstatus));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->capabilityinfo));
			dbus_message_iter_next(&iter_struct);

					
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->ESSID));
			dbus_message_iter_next(&iter_struct);


			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->IEs_INFO));
			dbus_message_iter_next(&iter_array);


			vty_out(vty,"%02X:%02X:%02X:%02X:%02X:%02X %-4d %-7d %-4d %-5d %-7d %-5d %-12s %-9s\n",\
				rouge_ap->BSSID[0],rouge_ap->BSSID[1],rouge_ap->BSSID[2],rouge_ap->BSSID[3],rouge_ap->BSSID[4],\
				rouge_ap->BSSID[5],rouge_ap->Rate,rouge_ap->Channel,rouge_ap->RSSI,rouge_ap->NOISE,rouge_ap->BEACON_INT,\
				rouge_ap->capabilityinfo,rouge_ap->ESSID,rouge_ap->IEs_INFO);

			free(rouge_ap);		
			rouge_ap = NULL;

		}
		vty_out(vty,"==============================================================================\n");
	}
#endif
	dcli_list = dcli_ac_show_api_group_two(
		index,
		THIRD,/*"show rogue ap list bywtp ID"*/
		wtpid,
		0,
		0,
		&ret,
		0,
		&localid,
		//dcli_list,
		dcli_dbus_connection,
		WID_DBUS_CONF_METHOD_ROGUE_AP_LIST_BYWTPID
		);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
		vty_out(vty,"wtp %d detection rouge ap list \n",wtpid);
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"%-17s %-4s %-7s %-4s %-5s %-7s %-5s %-12s %-10s\n","mac","rate","channel","rssi","noise","bea_int","capab","essid","ie_info");
		if(dcli_list->rouge_ap_list->neighborapInfos != NULL){
			int len;
			int j= 0;
			struct Neighbor_AP_ELE *head;
			len = dcli_list->rouge_ap_list->neighborapInfosCount;
			head = dcli_list->rouge_ap_list->neighborapInfos;
			for(j=0;j<len;j++){
				vty_out(vty,"%02X:%02X:%02X:%02X:%02X:%02X %-4d %-7d %-4d %-5d %-7d %-5d %-12s %-9s\n",\
					head->BSSID[0],head->BSSID[1],head->BSSID[2],head->BSSID[3],head->BSSID[4],\
					head->BSSID[5],head->Rate,head->Channel,head->RSSI,head->NOISE,head->BEACON_INT,\
					head->capabilityinfo,head->ESSID,head->IEs_INFO);
				head = head->next;
			}
			vty_out(vty,"==============================================================================\n");
		}
		//CW_FREE_OBJECT(dcli_list->rouge_ap_list->neighborapInfos);
		dcli_ac_free_fun_two(WID_DBUS_CONF_METHOD_ROGUE_AP_LIST_BYWTPID,dcli_list);
		dcli_list = NULL;
	}
	else if (ret == WTP_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> wtp %d does not exist\n",wtpid);
	}
	else if(ret == WID_AP_SCANNING_DISABLE)
	{
		vty_out(vty,"radio resource managment is disable please enable first\n");
	}
	else if(ret == WTP_ID_LARGE_THAN_MAX)
	{
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
	}
	else
	{
		vty_out(vty,"good luck there is no rouge ap\n");
	}
	//CW_FREE_OBJECT(dcli_list->rouge_ap_list);
	//CW_FREE_OBJECT(dcli_list);
//	dbus_message_unref(reply);	
	
	return CMD_SUCCESS;			
}
//fengwenchao add 20101221
DEFUN(show_neighbor_ap_list_func,
		  show_neighbor_ap_list_cmd,
		  "show neighbor ap list of all wtp",
		  "show neighbor ap list\n"
		  "show neighbor ap list\n"
		  "wtpid value\n"
	 )
{
	int ret;
	int ret1;
	//int num = 0;
	int i = 0;
	int j = 0;
	int k = 0;
	int length = 0;
	int index = 0;
	int wtp_num = 0;

	struct allwtp_neighborap *neighborap = NULL;
	struct allwtp_neighborap *showneighborap = NULL;
	//struct allwtp_neighborap_radioinfo *radioshow = NULL;
	//struct Neighbor_AP_ELE *neighshow = NULL;
	
	int localid = 1;
	int slot_id = HostSlotId;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	neighborap = show_neighbor_ap_list_cmd_allap(localid,index,dcli_dbus_connection,&wtp_num,&ret,&ret1);

	

	if((neighborap != NULL)&&(0 == ret))
	{
		vty_out(vty,"WTPnum : %d\n",wtp_num);	
		vty_out(vty,"========================================================================== \n");
		for (i = 0; i < wtp_num; i++)
			{
				if(showneighborap == NULL)
					showneighborap = neighborap->allwtp_neighborap_list;
				else 
					showneighborap = showneighborap->next;
				if(showneighborap == NULL)
					break;
				
				vty_out(vty,"\n");
				vty_out(vty,"------------------------------WTP information------------------------------\n");
				vty_out(vty,"WTPID:   %-5d  \t\t\t\t",showneighborap->wtpid);
				vty_out(vty,"WTP MAC:  %02X:%02X:%02X:%02X:%02X:%02X \n",
				showneighborap->WTPmac[0],showneighborap->WTPmac[1],showneighborap->WTPmac[2],
				showneighborap->WTPmac[3],showneighborap->WTPmac[4],showneighborap->WTPmac[5]);

				struct allwtp_neighborap_radioinfo *radioshow = NULL;
				for(j = 0; j < showneighborap->radio_num;j++)
				{
		
					if(radioshow == NULL)
						{
                   			radioshow=showneighborap->radioinfo_head;
					
						}
					else
						{
                   			radioshow=radioshow->next;
				
						}
					if(radioshow == NULL){
				
						break;
						}

					vty_out(vty,"--------------------------wtpWirelessIfIndex information------------------------\n");
					vty_out(vty,"wtpWirelessIfIndex :  %d\n",radioshow->wtpWirelessIfIndex);
					vty_out(vty,"================================================================================\n");
					vty_out(vty,"================================================================================\n");
					vty_out(vty,"\n");
					if((radioshow->failreason != 0)&&(radioshow->rouge_ap_count == 0))
						{
							vty_out(vty,"AP scanning disable or have no neighbroAP \n");
						}
					else
						{
							struct Neighbor_AP_ELE *neighshow = NULL;
							vty_out(vty,"detection neighbor ap list \n");
							vty_out(vty,"==============================================================================\n");
							vty_out(vty,"%-17s %-5s %-4s %-7s %-4s %-5s %-7s %-5s %-12s\n","mac","wtpid","rate","channel","rssi","noise","bea_int","capab","essid");
							for(k = 0; k < radioshow->rouge_ap_count;k++)
								{
									//vty_out(vty,"radioshow->rouge_ap_count = %d\n",radioshow->rouge_ap_count);
									if(neighshow == NULL)
									{
                   						neighshow=radioshow->neighborapInfos_head;
									
									}
									else
									{
                   						neighshow=neighshow->next;
									
									}
									if(neighshow == NULL){
									
										break;
									}

									vty_out(vty,"%02X:%02X:%02X:%02X:%02X:%02X %-5d %-4d %-7d %-4d %-5d %-7d %-5d %-12s\n",\
									neighshow->BSSID[0],neighshow->BSSID[1],neighshow->BSSID[2],neighshow->BSSID[3],neighshow->BSSID[4],\
									neighshow->BSSID[5],neighshow->wtpid,neighshow->Rate,neighshow->Channel,neighshow->RSSI,neighshow->NOISE,neighshow->BEACON_INT,\
									neighshow->capabilityinfo,neighshow->ESSID);								
								}
							vty_out(vty,"==============================================================================\n");
						}
							/*if(neighshow != NULL)
								{
									neighshow = NULL;
								}*/
				}
				
				/*if(radioshow != NULL){
					radioshow = NULL;
				}*/

			}
	
	}
	else
		vty_out(vty,"<error>   ret = %d\n",ret);
	dcli_free_allwtp_neighbor_ap(neighborap);
		return CMD_SUCCESS;	
}
//fengwenchao add end

DEFUN(show_neighbor_ap_list_bywtpid_func,
		  show_neighbor_ap_list_bywtpid_cmd,
		  "show neighbor ap list bywtp ID [remote] [local] [PARAMETER]",
		  SHOW_STR
		  "show neighbor ap list\n"
		  "show neighbor ap list bywtp ID\n"
		  "ap list\n"
		  "bywtpid WTPID\n"
		  "wtpid value\n"
		  "'remote' or 'local' hansi\n"
		  "'remote' or 'local' hansi\n"
		  "slotid-instid\n"
	 )
{
	int ret = -1;
	int num = 0;
	int i = 0;
	int length = 0;
	int wtpid = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	wtpid = atoi(argv[0]);
	if ((wtpid <= 0) || (wtpid >= WTP_NUM)) {
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_FAILURE;
		}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	DCLI_AC_API_GROUP_TWO *dcli_list = NULL;

	if((argc == 2)||(argc == 4)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 3){
		if (!strcmp(argv[1],"remote")){
			localid = 0;
		}else if(!strcmp(argv[1],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[1],"remote"))&&(!strcmp(argv[2],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[2],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[2], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}
#if 0
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter,iter_array;
	DBusError err;

    int state = 1;
	int wtpid = 0;
	struct Neighbor_AP_ELE *rouge_ap = NULL;

	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST_BYWTPID);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST_BYWTPID);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);


	if(ret == 0 )
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		vty_out(vty,"wtp %d detection neighbor ap list \n",wtpid);
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"%-17s %-5s %-4s %-7s %-4s %-5s %-7s %-5s %-12s\n","mac","wtpid","rate","channel","rssi","noise","bea_int","capab","essid");
		for (i = 0; i < num; i++)
		{
			DBusMessageIter iter_struct;
			
			rouge_ap = (struct Neighbor_AP_ELE *)malloc(sizeof(struct Neighbor_AP_ELE));
			if((rouge_ap == NULL))
			{
				//printf("##malloc memory fail #\n");
				break;
			}

			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[0]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[1]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[2]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[3]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[4]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[5]));
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->wtpid));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->Rate));
			dbus_message_iter_next(&iter_struct);	

			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->Channel));
			dbus_message_iter_next(&iter_struct);				
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->RSSI));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->NOISE));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BEACON_INT));
			dbus_message_iter_next(&iter_struct);	
			
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->status));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->opstatus));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->capabilityinfo));
			dbus_message_iter_next(&iter_struct);

					
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->ESSID));
			dbus_message_iter_next(&iter_struct);


			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->IEs_INFO));
			dbus_message_iter_next(&iter_array);


			vty_out(vty,"%02X:%02X:%02X:%02X:%02X:%02X %-5d %-4d %-7d %-4d %-5d %-7d %-5d %-12s\n",\
				rouge_ap->BSSID[0],rouge_ap->BSSID[1],rouge_ap->BSSID[2],rouge_ap->BSSID[3],rouge_ap->BSSID[4],\
				rouge_ap->BSSID[5],rouge_ap->wtpid,rouge_ap->Rate,rouge_ap->Channel,rouge_ap->RSSI,rouge_ap->NOISE,rouge_ap->BEACON_INT,\
				rouge_ap->capabilityinfo,rouge_ap->ESSID);

			free(rouge_ap);		
			rouge_ap = NULL;

		}
		vty_out(vty,"==============================================================================\n");
	}
#endif
	if(vty->node != VIEW_NODE){
		dcli_list = dcli_ac_show_api_group_two(
			index,
			FOURTH,/*"show neighbor ap list bywtp ID"*/
			wtpid,
			0,
			0,
			&ret,
			0,
			&localid,
			//dcli_list,
			dcli_dbus_connection,
			WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST_BYWTPID
			);
		if(ret == -1){
			cli_syslog_info("<error> failed get reply.\n");
		}
		else if(ret == 0){	
			vty_out(vty,"wtp %d detection neighbor ap list \n",wtpid);
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"%-17s %-5s %-4s %-7s %-4s %-5s %-7s %-5s %-12s\n","mac","wtpid","rate","channel","rssi","noise","bea_int","capab","essid");
			if(dcli_list->rouge_ap_list->neighborapInfos != NULL){
				int len;
				int j= 0;
				struct Neighbor_AP_ELE *head;
				len = dcli_list->rouge_ap_list->neighborapInfosCount;
				head = dcli_list->rouge_ap_list->neighborapInfos;
				for(j=0;j<len;j++){
					vty_out(vty,"%02X:%02X:%02X:%02X:%02X:%02X %-5d %-4d %-7d %-4d %-5d %-7d %-5d %-12s\n",\
						head->BSSID[0],head->BSSID[1],head->BSSID[2],head->BSSID[3],head->BSSID[4],\
						head->BSSID[5],head->wtpid,head->Rate,head->Channel,head->RSSI,head->NOISE,head->BEACON_INT,\
						head->capabilityinfo,head->ESSID);
					head = head->next;
				}
				vty_out(vty,"==============================================================================\n");
			}
			//CW_FREE_OBJECT(dcli_list->rouge_ap_list->neighborapInfos);
			dcli_ac_free_fun_two(WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST_BYWTPID,dcli_list);
			dcli_list = NULL;
		}
		else if (ret == WTP_ID_NOT_EXIST)
		{
			vty_out(vty,"<error> wtp %d does not exist\n",wtpid);
		}
		else if(ret == WID_AP_SCANNING_DISABLE)
		{
			vty_out(vty,"radio resource managment is disable please enable first\n");
		}
		else if(ret == WTP_ID_LARGE_THAN_MAX)
		{
			vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
		}
		else
		{
			vty_out(vty,"sorry there is no neighbor ap\n");
		}
		//CW_FREE_OBJECT(dcli_list->rouge_ap_list);
		//CW_FREE_OBJECT(dcli_list);
	//	dbus_message_unref(reply);	
	}

	if(vty->node == VIEW_NODE){
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			//for remote hansi info
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}
	
		 hansi_parameter:
				dcli_list = dcli_ac_show_api_group_two(
					profile,
					FOURTH,/*"show neighbor ap list bywtp ID"*/
					wtpid,
					0,
					0,
					&ret,
					0,
					&localid,
					dcli_dbus_connection,
					WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST_BYWTPID
					);
			    vty_out(vty,"==============================================================================\n");
			    vty_out(vty,"hansi %d-%d\n",slot_id,profile);
			    vty_out(vty,"-----------------------------------------------------------------------------\n");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0){	
					vty_out(vty,"wtp %d detection neighbor ap list \n",wtpid);
					vty_out(vty,"-------------------------------------------------------------------------\n");
					vty_out(vty,"%-17s %-5s %-4s %-7s %-4s %-5s %-7s %-5s %-12s\n","mac","wtpid","rate","channel","rssi","noise","bea_int","capab","essid");
					if(dcli_list->rouge_ap_list->neighborapInfos != NULL){
						int len;
						int j= 0;
						struct Neighbor_AP_ELE *head;
						len = dcli_list->rouge_ap_list->neighborapInfosCount;
						head = dcli_list->rouge_ap_list->neighborapInfos;
						for(j=0;j<len;j++){
							vty_out(vty,"%02X:%02X:%02X:%02X:%02X:%02X %-5d %-4d %-7d %-4d %-5d %-7d %-5d %-12s\n",\
								head->BSSID[0],head->BSSID[1],head->BSSID[2],head->BSSID[3],head->BSSID[4],\
								head->BSSID[5],head->wtpid,head->Rate,head->Channel,head->RSSI,head->NOISE,head->BEACON_INT,\
								head->capabilityinfo,head->ESSID);
							head = head->next;
						}
					}
					dcli_ac_free_fun_two(WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST_BYWTPID,dcli_list);
					dcli_list = NULL;
				}
				else if (ret == WTP_ID_NOT_EXIST)
				{
					vty_out(vty,"<error> wtp %d does not exist\n",wtpid);
				}
				else if(ret == WID_AP_SCANNING_DISABLE)
				{
					vty_out(vty,"radio resource managment is disable please enable first\n");
				}
				else if(ret == WTP_ID_LARGE_THAN_MAX)
				{
					vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
				}
				else
				{
					vty_out(vty,"sorry there is no neighbor ap\n");
				}
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}

		//for local hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 1;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}
		
			local_hansi_parameter:
				dcli_list = dcli_ac_show_api_group_two(
					profile,
					FOURTH,/*"show neighbor ap list bywtp ID"*/
					wtpid,
					0,
					0,
					&ret,
					0,
					&localid,
					dcli_dbus_connection,
					WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST_BYWTPID
					);
			    vty_out(vty,"==============================================================================\n");
			    vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
			    vty_out(vty,"-----------------------------------------------------------------------------\n");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0){	
					vty_out(vty,"wtp %d detection neighbor ap list \n",wtpid);
					vty_out(vty,"-------------------------------------------------------------------------\n");
					vty_out(vty,"%-17s %-5s %-4s %-7s %-4s %-5s %-7s %-5s %-12s\n","mac","wtpid","rate","channel","rssi","noise","bea_int","capab","essid");
					if(dcli_list->rouge_ap_list->neighborapInfos != NULL){
						int len;
						int j= 0;
						struct Neighbor_AP_ELE *head;
						len = dcli_list->rouge_ap_list->neighborapInfosCount;
						head = dcli_list->rouge_ap_list->neighborapInfos;
						for(j=0;j<len;j++){
							vty_out(vty,"%02X:%02X:%02X:%02X:%02X:%02X %-5d %-4d %-7d %-4d %-5d %-7d %-5d %-12s\n",\
								head->BSSID[0],head->BSSID[1],head->BSSID[2],head->BSSID[3],head->BSSID[4],\
								head->BSSID[5],head->wtpid,head->Rate,head->Channel,head->RSSI,head->NOISE,head->BEACON_INT,\
								head->capabilityinfo,head->ESSID);
							head = head->next;
						}
					}
					dcli_ac_free_fun_two(WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST_BYWTPID,dcli_list);
					dcli_list = NULL;
				}
				else if (ret == WTP_ID_NOT_EXIST)
				{
					vty_out(vty,"<error> wtp %d does not exist\n",wtpid);
				}
				else if(ret == WID_AP_SCANNING_DISABLE)
				{
					vty_out(vty,"radio resource managment is disable please enable first\n");
				}
				else if(ret == WTP_ID_LARGE_THAN_MAX)
				{
					vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
				}
				else
				{
					vty_out(vty,"sorry there is no neighbor ap\n");
				}
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}
	}
	
	return CMD_SUCCESS;			
}

DEFUN(show_neighbor_ap_list_bywtpid2_func,
		  show_neighbor_ap_list_bywtpid2_cmd,
		  "show neighbor ap list2 bywtp ID",
		  "show neighbor ap list2\n"
		  "show neighbor ap list2 bywtp ID\n"
		  "wtpid value\n"
	 )
{
	int ret = -1;
	int num = 0;
	int i = 0;
	int length = 0;
	int wtpid = 0;
	wtpid = atoi(argv[0]);
	if ((wtpid <= 0) || (wtpid >= WTP_NUM)) {
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_FAILURE;
		}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	DCLI_AC_API_GROUP_TWO *dcli_list = NULL;
#if 0
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter,iter_array;
	DBusError err;

    int state = 1;
	int wtpid = 0;
	struct Neighbor_AP_ELE *rouge_ap = NULL;

	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST_BYWTPID);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST_BYWTPID);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);


	if(ret == 0 )
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		vty_out(vty,"wtp %d detection neighbor ap list \n",wtpid);
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"%-17s %-5s %-4s %-7s %-4s %-5s %-7s %-5s %-12s\n","mac","wtpid","rate","channel","rssi","noise","bea_int","capab","essid");
		for (i = 0; i < num; i++)
		{
			DBusMessageIter iter_struct;
			
			rouge_ap = (struct Neighbor_AP_ELE *)malloc(sizeof(struct Neighbor_AP_ELE));
			if((rouge_ap == NULL))
			{
				//printf("##malloc memory fail #\n");
				break;
			}

			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[0]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[1]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[2]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[3]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[4]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BSSID[5]));
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->wtpid));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->Rate));
			dbus_message_iter_next(&iter_struct);	

			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->Channel));
			dbus_message_iter_next(&iter_struct);				
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->RSSI));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->NOISE));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->BEACON_INT));
			dbus_message_iter_next(&iter_struct);	
			
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->status));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->opstatus));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->capabilityinfo));
			dbus_message_iter_next(&iter_struct);

					
			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->ESSID));
			dbus_message_iter_next(&iter_struct);


			dbus_message_iter_get_basic(&iter_struct,&(rouge_ap->IEs_INFO));
			dbus_message_iter_next(&iter_array);


			vty_out(vty,"%02X:%02X:%02X:%02X:%02X:%02X %-5d %-4d %-7d %-4d %-5d %-7d %-5d %-12s\n",\
				rouge_ap->BSSID[0],rouge_ap->BSSID[1],rouge_ap->BSSID[2],rouge_ap->BSSID[3],rouge_ap->BSSID[4],\
				rouge_ap->BSSID[5],rouge_ap->wtpid,rouge_ap->Rate,rouge_ap->Channel,rouge_ap->RSSI,rouge_ap->NOISE,rouge_ap->BEACON_INT,\
				rouge_ap->capabilityinfo,rouge_ap->ESSID);

			free(rouge_ap);		
			rouge_ap = NULL;

		}
		vty_out(vty,"==============================================================================\n");
	}
#endif
	dcli_list = dcli_ac_show_api_group_two(
		index,
		FOURTH,/*"show neighbor ap list bywtp ID"*/
		wtpid,
		0,
		0,
		&ret,
		0,
		&localid,
		//dcli_list,
		dcli_dbus_connection,
		WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST_BYWTPID2
		);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
		vty_out(vty,"wtp %d detection neighbor ap list \n",wtpid);
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"%-17s %-5s %-4s %-7s %-4s %-5s %-7s %-5s %-12s\n","mac","wtpid","rate","channel","rssi","noise","bea_int","capab","essid");
		if(dcli_list->rouge_ap_list->neighborapInfos != NULL){
			int len;
			int j= 0;
			struct Neighbor_AP_ELE *head;
			len = dcli_list->rouge_ap_list->neighborapInfosCount;
			head = dcli_list->rouge_ap_list->neighborapInfos;
			for(j=0;j<len;j++){
				vty_out(vty,"%02X:%02X:%02X:%02X:%02X:%02X %-5d %-4d %-7d %-4d %-5d %-7d %-5d %-12s\n",\
					head->BSSID[0],head->BSSID[1],head->BSSID[2],head->BSSID[3],head->BSSID[4],\
					head->BSSID[5],head->wtpid,head->Rate,head->Channel,head->RSSI,head->NOISE,head->BEACON_INT,\
					head->capabilityinfo,head->ESSID);
				head = head->next;
			}
			vty_out(vty,"==============================================================================\n");
		}
		//CW_FREE_OBJECT(dcli_list->rouge_ap_list->neighborapInfos);
		dcli_ac_free_fun_two(WID_DBUS_CONF_METHOD_NEIGHBOR_AP_LIST_BYWTPID2,dcli_list);
		dcli_list = NULL;
	}
	else if (ret == WTP_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> wtp %d does not exist\n",wtpid);
	}
	else if(ret == WID_AP_SCANNING_DISABLE)
	{
		vty_out(vty,"radio resource managment is disable please enable first\n");
	}
	else if(ret == WTP_ID_LARGE_THAN_MAX)
	{
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
	}
	else
	{
		vty_out(vty,"sorry there is no neighbor ap\n");
	}
	//CW_FREE_OBJECT(dcli_list->rouge_ap_list);
	//CW_FREE_OBJECT(dcli_list);
//	dbus_message_unref(reply);	
	
	return CMD_SUCCESS;			
}



//fengwenchao add 20101227

DEFUN(show_wids_device_of_all_cmd_func,
		  show_wids_device_of_all_cmd,
		  "show wids device of all",
		  "show wids device of all\n"
		  "show wids device of all\n"
	 )
{
	int ret = 0;
	int num = 0;
	int i = 0;
	int length = 0;
	unsigned int lasttime = 0;
	int wtpid = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	DCLI_AC_API_GROUP_TWO *dcli_list = NULL;
	

	dcli_list = show_wids_device_of_all_device(localid,dcli_dbus_connection,index,&ret,&lasttime);

	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
		vty_out(vty,"detection wids device list \n",wtpid);
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"%-17s \n","mac");
		if(dcli_list->wids_device_list->wids_device_info!= NULL){
			int len = 0;
			int j= 0;

			//unsigned char lasttime[128] = {0};
			struct tag_wids_device_ele *head;
			len = dcli_list->wids_device_list->count;
			head = dcli_list->wids_device_list->wids_device_info;
			for(j=0;j<len;j++){
				vty_out(vty,"-------------------------------------------------\n");
				vty_out(vty,"blackListDeviceMacID  %d\n",j+1);
				vty_out(vty,"blackListDeviceMAC  :%02X:%02X:%02X:%02X:%02X:%02X \n",head->bssid[0],head->bssid[1],head->bssid[2],head->bssid[3],head->bssid[4],head->bssid[5]);
				vty_out(vty,"blackListAddReason  : Add By dynamic\n");	
				vty_out(vty,"blackListDuration   : %d\n",lasttime);	
				vty_out(vty,"blackListAddType   :  2\n");
				/*fengwenchao add 20110513*/
				vty_out(vty,"first attack   :  %d\n",head->fst_attack);
				vty_out(vty,"last attack   :  %d\n",head->lst_attack);
				/*fengwenchao add end*/
				vty_out(vty,"-------------------------------------------------\n");
				vty_out(vty,"\n");

				head = head->next;
			}
			vty_out(vty,"==============================================================================\n");
		}
		CW_FREE_OBJECT(dcli_list->wids_device_list->wids_device_info);
		CW_FREE_OBJECT(dcli_list->wids_device_list);
		CW_FREE_OBJECT(dcli_list);
	}
	else if(ret == NO_WIDS_DEVICE)
	{
		vty_out(vty,"good luck there is no wids device\n");
	}
	else
	{
		vty_out(vty,"error \n");
	}
	
	return CMD_SUCCESS; 		
}

//fengwenchao add end

DEFUN(show_wids_device_list_cmd_func,
		  show_wids_device_list_cmd,
		  "show wids device list",
		  "show wids device list\n"
		  "show wids device list\n"
	 )
{
	int ret;
	int num = 0;
	int i = 0;
	int length = 0;

	int wtpid = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	DCLI_AC_API_GROUP_TWO *dcli_list = NULL;
#if 0
	DBusMessage *query, *reply; 
	DBusMessageIter  iter,iter_array;
	DBusError err;

	int state = 1;
	int wtpid = 0;
	
	unsigned char attacktype[WIDS_TYPE_LEN];
	unsigned char frametype[WIDS_TYPE_LEN];

	unsigned char firsttime[128];
	unsigned char lasttime[128];
	
	struct tag_wids_device_ele *wids_device = NULL;

	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_LIST);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_LIST);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);


	if(ret == 0 )
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		vty_out(vty,"detection wids device list \n",wtpid);
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"%-25s\n%-25s\n%-17s %-11s %-10s %-12s %-8s %-8s %-17s\n","fst_attack","lst_attack","mac","attacktype","frametype","attackcount","channel","rssi","bssid");
		for (i = 0; i < num; i++)
		{
			DBusMessageIter iter_struct;
			
			wids_device = (struct tag_wids_device_ele *)malloc(sizeof(struct tag_wids_device_ele));
			if((wids_device == NULL))
			{
				//printf("##malloc memory fail #\n");
				break;
			}

			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[0]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[1]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[2]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[3]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[4]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[5]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(wids_device->attacktype));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(wids_device->frametype));
			dbus_message_iter_next(&iter_struct);	

			dbus_message_iter_get_basic(&iter_struct,&(wids_device->attackcount));
			dbus_message_iter_next(&iter_struct);	
			
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->fst_attack));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->lst_attack));
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->channel));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->rssi));
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->vapbssid[0]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->vapbssid[1]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->vapbssid[2]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->vapbssid[3]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->vapbssid[4]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->vapbssid[5]));
			dbus_message_iter_next(&iter_array);

			CheckWIDSType(attacktype,frametype,wids_device->attacktype,wids_device->frametype);

			sn//printf(firsttime,127,"%s",ctime(&wids_device->fst_attack));
			sn//printf(lasttime,127,"%s",ctime(&wids_device->lst_attack));
			
			
			vty_out(vty,"%s%s%02X:%02X:%02X:%02X:%02X:%02X %-11s %-10s %-12d %-8d %-8d %02X:%02X:%02X:%02X:%02X:%02X\n",\
				firsttime,lasttime,wids_device->bssid[0],wids_device->bssid[1],\
				wids_device->bssid[2],wids_device->bssid[3],wids_device->bssid[4],\
				wids_device->bssid[5],attacktype,frametype,wids_device->attackcount,\
				wids_device->channel,wids_device->rssi,wids_device->vapbssid[0],wids_device->vapbssid[1],\
				wids_device->vapbssid[2],wids_device->vapbssid[3],wids_device->vapbssid[4],wids_device->vapbssid[5]);

			free(wids_device);	
			wids_device = NULL;

		}
		vty_out(vty,"==============================================================================\n");
	}
#endif
	dcli_list = dcli_ac_show_api_group_two(
		index,
		FIFTH,/*"show wids device list"*/
		wtpid,
		0,
		0,
		&ret,
		0,
		&localid,
		//dcli_list,
		dcli_dbus_connection,
		WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_LIST
		);
	//printf("#######ret is %d \n",ret);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
		vty_out(vty,"detection wids device list \n",wtpid);
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"%-25s\n%-25s\n%-17s %-11s %-10s %-12s %-8s %-8s %-17s %-5s\n","fst_attack","lst_attack","mac","attacktype","frametype","attackcount","channel","rssi","bssid","RogStaAttackStatus"); //fengwenchao modify 20110415
		if(dcli_list->wids_device_list->wids_device_info!= NULL){
			int len;
			int j= 0;
			unsigned char attacktype[WIDS_TYPE_LEN];
			unsigned char frametype[WIDS_TYPE_LEN];
			unsigned char firsttime[128];
			unsigned char lasttime[128];
			struct tag_wids_device_ele *head;
			len = dcli_list->wids_device_list->count;
			head = dcli_list->wids_device_list->wids_device_info;
			for(j=0;j<len;j++){
				snprintf(firsttime,127,"%s",ctime(&(head->fst_attack)));
				snprintf(lasttime,127,"%s",ctime(&head->lst_attack));
				CheckWIDSType(attacktype,frametype,head->attacktype,head->frametype);
				vty_out(vty,"%s%s%02X:%02X:%02X:%02X:%02X:%02X %-11s %-10s %-12d %-8d %-8d %02X:%02X:%02X:%02X:%02X:%02X %-10d\n",\  
					firsttime,lasttime,head->bssid[0],head->bssid[1],\
					head->bssid[2],head->bssid[3],head->bssid[4],\
					head->bssid[5],attacktype,frametype,head->attackcount,\
					head->channel,head->rssi,head->vapbssid[0],head->vapbssid[1],\
					head->vapbssid[2],head->vapbssid[3],head->vapbssid[4],head->vapbssid[5],head->RogStaAttackStatus);
				head = head->next;
			}
			vty_out(vty,"==============================================================================\n");
		}
		//CW_FREE_OBJECT(dcli_list->wids_device_list->wids_device_info);
		dcli_ac_free_fun_two(WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_LIST,dcli_list);
		dcli_list = NULL;
	}
	else if(ret == NO_WIDS_DEVICE)
	{
		vty_out(vty,"good luck there is no wids device\n");
	}
	else
	{
		vty_out(vty,"error \n");
	}
	//CW_FREE_OBJECT(dcli_list->wids_device_list);
	//CW_FREE_OBJECT(dcli_list);

		
//	dbus_message_unref(reply);	
	
	return CMD_SUCCESS; 		
}


DEFUN(show_wids_device_list_bywtpid_cmd_func,
		  show_wids_device_list_bywtpid_cmd,
		  "show wids device list bywtp ID",
		  "show wids device list\n"
		  "show wids device list bywtp ID\n"
		  "wtpid value\n"
	 )
{
	int ret;
	int num = 0;
	int i = 0;
	int length = 0;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter,iter_array;
	DBusError err;

	int state = 1;
	int wtpid = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	wtpid = atoi(argv[0]);
	if ((wtpid <= 0) || (wtpid >= WTP_NUM)) {
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_FAILURE;
		}
	DCLI_AC_API_GROUP_TWO *dcli_list = NULL;
#if 0
	unsigned char attacktype[WIDS_TYPE_LEN];
	unsigned char frametype[WIDS_TYPE_LEN];
	struct tag_wids_device_ele *wids_device = NULL;

	wtpid = atoi(argv[0]);
	if ((wtpid <= 0) || (wtpid >= WTP_NUM)) {
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_FAILURE;
		}
	
	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_LIST_BYWTPID);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_LIST_BYWTPID);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	


	if(ret == 0 )
	{

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		vty_out(vty,"wtp %d detection wids device list \n",wtpid);
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"%-17s %-11s %-10s %-12s %-15s\n","mac","attacktype","frametype","attackcount","fst_attack");

		for (i = 0; i < num; i++)
		{
			DBusMessageIter iter_struct;
			
			wids_device = (struct tag_wids_device_ele *)malloc(sizeof(struct tag_wids_device_ele));
			if((wids_device == NULL))
			{
				//printf("##malloc memory fail #\n");
				break;
			}

			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[0]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[1]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[2]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[3]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[4]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->bssid[5]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(wids_device->attacktype));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(wids_device->frametype));
			dbus_message_iter_next(&iter_struct);	

			dbus_message_iter_get_basic(&iter_struct,&(wids_device->attackcount));
			dbus_message_iter_next(&iter_struct);	
			
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->fst_attack));
			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&(wids_device->lst_attack));
			dbus_message_iter_next(&iter_array);


			CheckWIDSType(attacktype,frametype,wids_device->attacktype,wids_device->frametype);
			
			vty_out(vty,"%02X:%02X:%02X:%02X:%02X:%02X %-11s %-10s %-12d %-15s\n",\
				wids_device->bssid[0],wids_device->bssid[1],wids_device->bssid[2],wids_device->bssid[3],wids_device->bssid[4],\
				wids_device->bssid[5],attacktype,frametype,wids_device->attackcount,\
				ctime(&wids_device->fst_attack));

			free(wids_device); 	
			wids_device = NULL;

		}
		vty_out(vty,"==============================================================================\n");
	}
#endif
	dcli_list = dcli_ac_show_api_group_two(
		index,
		SIXTH,/*"show wids device list bywtp ID"*/
		wtpid,
		0,
		0,
		&ret,
		0,
		&localid,
		//dcli_list,
		dcli_dbus_connection,
		WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_LIST_BYWTPID
		);
	//printf("#######ret is %d \n",ret);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
		vty_out(vty,"wtp %d detection wids device list \n",wtpid);
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"%-17s %-11s %-10s %-12s %-15s\n","mac","attacktype","frametype","attackcount","fst_attack");
		if(dcli_list->wids_device_list->wids_device_info!= NULL){
			int len;
			int j= 0;
			unsigned char attacktype[WIDS_TYPE_LEN];
			unsigned char frametype[WIDS_TYPE_LEN];
			unsigned char firsttime[128];
			unsigned char lasttime[128];
			struct tag_wids_device_ele *head;
			len = dcli_list->wids_device_list->count;
			head = dcli_list->wids_device_list->wids_device_info;
			for(j=0;j<len;j++){
			//	snprintf(firsttime,127,"%s",ctime(&(head->fst_attack)));
			//	snprintf(lasttime,127,"%s",ctime(&head->lst_attack));
				CheckWIDSType(attacktype,frametype,head->attacktype,head->frametype);
				vty_out(vty,"%02X:%02X:%02X:%02X:%02X:%02X %-11s %-10s %-12d %-15s\n",\
					head->bssid[0],head->bssid[1],head->bssid[2],head->bssid[3],head->bssid[4],\
					head->bssid[5],attacktype,frametype,head->attackcount,\
					ctime(&head->fst_attack));
				head = head->next;
			}
			vty_out(vty,"==============================================================================\n");
		}
		//CW_FREE_OBJECT(dcli_list->wids_device_list->wids_device_info);
		dcli_ac_free_fun_two(WID_DBUS_CONF_METHOD_SHOW_WIDS_DEVICE_LIST_BYWTPID,dcli_list);
		dcli_list = NULL;
	}
	else if (ret == WTP_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> wtp %d does not exist\n",wtpid);
	}
	else if(ret == WTP_ID_LARGE_THAN_MAX)
	{
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
	}
	
	else if(ret == NO_WIDS_DEVICE)
	{
		vty_out(vty,"good luck there is no wids device\n");
	}
	else
	{
		vty_out(vty,"error \n");
	}
		
	//CW_FREE_OBJECT(dcli_list->wids_device_list);
	//CW_FREE_OBJECT(dcli_list);
	return CMD_SUCCESS; 		
}


DEFUN(clear_wids_device_list_cmd_func,
		  clear_wids_device_list_cmd,
		  "clear wids device list",
		  "clear wids device list\n"
		  "clear wids device list\n"
	 )
{
	int ret;
	int i = 0;
	int num = 0;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter,iter_array;
	DBusError err;

	int state = 1;
	int wtpid = 0;
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_CLEAR_WIDS_DEVICE_LIST);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_CLEAR_WIDS_DEVICE_LIST);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);


	if(ret == 0 )
	{
		vty_out(vty,"clear wids device success\n");

	}	
	else
	{
		vty_out(vty,"error\n");
	}
		
	dbus_message_unref(reply);	
	
	return CMD_SUCCESS; 		
}


DEFUN(clear_wids_device_list_bywtpid_cmd_func,
		  clear_wids_device_list_bywtpid_cmd,
		  "clear wids device list bywtp ID",
		  "clear wids device list\n"
		  "clear wids device list bywtp ID\n"
		  "wtpid value\n"
	 )
{
	int ret;
	int i = 0;
	int num = 0;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter,iter_array;
	DBusError err;

	int state = 1;
	int wtpid = 0;
	wid_wids_statistics widsstatis;

	wtpid = atoi(argv[0]);
	if ((wtpid <= 0) || (wtpid >= WTP_NUM)) {
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_FAILURE;
		}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_CLEAR_WIDS_DEVICE_LIST_BYWTPID);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_CLEAR_WIDS_DEVICE_LIST_BYWTPID);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);


	if(ret == 0 )
	{
		vty_out(vty,"wtp %d clear wids device success\n",wtpid);

	}	
	else if (ret == WTP_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> wtp %d does not exist\n",wtpid);
	}
	else if(ret == WTP_ID_LARGE_THAN_MAX)
	{
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
	}
	else
	{
		vty_out(vty,"error\n");
	}
		
	dbus_message_unref(reply);	
	
	return CMD_SUCCESS; 		
}



DEFUN(show_wids_statistics_list_bywtpid_cmd_func,
		  show_wids_statistics_list_bywtpid_cmd,
		  "show wids statistics bywtp ID",
		  "show wids statistics list\n"
		  "show wids statistics bywtp ID\n"
		  "wtpid value\n"
	 )
{
	int ret;
	int i = 0;
	int num = 0;

//	DBusMessage *query, *reply;	
//	DBusMessageIter	 iter,iter_array;
//	DBusError err;

    int state = 1;
	int wtpid = 0;
	DCLI_AC_API_GROUP_THREE *widsstatis = NULL;
	CW_CREATE_OBJECT_ERR(widsstatis, DCLI_AC_API_GROUP_THREE, return -1;);	
	widsstatis->WTPIP = NULL;

	wtpid = atoi(argv[0]);
	if ((wtpid <= 0) || (wtpid >= WTP_NUM)) {
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_FAILURE;
		}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#if 0	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WIDS_STATISTICS_LIST_BYWTPID);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WIDS_STATISTICS_LIST_BYWTPID);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);


	if(ret == 0 )
	{
		vty_out(vty,"==============================================================================\n");	
		vty_out(vty,"%-9s %-9s %-8s\n","flooding","spoofing","weakiv");

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(widsstatis.floodingcount));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(widsstatis.sproofcount));
		dbus_message_iter_next(&iter);	

		dbus_message_iter_get_basic(&iter,&(widsstatis.weakivcount));
		dbus_message_iter_next(&iter);

		/*00001 0 1 0 
		////printf("00001 %d %d %d \n",widsstatis.floodingcount,widsstatis.sproofcount,widsstatis.weakivcount);*/

		vty_out(vty,"%-9d %-9d %-8d\n",widsstatis.floodingcount,widsstatis.sproofcount,widsstatis.weakivcount);

		vty_out(vty,"==============================================================================\n");
	}
#endif
	widsstatis = dcli_ac_show_api_group_three(
		index,
		THIRD,/*"show wids statistics bywtp ID"*/
		wtpid,
		0,
		0,
		&ret,
		0,
		&localid,
		//widsstatis,
		dcli_dbus_connection,
		WID_DBUS_CONF_METHOD_SHOW_WIDS_STATISTICS_LIST_BYWTPID
		);
	//printf("#######ret is %d \n",ret);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
		vty_out(vty,"==============================================================================\n");	
		vty_out(vty,"%-9s %-9s %-8s\n","flooding","spoofing","weakiv");

		vty_out(vty,"%-9d %-9d %-8d\n",widsstatis->floodingcount,widsstatis->sproofcount,widsstatis->weakivcount);

		vty_out(vty,"==============================================================================\n");
		dcli_ac_free_fun_three(WID_DBUS_CONF_METHOD_SHOW_WIDS_STATISTICS_LIST_BYWTPID,widsstatis);
		widsstatis = NULL;
	}
	else if (ret == WTP_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> wtp %d does not exist\n",wtpid);
	}
	else if(ret == WTP_ID_LARGE_THAN_MAX)
	{
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
	}
	else
	{
		vty_out(vty,"error\n");
	}
		
//	dbus_message_unref(reply);	
//	CW_FREE_OBJECT(widsstatis);
	return CMD_SUCCESS;			
}

DEFUN(show_wids_statistics_list_cmd_func,
		  show_wids_statistics_list_cmd,
		  "show wids statistics",
		  "show wids statistics list\n"
		  "show wids statistics \n"
	 )
{
	int ret;

//	DBusMessage *query, *reply; 
//	DBusMessageIter  iter,iter_array;
//	DBusError err;

	int state = 1;
	int wtpid = 0;
	DCLI_AC_API_GROUP_THREE *widsstatis = NULL;
//	CW_CREATE_OBJECT_ERR(widsstatis, DCLI_AC_API_GROUP_THREE, return NULL;);	
//	widsstatis->WTPIP = NULL;

	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#if 0	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WIDS_STATISTICS_LIST);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WIDS_STATISTICS_LIST);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);


	if(ret == 0 )
	{
		vty_out(vty,"==============================================================================\n");	
		vty_out(vty,"%-9s %-9s %-8s\n","flooding","spoofing","weakiv");

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(widsstatis.floodingcount));
		dbus_message_iter_next(&iter);

		dbus_message_iter_get_basic(&iter,&(widsstatis.sproofcount));
		dbus_message_iter_next(&iter);	

		dbus_message_iter_get_basic(&iter,&(widsstatis.weakivcount));
		dbus_message_iter_next(&iter);


		vty_out(vty,"%-9d %-9d %-8d\n",widsstatis.floodingcount,widsstatis.sproofcount,widsstatis.weakivcount);

		vty_out(vty,"==============================================================================\n");
	}
#endif
//printf("before group three.\n");
	widsstatis = dcli_ac_show_api_group_three(
		index,
		FOURTH,/*"show wids statistics"*/
		wtpid,
		0,
		0,
		&ret,
		0,
		&localid,
		//widsstatis,		
		dcli_dbus_connection,
		WID_DBUS_CONF_METHOD_SHOW_WIDS_STATISTICS_LIST
		);
	//printf("#######ret is %d \n",ret);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
		vty_out(vty,"==============================================================================\n");	
		vty_out(vty,"%-9s %-9s %-8s\n","flooding","spoofing","weakiv");

		vty_out(vty,"%-9d %-9d %-8d\n",widsstatis->floodingcount,widsstatis->sproofcount,widsstatis->weakivcount);

		vty_out(vty,"==============================================================================\n");
		dcli_ac_free_fun_three(WID_DBUS_CONF_METHOD_SHOW_WIDS_STATISTICS_LIST,widsstatis);
		widsstatis = NULL;
	}
	else
	{
		vty_out(vty,"error\n");
	}
		
//	dbus_message_unref(reply);	
//	CW_FREE_OBJECT(widsstatis);
	return CMD_SUCCESS; 		
}


DEFUN(clear_wids_statistics_list_bywtpid_cmd_func,
		  clear_wids_statistics_list_bywtpid_cmd,
		  "clear wids statistics bywtp ID",
		  "clear wids statistics list\n"
		  "clear wids statistics bywtp ID\n"
		  "wtpid value\n"
	 )
{
	int ret;
	int i = 0;
	int num = 0;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter,iter_array;
	DBusError err;

	int state = 1;
	int wtpid = 0;
	wid_wids_statistics widsstatis;

	wtpid = atoi(argv[0]);
	if ((wtpid <= 0) || (wtpid >= WTP_NUM)) {
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_FAILURE;
		}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_CLEAR_WIDS_STATISTICS_LIST_BYWTPID);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_CLEAR_WIDS_STATISTICS_LIST_BYWTPID);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);


	if(ret == 0 )
	{
		vty_out(vty,"wtp %d clear wids statistics success\n",wtpid);

	}	
	else if (ret == WTP_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> wtp %d does not exist\n",wtpid);
	}
	else if(ret == WTP_ID_LARGE_THAN_MAX)
	{
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
	}
	else
	{
		vty_out(vty,"error\n");
	}
		
	dbus_message_unref(reply);	
	
	return CMD_SUCCESS; 		
}


DEFUN(clear_wids_statistics_list_cmd_func,
		  clear_wids_statistics_list_cmd,
		  "clear wids statistics",
		  "clear wids statistics list\n"
		  "clear wids statistics \n"
	 )
{
	int ret;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter,iter_array;
	DBusError err;

	int state = 1;
	int wtpid = 0;
	wid_wids_statistics widsstatis;

	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_CLEAR_WIDS_STATISTICS_LIST);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_CLEAR_WIDS_STATISTICS_LIST);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);


	if(ret == 0 )
	{
		vty_out(vty,"clear wids statistics success \n");

	}
	else
	{
		vty_out(vty,"error\n");
	}
		
	dbus_message_unref(reply);	
	
	return CMD_SUCCESS; 		
}

DEFUN(show_wirelesscontrol_whitelist_cmd_func,
		  show_wirelesscontrol_whitelist_cmd,
		  "show wireless-control whitelist",
		  "show wireless-control list\n"
		  "show wireless-control whitelist\n"
	 )
{
	int ret;
	int num = 0;
	int i = 0;
	int length = 0;

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	DCLI_AC_API_GROUP_ONE  *dcli_list = NULL;
	CW_CREATE_OBJECT_ERR(dcli_list, DCLI_AC_API_GROUP_ONE, return -1;);	
	dcli_list->dcli_attack_mac_list = NULL;
	dcli_list->dcli_essid_list = NULL;
	dcli_list->dcli_oui_list = NULL;
	
#if 0
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter,iter_array;
	DBusError err;

    int state = 1;
	unsigned char whitemac[DCLIAC_MAC_LEN];	
	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WHITELIST_SHOW);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_WHITELIST_SHOW);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret == 0)
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		vty_out(vty,"white list is \n");
		vty_out(vty,"%-17s \n","mac:");
		for (i = 0; i < num; i++)
		{
			DBusMessageIter iter_struct;
			
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(whitemac[0]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(whitemac[1]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(whitemac[2]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(whitemac[3]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(whitemac[4]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(whitemac[5]));

			dbus_message_iter_next(&iter_array);


			vty_out(vty,"%02X:%02X:%02X:%02X:%02X:%02X\n",whitemac[0],whitemac[1],whitemac[2],whitemac[3],whitemac[4],whitemac[5]);

		}
	}
#endif
	dcli_list = dcli_ac_show_api_group_one(
			index,
			0,/*"show wireless-control whitelist"*/
			0,
			0,
			&ret,
			0,
			&localid,
			//&dcli_list,			
			dcli_dbus_connection,
			WID_DBUS_CONF_METHOD_WHITELIST_SHOW
			);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
		vty_out(vty,"white list is \n");
		vty_out(vty,"%-17s \n","mac:");
		if(dcli_list->dcli_attack_mac_list!= NULL){
			int len;
			int j= 0;
			struct attack_mac_node *head;
			len = dcli_list->dcli_attack_mac_list->list_len;
			head = dcli_list->dcli_attack_mac_list->attack_mac_list;
			for(j=0;j<len;j++){
				if(head != NULL){
					vty_out(vty,"\t%d\t%02X:%02X:%02X:%02X:%02X:%02X\n",j+1,head->mac[0],head->mac[1],head->mac[2],head->mac[3],head->mac[4],head->mac[5]);
					head = head->next;
				}
			}
		}
		//CW_FREE_OBJECT(dcli_list->dcli_attack_mac_list->attack_mac_list);
		dcli_ac_free_fun(WID_DBUS_CONF_METHOD_WHITELIST_SHOW,dcli_list);
		dcli_list = NULL;
	}
	else if(ret == WID_NO_WHITELIST)
	{
		vty_out(vty,"there is no white list \n");
	}
	else
	{
		vty_out(vty,"error %d \n",ret);
	}
	//CW_FREE_OBJECT(dcli_list->dcli_attack_mac_list);
	//CW_FREE_OBJECT(dcli_list);

		
//	dbus_message_unref(reply);	
	
	return CMD_SUCCESS;				
}

DEFUN(show_wirelesscontrol_blacklist_cmd_func,
		  show_wirelesscontrol_blacklist_cmd,
		  "show wireless-control blacklist",
		  "show wireless-control list\n"
		  "show wireless-control blacklist\n"
	 )
{
	int ret;
	int num = 0;
	int i = 0;
	int length = 0;
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	DCLI_AC_API_GROUP_ONE  *dcli_list = NULL;
	CW_CREATE_OBJECT_ERR(dcli_list, DCLI_AC_API_GROUP_ONE, return -1;);	
	dcli_list->dcli_attack_mac_list = NULL;
	dcli_list->dcli_essid_list = NULL;
	dcli_list->dcli_oui_list = NULL;
	
#if 0
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter,iter_array;
	DBusError err;

    int state = 1;
	unsigned char blackmac[DCLIAC_MAC_LEN];	
	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_BLACKLIST_SHOW);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_BLACKLIST_SHOW);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret == 0 )
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		vty_out(vty,"black list is \n");
		vty_out(vty,"%-17s \n","mac:");
		for (i = 0; i < num; i++)
		{
			DBusMessageIter iter_struct;
			
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(blackmac[0]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(blackmac[1]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(blackmac[2]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(blackmac[3]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(blackmac[4]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(blackmac[5]));

			dbus_message_iter_next(&iter_array);


			vty_out(vty,"%02X:%02X:%02X:%02X:%02X:%02X\n",blackmac[0],blackmac[1],blackmac[2],blackmac[3],blackmac[4],blackmac[5]);

		}
	}
#endif
	dcli_list = dcli_ac_show_api_group_one(
			index,
			0,/*"show wireless-control blacklist"*/
			0,
			0,
			&ret,
			0,
			&localid,
		//	&dcli_list,
			dcli_dbus_connection,
			WID_DBUS_CONF_METHOD_BLACKLIST_SHOW
			);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
		vty_out(vty,"black list is \n");
		vty_out(vty,"%-17s \n","mac:");
		if(dcli_list->dcli_attack_mac_list!= NULL){
			int len;
			int j= 0;
			struct attack_mac_node *head;
			len = dcli_list->dcli_attack_mac_list->list_len;
			head = dcli_list->dcli_attack_mac_list->attack_mac_list;
			for(j=0;j<len;j++){
				if(head != NULL){
					vty_out(vty,"\t%d\t%02X:%02X:%02X:%02X:%02X:%02X\n",j+1,head->mac[0],head->mac[1],head->mac[2],head->mac[3],head->mac[4],head->mac[5]);
					head = head->next;
				}
			}
		}
		//CW_FREE_OBJECT(dcli_list->dcli_attack_mac_list->attack_mac_list);
		dcli_ac_free_fun(WID_DBUS_CONF_METHOD_BLACKLIST_SHOW,dcli_list);
		dcli_list = NULL;
	}
	else if(ret == WID_NO_BLACKLIST)
	{
		vty_out(vty,"there is no black list \n");
	}
	else
	{
		vty_out(vty,"error %d \n",ret);
	}
	
	//CW_FREE_OBJECT(dcli_list->dcli_attack_mac_list);
	//CW_FREE_OBJECT(dcli_list);
	
//	dbus_message_unref(reply);	
	
	return CMD_SUCCESS;				
}
DEFUN(dynamic_channel_selection_func,
		  dynamic_channel_selection_cmd,
		  "(open|close) dynamic channel selection",
		  "open or close dynamic channel selection function\n"
		  "dynamic channel selection\n"
	 )
	{
		int ret;
	
		DBusMessage *query, *reply; 
		DBusMessageIter  iter;
		DBusError err;
	
		int policy = 0;
	
		
		if (!strcmp(argv[0],"open"))
		{
			policy = 1; 
		}
		else if (!strcmp(argv[0],"close"))
		{
			policy = 0; 
		}
		else
		{
			vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
			return CMD_SUCCESS;
		}
		
		int localid = 1;
		int slot_id = HostSlotId;
		int index = 0;
		char BUSNAME[PATH_LEN];
		char OBJPATH[PATH_LEN];
		char INTERFACE[PATH_LEN];
		if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
			index = 0;
		}else if(vty->node == HANSI_NODE){
			index = (int)vty->index;
			localid = vty->local;
			slot_id = vty->slotindex;
		}
		else if(vty->node == LOCAL_HANSI_NODE){
			index = (int)vty->index;
			localid = vty->local;
			slot_id = vty->slotindex;
		}
		DBusConnection *dcli_dbus_connection = NULL;
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_DYNAMIC_CHANNEL_SELECTION);
		
	/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
							WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_DYNAMIC_CHANNEL_SELECTION);*/
		dbus_error_init(&err);
	
	
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&policy,
								 DBUS_TYPE_INVALID);
	
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		
		if (NULL == reply)
		{
			cli_syslog_info("<error> failed get reply.\n");
			if (dbus_error_is_set(&err))
			{
				cli_syslog_info("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
	
			return CMD_SUCCESS;
		}
		
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
	
		if(ret == 0)
		{
			vty_out(vty," %s dynamic channel selection successfully, country-code has changed to US\n",argv[0]);
		}				
		else
		{
			vty_out(vty,"failure,you should enable radio resource management first\n");
		}
			
		dbus_message_unref(reply);
	
		
		return CMD_SUCCESS; 		
	}

DEFUN(dynamic_channel_selection_range_func,
		  dynamic_channel_selection_range_cmd,
		  "set dynamic channel selection range .CHANNEL",
		  "dynamic channel selection range\n"
		  "eg:set dynamic channel selection 1 6 11\n"
	 )
{
	int ret;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	int policy = 0;
	unsigned char channel[4] = {0,0,0,0};
	int i;
	if(argc <= 1 || argc > 4){
		vty_out(vty,"range of the channel num is 2-4.\n");
		return CMD_SUCCESS;
	}
	for(i = 0; i < argc; i++){		
		ret = parse_char_ID((char*)argv[i], &channel[i]);
		if(ret != WID_DBUS_SUCCESS){
           if(ret == WID_ILLEGAL_INPUT){
             vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
		   }
		   else{
			vty_out(vty,"patameter format error.\n");
		   	}
			return CMD_SUCCESS;
		}
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	ret = dcli_ac_set_dynamic_channel_selection_range(localid,index,argc,channel,dcli_dbus_connection);

	if(ret == 0)
	{
		vty_out(vty," %s dynamic channel selection successfully\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"failure,you should enable radio resource management first\n");
	}
	
	return CMD_SUCCESS; 		
}


DEFUN(set_system_country_code_func,
		  set_system_country_code_cmd,
		  "country-code CODE",
		  "set country-code\n"
		  "set the country-code according to ISO-3166-1,like 'CN' or 'US'\n"
	 )
{
	int ret = 0;
	int wtpid = 0;/*wcl add for OSDEVTDPB-31*/
	int radioid = 0;/*wcl add for OSDEVTDPB-31*/
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	
	int country_code = COUNTRY_USA_US;
	
	country_code = parse_country_code((char *) argv[0]);
	/*//printf("country_code %d",country_code);*/
	
	
	if (country_code == COUNTRY_CODE_ERROR_SMALL_LETTERS)
	{
		vty_out(vty,"<error> input country code should be capital letters\n");
		return CMD_FAILURE;
	}
	if (country_code == COUNTRY_CODE_ERROR)
	{
		vty_out(vty,"<error> input country code error\n");
		return CMD_FAILURE;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	} else if(vty->node == HANSI_WTP_NODE){/*wcl add for OSDEVTDPB-31*/
		index = (int)vty->index; 		
		wtpid = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_WTP_NODE){
		index = (int)vty->index; 		
		wtpid = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == HANSI_RADIO_NODE){
		index = (int)vty->index; 		
		localid = vty->local;
        slot_id = vty->slotindex;
		radioid = (int)vty->index_sub;
	}else if (vty->node == LOCAL_HANSI_RADIO_NODE){
        index = (int)vty->index;
        localid = vty->local;
        slot_id = vty->slotindex;
		radioid = (int)vty->index_sub;
    }/*wcl add for OSDEVTDPB-31*/
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_COUNTRY_CODE);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_COUNTRY_CODE);*/
	


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&country_code,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_UINT32,&radioid,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);


	if(ret == COUNTRY_CODE_SUCCESS)
	{
		vty_out(vty,"system country code set %s successfully\n",argv[0]);
	}
	
	else if (ret == COUNTRY_CODE_NO_CHANGE)
	{
		vty_out(vty,"system country code is %s, no need to change\n",argv[0]);
	}
	else
	{
		vty_out(vty,"<error> system country code error\n");
	}
		
	dbus_message_unref(reply);	
	
	return CMD_SUCCESS;			
}
DEFUN(undo_system_country_code_func,
		  undo_system_country_code_cmd,
		  "undo country-code",
		  "set country-code\n"
		  "set the country-code default 'CN'\n"
	 )
{
	int ret = 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	dbus_error_init(&err);
	
	int country_code = COUNTRY_USA_US;/*default value*/
	
	/*//printf("country_code %d",country_code);*/
	
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_UNDO_COUNTRY_CODE);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_UNDO_COUNTRY_CODE);*/
	


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&country_code,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);


	if(ret == COUNTRY_CODE_SUCCESS)
	{
		vty_out(vty,"system country code set default 'US' successfully\n");
	}
	
	else if (ret == COUNTRY_CODE_NO_CHANGE)
	{
		vty_out(vty,"system country code is default, no need to change\n");
	}
	else
	{
		vty_out(vty,"<error> system country code error\n");
	}
		
	dbus_message_unref(reply);	
	
	return CMD_SUCCESS;			
}

/*ht add,08.12.01*/
DEFUN(set_asd_daemonlog_debug_open_func,
	  set_asd_daemonlog_debug_open_cmd,
	  "set asd daemonlog (default|dbus|80211|1x|wpa|wapi|leave |all) debug (open|close)",
	  "asd config\n"
	  "asd daemonlog config\n"
	  "asd daemonlog debug open|close\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    unsigned int daemonlogtype;
    unsigned int daemonloglevel;
	

	if (!strcmp(argv[0],"default"))
	{
		daemonlogtype = ASD_DEFAULT;	
	}
	else if (!strcmp(argv[0],"dbus"))
	{
		daemonlogtype = ASD_DBUS;	
	}
	else if (!strcmp(argv[0],"80211"))
	{
		daemonlogtype = ASD_80211;	
	}
	else if (!strcmp(argv[0],"1x"))
	{
		daemonlogtype = ASD_1X;	
	}
	else if (!strcmp(argv[0],"wpa"))
	{
		daemonlogtype = ASD_WPA;	
	}
	else if (!strcmp(argv[0],"wapi"))
	{
		daemonlogtype = ASD_WAPI;	
	}
	else if (!strcmp(argv[0],"leave"))
	{
		daemonlogtype = ASD_LEAVE;	
	}
	else if (!strcmp(argv[0],"all"))
	{
		daemonlogtype = ASD_ALL;	
	}
	else
	{
		vty_out(vty,"<error> input patameter should only be default|dbus|80211|1x|wpa|wapi|leave|all\n");
		return CMD_SUCCESS;
	}

	
	
	if (!strcmp(argv[1],"open"))
	{
		daemonloglevel = 1;	
	}
	else if (!strcmp(argv[1],"close"))
	{
		daemonloglevel = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter should only be 'open' or 'close'\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_ASD_DAEMONLOG_DEBUG);
	
/*	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_ASD_DAEMONLOG_DEBUG);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&daemonlogtype,
							 DBUS_TYPE_UINT32,&daemonloglevel,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"asd set daemonlog debug %s successfully\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}

/*ht add,08.12.04*/
DEFUN(set_hostapd_logger_printflag_open_func,
	  set_hostapd_logger_printflag_open_cmd,
	  "set asd logger (open|close)",
	  "asd config\n"
	  "asd logger config\n"
	  "asd logger printflag open|close\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    unsigned char printflag;

	
	if (!strcmp(argv[0],"open"))
	{
		printflag = 1;	
	}
	else if (!strcmp(argv[0],"close"))
	{
		printflag = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter should only be 'open' or 'close'\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_ASD_LOGGER_PRINTFLAG);
	
/*	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_HOSTAPD_LOGGER_PRINTFLAG);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&printflag,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"asd set hostapd logger printflag %s successfully\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}

DEFUN(set_asd_daemonlog_level_cmd_func,
	  set_asd_daemonlog_level_cmd,
	  "set asd daemonlog level (dump|debug|info|notice|warning|error|crit|alert|emerg|default)",
	  "asd config\n"
	  "asd daemonlog config\n"
	  "asd daemonlog config\n"
	  "asd daemonlog level\n"
	  "dump level,the minimum level\n"
	  "debug level\n"
	  "info level\n"
	  "notice level\n"
	  "warning level\n"
 	  "error level\n"
 	  "crit level\n"
 	  "alert level\n"
 	  "emerg level\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    unsigned int daemonloglevel = 5;
	
	if (!strcmp(argv[0],"dump"))
	{
		daemonloglevel = 1;	
	}
	else if (!strcmp(argv[0],"debug"))
	{
		daemonloglevel = 2;	
	}
	else if (!strcmp(argv[0],"info"))
	{
		daemonloglevel = 3;	
	}
	else if ((!strcmp(argv[0],"notice"))||(!strcmp(argv[0],"default")))
	{
		daemonloglevel = 4;	
	}
	else if (!strcmp(argv[0],"warning"))
	{
		daemonloglevel = 5;	
	}
	else if (!strcmp(argv[0],"error"))
	{
		daemonloglevel = 6;	
	}
	else if (!strcmp(argv[0],"crit"))
	{
		daemonloglevel = 7;	
	}
	else if (!strcmp(argv[0],"alert"))
	{
		daemonloglevel = 8;	
	}
	else if (!strcmp(argv[0],"emerg"))
	{
		daemonloglevel = 9;	
	}
	else
	{
		vty_out(vty,"<error> input patameter should only be dump|debug|info|notice|warning|error|crit|alert|emerg\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_ASD_DAEMONLOG_LEVEL);
	
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&daemonloglevel,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"asd set daemonlog level %s successfully\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;			
}

DEFUN(transmit_power_control_func,
		  transmit_power_control_cmd,
		  "(open|close) transmit power control",
		  "open or close transmit power control function\n"
		  " transmit power control\n"
	 )
{
	int ret;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	int policy = 0;

	
	if (!strcmp(argv[0],"open"))
	{
		policy = 1; 
	}
	else if (!strcmp(argv[0],"close"))
	{
		policy = 0; 
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_TRANSMIT_POWER_CONTROL);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_TRANSMIT_POWER_CONTROL);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," %s transmit power control successfully\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"failure,you should enable radio resource management first\n");
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS; 		
}

DEFUN(set_txpower_threshold_cmd_func,
		  set_txpower_threshold_cmd,
	 	  "set txpower threshold VALUE",
		  "ac config\n"
		  "ac txpower threshold config\n"
		  "ac txpower threshold config value\n"
	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    int txpower_threshold = 0;
	int ret;
	
	txpower_threshold = atoi(argv[0]);
	
	if (txpower_threshold < 20 || txpower_threshold > 35)
	{	
		vty_out(vty,"<error> input parameter should be 20 to 35\n"); /*fengwenchao change 25 to 20 for AXSSZFI-557*/
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_TXPOWER_THRESHOLD);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_TXPOWER_THRESHOLD);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&txpower_threshold,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"set txpower threshold %s successfully\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;	
	
}
DEFUN(set_rogue_ap_trap_threshold_func,
		  set_rogue_ap_trap_threshold_cmd,
	 	  "set rogue ap trap threshold VALUE",
		  "ac config\n"
		  "ac rogue ap trap threshold config\n"
		  "ac rogue ap trap threshold config value\n"
	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    int rogue_ap_threshold = 0;
	int ret;
	
	rogue_ap_threshold = atoi(argv[0]);
	
	if (rogue_ap_threshold < 1 || rogue_ap_threshold > 200)
	{	
		vty_out(vty,"<error> input patameter should be 1 to 200\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_ROGUE_AP_TRAP_THRESHOLD);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_ROGUE_AP_TRAP_THRESHOLD);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&rogue_ap_threshold,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"set rogue ap trap threshold %s successfully\n",argv[0]);
	}
	else if(ret == WID_AP_SCANNING_DISABLE)
	{
		vty_out(vty,"radio resource managment is disable please enable first\n");
	}
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;	
	
}

DEFUN(set_coverage_threshold_cmd_func,
		  set_coverage_threshold_cmd,
	 	  "set coverage threshold VALUE",
		  "ac config\n"
		  "ac coverage threshold config\n"
		  "ac coverage threshold config value\n"
	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    int coverage_threshold = 0;
	int ret;
	
	coverage_threshold = atoi(argv[0]);
	
	if (coverage_threshold < 5 || coverage_threshold > 15)
	{	
		vty_out(vty,"<error> input patameter should be 5 to 15\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_COVERAGE_THRESHOLD);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_COVERAGE_THRESHOLD);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&coverage_threshold,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"set coverage threshold %s successfully\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;	
	
}

DEFUN(show_neighbor_rssi_info_bywtpid_cmd_func,
		  show_neighbor_rssi_info_bywtpid_cmd,
	 	  "show neighbor rssi bywtpid ID [remote] [local] [PARAMETER]",
		  SHOW_STR
		  "show neighbor rssi info\n"
		  "neighbor rssi info\n"
		  "bywtpid WTPID\n"
		  "wtpid value\n"
		  "'remote' or 'local' hansi\n"
		  "'remote' or 'local' hansi\n"
		  "slotid-instid\n"
	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	int ret = -1;
	int wtpid = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	unsigned char txpowr;
	unsigned char rssi[DCLIAC_RADIO_NUM_LEN];

	DCLI_AC_API_GROUP_THREE *RSSI = NULL;
	//CW_CREATE_OBJECT_ERR(RSSI, DCLI_AC_API_GROUP_THREE, return NULL;);	
	//RSSI->WTPIP = NULL;
	
	wtpid = atoi(argv[0]);
	
	if ((wtpid <= 0) || (wtpid >= WTP_NUM))
	{
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_FAILURE;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
//	char BUSNAME[PATH_LEN];
//	char OBJPATH[PATH_LEN];
//	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	if((argc == 2)||(argc == 4)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 3){
		if (!strcmp(argv[1],"remote")){
			localid = 0;
		}else if(!strcmp(argv[1],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[1],"remote"))&&(!strcmp(argv[2],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[2],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[2], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}
#if 0	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_NEIGHBOR_RSSI_INFO);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SHOW_NEIGHBOR_RSSI_INFO);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);


	if(ret == 0 )
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&txpowr);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&rssi[0]);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&rssi[1]);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&rssi[2]);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&rssi[3]);

		
		vty_out(vty,"wtp %d neighbor ap rssi list \n",wtpid);
		vty_out(vty,"txpower:%d\n",txpowr);	
		vty_out(vty,"rssi list:%d %d %d %d \n",rssi[0],rssi[1],rssi[2],rssi[3]);	
	}
#endif
	if(vty->node != VIEW_NODE){
		RSSI = dcli_ac_show_api_group_three(
			index,
			FIFTH,/*"show neighbor rssi bywtpid ID"*/
			wtpid,
			0,
			0,
			&ret,
			0,
			&localid,
			//RSSI,
			dcli_dbus_connection,
			WID_DBUS_CONF_METHOD_SHOW_NEIGHBOR_RSSI_INFO
			);
		//printf("#######ret is %d \n",ret);
		if(ret == -1){
			cli_syslog_info("<error> failed get reply.\n");
		}
		else if(ret == 0){	
					
			vty_out(vty,"wtp %d neighbor ap rssi list \n",wtpid);
			vty_out(vty,"txpower:%d\n",RSSI->txpowr);	
			vty_out(vty,"rssi list:%d %d %d %d \n",RSSI->rssi[0],RSSI->rssi[1],RSSI->rssi[2],RSSI->rssi[3]);	
			dcli_ac_free_fun_three(WID_DBUS_CONF_METHOD_SHOW_NEIGHBOR_RSSI_INFO,RSSI);
			RSSI = NULL;
		}
		else if (ret == WTP_ID_NOT_EXIST)
		{
			vty_out(vty,"<error> wtp %d does not exist\n",wtpid);
		}
		else if(ret == WID_AP_SCANNING_DISABLE)
		{
			vty_out(vty,"radio resource managment is disable please enable first\n");
		}
		else if(ret == WID_TRANSMIT_POWER_CONTROL_DISABLE)
		{
			vty_out(vty,"transmit power control is disable please enable first\n");
		}	
		else if(ret == WTP_ID_LARGE_THAN_MAX)
		{
			vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
		}
		else
		{
			vty_out(vty,"sorry there is no neighbor ap\n");
		}
			
	//	dbus_message_unref(reply);	
	//	CW_FREE_OBJECT(RSSI);
	}

	if(vty->node == VIEW_NODE){
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			//for remote hansi info
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}
	
		 hansi_parameter:
				RSSI = dcli_ac_show_api_group_three(
					profile,
					FIFTH,/*"show neighbor rssi bywtpid ID"*/
					wtpid,
					0,
					0,
					&ret,
					0,
					&localid,
					dcli_dbus_connection,
					WID_DBUS_CONF_METHOD_SHOW_NEIGHBOR_RSSI_INFO
					);
			    vty_out(vty,"==============================================================================\n");
			    vty_out(vty,"hansi %d-%d\n",slot_id,profile);
			    vty_out(vty,"-----------------------------------------------------------------------------\n");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0){	
							
					vty_out(vty,"wtp %d neighbor ap rssi list \n",wtpid);
					vty_out(vty,"txpower:%d\n",RSSI->txpowr);	
					vty_out(vty,"rssi list:%d %d %d %d \n",RSSI->rssi[0],RSSI->rssi[1],RSSI->rssi[2],RSSI->rssi[3]);	
					dcli_ac_free_fun_three(WID_DBUS_CONF_METHOD_SHOW_NEIGHBOR_RSSI_INFO,RSSI);
					RSSI = NULL;
				}
				else if (ret == WTP_ID_NOT_EXIST)
				{
					vty_out(vty,"<error> wtp %d does not exist\n",wtpid);
				}
				else if(ret == WID_AP_SCANNING_DISABLE)
				{
					vty_out(vty,"radio resource managment is disable please enable first\n");
				}
				else if(ret == WID_TRANSMIT_POWER_CONTROL_DISABLE)
				{
					vty_out(vty,"transmit power control is disable please enable first\n");
				}	
				else if(ret == WTP_ID_LARGE_THAN_MAX)
				{
					vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
				}
				else
				{
					vty_out(vty,"sorry there is no neighbor ap\n");
				}
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}

		//for local hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 1;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}
		
			local_hansi_parameter:
				RSSI = dcli_ac_show_api_group_three(
					profile,
					FIFTH,/*"show neighbor rssi bywtpid ID"*/
					wtpid,
					0,
					0,
					&ret,
					0,
					&localid,
					dcli_dbus_connection,
					WID_DBUS_CONF_METHOD_SHOW_NEIGHBOR_RSSI_INFO
					);
			    vty_out(vty,"==============================================================================\n");
			    vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
			    vty_out(vty,"-----------------------------------------------------------------------------\n");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0){	
							
					vty_out(vty,"wtp %d neighbor ap rssi list \n",wtpid);
					vty_out(vty,"txpower:%d\n",RSSI->txpowr);	
					vty_out(vty,"rssi list:%d %d %d %d \n",RSSI->rssi[0],RSSI->rssi[1],RSSI->rssi[2],RSSI->rssi[3]);	
					dcli_ac_free_fun_three(WID_DBUS_CONF_METHOD_SHOW_NEIGHBOR_RSSI_INFO,RSSI);
					RSSI = NULL;
				}
				else if (ret == WTP_ID_NOT_EXIST)
				{
					vty_out(vty,"<error> wtp %d does not exist\n",wtpid);
				}
				else if(ret == WID_AP_SCANNING_DISABLE)
				{
					vty_out(vty,"radio resource managment is disable please enable first\n");
				}
				else if(ret == WID_TRANSMIT_POWER_CONTROL_DISABLE)
				{
					vty_out(vty,"transmit power control is disable please enable first\n");
				}	
				else if(ret == WTP_ID_LARGE_THAN_MAX)
				{
					vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
				}
				else
				{
					vty_out(vty,"sorry there is no neighbor ap\n");
				}
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}
	}
	
	return CMD_SUCCESS;		
}
DEFUN(set_transmit_power_control_scope_func,
	  set_transmit_power_control_scope_cmd,
	  "set transmit power control scope (own|all)",
	  CONFIG_STR
	  "set transmit power control scope config \n"
	  "set transmit power control scope config \n"
	  "set transmit power control scope config\n"
	  "set transmit power control scope config\n"
	  "set transmit power control scope vaule\n"
	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	unsigned char scope;
	int ret;

	if (!strcmp(argv[0],"own"))
	{
		scope = 0;	
	}		
	else if (!strcmp(argv[0],"all"))
	{
		scope = 1;
	}
	else
	{
		//printf("<error> input patameter should only be 'own' or 'all' *****\n");
		return CMD_SUCCESS;
	}

	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_CONTROL_SCOPE);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_CONTROL_SCOPE);*/


	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&scope,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		vty_out(vty,"set transmit power control scope %s successfully\n",argv[0]);
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;		
	
}
DEFUN(set_wirelesscontrol_auto_ap_switch_func,
	  set_wirelesscontrol_auto_ap_switch_cmd,
	  "set auto_ap_login switch (enable|disable)",
	  "wireless-control config\n"
	  "auto_ap_login_switch config\n"
	  "auto_ap_login_switch enable|disable\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    unsigned int dynamic_ap_login_switch = 0;

	
	if (!strcmp(argv[0],"enable"))
	{
		dynamic_ap_login_switch = 1;	
	}
	else if (!strcmp(argv[0],"disable"))
	{
		dynamic_ap_login_switch = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter should only be 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_SWITCH);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_SWITCH);*/
	
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&dynamic_ap_login_switch,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"set wireless-control dynamic_ap_login_switch %s successfully\n",argv[0]);
	}				
	else if(ret == AUTO_AP_LOGIN_INTERFACE_NOT_SET)
	{
		vty_out(vty,"<error> auto_ap_login interface has not set yet\n");
	}
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}
DEFUN(set_wireless_memory_trace_switch_func,
	  set_wireless_memory_trace_switch_cmd,
	  "set wid_memory_trace_switch (enable|disable)",
	  "wid memory trace config\n"
	  "wireless_memory_trace_switch config\n"
	  "wireless_memory_trace_switch enable|disable\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    unsigned int log_print_switch = 0;

	if (!strcmp(argv[0],"enable"))
	{
		log_print_switch = 1;	
	}
	else if (!strcmp(argv[0],"disable"))
	{
		log_print_switch = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter should only be 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_MEMORY_TRACE_SWITCH);
	
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&log_print_switch,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"set wid_memory_trace_switch %s successfully\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}

DEFUN(set_wirelesscontrol_auto_ap_binding_l3_interface_func,
	  set_wirelesscontrol_auto_ap_binding_l3_interface_cmd,
	  "set auto_ap_login interface (uplink|downlink) IFNAME",
	  "wireless-control config\n"
	  "auto_ap_login_binding_l3_interface config\n"
	  "auto_ap_login_binding_l3_interface\n"
	  "auto_ap_login_binding_l3_interface add/remove\n"
	  "auto_ap_login_binding_l3_interface IFNAME\n"
	 )
{
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = 0;
	int len = 0;
	int quitreason = 0;
	char *name;
	unsigned char policy = 2;
    
	
	len = strlen(argv[1]);
	
	if(len > 15)
	{		
		vty_out(vty,"<error> interface name is too long,should be no more than 15\n");
		return CMD_SUCCESS;
	}
	
	if (!strcmp(argv[0],"uplink"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[0],"downlink"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'uplink'or 'downlink'\n");
		return CMD_SUCCESS;
	}	
	
	name = (char*)malloc(strlen(argv[1])+1);
	memset(name, 0, strlen(argv[1])+1);
	memcpy(name, argv[1], strlen(argv[1])); 
	
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_L3_INTERFACE);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_L3_INTERFACE);*/
	
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&policy,
							 DBUS_TYPE_STRING,&name,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		free(name);
		name = NULL;
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_get_basic(&iter,&quitreason);
	/*//printf("ret %d quitreason %d\n",ret,quitreason);*/
	if(ret == 0)
	{
		vty_out(vty,"set wireless-control dynamic_ap_login_interface %s %s successfully\n",argv[0],argv[1]);
	}
	else if(ret == SWITCH_IS_DISABLE)
		vty_out(vty,"<error> auto ap login switch is enable,you should disable it first\n");
	else if(ret == APPLY_IF_FAIL)
		vty_out(vty,"<error> interface %s error, no index or interface down\n",argv[1]);
	else if(ret == IF_BINDING_FLAG){
		vty_out(vty,"<error>interface %s has be binded in other hansi.\n",name);
	}
	else
	{
		if(quitreason == IF_DOWN)
		{
			vty_out(vty,"<error> interface %s is down\n",argv[1]);
		}
		else if(quitreason == IF_NOFLAGS)
		{
			vty_out(vty,"<error> interface %s is no flags\n",argv[1]);
		}
		else if(quitreason == IF_NOINDEX)
		{
			vty_out(vty,"<error> interface %s is no index\n",argv[1]);
		}
		else
		{
			vty_out(vty,"<error>  interface %s error\n",argv[1]);
		}
	}
		
	dbus_message_unref(reply);
	free(name);
	name = NULL;
	return CMD_SUCCESS;			
}
DEFUN(set_wirelesscontrol_auto_ap_binding_l3_interface_new_func,
	  set_wirelesscontrol_auto_ap_binding_l3_interface_new_cmd,
	  "set auto_ap_login interface (add|del) IFNAME",
	  "wireless-control config\n"
	  "auto_ap_login_binding_l3_interface config\n"
	  "auto_ap_login_binding_l3_interface\n"
	  "auto_ap_login_binding_l3_interface add/del\n"
	  "auto_ap_login_binding_l3_interface IFNAME\n"
	 )
{
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = 0;
	int len = 0;
	int quitreason = 0;
	char *name = NULL;
	unsigned char policy = 2;
    
	
	len = strlen(argv[1]);
	
	if(len > 15)
	{		
		vty_out(vty,"<error> interface name is too long,should be no more than 15\n");
		return CMD_SUCCESS;
	}
	
	if (!strcmp(argv[0],"add"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[0],"del"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'add'or 'del'\n");
		return CMD_SUCCESS;
	}	
	
	name = (char*)malloc(strlen(argv[1])+1);
	if (name == NULL)
	{
		vty_out(vty, "CRITICAL ERROR: malloc error!!!\n");
		return CMD_WARNING;
	}
	memset(name, 0, strlen(argv[1])+1);
	memcpy(name, argv[1], strlen(argv[1])); 
		
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_L3_INTERFACE);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_L3_INTERFACE);*/
	
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&policy,
							 DBUS_TYPE_STRING,&name,
							 DBUS_TYPE_UINT32,&boot_flag,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		free(name);
		name = NULL;
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_get_basic(&iter,&quitreason);
	/*//printf("ret %d quitreason %d\n",ret,quitreason);*/
	if(ret == 0)
	{
		vty_out(vty,"set wireless-control dynamic_ap_login_interface %s %s successfully\n",argv[0],argv[1]);
	}
	else if (ret == IF_BINDING_FLAG)		/* Huangleilei fixed for AXSSZFI-1615 */
	{
		vty_out(vty, "<error> interface %s has be binded in other hansi.\n", argv[1]);
	}
	else if(ret == SWITCH_IS_DISABLE)
		vty_out(vty,"<error> auto ap login switch is enable,you should disable it first\n");
	else if(ret == APPLY_IF_FAIL)
		vty_out(vty,"<error> interface %s error, no index or interface down\n",argv[1]);
	else if (ret == INTERFACE_NOT_BE_BINDED)
	{
		vty_out(vty, "<error>interface %s has not been added or has already been deleted. ", argv[1]);
	}
	else
	{
		if(quitreason == IF_DOWN)
		{
			vty_out(vty,"<error> interface %s is down\n",argv[1]);
		}
		else if(quitreason == IF_NOFLAGS)
		{
			vty_out(vty,"<error> interface %s is no flags\n",argv[1]);
		}
		else if(quitreason == IF_NOINDEX)
		{
			vty_out(vty,"<error> interface %s is no index\n",argv[1]);
		}else if (quitreason == WID_INTERFACE_NOT_BE_LOCAL_BOARD){
			vty_out(vty,"<error> %s is no local interface, permission denial\n",argv[1]);
		}
		else
		{
			vty_out(vty,"<error>  interface %s error\n",argv[1]);
		}
	}
		
	dbus_message_unref(reply);
	free(name);
	name = NULL;
	return CMD_SUCCESS;			
}

DEFUN(set_wirelesscontrol_auto_ap_binding_wlan_func,
	  set_wirelesscontrol_auto_ap_binding_wlan_cmd,
	  "set auto_ap_login wlan ID base interface IFNAME",
	  "wireless-control config\n"
	  "auto_ap_login_binding_wlan config\n"
	  "auto_ap_login_binding_wlan wlanid\n"
	  "wlan id \n"
	  "auto_ap_login_binding_wlan base interface\n"
	  "auto_ap_login_binding_wlan base interface\n"
	  "auto_ap_login_binding_wlan base interface ifname lenth<1-15>\n"
	 )
{
	int ret,ret1;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    unsigned int wlanid = 0;
	int len = 0;
	char *name;
	
	len = strlen(argv[1]);
	if(len > 15)
	{		
		vty_out(vty,"<error> interface name is too long,should be no more than 15\n");
		return CMD_SUCCESS;
	}
	
	ret1 = parse_int_ID((char*)argv[0], &wlanid);
	if(ret1 != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format,wlanid should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	
	name = (char*)malloc(strlen(argv[1])+1);
	memset(name, 0, strlen(argv[1])+1);
	memcpy(name, argv[1], strlen(argv[1])); 
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_WLANID);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_WLANID);*/
	
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wlanid,
							 DBUS_TYPE_STRING,&name,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		free(name);
		name = NULL;
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"set wireless-control dynamic_ap_login_binding wlan %s successfully\n",argv[0]);
	}
	else if(ret == WLAN_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> wlan %s not exist\n",argv[0]);
	}
	else if(ret == Wlan_IF_NOT_BE_BINDED)
	{
		vty_out(vty,"<error> wlan %s has not bind interface %s\n",argv[0],argv[1]);
	}
	else if(ret == AUTO_AP_LOGIN_INTERFACE_ERROR)
	{
		vty_out(vty,"<error> interface %s not in the auto ap login interface\n",argv[1]);
	}
	else if(ret == AUTO_AP_LOGIN_INTERFACE_NOT_SET)
	{
		vty_out(vty,"<error> auto_ap_login interface has not set yet\n");
	}
	else if(ret == WTP_OVER_MAX_BSS_NUM)
	{
		vty_out(vty,"<error> auto_ap_login interface %s wlan num is already %d\n",argv[1],L_BSS_NUM);
	}
	else if(ret == SWITCH_IS_DISABLE)
		vty_out(vty,"<error> auto ap login switch is enable,you should disable it first\n");
	else if (ret == WID_INTERFACE_NOT_BE_LOCAL_BOARD){
		vty_out(vty,"<error> %s is no local interface, permission denial\n",argv[1]);
	}
	else if(ret == APPLY_IF_FAIL)
	{
		vty_out(vty,"<error> interface %s error, no index or interface down\n",argv[1]);
	}
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to some wlan, and the operation of the wlan was not successful\n");
	}
	else
	{
		vty_out(vty,"<error>  other unknow error: %d\n", ret);
	}
		
	dbus_message_unref(reply);
	free(name);
	name = NULL;
	return CMD_SUCCESS;			
}
DEFUN(del_wirelesscontrol_auto_ap_binding_wlan_func,
	  del_wirelesscontrol_auto_ap_binding_wlan_cmd,
	  "del auto_ap_login wlan ID base interface IFNAME",
	  "wireless-control config\n"
	  "auto_ap_login_binding_wlan config\n"
	  "auto_ap_login_binding_wlan wlanid\n"
	  "wlan id \n"
	  "auto_ap_login_binding_wlan base interface\n"
	  "auto_ap_login_binding_wlan base interface\n"
	  "auto_ap_login_binding_wlan base interface ifname lenth<1-15>\n"
	 )
{
	int ret,ret1;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    unsigned int wlanid = 0;
	int len = 0;
	char *name;
	
	len = strlen(argv[1]);
	if(len > 15)
	{		
		vty_out(vty,"<error> interface name is too long,should be no more than 15\n");
		return CMD_SUCCESS;
	}
	
	ret1 = parse_int_ID((char*)argv[0], &wlanid);
	if(ret1 != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format,wlanid should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	
	name = (char*)malloc(strlen(argv[1])+1);
	memset(name, 0, strlen(argv[1])+1);
	memcpy(name, argv[1], strlen(argv[1])); 
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_DEL_WID_DYNAMIC_AP_LOGIN_WLANID);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_DEL_WID_DYNAMIC_AP_LOGIN_WLANID);*/
	
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wlanid,
							 DBUS_TYPE_STRING,&name,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		free(name);
		name = NULL;
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"del wireless-control dynamic_ap_login_binding wlan %s successfully\n",argv[0]);
	}
	else if(ret == WLAN_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> wlan %s not exist\n",argv[0]);
	}
	else if(ret == Wlan_IF_NOT_BE_BINDED)
	{
		vty_out(vty,"<error> wlan %s has not bind interface %s\n",argv[0],argv[1]);
	}
	else if(ret == AUTO_AP_LOGIN_INTERFACE_ERROR)
	{
		vty_out(vty,"<error> interface %s not in the auto ap login interface\n",argv[1]);
	}
	else if(ret == AUTO_AP_LOGIN_INTERFACE_NOT_SET)
	{
		vty_out(vty,"<error> auto_ap_login interface has not set yet\n");
	}
	else if(ret == WTP_OVER_MAX_BSS_NUM)
	{
		vty_out(vty,"<error> auto_ap_login interface %s wlan num is 0\n",argv[1]);
	}
	else if(ret == SWITCH_IS_DISABLE)
	{
		vty_out(vty,"<error> auto ap login switch is enable,you should disable it first\n");
	}
	else if(ret == APPLY_IF_FAIL)
	{
		vty_out(vty,"<error>input interface dosen't exist!\n");
	}
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to some wlan, and the operation of the wlan was not successful\n");
	}
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	free(name);
	name = NULL;
	return CMD_SUCCESS;			
}

DEFUN(set_wirelesscontrol_auto_ap_save_config_switch_func,
	  set_wirelesscontrol_auto_ap_save_config_switch_cmd,
	  "set auto_ap_login save_config_switch (enable|disable)",
	  "wireless-control config\n"
	  "auto_ap_login_save_config_switch config\n"
	  "auto_ap_login_save_config_switch enable|disable\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    unsigned int dynamic_ap_login_save_config_switch = 0;

	
	if (!strcmp(argv[0],"enable"))
	{
		dynamic_ap_login_save_config_switch = 1;	
	}
	else if (!strcmp(argv[0],"disable"))
	{
		dynamic_ap_login_save_config_switch = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter should only be 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_SAVE_CONFIG_SWITCH);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_DYNAMIC_AP_LOGIN_SAVE_CONFIG_SWITCH);*/
	
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&dynamic_ap_login_save_config_switch,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"set wireless-control dynamic_ap_login_save_config_switch %s successfully\n",argv[0]);
	}
	else if(ret == SWITCH_IS_DISABLE)
		vty_out(vty,"<error> auto ap login switch is enable,you should disable it first\n");
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}
DEFUN(show_auto_ap_config_func,
		  show_auto_ap_config_cmd,
		  "show auto_ap_login config [remote] [local] [PARAMETER]",
		  "show auto ap login config\n"
		  "show auto ap login config\n"
		  "show auto ap login config\n"
		  "'remote' or 'local' hansi\n"
		  "'remote' or 'local' hansi\n"
		  "slotid-instid\n"
	 )
{
	int ret = 0;
	char *ifname;
	int wlanid = 0;
	int login_switch = 0;
	int save_switch = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;

	char dis[] = "disable";
	char en[] = "enable";
	int state = 0;
	int i = 0;
	int j = 0;
/*	wid_auto_ap_info info;
		info.auto_ap_switch = 0;
		info.save_switch = 0;
		info.ifnum = 0;
		info.auto_ap_if = NULL;
*/
	//wid_auto_ap_if ifinfo;
	DCLI_AC_API_GROUP_FIVE *auto_ap_login = NULL;
/*	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
*/	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	if((argc == 1)||(argc == 3)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 2){
		if (!strcmp(argv[0],"remote")){
			localid = 0;
		}else if(!strcmp(argv[0],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[0],"remote"))&&(!strcmp(argv[1],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[1],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[1], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}
#if 0	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WID_DYNAMIC_AP_LOGIN_SAVE_CONFIG);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WID_DYNAMIC_AP_LOGIN_SAVE_CONFIG);*/
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&info.auto_ap_switch);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&info.save_switch);

	vty_out(vty,"Wireless control auto ap login configure summary\n");
	vty_out(vty,"==============================================================================\n");
	vty_out(vty,"AUTO AP LOGIN SWITCH: %s\n",(info.auto_ap_switch == 1)?en:dis);
	vty_out(vty,"AUTO AP LOGIN SAVE CONFIG SWITCH: %s\n",(info.save_switch == 1)?en:dis);
	
	

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&info.ifnum);
	vty_out(vty,"AUTO AP LOGIN BINDING INTERFACE NUM: %d\n",info.ifnum);
	for(i=0;i<info.ifnum;i++)
	{
		vty_out(vty,"--------------------------------------------------\n");
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&ifname);
	
		vty_out(vty,"AUTO AP LOGIN BINDING INTERFACE: %s\n",ifname);

		ifinfo.ifindex = 0;
		ifinfo.wlannum = 0;
		memset(ifinfo.wlanid,0,L_BSS_NUM);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&ifinfo.ifindex);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&ifinfo.wlannum);
		vty_out(vty,"	WLAN num:	%d\n",ifinfo.wlannum);
		for(j=0;j<L_BSS_NUM;j++)
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&ifinfo.wlanid[j]);
			if(ifinfo.wlanid[j] != 0)
			{
				vty_out(vty,"	WLAN:	wlan %d\n",ifinfo.wlanid[j]);
			}
		}
	
	}
	vty_out(vty,"==============================================================================\n");
#endif
	if(vty->node != VIEW_NODE){
		auto_ap_login = dcli_ac_show_api_group_five(
			index,
			THIRD,/*"show auto_ap_login config"*/
			0,
			0,
			1,
			&ret,
			0,
			0,
			localid,
			//auto_ap_login,
			dcli_dbus_connection,
			WID_DBUS_CONF_METHOD_SHOW_WID_DYNAMIC_AP_LOGIN_SAVE_CONFIG
			);
		if(ret == -1){
			cli_syslog_info("<error> failed get reply.\n");
		}
		else/* if(ret == 0)*/{	
			if((auto_ap_login != NULL)&&(auto_ap_login->auto_login != NULL)){
				vty_out(vty,"Wireless control auto ap login configure summary\n");
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"AUTO AP LOGIN SWITCH: %s\n",(auto_ap_login->auto_login->auto_ap_switch == 1)?en:dis);
				vty_out(vty,"AUTO AP LOGIN SAVE CONFIG SWITCH: %s\n",(auto_ap_login->auto_login->save_switch == 1)?en:dis);
				
				vty_out(vty,"AUTO AP LOGIN BINDING INTERFACE NUM: %d\n",auto_ap_login->auto_login->ifnum);
				//int ifnum = (int)auto_ap_login->auto_login->ifnum;
				//for(i=0;i<ifnum ;i++)
				if(auto_ap_login->auto_login->auto_ap_if){
					//vty_out(vty,"auto_ap_if !=NULL	---------------------------------------\n");
					wid_auto_ap_if *head = NULL;
					head = auto_ap_login->auto_login->auto_ap_if;
				while(head)
				{	
					vty_out(vty,"--------------------------------------------------\n");
				
					vty_out(vty,"AUTO AP LOGIN BINDING INTERFACE: %s\n",head->ifname);

				//	head->auto_ap_if->ifindex = 0;
				//	head->auto_ap_if->wlannum = 0;
				//	memset(head->auto_ap_if->wlanid,0,L_BSS_NUM);

					vty_out(vty,"	WLAN num:	%d\n",head->wlannum);
					for(j=0;j<L_BSS_NUM;j++)
					{
						if(head->wlanid[j] != 0)
						{
							vty_out(vty,"	WLAN:	wlan %d\n",head->wlanid[j]);
						}
					}
					head = head->ifnext;
				}
				}
				vty_out(vty,"==============================================================================\n");
			}
			dcli_ac_free_fun_five(WID_DBUS_CONF_METHOD_SHOW_WID_DYNAMIC_AP_LOGIN_SAVE_CONFIG,auto_ap_login);
			auto_ap_login = NULL;
		}
		//else vty_out(vty,"ret is %d.\n",ret);
	//	CW_FREE_OBJECT(auto_ap_login->auto_login->auto_ap_if);	
	//	CW_FREE_OBJECT(auto_ap_login->auto_login);		
	//	CW_FREE_OBJECT(auto_ap_login);		
	//	dbus_message_unref(reply);
	}

	if(vty->node == VIEW_NODE){
		//for remote hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

		hansi_parameter:
				auto_ap_login = dcli_ac_show_api_group_five(
					profile,
					THIRD,/*"show auto_ap_login config"*/
					0,
					0,
					1,
					&ret,
					0,
					0,
					localid,
					dcli_dbus_connection,
					WID_DBUS_CONF_METHOD_SHOW_WID_DYNAMIC_AP_LOGIN_SAVE_CONFIG
					);
			    vty_out(vty,"==============================================================================\n");
			    vty_out(vty,"hansi %d-%d\n",slot_id,profile);
			    vty_out(vty,"-----------------------------------------------------------------------\n");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else/* if(ret == 0)*/{	
					if((auto_ap_login != NULL)&&(auto_ap_login->auto_login != NULL)){
						vty_out(vty,"Wireless control auto ap login configure summary\n");
						vty_out(vty,"=======================================================================\n");
						vty_out(vty,"AUTO AP LOGIN SWITCH: %s\n",(auto_ap_login->auto_login->auto_ap_switch == 1)?en:dis);
						vty_out(vty,"AUTO AP LOGIN SAVE CONFIG SWITCH: %s\n",(auto_ap_login->auto_login->save_switch == 1)?en:dis);
						
						vty_out(vty,"AUTO AP LOGIN BINDING INTERFACE NUM: %d\n",auto_ap_login->auto_login->ifnum);
						if(auto_ap_login->auto_login->auto_ap_if){
							wid_auto_ap_if *head = NULL;
							head = auto_ap_login->auto_login->auto_ap_if;
						while(head)
						{	
							vty_out(vty,"--------------------------------------------------\n");
							vty_out(vty,"AUTO AP LOGIN BINDING INTERFACE: %s\n",head->ifname);
							vty_out(vty,"	WLAN num:	%d\n",head->wlannum);
							for(j=0;j<L_BSS_NUM;j++)
							{
								if(head->wlanid[j] != 0)
								{
									vty_out(vty,"	WLAN:	wlan %d\n",head->wlanid[j]);
								}
							}
							head = head->ifnext;
						}
						}
					}
					dcli_ac_free_fun_five(WID_DBUS_CONF_METHOD_SHOW_WID_DYNAMIC_AP_LOGIN_SAVE_CONFIG,auto_ap_login);
					auto_ap_login = NULL;
				}
				vty_out(vty,"==============================================================================\n");
				if(argc == 2){
					return CMD_SUCCESS;
				}
			}
		}

	//for local hansi info
	for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		localid = 1;
		
		for (profile = 1; profile < MAX_INSTANCE; profile++) 
		{
			instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
			if (INSTANCE_NO_CREATED == instRun) {
				continue;
			}
	
		local_hansi_parameter:
			auto_ap_login = dcli_ac_show_api_group_five(
				profile,
				THIRD,/*"show auto_ap_login config"*/
				0,
				0,
				1,
				&ret,
				0,
				0,
				localid,
				dcli_dbus_connection,
				WID_DBUS_CONF_METHOD_SHOW_WID_DYNAMIC_AP_LOGIN_SAVE_CONFIG
				);
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
			vty_out(vty,"-----------------------------------------------------------------------\n");
			if(ret == -1){
				cli_syslog_info("<error> failed get reply.\n");
			}
			else/* if(ret == 0)*/{	
				if((auto_ap_login != NULL)&&(auto_ap_login->auto_login != NULL)){
					vty_out(vty,"Wireless control auto ap login configure summary\n");
					vty_out(vty,"=======================================================================\n");
					vty_out(vty,"AUTO AP LOGIN SWITCH: %s\n",(auto_ap_login->auto_login->auto_ap_switch == 1)?en:dis);
					vty_out(vty,"AUTO AP LOGIN SAVE CONFIG SWITCH: %s\n",(auto_ap_login->auto_login->save_switch == 1)?en:dis);
					
					vty_out(vty,"AUTO AP LOGIN BINDING INTERFACE NUM: %d\n",auto_ap_login->auto_login->ifnum);
					if(auto_ap_login->auto_login->auto_ap_if){
						wid_auto_ap_if *head = NULL;
						head = auto_ap_login->auto_login->auto_ap_if;
					while(head)
					{	
						vty_out(vty,"--------------------------------------------------\n");
						vty_out(vty,"AUTO AP LOGIN BINDING INTERFACE: %s\n",head->ifname);
						vty_out(vty,"	WLAN num:	%d\n",head->wlannum);
						for(j=0;j<L_BSS_NUM;j++)
						{
							if(head->wlanid[j] != 0)
							{
								vty_out(vty,"	WLAN:	wlan %d\n",head->wlanid[j]);
							}
						}
						head = head->ifnext;
					}
					}
				}
				dcli_ac_free_fun_five(WID_DBUS_CONF_METHOD_SHOW_WID_DYNAMIC_AP_LOGIN_SAVE_CONFIG,auto_ap_login);
				auto_ap_login = NULL;
			}
			vty_out(vty,"==============================================================================\n");
			if(argc == 2){
				return CMD_SUCCESS;
			}
			}
		}
	}
	
	return CMD_SUCCESS;			
}
DEFUN(clear_auto_ap_config_func,
	  clear_auto_ap_config_cmd,
	  "clear auto_ap_login config",
	  "clear auto ap login config\n"
	  "clear auto ap login config\n"
	  "clear auto ap login config\n"
	 )
{
	int ret = 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_CLEAR_WID_DYNAMIC_AP_LOGIN_CONFIG);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_CLEAR_WID_DYNAMIC_AP_LOGIN_CONFIG);*/
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"clear Wireless control auto ap login configure successfully\n");
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;			
}
/*xiaodawei add wireless_data and wired_data switch, 20110511*/
/*	wireless_data switch:1 means wireless_data_pkts(bytes)=wireless_data_pkts(bytes)-multicast_pkts(multicast_bytes)
						  0 means no treatment
	wired_data switch:1 means eth_data_pkts(bytes)=wireless_data_pkts(bytes)+random
						  0 means no treatment
*/
DEFUN(set_ap_data_exclude_multicast_func,
		  set_ap_data_exclude_multicast_cmd,
		  "set ap (wireless_data|wired_data) revise (enable|disable)",
		  "ap (wireless_data|wired_data)  config\n"
		  "ap (wireless_data|wired_data)  config\n"
		  "ap wireless_data config\n"
		  "ap wired_data config\n"
		  "set ap (wireless_data|wired_data) revise (enable|disable)\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char datatype = 0;
    	unsigned char policy = 0;

	
	if (!strcmp(argv[0],"wireless_data"))
	{
		datatype = 0;	
	}
	else if (!strcmp(argv[0],"wired_data"))
	{
		datatype = 1;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'wireless_data' or 'wired_data'\n");
		return CMD_SUCCESS;
	}
	if (!strcmp(argv[1],"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[1],"disable"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_DATA_EXCLUDE_MULTICAST);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AP_STATISTICS);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&datatype,
							 DBUS_TYPE_BYTE,&policy,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"set ap %s revise %s successfully\n",argv[0],argv[1]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}

/*END*/

DEFUN(set_ap_statistics_func,
		  set_ap_statistics_cmd,
		  "set ap statistics (enable|disable)",
		  "ap statistics config\n"
		  "ap statistics information enable|disable\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    int policy = 0;

	
	if (!strcmp(argv[0],"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[0],"disable"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_STATISTICS);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AP_STATISTICS);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," set ap statistics %s successfully\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}

DEFUN(set_ap_statistics_inter_func,
		  set_ap_statistics_inter_cmd,
		  "set ap statistics interval VALUE",
		  "ap statistics config\n"
		  "ap statistics information enable|disable\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	unsigned int interval = 0;
	unsigned int wtpid = 0;
	int index = 0;
	ret = parse_int_ID((char*)argv[0], &interval);

	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
			    vty_out(vty,"<error> unknown id format\n");
			}
			return CMD_SUCCESS;
	}	
	if ((interval <= 0) || (interval > 32767)) {   /*fengwenchao change 30 to 32767*/
		vty_out(vty,"<error> input interval should be 1 to 32767\n");
		return CMD_FAILURE;
	}
	int localid = 1;
	int slot_id = HostSlotId;
	if(vty->node == CONFIG_NODE)
	{
		index = 0;
		wtpid = 0;
	}else if(vty->node == WTP_NODE)
	{
		index = 0;
		wtpid = (unsigned int)vty->index;
	}else if(vty->node == HANSI_NODE)
	{
		index = (unsigned int)vty->index;
		wtpid = 0;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == HANSI_WTP_NODE)
	{
		index = (unsigned int)vty->index;
		wtpid = (unsigned int)vty->index_sub;
		localid = vty->local;
		slot_id  = vty->slotindex;
	}/*else if(vty->node == LOCAL_HANSI_WTP_NODE){
		index = (unsigned int)vty->index;
		wtpid = (unsigned int)vty->index_sub;
		localid = vty->local;
		slot_id  = vty->slotindex;
	}*/else if(vty->node == LOCAL_HANSI_NODE){
		wtpid = 0;
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	if(/*(wtpid<0)||*/(wtpid>(WTP_NUM-1))){
		vty_out(vty,"invalid wtpid  %d\n",wtpid);
		return CMD_FAILURE;
	}
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	//printf("wtpid:%d,index:%d\n",wtpid,index);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_STATISTICS_INTERVAL);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_UINT32,&interval,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(ret == 0)
	{
		vty_out(vty," set wtp %d statistics interval %s successfully\n",wtpid,argv[0]);
	}				
	else
	{
		vty_out(vty,"<error>  service wtp set failed \n");
	}
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}

DEFUN(show_ap_statistics_list_func,
		  show_ap_statistics_list_bywtpid_cmd,
		  "show ap statistics list bywtp ID [remote] [local] [PARAMETER]",
		  SHOW_STR
		  "show ap statistics list\n"
		  "show ap statistics list\n"
		  "show ap statistics list\n"
		  "show ap statistics list\n"
		  "WTPID\n"
		  "'remote' or 'local' hansi\n"
		  "'remote' or 'local' hansi\n"
		  "slotid-instid\n"
	 )
{
	int ret = -1;
	int num = 0;
	int i = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter,iter_array;
	DBusError err;

	int wtpid = 0;

	wtpid = atoi(argv[0]);
	if ((wtpid <= 0) || (wtpid >= WTP_NUM)) {
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_FAILURE;
		}

	DCLI_AC_API_GROUP_THREE *statics = NULL;
	//CW_CREATE_OBJECT_ERR(statics, DCLI_AC_API_GROUP_THREE, return NULL;);	
	//statics->ap_statics_list = NULL;
	//statics->WTPIP = NULL;
	int index = 0;
//	char BUSNAME[PATH_LEN];
//	char OBJPATH[PATH_LEN];
//	char INTERFACE[PATH_LEN];
	int localid = 1;
	int slot_id = HostSlotId;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	if((argc == 2)||(argc == 4)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 3){
		if (!strcmp(argv[1],"remote")){
			localid = 0;
		}else if(!strcmp(argv[1],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[1],"remote"))&&(!strcmp(argv[2],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[2],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[2], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}
#if 0
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_SHOW_STATISTICS_LIST);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AP_SHOW_STATISTICS_LIST);*/
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret == 0 )
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		vty_out(vty,"show ap statistics list \n");
		vty_out(vty,"==============================================================================\n");
/*		vty_out(vty,"%-6s %-17s %-7s %-7s %-6s %-6s %-7s %-7s %-6s %-6s\n","type","mac","rx_pkt","tx_pkt","rx_err","tx_err","rx_bts","tx_bts","rx_rt","tx_rt");*/
		for (i = 0; i < num; i++)
		{
			DBusMessageIter iter_struct;
			
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(ap_statics.type));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(ap_statics.radioId));
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(ap_statics.wlanId));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(ap_statics.mac[0]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(ap_statics.mac[1]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(ap_statics.mac[2]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(ap_statics.mac[3]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(ap_statics.mac[4]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(ap_statics.mac[5]));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(ap_statics.rx_packets));
			dbus_message_iter_next(&iter_struct);	
					
			dbus_message_iter_get_basic(&iter_struct,&(ap_statics.tx_packets));
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(ap_statics.rx_errors));
			dbus_message_iter_next(&iter_struct);	
					
			dbus_message_iter_get_basic(&iter_struct,&(ap_statics.tx_errors));
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(ap_statics.rx_bytes));
			dbus_message_iter_next(&iter_struct);	
					
			dbus_message_iter_get_basic(&iter_struct,&(ap_statics.tx_bytes));
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(ap_statics.rx_rate));
			dbus_message_iter_next(&iter_struct);	
					
			dbus_message_iter_get_basic(&iter_struct,&(ap_statics.tx_rate));
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(ap_statics.ast_rx_crcerr));
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(ap_statics.ast_rx_badcrypt));
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(ap_statics.ast_rx_badmic));
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(ap_statics.ast_rx_phyerr));
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(ap_statics.rx_drop));
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(ap_statics.tx_drop));
			
			dbus_message_iter_next(&iter_array);

			if(ap_statics.type == 0)
			{
				sprintf(ap_statics.ifname,"ath%d",ap_statics.wlanId);
				if(ap_statics.rx_rate > 1)
				{						
					ap_statics.rx_band = ((int)((float)ap_statics.rx_rate/20))%100;
				}
				else
				{
					ap_statics.rx_band = 0;
				}
				if(ap_statics.tx_rate > 1)
				{						
					ap_statics.tx_band = ((int)((float)ap_statics.tx_rate/15))%100;		
				}
				else
				{
					ap_statics.tx_band = 0;	
				}				
			}
			else if(ap_statics.type == 1)
			{
				sprintf(ap_statics.ifname,"eth%d",ap_statics.wlanId);
				if(ap_statics.rx_rate > 1)
				{						
					ap_statics.rx_band = ((int)((float)ap_statics.rx_rate/100))%100;
				}
				else
				{
					ap_statics.rx_band = 0;
				}
				if(ap_statics.tx_rate > 1)
				{						
					ap_statics.tx_band = ((int)((float)ap_statics.tx_rate/100))%100;		
				}
				else
				{
					ap_statics.tx_band = 0;	
				}					
			}
			else
			{
				sprintf(ap_statics.ifname,"wifi%d",ap_statics.wlanId);
				if(ap_statics.rx_rate > 1)
				{						
					ap_statics.rx_band = ((int)((float)ap_statics.rx_rate/20))%100;
				}
				else
				{
					ap_statics.rx_band = 0;
				}	
				if(ap_statics.tx_rate > 1)
				{						
					ap_statics.tx_band = ((int)((float)ap_statics.tx_rate/15))%100;		
				}
				else
				{
					ap_statics.tx_band = 0;	
				}
			}
			vty_out(vty,"%-6s %-17s %-7s %-7s %-6s %-6s %-7s %-7s %-6s %-6s\n","type","mac","rx_pkt","tx_pkt","rx_err","tx_err","rx_bts","tx_bts","rx_rt","tx_rt");
			vty_out(vty,"%-6s %02X:%02X:%02X:%02X:%02X:%02X %-7d %-7d %-6d %-6d %-7d %-7d %-6d %-6d \n\n",\
				ap_statics.ifname,ap_statics.mac[0],ap_statics.mac[1],ap_statics.mac[2],\
				ap_statics.mac[3],ap_statics.mac[4],ap_statics.mac[5],ap_statics.rx_packets,ap_statics.tx_packets,\
				ap_statics.rx_errors,ap_statics.tx_errors,ap_statics.rx_bytes,ap_statics.tx_bytes,ap_statics.rx_rate,ap_statics.tx_rate);
			vty_out(vty,"%-8s %-8s %-8s %-8s %-8s %-8s %-8s %-10s %-10s\n","CRC_err","key_err","MIC_err","phy_err","WlanID","rx_drop","tx_drop","rx_band","tx_band");
			vty_out(vty,"%-8d %-8d %-8d %-8d %-8d %-8d %-8d %-10d %-10d\n\n",ap_statics.ast_rx_crcerr,ap_statics.ast_rx_badcrypt,ap_statics.ast_rx_badmic,ap_statics.ast_rx_phyerr,\
				ap_statics.wlanId,ap_statics.rx_drop,ap_statics.tx_drop,ap_statics.rx_band,ap_statics.tx_band);
		}
		vty_out(vty,"==============================================================================\n");
	}
#endif
	if(vty->node != VIEW_NODE){
		statics = dcli_ac_show_api_group_three(
			index,
			SIXTH,/*"show ap statistics list bywtp ID"*/
			wtpid,
			0,
			0,
			&ret,
			0,
			&localid,
			//statics,
			dcli_dbus_connection,
			WID_DBUS_CONF_METHOD_AP_SHOW_STATISTICS_LIST
			);
		//printf("#######ret is %d \n",ret);
		if(ret == -1){
			cli_syslog_info("<error> failed get reply.\n");
		}
		else if(ret == 0){	
			if((statics)&&(statics->ap_statics_list)&&(statics->ap_statics_list->ap_statics_ele)){
				wlan_stats_info *head = NULL;
				head = statics->ap_statics_list->ap_statics_ele;
				vty_out(vty,"show ap statistics list \n");
				vty_out(vty,"==============================================================================\n");
				while(head){	
#if 0
					vty_out(vty,"%-6s %-17s %-7s %-7s %-6s %-6s %-7s %-7s %-6s %-6s\n","type","mac","rx_pkt","tx_pkt","rx_err","tx_err","rx_bts","tx_bts","rx_rt","tx_rt");
					vty_out(vty,"%-6s %02X:%02X:%02X:%02X:%02X:%02X %-7d %-7d %-6d %-6d %-7llu %-7llu %-6d %-6d \n\n",\
						head->ifname,head->mac[0],head->mac[1],head->mac[2],\
						head->mac[3],head->mac[4],head->mac[5],head->rx_packets,head->tx_packets,\
						head->rx_errors,head->tx_errors,head->rx_bytes,head->tx_bytes,head->rx_rate,head->tx_rate);
					vty_out(vty,"%-8s %-8s %-8s %-8s %-8s %-8s %-8s %-10s %-10s\n","CRC_err","key_err","MIC_err","phy_err","WlanID","rx_drop","tx_drop","rx_band","tx_band");
					vty_out(vty,"%-8d %-8d %-8d %-8d %-8d %-8d %-8d %-10d %-10d\n\n",head->ast_rx_crcerr,head->ast_rx_badcrypt,head->ast_rx_badmic,head->ast_rx_phyerr,\
						head->wlanId,head->rx_drop,head->tx_drop,head->rx_band,head->tx_band);
					vty_out(vty,"%-8s %-8s %-12s %-12s\n","rx_mgmt","tx_mgmt","rx_pkt_mgmt","tx_pkt_mgmt");
					vty_out(vty,"%-8d %-8d %-12d %-12d\n",head->rx_mgmt,head->tx_mgmt,head->rx_pkt_mgmt,head->tx_pkt_mgmt);
#endif
	                vty_out(vty,"%-6s %-17s %-7s %-7s %-6s %-6s %-7s %-7s %-6s \n","type","mac","rx_pkt","tx_pkt","rx_err","tx_err","rx_bts","tx_bts","rx_rt");
	                vty_out(vty,"%-6s %02X:%02X:%02X:%02X:%02X:%02X %-7d %-7d %-6d %-6d %-7llu %-7llu %-6d \n\n",\
	                    head->ifname,head->mac[0],head->mac[1],head->mac[2],\
	                    head->mac[3],head->mac[4],head->mac[5],head->rx_packets,head->tx_packets,\
	                    head->rx_errors,head->tx_errors,head->rx_bytes,head->tx_bytes,head->rx_rate);

	                vty_out(vty,"%-6s %-8s %-8s %-8s %-8s %-7s %-8s %-8s %-8s\n","tx_rt","CRC_err","key_err","MIC_err","phy_err","WlanID","rx_drop","tx_drop","rx_band");
	                vty_out(vty,"%-6d %-8d %-8d %-8d %-8d %-7d %-8d %-8d %-8d \n\n",head->tx_rate,head->ast_rx_crcerr,head->ast_rx_badcrypt,head->ast_rx_badmic,head->ast_rx_phyerr,\
	                    head->wlanId,head->rx_drop,head->tx_drop,head->rx_band);

	                /*zhaoruijia,添加ap显示信息将wlan_stats_info_profile结构体中数据显示完全，start*/

	                vty_out(vty,"%-8s %-9s %-9s %-15s %-15s %-14s \n","tx_band","rx_frame","tx_frame","rx_error_frame","tx_error_frame","rx_drop_frame"); 
	                vty_out(vty,"%-8u %-9u %-9u %-15u %-15u %-14u \n\n",head->tx_band,head->rx_frame,head->tx_frame,head->rx_error_frame,head->tx_error_frame,\
	                                                                head->rx_drop_frame);


	                vty_out(vty,"%-14s %-11s %-11s %-13s %-13s %-13s  \n","tx_drop_frame","rx_unicast","tx_unicast","rx_multicast","tx_multicast","rx_broadcast"); 
	                vty_out(vty,"%-14u %-11llu %-11llu %-13u %-13u %-13u \n\n",head->tx_drop_frame,head->rx_unicast,head->tx_unicast,head->rx_multicast,\
	                                                                   head->tx_multicast,head->rx_broadcast);

	                vty_out(vty,"%-13s %-15s %-15s %-17s %-17s  \n","tx_broadcast","rx_pkt_unicast","tx_pkt_unicast","rx_pkt_multicast", "tx_pkt_multicast");
	                vty_out(vty,"%-13u %-15u %-15u %-17u %-17u \n\n",head->tx_broadcast,head->rx_pkt_unicast,head->tx_pkt_unicast,head->rx_pkt_multicast,head->tx_pkt_multicast);
	                                                           

	                vty_out(vty,"%-17s %-17s %-13s %-13s %-12s   \n","rx_pkt_broadcast","tx_pkt_broadcast","rx_pkt_retry","tx_pkt_retry","rx_pkt_data"); 
	                vty_out(vty,"%-17u %-17u %-13u %-13u %-12u \n\n",head->rx_pkt_broadcast,head->tx_pkt_broadcast,head->rx_pkt_retry,head->tx_pkt_retry,head->rx_pkt_data);


	                vty_out(vty,"%-12s %-9s %-9s %-8s %-8s %-12s %-12s\n","tx_pkt_data","rx_retry","tx_retry","rx_mgmt","tx_mgmt","rx_pkt_mgmt","tx_pkt_mgmt");
	                vty_out(vty,"%-12u  %-9u %-9u %-8llu %-8llu %-12u %-12u\n",head->tx_pkt_data,head->rx_retry,head->tx_retry,head->rx_mgmt,head->tx_mgmt,head->rx_pkt_mgmt,head->tx_pkt_mgmt);
	                /*zhaoruijia,添加ap显示信息将wlan_stats_info_profile结构体中数据显示完全，end*/

	                /* zhangshu append to show "rx_sum_bytes" and "tx_sum_bytes"  */
	                /*zhaoruijia,20100907,添加rx_pkt_control，tx_pkt_control */
	                /* zhangshu append for showing rx_errors_frames , 2010-09-26 */
	                vty_out(vty,"%-12s %-12s %-15s %-15s %-12s\n %","rx_sum_bytes","tx_sum_bytes","rx_pkt_control","tx_pkt_control","rx_errors_frames"); 
	                vty_out(vty,"%-12llu %-12llu %-15u %-15u %-12u\n\n",head->rx_sum_bytes,head->tx_sum_bytes,head->rx_pkt_control,head->tx_pkt_control,head->rx_errors_frames);
					vty_out(vty,"==============================================================================\n");
					head = head->next;
					//CW_FREE_OBJECT(statics->ap_statics_list->ap_statics_ele);
				}
			}
			dcli_ac_free_fun_three(WID_DBUS_CONF_METHOD_AP_SHOW_STATISTICS_LIST,statics);
			statics = NULL;
		}
		else if(ret == WID_AP_NO_STATICS)
		{
			vty_out(vty,"ap have not statistics information\n");
		}
		else if(ret == WTP_ID_NOT_EXIST)
		{
			vty_out(vty,"wtp id no exist\n");
		}
		else if(ret == WTP_ID_LARGE_THAN_MAX)
		{
			vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
		}
			
	//	dbus_message_unref(reply);	
	//	CW_FREE_OBJECT(statics->ap_statics_list);
	//	CW_FREE_OBJECT(statics);
	}


	if(vty->node == VIEW_NODE){
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			//for remote hansi info
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}
	
		 hansi_parameter:
				statics = dcli_ac_show_api_group_three(
					profile,
					SIXTH,/*"show ap statistics list bywtp ID"*/
					wtpid,
					0,
					0,
					&ret,
					0,
					&localid,
					dcli_dbus_connection,
					WID_DBUS_CONF_METHOD_AP_SHOW_STATISTICS_LIST
					);
			    vty_out(vty,"==============================================================================\n");
			    vty_out(vty,"hansi %d-%d\n",slot_id,profile);
			    vty_out(vty,"-----------------------------------------------------------------------------\n");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0){	
					if((statics)&&(statics->ap_statics_list)&&(statics->ap_statics_list->ap_statics_ele)){
						wlan_stats_info *head = NULL;
						head = statics->ap_statics_list->ap_statics_ele;
						vty_out(vty,"show ap statistics list \n");
						vty_out(vty,"-----------------------------------------------------------------------\n");
						while(head){	
							vty_out(vty,"%-6s %-17s %-7s %-7s %-6s %-6s %-7s %-7s %-6s \n","type","mac","rx_pkt","tx_pkt","rx_err","tx_err","rx_bts","tx_bts","rx_rt");
							vty_out(vty,"%-6s %02X:%02X:%02X:%02X:%02X:%02X %-7d %-7d %-6d %-6d %-7llu %-7llu %-6d \n\n",\
								head->ifname,head->mac[0],head->mac[1],head->mac[2],\
								head->mac[3],head->mac[4],head->mac[5],head->rx_packets,head->tx_packets,\
								head->rx_errors,head->tx_errors,head->rx_bytes,head->tx_bytes,head->rx_rate);
			
							vty_out(vty,"%-6s %-8s %-8s %-8s %-8s %-7s %-8s %-8s %-8s\n","tx_rt","CRC_err","key_err","MIC_err","phy_err","WlanID","rx_drop","tx_drop","rx_band");
							vty_out(vty,"%-6d %-8d %-8d %-8d %-8d %-7d %-8d %-8d %-8d \n\n",head->tx_rate,head->ast_rx_crcerr,head->ast_rx_badcrypt,head->ast_rx_badmic,head->ast_rx_phyerr,\
								head->wlanId,head->rx_drop,head->tx_drop,head->rx_band);
			
							/*zhaoruijia,添加ap显示信息将wlan_stats_info_profile结构体中数据显示完全，start*/
			
							vty_out(vty,"%-8s %-9s %-9s %-15s %-15s %-14s \n","tx_band","rx_frame","tx_frame","rx_error_frame","tx_error_frame","rx_drop_frame"); 
							vty_out(vty,"%-8u %-9u %-9u %-15u %-15u %-14u \n\n",head->tx_band,head->rx_frame,head->tx_frame,head->rx_error_frame,head->tx_error_frame,\
																			head->rx_drop_frame);
			
			
							vty_out(vty,"%-14s %-11s %-11s %-13s %-13s %-13s  \n","tx_drop_frame","rx_unicast","tx_unicast","rx_multicast","tx_multicast","rx_broadcast"); 
							vty_out(vty,"%-14u %-11llu %-11llu %-13u %-13u %-13u \n\n",head->tx_drop_frame,head->rx_unicast,head->tx_unicast,head->rx_multicast,\
																			   head->tx_multicast,head->rx_broadcast);
			
							vty_out(vty,"%-13s %-15s %-15s %-17s %-17s	\n","tx_broadcast","rx_pkt_unicast","tx_pkt_unicast","rx_pkt_multicast", "tx_pkt_multicast");
							vty_out(vty,"%-13u %-15u %-15u %-17u %-17u \n\n",head->tx_broadcast,head->rx_pkt_unicast,head->tx_pkt_unicast,head->rx_pkt_multicast,head->tx_pkt_multicast);
																	   
			
							vty_out(vty,"%-17s %-17s %-13s %-13s %-12s	 \n","rx_pkt_broadcast","tx_pkt_broadcast","rx_pkt_retry","tx_pkt_retry","rx_pkt_data"); 
							vty_out(vty,"%-17u %-17u %-13u %-13u %-12u \n\n",head->rx_pkt_broadcast,head->tx_pkt_broadcast,head->rx_pkt_retry,head->tx_pkt_retry,head->rx_pkt_data);
			
			
							vty_out(vty,"%-12s %-9s %-9s %-8s %-8s %-12s %-12s\n","tx_pkt_data","rx_retry","tx_retry","rx_mgmt","tx_mgmt","rx_pkt_mgmt","tx_pkt_mgmt");
							vty_out(vty,"%-12u	%-9u %-9u %-8llu %-8llu %-12u %-12u\n",head->tx_pkt_data,head->rx_retry,head->tx_retry,head->rx_mgmt,head->tx_mgmt,head->rx_pkt_mgmt,head->tx_pkt_mgmt);
							/*zhaoruijia,添加ap显示信息将wlan_stats_info_profile结构体中数据显示完全，end*/
			
							/* zhangshu append to show "rx_sum_bytes" and "tx_sum_bytes"  */
							/*zhaoruijia,20100907,添加rx_pkt_control，tx_pkt_control */
							/* zhangshu append for showing rx_errors_frames , 2010-09-26 */
							vty_out(vty,"%-12s %-12s %-15s %-15s %-12s\n %","rx_sum_bytes","tx_sum_bytes","rx_pkt_control","tx_pkt_control","rx_errors_frames"); 
							vty_out(vty,"%-12llu %-12llu %-15u %-15u %-12u\n\n",head->rx_sum_bytes,head->tx_sum_bytes,head->rx_pkt_control,head->tx_pkt_control,head->rx_errors_frames);
							vty_out(vty,"-----------------------------------------------------------------------\n");
							head = head->next;
						}
					}
					dcli_ac_free_fun_three(WID_DBUS_CONF_METHOD_AP_SHOW_STATISTICS_LIST,statics);
					statics = NULL;
				}
				else if(ret == WID_AP_NO_STATICS)
				{
					vty_out(vty,"ap have not statistics information\n");
				}
				else if(ret == WTP_ID_NOT_EXIST)
				{
					vty_out(vty,"wtp id no exist\n");
				}
				else if(ret == WTP_ID_LARGE_THAN_MAX)
				{
					vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
				}
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}

		//for local hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 1;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}
		
			local_hansi_parameter:
				statics = dcli_ac_show_api_group_three(
					profile,
					SIXTH,/*"show ap statistics list bywtp ID"*/
					wtpid,
					0,
					0,
					&ret,
					0,
					&localid,
					dcli_dbus_connection,
					WID_DBUS_CONF_METHOD_AP_SHOW_STATISTICS_LIST
					);
			    vty_out(vty,"==============================================================================\n");
			    vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
			    vty_out(vty,"-----------------------------------------------------------------------------\n");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0){	
					if((statics)&&(statics->ap_statics_list)&&(statics->ap_statics_list->ap_statics_ele)){
						wlan_stats_info *head = NULL;
						head = statics->ap_statics_list->ap_statics_ele;
						vty_out(vty,"show ap statistics list \n");
						vty_out(vty,"-----------------------------------------------------------------------\n");
						while(head){	
							vty_out(vty,"%-6s %-17s %-7s %-7s %-6s %-6s %-7s %-7s %-6s \n","type","mac","rx_pkt","tx_pkt","rx_err","tx_err","rx_bts","tx_bts","rx_rt");
							vty_out(vty,"%-6s %02X:%02X:%02X:%02X:%02X:%02X %-7d %-7d %-6d %-6d %-7llu %-7llu %-6d \n\n",\
								head->ifname,head->mac[0],head->mac[1],head->mac[2],\
								head->mac[3],head->mac[4],head->mac[5],head->rx_packets,head->tx_packets,\
								head->rx_errors,head->tx_errors,head->rx_bytes,head->tx_bytes,head->rx_rate);
			
							vty_out(vty,"%-6s %-8s %-8s %-8s %-8s %-7s %-8s %-8s %-8s\n","tx_rt","CRC_err","key_err","MIC_err","phy_err","WlanID","rx_drop","tx_drop","rx_band");
							vty_out(vty,"%-6d %-8d %-8d %-8d %-8d %-7d %-8d %-8d %-8d \n\n",head->tx_rate,head->ast_rx_crcerr,head->ast_rx_badcrypt,head->ast_rx_badmic,head->ast_rx_phyerr,\
								head->wlanId,head->rx_drop,head->tx_drop,head->rx_band);
			
							/*zhaoruijia,添加ap显示信息将wlan_stats_info_profile结构体中数据显示完全，start*/
			
							vty_out(vty,"%-8s %-9s %-9s %-15s %-15s %-14s \n","tx_band","rx_frame","tx_frame","rx_error_frame","tx_error_frame","rx_drop_frame"); 
							vty_out(vty,"%-8u %-9u %-9u %-15u %-15u %-14u \n\n",head->tx_band,head->rx_frame,head->tx_frame,head->rx_error_frame,head->tx_error_frame,\
																			head->rx_drop_frame);
			
			
							vty_out(vty,"%-14s %-11s %-11s %-13s %-13s %-13s  \n","tx_drop_frame","rx_unicast","tx_unicast","rx_multicast","tx_multicast","rx_broadcast"); 
							vty_out(vty,"%-14u %-11llu %-11llu %-13u %-13u %-13u \n\n",head->tx_drop_frame,head->rx_unicast,head->tx_unicast,head->rx_multicast,\
																			   head->tx_multicast,head->rx_broadcast);
			
							vty_out(vty,"%-13s %-15s %-15s %-17s %-17s	\n","tx_broadcast","rx_pkt_unicast","tx_pkt_unicast","rx_pkt_multicast", "tx_pkt_multicast");
							vty_out(vty,"%-13u %-15u %-15u %-17u %-17u \n\n",head->tx_broadcast,head->rx_pkt_unicast,head->tx_pkt_unicast,head->rx_pkt_multicast,head->tx_pkt_multicast);
																	   
			
							vty_out(vty,"%-17s %-17s %-13s %-13s %-12s	 \n","rx_pkt_broadcast","tx_pkt_broadcast","rx_pkt_retry","tx_pkt_retry","rx_pkt_data"); 
							vty_out(vty,"%-17u %-17u %-13u %-13u %-12u \n\n",head->rx_pkt_broadcast,head->tx_pkt_broadcast,head->rx_pkt_retry,head->tx_pkt_retry,head->rx_pkt_data);
			
			
							vty_out(vty,"%-12s %-9s %-9s %-8s %-8s %-12s %-12s\n","tx_pkt_data","rx_retry","tx_retry","rx_mgmt","tx_mgmt","rx_pkt_mgmt","tx_pkt_mgmt");
							vty_out(vty,"%-12u	%-9u %-9u %-8llu %-8llu %-12u %-12u\n",head->tx_pkt_data,head->rx_retry,head->tx_retry,head->rx_mgmt,head->tx_mgmt,head->rx_pkt_mgmt,head->tx_pkt_mgmt);
							/*zhaoruijia,添加ap显示信息将wlan_stats_info_profile结构体中数据显示完全，end*/
			
							/* zhangshu append to show "rx_sum_bytes" and "tx_sum_bytes"  */
							/*zhaoruijia,20100907,添加rx_pkt_control，tx_pkt_control */
							/* zhangshu append for showing rx_errors_frames , 2010-09-26 */
							vty_out(vty,"%-12s %-12s %-15s %-15s %-12s\n %","rx_sum_bytes","tx_sum_bytes","rx_pkt_control","tx_pkt_control","rx_errors_frames"); 
							vty_out(vty,"%-12llu %-12llu %-15u %-15u %-12u\n\n",head->rx_sum_bytes,head->tx_sum_bytes,head->rx_pkt_control,head->tx_pkt_control,head->rx_errors_frames);
							vty_out(vty,"-----------------------------------------------------------------------\n");
							head = head->next;
						}
					}
					dcli_ac_free_fun_three(WID_DBUS_CONF_METHOD_AP_SHOW_STATISTICS_LIST,statics);
					statics = NULL;
				}
				else if(ret == WID_AP_NO_STATICS)
				{
					vty_out(vty,"ap have not statistics information\n");
				}
				else if(ret == WTP_ID_NOT_EXIST)
				{
					vty_out(vty,"wtp id no exist\n");
				}
				else if(ret == WTP_ID_LARGE_THAN_MAX)
				{
					vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
				}
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}
	}

	return CMD_SUCCESS;			
}

DEFUN(show_ap_ip_func,
		  show_ap_ip_bywtpid_cmd,
		  "show ap ip bywtp ID",
		  "show ap ip \n"
		  "show ap ip \n"
		  "show ap ip \n"
	 )
{
	int ret = -1;
	int retval = 0;
//	DBusMessage *query, *reply;	
//	DBusMessageIter	 iter;
//	DBusError err;

	DCLI_AC_API_GROUP_THREE *network = NULL;
	//CW_CREATE_OBJECT_ERR(network, DCLI_AC_API_GROUP_THREE, return NULL;);	
	//network->WTPIP = NULL;

	int wtpid = 0;
	
	unsigned char myipBuf[DCLIAC_BUF_LEN] = {0};	
	unsigned char *myipPtr = myipBuf;
	
	unsigned char mygatewayBuf[DCLIAC_BUF_LEN] = {0};	
	unsigned char *mygatewayPtr = mygatewayBuf;
	unsigned char mymaskBuf[DCLIAC_BUF_LEN] = {0};	

	wtpid = atoi(argv[0]);
	if ((wtpid <= 0) || (wtpid >= WTP_NUM)) {
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_FAILURE;
		}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#if 0
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_SHOW_IPADDR);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AP_SHOW_IPADDR);*/
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret == 0 )
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&ipaddr);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&mask);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&gatewayaddr);


		retval = ip_long2str(ipaddr,&myipPtr);
		retval = ip_long2str(gatewayaddr,&mygatewayPtr);

		if(mask == 8)
		{
			strcpy(mymaskBuf, "255.0.0.0");
		}
		else if(mask == 16)
		{
			strcpy(mymaskBuf, "255.255.0.0");
		}
		else
		{
			strcpy(mymaskBuf, "255.255.255.0");
		}
		
		vty_out(vty,"show ap ip list \n");
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"%-17s %-17s %-17s \n","ipaddr","netmask","gateway");


		vty_out(vty,"%-17s %-17s %-17s\n",\
			    myipBuf,mymaskBuf,mygatewayBuf);

		vty_out(vty,"==============================================================================\n");
	}
#endif
	network = dcli_ac_show_api_group_three(
		index,
		SECOND,/*"show ap ip bywtp ID"*/
		wtpid,
		0,
		0,
		&ret,
		0,
		&localid,
		//network,
		dcli_dbus_connection,
		WID_DBUS_CONF_METHOD_AP_SHOW_IPADDR
		);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
		retval = ip_long2str(network->wtpip,&myipPtr);
		retval = ip_long2str(network->ap_gateway,&mygatewayPtr);

		if(network->ap_mask_new == 8)
		{
			strcpy(mymaskBuf, "255.0.0.0");
		}
		else if(network->ap_mask_new == 16)
		{
			strcpy(mymaskBuf, "255.255.0.0");
		}
		else
		{
			strcpy(mymaskBuf, "255.255.255.0");
		}
		
		vty_out(vty,"show ap ip list \n");
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"%-17s %-17s %-17s \n","ipaddr","netmask","gateway");


		vty_out(vty,"%-17s %-17s %-17s\n",\
			    myipBuf,mymaskBuf,mygatewayBuf);

		vty_out(vty,"==============================================================================\n");
		dcli_ac_free_fun_three(WID_DBUS_CONF_METHOD_AP_SHOW_IPADDR,network);
		network = NULL;
	}
	else if(ret == WID_AP_NO_STATICS)
	{
		vty_out(vty,"ap have not ip information\n");
	}
	else if(ret == WTP_ID_NOT_EXIST)
	{
		vty_out(vty,"wtp id no exist\n");
	}
	else if(ret == WTP_ID_LARGE_THAN_MAX)
	{
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
	}
		
//	dbus_message_unref(reply);	
//	CW_FREE_OBJECT(network);
	return CMD_SUCCESS;			
}

DEFUN(set_ap_timestamp_func,
		  set_ap_timestamp_cmd,
		  "set ap timestamp ",
		  "ap timestamp config\n"
		  "ap timestamp synchronization with ac\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    int timestamp = 0;
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_TIMESTAMP);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AP_TIMESTAMP);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&timestamp,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," set ap timestamp successfully\n");
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}
DEFUN(show_ap_model_infomation_func,
	  show_ap_model_infomation_cmd,
	  "show ap MODEL infomation",
	  "show ap MODEL infomation\n"
	  "ap infomation\n"
	  "ap model 1110T 2010 2110\n"
	  "ap infomation\n"
	 )
{
	int ret = -1;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	int i = 0;
	unsigned int model = 0 ;
	unsigned int mtu = 0 ;
	unsigned int rate = 0 ;
	unsigned short ap_11a_antenna_gain = 0 ;
	unsigned short ap_11bg_antenna_gain = 0 ;
	char* ap_model;
	char* hw_version;
	char* sw_name;
	char* sw_version;
	char* sw_supplier;
	char* supplier;
	unsigned short eth_num = 0;
	unsigned short wifi_num = 0;
	char iftype[] = "Ethernet";
	char iftype2[] = "Wifi";
	char eth[2][5] = {"eth0","eth1"};
	char wifi[2][6] = {"wifi0","wifi1"};
	char description[2][32] = {"Main ethernet interface","Secondary ethernet interface"};
	char description2[2][32] = {"Main wireless interface","Secondary wireless interface"};
	if (!strcmp(argv[0],"1110T"))
	{
		model = 1;	
	}
	else if (!strcmp(argv[0],"2010"))
	{
		model = 2;	
	}
	else if (!strcmp(argv[0],"2110"))
	{
		model = 3;	
	}
	else if (!strcmp(argv[0],"AQ1000"))
	{
		model = 4;	
	}
	else if (!strcmp(argv[0],"AQ1000-H"))
	{
		model = 5;	
	}
	else if (!strcmp(argv[0],"AQ3110-H"))
	{
		model = 6;	
	}
	else if (!strcmp(argv[0],"AQ3120-H"))
	{
		model = 7;	
	}
	else
	{
		vty_out(vty,"<error> input model is wrong\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	DCLI_AC_API_GROUP_FOUR *modelinfo = NULL;
//	CW_CREATE_OBJECT_ERR(modelinfo, DCLI_AC_API_GROUP_FOUR, return NULL;);	
//	modelinfo->model_info = NULL;
#if 0	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_SHOW_MODEL_INFOMATION);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AP_SHOW_MODEL_INFOMATION);*/
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&model,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret == 0 )
	{
	

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&ap_model);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&ap_11a_antenna_gain);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&ap_11bg_antenna_gain);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&eth_num);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&wifi_num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&hw_version);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sw_name);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sw_version);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&sw_supplier);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&supplier);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&mtu);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&rate);
		
		vty_out(vty,"show ap model infomation \n");
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"Model:	%s\n",ap_model);
		vty_out(vty,"HW version:	%s\n",hw_version);
		vty_out(vty,"HW supplier:	%s\n",supplier);
		vty_out(vty,"SW version:	%s\n",sw_version);
		vty_out(vty,"SW name:	%s\n",sw_name);
		vty_out(vty,"SW supplier:	%s\n",sw_supplier);
		vty_out(vty,"AP IF MTU:	%d\n",mtu);
		vty_out(vty,"AP IF rate:	%d M\n",rate);
		vty_out(vty,"802.11a antenna gain:	%d dbi\n",ap_11a_antenna_gain);
		vty_out(vty,"802.11bg antenna gain:	%d dbi\n",ap_11bg_antenna_gain);
		vty_out(vty,"Interface type:	%s	%s\n",iftype,iftype2);
		vty_out(vty,"Ethernet interface num:	%d\n",eth_num);
		for(i=0;i<eth_num;i++)
		{
			vty_out(vty,"Ethernet interface description:	%s	%s\n",eth[i],description[i]);
		}
		vty_out(vty,"Wireless interface num:	%d\n",wifi_num);
		for(i=0;i<wifi_num;i++)
		{
			vty_out(vty,"Wireless interface description:	%s	%s\n",wifi[i],description2[i]);
		}
		vty_out(vty,"==============================================================================\n");
	}
#endif
	modelinfo = dcli_ac_show_api_group_four(
		index,
		FIRST,/*"show ap MODEL infomation"*/
		0,
		model,
		&ret,
		0,
		0,
		localid,
	//	modelinfo,
		dcli_dbus_connection,
		WID_DBUS_CONF_METHOD_AP_SHOW_MODEL_INFOMATION
		);
	//printf("#######ret is %d \n",ret);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
		model_infomation *head = NULL;
		if(modelinfo != NULL)
			head = modelinfo->model_info;
		if(head != NULL){

				vty_out(vty,"show ap model infomation \n");
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"Model: %s\n",head->model);
				vty_out(vty,"HW version:	%s\n",head->hw_version);
				vty_out(vty,"HW supplier:	%s\n",head->supplier);
				vty_out(vty,"SW version:	%s\n",head->sw_version);
				vty_out(vty,"SW name:	%s\n",head->sw_name);
				vty_out(vty,"SW supplier:	%s\n",head->sw_supplier);
				vty_out(vty,"AP IF MTU: %d\n",head->ap_if_mtu);
				vty_out(vty,"AP IF rate:	%d M\n",head->ap_if_rate);
				vty_out(vty,"802.11a antenna gain:	%d dbi\n",head->ap_11a_antenna_gain);
				vty_out(vty,"802.11bg antenna gain: %d dbi\n",head->ap_11bg_antenna_gain);
				vty_out(vty,"Interface type:	%s	%s\n",iftype,iftype2);
				vty_out(vty,"Ethernet interface num:	%d\n",head->ap_eth_num);
				for(i=0;i<head->ap_eth_num;i++)
				{
					vty_out(vty,"Ethernet interface description:	%s	%s\n",eth[i],description[i]);
				}
				vty_out(vty,"Wireless interface num:	%d\n",head->ap_wifi_num);
				for(i=0;i<head->ap_wifi_num;i++)
				{
					vty_out(vty,"Wireless interface description:	%s	%s\n",wifi[i],description2[i]);
				}
				vty_out(vty,"==============================================================================\n");

		}
		dcli_ac_free_fun_four(WID_DBUS_CONF_METHOD_AP_SHOW_MODEL_INFOMATION,modelinfo);
		modelinfo = NULL;
	}
	else if(ret == SUPPORT_VERSION_NOT_EXIST)
	{
		vty_out(vty,"<error> ac support model does not set\n");
	}
	else if(ret == VERSION_NOT_SUPPORTED)
	{
		vty_out(vty,"<error> this model does not supportted, set it first\n");
	}
//	CW_FREE_OBJECT(modelinfo->model_info);		
//	CW_FREE_OBJECT(modelinfo);		
//	dbus_message_unref(reply);	
	
	return CMD_SUCCESS;			
}
DEFUN(show_ap_txpower_control_func,
	  show_ap_txpower_control_cmd,
	  "show ap txpower control [remote] [local] [PARAMETER]",
	  "show infomation\n"
	  "ap infomation\n"
	  "txpower control\n"
	  "txpower control\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{
	int ret = -1;
/*	unsigned char scope = 0;
	unsigned char th1 = 0;
	unsigned char th2 = 0;
	unsigned char constant = 0;
	unsigned char max = 0;*/
	char enable[] = "enable";
	char disable[] = "disable";
	char own[] = "own";
	char all[] = "all";
	
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	DCLI_AC_API_GROUP_FIVE *tx_control = NULL;

	if((argc == 1)||(argc == 3)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 2){
		if (!strcmp(argv[0],"remote")){
			localid = 0;
		}else if(!strcmp(argv[0],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[0],"remote"))&&(!strcmp(argv[1],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[1],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[1], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}
#if 0	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_SHOW_WTP_TXPOWER_CONTROL);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AP_SHOW_WTP_TXPOWER_CONTROL);*/
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&state);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&scope);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&th1);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&th2);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&constant);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&max);
	
		

		
	vty_out(vty,"show ap txpower control infomation \n");
	vty_out(vty,"==============================================================================\n");
	vty_out(vty,"Txpower control switch:	%s\n",(state==1)?enable:disable);
	vty_out(vty,"Txpower control scope:	%s\n",(scope==0)?own:all);
	vty_out(vty,"Coverage threshold:	%d\n",th1);
	vty_out(vty,"Txpower threshold:	%d\n",th2);
	vty_out(vty,"Txpower constant:	%d\n",constant);
	vty_out(vty,"Max Txpower:	%d\n",max);
	vty_out(vty,"==============================================================================\n");
#endif	
	if(vty->node != VIEW_NODE){
		tx_control = dcli_ac_show_api_group_five(
			index,
			FOURTH,/*"show ap txpower control"*/
			0,
			0,
			0,
			&ret,
			0,
			0,
			localid,
		//	tx_control,
			dcli_dbus_connection,
			WID_DBUS_CONF_METHOD_AP_SHOW_WTP_TXPOWER_CONTROL
			);
		//printf("#######ret is %d \n",ret);
		if(ret == -1){
			cli_syslog_info("<error> failed get reply.\n");
		}
		else if(ret == 0){	
			vty_out(vty,"show ap txpower control infomation \n");
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"Txpower control switch:	%s\n",(tx_control->tx_control->state==1)?enable:disable);
			vty_out(vty,"Txpower control scope: %s\n",(tx_control->tx_control->scope==0)?own:all);
			vty_out(vty,"Coverage threshold:	%d\n",tx_control->tx_control->th1);
			vty_out(vty,"Txpower threshold: %d\n",tx_control->tx_control->th2);
			vty_out(vty,"Txpower constant:	%d\n",tx_control->tx_control->constant);
			vty_out(vty,"Max Txpower:	%d\n",tx_control->tx_control->max);
			vty_out(vty,"==============================================================================\n");
			dcli_ac_free_fun_five(WID_DBUS_CONF_METHOD_AP_SHOW_WTP_TXPOWER_CONTROL,tx_control);
			tx_control = NULL;
		}else 
			vty_out(vty,"ret is %d \n",ret);
	//	CW_FREE_OBJECT(tx_control->tx_control);	
	//	CW_FREE_OBJECT(tx_control);			
	//	dbus_message_unref(reply);	
	}

	if(vty->node == VIEW_NODE){
		//for remote hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}
	
		hansi_parameter:
				tx_control = dcli_ac_show_api_group_five(
					profile,
					FOURTH,/*"show ap txpower control"*/
					0,
					0,
					0,
					&ret,
					0,
					0,
					localid,
					dcli_dbus_connection,
					WID_DBUS_CONF_METHOD_AP_SHOW_WTP_TXPOWER_CONTROL
					);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0){	
					vty_out(vty,"show ap txpower control infomation \n");
					vty_out(vty,"-----------------------------------------------------------------------------\n");
					vty_out(vty,"Txpower control switch:	%s\n",(tx_control->tx_control->state==1)?enable:disable);
					vty_out(vty,"Txpower control scope: %s\n",(tx_control->tx_control->scope==0)?own:all);
					vty_out(vty,"Coverage threshold:	%d\n",tx_control->tx_control->th1);
					vty_out(vty,"Txpower threshold: %d\n",tx_control->tx_control->th2);
					vty_out(vty,"Txpower constant:	%d\n",tx_control->tx_control->constant);
					vty_out(vty,"Max Txpower:	%d\n",tx_control->tx_control->max);
					dcli_ac_free_fun_five(WID_DBUS_CONF_METHOD_AP_SHOW_WTP_TXPOWER_CONTROL,tx_control);
					tx_control = NULL;
				}else 
					vty_out(vty,"ret is %d \n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 2){
					return CMD_SUCCESS;
				}
			}
		}

		//for local hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 1;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}
		
			local_hansi_parameter:
				tx_control = dcli_ac_show_api_group_five(
					profile,
					FOURTH,/*"show ap txpower control"*/
					0,
					0,
					0,
					&ret,
					0,
					0,
					localid,
					dcli_dbus_connection,
					WID_DBUS_CONF_METHOD_AP_SHOW_WTP_TXPOWER_CONTROL
					);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0){	
					vty_out(vty,"show ap txpower control infomation \n");
					vty_out(vty,"-----------------------------------------------------------------------------\n");
					vty_out(vty,"Txpower control switch:	%s\n",(tx_control->tx_control->state==1)?enable:disable);
					vty_out(vty,"Txpower control scope: %s\n",(tx_control->tx_control->scope==0)?own:all);
					vty_out(vty,"Coverage threshold:	%d\n",tx_control->tx_control->th1);
					vty_out(vty,"Txpower threshold: %d\n",tx_control->tx_control->th2);
					vty_out(vty,"Txpower constant:	%d\n",tx_control->tx_control->constant);
					vty_out(vty,"Max Txpower:	%d\n",tx_control->tx_control->max);
					dcli_ac_free_fun_five(WID_DBUS_CONF_METHOD_AP_SHOW_WTP_TXPOWER_CONTROL,tx_control);
					tx_control = NULL;
				}else 
					vty_out(vty,"ret is %d \n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 2){
					return CMD_SUCCESS;
				}
			}
		}
	}

	return CMD_SUCCESS;			
}
DEFUN(set_receiver_signal_level_func,
	  set_receiver_signal_level_cmd,
	  "set receiver signal level LEVEL",
	  CONFIG_STR
	  "wireless-control configuration\n"
	  "wireless-control configuration\n"
	  "wireless-control configuration\n"
	 )
{	
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	unsigned int level = 0;
	int ret = WID_DBUS_SUCCESS;
	
	level = atoi(argv[0]);
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_RECEIVER_SIGNAL_LEVEL);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_RECEIVER_SIGNAL_LEVEL);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&level,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
		
	dbus_message_unref(reply);
	if(ret == 0)
	{		
		vty_out(vty,"set receiver signal level %s successfully\n",argv[0]);
	}
	return CMD_SUCCESS;
}

DEFUN(show_receiver_signal_level_func,
	  show_receiver_signal_level_cmd,
	  "show receiver signal level",
	  SHOW_STR
	  "wireless-control information\n"
	  "wireless-control information\n"
	  "wireless-control information\n"
	 )
{

	DCLI_AC_API_GROUP_FIVE *receiver_sig_lev = NULL;
	CW_CREATE_OBJECT_ERR(receiver_sig_lev, DCLI_AC_API_GROUP_FIVE, return -1;); 
	receiver_sig_lev->num = 0;
	int ret = 0;

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#if 0	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	unsigned int level = 0;
	int ret = WID_DBUS_SUCCESS;	
	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_RECEIVER_SIGNAL_LEVEL);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SHOW_RECEIVER_SIGNAL_LEVEL);*/
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&level);

	dbus_message_unref(reply);
			
	vty_out(vty,"==============================================================================\n");
	vty_out(vty,"Receiver signal level:	%0.1f V\n",level/10.0);
	vty_out(vty,"==============================================================================\n");
	
#endif	
	receiver_sig_lev = dcli_ac_show_api_group_five(
		index,
		FIFTH,/*"show receiver signal level"*/
		0,
		0,
		0,
		&ret,
		0,
		0,
		localid,
		//receiver_sig_lev,
		dcli_dbus_connection,
		WID_DBUS_CONF_METHOD_SHOW_RECEIVER_SIGNAL_LEVEL
		);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"Receiver signal level: %0.1f V\n",receiver_sig_lev->num/10.0);
		vty_out(vty,"==============================================================================\n");
		dcli_ac_free_fun_five(WID_DBUS_CONF_METHOD_SHOW_RECEIVER_SIGNAL_LEVEL,receiver_sig_lev);
		receiver_sig_lev = NULL;
	}
	else 
		vty_out(vty,"ret is %d.\n",ret);
//	CW_FREE_OBJECT(receiver_sig_lev);		
	return CMD_SUCCESS;
}
DEFUN(set_monitor_time_func,
	  set_monitor_time_cmd,
	  "set monitor time TIME",
	  CONFIG_STR
	  "information\n"
	  "monitor time\n"
	  "monitor time\n"
	 )
{	
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	int ret = WID_DBUS_SUCCESS;
	unsigned int time;
	int len = 0;

	ret = parse_int_ID((char*)argv[0], &time);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_MONITOR_TIME);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_MONITOR_TIME);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&time,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
		
	dbus_message_unref(reply);
	if(ret == 0)
	{		
		vty_out(vty,"set monitor time %s successfully\n",argv[0]);
	}
	else 
	vty_out(vty,"<error> %d\n",ret);
	
	return CMD_SUCCESS;
}

DEFUN(set_sample_time_func,
	  set_sample_time_cmd,
	  "set sample time TIME",
	  CONFIG_STR
	  "information\n"
	  "sample time\n"
	  "sample time\n"
	 )
{	
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	int ret = WID_DBUS_SUCCESS;
	unsigned int time;

	ret = parse_int_ID((char*)argv[0], &time);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_SAMPLE_TIME);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_SAMPLE_TIME);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&time,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
		
	dbus_message_unref(reply);
	if(ret == 0)
	{		
		vty_out(vty,"set sample time %s successfully\n",argv[0]);
	}
	else 
	vty_out(vty,"<error> %d\n",ret);
	
	return CMD_SUCCESS;
}
DEFUN(show_sample_info_func,
	  show_sample_info_cmd,
	  "show sample infomation",
	  SHOW_STR
	  "information\n"
	  "sample time\n"
	  "sample time\n"
	 )
{	
	
/*	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	wid_sample_info sampleinfo;
	sampleinfo.monitor_switch = 0;
	sampleinfo.monitor_time = 0;
	sampleinfo.sample_switch = 0;
	sampleinfo.sample_time = 0;*/
	char en[] = "enable";
	char dis[] = "disable";
	int ret = WID_DBUS_SUCCESS;
	
	DCLI_AC_API_GROUP_FIVE *sample_info = NULL;

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#if 0	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WID_SAMPLE_INFO);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WID_SAMPLE_INFO);*/
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&sampleinfo.monitor_switch);
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&sampleinfo.monitor_time);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&sampleinfo.sample_switch);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&sampleinfo.sample_time);
	dbus_message_unref(reply);
	
	vty_out(vty,"==============================================================================\n");
	vty_out(vty,"Wireless sample infomation\n");
	vty_out(vty,"monitor_switch:	%s\n",(sampleinfo.monitor_switch == 0)?dis:en);
	vty_out(vty,"sample_switch:	%s\n",(sampleinfo.sample_switch == 0)?dis:en);
	vty_out(vty,"monitor_time:	%d s\n",sampleinfo.monitor_time);
	vty_out(vty,"sample_time:	%d s\n",sampleinfo.sample_time);
	vty_out(vty,"==============================================================================\n");
#endif	
	sample_info = dcli_ac_show_api_group_five(
		index,
		SIXTH,/*"show sample infomation"*/
		0,
		0,
		0,
		&ret,
		0,
		0,
		localid,
	//	sample_info,
		dcli_dbus_connection,
		WID_DBUS_CONF_METHOD_SHOW_WID_SAMPLE_INFO
		);
	//printf("#######ret is %d \n",ret);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"Wireless sample infomation\n");
		vty_out(vty,"monitor_switch:	%s\n",(sample_info->sample_info->monitor_switch == 0)?dis:en);
		vty_out(vty,"sample_switch: %s\n",(sample_info->sample_info->sample_switch == 0)?dis:en);
		vty_out(vty,"monitor_time:	%d s\n",sample_info->sample_info->monitor_time);
		vty_out(vty,"sample_time:	%d s\n",sample_info->sample_info->sample_time);
		vty_out(vty,"==============================================================================\n");
		dcli_ac_free_fun_five(WID_DBUS_CONF_METHOD_SHOW_WID_SAMPLE_INFO,sample_info);
		sample_info = NULL;
	}
	else 
		vty_out(vty,"ret:	%d\n",ret);
//	CW_FREE_OBJECT(sample_info->sample_info);		
//	CW_FREE_OBJECT(sample_info);		
	return CMD_SUCCESS;
}
DEFUN(set_monitor_enable_func,
	  set_monitor_enable_cmd,
	  "set wireless-control monitor (enable|disable)",
	  CONFIG_STR
	  "wireless-control config\n"
	  "monitor information\n"
	  "enable|disable\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    int policy = 0;

	
	if (!strcmp(argv[0],"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[0],"disable"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_MONITOR_ENABLE);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_MONITOR_ENABLE);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," set wireless control monitor %s successfully\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}
DEFUN(set_sample_enable_func,
	  set_sample_enable_cmd,
	  "set wireless-control sample (enable|disable)",
	  CONFIG_STR
	  "wireless-control config\n"
	  "sample information\n"
	  "enable|disable\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    int policy = 0;

	
	if (!strcmp(argv[0],"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[0],"disable"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_SAMPLE_ENABLE);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_SAMPLE_ENABLE);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," set wireless control sample %s successfully\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}

#if 0
 // Moved to dcli_wsm.c 
DEFUN(set_ipfwd_func,
	  set_ipfwd_cmd,
	  "set flow-based-forwarding (enable|disable)",
	  CONFIG_STR
	  "wireless-control config\n"
	  "ipfwd config\n"
	  "enable|disable\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    int policy = 0;

	
	if (!strcmp(argv[0],"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[0],"disable"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_IPFWD);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_IPFWD);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," set ipfwd %s successfully\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}

/*luoxun add*/
DEFUN(show_ipfwd_func,
	  show_ipfwd_cmd,
	  "show flow-based-forwarding",
	  SHOW_STR
	  "flow-based-forwarding information\n"
	  "flow-based-forwarding information\n"
	  "flow-based-forwarding information\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int state = 0;
	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WID_IPFWD);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SHOW_WID_IPFWD);*/
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection, query, -1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&state);
		
	dbus_message_unref(reply);

	vty_out(vty,"==============================================\n");
	if(0 == state)
		vty_out(vty,"flow-based-forwarding state:	disable\n");
	else if(1 == state)
		vty_out(vty,"flow-based-forwarding state:	enable\n");
	else
		vty_out(vty,"flow-based-forwarding state:	unexpected return state=%d\n", state);
	vty_out(vty,"==============================================\n");
	
	return CMD_SUCCESS;
}

#endif


DEFUN(show_model_list_cmd_func,
	  show_model_list_cmd,
	  "show model (list|all) [remote] [local] [PARAMETER]",
	  SHOW_STR
	  "Display model all information\n"
	  "List model summary\n"
	  "List model summary\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{	
 	DCLI_AC_API_GROUP_FOUR *conf_info = NULL;
//	CW_CREATE_OBJECT_ERR(conf_info, DCLI_AC_API_GROUP_FOUR, return NULL;);	
//	conf_info->config_ver_info = NULL;

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;		

	int i=0,ret=0,num=0;
	int j=0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	if((argc == 2)||(argc == 4)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 3){
		if (!strcmp(argv[1],"remote")){
			localid = 0;
		}else if(!strcmp(argv[1],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[1],"remote"))&&(!strcmp(argv[2],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[2],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[2], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}
#if 0	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_MODEL_LIST);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SHOW_MODEL_LIST);*/
	
	dbus_error_init(&err);	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	char *apmodel = NULL;
	char *versionname = NULL;
	char *versionpath = NULL;
	char *apcode = NULL;
	unsigned char radionum;
	unsigned char bssnum;

	vty_out(vty,"AC support version list summary\n");

    vty_out(vty,"==============================================================================\n");
	vty_out(vty,"%-25s %-12s %-9s %-19s %-5s %-5s\n","Model","Apcode","CodeTag","Ver_path","R_Num","B_Num");


	if(ret == 0 ){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for (i = 0; i < num; i++) {
			DBusMessageIter iter_struct;
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(apmodel));	
			
			dbus_message_iter_next(&iter_struct);
			

			dbus_message_iter_get_basic(&iter_struct,&(apcode));	
			
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(versionname));
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(versionpath));
			
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(radionum));	

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(bssnum));	
			
			dbus_message_iter_next(&iter_array);
			
			vty_out(vty,"%-25s %-12s %-9s %-19s %-5d %-5d\n",apmodel,apcode,versionname,versionpath,radionum,bssnum);

			
		}
	}
#endif
	if(vty->node != VIEW_NODE){
		conf_info = dcli_ac_show_api_group_four(
			index,
			SECOND,/*"show model (list|all)"*/
			0,
			0,
			&ret,
			0,
			0,
			localid,
		//	conf_info,
			dcli_dbus_connection,
			WID_DBUS_CONF_METHOD_SHOW_MODEL_LIST
			);
		//printf("#######ret is %d \n",ret);

		vty_out(vty,"AC support version list summary\n");
	    vty_out(vty,"==============================================================================\n");
		vty_out(vty,"%-24s %-9s %-16s %-5s %-5s\n","Model","CodeTag","Ver_path","R_Num","B_Num");
		if(ret == -1){
			cli_syslog_info("<error> failed get reply.\n");
		}
		else if(ret == 0){	
			int len = 0;
			CWConfigVersionInfo_dcli *head = NULL;		
			if(conf_info->config_ver_info != NULL){
				len = conf_info->config_ver_info->list_len;
				head = conf_info->config_ver_info->config_ver_node;
				for(i=0;i<len;i++){
					if(head == NULL)
						break;
					vty_out(vty,"%-24s %-9s %-16s %-5d %-5d\n",head->str_ap_model,head->str_ap_version_name,head->str_ap_version_path,head->radio_num,head->bss_num);
					head = head->next;	
				}
			}
			//CW_FREE_OBJECT(conf_info->config_ver_info->config_ver_node);	
			dcli_ac_free_fun_four(WID_DBUS_CONF_METHOD_SHOW_MODEL_LIST,conf_info);
			conf_info = NULL;
		}else{
			vty_out(vty,"ret is %d.\n",ret);
	 	}
		vty_out(vty,"==============================================================================\n");
	//	dbus_message_unref(reply);	
	//	CW_FREE_OBJECT(conf_info->config_ver_info);	
	//	CW_FREE_OBJECT(conf_info);	
	}

	if(vty->node == VIEW_NODE){
		//for remote hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

		hansi_parameter:
				conf_info = dcli_ac_show_api_group_four(
					profile,
					SECOND,/*"show model (list|all)"*/
					0,
					0,
					&ret,
					0,
					0,
					localid,
					dcli_dbus_connection,
					WID_DBUS_CONF_METHOD_SHOW_MODEL_LIST
					);
			
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d     AC support version list summary\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				vty_out(vty,"%-24s %-9s %-16s %-5s %-5s\n","Model","CodeTag","Ver_path","R_Num","B_Num");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0){	
					int len = 0;
					CWConfigVersionInfo_dcli *head = NULL;		
					if(conf_info->config_ver_info != NULL){
						len = conf_info->config_ver_info->list_len;
						head = conf_info->config_ver_info->config_ver_node;
						for(i=0;i<len;i++){
							if(head == NULL)
								break;
							vty_out(vty,"%-24s %-9s %-16s %-5d %-5d\n",head->str_ap_model,head->str_ap_version_name,head->str_ap_version_path,head->radio_num,head->bss_num);
							head = head->next;	
						}
					}
					dcli_ac_free_fun_four(WID_DBUS_CONF_METHOD_SHOW_MODEL_LIST,conf_info);
					conf_info = NULL;
				}else{
					vty_out(vty,"ret is %d.\n",ret);
				}
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}

		//for local hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 1;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

		local_hansi_parameter:
				conf_info = dcli_ac_show_api_group_four(
					profile,
					SECOND,/*"show model (list|all)"*/
					0,
					0,
					&ret,
					0,
					0,
					localid,
					dcli_dbus_connection,
					WID_DBUS_CONF_METHOD_SHOW_MODEL_LIST
					);
			
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"local hansi %d-%d     AC support version list summary\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				vty_out(vty,"%-24s %-9s %-16s %-5s %-5s\n","Model","CodeTag","Ver_path","R_Num","B_Num");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0){	
					int len = 0;
					CWConfigVersionInfo_dcli *head = NULL;		
					if(conf_info->config_ver_info != NULL){
						len = conf_info->config_ver_info->list_len;
						head = conf_info->config_ver_info->config_ver_node;
						for(i=0;i<len;i++){
							if(head == NULL)
								break;
							vty_out(vty,"%-24s %-9s %-16s %-5d %-5d\n",head->str_ap_model,head->str_ap_version_name,head->str_ap_version_path,head->radio_num,head->bss_num);
							head = head->next;	
						}
					}
					dcli_ac_free_fun_four(WID_DBUS_CONF_METHOD_SHOW_MODEL_LIST,conf_info);
					conf_info = NULL;
				}else{
					vty_out(vty,"ret is %d.\n",ret);
				}
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}
	}

	return CMD_SUCCESS;
}

DEFUN(show_model_cmd_func,
	  show_model_cmd,
	  "show model .MODEL",
	  SHOW_STR
	  "Display model information\n"
	  "model detail info\n"

	 )
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	char *model = NULL;
	
//	struct model_detail_info modelinfo;
	
	int ret,i;

	model = WID_parse_ap_extension_command(argv,argc);	
	if(model == NULL){
		vty_out(vty,"UNKNOWN COMMAND\n");
		return CMD_SUCCESS;
	}
	str2higher(&model);
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	DCLI_AC_API_GROUP_FOUR *modelinfo = NULL;
	//CW_CREATE_OBJECT_ERR(modelinfo, DCLI_AC_API_GROUP_FOUR, return NULL;);	
	//modelinfo->config_ver_info = NULL;
#if 0	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_MODEL);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SHOW_MODEL);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,		
						DBUS_TYPE_STRING,&model,
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if(model)
	{	
		free(model);
		model = NULL;
	}
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);


	vty_out(vty,"Model Information summary\n");

    vty_out(vty,"==============================================================================\n");

	if(ret == 0 )
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&modelinfo.str_ap_model);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(modelinfo.str_ap_version_name));	
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(modelinfo.str_ap_version_path));
	
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(modelinfo.str_ap_code));
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(modelinfo.radio_num));	

		for(i=0;i<modelinfo.radio_num;i++)
		{
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(modelinfo.radio_info[i].radio_type));

			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(modelinfo.radio_info[i].radio_id));	
			
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(modelinfo.radio_info[i].reserved));	

			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(modelinfo.radio_info[i].bss_count));	
		}	
	
		vty_out(vty,"Model: %s\n",modelinfo.str_ap_model);
		vty_out(vty,"CodeTag: %s\n",modelinfo.str_ap_version_name);
		vty_out(vty,"Path: %s\n",modelinfo.str_ap_version_path);
		vty_out(vty,"Develop Code: %s\n",modelinfo.str_ap_code);
		vty_out(vty,"Radio count: %d\n",modelinfo.radio_num);
		for(i=0;i<modelinfo.radio_num;i++)
		{
			vty_out(vty,"Radio id: %d\n",modelinfo.radio_info[i].radio_id);
			vty_out(vty,"Radio type: %d\n",modelinfo.radio_info[i].radio_type);
			vty_out(vty,"Radio power: %s\n",(modelinfo.radio_info[i].reserved == 1 ? "High":"Normal"));
			vty_out(vty,"BSS count: %d\n",modelinfo.radio_info[i].bss_count);
		}
		
	}
#endif
	modelinfo = dcli_ac_show_api_group_four(
		index,
		THIRD,/*"show model .MODEL"*/
		0,
		0,
		&ret,
		model,/* char *char1 */
		0,
		localid,
	//	modelinfo,
		dcli_dbus_connection,
		WID_DBUS_CONF_METHOD_SHOW_MODEL
		);
	////printf("#######ret is %d \n",ret);
	vty_out(vty,"Model Information summary\n");

    vty_out(vty,"==============================================================================\n");
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
		CWConfigVersionInfo_dcli *head;
			if((modelinfo != NULL)&&(modelinfo->config_ver_info != NULL)){
				head = modelinfo->config_ver_info->config_ver_node;
				if(head != NULL){
					vty_out(vty,"Model: %s\n",head->str_ap_model);
					vty_out(vty,"CodeTag: %s\n",head->str_ap_version_name);
					vty_out(vty,"Path: %s\n",head->str_ap_version_path);
					//vty_out(vty,"Develop Code: %s\n",head->str_ap_code);
					vty_out(vty,"Radio count: %d\n",head->radio_num);
					for(i=0;i<head->radio_num;i++)
					{
						vty_out(vty,"Radio id: %d\n",head->radio_info[i].radio_id);
						vty_out(vty,"Radio type: %d\n",head->radio_info[i].radio_type);
						vty_out(vty,"Radio power: %s\n",(head->radio_info[i].reserved1 == 1 ? "High":"Normal"));
						vty_out(vty,"BSS count: %d\n",head->radio_info[i].bss_count);
					}
				}
			}
		//	CW_FREE_OBJECT(modelinfo->config_ver_info->config_ver_node); 	
			dcli_ac_free_fun_four(WID_DBUS_CONF_METHOD_SHOW_MODEL,modelinfo);
			modelinfo = NULL;
	}
	else
	{
		vty_out(vty,"error this model doesn't exist\n");
	}
	
	vty_out(vty,"==============================================================================\n");	

//	dbus_message_unref(reply);	
//	CW_FREE_OBJECT(modelinfo->config_ver_info);		
//	CW_FREE_OBJECT(modelinfo);		
	return CMD_SUCCESS;


}

DEFUN(set_model_cmd_func,
	  set_model_cmd,
	  "set model to new .MODELTONEW",
	  SHOW_STR
	  "Chage old model for new model\n"
	  "old model to new one\n"
	  "eg set model to new AQ 2010 to AW 3110"

	 )
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	char *oldmodel = NULL;
	char *newmodel = NULL;
	int ret;

	oldmodel = WID_parse_CMD_str(argv,argc,"to",1);	
	if(oldmodel == NULL){
		vty_out(vty,"UNKNOWN COMMAND\n");
		vty_out(vty,"COMMAND should be :set model to new AQ 2010 to AW 3110\n");
		return CMD_SUCCESS;
	}
	newmodel = WID_parse_CMD_str(argv,argc,"to",0);	
	if(newmodel == NULL){
		free(oldmodel);
		vty_out(vty,"UNKNOWN COMMAND\n");		
		vty_out(vty,"COMMAND should be :set model to new AQ 2010 to AW 3110\n");
		return CMD_SUCCESS;
	}
	str2higher(&oldmodel);
	str2higher(&newmodel);
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_MODEL);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_MODEL);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,		
						DBUS_TYPE_STRING,&oldmodel,
						DBUS_TYPE_STRING,&newmodel,
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		if(oldmodel)
		{	
			free(oldmodel);
			oldmodel = NULL;
		}
		if(newmodel)
		{	
			free(newmodel);
			newmodel = NULL;
		}
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0 )
	{
		vty_out(vty,"model:%s change to model:%s success\n",oldmodel,newmodel);
	}
	else if(ret == MODEL_NO_CONFIG)
	{
		vty_out(vty,"error new model:%s is not configuration please change other name\n",newmodel);
	}
	else
	{
		vty_out(vty,"error this model doesn't exist\n");
	}

	if(oldmodel)
	{	
		free(oldmodel);
		oldmodel = NULL;
	}
	if(newmodel)
	{	
		free(newmodel);
		newmodel = NULL;
	}
	dbus_message_unref(reply);	

	return CMD_SUCCESS;

}
DEFUN(show_ap_model_code_func,
	  show_ap_model_code_cmd,
	  "show ap model-code-information .MODEL",
	  SHOW_STR
	  "ap information\n"
	  "ap model code information\n"
	  "ap model like AQ2010\n"
	  "model detail information\n"

	 )
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;

	char *model = NULL;
	
	//wid_code_infomation modelinfo;
	char cpu_type[2][11] = {"soc","octeon5010"};
	char mem_type[2][6] = {"flash","card"};
	char iftype[] = "Ethernet";
	char iftype2[] = "Wifi";
	char eth[4][5] = {"eth0","eth1","eth2","eth3"};
	char wifi[4][6] = {"wifi0","wifi1","wifi2","wifi3"};
	char description[4][32] = {"Main ethernet interface","Secondary ethernet interface","none","none"};
	char description2[4][32] = {"Main wireless interface","Secondary wireless interface","none","none"};
	char radio_mode[16][5] = {"none","b","g","bg","n","bn","gn","bgn","a","ab","ag","abg","an","abn","agn","abgn"};
	int ret=0,i=0;

	model = WID_parse_ap_extension_command(argv,argc);
	if(model == NULL){
		vty_out(vty,"UNKNOWN COMMAND\n");
		return CMD_SUCCESS;
	}
	str2higher(&model);
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	DCLI_AC_API_GROUP_FOUR *codeinfo = NULL;
	//CW_CREATE_OBJECT_ERR(codeinfo, DCLI_AC_API_GROUP_FOUR, return NULL;);	
	//codeinfo->code_info = NULL;
#if 0
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_SHOW_MODEL_CODE_INFOMATION);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AP_SHOW_MODEL_CODE_INFOMATION);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,		
						DBUS_TYPE_STRING,&model,
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) 
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		if(model)
		{	
			free(model);
			model = NULL;
		}
		return CMD_SUCCESS;
	}

	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);


	vty_out(vty,"Model Information summary\n");

    vty_out(vty,"==============================================================================\n");

	if(ret == 0 )
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&modelinfo.code);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(modelinfo.hw_version));	
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(modelinfo.supplier));
	
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(modelinfo.sw_name));
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(modelinfo.sw_version));	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(modelinfo.sw_supplier));	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(modelinfo.ap_if_mtu));	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(modelinfo.ap_if_rate));	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(modelinfo.card_capacity));

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(modelinfo.flash_capacity));	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(modelinfo.cpu_type));	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(modelinfo.mem_type));	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(modelinfo.ap_eth_num));	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(modelinfo.ap_wifi_num));

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(modelinfo.ap_antenna_gain));	

		for(i=0;i<modelinfo.ap_wifi_num;i++)
		{
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&(modelinfo.support_mode[i]));	
		}	
	
		vty_out(vty,"Model:	%s\n",model);
		vty_out(vty,"Code:	%s\n",modelinfo.code);
		vty_out(vty,"HW version:	%s\n",modelinfo.hw_version);
		vty_out(vty,"HW supplier:	%s\n",modelinfo.supplier);
		vty_out(vty,"SW version:	%s\n",modelinfo.sw_version);
		vty_out(vty,"SW name:	%s\n",modelinfo.sw_name);
		vty_out(vty,"SW supplier:	%s\n",modelinfo.sw_supplier);
		vty_out(vty,"AP CPU type:	%s\n",(modelinfo.cpu_type == 1)?cpu_type[0]:cpu_type[1]);
		vty_out(vty,"AP memory type:	%s\n",(modelinfo.mem_type == 1)?mem_type[0]:mem_type[1]);
		vty_out(vty,"AP card capacity:	%d M\n",modelinfo.card_capacity);
		vty_out(vty,"AP flash capacity:	%d M\n",modelinfo.flash_capacity);
		vty_out(vty,"AP IF MTU:	%d\n",modelinfo.ap_if_mtu);
		vty_out(vty,"AP IF rate:	%d M\n",modelinfo.ap_if_rate);
		vty_out(vty,"AP antenna gain:	%d dbi\n",modelinfo.ap_antenna_gain);
		vty_out(vty,"Interface type:	%s	%s\n",iftype,iftype2);
		vty_out(vty,"Ethernet interface num:	%d\n",modelinfo.ap_eth_num);
		for(i=0;i<modelinfo.ap_eth_num;i++)
		{
			vty_out(vty,"Ethernet interface description:	%s	%s\n",eth[i],description[i]);
		}
		vty_out(vty,"Wireless interface num:	%d\n",modelinfo.ap_wifi_num);
		for(i=0;i<modelinfo.ap_wifi_num;i++)
		{
			vty_out(vty,"Wireless interface description:	%s	%s\n",wifi[i],description2[i]);
			vty_out(vty,"%s support radio mode:	%s\n",wifi[i],radio_mode[(modelinfo.support_mode[i])]);
		}
	}
#endif	
	codeinfo = dcli_ac_show_api_group_four(
		index,
		FOURTH,/*"show ap model-code-information .MODEL"*/
		0,
		0,
		&ret,
		model,/*char1*/
		0,
		localid,
	//	codeinfo,
		dcli_dbus_connection,
		WID_DBUS_CONF_METHOD_AP_SHOW_MODEL_CODE_INFOMATION
		);
	//printf("#######ret is %d \n",ret);
	vty_out(vty,"Model Information summary\n");

    vty_out(vty,"==============================================================================\n");
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
		wid_code_infomation *head = NULL;
		if(codeinfo != NULL)
			head = codeinfo->code_info;
		if(head != NULL){
			vty_out(vty,"Model: %s\n",model);
			//vty_out(vty,"Code:	%s\n",head->code);
			vty_out(vty,"HW version:	%s\n",head->hw_version);
			vty_out(vty,"HW supplier:	%s\n",head->supplier);
			vty_out(vty,"SW version:	%s\n",head->sw_version);
			vty_out(vty,"SW name:	%s\n",head->sw_name);
			vty_out(vty,"SW supplier:	%s\n",head->sw_supplier);
			vty_out(vty,"AP CPU type:	%s\n",(head->cpu_type == 1)?cpu_type[0]:cpu_type[1]);
			vty_out(vty,"AP memory type:	%s\n",(head->mem_type == 1)?mem_type[0]:mem_type[1]);
			vty_out(vty,"AP card capacity:	%d M\n",head->card_capacity);
			vty_out(vty,"AP flash capacity: %d M\n",head->flash_capacity);
			vty_out(vty,"AP IF MTU: %d\n",head->ap_if_mtu);
			vty_out(vty,"AP IF rate:	%d M\n",head->ap_if_rate);
			vty_out(vty,"AP antenna gain:	%d dbi\n",head->ap_antenna_gain);
			vty_out(vty,"Interface type:	%s	%s\n",iftype,iftype2);
			vty_out(vty,"Ethernet interface num:	%d\n",head->ap_eth_num);
			for(i=0;i<head->ap_eth_num;i++)
			{
				vty_out(vty,"Ethernet interface description:	%s	%s\n",eth[i],description[i]);
			}
			vty_out(vty,"Wireless interface num:	%d\n",head->ap_wifi_num);
			for(i=0;i<head->ap_wifi_num;i++)
			{
				vty_out(vty,"Wireless interface description:	%s	%s\n",wifi[i],description2[i]);
				vty_out(vty,"%s support radio mode: %s\n",wifi[i],radio_mode[(head->support_mode[i])]);
			}
		}
		dcli_ac_free_fun_four(WID_DBUS_CONF_METHOD_AP_SHOW_MODEL_CODE_INFOMATION,codeinfo);
		codeinfo = NULL;
	}
	else
	{
		vty_out(vty,"error this model doesn't exist\n");
	}
	
	vty_out(vty,"==============================================================================\n");	

	//CW_FREE_OBJECT(codeinfo->code_info);		
	//CW_FREE_OBJECT(codeinfo);		
	return CMD_SUCCESS;


}
DEFUN(set_ap_cm_threshold_func,
	  set_ap_cm_threshold_cmd,
	  "set ap (cpu|memoryuse|temperature) threshold PARAMETER",
	  CONFIG_STR
	  "ap config\n"
	  "ap cpu/memoryuse/temperature config\n"
	  "ap cpu/memoryuse/temperature threshold config\n"
	  "ap cpu/memoryuse/temperature threshold PARAMETER\n"
	 )
{
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int value;
	int ret = WID_DBUS_SUCCESS;
    int policy = 0;

	
	if (!strcmp(argv[0],"cpu"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[0],"memoryuse"))
	{
		policy = 2;	
	}
	else if (!strcmp(argv[0],"temperature"))
	{
		policy = 3;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'cpu','temperature'or 'memoryuse'\n");
		return CMD_SUCCESS;
	}

	ret = parse_int_ID((char*)argv[1], &value);
	if(ret != WID_DBUS_SUCCESS){
			vty_out(vty,"<error> unknown id format\n");
			return CMD_SUCCESS;
	}
	if(policy == 1)
	{
		if(value > 9999 || value == 0){
			vty_out(vty,"<error> ap cpu threshold parameters error\n");
			return CMD_SUCCESS;
		}
	}
	else if((policy == 2)||(policy == 3))
	{
		if(value > 99 || value == 0){
			vty_out(vty,"<error> ap memory use threshold parameters error\n");
			return CMD_SUCCESS;
		}
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_AP_CM_THRESHOLD);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_AP_CM_THRESHOLD);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
							 DBUS_TYPE_UINT32,&value,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," set ap %s threshold %s successfully\n",argv[0],argv[1]);
	}	
	else if (ret == WTP_NOT_IN_RUN_STATE)
	{
		vty_out(vty,"<error> wtp id does not run\n");
	}
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}
DEFUN(show_ap_threshold_func,
	  show_ap_threshold_cmd,
	  "show ap threshold",
	  SHOW_STR
	  "ap information\n"
	  "ap threshold information\n"
	 )
{	
	
/*	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;*/
	int ret = WID_DBUS_SUCCESS;
	int i = 0;
	unsigned int cpu = 0;
	unsigned int memoryuse = 0;
	unsigned int temperature = 0;

	DCLI_AC_API_GROUP_FIVE *ap_threshold = NULL;
	//CW_CREATE_OBJECT_ERR(ap_threshold, DCLI_AC_API_GROUP_FIVE, return NULL;); 

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#if 0	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_AP_CM_THRESHOLD);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SHOW_AP_CM_THRESHOLD);*/
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&cpu);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&memoryuse);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&temperature);


	dbus_message_unref(reply);
#endif
	ap_threshold = dcli_ac_show_api_group_five(
		index,
		SEVENTH,/*"show ap threshold"*/
		0,
		0,
		0,
		&ret,
		0,
		0,
		localid,
	//	ap_threshold,
		dcli_dbus_connection,
		WID_DBUS_CONF_METHOD_SHOW_AP_CM_THRESHOLD
		);
	//printf("#######ret is %d \n",ret);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"AP cpu threshold:	%d\n",ap_threshold->cpu);
		vty_out(vty,"AP memoryuse threshold: %d\n",ap_threshold->memoryuse);
		vty_out(vty,"AP temperature threshold: %d\n",ap_threshold->temperature);
		vty_out(vty,"==============================================================================\n");
		dcli_ac_free_fun_five(WID_DBUS_CONF_METHOD_SHOW_AP_CM_THRESHOLD,ap_threshold);
		ap_threshold = NULL;
	}
	else if(ret == WID_DBUS_ERROR) 
		vty_out(vty,"<error> other error\n");

//	CW_FREE_OBJECT(ap_threshold);		
	return CMD_SUCCESS;
}
DEFUN(show_ap_rrm_config_func,
	  show_ap_rrm_config_cmd,
	  "show radio resource management configuration [remote] [local] [PARAMETER]",
	  SHOW_STR
	  "radio resource management configuration\n"
	  "radio resource management configuration\n"
	  "radio resource management configuration\n"
	  "radio resource management configuration\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{
	int ret = -1;

/*	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char rrm_state = 0;
	unsigned int d_channel_state = 0;
	unsigned short report_interval = 0;
	unsigned char flag = 0;*/
	char enable[] = "enable";
	char disable[] = "disable";
	DCLI_AC_API_GROUP_FIVE *resource_mg = NULL;
//	CW_CREATE_OBJECT_ERR(resource_mg, DCLI_AC_API_GROUP_FIVE, return NULL;); 
	char apmode[] = "ap";
	char adhocmode[] = "adhoc";
	char allmode[] = "all";
	
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	if((argc == 1)||(argc == 3)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 2){
		if (!strcmp(argv[0],"remote")){
			localid = 0;
		}else if(!strcmp(argv[0],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[0],"remote"))&&(!strcmp(argv[1],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[1],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[1], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}
#if 0	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_RRM_CONFIG);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SHOW_RRM_CONFIG);*/
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&rrm_state);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&flag);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&report_interval);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&d_channel_state);
	
	/*print infomation	*/
	vty_out(vty,"show radio resource management configuration\n");
	vty_out(vty,"==============================================================================\n");
	vty_out(vty,"RRM switch:	%s\n",(rrm_state==1)?enable:disable);
	vty_out(vty,"report interval:	%d\n",report_interval);
	vty_out(vty,"dynamic channel selection switch:	%s\n",(d_channel_state==1)?enable:disable);
	vty_out(vty,"==============================================================================\n");
	
	
	dbus_message_unref(reply);	
#endif	
	if(vty->node != VIEW_NODE){
		resource_mg = dcli_ac_show_api_group_five(
			index,
			EIGHTH,/*"show radio resource management configuration"*/
			0,
			0,
			0,
			&ret,
			0,
			0,
			localid,
		//	resource_mg,
			dcli_dbus_connection,
			WID_DBUS_CONF_METHOD_SHOW_RRM_CONFIG
			);
		//printf("#######ret is %d \n",ret);
		if(ret == -1){
			cli_syslog_info("<error> failed get reply.\n");
		}
		else if(ret == 0){	
			vty_out(vty,"show radio resource management configuration\n");
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"RRM switch:	%s\n",(resource_mg->rrm_state==1)?enable:disable);
			vty_out(vty,"report interval:	%d\n",resource_mg->report_interval);
			vty_out(vty,"dynamic channel selection switch:	%s\n",(resource_mg->d_channel_state==1)?enable:disable);
		vty_out(vty,"countermeasures switch: %s\n",(resource_mg->countermeasures_switch==1)?enable:disable);
		vty_out(vty,"countermeasures mode: %s\n",(resource_mg->countermeasures_mode==0)?apmode:((resource_mg->countermeasures_mode==1)?adhocmode:allmode));

			vty_out(vty,"==============================================================================\n");
			dcli_ac_free_fun_five(WID_DBUS_CONF_METHOD_SHOW_RRM_CONFIG,resource_mg);
			resource_mg = NULL;
		}else 
			vty_out(vty,"error id :\n",ret);

	//	CW_FREE_OBJECT(resource_mg);
	}

	if(vty->node == VIEW_NODE){
		//for remote hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}
	
		hansi_parameter:
				resource_mg = dcli_ac_show_api_group_five(
					profile,
					EIGHTH,/*"show radio resource management configuration"*/
					0,
					0,
					0,
					&ret,
					0,
					0,
					localid,
					dcli_dbus_connection,
					WID_DBUS_CONF_METHOD_SHOW_RRM_CONFIG
					);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0){	
					vty_out(vty,"show radio resource management configuration\n");
					vty_out(vty,"-----------------------------------------------------------------------------\n");
					vty_out(vty,"RRM switch:	%s\n",(resource_mg->rrm_state==1)?enable:disable);
					vty_out(vty,"report interval:	%d\n",resource_mg->report_interval);
					vty_out(vty,"dynamic channel selection switch:	%s\n",(resource_mg->d_channel_state==1)?enable:disable);
					vty_out(vty,"countermeasures switch: %s\n",(resource_mg->countermeasures_switch==1)?enable:disable);
					vty_out(vty,"countermeasures mode: %s\n",(resource_mg->countermeasures_mode==0)?apmode:((resource_mg->countermeasures_mode==1)?adhocmode:allmode));
			
					dcli_ac_free_fun_five(WID_DBUS_CONF_METHOD_SHOW_RRM_CONFIG,resource_mg);
					resource_mg = NULL;
				}else 
					vty_out(vty,"error id :\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 2){
					return CMD_SUCCESS;
				}
			}
		}

		//for local hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 1;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}
		
			local_hansi_parameter:
				resource_mg = dcli_ac_show_api_group_five(
				profile,
				EIGHTH,/*"show radio resource management configuration"*/
				0,
				0,
				0,
				&ret,
				0,
				0,
				localid,
				dcli_dbus_connection,
				WID_DBUS_CONF_METHOD_SHOW_RRM_CONFIG
				);
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
			vty_out(vty,"-----------------------------------------------------------------------------\n");
			if(ret == -1){
				cli_syslog_info("<error> failed get reply.\n");
			}
			else if(ret == 0){	
				vty_out(vty,"show radio resource management configuration\n");
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				vty_out(vty,"RRM switch:	%s\n",(resource_mg->rrm_state==1)?enable:disable);
				vty_out(vty,"report interval:	%d\n",resource_mg->report_interval);
				vty_out(vty,"dynamic channel selection switch:	%s\n",(resource_mg->d_channel_state==1)?enable:disable);
			vty_out(vty,"countermeasures switch: %s\n",(resource_mg->countermeasures_switch==1)?enable:disable);
			vty_out(vty,"countermeasures mode: %s\n",(resource_mg->countermeasures_mode==0)?apmode:((resource_mg->countermeasures_mode==1)?adhocmode:allmode));

				dcli_ac_free_fun_five(WID_DBUS_CONF_METHOD_SHOW_RRM_CONFIG,resource_mg);
				resource_mg = NULL;
			}else 
				vty_out(vty,"error id :\n",ret);
			vty_out(vty,"==============================================================================\n");
			if(argc == 2){
				return CMD_SUCCESS;
			}
			}
		}
	}

	return CMD_SUCCESS;			
}


DEFUN(set_neighbordead_interval_cmd_func,
		  set_neighbordead_interval_cmd,
	 	  "set neighbordead interval VALUE",
		  "ac config\n"
		  "ac neighbordead interval config\n"
		  "ac neighbordead interval config value\n"
	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    int neighbordead_interval = 0;
	int ret;
	
	neighbordead_interval = atoi(argv[0]);
	
	if (neighbordead_interval < 1 || neighbordead_interval > 2000)
	{	
		vty_out(vty,"<error> input patameter should be 20 to 2000\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_NEIGHBORDEAD_INTERVAL);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_NEIGHBORDEAD_INTERVAL);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&neighbordead_interval,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"set neighbordead interval %s successfully\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;	
	
}

DEFUN(show_neighbordead_interval_cmd_func,
		  show_neighbordead_interval_cmd,
	 	  "show neighbordead interval",
		  "ac config\n"
		  "ac neighbordead interval config\n"
		  "ac neighbordead interval config\n"
	 )
{
/*	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;*/

	int ret = -1;
	DCLI_AC_API_GROUP_FIVE *interval = NULL;
//	CW_CREATE_OBJECT_ERR(interval, DCLI_AC_API_GROUP_FIVE, return NULL;); 

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#if 0	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_NEIGHBORDEAD_INTERVAL);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SHOW_NEIGHBORDEAD_INTERVAL);*/
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}		

		return CMD_SUCCESS;
	}
	
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&neighbordead_interval);
		

		
	vty_out(vty,"show neighbordead interval config\n");
	vty_out(vty,"==============================================================================\n");
	vty_out(vty,"neighbordead interval:%d s\n",neighbordead_interval);
	vty_out(vty,"==============================================================================\n");
		
	dbus_message_unref(reply);	
#endif
	interval = dcli_ac_show_api_group_five(
		index,
		NINTH,/*"show neighbordead interval"*/
		0,
		0,
		0,
		&ret,
		0,
		0,
		localid,
	//	interval,
		dcli_dbus_connection,
		WID_DBUS_CONF_METHOD_SHOW_NEIGHBORDEAD_INTERVAL
		);
	//printf("#######ret is %d \n",ret);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
		vty_out(vty,"show neighbordead interval config\n");
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"neighbordead interval:%d s\n",interval->neighbordead_interval);
		vty_out(vty,"==============================================================================\n");
		dcli_ac_free_fun_five(WID_DBUS_CONF_METHOD_SHOW_NEIGHBORDEAD_INTERVAL,interval);
		interval = NULL;
	}
	else 
		vty_out(vty,"error id :%d \n",ret);

//	CW_FREE_OBJECT(interval);		
	return CMD_SUCCESS;	

	
}

DEFUN(update_bak_ac_config_func,
	  update_bak_ac_config_cmd,
	  "update bak ac wireless info",
	  "update bak ac wireless info\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_BAK_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_BAK_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_AC_METHOD_UPDATE_BAK_INFO);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_BAK_OBJPATH,\
						WID_BAK_INTERFACE,WID_DBUS_AC_METHOD_UPDATE_BAK_INFO);*/
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);	
	dbus_message_unref(reply);	
	if(ret == 0){
		vty_out(vty,"update bak ac wireless info successfully\n");		
	}
	return CMD_SUCCESS;			
}

DEFUN(synchronize_wsm_table_func,
	  synchronize_wsm_table_cmd,
	  "synchronize wsm table info",
	  "synchronize wsm table info\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_BAK_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_BAK_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_AC_METHOD_SYNCHRONIZE_INFO);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_BAK_OBJPATH,\
						WID_BAK_INTERFACE,WID_DBUS_AC_METHOD_SYNCHRONIZE_INFO);*/
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);	
	dbus_message_unref(reply);	
	if(ret == 0){
		vty_out(vty,"update bak ac wireless info successfully\n");		
	}
	return CMD_SUCCESS;			
}

DEFUN(synchronize_asd_table_func,
	  synchronize_asd_table_cmd,
	  "synchronize asd table info",
	  "synchronize asd table info\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_BAK_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_BAK_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_AC_METHOD_ASD_SYNCHRONIZE_INFO);

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);	
	dbus_message_unref(reply);	
	if(ret == 0){
		vty_out(vty,"update wireless info to asd successfully\n");		
	}
	return CMD_SUCCESS;			
}

DEFUN(notice_vrrp_state_func,
	  notice_vrrp_state_cmd,
	  "notice vrrp state",
	  "notice vrrp state\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_BAK_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_BAK_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_AC_METHOD_NOTICE_INFO);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_BAK_OBJPATH,\
						WID_BAK_INTERFACE,WID_DBUS_AC_METHOD_NOTICE_INFO);*/
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);	
	dbus_message_unref(reply);	
	if(ret == 0){
		vty_out(vty,"notice vrrp successfully\n");		
	}
	return CMD_SUCCESS;			
}


DEFUN(show_vrrp_state_func,
	  show_vrrp_state_cmd,
	  "show wid vrrp state [remote] [local] [PARAMETER]",
	  "show wid vrrp state\n"
	  "show wid vrrp state\n"
	  "vrrp state\n"
	  "vrrp state\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{
	int ret;
/*	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;*/
	DCLI_AC_API_GROUP_FIVE *vrrp_state = NULL;
//	CW_CREATE_OBJECT_ERR(vrrp_state, DCLI_AC_API_GROUP_FIVE, return NULL;); 
//	vrrp_state->wid_vrrp = NULL;
/*	int flag,vrid,master_uplinkip,master_downlinkip,bak_uplinkip,bak_downlinkip,vir_uplinkip,vir_downlinkip,global_ht_ip,global_ht_opposite_ip;
	char *vir_uplinkname;
	char *vir_downlinkname;
	char *global_ht_ifname;*/
	char buf[10];
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	if((argc == 1)||(argc == 3)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 2){
		if (!strcmp(argv[0],"remote")){
			localid = 0;
		}else if(!strcmp(argv[0],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[0],"remote"))&&(!strcmp(argv[1],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[1],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[1], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}
#if 0	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_BAK_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_BAK_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_AC_METHOD_VRRP_INFO);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_BAK_OBJPATH,\
						WID_BAK_INTERFACE,WID_DBUS_AC_METHOD_VRRP_INFO);*/
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	if (!(dbus_message_get_args ( reply, &err,	
								DBUS_TYPE_UINT32,&vrid,
								DBUS_TYPE_UINT32,&flag,
								DBUS_TYPE_UINT32,&master_uplinkip,
								DBUS_TYPE_UINT32,&master_downlinkip,
								DBUS_TYPE_UINT32,&bak_uplinkip,
								DBUS_TYPE_UINT32,&bak_downlinkip,
								DBUS_TYPE_UINT32,&vir_uplinkip,
								DBUS_TYPE_UINT32,&vir_downlinkip,
								DBUS_TYPE_STRING,&vir_uplinkname,
								DBUS_TYPE_STRING,&vir_downlinkname,								
								DBUS_TYPE_STRING,&global_ht_ifname,
								DBUS_TYPE_UINT32,&global_ht_ip,
								DBUS_TYPE_UINT32,&global_ht_opposite_ip,
								DBUS_TYPE_INVALID))){

				
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}
#endif	
	if(vty->node != VIEW_NODE){
		vrrp_state = dcli_ac_show_api_group_five(
			index,
			TENTH,/*"show wid vrrp state"*/
			0,
			0,
			0,
			&ret,
			0,
			0,
			localid,
		//	vrrp_state,
			dcli_dbus_connection,
			WID_DBUS_AC_METHOD_VRRP_INFO
			);
		//printf("#######ret is %d \n",ret);
		if(ret == -1){
			cli_syslog_info("<error> failed get reply.\n");
		}
		else if(ret == 0){	

			if(vrrp_state->wid_vrrp->flag == 0){
				memcpy(buf,"master",7);
			}else if(vrrp_state->wid_vrrp->flag == 1){
				memcpy(buf,"bak",4);
			}else if(vrrp_state->wid_vrrp->flag == 2){
				memcpy(buf,"disable",8);
			}

			vty_out(vty,"wid vrrp info:\n");
			vty_out(vty,"wid vrrp state: %s\n",buf);
			vty_out(vty,"vir_downlinkname: %s\n",vrrp_state->wid_vrrp->vir_downlinkname);
			vty_out(vty,"vir_downlinkip: %d.%d.%d.%d\n",((unsigned char*)&vrrp_state->wid_vrrp->vir_downlinkip)[0],((unsigned char*)&vrrp_state->wid_vrrp->vir_downlinkip)[1],((unsigned char*)&vrrp_state->wid_vrrp->vir_downlinkip)[2],((unsigned char*)&vrrp_state->wid_vrrp->vir_downlinkip)[3]);
			vty_out(vty,"vir_uplinkname: %s\n",vrrp_state->wid_vrrp->vir_uplinkname);
			vty_out(vty,"vir_uplinkip: %d.%d.%d.%d\n",((unsigned char*)&vrrp_state->wid_vrrp->vir_uplinkip)[0],((unsigned char*)&vrrp_state->wid_vrrp->vir_uplinkip)[1],((unsigned char*)&vrrp_state->wid_vrrp->vir_uplinkip)[2],((unsigned char*)&vrrp_state->wid_vrrp->vir_uplinkip)[3]);
			vty_out(vty,"global_ht_ifname: %s\n",vrrp_state->wid_vrrp->global_ht_ifname);
			vty_out(vty,"heart beat: %d.%d.%d.%d\n",((unsigned char*)&vrrp_state->wid_vrrp->global_ht_ip)[0],((unsigned char*)&vrrp_state->wid_vrrp->global_ht_ip)[1],((unsigned char*)&vrrp_state->wid_vrrp->global_ht_ip)[2],((unsigned char*)&vrrp_state->wid_vrrp->global_ht_ip)[3]);
			vty_out(vty,"master heart beat ip : %d.%d.%d.%d\n",((unsigned char*)&vrrp_state->wid_vrrp->global_ht_opposite_ip)[0],((unsigned char*)&vrrp_state->wid_vrrp->global_ht_opposite_ip)[1],((unsigned char*)&vrrp_state->wid_vrrp->global_ht_opposite_ip)[2],((unsigned char*)&vrrp_state->wid_vrrp->global_ht_opposite_ip)[3]);
			dcli_ac_free_fun_five(WID_DBUS_AC_METHOD_VRRP_INFO,vrrp_state);
		}else 
			vty_out(vty,"error num:\n",ret);

	//	CW_FREE_OBJECT(vrrp_state->wid_vrrp);	
	//	CW_FREE_OBJECT(vrrp_state);	
	}

	if(vty->node == VIEW_NODE){
		//for remote hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}
	
		hansi_parameter:
				vrrp_state = dcli_ac_show_api_group_five(
					profile,
					TENTH,/*"show wid vrrp state"*/
					0,
					0,
					0,
					&ret,
					0,
					0,
					localid,
					dcli_dbus_connection,
					WID_DBUS_AC_METHOD_VRRP_INFO
					);
			    vty_out(vty,"==============================================================================\n");
			    vty_out(vty,"hansi %d-%d\n",slot_id,profile);
			    vty_out(vty,"-----------------------------------------------------------------------------\n");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0){	

					if(vrrp_state->wid_vrrp->flag == 0){
						memcpy(buf,"master",7);
					}else if(vrrp_state->wid_vrrp->flag == 1){
						memcpy(buf,"bak",4);
					}else if(vrrp_state->wid_vrrp->flag == 2){
						memcpy(buf,"disable",8);
					}

					vty_out(vty,"wid vrrp info:\n");
					vty_out(vty,"wid vrrp state: %s\n",buf);
					vty_out(vty,"vir_downlinkname: %s\n",vrrp_state->wid_vrrp->vir_downlinkname);
					vty_out(vty,"vir_downlinkip: %d.%d.%d.%d\n",((unsigned char*)&vrrp_state->wid_vrrp->vir_downlinkip)[0],((unsigned char*)&vrrp_state->wid_vrrp->vir_downlinkip)[1],((unsigned char*)&vrrp_state->wid_vrrp->vir_downlinkip)[2],((unsigned char*)&vrrp_state->wid_vrrp->vir_downlinkip)[3]);
					vty_out(vty,"vir_uplinkname: %s\n",vrrp_state->wid_vrrp->vir_uplinkname);
					vty_out(vty,"vir_uplinkip: %d.%d.%d.%d\n",((unsigned char*)&vrrp_state->wid_vrrp->vir_uplinkip)[0],((unsigned char*)&vrrp_state->wid_vrrp->vir_uplinkip)[1],((unsigned char*)&vrrp_state->wid_vrrp->vir_uplinkip)[2],((unsigned char*)&vrrp_state->wid_vrrp->vir_uplinkip)[3]);
					vty_out(vty,"global_ht_ifname: %s\n",vrrp_state->wid_vrrp->global_ht_ifname);
					vty_out(vty,"heart beat: %d.%d.%d.%d\n",((unsigned char*)&vrrp_state->wid_vrrp->global_ht_ip)[0],((unsigned char*)&vrrp_state->wid_vrrp->global_ht_ip)[1],((unsigned char*)&vrrp_state->wid_vrrp->global_ht_ip)[2],((unsigned char*)&vrrp_state->wid_vrrp->global_ht_ip)[3]);
					vty_out(vty,"master heart beat ip : %d.%d.%d.%d\n",((unsigned char*)&vrrp_state->wid_vrrp->global_ht_opposite_ip)[0],((unsigned char*)&vrrp_state->wid_vrrp->global_ht_opposite_ip)[1],((unsigned char*)&vrrp_state->wid_vrrp->global_ht_opposite_ip)[2],((unsigned char*)&vrrp_state->wid_vrrp->global_ht_opposite_ip)[3]);
					dcli_ac_free_fun_five(WID_DBUS_AC_METHOD_VRRP_INFO,vrrp_state);
				}else 
					vty_out(vty,"error num:\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 2){
					return CMD_SUCCESS;
				}
			}
		}

	//for local hansi info
	for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		localid = 1;
		
		for (profile = 1; profile < MAX_INSTANCE; profile++) 
		{
			instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
			if (INSTANCE_NO_CREATED == instRun) {
				continue;
			}

		local_hansi_parameter:
			vrrp_state = dcli_ac_show_api_group_five(
				profile,
				TENTH,/*"show wid vrrp state"*/
				0,
				0,
				0,
				&ret,
				0,
				0,
				localid,
				dcli_dbus_connection,
				WID_DBUS_AC_METHOD_VRRP_INFO
				);
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
			vty_out(vty,"-----------------------------------------------------------------------------\n");
			if(ret == -1){
				cli_syslog_info("<error> failed get reply.\n");
			}
			else if(ret == 0){	

				if(vrrp_state->wid_vrrp->flag == 0){
					memcpy(buf,"master",7);
				}else if(vrrp_state->wid_vrrp->flag == 1){
					memcpy(buf,"bak",4);
				}else if(vrrp_state->wid_vrrp->flag == 2){
					memcpy(buf,"disable",8);
				}

				vty_out(vty,"wid vrrp info:\n");
				vty_out(vty,"wid vrrp state: %s\n",buf);
				vty_out(vty,"vir_downlinkname: %s\n",vrrp_state->wid_vrrp->vir_downlinkname);
				vty_out(vty,"vir_downlinkip: %d.%d.%d.%d\n",((unsigned char*)&vrrp_state->wid_vrrp->vir_downlinkip)[0],((unsigned char*)&vrrp_state->wid_vrrp->vir_downlinkip)[1],((unsigned char*)&vrrp_state->wid_vrrp->vir_downlinkip)[2],((unsigned char*)&vrrp_state->wid_vrrp->vir_downlinkip)[3]);
				vty_out(vty,"vir_uplinkname: %s\n",vrrp_state->wid_vrrp->vir_uplinkname);
				vty_out(vty,"vir_uplinkip: %d.%d.%d.%d\n",((unsigned char*)&vrrp_state->wid_vrrp->vir_uplinkip)[0],((unsigned char*)&vrrp_state->wid_vrrp->vir_uplinkip)[1],((unsigned char*)&vrrp_state->wid_vrrp->vir_uplinkip)[2],((unsigned char*)&vrrp_state->wid_vrrp->vir_uplinkip)[3]);
				vty_out(vty,"global_ht_ifname: %s\n",vrrp_state->wid_vrrp->global_ht_ifname);
				vty_out(vty,"heart beat: %d.%d.%d.%d\n",((unsigned char*)&vrrp_state->wid_vrrp->global_ht_ip)[0],((unsigned char*)&vrrp_state->wid_vrrp->global_ht_ip)[1],((unsigned char*)&vrrp_state->wid_vrrp->global_ht_ip)[2],((unsigned char*)&vrrp_state->wid_vrrp->global_ht_ip)[3]);
				vty_out(vty,"master heart beat ip : %d.%d.%d.%d\n",((unsigned char*)&vrrp_state->wid_vrrp->global_ht_opposite_ip)[0],((unsigned char*)&vrrp_state->wid_vrrp->global_ht_opposite_ip)[1],((unsigned char*)&vrrp_state->wid_vrrp->global_ht_opposite_ip)[2],((unsigned char*)&vrrp_state->wid_vrrp->global_ht_opposite_ip)[3]);
				dcli_ac_free_fun_five(WID_DBUS_AC_METHOD_VRRP_INFO,vrrp_state);
				vrrp_state = NULL;
			}else 
				vty_out(vty,"error num:\n",ret);
			vty_out(vty,"==============================================================================\n");
			if(argc == 2){
				return CMD_SUCCESS;
			}
			}
		}
	}
	
	return CMD_SUCCESS; 			
}

DEFUN(show_vrrp_sock_cmd_func,
		  show_vrrp_sock_cmd,
		  "show wid vrrp sock list",
		  "show wid vrrp sock list\n"
	 )
{
	int ret;
	int num = 0;
	int i = 0;
	int length = 0;
	int sock;
	int ip;
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	DCLI_AC_API_GROUP_FOUR*baksock = NULL;
//	CW_CREATE_OBJECT_ERR(baksock, DCLI_AC_API_GROUP_FOUR, return NULL;);	
//	baksock->bak_sock = NULL;
#if 0
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter,iter_array;
	DBusError err;

    int state = 1;
	unsigned char mac[DCLIAC_MAC_LEN];	
	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_BAK_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_BAK_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_AC_METHOD_VRRP_SOCK_INFO);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_BAK_OBJPATH,\
						WID_BAK_INTERFACE,WID_DBUS_AC_METHOD_VRRP_SOCK_INFO);*/
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	vty_out(vty,"%-15s %-17s\n","connected sock","connected ip");
	if(ret == 0)
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for (i = 0; i < num; i++)
		{
			DBusMessageIter iter_struct;
			
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(sock));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(ip));
			dbus_message_iter_next(&iter_array);

			vty_out(vty,"%-15d %d.%d.%d.%d\n",sock,((unsigned char*)&ip)[0],((unsigned char*)&ip)[1],((unsigned char*)&ip)[2],((unsigned char*)&ip)[3]);

		}
	}
#endif	
	baksock = dcli_ac_show_api_group_four(
		index,
		FIFTH,/*"show wid vrrp sock list"*/
		0,
		0,
		&ret,
		0,
		0,
		localid,
		//baksock,
		dcli_dbus_connection,
		WID_DBUS_AC_METHOD_VRRP_SOCK_INFO
		);
	//printf("#######ret is %d \n",ret);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
		unsigned int len = 0;
		struct bak_sock *head;
		if(baksock->bak_sock != NULL){
			len = baksock->bak_sock->list_len;
			head = baksock->bak_sock->b_sock_node;
			int j;
			vty_out(vty,"%-15s %-17s\n","connected sock","connected ip");
			for(j=0;j<len;j++){
				vty_out(vty,"%-15d %d.%d.%d.%d\n",head->sock,((unsigned char*)&head->ip)[0],((unsigned char*)&head->ip)[1],((unsigned char*)&head->ip)[2],((unsigned char*)&head->ip)[3]);
				head = head->next;
			}
		}
	//	CW_FREE_OBJECT(baksock->bak_sock->b_sock_node);		
		dcli_ac_free_fun_four(WID_DBUS_AC_METHOD_VRRP_SOCK_INFO,baksock);
		baksock = NULL;
	}else {
		vty_out(vty,"ret is %d\n",ret);
	}


//	CW_FREE_OBJECT(baksock->bak_sock);

//	dbus_message_unref(reply);	
	
	return CMD_SUCCESS;				
}
DEFUN(set_wtp_wids_interval_cmd_func,
	  set_wtp_wids_interval_cmd,
	  "set wtp wids interval PARAMETER",
	  "wireless-control config\n"
	  "wtp infomation\n"
	  "wtp wids infomation\n"
	  "wtp wids interval\n"
	  "wtp wids interval s\n"
	 )
{
	int ret = 0;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    unsigned char interval = 0;
	
	ret = parse_char_ID((char*)argv[0], &interval);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
			vty_out(vty,"<error> unknown id format\n");
			}
			return CMD_SUCCESS;
	}	
	if(interval > 5 || interval == 0){
		vty_out(vty,"<error> wtp wids interval error,should be 1 to 5 second\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SET_WTP_WIDS_INTERVAL);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_WTP_METHOD_SET_WTP_WIDS_INTERVAL);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,	
							 DBUS_TYPE_BYTE,&interval,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"set wtp wids interval %s successfully\n",argv[0]);
	}
	else if (ret == SWITCH_IS_DISABLE)
		vty_out(vty,"<error> wids switch is enable\n");
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;			
}
DEFUN(set_wtp_wids_threshold_cmd_func,
	  set_wtp_wids_threshold_cmd,
	  "set wtp wids (probe|other) threshold PARAMETER",
	  "wireless-control config\n"
	  "wtp infomation\n"
	  "wtp wids infomation\n"
	  "wtp wids (probe|other) threshold\n"
	  "wtp wids threshold\n"
	  "wtp wids threshold\n"
	 )
{
	int ret = 0;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    unsigned char threshold = 0;
	unsigned char policy = 2;

	if (!strcmp(argv[0],"probe"))
	{
		policy = 0;	
	}		
	else if (!strcmp(argv[0],"other"))
	{
		policy = 1;
	}
	else
	{
		vty_out(vty,"<error> input patameter should only be 'probe' or 'other'\n");
		return CMD_SUCCESS;
	}
	
	ret = parse_char_ID((char*)argv[1], &threshold);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
			vty_out(vty,"<error> unknown id format\n");
			}
			return CMD_SUCCESS;
	}	
	if(threshold > 100 || threshold == 0){
		vty_out(vty,"<error> wtp wids threshold error,should be 1 to 100\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SET_WTP_WIDS_THRESHOLD);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_WTP_METHOD_SET_WTP_WIDS_THRESHOLD);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&policy,
							 DBUS_TYPE_BYTE,&threshold,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"set wtp wids %s threshold %s successfully\n",argv[0],argv[1]);
	}
	else if (ret == SWITCH_IS_DISABLE)
		vty_out(vty,"<error> wids switch is enable\n");
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}
DEFUN(set_wtp_wids_lasttime_cmd_func,
	  set_wtp_wids_lasttime_cmd,
	  "set wtp wids lasttime in black PARAMETER",
	  "wireless-control config\n"
	  "wtp infomation\n"
	  "wtp wids infomation\n"
	  "wtp wids lasttime in black\n"
	  "wtp wids lasttime in black\n"
	  "wtp wids lasttime in black\n"
	  "wtp wids lasttime in black\n"
	 )
{
	int ret = 0;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    unsigned int time = 0;

	ret = parse_int_ID((char*)argv[0], &time);
	if(ret != WID_DBUS_SUCCESS){
			vty_out(vty,"<error> unknown id format\n");
			return CMD_SUCCESS;
	}	
	if(time > 36000 || time == 0){
		vty_out(vty,"<error> wtp wids lasttime in black error,should be 1 to 36000\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SET_WTP_WIDS_LASTTIME_IN_BLACK);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_WTP_METHOD_SET_WTP_WIDS_LASTTIME_IN_BLACK);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&time,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"set wtp wids lasttime in black %s successfully\n",argv[0]);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}
/*
zhanglei add 
*/
DEFUN(batch_config_func,
	  batch_config_cmd,
	  "batch-config .COMMAND",
	  "batch-config\n"
	  "eg:batch-config <1-10> <1-10> <1-10> command interface radio*-*.*\n"
	 )
{
	int i = 0;
	int j = 0; 
	char * command = NULL;
	int a[5];
	int c = 0;
	int ret = 0;
	int len = 0;
	batch_arg arg[5];
	char cmd[512];
	char buf[512];
	char buf2[1024];
	int total = 1;	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	int flag = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
		flag = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
		flag = 1;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
		flag = 2;
	}
	command = WID_parse_CMD_str(argv,argc,"command",0);	
	if(command == NULL){
		vty_out(vty,"UNKNOWN COMMAND\n");
		vty_out(vty,"COMMAND should be :eg:batch-config <1-10> <1-10> <1-10> command interface radio*-*.*\n");
		return CMD_SUCCESS;
	}
	for(i=0;i<argc;i++){
		len = strlen(argv[i]);
		if(len == strlen(argv[i])&&(strcmp("command",argv[i])==0))
			break;
	}
	if(i == argc){
		free(command);
		command = NULL;
		vty_out(vty,"UNKNOWN COMMAND\n");
		vty_out(vty,"COMMAND should be :eg:batch-config <1-10> <1-10> <1-10> command interface radio*-*.*\n");
		return CMD_SUCCESS;		
	}
	if(i > 5){
		free(command);
		command = NULL;
		vty_out(vty,"only 5 parameter permit\n");
		return CMD_SUCCESS;				
	}
	memset(arg,0,5*sizeof(batch_arg));
	memset(cmd,0,512);
//	//printf("i: %d\n",i);
	for(j=0;j<i;j++){
		ret = Check_Batch_Command_Format((char *)argv[j],&(arg[j]));
		if(ret == WID_UNKNOWN_ID){
			free(command);
			vty_out(vty,"UNKNOWN COMMAND\n");
			vty_out(vty,"COMMAND should be :eg:batch-config <1-10> <1-10> <1-10> command interface radio*-*.*\n");
			return CMD_SUCCESS; 	
		}
		total = total*arg[j].num;
	}
	
	c = parse_str_by_character(command,cmd,'*');
//	//printf("cmd %s\n",cmd);
	if((c != i)){
		free(command);
		vty_out(vty,"number of parameters not match\n");
		vty_out(vty,"COMMAND should be :eg:batch-config <1-10> <1-10> <1-10> command interface radio*-*.*\n");
		return CMD_SUCCESS; 	
	}
	for(a[0]=arg[0].min;a[0]<arg[0].max+1;a[0]++)
		for(a[1]=arg[1].min;a[1]<arg[1].max+1;a[1]++)
			for(a[2]=arg[2].min;a[2]<arg[2].max+1;a[2]++)
				for(a[3]=arg[3].min;a[3]<arg[3].max+1;a[3]++)
					for(a[4]=arg[4].min;a[4]<arg[4].max+1;a[4]++)
					{
						memset(buf, 0, 512);						
						memset(buf2, 0, 1024);	
						sprintf(buf,cmd,a[0],a[1],a[2],a[3],a[4]);						
						////printf("buf %s\n",buf);
						if(flag == 0)
							sprintf(buf2,"/opt/bin/vtysh -c \"configure terminal\n %s\"\n",buf);
						else if(flag == 1)
							sprintf(buf2,"/opt/bin/vtysh -c \"configure terminal\n config hansi %d-%d\n %s\"\n",slot_id,index,buf);
						else if(flag == 2)
							sprintf(buf2,"/opt/bin/vtysh -c \"configure terminal\n config local-hansi %d-%d\n %s\"\n",slot_id,index,buf);
						////printf("buf2 %s\n",buf2);
						system(buf2);
					}
	free(command);
	return CMD_SUCCESS;
}
DEFUN(set_ap_update_img_timer_func,
	  set_ap_update_img_timer_cmd,
	  "set ap update img timer TIME",
	  CONFIG_STR
	  "ap update information\n"
	  "ap update information\n"
	  "ap update information\n"
	  "ap update timer\n"
	  "ap update timer <5-3600>\n"    //fengwenchao modify 20110427
	 )
{	
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	int ret = WID_DBUS_SUCCESS;
	unsigned int time = 0;

	ret = parse_int_ID((char*)argv[0], &time);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}
	/*fengwenchao add 20110427*/
	if((time < 5)||(time > 3600))
	{
		vty_out(vty,"<error> input time should be 5-3600\n");
		return CMD_SUCCESS;
	}
	/*fengwenchao add end*/
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = (int)vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_AP_UPDATE_TIMER);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_AP_UPDATE_TIMER);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&time,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
		
	dbus_message_unref(reply);
	if(ret == 0)
	{		
		vty_out(vty,"set sample time %s successfully\n",argv[0]);
	}
	else 
	vty_out(vty,"<error> %d\n",ret);
	
	return CMD_SUCCESS;
}
DEFUN(show_ap_update_img_timer_cmd_func,
	  show_ap_update_img_timer_cmd,
	  "show ap update img timer",
	  SHOW_STR
	  "ap update information\n"
	  "ap update information\n"
	  "ap update information\n"
	  "ap update timer\n"
	 )
{
/*	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;*/
	
	DCLI_AC_API_GROUP_FIVE *up_timer = NULL;
//	CW_CREATE_OBJECT_ERR(up_timer, DCLI_AC_API_GROUP_FIVE, return NULL;); 
	int ret = 0 ;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#if 0
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_AP_UPDATE_TIMER);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SHOW_AP_UPDATE_TIMER);*/
	
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&timer);
#endif
	up_timer = dcli_ac_show_api_group_five(
		index,
		ELEVENTH,/*"show ap update img timer"*/
		0,
		0,
		1,/*didn't transfer ret,so set it to 1 for function dcli_ac_show_api_group_five() */
		&ret,
		0,
		0,
		localid,
	//	up_timer,
		dcli_dbus_connection,
		WID_DBUS_CONF_METHOD_SHOW_AP_UPDATE_TIMER
		);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
		vty_out(vty,"######################################\n");
		vty_out(vty,"AP update img timer %d\n",up_timer->timer);
		vty_out(vty,"######################################\n");
		dcli_ac_free_fun_five(WID_DBUS_CONF_METHOD_SHOW_AP_UPDATE_TIMER,up_timer);
		up_timer = NULL;
	}
	else 
		vty_out(vty,"error num: %d\n",ret);
//	dbus_message_unref(reply);		
//	CW_FREE_OBJECT(up_timer);		
	return CMD_SUCCESS;
	
}

DEFUN(update_wtpcompatible_cmd_func,
	  update_wtpcompatible_cmd,
	  "update wtpcompatible",
	  "update wtpcompatible\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_UPDATE_WTPCOMPATIBLE);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_UPDATE_WTPCOMPATIBLE);*/
	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);	
	dbus_message_unref(reply);	
	if(ret == 0){
		vty_out(vty,"update wtpcompatible successfully\n");		
	}
	return CMD_SUCCESS;			
}

DEFUN(set_ap_update_fail_count_func,
	  set_ap_update_fail_count_cmd,
	  "set ap update fail count COUNT",
	  CONFIG_STR
	  "ap update information\n"
	  "ap update information\n"
	  "ap update information\n"
	  "ap update count\n"
	  "ap update count\n"
	 )
{	
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	int ret = WID_DBUS_SUCCESS;
	unsigned char count = 0;   //fengwenchao change int to char for AXSSZFI-646,20111215
	
	ret = parse_char_ID((char*)argv[0], &count);  //fengwenchao change int to char for AXSSZFI-646,20111215
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format, input count should be 0~255\n");
		return CMD_SUCCESS;
	}
	/*fengwenchao add 20111215,for AXSSZFI-646*/
	#if 0
	if(count < 0||count > 255)
	{
		vty_out(vty,"<error> input count should be 0~255\n");
		return CMD_SUCCESS;		
	}
	#endif
	/*fengwenchao add end*/
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_AP_UPDATE_FAIL_COUNT);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_AP_UPDATE_FAIL_COUNT);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&count,  //fengwenchao change int to char for AXSSZFI-646,20111215
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
		
	dbus_message_unref(reply);
	if(ret == 0)
	{		
		vty_out(vty,"set update max fail count %s successfully\n",argv[0]);
	}
	else 
	vty_out(vty,"<error> %d\n",ret);
	
	return CMD_SUCCESS;
}

DEFUN(show_ap_update_fail_count_cmd_func,
	  show_ap_update_fail_count_cmd,
	  "show ap update fail count",
	  SHOW_STR
	  "ap update information\n"
	  "ap update information\n"
	  "ap update information\n"
	  "ap update fail count\n"
	 )
{
/*	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;*/
	
//	unsigned int count = 0;
	int ret = 0;
	DCLI_AC_API_GROUP_FIVE *update_fail = NULL;
//	CW_CREATE_OBJECT_ERR(update_fail, DCLI_AC_API_GROUP_FIVE, return NULL;); 

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
#if 0	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_AP_UPDATE_FAIL_COUNT);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SHOW_AP_UPDATE_FAIL_COUNT);*/
	
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&count);
#endif
	update_fail = dcli_ac_show_api_group_five(
		index,
		TWELFTH,/*"show wireless-control config"*/
		0,
		0,
		1,/*didn't transfer ret,so set it to 1 for function dcli_ac_show_api_group_five() */
		&ret,
		0,
		0,
		localid,
	//	update_fail,
		dcli_dbus_connection,
		WID_DBUS_CONF_METHOD_SHOW_AP_UPDATE_FAIL_COUNT
		);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
		vty_out(vty,"AP update max fail count %d\n",update_fail->num);
		dcli_ac_free_fun_five(WID_DBUS_CONF_METHOD_SHOW_AP_UPDATE_FAIL_COUNT,update_fail);
		update_fail = NULL;
	}
//	dbus_message_unref(reply);		
//	CW_FREE_OBJECT(update_fail); 	
	return CMD_SUCCESS;
	
}

DEFUN(set_wid_watch_dog_cmd_func,
	  set_wid_watch_dog_cmd,
	  "set wid watch dog (open|close)",
	  CONFIG_STR
	  "wid watch dog\n"
	  "wid watch dog\n"
	  "wid watch dog\n"
	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	unsigned char open;
	int ret;

	if (!strcmp(argv[0],"open"))
	{
		open = 1;	
	}		
	else if (!strcmp(argv[0],"close"))
	{
		open = 0;
	}

	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_WATCH_DOG);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_WATCH_DOG);*/


	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&open,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		vty_out(vty,"set wid watch dog %s successfully\n",argv[0]);
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;		
	
}

DEFUN(show_ap_network_func,
		  show_ap_network_bywtpid_cmd,
		  "show ap network bywtp ID",
		  "show ap network link\n"
		  "show ap network link by wtpid \n"
		  "show ap network information\n"
	 )
{
	int ret = -1;
	int retval;

	DCLI_AC_API_GROUP_THREE *network = NULL;
	//CW_CREATE_OBJECT_ERR(network, DCLI_AC_API_GROUP_THREE, return NULL;);	
	//network->WTPIP = NULL;
	int wtpid = 0;
	wtpid = atoi(argv[0]);
	if ((wtpid <= 0) || (wtpid >= WTP_NUM)) {
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_FAILURE;
		}	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);


//	DBusMessage *query, *reply;	
//	DBusMessageIter	 iter;
//	DBusError err;

	char ip[WTP_WTP_IP_LEN+1];
	
	unsigned char myipBuf[DCLIAC_BUF_LEN] = {0};	
	unsigned char *myipPtr = myipBuf;
	unsigned int ipaddr = 0;

//	unsigned char *wtpip = NULL; 
	
	unsigned char mygatewayBuf[DCLIAC_BUF_LEN] = {0};	
	unsigned char *mygatewayPtr = mygatewayBuf;
//	unsigned int gatewayaddr = 0;

	unsigned char myfstdnsBuf[DCLIAC_BUF_LEN] = {0};	
	unsigned char *myfstdnsPtr = myfstdnsBuf;
//	unsigned int fstdnsaddr = 0;

	unsigned char mysnddnsBuf[DCLIAC_BUF_LEN] = {0};	
	unsigned char *mysnddnsPtr = mysnddnsBuf;
//	unsigned int snddnsaddr = 0;	

	unsigned char mymaskBuf[DCLIAC_BUF_LEN] = {0};	
	unsigned char *mymaskPtr = mymaskBuf;
//	unsigned int maskaddr = 0;	
#if 0

	wtpid = atoi(argv[0]);
	if ((wtpid <= 0) || (wtpid >= WTP_NUM)) {
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_FAILURE;
		}

	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_SHOW_NETWORK);	
	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AP_SHOW_NETWORK);
					*/
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret == 0 )
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&wtpip);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&maskaddr);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&gatewayaddr);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&fstdnsaddr);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&snddnsaddr);		
	
	
		//retval = ip_long2str(ipaddr,&myipPtr);
		retval = ip_long2str(gatewayaddr,&mygatewayPtr);
		retval = ip_long2str(maskaddr,&mymaskPtr);
		retval = ip_long2str(fstdnsaddr,&myfstdnsPtr);
		retval = ip_long2str(snddnsaddr,&mysnddnsPtr);		

		retval = wtp_check_wtp_ip_addr(ip,wtpip);
					

		
		vty_out(vty,"show ap ip list \n");
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"%-18s %-16s %-14s %-14s %-14s\n","ipaddr","netmask","gateway","fstdns","snddns");

		if(retval == 0)
		{
			vty_out(vty,"%-18s %-16s %-14s %-14s %-14s\n",\
			    wtpip,mymaskBuf,mygatewayBuf,myfstdnsBuf,mysnddnsBuf);
		}
		else
		{
			vty_out(vty,"%-18s %-16s %-14s %-14s %-14s\n",\
			    ip,mymaskBuf,mygatewayBuf,myfstdnsBuf,mysnddnsBuf);
		}
		
		vty_out(vty,"==============================================================================\n");
	}
#endif
	network = dcli_ac_show_api_group_three(
		index,
		FIRST,/*"show ap network bywtp ID"*/
		wtpid,
		0,
		0,
		&ret,
		0,
		&localid,
		//network,
		dcli_dbus_connection,
		WID_DBUS_CONF_METHOD_AP_SHOW_NETWORK
		);
	//printf("#######ret is %d \n",ret);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
		retval = ip_long2str(network->ap_gateway,&mygatewayPtr);
		retval = ip_long2str(network->ap_mask_new,&mymaskPtr);
		retval = ip_long2str(network->ap_dnsfirst,&myfstdnsPtr);
		retval = ip_long2str(network->ap_dnssecend,&mysnddnsPtr);		

		retval = wtp_check_wtp_ip_addr(ip,network->WTPIP);			

		
		vty_out(vty,"show ap ip list \n");
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"%-18s %-16s %-14s %-14s %-14s\n","ipaddr","netmask","gateway","fstdns","snddns");

		if(retval == 0)
		{
			vty_out(vty,"%-18s %-16s %-14s %-14s %-14s\n",\
			    network->WTPIP,mymaskBuf,mygatewayBuf,myfstdnsBuf,mysnddnsBuf);
		}
		else
		{
			vty_out(vty,"%-18s %-16s %-14s %-14s %-14s\n",\
			    ip,mymaskBuf,mygatewayBuf,myfstdnsBuf,mysnddnsBuf);
		}
		
		vty_out(vty,"==============================================================================\n");
		dcli_ac_free_fun_three(WID_DBUS_CONF_METHOD_AP_SHOW_NETWORK,network);
		network = NULL;
	}
	else if(ret == WID_AP_NO_STATICS)
	{
		vty_out(vty,"ap have not ip information\n");
	}
	else if(ret == WTP_ID_NOT_EXIST)
	{
		vty_out(vty,"wtp id no exist\n");
	}
	else if(ret == WTP_ID_LARGE_THAN_MAX)
	{
		vty_out(vty,"<error> input wtp id should be 1 to %d\n",WTP_NUM-1);
	}
		
//	dbus_message_unref(reply);	
//	CW_FREE_OBJECT(network);
	return CMD_SUCCESS;			
}

DEFUN(ac_load_balance_cmd_func,
	  ac_load_balance_cmd,
	  "set ac balance method (number | flow | disable)",
	  "ac load balance\n"
	  "the method of balance\n" 
	  "enable or disable\n"

	 )
{
	int ret;
	unsigned char method=0;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	

	if (!strcmp(argv[0],"number")||(tolower(argv[0][0]) == 'n')){
		method=1;
	}
	else if (!strcmp(argv[0],"flow")||(tolower(argv[0][0]) == 'f')){
		method=2;
	} 
	else if (!strcmp(argv[0],"disable")||(tolower(argv[0][0]) == 'd')){
		method=0;
	}

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AC_LOAD_BALANCE);	

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AC_LOAD_BALANCE);
*/
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&method,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		vty_out(vty,"set ac %s balance successfully.\n",argv[0]);
	else if(ret == WID_DBUS_ERROR)
		vty_out(vty,"<error> operation fail .\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/*ht add 10.03.05*/
DEFUN(ac_balance_parameter_cmd_func,
	  ac_balance_parameter_cmd,
	  "set ac (number | flow) balance parameter NUMBER",
	  "set ac balance parameter\n"
	  "ac balance parameter\n" 
	  "number balance parameter <1-10>\n"
	  "flow balance parameter <1-30>\n"
	 )
{
	int ret;
	unsigned char method = 0;
	unsigned int bal_para=0;
	int res = WID_DBUS_SUCCESS;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	
	if (!strcmp(argv[0],"number")||(tolower(argv[0][0]) == 'n')){
		method=1;
	}
	else if (!strcmp(argv[0],"flow")||(tolower(argv[0][0]) == 'f')){
		method=2;
	}else {
		vty_out(vty,"<error> unknown input\n");
		return CMD_SUCCESS;
	}
	
	res = parse_int((char*)argv[1], &bal_para);
	if(res != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown input\n");
		return CMD_SUCCESS;
	}	
	if(method == 1){
		if(bal_para > 10 || bal_para < 1){
			vty_out(vty,"<error> balance parameter should be 1 to 10\n");
			return CMD_SUCCESS;
		}
	}else if(method == 2){
		if(bal_para > 30 || bal_para < 1){
			vty_out(vty,"<error> balance parameter should be 1 to 30\n");
			return CMD_SUCCESS;
		}
	}
			
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_AC_LOAD_BALANCE_PARA);	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&method,
							 DBUS_TYPE_UINT32,&bal_para,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		vty_out(vty,"set ac %s balance parameter %d successfully.\n",argv[0],bal_para);
	else if(ret == WID_DBUS_ERROR)
		vty_out(vty,"<error> operation fail .\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
/*nl add 20100316*/
DEFUN(set_ac_all_ap_extension_information_enable_func,
	  set_ac_all_ap_extension_information_enable_cmd,
	  "set ac extension infomation switch  (enable|disable)",
	  CONFIG_STR
	  "wireless-control config\n"
	  "all ap extension infomation\n"
	  "enable|disable\n"
	 )
{
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	dbus_error_init(&err);

    int policy = 0;

	if (!strcmp(argv[0],"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[0],"disable"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_AC_EXTENTION_INFOR_ENABLE);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," set ac extension infomation switch %s successfully\n",argv[0]);
	}				\
	else if(ret == WTP_ID_NOT_EXIST)
	{
		vty_out(vty," Operation successed.There is no wtp now.\n");
	}
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}

DEFUN(show_ac_balance_func,
	  show_ac_balance_cmd,
	  "show ac balance configuration",
	  SHOW_STR
	  "ac configuration\n"
	  "ac configuration information\n"
	 )
{	
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	int ret = WID_DBUS_SUCCESS;
	int i = 0;

	DCLI_AC_API_GROUP_FIVE *balance = NULL;

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	balance = dcli_ac_show_api_group_five(
		index,
		THIRTEENTH,/*"show ac balance configuration"*/
		0,
		0,
		0,
		&ret,
		0,
		0,
		localid,
	//	balance,
		dcli_dbus_connection,
		WID_DBUS_CONF_METHOD_SHOW_AC_BALANCE_CONFIGURATION
		);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){	
		char *state[3]={"disable","number","flow"};
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"AC balance state :       %s\n",state[balance->state]);			
		vty_out(vty,"AC num balance para :    %d\n",balance->number);
		vty_out(vty,"AC flow balance  para :  %d\n",balance->flow);
		vty_out(vty,"==============================================================================\n");
		dcli_ac_free_fun_five(WID_DBUS_CONF_METHOD_SHOW_AC_BALANCE_CONFIGURATION,balance);
		balance = NULL;
	}
	else if(ret == WID_DBUS_ERROR) 
		vty_out(vty,"<error> other error.error id : %d \n",ret);

//	CW_FREE_OBJECT(balance);		
	return CMD_SUCCESS;
}

DEFUN(set_ap_hotreboot_func,
		  set_ap_hotreboot_cmd,
		  "set ap hotreboot (enable|disable)",
		  "ap hotreboot config\n"
		  "ap hotreboot information enable|disable when ap quit\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    int policy = 0;

	
	if (!strcmp(argv[0],"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[0],"disable"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_HOTREBOOT);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AP_HOTREBOOT);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," set ap hotreboot %s successfully\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}

DEFUN(set_ap_access_through_nat_func,
		  set_ap_access_through_nat_cmd,
		  "set ap access through nat (enable|disable)",
		  "ap access through nat\n"
		  "ap access through nat\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    int policy = 0;

	
	if (!strcmp(argv[0],"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[0],"disable"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_ACCESS_THROUGH_NAT);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AP_ACCESS_THROUGH_NAT);*/
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," set ap access through nat %s successfully\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}

DEFUN (show_hansi_interface, show_hansi_interface_cmd,
       "show interface [IFNAME]",  
       SHOW_STR
       "Interface status and configuration\n"
       "Inteface name\n")
{
		int wtpid,radioid,wlanid;
		unsigned char id;
		char name[128];
		int index = 0;	
		int local = 0;
		int slotid = 0;
		int vrrid1 =0 ;
        FILE *fp = NULL;
    /*    fp = stdout;*/
		unsigned int ebr_id =0;
		memset(name, 0, 128);

		/*gujd : 2013-01-28, pm 4:30. Change fp from stdout to vty->fp, which is used by ">more" */
		if(vty->fp)
		   fp = vty->fp;
		else
		   fp = stdout;
		
	   if(argc != 0){
		  if (!strncasecmp(argv[0],"wlan",4)){
			  if(vty->node == HANSI_NODE){
				  index = (int)vty->index;
			  	  slotid = vty->slotindex;
			}
			  else if(vty->node == LOCAL_HANSI_NODE){
				  index = (int)vty->index;
			  	  slotid = vty->slotindex;
				  local = 1;
			  }
			if(parse_char_ID((char *)(argv[0]+4), &id) == WID_DBUS_SUCCESS){
				if(local)
					sprintf(name,"show interface wlanl%d-%d-%d",slotid,index,id);
				else
					sprintf(name,"show interface wlan%d-%d-%d",slotid,index,id);					
				vtysh_client_execute(&vtysh_client[0], name, fp);
			}else if(parse_wlan_ifname(argv[0]+4, &vrrid1,&id)==0){
				if(local)
					sprintf(name,"show interface wlanl%d-%d-%d",slotid,vrrid1,id);
				else
					sprintf(name,"show interface wlan%d-%d-%d",slotid,vrrid1,id);
			}
			else{
			    if(parse_char_ID((char *)(argv[0]+4), &id) == WID_ILLEGAL_INPUT){
                    vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
                    return CMD_SUCCESS;
				}
				sprintf(name,"show interface %s",argv[0]);
				vtysh_client_execute(&vtysh_client[0], name, fp);
			}
		  }else if (!strncasecmp(argv[0],"radio",5)){
			  if(vty->node == HANSI_NODE){
				  index = (int)vty->index;
			  	  slotid = vty->slotindex;
			  }
			  if(vty->node == LOCAL_HANSI_NODE){
				  index = (int)vty->index;
				  slotid = vty->slotindex;
				  local = 1;
			  }
			  if(parse_radio_ifname((char *)(argv[0]+5), &wtpid,&radioid,&wlanid) == 0){				  
				  //sprintf(name,"show interface radio%d-%d-%d.%d",index,wtpid,radioid,wlanid);	
				  if(local)
				  	sprintf(name,"show interface r%d-%d-%d.%d",index,wtpid,radioid,wlanid);
				  else
					sprintf(name,"show interface r%d-%d-%d-%d.%d",slotid,index,wtpid,radioid,wlanid);
				  	
				  vtysh_client_execute(&vtysh_client[0], name, fp);
			  }else if (parse_radio_ifname_v2(argv[0]+5, &wtpid,&radioid,&wlanid,&vrrid1) == 0){
				  if(local)
				  	sprintf(name,"show interface r%d-%d-%d.%d",vrrid1,wtpid,radioid,wlanid);
				  else
					sprintf(name,"show interface r%d-%d-%d-%d.%d",slotid,vrrid1,wtpid,radioid,wlanid);
				  	
				  vtysh_client_execute(&vtysh_client[0], name, fp);
			  }
			  else{
				  sprintf(name,"show interface r%s",argv[0]+5);
				  vtysh_client_execute(&vtysh_client[0], name, fp);
			  } 	
		  }else if (!strncasecmp(argv[0],"r",1)){
			  if(vty->node == HANSI_NODE){
				  index = (int)vty->index;
			  	  slotid = vty->slotindex;
			  }
			  if(vty->node == LOCAL_HANSI_NODE){
				  index = (int)vty->index;
				  slotid = vty->slotindex;
				  local = 1;
			  }
			  if(parse_radio_ifname((char *)(argv[0]+1), &wtpid,&radioid,&wlanid) == 0){
				  //sprintf(name,"show interface radio%d-%d-%d.%d",index,wtpid,radioid,wlanid);	
				  if(local)
				  	sprintf(name,"show interface r%d-%d-%d.%d",index,wtpid,radioid,wlanid);
				  else
					sprintf(name,"show interface r%d-%d-%d-%d.%d",slotid,index,wtpid,radioid,wlanid);
				  vtysh_client_execute(&vtysh_client[0], name, fp);
			  }else if (parse_radio_ifname_v2(argv[0]+1, &wtpid,&radioid,&wlanid,&vrrid1) == 0){
				  if(local)
				  	sprintf(name,"show interface r%d-%d-%d.%d",vrrid1,wtpid,radioid,wlanid);
				  else
					sprintf(name,"show interface r%d-%d-%d-%d.%d",slotid,vrrid1,wtpid,radioid,wlanid);
				  	
				  vtysh_client_execute(&vtysh_client[0], name, fp);
			  }
			  else{
				  sprintf(name,"show interface %s",argv[0]);
				  vtysh_client_execute(&vtysh_client[0], name, fp);
			  } 	
		  }else if (!strncasecmp(argv[0],"ebr",3)){
			  if(vty->node == HANSI_NODE){
				  index = (int)vty->index;
				  slotid = vty->slotindex;
			  }
			  else if (vty->node == LOCAL_HANSI_NODE){
				  index = (int)vty->index;
				  slotid = vty->slotindex;
				  local = 1;
			  }
			  if(parse_int_ID((char *)(argv[0]+3), &ebr_id)== WID_DBUS_SUCCESS){
			  	  if(local)
				  	  sprintf(name,"show interface ebrl%d-%d-%d",slotid,index,ebr_id);
				  else
					  sprintf(name,"show interface ebr%d-%d-%d",slotid,index,ebr_id);
				  vtysh_client_execute(&vtysh_client[0], name, fp);
			  }else if(parse_wlan_ifname_v2(argv[0]+3, &vrrid1,&ebr_id)==0){
				if(local)
					sprintf(name,"show interface ebrl%d-%d-%d",slotid,vrrid1,ebr_id);
				else
					sprintf(name,"show interface ebr%d-%d-%d",slotid,vrrid1,ebr_id);					
			  }
			  else{
			       if(parse_int_ID((char *)(argv[0]+3), &ebr_id) == WID_ILLEGAL_INPUT){
                    vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
                    return CMD_SUCCESS;
				 }
				  sprintf(name,"show interface %s",argv[0]);
				  vtysh_client_execute(&vtysh_client[0], name, fp);
			  }		
		  }
		  else /*gjd : add for other interface show under hansi node . 2011-10-20 : pm 5;25.*/
		  	{
		  		sprintf(name,"show interface %s",argv[0]);
				vtysh_client_execute(&vtysh_client[0], name, fp);
		  	}
	  }else{
			  sprintf(name,"show interface");
			  vtysh_client_execute(&vtysh_client[0], name, fp);
	  }
	  return CMD_SUCCESS;
}

DEFUN(set_wtp_wids_policy_cmd_func,
	  set_wtp_wids_policy_cmd,
	  "set wtp wids policy (no|forbid)",
	  "wireless-control config\n"
	  "wtp infomation\n"
	  "wtp wids infomation\n"
	  "wtp wids policy\n"
	  "wtp wids policy no or forbid\n"
	 )
{
	int ret = 0;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    unsigned char policy = 0;
	
	if (!strcmp(argv[0],"forbid"))
	{
		policy = 1;	
	}	
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SET_WTP_WIDS_POLICY);

	
	//query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
	//					WID_DBUS_INTERFACE,WID_DBUS_WTP_METHOD_SET_WTP_WIDS_POLICY);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,	
							 DBUS_TYPE_BYTE,&policy,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"set wtp wids policy %s successfully\n",argv[0]);
	}
	else if (ret == SWITCH_IS_DISABLE)
		vty_out(vty,"<error> wids switch is enable\n");
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;			
}

DEFUN(add_wids_mac_func,
		  add_wids_mac_cmd,
		  "add wids ignore mac MAC",
		  "wireless control\n"
		  "add wids ignore MAC \n"
		  "MAC xx:xx:xx:xx:xx:xx\n"
	 )
{
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	WIDMACADDR  macaddr;
	ret = wid_parse_mac_addr((char *)argv[0],&macaddr);
	if (CMD_FAILURE == ret) {
		vty_out(vty,"<error> Unknown mac addr format.\n");
		return CMD_FAILURE;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_ADD_WID_MAC);
	
	//query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
	//					WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_ADD_WID_MAC);

	dbus_error_init(&err);
	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[0],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[1],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[2],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[3],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[4],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[5],
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," add wids ignore mac successfully\n");
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return CMD_SUCCESS;			
}

DEFUN(del_wids_mac_func,
		  del_wids_mac_cmd,
		  "del wids ignore mac MAC",
		  "wireless control\n"
		  "del wids ignore MAC \n"
		  "MAC xx:xx:xx:xx:xx:xx\n"
	 )
{
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	WIDMACADDR  macaddr;
	ret = wid_parse_mac_addr((char *)argv[0],&macaddr);
	if (CMD_FAILURE == ret) {
		vty_out(vty,"<error> Unknown mac addr format.\n");
		return CMD_FAILURE;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_DEL_WID_MAC);
		
	//query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
		//				WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_DEL_WID_MAC);

	dbus_error_init(&err);
	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[0],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[1],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[2],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[3],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[4],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[5],
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," del wids ignore mac successfully\n");
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return CMD_SUCCESS;			
}

DEFUN(show_wids_mac_cmd_func,
		  show_wids_mac_list_cmd,
		  "show wids ignore mac list",
		  "show wids ignore mac list\n"
		   "show wids ignore mac list\n"
	 )
{
	int ret;
	int num = 0;
	int i = 0;
	int length = 0;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter,iter_array;
	DBusError err;

    int state = 1;
	unsigned char mac[DCLIAC_MAC_LEN];	

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE, WID_DBUS_CONF_METHOD_WIDS_MAC_SHOW );
	//query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
		//				WID_DBUS_INTERFACE, WID_DBUS_CONF_METHOD_WIDS_MAC_SHOW );
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret == 0)
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for (i = 0; i < num; i++)
		{
			DBusMessageIter iter_struct;
			
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(mac[0]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(mac[1]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(mac[2]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(mac[3]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(mac[4]));
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(mac[5]));

			dbus_message_iter_next(&iter_array);


			vty_out(vty,"\t%d\t%02X:%02X:%02X:%02X:%02X:%02X\n",i+1,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

		}
	}
	else if(ret == WID_NO_WHITELIST)
	{
		vty_out(vty,"there is no wids ignore mac list \n");
	}
	else
	{
		vty_out(vty,"error %d \n",ret);
	}
		

		
	dbus_message_unref(reply);	
	
	return CMD_SUCCESS;				
}

DEFUN(set_ap_countermeasures_func,
		  set_ap_countermeasures_cmd,
		  "set radio resource management countermeasures (enable|disable)",
		  "ap rrm countermeasures config\n"
		  "ap countermeasures policy enable|disable\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    int policy = 0;

	
	if (!strcmp(argv[0],"enable"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[0],"disable"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_COUNTERMEASURES);
		
	//query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
	//					WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AP_COUNTERMEASURES);
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," set ap countermeasures %s successfully\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}

DEFUN(set_ap_countermeasures_mode_func,
		  set_ap_countermeasures_mode_cmd,
		  "set radio resource management countermeasures mode (ap|adhoc|all)",
		  "ap rrm countermeasures config\n"
		  "ap countermeasures policy enable|disable\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

    int policy = 0;

	
	if (!strcmp(argv[0],"ap"))
	{
		policy = 0;	
	}
	else if (!strcmp(argv[0],"adhoc"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[0],"all"))
	{
		policy = 2;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_AP_COUNTERMEASURES_MODE);
		
	//query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
		//				WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_AP_COUNTERMEASURES_MODE);
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," set ap countermeasures mode %s successfully\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}

DEFUN(modify_legal_essid_func,
		  modify_legal_essid_cmd,
		  "modify legal essid  ESSID to ESSID",
		  "modify legal essid\n"
	 )
{

	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	if (strlen(argv[0]) > 32)
	{
		vty_out(vty,"<error> first essid is too long,out of the limit of 32\n");
		return CMD_SUCCESS;
	}
	if (strlen(argv[1]) > 32)
	{
		vty_out(vty,"<error> second essid is too long,out of the limit of 32\n");
		return CMD_SUCCESS;
	}

	char *essid=(char *)malloc(strlen(argv[0])+1);
	memset(essid,0,strlen(argv[0])+1);
	memcpy(essid,argv[0],strlen(argv[0]));

	char *essid_new=(char *)malloc(strlen(argv[1])+1);
	memset(essid_new,0,strlen(argv[1])+1);
	memcpy(essid_new,argv[1],strlen(argv[1]));
	
	/*printf("essid:\t%s\n",essid);*/
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_MODIFY_ESSID);


	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_MODIFY_ESSID );*/
	dbus_error_init(&err);
	

	dbus_message_append_args(query,
							 DBUS_TYPE_STRING,&essid,
							 DBUS_TYPE_STRING,&essid_new,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	free(essid);
	essid=NULL;
	free(essid_new);
	essid_new=NULL;
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," modify legal essid %s successfully\n",argv[0]);
	}	
	else if(ret == ESSID_LIST_IS_NULL)
	{
		vty_out(vty,"<error>%d The essid list is null,there is no essid!\n",ret);
	}
	else if(ret == ESSID_NOT_EXIT)
	{
		vty_out(vty,"<error>%d The essid input is not exit!\n",ret);
	}
	else 
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return CMD_SUCCESS;	
}

DEFUN(set_wid_mac_whitelist_cmd_func,
	  set_wid_mac_whitelist_cmd,
	  "set mac whitelist (enable|disable)",
	  CONFIG_STR
	  "wireless-control mac whitelist\n"
	  "wireless-control mac whitelist\n"
	  "wireless-control mac whitelist enable/disable\n"
	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	unsigned char macfilterflag = 0;
	int ret;

	if (!strcmp(argv[0],"enable"))
	{
		macfilterflag = 1;	
	}	
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_MAC_WHITELIST_SWITCH);
	
	//query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
	//					WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_MAC_WHITELIST_SWITCH);


	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&macfilterflag,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		vty_out(vty,"wireless-control set mac whitelist  %s successfully\n",argv[0]);
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;		
	
}

DEFUN(set_wid_essid_whitelist_cmd_func,
	  set_wid_essid_whitelist_cmd,
	  "set essid whitelist (enable|disable)",
	  CONFIG_STR
	  "wireless-control essid whitelist\n"
	  "wireless-control essid whitelist\n"
	  "wireless-control essid whitelist enable/disable\n"
	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	unsigned char essidfilterflag = 0;
	int ret;

	if (!strcmp(argv[0],"enable"))
	{
		essidfilterflag = 1;	
	}	

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_ESSID_WHITELIST_SWITCH);
	
	//query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
		//				WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_ESSID_WHITELIST_SWITCH);


	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&essidfilterflag,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		vty_out(vty,"wireless-control set essid whitelist  %s successfully\n",argv[0]);
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;		
	
}

DEFUN(change_wirelesscontrol_whitelist_func,
		  change_wirelesscontrol_whitelist_cmd,
		  "change mac MACSRC to MACDES whitelist",
		  "wireless control\n"
		  "change MAC into whitelist\n"
		  "MACSRC xx:xx:xx:xx:xx:xx\n"
		  "MACDES xx:xx:xx:xx:xx:xx\n"
	 )
{
	int ret;
	WIDMACADDR macaddr; 
	WIDMACADDR macaddrdest; 
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	ret = wid_parse_mac_addr((char *)argv[0],&macaddr);
	if (CMD_FAILURE == ret) {
		vty_out(vty,"<error> Unknown mac addr format.\n");
		return CMD_FAILURE;
	}
	
	ret = wid_parse_mac_addr((char *)argv[1],&macaddrdest);
	if (CMD_FAILURE == ret) {
		vty_out(vty,"<error> Unknown mac addr format.\n");
		return CMD_FAILURE;
	}	

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_CHANGE_WHITELIST);	
	//query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
		//				WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_CHANGE_WHITELIST);
	dbus_error_init(&err);
	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[0],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[1],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[2],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[3],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[4],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[5],

							 DBUS_TYPE_BYTE,  &macaddrdest.macaddr[0],
							 DBUS_TYPE_BYTE,  &macaddrdest.macaddr[1],
							 DBUS_TYPE_BYTE,  &macaddrdest.macaddr[2],
							 DBUS_TYPE_BYTE,  &macaddrdest.macaddr[3],
							 DBUS_TYPE_BYTE,  &macaddrdest.macaddr[4],
							 DBUS_TYPE_BYTE,  &macaddrdest.macaddr[5],							 
							 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	/*printf("mac :: %02X:%02X:%02X:%02X:%02X:%02X\n",macaddr.macaddr[0],macaddr.macaddr[1],macaddr.macaddr[2],macaddr.macaddr[3],macaddr.macaddr[4],macaddr.macaddr[5]);*/
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," change mac MAC whitelist successfully\n");
	}	
	else if(MAC_DOESNOT_EXIT == ret)
	{
		vty_out(vty,"src mac isn't exist\n");
	}	
	else if(MAC_ALREADY_EXIT == ret)
	{
		vty_out(vty,"dst mac already in white list\n");
	}		
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return CMD_SUCCESS;			
}

DEFUN(change_wirelesscontrol_blacklist_func,
		  change_wirelesscontrol_blacklist_cmd,
		  "change mac MACSRC to MACDES blacklist",
		  "wireless control\n"
		  "change MAC into blacklist\n"
		  "MACSRC xx:xx:xx:xx:xx:xx\n"
		  "MACDES xx:xx:xx:xx:xx:xx\n"
	 )
{
	int ret;
	WIDMACADDR macaddr; 
	WIDMACADDR macaddrdest; 
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	ret = wid_parse_mac_addr((char *)argv[0],&macaddr);
	if (CMD_FAILURE == ret) {
		vty_out(vty,"<error> Unknown mac addr format.\n");
		return CMD_FAILURE;
	}
	
	ret = wid_parse_mac_addr((char *)argv[1],&macaddrdest);
	if (CMD_FAILURE == ret) {
		vty_out(vty,"<error> Unknown mac addr format.\n");
		return CMD_FAILURE;
	}	
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_CHANGE_BLACKLIST);		
	//query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
		//				WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_CHANGE_BLACKLIST);
	dbus_error_init(&err);
	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[0],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[1],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[2],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[3],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[4],
							 DBUS_TYPE_BYTE,  &macaddr.macaddr[5],

							 DBUS_TYPE_BYTE,  &macaddrdest.macaddr[0],
							 DBUS_TYPE_BYTE,  &macaddrdest.macaddr[1],
							 DBUS_TYPE_BYTE,  &macaddrdest.macaddr[2],
							 DBUS_TYPE_BYTE,  &macaddrdest.macaddr[3],
							 DBUS_TYPE_BYTE,  &macaddrdest.macaddr[4],
							 DBUS_TYPE_BYTE,  &macaddrdest.macaddr[5],							 
							 
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	/*printf("mac :: %02X:%02X:%02X:%02X:%02X:%02X\n",macaddr.macaddr[0],macaddr.macaddr[1],macaddr.macaddr[2],macaddr.macaddr[3],macaddr.macaddr[4],macaddr.macaddr[5]);*/
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty," change mac MAC whitelist successfully\n");
	}	
	else if(ret == MAC_DOESNOT_EXIT)
	{
		vty_out(vty,"src mac isn't exist\n");
	}	
	else if(ret == MAC_ALREADY_EXIT)
	{
		vty_out(vty,"dst mac already in black list\n");
	}		
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return CMD_SUCCESS;			
}

DEFUN(set_dbus_count_func,
	  set_dbus_count_cmd,
	  "set (asd|wid) dbus count (enable|disable)",
	  "set asd/wid dbus count switch"
	  "asd or wid"
	  "enable or disable"
	 )
{	
	int ret = ASD_DBUS_SUCCESS;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	unsigned int type = 0;
	if(strncmp("enable",argv[1],strlen(argv[1])) == 0){
		type = 1;
	}else if(strncmp("disable",argv[1],strlen(argv[1])) == 0){
		type = 0;
	}else{
		vty_out(vty,"<error>  unknow command\n");	
		return CMD_SUCCESS;
	}
	
	if(strncmp("asd",argv[0],strlen(argv[0])) == 0){

		int localid = 1;
		int slot_id = HostSlotId;
		int index = 0;
		char BUSNAME[PATH_LEN];
		char OBJPATH[PATH_LEN];
		char INTERFACE[PATH_LEN];
		if((vty->node == CONFIG_NODE)){
			index = 0;
		}else if(vty->node == HANSI_NODE){
			index = (int)vty->index;
			localid = vty->local;
			slot_id = vty->slotindex;
		}
		else if(vty->node == LOCAL_HANSI_NODE){
			index = (int)vty->index;
			localid = vty->local;
			slot_id = vty->slotindex;
		}
		DBusConnection *dcli_dbus_connection = NULL;
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SET_DBUS_COUNT);

		
		/*ery = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
							ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SET_DBUS_COUNT);*/
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32, &type,								 
								 DBUS_TYPE_INVALID);

		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		
		if (NULL == reply) {
			cli_syslog_info("<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				cli_syslog_info("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}
		
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);

		if(ret == ASD_DBUS_SUCCESS){
			vty_out(vty,"set %s dbus count %s successful\n",argv[0],argv[1]);
		}else
			vty_out(vty,"<error>  %d\n",ret);
		
		dbus_message_unref(reply);
	}
	else if(strncmp("wid",argv[0],strlen(argv[0])) == 0){

		int localid = 1;
		int slot_id = HostSlotId;
		int index = 0;
		char BUSNAME[PATH_LEN];
		char OBJPATH[PATH_LEN];
		char INTERFACE[PATH_LEN];
		if(vty->node == CONFIG_NODE){
			index = 0;
		}else if(vty->node == HANSI_NODE){
			index = (int)vty->index;
			localid = vty->local;
			slot_id = vty->slotindex;
		}
		else if(vty->node == LOCAL_HANSI_NODE){
			index = (int)vty->index;
			localid = vty->local;
			slot_id = vty->slotindex;
		}
		DBusConnection *dcli_dbus_connection = NULL;
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_DBUS_COUNT);		

		
		/*ery = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
							WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SET_DBUS_COUNT);*/
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32, &type,								 
								 DBUS_TYPE_INVALID);	
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		
		if (NULL == reply) {
			cli_syslog_info("<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				cli_syslog_info("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}
		
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);	
	
		if(ret == WID_DBUS_SUCCESS){
			vty_out(vty,"set %s dbus count %s successful\n",argv[0],argv[1]);
		}else
			vty_out(vty,"<error>  %d\n",ret);
		
		dbus_message_unref(reply);
	}else{
		vty_out(vty,"<error>  unknow command\n");
	}
	return CMD_SUCCESS;
}

DEFUN(wireless_interface_vmac_state_func,
	  wireless_interface_vmac_state_cmd,
	  "set wireless interface vmac (enable|disable)",
	  "notice vrrp state\n"
	 )
{
	int ret;

	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int policy;
	if(strcmp(argv[0],"enable") == 0){
		policy = 1;
	}else if(strcmp(argv[0],"disable") == 0){
		policy = 0;
	}else{
		vty_out(vty,"unknown command\n");	
		return CMD_SUCCESS;		
	}

	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_BAK_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_BAK_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_AC_METHOD_SET_INTERFACE_VMAC);		
//	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_BAK_OBJPATH,\
//						WID_BAK_INTERFACE,WID_DBUS_AC_METHOD_SET_INTERFACE_VMAC);
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);	
	dbus_message_unref(reply);	
	if(ret == 0){
		vty_out(vty,"set wireless interface vmac %s successfully\n",argv[0]);		
	}
	return CMD_SUCCESS;			
}

DEFUN(show_dbus_count_func,
	  show_dbus_count_cmd,
	  "show (asd|wid) dbus count",
	  "display asd/wid dbus count"
	  "asd or wid"
	  "dispaly dbus count"
	 )
{	
	int ret = ASD_DBUS_SUCCESS;
	unsigned int type[7] = {0};
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	if(strncmp("asd",argv[0],strlen(argv[0]))==0){

		
		int localid = 1;
		int slot_id = HostSlotId;
		int index = 0;
		char BUSNAME[PATH_LEN];
		char OBJPATH[PATH_LEN];
		char INTERFACE[PATH_LEN];
		if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
			index = 0;
		}else if(vty->node == HANSI_NODE){
			index = (int)vty->index;
			localid = vty->local;
			slot_id = vty->slotindex;
		}
		else if(vty->node == LOCAL_HANSI_NODE){
			index = (int)vty->index;
			localid = vty->local;
			slot_id = vty->slotindex;
		}
		DBusConnection *dcli_dbus_connection = NULL;
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SHOW_DBUS_COUNT);
		
		/*query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
							ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SHOW_DBUS_COUNT);*/
		dbus_error_init(&err);

		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		
		if (NULL == reply) {
			cli_syslog_info("<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				cli_syslog_info("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}
		
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(type[0]));	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(type[1]));	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(type[2]));	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(type[3]));	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(type[4]));	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(type[5]));	


		if(ret == ASD_DBUS_SUCCESS){
			vty_out(vty,"ASD dbus count\n");
			vty_out(vty,"======================================================\n");
			vty_out(vty,"SNMP:    %d\n",type[0]);
			vty_out(vty,"CLI:     %d\n",type[1]);
			vty_out(vty,"WEB:     %d\n",type[2]);
			vty_out(vty,"PORTAL:  %d\n",type[3]);
			vty_out(vty,"TRAP:    %d\n",type[4]);
			vty_out(vty,"OTHER:   %d\n",type[5]);
			vty_out(vty,"======================================================\n");
		}else
			vty_out(vty,"<error>  %d\n",ret);
		
		dbus_message_unref(reply);
	}
	else if(strncmp("wid",argv[0],strlen(argv[0]))==0){


		
		int localid = 1;
		int slot_id = HostSlotId;
		int index = 0;
		char BUSNAME[PATH_LEN];
		char OBJPATH[PATH_LEN];
		char INTERFACE[PATH_LEN];
		if(vty->node == CONFIG_NODE){
			index = 0;
		}else if(vty->node == HANSI_NODE){
			index = (int)vty->index;
			localid = vty->local;
			slot_id = vty->slotindex;
		}
		else if(vty->node == LOCAL_HANSI_NODE){
			index = (int)vty->index;
			localid = vty->local;
			slot_id = vty->slotindex;
		}
		DBusConnection *dcli_dbus_connection = NULL;
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_DBUS_COUNT);		


		/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
							WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SHOW_DBUS_COUNT);*/
		dbus_error_init(&err);
	
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		
		if (NULL == reply) {
			cli_syslog_info("<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				cli_syslog_info("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}
		
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(type[0]));	
	
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(type[1]));	
	
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(type[2]));	
	
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(type[3]));	
	
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(type[4]));	
	
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(type[5]));	

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(type[6]));		
	
		if(ret == WID_DBUS_SUCCESS){
			vty_out(vty,"WID dbus count\n");
			vty_out(vty,"======================================================\n");
			vty_out(vty,"SNMP:	  %d\n",type[0]);
			vty_out(vty,"CLI:	  %d\n",type[1]);
			vty_out(vty,"WEB:	  %d\n",type[2]);
			vty_out(vty,"PORTAL:  %d\n",type[3]);
			vty_out(vty,"TRAP:	  %d\n",type[4]);
			vty_out(vty,"HANSI:   %d\n",type[5]);			
			vty_out(vty,"OTHER:   %d\n",type[6]);
			vty_out(vty,"======================================================\n");
		}else
			vty_out(vty,"<error>  %d\n",ret);
		
		dbus_message_unref(reply);
	}else{
		vty_out(vty,"<error>  unknow command\n");
	}
	return CMD_SUCCESS;
}
DEFUN(show_access_wtp_vendor_count_cmd_func,
		  show_access_wtp_vendor_count_cmd,
		  "show access wtp vendor count [remote] [local] [PARAMETER]",
		  "show access wtp vendor count\n"
		  "show access wtp vendor count\n"
		  "show access wtp vendor count\n"
		  "vendor info\n"
		  "license count\n"
		  "'remote' or 'local' hansi\n"
		  "'remote' or 'local' hansi\n"
		  "slotid-instid\n"
	 )
{
	int ret;
	int state = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter,iter_array;
	DBusError err;
	int index = 0;
	int i = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	int licensetypecount;
	int bind_lic_count = 0;
	int cur_count = 0;
	int max_count = 0;
	int bind_flag = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	int count[5];
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	if((argc == 1)||(argc == 3)){
		vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
		return CMD_SUCCESS;
	}
	if(argc == 2){
		if (!strcmp(argv[0],"remote")){
			localid = 0;
		}else if(!strcmp(argv[0],"local")){
			localid = 1;
		}else{
			vty_out(vty,"parameter should be 'remote' or 'local'\n");
			return CMD_SUCCESS;
		}
		
		if((!strcmp(argv[0],"remote"))&&(!strcmp(argv[1],"local"))){
			vty_out(vty,"<error>input parameter should be 'remote SLOTID-INSTID' or 'local SLOTID-INSTID'\n");
			return CMD_SUCCESS;
		}
		
		ret = parse_slot_hansi_id((char*)argv[1],&slot_id,&profile);
		if(ret != WID_DBUS_SUCCESS){
			slot_id = HostSlotId;
			flag = 1;
			ret = parse_int_ID((char*)argv[1], &profile);
			if(ret != WID_DBUS_SUCCESS){
				if(ret == WID_ILLEGAL_INPUT){
					vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
				}
				else{
					vty_out(vty,"<error> unknown id format\n");
				}
				return CMD_WARNING;
			}	
		}
		if(distributFag == 0){
			if(slot_id != 0){
				vty_out(vty,"<error> slot id should be 0\n");
				return CMD_WARNING;
			}	
		}else if(flag == 1){
			slot_id = HostSlotId;
		}
		if(slot_id >= MAX_SLOT_NUM || slot_id < 0){
			vty_out(vty,"<error> slot id should be 1 to %d\n",MAX_SLOT_NUM-1);
			return CMD_WARNING;
		}	
		if(profile >= MAX_INSTANCE || profile == 0){
			vty_out(vty,"<error> hansi id should be 1 to %d\n",MAX_INSTANCE-1);
			return CMD_WARNING;
		}
		instRun = dcli_hmd_hansi_is_running(vty,slot_id,localid,profile);
		if (INSTANCE_NO_CREATED == instRun) {
			vty_out(vty,"<error> the instance %s %d-%d is not running\n",((localid == 1)?"local-hansi":"hansi"),slot_id,profile);
			return CMD_WARNING;
		}
		
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		if(localid == 0)
			goto hansi_parameter;
		else if(localid == 1)
			goto local_hansi_parameter; 
	}

	if(vty->node != VIEW_NODE){	
		ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE, WID_DBUS_CONF_METHOD_ACCESS_WTP_VENDOR_COUNT_SHOW );
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&state,
								 DBUS_TYPE_INVALID);
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		dbus_message_unref(query);
		if (NULL == reply)
		{
			cli_syslog_info("<error> failed get reply.\n");
			if (dbus_error_is_set(&err))
			{
				cli_syslog_info("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		vty_out(vty,"############################################################################\n");
		/*xiaodawei modify 20101108, add binding flag*/
		vty_out(vty,"%-12s %-18s %-12s %-12s %-8s %-10s\n","LicenseType","CurrentWtpCount","MaxWtpCount","BindingFlag","Shared","assign num");
		if(ret == 0)
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&licensetypecount);
		
			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);
			for (i = 0; i < licensetypecount; i++)
			{
				DBusMessageIter iter_struct;
				dbus_message_iter_recurse(&iter_array,&iter_struct);
		
				dbus_message_iter_get_basic(&iter_struct,&count[0]);
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&count[1]);
				dbus_message_iter_next(&iter_struct);

				dbus_message_iter_get_basic(&iter_struct,&count[2]);

				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&count[3]);

				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&count[4]);
				
				dbus_message_iter_next(&iter_array);	/*xiaodawei add for bindingflag, 20101108*/
				
				vty_out(vty,"%-12d %-18d %-12d %-12d %-8s %-10d\n",i+1,count[0],count[1],count[2],(count[3]==1)?"yes":"no",count[4]);
		
			}
			vty_out(vty,"############################################################################\n");
			vty_out(vty,"%-12s %-18s %-12s\n","BindType","CurrentWtpCount","MaxWtpCount");

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&bind_lic_count);
			vty_out(vty,"bind_lic_count %d \n",bind_lic_count);
			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);
			for (i = 0; i < bind_lic_count; i++)
			{
				DBusMessageIter iter_struct;
				dbus_message_iter_recurse(&iter_array,&iter_struct);
		
				dbus_message_iter_get_basic(&iter_struct,&cur_count);
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&max_count);
				dbus_message_iter_next(&iter_struct);

				dbus_message_iter_get_basic(&iter_struct,&bind_flag);

				dbus_message_iter_next(&iter_array);
				
				vty_out(vty,"%-12d %-18d %-12d\n",bind_flag,cur_count,max_count);
			}
			
		}
		else
		{
			vty_out(vty,"error %d \n",ret);
		}
		vty_out(vty,"############################################################################\n");
		dbus_message_unref(reply);	
	}

	if(vty->node == VIEW_NODE){
		//for remote hansi info
		for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
			ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
			localid = 0;
			
			for (profile = 1; profile < MAX_INSTANCE; profile++) 
			{
				instRun = dcli_hmd_hansi_is_running(vty,slot_id,0,profile);
				if (INSTANCE_NO_CREATED == instRun) {
					continue;
				}

		hansi_parameter:
				ReInitDbusPath_V2(localid,profile,WID_DBUS_BUSNAME,BUSNAME);
				ReInitDbusPath_V2(localid,profile,WID_DBUS_OBJPATH,OBJPATH);
				ReInitDbusPath_V2(localid,profile,WID_DBUS_INTERFACE,INTERFACE);
				query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE, WID_DBUS_CONF_METHOD_ACCESS_WTP_VENDOR_COUNT_SHOW );
				dbus_error_init(&err);
				dbus_message_append_args(query,
										 DBUS_TYPE_UINT32,&state,
										 DBUS_TYPE_INVALID);
				reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
				dbus_message_unref(query);
				if (NULL == reply)
				{
					cli_syslog_info("<error> failed get reply.\n");
					if (dbus_error_is_set(&err))
					{
						cli_syslog_info("%s raised: %s",err.name,err.message);
						dbus_error_free_for_dcli(&err);
					}
					return CMD_SUCCESS;
				}
				dbus_message_iter_init(reply,&iter);
				dbus_message_iter_get_basic(&iter,&ret);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"----------------------------------------------------------------------\n");
				/*xiaodawei modify 20101108, add binding flag*/
				vty_out(vty,"%-12s %-18s %-12s %-12s %-8s %-10s\n","LicenseType","CurrentWtpCount","MaxWtpCount","BindingFlag","Shared","assign num");
				if(ret == 0)
				{
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&licensetypecount);
				
					dbus_message_iter_next(&iter);	
					dbus_message_iter_recurse(&iter,&iter_array);
					for (i = 0; i < licensetypecount; i++)
					{
						DBusMessageIter iter_struct;
						dbus_message_iter_recurse(&iter_array,&iter_struct);
				
						dbus_message_iter_get_basic(&iter_struct,&count[0]);
						dbus_message_iter_next(&iter_struct);
						
						dbus_message_iter_get_basic(&iter_struct,&count[1]);
						dbus_message_iter_next(&iter_struct);

						dbus_message_iter_get_basic(&iter_struct,&count[2]);

						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct,&count[3]);

						dbus_message_iter_next(&iter_struct);
						dbus_message_iter_get_basic(&iter_struct,&count[4]);
						
						dbus_message_iter_next(&iter_array);	/*xiaodawei add for bindingflag, 20101108*/
						
						vty_out(vty,"%-12d %-18d %-12d %-12d %-8s %-10d\n",i+1,count[0],count[1],count[2],(count[3]==1)?"yes":"no",count[4]);
				
					}
					vty_out(vty,"----------------------------------------------------------------------\n");
					vty_out(vty,"%-12s %-18s %-12s\n","BindType","CurrentWtpCount","MaxWtpCount");

					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&bind_lic_count);
					vty_out(vty,"bind_lic_count %d \n",bind_lic_count);
					dbus_message_iter_next(&iter);	
					dbus_message_iter_recurse(&iter,&iter_array);
					for (i = 0; i < bind_lic_count; i++)
					{
						DBusMessageIter iter_struct;
						dbus_message_iter_recurse(&iter_array,&iter_struct);
				
						dbus_message_iter_get_basic(&iter_struct,&cur_count);
						dbus_message_iter_next(&iter_struct);
						
						dbus_message_iter_get_basic(&iter_struct,&max_count);
						dbus_message_iter_next(&iter_struct);

						dbus_message_iter_get_basic(&iter_struct,&bind_flag);

						dbus_message_iter_next(&iter_array);
						
						vty_out(vty,"%-12d %-18d %-12d\n",bind_flag,cur_count,max_count);
					}
					
				}
				else
				{
					vty_out(vty,"error %d \n",ret);
				}
				vty_out(vty,"==============================================================================\n");
				if(argc == 2){
					return CMD_SUCCESS;
				}
				dbus_message_unref(reply);	
				}
			}

	//for local hansi info
	for(slot_id = 1;slot_id < MAX_SLOT_NUM;slot_id++){			
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		localid = 1;
		
		for (profile = 1; profile < MAX_INSTANCE; profile++) 
		{
			instRun = dcli_hmd_hansi_is_running(vty,slot_id,1,profile);
			if (INSTANCE_NO_CREATED == instRun) {
				continue;
			}

	local_hansi_parameter:
			ReInitDbusPath_V2(localid,profile,WID_DBUS_BUSNAME,BUSNAME);
			ReInitDbusPath_V2(localid,profile,WID_DBUS_OBJPATH,OBJPATH);
			ReInitDbusPath_V2(localid,profile,WID_DBUS_INTERFACE,INTERFACE);
			query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE, WID_DBUS_CONF_METHOD_ACCESS_WTP_VENDOR_COUNT_SHOW );
			dbus_error_init(&err);
			dbus_message_append_args(query,
									 DBUS_TYPE_UINT32,&state,
									 DBUS_TYPE_INVALID);
			reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
			dbus_message_unref(query);
			if (NULL == reply)
			{
				cli_syslog_info("<error> failed get reply.\n");
				if (dbus_error_is_set(&err))
				{
					cli_syslog_info("%s raised: %s",err.name,err.message);
					dbus_error_free_for_dcli(&err);
				}
				return CMD_SUCCESS;
			}
			dbus_message_iter_init(reply,&iter);
			dbus_message_iter_get_basic(&iter,&ret);
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
			vty_out(vty,"----------------------------------------------------------------------\n");
			/*xiaodawei modify 20101108, add binding flag*/
			vty_out(vty,"%-12s %-18s %-12s %-12s %-8s %-10s\n","LicenseType","CurrentWtpCount","MaxWtpCount","BindingFlag","Shared","assign num");
			if(ret == 0)
			{
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&licensetypecount);
			
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);
				for (i = 0; i < licensetypecount; i++)
				{
					DBusMessageIter iter_struct;
					dbus_message_iter_recurse(&iter_array,&iter_struct);
			
					dbus_message_iter_get_basic(&iter_struct,&count[0]);
					dbus_message_iter_next(&iter_struct);
					
					dbus_message_iter_get_basic(&iter_struct,&count[1]);
					dbus_message_iter_next(&iter_struct);

					dbus_message_iter_get_basic(&iter_struct,&count[2]);

					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&count[3]);

					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&count[4]);
					
					dbus_message_iter_next(&iter_array);	/*xiaodawei add for bindingflag, 20101108*/
					
					vty_out(vty,"%-12d %-18d %-12d %-12d %-8s %-10d\n",i+1,count[0],count[1],count[2],(count[3]==1)?"yes":"no",count[4]);
			
				}
				vty_out(vty,"----------------------------------------------------------------------\n");
				vty_out(vty,"%-12s %-18s %-12s\n","BindType","CurrentWtpCount","MaxWtpCount");

				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&bind_lic_count);
				vty_out(vty,"bind_lic_count %d \n",bind_lic_count);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);
				for (i = 0; i < bind_lic_count; i++)
				{
					DBusMessageIter iter_struct;
					dbus_message_iter_recurse(&iter_array,&iter_struct);
			
					dbus_message_iter_get_basic(&iter_struct,&cur_count);
					dbus_message_iter_next(&iter_struct);
					
					dbus_message_iter_get_basic(&iter_struct,&max_count);
					dbus_message_iter_next(&iter_struct);

					dbus_message_iter_get_basic(&iter_struct,&bind_flag);

					dbus_message_iter_next(&iter_array);
					
					vty_out(vty,"%-12d %-18d %-12d\n",bind_flag,cur_count,max_count);
				}
				
			}
			else
			{
				vty_out(vty,"error %d \n",ret);
			}
			vty_out(vty,"==============================================================================\n");
			if(argc == 2){
				return CMD_SUCCESS;
			}
			dbus_message_unref(reply);	
			}
		}
	}
	
	return CMD_SUCCESS;				
}
/*xiaodawei add for set license binding 20101029*/
DEFUN(set_license_binding_cmd_func,
		  set_license_binding_cmd,
		  "set license LICENSELIST (binding|nobinding)",
		  "set license\n"
		   "set license type binding\n"
		   "license list(e.g. 1,2,5 means binding license 1 2 5)\n"
	 )
{
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char type;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	char *string = NULL;
	unsigned int strlength = 0;
	int licensetype = 0;
	strlength = strlen(argv[0]);
	string = (char*)malloc(strlength+1);
	if (string == NULL)
	{
		vty_out(vty, "<critical error> alloc memory error!!!\n");
		return CMD_WARNING;
	}
	memset(string, 0, strlength+1);
	memcpy(string, argv[0], strlength);
	if ((!strcmp(argv[1],"binding")))
		type = 1;			
	else if ((!strcmp(argv[1],"nobinding")))
		type = 0;	
	else {		
		vty_out(vty,"<error> parameter illegal!\n");
		free(string);
		string = NULL;
		return CMD_SUCCESS;
	}
	int localid = 1;
	int slot_id = HostSlotId;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE, WID_DBUS_CONF_METHOD_SET_LICENSE_BINDING );
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&type,
							 DBUS_TYPE_STRING,&string,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(ret == 0)
	{
		vty_out(vty, "set license binding successfully!\n");
	}
	else if(ret == WRONG_LICENSE_LIST)
	{
		vty_out(vty, "wrong license type list!\n");
	}
	else if(ret == LICENSE_NUM_LARGER_THAN_MAX)
	{
		vty_out(vty, "exceed the max number of license type!\n");
	}
	else if(ret == LICENSE_NUM_LESS_THAN_TWO)
	{
		vty_out(vty, "the number of license type should be more than one!\n");
	}
	else if(ret == LICENSE_NOT_EXIST)
	{
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&licensetype);
		vty_out(vty, "license type %d doesn't exist!\n", licensetype);
	}
	else if(ret == LICENSE_TYPE_DUPLICATED)
	{
		vty_out(vty, "duplicated license type!\n");
	}
	else if(ret == LICENSE_TYPE_BINDED)
	{
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&licensetype);
		vty_out(vty, "license type %d has been binded!\n", licensetype);
	}
	else if(ret == LICENSE_NO_BINDING_SUCCESS)
	{
		vty_out(vty, "set license nobinding successfully!\n");
	}
	else if(ret == LICENSE_NOT_BINDED)
	{
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&licensetype);
		vty_out(vty, "license type %d has not been binded!\n", licensetype);
	}
	else if(ret == LICENSE_BINDED_ANOTHER)
	{
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&licensetype);
		vty_out(vty, "license type %d wasn't been binded the same!\n", licensetype);
	}
	else if(ret == CURRENT_NUM_MORE_THAN_MAX)
	{
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&licensetype);
		vty_out(vty, "After unbinding license type %d, current num was more than max!\n", licensetype);
		vty_out(vty, "please make sure the current num is less than max before unbinding!!\n");
	}
	else
	{
		vty_out(vty, "error %d \n", ret);
	}

	if(string!=NULL){
		free(string);
		string = NULL;
	}
	dbus_message_unref(reply);	
	return CMD_SUCCESS;				
}
/*###############END#######################*/
DEFUN(set_wids_judge_policy_func,
		  set_wids_judge_policy_cmd,
		  "set wids judge policy VALUE",
		  "wids judge policy control\n"
		  "wids judge policy mac 1\n"
		  "wids judge policy essid 2\n"
		  "wids judge policy oui 4\n"
	 )
{
	int ret = 0;
	int mode = 0;
	int policy = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	mode = atoi(argv[0]);
	
	if (mode < 1 || mode > 7)
	{	
		vty_out(vty,"<error> input patameter should be 1 to 7\n");
		return CMD_SUCCESS;
	}


	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = set_wids_judge_policy(index,localid,mode,policy,dcli_dbus_connection);
	
	if(ret == 0)
	{
		vty_out(vty,"set wids judge policy %s %s successfully\n",argv[0],argv[1]);
	}	
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	return CMD_SUCCESS;			
}

DEFUN(show_wids_judge_policy_func,
		  show_wids_judge_policy_cmd,
		  "show wids judge policy",
		  "wids judge policy control\n"
		  "wids judge policy mac\n"
		  "wids judge policy essid \n"
		  "wids judge policy oui\n"
	 )
{
	int ret = 0;
	int index = 0;
	int mode = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = show_wids_judge_policy(index,localid,&mode,dcli_dbus_connection);
	
	if(ret == 0)
	{
		vty_out(vty,"wids judge policy:");
		if(mode != 0){				
			vty_out(vty,"|");			
			if((mode&0x01) > 0)
				vty_out(vty,"MAC|");			
			if((mode&0x02) > 0)
				vty_out(vty,"ESSID|");
			if((mode&0x04) > 0)
				vty_out(vty,"OUI|");
		}
		else
		{
			vty_out(vty,"no policy\n");
		}
		vty_out(vty,"\n");

	}	
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	return CMD_SUCCESS;			
}
DEFUN(set_wids_monitor_mode_func,
		  set_wids_monitor_mode_cmd,
		  "set wids scanning mode (monitor|halfmonitor|disable)",
		  "wids monitor mode control\n"
		  "monitor mode ,scanning all channel\n"
		  "half monitor ,scanning current or all channel\n"
		  "disable ,dont scanning\n"
	 )
{
	int ret = 0;
	int mode = 0;
	unsigned int wtpid = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	if (!strcmp(argv[0],"monitor"))
	{
		mode = 2;	
	}else if(!strcmp(argv[0],"halfmonitor"))
	{
		mode = 1;	
	}else if(!strcmp(argv[0],"disable"))
	{
		mode = 0;	
	}
	else{
		vty_out(vty,"<error> invalid input.\n");
		return CMD_FAILURE;
	}


	int index = 0;
	if(vty->node == CONFIG_NODE){
		index = 0;
		wtpid = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		wtpid = 0;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		wtpid = 0;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == HANSI_WTP_NODE){
		index = (int)vty->index;
		wtpid = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_WTP_NODE){
		index = (int)vty->index;
		wtpid = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == WTP_NODE){
		index = 0;
		wtpid = (int)vty->index;
	}
	if((wtpid < 0)||(wtpid > (WTP_NUM-1))){
		vty_out(vty,"<error> invalid wtp id.\n");
		return CMD_FAILURE;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = set_wids_monitor_mode(index,localid,wtpid,mode,dcli_dbus_connection);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0)
	{
		vty_out(vty,"set wids scanning mode %s successfully\n",argv[0]);
	}	
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	return CMD_SUCCESS;			
}

DEFUN(set_wids_scanning_mode_func,
		  set_wids_scanning_mode_cmd,
		  "set wids scanning channel mode (auto|all|manual)",
		  "wids scanning mode control\n"
		  "wids scanning mode all\n"
		  "wids scanning mode auto\n"
		  "wids scanning mode manual\n"
	 )
{
	int ret = 0;
	int mode = 0;
	unsigned int wtpid = 0;
	if (!strcmp(argv[0],"auto"))
	{
		mode = 1;	
	}else if(!strcmp(argv[0],"all"))
	{
		mode = 2;	
	}else if(!strcmp(argv[0],"manual"))
	{
		mode = 3;	
	}	
	else{
		vty_out(vty,"<error> invalid input.\n");
		return CMD_FAILURE;
	}

	int index = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	if(vty->node == WTP_NODE){
		index = 0;
		wtpid = (int)vty->index;
	}else if(vty->node == HANSI_WTP_NODE){
		index = (int)vty->index;
		wtpid = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_WTP_NODE){
		index = (int)vty->index;
		wtpid = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == CONFIG_NODE){
		index = 0;
		wtpid = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		wtpid = 0;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		wtpid = 0;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	if(/*(wtpid <= 0)&&*/(wtpid > (WTP_NUM-1))){
		vty_out(vty,"<error> invalid wtp id.\n");
		return CMD_FAILURE;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = set_wids_scanning_mode(index,localid,wtpid,mode,dcli_dbus_connection);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0)
	{
		vty_out(vty,"set wids scanning mode %s successfully\n",argv[0]);
	}
	else
	{
		vty_out(vty,"<error>wids monitor mode is not halfmonitor.\n");
	}
		
	return CMD_SUCCESS;			
}

DEFUN(set_wids_scanning_channellist_func,
	  set_wids_scanning_channellist_cmd,
	  "set wids scanning channellist CHANNELLIST",
	  "set wids scanning channellist\n"
	  "set wids scanning channellist such as 1,2,3\n"
	 )
{
	
		int mode = 0;
		int ret=0;
		int n=0;
		unsigned int num=0;
		int i = 0;
		int j =0;
		int channel = 0;
		int list[SCANNING_CHANNEL_NUM];
		DBusMessage *query, *reply; 
		DBusMessageIter  iter,iter_array;
		DBusError err;	
		dbus_error_init(&err);
	
		/*parse input list*/
		//ret = parse_rate_list((char *)argv[0],&n,list);
		ret = parse_input_list((char *)argv[0],&n,list,SCANNING_CHANNEL_NUM,1,165);
		if (ret != WID_DBUS_SUCCESS)
		{
			vty_out(vty,"<error> input parameter should be <1-165>\n");
			return CMD_SUCCESS;
		}
		/*remove the repeat rate,process the order*/
		num = process_input_list(list,n);/*the num in the input list*/
		for(j=0;j<num;j++){
			channel = list[j];
			if ((channel <= 0)||(channel > 14)) 
			{
				if (!((channel == 149)||(channel == 153)||(channel == 157)||(channel == 161)||(channel == 165)\
					||(channel == 34)||(channel == 36)||(channel == 38)||(channel == 40)||(channel == 42)\
					||(channel == 44)||(channel == 46)||(channel == 48)||(channel == 52)||(channel == 56)\
					||(channel == 58)||(channel == 60)||(channel == 100)||(channel == 104)||(channel == 108)\
					||(channel == 112)||(channel == 116)||(channel == 120)||(channel == 124)||(channel == 128)\
					||(channel == 132)||(channel == 136)||(channel == 140)\
					)) 
				{
					vty_out(vty,"<error> input parameter %s error\n",argv[0]);
					vty_out(vty,"11a receive channel list is:  36 ..;149 153 157 161\n");
					return CMD_SUCCESS;
				}
			}
		}		
		int index = 0;
		unsigned wtpid = 0;
		int localid = 1;
		int slot_id = HostSlotId;
		if(vty->node == CONFIG_NODE){
			index = 0;
			wtpid = 0;
		}else if(vty->node == WTP_NODE){
			index = 0;
			wtpid = (int)vty->index;
		}else if(vty->node == HANSI_NODE){
			index = (int)vty->index;
			wtpid = 0;
			localid = vty->local;
			slot_id = vty->slotindex;
		}else if(vty->node == LOCAL_HANSI_NODE){
			index = (int)vty->index;
			wtpid = 0;
			localid = vty->local;
			slot_id = vty->slotindex;
		}else if(vty->node == HANSI_WTP_NODE){
			index = (int)vty->index;
			wtpid = (int)vty->index_sub;
			localid = vty->local;
			slot_id = vty->slotindex;
		}else if(vty->node == LOCAL_HANSI_WTP_NODE){
			index = (int)vty->index;
			wtpid = (int)vty->index_sub;
			localid = vty->local;
			slot_id = vty->slotindex;
		}
		if(/*(wtpid < 0)||*/(wtpid > (WTP_NUM-1))){
			vty_out(vty,"<error> invalid wtpid.\n");
			return CMD_SUCCESS;
		}
		DBusConnection *dcli_dbus_connection = NULL;
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		ret = set_wids_scanning_channel(index,localid,wtpid,num,list,dcli_dbus_connection);
		if(ret == -1){
			cli_syslog_info("<error> failed get reply.\n");
		}
		else if((ret == COUNTRY_CODE_SUCCESS)||(ret == WID_DBUS_SUCCESS))
		{
			if(num > SCANNING_CHANNEL_NUM)
			{
				vty_out(vty,"<error> an unexpect error\n");
				return CMD_SUCCESS;
			}
			vty_out(vty,"set scanning channel list successfully\n");
			vty_out(vty,"scanning channel num : %d\n",num);
			vty_out(vty,"scanning channel list : ");
			for(i=0;i<num;i++)
			{
				vty_out(vty,"%d ",list[i]);
			}
			vty_out(vty," \n");
		}
		else
		{
			switch(ret)
			{
				case COUNTRY_CHINA_CN : vty_out(vty,"<error> one or more channel is invalid in CHINA\n");
										break;
				case COUNTRY_EUROPE_EU : vty_out(vty,"<error> one or more channel is invalid in EUROPE\n");
										break;
		        case COUNTRY_USA_US : vty_out(vty,"<error> one or more channel is invalid in USA\n");
										break;
				case COUNTRY_JAPAN_JP : vty_out(vty,"<error> one or more channel is invalid in JAPAN\n");
										break;
				case COUNTRY_FRANCE_FR : vty_out(vty,"<error> one or more channel is invalid in FRANCE\n");
										break;
				case COUNTRY_SPAIN_ES : vty_out(vty,"<error> one or more channel is invalid in SPAIN\n");
										break;
//				case COUNTRY_CODE_SUCCESS : break;
				default : vty_out(vty,"<error>  %d\n",ret);
										break;
			}
		}
	
	return CMD_SUCCESS;
	

}

DEFUN(show_wids_scanning_mode_func,
		  show_wids_scanning_mode_cmd,
		  "show wids scanning mode",
		  "show wids scanning mode control\n"
		  "show wids scanning mode all\n"
		  "show wids scanning mode auto\n"
		  "show wids scanning mode manual\n"
	 )
{
	int ret = 0;
	char monitormode[3][12] = {"disable","halfmonitor","monitor"};
	char mode[3][7] = {"auto","all","manual"};
	int monitor_type = 0;
	int mode_type = 0;
	int i = 0;
	int num =0;
	int index = 0;
	unsigned int wtpid = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	if(vty->node == WTP_NODE){
		index = 0;
		wtpid = (int)vty->index;
	}else if(vty->node == HANSI_WTP_NODE){
		index = (int)vty->index;
		wtpid = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_WTP_NODE){
		index = (int)vty->index;
		wtpid = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == CONFIG_NODE){
		index = 0;
		wtpid = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		wtpid = 0;
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		wtpid = 0;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	if(/*(wtpid < 0)||*/(wtpid > (WTP_NUM-1))){
		vty_out(vty,"<error>  %d\n",ret);
		return CMD_FAILURE;
	}
	DCLI_WTP_API_GROUP_ONE *WIDS_INFO = NULL;
	//printf("1111 wtp:%d,index:%d\n",wtpid,index);
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	WIDS_INFO = (typeof(WIDS_INFO))show_wids_scanning_mode_channel(index,localid,wtpid,&ret,dcli_dbus_connection);
	//printf("1111 ret:%d\n",ret);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
		dcli_ac_free_fun_scanning_show(WIDS_INFO);
		return CMD_FAILURE;
	}
	else if(ret == 0)
	{
		monitor_type = (int)WIDS_INFO->WTP[0]->WIDS.monitorMode;
		vty_out(vty,"wids monitor mode %s\n",monitormode[monitor_type]);
		mode_type = (int)WIDS_INFO->WTP[0]->WIDS.scanningMode;
		vty_out(vty,"wids scanning mode %s\n",(mode_type != 0)?mode[mode_type-1]:"none");
		if(monitor_type == 1){//only when monitor mode is halfmonitor ,channel mode can be manual
			if((mode_type == 3)&&(WIDS_INFO->WTP[0]->WIDS.num != 0)){/*mode is manual*/
				vty_out(vty,"scanning channel list : ");
				num = WIDS_INFO->WTP[0]->WIDS.num;
				for(i=0;i<num;i++)
				{
					vty_out(vty,"%d ",WIDS_INFO->WTP[0]->WIDS.channel[i]);
				}
				vty_out(vty," \n");
			}
		}
	}	
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
	dcli_ac_free_fun_scanning_show(WIDS_INFO);
	return CMD_SUCCESS;			
}

DEFUN(set_wtp_link_detect_func,
	  set_wtp_link_detect_cmd,
	  "set wtp link detect (enable|disable)",
	  "set wtp link detect"
	  "link detect"
	  "enable or disable"
	 )
{
	int ret;
	unsigned char type=0;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int index = 0;

	dbus_error_init(&err);
	

	if (!strncmp(argv[0],"enable",strlen(argv[0]))){
		type = 1;
	}
	else if (!strncmp(argv[0],"disable",strlen(argv[0]))){
		type = 0;
	} 
	int localid = 1;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE, WID_DBUS_CONF_METHOD_SET_WTP_LINK_DETECT );

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&type,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		vty_out(vty,"set wtp link detect %s successfully.\n",argv[0]);
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(set_wsm_switch_func,
	  set_wsm_switch_cmd,
	  "set wsm switch (enable|disable)",
	  "set wsm switch"
	  "wsm switch"
	  "enable or disable"
	 )
{
	int ret;
	unsigned int type=0;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int index = 0;

	dbus_error_init(&err);
	

	if (!strncmp(argv[0],"enable",strlen(argv[0]))){
		type = 1;
	}
	else if (!strncmp(argv[0],"disable",strlen(argv[0]))){
		type = 0;
	} 
	int localid = 1;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE, WID_DBUS_CONF_METHOD_SET_WSM_SWITCH);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		vty_out(vty,"set wsm switch %s successfully.\n",argv[0]);
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(set_vlan_switch_func,
	  set_vlan_switch_cmd,
	  "set vlan switch (enable|disable)",
	  "set vlan switch"
	  "vlan switch"
	  "enable or disable"
	 )
{
	int ret;
	unsigned char type=0;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int index = 0;

	dbus_error_init(&err);
	

	if (!strncmp(argv[0],"enable",strlen(argv[0]))){
		type = 1;
	}
	else if (!strncmp(argv[0],"disable",strlen(argv[0]))){
		type = 0;
	} 
	int localid = 1;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE, WID_DBUS_CONF_METHOD_SET_VLAN_SWITCH);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&type,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		vty_out(vty,"set vlan switch %s successfully.\n",argv[0]);
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(set_dhcp_option82_switch_func,
	  set_dhcp_option82_switch_cmd,
	  "set dhcp option82 switch (enable|disable)",
	  "set dhcp option82 switch"
	  "dhcp option82 switch"
	  "enable or disable"
	 )
{
	int ret;
	unsigned char type=0;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int index = 0;

	dbus_error_init(&err);
	

	if (!strncmp(argv[0],"enable",strlen(argv[0]))){
		type = 1;
	}
	else if (!strncmp(argv[0],"disable",strlen(argv[0]))){
		type = 0;
	} 
	int localid = 1;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE, WID_DBUS_CONF_METHOD_SET_DHCP_OPTION82_SWITCH);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&type,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		vty_out(vty,"set dhcp option82 switch %s successfully.\n",argv[0]);
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


#if 0
/*zhaoruijia,20100913,for ap auto update,start*/
DEFUN(service_tftp_state_cmd_func,
	     service_tftp_state_cmd,
	     "service tftp (enable|disable)",
	     CONFIG_STR
	     "Enable or Disable tftpd-hpa service\n"
	  
	  )
{
    
	int ret;
    DBusMessage *query = NULL;
	DBusMessage  *reply = NULL;	
	DBusMessageIter	 iter;
	DBusError err;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	unsigned int ap_auto_update_service_tftp = 0;
	
	if (!strcmp(argv[0],"enable"))
	{
		ap_auto_update_service_tftp = 1;
	}
	else if (!strcmp(argv[0],"disable"))
	{
		
		ap_auto_update_service_tftp = 0;
	}
    else
	{
		vty_out(vty, "Please input enable or disable.\n");
		return CMD_SUCCESS;
	}
	int localid = 1;
   ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
   ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
   ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
   query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE, WID_DBUS_CONF_METHOD_SERVICE_TFTP_SWITCH);

   dbus_error_init(&err);

   dbus_message_append_args(query,
                            DBUS_TYPE_UINT32,&ap_auto_update_service_tftp,
                            DBUS_TYPE_INVALID);

   reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
       
   dbus_message_unref(query);
   if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
    if (ret == WID_DBUS_SUCCESS)
    {
        if(1== ap_auto_update_service_tftp){
          vty_out(vty,"service tftp enable successfull \n");
		}
		else{
          vty_out(vty,"service tftp disable successfull \n");
		}
	}
	else if(ret == WID_DBUS_ERROR)
	{
       vty_out(vty,"<error>  service tftp set failed \n");
       
    }


	dbus_message_unref(reply);
	return CMD_SUCCESS;

}


DEFUN(show_service_tftp_state_cmd_func,
	     show_service_tftp_state_cmd,
		 "show service tftp",
		 CONFIG_STR
		 "show tftpd-hpa service state \n"
	  
	  )
{

  int ret = 0;
  DBusMessage *query = NULL;
  DBusMessage  *reply = NULL;	
  DBusMessageIter	 iter;
  DBusError err;
  int index = 0;
  char BUSNAME[PATH_LEN];
  char OBJPATH[PATH_LEN];
  char INTERFACE[PATH_LEN];
  unsigned int ap_auto_update_service_tftp = 0;
  int localid = 1;
  ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
  ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
  ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
  query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_SERVICE_TFTP_SWITCH);
  dbus_error_init(&err);
  reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
  
  dbus_message_unref(query);
	  
  if (NULL == reply)
  {
		  cli_syslog_info("<error> failed get reply.\n");
		  if (dbus_error_is_set(&err))
		  {
			  cli_syslog_info("%s raised: %s",err.name,err.message);
			  dbus_error_free_for_dcli(&err);
		  }
		  
  
		  return CMD_SUCCESS;
   }

 dbus_message_iter_init(reply,&iter);
 dbus_message_iter_get_basic(&iter,&ap_auto_update_service_tftp);


  vty_out(vty,"==============================================================================\n");
 
   if(1 == ap_auto_update_service_tftp){
       vty_out(vty, "SERVICE TFTP: enable \n");
  }
  else if(0 == ap_auto_update_service_tftp){
       vty_out(vty, "SERVICE TFTP: disable \n");
  }
  else{
       vty_out(vty, "SERVICE TFTP: failed  \n");
  }
  
  vty_out(vty,"==============================================================================\n");
  
  dbus_message_unref(reply);
  return CMD_SUCCESS;
}
/*zhaoruijia,20100913,for ap auto update,end*/
DEFUN(service_ftp_state_cmd_func,
	     service_ftp_state_cmd,
	     "service ftp (enable|disable)",
	     CONFIG_STR
	     "Enable or Disable pure-ftpd service\n"
	  
	  )
{
    
	int ret;
    DBusMessage *query = NULL;
	DBusMessage  *reply = NULL;	
	DBusMessageIter	 iter;
	DBusError err;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	unsigned int service_ftp_state = 0;
	
	if (!strcmp(argv[0],"enable"))
	{
		service_ftp_state = 1;
	}
	else if (!strcmp(argv[0],"disable"))
	{
		
		service_ftp_state = 0;
	}
    else
	{
		vty_out(vty, "Please input enable or disable.\n");
		return CMD_SUCCESS;
	}
	int localid = 1;
   ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
   ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
   ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
   query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE, WID_DBUS_CONF_METHOD_SERVICE_FTP_SWITCH);

   dbus_error_init(&err);

   dbus_message_append_args(query,
                            DBUS_TYPE_UINT32,&service_ftp_state,
                            DBUS_TYPE_INVALID);

   reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
       
   dbus_message_unref(query);
   if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
    if (ret == WID_DBUS_SUCCESS)
    {
        if(1== service_ftp_state){
          vty_out(vty,"service ftp enable successfull \n");
		}
		else{
          vty_out(vty,"service ftp disable successfull \n");
		}
	}
	else if(ret == WID_DBUS_ERROR)
	{
       vty_out(vty,"<error>  service ftp set failed \n");
       
    }


	dbus_message_unref(reply);
	return CMD_SUCCESS;

}


DEFUN(show_service_ftp_state_cmd_func,
	     show_service_ftp_state_cmd,
		 "show service ftp",
		 CONFIG_STR
		 "show pure-ftpd service state \n"
	  
	  )
{

  int ret = 0;
  DBusMessage *query = NULL;
  DBusMessage  *reply = NULL;	
  DBusMessageIter	 iter;
  DBusError err;
  int index = 0;
  char BUSNAME[PATH_LEN];
  char OBJPATH[PATH_LEN];
  char INTERFACE[PATH_LEN];
  unsigned int service_ftp_state = 0;
  int localid = 1;
  ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
  ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
  ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
  query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOW_SERVICE_FTP_SWITCH);
  dbus_error_init(&err);
  reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
  
  dbus_message_unref(query);
	  
  if (NULL == reply)
  {
		  cli_syslog_info("<error> failed get reply.\n");
		  if (dbus_error_is_set(&err))
		  {
			  cli_syslog_info("%s raised: %s",err.name,err.message);
			  dbus_error_free_for_dcli(&err);
		  }
		  
  
		  return CMD_SUCCESS;
   }

 dbus_message_iter_init(reply,&iter);
 dbus_message_iter_get_basic(&iter,&service_ftp_state);


  vty_out(vty,"==============================================================================\n");
 
   if(1 == service_ftp_state){
       vty_out(vty, "SERVICE FTP: enable \n");
  }
  else if(0 == service_ftp_state){
       vty_out(vty, "SERVICE FTP: disable \n");
  }
  else{
       vty_out(vty, "SERVICE FTP: failed  \n");
  }
  
  vty_out(vty,"==============================================================================\n");
  
  dbus_message_unref(reply);
  return CMD_SUCCESS;
}
#endif
DEFUN(set_process_run_cores_func,
	  set_process_run_cores_cmd,
	  "set process NAME run cores ID",
	  "set process run cores"
	  "set process run cores"
	  "process name"
	  "run cores"
	  "core id : 1、2、3、7-10"
	 )
{
	int ret;
	unsigned short core = 0;
	char buf[128] = {0};
	int index = 0;
	
	int localid = 1;
	int slot_id = HostSlotId;
	if(vty->node == CONFIG_NODE){
		  index = 0;
	  }else if(vty->node == HANSI_NODE){
		  index = (int)vty->index;
		  localid = vty->local;
		  slot_id = vty->slotindex;
	 }
	  else if(vty->node == LOCAL_HANSI_NODE){
		  index = (int)vty->index;
		  localid = vty->local;
		  slot_id = vty->slotindex;
	  }
	  DBusConnection *dcli_dbus_connection = NULL;
	  ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = parse_core_id_list((char *)argv[1],&core);
	if(ret != 0){
		vty_out(vty,"core id should be 1-16\n");
		vty_out(vty,"eg:set process wid run cores 1,2,3,10-16\n");
		return CMD_SUCCESS;		
	}
	if(index == 0)
		sprintf(buf,"sudo /usr/bin/mcore_set.sh %s 0x%04x > /dev/null 2>&1",argv[0], core);
	else
		sprintf(buf,"sudo /usr/bin/mcore_set.sh %s 0x%04x %d > /dev/null 2>&1",argv[0], core, index);		
	ret = system(buf);
	return CMD_SUCCESS;
}
/*zhaoruijia,20101103,for wid wsm error handle control,start*/
DEFUN(set_wid_wsm_error_handle_state_func,
          set_wid_wsm_error_handle_state_cmd,
          "set (wid|wsm) error handle (enable|disable)",
          SETT_STR
          "wid module"
          "wsm module"
          "error message"
          "handle the error"
          "Enable the error handle\n"
          "Disable the error handle\n")
{
  int ret = 0;
  DBusMessage *query = NULL;
  DBusMessage  *reply = NULL;	
  DBusMessageIter	 iter;
  DBusError err;
  unsigned int wid_wsm_error_handle_state = 0;
  unsigned int sendToWidOrWsm = 1;//1--send to wid,0--send to wsm
  int index = 0;
  char BUSNAME[PATH_LEN]={0};
  char OBJPATH[PATH_LEN]={0};
  char INTERFACE[PATH_LEN]={0};
  char PATH[PATH_LEN]={0};

  int localid = 1;
  int slot_id = HostSlotId;
  if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

  if (!strcmp(argv[0],"wid"))
		{
			sendToWidOrWsm = 1;
		}
		else if (!strcmp(argv[0],"wsm"))
		{
			
			sendToWidOrWsm = 0;
		}
		else
		{
			vty_out(vty, "Please input wid or wsm.\n");
			return CMD_FAILURE;
		}


  if (!strcmp(argv[1],"enable"))
		{
			wid_wsm_error_handle_state = 1;
		}
		else if (!strcmp(argv[1],"disable"))
		{
			
			wid_wsm_error_handle_state = 0;
		}
		else
		{
			vty_out(vty, "Please input enable or disable.\n");
			return CMD_FAILURE;
		}

       if(sendToWidOrWsm){

		  ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
          ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
          ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
          query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE, WID_DBUS_CONF_METHOD_SET_WID_ERROR_HANDLE_STATE);

	   }
	   else{

		   sprintf(BUSNAME, "%s%d", WSM_DBUS_BUSNAME, index);
		   sprintf(OBJPATH, "%s%d", WSM_DBUS_OBJPATH, index);
		   sprintf(INTERFACE, "%s%d", WSM_DBUS_INTERFACE, index);
		   sprintf(PATH, "%s%d", WSM_DBUS_CONF_METHOD_SET_WSM_ERROR_HANDLE_STATE, index);

           query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,PATH);
	   }
       
	
	   dbus_error_init(&err);
	
	   dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&wid_wsm_error_handle_state,
								DBUS_TYPE_INVALID);
	
	   reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		   
	   dbus_message_unref(query);
	   if (NULL == reply)
		{
			cli_syslog_info("<error> failed get reply.\n");
			if (dbus_error_is_set(&err))
			{
				cli_syslog_info("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
	
			return CMD_SUCCESS;
		}
		
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		if (ret == WID_DBUS_SUCCESS)
		{
			if(1 == sendToWidOrWsm){
				if(1 == wid_wsm_error_handle_state){
                   vty_out(vty,"set wid error handle enable successfull!\n");
				}
				else if(0 == wid_wsm_error_handle_state){
                   vty_out(vty,"set wid error handle disable successfull!\n");
				}

			}
			else if(0 == sendToWidOrWsm){
				if(1 == wid_wsm_error_handle_state){
                   vty_out(vty,"set wsm error handle enable successfull!\n");
				}
				else if(0 == wid_wsm_error_handle_state){
                   vty_out(vty,"set wsm error handle disable successfull!\n");
				}

			}
		}
		else
		{
            vty_out(vty,"<error>Error handle set failed!\n");
		}
	
		dbus_message_unref(reply);
		return CMD_SUCCESS;
		
  
    



}

/*zhaoruijia,20101103,for wid wsm error handle control,end*/
/*wuwl add 2011-03-24 for listen specil interface*/
DEFUN(set_wirelesscontrol_listen_l3_interface_func,
	  set_wirelesscontrol_listen_l3_interface_cmd,
	  "set wireless-control listen interface (add|del) IFNAME",
	  "wireless-control listen interface config\n"
	  "add or delete l3 interface\n"
	  "l3 interface name for wid listen\n"

	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = 0;
	int len = 0;
	int quitreason = 0;
	char *name;
	unsigned char policy = 2;
    
	
	len = strlen(argv[1]);
	
	if(len > 15)
	{		
		vty_out(vty,"<error> interface name is too long,should be no more than 15\n");
		return CMD_SUCCESS;
	}
	
	if (!strcmp(argv[0],"add"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[0],"del"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'add'or 'del'\n");
		return CMD_SUCCESS;
	}	
	
	name = (char*)malloc(strlen(argv[1])+1);
	memset(name, 0, strlen(argv[1])+1);
	memcpy(name, argv[1], strlen(argv[1])); 
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_LISTEN_L3_INTERFACE);
	
	
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&policy,
							 DBUS_TYPE_STRING,&name,
							 DBUS_TYPE_UINT32,&boot_flag,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		free(name);
		name = NULL;
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_get_basic(&iter,&quitreason);
	/*//printf("ret %d quitreason %d\n",ret,quitreason);*/
	if((ret == 0)||(ret == IF_HAS_BEEN_LISTENNING))
	{
		vty_out(vty,"set wireless-control listen downlink interface %s %s successfully\n",argv[0],argv[1]);
	}
	else if(ret == SWITCH_IS_DISABLE)
		vty_out(vty,"<error> auto ap login switch is enable,you should disable it first\n");
	else if(ret == APPLY_IF_FAIL)
		vty_out(vty,"<error> interface %s error, no index or interface down\n",argv[1]);
	else if (ret == INTERFACE_NOT_EXIST)
	{
		vty_out(vty, "<warning> this interface has not been added or has already been deleted.\n");
	}
	else
	{
		if(quitreason == IF_DOWN)
		{
			vty_out(vty,"<error> interface %s is down\n",argv[1]);
		}
		else if(quitreason == IF_NOFLAGS)
		{
			vty_out(vty,"<error> interface %s is no flags\n",argv[1]);
		}
		else if(quitreason == IF_NOINDEX)
		{
			vty_out(vty,"<error> interface %s is no index\n",argv[1]);
		}
		else if (ret == WID_INTERFACE_NOT_BE_LOCAL_BOARD){
			vty_out(vty,"<error> %s is no local interface, permission denial\n",argv[1]);
		}
		else if(ret == IF_BINDING_FLAG){
			vty_out(vty,"<error> %s is other hansi listen\n",argv[1]);
		}
		else
		{
			vty_out(vty,"<error,%d> interface %s had been deleted or never been added\n", ret, argv[1]);
		}
	}
		
	dbus_message_unref(reply);
	free(name);
	name = NULL;
	return CMD_SUCCESS;			
}

/*wuwl add 2012-06-18 for listen specil ip*/
DEFUN(set_wirelesscontrol_listen_ip_func,
	  set_wirelesscontrol_listen_ip_cmd,
	  "set wireless-control listen ip (add|del) IPADDR",
	  "wireless-control listen ip config\n"
	  "add or delete ip\n"
	  "ipaddr for wid listen\n"

	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int ret = 0;
	int quitreason = 0;
	unsigned int ip=0;
	unsigned char policy = 2;
	
	if (!strcmp(argv[0],"add"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[0],"del"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'add'or 'del'\n");
		return CMD_SUCCESS;
	}	
	
	ret = WID_Check_IP_Format((char*)argv[1]);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown ip format\n");
		return CMD_SUCCESS;
	}
	ip = dcli_ip2ulong((char*)argv[1]);
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_WID_LISTEN_IP);
	
	
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&policy,
							 DBUS_TYPE_UINT32,&ip,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_get_basic(&iter,&quitreason);
	/*//printf("ret %d quitreason %d\n",ret,quitreason);*/
	if((ret == 0)||(ret == IF_HAS_BEEN_LISTENNING))
	{
		vty_out(vty,"set wireless-control listen downlink interface %s %d.%d.%d.%d successfully\n",
					argv[0], 
					(ip>>24)&0xFF, (ip>>16)&0xFF, (ip>>8)&0xFF, (ip)&0xFF);
	}
	else if(ret == SWITCH_IS_DISABLE)
		vty_out(vty,"<error> auto ap login switch is enable,you should disable it first\n");
	else if(ret == APPLY_IF_FAIL)
		vty_out(vty,"<error> interface %d.%d.%d.%d error, no index or interface down\n", (ip>>24)&0xFF, (ip>>16)&0xFF, (ip>>8)&0xFF, (ip)&0xFF);
	
	else
	{
		if(quitreason == IF_DOWN)
		{
			vty_out(vty,"<error> interface %d.%d.%d.%d is down\n", (ip>>24)&0xFF, (ip>>16)&0xFF, (ip>>8)&0xFF, (ip)&0xFF);
		}
		else if(quitreason == IF_NOFLAGS)
		{
			vty_out(vty,"<error> interface %d.%d.%d.%d is no flags\n", (ip>>24)&0xFF, (ip>>16)&0xFF, (ip>>8)&0xFF, (ip)&0xFF);
		}
		else if(quitreason == IF_NOINDEX)
		{
			vty_out(vty,"<error> interface %d.%d.%d.%d is no index\n", (ip>>24)&0xFF, (ip>>16)&0xFF, (ip>>8)&0xFF, (ip)&0xFF);
		}
		else if (ret == WID_INTERFACE_NOT_BE_LOCAL_BOARD){
			vty_out(vty,"<error> %d.%d.%d.%d is no local interface, permission denial\n", (ip>>24)&0xFF, (ip>>16)&0xFF, (ip>>8)&0xFF, (ip)&0xFF);
		}
		else if(ret == MORE_THAN_ONE_IF)
			vty_out(vty,"more if have this ip:%d.%d.%d.%d.",(ip>>24)&0xFF,(ip>>16)&0xFF,(ip>>8)&0xFF,(ip)&0xFF);
		else if(ret == NO_IF_HAS_THIS_IP){
			vty_out(vty,"<warning>no interface has this ip:%d.%d.%d.%d, will listen it when it exsit.",(ip>>24)&0xFF,(ip>>16)&0xFF,(ip>>8)&0xFF,(ip)&0xFF);
		}
		else if (ret == INTERFACE_NOT_EXIST)
		{
			vty_out(vty, "<warning> this ip has not been added or has already been deleted.\n");
		}
		else
		{
			vty_out(vty,"<error, %d>  %s %d.%d.%d.%d error which had been deleted or never been added\n", 
						ret, 
						argv[0], 
						(ip>>24)&0xFF, (ip>>16)&0xFF, (ip>>8)&0xFF, (ip)&0xFF);
		}
	}
		
	dbus_message_unref(reply);
	return CMD_SUCCESS;			
}

DEFUN(show_wireless_listen_if_cmd_func,
	     show_wireless_listen_if_cmd,
		 "show wireless-control listenning interface",
		 CONFIG_STR
		 "show wireless listenning interface list\n"
	  
	  )
{

	int ret = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	if(vty->node == CONFIG_NODE){
	  index = 0;
	}else if(vty->node == HANSI_NODE){
	  index = (int)vty->index;
	  localid = vty->local;
	  slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	Listen_IF *Listen_IF = NULL;
	struct Listenning_IF *TMP = NULL;
	Listen_IF = (typeof(Listen_IF))dcli_ac_show_wid_listen_if(localid,index,&ret,dcli_dbus_connection);
	vty_out(vty,"==============================================================================\n");
	if(ret == -1){
		vty_out(vty, "<error> failed get reply.\n");
		return CMD_WARNING;
	}
	if(Listen_IF->count == 0){
	  vty_out(vty, "wid listenning interface:NULL\n");
	}
	TMP = Listen_IF->interface;
	vty_out(vty, "wid listenning interface count:%d\n",Listen_IF->count);
	while(TMP != NULL)
	{
		if ((TMP->lic_flag & DOWN_LINK_IF_TYPE) == DOWN_LINK_IF_TYPE)
			vty_out(vty, 
					"wid listenning interface:%s, ip:%d.%d.%d.%d, type:%s, [IF]\n",
					TMP->ifname,
					(TMP->ipaddr >> 24)&0xFF,(TMP->ipaddr >> 16)&0xFF,(TMP->ipaddr >> 8)&0xFF,(TMP->ipaddr)&0xFF,
					/*(TMP->lic_flag == LIC_TYPE)?"LIC SYN":*/"DOWNLINK");
		else if ((TMP->lic_flag & DOWN_LINK_IP_TYPE) == DOWN_LINK_IP_TYPE)
			vty_out(vty, 
					"wid listenning interface:%s, ip:%d.%d.%d.%d, type:%s, [IP]\n",
					TMP->ifname,
					(TMP->ipaddr >> 24)&0xFF,(TMP->ipaddr >> 16)&0xFF,(TMP->ipaddr >> 8)&0xFF,(TMP->ipaddr)&0xFF,
					/*(TMP->lic_flag == LIC_TYPE)?"LIC SYN":*/"DOWNLINK");
		else if ((TMP->lic_flag & LIC_TYPE) == LIC_TYPE)
		{
			vty_out(vty, 
					"wid listenning interface:%s, ip:%d.%d.%d.%d, type:%s, [AC-STATE]\n",
					TMP->ifname,
					(TMP->ipaddr >> 24)&0xFF,(TMP->ipaddr >> 16)&0xFF,(TMP->ipaddr >> 8)&0xFF,(TMP->ipaddr)&0xFF,
					/*(TMP->lic_flag == LIC_TYPE)?*/"LIC SYN"/*:"DOWNLINK"*/);
		}
		else
		{
			vty_out(vty, "other interface flag:%x, [up to now(2012-11-5)]\n", TMP->lic_flag);
		}
		TMP = TMP->if_next;
	}
	vty_out(vty,"==============================================================================\n");
	dcli_ac_free_listen_if_node(Listen_IF);

	return CMD_SUCCESS;
}
/*xiaodawei add for iperf, 20110224*/
DEFUN(iperf_wtpip_cmd_func,
	  iperf_wtpip_cmd,
	  "iperf WTPIP",
	  "iperf"
	  "ip address like 100.1.1.10"
	 )
{	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	char *wtp_ip = NULL;
	double jitter = 0;
	double datagramloss = 0;

	ret = WID_Check_IP_Format((char*)argv[0]);	//check ip 
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty, "<error> unknown ip format\n");
		return CMD_SUCCESS;
	}

	wtp_ip = (char*)malloc(strlen(argv[0])+1);
	memset(wtp_ip, 0, strlen(argv[0])+1);
	memcpy(wtp_ip, argv[0], strlen(argv[0]));
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_IPERF_WTPIP);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_STRING,&wtp_ip,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&jitter);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&datagramloss);;

		vty_out(vty,"jitter\t\t%.3f ms\n",jitter);
		vty_out(vty,"datagramloss\t%.1f\%\n",datagramloss);
		
	}				
	else
	{
		vty_out(vty,"No Response!Please check out ip address or maybe ap doesn't support iperf!!\n");
	}
		
	dbus_message_unref(reply);
	if(wtp_ip){
		free(wtp_ip);
		wtp_ip = NULL;
	}

	return CMD_SUCCESS;	

}
/*iperf END*/

DEFUN(set_multicast_listen_cmd_func,
	  set_multicast_listen_cmd,
	  "set wireless multicast listen (enable|disable)",
	  "wireless setting"
	  "multicast listen setting"
	 )
{	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int state = 0;
	if (!strcmp(argv[0],"enable")||(tolower(argv[0][0]) == 'e')){
		state = 1;
	}else if(!strcmp(argv[0],"disable")||(tolower(argv[0][0]) == 'd')){
		state = 0;
	}else{
		vty_out(vty,"unknown parameter %s\n",argv[0]);
		return CMD_WARNING;
	}
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_MULTICAST_LISTEN_SETTING);
	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"set wireless multicast listen %s successfully\n",argv[0]);
	}
	else
	{
		vty_out(vty,"error num %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return CMD_SUCCESS;	

}
/*fengwenchao add for AXSSZFI-680 20120228*/
DEFUN(delete_slotid_img_cmd_func,
	  delete_slotid_img_cmd,
	  "delete slotid SLOTID IMG",
	  "delete ap img"
	  "slotid"
	  "slotid"
	  "AP IMG"
	 )
{
	DBusMessage *query = NULL, *reply = NULL;	
	DBusMessageIter	 iter;
	DBusError err;
	int profile = 0;
	int localid = 0;
	int ret = 0;
	unsigned int slot_id_target = 0;
	unsigned int slot_id = HostSlotId;
	char *ap_img_name = NULL;
	if (strlen(argv[0] > 255))
	{
		vty_out(vty, "<error> image name is too long\n");
		return 0;
	}
	slot_id_target = strtoul((char *)argv[0],NULL,10);
	if((slot_id_target < 1)||slot_id_target > MAX_SLOT){
        vty_out(vty,"%% Bad parameter : %s !",argv[0]);
		return CMD_WARNING;
	}

	ap_img_name = (char *)malloc(256);	
	if(NULL == ap_img_name){
	   return CMD_WARNING;
	}
	memset(ap_img_name,0,256);
	memcpy(ap_img_name,argv[1],strlen(argv[1]));

	  DBusConnection *dcli_dbus_connection = NULL;
	  ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);	

	query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_DELETE_SLOTID_IMG);
																	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&slot_id_target,
							 DBUS_TYPE_STRING,&ap_img_name,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);	

	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"delete slotid %s img %s successfully\n",argv[0],argv[1]);
	}
	else if(ret == HMD_DBUS_SLOT_ID_NOT_EXIST)
	{
		vty_out(vty,"input slotid %s is not exist\n",argv[0]);
	}
	else if(ret == SYSTEM_CMD_ERROR)
	{
		vty_out(vty,"there are some system error\n");
	}
	else if (ret == HMD_DBUS_ERROR)
	{
		vty_out(vty,"input img %s is not exist\n",argv[1]);
	}
	else if(ret == HMD_DBUS_WARNNING)
	{
		vty_out(vty,"<warning>input slotid %s is not local broad,can not ensure successful,please check it on that broad\n",argv[0]);
	}
	else
	{
		vty_out(vty,"error %d\n",ret);
	}
		
	dbus_message_unref(reply);
	free(ap_img_name);
	ap_img_name = NULL;
	return CMD_SUCCESS;		
}
/*fengwenchao add end*/
/*fengwenchao add 20120117 for onlinebug-96*/
DEFUN( set_ac_master_bak_correct_wtp_state_timer_func,
		  set_ac_master_bak_correct_wtp_state_timer_cmd,
		  "set ac master_bak correct wtp state timer INTERVAL",
		  "set ac master_bak correct wtp state timer\n"
		  "set ac master_bak correct wtp state timer\n"
		  "set ac master_bak correct wtp state timer\n"
		  "set ac master_bak correct wtp state timer\n"
		  "set ac master_bak correct wtp state timer\n"
		  "set ac master_bak correct wtp state timer\n"
		  "set ac master_bak correct wtp state timer\n"
		  "Interval value: valid range [0-900]\n"
	 )
{
	int ret = 0;

	DBusMessage *query = NULL, *reply = NULL;	
	DBusMessageIter	 iter;
	DBusError err;
	int index = 0; 	
	int interval = 0;
	ret = parse_int_ID((char*)argv[0], &interval);
	if(ret != WID_DBUS_SUCCESS){
			vty_out(vty,"<error> unknown id format\n");
			return CMD_SUCCESS;
	}	
	if(interval > 900 || interval < 0){
		vty_out(vty,"<error> timer should be 0 to 900\n");
		return CMD_SUCCESS;
	}	
	int localid = 1;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SET_AC_MASTER_BAK_CORRECT_WTP_STATE_TIMER);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&interval,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);

	if (NULL == reply)
	{
		vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);		

	if(ret == 0)
		vty_out(vty,"set ac master_bak correct wtp state timer %s successfully\n",argv[0]);
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	return CMD_SUCCESS;		
}
/*fengwenchao add end*/

DEFUN( set_ac_master_ipaddr_func,
		  set_ac_master_ipaddr_cmd,
		  "set ac state (master|bakup|disable) addr IPADDR",
		  "set ac state master or bak\n"
		  "set ac state master or bak\n"
		  "set ac master addr\n"
		  "IPADDR: ipaddr of master ac.\n"
	 )
{
	int ret = 0;
	DBusMessage *query = NULL, *reply = NULL;	
	DBusMessageIter	 iter;
	DBusError err;
	int index = 0; 	
	int interval = 0;
	char type = 0;
	unsigned int ip = 0;
	if(!strcmp("master",(char*)argv[0])){
		type = 1;
	}else if(!strcmp("bakup",(char*)argv[0])){
		type = 2;
	}else if(!strcmp("disable",(char*)argv[0])){
		type = 3;
	}else{
		vty_out(vty,"<error> invalid input,input should be master or bakup.");
		return CMD_WARNING;
	}

	ret = WID_Check_IP_Format((char*)argv[1]);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown ip format\n");
		return CMD_SUCCESS;
	}
	ip = dcli_ip2ulong((char*)argv[1]);
	int localid = 1;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WTP_METHOD_SET_AC_ACTIVE_BAK_STATE);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&type,
							 DBUS_TYPE_UINT32,&ip,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);

	if (NULL == reply)
	{
		vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);		

	if(ret == 0)
		vty_out(vty,"set ac state %s addr %d.%d.%d.%d successfully\n",argv[0],(ip>>24)&0xFF,(ip>>16)&0xFF,(ip>>8)&0xFF,(ip)&0xFF);
	else if(ret == MORE_THAN_ONE_IF)
		vty_out(vty,"more if have this ip:%d.%d.%d.%d.",(ip>>24)&0xFF,(ip>>16)&0xFF,(ip>>8)&0xFF,(ip)&0xFF);
	else if(ret == NO_IF_HAS_THIS_IP){
		vty_out(vty,"<warning>no if has this ip:%d.%d.%d.%d,will listen it when it exsit.",(ip>>24)&0xFF,(ip>>16)&0xFF,(ip>>8)&0xFF,(ip)&0xFF);
	}else if(ret == INVALID_TYPE){
		vty_out(vty,"<err>please disable first.");
	}
	else if (ret == INTERFACE_NOT_EXIST)
	{
		vty_out(vty, "<warning> no interface binding this ip.\n");
	}
	else if (ret == AC_STATE_IP_NOT_EXIST)
	{
		vty_out(vty, "<warning> this ip has not been added or has already been deleted.\n");
	}
	else if (ret == AC_STATE_FIRST_DISABLE)
	{
		vty_out(vty, "<error> please disable it first.\n");
	}
	else
		vty_out(vty,"<error>  unknown error %d.\n", ret);
	
#ifdef __nouseif__
	if(ifname){
		free(ifname);
		ifname = NULL;
	}
#endif

	return CMD_SUCCESS;		
}


/* 
  *****************************************************************************
  *
  * NOTES:	set bakup router's synchronization interval time, unit: second
  * INPUT:	 
  * OUTPUT:	 
  * return:	 
  *  
  * author: 		Huang Leilei
  * begin time:	2012-10-22 12:00 
  * finish time:		2012-10-24 19:00 
  * history:
  *
  *****************************************************************************
  */
DEFUN(set_bak_check_interval_func,
		  set_bak_check_interval_cmd,
		  "set bak_check_interval VALUE",
		  "bak_check_interval config\n"
		  "set backup requests master-infomation's interval-time\n"
		  "interval <60-86400>\n"
		  )
{
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int interval = 0;
	int index = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	ret = parse_int_ID((char*)argv[0], &interval);
	
	if(ret != WID_DBUS_SUCCESS)
	{
		    vty_out(vty,"<error> unknown id format\n");
			return CMD_SUCCESS;
	}	
	if ((interval < 60) || (interval > 86400)) {
		vty_out(vty,"<error> input interval should be 60 to 86400\n");
		return CMD_WARNING;
	}
	
	if(vty->node == HANSI_NODE){
		index = (int)(vty->index);
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)(vty->index);
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);	

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_BAK_CHECK_INTERVAL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_INT32,&interval,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply)
	{
		vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(ret == 0)
	{
		vty_out(vty, "set bak_check_interval interval %s seconds successfully\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;			
}

/* 
  *****************************************************************************
  *
  * NOTES:	 show bakup router's synchronization interval time,  unit: second
  * INPUT:	 
  * OUTPUT:	 
  * return:	 
  *  
  * author: 		Huang Leilei
  * begin time:	2012-10-24 9:00 
  * finish time:		2012-10-24 11:00 
  * history:		
  *
  *****************************************************************************
  */
DEFUN(show_bak_check_interval_cmd_func,
	     show_bak_check_interval_cmd,
		 "show bak_check_interval",
		 CONFIG_STR
		 "show backup requests master-infomation's interval-time\n"
	  )
{

	int ret = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	
	if(vty->node == CONFIG_NODE){
	  index = 0;
	}else if(vty->node == HANSI_NODE){
	  index = (int)(vty->index);
	  localid = vty->local;
	  slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)(vty->index);
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	int bak_check_interval = 0;
	
	bak_check_interval = dcli_ac_show_bak_check_interval(localid, index, &ret, dcli_dbus_connection);
	
	if (ret == -1)
	{
		vty_out(vty, "<error> faild get bak_check_interval.\n");
	}
	else
	{
		vty_out(vty, "BakCheckInterval's value is: %d seconds\n", bak_check_interval);
	}

	return CMD_SUCCESS;
}

/* 
  *****************************************************************************
  *
  * NOTES:	set backup router's lincense info sysnchronizationg interval time, unit: second
  * INPUT:	 
  * OUTPUT:	 
  * return:	 
  *  
  * author: 		Huang Leilei
  * begin time:	2012-10-22 12:00 
  * finish time:		2012-10-24 19:00 
  * history:
  *
  *****************************************************************************
  */
DEFUN(set_lic_bak_req_interval_func,
		  set_lic_bak_req_interval_cmd,
		  "set lic_bak_req_interval VALUE",
		  "lic_bak_req_interval config\n"
		  "set backup requests license-synchronization's interval-time\n"
		  "interval <60-86400>\n"
	 )
{
	int ret;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	int interval = 0;
	int index = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	ret = parse_int_ID((char*)argv[0], &interval);
	
	if(ret != WID_DBUS_SUCCESS)
	{
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if ((interval < 60) || (interval > 86400)) {
		vty_out(vty,"<error> input interval should be 60 to 86400\n");
		return CMD_WARNING;
	}
	if(vty->node == HANSI_NODE){
		index = (int)(vty->index);
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)(vty->index);
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);	

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_LIC_BAK_REQ_INTERVAL);
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_INT32,&interval,
							 DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if (NULL == reply)
	{
		vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(ret == 0)
	{
		vty_out(vty, "set lic_bak_req_interval %s seconds successfully\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;			
}

/* 
  *****************************************************************************
  *
  * NOTES:	 show bakup router's lincense info synchronization interval time,  unit: second
  * INPUT:	 
  * OUTPUT:	 
  * return:	 
  *  
  * author: 		Huang Leilei
  * begin time:	2012-10-24 9:00 
  * finish time:		2012-10-24 11:00 
  * history:		
  *
  ****************************************************************************
  */
DEFUN(show_lic_bak_req_interval_cmd_func,
	     show_lic_bak_req_interval_cmd,
		 "show lic_bak_req_interval",
		 CONFIG_STR
		 "show backup requests license-synchronization's interval-time\n"
	  )
{

	int ret = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	
	if(vty->node == CONFIG_NODE){
	  index = 0;
	}else if(vty->node == HANSI_NODE){
	  index = (int)(vty->index);
	  localid = vty->local;
	  slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)(vty->index);
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	int lic_bak_req_interval = 0;
	
	lic_bak_req_interval = dcli_ac_show_lic_bak_req_interval(localid, index, &ret, dcli_dbus_connection);
	
	if (ret == -1)
	{
		vty_out(vty, "<error> faild get bak_check_interval.\n");
	}
	else
	{
		vty_out(vty, "LicBakReqInterval's value is: %d seconds\n", lic_bak_req_interval);
	}

	return CMD_SUCCESS;
}
/*qiuchen add the next 3 cmds for Henan Mobile Logsystem
***asd_wid log group:	Logs for Henan Mobile will be printed when hn_mobile enable
					Logs for Autelan logsystem will be printed when mobile enable
***ac management ip: It is used in Henan Mobile logsystem when sta roaming happened
***show ac managemetn ip:To show the management ip address that is configured
*/
DEFUN(set_asd_new_log_group_cmd_func,
		set_asd_new_log_group_cmd,
		"set sta_ap syslog format (hn_mobile|mobile) (enable|disable)",
		"set asd & wid log format\n"
		"log format: henan or mobile\n"
		"enable\n"
		"disable\n"
)
{
	int ret;

	DBusMessage *query, *reply;
	DBusMessage *query2, *reply2;
	DBusMessageIter  iter;
	DBusError err;

	unsigned char group;
	unsigned char switchi;
	

	if (!strcmp(argv[0],"hn_mobile"))
	{
		group = 0x01;	
	}
	else if (!strcmp(argv[0],"mobile"))
	{
		group = 0x02;	
	}
	else
	{
		vty_out(vty,"<error> input patameter should only be hn_mobile|mobile\n");
		return CMD_SUCCESS;
	}
	if (!strcmp(argv[1],"enable"))
	{
		switchi = 1; 
	}
	else if (!strcmp(argv[1],"disable"))
	{
		switchi = 0; 
	}
	else
	{
		vty_out(vty,"<error> input patameter should only be 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_ASD_LOG_GROUP_ACTIVATED);
	
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&group,
							 DBUS_TYPE_BYTE,&switchi,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"asd successfully set log group %s %s\n",argv[0],argv[1]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	/*Set gWIDLOGHN*/

	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

	query2 = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_HNLOG_SWITCH_ACTIVATED);
	dbus_error_init(&err);


	dbus_message_append_args(query2,
							 DBUS_TYPE_BYTE,&group,
							 DBUS_TYPE_BYTE,&switchi,
							 DBUS_TYPE_INVALID);

	reply2 = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query2,-1, &err);
	
	dbus_message_unref(query2);
	
	if (NULL == reply2)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply2,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"wid successfully set log group %s %s\n",argv[0],argv[1]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply2);
	
	return CMD_SUCCESS; 		
}
DEFUN(set_ac_management_ipaddr_cmd_func,
		set_ac_management_ipaddr_cmd,
		"set ac_management ip_address IPADDR",
		""
		""
		""
	)
{
	int ret;

	DBusMessage *query, *reply;
	DBusMessage *query2,*reply2;
	DBusMessageIter  iter;
	DBusError err;

	unsigned long man_ip = 0;
    man_ip = inet_addr((unsigned char*)argv[0]);

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SET_AC_MANAGEMENT_IP);

	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&man_ip,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		

		return CMD_SUCCESS;
	}

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"set asd ac management ip address %s success!\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
	dbus_message_unref(reply);

	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

	query2 = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SET_AC_MANAGEMENT_IP);
	dbus_error_init(&err);


	dbus_message_append_args(query2,
							 DBUS_TYPE_UINT32,&man_ip,
							 DBUS_TYPE_INVALID);

	reply2 = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query2,-1, &err);
	
	dbus_message_unref(query2);
	
	if (NULL == reply2)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply2,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"set wid ac management ip %s success!\n",argv[0]);
	}				
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply2);

	return CMD_SUCCESS;
}
DEFUN(show_ac_management_ip_addr_cmd_func,
		show_ac_management_ip_addr_cmd,
		"show ac_mangement ip address",
		""
		""
	)
	{
		int ret;
	
		DBusMessage *query, *reply;
		DBusMessageIter  iter;
		DBusError err;
	
		unsigned long man_ip = 0;
	
		int localid = 1;
		int slot_id = HostSlotId;
		int index = 0;
		char BUSNAME[PATH_LEN];
		char OBJPATH[PATH_LEN];
		char INTERFACE[PATH_LEN];
		if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
			index = 0;
		}else if(vty->node == HANSI_NODE){
			index = (int)vty->index;
			localid = vty->local;
			slot_id = vty->slotindex;
		}
		else if(vty->node == LOCAL_HANSI_NODE){
			index = (int)vty->index;
			localid = vty->local;
			slot_id = vty->slotindex;
		}
		DBusConnection *dcli_dbus_connection = NULL;
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	
		ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SHOW_AC_MANAGEMENT_IP);
	
		dbus_error_init(&err);
	
	
		dbus_message_append_args(query,
								 DBUS_TYPE_INVALID);
	
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
		dbus_message_unref(query);
	
		if (NULL == reply)
		{
			cli_syslog_info("<error> failed get reply.\n");
			if (dbus_error_is_set(&err))
			{
				cli_syslog_info("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			
	
			return CMD_SUCCESS;
		}
	
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
	
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&man_ip);
		if(ret == 0)
		{
			vty_out(vty," ac management ip address %lu.%lu.%lu.%lu \n",((man_ip & 0xff000000) >> 24),((man_ip & 0xff0000) >> 16),	\
									((man_ip & 0xff00) >> 8),(man_ip & 0xff));
		}				
		else
		{
			vty_out(vty,"<error>  %d\n",ret);
		}
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}
	
/* For new format of mobile syslog 2013-07-29 */	
DEFUN(set_log_statistics_interval_cmd_func,
		set_log_statistics_interval_cmd,
		"set log statistics interval (ap_stat|tunnel_stat|radio_stat|roam_stat|all) INTERVAL",
		"log statistics interval configuration\n"
		"log statistics interval\n"
		"interval\n"
		"interval\n"
)
{
	int ret;

	DBusMessage *query, *reply;
	DBusMessage *query2, *reply2;
	DBusMessageIter  iter;
	DBusError err;

	unsigned char type;
	unsigned int interval;
	

	if (!strcmp(argv[0],"ap_stat"))
	{
		type = AP_STATISTICS;	
	}
	else if (!strcmp(argv[0],"tunnel_stat"))
	{
		type = TUNNEL_STATISTICS;	
	}
	else if (!strcmp(argv[0],"radio_stat"))
	{
		type = RADIO_STATISTICS;	
	}
	else if (!strcmp(argv[0],"roam_stat"))
	{
		type = ROAM_STATISTICS;	
	}
	else if (!strcmp(argv[0],"all"))
	{
		type = ALL_STATISTICS;	
	}
	else
	{
		vty_out(vty,"<error> input patameter should only be hn_mobile|mobile\n");
		return CMD_SUCCESS;
	}
	
	ret = parse_int_ID(argv[1],&interval);
	if(ret != WID_DBUS_SUCCESS){
        if(ret == WID_ILLEGAL_INPUT){
         	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
        }
		else{
			vty_out(vty,"<error> unknown id format\n");
		}
		return CMD_SUCCESS;
	}
	ret = check_logX_interval(3600,interval,2);/*检查interval是不是900s的2的整数次幂倍*/
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> illegal input:the interval should be 900*2^N(s)(N=0,1,2,...) \n");
	}

	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	
	/*Set Ap_STAT or Roam_STAT or both of the parameter in asd*/
	if(type == AP_STATISTICS || type == RADIO_STATISTICS || type == ALL_STATISTICS){
		DBusConnection *dcli_dbus_connection = NULL;
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		
		ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_CONFIG_METHOD_SET_LOG_STATISTICS_INTERVAL);
		
		dbus_error_init(&err);


		dbus_message_append_args(query,
								 DBUS_TYPE_BYTE,&type,
								 DBUS_TYPE_UINT32,&interval,
								 DBUS_TYPE_INVALID);

		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		
		if (NULL == reply)
		{
			cli_syslog_info("<error> failed get reply.\n");
			if (dbus_error_is_set(&err))
			{
				cli_syslog_info("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
			}
			

			return CMD_SUCCESS;
		}
		
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);

		if(ret == 0)
		{
			vty_out(vty,"set log statistics interval %s %s successfully\n",argv[0],argv[1]);
		}				
		else
		{
			vty_out(vty,"<error>  %d\n",ret);
		}
			
		dbus_message_unref(reply);
	}

	/*Set Tunnle_STAT or Radio_STAT or both of the parameter in wid*/
	else if(type == TUNNEL_STATISTICS || type == RADIO_STATISTICS || type == ALL_STATISTICS){
		ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);

		query2 = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONFIG_METHOD_SET_LOG_STATISTICS_INTERVAL);
		dbus_error_init(&err);


		dbus_message_append_args(query2,
	  							 DBUS_TYPE_BYTE,&type,
	  							 DBUS_TYPE_UINT32,&interval,
								 DBUS_TYPE_INVALID);

		reply2 = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query2,-1, &err);
		
		dbus_message_unref(query2);
		
		if (NULL == reply2)
		{
			cli_syslog_info("<error> failed get reply.\n");
			if (dbus_error_is_set(&err))
			{
				cli_syslog_info("%s raised: %s",err.name,err.message);
				dbus_error_free(&err);
			}
			

			return CMD_SUCCESS;
		}
		
		dbus_message_iter_init(reply2,&iter);
		dbus_message_iter_get_basic(&iter,&ret);

		if(ret == 0)
		{
			vty_out(vty,"set log statistics interval %s %s successfully\n",argv[0],argv[1]);
		}				
		else
		{
			vty_out(vty,"<error>  %d\n",ret);
		}
			
		dbus_message_unref(reply2);
	}
	return CMD_SUCCESS; 		
}
/*chenjun12.23*/
DEFUN(show_is_secondary_cmd_func,
		  show_is_secondary_cmd,
	 	  "show is_secondary",
		  "show is secondary\n"
		  "show is  secondary\n"
	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter  iter;
	DBusError err;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int ret = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	int index = 0;
	
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_IS_SECONDARY_SHOW);
	dbus_error_init(&err);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);

	if (reply == NULL) 
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s", err.name, err.message);
			dbus_error_free_for_dcli(&err);
		}
		
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply, &iter);
	dbus_message_iter_get_basic(&iter, &ret);
	
	if(ret == 0){
		vty_out(vty,"show is secondary master \n");
	}
	else if(ret == 1){	
		vty_out(vty,"show is secondary bak\n");
	}
	else  if(ret == 2){
		vty_out(vty,"show is secondary disable \n");
	}
	else
	        vty_out(vty,"<error>  %d\n",ret);

	
	return CMD_SUCCESS;	

	
}
/*chenjun12.23*/
DEFUN(set_is_secondary_func,
	  set_is_secondary_cmd,
	  "set is_secondary (master |bak |disable)",
	  "set is secondary"
	  "set ac secondary state"
	  "master or bak or disable "
	 )
{
	int ret;
	unsigned char type=0;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int index = 0;

	dbus_error_init(&err);
	
	if (!strncmp(argv[0],"master",strlen(argv[0]))){
		type = 0;
	}
	else if (!strncmp(argv[0],"bak",strlen(argv[0]))){
		type = 1;
	} 
	else if (!strncmp(argv[0],"disable",strlen(argv[0]))){
		type = 2;
	} 
	else 
	{
		vty_out(vty, "Please input master or bak or disable.\n");
		return CMD_FAILURE;
	}
	
	int localid = 1;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = (int)vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE, WID_DBUS_CONF_METHOD_SET_IS_SECONDARY);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&type,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply)
	{
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err))
		{
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
		vty_out(vty,"set is secondary %s successfully.\n",argv[0]);
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int check_logX_interval(unsigned int base,unsigned int interval,int ebase)
{
	/*Fuction:check if interval == base*ebase^N (N=0,1,2,...)*/
	int ret = 0;
	double a = 0,x = 0,x1 = 0;
	if(interval < base)
		ret = 1;
	a = (double)interval/base;
	x = log(a)/log(ebase);
	x1 = x;
	if(x1-(int)x)
		ret = 1;
	printf("a = %f,x = %f,x1-(int)x = %f\n",a,x,x1-(int)x);
	return ret;
}

void dcli_ac_init(void)
{
	install_element(VIEW_NODE,&show_model_list_cmd);													/*a26*/
	install_element(VIEW_NODE,&show_access_wtp_vendor_count_cmd);										/*a24*/
	install_element(VIEW_NODE,&show_auto_ap_config_cmd);												/*a14*/
	install_element(VIEW_NODE,&show_vrrp_state_cmd);													/*a31*/
	install_element(VIEW_NODE,&show_wid_trap_switch_info_cmd);											/*a33*/
	install_element(VIEW_NODE,&show_wid_config_cmd);													/*a1*/
	install_element(VIEW_NODE,&show_ap_rrm_config_cmd);
	install_element(VIEW_NODE,&show_ap_txpower_control_cmd);
	install_element(VIEW_NODE,&show_ap_statistics_list_bywtpid_cmd);
	install_element(VIEW_NODE,&show_rogue_ap_trap_threshold_cmd);
	install_element(VIEW_NODE,&show_neighbor_ap_list_bywtpid_cmd);
	install_element(VIEW_NODE,&show_neighbor_rssi_info_bywtpid_cmd);
#if 0
	/*-----------------------------------VIEW_NODE-----------------------------------*/
	install_element(VIEW_NODE,&show_wid_config_cmd);													/*a1*/
	install_element(VIEW_NODE,&show_rogue_ap_list_cmd);													/*a2*/
	install_element(VIEW_NODE,&show_rogue_ap_trap_threshold_cmd);										/*a3*/
	install_element(VIEW_NODE,&show_rogue_ap_list_v1_cmd);												/*a4*/
	install_element(VIEW_NODE,&show_rogue_ap_list_bywtpid_cmd);											/*a5*/
	install_element(VIEW_NODE,&show_neighbor_ap_list_bywtpid_cmd);										/*a6*/
	install_element(VIEW_NODE,&show_wirelesscontrol_whitelist_cmd);										/*a7*/
	install_element(VIEW_NODE,&show_wirelesscontrol_blacklist_cmd);										/*a8*/
	install_element(VIEW_NODE,&show_wids_device_list_cmd);												/*a9*/
	install_element(VIEW_NODE,&show_wids_device_list_bywtpid_cmd);										/*a10*/
	
	install_element(VIEW_NODE,&show_wids_statistics_list_cmd);											/*a11*/
	install_element(VIEW_NODE,&show_wids_statistics_list_bywtpid_cmd);									/*a12*/
	install_element(VIEW_NODE,&show_neighbor_rssi_info_bywtpid_cmd);									/*a13*/
	install_element(VIEW_NODE,&show_auto_ap_config_cmd);												/*a14*/
	install_element(VIEW_NODE,&show_ap_statistics_list_bywtpid_cmd);									/*a15*/
	install_element(VIEW_NODE,&show_ap_ip_bywtpid_cmd);													/*a16*/
	install_element(VIEW_NODE,&show_ap_model_infomation_cmd);											/*a17*/
	install_element(VIEW_NODE,&show_ap_txpower_control_cmd);											/*a18*/
	install_element(VIEW_NODE,&show_receiver_signal_level_cmd);											/*a19*/
	install_element(VIEW_NODE,&show_sample_info_cmd);													/*a20*/
	
	install_element(VIEW_NODE,&show_manufacturer_oui_list_cmd);											/*a21*/
	install_element(VIEW_NODE,&show_legal_essid_list_cmd);												/*a22*/
	install_element(VIEW_NODE,&show_attack_mac_list_cmd);												/*a23*/
	install_element(VIEW_NODE,&show_access_wtp_vendor_count_cmd);										/*a24*/
	install_element(VIEW_NODE,&show_model_cmd);															/*a25*/
	install_element(VIEW_NODE,&show_model_list_cmd);													/*a26*/
	install_element(VIEW_NODE,&show_ap_model_code_cmd);													/*a27*/
	install_element(VIEW_NODE,&show_ap_threshold_cmd);													/*a28*/
	install_element(VIEW_NODE,&show_ap_rrm_config_cmd);													/*a29*/
	install_element(VIEW_NODE,&show_neighbordead_interval_cmd);											/*a30*/
	
	install_element(VIEW_NODE,&show_vrrp_state_cmd);													/*a31*/
	//install_element(VIEW_NODE,&show_vrrp_sock_cmd);			/*fengwenchao comment 20110418*/											/*a32*/
	install_element(VIEW_NODE,&show_wid_trap_switch_info_cmd);											/*a33*/
	install_element(VIEW_NODE,&show_ap_update_img_timer_cmd);											/*a34*/
	install_element(VIEW_NODE,&show_ap_update_fail_count_cmd);											/*a35*/
	install_element(VIEW_NODE,&show_ap_network_bywtpid_cmd);											/*a36*/
	install_element(VIEW_NODE,&show_wids_mac_list_cmd);													/*a37*/
	install_element(VIEW_NODE,&show_ac_balance_cmd);/*wuwl add 09/12/03*/								/*a38*/
	//install_element(VIEW_NODE,&set_license_binding_cmd);/*xiaodawei add, 20101111*/
	/*-----------------------------------ENABLE_NODE-----------------------------------*/
	install_element(ENABLE_NODE,&show_wid_config_cmd);													/*a1*/
	install_element(ENABLE_NODE,&show_rogue_ap_list_cmd);												/*a2*/
	install_element(ENABLE_NODE,&show_rogue_ap_trap_threshold_cmd);										/*a3*/
	install_element(ENABLE_NODE,&show_rogue_ap_list_v1_cmd);											/*a4*/
	install_element(ENABLE_NODE,&show_rogue_ap_list_bywtpid_cmd);										/*a5*/
	install_element(ENABLE_NODE,&show_neighbor_ap_list_bywtpid_cmd);									/*a6*/
	install_element(ENABLE_NODE,&show_wirelesscontrol_whitelist_cmd);									/*a7*/
	install_element(ENABLE_NODE,&show_wirelesscontrol_blacklist_cmd);									/*a8*/
	install_element(ENABLE_NODE,&show_wids_device_list_cmd);											/*a9*/
	install_element(ENABLE_NODE,&show_wids_device_list_bywtpid_cmd);									/*a10*/
	
	install_element(ENABLE_NODE,&show_wids_statistics_list_cmd);										/*a11*/
	install_element(ENABLE_NODE,&show_wids_statistics_list_bywtpid_cmd);								/*a12*/
	install_element(ENABLE_NODE,&show_neighbor_rssi_info_bywtpid_cmd);									/*a13*/
	install_element(ENABLE_NODE,&show_auto_ap_config_cmd);												/*a14*/
	install_element(ENABLE_NODE,&show_ap_statistics_list_bywtpid_cmd);									/*a15*/
	install_element(ENABLE_NODE,&show_ap_ip_bywtpid_cmd);												/*a16*/
	install_element(ENABLE_NODE,&show_ap_model_infomation_cmd);											/*a17*/
	install_element(ENABLE_NODE,&show_ap_txpower_control_cmd);											/*a18*/
	install_element(ENABLE_NODE,&show_receiver_signal_level_cmd);										/*a19*/
	install_element(ENABLE_NODE,&show_sample_info_cmd);													/*a20*/
	
	install_element(ENABLE_NODE,&show_manufacturer_oui_list_cmd);										/*a21*/
	install_element(ENABLE_NODE,&show_legal_essid_list_cmd);											/*a22*/
	install_element(ENABLE_NODE,&show_attack_mac_list_cmd);												/*a23*/
	install_element(ENABLE_NODE,&show_access_wtp_vendor_count_cmd);										/*a24*/
	install_element(ENABLE_NODE,&show_model_cmd);														/*a25*/
	install_element(ENABLE_NODE,&show_model_list_cmd);													/*a26*/
	install_element(ENABLE_NODE,&show_ap_model_code_cmd);												/*a27*/
	install_element(ENABLE_NODE,&show_ap_threshold_cmd);												/*a28*/
	install_element(ENABLE_NODE,&show_ap_rrm_config_cmd);												/*a29*/
	install_element(ENABLE_NODE,&show_neighbordead_interval_cmd);										/*a30*/
	
	install_element(ENABLE_NODE,&show_vrrp_state_cmd);													/*a31*/
	//install_element(ENABLE_NODE,&show_vrrp_sock_cmd);			/*fengwenchao comment 20110418*/										/*a32*/
	install_element(ENABLE_NODE,&show_wid_trap_switch_info_cmd);										/*a33*/
	install_element(ENABLE_NODE,&show_ap_update_img_timer_cmd);											/*a34*/
	install_element(ENABLE_NODE,&show_ap_update_fail_count_cmd);										/*a35*/
	install_element(ENABLE_NODE,&show_ap_network_bywtpid_cmd);											/*a36*/
	install_element(ENABLE_NODE,&show_wids_mac_list_cmd);												/*a37*/
	install_element(ENABLE_NODE,&show_ac_balance_cmd);/*wuwl add 09/12/03*/								/*a38*/
	//install_element(ENABLE_NODE,&set_license_binding_cmd);/*xiaodawei add, 20101111*/
	install_element(ENABLE_NODE,&set_process_run_cores_cmd);	
	/*
	install_element(ENABLE_NODE,&set_wid_hw_version_cmd);	
	install_element(ENABLE_NODE,&set_wid_sw_version_cmd);	
	install_element(ENABLE_NODE,&set_wid_lev3_protocol_cmd);
	install_element(ENABLE_NODE,&set_wid_auth_security_cmd);
	install_element(ENABLE_NODE,&set_wid_name_cmd);
	*/

/*	install_element(ENABLE_NODE,&set_wirelesscontrol_max_wtp_cmd);*/
	install_element(ENABLE_NODE,&set_wid_max_mtu_cmd);
	install_element(ENABLE_NODE,&set_wid_log_switch_cmd);
	install_element(ENABLE_NODE,&set_wid_log_size_cmd);
	install_element(ENABLE_NODE,&set_wid_log_level_cmd);
	install_element(ENABLE_NODE,&set_wid_daemonlog_debug_open_cmd);
	install_element(ENABLE_NODE,&dynamic_channel_selection_cmd);
	install_element(ENABLE_NODE,&set_asd_daemonlog_debug_open_cmd);
	install_element(ENABLE_NODE,&set_hostapd_logger_printflag_open_cmd);
	install_element(ENABLE_NODE,&show_dbus_count_cmd);	
	install_element(ENABLE_NODE,&iperf_wtpip_cmd);		//xiaodawei add, 20110303
	
	/*-----------------------------------CONFIG_NODE-----------------------------------*/
	install_element(CONFIG_NODE,&set_process_run_cores_cmd);	
	install_element(CONFIG_NODE,&show_wid_config_cmd);													/*a1*/
	install_element(CONFIG_NODE,&show_rogue_ap_list_cmd);												/*a2*/
	install_element(CONFIG_NODE,&show_rogue_ap_trap_threshold_cmd);										/*a3*/
	install_element(CONFIG_NODE,&show_rogue_ap_list_v1_cmd);											/*a4*/
	install_element(CONFIG_NODE,&show_rogue_ap_list_bywtpid_cmd);										/*a5*/
	install_element(CONFIG_NODE,&show_neighbor_ap_list_bywtpid_cmd);									/*a6*/
	install_element(CONFIG_NODE,&show_wirelesscontrol_whitelist_cmd);									/*a7*/
	install_element(CONFIG_NODE,&show_wirelesscontrol_blacklist_cmd);									/*a8*/
	install_element(CONFIG_NODE,&show_wids_device_list_cmd);											/*a9*/
	install_element(CONFIG_NODE,&show_wids_device_list_bywtpid_cmd);									/*a10*/
	
	install_element(CONFIG_NODE,&show_wids_statistics_list_cmd);										/*a11*/
	install_element(CONFIG_NODE,&show_wids_statistics_list_bywtpid_cmd);								/*a12*/
	install_element(CONFIG_NODE,&show_neighbor_rssi_info_bywtpid_cmd);									/*a13*/
	install_element(CONFIG_NODE,&show_auto_ap_config_cmd);												/*a14*/
	install_element(CONFIG_NODE,&show_ap_statistics_list_bywtpid_cmd);									/*a15*/
	install_element(CONFIG_NODE,&show_ap_ip_bywtpid_cmd);												/*a16*/
	install_element(CONFIG_NODE,&show_ap_model_infomation_cmd);											/*a17*/
	install_element(CONFIG_NODE,&show_ap_txpower_control_cmd);											/*a18*/
	install_element(CONFIG_NODE,&show_receiver_signal_level_cmd);										/*a19*/
	install_element(CONFIG_NODE,&show_sample_info_cmd);													/*a20*/
	
	install_element(CONFIG_NODE,&show_manufacturer_oui_list_cmd);										/*a21*/
	install_element(CONFIG_NODE,&show_legal_essid_list_cmd);											/*a22*/
	install_element(CONFIG_NODE,&show_attack_mac_list_cmd);												/*a23*/
	install_element(CONFIG_NODE,&show_access_wtp_vendor_count_cmd);										/*a24*/
	install_element(CONFIG_NODE,&show_model_cmd);														/*a25*/
	install_element(CONFIG_NODE,&show_model_list_cmd);													/*a26*/
	install_element(CONFIG_NODE,&show_ap_model_code_cmd);												/*a27*/
	install_element(CONFIG_NODE,&show_ap_threshold_cmd);												/*a28*/
	install_element(CONFIG_NODE,&show_ap_rrm_config_cmd);												/*a29*/
	install_element(CONFIG_NODE,&show_neighbordead_interval_cmd);										/*a30*/
	
	install_element(CONFIG_NODE,&show_vrrp_state_cmd);													/*a31*/
	//install_element(CONFIG_NODE,&show_vrrp_sock_cmd);			/*fengwenchao comment 20110418*/										/*a32*/
	install_element(CONFIG_NODE,&show_wid_trap_switch_info_cmd);										/*a33*/
	install_element(CONFIG_NODE,&show_ap_update_img_timer_cmd);											/*a34*/
	install_element(CONFIG_NODE,&show_ap_update_fail_count_cmd);										/*a35*/
	install_element(CONFIG_NODE,&show_ap_network_bywtpid_cmd);											/*a36*/
	install_element(CONFIG_NODE,&show_wids_mac_list_cmd);												/*a37*/
	install_element(CONFIG_NODE,&show_ac_balance_cmd);/*wuwl add 09/12/03*/								/*a38*/
	//install_element(CONFIG_NODE,&set_license_binding_cmd);/*xiaodawei add, 20101111*/
	/*
	install_element(CONFIG_NODE,&set_wid_hw_version_cmd);	
	install_element(CONFIG_NODE,&set_wid_sw_version_cmd);
	install_element(CONFIG_NODE,&set_wid_lev3_protocol_cmd);
	install_element(CONFIG_NODE,&set_wid_auth_security_cmd);
	install_element(CONFIG_NODE,&set_wid_name_cmd);
	*/
	install_element(CONFIG_NODE,&set_ap_scanning_cmd);
	install_element(CONFIG_NODE,&set_ap_scanning_report_interval_cmd);
	/*install_element(CONFIG_NODE,&update_ap_scanning_info_cmd);*/
	install_element(CONFIG_NODE,&set_wirelesscontrol_whitelist_cmd);
	install_element(CONFIG_NODE,&set_wirelesscontrol_blacklist_cmd);
	install_element(CONFIG_NODE,&delete_wirelesscontrol_whitelist_cmd);
	install_element(CONFIG_NODE,&delete_wirelesscontrol_blacklist_cmd);
	install_element(CONFIG_NODE,&clear_wids_device_list_cmd);
	install_element(CONFIG_NODE,&clear_wids_device_list_bywtpid_cmd);
	install_element(CONFIG_NODE,&clear_wids_statistics_list_cmd);
	install_element(CONFIG_NODE,&clear_wids_statistics_list_bywtpid_cmd);
	install_element(CONFIG_NODE,&show_wids_device_of_all_cmd);   //fengwenchao add 20101227

	/*install_element(CONFIG_NODE,&show_radio_resource_managment_cmd);
	
//	install_element(CONFIG_NODE,&set_wirelesscontrol_max_wtp_cmd);*/
	install_element(CONFIG_NODE,&set_wid_max_mtu_cmd);
	install_element(CONFIG_NODE,&set_wid_log_switch_cmd);
	install_element(CONFIG_NODE,&set_wid_log_size_cmd);	
	install_element(CONFIG_NODE,&set_wid_log_level_cmd);
	install_element(CONFIG_NODE,&set_wid_daemonlog_debug_open_cmd);
	install_element(CONFIG_NODE,&dynamic_channel_selection_cmd);	
	install_element(CONFIG_NODE,&dynamic_channel_selection_range_cmd);
	install_element(CONFIG_NODE,&transmit_power_control_cmd);
	install_element(CONFIG_NODE,&set_txpower_threshold_cmd);
	install_element(CONFIG_NODE,&set_rogue_ap_trap_threshold_cmd);
	install_element(CONFIG_NODE,&set_coverage_threshold_cmd);
	install_element(CONFIG_NODE,&set_transmit_power_control_scope_cmd);
	/*country code area*/
	install_element(CONFIG_NODE,&set_system_country_code_cmd);
	install_element(CONFIG_NODE,&undo_system_country_code_cmd);
	install_element(CONFIG_NODE,&set_asd_daemonlog_debug_open_cmd);
	install_element(CONFIG_NODE,&set_hostapd_logger_printflag_open_cmd);
	install_element(CONFIG_NODE,&set_asd_daemonlog_level_cmd);
	/*auto ap area*/
	install_element(CONFIG_NODE,&set_wirelesscontrol_auto_ap_switch_cmd);
	install_element(CONFIG_NODE,&set_wirelesscontrol_auto_ap_binding_l3_interface_cmd);
	install_element(CONFIG_NODE,&set_wirelesscontrol_auto_ap_binding_l3_interface_new_cmd);
	install_element(CONFIG_NODE,&set_wirelesscontrol_listen_l3_interface_cmd);
	install_element(CONFIG_NODE,&set_wirelesscontrol_auto_ap_binding_wlan_cmd);
	install_element(CONFIG_NODE,&del_wirelesscontrol_auto_ap_binding_wlan_cmd);
	install_element(CONFIG_NODE,&set_wirelesscontrol_auto_ap_save_config_switch_cmd);
	install_element(CONFIG_NODE,&clear_auto_ap_config_cmd);
	install_element(CONFIG_NODE,&set_ap_data_exclude_multicast_cmd);
	install_element(CONFIG_NODE,&set_ap_statistics_cmd);
	install_element(CONFIG_NODE,&set_ap_statistics_inter_cmd);
	install_element(WTP_NODE,&set_ap_statistics_inter_cmd);
	install_element(CONFIG_NODE,&set_ap_timestamp_cmd);
	
	install_element(CONFIG_NODE,&set_receiver_signal_level_cmd);
	install_element(CONFIG_NODE,&set_monitor_time_cmd);
	install_element(CONFIG_NODE,&set_sample_time_cmd);
	install_element(CONFIG_NODE,&set_monitor_enable_cmd);
	install_element(CONFIG_NODE,&set_sample_enable_cmd);
	

	install_element(CONFIG_NODE,&add_legal_manufacturer_cmd);/*xm add*/
	install_element(CONFIG_NODE,&add_legal_essid_cmd);
	install_element(CONFIG_NODE,&modify_legal_essid_cmd);
	install_element(CONFIG_NODE,&add_attack_ap_mac_cmd);
	
	install_element(CONFIG_NODE,&set_ac_all_ap_extension_information_enable_cmd);/*nl add*/
	
	install_element(CONFIG_NODE,&del_legal_manufacturer_cmd);
	install_element(CONFIG_NODE,&del_legal_essid_cmd);
	install_element(CONFIG_NODE,&del_attack_ap_mac_cmd);
	install_element(CONFIG_NODE,&wireless_interface_vmac_state_cmd);
	install_element(CONFIG_NODE,&set_wtp_link_detect_cmd);
	install_element(CONFIG_NODE,&set_wsm_switch_cmd);

#if 0
// Moved to dcli_wsm.c

	/*ipfwd*/
	install_element(CONFIG_NODE,&set_ipfwd_cmd);
	install_element(CONFIG_NODE,&show_ipfwd_cmd); /*luoxun add.*/
#endif

	install_element(CONFIG_NODE,&set_model_cmd);
	install_element(CONFIG_NODE,&set_ap_cm_threshold_cmd);
	install_element(CONFIG_NODE,&set_neighbordead_interval_cmd);
	install_element(CONFIG_NODE,&update_bak_ac_config_cmd);
	install_element(CONFIG_NODE,&synchronize_wsm_table_cmd);
	install_element(CONFIG_NODE,&synchronize_asd_table_cmd);
	install_element(CONFIG_NODE,&notice_vrrp_state_cmd);
	install_element(CONFIG_NODE,&set_wtp_wids_interval_cmd);
	install_element(CONFIG_NODE,&set_wtp_wids_threshold_cmd);
	install_element(CONFIG_NODE,&set_wtp_wids_lasttime_cmd);

	install_element(CONFIG_NODE,&batch_config_cmd);	
	install_element(CONFIG_NODE,&set_wid_trap_open_cmd);
	install_element(CONFIG_NODE,&set_wid_trap_switch_able_cmd);
	install_element(CONFIG_NODE,&update_wtpcompatible_cmd);
	install_element(CONFIG_NODE,&set_ap_update_fail_count_cmd);
	install_element(CONFIG_NODE,&set_wid_watch_dog_cmd);
	install_element(CONFIG_NODE,&set_ap_update_img_timer_cmd);  //fengwenchao add 20110216

	install_element(CONFIG_NODE,&ac_load_balance_cmd);/*wuwl add 09/12/03*/
	install_element(CONFIG_NODE,&ac_balance_parameter_cmd);/*ht add 10.3.5*/
	install_element(CONFIG_NODE,&set_ap_hotreboot_cmd);
	install_element(CONFIG_NODE,&set_ap_access_through_nat_cmd);
	install_element(CONFIG_NODE,&set_wtp_wids_policy_cmd);
	install_element(CONFIG_NODE,&add_wids_mac_cmd);
	install_element(CONFIG_NODE,&del_wids_mac_cmd);
	install_element(CONFIG_NODE,&set_ap_countermeasures_cmd);
	install_element(CONFIG_NODE,&set_ap_countermeasures_mode_cmd);		
	install_element(CONFIG_NODE,&set_wid_mac_whitelist_cmd);
	install_element(CONFIG_NODE,&set_wid_essid_whitelist_cmd);	
	install_element(CONFIG_NODE,&change_wirelesscontrol_blacklist_cmd);
	install_element(CONFIG_NODE,&change_wirelesscontrol_whitelist_cmd);	
    install_element(CONFIG_NODE,&service_tftp_state_cmd);
	install_element(CONFIG_NODE,&show_service_tftp_state_cmd);
	install_element(CONFIG_NODE,&show_neighbor_ap_list_bywtpid2_cmd);
	
	install_element(CONFIG_NODE,&service_ftp_state_cmd);
	install_element(CONFIG_NODE,&show_service_ftp_state_cmd);
	install_element(CONFIG_NODE,&set_wid_wsm_error_handle_state_cmd);
	
	install_element(CONFIG_NODE,&show_dbus_count_cmd);
	install_element(CONFIG_NODE,&set_dbus_count_cmd);
	install_element(CONFIG_NODE,&show_neighbor_ap_list_cmd);       //fengwenchao add 20101216
	install_element(CONFIG_NODE,&iperf_wtpip_cmd);		//xiaodawei add, 20110303
	install_element(CONFIG_NODE,&set_wids_judge_policy_cmd);
	install_element(CONFIG_NODE,&show_wids_judge_policy_cmd);
	install_element(CONFIG_NODE,&set_wids_monitor_mode_cmd);
	install_element(WTP_NODE,&set_wids_monitor_mode_cmd);
	install_element(CONFIG_NODE,&set_wids_scanning_mode_cmd);
	install_element(WTP_NODE,&set_wids_scanning_mode_cmd);
	install_element(CONFIG_NODE,&set_wids_scanning_channellist_cmd);
	install_element(WTP_NODE,&set_wids_scanning_channellist_cmd);
	install_element(CONFIG_NODE,&show_wids_scanning_mode_cmd);
	install_element(WTP_NODE,&show_wids_scanning_mode_cmd);

	install_element(CONFIG_NODE,&show_wireless_listen_if_cmd);
#endif
	install_element(CONFIG_NODE,&batch_config_cmd);	

/***************************************************************************/
	/*
	install_element(ENABLE_NODE,&set_wid_hw_version_cmd);	
	install_element(ENABLE_NODE,&set_wid_sw_version_cmd);	
	install_element(ENABLE_NODE,&set_wid_lev3_protocol_cmd);
	install_element(ENABLE_NODE,&set_wid_auth_security_cmd);
	install_element(ENABLE_NODE,&set_wid_name_cmd);
	*/
/*	install_element(ENABLE_NODE,&set_wirelesscontrol_max_wtp_cmd);*/

	/*-----------------------------------HANSI_NODE-----------------------------------*/
	install_element(HANSI_NODE,&show_wid_config_cmd);													/*a1*/
	install_element(HANSI_NODE,&show_rogue_ap_list_cmd);												/*a2*/
	install_element(HANSI_NODE,&show_rogue_ap_trap_threshold_cmd);										/*a3*/
	install_element(HANSI_NODE,&show_rogue_ap_list_v1_cmd);												/*a4*/
	install_element(HANSI_NODE,&show_rogue_ap_list_bywtpid_cmd);										/*a5*/
	install_element(HANSI_NODE,&show_neighbor_ap_list_bywtpid_cmd);										/*a6*/
	install_element(HANSI_NODE,&show_wirelesscontrol_whitelist_cmd);									/*a7*/
	install_element(HANSI_NODE,&show_wirelesscontrol_blacklist_cmd);									/*a8*/
	install_element(HANSI_NODE,&show_wids_device_list_cmd);												/*a9*/
	install_element(HANSI_NODE,&show_wids_device_list_bywtpid_cmd);										/*a10*/

	install_element(HANSI_NODE,&show_wids_statistics_list_cmd);											/*a11*/
	install_element(HANSI_NODE,&show_wids_statistics_list_bywtpid_cmd);									/*a12*/
	install_element(HANSI_NODE,&show_neighbor_rssi_info_bywtpid_cmd);									/*a13*/
	install_element(HANSI_NODE,&show_auto_ap_config_cmd);												/*a14*/
	install_element(HANSI_NODE,&show_ap_statistics_list_bywtpid_cmd);									/*a15*/
	install_element(HANSI_NODE,&show_ap_ip_bywtpid_cmd);												/*a16*/
	install_element(HANSI_NODE,&show_ap_model_infomation_cmd);											/*a17*/
	install_element(HANSI_NODE,&show_ap_txpower_control_cmd);											/*a18*/
	install_element(HANSI_NODE,&show_receiver_signal_level_cmd);										/*a19*/
	install_element(HANSI_NODE,&show_sample_info_cmd);													/*a20*/

	install_element(HANSI_NODE,&show_manufacturer_oui_list_cmd);										/*a21*/
	install_element(HANSI_NODE,&show_legal_essid_list_cmd);												/*a22*/
	install_element(HANSI_NODE,&show_attack_mac_list_cmd);												/*a23*/
	install_element(HANSI_NODE,&show_model_cmd);														/*a24*/
	install_element(HANSI_NODE,&show_access_wtp_vendor_count_cmd);										/*a25*/
	install_element(HANSI_NODE,&show_model_list_cmd);													/*a26*/
	install_element(HANSI_NODE,&show_ap_model_code_cmd);												/*a27*/
	install_element(HANSI_NODE,&show_ap_threshold_cmd);													/*a28*/
	install_element(HANSI_NODE,&show_ap_rrm_config_cmd);												/*a29*/
	install_element(HANSI_NODE,&show_neighbordead_interval_cmd);										/*a30*/

	install_element(HANSI_NODE,&show_vrrp_state_cmd);													/*a31*/
	//install_element(HANSI_NODE,&show_vrrp_sock_cmd);			/*fengwenchao comment 20110418*/										/*a32*/
	install_element(HANSI_NODE,&show_ap_update_img_timer_cmd);											/*a33*/
	install_element(HANSI_NODE,&show_ap_update_fail_count_cmd);											/*a34*/
	install_element(HANSI_NODE,&show_wid_trap_switch_info_cmd);											/*a35*/
	//install_element(HANSI_NODE,&set_license_binding_cmd);/*xiaodawei add, 20101111*/
	/*
	install_element(CONFIG_NODE,&set_wid_hw_version_cmd);	
	install_element(CONFIG_NODE,&set_wid_sw_version_cmd);
	install_element(CONFIG_NODE,&set_wid_lev3_protocol_cmd);
	install_element(CONFIG_NODE,&set_wid_auth_security_cmd);
	install_element(CONFIG_NODE,&set_wid_name_cmd);
	*/
	install_element(HANSI_NODE,&set_ap_scanning_cmd);
	install_element(HANSI_NODE,&set_ap_scanning_report_interval_cmd);
	/*install_element(CONFIG_NODE,&update_ap_scanning_info_cmd);*/
	install_element(HANSI_NODE,&set_wirelesscontrol_whitelist_cmd);
	install_element(HANSI_NODE,&set_wirelesscontrol_blacklist_cmd);
	install_element(HANSI_NODE,&delete_wirelesscontrol_whitelist_cmd);
	install_element(HANSI_NODE,&delete_wirelesscontrol_blacklist_cmd);
	install_element(HANSI_NODE,&clear_wids_device_list_cmd);
	install_element(HANSI_NODE,&clear_wids_device_list_bywtpid_cmd);
	install_element(HANSI_NODE,&clear_wids_statistics_list_cmd);
	install_element(HANSI_NODE,&clear_wids_statistics_list_bywtpid_cmd);
	/*install_element(CONFIG_NODE,&show_radio_resource_managment_cmd);
//	install_element(CONFIG_NODE,&set_wirelesscontrol_max_wtp_cmd);*/
	install_element(HANSI_NODE, &set_wirelesscontrol_max_wtp_cmd);
	install_element(HANSI_NODE,&set_wid_max_mtu_cmd);
	install_element(HANSI_NODE,&set_wid_log_switch_cmd);
	install_element(HANSI_NODE,&set_wid_log_size_cmd); 
	install_element(HANSI_NODE,&set_wid_log_level_cmd);
	install_element(HANSI_NODE,&set_wid_daemonlog_debug_open_cmd);
	install_element(HANSI_NODE,&dynamic_channel_selection_cmd);			
	install_element(HANSI_NODE,&dynamic_channel_selection_range_cmd);
	install_element(HANSI_NODE,&transmit_power_control_cmd);
	install_element(HANSI_NODE,&set_txpower_threshold_cmd);
	install_element(HANSI_NODE,&set_rogue_ap_trap_threshold_cmd);
	install_element(HANSI_NODE,&set_coverage_threshold_cmd);
	install_element(HANSI_NODE,&set_transmit_power_control_scope_cmd);
	/*country code area*/
	install_element(HANSI_NODE,&set_system_country_code_cmd);
	install_element(HANSI_NODE,&undo_system_country_code_cmd);
	install_element(HANSI_NODE,&set_asd_daemonlog_debug_open_cmd);
	install_element(HANSI_NODE,&set_hostapd_logger_printflag_open_cmd);
	install_element(HANSI_NODE,&set_asd_daemonlog_level_cmd);
	/*auto ap area*/
	install_element(HANSI_NODE,&set_wirelesscontrol_auto_ap_switch_cmd);
	install_element(HANSI_NODE,&set_wireless_memory_trace_switch_cmd);
	install_element(HANSI_NODE,&set_wirelesscontrol_auto_ap_binding_l3_interface_cmd);
	install_element(HANSI_NODE,&set_wirelesscontrol_auto_ap_binding_l3_interface_new_cmd);
	install_element(HANSI_NODE,&set_wirelesscontrol_listen_l3_interface_cmd);
	install_element(HANSI_NODE,&set_wirelesscontrol_listen_ip_cmd);
	install_element(HANSI_NODE,&set_wirelesscontrol_auto_ap_binding_wlan_cmd);
	install_element(HANSI_NODE,&del_wirelesscontrol_auto_ap_binding_wlan_cmd);
	install_element(HANSI_NODE,&set_wirelesscontrol_auto_ap_save_config_switch_cmd);
	install_element(HANSI_NODE,&clear_auto_ap_config_cmd);
	install_element(HANSI_NODE,&set_ap_data_exclude_multicast_cmd);
	install_element(HANSI_NODE,&set_ap_statistics_cmd);
	install_element(HANSI_NODE,&set_ap_statistics_inter_cmd);
	install_element(HANSI_WTP_NODE,&set_ap_statistics_inter_cmd);
	install_element(HANSI_NODE,&set_ap_timestamp_cmd);
	install_element(HANSI_NODE,&set_receiver_signal_level_cmd);
	install_element(HANSI_NODE,&set_monitor_time_cmd);
	install_element(HANSI_NODE,&set_sample_time_cmd);
	install_element(HANSI_NODE,&set_monitor_enable_cmd);
	install_element(HANSI_NODE,&set_sample_enable_cmd);
	install_element(HANSI_NODE,&add_legal_manufacturer_cmd);/*xm add*/
	install_element(HANSI_NODE,&add_legal_essid_cmd);
	install_element(HANSI_NODE,&add_attack_ap_mac_cmd);
	install_element(HANSI_NODE,&set_ac_all_ap_extension_information_enable_cmd);/*nl add*/
	install_element(HANSI_NODE,&del_legal_manufacturer_cmd);
	install_element(HANSI_NODE,&del_legal_essid_cmd);
	install_element(HANSI_NODE,&del_attack_ap_mac_cmd);
	install_element(HANSI_NODE,&show_wids_device_of_all_cmd);   //fengwenchao add 20101227
		
	/*ipfwd*/
	install_element(HANSI_NODE,&set_model_cmd);
	install_element(HANSI_NODE,&set_ap_cm_threshold_cmd);
	install_element(HANSI_NODE,&set_neighbordead_interval_cmd);
	install_element(HANSI_NODE,&update_bak_ac_config_cmd);
	install_element(HANSI_NODE,&synchronize_wsm_table_cmd);
	install_element(HANSI_NODE,&synchronize_asd_table_cmd);
	install_element(HANSI_NODE,&notice_vrrp_state_cmd);
	install_element(HANSI_NODE,&set_wtp_wids_interval_cmd);
	install_element(HANSI_NODE,&set_wtp_wids_threshold_cmd);
	install_element(HANSI_NODE,&set_wtp_wids_lasttime_cmd);
	install_element(HANSI_NODE,&batch_config_cmd); 
	install_element(HANSI_NODE,&set_wid_trap_open_cmd);
	install_element(HANSI_NODE,&set_ap_update_img_timer_cmd);

	install_element(HANSI_NODE,&update_wtpcompatible_cmd);
	install_element(HANSI_NODE,&set_ap_update_fail_count_cmd);
	install_element(HANSI_NODE,&set_wid_watch_dog_cmd);
	install_element(HANSI_NODE,&set_ap_access_through_nat_cmd);
	install_element(HANSI_NODE,&show_hansi_interface_cmd);			
	install_element(HANSI_NODE,&set_wid_trap_switch_able_cmd);
	install_element(HANSI_NODE,&modify_legal_essid_cmd);
	install_element(HANSI_NODE,&set_wid_mac_whitelist_cmd);
	install_element(HANSI_NODE,&set_wid_essid_whitelist_cmd);	
	install_element(HANSI_NODE,&change_wirelesscontrol_blacklist_cmd);
	install_element(HANSI_NODE,&change_wirelesscontrol_whitelist_cmd); 	
	install_element(HANSI_NODE,&wireless_interface_vmac_state_cmd);
	install_element(HANSI_NODE,&set_wtp_link_detect_cmd);
	//install_element(HANSI_NODE,&set_wsm_switch_cmd); //wuwl del.share mem has been removed,so cannot get msg from wsm.
	install_element(HANSI_NODE,&set_vlan_switch_cmd);
	install_element(HANSI_NODE,&set_dhcp_option82_switch_cmd);
	install_element(HANSI_NODE,&set_wid_wsm_error_handle_state_cmd);
	install_element(HANSI_NODE,&show_dbus_count_cmd);
	install_element(HANSI_NODE,&set_dbus_count_cmd);
	install_element(HANSI_NODE,&set_process_run_cores_cmd);	

	install_element(HANSI_NODE,&show_neighbor_ap_list_cmd);       //fengwenchao add 20101216
	install_element(HANSI_NODE,&iperf_wtpip_cmd);		//xiaodawei add, 20110303
	install_element(HANSI_NODE,&set_ap_countermeasures_cmd);	   //fengwenchao add 20110516	
	install_element(HANSI_NODE,&set_wids_judge_policy_cmd);
	install_element(HANSI_NODE,&show_wids_judge_policy_cmd);
	install_element(HANSI_NODE,&set_wids_monitor_mode_cmd);
	install_element(HANSI_WTP_NODE,&set_wids_monitor_mode_cmd);
	install_element(HANSI_NODE,&set_wids_scanning_mode_cmd);
	install_element(HANSI_WTP_NODE,&set_wids_scanning_mode_cmd);
	install_element(HANSI_NODE,&set_wids_scanning_channellist_cmd);
	install_element(HANSI_WTP_NODE,&set_wids_scanning_channellist_cmd);
	install_element(HANSI_NODE,&show_wids_scanning_mode_cmd);
	install_element(HANSI_WTP_NODE,&show_wids_scanning_mode_cmd);
	install_element(HANSI_NODE,&show_wireless_listen_if_cmd);
	install_element(HANSI_NODE,&show_wids_mac_list_cmd);												/*a37*/
	install_element(HANSI_NODE,&show_ac_balance_cmd);/*wuwl add 09/12/03*/								/*a38*/
	install_element(HANSI_NODE,&show_ap_network_bywtpid_cmd);											/*a36*/

	install_element(HANSI_NODE,&ac_load_balance_cmd);/*wuwl add 09/12/03*/
	install_element(HANSI_NODE,&ac_balance_parameter_cmd);/*ht add 10.3.5*/
	install_element(HANSI_NODE,&set_ap_hotreboot_cmd);
	install_element(HANSI_NODE,&add_wids_mac_cmd);
	install_element(HANSI_NODE,&del_wids_mac_cmd);
	install_element(HANSI_NODE,&set_ap_countermeasures_mode_cmd);		
	install_element(HANSI_NODE,&show_neighbor_ap_list_bywtpid2_cmd);
	install_element(HANSI_NODE,&set_multicast_listen_cmd);
	install_element(HANSI_NODE,&delete_slotid_img_cmd);//fengwenchao add 20120228 for AXSSZFI-680
	install_element(HANSI_NODE,&set_ac_master_bak_correct_wtp_state_timer_cmd); //fengwenchao add 20120117 for onlinebug-96
	
	install_element(HANSI_NODE, &set_bak_check_interval_cmd);			/* Huang Leilei 2012-10-23 add */
	install_element(HANSI_NODE, &show_bak_check_interval_cmd);			/* Huang Leilei 2012-10-23 add */
	install_element(HANSI_NODE, &set_lic_bak_req_interval_cmd);			/* Huang Leilei 2012-10-25 add */
	install_element(HANSI_NODE, &show_lic_bak_req_interval_cmd);		/* Huang Leilei 2012-10-25 add */
	install_element(HANSI_NODE, &set_asd_new_log_group_cmd);//Qiuchen
	install_element(HANSI_NODE, &set_ac_management_ipaddr_cmd);
	install_element(HANSI_NODE, &show_ac_management_ip_addr_cmd);
	install_element(HANSI_NODE, &set_log_statistics_interval_cmd);
/************wcl add for OSDEVTDPB-31*******************/
 	install_element(HANSI_WTP_NODE,&set_system_country_code_cmd);
	install_element(LOCAL_HANSI_WTP_NODE,&set_system_country_code_cmd);
 	install_element(HANSI_RADIO_NODE,&set_system_country_code_cmd);
	install_element(LOCAL_HANSI_RADIO_NODE,&set_system_country_code_cmd);
	/************wcl add for OSDEVTDPB-31*******************/

	/*-------------------------LOCAL HANSI NODE--------------------------------*/
	
	install_element(LOCAL_HANSI_NODE,&show_wid_config_cmd);													/*a1*/
	install_element(LOCAL_HANSI_NODE,&show_rogue_ap_list_cmd);												/*a2*/
	install_element(LOCAL_HANSI_NODE,&show_rogue_ap_trap_threshold_cmd);										/*a3*/
	install_element(LOCAL_HANSI_NODE,&show_rogue_ap_list_v1_cmd); 											/*a4*/
	install_element(LOCAL_HANSI_NODE,&show_rogue_ap_list_bywtpid_cmd);										/*a5*/
	install_element(LOCAL_HANSI_NODE,&show_neighbor_ap_list_bywtpid_cmd); 									/*a6*/
	install_element(LOCAL_HANSI_NODE,&show_wirelesscontrol_whitelist_cmd);									/*a7*/
	install_element(LOCAL_HANSI_NODE,&show_wirelesscontrol_blacklist_cmd);									/*a8*/
	install_element(LOCAL_HANSI_NODE,&show_wids_device_list_cmd); 											/*a9*/
	install_element(LOCAL_HANSI_NODE,&show_wids_device_list_bywtpid_cmd); 									/*a10*/

	install_element(LOCAL_HANSI_NODE,&show_wids_statistics_list_cmd); 										/*a11*/
	install_element(LOCAL_HANSI_NODE,&show_wids_statistics_list_bywtpid_cmd); 								/*a12*/
	install_element(LOCAL_HANSI_NODE,&show_neighbor_rssi_info_bywtpid_cmd);									/*a13*/
	install_element(LOCAL_HANSI_NODE,&show_auto_ap_config_cmd);												/*a14*/
	install_element(LOCAL_HANSI_NODE,&show_ap_statistics_list_bywtpid_cmd);									/*a15*/
	install_element(LOCAL_HANSI_NODE,&show_ap_ip_bywtpid_cmd);												/*a16*/
	install_element(LOCAL_HANSI_NODE,&show_ap_model_infomation_cmd);											/*a17*/
	install_element(LOCAL_HANSI_NODE,&show_ap_txpower_control_cmd);											/*a18*/
	install_element(LOCAL_HANSI_NODE,&show_receiver_signal_level_cmd);										/*a19*/
	install_element(LOCAL_HANSI_NODE,&show_sample_info_cmd);													/*a20*/

	install_element(LOCAL_HANSI_NODE,&show_manufacturer_oui_list_cmd);										/*a21*/
	install_element(LOCAL_HANSI_NODE,&show_legal_essid_list_cmd); 											/*a22*/
	install_element(LOCAL_HANSI_NODE,&show_attack_mac_list_cmd);												/*a23*/
	install_element(LOCAL_HANSI_NODE,&show_model_cmd);														/*a24*/
	install_element(LOCAL_HANSI_NODE,&show_access_wtp_vendor_count_cmd);										/*a25*/
	install_element(LOCAL_HANSI_NODE,&show_model_list_cmd);													/*a26*/
	install_element(LOCAL_HANSI_NODE,&show_ap_model_code_cmd);												/*a27*/
	install_element(LOCAL_HANSI_NODE,&show_ap_threshold_cmd); 												/*a28*/
	install_element(LOCAL_HANSI_NODE,&show_ap_rrm_config_cmd);												/*a29*/
	install_element(LOCAL_HANSI_NODE,&show_neighbordead_interval_cmd);										/*a30*/

	install_element(LOCAL_HANSI_NODE,&show_vrrp_state_cmd);													/*a31*/
	//install_element(HANSI_NODE,&show_vrrp_sock_cmd);			/*fengwenchao comment 20110418*/										/*a32*/
	install_element(LOCAL_HANSI_NODE,&show_ap_update_img_timer_cmd);											/*a33*/
	install_element(LOCAL_HANSI_NODE,&show_ap_update_fail_count_cmd); 										/*a34*/
	install_element(LOCAL_HANSI_NODE,&show_wid_trap_switch_info_cmd); 										/*a35*/
	//install_element(HANSI_NODE,&set_license_binding_cmd);/*xiaodawei add, 20101111*/
	/*
	install_element(CONFIG_NODE,&set_wid_hw_version_cmd);	
	install_element(CONFIG_NODE,&set_wid_sw_version_cmd);
	install_element(CONFIG_NODE,&set_wid_lev3_protocol_cmd);
	install_element(CONFIG_NODE,&set_wid_auth_security_cmd);
	install_element(CONFIG_NODE,&set_wid_name_cmd);
	*/
	install_element(LOCAL_HANSI_NODE,&set_ap_scanning_cmd);
	install_element(LOCAL_HANSI_NODE,&set_ap_scanning_report_interval_cmd);
	/*install_element(CONFIG_NODE,&update_ap_scanning_info_cmd);*/
	install_element(LOCAL_HANSI_NODE,&set_wirelesscontrol_whitelist_cmd);
	install_element(LOCAL_HANSI_NODE,&set_wirelesscontrol_blacklist_cmd);
	install_element(LOCAL_HANSI_NODE,&delete_wirelesscontrol_whitelist_cmd);
	install_element(LOCAL_HANSI_NODE,&delete_wirelesscontrol_blacklist_cmd);
	install_element(LOCAL_HANSI_NODE,&clear_wids_device_list_cmd);
	install_element(LOCAL_HANSI_NODE,&clear_wids_device_list_bywtpid_cmd);
	install_element(LOCAL_HANSI_NODE,&clear_wids_statistics_list_cmd);
	install_element(LOCAL_HANSI_NODE,&clear_wids_statistics_list_bywtpid_cmd);
	/*install_element(CONFIG_NODE,&show_radio_resource_managment_cmd);
//	install_element(CONFIG_NODE,&set_wirelesscontrol_max_wtp_cmd);*/
	install_element(LOCAL_HANSI_NODE, &set_wirelesscontrol_max_wtp_cmd);
	install_element(LOCAL_HANSI_NODE,&set_wid_max_mtu_cmd);
	install_element(LOCAL_HANSI_NODE,&set_wid_log_switch_cmd);
	install_element(LOCAL_HANSI_NODE,&set_wid_log_size_cmd); 
	install_element(LOCAL_HANSI_NODE,&set_wid_log_level_cmd);
	install_element(LOCAL_HANSI_NODE,&set_wid_daemonlog_debug_open_cmd);
	install_element(LOCAL_HANSI_NODE,&dynamic_channel_selection_cmd); 		
	install_element(LOCAL_HANSI_NODE,&dynamic_channel_selection_range_cmd);
	install_element(LOCAL_HANSI_NODE,&transmit_power_control_cmd);
	install_element(LOCAL_HANSI_NODE,&set_txpower_threshold_cmd);
	install_element(LOCAL_HANSI_NODE,&set_rogue_ap_trap_threshold_cmd);
	install_element(LOCAL_HANSI_NODE,&set_coverage_threshold_cmd);
	install_element(LOCAL_HANSI_NODE,&set_transmit_power_control_scope_cmd);
	/*country code area*/
	install_element(LOCAL_HANSI_NODE,&set_system_country_code_cmd);
	install_element(LOCAL_HANSI_NODE,&undo_system_country_code_cmd);
	install_element(LOCAL_HANSI_NODE,&set_asd_daemonlog_debug_open_cmd);
	install_element(LOCAL_HANSI_NODE,&set_hostapd_logger_printflag_open_cmd);
	install_element(LOCAL_HANSI_NODE,&set_asd_daemonlog_level_cmd);
	/*auto ap area*/
	install_element(LOCAL_HANSI_NODE,&set_wirelesscontrol_auto_ap_switch_cmd);
	install_element(LOCAL_HANSI_NODE,&set_wireless_memory_trace_switch_cmd);
	install_element(LOCAL_HANSI_NODE,&set_wirelesscontrol_auto_ap_binding_l3_interface_cmd);
	install_element(LOCAL_HANSI_NODE,&set_wirelesscontrol_auto_ap_binding_l3_interface_new_cmd);
	install_element(LOCAL_HANSI_NODE,&set_wirelesscontrol_listen_l3_interface_cmd);
	install_element(LOCAL_HANSI_NODE,&set_wirelesscontrol_listen_ip_cmd);
	install_element(LOCAL_HANSI_NODE,&set_wirelesscontrol_auto_ap_binding_wlan_cmd);
	install_element(LOCAL_HANSI_NODE,&del_wirelesscontrol_auto_ap_binding_wlan_cmd);
	install_element(LOCAL_HANSI_NODE,&set_wirelesscontrol_auto_ap_save_config_switch_cmd);
	install_element(LOCAL_HANSI_NODE,&clear_auto_ap_config_cmd);
	install_element(LOCAL_HANSI_NODE,&set_ap_data_exclude_multicast_cmd);
	install_element(LOCAL_HANSI_NODE,&set_ap_statistics_cmd);
	install_element(LOCAL_HANSI_NODE,&set_ap_statistics_inter_cmd);
	//install_element(LOCAL_HANSI_WTP_NODE,&set_ap_statistics_inter_cmd);
	install_element(LOCAL_HANSI_NODE,&set_ap_timestamp_cmd);
	install_element(LOCAL_HANSI_NODE,&set_receiver_signal_level_cmd);
	install_element(LOCAL_HANSI_NODE,&set_monitor_time_cmd);
	install_element(LOCAL_HANSI_NODE,&set_sample_time_cmd);
	install_element(LOCAL_HANSI_NODE,&set_monitor_enable_cmd);
	install_element(LOCAL_HANSI_NODE,&set_sample_enable_cmd);
	install_element(LOCAL_HANSI_NODE,&add_legal_manufacturer_cmd);/*xm add*/
	install_element(LOCAL_HANSI_NODE,&add_legal_essid_cmd);
	install_element(LOCAL_HANSI_NODE,&add_attack_ap_mac_cmd);
	install_element(LOCAL_HANSI_NODE,&set_ac_all_ap_extension_information_enable_cmd);/*nl add*/
	install_element(LOCAL_HANSI_NODE,&del_legal_manufacturer_cmd);
	install_element(LOCAL_HANSI_NODE,&del_legal_essid_cmd);
	install_element(LOCAL_HANSI_NODE,&del_attack_ap_mac_cmd);
	install_element(LOCAL_HANSI_NODE,&show_wids_device_of_all_cmd);	//fengwenchao add 20101227
		
	/*ipfwd*/
	install_element(LOCAL_HANSI_NODE,&set_model_cmd);
	install_element(LOCAL_HANSI_NODE,&set_ap_cm_threshold_cmd);
	install_element(LOCAL_HANSI_NODE,&set_neighbordead_interval_cmd);
	install_element(LOCAL_HANSI_NODE,&update_bak_ac_config_cmd);
	install_element(LOCAL_HANSI_NODE,&synchronize_wsm_table_cmd);
	install_element(LOCAL_HANSI_NODE,&synchronize_asd_table_cmd);
	install_element(LOCAL_HANSI_NODE,&notice_vrrp_state_cmd);
	install_element(LOCAL_HANSI_NODE,&set_wtp_wids_interval_cmd);
	install_element(LOCAL_HANSI_NODE,&set_wtp_wids_threshold_cmd);
	install_element(LOCAL_HANSI_NODE,&set_wtp_wids_lasttime_cmd);
	install_element(LOCAL_HANSI_NODE,&batch_config_cmd); 
	install_element(LOCAL_HANSI_NODE,&set_wid_trap_open_cmd);
	install_element(LOCAL_HANSI_NODE,&set_ap_update_img_timer_cmd);

	install_element(LOCAL_HANSI_NODE,&update_wtpcompatible_cmd);
	install_element(LOCAL_HANSI_NODE,&set_ap_update_fail_count_cmd);
	install_element(LOCAL_HANSI_NODE,&set_wid_watch_dog_cmd);
	install_element(LOCAL_HANSI_NODE,&set_ap_access_through_nat_cmd);
	install_element(LOCAL_HANSI_NODE,&show_hansi_interface_cmd);			
	install_element(LOCAL_HANSI_NODE,&set_wid_trap_switch_able_cmd);
	install_element(LOCAL_HANSI_NODE,&modify_legal_essid_cmd);
	install_element(LOCAL_HANSI_NODE,&set_wid_mac_whitelist_cmd);
	install_element(LOCAL_HANSI_NODE,&set_wid_essid_whitelist_cmd);	
	install_element(LOCAL_HANSI_NODE,&change_wirelesscontrol_blacklist_cmd);
	install_element(LOCAL_HANSI_NODE,&change_wirelesscontrol_whitelist_cmd);	
	install_element(LOCAL_HANSI_NODE,&wireless_interface_vmac_state_cmd);
	install_element(LOCAL_HANSI_NODE,&set_wtp_link_detect_cmd);
	//install_element(LOCAL_HANSI_NODE,&set_wsm_switch_cmd);  //wuwl del.share mem has been removed,so cannot get msg from wsm.
	install_element(LOCAL_HANSI_NODE,&set_vlan_switch_cmd);	
	install_element(LOCAL_HANSI_NODE,&set_dhcp_option82_switch_cmd);
	install_element(LOCAL_HANSI_NODE,&set_wid_wsm_error_handle_state_cmd);
	install_element(LOCAL_HANSI_NODE,&show_dbus_count_cmd);
	install_element(LOCAL_HANSI_NODE,&set_dbus_count_cmd);
	install_element(LOCAL_HANSI_NODE,&set_process_run_cores_cmd); 

	install_element(LOCAL_HANSI_NODE,&show_neighbor_ap_list_cmd); 	  //fengwenchao add 20101216
	install_element(LOCAL_HANSI_NODE,&iperf_wtpip_cmd);		//xiaodawei add, 20110303
	install_element(LOCAL_HANSI_NODE,&set_wids_judge_policy_cmd);
	install_element(LOCAL_HANSI_NODE,&show_wids_judge_policy_cmd);
	install_element(LOCAL_HANSI_NODE,&set_wids_monitor_mode_cmd);
	install_element(LOCAL_HANSI_WTP_NODE,&set_wids_monitor_mode_cmd);
	install_element(LOCAL_HANSI_NODE,&set_wids_scanning_mode_cmd);
	install_element(LOCAL_HANSI_WTP_NODE,&set_wids_scanning_mode_cmd);
	install_element(LOCAL_HANSI_NODE,&set_wids_scanning_channellist_cmd);
	install_element(LOCAL_HANSI_WTP_NODE,&set_wids_scanning_channellist_cmd);
	install_element(LOCAL_HANSI_NODE,&show_wids_scanning_mode_cmd);
	install_element(LOCAL_HANSI_WTP_NODE,&show_wids_scanning_mode_cmd);
			
	install_element(LOCAL_HANSI_NODE,&show_wireless_listen_if_cmd);
	
	install_element(HIDDENDEBUG_NODE,&set_wirelesscontrol_max_wtp_cmd);
	install_element(LOCAL_HANSI_NODE,&show_wids_mac_list_cmd);												/*a37*/
	install_element(LOCAL_HANSI_NODE,&show_ac_balance_cmd);/*wuwl add 09/12/03*/								/*a38*/
	install_element(LOCAL_HANSI_NODE,&show_ap_network_bywtpid_cmd);											/*a36*/

	install_element(LOCAL_HANSI_NODE,&ac_load_balance_cmd);/*wuwl add 09/12/03*/
	install_element(LOCAL_HANSI_NODE,&ac_balance_parameter_cmd);/*ht add 10.3.5*/
	install_element(LOCAL_HANSI_NODE,&set_ap_hotreboot_cmd);
	install_element(LOCAL_HANSI_NODE,&add_wids_mac_cmd);
	install_element(LOCAL_HANSI_NODE,&del_wids_mac_cmd);
	install_element(LOCAL_HANSI_NODE,&set_ap_countermeasures_cmd);  //L
	install_element(LOCAL_HANSI_NODE,&set_ap_countermeasures_mode_cmd);		
	install_element(LOCAL_HANSI_NODE,&show_neighbor_ap_list_bywtpid2_cmd);
	install_element(LOCAL_HANSI_NODE,&set_multicast_listen_cmd);
	install_element(LOCAL_HANSI_NODE,&delete_slotid_img_cmd);//fengwenchao add 20120228 for AXSSZFI-680
	install_element(LOCAL_HANSI_NODE,&set_ac_master_bak_correct_wtp_state_timer_cmd); //fengwenchao add 20120117 for onlinebug-96

	install_element(LOCAL_HANSI_NODE, &set_bak_check_interval_cmd);			/* Huang Leilei 2012-10-23 add */
	install_element(LOCAL_HANSI_NODE, &show_bak_check_interval_cmd);		/* Huang Leilei 2012-10-23 add */
	install_element(LOCAL_HANSI_NODE, &set_lic_bak_req_interval_cmd);		/* Huang Leilei 2012-10-25 add */
	install_element(LOCAL_HANSI_NODE, &show_lic_bak_req_interval_cmd);		/* Huang Leilei 2012-10-25 add */
	
	install_element(LOCAL_HANSI_NODE, &set_asd_new_log_group_cmd);//Qiuchen
	install_element(LOCAL_HANSI_NODE, &set_ac_management_ipaddr_cmd);
	install_element(LOCAL_HANSI_NODE, &show_ac_management_ip_addr_cmd);
	install_element(LOCAL_HANSI_NODE, &set_log_statistics_interval_cmd);

	install_element(LOCAL_HANSI_NODE,&set_ac_master_ipaddr_cmd); 
	install_element(HANSI_NODE,&set_ac_master_ipaddr_cmd); 
	install_element(LOCAL_HANSI_NODE,&show_is_secondary_cmd); /*chenjun12.23*/
	install_element(HANSI_NODE,&show_is_secondary_cmd); 
	install_element(LOCAL_HANSI_NODE,&set_is_secondary_cmd); /*chenjun12.23*/
	install_element(HANSI_NODE,&set_is_secondary_cmd); 
	return;
}

#endif

