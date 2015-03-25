#ifdef _D_WCPSS_
#include <string.h>
#include <stdio.h>
#include <dbus/dbus.h>
#include "wcpss/waw.h"
#include "wcpss/wid/WID.h"
#include "dbus/wcpss/ACDbusDef1.h"
#include "dbus/asd/ASDDbusDef1.h"
#include "wcpss/asd/asd.h"
#include "wid_ac.h"
#include "dbus/wcpss/dcli_wid_radio.h"
#include "asd_sta.h"
typedef enum{
	dcli_radio_check_sectorid=0,
	dcli_radio_check_comma,
	dcli_radio_check_fail,
	dcli_radio_check_end,
	dcli_radio_check_success,
	dcli_radio_check_bar
}radio_sector_state;
typedef enum{
	dcli_radio_check_tx_chainmask=0,
	dcli_radio_check_comma2,
	dcli_radio_check_fail2,
	dcli_radio_check_end2,
	dcli_radio_check_success2,
	dcli_radio_check_bar2
}radio_tx_chainmask_state;

#define SPLIT_COMMA 	','	
int parse_short_ID(char* str,unsigned short* ID){
	 /* before modify*/
	char *endptr = NULL;
	char c;
	unsigned long int t_ID; 
	c = str[0];
	if (c>='0'&&c<='9'){
		t_ID = strtoul(str,&endptr,10);
		if(t_ID < 65536 ){
		  *ID = (unsigned short)t_ID;
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

void delsame_radio
(
	update_radio_list *pradiolist
)
{
	struct tag_radioid *p = NULL,
					   *q = NULL,
					   *temp1 = NULL,
					   *temp2 = NULL;
	
	p = pradiolist->radioidlist;
	if (p == NULL)
	{
		return;
	}
	
	q = p->next;
	if (q == NULL)
	{
		return;
	}
	
	while ((p != NULL) && (p->next != NULL))
	{
		temp1 = p;
		q = p->next;  
		
		while(q)
		{
			if(p->radioid != q->radioid)
			{
				q = q->next;
				temp1 = temp1->next;
			}
			else
			{
				temp2 = q;
				q = q->next;
				temp1->next = q;
				free(temp2);
				temp2 = NULL;
				
				pradiolist->count--;
			}
		}
		p = p->next;
	}
}


void destroy_input_radio_list
(
	update_radio_list *pradiolist
)
{
	if (pradiolist == NULL)
	{
		return;
	}

	struct tag_radioid *phead = NULL;
	struct tag_radioid *pnext = NULL;
	
	phead = pradiolist->radioidlist;
	
	while (phead != NULL)
	{			
		pnext = phead->next;	
		free(phead);
		phead = NULL;
		phead = pnext;
	}	
	
	free(pradiolist);
	pradiolist = NULL;

	return;
}


int process_rate_list(int iArray[],int num) 
{ 

	int iTemp,i,j,iCount=1; 
	/*≈≈–Ú */
	for(i=1;i<num;i++)
	{
		for(j=0;j<i;j++) 
		{ 
			if(iArray[i]<iArray[j]) 
			{ 
				iTemp=iArray[i]; 
				iArray[i]=iArray[j]; 
				iArray[j]=iTemp; 

			} 
		} 
	}
	/*for(i=0;i<num;i++)
	//{
	//	printf("%d ",iArray[i]); 
	//}
	//printf("\n"); 
	//»•÷ÿ∏¥ */
	i=0; 
	j=1; 
	while(j<num) 
	{ 
		if(iArray[i]==iArray[j]) 
		{ 

			j++; 
		} 
		else 
		{ 
			iArray[++i]=iArray[j]; 
			j++; 
			iCount++; 

		} 


	} 
	
	/*for(i=0;i<iCount;i++)
	//{
	//	printf("%d %d \n",i,iArray[i]); 
	//}*/
	return iCount;
}

#if 0
int parse_rate_list(char* ptr,int* count,int rate[])
{
	char* endPtr = NULL;
	int   i=0;
	int  j=0;
	endPtr = ptr;
	rate_list_state state = check_rate;

	while(1){
		switch(state)
		{
			
		case check_rate: 
			
				rate[i] = strtoul(endPtr,&endPtr,10);
			
				if(rate[i]>=10&&rate[i]<=540)
				{
            		state=check_comma;	
				}
				else
					state=check_fail;
				
				break;
		
		case check_comma: 
			
			if (RATE_SPLIT_COMMA == endPtr[0]){
				endPtr = (char*)endPtr + 1;
				state = check_rate;
			
				i++;
				if(i>=20) return -1;
				}
			else
				state = check_end;
			break;
				
		
		case check_fail:
	
			return -1;
			break;

		case check_end: 
	
			if ('\0' == endPtr[0]) {
				state = check_success;
				i++;
				if(i>=20) return -1;
				}
			else
				state = check_fail;
				break;
			
		case check_success: 

			*count = i;
			return 0;
			break;
			
		default: break;
		}
		
		}
		
}

int parse_radio_id(char* ptr,int *wtpid,int *radioid)
{
	
    radio_id_state state = check_wtpid;
	char *str = NULL;
	str = ptr;
   
	while(1){
		switch(state){
			
		case check_wtpid: 
			
			*wtpid = strtoul(str,&str,10);
			
			if(*wtpid > 0 && *wtpid < 4095){
        		state=check_sub;
			}
			else state=check_fail_;
			
			break;

		case check_sub: 
		
			if (PARSE_RADIO_IFNAME_SUB_== str[0]){
				if ('\0' == str[1]){
					state = check_fail_;
				
				}
				else{		
					state = check_radioid;
				}
				}
			else
				state = check_fail_;
			break;

		case check_radioid: 
		
			*radioid = strtoul((char *)&(str[1]),&str,10);

			if(*radioid >= 0 && *radioid < 4){/*radioid range 0 1 2 3*/
        		state=check_end_;
			}
			else state=check_fail_;
			
			break;
		
		case check_fail_:
	
		
            return -1;
			break;

		case check_end_: 
	
			if ('\0' == str[0]) {
				state = check_success_;
				
				}
			else
				state = check_fail_;
				break;
			
		case check_success_: 
		
			return 0;
			break;
			
		default: break;
		}
		
		}
		
}
#endif

int dcli_radio_add_wlanid_node(WID_WTP_RADIO *LIST_RADIO,struct wlanid *node)
{
	if(node == NULL) return NULL;
	
	if(LIST_RADIO->Wlan_Id == NULL){
		LIST_RADIO->Wlan_Id = node;
		LIST_RADIO->Wlan_Id->next = NULL;
	}
	else{
		while(LIST_RADIO->Wlan_Id != NULL){
			LIST_RADIO->Wlan_Id = LIST_RADIO->Wlan_Id->next;
		}
		if(LIST_RADIO->Wlan_Id->next == NULL){
			LIST_RADIO->Wlan_Id->next = node;
			node->next = NULL;
		}
	}
}

int	dcli_radio_add_wds_bss_node(WID_BSS * BSS,unsigned char *MAC){//add
		struct wds_bssid *wds = NULL;

		if(BSS->wds_bss_list == NULL){
			
			wds = malloc(sizeof(struct wds_bssid));
			if(wds == NULL){
				return WID_DBUS_ERROR;
			}
			memset(wds, 0, sizeof(struct wds_bssid));
			memcpy(wds->BSSID, MAC, MAC_LEN);
			wds->next = NULL;
			BSS->wds_bss_list = wds;				
		}else{
			wds = malloc(sizeof(struct wds_bssid));
			if(wds == NULL){
				return WID_DBUS_ERROR;
			}
			memset(wds, 0, sizeof(struct wds_bssid));
			memcpy(wds->BSSID, MAC, MAC_LEN);
			wds->next = NULL;
			wds->next = BSS->wds_bss_list;
			BSS->wds_bss_list = wds;
		}
		return 0;
}
int radio_sector_parse_func(char* sector,unsigned short* sectorid){
	unsigned int ret = 0;
	//char str[4];
//	str = NULL;
	if (!strcmp(sector,"0"))
	{
		*sectorid = 0;
	}		
	else if (!strcmp(sector,"1"))
	{
		*sectorid = 1;
	}
	else if (!strcmp(sector,"2"))
	{
		*sectorid = 2;
	}
	else if (!strcmp(sector,"3"))
	{
		*sectorid = 3;
	}
	else if (!strcmp(sector,"all"))
	{
		*sectorid = 4;
	}
	else
	{
		
		ret = -1;
	}
	return ret ;
}

int radio_Netgear_super_g_technology_parse_func(char* sector,unsigned short* type){
	unsigned int ret = 0;
	if (!strcmp(sector,"bursting"))
	{
		*type = 1;
	}		
	else if (!strcmp(sector,"fastFrame"))
	{
		*type = 2;
	}
	else if (!strcmp(sector,"compression"))
	{
		*type = 3;
	}
	else
	{
		ret = -1;
	}
	return ret ;
}
int dcli_radio_method_parse_first(char *DBUS_METHOD){
	unsigned int sn = 0;
	if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_RADIOLIST))
	{
		sn = 1;
	}		
	else if (!strcmp(DBUS_METHOD,WID_DBUS_CONF_METHOD_SHOWRADIO))
	{
		sn = 2;
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_RADIO_METHOD_SHOW_CHANNEL_CHANGE))
	{
		sn = 3;
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_RADIO_METHOD_SHOW_RADIO_QOS))
	{
		sn = 4;
	}
	else if (!strcmp(DBUS_METHOD,WID_DBUS_RADIO_METHOD_SHOW_BSS_LIST))
	{
		sn = 5;
	}	
	else if(!strcmp(DBUS_METHOD,WID_DBUS_RADIO_METHOD_SHOW_WDS_BSSID_INFO))
	{
		sn = 6;
	}	
	else if(!strcmp(DBUS_METHOD,WID_DBUS_RADIO_METHOD_SHOW_BSS_MAX_THROUGHPUT))
	{
		sn = 7;
	}
	else{}
	return sn ;
}
void dcli_radio_free_fun(char *DBUS_METHOD,DCLI_RADIO_API_GROUP_ONE *RADIOINFO){
		int i = 0;
		int dcli_sn = 0;
		struct wlanid *tmp = NULL;
		dcli_sn = dcli_radio_method_parse_first(DBUS_METHOD);

		switch(dcli_sn){
			case 1 :{
				
				if(RADIOINFO)
				{
					for (i = 0; i < RADIOINFO->radio_num; i++) {
					
						free(RADIOINFO->RADIO[i]);
						RADIOINFO->RADIO[i] = NULL;		
					}
					
						free(RADIOINFO->RADIO);
						RADIOINFO->RADIO = NULL;
						free(RADIOINFO);
						RADIOINFO = NULL;
				}
			}
			break;
			case 2 :{	
				if(RADIOINFO)
				{
						int j = 0;
						if((RADIOINFO)&&(RADIOINFO->RADIO)&&(RADIOINFO->RADIO[0])){
							for(j=0;j<(RADIOINFO->RADIO[0]->Support_Rate_Count);j++)
							{
								CW_FREE_OBJECT(RADIOINFO->RADIO[0]->RadioRate[j]);
							}
							for (i = 0; i < RADIOINFO->bss_num; i++) 
							{
								free(RADIOINFO->RADIO[0]->BSS[i]->BSSID);
								free(RADIOINFO->RADIO[0]->BSS[i]);
							}
							CW_FREE_OBJECT(RADIOINFO->RADIO[0]->WlanId);
							for(i=0;i<RADIOINFO->wlan_num;i++)
							{
								tmp = RADIOINFO->RADIO[0]->Wlan_Id;
								if(tmp->next != NULL);
								{
									RADIOINFO->RADIO[0]->Wlan_Id = tmp->next;
								}
								CW_FREE_OBJECT(tmp->ESSID);
								CW_FREE_OBJECT(tmp);
							}
							free(RADIOINFO->RADIO[0]);
						}
						if(RADIOINFO){
							free(RADIOINFO->RADIO);
							free(RADIOINFO);
							RADIOINFO = NULL;
						}
				}
			}
			break;
			case 3 :{
				CW_FREE_OBJECT(RADIOINFO);
			}
			break;
			case 4 :{
				if(RADIOINFO){
						for (i = 0; i <RADIOINFO->qos_num; i++)
						{
							if(RADIOINFO->RADIO)
								CW_FREE_OBJECT(RADIOINFO->RADIO[i]);
						}
						CW_FREE_OBJECT(RADIOINFO->RADIO);
						CW_FREE_OBJECT(RADIOINFO);
				}
			}
			break;
			case 5 :{
				if(RADIOINFO){
					for (i = 0; i < RADIOINFO->bss_num_int; i++) 
					{	
						if((RADIOINFO->RADIO)&&(RADIOINFO->RADIO[0])&&(RADIOINFO->RADIO[0]->BSS[i]))
						{
							free(RADIOINFO->RADIO[0]->BSS[i]->BSSID);
							RADIOINFO->RADIO[0]->BSS[i]->BSSID = NULL;
							free(RADIOINFO->RADIO[0]->BSS[i]);
							RADIOINFO->RADIO[0]->BSS[i] = NULL; 
						}
					}
					//CW_FREE_OBJECT(RADIOINFO->RADIO[0]->BSS);
					if(RADIOINFO->RADIO){
						CW_FREE_OBJECT(RADIOINFO->RADIO[0]);
						CW_FREE_OBJECT(RADIOINFO->RADIO);
					}
					CW_FREE_OBJECT(RADIOINFO);
				}
			}
			break;
			case 6 :{
				if(RADIOINFO)
				{		
					if((RADIOINFO->BSS)&&(RADIOINFO->BSS[0])){
						CW_FREE_OBJECT(RADIOINFO->BSS[0]->wds_bss_list);
						CW_FREE_OBJECT(RADIOINFO->BSS[0]);		
					}
					CW_FREE_OBJECT(RADIOINFO->BSS);
					CW_FREE_OBJECT(RADIOINFO);
				}
			}
			break;
			case 7 :{
				if(RADIOINFO){
					for (i = 0; i < RADIOINFO->bss_num_int; i++) 
					{
						if((RADIOINFO->WTP)&&(RADIOINFO->WTP[0])&&(RADIOINFO->WTP[0]->WTP_Radio[0]->BSS)&&(RADIOINFO->WTP[0]->WTP_Radio[0]->BSS[i])){
							if(RADIOINFO->WTP[0]->WTP_Radio[0]->BSS[i]->BSSID)
							{
								free(RADIOINFO->WTP[0]->WTP_Radio[0]->BSS[i]->BSSID);
								RADIOINFO->WTP[0]->WTP_Radio[0]->BSS[i]->BSSID = NULL;
							}
							if(RADIOINFO->WTP[0]->WTP_Radio[0]->BSS[i])
							{
								free(RADIOINFO->WTP[0]->WTP_Radio[0]->BSS[i]);
								RADIOINFO->WTP[0]->WTP_Radio[0]->BSS[i] = NULL;
							}
						}
					}
					if((RADIOINFO->WTP)&&(RADIOINFO->WTP[0])&&(RADIOINFO->WTP[0]->WTP_Radio)){
						CW_FREE_OBJECT(RADIOINFO->WTP[0]->WTP_Radio[0]);
					}
					if(RADIOINFO->WTP){
						CW_FREE_OBJECT(RADIOINFO->WTP[0]);
						CW_FREE_OBJECT(RADIOINFO->WTP);
					}
					
					free(RADIOINFO);
					RADIOINFO = NULL;
				}
			}
			break;
			default : break;
			
		}

}
void* dcli_radio_show_api_group_one(
	int index,
	unsigned int id,
	unsigned int* num,
	unsigned int* ret,
	unsigned char *num_char,
	//DCLI_RADIO_API_GROUP_ONE *LIST,
	DBusConnection *dcli_dbus_connection,
	char *DBUS_METHOD
	)
{	
	//int ret;
	int i = 0;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusMessageIter  iter_array;
	
	DBusMessageIter  iter_struct;
	DBusError err;
	DCLI_RADIO_API_GROUP_ONE *LIST = NULL;

	unsigned dcli_sn = 0;	
	int radio_id = id;
	unsigned char wlanid = 0;
	struct wlanid *tmp = NULL,*last = NULL;
	int localid = *num;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);

	dcli_sn = dcli_radio_method_parse_first(DBUS_METHOD);
	if(dcli_sn == 1){
		ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
		dbus_error_init(&err);
	}
	else if(dcli_sn == 2){
		ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&radio_id,
								 DBUS_TYPE_INVALID);
		
	}
	else if(dcli_sn == 3){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&radio_id,
								 DBUS_TYPE_INVALID);
	}
	else if(dcli_sn == 4){
		ReInitDbusPath_V2(localid,index,WID_DBUS_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,WID_DBUS_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
		dbus_error_init(&err);
	}
	else if(dcli_sn == 5){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&radio_id, 
								 DBUS_TYPE_INVALID);
	}
	else if(dcli_sn == 6){
		wlanid = *num_char;
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_BYTE,&wlanid,
								 DBUS_TYPE_UINT32,&radio_id,
								 DBUS_TYPE_INVALID);
	}
	else if(dcli_sn == 7){
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,DBUS_METHOD);
		dbus_error_init(&err);
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&radio_id,
								 DBUS_TYPE_INVALID);
	}
	else if(dcli_sn == 8){}
	else if(dcli_sn == 9){}
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		//printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		*ret = -1;
		return NULL;
	}

	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&(*ret));
	//printf("ccccccccc  ret is %d \n",*ret);
	if(*ret == 0 )
	{	
		CW_CREATE_OBJECT_ERR(LIST, DCLI_RADIO_API_GROUP_ONE, return NULL;); 
		LIST->RADIO = NULL;
		if(dcli_sn == 1){
		//	LIST->RADIO = (WID_WTP_RADIO*)malloc(G_RADIO_NUM*sizeof(WID_WTP_RADIO*));
		//	LIST->RADIO[i] =  (WID_WTP_RADIO*)malloc(sizeof(WID_WTP_RADIO));
		//	LIST->RADIO[i]->WTPMAC = (unsigned char *)malloc(MAC_LEN +1);
		//	memset(LIST->WTP[0]->WTPMAC,0,(MAC_LEN +1));
		
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->radio_num);
			
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);
				
				LIST->RADIO = (WID_WTP_RADIO*)malloc(LIST->radio_num*sizeof(WID_WTP_RADIO*));
				for (i = 0; i < LIST->radio_num; i++) {
					LIST->RADIO[i] = (WID_WTP_RADIO*)malloc(sizeof(WID_WTP_RADIO));
					DBusMessageIter iter_struct;
					
				//	RADIO[i] = (WID_WTP_RADIO*)malloc(sizeof(WID_WTP_RADIO));
					dbus_message_iter_recurse(&iter_array,&iter_struct);
				
					dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[i]->WTPID));
				
					dbus_message_iter_next(&iter_struct);
					
					dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[i]->Radio_G_ID));
				
					dbus_message_iter_next(&iter_struct);
		
					dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[i]->Radio_Type));
				
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[i]->Support_Rate_Count));	
					
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[i]->FragThreshold));	
					
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[i]->BeaconPeriod));	
					
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[i]->IsShortPreamble)); 
					
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[i]->DTIMPeriod));	
					
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[i]->rtsthreshold));
					
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[i]->ShortRetry));
					
					dbus_message_iter_next(&iter_struct);
					dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[i]->LongRetry));
				
					dbus_message_iter_next(&iter_struct);
				
					dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[i]->Radio_Chan));
					
					dbus_message_iter_next(&iter_struct);
				
					dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[i]->Radio_TXP));
					
					dbus_message_iter_next(&iter_struct);
				
					dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[i]->AdStat));
					
					dbus_message_iter_next(&iter_struct);
				
					dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[i]->OpStat));
							
					dbus_message_iter_next(&iter_array);
				}
			
		}
		else if(dcli_sn == 2){	
			unsigned char wlan_id = 0;
			char *ESSID = NULL;

			char* essid = NULL;
			essid =(char*) malloc(32+1);
			memset(essid,0,32);
				
			LIST->RADIO = (WID_WTP_RADIO*)malloc(1*sizeof(WID_WTP_RADIO*));
			LIST->RADIO[0] =  (WID_WTP_RADIO*)malloc(sizeof(WID_WTP_RADIO));
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->WTPID);	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->Radio_L_ID);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->Radio_G_ID);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->Radio_Type);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->Support_Rate_Count);	
			int rate = 0;			
			LIST->RADIO[0]->RadioRate =  (int *)malloc((LIST->RADIO[0]->Support_Rate_Count)*sizeof(int *));
			for (i=0;i<(LIST->RADIO[0]->Support_Rate_Count);i++)
			{
				LIST->RADIO[0]->RadioRate[i] =  (int *)malloc(sizeof(int));
				memset(LIST->RADIO[0]->RadioRate[i],0,4);
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&rate);/*(*(LIST->RADIO[0]->RadioRate[i]))*/
				memcpy(LIST->RADIO[0]->RadioRate[i],&rate,4);
			}
			#if TEST_SWITCH_WAY
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->wlan_num); 
			//LIST->RADIO[0]->WlanId =  (struct wlanid *)malloc((LIST->wlan_num)*sizeof(struct wlanid));
			LIST->RADIO[0]->WlanId =  (unsigned char*)malloc((LIST->wlan_num)*sizeof(unsigned char));
			memset(LIST->RADIO[0]->WlanId, 0,LIST->wlan_num);
			LIST->RADIO[0]->Wlan_Id = NULL;
			int j = 0;

			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);

			for (i = 0; i < LIST->wlan_num; i++)
			{	
				DBusMessageIter iter_struct;
				DBusMessageIter iter_sub_array;

				dbus_message_iter_recurse(&iter_array,&iter_struct);
			
				dbus_message_iter_get_basic(&iter_struct,&wlan_id);
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_recurse(&iter_struct,&iter_sub_array);

				for(j=0;j<ESSID_DEFAULT_LEN;j++){
					DBusMessageIter iter_sub_struct;
					dbus_message_iter_recurse(&iter_sub_array,&iter_sub_struct);
					dbus_message_iter_get_basic(&iter_sub_struct,&essid[j]);
					dbus_message_iter_next(&iter_sub_struct);
					dbus_message_iter_next(&iter_sub_array);
				}
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_next(&iter_array);
				LIST->RADIO[0]->WlanId[i] = wlan_id;
				
				tmp = (struct wlanid *)malloc(sizeof(struct wlanid));
				memset(tmp,0,sizeof(struct wlanid));
				tmp->wlanid = wlan_id;
				tmp->ESSID = (char *)malloc(ESSID_LENGTH+1);
				memset(tmp->ESSID,0,ESSID_LENGTH+1);
				memcpy(tmp->ESSID,essid,strlen(essid));
				tmp->next = NULL;
				
				if(LIST->RADIO[0]->Wlan_Id == NULL)
				{
					LIST->RADIO[0]->Wlan_Id = tmp;
					last = tmp;
				}
				else
				{
					last->next = tmp;
					last = tmp;
				}
				//printf("in pub api,wlanid is %d \n",(LIST->RADIO[0]->WlanId[i]));
			}
			free(essid);
			#endif
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->FragThreshold);	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->BeaconPeriod);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->IsShortPreamble);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->DTIMPeriod);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->rtsthreshold);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->ShortRetry);
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->LongRetry);
						
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->Radio_Chan);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->Radio_TXP);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->AdStat);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->OpStat);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->upcount);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->downcount);				

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->auto_channel_cont);	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->bss_num);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);
			for (i = 0; i < LIST->bss_num; i++) {
				DBusMessageIter iter_struct;
				LIST->RADIO[0]->BSS[i]= (WID_BSS*)malloc(sizeof(WID_BSS));
				LIST->RADIO[0]->BSS[i]->BSSID = (unsigned char *)malloc(6);
				memset(LIST->RADIO[0]->BSS[i]->BSSID,0,6);
				dbus_message_iter_recurse(&iter_array,&iter_struct);

				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->bss_max_allowed_sta_num));
			
				dbus_message_iter_next(&iter_struct);

				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->WlanID));
			
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->Radio_G_ID));
			
				dbus_message_iter_next(&iter_struct);
		
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->BSSID[0]));
			
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->BSSID[1]));
			
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->BSSID[2]));
			
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->BSSID[3]));
			
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->BSSID[4]));
			
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->BSSID[5]));
			
				dbus_message_iter_next(&iter_struct);
			
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->State));
				
				dbus_message_iter_next(&iter_struct);
			
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->BSS_IF_POLICY));

				dbus_message_iter_next(&iter_struct);
			
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->keyindex));/*sz add 05-30*/

				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->upcount));

				dbus_message_iter_next(&iter_struct);	
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->downcount));

				dbus_message_iter_next(&iter_struct);				
			
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->WDSStat));
						
				dbus_message_iter_next(&iter_array);
			}

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->channel_offset);	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->chainmask_num);
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->tx_chainmask_state_value);	
			/* zhangshu add for rx_chainmask, 2010-10-09 */
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->rx_chainmask_state_value);
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->Ampdu.Able);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->Ampdu.AmpduLimit);	

			/* zhangshu add for ampdu & amsdu, 2010-10-09 */
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->Ampdu.subframe);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->Amsdu.Able);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->Amsdu.AmsduLimit);	
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->Amsdu.subframe);
			
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->MixedGreenfield.Mixed_Greenfield);	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->txpowerautostate);	


			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->guardinterval);	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->cwmode);	
			
			//qiuchen add
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->mcs_count);
			//printf("show radio mcs count is %d\n",LIST->RADIO[0]->mcs_count);
			
			for(i=0;i<LIST->RADIO[0]->mcs_count;i++){
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->mcs_list[i]);
			}
			//qiuchen add end
			dbus_message_iter_next(&iter);	                          
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->txpowerstep);	//fengwenchao add 20110329
			dbus_message_iter_next(&iter);	                          
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->Radio_country_code);  /*wcl add for OSDEVTDPB-31*/
		}
		else if(dcli_sn == 3){
				LIST->RADIO = (WID_WTP_RADIO*)malloc(1*sizeof(WID_WTP_RADIO*));
				LIST->RADIO[0] =  (WID_WTP_RADIO*)malloc(sizeof(WID_WTP_RADIO));
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->channelchangetime);

				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&LIST->interval);

	}
		else if(dcli_sn == 4){

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->qos_num);
		
			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);
			LIST->RADIO = (WID_WTP_RADIO*)malloc((LIST->qos_num)*sizeof(WID_WTP_RADIO *));
			for (i = 0; i < LIST->qos_num; i++) {
				LIST->RADIO[i] = (WID_WTP_RADIO*)malloc(sizeof(WID_WTP_RADIO));
				DBusMessageIter iter_struct;
				dbus_message_iter_recurse(&iter_array,&iter_struct);
			
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[i]->Radio_G_ID));
			
				dbus_message_iter_next(&iter_struct);

				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[i]->QOSID));
						
				dbus_message_iter_next(&iter_array);
			}
		}
		else if (dcli_sn == 5){	
			char *nas_id;
			LIST->RADIO = (WID_WTP_RADIO*)malloc(1*sizeof(WID_WTP_RADIO *));
			LIST->RADIO[0] = (WID_WTP_RADIO*)malloc(sizeof(WID_WTP_RADIO));
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->bss_num_int);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->auto_channel);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->diversity);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&LIST->RADIO[0]->txantenna);
		
			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);
		//	LIST->RADIO[0]->BSS = (WID_BSS*)malloc((LIST->bss_num)*sizeof(WID_BSS));
			for (i = 0; i < LIST->bss_num_int; i++) {
				LIST->RADIO[0]->BSS[i] = (WID_BSS*)malloc(sizeof(WID_BSS));
				LIST->RADIO[0]->BSS[i]->BSSID = (unsigned char *)malloc(6);
				memset(LIST->RADIO[0]->BSS[i]->BSSID,0,6);

				DBusMessageIter iter_struct;				
			//	BSS[i] = (WID_BSS *)malloc(sizeof(WID_BSS));
			//	BSS[i]->BSSID = (unsigned char *)malloc(6);
			//	memset(BSS[i]->BSSID,0,6);
				
				dbus_message_iter_recurse(&iter_array,&iter_struct);
			
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->BSSIndex));

				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->WlanID));
			
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->BSSID[0]));

				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->BSSID[1]));

				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->BSSID[2]));

				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->BSSID[3]));

				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->BSSID[4]));

				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->BSSID[5]));

				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->vlanid));

				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->wlan_vlanid));

				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->State));

				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->BSS_IF_POLICY));
				
				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->BSS_TUNNEL_POLICY));				

				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->ath_l2_isolation));

				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->cwmmode));

				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->traffic_limit_able));

				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->traffic_limit));

				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->average_rate));

				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->send_traffic_limit));

				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->send_average_rate));

				dbus_message_iter_next(&iter_struct);
				
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->ip_mac_binding));
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(nas_id));
				memset(LIST->RADIO[0]->BSS[i]->nas_id,0,NAS_IDENTIFIER_NAME);
				memcpy(LIST->RADIO[0]->BSS[i]->nas_id,nas_id,NAS_IDENTIFIER_NAME);
				
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(LIST->RADIO[0]->BSS[i]->hotspot_id));
				dbus_message_iter_next(&iter_array);
			}
		}
		else if(dcli_sn == 6){	
				LIST->BSS = (WID_BSS*)malloc(sizeof(WID_BSS*));					
				LIST->BSS[0] = (WID_BSS*)malloc(sizeof(WID_BSS));	
				LIST->BSS[0]->wds_bss_list = NULL;
				unsigned char *mac = NULL;
				mac = (char*)malloc(MAC_LEN);		
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&LIST->bss_num_int);
				//printf("bss_num is %d\n",LIST->bss_num_int);	
				dbus_message_iter_next(&iter);	
				dbus_message_iter_recurse(&iter,&iter_array);	
				for (i = 0; i < LIST->bss_num_int; i++) {
					memset(mac,0,MAC_LEN);
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
					dcli_radio_add_wds_bss_node(LIST->BSS[0],mac);
				}
				CW_FREE_OBJECT(mac);
			}
		else if(dcli_sn == 7){

			char *bssid = NULL;
			bssid = (unsigned char *)malloc(6);
			LIST->WTP = (WID_WTP*)malloc(1*sizeof(WID_WTP*));
			LIST->WTP[0] =  (WID_WTP*)malloc(sizeof(WID_WTP));
			LIST->WTP[0]->WTP_Radio[0] =  (WID_WTP_RADIO*)malloc(sizeof(WID_WTP_RADIO));
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&LIST->WTP[0]->WTP_Radio[0]->bandwidth);
		
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,&LIST->bss_num_int);

			/*fengwenchao modify begin 20120502 for autelan-2917*/
			dbus_message_iter_next(&iter);	
			dbus_message_iter_recurse(&iter,&iter_array);	
			for(i=0;i<LIST->bss_num_int;i++)
			{
				DBusMessageIter iter_struct;
				dbus_message_iter_recurse(&iter_array,&iter_struct);			
				LIST->WTP[0]->WTP_Radio[0]->BSS[i]= (WID_BSS*)malloc(sizeof(WID_BSS));
				LIST->WTP[0]->WTP_Radio[0]->BSS[i]->BSSID = (unsigned char *)malloc(6);
				memset(LIST->WTP[0]->WTP_Radio[0]->BSS[i]->BSSID,0,6);
				
				dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[0]->BSS[i]->BSSIndex);/*global bssindex, %32 = local bss index*/
				
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[0]->BSS[i]->WlanID);
				
				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[0]->BSS[i]->band_width);

				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&bssid[0]);

				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(bssid[1]));

				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(bssid[2]));

				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(bssid[3]));

				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(bssid[4]));

				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&(bssid[5]));

				dbus_message_iter_next(&iter_struct);
				dbus_message_iter_get_basic(&iter_struct,&LIST->WTP[0]->WTP_Radio[0]->BSS[i]->bss_max_allowed_sta_num);

				dbus_message_iter_next(&iter_array);
				memset(LIST->WTP[0]->WTP_Radio[0]->BSS[i]->BSSID,0,6);
				memcpy(LIST->WTP[0]->WTP_Radio[0]->BSS[i]->BSSID,bssid,6);
			}
			/*fengwenchao modify end*/
			if(bssid != NULL){
				free(bssid);
				bssid = NULL;
			}
		}
		else if(dcli_sn == 8){}
		else if (dcli_sn == 9){}
	}
	dbus_message_unref(reply);
	return LIST;

}
int radio_set_method_function(int localid, int index,int radioid,int wlanid,unsigned short resv_short,unsigned int resv_int2,char* resv_char,char* METHOD,DBusConnection *dcli_dbus_connection)
{
	//unsigned int radio_id; 
	int ret;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;	
	
	//int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	if (!strcmp(METHOD,WID_DBUS_RADIO_SECTOR_SET_CMD))
	{
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,METHOD);
		dbus_error_init(&err);
		
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&radioid,
								 DBUS_TYPE_UINT16,&resv_short,/*sector value*/
								 DBUS_TYPE_UINT32,&resv_int2,
								 DBUS_TYPE_INVALID);
	}		
	else if (!strcmp(METHOD,WID_DBUS_RADIO_SECTOR_POWER_SET_CMD))
	{
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,METHOD);
		dbus_error_init(&err);
		
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&radioid,
								 DBUS_TYPE_UINT16,&resv_short,/*sector num*/
								 DBUS_TYPE_UINT32,&resv_int2,/*sector value*/
								 DBUS_TYPE_INVALID);
	}
	else if (!strcmp(METHOD,WID_DBUS_RADIO_NETGEAR_G_SET_CMD))
	{
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,METHOD);
		dbus_error_init(&err);
		
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&radioid,
								 DBUS_TYPE_UINT16,&resv_short,/*netgear G type*/
								 DBUS_TYPE_UINT32,&resv_int2,/*netgear G state*/
								 DBUS_TYPE_INVALID);
	}
	else
	{
		ret = -1;
	}
	if(ret != -1){
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		//vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		ret = -1;
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
	}
	return ret;

}
int radio_set_method_functiontwo(int localid, int index,int radioid,int wlanid,unsigned char resv_tx_char,unsigned int resv_int2,char* resv_char,char* METHOD,DBusConnection *dcli_dbus_connection)
{
	//unsigned int radio_id; 
	
	int ret;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;	
	
	//int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);

    if (!strcmp(METHOD,WID_DBUS_RADIO_TX_CHAINMASK_SET_CMD))
	{
	
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,METHOD);
		dbus_error_init(&err);
		
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&radioid,
								 DBUS_TYPE_BYTE,&resv_tx_char,/*tx_chainmask value*/
								 DBUS_TYPE_UINT32,&resv_int2,
								 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		//vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		ret = -1;
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
	}		
	else
	{
		ret = -1;
	}
	return ret;

}

