#ifdef _D_WCPSS_
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <dbus/dbus.h>
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "wcpss/asd/asd.h"
#include "wid_wqos.h"
int wid_wqos_parse_char_ID(char* str,unsigned char* ID)
{
	char *endptr = NULL;
	char c;
	c = str[0];
	if (c>='0'&&c<='9')
	{
		*ID= strtoul(str,&endptr,10);
		if(endptr[0] == '\0')
		{
			return WID_DBUS_SUCCESS;
		}
		else
		{
			return WID_UNKNOWN_ID;
		}
	}
	else
	{
		return WID_UNKNOWN_ID;
	}
}
int wid_wqos_parse_short_ID(char* str,unsigned short* ID)
{
	char *endptr = NULL;
	char c;
	c = str[0];
	if (c>='0'&&c<='9')
	{
		*ID= strtoul(str,&endptr,10);
		if(endptr[0] == '\0')
		{
			return WID_DBUS_SUCCESS;
		}
		else
		{
			return WID_UNKNOWN_ID;
		}
	}
	else
	{
		return WID_UNKNOWN_ID;
	}
}

int parse_dot1p_list(char* ptr,unsigned char* count,unsigned char dot1plist[])
{
	char* endPtr = NULL;
	int   i=0;
	endPtr = ptr;
	dot1p_list_state state = qos_check_dot1p_state;

	while(1){
		switch(state){
			
		case qos_check_dot1p_state: 
				dot1plist[i] = strtoul(endPtr,&endPtr,10);
				
				if((dot1plist[i])<8){
            		state=qos_check_comma_state;
				}
				else
					state=qos_check_fail_state;
				
				break;
		
		case qos_check_comma_state:
			if (QOS_DOT1P_COMMA == endPtr[0]){
				endPtr = (char*)endPtr + 1;
				state = qos_check_dot1p_state;
				
				i++;
				}
			else
				state = qos_check_end_state;
			break;
				
		
		case qos_check_fail_state:
			
			return -1;
			break;

		case qos_check_end_state: 
			
			if ('\0' == endPtr[0]) {
				state = qos_check_success_state;
				i++;
				}
			else
				state = qos_check_fail_state;
				break;
			
		case qos_check_success_state: 
			
			
			*count = i;
			return 0;
			break;
			
		default: break;
		}
		
		}
		
}
unsigned char QosRemoveListRepId(unsigned char list[],unsigned char num)

{
	int i,j,k;
	for(i=0;i<num;i++){ 
        for(j=i+1;j<num;j++)  { 
              if(list[i]==list[j])  { 
                  num--;
                  for(k=j;k<num;k++) { 
                       list[k]=list[k+1]; 
                  } 
                  j--; 
               } 
         } 
     } 
     for(i=0;i<num;i++){
	 /*//printf("%d",list[i]);
     ////printf("\n");*/
	 }
     return num; 

}
int parse_qos_flow_parameter_type(char* str,unsigned int type){

	
	if ((!strcmp(str,"averagerate"))||(!strcmp(str,"AVERAGERATE")))
	{
		type = averagerate_type;
		return WID_DBUS_SUCCESS;
	}
	else if ((!strcmp(str,"maxburstiness"))||(!strcmp(str,"MAXBURSTINESS")))
	{
		type = max_burstiness_type;
		return WID_DBUS_SUCCESS;
	}
	else if ((!strcmp(str,"managepriority"))||(!strcmp(str,"MANAGEPRIORITY")))
	{
		type = manage_priority_type;
		return WID_DBUS_SUCCESS;
	}
	else if ((!strcmp(str,"shovepriority"))||(!strcmp(str,"SHOVEPRIORITY")))
	{
		type = shove_priority_type;
		return WID_DBUS_SUCCESS;
	}
	else if ((!strcmp(str,"grabpriority"))||(!strcmp(str,"GRABPRIORITY")))
	{
		type = grab_priority_type;
		return WID_DBUS_SUCCESS;
	}
	else
	{
		type = unkonwn_type;
		return WID_DBUS_ERROR;
	}
}
int parse_qos_parameter_type(char* str,unsigned int type){

	
	if ((!strcmp(str,"totalbandwidth"))||(!strcmp(str,"TOTALBANDWIDTH")))
	{
		type = totalbandwidth_type;
		return WID_DBUS_SUCCESS;
	}
	else if ((!strcmp(str,"resourcescale"))||(!strcmp(str,"RESOURCESCALE")))
	{
		type = resourcescale_type;
		return WID_DBUS_SUCCESS;
	}
	else if ((!strcmp(str,"sharebandwidth"))||(!strcmp(str,"SHAREBANDWIDTH")))
	{
		type = sharebandwidth_type;
		return WID_DBUS_SUCCESS;
	}
	else if ((!strcmp(str,"resourcesharescale"))||(!strcmp(str,"RESOURCESHARESCALE")))
	{
		type = resourcesharescale_type;
		return WID_DBUS_SUCCESS;
	}
	else
	{
		type = qos_unkonwn_type;
		return WID_DBUS_ERROR;
	}
}

