#ifdef _D_WCPSS_
#include <string.h>
#include <zebra.h>
#include "vtysh/vtysh.h"
#include <dbus/dbus.h>

#include "command.h"

#include "../dcli_main.h"
#include "../dcli_dhcp.h"
#include "dcli_wlan.h"
#include "wid_wlan.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "dbus/wcpss/dcli_wid_wlan.h"
#include "bsd/bsdpub.h"

struct cmd_node wlan_node =
{
	WLAN_NODE,
	"%s(config-wlan %d)# ",
	1
};

struct cmd_node hansi_wlan_node =
{
	HANSI_WLAN_NODE,
	"%s(hansi-wlan %d-%d-%d)# ",
	1
};
/*fengwenchao add 20110507*/
struct cmd_node local_hansi_wlan_node =
{
	LOCAL_HANSI_WLAN_NODE,
	"%s(local-hansi-wlan %d-%d-%d)# ",
	1
};
/*fengwenchao add end*/
struct cmd_node wlan_node1 =
{
	WLAN_NODE1,
	" ",
	1
};

void str2lower(char **str) {  
	int i,len;
	char *ptr;

	len = strlen(*str);
	ptr = *str;

	for(i=0; i<len ; i++) {
		if((ptr[i]<='Z')&&(ptr[i]>='A'))  
			ptr[i] = ptr[i]-'A'+'a';  
	}
	
	return;
}

int strcheck(char **str) {  
	int i,len;
	char *ptr;

	len = strlen(*str);
	ptr = *str;

	for(i=0; i<len ; i++) {
		if((ptr[i]<32)||(ptr[i]>126))  
			return 0;  
	}
	
	return 1;
}

int parse_int(char* str,unsigned int* ID){
	char *endptr = NULL;
	char c;
	c = str[0];
	if (c>='0'&&c<='9'){
		*ID= strtoul(str,&endptr,10);
		if(endptr[0] == '\0')
			return WID_DBUS_SUCCESS;
		else
			return WID_UNKNOWN_ID;
	}
	else
		return WID_UNKNOWN_ID;
}

/*zhaoruijia,20100908,start*/

int parse_char_ID(char* str,unsigned char* ID){
	 /* before modify*/
	char *endptr = NULL;
	char c;
	unsigned long int t_ID = 0;
	c = str[0];
	if (c>='0'&&c<='9'){
		t_ID=  strtoul(str,&endptr,10);
		if(t_ID < 256){
          *ID = (unsigned char)t_ID;
          if((c=='0')&&(str[1]!='\0'))
		  	return WID_UNKNOWN_ID;
		  else if(endptr[0] == '\0')
			return WID_DBUS_SUCCESS;
		  else
			return WID_UNKNOWN_ID;
		}
		else{
            return WID_ILLEGAL_INPUT;
		}
		
	}
	else
		return WID_UNKNOWN_ID;
	
}
/*zhaoruijia,20100908,,end*/
/*sz20080820*/
void CheckWIDIfPolicy(char *whichinterface, unsigned char wlan_if_policy){
	
	switch(wlan_if_policy){

		case 0 :
			strcpy(whichinterface, "NO_IF");
			break;
			
		case 1 :
			strcpy(whichinterface, "WLAN_IF");
			break;
		
		case 2 :
			strcpy(whichinterface, "BSS_IF");
			break;
		}
}


int Check_Time_Format(char* str){
	char *endptr = NULL;
	int endptr1 = 0;
	char c;
	int h,m,s,i;
	int time;
	c = str[0];
	if (c>='0'&&c<='9'){
		h= strtoul(str,&endptr,10);
		if(h < 0||h > 23)
			return -1;
		if(endptr[0] == '\0'||endptr[0] != ':')
			return -1;
		else{
			endptr1 = &endptr[1];
			m= strtoul(&endptr[1],&endptr,10);				
			if(m < 0||m > 59)
				return -1;				
		}
		
		if(endptr[0] == '\0'||endptr[0] != ':')
			return -1;
		else{
			endptr1 = &endptr[1];
			s = strtoul(&endptr[1],&endptr,10);				
			if(s < 0||s > 59)
				return -1;				
		}
		if(endptr[0] == '\0'){
			time = h*3600 + m*60 + s;
			return time;
		}
		else
			return -1;
	}
	else
		return -1;

}


int ssid_illegal_character_check(char *str, int len)//xiaodawei add for checking ssid illegal character, 20110509
{
	int m = 0;
	char *tmp = NULL;
	tmp = str;
	if(str == NULL)
	{
		return -1;
	}
	if(strlen(str)!=len)
	{
		return -1;	//-1 means ssid is null or the parameter len do not match length of ssid
	}
	for(m=0; m<len; m++)
	{
		if(tmp[m]>=32&&tmp[m]<=126&&tmp[m]!='\\'&&tmp[m]!='"'&&tmp[m]!='&'&&tmp[m]!='`'&&tmp[m]!='('&&tmp[m]!=')'&&tmp[m]!='*') /*wcl modify*/
			continue;
		else
			break;
	}
	if(m==len)
	{
		return 0;
	}
	else
	{
		return -2;	//-2 means illegal character
	}
}

DEFUN(show_wlan_list_cmd_func,
	  show_wlan_list_cmd,
	  "show wlan (list|all) [remote] [local] [PARAMETER]",
	  SHOW_STR
	  "Display wireless lan information\n"
	  "List wireless lan summary\n"
	  "All wireless lan summary\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{
/*	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	*/
	char  dis[] = "disable";
	char  en[] = "enable";
	char whichinterface[WLAN_IF_NAME_LEN];
 	char  y[]= "yes";
	char  n[]= "no";/*sz20080820*/
	//WID_WLAN *WLAN[WLAN_NUM];	
	int ret,i=0;
	unsigned char num;
	unsigned int enable_num = 0;	
	
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	int localid = 1;   //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	
	int index = 0;
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;		
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_NODE)
	{
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/
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
	ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath(index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WLANLIST);
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_WLANLIST);*/
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
	if(ret == 0 ){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		
		for (i = 0; i < num; i++) {
			DBusMessageIter iter_struct;
			
			WLAN[i] = (WID_WLAN*)malloc(sizeof(WID_WLAN));
			dbus_message_iter_recurse(&iter_array,&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(WLAN[i]->WlanName));
		
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(WLAN[i]->WlanID));
		
			dbus_message_iter_next(&iter_struct);

			dbus_message_iter_get_basic(&iter_struct,&(WLAN[i]->ESSID));
		
			dbus_message_iter_next(&iter_struct);
		
			dbus_message_iter_get_basic(&iter_struct,&(WLAN[i]->Status));
					
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(WLAN[i]->SecurityID));
			
			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(WLAN[i]->HideESSid));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(WLAN[i]->wlan_if_policy));

			dbus_message_iter_next(&iter_struct);
			
			dbus_message_iter_get_basic(&iter_struct,&(WLAN[i]->WDSStat));
			if(WLAN[i]->Status == 0)
			{
				enable_num++;
			}
			dbus_message_iter_next(&iter_array);
		}
	}
	 

	dbus_message_unref(reply);
#endif	
	DCLI_WLAN_API_GROUP * WLANINFO = NULL;
	if(vty->node != VIEW_NODE){
		WLANINFO = (DCLI_WLAN_API_GROUP *) dcli_wlan_show_api_group(
			index,
			0,
			localid,    //fengwenchao add 20110507
			&ret,
			0,        
			dcli_dbus_connection,
			WID_DBUS_CONF_METHOD_WLANLIST
			);
	    vty_out(vty,"wlan list summary\n");
		vty_out(vty,"wlan enable num:	%d\n",(ret != 0)?0:WLANINFO->enable_num);
	    vty_out(vty,"==============================================================================\n");
		{
			int i = 0;
			int mesh_cout = 0;
			if (ret == 0)
			{
				for (i = 0; i < WLANINFO->wlan_num; i++)
				{
					if (WLANINFO->WLAN[i]->wds_mesh == 1)
						mesh_cout ++;
				}
				if ((mesh_cout >= WLANINFO->wlan_num / 2 && WLANINFO->wlan_num != 1)
					|| (WLANINFO->wlan_num == 1 && WLANINFO->WLAN[0]->wds_mesh == 1))
				{
					vty_out(vty,"%-6s %-12s %-9s %-10s %-8s %-32s\n","WlanID","WlanName","WlanState","SecurityID","MESH","ESSID");
				}
				else 
				{
					vty_out(vty,"%-6s %-12s %-9s %-10s %-8s %-32s\n","WlanID","WlanName","WlanState","SecurityID","WDS","ESSID");
				}
			}
		}
		/*vty_out(vty,"%-6s %-15s %-9s %-10s %-16s %-9s %-8s\n","WlanID","WlanName","WlanState","SecurityID","ESSID","HideESSID","IFPolicy");*/
		//vty_out(vty,"%-6s %-12s %-9s %-10s %-8s %-32s\n","WlanID","WlanName","WlanState","SecurityID","WDS","ESSID");
		if(ret == -1){
			cli_syslog_info("<error> failed get reply.\n");
		}
		else if(ret == 0 ){
			for (i = 0; i < WLANINFO->wlan_num; i++) {
			//	memset(whichinterface,0,WLAN_IF_NAME_LEN);
			//	CheckWIDIfPolicy(whichinterface,WLAN[i]->wlan_if_policy);
				/*vty_out(vty,"%-6d %-15s %-9s %-10d %-16s %-9s %-8s\n",WLAN[i]->WlanID,WLAN[i]->WlanName,((WLAN[i]->Status == 1)?dis:en),WLAN[i]->SecurityID,WLAN[i]->ESSID,((WLAN[i]->HideESSid==1)?y:n),whichinterface);//sz20080820*/
				/*vty_out(vty,"%-6d %-15s %-9s %-10d %-32s\n",WLAN[i]->WlanID,WLAN[i]->WlanName,((WLAN[i]->Status == 1)?dis:en),WLAN[i]->SecurityID,WLAN[i]->ESSID);//sz20080820*/
				if(WLANINFO->WLAN[i]->SecurityID == 0)	
				{
					vty_out(vty,"%-6d %-12s %-9s NONE       %-8s %-32s\n",WLANINFO->WLAN[i]->WlanID,WLANINFO->WLAN[i]->WlanName,((WLANINFO->WLAN[i]->Status == 1)?dis:en),(WLANINFO->WLAN[i]->WDSStat==1)?en:dis,WLANINFO->WLAN[i]->ESSID);/*sz20080820*/
				}
				else
				{
					vty_out(vty,"%-6d %-12s %-9s %-10d %-8s %-32s\n",WLANINFO->WLAN[i]->WlanID,WLANINFO->WLAN[i]->WlanName,((WLANINFO->WLAN[i]->Status == 1)?dis:en),WLANINFO->WLAN[i]->SecurityID,(WLANINFO->WLAN[i]->WDSStat==1)?en:dis,WLANINFO->WLAN[i]->ESSID);/*sz20080820*/
				
				}
			//	free(WLANINFO->WLAN[i]);
			}
			dcli_wlan_free_fun(WID_DBUS_CONF_METHOD_WLANLIST,WLANINFO);
		}
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"wlan list summary\n");
		vty_out(vty,"wlan enable num:	%d\n",(ret != 0)?0:WLANINFO->enable_num);
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
				WLANINFO = (DCLI_WLAN_API_GROUP *) dcli_wlan_show_api_group(
					profile,
					0,
					localid,    //fengwenchao add 20110507
					&ret,
					0,        
					dcli_dbus_connection,
					WID_DBUS_CONF_METHOD_WLANLIST
					);
			    vty_out(vty,"wlan list summary:   hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"wlan enable num:	%d\n",(ret != 0)?0:WLANINFO->enable_num);
				vty_out(vty,"====================================================================\n");
				{
					int i = 0;
					int mesh_cout = 0;
					if (ret == 0)
					{
						for (i = 0; i < WLANINFO->wlan_num; i++)
						{
							if (WLANINFO->WLAN[i]->wds_mesh == 1)
								mesh_cout ++;
						}
						if ((mesh_cout >= WLANINFO->wlan_num / 2 && WLANINFO->wlan_num != 1)
							|| (WLANINFO->wlan_num == 1 && WLANINFO->WLAN[0]->wds_mesh == 1))
						{
							vty_out(vty,"%-6s %-12s %-9s %-10s %-8s %-32s\n","WlanID","WlanName","WlanState","SecurityID","MESH","ESSID");
						}
						else 
						{
							vty_out(vty,"%-6s %-12s %-9s %-10s %-8s %-32s\n","WlanID","WlanName","WlanState","SecurityID","WDS","ESSID");
						}
					}
				}
				//vty_out(vty,"%-6s %-12s %-9s %-10s %-8s %-32s\n","WlanID","WlanName","WlanState","SecurityID","WDS","ESSID");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0 ){
					for (i = 0; i < WLANINFO->wlan_num; i++) {
						if(WLANINFO->WLAN[i]->SecurityID == 0)	
						{
							vty_out(vty,"%-6d %-12s %-9s NONE       %-8s %-32s\n",WLANINFO->WLAN[i]->WlanID,WLANINFO->WLAN[i]->WlanName,((WLANINFO->WLAN[i]->Status == 1)?dis:en),(WLANINFO->WLAN[i]->WDSStat==1)?en:dis,WLANINFO->WLAN[i]->ESSID);/*sz20080820*/
						}
						else
						{
							vty_out(vty,"%-6d %-12s %-9s %-10d %-8s %-32s\n",WLANINFO->WLAN[i]->WlanID,WLANINFO->WLAN[i]->WlanName,((WLANINFO->WLAN[i]->Status == 1)?dis:en),WLANINFO->WLAN[i]->SecurityID,(WLANINFO->WLAN[i]->WDSStat==1)?en:dis,WLANINFO->WLAN[i]->ESSID);/*sz20080820*/
						}
					}
					dcli_wlan_free_fun(WID_DBUS_CONF_METHOD_WLANLIST,WLANINFO);
				}
				vty_out(vty,"====================================================================\n");
				vty_out(vty,"wlan list summary\n");
				vty_out(vty,"wlan enable num:	%d\n",(ret != 0)?0:WLANINFO->enable_num);
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
			WLANINFO = (DCLI_WLAN_API_GROUP *) dcli_wlan_show_api_group(
				profile,
				0,
				localid,    //fengwenchao add 20110507
				&ret,
				0,        
				dcli_dbus_connection,
				WID_DBUS_CONF_METHOD_WLANLIST
				);
		    vty_out(vty,"wlan list summary:   local-hansi %d-%d\n",slot_id,profile);
			vty_out(vty,"wlan enable num:	%d\n",(ret != 0)?0:WLANINFO->enable_num);
			vty_out(vty,"====================================================================\n");
			{
				int i = 0;
				int mesh_cout = 0;
				if (ret == 0)
				{
					for (i = 0; i < WLANINFO->wlan_num; i++)
					{
						if (WLANINFO->WLAN[i]->wds_mesh == 1)
							mesh_cout ++;
					}
					if ((mesh_cout >= WLANINFO->wlan_num / 2 && WLANINFO->wlan_num != 1)
						|| (WLANINFO->wlan_num == 1 && WLANINFO->WLAN[0]->wds_mesh == 1))
					{
						vty_out(vty,"%-6s %-12s %-9s %-10s %-8s %-32s\n","WlanID","WlanName","WlanState","SecurityID","MESH","ESSID");
					}
					else 
					{
						vty_out(vty,"%-6s %-12s %-9s %-10s %-8s %-32s\n","WlanID","WlanName","WlanState","SecurityID","WDS","ESSID");
					}
				}
			}
			//vty_out(vty,"%-6s %-12s %-9s %-10s %-8s %-32s\n","WlanID","WlanName","WlanState","SecurityID","WDS","ESSID");
			if(ret == -1){
				cli_syslog_info("<error> failed get reply.\n");
			}
			else if(ret == 0 ){
				for (i = 0; i < WLANINFO->wlan_num; i++) {
					if(WLANINFO->WLAN[i]->SecurityID == 0)	
					{
						vty_out(vty,"%-6d %-12s %-9s NONE       %-8s %-32s\n",WLANINFO->WLAN[i]->WlanID,WLANINFO->WLAN[i]->WlanName,((WLANINFO->WLAN[i]->Status == 1)?dis:en),(WLANINFO->WLAN[i]->WDSStat==1)?en:dis,WLANINFO->WLAN[i]->ESSID);/*sz20080820*/
					}
					else
					{
						vty_out(vty,"%-6d %-12s %-9s %-10d %-8s %-32s\n",WLANINFO->WLAN[i]->WlanID,WLANINFO->WLAN[i]->WlanName,((WLANINFO->WLAN[i]->Status == 1)?dis:en),WLANINFO->WLAN[i]->SecurityID,(WLANINFO->WLAN[i]->WDSStat==1)?en:dis,WLANINFO->WLAN[i]->ESSID);/*sz20080820*/
					}
				}
				dcli_wlan_free_fun(WID_DBUS_CONF_METHOD_WLANLIST,WLANINFO);
			}
			vty_out(vty,"====================================================================\n");
			vty_out(vty,"wlan list summary\n");
			vty_out(vty,"wlan enable num:	%d\n",(ret != 0)?0:WLANINFO->enable_num);
			vty_out(vty,"==============================================================================\n");
			if(argc == 3){
				return CMD_SUCCESS;
			}
			}
		}
	}
	return CMD_SUCCESS;
	
}

