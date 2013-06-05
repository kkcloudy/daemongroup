#ifdef _D_WCPSS_
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <zebra.h>
#include <dbus/dbus.h>

#include "command.h"
#include "if.h"

#include "../dcli_main.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "wcpss/asd/asd.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dcli_wqos.h"  
#include "wid_ac.h"
#include "wid_wtp.h"
#include "wid_wqos.h"
#include "dbus/wcpss/dcli_wid_wtp.h"
#include "bsd/bsdpub.h"

struct cmd_node wqos_node =
{
	WQOS_NODE,
	"%s(config-qos %d)# "
};
struct cmd_node hansi_wqos_node =
{
	HANSI_WQOS_NODE,
	"%s(hansi-wqos %d-%d-%d)# ",
};

struct cmd_node local_hansi_wqos_node =
{
	LOCAL_HANSI_WQOS_NODE,
	"%s(local-hansi-wqos %d-%d-%d)# ",
};

extern char dcli_vty_ifname[INTERFACE_NAMSIZ+1];
extern char vlan_eth_port_ifname [INTERFACE_NAMSIZ];

int wid_interface_ifname_wlan(char *ptr,struct vty *vty, char *line)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	
	int ret = 0;
	int result = 0;
	int local = 1;
	int slotid = 0;
	unsigned char ID = 0;
	unsigned char policy = 1;
	char *id = (char *)malloc(sizeof(char)*25);
	memset(id,0,25);
	if(id == NULL)
	{
		vty_out(vty,"malloc error\n");
		return MALLOC_ERROR;
	}
	memcpy(id,ptr+4,(strlen(ptr)-4));
	ret = parse_char_ID(id,&ID);
	if (ret != WID_DBUS_SUCCESS) 
	{
         if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"input id invalid\n");
			}
		WID_IF_FREE_OBJECT(id);
		return CMD_WARNING;
	}
		
	if ((ID < 1) || (ID > WLAN_NUM))
	{
		WID_IF_FREE_OBJECT(id);
		vty_out(vty,"input id invalid\n");
		return CMD_WARNING;
	}
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
		slotid = vty->slotindex;
		local = 0;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		slotid = vty->slotindex;
		local = 1;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slotid,distributFag);
	ReInitDbusPath_V2(local,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(local,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(local,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_L3IF_POLICY_BR);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
					WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_L3IF_POLICY_BR);*/

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&ID,
							 DBUS_TYPE_BYTE,&policy,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,3000000, &err);

	WID_IF_FREE_OBJECT(id);
	
	dbus_message_unref(query);
		
	if (NULL == reply)
	{
		/*vty_out(vty,"failed get reply.\n");*/
		if (dbus_error_is_set(&err))
		{
			/*cli_syslog_info("%s raised: %s",err.name,err.message);*/
			dbus_error_free_for_dcli(&err);
		}
		vty_out(vty,"no reply\n");
		return CMD_WARNING;
	}
		
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
/*	sleep(1); */
	
	if((ret == 0)||(ret == UNKNOWN_ERROR))/*interface success*/
	{
		if(local){
			sprintf(line,"interface wlanl%d-%d-%d",slotid,index,ID);
			
			memset(vlan_eth_port_ifname,0,INTERFACE_NAMSIZ);				
			memset(dcli_vty_ifname,0,INTERFACE_NAMSIZ+1);
			sprintf(vlan_eth_port_ifname,"wlanl%d-%d-%d",slotid,index,ID);		
			sprintf(dcli_vty_ifname,"wlanl%d-%d-%d",slotid,index,ID);
		}else{
			sprintf(line,"interface wlan%d-%d-%d",slotid,index,ID);
			
			memset(vlan_eth_port_ifname,0,INTERFACE_NAMSIZ);				
			memset(dcli_vty_ifname,0,INTERFACE_NAMSIZ+1);
			sprintf(vlan_eth_port_ifname,"wlan%d-%d-%d",slotid,index,ID);		
			sprintf(dcli_vty_ifname,"wlan%d-%d-%d",slotid,index,ID);
		}

		if(ret == UNKNOWN_ERROR)  //fengwenchao add for AXSSZFI-1587
			vty_out(vty,"wlan interface is already exist\n");
		result = CMD_SUCCESS;
	}
	else 
	{
		if(ret == MALLOC_ERROR)
			vty_out(vty,"malloc error\n");
		else if(ret == CMD_WARNING)
			vty_out(vty,"input wlan id is invalid\n");
		else if(ret == WLAN_ID_NOT_EXIST)
			vty_out(vty,"Wlan ID is not exist\n");
		else if(ret == WLAN_BE_ENABLE)
			vty_out(vty,"WLAN is enable, if you want to operate this, please disable it first\n");
		else if(ret == WLAN_CREATE_L3_INTERFACE_FAIL)
			vty_out(vty,"WLAN create l3 interface fail\n"); 
		else if(ret == WLAN_DELETE_L3_INTERFACE_FAIL)
			vty_out(vty,"WLAN delete l3 interface fail\n");
		else if(ret == WLAN_BINDING_VLAN)
			vty_out(vty,"WLAN have already binding vlan,please undo wlan-vlan binding first\n");
		else if(ret == WLAN_CREATE_BR_FAIL)
			vty_out(vty,"WLAN create br fail\n");
		else if(ret == WLAN_DELETE_BR_FAIL)
			vty_out(vty,"WLAN delete br fail\n");
		else if(ret == BSS_L3_INTERFACE_ADD_BR_FAIL)
			vty_out(vty,"WLAN add bss interface to br fail\n");
		else if(ret == BSS_L3_INTERFACE_DEL_BR_FAIL)
			vty_out(vty,"WLAN del bss interface to br fail\n");
		else if(ret == RADIO_IN_EBR)
			vty_out(vty,"<error>some radio interface in ebr,please remove it from ebr");
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else
			vty_out(vty,"error %d\n",ret);

		result = CMD_WARNING;
	
	}
	return result;		
	

}
int wid_interface_ifname_radio(char *ptr,struct vty *vty,char *line)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	int ret = 0;
	int result = 0;
	char ID = 0;
	int wtpid = 0;
	int wlanid = 0;
	int radioid = 0;
	int g_radioid = 0;
	unsigned char policy = 2;
	int local = 1;
	int slotid = 0;
	char *id = (char *)malloc(sizeof(char)*25);
	memset(id,0,25);
	if(id == NULL)
	{
		vty_out(vty,"malloc error\n");
		return MALLOC_ERROR;
	}
	if((!strncasecmp(ptr,"radio",5)))
	memcpy(id,ptr+5,(strlen(ptr)-5));
	else if(!strncasecmp(ptr,"r",1))
		memcpy(id,ptr+1,(strlen(ptr)-1));
	ret = parse_radio_ifname(id,&wtpid,&radioid,&wlanid);
	/*printf("wtpid:%d,radioid:%d,wlanid:%d\n",wtpid,radioid,wlanid);*/
	if (ret != WID_DBUS_SUCCESS) 
	{
		WID_IF_FREE_OBJECT(id);
		vty_out(vty,"input id invalid\n");
		return CMD_WARNING;
	}
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
		slotid = vty->slotindex;
		local = 0;
	}else if (vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		local = 1;
		slotid = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slotid,distributFag);
	ReInitDbusPath_V2(local,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(local,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(local,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_L3IF_POLICY2);
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
					WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_L3IF_POLICY2);*/

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wlanid,
							 DBUS_TYPE_BYTE,&policy,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_UINT32,&radioid,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	WID_IF_FREE_OBJECT(id);

	dbus_message_unref(query);
		
	if (NULL == reply)
	{
		vty_out(vty,"no reply\n");
		return CMD_WARNING;
	}
		
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
/*	sleep(1);*/

	if(ret == 0)/*interface success*/
	{
		if(local){
			sprintf(line,"interface r%d-%d-%d.%d",index,wtpid,radioid,wlanid);
			memset(vlan_eth_port_ifname,0,INTERFACE_NAMSIZ);				
			memset(dcli_vty_ifname,0,INTERFACE_NAMSIZ+1);
			sprintf(vlan_eth_port_ifname,"r%d-%d-%d.%d",index,wtpid,radioid,wlanid);		
			sprintf(dcli_vty_ifname,"r%d-%d-%d.%d",index,wtpid,radioid,wlanid);
		}else{
			sprintf(line,"interface r%d-%d-%d-%d.%d",slotid,index,wtpid,radioid,wlanid);
			memset(vlan_eth_port_ifname,0,INTERFACE_NAMSIZ);				
			memset(dcli_vty_ifname,0,INTERFACE_NAMSIZ+1);
			sprintf(vlan_eth_port_ifname,"r%d-%d-%d-%d.%d",slotid,index,wtpid,radioid,wlanid);		
			sprintf(dcli_vty_ifname,"r%d%d-%d-%d.%d",slotid,index,wtpid,radioid,wlanid);
		}
		
		result = CMD_SUCCESS;
	}
	else 
	{
		if(ret == MALLOC_ERROR)
			vty_out(vty,"malloc error\n");
		else if(ret == CMD_WARNING)
			vty_out(vty,"input id is invalid\n");
		else if(ret == WTP_ID_NOT_EXIST)
			vty_out(vty,"WTP ID is not exist\n");
		else if(ret == WLAN_ID_NOT_EXIST)
			vty_out(vty,"Wlan ID is not exist\n");
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"RADIO ID is not exist\n");
		else if(ret == BSS_NOT_EXIST)
			vty_out(vty,"BSS is not exist\n");
		else if(ret == WTP_IS_NOT_BINDING_WLAN_ID)
			vty_out(vty,"WTP is not binding wlan,binding it first\n");
		else if(ret == WTP_WLAN_BINDING_NOT_MATCH)
			vty_out(vty,"WTP binding WLAN is not match,check again\n");
		else if(ret == BSS_BE_ENABLE)
			vty_out(vty,"BSS is enable, if you want to operate this, please disable it first\n");
		else if(ret == IF_POLICY_CONFLICT)
			vty_out(vty,"WLAN is no_interface , BSS can not be wlan_interface\n");
		else if(ret == BSS_CREATE_L3_INTERFACE_FAIL)
			vty_out(vty,"BSS create l3 interface fail\n"); 
		else if(ret == BSS_DELETE_L3_INTERFACE_FAIL)
			vty_out(vty,"BSS delete l3 interface fail\n");
		else if(ret == WLAN_CREATE_BR_FAIL)
			vty_out(vty,"WLAN create l3 interface fail\n"); 
		else if(ret == WLAN_DELETE_BR_FAIL)
			vty_out(vty,"WLAN delete l3 interface fail\n");
		else if(ret == BSS_L3_INTERFACE_ADD_BR_FAIL)
			vty_out(vty,"WLAN create l3 interface fail\n"); 
		else if(ret == BSS_L3_INTERFACE_DEL_BR_FAIL)
			vty_out(vty,"WLAN delete l3 interface fail\n");
		else if (ret == RADIO_IN_EBR)
			vty_out(vty, "this radio is in ebr\n");
		else if (ret == L3_INTERFACE_ERROR)
			vty_out(vty, "layer3-interface error\n");
		else if (ret == UNKNOWN_ERROR)
			vty_out(vty, "unknown error occur\n");
		else
		{
			vty_out(vty,"other error %d\n",ret);
			cli_syslog_info("<error> %s %s %d ret:%d\n", __FILE__, __func__, __LINE__, ret);
		}

		result = CMD_WARNING;
	
	}
	return result;	
}
int wid_interface_ifname_ebr(char *ptr,struct vty *vty,char *line)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	int ret = 0;
	int result = 0;
	unsigned int ID = 0;
	char *id = (char *)malloc(sizeof(char)*25);
	int slotid = 0;
	int local = 1;
	memset(id,0,25);
	if(id == NULL)
	{
		vty_out(vty,"malloc error\n");
		return MALLOC_ERROR;
	}
	memcpy(id,ptr+3,(strlen(ptr)-3));
	
	ret = parse_int_ID(id,&ID);
	if (ret != WID_DBUS_SUCCESS) 
	{
		WID_IF_FREE_OBJECT(id);
		vty_out(vty,"input id invalid\n");
		return CMD_WARNING;
	}
		
	if ((ID < 1) || (ID > EBR_NUM))
	{
		WID_IF_FREE_OBJECT(id);
		vty_out(vty,"input id invalid\n");
		return CMD_WARNING;
	}
	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
		slotid = vty->slotindex;
		local = 0;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		slotid = vty->slotindex;
		local = 1;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slotid,distributFag);
	ReInitDbusPath_V2(local,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(local,index,WID_DBUS_EBR_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(local,index,WID_DBUS_EBR_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_EBR_METHOD_EBR_INTERFACE_EBR);
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_EBR_OBJPATH,\
					WID_DBUS_EBR_INTERFACE,WID_DBUS_EBR_METHOD_EBR_INTERFACE_EBR);*/

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ID,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	WID_IF_FREE_OBJECT(id);

	dbus_message_unref(query);
		
	if (NULL == reply)
	{
		vty_out(vty,"no reply\n");
		return CMD_WARNING;
	}
		
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
/*	sleep(1);*/

	if(ret == 0)/*interface success*/
	{

		if(local){
			sprintf(line,"interface ebrl%d-%d-%d",slotid,index,ID);
			memset(vlan_eth_port_ifname,0,INTERFACE_NAMSIZ);				
			memset(dcli_vty_ifname,0,INTERFACE_NAMSIZ+1);
			sprintf(vlan_eth_port_ifname,"ebrl%d-%d-%d",slotid,index,ID);		
			sprintf(dcli_vty_ifname,"ebrl%d-%d-%d",slotid,index,ID);
		}else{
			sprintf(line,"interface ebr%d-%d-%d",slotid,index,ID);
			memset(vlan_eth_port_ifname,0,INTERFACE_NAMSIZ);				
			memset(dcli_vty_ifname,0,INTERFACE_NAMSIZ+1);
			sprintf(vlan_eth_port_ifname,"ebr%d-%d-%d",slotid,index,ID);		
			sprintf(dcli_vty_ifname,"ebr%d-%d-%d",slotid,index,ID);
		}
		result = CMD_SUCCESS;
	}
	else 
	{
		if(ret == MALLOC_ERROR)
			vty_out(vty,"malloc error\n");
		else if(ret == CMD_WARNING)
			vty_out(vty,"input id is invalid\n");
		else if(ret == WID_EBR_NOT_EXIST)
			vty_out(vty,"EBR ID is not exist\n");
		else if(ret == WID_EBR_ERROR)
			vty_out(vty,"EBR interface error\n");
		else
			vty_out(vty,"error %d\n",ret);

		result = CMD_WARNING;
	
	}
	return result;	
}

int wid_no_interface_ifname_wlan(char *ptr,struct vty *vty)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	int ret = 0;
	int result = 0;
	int local = 1;
	int slotid = 0;
	unsigned char ID = 0;
	unsigned char policy = 0;
	char *id = (char *)malloc(sizeof(char)*25);
	memset(id,0,25);
	if(id == NULL)
	{
		vty_out(vty,"malloc error\n");
		return MALLOC_ERROR;
	}

	memcpy(id,ptr+4,(strlen(ptr)-4));
	
	ret = parse_char_ID(id,&ID);
		
	if (ret != WID_DBUS_SUCCESS) 
	{
	   if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"input id invalid\n");
			}
		WID_IF_FREE_OBJECT(id);
		return CMD_WARNING;
	}
		
	if ((ID < 1) || (ID > WLAN_NUM))
	{
		WID_IF_FREE_OBJECT(id);
		vty_out(vty,"input id invalid\n");
		return CMD_WARNING;
	}
		
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
		slotid = vty->slotindex;
		local = 0;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		slotid = vty->slotindex;
		local = 1;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slotid,distributFag);
	hmd_wireless_check_setting(dcli_dbus_connection, slotid,0);

	ReInitDbusPath_V2(local,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(local,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(local,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_L3IF_POLICY_BR);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
					WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_L3IF_POLICY_BR);*/

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&ID,
							 DBUS_TYPE_BYTE,&policy,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,3000000, &err);   //fengwenchao change -1 to 300000 ,20110601

	WID_IF_FREE_OBJECT(id);
	
	dbus_message_unref(query);
		
	if (NULL == reply)
	{
		/*vty_out(vty,"failed get reply.\n");*/
		if (dbus_error_is_set(&err))
		{
			/*cli_syslog_info("%s raised: %s",err.name,err.message);*/
			dbus_error_free_for_dcli(&err);
		}
		vty_out(vty,"no reply\n");
		hmd_wireless_check_setting(dcli_dbus_connection, slotid,1);
		return CMD_WARNING;
	}
		
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
	sleep(1);
	hmd_wireless_check_setting(dcli_dbus_connection, slotid,1);

	if((ret == 0)||(ret == UNKNOWN_ERROR))/*no interface success*/
	{
		result = CMD_SUCCESS;
		if(ret == UNKNOWN_ERROR)  //fengwenchao add for AXSSZFI-1587
			vty_out(vty,"wlan interface is not exist\n");
	}
	else 
	{
		if(ret == MALLOC_ERROR)
			vty_out(vty,"malloc error\n");
		else if(ret == CMD_WARNING)
			vty_out(vty,"input wlan id is invalid\n");
		else if(ret == WLAN_ID_NOT_EXIST)
			vty_out(vty,"Wlan ID is not exist\n");
		else if(ret == WLAN_BE_ENABLE)
			vty_out(vty,"WLAN is enable, if you want to operate this, please disable it first\n");
		else if(ret == WLAN_CREATE_L3_INTERFACE_FAIL)
			vty_out(vty,"WLAN create l3 interface fail\n"); 
		else if(ret == WLAN_DELETE_L3_INTERFACE_FAIL)
			vty_out(vty,"WLAN delete l3 interface fail\n");
		else if(ret == WLAN_BINDING_VLAN)
		vty_out(vty,"WLAN have already binding vlan,please undo wlan-vlan binding first\n");
		else if(ret == WLAN_CREATE_BR_FAIL)
			vty_out(vty,"WLAN create br fail\n");
		else if(ret == WLAN_DELETE_BR_FAIL)
			vty_out(vty,"WLAN delete br fail\n");
		else if(ret == BSS_L3_INTERFACE_ADD_BR_FAIL)
			vty_out(vty,"WLAN add bss interface to br fail\n");
		else if(ret == BSS_L3_INTERFACE_DEL_BR_FAIL)
			vty_out(vty,"WLAN del bss interface to br fail\n");
		else
			vty_out(vty,"error %d\n",ret);

		result = CMD_WARNING;
	
	}
	return result;

}
int wid_no_interface_ifname_radio(char *ptr,struct vty *vty)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	int ebr_id = 0;
	int ret = 0;
	int result = 0;
	char ID = 0;
	int wtpid = 0;
	int wlanid = 0;
	int radioid = 0;
	int g_radioid = 0;
	unsigned char policy = 0;
	int local = 1;
	int slotid = 0;
	unsigned char ifindex_name[256] = {0};
	char *id = (char *)malloc(sizeof(char)*25);
	memset(id,0,25);
	if(id == NULL)
	{	
		vty_out(vty,"malloc error\n");
		return MALLOC_ERROR;
	}

	if((!strncasecmp(ptr,"radio",5)))
	memcpy(id,ptr+5,(strlen(ptr)-5));
	else if(!strncasecmp(ptr,"r",1))
		memcpy(id,ptr+1,(strlen(ptr)-1));

	ret = parse_radio_ifname(id,&wtpid,&radioid,&wlanid);
	/*printf("wtpid:%d,radioid:%d,wlanid:%d\n",wtpid,radioid,wlanid);*/
	if (ret != WID_DBUS_SUCCESS) 
	{
		WID_IF_FREE_OBJECT(id);
		vty_out(vty,"input id invalid\n");
		return CMD_WARNING;
	}
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
		slotid = vty->slotindex;
		local = 0;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		slotid = vty->slotindex;
		local = 1;
	}
	/*ret = check_whether_in_ebr(index,wtpid,radioid,wlanid,&ebr_id);
	if(ret == RADIO_IN_EBR)
	{
		WID_IF_FREE_OBJECT(id);
		if((ebr_id > 0)&&(if_indextoname(ebr_id,ifindex_name)))
			vty_out(vty,"error,interface in ebr %s,please del it from ebr\n",ifindex_name);
		return CMD_WARNING;
	}*/
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slotid,distributFag);
	ReInitDbusPath_V2(local,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(local,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(local,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_L3IF_POLICY2);
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
					WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_L3IF_POLICY2);*/

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wlanid,
							 DBUS_TYPE_BYTE,&policy,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_UINT32,&radioid,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	WID_IF_FREE_OBJECT(id);

	dbus_message_unref(query);
		
	if (NULL == reply)
	{
		vty_out(vty,"no reply\n");
		return CMD_WARNING;
	}
		
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
	sleep(1);

	if(ret == 0)/*no interface success*/
	{
		result = CMD_SUCCESS;
	}
	else 
	{	
		if(ret == MALLOC_ERROR)
			vty_out(vty,"malloc error\n");
		else if(ret == CMD_WARNING)
			vty_out(vty,"input id is invalid\n");
		else if(ret == WTP_ID_NOT_EXIST)
			vty_out(vty,"WTP ID is not exist\n");
		else if(ret == WLAN_ID_NOT_EXIST)
			vty_out(vty,"Wlan ID is not exist\n");
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"WTP ID is not exist\n");
		else if(ret == WTP_IS_NOT_BINDING_WLAN_ID)
			vty_out(vty,"WTP is not binding wlan\n");
		else if(ret == BSS_NOT_EXIST)
			vty_out(vty,"BSS is not exist\n");
		else if(ret == WTP_WLAN_BINDING_NOT_MATCH)
			vty_out(vty,"WTP binding WLAN is not match,check again\n");
		else if(ret == BSS_BE_ENABLE)
			vty_out(vty,"BSS is enable, if you want to operate this, please disable it first\n");
		else if(ret == BSS_CREATE_L3_INTERFACE_FAIL)
			vty_out(vty,"BSS create l3 interface fail\n"); 
		else if(ret == BSS_DELETE_L3_INTERFACE_FAIL)
			vty_out(vty,"BSS delete l3 interface fail\n");
		else if(ret == BSS_L3_INTERFACE_DEL_BR_FAIL)
			vty_out(vty,"BSS remove l3 interface from wlan br fail\n");
		else if(ret == RADIO_IN_EBR)
			vty_out(vty,"<error> interface in ebr,please del it from ebr\n");
		else
			vty_out(vty,"error %d\n",ret);

		result = CMD_WARNING;
	
	}
	return result;
}