#if 0
int radio_set_method_tx_chainmask(int localid,  int index,unsigned int radioid,unsigned char policy,DBusConnection *dcli_dbus_connection)
{
	//unsigned int radio_id; 
	
	int ret;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;	
	
	//int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);

	
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_TX_CHAINMASK_SET_CMD_V2);
		dbus_error_init(&err);
		
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&radioid,
								 DBUS_TYPE_BYTE,&policy,/*tx_chainmask value*/
								 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		//vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free(&err);
		}
		ret = -1;
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
	return ret;

}
#endif

/* zhangshu add for set chainmask value,2010-10-09 */
int radio_set_method_chainmask(int localid, int index,unsigned int radioid,unsigned char policy, unsigned char type, DBusConnection *dcli_dbus_connection)
{
	//unsigned int radio_id; 
	
	int ret;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;	
	
	//int index;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);

	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_TX_CHAINMASK_SET_CMD_V2);
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radioid,
							 DBUS_TYPE_BYTE,&policy,/* chainmask value, zhangshu modify 2010-10-09*/
							 DBUS_TYPE_BYTE,&type,/* chainmask type, zhangshu add 2010-10-09 */
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		//vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			//vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		ret = -1;
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);
	return ret;

}
/* zhangshu add end,2010-10-09 */

