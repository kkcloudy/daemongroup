#ifndef HMD_LOCAL_C
#define HMD_LOCAL_C

#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <dbus/dbus.h>
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "wcpss/asd/asd.h"

#include "hmd/hmdpub.h"
#include "dbus/hmd/HmdDbusPath.h"
//#include "dbus/hmd/HmdDbusDef.h"
//#include "dcli/lib/dcli_local_hansi.h"
#include "hmd_local.h"
#include "wid_ac.h"


#define		DCLI_FORMIB_FREE_OBJECT(obj_name)				{if(obj_name){free((obj_name)); (obj_name) = NULL;}}
/*--------------------------------------*/
void dcli_free_HmdBoardInfo(struct Hmd_Board_Info_show *hmd_board_node)
{
	struct Hmd_Board_Info_show* hmd_board_temp = NULL;
	int i =0;
	int j = 0;
	int k = 0;
	if(hmd_board_node == NULL)
		return ;
	
	if(hmd_board_node->hmd_board_last  != NULL)
	{
		hmd_board_node->hmd_board_last = NULL;
	}	

	while(hmd_board_node->hmd_board_list != NULL) 
	{
		hmd_board_temp = hmd_board_node->hmd_board_list;
		hmd_board_node->hmd_board_list = hmd_board_temp->next;
		
		for(i = 0; i < hmd_board_temp->InstNum; i++)
		{
			if(hmd_board_temp->Hmd_Inst[i])
			{
				DCLI_FORMIB_FREE_OBJECT(hmd_board_temp->Hmd_Inst[i]);
				/*for(j = 0; j < hmd_board_temp->Hmd_Inst[i]->Inst_UNum; j++)
				{
					DCLI_FORMIB_FREE_OBJECT(hmd_board_temp->Hmd_Inst[i]->Inst_Uplink[j].ifname);
				}

				for(j = 0; j < hmd_board_temp->Hmd_Inst[i]->Inst_DNum; j++)
				{

					DCLI_FORMIB_FREE_OBJECT(hmd_board_temp->Hmd_Inst[i]->Inst_Downlink[j].ifname);
				}

				for(j = 0; j < hmd_board_temp->Hmd_Inst[i]->Inst_GNum; j++)
				{
					DCLI_FORMIB_FREE_OBJECT(hmd_board_temp->Hmd_Inst[i]->Inst_Gateway[j].ifname);
				}
				
				DCLI_FORMIB_FREE_OBJECT(hmd_board_temp->Hmd_Inst[i]->Inst_Hb.ifname);*/
				
			}
		}
		for(i = 0; i < hmd_board_temp->LocalInstNum;i++)
		{
			if(hmd_board_temp->Hmd_Local_Inst[i])
			{
				DCLI_FORMIB_FREE_OBJECT(hmd_board_temp->Hmd_Local_Inst[i]);
				/*for(j = 0; j < hmd_board_temp->Hmd_Local_Inst[i]->Inst_UNum; j++)
				{
					DCLI_FORMIB_FREE_OBJECT(hmd_board_temp->Hmd_Local_Inst[i]->Inst_Uplink[j].ifname);

				}

				for(j = 0; j < hmd_board_temp->Hmd_Local_Inst[i]->Inst_DNum; j++)
				{
	
					DCLI_FORMIB_FREE_OBJECT(hmd_board_temp->Hmd_Local_Inst[i]->Inst_Downlink[j].ifname);

				}

				for(j = 0; j < hmd_board_temp->Hmd_Local_Inst[i]->Inst_GNum; j++)
				{
					DCLI_FORMIB_FREE_OBJECT(hmd_board_temp->Hmd_Local_Inst[i]->Inst_Gateway[j].ifname);

				}	*/			
				
			}

		}		
		
		hmd_board_temp->next = NULL;
		free(hmd_board_temp);
		hmd_board_temp = NULL;
	}
	
	free(hmd_board_node);
	hmd_board_node = NULL;
	return ;	
}


struct Hmd_Board_Info_show* show_hmd_info_show(DBusConnection *dcli_dbus_connection, int *board_num, int *ret)
{
	DBusMessage *query, *reply;
	DBusError err;
	DBusMessageIter	 iter;
	DBusMessageIter  iter_array;	