int wid_no_interface_ifname_ebr(char *ptr,struct vty *vty)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	int ret = 0;
	int result = 0;
	unsigned char ID = 0;
	//unsigned char policy = 0;
	int isAdd = 0;	
	unsigned int EBRID = 0;
	//char *name = NULL;
	char *name_d = "0";	
	int local = 1;
	int slotid =0;
	char *id = (char *)malloc(sizeof(char)*25);
	memset(id,0,25);
	if(id == NULL)
	{
		vty_out(vty,"malloc error\n");
		return MALLOC_ERROR;
	}

	memcpy(id,ptr+3,(strlen(ptr)-3));
	
	ret = parse_char_ID(id,&ID);
		
	if (ret != WID_DBUS_SUCCESS) 
	{
	   if(ret == WID_ILLEGAL_INPUT){
        	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
        }
		else{
			vty_out(vty,"input id invalid\n");
		}
		WID_IF_FREE_OBJECT(id);
		return CMD_WARNING;
	}
		
	if ((ID < 1) || (ID > EBR_NUM))
	{
		WID_IF_FREE_OBJECT(id);
		vty_out(vty,"input id invalid\n");
		return CMD_WARNING;
	}
	EBRID = (unsigned)ID;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
		slotid = vty->slotindex;
		local = 0;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = vty->index;
		slotid = vty->slotindex;
		local = 1;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slotid,distributFag);
	ReInitDbusPath_V2(local,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(local,index,WID_DBUS_EBR_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(local,index,WID_DBUS_EBR_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_EBR_METHOD_ADD_DEL_EBR);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_EBR_OBJPATH,\
						WID_DBUS_EBR_INTERFACE,WID_DBUS_EBR_METHOD_ADD_DEL_EBR);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&isAdd,								
						DBUS_TYPE_UINT32,&EBRID,
						DBUS_TYPE_STRING,&name_d,							 
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
	sleep(1);
	
	if(ret == 0)
		{			
			result = CMD_SUCCESS;
		}
	else
		{
			if(ret == WID_EBR_NOT_EXIST)
				vty_out(vty,"<error> ebr id does not exist\n");
			else if(ret == SYSTEM_CMD_ERROR)
				vty_out(vty,"<error> system cmd error\n");
			else if(ret == WID_EBR_SHOULD_BE_DISABLE)			
				vty_out(vty,"<error> ebr is enable,please disable it first\n");
			else
				vty_out(vty,"<error>  %d\n",ret);

			result = CMD_WARNING;
		}


	return result;	
}

int dcli_forward_mode_config(struct vty *vty,char * ifName,unsigned int ismode)
{

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	int ret = 0;
	int result = 0;
	char ID = 0;
	int wtpid = 0;
	int wlanid = 0;
	int radioid = 0;
	int g_radioid = 0;
	unsigned int mode = ismode;

	char *id = (char *)malloc(sizeof(char)*25);
	memset(id,0,25);
	if(id == NULL)
	{
		return MALLOC_ERROR;
	}
	memcpy(id,ifName+5,(strlen(ifName)-5));
	ret = parse_radio_ifname(id,&wtpid,&radioid,&wlanid);
	
	/*printf("005 wtpid:%d,radioid:%d,wlanid:%d\n",wtpid,radioid,wlanid);*/
	
	if (ret != WID_DBUS_SUCCESS) 
	{
		WID_IF_FREE_OBJECT(id);
		
		return CMD_WARNING;
	}
	
	int local = 1;
	int slotid = 0;
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == INTERFACE_NODE){
		index = 0;
		slotid = vty->slotindex;
		local = vty->local;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slotid,distributFag);
	ReInitDbusPath_V2(local,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(local,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(local,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_FORMARD_MODE);
	/*
	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
					WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_FORMARD_MODE);
	*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&wlanid,
							 DBUS_TYPE_UINT32,&ismode,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_UINT32,&radioid,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	WID_IF_FREE_OBJECT(id);

	dbus_message_unref(query);
		
	if (NULL == reply)
	{
		return CMD_WARNING;
	}
		
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
	sleep(1);

	if(ret == 0)/*interface success*/
	{
		result = CMD_SUCCESS;
	}
	else 
	{
		if(ret == MALLOC_ERROR)
			vty_out(vty,"malloc error\n");
		else if(ret == CMD_WARNING)
			vty_out(vty,"input id is invalid\n");
		else if(ret == WTP_ID_NOT_EXIST)
			vty_out(vty,"WTP ID is not exist\n");
		else if(ret == WLAN_ID_NOT_EXIST)
			vty_out(vty,"Wlan ID is not exist\n");
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"RADIO ID is not exist\n");
		else if(ret == BSS_NOT_EXIST)
			vty_out(vty,"BSS is not exist\n");
		else if(ret == WTP_IS_NOT_BINDING_WLAN_ID)
			vty_out(vty,"WTP is not binding wlan,binding it first\n");
		else if(ret == WTP_WLAN_BINDING_NOT_MATCH)
			vty_out(vty,"WTP binding WLAN is not match,check again\n");
		else if(ret == BSS_BE_ENABLE)
			vty_out(vty,"BSS is enable, if you want to operate this, please disable it first\n");
		else if(ret == IF_POLICY_CONFLICT)
			vty_out(vty,"WLAN is no_interface , BSS can not be wlan_interface\n");
		else if(ret == BSS_CREATE_L3_INTERFACE_FAIL)
			vty_out(vty,"BSS create l3 interface fail\n"); 
		else if(ret == BSS_DELETE_L3_INTERFACE_FAIL)
			vty_out(vty,"BSS delete l3 interface fail\n");
		else if(ret == WLAN_CREATE_BR_FAIL)
			vty_out(vty,"WLAN bridge interface fail\n"); 
		else if(ret == WLAN_DELETE_BR_FAIL)
			vty_out(vty,"WLAN delete l3 interface fail\n");
		else if(ret == BSS_L3_INTERFACE_ADD_BR_FAIL)
			vty_out(vty,"BSS add BR interface fail\n"); 
		else if(ret == BSS_L3_INTERFACE_DEL_BR_FAIL)
			vty_out(vty,"BSS delete BR interface fail\n");
		else
			vty_out(vty,"error %d\n",ret);

		result = CMD_WARNING;
	
	}
	return result;	
}

int dcli_tunnel_mode_config(struct vty *vty,char * ifName,unsigned int ismode,unsigned int modeflag,char nodeFlag,unsigned int wlanID)
{

	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter  iter;
	int ret = 0;
	int result = 0;
	char ID = 0;
	int wtpid = 0;
	int wlanid = 0;
	int radioid = 0;
	int g_radioid = 0;
	unsigned int mode = ismode;
	int vvrrid = 0;
	int index = 0;
	int local = 1;
	int slotid = 0;
	char *id = (char *)malloc(sizeof(char)*25);
	memset(id,0,25);
	if(id == NULL)
	{
		return MALLOC_ERROR;
	}
	if(0 == nodeFlag){//interface wlan or radio node
		if(1 == modeflag){
			memcpy(id,ifName+5,(strlen(ifName)-5));
			ret = parse_radio_ifname(id,&wtpid,&radioid,&wlanid);
			/*printf("wtpid:%d,radioid:%d,wlanid:%d\n",wtpid,radioid,wlanid);*/
			if(ret != WID_DBUS_SUCCESS){
				memcpy(id,ifName+1,(strlen(ifName)-1));
				ret = parse_radio_ifname_v2(id,&wtpid,&radioid,&wlanid,&vvrrid);
			}
			if (ret != WID_DBUS_SUCCESS) 
			{
				WID_IF_FREE_OBJECT(id);
				
				return CMD_WARNING;
			}
		}else{
			memcpy(id,ifName+4,(strlen(ifName)-4));
			ret = parse_wlan_ifname_v2(id,&wlanid,&vvrrid);
		}
		
		index = vvrrid;
	}else{ // wlan or radio node
		if(vty->node == HANSI_WLAN_NODE){
			index = vty->index;
			wlanid = (int)vty->index_sub;	
			radioid = 0;//not use set to 0
			slotid = vty->slotindex;
			local = 0;
		}else if(vty->node == LOCAL_HANSI_WLAN_NODE){
			index = vty->index;
			wlanid = (int)vty->index_sub;	
			radioid = 0;//not use set to 0
			slotid = vty->slotindex;
			local = 1;
		}else if(vty->node == HANSI_RADIO_NODE){
			index = vty->index; 		
			g_radioid = (int)vty->index_sub;
			radioid = g_radioid%L_RADIO_NUM;
			wtpid = g_radioid/L_RADIO_NUM;
			wlanid = (int)wlanID;
			slotid = vty->slotindex;
			local = 0;
		}else if(vty->node == LOCAL_HANSI_RADIO_NODE){
			index = vty->index; 		
			g_radioid = (int)vty->index_sub;
			radioid = g_radioid%L_RADIO_NUM;
			wtpid = g_radioid/L_RADIO_NUM;
			wlanid = (int)wlanID;
			slotid = vty->slotindex;
			local = 1;
		}else if(vty->node == HANSI_NODE){
				index = vty->index;
				wlanid = 0;	
				radioid = 0;
				slotid = vty->slotindex;
				local = 0;
		}else if(vty->node == LOCAL_HANSI_NODE){
				index = vty->index;
				wlanid = 0;	
				radioid = 0;
				slotid = vty->slotindex;
				local = 1;
		}else {
			vty_out(vty,"err node.\n");
		}
	}
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == INTERFACE_NODE){
		index = 0;
		slotid = vty->slotindex;
		local = vty->local;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slotid,distributFag);
	ReInitDbusPath_V2(local,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(local,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(local,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_TUNNEL_MODE);
	/*
	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
					WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_TUNNEL_MODE);
	*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&modeflag,
							 DBUS_TYPE_UINT32,&wlanid,
							 DBUS_TYPE_UINT32,&ismode,
							 DBUS_TYPE_UINT32,&wtpid,
							 DBUS_TYPE_UINT32,&radioid,
							 DBUS_TYPE_BYTE,&nodeFlag,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	WID_IF_FREE_OBJECT(id);

	dbus_message_unref(query);
		
	if (NULL == reply)
	{
		return CMD_WARNING;
	}
		
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
	sleep(1);

	if(ret == 0)/*interface success*/
	{
		result = CMD_SUCCESS;
	}
	else 
	{
		if(ret == MALLOC_ERROR)
			vty_out(vty,"malloc error\n");
		else if(ret == CMD_WARNING)
			vty_out(vty,"input id is invalid\n");
		else if(ret == WTP_ID_NOT_EXIST)
			vty_out(vty,"WTP ID is not exist\n");
		else if(ret == WLAN_ID_NOT_EXIST)
			vty_out(vty,"Wlan ID is not exist\n");
		else if(ret == RADIO_ID_NOT_EXIST)
			vty_out(vty,"RADIO ID is not exist\n");
		else if(ret == BSS_NOT_EXIST)
			vty_out(vty,"BSS is not exist\n");
		else if(ret == WTP_IS_NOT_BINDING_WLAN_ID)
			vty_out(vty,"WTP is not binding wlan,binding it first\n");
		else if(ret == WTP_WLAN_BINDING_NOT_MATCH)
			vty_out(vty,"WTP binding WLAN is not match,check again\n");
		else if(ret == BSS_BE_ENABLE)
			vty_out(vty,"BSS is enable, if you want to operate this, please disable it first\n");
		else if(ret == IF_POLICY_CONFLICT)
			vty_out(vty,"WLAN is no_interface , BSS can not be wlan_interface\n");
		else if(ret == BSS_CREATE_L3_INTERFACE_FAIL)
			vty_out(vty,"BSS create l3 interface fail\n"); 
		else if(ret == BSS_DELETE_L3_INTERFACE_FAIL)
			vty_out(vty,"BSS delete l3 interface fail\n");
		else if(ret == WLAN_CREATE_BR_FAIL)
			vty_out(vty,"WLAN create l3 interface fail\n"); 
		else if(ret == WLAN_DELETE_BR_FAIL)
			vty_out(vty,"WLAN delete l3 interface fail\n");
		else if(ret == BSS_L3_INTERFACE_ADD_BR_FAIL)
			vty_out(vty,"BSS add BR interface fail\n"); 
		else if(ret == BSS_L3_INTERFACE_DEL_BR_FAIL)
			vty_out(vty,"BSS delete BR interface fail\n");
		else if(ret == NO_SURPPORT_IPIP)
			vty_out(vty,"bridge mode cann't config ipip, change forward mode route first\n");
		else if(ret == WLAN_BE_ENABLE)
			vty_out(vty,"wlan is enable ,disable it first.\n");
		else if(ret == BSS_IF_NEED_CREATE)
			vty_out(vty,"radio%d-%d.%d if is not exsit.",wtpid,radioid,wlanid);
		else if(ret == INTERFACE_NOT_L3_IF)
			vty_out(vty,"wlan %d is not L3 interface.",wlanid);
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
			vty_out(vty, "<warning> you want to some wlan, and the operation of the wlan was not successful\n");
		else
			vty_out(vty,"error %d\n",ret);

		result = CMD_WARNING;
	
	}
	return result;	
}

/*qos area*/
DEFUN(wid_create_qos_profile_cmd_func,
		wid_create_qos_profile_cmd,
		"create wireless qos profile ID PROFILENAME",
		"Configuration service\n"
		"wireless service\n"
		"qos service\n"
		"qos service\n"
		"assign qos profile ID for qos profile\n"
		"assign qos profile NAME\n"
	)
{
	int ret,len;
	int isAdd = 1;	
	int qos_id = 0;
	char *name = NULL;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	isAdd = 1;			

	ret = parse_int_ID((char*)argv[0], &qos_id);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}
	
	if(qos_id >= QOS_NUM || qos_id <= 0){
		vty_out(vty,"<error> qos id should be 1 to %d\n",QOS_NUM-1);      //fengwenchao modify 20110427
		return CMD_SUCCESS;
	}
	
	len = strlen(argv[1]);
	if(len > 15){
		vty_out(vty,"<error> qos name is too long,it should be 1 to 15 \n");
		return CMD_SUCCESS;
	}
	name = (char*)malloc(strlen(argv[1])+1);
	memset(name, 0, strlen(argv[1])+1);
	memcpy(name, argv[1], strlen(argv[1]));	
	
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		 index = vty->index;
		 localid = vty->local;
		 slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
	    index = vty->index;
	    localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_INTERFACE,INTERFACE); 
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_ADD_DEL_QOS);

		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_ADD_DEL_QOS);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&isAdd,								
						DBUS_TYPE_UINT32,&qos_id,
						DBUS_TYPE_STRING,&name,							 
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		if(name)
		{
			free(name);
			name = NULL;
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == 0)
			vty_out(vty,"qos profile %d was successfully created.\n",qos_id);
		else if(ret == WID_QOS_BE_USED)
			vty_out(vty,"<error> qos id exist\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	if(name)
	{
		free(name);
		name = NULL;
	}
	return CMD_SUCCESS;	
}
DEFUN(wid_delete_qos_profile_cmd_func,
		wid_delete_qos_profile_cmd,
		"delete wireless qos profile ID",
		"Configuration service\n"
		"wireless service\n"
		"qos service\n"
		"qos profile\n"
		"assign qos ID for qos profile\n"
	)
{
	int ret;
	int isAdd = 1;	
	int qos_id = 0;
	char *name = NULL;
	char *name_d = "0";
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	isAdd = 0;	
	ret = parse_int_ID((char*)argv[0], &qos_id);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(qos_id >= QOS_NUM || qos_id == 0){
		vty_out(vty,"<error> qos id should be 1 to %d\n",QOS_NUM-1);   //fengwenchao modify 20110427
		return CMD_SUCCESS;
	}

	name = name_d;
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		 index = vty->index;
		 localid = vty->local;
		 slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
	    index = vty->index;
	    localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_INTERFACE,INTERFACE); 
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_ADD_DEL_QOS);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_ADD_DEL_QOS);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&isAdd,								
						DBUS_TYPE_UINT32,&qos_id,
						DBUS_TYPE_STRING,&name,							 
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

		if(ret == 0)
			vty_out(vty,"qos profile %d was successfully deleted.\n",qos_id);
		else if(ret == WID_QOS_NOT_EXIST)
			vty_out(vty,"<error> qos id does not exist\n");			
		else if(ret == WID_QOS_RADIO_SHOULD_BE_DISABLE)			
			vty_out(vty,"<error> this qos profile be used by some radios,please disable them first\n");
		else if(ret == WID_QOS_BE_USED_BY_RADIO)           //fengwenchao add 20110427
			vty_out(vty,"<error>this qos now be used by some radios,please delete them,or you can use <delete radio with wireless qos profile ID> command delete them");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;	
}
/*fengwenchao add 20110427*/
DEFUN(wid_delete_radio_with_qos_profile_cmd_func,
		wid_delete_radio_with_qos_profile_cmd,
		"delete radio with wireless qos profile ID",
		"Configuration service\n"
		"wireless service\n"
		"qos service\n"
		"qos profile\n"
		"assign qos ID for qos profile\n"
		"qos profile\n"
		"qos id <1-15>\n"
	)
{
	int ret;	
	int qos_id = 0;

    //  DBusConnection *dbus_connection = dcli_dbus_connection;

	ret = parse_int_ID((char*)argv[0], &qos_id);
	
	if(ret != WID_DBUS_SUCCESS)
	{
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}

	if(qos_id >= QOS_NUM || qos_id == 0)
	{
		vty_out(vty,"<error> qos id should be 1 to %d\n",QOS_NUM-1);
		return CMD_SUCCESS;
	}

	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		 index = vty->index;
		 localid = vty->local;
		 slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
	    index = vty->index;
	    localid = vty->local;
	    slot_id = vty->slotindex;
	}
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ret = wid_delete_radio_with_qos_profile(index,dcli_dbus_connection, qos_id,localid);

	if(ret == 0)
	{
		vty_out(vty,"qos profile %d was successfully deleted  from radios.\n",qos_id);
	}
	else if(ret == WID_QOS_NOT_EXIST)
	{
		vty_out(vty,"qos profile %d was not exist.\n",qos_id);
	}
	else
	{
		vty_out(vty,"ret = %d\n",ret);
	}

	return CMD_SUCCESS;
}
/*fengwenchao add end*/
DEFUN(wid_show_qos_profile_cmd_func,
	  wid_show_qos_profile_cmd,
	  "show wireless qos profile ID [remote] [local] [PARAMETER]",
	  SHOW_STR
	  "wireless service\n"
	  "Display qos profile information\n"
	  "qos profile\n"
	  "ID of qos profile\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{	
	int ret;
	int qos_id = 0;
	int	ID = 0;
	char *name;
	int i = 0;
	int j = 0;
	int k = 0;
	int num = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;

	//AC_QOS *qos[QOS_NUM];
//	int dcli_sn = 2;
	
/*	qos_profile *radio_qos[DCLIWQOS_QOS_FLOW_NUM];
	qos_profile *client_qos[DCLIWQOS_QOS_FLOW_NUM];
*/
	char ack[] = "ACK";
	char noack[] = "NOACK";
/*	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
*/	
	ret = parse_int_ID((char*)argv[0], &qos_id);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(qos_id >= QOS_NUM || qos_id == 0){
		vty_out(vty,"<error> qos id should be 1 to %d\n",QOS_NUM-1);
		return CMD_SUCCESS;
	}
	
/*	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}
	ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ReInitDbusPath(index,WID_DBUS_QOS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SHOW_QOS);
*/

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SHOW_QOS);*/
/*	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&qos_id,
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
	
	if(ret == 0 )
	{	
vty_out(vty,"ret == 0=========================================\n");
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&(dcli_qos->QosID));
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&name);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
vty_out(vty,"ID == %d,=============================\n",ID);
		for (i = 0; i < 4; i++) {
			DBusMessageIter iter_struct;
			
			dcli_qos->radio_qos[i] = (qos_profile *)malloc(sizeof(qos_profile));
			dcli_qos->client_qos[i] = (qos_profile *)malloc(sizeof(qos_profile));
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);

			/radio qos info/
		
			dbus_message_iter_get_basic(&iter_struct,&(dcli_qos->radio_qos[i]->QueueDepth));
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(dcli_qos->radio_qos[i]->CWMin));
		
			dbus_message_iter_next(&iter_struct);
	
			dbus_message_iter_get_basic(&iter_struct,&(dcli_qos->radio_qos[i]->CWMax));
		
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(dcli_qos->radio_qos[i]->AIFS));
					
			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(dcli_qos->radio_qos[i]->TXOPlimit));
			/
			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(radio_qos[i]->Dot1PTag));
					
			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(radio_qos[i]->DSCPTag));
			/		
			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(dcli_qos->radio_qos[i]->ACK));

			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(dcli_qos->radio_qos[i]->mapstate));

			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(dcli_qos->radio_qos[i]->wmm_map_dot1p));

			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(dcli_qos->radio_qos[i]->dot1p_map_wmm_num));

			for(j=0;j<8;j++)
			{
				dbus_message_iter_next(&iter_struct);
					
				dbus_message_iter_get_basic(&iter_struct,&(dcli_qos->radio_qos[i]->dot1p_map_wmm[j]));

			}

			

			/client qos info/
	
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(dcli_qos->client_qos[i]->QueueDepth));
					
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(dcli_qos->client_qos[i]->CWMin));
		
			dbus_message_iter_next(&iter_struct);
	
			dbus_message_iter_get_basic(&iter_struct,&(dcli_qos->client_qos[i]->CWMax));
		
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(dcli_qos->client_qos[i]->AIFS));
					
			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(dcli_qos->client_qos[i]->TXOPlimit));
			/
			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(client_qos[i]->Dot1PTag));
					
			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(client_qos[i]->DSCPTag));
					
			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(client_qos[i]->ACK));
			/
			dbus_message_iter_next(&iter_array);
		}
	}
*/	
	DCLI_WQOS *WQOS = NULL;
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
		