int parse_wlan_ifname(char* ptr,int *vrrid,char *wlanid)
{
	
    radio_ifname_state state = check_vrrip;
	char *str = NULL;
	str = ptr;
   
	while(1){
		switch(state){
			
		case check_vrrip: 
			
			*vrrid = strtoul(str,&str,10);
			
			if(*vrrid >= 0 && *vrrid < 16){
        		state=check_sub;
			}
			else state=check_fail;
			
			break;

		case check_sub: 
		
			if (PARSE_RADIO_IFNAME_SUB == str[0]){
		
				state = check_wlanid;
				}
			else
				state = check_fail;
			break;

		case check_wlanid: 
		
			*wlanid = strtoul((char *)&(str[1]),&str,10);

			if(*wlanid > 0 && *wlanid < 128){/*radioid range 0 1 2 3*/
        		state=check_end;
			}
			else state=check_fail;
			
			break;		
		
		case check_fail:
	
		
            return -1;
			break;

		case check_end: 
	
			if ('\0' == str[0]) {
				state = check_success;
				
				}
			else
				state = check_fail;
				break;
			
		case check_success: 
		
			return 0;
			break;
			
		default: break;
		}
		
		}
		
}

int parse_wlan_ifname_v2(char* ptr,int *vrrid,int *wlanid)
{
	
    radio_ifname_state state = check_vrrip;
	char *str = NULL;
	str = ptr;
   
	while(1){
		switch(state){
			
		case check_vrrip: 
			
			*vrrid = strtoul(str,&str,10);
			
			if(*vrrid >= 0 && *vrrid < 16){
        		state=check_sub;
			}
			else state=check_fail;
			
			break;

		case check_sub: 
		
			if (PARSE_RADIO_IFNAME_SUB == str[0]){
		
				state = check_wlanid;
				}
			else
				state = check_fail;
			break;

		case check_wlanid: 
		
			*wlanid = strtoul((char *)&(str[1]),&str,10);

			if(*wlanid > 0 && *wlanid <= 1024){
        		state=check_end;
			}
			else state=check_fail;
			
			break;		
		
		case check_fail:
	
		
            return -1;
			break;

		case check_end: 
	
			if ('\0' == str[0]) {
				state = check_success;
				
				}
			else
				state = check_fail;
				break;
			
		case check_success: 
		
			return 0;
			break;
			
		default: break;
		}
		
		}
		
}

/*for dcli_intf.c*/
int parse_radio_ifname(char* ptr,int *wtpid,int *radioid,int *wlanid)
{
	
    radio_ifname_state state = check_wtpid;
	char *str = NULL;
	str = ptr;
   
	while(1){
		switch(state){
			
		case check_wtpid: 
			
			*wtpid = strtoul(str,&str,10);
			
			if(*wtpid > 0 && *wtpid <= 4096){
        		state=check_sub;
			}
			else state=check_fail;
			
			break;

		case check_sub: 
		
			if (PARSE_RADIO_IFNAME_SUB == str[0]){
		
				state = check_radioid;
				}
			else
				state = check_fail;
			break;

		case check_radioid: 
		
			*radioid = strtoul((char *)&(str[1]),&str,10);

			if(*radioid >= 0 && *radioid < 4){/*radioid range 0 1 2 3*/
        		state=check_point;
			}
			else state=check_fail;
			
			break;

		case check_point: 
		
			if (PARSE_RADIO_IFNAME_POINT == str[0]){
			
				state = check_wlanid;
				
				}
			else
				state = check_fail;
			break;
				
		case check_wlanid: 
		
			*wlanid = strtoul((char *)&(str[1]),&str,10);

			if(*wlanid > 0 && *wlanid < WLAN_NUM+1){
        		state=check_end;
			}
			else state=check_fail;
			
			break;
			
		
		
		case check_fail:
	
		
            return -1;
			break;

		case check_end: 
	
			if ('\0' == str[0]) {
				state = check_success;
				
				}
			else
				state = check_fail;
				break;
			
		case check_success: 
		
			return 0;
			break;
			
		default: break;
		}
		
		}
		
}