	int i =0;
	int j =0;
	int k =0;
	struct Hmd_Board_Info_show* hmd_board_head = NULL;
	struct Hmd_Board_Info_show* hmd_board_node = NULL;

	query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_SHOW_HMD_INFO);
	dbus_error_init(&err);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return NULL;
	}	

	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_iter_next(&iter);	
	dbus_message_iter_get_basic(&iter,board_num);
	//printf("  board_num  =  %d  \n",*board_num);

	if((*ret ==0)&&(*board_num != 0))
	{
		if((hmd_board_head = (struct Hmd_Board_Info_show*)malloc(sizeof(struct Hmd_Board_Info_show))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
		memset(hmd_board_head,0,sizeof(struct Hmd_Board_Info_show));
		hmd_board_head->next;
		hmd_board_head->hmd_board_list = NULL;
		hmd_board_head->hmd_board_last = NULL;

		dbus_message_iter_next(&iter);	
		dbus_message_iter_recurse(&iter,&iter_array);

		for(i = 0; i < *board_num ; i++)
		{
			//DBusMessageIter iter_board_array;
			DBusMessageIter iter_board_struct;
			DBusMessageIter iter_inst_array;
			DBusMessageIter iter_inst_struct;
			DBusMessageIter iter_inst_uplink_array;
			DBusMessageIter iter_inst_uplink_struct;
			DBusMessageIter iter_inst_downlink_array;
			DBusMessageIter iter_inst_downlink_struct;
			DBusMessageIter iter_inst_gateway_array;
			DBusMessageIter iter_inst_gateway_struct;
			
			DBusMessageIter iter_l_inst_array;
			DBusMessageIter iter_l_inst_struct;
			DBusMessageIter iter_l_inst_uplink_array;
			DBusMessageIter iter_l_inst_uplink_struct;
			DBusMessageIter iter_l_inst_downlink_array;
			DBusMessageIter iter_l_inst_downlink_struct;
			DBusMessageIter iter_l_inst_gateway_array;
			DBusMessageIter iter_l_inst_gateway_struct;

			
			if((hmd_board_node = (struct Hmd_Board_Info_show*)malloc(sizeof(struct Hmd_Board_Info_show))) == NULL){
					dcli_free_HmdBoardInfo(hmd_board_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			memset(hmd_board_node,0,sizeof(struct Hmd_Board_Info_show));
			hmd_board_node->next;
			hmd_board_node->hmd_board_list = NULL;
			hmd_board_node->hmd_board_last = NULL;

			if(hmd_board_head->hmd_board_list == NULL){
				hmd_board_head->hmd_board_list = hmd_board_node;
				hmd_board_head->next = hmd_board_node;
			}
			else{
				hmd_board_head->hmd_board_last->next = hmd_board_node;
			}
			hmd_board_head->hmd_board_last = hmd_board_node;

			dbus_message_iter_recurse(&iter_array,&iter_board_struct);
			dbus_message_iter_get_basic(&iter_board_struct,&(hmd_board_node->slot_no));
		    
			dbus_message_iter_next(&iter_board_struct);	
			dbus_message_iter_get_basic(&iter_board_struct,&(hmd_board_node->InstNum));
			//printf("  hmd_board_node->InstNum   =  %d \n",hmd_board_node->InstNum);
			dbus_message_iter_next(&iter_board_struct);	
			dbus_message_iter_get_basic(&iter_board_struct,&(hmd_board_node->LocalInstNum));	
			//printf("  hmd_board_node->LocalInstNum  =  %d \n",hmd_board_node->LocalInstNum);
			 dbus_message_iter_next(&iter_board_struct);
			dbus_message_iter_recurse(&iter_board_struct,&iter_inst_array);

			/*Hmd_Inst  begin*/
			for(j = 0; j < hmd_board_node->InstNum; j++)
			{
				hmd_board_node->Hmd_Inst[j] = (struct Hmd_Inst_Mgmt *)malloc(sizeof(struct Hmd_Inst_Mgmt));

				dbus_message_iter_recurse(&iter_inst_array,&iter_inst_struct);

				//printf("  hmd_board_node =  %p \n",hmd_board_node);
				//printf("  Hmd_Inst[%d] =  %p \n",j,hmd_board_node->Hmd_Inst[j]);

				dbus_message_iter_get_basic(&iter_inst_struct,&(hmd_board_node->Hmd_Inst[j]->Inst_ID)); 
				dbus_message_iter_next(&iter_inst_struct);
				dbus_message_iter_get_basic(&iter_inst_struct,&(hmd_board_node->Hmd_Inst[j]->slot_no)); 
				dbus_message_iter_next(&iter_inst_struct);
				dbus_message_iter_get_basic(&iter_inst_struct,&(hmd_board_node->Hmd_Inst[j]->InstState)); 
				dbus_message_iter_next(&iter_inst_struct);
				dbus_message_iter_get_basic(&iter_inst_struct,&(hmd_board_node->Hmd_Inst[j]->Inst_UNum)); 
				dbus_message_iter_next(&iter_inst_struct);
				dbus_message_iter_get_basic(&iter_inst_struct,&(hmd_board_node->Hmd_Inst[j]->Inst_DNum)); 
				dbus_message_iter_next(&iter_inst_struct);
				dbus_message_iter_get_basic(&iter_inst_struct,&(hmd_board_node->Hmd_Inst[j]->Inst_GNum)); 
				dbus_message_iter_next(&iter_inst_struct);
				dbus_message_iter_get_basic(&iter_inst_struct,&(hmd_board_node->Hmd_Inst[j]->priority));
				dbus_message_iter_next(&iter_inst_struct);
				dbus_message_iter_get_basic(&iter_inst_struct,&(hmd_board_node->Hmd_Inst[j]->isActive)); 

				/*Inst_Hb begin*/
				char *ifname = NULL;
				dbus_message_iter_next(&iter_inst_struct);
				dbus_message_iter_get_basic(&iter_inst_struct,&(ifname)); 	
				//printf("  ifname  =  %s \n ",ifname);
							  
				memset(hmd_board_node->Hmd_Inst[j]->Inst_Hb.ifname, 0, strlen(ifname)+1);
				memcpy(hmd_board_node->Hmd_Inst[j]->Inst_Hb.ifname, ifname, strlen(ifname));	

				dbus_message_iter_next(&iter_inst_struct);
				dbus_message_iter_get_basic(&iter_inst_struct,&(hmd_board_node->Hmd_Inst[j]->Inst_Hb.real_ip)); 	
				dbus_message_iter_next(&iter_inst_struct);
				dbus_message_iter_get_basic(&iter_inst_struct,&(hmd_board_node->Hmd_Inst[j]->Inst_Hb.vir_ip)); 	
				dbus_message_iter_next(&iter_inst_struct);
				dbus_message_iter_get_basic(&iter_inst_struct,&(hmd_board_node->Hmd_Inst[j]->Inst_Hb.remote_r_ip)); 	
			//	dbus_message_iter_next(&iter_inst_struct);
			//	dbus_message_iter_get_basic(&iter_inst_struct,&(hmd_board_node->Hmd_Inst[j]->Inst_Hb.umask)); 	
			//	dbus_message_iter_next(&iter_inst_struct);
			//	dbus_message_iter_get_basic(&iter_inst_struct,&(hmd_board_node->Hmd_Inst[j]->Inst_Hb.dmask)); 					
				/*Inst_Hb  end*/
				
				/*Inst_Uplink  begin*/
				dbus_message_iter_next(&iter_inst_struct);
				dbus_message_iter_recurse(&iter_inst_struct,&iter_inst_uplink_array);

				for(k = 0; k < (hmd_board_node->Hmd_Inst[j]->Inst_UNum) ; k++)
				{
					dbus_message_iter_recurse(&iter_inst_uplink_array,&iter_inst_uplink_struct);

					dbus_message_iter_get_basic(&iter_inst_uplink_struct,&(ifname)); 
					memset(hmd_board_node->Hmd_Inst[j]->Inst_Uplink[k].ifname, 0, strlen(ifname)+1);
					memcpy(hmd_board_node->Hmd_Inst[j]->Inst_Uplink[k].ifname, ifname, strlen(ifname));	
					
					
					dbus_message_iter_next(&iter_inst_uplink_struct);
					dbus_message_iter_get_basic(&iter_inst_uplink_struct,&(hmd_board_node->Hmd_Inst[j]->Inst_Uplink[k].real_ip)); 
					dbus_message_iter_next(&iter_inst_uplink_struct);
					dbus_message_iter_get_basic(&iter_inst_uplink_struct,&(hmd_board_node->Hmd_Inst[j]->Inst_Uplink[k].vir_ip));	
					dbus_message_iter_next(&iter_inst_uplink_struct);
					dbus_message_iter_get_basic(&iter_inst_uplink_struct,&(hmd_board_node->Hmd_Inst[j]->Inst_Uplink[k].remote_r_ip)); 	
					dbus_message_iter_next(&iter_inst_uplink_struct);
					dbus_message_iter_get_basic(&iter_inst_uplink_struct,&(hmd_board_node->Hmd_Inst[j]->Inst_Uplink[k].mask));	
			//		dbus_message_iter_next(&iter_inst_uplink_struct);
			//		dbus_message_iter_get_basic(&iter_inst_uplink_struct,&(hmd_board_node->Hmd_Inst[j]->Inst_Uplink[k].dmask)); 
				
					dbus_message_iter_next(&iter_inst_uplink_array);
				}
				/*Inst_Uplink  end*/

				/*Inst_Downlink begin*/
				dbus_message_iter_next(&iter_inst_struct);
				dbus_message_iter_recurse(&iter_inst_struct,&iter_inst_downlink_array);

				for(k = 0; k < (hmd_board_node->Hmd_Inst[j]->Inst_DNum) ; k++)
				{
					dbus_message_iter_recurse(&iter_inst_downlink_array,&iter_inst_downlink_struct);

					dbus_message_iter_get_basic(&iter_inst_downlink_struct,&(ifname)); 								  
					memset(hmd_board_node->Hmd_Inst[j]->Inst_Downlink[k].ifname, 0, strlen(ifname)+1);
					memcpy(hmd_board_node->Hmd_Inst[j]->Inst_Downlink[k].ifname, ifname, strlen(ifname));	
										
					dbus_message_iter_next(&iter_inst_downlink_struct);
					dbus_message_iter_get_basic(&iter_inst_downlink_struct,&(hmd_board_node->Hmd_Inst[j]->Inst_Downlink[k].real_ip)); 
					dbus_message_iter_next(&iter_inst_downlink_struct);
					dbus_message_iter_get_basic(&iter_inst_downlink_struct,&(hmd_board_node->Hmd_Inst[j]->Inst_Downlink[k].vir_ip));	
					dbus_message_iter_next(&iter_inst_downlink_struct);
					dbus_message_iter_get_basic(&iter_inst_downlink_struct,&(hmd_board_node->Hmd_Inst[j]->Inst_Downlink[k].remote_r_ip)); 	
					dbus_message_iter_next(&iter_inst_downlink_struct);
					dbus_message_iter_get_basic(&iter_inst_downlink_struct,&(hmd_board_node->Hmd_Inst[j]->Inst_Downlink[k].mask));	
			//		dbus_message_iter_next(&iter_inst_downlink_struct);
			//		dbus_message_iter_get_basic(&iter_inst_downlink_struct,&(hmd_board_node->Hmd_Inst[j]->Inst_Downlink[k].dmask)); 
				
					dbus_message_iter_next(&iter_inst_downlink_array);
				}				
				/*Inst_Downlink  end*/

				/*Inst_Gateway begin*/
				dbus_message_iter_next(&iter_inst_struct);
				dbus_message_iter_recurse(&iter_inst_struct,&iter_inst_gateway_array);

				for(k = 0; k < (hmd_board_node->Hmd_Inst[j]->Inst_GNum) ; k++)
				{
					dbus_message_iter_recurse(&iter_inst_gateway_array,&iter_inst_gateway_struct);

					dbus_message_iter_get_basic(&iter_inst_gateway_struct,&(ifname)); 								  
					memset(hmd_board_node->Hmd_Inst[j]->Inst_Gateway[k].ifname, 0, strlen(ifname)+1);
					memcpy(hmd_board_node->Hmd_Inst[j]->Inst_Gateway[k].ifname, ifname, strlen(ifname));	
					
					
					dbus_message_iter_next(&iter_inst_gateway_struct);
					dbus_message_iter_get_basic(&iter_inst_gateway_struct,&(hmd_board_node->Hmd_Inst[j]->Inst_Gateway[k].real_ip)); 
					dbus_message_iter_next(&iter_inst_gateway_struct);
					dbus_message_iter_get_basic(&iter_inst_gateway_struct,&(hmd_board_node->Hmd_Inst[j]->Inst_Gateway[k].vir_ip));	
					dbus_message_iter_next(&iter_inst_gateway_struct);
					dbus_message_iter_get_basic(&iter_inst_gateway_struct,&(hmd_board_node->Hmd_Inst[j]->Inst_Gateway[k].remote_r_ip)); 	
					dbus_message_iter_next(&iter_inst_gateway_struct);
					dbus_message_iter_get_basic(&iter_inst_gateway_struct,&(hmd_board_node->Hmd_Inst[j]->Inst_Gateway[k].mask));	
			//		dbus_message_iter_next(&iter_inst_gateway_struct);
			//		dbus_message_iter_get_basic(&iter_inst_gateway_struct,&(hmd_board_node->Hmd_Inst[j]->Inst_Gateway[k].dmask)); 
				
					dbus_message_iter_next(&iter_inst_gateway_array);
				}					
				/*Inst_Gateway  end*/
				
				dbus_message_iter_next(&iter_inst_array);
			}
			/*Hmd_Inst  end*/
			
			dbus_message_iter_next(&iter_board_struct);
			dbus_message_iter_recurse(&iter_board_struct,&iter_l_inst_array);
			
			/*Hmd_Local_Inst  begin*/
			for(j = 0; j < hmd_board_node->LocalInstNum; j++)
			{
				hmd_board_node->Hmd_Local_Inst[j] = (struct Hmd_L_Inst_Mgmt *)malloc(sizeof(struct Hmd_L_Inst_Mgmt));
				dbus_message_iter_recurse(&iter_l_inst_array,&iter_l_inst_struct);
				
				dbus_message_iter_get_basic(&iter_l_inst_struct,&(hmd_board_node->Hmd_Local_Inst[j]->Inst_ID)); 
				dbus_message_iter_next(&iter_l_inst_struct);
				dbus_message_iter_get_basic(&iter_l_inst_struct,&(hmd_board_node->Hmd_Local_Inst[j]->slot_no)); 
				dbus_message_iter_next(&iter_l_inst_struct);
				dbus_message_iter_get_basic(&iter_l_inst_struct,&(hmd_board_node->Hmd_Local_Inst[j]->InstState)); 
				dbus_message_iter_next(&iter_l_inst_struct);
				dbus_message_iter_get_basic(&iter_l_inst_struct,&(hmd_board_node->Hmd_Local_Inst[j]->Inst_UNum)); 
				dbus_message_iter_next(&iter_l_inst_struct);
				dbus_message_iter_get_basic(&iter_l_inst_struct,&(hmd_board_node->Hmd_Local_Inst[j]->Inst_DNum)); 
				dbus_message_iter_next(&iter_l_inst_struct);
				dbus_message_iter_get_basic(&iter_l_inst_struct,&(hmd_board_node->Hmd_Local_Inst[j]->Inst_GNum)); 
				dbus_message_iter_next(&iter_l_inst_struct);
				dbus_message_iter_get_basic(&iter_l_inst_struct,&(hmd_board_node->Hmd_Local_Inst[j]->priority)); 
				dbus_message_iter_next(&iter_l_inst_struct);
				dbus_message_iter_get_basic(&iter_l_inst_struct,&(hmd_board_node->Hmd_Local_Inst[j]->isActive)); 
				char *ifname = NULL;

				/*Inst_Uplink  begin*/
				dbus_message_iter_next(&iter_l_inst_struct);
				dbus_message_iter_recurse(&iter_l_inst_struct,&iter_l_inst_uplink_array);

				for(k = 0; k < (hmd_board_node->Hmd_Local_Inst[j]->Inst_UNum) ; k++)
				{
					dbus_message_iter_recurse(&iter_l_inst_uplink_array,&iter_l_inst_uplink_struct);

					dbus_message_iter_get_basic(&iter_l_inst_uplink_struct,&(ifname)); 							  
					memset(hmd_board_node->Hmd_Local_Inst[j]->Inst_Uplink[k].ifname, 0, strlen(ifname)+1);
					memcpy(hmd_board_node->Hmd_Local_Inst[j]->Inst_Uplink[k].ifname, ifname, strlen(ifname));	
					
					
					dbus_message_iter_next(&iter_l_inst_uplink_struct);
					dbus_message_iter_get_basic(&iter_l_inst_uplink_struct,&(hmd_board_node->Hmd_Local_Inst[j]->Inst_Uplink[k].real_ip)); 
					dbus_message_iter_next(&iter_l_inst_uplink_struct);
					dbus_message_iter_get_basic(&iter_l_inst_uplink_struct,&(hmd_board_node->Hmd_Local_Inst[j]->Inst_Uplink[k].vir_ip));	
					dbus_message_iter_next(&iter_l_inst_uplink_struct);
					dbus_message_iter_get_basic(&iter_l_inst_uplink_struct,&(hmd_board_node->Hmd_Local_Inst[j]->Inst_Uplink[k].remote_r_ip)); 	
					dbus_message_iter_next(&iter_l_inst_uplink_struct);
					dbus_message_iter_get_basic(&iter_l_inst_uplink_struct,&(hmd_board_node->Hmd_Local_Inst[j]->Inst_Uplink[k].mask));	
			//		dbus_message_iter_next(&iter_l_inst_uplink_struct);
			//		dbus_message_iter_get_basic(&iter_l_inst_uplink_struct,&(hmd_board_node->Hmd_Local_Inst[j]->Inst_Uplink[k].dmask)); 
				
					dbus_message_iter_next(&iter_l_inst_uplink_array);
				}
				/*Inst_Uplink  end*/

				/*Inst_Downlink begin*/
				dbus_message_iter_next(&iter_l_inst_struct);
				dbus_message_iter_recurse(&iter_l_inst_struct,&iter_l_inst_downlink_array);

				for(k = 0; k < (hmd_board_node->Hmd_Local_Inst[j]->Inst_DNum) ; k++)
				{
					dbus_message_iter_recurse(&iter_l_inst_downlink_array,&iter_l_inst_downlink_struct);

					dbus_message_iter_get_basic(&iter_l_inst_downlink_struct,&(ifname)); 								  
					memset(hmd_board_node->Hmd_Local_Inst[j]->Inst_Downlink[k].ifname, 0, strlen(ifname)+1);
					memcpy(hmd_board_node->Hmd_Local_Inst[j]->Inst_Downlink[k].ifname, ifname, strlen(ifname));	
					
					
					dbus_message_iter_next(&iter_l_inst_downlink_struct);
					dbus_message_iter_get_basic(&iter_l_inst_downlink_struct,&(hmd_board_node->Hmd_Local_Inst[j]->Inst_Downlink[k].real_ip)); 
					dbus_message_iter_next(&iter_l_inst_downlink_struct);
					dbus_message_iter_get_basic(&iter_l_inst_downlink_struct,&(hmd_board_node->Hmd_Local_Inst[j]->Inst_Downlink[k].vir_ip));	
					dbus_message_iter_next(&iter_l_inst_downlink_struct);
					dbus_message_iter_get_basic(&iter_l_inst_downlink_struct,&(hmd_board_node->Hmd_Local_Inst[j]->Inst_Downlink[k].remote_r_ip)); 	
					dbus_message_iter_next(&iter_l_inst_downlink_struct);
					dbus_message_iter_get_basic(&iter_l_inst_downlink_struct,&(hmd_board_node->Hmd_Local_Inst[j]->Inst_Downlink[k].mask));	
			//		dbus_message_iter_next(&iter_l_inst_downlink_struct);
			//		dbus_message_iter_get_basic(&iter_l_inst_downlink_struct,&(hmd_board_node->Hmd_Local_Inst[j]->Inst_Downlink[k].dmask)); 
				
					dbus_message_iter_next(&iter_l_inst_downlink_array);
				}				
				/*Inst_Downlink  end*/

				/*Inst_Gateway begin*/
				dbus_message_iter_next(&iter_l_inst_struct);
				dbus_message_iter_recurse(&iter_l_inst_struct,&iter_l_inst_gateway_array);

				for(k = 0; k < (hmd_board_node->Hmd_Local_Inst[j]->Inst_GNum) ; k++)
				{
					dbus_message_iter_recurse(&iter_l_inst_gateway_array,&iter_l_inst_gateway_struct);

					dbus_message_iter_get_basic(&iter_l_inst_gateway_struct,&(ifname)); 							  
					memset(hmd_board_node->Hmd_Local_Inst[j]->Inst_Gateway[k].ifname, 0, strlen(ifname)+1);
					memcpy(hmd_board_node->Hmd_Local_Inst[j]->Inst_Gateway[k].ifname, ifname, strlen(ifname));	
					
					
					dbus_message_iter_next(&iter_l_inst_gateway_struct);
					dbus_message_iter_get_basic(&iter_l_inst_gateway_struct,&(hmd_board_node->Hmd_Local_Inst[j]->Inst_Gateway[k].real_ip)); 
					dbus_message_iter_next(&iter_l_inst_gateway_struct);
					dbus_message_iter_get_basic(&iter_l_inst_gateway_struct,&(hmd_board_node->Hmd_Local_Inst[j]->Inst_Gateway[k].vir_ip));	
					dbus_message_iter_next(&iter_l_inst_gateway_struct);
					dbus_message_iter_get_basic(&iter_l_inst_gateway_struct,&(hmd_board_node->Hmd_Local_Inst[j]->Inst_Gateway[k].remote_r_ip)); 	
					dbus_message_iter_next(&iter_l_inst_gateway_struct);
					dbus_message_iter_get_basic(&iter_l_inst_gateway_struct,&(hmd_board_node->Hmd_Local_Inst[j]->Inst_Gateway[k].mask));	
			//		dbus_message_iter_next(&iter_l_inst_gateway_struct);
			//		dbus_message_iter_get_basic(&iter_l_inst_gateway_struct,&(hmd_board_node->Hmd_Local_Inst[j]->Inst_Gateway[k].dmask)); 
				
					dbus_message_iter_next(&iter_l_inst_gateway_array);
				}					
				/*Inst_Gateway  end*/

				dbus_message_iter_next(&iter_l_inst_array);
			}
			/*//Hmd_Local_Inst  end*/
			
			dbus_message_iter_next(&iter_array);
		}
	}
	dbus_message_unref(reply);
	
	return hmd_board_head;
}



int hmd_wireless_check_setting(DBusConnection *dcli_dbus_connection,int slot_id , int state){
	int ret = 0;
	DBusMessage *query, *reply;
	DBusError err;
	
	query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,\
						HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_SET_HANSI_CHECK_STATE);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return ret;
	}
	
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)) {

	}
	dbus_message_unref(reply);
	return ret;
}
/*fengwenchao add 20130412 for hmd timer config save begin*/
int hmd_timer_config_save_state(DBusConnection *dcli_dbus_connection,int slot_id , int state){
	int ret = 0;
	DBusMessage *query, *reply;
	DBusError err;
	
	query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,\
						HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_SET_HMD_TIMER_CONFIG_SAVE_STATE);
	

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&state,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return ret;
	}
	
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)) {

	}
	dbus_message_unref(reply);
	return ret;
}

int hmd_timer_config_save_timer(DBusConnection *dcli_dbus_connection,int slot_id , int timer){
	int ret = 0;
	DBusMessage *query, *reply;
	DBusError err;
	
	query = dbus_message_new_method_call(HMD_DBUS_BUSNAME,HMD_DBUS_OBJPATH,\
						HMD_DBUS_INTERFACE,HMD_DBUS_METHOD_SET_HMD_TIMER_CONFIG_SAVE_TIMER);
	

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&timer,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		if (dbus_error_is_set(&err)) {
			dbus_error_free(&err);
		}
		return ret;
	}
	
	if (dbus_message_get_args ( reply, &err,
					DBUS_TYPE_UINT32,&ret,
					DBUS_TYPE_INVALID)) {

	}
	dbus_message_unref(reply);
	return ret;
}
/*fengwenchao add end*/
























#endif