//	DBusMessage *reply; 
//	DBusError err;
		
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		 index = vty->index;
		 localid = vty->local;
		 slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
	    index = vty->index;
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
	
	if(vty->node != VIEW_NODE){
		WQOS = (DCLI_WQOS*)dcli_wqos_wireless_qos_show_config_info
			(
				index,/*"show wireless qos profile ID"*/
				&ret,
				localid,
				qos_id,			
				0,
			//	qos,
				dcli_dbus_connection,
				WID_DBUS_QOS_METHOD_SHOW_QOS
			);
		if(ret == -1){
			cli_syslog_info("<error> failed get reply.\n");
		}
		else if(ret == 0){
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"Wireless Qos profile %d infomation\n",WQOS->qos[0]->QosID);
			vty_out(vty,"Wireless Qos profile name: %s\n",WQOS->qos[0]->name);
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"radio qos infomation\n");
			vty_out(vty,"             QueueDepth CWMin CWMax AIFS TXOPlimit    ACK\n");

			vty_out(vty,"BESTEFFORT:  %10d %5d %5d %4d %9d %6s\n",WQOS->qos[0]->radio_qos[0]->QueueDepth
																,WQOS->qos[0]->radio_qos[0]->CWMin
																,WQOS->qos[0]->radio_qos[0]->CWMax
																,WQOS->qos[0]->radio_qos[0]->AIFS
																,WQOS->qos[0]->radio_qos[0]->TXOPlimit
																,(WQOS->qos[0]->radio_qos[0]->ACK == 1)?ack:noack);
				
			vty_out(vty,"BACKGROUND:  %10d %5d %5d %4d %9d %6s\n",WQOS->qos[0]->radio_qos[1]->QueueDepth
																,WQOS->qos[0]->radio_qos[1]->CWMin
																,WQOS->qos[0]->radio_qos[1]->CWMax
																,WQOS->qos[0]->radio_qos[1]->AIFS
																,WQOS->qos[0]->radio_qos[1]->TXOPlimit
																,(WQOS->qos[0]->radio_qos[1]->ACK == 1)?ack:noack);

			vty_out(vty,"VIDEO:       %10d %5d %5d %4d %9d %6s\n",WQOS->qos[0]->radio_qos[2]->QueueDepth
																,WQOS->qos[0]->radio_qos[2]->CWMin
																,WQOS->qos[0]->radio_qos[2]->CWMax
																,WQOS->qos[0]->radio_qos[2]->AIFS
																,WQOS->qos[0]->radio_qos[2]->TXOPlimit
																,(WQOS->qos[0]->radio_qos[2]->ACK == 1)?ack:noack);

			vty_out(vty,"VOICE:       %10d %5d %5d %4d %9d %6s\n",WQOS->qos[0]->radio_qos[3]->QueueDepth
																,WQOS->qos[0]->radio_qos[3]->CWMin
																,WQOS->qos[0]->radio_qos[3]->CWMax
																,WQOS->qos[0]->radio_qos[3]->AIFS
																,WQOS->qos[0]->radio_qos[3]->TXOPlimit
																,(WQOS->qos[0]->radio_qos[3]->ACK == 1)?ack:noack);
			/*
			for(i=0;i<4;i++)
			{
				vty_out(vty,"Radio:  %10d %5d %5d %4d %9d %6s\n",radio_qos[i]->QueueDepth
																,radio_qos[i]->CWMin
																,radio_qos[i]->CWMax
																,radio_qos[i]->AIFS
																,radio_qos[i]->TXOPlimit
																,(radio_qos[i]->ACK == 1)?ack:noack);
					
				
			}*/
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"client qos infomation\n");
			vty_out(vty,"             QueueDepth CWMin CWMax AIFS TXOPlimit\n");
			
			vty_out(vty,"BESTEFFORT:  %10d %5d %5d %4d %9d\n",WQOS->qos[0]->client_qos[0]->QueueDepth
																,WQOS->qos[0]->client_qos[0]->CWMin
																,WQOS->qos[0]->client_qos[0]->CWMax
																,WQOS->qos[0]->client_qos[0]->AIFS
																,WQOS->qos[0]->client_qos[0]->TXOPlimit);
				
			vty_out(vty,"BACKGROUND:  %10d %5d %5d %4d %9d\n",WQOS->qos[0]->client_qos[1]->QueueDepth
																,WQOS->qos[0]->client_qos[1]->CWMin
																,WQOS->qos[0]->client_qos[1]->CWMax
																,WQOS->qos[0]->client_qos[1]->AIFS
																,WQOS->qos[0]->client_qos[1]->TXOPlimit);

			vty_out(vty,"VIDEO:       %10d %5d %5d %4d %9d\n",WQOS->qos[0]->client_qos[2]->QueueDepth
																,WQOS->qos[0]->client_qos[2]->CWMin
																,WQOS->qos[0]->client_qos[2]->CWMax
																,WQOS->qos[0]->client_qos[2]->AIFS
																,WQOS->qos[0]->client_qos[2]->TXOPlimit);

			vty_out(vty,"VOICE:       %10d %5d %5d %4d %9d\n",WQOS->qos[0]->client_qos[3]->QueueDepth
																,WQOS->qos[0]->client_qos[3]->CWMin
																,WQOS->qos[0]->client_qos[3]->CWMax
																,WQOS->qos[0]->client_qos[3]->AIFS
																,WQOS->qos[0]->client_qos[3]->TXOPlimit);
			
			/*
			for(i=0;i<4;i++)
			{
				vty_out(vty,"Client: %10d %5d %5d %4d %9d\n",client_qos[i]->QueueDepth
															,client_qos[i]->CWMin
															,client_qos[i]->CWMax
															,client_qos[i]->AIFS
															,client_qos[i]->TXOPlimit);
					
				free(client_qos[i]);
				client_qos[i] = NULL ;
			}*/
			
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"WMM map infomation\n");
			vty_out(vty,"\n");
			if(WQOS->qos[0]->radio_qos[0]->mapstate == 1)
			{
				vty_out(vty,"WMM BESTEFFORT map dot1p: %d\n",WQOS->qos[0]->radio_qos[0]->wmm_map_dot1p);
				
			}
			if(WQOS->qos[0]->radio_qos[1]->mapstate == 1)
			{
				vty_out(vty,"WMM BACKGROUND map dot1p: %d\n",WQOS->qos[0]->radio_qos[1]->wmm_map_dot1p);
				
			}
			if(WQOS->qos[0]->radio_qos[2]->mapstate == 1)
			{
				vty_out(vty,"WMM VIDEO map dot1p: %d\n",WQOS->qos[0]->radio_qos[2]->wmm_map_dot1p);
				
			}
			if(WQOS->qos[0]->radio_qos[3]->mapstate == 1)
			{
				vty_out(vty,"WMM VOICE map dot1p: %d\n",WQOS->qos[0]->radio_qos[3]->wmm_map_dot1p);
				
			}
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"Dot1p map infomation\n");
			vty_out(vty,"\n");
			
			for(j=0;j<WQOS->qos[0]->radio_qos[0]->dot1p_map_wmm_num;j++)
			{
				vty_out(vty,"Dot1p %d map BESTERRORT\n",WQOS->qos[0]->radio_qos[0]->dot1p_map_wmm[j]);
			}
			
			if(WQOS->qos[0]->radio_qos[1]->dot1p_map_wmm_num != 0)
			{
				for(j=0;j<WQOS->qos[0]->radio_qos[1]->dot1p_map_wmm_num;j++)
				{
					vty_out(vty,"Dot1p %d map BACKGROUND\n",WQOS->qos[0]->radio_qos[1]->dot1p_map_wmm[j]);
				}
			}
			if(WQOS->qos[0]->radio_qos[2]->dot1p_map_wmm_num != 0)
			{
				for(j=0;j<WQOS->qos[0]->radio_qos[2]->dot1p_map_wmm_num;j++)
				{
					vty_out(vty,"Dot1p %d map VIDEO\n",WQOS->qos[0]->radio_qos[2]->dot1p_map_wmm[j]);
				}
			}
			if(WQOS->qos[0]->radio_qos[3]->dot1p_map_wmm_num != 0)
			{
				for(j=0;j<WQOS->qos[0]->radio_qos[3]->dot1p_map_wmm_num;j++)
				{
					vty_out(vty,"Dot1p %d map VOICE\n",WQOS->qos[0]->radio_qos[3]->dot1p_map_wmm[j]);
				}
			}
			vty_out(vty,"==============================================================================\n");
			dcli_wqos_free_fun(WID_DBUS_QOS_METHOD_SHOW_QOS,WQOS);
		}
		else if(ret == WTP_ID_NOT_EXIST)
		{
			vty_out(vty,"<error> qos id does not exist\n");
		}
		else 
		{	
			vty_out(vty,"<error>  %d\n",ret);
		}
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
				WQOS = (DCLI_WQOS*)dcli_wqos_wireless_qos_show_config_info
					(
						profile,/*"show wireless qos profile ID"*/
						&ret,
						localid,
						qos_id, 		
						0,
						dcli_dbus_connection,
						WID_DBUS_QOS_METHOD_SHOW_QOS
					);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0){
					vty_out(vty,"========================================================================\n");
					vty_out(vty,"Wireless Qos profile %d infomation\n",WQOS->qos[0]->QosID);
					vty_out(vty,"Wireless Qos profile name: %s\n",WQOS->qos[0]->name);
					vty_out(vty,"========================================================================\n");
					vty_out(vty,"radio qos infomation\n");
					vty_out(vty,"			  QueueDepth CWMin CWMax AIFS TXOPlimit    ACK\n");
			
					vty_out(vty,"BESTEFFORT:  %10d %5d %5d %4d %9d %6s\n",WQOS->qos[0]->radio_qos[0]->QueueDepth
																		,WQOS->qos[0]->radio_qos[0]->CWMin
																		,WQOS->qos[0]->radio_qos[0]->CWMax
																		,WQOS->qos[0]->radio_qos[0]->AIFS
																		,WQOS->qos[0]->radio_qos[0]->TXOPlimit
																		,(WQOS->qos[0]->radio_qos[0]->ACK == 1)?ack:noack);
						
					vty_out(vty,"BACKGROUND:  %10d %5d %5d %4d %9d %6s\n",WQOS->qos[0]->radio_qos[1]->QueueDepth
																		,WQOS->qos[0]->radio_qos[1]->CWMin
																		,WQOS->qos[0]->radio_qos[1]->CWMax
																		,WQOS->qos[0]->radio_qos[1]->AIFS
																		,WQOS->qos[0]->radio_qos[1]->TXOPlimit
																		,(WQOS->qos[0]->radio_qos[1]->ACK == 1)?ack:noack);
			
					vty_out(vty,"VIDEO: 	  %10d %5d %5d %4d %9d %6s\n",WQOS->qos[0]->radio_qos[2]->QueueDepth
																		,WQOS->qos[0]->radio_qos[2]->CWMin
																		,WQOS->qos[0]->radio_qos[2]->CWMax
																		,WQOS->qos[0]->radio_qos[2]->AIFS
																		,WQOS->qos[0]->radio_qos[2]->TXOPlimit
																		,(WQOS->qos[0]->radio_qos[2]->ACK == 1)?ack:noack);
			
					vty_out(vty,"VOICE: 	  %10d %5d %5d %4d %9d %6s\n",WQOS->qos[0]->radio_qos[3]->QueueDepth
																		,WQOS->qos[0]->radio_qos[3]->CWMin
																		,WQOS->qos[0]->radio_qos[3]->CWMax
																		,WQOS->qos[0]->radio_qos[3]->AIFS
																		,WQOS->qos[0]->radio_qos[3]->TXOPlimit
																		,(WQOS->qos[0]->radio_qos[3]->ACK == 1)?ack:noack);
					vty_out(vty,"==============================================================================\n");
					vty_out(vty,"client qos infomation\n");
					vty_out(vty,"			  QueueDepth CWMin CWMax AIFS TXOPlimit\n");
					
					vty_out(vty,"BESTEFFORT:  %10d %5d %5d %4d %9d\n",WQOS->qos[0]->client_qos[0]->QueueDepth
																		,WQOS->qos[0]->client_qos[0]->CWMin
																		,WQOS->qos[0]->client_qos[0]->CWMax
																		,WQOS->qos[0]->client_qos[0]->AIFS
																		,WQOS->qos[0]->client_qos[0]->TXOPlimit);
						
					vty_out(vty,"BACKGROUND:  %10d %5d %5d %4d %9d\n",WQOS->qos[0]->client_qos[1]->QueueDepth
																		,WQOS->qos[0]->client_qos[1]->CWMin
																		,WQOS->qos[0]->client_qos[1]->CWMax
																		,WQOS->qos[0]->client_qos[1]->AIFS
																		,WQOS->qos[0]->client_qos[1]->TXOPlimit);
			
					vty_out(vty,"VIDEO: 	  %10d %5d %5d %4d %9d\n",WQOS->qos[0]->client_qos[2]->QueueDepth
																		,WQOS->qos[0]->client_qos[2]->CWMin
																		,WQOS->qos[0]->client_qos[2]->CWMax
																		,WQOS->qos[0]->client_qos[2]->AIFS
																		,WQOS->qos[0]->client_qos[2]->TXOPlimit);
			
					vty_out(vty,"VOICE: 	  %10d %5d %5d %4d %9d\n",WQOS->qos[0]->client_qos[3]->QueueDepth
																		,WQOS->qos[0]->client_qos[3]->CWMin
																		,WQOS->qos[0]->client_qos[3]->CWMax
																		,WQOS->qos[0]->client_qos[3]->AIFS
																		,WQOS->qos[0]->client_qos[3]->TXOPlimit);
					
					
					vty_out(vty,"==============================================================================\n");
					vty_out(vty,"WMM map infomation\n");
					vty_out(vty,"\n");
					if(WQOS->qos[0]->radio_qos[0]->mapstate == 1)
					{
						vty_out(vty,"WMM BESTEFFORT map dot1p: %d\n",WQOS->qos[0]->radio_qos[0]->wmm_map_dot1p);
						
					}
					if(WQOS->qos[0]->radio_qos[1]->mapstate == 1)
					{
						vty_out(vty,"WMM BACKGROUND map dot1p: %d\n",WQOS->qos[0]->radio_qos[1]->wmm_map_dot1p);
						
					}
					if(WQOS->qos[0]->radio_qos[2]->mapstate == 1)
					{
						vty_out(vty,"WMM VIDEO map dot1p: %d\n",WQOS->qos[0]->radio_qos[2]->wmm_map_dot1p);
						
					}
					if(WQOS->qos[0]->radio_qos[3]->mapstate == 1)
					{
						vty_out(vty,"WMM VOICE map dot1p: %d\n",WQOS->qos[0]->radio_qos[3]->wmm_map_dot1p);
						
					}
					vty_out(vty,"==============================================================================\n");
					vty_out(vty,"Dot1p map infomation\n");
					vty_out(vty,"\n");
					
					for(j=0;j<WQOS->qos[0]->radio_qos[0]->dot1p_map_wmm_num;j++)
					{
						vty_out(vty,"Dot1p %d map BESTERRORT\n",WQOS->qos[0]->radio_qos[0]->dot1p_map_wmm[j]);
					}
					
					if(WQOS->qos[0]->radio_qos[1]->dot1p_map_wmm_num != 0)
					{
						for(j=0;j<WQOS->qos[0]->radio_qos[1]->dot1p_map_wmm_num;j++)
						{
							vty_out(vty,"Dot1p %d map BACKGROUND\n",WQOS->qos[0]->radio_qos[1]->dot1p_map_wmm[j]);
						}
					}
					if(WQOS->qos[0]->radio_qos[2]->dot1p_map_wmm_num != 0)
					{
						for(j=0;j<WQOS->qos[0]->radio_qos[2]->dot1p_map_wmm_num;j++)
						{
							vty_out(vty,"Dot1p %d map VIDEO\n",WQOS->qos[0]->radio_qos[2]->dot1p_map_wmm[j]);
						}
					}
					if(WQOS->qos[0]->radio_qos[3]->dot1p_map_wmm_num != 0)
					{
						for(j=0;j<WQOS->qos[0]->radio_qos[3]->dot1p_map_wmm_num;j++)
						{
							vty_out(vty,"Dot1p %d map VOICE\n",WQOS->qos[0]->radio_qos[3]->dot1p_map_wmm[j]);
						}
					}
					dcli_wqos_free_fun(WID_DBUS_QOS_METHOD_SHOW_QOS,WQOS);
				}
				else if(ret == WTP_ID_NOT_EXIST)
				{
					vty_out(vty,"<error> qos id does not exist\n");
				}
				else 
				{	
					vty_out(vty,"<error>  %d\n",ret);
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
				WQOS = (DCLI_WQOS*)dcli_wqos_wireless_qos_show_config_info
					(
						profile,/*"show wireless qos profile ID"*/
						&ret,
						localid,
						qos_id, 		
						0,
						dcli_dbus_connection,
						WID_DBUS_QOS_METHOD_SHOW_QOS
					);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0){
					vty_out(vty,"========================================================================\n");
					vty_out(vty,"Wireless Qos profile %d infomation\n",WQOS->qos[0]->QosID);
					vty_out(vty,"Wireless Qos profile name: %s\n",WQOS->qos[0]->name);
					vty_out(vty,"========================================================================\n");
					vty_out(vty,"radio qos infomation\n");
					vty_out(vty,"			  QueueDepth CWMin CWMax AIFS TXOPlimit    ACK\n");
			
					vty_out(vty,"BESTEFFORT:  %10d %5d %5d %4d %9d %6s\n",WQOS->qos[0]->radio_qos[0]->QueueDepth
																		,WQOS->qos[0]->radio_qos[0]->CWMin
																		,WQOS->qos[0]->radio_qos[0]->CWMax
																		,WQOS->qos[0]->radio_qos[0]->AIFS
																		,WQOS->qos[0]->radio_qos[0]->TXOPlimit
																		,(WQOS->qos[0]->radio_qos[0]->ACK == 1)?ack:noack);
						
					vty_out(vty,"BACKGROUND:  %10d %5d %5d %4d %9d %6s\n",WQOS->qos[0]->radio_qos[1]->QueueDepth
																		,WQOS->qos[0]->radio_qos[1]->CWMin
																		,WQOS->qos[0]->radio_qos[1]->CWMax
																		,WQOS->qos[0]->radio_qos[1]->AIFS
																		,WQOS->qos[0]->radio_qos[1]->TXOPlimit
																		,(WQOS->qos[0]->radio_qos[1]->ACK == 1)?ack:noack);
			
					vty_out(vty,"VIDEO: 	  %10d %5d %5d %4d %9d %6s\n",WQOS->qos[0]->radio_qos[2]->QueueDepth
																		,WQOS->qos[0]->radio_qos[2]->CWMin
																		,WQOS->qos[0]->radio_qos[2]->CWMax
																		,WQOS->qos[0]->radio_qos[2]->AIFS
																		,WQOS->qos[0]->radio_qos[2]->TXOPlimit
																		,(WQOS->qos[0]->radio_qos[2]->ACK == 1)?ack:noack);
			
					vty_out(vty,"VOICE: 	  %10d %5d %5d %4d %9d %6s\n",WQOS->qos[0]->radio_qos[3]->QueueDepth
																		,WQOS->qos[0]->radio_qos[3]->CWMin
																		,WQOS->qos[0]->radio_qos[3]->CWMax
																		,WQOS->qos[0]->radio_qos[3]->AIFS
																		,WQOS->qos[0]->radio_qos[3]->TXOPlimit
																		,(WQOS->qos[0]->radio_qos[3]->ACK == 1)?ack:noack);
					vty_out(vty,"==============================================================================\n");
					vty_out(vty,"client qos infomation\n");
					vty_out(vty,"			  QueueDepth CWMin CWMax AIFS TXOPlimit\n");
					
					vty_out(vty,"BESTEFFORT:  %10d %5d %5d %4d %9d\n",WQOS->qos[0]->client_qos[0]->QueueDepth
																		,WQOS->qos[0]->client_qos[0]->CWMin
																		,WQOS->qos[0]->client_qos[0]->CWMax
																		,WQOS->qos[0]->client_qos[0]->AIFS
																		,WQOS->qos[0]->client_qos[0]->TXOPlimit);
						
					vty_out(vty,"BACKGROUND:  %10d %5d %5d %4d %9d\n",WQOS->qos[0]->client_qos[1]->QueueDepth
																		,WQOS->qos[0]->client_qos[1]->CWMin
																		,WQOS->qos[0]->client_qos[1]->CWMax
																		,WQOS->qos[0]->client_qos[1]->AIFS
																		,WQOS->qos[0]->client_qos[1]->TXOPlimit);
			
					vty_out(vty,"VIDEO: 	  %10d %5d %5d %4d %9d\n",WQOS->qos[0]->client_qos[2]->QueueDepth
																		,WQOS->qos[0]->client_qos[2]->CWMin
																		,WQOS->qos[0]->client_qos[2]->CWMax
																		,WQOS->qos[0]->client_qos[2]->AIFS
																		,WQOS->qos[0]->client_qos[2]->TXOPlimit);
			
					vty_out(vty,"VOICE: 	  %10d %5d %5d %4d %9d\n",WQOS->qos[0]->client_qos[3]->QueueDepth
																		,WQOS->qos[0]->client_qos[3]->CWMin
																		,WQOS->qos[0]->client_qos[3]->CWMax
																		,WQOS->qos[0]->client_qos[3]->AIFS
																		,WQOS->qos[0]->client_qos[3]->TXOPlimit);
					
					
					vty_out(vty,"==============================================================================\n");
					vty_out(vty,"WMM map infomation\n");
					vty_out(vty,"\n");
					if(WQOS->qos[0]->radio_qos[0]->mapstate == 1)
					{
						vty_out(vty,"WMM BESTEFFORT map dot1p: %d\n",WQOS->qos[0]->radio_qos[0]->wmm_map_dot1p);
						
					}
					if(WQOS->qos[0]->radio_qos[1]->mapstate == 1)
					{
						vty_out(vty,"WMM BACKGROUND map dot1p: %d\n",WQOS->qos[0]->radio_qos[1]->wmm_map_dot1p);
						
					}
					if(WQOS->qos[0]->radio_qos[2]->mapstate == 1)
					{
						vty_out(vty,"WMM VIDEO map dot1p: %d\n",WQOS->qos[0]->radio_qos[2]->wmm_map_dot1p);
						
					}
					if(WQOS->qos[0]->radio_qos[3]->mapstate == 1)
					{
						vty_out(vty,"WMM VOICE map dot1p: %d\n",WQOS->qos[0]->radio_qos[3]->wmm_map_dot1p);
						
					}
					vty_out(vty,"==============================================================================\n");
					vty_out(vty,"Dot1p map infomation\n");
					vty_out(vty,"\n");
					
					for(j=0;j<WQOS->qos[0]->radio_qos[0]->dot1p_map_wmm_num;j++)
					{
						vty_out(vty,"Dot1p %d map BESTERRORT\n",WQOS->qos[0]->radio_qos[0]->dot1p_map_wmm[j]);
					}
					
					if(WQOS->qos[0]->radio_qos[1]->dot1p_map_wmm_num != 0)
					{
						for(j=0;j<WQOS->qos[0]->radio_qos[1]->dot1p_map_wmm_num;j++)
						{
							vty_out(vty,"Dot1p %d map BACKGROUND\n",WQOS->qos[0]->radio_qos[1]->dot1p_map_wmm[j]);
						}
					}
					if(WQOS->qos[0]->radio_qos[2]->dot1p_map_wmm_num != 0)
					{
						for(j=0;j<WQOS->qos[0]->radio_qos[2]->dot1p_map_wmm_num;j++)
						{
							vty_out(vty,"Dot1p %d map VIDEO\n",WQOS->qos[0]->radio_qos[2]->dot1p_map_wmm[j]);
						}
					}
					if(WQOS->qos[0]->radio_qos[3]->dot1p_map_wmm_num != 0)
					{
						for(j=0;j<WQOS->qos[0]->radio_qos[3]->dot1p_map_wmm_num;j++)
						{
							vty_out(vty,"Dot1p %d map VOICE\n",WQOS->qos[0]->radio_qos[3]->dot1p_map_wmm[j]);
						}
					}
					dcli_wqos_free_fun(WID_DBUS_QOS_METHOD_SHOW_QOS,WQOS);
				}
				else if(ret == WTP_ID_NOT_EXIST)
				{
					vty_out(vty,"<error> qos id does not exist\n");
				}
				else 
				{	
					vty_out(vty,"<error>  %d\n",ret);
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
DEFUN(wid_show_qos_profile_list_cmd_func,
	  wid_show_qos_profile_list_cmd,
	  "show wireless qos profile (all|list) [remote] [local] [PARAMETER]",
	  SHOW_STR
	  "wireless service\n"
	  "Display qos profile information\n"
	  "qos profile\n"
	  "all of qos profile\n"
	  "all of qos profile\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{	
	int ret = 0;
	int i = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	unsigned int num = 0;
	AC_QOS *qos[QOS_NUM];
	int dcli_sn = 1;
	char en[] = "enable";
	char dis[] = "disable";
/*	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	
	
	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}
	ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ReInitDbusPath(index,WID_DBUS_QOS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SHOW_QOS_LIST);
*/
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SHOW_QOS_LIST);*/
/*	dbus_error_init(&err);

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
	
	if(ret == 0 )
	{	
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for (i = 0; i < num; i++) {
			DBusMessageIter iter_struct;
			
			qos[i] = (AC_QOS*)malloc(sizeof(AC_QOS));
			qos[i]->radio_qos[0] = (qos_profile*)malloc(sizeof(qos_profile));
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(qos[i]->QosID));
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(qos[i]->name));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(qos[i]->radio_qos[0]->mapstate));

			dbus_message_iter_next(&iter_array);
		}
	}
	
	dbus_message_unref(reply);
*/	
	int index = 0;
    int localid = 1;
    int slot_id = HostSlotId;

//	DBusMessage *reply; 
//	DBusError err;
	DCLI_WQOS *WQOS = NULL;
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		 index = vty->index;
		 localid = vty->local;
		 slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
	    index = vty->index;
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
	
	if(vty->node != VIEW_NODE){
		WQOS = (DCLI_WQOS*)dcli_wqos_wireless_qos_show_config_info
			(
				index,/*"show wireless qos profile (all|list)"*/
				&ret,
				localid,
				0,
				0,
				//qos,
				dcli_dbus_connection,
				WID_DBUS_QOS_METHOD_SHOW_QOS_LIST
			);
	/*	if (NULL == reply) {
			cli_syslog_info("<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				cli_syslog_info("%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}*/
	    vty_out(vty,"Wireless qos profile list summary:\n");	
	    vty_out(vty,"%d qos profile exist\n",(ret != 0)?0:WQOS->qos_num);
	    vty_out(vty,"==============================================================================\n");
		vty_out(vty,"%-5s	%-16s	%7s\n","QOSID","QOSName","wmm_map");
		if(ret == -1){
			cli_syslog_info("<error> failed get reply.\n");
		}
		else if(ret == 0){
			for (i = 0; i < WQOS->qos_num; i++) {	
				vty_out(vty,"%-5d	%-16s	%7s\n",WQOS->qos[i]->QosID,WQOS->qos[i]->name,(WQOS->qos[i]->radio_qos[0]->mapstate == 1)?en:dis);
			}
			dcli_wqos_free_fun(WID_DBUS_QOS_METHOD_SHOW_QOS_LIST,WQOS);
		}
		else if(ret == 5){
			vty_out(vty,"qos not exsit\n");
		}
		vty_out(vty,"==============================================================================\n");
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
				WQOS = (DCLI_WQOS*)dcli_wqos_wireless_qos_show_config_info
					(
						profile,/*"show wireless qos profile (all|list)"*/
						&ret,
						localid,
						0,
						0,
						dcli_dbus_connection,
						WID_DBUS_QOS_METHOD_SHOW_QOS_LIST
					);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				vty_out(vty,"Wireless qos profile list summary:\n");	
				vty_out(vty,"%d qos profile exist\n",(ret != 0)?0:WQOS->qos_num);
				vty_out(vty,"========================================================================\n");
				vty_out(vty,"%-5s	%-16s	%7s\n","QOSID","QOSName","wmm_map");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0){
					for (i = 0; i < WQOS->qos_num; i++) {	
						vty_out(vty,"%-5d	%-16s	%7s\n",WQOS->qos[i]->QosID,WQOS->qos[i]->name,(WQOS->qos[i]->radio_qos[0]->mapstate == 1)?en:dis);
					}
					dcli_wqos_free_fun(WID_DBUS_QOS_METHOD_SHOW_QOS_LIST,WQOS);
				}
				else if(ret == 5){
					vty_out(vty,"qos not exsit\n");
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
				WQOS = (DCLI_WQOS*)dcli_wqos_wireless_qos_show_config_info
					(
						profile,/*"show wireless qos profile (all|list)"*/
						&ret,
						localid,
						0,
						0,
						dcli_dbus_connection,
						WID_DBUS_QOS_METHOD_SHOW_QOS_LIST
					);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				vty_out(vty,"Wireless qos profile list summary:\n");	
				vty_out(vty,"%d qos profile exist\n",(ret != 0)?0:WQOS->qos_num);
				vty_out(vty,"========================================================================\n");
				vty_out(vty,"%-5s	%-16s	%7s\n","QOSID","QOSName","wmm_map");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0){
					for (i = 0; i < WQOS->qos_num; i++) {	
						vty_out(vty,"%-5d	%-16s	%7s\n",WQOS->qos[i]->QosID,WQOS->qos[i]->name,(WQOS->qos[i]->radio_qos[0]->mapstate == 1)?en:dis);
					}
					dcli_wqos_free_fun(WID_DBUS_QOS_METHOD_SHOW_QOS_LIST,WQOS);
				}
				else if(ret == 5){
					vty_out(vty,"qos not exsit\n");
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


DEFUN(wid_config_qos_profile_cmd_func,
	  wid_config_qos_profile_cmd,
	  "config wireless qos ID",
	  CONFIG_STR
	  "wireless service\n"
	  "qos information\n"
	  "qos id that you want to config\n"
	 )
{	int ret = 0;
	int qos_id = 0;
	int status = 0;
	int wlanid = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	ret = parse_int_ID((char*)argv[0], &qos_id);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_WARNING;
	}	
	if(qos_id >= QOS_NUM || qos_id == 0){
		vty_out(vty,"<error> qos id should be 1 to %d\n",QOS_NUM-1);
		return CMD_WARNING;
	}
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		 index = vty->index;
		 localid = vty->local;
		 slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
	    index = vty->index;
	    localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_INTERFACE,INTERFACE); 
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_CONFIG_QOS);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_CONFIG_QOS);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&qos_id,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_WARNING;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret); 

	if(ret == 0)
	{
		if(vty->node == CONFIG_NODE){
			vty->node = WQOS_NODE;
			vty->index = (void *)qos_id;
		}else if(vty->node == HANSI_NODE){			
			vty->node = HANSI_WQOS_NODE;
			vty->index_sub = (void *)qos_id;
		}else if(vty->node == LOCAL_HANSI_NODE){			
			vty->node = LOCAL_HANSI_WQOS_NODE;
			vty->index_sub = (void *)qos_id;
		}
	}
	else if(ret == WID_QOS_NOT_EXIST)
	{
		vty_out(vty,"<error> qos id does not exist\n");
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
		dbus_message_unref(reply);
		return CMD_WARNING;
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(wid_config_qos_info_cmd_func,
	  wid_config_qos_info_cmd,
	  /*"set QOSSTREAM CWMIN CWMAX AFIS ACK",*/
	  "set radio (besteffort|background|video|voice) cwmin CWMIN cwmax CWMAX aifs AIFS txoplimit TXOPLIMIT (ack|noack)",
	  "Configure qos service\n"
	  "radio qos service\n"
	  "qos streams\n"
	  "qos parameters\n"
	 )
{	
	int ret = 0;
	int ID = 0;
	int qos_stream_id = 5;
	unsigned short cwmin = 0;
	unsigned short cwmax = 0;
	unsigned char aifs = 0;
	unsigned char ack = 2;
	unsigned short txoplimit = 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//ID = (int)vty->index;

	
	if ((!strcmp(argv[0],"VOICE"))||(!strcmp(argv[0],"voice")))
		qos_stream_id = 3;			
	else if ((!strcmp(argv[0],"VIDEO"))||(!strcmp(argv[0],"video")))
		qos_stream_id = 2;	
	else if ((!strcmp(argv[0],"BESTEFFORT"))||(!strcmp(argv[0],"besteffort")))
		qos_stream_id = 0;
	else if ((!strcmp(argv[0],"BACKGROUND"))||(!strcmp(argv[0],"background")))
		qos_stream_id = 1;
	else 
	{		
		vty_out(vty,"<error> unknown qos type.\n");
		return CMD_SUCCESS;
	}
	/*cwmin*/
	ret = wid_wqos_parse_short_ID((char*)argv[1], &cwmin);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(cwmin >= QOS_CWMIN_NUM ){
		vty_out(vty,"<error> qos cwmin should be 0 to %d\n",QOS_CWMIN_NUM-1);
		return CMD_SUCCESS;
	}
	/*cwmax*/
	ret = wid_wqos_parse_short_ID((char*)argv[2], &cwmax);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(cwmax >= QOS_CWMAX_NUM){
		vty_out(vty,"<error> qos cwmax should be 0 to %d\n",QOS_CWMAX_NUM-1);
		return CMD_SUCCESS;
	}
	/*fengwenchao add 20110517 for  cwmax and cwmin comparing */
	if(cwmin > cwmax)
	{
		vty_out(vty,"<error> cwmin is not allow larger than cwmax\n");
		return CMD_SUCCESS;		
	}
	/*fengwenchao add end*/	
	/*aifs*/
	ret = wid_wqos_parse_char_ID((char*)argv[3], &aifs);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(aifs >= QOS_AIFS_NUM ){
		vty_out(vty,"<error> qos aifs should be 0 to %d\n",QOS_AIFS_NUM-1);
		return CMD_SUCCESS;
	}
	/*txoplimit*/
	ret = wid_wqos_parse_short_ID((char*)argv[4], &txoplimit);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(txoplimit > QOS_TXOPLIMIT_NUM ){
		vty_out(vty,"<error> qos aifs should be 0 to %d\n",QOS_TXOPLIMIT_NUM);
		return CMD_SUCCESS;
	}
	/*ack*/
	if ((!strcmp(argv[5],"ACK"))||(!strcmp(argv[5],"ack")))
		ack = 1;
	else if ((!strcmp(argv[5],"NOACK"))||(!strcmp(argv[5],"noack")))
		ack = 0;
	else 
	{		
		vty_out(vty,"<error> unknown qos type.\n");
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WQOS_NODE){
		index = 0;			
		ID = (int)vty->index;
	}else if(vty->node == HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
	    slot_id = vty->slotindex;
	}
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_INTERFACE,INTERFACE); 
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_INFO);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_INFO);*/

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ID,
							 DBUS_TYPE_UINT32,&qos_stream_id,
							 DBUS_TYPE_UINT16,&cwmin,
							 DBUS_TYPE_UINT16,&cwmax,
							 DBUS_TYPE_BYTE,&aifs,
							 DBUS_TYPE_BYTE,&ack,
							 DBUS_TYPE_UINT16,&txoplimit,
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

		if(ret == WID_DBUS_SUCCESS)
			vty_out(vty,"set qos radio info successfully\n");
		else if(ret == WID_QOS_NOT_EXIST)			
			vty_out(vty,"<error> qos profile does not exist.\n");	
		else if(ret == WID_QOS_RADIO_SHOULD_BE_DISABLE)			
			vty_out(vty,"<error> this qos profile is used by some radios,please disable them first\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(wid_config_client_qos_info_cmd_func,
	  wid_config_client_qos_info_cmd,
	  /*"set client QOSSTREAM CWMIN CWMAX AFIS ACK",*/
	  "set client (besteffort|background|video|voice) cwmin CWMIN cwmax CWMAX aifs AIFS txoplimit TXOPLIMIT",
	  "Configure qos service\n"
	  "client qos service\n"
	  "qos streams\n"
	  "qos parameters\n"
	 )
{	
	int ret = 0;
	int ID = 0;
	int qos_stream_id = 5;
	unsigned short cwmin = 0;
	unsigned short cwmax = 0;
	unsigned char aifs = 0;
	unsigned short txoplimit = 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//ID = (int)vty->index;

	if ((!strcmp(argv[0],"VOICE"))||(!strcmp(argv[0],"voice")))
		qos_stream_id = 3;			
	else if ((!strcmp(argv[0],"VIDEO"))||(!strcmp(argv[0],"video")))
		qos_stream_id = 2;	
	else if ((!strcmp(argv[0],"BESTEFFORT"))||(!strcmp(argv[0],"besteffort")))
		qos_stream_id = 0;
	else if ((!strcmp(argv[0],"BACKGROUND"))||(!strcmp(argv[0],"background")))
		qos_stream_id = 1;
	else 
	{		
		vty_out(vty,"<error> unknown qos type.\n");
		return CMD_SUCCESS;
	}
	/*cwmin*/
	ret = wid_wqos_parse_short_ID((char*)argv[1], &cwmin);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(cwmin >= QOS_CWMIN_NUM ){
		vty_out(vty,"<error> qos cwmin should be 0 to %d\n",QOS_CWMIN_NUM-1);
		return CMD_SUCCESS;
	}
	/*cwmax*/
	ret = wid_wqos_parse_short_ID((char*)argv[2], &cwmax);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(cwmax >= QOS_CWMAX_NUM){
		vty_out(vty,"<error> qos cwmax should be 0 to %d\n",QOS_CWMAX_NUM-1);
		return CMD_SUCCESS;
	}
	/*fengwenchao add 20110517 for  cwmax and cwmin comparing */
	if(cwmin > cwmax)
	{
		vty_out(vty,"<error> cwmin is not allow larger than cwmax\n");
		return CMD_SUCCESS;		
	}
	/*fengwenchao add end*/	
	/*aifs*/
	ret = wid_wqos_parse_char_ID((char*)argv[3], &aifs);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(aifs >= QOS_AIFS_NUM ){
		vty_out(vty,"<error> qos aifs should be 0 to %d\n",QOS_AIFS_NUM-1);
		return CMD_SUCCESS;
	}
	/*txoplimit*/
	ret = wid_wqos_parse_short_ID((char*)argv[4], &txoplimit);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(txoplimit > QOS_TXOPLIMIT_NUM ){
		vty_out(vty,"<error> qos aifs should be 0 to %d\n",QOS_TXOPLIMIT_NUM);
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WQOS_NODE){
		index = 0;			
		ID = (int)vty->index;
	}else if(vty->node == HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_INTERFACE,INTERFACE); 
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_INFO_CLIENT);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_INFO_CLIENT);*/

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&ID,
							 DBUS_TYPE_UINT32,&qos_stream_id,
							 DBUS_TYPE_UINT16,&cwmin,
							 DBUS_TYPE_UINT16,&cwmax,
							 DBUS_TYPE_BYTE,&aifs,
							 DBUS_TYPE_UINT16,&txoplimit,
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

		if(ret == WID_DBUS_SUCCESS)
			vty_out(vty,"set qos client info successfully\n");
		else if(ret == WID_QOS_NOT_EXIST)			
			vty_out(vty,"<error> qos profile does not exist.\n");	
		else if(ret == WID_QOS_RADIO_SHOULD_BE_DISABLE)			
			vty_out(vty,"<error> this qos profile is used by some radios,please disable them first\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(wid_config_set_qos_wmm_cmd_func,
		wid_config_set_qos_wmm_cmd,
		"set wmm map (enable|disable)",
		"Configuration service\n"
		"WMM service\n"
		"dot1p and wmm map \n"
	)
{
	int ret;
	int isAdd = 2;	
	int ID = 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//ID = (int)vty->index;
	
	if ((!strcmp(argv[0],"enable"))||(!strcmp(argv[0],"ENABLE")))
		isAdd = 1;
	else if ((!strcmp(argv[0],"disable"))||(!strcmp(argv[0],"DISABLE")))
		isAdd = 0;
	else 
	{		
		vty_out(vty,"<error> input parameter should be only 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}

	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WQOS_NODE){
		index = 0;			
		ID = (int)vty->index;
	}else if(vty->node == HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_INTERFACE,INTERFACE); 
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_MAP);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_MAP);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&ID,								
						DBUS_TYPE_UINT32,&isAdd,						 
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

		if(ret == 0)
			vty_out(vty,"qos profile %d wmm map %s successfully\n",ID,argv[0]);
		else if(ret == WID_QOS_NOT_EXIST)
			vty_out(vty,"<error> qos id does not exist\n");			
		else if(ret == WID_QOS_RADIO_SHOULD_BE_DISABLE)			
			vty_out(vty,"<error> this qos profile be used by some radios,please disable them first\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;	
}
DEFUN(wid_config_set_qos_wmm_map_dot1p_cmd_func,
		wid_config_set_qos_wmm_map_dot1p_cmd,
		"wmm (besteffort|background|video|voice) map dot1p <0-7>",
		"WMM service\n"
		"WMM service\n"
		"wmm map dot1p\n"
		"dot1p and wmm map \n"
	)
{
	int ret = 0;
	int wmm_order = 4;	
	int ID = 0;
	unsigned char dot1p = 8;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//ID = (int)vty->index;
	
	
	if ((!strcmp(argv[0],"VOICE"))||(!strcmp(argv[0],"voice")))
		wmm_order = 3;			
	else if ((!strcmp(argv[0],"VIDEO"))||(!strcmp(argv[0],"video")))
		wmm_order = 2;	
	else if ((!strcmp(argv[0],"BESTEFFORT"))||(!strcmp(argv[0],"besteffort")))
		wmm_order = 0;
	else if ((!strcmp(argv[0],"BACKGROUND"))||(!strcmp(argv[0],"background")))
		wmm_order = 1;
	else 
	{		
		vty_out(vty,"<error> input parameter should be only 'voice' 'video' 'besteffort' or 'background'\n");
		return CMD_SUCCESS;
	}
	
	
	/*ret = parse_char_ID((char*)argv[1], &dot1p);*/
	dot1p = atoi(argv[1]);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(dot1p >= 8 ){
		vty_out(vty,"<error> qos dot1p should be 0 to 7\n");
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WQOS_NODE){
		index = 0;			
		ID = (int)vty->index;
	}else if(vty->node == HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
	    slot_id = vty->slotindex;
	}
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_INTERFACE,INTERFACE); 
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_WMM_MAP_DOT1P);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_WMM_MAP_DOT1P);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&ID,								
						DBUS_TYPE_UINT32,&wmm_order,
						DBUS_TYPE_BYTE,&dot1p,
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

		if(ret == 0)
			vty_out(vty,"qos profile %d wmm %s map dot1p %s successfully\n",ID,argv[0],argv[1]);
		else if(ret == WID_QOS_NOT_EXIST)
			vty_out(vty,"<error> qos id does not exist\n");			
		else if(ret == WID_QOS_RADIO_SHOULD_BE_DISABLE)			
			vty_out(vty,"<error> this qos profile be used by some radios,please disable them first\n");
		else if(ret == WID_QOS_WMM_MAP_DISABLE)			
			vty_out(vty,"<error> this qos map is disable,please enable it first\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;	
}
DEFUN(wid_config_set_qos_dot1p_map_wmm_cmd_func,
		wid_config_set_qos_dot1p_map_wmm_cmd,
		"dot1p LIST map (besteffort|background|video|voice)",
		"WMM service\n"
		"dot1p LIST\n"
		"dot1p map wmm\n"
		"WMM service order \n"
	)
{
	int ret;
	int wmm_order = 4;	
	int ID = 0;
	unsigned char dot1p[DCLIWQOS_DOT1P_LIST_NUM];
	unsigned char n = 0;
	unsigned char num = 0;
	int i = 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter,iter_array;
	DBusError err;
	//ID = (int)vty->index;
	
	ret = parse_dot1p_list((char*)argv[0],&n,dot1p);

	num = QosRemoveListRepId(dot1p,n);

	if (-1 == ret) {
    	vty_out(vty,"<error> input parameter is illegal\n");
		return CMD_FAILURE;
	}

	
	if ((!strcmp(argv[1],"VOICE"))||(!strcmp(argv[1],"voice")))
		wmm_order = 3;			
	else if ((!strcmp(argv[1],"VIDEO"))||(!strcmp(argv[1],"video")))
		wmm_order = 2;	
	else if ((!strcmp(argv[1],"BESTEFFORT"))||(!strcmp(argv[1],"besteffort")))
		wmm_order = 0;
	else if ((!strcmp(argv[1],"BACKGROUND"))||(!strcmp(argv[1],"background")))
		wmm_order = 1;
	else 
	{		
		vty_out(vty,"<error> input parameter should be only 'voice' 'video' 'besteffort' or 'background'\n");
		return CMD_SUCCESS;
	}
	
	
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WQOS_NODE){
		index = 0;			
		ID = (int)vty->index;
	}else if(vty->node == HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_INTERFACE,INTERFACE); 
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_DOT1P_MAP_WMM);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_DOT1P_MAP_WMM);*/
	dbus_error_init(&err);

	
	dbus_message_iter_init_append (query, &iter);

	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&ID);

	dbus_message_iter_append_basic (&iter,DBUS_TYPE_BYTE,&num);

	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wmm_order);
			
	for(i = 0; i < num; i++)
	{
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_BYTE,&dot1p[i]);
	}
	
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

		if(ret == 0)
			vty_out(vty,"qos profile %d dot1p %s map wmm %s successfully\n",ID,argv[0],argv[1]);
		else if(ret == WID_QOS_NOT_EXIST)
			vty_out(vty,"<error> qos id does not exist\n");			
		else if(ret == WID_QOS_RADIO_SHOULD_BE_DISABLE)			
			vty_out(vty,"<error> this qos profile be used by some radios,please disable them first\n");
		else if(ret == WID_QOS_WMM_MAP_DISABLE)			
			vty_out(vty,"<error> this qos map is disable,please enable it first\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;	
}
DEFUN(wid_show_qos_extension_info_cmd_func,
	  wid_show_qos_extension_info_cmd,
	  "show wireless qos ID extension info [remote] [local] [PARAMETER]",
	  SHOW_STR
	  "wireless qos\n"
	  "Display qos profile information\n"
	  "ID of qos profile <1-15>\n"
	  "qos extension information\n"
	  "qos extension information\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{	
	int ret = 0;
	int qos_id = 0;
	int dcli_sn = 3;
	int	ID = 0;
	int num = 0;
	int i = 0;
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	char enable[] = "enable";
	char disable[] = "disable";
//AC_QOS *qos[QOS_NUM];
//	AC_QOS **qos;
/*	qos = (AC_QOS *)malloc(sizeof(AC_QOS));
	char *manage_name;
	char *grab_name;
	char *shove_name;
	
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
*/	
	ret = parse_int_ID((char*)argv[0], &qos_id);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(qos_id >= QOS_NUM || qos_id == 0){
		vty_out(vty,"<error> qos id should be 1 to %d\n",QOS_NUM-1);
		return CMD_SUCCESS;
	}
	
/*	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
	}
	ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ReInitDbusPath(index,WID_DBUS_QOS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SHOW_QOS_EXTENSION_INFO);
*/
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SHOW_QOS_EXTENSION_INFO);*/
/*	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&qos_id,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		if(qos)
		{
			free(qos);
			qos = NULL;
		}
		return CMD_SUCCESS;
	}

	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	
	if(ret == 0 )
	{	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&qos->QosID);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&qos->qos_total_bandwidth);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&qos->qos_res_scale);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&qos->qos_share_bandwidth);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&qos->qos_res_share_scale);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&manage_name);
		memset(qos->qos_manage_arithmetic,0,WID_QOS_ARITHMETIC_NAME_LEN);
		memcpy(qos->qos_manage_arithmetic,manage_name,strlen(manage_name));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&grab_name);
		memset(qos->qos_res_grab_arithmetic,0,WID_QOS_ARITHMETIC_NAME_LEN);
		memcpy(qos->qos_res_grab_arithmetic,grab_name,strlen(grab_name));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&shove_name);
		memset(qos->qos_res_shove_arithmetic,0,WID_QOS_ARITHMETIC_NAME_LEN);
		memcpy(qos->qos_res_shove_arithmetic,shove_name,strlen(shove_name));
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&qos->qos_use_res_grab);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&qos->qos_use_res_shove);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for (i = 0; i < 4; i++) {
			DBusMessageIter iter_struct;
			
			qos->radio_qos[i] = (qos_profile *)malloc(sizeof(qos_profile));
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(qos->radio_qos[i]->qos_average_rate));
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(qos->radio_qos[i]->qos_max_degree));
		
			dbus_message_iter_next(&iter_struct);
	
			dbus_message_iter_get_basic(&iter_struct,&(qos->radio_qos[i]->qos_policy_pri));
		
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(qos->radio_qos[i]->qos_res_shove_pri));
					
			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(qos->radio_qos[i]->qos_res_grab_pri));
			
			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(qos->radio_qos[i]->qos_max_parallel));
					
			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(qos->radio_qos[i]->qos_bandwidth));
				
			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(qos->radio_qos[i]->qos_bandwidth_scale));

			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(qos->radio_qos[i]->qos_use_wred));

			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(qos->radio_qos[i]->qos_use_traffic_shaping));

			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(qos->radio_qos[i]->qos_use_flow_eq_queue));

			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(qos->radio_qos[i]->qos_flow_average_rate));

			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(qos->radio_qos[i]->qos_flow_max_degree));

			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(qos->radio_qos[i]->qos_flow_max_queuedepth));

			dbus_message_iter_next(&iter_array);
		}
	}
	*/
	int index = 0;
    int localid = 1;
    int slot_id = HostSlotId;