int parse_radio_ifname_v2(char* ptr,int *wtpid,int *radioid,int *wlanid,int *vrrid)
{
	
    radio_ifname_state state = check_vrrip;
	char *str = NULL;
	str = ptr;
   
	while(1){
		switch(state){
			
		case check_vrrip: 
			*vrrid = strtoul(str,&str,10);
			if(*vrrid >=0 && *vrrid <= 16){
        		state=check_sub;
			}
			else state=check_fail;
			break;
		case check_wtpid: 
			*wtpid = strtoul((char *)&str[1],&str,10);
			if(*wtpid > 0 && *wtpid <= 4096){
        		state=check_sub2;
			}
			else state=check_fail;
			
			break;
		case check_sub: 
		
			if (PARSE_RADIO_IFNAME_SUB == str[0]){
		
				state = check_wtpid;
			}
			else
				state = check_fail;
			break;

		case check_sub2: 
			if (PARSE_RADIO_IFNAME_SUB == str[0]){
		
				state = check_radioid;
				}
			else
				state = check_fail;
			break;

		case check_radioid: 
			*radioid = strtoul((char *)&(str[1]),&str,10);

			if(*radioid >= 0 && *radioid < 4){/*radioid range 0 1 2 3*/
        		state=check_point;
			}
			else state=check_fail;
			
			break;

		case check_point: 
			if (PARSE_RADIO_IFNAME_POINT == str[0]){
			
				state = check_wlanid;
				
				}
			else
				state = check_fail;
			break;
				
		case check_wlanid: 
			*wlanid = strtoul((char *)&(str[1]),&str,10);

			if(*wlanid > 0 && *wlanid < WLAN_NUM+1){
        		state=check_end;
			}
			else state=check_fail;
			
			break;
			
		
		
		case check_fail:
		
            return -1;
			break;

		case check_end: 
			if ('\0' == str[0]) {
				state = check_success;
				
				}
			else
				state = check_fail;
				break;
			
		case check_success: 
			return 0;
			break;
			
		default: break;
		}
		
		}
		
}

int parse_radio_ifname_v3(char* ptr,int *wtpid,int *radioid,int *wlanid,int *vrrid,int *slotid)
{
	
    radio_ifname_state state = check_slotid;
	char *str = NULL;
	str = ptr;
   
	while(1){
		switch(state){
		case check_slotid: 
			*slotid = strtoul(str,&str,10);
			if(*slotid >=0 && *slotid <= 16){
				state=check_sub;
			}
			else state=check_fail;
			break;			
		case check_vrrip: 
			*vrrid = strtoul((char *)&str[1],&str,10);
			if(*vrrid >=0 && *vrrid <= 16){
        		state=check_sub2;
			}
			else state=check_fail;
			break;
		case check_wtpid: 
			*wtpid = strtoul((char *)&str[1],&str,10);
			if(*wtpid > 0 && *wtpid <= 4096){
        		state=check_sub3;
			}
			else state=check_fail;
			
			break;
		case check_sub: 
		
			if (PARSE_RADIO_IFNAME_SUB == str[0]){
		
				state = check_vrrip;
			}
			else
				state = check_fail;
			break;

		case check_sub2: 
			if (PARSE_RADIO_IFNAME_SUB == str[0]){
		
				state = check_wtpid;
				}
			else
				state = check_fail;
			break;
		case check_sub3: 
			if (PARSE_RADIO_IFNAME_SUB == str[0]){
		
				state = check_radioid;
				}
			else
				state = check_fail;
			break;

		case check_radioid: 
			*radioid = strtoul((char *)&(str[1]),&str,10);

			if(*radioid >= 0 && *radioid < 4){/*radioid range 0 1 2 3*/
        		state=check_point;
			}
			else state=check_fail;
			
			break;

		case check_point: 
			if (PARSE_RADIO_IFNAME_POINT == str[0]){
			
				state = check_wlanid;
				
				}
			else
				state = check_fail;
			break;
				
		case check_wlanid: 
			*wlanid = strtoul((char *)&(str[1]),&str,10);

			if(*wlanid > 0 && *wlanid < WLAN_NUM+1){
        		state=check_end;
			}
			else state=check_fail;
			
			break;
			
		
		
		case check_fail:
		
            return -1;
			break;

		case check_end: 
			if ('\0' == str[0]) {
				state = check_success;
				
				}
			else
				state = check_fail;
				break;
			
		case check_success: 
			return 0;
			break;
			
		default: break;
		}
		
		}
		
}