int radio_set_wds_bridge_distance(int localid, int index,int RadioID,int distance,DBusConnection *dcli_dbus_connection)
{
	int ret;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;	
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_WDS_DISTANCE);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&RadioID,
							 DBUS_TYPE_UINT32,&distance,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return 0;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);
	return ret;

}

int radio_set_wds_remote_brmac(int localid, int index,int RadioID,int is_add, unsigned char *macAddr,DBusConnection *dcli_dbus_connection)
{
	int ret;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;	
	
	printf("radioid %d\n",RadioID);
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_WDS_REMOTE_BRMAC);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&RadioID,
							 DBUS_TYPE_UINT32,&is_add,
							 DBUS_TYPE_BYTE,&macAddr[0],
							 DBUS_TYPE_BYTE,&macAddr[1],
							 DBUS_TYPE_BYTE,&macAddr[2],
							 DBUS_TYPE_BYTE,&macAddr[3],
							 DBUS_TYPE_BYTE,&macAddr[4],
							 DBUS_TYPE_BYTE,&macAddr[5],
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return 0;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);
	return ret;

}

int radio_set_wds_wep_key(int localid, int index,int RadioID,char *key,DBusConnection *dcli_dbus_connection)
{
	int ret;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;	
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_WDS_WEP_KEY);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&RadioID,
							 DBUS_TYPE_STRING,&key,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return 0;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);
	return ret;

}

int radio_set_wds_encryption_type(int localid, int index,int RadioID,int type,DBusConnection *dcli_dbus_connection)
{
	int ret;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;	
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_WDS_ENCRYPTION_TYPE);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&RadioID,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return 0;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);
	return ret;

}

int radio_set_wds_aes_key(int localid, int index,int RadioID,char *key, unsigned char *macAddr,DBusConnection *dcli_dbus_connection)
{
	int ret;
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;	
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_WDS_AES_KEY);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&RadioID,
							 DBUS_TYPE_STRING,&key,
							 DBUS_TYPE_BYTE,&macAddr[0],
							 DBUS_TYPE_BYTE,&macAddr[1],
							 DBUS_TYPE_BYTE,&macAddr[2],
							 DBUS_TYPE_BYTE,&macAddr[3],
							 DBUS_TYPE_BYTE,&macAddr[4],
							 DBUS_TYPE_BYTE,&macAddr[5],
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return 0;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);
	return ret;

}

