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
//#include <zebra.h>
#include <dbus/dbus.h>
//#include "command.h"
//#include "if.h"
//#include "dcli_main.h"
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDbusDef1.h"
//#include "dcli_ebr.h"  


dcli_ebr_add_ebr_name_node(ETHEREAL_BRIDGE *EBR,EBR_IF_LIST *node,int flag)
{
	if(node ==NULL)
		return -1;	

	
	if(flag == 0){
		if(EBR->iflist == NULL){
		//	printf("tunue_wlan_vlan == NULL \n");
			node->ifnext = NULL;
			EBR->iflist = node;
		}
		/*else{
		//	printf("11 tunue_wlan_vlan != NULL \n");
			while(EBR->iflist->ifnext != NULL)
				EBR->iflist = EBR->iflist->ifnext;
				
		//	printf("22 tunue_wlan_vlan != NULL \n");
			EBR->iflist->ifnext = node;
		}	*/

		else{		
			node->ifnext = EBR->iflist->ifnext;
			EBR->iflist->ifnext = node;			
		}
	}

	else{
		if(EBR->uplinklist == NULL){
		//	printf("tunue_wlan_vlan == NULL \n");
			node->ifnext = NULL;
			EBR->uplinklist = node;
		}
		/*else{
		//	printf("11 tunue_wlan_vlan != NULL \n");
			while(EBR->uplinklist->ifnext != NULL){
				EBR->uplinklist = EBR->uplinklist->ifnext;
			}	
		//	printf("22 tunue_wlan_vlan != NULL \n");
			EBR->uplinklist->ifnext = node;
			node->ifnext = NULL;
		}	*/
		else{
			node->ifnext = EBR->uplinklist->ifnext;
			EBR->uplinklist->ifnext = node;		
			}
	}
		return 0;
}

