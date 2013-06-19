#ifdef _D_WCPSS_
#include <string.h>
//#include <zebra.h>
#include <dbus/dbus.h>

//#include "command.h"

//#include "dcli_main.h"
//#include "dcli_ac_ip_list.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"

int dcli_ac_ip_list_method_parse(char *DBUS_METHOD){
	unsigned int sn = 0;
	if (!strcmp(DBUS_METHOD,WID_DBUS_ACIPLIST_METHOD_SHOW_AC_IP_LIST_ONE))
	{
		sn = 1;/*"show wlan (list|all)"*/
	}		
	else if (!strcmp(DBUS_METHOD,WID_DBUS_ACIPLIST_METHOD_SHOW_AC_IP_LIST))
	{
		sn = 2;/*"show wlan WLANID"*/
	}
	else// if(!strcmp(DBUS_METHOD,WID_DBUS_CONF_METsHOD_SHOW_WIDS_DEVICE_LIST_BYWTPID))
	{
	//	sn = 6;/**/
	}
	return sn;
}
dcli_ac_ip_list_add_node(wid_ac_ip_group *IPLIST,struct wid_ac_ip *node)
{
		if(node ==NULL)
			return -1;	
		struct wid_ac_ip *tmp;
		if(IPLIST->ip_list == NULL){
		//	printf("tunue_wlan_vlan == NULL \n");
			node->next = NULL;
			IPLIST->ip_list = node;
		}
		else{
		//	printf("11 tunue_wlan_vlan != NULL \n");
			tmp = IPLIST->ip_list;
			while(tmp->next != NULL){
				tmp = tmp->next;
			}	
		//	printf("22 tunue_wlan_vlan != NULL \n");
			tmp->next = node;
			node->next = NULL;
		}	
		return 0;
}
void dcli_ac_ip_list_free_fun(char *DBUS_METHOD,DCLI_AC_IP_LIST_API_GROUP *IPINFO){
		int i = 0;
		int dcli_sn = 0;
		dcli_sn = dcli_ac_ip_list_method_parse(DBUS_METHOD);
		
		struct wid_ac_ip *tmp = NULL;
		struct wid_ac_ip *tmp1 = NULL;

		switch(dcli_sn){
			case 1 :{
				if((IPINFO)&&(IPINFO->AC_IP_LIST)&&(IPINFO->AC_IP_LIST[0])){
					tmp = IPINFO->AC_IP_LIST[0]->ip_list;
					while(tmp != NULL)
					{
						tmp1 = tmp;
						tmp = tmp->next;
						CW_FREE_OBJECT(tmp1->ip);
						CW_FREE_OBJECT(tmp1);
						
					}
					CW_FREE_OBJECT(IPINFO->AC_IP_LIST[0]->ifname);
					CW_FREE_OBJECT(IPINFO->AC_IP_LIST[0]->ipaddr);
					CW_FREE_OBJECT(IPINFO->AC_IP_LIST[0]);
				}
				if(IPINFO){
					CW_FREE_OBJECT(IPINFO->AC_IP_LIST);
					CW_FREE_OBJECT(IPINFO);
				}
			}
			break;
			case 2 :{
				if(IPINFO){
					for (i = 0; i < IPINFO->ip_list_num; i++){
						if((IPINFO->AC_IP_LIST)&&(IPINFO->AC_IP_LIST[i])){
							tmp = IPINFO->AC_IP_LIST[i]->ip_list;
							while(tmp != NULL)
							{
								tmp1 = tmp;
								tmp = tmp->next;
								CW_FREE_OBJECT(tmp1->ip);
								CW_FREE_OBJECT(tmp1);
								
							}
							CW_FREE_OBJECT(IPINFO->AC_IP_LIST[i]->ifname);
							CW_FREE_OBJECT(IPINFO->AC_IP_LIST[i]);
						}
					}
					if(IPINFO){
						CW_FREE_OBJECT(IPINFO->AC_IP_LIST);
						CW_FREE_OBJECT(IPINFO);
					}
				}
			}
			break;
			case 3 :{}
			break;
			case 4 :{}
			break;
			default : break;
			
		}
}
void *dcli_ac_ip_list_show_api_group(
	int index,
	unsigned char ebrid,
	unsigned int id,
	unsigned int* ret,
	unsigned char *num4,
	DBusConnection *dcli_dbus_connection,
	char *DBUS_METHOD
	)
{	
	int num;
	unsigned char num_char;
	int i = 0;	
	int localid = 0;
	unsigned int dcli_sn = 0;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	dbus_error_init(&err);
	//int wlanid = id1;
	localid = (int)*num4;
	DCLI_AC_IP_LIST_API_GROUP * LIST = NULL;
	CW_CREATE_OBJECT_ERR(LIST, DCLI_AC_IP_LIST_API_GROUP, return NULL;); 
	LIST->AC_IP_LIST = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
    ReInitDbusPath_V2(localid,index,WID_DBUS_ACIPLIST_OBJPATH,OBJPATH);
    ReInitDbusPath_V2(localid,index,WID_DBUS_ACIPLIST_INTERFACE,INTERFACE);

	dcli_sn = dcli_ac_ip_list_method_parse(DBUS_METHOD);
	switch(dcli_sn){
		case 1 :{
			query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
			dbus_error_init(&err);
			dbus_message_append_args(query,
									 DBUS_TYPE_BYTE,&ebrid,
									 DBUS_TYPE_INVALID);
		}
		break;
		case 2 :{
			query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
			dbus_error_init(&err);
		}
		break;
		case 3 :{}
		break;
		default : break;
	}
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);

	if (NULL == reply) {
		//printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
		//	printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		if(LIST){
			free(LIST);
			LIST = NULL;
		}
		*ret = -1;
		return NULL;
	}

	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&(*ret));
	//printf("ccccccccc  ret is %d \n",*ret);
	//printf("ccccccccc  dcli_sn is %d \n",dcli_sn);
	if(*ret == 0 )
	{	
		char*ifname = NULL;
		char*ip = NULL;
		switch(dcli_sn){
		case 1 :{	
			LIST->AC_IP_LIST = (wid_ac_ip_group *)malloc(1*sizeof(wid_ac_ip_group*));
			LIST->AC_IP_LIST[0] = (wid_ac_ip_group *)malloc(sizeof(wid_ac_ip_group));
			LIST->AC_IP_LIST[0]->ip_list = NULL;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->AC_IP_LIST[0]->GroupID);
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&ifname);/*LIST->AC_IP_LIST[0]->ifname*/

			LIST->AC_IP_LIST[0]->ifname = (char*)malloc(strlen(ifname)+1);
			memset(LIST->AC_IP_LIST[0]->ifname,0,strlen(ifname)+1);
			memcpy(LIST->AC_IP_LIST[0]->ifname,ifname,strlen(ifname));
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&ip);/*LIST->AC_IP_LIST[0]->ifname*/
			
			LIST->AC_IP_LIST[0]->ipaddr = (char*)malloc(strlen(ip)+1);
			memset(LIST->AC_IP_LIST[0]->ipaddr,0,strlen(ip)+1);
			memcpy(LIST->AC_IP_LIST[0]->ipaddr,ip,strlen(ip));
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->AC_IP_LIST[0]->load_banlance);

			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->AC_IP_LIST[0]->diff_count);			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&num);
			struct wid_ac_ip **iplist = NULL;
			iplist = malloc(num* sizeof(struct wid_ac_ip *));
			memset(iplist, 0, num*sizeof(struct wid_ac_ip*));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);
			
			for (i = 0; i < num; i++) {
				DBusMessageIter iter_struct;
				
				iplist[i] = (struct wid_ac_ip *)malloc(sizeof(struct wid_ac_ip));
				
				dbus_message_iter_recurse(&iter_array,&iter_struct);
			
				dbus_message_iter_get_basic(&iter_struct,&ip);/*(iplist[i]->ip)*/

				iplist[i]->ip = (char*)malloc(strlen(ip)+1);
				memset(iplist[i]->ip,0,strlen(ip)+1);
				memcpy(iplist[i]->ip,ip,strlen(ip));

				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(iplist[i]->priority));
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(iplist[i]->threshold));				
				
				dbus_message_iter_next(&iter_struct);	
				dbus_message_iter_get_basic(&iter_struct,&(iplist[i]->wtpcount));				
			
				dbus_message_iter_next(&iter_array);
				dcli_ac_ip_list_add_node(LIST->AC_IP_LIST[0],iplist[i]);
			}
		}
		break;
		case 2 :{	
			int list_num = 0;
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&list_num);

			LIST->AC_IP_LIST = (wid_ac_ip_group *)malloc(list_num*sizeof(wid_ac_ip_group*));
			LIST->ip_list_num = list_num;
			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);
			
			for (i = 0; i < LIST->ip_list_num; i++) {
				DBusMessageIter iter_struct;
				LIST->AC_IP_LIST[i] = (wid_ac_ip_group *)malloc(sizeof(wid_ac_ip_group));
				LIST->AC_IP_LIST[i]->ip_list = NULL;
				//ACIPLIST[i] = (wid_ac_ip_group *)malloc(sizeof(wid_ac_ip_group));
				
				dbus_message_iter_recurse(&iter_array,&iter_struct);
			
				dbus_message_iter_get_basic(&iter_struct,&(LIST->AC_IP_LIST[i]->GroupID));
			
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&ifname);/*LIST->AC_IP_LIST[i]->ifname*/

				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->AC_IP_LIST[i]->load_banlance));	
				
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->AC_IP_LIST[i]->diff_count));					
				dbus_message_iter_next(&iter_array);

				LIST->AC_IP_LIST[i]->ifname = (char*)malloc(strlen(ifname)+1);
				memset(LIST->AC_IP_LIST[i]->ifname,0,strlen(ifname)+1);
				memcpy(LIST->AC_IP_LIST[i]->ifname,ifname,strlen(ifname));
			}
		}
		break;
		case 3 :{}
		break;
		default : break;
		}
	}
	dbus_message_unref(reply);
	return LIST;

}
#endif