DEFUN(config_wlan_cmd_func,
	  config_wlan_cmd,
	  "config wlan WLANID",
	  CONFIG_STR
	  "Wireless LAN information\n"
	  "Wlan id that you want to config\n"
	 )
{	int ret;
	unsigned char wlan_id,status,wlanid;
	
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	/*wlan_id = atoi(argv[0]);*/
	DBusMessage *query, *reply;
	DBusError err;
	ret = parse_char_ID((char*)argv[0], &wlan_id);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_WARNING;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_WARNING;
	}
	if(vty->node == CONFIG_NODE){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
		localid = vty->local;     //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_NODE)
	{
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WLAN);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_WLAN);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&wlan_id,
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
	
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)) {
		if(ret == 0){
			if(vty->node == CONFIG_NODE){
				vty->node = WLAN_NODE;
				vty->index = (void *)wlan_id;
			}else if(vty->node == HANSI_NODE){	
				vty->node = HANSI_WLAN_NODE;
				vty->index_sub = (void *)wlan_id;
			}else if(vty->node == LOCAL_HANSI_NODE){	
				vty->node = LOCAL_HANSI_WLAN_NODE;
				vty->index_sub = (void *)wlan_id;
			}
		}else if (ret == WLAN_ID_NOT_EXIST){
			vty_out(vty,"<error> wlan id does not exist\n");
			dbus_message_unref(reply);
			return CMD_WARNING;
		}
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else{
			vty_out(vty,"<error>  %d\n",ret);
			dbus_message_unref(reply);
			return CMD_WARNING;
		}
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

//fengwenchao add 20101224    merry christmas
DEFUN(show_wlan_of_all_cmd_func,
	  show_wlan_of_all_cmd,
	  "show wlan of all",
	  CONFIG_STR
	  "Wireless LAN information\n"
	  "Wlan id that you want to config\n"
	 )
{	
	int ret = 0;
	int wlan_num = 0;

	char SecurityType[20];
	char EncryptionType[20];
	char  y[]="yes";
	char  n[]="no";
	char  dis[] = "disable";
	char  en[] = "enable";	
	char  _n[]="number";
	char  _f[]="flow";
	int i,j=0;
	
	DBusConnection *dbus_connection = dcli_dbus_connection;	

    struct WLAN_INFO *wlan_info = NULL;	
    struct WLAN_INFO *wlan_info_show = NULL;		
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507

	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
		localid = vty->local;     //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}	
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_NODE)
	{
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	ReInitDbusConnection(&dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	wlan_info = show_wlan_of_all(dbus_connection,index,localid,&ret,&wlan_num);

	//vty_out(vty,"wlan_num = %d \n",wlan_num);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if((ret == 0 )&&(wlan_info != NULL))
		{
			for(i=0;i<wlan_num;i++)
			{
				if(wlan_info_show == NULL){
					wlan_info_show = wlan_info->wlan_info_list;

					}
				else 
					{wlan_info_show = wlan_info_show->next;

					}
				if(wlan_info_show == NULL){

					break;

					}
				memset(SecurityType, 0, 20);
				memset(EncryptionType, 0, 20);
				CheckSecurityType(SecurityType, wlan_info_show->wlanBindSecType);
				CheckEncryptionType(EncryptionType, wlan_info_show->wlanBindEncryType);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"WLAN ID : %d\n",wlan_info_show->Wlanid);
				vty_out(vty,"WLAN STATUS : %s\n",((wlan_info_show->wlanServiceEnable) == 1)?dis:en);
				vty_out(vty,"WLAN MAX STA NUM : %d\n",wlan_info_show->wlanMaxConnectUsr);
				vty_out(vty,"wlanStaOnlineNum : %d\n",wlan_info_show->wlanStaOnlineNum);
				vty_out(vty,"wlanUsrWirelessResoUseRate : %6.5f%\n",wlan_info_show->wlanUsrWirelessResoUseRate);
				vty_out(vty,"WLAN NUMBER BALANCE PARAMETER : %d\n",wlan_info_show->wlanLoadBalanceUsersDiffThreshhd);
				vty_out(vty,"WLAN FLOW BALANCE PARAMETER : %d\n",wlan_info_show->wlanLoadBalanceTrafficDiffThreshhd);
				vty_out(vty,"WLAN LOAD BALANCE FUNCTION: %s\n",(wlan_info_show->wlanLoadBalanceFunction==1)? en:dis);
				if(wlan_info_show->wlanLoadBalanceFunction==1){
					vty_out(vty,"WLAN BALANCE METHOD: %s\n",(wlan_info_show->wlanLoadBalanceStatus==1)? _n:_f);
				}		

				/*vty_out(vty,"WLAN TRAFFIC LIMIT: %d\n",WLANINFO->WLAN[0]->wlan_traffic_limit);
				vty_out(vty,"WLAN SEND TRAFFIC LIMIT: %d\n",WLANINFO->WLAN[0]->wlan_send_traffic_limit);
				vty_out(vty,"WLAN STA AVERAGE TRAFFIC LIMIT: %d\n",WLANINFO->WLAN[0]->wlan_station_average_traffic_limit);
				vty_out(vty,"WLAN STA AVERAGE SEND TRAFFIC LIMIT: %d\n",WLANINFO->WLAN[0]->wlan_station_average_send_traffic_limit);*/

				
				if (wlan_info_show->wlanBindSecurity == 0)
				{
					vty_out(vty,"SecurityID : NONE\n");
				}
				else
				{
					vty_out(vty,"SecurityID : %d\n",wlan_info_show->wlanBindSecurity);
				}
				vty_out(vty,"SecurityType : %s\n",SecurityType);
				vty_out(vty,"EncryptionType : %s\n",EncryptionType);
				
				vty_out(vty,"HideESSID : %s\n",((wlan_info_show->wlanHideEssid)==1)?y:n);

				/*fengwenchao add 20110401 for dot11WlanDataPktsTable*/
				vty_out(vty,"ethernetRecvCorrectFrames : %d\n",wlan_info_show->ethernetRecvCorrectFrames);
				vty_out(vty,"ethernetRecvCorrectBytes : %llu\n",wlan_info_show->ethernetRecvCorrectBytes);
				vty_out(vty,"ethernetSendCorrectBytes : %llu\n",wlan_info_show->ethernetSendCorrectBytes);
				/*fengwenchao add end*/

				struct ifi *ifi_show = NULL;

				for(j=0;j<wlan_info_show->bifnum;j++)
				{
					if(ifi_show == NULL)
						ifi_show = wlan_info_show->ifi_head;
					else 
					ifi_show = ifi_show->ifi_next;

					if(ifi_show == NULL)
						break;

					if(ifi_show->ifi_name[0] != '/0')
						vty_out(vty,"apply interface: %s \n",ifi_show->ifi_name);
					else
						vty_out(vty,"apply interface: %s \n","NONE");

				}

				vty_out(vty,"==============================================================================\n");
							
			}

	}else if (ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	dcli_wlan_free_info_all(wlan_info);	
	return CMD_SUCCESS;
}

//fengwenchao add end

DEFUN(show_wlan_cmd_func,
	  show_wlan_cmd,
	  "show wlan WLANID [remote] [local] [PARAMETER]",
	  CONFIG_STR
	  "Wireless LAN information\n"
	  "Wlan id that you want to config\n"
	  "'remote' or 'local' hansi\n"
	  "'remote' or 'local' hansi\n"
	  "slotid-instid\n"
	 )
{	unsigned int ret;
	unsigned char wlan_id,status,num;
	//WID_WLAN *WLAN;
	char *eth, *nas_id;
	char  dis[] = "disable";
	char  en[] = "enable";
	char  _n[]="number";
	char  _f[]="flow";
	char  whichinterface[WLAN_IF_NAME_LEN];
	char  y[]="yes";
	char  n[]="no";/*sz20080820*/
	char * key2;
	int input_type=-1;
	char asc[]="ASCII";
	char hex2[]="HEX";
	char unk[]="Unknown";
	char SecurityType[20];
	char EncryptionType[20];
	int profile = 0;
	int instRun = 0;
	int flag = 0;
	char  capwap3[] = "CAPWAP802DOT3";
	char  capwap11[] = "CAPWAP802DOT11";
	char  ipip[] = "IPIP";
	char nas_id_str[NAS_IDENTIFIER_NAME + 1] = {0}; //fengwenchao add 20121129 for autelan-3058
  /*  wlan_id = atoi(argv[0]);*/
/*	DBusMessage *query, *reply;
	DBusError err;	
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	*/
	
	ret = parse_char_ID((char*)argv[0], &wlan_id);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_SUCCESS;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	
	DCLI_WLAN_API_GROUP * WLANINFO = NULL;
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if((vty->node == CONFIG_NODE)||(vty->node == ENABLE_NODE)){
		index = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index;
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_NODE)
	{
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/
	
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
	ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath(index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_SHOWWLAN);
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_SHOWWLAN);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&wlan_id,
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
	if(ret == 0 ){

		WLAN = (WID_WLAN*)malloc(sizeof(WID_WLAN));
	
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(WLAN->balance_para));/*xm*/

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(WLAN->flow_balance_para));/*xm*/

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(WLAN->balance_switch));/*xm*/

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(WLAN->balance_method));/*xm*/
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(WLAN->wlan_max_allowed_sta_num));
		
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&(WLAN->WlanName));
		
		dbus_message_iter_next(&iter);		
		dbus_message_iter_get_basic(&iter,&(WLAN->WlanID));
		
		dbus_message_iter_next(&iter);		
		dbus_message_iter_get_basic(&iter,&(WLAN->ESSID));
		
		dbus_message_iter_next(&iter);		
		dbus_message_iter_get_basic(&iter,&(WLAN->Status));

		dbus_message_iter_next(&iter);		
		dbus_message_iter_get_basic(&iter,&(WLAN->SecurityID));
		
		dbus_message_iter_next(&iter);		
		dbus_message_iter_get_basic(&iter,&(WLAN->SecurityType));
		dbus_message_iter_next(&iter);		
		dbus_message_iter_get_basic(&iter,&(WLAN->EncryptionType));
		dbus_message_iter_next(&iter);		
		dbus_message_iter_get_basic(&iter,&(key2));
		dbus_message_iter_next(&iter);		
		dbus_message_iter_get_basic(&iter,&(WLAN->asic_hex));
		dbus_message_iter_next(&iter);		
		dbus_message_iter_get_basic(&iter,&(WLAN->HideESSid));
		
		dbus_message_iter_next(&iter);		
		dbus_message_iter_get_basic(&iter,&(WLAN->wlan_if_policy));

		dbus_message_iter_next(&iter);		
		dbus_message_iter_get_basic(&iter,&(WLAN->Roaming_Policy));

		dbus_message_iter_next(&iter);		
		dbus_message_iter_get_basic(&iter,&(WLAN->WDSStat));
						
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&num);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);
		memset(SecurityType, 0, 20);
		memset(EncryptionType, 0, 20);
		CheckSecurityType(SecurityType, WLAN->SecurityType);
		CheckEncryptionType(EncryptionType, WLAN->EncryptionType);
		memcpy(WLAN->WlanKey,key2,DEFAULT_LEN);
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"WLAN NAME : %s\n",WLAN->WlanName);
		vty_out(vty,"WLAN ID : %d\n",WLAN->WlanID);
		vty_out(vty,"WLAN ESSID : %s\n",WLAN->ESSID);
		vty_out(vty,"WLAN STATUS : %s\n",((WLAN->Status) == 1)?dis:en);
		vty_out(vty,"WLAN MAX STA NUM : %d\n",WLAN->wlan_max_allowed_sta_num);
		vty_out(vty,"WLAN NUMBER BALANCE PARAMETER : %d\n",WLAN->balance_para);
		vty_out(vty,"WLAN FLOW BALANCE PARAMETER : %d\n",WLAN->flow_balance_para);
		vty_out(vty,"WLAN LOAD BALANCE FUNCTION: %s\n",(WLAN->balance_switch==1)? en:dis);
		if(WLAN->balance_switch==1){
			vty_out(vty,"WLAN BALANCE METHOD: %s\n",(WLAN->balance_method==1)? _n:_f);
		}		
		vty_out(vty,"WLAN ROAMING FUNCTION: %s\n",(WLAN->Roaming_Policy==0)? dis:en);

		vty_out(vty,"WLAN WDS FUNCTION: %s\n",(WLAN->WDSStat==1)? en:dis);		
		if (WLAN->SecurityID == 0)
		{
			vty_out(vty,"SecurityID : NONE\n");
		}
		else
		{
			vty_out(vty,"SecurityID : %d\n",WLAN->SecurityID);
		}
		vty_out(vty,"SecurityType : %s\n",SecurityType);
		vty_out(vty,"EncryptionType : %s\n",EncryptionType);
		vty_out(vty,"SecurityKEY(%s): %s\n\n",(WLAN->asic_hex==0)?asc:((WLAN->asic_hex==1)?hex2:unk),WLAN->WlanKey);
		vty_out(vty,"HideESSID : %s\n",((WLAN->HideESSid)==1)?y:n);
		memset(whichinterface,0,WLAN_IF_NAME_LEN);
		CheckWIDIfPolicy(whichinterface,WLAN->wlan_if_policy);
		vty_out(vty,"IFPolicy : %s\n",whichinterface);/*sz20080820*/
		free(WLAN);
		WLAN = NULL;
		int i;
		for (i = 0; i < num; i++) {
			DBusMessageIter iter_struct;
			dbus_message_iter_recurse(&iter_array,&iter_struct);
						
			dbus_message_iter_get_basic(&iter_struct,&eth);
			dbus_message_iter_next(&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&(nas_id));
					
			dbus_message_iter_next(&iter_array);
			
			vty_out(vty,"apply interface: %s (nas_id :%s)\n",eth,nas_id);
		}
		vty_out(vty,"==============================================================================\n");
	}else if (ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
#endif
	if(vty->node != VIEW_NODE){
		WLANINFO = NULL;
		WLANINFO = (DCLI_WLAN_API_GROUP *) dcli_wlan_show_api_group(
			index,
			wlan_id,
			localid,    //fengwenchao add 20110507
			&ret,
			0,
			dcli_dbus_connection,
			WID_DBUS_CONF_METHOD_SHOWWLAN
			);
		//printf("###########ret is %d.\n",ret);
		if(ret == -1){
			cli_syslog_info("<error> failed get reply.\n");
		}
		else if(ret == 0 ){
			memset(SecurityType, 0, 20);
			memset(EncryptionType, 0, 20);
			CheckSecurityType(SecurityType, WLANINFO->WLAN[0]->SecurityType);
			CheckEncryptionType(EncryptionType, WLANINFO->WLAN[0]->EncryptionType);
			vty_out(vty,"==============================================================================\n");
			vty_out(vty,"WLAN NAME : %s\n",WLANINFO->WLAN[0]->WlanName);
			vty_out(vty,"WLAN ID : %d\n",WLANINFO->WLAN[0]->WlanID);
			vty_out(vty,"WLAN ESSID : %s\n",WLANINFO->WLAN[0]->ESSID);
			vty_out(vty,"WLAN STATUS : %s\n",((WLANINFO->WLAN[0]->Status) == 1)?dis:en);
			vty_out(vty,"WLAN MAX STA NUM : %d\n",WLANINFO->WLAN[0]->wlan_max_allowed_sta_num);
			vty_out(vty,"WLAN NUMBER BALANCE PARAMETER : %d\n",WLANINFO->WLAN[0]->balance_para);
			vty_out(vty,"WLAN FLOW BALANCE PARAMETER : %d\n",WLANINFO->WLAN[0]->flow_balance_para);
			vty_out(vty,"WLAN LOAD BALANCE FUNCTION: %s\n",(WLANINFO->WLAN[0]->balance_switch==1)? en:dis);
			if(WLANINFO->WLAN[0]->balance_switch==1){
				vty_out(vty,"WLAN BALANCE METHOD: %s\n",(WLANINFO->WLAN[0]->balance_method==1)? _n:_f);
			}		
			vty_out(vty,"WLAN ROAMING FUNCTION: %s\n",(WLANINFO->WLAN[0]->Roaming_Policy==0)? dis:en);
			vty_out(vty,"WLAN TRAFFIC LIMIT: %d\n",WLANINFO->WLAN[0]->wlan_traffic_limit);
			vty_out(vty,"WLAN SEND TRAFFIC LIMIT: %d\n",WLANINFO->WLAN[0]->wlan_send_traffic_limit);
			vty_out(vty,"WLAN STA AVERAGE TRAFFIC LIMIT: %d\n",WLANINFO->WLAN[0]->wlan_station_average_traffic_limit);
			vty_out(vty,"WLAN STA AVERAGE SEND TRAFFIC LIMIT: %d\n",WLANINFO->WLAN[0]->wlan_station_average_send_traffic_limit);
			if (WLANINFO->WLAN[0]->wds_mesh == 1)
				vty_out(vty,"WLAN MESH FUNCTION: %s\n",(WLANINFO->WLAN[0]->WDSStat==1)? en:dis);		
			else if (WLANINFO->WLAN[0]->wds_mesh == 0)
				vty_out(vty,"WLAN WDS FUNCTION: %s\n",(WLANINFO->WLAN[0]->WDSStat==1)? en:dis);
			else
			{
			}
			vty_out(vty,"WLAN FLOW CHECK:%s\n",(WLANINFO->WLAN[0]->flow_check == 1)?en:dis);
			if(WLANINFO->WLAN[0]->flow_check == 1){	
				vty_out(vty,"WLAN NO FLOW TIME:%d\n",WLANINFO->WLAN[0]->no_flow_time);
				vty_out(vty,"WLAN LIMIT MIN FLOW:%d\n",WLANINFO->WLAN[0]->limit_flow);
			}
			vty_out(vty,"WLAN EAP SWITCH: %s\n",(WLANINFO->WLAN[0]->eap_mac_switch==1)? en:dis);
			if(WLANINFO->WLAN[0]->eap_mac_switch==1){
			    vty_out(vty,"WLAN EAP MAC: %s\n",(char*)WLANINFO->WLAN[0]->eap_mac);
			}
			/* Huang leilei add, 2012-11-15 19:00 */
			if (WLANINFO->WLAN[0]->wlan_if_policy == WLAN_INTERFACE)
			{
				vty_out(vty, "WLAN BRIDGE ISOLATION:           %s\n", (WLANINFO->WLAN[0]->isolation_policy == 1) ? en : dis);
				vty_out(vty, "WLAN BRIDGE SAMEPORTSWITCH:      %s\n", (WLANINFO->WLAN[0]->sameportswitch == 1) ? en : dis);
				vty_out(vty, "WLAN BRIDGE UNICAST SOLICIT:     %s\n", (WLANINFO->WLAN[0]->bridge_ucast_solicit_stat == 1) ? en : dis);
				vty_out(vty, "WLAN BRIDGE MULTICAST SOLICIT:   %s\n", (WLANINFO->WLAN[0]->bridge_mcast_solicit_stat == 1) ? en : dis);
			}
			if (WLANINFO->WLAN[0]->SecurityID == 0)
			{
				vty_out(vty,"SecurityID : NONE\n");
			}
			else
			{
				vty_out(vty,"SecurityID : %d\n",WLANINFO->WLAN[0]->SecurityID);
			}
			vty_out(vty,"SecurityType : %s\n",SecurityType);
			vty_out(vty,"EncryptionType : %s\n",EncryptionType);
			vty_out(vty,"SecurityKEY(%s): %s\n\n",(WLANINFO->WLAN[0]->asic_hex==0)?asc:((WLANINFO->WLAN[0]->asic_hex==1)?hex2:unk),WLANINFO->WLAN[0]->WlanKey);
			vty_out(vty,"HideESSID : %s\n",((WLANINFO->WLAN[0]->HideESSid)==1)?y:n);
			vty_out(vty,"SecurityIndex : %d\n",WLANINFO->WLAN[0]->SecurityIndex);
			memset(whichinterface,0,WLAN_IF_NAME_LEN);
			CheckWIDIfPolicy(whichinterface,WLANINFO->WLAN[0]->wlan_if_policy);
			vty_out(vty,"IFPolicy : %s\n",whichinterface);
			unsigned char wlan_tu_policy = 0;
			wlan_tu_policy = WLANINFO->WLAN[0]->WLAN_TUNNEL_POLICY;
		
			vty_out(vty,"tunnel policy: %s\n",(wlan_tu_policy ==8 )?ipip:((wlan_tu_policy ==2 )?capwap3:capwap11));

		//	free(WLANINFO->WLAN[0]);
		//	WLANINFO->WLAN[0] = NULL;
			int i;
			if((WLANINFO != NULL)&&(WLANINFO->WLAN[0] != NULL)&&(WLANINFO->WLAN[0]->Wlan_Ifi != NULL)){
				struct ifi * head = NULL;
				head = WLANINFO->WLAN[0]->Wlan_Ifi;
				while(head != NULL) {
					/*fengwenchao add 20121129 for autelan-3058*/
					memset(nas_id_str, 0, NAS_IDENTIFIER_NAME + 1);
					memcpy(nas_id_str, head->nas_id, NAS_IDENTIFIER_NAME);
					/*fengwenchao add end*/
					vty_out(vty,"apply interface: %s (nas_id :%s)\n",head->ifi_name,nas_id_str);
					head = head->ifi_next;
				}
			}else{
				vty_out(vty,"apply interface: %s (nas_id :%s)\n","NONE","NONE");
			}
			vty_out(vty,"HotSpotID : %d\n",WLANINFO->WLAN[0]->hotspot_id);
			vty_out(vty,"==============================================================================\n");
			dcli_wlan_free_fun(WID_DBUS_CONF_METHOD_SHOWWLAN,WLANINFO);
		}else if (ret == WLAN_ID_NOT_EXIST)
			vty_out(vty,"<error> wlan id does not exist\n");
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else
			vty_out(vty,"<error>  %d\n",ret);
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
			
				WLANINFO = NULL;
				WLANINFO = (DCLI_WLAN_API_GROUP *) dcli_wlan_show_api_group(
					profile,
					wlan_id,
					localid,	//fengwenchao add 20110507
					&ret,
					0,
					dcli_dbus_connection,
					WID_DBUS_CONF_METHOD_SHOWWLAN
					);
				
				vty_out(vty,"==============================================================================\n");
			    vty_out(vty,"hansi %d-%d\n",slot_id,profile);
			    vty_out(vty,"-----------------------------------------------------------------------------\n");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0 ){
					memset(SecurityType, 0, 20);
					memset(EncryptionType, 0, 20);
					CheckSecurityType(SecurityType, WLANINFO->WLAN[0]->SecurityType);
					CheckEncryptionType(EncryptionType, WLANINFO->WLAN[0]->EncryptionType);
					vty_out(vty,"WLAN NAME : %s\n",WLANINFO->WLAN[0]->WlanName);
					vty_out(vty,"WLAN ID : %d\n",WLANINFO->WLAN[0]->WlanID);
					vty_out(vty,"WLAN ESSID : %s\n",WLANINFO->WLAN[0]->ESSID);
					vty_out(vty,"WLAN STATUS : %s\n",((WLANINFO->WLAN[0]->Status) == 1)?dis:en);
					vty_out(vty,"WLAN MAX STA NUM : %d\n",WLANINFO->WLAN[0]->wlan_max_allowed_sta_num);
					vty_out(vty,"WLAN NUMBER BALANCE PARAMETER : %d\n",WLANINFO->WLAN[0]->balance_para);
					vty_out(vty,"WLAN FLOW BALANCE PARAMETER : %d\n",WLANINFO->WLAN[0]->flow_balance_para);
					vty_out(vty,"WLAN LOAD BALANCE FUNCTION: %s\n",(WLANINFO->WLAN[0]->balance_switch==1)? en:dis);
					if(WLANINFO->WLAN[0]->balance_switch==1){
						vty_out(vty,"WLAN BALANCE METHOD: %s\n",(WLANINFO->WLAN[0]->balance_method==1)? _n:_f);
					}		
					vty_out(vty,"WLAN ROAMING FUNCTION: %s\n",(WLANINFO->WLAN[0]->Roaming_Policy==0)? dis:en);
					vty_out(vty,"WLAN TRAFFIC LIMIT: %d\n",WLANINFO->WLAN[0]->wlan_traffic_limit);
					vty_out(vty,"WLAN SEND TRAFFIC LIMIT: %d\n",WLANINFO->WLAN[0]->wlan_send_traffic_limit);
					vty_out(vty,"WLAN STA AVERAGE TRAFFIC LIMIT: %d\n",WLANINFO->WLAN[0]->wlan_station_average_traffic_limit);
					vty_out(vty,"WLAN STA AVERAGE SEND TRAFFIC LIMIT: %d\n",WLANINFO->WLAN[0]->wlan_station_average_send_traffic_limit);

					if (WLANINFO->WLAN[0]->wds_mesh == 1)
						vty_out(vty,"WLAN MESH FUNCTION: %s\n",(WLANINFO->WLAN[0]->WDSStat==1)? en:dis);		
					else if (WLANINFO->WLAN[0]->wds_mesh == 0)
						vty_out(vty,"WLAN WDS FUNCTION: %s\n",(WLANINFO->WLAN[0]->WDSStat==1)? en:dis);
					else
					{
					}
					//vty_out(vty,"WLAN WDS FUNCTION: %s\n",(WLANINFO->WLAN[0]->WDSStat==1)? en:dis); 	
					vty_out(vty,"WLAN FLOW CHECK:%s\n",(WLANINFO->WLAN[0]->flow_check == 1)?en:dis);
					if(WLANINFO->WLAN[0]->flow_check == 1){ 
						vty_out(vty,"WLAN NO FLOW TIME:%d\n",WLANINFO->WLAN[0]->no_flow_time);
						vty_out(vty,"WLAN LIMIT MIN FLOW:%d\n",WLANINFO->WLAN[0]->limit_flow);
					}
					vty_out(vty,"WLAN EAP SWITCH: %s\n",(WLANINFO->WLAN[0]->eap_mac_switch==1)? en:dis);
					if(WLANINFO->WLAN[0]->eap_mac_switch==1){
						vty_out(vty,"WLAN EAP MAC: %s\n",(char*)WLANINFO->WLAN[0]->eap_mac);
					}
					if (WLANINFO->WLAN[0]->wlan_if_policy == WLAN_INTERFACE)
					{
						vty_out(vty, "WLAN BRIDGE ISOLATION:           %s\n", (WLANINFO->WLAN[0]->isolation_policy == 1) ? en : dis);
						vty_out(vty, "WLAN BRIDGE MULTICAST ISOLATION: %s\n", (WLANINFO->WLAN[0]->multicast_isolation_policy == 1) ? en : dis);
						vty_out(vty, "WLAN BRIDGE SAMEPORTSWITCH:      %s\n", (WLANINFO->WLAN[0]->sameportswitch == 1) ? en : dis);
						vty_out(vty, "WLAN BRIDGE UNICAST SOLICIT:     %s\n", (WLANINFO->WLAN[0]->bridge_ucast_solicit_stat == 1) ? en : dis);
						vty_out(vty, "WLAN BRIDGE MULTICAST SOLICIT:   %s\n", (WLANINFO->WLAN[0]->bridge_mcast_solicit_stat == 1) ? en : dis);
					}
					if (WLANINFO->WLAN[0]->SecurityID == 0)
					{
						vty_out(vty,"SecurityID : NONE\n");
					}
					else
					{
						vty_out(vty,"SecurityID : %d\n",WLANINFO->WLAN[0]->SecurityID);
					}
					vty_out(vty,"SecurityType : %s\n",SecurityType);
					vty_out(vty,"EncryptionType : %s\n",EncryptionType);
					vty_out(vty,"SecurityKEY(%s): %s\n\n",(WLANINFO->WLAN[0]->asic_hex==0)?asc:((WLANINFO->WLAN[0]->asic_hex==1)?hex2:unk),WLANINFO->WLAN[0]->WlanKey);
					vty_out(vty,"HideESSID : %s\n",((WLANINFO->WLAN[0]->HideESSid)==1)?y:n);
					vty_out(vty,"SecurityIndex : %d\n",WLANINFO->WLAN[0]->SecurityIndex);
					memset(whichinterface,0,WLAN_IF_NAME_LEN);
					CheckWIDIfPolicy(whichinterface,WLANINFO->WLAN[0]->wlan_if_policy);
					vty_out(vty,"IFPolicy : %s\n",whichinterface);

					if((WLANINFO != NULL)&&(WLANINFO->WLAN[0] != NULL)&&(WLANINFO->WLAN[0]->Wlan_Ifi != NULL)){
						struct ifi * head = NULL;
						head = WLANINFO->WLAN[0]->Wlan_Ifi;
						while(head != NULL) {
							/*fengwenchao add 20121129 for autelan-3058*/
							memset(nas_id_str, 0, NAS_IDENTIFIER_NAME + 1);
							memcpy(nas_id_str, head->nas_id, NAS_IDENTIFIER_NAME);
							/*fengwenchao add end*/							
							vty_out(vty,"apply interface: %s (nas_id :%s)\n",head->ifi_name,head->nas_id);
							head = head->ifi_next;
						}
					}else{
						vty_out(vty,"apply interface: %s (nas_id :%s)\n","NONE","NONE");
					}
					vty_out(vty,"HotSpotID : %d\n",WLANINFO->WLAN[0]->hotspot_id);
					dcli_wlan_free_fun(WID_DBUS_CONF_METHOD_SHOWWLAN,WLANINFO);
				}else if (ret == WLAN_ID_NOT_EXIST)
					vty_out(vty,"<error> wlan id does not exist\n");
				else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
				{
					vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
				}
				else
					vty_out(vty,"<error>  %d\n",ret);
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
				WLANINFO = NULL;
				WLANINFO = (DCLI_WLAN_API_GROUP *) dcli_wlan_show_api_group(
					profile,
					wlan_id,
					localid,	//fengwenchao add 20110507
					&ret,
					0,
					dcli_dbus_connection,
					WID_DBUS_CONF_METHOD_SHOWWLAN
					);
				
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if(ret == -1){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else if(ret == 0 ){
					memset(SecurityType, 0, 20);
					memset(EncryptionType, 0, 20);
					CheckSecurityType(SecurityType, WLANINFO->WLAN[0]->SecurityType);
					CheckEncryptionType(EncryptionType, WLANINFO->WLAN[0]->EncryptionType);
					vty_out(vty,"WLAN NAME : %s\n",WLANINFO->WLAN[0]->WlanName);
					vty_out(vty,"WLAN ID : %d\n",WLANINFO->WLAN[0]->WlanID);
					vty_out(vty,"WLAN ESSID : %s\n",WLANINFO->WLAN[0]->ESSID);
					vty_out(vty,"WLAN STATUS : %s\n",((WLANINFO->WLAN[0]->Status) == 1)?dis:en);
					vty_out(vty,"WLAN MAX STA NUM : %d\n",WLANINFO->WLAN[0]->wlan_max_allowed_sta_num);
					vty_out(vty,"WLAN NUMBER BALANCE PARAMETER : %d\n",WLANINFO->WLAN[0]->balance_para);
					vty_out(vty,"WLAN FLOW BALANCE PARAMETER : %d\n",WLANINFO->WLAN[0]->flow_balance_para);
					vty_out(vty,"WLAN LOAD BALANCE FUNCTION: %s\n",(WLANINFO->WLAN[0]->balance_switch==1)? en:dis);
					if(WLANINFO->WLAN[0]->balance_switch==1){
						vty_out(vty,"WLAN BALANCE METHOD: %s\n",(WLANINFO->WLAN[0]->balance_method==1)? _n:_f);
					}		
					vty_out(vty,"WLAN ROAMING FUNCTION: %s\n",(WLANINFO->WLAN[0]->Roaming_Policy==0)? dis:en);
					vty_out(vty,"WLAN TRAFFIC LIMIT: %d\n",WLANINFO->WLAN[0]->wlan_traffic_limit);
					vty_out(vty,"WLAN SEND TRAFFIC LIMIT: %d\n",WLANINFO->WLAN[0]->wlan_send_traffic_limit);
					vty_out(vty,"WLAN STA AVERAGE TRAFFIC LIMIT: %d\n",WLANINFO->WLAN[0]->wlan_station_average_traffic_limit);
					vty_out(vty,"WLAN STA AVERAGE SEND TRAFFIC LIMIT: %d\n",WLANINFO->WLAN[0]->wlan_station_average_send_traffic_limit);
					if (WLANINFO->WLAN[0]->wds_mesh == 1)
						vty_out(vty,"WLAN MESH FUNCTION: %s\n",(WLANINFO->WLAN[0]->WDSStat==1)? en:dis);		
					else if (WLANINFO->WLAN[0]->wds_mesh == 0)
						vty_out(vty,"WLAN WDS FUNCTION: %s\n",(WLANINFO->WLAN[0]->WDSStat==1)? en:dis);
					else
					{
					}
					//vty_out(vty,"WLAN WDS FUNCTION: %s\n",(WLANINFO->WLAN[0]->WDSStat==1)? en:dis); 	
					vty_out(vty,"WLAN FLOW CHECK:%s\n",(WLANINFO->WLAN[0]->flow_check == 1)?en:dis);
					if(WLANINFO->WLAN[0]->flow_check == 1){ 
						vty_out(vty,"WLAN NO FLOW TIME:%d\n",WLANINFO->WLAN[0]->no_flow_time);
						vty_out(vty,"WLAN LIMIT MIN FLOW:%d\n",WLANINFO->WLAN[0]->limit_flow);
					}
					vty_out(vty,"WLAN EAP SWITCH: %s\n",(WLANINFO->WLAN[0]->eap_mac_switch==1)? en:dis);
					if(WLANINFO->WLAN[0]->eap_mac_switch==1){
						vty_out(vty,"WLAN EAP MAC: %s\n",(char*)WLANINFO->WLAN[0]->eap_mac);
					}
					if (WLANINFO->WLAN[0]->wlan_if_policy == WLAN_INTERFACE)
					{
						vty_out(vty, "WLAN BRIDGE ISOLATION:           %s\n", (WLANINFO->WLAN[0]->isolation_policy == 1) ? en : dis);
						vty_out(vty, "WLAN BRIDGE MULTICAST ISOLATION: %s\n", (WLANINFO->WLAN[0]->multicast_isolation_policy == 1) ? en : dis);
						vty_out(vty, "WLAN BRIDGE SAMEPORTSWITCH:      %s\n", (WLANINFO->WLAN[0]->sameportswitch == 1) ? en : dis);
						vty_out(vty, "WLAN BRIDGE UNICAST SOLICIT:     %s\n", (WLANINFO->WLAN[0]->bridge_ucast_solicit_stat == 1) ? en : dis);
						vty_out(vty, "WLAN BRIDGE MULTICAST SOLICIT:   %s\n", (WLANINFO->WLAN[0]->bridge_mcast_solicit_stat == 1) ? en : dis);
					}
					if (WLANINFO->WLAN[0]->SecurityID == 0)
					{
						vty_out(vty,"SecurityID : NONE\n");
					}
					else
					{
						vty_out(vty,"SecurityID : %d\n",WLANINFO->WLAN[0]->SecurityID);
					}
					vty_out(vty,"SecurityType : %s\n",SecurityType);
					vty_out(vty,"EncryptionType : %s\n",EncryptionType);
					vty_out(vty,"SecurityKEY(%s): %s\n\n",(WLANINFO->WLAN[0]->asic_hex==0)?asc:((WLANINFO->WLAN[0]->asic_hex==1)?hex2:unk),WLANINFO->WLAN[0]->WlanKey);
					vty_out(vty,"HideESSID : %s\n",((WLANINFO->WLAN[0]->HideESSid)==1)?y:n);
					vty_out(vty,"SecurityIndex : %d\n",WLANINFO->WLAN[0]->SecurityIndex);
					memset(whichinterface,0,WLAN_IF_NAME_LEN);
					CheckWIDIfPolicy(whichinterface,WLANINFO->WLAN[0]->wlan_if_policy);
					vty_out(vty,"IFPolicy : %s\n",whichinterface);

					if((WLANINFO != NULL)&&(WLANINFO->WLAN[0] != NULL)&&(WLANINFO->WLAN[0]->Wlan_Ifi != NULL)){
						struct ifi * head = NULL;
						head = WLANINFO->WLAN[0]->Wlan_Ifi;
						while(head != NULL) {
							/*fengwenchao add 20121129 for autelan-3058*/
							memset(nas_id_str, 0, NAS_IDENTIFIER_NAME + 1);
							memcpy(nas_id_str, head->nas_id, NAS_IDENTIFIER_NAME);
							/*fengwenchao add end*/	
							vty_out(vty,"apply interface: %s (nas_id :%s)\n",head->ifi_name,head->nas_id);
							head = head->ifi_next;
						}
					}else{
						vty_out(vty,"apply interface: %s (nas_id :%s)\n","NONE","NONE");
					}
					vty_out(vty,"HotSpotID : %d\n",WLANINFO->WLAN[0]->hotspot_id);
					dcli_wlan_free_fun(WID_DBUS_CONF_METHOD_SHOWWLAN,WLANINFO);
				}else if (ret == WLAN_ID_NOT_EXIST)
					vty_out(vty,"<error> wlan id does not exist\n");
				else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
				{
					vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
				}
				else
					vty_out(vty,"<error>  %d\n",ret);
				vty_out(vty,"==============================================================================\n");
				if(argc == 3){
					return CMD_SUCCESS;
				}
			}
		}
	}
	
	return CMD_SUCCESS;
}

//weichao add 
DEFUN(config_flow_check_cmd_func,
	  config_flow_check_cmd,
	  "flow check (enable|disable)",
	  "flow check.\n"
	  "network flow check\n"
	  "default disable"
	 )
{

	int ret = ASD_DBUS_SUCCESS;
	unsigned short  type = 0;
	unsigned char WlanID = 0;	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	if ((!strcmp(argv[0],"enable")))
		type = 1;			
	else if ((!strcmp(argv[0],"disable")))
		type = 0;	
	else 
	{		
		vty_out(vty,"<error> unknown encryption type.\n");
		return CMD_SUCCESS;
	}	
		int index = 0;
		int localid = 1;  
		int slot_id = HostSlotId;   	
		char BUSNAME[PATH_LEN];
		char OBJPATH[PATH_LEN];
		char INTERFACE[PATH_LEN];
		if(vty->node == WLAN_NODE){
			index = 0;			
			WlanID = (int)vty->index;
		}else if(vty->node == HANSI_WLAN_NODE){
			index = vty->index;			
			WlanID = (int)vty->index_sub;
			localid = vty->local;   
			slot_id = vty->slotindex; 
		}
		else if (vty->node == LOCAL_HANSI_WLAN_NODE)
		{
			index = vty->index;			
			WlanID = (int)vty->index_sub;
			localid = vty->local;
			slot_id = vty->slotindex;
		}
		DBusConnection *dcli_dbus_connection = NULL;
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		
		ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);		
		ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);		
		ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_FLOW_CHECK);
		dbus_error_init(&err);
		
	
		dbus_message_append_args(query,
								 DBUS_TYPE_BYTE,&WlanID,
								 DBUS_TYPE_UINT16,&type,
								 DBUS_TYPE_INVALID);
	
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		if (NULL == reply) {
			
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}
		
		dbus_message_iter_init(reply,&iter);
		
		dbus_message_iter_get_basic(&iter,&ret);
		
			if(ret == WID_DBUS_SUCCESS){
				if(1==type)
					vty_out(vty,"the flow check enable successfully!\n");
				
				else
					vty_out(vty,"the flow check disable successfully!\n");
			}	
			else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
			{
				vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
			}
			else 
			{
				vty_out(vty, "<error> other unknow error happend: %d\n", ret);
			}
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}

//weichao add 
DEFUN(set_no_flow_time_cmd_func,
	  set_no_flow_time_cmd,
	  "set no flow time   <0-86400>",
	  "set the no flow time.\n"
	  "max 86400 second\n"
	  "24*60*60"
	 )
{

	int ret = ASD_DBUS_SUCCESS;
	unsigned char WlanID = 0;	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int  flow_time=900;

	flow_time = atoi(argv[0]);

		if(flow_time<0||flow_time>86400)
			{
				vty_out(vty,"<error> input period value should be 0 to 86400.\n");
				return CMD_SUCCESS;
			}
		int index = 0;
		int localid = 1;  
		int slot_id = HostSlotId;   	
		char BUSNAME[PATH_LEN];
		char OBJPATH[PATH_LEN];
		char INTERFACE[PATH_LEN];
		if(vty->node == WLAN_NODE){
			index = 0;			
			WlanID = (int)vty->index;
		}else if(vty->node == HANSI_WLAN_NODE){
			index = vty->index;			
			WlanID = (int)vty->index_sub;
			localid = vty->local;   
			slot_id = vty->slotindex; 
		}
		else if (vty->node == LOCAL_HANSI_WLAN_NODE)
		{
			index = vty->index;			
			WlanID = (int)vty->index_sub;
			localid = vty->local;
			slot_id = vty->slotindex;
		}
		DBusConnection *dcli_dbus_connection = NULL;
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		
		ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);		
		ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);		
		ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_NO_FLOW_TIME);
		
	
		
	
		dbus_message_append_args(query,
								 DBUS_TYPE_BYTE,&WlanID,
								 DBUS_TYPE_UINT32,&flow_time,
								 DBUS_TYPE_INVALID);
	
		dbus_error_init(&err);
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
		
		dbus_message_unref(query);
		if (NULL == reply) {
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}
		
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		
			if(ret == WID_DBUS_SUCCESS)
				vty_out(vty,"set the no flow time successfully!\n");
			else if(ret == FLOW_CHECK_BE_DISABLE){
				vty_out(vty,"set the no flow time successfully!(the flow check is disable)\n");
			}
			else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
			{
				vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
			}
			else 
			{
				vty_out(vty, "<error> other unknow error happend: %d\n", ret);
			}
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}

//weichao add 
DEFUN(set_limit_min_flow_cmd_func,
	  set_limit_min_flow_cmd,
	  "set limit min flow  <0-10485760>",
	  "set the limit min flow.\n"
	  "max 10MB \n"
	  "1024*1024*10 Bytes"
	 )
{
	int ret = ASD_DBUS_SUCCESS;
	unsigned char WlanID = 0;	
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int min_flow=10240;

	min_flow = atoi(argv[0]);

		if(min_flow<0||min_flow>10485760)
			{
				vty_out(vty,"<error> input period value should be 0 to 10485760.\n");
				return CMD_SUCCESS;
			}
		int index = 0;
		int localid = 1;  
		int slot_id = HostSlotId;   	
		char BUSNAME[PATH_LEN];
		char OBJPATH[PATH_LEN];
		char INTERFACE[PATH_LEN];
		if(vty->node == WLAN_NODE){
			index = 0;			
			WlanID = (int)vty->index;
		}else if(vty->node == HANSI_WLAN_NODE){
			index = vty->index;			
			WlanID = (int)vty->index_sub;
			localid = vty->local;   
			slot_id = vty->slotindex; 
		}
		else if (vty->node == LOCAL_HANSI_WLAN_NODE)
		{
			index = vty->index;			
			WlanID = (int)vty->index_sub;
			localid = vty->local;
			slot_id = vty->slotindex;
		}
		DBusConnection *dcli_dbus_connection = NULL;
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		
		ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);		
		ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);		
		ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_LIMIT_MIN_FLOW);
		
	
		
		dbus_error_init(&err);
	
		dbus_message_append_args(query,
								 DBUS_TYPE_BYTE,&WlanID,
								 DBUS_TYPE_UINT32,&min_flow,
								 DBUS_TYPE_INVALID);
	
		reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
		
		dbus_message_unref(query);
		if (NULL == reply) {
			vty_out(vty,"<error> failed get reply.\n");
			if (dbus_error_is_set(&err)) {
				vty_out(vty,"%s raised: %s",err.name,err.message);
				dbus_error_free_for_dcli(&err);
			}
			return CMD_SUCCESS;
		}
		
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,&ret);
		
			if(ret == WID_DBUS_SUCCESS)
				vty_out(vty,"set the limit min flow successfully!\n");
			else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
			{
				vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
			}
			else 
			{
				vty_out(vty, "<error> other unknow error happend: %d\n", ret);
			}
		dbus_message_unref(reply);
		return CMD_SUCCESS;
	}