int parse_radio_sector_list(char* ptr,short *id)
{
	char* endPtr = NULL;
	int   sectorid1 = 0;
	int   sectorid2 = 0;
	int   i = 0;
	endPtr = ptr;
	radio_sector_state state = dcli_radio_check_sectorid;
//	printf("00000000000 *id is %d\n",*id);
	while(1){
		switch(state){
			case dcli_radio_check_sectorid: 
				sectorid1 = strtoul(endPtr,&endPtr,10);
				if(sectorid1>=0&&sectorid1<SECTOR_NUM)	{
					state=dcli_radio_check_comma;
				}else
					state=dcli_radio_check_fail;
				break;
			case dcli_radio_check_comma: 
				if(SPLIT_COMMA == endPtr[0]){
					endPtr = (char*)endPtr + 1;
					state = dcli_radio_check_sectorid;
					*id |= 1<<sectorid1;	
				//	printf("1111111111111 sectorid1 is %d\n",sectorid1);
				//	printf("2222222222222 *id is %d\n",*id);
					i++;
				}else
					state = dcli_radio_check_end;
				break;
			case dcli_radio_check_fail:
				return -1;
			case dcli_radio_check_end: 
				if ('\0' == endPtr[0]) 	{
					state = dcli_radio_check_success;
				}else
					state = dcli_radio_check_fail;
				break;
			case dcli_radio_check_success: 
				*id |= 1<<sectorid1; 
			//	printf("3333333333333 sectorid1 is %d\n",sectorid1);
			//	printf("4444444444444 *id is %d\n",*id);
				return 0;
				break;
			default:
				break;
		}
		
	}
	
}
int parse_radio_tx_chainmask_list(char* ptr,unsigned char *id)
{
	char* endPtr = NULL;
	int   tx_chainmask = 0;
	int   i = 0;
	endPtr = ptr;
	radio_tx_chainmask_state  state = dcli_radio_check_tx_chainmask;
	
	while(1){
		switch(state){
			case dcli_radio_check_tx_chainmask: 
				tx_chainmask = strtoul(endPtr,&endPtr,10);
				if(tx_chainmask>=0&&tx_chainmask<TX_CHANIMASK_NUM)	{
					state=dcli_radio_check_comma2;
				}else
					state=dcli_radio_check_fail2;
				break;
			case dcli_radio_check_comma2: 
				if(SPLIT_COMMA == endPtr[0]){
					endPtr = (char*)endPtr + 1;
					state = dcli_radio_check_tx_chainmask;
					*id |= 1<<tx_chainmask;	
					i++;
				}else
					state = dcli_radio_check_end2;
				break;
			case dcli_radio_check_fail2:
				return -1;
			case dcli_radio_check_end2: 
				if ('\0' == endPtr[0]) 	{
					state = dcli_radio_check_success2;
				}else
					state = dcli_radio_check_fail2;
				break;
			case dcli_radio_check_success2: 
				*id |= 1<<tx_chainmask; 
				return 0;
				break;
			default:
				break;
		}
		
	}
	
}

int conversion_parameter_hex(int **sectorID,int num,int policy,int *hex_id2)
{

	int i = 0;
	int id = 0;
	int hex_id = 0; 
	for(i=0;((i<num)&&(i<SECTOR_NUM));i++){

		//	printf("sector %d is %d.\n",i,sectorID[i]);
			id = sectorID[i];
		//	if(policy == 1){
				switch(id)
				{
					case 0:
					{
						hex_id |= 0x0001;
						
					}
						break;
					case 1:
					{
						hex_id |= 0x0010;
					}
						break;
					case 2:
					{
						hex_id |= 0x0100;
					}
						break;
					case 3:
					{
						hex_id |= 0x1000;
					}
						break;
					default:
						
						break;
				}
		//	}
		/*	else if(policy == 1){
				switch(id)
				{
					case 0:
					{
						hex_id &= ~0x0001;
						
					}
						break;
					case 1:
					{
						hex_id &= ~0x0010;
					}
						break;
					case 2:
					{
						hex_id &= ~0x0100;
					}
						break;
					case 3:
					{
						hex_id &= ~0x1000;
					}
						break;
					default:
						
						break;
				}
			}*/
			
	}
	*hex_id2 = hex_id;
	return 0;
}

void wid_set_sta_info(DBusConnection *dcli_dbus_connection,int index, unsigned char mac[MAC_LEN], 
	unsigned char wlan_id, unsigned int radio_id, unsigned char type, unsigned int value, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError	err;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	char METHOD[PATH_LEN];

	memset(BUSNAME,0,PATH_LEN);
	memset(OBJPATH,0,PATH_LEN);
	memset(INTERFACE,0,PATH_LEN);
	memset(METHOD,0,PATH_LEN);

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	if(type == VLANID){
		memcpy(METHOD,WID_DBUS_RADIO_METHOD_WLAN_SET_STA_VLANID,strlen(WID_DBUS_RADIO_METHOD_WLAN_SET_STA_VLANID));
	}else if(type == LIMIT) {
		memcpy(METHOD,WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_STA_VALUE,strlen(WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_STA_VALUE));
	}else if(type == SEND_LIMIT){
		memcpy(METHOD,WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_STA_SEND_VALUE,strlen(WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_STA_SEND_VALUE));
	}
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,METHOD);
	
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
								DBUS_TYPE_BYTE,&wlan_id,
								DBUS_TYPE_UINT32,&radio_id,
								DBUS_TYPE_UINT32,&value,
								DBUS_TYPE_BYTE,&mac[0],
								DBUS_TYPE_BYTE,&mac[1],
								DBUS_TYPE_BYTE,&mac[2],
								DBUS_TYPE_BYTE,&mac[3],
								DBUS_TYPE_BYTE,&mac[4],
								DBUS_TYPE_BYTE,&mac[5],
								DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return ;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_unref(reply);
	
	return ;
}

void asd_set_sta_info(DBusConnection *dcli_dbus_connection,int index, unsigned char mac[MAC_LEN], 
	unsigned char wlan_id, unsigned int radio_id, unsigned char type, unsigned int value, int localid, unsigned int *ret)
{
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError	err;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	char METHOD[PATH_LEN];

	memset(BUSNAME,0,PATH_LEN);
	memset(OBJPATH,0,PATH_LEN);
	memset(INTERFACE,0,PATH_LEN);
	memset(METHOD,0,PATH_LEN);

	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);

	if(type == LIMIT) {
		memcpy(METHOD,ASD_DBUS_STA_METHOD_STA_TRAFFIC_LIMIT,strlen(ASD_DBUS_STA_METHOD_STA_TRAFFIC_LIMIT));
	}else if(type == SEND_LIMIT){
		memcpy(METHOD,ASD_DBUS_STA_METHOD_STA_SEND_TRAFFIC_LIMIT,strlen(ASD_DBUS_STA_METHOD_STA_SEND_TRAFFIC_LIMIT));
	}
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,METHOD);
	
	dbus_error_init(&err);
	
	dbus_message_append_args(query,
								DBUS_TYPE_BYTE,&wlan_id,
								DBUS_TYPE_UINT32,&radio_id,
								DBUS_TYPE_UINT32,&value,
								DBUS_TYPE_BYTE,&mac[0],
								DBUS_TYPE_BYTE,&mac[1],
								DBUS_TYPE_BYTE,&mac[2],
								DBUS_TYPE_BYTE,&mac[3],
								DBUS_TYPE_BYTE,&mac[4],
								DBUS_TYPE_BYTE,&mac[5],
								DBUS_TYPE_INVALID);
	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		*ret = ASD_DBUS_ERROR;
		if (dbus_error_is_set(&err)) {
			dbus_error_free_for_dcli(&err);
		}
		return ;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	dbus_message_unref(reply);
	
	return ;
}


int radio_set_radio_inter_vap_type(int localid, int index,int RadioID,unsigned char policy,DBusConnection *dcli_dbus_connection)
{
	DBusMessage *query, *reply;	
	DBusMessageIter	 iter;
	DBusError err;
	
	unsigned int ret;
	
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_INTER_VAP_FORVARDING_ABLE);


	dbus_error_init(&err);
	dbus_message_append_args(query,
							DBUS_TYPE_UINT32,&RadioID,
							DBUS_TYPE_BYTE,&policy,
							DBUS_TYPE_INVALID);

	
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		//vty_out(vty,"<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
		//	vty_out(vty,"%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		ret = -1;
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);
	return ret;
}
int dcli_wlan_wtp_list_sta_static_arp(int localid, int index,int policy,unsigned char wlanid,struct tag_wtpid_list * wtplist,char *ifname,DBusConnection *dcli_dbus_connection){
	DBusMessage *query, *reply;
	DBusMessageIter  iter;
	DBusMessageIter	 iter_array;/*wcl add for AUTELAN-2836*/
	DBusError err;
	int i;
	struct tag_wtpid *tmp = NULL;
	unsigned int num;	
	int ret;
	dbus_error_init(&err);

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_WTP_LIST_METHOD_STA_STATIC_ARP);

	num = wtplist->count;
	dbus_message_iter_init_append (query, &iter);
			

	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &policy);
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_BYTE,
										 &wlanid);
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_STRING,
										 &ifname);
		// Total slot count
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &num);
	dbus_message_iter_open_container (&iter,
							DBUS_TYPE_ARRAY,
							DBUS_STRUCT_BEGIN_CHAR_AS_STRING
							DBUS_TYPE_UINT32_AS_STRING
							DBUS_STRUCT_END_CHAR_AS_STRING,
							&iter_array);/*wcl add for AUTELAN-2836*/
	tmp = wtplist->wtpidlist;
/*	for(i = 0; i < num; i++){
		
		dbus_message_iter_append_basic (&iter,
											 DBUS_TYPE_UINT32,
											 &(tmp->wtpid));
		tmp = tmp->next;

	}	*/
	
	for(i = 0; i < num; i++){
		DBusMessageIter iter_struct;
			
		dbus_message_iter_open_container(&iter_array,DBUS_TYPE_STRUCT,NULL,&iter_struct);
		
		dbus_message_iter_append_basic(&iter_struct,DBUS_TYPE_UINT32,&(tmp->wtpid));
		dbus_message_iter_close_container (&iter_array, &iter_struct);
		
		tmp = tmp->next;
	}
	dbus_message_iter_close_container (&iter, &iter_array);/*wcl add for AUTELAM-2836*/
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);


	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);	

	//destroy_input_wtp_list(wtplist);
	return ret;
}

int dcli_wlan_wtp_sta_static_arp(int localid, int index,int policy,unsigned char wlanid,unsigned int radioid,char * ifname, DBusConnection *dcli_dbus_connection){
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int i;
	int ret;
	dbus_error_init(&err);

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_WLAN_WTP_METHOD_STA_STATIC_ARP);

	dbus_message_iter_init_append (query, &iter);
			

	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &policy);

	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_BYTE,
										 &wlanid);
		// Total slot count
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &radioid);

	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_STRING,
										 &ifname);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);


	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);	

	return ret;
}

/* zhangshu modify for 11n para, 2010-10-09 */
int dcli_radio_11n_set_ampdu_able(int localid, int index,unsigned char policy,unsigned int radioid,unsigned char type,DBusConnection *dcli_dbus_connection){
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int i;
	int ret;
	dbus_error_init(&err);

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_AMPDU_ABLE);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radioid,
							 DBUS_TYPE_BYTE,  &policy,
							 DBUS_TYPE_BYTE,  &type,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);	

	return ret;
}

/* zhangshu modify for 11n para, 2010-10-09 */
int dcli_radio_11n_set_ampdu_limit(int localid, int index,unsigned int radioid,unsigned int limit,unsigned char type, DBusConnection *dcli_dbus_connection){
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int i;
	int ret;
	//dbus_error_init(&err);

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_AMPDU_LIMIT);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radioid,
							 DBUS_TYPE_UINT32,&limit,
							 DBUS_TYPE_BYTE, &type,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);	

	return ret;
}

/* zhangshu add for 11n para, 2010-10-09 */
int dcli_radio_11n_set_ampdu_subframe(int localid, int index,unsigned int radioid,unsigned char subframe,unsigned char type, DBusConnection *dcli_dbus_connection){
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int i;
	int ret;
	//dbus_error_init(&err);

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_AMPDU_SUBFRAME);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radioid,
							 DBUS_TYPE_BYTE, &subframe,
							 DBUS_TYPE_BYTE, &type,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);	

	return ret;
}


int dcli_radio_11n_set_mixed_puren_switch(int localid, int index,unsigned char policy,unsigned char wlanid,unsigned int radioid,DBusConnection *dcli_dbus_connection){
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int i;
	int ret;
	//dbus_error_init(&err);

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_MIXED_PURE_N);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radioid,
							 DBUS_TYPE_BYTE,  &wlanid,
							 DBUS_TYPE_BYTE,  &policy,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);	

	return ret;
}

void dcli_radio_11n_channel_offset(int localid, int index,unsigned int radioid,char policy,int *ret,unsigned int *max_channel,unsigned int *min_channel,unsigned char *current_channel,DBusConnection *dcli_dbus_connection){
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int i;
	
	//dbus_error_init(&err);

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_CHANNEL_OFFSET);

	dbus_error_init(&err);

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radioid,
							 DBUS_TYPE_BYTE,&policy,
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);

	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	if(*ret == CHANNEL_CWMODE_HT40){

		 dbus_message_iter_next(&iter);	
		 dbus_message_iter_get_basic(&iter,max_channel);
		 dbus_message_iter_next(&iter);	    
		 dbus_message_iter_get_basic(&iter,min_channel);
		 dbus_message_iter_next(&iter);	    
		 dbus_message_iter_get_basic(&iter,current_channel);
		 
	 }

	dbus_message_unref(reply);	


}