int dcli_wqos_method_parse_fist(char *DBUS_METHOD){
	unsigned int sn = 0;
	if (!strcmp(DBUS_METHOD,WID_DBUS_QOS_METHOD_SHOW_QOS_LIST))
	{
		sn = 1;
	}		
	else if (!strcmp(DBUS_METHOD,WID_DBUS_QOS_METHOD_SHOW_QOS))
	{
		sn = 2;
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_QOS_METHOD_SHOW_QOS_EXTENSION_INFO))
	{
		sn = 3;
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_QOS_METHOD_SHOW_RADIO_QOS_INFO))
	{
		sn = 4;
	}
	return sn ;
}
void* dcli_wqos_wireless_qos_show_config_info(
	int index,
	unsigned int* ret,
	unsigned int id,
	unsigned int qos_id,
	unsigned int vid,
	//AC_QOS **qos,
	DBusConnection *dcli_dbus_connection,
	char *DBUS_METHOD
	)
{	
	int i = 0;
	int localid = id;
	unsigned int dcli_sn = 0;
//	int num = 0;
	AC_QOS *qos[QOS_NUM];
	DCLI_WQOS *WQOS = NULL;
	char en[] = "enable";
	char dis[] = "disable";
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
    
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	dcli_sn = dcli_wqos_method_parse_fist(DBUS_METHOD);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_INTERFACE,INTERFACE);
	switch(dcli_sn){
		case WID_DBUS_QOS_METHOD_SHOW_QOS_LIST_CASE :
			{
				query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
				dbus_error_init(&err);
			}
		break;
		case WID_DBUS_QOS_METHOD_SHOW_QOS_CASE :
			{
				query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
				dbus_error_init(&err);		
				dbus_message_append_args(query,
										 DBUS_TYPE_UINT32,&qos_id,
										 DBUS_TYPE_INVALID);
			}
		break;
		case WID_DBUS_QOS_METHOD_SHOW_QOS_EXTENSION_INFO_CASE :
			{
				query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
				dbus_error_init(&err);		
				dbus_message_append_args(query,
										 DBUS_TYPE_UINT32,&qos_id,
										 DBUS_TYPE_INVALID);
			}
		break;
		case WID_DBUS_QOS_METHOD_SHOW_RADIO_QOS_INFO_CASE :
			{	
				int WTPID = qos_id;
				int radio_l_id = vid;
				query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_QOS_METHOD_SHOW_RADIO_QOS_INFO);
				dbus_error_init(&err);
				dbus_message_append_args(query,
										 DBUS_TYPE_UINT32,&WTPID,
										 DBUS_TYPE_UINT32,&radio_l_id,
										 DBUS_TYPE_INVALID);
			}
		break;
		default :break;
	}

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply){
		dbus_error_free_for_dcli(&err);

		*ret = -1;
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if((*ret) == 0 )
	{	
		char* name = NULL;
		WQOS = (DCLI_WQOS*)malloc(sizeof(DCLI_WQOS));
		WQOS->qos_num = 0;
		WQOS->qos = NULL;
		
		switch(dcli_sn){
			case 1 :{	
				
				//AC_QOS *qos[QOS_NUM];
				//WQOS->qos = (AC_QOS*)malloc(1*sizeof(AC_QOS*));    /*fengwenchao modify 20110223 ,for bug Autelan2181*/
				
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&WQOS->qos_num);

				WQOS->qos = (AC_QOS*)malloc((WQOS->qos_num)*sizeof(AC_QOS*));   /*fengwenchao modify 20110223 ,for bug Autelan2181*/

				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);
				for (i = 0; i < (WQOS->qos_num); i++) {
					DBusMessageIter iter_struct;
					
					WQOS->qos[i] = (AC_QOS*)malloc(sizeof(AC_QOS));
					WQOS->qos[i]->radio_qos[0] = (qos_profile*)malloc(sizeof(qos_profile));
					dbus_message_iter_recurse(&iter_array,&iter_struct);
				
					dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[i]->QosID));
				
					dbus_message_iter_next(&iter_struct);
					
					dbus_message_iter_get_basic(&iter_struct,&name);/*(WQOS->qos[i]->name)*/

						WQOS->qos[i]->name = (char*)malloc(strlen(name)+1);
						memset(WQOS->qos[i]->name,0,strlen(name)+1);
						memcpy(WQOS->qos[i]->name,name,strlen(name));

					dbus_message_iter_next(&iter_struct);
					
					dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[i]->radio_qos[0]->mapstate));

					dbus_message_iter_next(&iter_array);
				}
			}
			break;
			case 2 :{	
				int j =0;
				WQOS->qos = (AC_QOS*)malloc(1*sizeof(AC_QOS*));
				WQOS->qos[0] = (AC_QOS*)malloc(sizeof(AC_QOS));
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&(WQOS->qos[0]->QosID));
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&name);/*(WQOS->qos[0]->name)*/

					WQOS->qos[0]->name = (char*)malloc(strlen(name)+1);
					memset(WQOS->qos[0]->name,0,strlen(name)+1);
					memcpy(WQOS->qos[0]->name,name,strlen(name));

				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);

				for (i = 0; i < 4; i++) {
					DBusMessageIter iter_struct;

					WQOS->qos[0]->radio_qos[i] = (qos_profile *)malloc(sizeof(qos_profile));
					WQOS->qos[0]->client_qos[i] = (qos_profile *)malloc(sizeof(qos_profile));

					dbus_message_iter_recurse(&iter_array,&iter_struct);

					//radio qos info

					dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->radio_qos[i]->QueueDepth));

					dbus_message_iter_next(&iter_struct);

					dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->radio_qos[i]->CWMin));

					dbus_message_iter_next(&iter_struct);

					dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->radio_qos[i]->CWMax));

					dbus_message_iter_next(&iter_struct);

					dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->radio_qos[i]->AIFS));
							
					dbus_message_iter_next(&iter_struct);
							
					dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->radio_qos[i]->TXOPlimit));
					/*
					dbus_message_iter_next(&iter_struct);
							
					dbus_message_iter_get_basic(&iter_struct,&(radio_qos[i]->Dot1PTag));
							
					dbus_message_iter_next(&iter_struct);
							
					dbus_message_iter_get_basic(&iter_struct,&(radio_qos[i]->DSCPTag));
					*/		
					dbus_message_iter_next(&iter_struct);
							
					dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->radio_qos[i]->ACK));

					dbus_message_iter_next(&iter_struct);
							
					dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->radio_qos[i]->mapstate));

					dbus_message_iter_next(&iter_struct);
							
					dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->radio_qos[i]->wmm_map_dot1p));

					dbus_message_iter_next(&iter_struct);
							
					dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->radio_qos[i]->dot1p_map_wmm_num));

					for(j=0;j<8;j++)
					{
						dbus_message_iter_next(&iter_struct);
							
						dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->radio_qos[i]->dot1p_map_wmm[j]));

					}

					/*client qos info*/

					dbus_message_iter_next(&iter_struct);

					dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->client_qos[i]->QueueDepth));
							
					dbus_message_iter_next(&iter_struct);

					dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->client_qos[i]->CWMin));

					dbus_message_iter_next(&iter_struct);

					dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->client_qos[i]->CWMax));

					dbus_message_iter_next(&iter_struct);

					dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->client_qos[i]->AIFS));
							
					dbus_message_iter_next(&iter_struct);
							
					dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->client_qos[i]->TXOPlimit));
					/*
					dbus_message_iter_next(&iter_struct);
							
					dbus_message_iter_get_basic(&iter_struct,&(client_qos[i]->Dot1PTag));
							
					dbus_message_iter_next(&iter_struct);
							
					dbus_message_iter_get_basic(&iter_struct,&(client_qos[i]->DSCPTag));
							
					dbus_message_iter_next(&iter_struct);
							
					dbus_message_iter_get_basic(&iter_struct,&(client_qos[i]->ACK));
					*/
					dbus_message_iter_next(&iter_array);
				}
			}
			break;
			case 3 :{
				int i = 0;
				WQOS->qos = (AC_QOS*)malloc(1*sizeof(AC_QOS*));
				WQOS->qos[0] = (AC_QOS *)malloc(sizeof(AC_QOS));				
				char *manage_name = NULL;
				char *grab_name = NULL;
				char *shove_name = NULL;
				{	
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&WQOS->qos[0]->QosID);
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&WQOS->qos[0]->qos_total_bandwidth);
			
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&WQOS->qos[0]->qos_res_scale);
			
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&WQOS->qos[0]->qos_share_bandwidth);
			
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&WQOS->qos[0]->qos_res_share_scale);
			
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&manage_name);
					memset(WQOS->qos[0]->qos_manage_arithmetic,0,WID_QOS_ARITHMETIC_NAME_LEN);
					memcpy(WQOS->qos[0]->qos_manage_arithmetic,manage_name,strlen(manage_name));
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&grab_name);
					memset(WQOS->qos[0]->qos_res_grab_arithmetic,0,WID_QOS_ARITHMETIC_NAME_LEN);
					memcpy(WQOS->qos[0]->qos_res_grab_arithmetic,grab_name,strlen(grab_name));
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&shove_name);
					memset(WQOS->qos[0]->qos_res_shove_arithmetic,0,WID_QOS_ARITHMETIC_NAME_LEN);
					memcpy(WQOS->qos[0]->qos_res_shove_arithmetic,shove_name,strlen(shove_name));
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&WQOS->qos[0]->qos_use_res_grab);
			
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&WQOS->qos[0]->qos_use_res_shove);
					
					dbus_message_iter_next(&iter);	
					dbus_message_iter_recurse(&iter,&iter_array);
					
					for (i = 0; i < 4; i++) {
						DBusMessageIter iter_struct;
						
						WQOS->qos[0]->radio_qos[i] = (qos_profile *)malloc(sizeof(qos_profile));
						
						dbus_message_iter_recurse(&iter_array,&iter_struct);
					
						dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->radio_qos[i]->qos_average_rate));
					
						dbus_message_iter_next(&iter_struct);
						
						dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->radio_qos[i]->qos_max_degree));
					
						dbus_message_iter_next(&iter_struct);
				
						dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->radio_qos[i]->qos_policy_pri));
					
						dbus_message_iter_next(&iter_struct);
					
						dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->radio_qos[i]->qos_res_shove_pri));
								
						dbus_message_iter_next(&iter_struct);
								
						dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->radio_qos[i]->qos_res_grab_pri));
						
						dbus_message_iter_next(&iter_struct);
								
						dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->radio_qos[i]->qos_max_parallel));
								
						dbus_message_iter_next(&iter_struct);
								
						dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->radio_qos[i]->qos_bandwidth));
							
						dbus_message_iter_next(&iter_struct);
								
						dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->radio_qos[i]->qos_bandwidth_scale));
			
						dbus_message_iter_next(&iter_struct);
								
						dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->radio_qos[i]->qos_use_wred));
			
						dbus_message_iter_next(&iter_struct);
								
						dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->radio_qos[i]->qos_use_traffic_shaping));
			
						dbus_message_iter_next(&iter_struct);
								
						dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->radio_qos[i]->qos_use_flow_eq_queue));
			
						dbus_message_iter_next(&iter_struct);
								
						dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->radio_qos[i]->qos_flow_average_rate));
			
						dbus_message_iter_next(&iter_struct);
								
						dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->radio_qos[i]->qos_flow_max_degree));
			
						dbus_message_iter_next(&iter_struct);
								
						dbus_message_iter_get_basic(&iter_struct,&(WQOS->qos[0]->radio_qos[i]->qos_flow_max_queuedepth));
			
						dbus_message_iter_next(&iter_array);
					}
				}
				
			}
			break;
			case 4 :
			{			
						WQOS->qos = (AC_QOS*)malloc(1*sizeof(AC_QOS*));			
						WQOS->qos[0] = (AC_QOS *)malloc(sizeof(AC_QOS));
						dbus_message_iter_next(&iter);	
						dbus_message_iter_get_basic(&iter,&WQOS->qos[0]->QosID); 	
			}
			break;
			default :break;
		}
	}
	dbus_message_unref(reply);
	return WQOS;

}
void dcli_wqos_free_fun(char *DBUS_METHOD,DCLI_WQOS *WQOS){
		int i = 0;
		int dcli_sn = 0;
		dcli_sn = dcli_wqos_method_parse_fist(DBUS_METHOD);

		switch(dcli_sn){
			case 1 :{
				if(WQOS){
					for (i = 0; i < WQOS->qos_num; i++) {	
							
						if((WQOS->qos)&&(WQOS->qos[i])&&(WQOS->qos[i]->radio_qos)&&(WQOS->qos[i]->radio_qos[0]))
						{
							free(WQOS->qos[i]->radio_qos[0]);
							WQOS->qos[i]->radio_qos[0] = NULL;	
						}
						if((WQOS->qos)&&(WQOS->qos[i])&&(WQOS->qos[i]->name))
						{
							free(WQOS->qos[i]->name);
							WQOS->qos[i]->name = NULL;
						}
						if((WQOS->qos)&&(WQOS->qos[i]))
						{
							free(WQOS->qos[i]);
							WQOS->qos[i] = NULL;	
						}
					}
					CW_FREE_OBJECT(WQOS->qos);
					CW_FREE_OBJECT(WQOS);
				}
			}
			break;
			case 2 :{
				if((WQOS)&&(WQOS->qos[0]))
				for(i=0;i<4;i++)
				{	
					if((WQOS->qos[0]->client_qos)&&(WQOS->qos[0]->client_qos[i])){
						free(WQOS->qos[0]->client_qos[i]);
						WQOS->qos[0]->client_qos[i] = NULL ;
					}
					if((WQOS->qos[0]->radio_qos)&&(WQOS->qos[0]->radio_qos[i])){
						free(WQOS->qos[0]->radio_qos[i]);
						WQOS->qos[0]->radio_qos[i] = NULL ;
					}
				}
				if((WQOS)&&(WQOS->qos)&&(WQOS->qos[0])){
					CW_FREE_OBJECT(WQOS->qos[0]->name);					
				}
				if((WQOS)&&(WQOS->qos)){
					CW_FREE_OBJECT(WQOS->qos[0]);
					CW_FREE_OBJECT(WQOS->qos);
				}
				CW_FREE_OBJECT(WQOS);
			}
			break;
			case 3 :{
				for(i=0;i<4;i++)
				{
					if((WQOS)&&(WQOS->qos)&&(WQOS->qos[0])&&(WQOS->qos[0]->radio_qos)&&(WQOS->qos[0]->radio_qos[i]))
					{
						free(WQOS->qos[0]->radio_qos[i]);
						WQOS->qos[0]->radio_qos[i] = NULL;
					}
				}
				
				if((WQOS)&&(WQOS->qos)&&(WQOS->qos[0]))
				{
					free(WQOS->qos[0]);
					WQOS->qos[0] = NULL;
				}
				if(WQOS)
				{
					CW_FREE_OBJECT(WQOS->qos);
				}
				CW_FREE_OBJECT(WQOS);
			}
			break;
			case 4 :{
				if((WQOS)&&(WQOS->qos))
				{
					CW_FREE_OBJECT(WQOS->qos[0]);
					CW_FREE_OBJECT(WQOS->qos);
				}
				CW_FREE_OBJECT(WQOS);
			}
			break;
			default : break;
			
		}
}
/*fengwenchao add 20110427*/
int wid_delete_radio_with_qos_profile(int index,DBusConnection *dcli_dbus_connection, int qos_id,int localid)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter	 iter_array;	

	int ret = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_QOS_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,\
							INTERFACE,WID_DBUS_CONF_METHOD_DELETE_RADIO_WITH_QOS_PROFILE);
										
	dbus_error_init(&err);

	dbus_message_append_args(query,
						DBUS_TYPE_UINT32,&qos_id,								
						DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);	

	dbus_message_unref(query);
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);

	return ret;
}
/*fengwenchao add end*/
#endif