DEFUN(config_wlan_service_cmd_func,
	  config_wlan_service_cmd,
	  "service (enable|disable)",
	  SERVICE_STR
	  "Make service enable/disable\n"
	 )
{	int ret = CMD_SUCCESS;
	unsigned char wlan_id;
	unsigned char stat;	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	str2lower(&argv[0]);
	if (!strcmp(argv[0],"enable")||(tolower(argv[0][0]) == 'e')){
		DBusMessage *query_check, *reply_check; 
		DBusMessageIter  iter_check;
		DBusError err_check;
		int ret_check;
		
		int index = 0;
		int localid = 1;  //fengwenchao add 20110507
		int slot_id = HostSlotId;   //fengwenchao add 20110507		
		char BUSNAME[PATH_LEN];
		char OBJPATH[PATH_LEN];
		char INTERFACE[PATH_LEN];
		if(vty->node == WLAN_NODE){
			index = 0;			
			wlan_id = (int)vty->index;
		}else if(vty->node == HANSI_WLAN_NODE){
			index = vty->index;			
			wlan_id = (int)vty->index_sub;
			localid = vty->local;   //fengwenchao add 20110507
			slot_id = vty->slotindex; //fengwenchao add 20110507
		}
		/*fengwenchao add 20110507*/
		else if (vty->node == LOCAL_HANSI_WLAN_NODE)
		{
			index = vty->index;			
			wlan_id = (int)vty->index_sub;
			localid = vty->local;
			slot_id = vty->slotindex;
		}
		DBusConnection *dcli_dbus_connection = NULL;
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		/*fengwenchao add end*/
		
		ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);		
		ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);		
		ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
		query_check = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_SECURITY_WLAN_CHECK);
	/*	query_check = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
							ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_SECURITY_WLAN_CHECK);*/
		
		dbus_error_init(&err_check);
		
		dbus_message_append_args(query_check,
								 DBUS_TYPE_BYTE,&wlan_id,
								 DBUS_TYPE_INVALID);
		
		reply_check = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query_check,-1, &err_check);
		
		dbus_message_unref(query_check);
		
		if (NULL == reply_check) {
			cli_syslog_info("<error> failed get reply.\n");
			if (dbus_error_is_set(&err_check)) {
				cli_syslog_info("%s raised: %s",err_check.name,err_check.message);
				dbus_error_free_for_dcli(&err_check);
			}
			return CMD_SUCCESS;
		}
		
		dbus_message_iter_init(reply_check,&iter_check);
		dbus_message_iter_get_basic(&iter_check,&ret_check);
		
		dbus_message_unref(reply_check);
			if(ret_check == ASD_DBUS_SUCCESS)
				stat = 0;
			else if(ret_check == ASD_SECURITY_PROFILE_NOT_BIND_WLAN){
				vty_out(vty,"<error> no security profile binded\n");
				return CMD_SUCCESS;
			}			
			else if(ret == IF_POLICY_CONFLICT)
				vty_out(vty,"<error> wtp interface policy conflict\n");
			else if(ret == L3_INTERFACE_ERROR)
				vty_out(vty,"<error> you map layer3 interace error\n");
			else if(ret == SECURITYINDEX_IS_SAME)  //fengwenchao add 20110112
				vty_out(vty,"<error> wlan bingding securithindex same\n");
			else {
				vty_out(vty,"<error>  %d\n",ret_check);
				return CMD_SUCCESS;
			}
	
	}		
	else if (!strcmp(argv[0],"disable")||(tolower(argv[0][0]) == 'd'))
		stat = 1;
	
	vty_out(vty,"Wlan operation  running.......\n");
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;		
		wlan_id = (int)vty->index;
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;		
		wlan_id = (int)vty->index_sub;		
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		wlan_id = (int)vty->index_sub;	
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_ENABLE);
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_ENABLE);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&wlan_id,
							 DBUS_TYPE_BYTE,&stat,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err);
	
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
			vty_out(vty,"Wlan %d %s successfully\n",wlan_id,argv[0]);
		else if (ret == INTERFACE_NOT_BE_BINDED)
			vty_out(vty,"<error> you should bind interface first\n");
		else if(ret == L3_INTERFACE_ERROR)
			vty_out(vty,"<error> you map layer3 interace error\n");
		else if(ret == SECURITYINDEX_IS_SAME)  //fengwenchao add 20110112
			vty_out(vty,"<error> wlan bingding securithindex same\n");
		else if(ret == WID_DBUS_ERROR)
			vty_out(vty,"<error> wlan not bingding security\n");
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(config_wds_service_cmd_func,
	  config_wds_service_cmd,
	  "(wds|mesh) (enable|disable)",
	  SERVICE_STR
	  "Make wds service enable/disable\n"
	 )
{
	int ret;
	unsigned char wlan_id;
	unsigned char wds_state;/* 0x00 disable,0x01 enable for wds;0x02,disable,0x03 enbale for mesh*/	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	str2lower(&argv[1]);
	if(!strcmp(argv[0],"wds")){
		if (!strcmp(argv[1],"enable")||(tolower(argv[1][0]) == 'e'))
		{
			wds_state = 1;	
		
		}		
		else if (!strcmp(argv[1],"disable")||(tolower(argv[1][0]) == 'd'))
		{
			wds_state = 0;
		}
	}
	else if(!strcmp(argv[0],"mesh")){
		if (!strcmp(argv[1],"enable")||(tolower(argv[1][0]) == 'e'))
		{
			wds_state = 3;	
		
		}		
		else if (!strcmp(argv[1],"disable")||(tolower(argv[1][0]) == 'd'))
		{
			wds_state = 2;
		}
	}
	else{
		vty_out(vty,"UNKOWN COMMAND\n");
	}
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;		
		wlan_id = (int)vty->index;
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;		
		wlan_id = (int)vty->index_sub;
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}	
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		wlan_id = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WDS_METHOD_ENABLE);
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WDS_METHOD_ENABLE);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&wlan_id,
							 DBUS_TYPE_BYTE,&wds_state,
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
	if((wds_state&0x02) == 0){
		vty_out(vty,"Wlan %d %s wds successfully\n",wlan_id,argv[1]);
	}
	else{
		vty_out(vty,"Wlan %d %s mesh successfully\n",wlan_id,argv[1]);
	}
	else if (ret == INTERFACE_NOT_BE_BINDED)
		vty_out(vty,"<error> you should bind interface first\n");
	else if(ret == L3_INTERFACE_ERROR)
		vty_out(vty,"<error> you map layer3 interace error\n");
	else if(ret == WLAN_BE_DISABLE)
		vty_out(vty,"<error> you must first service enable wlan\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
}

DEFUN(create_wlan_cmd_func,
		create_wlan_cmd,
		"create wlan WLANID WLANNAME .ESSID", /*wcl modify*/
		"create wireless server\n"
		"wlan service\n"
		"assign WLAN ID for wlan\n"
		"assign WLAN NAME\n"
		"assign ESSID\n"
	)
{
	int ret,len;
	unsigned char isAdd;	
	unsigned char wlan_id;
	unsigned int wlan_int;
	char *name;
	char *ESSID;	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	isAdd = 1;
/*	wlan_id = atoi(argv[0]);*/
	ret = parse_int_ID((unsigned char*)argv[0], &wlan_int);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(wlan_int >= WLAN_NUM || wlan_int == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	wlan_id = wlan_int;
	len = strlen(argv[1]);
	if(len > 15){		
		vty_out(vty,"<error> wlan name is too long,out of the limit of 15\n");
		return CMD_SUCCESS;
	}
	name = (char*)malloc(strlen(argv[1])+1);
	memset(name, 0, strlen(argv[1])+1);
	memcpy(name, argv[1], strlen(argv[1]));		
	/*len = strlen(argv[2]);
	if(len > 32){		
		vty_out(vty,"<error> essid is too long,out of the limit of 32\n");
		return CMD_SUCCESS;
	}
	ESSID = (char*)malloc(strlen(argv[2])+1);
	memset(ESSID, 0, strlen(argv[2])+1);
	memcpy(ESSID, argv[2], strlen(argv[2]));
	*/ /*wcl modify*/
		/*wcl add*/
	ESSID= WID_parse_CMD_str(&argv[2],argc-2,NULL,0);
	if(ESSID== NULL){		
		vty_out(vty,"UNKNOWN COMMAND\n");
		vty_out(vty,"COMMAND should be :create wlan 1 w1 aq w\n");
		if(name){
			free(name);
			name = NULL;
		}
		return CMD_SUCCESS;
	}
	len = strlen(ESSID);
	if(len > 32)
	{		
		vty_out(vty,"<error> essid is too long,out of the limit of 32\n");
		if(name){
			free(name);
			name = NULL;
		}
		if(ESSID){
			free(ESSID);
			ESSID = NULL;
		}	
		return CMD_SUCCESS;
	}
	/*end*/
	if(-1 == ssid_illegal_character_check(ESSID,len)){
		vty_out(vty,"essid is null!or checkout the parameter len!\n");
		if(name){
			free(name);
			name = NULL;
		}
		if(ESSID){
			free(ESSID);
			ESSID = NULL;
		}
		return CMD_SUCCESS;
	}
	else if(-2==ssid_illegal_character_check(ESSID,len)){
		vty_out(vty,"illegal essid name!! ` \ \" & * ( ) not supported!\n");
		if(name){
			free(name);
			name = NULL;
		}
		if(ESSID){
			free(ESSID);
			ESSID = NULL;
		}
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;		
	}else if(vty->node == HANSI_NODE){
		index = vty->index;	
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_NODE)
	{
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_ADD_DEL_WLAN);
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_ADD_DEL_WLAN);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_BYTE,&isAdd,								
						DBUS_TYPE_BYTE,&wlan_id,
						DBUS_TYPE_STRING,&name,
						DBUS_TYPE_STRING,&ESSID,							 
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
		if(ESSID)
		{
			free(ESSID);
			ESSID = NULL;
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == 0){
				vty_out(vty,"Wlan %d was successfully created.\n",wlan_id);
		}
		else if(ret == WLAN_ID_BE_USED)
			vty_out(vty,"<error> wlan id exist\n");
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	free(name);
	free(ESSID);

	return CMD_SUCCESS;	
}

DEFUN(create_wlan_cmd_func_cn,
		create_wlan_cmd_cn,
		"create wlan_ascii WLANID WLANNAME ESSID",
		"create wireless cn server\n"
		"wlan cn service\n"
		"assign cn WLAN ID for wlan\n"
		"assign cn WLAN NAME\n"
		"assign cn ESSID\n"
	)
{
	int ret,len;
	unsigned char isAdd;	
	unsigned char wlan_id;
	unsigned char *name;
	unsigned char *ESSID;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char a[2];
	isAdd = 1;
	int i,j;
/*	wlan_id = atoi(argv[0]);*/
	
	ret = parse_char_ID((char*)argv[0], &wlan_id);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_SUCCESS;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	len = strlen(argv[1]);
	if(len > 15){		
		vty_out(vty,"<error> wlan name is too long,out of the limit of 15\n");
		return CMD_SUCCESS;
	}
	if(check_ascii_32_to126(argv[1]) == CW_FALSE_DCLI)
	{
		vty_out(vty,"<error> wlan name %s is illegal\n",argv[1]);
		return WID_ILLEGAL_INPUT;
	}
	
	name = (char*)malloc(strlen(argv[1])+1);
	memset(name, 0, strlen(argv[1])+1);
	memcpy(name, argv[1], strlen(argv[1]));		
	len = strlen(argv[2]);
	if(len > 64){		
		vty_out(vty,"<error> essid is too long,out of the limit of 32\n");
		if(name){
			free(name);
			name = NULL;
		}
		return CMD_SUCCESS;
	}
#if 0
	ESSID = (char*)malloc(strlen(argv[2])+1);
	memset(ESSID, 0, strlen(argv[2])+1);
	for(i = 2;i < argc; i++ ){
		parse_char_ID((char*)argv[i], &id);		
		ESSID[i-2] = id;	
	}
#endif
	unsigned long ul;
	ESSID = (unsigned char*)malloc(ESSID_DEFAULT_LEN);
	memset(ESSID, 0, ESSID_DEFAULT_LEN);
	for(i = 0;i < len;i=i+2)
	{
		memcpy(a,argv[2]+i,2);
		ul = HexToDec(a);
		ESSID[i/2] = ul;
	}
	
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	if(vty->node == CONFIG_NODE){
		index = 0;		
	}else if(vty->node == HANSI_NODE){
		index = vty->index;		
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_NODE)
	{
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/
	
	ret = create_wlan_CN(index,localid,isAdd,wlan_id,name,ESSID,dcli_dbus_connection);
	
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
		if(name)
		{
			free(name);
			name = NULL;
		}
		if(ESSID)
		{
			free(ESSID);
			ESSID = NULL;
		}
	}
	else if(ret == 0){
			vty_out(vty,"Wlan %d was successfully created.\n",wlan_id);
	}
	else if(ret == WLAN_ID_BE_USED)
		vty_out(vty,"<error> wlan id exist\n");
	else if(ret == WID_ILLEGAL_INPUT)
		vty_out(vty,"<error> illegal input.\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	//dbus_message_unref(reply);
	free(name);
	free(ESSID);

	return CMD_SUCCESS;	
}
DEFUN(delete_wlan_cmd_func,
		delete_wlan_cmd,
		"delete wlan WLANID",
		"delete wireless server\n"
		"wlan service\n"
		"WLAN ID of wlan\n"
	)
{
	int ret;
	unsigned char isAdd;	
	unsigned char wlan_id;
	char *name;
	char *ESSID;	
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	isAdd = 0;
/*	wlan_id = atoi(argv[0]);*/
	
	ret = parse_char_ID((char*)argv[0], &wlan_id);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_SUCCESS;
	}	
	if(wlan_id >= WLAN_NUM || wlan_id == 0){
		vty_out(vty,"<error> wlan id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	name = (char*)malloc(1);
	memset(name, 0, 1);
	ESSID = (char*)malloc(1);
	memset(ESSID, 0, 1);
	
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == CONFIG_NODE){
		index = 0;		
	}else if(vty->node == HANSI_NODE){
		index = vty->index;				
		localid = vty->local;	//fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}	
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_NODE)
	{
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_ADD_DEL_WLAN);
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_OBJPATH,\
						WID_DBUS_INTERFACE,WID_DBUS_CONF_METHOD_ADD_DEL_WLAN);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_BYTE,&isAdd,								
						DBUS_TYPE_BYTE,&wlan_id,
						DBUS_TYPE_STRING,&name,
						DBUS_TYPE_STRING,&ESSID,							 
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
		if(ESSID)
		{
			free(ESSID);
			ESSID = NULL;
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == 0){
			vty_out(vty,"Wlan %d was successfully deleted.\n",wlan_id);
		}
		else if(ret == WLAN_ID_NOT_EXIST)
			vty_out(vty,"<error> wlan id does not exist\n");
		else if(ret == WLAN_BE_ENABLE)
			vty_out(vty,"<error> wlan is enable,please disable it first\n");
		else if(ret == RADIO_IN_EBR)
			vty_out(vty,"<error> some radios interface in ebr,please delete it from ebr first\n");
		else if (ret == DELETE_WLAN_SPEN_TOO_MUCH_TIME) 			 /* Huangleilei add for AXSSZFI-1622 */
			vty_out(vty, "<warning> AC has to create a new thread to delete this wlan\n");
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	free(name);
	free(ESSID);

	return CMD_SUCCESS;	
}


DEFUN(wlan_apply_security_cmd_func,
	  wlan_apply_security_cmd,
	  "apply securityID SECURITYID",
	  SERVICE_STR
	  "security profile bind with wlan\n"
	  "SecurityID which you bind wlan\n"
	 )
{	int ret;
	unsigned char security_id;
	unsigned char WlanID;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	//WlanID = (int)vty->index;	
/*	security_id = atoi(argv[0]);*/
	
	ret = parse_char_ID((char*)argv[0], &security_id);
	if(ret != WID_DBUS_SUCCESS){
            if(ret == WID_ILLEGAL_INPUT){
            	vty_out(vty,"<error> illegal input:Input exceeds the maximum value of the parameter type \n");
            }
			else{
		vty_out(vty,"<error> unknown id format\n");
			}
		return CMD_SUCCESS;
	}	
	if(security_id >= WLAN_NUM || security_id == 0){
		vty_out(vty,"<error> security id should be 1 to %d\n",WLAN_NUM-1);
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (int)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (int)vty->index_sub;	
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (int)vty->index_sub;	
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_SECURITY_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_SECURITY_METHOD_APPLY_WLAN);
/*	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_SECURITY_OBJPATH,\
						ASD_DBUS_SECURITY_INTERFACE,ASD_DBUS_SECURITY_METHOD_APPLY_WLAN);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&security_id,
							 DBUS_TYPE_BYTE,&WlanID,
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

		if(ret == ASD_DBUS_SUCCESS)
			vty_out(vty,"Security %d was successfully binded.\n",security_id);
		else if(ret == ASD_WLAN_NOT_EXIST)
			vty_out(vty,"<error> wlan %d does not exist\n",WlanID);
		else if(ret == ASD_SECURITY_NOT_EXIST)
			vty_out(vty,"<error> asd security profile does not exist\n");
		else if(ret == ASD_SECURITY_PROFILE_NOT_INTEGRITY)
			vty_out(vty,"<error> asd security profile does not complete\n");
		else if(ret == ASD_SECURITY_TYPE_NOT_MATCH_ENCRYPTION_TYPE)
			vty_out(vty,"<error> encryption type dose not match security type\n");
		else if(ret == ASD_SECURITY_RDC_NOT_EXIT)
			vty_out(vty,"<error> security rdc has not config!\n");
		else if(ret == ASD_SECURITY_WLAN_SHOULD_BE_DISABLE)
			vty_out(vty,"<error> wlan is enable,you should disable it first\n");
		else 
		{
			vty_out(vty, "<error> other unknow error happend: %d\n", ret);
		}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(wlan_apply_interface_cmd_func,
	  wlan_apply_interface_cmd,
	  "wlan apply interface IFNAME",
	  "wlan apply interface IFNAME\n"
	  "wlan binding information\n" 
	  "assign Interface name\n"

	 )
{

	int ret;
	int ret6;
	unsigned char WlanID = 0;
	unsigned char * ifname;
	int insize = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	insize = strlen(argv[0]);
	if(insize > 16)
	{
		vty_out(vty,"<error> input parameter %s size is %d,excel the limit of 16\n",argv[0],insize);
		return CMD_SUCCESS;	
	}
	//WlanID = (int)vty->index;
	
	ifname = (char*)malloc(strlen(argv[0])+1);
	memset(ifname, 0, strlen(argv[0])+1);
	memcpy(ifname, argv[0], strlen(argv[0]));	
	
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (int)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (int)vty->index_sub;			
		localid = vty->local;	//fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (int)vty->index_sub;	
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_APPAY_WLAN_IFNAME);
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_APPAY_WLAN_IFNAME);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_STRING,&ifname,
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
		if(ifname)
		{
			free(ifname);
			ifname = NULL;
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,&ret6);

	if((ret == 0)&&(ret6 == 0))
	{
		vty_out(vty,"IPV4:wlan %d binding interface %s successfully.\n",WlanID,ifname);
		vty_out(vty,"IPV6:wlan %d binding interface %s successfully.\n",WlanID,ifname);
	}
	else if(ret == 0){
		vty_out(vty,"IPV4:wlan %d binding interface %s successfully.\n",WlanID,ifname);
		if(ret6 == WLAN_ID_NOT_EXIST) 
			vty_out(vty,"<warnning>IPV6: wlan id does not exist\n");
		else if(ret6 == APPLY_IF_FAIL){
			vty_out(vty,"<warnning>IPV6: interface %s dose not exist\n",ifname);
		}
		else if(ret6 == WLAN_BE_ENABLE)
			vty_out(vty,"<warnning>IPV6: wlan is enable,please disable it first\n");
		else vty_out(vty,"<warnning>IPV6: wlan apply interface fail.errno is %d.\n",ret6);
	}
	else if(ret6 ==0){
		if(ret == WLAN_ID_NOT_EXIST) 
			vty_out(vty,"<warnning>IPV4: wlan id does not exist\n");
		else if(ret == APPLY_IF_FAIL){
			vty_out(vty,"<warnning>IPV4: interface %s dose not exist\n",ifname);
		}
		else if(ret == WLAN_BE_ENABLE){
			vty_out(vty,"<warnning>IPV4: wlan is enable,please disable it first\n");
		}
		else if(ret == IF_BINDING_FLAG){
			vty_out(vty,"<error>IPV4: interface %s has be binded in other hansi.\n",ifname);
		}
		else if (ret == WID_INTERFACE_NOT_BE_LOCAL_BOARD){
			vty_out(vty,"<error> %s is no local interface, permission denial\n",ifname);
		}
		else 
			vty_out(vty,"<warnning>IPV4: wlan apply interface fail.errno is %d.\n",ret);
		vty_out(vty,"IPV6:wlan %d binding interface %s successfully.\n",WlanID,ifname);
	}
	else{
		if(ret == WLAN_ID_NOT_EXIST) 
			vty_out(vty,"<error>IPV4: wlan id does not exist\n");
		else if(ret == APPLY_IF_FAIL){
			vty_out(vty,"<error>IPV4: interface %s dose not exist\n",ifname);
		}
		else if(ret == WLAN_BE_ENABLE){
			vty_out(vty,"<error>IPV4: wlan is enable,please disable it first\n");
		}
		else if(ret == IF_BINDING_FLAG){
			vty_out(vty,"<error>IPV4: interface %s has be binded in other hansi.\n",ifname);
		}
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> IPV4: you want to delete wlan, please do not operate like this\n");
		}
		else 
			vty_out(vty,"<error>IPV4: wlan apply interface fail.errno is %d.\n",ret);
		
		if(ret6 == WLAN_ID_NOT_EXIST) 
			vty_out(vty,"<error>IPV6: wlan id does not exist\n");
		else if(ret6 == APPLY_IF_FAIL){
			vty_out(vty,"<error>IPV6: interface %s dose not exist\n",ifname);
		}
		else if(ret6 == WLAN_BE_ENABLE)
			vty_out(vty,"<error>IPV6: wlan is enable,please disable it first\n");
		else if (ret == WID_INTERFACE_NOT_BE_LOCAL_BOARD){
			vty_out(vty,"<error> %s is no local interface, permission denial\n",ifname);
		}		
		else if (ret6 == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else vty_out(vty,"<error>IPV6: wlan apply interface fail.errno is %d.\n",ret6);
	}
		
	dbus_message_unref(reply);
	free(ifname);
	ifname = NULL;
	return CMD_SUCCESS;
}
DEFUN(wlan_apply_ipv6interface_cmd_func,
	  wlan_apply_ipv6interface_cmd,
	  "wlan apply ipv6interface IFNAME",
	  "wlan apply ipv6interface IFNAME\n"
	  "wlan binding ipv6 information\n" 
	  "assign Interface ipv6 name\n"

	 )
{

	int ret;
	unsigned char WlanID = 0;
	unsigned char * ifname;
	int insize = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	insize = strlen(argv[0]);
	if(insize > 16)
	{
		vty_out(vty,"<error> input parameter %s size is %d,excel the limit of 16\n",argv[0],insize);
		return CMD_SUCCESS;	
	}
	WlanID = (int)vty->index;
	
	ifname = (char*)malloc(strlen(argv[0])+1);
	memset(ifname, 0, strlen(argv[0])+1);
	memcpy(ifname, argv[0], strlen(argv[0]));	

	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (int)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (int)vty->index_sub;	
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (int)vty->index_sub;	
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_APPAY_WLAN_IFNAME_IPV6);
	
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_APPAY_WLAN_IFNAME_IPV6);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_STRING,&ifname,
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
		if(ifname)
		{
			free(ifname);
			ifname = NULL;
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"wlan %d binding interface %s successfully.\n",WlanID,ifname);
	}				
	else if(ret == WLAN_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> wlan id does not exist\n");
	}
	else if(ret == APPLY_IF_FAIL){
		vty_out(vty,"<error> interface %s dose not exist\n",ifname);
	}
	else if(ret == WLAN_BE_ENABLE){
		vty_out(vty,"<error> wlan is enable,please disable it first\n");
	}
	else if(ret == BINDING_IPV6_ADDRE_RROR){
		vty_out(vty,"<error> wlan bingding ipv6 addr error make sure interface have ipv6 address\n");
	}
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	
	else{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);
	free(ifname);
	ifname = NULL;
	return CMD_SUCCESS;
}

/*added by weiay 20080701*/
DEFUN(wlan_delete_interface_cmd_func,
	  wlan_delete_interface_cmd,
	  "wlan delete interface IFNAME",
	  "wlan delete interface IFNAME\n"
	  "wlan delete binding information\n" 
	  "delete Interface name\n"

	 )


{
	int ret;
	unsigned char WlanID = 0;
	unsigned char * ifname;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	//WlanID = (unsigned char)vty->index;
	
	ifname = (char*)malloc(strlen(argv[0])+1);
	memset(ifname, 0, strlen(argv[0])+1);
	memcpy(ifname, argv[0], strlen(argv[0]));	

	
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (int)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (int)vty->index_sub;	
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_DELETE_IF);
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_DELETE_IF);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_STRING,&ifname,
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
		if(ifname)
		{
			free(ifname);
			ifname = NULL;
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	if(ret == 0)
	{
		vty_out(vty,"wlan %d delete interface %s successfully.\n",WlanID,ifname);
	}	
	else if(ret == WLAN_NOT_BINDING_IF)
	{
		vty_out(vty,"<error> wlan does not bind this interface\n");
	}		
	else if(ret == WLAN_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> wlan id does not exist\n");
	}
	else if(ret == APPLY_IF_FAIL){
		vty_out(vty,"<error> interface %s does not exist\n",ifname);
	}
	else if(ret == WLAN_BE_ENABLE){
		vty_out(vty,"<error> wlan is enable,please disable it first\n");
	}
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);
	free(ifname);
	ifname = NULL;
	return CMD_SUCCESS;


}
DEFUN(wlan_show_interface_cmd_func,
		wlan_show_interface_cmd,
		"wlan show interface (list|all)",
		"wlan show interface list\n"
		"wlan show interface operate\n" 
		"wlan binding interface list\n" 
		"show interface list\n"
		)
{
	return CMD_SUCCESS; 

}

DEFUN(wlan_hideessid_cmd_func,
	     wlan_hideessid_cmd,
	     "hideessid (yes|no)",
	     SERVICE_STR
	    "Make wlan hideessid yes/no\n"
	 )
{
	int ret;
	unsigned char WlanID = 0;
	unsigned char hideessid;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	//WlanID = (unsigned char)vty->index;
	
	if (!strcmp(argv[0],"yes"))
	{
		hideessid = 1;	
	}		
	else if (!strcmp(argv[0],"no"))
	{
		hideessid = 0;
	}
	else
	{
		vty_out(vty,"<error> input patameter should only be 'yes' or 'no'\n");
		return CMD_SUCCESS;
	}

	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (unsigned char)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (unsigned char)vty->index_sub;	
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (unsigned char)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_HIDE_ESSID);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_HIDE_ESSID);*/

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_BYTE,&hideessid,
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
		vty_out(vty,"wlan %d set hidessid %s successfully\n",WlanID,argv[0]);
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else if(ret == WLAN_BE_ENABLE)
		vty_out(vty,"<error> wlan is enable, please disable it first\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;

}

DEFUN(wlan_map_interface_cmd_func,
	  wlan_map_interface_cmd,
	  "wlan map interface",
	  "wlan map interface\n"
	  "wlan map L3 interface\n" 
	  "wlan map L3 interface\n" 
	 )
{
	int ret;
	unsigned char WlanID = 0;
	unsigned char wlan_oplicy;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
//	WlanID = (unsigned char)vty->index;
	
	wlan_oplicy = 1;
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (unsigned char)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (unsigned char)vty->index_sub;	
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507		
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (unsigned char)vty->index_sub;	
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_L3IF_POLICY);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_L3IF_POLICY);*/

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_BYTE,&wlan_oplicy,
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
		vty_out(vty,"wlan %d map interface successfully\n",WlanID);
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else if(ret == WLAN_BE_ENABLE)
		vty_out(vty,"<error> wlan is enable, please disable it first\n");
	else if(ret == WLAN_CREATE_L3_INTERFACE_FAIL)
		vty_out(vty,"<error> wlan create layer3 interface failed\n");	
	else if(ret == WLAN_BINDING_VLAN)
		vty_out(vty,"WLAN have already binding vlan,please undo wlan-vlan binding first\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;


}
DEFUN(wlan_unmap_interface_cmd_func,
	  wlan_unmap_interface_cmd,
	  "wlan unmap interface",
	  "wlan unmap interface\n"
	  "wlan unmap L3 interface\n" 
	  "wlan unmap L3 interface\n" 
	 )
{
	int ret;
	unsigned char WlanID = 0;
	unsigned char wlan_oplicy;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	WlanID = (unsigned char)vty->index;
	
	wlan_oplicy = 0;

	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_L3IF_POLICY);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_BYTE,&wlan_oplicy,
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
		vty_out(vty,"wlan %d unmap interface successfully\n",WlanID);
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"wlan id does not exist\n");
	else if(ret == WLAN_BE_ENABLE)
		vty_out(vty,"wlan is enable, please disable it first\n");
	else if(ret == WLAN_CREATE_L3_INTERFACE_FAIL)
		vty_out(vty,"wlan create layer3 interface failed\n");	
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;


}