//	DBusMessage *reply; 
//	DBusError err;
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		 index = vty->index;
		 localid = vty->local;
		 slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
	    index = vty->index;
	    localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DCLI_WQOS *WQOS = NULL;
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
	
	if(vty->node != VIEW_NODE){
		WQOS = (DCLI_WQOS*)dcli_wqos_wireless_qos_show_config_info
			(
				index,
				&ret,
				localid,
				qos_id,			
				0,
			//	qos,
				dcli_dbus_connection,
				WID_DBUS_QOS_METHOD_SHOW_QOS_EXTENSION_INFO
			);
		if(ret == -1){
			cli_syslog_info("<error> failed get reply.\n");
		}
		else if(ret == 0){
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"Wireless Qos profile %d extension infomation\n",WQOS->qos[0]->QosID);
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"qos total bandwidth	%d\n",WQOS->qos[0]->qos_total_bandwidth);
			vty_out(vty,"qos resource percentage	%d %\n",WQOS->qos[0]->qos_res_scale);
			vty_out(vty,"qos share bandwidth	%d\n",WQOS->qos[0]->qos_share_bandwidth);
			vty_out(vty,"qos resource share percentage	%d %\n",WQOS->qos[0]->qos_res_share_scale);
			vty_out(vty,"qos manage arithmetic name	%s\n",WQOS->qos[0]->qos_manage_arithmetic);
			vty_out(vty,"qos grab arithmetic name	%s\n",WQOS->qos[0]->qos_res_grab_arithmetic);
			vty_out(vty,"qos shove arithmetic name	%s\n",WQOS->qos[0]->qos_res_shove_arithmetic);
			vty_out(vty,"qos grab arithmetic	%s\n",(WQOS->qos[0]->qos_use_res_grab == 1)?enable:disable);
			vty_out(vty,"qos shove arithmetic	%s\n",(WQOS->qos[0]->qos_use_res_shove == 1)?enable:disable);
			vty_out(vty,"----------\n");
			vty_out(vty,"radio qos extension infomation\n");
			vty_out(vty,"----------\n");
			vty_out(vty,"BESTEFFORT\n");
			vty_out(vty,"BESTEFFORT:  average rate	%d\n",WQOS->qos[0]->radio_qos[0]->qos_average_rate);
			vty_out(vty,"BESTEFFORT:  max burstiness	%d\n",WQOS->qos[0]->radio_qos[0]->qos_max_degree);													
			vty_out(vty,"BESTEFFORT:  manage policy priority	%d\n",WQOS->qos[0]->radio_qos[0]->qos_policy_pri);	
			vty_out(vty,"BESTEFFORT:  shove priority	%d\n",WQOS->qos[0]->radio_qos[0]->qos_res_shove_pri);	
			vty_out(vty,"BESTEFFORT:  grab priority	%d\n",WQOS->qos[0]->radio_qos[0]->qos_res_grab_pri);	
			vty_out(vty,"BESTEFFORT:  max parallel	%d\n",WQOS->qos[0]->radio_qos[0]->qos_max_parallel);	
			vty_out(vty,"BESTEFFORT:  bandwidth	%d\n",WQOS->qos[0]->radio_qos[0]->qos_bandwidth);	
			vty_out(vty,"BESTEFFORT:  bandwidth percentage	%d\n",WQOS->qos[0]->radio_qos[0]->qos_bandwidth_scale);
			vty_out(vty,"BESTEFFORT:  use WRED	%s\n",(WQOS->qos[0]->radio_qos[0]->qos_use_wred == 1)?enable:disable);	
			vty_out(vty,"BESTEFFORT:  use traffic shaping	%s\n",(WQOS->qos[0]->radio_qos[0]->qos_use_traffic_shaping == 1)?enable:disable);	
			vty_out(vty,"----------\n");
			vty_out(vty,"BACKGROUND\n");
			vty_out(vty,"BACKGROUND:  average rate	%d\n",WQOS->qos[0]->radio_qos[1]->qos_average_rate);
			vty_out(vty,"BACKGROUND:  max burstiness	%d\n",WQOS->qos[0]->radio_qos[1]->qos_max_degree);													
			vty_out(vty,"BACKGROUND:  manage policy priority	%d\n",WQOS->qos[0]->radio_qos[1]->qos_policy_pri);	
			vty_out(vty,"BACKGROUND:  shove priority	%d\n",WQOS->qos[0]->radio_qos[1]->qos_res_shove_pri);	
			vty_out(vty,"BACKGROUND:  grab priority	%d\n",WQOS->qos[0]->radio_qos[1]->qos_res_grab_pri);	
			vty_out(vty,"BACKGROUND:  max parallel	%d\n",WQOS->qos[0]->radio_qos[1]->qos_max_parallel);	
			vty_out(vty,"BACKGROUND:  bandwidth	%d\n",WQOS->qos[0]->radio_qos[1]->qos_bandwidth);	
			vty_out(vty,"BACKGROUND:  bandwidth percentage	%d\n",WQOS->qos[0]->radio_qos[1]->qos_bandwidth_scale);
			vty_out(vty,"BACKGROUND:  use WRED	%s\n",(WQOS->qos[0]->radio_qos[1]->qos_use_wred == 1)?enable:disable);	
			vty_out(vty,"BACKGROUND:  use traffic shaping	%s\n",(WQOS->qos[0]->radio_qos[1]->qos_use_traffic_shaping == 1)?enable:disable);	
			vty_out(vty,"----------\n");
			vty_out(vty,"VIDEO\n");
			vty_out(vty,"VIDEO:       average rate	%d\n",WQOS->qos[0]->radio_qos[2]->qos_average_rate);
			vty_out(vty,"VIDEO:       max burstiness	%d\n",WQOS->qos[0]->radio_qos[2]->qos_max_degree);													
			vty_out(vty,"VIDEO:       manage policy priority	%d\n",WQOS->qos[0]->radio_qos[2]->qos_policy_pri);	
			vty_out(vty,"VIDEO:       shove priority	%d\n",WQOS->qos[0]->radio_qos[2]->qos_res_shove_pri);	
			vty_out(vty,"VIDEO:       grab priority	%d\n",WQOS->qos[0]->radio_qos[2]->qos_res_grab_pri);	
			vty_out(vty,"VIDEO:       max parallel	%d\n",WQOS->qos[0]->radio_qos[2]->qos_max_parallel);	
			vty_out(vty,"VIDEO:       bandwidth	%d\n",WQOS->qos[0]->radio_qos[2]->qos_bandwidth);	
			vty_out(vty,"VIDEO:       bandwidth percentage	%d\n",WQOS->qos[0]->radio_qos[2]->qos_bandwidth_scale);
			vty_out(vty,"VIDEO:       use WRED	%s\n",(WQOS->qos[0]->radio_qos[2]->qos_use_wred == 1)?enable:disable);	
			vty_out(vty,"VIDEO:       use traffic shaping	%s\n",(WQOS->qos[0]->radio_qos[2]->qos_use_traffic_shaping == 1)?enable:disable);	
			vty_out(vty,"VIDEO:       use flow equity queue	%s\n",(WQOS->qos[0]->radio_qos[2]->qos_use_flow_eq_queue == 1)?enable:disable);	
			vty_out(vty,"VIDEO:       flow queue lenth	%d\n",WQOS->qos[0]->radio_qos[2]->qos_flow_max_queuedepth);	
			vty_out(vty,"VIDEO:       flow average rate	%d\n",WQOS->qos[0]->radio_qos[2]->qos_flow_average_rate);	
			vty_out(vty,"VIDEO:       flow max burstiness	%d\n",WQOS->qos[0]->radio_qos[2]->qos_flow_max_degree);	
			vty_out(vty,"----------\n");
			vty_out(vty,"VOICE\n");
			vty_out(vty,"VOICE:       average rate	%d\n",WQOS->qos[0]->radio_qos[3]->qos_average_rate);
			vty_out(vty,"VOICE:       max burstiness	%d\n",WQOS->qos[0]->radio_qos[3]->qos_max_degree);													
			vty_out(vty,"VOICE:       manage policy priority	%d\n",WQOS->qos[0]->radio_qos[3]->qos_policy_pri);	
			vty_out(vty,"VOICE:       shove priority	%d\n",WQOS->qos[0]->radio_qos[3]->qos_res_shove_pri);	
			vty_out(vty,"VOICE:       grab priority	%d\n",WQOS->qos[0]->radio_qos[3]->qos_res_grab_pri);	
			vty_out(vty,"VOICE:       max parallel	%d\n",WQOS->qos[0]->radio_qos[3]->qos_max_parallel);	
			vty_out(vty,"VOICE:       bandwidth	%d\n",WQOS->qos[0]->radio_qos[3]->qos_bandwidth);	
			vty_out(vty,"VOICE:       bandwidth percentage	%d\n",WQOS->qos[0]->radio_qos[3]->qos_bandwidth_scale);
			vty_out(vty,"VOICE:       use WRED	%s\n",(WQOS->qos[0]->radio_qos[3]->qos_use_wred == 1)?enable:disable);	
			vty_out(vty,"VOICE:       use traffic shaping	%s\n",(WQOS->qos[0]->radio_qos[3]->qos_use_traffic_shaping == 1)?enable:disable);	
			vty_out(vty,"VOICE:       use flow equity queue	%s\n",(WQOS->qos[0]->radio_qos[3]->qos_use_flow_eq_queue == 1)?enable:disable);	
			vty_out(vty,"VOICE:       flow queue lenth	%d\n",WQOS->qos[0]->radio_qos[3]->qos_flow_max_queuedepth);	
			vty_out(vty,"VOICE:       flow average rate	%d\n",WQOS->qos[0]->radio_qos[3]->qos_flow_average_rate);	
			vty_out(vty,"VOICE:       flow max burstiness	%d\n",WQOS->qos[0]->radio_qos[3]->qos_flow_max_degree);
			vty_out(vty,"\n");
			vty_out(vty,"==============================================================================\n");
		/*	for(i=0;i<4;i++)
			{
				if(WQOS->qos[0]->radio_qos[i])
				{
					free(WQOS->qos[0]->radio_qos[i]);
					WQOS->qos[0]->radio_qos[i] = NULL;
				}
			}*/
			dcli_wqos_free_fun(WID_DBUS_QOS_METHOD_SHOW_QOS_EXTENSION_INFO,WQOS);
		}
		else if(ret == WTP_ID_NOT_EXIST)
		{
			vty_out(vty,"<error> qos id does not exist\n");
		}
		else 
		{	
			vty_out(vty,"<error>  %d\n",ret);
		}
	//	dbus_message_unref(reply);
	/*	if(WQOS->qos[0])
		{
			free(WQOS->qos[0]);
			WQOS->qos[0] = NULL;
		}*/
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
				WQOS = (DCLI_WQOS*)dcli_wqos_wireless_qos_show_config_info
					(
						profile,
						&ret,
						localid,
						qos_id, 		
						0,
						dcli_dbus_connection,
						WID_DBUS_QOS_METHOD_SHOW_QOS_EXTENSION_INFO
					);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0){
					vty_out(vty,"=========================================================================\n");
					vty_out(vty,"Wireless Qos profile %d extension infomation\n",WQOS->qos[0]->QosID);
					vty_out(vty,"=========================================================================\n");
					vty_out(vty,"qos total bandwidth	%d\n",WQOS->qos[0]->qos_total_bandwidth);
					vty_out(vty,"qos resource percentage	%d %\n",WQOS->qos[0]->qos_res_scale);
					vty_out(vty,"qos share bandwidth	%d\n",WQOS->qos[0]->qos_share_bandwidth);
					vty_out(vty,"qos resource share percentage	%d %\n",WQOS->qos[0]->qos_res_share_scale);
					vty_out(vty,"qos manage arithmetic name %s\n",WQOS->qos[0]->qos_manage_arithmetic);
					vty_out(vty,"qos grab arithmetic name	%s\n",WQOS->qos[0]->qos_res_grab_arithmetic);
					vty_out(vty,"qos shove arithmetic name	%s\n",WQOS->qos[0]->qos_res_shove_arithmetic);
					vty_out(vty,"qos grab arithmetic	%s\n",(WQOS->qos[0]->qos_use_res_grab == 1)?enable:disable);
					vty_out(vty,"qos shove arithmetic	%s\n",(WQOS->qos[0]->qos_use_res_shove == 1)?enable:disable);
					vty_out(vty,"----------\n");
					vty_out(vty,"radio qos extension infomation\n");
					vty_out(vty,"----------\n");
					vty_out(vty,"BESTEFFORT\n");
					vty_out(vty,"BESTEFFORT:  average rate	%d\n",WQOS->qos[0]->radio_qos[0]->qos_average_rate);
					vty_out(vty,"BESTEFFORT:  max burstiness	%d\n",WQOS->qos[0]->radio_qos[0]->qos_max_degree);													
					vty_out(vty,"BESTEFFORT:  manage policy priority	%d\n",WQOS->qos[0]->radio_qos[0]->qos_policy_pri);	
					vty_out(vty,"BESTEFFORT:  shove priority	%d\n",WQOS->qos[0]->radio_qos[0]->qos_res_shove_pri);	
					vty_out(vty,"BESTEFFORT:  grab priority %d\n",WQOS->qos[0]->radio_qos[0]->qos_res_grab_pri);	
					vty_out(vty,"BESTEFFORT:  max parallel	%d\n",WQOS->qos[0]->radio_qos[0]->qos_max_parallel);	
					vty_out(vty,"BESTEFFORT:  bandwidth %d\n",WQOS->qos[0]->radio_qos[0]->qos_bandwidth);	
					vty_out(vty,"BESTEFFORT:  bandwidth percentage	%d\n",WQOS->qos[0]->radio_qos[0]->qos_bandwidth_scale);
					vty_out(vty,"BESTEFFORT:  use WRED	%s\n",(WQOS->qos[0]->radio_qos[0]->qos_use_wred == 1)?enable:disable);	
					vty_out(vty,"BESTEFFORT:  use traffic shaping	%s\n",(WQOS->qos[0]->radio_qos[0]->qos_use_traffic_shaping == 1)?enable:disable);	
					vty_out(vty,"----------\n");
					vty_out(vty,"BACKGROUND\n");
					vty_out(vty,"BACKGROUND:  average rate	%d\n",WQOS->qos[0]->radio_qos[1]->qos_average_rate);
					vty_out(vty,"BACKGROUND:  max burstiness	%d\n",WQOS->qos[0]->radio_qos[1]->qos_max_degree);													
					vty_out(vty,"BACKGROUND:  manage policy priority	%d\n",WQOS->qos[0]->radio_qos[1]->qos_policy_pri);	
					vty_out(vty,"BACKGROUND:  shove priority	%d\n",WQOS->qos[0]->radio_qos[1]->qos_res_shove_pri);	
					vty_out(vty,"BACKGROUND:  grab priority %d\n",WQOS->qos[0]->radio_qos[1]->qos_res_grab_pri);	
					vty_out(vty,"BACKGROUND:  max parallel	%d\n",WQOS->qos[0]->radio_qos[1]->qos_max_parallel);	
					vty_out(vty,"BACKGROUND:  bandwidth %d\n",WQOS->qos[0]->radio_qos[1]->qos_bandwidth);	
					vty_out(vty,"BACKGROUND:  bandwidth percentage	%d\n",WQOS->qos[0]->radio_qos[1]->qos_bandwidth_scale);
					vty_out(vty,"BACKGROUND:  use WRED	%s\n",(WQOS->qos[0]->radio_qos[1]->qos_use_wred == 1)?enable:disable);	
					vty_out(vty,"BACKGROUND:  use traffic shaping	%s\n",(WQOS->qos[0]->radio_qos[1]->qos_use_traffic_shaping == 1)?enable:disable);	
					vty_out(vty,"----------\n");
					vty_out(vty,"VIDEO\n");
					vty_out(vty,"VIDEO: 	  average rate	%d\n",WQOS->qos[0]->radio_qos[2]->qos_average_rate);
					vty_out(vty,"VIDEO: 	  max burstiness	%d\n",WQOS->qos[0]->radio_qos[2]->qos_max_degree);													
					vty_out(vty,"VIDEO: 	  manage policy priority	%d\n",WQOS->qos[0]->radio_qos[2]->qos_policy_pri);	
					vty_out(vty,"VIDEO: 	  shove priority	%d\n",WQOS->qos[0]->radio_qos[2]->qos_res_shove_pri);	
					vty_out(vty,"VIDEO: 	  grab priority %d\n",WQOS->qos[0]->radio_qos[2]->qos_res_grab_pri);	
					vty_out(vty,"VIDEO: 	  max parallel	%d\n",WQOS->qos[0]->radio_qos[2]->qos_max_parallel);	
					vty_out(vty,"VIDEO: 	  bandwidth %d\n",WQOS->qos[0]->radio_qos[2]->qos_bandwidth);	
					vty_out(vty,"VIDEO: 	  bandwidth percentage	%d\n",WQOS->qos[0]->radio_qos[2]->qos_bandwidth_scale);
					vty_out(vty,"VIDEO: 	  use WRED	%s\n",(WQOS->qos[0]->radio_qos[2]->qos_use_wred == 1)?enable:disable);	
					vty_out(vty,"VIDEO: 	  use traffic shaping	%s\n",(WQOS->qos[0]->radio_qos[2]->qos_use_traffic_shaping == 1)?enable:disable);	
					vty_out(vty,"VIDEO: 	  use flow equity queue %s\n",(WQOS->qos[0]->radio_qos[2]->qos_use_flow_eq_queue == 1)?enable:disable); 
					vty_out(vty,"VIDEO: 	  flow queue lenth	%d\n",WQOS->qos[0]->radio_qos[2]->qos_flow_max_queuedepth); 
					vty_out(vty,"VIDEO: 	  flow average rate %d\n",WQOS->qos[0]->radio_qos[2]->qos_flow_average_rate);	
					vty_out(vty,"VIDEO: 	  flow max burstiness	%d\n",WQOS->qos[0]->radio_qos[2]->qos_flow_max_degree); 
					vty_out(vty,"----------\n");
					vty_out(vty,"VOICE\n");
					vty_out(vty,"VOICE: 	  average rate	%d\n",WQOS->qos[0]->radio_qos[3]->qos_average_rate);
					vty_out(vty,"VOICE: 	  max burstiness	%d\n",WQOS->qos[0]->radio_qos[3]->qos_max_degree);													
					vty_out(vty,"VOICE: 	  manage policy priority	%d\n",WQOS->qos[0]->radio_qos[3]->qos_policy_pri);	
					vty_out(vty,"VOICE: 	  shove priority	%d\n",WQOS->qos[0]->radio_qos[3]->qos_res_shove_pri);	
					vty_out(vty,"VOICE: 	  grab priority %d\n",WQOS->qos[0]->radio_qos[3]->qos_res_grab_pri);	
					vty_out(vty,"VOICE: 	  max parallel	%d\n",WQOS->qos[0]->radio_qos[3]->qos_max_parallel);	
					vty_out(vty,"VOICE: 	  bandwidth %d\n",WQOS->qos[0]->radio_qos[3]->qos_bandwidth);	
					vty_out(vty,"VOICE: 	  bandwidth percentage	%d\n",WQOS->qos[0]->radio_qos[3]->qos_bandwidth_scale);
					vty_out(vty,"VOICE: 	  use WRED	%s\n",(WQOS->qos[0]->radio_qos[3]->qos_use_wred == 1)?enable:disable);	
					vty_out(vty,"VOICE: 	  use traffic shaping	%s\n",(WQOS->qos[0]->radio_qos[3]->qos_use_traffic_shaping == 1)?enable:disable);	
					vty_out(vty,"VOICE: 	  use flow equity queue %s\n",(WQOS->qos[0]->radio_qos[3]->qos_use_flow_eq_queue == 1)?enable:disable); 
					vty_out(vty,"VOICE: 	  flow queue lenth	%d\n",WQOS->qos[0]->radio_qos[3]->qos_flow_max_queuedepth); 
					vty_out(vty,"VOICE: 	  flow average rate %d\n",WQOS->qos[0]->radio_qos[3]->qos_flow_average_rate);	
					vty_out(vty,"VOICE: 	  flow max burstiness	%d\n",WQOS->qos[0]->radio_qos[3]->qos_flow_max_degree);
					vty_out(vty,"\n");
					dcli_wqos_free_fun(WID_DBUS_QOS_METHOD_SHOW_QOS_EXTENSION_INFO,WQOS);
				}
				else if(ret == WTP_ID_NOT_EXIST)
				{
					vty_out(vty,"<error> qos id does not exist\n");
				}
				else 
				{	
					vty_out(vty,"<error>  %d\n",ret);
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
				WQOS = (DCLI_WQOS*)dcli_wqos_wireless_qos_show_config_info
					(
						index,
						&ret,
						localid,
						qos_id, 		
						0,
						dcli_dbus_connection,
						WID_DBUS_QOS_METHOD_SHOW_QOS_EXTENSION_INFO
					);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"hansi %d-%d\n",slot_id,profile);
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0){
					vty_out(vty,"=========================================================================\n");
					vty_out(vty,"Wireless Qos profile %d extension infomation\n",WQOS->qos[0]->QosID);
					vty_out(vty,"=========================================================================\n");
					vty_out(vty,"qos total bandwidth	%d\n",WQOS->qos[0]->qos_total_bandwidth);
					vty_out(vty,"qos resource percentage	%d %\n",WQOS->qos[0]->qos_res_scale);
					vty_out(vty,"qos share bandwidth	%d\n",WQOS->qos[0]->qos_share_bandwidth);
					vty_out(vty,"qos resource share percentage	%d %\n",WQOS->qos[0]->qos_res_share_scale);
					vty_out(vty,"qos manage arithmetic name %s\n",WQOS->qos[0]->qos_manage_arithmetic);
					vty_out(vty,"qos grab arithmetic name	%s\n",WQOS->qos[0]->qos_res_grab_arithmetic);
					vty_out(vty,"qos shove arithmetic name	%s\n",WQOS->qos[0]->qos_res_shove_arithmetic);
					vty_out(vty,"qos grab arithmetic	%s\n",(WQOS->qos[0]->qos_use_res_grab == 1)?enable:disable);
					vty_out(vty,"qos shove arithmetic	%s\n",(WQOS->qos[0]->qos_use_res_shove == 1)?enable:disable);
					vty_out(vty,"----------\n");
					vty_out(vty,"radio qos extension infomation\n");
					vty_out(vty,"----------\n");
					vty_out(vty,"BESTEFFORT\n");
					vty_out(vty,"BESTEFFORT:  average rate	%d\n",WQOS->qos[0]->radio_qos[0]->qos_average_rate);
					vty_out(vty,"BESTEFFORT:  max burstiness	%d\n",WQOS->qos[0]->radio_qos[0]->qos_max_degree);													
					vty_out(vty,"BESTEFFORT:  manage policy priority	%d\n",WQOS->qos[0]->radio_qos[0]->qos_policy_pri);	
					vty_out(vty,"BESTEFFORT:  shove priority	%d\n",WQOS->qos[0]->radio_qos[0]->qos_res_shove_pri);	
					vty_out(vty,"BESTEFFORT:  grab priority %d\n",WQOS->qos[0]->radio_qos[0]->qos_res_grab_pri);	
					vty_out(vty,"BESTEFFORT:  max parallel	%d\n",WQOS->qos[0]->radio_qos[0]->qos_max_parallel);	
					vty_out(vty,"BESTEFFORT:  bandwidth %d\n",WQOS->qos[0]->radio_qos[0]->qos_bandwidth);	
					vty_out(vty,"BESTEFFORT:  bandwidth percentage	%d\n",WQOS->qos[0]->radio_qos[0]->qos_bandwidth_scale);
					vty_out(vty,"BESTEFFORT:  use WRED	%s\n",(WQOS->qos[0]->radio_qos[0]->qos_use_wred == 1)?enable:disable);	
					vty_out(vty,"BESTEFFORT:  use traffic shaping	%s\n",(WQOS->qos[0]->radio_qos[0]->qos_use_traffic_shaping == 1)?enable:disable);	
					vty_out(vty,"----------\n");
					vty_out(vty,"BACKGROUND\n");
					vty_out(vty,"BACKGROUND:  average rate	%d\n",WQOS->qos[0]->radio_qos[1]->qos_average_rate);
					vty_out(vty,"BACKGROUND:  max burstiness	%d\n",WQOS->qos[0]->radio_qos[1]->qos_max_degree);													
					vty_out(vty,"BACKGROUND:  manage policy priority	%d\n",WQOS->qos[0]->radio_qos[1]->qos_policy_pri);	
					vty_out(vty,"BACKGROUND:  shove priority	%d\n",WQOS->qos[0]->radio_qos[1]->qos_res_shove_pri);	
					vty_out(vty,"BACKGROUND:  grab priority %d\n",WQOS->qos[0]->radio_qos[1]->qos_res_grab_pri);	
					vty_out(vty,"BACKGROUND:  max parallel	%d\n",WQOS->qos[0]->radio_qos[1]->qos_max_parallel);	
					vty_out(vty,"BACKGROUND:  bandwidth %d\n",WQOS->qos[0]->radio_qos[1]->qos_bandwidth);	
					vty_out(vty,"BACKGROUND:  bandwidth percentage	%d\n",WQOS->qos[0]->radio_qos[1]->qos_bandwidth_scale);
					vty_out(vty,"BACKGROUND:  use WRED	%s\n",(WQOS->qos[0]->radio_qos[1]->qos_use_wred == 1)?enable:disable);	
					vty_out(vty,"BACKGROUND:  use traffic shaping	%s\n",(WQOS->qos[0]->radio_qos[1]->qos_use_traffic_shaping == 1)?enable:disable);	
					vty_out(vty,"----------\n");
					vty_out(vty,"VIDEO\n");
					vty_out(vty,"VIDEO: 	  average rate	%d\n",WQOS->qos[0]->radio_qos[2]->qos_average_rate);
					vty_out(vty,"VIDEO: 	  max burstiness	%d\n",WQOS->qos[0]->radio_qos[2]->qos_max_degree);													
					vty_out(vty,"VIDEO: 	  manage policy priority	%d\n",WQOS->qos[0]->radio_qos[2]->qos_policy_pri);	
					vty_out(vty,"VIDEO: 	  shove priority	%d\n",WQOS->qos[0]->radio_qos[2]->qos_res_shove_pri);	
					vty_out(vty,"VIDEO: 	  grab priority %d\n",WQOS->qos[0]->radio_qos[2]->qos_res_grab_pri);	
					vty_out(vty,"VIDEO: 	  max parallel	%d\n",WQOS->qos[0]->radio_qos[2]->qos_max_parallel);	
					vty_out(vty,"VIDEO: 	  bandwidth %d\n",WQOS->qos[0]->radio_qos[2]->qos_bandwidth);	
					vty_out(vty,"VIDEO: 	  bandwidth percentage	%d\n",WQOS->qos[0]->radio_qos[2]->qos_bandwidth_scale);
					vty_out(vty,"VIDEO: 	  use WRED	%s\n",(WQOS->qos[0]->radio_qos[2]->qos_use_wred == 1)?enable:disable);	
					vty_out(vty,"VIDEO: 	  use traffic shaping	%s\n",(WQOS->qos[0]->radio_qos[2]->qos_use_traffic_shaping == 1)?enable:disable);	
					vty_out(vty,"VIDEO: 	  use flow equity queue %s\n",(WQOS->qos[0]->radio_qos[2]->qos_use_flow_eq_queue == 1)?enable:disable); 
					vty_out(vty,"VIDEO: 	  flow queue lenth	%d\n",WQOS->qos[0]->radio_qos[2]->qos_flow_max_queuedepth); 
					vty_out(vty,"VIDEO: 	  flow average rate %d\n",WQOS->qos[0]->radio_qos[2]->qos_flow_average_rate);	
					vty_out(vty,"VIDEO: 	  flow max burstiness	%d\n",WQOS->qos[0]->radio_qos[2]->qos_flow_max_degree); 
					vty_out(vty,"----------\n");
					vty_out(vty,"VOICE\n");
					vty_out(vty,"VOICE: 	  average rate	%d\n",WQOS->qos[0]->radio_qos[3]->qos_average_rate);
					vty_out(vty,"VOICE: 	  max burstiness	%d\n",WQOS->qos[0]->radio_qos[3]->qos_max_degree);													
					vty_out(vty,"VOICE: 	  manage policy priority	%d\n",WQOS->qos[0]->radio_qos[3]->qos_policy_pri);	
					vty_out(vty,"VOICE: 	  shove priority	%d\n",WQOS->qos[0]->radio_qos[3]->qos_res_shove_pri);	
					vty_out(vty,"VOICE: 	  grab priority %d\n",WQOS->qos[0]->radio_qos[3]->qos_res_grab_pri);	
					vty_out(vty,"VOICE: 	  max parallel	%d\n",WQOS->qos[0]->radio_qos[3]->qos_max_parallel);	
					vty_out(vty,"VOICE: 	  bandwidth %d\n",WQOS->qos[0]->radio_qos[3]->qos_bandwidth);	
					vty_out(vty,"VOICE: 	  bandwidth percentage	%d\n",WQOS->qos[0]->radio_qos[3]->qos_bandwidth_scale);
					vty_out(vty,"VOICE: 	  use WRED	%s\n",(WQOS->qos[0]->radio_qos[3]->qos_use_wred == 1)?enable:disable);	
					vty_out(vty,"VOICE: 	  use traffic shaping	%s\n",(WQOS->qos[0]->radio_qos[3]->qos_use_traffic_shaping == 1)?enable:disable);	
					vty_out(vty,"VOICE: 	  use flow equity queue %s\n",(WQOS->qos[0]->radio_qos[3]->qos_use_flow_eq_queue == 1)?enable:disable); 
					vty_out(vty,"VOICE: 	  flow queue lenth	%d\n",WQOS->qos[0]->radio_qos[3]->qos_flow_max_queuedepth); 
					vty_out(vty,"VOICE: 	  flow average rate %d\n",WQOS->qos[0]->radio_qos[3]->qos_flow_average_rate);	
					vty_out(vty,"VOICE: 	  flow max burstiness	%d\n",WQOS->qos[0]->radio_qos[3]->qos_flow_max_degree);
					vty_out(vty,"\n");
					dcli_wqos_free_fun(WID_DBUS_QOS_METHOD_SHOW_QOS_EXTENSION_INFO,WQOS);
				}
				else if(ret == WTP_ID_NOT_EXIST)
				{
					vty_out(vty,"<error> qos id does not exist\n");
				}
				else 
				{	
					vty_out(vty,"<error>  %d\n",ret);
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

DEFUN(wid_config_set_qos_total_bandwidth_cmd_func,
		wid_config_set_qos_total_bandwidth_cmd,
		"set wireless qos total bandwidth BANDWIDTH",
		CONFIG_STR
		"Wireless Configuration\n"
		"Qos Configuration\n"
		"total bandwidth\n"
		"total bandwidth\n"
		"bandwidth 1-25M\n"
	)
{
	int ret;
	unsigned char bandwidth = 0;	
	int ID = 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//ID = (int)vty->index;
	
	ret = wid_wqos_parse_char_ID((char*)argv[0], &bandwidth);
	
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(bandwidth > 25){
		vty_out(vty,"<error> qos dot1p should be 1 to 25\n");
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WQOS_NODE){
		index = 0;			
		ID = (int)vty->index;
	}else if(vty->node == HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_INTERFACE,INTERFACE); 
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_TOTAL_BANDWIDTH);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_TOTAL_BANDWIDTH);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&ID,								
						DBUS_TYPE_BYTE,&bandwidth,						 
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

		if(ret == 0)
			vty_out(vty,"set qos profile %d total bandwidth %s successfully\n",ID,argv[0]);
		else if(ret == WID_QOS_NOT_EXIST)
			vty_out(vty,"<error> qos id does not exist\n");			
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;	
}
DEFUN(wid_config_set_qos_flow_parameter_cmd_func,
		wid_config_set_qos_flow_parameter_cmd,
		/*"set (besteffort|background|video|voice) PARAMETER VALUE",*/
		"set (besteffort|background|video|voice) (averagerate|maxburstiness|managepriority|shovepriority|grabpriority|maxparallel|bandwidth|bandwidthpercentage|flowqueuelenth|flowaveragerate|flowmaxburstiness) VALUE",
		CONFIG_STR
		"Qos flow Configuration\n"
		"Qos flow parameter Configuration\n"
		"flow parameter value\n"
	)
{
	int ret;
	unsigned int value = 0;	
	int ID = 0;
	unsigned char qos_stream_id = 0;
	unsigned int paramater_type = 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//ID = (int)vty->index;

	if ((!strcmp(argv[0],"BESTEFFORT"))||(!strcmp(argv[0],"besteffort")))
	{
		qos_stream_id = 0;
	}
	else if ((!strcmp(argv[0],"BACKGROUND"))||(!strcmp(argv[0],"background")))
	{
		qos_stream_id = 1;
	}
	else if ((!strcmp(argv[0],"VIDEO"))||(!strcmp(argv[0],"video")))
	{
		qos_stream_id = 2;	
	}
	else if ((!strcmp(argv[0],"VOICE"))||(!strcmp(argv[0],"voice")))
	{	
		qos_stream_id = 3;			
	}
	else 
	{		
		vty_out(vty,"<error> unknown qos flow type.\n");
		return CMD_SUCCESS;
	}

	if (!strcmp(argv[1],"averagerate"))
	{
		paramater_type = 1;
	}
	else if (!strcmp(argv[1],"maxburstiness"))
	{
		paramater_type = 2;
	}
	else if (!strcmp(argv[1],"managepriority"))
	{
		paramater_type = 3;	
	}
	else if (!strcmp(argv[1],"shovepriority"))
	{	
		paramater_type = 4;			
	}
	else if (!strcmp(argv[1],"grabpriority"))
	{	
		paramater_type = 5;			
	}
	else if (!strcmp(argv[1],"maxparallel"))
	{
		paramater_type = 6;	
	}
	else if (!strcmp(argv[1],"bandwidth"))
	{	
		paramater_type = 7;			
	}
	else if (!strcmp(argv[1],"bandwidthpercentage"))
	{	
		paramater_type = 8;			
	}
	else if (!strcmp(argv[1],"flowqueuelenth"))
	{	
		paramater_type = 9;			
	}
	else if (!strcmp(argv[1],"flowaveragerate"))
	{	
		paramater_type = 10;			
	}
	else if (!strcmp(argv[1],"flowmaxburstiness"))
	{	
		paramater_type = 11;			
	}
	else 
	{		
		vty_out(vty,"<error> unknown qos parameter type.\n");
		return CMD_SUCCESS;
	}
	/*
	ret = parse_qos_flow_parameter_type((char*)argv[1],paramater_type);
	
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown flow parameter format\n");
		return CMD_SUCCESS;
	}
	*/
	ret = parse_int_ID((char*)argv[2], &value);
	
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown value format\n");
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WQOS_NODE){
		index = 0;			
		ID = (int)vty->index;
	}else if(vty->node == HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_INTERFACE,INTERFACE); 
 
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_FLOW_PARAMETER);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_FLOW_PARAMETER);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&ID,								
						DBUS_TYPE_BYTE,&qos_stream_id,	
						DBUS_TYPE_UINT32,&paramater_type,
						DBUS_TYPE_UINT32,&value,
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

		if(ret == 0)
			vty_out(vty,"set qos profile %s %s %s successfully\n",argv[0],argv[1],argv[2]);
		else if(ret == WID_QOS_NOT_EXIST)
			vty_out(vty,"<error> qos id does not exist\n");			
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;	
}
DEFUN(wid_config_set_qos_parameter_cmd_func,
		wid_config_set_qos_parameter_cmd,
		"set qos (totalbandwidth|resourcescale|sharebandwidth|resourcesharescale) VALUE",
		/*"set qos PARAMETER VALUE",*/
		CONFIG_STR
		"Qos Configuration\n"
		"Qos parameter Configuration\n"
		"flow parameter value\n"
	)
{
	int ret;
	unsigned int value = 0;	
	int ID = 0;
	unsigned int paramater_type = 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//ID = (int)vty->index;

	/*
	ret = parse_qos_parameter_type((char*)argv[0],paramater_type);
	
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown flow parameter format\n");
		return CMD_SUCCESS;
	}
	*/
	if (!strcmp(argv[0],"totalbandwidth"))
	{
		paramater_type = 1;
	}
	else if (!strcmp(argv[0],"resourcescale"))
	{
		paramater_type = 2;
	}
	else if (!strcmp(argv[0],"sharebandwidth"))
	{
		paramater_type = 3;	
	}
	else if (!strcmp(argv[0],"resourcesharescale"))
	{	
		paramater_type = 4;			
	}
	else 
	{		
		vty_out(vty,"<error> unknown qos parameter type.\n");
		return CMD_SUCCESS;
	}
	
	ret = parse_int_ID((char*)argv[1], &value);
	
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown value format\n");
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WQOS_NODE){
		index = 0;			
		ID = (int)vty->index;
	}else if(vty->node == HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_INTERFACE,INTERFACE); 
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_PARAMETER);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_PARAMETER);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&ID,								
						DBUS_TYPE_UINT32,&paramater_type,
						DBUS_TYPE_UINT32,&value,
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

		if(ret == 0)
			vty_out(vty,"set qos profile %s %s successfully\n",argv[0],argv[1]);
		else if(ret == WID_QOS_NOT_EXIST)
			vty_out(vty,"<error> qos id does not exist\n");			
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;	
}
DEFUN(wid_config_set_qos_policy_used_cmd_func,
		wid_config_set_qos_policy_used_cmd,
		"set qos (grab|shove) policy (used|unused)",
		CONFIG_STR
		"Qos Configuration\n"
		"grab or shove\n"
		"Qos policy Configuration\n"
		"uesd or unused\n"
	)
{
	int ret;
	unsigned int value = 0;	
	int ID = 0;
	unsigned char policy = 0;
	unsigned char used = 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//ID = (int)vty->index;


	if (!strcmp(argv[0],"grab"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[0],"shove"))
	{	
		policy = 2;			
	}
	else 
	{		
		vty_out(vty,"<error> unknown qos policy type.\n");
		return CMD_SUCCESS;
	}

	if (!strcmp(argv[1],"used"))
	{
		used = 1;	
	}
	else if (!strcmp(argv[1],"unused"))
	{	
		used = 0;			
	}
	else 
	{		
		vty_out(vty,"<error> unknown qos policy type.\n");
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WQOS_NODE){
		index = 0;			
		ID = (int)vty->index;
	}else if(vty->node == HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
	    slot_id = vty->slotindex;
	}
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_INTERFACE,INTERFACE); 
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_POLICY);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_POLICY);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&ID,								
						DBUS_TYPE_BYTE,&policy,	
						DBUS_TYPE_BYTE,&used,
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

		if(ret == 0)
			vty_out(vty,"set qos profile %s policy %s successfully\n",argv[0],argv[1]);
		else if(ret == WID_QOS_NOT_EXIST)
			vty_out(vty,"<error> qos id does not exist\n");			
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;	
}
DEFUN(wid_config_set_qos_policy_name_cmd_func,
		wid_config_set_qos_policy_name_cmd,
		"set qos (grab|shove) policy name NAME",
		CONFIG_STR
		"Qos Configuration\n"
		"grab or shove\n"
		"Qos policy Configuration\n"
		"Qos policy name\n"
		"name lenth size 1-15\n"
	)
{
	int ret;
	int ID = 0;
	unsigned char policy = 0;
	char *name;
	unsigned int lenth = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
//	ID = (int)vty->index;

	lenth = strlen((char *)argv[1]);

	if(lenth > 15)
	{		
		vty_out(vty,"<error> policy name too long\n");
		return CMD_SUCCESS;
	}
	
	name = (char *)malloc(lenth+1);
	memset(name,0,lenth+1);
	memcpy(name,argv[1],lenth);
	
	if (!strcmp(argv[0],"grab"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[0],"shove"))
	{	
		policy = 2;			
	}
	else 
	{		
		vty_out(vty,"<error> unknown qos policy type.\n");
		if(name)
		{
			free(name);
			name = NULL;
		}
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WQOS_NODE){
		index = 0;			
		ID = (int)vty->index;
	}else if(vty->node == HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_INTERFACE,INTERFACE); 
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_POLICY_NAME);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_POLICY_NAME);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&ID,								
						DBUS_TYPE_BYTE,&policy,	
						DBUS_TYPE_STRING,&name,
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		if(name)
		{
			free(name);
			name = NULL;
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == 0)
			vty_out(vty,"set qos profile %s policy name %s successfully\n",argv[0],argv[1]);
		else if(ret == WID_QOS_NOT_EXIST)
			vty_out(vty,"<error> qos id does not exist\n");			
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	if(name)
	{
		free(name);
		name = NULL;
	}
	return CMD_SUCCESS;	
}
DEFUN(wid_config_set_qos_manage_arithmetic_name_cmd_func,
		wid_config_set_qos_manage_arithmetic_name_cmd,
		"set qos manage arithmetic name NAME",
		CONFIG_STR
		"Qos Configuration\n"
		"manage arithmetic\n"
		"manage arithmetic\n"
		"manage arithmetic name\n"
		"name lenth size 1-15\n"
	)
{
	int ret;
	int ID = 0;
	char *name;
	unsigned int lenth = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//ID = (int)vty->index;

	lenth = strlen((char *)argv[0]);

	if(lenth > 15)
	{		
		vty_out(vty,"<error> arithmetic name too long\n");
		return CMD_SUCCESS;
	}
	
	name = (char *)malloc(lenth+1);
	memset(name,0,lenth+1);
	memcpy(name,argv[0],lenth);
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WQOS_NODE){
		index = 0;			
		ID = (int)vty->index;
	}else if(vty->node == HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_INTERFACE,INTERFACE); 
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_MANAGE_ARITHMETIC_NAME);
		
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_MANAGE_ARITHMETIC_NAME);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&ID,	
						DBUS_TYPE_STRING,&name,
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		if(name)
		{
			free(name);
			name = NULL;
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == 0)
			vty_out(vty,"set qos profile manage arithmetic name %s successfully\n",argv[0]);
		else if(ret == WID_QOS_NOT_EXIST)
			vty_out(vty,"<error> qos id does not exist\n");			
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	if(name)
	{
		free(name);
		name = NULL;
	}
	return CMD_SUCCESS;	
}