int dcli_ebr_method_parse(char *DBUS_METHOD){
	unsigned int sn = 0;
	if (!strcmp(DBUS_METHOD,WID_DBUS_EBR_METHOD_SHOW_EBR))
	{
		sn = 1;
	}		
	else if (!strcmp(DBUS_METHOD,WID_DBUS_EBR_METHOD_SHOW_EBR_LIST))
	{
		sn = 2;
	}
	else// if(!strcmp(DBUS_METHOD,WID_DBUS_CONF_METsHOD_SHOW_WIDS_DEVICE_LIST_BYWTPID))
	{
	//	sn = 6;/**/
	}
	return sn;
}
void dcli_ebr_free_fun(char *DBUS_METHOD,DCLI_EBR_API_GROUP *EBRINFO){
		int i = 0;
		int dcli_sn = 0;
		dcli_sn = dcli_ebr_method_parse(DBUS_METHOD);

		switch(dcli_sn){
			case 1 :{
				CW_FREE_OBJECT(EBRINFO->EBR[0]->name);
				if((EBRINFO)&&(EBRINFO->EBR[0])&&(EBRINFO->EBR[0]->iflist)){
					EBR_IF_LIST    *phead = NULL;
					EBR_IF_LIST    *pnext = NULL;
					phead = EBRINFO->EBR[0]->iflist;
					EBRINFO->EBR[0]->iflist = NULL;
					while(phead){
						pnext = phead->ifnext;
						CW_FREE_OBJECT(phead->ifname);
						CW_FREE_OBJECT(phead);
						phead = pnext;
					}

				}
				if((EBRINFO)&&(EBRINFO->EBR[0])&&(EBRINFO->EBR[0]->uplinklist)){
					EBR_IF_LIST    *phead = NULL;
					EBR_IF_LIST    *pnext = NULL;
					phead = EBRINFO->EBR[0]->uplinklist;
					EBRINFO->EBR[0]->uplinklist = NULL;
					while(phead){
						pnext = phead->ifnext;
						CW_FREE_OBJECT(phead->ifname);
						CW_FREE_OBJECT(phead);
						phead = pnext;
					}
				}
				if((EBRINFO)&&(EBRINFO->EBR)){
					CW_FREE_OBJECT(EBRINFO->EBR[0]);
					CW_FREE_OBJECT(EBRINFO->EBR);
				}
				CW_FREE_OBJECT(EBRINFO);
			}
			break;
			case 2 :{
				if(EBRINFO){
					for (i = 0; i < EBRINFO->ebr_num; i++) {
						if((EBRINFO->EBR)&&(EBRINFO->EBR[i])){
							CW_FREE_OBJECT(EBRINFO->EBR[i]->name);
							CW_FREE_OBJECT(EBRINFO->EBR[i]);
						}
					}
					CW_FREE_OBJECT(EBRINFO->EBR);
					CW_FREE_OBJECT(EBRINFO);
				}
			}
			break;
			case 3 :{

			}
			break;
			case 4 :{

			}
			break;
			default : break;
			
		}

}
void *dcli_ebr_show_api_group(
	int index,
	unsigned char wlanid,
	unsigned int EBRID,
	unsigned int* ret,
	unsigned char *num4,
	DBusConnection *dcli_dbus_connection,
	char *DBUS_METHOD
	)
{	
	int num,num2;
	unsigned char num_char;
	int i = 0;	
	unsigned int dcli_sn = 0;
	//printf("num4=%d.\n",*num4);
	int localid = (int)*num4;
	//printf("localid=%d.\n",localid);
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	DBusError err;
	dbus_error_init(&err);
	//int wlanid = id1;
	
	DCLI_EBR_API_GROUP * LIST = NULL;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_EBR_INTERFACE,INTERFACE);

	dcli_sn = dcli_ebr_method_parse(DBUS_METHOD);
	switch(dcli_sn){
		case 1 :{
			query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
			dbus_error_init(&err);
			dbus_message_append_args(query,
									 DBUS_TYPE_UINT32,&EBRID,
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
		case 4 :{}
		break;
		default : break;
	}
	reply = dbus_connection_send_with_reply_and_block(dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);

	if (NULL == reply) {
		//printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
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
		char* name = NULL;
		char* ifname = NULL;
		char* ifname2 = NULL;
		CW_CREATE_OBJECT_ERR(LIST, DCLI_EBR_API_GROUP, return NULL;); 
		LIST->EBR = NULL;
		//LIST->ebr_num = 0;
		LIST->ebr_num = 0;
		switch(dcli_sn){
		case 1 :{	
			
			LIST->EBR = (ETHEREAL_BRIDGE *)malloc(sizeof(ETHEREAL_BRIDGE*));
			LIST->EBR[0] = (ETHEREAL_BRIDGE *)malloc(sizeof(ETHEREAL_BRIDGE));
			LIST->EBR[0]->iflist = NULL;
			LIST->EBR[0]->uplinklist = NULL;
		//	ebr = (ETHEREAL_BRIDGE *)malloc(sizeof(ETHEREAL_BRIDGE));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->EBR[0]->EBRID);
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&name);/*LIST->EBR[0]->name*/
			
				LIST->EBR[0]->name = (char*)malloc(strlen(name)+1);
				memset(LIST->EBR[0]->name,0,strlen(name)+1);
				memcpy(LIST->EBR[0]->name,name,strlen(name));
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->EBR[0]->state);
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->EBR[0]->isolation_policy);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->EBR[0]->multicast_isolation_policy);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->EBR[0]->sameportswitch);

			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &LIST->EBR[0]->bridge_ucast_solicit_stat);
			

			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter, &LIST->EBR[0]->bridge_mcast_solicit_stat);


			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->EBR[0]->multicast_fdb_learn);
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&num);
			EBR_IF_LIST **iflist = NULL;
			EBR_IF_LIST **uplinklist = NULL;
			iflist = malloc(num* sizeof(EBR_IF_LIST *));
			memset(iflist, 0, num*sizeof(EBR_IF_LIST*));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);
			//printf("ebr ifname num is %d \n",num);		
			for (i = 0; i < num; i++) {
				DBusMessageIter iter_struct;
				iflist[i] = (EBR_IF_LIST *)malloc(sizeof(EBR_IF_LIST));
				
				dbus_message_iter_recurse(&iter_array,&iter_struct);
			
				dbus_message_iter_get_basic(&iter_struct,&ifname);/*(iflist[i]->ifname)*/

					iflist[i]->ifname = (char*)malloc(strlen(ifname)+1);
					memset(iflist[i]->ifname,0,strlen(ifname)+1);
					memcpy(iflist[i]->ifname,ifname,strlen(ifname));
			
				dbus_message_iter_next(&iter_array);
				dcli_ebr_add_ebr_name_node(LIST->EBR[0],iflist[i],0);/*falg is 0 -->iflist*/
			}
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&num2);
			
			uplinklist = malloc(num2* sizeof(EBR_IF_LIST *));
			memset(uplinklist, 0, num2*sizeof(EBR_IF_LIST*));
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);
			
			for (i = 0; i < num2; i++) {
				DBusMessageIter iter_struct;
				
				uplinklist[i] = (EBR_IF_LIST *)malloc(sizeof(EBR_IF_LIST));
				
				dbus_message_iter_recurse(&iter_array,&iter_struct);
			
				dbus_message_iter_get_basic(&iter_struct,&ifname2);/*(uplinklist[i]->ifname)*/

					uplinklist[i]->ifname = (char*)malloc(strlen(ifname2)+1);
					memset(uplinklist[i]->ifname,0,strlen(ifname2)+1);
					memcpy(uplinklist[i]->ifname,ifname2,strlen(ifname2));
			
				dbus_message_iter_next(&iter_array);
				
				dcli_ebr_add_ebr_name_node(LIST->EBR[0],uplinklist[i],1);/*falg is 1 -->uplinklist*/
			}
		}
		break;
		case 2 :{	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->ebr_num);
			LIST->EBR = (ETHEREAL_BRIDGE *)malloc((LIST->ebr_num)*sizeof(ETHEREAL_BRIDGE*));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);
			
			for (i = 0; i < LIST->ebr_num; i++) {
				DBusMessageIter iter_struct;
				LIST->EBR[i] = (ETHEREAL_BRIDGE *)malloc(sizeof(ETHEREAL_BRIDGE));
				//ebr[i] = (ETHEREAL_BRIDGE *)malloc(sizeof(ETHEREAL_BRIDGE));
				
				dbus_message_iter_recurse(&iter_array,&iter_struct);
			
				dbus_message_iter_next(&iter);	
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter_struct,&(LIST->EBR[i]->EBRID));
			
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&name);/*(LIST->EBR[i]->name)*/

				LIST->EBR[i]->name = (char*)malloc(strlen(name)+1);
				memset(LIST->EBR[i]->name,0,strlen(name)+1);
				memcpy(LIST->EBR[i]->name,name,strlen(name));

				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->EBR[i]->state));

				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->EBR[i]->isolation_policy));

				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->EBR[i]->multicast_isolation_policy));
				

				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct, &(LIST->EBR[i]->bridge_ucast_solicit_stat));
				

				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct, &(LIST->EBR[i]->bridge_mcast_solicit_stat));


				dbus_message_iter_next(&iter_struct);				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->EBR[i]->multicast_fdb_learn));
				
				dbus_message_iter_next(&iter_array);
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
	dbus_message_unref(reply);
	return LIST;

}
#endif