DEFUN(wlan_bss_map_interface_cmd_func,
	  wlan_bss_map_interface_cmd,
	  "wlan bss map interface",
	  "wlan bss map interface\n"
	  "wlan bss map L3 interface\n" 
	  "wlan bss map L3 interface\n" 
	 )
{
	int ret;
	unsigned char WlanID = 0;
	unsigned char wlan_oplicy;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	WlanID = (unsigned char)vty->index;
	
	wlan_oplicy = 2;

	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_L3IF_POLICY);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_BYTE,&wlan_oplicy,
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
		vty_out(vty,"wlan %d bss map interface successfully\n",WlanID);
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> Wlan id does not exist\n");
	else if(ret == WLAN_BE_ENABLE)
		vty_out(vty,"<error> wlan is enable, please disable it first\n");
	else if(ret == WLAN_CREATE_L3_INTERFACE_FAIL)
		vty_out(vty,"<error> wlan create layer3 interface failed\n");	
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;


}
DEFUN(wlan_bss_unmap_interface_cmd_func,
	  wlan_bss_unmap_interface_cmd,
	  "wlan bss unmap interface",
	  "wlan bss unmap interface\n"
	  "wlan bss unmap L3 interface\n" 
	  "wlan bss unmap L3 interface\n" 
	 )
{
	int ret;
	unsigned char WlanID = 0;
	unsigned char wlan_oplicy;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	
	WlanID = (unsigned char)vty->index;
	
	wlan_oplicy = 0;

	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_L3IF_POLICY);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_BYTE,&wlan_oplicy,
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
		vty_out(vty,"Wlan %d bss unmap interface successfully\n",WlanID);
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else if(ret == WLAN_BE_ENABLE)
		vty_out(vty,"<error> wlan is enable, please disable it first\n");
	else if(ret == WLAN_CREATE_L3_INTERFACE_FAIL)
		vty_out(vty,"<error> wlan create layer3 interface failed\n");	
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;


}

/*xm add 08/12/03*/
DEFUN(wlan_max_num_cmd_func,
	  wlan_max_num_cmd,
	  "set wlan max sta num NUM",
	  "set wlan max sta num\n"
	  "wlan max sta\n" 
	  "max num\n"

	 )
{
	int ret;
	unsigned char WlanID = 0;
	unsigned int wlan_max_sta=0;
	unsigned int type=2;
	unsigned int wlanid;
	unsigned int stanum;
	unsigned int radioid = 0; //fengwenchao add 20110512
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	
	//WlanID = (unsigned char)vty->index;
	
	//wlan_max_sta=atoi(argv[0]);
	ret = parse_int_ID((char*)argv[0], &wlan_max_sta);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown NUM format\n");
		return CMD_SUCCESS;
	}	

	/*compare max_sta_num  with  current sta num*/
	/*fengwenchao add 20110412*/
	if((wlan_max_sta > 65536)||(wlan_max_sta < 1))
	{
		vty_out(vty,"<error> input max sta num should be 1-65536\n");
		return CMD_SUCCESS;
	}
	/*fengwenchao add end*/
	
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (unsigned char)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (unsigned char)vty->index_sub;	
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (unsigned char)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	wlanid=(unsigned int)WlanID;
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_GET_STA_INFO);
/*	query = dbus_message_new_method_call(ASD_DBUS_BUSNAME,ASD_DBUS_STA_OBJPATH,\
						ASD_DBUS_STA_INTERFACE,ASD_DBUS_STA_METHOD_GET_STA_INFO);*/
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&wlanid,
							 DBUS_TYPE_UINT32,&radioid,     //fengwenchao add 20110512
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
	dbus_message_iter_get_basic(&iter,&stanum);
	dbus_message_unref(reply);

	if(stanum==-1){
		vty_out(vty,"<error> wlanid not exist.\n");
		return CMD_SUCCESS;
	}
	
	if(wlan_max_sta< stanum){
		vty_out(vty,"<error> %d sta(s) has accessed before you set max sta num %d .\n",stanum,wlan_max_sta);
		return CMD_SUCCESS;
	}


	memset(BUSNAME,0,PATH_LEN);	
	memset(OBJPATH,0,PATH_LEN);
	memset(INTERFACE,0,PATH_LEN);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_APPAY_WLAN_MAX_STA);
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_APPAY_WLAN_MAX_STA);*/

	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_UINT32,&wlan_max_sta,
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
		vty_out(vty,"wlan %d set max sta num %d successfully\n",WlanID,wlan_max_sta);
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else if(ret == WID_DBUS_ERROR)
		vty_out(vty,"<error> operation fail .\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);

	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}


DEFUN(set_interface_nasid_cmd_func,
		set_interface_nasid_cmd,
		"wlan apply interface IFNAME nas_identifier NASID",
		"set interface nas_id\n"
		"interface information\n"
		"interface name\n"
		"nas identifier infornation\n"
		"nas identifier\n"
		)
{
	int ret;
	char *ifname;
	char *nas_id;
	int insize = 0;
	unsigned char WLANID;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	/*make robust code*/
	insize = strlen(argv[0]);
	if(insize > 16)
	{
		vty_out(vty,"<error> the length of input parameter %s is %d ,excel the limit of 16\n",argv[0],insize);
		return CMD_SUCCESS;	
	}	
	insize = strlen(argv[1]);
	if(insize > 128)
	{
		vty_out(vty,"<error> the length of input parameter %s is %d ,excel the limit of 128\n",argv[0],insize);
		return CMD_SUCCESS;	
	}
	insize = strcheck(&argv[1]);	
	if(insize == 0)
	{
		vty_out(vty,"<error> nas identifier %s include unknow character\n",argv[1]);
		return CMD_SUCCESS;	
	}
	ifname = (char*)malloc(strlen(argv[0])+1);
	memset(ifname, 0, strlen(argv[0])+1);
	memcpy(ifname, argv[0], strlen(argv[0]));	
	nas_id = (char*)malloc(strlen(argv[1])+1);
	memset(nas_id, 0, strlen(argv[1])+1);
	memcpy(nas_id, argv[1], strlen(argv[1]));	
	//WLANID = (int)vty->index;
	
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WLANID = (int)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WLANID = (int)vty->index_sub;	
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WLANID = (int)vty->index_sub;	
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_SET_NAS_IDENTIFIER);
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_SET_NAS_IDENTIFIER);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_BYTE,&WLANID,
						DBUS_TYPE_STRING,&ifname,
						DBUS_TYPE_STRING,&nas_id,						 
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		if(ifname)
		{
			free(ifname);
			ifname = NULL;
		}
		if(nas_id)
		{
			free(nas_id);
			nas_id = NULL;
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == 0)
			vty_out(vty,"wlan %d apply interface %s nas_id %s successfully.\n",WLANID,ifname,nas_id);
		else if(ret == INTERFACE_NOT_EXIST)
			vty_out(vty,"<error> interface %s does not exist\n",ifname);		
		else if(ret == APPLY_IF_FAIL)
			vty_out(vty,"<error>interface %s unused\n",ifname);
		else if(ret == WLAN_APPLY_SECURITY_FIRST)
			vty_out(vty,"<error> you should apply security first\n");		
		else if(ret == WLAN_NOT_NEED_NAS)
			vty_out(vty,"<error> no nas_id needed,please use <wlan apply interface IFNAME>,without nas_identifier\n");
		else if(ret == WLAN_BE_ENABLE)
			vty_out(vty,"<error> wlan be enable,please service disable first\n");		
		else if(ret == IF_BINDING_FLAG){
			vty_out(vty,"<error>interface %s has be binded in other hansi.\n",ifname);
		}
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	free(ifname);
	ifname = NULL;	
	free(nas_id);
	nas_id = NULL;
	return CMD_SUCCESS;	

}
DEFUN(remove_interface_nasid_cmd_func,
		remove_interface_nasid_cmd,
		"wlan remove interface IFNAME nas_identifier",
		CONFIG_STR
		"remove interface nas_identifier\n"
		"remove interface nas_identifier\n"
		"interface name\n"
		"remove interface nas_identifier\n"
		)
{
	int ret;
	char *ifname;
	int insize = 0;
	unsigned char WLANID;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	/*make robust code*/
	insize = strlen(argv[0]);
	if(insize > 16)
	{
		vty_out(vty,"<error> the length of input parameter %s is %d ,excel the limit of 16\n",argv[0],insize);
		return CMD_SUCCESS;	
	}	
	
	ifname = (char*)malloc(strlen(argv[0])+1);
	memset(ifname, 0, strlen(argv[0])+1);
	memcpy(ifname, argv[0], strlen(argv[0]));	
		
//	WLANID = (int)vty->index;
	
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WLANID = (int)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WLANID = (int)vty->index_sub;	
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WLANID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_REMOVE_NAS_IDENTIFIER);
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_REMOVE_NAS_IDENTIFIER);*/
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_BYTE,&WLANID,
						DBUS_TYPE_STRING,&ifname,					 
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		cli_syslog_info("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			cli_syslog_info("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		if(ifname)
		{
			free(ifname);
			ifname = NULL;
		}
		return CMD_SUCCESS;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

		if(ret == 0)
			vty_out(vty,"wlan %d remove interface %s nas_id successfully.\n",WLANID,ifname);
		else if(ret == WLAN_ID_NOT_EXIST)
			vty_out(vty,"<error> wlan id is not exist\n",WLANID);
		else if(ret == WLAN_NOT_BINDING_IF)
			vty_out(vty,"<error>wlan %d is not binding interface %s\n",WLANID,ifname);
		else if(ret == APPLY_IF_FAIL)
			vty_out(vty,"<error>interface %s error\n",ifname);	
		else if(ret == WLAN_NOT_NEED_NAS)
			vty_out(vty,"<error> no nas_id needed,please use <wlan apply interface IFNAME>,without nas_identifier\n");
		else if(ret == WLAN_BE_ENABLE)
			vty_out(vty,"<error> wlan be enable,please service disable first\n");		
		else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
		{
			vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
		}
		else
			vty_out(vty,"<error>  %d\n",ret);
	dbus_message_unref(reply);
	free(ifname);
	ifname = NULL;
	return CMD_SUCCESS;	

}


/*xm add 08/12/29*/
DEFUN(wlan_balance_parameter_cmd_func,
	  wlan_balance_parameter_cmd,
	  "set wlan number balance parameter NUMBER",
	  "set wlan balance parameter\n"
	  "balance method is number\n"
	  "wlan balance parameter\n" 
	  "balance parameter\n"
	  "balance parameter <1-10>\n"
	 )
{
	int ret;
	unsigned char WlanID = 0;
	unsigned int bal_para=0;
	int res = WID_DBUS_SUCCESS;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	
//	WlanID = (unsigned char)vty->index;
	res = parse_int((char*)argv[0], &bal_para);
	if(res != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown input\n");
		return CMD_SUCCESS;
	}	
	if(bal_para > 10 || bal_para < 1){
		vty_out(vty,"<error> balance parameter should be 1 to 10\n");
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (int)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (int)vty->index_sub;	
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (int)vty->index_sub;	
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_APPAY_WLAN_BALANCE_PARA);
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_APPAY_WLAN_BALANCE_PARA);*/

	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
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
		vty_out(vty,"set wlan %d number balance parameter %d successfully.\n",WlanID,bal_para);
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else if(ret == WID_DBUS_ERROR)
		vty_out(vty,"<error> operation fail .\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	

	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/*xm add 09/02/05*/
DEFUN(wlan_flow_balance_parameter_cmd_func,
	  wlan_flow_balance_parameter_cmd,
	  "set wlan flow balance parameter NUMBER",
	  "set wlan balance parameter\n"
	  "balance method is flow\n"
	  "wlan balance parameter\n" 
	  "balance parameter\n"
	  "balance parameter <1-30>\n"
	 )
{
	int ret;
	unsigned char WlanID = 0;
	unsigned int bal_para=0;
	int res = WID_DBUS_SUCCESS;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	
//	WlanID = (unsigned char)vty->index;
	res = parse_int((char*)argv[0], &bal_para);
	if(res != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown input\n");
		return CMD_SUCCESS;
	}	
	if(bal_para > 30 || bal_para < 1){
		vty_out(vty,"<error> balance parameter should be 1 to 30.\n");
		return CMD_SUCCESS;
	}


	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (int)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (int)vty->index_sub;	
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (int)vty->index_sub;	
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_APPAY_WLAN_FLOW_BALANCE_PARA);
	
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_APPAY_WLAN_FLOW_BALANCE_PARA);*/

	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
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
		vty_out(vty,"set wlan %d flow balance parameter %d successfully.\n",WlanID,bal_para);
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else if(ret == WID_DBUS_ERROR)
		vty_out(vty,"<error> operation fail .\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	

	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

/* zhangshu add for set eap mac,2010-10-22 */
DEFUN(wlan_set_eap_mac_func,
	  wlan_set_eap_mac_cmd,
	  "set 1xServer switch (enable|disable) based mac MAC",
	  "set eap switch\n"
	  "set eap switch\n" 
	  "set eap switch enable/disable\n"
	  "set eap mac\n"
	  "set eap mac\n"
	 )
{
	int ret;
	unsigned char WlanID = 0;
	unsigned char policy =0;
	unsigned char *eap_mac = NULL;
	
	int i = 0;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	
	if (!strcmp(argv[0],"enable")){
		policy = 1;
	}
	else if (!strcmp(argv[0],"disable")){
	    policy = 0;
	} 
	else {
		vty_out(vty,"<error> unknown input\n");
		return CMD_SUCCESS;
	}
    /* parse mac */
    if(strcmp(argv[1],"0") != 0)
    {
        ret = wid_mac_format_check((char *)argv[1],strlen(argv[1]));
    	if (CMD_FAILURE == ret) {
    		vty_out(vty,"<error> Unknown mac addr format.\n");
    		return CMD_FAILURE;
    	}
	}
	int len = strlen(argv[1]);
	eap_mac = (char*)malloc(len+1);
	memset(eap_mac,0,len+1);
	memcpy(eap_mac,argv[1],len);
   
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (int)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = (int)vty->index;			
		WlanID = (int)vty->index_sub;	
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (int)vty->index_sub;	
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DEBUS_WLAN_METHOD_SET_EAP_MAC);
	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_BYTE,&policy,
							 DBUS_TYPE_STRING,&eap_mac,
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
		vty_out(vty,"operation successfully.\n");
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else if(ret == WID_DBUS_ERROR)
		vty_out(vty,"<error> operation fail .\n");
	else if(ret == WLAN_BE_ENABLE)//hanjunwei changed
		vty_out(vty,"<error> wlan is enable,disable it first.\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
    free(eap_mac);
	eap_mac = NULL;
	
	return CMD_SUCCESS;
}
/*zhangshu add end */

DEFUN(wlan_load_balance_cmd_func,
	  wlan_load_balance_cmd,
	  "set wlan (number|flow|disable) balance",
	  "wlan load balance\n"
	  "the method of balance\n" 
	  "enable or disable\n"

	 )
{
	int ret;
	unsigned char WlanID = 0;
	unsigned char state=0;
	unsigned char method=0;
	const char en[]="enable";
	const char di[]="disable";

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
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (int)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = (int)vty->index;			
		WlanID = (int)vty->index_sub;	
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (int)vty->index_sub;	
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_APPAY_WLAN_BALANCE_SWITCH);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_APPAY_WLAN_BALANCE_SWITCH);*/

	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
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
		vty_out(vty,"set wlan %s balance successfully.\n",argv[0]);
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else if(ret == WID_DBUS_ERROR)
		vty_out(vty,"<error> operation fail .\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(wlan_roam_policy_cmd_func,
	  wlan_roam_policy_cmd,
	  "wlan l3 roaming policy (enable|disable)",  //fengwenchao comment "fast option" 20111215
	  "wlan roaming\n"
	  "policy of roaming\n" 
	  "enable or disable\n"

	 )
{
	int ret;
	unsigned char WlanID = 0;	
	unsigned char policy = 0;
	unsigned char state = 0;
	const char en[]="enable";
	const char di[]="disable";

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	
//	WlanID = (unsigned char)vty->index;

	/*if (!strcmp(argv[0],"fast")||(tolower(argv[0][0]) == 'f')){  //fengwenchao comment "fast option" 20111215
		policy=1;
	}
	else*/
/*	if (!strcmp(argv[0],"l3")||(tolower(argv[0][0]) == 'l')){ 
		policy=2;
	}*/
	//policy = 2;
	
	if (!strcmp(argv[0],"enable")||(tolower(argv[0][0]) == 'e')){
		state=1;
	}
	else if (!strcmp(argv[0],"disable")||(tolower(argv[0][0]) == 'd')){
		state=0;
	}
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (int)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (int)vty->index_sub;	
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (int)vty->index_sub;	
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_ROAMING_POLICY);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_ROAMING_POLICY);*/

	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_BYTE,&state,
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
		vty_out(vty,"set wlan roaming policy %s successfully.\n",(state==1)?en:di);
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else if(ret == WID_DBUS_ERROR)
		vty_out(vty,"<error> operation fail .\n");
	else if(ret == WLAN_BE_ENABLE)
		vty_out(vty,"<error> wlan should be disable first\n");
	else if(ret == RAOMING_DISABLE_FIRST)		
		vty_out(vty,"<error> roaming should be disable first\n");		
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	

	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(set_wlan_vlan_cmd_func,
	  set_wlan_vlan_cmd,
	  "set local wlan-vlan ID",
	  "wlan configuration\n"
	  "local mode\n"
	  "wlan vlan qos policy\n"
	  "vlan ID <1-4094>\n" 
	 )
{
	int ret = WID_DBUS_SUCCESS;;
	unsigned char WlanID = 0;
	unsigned int vlanid=0;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	
	//WlanID = (unsigned char)vty->index;
	
	ret = parse_int_ID((char*)argv[0], &vlanid);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown input\n");
		return CMD_SUCCESS;
	}	
	if(vlanid > 4094 || vlanid < 1){
		vty_out(vty,"<error> input parameter should be 1 to 4094\n");
		return CMD_SUCCESS;
	}
	
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (int)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (int)vty->index_sub;	
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_SET_WLAN_VLANID);
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_SET_WLAN_VLANID);*/

	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_UINT32,&vlanid,
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
		vty_out(vty,"set wlan-vlanid %s successfully.\n",argv[0]);
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else if(ret == IF_POLICY_CONFLICT)
		vty_out(vty,"<error> wlan is in other L3 interface\n");
	else if(ret == WLAN_BE_ENABLE)
		vty_out(vty,"<error>  wlan should be disable first\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	

	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

//mahz add 2011.5.25
DEFUN(set_nas_port_id_cmd_func,
	  set_nas_port_id_cmd,
	  "set wlan nas-port-id NAS_PORT_ID",
	  "wlan configuration\n"
	  "local mode\n"
	 )
{
	int ret = WID_DBUS_SUCCESS;
	int len = 0;
	char *nas_port_id = NULL;
	unsigned char WlanID = 0;
	int localid = 1;
	int slot_id = HostSlotId;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	
	len = strlen(argv[0]);
	if(len > 32){		
		vty_out(vty,"<error> nas-port-id name is too long,out of the limit of 32\n");
		return CMD_SUCCESS;
	}
	nas_port_id = (char*)malloc(strlen(argv[0])+1);
	memset(nas_port_id, 0, strlen(argv[0])+1);
	memcpy(nas_port_id, argv[0], strlen(argv[0]));		
	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (int)vty->index;	
	}
	else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (int)vty->index_sub;	
		localid = vty->local;   
		slot_id = vty->slotindex; 
	}
	else if (vty->node == LOCAL_HANSI_WLAN_NODE){
		index = vty->index;
		WlanID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);

	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_SET_NAS_PORT_ID);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_STRING,&nas_port_id,
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
		vty_out(vty,"set nas_port_id %s successfully.\n",argv[0]);
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else if(ret == WLAN_BE_ENABLE)
		vty_out(vty,"<error>  wlan should be disable first\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	free(nas_port_id);
	nas_port_id = NULL;
	
	return CMD_SUCCESS;
}
DEFUN(set_wlan_hotspot_id_cmd_func,
	  set_wlan_hotspot_id_cmd,
	  "set wlan hotspotid HOTSPOT_ID",
	  "wlan configuration\n"
	  "local mode\n"
	 )
{
	int ret = WID_DBUS_SUCCESS;
	unsigned int hotspotid = 0;
	unsigned char WlanID = 0;
	int localid = 1;
	int slot_id = HostSlotId;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	
	ret = parse_int_ID((char*)argv[0], &hotspotid);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(hotspotid > HOTSPOT_ID|| hotspotid == 0){
		vty_out(vty,"<error> hotspot id should be 1 to %d\n",HOTSPOT_ID);
		return CMD_SUCCESS;
	}
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (int)vty->index;	
	}
	else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (int)vty->index_sub;	
		localid = vty->local;   
		slot_id = vty->slotindex; 
	}
	else if (vty->node == LOCAL_HANSI_WLAN_NODE){
		index = vty->index;
		WlanID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_SET_WLAN_HOTSPOTID);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_UINT32,&hotspotid,
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
		vty_out(vty,"set hotspot_id %d successfully.\n",hotspotid);
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else if(ret == WLAN_BE_ENABLE)
		vty_out(vty,"<error>  wlan should be disable first\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
}
DEFUN(clean_wlan_hotspot_id_cmd_func,
	  clean_wlan_hotspot_id_cmd,
	  "clean wlan hotspotid",
	  "wlan configuration\n"
	  "local mode\n"
	 )
{
	int ret = WID_DBUS_SUCCESS;
	unsigned int hotspotid = 0;
	unsigned char WlanID = 0;
	int localid = 1;
	int slot_id = HostSlotId;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	
	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (int)vty->index;	
	}
	else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (int)vty->index_sub;	
		localid = vty->local;   
		slot_id = vty->slotindex; 
	}
	else if (vty->node == LOCAL_HANSI_WLAN_NODE){
		index = vty->index;
		WlanID = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_CLEAN_WLAN_HOTSPOTID);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
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
		vty_out(vty,"clean wlan hotspotid successfully.\n");
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else if(ret == WLAN_BE_ENABLE)
		vty_out(vty,"<error>  wlan should be disable first\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
	
	return CMD_SUCCESS;
}


DEFUN(set_wlan_vlan_priority_cmd_func,
	  set_wlan_vlan_priority_cmd,
	  "set wlan-1p priority VALUE",
	  "wlan configuration\n"
	  "wlan 802.11p policy\n"
	  "qos priority\n"
	  "priority value <0-7>\n"
	 )
{
	int ret = WID_DBUS_SUCCESS;;
	unsigned char WlanID = 0;
	unsigned int priority = 0;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	
//	WlanID = (unsigned char)vty->index;
	
	ret = parse_int_ID((char*)argv[0], &priority);
	if(ret != WID_DBUS_SUCCESS)
	{
		vty_out(vty,"<error> unknown input\n");
		return CMD_SUCCESS;
	}	
	if(priority > 7)
	{
		vty_out(vty,"<error> input parameter should be 0 to 7\n");
		return CMD_SUCCESS;
	}


	
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (int)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (int)vty->index_sub;	
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (int)vty->index_sub;	
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_SET_WLAN_VLAN_PRIORITY);
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_SET_WLAN_VLAN_PRIORITY);*/

	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_UINT32,&priority,
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
		vty_out(vty,"set wlan-vlanid %s successfully.\n",argv[0]);
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else if(ret == IF_POLICY_CONFLICT)
		vty_out(vty,"<error> wlan is in other L3 interface\n");
	else if(ret == WLAN_NOT_BINDING_VLAN)
		vty_out(vty,"<error> wlan has not binding vlan\n");
	else if(ret == WLAN_BE_ENABLE)
		vty_out(vty,"<error>  wlan should be disable first\n");
	else if(ret == SYSTEM_CMD_ERROR)
		vty_out(vty,"<error>  wlan is under tunnel wlan-vlan policy\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	

	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(undo_wlan_vlan_cmd_func,
	  undo_wlan_vlan_cmd,
	  "undo local wlan-vlan binding",
	  "undo wlan configuration\n"
	  "local mode\n"
	  "wlan vlan qos policy\n"
	  "wlan vlan qos policy\n" 
	 )
{
	int ret = WID_DBUS_SUCCESS;;
	unsigned char WlanID = 0;
	unsigned int vlanid = 0;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	
	//WlanID = (unsigned char)vty->index;
	
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (int)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (int)vty->index_sub;	
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (int)vty->index_sub;	
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_UNDO_WLAN_VLANID);
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_UNDO_WLAN_VLANID);*/

	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_UINT32,&vlanid,
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
		vty_out(vty,"undo wlan-vlanid successfully\n");
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else if(ret == IF_POLICY_CONFLICT)
		vty_out(vty,"<error> wlan is in other L3 interface\n");
	else if(ret == WLAN_BE_ENABLE)
		vty_out(vty,"<error>  wlan should be disable first\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	

	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(show_wlan_vlan_info_cmd_func,
	  show_wlan_vlan_info_cmd,
	  "show wlan_vlan info",
	  "wlan configuration\n"
	  "wlan vlan qos policy\n"
	  "wlan vlan qos policy infomation\n"
	 )
{
	int ret = WID_DBUS_SUCCESS;;
	unsigned char WlanID = 0;
	unsigned int vlanid = 0;
	unsigned char priority = 0;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	
	//WlanID = (unsigned char)vty->index;
	
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (int)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (int)vty->index_sub;	
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (int)vty->index_sub;	
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
#if 0	
	ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath(index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_SHOW_WLAN_VLAN_INFO);
	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_SHOW_WLAN_VLAN_INFO);*/

	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
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
		dbus_message_iter_get_basic(&iter,&vlanid);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&priority);
	}
	if(ret == 0)
	{
		vty_out(vty,"wlan	%d\n",WlanID);
		if(vlanid == 0)
		{
			vty_out(vty,"vlan	not binded\n");
		}
		else
		{
			vty_out(vty,"vlan	%d\n",vlanid);
		}
		vty_out(vty,"priority	%d\n",priority);
	}
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else
		vty_out(vty,"<error>  %d\n",ret);
	

	
	dbus_message_unref(reply);