DEFUN(wid_show_qos_radio_cmd_func,
	  wid_show_qos_radio_cmd,
	  "show qos info by wtp WTPID radio RADIOID type (besteffort|background|video|voice)",
	  SHOW_STR
	  "wireless service\n"
	  "Display qos profile information by local radio\n"
	  "qos profile\n"
	  "radio apply qos profile\n"
	 )
{	
	int ret,ret2;
	int radio_l_id = 0;
	int	WTPID = 0;
	char *name;
	int i = 0;
	int j = 0;
	int k = 0;
	int num = 0;
	int qostype = 0;
	int qos_id = 0;
	
	
	qos_profile *radio_qos[DCLIWQOS_QOS_FLOW_NUM];
	qos_profile *client_qos[DCLIWQOS_QOS_FLOW_NUM];

	char ack[] = "ACK";
	char noack[] = "NOACK";
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	DBusError err;
	
	ret = parse_int_ID((char*)argv[0], &WTPID);
	ret2 = parse_int_ID((char*)argv[1], &radio_l_id);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown wtpid format\n");
		return CMD_SUCCESS;
	}	
	if(ret2 != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown local radio id format\n");
		return CMD_SUCCESS;
	}
	if(WTPID >= WTP_NUM || WTPID == 0){
		vty_out(vty,"<error> wtp id should be 1 to %d\n",WTP_NUM-1);
		return CMD_SUCCESS;
	}
	if(radio_l_id >= 4){
		vty_out(vty,"<error> local radio id should be 0 to 3.\n");
		return CMD_SUCCESS;
	}
	
	if ((!strcmp(argv[2],"BESTEFFORT"))||(!strcmp(argv[2],"besteffort")))
	{
		qostype = 0;
	}
	else if ((!strcmp(argv[2],"BACKGROUND"))||(!strcmp(argv[2],"background")))
	{
		qostype = 1;
	}
	else if ((!strcmp(argv[2],"VIDEO"))||(!strcmp(argv[2],"video")))
	{
		qostype = 2;	
	}
	else if ((!strcmp(argv[2],"VOICE"))||(!strcmp(argv[2],"voice")))
	{	
		qostype = 3;			
	}
	else 
	{		
		vty_out(vty,"<error> unknown qos flow type.\n");
		vty_out(vty,"<error> input parameter should be only 'voice' 'video' 'besteffort' or 'background'\n");
		return CMD_SUCCESS;
	}
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		 index = vty->index;
		 localid = vty->local;
		 slot_id = vty->slotindex;
	}else if (vty->node == LOCAL_HANSI_NODE){
	    index = vty->index;
	    localid = vty->local;
	    slot_id = vty->slotindex;
	}