int radio_set_txpower_step(int localid, int index,unsigned int radio_id,unsigned short txpowerstep,DBusConnection *dcli_dbus_connection)
{
    DBusMessage *query = NULL;
	DBusMessage	*reply = NULL; 
	DBusMessageIter  iter;
	DBusError err;
	int ret = 0;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_TXPOWER_STEP);
	dbus_error_init(&err);
	//printf("radio_id = %d\n",radio_id);
	//printf("txpowerstep = %d\n",txpowerstep);
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_UINT16,&txpowerstep,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);	
	return ret;
}
/*fengwenchao add 20120222 for RDIR-25*/
int set_radio_wlan_limit_rssi_access_sta_set(int localid,int index,unsigned int radio_id,unsigned char wlanid,unsigned char rssi,DBusConnection *dcli_dbus_connection)
{
  	DBusMessage *query = NULL;
	DBusMessage	*reply = NULL; 
	DBusMessageIter  iter;
	DBusError err;
	int ret = 0;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_RADIO_WLAN_LIMIT_RSSI_ACCESS_STA);
	dbus_error_init(&err);									

	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&radio_id,
							 DBUS_TYPE_BYTE,&wlanid,
							 DBUS_TYPE_BYTE,&rssi,
							 DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);
	dbus_message_unref(query);
	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);
	dbus_message_unref(reply);	
	return ret;

}
/*fengwenchao add end*/
int dcli_radio_service_control_timer(int localid, int index,int policy,unsigned int radioid,int is_once,int wday,int time,DBusConnection *dcli_dbus_connection){
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int i;
	int ret;
	dbus_error_init(&err);

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SERVICE_CONTROL_TIMER);

	dbus_message_iter_init_append (query, &iter);
			

	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &policy);

		// Total slot count
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &radioid);

	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &is_once);
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &time);
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &wday);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);


	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);	

	return ret;
}
int dcli_radio_timer_able(int localid, int index,int policy,int timer,unsigned int radioid,DBusConnection *dcli_dbus_connection){
	DBusMessage *query, *reply; 
	DBusMessageIter  iter;
	DBusError err;
	int i;
	int ret;
	dbus_error_init(&err);

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_TIMER_ABLE);

	dbus_message_iter_init_append (query, &iter);
			

	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &policy);
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &timer);
	dbus_message_iter_append_basic (&iter,
										 DBUS_TYPE_UINT32,
										 &radioid);

	reply = dbus_connection_send_with_reply_and_block (dcli_dbus_connection,query,-1, &err);

	dbus_message_unref(query);


	if (NULL == reply) {
		printf("<error> failed get reply.\n");
		if (dbus_error_is_set(&err)) {
			printf("%s raised: %s",err.name,err.message);
			dbus_error_free_for_dcli(&err);
		}
		return -1;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	dbus_message_unref(reply);	

	return ret;
}
struct WtpList * set_radio_channel_cmd_channel(int localid, int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char channel,int *count,int *ret,int *ret1,int *ret2,
	unsigned short *cwmode,char *channel_offset)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_SET_CHAN);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&channel,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(type==0)
		{	
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,ret1);
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,ret2);
			if(*ret2 != CHANNEL_CWMODE_SUCCESS)
				{
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,cwmode);
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,channel_offset);
				}			
		}
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;	
}

struct WtpList * set_radio_txpower_cmd_txpower(int localid, int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,u_int16_t txp,int *count,int *ret,int *ret1)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;
	//printf("struct :  txp = %d \n",txp);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_SET_TXP);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT16,&txp,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	if(type==0)
		{
			//printf("ZZZZZZZZZZZZZZZZZZZZZZZ\n");
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,ret1);
		}
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}

	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}

struct WtpList * set_radio_txpowerof_cmd_txpoweroffset(int localid, int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,u_int16_t txpof,int *count,int *ret,int *ret1)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;
	//printf("struct :  txpof = %d \n",txpof);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_SET_TXPOF);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT16,&txpof,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	if(type==0)
		{
			//printf("ZZZZZZZZZZZZZZZZZZZZZZZ\n");
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,ret1);
		}
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}

	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}

struct WtpList * set_radio_ratelist_cmd_set_support_ratelist(int localid, int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,int num,int *count,int *ret,int *ret1,int *list,int *mode)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	//int modes = 0;
	//int nums = 0;
	int list1[RADIO_RATE_LIST_LEN];
	int list2[RADIO_RATE_LIST_LEN];
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	for(i=0;i<num;i++)
		{
			list2[i] = list[i];
		}

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_SET_SUPPORT_RATELIST);
		
	dbus_error_init(&err);

	/*dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&num,
									DBUS_TYPE_INVALID);
	*/
	dbus_message_iter_init_append (query, &iter);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&type);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&id);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&num);
		
	for(i = 0; i < num; i++)
	{
		//printf("list[i]= %d\n",list2[i]);
		dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&list2[i]);
		//printf("list[i]= %d\n",list2[i]);
	}

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(type==0)
		{
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,mode);
		
			if(*ret == 0)
			{
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,count);
				if(*count > 20)
					{
						*ret1 = -1;
					}
				
				for(i=0;i<(*count);i++)
				{
					dbus_message_iter_next(&iter);
					dbus_message_iter_get_basic(&iter,&list1[i]);
					list[i] = list1[i];
				}
			}		
	
		}
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}


void* set_radio_max_rate_cmd_set_max_rate_v1(int localid, int index,DBusConnection *dbus_connection,
	unsigned int id,int rate,int *ret,int *ret1,int *num,int *mode,int *list1,
	struct dcli_n_rate_info *nRateInfo, unsigned char *chan,int wflag)
{
    //printf(" id = %d, rate = %d, wflag = %d\n",id,rate,wflag);
	int i = 0;
	int radioid = 0;
	int modes = 0;
	int list2[RADIO_RATE_LIST_LEN];
	
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];	
	
	dbus_error_init(&err);
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_SET_MAX_RATE);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&rate,
									DBUS_TYPE_UINT32,&wflag,
									DBUS_TYPE_INVALID);
	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	dbus_message_iter_next(&iter);
	dbus_message_iter_get_basic(&iter,mode);
	
	if(*ret == 0)
	{
	    if(mode < 8)
        {
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,num);
			//*num = nums;
			if(*num > 20)
			{
				//vty_out(vty,"<error> an unexpect error\n");
				//return CMD_SUCCESS;
				*ret1 = -1;
			}
			for(i=0;i<*num;i++)
			{
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,&list2[i]);
				list1[i]=list2[i];
			}			
		}
		else if(wflag == 0)
		{
		    dbus_message_iter_next(&iter);
    		dbus_message_iter_get_basic(&iter,&(nRateInfo->mcs));
    		dbus_message_iter_next(&iter);
    		dbus_message_iter_get_basic(&iter,&(nRateInfo->cwmode));
    		dbus_message_iter_next(&iter);
    		dbus_message_iter_get_basic(&iter,&(nRateInfo->guard_interval));
    		dbus_message_iter_next(&iter);
    		dbus_message_iter_get_basic(&iter,chan);
    		//printf("mcs = %d, cwmode = %d, gi = %d, chan = %d\n",nRateInfo->mcs,nRateInfo->cwmode,nRateInfo->guard_interval,*chan);
		}
	}
	return NULL;
}


struct WtpList * set_radio_max_rate_cmd_set_max_rate(int localid, int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,int rate,int *count,int *ret,int *ret1,int *num,int *mode,int *list1)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	int modes = 0;
	int nums = 0;
	int list2[RADIO_RATE_LIST_LEN];
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_SET_MAX_RATE);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&rate,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if(type==0)
		{
			dbus_message_iter_next(&iter);
			dbus_message_iter_get_basic(&iter,mode);
			//*mode =modes;
		
			if(*ret == 0)
			{
				dbus_message_iter_next(&iter);
				dbus_message_iter_get_basic(&iter,num);
				//*num = nums;
				if(*num > 20)
				{
					//vty_out(vty,"<error> an unexpect error\n");
					//return CMD_SUCCESS;
					*ret1 = -1;
				}
				for(i=0;i<*num;i++)
				{
					dbus_message_iter_next(&iter);
					dbus_message_iter_get_basic(&iter,&list2[i]);
					list1[i]=list2[i];
				}			
			}
		}
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}

struct WtpList * set_radio_mode_cmd_11a_11b_11g_11bg_11bgn_11an(int localid, int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned int mode,int *list,int *count,int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	//int list[RADIO_RATE_LIST_LEN];
	int i = 0;
	int j = 0;
	int radioid = 0;
	int successfulnum = 0;
	int successfulradioid = 0;
	int rate = 0;
	int ratenum = 0;
	char fail_reason = 0;
	int list1[RADIO_RATE_LIST_LEN];
	//int list[RADIO_RATE_LIST_LEN];
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;
	struct RadioRate_list *radiorate_list = NULL;
	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_SET_MODE);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&mode,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if((type == 0)&&(*ret==0))
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,count);

			if(*count > 20)
				{
					*ret = -1;
				}
			else
				{
					for(i=0;i<*count;i++)
					{
						dbus_message_iter_next(&iter);
						dbus_message_iter_get_basic(&iter,&list1[i]);
						list[i] = list1[i];
					}
				}
		}
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	

		}
		
	dbus_message_unref(reply);
	return radio_list_head ;

}

struct WtpList * set_radio_beaconinterval_cmd_beaconinterval(int localid, int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned short beaconinterval,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_SET_BEACON);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT16,&beaconinterval,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}

struct WtpList * set_radio_fragmentation_cmd_fragmentation(int localid, int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned short fragmentation,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_SET_FRAGMENTATION);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT16,&fragmentation,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}

struct WtpList * set_radio_dtim_cmd_dtim(int localid, int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char dtim,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_SET_DTIM);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&dtim,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;	
}

struct WtpList * set_radio_rtsthreshold_cmd_rtsthreshold(int localid, int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned short rtsthre,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_SET_RTSTHROLD);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT16,&rtsthre,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;	
}

struct WtpList * set_radio_service_cmd_radio_enable_disable(int localid, int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char status,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_SET_STATUS);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&status,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}

struct WtpList * set_wds_service_cmd_wlan_wds(int localid, int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char wlanid,unsigned char status,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_SET_WDS_STATUS);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&wlanid,
									DBUS_TYPE_BYTE,&status,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;	
}

struct WtpList * set_radio_preamble_cmd_preamble(int localid, int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char preamble,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_SET_PREAMBLE);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&preamble,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;	
}

struct WtpList * set_radio_longretry_cmd_longretry(int localid, int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char longretry,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_SET_LONGRETRY);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&longretry,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;	
}

struct WtpList * set_radio_shortretry_cmd_shortretry(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char shortretry,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_SET_SHORTRETRY);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&shortretry,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}
int set_radio_bss_max_num_asd_wid(int localid,  int index,DBusConnection *dbus_connection,unsigned int id,
	unsigned int type,unsigned int max_sta_num,unsigned int wlanid,int *ret)   //fengwenchao modify 20110512
{
	int stanum = 0;
	unsigned char wlan_wid = 0;  //fengwenchao add 20110512
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_GET_STA_INFO);

	printf("3333333333333333333333333333\n");
	//bssid=id*L_BSS_NUM+bss_index-1;
	dbus_error_init(&err);
		
	dbus_message_append_args(query,
							 DBUS_TYPE_UINT32,&type,
							 DBUS_TYPE_UINT32,&wlanid,    //fengwenchao modify 20110512
							  DBUS_TYPE_UINT32,&id,   	   //fengwenchao modify 20110512
							 DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
		
	dbus_message_unref(query);
		
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&stanum);
	dbus_message_unref(reply);

	printf("stanum = %d\n",stanum);

	if(stanum==-1)
		{
			*ret = BSS_NOT_EXIST;		
			//return *ret;
		}
	else if(max_sta_num< stanum)
		{
			*ret = SET_MAX_STANUM_SMALLER_THAN_CURRENT_STANUM;	
			//return *ret;
		}
	else
		{
			memset(BUSNAME,0,PATH_LEN);
			memset(OBJPATH,0,PATH_LEN);
			memset(INTERFACE,0,PATH_LEN);
			wlan_wid = (unsigned char)wlanid;  //fengwenchao add 20110512
			ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
			ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
			ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
			query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_SET_BSS_MAX_STA);

			dbus_message_append_args(query,
									 DBUS_TYPE_UINT32,&id,
									 DBUS_TYPE_BYTE,&wlan_wid,     //fengwenchao modify 20110512
							 		DBUS_TYPE_UINT32,&max_sta_num,
							 		DBUS_TYPE_INVALID);

			reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
		
			dbus_message_unref(query);
	
			if (NULL == reply)
				{
				dbus_error_free_for_dcli(&err);

					return NULL;
				}
	
			dbus_message_iter_init(reply,&iter);
			dbus_message_iter_get_basic(&iter,ret);
			dbus_message_unref(reply);
		}
	printf("ret = %d\n",*ret);
	return *ret;
	
}
struct WtpList * set_radio_bss_max_num_cmd_set_bss_max_sta_num(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int TYPE,unsigned int id,unsigned int type,unsigned int max_sta_num,unsigned int wlanid,
	int *count,int *ret,int *ret2,int *failnum)     //fengwenchao modify 20110512
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	int i = 0;
	int j = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;
	struct RadioList *fail_radio_list = NULL;
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_RADIO_CHECK_RADIO_MEMBER);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&TYPE,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	//printf("11111111111111111111111\n");
	if((TYPE==0)&&(*ret==0))
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			//*bssid=radioid*L_BSS_NUM+bss_index-1;   //fengwenchao comment 20110512

			*ret = set_radio_bss_max_num_asd_wid(localid,index,dbus_connection,radioid,type,max_sta_num,wlanid,ret);     //fengwenchao modify 20110512
		}
	
	if((TYPE==1)&&(*ret==0)){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0)
			{
				fail_radio_list = (struct RadioList*)malloc((*count) * sizeof(struct RadioList));
			}
		
		for(i=0; i < (*count); i++)
			{	
				dbus_message_iter_next(&iter);	
				dbus_message_iter_get_basic(&iter,&radioid);
				printf("2222222222222222222222222222222222222\n");
				//*bssid=radioid*L_BSS_NUM+bss_index-1;   //fengwenchao comment 20110512
					
				*ret2 = set_radio_bss_max_num_asd_wid(localid,index,dbus_connection,radioid,type,max_sta_num,wlanid,ret2);  //fengwenchao modify 20110512

				if(*ret2 != 0)
					{
						fail_radio_list[j].RadioId = radioid;
						fail_radio_list[j].FailReason = *ret2;
						j++;
						printf("failnum = %d\n",j);
						printf("ret2 = %d\n",*ret2);
					}

			}
		*failnum = j;
	}
	
	dbus_message_unref(reply);
	return fail_radio_list ;		
}