#endif
	DCLI_WLAN_API_GROUP * WLANINFO = NULL;
	WLANINFO =  dcli_wlan_show_api_group(
		index,
		WlanID,
		localid,        //fengwenchao add 20110507
		&ret,
		0,
		dcli_dbus_connection,
		WID_DBUS_WLAN_METHOD_SHOW_WLAN_VLAN_INFO
		);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0 ){
		vty_out(vty,"wlan	%d\n",WlanID);
		if(WLANINFO->WLAN[0]->vlanid == 0)
		{
			vty_out(vty,"vlan	not binded\n");
		}
		else
		{
			vty_out(vty,"vlan	%d\n",WLANINFO->WLAN[0]->vlanid);/*WLAN[0]->vlanid*/
		}
		vty_out(vty,"priority	%d\n",WLANINFO->WLAN[0]->wlan_1p_priority);/*WLAN[0]->wlan_1p_priority*/
		dcli_wlan_free_fun(WID_DBUS_WLAN_METHOD_SHOW_WLAN_VLAN_INFO,WLANINFO);
	}
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	return CMD_SUCCESS;
}
DEFUN(wlan_set_bridge_isolation_func,
	  wlan_set_bridge_isolation_cmd,
	  "set bridge_isolation (enable|disable)",
	  CONFIG_STR
	  "wlan bridge isolation\n"
	  "wlan bridge isolation\n" 

	 )
{
	int ret;
	unsigned char WlanID = 0;
	unsigned char state=0;
	const char en[]="enable";
	const char di[]="disable";

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	
	//WlanID = (unsigned char)vty->index;
	
	if (!strcmp(argv[0],"enable")){
		state=1;
	}
	else if (!strcmp(argv[0],"disable")){
		state=0;
	}
	else
	{
		vty_out(vty,"<error> input parameter should only be 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (int)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (int)vty->index_sub;	
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (int)vty->index_sub;	
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_SET_BRIDGE_ISOLATION);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_SET_BRIDGE_ISOLATION);*/

	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_BYTE,&state,
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
		vty_out(vty,"set wlan bridge isolation %s successfully.\n",(state==1)?en:di);
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else if(ret == IF_POLICY_CONFLICT)
		vty_out(vty,"<error> wlan is not wlan if policy\n");
	else if(ret == WLAN_BE_ENABLE)
		vty_out(vty,"<error> wlan should be disable first\n");		
	else if(ret == WLAN_CREATE_BR_FAIL)
		vty_out(vty,"<error> wlan bridge error\n");	
	else if(ret == SYSTEM_CMD_ERROR)
		vty_out(vty,"<error> system cmd process error\n");
	else if(ret == ISOLATION_CONFLICT)
		vty_out(vty,"<error> sameportswitch and isolation are conflict,disable sameportswitch first\n");	
	else if (ret == WLAN_APPLY_SECURITY_FIRST)
	{
		vty_out(vty, "<error> apply security in this wlan first\n");
	}
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	

	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(wlan_set_bridge_multicast_isolation_func,
	  wlan_set_bridge_multicast_isolation_cmd,
	  "set bridge_multicast_isolation (enable|disable)",
	  CONFIG_STR
	  "wlan bridge multicast isolation\n"
	  "wlan bridge multicast isolation\n" 

	 )
{
	int ret;
	unsigned char WlanID = 0;
	unsigned char state=0;
	const char en[]="enable";
	const char di[]="disable";

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	
//	WlanID = (unsigned char)vty->index;
	
	if (!strcmp(argv[0],"enable")){
		state=1;
	}
	else if (!strcmp(argv[0],"disable")){
		state=0;
	}
	else
	{
		vty_out(vty,"<error> input parameter should only be 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (int)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (int)vty->index_sub;	
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (int)vty->index_sub;	
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_SET_BRIDGE_MULTICAST_ISOLATION);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_SET_BRIDGE_MULTICAST_ISOLATION);*/

	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_BYTE,&state,
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
		vty_out(vty,"set wlan bridge multicast isolation %s successfully.\n",(state==1)?en:di);
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else if(ret == IF_POLICY_CONFLICT)
		vty_out(vty,"<error> wlan is not wlan if policy\n");
	else if(ret == WLAN_BE_ENABLE)
		vty_out(vty,"<error> wlan should be disable first\n");		
	else if(ret == WLAN_CREATE_BR_FAIL)
		vty_out(vty,"<error> wlan bridge error\n");	
	else if(ret == SYSTEM_CMD_ERROR)
		vty_out(vty,"<error> system cmd process error\n");	
	else if(ret == ISOLATION_CONFLICT)
		vty_out(vty,"<error> sameportswitch and isolation are conflict,disable sameportswitch first\n");	
	else if (ret == WLAN_APPLY_SECURITY_FIRST)
	{
		vty_out(vty, "<error> apply security in this wlan first\n");
	}	
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	

	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(wlan_set_sameportswitch_func,
	  wlan_set_sameportswitch_cmd,
	  "set bridge sameportswitch (enable|disable)",
	  CONFIG_STR
	  "wlan bridge sameportswitch\n"
	  "wlan bridge sameportswitch\n" 
	 )
{
	int ret;
	unsigned char WlanID = 0;
	unsigned char state=0;

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	
	//WlanID = (unsigned char)vty->index;
	
	if (!strcmp(argv[0],"enable")){
		state=1;
	}
	else if (!strcmp(argv[0],"disable")){
		state=0;
	}
	else
	{
		vty_out(vty,"<error> input parameter should only be 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (int)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (int)vty->index_sub;	
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (int)vty->index_sub;	
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_SET_BRIDGE_SAMEPORTSWICTH);
/*
	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_SET_BRIDGE_SAMEPORTSWICTH);*/

	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_BYTE,&state,
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
		vty_out(vty,"set wlan bridge sameportswitch %s successfully.\n",argv[0]);
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else if(ret == IF_POLICY_CONFLICT)
		vty_out(vty,"<error> wlan is not wlan if policy\n");
	else if(ret == WLAN_BE_ENABLE)
		vty_out(vty,"<error> wlan should be disable first\n");		
	else if(ret == WLAN_CREATE_BR_FAIL)
		vty_out(vty,"<error> wlan bridge error\n");	
	else if(ret == SYSTEM_CMD_ERROR)
		vty_out(vty,"<error> system cmd process error\n");
	else if(ret == ISOLATION_CONFLICT)
		vty_out(vty,"<error> sameportswitch and isolation are conflict,disable isolation first\n");	
	else if (ret == WLAN_APPLY_SECURITY_FIRST)
	{
		vty_out(vty, "<error> apply security in this wlan first\n");
	}
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	

	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(wlan_show_bridge_isolation_func,
	  wlan_show_bridge_isolation_cmd,
	  "show bridge_isolation infomation",
	  CONFIG_STR
	  "wlan bridge multicast isolation infomation\n"
	  "wlan bridge multicast isolation infomation\n" 

	 )
{
	int ret;
	unsigned char WlanID = 0;
	unsigned char isolation_policy = 0;
	unsigned char multicast_isolation_policy = 0;
	unsigned char sameportswitch = 0;
	const char en[]="enable";
	const char dis[]="disable";

	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;

	dbus_error_init(&err);
	
	//WlanID = (unsigned char)vty->index;
	
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (int)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (int)vty->index_sub;	
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (int)vty->index_sub;	
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
#if 0	
	ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath(index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_SHOW_BRIDGE_ISOLATION);

/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_SHOW_BRIDGE_ISOLATION);*/

	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
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
		dbus_message_iter_get_basic(&iter,&isolation_policy);

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&multicast_isolation_policy);

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&sameportswitch);
	}
	
	if(ret == 0)
	{
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"wlan %d bridge isolation infomation\n",WlanID);
		vty_out(vty,"bridge isolation:	%s\n",(isolation_policy == 0)?dis:en);
		vty_out(vty,"bridge multicast isolation:	%s\n",(multicast_isolation_policy == 0)?dis:en);
		vty_out(vty,"bridge sameportswitch:	%s\n",(sameportswitch == 0)?dis:en);
		vty_out(vty,"==============================================================================\n");
	
	}
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");	
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
#endif
	DCLI_WLAN_API_GROUP * WLANINFO = NULL;
	WLANINFO = (DCLI_WLAN_API_GROUP *) dcli_wlan_show_api_group(
		index,
		WlanID,
		localid,        //fengwenchao add 20110507
		&ret,
		0,
		dcli_dbus_connection,
		WID_DBUS_WLAN_METHOD_SHOW_BRIDGE_ISOLATION
		);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0 ){
		vty_out(vty,"==============================================================================\n");
		vty_out(vty,"wlan %d bridge isolation infomation\n",WlanID);
		vty_out(vty,"bridge isolation:	%s\n",(WLANINFO->WLAN[0]->isolation_policy == 0)?dis:en);
		vty_out(vty,"bridge multicast isolation:	%s\n",(WLANINFO->WLAN[0]->multicast_isolation_policy == 0)?dis:en);
		vty_out(vty,"bridge sameportswitch:	%s\n",(WLANINFO->WLAN[0]->sameportswitch == 0)?dis:en);
		vty_out(vty,"==============================================================================\n");
	
		dcli_wlan_free_fun(WID_DBUS_WLAN_METHOD_SHOW_BRIDGE_ISOLATION,WLANINFO);
	}
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");	
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	return CMD_SUCCESS;
}
DEFUN(set_tunnel_wlan_vlan_cmd_func,
	  set_tunnel_wlan_vlan_cmd,
	  "set tunnel wlan-vlan (add|delete) interface IFNAME",
	  "wlan configuration\n"
	  "tunnel mode\n"
	  "wlan vlan policy\n"
	  "add if to br / remove if from br\n"
	  "add/delete interface"
	  "IFNAME like radio1-0.1\n" 
	 )
{
	int ret = WID_DBUS_SUCCESS;;
	unsigned char WlanID = 0;
	char *name;
	int state = 0;
	int lenth = 0;
	char *id = 0;
	int l_wtpid = 0;
	int l_radioid = 0;
	int l_wlanid = 0;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	dbus_error_init(&err);
	
	//WlanID = (unsigned char)vty->index;
	if(vty->node == WLAN_NODE){
		WlanID = (int)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		WlanID = (int)vty->index_sub;	
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		WlanID = (int)vty->index_sub;	
	}
	/*fengwenchao add end*/	

	if (!strcmp(argv[0],"add")){
		state = 1;
	}
	else if (!strcmp(argv[0],"delete")){
		state = 0;
	}
	else
	{
		vty_out(vty,"<error> input parameter should only be 'add' or 'delete'\n");
		return CMD_SUCCESS;
	}
	if (strncasecmp(argv[1],"radio",5))
	{
		vty_out(vty,"<error> input interface name should only start with 'radio',other interface you should use ebr configuration\n");
		return CMD_SUCCESS;
	}
	else
	{
		lenth = strlen((char *)argv[1]);

		if(lenth > 15)
		{		
			vty_out(vty,"<error> if name too long\n");
			return CMD_SUCCESS;
		}
		
		name = (char *)malloc(sizeof(char)*20);
		if(name == NULL)
		{
			return MALLOC_ERROR;
		}
		memset(name,0,20);
		memcpy(name,argv[1],strlen(argv[1]));

		char *id = (char *)malloc(sizeof(char)*25);
		memset(id,0,25);
		if(id == NULL)
		{
			if(name)
			{
				free(name);
				name = NULL;
			}
			return MALLOC_ERROR;
		}
		memcpy(id,name+5,(lenth-5));
		ret = parse_radio_ifname(id,&l_wtpid,&l_radioid,&l_wlanid);
		/*printf("wtpid:%d,radioid:%d,wlanid:%d\n",l_wtpid,l_radioid,l_wlanid);*/
		if (ret != WID_DBUS_SUCCESS) 
		{
			if(id);
			{
				free(id);
				id = NULL;
			}
			if(name);
			{
				free(name);
				name = NULL;
			}
			return CMD_WARNING;
		}
		
		if (l_wlanid != WlanID)
		{
			vty_out(vty,"<error> input interface name with wlanid %d not the same with wlan id %d\n",l_wlanid,WlanID);
			if(id);
			{
				free(id);
				id = NULL;
			}
			if(name);
			{
				free(name);
				name = NULL;
			}
			return CMD_WARNING;
		}

	}
	
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (int)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (int)vty->index_sub;			
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (int)vty->index_sub;		
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	memset(name,0,20);
	sprintf(name,"radio%d-%d-%d.%d",index,l_wtpid,l_radioid,l_wlanid);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_SET_TUNNEL_WLAN_VLAN);
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_SET_TUNNEL_WLAN_VLAN);*/

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_UINT32,&state,
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
		if(name)
		{
			free(name);
			name = NULL;
		}
		if(id)
		{
			free(id);
			id = NULL;
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);


	if(ret == 0)
		vty_out(vty,"set tunnel wlan vlan %s %s successfully.\n",argv[0],argv[1]);
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else if(ret == IF_POLICY_CONFLICT)
		vty_out(vty,"<error> wlan is in local wlan-vlan interface\n");
	else if(ret == WLAN_BE_ENABLE)
		vty_out(vty,"<error>  wlan should be disable first\n");
	else if(ret == APPLY_IF_FAIL)
		vty_out(vty,"<error>  input ifname is wrong\n");
	else if(ret == WLAN_CREATE_BR_FAIL)
		vty_out(vty,"<error>  wlan is not in tunnel mode\n");
	else if(ret == SYSTEM_CMD_ERROR)
		vty_out(vty,"<error>  if is already %s,or system cmd error\n",argv[0]);
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	if(name)
	{
		free(name);
		name = NULL;
	}
	if(id)
	{
		free(id);
		id = NULL;
	}
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(show_tunnel_wlan_vlan_cmd_func,
	  show_tunnel_wlan_vlan_cmd,
	  "show tunnel wlan-vlan infomation",
	  "show wlan configuration\n"
	  "tunnel mode\n"
	  "wlan vlan policy\n"
	 )
{
	int ret = WID_DBUS_SUCCESS;;
	unsigned char WlanID = 0;
	unsigned int num = 0;
	int i = 0;
	char *name;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	dbus_error_init(&err);
	
	//WlanID = (unsigned char)vty->index;

	
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (int)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (int)vty->index_sub;	
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (int)vty->index_sub;	
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
#if 0	
	ReInitDbusPath(index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath(index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath(index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_SHOW_TUNNEL_WLAN_VLAN);
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_SHOW_TUNNEL_WLAN_VLAN);*/

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
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
		
		/*printf("num %d\n",num);*/
		vty_out(vty,"Wlan %d tunnel wlan-vlan infomation\n",WlanID);
		vty_out(vty,"==============================================================================\n");
		if(num != 0)
		{
			for(i=0;i<num;i++)
			{
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&name);
				/*printf("name %s\n",name);*/
				vty_out(vty,"tunnel wlan-vlan interface: %s\n",name);
			}
		}	
	}
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else if(ret == IF_POLICY_CONFLICT)
		vty_out(vty,"<error> wlan is in local wlan-vlan interface\n");
	else if(ret == WLAN_BE_ENABLE)
		vty_out(vty,"<error>  wlan should be disable first\n");
	else if(ret == APPLY_IF_FAIL)
		vty_out(vty,"<error>  input ifname is wrong\n");
	else if(ret == WLAN_CREATE_BR_FAIL)
		vty_out(vty,"<error>  wlan is not in tunnel mode\n");
	else if(ret == BSS_L3_INTERFACE_ADD_BR_FAIL)
		vty_out(vty,"<error>  add if %s to br fail\n",argv[0]);
	else if(ret == BSS_L3_INTERFACE_DEL_BR_FAIL)
		vty_out(vty,"<error>  remove if %s from br fail\n",argv[0]);
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	dbus_message_unref(reply);
#endif
	DCLI_WLAN_API_GROUP * WLANINFO = NULL;
	WLANINFO = (DCLI_WLAN_API_GROUP *) dcli_wlan_show_api_group(
		index,
		WlanID,
		localid,                //fengwenchao add 20110507
		&ret,
		0,
		dcli_dbus_connection,
		WID_DBUS_WLAN_METHOD_SHOW_TUNNEL_WLAN_VLAN
		);
	if(ret == -1){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == 0 ){
		vty_out(vty,"Wlan %d tunnel wlan-vlan infomation\n",WlanID);
		vty_out(vty,"==============================================================================\n");
		
		if((WLANINFO)&&(WLANINFO->WLAN)&&(WLANINFO->WLAN[0]->tunnel_wlan_vlan)){
			//vty_out(vty,"111111111111=========================================\n");
			struct WID_TUNNEL_WLAN_VLAN *head = NULL;
			head = WLANINFO->WLAN[0]->tunnel_wlan_vlan;
			{
				//vty_out(vty,"2222222222222=========================================\n");
				while(head)
				{
					//vty_out(vty,"3333333333333=========================================\n");
					vty_out(vty,"tunnel wlan-vlan interface: %s\n",head->ifname);
					head = head->ifnext;
				}
			}	
		}
		dcli_wlan_free_fun(WID_DBUS_WLAN_METHOD_SHOW_TUNNEL_WLAN_VLAN,WLANINFO);
		vty_out(vty,"==============================================================================\n");
	}
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else if(ret == IF_POLICY_CONFLICT)
		vty_out(vty,"<error> wlan is in local wlan-vlan interface\n");
	else if(ret == WLAN_BE_ENABLE)
		vty_out(vty,"<error>  wlan should be disable first\n");
	else if(ret == APPLY_IF_FAIL)
		vty_out(vty,"<error>  input ifname is wrong\n");
	else if(ret == WLAN_CREATE_BR_FAIL)
		vty_out(vty,"<error>  wlan is not in tunnel mode\n");
	else if(ret == BSS_L3_INTERFACE_ADD_BR_FAIL)
		vty_out(vty,"<error>  add if %s to br fail\n",argv[0]);
	else if(ret == BSS_L3_INTERFACE_DEL_BR_FAIL)
		vty_out(vty,"<error>  remove if %s from br fail\n",argv[0]);
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	return CMD_SUCCESS;
}
DEFUN(set_wlan_essid_func,
	  set_wlan_essid_cmd,
	  "set wlan essid .ESSID", /*wcl modify*/
	  CONFIG_STR
	  "wlan configuration\n" 
	  "wlan essid\n"
	  "wlan essid\n"
	 )
{
	int ret;
	unsigned char WlanID = 0;
	char *essid;
	int len = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	//WlanID = (unsigned char)vty->index;
	
	/*len = strlen(argv[0]);
	if(len > 32){		
		vty_out(vty,"<error> essid is too long,out of the limit of 32\n");
		return CMD_SUCCESS;
	}
	essid = (char*)malloc(strlen(argv[0])+1);
	memset(essid, 0, strlen(argv[0])+1);
	memcpy(essid, argv[0], strlen(argv[0]));	
	*/ /*wcl modify*/
		
	/*wcl add*/
	essid = WID_parse_CMD_str(&argv[0],argc,NULL,0);
			if(essid == NULL){		
				vty_out(vty,"UNKNOWN COMMAND\n");
				vty_out(vty,"COMMAND should be :set wlan essid as sd\n");
				return CMD_SUCCESS;
			}
			len = strlen(essid);
			if(len > 32){		
				vty_out(vty,"<error> essid is too long,out of the limit of 32\n");
				if(essid){
					free(essid);
					essid = NULL;
				}
				return CMD_SUCCESS;
			}
			if(-1 == ssid_illegal_character_check(essid,len)){
				vty_out(vty,"essid is null!or checkout the parameter len!\n");
				if(essid){
					free(essid);
					essid = NULL;
				}
				return CMD_SUCCESS;
			}
			else if(-2==ssid_illegal_character_check(essid,len)){
				vty_out(vty,"illegal essid name!! ` \ \" & * ( ) not supported!\n");
				if(essid){
					free(essid);
					essid = NULL;
				}
				return CMD_SUCCESS;
			}
			

	/*end*/

	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (unsigned char)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (unsigned char)vty->index_sub;	
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (unsigned char)vty->index_sub;	
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_SET_ESSID);
/*	query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_SET_ESSID);*/
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_STRING,&essid,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if(essid)
	{
		free(essid);
		essid = NULL;
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

	if(ret == 0)
	{
		vty_out(vty,"set wlan %d essid %s successfully.\n",WlanID,argv[0]);
	}	
	else if(ret == WLAN_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> wlan id does not exist\n");
	}
	else if(ret == WLAN_BE_ENABLE)
	{
		vty_out(vty,"<error> wlan is enable,please disable it first\n");
	}
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return CMD_SUCCESS;


}

//fengwenchao add 20110307 for autelan-2194
DEFUN(set_wlan_ascii_essid_func,
	  set_wlan_ascii_essid_cmd,
	  "set wlan_ascii essid ESSID",
	  CONFIG_STR
	  "wlan configuration\n" 
	  "wlan essid\n"
	  "wlan essid\n"
	 )
{
	
	int ret;
	unsigned char WlanID = 0;
	unsigned char *ESSID;
	int len = 0;
	int i = 0;
	//unsigned char a[2] = {0,0};
	unsigned char *a = NULL;
	a = (unsigned char*)malloc(3);
	//unsigned char isAdd = 1;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
		
	len = strlen(argv[0]);
	if(len > 64)
		{		
			vty_out(vty,"<error> essid is too long,out of the limit of 64\n");
			if(a){
				free(a);
				a = NULL;
			}
			return CMD_SUCCESS;
		}
	//printf("len:%d\n",len);
	//printf("argv[0]:%s\n",argv[0]);
	//printf("argv[0]+2:%s\n",argv[0]+2);
	unsigned long ul;
	ESSID = (unsigned char*)malloc(ESSID_DEFAULT_LEN);
	memset(ESSID, 0, ESSID_DEFAULT_LEN);
	for(i = 0;i < len;i=i+2)
	{
		memset(a,0,3);
		//printf("argv[0]+%d=%s\n",i,argv[0]+i);
		memcpy(a,argv[0]+i,2);
		//printf("a = %s \n",a);
		ul = HexToDec(a);
		ESSID[i/2] = ul;
		//printf("u1 = %d \n",ul);
		memset(a,0,3);
	}

	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (unsigned char)vty->index; 
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index; 		
		WlanID = (unsigned char)vty->index_sub; 
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (unsigned char)vty->index_sub; 
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_SET_ASCII_ESSID);
																	
	dbus_error_init(&err);
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_BYTE,&ESSID[0],
							 DBUS_TYPE_BYTE,&ESSID[1],
							 DBUS_TYPE_BYTE,&ESSID[2],
							 DBUS_TYPE_BYTE,&ESSID[3],
							 DBUS_TYPE_BYTE,&ESSID[4],
							
							 DBUS_TYPE_BYTE,&ESSID[5],
							 DBUS_TYPE_BYTE,&ESSID[6],
							 DBUS_TYPE_BYTE,&ESSID[7],
							 DBUS_TYPE_BYTE,&ESSID[8],
							 DBUS_TYPE_BYTE,&ESSID[9],
							
							 DBUS_TYPE_BYTE,&ESSID[10],
							 DBUS_TYPE_BYTE,&ESSID[11],
							 DBUS_TYPE_BYTE,&ESSID[12],
							 DBUS_TYPE_BYTE,&ESSID[13],
							 DBUS_TYPE_BYTE,&ESSID[14],
							
							 DBUS_TYPE_BYTE,&ESSID[15],
							 DBUS_TYPE_BYTE,&ESSID[16],
							 DBUS_TYPE_BYTE,&ESSID[17],
							 DBUS_TYPE_BYTE,&ESSID[18],
							 DBUS_TYPE_BYTE,&ESSID[19],
							
							 DBUS_TYPE_BYTE,&ESSID[20],
							 DBUS_TYPE_BYTE,&ESSID[21],
							 DBUS_TYPE_BYTE,&ESSID[22],
							 DBUS_TYPE_BYTE,&ESSID[23],
							 DBUS_TYPE_BYTE,&ESSID[24],
							 
							 DBUS_TYPE_BYTE,&ESSID[25],
							 DBUS_TYPE_BYTE,&ESSID[26],
							 DBUS_TYPE_BYTE,&ESSID[27],
							 DBUS_TYPE_BYTE,&ESSID[28],
							 DBUS_TYPE_BYTE,&ESSID[29],
							 
							 DBUS_TYPE_BYTE,&ESSID[30],
							 DBUS_TYPE_BYTE,&ESSID[31],
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	if(ESSID)
	{
		free(ESSID);
		ESSID = NULL;
	}
	if(a)
	{
		free(a);
		a = NULL;	
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

	if(ret == 0)
	{
		vty_out(vty,"set wlan %d ascii essid %s successfully.\n",WlanID,argv[0]);
	}	
	else if(ret == WLAN_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> wlan id does not exist\n");
	}
	else if(ret == WLAN_BE_ENABLE)
	{
		vty_out(vty,"<error> wlan is enable,please disable it first\n");
	}
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return CMD_SUCCESS;
	
}
//fengwenchao add end

/*nl add 20100318*/
DEFUN(set_whole_wlan_traffic_limit_func,
	  set_whole_wlan_traffic_limit_cmd,
	  "set wlan traffic limit value VALUE",
	  CONFIG_STR
	  "wlan configuration\n" 
	  "wlan traffic limit value\n"
	  "wlan traffic limit value\n"
	 )
{
	int ret;
	unsigned char WlanID = 0;
	unsigned int traffic_value;
	int len = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	ret = parse_int_ID((char *)argv[0],&traffic_value);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	} 
	/*copy from 1.2omc for autelan-2140*/
	if(traffic_value<0 || traffic_value> 300000)
	{
		vty_out(vty,"<error> input parameter should be 0~300000\n");
		return CMD_SUCCESS;
	}	
	/*copy end*/	
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (unsigned char)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (unsigned char)vty->index_sub;	
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (unsigned char)vty->index_sub;	
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_WHOLE_WLAN_TRAFFIC_LIMIT_VALUE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_UINT32,&traffic_value,
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
		vty_out(vty,"set wlan %d traffic limit value %d successfully.\n",WlanID,traffic_value);
	}	
	else if(ret == WLAN_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> wlan id does not exist\n");
	}
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return CMD_SUCCESS;

}
/*nl add 20100318*/
DEFUN(set_whole_wlan_send_traffic_limit_func,
	  set_whole_wlan_send_traffic_limit_cmd,
	  "set wlan send traffic limit value VALUE",
	  CONFIG_STR
	  "wlan configuration\n" 
	  "wlan send traffic limit value\n"
	  "wlan send traffic limit value\n"
	 )
{
	int ret;
	unsigned char WlanID = 0;
	unsigned int send_traffic_value;
	int len = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	ret = parse_int_ID((char *)argv[0],&send_traffic_value);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	} 
	/*fengwenchao add for AXSSZFI-22*/
	if(send_traffic_value<0 || send_traffic_value> 300000)
	{
		vty_out(vty,"<error> input parameter should be 0~300000\n");
		return CMD_SUCCESS;
	}
	/*fengwenchao add end*/
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (unsigned char)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (unsigned char)vty->index_sub;
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (unsigned char)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_WHOLE_WLAN_SEND_TRAFFIC_LIMIT_VALUE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_UINT32,&send_traffic_value,
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
		vty_out(vty,"set wlan %d send traffic limit value %d successfully.\n",WlanID,send_traffic_value);
	}	
	else if(ret == WLAN_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> wlan id does not exist\n");
	}
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return CMD_SUCCESS;

}

/*nl add 20100401*/
DEFUN(set_whole_wlan_station_average_traffic_limit_func,
	  set_whole_wlan_station_average_traffic_limit_cmd,
	  "set wlan traffic limit station average value VALUE",
	  CONFIG_STR
	  "wlan configuration\n" 
	  "wlan traffic limit station average value\n"
	  "wlan traffic limit station average value\n"
	 )
{
	int ret;
	unsigned char WlanID = 0;
	unsigned int station_average_traffic_value;
	int len = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	ret = parse_int_ID((char *)argv[0],&station_average_traffic_value);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	} 
	/*fengwenchao add for AXSSZFI-22*/
	if(station_average_traffic_value<0 || station_average_traffic_value> 300000)
	{
		vty_out(vty,"<error> input parameter should be 0~300000\n");
		return CMD_SUCCESS;
	}
	/*fengwenchao add end*/

	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (unsigned char)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (unsigned char)vty->index_sub;	
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (unsigned char)vty->index_sub;	
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_WHOLE_WLAN_STA_AVERAGE_TRAFFIC_LIMIT_VALUE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_UINT32,&station_average_traffic_value,
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
		vty_out(vty,"set wlan %d station average traffic limit value %d successfully.\n",WlanID,station_average_traffic_value);
	}	
	else if(ret == WLAN_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> wlan id does not exist\n");
	}
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return CMD_SUCCESS;

}
/*nl add 20100401*/
DEFUN(set_whole_wlan_station_average_send_traffic_limit_func,
	  set_whole_wlan_station_average_send_traffic_limit_cmd,
	  "set wlan traffic limit station send average value VALUE",
	  CONFIG_STR
	  "wlan configuration\n" 
	  "wlan send traffic limit station average value\n"
	  "wlan send traffic limit station average value\n"
	 )
{
	int ret;
	unsigned char WlanID = 0;
	unsigned int station_average_send_traffic_value;
	int len = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;

	ret = parse_int_ID((char *)argv[0],&station_average_send_traffic_value);
	
	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	} 
	/*fengwenchao add for AXSSZFI-22*/
	if(station_average_send_traffic_value<0 || station_average_send_traffic_value> 300000)
	{
		vty_out(vty,"<error> input parameter should be 0~300000\n");
		return CMD_SUCCESS;
	}
	/*fengwenchao add end*/	
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (unsigned char)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (unsigned char)vty->index_sub;
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (unsigned char)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_WHOLE_WLAN_STA_AVERAGE_SEND_TRAFFIC_LIMIT_VALUE);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
							 DBUS_TYPE_UINT32,&station_average_send_traffic_value,
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
		vty_out(vty,"set wlan %d station average send traffic limit value %d successfully.\n",WlanID,station_average_send_traffic_value);
	}	
	else if(ret == WLAN_ID_NOT_EXIST)
	{
		vty_out(vty,"<error> wlan id does not exist\n");
	}
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	return CMD_SUCCESS;

}


DEFUN(set_wlan_sta_ip_mac_binding_func,
		  set_wlan_sta_ip_mac_binding_cmd,
		  "set wlan sta_ip_mac binding (enable|disable)",
		  "sta ip mac binding config\n"
		  "sta ip mac binding enable|disable\n"
	 )
{	
	int ret;
	unsigned char wlanid = 0;
	unsigned int i = 0;
    int policy = 0;
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	
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

	if(vty->node == WLAN_NODE){
		index = 0;			
		wlanid = (int)vty->index;
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index; 		
		wlanid = (int)vty->index_sub;
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		wlanid = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ret = dcli_wlan_sta_ip_mac_binding(index,localid,policy,wlanid,dcli_dbus_connection);
	if(ret == 0)
		vty_out(vty,"set wtp sta info report %s base successfully\n",argv[0]);
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	return CMD_SUCCESS; 

}

DEFUN(set_wlan_servive_timer_func,
		  set_wlan_servive_timer_func_cmd,
		  "set wlan (start|stop) service at TIME (once|cycle) .WEEKDAYS",
		  "set wlan service control timer\n"
		  "set wlan service control timer\n"
		  "set wlan start service timer\n"
		  "set wlan stop service timer\n"
		  "start service or stop service\n"
		  "time like 21:12:32 \n"
		  "time like 21:12:32 \n"
		  "once or cycle\n"
		  "once or cycle\n"
		  "weekdays you want (like Sun Mon Tue Wed Thu Fri Sat or hebdomad)\n"
		  "eg: set wlan start service at 21:12:34 once mon tue sat"
	 )
{	
	int ret;
	unsigned char wlanid = 0;
	unsigned int i = 0;
    int policy = 0;	
    int is_once = 0;
	int time;
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	int num = 0;
	int wday = 0;
	if (!strcmp(argv[0],"start"))
	{
		policy = 1;	
	}
	else if (!strcmp(argv[0],"stop"))
	{
		policy = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'start' or 'stop'\n");
		return CMD_SUCCESS;
	}
	time = Check_Time_Format(argv[1]);
	if(time == -1){
		vty_out(vty,"<error> input patameter format should be 12:32:56\n");
		return CMD_SUCCESS;
	}

	if (!strcmp(argv[2],"once"))
	{
		is_once = 1;	
	}
	else if (!strcmp(argv[2],"cycle"))
	{
		is_once = 0;	
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'once' or 'cycle'\n");
		return CMD_SUCCESS;
	}
	num = argc - 3;
	if(num <= 0){
		vty_out(vty,"<error> weekdays you want (like Sun Mon Tue Wed Thu Fri Sat or hebdomad)\n");
		return CMD_SUCCESS;		
	}
	for(i = 3; i < argc; i++){
		str2lower(&(argv[i]));
		if(strcmp(argv[i],"mon") == 0){
			wday |= 0x01; 
		}
		else if(strcmp(argv[i],"tue") == 0){
			wday |= 0x02;

		}
		else if(strcmp(argv[i],"wed") == 0){
			wday |= 0x04;

		}
		else if(strcmp(argv[i],"thu") == 0){
			wday |= 0x08;

		}
		else if(strcmp(argv[i],"fri") == 0){
			wday |= 0x10;

		}
		else if(strcmp(argv[i],"sat") == 0){
			wday |= 0x20;

		}
		else if(strcmp(argv[i],"sun") == 0){
			wday |= 0x40;

		}
		else if(strcmp(argv[i],"hebdomad") == 0){
			wday |= 0x7f;
			break;
		}else{
			vty_out(vty,"<error> weekdays you want (like Sun Mon Tue Wed Thu Fri Sat or hebdomad)\n");
			return CMD_SUCCESS; 	
		}
	}
	
	if(vty->node == WLAN_NODE){
		index = 0;			
		wlanid = (int)vty->index;
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index; 		
		wlanid = (int)vty->index_sub;
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		wlanid = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/
	
	ret = dcli_wlan_service_control_timer(index,localid,policy,wlanid,is_once,wday,time,dcli_dbus_connection);
	if(ret == 0)
		vty_out(vty,"set wlan %s service successfully\n",argv[0]);
	else if(ret == WLAN_SERVICE_CONTROL_BE_USED)								//mahz add 2011.5.4
		vty_out(vty,"the starttimer or stoptimer should be disabled\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	return CMD_SUCCESS; 

}
DEFUN(set_wlan_timer_able_func,
		  set_wlan_timer_able_cmd,
		  "set wlan (starttimer|stoptimer) (enable|disable)",
		  "sta ip mac binding config\n"
		  "sta ip mac binding enable|disable\n"
	 )
{	
	int ret;
	unsigned char wlanid = 0;
	unsigned int i = 0;
    int policy = 0;
	int timer = 0;
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507

	if (!strcmp(argv[0],"starttimer"))
	{
		timer = 1;	
	}
	else if (!strcmp(argv[0],"stoptimer"))
	{
		timer = 0;	
	}
	else
	{
		vty_out(vty,"<error> first input patameter only with 'starttimer' or 'stoptimer'\n");
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
		vty_out(vty,"<error>second input patameter only with 'enable' or 'disable'\n");
		return CMD_SUCCESS;
	}

	if(vty->node == WLAN_NODE){
		index = 0;			
		wlanid = (int)vty->index;
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index; 		
		wlanid = (int)vty->index_sub;
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		wlanid = (int)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ret = dcli_wlan_timer_able(index,localid,policy,timer,wlanid,dcli_dbus_connection);
	if(ret == 0)
		vty_out(vty,"set wlan %s %s successfully\n",argv[0],argv[1]);
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	
	return CMD_SUCCESS; 

}


DEFUN(show_wlan_ptk_info_cmd_func,
	  show_wlan_ptk_info_cmd,
	  "show wlan ptk info",
	  "wlan configuration\n"
	  "wlan ptk info\n"
	  "wlan ptk info\n"
	 )
{
	int ret = WID_DBUS_SUCCESS;;
	unsigned char WlanID = 0;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	wid_wifi_info info;

	dbus_error_init(&err);
	
	WlanID = (unsigned char)vty->index;

	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		WlanID = (unsigned char)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		WlanID = (unsigned char)vty->index_sub;
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		WlanID = (unsigned char)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_SHOW_WLAN_PTK_INFO);

	/*query = dbus_message_new_method_call(WID_DBUS_BUSNAME,WID_DBUS_WLAN_OBJPATH,\
						WID_DBUS_WLAN_INTERFACE,WID_DBUS_WLAN_METHOD_SHOW_WLAN_PTK_INFO);*/

	

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&WlanID,
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
		dbus_message_iter_get_basic(&iter,&info.tx_packets);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&info.tx_errors);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&info.tx_unicast);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&info.tx_broadcast);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&info.tx_drop);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&info.tx_bytes);
	
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&info.rx_packets);
		
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&info.rx_errors);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&info.rx_unicast);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&info.rx_broadcast);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&info.rx_drop);

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&info.rx_bytes);
	}
	if(ret == 0)
	{
		vty_out(vty,"wlan	%d\n",WlanID);
		vty_out(vty,"wlan tx_packets		%d\n",info.tx_packets);
		vty_out(vty,"wlan tx_errors		%d\n",info.tx_errors);
		vty_out(vty,"wlan tx_unicast		%d\n",info.tx_unicast);
		vty_out(vty,"wlan tx_broadcast	%d\n",info.tx_broadcast);
		vty_out(vty,"wlan tx_drop		%d\n",info.tx_drop);
		vty_out(vty,"wlan tx_bytes		%d\n",info.tx_bytes);
		
		vty_out(vty,"wlan rx_packets		%d\n",info.rx_packets);
		vty_out(vty,"wlan rx_errors		%d\n",info.rx_errors);
		vty_out(vty,"wlan rx_unicast		%d\n",info.rx_unicast);
		vty_out(vty,"wlan rx_broadcast	%d\n",info.rx_broadcast);
		vty_out(vty,"wlan rx_drop		%d\n",info.rx_drop);
		vty_out(vty,"wlan rx_bytes		%d\n",info.rx_bytes);

	}
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan id does not exist\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>  %d\n",ret);
	

	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