#if 0	
	ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ReInitDbusPath(index,WID_DBUS_QOS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SHOW_RADIO_QOS_INFO);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SHOW_RADIO_QOS_INFO);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&WTPID,
					 		 DBUS_TYPE_UINT32,&radio_l_id,
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
	
	if(ret == 0 )
	{	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&qos_id);		
		
		dbus_message_unref(reply);/*free reply before  return*/
	}
#endif
	DCLI_WQOS *WQOS = NULL;
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	WQOS = (DCLI_WQOS*)dcli_wqos_wireless_qos_show_config_info
		(
			index,
			&ret,
			localid,
			WTPID,			
			radio_l_id,
		//	qos,
			dcli_dbus_connection,
			WID_DBUS_QOS_METHOD_SHOW_RADIO_QOS_INFO
		);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){
		qos_id = WQOS->qos[0]->QosID;
		
		dcli_wqos_free_fun(WID_DBUS_QOS_METHOD_SHOW_RADIO_QOS_INFO,WQOS);
	//	dbus_message_unref(reply);/*free reply before  return*/
	}
	else {
		
		
		if(ret == WTP_ID_NOT_EXIST){
			vty_out(vty,"<error> wtp id not exist.\n");
		}
		else if(ret == RADIO_ID_NOT_EXIST){
			vty_out(vty,"<error> radio id not exist.\n");
		}
		else if(ret == WID_QOS_NOT_EXIST){
			vty_out(vty,"<error> qos id not exist.\n");
		}
		else if(ret == RADIO_NO_BINDING_WQOS){
			vty_out(vty,"<error> wtp %d radio %d didn't bind qos.\n",WTPID,radio_l_id);
		}
		else if(qos_id >= QOS_NUM || qos_id == 0){
			vty_out(vty,"<error> qos id is %d,but it should be 1 to %d\n",qos_id,QOS_NUM-1);
		}
		else{
			vty_out(vty,"<error> ret is %d.\n",ret);
		}
		//dbus_message_unref(reply);
		return CMD_SUCCESS;	
	}