struct WtpList * set_radio_bss_l3_policy_cmd_bss_no_wlan_bss_interface(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char wlanid,unsigned int bsspolicy,int *count,unsigned int *ret)    //fengwenchao modify 20110512
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_SET_BSS_L3_POLICY);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&wlanid,       //fengwenchao modify 20110511
									DBUS_TYPE_UINT32,&bsspolicy,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;	
}

struct WtpList * set_radio_apply_wlan_cmd_radio_apply_wlan(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int g_radio_id,unsigned int g_id, unsigned int l_radio_id, unsigned char wlanid,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;
	DBusMessageIter iter_struct;
	DBusMessageIter	 iter_array;
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_APPLY_WLAN);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&g_radio_id,
									DBUS_TYPE_UINT32,&g_id,
									DBUS_TYPE_UINT32,&l_radio_id,
									DBUS_TYPE_BYTE,&wlanid,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	*count = 0;
	if(type == 1 && ret == WID_DBUS_SUCCESS){

		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}

		dbus_message_iter_next(&iter);
		dbus_message_iter_recurse(&iter,&iter_array);
	
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;

			dbus_message_iter_recurse(&iter_array,&iter_struct);
			dbus_message_iter_get_basic(&iter_struct,&radioid);

			dbus_message_iter_next(&iter_struct);	
			dbus_message_iter_get_basic(&iter_struct,&fail_reason);

			dbus_message_iter_next(&iter_array);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}

struct WtpList * set_radio_apply_qos_cmd_radio_apply_qos(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned int qosid,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_APPLY_QOS);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&qosid,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}

struct WtpList * set_radio_delete_qos_cmd_radio_delete_qos(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned int qosid,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_DELETE_QOS);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&qosid,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}

struct WtpList * set_radio_bss_max_throughput_cmd_set_bss_max_throughput(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char wlanid,unsigned int throughput,int *count,unsigned int *ret)   //fengwenchao modify 20110512
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_SET_BSS_MAX_THROUGHPUT);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									 DBUS_TYPE_BYTE,&wlanid,         //fengwenchao modify 20110512
									DBUS_TYPE_UINT32,&throughput,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}

struct WtpList * set_radio_delete_wlan_cmd_radio_delete_wlan(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int g_radio_id, unsigned int g_id, unsigned int l_radio_id, unsigned char wlanid,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_DELETE_WLAN);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&g_radio_id,
									DBUS_TYPE_UINT32, &g_id,
									DBUS_TYPE_UINT32, &l_radio_id,
									DBUS_TYPE_BYTE,&wlanid,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	*count = 0;
	if(type == 1 && ret == WID_DBUS_SUCCESS){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}

struct WtpList * set_radio_enable_wlan_cmd_radio_enable_wlan(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char wlanid,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_ENABLE_WLAN);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&wlanid,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;	
}

struct WtpList * set_radio_disable_wlan_cmd_radio_disable_wlan_ID(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char wlanid,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_DISABLE_WLAN);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&wlanid,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;	
}

struct WtpList * set_radio_default_config_cmd_recover_default_config(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char reserved,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_REVOVER_DEFAULT_CONFIG);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&reserved,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}

struct WtpList * radio_apply_wlan_base_vlan_cmd_radio_apply_wlan_base_vlan(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned int vlan_id,unsigned char wlan_id,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_RADIO_APPLY_WLANID_BASE_VLANID);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&wlan_id,
									DBUS_TYPE_UINT32,&vlan_id,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}

struct WtpList * set_radio_max_throughout_cmd_set_radio_max_throughout(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char bandwidth,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_SET_MAX_THROUGHOUT);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&bandwidth,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}

struct WtpList * set_radio_l2_isolation_cmd_set_wlan_ID_l2_isolation(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char wlanid,unsigned int policy,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_SET_RADIO_L2_ISOLATION_ABLE);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&wlanid,
									DBUS_TYPE_UINT32,&policy,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}

struct WtpList * set_radio_11n_cwmmode_cmd_set_11n_wlan_ID_cwmmode(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char wlanid,unsigned char policy,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_11N_SET_RADIO_CWMMODE);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&wlanid,
									DBUS_TYPE_BYTE,&policy,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}

struct WtpList * radio_wlan_wds_bssid_cmd_wlan_wds_bssid_MAC(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char * mac,unsigned char wlan_id,unsigned char list_type,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_WDS_WLAN_SET);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&wlan_id,
									DBUS_TYPE_BYTE,&list_type,
									DBUS_TYPE_BYTE,&mac[0],
									DBUS_TYPE_BYTE,&mac[1],
									DBUS_TYPE_BYTE,&mac[2],
									DBUS_TYPE_BYTE,&mac[3],
									DBUS_TYPE_BYTE,&mac[4],
									DBUS_TYPE_BYTE,&mac[5],
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;	
}

struct WtpList * set_ap_radio_auto_channel_cmd_set_radio_auto_channel(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned int policy,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_SET_WTP_RADIO_AUTO_CHANNEL);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&policy,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;	
}

struct WtpList * set_ap_radio_auto_channel_cont_cmd_set_radio_auto(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned int policy,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_SET_WTP_RADIO_AUTO_CHANNEL_CONT);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&policy,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;	
}

struct WtpList * set_ap_radio_diversity_cmd_set_radio_diversity(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned int policy,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_SET_WTP_RADIO_DIVERSITY);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&policy,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}

struct WtpList * set_ap_radio_txantenna_cmd_set_radio_txantenna(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned int policy,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_SET_WTP_RADIO_TXANTENNA);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&policy,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}

struct WtpList * radio_bss_taffic_limit_cmd_wlan_ID_traffic_limit(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char wlanid,unsigned char policy,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_ABLE);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&wlanid,
									DBUS_TYPE_BYTE,&policy,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}

struct WtpList * radio_bss_taffic_limit_value_cmd_wlan_ID_traffic_limit_value(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char wlanid,unsigned int value,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_VALUE);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&wlanid,
									DBUS_TYPE_UINT32,&value,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;	
}

struct WtpList * radio_bss_taffic_limit_average_value_cmd_wlan_ID_traffic_limit_station_average_value(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char wlanid,unsigned int value,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_AVERAGE_VALUE);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&wlanid,
									DBUS_TYPE_UINT32,&value,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;

}

struct WtpList * radio_bss_taffic_limit_send_value_cmd_wlan_ID_traffic_limit_send_value(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char wlanid,unsigned int value,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_SEND_VALUE);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&wlanid,
									DBUS_TYPE_UINT32,&value,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;			
}

struct WtpList * radio_bss_taffic_limit_average_send_value_cmd_wlan_configure(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char wlanid,unsigned int value,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_AVERAGE_SEND_VALUE);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&wlanid,
									DBUS_TYPE_UINT32,&value,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}
int radio_asd_sta_traffic_cancel(int localid,  int index,DBusConnection *dbus_connection,unsigned int id,unsigned char wlan_id,unsigned char *mac1,unsigned int *ret_asd)
{
		DBusMessage *query, *reply; 
		DBusMessageIter  iter;
		DBusError err;
		//unsigned int ret;
		unsigned char cancel_flag = 0;
 
		char BUSNAME[PATH_LEN];
		char OBJPATH[PATH_LEN];
		char INTERFACE[PATH_LEN];

		ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_STA_SEND_TRAFFIC_LIMIT_CANCEL);
		
		dbus_error_init(&err);
	
		dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&id,
								DBUS_TYPE_BYTE,&wlan_id,
								DBUS_TYPE_BYTE,&mac1[0],
								DBUS_TYPE_BYTE,&mac1[1],
								DBUS_TYPE_BYTE,&mac1[2],
								DBUS_TYPE_BYTE,&mac1[3],
								DBUS_TYPE_BYTE,&mac1[4],
								DBUS_TYPE_BYTE,&mac1[5],
								DBUS_TYPE_INVALID);
	
		
		reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		
		if (NULL == reply) {
			dbus_error_free_for_dcli(&err);

			return NULL;
		}
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,ret_asd);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&cancel_flag);
		
		dbus_message_unref(reply);
		return cancel_flag;
}
struct WtpList *radio_bss_taffic_limit_sta_send_value_cmd_wlan_traffic_limit_station_send_value(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int TYPE,unsigned int id,unsigned char mac1[MAC_LEN],unsigned char wlan_id,unsigned char type,unsigned int value,
	int *count,unsigned int *ret,unsigned int *ret1,unsigned int *ret2,unsigned int *ret3,int *failnum)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;
	printf("!!!!!!!!!!!!!!!!!!!!!\n");
	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int j = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;
	struct RadioList *fail_radio_list = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	printf("TYPE= %d,id =%d\n",TYPE,id);
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_RADIO_METHOD_RADIO_CHECK_RADIO_MEMBER);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&TYPE,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	printf("TYPE= %d,id =%d\n",TYPE,id);
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	printf("ret = %d\n",*ret);
	
	if((TYPE==0)&&(*ret == WID_DBUS_SUCCESS))
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			asd_set_sta_info(dbus_connection,index,mac1,wlan_id,id,type,value,localid,ret);
			//printf("ret = %d\n",*ret);
			if(*ret == 0)
				{
					wid_set_sta_info(dbus_connection,index,mac1,wlan_id,id,type,value,localid,ret2);
					//printf("ret2 = %d\n",*ret2);
					if(*ret2 == 0)
						{
							set_sta_static_info(dbus_connection,index,mac1,wlan_id,id,type,value,localid,ret3);
						}
				}		
		}
	
	if((TYPE==1)&&(*ret == WID_DBUS_SUCCESS)){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){

		fail_radio_list = (struct RadioList*)malloc((*count) * sizeof(struct RadioList));
		}

		for(i=0; i < (*count); i++){

			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			asd_set_sta_info(dbus_connection,index,mac1,wlan_id,radioid,type,value,localid,ret1);
			if(*ret1 == 0)
				{
					wid_set_sta_info(dbus_connection,index,mac1,wlan_id,radioid,type,value,localid,ret2);
					//printf("ret2 = %d\n",*ret2);
					if(*ret2 == 0)
						{
							set_sta_static_info(dbus_connection,index,mac1,wlan_id,radioid,type,value,localid,ret3);
							//printf("ret3 = %d\n",*ret3);
							if(*ret3 != 0)
								{
									fail_radio_list[j].RadioId = radioid;
									fail_radio_list[j].FailReason = *ret3;
									j ++;
								}
						}
					else
						{
							fail_radio_list[j].RadioId = radioid;
							fail_radio_list[j].FailReason = *ret2;
							j ++;
						}
				}
			else
				{
					fail_radio_list[j].RadioId = radioid;
					fail_radio_list[j].FailReason = *ret1;
					j ++;
				}
		}
		*failnum = j;
	}
	
	dbus_message_unref(reply);
	return fail_radio_list ;		
}