DEFUN(show_all_wlan_ssid_config_information_func,
	  show_all_wlan_ssid_config_information_cmd,
	  "show ssid config information of all wlan [remote] [local] [PARAMETER]",
	  SHOW_STR
	  "show ssid config information of all wlan\n"
  	  "ssid config info\n"
  	  "ssid config info\n"
	  "of all wlan\n"
	  "all wlan\n"
	  "all wlan\n"
  	  "'remote' or 'local' hansi\n"
  	  "'remote' or 'local' hansi\n"
  	  "slotid-instid\n"
	 )
{
		int wlan_num;
		int ret=0;
		struct SSIDConfigInfo  *WlanNode = NULL;
		struct SSIDConfigInfo  *WlanHead = NULL;
		
		int profile = 0;
		int instRun = 0;
		int flag = 0;
		int index=0;
		int localid = 1;  //fengwenchao add 20110507
		int slot_id = HostSlotId;   //fengwenchao add 20110507
		 if(vty->node == HANSI_NODE)
		{
			index = vty->index;		
			localid = vty->local;   //fengwenchao add 20110507
			slot_id = vty->slotindex; //fengwenchao add 20110507
		}
		else if(vty->node == HANSI_WLAN_NODE){
			index = vty->index;
			localid = vty->local;   //fengwenchao add 20110507
			slot_id = vty->slotindex; //fengwenchao add 20110507
		}
		/*fengwenchao add 20110507*/
		else if(vty->node == LOCAL_HANSI_NODE)
		{
			index = vty->index;
			localid = vty->local;   //fengwenchao add 20110507
			slot_id = vty->slotindex; //fengwenchao add 20110507			
		}
		else if (vty->node == LOCAL_HANSI_WLAN_NODE)
		{
			index = vty->index;
			localid = vty->local;
			slot_id = vty->slotindex;
		}
		DBusConnection *dcli_dbus_connection = NULL;
		ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
		/*fengwenchao add end*/
		
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
			WlanHead=show_SSIDConfig_info_of_all_wlan(index,localid,dcli_dbus_connection,&wlan_num,&ret);
			if(WlanHead!=NULL && ret==0){
				WlanNode=WlanHead;
				vty_out(vty,"wlan_num		%d\n",wlan_num);
				vty_out(vty,"=================================================================\n");
				while(WlanNode){
					vty_out(vty,"-----------------------------------------------------------------\n");
					vty_out(vty,"wlanCurrID			%d\n",WlanNode->wlanCurrID);
					vty_out(vty,"Applied SecurityId:		%d\n",WlanNode->SecurityID);
					vty_out(vty,"NewSSIDName			%s\n",WlanNode->NewSSIDName);
					vty_out(vty,"NewSSIDEnabled			%d\n",WlanNode->NewSSIDEnabled);
					vty_out(vty,"NewSSIDHidden			%d\n",WlanNode->NewSSIDHidden);
					vty_out(vty,"NewStaIsolate			%d\n",WlanNode->NewStaIsolate);
					vty_out(vty,"NewDot11Auth			%d\n",WlanNode->NewDot11Auth);
					vty_out(vty,"Newsecurity			%d\n",WlanNode->Newsecurity);
					vty_out(vty,"NewAuthenMode			%d\n",WlanNode->NewAuthenMode);
					vty_out(vty,"NewSecurityCiphers		%d\n",WlanNode->NewSecurityCiphers);
					vty_out(vty,"NewEncrInputType		%d\n",WlanNode->NewEncrInputType);
					vty_out(vty,"NewSecurityKEY			%s\n",WlanNode->NewSecurityKEY);
					vty_out(vty,"NewExtensibleAuth		%d\n",WlanNode->NewExtensibleAuth);
					vty_out(vty,"NewAuthIP			%s\n",WlanNode->NewAuthIP);
					vty_out(vty,"NewAuthPort			%d\n",WlanNode->NewAuthPort);
					vty_out(vty,"NewAuthSharedSecret		%s\n",WlanNode->NewAuthSharedSecret);
					vty_out(vty,"NewAcctIP			%s\n",WlanNode->NewAcctIP);
					vty_out(vty,"NewAcctPort			%d\n",WlanNode->NewAcctPort);
					vty_out(vty,"NewAcctSharedSecret		%s\n",WlanNode->NewAcctSharedSecret);
					vty_out(vty,"NewVlanId			%d\n",WlanNode->NewVlanId);
					vty_out(vty,"NewMaxSimultUsers		%d\n",WlanNode->NewMaxSimultUsers);
					vty_out(vty,"NewStaUplinkMaxRate		%d\n",WlanNode->NewStaUplinkMaxRate);
					vty_out(vty,"NewStaDwlinkMaxRate		%d\n",WlanNode->NewStaDwlinkMaxRate);
					vty_out(vty,"SSIDRowStatus			%d\n",WlanNode->SSIDRowStatus);
					vty_out(vty,"Terminal Aged Time:		%d\n",WlanNode->sta_aged);
					vty_out(vty,"Max No Traffic Time:		%d\n",WlanNode->authentication_aged);
					vty_out(vty,"WAPI AS IP:		%s\n",WlanNode->asip);
					vty_out(vty,"WAPI AS CERT PATH:		%s\n",WlanNode->as_path);
					vty_out(vty,"WAPI AE CERT PATH:		%s\n",WlanNode->ae_path);
					vty_out(vty,"WAPI CERT TPYE: 		%s\n",(WlanNode->cert_type==1)?"X.509":((WlanNode->cert_type==2)?"GBW":" "));
					vty_out(vty,"-----------------------------------------------------------------\n");
					WlanNode=WlanNode->next;
					}
				vty_out(vty,"=================================================================\n");
				}
			else if(ret == WLAN_ID_NOT_EXIST){
				vty_out(vty,"<error> there is no wlan\n");
			}
			else if(ret == ASD_SECURITY_NOT_EXIST){
				vty_out(vty,"<error> security profile does not exist\n");
			}
			else if(ret == ASD_DBUS_ERROR || ret == WID_DBUS_ERROR){
				cli_syslog_info("<error> failed get reply.\n");
			}
			else{
				vty_out(vty,"<error> ret=%d.\n",ret);
			}
			
			if(WlanHead){
				dcli_free_SSIDConfigInfo_head(WlanHead);
			}
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
					WlanHead=show_SSIDConfig_info_of_all_wlan(profile,localid,dcli_dbus_connection,&wlan_num,&ret);
					vty_out(vty,"==============================================================================\n");
					vty_out(vty,"hansi %d-%d\n",slot_id,profile);
					vty_out(vty,"-----------------------------------------------------------------------------\n");
					if(WlanHead!=NULL && ret==0){
						WlanNode=WlanHead;
						vty_out(vty,"wlan_num		%d\n",wlan_num);
						vty_out(vty,"=================================================================\n");
						while(WlanNode){
							vty_out(vty,"-----------------------------------------------------------------\n");
							vty_out(vty,"wlanCurrID			%d\n",WlanNode->wlanCurrID);
							vty_out(vty,"Applied SecurityId:		%d\n",WlanNode->SecurityID);
							vty_out(vty,"NewSSIDName			%s\n",WlanNode->NewSSIDName);
							vty_out(vty,"NewSSIDEnabled			%d\n",WlanNode->NewSSIDEnabled);
							vty_out(vty,"NewSSIDHidden			%d\n",WlanNode->NewSSIDHidden);
							vty_out(vty,"NewStaIsolate			%d\n",WlanNode->NewStaIsolate);
							vty_out(vty,"NewDot11Auth			%d\n",WlanNode->NewDot11Auth);
							vty_out(vty,"Newsecurity			%d\n",WlanNode->Newsecurity);
							vty_out(vty,"NewAuthenMode			%d\n",WlanNode->NewAuthenMode);
							vty_out(vty,"NewSecurityCiphers		%d\n",WlanNode->NewSecurityCiphers);
							vty_out(vty,"NewEncrInputType		%d\n",WlanNode->NewEncrInputType);
							vty_out(vty,"NewSecurityKEY			%s\n",WlanNode->NewSecurityKEY);
							vty_out(vty,"NewExtensibleAuth		%d\n",WlanNode->NewExtensibleAuth);
							vty_out(vty,"NewAuthIP			%s\n",WlanNode->NewAuthIP);
							vty_out(vty,"NewAuthPort			%d\n",WlanNode->NewAuthPort);
							vty_out(vty,"NewAuthSharedSecret		%s\n",WlanNode->NewAuthSharedSecret);
							vty_out(vty,"NewAcctIP			%s\n",WlanNode->NewAcctIP);
							vty_out(vty,"NewAcctPort			%d\n",WlanNode->NewAcctPort);
							vty_out(vty,"NewAcctSharedSecret		%s\n",WlanNode->NewAcctSharedSecret);
							vty_out(vty,"NewVlanId			%d\n",WlanNode->NewVlanId);
							vty_out(vty,"NewMaxSimultUsers		%d\n",WlanNode->NewMaxSimultUsers);
							vty_out(vty,"NewStaUplinkMaxRate		%d\n",WlanNode->NewStaUplinkMaxRate);
							vty_out(vty,"NewStaDwlinkMaxRate		%d\n",WlanNode->NewStaDwlinkMaxRate);
							vty_out(vty,"SSIDRowStatus			%d\n",WlanNode->SSIDRowStatus);
							vty_out(vty,"Terminal Aged Time:		%d\n",WlanNode->sta_aged);
							vty_out(vty,"Max No Traffic Time:		%d\n",WlanNode->authentication_aged);
							vty_out(vty,"WAPI AS IP:		%s\n",WlanNode->asip);
							vty_out(vty,"WAPI AS CERT PATH:		%s\n",WlanNode->as_path);
							vty_out(vty,"WAPI AE CERT PATH:		%s\n",WlanNode->ae_path);
							vty_out(vty,"WAPI CERT TPYE: 		%s\n",(WlanNode->cert_type==1)?"X.509":((WlanNode->cert_type==2)?"GBW":" "));
							vty_out(vty,"-----------------------------------------------------------------\n");
							WlanNode=WlanNode->next;
							}
						}
					else if(ret == WLAN_ID_NOT_EXIST){
						vty_out(vty,"<error> there is no wlan\n");
					}
					else if(ret == ASD_SECURITY_NOT_EXIST){
						vty_out(vty,"<error> security profile does not exist\n");
					}
					else if(ret == ASD_DBUS_ERROR || ret == WID_DBUS_ERROR){
						cli_syslog_info("<error> failed get reply.\n");
					}
					else{
						vty_out(vty,"<error> ret=%d.\n",ret);
					}
					
					if(WlanHead){
						dcli_free_SSIDConfigInfo_head(WlanHead);
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
				WlanHead=show_SSIDConfig_info_of_all_wlan(profile,localid,dcli_dbus_connection,&wlan_num,&ret);
				vty_out(vty,"==============================================================================\n");
				vty_out(vty,"local hansi %d-%d\n",slot_id,profile);
				vty_out(vty,"-----------------------------------------------------------------------------\n");
				if(WlanHead!=NULL && ret==0){
					WlanNode=WlanHead;
					vty_out(vty,"wlan_num		%d\n",wlan_num);
					vty_out(vty,"=================================================================\n");
					while(WlanNode){
						vty_out(vty,"-----------------------------------------------------------------\n");
						vty_out(vty,"wlanCurrID			%d\n",WlanNode->wlanCurrID);
						vty_out(vty,"Applied SecurityId:		%d\n",WlanNode->SecurityID);
						vty_out(vty,"NewSSIDName			%s\n",WlanNode->NewSSIDName);
						vty_out(vty,"NewSSIDEnabled			%d\n",WlanNode->NewSSIDEnabled);
						vty_out(vty,"NewSSIDHidden			%d\n",WlanNode->NewSSIDHidden);
						vty_out(vty,"NewStaIsolate			%d\n",WlanNode->NewStaIsolate);
						vty_out(vty,"NewDot11Auth			%d\n",WlanNode->NewDot11Auth);
						vty_out(vty,"Newsecurity			%d\n",WlanNode->Newsecurity);
						vty_out(vty,"NewAuthenMode			%d\n",WlanNode->NewAuthenMode);
						vty_out(vty,"NewSecurityCiphers		%d\n",WlanNode->NewSecurityCiphers);
						vty_out(vty,"NewEncrInputType		%d\n",WlanNode->NewEncrInputType);
						vty_out(vty,"NewSecurityKEY			%s\n",WlanNode->NewSecurityKEY);
						vty_out(vty,"NewExtensibleAuth		%d\n",WlanNode->NewExtensibleAuth);
						vty_out(vty,"NewAuthIP			%s\n",WlanNode->NewAuthIP);
						vty_out(vty,"NewAuthPort			%d\n",WlanNode->NewAuthPort);
						vty_out(vty,"NewAuthSharedSecret		%s\n",WlanNode->NewAuthSharedSecret);
						vty_out(vty,"NewAcctIP			%s\n",WlanNode->NewAcctIP);
						vty_out(vty,"NewAcctPort			%d\n",WlanNode->NewAcctPort);
						vty_out(vty,"NewAcctSharedSecret		%s\n",WlanNode->NewAcctSharedSecret);
						vty_out(vty,"NewVlanId			%d\n",WlanNode->NewVlanId);
						vty_out(vty,"NewMaxSimultUsers		%d\n",WlanNode->NewMaxSimultUsers);
						vty_out(vty,"NewStaUplinkMaxRate		%d\n",WlanNode->NewStaUplinkMaxRate);
						vty_out(vty,"NewStaDwlinkMaxRate		%d\n",WlanNode->NewStaDwlinkMaxRate);
						vty_out(vty,"SSIDRowStatus			%d\n",WlanNode->SSIDRowStatus);
						vty_out(vty,"Terminal Aged Time:		%d\n",WlanNode->sta_aged);
						vty_out(vty,"Max No Traffic Time:		%d\n",WlanNode->authentication_aged);
						vty_out(vty,"WAPI AS IP:		%s\n",WlanNode->asip);
						vty_out(vty,"WAPI AS CERT PATH:		%s\n",WlanNode->as_path);
						vty_out(vty,"WAPI AE CERT PATH:		%s\n",WlanNode->ae_path);
						vty_out(vty,"WAPI CERT TPYE: 		%s\n",(WlanNode->cert_type==1)?"X.509":((WlanNode->cert_type==2)?"GBW":" "));
						vty_out(vty,"-----------------------------------------------------------------\n");
						WlanNode=WlanNode->next;
						}
					}
				else if(ret == WLAN_ID_NOT_EXIST){
					vty_out(vty,"<error> there is no wlan\n");
				}
				else if(ret == ASD_SECURITY_NOT_EXIST){
					vty_out(vty,"<error> security profile does not exist\n");
				}
				else if(ret == ASD_DBUS_ERROR || ret == WID_DBUS_ERROR){
					cli_syslog_info("<error> failed get reply.\n");
				}
				else{
					vty_out(vty,"<error> ret=%d.\n",ret);
				}
				
				if(WlanHead){
					dcli_free_SSIDConfigInfo_head(WlanHead);
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
/*add for showing wlan wapi basic  information 20100521 nl bb1*/
DEFUN(show_all_wlan_wapi_basic_information_func,
	  show_all_wlan_wapi_basic_information_cmd,
	  "show wlan wapi basic information_of all_wtp",
	  SHOW_STR
	  "Display wlan wapi basic information of all wtps\n"
	  "List wlan wapi basic information\n"
	  "by wtp display wapi basic information\n"
	 )
{
	DBusConnection *dbus_connection = dcli_dbus_connection;
	
	struct ConfigWapiInfo *WlanHead = NULL;
	struct ConfigWapiInfo *WlanShowNode = NULL;
	int ret=0;
	int i=0;
	int j=0;
	unsigned int num = 0;
	
	int index=0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507

	/*fengwenchao add 20110507*/
	if(vty->node == HANSI_NODE)
	{
		index = vty->index;		
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507		
	}
	else if (vty->node == LOCAL_HANSI_NODE)
	{
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	
	ReInitDbusConnection(&dbus_connection,slot_id);
	/*fengwenchao add end*/

	WlanHead = show_ConfigWapiInfo_of_all_wtp(index,localid,dbus_connection, &num, &ret);

	if((WlanHead!=NULL)&&(ret == 0)){
		vty_out(vty,"WAPI WLANNum:%-5d  \n",num);
		vty_out(vty,"========================================================================== \n");
		for (i = 0; i < num; i++) {	
			if(WlanShowNode == NULL)
				WlanShowNode = WlanHead->ConfigWapiInfo_list;
			else 
				WlanShowNode = WlanShowNode->next;

			if(WlanShowNode == NULL)
				break;
			
			vty_out(vty,"------------------------------WAPI WLAN information------------------------------\n");
			vty_out(vty,"WLANID:   %-5d  \t\t\t\t",WlanShowNode->WlanId);
			vty_out(vty,"SecurityID:   %-5d  \n",WlanShowNode->SecurityID);
			vty_out(vty,"WapiCipherKeyCharType:   %-5d  \t\t",WlanShowNode->WapiCipherKeyCharType);
			vty_out(vty,"WAPIASIPAddress:   %s  \n",WlanShowNode->WAPIASIPAddress);
				
			vty_out(vty,"-------------------------------------------------------------------------\n");
		}
		vty_out(vty,"========================================================================== \n");
	}

	else if (ret == ASD_DBUS_ERROR){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == ASD_WLAN_NOT_EXIST){
		vty_out(vty,"<error> There is no Wlan now.\n");
	}
	else if(ret == ASD_WAPI_WLAN_NOT_EXIST){
		vty_out(vty,"<error> There is no Wapi Wlan now.\n");
	}
	else{
		vty_out(vty,"<error> ret = %d\n",ret);
	}
	dcli_free_wtp_ConfigWapiInfoInfo(WlanHead);
	return CMD_SUCCESS;
}

/*add for showing wlan WtpWAPIPerformanceStatsInfo  information 20100531 nl bb2*/
DEFUN(show_all_wlan_wapi_performance_stats_information_func,
	  show_all_wlan_wapi_performance_stats_information_cmd,
	  "show all the wapi_wlan performance_stats information",
	  SHOW_STR
	  "Display all wapi wlan performance stats information   \n"
	  "List  wapi wlan performance stats information\n"
	  "by wlan display wapi performance stats information\n"
	 )
{
	DBusConnection *dbus_connection = dcli_dbus_connection;
	
	struct WtpWAPIPerformanceStatsInfo *WlanHead = NULL;
	struct WtpWAPIPerformanceStatsInfo *WlanShowNode = NULL;
	int ret=0;
	int i=0;
	int j=0;
	unsigned int num = 0;
	
	int index=0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	if(vty->node == HANSI_NODE){
		index = vty->index;		
		localid = vty->local;   //fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}	
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_NODE)
	{
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	ReInitDbusConnection(&dbus_connection,slot_id);
	/*fengwenchao add end*/
	WlanHead = show_WtpWAPIPerformanceStatsInfo_of_all_wlan(index,localid,dbus_connection, &num, &ret);
	
	if((WlanHead!=NULL)&&(ret == 0)){
		vty_out(vty,"WAPI WLANNum:%-5d  \n",num);
		vty_out(vty,"========================================================================== \n");
		for (i = 0; i < num; i++) {	
			if(WlanShowNode == NULL)
				WlanShowNode = WlanHead->WtpWAPIPerformanceStatsInfo_list;
			else 
				WlanShowNode = WlanShowNode->next;

			if(WlanShowNode == NULL)
				break;
			
			vty_out(vty,"------------------------------WAPI WLAN information------------------------------\n");
			vty_out(vty,"WLANID:   %-5d  \t\t\t\t",WlanShowNode->WlanId);
			vty_out(vty,"SecurityID:   %-5d  \n",WlanShowNode->SecurityID);
			vty_out(vty,"SecurityType:   %-5d  \n",WlanShowNode->SecurityType);
			vty_out(vty,"wlan_bss_num:   %-5d  \t\t\t\t",WlanShowNode->wlan_bss_num);
			vty_out(vty,"wlan_total_sta_num:   %d  \n",WlanShowNode->wlan_total_sta_num);
				
			vty_out(vty,"-------------------------------------------------------------------------\n");
			
			struct Sub_Sta_WtpWAPIPerformance *StaShowNode = NULL;

			for(j=0;j<WlanShowNode->wlan_total_sta_num;j++){
				
				if(StaShowNode == NULL)
					StaShowNode = WlanShowNode->Sub_Sta_WtpWAPIPerformance_head;
				else 
					StaShowNode = StaShowNode->next;

				if(StaShowNode == NULL){
					vty_out(vty,"break\n");
					break;
				}
				
				vty_out(vty,"------------------------station information:------------------------------- \n");
				
				vty_out(vty,"sta_seq:  %-5d  \n",(StaShowNode->sta_seq+1));
				
				vty_out(vty,"STA MAC:  %02X:%02X:%02X:%02X:%02X:%02X \n",
				StaShowNode->staMacAddr[0],StaShowNode->staMacAddr[1],StaShowNode->staMacAddr[2],
				StaShowNode->staMacAddr[3],StaShowNode->staMacAddr[4],StaShowNode->staMacAddr[5]);
				
				vty_out(vty,"wtpWapiVersion:  %-5d  \t\t\t\t",(StaShowNode->wtpWapiVersion));
				vty_out(vty,"wtpWapiControlledPortStatus:  %-5d  \n",StaShowNode->wtpWapiControlledPortStatus);
				
				vty_out(vty,"wtpWapiWPIReplayCounters:  %-5d  \t\t\t",(StaShowNode->wtpWapiWPIReplayCounters));
				vty_out(vty,"wtpWapiWPIDecryptableErrors:  %-5d  \n",StaShowNode->wtpWapiWPIDecryptableErrors);
				vty_out(vty,"wtpWapiWPIMICErrors:  %-5d  \t\t\t\t",(StaShowNode->wtpWapiWPIMICErrors));
				vty_out(vty,"wtpWapiWAISignatureErrors:  %-5d  \n",StaShowNode->wtpWapiWAISignatureErrors);
				
				vty_out(vty,"wtpWapiWAIHMACErrors:  %-5d  \t\t\t\t",(StaShowNode->wtpWapiWAIHMACErrors));
				vty_out(vty,"wtpWapiWAIAuthResultFailures:  %-5d  \n",StaShowNode->wtpWapiWAIAuthResultFailures);
				vty_out(vty,"wtpWapiWAIDiscardCounters:  %-5d  \t\t\t",(StaShowNode->wtpWapiWAIDiscardCounters));
				vty_out(vty,"wtpWapiWAITimeoutCounters:  %-5d  \n",StaShowNode->wtpWapiWAITimeoutCounters);
				
				
				vty_out(vty,"wtpWapiWAIFormatErrors:  %-5d  \t\t\t",(StaShowNode->wtpWapiWAIFormatErrors));
				vty_out(vty,"wtpWapiWAICertificateHandshakeFailures:  %-5d  \n",StaShowNode->wtpWapiWAICertificateHandshakeFailures);
				vty_out(vty,"wtpWapiWAIUnicastHandshakeFailures:  %-5d  \t\t",(StaShowNode->wtpWapiWAIUnicastHandshakeFailures));
				vty_out(vty,"wtpWapiWAIMulticastHandshakeFailures:  %-5d  \n",StaShowNode->wtpWapiWAIMulticastHandshakeFailures);

				vty_out(vty,"STA wtpWapiSelectedUnicastCipher:  %02X:%02X:%02X:%02X \n",
								StaShowNode->wtpWapiSelectedUnicastCipher[0],StaShowNode->wtpWapiSelectedUnicastCipher[1],
								StaShowNode->wtpWapiSelectedUnicastCipher[2],StaShowNode->wtpWapiSelectedUnicastCipher[3]);

				vty_out(vty,"\n  ");
			}

			vty_out(vty,"-------------------------------------------------------------------------\n");
		}
		vty_out(vty,"========================================================================== \n");
	}

	else if (ret == ASD_DBUS_ERROR){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == ASD_WLAN_NOT_EXIST){
		vty_out(vty,"<error> There is no Wlan now.\n");
	}
	else if(ret == ASD_WAPI_WLAN_NOT_EXIST){
		vty_out(vty,"<error> There is no Wapi Wlan now.\n");
	}
	else{
		vty_out(vty,"<error> ret = %d\n",ret);
	}
	dcli_free_WtpWAPIPerformanceStatsInfo(WlanHead);
	return CMD_SUCCESS;
}

/*add for showing wlan wapi extend config information 20100601 nl bb3*/
DEFUN(show_all_wlan_wapi_extend_config_information_func,
	  show_all_wlan_wapi_extend_config_information_cmd,
	  "show all the wapi_wlan extend_config information",
	  SHOW_STR
	  "Display all wapi wlan wapi extend config information\n"
	  "List  wapi wlan extend config information\n"
	  "by wlan display wapi wlan extend config information\n"
	 )
{
	DBusConnection *dbus_connection = dcli_dbus_connection;
	
	struct WtpWAPIExtendConfigInfo *WlanHead = NULL;
	struct WtpWAPIExtendConfigInfo *WlanShowNode = NULL;
	int ret=0;
	int i=0;
	int j=0;
	unsigned int num = 0;
	
	int index=0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	if(vty->node == HANSI_NODE)
	{
		index = vty->index; 	
		localid = vty->local;	//fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_NODE)
	{
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	ReInitDbusConnection(&dbus_connection,slot_id,distributFag);	
	/*fengwenchao add end*/

	WlanHead = show_All_WAPIWlan_ExtendConfigInfo(index,localid,dbus_connection, &num, &ret);

	if((WlanHead!=NULL)&&(ret == 0)){
		vty_out(vty,"WAPI WLANNum:%-5d  \n",num);
		vty_out(vty,"========================================================================== \n");
		for (i = 0; i < num; i++) {	
			if(WlanShowNode == NULL)
				WlanShowNode = WlanHead->WtpWAPIExtendConfigInfo_list;
			else 
				WlanShowNode = WlanShowNode->next;

			if(WlanShowNode == NULL)
				break;
			
			vty_out(vty,"------------------------------WAPI WLAN information------------------------------\n");
			vty_out(vty,"WapiWlanID:   %-5d  \t\t\t\t\t",WlanShowNode->WapiWlanID);
			vty_out(vty,"SecurityID:   %-5d  \n",WlanShowNode->SecurityID);
			vty_out(vty,"SecurityType:   %-5d  \t\t\t\t\t",WlanShowNode->SecurityType);
			vty_out(vty,"wtpWapiConfigVersion:   %d  \n",WlanShowNode->wtpWapiConfigVersion);

			vty_out(vty,"wtpWapiControlledAuthenControlenabled:   %-5d  \t",WlanShowNode->wtpWapiControlledAuthenControlenabled);
			vty_out(vty,"wtpWapiMulticastCipherSize:   %-5d  \n",WlanShowNode->wtpWapiMulticastCipherSize);
			vty_out(vty,"wtpWapiWPIOptionImplement:   %-5d  \t\t\t",WlanShowNode->wtpWapiWPIOptionImplement);
			vty_out(vty,"wtpWapiWPIPreauthImplemented:   %d  \n",WlanShowNode->wtpWapiWPIPreauthImplemented);
			
			vty_out(vty,"wtpWapiEnabled:   %-5d  \t\t\t\t",WlanShowNode->wtpWapiEnabled);
			vty_out(vty,"wtpWapiPreauthEnabled:   %d  \n",WlanShowNode->wtpWapiPreauthEnabled);
			vty_out(vty,"wtpWapiUnicastKeysSupported:   %-5d	\t\t",WlanShowNode->wtpWapiUnicastKeysSupported);
			vty_out(vty,"wtpWapiUnicastRekeyMethod:	 %-5d  \n",WlanShowNode->wtpWapiUnicastRekeyMethod);
			
			vty_out(vty,"wtpWapiUnicastRekeyTime:   %-5d	\t\t",WlanShowNode->wtpWapiUnicastRekeyTime);
			vty_out(vty,"wtpWapiUnicastRekeyPackets:	 %d  \n",WlanShowNode->wtpWapiUnicastRekeyPackets);
			vty_out(vty,"wtpWapiMulticastRekeyMethod:   %-5d  \t\t\t",WlanShowNode->wtpWapiMulticastRekeyMethod);
			vty_out(vty,"wtpWapiMulticastRekeyTime:   %d  \n",WlanShowNode->wtpWapiMulticastRekeyTime);

			vty_out(vty,"wtpWapiMulticastRekeyPackets:   %-5d  \t\t",WlanShowNode->wtpWapiMulticastRekeyPackets);
			vty_out(vty,"wtpWapiMulticastRekeyStrict:   %d  \n",WlanShowNode->wtpWapiMulticastRekeyStrict);
			vty_out(vty,"wtpWapiCertificateUpdateCount:   %-5d	\t\t",WlanShowNode->wtpWapiCertificateUpdateCount);
			vty_out(vty,"wtpWapiMulticastUpdateCount:	 %-5d  \n",WlanShowNode->wtpWapiMulticastUpdateCount);
			
			vty_out(vty,"wtpWapiUnicastUpdateCount:   %-5d	\t\t",WlanShowNode->wtpWapiUnicastUpdateCount);
			vty_out(vty,"wtpWapiBKLifetime:	 %d   \n",WlanShowNode->wtpWapiBKLifetime);
			vty_out(vty,"wtpWapiBKReauthThreshold:   %-5d  \t\t\t",WlanShowNode->wtpWapiBKReauthThreshold);
			vty_out(vty,"wtpWapiSATimeout:   %d  \n",WlanShowNode->wtpWapiSATimeout);

			vty_out(vty,"wtpWapiPSKValue:	 %s  \t\t\t\t",WlanShowNode->wtpWapiPSKValue);
			vty_out(vty,"wtpWapiPSKPassPhrase:	 %s  \n",WlanShowNode->wtpWapiPSKPassPhrase);
			vty_out(vty,"wtpWapiMulticastCipher:	%02X:%02X:%02X:%02X \n",
						WlanShowNode->wtpWapiMulticastCipher[0],WlanShowNode->wtpWapiMulticastCipher[1],
						WlanShowNode->wtpWapiMulticastCipher[2],WlanShowNode->wtpWapiMulticastCipher[3]);
			vty_out(vty,"wtpWapiAuthSuiteSelected:	%02X:%02X:%02X:%02X \t\t",
						WlanShowNode->wtpWapiAuthSuiteSelected[0],WlanShowNode->wtpWapiAuthSuiteSelected[1],
						WlanShowNode->wtpWapiAuthSuiteSelected[2],WlanShowNode->wtpWapiAuthSuiteSelected[3]);
			vty_out(vty,"wtpWapiAuthSuiteRequested:	%02X:%02X:%02X:%02X \n",
						WlanShowNode->wtpWapiAuthSuiteRequested[0],WlanShowNode->wtpWapiAuthSuiteRequested[1],
						WlanShowNode->wtpWapiAuthSuiteRequested[2],WlanShowNode->wtpWapiAuthSuiteRequested[3]);
			vty_out(vty,"wtpWapiBKIDUsed:	%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
					WlanShowNode->wtpWapiBKIDUsed[0],WlanShowNode->wtpWapiBKIDUsed[1],WlanShowNode->wtpWapiBKIDUsed[2],WlanShowNode->wtpWapiBKIDUsed[3],
					WlanShowNode->wtpWapiBKIDUsed[4],WlanShowNode->wtpWapiBKIDUsed[5],WlanShowNode->wtpWapiBKIDUsed[6],WlanShowNode->wtpWapiBKIDUsed[7],
					WlanShowNode->wtpWapiBKIDUsed[8],WlanShowNode->wtpWapiBKIDUsed[9],WlanShowNode->wtpWapiBKIDUsed[10],WlanShowNode->wtpWapiBKIDUsed[11],
					WlanShowNode->wtpWapiBKIDUsed[12],WlanShowNode->wtpWapiBKIDUsed[13],WlanShowNode->wtpWapiBKIDUsed[14],WlanShowNode->wtpWapiBKIDUsed[15]);

			vty_out(vty,"-------------------------------------------------------------------------\n");
		}
		vty_out(vty,"========================================================================== \n");
	}

	else if (ret == ASD_DBUS_ERROR){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == ASD_WLAN_NOT_EXIST){
		vty_out(vty,"<error> There is no Wlan now.\n");
	}
	else if(ret == ASD_WAPI_WLAN_NOT_EXIST){
		vty_out(vty,"<error> There is no Wapi Wlan now.\n");
	}
	else{
		vty_out(vty,"<error> ret = %d\n",ret);
	}
	dcli_free_WAPI_WLAN_ExtendConfigInfo(WlanHead);
	return CMD_SUCCESS;
}
/*add for showing wlan unicast information 20100525 nl bb4*/
DEFUN(show_all_wlan_unicast_information_func,
	  show_all_wlan_unicast_information_cmd,
	  "show wlan unicast information_of all_wtp",
	  SHOW_STR
	  "Display wlan unicast information of all wtps\n"
	  "List wlan unicast information\n"
	  "by wtp display unicast information\n"
	 )
{
	DBusConnection *dbus_connection = dcli_dbus_connection;
	
	struct UnicastInfo *WlanHead = NULL;
	struct UnicastInfo *WlanShowNode = NULL;
	int ret=0;
	int i=0;
	int j=0;
	unsigned int num = 0;
	
	int index=0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	if(vty->node == HANSI_NODE)
	{
		index = vty->index; 	
		localid = vty->local;	//fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_NODE)
	{
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	ReInitDbusConnection(&dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/

	WlanHead = show_UnicastInfo_of_all_wtp(index,localid,dbus_connection, &num, &ret);

	if((WlanHead!=NULL)&&(ret == 0)){
		vty_out(vty,"WAPI WLANNum:%-5d  \n",num);
		vty_out(vty,"========================================================================== \n");
		for (i = 0; i < num; i++) {	
			if(WlanShowNode == NULL)
				WlanShowNode = WlanHead->UnicastInfo_list;
			else 
				WlanShowNode = WlanShowNode->next;

			if(WlanShowNode == NULL)
				break;
			
			vty_out(vty,"------------------------------WAPI WLAN information------------------------------\n");
			vty_out(vty,"WLANID:   %-5d  \t\t\t\t",WlanShowNode->UnicastWlanID);
			vty_out(vty,"SecurityID:   %-5d  \n",WlanShowNode->SecurityID);
			vty_out(vty,"NewUnicastCipherEnabled:   %-5d  \t\t",WlanShowNode->NewUnicastCipherEnabled);
			vty_out(vty,"SecurityType:   %d  \n",WlanShowNode->SecurityType);

			vty_out(vty,"-------------------------------------------------------------------------\n");
		}
		vty_out(vty,"========================================================================== \n");
	}

	else if (ret == ASD_DBUS_ERROR){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == ASD_WLAN_NOT_EXIST){
		vty_out(vty,"<error> There is no Wlan now.\n");
	}
	else if(ret == ASD_WAPI_WLAN_NOT_EXIST){
		vty_out(vty,"<error> There is no Wapi Wlan now.\n");
	}
	else{
		vty_out(vty,"<error> ret = %d\n",ret);
	}
	dcli_free_wtp_UnicastInfo(WlanHead);
	return CMD_SUCCESS;
}

/*add for showing wlan BssWAPIPerformanceStatsInfo  information 20100604 nl bb5*/
DEFUN(show_all_wlan_bss_wapi_performance_stats_information_func,
	  show_all_wlan_bss_wapi_performance_stats_information_cmd,
	  "show the all wapi_wlan bss performance_stats information",
	  SHOW_STR
	  "Display all  wapi wlan bss performance stats information   \n"
	  "List  wapi wlan bss performance stats information\n"
	  "by wlan display wapi bss performance stats information\n"
	 )
{
	DBusConnection *dbus_connection = dcli_dbus_connection;
	
	struct BssWAPIPerformanceStatsInfo *WlanHead = NULL;
	struct BssWAPIPerformanceStatsInfo *WlanShowNode = NULL;
	int ret=0;
	int i=0;
	int j=0;
	unsigned int num = 0;

	int index=0;
	int localid = 1;  //fengwenchao add 20110507
	int slot_id = HostSlotId;   //fengwenchao add 20110507
	 if(vty->node == HANSI_NODE)
	 {
		index = vty->index; 	
		localid = vty->local;	//fengwenchao add 20110507
		slot_id = vty->slotindex; //fengwenchao add 20110507
	}
	/*fengwenchao add 20110507*/
	else if (vty->node == LOCAL_HANSI_NODE)
	{
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	ReInitDbusConnection(&dbus_connection,slot_id,distributFag);
	/*fengwenchao add end*/	
	WlanHead = show_BssWAPIPerformanceStatsInfo_of_all_wlan(index,localid,dbus_connection, &num, &ret);
	
	if((WlanHead!=NULL)&&(ret == 0)){
		vty_out(vty,"WAPI WLANNum:%-5d  \n",num);
		vty_out(vty,"========================================================================== \n");
		for (i = 0; i < num; i++) {	
			if(WlanShowNode == NULL)
				WlanShowNode = WlanHead->BssWAPIPerformanceStatsInfo_list;
			else 
				WlanShowNode = WlanShowNode->next;

			if(WlanShowNode == NULL)
				break;
			
			vty_out(vty,"------------------------------WAPI WLAN information------------------------------\n");
			vty_out(vty,"WlanID:   %-5d  \t\t\t\t",WlanShowNode->WlanID);
			vty_out(vty,"SecurityID:   %-5d  \n",WlanShowNode->SecurityID);
			vty_out(vty,"SecurityType:   %-5d  \t\t\t\t",WlanShowNode->SecurityType);
			vty_out(vty,"wlan_bss_num:   %-5d  \n",WlanShowNode->wlan_bss_num);

			vty_out(vty,"-------------------------------------------------------------------------\n");
			
			struct Sub_BssWAPIPerformanceStatsInfo *BssShowNode = NULL;

			for(j=0;j<WlanShowNode->wlan_bss_num;j++){
				if(BssShowNode == NULL)
					BssShowNode = WlanShowNode->Sub_BssWAPIPerformanceStatsInfo_head;
				else 
					BssShowNode = BssShowNode->next;

				if(BssShowNode == NULL){
					vty_out(vty,"break\n");
					break;
				}
				
				vty_out(vty,"------------------------bss information:------------------------------- \n");
				
				vty_out(vty,"bss_seq:  %-5d  \t\t\t\t\t",(BssShowNode->bss_id+1));
				
				vty_out(vty,"WTP MAC:  %02X:%02X:%02X:%02X:%02X:%02X \n",
				BssShowNode->wtpMacAddr[0],BssShowNode->wtpMacAddr[1],BssShowNode->wtpMacAddr[2],
				BssShowNode->wtpMacAddr[3],BssShowNode->wtpMacAddr[4],BssShowNode->wtpMacAddr[5]);
				vty_out(vty,"wlan_id:  %-5d  \t\t\t\t\t",(BssShowNode->wlan_id));
				vty_out(vty,"wtp_id:  %-5d  \n",BssShowNode->wtp_id);
				
				vty_out(vty,"BSS MAC:  %02X:%02X:%02X:%02X:%02X:%02X \n",
				BssShowNode->wtpBssCurrID[0],BssShowNode->wtpBssCurrID[1],BssShowNode->wtpBssCurrID[2],
				BssShowNode->wtpBssCurrID[3],BssShowNode->wtpBssCurrID[4],BssShowNode->wtpBssCurrID[5]);

				vty_out(vty,"bssWapiWPIReplayCounters:  %-5d  \t\t\t",(BssShowNode->bssWapiWPIReplayCounters));
				vty_out(vty,"bssWapiWPIDecryptableErrors:  %-5d  \n",BssShowNode->bssWapiWPIDecryptableErrors);
				vty_out(vty,"bssWapiWPIMICErrors:  %-5d  \t\t\t\t",(BssShowNode->bssWapiWPIMICErrors));
				vty_out(vty,"bssWapiWAISignatureErrors:  %-5d  \n",BssShowNode->bssWapiWAISignatureErrors);
				
				vty_out(vty,"bssWapiWAIHMACErrors:  %-5d  \t\t\t\t",(BssShowNode->bssWapiWAIHMACErrors));
				vty_out(vty,"bssWapiWAIAuthResultFailures:  %-5d  \n",BssShowNode->bssWapiWAIAuthResultFailures);
				vty_out(vty,"bssWapiWAIDiscardCounters:  %-5d  \t\t\t",(BssShowNode->bssWapiWAIDiscardCounters));
				vty_out(vty,"bssWapiWAITimeoutCounters:  %-5d  \n",BssShowNode->bssWapiWAITimeoutCounters);
				
				
				vty_out(vty,"bssWapiWAIFormatErrors:  %-5d  \t\t\t",(BssShowNode->bssWapiWAIFormatErrors));
				vty_out(vty,"bssWapiWAICertificateHandshakeFailures:  %-5d  \n",BssShowNode->bssWapiWAICertificateHandshakeFailures);
				vty_out(vty,"bssWapiWAIUnicastHandshakeFailures:  %-5d  \t\t",(BssShowNode->bssWapiWAIUnicastHandshakeFailures));
				vty_out(vty,"bssWapiWAIMulticastHandshakeFailures:  %-5d  \n",BssShowNode->bssWapiWAIMulticastHandshakeFailures);

				vty_out(vty,"\n  ");

			}

			vty_out(vty,"-------------------------------------------------------------------------\n");
		}
		vty_out(vty,"========================================================================== \n");
	}

	else if (ret == ASD_DBUS_ERROR){
		cli_syslog_info("<error> failed get reply.\n");
	}
	else if(ret == ASD_WLAN_NOT_EXIST){
		vty_out(vty,"<error> There is no Wlan now.\n");
	}
	else if(ret == ASD_WAPI_WLAN_NOT_EXIST){
		vty_out(vty,"<error> There is no Wapi Wlan now.\n");
	}
	else{
		vty_out(vty,"<error> ret = %d\n",ret);
	}
	dcli_free_BssWAPIPerformanceStatsInfo(WlanHead);
	return CMD_SUCCESS;
}
/*wuwl add for REQUIREMENTS-340*/
DEFUN(set_wlan_uni_muti_bro_cast_isolation_set_func,
	  set_wlan_uni_muti_bro_cast_isolation_set_cmd,
	  "set wlan (unicast|multicast_broadcast|unicast_and_multicast_broadcast|wifi) isolation (enable|disable)",
	  CONFIG_STR
	  "wtp config\n"
	  "wtp no response to sta probe request switch\n"
	  "wtp no response to sta probe request switch\n"
	  "wtp no response to sta probe request switch\n"
	  "switch enable or disable\n"
	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char policy = 0;
	unsigned int unicast = 0;
	unsigned int rate = 0;
	int ret;
	unsigned char wlanid = 0;
	if (!strcmp(argv[0],"unicast"))
	{
		unicast = 1;	
	}
	else if (!strcmp(argv[0],"multicast_broadcast"))
	{
		unicast = 0;	
	}
	else if(!strcmp(argv[0],"unicast_and_multicast_broadcast")){
		unicast = 2;	
	}
	else if(!strcmp(argv[0],"wifi")){
		unicast = 3;
	}
	else
	{
		vty_out(vty,"<error> input patameter only with 'unicast' or 'multicast_broadcast'\n");
		return CMD_WARNING;
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
		wlanid = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index; 		
		wlanid = 0;
		localid = vty->local;
        slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = vty->index; 		
		wlanid = 0;
		localid = vty->local;
        slot_id = vty->slotindex;
	}else if(vty->node == WLAN_NODE){
		index = 0; 		
		wlanid = (int)vty->index;;
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index; 		
		wlanid = (int)vty->index_sub;
		localid = vty->local;
        slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_WLAN_NODE){
		index = vty->index; 		
		wlanid = (int)vty->index_sub;
		localid = vty->local;
        slot_id = vty->slotindex;
	}
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WLAN_UNI_MUTI_BR_CAST_ISOLATION_SW_AND_RATE_SET);
																			
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&unicast,
							 DBUS_TYPE_BYTE,&policy,
							 DBUS_TYPE_UINT32,&rate,
							 DBUS_TYPE_BYTE,&wlanid,
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
		vty_out(vty," set wlan %d %s isolation %s successfully\n",wlanid,argv[0],argv[1]);
	}
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan %d does not exist\n",wlanid);
	else if(ret == WLAN_BE_ENABLE)
		vty_out(vty,"<error> wlan is enable, please disable it first\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to some wlan, and the operation of the wlan was not successful\n");
	}
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}
/*wuwl add for REQUIREMENTS-340*/
DEFUN(set_wlan_muti_bro_cast_rate_set_func,
	  set_wlan_muti_bro_cast_rate_set_cmd,
	  "set wlan multicast_broadcast_rate RATE",
	  CONFIG_STR
	  "wtp config\n"
	  "wtp no response to sta probe request switch\n"
	  "wtp no response to sta probe request switch\n"
	  "wtp no response to sta probe request switch\n"
	  "switch enable or disable\n"
	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char policy = 0;
	unsigned int unicast = 0;
	unsigned int rate = 0;
	int ret;
	unsigned char wlanid = 0;

	ret = parse_int_ID((char *)argv[0],&rate);
	if(ret == 0)  //fengwenchao add  20120331 for autelan-2884
	{
		if (rate == 10 ||rate == 20||rate == 55||rate == 60||rate == 90||rate == 110||rate == 120||rate == 180||rate == 240||rate == 360||rate == 480||rate == 540)
		{
		}else{
			vty_out(vty,"%% Invalid rate: %s !\n", argv[0]);
			vty_out(vty,"10 20 55 60 90 110 120 180 240 360 480 540.\n");
			vty_out(vty,"if you want to set rate 5.5M,you should input 55.\n");
			return CMD_WARNING;
		}
	}
	else
	{
		vty_out(vty,"<error>unkown format\n");
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
		wlanid = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index; 		
		wlanid = 0;
		localid = vty->local;
        slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = vty->index; 		
		wlanid = 0;
		localid = vty->local;
        slot_id = vty->slotindex;
	}else if(vty->node == WLAN_NODE){
		index = 0; 		
		wlanid = (int)vty->index;;
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index; 		
		wlanid = (int)vty->index_sub;
		localid = vty->local;
        slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_WLAN_NODE){
		index = vty->index; 		
		wlanid = (int)vty->index_sub;
		localid = vty->local;
        slot_id = vty->slotindex;
	}
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WLAN_MUTI_BR_CAST_RATE_SET);
																			
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&unicast,
							 DBUS_TYPE_BYTE,&policy,
							 DBUS_TYPE_UINT32,&rate,
							 DBUS_TYPE_BYTE,&wlanid,
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
		vty_out(vty," set wlan %d rate %d successfully\n",wlanid,rate);
	}
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan %d does not exist\n",wlanid);
	else if(ret == RADIO_SUPPORT_RATE_EXIST)
		vty_out(vty,"<error> some radio not support rate %d.\n",rate);
	else if(ret == WLAN_BE_ENABLE)
		vty_out(vty,"<error> wlan is enable, please disable it first\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to some wlan, and the operation of the wlan was not successful\n");
	}
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}

/*wuwl add for REQUIREMENTS-340*/
DEFUN(set_wlan_not_response_sta_probe_request_func,
	  set_wlan_not_response_sta_probe_request_cmd,
	  "set wlan no response to sta probe request (enable|disable)",
	  CONFIG_STR
	  "wtp config\n"
	  "wtp no response to sta probe request switch\n"
	  "wtp no response to sta probe request switch\n"
	  "wtp no response to sta probe request switch\n"
	  "switch enable or disable\n"
	 )
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned int policy = 0;
	unsigned char wlanid = 0;
	int ret;
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
		wlanid = 0;
	}else if(vty->node == HANSI_NODE){
		index = vty->index; 		
		wlanid = 0;
		localid = vty->local;
        slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = vty->index; 		
		wlanid = 0;
		localid = vty->local;
        slot_id = vty->slotindex;
	}else if(vty->node == WLAN_NODE){
		index = 0; 		
		wlanid = (int)vty->index;;
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index; 		
		wlanid = (int)vty->index_sub;
		localid = vty->local;
        slot_id = vty->slotindex;
	}else if(vty->node == LOCAL_HANSI_WLAN_NODE){
		index = vty->index; 		
		wlanid = (int)vty->index_sub;
		localid = vty->local;
        slot_id = vty->slotindex;
	}
    DBusConnection *dcli_dbus_connection = NULL;
    ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WLAN_NO_RESPONSE_TO_STA_PROBLE_REQUEST);
																			
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&policy,
							 DBUS_TYPE_BYTE,&wlanid,
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
		if(wlanid != 0)
			vty_out(vty," set wlan %d no response to sta probe request %s successfully\n",wlanid,argv[0]);
		else
			vty_out(vty," set all wlan no response to sta probe request %s successfully\n",argv[0]);
	}
	else if(ret == WLAN_ID_NOT_EXIST)
		vty_out(vty,"<error> wlan %d does not exist\n",wlanid);
	else if(ret == WLAN_BE_ENABLE)
		vty_out(vty,"<error> wlan is enable, please disable it first\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to some wlan, and the operation of the wlan was not successful\n");
	}
	else
	{
		vty_out(vty,"<error>  %d\n",ret);
	}
		
	dbus_message_unref(reply);

	
	return CMD_SUCCESS;			
}
/*fengwenchao add 20120323*/
DEFUN(set_wlan_bss_allow_max_stanum_func,
	  set_wlan_bss_allow_max_stanum_cmd,
	  "set wlan bss allow max stanum STANUM",
	  "set wlan allow\n"
	  "set wlan allow\n"
	  "set wlan allow\n"
	  "max stanum\n"
	  "max stanum\n"
	  "stanum:0-128\n"
	 )
{
	int ret=0;
	DBusMessage *query = NULL, *reply = NULL;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;
	unsigned char wlanid =0;
	unsigned char type = 0;
	unsigned int wlanid_asd = 0;
	unsigned int wlan_num = 0;
	unsigned int wlan_asd_id_success[WLAN_NUM] ={0};
	unsigned int wlan_asd_id_success_num = 0;
	unsigned int wlan_asd_id_fail[WLAN_NUM] = {0};
	unsigned int wlan_asd_id_fail_num = 0;
	unsigned int wlan_wid_id_fail_num = 0;
	int stanum = 0;
	int i = 0;
	int flag = 0;
	int localid = 1;
	int slot_id = HostSlotId;	
	ret = parse_int_ID((char *)argv[0],&stanum);	//fengwenchao modify 20110512

	if (ret != WID_DBUS_SUCCESS)
	{	
		vty_out(vty,"<error> input parameter %s error\n",argv[0]);
		return CMD_SUCCESS;
	}

	if((stanum< 0)||(stanum > 128))	//fengwenchao modify 20110415
	{
		vty_out(vty,"<error> max station number should be  0 to 128\n");
		return CMD_SUCCESS;
	}

	dbus_error_init(&err);

	int index = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	if(vty->node == HANSI_NODE)
	{
		index = vty->index; 
		localid = vty->local;
		slot_id = vty->slotindex;		
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = vty->index; 		
		localid = vty->local;
		slot_id = vty->slotindex;		
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index; 		
		wlanid = (unsigned char)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;		
		type = 1;
	}else if (vty->node == LOCAL_HANSI_WLAN_NODE){
		index = vty->index;
		wlanid = (unsigned char)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;
		type = 1;
	}
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_GET_STA_INFO_NEW);
																	
	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&wlanid,
							 DBUS_TYPE_UINT32,&stanum,
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
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&wlan_num);
	//printf("wlan_num = %d \n",wlan_num);
	for(i =0; i < wlan_num; i++)
	{
		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&flag);

		dbus_message_iter_next(&iter);
		dbus_message_iter_get_basic(&iter,&wlanid_asd);
		//printf("flag  = %d \n",flag);
		//printf("wlanid_asd  = %d \n",wlanid_asd);
		if(flag == 1)
		{
			wlan_asd_id_fail[i] = wlanid_asd;
			wlan_asd_id_fail_num++;
		}
		else
		{
			wlan_asd_id_success[i] = wlanid_asd;
			wlan_asd_id_success_num++;
		}
	}
	
	dbus_message_unref(reply);
	
	if(ret == 0)
	{
		if(wlan_asd_id_fail_num != 0)
		{
			vty_out(vty,"<error> set max_sta_num fail  ");
			for(i = 0; i <wlan_asd_id_fail_num; i++)
			{
				if(i == (wlan_asd_id_fail_num-1))
					vty_out(vty,"%d\n",wlan_asd_id_fail[i]);
				else
					vty_out(vty,"%d,",wlan_asd_id_fail[i]);

			}
		}
		if(wlan_asd_id_success_num == 0)
			return CMD_SUCCESS;
	}
	else
	{
		vty_out(vty,"<error> wlan id is not exist \n");
		return CMD_SUCCESS;
	}

	memset(BUSNAME,0,PATH_LEN);	
	memset(OBJPATH,0,PATH_LEN);
	memset(INTERFACE,0,PATH_LEN);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_WLAN_MAX_STA_NEW);
	


	dbus_message_iter_init_append (query, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_BYTE,&type);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&stanum);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wlan_asd_id_success_num);
	
	for(i = 0; i < wlan_asd_id_success_num; i++)
	{
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&wlan_asd_id_success[i]);
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

	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,&wlan_wid_id_fail_num);

	if(wlan_wid_id_fail_num != 0)
	{
		vty_out(vty,"fail set  ");
		for(i=0;i<wlan_wid_id_fail_num;i++)
		{
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&wlanid);
			vty_out(vty,"%d ",wlanid);
		}
		vty_out(vty,"\n");
	}
	dbus_message_unref(reply);

	if(ret == 0)
		vty_out(vty,"set allow max sta num %d successful\n",stanum);
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete some wlan, and the operation of the wlan was not successful\n");
	}
	else
		vty_out(vty,"<error> %d\n",ret);
	
	return CMD_SUCCESS;
}