#if 0	
	ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ReInitDbusPath(index,WID_DBUS_QOS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SHOW_QOS);
		
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SHOW_QOS);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&qos_id,
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
	
	if(ret == 0 )
	{	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&qos_id);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&name);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for (i = 0; i < 4; i++) {
			DBusMessageIter iter_struct;
			
			radio_qos[i] = (qos_profile *)malloc(sizeof(qos_profile));
			client_qos[i] = (qos_profile *)malloc(sizeof(qos_profile));
			
			dbus_message_iter_recurse(&iter_array,&iter_struct);

			/*radio qos info*/
		
			dbus_message_iter_get_basic(&iter_struct,&(radio_qos[i]->QueueDepth));
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(radio_qos[i]->CWMin));
		
			dbus_message_iter_next(&iter_struct);
	
			dbus_message_iter_get_basic(&iter_struct,&(radio_qos[i]->CWMax));
		
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(radio_qos[i]->AIFS));
					
			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(radio_qos[i]->TXOPlimit));
			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(radio_qos[i]->ACK));

			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(radio_qos[i]->mapstate));

			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(radio_qos[i]->wmm_map_dot1p));

			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(radio_qos[i]->dot1p_map_wmm_num));

			for(j=0;j<8;j++)
			{
				dbus_message_iter_next(&iter_struct);
					
				dbus_message_iter_get_basic(&iter_struct,&(radio_qos[i]->dot1p_map_wmm[j]));

			}			

			/*client qos info*/
	
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(client_qos[i]->QueueDepth));
					
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(client_qos[i]->CWMin));
		
			dbus_message_iter_next(&iter_struct);
	
			dbus_message_iter_get_basic(&iter_struct,&(client_qos[i]->CWMax));
		
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(client_qos[i]->AIFS));
					
			dbus_message_iter_next(&iter_struct);
					
			dbus_message_iter_get_basic(&iter_struct,&(client_qos[i]->TXOPlimit));
			dbus_message_iter_next(&iter_array);
		}
	}
	