struct WtpList * radio_bss_taffic_limit_cancel_sta_send_value_cmd_wlan_configure(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char wlan_id,unsigned char *mac1, int *count,
	unsigned int *ret,unsigned int *ret_asd,unsigned int *ret_wid,int *failnum)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int j = 0;
	int radioid = 0;
	unsigned char cancel_flag = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *Radionotaccess = NULL;
	struct RadioList *radio_list_head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_RADIO_CHECK_RADIO_MEMBER);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if((type==0)&&(*ret==0))
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			cancel_flag = radio_asd_sta_traffic_cancel(localid,index,dbus_connection,radioid,wlan_id,mac1,ret_asd);
			printf("cancel_flag = %d\n",cancel_flag);
			printf("ret_asd = %d\n",*ret_asd);
			if(*ret_asd == ASD_DBUS_SUCCESS)
				{
					
					memset(BUSNAME,0,PATH_LEN);
					memset(OBJPATH,0,PATH_LEN);
					memset(INTERFACE,0,PATH_LEN);
					
					ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
					ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
					ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
					query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_CANCEL_STA_SEND_VALUE);
						
					dbus_error_init(&err);
					
					dbus_message_append_args(query,
											DBUS_TYPE_UINT32,&radioid,
											DBUS_TYPE_BYTE,&wlan_id,
											DBUS_TYPE_BYTE,&cancel_flag,
											DBUS_TYPE_BYTE,&mac1[0],
											DBUS_TYPE_BYTE,&mac1[1],
											DBUS_TYPE_BYTE,&mac1[2],
											DBUS_TYPE_BYTE,&mac1[3],
											DBUS_TYPE_BYTE,&mac1[4],
											DBUS_TYPE_BYTE,&mac1[5],
											DBUS_TYPE_INVALID);
					
						
					reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
						
					//dbus_message_unref(query);
						
					if (NULL == reply) {
				
					dbus_error_free_for_dcli(&err);

						return NULL;
					}
						dbus_message_iter_init(reply,&iter);
						dbus_message_iter_get_basic(&iter,ret_wid);
						//dbus_message_unref(reply);
						printf("ret_wid = %d\n",*ret_wid);
				}
		}

	if((type==1)&&(*ret==0))
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,count);
			if(*count != 0)
				{
					if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL)
						{
							*ret = MALLOC_ERROR;
							dbus_message_unref(reply);
							return NULL;
						}	
					memset(radio_list_head,0,sizeof(struct RadioList));
					radio_list_head->RadioList_list = NULL;
					radio_list_head->RadioList_last = NULL;
				}
			for(i=0; i < (*count); i++)
				{
					if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL)
						{
							dcli_free_RadioList(radio_list_head);
							*ret = MALLOC_ERROR;
							dbus_message_unref(reply);
							return NULL;
						}			
					memset(RadioNode,0,sizeof(struct RadioList));
					RadioNode->next = NULL;

					if(radio_list_head->RadioList_list == NULL)
						{
							radio_list_head->RadioList_list = RadioNode;
							radio_list_head->next = RadioNode;
						}
					else
						{
							radio_list_head->RadioList_last->next = RadioNode;
						}
					radio_list_head->RadioList_last = RadioNode;
			
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&radioid);

					RadioNode->RadioId = radioid;
				}
			Radionotaccess = (struct RadioList*)malloc((*count)*(sizeof(struct RadioList)));
			for(i=0; i<(*count); i++)
				{
					if(Radio_Show_Node == NULL)
						Radio_Show_Node = radio_list_head->RadioList_list;
					else 
						Radio_Show_Node = Radio_Show_Node->next;
					if(Radio_Show_Node == NULL)
						break;
					printf("Radio_Show_Node->RadioId = %d \n",Radio_Show_Node->RadioId);	

					cancel_flag = radio_asd_sta_traffic_cancel(localid,index,dbus_connection,Radio_Show_Node->RadioId,wlan_id,mac1,ret_asd);

					if(*ret_asd == ASD_DBUS_SUCCESS)
						{	
							memset(BUSNAME,0,PATH_LEN);
							memset(OBJPATH,0,PATH_LEN);
							memset(INTERFACE,0,PATH_LEN);
					
							ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
							ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
							ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
							query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_CANCEL_STA_SEND_VALUE);
							
							dbus_error_init(&err);
						
							dbus_message_append_args(query,
													 DBUS_TYPE_UINT32,&Radio_Show_Node->RadioId,
													 DBUS_TYPE_BYTE,&wlan_id,
													 DBUS_TYPE_BYTE,&cancel_flag,
													 DBUS_TYPE_BYTE,&mac1[0],
													 DBUS_TYPE_BYTE,&mac1[1],
													 DBUS_TYPE_BYTE,&mac1[2],
													 DBUS_TYPE_BYTE,&mac1[3],
													 DBUS_TYPE_BYTE,&mac1[4],
													 DBUS_TYPE_BYTE,&mac1[5],
													 DBUS_TYPE_INVALID);
						
							reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
						
							//dbus_message_unref(query);
						
							if (NULL == reply)
								{
								dbus_error_free_for_dcli(&err);

									if(Radionotaccess){
										free(Radionotaccess);
										Radionotaccess = NULL;
									}
									dcli_free_RadioList(radio_list_head);
									return NULL;
								}
							dbus_message_iter_init(reply,&iter);
							dbus_message_iter_get_basic(&iter,ret_wid);
							//dbus_message_unref(reply);
							if(*ret_wid != 0)
								{
									Radionotaccess[j].RadioId = Radio_Show_Node->RadioId;
									Radionotaccess[j].FailReason = *ret_wid;
									j++;
								}
						}
					else
						{
							Radionotaccess[j].RadioId = Radio_Show_Node->RadioId;
							Radionotaccess[j].FailReason = *ret_asd;
							j++;
						}
				}
		*failnum = j;
		printf("failnum = %d\n",*failnum);
		printf("j = %d\n",j);
		dcli_free_RadioList(radio_list_head);
	}
	
	dbus_message_unref(reply);
	return Radionotaccess ;		
}
int radio_asd_sta_taffic_value(int localid,  int index,DBusConnection *dbus_connection,unsigned int id,unsigned char wlan_id,unsigned char *mac1,unsigned int *ret_asd)
{
		DBusMessage *query, *reply; 
		DBusMessageIter  iter;
		DBusError err;

		unsigned char cancel_flag = 0;   

		char BUSNAME[PATH_LEN];
		char OBJPATH[PATH_LEN];
		char INTERFACE[PATH_LEN];
	
		ReInitDbusPath_V2(localid,index,ASD_DBUS_BUSNAME,BUSNAME);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_OBJPATH,OBJPATH);
		ReInitDbusPath_V2(localid,index,ASD_DBUS_STA_INTERFACE,INTERFACE);
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,ASD_DBUS_STA_METHOD_STA_TRAFFIC_LIMIT_CANCEL);
	
		dbus_error_init(&err);
	
		dbus_message_append_args(query,
								DBUS_TYPE_UINT32,&id,
								DBUS_TYPE_BYTE,&wlan_id,
								DBUS_TYPE_BYTE,&mac1[0],
								DBUS_TYPE_BYTE,&mac1[1],
								DBUS_TYPE_BYTE,&mac1[2],
								DBUS_TYPE_BYTE,&mac1[3],
								DBUS_TYPE_BYTE,&mac1[4],
								DBUS_TYPE_BYTE,&mac1[5],
								DBUS_TYPE_INVALID);
	
		
		reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
		
		dbus_message_unref(query);
		
		if (NULL == reply) {
	
		dbus_error_free_for_dcli(&err);

			return NULL;
		}
		dbus_message_iter_init(reply,&iter);
		dbus_message_iter_get_basic(&iter,ret_asd);
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,&cancel_flag);
	return cancel_flag;
}
struct WtpList * radio_bss_taffic_limit_cancel_sta_value_cmd_wlan_configure(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char wlan_id,unsigned char *mac1, int *count,
	unsigned int *ret,unsigned int *ret_asd,unsigned int *ret_wid,int *failnum)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int j = 0;
	int radioid = 0;
	unsigned char cancel_flag = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *Radionotaccess = NULL;
	struct RadioList *radio_list_head = NULL;
	struct RadioList *Radio_Show_Node = NULL;
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_RADIO_CHECK_RADIO_MEMBER);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);

	if((type==0)&&(*ret==0))
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			cancel_flag = radio_asd_sta_taffic_value(localid,index,dbus_connection,radioid,wlan_id,mac1,ret_asd);

			if(*ret_asd == ASD_DBUS_SUCCESS)
				{
					
					memset(BUSNAME,0,PATH_LEN);
					memset(OBJPATH,0,PATH_LEN);
					memset(INTERFACE,0,PATH_LEN);
					
					ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
					ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
					ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
					query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_CANCEL_STA_VALUE);
						
					dbus_error_init(&err);											
					
					dbus_message_append_args(query,
											DBUS_TYPE_UINT32,&radioid,
											DBUS_TYPE_BYTE,&wlan_id,
											DBUS_TYPE_BYTE,&cancel_flag,
											DBUS_TYPE_BYTE,&mac1[0],
											DBUS_TYPE_BYTE,&mac1[1],
											DBUS_TYPE_BYTE,&mac1[2],
											DBUS_TYPE_BYTE,&mac1[3],
											DBUS_TYPE_BYTE,&mac1[4],
											DBUS_TYPE_BYTE,&mac1[5],
											DBUS_TYPE_INVALID);
					
						
					reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
						
					dbus_message_unref(query);
						
					if (NULL == reply) {
				
					dbus_error_free_for_dcli(&err);

						return NULL;
					}
						dbus_message_iter_init(reply,&iter);
						dbus_message_iter_get_basic(&iter,ret_wid);
						//dbus_message_unref(reply);
				}
		}

	if((type==1)&&(*ret==0))
		{
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,count);
			if(*count != 0)
				{
					if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL)
						{
							*ret = MALLOC_ERROR;
							dbus_message_unref(reply);
							return NULL;
						}	
					memset(radio_list_head,0,sizeof(struct RadioList));
					radio_list_head->RadioList_list = NULL;
					radio_list_head->RadioList_last = NULL;
				}
			for(i=0; i < (*count); i++)
				{
					if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL)
						{
							dcli_free_RadioList(radio_list_head);
							*ret = MALLOC_ERROR;
							dbus_message_unref(reply);
							return NULL;
						}			
					memset(RadioNode,0,sizeof(struct RadioList));
					RadioNode->next = NULL;

					if(radio_list_head->RadioList_list == NULL)
						{
							radio_list_head->RadioList_list = RadioNode;
							radio_list_head->next = RadioNode;
						}
					else
						{
							radio_list_head->RadioList_last->next = RadioNode;
						}
					radio_list_head->RadioList_last = RadioNode;
			
					dbus_message_iter_next(&iter);	
					dbus_message_iter_get_basic(&iter,&radioid);

					RadioNode->RadioId = radioid;
				}
			Radionotaccess = (struct RadioList*)malloc((*count)*(sizeof(struct RadioList)));
			for(i=0; i<(*count); i++)
				{
					if(Radio_Show_Node == NULL)
						Radio_Show_Node = radio_list_head->RadioList_list;
					else 
						Radio_Show_Node = Radio_Show_Node->next;
					if(Radio_Show_Node == NULL)
						break;
					printf("Radio_Show_Node->RadioId = %d \n",Radio_Show_Node->RadioId);	

					cancel_flag = radio_asd_sta_taffic_value(localid,index,dbus_connection,Radio_Show_Node->RadioId,wlan_id,mac1,ret_asd);

					if(*ret_asd == ASD_DBUS_SUCCESS)
						{	
							memset(BUSNAME,0,PATH_LEN);
							memset(OBJPATH,0,PATH_LEN);
							memset(INTERFACE,0,PATH_LEN);
					
							ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
							ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
							ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
							query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,WID_DBUS_RADIO_METHOD_WLAN_TRAFFIC_LIMIT_CANCEL_STA_VALUE);
							
							dbus_error_init(&err);
						
							dbus_message_append_args(query,
													 DBUS_TYPE_UINT32,&Radio_Show_Node->RadioId,
													 DBUS_TYPE_BYTE,&wlan_id,
													 DBUS_TYPE_BYTE,&cancel_flag,
													 DBUS_TYPE_BYTE,&mac1[0],
													 DBUS_TYPE_BYTE,&mac1[1],
													 DBUS_TYPE_BYTE,&mac1[2],
													 DBUS_TYPE_BYTE,&mac1[3],
													 DBUS_TYPE_BYTE,&mac1[4],
													 DBUS_TYPE_BYTE,&mac1[5],
													 DBUS_TYPE_INVALID);
						
							reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
						
							dbus_message_unref(query);
						
							if (NULL == reply)
								{
									dbus_error_free_for_dcli(&err);

									dcli_free_RadioList(radio_list_head);	
									if(Radionotaccess){
										free(Radionotaccess);
										Radionotaccess = NULL;
									}
									return NULL;
								}
							dbus_message_iter_init(reply,&iter);
							dbus_message_iter_get_basic(&iter,ret_wid);
							dbus_message_unref(reply);
							if(*ret_wid != 0)
								{
									Radionotaccess[j].RadioId = Radio_Show_Node->RadioId;
									Radionotaccess[j].FailReason = *ret_wid;
									j++;
								}
						}
					else
						{
							Radionotaccess[j].RadioId = Radio_Show_Node->RadioId;
							Radionotaccess[j].FailReason = *ret_asd;
							j++;
						}
				}
		*failnum = j;
		printf("failnum = %d\n",*failnum);
		printf("j = %d\n",j);
		dcli_free_RadioList(radio_list_head);
	}

	dbus_message_unref(reply);
	return Radionotaccess ;		
}

struct WtpList * radio_apply_wlan_clean_vlan_cmd_radio_apply_wlan_ID(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char wlanid,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_RADIO_APPLY_WLANID_CLEAN_VLANID);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&wlanid,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;	
}

struct WtpList * set_sta_dhcp_before_authorized_cmd_wlan_ID_sta_dhcp(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char wlanid,int policy,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;
	printf("!!!!!!!!!!!!!!!!!!!!!!!!\n");
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_WLAN_SET_STA_DHCP_BEFORE_AUTHERIZED);
	printf("type= %d,id = %d ,wlanid = %d,policy = %d\n",type,id,wlanid,policy);	
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&wlanid,
									DBUS_TYPE_UINT32,&policy,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;	
}

struct WtpList * set_sta_ip_mac_binding_cmd_wlan_ID_sta_ip_mac_binding(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char wlanid,int policy,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_WLAN_SET_STA_IP_MAC_BINDING);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&wlanid,
									DBUS_TYPE_UINT32,&policy,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}

struct WtpList * set_radio_guard_interval_cmd_11n_guard_interval(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned short guard_interval,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_SET_GUARD_INTERVAL);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT16,&guard_interval,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}