DEFUN(set_wlan_l2_isolation_func,
	  set_wlan_l2_isolation_cmd,
	  "set wlan l2 isolation (enable|disable)",
	  "wlan config\n"
	  "wlan ID \n"
	  "radio l2 isolation config\n"
	  "radio l2 isolation config\n"
	  "radio l2 isolation enable|disable\n"
	 )
{
	DBusMessage *query = NULL, *reply = NULL;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char wlanid = 0;
	int ret = WID_DBUS_SUCCESS;
	int policy = 0;
	int localid = 1;
	int slot_id = HostSlotId;

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
	if(vty->node == HANSI_NODE){
		index = vty->index;
		localid = vty->local;
		slot_id = vty->slotindex;			
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = vty->index;			
		localid = vty->local;
		slot_id = vty->slotindex;			
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index; 		
		wlanid = (unsigned char)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;			
	}else if(vty->node == LOCAL_HANSI_WLAN_NODE){
		index = vty->index; 		
		wlanid = (unsigned char)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;		
	}
	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_WLAN_L2_ISOLATION);
																	
	dbus_error_init(&err);


	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&wlanid,
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

	if(ret == 0)
		vty_out(vty,"set wlan l2 isolation %s successful\n",argv[0]);
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error> %d\n",ret);

	return CMD_SUCCESS;

}

DEFUN(set_wlan_bss_sta_static_arp_func,
	  set_wlan_bss_sta_static_arp_cmd,
	  "set wlan bss sta_static_arp (enable|disable) base IFNAME",
	  "set wlan bss\n"
	  "set wlan bss\n"
	  "set wlan bss\n"
	  "sta static arp config\n"
	  "enable/disable\n"
	  "base ifname\n"
	  "ifname (like: eth0-1)"
	 )
{
	DBusMessage *query = NULL, *reply = NULL;	
	DBusMessageIter	 iter;
	DBusError err;
	int policy = 0;
	int ret = 0;
	int index = 0;
	char *ifname = NULL;
	unsigned char wlanid = 0;
	int localid = 1;
	int slot_id = HostSlotId;	
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

	ifname = malloc(strlen(argv[1])+1);
	memset(ifname, 0, strlen(argv[1])+1);
	memcpy(ifname, argv[1], strlen(argv[1]));	
	
	if(vty->node == HANSI_NODE){
		index = vty->index; 
		localid = vty->local;
		slot_id = vty->slotindex;	
	}
	else if(vty->node == LOCAL_HANSI_NODE){
		index = vty->index; 
		localid = vty->local;
		slot_id = vty->slotindex;
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index; 		
		wlanid = (unsigned char)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;	
	}else if(vty->node == LOCAL_HANSI_WLAN_NODE){
		index = vty->index; 		
		wlanid = (unsigned char)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;		
	}	

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_WLAN_STA_STATIC_ARP);
																																		
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&wlanid,
							 DBUS_TYPE_UINT32,&policy,
							 DBUS_TYPE_STRING,&ifname,
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
	
	if(ifname){
	free(ifname);
	ifname = NULL;	
	}

	if(ret == 0)
		vty_out(vty,"set wlan bss sta_static_arp %s base %s successful\n",argv[0],argv[1]);
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error> %d\n",ret);
	
	return CMD_SUCCESS;


}