#endif
	WQOS = (DCLI_WQOS*)dcli_wqos_wireless_qos_show_config_info
		(
			index,
			&ret,
			0,
			qos_id,			
			0,
		//	qos,
			dcli_dbus_connection,
			WID_DBUS_QOS_METHOD_SHOW_QOS
		);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0){
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"wtp %d local radio %d apply Wireless Qos %d.\n",WTPID,radio_l_id,qos_id);
		vty_out(vty,"Wireless Qos profile name: %s\n",WQOS->qos[0]->name);
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"radio qos infomation\n");
		vty_out(vty,"             QueueDepth CWMin CWMax AIFS TXOPlimit    ACK\n");
		if(qostype == 0)		
		vty_out(vty,"BESTEFFORT:  %10d %5d %5d %4d %9d %6s\n",WQOS->qos[0]->radio_qos[0]->QueueDepth
															,WQOS->qos[0]->radio_qos[0]->CWMin
															,WQOS->qos[0]->radio_qos[0]->CWMax
															,WQOS->qos[0]->radio_qos[0]->AIFS
															,WQOS->qos[0]->radio_qos[0]->TXOPlimit
															,(WQOS->qos[0]->radio_qos[0]->ACK == 1)?ack:noack);
		else if(qostype == 1)		
		vty_out(vty,"BACKGROUND:  %10d %5d %5d %4d %9d %6s\n",WQOS->qos[0]->radio_qos[1]->QueueDepth
															,WQOS->qos[0]->radio_qos[1]->CWMin
															,WQOS->qos[0]->radio_qos[1]->CWMax
															,WQOS->qos[0]->radio_qos[1]->AIFS
															,WQOS->qos[0]->radio_qos[1]->TXOPlimit
															,(WQOS->qos[0]->radio_qos[1]->ACK == 1)?ack:noack);
		else if(qostype == 2)		
		vty_out(vty,"VIDEO:       %10d %5d %5d %4d %9d %6s\n",WQOS->qos[0]->radio_qos[2]->QueueDepth
															,WQOS->qos[0]->radio_qos[2]->CWMin
															,WQOS->qos[0]->radio_qos[2]->CWMax
															,WQOS->qos[0]->radio_qos[2]->AIFS
															,WQOS->qos[0]->radio_qos[2]->TXOPlimit
															,(WQOS->qos[0]->radio_qos[2]->ACK == 1)?ack:noack);
		else if(qostype == 3)		
		vty_out(vty,"VOICE:       %10d %5d %5d %4d %9d %6s\n",WQOS->qos[0]->radio_qos[3]->QueueDepth
															,WQOS->qos[0]->radio_qos[3]->CWMin
															,WQOS->qos[0]->radio_qos[3]->CWMax
															,WQOS->qos[0]->radio_qos[3]->AIFS
															,WQOS->qos[0]->radio_qos[3]->TXOPlimit
															,(WQOS->qos[0]->radio_qos[3]->ACK == 1)?ack:noack);
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"client qos infomation\n");
		vty_out(vty,"             QueueDepth CWMin CWMax AIFS TXOPlimit\n");
	if(qostype == 0)	
			vty_out(vty,"BESTEFFORT:  %10d %5d %5d %4d %9d\n",WQOS->qos[0]->client_qos[0]->QueueDepth
																,WQOS->qos[0]->client_qos[0]->CWMin
																,WQOS->qos[0]->client_qos[0]->CWMax
																,WQOS->qos[0]->client_qos[0]->AIFS
																,WQOS->qos[0]->client_qos[0]->TXOPlimit);
		else if(qostype == 1)		
			vty_out(vty,"BACKGROUND:  %10d %5d %5d %4d %9d\n",WQOS->qos[0]->client_qos[1]->QueueDepth
																,WQOS->qos[0]->client_qos[1]->CWMin
																,WQOS->qos[0]->client_qos[1]->CWMax
																,WQOS->qos[0]->client_qos[1]->AIFS
																,WQOS->qos[0]->client_qos[1]->TXOPlimit);
		else if(qostype == 2)
			vty_out(vty,"VIDEO:       %10d %5d %5d %4d %9d\n",WQOS->qos[0]->client_qos[2]->QueueDepth
																,WQOS->qos[0]->client_qos[2]->CWMin
																,WQOS->qos[0]->client_qos[2]->CWMax
																,WQOS->qos[0]->client_qos[2]->AIFS
																,WQOS->qos[0]->client_qos[2]->TXOPlimit);
		else if(qostype == 3)		
			vty_out(vty,"VOICE:       %10d %5d %5d %4d %9d\n",WQOS->qos[0]->client_qos[3]->QueueDepth
																,WQOS->qos[0]->client_qos[3]->CWMin
																,WQOS->qos[0]->client_qos[3]->CWMax
																,WQOS->qos[0]->client_qos[3]->AIFS
																,WQOS->qos[0]->client_qos[3]->TXOPlimit);
			
			
			vty_out(vty,"==============================================================================\n");
		dcli_wqos_free_fun(WID_DBUS_QOS_METHOD_SHOW_QOS,WQOS);
	}
	else if(ret == WTP_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> qos id does not exist\n");
	}
	else 
	{	
		vty_out(vty,"<error>error no. is  %d\n",ret);
	}
//	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(wid_config_set_qos_flow_parameter_cmd_func_v2,
		wid_config_set_qos_flow_parameter_cmd_v2,
		"set (video|voice) (flowqueuelenth|flowaveragerate|flowmaxburstiness) VALUE",
		CONFIG_STR
		"Qos flow Configuration\n"
		"Qos flow parameter Configuration\n"
		"flow parameter value\n"
	)
{
	int ret;
	unsigned int value = 0;	
	int ID = 0;
	unsigned char qos_stream_id = 0;
	unsigned int paramater_type = 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//ID = (int)vty->index;

	if ((!strcmp(argv[0],"BESTEFFORT"))||(!strcmp(argv[0],"besteffort")))
	{
		qos_stream_id = 0;
	}
	else if ((!strcmp(argv[0],"BACKGROUND"))||(!strcmp(argv[0],"background")))
	{
		qos_stream_id = 1;
	}
	else if ((!strcmp(argv[0],"VIDEO"))||(!strcmp(argv[0],"video")))
	{
		qos_stream_id = 2;	
	}
	else if ((!strcmp(argv[0],"VOICE"))||(!strcmp(argv[0],"voice")))
	{	
		qos_stream_id = 3;			
	}
	else 
	{		
		vty_out(vty,"<error> unknown qos flow type.\n");
		return CMD_SUCCESS;
	}

	if (!strcmp(argv[1],"flowqueuelenth"))
	{
		paramater_type = 9;
	}
	else if (!strcmp(argv[1],"flowaveragerate"))
	{
		paramater_type = 10;
	}
	else if (!strcmp(argv[1],"flowmaxburstiness"))
	{
		paramater_type = 11;	
	}
	else 
	{		
		vty_out(vty,"<error> unknown qos parameter type.\n");
		return CMD_SUCCESS;
	}
	/*
	ret = parse_qos_flow_parameter_type((char*)argv[1],paramater_type);
	
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown flow parameter format\n");
		return CMD_SUCCESS;
	}
	*/
	ret = parse_int_ID((char*)argv[2], &value);
	//printf("value here is %d.\n",value);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown value format\n");
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WQOS_NODE){
		index = 0;			
		ID = (int)vty->index;
	}else if(vty->node == HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
	    slot_id = vty->slotindex;
	}
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_INTERFACE,INTERFACE); 
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_FLOW_PARAMETER_V2);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_FLOW_PARAMETER_V2);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&ID,								
						DBUS_TYPE_BYTE,&qos_stream_id,	
						DBUS_TYPE_UINT32,&paramater_type,
						DBUS_TYPE_UINT32,&value,
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

		if(ret == 0)
			vty_out(vty,"set qos profile %s %s %s successfully\n",argv[0],argv[1],argv[2]);
		else if(ret == WID_QOS_NOT_EXIST)
			vty_out(vty,"<error> qos id does not exist\n");			
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;	
}
DEFUN(wid_config_set_qos_flow_able_cmd_func,
		wid_config_set_qos_flow_able_cmd,
		"set (besteffort|background|video|voice) (usetrafficshaping|usewred) (enable|disable)",
		CONFIG_STR
		"Qos flow Configuration\n"
		"Qos flow parameter Configuration\n"
		"flow parameter value\n"
	)
{
	int ret;
	unsigned int value = 0;	
	int ID = 0;
	unsigned char qos_stream_id = 0;
	unsigned int able_type = 0;
	unsigned int use_type = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	ID = (int)vty->index;

	if ((!strcmp(argv[0],"BESTEFFORT"))||(!strcmp(argv[0],"besteffort")))
	{
		qos_stream_id = 0;
	}
	else if ((!strcmp(argv[0],"BACKGROUND"))||(!strcmp(argv[0],"background")))
	{
		qos_stream_id = 1;
	}
	else if ((!strcmp(argv[0],"VIDEO"))||(!strcmp(argv[0],"video")))
	{
		qos_stream_id = 2;	
	}
	else if ((!strcmp(argv[0],"VOICE"))||(!strcmp(argv[0],"voice")))
	{	
		qos_stream_id = 3;			
	}
	else 
	{		
		vty_out(vty,"<error> unknown qos flow type(first parameter).\n");
		return CMD_SUCCESS;
	}
	if (!strcmp(argv[1],"usetrafficshaping"))
	{
		use_type = 1;
	}
	else if (!strcmp(argv[1],"usewred"))
	{
		use_type = 2;
	}
	else 
	{		
		vty_out(vty,"<error> unknown qos sencond parameter type.\n");
		return CMD_SUCCESS;
	}

	if (!strcmp(argv[2],"enable"))
	{
		able_type = 1;
	}
	else if (!strcmp(argv[2],"disable"))
	{
		able_type = 0;
	}
	else 
	{		
		vty_out(vty,"<error> unknown qos parameter type(third parameter).\n");
		return CMD_SUCCESS;
	}
	/*
	ret = parse_qos_flow_parameter_type((char*)argv[1],paramater_type);
	
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown flow parameter format\n");
		return CMD_SUCCESS;
	}
	*/
	//ret = parse_int_ID((char*)argv[2], &value);
	
/*	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown value format\n");
		return CMD_SUCCESS;
	}*/

	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WQOS_NODE){
		index = 0;			
		ID = (int)vty->index;
	}else if(vty->node == HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_INTERFACE,INTERFACE); 
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_FLOW_ABLE);

	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_FLOW_ABLE);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&ID,								
						DBUS_TYPE_BYTE,&qos_stream_id,	
						DBUS_TYPE_UINT32,&use_type,
						DBUS_TYPE_UINT32,&able_type,
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

		if(ret == 0)
			vty_out(vty,"set qos profile %s %s successfully\n",argv[0],argv[1]);
		else if(ret == WID_QOS_NOT_EXIST)
			vty_out(vty,"<error> qos id does not exist\n");			
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;	
}

DEFUN(wid_config_set_qos_flow_able_cmd_func_v2,
		wid_config_set_qos_flow_able_cmd_v2,
		"set (video|voice) useflowequityqueue (enable|disable)",
		CONFIG_STR
		"Qos flow Configuration\n"
		"Qos flow parameter Configuration\n"
		"flow parameter value\n"
	)
{
	int ret;
	unsigned int value = 0;	
	int ID = 0;
	unsigned char qos_stream_id = 0;
	unsigned int able_type = 0;
	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	ID = (int)vty->index;

	if ((!strcmp(argv[0],"BESTEFFORT"))||(!strcmp(argv[0],"besteffort")))
	{
		qos_stream_id = 0;
	}
	else if ((!strcmp(argv[0],"BACKGROUND"))||(!strcmp(argv[0],"background")))
	{
		qos_stream_id = 1;
	}
	else if ((!strcmp(argv[0],"VIDEO"))||(!strcmp(argv[0],"video")))
	{
		qos_stream_id = 2;	
	}
	else if ((!strcmp(argv[0],"VOICE"))||(!strcmp(argv[0],"voice")))
	{	
		qos_stream_id = 3;			
	}
	else 
	{		
		vty_out(vty,"<error> unknown qos flow type.\n");
		return CMD_SUCCESS;
	}

	if (!strcmp(argv[1],"enable"))
	{
		able_type = 1;
	}
	else if (!strcmp(argv[1],"disable"))
	{
		able_type = 0;
	}
	else 
	{		
		vty_out(vty,"<error> unknown qos parameter type.\n");
		return CMD_SUCCESS;
	}
	/*
	ret = parse_qos_flow_parameter_type((char*)argv[1],paramater_type);
	
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown flow parameter format\n");
		return CMD_SUCCESS;
	}
	*/
	/*ret = parse_int_ID((char*)argv[2], &value);
	
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown value format\n");
		return CMD_SUCCESS;
	}*/


	int index = 0;
	int localid = 1;
    int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WQOS_NODE){
		index = 0;			
		ID = (int)vty->index;
	}else if(vty->node == HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if(vty->node == LOCAL_HANSI_WQOS_NODE){
		index = vty->index; 		
		ID = (int)vty->index_sub;
		localid = vty->local;
	    slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_INTERFACE,INTERFACE); 
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_FLOW_ABLE_V2);

	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_QOS_OBJPATH,\
						WID_DBUS_QOS_INTERFACE,WID_DBUS_QOS_METHOD_SET_QOS_FLOW_ABLE_V2);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&ID,								
						DBUS_TYPE_BYTE,&qos_stream_id,	
						DBUS_TYPE_UINT32,&able_type,
					//	DBUS_TYPE_UINT32,&value,
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

		if(ret == 0)
			vty_out(vty,"set qos profile %s %s successfully\n",argv[0],argv[1]);
		else if(ret == WID_QOS_NOT_EXIST)
			vty_out(vty,"<error> qos id does not exist\n");			
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;	
}
void dcli_wqos_init(void) {
	install_element(VIEW_NODE,&wid_show_qos_extension_info_cmd);										/*a4*/
	install_element(VIEW_NODE,&wid_show_qos_profile_list_cmd);
	install_element(VIEW_NODE,&wid_show_qos_profile_cmd);
#if 0
	install_node(&wqos_node,NULL);
	install_default(WQOS_NODE);
	/*------------------------------VIEW_NODE------------------------------*/

	install_element(VIEW_NODE,&wid_show_qos_profile_list_cmd);											/*a1*/
	install_element(VIEW_NODE,&wid_show_qos_profile_cmd);												/*a2*/
	install_element(VIEW_NODE,&wid_show_qos_radio_cmd);													/*a3*/
	install_element(VIEW_NODE,&wid_show_qos_extension_info_cmd);										/*a4*/
	
	/*------------------------------ENABLE_NODE------------------------------*/
	install_element(ENABLE_NODE,&wid_show_qos_profile_list_cmd);										/*a1*/
	install_element(ENABLE_NODE,&wid_show_qos_profile_cmd);												/*a2*/
	install_element(ENABLE_NODE,&wid_show_qos_radio_cmd);												/*a3*/
	install_element(ENABLE_NODE,&wid_show_qos_extension_info_cmd);										/*a4*/
	
	/*------------------------------CONFIG_NODE------------------------------*/
	install_element(CONFIG_NODE,&wid_show_qos_profile_list_cmd);										/*a1*/
	install_element(CONFIG_NODE,&wid_show_qos_profile_cmd);												/*a2*/
	install_element(CONFIG_NODE,&wid_show_qos_radio_cmd);												/*a3*/
	install_element(CONFIG_NODE,&wid_show_qos_extension_info_cmd);										/*a4*/
	
	install_element(CONFIG_NODE,&wid_config_qos_profile_cmd);	
	install_element(CONFIG_NODE,&wid_create_qos_profile_cmd);		
	install_element(CONFIG_NODE,&wid_delete_qos_profile_cmd);	
	install_element(CONFIG_NODE,&wid_delete_radio_with_qos_profile_cmd);    //fengwenchao add 20110427
	
	/*------------------------------WQOS_NODE------------------------------*/
	install_element(WQOS_NODE,&wid_config_qos_info_cmd);	
	install_element(WQOS_NODE,&wid_config_client_qos_info_cmd);
	install_element(WQOS_NODE,&wid_config_set_qos_wmm_cmd);
	install_element(WQOS_NODE,&wid_config_set_qos_wmm_map_dot1p_cmd);
	install_element(WQOS_NODE,&wid_config_set_qos_dot1p_map_wmm_cmd);
	/*set wireless qos info for mib*/
	install_element(WQOS_NODE,&wid_config_set_qos_total_bandwidth_cmd);
	install_element(WQOS_NODE,&wid_show_qos_extension_info_cmd);
	install_element(WQOS_NODE,&wid_config_set_qos_flow_parameter_cmd);
	install_element(WQOS_NODE,&wid_config_set_qos_parameter_cmd);
	install_element(WQOS_NODE,&wid_config_set_qos_policy_used_cmd);
	install_element(WQOS_NODE,&wid_config_set_qos_policy_name_cmd);
	install_element(WQOS_NODE,&wid_config_set_qos_manage_arithmetic_name_cmd);

	install_element(WQOS_NODE,&wid_config_set_qos_flow_parameter_cmd_v2);
	install_element(WQOS_NODE,&wid_config_set_qos_flow_able_cmd);
	install_element(WQOS_NODE,&wid_config_set_qos_flow_able_cmd_v2);
	install_element(WQOS_NODE,&wid_show_qos_extension_info_cmd);
#endif	
/********************************************************************************/
	install_node(&hansi_wqos_node,NULL,"HANSI_WQOS_NODE");
	install_default(HANSI_WQOS_NODE);

	install_node(&local_hansi_wqos_node,NULL,"LOCAL_HANSI_WQOS_NODE");
	install_default(LOCAL_HANSI_WQOS_NODE);
	
	/*------------------------------HANSI_NODE------------------------------*/
	install_element(HANSI_NODE,&wid_show_qos_profile_list_cmd);											/*a1*/
	install_element(HANSI_NODE,&wid_show_qos_profile_cmd);												/*a2*/
	install_element(HANSI_NODE,&wid_show_qos_extension_info_cmd);										/*a4*/
	
	install_element(HANSI_NODE,&wid_config_qos_profile_cmd);		
	install_element(HANSI_NODE,&wid_create_qos_profile_cmd);		
	install_element(HANSI_NODE,&wid_delete_qos_profile_cmd);
	install_element(HANSI_NODE,&wid_delete_radio_with_qos_profile_cmd);    //fengwenchao add 20110427

	install_element(LOCAL_HANSI_NODE,&wid_show_qos_profile_list_cmd);											/*a1*/
	install_element(LOCAL_HANSI_NODE,&wid_show_qos_profile_cmd);												/*a2*/
	install_element(LOCAL_HANSI_NODE,&wid_show_qos_extension_info_cmd);										/*a4*/
	
	install_element(LOCAL_HANSI_NODE,&wid_config_qos_profile_cmd);		
	install_element(LOCAL_HANSI_NODE,&wid_create_qos_profile_cmd);		
	install_element(LOCAL_HANSI_NODE,&wid_delete_qos_profile_cmd);
	install_element(LOCAL_HANSI_NODE,&wid_delete_radio_with_qos_profile_cmd);    //fengwenchao add 20110427
	
	/*------------------------------HANSI_WQOS_NODE------------------------------*/
	install_element(HANSI_WQOS_NODE,&wid_config_qos_info_cmd);	
	install_element(HANSI_WQOS_NODE,&wid_config_client_qos_info_cmd);
	install_element(HANSI_WQOS_NODE,&wid_config_set_qos_wmm_cmd);
	install_element(HANSI_WQOS_NODE,&wid_config_set_qos_wmm_map_dot1p_cmd);
	install_element(HANSI_WQOS_NODE,&wid_config_set_qos_dot1p_map_wmm_cmd);
	install_element(HANSI_WQOS_NODE,&wid_show_qos_radio_cmd);											
	/*set wireless qos info for mib*/
	install_element(HANSI_WQOS_NODE,&wid_config_set_qos_total_bandwidth_cmd);
	install_element(HANSI_WQOS_NODE,&wid_show_qos_extension_info_cmd);
	install_element(HANSI_WQOS_NODE,&wid_config_set_qos_flow_parameter_cmd);
	install_element(HANSI_WQOS_NODE,&wid_config_set_qos_parameter_cmd);
	install_element(HANSI_WQOS_NODE,&wid_config_set_qos_policy_used_cmd);
	install_element(HANSI_WQOS_NODE,&wid_config_set_qos_policy_name_cmd);
	install_element(HANSI_WQOS_NODE,&wid_config_set_qos_manage_arithmetic_name_cmd);

	install_element(HANSI_WQOS_NODE,&wid_config_set_qos_flow_parameter_cmd_v2);
	install_element(HANSI_WQOS_NODE,&wid_config_set_qos_flow_able_cmd);
	install_element(HANSI_WQOS_NODE,&wid_config_set_qos_flow_able_cmd_v2);
	install_element(HANSI_WQOS_NODE,&wid_show_qos_extension_info_cmd);

	install_element(LOCAL_HANSI_WQOS_NODE,&wid_config_qos_info_cmd);	
	install_element(LOCAL_HANSI_WQOS_NODE,&wid_config_client_qos_info_cmd);
	install_element(LOCAL_HANSI_WQOS_NODE,&wid_config_set_qos_wmm_cmd);
	install_element(LOCAL_HANSI_WQOS_NODE,&wid_config_set_qos_wmm_map_dot1p_cmd);
	install_element(LOCAL_HANSI_WQOS_NODE,&wid_config_set_qos_dot1p_map_wmm_cmd);
	install_element(LOCAL_HANSI_WQOS_NODE,&wid_show_qos_radio_cmd);											
	/*set wireless qos info for mib*/
	install_element(LOCAL_HANSI_WQOS_NODE,&wid_config_set_qos_total_bandwidth_cmd);
	install_element(LOCAL_HANSI_WQOS_NODE,&wid_show_qos_extension_info_cmd);
	install_element(LOCAL_HANSI_WQOS_NODE,&wid_config_set_qos_flow_parameter_cmd);
	install_element(LOCAL_HANSI_WQOS_NODE,&wid_config_set_qos_parameter_cmd);
	install_element(LOCAL_HANSI_WQOS_NODE,&wid_config_set_qos_policy_used_cmd);
	install_element(LOCAL_HANSI_WQOS_NODE,&wid_config_set_qos_policy_name_cmd);
	install_element(LOCAL_HANSI_WQOS_NODE,&wid_config_set_qos_manage_arithmetic_name_cmd);

	install_element(LOCAL_HANSI_WQOS_NODE,&wid_config_set_qos_flow_parameter_cmd_v2);
	install_element(LOCAL_HANSI_WQOS_NODE,&wid_config_set_qos_flow_able_cmd);
	install_element(LOCAL_HANSI_WQOS_NODE,&wid_config_set_qos_flow_able_cmd_v2);
	install_element(LOCAL_HANSI_WQOS_NODE,&wid_show_qos_extension_info_cmd);

	return;
}



#endif