struct WtpList * set_radio_mcs_cmd_11n_mcs(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,update_mcs_list *mcs,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int j = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;
	struct tag_mcsid *temp = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUS_RADIO_METHOD_SET_MCS);
	//qiuchen change it	
	dbus_error_init(&err);
	dbus_message_iter_init_append (query, &iter);
	
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&type);							 
	dbus_message_iter_append_basic (&iter, DBUS_TYPE_UINT32,&id);
	dbus_message_iter_append_basic (&iter,DBUS_TYPE_UINT32,&(mcs->count));
	
	temp = mcs->mcsidlist;
	for(j=0;((i<mcs->count) && temp);j++){
		dbus_message_iter_append_basic(&iter,DBUS_TYPE_UINT32,&(temp->mcsid));
		temp = temp->next;
	}
	/*us_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&mcs->count,
									DBUS_TYPE_INVALID);*/

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}

struct WtpList * set_radio_cmmode_cmd_11n_cwmode(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned short cwmode,int *count,unsigned int *ret,unsigned int *max_channel,unsigned int *min_channel,unsigned char *current_channel)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_RADIO_METHOD_SET_CMMODE);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT16,&cwmode,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	//fengwenchao add 20110324
	if(type == 0)
		{
			if(*ret == CHANNEL_CROSS_THE_BORDER)
			{		
				 dbus_message_iter_next(&iter); 
				 dbus_message_iter_get_basic(&iter,max_channel);
				 dbus_message_iter_next(&iter); 	
				 dbus_message_iter_get_basic(&iter,min_channel);
				 dbus_message_iter_next(&iter); 	
				 dbus_message_iter_get_basic(&iter,current_channel);			 
			 }
		}
	//fengwenchao add end
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}



struct WtpList * set_radio_netgear_set_radio_sector_power_set_radio_sector_list(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned short resv_short,unsigned int resv_int2,char *METHOD,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];
	
	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	if (!strcmp(METHOD,WID_DBUS_RADIO_SECTOR_SET_CMD))
	{
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,METHOD);
		dbus_error_init(&err);
		
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&type,
								 DBUS_TYPE_UINT32,&id,
								 DBUS_TYPE_UINT16,&resv_short,/*sector value*/
								 DBUS_TYPE_UINT32,&resv_int2,
								 DBUS_TYPE_INVALID);
	}		
	else if (!strcmp(METHOD,WID_DBUS_RADIO_SECTOR_POWER_SET_CMD))
	{
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,METHOD);
		dbus_error_init(&err);
		
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&type,
								 DBUS_TYPE_UINT32,&id,
								 DBUS_TYPE_UINT16,&resv_short,/*sector num*/
								 DBUS_TYPE_UINT32,&resv_int2,/*sector value*/
								 DBUS_TYPE_INVALID);
	}
	else if (!strcmp(METHOD,WID_DBUS_RADIO_NETGEAR_G_SET_CMD))
	{
		query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,METHOD);
		dbus_error_init(&err);
		
		dbus_message_append_args(query,
								 DBUS_TYPE_UINT32,&type,
								 DBUS_TYPE_UINT32,&id,
								 DBUS_TYPE_UINT16,&resv_short,/*netgear G type*/
								 DBUS_TYPE_UINT32,&resv_int2,/*netgear G state*/
								 DBUS_TYPE_INVALID);
	}
	else
	{
		ret = -1;
	}

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		//vty_out(vty,"<error> failed get reply.\n");
		dbus_error_free_for_dcli(&err);

		ret = -1;
		return NULL;
	}
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;
}

struct WtpList * set_wds_bridge_distance_cmd_set_distance(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,int distance,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_RADIO_METHOD_SET_WDS_DISTANCE);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&distance,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}

struct WtpList * wds_remote_brmac_cmd_add_del_MAC(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,int is_add,unsigned char *macAddr, int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_RADIO_METHOD_SET_WDS_REMOTE_BRMAC);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&is_add,
									DBUS_TYPE_BYTE,&macAddr[0],
									DBUS_TYPE_BYTE,&macAddr[1], 
									DBUS_TYPE_BYTE,&macAddr[2], 
									DBUS_TYPE_BYTE,&macAddr[3], 
									DBUS_TYPE_BYTE,&macAddr[4], 
									DBUS_TYPE_BYTE,&macAddr[5], 
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}


struct WtpList * wds_encryption_type_cmd_set_wds_encrption_type(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int TYPE,unsigned int id,unsigned int type,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_RADIO_METHOD_SET_WDS_ENCRYPTION_TYPE);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&TYPE,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&type, 
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(TYPE == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;		
}


struct WtpList * set_radio_inter_vap_forwarding_cmd_set_inter_VAP_forwarding(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char policy,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_RADIO_METHOD_INTER_VAP_FORVARDING_ABLE);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&policy, 
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;	
}

struct WtpList * set_radio_intra_vap_forwarding_cmd_set_intra_VAP_forwarding(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char policy,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_RADIO_METHOD_INTRA_VAP_FORVARDING_ABLE);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&policy, 
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;
}


struct WtpList * set_radio_keep_alive_period_cmd_set_radio_keep_alive_period(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned int period,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_RADIO_METHOD_SET_KEEP_ALIVE_PERIOD);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&period, 
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;
}


struct WtpList * set_radio_keep_alive_idle_time_cmd_set_radio_keep_alive_idle_time(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned int period,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_RADIO_METHOD_SET_KEEP_ALIVE_IDLE_TIME);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&period, 
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;
}


struct WtpList * set_radio_congestion_avoidance_cmd_set_radio_congestion_avoidance(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned int congestion_av_state,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_RADIO_METHOD_SET_CONGESTION_AVOID_STATE);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&congestion_av_state, 
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;
}


struct WtpList * wds_wep_key_cmd_set_wds_wep_key(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char *key,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_RADIO_METHOD_SET_WDS_WEP_KEY);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_STRING,&key, 
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;
}

struct WtpList * wds_aes_key_cmd_setwds_brmac(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char *key,unsigned char * macAddr,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_RADIO_METHOD_SET_WDS_AES_KEY);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_STRING,&key, 
									DBUS_TYPE_BYTE,&macAddr[0],
									DBUS_TYPE_BYTE,&macAddr[1],
									DBUS_TYPE_BYTE,&macAddr[2],
									DBUS_TYPE_BYTE,&macAddr[3],
									DBUS_TYPE_BYTE,&macAddr[4],
									DBUS_TYPE_BYTE,&macAddr[5],
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;
}

struct WtpList * set_wtp_sta_static_arp_enable_cmd_set_wlan_sta_static_arp(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned int policy,unsigned char wlanid,char *ifname, int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_WLAN_WTP_METHOD_STA_STATIC_ARP);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&policy, 
									DBUS_TYPE_BYTE,&wlanid,
									DBUS_TYPE_STRING,&ifname,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;
}


struct WtpList * set_radio_11n_ampdu_able_cmd_11n(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int TYPE,unsigned int id,unsigned char policy,unsigned char type,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_RADIO_METHOD_SET_AMPDU_ABLE);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&TYPE,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&policy, 
									DBUS_TYPE_BYTE,&type,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(TYPE == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;
}


struct WtpList * set_radio_11n_ampdu_limit_cmd_11n(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int TYPE,unsigned int id,unsigned int limit,unsigned char type,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_RADIO_METHOD_SET_AMPDU_LIMIT);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&TYPE,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&limit, 
									DBUS_TYPE_BYTE,&type,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(TYPE == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;
}


struct WtpList * set_radio_11n_ampdu_subframe_cmd_11n(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int TYPE,unsigned int id,unsigned char subframe,unsigned char type,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_RADIO_METHOD_SET_AMPDU_SUBFRAME);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&TYPE,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&subframe, 
									DBUS_TYPE_BYTE,&type,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(TYPE == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;
}


struct WtpList * set_radio_11n_puren_mixed_cmd_wlan_VALUE_workmode(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char policy,unsigned char WLANID,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_RADIO_METHOD_SET_MIXED_PURE_N);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&policy, 
									DBUS_TYPE_BYTE,&WLANID,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;
}

struct WtpList * set_tx_chainmask_cmd_set_tx_chainmask_LIST_enable_disable(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char hex_id,unsigned int policy,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_RADIO_TX_CHAINMASK_SET_CMD);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&hex_id, 
									DBUS_TYPE_UINT32,&policy,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;
}


struct WtpList * set_radio_11n_channel_offset_cmd_channel_offset(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned char policy,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_RADIO_METHOD_SET_CHANNEL_OFFSET);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&policy, 
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;
}


struct WtpList * set_tx_chainmask_v2_cmd_tx_chainmask_rx_chainmask(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int TYPE,unsigned int id,unsigned char policy,unsigned char type,int *count,int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_RADIO_TX_CHAINMASK_SET_CMD_V2);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&TYPE,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_BYTE,&policy, 
									DBUS_TYPE_BYTE,&type,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		ret = -1;
		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(TYPE == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;
}

struct WtpList * set_radio_txpowerstep_cmd_txpowerstep(int localid,  int index,DBusConnection *dbus_connection,
	unsigned int type,unsigned int id,unsigned short txpowerstep,int *count,unsigned int *ret)
{
	DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	int i = 0;
	int radioid = 0;
	char fail_reason = 0;
	struct RadioList  *RadioNode = NULL;
	struct RadioList *radio_list_head = NULL;

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										WID_DBUS_RADIO_METHOD_SET_TXPOWER_STEP);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&type,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT16,&txpowerstep, 					 
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return NULL;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,ret);
	
	if(type == 1){
		dbus_message_iter_next(&iter);	
		dbus_message_iter_get_basic(&iter,count);
		if(*count != 0){
			if((radio_list_head = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
				*ret = MALLOC_ERROR;
				dbus_message_unref(reply);
				return NULL;
			}
			
			memset(radio_list_head,0,sizeof(struct RadioList));
			radio_list_head->RadioList_list = NULL;
			radio_list_head->RadioList_last = NULL;
		}
		
		for(i=0; i < (*count); i++){
			if((RadioNode = (struct RadioList*)malloc(sizeof(struct RadioList))) == NULL){
					dcli_free_RadioList(radio_list_head);
					*ret = MALLOC_ERROR;
					dbus_message_unref(reply);
					return NULL;
			}
			
			memset(RadioNode,0,sizeof(struct RadioList));
			RadioNode->next = NULL;

			if(radio_list_head->RadioList_list == NULL){
				radio_list_head->RadioList_list = RadioNode;
				radio_list_head->next = RadioNode;
			}
			else{
				radio_list_head->RadioList_last->next = RadioNode;
			}
			
			radio_list_head->RadioList_last = RadioNode;
			
			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&radioid);

			dbus_message_iter_next(&iter);	
			dbus_message_iter_get_basic(&iter,&fail_reason);
			
			RadioNode->FailReason = fail_reason;
			RadioNode->RadioId = radioid;
		}
	}
	
	dbus_message_unref(reply);
	return radio_list_head ;
}

void dcli_free_RadioList(struct RadioList *RadioNode)
{
	struct RadioList *tmp = NULL;
	
	if(RadioNode == NULL)
		return ;
	
	if(RadioNode->RadioList_last != NULL) {
		RadioNode->RadioList_last = NULL;
	}

	while(RadioNode->RadioList_list != NULL){
		tmp = RadioNode->RadioList_list;
		RadioNode->RadioList_list = tmp->next;
		tmp->next = NULL;
		free(tmp);
		tmp = NULL;
	}
	
	free(RadioNode);
	RadioNode = NULL;
	return ;
}


/*
** set 11n rate paras
** book add, 2010-10-20
*/
int set_11n_rate_paras(int localid,int index,DBusConnection *dbus_connection,int mcs,int cwmode,int gi,unsigned int id,unsigned char chan)
{
    int ret = 0;
    DBusError err;
	DBusMessageIter  iter;
	DBusMessage *reply =NULL; 
	DBusMessage *query = NULL;

	char BUSNAME[PATH_LEN];
	char OBJPATH[PATH_LEN];
	char INTERFACE[PATH_LEN];

	ReInitDbusPath_V2(localid,index,WID_DBUS_BUSNAME,BUSNAME);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_OBJPATH,OBJPATH);
	ReInitDbusPath_V2(localid,index,WID_DBUS_RADIO_INTERFACE,INTERFACE);
	
	query = dbus_message_new_method_call(BUSNAME,OBJPATH,INTERFACE,
										 WID_DBUG_RADIO_METHOD_SET_11N_RATE_PARAS);
		
	dbus_error_init(&err);

	dbus_message_append_args(query,
									DBUS_TYPE_UINT32,&id,
									DBUS_TYPE_UINT32,&mcs,
									DBUS_TYPE_UINT32,&cwmode,
									DBUS_TYPE_UINT32,&gi,
									DBUS_TYPE_BYTE,&chan,
									DBUS_TYPE_INVALID);

	reply = dbus_connection_send_with_reply_and_block (dbus_connection,query,-1, &err);
	
	dbus_message_unref(query);
	
	if (NULL == reply) {
		dbus_error_free_for_dcli(&err);

		return -1;
	}
	
	dbus_message_iter_init(reply,&iter);
	dbus_message_iter_get_basic(&iter,&ret);

	return ret;
}
#endif