DEFUN(set_wlan_limit_rssi_access_sta_cmd_func,
	set_wlan_limit_rssi_access_sta_cmd,
	"set wlan access sta limit rssi RSSI",
	"set wlan limit rssi access sta\n"
	"wlan\n"
	"access sta\n"
	"access sta\n"
	"acess sta limit\n"
	"acess sta limit rssi\n"
	"rssi:range 0 to 95  0:means close this function\n"
	)
{
	DBusMessage *query = NULL, *reply = NULL;	
	DBusMessageIter	 iter;
	DBusError err;
	int policy = 0;
	int ret = 0;
	int index = 0;
	unsigned char rssi =0;
	unsigned char wlanid = 0;
	int localid = 1;
	int slot_id = HostSlotId;

	ret = parse_char_ID((char *)argv[0],&rssi);
	if(ret != WID_DBUS_SUCCESS){
		vty_out(vty,"<error> unknown id format\n");
		return CMD_SUCCESS;
	}	
	if(rssi > 95 ||rssi < 0){
		vty_out(vty,"<error> RSSI should be 0 to 95\n");
		return CMD_SUCCESS;
	}

	if(vty->node == HANSI_NODE){
		index = vty->index; 
		localid = vty->local;
		slot_id = vty->slotindex;		
	}else if(vty->node == LOCAL_HANSI_NODE){
		index = vty->index; 		
		localid = vty->local;
		slot_id = vty->slotindex;		
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index; 		
		wlanid = (unsigned char)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;		
	}else if(vty->node == LOCAL_HANSI_WLAN_NODE){
		index = vty->index; 		
		wlanid = (unsigned char)vty->index_sub;
		localid = vty->local;
		slot_id = vty->slotindex;		
	}	
	printf("wlanid =  %d \n",wlanid);
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];	
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_WLAN_ACCESS_STA_LIMIT_RSSI);
																																						
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&wlanid,
							 DBUS_TYPE_BYTE,&rssi,
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

	if(ret == 0)
		vty_out(vty,"set wlan access sta limit rssi %s\n",argv[0]);
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error> %d\n",ret);

	return CMD_SUCCESS;

}
/*fengwenchao add end*/
//weichao add 
DEFUN(set_wlan_bss_multi_user_optimize_cmd_func,
	set_wlan_bss_multi_user_optimize_cmd,
	"set wlan all bss  multi_user switch (enable|disable)",
	"Set command\n"
	"wlan\n"
	"Set all bss based wlan\n"
	"bss\n"
	"multi_user\n"
	"switch\n"
	"default disable"
)
{
	int ret = 0 ; 
	DBusMessage *query,*reply;
	DBusError err;
	DBusMessageIter iter;
	unsigned char type = 0 ; 
	unsigned char wlanid = 0 ;
	
	if(!strcmp(argv[0],"enable"))
	{
		type = 1; 
	}
	else if(!strcmp(argv[0],"disable"))
	{
		type = 0 ; 
	}
	dbus_error_init(&err);
	
	int index = 0;
	int localid = 1;  
	int slot_id = HostSlotId;   
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		wlanid = (unsigned char)vty->index;	
	}else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		wlanid = (unsigned char)vty->index_sub;	
		localid = vty->local; 
		slot_id = vty->slotindex; 
	}
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		wlanid = (unsigned char)vty->index_sub;	
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_WHOLE_WLAN_BSS_MULTI_USER_OPTIMIZE_SWITCH);
	
	dbus_message_append_args(query,
					DBUS_TYPE_BYTE,&wlanid,
					DBUS_TYPE_BYTE,&type,
					DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
		
	dbus_message_unref(query);
	
	if (NULL == reply) {
		vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return CMD_SUCCESS;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	if(WID_DBUS_SUCCESS == ret )
		vty_out(vty,"set bss wlan %d multi_user switch %s successfully!\n",wlanid,argv[0]);
	else if(ret==WLAN_ID_NOT_EXIST) 	
		vty_out(vty,"bss %d not exist .\n",wlanid);
	else if(ret==WID_DBUS_ERROR)
		vty_out(vty,"operation fail! .\n");
	else if(ret == Wlan_IF_NOT_BE_BINDED)	  
		vty_out(vty,"wlan is not binded radio .\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"<error>%d\n",ret);
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}
DEFUN(set_wlan_tunnel_mode_enable_cmd_func,
	  set_wlan_tunnel_mode_enable_cmd,
	  "set wlan forwarding mode (tunnel|local)",
	  SERVICE_STR
	  "enable or disable"
	 )
{	
	int ret = 0;
	unsigned char wlanid = 0;
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	unsigned char state = 0;	
	if(strncmp("tunnel",argv[0],strlen(argv[0])) == 0){
		state = 1;
	}else if(strncmp("local",argv[0],strlen(argv[0])) == 0){
		state = 0;
	}
	int index = 0;
	int localid = 1;
	int slot_id = HostSlotId;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	if(vty->node == WLAN_NODE){
		index = 0;
		wlanid = (unsigned char)vty->index; 
	}
	else if(vty->node == HANSI_WLAN_NODE){
		index = vty->index;			
		wlanid = (int)vty->index_sub;	
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	else if (vty->node == LOCAL_HANSI_WLAN_NODE)
	{
		index = vty->index;
		wlanid = (int)vty->index_sub;	
		localid = vty->local;
		slot_id = vty->slotindex;
	}
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_WLAN_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_METHOD_TUNNEL_NODE_SETTING);
	
	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_BYTE,&state,
							 DBUS_TYPE_BYTE,&wlanid,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,3000000, &err);
	
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
		vty_out(vty,"set wlan tunnel mode %s successfully\n",argv[0]);
	else if(ret == WLAN_BE_ENABLE)
		vty_out(vty,"set wlan tunnel mode %s failed! wlan is enable,please disable it first\n",argv[0]);
	else if(ret == WLAN_ID_NOT_EXIST) //fengwenchao add 20121203 for axsszfi-1283
		vty_out(vty,"<error>wlanid is not exist\n");
	else if (ret == WID_WANT_TO_DELETE_WLAN)		/* Huangleilei add for ASXXZFI-1622 */
	{
		vty_out(vty, "<warning> you want to delete wlan, please do not operate like this\n");
	}
	else
		vty_out(vty,"set wlan tunnel mode %s failed %d\n",argv[0],ret);		
	
	dbus_message_unref(reply);
	return CMD_SUCCESS;
}

int dcli_wlan_show_running_config_start(struct vty*vty) {	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	int res = 1;
	int slot_id = HostSlotId;
	int index = 0;
	int localid = 1;  //fengwenchao add 20110507
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	index = 0;
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WLAN_SHOW_RUNNING_CONFIG_START);
/*	query = dbus_message_new_method_call(
							WID_DBUS_BUSNAME,		\
							WID_DBUS_OBJPATH , \
							WID_DBUS_INTERFACE ,		\
							WID_DBUS_CONF_METHOD_WLAN_SHOW_RUNNING_CONFIG_START);*/

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err);//fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show wlan config failed get reply.\n");
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
		char _tmpstr[64];
		memset(_tmpstr,0,64);
		sprintf(_tmpstr,BUILDING_MOUDLE,"WLAN1");
		vtysh_add_show_string(_tmpstr);
		vtysh_add_show_string(showStr);		
		dcli_config_write(showStr,1,slot_id,index,0,0);
		dbus_message_unref(reply);
		res=0;
	} 
	else 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return 1;
	}
	
	res = dcli_wlan_list_show_running_config(vty); 

	return res;	
}
extern struct vtysh_client vtysh_client[];

int dcli_wlan_show_running_config_end(struct vty*vty) {	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	int slot_id = HostSlotId;	
	int index = 0;
	int localid= 1;   //fengwenchao add 20110507
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	index = 0;
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WLAN_SHOW_RUNNING_CONFIG_END);
/*	query = dbus_message_new_method_call(
							WID_DBUS_BUSNAME,		\
							WID_DBUS_OBJPATH , \
							WID_DBUS_INTERFACE ,		\
							WID_DBUS_CONF_METHOD_WLAN_SHOW_RUNNING_CONFIG_END);*/

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err); //fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show wlan config failed get reply.\n");
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
		char _tmpstr[64];
		memset(_tmpstr,0,64);
		sprintf(_tmpstr,BUILDING_MOUDLE,"WLAN2");
		vtysh_add_show_string(_tmpstr);
		vtysh_add_show_string(showStr);
		dcli_config_write(showStr,1,slot_id,index,0,0);
		dbus_message_unref(reply);
		return 0;	
	} 
	else 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return 1;	
	}
  return 0;
}


char* dcli_hansi_wlan_show_running_config_start(int localid ,int slot_id,int index) {	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	int res = 1;
	char *tmp;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WLAN_SHOW_RUNNING_CONFIG_START);
/*	query = dbus_message_new_method_call(
							WID_DBUS_BUSNAME,		\
							WID_DBUS_OBJPATH , \
							WID_DBUS_INTERFACE ,		\
							WID_DBUS_CONF_METHOD_WLAN_SHOW_RUNNING_CONFIG_START);*/

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err); //fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show wlan config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) 
	{
		
		tmp = (char *)malloc(strlen(showStr)+1);
		memset(tmp, 0, strlen(showStr)+1);
		memcpy(tmp,showStr,strlen(showStr));			
//		dcli_config_write(showStr,localid,slot_id,index,0,0);
		dbus_message_unref(reply);
		return tmp;	
	} 
	else 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return NULL;
	}
	
	return NULL;	
}

int dcli_hansi_wlan_show_running_config_end(int localid, int slot_id,int index) {	
	char *showStr = NULL,*cursor = NULL,ch = 0,tmpBuf[SHOWRUN_PERLINE_SIZE] = {0};
	DBusMessage *query, *reply;
	DBusError err;
	char *tmp;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	DBusConnection *dcli_dbus_connection = NULL;
	ReInitDbusConnection(&dcli_dbus_connection,slot_id,distributFag);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_CONF_METHOD_WLAN_SHOW_RUNNING_CONFIG_END);
/*	query = dbus_message_new_method_call(
							WID_DBUS_BUSNAME,		\
							WID_DBUS_OBJPATH , \
							WID_DBUS_INTERFACE ,		\
							WID_DBUS_CONF_METHOD_WLAN_SHOW_RUNNING_CONFIG_END);*/

	dbus_error_init(&err);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,300000, &err); //fengwenchao change "-1" to 300000 for TESTBED-7,20111213

	dbus_message_unref(query);
	if (NULL == reply) {
		printf("show wlan config failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return NULL;
	}

	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_STRING, &showStr,
					DBUS_TYPE_INVALID)) 
	{
		tmp = (char *)malloc(strlen(showStr)+1);
		memset(tmp, 0, strlen(showStr)+1);
		memcpy(tmp,showStr,strlen(showStr));		
//		dcli_config_write(showStr,localid,slot_id,index,0,0);
		dbus_message_unref(reply);
		return tmp;	
	} 
	else 
	{
		printf("Failed get args.\n");
		if (dbus_error_is_set(&err)) 
		{
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		dbus_message_unref(reply);
		return NULL;	
	}
  return NULL;
}



void dcli_wlan_init(void) {
	install_element(VIEW_NODE,&show_wlan_cmd);
	install_element(VIEW_NODE,&show_wlan_list_cmd);
	install_element(VIEW_NODE,&show_all_wlan_ssid_config_information_cmd);
#if 0
	install_node(&wlan_node,dcli_wlan_show_running_config_start);
	install_node(&wlan_node1,dcli_wlan_show_running_config_end);

	install_default(WLAN_NODE);

	/**********************************************VIEW_NODE**************************************************/
	install_element(VIEW_NODE,&show_wlan_list_cmd);															/*a1*/
	install_element(VIEW_NODE,&show_wlan_cmd);																/*a2*/
	/*====================for mib begin====================*/
	install_element(VIEW_NODE,&show_all_wlan_wapi_basic_information_cmd);/*nl add 20100521*/						/*b1*/
	install_element(VIEW_NODE,&show_all_wlan_wapi_performance_stats_information_cmd);/*nl add 20100531*/			/*b2*/
	install_element(VIEW_NODE,&show_all_wlan_wapi_extend_config_information_cmd);/*nl add 20100601*/				/*b3*/
	install_element(VIEW_NODE,&show_all_wlan_bss_wapi_performance_stats_information_cmd);/*nl add 20100604*/		/*b4*/
	install_element(VIEW_NODE,&show_all_wlan_unicast_information_cmd);/*nl add 20100526*/							/*b5*/
	install_element(VIEW_NODE,&show_all_wlan_ssid_config_information_cmd);/*lzh add*/								/*b6*/
	/*====================for mib end====================*/
	
	/**********************************************ENABLE_NODE**************************************************/
	install_element(ENABLE_NODE,&show_wlan_list_cmd);															/*a1*/
	install_element(ENABLE_NODE,&show_wlan_cmd);																/*a2*/
	/*	install_element(ENABLE_NODE,&create_wlan_cmd);*/
	/*====================for mib begin====================*/
	install_element(ENABLE_NODE,&show_all_wlan_wapi_basic_information_cmd);/*nl add 20100521*/						/*b1*/
	install_element(ENABLE_NODE,&show_all_wlan_wapi_performance_stats_information_cmd);/*nl add 20100531*/			/*b2*/
	install_element(ENABLE_NODE,&show_all_wlan_wapi_extend_config_information_cmd);/*nl add 20100601*/				/*b3*/
	install_element(ENABLE_NODE,&show_all_wlan_bss_wapi_performance_stats_information_cmd);/*nl add 20100604*/		/*b4*/
	install_element(ENABLE_NODE,&show_all_wlan_unicast_information_cmd);/*nl add 20100526*/							/*b5*/
	install_element(ENABLE_NODE,&show_all_wlan_ssid_config_information_cmd);/*lzh add*/								/*b6*/
	/*====================for mib end====================*/
	/**********************************************CONFIG_NODE**************************************************/
	install_element(CONFIG_NODE,&show_wlan_cmd);																/*a1*/
	install_element(CONFIG_NODE,&show_wlan_list_cmd);															/*a2*/
	/*====================for mib begin====================*/
	install_element(CONFIG_NODE,&show_all_wlan_wapi_basic_information_cmd);/*nl add 20100521*/						/*b1*/
	install_element(CONFIG_NODE,&show_all_wlan_wapi_performance_stats_information_cmd);/*nl add 20100531*/			/*b2*/
	install_element(CONFIG_NODE,&show_all_wlan_wapi_extend_config_information_cmd);/*nl add 20100601*/				/*b3*/
	install_element(CONFIG_NODE,&show_all_wlan_bss_wapi_performance_stats_information_cmd);/*nl add 20100604*/		/*b4*/
	install_element(CONFIG_NODE,&show_all_wlan_unicast_information_cmd);/*nl add 20100526*/							/*b5*/
	install_element(CONFIG_NODE,&show_all_wlan_ssid_config_information_cmd);/*lzh add*/								/*b6*/
	/*====================for mib end====================*/
	install_element(CONFIG_NODE,&create_wlan_cmd);	
	install_element(CONFIG_NODE,&create_wlan_cmd_cn);	
	install_element(CONFIG_NODE,&delete_wlan_cmd);
	install_element(CONFIG_NODE,&config_wlan_cmd);
	install_element(CONFIG_NODE,&show_wlan_of_all_cmd);     //fengwenchao add 20101223
	/****************************************************WLAN_NODE**************************************************/
	install_element(WLAN_NODE,&show_wlan_vlan_info_cmd);
	install_element(WLAN_NODE,&wlan_show_bridge_isolation_cmd);
	install_element(WLAN_NODE,&show_tunnel_wlan_vlan_cmd);
	install_element(WLAN_NODE,&config_wlan_service_cmd);	
	install_element(WLAN_NODE,&config_flow_check_cmd);		
	install_element(WLAN_NODE,&set_no_flow_time_cmd);
	install_element(WLAN_NODE,&set_limit_min_flow_cmd);
	install_element(WLAN_NODE,&config_wds_service_cmd);	
	install_element(WLAN_NODE,&wlan_apply_security_cmd);
	install_element(WLAN_NODE,&wlan_apply_interface_cmd);
	//install_element(WLAN_NODE,&wlan_apply_ipv6interface_cmd);
	install_element(WLAN_NODE,&wlan_delete_interface_cmd);/*20080701*/
	install_element(WLAN_NODE,&wlan_hideessid_cmd);/*20080701*/
	/*install_element(WLAN_NODE,&wlan_map_interface_cmd);
	//install_element(WLAN_NODE,&wlan_unmap_interface_cmd);
	//install_element(WLAN_NODE,&wlan_bss_map_interface_cmd);
	//install_element(WLAN_NODE,&wlan_bss_unmap_interface_cmd);*/
	
	install_element(WLAN_NODE,&wlan_max_num_cmd);/*xm add 08/12/03*/
	install_element(WLAN_NODE,&wlan_load_balance_cmd);/*xm add 08/12/03*/
	install_element(WLAN_NODE,&set_interface_nasid_cmd);/*zhanglei add*/
	install_element(WLAN_NODE,&remove_interface_nasid_cmd);
	
	install_element(WLAN_NODE,&wlan_balance_parameter_cmd);/*xm add 08/12/29*/
	install_element(WLAN_NODE,&wlan_flow_balance_parameter_cmd);/*xm add 09/02/05*/
	install_element(WLAN_NODE,&wlan_set_eap_mac_cmd);/*zhangshu add 2010-10-22*/
	install_element(WLAN_NODE,&wlan_roam_policy_cmd);/*zhanglei*/
	install_element(WLAN_NODE,&set_wlan_vlan_cmd);
	install_element(WLAN_NODE,&undo_wlan_vlan_cmd);
	install_element(WLAN_NODE,&set_wlan_vlan_priority_cmd);

	install_element(WLAN_NODE,&set_whole_wlan_traffic_limit_cmd);
	install_element(WLAN_NODE,&set_whole_wlan_send_traffic_limit_cmd);
	install_element(WLAN_NODE,&set_whole_wlan_station_average_traffic_limit_cmd);
	install_element(WLAN_NODE,&set_whole_wlan_station_average_send_traffic_limit_cmd);
	install_element(WLAN_NODE,&wlan_set_bridge_isolation_cmd);
	install_element(WLAN_NODE,&wlan_set_bridge_multicast_isolation_cmd);
	install_element(WLAN_NODE,&wlan_set_sameportswitch_cmd);

	install_element(WLAN_NODE,&set_tunnel_wlan_vlan_cmd);
	install_element(WLAN_NODE,&set_wlan_essid_cmd);
	install_element(WLAN_NODE,&set_wlan_ascii_essid_cmd);     /*fengwenchao add 20110307*/
	install_element(WLAN_NODE,&set_wlan_sta_ip_mac_binding_cmd);
	install_element(WLAN_NODE,&set_wlan_servive_timer_func_cmd);
	install_element(WLAN_NODE,&set_wlan_timer_able_cmd);
	install_element(WLAN_NODE,&show_wlan_ptk_info_cmd);
	install_element(WLAN_NODE,&set_nas_port_id_cmd);			//mahz add 2011.5.25

#endif
	
	
	/*****************************************HANSI_NODE****************************************************/
	install_node(&hansi_wlan_node,NULL,"HANSI_WLAN_NODE");
	install_default(HANSI_WLAN_NODE);
		
	install_element(HANSI_NODE,&show_wlan_cmd);	
	install_element(HANSI_NODE,&show_wlan_list_cmd);
	install_element(HANSI_NODE,&create_wlan_cmd);	
	install_element(HANSI_NODE,&create_wlan_cmd_cn);
	install_element(HANSI_NODE,&delete_wlan_cmd);
	install_element(HANSI_NODE,&config_wlan_cmd);
	/*====================for mib begin====================*/
	install_element(HANSI_NODE,&show_all_wlan_wapi_basic_information_cmd);/*nl add 20100521*/						/*b1*/
	install_element(HANSI_NODE,&show_all_wlan_wapi_performance_stats_information_cmd);/*nl add 20100531*/			/*b2*/
	install_element(HANSI_NODE,&show_all_wlan_wapi_extend_config_information_cmd);/*nl add 20100601*/				/*b3*/
	install_element(HANSI_NODE,&show_all_wlan_bss_wapi_performance_stats_information_cmd);/*nl add 20100604*/		/*b4*/
	install_element(HANSI_NODE,&show_all_wlan_unicast_information_cmd);/*nl add 20100526*/						/*b5*/
	install_element(HANSI_NODE,&show_all_wlan_ssid_config_information_cmd);/*lzh add*/ 							/*b6*/
	install_element(HANSI_NODE,&show_wlan_of_all_cmd);     //fengwenchao add 20101223
	install_element(HANSI_NODE,&set_wlan_uni_muti_bro_cast_isolation_set_cmd);
	install_element(HANSI_NODE,&set_wlan_muti_bro_cast_rate_set_cmd);
	install_element(HANSI_NODE,&set_wlan_not_response_sta_probe_request_cmd);
	/*====================for mib end====================*/
	
	install_element(HANSI_NODE,&set_wlan_bss_allow_max_stanum_cmd);//fengwenchao add 20120323
	install_element(HANSI_NODE,&set_wlan_l2_isolation_cmd);//fengwenchao add 20120323
	install_element(HANSI_NODE,&set_wlan_bss_sta_static_arp_cmd);//fengwenchao add 20120323
	install_element(HANSI_NODE,&set_wlan_limit_rssi_access_sta_cmd);//fengwenchao add 20120323
	/*****************************************HANSI_WLAN_NODE****************************************************/
	install_element(HANSI_WLAN_NODE,&config_wlan_service_cmd);	
	install_element(HANSI_WLAN_NODE,&config_wds_service_cmd); 
	install_element(HANSI_WLAN_NODE,&wlan_apply_security_cmd);
	install_element(HANSI_WLAN_NODE,&wlan_apply_interface_cmd);
	install_element(HANSI_WLAN_NODE,&wlan_delete_interface_cmd);
	install_element(HANSI_WLAN_NODE,&wlan_hideessid_cmd);
	install_element(HANSI_WLAN_NODE,&wlan_max_num_cmd);
	install_element(HANSI_WLAN_NODE,&wlan_load_balance_cmd);
	install_element(HANSI_WLAN_NODE,&set_interface_nasid_cmd);
	install_element(HANSI_WLAN_NODE,&remove_interface_nasid_cmd);
	
	install_element(HANSI_WLAN_NODE,&config_flow_check_cmd);
	install_element(HANSI_WLAN_NODE,&set_no_flow_time_cmd);
	install_element(HANSI_WLAN_NODE,&set_limit_min_flow_cmd);
	install_element(HANSI_WLAN_NODE,&wlan_balance_parameter_cmd);
	install_element(HANSI_WLAN_NODE,&wlan_flow_balance_parameter_cmd); 		
	install_element(HANSI_WLAN_NODE,&wlan_set_eap_mac_cmd);/*zhangshu add 2010-10-22*/
	install_element(HANSI_WLAN_NODE,&wlan_roam_policy_cmd);
	install_element(HANSI_WLAN_NODE,&set_wlan_vlan_cmd);
	install_element(HANSI_WLAN_NODE,&undo_wlan_vlan_cmd);
	//install_element(HANSI_WLAN_NODE,&set_wlan_vlan_priority_cmd);
	install_element(HANSI_WLAN_NODE,&show_wlan_vlan_info_cmd);
	install_element(HANSI_WLAN_NODE,&wlan_show_bridge_isolation_cmd);
	install_element(HANSI_WLAN_NODE,&show_tunnel_wlan_vlan_cmd);
	install_element(HANSI_WLAN_NODE,&show_all_wlan_ssid_config_information_cmd);
	install_element(HANSI_WLAN_NODE,&set_whole_wlan_traffic_limit_cmd);
	install_element(HANSI_WLAN_NODE,&set_whole_wlan_send_traffic_limit_cmd);
	
	install_element(HANSI_WLAN_NODE,&set_whole_wlan_station_average_traffic_limit_cmd);
	install_element(HANSI_WLAN_NODE,&set_whole_wlan_station_average_send_traffic_limit_cmd);
	install_element(HANSI_WLAN_NODE,&wlan_set_bridge_isolation_cmd);
	install_element(HANSI_WLAN_NODE,&wlan_set_bridge_multicast_isolation_cmd);
	install_element(HANSI_WLAN_NODE,&wlan_set_sameportswitch_cmd);
	install_element(HANSI_WLAN_NODE,&set_tunnel_wlan_vlan_cmd);
	install_element(HANSI_WLAN_NODE,&set_wlan_essid_cmd);
	install_element(HANSI_WLAN_NODE,&set_wlan_ascii_essid_cmd);     /*fengwenchao add 20110307*/	
	install_element(HANSI_WLAN_NODE,&wlan_apply_ipv6interface_cmd);
	install_element(HANSI_WLAN_NODE,&set_wlan_sta_ip_mac_binding_cmd);
	install_element(HANSI_WLAN_NODE,&set_wlan_servive_timer_func_cmd);
	install_element(HANSI_WLAN_NODE,&set_wlan_timer_able_cmd);
	install_element(HANSI_WLAN_NODE,&set_wlan_uni_muti_bro_cast_isolation_set_cmd);
	install_element(HANSI_WLAN_NODE,&set_wlan_muti_bro_cast_rate_set_cmd);
	install_element(HANSI_WLAN_NODE,&set_wlan_not_response_sta_probe_request_cmd);
	install_element(HANSI_WLAN_NODE,&show_wlan_ptk_info_cmd);
	install_element(HANSI_WLAN_NODE,&set_nas_port_id_cmd);			//mahz add 2011.5.25
	install_element(HANSI_WLAN_NODE,&set_wlan_bss_allow_max_stanum_cmd);//fengwenchao add 20120323
	install_element(HANSI_WLAN_NODE,&set_wlan_l2_isolation_cmd);//fengwenchao add 20120323
	install_element(HANSI_WLAN_NODE,&set_wlan_bss_sta_static_arp_cmd);//fengwenchao add 20120323
	install_element(HANSI_WLAN_NODE,&set_wlan_limit_rssi_access_sta_cmd);//fengwenchao add 20120323
	install_element(HANSI_WLAN_NODE,&set_wlan_hotspot_id_cmd);		
	install_element(HANSI_WLAN_NODE,&clean_wlan_hotspot_id_cmd);		
	install_element(HANSI_WLAN_NODE,&set_wlan_bss_multi_user_optimize_cmd);		
	install_element(HANSI_WLAN_NODE,&set_wlan_tunnel_mode_enable_cmd);
/*fengwenchao add 20110507*/
/****************************************LOCAL_HANSI_NODE**********************************/
install_node(&local_hansi_wlan_node,NULL,"LOCAL_HANSI_WLAN_NODE");
install_default(LOCAL_HANSI_WLAN_NODE);
	
install_element(LOCAL_HANSI_NODE,&show_wlan_cmd); 
install_element(LOCAL_HANSI_NODE,&show_wlan_list_cmd);
install_element(LOCAL_HANSI_NODE,&create_wlan_cmd);	
install_element(LOCAL_HANSI_NODE,&create_wlan_cmd_cn);
install_element(LOCAL_HANSI_NODE,&delete_wlan_cmd);
install_element(LOCAL_HANSI_NODE,&config_wlan_cmd);
/*====================for mib begin====================*/
install_element(LOCAL_HANSI_NODE,&show_all_wlan_wapi_basic_information_cmd);/*nl add 20100521*/						/*b1*/
install_element(LOCAL_HANSI_NODE,&show_all_wlan_wapi_performance_stats_information_cmd);/*nl add 20100531*/			/*b2*/
install_element(LOCAL_HANSI_NODE,&show_all_wlan_wapi_extend_config_information_cmd);/*nl add 20100601*/				/*b3*/
install_element(LOCAL_HANSI_NODE,&show_all_wlan_bss_wapi_performance_stats_information_cmd);/*nl add 20100604*/		/*b4*/
install_element(LOCAL_HANSI_NODE,&show_all_wlan_unicast_information_cmd);/*nl add 20100526*/						/*b5*/
install_element(LOCAL_HANSI_NODE,&show_all_wlan_ssid_config_information_cmd);/*lzh add*/							/*b6*/
install_element(LOCAL_HANSI_NODE,&show_wlan_of_all_cmd);	   //fengwenchao add 20101223
install_element(LOCAL_HANSI_NODE,&set_wlan_bss_allow_max_stanum_cmd);//fengwenchao add 20120323
install_element(LOCAL_HANSI_NODE,&set_wlan_l2_isolation_cmd);//fengwenchao add 20120323
install_element(LOCAL_HANSI_NODE,&set_wlan_bss_sta_static_arp_cmd);//fengwenchao add 20120323
install_element(LOCAL_HANSI_NODE,&set_wlan_limit_rssi_access_sta_cmd);//fengwenchao add 20120323
install_element(LOCAL_HANSI_NODE,&set_wlan_uni_muti_bro_cast_isolation_set_cmd);
install_element(LOCAL_HANSI_NODE,&set_wlan_muti_bro_cast_rate_set_cmd);
install_element(LOCAL_HANSI_NODE,&set_wlan_not_response_sta_probe_request_cmd);
/*====================for mib end====================*/

/****************************************LOCAL_HANSI_WLAN_NODE****************************/
install_element(LOCAL_HANSI_WLAN_NODE,&config_wlan_service_cmd);	
install_element(LOCAL_HANSI_WLAN_NODE,&config_wds_service_cmd); 
install_element(LOCAL_HANSI_WLAN_NODE,&wlan_apply_security_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&wlan_apply_interface_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&wlan_delete_interface_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&wlan_hideessid_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&wlan_max_num_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&wlan_load_balance_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&set_interface_nasid_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&remove_interface_nasid_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&config_flow_check_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&set_no_flow_time_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&set_limit_min_flow_cmd);

install_element(LOCAL_HANSI_WLAN_NODE,&wlan_balance_parameter_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&wlan_flow_balance_parameter_cmd);		
install_element(LOCAL_HANSI_WLAN_NODE,&wlan_set_eap_mac_cmd);/*zhangshu add 2010-10-22*/
install_element(LOCAL_HANSI_WLAN_NODE,&wlan_roam_policy_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&set_wlan_vlan_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&undo_wlan_vlan_cmd);
//install_element(LOCAL_HANSI_WLAN_NODE,&show_wlan_vlan_info_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&wlan_show_bridge_isolation_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&show_tunnel_wlan_vlan_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&show_all_wlan_ssid_config_information_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&set_whole_wlan_traffic_limit_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&set_whole_wlan_send_traffic_limit_cmd);

install_element(LOCAL_HANSI_WLAN_NODE,&set_whole_wlan_station_average_traffic_limit_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&set_whole_wlan_station_average_send_traffic_limit_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&wlan_set_bridge_isolation_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&wlan_set_bridge_multicast_isolation_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&wlan_set_sameportswitch_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&set_tunnel_wlan_vlan_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&set_wlan_essid_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&set_wlan_ascii_essid_cmd); 	/*fengwenchao add 20110307*/	
install_element(LOCAL_HANSI_WLAN_NODE,&wlan_apply_ipv6interface_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&set_wlan_sta_ip_mac_binding_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&set_wlan_servive_timer_func_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&set_wlan_timer_able_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&show_wlan_ptk_info_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&set_wlan_hotspot_id_cmd);		
install_element(LOCAL_HANSI_WLAN_NODE,&clean_wlan_hotspot_id_cmd);		
install_element(LOCAL_HANSI_WLAN_NODE,&set_wlan_bss_allow_max_stanum_cmd);//fengwenchao add 20120323
install_element(LOCAL_HANSI_WLAN_NODE,&set_wlan_l2_isolation_cmd);//fengwenchao add 20120323
install_element(LOCAL_HANSI_WLAN_NODE,&set_wlan_bss_sta_static_arp_cmd);//fengwenchao add 20120323
install_element(LOCAL_HANSI_WLAN_NODE,&set_wlan_limit_rssi_access_sta_cmd);//fengwenchao add 20120323
install_element(LOCAL_HANSI_WLAN_NODE,&set_wlan_bss_multi_user_optimize_cmd); 	

/*fengwenchao add end*/
install_element(LOCAL_HANSI_WLAN_NODE,&set_wlan_uni_muti_bro_cast_isolation_set_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&set_wlan_muti_bro_cast_rate_set_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&set_wlan_not_response_sta_probe_request_cmd);
install_element(LOCAL_HANSI_WLAN_NODE,&set_wlan_tunnel_mode_enable_cmd);
	return;
}

#endif
